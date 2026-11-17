/*
 * dsa_intro.cpp - Nativer 8086-Interpreter fuer das Original-Intro (INTRO_E.OVL)
 *
 * Herkunft: Uebersetzt aus werkzeuge/intro.js von MobileEyes
 *           (https://github.com/Kerygma83/mobile-eyes, Commit b5e8cc39,
 *           Autor: Gero Ohrner, Lizenz: GNU General Public License v3.0).
 *
 * Im DOS-Original startet DSA1_CD.BAT / SCHICK.BAT zuerst INTRO.EXE, das den
 * Musiktreiber laedt und INTRO_E.OVL (Turbo Pascal, VGA-Modus 13h, 320x200x8)
 * ausfuehrt. Hier laeuft INTRO_E.OVL im Modus "nomusic" fuer genau einen
 * Durchlauf; die von Prozedur 0x6f angeforderten Musikstuecke (Stueck 0 = Logos,
 * Titel, Abspann; Stueck 1 = Szenen) werden wie in der GOG-Fassung und in
 * MobileEyes als CD-Audio-Spuren Track 2 (MUSIC/Track02.OGG) und Track 3
 * (MUSIC/Track03.OGG) abgespielt. Jede Taste (ausser F10/F11/F12/Alt+Enter)
 * oder ein Mausklick beendet das Intro sofort.
 */

#include "dsa_platform.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include <vector>

extern "C" {
extern unsigned char *g_vga_memstart;
extern volatile signed int g_mouse1_event2;
extern volatile signed int g_mouse_leftclick_event;
extern volatile signed int g_mouse_rightclick_event;
void wait_for_vsync(void);
}

namespace {

static constexpr int64_t CPU_HZ = 25000000LL;
static constexpr int64_t PIT_HZ = 1193182LL;
static constexpr int64_t VSYNC_HZ = 70LL;
static constexpr uint16_t SEG_LADEN = 0x1000;
static constexpr uint16_t BIOS_SEG = 0xf000;

struct IntroVm {
    std::map<std::string, std::vector<uint8_t>> dateien;
    std::vector<uint8_t> m;
    uint16_t r[8]; // ax cx dx bx sp bp si di
    uint16_t s[4]; // es cs ss ds
    uint16_t ip;
    uint16_t fl;
    int64_t befehle;
    int64_t gespult;
    int64_t grenze;
    int64_t letzte3da;
    int64_t letzteTastenfrage;
    int letzteTaste;
    int letzterStatus;
    int64_t schreibe;

    struct {
        int ip;
        int cs;
        int64_t befehle;
        int64_t schreibe;
        uint16_t regs[12];
    } schleife;

    uint8_t palette[768];
    int dacSchreib;
    int dacLese;
    int dacTeil;
    int pitTeiler;
    int pitZugriff;
    int pitLo;
    int64_t pitNaechst;
    int pitModus;
    int64_t pitStart;
    int pitLatch;
    bool pitLeseHi;
    int pitKanal;
    bool irqWartet;
    bool irqAktiv;
    int picMaske;
    std::vector<uint16_t> tasten;

    struct HandleEntry {
        const std::vector<uint8_t> *d;
        size_t pos;
    };
    std::map<int, HandleEntry> handles;
    int naechstesHandle;
    uint16_t speicherFrei;
    uint16_t psp;
    bool hasDta;
    uint16_t dtaSeg;
    uint16_t dtaOff;
    bool beendet;
    bool fehler;
    bool angehalten;
    int durchlaeufe;
    int64_t trackStartBefehle;
    int aktuellerTrack;

    int segUeber;
    int ea;
    int eaReg;
    uint16_t eaOff;
    uint16_t eaSeg;

    IntroVm()
        : m(0x110000, 0),
          ip(0),
          fl(0x0202),
          befehle(0),
          gespult(0),
          grenze(0),
          letzte3da(-1000000000LL),
          letzteTastenfrage(-1000000000LL),
          letzteTaste(0),
          letzterStatus(-1),
          schreibe(0),
          dacSchreib(0),
          dacLese(0),
          dacTeil(0),
          pitTeiler(65536),
          pitZugriff(3),
          pitLo(-1),
          pitNaechst(0),
          pitModus(3),
          pitStart(0),
          pitLatch(-1),
          pitLeseHi(false),
          pitKanal(0),
          irqWartet(false),
          irqAktiv(false),
          picMaske(0),
          naechstesHandle(5),
          speicherFrei(0),
          psp(0),
          hasDta(false),
          dtaSeg(0),
          dtaOff(0),
          beendet(false),
          fehler(false),
          angehalten(false),
          durchlaeufe(0),
          trackStartBefehle(0),
          aktuellerTrack(0),
          segUeber(-1),
          ea(-1),
          eaReg(0),
          eaOff(0),
          eaSeg(0) {
        std::memset(r, 0, sizeof(r));
        std::memset(s, 0, sizeof(s));
        schleife.ip = -1;
        schleife.cs = -1;
        schleife.befehle = 0;
        schleife.schreibe = 0;
        std::memset(schleife.regs, 0, sizeof(schleife.regs));
        std::memset(palette, 0, sizeof(palette));
    }

    bool init(bool cd_sync = true) {
        biosEinrichten();
        if (!laden("INTRO_E.OVL", cd_sync ? "" : "nomusic")) {
            return false;
        }
        durchlaufMarke();
        if (!cd_sync) {
            musikMarke();
        }
        return true;
    }

    void musikMarke() {
        uint32_t a = ((uint32_t)s[1] << 4) + 0x6f;
        const uint8_t erwartet[8] = {0x55, 0x89, 0xe5, 0x80, 0x3e, 0x2a, 0x30, 0x01};
        for (int i = 0; i < 8; ++i) {
            if (m[a + i] != erwartet[i]) return;
        }
        m[a] = 0xf1;
        m[a + 1] = 0xfd;
        m[a + 2] = 0x90;
    }

    void durchlaufMarke() {
        uint32_t a = ((uint32_t)s[1] << 4) + 0x4008;
        uint16_t rel = (uint16_t)(m[a + 1] | (m[a + 2] << 8));
        if (m[a] != 0xe8 || ((rel + 0x400b) & 0xffff) != 0x3459) return;
        m[a] = 0xf1;
        m[a + 1] = 0xfe;
        m[a + 2] = 0x90;
        durchlaeufe = 0;
    }

    inline uint32_t lin(uint16_t seg, uint16_t off) const {
        return (((uint32_t)seg << 4) + (uint32_t)off) & 0xfffffu;
    }
    inline uint8_t rb(uint16_t seg, uint16_t off) const {
        return m[lin(seg, off)];
    }
    inline uint16_t rw(uint16_t seg, uint16_t off) const {
        return (uint16_t)(m[lin(seg, off)] | (m[lin(seg, (uint16_t)(off + 1))] << 8));
    }
    inline void wb(uint16_t seg, uint16_t off, uint8_t v) {
        m[lin(seg, off)] = v;
        schreibe++;
    }
    inline void ww(uint16_t seg, uint16_t off, uint16_t v) {
        m[lin(seg, off)] = (uint8_t)(v & 0xff);
        m[lin(seg, (uint16_t)(off + 1))] = (uint8_t)((v >> 8) & 0xff);
        schreibe++;
    }

