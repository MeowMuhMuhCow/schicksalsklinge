/*
 * seg049 - Gruppenverwaltung und Nachtruhe
 *
 * Inhalt:    Sortiert die Heldenplätze (aktive Gruppe zuerst), speichert und
 *            wechselt Gruppenpositionen, teilt und vereinigt Gruppen, tauscht
 *            Helden per Menü oder durch Ziehen des Portraits und berechnet die
 *            Regeneration eines Helden im Schlaf (GRP_hero_sleep, dort auch
 *            Seekrankheit und Abschluss des Brauens).
 * Aufrufer:  Hauptschleife/Kampf (seg002, seg032), Menüs in Stadt und Dungeon
 *            (seg066, seg076), Lager/Herberge (seg051, seg052, seg054),
 *            Schiffsreise (seg063), Alchimie (seg104), Gefängnis/Ereignisse
 *            (seg025, seg075, seg081, seg083, seg084, seg120).
 * Daten:     keine eigenen.
 * Besonders: Original-Bug 13/14 (GRP_split). Neu: GRP_swap_heroes und
 *            GRP_move_hero ändern g_wildcamp_guards nur in eine Richtung;
 *            GRP_move_hero liest bei Maus-x < 3 vor g_hero_pic_posx;
 *            GRP_switch_to_next: "state || mode != 2" ist immer wahr.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg049.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg004.h"
#include "seg007.h"
#include "seg008.h"
#include "seg028.h"
#include "seg029.h"
#include "seg047.h"
#include "seg049.h"
#include "seg096.h"
#include "seg097.h"
#include "seg104.h"

/**
 * \brief   qsort() comparator for GRP_sort_heroes()
 *
 *          Order: heroes of the active group (by slot_pos), then empty slots,
 *          then heroes of other groups (by slot_pos).
 */
int GRP_compare_heroes(const void *p1, const void *p2)
{
	struct struct_hero *hero1 = (struct struct_hero*)p1;
	struct struct_hero *hero2 = (struct struct_hero*)p2;

	if ((hero1->typus != HERO_TYPE_NONE) &&
		(hero1->group_id == gs_active_group_id) &&
		(hero2->typus != HERO_TYPE_NONE) &&
		(hero2->group_id == gs_active_group_id))
	{
		if (hero1->slot_pos < hero2->slot_pos)
		{
			return -1;
		} else {
			return 1;
		}
	}

	if ((hero1->typus != HERO_TYPE_NONE) &&
		(hero1->group_id == gs_active_group_id) &&
		(hero2->typus != HERO_TYPE_NONE) &&
		(hero2->group_id != gs_active_group_id))
	{
		return -1;
	}

	if ((hero1->typus != HERO_TYPE_NONE) &&
		(hero1->group_id != gs_active_group_id) &&
		(hero2->typus != HERO_TYPE_NONE) &&
		(hero2->group_id == gs_active_group_id))
	{
		return 1;
	}

	if ((hero1->typus != HERO_TYPE_NONE) &&
		(hero1->group_id != gs_active_group_id) &&
		(hero2->typus != HERO_TYPE_NONE) &&
		(hero2->group_id != gs_active_group_id))
	{
		if (hero1->slot_pos < hero2->slot_pos)
		{
			return -1;
		} else {
			return 1;
		}
	}

	if (!(hero1->typus) &&
		(hero2->typus != HERO_TYPE_NONE) &&
		(hero2->group_id == gs_active_group_id))
	{
		return 1;
	}

	if (!(hero1->typus) &&
		(hero2->typus != HERO_TYPE_NONE) &&
		(hero2->group_id != gs_active_group_id))
	{
		return -1;
	}

	if (!(hero1->typus) && !(hero2->typus))
	{
		return 0;
	}

	if ((hero1->typus != HERO_TYPE_NONE) &&
		(hero1->group_id == gs_active_group_id) &&
		!(hero2->typus))
	{
		return -1;
	}

	if ((hero1->typus != HERO_TYPE_NONE) &&
		(hero1->group_id != gs_active_group_id) &&
		!(hero2->typus))
	{
		return 1;
	}

	return 0;
}

