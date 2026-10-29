# Wiederaufbau der Werkbank

Claude arbeitet in einer Cloud-Umgebung, die nach der Sitzung gelöscht wird. Alles Dauerhafte liegt in **diesem Ordner**.
Die Werkbank (DOSBox mit Messfühlern, Compiler-Kopie, Werkzeuge) wird in jeder neuen Sitzung neu aufgebaut. Dauer: ca. 5 Minuten.

## Was MooMoo tut

1. Neuen Chat in der Claude-Desktop-App auf dem Rechner **miaukuh** starten.
2. Über „+“ → „Add folder“ freigeben: `…\Q 005 DSA Schicksalsklinge` (und für Spieltests den GOG-Ordner `Realms of Arkania`).
3. Diesen Satz schicken:

> **Projekt Schicksalsklinge. Lies PROJEKT.md und WIEDERAUFBAU.md im freigegebenen Ordner, bau die Werkbank auf und mach weiter.**

Q 004 (Sternenschweif) wird nicht gebraucht. Der Compiler liegt in `Compiler\`, das Original in `Original\`.

## Was Claude tut

| Schritt | Wie |
|---|---|
| 1. Stand lesen | `PROJEKT.md` und diese Datei lesen |
| 2. Ordner holen | auf dem Rechner `Quelle`, `Werkzeuge`, `Compiler/BIN`, `Compiler/INCLUDE`, `Compiler/LIB`, `Original/SCHICKM.EXE` per `tar` packen, in `_Transfer_Werkbank\` legen und stagen (aus dem Sitzungsordner lässt sich nicht stagen) |
| 3. Werkbank bauen | `Compiler` nach `~/sternenschweif/bc/`, `SCHICKM.EXE` nach `~/sternenschweif/orig/`, Werkzeuge nach `~/sternenschweif/tools/` und `tools5/`, dann `bash ~/sternenschweif/tools/setup_werkbank.sh` (klont Bright-Eyes für den DOSBox-Fork, baut SDL 1.2 und DOSBox mit Messfühlern) |
| 4. Arbeitsstand | `git clone Werkzeuge/werkbank_git.bundle ~/sternenschweif/BE2` und `git checkout attic` (oder einfach `Quelle\` nehmen) |
| 5. Eichtest | `bash ~/sternenschweif/tools/b1.sh ~/sternenschweif/BE2/src/schick_dos b1` → **„BYTEGLEICH mit dem Original“** (ca. 1,5 min) |
| 6. Arbeiten | Ergebnisse sofort zurück in diesen Ordner: `bash Werkzeuge/sichern.sh`, Datei mit **eindeutigem Namen** schreiben, **Prüfsumme kontrollieren**, mit `tar --overwrite` auspacken |

## Stolpersteine (schon gelöst)

| Problem | Lösung |
|---|---|
| Sicherung kam „still“ mit altem Inhalt an | Dateien beim Zurückschreiben immer neu benennen (Zeitstempel im Namen), MD5 auf dem Rechner prüfen |
| `tar` auf dem Rechner: „Cannot open: File exists“ | Löschen ist dort gesperrt, deshalb `tar --overwrite` |
| Dateien auf dem Rechner löschen | geht nicht ohne Freigabe, Altes nach `_zu_loeschen\` verschieben |
| DOSBox findet den Arbeitsordner nicht | Ordnernamen unter `~/sternenschweif` höchstens 8 Zeichen (DOS) |
| BCC findet Header nicht | `TURBOC.CFG` auf `C:\BC\INCLUDE` / `C:\BC\LIB` (macht das Setup-Skript bzw. `bauen.py`) |
| Linken mit TLINK schreibt den Programmnamen klein | in der IDE linken: `BC.EXE /b BLADEM.PRJ` |
| Header-Reihenfolge ändern | Vorsicht: Im C-Modus bestimmt die erste Deklaration die Lage im BSS und die Overlay-Einsprünge (`Analyse\LEITFADEN_C.md`). Nach jeder Änderung bauen |

## Regel

Nichts Wichtiges darf nur in der Cloud liegen. Am Ende jedes Arbeitsschritts wandert der Stand in diesen Ordner.