    void biosEinrichten() {
        for (int n = 0; n < 256; ++n) {
            uint16_t o = (uint16_t)(n * 4);
            ww(0, o, o);
            ww(0, (uint16_t)(o + 2), BIOS_SEG);
            wb(BIOS_SEG, o, 0xf1);
            wb(BIOS_SEG, (uint16_t)(o + 1), (uint8_t)n);
            wb(BIOS_SEG, (uint16_t)(o + 2), 0xcf);
        }
        const uint8_t code[] = {
            0x50, 0x1e, 0x31, 0xc0, 0x8e, 0xd8, 0xff, 0x06, 0x6c, 0x04,
            0x75, 0x04, 0xff, 0x06, 0x6e, 0x04, 0xcd, 0x1c, 0xb0, 0x20,
            0xe6, 0x20, 0x1f, 0x58, 0xcf
        };
        uint16_t o8 = 0x500;
        for (size_t i = 0; i < sizeof(code); ++i) {
            wb(BIOS_SEG, (uint16_t)(o8 + i), code[i]);
        }
        ww(0, 8 * 4, o8);
        ww(0x40, 0x10, 0x0021);
        wb(0x40, 0x49, 3);
        ww(0x40, 0x4a, 80);
        ww(0x40, 0x4c, 4000);
        ww(0x40, 0x63, 0x3d4);
        wb(0x40, 0x84, 24);
        ww(0x40, 0x85, 16);
        ww(0x40, 0x13, 640);
    }

    const std::vector<uint8_t> *datei(const std::string &name) const {
        size_t pos = name.find_last_of("\\/:");
        std::string base = (pos == std::string::npos) ? name : name.substr(pos + 1);
        for (char &c : base) c = (char)std::toupper((unsigned char)c);
        auto it = dateien.find(base);
        return (it != dateien.end()) ? &it->second : nullptr;
    }

    bool laden(const char *name, const char *argumente) {
        const std::vector<uint8_t> *b_ptr = datei(name);
        if (!b_ptr || b_ptr->size() < 0x1c) return false;
        const std::vector<uint8_t> &b = *b_ptr;
        auto w = [&](size_t o) -> uint16_t {
            return (uint16_t)(b[o] | (b[o + 1] << 8));
        };
        uint32_t kopf = (uint32_t)w(8) * 16u;
        uint32_t groesse = (uint32_t)(w(4) - 1) * 512u + (uint32_t)(w(2) ? w(2) : 512) - kopf;
        if (kopf + groesse > b.size()) {
            groesse = (uint32_t)(b.size() - kopf);
        }
        uint16_t start = SEG_LADEN + 0x10;
        std::memcpy(&m[(uint32_t)start * 16u], &b[kopf], groesse);
        uint16_t reloc_cnt = w(6);
        uint16_t reloc_off = w(0x18);
        for (uint16_t i = 0; i < reloc_cnt; ++i) {
            uint16_t off = w(reloc_off + i * 4);
            uint16_t seg = w(reloc_off + i * 4 + 2);
            uint32_t a = (uint32_t)(start + seg) * 16u + off;
            uint16_t v = (uint16_t)((m[a] | (m[a + 1] << 8)) + start);
            m[a] = (uint8_t)(v & 0xff);
            m[a + 1] = (uint8_t)((v >> 8) & 0xff);
        }
        uint16_t ende = 0xa000;
        wb(SEG_LADEN, 0, 0xcd);
        wb(SEG_LADEN, 1, 0x20);
        ww(SEG_LADEN, 2, ende);
        uint16_t umg = 0x0f00;
        const char umgText[] = "PATH=C:\\\0\0\x01\0C:\\INTRO_E.OVL\0";
        for (size_t i = 0; i < sizeof(umgText) - 1; ++i) {
            wb(umg, (uint16_t)i, (uint8_t)umgText[i]);
        }
        ww(SEG_LADEN, 0x2c, umg);
        std::string arg = (argumente && argumente[0]) ? (std::string(" ") + argumente) : std::string("");
        wb(SEG_LADEN, 0x80, (uint8_t)arg.size());
        for (size_t i = 0; i < arg.size(); ++i) {
            wb(SEG_LADEN, (uint16_t)(0x81 + i), (uint8_t)arg[i]);
        }
        wb(SEG_LADEN, (uint16_t)(0x81 + arg.size()), 13);
        psp = SEG_LADEN;
        speicherFrei = (uint16_t)(start + ((groesse + 15u) / 16u) + w(0x0a));
        s[0] = psp;
        s[3] = psp;
        s[1] = (uint16_t)(start + w(0x16));
        ip = w(0x14);
        s[2] = (uint16_t)(start + w(0x0e));
        r[4] = w(0x10);
        return true;
    }

    inline uint8_t get8(int i) const {
        return (i < 4) ? (uint8_t)(r[i] & 0xff) : (uint8_t)(r[i - 4] >> 8);
    }
    inline void set8(int i, int v) {
        if (i < 4) r[i] = (uint16_t)((r[i] & 0xff00) | (v & 0xff));
        else r[i - 4] = (uint16_t)((r[i - 4] & 0x00ff) | ((v & 0xff) << 8));
    }
    inline int flag(int b) const {
        return (fl >> b) & 1;
    }
    inline void setFlag(int b, bool an) {
        if (an) fl |= (uint16_t)(1u << b);
        else fl &= (uint16_t)~(1u << b);
    }
    inline void push(uint16_t v) {
        r[4] = (uint16_t)((r[4] - 2) & 0xffff);
        ww(s[2], r[4], v);
    }
    inline uint16_t pop() {
        uint16_t v = rw(s[2], r[4]);
        r[4] = (uint16_t)((r[4] + 2) & 0xffff);
        return v;
    }
    inline uint8_t holB() {
        uint8_t v = rb(s[1], ip);
        ip = (uint16_t)((ip + 1) & 0xffff);
        return v;
    }
    inline uint16_t holW() {
        uint16_t v = rw(s[1], ip);
        ip = (uint16_t)((ip + 2) & 0xffff);
        return v;
    }

    int modrm() {
        uint8_t b = holB();
        int mod = b >> 6;
        int reg = (b >> 3) & 7;
        int rm = b & 7;
        if (mod == 3) {
            ea = -1;
            eaReg = rm;
            return reg;
        }
        int32_t off = 0;
        int seg = 3;
        switch (rm) {
            case 0: off = r[3] + r[6]; break;
            case 1: off = r[3] + r[7]; break;
            case 2: off = r[5] + r[6]; seg = 2; break;
            case 3: off = r[5] + r[7]; seg = 2; break;
            case 4: off = r[6]; break;
            case 5: off = r[7]; break;
            case 6:
                if (mod == 0) off = holW();
                else { off = r[5]; seg = 2; }
                break;
            case 7: off = r[3]; break;
        }
        if (mod == 1) {
            int d = holB();
            off += (d < 128) ? d : (d - 256);
        } else if (mod == 2) {
            off += holW();
        }
        eaOff = (uint16_t)(off & 0xffff);
        eaSeg = (segUeber >= 0) ? s[segUeber] : s[seg];
        ea = 1;
        return reg;
    }

    inline uint8_t rmB() const {
        return (ea < 0) ? get8(eaReg) : rb(eaSeg, eaOff);
    }
    inline uint16_t rmW() const {
        return (ea < 0) ? r[eaReg] : rw(eaSeg, eaOff);
    }
    inline void setRmB(int v) {
        if (ea < 0) set8(eaReg, v);
        else wb(eaSeg, eaOff, (uint8_t)(v & 0xff));
    }
    inline void setRmW(int v) {
        if (ea < 0) r[eaReg] = (uint16_t)(v & 0xffff);
        else ww(eaSeg, eaOff, (uint16_t)(v & 0xffff));
    }
    inline uint16_t datSeg() const {
        return (segUeber >= 0) ? s[segUeber] : s[3];
    }

    void szp(int v, int w) {
        int mask = w ? 0xffff : 0xff;
        v &= mask;
        setFlag(6, v == 0);
        setFlag(7, (v & (w ? 0x8000 : 0x80)) != 0);
        int p = v & 0xff;
        p ^= p >> 4;
        p ^= p >> 2;
        p ^= p >> 1;
        setFlag(2, !(p & 1));
    }

