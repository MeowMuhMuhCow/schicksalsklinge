# Das Schwarze Auge: Die Schicksalsklinge — Nordlandtrilogie Starter (`NORDLANDTRILOGIE.EXE`)
## Native Release Package (V3.02de, without DOSBox)

Unified launcher and native Windows port for **Das Schwarze Auge: Die Schicksalsklinge** (German version V3.02de, 1992/1993 Attic Entertainment Software):
- **Part 1:** *Die Schicksalsklinge* (1992/1993) — **included** (`SCHICK_PORT_ALLES.EXE`, `SCHICK_PORT_WICHTIG.EXE`, `SCHICK_PORT_ORIG.EXE`)
- **Part 2:** *Sternenschweif* (1994) — prepared (unlocked automatically once the port executable is present)
- **Part 3:** *Schatten über Riva* (1996) — prepared (unlocked automatically once the port executable is present)

---

## 1. Quick Start

1. Double-click **`NORDLANDTRILOGIE.EXE`**.
2. **Provide Original Game Data (German V3.02de):**
   - Standard GOG installations are detected automatically (`CHECK OK`).
   - Or simply **drag & drop** your game folder or a **`.zip` archive** containing the German V3.02de game files (`SCHICK.DAT`, `DSAGEN.DAT`, `SCHICKM.EXE`) directly onto the launcher window.
3. Click **`>> Spiel starten`** on *Die Schicksalsklinge* (or **`Einstellungen...`** to customize settings, or press **`F5`** to start the last game directly).

---

## 2. Display Scaling & Features

- **Authentic 4:3 Aspect Ratio (`--fenster=2|3|4|5|vollbild`, default: `vollbild`):**
  - Always preserves the exact **4:3 aspect ratio** of the original 320×200 CRT display (like DOSBox `aspect=true`), with clean black borders outside the 4:3 image area.
  - Window sizes: **`2x (640×480)`**, **`3x (960×720)`**, **`4x (1280×960)`**, **`5x (1600×1200)`**, or **`Vollbild`** (fullscreen).
  - Press **`Alt+Enter`** at any time during gameplay to toggle between windowed and fullscreen mode. The window is also freely resizable.
- **Crisp Original Pixel Scaling (`--skalierung=ganzzahlig|fuellend`, default: `fuellend`):**
  - **`ganzzahlig` (`DSA_SKALIERUNG=ganzzahlig`):** Integer multiples only (`320*k × 240*k`, e.g. 5× horizontal / 6× vertical = 1600×1200 exact) with equal pixel column widths and black borders.
  - **`fuellend` (`DSA_SKALIERUNG=fuellend`):** Fills the maximum 4:3 area using **Sharp Bilinear** (Nearest-Neighbor to the largest integer multiple first, then bilinear only for the fractional remainder).
- **Three Patch Levels (`[S]`):** `Alle Fixes` (`SCHICK_PORT_ALLES.EXE`), `Wichtige Fixes` (`SCHICK_PORT_WICHTIG.EXE`), `Original` (`SCHICK_PORT_ORIG.EXE`).
- **Roll Log (`[P]`):** Logs all rolls to `PROBEN.CSV`, archives previous logs to `Probenlogs\`, and opens `Probenlog\probenlog_fenster.ps1`.
- **Crash Diagnostics (`absturz.log`):** Automatically displays crash details and reporting instructions (`https://github.com/MeowMuhMuhCow/schicksalsklinge/issues`) if `absturz.log` was written.

---

## 3. License

Licensed under the **GNU General Public License v3.0 (GPLv3)**. See [`COPYING`](COPYING) for full license terms and third-party credits. No proprietary game data from Attic Entertainment Software is included.
