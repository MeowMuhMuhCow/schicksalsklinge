#!/usr/bin/env python3
"""
prepare_sources.py - Bereitet Port/src/ aus Quelle/ dieser Fassung (MooMoo) vor.

Grundsatz (Nachtrag 2):
- Quelle/ bleibt 100 % unberuehrt.
- Die gesamte Spiellogik und alle 284 Fehlerbehebungen (s_fix.h: FIX_BUGS=0/1/2,
  FIX_EXPLOITS, FIX_nnnn, FIX_PROBENLOG, FIX_HIMMEL, FIX_SPEICHERN) stammen
  ausschliesslich aus Quelle/ dieser Fassung (MooMoo).
- Aus BrightEyes-SDL2 / MobileEyes (GPL v3) werden nur die Plattform-Ersatzteile
  (seg008..011 in C, SDL2-Eingabe/Timer-Schleife, natives Speichern/Laden ueber
  datseg_status.h, 32/64-Bit-Anpassungen) in die #if !defined(__BORLANDC__)-Zweige
  eingesetzt.
"""

from __future__ import annotations

import os
import re
import shutil
import sys
from pathlib import Path

PORT_DIR = Path(__file__).resolve().parent.parent
ROOT_DIR = PORT_DIR.parent
QUELLE_DIR = ROOT_DIR / "Quelle"
BE_DOSBOX_DIR = QUELLE_DIR / "BrightEyes_DOSBox"
FINAL_SRC_DIR = PORT_DIR / "src"
SRC_DIR = Path("/tmp/schick_src_stage") if os.name != "nt" else FINAL_SRC_DIR


def replace_once(text: str, old: str, new: str, label: str) -> str:
    if old not in text:
        raise RuntimeError(f"[{label}] Target string not found:\n{old[:200]}")
    return text.replace(old, new, 1)


def generate_datseg_status_h(datseg_c_path: Path, datseg_h_path: Path, out_path: Path) -> None:
    """Erzeugt Port/src/datseg_status.h aus Port/src/datseg.c (ds:0x2d34..0x4474, 5952 Bytes)."""
    start_addr, end_addr = 0x2D34, 0x4474
    volatile_vars = {"gs_harbor_options"}
    lines = datseg_c_path.read_text(encoding="latin-1").split("\n")
    decl_re = re.compile(
        r"^\s*(?:static\s+)?((?:const\s+)?(?:unsigned\s+|signed\s+)?(?:struct\s+)?\w+)"
        r"\s*(\*?)\s*(\w+)\s*((?:\[[^\]]*\])*)\s*(=|;)"
    )

    entries: dict[int, tuple[str, bool, str]] = {}
    stack: list[bool] = []
    for i, line in enumerate(lines):
        s = line.strip()
        if s.startswith("#if"):
            stack.append("BORLANDC" in s and "!" not in s)
            continue
        if s.startswith("#else"):
            stack[-1] = not stack[-1]
            continue
        if s.startswith("#endif"):
            stack.pop()
            continue
        if any(stack):
            continue
        m = re.search(r"ds:(0x[0-9a-fA-F]+)", line)
        if not m:
            continue
        addr = int(m.group(1), 16)
        if not (start_addr <= addr < end_addr):
            continue
        if "start of the BSS" in "\n".join(lines[:i]):
            break
        j = i
        d = decl_re.match(line.split("//")[0])
        while not d and j > 0:
            j -= 1
            d = decl_re.match(lines[j].split("//")[0])
        if not d:
            raise RuntimeError(f"Could not parse declaration at datseg.c:{i+1}: {line}")
        typ, ptr, name, arr = d.group(1), d.group(2), d.group(3), d.group(4)
        if addr in entries:
            raise RuntimeError(f"Duplicate address {addr:#x}: {entries[addr][0]} and {name}")
        entries[addr] = (name, bool(ptr), f"{typ} {ptr}{name}{arr}")

    addrs = sorted(entries)
    hdr_text = datseg_h_path.read_text(encoding="latin-1")
    missing = [
        entries[a][2]
        for a in addrs
        if not re.search(r"extern[^;]*\b" + entries[a][0] + r"\b", hdr_text)
    ]

    out_lines = [
        "/* Automatisch erzeugt von Port/tools/prepare_sources.py aus Quelle/datseg.c.",
        " * Spielzustand im DOS-Datensegment (ds:0x2d34-0x4474, 5952 Bytes):",
        " *   STATUS_VAR(offset, laenge, variable)   Daten, laenge = Platz unter DOS",
        " *   STATUS_PTR(offset, laenge, variable)   Zeiger (unter DOS 4 Byte), wird umgerechnet",
        " *   STATUS_DUMMY(offset, laenge, variable) Platzhalter fuer nativ ausgelagerte Zeiger",
        " *   STATUS_SKIP(offset, laenge, variable)  enthaelt Zeiger, wird neu aufgebaut */",
        f"#define STATUS_DOS_LENGTH {end_addr - start_addr}",
        "#ifndef STATUS_DECLARED",
        "#define STATUS_DECLARED",
        *[f"extern {d};" for d in missing],
        "#endif",
    ]
    for k, addr in enumerate(addrs):
        name, is_ptr, _ = entries[addr]
        length = (addrs[k + 1] if k + 1 < len(addrs) else end_addr) - addr
        kind = (
            "STATUS_PTR"
            if is_ptr
            else "STATUS_SKIP"
            if name in volatile_vars
            else "STATUS_DUMMY"
            if name.endswith("_obsolete")
            else "STATUS_VAR"
        )
        out_lines.append(f"{kind}(0x{addr - start_addr:04x}, {length}, {name})")
    out_path.write_text("\n".join(out_lines) + "\n", encoding="latin-1")
    print(f"      datseg_status.h erzeugt ({len(addrs)} Eintraege, {end_addr - start_addr} Bytes).")


def patch_platform_and_headers() -> None:
    # 1. platform.h
    platform_h = """#ifndef PLATFORM_H
#define PLATFORM_H

#if defined(__BORLANDC__)
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned long uint32_t;
typedef signed long int32_t;
typedef uint8_t huge * HugePt;
#define STATIC_ASSERT(expr, msg) typedef char static_assert_##msg[(expr) ? 1 : -1]
#else
#include <stdint.h>
#include <assert.h>
typedef uint8_t * HugePt;
#if defined(__cplusplus)
#define STATIC_ASSERT(expr, msg) static_assert(expr, #msg)
#else
#define STATIC_ASSERT(expr, msg) typedef char static_assert_##msg[(expr) ? 1 : -1]
#endif
#endif

STATIC_ASSERT(sizeof(int8_t) == 1, int8_needs_to_be_1_byte);
STATIC_ASSERT(sizeof(int16_t) == 2, int16_needs_to_be_2_bytes);
STATIC_ASSERT(sizeof(int32_t) == 4, int32_needs_to_be_4_bytes);

#endif /* PLATFORM_H */
"""
    (SRC_DIR / "platform.h").write_text(platform_h, encoding="latin-1")

    # 2. hero.h
    hero_p = SRC_DIR / "hero.h"
    hero_txt = hero_p.read_text(encoding="latin-1")
    hero_txt = replace_once(
        hero_txt,
        """#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
  #include <assert.h>
#define STATIC_ASSERT(expr, msg) static_assert(expr, #msg)
#else
  #define STATIC_ASSERT(expr, msg) typedef char static_assert_##msg[(expr) ? 1 : -1]
#endif

#if defined(__BORLANDC__) || defined(__WATCOMC__)
typedef signed long int32_t;
#else
typedef signed int int32_t;
#endif

STATIC_ASSERT(sizeof(int32_t) == 4, int32_needs_to_be_4_bytes);""",
        '#include "platform.h"',
        "hero.h",
    )
    hero_p.write_text(hero_txt, encoding="latin-1")

    # 3. v302de.h
    v302_p = SRC_DIR / "v302de.h"
    v302_txt = v302_p.read_text(encoding="latin-1")
    # Wichtig (Nachtrag 2): M302de_ORIGINAL_BUGFIX und M302de_FEATURE_MOD NICHT aktivieren,
    # damit ausschliesslich s_fix.h dieser Fassung (FIX_BUGS=0/1/2, FIX_nnnn) das Verhalten steuert!
    v302_txt = replace_once(
        v302_txt,
        """/* comment this out to have the original, but buggy behaviour */
#define M302de_ORIGINAL_BUGFIX

/* BrightEyes implements a few (moderate) feature extensions/mods.
 * comment this out to avoid the new features */
#define M302de_FEATURE_MOD""",
        """/* Im Port dieser Fassung (MooMoo) steuert ausschliesslich s_fix.h (FIX_BUGS / FIX_nnnn)
 * die Fehlerbehebungen, damit FIX_BUGS=0 exakt das Originalverhalten ergibt. */
/* #define M302de_ORIGINAL_BUGFIX */
/* #define M302de_FEATURE_MOD */""",
        "v302de.h:M302de_macros",
    )
    v302_txt = replace_once(
        v302_txt,
        """typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
#if defined(__BORLANDC__)
typedef unsigned long uint32_t;
typedef signed long int32_t;
typedef uint8_t huge * HugePt;
#else
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef uint8_t* HugePt;
#endif""",
        '#include "platform.h"',
        "v302de.h:platform_h",
    )
    v302_txt = replace_once(
        v302_txt,
        '#include "common.h"\n#include "datseg.h"',
        '#if !defined(__BORLANDC__)\n#include "vgalib.h"\n#endif\n\n#include "common.h"\n#include "datseg.h"',
        "v302de.h:vgalib_h",
    )
    v302_txt = replace_once(
        v302_txt,
        """#define _open open
#define _creat creat
#define _read read

static inline int bioskey(const int cmd) { return 0; }""",
        """#include <fcntl.h>
#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif
#ifndef O_BINARY
#define O_BINARY 0
#endif

static inline int schick_creat_binary(const char *fname, int attr)
{
	(void)attr;
	return open(fname, O_BINARY | O_CREAT | O_TRUNC | O_WRONLY, 0666);
}

static inline int schick_open_binary(const char *fname, int mode)
{
	return open(fname, mode | O_BINARY);
}

#define _open schick_open_binary
#define _creat schick_creat_binary
#define _read read

extern signed int bioskey(const signed int cmd);
static inline signed int CD_bioskey(const signed int cmd) { return bioskey(cmd); }""",
        "v302de.h:bioskey_and_open",
    )
    v302_txt = replace_once(
        v302_txt,
        """static inline void update_informer_cond(const int informer)
{
	if ((0 <= informer) && (informer < 15) && (gs_informer_flags[informer] == 0)) {
		gs_informer_flags[informer] = 1;
	}
}""",
        """static inline void update_informer_cond(const int informer)
{
#if FIX_0076
	if ((0 <= informer) && (informer < 15) && (gs_informer_flags[informer] == 0)) {
		gs_informer_flags[informer] = 1;
	}
#else
	if ((0 <= informer) && (informer < 15)) {
		gs_informer_flags[informer] = 1;
	}
#endif
}""",
        "v302de.h:update_informer_cond",
    )
    v302_txt = replace_once(
        v302_txt,
        """static inline signed char get_cb_val(const signed short x, const signed short y) {
	return *(g_chessboard + 25 * y + x);
}""",
        """static inline signed char get_cb_val(const signed short x, const signed short y) {
	if ((x < 0) || (x >= 25) || (y < 0) || (y >= 25)) {
		return 0;
	}
	return *(g_chessboard + 25 * y + x);
}
#if !defined(_WIN32)
#define itoa my_itoa
#endif""",
        "v302de.h:get_cb_val",
    )
    v302_txt = replace_once(
        v302_txt,
        "static inline void D1_LOG(...) { }",
        'static inline void D1_LOG(const char *fmt, ...) { (void)fmt; }\n#include "dsa_platform.h"',
        "v302de.h:D1_LOG",
    )
    v302_p.write_text(v302_txt, encoding="latin-1")

    # 4. datseg.h
    ds_h_p = SRC_DIR / "datseg.h"
    ds_h_txt = ds_h_p.read_text(encoding="latin-1")
    ds_h_txt = replace_once(
        ds_h_txt,
        "extern unsigned char g_herokeeping_flag;\t//ds:0x4649; seg002",
        "extern unsigned char g_herokeeping_flag;\t//ds:0x4649; seg002\nextern int32_t g_level_ap_tab[21];\t\t//ds:0x464e; seg002",
        "datseg.h:g_level_ap_tab",
    )
    ds_h_txt = replace_once(
        ds_h_txt,
        "extern const char* g_fnames_v302de[];\t\t//ds:0x4c8c; seg002, seg026",
        "extern const char* g_fnames_v302de[];\t\t//ds:0x4c8c; seg002, seg026\nextern const char g_chr_file_suffix[];\t\t//ds:0x5e3e; seg026, seg120",
        "datseg.h:g_chr_file_suffix",
    )
    ds_h_txt = replace_once(
        ds_h_txt,
        "extern const uint8_t g_palette_fight2[20][3];\t\t\t//ds:0x7d0e; seg032, seg074",
        "extern const uint8_t g_palette_fight2[][3];\t\t\t//ds:0x7d0e; seg032, seg074",
        "datseg.h:g_palette_fight2",
    )
    ds_h_txt = replace_once(
        ds_h_txt,
        "extern uint8_t g_color_black[3];\t\t\t// ds:0xb22d; seg029, seg120",
        "extern uint8_t g_color_black[3];\t\t\t// ds:0xb22d; seg029, seg120\nextern const char g_str_temp_dir[];\t\t\t// ds:0xb311;",
        "datseg.h:g_str_temp_dir",
    )
    ds_h_txt = replace_once(
        ds_h_txt,
        "extern int8_t g_fig_move_pathdir[10];\t// ds:0xd823; seg034, seg036, seg037, seg038",
        "#if defined(__BORLANDC__)\nextern int8_t g_fig_move_pathdir[10];\t// ds:0xd823; seg034, seg036, seg037, seg038\n#else\nextern int8_t g_fig_move_pathdir[20];\t// ds:0xd823; seg034, seg036, seg037, seg038\n#endif\nextern uint8_t *g_buffer_xx;",
        "datseg.h:g_fig_move_pathdir",
    )
    ds_h_txt = replace_once(
        ds_h_txt,
        "extern void interrupt far (*g_mouse_handler_bak)(__CPPARGS);\t// ds:0xbcdb; seg002",
        "#if defined(__BORLANDC__)\nextern void interrupt far (*g_mouse_handler_bak)(__CPPARGS);\t// ds:0xbcdb; seg002\n#endif",
        "datseg.h:g_mouse_handler_bak",
    )
    ds_h_txt = replace_once(
        ds_h_txt,
        "extern void interrupt far (*g_bc_timer)(__CPPARGS);\t// ds:0xe274; seg004",
        "#if defined(__BORLANDC__)\nextern void interrupt far (*g_bc_timer)(__CPPARGS);\t// ds:0xe274; seg004\n#endif\nextern signed int g_snd_voc_enabled;\t// ds:0x447c; seg002",
        "datseg.h:g_bc_timer",
    )
    ds_h_p.write_text(ds_h_txt, encoding="latin-1")


