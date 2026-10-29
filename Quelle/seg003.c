/*
 * seg003 - Sichtfeld in Stadt und Dungeon, Tuer-Animation, Leichen pluendern
 *
 * Inhalt:    update_visual_squares() bestimmt aus Position und Blickrichtung die 29 sichtbaren Kartenfelder
 *            (g_visual_square_vals) sowie die Felder vor und hinter der Gruppe; bewegt wird dabei nichts.
 *            update_direction() dreht die Gruppe, door_frame() zeichnet ein Bild der Tuer-Animation,
 *            loot_corpse() fragt beim Fund einer Leiche, ob man sie untersucht oder Borons Segen spricht.
 * Aufrufer:  Stadt- und Dungeonsteuerung (seg066, seg075, seg076), Dungeon-Ereignisse (loot_corpse).
 * Daten:     g_visual_square_north/east/south/west (je 31 Versaetze, die letzten zwei fuer vorne/hinten).
 * Besonders: door_frame(): Ausschnitt fuer y < 0 vergroessert die Hoehe statt sie zu verkleinern
 *            (Original-Bug, wirkt nicht, da y immer >= 0 ist). Leiche untersuchen: Boron -20, Segen: Boron +20.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Y seg003.c
 */

#include <stdio.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg008.h"
#include "seg097.h"

#if 0
static const char no_way_msg[][41] = {
	"IN DIESE RICHTUNG GEHT ES NICHT WEITER.",
	"YOU CAN GO NO FURTHER IN THIS DIRECTION."
};
#endif

/**
 * \brief   dreht die Gruppe
 *
 * \param   mod         Drehung in Vierteln (1 = rechts, 3 = links)
 * \return              immer -1
 */
signed int update_direction(const unsigned char mod)
{
	/* save old direction */
	gs_viewdir_bak = gs_viewdir;
	/* set new direction */
	gs_viewdir = ((gs_viewdir + mod) & 0x3);

	/* set bogus variable to 1 */
	/* (g_viewdir_flag_bogus wird nur geschrieben, nie gelesen) */
	g_viewdir_flag_bogus = 1;

	return -1;
}

/**
 * \brief   ermittelt die sichtbaren Kartenfelder vor der Gruppe
 *
 *          Fuellt g_visual_square_vals[29] aus der Karte (Felder ausserhalb der Karte werden zu 0xb0,
 *          ausser 0xa0/0xb0) und setzt g_steptarget_front/back auf die Felder vor und hinter der Gruppe.
 *          Trotz des Namens wird die Gruppe nicht bewegt.
 */
void update_visual_squares(void)
{
	volatile signed int boundary_flag;
	unsigned char mapval;
	volatile signed int i;
	uint8_t *p_map_small;
	uint8_t *p_map_large;
	struct point8s *p_visual_square;

	signed int x;
	signed int y;

	p_map_small = p_map_large = g_dng_map;

	/* direction */
#if defined(__BORLANDC__)
	p_visual_square = (struct point8s*)MK_FP(_DS,
				((gs_viewdir == 0) ? FP_OFF(g_visual_square_north) :
				((gs_viewdir == 1) ? FP_OFF(g_visual_square_east) :
				((gs_viewdir == 2) ? FP_OFF(g_visual_square_south) : FP_OFF(g_visual_square_west)))));
#else
	p_visual_square = ((gs_viewdir == 0) ? g_visual_square_north :
			((gs_viewdir == 1) ? g_visual_square_east :
			((gs_viewdir == 2) ? g_visual_square_south : g_visual_square_west)));

#endif

	for (i = 0; i < 29; i++, p_visual_square++) {
		boundary_flag = 0;
		x = gs_x + p_visual_square->x;
		y = gs_y + p_visual_square->y;

		if (x < 0) {
			x = 0;
			boundary_flag = 1;
		} else {
			if (g_map_size_x - 1 < x) {
				x = g_map_size_x - 1;
				boundary_flag = 1;
			}
		}

		if (y < 0) {
			y = 0;
			boundary_flag = 1;
		} else {
			if (y > 15) {
				y = 15;
				boundary_flag = 1;
			}
		}

		mapval = (g_map_size_x == 16) ?
			/* dungeon or small town */
			*(p_map_small + MAP_POS(x,y )) :
			/* large city */
			*(p_map_large + LARGE_MAP_POS(x, y));

		if (boundary_flag != 0) {
			g_visual_square_vals[i] = ((mapval == 0xa0) || (mapval == 0xb0) ? mapval : 0xb0);
		} else {
			g_visual_square_vals[i] = mapval;
		}
	}

	/* p_visual_square zeigt nach der Schleife auf Eintrag 29 der Tabelle: [0] = Feld vor, [1] = Feld hinter der Gruppe */
	if (g_map_size_x == 16) {
		/* dungeon or small town */
		g_steptarget_front = *(p_map_small + MAP_POS(gs_x + p_visual_square[0].x, gs_y + p_visual_square[0].y));

		g_steptarget_back  = *(p_map_small + MAP_POS(gs_x + p_visual_square[1].x, gs_y + p_visual_square[1].y));
	} else {
		/* large city */
		g_steptarget_front = *(p_map_large + LARGE_MAP_POS(gs_x + p_visual_square[0].x, gs_y + p_visual_square[0].y));

		g_steptarget_back  = *(p_map_large + LARGE_MAP_POS(gs_x + p_visual_square[1].x, gs_y + p_visual_square[1].y));
	}
}

