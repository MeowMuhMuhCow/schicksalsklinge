/*
 * seg075 - Dungeon: 3D-Ansicht, Licht, Tueren, Fallgruben, Betreten (allgemein 1/2)
 *
 * Inhalt:    Zeichnet die Dungeonansicht: Decke und Boden (DNG_floor_ceil), verdeckte
 *            Felder ausblenden (DNG_hide_occluded_squares), Feldart -> Grafiknummer
 *            (DNG_set_squares_tex), alle sichtbaren Felder von hinten nach vorn zeichnen
 *            (DNG_draw_squares, DNG_draw_image) und auf den Bildschirm kopieren (DNG_show_view).
 *            DNG_lights() dunkelt die Palette je nach Lichtquelle ab und setzt
 *            gs_dungeon_light. Dazu Schritt vor/zurueck mit Todesfalle (DNG_timestep),
 *            Ebenenwechsel, Tueranimation, Fallgruben (DNG_pitfall, DNG01_pitfall mit
 *            Seil/Zauberstab) und die Startposition jedes Dungeons (DNG_enter_dungeon).
 * Aufrufer:  Dungeonhauptschleife und Dungeonmenue (seg076), Dungeonereignisse
 *            seg077-seg092 (DNG_pitfall, DNG_inc/dec_level, DNG_update_pos), seg025
 *            (DNG_lights), seg081/seg118 (DNG_check_climb_tools), Reise/Stadt
 *            (DNG_enter_dungeon).
 * Daten:     g_dng_gfxtab_wood/_stone/_marble (je 88 Eintraege: bis zu drei Teilbilder mit
 *            Versatz je Grafik), g_dng_tex_wall..9 (Grafiknummer je Feldart und
 *            Sichtfeld), g_dng_square_pos..3 (Grundposition je Sichtfeld),
 *            g_dng_square_draw_order (Zeichenreihenfolge der 22 Sichtfelder).
 * Besonders: DNG_draw_image(): oben angeschnittene Bilder werden um 2*|y| Zeilen zu hoch
 *            kopiert ("height2 -= y" statt "+="), gespiegelte angeschnittene Bilder zeigen
 *            vermutlich den falschen Ausschnitt. DNG01_pitfall(): die Namensliste der
 *            Kletterer nimmt einfach die Heldenplaetze 0,1,2,... ohne Gruppenpruefung.
 *            DNG_timestep(): die Todesfalle beendet das Spiel auch fuer Gruppen
 *            ausserhalb (Bright-Eyes-Hinweis).
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg075.c
 */

#include <string.h>
#include <stdio.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg003.h"
#include "seg004.h"
#include "seg007.h"
#include "seg008.h"
#include "seg027.h"
#include "seg028.h"
#include "seg047.h"
#include "seg049.h"
#include "seg075.h"
#include "seg096.h"
#include "seg097.h"
#include "seg106.h"