def patch_segments() -> None:
    # seg001.c / seg001.h
    s1_p = SRC_DIR / "seg001.c"
    s1_txt = s1_p.read_text(encoding="latin-1")
    s1_txt = replace_once(
        s1_txt,
        '#include "seg120.h"',
        '#include "seg120.h"\n#if !defined(__BORLANDC__)\n#include "dsa_platform.h"\n#endif',
        "seg001.c:include_dsa_platform",
    )
    s1_txt = replace_once(
        s1_txt,
        "static const int g_cd_audio_tracklist[15] = {",
        "static const int16_t g_cd_audio_tracklist[15] = {",
        "seg001.c:g_cd_audio_tracklist_int16",
    )
    s1_txt = replace_once(
        s1_txt,
        """signed int CD_bioskey(const signed int cmd)
{
#if defined(__BORLANDC__)""",
        """#if defined(__BORLANDC__)
signed int CD_bioskey(const signed int cmd)
{""",
        "seg001.c:CD_bioskey_start",
    )
    s1_txt = replace_once(
        s1_txt,
        """#else
	return 0;
#endif
}

#if defined(__BORLANDC__)
/* CD_audio_stop_hsg()""",
        """}

/* CD_audio_stop_hsg()""",
        "seg001.c:CD_bioskey_end",
    )
    s1_txt = replace_once(
        s1_txt,
        """	req[6].status = 0;
	CD_driver_request(&req[6]);
#endif
}""",
        """	req[6].status = 0;
	CD_driver_request(&req[6]);
#else
	if (g_cd_audio_paused) return;
	g_cd_audio_paused = 1;
	dsa_platform_pause_cd_audio(1);
	dsa_platform_pause_music(1);
#endif
}""",
        "seg001.c:CD_audio_pause",
    )
    s1_txt = replace_once(
        s1_txt,
        """	req[7].status = 0;
	CD_driver_request(&req[7]);
#endif
}""",
        """	req[7].status = 0;
	CD_driver_request(&req[7]);
#else
	if (!g_cd_audio_paused) return;
	g_cd_audio_paused = 0;
	dsa_platform_pause_cd_audio(0);
	dsa_platform_pause_music(0);
#endif
}""",
        "seg001.c:CD_audio_resume",
    )
    s1_txt = replace_once(
        s1_txt,
        """		if (g_cd_audio_paused)
		{
			g_cd_audio_paused = 0;
			CD_audio_pause();
		}
	}
#endif
}""",
        """		if (g_cd_audio_paused)
		{
			g_cd_audio_paused = 0;
			CD_audio_pause();
		}
	}
#else
	/* Uebernommen aus MobileEyes (Commit 4638425f, GPL v3, Gero Ohrner):
	 * Platz in g_cd_audio_tracklist = Nummer des CD-Tracks (MUSIC/TrackNN.OGG der GOG-Fassung) */
	signed int i;
	for (i = 0; i < 15; i++) {
		if (g_cd_audio_tracklist[i] == index) break;
	}
	g_cd_audio_track = i + 1;
	if (g_cd_audio_track_bak != g_cd_audio_track) {
		g_cd_audio_track_bak = g_cd_audio_track;
		dsa_platform_play_cd_track(g_cd_audio_track);
		if (g_cd_audio_paused) {
			dsa_platform_pause_cd_audio(1);
		}
	}
#endif
}""",
        "seg001.c:CD_set_track",
    )
    s1_p.write_text(s1_txt, encoding="latin-1")

    s1h_p = SRC_DIR / "seg001.h"
    s1h_txt = s1h_p.read_text(encoding="latin-1")
    s1h_txt = replace_once(
        s1h_txt,
        "signed int CD_bioskey(const signed int);",
        "#if defined(__BORLANDC__)\nsigned int CD_bioskey(const signed int);\n#endif",
        "seg001.h:CD_bioskey",
    )
    s1h_p.write_text(s1h_txt, encoding="latin-1")

    # seg008.h & seg008.c
    s8h_p = SRC_DIR / "seg008.h"
    s8h_txt = s8h_p.read_text(encoding="latin-1")
    s8h_txt = replace_once(
        s8h_txt,
        """void set_video_mode(const int16_t);
void set_video_page(const int16_t);
void save_display_stat(int16_t*);
void set_color(uint8_t*, const uint8_t);
void set_palette(uint8_t*, const signed int, const signed int);""",
        """#if defined(__BORLANDC__)
void set_video_mode(const int16_t);
void set_video_page(const int16_t);
void save_display_stat(int16_t*);
void set_color(uint8_t*, const uint8_t);
void set_palette(uint8_t*, const signed int, const signed int);
#else
void set_palette(const unsigned char*, const unsigned char, const unsigned short);
static inline void set_color(uint8_t *pal, const uint8_t color)
{
	set_palette(pal, color, 1);
}
#endif""",
        "seg008.h:set_palette",
    )
    s8h_p.write_text(s8h_txt, encoding="latin-1")

    s8_p = SRC_DIR / "seg008.c"
    s8_txt = s8_p.read_text(encoding="latin-1")
    s8_txt = replace_once(
        s8_txt,
        """void set_video_mode(const int16_t mode)
{
}

void set_video_page(const int16_t page)
{
}

void save_display_stat(int16_t* p)
{
}

void set_color(uint8_t *ptr, uint8_t color)
{
}

void set_palette(uint8_t *ptr, const signed int first_color, const signed int colors)
{
}""",
        """/* set_palette / set_color / sdl_init_video stehen in Port/platform/vgalib.c */""",
        "seg008.c:remove_stubs",
    )
    s8_p.write_text(s8_txt, encoding="latin-1")

    s2h_p = SRC_DIR / "seg002.h"
    s2h_txt = s2h_p.read_text(encoding="latin-1")
    s2h_txt = replace_once(
        s2h_txt,
        "int32_t swap_u32(uint32_t);",
        "uint32_t swap_u32(uint32_t);",
        "seg002.h:swap_u32",
    )
    s2h_p.write_text(s2h_txt, encoding="latin-1")

    # seg002.c
    s2_p = SRC_DIR / "seg002.c"
    s2_txt = s2_p.read_text(encoding="latin-1")
    s2_txt = replace_once(
        s2_txt,
        """static inline int16_t readws(uint8_t *p)
{
	return ((int16_t)(readb(p + 1) << 8) | (readb(p)));
}

static inline int32_t readds(uint8_t *p)
{
	return ((int32_t)(readws(p + 2) << 16) | readws(p));
}""",
        """static inline uint16_t readws(uint8_t *p)
{
	return (uint16_t)((readb(p + 1) << 8) | readb(p));
}

static inline uint32_t readds(uint8_t *p)
{
	return ((uint32_t)readws(p + 2) << 16) | (uint32_t)readws(p);
}""",
        "seg002.c:readws_readds",
    )
    s2_txt = replace_once(
        s2_txt,
        "int32_t swap_u32(uint32_t v)",
        "uint32_t swap_u32(uint32_t v)",
        "seg002.c:swap_u32",
    )
    s2_txt = replace_once(
        s2_txt,
        '#include "seg120.h"\n#include "seg121.h"',
        '#include "seg120.h"\n#include "seg121.h"\n\n#if !defined(__BORLANDC__)\n#if defined(_WIN32)\n#include <direct.h>\n#include <io.h>\n#define MKDIR_PORT_S2(p) _mkdir(p)\n#else\n#include <sys/stat.h>\n#define MKDIR_PORT_S2(p) mkdir(p, 0755)\n#endif\n#include "dsa_platform.h"\n#include "vgalib.h"\nstatic int g_skip_copy_protection = \n#if defined(SKIP_COPY_PROTECTION)\n    SKIP_COPY_PROTECTION;\n#else\n    1;\n#endif\n#endif',
        "seg002.c:includes",
    )
    s2_txt = replace_once(
        s2_txt,
        "int8_t g_fig_move_pathdir[10]; // ds:0xd823",
        "#if defined(__BORLANDC__)\nint8_t g_fig_move_pathdir[10]; // ds:0xd823\n#else\nint8_t g_fig_move_pathdir[20]; // ds:0xd823\nuint8_t *g_ail_voc_buffer;\nsigned int g_ail_music_inactive;\nuint8_t *g_ail_midi_buffer;\n#endif",
        "seg002.c:g_fig_move_pathdir",
    )
    s2_txt = replace_once(
        s2_txt,
        """void mouse_move_cursor(const signed int x, const signed int y)
{
#if defined(__BORLANDC__)
	int16_t l1 = 4;
	int16_t l3;
	int16_t l4 = x;
	int16_t l5 = y;
	int16_t l6;

	mouse_action(&l1, &l3, &l4, &l5, &l6);
#endif
}""",
        """void mouse_move_cursor(const signed int x, const signed int y)
{
#if defined(__BORLANDC__)
	int16_t l1 = 4;
	int16_t l3;
	int16_t l4 = x;
	int16_t l5 = y;
	int16_t l6;

	mouse_action(&l1, &l3, &l4, &l5, &l6);
#else
	dsa_platform_set_mouse_pos(x, y);
#endif
}""",
        "seg002.c:mouse_move_cursor",
    )

    s2_txt = replace_once(
        s2_txt,
        """static void play_music_file(const signed int index)
{
#if defined(__BORLANDC__)
	if (g_music_enabled != 0) {
		do_play_music_file(index);
	}
#endif
}""",
        """static void play_music_file(const signed int index)
{
	if (g_music_enabled != 0) {
		do_play_music_file(index);
	}
}""",
        "seg002.c:play_music_file",
    )
    s2_txt = replace_once(
        s2_txt,
        """	}
#endif
}

/* reserviert den MIDI-Puffer und startet AIL.""",
        """	}
#else
	/* Uebernommen aus MobileEyes (Commit 4638425f, GPL v3, Gero Ohrner):
	 * Standard = Musik ueber die Audio-CD (MUSIC/TrackNN.OGG der GOG-Fassung, g_use_cdaudio_flag = 1),
	 * bei Schalter --music=gm oder --music=opl hingegen XMIDI (g_use_cdaudio_flag = 0). */
	g_use_cdaudio_flag = (dsa_platform_get_music_mode() == DSA_MUSIC_MODE_CD) ? 1 : 0;
	g_ail_music_inactive = 0;
	g_snd_voc_enabled = 1;
	g_snd_effects_enabled = 1;
	{
		signed int handle = load_archive_file(ARCHIVE_FILE_SAMPLE_AD);
		if (handle != -1) {
			uint8_t *ad_buf = (uint8_t*)schick_alloc(0x4000);
			if (ad_buf) {
				uint16_t ad_len = read_archive_file(handle, ad_buf, 0x3fff);
				dsa_platform_load_adlib_bank(ad_buf, ad_len);
				free(ad_buf);
			}
			close(handle);
		}
	}
#endif
}

/* reserviert den MIDI-Puffer und startet AIL.""",
        "seg002.c:read_sound_cfg",
    )
    s2_txt = replace_once(
        s2_txt,
        """void init_AIL(const uint32_t size)
{
#if defined(__BORLANDC__)
	if ((g_ail_midi_buffer = (uint8_t*)schick_alloc(size))) {
		AIL_startup();
		g_ail_music_inactive = 1;
	}
#endif
}

void exit_AIL(void)
{
#if defined(__BORLANDC__)
	AIL_shutdown((uint8_t*)NULL);

	if (g_ail_timbre_cache) {
		free((void*)g_ail_timbre_cache);
	}

	if (g_ail_state_table) {
		free((void*)g_ail_state_table);
	}

	if (g_ail_midi_buffer) {
		free((void*)g_ail_midi_buffer);
	}

	if (g_ail_music_driver_buf2) {
		free((void*)g_ail_music_driver_buf2);
	}

	/* set all pointers to NULL */
	g_ail_timbre_cache = g_ail_state_table = g_ail_midi_buffer = g_ail_music_driver_buf2 = NULL;

	if (g_snd_voc_enabled) {
		free_voc_buffer();
	}
#endif
}""",
        """void init_AIL(const uint32_t size)
{
#if defined(__BORLANDC__)
	if ((g_ail_midi_buffer = (uint8_t*)schick_alloc(size))) {
		AIL_startup();
		g_ail_music_inactive = 1;
	}
#else
	g_ail_midi_buffer = (uint8_t*)schick_alloc(size < 0x8000 ? 0x8000 : size);
	g_ail_music_inactive = 0;
#endif
}

void exit_AIL(void)
{
#if defined(__BORLANDC__)
	AIL_shutdown((uint8_t*)NULL);

	if (g_ail_timbre_cache) {
		free((void*)g_ail_timbre_cache);
	}

	if (g_ail_state_table) {
		free((void*)g_ail_state_table);
	}

	if (g_ail_midi_buffer) {
		free((void*)g_ail_midi_buffer);
	}

	if (g_ail_music_driver_buf2) {
		free((void*)g_ail_music_driver_buf2);
	}

	/* set all pointers to NULL */
	g_ail_timbre_cache = g_ail_state_table = g_ail_midi_buffer = g_ail_music_driver_buf2 = NULL;

	if (g_snd_voc_enabled) {
		free_voc_buffer();
	}
#else
	dsa_platform_stop_cd_audio();
	dsa_platform_stop_music();
	dsa_platform_stop_voc();
	if (g_ail_midi_buffer) {
		free((void*)g_ail_midi_buffer);
		g_ail_midi_buffer = NULL;
	}
	if (g_snd_voc_enabled) {
		free_voc_buffer();
	}
#endif
}""",
        "seg002.c:init_exit_AIL",
    )
    s2_txt = replace_once(
        s2_txt,
        """static signed int start_midi_playback(const signed int seq)
{
#if defined(__BORLANDC__)
	if (prepare_midi_playback(seq)) {
		AIL_start_sequence(g_ail_music_driver_id, seq);
		return 1;
	}
#endif
	return 0;
}""",
        """static uint32_t g_native_midi_len = 0;
static uint32_t g_native_voc_len = 0;

static signed int start_midi_playback(const signed int seq)
{
#if defined(__BORLANDC__)
	if (prepare_midi_playback(seq)) {
		AIL_start_sequence(g_ail_music_driver_id, seq);
		return 1;
	}
#else
	(void)seq;
	if (g_ail_midi_buffer && g_native_midi_len > 0) {
		return dsa_platform_play_xmidi(g_ail_midi_buffer, g_native_midi_len, 1);
	}
#endif
	return 0;
}""",
        "seg002.c:start_midi_playback",
    )
    s2_txt = replace_once(
        s2_txt,
        """static signed int do_load_midi_file(const signed int index)
{
#if defined(__BORLANDC__)
	signed int handle;

	if ((handle = load_archive_file(index)) != -1) {
		read_archive_file(handle, g_ail_midi_buffer, 0x7fff);
		close(handle);
		return 1;
	}
#endif
	return 0;
}""",
        """static signed int do_load_midi_file(const signed int index)
{
#if defined(__BORLANDC__)
	signed int handle;

	if ((handle = load_archive_file(index)) != -1) {
		read_archive_file(handle, g_ail_midi_buffer, 0x7fff);
		close(handle);
		return 1;
	}
#else
	signed int handle;
	if (g_ail_midi_buffer && (handle = load_archive_file(index)) != -1) {
		g_native_midi_len = read_archive_file(handle, g_ail_midi_buffer, 0x7fff);
		close(handle);
		return (g_native_midi_len > 0) ? 1 : 0;
	}
#endif
	return 0;
}""",
        "seg002.c:do_load_midi_file",
    )
    s2_txt = replace_once(
        s2_txt,
        """static void do_play_music_file(const signed int index)
{
#if defined(__BORLANDC__)
	if (!g_ail_music_inactive && (g_ail_music_driver_descr[1] == 3)) {

		stop_midi_playback();
		load_midi_file(index);
		start_midi_playback(0);
	}
#endif
}

static void stop_midi_playback(void)
{
#if defined(__BORLANDC__)
	if (!g_ail_music_inactive && (g_ail_music_driver_descr[1] == 3))
	{
		AIL_stop_sequence(g_ail_music_driver_id, g_ail_sequence);
		AIL_release_sequence_handle(g_ail_music_driver_id, g_ail_sequence);
	}
#endif
}""",
        """static void do_play_music_file(const signed int index)
{
#if defined(__BORLANDC__)
	if (!g_ail_music_inactive && (g_ail_music_driver_descr[1] == 3)) {

		stop_midi_playback();
		load_midi_file(index);
		start_midi_playback(0);
	}
#else
	if (!g_ail_music_inactive) {
		stop_midi_playback();
		if (load_midi_file(index)) {
			start_midi_playback(0);
		}
	}
#endif
}

static void stop_midi_playback(void)
{
#if defined(__BORLANDC__)
	if (!g_ail_music_inactive && (g_ail_music_driver_descr[1] == 3))
	{
		AIL_stop_sequence(g_ail_music_driver_id, g_ail_sequence);
		AIL_release_sequence_handle(g_ail_music_driver_id, g_ail_sequence);
	}
#else
	dsa_platform_stop_music();
#endif
}""",
        "seg002.c:do_play_stop_midi",
    )
    s2_txt = replace_once(
        s2_txt,
        """	return retval;
#else
	return 0;
#endif
}

void play_voc(const signed int index)
{
#if defined(__BORLANDC__)
	if (g_snd_voc_enabled && g_snd_effects_enabled) {
		SND_set_volume(90);
		SND_play_voc(index);
	}
#endif
}

void play_voc_delay(const signed int index)
{
#if defined(__BORLANDC__)
	if (g_snd_voc_enabled && g_snd_effects_enabled) {
		SND_set_volume(90);
		SND_play_voc(index);

		while (AIL_VOC_playback_status(g_ail_digi_driver_id) == 2) {
			wait_for_vsync();
		}
	}
#endif
}

void alloc_voc_buffer(uint32_t size)
{
#if defined(__BORLANDC__)
	if (g_snd_voc_enabled) {
		if ((g_ail_voc_buffer = (uint8_t*)schick_alloc(size))) ;
	}
#endif
}

static void free_voc_buffer(void)
{
#if defined(__BORLANDC__)
	if (g_snd_voc_enabled != 0) {

		if (g_ail_voc_buffer) {
			free((void*)g_ail_voc_buffer);
		}

		if (g_ail_digi_driver_buf2) {
			free((void*)g_ail_digi_driver_buf2);
		}

		g_ail_voc_buffer = g_ail_digi_driver_buf2 = NULL;

	}
#endif
}

static signed int read_new_voc_file(const signed int index)
{
#if defined(__BORLANDC__)
	if (AIL_VOC_playback_status(g_ail_digi_driver_id) == 2) {
		SND_stop_digi();
	}

	if (read_voc_file(index)) {

		AIL_format_VOC_file(g_ail_digi_driver_id, g_ail_voc_buffer, -1);
		return 1;
	}
#endif
	return 0;
}

static signed int read_voc_file(const signed int index)
{
#if defined(__BORLANDC__)
	signed int handle;

	if ( (handle = load_archive_file(index)) != -1) {
		read_archive_file(handle, g_ail_voc_buffer, 0x7fff);
		close(handle);
		return 1;
	}
#endif
	return 0;
}

static void SND_play_voc(const signed int index)
{
#if defined(__BORLANDC__)
	if (g_snd_voc_enabled) {

		AIL_stop_digital_playback(g_ail_digi_driver_id);
		read_new_voc_file(index);
		AIL_play_VOC_file(g_ail_digi_driver_id, g_ail_voc_buffer, -1);
		AIL_start_digital_playback(g_ail_digi_driver_id);
	}
#endif
}

static void SND_stop_digi(void)
{
#if defined(__BORLANDC__)
	if (g_snd_voc_enabled) {
		AIL_stop_digital_playback(g_ail_digi_driver_id);
	}
#endif
}

static void SND_set_volume(const uint16_t volume)
{
#if defined(__BORLANDC__)
	if (g_snd_voc_enabled) {

		AIL_set_digital_playback_volume(g_ail_digi_driver_id, volume);

		if (!volume) {
			SND_stop_digi();
		}
	}
#endif
}""",
        """	return retval;
#else
	g_snd_voc_enabled = 1;
	return 1;
#endif
}

void play_voc(const signed int index)
{
	if (g_snd_voc_enabled && g_snd_effects_enabled) {
		SND_set_volume(90);
		SND_play_voc(index);
	}
}

void play_voc_delay(const signed int index)
{
#if defined(__BORLANDC__)
	if (g_snd_voc_enabled && g_snd_effects_enabled) {
		SND_set_volume(90);
		SND_play_voc(index);

		while (AIL_VOC_playback_status(g_ail_digi_driver_id) == 2) {
			wait_for_vsync();
		}
	}
#else
	if (g_snd_voc_enabled && g_snd_effects_enabled) {
		SND_set_volume(90);
		SND_play_voc(index);
		if (!dsa_platform_is_fast_replay()) {
			while (dsa_platform_is_voc_playing()) {
				wait_for_vsync();
			}
		}
	}
#endif
}

void alloc_voc_buffer(uint32_t size)
{
#if defined(__BORLANDC__)
	if (g_snd_voc_enabled) {
		if ((g_ail_voc_buffer = (uint8_t*)schick_alloc(size))) ;
	}
#else
	if (g_snd_voc_enabled) {
		g_ail_voc_buffer = (uint8_t*)schick_alloc(size < 0x8000 ? 0x8000 : size);
	}
#endif
}

static void free_voc_buffer(void)
{
#if defined(__BORLANDC__)
	if (g_snd_voc_enabled != 0) {

		if (g_ail_voc_buffer) {
			free((void*)g_ail_voc_buffer);
		}

		if (g_ail_digi_driver_buf2) {
			free((void*)g_ail_digi_driver_buf2);
		}

		g_ail_voc_buffer = g_ail_digi_driver_buf2 = NULL;

	}
#else
	if (g_snd_voc_enabled != 0 && g_ail_voc_buffer) {
		free((void*)g_ail_voc_buffer);
		g_ail_voc_buffer = NULL;
	}
#endif
}

static signed int read_new_voc_file(const signed int index)
{
#if defined(__BORLANDC__)
	if (AIL_VOC_playback_status(g_ail_digi_driver_id) == 2) {
		SND_stop_digi();
	}

	if (read_voc_file(index)) {

		AIL_format_VOC_file(g_ail_digi_driver_id, g_ail_voc_buffer, -1);
		return 1;
	}
#else
	if (dsa_platform_is_voc_playing()) {
		SND_stop_digi();
	}
	if (read_voc_file(index)) {
		return 1;
	}
#endif
	return 0;
}

static signed int read_voc_file(const signed int index)
{
#if defined(__BORLANDC__)
	signed int handle;

	if ( (handle = load_archive_file(index)) != -1) {
		read_archive_file(handle, g_ail_voc_buffer, 0x7fff);
		close(handle);
		return 1;
	}
#else
	signed int handle;
	if (g_ail_voc_buffer && (handle = load_archive_file(index)) != -1) {
		g_native_voc_len = read_archive_file(handle, g_ail_voc_buffer, 0x7fff);
		close(handle);
		return (g_native_voc_len > 0) ? 1 : 0;
	}
#endif
	return 0;
}

static void SND_play_voc(const signed int index)
{
#if defined(__BORLANDC__)
	if (g_snd_voc_enabled) {

		AIL_stop_digital_playback(g_ail_digi_driver_id);
		read_new_voc_file(index);
		AIL_play_VOC_file(g_ail_digi_driver_id, g_ail_voc_buffer, -1);
		AIL_start_digital_playback(g_ail_digi_driver_id);
	}
#else
	if (g_snd_voc_enabled) {
		SND_stop_digi();
		if (read_new_voc_file(index)) {
			dsa_platform_play_voc(g_ail_voc_buffer, g_native_voc_len);
		}
	}
#endif
}

static void SND_stop_digi(void)
{
#if defined(__BORLANDC__)
	if (g_snd_voc_enabled) {
		AIL_stop_digital_playback(g_ail_digi_driver_id);
	}
#else
	if (g_snd_voc_enabled) {
		dsa_platform_stop_voc();
	}
#endif
}

static void SND_set_volume(const uint16_t volume)
{
#if defined(__BORLANDC__)
	if (g_snd_voc_enabled) {

		AIL_set_digital_playback_volume(g_ail_digi_driver_id, volume);

		if (!volume) {
			SND_stop_digi();
		}
	}
#else
	if (g_snd_voc_enabled) {
		dsa_platform_set_sfx_volume((int)volume);
		if (!volume) {
			SND_stop_digi();
		}
	}
#endif
}""",
        "seg002.c:voc_functions",
    )

    sdl_input_block = """static void mouse_motion(void)
{
	/* return if mouse was not moved and the cursor remains */
	if (g_mouse_moved || (g_last_cursor != g_current_cursor)) {

		/* set new cursor */
		g_last_cursor = g_current_cursor;

		/* check if the new cursor is the default cursor */
		if (g_current_cursor == &g_default_mouse_cursor) {
			/* set cursor size 0x0 */
			/* genauer: Hotspot oben links (0,0) */
			g_mouse_pointer_offsetx = g_mouse_pointer_offsety = 0;
		} else {
			/* set cursor size 8x8 */
			/* genauer: Hotspot in der Mitte des 16x16-Cursors (8,8) */
			g_mouse_pointer_offsetx = g_mouse_pointer_offsety = 8;
		}

		/* reset mouse was moved */
		g_mouse_moved = 0;
		mouse_bg();
		mouse_cursor();
	}
}

#if !defined(__BORLANDC__)
/**
 * Emulation des DOS-Timer-Interrupts (INT 8, 18,2 Hz).
 * Ruft timer_isr() (seg004.c) entsprechend der verstrichenen Echtzeit auf.
 */
static void sdl_timer_emulation(void)
{
	static double next = 0.0;
	static int in_timer = 0;
	const double period = 1000.0 / 18.2065;
	const double now = (double)dsa_platform_get_ticks_ms();
	int ticks = 0;

	if (!g_native_timer_active || in_timer) return;

	if (next == 0.0 || now - next > 10.0 * period) next = now;

	in_timer = 1;
	while (now >= next && ticks < 4) {
		timer_isr();
		next += period;
		ticks++;
	}
	in_timer = 0;
}

static void sdl_poll_input(void)
{
	int mx = g_mouse_posx, my = g_mouse_posy;
	int moved = 0, left_click = 0, right_click = 0;

	sdl_timer_emulation();

	dsa_platform_set_mouse_range_x(g_mouse_posx_min, g_mouse_posx_max);
	dsa_platform_set_mouse_range_y(g_mouse_posy_min, g_mouse_posy_max);
	dsa_platform_poll_mouse_events(&mx, &my, &moved, &left_click, &right_click);

	if (!g_mouse_locked) {
		if (left_click) {
			g_mouse1_event2 = 1;
			g_mouse_leftclick_event = 1;
		}
		if (right_click) {
			g_mouse_rightclick_event = 1;
		}
		if (moved) {
			g_mouse_posx = mx;
			g_mouse_posy = my;
			if (g_mouse_posx > g_mouse_posx_max) g_mouse_posx = g_mouse_posx_max;
			if (g_mouse_posx < g_mouse_posx_min) g_mouse_posx = g_mouse_posx_min;
			if (g_mouse_posy < g_mouse_posy_min) g_mouse_posy = g_mouse_posy_min;
			if (g_mouse_posy > g_mouse_posy_max) g_mouse_posy = g_mouse_posy_max;
			g_mouse_moved = 1;
		}
		/* Wie in mouse_isr() des DOS-Originals: Richtungspfeil-Cursor im 3D-Sichtfenster */
		if (((gs_dungeon_id != DUNGEON_ID_NONE) || (gs_town_id != TOWN_ID_NONE)) &&
			!gs_town_loc_type && !g_dialogbox_lock && (g_pp20_index == ARCHIVE_FILE_PLAYM_UK))
		{
			g_current_cursor = (is_mouse_in_rect( 68,  4, 171,  51) ? &g_cursor_arrow_up :
					   (is_mouse_in_rect( 68, 89, 171, 136) ? &g_cursor_arrow_down :
					   (is_mouse_in_rect( 16, 36,  67,  96) ? &g_cursor_arrow_left :
					   (is_mouse_in_rect(172, 36, 223,  96) ? &g_cursor_arrow_right :
					   (!is_mouse_in_rect(16,  4, 223, 138) ? &g_default_mouse_cursor : g_current_cursor)))));
		} else if (g_dialogbox_lock) {
			g_current_cursor = &g_default_mouse_cursor;
		}
	} else if (right_click) {
		g_mouse_rightclick_event = 1;
	}
}

void sdl_wait_vsync(void)
{
	sdl_poll_input();
	mouse_motion();
	sdl_present_if_changed();
	dsa_platform_delay_ms(14);
}

signed int bioskey(const signed int cmd)
{
	sdl_poll_input();
	mouse_motion();
	sdl_present_if_changed();
	if (cmd == 1) {
		unsigned short ready = dsa_platform_key_ready();
		if (!ready) {
			dsa_platform_delay_ms(1);
		}
		return ready ? 1 : 0;
	} else if (cmd == 0) {
		if (dsa_platform_key_ready()) {
			return (signed int)dsa_platform_key_read();
		}
		return 0;
	}
	return 0;
}
#endif"""

    s2_txt = replace_once(
        s2_txt,
        """static void mouse_motion(void)
{
	/* return if mouse was not moved and the cursor remains */
	if (g_mouse_moved || (g_last_cursor != g_current_cursor)) {

		/* set new cursor */
		g_last_cursor = g_current_cursor;

		/* check if the new cursor is the default cursor */
		if (g_current_cursor == &g_default_mouse_cursor) {
			/* set cursor size 0x0 */
			/* genauer: Hotspot oben links (0,0) */
			g_mouse_pointer_offsetx = g_mouse_pointer_offsety = 0;
		} else {
			/* set cursor size 8x8 */
			/* genauer: Hotspot in der Mitte des 16x16-Cursors (8,8) */
			g_mouse_pointer_offsetx = g_mouse_pointer_offsety = 8;
		}

		/* reset mouse was moved */
		g_mouse_moved = 0;
		mouse_bg();
		mouse_cursor();
	}
}""",
        sdl_input_block,
        "seg002.c:mouse_motion_and_sdl",
    )

    s2_txt = replace_once(
        s2_txt,
        """void flush_keyboard_queue_alt(void)
{
#if defined(__BORLANDC__)
	while (CD_bioskey(1)) {
		bioskey(0);
	}
#endif
}""",
        """void flush_keyboard_queue_alt(void)
{
	while (CD_bioskey(1)) {
		bioskey(0);
	}
}""",
        "seg002.c:flush_keyboard_queue_alt",
    )

    # dawning & nightfall: 2 separate palette calls natively
    s2_txt = replace_once(
        s2_txt,
        """			wait_for_vsync();

			set_palette(gs_palette_floor, 0, 0x20);
			/* 0x40 Farben: gs_palette_buildings und das direkt folgende gs_palette_sky in einem Rutsch */
			set_palette(gs_palette_buildings, 0x80, 0x40);
		}
	}
}""",
        """			wait_for_vsync();

			set_palette(gs_palette_floor, 0, 0x20);
#if defined(__BORLANDC__)
			/* 0x40 Farben: gs_palette_buildings und das direkt folgende gs_palette_sky in einem Rutsch */
			set_palette(gs_palette_buildings, 0x80, 0x40);
#else
			set_palette(gs_palette_buildings, 0x80, 0x20);
			set_palette(gs_palette_sky, 0x80 + 0x20, 0x20);
#endif
		}
	}
}""",
        "seg002.c:dawning",
    )
    s2_txt = replace_once(
        s2_txt,
        """			wait_for_vsync();

			set_palette(gs_palette_floor, 0, 0x20);
			set_palette(gs_palette_buildings, 0x80, 0x40);
		}
	}
}""",
        """			wait_for_vsync();

			set_palette(gs_palette_floor, 0, 0x20);
#if defined(__BORLANDC__)
			set_palette(gs_palette_buildings, 0x80, 0x40);
#else
			set_palette(gs_palette_buildings, 0x80, 0x20);
			set_palette(gs_palette_sky, 0x80 + 0x20, 0x20);
#endif
		}
	}
}""",
        "seg002.c:nightfall",
    )

    # update_travelmap: g_pic_copy.dst = g_vga_memstart natively
    s2_txt = replace_once(
        s2_txt,
        """	p2 = g_buffer6_ptr + 2100;
	p3 = g_buffer6_ptr + 1000;

	if (g_trv_menu_selection && gs_show_travel_map) {""",
        """	p2 = g_buffer6_ptr + 2100;
	p3 = g_buffer6_ptr + 1000;

#if !defined(__BORLANDC__)
	g_pic_copy.dst = g_vga_memstart;
#endif

	if (g_trv_menu_selection && gs_show_travel_map) {""",
        "seg002.c:update_travelmap",
    )

    # main() und copy_protection() in seg002.c
    s2_txt = replace_once(
        s2_txt,
        "int main(int argc, char** argv)\n{",
        """#if !defined(__BORLANDC__)
extern int ngen_run(int init_level, int called_from_game);
extern int ngen_run_intro(void);
static int g_run_intro = 0;

static void schick_format_crash_state(char *buf, int buf_size)
{
	int n = snprintf(buf, (size_t)buf_size,
		"Spielort:        town_id=%d dungeon_id=%d loc_type=%d pos=(%d,%d) dir=%d\\n"
		"Spielstand:      active_group=%d day_timer=%ld datum=%d.%d.%d\\n",
		(int)gs_town_id, (int)gs_dungeon_id, (int)gs_town_loc_type,
		(int)gs_x, (int)gs_y, (int)gs_viewdir,
		(int)gs_active_group_id, (long)gs_day_timer,
		(int)gs_day_of_month, (int)gs_month, (int)gs_year
	);
	if (g_heroes != NULL && n > 0 && n < buf_size) {
		for (int hi = 0; hi <= 6 && n < buf_size - 64; ++hi) {
			struct struct_hero *h = get_hero(hi);
			if (h && h->typus != 0) {
				n += snprintf(buf + n, (size_t)(buf_size - n),
					"Held[%d]:         %s (Typ=%d, Grp=%d, LE=%d/%d, AE=%d/%d)\\n",
					hi, h->alias, (int)h->typus, (int)h->group_id,
					(int)h->le, (int)h->le_max, (int)h->ae, (int)h->ae_max
				);
			}
		}
	}
}
#endif

int main(int argc, char** argv)
{""",
        "seg002.c:crash_state_cb",
    )
    s2_txt = replace_once(
        s2_txt,
        """	g_pregame_state = 1;

	g_playmask_us = 1;""",
        """#if !defined(__BORLANDC__)
	{
		dsa_platform_register_crash_state_cb(schick_format_crash_state);
		const char *env_cp = getenv("DSA_COPY_PROTECTION");
		if (env_cp && env_cp[0]) {
			g_skip_copy_protection = (env_cp[0] == '0') ? 1 : 0;
		}
		const char *env_intro = getenv("DSA_INTRO");
		if (env_intro && env_intro[0] == '1') {
			g_run_intro = 1;
		}
		for (int arg_i = 1; arg_i < argc; ++arg_i) {
			if (strcmp(argv[arg_i], "--copy-protection") == 0) {
				g_skip_copy_protection = 0;
			} else if (strcmp(argv[arg_i], "--skip-copy-protection") == 0) {
				g_skip_copy_protection = 1;
			} else if (strcmp(argv[arg_i], "--intro") == 0) {
				g_run_intro = 1;
			} else if (strcmp(argv[arg_i], "--coverage") == 0) {
				g_dsa_coverage_enabled = 1;
			} else if (strcmp(argv[arg_i], "--fast-replay") == 0) {
				dsa_platform_set_fast_replay(1);
			} else if (strcmp(argv[arg_i], "--cd") == 0 || strcmp(argv[arg_i], "--music=cd") == 0 || strcmp(argv[arg_i], "--music=ogg") == 0) {
				dsa_platform_set_music_mode(DSA_MUSIC_MODE_CD);
			} else if (strcmp(argv[arg_i], "--gm") == 0 || strcmp(argv[arg_i], "--music=gm") == 0 || strcmp(argv[arg_i], "--music=midi") == 0) {
				dsa_platform_set_music_mode(DSA_MUSIC_MODE_GM);
			} else if (strcmp(argv[arg_i], "--opl") == 0 || strcmp(argv[arg_i], "--adlib") == 0 || strcmp(argv[arg_i], "--music=opl") == 0 || strcmp(argv[arg_i], "--music=adlib") == 0) {
				dsa_platform_set_music_mode(DSA_MUSIC_MODE_OPL);
			} else if (strcmp(argv[arg_i], "--music") == 0 && arg_i + 1 < argc) {
				const char *m = argv[++arg_i];
				if (strcmp(m, "opl") == 0 || strcmp(m, "adlib") == 0) {
					dsa_platform_set_music_mode(DSA_MUSIC_MODE_OPL);
				} else if (strcmp(m, "gm") == 0 || strcmp(m, "midi") == 0) {
					dsa_platform_set_music_mode(DSA_MUSIC_MODE_GM);
				} else {
					dsa_platform_set_music_mode(DSA_MUSIC_MODE_CD);
				}
			} else if (strcmp(argv[arg_i], "--dump-wav") == 0 && arg_i + 1 < argc) {
				dsa_platform_set_dump_wav(argv[++arg_i]);
			} else if (strcmp(argv[arg_i], "--record") == 0 && arg_i + 1 < argc) {
				dsa_platform_set_record_file(argv[++arg_i]);
			} else if (strcmp(argv[arg_i], "--replay") == 0 && arg_i + 1 < argc) {
				dsa_platform_set_replay_file(argv[++arg_i]);
			} else if (strcmp(argv[arg_i], "--gamedir") == 0 && arg_i + 1 < argc) {
				dsa_platform_set_game_dir(argv[++arg_i]);
			} else if (argv[arg_i][0] != '-') {
				dsa_platform_set_game_dir(argv[arg_i]);
			}
		}
		if (access("SCHICK.DAT", 0) != 0) {
			const char *gd = dsa_platform_get_game_dir();
			if (gd && gd[0]) {
				chdir(gd);
			}
		}
		MKDIR_PORT_S2("TEMP");
	}
#endif
	g_pregame_state = 1;

	g_playmask_us = 1;""",
        "seg002.c:main_args",
    )

    s2_txt = replace_once(
        s2_txt,
        """		} else {
			/* disable sound */
			exit_AIL();
			GUI_output(g_snd_txt_disabled_mem2);
		}""",
        """		} else {
			/* disable sound */
			exit_AIL();
#if defined(__BORLANDC__)
			GUI_output(g_snd_txt_disabled_mem2);
#endif
		}""",
        "seg002.c:disable_sound_msg",
    )

    s2_txt = replace_once(
        s2_txt,
        """		/* select game mode */
		g_game_mode = GAME_MODE_UNSPECIFIED;""",
        """#if !defined(__BORLANDC__)
		if (g_run_intro) {
			call_mouse_bg();
			ngen_run_intro();
			refresh_colors();
			dsa_platform_set_mouse_range_x(0, 319);
			dsa_platform_set_mouse_range_y(0, 199);
			call_mouse();
			flush_keyboard_queue();
		}
#endif
		/* select game mode */
		g_game_mode = GAME_MODE_UNSPECIFIED;""",
        "seg002.c:run_intro",
    )

    s2_txt = replace_once(
        s2_txt,
        """static signed int copy_protection(void)
{
	signed int i;""",
        """static signed int copy_protection(void)
{
#if !defined(__BORLANDC__)
	if (g_skip_copy_protection) {
		return 1;
	}
#endif
	signed int i;""",
        "seg002.c:copy_protection",
    )
    s2_p.write_text(s2_txt, encoding="latin-1")

    # seg003.c: bounds check in move()
    s3_p = SRC_DIR / "seg003.c"
    s3_txt = s3_p.read_text(encoding="latin-1")
    s3_txt = replace_once(
        s3_txt,
        """	if (g_map_size_x == 16) {
		/* dungeon or small town */
		g_steptarget_front = *(p_map_small + MAP_POS(gs_x + p_visual_square[0].x, gs_y + p_visual_square[0].y));

		g_steptarget_back  = *(p_map_small + MAP_POS(gs_x + p_visual_square[1].x, gs_y + p_visual_square[1].y));
	} else {
		/* large city */
		g_steptarget_front = *(p_map_large + LARGE_MAP_POS(gs_x + p_visual_square[0].x, gs_y + p_visual_square[0].y));

		g_steptarget_back  = *(p_map_large + LARGE_MAP_POS(gs_x + p_visual_square[1].x, gs_y + p_visual_square[1].y));
	}""",
        """#if defined(__BORLANDC__)
	if (g_map_size_x == 16) {
		/* dungeon or small town */
		g_steptarget_front = *(p_map_small + MAP_POS(gs_x + p_visual_square[0].x, gs_y + p_visual_square[0].y));

		g_steptarget_back  = *(p_map_small + MAP_POS(gs_x + p_visual_square[1].x, gs_y + p_visual_square[1].y));
	} else {
		/* large city */
		g_steptarget_front = *(p_map_large + LARGE_MAP_POS(gs_x + p_visual_square[0].x, gs_y + p_visual_square[0].y));

		g_steptarget_back  = *(p_map_large + LARGE_MAP_POS(gs_x + p_visual_square[1].x, gs_y + p_visual_square[1].y));
	}
#else
	g_steptarget_front = 0;
	g_steptarget_back = 0;
	if (g_map_size_x == 16) {
		const int front_index = MAP_POS(gs_x + p_visual_square[0].x, gs_y + p_visual_square[0].y);
		const int back_index = MAP_POS(gs_x + p_visual_square[1].x, gs_y + p_visual_square[1].y);
		if ((0 <= front_index) && (front_index < 16 * 16)) g_steptarget_front = *(p_map_small + front_index);
		if ((0 <= back_index) && (back_index < 16 * 16)) g_steptarget_back = *(p_map_small + back_index);
	} else {
		const int front_index = LARGE_MAP_POS(gs_x + p_visual_square[0].x, gs_y + p_visual_square[0].y);
		const int back_index = LARGE_MAP_POS(gs_x + p_visual_square[1].x, gs_y + p_visual_square[1].y);
		if ((0 <= front_index) && (front_index < 32 * 16)) g_steptarget_front = *(p_map_large + front_index);
		if ((0 <= back_index) && (back_index < 32 * 16)) g_steptarget_back = *(p_map_large + back_index);
	}
#endif""",
        "seg003.c:move_bounds",
    )
    s3_p.write_text(s3_txt, encoding="latin-1")

    # seg004.c & seg004.h
    s4h_p = SRC_DIR / "seg004.h"
    s4h_txt = s4h_p.read_text(encoding="latin-1")
    s4h_txt = replace_once(
        s4h_txt,
        """#if defined(__BORLANDC__)
void interrupt timer_isr(void);
#endif""",
        """#if defined(__BORLANDC__)
void interrupt timer_isr(void);
#else
void timer_isr(void);
extern signed int g_native_timer_active;
void sdl_wait_vsync(void);
#endif""",
        "seg004.h:timer_isr",
    )
    s4h_p.write_text(s4h_txt, encoding="latin-1")

    s4_p = SRC_DIR / "seg004.c"
    s4_txt = s4_p.read_text(encoding="latin-1")
    s4_txt = replace_once(
        s4_txt,
        '#include "seg004.h"\n#include "seg008.h"',
        '#include "seg004.h"\n#include "seg008.h"\n#if !defined(__BORLANDC__)\n#include "vgalib.h"\n#endif',
        "seg004.c:vgalib_h",
    )
    s4_txt = replace_once(
        s4_txt,
        """void save_and_set_timer(void)
{
#if defined(__BORLANDC__)
	g_bc_timer = getvect(8);
	setvect(8, (void interrupt far(*)(__CPPARGS))timer_isr);
#endif
}

void set_timer(void)
{
#if defined(__BORLANDC__)
	setvect(8, (void interrupt far(*)(__CPPARGS))timer_isr);
#endif
}

void reset_timer(void)
{
#if defined(__BORLANDC__)
	setvect(8, (void interrupt far(*)(__CPPARGS))g_bc_timer);
#endif
}""",
        """#if !defined(__BORLANDC__)
signed int g_native_timer_active = 0;
#endif

void save_and_set_timer(void)
{
#if defined(__BORLANDC__)
	g_bc_timer = getvect(8);
	setvect(8, (void interrupt far(*)(__CPPARGS))timer_isr);
#else
	g_native_timer_active = 1;
#endif
}

void set_timer(void)
{
#if defined(__BORLANDC__)
	setvect(8, (void interrupt far(*)(__CPPARGS))timer_isr);
#else
	g_native_timer_active = 1;
#endif
}

void reset_timer(void)
{
#if defined(__BORLANDC__)
	setvect(8, (void interrupt far(*)(__CPPARGS))g_bc_timer);
#else
	g_native_timer_active = 0;
#endif
}""",
        "seg004.c:timer_funcs",
    )
    s4_txt = replace_once(
        s4_txt,
        """	 while (g_ani_busy) {
		/* REMARK: SPEEDFIX
		 * enter emulation mode frequently, that the timer can reset this variable
		wait_for_vsync();
		 * */
	 }""",
        """	 while (g_ani_busy) {
#if !defined(__BORLANDC__)
		wait_for_vsync();
#endif
	 }""",
        "seg004.c:init_ani_busy_loop",
    )
    s4_txt = replace_once(
        s4_txt,
        """#if defined(__BORLANDC__)
void interrupt timer_isr(void)
{""",
        """#if defined(__BORLANDC__)
void interrupt timer_isr(void)
#else
void timer_isr(void)
#endif
{""",
        "seg004.c:timer_isr_head",
    )
    s4_txt = replace_once(
        s4_txt,
        "	restart_midi_playback_IRQ();",
        "#if defined(__BORLANDC__)\n\trestart_midi_playback_IRQ();\n#endif",
        "seg004.c:restart_midi_playback_IRQ",
    )
    s4_txt = replace_once(
        s4_txt,
        """		/* disable interrupts */
		asm { cli; }""",
        """#if defined(__BORLANDC__)
		/* disable interrupts */
		asm { cli; }
#endif""",
        "seg004.c:cli",
    )
    s4_txt = replace_once(
        s4_txt,
        """		/* enable interrupts */
		asm { sti; }
	}

	/* call the old timer ISR */
	((void interrupt far(*)(__CPPARGS))g_bc_timer)();
}

/* wartet auf den naechsten Timer-Tick; wird nie aufgerufen */
static void unused_gfx_spinlock(void)""",
        """#if defined(__BORLANDC__)
		/* enable interrupts */
		asm { sti; }
#endif
	}

#if defined(__BORLANDC__)
	/* call the old timer ISR */
	((void interrupt far(*)(__CPPARGS))g_bc_timer)();
#endif
}

#if defined(__BORLANDC__)
/* wartet auf den naechsten Timer-Tick; wird nie aufgerufen */
static void unused_gfx_spinlock(void)""",
        "seg004.c:sti_and_old_timer",
    )
    s4_txt = replace_once(
        s4_txt,
        """	y = g_wallclock_y + g_wallclock_pos_y[pos];""",
        """#if defined(__BORLANDC__)
	y = g_wallclock_y + g_wallclock_pos_y[pos];
#else
	y = g_wallclock_y + g_wallclock_pos_y[pos > 80 ? 80 : (pos < 0 ? 0 : pos)];
#endif""",
        "seg004.c:draw_wallclock_clamp",
    )
    s4_txt = replace_once(
        s4_txt,
        """void schick_set_video(void)
{
	struct uint8_t_3 pal_white = g_color_pal_white;;

	set_video_mode(0x13);
	set_color((uint8_t*)&pal_white, 0xff);
}

void schick_reset_video(void)
{
#if defined(__BORLANDC__)
	set_video_mode(g_video_mode_bak);
	set_video_page(g_video_page_bak);
#endif
}""",
        """void schick_set_video(void)
{
	struct uint8_t_3 pal_white = g_color_pal_white;

#if defined(__BORLANDC__)
	set_video_mode(0x13);
	set_color((uint8_t*)&pal_white, 0xff);
#else
	sdl_init_video();
	set_color((uint8_t*)&pal_white, 0xff);
#endif
}

void schick_reset_video(void)
{
#if defined(__BORLANDC__)
	set_video_mode(g_video_mode_bak);
	set_video_page(g_video_page_bak);
#else
	sdl_exit_video();
#endif
}""",
        "seg004.c:schick_set_video",
    )
    s4_txt = replace_once(
        s4_txt,
        """	count = y2 - y1 + 1;
	dst = y1 * 320 + x + ptr;

	draw_h_spaced_dots(dst, count, color, 320);
}""",
        """	count = y2 - y1 + 1;
	dst = y1 * 320 + x + ptr;

#if defined(__BORLANDC__)
	draw_h_spaced_dots(dst, count, color, 320);
#else
	{
		const int off = y1 * 320 + x;
		if ((off < 0) || (off >= 320 * 200)) return;
		draw_h_spaced_dots(dst, count, color, 320);
		sdl_forced_update();
	}
#endif
}""",
        "seg004.c:do_v_line",
    )
    s4_txt = replace_once(
        s4_txt,
        """	dst = g_pic_copy.dst;

	pic_copy(dst, x1, y1, x2, y2, v1, v2, v3, v4, width, height, src, mode);
}""",
        """	dst = g_pic_copy.dst;

	pic_copy(dst, x1, y1, x2, y2, v1, v2, v3, v4, width, height, src, mode);
#if !defined(__BORLANDC__)
	sdl_forced_update();
#endif
}""",
        "seg004.c:do_pic_copy",
    )
    s4_txt = replace_once(
        s4_txt,
        """#if defined(__BORLANDC__)
	fill_rect(FP_SEG(dst), FP_OFF(dst), color, width, height);
#else
	fill_rect(dst, color, width, height);
#endif""",
        """#if defined(__BORLANDC__)
	fill_rect(FP_SEG(dst), FP_OFF(dst), color, width, height);
#else
	vgalib_fill_rect(dst, color, width, height);
	sdl_forced_update();
#endif""",
        "seg004.c:do_fill_rect",
    )
    s4_txt = replace_once(
        s4_txt,
        """#else
	//SDL_Delay(16);
#endif""",
        """#else
	sdl_wait_vsync();
#endif""",
        "seg004.c:wait_for_vsync",
    )
    s4_p.write_text(s4_txt, encoding="latin-1")

    # seg005.c: g_buffer_xx und wait_for_vsync im Kampf
    s5_p = SRC_DIR / "seg005.c"
    s5_txt = s5_p.read_text(encoding="latin-1")
    s5_txt = replace_once(
        s5_txt,
        """extern char g_str_temp_xx[8];
static char *g_str_temp_xx_ptr = (char*)&g_str_temp_xx[0]; // ds:0x4b68, to STR_TEMP_XX; uint8_t*""",
        """#if defined(__BORLANDC__)
extern char g_str_temp_xx[8];
static char *g_str_temp_xx_ptr = (char*)&g_str_temp_xx[0]; // ds:0x4b68, to STR_TEMP_XX; uint8_t*
#endif""",
        "seg005.c:g_str_temp_xx_ptr",
    )
    s5_txt = replace_once(
        s5_txt,
        'char g_str_temp_xx[8] = "TEMP\\\\XX"; // ds:0x4b95',
        '#if defined(__BORLANDC__)\nchar g_str_temp_xx[8] = "TEMP\\\\XX"; // ds:0x4b95\n#else\nuint8_t *g_buffer_xx = NULL;\n#endif',
        "seg005.c:g_str_temp_xx",
    )
    s5_txt = replace_once(
        s5_txt,
        """	/* TODO: should be O_BINARY | O_WRONLY */
	handle = _creat(g_str_temp_xx_ptr, 0);
	write(handle, g_buffer8_ptr, 64000);
	close(handle);""",
        """#if defined(__BORLANDC__)
	/* TODO: should be O_BINARY | O_WRONLY */
	handle = _creat(g_str_temp_xx_ptr, 0);
	write(handle, g_buffer8_ptr, 64000);
	close(handle);
#else
	memcpy(g_buffer_xx, g_buffer8_ptr, 320 * 200);
#endif""",
        "seg005.c:write_temp_xx",
    )
    s5_txt = replace_once(
        s5_txt,
        """			/* REMARK: SPEEDFIX
			wait_for_vsync();
			*/
			if (fight_printer()) {""",
        """#if !defined(__BORLANDC__)
			wait_for_vsync();
#endif
			if (fight_printer()) {""",
        "seg005.c:fight_wait_vsync",
    )
    s5_txt = replace_once(
        s5_txt,
        """	/* read TEMP/XX */
	/* TODO: should be O_BINARY | O_RDONLY */
	handle = _open(g_str_temp_xx_ptr, 0);
	_read(handle, g_buffer8_ptr, 64000);
	close(handle);""",
        """#if defined(__BORLANDC__)
	/* read TEMP/XX */
	/* TODO: should be O_BINARY | O_RDONLY */
	handle = _open(g_str_temp_xx_ptr, 0);
	_read(handle, g_buffer8_ptr, 64000);
	close(handle);
#else
	memcpy(g_buffer8_ptr, g_buffer_xx, 320 * 200);
#endif""",
        "seg005.c:read_temp_xx",
    )
    s5_p.write_text(s5_txt, encoding="latin-1")

    # seg007.c: _rotl conflict with MinGW <stdlib.h> + guard against frozen g_random_schick_seed2 in tight loops
    s7_p = SRC_DIR / "seg007.c"
    s7_txt = s7_p.read_text(encoding="latin-1")
    s7_txt = replace_once(
        s7_txt,
        """#if !defined(__BORLANDC__)
static inline uint16_t _rotl(const uint16_t op, const uint8_t count)
{
	return (op << count) | (op >> (16 - count));
}
#endif""",
        """#if !defined(__BORLANDC__)
#undef _rotl
static inline uint16_t schick_rotl16(const uint16_t op, const uint8_t count)
{
	return (op << count) | (op >> (16 - count));
}
#define _rotl(op, count) schick_rotl16(op, count)
#endif""",
        "seg007.c:_rotl",
    )
    s7_txt = replace_once(
        s7_txt,
        """	/* rand_seed XOR rand_seed2 */
	retval = g_random_schick_seed ^ g_random_schick_seed2;""",
        """#if !defined(__BORLANDC__)
	{
		static uint16_t s_last_seed2 = 0xffff;
		static int s_same_seed2_calls = 0;
		if ((uint16_t)g_random_schick_seed2 == s_last_seed2) {
			if (++s_same_seed2_calls > 4) {
				g_random_schick_seed2++;
				s_last_seed2 = (uint16_t)g_random_schick_seed2;
				s_same_seed2_calls = 0;
			}
		} else {
			s_last_seed2 = (uint16_t)g_random_schick_seed2;
			s_same_seed2_calls = 1;
		}
	}
#endif
	/* rand_seed XOR rand_seed2 */
	retval = g_random_schick_seed ^ g_random_schick_seed2;""",
        "seg007.c:random_schick_seed2_guard",
    )
    s7_p.write_text(s7_txt, encoding="latin-1")

    # seg024.c: no pointer to gs_diary_entries[-1] when counter == 0
    s24_p = SRC_DIR / "seg024.c"
    s24_txt = s24_p.read_text(encoding="latin-1")
    s24_txt = replace_once(
        s24_txt,
        """	/* make a pointer to the last entry */
	/* Original-Bug (vermutlich ohne Folgen): bei gs_diary_entry_counter == 0 ist das gs_diary_entries[-1] */
	entry = &gs_diary_entries[gs_diary_entry_counter - 1];

	/* avoid double entries for the same town */
#if FIX_0287
	/* Fix 287: bei leerem Tagebuch nicht mit gs_diary_entries[-1] vergleichen */
	if ((gs_diary_entry_counter == 0) || (gs_town_id != entry->town_id)) {
#else
	if (gs_town_id != entry->town_id) {
#endif""",
        """	/* avoid double entries for the same town */
#if defined(__BORLANDC__)
	entry = &gs_diary_entries[gs_diary_entry_counter - 1];
#if FIX_0287
	if ((gs_diary_entry_counter == 0) || (gs_town_id != entry->town_id)) {
#else
	if (gs_town_id != entry->town_id) {
#endif
#else
	if ((gs_diary_entry_counter == 0) || (gs_town_id != gs_diary_entries[gs_diary_entry_counter - 1].town_id)) {
#endif""",
        "seg024.c:diary_new_entry",
    )
    s24_p.write_text(s24_txt, encoding="latin-1")

    # seg026.c: Native save/load (Windows + Linux), read_chr_temp, copy_chr_names
    s26_p = SRC_DIR / "seg026.c"
    s26_txt = s26_p.read_text(encoding="latin-1")
    s26_txt = replace_once(
        s26_txt,
        """#if defined(__BORLANDC__)
#include <DIR.H>
#include <DOS.H>
#include <IO.H>
#else
#include <unistd.h>
#endif""",
        """#if defined(__BORLANDC__)
#include <DIR.H>
#include <DOS.H>
#include <IO.H>
#else
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#if defined(_WIN32)
#include <io.h>
#else
#include <strings.h>
#define _stricmp strcasecmp
#endif
#endif""",
        "seg026.c:includes",
    )
    s26_txt = replace_once(
        s26_txt,
        'char g_chr_file_suffix[5] = ".CHR"; // ds:0x5e3e',
        'const char g_chr_file_suffix[5] = ".CHR"; // ds:0x5e3e',
        "seg026.c:g_chr_file_suffix",
    )

    native_saveload_code = """#if !defined(__BORLANDC__)
/* ---- Spielstaende nativ (Windows & Linux) ----
 * Setzt den 5952-Byte-Block ds:0x2d34..0x4474 ueber datseg_status.h zusammen,
 * sodass DOS- und Port-Spielstaende (.GAM) 100 % kompatibel sind. */

#define STATUS_VAR(off, len, var) STATIC_ASSERT(sizeof(var) == (len), size_mismatch_##var);
#define STATUS_DUMMY STATUS_VAR
#define STATUS_PTR(off, len, var) STATIC_ASSERT((len) == 4, ptr_size_mismatch_##var);
#define STATUS_SKIP(off, len, var)
#include "datseg_status.h"
#undef STATUS_VAR
#undef STATUS_DUMMY
#undef STATUS_PTR
#undef STATUS_SKIP

static void status_to_dos(uint8_t *buf)
{
	uint32_t hero_off;
	memset(buf, 0, STATUS_DOS_LENGTH);

#define STATUS_VAR(off, len, var) memcpy(buf + (off), &(var), (len));
#define STATUS_DUMMY STATUS_VAR
#define STATUS_PTR(off, len, var) \\
	hero_off = ((var) != NULL) ? (uint32_t)(((struct struct_hero*)(var) - g_heroes) * sizeof(struct struct_hero)) : 0; \\
	memcpy(buf + (off), &hero_off, 4);
#define STATUS_SKIP(off, len, var)
#include "datseg_status.h"
#undef STATUS_VAR
#undef STATUS_DUMMY
#undef STATUS_PTR
#undef STATUS_SKIP
}

static void status_from_dos(const uint8_t *buf)
{
	uint32_t hero_off;

#define STATUS_VAR(off, len, var) memcpy(&(var), buf + (off), (len));
#define STATUS_DUMMY STATUS_VAR
#define STATUS_PTR(off, len, var) \\
	memcpy(&hero_off, buf + (off), 4); \\
	(var) = (hero_off && (hero_off / sizeof(struct struct_hero) <= 6)) \\
		? (g_heroes + (hero_off / sizeof(struct struct_hero))) : NULL;
#define STATUS_SKIP(off, len, var)
#include "datseg_status.h"
#undef STATUS_VAR
#undef STATUS_DUMMY
#undef STATUS_PTR
#undef STATUS_SKIP
}

static int has_suffix_ci(const char *name, const char *suffix)
{
	const size_t nlen = strlen(name);
	const size_t slen = strlen(suffix);
	return (nlen >= slen) && (_stricmp(name + nlen - slen, suffix) == 0);
}

static signed int open_file_ci(const char *fname, int flags)
{
	signed int h = _open(fname, flags);
	DIR *dir;
	struct dirent *dp;

	if (h != -1) return h;

	dir = opendir(".");
	if (!dir) return -1;
	while ((dp = readdir(dir)) != NULL) {
		if (_stricmp(dp->d_name, fname) == 0) {
			h = _open(dp->d_name, flags);
			break;
		}
	}
	closedir(dir);
	return h;
}
#endif
"""

    s26_txt = replace_once(
        s26_txt,
        """signed int load_game_state(void)
{
#if defined(__BORLANDC__)""",
        native_saveload_code + """\nsigned int load_game_state(void)
{
#if 1""",
        "seg026.c:insert_native_saveload",
    )
    s26_txt = replace_once(
        s26_txt,
        """	signed char version[4];
	struct ffblk blk;
	char name[20];""",
        """	signed char version[4];
#if defined(__BORLANDC__)
	struct ffblk blk;
#endif
	char name[20];""",
        "seg026.c:load_ffblk",
    )
    s26_txt = replace_once(
        s26_txt,
        """#if defined(__BORLANDC__)
		done = findfirst(g_text_output_buf, &blk, 0);

		if (done == 0) {

			do {
				/* (&blk + 30 = blk.ff_name) */
				sprintf(g_text_output_buf, g_str_temp_fmt_ptr, ((char*)(&blk))+ 30);
				unlink(g_text_output_buf);

				done = findnext(&blk);

			} while (done == 0);
		}
#endif""",
        """#if defined(__BORLANDC__)
		done = findfirst(g_text_output_buf, &blk, 0);

		if (done == 0) {

			do {
				/* (&blk + 30 = blk.ff_name) */
				sprintf(g_text_output_buf, g_str_temp_fmt_ptr, ((char*)(&blk))+ 30);
				unlink(g_text_output_buf);

				done = findnext(&blk);

			} while (done == 0);
		}
#else
		{
			DIR *dir = opendir("TEMP");
			struct dirent *dp;
			if (dir != NULL) {
				while ((dp = readdir(dir)) != NULL) {
					if (dp->d_name[0] == '.') continue;
					sprintf(g_text_output_buf, g_str_temp_fmt_ptr, dp->d_name);
					unlink(g_text_output_buf);
				}
				closedir(dir);
			}
		}
#endif""",
        "seg026.c:load_clean_temp",
    )
    s26_txt = replace_once(
        s26_txt,
        """		/* liest den 4-Byte-Verweis auf die CHR-Daten; er wird gleich darauf ueberschrieben, also nicht benutzt */
		_read(handle_sg, &gs_datseg_status_start, 4);

		/* read game status */
		/* TODO: check pointer arithmetics work with other pointers */
		p_status_start = (HugePt)&gs_datseg_status_start;
		p_status_end = (HugePt)&gs_datseg_status_end;
		status_length = p_status_end - p_status_start;	/* REMARK: pointer arithmetic */

		_read(handle_sg, &gs_datseg_status_start, status_length);""",
        """#if defined(__BORLANDC__)
		/* liest den 4-Byte-Verweis auf die CHR-Daten; er wird gleich darauf ueberschrieben, also nicht benutzt */
		_read(handle_sg, &gs_datseg_status_start, 4);

		/* read game status */
		/* TODO: check pointer arithmetics work with other pointers */
		p_status_start = (HugePt)&gs_datseg_status_start;
		p_status_end = (HugePt)&gs_datseg_status_end;
		status_length = p_status_end - p_status_start;	/* REMARK: pointer arithmetic */

		_read(handle_sg, &gs_datseg_status_start, status_length);
#else
		{
			uint32_t dummy_pos;
			uint8_t *status_buf = (uint8_t*)calloc(STATUS_DOS_LENGTH, 1);
			_read(handle_sg, (uint8_t*)&dummy_pos, 4);
			status_length = STATUS_DOS_LENGTH;
			if (status_buf != NULL) {
				_read(handle_sg, status_buf, status_length);
				status_from_dos(status_buf);
				free(status_buf);
			}
		}
#endif""",
        "seg026.c:load_status_block",
    )
    s26_txt = replace_once(
        s26_txt,
        """#if defined(__BORLANDC__)
		/* search for "*.CHR" */
		done = findfirst(g_all_chr_wildcard, &blk, 0);

		while (done == 0) {

			sprintf(g_text_output_buf, g_str_temp_fmt_ptr, ((char*)(&blk)) + 30);

			if ((handle_sg = open(g_text_output_buf, O_BINARY | O_RDWR)) == -1) {
				handle = open(blk.ff_name, O_BINARY | O_RDWR);
				_read(handle, g_renderbuf_ptr, sizeof(struct struct_hero));
				close(handle);

				/* TODO: should be O_BINARY | O_WRONLY */
				handle_sg = _creat(g_text_output_buf, 0);
				write(handle_sg, g_renderbuf_ptr, sizeof(struct struct_hero));
			} else {
				/* Yes, indeed! */
			}

			close(handle_sg);

			done = findnext(&blk);
		}
#endif""",
        """#if defined(__BORLANDC__)
		/* search for "*.CHR" */
		done = findfirst(g_all_chr_wildcard, &blk, 0);

		while (done == 0) {

			sprintf(g_text_output_buf, g_str_temp_fmt_ptr, ((char*)(&blk)) + 30);

			if ((handle_sg = open(g_text_output_buf, O_BINARY | O_RDWR)) == -1) {
				handle = open(blk.ff_name, O_BINARY | O_RDWR);
				_read(handle, g_renderbuf_ptr, sizeof(struct struct_hero));
				close(handle);

				/* TODO: should be O_BINARY | O_WRONLY */
				handle_sg = _creat(g_text_output_buf, 0);
				write(handle_sg, g_renderbuf_ptr, sizeof(struct struct_hero));
			} else {
				/* Yes, indeed! */
			}

			close(handle_sg);

			done = findnext(&blk);
		}
#else
		{
			DIR *dir = opendir(".");
			struct dirent *dp;
			if (dir != NULL) {
				while ((dp = readdir(dir)) != NULL) {
					if (!has_suffix_ci(dp->d_name, g_chr_file_suffix)) continue;
					sprintf(g_text_output_buf, g_str_temp_fmt_ptr, dp->d_name);
					if ((handle_sg = _open(g_text_output_buf, O_BINARY | O_RDWR)) == -1) {
						handle = _open(dp->d_name, O_BINARY | O_RDWR);
						if (handle != -1) {
							long flen = lseek(handle, 0, SEEK_END);
							lseek(handle, 0, SEEK_SET);
							if ((flen == (long)sizeof(struct struct_hero)) || (flen == (long)sizeof(struct struct_hero) - 16)) {
								handle_sg = _creat(g_text_output_buf, 0);
								if (flen == (long)sizeof(struct struct_hero)) {
									_read(handle, g_renderbuf_ptr, sizeof(struct struct_hero));
								} else {
									_read(handle, g_renderbuf_ptr + 16, sizeof(struct struct_hero) - 16);
									memcpy(g_renderbuf_ptr, g_renderbuf_ptr + 16, 16);
								}
								write(handle_sg, g_renderbuf_ptr, sizeof(struct struct_hero));
								close(handle_sg);
							}
							close(handle);
						}
					} else {
						close(handle_sg);
					}
				}
				closedir(dir);
			}
		}
#endif""",
        "seg026.c:load_chr_scan",
    )
    s26_txt = replace_once(
        s26_txt,
        """	return retval;
#else
	return 0;
#endif
}

#if FIX_SPEICHERN""",
        """	return retval;
#endif
}

#if FIX_SPEICHERN""",
        "seg026.c:load_game_state_end",
    )
    s26_txt = replace_once(
        s26_txt,
        """signed int save_game_state(void)
{
#if defined(__BORLANDC__)""",
        """signed int save_game_state(void)
{
#if 1""",
        "seg026.c:save_game_state_start",
    )
    s26_txt = replace_once(
        s26_txt,
        """	uint32_t len;
	struct ffblk blk;""",
        """	uint32_t len;
#if defined(__BORLANDC__)
	struct ffblk blk;
#endif""",
        "seg026.c:save_ffblk",
    )
    s26_txt = replace_once(
        s26_txt,
        """		/* TODO: check if pointer arithmetics work with other pointers */
		p_status_start = (HugePt)&gs_datseg_status_start;
		p_status_end = (HugePt)&gs_datseg_status_end;
		status_length = p_status_end - p_status_start;""",
        """#if defined(__BORLANDC__)
		/* TODO: check if pointer arithmetics work with other pointers */
		p_status_start = (HugePt)&gs_datseg_status_start;
		p_status_end = (HugePt)&gs_datseg_status_end;
		status_length = p_status_end - p_status_start;
#else
		p_status_start = NULL;
		p_status_end = NULL;
		status_length = STATUS_DOS_LENGTH;
#endif""",
        "seg026.c:save_status_len",
    )
    s26_txt = replace_once(
        s26_txt,
        """		/* save the status section 5952 bytes */
		filepos += write(handle_sg, p_status_start, status_length);""",
        """		/* save the status section 5952 bytes */
#if defined(__BORLANDC__)
		filepos += write(handle_sg, p_status_start, status_length);
#else
		{
			uint8_t *status_buf = (uint8_t*)calloc(STATUS_DOS_LENGTH, 1);
			if (status_buf != NULL) {
				status_to_dos(status_buf);
				filepos += write(handle_sg, status_buf, status_length);
				free(status_buf);
			}
		}
#endif""",
        "seg026.c:save_status_write",
    )
    s26_txt = replace_once(
        s26_txt,
        """			sprintf(g_text_output_buf, g_str_temp_fmt_ptr, g_fnames_v302de[tw_bak]);

			done = findfirst(g_text_output_buf, &blk, 0);""",
        """			sprintf(g_text_output_buf, g_str_temp_fmt_ptr, g_fnames_v302de[tw_bak]);

#if defined(__BORLANDC__)
			done = findfirst(g_text_output_buf, &blk, 0);
#else
			{
				struct stat st;
				done = (stat(g_text_output_buf, &st) == 0 && S_ISREG(st.st_mode)) ? 0 : -1;
			}
#endif""",
        "seg026.c:save_findfirst_temp",
    )
    s26_txt = replace_once(
        s26_txt,
        """		/* append all CHR files */
		lseek(handle_sg, filepos, 0);
		sprintf(g_text_output_buf, g_str_temp_fmt_ptr, g_all_chr_wildcard2);

		done = findfirst(g_text_output_buf, &blk, 0);
		/* Original-Bug (latent): do-while ohne Pruefung von done; findet findfirst nichts,
		 * wird einmal mit ungueltigem blk.ff_name gelesen. Praktisch gibt es immer CHR-Dateien. */
#if FIX_0289
		/* Fix 289: nur anhaengen, wenn findfirst() etwas gefunden hat */
		if (done == 0)
#endif
		do {
			/* create the CHR filename */
			sprintf(g_text_output_buf, g_str_temp_fmt_ptr, blk.ff_name);

			/* read the CHR file from temp */
			handle = open(g_text_output_buf, O_BINARY | O_RDWR);
			_read(handle, g_renderbuf_ptr, sizeof(struct struct_hero));
			close(handle);

			/* append it */
			len = write(handle_sg, g_renderbuf_ptr, sizeof(struct struct_hero));

			if (len != sizeof(struct struct_hero)) {

				GUI_output(get_ttx(348)); /* "SPEICHERFEHLER!@EVENTUELL DISKETTE VOLL?@FUNKTION WIRD ABGEBROCHEN!" */
				close(handle_sg);
				return 0;
			}

			done = findnext(&blk);

		} while (done == 0);""",
        """		/* append all CHR files */
		lseek(handle_sg, filepos, 0);
#if defined(__BORLANDC__)
		sprintf(g_text_output_buf, g_str_temp_fmt_ptr, g_all_chr_wildcard2);

		done = findfirst(g_text_output_buf, &blk, 0);
		/* Original-Bug (latent): do-while ohne Pruefung von done; findet findfirst nichts,
		 * wird einmal mit ungueltigem blk.ff_name gelesen. Praktisch gibt es immer CHR-Dateien. */
#if FIX_0289
		/* Fix 289: nur anhaengen, wenn findfirst() etwas gefunden hat */
		if (done == 0)
#endif
		do {
			/* create the CHR filename */
			sprintf(g_text_output_buf, g_str_temp_fmt_ptr, blk.ff_name);

			/* read the CHR file from temp */
			handle = open(g_text_output_buf, O_BINARY | O_RDWR);
			_read(handle, g_renderbuf_ptr, sizeof(struct struct_hero));
			close(handle);

			/* append it */
			len = write(handle_sg, g_renderbuf_ptr, sizeof(struct struct_hero));

			if (len != sizeof(struct struct_hero)) {

				GUI_output(get_ttx(348)); /* "SPEICHERFEHLER!@EVENTUELL DISKETTE VOLL?@FUNKTION WIRD ABGEBROCHEN!" */
				close(handle_sg);
				return 0;
			}

			done = findnext(&blk);

		} while (done == 0);
#else
		{
			DIR *dir = opendir("TEMP");
			struct dirent *dp;
			if (dir != NULL) {
				while ((dp = readdir(dir)) != NULL) {
					if (!has_suffix_ci(dp->d_name, g_chr_file_suffix)) continue;
					sprintf(g_text_output_buf, g_str_temp_fmt_ptr, dp->d_name);
					handle = _open(g_text_output_buf, O_BINARY | O_RDWR);
					if (handle != -1) {
						_read(handle, g_renderbuf_ptr, sizeof(struct struct_hero));
						close(handle);
						len = write(handle_sg, g_renderbuf_ptr, sizeof(struct struct_hero));
						if (len != sizeof(struct struct_hero)) {
							GUI_output(get_ttx(348));
							closedir(dir);
							close(handle_sg);
							return 0;
						}
					}
				}
				closedir(dir);
			}
		}
#endif""",
        "seg026.c:save_append_chr",
    )

    # read_chr_temp & copy_chr_names in seg026.c
    s26_txt = replace_once(
        s26_txt,
        """signed int read_chr_temp(char *fname, const signed int hero_pos, const signed int group_id)
{
#if defined(__BORLANDC__)""",
        """signed int read_chr_temp(char *fname, const signed int hero_pos, const signed int group_id)
{
#if 1""",
        "seg026.c:read_chr_temp",
    )

    s26_txt = replace_once(
        s26_txt,
        """signed int copy_chr_names(char *ptr, const signed int temple_id)
{
#if defined(__BORLANDC__)""",
        """signed int copy_chr_names(char *ptr, const signed int temple_id)
{
#if !defined(__BORLANDC__)
	signed int count = 0;
	DIR *dir = opendir("TEMP");
	struct dirent *dp;
	char path[260];
	struct struct_hero *hero = (struct struct_hero*)g_renderbuf_ptr;

	if (dir != NULL) {
		while ((dp = readdir(dir)) != NULL) {
			if (!has_suffix_ci(dp->d_name, g_chr_file_suffix)) continue;
			snprintf(path, sizeof(path), "TEMP/%s", dp->d_name);
			{
				signed int handle = _open(path, O_BINARY | O_RDONLY);
				if (handle != -1) {
					_read(handle, hero, sizeof(struct struct_hero));
					close(handle);
					if (((hero->temple_id == temple_id) && !hero->slot_pos) ||
						(!hero->slot_pos && (temple_id == -1)))
					{
						strcpy((char*)ptr + 32 * count, hero->name);
						strcpy((char*)ptr + 32 * count + 16, hero->alias);
						count++;
					}
				}
			}
		}
		closedir(dir);
	}
	return count;
#elif defined(__BORLANDC__)""",
        "seg026.c:copy_chr_names",
    )
    s26_p.write_text(s26_txt, encoding="latin-1")

    # seg027.c: scenarios_num & fight_count 16-bit reads, disable EMS memslots in load_ani
    s27_p = SRC_DIR / "seg027.c"
    s27_txt = s27_p.read_text(encoding="latin-1")
    s27_txt = replace_once(
        s27_txt,
        """	/* clear the old ani */
	clear_ani();

	/* count to the ordered ani in an array */
	for (i = 0; i < 37; i++) {
		if (ani_id == g_memslots_anis[i].figure)
			break;
	}""",
        """	/* clear the old ani */
	clear_ani();

#if defined(__BORLANDC__)
	/* count to the ordered ani in an array */
	for (i = 0; i < 37; i++) {
		if (ani_id == g_memslots_anis[i].figure)
			break;
	}
#else
	i = 37;
#endif""",
        "seg027.c:load_ani",
    )
    s27_txt = replace_once(
        s27_txt,
        """void load_scenario(signed int scenario_id)
{
	signed int handle;
	signed int scenarios_num;""",
        """void load_scenario(signed int scenario_id)
{
	signed int handle;
	int16_t scenarios_num = 0;""",
        "seg027.c:scenarios_num",
    )
    s27_txt = replace_once(
        s27_txt,
        """signed int count_fight_enemies(signed int fight_id)
{
	signed int enemy_i;
	signed int enemy_count;
	struct fight *fight_lst_buf;
	signed int handle;
	signed int fight_count;""",
        """signed int count_fight_enemies(signed int fight_id)
{
	signed int enemy_i;
	signed int enemy_count;
	struct fight *fight_lst_buf;
	signed int handle;
	int16_t fight_count = 0;""",
        "seg027.c:fight_count_1",
    )
    s27_txt = replace_once(
        s27_txt,
        """void read_fight_lst(signed int fight_id)
{
	signed int handle;
	signed int fight_count;""",
        """void read_fight_lst(signed int fight_id)
{
	signed int handle;
	int16_t fight_count = 0;""",
        "seg027.c:fight_count_2",
    )
    s27_p.write_text(s27_txt, encoding="latin-1")

    # seg028.c: partners 16-bit read in load_informer_tlk & load_tlk
    s28_p = SRC_DIR / "seg028.c"
    s28_txt = s28_p.read_text(encoding="latin-1")
    s28_txt = replace_once(
        s28_txt,
        """void load_informer_tlk(const signed int index)
{
	signed int i;
	signed int handle;

	int32_t text_len;
	int32_t off;
	signed int partners;""",
        """void load_informer_tlk(const signed int index)
{
	signed int i;
	signed int handle;

	int32_t text_len;
	int32_t off;
	int16_t partners = 0;""",
        "seg028.c:load_informer_tlk_partners",
    )
    s28_txt = replace_once(
        s28_txt,
        """void load_tlk(const signed int index)
{
	signed int i;
	signed int handle;
	int32_t text_len;
	int32_t off;
	signed int partners;""",
        """void load_tlk(const signed int index)
{
	signed int i;
	signed int handle;
	int32_t text_len;
	int32_t off;
	int16_t partners = 0;""",
        "seg028.c:load_tlk_partners",
    )
    s28_p.write_text(s28_txt, encoding="latin-1")

    # seg029.c: bounds check in draw_status_line when !FIX_0043
    s29_p = SRC_DIR / "seg029.c"
    s29_txt = s29_p.read_text(encoding="latin-1")
    s29_txt = replace_once(
        s29_txt,
        """#if FIX_0043
			/* Fix 43: das Feld hat 4 Werte je Held (sonst wird beim NSC g_disease_strength ueberschrieben) */
			for (j = 0; j < 4; j++) {
#else
			for (j = 0; j < 6; j++) {
#endif""",
        """#if FIX_0043 || !defined(__BORLANDC__)
			/* Fix 43 / nativ: das Feld hat 4 Werte je Held */
			for (j = 0; j < 4; j++) {
#else
			for (j = 0; j < 6; j++) {
#endif""",
        "seg029.c:draw_status_line",
    )
    s29_p.write_text(s29_txt, encoding="latin-1")

    # seg030.c & seg031.c: states_offset relative to gs_dialog_states
    s30_p = SRC_DIR / "seg030.c"
    s30_txt = s30_p.read_text(encoding="latin-1")
    s30_txt = replace_once(
        s30_txt,
        "\tpartners_tab = &gs_dialog_partners[0];\n\tstates_tab = (uint8_t*)partners_tab[tlk_informer].states_offset;",
        "\tpartners_tab = &gs_dialog_partners[0];\n#if defined(__BORLANDC__)\n\tstates_tab = (uint8_t*)partners_tab[tlk_informer].states_offset;\n#else\n\tstates_tab = (uint8_t*)&gs_dialog_states + (uint16_t)partners_tab[tlk_informer].states_offset;\n#endif",
        "seg030.c:states_offset",
    )
    s30_p.write_text(s30_txt, encoding="latin-1")

    s31_p = SRC_DIR / "seg031.c"
    s31_txt = s31_p.read_text(encoding="latin-1")
    s31_txt = replace_once(
        s31_txt,
        "\tpartners_tab = &gs_dialog_partners[0];\n\tstates_tab = (struct struct_dialog_state*)partners_tab[informer_id].states_offset;",
        "\tpartners_tab = &gs_dialog_partners[0];\n#if defined(__BORLANDC__)\n\tstates_tab = (struct struct_dialog_state*)partners_tab[informer_id].states_offset;\n#else\n\tstates_tab = (struct struct_dialog_state*)((uint8_t*)&gs_dialog_states + (uint16_t)partners_tab[informer_id].states_offset);\n#endif",
        "seg031.c:states_offset",
    )
    s31_p.write_text(s31_txt, encoding="latin-1")

    # seg035.c: check bound loot_num < 30 first
    s35_p = SRC_DIR / "seg035.c"
    s35_txt = s35_p.read_text(encoding="latin-1")
    s35_txt = replace_once(
        s35_txt,
        "while ((item_id = g_current_fight->loot[loot_num]) && (loot_num < 30) && (item_id != ITEM_ID_KNOCHEN_MIT_RUNE))",
        "#if defined(__BORLANDC__)\n\t\twhile ((item_id = g_current_fight->loot[loot_num]) && (loot_num < 30) && (item_id != ITEM_ID_KNOCHEN_MIT_RUNE))\n#else\n\t\twhile ((loot_num < 30) && (item_id = g_current_fight->loot[loot_num]) && (item_id != ITEM_ID_KNOCHEN_MIT_RUNE))\n#endif",
        "seg035.c:loot_bound",
    )
    s35_p.write_text(s35_txt, encoding="latin-1")

    # seg063.c: sea_travel course check
    s63_p = SRC_DIR / "seg063.c"
    s63_txt = s63_p.read_text(encoding="latin-1")
    s63_txt = replace_once(
        s63_txt,
        "\t\t} while (gs_travel_course_ptr[0] != -1);",
        "#if defined(__BORLANDC__)\n\t\t} while (gs_travel_course_ptr[0] != -1);\n#else\n\t\t} while ((gs_travel_course_ptr[0] != -1) && (gs_travel_step_counter != 0));\n#endif",
        "seg063.c:sea_travel",
    )
    s63_p.write_text(s63_txt, encoding="latin-1")

    # seg066.h & seg066.c: get_town_tile_type prototype, load_town_texture bounds check & town_fade_and_colors 3 palettes
    s66h_p = SRC_DIR / "seg066.h"
    s66h_txt = s66h_p.read_text(encoding="latin-1")
    s66h_txt = replace_once(
        s66h_txt,
        "signed int get_town_tile_type();",
        "#if defined(__BORLANDC__)\nsigned int get_town_tile_type();\n#else\nsigned int get_town_tile_type(const unsigned char);\n#endif",
        "seg066.h:get_town_tile_type",
    )
    s66h_p.write_text(s66h_txt, encoding="latin-1")

    s66_p = SRC_DIR / "seg066.c"
    s66_txt = s66_p.read_text(encoding="latin-1")
    s66_txt = replace_once(
        s66_txt,
        """#if FIX_HIMMEL
#include <io.h>
#include <fcntl.h>
#endif""",
        """#if FIX_HIMMEL
#if defined(__BORLANDC__) || defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif
#include <fcntl.h>
#endif""",
        "seg066.c:io_h",
    )
    s66_txt = replace_once(
        s66_txt,
        """	nvf.src = g_town_tex[tex_set];

	if (tex_set == 48 || tex_set == 49) {
		nvf.src = (uint8_t*)g_buffer7_ptr;
	}""",
        """#if defined(__BORLANDC__)
	nvf.src = g_town_tex[tex_set];

	if (tex_set == 48 || tex_set == 49) {
		nvf.src = (uint8_t*)g_buffer7_ptr;
	}
#else
	if (tex_set == 48 || tex_set == 49) {
		nvf.src = (uint8_t*)g_buffer7_ptr;
	} else if ((0 <= tex_set) && (tex_set <= 5)) {
		nvf.src = g_town_tex[tex_set];
	} else {
		nvf.src = g_town_tex[0];
	}
#endif""",
        "seg066.c:load_town_texture",
    )
    s66_txt = replace_once(
        s66_txt,
        """#if defined(__BORLANDC__)
		/* 0x120 = gs_palette_floor, gs_palette_buildings and gs_palette_sky, which lie one after another */
		memset(g_renderbuf_ptr, 0, 0x120);
		memcpy(dst, gs_palette_floor, 0x120);
#else
		memset(g_renderbuf_ptr, 0, 3 * 0x20);
		memcpy(dst, gs_palette_floor, 3 * 0x20);
#endif

		for (i = 0; i < 64; i += 2) {

			pal_fade_in((int8_t*)pal_ptr, (int8_t*)dst, i, 0x60);
			pal_fade_in((int8_t*)pal_ptr, (int8_t*)dst, i + 1, 0x60);

			wait_for_vsync();

			set_palette(pal_ptr, 0x00, 0x20);
			set_palette(pal_ptr + 0x60, 0x80, 0x40);
		}

		g_fading_state = 0;

	} else {

		wait_for_vsync();

		set_palette(gs_palette_floor, 0x00, 0x20);
		set_palette(gs_palette_buildings, 0x80, 0x40);
	}""",
        """#if defined(__BORLANDC__)
		/* 0x120 = gs_palette_floor, gs_palette_buildings and gs_palette_sky, which lie one after another */
		memset(g_renderbuf_ptr, 0, 0x120);
		memcpy(dst, gs_palette_floor, 0x120);
#else
		memset(g_renderbuf_ptr, 0, 3 * 0x60);
		memcpy(dst, gs_palette_floor, 3 * 0x20);
		memcpy(dst + 3 * 0x20, gs_palette_buildings, 3 * 0x20);
		memcpy(dst + 3 * 0x40, gs_palette_sky, 3 * 0x20);
#endif

		for (i = 0; i < 64; i += 2) {

			pal_fade_in((int8_t*)pal_ptr, (int8_t*)dst, i, 0x60);
			pal_fade_in((int8_t*)pal_ptr, (int8_t*)dst, i + 1, 0x60);

			wait_for_vsync();

			set_palette(pal_ptr, 0x00, 0x20);
			set_palette(pal_ptr + 0x60, 0x80, 0x40);
		}

		g_fading_state = 0;

	} else {

		wait_for_vsync();

		set_palette(gs_palette_floor, 0x00, 0x20);
#if defined(__BORLANDC__)
		set_palette(gs_palette_buildings, 0x80, 0x40);
#else
		set_palette(gs_palette_buildings, 0x80, 0x20);
		set_palette(gs_palette_sky, 0x80 + 0x20, 0x20);
#endif
	}""",
        "seg066.c:town_fade_and_colors",
    )
    s66_p.write_text(s66_txt, encoding="latin-1")

    # seg074.c: g_palette_fight2 size 32 natively
    s74_p = SRC_DIR / "seg074.c"
    s74_txt = s74_p.read_text(encoding="latin-1")
    s74_txt = replace_once(
        s74_txt,
        """const uint8_t g_palette_fight2[20][3] = {
	{ 0x00, 0x00, 0x00 },
	{ 0x3f, 0x00, 0x00 },
	{ 0x00, 0x3f, 0x00 },
	{ 0x00, 0x00, 0x3f },
	{ 0x3f, 0x3f, 0x00 },
	{ 0x3f, 0x00, 0x3f },
	{ 0x00, 0x3f, 0x3f },
	{ 0x3f, 0x1f, 0x1f },
	{ 0x1f, 0x1f, 0x3f },
	{ 0x0f, 0x1f, 0x0f },
	{ 0x0f, 0x0f, 0x1f },
	{ 0x20, 0x05, 0x00 },
	{ 0x3f, 0x1f, 0x00 },
	{ 0x3f, 0x00, 0x1f },
	{ 0x28, 0x22, 0x00 },
	{ 0x1f, 0x1f, 0x1f },
	{ 0x20, 0x00, 0x33 },
	{ 0x27, 0x13, 0x00 },
	{ 0x00, 0x30, 0x00 },
	{ 0x0a, 0x0a, 0x0a }
}; // ds:0x7d0e""",
        """#if defined(__BORLANDC__)
const uint8_t g_palette_fight2[20][3] = {
#else
const uint8_t g_palette_fight2[32][3] = {
#endif
	{ 0x00, 0x00, 0x00 },
	{ 0x3f, 0x00, 0x00 },
	{ 0x00, 0x3f, 0x00 },
	{ 0x00, 0x00, 0x3f },
	{ 0x3f, 0x3f, 0x00 },
	{ 0x3f, 0x00, 0x3f },
	{ 0x00, 0x3f, 0x3f },
	{ 0x3f, 0x1f, 0x1f },
	{ 0x1f, 0x1f, 0x3f },
	{ 0x0f, 0x1f, 0x0f },
	{ 0x0f, 0x0f, 0x1f },
	{ 0x20, 0x05, 0x00 },
	{ 0x3f, 0x1f, 0x00 },
	{ 0x3f, 0x00, 0x1f },
	{ 0x28, 0x22, 0x00 },
	{ 0x1f, 0x1f, 0x1f },
	{ 0x20, 0x00, 0x33 },
	{ 0x27, 0x13, 0x00 },
	{ 0x00, 0x30, 0x00 },
	{ 0x0a, 0x0a, 0x0a }
}; // ds:0x7d0e""",
        "seg074.c:g_palette_fight2",
    )
    s74_p.write_text(s74_txt, encoding="latin-1")

    # seg075.c: copy all 3 palettes when fading out before Zwingfeste
    s75_p = SRC_DIR / "seg075.c"
    s75_txt = s75_p.read_text(encoding="latin-1")
    s75_txt = replace_once(
        s75_txt,
        """#if defined(__BORLANDC__)
		memset(g_renderbuf_ptr, 0, 0x120);
		memcpy(g_renderbuf_ptr + 500, gs_palette_floor, 0x120);
#else
		memset(g_renderbuf_ptr, 0, 3 * 0x20);
		memcpy(g_renderbuf_ptr + 500, gs_palette_floor, 3 * 0x20);
#endif""",
        """#if defined(__BORLANDC__)
		memset(g_renderbuf_ptr, 0, 0x120);
		memcpy(g_renderbuf_ptr + 500, gs_palette_floor, 0x120);
#else
		memset(g_renderbuf_ptr, 0, 3 * 0x60);
		memcpy(g_renderbuf_ptr + 500, gs_palette_floor, 3 * 0x20);
		memcpy(g_renderbuf_ptr + 500 + 3 * 0x20, gs_palette_buildings, 3 * 0x20);
		memcpy(g_renderbuf_ptr + 500 + 3 * 0x40, gs_palette_sky, 3 * 0x20);
#endif""",
        "seg075.c:fading_palettes",
    )
    s75_p.write_text(s75_txt, encoding="latin-1")

    # seg094.c: 16-bit vs 32-bit int read of town y coordinate
    s94_p = SRC_DIR / "seg094.c"
    s94_txt = s94_p.read_text(encoding="latin-1")
    s94_txt = replace_once(
        s94_txt,
        "\ty = g_current_town_aniy = *(signed int*)((uint8_t*)g_town_positions + 4 * (town_id - 1) + 2); /* = g_town_positions[town_id - 1].y */",
        "#if defined(__BORLANDC__)\n\ty = g_current_town_aniy = *(signed int*)((uint8_t*)g_town_positions + 4 * (town_id - 1) + 2);\n#else\n\ty = g_current_town_aniy = g_town_positions[town_id - 1].y;\n#endif",
        "seg094.c:town_y",
    )
    s94_p.write_text(s94_txt, encoding="latin-1")

    # seg096.c: sdl_forced_update at end of GUI_print_string
    s96_p = SRC_DIR / "seg096.c"
    s96_txt = s96_p.read_text(encoding="latin-1")
    s96_txt = replace_once(
        s96_txt,
        """	call_mouse();
}

//7f0""",
        """	call_mouse();
#if !defined(__BORLANDC__)
	sdl_forced_update();
#endif
}

//7f0""",
        "seg096.c:GUI_print_string",
    )
    s96_p.write_text(s96_txt, encoding="latin-1")

    # seg100.c: enable FIX_0205 in native branch too
    s100_p = SRC_DIR / "seg100.c"
    s100_txt = s100_p.read_text(encoding="latin-1")
    s100_txt = replace_once(
        s100_txt,
        """#if defined(__BORLANDC__)
			sprintf(g_dtp2, get_tx(89)); /* "TIERE HABEN KEINE WAFFEN, DIE MAN ZERBRECHEN LASSEN KANN!" */
			/* g_spell_special_aecost bleibt -1: volle Kosten, obwohl nichts passiert
			 * (Bannbaladin setzt in diesem Fall 0) */
#if FIX_0205
			/* Fix 205: keine Kosten, wie bei Bannbaladin */
			g_spell_special_aecost = 0;
#endif
#else
			strcpy(g_dtp2, get_tx(89)); /* "TIERE HABEN KEINE WAFFEN, DIE MAN ZERBRECHEN LASSEN KANN!" */
#endif""",
        """#if defined(__BORLANDC__)
			sprintf(g_dtp2, get_tx(89)); /* "TIERE HABEN KEINE WAFFEN, DIE MAN ZERBRECHEN LASSEN KANN!" */
#else
			strcpy(g_dtp2, get_tx(89)); /* "TIERE HABEN KEINE WAFFEN, DIE MAN ZERBRECHEN LASSEN KANN!" */
#endif
#if FIX_0205
			/* Fix 205: keine Kosten, wie bei Bannbaladin */
			g_spell_special_aecost = 0;
#endif""",
        "seg100.c:FIX_0205",
    )
    s100_p.write_text(s100_txt, encoding="latin-1")

    # seg106.c: check bound i < 4 first in startup_equipment
    s106_p = SRC_DIR / "seg106.c"
    s106_txt = s106_p.read_text(encoding="latin-1")
    s106_txt = replace_once(
        s106_txt,
        "\twhile ((g_hero_startup_items[hero->typus - 1][i] != -1) && (i < 4)) {",
        "#if defined(__BORLANDC__)\n\twhile ((g_hero_startup_items[hero->typus - 1][i] != -1) && (i < 4)) {\n#else\n\twhile ((i < 4) && (g_hero_startup_items[hero->typus - 1][i] != -1)) {\n#endif",
        "seg106.c:startup_equipment",
    )
    s106_p.write_text(s106_txt, encoding="latin-1")

    # seg120.c: init_memory (g_buffer_xx), prepare_dirs (Windows & Linux + Spielpfad), cleanup_game
    s120_p = SRC_DIR / "seg120.c"
    s120_txt = s120_p.read_text(encoding="latin-1")
    s120_txt = replace_once(
        s120_txt,
        """#if defined(__BORLANDC__)
#include <ALLOC.H>
#include <CONIO.H>
#include <DIR.H>
#include <DOS.H>
#include <IO.H>
#include <PROCESS.H>
#else
#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif
#endif""",
        """#if defined(__BORLANDC__)
#include <ALLOC.H>
#include <CONIO.H>
#include <DIR.H>
#include <DOS.H>
#include <IO.H>
#include <PROCESS.H>
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#if defined(_WIN32)
#include <direct.h>
#include <io.h>
#define MKDIR_PORT(p) _mkdir(p)
#define _stricmp_port _stricmp
#else
#include <strings.h>
#define MKDIR_PORT(p) mkdir(p, 0755)
#define _stricmp_port strcasecmp
#endif
#include "dsa_platform.h"
#endif""",
        "seg120.c:includes",
    )
    s120_txt = replace_once(
        s120_txt,
        'static const char g_str_backslash_temp[6] = "\\\\TEMP"; // ds:0xb311',
        '#if defined(__BORLANDC__)\nstatic const char g_str_backslash_temp[6] = "\\\\TEMP"; // ds:0xb311\n#else\nconst char g_str_temp_dir[] = "TEMP"; // ds:0xb311\n#endif',
        "seg120.c:g_str_temp_dir",
    )
    s120_txt = replace_once(
        s120_txt,
        """#else
	g_vga_memstart = (unsigned char*)calloc(320 * 200, sizeof(unsigned char));
	g_vga_backbuffer = g_vga_memstart;

#endif""",
        """#else
	g_vga_memstart = (unsigned char*)calloc(320 * 200, sizeof(unsigned char));
	g_vga_backbuffer = g_vga_memstart;
	g_buffer_xx = (uint8_t*)calloc(320 * 200, sizeof(uint8_t));
	if ((g_vga_memstart == NULL) || (g_buffer_xx == NULL)) error = 1;
#endif""",
        "seg120.c:init_memory_vga",
    )
    s120_txt = replace_once(
        s120_txt,
        """	setdisk(drive_bak);
#endif
}""",
        """	setdisk(drive_bak);
#else
	{
		const char *game_dir = dsa_platform_get_game_dir();
		DIR *dir;
		struct dirent *ent;
		char path[260];

		/* Falls SCHICK.DAT nicht im aktuellen Arbeitsverzeichnis liegt, in den Spielordner wechseln */
		if (access("SCHICK.DAT", 0) != 0 && game_dir && game_dir[0]) {
			chdir(game_dir);
		}

		MKDIR_PORT("TEMP");

		/* Dateien in TEMP loeschen */
		dir = opendir("TEMP");
		if (dir != NULL) {
			while ((ent = readdir(dir)) != NULL) {
				if (ent->d_name[0] != '.' && strlen(ent->d_name) <= 12) {
					snprintf(path, sizeof(path), "TEMP/%s", ent->d_name);
					unlink(path);
				}
			}
			closedir(dir);
		}

		/* Alle *.CHR-Dateien aus dem Spielordner nach TEMP kopieren */
		dir = opendir(".");
		if (dir != NULL) {
			while ((ent = readdir(dir)) != NULL) {
				size_t nlen = strlen(ent->d_name);
				if (nlen >= 4 && nlen <= 12 && _stricmp_port(ent->d_name + nlen - 4, ".CHR") == 0) {
					signed int handle = _open(ent->d_name, O_BINARY | O_RDONLY);
					if (handle != -1) {
						const long f_length = lseek(handle, 0, SEEK_END);
						lseek(handle, 0, SEEK_SET);
						if ((f_length == (long)sizeof(struct struct_hero)) || (f_length == (long)sizeof(struct struct_hero) - 16)) {
							struct struct_hero hero;
							memset(&hero, 0, sizeof(hero));
							if (f_length == (long)sizeof(hero)) {
								_read(handle, &hero, sizeof(struct struct_hero));
							} else {
								_read(handle, (uint8_t*)&hero + 16, sizeof(struct struct_hero) - 16);
								memcpy(&hero, (uint8_t*)&hero + 16, 16);
							}
							close(handle);

							snprintf(path, sizeof(path), "TEMP/%s", ent->d_name);
							handle = _creat(path, 0);
							if (handle != -1) {
								write(handle, &hero, sizeof(struct struct_hero));
								close(handle);
							}
						} else {
							close(handle);
						}
					}
				}
			}
			closedir(dir);
		}
	}
#endif
}""",
        "seg120.c:prepare_dirs",
    )
    s120_txt = replace_once(
        s120_txt,
        """#if !defined(__BORLANDC__)
	if (g_vga_memstart) {
		free(g_vga_memstart);
		g_vga_memstart = NULL;
		g_vga_backbuffer = NULL;
	}
#endif""",
        """#if !defined(__BORLANDC__)
	if (g_vga_memstart) {
		free(g_vga_memstart);
		g_vga_memstart = NULL;
		g_vga_backbuffer = NULL;
	}
	if (g_buffer_xx) {
		free(g_buffer_xx);
		g_buffer_xx = NULL;
	}
#endif""",
        "seg120.c:cleanup_game",
    )
    s120_txt = replace_once(
        s120_txt,
        """		gs_day_of_week = 4;
		gs_day_of_month = 17;
		gs_month = 1;
		gs_year = 15;
	}
#endif
}""",
        """		gs_day_of_week = 4;
		gs_day_of_month = 17;
		gs_month = 1;
		gs_year = 15;
	}
#else
	{
		extern int ngen_run(int init_level, int called_from_game);
		call_mouse_bg();
		ngen_run((g_game_mode == GAME_MODE_ADVANCED) ? 2 : 1, 1);
		refresh_colors();
		dsa_platform_set_mouse_range_x(0, 319);
		dsa_platform_set_mouse_range_y(0, 199);
		call_mouse();
		gs_day_timer = HOURS(24) - 1;
		timewarp_until_time_of_day(1);
		gs_day_timer = HOURS(8);
		gs_day_of_week = 4;
		gs_day_of_month = 17;
		gs_month = 1;
		gs_year = 15;
	}
#endif
}""",
        "seg120.c:call_gen",
    )
    s120_p.write_text(s120_txt, encoding="latin-1")

    # --- seg094.c: Fix Unterlauf von gs_travel_step_counter / fb_start - 321 am Routenende ---
    s94_p = SRC_DIR / "seg094.c"
    s94_txt = s94_p.read_text(encoding="latin-1")
    s94_txt = replace_once(
        s94_txt,
        """	while (gs_travel_course_ptr[(gs_travel_mousehover = 0)] != -1 &&
		!gs_travel_detour &&
		g_game_state == GAME_STATE_MAIN)""",
        """	while (gs_travel_course_ptr[(gs_travel_mousehover = 0)] != -1 &&
#if !defined(__BORLANDC__)
		(gs_journey_direction != JOURNEY_DIRECTION_BACKWARD || gs_travel_step_counter > 0) &&
#endif
		!gs_travel_detour &&
		g_game_state == GAME_STATE_MAIN)""",
        "seg094.c:trv_do_journey_backward_guard",
    )
    s94_txt = replace_once(
        s94_txt,
        """		while (gs_travel_duration_per_step / 2 > gs_trv_i)
		{
			handle_input();""",
        """		while (gs_travel_duration_per_step / 2 > gs_trv_i)
		{
			handle_input();
#if !defined(__BORLANDC__)
			if (g_mouse_rightclick_event || g_action_id == KEY_SCAN_CODE_PAGE_UP) {
				break;
			}
#endif""",
        "seg094.c:trv_do_journey_pageup_preserve",
    )
    s94_txt = replace_once(
        s94_txt,
        """		do {
			gs_travel_course_ptr += (!reverse ? -2 : 2);
			gs_travel_step_counter--;
			*(fb_start + gs_travel_course_ptr[1] * 320 + gs_travel_course_ptr[0]) =
				g_trv_track_pixel_bak[gs_travel_step_counter];

		} while (gs_travel_course_ptr[0] != -1);""",
        """#if defined(__BORLANDC__)
		do {
			gs_travel_course_ptr += (!reverse ? -2 : 2);
			gs_travel_step_counter--;
			*(fb_start + gs_travel_course_ptr[1] * 320 + gs_travel_course_ptr[0]) =
				g_trv_track_pixel_bak[gs_travel_step_counter];

		} while (gs_travel_course_ptr[0] != -1);
#else
		while (gs_travel_step_counter > 0) {
			gs_travel_course_ptr += (!reverse ? -2 : 2);
			gs_travel_step_counter--;
			if (gs_travel_course_ptr[0] == -1) {
				break;
			}
			*(fb_start + gs_travel_course_ptr[1] * 320 + gs_travel_course_ptr[0]) =
				g_trv_track_pixel_bak[gs_travel_step_counter];
		}
#endif""",
        "seg094.c:trv_do_journey_erase_track_bounds",
    )
    s94_p.write_text(s94_txt, encoding="latin-1")


