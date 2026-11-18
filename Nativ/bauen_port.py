#!/usr/bin/env python3
"""
bauen_port.py - Gesamtbau-Skript fuer den nativen 32-Bit-Windows-Port (Etappe 1)
von DSA 1: Die Schicksalsklinge (CD-Version 3.02 de).

Verwendung:
    python3 Port/bauen_port.py                  # baut FIX_BUGS=2 (alles) -> SCHICK_PORT.EXE + SCHICK_PORT_ALLES.EXE
    python3 Port/bauen_port.py wichtig          # baut FIX_BUGS=1         -> SCHICK_PORT.EXE + SCHICK_PORT_WICHTIG.EXE
    python3 Port/bauen_port.py original         # baut FIX_BUGS=0         -> SCHICK_PORT.EXE + SCHICK_PORT_ORIG.EXE
    python3 Port/bauen_port.py alle_stufen      # baut alle 3 Stufen (ORIG, WICHTIG, ALLES + SCHICK_PORT.EXE)
    python3 Port/bauen_port.py alles -DFIX_0042=0 -DSKIP_COPY_PROTECTION=0

Ablauf:
1. Fuehrt Port/tools/prepare_sources.py aus (kopiert Quelle/ dieser Fassung (MooMoo)
   nach Port/src/, erzeugt datseg_status.h und wendet die nativen Port-Anpassungen
   in #if !defined(__BORLANDC__) Zweigen an; Quelle/ bleibt unangetastet).
2. Kompiliert die 110 C-Module des Spiels sowie die SDL2-Plattformschicht parallel mit
   32-Bit-MinGW-GCC (-m32 -fpack-struct=1 -fsigned-char) im Arbeitsverzeichnis
   SCHICK_BUILD_DIR (oder --build-dir=...).
3. Linkt das native Windows-Executable nach Port/build/.
"""

from __future__ import annotations

from concurrent.futures import ThreadPoolExecutor, as_completed
import os
import shutil
import subprocess
import sys
from pathlib import Path

PORT_DIR = Path(__file__).resolve().parent
ROOT_DIR = PORT_DIR.parent


def _default_win_root() -> Path:
    if os.name == "nt":
        return Path(os.environ.get("SystemDrive", "C:") + "\\")
    parts = PORT_DIR.parts
    if len(parts) >= 3 and parts[1] == "mnt" and len(parts[2]) == 1:
        return Path("/") / parts[1] / parts[2]
    return Path("/tmp")


_WIN_ROOT = _default_win_root()
WIN_BUILD_DIR = Path(
    os.environ.get("SCHICK_BUILD_DIR", str(_WIN_ROOT / "Windows" / "Temp" / "schick_port_build"))
)
MINGW_BIN_WSL = os.environ.get(
    "SCHICK_MINGW_BIN",
    str(_WIN_ROOT / "Program Files (x86)" / "Embarcadero" / "Dev-Cpp" / "TDM-GCC-64" / "bin"),
)

STUFEN = {
    "original": (0, "SCHICK_PORT_ORIG.EXE"),
    "0":        (0, "SCHICK_PORT_ORIG.EXE"),
    "wichtig":  (1, "SCHICK_PORT_WICHTIG.EXE"),
    "1":        (1, "SCHICK_PORT_WICHTIG.EXE"),
    "alles":    (2, "SCHICK_PORT_ALLES.EXE"),
    "2":        (2, "SCHICK_PORT_ALLES.EXE"),
}


def run_cmd_win(cmd_str: str, cwd: Path = WIN_BUILD_DIR) -> tuple[int, str]:
    env = os.environ.copy()
    env["PATH"] = f"{MINGW_BIN_WSL}:{env.get('PATH', '')}"
    env["WSLENV"] = "PATH/l"
    res = subprocess.run(
        ["bash", "-c", cmd_str],
        cwd=str(cwd),
        env=env,
        stdin=subprocess.DEVNULL,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        encoding="latin-1",
    )
    return res.returncode, res.stdout


