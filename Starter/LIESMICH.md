# Das Schwarze Auge — Nordlandtrilogie Starter (`NORDLANDTRILOGIE.EXE`)
## Gemeinsames Release-Paket (*Die Schicksalsklinge* V3.02de & *Sternenschweif* V1.02de, nativ ohne DOSBox)

Willkommen beim gemeinsamen Starter für die nativen Windows-Ports der **Nordlandtrilogie**:
- **Teil 1:** *Die Schicksalsklinge* (1992/1993) — **in diesem Paket enthalten** (`SCHICK_PORT.EXE`, `SCHICK_PORT_WICHTIG.EXE`, `SCHICK_PORT_ORIG.EXE`)
- **Teil 2:** *Sternenschweif* (1994) — **in diesem Paket enthalten** (`SCHWEIF_PORT.EXE`, `SCHWEIF_PORT_WICHTIG.EXE`, `SCHWEIF_PORT_ORIG.EXE`)
- **Teil 3:** *Schatten über Riva* (1996) — vorbereitet (zeigt `Kommt bald`, solange noch kein Riva-Port im Ordner liegt; das originale Titelbild wird bereits angezeigt, sobald der Riva-Spielordner gewählt ist)

---

## 1. Schnellstart

1. **`NORDLANDTRILOGIE.EXE`** doppelklicken.
2. **Spieldaten bereitstellen** (Standard-GOG-Installationen aller drei Teile werden beim Start automatisch erkannt und mit **`✔ Spieldaten gefunden`** markiert):
   - Oder einfach den **Spielordner** bzw. ein **Archiv (`.zip` / `.7z` / `.rar`)** mit den Original-Spieldaten aus dem Windows-Explorer **in das Starter-Fenster ziehen (Drag & Drop)**.
   - Alternativ direkt auf der jeweiligen Spielkarte auf **`Ordner wählen...`** klicken.
3. **Direkt auf der Karte einstellen & starten:**
   - Jede Spielkarte zeigt links das **echte 4:3-Titelbild direkt aus deinem jeweiligen Spielordner** (ohne Spielordner ein dezentes Wappen als Platzhalter).
   - Direkt auf der Karte lassen sich **`Probenlog`**, **`Patches`**, **`Fenster`** und **`Intro`** per Klick umschalten oder über **`Einstellungen...`** im Detail anpassen.
   - Ein Klick auf **`>> Spiel starten`** startet das gewählte Spiel sofort.

---

## 2. Funktionen des Starters (Etappe 4)

### Echte Titelbilder zur Laufzeit (0 mitgelieferte Bildbytes)
- Sobald für *Die Schicksalsklinge*, *Sternenschweif* oder *Schatten über Riva* ein gültiger Spielordner hinterlegt ist, liest der Starter das jeweilige Original-Titelbild beim Start direkt aus den Spieldaten (`INTRODAT.VOL` / `SCHICK.DAT`, `DATA\STAR.DAT` / `gamea.gog`, `DATA\RIVA.ALF` / `game.gog`), dekodiert die 6-Bit-VGA-Palette originalgetreu und stellt das Bild im exakten **4:3-Seitenverhältnis** per Nearest-Neighbor mit sanftem Verlauf auf der Spielkarte dar.
- Es werden **0 Bildbytes** mitgeliefert oder auf der Festplatte zwischengespeichert.

### Klare Spielersprache (Deutsch & Englisch per `F2`)
- **Übersichtliche Statusanzeige:** **`✔ Programm gefunden`** und **`✔ Spieldaten gefunden`** (der technische Dateiname der aktiven `.EXE` erscheint dezent als **Tooltip**, wenn die Maus über *Programm gefunden* schwebt, sowie im **`F1`-Hilfefenster**).
- **Sprachumschaltung (`F2`):** Jederzeit per Knopf **`DE (F2)` / `EN (F2)`** oben rechts oder mit der Taste **`F2`** zwischen Deutsch und Englisch umschalten (wird in `einstellungen.ini` gespeichert).