/**
 * \brief   zeichnet ein Bild der Tuer-Animation in den Renderpuffer (208x135)
 *
 * \param   no          Bildnummer in der geladenen Dungeon-NVF
 * \param   x           X-Koordinate im Sichtfenster
 * \param   y           Y-Koordinate im Sichtfenster
 * \param   frame       Anzahl der oben abgeschnittenen Zeilen; je groesser, desto weiter ist die Tuer aufgeschoben
 */
void door_frame(const signed int no, signed int x, signed int y, const signed int frame)
{
	signed int width;
	signed int height;
	signed int src_width; /* hiess l1 */
	uint8_t *p1;
	uint8_t *p2;
	struct nvf_extract_desc nvf;

	nvf.dst = (uint8_t*)((HugePt)g_buffer9_ptr3 + 0x2e248L);
	nvf.src = (uint8_t*)g_buffer9_ptr3;
	nvf.image_num = no;
	nvf.compression_type = 3;
	nvf.width = &width;
	nvf.height = &height;
	process_nvf_extraction(&nvf);

	height -= frame;
	src_width = width;

	p1 = (uint8_t*)((((HugePt)g_buffer9_ptr3) + frame * width) + 0x2e248L);

	if ((x < 0) && ((x + width) > 0)) {
		width += x;
		p1 += __abs__(x);
		x = 0;
	}

	if ((y < 0) && ((y + height) > 0)) {
		/* Original-Bug: muesste height += y heissen (wie oben width += x); so wird die Hoehe groesser.
		 * Wirkt nicht, alle Aufrufer (seg075) uebergeben y >= 0. */
#if FIX_0281
		/* Fix 281: Anschnitt oben verkleinert die Hoehe */
		height += y;
#else
		height -= y;
#endif
		p1 += __abs__(y) * src_width;
		y = 0;
	}

	if ( (x < 208) && (y < 135) && (x >= 0) && (y >= 0)) {

		if (x + width > 208) {
			width = 208 - x;
		}

		if (y + height > 135) {
			height = 135 - y;
		}

		p2 = g_renderbuf_ptr + 208 * y + x;

		copy_solid(p2, p1, width, height, 208, src_width, 0);
	}
}

/**
 * \brief   looting of a corpse
 *
 * \param   chest_ptr   a pointer to the chest description
 * \param   text        output text
 * \param   flag        to mark this chest as done
 *
 *          Untersuchen: Boron -20 und Beute ueber chest_ptr->loot.func, Borons Segen: Boron +20.
 */
void loot_corpse(struct struct_chest* chest_ptr, const char *text, int8_t *flag)
{
	signed int answer;

	sprintf(g_text_output_buf, (char*)(!(*flag) ? get_ttx(523) : get_ttx(527)), text); /* "%s@WOLLT IHR" / "%s IHR KÖNNT NICHTS ENTDECKEN, WAS FÜR EUC..." */

	if (!(*flag)) {

		g_textbox_width = 7;

		answer = GUI_radio(g_text_output_buf, 3, get_ttx(524), get_ttx(525), get_ttx(526)) - 1; /* "DIE LEICHE UNTERSUCHEN?" / "BORONS SEGEN FÜR DEN TOTEN ER..." / "EUCH ABWENDEN UND GEHEN?" */

		g_textbox_width = 3;

		if (answer == 0) {

			/* examine the corpse */
			chest_ptr->loot.func(chest_ptr);

			if (!(*flag))	{

				/* mark this corpse as done */
				*flag = 1;
				/* Boron - 20 */
				gs_gods_estimation[GOD_ID_BORON] -= 20L;

			}

		} else if (answer == 1) {

			/* Borons Blessing */
			if (!(*flag)) {
				/* mark this corpse as done */
				*flag = 1;
				/* Boron + 20 */
				gs_gods_estimation[GOD_ID_BORON] += 20L;
			}
		}
	} else {
		GUI_output(g_text_output_buf);
	}
}

/* "IN DIESE RICHTUNG GEHT ES NICHT WEITER." */
void no_way(void)
{
	GUI_output(g_noway_string);
}
