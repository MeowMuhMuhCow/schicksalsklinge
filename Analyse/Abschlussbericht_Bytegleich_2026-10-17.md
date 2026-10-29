# Abschlussbericht: Schicksalsklinge aus dem Quellcode

Stand 17.10.2026. MooMoo mit Claude.

## Nachtrag 18.10.2026: 100 % identisch, ohne Nachbearbeitung

Mit dem neueren Code von BrightEyes (github.com/siebenstreich/BrightEyes, Stand 1686ed1,
Nachfolger von Bright-Eyes) entsteht eine EXE mit **derselben MD5 wie das Original**
(`4ce035f95ead01a8b2553b4487544e5d`), allein durch Compiler und Linker.

| Ursache der letzten 19 Bytes | Lösung |
|---|---|
| Der 4-KB-Stack in `seg013.asm` war ein normales Segment. Im Original hat er den Kombinationstyp `stack` und gehört zu DGROUP. Davon hängen ab: SS:SP im EXE-Kopf (236F:1000), die DGROUP-Größe und die beiden Stack-Einträge in der Segmenttabelle von `_EXEINFO_` | `stack2_seg segment para stack 'STACK'` + `DGROUP group stack2_seg` (10 Bytes) |
| TLINK 5.1 schreibt den Programmnamen in `_EXEINFO_` immer klein. Der Linker der Entwicklungsumgebung (BC.EXE) schreibt ihn groß wie im Original. **Attic hat in der IDE gelinkt.** | in der IDE linken, Projekt `BLADEM.PRJ` (9 Bytes) |

Außerdem fünf Stellen, an denen BrightEyes Inline-Assembler oder seltsame Casts brauchte, in C ersetzt
(alle weiter bytegleich):

| Stelle | Ursache | C-Lösung |
|---|---|---|
| seg048 `status_menu` (3×) | frühere Fälle springen vorwärts ins gemeinsame Ende. Attic: vermutlich `goto` in C, C++ erzeugt dabei einen toten Sprung | `case n: …; if (0) { case m: … }` |
| seg106 `pass_item` | Borland legt gleich endende Meldungen zusammen (Cross-Jumping) | zwei unbenutzte Sprungmarken |
| seg065, seg116 `sub_light_timers` | Attics C-Code rief die Funktion ohne den long-Prototyp auf, das Argument geht als int. C++ kann keinen zweiten Prototyp (anderer Linker-Name, Hennes „Linker-Problem“) | Aufruf über gecasteten Funktionszeiger |
| seg032 `FIG_set_cb_object` | Argument wird als char übergeben, gleicher Mechanismus | Aufruf über gecasteten Funktionszeiger |
| seg098 (Cast `(struct enemy_flags)`) | geprüft: reiner C/C++-Unterschied beim Adressieren von Bitfeldern, im C-Modus erzeugt Borland ohne Cast den Originalcode | Cast bleibt, Erklärung dokumentiert |

Die IDE wurde ohne Bildschirm per Tastenskript bedient (`Werkzeuge/iderun.sh`, `ide_link_tasten.txt`,
Messfühler `BE_KEYSCRIPT` im DOSBox-Patch). Patch für BrightEyes (2 Commits, `git am`):
`PATCH_fuer_BrightEyes.patch`. Die identische EXE: `Build/BLADEM_identisch.EXE`.

## Ergebnis (17.10.2026, mit dem alten Bright-Eyes-Stand)

Aus dem Bright-Eyes-Quellcode (github.com/Henne/Bright-Eyes, Revision df74edde, unverändert in
`Quelle/`) entsteht mit Borland C++ 3.1 eine BLADEM.EXE, die **in Code und Daten bytegleich** mit
der deutschen CD-Version 3.02 (SCHICKM.EXE, GOG) ist.

