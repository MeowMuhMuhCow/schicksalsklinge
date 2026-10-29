# Abschlussbericht: Die Schicksalsklinge aus dem Quelltext

Stand 20.10.2026. MooMoo mit Claude.

## Ergebnis

Aus dem überarbeiteten Quelltext baut Borland C++ 3.1 eine BLADEM.EXE, die **Byte für Byte dem Original gleicht**:
deutsche CD-Fassung 3.02 (GOG, SCHICKM.EXE), 556.208 Bytes, MD5 `4ce035f95ead01a8b2553b4487544e5d`. Die EXE wird nicht
nachbearbeitet. Sie entsteht allein durch Compiler, Assembler und den Linker der Entwicklungsumgebung. Geprüft in der
Cloud-Werkbank **und auf einem echten Windows-Rechner** (GOG-DOSBox, 35 Sekunden).

| Teil | Endstand |
|---|---|
| Module | 111: 11 residente (seg001–seg013), 100 Overlays (seg024–seg122) |
| in C | **106** Module, rund 86.700 Zeilen. **Alles als C übersetzt, wie bei Attic** (Bright-Eyes übersetzte als C++) |
| in Assembler | 4 von Attic (seg008 VGA, seg009 PowerPacker, seg010 EMS, seg013 CD-Anfragen/Stack), dazu die zugekaufte Miles-Klangbibliothek (AIL 2.14). Alle kommentiert |
| Daten | Jedes Modul trägt seine Daten selbst. `datseg.c` gehört zum Hauptmodul seg002 (mit `main()`), die EMS-Daten zu seg010 |
| Namen | geprüft in sieben Wellen, **117 globale Umbenennungen**, dazu Hunderte lokale. Liste: `Analyse/Beschriftung/` |
| Kommentare | deutscher Kopfkommentar in jedem Modul, Spieltexte an 3.406 von 3.477 Textstellen |
| Fehler im Original | siehe `Analyse/Fehler/` (Zahlen unten) |
| Spieltexte | 95 Textdateien (4.858 Texte) und 20 Dialogdateien als Text in `Analyse/Spieltexte/` |
| Übersicht | `Analyse/Quelltextkarte.html`, Namensliste `Analyse/Namen/namen.tsv` |
| Kopierschutz | CD-Prüfung und Handbuch-/Kartenabfrage, unverändert und ohne Abschalter: `Analyse/Bericht_Kopierschutz.md` |
| Bauen | `Bauen/BAUEN.bat` (Windows, GOG-DOSBox), `Werkzeuge/b1.sh` (Linux-Werkbank) |

**Grundsatz** wie bei Sternenschweif: Was Attic in C hatte, steht in C. Was Attic in Assembler hatte, bleibt Assembler.
Der bytegleiche Build ist der Schiedsrichter. Die Namen stammen von Bright-Eyes und uns, Attics Bezeichner sind verloren.

## Zeitleiste

| Datum | Schritt |
|---|---|
| 17.10. | Bright-Eyes (df74edde) bytegleich gebaut, 16 echte Fehler im Bright-Eyes-Code gefunden, Absturz in `pass_item` behoben (Vorgeschichte: `Analyse/Abschlussbericht_Bytegleich_2026-10-17.md`) |
| 18.10. | mit BrightEyes 1686ed1 100 % identisch: Stack-Segment, Linken in der IDE. Patch an siebenstreich |
| 19.10. | neuer Stand auf 1686ed1, **ganz auf C umgestellt**, BSS-Reihenfolge, Krücken entfernt, Assembler in Attics Schreibweise, Spieltexte im Code, Bau-Paket für Windows |
| 19./20.10. | Beschriftungsprüfung aller Module in sieben Wellen, 117 Umbenennungen, Fehlerliste, Abgleich mit 328 Spielermeldungen aus dem Netz |

## Was wir über Attic und den Compiler gelernt haben

| Befund | Beleg |
|---|---|
| **Attic hat DSA 1 in C geschrieben.** Alle 106 C-Module sind als `.c` bytegleich, die C++-Krücken von Bright-Eyes fielen weg | Bau |
| **Gelinkt in der IDE.** `BC.EXE /b BLADEM.PRJ` linkt ohne Bildschirm, das Tastenskript ist überflüssig | Bau-Paket |
| **TASM mit `JUMPS`.** Die von Bright-Eyes ausgeschriebenen NOPs und Sprungumwege sind die automatische Arbeit von TASM. seg008 und seg009 sind damit zusammen 990 Zeilen kürzer (3.265 → 2.275) | seg008/009, auf Anhieb bytegleich |
| **BSS rückwärts:** Borland C legt nicht vorbelegte Variablen in umgekehrter Reihenfolge der ersten Deklaration an. Attics Header führte sie also absteigend | Testdateien, `datseg.h` |
| **Overlay-Einsprünge in der Reihenfolge der ersten Deklaration**: Die Prototypen in den Headern bestimmen die Einsprungtabellen | seg066 |
| **Deklarationen ohne Prototyp**, typisch für C 1992: `get_border_index()` und `sub_light_timers()` waren so deklariert, deshalb erweitern einige Aufrufer ihr Argument auf int, andere nicht | seg066, seg065/116 |
| **`goto`** im Statusmenü (C erzeugt dabei keinen toten Sprung wie C++) | seg048 |
| Bitfeld-Zugriffe: mal gefaltet, mal nicht – vermutlich ein Makro bei Attic. Wo nötig steht ein Cast mit Kommentar | seg036/037/042/043 |
| Ein **ungenutzter AdLib-Musiktreiber** (OPL2, eigener Timer-Interrupt) liegt in seg008, dazu Cursor- und EGA-Hilfen | seg008 |
| Der PowerPacker-Entpacker arbeitet **rückwärts**; deshalb kann ein Bild über seinen gepackten Daten entpackt werden | seg009 |

