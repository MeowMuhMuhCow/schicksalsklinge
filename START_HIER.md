# Die Schicksalsklinge – Quelltext · START HIER

**Das Schwarze Auge 1: Die Schicksalsklinge** (Attic, 1992), deutsche CD-Fassung 3.02, als lesbarer C-Quelltext.
Borland C++ 3.1 baut daraus eine BLADEM.EXE, die **Byte für Byte dem Original gleicht** (MD5 `4ce035f95ead01a8b2553b4487544e5d`).
Grundlage: BrightEyes (GPL). Überarbeitet von MooMoo mit Claude, 19./20.10.2026.

Stand dieses Pakets: **20.10.2026**. Rechte am Spiel bei den Rechteinhabern, siehe `RECHTE.md`.

## Was du tun musst (einmalig)

| # | Schritt |
|---|---|
| 1 | Dieses Zip irgendwohin entpacken, z. B. `C:\Schicksalsklinge\` |
| 2 | **Python 3** installieren (python.org → Downloads), beim Installieren **„Add python.exe to PATH“** ankreuzen |
| 3 | **Borland C++ 3.1** besorgen, zu finden auf archive.org: https://archive.org/details/bcpp31. Dann nach `Compiler\` kopieren, also so, dass dort `BIN\`, `INCLUDE\` und `LIB\` liegen. Er ist nicht im Paket, weil die Rechte bei Embarcadero liegen. |
| 4 | Aus deiner Schicksalsklinge-Installation (GOG) die Datei **`SCHICKM.EXE`** nach `Original\` kopieren. Sie dient nur zum Vergleich. |
| 5 | Die Schicksalsklinge von **GOG** installiert haben. Von dort kommen DOSBox und die Spieldaten. |

## Dann

| Ich will … | dann … | Ergebnis |
|---|---|---|
| **die EXE aus dem Quelltext bauen** | `Bauen\BAUEN.bat` doppelklicken | nach etwa einer halben bis zwei Minuten **„BYTEGLEICH mit dem Original“**. Die EXE liegt dann in `Build\BLADEM.EXE`, das Protokoll in `Build\bau.log`. |
| **das Spiel aus dem Quelltext spielen** | danach `BLADEM_STARTEN.bat` doppelklicken | Das Spiel startet mit der selbst gebauten EXE und den GOG-Daten. Das Original wird nicht angefasst. |
| **eine Fassung mit behobenen Fehlern bauen** | `Bauen\BAUEN.bat wichtig` oder `Bauen\BAUEN.bat alles` | erst der bytegleiche Bau, dann zusätzlich `Build\BLADEM_FIX_WICHTIG.EXE` (kritisch + mittel) bzw. `BLADEM_FIX_ALLES.EXE`. Welcher Fehler wie behoben ist: `Bauen\FIXES.md` |
| **die Fassung mit behobenen Fehlern spielen** | `BLADEM_FIX_WICHTIG_STARTEN.bat` oder `BLADEM_FIX_ALLES_STARTEN.bat` | eigene Spielstand-Ordner `Build\Testspielstaende_Fix_*`. **Im Spiel ungetestet**, Prüfliste in `Analyse\Bericht_Fehler_Patches.md` |
| **verstehen, wie der Code aufgebaut ist** | `LIESMICH.md` lesen | Aufbau, Lesereihenfolge, Begriffe |
| **einen Überblick zum Durchklicken** | `Analyse\Quelltextkarte.html` im Browser öffnen | alle Module nach Spielbereich, dazu die Fehler |
| **wissen, was gemacht und gefunden wurde** | `ABSCHLUSSBERICHT.md` | Ergebnis, Funde über Attic und den Compiler, Beschriftungsprüfung |
| **die Fehler im Original, für Spieler** | `Analyse\Fehler\Spieler\Fehlerliste_Schicksalsklinge.pdf` (auch .docx/.md) | was man im Spiel merkt, Cheat-Liste mit 47 Exploits, im Netz bekannt ja/nein |
| **die Fehler im Original, technisch** | `Analyse\Fehler\Originalfehler.md` und `Analyse\Fehler\Fehler_Raster.md` | jeder Fehler eingeordnet als sicher/unsicher × kritisch/mittel/egal, mit Codestelle, Herkunft (neu, Bright-Eyes, Netz) und Fix-Vorschlag |
| **den Kopierschutz verstehen** | `Analyse\Bericht_Kopierschutz.md` | CD-Prüfung und Handbuch-/Kartenabfrage |
| **einen Namen nachschlagen** | `Analyse\Namen\namen.tsv` | alle Funktionen und Daten mit Modul und Adresse |

## Was drin ist

| Ordner / Datei | Inhalt |
|---|---|
| `START_HIER.md` | diese Datei |
| `LIESMICH.md` | Aufbau des Quelltexts, Lesereihenfolge, Begriffe |
| `ABSCHLUSSBERICHT.md` | Gesamtbericht |
| `Quelle\` | **der Quelltext**: 106 Module in C, 4 in Attics Assembler, die Miles-Klangbibliothek, Header, Projekt der Entwicklungsumgebung |
| `Bauen\` | Bau-Paket für Windows: `BAUEN.bat`, `bauen.py`, Anleitung `BAUEN.md` |
| `BLADEM_STARTEN.bat` | Spiel mit der gebauten EXE starten |
| `Analyse\` | Fehlerliste für Spieler, Fehlerliste und Raster, Namensliste, Bericht zum Kopierschutz, Quelltextkarte, Leitfaden zu Attics Compiler, Beschriftungsprüfung, Spieltexte, Steckbriefe |
| `Werkzeuge\` | die Linux-Werkzeuge der Rekonstruktion, zum Nachvollziehen (`WIEDERAUFBAU.md`: Werkbank neu aufsetzen) |
| `Referenz\` | der unveränderte Bright-Eyes-Stand, mit der GPL |
| `Compiler\`, `Original\` | **leer**, hier kommen Schritt 3 und 4 hinein |

## Fehler beheben per Schalter

| | |
|---|---|
| Prinzip | Jede Behebung steht im Quelltext hinter `#if FIX_nnnn`. Ohne Schalter (`FIX_BUGS=0`) ergibt der Bau das bytegleiche Original |
| Stufen | `FIX_BUGS 1` = wichtig (kritisch + mittel), `2` = alles. `FIX_EXPLOITS 0` lässt die Exploits im Spiel. Jeder Fehler einzeln: `FIX_0181` usw. in `Quelle\s_fix.h` |
| Umfang | wichtig: 120 ganz, 5 teilweise. alles: zusätzlich 118 ganz, 5 teilweise. 23 von 25 kritischen Fehlern behoben. Was nicht geht und warum: `Bauen\FIXES.md` |
| Achtung | Fix 181 macht auf Landreisen erstmals Höhlenbär, Kvillotter und die Jagden (Karene, Wildschwein, Auerochse, Nashorn) erreichbar, das verändert das Spiel spürbar. Abschalten: `--fixflags "-DFIX_0181=0"` |
| Geprüft | Standardbau bytegleich, beide Fix-EXEs statisch geprüft und in DOSBox gestartet. **Im Spiel ungetestet, Tester willkommen** |