| Teil | Ergebnis |
|---|---|
| 110 Code-Segmente (Hauptprogramm und 99 Overlays) | alle bytegleich, inklusive aller Aufrufziele |
| Overlay-Bereich (397.632 Bytes) | 0 Abweichungen |
| Datensegment | 1.508 Variablen an Originaladresse, Inhalt bytegleich |
| Overlay-Einsprungtabellen | alle 99 wie im Original |
| MZ-Kopf | identisch |
| Dateigröße | 556.208 Bytes, wie das Original |
| Nicht identisch | Reihenfolge der Relocation-Liste (Menge identisch, 1.920 Einträge) und der Dateiname im Overlay-Block: „bladem.exe“ statt „BLADEM.EXE“ (9 Bytes) |

Die beiden Reste sind Buchhaltung des Linkers, kein Programmcode. Vermutlich hatte Attic eine
andere TLINK-Version und die Daten auf mehrere Quelldateien verteilt.

Das Spiel läuft: Intro, Kopierschutzabfrage, Spiel (vorige Fassung am echten Rechner gespielt).

## Wie es gebaut wird

`Werkzeuge/build_dsa1.sh` erledigt alles in einer Arbeitskopie. `Quelle/` bleibt unverändert.

| Schritt | Werkzeug | Was passiert |
|---|---|---|
| 1 | `datseg_patch.py` | Datensegment in Originalform: `ds` als extern, Stackgröße, Kopie der C-Bibliotheksdaten entfernt, Stacksegmente, ein far call in SEG010, Linkreihenfolge |
| 2 | `be_fixes.py` | 34 Korrekturen am Bright-Eyes-Code (Liste unten) |
| 3 | `bc_ready.bat` | 111 Module mit BCC 3.1 / TASM übersetzen |
| 4 | `objsym.py` | Symbolnamen angleichen (absichtlich abweichende Prototypen, `_ds` → `DATASEG@`) |
| 5 | TLINK, Lauf 1 | MAP für die nächsten Schritte |
| 6 | `objpuborder.py` | Overlay-Einsprünge in Attics Reihenfolge (TLINK legt sie in umgekehrter PUBDEF-Reihenfolge an) |
| 7 | `objfixgen.py` | fest eingetragene Segmentwerte werden echte Verweise (1.137 Fixups) |
| 8 | TLINK, Lauf 2 | BLADEM.EXE, Linkdatum 12.01.1994 wie im Original |
| 9 | `exemod_stack.py` | Start-SP 0x1000 (wie EXEMOD) |
| 10 | `mapcheck`, `stubcmp`, `segdiff`, `exediff`, `relcmp` | Prüfung gegen das Original |

## Korrekturen am Bright-Eyes-Code

### Falscher Code (Spielverhalten anders als im Original)

| Datei | Funktion | Befund |
|---|---|---|
| seg106 | `pass_item` | **Absturz:** zwei Platzhalter `call 0000:FFFF`, nur für den Disassembly-Vergleich gedacht. Beim Weitergeben eines Gegenstands in den Rucksack eines anderen Helden springt das Programm dorthin |
| seg002 | `cruft_1` | Original stoppt die Musiksequenz, Bright Eyes startet sie |
| seg002 | `do_timers` | Modulo mit Vorzeichen (`F_LMOD@`), nicht ohne |
| seg002 | `timewarp` | Division mit Vorzeichen |
| seg004 | `update_wallclock` | Original vergleicht `d / 771`, nicht `d % 771` |
| seg030 | `prepare_date_str` | Division mit Vorzeichen |
| seg030 | `do_talk` | `GUI_get_ptr2` statt `GUI_get_ptr` |
| seg064 | `get_passage_travel_hours` | `tmp` ist vorzeichenbehaftet |
| seg084 | `DNG09_handler` | Rest (`% 90`), nicht Division |
| seg097 | `GUI_1c2` | ruft `GUI_write_char_to_screen_xy` |
| seg110 | `tevent_043` | Lagerplatz, nicht Auffüllplatz |
| seg117 | `TLK_way_to_ruin` | prüft Eigenschaft GE, nicht Talent Äxte |
| seg118 | `tevent_037` | `GUI_get_ptr2` statt `GUI_get_ptr` |
| seg058 | `repair_screen` | `select_hero_ok_forced` statt `select_hero_ok` |
| seg001 | `CD_unused1` | schreibt das IOCTL-Kommando nach `cd_buf1[0]`, nicht `cd_buf1[252]` |
| datseg | `g_attack_items` | Hylailer Feuer steht vor Miasthmaticum |

