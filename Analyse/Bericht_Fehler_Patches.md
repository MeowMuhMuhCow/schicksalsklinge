# Bericht: Fehlerbehebungen per Schalter (Fix-Bau) – Die Schicksalsklinge

Stand 29.10.2026, Werkbank `/root/q005` (Cloud). BLADEM.EXE CD 3.02 de. Der Ausgangsstand von Q 005 blieb
unverändert; alles Neue liegt in `Q 005\Fix_Abgabe\`.

> **Alle Behebungen sind im Spiel ungetestet.** Sie sind am Quelltext entworfen, übersetzt, statisch geprüft und
> starten in DOSBox bis zur ersten fehlenden Spieldatei. Gespielt hat damit noch niemand. **Tester willkommen** –
> Rückmeldungen bitte mit der Nummer aus `FIXES.md`.

## Ergebnis in Kürze

| Punkt | Ergebnis |
|---|---|
| Standardbau (`FIX_BUGS=0`, Vorgabe) | **BYTEGLEICH mit dem Original** (MD5 4ce035f95ead01a8b2553b4487544e5d, 556 208 Byte), Vollbau mit allen Schaltern im Quelltext; zusätzlich alle 106 C-Module einzeln objektgleich zum Standardbau (LEDATA/FIXUPP) |
| `BLADEM_FIX_WICHTIG.EXE` (`FIX_BUGS=1`) | 561 088 Byte, MD5 21f2951fc55c8cfddcffa19d02a5904a, fixcheck IN ORDNUNG |
| `BLADEM_FIX_ALLES.EXE` (`FIX_BUGS=2`) | 563 088 Byte, MD5 c793f6554f3bc90c58b09bbc4c285fda, fixcheck IN ORDNUNG |
| fixcheck (beide) | 234 Segmente an gleicher Stelle und gleich lang (Overlay-Code ausgenommen), Datensegment Byte für Byte gleich, 99 Overlays mit 958 Einsprüngen auf dieselben Funktionen, MZ-Stack und -Einsprung gleich |
| Startprobe DOSBox (ohne Spieldaten) | Original, WICHTIG und ALLES nehmen denselben Weg (Overlay-Kopf, EMS-Abfrage, Timer) bis zur ersten fehlenden Datei `SCHICK.DAT`; nur die Lesegrößen der (größeren) Overlays unterscheiden sich |
| Windows-Paket | `Bauen\BAUEN.bat wichtig` / `alles`, Fix-Teil in `bauen.py`; unter Linux auf einer Kopie des Projektordners gegengeprüft: Standardbau BYTEGLEICH, `bauen.py alles` ergab dieselbe ALLES-EXE (gleiche MD5) |
| Spielstandformat | unverändert, alte Spielstände bleiben ladbar (keine Struktur, kein Datensegment geändert) |

## Umfang

417 Nummern im Raster (`Analyse/Fehler/Fehler_Raster.tsv`), davon 36 laut Raster kein Fehler (kein Schalter).

| Stufe | ja | teilweise | nein |
|---|---|---|---|
| wichtig (kritisch + mittel) | 120 | 5 | 47 |
| alles (zusätzlich egal) | 118 | 5 | 86 |

Von den 25 *kritischen* Nummern sind 23 behoben, 214 teilweise (Absturz verhindert, Datensatz bleibt), 362 nicht
(Fluch der Goldmünzen: Absicht unklar). Die Tabelle mit Nummer, Titel, Kategorie, Exploit, Stufe, umgesetzt, Änderung
bzw. Grund und Dateien steht in `FIXES.md`.

## Was nicht umgesetzt ist und warum

| Grund | etwa | Beispiele |
|---|---|---|
| Absicht unklar, Balance, nicht belegbar (im Zweifel nicht) | 55 | 172 (Brückensturz: welche Bedingung gemeint war), 232 (Gift alle 5 Minuten auch bei Rast: dutzende Meldungen und meist der Tod), 380 (Böser Blick: zwei Attacken gewollt?), 198 (Balsam-Mindestkosten), 369/371 (Fahrtrichtung der Strecke) |
| Spieldaten, Spieltexte oder Datensegment müssten sich ändern | 43 | 1–3 (Speer, Schneidzahn, MR-Würfel in MONSTER.DAT), 146 (Axt-Liste), 194/261/157 (Texte ohne `%s`), 391/392, 303, 409 |
| Spielstandformat bzw. neuer Speicher | 13 | 268 (Merker für dauerhaften Verlust), 365 (Reiseziel je Gruppe), 204, 374 |
| folgenlos oder kein Fix nötig | 15 | 155, 276–279, 305, 310, 381 |
| neue Spielmechanik statt Fehlerbehebung | 10 | 192 (Gardianum), 213 (Feuerbann), 218, 382 |
| residentes Segment ohne Platz | 2 | 4, 5 (seg005: +6 bzw. +68 Byte, kein toter Code im Modul) |

Teilweise (10): 47, 53, 66, 212, 214, 226, 242, 327, 341, 390 – jeweils ist nur der eindeutige Teil behoben, der
Rest braucht neue Texte oder eine Designentscheidung (Gründe in `FIXES.md`).

## Technik

- **Schalter:** `Quelle/s_fix.h` (in `v302de.h` eingebunden) und `Quelle/s_fix.inc` (TASM, derzeit ohne Benutzer)
  erzeugt `Werkzeuge/mkfix.py` aus dem Raster und `Werkzeuge/fix_status.tsv` (umgesetzt/Grund, von Hand gepflegt).
  `FIX_BUGS 0/1/2`, `FIX_EXPLOITS` (Vorgabe 1 ab Stufe 1), jeder `FIX_nnnn` übersteuerbar. mkfix schreibt auch
  `Werkzeuge/fix_schalter.tsv` und `Bauen/FIXES.md`.
- **Im Code:** `#if FIX_nnnn` mit Kommentar `/* Fix n: … */`, die neue Fassung, `#else`, das Original, `#endif`.
  Bright-Eyes-Blöcke (`M302de_ORIGINAL_BUGFIX`) wurden geprüft und, wo richtig, auf `FIX_nnnn` umgestellt (C89, ohne
  neue Literale); wo sie falsch oder unnötig sind, bleiben sie wie im Original unübersetzt stehen.