def generate_ngen_embed_c() -> None:
    """
    Erzeugt Port/src/ngen_embed.c aus Port/tools/ngen_base.c (BrightEyes src/gen/ngen.c, GPL v3).
    Bindet die Heldenerschaffung (GEN.EXE v1.07) und das Roaming-Gods-Intro direkt an
    dsa_platform.h und vgalib.h im selben SDL2-Fenster an.
    """
    base_p = PORT_DIR / "tools" / "ngen_base.c"
    txt = base_p.read_text(encoding="latin-1")

    header_repl = """#include <stdint.h>
#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif
#ifndef O_BINARY
#define O_BINARY 0
#endif

#include "dsa_platform.h"
#include "vgalib.h"

#pragma pack(1)
struct gen_attribs {
	signed char normal;
	signed char current;
	signed char mod;
};

struct gen_inv_flags {
	unsigned short broken : 1;
	unsigned short half_empty : 1;
	unsigned short empty : 1;
	unsigned short magic : 1;
	unsigned short bit4 : 1;
	unsigned short poison_expurgicum : 1;
	unsigned short poison_vomicum : 1;
	unsigned short magic_revealed : 1;
	unsigned short bit8 : 8;
};

struct gen_inventory {
	signed short item_id;
	signed short quantity;
	struct gen_inv_flags flags;
	signed char bf;
	signed char rs_lost;
	signed char lighting_timer;
	signed char poison_type;
	signed char nr_poison_charges;
	signed char unused1;
	signed char unused2;
	signed char unused3;
};

struct struct_hero {
	char name[16];
	char alias[16];
	signed char items_num;
	signed char typus;
	signed char sex;
	unsigned char height;
	signed short weight;
	signed char god;
	signed char level;
	int32_t ap;
	int32_t money;
	signed char rs_bonus1;
	signed char rs_bonus2;
	signed char rs_be;
	signed char bp_left;
	struct gen_attribs attrib[14];
	signed short le_max;
	signed short le;
	signed short ae_max;
	signed short ae;
	signed char mr;
	signed char atpa_base;
	signed char at_weapon[7];
	signed char pa_weapon[7];
	signed char w_at_mod;
	signed char w_pa_mod;
	signed char w_type;
	signed char atpa_mod;
	signed char le_malus;
	signed char timer_id;
	signed char start_gear;
	signed char herbs;
	signed char hunger_timer;
	signed char hunger;
	signed char thirst;
	signed char fighter_id;
	signed char viewdir;
	signed char actions;
	signed char action_id;
	signed char spell_id;
	signed char enemy_id;
	signed char group_no;
	signed char temple_id;
	signed char npc_id;
	signed char group_pos;
	int32_t heal_timer;
	int32_t staffspell_timer;
	signed char recipe_id;
	signed char recipe_timer;
	signed char ruhe_koerper;
	signed char blind_timer;
	signed char ecliptifactus_timer;
	signed char saftkraft;
	signed char fireban;
	signed char invisible;
	signed char sprite_no;
	signed char alchemy_inn_id;
	signed short escape_position;
	signed char jail;
	signed char axxeleratus;
	signed char drunk;
	signed char unkn10[8];
	signed char flags1;
	signed char flags2;
	signed short unkn11;
	signed char sick[8][5];
	signed char poison[10][5];
	signed char skills[52];
	signed char skill_incs;
	signed char spells[86];
	signed char spell_incs;
	signed char spell_school;
	signed char staff_level;
	struct gen_inventory inventory[23];
	signed short load;
	unsigned char pic[1024];
};
#pragma pack()

extern void decomp_pp20(uint8_t *src, uint8_t *dst, uint8_t *p3, uint32_t plen);
static void gen_decomp_pp20(unsigned char *dst, unsigned char *src, signed long plen)
{
	decomp_pp20(src, dst, NULL, (uint32_t)plen);
}
#define decomp_pp20 gen_decomp_pp20

static inline uint16_t readws(const unsigned char *p)
{
	return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}
static inline uint32_t readd(const unsigned char *p)
{
	return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static inline uint32_t readds(const unsigned char *p)
{
	return readd(p);
}"""

    txt = replace_once(
        txt,
        """#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
// lseek(), close(), read(), write()
#if defined(_MSC_VER)
#include <io.h>
#else
#include <unistd.h>
#endif

#include "ail_stub.h"
#endif

#include "hero.h"

#include "powerp20.h"
#include "vgalib.h"

/* portable Memory Access */
#if !defined(__BORLANDC__)
static inline signed short readws(const unsigned char *p) { return *(const signed short*)p; }
static inline unsigned int readd(const unsigned char *p) { return *(const unsigned int*)p; }
static inline signed int readds(const unsigned char *p) { return *(const signed int*)p; }""",
        header_repl + "\n#endif\n#if 0",
        "ngen:headers",
    )

    txt = replace_once(
        txt,
        """#if !defined(__BORLANDC__)
static SDL_Cursor *g_sdl_cursor = NULL;
static Uint8 *g_sdl_cursor_data_r = NULL;
static Uint8 *g_sdl_cursor_mask_r = NULL;
static Uint8 g_sdl_cursor_data_o[16*16];
static Uint8 g_sdl_cursor_mask_o[16*16];
#else
static char g_mouse_backbuffer[16 * 16];
static unsigned short *g_mouse_current_cursor = g_mouse_mask;
#endif""",
        """static char g_mouse_backbuffer[16 * 16];
static unsigned short *g_mouse_current_cursor = g_mouse_mask;""",
        "ngen:mouse_vars",
    )

    txt = replace_once(
        txt,
        "unsigned char* g_vga_memstart;",
        "extern unsigned char* g_vga_memstart;\nstatic int g_owns_vga_memstart = 0;",
        "ngen:g_vga_memstart",
    )

    txt = replace_once(
        txt,
        "signed short g_called_with_args;",
        "static signed short g_called_with_args;",
        "ngen:g_called_with_args",
    )

    txt = replace_once(
        txt,
        "static Mix_Music *music = NULL;",
        "/* music via dsa_platform */",
        "ngen:music_var",
    )

    txt = replace_once(
        txt,
        """static SDL_TimerID g_sdl_timer_id = 0;
static SDL_mutex *g_sdl_timer_mutex = NULL;""",
        "/* timer via dsa_platform */",
        "ngen:timer_vars",
    )

    txt = replace_once(
        txt,
        """#if defined(__BORLANDC__)
	g_vga_memstart = (unsigned char*)MK_FP(0xa000, 0x0);
#else
	g_vga_memstart = (unsigned char*)calloc(O_WIDTH * O_HEIGHT, 1);
#endif""",
        """	if (g_vga_memstart == NULL) {
		g_vga_memstart = (unsigned char*)calloc(O_WIDTH * O_HEIGHT, 1);
		g_owns_vga_memstart = 1;
	} else {
		memset(g_vga_memstart, 0, O_WIDTH * O_HEIGHT);
		g_owns_vga_memstart = 0;
	}""",
        "ngen:alloc_vga",
    )

    txt = replace_once(
        txt,
        """void free_buffers(void)
{
	int i;

	if (g_vga_memstart) {
#if !defined(__BORLANDC__)
		gen_free(g_vga_memstart);
#endif
		g_vga_memstart = NULL;
		g_gfx_ptr = NULL;
	}""",
        """static void free_buffers(void)
{
	int i;

	if (g_vga_memstart) {
		if (g_owns_vga_memstart) {
			gen_free(g_vga_memstart);
			g_vga_memstart = NULL;
			g_owns_vga_memstart = 0;
		}
		g_gfx_ptr = NULL;
	}""",
        "ngen:free_buffers",
    )

    txt = replace_once(
        txt,
        """#if !defined(__BORLANDC__)
static void sdl_mouse_cursor_scaled(void)""",
        """#if 0
static void sdl_mouse_cursor_scaled(void)""",
        "ngen:sdl_mouse_cursor_scaled",
    )

    txt = replace_once(
        txt,
        """		sdl_mouse_cursor_scaled();

		/* move cursor to initial position */
		const int ratio = sdl_get_ratio();
		SDL_WarpMouseInWindow(sdl_get_window(), g_mouse_posx * ratio, g_mouse_posy * ratio);
#endif""",
        """#endif
#endif""",
        "ngen:mouse_enable_tail",
    )

    txt = replace_once(
        txt,
        """		signed short *mouse_cursor = (signed short*)g_mouse_mask + 16;""",
        """		g_mouse_current_cursor = g_mouse_mask;
		dsa_platform_set_mouse_pos(g_mouse_posx, g_mouse_posy);
#if 0
		signed short *mouse_cursor = (signed short*)g_mouse_mask + 16;""",
        "ngen:mouse_enable_head",
    )

    txt = replace_once(
        txt,
        """void mouse_disable(void)
{
	if (g_have_mouse == 2) {
#if defined(__BORLANDC__)
		unsigned short p1, p2, p3, p4, p5;

		/* restore the old int 0x78 handler */
		setvect(0x78, (void interrupt far (*)(void))g_irq78_bak);

		/* uninstall mouse event handler */
		p1 = 0x0c;
		p3 = 0;
		p4 = 0;
		p5 = 0;

		do_mouse_action(&p1, &p2, &p3, &p4, &p5);
#else
		SDL_ShowCursor(SDL_DISABLE);

		SDL_FreeCursor(g_sdl_cursor);

		gen_free(g_sdl_cursor_data_r);
		g_sdl_cursor_data_r = NULL;
		gen_free(g_sdl_cursor_mask_r);
		g_sdl_cursor_mask_r = NULL;
		g_sdl_cursor = NULL;
#endif
	}
}""",
        """static void mouse_disable(void)
{
}""",
        "ngen:mouse_disable",
    )

    txt = replace_once(
        txt,
        """	const int ratio = sdl_get_ratio();
	SDL_WarpMouseInWindow(sdl_get_window(), x * ratio, y * ratio);""",
        """	dsa_platform_set_mouse_pos(x, y);""",
        "ngen:mouse_move_cursor",
    )

    txt = replace_once(
        txt,
        """static void mouse_cursor_draw(void)
{
#if defined(__BORLANDC__)""",
        """static void mouse_cursor_draw(void)
{
#if 1""",
        "ngen:mouse_cursor_draw",
    )

    txt = replace_once(
        txt,
        """static void mouse_save_bg(void)
{
#if defined(__BORLANDC__)""",
        """static void mouse_save_bg(void)
{
#if 1""",
        "ngen:mouse_save_bg",
    )

    txt = replace_once(
        txt,
        """static void mouse_restore_bg(void)
{
#if defined(__BORLANDC__)""",
        """static void mouse_restore_bg(void)
{
#if 1""",
        "ngen:mouse_restore_bg",
    )

    txt = replace_once(
        txt,
        "void mouse_bg(void)",
        "static void mouse_bg(void)",
        "ngen:mouse_bg",
    )

    txt = replace_once(
        txt,
        "retval = ((signed long)swap_u32(retval)) >> 8;",
        "retval = (signed long)(((uint32_t)swap_u32((uint32_t)retval)) >> 8);",
        "ngen:process_nvf_swap",
    )

    txt = replace_once(
        txt,
        """#if !defined(__BORLANDC__)
static int g_sdl_quit_event = 0;

static int sdl_event_loop(const int cmd)""",
        """static int g_sdl_quit_event = 0;

static void gen_poll_input(void)
{
	static double next_tick = 0.0;
	const double period = 1000.0 / 18.2065;
	const double now = (double)dsa_platform_get_ticks_ms();
	int mx = g_mouse_posx, my = g_mouse_posy;
	int moved = 0, left_click = 0, right_click = 0;

	if (next_tick == 0.0 || now - next_tick > 10.0 * period) next_tick = now;
	while (now >= next_tick) {
		g_random_gen_seed2++;
		if (g_random_gen_seed2 < 0) g_random_gen_seed2 = 0;
		next_tick += period;
	}

	dsa_platform_set_mouse_range_x(g_mouse_posx_min, g_mouse_posx_max);
	dsa_platform_set_mouse_range_y(g_mouse_posy_min, g_mouse_posy_max);
	dsa_platform_poll_mouse_events(&mx, &my, &moved, &left_click, &right_click);

	if (!g_mouse_locked) {
		if (left_click) g_mouse_leftclick_event = 1;
		if (right_click) g_mouse_rightclick_event = 1;
		if (moved) {
			g_mouse_posx = (signed short)mx;
			g_mouse_posy = (signed short)my;
			if (g_mouse_posx > g_mouse_posx_max) g_mouse_posx = g_mouse_posx_max;
			if (g_mouse_posx < g_mouse_posx_min) g_mouse_posx = g_mouse_posx_min;
			if (g_mouse_posy < g_mouse_posy_min) g_mouse_posy = g_mouse_posy_min;
			if (g_mouse_posy > g_mouse_posy_max) g_mouse_posy = g_mouse_posy_max;
			g_mouse_moved = 1;
		}
	} else if (right_click) {
		g_mouse_rightclick_event = 1;
	}
}

#if 0
static int sdl_event_loop(const int cmd)""",
        "ngen:gen_poll_input",
    )

    txt = replace_once(
        txt,
        """	// wait 16ms
	SDL_Delay(16);
#endif
}

static signed short get_bioskey(const int cmd)
{
#if !defined(__BORLANDC__)
	if (cmd == 0) {
		// return the pressed key imediately
		int keycode = sdl_event_loop(0);
		return keycode;
	} if (cmd == 1) {
		// check if a key was pressed
		int pressed = sdl_event_loop(1);
		return pressed;
	} else return 0;
#else
	return bioskey(cmd);
#endif
}""",
        """	gen_poll_input();
	mouse_motion();
	sdl_present_if_changed();
	dsa_platform_delay_ms(14);
#endif
}

static signed short get_bioskey(const int cmd)
{
	gen_poll_input();
	mouse_motion();
	sdl_present_if_changed();
	if (cmd == 1) {
		unsigned short ready = dsa_platform_key_ready();
		if (!ready) {
			dsa_platform_delay_ms(1);
		}
		return ready ? 1 : 0;
	} else if (cmd == 0) {
		if (dsa_platform_key_ready()) {
			unsigned short k = dsa_platform_key_read();
			if ((k & 0xff) == 'y' || (k & 0xff) == 'Y' || (k & 0xff) == 'z' || (k & 0xff) == 'Z') {
				k = (KEY_Y << 8) | (k & 0xff);
			}
			return (signed short)k;
		}
		return 0;
	}
	return 0;
}""",
        "ngen:wait_for_vsync_and_bioskey",
    )

    txt = replace_once(
        txt,
        "void exit_video(void)",
        "static void exit_video(void)",
        "ngen:exit_video",
    )
    txt = replace_once(
        txt,
        "void call_fill_rect_gen(",
        "static void call_fill_rect_gen(",
        "ngen:call_fill_rect_gen",
    )
    txt = replace_once(
        txt,
        "signed short gui_radio(",
        "static signed short gui_radio(",
        "ngen:gui_radio",
    )
    txt = replace_once(
        txt,
        "SDL_Delay(25);",
        "wait_for_vsync();",
        "ngen:gui_radio_delay",
    )
    txt = replace_once(
        txt,
        "SDL_Delay(75);",
        "wait_for_vsync();",
        "ngen:do_gen_delay",
    )

    txt = replace_once(
        txt,
        """static void start_music(const signed short track)
{
#if defined(__BORLANDC__)""",
        """static void start_music(const signed short track)
{
	/* Uebernommen aus MobileEyes (Commit 734d7dca, GPL v3, Gero Ohrner):
	 * ngen spielt ab dem Start CD-Track 4 ("A New Life is Born", MUSIC/Track04.OGG) in Schleife */
	if (dsa_platform_get_music_mode() == DSA_MUSIC_MODE_CD) {
		if (dsa_platform_play_cd_track(4)) {
			return;
		}
	}
	int handle = open_datfile(track);
	if (handle != -1) {
		signed long flen = get_filelength();
		if (flen > 0 && flen < 50000) {
			read_datfile(handle, g_page_buffer, (signed short)flen);
			dsa_platform_play_xmidi(g_page_buffer, (uint32_t)flen, 1);
		}
		close(handle);
	}
}
static void init_music(void)
{
	int handle = open_datfile(35);
	if (handle != -1) {
		signed long flen = get_filelength();
		if (flen > 0 && flen < 50000) {
			read_datfile(handle, g_page_buffer, (signed short)flen);
			dsa_platform_load_adlib_bank(g_page_buffer, (uint32_t)flen);
		}
		close(handle);
	}
}
static void exit_music(void)
{
	dsa_platform_stop_cd_audio();
	dsa_platform_stop_music();
}
static void set_timer_isr(void)
{
}
static void restore_timer_isr(void)
{
}
#if 0
static void start_music_orig(const signed short track)
{
#if defined(__BORLANDC__)""",
        "ngen:music_and_timer_head",
    )

    txt = replace_once(
        txt,
        """void restore_timer_isr(void)
{
#if defined(__BORLANDC__)
	setvect(0x1c, (void interrupt far (*)(void))g_timer_isr_bak);
#else
	if (SDL_LockMutex(g_sdl_timer_mutex) == 0) {

		SDL_bool timer_removed = SDL_RemoveTimer(g_sdl_timer_id);

		if (timer_removed == SDL_FALSE) {
			fprintf(stderr, "WARNING: Failed to remove timer\\n");
		}

		if (SDL_UnlockMutex(g_sdl_timer_mutex) == -1) {
			fprintf(stderr, "ERROR: Unlock Mutex in %s\\n", __func__);
		}
	} else {
		fprintf(stderr, "ERROR: Lock Mutex in %s\\n", __func__);
	}
#endif
}""",
        """void restore_timer_isr_orig(void)
{
}
#endif""",
        "ngen:music_and_timer_tail",
    )

    txt = replace_once(
        txt,
        """	answer = 1;
#else
	g_menu_tiles = 5;
	answer = gui_radio(get_text(290), 2, get_text(291), get_text(292));
	g_menu_tiles = 3;
	if (answer == -1) return;
#endif""",
        """	answer = 1;
#else
	/* Im Port von V3.02de sind Helden-Dateien immer 1754 Byte gross */
	answer = 1;
#endif""",
        "ngen:save_chr_format",
    )

    txt = replace_once(
        txt,
        """#if defined(__BORLANDC__)
			strncpy(path, g_str_temp_dir, 20);
			strcat(path, filename);
			handle = open(filename, O_BINARY | _O_WRITABLE | O_TRUNC | O_CREAT | O_DENYNONE | O_RDWR, S_IREAD | S_IWRITE);
#elif defined(_WIN32)
			strncpy(path, g_str_temp_dir, 20);
			strcat(path, filename);
			handle = _open(filename, (_O_BINARY | _O_CREAT | _O_TRUNC | _O_WRONLY), _S_IWRITE);
#else
			strncpy(path, "TEMP/", 6);
			strcat(path, filename);
			handle = open(path, (O_TRUNC | O_CREAT| O_RDWR), (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
#endif""",
        """			snprintf(path, sizeof(path), "TEMP/%s", filename);
			handle = open(path, O_BINARY | O_TRUNC | O_CREAT | O_WRONLY, 0666);""",
        "ngen:save_chr_temp_path",
    )

    txt = replace_once(
        txt,
        """int main(int argc, char** argv)
{""",
        """int ngen_run_intro(void)
{
	if (detect_datfile() == -1) return -1;
	if (sizeof(g_hero) != 0x6da) return -1;
	if (alloc_buffers() > 0) {
		free_buffers();
		return -1;
	}
	g_mouse_leftclick_event = 0;
	g_mouse_rightclick_event = 0;
	g_mouse_moved = 0;
	g_mouse_locked = 0;
	g_mouse_refresh_flag = -1;
	g_mouse_posx_min = 0;
	g_mouse_posy_min = 0;
	g_mouse_posx_max = O_WIDTH - 10;
	g_mouse_posy_max = O_HEIGHT - 1;
	g_mouse_posx = O_WIDTH / 2;
	g_mouse_posy = O_HEIGHT / 2;
	g_mouse_posx_bak = O_WIDTH / 2;
	g_mouse_posy_bak = O_HEIGHT / 2;
	load_essential_files();
	init_colors();
	mouse_enable();
	init_music();
	start_music(33);
	intro();
	exit_music();
	call_fill_rect_gen(g_vga_memstart, 0, 0, O_WIDTH - 1, O_HEIGHT - 1, 0);
	mouse_disable();
	free_buffers();
	return 0;
}

int ngen_run(int init_level, int called_from_game)
{
	int l_level = (init_level == 1 || init_level == 2) ? init_level : -1;
	g_called_with_args = called_from_game ? 1 : 0;
	g_sdl_quit_event = 0;
	g_mouse_leftclick_event = 0;
	g_mouse_rightclick_event = 0;
	g_mouse_moved = 0;
	g_mouse_locked = 0;
	g_mouse_refresh_flag = -1;
	g_mouse_posx_min = 0;
	g_mouse_posy_min = 0;
	g_mouse_posx_max = O_WIDTH - 10;
	g_mouse_posy_max = O_HEIGHT - 1;
	g_mouse_posx = O_WIDTH / 2;
	g_mouse_posy = O_HEIGHT / 2;
	g_mouse_posx_bak = O_WIDTH / 2;
	g_mouse_posy_bak = O_HEIGHT / 2;
	g_clear_archetype_pic = 1;
	g_menu_tiles = 3;
	g_in_infobox = 0;
	g_text_x_mod = 0;
	g_bool_mode = 0;

	if (detect_datfile() == -1) return -1;
	if (sizeof(g_hero) != 0x6da) return -1;
	if (alloc_buffers() > 0) {
		free_buffers();
		return -1;
	}
	load_essential_files();
	init_colors();
	mouse_enable();
	init_music();
	start_music(33);
	if (g_called_with_args == 0) {
		intro();
		init_colors();
	}
	load_common_files();
	load_pages();
	mouse_cursor();
	flush_keyboard_queue();
	while (l_level == -1) {
		l_level = gui_radio(get_text(0), 2, get_text(1), get_text(2));
		if (g_sdl_quit_event) {
			exit_music();
			free_buffers();
			return 0;
		}
	}
	do_gen(&g_hero, l_level);
	exit_music();
	mouse_bg();
	mouse_disable();
	call_fill_rect_gen(g_vga_memstart, 0, 0, O_WIDTH - 1, O_HEIGHT - 1, 0);
	free_buffers();
	return 0;
}

#if 0
int main_ngen_unused(int argc, char** argv)
{""",
        "ngen:main_to_ngen_run",
    )

    txt += "\n#endif\n"
    (SRC_DIR / "ngen_embed.c").write_text(txt, encoding="latin-1")
    print("      ngen_embed.c erzeugt.")