    uint16_t alu(int op, int a, int b, int w) {
        int mask = w ? 0xffff : 0xff;
        int top = w ? 0x8000 : 0x80;
        int res = 0;
        switch (op) {
            case 0:
            case 2: {
                int c = (op == 2) ? flag(0) : 0;
                res = a + b + c;
                setFlag(0, res > mask);
                setFlag(11, ((a ^ res) & (b ^ res) & top) != 0);
                setFlag(4, ((a ^ b ^ res) & 0x10) != 0);
                break;
            }
            case 5:
            case 3:
            case 7: {
                int c = (op == 3) ? flag(0) : 0;
                res = a - b - c;
                setFlag(0, res < 0);
                setFlag(11, ((a ^ b) & (a ^ res) & top) != 0);
                setFlag(4, ((a ^ b ^ res) & 0x10) != 0);
                break;
            }
            case 1:
                res = a | b;
                setFlag(0, false);
                setFlag(11, false);
                break;
            case 4:
                res = a & b;
                setFlag(0, false);
                setFlag(11, false);
                break;
            case 6:
                res = a ^ b;
                setFlag(0, false);
                setFlag(11, false);
                break;
        }
        res &= mask;
        szp(res, w);
        return (uint16_t)res;
    }

    uint16_t incdec(int v, int w, bool dec) {
        int mask = w ? 0xffff : 0xff;
        int top = w ? 0x8000 : 0x80;
        int res = (dec ? (v - 1) : (v + 1)) & mask;
        setFlag(11, dec ? (v == top) : (res == top));
        setFlag(4, ((v ^ res) & 0x10) != 0);
        szp(res, w);
        return (uint16_t)res;
    }

    uint16_t schieben(int op, int v, int n, int w) {
        int mask = w ? 0xffff : 0xff;
        int top = w ? 0x8000 : 0x80;
        n &= 0x1f;
        if (!n) return (uint16_t)v;
        int cf = flag(0);
        int vorher = v;
        for (int i = 0; i < n; ++i) {
            switch (op) {
                case 0: cf = (v & top) ? 1 : 0; v = ((v << 1) | cf) & mask; break;
                case 1: cf = v & 1; v = (v >> 1) | (cf ? top : 0); break;
                case 2: { int c = (v & top) ? 1 : 0; v = ((v << 1) | cf) & mask; cf = c; break; }
                case 3: { int c = v & 1; v = (v >> 1) | (cf ? top : 0); cf = c; break; }
                case 4:
                case 6: cf = (v & top) ? 1 : 0; v = (v << 1) & mask; break;
                case 5: cf = v & 1; v >>= 1; break;
                case 7: cf = v & 1; v = (v >> 1) | (v & top); break;
            }
        }
        setFlag(0, cf != 0);
        if (op >= 4) szp(v, w);
        if (op == 0 || op == 2 || op == 4 || op == 6) setFlag(11, (((v & top) ? 1 : 0) ^ cf) != 0);
        else if (op == 1 || op == 3) setFlag(11, ((v ^ (v << 1)) & top) != 0);
        else if (op == 5) setFlag(11, (vorher & top) != 0);
        else setFlag(11, false);
        return (uint16_t)v;
    }

    bool bed(int c) const {
        switch (c) {
            case 0: return flag(11) != 0;
            case 1: return flag(11) == 0;
            case 2: return flag(0) != 0;
            case 3: return flag(0) == 0;
            case 4: return flag(6) != 0;
            case 5: return flag(6) == 0;
            case 6: return flag(0) != 0 || flag(6) != 0;
            case 7: return flag(0) == 0 && flag(6) == 0;
            case 8: return flag(7) != 0;
            case 9: return flag(7) == 0;
            case 10: return flag(2) != 0;
            case 11: return flag(2) == 0;
            case 12: return flag(7) != flag(11);
            case 13: return flag(7) == flag(11);
            case 14: return flag(6) != 0 || flag(7) != flag(11);
            case 15: return flag(6) == 0 && flag(7) == flag(11);
        }
        return false;
    }

    void interrupt(int n) {
        push(fl);
        push(s[1]);
        push(ip);
        setFlag(9, false);
        setFlag(8, false);
        ip = rw(0, (uint16_t)(n * 4));
        s[1] = rw(0, (uint16_t)(n * 4 + 2));
    }

    inline int64_t pitTakt() const {
        return (CPU_HZ * (int64_t)pitTeiler) / PIT_HZ;
    }

    uint16_t pitZaehler() const {
        int64_t takte = ((befehle - pitStart) * PIT_HZ) / CPU_HZ;
        int64_t t = pitTeiler;
        if ((pitModus & 3) == 3) return (uint16_t)((t - ((takte * 2) % t)) & 0xfffe);
        return (uint16_t)((t - (takte % t)) & 0xffff);
    }

    void laufen(int64_t bis) {
        if (!pitNaechst) pitNaechst = befehle + pitTakt();
        while (befehle < bis && !beendet) {
            if (befehle >= pitNaechst) {
                pitNaechst += pitTakt();
                irqWartet = true;
            }
            if (irqWartet && !irqAktiv && flag(9) && !(picMaske & 1)) {
                irqWartet = false;
                irqAktiv = true;
                angehalten = false;
                interrupt(8);
            }
            if (angehalten) {
                befehle = std::min(bis, pitNaechst);
                continue;
            }
            int64_t ende = std::min(bis, pitNaechst);
            grenze = ende;
            while (befehle < ende && !beendet && !angehalten) {
                schritt();
                befehle++;
                if (irqWartet && !irqAktiv && (fl & 0x200) && !(picMaske & 1)) break;
            }
        }
    }

    void warteschleife(uint16_t start_ip) {
        bool gleich = (schleife.ip == (int)start_ip &&
                       schleife.cs == (int)s[1] &&
                       (befehle - schleife.befehle) < 32 &&
                       schleife.schreibe == schreibe &&
                       r[0] == schleife.regs[0] && r[1] == schleife.regs[1] &&
                       r[2] == schleife.regs[2] && r[3] == schleife.regs[3] &&
                       r[4] == schleife.regs[4] && r[5] == schleife.regs[5] &&
                       r[6] == schleife.regs[6] && r[7] == schleife.regs[7] &&
                       s[0] == schleife.regs[8] && s[2] == schleife.regs[10] &&
                       s[3] == schleife.regs[11]);
        if (gleich && (fl & 0x200)) {
            vorspulen(pitNaechst);
        }
        schleife.ip = start_ip;
        schleife.cs = s[1];
        schleife.befehle = befehle;
        schleife.schreibe = schreibe;
        for (int i = 0; i < 8; ++i) schleife.regs[i] = r[i];
        schleife.regs[8] = s[0];
        schleife.regs[10] = s[2];
        schleife.regs[11] = s[3];
    }

    void vorspulen(int64_t ziel) {
        int64_t bis = std::min({ziel, grenze ? grenze : ziel, pitNaechst}) - 1;
        if (bis > befehle) {
            gespult += (bis - befehle);
            befehle = bis;
        }
    }

    static inline int32_t vz(int v, int w) {
        return w ? ((v & 0x8000) ? (v - 0x10000) : v) : ((v & 0x80) ? (v - 0x100) : v);
    }