def build_stufe(fix_bugs: int, exe_name: str, extra_defines: list[str], copy_to_default: bool) -> Path:
    obj_dir = WIN_BUILD_DIR / f"obj_fix{fix_bugs}"
    obj_dir.mkdir(parents=True, exist_ok=True)

    def_flags = [f"-DFIX_BUGS={fix_bugs}", "-DSKIP_COPY_PROTECTION=1"] + extra_defines
    def_str = " ".join(def_flags)

    c_files = sorted((WIN_BUILD_DIR / "src").glob("*.c"))
    h_mtime = max((p.stat().st_mtime for p in (WIN_BUILD_DIR / "src").glob("*.h")), default=0.0)
    dirty_c: list[str] = []

    for c_path in c_files:
        stem = c_path.stem
        obj_path = obj_dir / f"{stem}.o"
        if obj_path.exists() and obj_path.stat().st_mtime >= max(c_path.stat().st_mtime, h_mtime):
            continue
        dirty_c.append(f"../src/{c_path.name}")

    if dirty_c:
        print(f"      Kompiliere {len(dirty_c)} Spiel-Module gebuendelt fuer FIX_BUGS={fix_bugs} ({exe_name})...")
        c_rsp = obj_dir / "dirty_c.rsp"
        c_rsp.write_text("\n".join(dirty_c) + "\n", encoding="utf-8")
        batch_cmd = (
            f"gcc.exe -m32 -std=gnu99 -O2 -fno-strict-overflow -fno-aggressive-loop-optimizations -fpack-struct=1 -fsigned-char -w "
            f"{def_str} -I../src -I../src/AIL -I../platform -I../third_party/SDL2/include "
            f"-c @dirty_c.rsp"
        )
        rc, out = run_cmd_win(batch_cmd, cwd=obj_dir)
        if rc != 0:
            print(f"\n--- FEHLER beim gebuendelten Kompilieren ---\n{out}", file=sys.stderr)
            raise RuntimeError("Spiel-Module konnten nicht uebersetzt werden.")

    # Plattform-Dateien
    plat_defs = [
        (
            "vgalib.c",
            WIN_BUILD_DIR / "platform/vgalib.c",
            f"gcc.exe -m32 -std=gnu99 -O2 -Isrc -Iplatform -Ithird_party/SDL2/include "
            f"-c platform/vgalib.c -o {obj_dir.name}/vgalib.o",
            obj_dir / "vgalib.o",
        ),
        (
            "sdl2_win32_provider.c",
            WIN_BUILD_DIR / "third_party/SDL2/sdl2_win32_provider.c",
            f"gcc.exe -m32 -std=gnu99 -O2 -Ithird_party/SDL2/include "
            f"-c third_party/SDL2/sdl2_win32_provider.c -o {obj_dir.name}/sdl2_win32_provider.o",
            obj_dir / "sdl2_win32_provider.o",
        ),
        (
            "stb_vorbis.c",
            WIN_BUILD_DIR / "third_party/stb_vorbis/stb_vorbis.c",
            f"gcc.exe -m32 -std=gnu99 -O2 -w "
            f"-c third_party/stb_vorbis/stb_vorbis.c -o {obj_dir.name}/stb_vorbis.o",
            obj_dir / "stb_vorbis.o",
        ),
        (
            "dbopl.cpp",
            WIN_BUILD_DIR / "third_party/dbopl/dbopl.cpp",
            f"g++.exe -m32 -O2 -Ithird_party/dbopl "
            f"-c third_party/dbopl/dbopl.cpp -o {obj_dir.name}/dbopl.o",
            obj_dir / "dbopl.o",
        ),
        (
            "dsa_platform_sdl2.cpp",
            WIN_BUILD_DIR / "platform/dsa_platform_sdl2.cpp",
            f"g++.exe -m32 -O2 -Iplatform -Ithird_party/SDL2/include -Ithird_party/dbopl "
            f"-c platform/dsa_platform_sdl2.cpp -o {obj_dir.name}/dsa_platform_sdl2.o",
            obj_dir / "dsa_platform_sdl2.o",
        ),
        (
            "dsa_intro.cpp",
            WIN_BUILD_DIR / "platform/dsa_intro.cpp",
            f"g++.exe -m32 -O2 -Iplatform "
            f"-c platform/dsa_intro.cpp -o {obj_dir.name}/dsa_intro.o",
            obj_dir / "dsa_intro.o",
        ),
    ]
    for pname, psrc, pcmd, pobj in plat_defs:
        if pobj.exists() and pobj.stat().st_mtime >= psrc.stat().st_mtime:
            continue
        print(f"      Kompiliere Plattform-Modul {pname}...")
        rc, out = run_cmd_win(pcmd)
        if rc != 0:
            print(f"\n--- FEHLER in {pname} ---\n{out}", file=sys.stderr)
            raise RuntimeError(f"{pname} konnte nicht uebersetzt werden.")

    rsp_file = WIN_BUILD_DIR / f"objs_fix{fix_bugs}.rsp"
    rsp_items = [f"{obj_dir.name}/{p.stem}.o" for p in c_files] + [
        f"{obj_dir.name}/vgalib.o",
        f"{obj_dir.name}/stb_vorbis.o",
        f"{obj_dir.name}/dbopl.o",
        f"{obj_dir.name}/dsa_platform_sdl2.o",
        f"{obj_dir.name}/dsa_intro.o",
        f"{obj_dir.name}/sdl2_win32_provider.o",
    ]
    rsp_file.write_text("\n".join(rsp_items) + "\n", encoding="utf-8")

    link_cmd = (
        f"g++.exe -m32 -mwindows -o {exe_name} @{rsp_file.name} -lgdi32 -luser32 -lwinmm"
    )
    rc, out = run_cmd_win(link_cmd)
    if rc != 0:
        print(out, file=sys.stderr)
        raise RuntimeError(f"Linken von {exe_name} fehlgeschlagen.")

    build_out_dir = PORT_DIR / "build"
    build_out_dir.mkdir(parents=True, exist_ok=True)
    out_exe = build_out_dir / exe_name
    shutil.copy2(WIN_BUILD_DIR / exe_name, out_exe)
    print(f"      Gebaut: {out_exe} ({out_exe.stat().st_size:,} Bytes)")

    release_dirs = [PORT_DIR / "release", ROOT_DIR / "Starter" / "release"]
    for rdir in release_dirs:
        if rdir.exists():
            shutil.copy2(out_exe, rdir / exe_name)

    if copy_to_default:
        shutil.copy2(WIN_BUILD_DIR / exe_name, WIN_BUILD_DIR / "SCHICK_PORT.EXE")
        default_exe = build_out_dir / "SCHICK_PORT.EXE"
        shutil.copy2(WIN_BUILD_DIR / exe_name, default_exe)
        for rdir in release_dirs:
            if rdir.exists():
                shutil.copy2(default_exe, rdir / "SCHICK_PORT.EXE")
        print(f"      Standard-EXE: {default_exe} ({default_exe.stat().st_size:,} Bytes)")

    return out_exe