### Einstellungen pro Spiel (automatisch gespeichert in `einstellungen.ini`)
- **1. Patches (`[S]`):**
  - **`Alle Fixes`** (empfohlen): Alle Fehlerkorrekturen und Komfortfunktionen.
  - **`Wichtige Fixes`**: Behebt Abstürze, Endlosschleifen und kritische Spielstand-Fehler bei unveränderter Spielmechanik.
  - **`Original`**: 100 % unverändertes Originalverhalten.
- **2. Fenstergröße (`[F]`, Standard: `Vollbild`):**
  - Seitenverhältnis immer exakt **4:3** wie damals auf dem Röhrenmonitor: **`2x (640×480)`**, **`3x (960×720)`**, **`4x (1280×960)`**, **`5x (1600×1200)`** oder **`Vollbild`** (`Alt+Enter` schaltet im Spiel jederzeit um).
- **3. Skalierung (`[K]`, Standard: `Füllend (scharf)`):**
  - **`Ganzzahlig (exakte Vielfache)`**: Nur ganze Vielfache, alle Pixelspalten exakt gleich breit.
  - **`Füllend (scharf)`**: Nutzt die gesamte 4:3-Fläche mit gestochen scharfen Pixelkanten.
- **4. Probenlog (`[P]`):**
  - **`Ausgeschaltet`** oder **`Eingeschaltet (mit Live-Fenster)`**.
- **5. Vorspann (`[I]`):**
  - **`Abspielen (Original-Vorspann)`** oder **`Überspringen`**.

### Tastenkürzel im Starter & im Spiel
- **Im Starter:**
  - `1` / `2` / `3`: Spielkarte wählen | `Pfeiltasten` / `Tab`: Navigation | `Enter` / `Leertaste`: Auswahl bestätigen / Spiel starten
  - `F1`: Hilfe ein-/ausblenden | `F2`: Sprache Deutsch/Englisch | `F5`: Schnellstart | `Esc`: Zurück / Beenden
- **Im Spiel:**
  - **`Alt+Enter`**: Vollbild / Fenster umschalten | **`Tab`**: Fenstergröße (`2x`..`5x`) durchschalten
  - **`F10`**: Musikquelle umschalten (`CD-Audio` ↔ `General MIDI` ↔ `AdLib/OPL2`) | **`F11` / `F12`**: Lautstärke leiser / lauter

---

## 3. Enthaltene Dateien in diesem Release-Paket

| Datei / Ordner | Beschreibung |
| :--- | :--- |
| `NORDLANDTRILOGIE.EXE` | Grafischer Starter für die Nordlandtrilogie (keine externen DLLs nötig) |
| `spiele.json` | Spielkatalog-Konfiguration für Teil 1–3 |
| `SCHICK_PORT.EXE` / `SCHICK_PORT_ALLES.EXE` | Nativer Port von *Die Schicksalsklinge* — Stufe **Alle Fixes** |
| `SCHICK_PORT_WICHTIG.EXE` | Nativer Port von *Die Schicksalsklinge* — Stufe **Wichtige Fixes** |
| `SCHICK_PORT_ORIG.EXE` | Nativer Port von *Die Schicksalsklinge* — Stufe **Original** |
| `SCHWEIF_PORT.EXE` / `SCHWEIF_PORT_ALLES.EXE` | Nativer Port von *Sternenschweif* — Stufe **Alle Fixes** |
| `SCHWEIF_PORT_WICHTIG.EXE` | Nativer Port von *Sternenschweif* — Stufe **Wichtige Fixes** |
| `SCHWEIF_PORT_ORIG.EXE` | Nativer Port von *Sternenschweif* — Stufe **Original** |
| `Probenlog/` | Live-Probenlog-Fenster (`probenlog_fenster.ps1`), HTML-Auswertung und Dokumentation |
| `SELBER_BAUEN.bat` | Baut `NORDLANDTRILOGIE.EXE` unter Windows mit MinGW-w64 direkt aus `src\` neu |
| `LIESMICH.md` / `README.md` | Dokumentation (Deutsch / Englisch) |
| `COPYING` | GNU General Public License v3.0 (GPLv3) und Herkunftsnachweise |

*(Hinweis: Dieses Paket enthält **keine** urheberrechtlich geschützten Original-Spieldaten oder Original-Grafiken von Attic Entertainment Software.)*