    void gruppe3(int w) {
        int reg = modrm();
        int v = w ? rmW() : rmB();
        switch (reg) {
            case 0:
            case 1:
                alu(4, v, w ? holW() : holB(), w);
                return;
            case 2:
                if (w) setRmW(~v & 0xffff);
                else setRmB(~v & 0xff);
                return;
            case 3: {
                uint16_t e = alu(5, 0, v, w);
                setFlag(0, v != 0);
                if (w) setRmW(e);
                else setRmB(e);
                return;
            }
            case 4: {
                if (w) {
                    uint32_t p = (uint32_t)r[0] * (uint32_t)v;
                    r[0] = (uint16_t)(p & 0xffff);
                    r[2] = (uint16_t)((p >> 16) & 0xffff);
                    bool h = (r[2] != 0);
                    setFlag(0, h);
                    setFlag(11, h);
                } else {
                    uint32_t p = (uint32_t)(r[0] & 0xff) * (uint32_t)v;
                    r[0] = (uint16_t)(p & 0xffff);
                    bool h = ((p >> 8) != 0);
                    setFlag(0, h);
                    setFlag(11, h);
                }
                return;
            }
            case 5: {
                if (w) {
                    int32_t p = vz(r[0], 1) * vz(v, 1);
                    r[0] = (uint16_t)(p & 0xffff);
                    r[2] = (uint16_t)(((uint32_t)p >> 16) & 0xffff);
                    bool h = (p < -32768 || p > 32767);
                    setFlag(0, h);
                    setFlag(11, h);
                } else {
                    int32_t p = vz(r[0] & 0xff, 0) * vz(v, 0);
                    r[0] = (uint16_t)(p & 0xffff);
                    bool h = (p < -128 || p > 127);
                    setFlag(0, h);
                    setFlag(11, h);
                }
                return;
            }
            case 6: {
                if (!v) { interrupt(0); return; }
                if (w) {
                    uint32_t n = ((uint32_t)r[2] << 16) | r[0];
                    uint32_t q = n / (uint32_t)v;
                    if (q > 0xffff) { interrupt(0); return; }
                    r[0] = (uint16_t)q;
                    r[2] = (uint16_t)(n % (uint32_t)v);
                } else {
                    uint32_t n = r[0];
                    uint32_t q = n / (uint32_t)v;
                    if (q > 0xff) { interrupt(0); return; }
                    r[0] = (uint16_t)(((n % (uint32_t)v) << 8) | q);
                }
                return;
            }
            case 7: {
                if (!v) { interrupt(0); return; }
                if (w) {
                    int32_t n = (int32_t)(((uint32_t)r[2] << 16) | r[0]);
                    int32_t d = vz(v, 1);
                    int32_t q = n / d;
                    if (q > 32767 || q < -32768) { interrupt(0); return; }
                    r[0] = (uint16_t)(q & 0xffff);
                    r[2] = (uint16_t)((n - q * d) & 0xffff);
                } else {
                    int32_t n = vz(r[0], 1);
                    int32_t d = vz(v, 0);
                    int32_t q = n / d;
                    if (q > 127 || q < -128) { interrupt(0); return; }
                    r[0] = (uint16_t)((((n - q * d) & 0xff) << 8) | (q & 0xff));
                }
                return;
            }
        }
    }

    void stringOp(uint8_t op, int rep) {
        int w = op & 1;
        int d = flag(10) ? -(w + 1) : (w + 1);
        uint16_t quelle = datSeg();
        auto einmal = [&]() {
            switch (op) {
                case 0xa4: wb(s[0], r[7], rb(quelle, r[6])); r[6] = (uint16_t)(r[6] + d); r[7] = (uint16_t)(r[7] + d); break;
                case 0xa5: ww(s[0], r[7], rw(quelle, r[6])); r[6] = (uint16_t)(r[6] + d); r[7] = (uint16_t)(r[7] + d); break;
                case 0xa6: alu(7, rb(quelle, r[6]), rb(s[0], r[7]), 0); r[6] = (uint16_t)(r[6] + d); r[7] = (uint16_t)(r[7] + d); break;
                case 0xa7: alu(7, rw(quelle, r[6]), rw(s[0], r[7]), 1); r[6] = (uint16_t)(r[6] + d); r[7] = (uint16_t)(r[7] + d); break;
                case 0xaa: wb(s[0], r[7], (uint8_t)(r[0] & 0xff)); r[7] = (uint16_t)(r[7] + d); break;
                case 0xab: ww(s[0], r[7], r[0]); r[7] = (uint16_t)(r[7] + d); break;
                case 0xac: set8(0, rb(quelle, r[6])); r[6] = (uint16_t)(r[6] + d); break;
                case 0xad: r[0] = rw(quelle, r[6]); r[6] = (uint16_t)(r[6] + d); break;
                case 0xae: alu(7, r[0] & 0xff, rb(s[0], r[7]), 0); r[7] = (uint16_t)(r[7] + d); break;
                case 0xaf: alu(7, r[0], rw(s[0], r[7]), 1); r[7] = (uint16_t)(r[7] + d); break;
                case 0x6c: wb(s[0], r[7], (uint8_t)portLesen(r[2], 0)); r[7] = (uint16_t)(r[7] + d); break;
                case 0x6d: ww(s[0], r[7], portLesen(r[2], 1)); r[7] = (uint16_t)(r[7] + d); break;
                case 0x6e: portSchreiben(r[2], rb(quelle, r[6])); r[6] = (uint16_t)(r[6] + d); break;
                case 0x6f: {
                    uint16_t v = rw(quelle, r[6]);
                    portSchreiben(r[2], (uint8_t)(v & 0xff));
                    portSchreiben((uint16_t)(r[2] + 1), (uint8_t)(v >> 8));
                    r[6] = (uint16_t)(r[6] + d);
                    break;
                }
            }
        };
        if (!rep) {
            einmal();
            return;
        }
        bool vergleich = (op == 0xa6 || op == 0xa7 || op == 0xae || op == 0xaf);
        while (r[1]) {
            einmal();
            r[1]--;
            befehle++;
            if (vergleich && (rep == 0xf3 ? !flag(6) : flag(6))) break;
        }
    }

    uint16_t portLesen(uint16_t p, int w = 0) {
        schreibe++;
        uint8_t v = 0xff;
        switch (p) {
            case 0x3da: {
                double bild = (double)CPU_HZ / (double)VSYNC_HZ;
                double zeile = bild / 400.0;
                double phase = std::fmod((double)befehle, bild);
                bool ruecklauf = (phase > bild * 0.95);
                double inZeile = std::fmod(phase, zeile);
                v = ruecklauf ? 0x09 : (inZeile > zeile * 0.8 ? 0x01 : 0x00);
                if ((befehle - letzte3da) < 64 && (int)v == letzterStatus) {
                    double basis = (double)befehle - phase;
                    double ziel;
                    if (ruecklauf) ziel = basis + bild;
                    else if (v & 1) ziel = std::min(basis + (std::floor(phase / zeile) + 1.0) * zeile, basis + bild * 0.95);
                    else ziel = std::min(basis + std::floor(phase / zeile) * zeile + zeile * 0.8, basis + bild * 0.95);
                    vorspulen((int64_t)ziel + 1);
                }
                letzte3da = befehle;
                letzterStatus = v;
                break;
            }
            case 0x3c9:
                v = palette[dacLese * 3 + dacTeil] & 0x3f;
                if (++dacTeil == 3) {
                    dacTeil = 0;
                    dacLese = (dacLese + 1) & 0xff;
                }
                break;
            case 0x40: {
                if (pitLatch < 0) pitLatch = pitZaehler();
                v = (uint8_t)(pitLeseHi ? (pitLatch >> 8) : (pitLatch & 0xff));
                if (pitLeseHi) pitLatch = -1;
                pitLeseHi = !pitLeseHi;
                break;
            }
            case 0x21: v = (uint8_t)picMaske; break;
            case 0x60: v = (uint8_t)letzteTaste; break;
            case 0x61: v = 0; break;
        }
        if (w) {
            return (uint16_t)(v | (portLesen((uint16_t)(p + 1), 0) << 8));
        }
        return v;
    }