- **Was gleich bleiben muss:** keine neuen Strings, Globalen oder statischen Daten (das Datensegment ist Teil der
  Spielstände: dort stehen Zeiger hinein); neue Hilfsfunktionen nur `static` in Overlay-Modulen bzw. im eigenen
  Modul, damit keine neuen Publics die Overlay-Einsprünge verschieben.
- **Residente Module** (seg001–seg007, seg008–seg013 Assembler) dürfen nicht länger werden. `segpad.py` füllt kürzere
  auf die Originallänge auf. In seg002 entfallen mit den Behebungen elf nie aufgerufene Funktionen (Schalter
  `FIX_PLATZ_SEG002`, von mkfix aus den Schaltern des Moduls abgeleitet), und `timewarp_until_time_of_day()` ruft wie
  bei Bright-Eyes `timewarp()` auf (gleich viele Takte, die Behebungen 79/233/234 dann nur einmal). seg007 (Nr. 284)
  wurde so umgestellt, dass es nicht wächst. seg005 (Nr. 4, 5) passt nicht.
- **Bau in der Cloud:** `Werkzeuge/build_fix.sh` (Standardbau muss bytegleich sein, dann alle Module mit den
  Schaltern, segpad, Link wie Attic mit `BC.EXE /b BLADEM.PRJ`, fixcheck). `Werkzeuge/fixcheck.py` ist neu.
