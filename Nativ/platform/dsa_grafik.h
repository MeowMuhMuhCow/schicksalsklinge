/*
 * dsa_grafik.h - Grafikmodi (G1), Paket-Lader aller Kategorien (G3) und
 *                Echtzeit-4x-Filter (R4/G5) hinter DSA_GRAFIK / --grafik=<modus>
 *
 * Feste Modusnamen (G1, identisch in Starter und Port):
 *   0 = original   (Standard: DSA_GRAFIK aus, 320x200, 100,00 % pixelidentisch)
 *   1 = scharf     (4x ganzzahlige Skalierung, 1280x800, scharfe Pixelkanten)
 *   2 = xbrz       (4x xBRZ kantengesteuerte Pixel-Art-Skalierung, 1280x800)
 *   3 = scalefx    (4x ScaleFX / HQ4x Subpixel-Ecken-Interpolation, 1280x800)
 *   4 = crt        (4x Dezenter CRT-Filter: Scanlines, Aperture-Grille & Nachleuchten, 1280x800)
 *   5 = paket      (4x Grafikpaket aller Kategorien aus Grafik/pakete/<verfahren>/ + Stiltreu-Pass & Fallback)
 */

#ifndef DSA_GRAFIK_H
#define DSA_GRAFIK_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define DSA_GRAFIK_SCALE  4
#define DSA_HIRES_MAX_W   (320 * DSA_GRAFIK_SCALE)
#define DSA_HIRES_MAX_H   (200 * DSA_GRAFIK_SCALE)

enum {
    DSA_GRAFIK_MODE_OFF        = 0, /* original */
    DSA_GRAFIK_MODE_INTEGER    = 1, /* scharf   */
    DSA_GRAFIK_MODE_XBRZ       = 2, /* xbrz     */
    DSA_GRAFIK_MODE_SCALEFX    = 3, /* scalefx  */
    DSA_GRAFIK_MODE_CRT        = 4, /* crt      */
    DSA_GRAFIK_MODE_PAKET      = 5, /* paket    */
    DSA_GRAFIK_MODE_COUNT      = 6
};

static int      g_dsa_grafik_enabled   = 0;
static int      g_dsa_grafik_mode      = DSA_GRAFIK_MODE_OFF;
static int      g_dsa_grafik_inited    = 0;
static char     g_dsa_verfahren[64]    = "stiltreu";
static char     g_dsa_paket_dir[512]   = "";
static char     g_dsa_fallback_dir[512] = "";

static uint32_t g_dsa_src_argb[320 * 200];
static uint32_t g_dsa_hires_argb[DSA_HIRES_MAX_W * DSA_HIRES_MAX_H];
static uint32_t g_dsa_crt_prev_argb[DSA_HIRES_MAX_W * DSA_HIRES_MAX_H];
static int      g_dsa_crt_has_prev = 0;

/* G5 Leistungs-Messwerte */
static double   g_dsa_pak_load_ms      = 0.0;
static double   g_dsa_render_sum_ms    = 0.0;
static uint32_t g_dsa_render_frames    = 0;
static uint32_t g_dsa_pak_hits_total   = 0;
static uint32_t g_dsa_pak_entries_cnt  = 0;

static double dsa_now_ms(void) {
#ifdef _WIN32
    LARGE_INTEGER freq, cnt;
    if (QueryPerformanceFrequency(&freq) && QueryPerformanceCounter(&cnt) && freq.QuadPart > 0) {
        return (double)cnt.QuadPart * 1000.0 / (double)freq.QuadPart;
    }
    return (double)GetTickCount();
#else
    return 0.0;
#endif
}

static const char *dsa_grafik_mode_id_str(int mode) {
    switch (mode) {
    case DSA_GRAFIK_MODE_INTEGER: return "scharf";
    case DSA_GRAFIK_MODE_XBRZ:    return "xbrz";
    case DSA_GRAFIK_MODE_SCALEFX: return "scalefx";
    case DSA_GRAFIK_MODE_CRT:     return "crt";
    case DSA_GRAFIK_MODE_PAKET:   return "paket";
    default:                      return "original";
    }
}

static const char *dsa_grafik_mode_name(int mode) {
    switch (mode) {
    case DSA_GRAFIK_MODE_INTEGER: return "scharf (Ganzzahlig 4x)";
    case DSA_GRAFIK_MODE_XBRZ:    return "xbrz (4x)";
    case DSA_GRAFIK_MODE_SCALEFX: return "scalefx (4x)";
    case DSA_GRAFIK_MODE_CRT:     return "crt (Roehrenmonitor 4x)";
    case DSA_GRAFIK_MODE_PAKET:   return "paket (Restauriertes Grafikpaket 4x)";
    default:                      return "original (320x200)";
    }
}

static int dsa_parse_mode_token(const char *tok, int *out_mode) {
    if (!tok || !tok[0]) return 0;
    if (strcmp(tok, "0") == 0 || STRICMP(tok, "original") == 0 || STRICMP(tok, "off") == 0 || STRICMP(tok, "aus") == 0) {
        *out_mode = DSA_GRAFIK_MODE_OFF;
        return 1;
    }
    if (strcmp(tok, "1") == 0 || STRICMP(tok, "scharf") == 0 || STRNICMP(tok, "ganz", 4) == 0 || STRNICMP(tok, "int", 3) == 0) {
        *out_mode = DSA_GRAFIK_MODE_INTEGER;
        return 1;
    }
    if (strcmp(tok, "2") == 0 || STRICMP(tok, "xbrz") == 0) {
        *out_mode = DSA_GRAFIK_MODE_XBRZ;
        return 1;
    }
    if (strcmp(tok, "3") == 0 || STRICMP(tok, "scalefx") == 0 || STRNICMP(tok, "hq", 2) == 0) {
        *out_mode = DSA_GRAFIK_MODE_SCALEFX;
        return 1;
    }
    if (strcmp(tok, "4") == 0 || STRICMP(tok, "crt") == 0) {
        *out_mode = DSA_GRAFIK_MODE_CRT;
        return 1;
    }
    if (strcmp(tok, "5") == 0 || STRICMP(tok, "paket") == 0 || STRICMP(tok, "grafikpaket") == 0 || STRNICMP(tok, "port", 4) == 0) {
        *out_mode = DSA_GRAFIK_MODE_PAKET;
        return 1;
    }
    return 0;
}