/**
 * \brief   sorts the hero slots 0..5 (the NPC in slot 6 stays) and renumbers slot_pos
 */
void GRP_sort_heroes(void)
{
	signed int hero_pos;

	qsort((void*)get_hero(0), 6, sizeof(struct struct_hero), GRP_compare_heroes);

	for (hero_pos = 0; hero_pos < 6; hero_pos++) {
		get_hero(hero_pos)->slot_pos = hero_pos + 1;
	}
}

/**
 * \brief   stores the current position (and the backup position) for a group
 *
 * \param   group_id    the group; with bit 0x8000 set, the status line is not redrawn
 */
void GRP_save_pos(signed int group_id)
{
	signed int skip_redraw = 0;

	if (group_id & 0x8000) {
		skip_redraw = 1;
	}

	group_id &= 0x7fff;

	GRP_sort_heroes();

	gs_groups_viewdir[group_id] = gs_viewdir;

	gs_groups_x[group_id] = gs_x;
	gs_groups_y[group_id] = gs_y;

	gs_town_groups_loctype[group_id] = gs_town_loc_type;
	gs_groups_town_id[group_id] = gs_town_id;
	gs_groups_dungeon_id[group_id] = gs_dungeon_id;
	gs_groups_dng_level[group_id] = gs_dungeon_level;
	gs_groups_viewdir_bak[group_id] = gs_viewdir_bak;

	gs_groups_x_bak[group_id] = gs_x_bak;
	gs_groups_y_bak[group_id] = gs_y_bak;

	gs_town_groups_loctype_bak[group_id] = gs_town_loc_type_bak;
	gs_groups_town_id_bak[group_id] = gs_town_id_bak;
	gs_groups_dungeon_id_bak[group_id] = gs_dungeon_id_bak;
	gs_groups_dng_level_bak[group_id] = gs_dungeon_level_bak;

	if (!skip_redraw) {
		draw_status_line();
	}
}

#if FIX_0243
/* Fix 243: handlungsfaehige Helden der aktiven Gruppe ohne den NSC (Platz 6) zaehlen
 * (wie count_heroes_available_in_group(), Bright-Eyes: count_heroes_available_in_group_ignore_npc) */
static signed int fix_count_group_no_npc(void)
{
	signed int i;
	signed int cnt = 0;
	struct struct_hero *hero = get_hero(0);

	for (i = 0; i < 6; i++, hero++) {
		if (hero->typus && (hero->group_id == gs_active_group_id) && !hero->flags.dead &&
			!hero->flags.petrified && !hero->flags.unconscious && !hero->flags.renegade)
		{
			cnt++;
		}
	}

	return cnt;
}
#endif

/**
 * \brief   menu "split group": the selected heroes form a new group at the current position
 */
void GRP_split(void)
{
	signed int new_group_id;
	signed int not_empty;
	signed int answer;

#if !FIX_0243
	/* Original-Bug 13:
	 * Split group does not work if the active group consists of 2 available heroes and there is an NPC in another group. */
	if (count_heroes_available_in_group() <= (get_hero(6)->typus != HERO_TYPE_NONE ? 2 : 1))
#else
	/* Fix 243: den NSC nicht mitzaehlen, egal in welcher Gruppe er ist (Bright-Eyes-Fix) */
	if (fix_count_group_no_npc() <= 1)
#endif
	{
		/* don't allow to split a group consisting of a single hero or of a single hero + NPC */

		GUI_output(get_ttx(514)); /* Einer alleine kann sich doch nicht aufteilen... Oder soll er sich zerreissen? */
	} else {

		not_empty = 0;
		new_group_id = 0;

		while (gs_group_member_counts[new_group_id] != 0) {
			new_group_id++;
		}

		do {
			g_hero_sel_exclude = 6;
			answer = select_hero_from_group(get_ttx(515)); /* "WER SOLL IN DIE NEUE GRUPPE AUFGENOMMEN WERDEN?" */

			if (answer == -1) {
				break;
			} else {

				not_empty = 1;
				get_hero(answer)->group_id = new_group_id;
				gs_group_member_counts[new_group_id]++;
				gs_group_member_counts[gs_active_group_id]--;
			}

		}
#if !FIX_0243
	/* Original-Bug 14:
	 * Split group does not allow to select all but one available hero of the active group if there is an NPC in another group. */
		while (count_heroes_available_in_group() > (get_hero(6)->typus != HERO_TYPE_NONE ? 2 : 1));
#else
		while (fix_count_group_no_npc() > 1);
#endif

		if (not_empty) {
			GRP_save_pos(new_group_id);
		}
	}
}