- **Windows:** `Bauen/bauen.py wichtig|alles [--exploits-lassen] [--fixflags "..."]` macht dasselbe nach dem
  Standardbau im selben Arbeitsordner (`FIX\`) und legt `Build\BLADEM_FIX_*.EXE` ab. `BAUEN.bat` reicht die Argumente
  durch und nennt die Startdatei.
- **Start:** `BLADEM_FIX_WICHTIG_STARTEN.bat` / `BLADEM_FIX_ALLES_STARTEN.bat` nach dem Vorbild von
  `BLADEM_STARTEN.bat` und Sternenschweif: die Fix-EXE wird als `BLADEM.EXE` nach `Build\Fix_Wichtig` bzw.
  `Build\Fix_Alles` kopiert, Spielstände gehen nach `Build\Testspielstaende_Fix_*` (Overlay auf C:), GOG-CD als D:.

## Was so nicht geht

- **Fehler in Spieldaten und Texten** (ITEMS.DAT, MONSTER.DAT, *.LTX) bleiben, weil die Aufgabe keine Datenänderung
  erlaubt. Nur wo sich ein Text zur Laufzeit eindeutig reparieren lässt, geschieht das (Nr. 193 `%S`, Nr. 206
  „VERWANDLET“, Nr. 156 „DES DES BODIR“).
- **Neue Meldungen** gehen nicht ohne neue Strings im Datensegment. Wo die Behebung eine Meldung bräuchte, ist sie
  teilweise (212, 226) oder nein (395, 262).
- **Zufallsbegegnungen auf Reisen (181):** Die Behebung macht erstmals Jagden, Bär, Kvillotter, Krakenmolch, Piraten
  usw. erreichbar. Damit laufen auch deren eigene, bisher nie erreichte Fehler; die bekannten (177–180, 327, 375) sind
  behoben. Landschaft 7 (Nr. 175) liest weiter eine verschobene Tabellenspalte. Wer das nicht will:
  `--fixflags "-DFIX_0181=0"`.
- **Spiellogik** ließ sich ohne die GOG-Daten nicht ausprobieren.

## Bitte im Spiel prüfen (nach Wichtigkeit)

1. **Start, Laden, Speichern** mit beiden Fix-EXEs und einem alten Spielstand (Format unverändert); danach den
   Spielstand mit dem Original wieder laden.
2. **Reisen (181, 177–180, 327, 375, 150, 151, 152):** mehrere Strecken fahren; es müssen jetzt verschiedene
   Begegnungen kommen (Jagd, Bär, Schlange, auf See Krakenmolch/Piraten). Krakenmolch mit schwacher Gruppe: das
   Ereignis muss enden. Furt, Fähre, „Umkehren“ auf dem Rückweg.
3. **Inventar (222, 220, 397, 398):** Kraftgürtel zwischen den Händen hin- und herlegen – KK darf nicht steigen;
   Gegenstände zwischen Körperplätzen tauschen; Stapel über 99 zusammenlegen; Tausch zwischen Helden mit voller Last.
4. **Zeit (79, 233, 234, 128):** Schritte in Stadt und Dungeon mit brennender Fackel (sie muss herunterbrennen), Rast
   über Nacht mit verfluchtem Kettenhemd, Swafnild (Mitternacht), Heil- und Stabzaubersperre.
5. **NSC (255, 256, 264, 80):** Harika verabschieden (Steigerungsversuche bleiben), einen NSC entlassen und nach 31
   bzw. 64 Tagen in seiner Taverne wiedertreffen; Verfallene Herberge, Versteinerungsfalle mit NSC.
6. **Fallen und Zufallsheld (235):** Fallen mit NSC in der Gruppe und mit Teilgruppen.
7. **Alchimie (259, 260):** Brauen mit Abtrennen bei vielen Teilgruppen, einzelnem Helden plus NSC.
8. **Magie (189, 190, 191, 196, 197, 199, 207):** Stabzauber bis zum Ende, Skelettarius oft hintereinander,
   Foramen frei gesprochen, Hexenspeichel und Armatrutz mit knappen AE.
9. **Akademie (39, 67, 68):** Identifizieren und Fluch brechen an zwei Tagen hintereinander.
10. **Seereisen (127, 137):** Abfahrten um 9 Uhr auf anderen Routen als Thorwal–Prem.

Weitere Einzelprüfungen: Wald verirrt (184, Teilgruppen müssen überleben), Bach durchwaten (164), Gorah/Silentium
(159), Wegzoll pro Person (162), Brauen-Textdatei (263), Tollwut (212, 273), Schlachtfeldfieber und Paralyse
(269, 270), Wachen bei kurzer Rast (130).

## Abgabe (`Q 005\Fix_Abgabe\`)

- `patches/01_quelle.patch`, `02_bauen.patch`, `03_werkzeuge.patch`, `04_analyse.patch` – `-p1` gegen den
  Ausgangsstand von Q 005 (auf einer Kopie mit `patch -p1` geprüft)
- `geaenderte_Dateien.zip` – dieselben Dateien vollständig, in der Ordnerstruktur von Q 005
- `BLADEM_FIX_WICHTIG.EXE`, `BLADEM_FIX_ALLES.EXE` (dazu die MAP-Dateien)
- `FIXES.md`, `BERICHT.md` (dieser Bericht), `BLADEM_FIX_WICHTIG_STARTEN.bat`, `BLADEM_FIX_ALLES_STARTEN.bat`
- `PRUEFSUMMEN.txt`, `fortschritt.log`
