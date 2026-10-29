# BLADEM.EXE selbst bauen

Dieser Ordner baut aus dem rekonstruierten Quelltext die Spiel-EXE der *Schicksalsklinge* (DSA 1, Attic 1992) und
vergleicht sie Byte für Byte mit dem Original. Am Ende steht im Fenster:

> **BYTEGLEICH mit dem Original**

## Einmal vorbereiten

| Schritt | Was |
|---|---|
| 1 | **Python für Windows** installieren: [python.org/downloads](https://www.python.org/downloads/), beim Installieren **„Add python.exe to PATH“** ankreuzen |
| 2 | Borland C++ 3.1 nach `Compiler\` (BIN, INCLUDE, LIB) und `SCHICKM.EXE` aus der GOG-Installation nach `Original\`, siehe `START_HIER.md` |

DOSBox ist schon da, nämlich das aus der GOG-Schicksalsklinge (`Realms of Arkania\DOSBOX\DOSBox.exe`).

**Ohne Installation:** das „Windows embeddable package (64-bit)“ von python.org nach `Bauen\python\` entpacken, sodass
`Bauen\python\python.exe` existiert. BAUEN.bat nimmt es dann von selbst.

## Bauen

Doppelklick auf **`BAUEN.bat`**. Dauer etwa eine halbe bis zwei Minuten, vier Schritte `[1/4] … [4/4]`.

| Ergebnis | Wo |
|---|---|
| gebaute EXE | `Build\BLADEM.EXE` (dazu `BLADEM.MAP`) |
| Protokoll | `Build\bau.log` |
| Arbeitsordner (wird bei jedem Bau geleert) | `%TEMP%\schicksalsklinge_bau` |

## Fix-Fassungen (Fehlerbehebungen)

`BAUEN.bat wichtig` bzw. `BAUEN.bat alles` baut nach dem normalen Bau zusätzlich eine EXE mit eingeschalteten
Fehlerbehebungen (Schalter `FIX_nnnn` in `Quelle\s_fix.h`, Liste mit Gründen in `FIXES.md`). Dauer etwa so lang wie
ein zweiter Bau. **Alle Behebungen sind im Spiel ungetestet – Tester willkommen.**

| Aufruf | Ergebnis | Inhalt |
|---|---|---|
| `BAUEN.bat wichtig` | `Build\BLADEM_FIX_WICHTIG.EXE` | Behebungen der Kategorien *kritisch* und *mittel* (`FIX_BUGS=1`) |
| `BAUEN.bat alles` | `Build\BLADEM_FIX_ALLES.EXE` | zusätzlich alle *egal*-Fehler (`FIX_BUGS=2`) |

Gestartet werden sie mit `BLADEM_FIX_WICHTIG_STARTEN.bat` bzw. `BLADEM_FIX_ALLES_STARTEN.bat` im Projektordner
(neben `BLADEM_STARTEN.bat`). Die Startdateien kopieren die Fix-EXE als `BLADEM.EXE` nach `Build\Fix_Wichtig` bzw.
`Build\Fix_Alles` (DOS kennt nur 8.3-Namen) und legen Spielstände in eigene Ordner
`Build\Testspielstaende_Fix_Wichtig` bzw. `Build\Testspielstaende_Fix_Alles`.

Der normale Bau läuft im Fix-Modus immer mit: er muss weiterhin **BYTEGLEICH** melden und ist die Vergleichsfassung.
Der Fix-Bau übersetzt alle Module mit `-DFIX_BUGS=1` bzw. `=2`, füllt geänderte residente Module auf ihre
Originallänge auf (`Werkzeuge\segpad.py`; wird eines länger, bricht der Bau ab), linkt und prüft mit
`Werkzeuge\fixcheck.py`: residente Segmente an gleicher Stelle, Datensegment Byte für Byte unverändert, alle
Overlay-Einsprünge zeigen auf dieselben Funktionen. Die Spielstände bleiben mit dem Original austauschbar (keine
Behebung ändert Strukturen oder das Spielstandformat).

Weitere Optionen: `BAUEN.bat alles --exploits-lassen` (Fehler, die dem Spieler nützen, bleiben drin, `FIX_EXPLOITS=0`),
`BAUEN.bat alles --fixflags "-DFIX_0222=0"` (einzelne Schalter übersteuern). Die EXE heißt dann trotzdem
`BLADEM_FIX_ALLES.EXE`.

## Was der Bau macht

| Schritt | Was passiert |
|---|---|
| 1 | Compiler und `Quelle\` in den Arbeitsordner kopieren, `TURBOC.CFG` auf `C:\BC` setzen, `DATUM.COM` anlegen (Datum 12.01.1994, steht in der EXE) |
| 2 | alle Module in der Reihenfolge von `Quelle\build.bat` übersetzen: 106 × BCC 3.1 (`-mlarge -O- -c -1 -Y/-Yo`), 5 × TASM 3.1 – immer im selben Durchgang und in derselben Reihenfolge (BCC liest in seltenen Fällen nicht vorbelegten Speicher, siehe `Analyse\LEITFADEN_C.md`) |
| 3 | in der Entwicklungsumgebung linken wie Attic: `BC.EXE /b BLADEM.PRJ` |
| 4 | Vergleich mit `Original\SCHICKM.EXE` (MD5 `4ce035f95ead01a8b2553b4487544e5d`) |

Wer eine Datei in `Quelle\` ändert, baut mit seiner Änderung. Die EXE ist dann natürlich nicht mehr bytegleich.

## Meldungen

| Meldung | Bedeutung |
|---|---|
| `BYTEGLEICH mit dem Original` | alles richtig |
| `ABWEICHUNG vom Original: erste Differenz bei Byte …` | gebaut, aber nicht gleich (Quelle geändert?) |
| `Uebersetzungsfehler` | ein Modul übersetzt nicht, die Zeilen stehen im Protokoll |
| `Fix-Modus: ... passt nicht in sein residentes Segment` | eine Behebung macht ein residentes Modul länger – so nicht baubar |
| `Fix-Modus: fixcheck meldet Fehler` | die Fix-EXE wäre mit dem Original nicht verträglich, sie wird nicht ausgegeben |
| `Borland C++ 3.1 nicht gefunden` | `Compiler\BIN\BCC.EXE` fehlt |
| `DOSBox nicht gefunden` | GOG-Pfad anders: DOSBox-Ordner nach `Bauen\DOSBox\` kopieren oder `--dosbox PFAD` |
| `Python 3 wurde nicht gefunden` | Schritt 1 oben fehlt |

## Optionen

`BAUEN.bat` reicht Optionen an `bauen.py` weiter, z. B. `BAUEN.bat --sichtbar`.

| Option | Wirkung |
|---|---|
| `wichtig` / `alles` | zusätzlich die Fix-EXE bauen (siehe oben) |
| `--exploits-lassen`, `--fixflags "..."` | nur mit `wichtig`/`alles`, siehe oben |
| `--sichtbar` | DOSBox-Fenster zeigen |
| `--dosbox PFAD` | andere DOSBox |
| `--compiler ORDNER` | Compiler woanders |
| `--baudir ORDNER` | anderer Arbeitsordner |
| `--projekt ORDNER` | Projektordner, falls `Bauen\` woanders liegt |