/**
 * \brief   merges all groups at the current position into the active group
 */
void GRP_merge(void)
{
	signed int group_id;
	signed int hero_pos;

	group_id = can_merge_group();

	if (group_id == -1) {

		GUI_output(get_ttx(516)); /* "AUSSER DER AKTUELLEN GRUPPE BEFINDET SICH KEINE AN DIESER STELLE!" */
	} else {

		do {

			gs_groups_viewdir[group_id] = gs_groups_x[group_id]
				= gs_groups_y[group_id] = gs_groups_town_id[group_id]
				= gs_groups_dungeon_id[group_id] = gs_groups_dng_level[group_id]
				= gs_groups_viewdir_bak[group_id] = gs_groups_x_bak[group_id]
				= gs_groups_y_bak[group_id] = gs_town_groups_loctype_bak[group_id]
				= gs_groups_town_id_bak[group_id] = gs_groups_dungeon_id_bak[group_id]
				= gs_groups_dng_level_bak[group_id] = 0;

			gs_group_member_counts[group_id] = 0;

			for (hero_pos = 0; hero_pos <= 6; hero_pos++) {

				if ((get_hero(hero_pos)->typus != HERO_TYPE_NONE) && (get_hero(hero_pos)->group_id == group_id))
				{
					get_hero(hero_pos)->group_id = gs_active_group_id;
					gs_group_member_counts[gs_active_group_id]++;
				}
			}

			GRP_sort_heroes();
			group_id = can_merge_group();

		} while (group_id != -1);

		draw_status_line();
	}
}

/**
 * \brief   switches to the next group which is able to act
 *
 * \param   mode        0 = from the menu (messages, jailed group: offer to wait until 6 o'clock),
 *                      1 = after an event (jail, brewing in the inn, ...),
 *                      2 = the active group cannot act anymore (seg002, seg032)
 *
 *          state: 0 = nobody can act, 1 = free, 2 = in jail (the last checked hero decides).
 *          Remark: if no group can act and the player declines to wait, the loop does not end.
 */
