# Schicksalsklinge unter Linux

Linux-Gegenstück zu `PATCHEN.bat` und den `*_STARTEN.bat`. Gleiche Patches, gleiches Ergebnis
(die Prüfsummen werden beim Patchen kontrolliert).

## Was man braucht

| Was | Woher |
|---|---|
| die deutsche GOG-Fassung von Schicksalsklinge | GOG-Installer, unter Linux z. B. mit `innoextract` entpackt, oder über Heroic/Lutris installiert. Gebraucht wird der Ordner mit `SCHICKM.EXE` |
| Python 3 | ist meist schon da (`python3 --version`) |
| DOSBox | DOSBox Staging (empfohlen), DOSBox oder DOSBox-X, aus der Paketverwaltung oder als Flatpak (`io.github.dosbox-staging`) |

## So geht's

```bash
cd schicksalsklinge
bash Linux/patchen.sh            # erzeugt Build/BLADEM_FIX_WICHTIG.EXE und Build/BLADEM_FIX_ALLES.EXE sowie Build/SCHICK.DAT
bash Linux/starten.sh wichtig    # kritische + mittlere Fehler behoben
bash Linux/starten.sh alles      # alle behebbaren Fehler
bash Linux/starten.sh wichtig log  # dazu das Probenlog: jede Probe erscheint live im Terminal (siehe Probenlog/LIESMICH.md)
```

Beim ersten Mal sucht `patchen.sh` den Spielordner (unter `~/GOG Games`, `~/Games`, `~/Spiele`, Wine und Steam) oder
fragt danach. Der Pfad wird in `Linux/SPIELPFAD.txt` gemerkt, zum Neu-Wählen die Datei löschen. Direkt angeben geht
auch: `SPIEL=~/Spiele/Schicksalsklinge bash Linux/patchen.sh`.

## Was beim Start passiert

| | |
|---|---|
| Spielordner | wird nicht angefasst. Beim ersten Start kopiert `starten.sh` ihn (ohne CD-Abbild und DOSBox) in den Ablageordner der Fassung, Standard `Build/Linux_C_<Fassung>`. Dort liegen auch die Spielstände dieser Fassung. Spätere Starts überschreiben dort nichts außer den korrigierten Spieldaten (SCHICK.DAT) |
| Laufwerke in DOSBox | C: = die Kopie, D: = CD-Abbild von GOG (`game.inst`, für die CD-Musik), E: = die Fix-EXE als `BLADEM.EXE` |
| Einstellungen | `dosbox_realms1.conf` aus dem Spielordner (Grafik, Sound) wird mitgeladen, falls vorhanden. Ohne: `OHNE_GOGCONF=1 bash Linux/starten.sh wichtig` |
| andere DOSBox | `DOSBOX=/pfad/zu/dosbox bash Linux/starten.sh wichtig` |
| eigenes CD-Abbild | z. B. aus der alten NLT-Sammlung: `CD=~/Spiele/NLT_Images/dsa1.cue bash Linux/starten.sh wichtig` (sonst wird `game.inst` aus dem GOG-Ordner genommen) |

## Einstellungen

Alle als Umgebungsvariable beim Aufruf, z. B. `VOLLBILD=1 INTRO=0 bash Linux/starten.sh wichtig`, oder dauerhaft in
einer Datei **`Linux/starten.conf`** (wird nicht ins Repo übernommen), eine Einstellung pro Zeile:

```
ABLAGE=~/Spiele/Schicksalsklinge_Spielstaende
CD=~/Spiele/NLT_Images/dsa1.cue
VOLLBILD=1
INTRO=0
```

Was beim Aufruf angegeben wird, hat Vorrang vor der Datei.

| Einstellung | Wirkung |
|---|---|
| `ABLAGE=ordner` | Spielstände und Probenlogs liegen in `<ordner>/<Fassung>` statt in `Build/` im Clone. So überleben sie ein Löschen und neues Klonen des Repos |
| `SPEICHERN=overlay` | nur DOSBox Staging oder DOSBox-X: kein Kopieren, der Spielordner ist C:, alles Geschriebene (Spielstände, korrigierte Spieldaten, Probenlog) landet als Overlay in der Ablage. Mit der klassischen DOSBox 0.74 wird weiter kopiert (die kann keine Overlays) |
| Overlay: Spielstand überschreiben | Überschreibt man im Spiel einen Spielstand, der im Spielordner liegt, löscht das Spiel die alte Datei. DOSBox Staging legt dafür in der Ablage eine Markierung `DBOVERLAY_DEL_<NAME>` an (z. B. `DBOVERLAY_DEL_NUMMER19.GAM`). Das Original im Spielordner bleibt erhalten, ist in DOSBox aber ausgeblendet, solange die Markierung existiert. Markierung löschen = Original wieder sichtbar |
| `VOLLBILD=1` | Vollbild |
| `INTRO=0` | Intro überspringen |
| `CYCLES=3000` | Tempo in DOSBox-Zyklen (bei Staging als `cpu_cycles`) |
| `SPIEL=ordner` | Spielordner (sonst gesucht bzw. aus `Linux/SPIELPFAD.txt`) |
| `CD=pfad` | eigenes CD-Abbild (siehe oben) |
| `DOSBOX=pfad` | andere DOSBox |
| `OHNE_GOGCONF=1` | ohne die DOSBox-Einstellungen von GOG |

## Selbst bauen

Bytegleich aus dem Quelltext, mit Borland C++ 3.1 in DOSBox (Compiler nach `Compiler/`, die eigene `SCHICKM.EXE`
nach `Original/`):

```bash
sudo dnf install python3 dosbox              # bzw. sudo apt install python3 dosbox
python3 Bauen/bauen.py                       # Build/BLADEM.EXE, am Ende „BYTEGLEICH mit dem Original“
python3 Bauen/bauen.py wichtig               # zusätzlich Build/BLADEM_FIX_WICHTIG.EXE (alles: _ALLES)
bash Linux/starten.sh quelltext              # die selbst gebaute EXE starten
```

Mit DOSBox Staging geht beim Bauen kurz ein Fenster auf, das ist normal. Mehr in `Bauen/BAUEN.md` (Abschnitt Linux).

Alle Behebungen sind im Spiel noch ungetestet. Rückmeldungen sind willkommen.