/* =========================================================================
 * G2 / G3 / G5: Schneller Binaerpaket-Lader (paket.bin < 0,05 s) + BMP-Fallback
 * ========================================================================= */

#pragma pack(push, 1)
struct DsaPakDiskEntry {
    uint64_t hash;
    uint16_t scaled_w;
    uint16_t scaled_h;
    uint8_t  cat_id;
    uint8_t  flags;
    uint16_t orig_w;
    uint32_t data_offset;
    uint32_t data_len;
};
#pragma pack(pop)

struct DsaPakRamItem {
    uint64_t        hash;
    uint16_t        orig_w;
    uint16_t        orig_h;
    uint16_t        scaled_w;
    uint16_t        scaled_h;
    uint8_t         cat_id;
    const uint8_t  *orig_bytes;
    const uint8_t  *bgr24_bytes;
};

static uint8_t       *g_dsa_pak_blob       = NULL;
static DsaPakRamItem *g_dsa_pak_items      = NULL;
static int            g_dsa_pak_item_count = 0;
static int            g_dsa_pak_loaded     = 0;

static void dsa_load_paket_bin_once(void) {
    if (g_dsa_pak_loaded) return;
    g_dsa_pak_loaded = 1;
    if (!g_dsa_paket_dir[0]) return;

    char bin_path[640];
    snprintf(bin_path, sizeof(bin_path), "%s/paket.bin", g_dsa_paket_dir);
    double t0 = dsa_now_ms();
    FILE *fp = fopen(bin_path, "rb");
    if (!fp) return;

    fseek(fp, 0, SEEK_END);
    long fsz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (fsz < 16 || fsz > 128 * 1024 * 1024) {
        fclose(fp);
        return;
    }

    g_dsa_pak_blob = (uint8_t *)malloc((size_t)fsz);
    if (!g_dsa_pak_blob) {
        fclose(fp);
        return;
    }
    if ((long)fread(g_dsa_pak_blob, 1, (size_t)fsz, fp) != fsz) {
        free(g_dsa_pak_blob);
        g_dsa_pak_blob = NULL;
        fclose(fp);
        return;
    }
    fclose(fp);

    if (memcmp(g_dsa_pak_blob, "DSAPAK02", 8) != 0) {
        free(g_dsa_pak_blob);
        g_dsa_pak_blob = NULL;
        return;
    }

    uint32_t cnt = *(uint32_t *)(g_dsa_pak_blob + 8);
    if (cnt == 0 || 16u + cnt * sizeof(DsaPakDiskEntry) > (uint32_t)fsz) {
        free(g_dsa_pak_blob);
        g_dsa_pak_blob = NULL;
        return;
    }

    g_dsa_pak_items = (DsaPakRamItem *)calloc(cnt, sizeof(DsaPakRamItem));
    const DsaPakDiskEntry *disk_tab = (const DsaPakDiskEntry *)(g_dsa_pak_blob + 16);
    int valid_cnt = 0;
    for (uint32_t i = 0; i < cnt; ++i) {
        const DsaPakDiskEntry &de = disk_tab[i];
        uint16_t ow = de.orig_w ? de.orig_w : (de.scaled_w / 4);
        uint16_t oh = de.scaled_h / 4;
        uint32_t orig_sz = (uint32_t)ow * (uint32_t)oh;
        uint32_t bgr_sz  = (uint32_t)de.scaled_w * (uint32_t)de.scaled_h * 3u;
        if (de.data_offset + orig_sz + bgr_sz <= (uint32_t)fsz) {
            DsaPakRamItem &it = g_dsa_pak_items[valid_cnt++];
            it.hash        = de.hash;
            it.orig_w      = ow;
            it.orig_h      = oh;
            it.scaled_w    = de.scaled_w;
            it.scaled_h    = de.scaled_h;
            it.cat_id      = de.cat_id;
            it.orig_bytes  = g_dsa_pak_blob + de.data_offset;
            it.bgr24_bytes = g_dsa_pak_blob + de.data_offset + orig_sz;
        }
    }
    g_dsa_pak_item_count  = valid_cnt;
    g_dsa_pak_entries_cnt = (uint32_t)valid_cnt;
    g_dsa_pak_load_ms     = dsa_now_ms() - t0;
}

static const DsaPakRamItem *dsa_find_pak_item(uint64_t hash) {
    if (!g_dsa_pak_loaded) dsa_load_paket_bin_once();
    /* Binaersuche, da paket.bin aufsteigend nach Hash sortiert ist */
    int lo = 0, hi = g_dsa_pak_item_count - 1;
    while (lo <= hi) {
        int mid = (lo + hi) >> 1;
        uint64_t mh = g_dsa_pak_items[mid].hash;
        if (mh == hash) return &g_dsa_pak_items[mid];
        if (mh < hash) lo = mid + 1;
        else           hi = mid - 1;
    }
    return NULL;
}