    void portSchreiben(uint16_t p, uint8_t val) {
        schreibe++;
        int v = val;
        switch (p) {
            case 0x3c8: dacSchreib = v; dacTeil = 0; break;
            case 0x3c7: dacLese = v; dacTeil = 0; break;
            case 0x3c9:
                palette[dacSchreib * 3 + dacTeil] = (uint8_t)(v & 0x3f);
                if (++dacTeil == 3) {
                    dacTeil = 0;
                    dacSchreib = (dacSchreib + 1) & 0xff;
                }
                break;
            case 0x43:
                if ((v >> 6) != 0) { pitKanal = v >> 6; break; }
                pitKanal = 0;
                if (((v >> 4) & 3) == 0) {
                    pitLatch = pitZaehler();
                    pitLeseHi = false;
                    break;
                }
                pitZugriff = (v >> 4) & 3;
                pitModus = (v >> 1) & 7;
                pitLo = -1;
                pitLatch = -1;
                pitLeseHi = false;
                break;
            case 0x40:
                if (pitKanal != 0) break;
                if (pitZugriff == 3) {
                    if (pitLo < 0) { pitLo = v; break; }
                    v = pitLo | (v << 8);
                    pitLo = -1;
                }
                pitTeiler = v ? v : 65536;
                pitStart = befehle;
                pitNaechst = befehle + pitTakt();
                break;
            case 0x20:
                if (v == 0x20) irqAktiv = false;
                break;
            case 0x21:
                picMaske = v;
                break;
        }
    }

    void dienst(int n) {
        int ah = r[0] >> 8;
        int al = r[0] & 0xff;
        auto cf = [&](bool an) {
            uint32_t a = lin(s[2], (uint16_t)(r[4] + 4));
            uint16_t f = (uint16_t)(m[a] | (m[a + 1] << 8));
            uint16_t nf = an ? (uint16_t)(f | 1u) : (uint16_t)(f & ~1u);
            m[a] = (uint8_t)(nf & 0xff);
            m[a + 1] = (uint8_t)(nf >> 8);
        };
        auto zf = [&](bool an) {
            uint32_t a = lin(s[2], (uint16_t)(r[4] + 4));
            uint16_t f = (uint16_t)(m[a] | (m[a + 1] << 8));
            uint16_t nf = an ? (uint16_t)(f | 0x40u) : (uint16_t)(f & ~0x40u);
            m[a] = (uint8_t)(nf & 0xff);
            m[a + 1] = (uint8_t)(nf >> 8);
        };
        switch (n) {
            case 0x10: video(ah, al); return;
            case 0x16:
                if (ah == 0 || ah == 0x10) {
                    if (tasten.empty()) {
                        ip = (uint16_t)((ip - 2) & 0xffff);
                        angehalten = true;
                        return;
                    }
                    r[0] = tasten.front();
                    tasten.erase(tasten.begin());
                    return;
                }
                if (ah == 1 || ah == 0x11) {
                    if (!tasten.empty()) {
                        r[0] = tasten.front();
                        zf(false);
                        return;
                    }
                    zf(true);
                    if ((befehle - letzteTastenfrage) < 40) vorspulen(pitNaechst);
                    letzteTastenfrage = befehle;
                    return;
                }
                if (ah == 2) { set8(0, 0); return; }
                return;
            case 0x1a:
                if (ah == 0) {
                    r[2] = rw(0x40, 0x6c);
                    r[1] = rw(0x40, 0x6e);
                    set8(0, 0);
                }
                return;
            case 0x21: dos(ah, al, cf); return;
            case 0x20: beendet = true; return;
            case 0x80:
                /* Exakter INT-80h-Handler aus INTRO.EXE (0490:0006..0140):
                 * AX=1: Stueck BX starten (BX=0 -> CD-Track 2, BX=1 -> CD-Track 3)
                 * AX=2: Musik stoppen
                 * AX=3: CD-Sekundenstand gegen Sync-Tabelle (ds:0x1c8..0x1f2) pruefen
                 *       und Szenenschritt 0..21 in den INT-82h-Vektor schreiben. */
                if (r[0] == 1) {
                    trackStartBefehle = befehle;
                    aktuellerTrack = (int)r[3] + 2;
                    dsa_platform_play_cd_track(aktuellerTrack);
                } else if (r[0] == 2) {
                    aktuellerTrack = 0;
                    dsa_platform_stop_cd_audio();
                } else if (r[0] == 3) {
                    static const int cd_sync_table[20] = {
                        1, 36, 37, 71, 72, 107, 108, 153, 154, 192,
                        193, 226, 227, 255, 256, 291, 292, 338, 339, 380
                    };
                    int64_t elapsed_sec = (befehle - trackStartBefehle) / CPU_HZ;
                    uint16_t step = 0;
                    while (step < 20 && cd_sync_table[step] < elapsed_sec) {
                        step++;
                    }
                    if (elapsed_sec >= 0x180) {
                        step++;
                    }
                    ww(0, 0x82 * 4, step);
                    ww(0, 0x82 * 4 + 2, 0);
                }
                return;
            case 0x81:
                /* Gesicherter Timer-Vektor fuer AIL in INTRO.EXE: im CD-Modus No-Op */
                return;
            case 0xfd: {
                uint16_t stueck = rw(s[2], (uint16_t)(r[4] + 2));
                trackStartBefehle = befehle;
                aktuellerTrack = (int)stueck + 2;
                dsa_platform_play_cd_track(aktuellerTrack);
                push(r[5]);
                r[5] = r[4];
                ip = 0x72;
                return;
            }
            case 0xfe:
                if (++durchlaeufe > 1) {
                    beendet = true;
                    return;
                }
                push((uint16_t)((ip + 1) & 0xffff));
                ip = 0x3459;
                return;
            case 0x1c:
            case 0x1b:
            case 0x23:
            case 0x24:
                return;
            case 0x11: r[0] = rw(0x40, 0x10); return;
            case 0x12: r[0] = 640; return;
            case 0x15: cf(true); set8(4, 0x86); return;
            default:
                if (n >= 0x08 && n <= 0x0f) {
                    portSchreiben(0x20, 0x20);
                    return;
                }
                return;
        }
    }

    void video(int ah, int al) {
        switch (ah) {
            case 0x00:
                wb(0x40, 0x49, (uint8_t)(al & 0x7f));
                if ((al & 0x7f) == 0x13) {
                    if (!(al & 0x80)) std::memset(&m[0xa0000], 0, 0x10000);
                    ww(0x40, 0x4a, 40);
                } else {
                    ww(0x40, 0x4a, 80);
                }
                return;
            case 0x0f:
                r[0] = (uint16_t)((rb(0x40, 0x4a) << 8) | rb(0x40, 0x49));
                set8(7, 0);
                return;
            case 0x10:
                if (al == 0x10) {
                    int i = r[3] & 0xff;
                    palette[i * 3] = (uint8_t)((r[2] >> 8) & 0x3f);
                    palette[i * 3 + 1] = (uint8_t)((r[1] >> 8) & 0x3f);
                    palette[i * 3 + 2] = (uint8_t)(r[1] & 0x3f);
                } else if (al == 0x12) {
                    for (int i = 0; i < (int)r[1] * 3; ++i) {
                        palette[(r[3] * 3 + i) % 768] = (uint8_t)(rb(s[0], (uint16_t)(r[2] + i)) & 0x3f);
                    }
                } else if (al == 0x15) {
                    int i = r[3] & 0xff;
                    r[2] = (uint16_t)(palette[i * 3] << 8);
                    r[1] = (uint16_t)((palette[i * 3 + 1] << 8) | palette[i * 3 + 2]);
                } else if (al == 0x17) {
                    for (int i = 0; i < (int)r[1] * 3; ++i) {
                        wb(s[0], (uint16_t)(r[2] + i), palette[(r[3] * 3 + i) % 768]);
                    }
                }
                return;
            case 0x12:
                if ((r[3] & 0xff) == 0x10) { r[3] = 0x0003; r[1] = 0x0009; }
                return;
            case 0x1a:
                if (al == 0) { r[0] = (uint16_t)((r[0] & 0xff00) | 0x1a); r[3] = 0x0008; }
                return;
            case 0x4f:
                r[0] = 0x0100;
                return;
            default:
                return;
        }
    }

