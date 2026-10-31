# Die Schicksalsklinge – Quelltext im Attic-Stil

## 281 Fehler behoben – 34 Jahre nach Release

**Inoffizielle Bugfix-Fassung · spielbar ohne Compiler · das Original bleibt unangetastet · [gleich loslegen](#schnellstart-bugfix-fassung-spielen-ohne-compiler)**

**Neu in Version 1.2:** Abgleich mit dem Osterpatch von **NRS** (2016): 29 weitere Fehler behoben, u. a. Manrek-Schatzkammertür, Totenschiff-Falltüren, Daspota-Tür und -Schatz, Oberorken-Wasserfalle, Gurthag, Treborn Kolberg, dazu korrigierte Spieldaten (Magieresistenzen, fehlende Texte, Rechtschreibung). Details in [BUGFIXES.md](BUGFIXES.md).

**Neu in Version 1.1:** Zauber so, wie das Lösungsbuch sie beschreibt: **Visibili macht die ganze Gruppe unsichtbar** (alle unsichtbar = kein Kampf), **Gardianum** fängt feindliche Kampfzauber ab, **Feuerbann** halbiert feindliches Feuer, **Große Gier** vertreibt Gegner. Details in [BUGFIXES.md](BUGFIXES.md).

> **Zum ersten Mal seit 1992:** Auf Landreisen trifft man **Höhlenbären und Kvillottern** und kann **Karene,
> Wildschweine, Auerochsen und Nashörner jagen**. Attic hatte das alles eingebaut, aber ein verkehrt herum
> geschriebener Vergleich machte es unerreichbar. Dazu: keine Spielhänger mehr bei Priester, Drache und Einbruch,
> keine 168.000 Dukaten mehr per ESC, und die Kvillotter beißt endlich. → [alle 248 Änderungen](BUGFIXES.md)

**Zusammen mit [Sternenschweif](https://github.com/MeowMuhMuhCow/sternenschweif): 508 Bugfixes für DSA 1 und DSA 2.**

**Das Schwarze Auge 1: Die Schicksalsklinge** (Attic, 1992), deutsche CD-Fassung 3.02, als lesbarer C-Quelltext.
Borland C++ 3.1 baut daraus eine `BLADEM.EXE`, die **Byte für Byte dem Original gleicht**
(MD5 `4ce035f95ead01a8b2553b4487544e5d`), **ohne Nachbearbeitung**.

> **Passt zur GOG-Fassung:** Grundlage ist die deutsche `SCHICKM.EXE` (CD-Version 3.02), wie sie in **„Realms of Arkania 1 - Blade of Destiny Classic“ bei GOG** enthalten ist (MD5 `4ce035f95ead01a8b2553b4487544e5d`). Andere Fassungen (Diskette, englisch, andere Patchstände) ergeben eine andere EXE. Vor dem Bauen die eigene EXE mit dieser Prüfsumme vergleichen.

*English: Source code of the 1992 DOS RPG "Realms of Arkania: Blade of Destiny" (German CD 3.02, as sold by GOG), based on the
Bright-Eyes project, reworked to plain C like the original. Rebuilds a byte-identical executable with Borland C++ 3.1. Includes an unofficial, untested
bug-fix build (~240 original bugs fixed, savegame compatible) behind compile-time switches.
Documentation is in German.*

## Schnellstart: Bugfix-Fassung spielen, ohne Compiler

**Alle Änderungen als übersichtliche Liste: [BUGFIXES.md](BUGFIXES.md).** Das fertige Paket zum Herunterladen gibt es unter **Releases** (rechts).

Für alle, die nur spielen wollen. Nötig sind Windows und Schicksalsklinge von GOG, sonst nichts: kein Python, kein Compiler.

1. Rechts unter **Releases** das Paket `…_Bugfix_1.2.zip` herunterladen (oder oben **Code → Download ZIP**) und irgendwohin entpacken.
2. **`PATCHEN.bat`** doppelklicken. Es findet den GOG-Spielordner (oder fragt danach), prüft per Prüfsumme, ob die
   eigene EXE genau die passende ist, und erzeugt daraus `Build\BLADEM_FIX_WICHTIG.EXE` und `Build\BLADEM_FIX_ALLES.EXE`.
   **Die Original-EXE wird nicht verändert**, das Spiel bleibt wie es ist.
3. **`BLADEM_FIX_WICHTIG_STARTEN.bat`** (wichtige Fehler) oder **`BLADEM_FIX_ALLES_STARTEN.bat`** (alle) doppelklicken.
   Das Spiel startet mit den GOG-Daten. Die Spielstände landen in eigenen Ordnern unter `Build\`, die echten bleiben unberührt.

**Funktionieren meine Spielstände weiter?** Ja. Die Bugfix-Fassung ändert das Speicherformat nicht, kein Feld, keine
Größe. Eure alten Spielstände aus dem GOG-Ordner erscheinen beim Laden automatisch und lassen sich weiterspielen.
Was ihr mit der Bugfix-Fassung neu speichert, landet in einem eigenen Ordner unter `Build\`. Eure Original-Spielstände
werden dabei nie überschrieben. Und weil das Format gleich bleibt, kann man jederzeit zurück zum Original.

Der Patch (`Patch\*.ndp`, je 35–60 KB) enthält nur die geänderten Bytes, kein Originalprogramm. Ohne gekauftes
Spiel lässt sich damit nichts anfangen.

## Danke

Dieses Projekt steht auf den Schultern von Menschen, die sich jahrelang, unbezahlt und aus reiner Liebe zu diesen
Spielen durch Maschinencode gearbeitet haben. **Ohne sie gäbe es das hier nicht.**

| | |
|---|---|
| **Henne** | Er hat im Projekt **Bright-Eyes** die eigentliche Herkulesarbeit geleistet: Die Schicksalsklinge Funktion für Funktion aus dem Maschinencode in binäräquivalenten Code zurückgewonnen, dazu die Werkzeuge und den Weg, den historischen Borland-Compiler in DOSBox anzusteuern. Über viele Jahre, größtenteils allein, mit einem Stundenaufwand, der sich kaum ermessen lässt. Er hat den Rohdiamanten ausgegraben. |
| **siebenstreich** | hat Bright-Eyes weitergeführt, den C-Code geschliffen und das Projekt am Leben gehalten (github.com/siebenstreich/BrightEyes), und hat unsere Fragen geduldig und offen beantwortet. |
| **cmfrydos** | entschlüsselt mit dem **Riva HD Viewer** die Datenformate der Nordlandtrilogie (Dungeons, Ereigniszonen, Dialoge, Kampfsystem) und teilt seine Ergebnisse offen im Forum und im Wiki. |
| **Das Crystals-DSA-Forum** | Seit 2006 sammeln die Spielerinnen und Spieler dort Fehler, Beobachtungen und Rätsel der Nordlandtrilogie, im Thread „Bugs in Sternenschweif“, in den Truhenpatch-Threads, in Tipps-Sammlungen und in der Technischen Werkstatt. Viele Einträge unserer Fehlerlisten gehen auf ihre Meldungen zurück und sind dort zuerst beschrieben worden. Danke an alle, die dort über die Jahre geschrieben, gemessen und nachgespielt haben, und an die Forenbetreiber, die das alles erhalten. |
| **NRS** | hat 2016 im Crystals-Forum einen Sammelpatch für die Schicksalsklinge veröffentlicht, direkt im Maschinencode, rund 190 Änderungen ohne Quelltext. Seine Lösungen haben uns gezeigt, wie viele Stellen gemeint waren, und seine Datenkorrekturen stecken in der Bugfix-Fassung 1.2. |
| **Alle Beteiligten an Bright-Eyes, am BrightEyes-Wiki und an den Reverse-Engineering-Projekten der Nordlandtrilogie** | für Karten, Datenformate, Dokumentation und den langen Atem. |
| **Attic Entertainment Software** | für diese Spiele. 30 Jahre später lohnt es sich immer noch, jeden Winkel davon zu verstehen. |

Fehler in diesem Repository sind unsere eigenen. Alles, was gut daran ist, beruht auf ihrer Vorarbeit.

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

## Bugfix-Fassung: die Schicksalsklinge ohne die Fehler von 1992

Neben dem originalgetreuen Bau gibt es eine **inoffizielle Fehlerbehebung**: rund 240 Fehler aus dem Original lassen
sich per Schalter beheben.

| | |
|---|---|
| Stufe „wichtig“ | kritische und mittlere Fehler, darunter 23 von 25 kritischen: Spiel hängt bei Priester, fliehendem Drachen und Einbruch, Totenschiff beendet das Spiel für alle Gruppen, Absturz bei der Gruppensuche, Stufenanstieg per ESC bringt 168.000 Dukaten |
| Stufe „alles“ | zusätzlich viele kleine Fehler ohne große Spielwirkung |
| Neu erreichbar | Fix 181: Auf Landreisen sind erstmals Höhlenbär, Kvillotter und die Jagden (Karene, Wildschwein, Auerochse, Nashorn) erreichbar, im Original kam praktisch nur ein einziger Wildtierkampf vor (abschaltbar mit `-DFIX_0181=0`). Dazu behoben: Bär und Krakenmolch würfeln AT/PA verkehrt, die Kvillotter beißt nie, Krakenmolch-Endlosschleife (177–180) |
| Exploits | standardmäßig mit behoben (z. B. KK +5 durch Umlegen am Körper). Wer sie behalten will: `FIX_EXPLOITS 0` |
| Spielstände | **kompatibel**, das Speicherformat ist unverändert |
| Bauen | `Bauen\BAUEN.bat wichtig` oder `Bauen\BAUEN.bat alles` → `Build\BLADEM_FIX_WICHTIG.EXE` bzw. `BLADEM_FIX_ALLES.EXE` |
| Spielen | `BLADEM_FIX_WICHTIG_STARTEN.bat` bzw. `BLADEM_FIX_ALLES_STARTEN.bat` (eigene Spielstand-Ordner) |
| Stand | statisch vollständig geprüft, in DOSBox gestartet, **im Spiel ungetestet. Tester willkommen** – Prüfliste in [Analyse/Bericht_Fehler_Patches.md](Analyse/Bericht_Fehler_Patches.md) |

Welcher Fehler wie behoben ist und was bewusst nicht geändert wurde: [Bauen/FIXES.md](Bauen/FIXES.md).

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
