/*
 * zip_extract.h - Eigenstaendiger ZIP-Leser und RFC-1951-Deflate-Entpacker
 * fuer den gemeinsamen Starter der Nordlandtrilogie (ohne externe Abhaengigkeiten).
 */

#ifndef STARTER_ZIP_EXTRACT_H
#define STARTER_ZIP_EXTRACT_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

struct ZipEntryInfo {
    std::string name;           /* Pfad innerhalb der ZIP (mit '/' als Trenner) */
    uint16_t    method;         /* 0 = Store, 8 = Deflate */
    uint32_t    comp_size;
    uint32_t    uncomp_size;
    uint32_t    local_hdr_off;
    bool        is_dir;
};

/* =========================================================================
 * RFC 1951 Deflate-Dekomprimierung (Blocktypen 0, 1, 2)
 * ========================================================================= */

struct DeflateBitReader {
    const uint8_t *data;
    size_t         size;
    size_t         byte_pos;
    uint32_t       bit_buf;
    int            bit_cnt;

    void init(const uint8_t *d, size_t s) {
        data = d;
        size = s;
        byte_pos = 0;
        bit_buf = 0;
        bit_cnt = 0;
    }

    void ensure(int n) {
        while (bit_cnt < n && byte_pos < size) {
            bit_buf |= ((uint32_t)data[byte_pos++]) << bit_cnt;
            bit_cnt += 8;
        }
    }

    uint32_t peek(int n) {
        ensure(n);
        return bit_buf & ((1u << n) - 1u);
    }

    void drop(int n) {
        bit_buf >>= n;
        bit_cnt -= n;
    }

    uint32_t read_bits(int n) {
        if (n == 0) return 0;
        ensure(n);
        uint32_t v = bit_buf & ((1u << n) - 1u);
        drop(n);
        return v;
    }

    void align_byte() {
        int rem = bit_cnt & 7;
        if (rem > 0) drop(rem);
    }
};

struct HuffTable {
    uint16_t counts[16];
    uint16_t symbols[320];

    bool build(const uint8_t *lengths, int num_syms) {
        memset(counts, 0, sizeof(counts));
        for (int i = 0; i < num_syms; ++i) {
            if (lengths[i] > 15) return false;
            counts[lengths[i]]++;
        }
        counts[0] = 0;
        uint16_t offsets[16];
        uint16_t sum = 0;
        for (int bits = 1; bits <= 15; ++bits) {
            offsets[bits] = sum;
            sum = (uint16_t)(sum + counts[bits]);
        }
        for (int i = 0; i < num_syms; ++i) {
            uint8_t len = lengths[i];
            if (len != 0) {
                symbols[offsets[len]++] = (uint16_t)i;
            }
        }
        return true;
    }

    int decode(DeflateBitReader &br) const {
        int code = 0;
        int first = 0;
        int index = 0;
        br.ensure(15);
        for (int len = 1; len <= 15; ++len) {
            code |= (int)(br.bit_buf & 1u);
            br.bit_buf >>= 1;
            br.bit_cnt -= 1;
            int count = counts[len];
            if (code - count < first) {
                return symbols[index + (code - first)];
            }
            index += count;
            first += count;
            first <<= 1;
            code <<= 1;
        }
        return -1;
    }
};