static void dsa_discover_paket_dirs(const char *exe_dir) {
    const char *env_pdir = getenv("DSA_GRAFIK_PAKET_DIR");
    if (env_pdir && env_pdir[0]) {
        strncpy(g_dsa_paket_dir, env_pdir, sizeof(g_dsa_paket_dir) - 1);
    }
    if (!g_dsa_paket_dir[0]) {
        const char *prefixes[] = { "", "../", "../../", NULL };
        for (int i = 0; prefixes[i]; ++i) {
            char cand[600], test_m[640];
            snprintf(cand, sizeof(cand), "%sGrafik/pakete/%s", prefixes[i], g_dsa_verfahren);
            snprintf(test_m, sizeof(test_m), "%s/manifest.json", cand);
            FILE *fp = fopen(test_m, "rb");
            if (fp) {
                fclose(fp);
                strncpy(g_dsa_paket_dir, cand, sizeof(g_dsa_paket_dir) - 1);
                break;
            }
        }
        if (!g_dsa_paket_dir[0] && exe_dir && exe_dir[0]) {
            char cand[600], test_m[640];
            snprintf(cand, sizeof(cand), "%s\\..\\..\\Grafik\\pakete\\%s", exe_dir, g_dsa_verfahren);
            snprintf(test_m, sizeof(test_m), "%s\\manifest.json", cand);
            FILE *fp = fopen(test_m, "rb");
            if (fp) {
                fclose(fp);
                strncpy(g_dsa_paket_dir, cand, sizeof(g_dsa_paket_dir) - 1);
            }
        }
    }
    /* Fallback-Ordner aus Etappe 1 (Grafik/paket_prototyp/portraets) */
    const char *fb_cands[] = {
        "Grafik/paket_prototyp/portraets",
        "../Grafik/paket_prototyp/portraets",
        "../../Grafik/paket_prototyp/portraets",
        NULL
    };
    for (int i = 0; fb_cands[i]; ++i) {
        char test_m[640];
        snprintf(test_m, sizeof(test_m), "%s/manifest.json", fb_cands[i]);
        FILE *fp = fopen(test_m, "rb");
        if (fp) {
            fclose(fp);
            strncpy(g_dsa_fallback_dir, fb_cands[i], sizeof(g_dsa_fallback_dir) - 1);
            break;
        }
    }
    if (!g_dsa_paket_dir[0] && g_dsa_fallback_dir[0]) {
        strncpy(g_dsa_paket_dir, g_dsa_fallback_dir, sizeof(g_dsa_paket_dir) - 1);
    }
}

static void dsa_grafik_init_from_env(const char *exe_dir) {
    (void)exe_dir;
    if (g_dsa_grafik_inited) return;
    g_dsa_grafik_inited  = 1;
    /* S1 (Etappe 3): Alle Grafikfilter (xbrz/scalefx/crt/paket) und FNV-1a-Grafikpaket-Laden
     * sind abgeklemmt; alte --grafik= / DSA_GRAFIK-Werte fallen stillschweigend auf Standard zurueck. */
    g_dsa_grafik_enabled = 0;
    g_dsa_grafik_mode    = DSA_GRAFIK_MODE_OFF;
}

/* =========================================================================
 * Farb-Hilfsfunktionen fuer xBRZ / ScaleFX / CRT / Stiltreu
 * ========================================================================= */

static inline float dsa_ycbcr_dist(uint32_t c1, uint32_t c2) {
    if (c1 == c2) return 0.0f;
    int r1 = (int)((c1 >> 16) & 0xFF), g1 = (int)((c1 >> 8) & 0xFF), b1 = (int)(c1 & 0xFF);
    int r2 = (int)((c2 >> 16) & 0xFF), g2 = (int)((c2 >> 8) & 0xFF), b2 = (int)(c2 & 0xFF);
    float dr = (float)(r1 - r2);
    float dg = (float)(g1 - g2);
    float db = (float)(b1 - b2);
    float y  =  0.299f * dr + 0.587f * dg + 0.114f * db;
    float cb = -0.168736f * dr - 0.331264f * dg + 0.5f * db;
    float cr =  0.5f * dr - 0.418688f * dg - 0.081312f * db;
    return sqrtf((2.2f * y) * (2.2f * y) + cb * cb + cr * cr);
}

static inline uint32_t dsa_lerp_argb(uint32_t c1, uint32_t c2, int w2_num, int den) {
    if (c1 == c2 || w2_num <= 0) return c1;
    if (w2_num >= den) return c2;
    int w1 = den - w2_num;
    uint32_t r = (((c1 >> 16) & 0xFF) * w1 + ((c2 >> 16) & 0xFF) * w2_num + (den >> 1)) / den;
    uint32_t g = (((c1 >> 8)  & 0xFF) * w1 + ((c2 >> 8)  & 0xFF) * w2_num + (den >> 1)) / den;
    uint32_t b = (( c1        & 0xFF) * w1 + ( c2        & 0xFF) * w2_num + (den >> 1)) / den;
    return 0xFF000000u | (r << 16) | (g << 8) | b;
}

static inline uint32_t dsa_get_clamped(const uint32_t *src, int w, int h, int x, int y) {
    if (x < 0) x = 0; else if (x >= w) x = w - 1;
    if (y < 0) y = 0; else if (y >= h) y = h - 1;
    return src[y * w + x];
}

/* =========================================================================
 * Modus 1 (scharf): Ganzzahlige Skalierung 4x (Nearest-Neighbor)
 * ========================================================================= */

static void dsa_scale4x_integer(const uint32_t *src, uint32_t *dst, int w, int h) {
    int W = w * 4;
    for (int y = 0; y < h; ++y) {
        const uint32_t *srow = src + y * w;
        for (int dy = 0; dy < 4; ++dy) {
            uint32_t *drow = dst + (y * 4 + dy) * W;
            for (int x = 0; x < w; ++x) {
                uint32_t c = srow[x];
                drow[x * 4 + 0] = c;
                drow[x * 4 + 1] = c;
                drow[x * 4 + 2] = c;
                drow[x * 4 + 3] = c;
            }
        }
    }
}

/* =========================================================================
 * Modus 2 (xbrz): xBRZ 4x
 * ========================================================================= */