def main() -> int:
    global WIN_BUILD_DIR, MINGW_BIN_WSL
    stufe_arg = "alles"
    extra_defines: list[str] = []
    force_prep = False

    for arg in sys.argv[1:]:
        if arg == "--prep":
            force_prep = True
        elif arg.startswith("--build-dir="):
            WIN_BUILD_DIR = Path(arg.split("=", 1)[1])
        elif arg.startswith("--mingw-bin="):
            MINGW_BIN_WSL = arg.split("=", 1)[1]
        elif arg.startswith("-D"):
            extra_defines.append(arg)
        elif arg.lower() in STUFEN or arg.lower() == "alle_stufen":
            stufe_arg = arg.lower()
        else:
            print(f"Unbekanntes Argument: {arg}", file=sys.stderr)
            return 2

    prep_py = PORT_DIR / "tools" / "prepare_sources.py"
    ngen_c = PORT_DIR / "tools" / "ngen_base.c"
    seg2_c = PORT_DIR / "src" / "seg002.c"
    need_prep = force_prep or (not seg2_c.exists()) or (prep_py.stat().st_mtime > seg2_c.stat().st_mtime) or (ngen_c.exists() and ngen_c.stat().st_mtime > seg2_c.stat().st_mtime)
    if need_prep:
        print("[1/3] Bereite Port-Quellen vor (Port/tools/prepare_sources.py)...")
        subprocess.run([sys.executable, str(prep_py)], check=True)
    else:
        print("[1/3] Port/src/ ist aktuell (prepare_sources.py unveraendert).")

    print("[2/3] Synchronisiere Quellen nach C:/Windows/Temp/schick_port_build...")
    WIN_BUILD_DIR.mkdir(parents=True, exist_ok=True)
    (WIN_BUILD_DIR / "src" / "AIL").mkdir(parents=True, exist_ok=True)
    stage_src = Path("/tmp/schick_src_stage") if Path("/tmp/schick_src_stage/seg002.c").exists() else (PORT_DIR / "src")
    for src_item in stage_src.rglob("*"):
        if src_item.is_file():
            rel = src_item.relative_to(stage_src)
            dst_item = WIN_BUILD_DIR / "src" / rel
            dst_item.parent.mkdir(parents=True, exist_ok=True)
            if not dst_item.exists() or dst_item.stat().st_size != src_item.stat().st_size or dst_item.read_bytes() != src_item.read_bytes():
                shutil.copy2(src_item, dst_item)
    shutil.copytree(PORT_DIR / "platform", WIN_BUILD_DIR / "platform", dirs_exist_ok=True)
    shutil.copytree(PORT_DIR / "third_party", WIN_BUILD_DIR / "third_party", dirs_exist_ok=True)

    print("[3/3] Kompiliere und linke native Windows-Port-Executable(s)...")
    if stufe_arg == "alle_stufen":
        build_stufe(0, "SCHICK_PORT_ORIG.EXE", extra_defines, copy_to_default=False)
        build_stufe(1, "SCHICK_PORT_WICHTIG.EXE", extra_defines, copy_to_default=False)
        build_stufe(2, "SCHICK_PORT_ALLES.EXE", extra_defines, copy_to_default=True)
    else:
        fix_bugs, exe_name = STUFEN[stufe_arg]
        build_stufe(fix_bugs, exe_name, extra_defines, copy_to_default=True)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