static bool inflate_rfc1951(const uint8_t *comp, size_t comp_len, uint8_t *out_buf, size_t out_cap, size_t *out_written) {
    static const uint16_t len_base[29] = {
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258
    };
    static const uint8_t len_extra[29] = {
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0
    };
    static const uint16_t dist_base[30] = {
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577
    };
    static const uint8_t dist_extra[30] = {
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13
    };
    static const uint8_t clen_order[19] = {
        16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
    };

    DeflateBitReader br;
    br.init(comp, comp_len);
    size_t out_pos = 0;

    int bfinal = 0;
    while (!bfinal) {
        bfinal = (int)br.read_bits(1);
        int btype = (int)br.read_bits(2);

        if (btype == 0) {
            br.align_byte();
            uint32_t len = br.read_bits(16);
            uint32_t nlen = br.read_bits(16);
            if ((len ^ 0xFFFFu) != nlen) return false;
            if (out_pos + len > out_cap) return false;
            for (uint32_t i = 0; i < len; ++i) {
                out_buf[out_pos++] = (uint8_t)br.read_bits(8);
            }
        } else if (btype == 1 || btype == 2) {
            HuffTable lit_ht, dist_ht;
            uint8_t lengths[320];
            memset(lengths, 0, sizeof(lengths));

            if (btype == 1) {
                for (int i = 0; i <= 143; ++i) lengths[i] = 8;
                for (int i = 144; i <= 255; ++i) lengths[i] = 9;
                for (int i = 256; i <= 279; ++i) lengths[i] = 7;
                for (int i = 280; i <= 287; ++i) lengths[i] = 8;
                lit_ht.build(lengths, 288);
                for (int i = 0; i < 30; ++i) lengths[i] = 5;
                dist_ht.build(lengths, 30);
            } else {
                int hlit  = (int)br.read_bits(5) + 257;
                int hdist = (int)br.read_bits(5) + 1;
                int hclen = (int)br.read_bits(4) + 4;
                if (hlit > 286 || hdist > 30) return false;

                uint8_t clen[19];
                memset(clen, 0, sizeof(clen));
                for (int i = 0; i < hclen; ++i) {
                    clen[clen_order[i]] = (uint8_t)br.read_bits(3);
                }
                HuffTable clen_ht;
                if (!clen_ht.build(clen, 19)) return false;

                int idx = 0;
                int total = hlit + hdist;
                while (idx < total) {
                    int sym = clen_ht.decode(br);
                    if (sym < 0) return false;
                    if (sym < 16) {
                        lengths[idx++] = (uint8_t)sym;
                    } else {
                        uint8_t rep_val = 0;
                        int rep_cnt = 0;
                        if (sym == 16) {
                            if (idx == 0) return false;
                            rep_val = lengths[idx - 1];
                            rep_cnt = 3 + (int)br.read_bits(2);
                        } else if (sym == 17) {
                            rep_cnt = 3 + (int)br.read_bits(3);
                        } else {
                            rep_cnt = 11 + (int)br.read_bits(7);
                        }
                        if (idx + rep_cnt > total) return false;
                        while (rep_cnt-- > 0) lengths[idx++] = rep_val;
                    }
                }
                if (!lit_ht.build(lengths, hlit)) return false;
                if (!dist_ht.build(lengths + hlit, hdist)) return false;
            }

            while (1) {
                int sym = lit_ht.decode(br);
                if (sym < 0) return false;
                if (sym < 256) {
                    if (out_pos >= out_cap) return false;
                    out_buf[out_pos++] = (uint8_t)sym;
                } else if (sym == 256) {
                    break;
                } else {
                    int lidx = sym - 257;
                    if (lidx < 0 || lidx >= 29) return false;
                    size_t copy_len = (size_t)len_base[lidx] + (size_t)br.read_bits(len_extra[lidx]);
                    int dsym = dist_ht.decode(br);
                    if (dsym < 0 || dsym >= 30) return false;
                    size_t copy_dist = (size_t)dist_base[dsym] + (size_t)br.read_bits(dist_extra[dsym]);
                    if (copy_dist == 0 || copy_dist > out_pos || out_pos + copy_len > out_cap) return false;
                    for (size_t k = 0; k < copy_len; ++k) {
                        out_buf[out_pos] = out_buf[out_pos - copy_dist];
                        out_pos++;
                    }
                }
            }
        } else {
            return false;
        }
    }

    if (out_written) *out_written = out_pos;
    return true;
}

/* =========================================================================
 * ZIP Central Directory Parser & Datei-Entpacker
 * ========================================================================= */