static void dsa_scale4x_xbrz(const uint32_t *src, uint32_t *dst, int w, int h) {
    const float EQ_THR = 32.0f;
    int W = w * 4;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            uint32_t E = src[y * w + x];
            uint32_t B = dsa_get_clamped(src, w, h, x,     y - 1);
            uint32_t D = dsa_get_clamped(src, w, h, x - 1, y);
            uint32_t F = dsa_get_clamped(src, w, h, x + 1, y);
            uint32_t H = dsa_get_clamped(src, w, h, x,     y + 1);

            if (B == E && D == E && F == E && H == E) {
                int bx = x * 4, by = y * 4;
                for (int dy = 0; dy < 4; ++dy) {
                    uint32_t *drow = dst + (by + dy) * W + bx;
                    drow[0] = drow[1] = drow[2] = drow[3] = E;
                }
                continue;
            }

            uint32_t A = dsa_get_clamped(src, w, h, x - 1, y - 1);
            uint32_t C = dsa_get_clamped(src, w, h, x + 1, y - 1);
            uint32_t G = dsa_get_clamped(src, w, h, x - 1, y + 1);
            uint32_t I = dsa_get_clamped(src, w, h, x + 1, y + 1);

            uint32_t blk[16];
            for (int k = 0; k < 16; ++k) blk[k] = E;

            if (D != B && E != D && E != B) {
                float wd1 = dsa_ycbcr_dist(E, C) + dsa_ycbcr_dist(E, G) + dsa_ycbcr_dist(D, B) * 2.0f + dsa_ycbcr_dist(A, E);
                float wd2 = dsa_ycbcr_dist(B, D) + dsa_ycbcr_dist(A, G) + dsa_ycbcr_dist(A, C) + dsa_ycbcr_dist(E, D) + dsa_ycbcr_dist(E, B);
                if (wd2 < wd1 || dsa_ycbcr_dist(D, B) < EQ_THR) {
                    uint32_t nc = (dsa_ycbcr_dist(E, D) <= dsa_ycbcr_dist(E, B)) ? D : B;
                    int steep   = (dsa_ycbcr_dist(D, G) < EQ_THR && E != G);
                    int shallow = (dsa_ycbcr_dist(B, C) < EQ_THR && E != C);
                    blk[0] = dsa_lerp_argb(E, nc, 7, 8);
                    blk[1] = dsa_lerp_argb(E, nc, shallow ? 4 : 2, 8);
                    blk[4] = dsa_lerp_argb(E, nc, steep   ? 4 : 2, 8);
                    if (steep && shallow) {
                        blk[2] = dsa_lerp_argb(E, nc, 1, 8);
                        blk[8] = dsa_lerp_argb(E, nc, 1, 8);
                        blk[5] = dsa_lerp_argb(E, nc, 2, 8);
                    }
                }
            }
            if (B != F && E != B && E != F) {
                float wd1 = dsa_ycbcr_dist(E, A) + dsa_ycbcr_dist(E, I) + dsa_ycbcr_dist(B, F) * 2.0f + dsa_ycbcr_dist(C, E);
                float wd2 = dsa_ycbcr_dist(B, F) + dsa_ycbcr_dist(C, A) + dsa_ycbcr_dist(C, I) + dsa_ycbcr_dist(E, B) + dsa_ycbcr_dist(E, F);
                if (wd2 < wd1 || dsa_ycbcr_dist(B, F) < EQ_THR) {
                    uint32_t nc = (dsa_ycbcr_dist(E, F) <= dsa_ycbcr_dist(E, B)) ? F : B;
                    int steep   = (dsa_ycbcr_dist(F, I) < EQ_THR && E != I);
                    int shallow = (dsa_ycbcr_dist(B, A) < EQ_THR && E != A);
                    blk[3] = dsa_lerp_argb(E, nc, 7, 8);
                    blk[2] = dsa_lerp_argb(E, nc, shallow ? 4 : 2, 8);
                    blk[7] = dsa_lerp_argb(E, nc, steep   ? 4 : 2, 8);
                    if (steep && shallow) {
                        blk[1]  = dsa_lerp_argb(E, nc, 1, 8);
                        blk[11] = dsa_lerp_argb(E, nc, 1, 8);
                        blk[6]  = dsa_lerp_argb(E, nc, 2, 8);
                    }
                }
            }
            if (F != H && E != F && E != H) {
                float wd1 = dsa_ycbcr_dist(E, C) + dsa_ycbcr_dist(E, G) + dsa_ycbcr_dist(F, H) * 2.0f + dsa_ycbcr_dist(I, E);
                float wd2 = dsa_ycbcr_dist(F, H) + dsa_ycbcr_dist(I, C) + dsa_ycbcr_dist(I, G) + dsa_ycbcr_dist(E, F) + dsa_ycbcr_dist(E, H);
                if (wd2 < wd1 || dsa_ycbcr_dist(F, H) < EQ_THR) {
                    uint32_t nc = (dsa_ycbcr_dist(E, F) <= dsa_ycbcr_dist(E, H)) ? F : H;
                    int steep   = (dsa_ycbcr_dist(F, C) < EQ_THR && E != C);
                    int shallow = (dsa_ycbcr_dist(H, G) < EQ_THR && E != G);
                    blk[15] = dsa_lerp_argb(E, nc, 7, 8);
                    blk[14] = dsa_lerp_argb(E, nc, shallow ? 4 : 2, 8);
                    blk[11] = dsa_lerp_argb(E, nc, steep   ? 4 : 2, 8);
                    if (steep && shallow) {
                        blk[13] = dsa_lerp_argb(E, nc, 1, 8);
                        blk[7]  = dsa_lerp_argb(E, nc, 1, 8);
                        blk[10] = dsa_lerp_argb(E, nc, 2, 8);
                    }
                }
            }
            if (H != D && E != H && E != D) {
                float wd1 = dsa_ycbcr_dist(E, A) + dsa_ycbcr_dist(E, I) + dsa_ycbcr_dist(H, D) * 2.0f + dsa_ycbcr_dist(G, E);
                float wd2 = dsa_ycbcr_dist(H, D) + dsa_ycbcr_dist(G, A) + dsa_ycbcr_dist(G, I) + dsa_ycbcr_dist(E, H) + dsa_ycbcr_dist(E, D);
                if (wd2 < wd1 || dsa_ycbcr_dist(H, D) < EQ_THR) {
                    uint32_t nc = (dsa_ycbcr_dist(E, D) <= dsa_ycbcr_dist(E, H)) ? D : H;
                    int steep   = (dsa_ycbcr_dist(D, A) < EQ_THR && E != A);
                    int shallow = (dsa_ycbcr_dist(H, I) < EQ_THR && E != I);
                    blk[12] = dsa_lerp_argb(E, nc, 7, 8);
                    blk[13] = dsa_lerp_argb(E, nc, shallow ? 4 : 2, 8);
                    blk[8]  = dsa_lerp_argb(E, nc, steep   ? 4 : 2, 8);
                    if (steep && shallow) {
                        blk[14] = dsa_lerp_argb(E, nc, 1, 8);
                        blk[4]  = dsa_lerp_argb(E, nc, 1, 8);
                        blk[9]  = dsa_lerp_argb(E, nc, 2, 8);
                    }
                }
            }

            int bx = x * 4, by = y * 4;
            for (int dy = 0; dy < 4; ++dy) {
                uint32_t *drow = dst + (by + dy) * W + bx;
                drow[0] = blk[dy * 4 + 0];
                drow[1] = blk[dy * 4 + 1];
                drow[2] = blk[dy * 4 + 2];
                drow[3] = blk[dy * 4 + 3];
            }
        }
    }
}