void GRP_switch_to_next(const signed int mode)
{
	signed int hero_pos;
	signed int state;

	signed int group_id;
	signed int done = 0;
	signed int dng_level;
#if FIX_0244
	signed int tries = 0;
	signed int jailed = 0;	/* 1 = eine Gruppe sitzt im Kerker, 2 = schon bis 6 Uhr gewartet */
#endif

	group_id = gs_active_group_id;

	do {
		/* find next group */
		group_id++;

		if (group_id == 6) {
			group_id = 0;
		}

#if FIX_0244
		/* Fix 244: nach einer vollen Runde ohne handlungsfaehige Gruppe nicht endlos weitersuchen:
		 * im Menue bei der aktiven Gruppe bleiben; nach einem Ereignis bis 6 Uhr warten, wenn eine
		 * Gruppe im Kerker sitzt (dann kommt sie frei), sonst ist das Spiel verloren */
		if (++tries > 6) {

			tries = 0;

			if ((mode != 0) && (jailed == 1)) {

				timewarp_until_time_of_day(HOURS(6));
				jailed = 2;

			} else {
				if ((mode != 0) && (jailed == 0)) {
					g_game_state = GAME_STATE_DEAD;
				}

				group_id = gs_active_group_id;
				break;
			}
		}
#endif

		if (gs_group_member_counts[group_id] != 0) {

			state = 0;

			for (hero_pos = 0; hero_pos < 6; hero_pos++) {

				if ((get_hero(hero_pos)->typus != HERO_TYPE_NONE) && (get_hero(hero_pos)->group_id == group_id) && check_hero(get_hero(hero_pos)))
				{
					if (get_hero(hero_pos)->jail) {
						/* hero is in prison */
						state = 2;
#if FIX_0244
						if (!jailed) {
							jailed = 1;
						}
#endif
					} else {
						/* hero is free */
						state = 1;
					}
				}
			}

			if (!state || state == 2) {

				if (mode == 0) {

					if (!state) {
						GUI_output(get_ttx(36)); /* "IN DIESER GRUPPE IST MOMENTAN NIEMAND IN DER LAGE ETWAS ZU TUN!" */
					} else {
						if ((state == 2) && GUI_bool(get_ttx(773))) { /* "IN EINE ZUGIGE ZELLE IM KNAST KANN NICHT UMGESCHALTEN WERDEN.@WOLLT..." */
							timewarp_until_time_of_day(HOURS(6));
							done = 1;
						}
					}
				}

			} else {

				if (state || mode != 2) { /* always true here, since state == 1 */
					done = 1;
				}
			}
		}

	} while (done == 0);

	if (gs_active_group_id != group_id) {

		if ( ((gs_town_id != TOWN_ID_NONE) && !gs_groups_town_id[group_id]) ||
			(!gs_town_id && (gs_groups_town_id[group_id] != TOWN_ID_NONE)))
		{
			set_palette(g_palette_allblack2, 0x00, 0x20);
			set_palette(g_palette_allblack2, 0x80, 0x20);
			set_palette(g_palette_allblack2, 0xa0, 0x20);
		}

		if (gs_groups_dungeon_id[group_id] && (gs_groups_dungeon_id[group_id] != gs_dungeon_id))
		{
			g_dng_loaded_dungeon_id = -1;
			g_area_prepared = AREA_TYPE_NONE;
		}

		if (gs_groups_town_id[group_id] && (gs_groups_town_id[group_id] != gs_town_id))
		{
			g_town_loaded_town_id = -1;
			g_area_prepared = AREA_TYPE_NONE;
		}

		/* save positions from the old group */
		gs_groups_viewdir[gs_active_group_id] = gs_viewdir;
		gs_groups_x[gs_active_group_id] = gs_x;
		gs_groups_y[gs_active_group_id] = gs_y;
		gs_town_groups_loctype[gs_active_group_id] = gs_town_loc_type;
		gs_groups_town_id[gs_active_group_id] = gs_town_id;
		gs_groups_dungeon_id[gs_active_group_id] = gs_dungeon_id;
		gs_groups_dng_level[gs_active_group_id] = gs_dungeon_level;
		gs_groups_viewdir_bak[gs_active_group_id] = gs_viewdir_bak;
		gs_groups_x_bak[gs_active_group_id] = gs_x_bak;
		gs_groups_y_bak[gs_active_group_id] = gs_y_bak;
		gs_town_groups_loctype_bak[gs_active_group_id] = gs_town_loc_type_bak;
		gs_groups_town_id_bak[gs_active_group_id] = gs_town_id_bak;
		gs_groups_dungeon_id_bak[gs_active_group_id] = gs_dungeon_id_bak;
		gs_groups_dng_level_bak[gs_active_group_id] = gs_dungeon_level_bak;

		/* set positions for the new group */
		gs_active_group_id = group_id;
		gs_viewdir = gs_groups_viewdir[group_id];
		gs_x = gs_groups_x[group_id];
		gs_y = gs_groups_y[group_id];
		gs_town_loc_type = gs_town_groups_loctype[group_id];
		gs_town_id = gs_groups_town_id[group_id];
		gs_dungeon_id = gs_groups_dungeon_id[group_id];
		dng_level = gs_dungeon_level;
		gs_dungeon_level = gs_groups_dng_level[group_id];

		if (dng_level != gs_dungeon_level) {
			load_area_description(1);
		}

		gs_viewdir_bak = gs_groups_viewdir_bak[group_id];
		gs_x_bak = gs_groups_x_bak[group_id];
		gs_y_bak = gs_groups_y_bak[group_id];
		gs_town_loc_type_bak = gs_town_groups_loctype_bak[group_id];
		gs_town_id_bak = gs_groups_town_id_bak[group_id];
		gs_dungeon_id_bak = gs_groups_dungeon_id_bak[group_id];
		gs_dungeon_level_bak = gs_groups_dng_level_bak[group_id];

		GRP_sort_heroes();

		for (group_id = 0; group_id <= 6; group_id++) {
			gs_food_message[group_id] = gs_unconscious_message[group_id] = 0; /* group_id is used as hero index here */
		}

		g_request_refresh = 1;
	}
}