def instrument_coverage_and_stubs() -> None:
    """
    Werkzeug W3 (Abdeckung) & W1 (Letzte_Funktion):
    Scannt alle Port/src/seg*.c nach nativ aktiven Top-Level-Funktionsdefinitionen
    (Funktionen in reinen #if defined(__BORLANDC__)-Bloecken werden ignoriert),
    schreibt Port/src/abdeckung_liste.tsv und fuegt am Beginn jedes Funktionsrumpfes
    dsa_trace_enter(id, "segXXX:func"); ein.
    """
    # Replay-Log-Protokollierung fuer Endkampf (seg116) und Abspann (seg065)
    seg065_p = SRC_DIR / "seg065.c"
    seg065_txt = seg065_p.read_text(encoding="latin-1")
    seg065_txt = replace_once(
        seg065_txt,
        "\t/* load FACE.NVF */",
        "#if !defined(__BORLANDC__)\n\tD1_INFO(\"[REPLAY-LOG] Aufruf final_intro() (seg065)\\n\");\n#endif\n\t/* load FACE.NVF */",
        "seg065:final_intro_log",
    )
    seg065_txt = replace_once(
        seg065_txt,
        "\t/* load OUTRO1.NVF */",
        "#if !defined(__BORLANDC__)\n\tD1_INFO(\"[REPLAY-LOG] Aufruf show_outro() (seg065): Abspann startet\\n\");\n#endif\n\t/* load OUTRO1.NVF */",
        "seg065:show_outro_start_log",
    )
    seg065_txt = replace_once(
        seg065_txt,
        "\t/* mark the game as done */\n\tgs_datseg_status_start = 99;",
        "\t/* mark the game as done */\n\tgs_datseg_status_start = 99;\n#if !defined(__BORLANDC__)\n\tD1_INFO(\"[REPLAY-LOG] Ende des Abspanns erreicht (gs_datseg_status_start=99, Speicherdialog)\\n\");\n#endif",
        "seg065:show_outro_end_log",
    )
    seg065_p.write_text(seg065_txt, encoding="latin-1")

    seg116_p = SRC_DIR / "seg116.c"
    seg116_txt = seg116_p.read_text(encoding="latin-1")
    seg116_txt = replace_once(
        seg116_txt,
        "\t\t\tif (!TRV_fight_event(FIGHT_ID_F144, 144)) {\n\t\t\t\t/* you won the final fight */",
        "\t\t\tif (!TRV_fight_event(FIGHT_ID_F144, 144)) {\n\t\t\t\t/* you won the final fight */\n#if !defined(__BORLANDC__)\n\t\t\t\tD1_INFO(\"[REPLAY-LOG] Endkampf F144 beendet: SIEGER (GAME_STATE_VICTORY, +500 AP)\\n\");\n#endif",
        "seg116:tevent_144_win_log",
    )
    seg116_p.write_text(seg116_txt, encoding="latin-1")

    func_def_re = re.compile(
        r"^(?:static\s+)?(?:inline\s+)?(?:const\s+|unsigned\s+|signed\s+|struct\s+\w+\s*|[A-Za-z_]\w*\s+)+[\*\s]*([A-Za-z_]\w*)\s*\([^;{}]*\)\s*\n\{\n",
        re.M,
    )
    skip_names = {"if", "for", "while", "switch", "schick_format_crash_state", "status_to_dos", "status_from_dos", "has_suffix_ci", "open_file_ci", "sdl_timer_emulation", "sdl_poll_input", "sdl_wait_vsync", "readb", "readws", "readds", "schick_rotl16"}
    catalog_lines = ["# id\tmodul\tfunktion"]
    func_id = 0

    for seg_p in sorted(SRC_DIR.glob("seg*.c")):
        seg_name = seg_p.stem
        txt = seg_p.read_text(encoding="latin-1")

        # Berechne fuer jedes Zeilenende, ob es in einem reinen __BORLANDC__-Block liegt
        lines = txt.split("\n")
        line_offsets: list[int] = []
        borland_line: list[bool] = []
        stack: list[bool] = []
        off = 0
        for line in lines:
            s = line.strip()
            if s.startswith("#if"):
                stack.append(("BORLANDC" in s) and ("!" not in s) and ("ifndef" not in s))
            elif s.startswith("#elif"):
                if stack:
                    stack[-1] = ("BORLANDC" in s) and ("!" not in s)
            elif s.startswith("#else"):
                if stack:
                    stack[-1] = False
            elif s.startswith("#endif"):
                if stack:
                    stack.pop()
            line_offsets.append(off)
            borland_line.append(any(stack))
            off += len(line) + 1

        import bisect

        def is_in_borland(char_pos: int) -> bool:
            idx = bisect.bisect_right(line_offsets, char_pos) - 1
            return borland_line[idx] if 0 <= idx < len(borland_line) else False

        def repl(m: re.Match) -> str:
            nonlocal func_id
            fn_name = m.group(1)
            if fn_name in skip_names or is_in_borland(m.start()):
                return m.group(0)
            cur_id = func_id
            func_id += 1
            catalog_lines.append(f"{cur_id}\t{seg_name}\t{fn_name}")
            return m.group(0) + f'\tdsa_trace_enter({cur_id}, "{seg_name}:{fn_name}");\n'

        new_txt = func_def_re.sub(repl, txt)
        seg_p.write_text(new_txt, encoding="latin-1")

    (SRC_DIR / "abdeckung_liste.tsv").write_text("\n".join(catalog_lines) + "\n", encoding="utf-8")
    print(f"      abdeckung_liste.tsv erzeugt ({func_id} nativ kompilierte Funktionen).")


