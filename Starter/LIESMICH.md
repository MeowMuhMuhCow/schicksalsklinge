# Das Schwarze Auge — Nordlandtrilogie Starter (`NORDLANDTRILOGIE.EXE`)
## Release-Paket für *Die Schicksalsklinge* (V3.02de, nativ ohne DOSBox)

Willkommen beim gemeinsamen Starter für die nativen Windows-Ports der **Nordlandtrilogie**:
- **Teil 1:** *Die Schicksalsklinge* (1992/1993) — **in diesem Paket enthalten** (`SCHICK_PORT_ALLES.EXE`, `SCHICK_PORT_WICHTIG.EXE`, `SCHICK_PORT_ORIG.EXE`)
- **Teil 2:** *Sternenschweif* (1994) — vorbereitet (automatisch freigeschaltet, sobald die Port-EXE im Ordner liegt)
- **Teil 3:** *Schatten über Riva* (1996) — vorbereitet (automatisch freigeschaltet, sobald die Port-EXE im Ordner liegt)

---

## 1. Schnellstart (in 3 einfachen Schritten)

1. **`NORDLANDTRILOGIE.EXE`** doppelklicken.
2. **Schritt 1 — Spieldaten bereitstellen** (wird automatisch mit **grünem Haken ✔** erkannt, falls die GOG-Version installiert ist):
   - Oder einfach den **Installationsordner** bzw. eine **`.zip`-Datei** mit den Original-Spieldaten (deutsche Version V3.02de mit `SCHICK.DAT`, `DSAGEN.DAT`, `SCHICKM.EXE`) aus dem Windows-Explorer **in das Starter-Fenster ziehen und ablegen (Drag & Drop)**.
   - Alternativ im Starter auf **`Ordner wählen...`** (`[O]`) klicken.
3. **Schritt 2 & 3 — Anzeige wählen & Spiel starten:**
   - Auf **`>> Spiel starten`** bei *Die Schicksalsklinge* klicken (oder über **`Einstellungen...`** Patch-Stufe, Fenstergröße, Skalierung, Probenlog und Vorspann anpassen).
   - Ab dem zweiten Start genügt im Hauptmenü auch ein Druck auf **`F5`** (*Letztes Spiel starten*), um sofort mit den zuletzt gewählten Einstellungen loszuspielen.

---

## 2. Funktionen des Starters & der 4:3-Originalskalierung

### Einstellungen pro Spiel (automatisch gespeichert in `einstellungen.ini`)
- **1. Patch-Stufe (`[S]`):**
  - `Alle Fixes` (`SCHICK_PORT_ALLES.EXE`, empfohlen): Alle 154 dokumentierten Fehlerkorrekturen aus `s_fix.h` sowie Komfortfunktionen.
  - `Wichtige Fixes` (`SCHICK_PORT_WICHTIG.EXE`): Behebt Abstürze, Endlosschleifen und Spielstand-Korruption bei unveränderter Spielmechanik.
  - `Original` (`SCHICK_PORT_ORIG.EXE`): 100 % historisches Originalverhalten von 1992/1993.
- **2. Fenstergröße (`[F]`, `--fenster=2|3|4|5|vollbild`, Standard: `vollbild`):**
  - Das Seitenverhältnis ist **immer exakt 4:3**, genau wie die 320×200-Auflösung damals auf dem Röhrenmonitor erschien (wie DOSBox `aspect=true`). Außerhalb des 4:3-Bildbereichs bleibt ein sauberer schwarzer Rand.
  - Wählbare Stufen: **`2x (640×480)`**, **`3x (960×720)`**, **`4x (1280×960)`**, **`5x (1600×1200)`** oder **`Vollbild`**.
  - Im laufenden Spiel schaltet **`Alt+Enter`** jederzeit zwischen Fenster und Vollbild um; das Fenster ist außerdem frei mit der Maus ziehbar und der Inhalt skaliert im 4:3-Verhältnis automatisch mit.