Diese Stellen erzeugen bei Bright Eyes oft bytegleichen Code, rufen aber eine andere Funktion oder
Bibliotheksroutine auf. Der Segmentvergleich blendet Aufrufziele aus und konnte sie nie finden.
Gefunden durch Vergleich der fertig gelinkten EXE.

### Gleiches Verhalten, andere Bytes

| Datei | Funktion | Befund |
|---|---|---|
| seg002 | `read_sound_cfg`, `read_music_driver`, `read_digi_driver` | `_close` statt `close` (über `bc__close`) |
| seg005 | `draw_fight_screen` | `_open` statt `open` (über neuen Wrapper `bc__open`) |
| seg000 | Wrapper | `bc__close` und `bc__open` für beide Builds: Borland → `_close`/`_open`, DOSBox → Aufruf ins Originalprogramm (0x2EDA bzw. 0x3636) |
| seg001 | `CD_0432` (2×) | `&req[2]` statt Konstante + `nop` |
| seg001 | `seg001_00c1` | Inline-Assembler kodiert `mov bx,si` / `add ax,dx` anders als das Original |
| seg048 | `status_menu` (3 Stellen) | Sprünge ins gemeinsame Fall-Ende vorwärts wie im Original |
| seg106 | `pass_item` | zwei Sprungmarken verhindern das Zusammenlegen gleich endender Meldungen |
| seg092 | global | unbenutzte Variable `func` verschob 4 Bytes BSS |

Alle Korrekturen liegen als Patch gegen df74edde in `Analyse/bright_eyes_korrekturen.patch` (18 Dateien). Der Bright-Eyes-DOSBox-Build übersetzt damit fehlerfrei.

## Erkenntnisse für Sternenschweif und Schatten über Riva

1. **Attic hat C kompiliert, Bright Eyes C++.** Unterschiede: Reihenfolge der PUBDEFs (und damit der
   Overlay-Einsprünge) und bei `goto` ein toter Zusatzsprung im C++-Compiler.
2. **Cross-Jumping:** Borland legt gleich endende Codestücke zusammen. Steht direkt vor dem Ziel
   dasselbe Stück, springen alle früheren vorwärts dorthin, sonst springen spätere rückwärts zum ersten.
   Eine Sprungmarke unterbricht das, auch unbenutzt und ohne ein einziges Byte.
3. **Aufrufziele vergleichen, nicht nur Bytes.** Die meisten echten Fehler lagen in Aufrufen, deren
   Maschinencode gleich ist und nur auf eine andere Funktion zeigt.
4. **Overlay-Einsprungtabellen** hängen an der PUBDEF-Reihenfolge. Falsche Reihenfolge = Funktionszeiger
   zeigen auf falsche Funktionen.
5. **Fest eingetragene Segmentwerte** (Bright Eyes `reloc_game + …`) müssen als echte Verweise ins OBJ,
   sonst fehlen Relocations und das Spiel stürzt beim Start ab.

## Messwerkzeuge

`Werkzeuge/dosbox_instrumentierung.patch` erweitert DOSBox um Messfühler (über Umgebungsvariablen):
Dateizugriffe, Tastendruck-Automatik, Screenshots, Ablaufspur mit Falle für far-Sprünge ins Leere und
Division durch 0, Speicherwache, INT-21h-Protokoll, festes Datum. Damit wurden Startabsturz,
Division-durch-0 und fehlende Relocations gefunden.

## Neu aufbauen

```
bash Werkzeuge/setup_werkbank.sh            # Borland C++ 3.1 vorher nach ~/sternenschweif/bc
bash Werkzeuge/build_dsa1.sh <Kopie dieses Ordners>
```

Ergebnis: `BLADEM.EXE`, MD5 `c07ec1ca5f608eea6bae0ff24cef95fe`.