/* =========================================================================
 * Modus 3 (scalefx): ScaleFX / HQ4x
 * ========================================================================= */

static void dsa_scale4x_scalefx(const uint32_t *src, uint32_t *dst, int w, int h) {
    const float THR = 42.0f;
    int W = w * 4;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            uint32_t E = src[y * w + x];
            uint32_t B = dsa_get_clamped(src, w, h, x,     y - 1);
            uint32_t D = dsa_get_clamped(src, w, h, x - 1, y);
            uint32_t F = dsa_get_clamped(src, w, h, x + 1, y);
            uint32_t H = dsa_get_clamped(src, w, h, x,     y + 1);

            int bx = x * 4, by = y * 4;
            if (B == E && D == E && F == E && H == E) {
                for (int dy = 0; dy < 4; ++dy) {
                    uint32_t *drow = dst + (by + dy) * W + bx;
                    drow[0] = drow[1] = drow[2] = drow[3] = E;
                }
                continue;
            }

            uint32_t A = dsa_get_clamped(src, w, h, x - 1, y - 1);
            uint32_t C = dsa_get_clamped(src, w, h, x + 1, y - 1);
            uint32_t G = dsa_get_clamped(src, w, h, x - 1, y + 1);
            uint32_t I = dsa_get_clamped(src, w, h, x + 1, y + 1);

            for (int dy = 0; dy < 4; ++dy) {
                float wy = (dy + 0.5f) * 0.25f;
                uint32_t v_nb = (wy < 0.5f) ? B : H;
                float v_str = (wy < 0.5f) ? (0.5f - wy) : (wy - 0.5f);
                uint32_t *drow = dst + (by + dy) * W + bx;

                for (int dx = 0; dx < 4; ++dx) {
                    float wx = (dx + 0.5f) * 0.25f;
                    uint32_t h_nb = (wx < 0.5f) ? D : F;
                    float h_str = (wx < 0.5f) ? (0.5f - wx) : (wx - 0.5f);
                    uint32_t diag_nb = (wy < 0.5f) ? ((wx < 0.5f) ? A : C) : ((wx < 0.5f) ? G : I);

                    float d_hv = dsa_ycbcr_dist(h_nb, v_nb);
                    float d_eh = dsa_ycbcr_dist(E, h_nb);
                    float d_ev = dsa_ycbcr_dist(E, v_nb);
                    float d_ed = dsa_ycbcr_dist(E, diag_nb);

                    uint32_t px = E;
                    if (d_hv < THR && d_eh > 8.0f && d_ev > 8.0f) {
                        float cw = (h_str + v_str) * 0.85f;
                        if (cw > 0.75f) cw = 0.75f;
                        if (d_ed < d_hv) cw *= 0.5f;
                        uint32_t avg_nb = dsa_lerp_argb(h_nb, v_nb, 1, 2);
                        px = dsa_lerp_argb(E, avg_nb, (int)(cw * 16.0f + 0.5f), 16);
                    } else if (d_eh < THR * 0.65f && h_str > 0.25f) {
                        px = dsa_lerp_argb(E, h_nb, (int)(h_str * 7.2f + 0.5f), 16);
                    } else if (d_ev < THR * 0.65f && v_str > 0.25f) {
                        px = dsa_lerp_argb(E, v_nb, (int)(v_str * 7.2f + 0.5f), 16);
                    }
                    drow[dx] = px;
                }
            }
        }
    }
}

/* =========================================================================
 * Modus 4 (crt): Dezenter CRT-Filter 4x
 * ========================================================================= */