struct dng_gfxtab g_dng_gfxtab_wood[88] = {
	 { { {      0,      0 }, {      8,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, { -32754,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {      4,     19 }, {      0,      9 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {      5,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {      3,    -34 }, {      0, -32753 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,    151 }, {      0,     12 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {      6,    102 }, {      0,     10 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {      6,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {      6,    -23 }, {      0, -32752 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,    -69 }, {      0, -32750 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {      7,     56 }, {      0,     13 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {      7,     56 }, {      0,     11 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {      7,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {      7,    -12 }, {      0, -32751 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {      7,    -12 }, {      0, -32749 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     39,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     37,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, { -32729,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     40,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     38,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, { -32728,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     43,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     41,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, { -32725,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     44,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     42,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, { -32724,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {     36,      5 }, {     45,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {     24,      0 }, {     46,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {     14,      0 }, {     47,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,     85 }, {     48,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,     48 }, {     49,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {     35,     12 }, {     51,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {     22,     10 }, {     52,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {     13,      6 }, {     53,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     25,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, { -32743,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     21,     19 }, {      0,     26 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     22,     26 }, {     23,     31 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     20,    -34 }, {      0, -32742 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,    151 }, {      0,     29 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     23,    102 }, {      0,     27 }, {     16,     13 } },     32 },
	 { { {      0,     -1 }, {     23,     16 }, {     13,     32 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     23,    -23 }, {      0, -32741 }, {     16,     13 } },     32 },
	 { { {      0,      0 }, {     -1,    -69 }, {      0, -32739 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,     56 }, {      0,     30 }, {      8,      8 } },     33 },
	 { { {      0,      0 }, {     24,     56 }, {      0,     28 }, {      8,      8 } },     33 },
	 { { {      0,      0 }, {     24,      8 }, {      8,     33 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,    -12 }, {      0, -32740 }, {      8,      8 } },     33 },
	 { { {      0,      0 }, {     24,    -12 }, {      0, -32738 }, {      8,      8 } },     33 },
	 { { {      0,     -1 }, {     22,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,    151 }, {      0,     29 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     23,    102 }, {      0,     27 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     23,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     23,    -23 }, {      0, -32741 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,    -69 }, {      0, -32739 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,     56 }, {      0,     30 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,     56 }, {      0,     28 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,    -12 }, {      0, -32740 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,    -12 }, {      0, -32738 }, {      0,      0 } },     -1 },
	 { { {     35,     32 }, {     55,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {     22,     27 }, {     56,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {     13,     16 }, {     57,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     22,     26 }, {     23,     34 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     23,    102 }, {      0,     27 }, {     16,     13 } },     35 },
	 { { {      0,     -1 }, {     23,     16 }, {     13,     35 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     23,    -23 }, {      0, -32741 }, {     16,     13 } },     35 },
	 { { {      0,      0 }, {     24,     56 }, {      0,     30 }, {      8,      8 } },     36 },
	 { { {      0,      0 }, {     24,     56 }, {      0,     28 }, {      8,      8 } },     36 },
	 { { {      0,      0 }, {     24,      8 }, {      8,     36 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,    -12 }, {      0, -32740 }, {      8,      8 } },     36 },
	 { { {      0,      0 }, {     24,    -12 }, {      0, -32738 }, {      8,      8 } },     36 },
	 { { {      0,      0 }, {     -1,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },

}; // ds:0x7dea
struct dng_gfxtab g_dng_gfxtab_stone[88] = {
	 { { {      0,      0 }, {      8,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, { -32754,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {      4,     19 }, {      0,      9 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {      5,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {      3,    -34 }, {      0, -32753 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,    151 }, {      0,     12 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {      6,    102 }, {      0,     10 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {      6,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {      6,    -23 }, {      0, -32752 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,    -69 }, {      0, -32750 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {      7,     56 }, {      0,     13 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {      7,     56 }, {      0,     11 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {      7,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {      7,    -12 }, {      0, -32751 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {      7,    -12 }, {      0, -32749 }, {      0,      0 } },     -1 },
	 { { {      0,      2 }, {     39,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      2,      2 }, {     37,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      2 }, { -32729,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     40,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      7,      0 }, {     38,      7 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, { -32728,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     43,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     41,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, { -32725,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     44,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     42,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, { -32724,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     22,     41 }, {     42,     45 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     23,     22 }, {     24,     46 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     24,     11 }, {     10,     47 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     22,     39 }, {     40,     48 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     23,     19 }, {     19,     49 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     24,     11 }, {      9,     -1 }, {      0,      0 } },     -1 },
	 { { {     47,    -14 }, {     51,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {     22,     -6 }, {     52,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {     13,     -1 }, {     53,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     25,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, { -32743,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     21,     19 }, {      0,     26 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     22,     35 }, {     29,     31 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     20,    -34 }, {      0, -32742 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,    151 }, {      0,     29 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     23,    102 }, {      0,     27 }, {     19,     18 } },     32 },
	 { { {      0,     -1 }, {     23,     19 }, {     18,     32 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     23,    -23 }, {      0, -32741 }, {     19,     18 } },     32 },
	 { { {      0,      0 }, {     -1,    -69 }, {      0, -32739 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,     56 }, {      0,     30 }, {     11,      9 } },     33 },
	 { { {      0,      0 }, {     24,     56 }, {      0,     28 }, {     11,      9 } },     33 },
	 { { {      0,      0 }, {     24,     11 }, {      9,     33 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,    -12 }, {      0, -32740 }, {     11,      9 } },     33 },
	 { { {      0,      0 }, {     24,    -12 }, {      0, -32738 }, {     11,      9 } },     33 },
	 { { {      0,     -1 }, {     22,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,    151 }, {      0,     29 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     23,    102 }, {      0,     27 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     23,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     23,    -23 }, {      0, -32741 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,    -69 }, {      0, -32739 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,     56 }, {      0,     30 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,     56 }, {      0,     28 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,    -12 }, {      0, -32740 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,    -12 }, {      0, -32738 }, {      0,      0 } },     -1 },
	 { { {     39,     32 }, {     55,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {     24,     20 }, {     56,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {     13,     16 }, {     57,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     22,     35 }, {     29,     34 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     23,    102 }, {      0,     27 }, {     19,     18 } },     35 },
	 { { {      0,     -1 }, {     23,     19 }, {     18,     35 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     23,    -23 }, {      0, -32741 }, {     19,     18 } },     35 },
	 { { {      0,      0 }, {     24,     56 }, {      0,     30 }, {     11,      9 } },     36 },
	 { { {      0,      0 }, {     24,     56 }, {      0,     28 }, {     11,      9 } },     36 },
	 { { {      0,      0 }, {     24,     11 }, {      9,     36 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,    -12 }, {      0, -32740 }, {     11,      9 } },     36 },
	 { { {      0,      0 }, {     24,    -12 }, {      0, -32738 }, {     11,      9 } },     36 },
	 { { {      0,      0 }, {     -1,    151 }, {      0,     12 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     23,     22 }, {     24,     46 }, {    102,     -4 } },     10 },
	 { { {      0,     -1 }, {     23,     22 }, {     24,     46 }, {    -23,     -4 } }, -32752 },
	 { { {      0,      0 }, {     -1,    -69 }, {      0, -32750 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,    151 }, {      0,     12 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     23,     19 }, {     19,     49 }, {    102,     -4 } },     10 },
	 { { {      0,     -1 }, {     23,     19 }, {     19,     49 }, {    -23,     -4 } }, -32752 },
	 { { {      0,      0 }, {     -1,    -69 }, {      0, -32750 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {      4,     19 }, {      0,      9 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {      3,    -34 }, {      0, -32753 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,     56 }, {      0,     30 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,     56 }, {      0,     28 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,    -12 }, {      0, -32740 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,    -12 }, {      0, -32738 }, {      0,      0 } },     -1 },
}; // ds:0x841a
struct dng_gfxtab g_dng_gfxtab_marble[88] = {
	 { { {      0,      0 }, {      8,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, { -32754,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,     -4 }, {      4,     19 }, {     -5,      9 }, {      0,      0 } },     -1 },
	 { { {      0,     -4 }, {      5,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,     -4 }, {      3,    -34 }, {     -5, -32753 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,    151 }, {      0,     12 }, {      0,      0 } },     -1 },
	 { { {      0,     -3 }, {      6,    102 }, {     -4,     10 }, {      0,      0 } },     -1 },
	 { { {      0,     -3 }, {      6,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,     -3 }, {      6,    -23 }, {     -4, -32752 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,    -69 }, {      0, -32750 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {      7,     56 }, {      0,     13 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {      7,     54 }, {     -1,     11 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {      7,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {      7,    -10 }, {     -1, -32751 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {      7,    -12 }, {      0, -32749 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     39,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     37,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, { -32729,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     40,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     38,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, { -32728,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     43,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     41,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, { -32725,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     44,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     42,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, { -32724,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,     -4 }, {      5,     27 }, {      0,     45 }, {      0,      0 } },     -1 },
	 { { {      0,     -3 }, {      6,     18 }, {      0,     46 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {      7,      9 }, {      0,     47 }, {      0,      0 } },     -1 },
	 { { {      0,     -4 }, {      5,     26 }, {      0,     48 }, {      0,      0 } },     -1 },
	 { { {      0,     -3 }, {      6,     17 }, {     -1,     49 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {     59,      4 }, {     51,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {     37,      3 }, {     52,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {     22,      3 }, {     53,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     25,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, { -32743,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     21,     19 }, {      0,     26 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     22,     28 }, {     15,     31 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     20,    -34 }, {      0, -32742 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,    151 }, {      0,     29 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     23,    102 }, {      0,     27 }, {     16,      8 } },     32 },
	 { { {      0,     -1 }, {     23,     16 }, {      8,     32 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     23,    -23 }, {      0, -32741 }, {     16,      8 } },     32 },
	 { { {      0,      0 }, {     -1,    -69 }, {      0, -32739 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,     56 }, {      0,     30 }, {      8,      4 } },     33 },
	 { { {      0,      0 }, {     24,     56 }, {      0,     28 }, {      8,      4 } },     33 },
	 { { {      0,      0 }, {     24,      8 }, {      4,     33 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,    -12 }, {      0, -32740 }, {      8,      4 } },     33 },
	 { { {      0,      0 }, {     24,    -12 }, {      0, -32738 }, {      8,      4 } },     33 },
	 { { {      0,     -1 }, {     22,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,    151 }, {      0,     29 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     23,    102 }, {      0,     27 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     23,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     23,    -23 }, {      0, -32741 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,    -69 }, {      0, -32739 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,     56 }, {      0,     30 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,     56 }, {      0,     28 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,    -12 }, {      0, -32740 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,    -12 }, {      0, -32738 }, {      0,      0 } },     -1 },
	 { { {     53,     52 }, {     55,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {     35,     36 }, {     56,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {     21,     21 }, {     57,      0 }, {      0,     -1 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     22,     28 }, {     15,     34 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     23,    102 }, {      0,     27 }, {     17,      8 } },     35 },
	 { { {      0,     -1 }, {     23,     17 }, {      8,     35 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {     23,    -23 }, {      0, -32741 }, {     17,      8 } },     35 },
	 { { {      0,      0 }, {     24,     56 }, {      0,     30 }, {     10,      2 } },     36 },
	 { { {      0,      0 }, {     24,     56 }, {      0,     28 }, {     10,      2 } },     36 },
	 { { {      0,      0 }, {     24,     10 }, {      2,     36 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     24,    -12 }, {      0, -32740 }, {     10,      2 } },     36 },
	 { { {      0,      0 }, {     24,    -12 }, {      0, -32738 }, {     10,      2 } },     36 },
	 { { {      0,      0 }, {     -1,    151 }, {      0,     12 }, {      0,      0 } },     -1 },
	 { { {      0,     -3 }, {      6,     18 }, {      0,     46 }, {    102,     -4 } },     10 },
	 { { {      0,     -3 }, {      6,     18 }, {      0,     46 }, {    -23,     -4 } }, -32752 },
	 { { {      0,      0 }, {     -1,    -69 }, {      0, -32750 }, {      0,      0 } },     -1 },
	 { { {      0,      0 }, {     -1,    151 }, {      0,     12 }, {      0,      0 } },     -1 },
	 { { {      0,     -3 }, {      6,     17 }, {     -1,     49 }, {    102,     -4 } },     10 },
	 { { {      0,     -3 }, {      6,     17 }, {     -1,     49 }, {    -23,     -4 } }, -32752 },
	 { { {      0,      0 }, {     -1,    -69 }, {      0, -32750 }, {      0,      0 } },     -1 },
	 { { {      0,     -4 }, {      4,     19 }, {     -5,      9 }, {      0,      0 } },     -1 },
	 { { {      0,     -4 }, {      3,    -34 }, {     -5, -32753 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {      7,     56 }, {      0,     13 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {      7,     54 }, {     -1,     11 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {      7,    -10 }, {     -1, -32751 }, {      0,      0 } },     -1 },
	 { { {      0,     -1 }, {      7,    -12 }, {      0, -32749 }, {      0,      0 } },     -1 },
}; // ds:0x8a4a
static unsigned char g_dng_tex_wall[22] = { 0x01, 0xff, 0x02, 0xff, 0x03, 0x04, 0x05, 0xff, 0x06, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0a, 0xff, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0xff }; // ds:0x907a
static unsigned char g_dng_tex_pit[22] = { 0xff, 0xff, 0xff, 0xff, 0x10, 0x11, 0x12, 0xff, 0xff, 0xff, 0x13, 0x14, 0x15, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }; // ds:0x9090
static unsigned char g_dng_tex_pit_in_ceiling[22] = { 0xff, 0xff, 0xff, 0xff, 0x16, 0x17, 0x18, 0xff, 0xff, 0xff, 0x19, 0x1a, 0x1b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }; // ds:0x90a6
static unsigned char g_dng_tex_stair_up[22] = { 0x01, 0xff, 0x02, 0xff, 0x53, 0x1c, 0x54, 0xff, 0x4b, 0x4b, 0x4c, 0x1d, 0x4d, 0x4e, 0x4e, 0xff, 0x55, 0x56, 0x1e, 0x57, 0x58, 0xff }; // ds:0x90bc
static unsigned char g_dng_tex_stair_down[22] = { 0x01, 0xff, 0x02, 0xff, 0x53, 0x1f, 0x54, 0xff, 0x4f, 0x4f, 0x50, 0x20, 0x51, 0x52, 0x52, 0xff, 0x55, 0x56, 0x21, 0x57, 0x58, 0xff }; // ds:0x90d2
static unsigned char g_dng_tex_blocker[22] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0x22, 0xff, 0xff, 0xff, 0xff, 0xff, 0x23, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x24, 0xff, 0xff, 0xff }; // ds:0x90e8
static unsigned char g_dng_tex_closed_door[22] = { 0x25, 0xff, 0x26, 0xff, 0x27, 0x28, 0x29, 0xff, 0x2a, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2e, 0xff, 0x2f, 0x30, 0x31, 0x32, 0x33, 0xff }; // ds:0x90fe
static unsigned char g_dng_tex_open_door[22] = { 0x25, 0xff, 0x26, 0xff, 0x27, 0x34, 0x29, 0xff, 0x35, 0x35, 0x36, 0x37, 0x38, 0x39, 0x39, 0xff, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0xff }; // ds:0x9114
static unsigned char g_dng_tex_smashed_door[22] = { 0x25, 0xff, 0x26, 0xff, 0x27, 0x42, 0x29, 0xff, 0x2a, 0x2a, 0x43, 0x44, 0x45, 0x2e, 0x2e, 0xff, 0x46, 0x47, 0x48, 0x49, 0x4a, 0xff }; // ds:0x912a
static unsigned char g_dng_tex_chest[22] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x40, 0x40, 0x40, 0x40, 0x40, 0xff, 0xff, 0x41, 0x41, 0x41, 0x41, 0x41, 0xff }; // ds:0x9140
static struct struct_point g_dng_square_pos[22] = {
	{   0,  1}, {  -1, -1}, { 189,  1}, { -19, 14},
	{   0, 14}, {  19, 14}, { 189, 14}, { 359, 14},
	{-253, 46}, {-0x0097, 46}, {-0x0031, 36}, {53, 36},
	{0x009b, 36}, {0x0101, 46}, {0x0167, 46}, {-0x005c, 50},
	{-0x0024, 50}, {20, 50}, {76, 50}, {0x0084, 50},
	{0x00bc, 50}, {0x00f4, 50}
}; // ds:0x9156
static struct struct_point g_dng_square_pos_stairs[22] = {
	{0x0000, 0x0001},
	{-0x0001, -0x0001},
	{0x00bd, 0x0001},
	{-0x0013, 0x000e},
	{0x0000, 0x000e},
	{0x0013, 0x000e},
	{0x00bd, 0x000e},
	{0x0167, 0x000e},
	{-0x00fd, 0x002e},
	{-0x0097, 0x002e},
	{-0x0031, 0x0024},
	{0x0035, 0x0024},
	{0x009b, 0x0024},
	{0x0101, 0x002e},
	{0x0167, 0x002e},
	{-0x0001, -0x0001},
	{-0x0024, 0x0032},
	{0x0014, 0x0032},
	{0x004c, 0x0032},
	{0x0084, 0x0032},
	{0x00bc, 0x0032},
        {-0x0001, -1}
}; // ds:0x91ae
static struct struct_point g_dng_square_pos_pit[22] = {
	{ -0x0001, -0x0001},
        { -0x0001, -0x0001},
	{ -0x0001, -0x0001},
        { -0x0001, -0x0001},
        {  0x0000, 0x0066},
        { 0x0037, 0x0065},
        { 0x00bc, 0x0066},
	{ -0x0001, -0x0001},
	{ -0x0001, -0x0001},
	{ -0x0001, -0x0001},
	{ 0x0000, 0x0057},
	{ 0x004a, 0x0057},
	{ 0x00a0, 0x0057},
	{ -0x0001, -0x0001},
        { -0x0001, -0x0001},
	{ -0x0001, -0x0001},
	{ -0x0001, -0x0001},
	{ -0x0001, -0x0001},
	{ -0x0001, -0x0001},
	{ -0x0001, -0x0001},
	{ -0x0001, -0x0001},
	{ -0x0001, -1}
}; // ds:0x9206
static struct struct_point g_dng_square_pos_pit_ceil[22] = {
	{-1, -1},
	{-1, -1},
        {-1, -1},
	{-1, -1},
	{ 2, 19},
	{ 0x003e, 0x0013},
	{ 0x00bc, 0x0013},
	{-1, -1},
	{-1, -1},
	{-1, -1},
	{ 0x0002, 0x0023},
	{ 0x0051, 0x0023},
	{ 0x00a4, 0x0023},
	{-1, -1},
	{-1, -1},
	{-1, -1},
	{-1, -1},
	{-1, -1},
	{-1, -1},
	{-1, -1},
        {-1, -1},
	{-1, -1}
}; // ds:0x925e
static signed char g_dng_square_draw_order[22] = { 0x0f, 0x10, 0x15, 0x14, 0x11, 0x13, 0x12, 0x08, 0x09, 0x0e, 0x0d, 0x0a, 0x0c, 0x0b, 0x03, 0x04, 0x07, 0x06, 0x05, 0x00, 0x02, 0x01 }; // ds:0x92b6
static char g_dng01_str_comma[3] = ", "; // ds:0x92cc
static char g_dng01_str_and[6] = " UND "; // ds:0x92cf

/**
 * \brief   draws ceiling and floor of the dungeon view into g_renderbuf
 *
 *          g_dng_floor_tex toggles with every step (DNG_timestep()) and selects the
 *          mirrored variant (compression type 5), which makes the movement visible.
 */
/* static */
void DNG_floor_ceil(void)
{
	struct nvf_extract_desc nvf;
	signed int width;
	signed int height;

	/* Load ceiling */
	nvf.dst = g_renderbuf_ptr;
	nvf.src = g_buffer9_ptr3;
	nvf.image_num = 0;
	nvf.compression_type = !g_dng_floor_tex ? 3 : 5;
	nvf.width = &width;
	nvf.height = &height;
	process_nvf_extraction(&nvf);


	/* Load floor (former comment said "ceiling"; lower part of the view, image 1 or 2 by viewdir) */
	nvf.dst = g_renderbuf_ptr + 0x4030;
	nvf.src = g_buffer9_ptr3;
	nvf.image_num = (!(gs_viewdir&1)) ? 1 : 2;
	nvf.compression_type = !g_dng_floor_tex ? 3 : 5;
	nvf.width = &width;
	nvf.height = &height;
	process_nvf_extraction(&nvf);

}

/**
 * \brief   hides squares of the view which are covered by walls or closed doors
 *
 *          A visual square whose tile type is a wall (15) or a closed door (1) covers
 *          the squares behind it; their values are set to 0 and are not drawn.
 *          Layout of g_visual_square_vals: 0,1,2 = left, own, right square; then rows
 *          3..7, 8..14, 15..21, 22..28 further ahead (center 5, 11, 18, 25).
 *          (formerly named DNG_turn, but it has nothing to do with turning)
 */
static void DNG_hide_occluded_squares(void)
{
	signed int tmp;

	if ( ((tmp = div16(g_visual_square_vals[3])) == 15) || (tmp == 1)) {
		g_visual_square_vals[3] = 0;
	}

	if ( ((tmp = div16(g_visual_square_vals[7])) == 15) || (tmp == 1)) {
		g_visual_square_vals[7] = 0;
	}

	if ( ((tmp = div16(g_visual_square_vals[8])) == 15) || (tmp == 1)) {
		g_visual_square_vals[8] = 0;
	}

	if ( ((tmp = div16(g_visual_square_vals[14])) == 15) || (tmp == 1)) {
		g_visual_square_vals[14] = 0;
	}

	if ( ((tmp = div16(g_visual_square_vals[15])) == 15) || (tmp == 1)) {
		g_visual_square_vals[15] = 0;
	}

	if ( ((tmp = div16(g_visual_square_vals[21])) == 15) || (tmp == 1)) {
		g_visual_square_vals[21] = 0;
	}

	if ( ((tmp = div16(g_visual_square_vals[0])) == 15) || (tmp == 1)) {

		g_visual_square_vals[9] = g_visual_square_vals[15] = g_visual_square_vals[22] = 0;

		if ( ((tmp = div16(g_visual_square_vals[5])) == 15) || (tmp == 1)) {

			g_visual_square_vals[4] = g_visual_square_vals[10] = g_visual_square_vals[16] = g_visual_square_vals[23] = 0;
		}
	}

	if ( ((tmp = div16(g_visual_square_vals[2])) == 15) || (tmp == 1)) {

		g_visual_square_vals[13] = g_visual_square_vals[21] = g_visual_square_vals[28] = 0;

		if ( ((tmp = div16(g_visual_square_vals[5])) == 15) || (tmp == 1)) {

			g_visual_square_vals[6] = g_visual_square_vals[12] = g_visual_square_vals[20] = g_visual_square_vals[27] = 0;
		}
	}

	if ( ((tmp = div16(g_visual_square_vals[4])) == 15) || (tmp == 1)) {

		g_visual_square_vals[9] = g_visual_square_vals[16] = g_visual_square_vals[22] = g_visual_square_vals[23] = 0;

		if ( ((tmp = div16(g_visual_square_vals[5])) == 15) || (tmp == 1)) {

			g_visual_square_vals[10] = 0;
		}

		if ( ((tmp = div16(g_visual_square_vals[11])) == 15) || (tmp == 1)) {

			g_visual_square_vals[10] = g_visual_square_vals[17] = g_visual_square_vals[18] = 0;

			g_visual_square_vals[24] = g_visual_square_vals[25] = 0;
		}
	}

	if ( ((tmp = div16(g_visual_square_vals[5])) == 15) || (tmp == 1)) {

		g_visual_square_vals[11] = g_visual_square_vals[17] = g_visual_square_vals[18] = 0;

		g_visual_square_vals[19] = g_visual_square_vals[24] = g_visual_square_vals[25] = 0;

		g_visual_square_vals[26] = 0;
	}

	if ( ((tmp = div16(g_visual_square_vals[6])) == 15) || (tmp == 1)) {

		g_visual_square_vals[13] = g_visual_square_vals[20] =
			g_visual_square_vals[27] = g_visual_square_vals[28] = 0;

		if ( ((tmp = div16(g_visual_square_vals[5])) == 15) || (tmp == 1)) {

			g_visual_square_vals[11] = g_visual_square_vals[12] = g_visual_square_vals[17] = 0;

			g_visual_square_vals[18] = g_visual_square_vals[19] = g_visual_square_vals[24] = 0;

			g_visual_square_vals[25] = g_visual_square_vals[26] = 0;

		}

		if ( ((tmp = div16(g_visual_square_vals[11])) == 15) || (tmp == 1)) {

				g_visual_square_vals[12] = g_visual_square_vals[18] = g_visual_square_vals[19] = 0;

				g_visual_square_vals[25] = g_visual_square_vals[26] = 0;
		}
	}

	if ( ((tmp = div16(g_visual_square_vals[9])) == 15) || (tmp == 1)) {

		g_visual_square_vals[22] = 0;

		if ( ((tmp = div16(g_visual_square_vals[17])) == 15) || (tmp == 1)) {

			g_visual_square_vals[16] = g_visual_square_vals[23] = 0;
		}
	}

	if ( ((tmp = div16(g_visual_square_vals[13])) == 15) || (tmp == 1)) {

		g_visual_square_vals[28] = 0;

		if ( ((tmp = div16(g_visual_square_vals[19])) == 15) || (tmp == 1)) {

			g_visual_square_vals[20] = g_visual_square_vals[27] = 0;
		}
	}

	if ( ((tmp = div16(g_visual_square_vals[10])) == 15) || (tmp == 1)) {

		g_visual_square_vals[9] = g_visual_square_vals[16] = g_visual_square_vals[22] = g_visual_square_vals[23] = 0;

		if ( ((tmp = div16(g_visual_square_vals[11])) == 15) || (tmp == 1)) {

			g_visual_square_vals[18] = 0;
		}

		if ( ((tmp = div16(g_visual_square_vals[11])) == 15) || (tmp == 1) ||
			((tmp = div16(g_visual_square_vals[18])) == 15) || (tmp == 1)) {

			g_visual_square_vals[17] = g_visual_square_vals[24] = g_visual_square_vals[25] = 0;
		}
	}

	if ( ((tmp = div16(g_visual_square_vals[12])) == 15) || (tmp == 1)) {

		g_visual_square_vals[13] = g_visual_square_vals[20] = g_visual_square_vals[27] = g_visual_square_vals[28] = 0;

		if ( ((tmp = div16(g_visual_square_vals[11])) == 15) || (tmp == 1)) {

			g_visual_square_vals[18] = 0;
		}

		if ( ((tmp = div16(g_visual_square_vals[11])) == 15) || (tmp == 1) ||
			((tmp = div16(g_visual_square_vals[18])) == 15) || (tmp == 1)) {

			g_visual_square_vals[19] = g_visual_square_vals[25] = g_visual_square_vals[26] = 0;
		}
	}

	if ( ((tmp = div16(g_visual_square_vals[11])) == 15) || (tmp == 1)) {
		g_visual_square_vals[18] = g_visual_square_vals[25] = 0;
	}

	if ( ((tmp = div16(g_visual_square_vals[16])) == 15) || (tmp == 1)) {

		g_visual_square_vals[22] = 0;

		if ( ((tmp = div16(g_visual_square_vals[24])) == 15) || (tmp == 1)) {

			g_visual_square_vals[23] = 0;
		}
	}

	if ( ((tmp = div16(g_visual_square_vals[20])) == 15) || (tmp == 1)) {

		g_visual_square_vals[28] = 0;

		if ( ((tmp = div16(g_visual_square_vals[26])) == 15) || (tmp == 1)) {

			g_visual_square_vals[27] = 0;
		}
	}

	if ( ((tmp = div16(g_visual_square_vals[18])) == 15) || (tmp == 1)) {

		g_visual_square_vals[25] = 0;
	}

	if ( ((tmp = div16(g_visual_square_vals[18])) == 15) || (tmp == 1) ||
		((tmp = div16(g_visual_square_vals[25])) == 15) || (tmp == 1)) {

		if ( ((tmp = div16(g_visual_square_vals[17])) == 15) || (tmp == 1)) {

			g_visual_square_vals[24] = 0;
		}

		if ( ((tmp = div16(g_visual_square_vals[19])) == 15) || (tmp == 1)) {

			g_visual_square_vals[26] = 0;
		}
	}
}

/**
 * \brief   renders the complete dungeon view and copies it to the screen
 */
void DNG_render_view(void)
{
	DNG_floor_ceil();
	update_visual_squares();
	DNG_hide_occluded_squares();
	DNG_set_squares_tex();
	DNG_draw_squares();
	DNG_draw_doorframe();
	DNG_show_view();
}

/**
 * \brief   extra graphic if the group stands on an open or smashed door and faces a wall
 *
 *          Draws image 0x36 at the top of the view (x depends on the graphics style),
 *          probably the frame of the door the group stands in.
 */
void DNG_draw_doorframe(void)
{
	signed int tmp;

	tmp = div16(g_visual_square_vals[1]);

	if ((tmp == 2) || (tmp == 9)) {

		if (div16(g_visual_square_vals[5]) == 15) {
			DNG_draw_image( ((gs_dungeon_gfx_style == 1) ? 0x4e : ((gs_dungeon_gfx_style == 2) ? 0x28 : 0x3e)), 0, 0x36);
		}
	}
}

/**
 * \brief   determines the graphic (index into g_dng_gfxtab, -1 = nothing) of each visual square
 *
 *          by tile type: 6 pit, 5 pit in ceiling, 3 stair down, 4 stair up, 7 blocker,
 *          1 closed door, 2 open door, 9 smashed door, 8 chest, 10/11/15 walls.
 */
void DNG_set_squares_tex(void)
{
	signed int i;
	signed int tmp;

	for (i = 21; i >= 0; i--) {

		tmp = div16(g_visual_square_vals[i]);

		g_visual_squares_tex[i] =
			(tmp == 6) ? g_dng_tex_pit[i] :
				(tmp == 5) ? g_dng_tex_pit_in_ceiling[i] :
				(tmp == 3) ? g_dng_tex_stair_down[i] :
				(tmp == 4) ? g_dng_tex_stair_up[i] :
				(tmp == 7) ? g_dng_tex_blocker[i] :
				(tmp == 1) ? g_dng_tex_closed_door[i] :
				(tmp == 2) ? g_dng_tex_open_door[i] :
				(tmp == 9) ? g_dng_tex_smashed_door[i] :
				(tmp == 8) ? g_dng_tex_chest[i] :
				(tmp == 10) ? g_dng_tex_wall[i] :
				(tmp == 11) ? g_dng_tex_wall[i] :
				(tmp != 15) ? -1 : g_dng_tex_wall[i];
	}
}

/**
 * \brief   draws one dungeon graphic (wall, door, stairs, chest, ...) into the view
 *
 *          The image is extracted to g_renderbuf_ptr + 0x7530 and copied with
 *          transparency (color 128) and clipping to the 208x135 view.
 *
 * \param   x           x position in the view (may be negative)
 * \param   y           y position in the view (may be negative)
 * \param   image_id    image number; bit 0x8000 = draw mirrored, bit 0x4000 is masked out
 *                      here (evaluated by the caller DNG_draw_squares())
 */
void DNG_draw_image(signed int x, signed int y, signed int image_id)
{
	signed int nvf_width;
	signed int nvf_height;
	signed int width2;
	signed int height2;
	signed int mirrored;
	uint8_t *src_ptr;
	uint8_t *dst_ptr;
	struct nvf_extract_desc nvf;

	mirrored = (image_id & 0x8000) ? 1 : 0;
	image_id &= 0x3fff;

	/* images 8..13 and 14..19 swap with every step (like floor and ceiling) */
	if ((image_id >= 8) && (image_id <= 13) && !g_dng_floor_tex) {
		image_id += 6;
	} else {
		if ((image_id >= 14) && (image_id <= 19) && !g_dng_floor_tex) {
			image_id -= 6;
		}
	}

	nvf.dst = src_ptr = g_renderbuf_ptr + 0x7530;
	nvf.src = (uint8_t*)g_buffer9_ptr3;
	nvf.image_num = image_id;
	nvf.width = &nvf_width;
	nvf.height = &nvf_height;
	nvf.compression_type = 3;
	process_nvf_extraction(&nvf);

	width2 = nvf_width;
	height2 = nvf_height;

	/* clip at the left border.
	 * REMARK: for mirrored images this skips the wrong columns: copy_solid_permuted() starts at
	 * src_ptr + width2 - 1, so the leftmost source columns are dropped instead of the rightmost
	 * ones (vermutlich wrong part shown; the same holds for the right border below). */
	if ((x < 0) && ((x + width2) > 0)) {
		width2 += x;
		src_ptr += __abs__(x);
		x = 0;
	}

	/* clip at the top border.
	 * Original-Bug: should be "height2 += y" as for x above. With y < 0 the height grows by |y|
	 * instead of shrinking, so up to 2*|y| rows behind the image are copied (only cut off by the
	 * clipping at row 135 below). */
	if ((y < 0) && ((y + height2) > 0)) {
		height2 -= y;
		src_ptr += __abs__(y) * nvf_width;
		y = 0;
	}

	if ((x < 208) && (y < 135) && (x >= 0) && (y >= 0)) {

		if (x + width2 > 208) {
			width2 = 208 - x;
		}

		if (y + height2 > 135) {
			height2 = 135 - y;
		}

		dst_ptr = g_renderbuf_ptr + 208 * y + x;

		if (!mirrored) {
			copy_solid(dst_ptr, src_ptr, width2, height2, 208, nvf_width, 128);
		} else {
			copy_solid_permuted(dst_ptr, src_ptr + width2 - 1, width2, height2, 208, nvf_width, 128);
		}
	}
}

/**
 * \brief   draws all 22 visible squares of the view, from the back to the front
 *
 *          The base position of a square depends on its tile type (pit, pit in ceiling,
 *          stairs or other). Each g_dng_gfxtab entry holds up to three images with
 *          offsets: (a[0], a[1].x), (a[1].y/a[2].x, a[2].y), (a[3], a[4].x).
 *          Images with bit 0x4000 are only drawn if the square's direction (val & 3)
 *          points towards the viewer.
 */
void DNG_draw_squares(void)
{
	signed int x;
	signed int y;
	signed char idx; /* first the index of the visual square, then its graphic (g_visual_squares_tex) */
	unsigned char tile_type;
	unsigned char square_val;
	/* REMARK: not sure if (x.y) is the correct terminology here */
	struct struct_point *ptr;

	signed int i;
	signed int j;

	for (i = 0; i < 22; i++) {

		idx = g_dng_square_draw_order[i];

		square_val = g_visual_square_vals[idx];

		tile_type = (unsigned char)div16(square_val);

		ptr = (tile_type == 6) ? &g_dng_square_pos_pit[idx] :
			(tile_type == 5) ? &g_dng_square_pos_pit_ceil[idx] :
			(tile_type == 4) ? &g_dng_square_pos_stairs[idx] :
			(tile_type == 3) ? &g_dng_square_pos_stairs[idx] : &g_dng_square_pos[idx];


		x = ptr[0].x;
		y = ptr[0].y;

		idx = g_visual_squares_tex[idx];

		if (idx != -1) {

			ptr = (struct struct_point*)&g_dng_gfxtab[idx - 1].a[0];

			if ((j = ptr[1].x) != -1) {

				if ((j & 0x4000) && ((((square_val & 3) + 2) & 3) != gs_viewdir)) {
				} else {
					DNG_draw_image(x + ptr[0].x, y + ptr[0].y, j);
				}
			}

			if ((j = ptr[2].y) != -1) {

				if ((j & 0x4000) && ((((square_val & 3) + 2) & 3) != gs_viewdir)) {
				} else {
					DNG_draw_image(x + ptr[1].y, y + ptr[2].x, j);
				}
			}

			if ((j = ptr[4].x) != -1) {

				if ((j & 0x4000) && ((((square_val & 3) + 2) & 3) != gs_viewdir)) {
				} else {
					DNG_draw_image(x + ptr[3].x, y + ptr[3].y, j);
				}
			}
		}
	}
}

/**
 * \brief   draws the compass, copies the rendered view to the screen and adjusts the palette to the light
 */
void DNG_show_view(void)
{
	draw_compass();

	g_pic_copy.x1 = g_ani_posx;
	g_pic_copy.y1 = g_ani_posy;
	g_pic_copy.x2 = g_ani_posx + 207;
	g_pic_copy.y2 = g_ani_posy + 134;
	g_pic_copy.src = g_renderbuf_ptr;

	call_mouse_bg();

	g_special_screen = 0;

	wait_for_vsync();

	do_pic_copy(1);

	DNG_lights();

	call_mouse();
}

/**
 * \brief   check for stafflevel >= 2 in current group
 *
 * \return              0 = false, 1 = true
 */
signed int is_staff_lvl2_in_group(void)
{
	struct struct_hero *hero_i;
	signed int i;

	hero_i = get_hero(0);
	for (i = 0; i <= 6; i++, hero_i++) {

		if ((hero_i->typus) && (hero_i->group_id == gs_active_group_id) &&
			check_hero(hero_i) && (hero_i->staff_level >= 2))
		{
			return 1;
		}
	}

	return 0;
}

/**
 * \brief   darkens the dungeon palette according to the light of the group
 *
 *          Attenuation 0 (light) .. 10 (dark): darkness timer -> 10; FLIM FLAM or a staff
 *          with level >= 2 (Ewige Flamme) -> 0; otherwise 10 - burning time of the best
 *          torch/lantern. If bit 0 of the own square is set (probably a lit square), it is
 *          light unless the darkness timer runs. Sets gs_dungeon_light (0 light, 1 at
 *          attenuation 9, 2 at 10), which is used for the automap and pitfalls.
 *          Does nothing if the own square has tile type 11.
 */
void DNG_lights(void)
{
	if (div16(g_visual_square_vals[1]) != 11) {

		signed int light_attenuation;
		signed char color;
		signed int i;

		/* copy palette */
		memcpy(g_text_output_buf, g_buffer11_ptr, 0xc0);

		if (!(g_visual_square_vals[1] & 1)) {

			if (gs_ingame_timers[INGAME_TIMER_DARKNESS]) {
				light_attenuation = 10;
			} else if (gs_ingame_timers[INGAME_TIMER_FLIM_FLAM] || is_staff_lvl2_in_group()) {
				light_attenuation = 0;
			} else {
				if ( (light_attenuation = get_max_light_time()) != -1) {
					light_attenuation = 10 - light_attenuation;
				} else {
					light_attenuation = 10;
				}
			}
		} else {
			if (gs_ingame_timers[INGAME_TIMER_DARKNESS]) {
				light_attenuation = 10;
			} else {
				light_attenuation = 0;
			}
		}

		gs_dungeon_light = ((light_attenuation == 9) ? 1 : (light_attenuation == 10) ? 2 : 0);

		/* set the color palette with respect to the lighting timer */
		light_attenuation *= 3;

		for (i = 0; i < 0xc0; i++) {

			color = g_text_output_buf[i] - light_attenuation;

			if (color < 0) {
				color = 0;
			}

			g_text_output_buf[i] = color;
		}

		wait_for_vsync();
		set_palette((uint8_t*)g_text_output_buf, 0x80, 0x40);
	}
}

/**
 * \brief   performs a step (forward or backward) in a dungeon. time advances by 1 minute.
 *
 * \param forward[in]	{ 1 = forward step, -1 = backward step}
 */
void DNG_timestep(const signed int forward)
{
	signed int viewdir;

	timewarp(MINUTES(1));

	if (gs_deathtrap && !(--gs_deathtrap_steps)) {
		/* oh oh: death trap is activated and there are no remaining steps in the dungeon... */

		if (gs_deathtrap == 1) {
			/* It is the death trap on the Totenschiff. -> it sinks...  */

			load_ani(18);
			init_ani(1);

			GUI_output(get_tx(23)); /* SHIP.DTX: "URPLÖTZLICH BEGINNT DAS GESAMTE SCHIFF ZU BEBEN..." */
		}

		gs_dungeon_id = DUNGEON_ID_NONE;

		/* exit game */
		/* Original-Bug: What if there is another group not on the Totenschiff?? They should survive. */
		g_game_state = GAME_STATE_DEAD;
	}

	if (g_game_state == GAME_STATE_MAIN) {

		viewdir = gs_viewdir;

		if (forward == 1) {

			/* go forward */

			if (!viewdir) {
				/* north */
				gs_y--;
			} else if (viewdir == EAST) {
				/* east */
				gs_x++;
			} else if (viewdir == SOUTH) {
				/* south */
				gs_y++;
			} else {
				/* west */
				gs_x--;
			}
		} else {

			/* go backward */

			if (!viewdir) {
				/* north */
				gs_y++;
			} else if (viewdir == EAST) {
				/* east */
				gs_x--;
			} else if (viewdir == SOUTH) {
				/* south */
				gs_y--;
			} else {
				/* west */
				gs_x++;
			}
		}

		g_dng_floor_tex ^= 1;
	}
}

/**
 * \brief   redraws the dungeon view and remembers position and direction it was drawn for
 */
void DNG_update_pos(void)
{
	disable_ani();

	DNG_render_view();

	g_dng_refresh_x_target = gs_x;
	g_dng_refresh_y_target = gs_y;
	g_dng_refresh_direction = gs_viewdir;
}

void DNG_inc_level(void)
{
	g_dng_level_changed = 1;
	gs_dungeon_level++;
	load_area_description(1);
	DNG_update_pos();
}

void DNG_dec_level(void)
{
	g_dng_level_changed = 1;
	gs_dungeon_level--;
	load_area_description(1);
	DNG_update_pos();
}

/**
 * \brief   animation of the door ahead opening (the door square is drawn as open door, 0x20)
 */
void DNG_open_door(void)
{
	signed int x;
	signed int y;
	signed int iters;
	signed int i;

	DNG_floor_ceil();
	update_visual_squares();
	g_visual_square_vals[5] = 32;
	DNG_hide_occluded_squares();
	DNG_set_squares_tex();
	DNG_draw_squares();

	memmove((uint8_t*)g_renderbuf_ptr + 0x7530, g_renderbuf_ptr, 0x6db0);

	if (!gs_dungeon_gfx_style) {
		x = 45;
		y = 38;
		iters = 19;
	} else if (gs_dungeon_gfx_style == 1) {
		x = 47;
		y = 30;
		iters = 20;
	} else {
		x = 54;
		y = 44;
		iters = 17;
	}

	play_voc(ARCHIVE_FILE_FX15_VOC);

	call_mouse_bg();

	for (i = 0; i < iters; i++) {

		door_frame(31, x, y, (i * 4) + 1);

		DNG_show_view();

		memmove(g_renderbuf_ptr, g_renderbuf_ptr + 30000, 28080);
	}

	call_mouse();
}

/**
 * \brief   animation of the door ahead closing (open door animation backwards)
 */
void DNG_close_door(void)
{
	signed int x;
	signed int y;
	signed int iters;
	signed int i;

	DNG_floor_ceil();
	update_visual_squares();
	DNG_hide_occluded_squares();
	DNG_set_squares_tex();
	DNG_draw_squares();

	memmove(g_renderbuf_ptr + 30000, g_renderbuf_ptr, 28080);

	if (!gs_dungeon_gfx_style) { /* dungeon graphics: wood */
		x = 45;
		y = 38;
		iters = 18;
	} else if (gs_dungeon_gfx_style == 1) { /* dungeon graphics: marble */
		x = 47;
		y = 30;
		iters = 19;
	} else { /* dungeon graphics: stone */
		x = 54;
		y = 44;
		iters = 16;
	}

	play_voc(ARCHIVE_FILE_FX15_VOC);

	call_mouse_bg();

	for (i = iters; i >= 0; i--) {

		door_frame(31, x, y, (i * 4) + 1);

		DNG_show_view();

		memmove(g_renderbuf_ptr, g_renderbuf_ptr + 30000, 28080);
	}

	call_mouse();
}

/**
 * \brief   pit on the Totenschiff (DNG01), called from seg077
 *
 *          In the dark (gs_dungeon_light != 0) a random part of the group falls down
 *          (DNG_pitfall(6)). With light the group sees the pit and climbs down: with a
 *          rope, a rope ladder or a mage with staff level >= 3 and at least two heroes,
 *          otherwise every hero of the group loses 1..5 LE (random_schick(5)). Then one level down.
 */
void DNG01_pitfall(void)
{
	play_voc(ARCHIVE_FILE_FX18_VOC);

	if (gs_dungeon_light) {

		if (DNG_pitfall(6)) {
			gs_x = gs_x_bak;
			gs_y = gs_y_bak;
		}
	} else {

		signed int l_si; /* REMARK: climb_result, counter, le_damage */
		signed int nr_heroes;
		signed int strongest_hero_pos;
		struct struct_hero *hero_auto;
		struct struct_hero *hero1;
		struct struct_hero *hero2;

		if (((l_si = DNG_check_climb_tools()) != -1) && ((nr_heroes = count_heroes_available_in_group()) > 1))
		{

			/* ropes oder staff */
			*g_dtp2 = '\0';

			if (l_si) {

				sprintf(g_dtp2, get_ttx(768), get_hero(l_si - 1)->alias); /* "%s VERWANDELT DEN ZAUBERSTAB IN EIN SEIL.@" */
			}

			if (nr_heroes == 2) {

				hero1 = get_first_hero_available_in_group();

				hero2 = get_second_hero_available_in_group();

				sprintf(g_text_output_buf, get_ttx(769), hero1->alias, hero2->alias); /* "%s KLETTERT AN DEM SEIL IN DIE TIEFE UND VERSUCHT DANN, %s AUFZUFAN..." */

				strcat(g_dtp2, g_text_output_buf);

				if (test_attrib(hero1, ATTRIB_GE, 2) <= 0) {

					sprintf(g_text_output_buf, get_ttx(770), hero1->alias, hero2->alias, (l_si = random_schick(3) + 1)); /* "@LEIDER STELLT SICH %s RECHT UNGESCHICKT AN UND SO FÜGT SICH %s DEN..." */

					strcat(g_dtp2, g_text_output_buf);

					sub_hero_le(hero2, l_si);
				}
			} else {

				hero_auto = get_hero((strongest_hero_pos = get_hero_KK_best()));

				nr_heroes--;

				l_si = 0;

				/* list the names of the other climbers.
				 * Original-Bug: takes the hero slots 0, 1, 2, ... (only skipping the strongest hero)
				 * without checking typus and group_id. If the group is split or a slot is empty,
				 * wrong or empty names are listed. */
				do {

					if (l_si == strongest_hero_pos) {
						l_si++;
					}

					strcat(g_dtp2, get_hero(l_si++)->alias);

					if (--nr_heroes) {

						strcat(g_dtp2, (nr_heroes >= 2 ? g_dng01_str_comma : g_dng01_str_and));
					}

				} while (nr_heroes);

				sprintf(g_text_output_buf, get_ttx(771), hero_auto->alias, /* "KLETTERN AN DEM SEIL IN DIE TIEFE, WÄHREND %s DAS SEIL HÄLT. ANSCHL..." */
					GUI_get_personal_pronoun(hero_auto->sex, GRAMMAR_CASE_1ST),
					GUI_get_personal_pronoun(hero_auto->sex, GRAMMAR_CASE_4TH));

				strcat(g_dtp2, g_text_output_buf);
			}

			GUI_output(g_dtp2);
		} else {

			sub_group_le(random_schick(5));
		}

		DNG_inc_level();
	}
}

/**
 * \brief check for climb tools in the inventory (stafflevel included)
 * \return {-1 = no, 0 = rope or ladder, otherwise position + 1 of a mage with stafflevel > 2 (staff spell "Seil des Adepten")}
 **/
signed int DNG_check_climb_tools(void)
{
	signed int i;
	struct struct_hero *hero = get_hero(0);

	/* check for a mage with staffspell > 2 */
	for (i = 0; i <= 6; i++, hero++) {

		/* TODO: potential Original-Bug: What if petrified / unconscious etc.? Compare to is_staff_lvl2_in_group where check_hero is called */
		if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
			!hero->flags.dead && (hero->typus == HERO_TYPE_MAGIER) && (hero->staff_level > 2))
		{
			return i + 1;
		}
	}

	/* check for ladder or rope */
	return ((get_first_hero_with_item(ITEM_ID_SEIL) != -1) || (get_first_hero_with_item(ITEM_ID_STRICKLEITER) != -1)) ? 0 : -1;
}

/**
 * \brief   A random number of heroes falls to the next lower dungeon level.
 *
 * \param   max_damage  Each fallen hero takes a random amount of LE damage in the range [1 .. max_damage].
 * \return  0: all heroes fell. / 1: only some heroes fell (forming a new group).
 */
signed int DNG_pitfall(const signed int max_damage)
{
	signed int hero_pos;
	signed int i;
	signed int num_fallen_heroes;
	signed int new_group_id;
	signed int retval;

	new_group_id = 0;
	retval = 0;

	g_dng_level_changed = 1;
	num_fallen_heroes = random_schick(gs_group_member_counts[gs_active_group_id]);

	/* If the roll indicates that all but one hero of the active group should fall, then all heroes will fall instead.
	 * Reason (probably): Prevent the NPC from ending up isolated as a single-hero group.
	 */

	if (gs_group_member_counts[gs_active_group_id] - 1 == num_fallen_heroes) {
		num_fallen_heroes = gs_group_member_counts[gs_active_group_id];
	}

	if (gs_group_member_counts[gs_active_group_id] != num_fallen_heroes) {
		/* Only part of the heroes in the active group falls. -> new group is needed */

		/* find empty group */
		while (gs_group_member_counts[new_group_id]) {
			new_group_id++;
		}

		for (i = 0; i < num_fallen_heroes; i++) {

			do {
				hero_pos = random_schick(7) - 1;

			} while (!get_hero(hero_pos)->typus || (get_hero(hero_pos)->group_id != gs_active_group_id) ||
					((num_fallen_heroes == 1) && (hero_pos == 6))); /* avoid that the NPC gets separated into a single group */

			get_hero(hero_pos)->group_id = (unsigned char)new_group_id;
			gs_group_member_counts[new_group_id]++;
			gs_group_member_counts[gs_active_group_id]--;
			sub_hero_le(get_hero(hero_pos), random_schick(max_damage));
		}

		GRP_save_pos(new_group_id);
		gs_groups_dng_level[new_group_id] = gs_dungeon_level + 1;

		retval = 1;

	} else {
		/* all heroes fall */

		hero_pos = 0;

		for (i = 0; i < num_fallen_heroes; i++) {

			while (!get_hero(hero_pos)->typus || (get_hero(hero_pos)->group_id != gs_active_group_id))
			{
				hero_pos++;
			}

			sub_hero_le(get_hero(hero_pos++), random_schick(max_damage));
		}

		DNG_inc_level();
	}

	return retval;
}

/**
 * \brief   enters a dungeon: sets start position, view direction and level, remembers the town
 *
 *          The Zwingfeste is entered with a fade-out of the palette.
 *
 * \param   dungeon_id  DUNGEON_ID_*
 */
void DNG_enter_dungeon(const signed int dungeon_id)
{
	signed int x_pos;
	signed int y_pos;
	signed int level;
	signed int viewdir;
	signed int i;
	uint8_t *ptr;

	switch (dungeon_id) {

		case DUNGEON_ID_TOTENSCHIFF:  x_pos = 9;  y_pos = 11;  viewdir = NORTH;  level = 2;  break;
		case DUNGEON_ID_VERFALLENE_HERBERGE:  x_pos = 1;  y_pos = 1;   viewdir = SOUTH;  level = 0;  break;
		case DUNGEON_ID_SPINNENHOEHLE:  x_pos = 1;  y_pos = 8;   viewdir = EAST;  level = 0;  break;

#if !defined(__BORLANDC__)
		case DUNGEON_ID_WOLFSHOEHLE:  x_pos = 7;  y_pos = 14;  viewdir = NORTH;  level = 0;  break;
#else
mark1:		case DUNGEON_ID_WOLFSHOEHLE:  x_pos = 7;  y_pos = 14;  viewdir = NORTH;  level = 0;  break;
#endif

		case DUNGEON_ID_GOBLINHOEHLE:  x_pos = 6;  y_pos = 14;  viewdir = NORTH;  level = 0;  break;
		case DUNGEON_ID_DASPOTASCHATZ:  x_pos = 13; y_pos = 14;  viewdir = NORTH;  level = 0;  break;
		case DUNGEON_ID_RUINE_DES_SCHWARZMAGIERS:  x_pos = 1;  y_pos = 13;  viewdir = NORTH;  level = 0;  break;
		case DUNGEON_ID_ORKBEHAUSUNG:  x_pos = 1;  y_pos = 14;  viewdir = EAST;  level = 0;  break;

#if !defined(__BORLANDC__)
		case DUNGEON_ID_KULTSTAETTE_DES_NAMENLOSEN:  x_pos = 7;  y_pos = 14;  viewdir = NORTH;  level = 0;  break;
#else
		case DUNGEON_ID_KULTSTAETTE_DES_NAMENLOSEN:  goto mark1;
mark2:			   goto mark1;
#endif

		case DUNGEON_ID_DRACHENHORT:  x_pos = 1;  y_pos = 3;   viewdir = EAST;  level = 0;  break;

#if !defined(__BORLANDC__)
		case DUNGEON_ID_PIRATENHOEHLE:  x_pos = 7;  y_pos = 14;  viewdir = NORTH;  level = 0;  break;
#else
		case DUNGEON_ID_PIRATENHOEHLE:  goto mark2;
#endif

		case DUNGEON_ID_ZWERGENFESTE:  x_pos = 13; y_pos = 14;  viewdir = NORTH;  level = 0;  break;
		case DUNGEON_ID_VERLASSENE_MINE: {
			x_pos = 7;
			y_pos = 14;
			viewdir = NORTH;
			level = 0;

			gs_dng13_collapsecount = 0;
			gs_dng13_herocount = count_heroes_in_group();
			break;
		}
		case DUNGEON_ID_ZWINGFESTE: {
			x_pos = 1;
			y_pos = 14;
			viewdir = WEST;
			level = gs_town_loc_type = LOCTYPE_NONE; /* == 0 */
			break;
		}
		case DUNGEON_ID_HYGGELIKS_RUINE:  x_pos = 1;  y_pos = 11;   viewdir = EAST;  level = 0;  break;
	}

	gs_x = x_pos;
	gs_y = y_pos;
	gs_viewdir = viewdir;
	gs_dungeon_level = level;
	gs_dungeon_id = dungeon_id;
	gs_town_loc_type_bak = gs_town_loc_type;
	gs_town_id_bak = gs_town_id;
	gs_town_loc_type = gs_town_id = TOWN_ID_NONE;
	g_dng_loaded_dungeon_id = g_town_loaded_town_id = -1;

	if (dungeon_id == DUNGEON_ID_ZWINGFESTE) {

		ptr = g_renderbuf_ptr + 500;

#if defined(__BORLANDC__)
		memset(g_renderbuf_ptr, 0, 0x120);
		memcpy(g_renderbuf_ptr + 500, gs_palette_floor, 0x120);
#else
		memset(g_renderbuf_ptr, 0, 3 * 0x20);
		memcpy(g_renderbuf_ptr + 500, gs_palette_floor, 3 * 0x20);
#endif

		for (i = 0; i < 0x40; i++) {

			pal_fade((int8_t*)ptr, (int8_t*)g_renderbuf_ptr);
			pal_fade((int8_t*)ptr + 0x60, (int8_t*)g_renderbuf_ptr + 0x60);
			pal_fade((int8_t*)ptr + 0xc0, (int8_t*)g_renderbuf_ptr + 0xc0);

			wait_for_vsync();

			set_palette(ptr, 0, 0x20);
			set_palette(ptr + 0x60, 0x80, 0x40);
		}

		do_fill_rect(g_renderbuf_ptr, 0, 0, 319, 199, 0);
		g_pic_copy.x1 = 0;
		g_pic_copy.y1 = 0;
		g_pic_copy.x2 = 240;
		g_pic_copy.y2 = 136;
		g_pic_copy.src = g_renderbuf_ptr;
		call_mouse_bg();
		do_pic_copy(1);
		call_mouse();
	}
}