/**
 * \brief   menu "swap heroes": exchanges the slots of two heroes of the group
 */
void GRP_swap_heroes(void)
{
	signed int hero_pos_1;
	signed int hero_pos_2;
	signed char guardstatus_bak;
	signed char magicstatus_bak;
	signed char replstatus_bak;
	signed char herbstatus_bak;
	signed char i;
	struct struct_hero tmp;

	if (!g_heroswap_allowed || !gs_group_member_counts[gs_active_group_id]) {
		return;
	}

	hero_pos_1 = select_hero_from_group(get_ttx(302)); /* "WELCHEN HELDEN UMSETZEN?" */

	if ((hero_pos_1 != -1) && (hero_pos_1 < 6)) {

		hero_pos_2 = select_hero_from_group(get_ttx(303)); /* "MIT WELCHEM HELDEN VERTAUSCHEN?" */

		if ((hero_pos_2 != -1) && (hero_pos_1 != hero_pos_2) && (hero_pos_2 < 6)) {

			for (i = 0; i < 3; i++) {

				if (g_wildcamp_guards[i] == hero_pos_1) { /* Original-Bug: a guard on hero_pos_2 is not changed to hero_pos_1 */
					g_wildcamp_guards[i] = hero_pos_2;
				}
#if FIX_0245
				/* Fix 245: auch die Wache des zweiten Helden mitnehmen */
				else if (g_wildcamp_guards[i] == hero_pos_2) {
					g_wildcamp_guards[i] = hero_pos_1;
				}
#endif
			}

			/* save hero1 in tmp */
			tmp = *get_hero(hero_pos_1);	/* struct_copy */

			guardstatus_bak = g_wildcamp_guardstatus[hero_pos_1];
			magicstatus_bak = g_wildcamp_magicstatus[hero_pos_1];
			replstatus_bak = g_wildcamp_replstatus[hero_pos_1];
			herbstatus_bak = g_wildcamp_herbstatus[hero_pos_1];

			*(get_hero(hero_pos_1)) = *(get_hero(hero_pos_2)); /* struct_copy */

			g_wildcamp_guardstatus[hero_pos_1] = g_wildcamp_guardstatus[hero_pos_2];
			g_wildcamp_magicstatus[hero_pos_1] = g_wildcamp_magicstatus[hero_pos_2];
			g_wildcamp_replstatus[hero_pos_1] = g_wildcamp_replstatus[hero_pos_2];
			g_wildcamp_herbstatus[hero_pos_1] = g_wildcamp_herbstatus[hero_pos_2];

			*(get_hero(hero_pos_2)) = tmp; /* struct_copy */

			g_wildcamp_guardstatus[hero_pos_2] = guardstatus_bak;
			g_wildcamp_magicstatus[hero_pos_2] = magicstatus_bak;
			g_wildcamp_replstatus[hero_pos_2] = replstatus_bak;
			g_wildcamp_herbstatus[hero_pos_2] = herbstatus_bak;

			if (get_hero(hero_pos_1)->typus) {
				get_hero(hero_pos_1)->action_id = FIG_ACTION_PARRY; /* FIG_ACTION_PARRY == 100 */
			}

			if (get_hero(hero_pos_2)->typus) {
				get_hero(hero_pos_2)->action_id = FIG_ACTION_PARRY;
			}

			get_hero(hero_pos_1)->slot_pos = hero_pos_1 + 1;
			get_hero(hero_pos_2)->slot_pos = hero_pos_2 + 1;
		}
	}

	draw_status_line();
}