Alle Muster: `Analyse/LEITFADEN_C.md`.

## Beschriftung: wo Bright-Eyes falsch lag

Die Bytes verzeihen jeden falschen Namen. Geprüft haben wir deshalb gegen die MAP des Baus, gegen die Spieltexte und gegen das,
was der Code tut. Zusammen rund 970 Befunde, davon:

| Art | Beispiele |
|---|---|
| Adressen (bewiesen) | 35 falsche `ds:0x…`-Kommentare |
| vertauschte Parameter | `decomp_pp20` (Quelle/Ziel), `do_v_line` (x/y), `do_fill_rect` („Breite“ ist x2), `FANI_prepare_spell_enemy`, `GUI_write_char_to_screen` |
| Doku verkehrt herum | `range_attack_check_ammo` (Modi), `FIG_damage_enemy`, `is_enemies_turn`, „ist Fluchtfeld“, „consume quietly“ |
| Name passt nicht | `move` → `update_visual_squares`, `FIG_check_hero_attack` → `FIG_check_target_cursor_step`, `hunt_bison` → `hunt_aurochs`, `spell_gardanium` → `spell_gardianum`, `DNG_stub1–5` |
| nicht C-tauglich | zwei Namen, die für Borland gleich sind (nur 32 Zeichen zählen) |

Listen: `Analyse/Beschriftung/` (`korrekturen.tsv`, `umbenennungen_uebernommen.tsv`, Befunde je Welle).

## Fehler im Original

Bewusst so nachgebaut. Behoben wird erst, wenn auch Schatten über Riva fertig ist (Fehler-Schalter, Tests im Spiel).

**417 Einträge**, jeder an der Stelle im Code geprüft: 381 echte Fehler, 36 geprüft und kein Fehler. 47 lassen sich ausnutzen.
Liste: `Analyse/Fehler/Originalfehler.md`, Einordnung: `Analyse/Fehler/Fehler_Raster.md` / `.tsv`. Für Spieler (ohne Codestellen, mit Cheat-Liste): `Analyse/Fehler/Spieler/Fehlerliste_Schicksalsklinge.pdf` / `.docx` / `.md`.

| | kritisch | mittel | egal | Summe |
|---|---:|---:|---:|---:|
| **sicher** | 18 | 115 | 204 | 337 |
| **unsicher** | 7 | 32 | 41 | 80 |
| **Summe** | 25 | 147 | 245 | 417 |

| Herkunft | |
|---|---|
| von uns neu gefunden, vorher niemandem bekannt | **112** |
| Bright-Eyes-Markierungen geprüft | 178, davon bei 30 Begründung oder Fix falsch (u. a. Bug 18: der Fix erzeugt den Fehler erst; Bug 35, 47, 48: Fix falsch) |
| Spielermeldungen aus dem Netz (Crystals-Foren u. a.) | 328 gesammelt und am Code geprüft: 183 bestätigt, 31 teilweise, 32 widerlegt, 42 nur Daten/Technik, 40 ungeklärt. 107 Einträge kamen dadurch neu dazu (Nr. 311–417) |

Die schwersten, z. B.: ESC beim Stufenanstieg schenkt rund 168.000 Dukaten (Nr. 248) · Schiffstempo in 16 Bit, Division durch 0 (137) ·
Magisches Bastardschwert „benutzen“ stürzt ab (214) · Böser Blick auf einen Priester: Endlosschleife (22) · Kraftgürtel-Exploit
in `move_item` (222) · verfluchtes Gold in Hyggeliks Ruine: jeder Kampf gilt sofort als gewonnen (362) · Zufallsbegegnungen
auf Reisen fast tot, weil der Vergleich verkehrt herum steht.

## Was bleibt

| Punkt | |
|---|---|
| 7 `_AX`-Zuweisungen (seg032/034/039/081/082/090) | Byte-Argumente, die das Original auf int erweitert; natürlich nur mit größerem Umbau der Header zu lösen |
| Bitfeld-Casts in seg036/037/042/043 | vermutlich Attics Makro, nicht rekonstruiert |
| GPL | Der Code bleibt GPL (Bright-Eyes). Für eine geschlossene Nutzung durch den Rechteinhaber braucht es die Zustimmung der Bright-Eyes-Autoren |
| später | Fehlertests im Spiel und Fehler-Patches per Schalter, nach Schatten über Riva |