static void dsa_scale4x_crt(const uint32_t *src, uint32_t *dst, int w, int h) {
    int W = w * 4;
    int H = h * 4;
    dsa_scale4x_integer(src, dst, w, h);

    static const int scan_mul[4] = { 256, 252, 238, 198 };

    for (int y = 0; y < H; ++y) {
        int sm = scan_mul[y & 3];
        uint32_t *row = dst + y * W;
        uint32_t *prev_row = g_dsa_crt_prev_argb + y * W;

        for (int x = 0; x < W; ++x) {
            uint32_t c = row[x];
            uint32_t cl = (x > 0) ? row[x - 1] : c;
            uint32_t cr = (x + 1 < W) ? row[x + 1] : c;

            int r = ((int)((c >> 16) & 0xFF) * 224 + (int)((cl >> 16) & 0xFF) * 16 + (int)((cr >> 16) & 0xFF) * 16) >> 8;
            int g = ((int)((c >> 8)  & 0xFF) * 224 + (int)((cl >> 8)  & 0xFF) * 16 + (int)((cr >> 8)  & 0xFF) * 16) >> 8;
            int b = ((int)( c        & 0xFF) * 224 + (int)( cl        & 0xFF) * 16 + (int)( cr        & 0xFF) * 16) >> 8;

            int sub = x & 3;
            if (sub == 0)      { r = (r * 262) >> 8; }
            else if (sub == 1) { g = (g * 262) >> 8; }
            else if (sub == 2) { b = (b * 262) >> 8; }

            r = (r * sm) >> 8;
            g = (g * sm) >> 8;
            b = (b * sm) >> 8;

            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;

            uint32_t cur_out = 0xFF000000u | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
            if (g_dsa_crt_has_prev) {
                cur_out = dsa_lerp_argb(cur_out, prev_row[x], 1, 8);
            }
            row[x] = cur_out;
            prev_row[x] = cur_out;
        }
    }
    g_dsa_crt_has_prev = 1;
}

/* =========================================================================
 * Modus 5 (paket, G3): Stiltreuer Basis-Pass fuer alle Kategorien
 *   (3D-Texturen Stadt & Dungeon, Kampffiguren, Hintergruende, Vorspann,
 *    Schrift nur wo lesbar besser) + Hash-Ueberlagerung aus paket.bin
 * ========================================================================= */

static void dsa_scale4x_stiltreu_all_categories(const unsigned char *vram, const uint32_t *src_argb, uint32_t *dst, int w, int h) {
    const float EQ_THR = 32.0f;
    int W = w * 4;

    for (int y = 0; y < h; ++y) {
        int ym1 = (y > 0) ? (y - 1) : 0;
        int yp1 = (y + 1 < h) ? (y + 1) : (h - 1);
        for (int x = 0; x < w; ++x) {
            int idx_0 = y * w + x;
            uint32_t E = src_argb[idx_0];
            uint8_t  vE = vram[idx_0];
            int xm1 = (x > 0) ? (x - 1) : 0;
            int xp1 = (x + 1 < w) ? (x + 1) : (w - 1);

            uint32_t B = src_argb[ym1 * w + x];
            uint32_t D = src_argb[y * w + xm1];
            uint32_t F = src_argb[y * w + xp1];
            uint32_t H = src_argb[yp1 * w + x];

            int bx = x * 4, by = y * 4;
            if (B == E && D == E && F == E && H == E) {
                for (int dy = 0; dy < 4; ++dy) {
                    uint32_t *drow = dst + (by + dy) * W + bx;
                    drow[0] = drow[1] = drow[2] = drow[3] = E;
                }
                continue;
            }

            /* Kategorie-Erkennung anhand Palettenbank:
             * - Schrift (0xC8..0xFF): Nur exakt gleiche Vordergrund-Indizes diagonal verbinden
             *   -> verhindert Ausfransen, macht Buchstabenboegen gestochen scharf!
             * - 3D-Stadt/Dungeon (0x80..0xBF) & Kampf/Monster (0x01..0x1F) & UI/Vorspann:
             *   2x2-Maserungskern bleibt 100% Originalfarbe E, Ecken werden auf lokaler Rampe geschaerft.
             */
            int is_text_band = (vE >= 0xC8);
            uint32_t blk[16];
            for (int k = 0; k < 16; ++k) blk[k] = E;

            if (is_text_band) {
                /* Scharfe Kontur-Verbindung fuer 1-Bit-Schrift (nur identische Schriftfarbe) */
                if (D == B && D != E) { blk[0] = D; }
                if (B == F && B != E) { blk[3] = B; }
                if (F == H && F != E) { blk[15] = F; }
                if (H == D && H != E) { blk[12] = H; }
            } else {
                int lum_e = (int)((E >> 16) & 0xFF) * 3 + (int)((E >> 8) & 0xFF) * 6 + (int)(E & 0xFF);
                int darker_cnt = 0;
                uint32_t orth[4] = { B, D, F, H };
                for (int k = 0; k < 4; ++k) {
                    int lk = (int)((orth[k] >> 16) & 0xFF) * 3 + (int)((orth[k] >> 8) & 0xFF) * 6 + (int)(orth[k] & 0xFF);
                    if (lk - lum_e > 180) darker_cnt++;
                }
                int is_outline = (darker_cnt >= 2);

                if (!is_outline) {
                    if (D != E && B != E && dsa_ycbcr_dist(D, B) < EQ_THR) {
                        uint32_t nc = (dsa_ycbcr_dist(E, D) <= dsa_ycbcr_dist(E, B)) ? D : B;
                        uint32_t mid = (dsa_ycbcr_dist(E, nc) <= 46.0f) ? dsa_lerp_argb(E, nc, 1, 2) : nc;
                        blk[0] = nc; blk[1] = mid; blk[4] = mid;
                    }
                    if (B != E && F != E && dsa_ycbcr_dist(B, F) < EQ_THR) {
                        uint32_t nc = (dsa_ycbcr_dist(E, F) <= dsa_ycbcr_dist(E, B)) ? F : B;
                        uint32_t mid = (dsa_ycbcr_dist(E, nc) <= 46.0f) ? dsa_lerp_argb(E, nc, 1, 2) : nc;
                        blk[3] = nc; blk[2] = mid; blk[7] = mid;
                    }
                    if (F != E && H != E && dsa_ycbcr_dist(F, H) < EQ_THR) {
                        uint32_t nc = (dsa_ycbcr_dist(E, F) <= dsa_ycbcr_dist(E, H)) ? F : H;
                        uint32_t mid = (dsa_ycbcr_dist(E, nc) <= 46.0f) ? dsa_lerp_argb(E, nc, 1, 2) : nc;
                        blk[15] = nc; blk[14] = mid; blk[11] = mid;
                    }
                    if (H != E && D != E && dsa_ycbcr_dist(H, D) < EQ_THR) {
                        uint32_t nc = (dsa_ycbcr_dist(E, D) <= dsa_ycbcr_dist(E, H)) ? D : H;
                        uint32_t mid = (dsa_ycbcr_dist(E, nc) <= 46.0f) ? dsa_lerp_argb(E, nc, 1, 2) : nc;
                        blk[12] = nc; blk[13] = mid; blk[8] = mid;
                    }
                }
                /* Der 2x2-Maserungskern bleibt 100 % unverfaelschte Originalfarbe E */
                blk[5] = blk[6] = blk[9] = blk[10] = E;
            }

            for (int dy = 0; dy < 4; ++dy) {
                uint32_t *drow = dst + (by + dy) * W + bx;
                drow[0] = blk[dy * 4 + 0];
                drow[1] = blk[dy * 4 + 1];
                drow[2] = blk[dy * 4 + 2];
                drow[3] = blk[dy * 4 + 3];
            }
        }
    }
}