/**
 * \brief   drags the portrait of a hero with the mouse to another slot
 *
 * \param   src_pos     slot of the dragged hero (the NPC in slot 6 cannot be moved)
 */
void GRP_move_hero(const signed int src_pos)
{
	signed int dst_pos;
	struct struct_hero *src;
	struct struct_hero *dst;
	signed char src_guardstatus;
	signed char src_magicstatus;
	signed char src_replstatus;
	signed char src_herbstatus;
	signed char i;

	/* don't move NPC */
	if (src_pos != 6) {

		g_mouse_posy_min = 157;
		g_mouse_posy_max = 157;
		g_mouse_posx_min = 1;
		g_mouse_posx_max = 260;

		call_mouse_bg();

		g_mouse_locked = 1;
		g_mouse_posy = 157;

		g_mouse_posx = g_hero_pic_posx[src_pos];
		g_pic_copy.x1 = g_mouse_posx;
		g_pic_copy.y1 = g_mouse_posy;
		g_pic_copy.x2 = g_mouse_posx + 31;
		g_pic_copy.y2 = g_mouse_posy + 31;
		g_pic_copy.src = g_icon;

		do_save_rect();

		g_mouse_locked = 0;

		call_mouse();

		g_mouse_locked = 1;

		g_pic_copy.x1 = g_hero_pic_posx[src_pos];
		g_pic_copy.y1 = 157;
		g_pic_copy.x2 = g_hero_pic_posx[src_pos] + 31;
		g_pic_copy.y2 = 188;
		g_pic_copy.src = g_renderbuf_ptr;

		do_save_rect();

		g_mouse_locked = 0;

		call_mouse_bg();

		g_mouse_posx_bak = g_mouse_posx;
		g_mouse_posy_bak = g_mouse_posy;

		while (!g_mouse1_event2) {
#if !defined(__BORLANDC__)
			/* call DOSBOX to handle mouse ISR */
			wait_for_vsync();
#endif
			if (g_mouse_moved) {

				g_mouse_locked = 1;

				wait_for_vsync();

				g_pic_copy.x1 = g_mouse_posx_bak;
				g_pic_copy.y1 = g_mouse_posy_bak;
				g_pic_copy.x2 = g_mouse_posx_bak + 31;
				g_pic_copy.y2 = g_mouse_posy_bak + 31;
				g_pic_copy.src = g_icon;

				do_pic_copy(0);

				g_pic_copy.x1 = g_mouse_posx;
				g_pic_copy.y1 = g_mouse_posy;
				g_pic_copy.x2 = g_mouse_posx + 31;
				g_pic_copy.y2 = g_mouse_posy + 31;
				g_pic_copy.src = g_icon;

				do_save_rect();

				g_pic_copy.src = g_renderbuf_ptr;

				do_pic_copy(0);

				g_mouse_posx_bak = g_mouse_posx;
				g_mouse_posy_bak = g_mouse_posy;
				g_mouse_moved = 0;
				g_mouse_locked = 0;
			}
		}

		g_pic_copy.x1 = g_mouse_posx_bak;
		g_pic_copy.y1 = g_mouse_posy_bak;
		g_pic_copy.x2 = g_mouse_posx_bak + 31;
		g_pic_copy.y2 = g_mouse_posy_bak + 31;
		g_pic_copy.src = g_icon;

		do_pic_copy(0);

		dst_pos = 6;
#if FIX_0298
		/* Fix 298: nicht vor die Tabelle lesen (Ergebnis wie bisher, hoechstens 0) */
		while ((--dst_pos > 0) && (g_hero_pic_posx[dst_pos] > g_mouse_posx))
#else
		while (g_hero_pic_posx[--dst_pos] > g_mouse_posx) /* for mouse x < 3 this reads before the table; dst_pos is clamped below */
#endif
		{
			;
		}

		if (dst_pos < 0) {
			dst_pos = 0;
		}

		if ((src_pos != dst_pos) && (dst_pos != 6)) { /* dst_pos != 6 is always true here */

			dst = get_hero(dst_pos);
			src = get_hero(src_pos);

			if (!dst->typus || (dst->group_id == gs_active_group_id)) {

				for (i = 0; i < 3; i++) {
					if (g_wildcamp_guards[i] == src_pos) { /* Original-Bug: a guard on dst_pos is not changed to src_pos */
						g_wildcamp_guards[i] = dst_pos;
					}
#if FIX_0245
					/* Fix 245: auch die Wache auf dem Zielplatz mitnehmen */
					else if (g_wildcamp_guards[i] == dst_pos) {
						g_wildcamp_guards[i] = src_pos;
					}
#endif
				}

				memcpy(g_renderbuf_ptr, src, sizeof(struct struct_hero));

				src_guardstatus = g_wildcamp_guardstatus[src_pos];
				src_magicstatus = g_wildcamp_magicstatus[src_pos];
				src_replstatus = g_wildcamp_replstatus[src_pos];
				src_herbstatus = g_wildcamp_herbstatus[src_pos];

				*src = *dst; /* struct_copy */

				g_wildcamp_guardstatus[src_pos] = g_wildcamp_guardstatus[dst_pos];
				g_wildcamp_magicstatus[src_pos] = g_wildcamp_magicstatus[dst_pos];
				g_wildcamp_replstatus[src_pos] = g_wildcamp_replstatus[dst_pos];
				g_wildcamp_herbstatus[src_pos] = g_wildcamp_herbstatus[dst_pos];

				memcpy(dst, g_renderbuf_ptr, sizeof(struct struct_hero));

				g_wildcamp_guardstatus[dst_pos] = src_guardstatus;
				g_wildcamp_magicstatus[dst_pos] = src_magicstatus;
				g_wildcamp_replstatus[dst_pos] = src_replstatus;
				g_wildcamp_herbstatus[dst_pos] = src_herbstatus;

				src->action_id = 100; /* FIG_ACTION_PARRY */
				dst->action_id = 100; /* FIG_ACTION_PARRY */
				dst->slot_pos = src_pos + 1;
				src->slot_pos = dst_pos + 1;
			}
		}

		g_mouse1_event2 = 0;
		g_mouse_posy_min = 0;
		g_mouse_posy_max = 200 - 1;
		g_mouse_posx_min = 0;
		g_mouse_posx_max = 320 - 1;

		call_mouse();
		draw_status_line();
	}
}