    template <typename CfFunc>
    void dos(int ah, int al, CfFunc cf) {
        auto text = [&](uint16_t seg, uint16_t off) -> std::string {
            std::string t;
            for (int i = 0; i < 128; ++i) {
                uint8_t c = rb(seg, (uint16_t)(off + i));
                if (!c) break;
                t.push_back((char)c);
            }
            return t;
        };
        cf(false);
        switch (ah) {
            case 0x30: r[0] = 0x0005; r[3] = 0; r[1] = 0; return;
            case 0x25: ww(0, (uint16_t)(al * 4), r[2]); ww(0, (uint16_t)(al * 4 + 2), s[3]); return;
            case 0x35: r[3] = rw(0, (uint16_t)(al * 4)); s[0] = rw(0, (uint16_t)(al * 4 + 2)); return;
            case 0x4a: {
                uint32_t neu = (uint32_t)s[0] + (uint32_t)r[3];
                if (neu > 0x9fff) { r[3] = (uint16_t)(0x9fff - s[0]); r[0] = 8; cf(true); return; }
                if (s[0] == psp) speicherFrei = (uint16_t)neu;
                return;
            }
            case 0x48: {
                uint16_t frei = (uint16_t)(0x9fff - speicherFrei);
                if (r[3] > frei) { r[3] = frei; r[0] = 8; cf(true); return; }
                r[0] = speicherFrei;
                speicherFrei = (uint16_t)(speicherFrei + r[3]);
                return;
            }
            case 0x49: return;
            case 0x3d: {
                std::string name = text(s[3], r[2]);
                const std::vector<uint8_t> *d = datei(name);
                if (!d) { r[0] = 2; cf(true); return; }
                int h = naechstesHandle++;
                handles[h] = HandleEntry{d, 0};
                r[0] = (uint16_t)h;
                return;
            }
            case 0x3c: r[0] = 5; cf(true); return;
            case 0x3e: handles.erase(r[3]); return;
            case 0x3f: {
                auto it = handles.find(r[3]);
                if (it == handles.end()) {
                    if (r[3] == 0) { r[0] = 0; return; }
                    r[0] = 6; cf(true); return;
                }
                HandleEntry &f = it->second;
                size_t avail = (f.pos < f.d->size()) ? (f.d->size() - f.pos) : 0;
                size_t n = std::min((size_t)r[1], avail);
                for (size_t i = 0; i < n; ++i) {
                    wb(s[3], (uint16_t)(r[2] + i), (*f.d)[f.pos + i]);
                }
                f.pos += n;
                r[0] = (uint16_t)n;
                return;
            }
            case 0x40:
                r[0] = r[1];
                return;
            case 0x42: {
                auto it = handles.find(r[3]);
                if (it == handles.end()) { r[0] = 6; cf(true); return; }
                HandleEntry &f = it->second;
                uint32_t uoff = ((uint32_t)r[1] << 16) | r[2];
                int32_t soff = (int32_t)uoff;
                int64_t np = (al == 0) ? (int64_t)uoff : ((al == 1) ? ((int64_t)f.pos + soff) : ((int64_t)f.d->size() + soff));
                if (np < 0) np = 0;
                f.pos = (size_t)np;
                r[0] = (uint16_t)(f.pos & 0xffff);
                r[2] = (uint16_t)((f.pos >> 16) & 0xffff);
                return;
            }
            case 0x44:
                if (al == 0) { r[2] = (r[3] <= 2) ? 0x80d3 : 0x0002; }
                return;
            case 0x4c: beendet = true; return;
            case 0x19: set8(0, 2); return;
            case 0x0e: set8(0, 26); return;
            case 0x1a: hasDta = true; dtaSeg = s[3]; dtaOff = r[2]; return;
            case 0x2f: s[0] = hasDta ? dtaSeg : psp; r[3] = hasDta ? dtaOff : 0x80; return;
            case 0x2a: r[1] = 1992; r[2] = 0x0301; set8(0, 0); return;
            case 0x2c: {
                int64_t t = (befehle * 100) / CPU_HZ;
                r[1] = 0;
                r[2] = (uint16_t)((((t / 100) % 60) << 8) | (t % 100));
                return;
            }
            case 0x47: wb(s[3], r[6], 0); return;
            case 0x0b: set8(0, tasten.empty() ? 0 : 0xff); return;
            case 0x4e:
            case 0x4f: r[0] = 18; cf(true); return;
            case 0x29: return;
            case 0x34: s[0] = 0x50; r[3] = 0x10; return;
        }
    }