/* =========================================================================
 * Hash-basierte Slot-Ueberlagerung aus paket.bin (Portraets, Gegenstaende,
 * UI-Icons, Kompass, Vorspann/Hintergrund-Banner) mit Fallback auf Original
 * ========================================================================= */

struct DsaOverlaySlotDesc {
    int x0, y0, w, h;
    int trns_key; /* -1 = keine Transparenz, 0 = Index 0 transparent */
};

static const DsaOverlaySlotDesc g_dsa_overlay_slots[] = {
    /* 1. Portraets (32x32 -> 128x128): Charakterbogen (16,9) + 7 Heldenslots in PLAYM_UK */
    {  16,   9, 32, 32, -1 },
    {   3, 157, 32, 32, -1 },
    {  46, 157, 32, 32, -1 },
    {  89, 157, 32, 32, -1 },
    { 132, 157, 32, 32, -1 },
    { 175, 157, 32, 32, -1 },
    { 218, 157, 32, 32, -1 },
    { 275, 157, 32, 32, -1 },
    /* 2. Gegenstaende (16x16 -> 64x64): Alle 23 Inventar-Slots aus g_invslot_iconxy_table (seg046.c) */
    { 0x45, 0x38, 16, 16, -1 },
    { 0x3b, 0x51, 16, 16, -1 },
    { 0x92, 0x51, 16, 16, -1 },
    { 0x45, 0x6b, 16, 16, -1 },
    { 0x88, 0x6b, 16, 16, -1 },
    { 0x45, 0x8d, 16, 16, -1 },
    { 0x88, 0xaf, 16, 16, -1 },
    { 0x10, 0x38, 16, 16, -1 },
    { 0x21, 0x38, 16, 16, -1 },
    { 0x10, 0x49, 16, 16, -1 },
    { 0x21, 0x49, 16, 16, -1 },
    { 0x10, 0x5a, 16, 16, -1 },
    { 0x21, 0x5a, 16, 16, -1 },
    { 0x10, 0x6b, 16, 16, -1 },
    { 0x21, 0x6b, 16, 16, -1 },
    { 0x10, 0x7c, 16, 16, -1 },
    { 0x21, 0x7c, 16, 16, -1 },
    { 0x10, 0x8d, 16, 16, -1 },
    { 0x21, 0x8d, 16, 16, -1 },
    { 0x10, 0x9e, 16, 16, -1 },
    { 0x21, 0x9e, 16, 16, -1 },
    { 0x10, 0xaf, 16, 16, -1 },
    { 0x21, 0xaf, 16, 16, -1 },
    /* 3. Hintergruende / Aktions-Icons (24x24 -> 96x96): 9 Aktions-Icons in PLAYM_UK */
    { 241,  57, 24, 24, -1 },
    { 268,  57, 24, 24, -1 },
    { 295,  57, 24, 24, -1 },
    { 241,  84, 24, 24, -1 },
    { 268,  84, 24, 24, -1 },
    { 295,  84, 24, 24, -1 },
    { 241, 111, 24, 24, -1 },
    { 268, 111, 24, 24, -1 },
    { 295, 111, 24, 24, -1 },
    /* 4. Kompass (52x22 -> 208x88) in Stadt-3D & Dungeon-3D */
    {  94, 115, 52, 22,  0 },
    /* 5. Vollbild-/Vorspann-Banner (64x32 -> 256x128) */
    {   0,   0, 64, 32, -1 },
    { 128,  40, 64, 32, -1 }
};

#define DSA_NUM_OVERLAY_SLOTS ((int)(sizeof(g_dsa_overlay_slots) / sizeof(g_dsa_overlay_slots[0])))
static const DsaPakRamItem *g_dsa_slot_last_hit[DSA_NUM_OVERLAY_SLOTS] = { NULL };

static uint64_t dsa_fnv1a_64_rect(const unsigned char *vram, int stride, int x0, int y0, int w, int h) {
    uint64_t hash = 0xCBF29CE484222325ULL;
    for (int dy = 0; dy < h; ++dy) {
        const unsigned char *row = vram + (y0 + dy) * stride + x0;
        for (int dx = 0; dx < w; ++dx) {
            hash ^= (uint64_t)row[dx];
            hash *= 0x100000001B3ULL;
        }
    }
    return hash;
}

