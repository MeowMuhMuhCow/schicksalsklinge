# Die Schicksalsklinge – der Quelltext

**Das Schwarze Auge 1: Die Schicksalsklinge** (Attic Entertainment Software, 1992), Quelltext der deutschen CD-Fassung 3.02.
Borland C++ 3.1 baut daraus eine BLADEM.EXE, die **Byte für Byte dem Original gleicht** (MD5 `4ce035f95ead01a8b2553b4487544e5d`).

Grundlage ist der Quelltext des Projekts **BrightEyes** (github.com/siebenstreich/BrightEyes, vorher Bright-Eyes von Henne,
GPL), den Freiwillige über Jahre aus dem Maschinencode zurückgewonnen haben. MooMoo hat ihn im Oktober 2026 mit
Claude (Anthropic) im Stil des Sternenschweif-Projekts überarbeitet: als C wie bei Attic, mit geprüften Namen, deutschen
Kommentaren, den Spieltexten im Code und einer Liste der Fehler im Original. Nur für den privaten Gebrauch und für die
Rechteinhaber, nicht zur Veröffentlichung. Der Quelltext steht unter der GPL (siehe `Referenz/Bright-Eyes_df74edde/COPYING_GPL.txt`).

## Erste Schritte

| Ich will … | dann … |
|---|---|
| **das Spiel aus dem Quelltext starten** | GOG-Schicksalsklinge installieren, `Bauen\BAUEN.bat`, dann `BLADEM_STARTEN.bat` |
| **einen Überblick** | `Analyse/Quelltextkarte.html` im Browser öffnen: alle Module nach Spielbereich, mit Funktionen und Fehlern |
| **wissen, was gemacht wurde** | `ABSCHLUSSBERICHT.md` |
| **selbst bauen** | Python 3 installieren, dann `Bauen/BAUEN.bat` (gut eine halbe Minute bis 2 Minuten, am Ende „BYTEGLEICH mit dem Original“) |
| **die Fehler im Original** | für Spieler: `Analyse/Fehler/Spieler/Fehlerliste_Schicksalsklinge.pdf`, technisch: `Analyse/Fehler/Originalfehler.md` und `Analyse/Fehler/Fehler_Raster.md` |
| **die Spieltexte** | `Analyse/Spieltexte/` (alle Texte und Dialoge als Textdateien) |

## Den Code lesen: wo anfangen?

| Reihenfolge | Datei | Was dort steht |
|---|---|---|
| 1 | `Quelle/v302de.h`, `common.h`, `datseg.h`, `hero.h` | Grundtypen, Konstanten und Aufzählungen, alle globalen Daten mit Originaladresse, die Heldenstruktur |
| 2 | `Quelle/seg002.c` | das Hauptmodul: `main()`, Zeitgeber, Tagesablauf, Klang, Maus, Speicher; bindet `datseg.c` (die globalen Daten) ein |
| 3 | `Quelle/seg025.c`, `seg066.c` | Orte und Stadt (die Hauptschleife in der Stadt) |
| 4 | `Quelle/seg032.c` | ein Kampf, Runde für Runde |
| 5 | `Quelle/seg075.c`, `seg076.c` | Dungeons: Bewegung, Zeichnen, Ereignisse |
| 6 | `Quelle/seg093.c`, `seg094.c`, `seg109.c` … | Reisen und Reiseereignisse |
| 7 | `Quelle/seg008.asm`, `seg009.asm` | Attics Assembler: VGA-Grafik und der PowerPacker-Entpacker |

Jede Datei beginnt mit einem deutschen Kopfkommentar: was das Modul tut, wer es aufruft, Besonderheiten und Fehler im Original.
An den Textstellen steht der Spieltext als Kommentar (`GUI_output(get_ttx(522)); /* "DIE TRUHE IST LEER!" */`).

## Aufbau

| Ordner | Inhalt |
|---|---|
| `Quelle/` | 106 Module in C (`segNNN.c`), 4 in Attics Assembler (`seg008/009/010/013.asm`), die Miles-Klangbibliothek (`AIL/`), Header, `BLADEM.PRJ` (Projekt der Entwicklungsumgebung), `build.bat` (Übersetzungsbefehle) |
| `Quelle/Spieldaten/` | Gegenstände, Monster und Kämpfe aus SCHICK.DAT als lesbares C (Bright-Eyes, nicht Teil der EXE) |
| `Quelle/BrightEyes_DOSBox/` | Hilfsdateien von Bright-Eyes für deren DOSBox-Fassung, nicht Teil der EXE |
| `Bauen/` | Bau-Paket für Windows: `BAUEN.bat`, `bauen.py`, `BAUEN.md` |
| `Build/` | die gebaute BLADEM.EXE |
| `Werkzeuge/` | Werkzeuge der Rekonstruktion (Linux-Werkbank) |
| `Analyse/` | Fehlerlisten (Spieler und technisch), Namensliste, Kopierschutz-Bericht, Quelltextkarte, Leitfaden zu Attics Compiler, Beschriftungsprüfung, Spieltexte, Steckbriefe |
| `Referenz/` | der unveränderte Bright-Eyes-Stand df74edde |
| `Original/` | SCHICKM.EXE als Vergleich |
| `Compiler/` | Borland C++ 3.1. **Nicht weitergeben**, die Rechte liegen bei Embarcadero |

## Namen und Begriffe

| Begriff | Bedeutung |
|---|---|
| `segNNN` | Modul Nr. NNN. seg001–seg013 sind resident (immer im Speicher), seg024–seg122 Overlays (werden bei Bedarf nachgeladen) |
| `g_…` / `gs_…` | globale Variable / globale Variable, die im Spielstand gespeichert wird |
| `// ds:0x…` | Adresse im Datensegment des Originals (gegen den Bau geprüft) |
| `Original-Bug` | Fehler im Original, bewusst so nachgebaut. Nummern ohne Fragezeichen stammen von Bright-Eyes |
| `M302de_ORIGINAL_BUGFIX` | Fix von Bright-Eyes, im Bau abgeschaltet |
| `vermutlich` | Deutung nicht sicher belegt |
| bytegleich | der gebaute Code ist Byte für Byte das Original |

Die Namen stammen nicht von Attic, deren Bezeichner sind beim Übersetzen verloren gegangen. Vergeben haben sie die
Bright-Eyes-Leute, geprüft und wo nötig berichtigt haben wir sie nach dem, was der Code tut, und nach den Spieltexten.