def main() -> int:
    print("[prepare_sources] Kopiere Quelle/ dieser Fassung (MooMoo) nach Port/src/ ...")
    SRC_DIR.mkdir(parents=True, exist_ok=True)
    (SRC_DIR / "AIL").mkdir(parents=True, exist_ok=True)

    q_cache = Path("/tmp/schick_quelle_cache")
    if os.name != "nt":
        if not (q_cache / "seg122.c").exists():
            q_cache.mkdir(parents=True, exist_ok=True)
            (q_cache / "AIL").mkdir(parents=True, exist_ok=True)
            (q_cache / "BrightEyes_DOSBox").mkdir(parents=True, exist_ok=True)
            for item in sorted(QUELLE_DIR.glob("*.[ch]")):
                shutil.copy2(item, q_cache / item.name)
            shutil.copy2(QUELLE_DIR / "s_fix.inc", q_cache / "s_fix.inc")
            shutil.copy2(QUELLE_DIR / "AIL" / "AIL.H", q_cache / "AIL" / "AIL.H")
            for seg_no in ("008", "009", "010", "011"):
                shutil.copy2(BE_DOSBOX_DIR / f"seg{seg_no}.cpp", q_cache / "BrightEyes_DOSBox" / f"seg{seg_no}.cpp")
        src_q = q_cache
        src_be = q_cache / "BrightEyes_DOSBox"
    else:
        src_q = QUELLE_DIR
        src_be = BE_DOSBOX_DIR

    for item in sorted(src_q.glob("*.[ch]")):
        shutil.copy2(item, SRC_DIR / item.name)
    shutil.copy2(src_q / "s_fix.inc", SRC_DIR / "s_fix.inc")
    shutil.copy2(src_q / "AIL" / "AIL.H", SRC_DIR / "AIL" / "AIL.H")

    # C-Ersatz fuer seg008, seg009, seg010, seg011 aus Quelle/BrightEyes_DOSBox/
    for seg_no in ("008", "009", "010", "011"):
        shutil.copy2(src_be / f"seg{seg_no}.cpp", SRC_DIR / f"seg{seg_no}.c")

    print("[prepare_sources] Wende Port-Anpassungen auf Port/src/ an ...")
    patch_platform_and_headers()
    patch_segments()
    generate_ngen_embed_c()
    instrument_coverage_and_stubs()
    generate_datseg_status_h(SRC_DIR / "datseg.c", SRC_DIR / "datseg.h", SRC_DIR / "datseg_status.h")

    if SRC_DIR != FINAL_SRC_DIR:
        FINAL_SRC_DIR.mkdir(parents=True, exist_ok=True)
        (FINAL_SRC_DIR / "AIL").mkdir(parents=True, exist_ok=True)
        updated = 0
        for item in SRC_DIR.rglob("*"):
            if item.is_file():
                rel = item.relative_to(SRC_DIR)
                dst = FINAL_SRC_DIR / rel
                dst.parent.mkdir(parents=True, exist_ok=True)
                b = item.read_bytes()
                if not dst.exists() or dst.read_bytes() != b:
                    dst.write_bytes(b)
                    updated += 1
        print(f"      {updated} geaenderte Dateien nach Port/src/ synchronisiert.")

    print("[prepare_sources] Fertig.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
