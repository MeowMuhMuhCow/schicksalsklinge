#!/usr/bin/env python3
"""
bauen_starter.py - Baut das gemeinsame Startprogramm der Nordlandtrilogie
(Starter/build/NORDLANDTRILOGIE.EXE) mit 32-Bit-TDM-GCC/MinGW und SDL2.

Verwendung:
    python3 Starter/bauen_starter.py
"""

from __future__ import annotations

import os
import shutil
import subprocess
import sys
from pathlib import Path

STARTER_DIR = Path(__file__).resolve().parent
SRC_DIR = STARTER_DIR / "src"
BUILD_OUT_DIR = STARTER_DIR / "build"


def _default_win_root() -> Path:
    if os.name == "nt":
        return Path(os.environ.get("SystemDrive", "C:") + "\\")
    parts = STARTER_DIR.parts
    if len(parts) >= 3 and len(parts[1]) == 3 and len(parts[2]) == 1:
        return Path("/") / parts[1] / parts[2]
    return Path("/tmp")


_WIN_ROOT = _default_win_root()
WIN_STAGE_DIR = Path(
    os.environ.get("STARTER_BUILD_DIR", str(_WIN_ROOT / "Windows" / "Temp" / "dsa_starter_build"))
)
MINGW_BIN = os.environ.get(
    "STARTER_MINGW_BIN",
    str(_WIN_ROOT / "Program Files (x86)" / "Embarcadero" / "Dev-Cpp" / "TDM-GCC-64" / "bin"),
)


def run_mingw(cmd_str: str, cwd: Path) -> tuple[int, str]:
    env = os.environ.copy()
    env["PATH"] = f"{MINGW_BIN}:{env.get('PATH', '')}"
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
        timeout=60,
    )
    return res.returncode, res.stdout


def main() -> int:
    print("[1/3] Synchronisiere Starter/src/ in das Bauverzeichnis...")
    WIN_STAGE_DIR.mkdir(parents=True, exist_ok=True)
    stage_src = WIN_STAGE_DIR / "src"
    if stage_src.exists():
        shutil.rmtree(stage_src)
    shutil.copytree(SRC_DIR, stage_src)

    print("[2/3] Kompiliere SDL2-Provider und Starter-Hauptprogramm (640x400, -mwindows)...")
    cmd_c = (
        "gcc.exe -m32 -std=gnu99 -O2 -s -fno-ident -Isrc "
        "-c src/sdl2_win32_provider.c -o sdl2_win32_provider.o"
    )
    rc, out = run_mingw(cmd_c, cwd=WIN_STAGE_DIR)
    if rc != 0:
        print(f"FEHLER beim Uebersetzen von sdl2_win32_provider.c:\n{out}", file=sys.stderr)
        return 1

    cmd_cpp = (
        "g++.exe -m32 -std=gnu++11 -O2 -s -fno-ident -Isrc "
        "-c src/starter_main.cpp -o starter_main.o"
    )
    rc, out = run_mingw(cmd_cpp, cwd=WIN_STAGE_DIR)
    if rc != 0:
        print(f"FEHLER beim Uebersetzen von starter_main.cpp:\n{out}", file=sys.stderr)
        return 1

    cmd_rc = (
        "windres.exe -F pe-i386 -Isrc src/starter_icon.rc -O coff -o starter_icon.res"
    )
    rc, out = run_mingw(cmd_rc, cwd=WIN_STAGE_DIR)
    if rc != 0:
        print(f"FEHLER beim Uebersetzen von starter_icon.rc:\n{out}", file=sys.stderr)
        return 1

    print("[3/3] Linke NORDLANDTRILOGIE.EXE...")
    cmd_link = (
        "g++.exe -m32 -mwindows -O2 -s -fno-ident "
        "-o NORDLANDTRILOGIE.EXE starter_main.o sdl2_win32_provider.o starter_icon.res "
        "-lgdi32 -luser32 -lshell32 -lole32 -lwinmm"
    )
    rc, out = run_mingw(cmd_link, cwd=WIN_STAGE_DIR)
    if rc != 0:
        print(f"FEHLER beim Linken von NORDLANDTRILOGIE.EXE:\n{out}", file=sys.stderr)
        return 1

    import struct
    built_exe = WIN_STAGE_DIR / "NORDLANDTRILOGIE.EXE"
    exe_data = bytearray(built_exe.read_bytes())
    pe_off = struct.unpack_from("<I", exe_data, 0x3C)[0]
    num_sec = struct.unpack_from("<H", exe_data, pe_off + 6)[0]
    opt_sz = struct.unpack_from("<H", exe_data, pe_off + 20)[0]
    sec_off = pe_off + 24 + opt_sz
    hdr_pad_start = ((sec_off + 40 * num_sec + 15) & ~15)
    first_rptr = struct.unpack_from("<I", exe_data, sec_off + 20)[0]
    for p_off in range(hdr_pad_start, first_rptr, 16):
        if exe_data[p_off] == 0:
            exe_data[p_off] = 0x90
    for s_idx in range(num_sec):
        _, vsz, _, rsz, rptr = struct.unpack_from("<8sIIII", exe_data, sec_off + 40 * s_idx)
        if rptr > 0 and rsz > vsz:
            for p_off in range(rptr + ((vsz + 15) & ~15), rptr + rsz, 16):
                if exe_data[p_off] == 0:
                    exe_data[p_off] = 0x90
    built_exe.write_bytes(exe_data)

    BUILD_OUT_DIR.mkdir(parents=True, exist_ok=True)
    out_exe = BUILD_OUT_DIR / "NORDLANDTRILOGIE.EXE"
    shutil.copy2(built_exe, out_exe)
    rel_dir = STARTER_DIR / "release"
    if rel_dir.is_dir():
        shutil.copy2(out_exe, rel_dir / "NORDLANDTRILOGIE.EXE")
    print(f"Erfolgreich gebaut: {out_exe} ({out_exe.stat().st_size:,} Bytes)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