/**
 * \brief   regeneration of a hero during one night
 *
 *          Nothing happens if the hero is dead or hero->staffspell_timer or
 *          hero->recipe_timer is running. On a ship: 9 % sea sickness.
 *          Diseased or poisoned heroes do not regenerate. A brewing hero
 *          finishes his recipe instead.
 *
 * \param   hero        the hero
 * \param   quality     quality of the sleep, added to each 1W6 roll for LE and AE
 */
void GRP_hero_sleep(struct struct_hero *hero, const signed int quality)
{
	signed int le_regen;
	signed int ae_regen;
	signed int diff;
	signed int tmp;

	if (!hero->flags.dead && (hero->staffspell_timer == 0) && (hero->recipe_timer == 0))
	{

		if (g_travel_by_ship && (random_schick(100) < 10)) {

			/* 9% chance of becoming sea-sick */

			sprintf(g_dtp2,	get_ttx(796), hero->alias); /* "%s IST SEEKRANK UND VERBRINGT MEHR ZEIT AN DER RELING, ALS IM BETT!" */
			GUI_output(g_dtp2);

		} else {

			if (!hero_is_diseased(hero) && !hero_is_poisoned(hero)) {

				if (hero->ruhe_koerper > 0) {
					hero->ruhe_koerper--;
				}

				if (!hero->flags.brewing) {

					le_regen = random_schick(6) + quality;
					ae_regen = random_schick(6) + quality;

					/* Original-Bug: next operator should be <= */
#if FIX_0246
					/* Fix 246: auch Ergebnis 0 auf 1 setzen */
					if (le_regen <= 0) {
#else
					if (le_regen < 0) {
#endif
						le_regen = 1;
					}

					/* Original-Bug: next operator should be <= */
#if FIX_0246
					if (ae_regen <= 0) {
#else
					if (ae_regen < 0) {
#endif
						ae_regen = 1;
					}

					/* magic users: the higher roll goes to LE */
					if ((hero->typus >= HERO_TYPE_HEXE) && (le_regen < ae_regen)) {
						tmp = ae_regen;
						ae_regen = le_regen;
						le_regen = tmp;
					}

					if (hero->ruhe_koerper > 0) {
						le_regen += le_regen;
						ae_regen += ae_regen;
					}

					/* Do LE */

					diff = hero->le_max - hero->le;

					if ((diff != 0) && le_regen) {

						if (diff < le_regen) {
							le_regen = diff;
						}

						add_hero_le(hero, le_regen);

						strcpy(g_text_output_buf, get_ttx(392)); /* "PUNKT" */

						if (le_regen > 1) {
							strcat(g_text_output_buf, get_ttx(393)); /* "E" */
						}

						sprintf(g_dtp2, get_ttx(319), hero->alias, le_regen, g_text_output_buf); /* "%s REGENERIERT %d LE-%s." */

						if (g_pp20_index == ARCHIVE_FILE_PLAYM_UK) {

							GUI_print_loc_line(g_dtp2);
							vsync_or_key(200);

						} else {

							if (g_travel_by_ship) {
								GUI_output(g_dtp2);
							}
						}

					}

					if (hero->typus >= HERO_TYPE_HEXE) {

						diff = hero->ae_max - hero->ae;

						if ((diff != 0) && ae_regen) {

							if (diff < ae_regen) {
								ae_regen = diff;
							}

							add_hero_ae(hero, ae_regen);

							strcpy(g_text_output_buf, get_ttx(392)); /* "PUNKT" */

							if (ae_regen > 1) {
								strcat(g_text_output_buf, get_ttx(393)); /* "E" */
							}

							sprintf(g_dtp2, get_ttx(320), hero->alias, ae_regen, g_text_output_buf); /* "%sS AE STEIGT UM %d %s." */

							if (g_pp20_index == ARCHIVE_FILE_PLAYM_UK) {

								GUI_print_loc_line(g_dtp2);
								vsync_or_key(200);

							} else {
								if (g_travel_by_ship) {
									GUI_output(g_dtp2);
								}
							}
						}
					}
				} else {
					/* hero brewing, HERO_STAFFSPELL_TIMER == 0, HERO_RECIPE_TIMER == 0, not dead, not diseased, not poisoned */
					/* TODO: potential Original-Bug: why is 'not poisoned', 'not diseased' and 'HERO_STAFFSPELL_TIMER == 0' needed to complete the brewing process? */
					/* Remark: hero->staffspell_timer is only read here, so it works as a 'no regeneration' timer
					 * (set by the staff spell, failed wound healing, brewing in the wild camp). */
					do_alchemy(hero, hero->recipe_id, 0);
				}
			} else {
				sprintf(g_dtp2,	get_ttx(558), hero->alias); /* "%s FÜHLT SICH NACH EINER BEINAHE SCHLAFLOSEN NACHT EHER SCHLECHTER..." */
				GUI_output(g_dtp2);
			}
		}
	}
}