    void schritt() {
        segUeber = -1;
        int rep = 0;
        uint16_t start = ip;
        uint8_t op = holB();
        for (;;) {
            if (op == 0x26 || op == 0x2e || op == 0x36 || op == 0x3e) {
                segUeber = (op >> 3) & 3;
                op = holB();
            } else if (op == 0xf2 || op == 0xf3) {
                rep = op;
                op = holB();
            } else if (op == 0xf0) {
                op = holB();
            } else {
                break;
            }
        }
        if (op < 0x40 && (op & 7) < 6) {
            int aluOp = op >> 3;
            int art = op & 7;
            int w = art & 1;
            switch (art) {
                case 0:
                case 1: {
                    int reg = modrm();
                    int a = w ? rmW() : rmB();
                    int b = w ? r[reg] : get8(reg);
                    uint16_t v = alu(aluOp, a, b, w);
                    if (aluOp != 7) {
                        if (w) setRmW(v);
                        else setRmB(v);
                    }
                    return;
                }
                case 2:
                case 3: {
                    int reg = modrm();
                    int a = w ? r[reg] : get8(reg);
                    int b = w ? rmW() : rmB();
                    uint16_t v = alu(aluOp, a, b, w);
                    if (aluOp != 7) {
                        if (w) r[reg] = v;
                        else set8(reg, v);
                    }
                    return;
                }
                case 4: {
                    uint16_t v = alu(aluOp, r[0] & 0xff, holB(), 0);
                    if (aluOp != 7) set8(0, v);
                    return;
                }
                case 5: {
                    uint16_t v = alu(aluOp, r[0], holW(), 1);
                    if (aluOp != 7) r[0] = v;
                    return;
                }
            }
        }
        switch (op) {
            case 0x06: push(s[0]); return;
            case 0x07: s[0] = pop(); return;
            case 0x0e: push(s[1]); return;
            case 0x16: push(s[2]); return;
            case 0x17: s[2] = pop(); return;
            case 0x1e: push(s[3]); return;
            case 0x1f: s[3] = pop(); return;
            case 0x27:
            case 0x2f: {
                int al = r[0] & 0xff;
                int cf = flag(0), af = flag(4);
                if ((al & 0xf) > 9 || af) {
                    al = (op == 0x27) ? (al + 6) : (al - 6);
                    setFlag(4, true);
                } else setFlag(4, false);
                if ((r[0] & 0xff) > 0x99 || cf) {
                    al = (op == 0x27) ? (al + 0x60) : (al - 0x60);
                    setFlag(0, true);
                } else setFlag(0, false);
                set8(0, al);
                szp(al & 0xff, 0);
                return;
            }
            case 0x37:
            case 0x3f: {
                if ((r[0] & 0xf) > 9 || flag(4)) {
                    int al = (op == 0x37) ? ((r[0] & 0xff) + 6) : ((r[0] & 0xff) - 6);
                    int ah = ((r[0] >> 8) + ((op == 0x37) ? 1 : -1)) & 0xff;
                    r[0] = (uint16_t)((ah << 8) | (al & 0xf));
                    setFlag(4, true);
                    setFlag(0, true);
                } else {
                    r[0] &= 0xff0f;
                    setFlag(4, false);
                    setFlag(0, false);
                }
                return;
            }
        }
        if (op >= 0x40 && op < 0x50) { int i = op & 7; r[i] = incdec(r[i], 1, op >= 0x48); return; }
        if (op >= 0x50 && op < 0x58) { push(op == 0x54 ? r[4] : r[op & 7]); return; }
        if (op >= 0x58 && op < 0x60) { r[op & 7] = pop(); return; }
        if (op >= 0x70 && op < 0x80) {
            int d = holB();
            if (bed(op & 15)) {
                if (d >= 128) warteschleife(start);
                ip = (uint16_t)((ip + (d < 128 ? d : (d - 256))) & 0xffff);
            }
            return;
        }
        if (op >= 0x91 && op < 0x98) { int i = op & 7; uint16_t t = r[0]; r[0] = r[i]; r[i] = t; return; }
        if (op >= 0xb0 && op < 0xb8) { set8(op & 7, holB()); return; }
        if (op >= 0xb8 && op < 0xc0) { r[op & 7] = holW(); return; }
        switch (op) {
            case 0x60: { uint16_t sp = r[4]; for (int i = 0; i < 8; ++i) push(i == 4 ? sp : r[i]); return; }
            case 0x61: { for (int i = 7; i >= 0; --i) { uint16_t v = pop(); if (i != 4) r[i] = v; } return; }
            case 0x62: modrm(); return;
            case 0x68: push(holW()); return;
            case 0x6a: { int v = holB(); push((uint16_t)(v < 128 ? v : (v | 0xff00))); return; }
            case 0x69:
            case 0x6b: {
                int reg = modrm();
                int32_t a = vz(rmW(), 1);
                int32_t b = (op == 0x69) ? vz(holW(), 1) : vz(holB(), 0);
                int32_t p = a * b;
                r[reg] = (uint16_t)(p & 0xffff);
                bool ueber = (p < -32768 || p > 32767);
                setFlag(0, ueber);
                setFlag(11, ueber);
                return;
            }
            case 0x6c:
            case 0x6d:
            case 0x6e:
            case 0x6f:
                stringOp(op, rep);
                return;
            case 0x80:
            case 0x81:
            case 0x82:
            case 0x83: {
                int reg = modrm();
                int w = op & 1;
                int a = w ? rmW() : rmB();
                int b = (op == 0x81) ? holW() : holB();
                if (op == 0x83 && (b & 0x80)) b |= 0xff00;
                uint16_t v = alu(reg, a, b, w);
                if (reg != 7) {
                    if (w) setRmW(v);
                    else setRmB(v);
                }
                return;
            }
            case 0x84: { int reg = modrm(); alu(4, rmB(), get8(reg), 0); return; }
            case 0x85: { int reg = modrm(); alu(4, rmW(), r[reg], 1); return; }
            case 0x86: { int reg = modrm(); uint8_t t = rmB(); setRmB(get8(reg)); set8(reg, t); return; }
            case 0x87: { int reg = modrm(); uint16_t t = rmW(); setRmW(r[reg]); r[reg] = t; return; }
            case 0x88: { int reg = modrm(); setRmB(get8(reg)); return; }
            case 0x89: { int reg = modrm(); setRmW(r[reg]); return; }
            case 0x8a: { int reg = modrm(); set8(reg, rmB()); return; }
            case 0x8b: { int reg = modrm(); r[reg] = rmW(); return; }
            case 0x8c: { int reg = modrm(); setRmW(s[reg & 3]); return; }
            case 0x8d: { int reg = modrm(); r[reg] = eaOff; return; }
            case 0x8e: { int reg = modrm(); s[reg & 3] = rmW(); return; }
            case 0x8f: { modrm(); setRmW(pop()); return; }
            case 0x90: return;
            case 0x98: r[0] = (uint16_t)((r[0] & 0x80) ? (r[0] | 0xff00) : (r[0] & 0xff)); return;
            case 0x99: r[2] = (uint16_t)((r[0] & 0x8000) ? 0xffff : 0); return;
            case 0x9a: { uint16_t o = holW(), sg = holW(); push(s[1]); push(ip); s[1] = sg; ip = o; return; }
            case 0x9b: return;
            case 0x9c: push((uint16_t)(fl | 0xf002)); return;
            case 0x9d: fl = (uint16_t)((pop() & 0x0fd5) | 0x0002); return;
            case 0x9e: fl = (uint16_t)((fl & 0xff00) | ((r[0] >> 8) & 0xd5) | 2); return;
            case 0x9f: set8(4, fl & 0xff); return;
            case 0xa0: set8(0, rb(datSeg(), holW())); return;
            case 0xa1: r[0] = rw(datSeg(), holW()); return;
            case 0xa2: wb(datSeg(), holW(), (uint8_t)(r[0] & 0xff)); return;
            case 0xa3: ww(datSeg(), holW(), r[0]); return;
            case 0xa4:
            case 0xa5:
            case 0xa6:
            case 0xa7:
            case 0xaa:
            case 0xab:
            case 0xac:
            case 0xad:
            case 0xae:
            case 0xaf:
                stringOp(op, rep);
                return;
            case 0xa8: alu(4, r[0] & 0xff, holB(), 0); return;
            case 0xa9: alu(4, r[0], holW(), 1); return;
            case 0xc0:
            case 0xc1:
            case 0xd0:
            case 0xd1:
            case 0xd2:
            case 0xd3: {
                int reg = modrm();
                int w = op & 1;
                int n = (op < 0xc2) ? holB() : ((op < 0xd2) ? 1 : (r[1] & 0xff));
                uint16_t v = schieben(reg, w ? rmW() : rmB(), n, w);
                if (w) setRmW(v);
                else setRmB(v);
                return;
            }
            case 0xc2: { uint16_t n = holW(); ip = pop(); r[4] = (uint16_t)((r[4] + n) & 0xffff); return; }
            case 0xc3: ip = pop(); return;
            case 0xc4:
            case 0xc5: {
                int reg = modrm();
                r[reg] = rw(eaSeg, eaOff);
                s[op == 0xc4 ? 0 : 3] = rw(eaSeg, (uint16_t)(eaOff + 2));
                return;
            }
            case 0xc6: modrm(); setRmB(holB()); return;
            case 0xc7: modrm(); setRmW(holW()); return;
            case 0xc8: {
                uint16_t groesse = holW();
                int tiefe = holB() & 31;
                push(r[5]);
                uint16_t rahmen = r[4];
                for (int i = 1; i < tiefe; ++i) {
                    r[5] = (uint16_t)((r[5] - 2) & 0xffff);
                    push(rw(s[2], r[5]));
                }
                if (tiefe) push(rahmen);
                r[5] = rahmen;
                r[4] = (uint16_t)((r[4] - groesse) & 0xffff);
                return;
            }
            case 0xc9: r[4] = r[5]; r[5] = pop(); return;
            case 0xca: { uint16_t n = holW(); ip = pop(); s[1] = pop(); r[4] = (uint16_t)((r[4] + n) & 0xffff); return; }
            case 0xcb: ip = pop(); s[1] = pop(); return;
            case 0xcc: interrupt(3); return;
            case 0xcd: interrupt(holB()); return;
            case 0xce: if (flag(11)) interrupt(4); return;
            case 0xcf: ip = pop(); s[1] = pop(); fl = (uint16_t)((pop() & 0x0fd5) | 2); return;
            case 0xd4: {
                int n = holB();
                if (!n) n = 10;
                int al = r[0] & 0xff;
                r[0] = (uint16_t)((((al / n) & 0xff) << 8) | (al % n));
                szp(r[0] & 0xff, 0);
                return;
            }
            case 0xd5: {
                int n = holB();
                int al = ((r[0] & 0xff) + (r[0] >> 8) * n) & 0xff;
                r[0] = (uint16_t)al;
                szp(al, 0);
                return;
            }
            case 0xd6: set8(0, flag(0) ? 0xff : 0); return;
            case 0xd7: set8(0, rb(datSeg(), (uint16_t)((r[3] + (r[0] & 0xff)) & 0xffff))); return;
            case 0xe0:
            case 0xe1:
            case 0xe2: {
                int d = holB();
                r[1] = (uint16_t)((r[1] - 1) & 0xffff);
                bool z = (flag(6) != 0);
                if (r[1] != 0 && (op == 0xe2 || (op == 0xe1 ? z : !z))) {
                    ip = (uint16_t)((ip + (d < 128 ? d : (d - 256))) & 0xffff);
                }
                return;
            }
            case 0xe3: {
                int d = holB();
                if (r[1] == 0) ip = (uint16_t)((ip + (d < 128 ? d : (d - 256))) & 0xffff);
                return;
            }
            case 0xe4: set8(0, portLesen(holB(), 0)); return;
            case 0xe5: r[0] = portLesen(holB(), 1); return;
            case 0xe6: portSchreiben(holB(), (uint8_t)(r[0] & 0xff)); return;
            case 0xe7: { uint8_t p = holB(); portSchreiben(p, (uint8_t)(r[0] & 0xff)); portSchreiben((uint16_t)(p + 1), (uint8_t)(r[0] >> 8)); return; }
            case 0xe8: { uint16_t d = holW(); push(ip); ip = (uint16_t)((ip + d) & 0xffff); return; }
            case 0xe9: { uint16_t d = holW(); ip = (uint16_t)((ip + d) & 0xffff); return; }
            case 0xea: { uint16_t o = holW(), sg = holW(); ip = o; s[1] = sg; return; }
            case 0xeb: {
                int d = holB();
                if (d >= 128) warteschleife(start);
                ip = (uint16_t)((ip + (d < 128 ? d : (d - 256))) & 0xffff);
                return;
            }
            case 0xec: set8(0, portLesen(r[2], 0)); return;
            case 0xed: r[0] = portLesen(r[2], 1); return;
            case 0xee: portSchreiben(r[2], (uint8_t)(r[0] & 0xff)); return;
            case 0xef: portSchreiben(r[2], (uint8_t)(r[0] & 0xff)); portSchreiben((uint16_t)(r[2] + 1), (uint8_t)(r[0] >> 8)); return;
            case 0xf1: { uint8_t n = holB(); dienst(n); return; }
            case 0xf4: angehalten = true; return;
            case 0xf5: setFlag(0, !flag(0)); return;
            case 0xf6:
            case 0xf7: gruppe3(op & 1); return;
            case 0xf8: setFlag(0, false); return;
            case 0xf9: setFlag(0, true); return;
            case 0xfa: setFlag(9, false); return;
            case 0xfb: setFlag(9, true); return;
            case 0xfc: setFlag(10, false); return;
            case 0xfd: setFlag(10, true); return;
            case 0xfe: {
                int reg = modrm();
                if (reg > 1) { beendet = true; fehler = true; return; }
                setRmB(incdec(rmB(), 0, reg == 1));
                return;
            }
            case 0xff: {
                int reg = modrm();
                switch (reg) {
                    case 0:
                    case 1: setRmW(incdec(rmW(), 1, reg == 1)); return;
                    case 2: { uint16_t z = rmW(); push(ip); ip = z; return; }
                    case 3: { uint16_t o = rw(eaSeg, eaOff), sg = rw(eaSeg, (uint16_t)(eaOff + 2)); push(s[1]); push(ip); ip = o; s[1] = sg; return; }
                    case 4: ip = rmW(); return;
                    case 5: ip = rw(eaSeg, eaOff); s[1] = rw(eaSeg, (uint16_t)(eaOff + 2)); return;
                    case 6: push(rmW()); return;
                }
                beendet = true;
                fehler = true;
                return;
            }
        }
        if (op >= 0xd8 && op <= 0xdf) { modrm(); return; }
        beendet = true;
        fehler = true;
    }
};

static bool load_intro_file_ci(const char *filename, std::vector<uint8_t> &out) {
    const char *gd = dsa_platform_get_game_dir();
    std::string candidates[4];
    candidates[0] = filename;
    if (gd && gd[0]) {
        candidates[1] = std::string(gd) + "/" + filename;
        candidates[2] = std::string(gd) + "\\" + filename;
    }
    for (const std::string &path : candidates) {
        if (path.empty()) continue;
        FILE *fp = std::fopen(path.c_str(), "rb");
        if (fp) {
            std::fseek(fp, 0, SEEK_END);
            long sz = std::ftell(fp);
            std::fseek(fp, 0, SEEK_SET);
            if (sz > 0) {
                out.resize((size_t)sz);
                size_t rd = std::fread(out.data(), 1, (size_t)sz, fp);
                std::fclose(fp);
                return rd == (size_t)sz;
            }
            std::fclose(fp);
        }
    }
    return false;
}

} // namespace

