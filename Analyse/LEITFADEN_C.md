# Leitfaden: Wie Attic DSA 1 geschrieben und übersetzt hat

Gesammelte Befunde aus der Rekonstruktion (Oktober 2026). Jede Regel ist am bytegleichen Bau geprüft.
Werkzeug: Borland C++ 3.1 (BCC, TASM 3.1, Linker der IDE).

## Übersetzen und Linken

| Befund | Beleg |
|---|---|
| **Attic hat DSA 1 in C geschrieben, nicht in C++.** Alle 106 C-Module sind als `.c` bytegleich | Bau, MD5 `4ce035f9…` |
| Optionen: `-mlarge -O- -c -1 -Y` (resident) bzw. `-Yo` (Overlay), AIL mit TASM `/m /w+ /ml` | `Quelle/build.bat` |
| **Gelinkt wurde in der IDE** (BC.EXE, Projekt BLADEM.PRJ). TLINK schreibt den Programmnamen in `_EXEINFO_` klein, die IDE groß wie im Original. `BC.EXE /b BLADEM.PRJ` linkt ohne Bildschirm | Nachtrag 18.10. im Abschlussbericht 2026-10-17, Bau-Paket |
| Linkdatum 12.01.1994 (steht in der EXE) | `DATUM.COM` im Bau-Paket |
| **TASM mit `JUMPS`**, ein Durchlauf (`/os /z`). Vorwärtssprünge mit kurzem Ziel bekommen 3 NOPs, zu weite bedingte Sprünge werden Gegensprung + `jmp`, `loop` außer Reichweite wird `loop $+4 / jmp short / jmp near`. Bright-Eyes hatte das von Hand nachgebaut (`NOP3`) | seg009.asm, auf Anhieb bytegleich |
| Der 4-KB-Stack in seg013 hat den Kombinationstyp `STACK` und gehört zu DGROUP | Abschlussbericht 17./18.10. |

## C-Modus von BCC 3.1: Reihenfolgen

| Befund | Folge für den Quelltext |
|---|---|
| **BSS rückwärts:** Nicht vorbelegte Variablen eines Moduls legt BCC in **umgekehrter Reihenfolge ihrer ersten Deklaration** an (C++: Definitionsreihenfolge). Gemessen mit Testdateien (`int a1; int b1;` → b1 vor a1; `extern`-Zeilen zählen als erste Deklaration) | Alle BSS-Variablen stehen in `datseg.h` je Modul **absteigend** nach Adresse (Block „generiert von bssorder.py“). Attics Header muss sie ebenso geführt haben |
| **Öffentliche Namen (PUBDEF) in der Reihenfolge der ersten Deklaration.** Daran hängen die Einsprungtabellen der Overlays | Die Prototypen in `segNNN.h` stehen in der Reihenfolge der Einsprünge (Kommentare `/* 0x2a */`). Ein Prototyp darf nicht verschoben werden |

## C-Modus: Ausdrücke

| Befund | Beispiel |
|---|---|
| Aufruf **ohne Prototyp**: Argumente werden auf int erweitert (`mov ah,0` bzw. `cbw`) | seg066: Header deklariert `signed int get_border_index();`, der Prototyp steht erst vor `town_set_visual_square_vals()`. Die beiden frühen Aufrufe erweitern, die späten nicht |
| **`goto` ohne toten Sprung** (C++ setzt hinter `goto` einen toten Zusatzsprung) | seg048 `status_menu`: frühere Fälle springen mit `goto` in das gemeinsame Ende |
| **Bitfelder:** Der natürliche Zugriff `g_enemy_sheets[i].flags.dead` wird in C oft zur Adresse gefaltet (`mov al,[bx+sym+0x31]`), in C++ nicht | seg032/033/034/038/098 natürlich. In seg036/037/042/043 ist das Original an manchen Stellen ungefaltet (`mov dx,sym / add dx,0x31`). Dort steht ein Cast, vermutlich hatte Attic ein Makro |
| Cross-Jumping: gleich endende Stücke werden zusammengelegt, eine (auch unbenutzte) Sprungmarke verhindert das | seg106 `pass_item` |
| Lokale Strukturkopie: `struct s a; ... a = g;` an der Stelle der Zuweisung (eine initialisierte Deklaration kopiert am Blockanfang) | seg063 `do_harbor` |
| Unbenannte Unions gibt es in C nicht | `struct struct_chest`: `lock.key`, `loot.content` |
| Borland unterscheidet Namen nur nach 32 Zeichen | `trv_journey_dest_town_viewdir` |

## Compilerfehler: nicht vorbelegter Speicher

BCC 3.1 liest bei der Registerwahl für `switch` zwei nie beschriebene Bytes. Das Ergebnis hängt davon ab, was vorher übersetzt
wurde (bei Sternenschweif O030). Bei DSA 1 bisher ohne Wirkung: Alle Module übersetzen einzeln und im Stapel gleich.
Gebaut wird trotzdem immer im Stapel in der Reihenfolge von `build.bat`.