- **3. Skalierung der Originalpixel (`[K]`, `--skalierung=ganzzahlig|fuellend`, Standard: `fuellend`):**
  - **`Ganzzahlig` (`--skalierung=ganzzahlig`):** Nur ganze Pixel-Vielfache (`320*k × 240*k`, z. B. 5× horizontal / 6× vertikal = 1600×1200 exakt) — alle 320 Originalpixel-Spalten sind exakt gleich breit, der Rest bleibt schwarzer Rand.
  - **`Füllend` (`--skalierung=fuellend`):** So groß wie möglich im exakten 4:3-Seitenverhältnis mittels **Sharp Bilinear** (zuerst Nearest-Neighbor auf das größte ganze Vielfache, dann nur der Übergangsrest per Bilinear — knackige Pixel ohne Flimmern).
- **4. Probenlog (`[P]`):**
  - Schreibt alle Talent-, Zauber-, Attributs- und Kampfproben in `PROBEN.CSV`, archiviert frühere Logs mit Zeitstempel nach `Probenlogs\` und öffnet parallel das Live-Probenlog-Fenster (`Probenlog\probenlog_fenster.ps1`).
- **5. Vorspann / Intro (`[I]`):**
  - Spielt das Original-Intro (`INTRO_E.OVL`) mit CD-Musik ab oder überspringt es direkt ins Hauptmenü (`--kein-intro`).

### Tastenkürzel im Starter & im Spiel
- **Im Starter:**
  - `Pfeiltasten` / `Tab`: Navigation | `Enter` / `Leertaste`: Auswahl bestätigen / Spiel starten
  - `F1`: Hilfe-Overlay ein-/ausblenden | `F5`: Letztes Spiel direkt starten | `Esc`: Zurück / Beenden
- **Im Spiel:**
  - **`Alt+Enter`**: Zwischen Vollbild und Fenstermodus umschalten
  - **`Tab`**: Fenstergröße (`2x`..`5x`) durchschalten
  - **`F10`**: Musikquelle umschalten (`CD-Audio` ↔ `General MIDI` ↔ `AdLib/OPL2`)
  - **`F11` / `F12`**: Gesamtlautstärke leiser / lauter
- **Schreibschutz-Umleitung & Absturz-Diagnose (`absturz.log`):**
  - Ist der Original-Spielordner schreibgeschützt, leitet der Starter Spielstände automatisch nach `Spielstaende\` um.
  - Sollte das Spiel unerwartet abstürzen, zeigt der Starter nach der Rückkehr automatisch den Inhalt von `absturz.log` samt Hinweis zur Fehlermeldung auf GitHub (`https://github.com/MeowMuhMuhCow/schicksalsklinge/issues`) an.

---

## 3. Enthaltene Dateien in diesem Release-Paket

| Datei / Ordner | Beschreibung |
| :--- | :--- |
| `NORDLANDTRILOGIE.EXE` | Grafischer Starter für die Nordlandtrilogie (keine externen DLLs nötig) |
| `spiele.json` | Spielkatalog-Konfiguration für Teil 1–3 |
| `SCHICK_PORT_ALLES.EXE` / `SCHICK_PORT.EXE` | Nativer Port von *Die Schicksalsklinge* — Stufe **Alle Fixes** |
| `SCHICK_PORT_WICHTIG.EXE` | Nativer Port von *Die Schicksalsklinge* — Stufe **Wichtige Fixes** |
| `SCHICK_PORT_ORIG.EXE` | Nativer Port von *Die Schicksalsklinge* — Stufe **100 % Original** |
| `Probenlog/` | Live-Probenlog-Fenster (`probenlog_fenster.ps1`), HTML-Auswertung (`Probenlog_Schicksalsklinge.html`) und Dokumentation |
| `LIESMICH.md` / `README.md` | Dokumentation |
| `COPYING` | GNU General Public License v3.0 (GPLv3) und Herkunftsnachweise |

*(Hinweis: Dieses Paket enthält **keine** urheberrechtlich geschützten Original-Spieldaten von Attic Entertainment Software.)*