extern "C" int dsa_run_original_intro(void) {
    IntroVm vm;
    const char *req_files[4] = {"INTRO_E.OVL", "INTRODAT.DIR", "INTRODAT.VOL", "INTRODAT.SND"};
    for (int i = 0; i < 4; ++i) {
        std::vector<uint8_t> buf;
        if (!load_intro_file_ci(req_files[i], buf)) {
            return 0;
        }
        vm.dateien[req_files[i]] = std::move(buf);
    }

    if (!vm.init()) {
        return 0;
    }

    while (dsa_platform_key_ready()) {
        dsa_platform_key_read();
    }
    g_mouse1_event2 = 0;
    g_mouse_leftclick_event = 0;
    g_mouse_rightclick_event = 0;

    int64_t ziel = 0;
    const int64_t step_cycles = CPU_HZ / VSYNC_HZ; // ~357142 Befehle pro 70-Hz-Frame

    while (!vm.beendet) {
        ziel += step_cycles;
        vm.laufen(ziel);

        unsigned char *vram = dsa_platform_get_vram();
        if (vram) {
            std::memcpy(vram, &vm.m[0xa0000], 320 * 200);
        }
        if (g_vga_memstart && g_vga_memstart != vram) {
            std::memcpy(g_vga_memstart, &vm.m[0xa0000], 320 * 200);
        }
        dsa_platform_set_palette_range(0, 256, vm.palette);
        dsa_platform_present();
        wait_for_vsync();

        if (dsa_platform_key_ready()) {
            dsa_platform_key_read();
            break;
        }
        if (g_mouse1_event2 || g_mouse_leftclick_event || g_mouse_rightclick_event) {
            break;
        }
    }

    dsa_platform_stop_cd_audio();

    while (dsa_platform_key_ready()) {
        dsa_platform_key_read();
    }
    g_mouse1_event2 = 0;
    g_mouse_leftclick_event = 0;
    g_mouse_rightclick_event = 0;

    if (g_vga_memstart) {
        std::memset(g_vga_memstart, 0, 320 * 200);
    }
    unsigned char *vram = dsa_platform_get_vram();
    if (vram) {
        std::memset(vram, 0, 320 * 200);
    }
    uint8_t black_pal[768];
    std::memset(black_pal, 0, sizeof(black_pal));
    black_pal[0xff * 3 + 0] = 0x3f;
    black_pal[0xff * 3 + 1] = 0x3f;
    black_pal[0xff * 3 + 2] = 0x3f;
    dsa_platform_set_palette_range(0, 256, black_pal);
    dsa_platform_present();

    return 1;
}
