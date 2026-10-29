# Die Schicksalsklinge – Quelltext im Attic-Stil

**Das Schwarze Auge 1: Die Schicksalsklinge** (Attic, 1992), deutsche CD-Fassung 3.02, als lesbarer C-Quelltext.
Borland C++ 3.1 baut daraus eine `BLADEM.EXE`, die **Byte für Byte dem Original gleicht**
(MD5 `4ce035f95ead01a8b2553b4487544e5d`), **ohne Nachbearbeitung**.

*English: Source code of the 1992 DOS RPG "Realms of Arkania: Blade of Destiny" (German CD 3.02), based on the
Bright-Eyes project, reworked to plain C like the original. Rebuilds a byte-identical executable with Borland C++ 3.1.
Documentation is in German.*

## Grundlage: Bright-Eyes

Die eigentliche Pionierarbeit, das Spiel aus dem Maschinencode in binäräquivalenten Code zurückzugewinnen, hat
**Henne** über viele Jahre im Projekt Bright-Eyes geleistet; **siebenstreich** hat es weitergeführt
(github.com/siebenstreich/BrightEyes). Dieser Stand baut darauf auf.

| Neu gegenüber Bright-Eyes | |
|---|---|
| **100 % bytegleich** | die letzten Bytes gefunden: Stack-Segment gehört zu DGROUP, Link mit dem IDE-Linker (`BC.EXE /b`), der den EXE-Namen groß schreibt wie das Original |
| **C statt C++** | Attic hat DSA 1 als C übersetzt; alle 106 Module als `.c`, wie im Original |
| Assembler | seg008/009/010/013 in Attics Schreibweise, kommentiert |
| Beschriftung | sieben Prüfwellen, 117 Umbenennungen, Spieltexte als Kommentar an 3.406 Textstellen |
| Fehler im Original | 417 Einträge, davon 381 echte Fehler und 47 Exploits ([Fehlerliste](Analyse/Fehler/Spieler/Fehlerliste_Schicksalsklinge.pdf)) |
| 16 Fehler in Bright-Eyes | falsche Aufrufziele, Vorzeichen, Absturz in `pass_item` – korrigiert |
| Bau-Paket für Windows | `Bauen\BAUEN.bat`, Standard-DOSBox aus der GOG-Installation |

## Bauen (Windows)

Ausführlich in [START_HIER.md](START_HIER.md):

1. Python 3 installieren.
2. Borland C++ 3.1 besorgen (z. B. https://archive.org/details/bcpp31) und nach `Compiler\` kopieren.
3. Die eigene `SCHICKM.EXE` aus der GOG-Installation nach `Original\` kopieren (nur zum Vergleich).
4. `Bauen\BAUEN.bat` → „BYTEGLEICH mit dem Original“ (unter einer Minute).
5. `BLADEM_STARTEN.bat` startet das Spiel mit der gebauten EXE und den GOG-Daten.

## Aufbau

| Ordner | Inhalt |
|---|---|
| `Quelle/` | 106 Module in C, Assembler, Miles-Klangbibliothek (`AIL/`), Header, Projekt der Entwicklungsumgebung |
| `Analyse/` | Fehlerliste, Namensliste, Kopierschutz-Bericht, Quelltextkarte, Beschriftungsprüfung, Spieltexte |
| `Bauen/` | Bau-Paket für Windows |
| `Werkzeuge/` | Werkzeuge der Bearbeitung |
| `Referenz/` | der unveränderte Bright-Eyes-Stand |

Lesereihenfolge: [LIESMICH.md](LIESMICH.md). Gesamtbericht: [ABSCHLUSSBERICHT.md](ABSCHLUSSBERICHT.md).
Fortsetzung: Sternenschweif (DSA 2) im selben Stil, vollständig rekonstruiert.

## Lizenz und Rechte

GPL v3 wie Bright-Eyes ([LICENSE](LICENSE)). Rechte am Spiel bei den Rechteinhabern, kein Originalprogramm, keine
Spieldaten, kein Compiler im Repository: [RECHTE.md](RECHTE.md).

MooMoo mit Claude, Oktober 2026.