static void dsa_apply_paket_overlays(const unsigned char *vram, const uint32_t *pal_argb32, uint32_t *hires_dst, int w, int h) {
    if (w != 320 || h != 200) return;
    int W = w * 4;

    for (int s = 0; s < DSA_NUM_OVERLAY_SLOTS; ++s) {
        const DsaOverlaySlotDesc &sd = g_dsa_overlay_slots[s];
        uint64_t cur_hash = dsa_fnv1a_64_rect(vram, w, sd.x0, sd.y0, sd.w, sd.h);
        const DsaPakRamItem *hit = dsa_find_pak_item(cur_hash);

        if (hit && hit->orig_w == sd.w && hit->orig_h == sd.h) {
            g_dsa_slot_last_hit[s] = hit;
        } else if (g_dsa_slot_last_hit[s] != NULL) {
            /* Pruefe, ob nur der Software-Mauszeiger ueber dem Slot schwebt (>= 75% identische Originalbytes) */
            const DsaPakRamItem *prev = g_dsa_slot_last_hit[s];
            int same_cnt = 0;
            int total_px = sd.w * sd.h;
            for (int dy = 0; dy < sd.h; ++dy) {
                const unsigned char *vrow = vram + (sd.y0 + dy) * w + sd.x0;
                const unsigned char *orow = prev->orig_bytes + dy * sd.w;
                for (int dx = 0; dx < sd.w; ++dx) {
                    if (vrow[dx] == orow[dx]) same_cnt++;
                }
            }
            if (same_cnt * 4 >= total_px * 3) {
                hit = prev;
            } else {
                g_dsa_slot_last_hit[s] = NULL;
            }
        }

        if (hit) {
            g_dsa_pak_hits_total++;
            int hx0 = sd.x0 * 4;
            int hy0 = sd.y0 * 4;
            int sw  = hit->scaled_w;
            for (int dy = 0; dy < sd.h; ++dy) {
                const unsigned char *vrow = vram + (sd.y0 + dy) * w + sd.x0;
                const unsigned char *orow = hit->orig_bytes + dy * sd.w;
                for (int dx = 0; dx < sd.w; ++dx) {
                    unsigned char v_px = vrow[dx];
                    if (sd.trns_key >= 0 && (int)v_px == sd.trns_key) {
                        continue;
                    }
                    int is_cursor_px = (v_px != orow[dx]);
                    uint32_t cursor_col = pal_argb32[v_px];
                    for (int sy = 0; sy < 4; ++sy) {
                        uint32_t *drow = hires_dst + (hy0 + dy * 4 + sy) * W + (hx0 + dx * 4);
                        const uint8_t *brow = hit->bgr24_bytes + ((dy * 4 + sy) * sw + (dx * 4)) * 3;
                        for (int sx = 0; sx < 4; ++sx) {
                            if (is_cursor_px) {
                                drow[sx] = cursor_col;
                            } else {
                                uint32_t b = brow[sx * 3 + 0];
                                uint32_t g = brow[sx * 3 + 1];
                                uint32_t r = brow[sx * 3 + 2];
                                drow[sx] = 0xFF000000u | (r << 16) | (g << 8) | b;
                            }
                        }
                    }
                }
            }
        }
    }
}

/* =========================================================================
 * Haupt-Renderfunktion fuer einen Frame bei aktivem DSA_GRAFIK + G5-Zeitmessung
 * ========================================================================= */

static void dsa_grafik_render_frame(const unsigned char *vram, const uint32_t *pal_argb32, int w, int h) {
    double t0 = dsa_now_ms();
    int total = w * h;
    for (int i = 0; i < total; ++i) {
        g_dsa_src_argb[i] = pal_argb32[vram[i]];
    }

    switch (g_dsa_grafik_mode) {
    case DSA_GRAFIK_MODE_INTEGER:
        dsa_scale4x_integer(g_dsa_src_argb, g_dsa_hires_argb, w, h);
        break;
    case DSA_GRAFIK_MODE_XBRZ:
        dsa_scale4x_xbrz(g_dsa_src_argb, g_dsa_hires_argb, w, h);
        break;
    case DSA_GRAFIK_MODE_SCALEFX:
        dsa_scale4x_scalefx(g_dsa_src_argb, g_dsa_hires_argb, w, h);
        break;
    case DSA_GRAFIK_MODE_CRT:
        dsa_scale4x_crt(g_dsa_src_argb, g_dsa_hires_argb, w, h);
        break;
    case DSA_GRAFIK_MODE_PAKET:
        dsa_scale4x_stiltreu_all_categories(vram, g_dsa_src_argb, g_dsa_hires_argb, w, h);
        dsa_apply_paket_overlays(vram, pal_argb32, g_dsa_hires_argb, w, h);
        break;
    default:
        dsa_scale4x_integer(g_dsa_src_argb, g_dsa_hires_argb, w, h);
        break;
    }

    double dt = dsa_now_ms() - t0;
    if (dt >= 0.0) {
        g_dsa_render_sum_ms += dt;
        g_dsa_render_frames++;
    }
}

static void dsa_grafik_write_perf_log_if_requested(void) {
    const char *perf_path = getenv("DSA_GRAFIK_PERF_LOG");
    if (!perf_path || !perf_path[0]) return;
    FILE *fp = fopen(perf_path, "a");
    if (!fp) return;
    double avg_ms = (g_dsa_render_frames > 0) ? (g_dsa_render_sum_ms / (double)g_dsa_render_frames) : 0.0;
    double fps    = (avg_ms > 0.001) ? (1000.0 / avg_ms) : 999.9;
    fprintf(fp,
        "Modus=%-8s | Paket_Laden=%.2f ms (Eintraege=%u, Treffer=%u) | Frames=%u | Mittel=%.2f ms/Frame (%.1f FPS)\n",
        dsa_grafik_mode_id_str(g_dsa_grafik_mode),
        g_dsa_pak_load_ms,
        (unsigned int)g_dsa_pak_entries_cnt,
        (unsigned int)g_dsa_pak_hits_total,
        (unsigned int)g_dsa_render_frames,
        avg_ms,
        fps
    );
    fclose(fp);
}

#endif /* DSA_GRAFIK_H */
