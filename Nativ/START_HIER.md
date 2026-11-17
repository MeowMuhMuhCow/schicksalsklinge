# Das Schwarze Auge: Die Schicksalsklinge — Nativer Port (V3.02de)

Willkommen beim nativen C/SDL2-Port von **Das Schwarze Auge: Die Schicksalsklinge** (deutsche CD-/Disketten-Version V3.02de, 1992/1993 Attic Entertainment Software).
Der Port läuft direkt unter Windows und Linux **ohne DOSBox** — einschließlich eingebauter **Heldenerschaffung (`ngen`)**, **100 % kompatibler `.GAM`- und `.CHR`-Spielstände** zum DOS-Original, **CD-Audio (`MUSIC/Track02..15.OGG`)**, **General MIDI** und **DOSBox-OPL2-AdLib-Emulation**.

---

## 1. Voraussetzungen: Original-Spieldaten (GOG oder CD-Version V3.02de)

Dieses Release-Paket enthält **ausschließlich den Programmquellcode** und **keine urheberrechtlich geschützten Spieldaten**. Zum Spielen wird eine vorhandene Installation der deutschen Version **V3.02de** benötigt (z. B. die GOG.com-Version *Realms of Arkania: Blade of Destiny (German)* oder die Original-CD-ROM):

- `SCHICK.DAT` (Hauptarchiv der Spieldaten)
- `TEMP\` (Unterordner für Spielstände; wird beim Start automatisch angelegt, falls nicht vorhanden)
- Optional für CD-Musik (Standardmodus): `MUSIC\Track02.OGG` .. `Track15.OGG` (liegen der GOG-Installation im Unterordner `MUSIC\` bei)

---

## 2. Kompilieren (Bauen)

### Option A: Ein-Befehl-Buildskript (`bauen_port.py`)
Baut automatisch alle drei Bugfix-Sammelstufen (`SCHICK_PORT_ORIG.EXE`, `SCHICK_PORT_WICHTIG.EXE`, `SCHICK_PORT_ALLES.EXE`) sowie die Standard-Programmdatei `SCHICK_PORT.EXE`:

```bash
python3 bauen_port.py
```

Einzelne Sammelstufe oder individuelle Fix-Schalter gezielt bauen:
```bash
python3 bauen_port.py orig
python3 bauen_port.py wichtig
python3 bauen_port.py alles
python3 bauen_port.py alles -DFIX_0042=0 -DFIX_HIMMEL=1 -DFIX_SPEICHERN=1 -DFIX_PROBENLOG=0
```

### Option B: CMake (`CMakeLists.txt`)
Unter Linux oder Windows (MinGW / MSVC) mit installiertem `SDL2`:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
```

---

## 3. Spiel starten (`SPIELPFAD`)

### Unter Windows (`start_schicksalsklinge.bat`)
1. Entweder die Umgebungsvariable `SPIELPFAD` auf das Verzeichnis mit `SCHICK.DAT` setzen:
   ```bat
   set "SPIELPFAD=C:\GOG Games\Realms of Arkania - Blade of Destiny German"
   start_schicksalsklinge.bat
   ```
2. Oder den Pfad direkt als ersten Parameter übergeben:
   ```bat
   start_schicksalsklinge.bat "C:\GOG Games\Realms of Arkania - Blade of Destiny German"
   ```
   *(Ohne Angabe sucht `start_schicksalsklinge.bat` automatisch im aktuellen Ordner sowie in den typischen GOG-Installationspfaden.)*

### Unter Linux (`start_schicksalsklinge.sh`)
```bash
SPIELPFAD="/pfad/zu/Schicksalsklinge" ./start_schicksalsklinge.sh
# oder direkt:
./build/SCHICK_PORT "/pfad/zu/Schicksalsklinge"
```

---

## 4. Musik- & Ton-Modi, Sammelstufen & Sondertasten

### Drei wählbare Musiktreiber
| Schalter | Modus | Beschreibung |
| :--- | :--- | :--- |
| `--music=cd` *(Standard)* | **CD-Audio (Ogg Vorbis)** | Spielt wie die GOG-Fassung (`DSA1_CD.BAT`) und `MobileEyes` (`g_use_cdaudio_flag = 1`) die 14 CD-Audio-Spuren `MUSIC/Track02.OGG` .. `Track15.OGG` ab; fällt automatisch auf General MIDI zurück, falls `MUSIC/` fehlt. |
| `--music=gm` | **General MIDI (MPU-401)** | Setzt die XMIDI-Sequenzen (`*.XMI` aus `SCHICK.DAT`) in Echtzeit für den Windows-MIDI-Synthesizer (*Microsoft GS Wavetable Synth*) um. |
| `--music=opl` | **AdLib / OPL2 (DOSBox DBOPL)** | Exakte OPL2-Hardware-Emulation (DOSBox 0.74 `DBOPL`, 44100 Hz) mit den 79 Original-FM-Instrumenten aus `SAMPLE.AD` und dem 9-Stimmen-AIL-2.0-Algorithmus. |

### Sondertasten im Spiel
- **`F10`**: Musiktreiber im laufenden Spiel live umschalten (`CD-Audio` → `General MIDI` → `AdLib/OPL2`)
- **`F11` / `F12`**: Gesamtlautstärke (Musik & `.VOC`-Soundeffekte) leiser / lauter stellen
- **`Alt+Enter`**: Vollbild / Fenstermodus umschalten
- **`Strg+S`**: Tonmenü im Spiel öffnen
- **`Strg+E`**: Charakterbogen öffnen
- **`Strg+Q`**: Spiel sofort beenden

### Die drei Bugfix-Sammelstufen (`Quelle/s_fix.h`)
1. **`SCHICK_PORT_ORIG.EXE` (`SAMMEL_ORIG`)**: 100 % Originalverhalten von 1992/1993 einschließlich aller historischen Original-Bugs.
2. **`SCHICK_PORT_WICHTIG.EXE` (`SAMMEL_WICHTIG`)**: Behebt Abstürze, Endlosschleifen, Pufferüberläufe und Spielstand-Korruption bei unveränderter Spielmechanik.
3. **`SCHICK_PORT_ALLES.EXE` / `SCHICK_PORT.EXE` (`SAMMEL_ALLES`, Standard)**: Aktiviert alle 154 dokumentierten Fehlerkorrekturen aus `s_fix.h` sowie die optionalen Komfortfunktionen (`FIX_HIMMEL`, `FIX_SPEICHERN` via Datei `APFREI.AN`, `FIX_PROBENLOG`).

---

## 5. Lizenz & Herkunft

Dieses Projekt steht unter der **GNU General Public License v3.0 (GPLv3)** — Details und alle Urheberhinweise siehe [`COPYING`](COPYING).
