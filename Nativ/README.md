# Realms of Arkania: Blade of Destiny (Das Schwarze Auge: Die Schicksalsklinge) — Native Port (V3.02de)

Native 32-bit/64-bit C/C++ and SDL2 port of **Das Schwarze Auge: Die Schicksalsklinge** (German version V3.02de, 1992/1993 Attic Entertainment Software).
Runs natively on Windows and Linux **without DOSBox**, featuring integrated **character generation (`ngen`)**, **100% binary-compatible `.GAM` and `.CHR` savegames** with the original DOS release, **CD-Audio (`MUSIC/Track02..15.OGG`)**, **General MIDI**, and **DOSBox OPL2/AdLib synthesis**.

---

## 1. Requirements: Original Game Data (V3.02de)

This release package contains **source code and build scripts only** — **no proprietary game data or binaries** are included. To play, you need the German **V3.02de** game data (e.g., from the GOG.com release *Realms of Arkania: Blade of Destiny (German)* or the original German CD-ROM):

- `SCHICK.DAT` (main data archive)
- `MUSIC/Track02.OGG` .. `Track15.OGG` (optional, used by default for original CD-Audio soundtrack as in the GOG release)

---

## 2. Building from Source

### Option A: Python Build Script (`bauen_port.py`)
Builds all three bugfix preset executables (`SCHICK_PORT_ORIG.EXE`, `SCHICK_PORT_WICHTIG.EXE`, `SCHICK_PORT_ALLES.EXE`) as well as the default `SCHICK_PORT.EXE`:

```bash
python3 bauen_port.py
```

You can also build a single preset or override individual bugfix flags from `s_fix.h`:
```bash
python3 bauen_port.pyalles -DFIX_0042=0 -DFIX_HIMMEL=1 -DFIX_SPEICHERN=1 -DFIX_PROBENLOG=0
```

### Option B: CMake (`CMakeLists.txt`)
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
```

---

## 3. Running the Game (`SPIELPFAD`)

### Windows (`start_schicksalsklinge.bat`)
```bat
set "SPIELPFAD=C:\GOG Games\Realms of Arkania - Blade of Destiny German"
start_schicksalsklinge.bat
```
Or pass the game directory directly as the first argument:
```bat
start_schicksalsklinge.bat "C:\GOG Games\Realms of Arkania - Blade of Destiny German"
```

### Linux (`start_schicksalsklinge.sh`)
```bash
SPIELPFAD="/path/to/BladeOfDestinyGerman" ./start_schicksalsklinge.sh
```

---

## 4. Audio Modes & Hotkeys

- `--music=cd` *(Default)*: Plays the 14 CD-Audio Ogg Vorbis tracks (`MUSIC/Track02.OGG` .. `Track15.OGG`) identically to the GOG release (`DSA1_CD.BAT`) and `MobileEyes` (`g_use_cdaudio_flag = 1`). Automatically falls back to General MIDI if `MUSIC/` is absent.
- `--music=gm`: Real-time XMIDI-to-General-MIDI playback via Windows Multimedia MIDI (`midiOut` / *Microsoft GS Wavetable Synth*).
- `--music=opl`: Cycle-accurate OPL2/AdLib FM synthesis via DOSBox 0.74 `DBOPL` using the 79 original timbres from `SAMPLE.AD`.
- **`F10`**: Cycle music engines live during gameplay (`CD-Audio` → `General MIDI` → `AdLib/OPL2`).
- **`F11` / `F12`**: Decrease / increase master volume.

---

## 5. License & Credits

Licensed under the **GNU General Public License v3.0 (GPLv3)**. See [`COPYING`](COPYING) for full license terms and third-party attributions (`Bright-Eyes`, `MobileEyes` by Gero Ohrner, DOSBox `DBOPL`, `stb_vorbis` by Sean Barrett, and Heikki Orsila's `pp20` decompressor).