static inline uint16_t zip_rd16(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static inline uint32_t zip_rd32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static bool zip_list_entries(const char *zip_win_path, std::vector<ZipEntryInfo> &out_entries) {
    out_entries.clear();
    FILE *fp = fopen(zip_win_path, "rb");
    if (!fp) return false;

    fseek(fp, 0, SEEK_END);
    long file_sz = ftell(fp);
    if (file_sz < 22) {
        fclose(fp);
        return false;
    }

    long search_len = (file_sz < 65557) ? file_sz : 65557;
    std::vector<uint8_t> tail((size_t)search_len);
    fseek(fp, file_sz - search_len, SEEK_SET);
    if (fread(tail.data(), 1, (size_t)search_len, fp) != (size_t)search_len) {
        fclose(fp);
        return false;
    }

    long eocd_pos = -1;
    for (long i = search_len - 22; i >= 0; --i) {
        if (tail[i] == 0x50 && tail[i + 1] == 0x4B && tail[i + 2] == 0x05 && tail[i + 3] == 0x06) {
            eocd_pos = i;
            break;
        }
    }
    if (eocd_pos < 0) {
        fclose(fp);
        return false;
    }

    uint16_t num_entries = zip_rd16(&tail[eocd_pos + 10]);
    uint32_t cd_size     = zip_rd32(&tail[eocd_pos + 12]);
    uint32_t cd_offset   = zip_rd32(&tail[eocd_pos + 16]);
    if ((uint64_t)cd_offset + (uint64_t)cd_size > (uint64_t)file_sz) {
        fclose(fp);
        return false;
    }

    std::vector<uint8_t> cd_buf(cd_size);
    fseek(fp, (long)cd_offset, SEEK_SET);
    if (cd_size > 0 && fread(cd_buf.data(), 1, cd_size, fp) != cd_size) {
        fclose(fp);
        return false;
    }
    fclose(fp);

    size_t pos = 0;
    for (uint16_t idx = 0; idx < num_entries && pos + 46 <= cd_size; ++idx) {
        const uint8_t *p = &cd_buf[pos];
        if (p[0] != 0x50 || p[1] != 0x4B || p[2] != 0x01 || p[3] != 0x02) break;
        uint16_t method    = zip_rd16(p + 10);
        uint32_t comp_sz   = zip_rd32(p + 20);
        uint32_t uncomp_sz = zip_rd32(p + 24);
        uint16_t name_len  = zip_rd16(p + 28);
        uint16_t extra_len = zip_rd16(p + 30);
        uint16_t comm_len  = zip_rd16(p + 32);
        uint32_t loc_off   = zip_rd32(p + 42);

        if (pos + 46 + name_len > cd_size) break;
        std::string name((const char *)(p + 46), name_len);
        for (size_t k = 0; k < name.size(); ++k) {
            if (name[k] == '\\') name[k] = '/';
        }

        ZipEntryInfo e;
        e.name          = name;
        e.method        = method;
        e.comp_size     = comp_sz;
        e.uncomp_size   = uncomp_sz;
        e.local_hdr_off = loc_off;
        e.is_dir        = (!name.empty() && name.back() == '/');
        out_entries.push_back(e);

        pos += 46 + (size_t)name_len + (size_t)extra_len + (size_t)comm_len;
    }

    return !out_entries.empty();
}

static bool zip_extract_single_entry(FILE *fp, const ZipEntryInfo &e, const char *dst_win_path) {
    uint8_t lhdr[30];
    if (fseek(fp, (long)e.local_hdr_off, SEEK_SET) != 0) return false;
    if (fread(lhdr, 1, 30, fp) != 30) return false;
    if (lhdr[0] != 0x50 || lhdr[1] != 0x4B || lhdr[2] != 0x03 || lhdr[3] != 0x04) return false;

    uint16_t name_len  = zip_rd16(lhdr + 26);
    uint16_t extra_len = zip_rd16(lhdr + 28);
    long data_off = (long)e.local_hdr_off + 30 + (long)name_len + (long)extra_len;
    if (fseek(fp, data_off, SEEK_SET) != 0) return false;

    std::vector<uint8_t> comp_buf(e.comp_size);
    if (e.comp_size > 0 && fread(comp_buf.data(), 1, e.comp_size, fp) != e.comp_size) {
        return false;
    }

    FILE *out_fp = fopen(dst_win_path, "wb");
    if (!out_fp) return false;

    bool ok = false;
    if (e.method == 0) {
        if (e.comp_size == 0 || fwrite(comp_buf.data(), 1, e.comp_size, out_fp) == e.comp_size) {
            ok = true;
        }
    } else if (e.method == 8) {
        std::vector<uint8_t> out_buf(e.uncomp_size > 0 ? e.uncomp_size : 1);
        size_t written = 0;
        if (e.uncomp_size == 0) {
            ok = true;
        } else if (inflate_rfc1951(comp_buf.data(), comp_buf.size(), out_buf.data(), out_buf.size(), &written) &&
                   written == e.uncomp_size) {
            if (fwrite(out_buf.data(), 1, written, out_fp) == written) {
                ok = true;
            }
        }
    }

    fclose(out_fp);
    return ok;
}

#endif /* STARTER_ZIP_EXTRACT_H */
