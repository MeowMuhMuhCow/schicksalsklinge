/*
 * seg036 - Kampf: Computerkampf der Helden (AFIG)
 *
 * Inhalt:    Steuert Helden automatisch: im Computerkampf, als NSC (eigene Fluchtregeln
 *            fuer Nariell, Harika, Curian, Ardora, Garsvik und Erwo) und wenn sie
 *            abtruennig sind. AFIG_hero_turn() waehlt Flucht, Zauber
 *            (AFIG_select_autospell() mit der Liste g_af_spell_list), Fernkampf
 *            (AFIG_select_range_target()) oder Nahkampf und bewegt den Helden sonst auf ein
 *            Ziel zu. FIG_prepare_hero_ani() baut die Bewegungsanimation eines Helden aus
 *            ANI.DAT.
 * Aufrufer:  FIG_menu() (seg033); FIG_prepare_hero_ani() auch aus seg033 und seg034.
 * Daten:     g_af_spell_list[11] (Zauber fuer den Computerkampf).
 * Besonders: Im Modus 0 (Ziel: Gegner) werden auch abtruennige, also verbuendete Gegner
 *            (Boeser Blick) angegriffen. AFIG_switch_to_melee_weapon() nimmt keine Speere
 *            (Bright-Eyes: Original-Bug). Mehrere Funktionsnamen und
 *            Parameterbeschreibungen passten nicht (Fernkampf statt Zauber, Zielmodus statt
 *            "renegade").
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg036.c
 */
#include <string.h>

#include "v302de.h"
#include "common.h"

#include "seg001.h"
#include "seg002.h"
#include "seg005.h"
#include "seg006.h"
#include "seg007.h"
#include "seg032.h"
#include "seg036.h"
#include "seg038.h"
#include "seg039.h"
#include "seg041.h"
#include "seg106.h"

static const signed char g_af_spell_list[11] = {
	SP_FULMINICTUS_DONNERKEIL,
	SP_HORRIPHOBUS,
	SP_IGNIFAXIUS_FLAMMENSTRAHL,
	SP_BALSAM_SALABUNDE,
	SP_BOESER_BLICK,
	SP_AXXELERATUS_BLITZGESCHWIND,
	SP_DUPLICATUS_DOPPELPEIN,
	SP_BLITZ_DICH_FIND,
	SP_PLUMBUMBARUM_UND_NARRETEI,
	SP_SAFT_KRAFT_MONSTERMACHT,
	SP_PARALUE_PARALEIN
}; // ds:0x5fac

static struct viewdir_offsets g_fig_viewdir_offsets3 = { { { 1, 0 }, { 0, -1 }, { -1, 0 }, { 0, 1 } } }; // ds:0x5fb7
static struct viewdir_offsets g_fig_viewdir_offsets4 = { { { 1, 0 }, { 0, -1 }, { -1, 0 }, { 0, 1 } } }; // ds:0x5fc7

/**
 * \brief   loads an ani clip from SCHICK.DAT
 *
 * \param dst           destination pointer for data
 * \param clip_num      number of the track within the source file.
 * \param src_file_id	id for the source file within SCHICK.DAT to use.
 *                      ANI.DAT     --> ANI_SRC_FILE_ID_ANI_DAT     == 2  (or in fact, all values != 3)
 *                      WEAPANI.DAT --> ANI_SRC_FILE_ID_WEAPANI_DAT == 3  (in fact, this never happens)
 * \return              the length of the track in bytes.
 */
static signed int load_ani_clip_from_file(int8_t *dst, const signed int clip_num, const signed int src_file_id)
{
	uint8_t *p_datbuffer;
	uint8_t *p_datitem;
	signed char len;
	signed int i;
	signed int clip_num_max;

	/* set the right buffer */
	p_datbuffer = g_buffer_anidat;

	if (src_file_id == ANI_SRC_FILE_ID_WEAPANI_DAT)
		// this never happens
		p_datbuffer = g_buffer_weapanidat;

	/* read how many ani sequences are in the file */
	clip_num_max = *(int16_t*)p_datbuffer;

	/* check if the desired ani_no is in the range.
	 * REMARK: valid numbers are 0..clip_num_max-1, clip_num == clip_num_max is not rejected */
	if (clip_num < 0)
		return 0;

#if FIX_0294
	/* Fix 294: gueltig sind 0..clip_num_max-1 */
	if (clip_num >= clip_num_max)
#else
	if (clip_num > clip_num_max)
#endif
		return 0;

	/* set p_datitem to the first (0) ani sequence */
	p_datitem = p_datbuffer;
	p_datitem += clip_num_max + 2;
	/* set len to the lenght first (0) ani sequence */
	len = p_datbuffer[2];

	/* forward to the desired ani sequence */
	for (i = 1; i <= clip_num; i++) {
		p_datitem += len;
		len = *(p_datbuffer + i + 2);
	}

	p_datitem++;

	len = len - 2;

	/* copy the ani sequence to dst */
	for (i = 0; len > i; i++) {
		*dst = *p_datitem;
		p_datitem++;
		dst++;
	}

	return len;
}

/**
 * \brief   builds the movement animation of a hero along the path in g_fig_move_pathdir,
 *          plays it and subtracts the BP (1 per single step, 2 per double step)
 *
 * \param   hero        pointer to the hero
 * \param   hero_pos    position of the hero in the group (object id = hero_pos + 1)
 *
 * ani_index_ptr_index_2/_1: first and (for a turn by 180 degrees) second turning clip,
 * see the table in FANI_prepare_fight_enemy_ani() (seg044.c).
 */
void FIG_prepare_hero_ani(struct struct_hero *hero, const signed int hero_pos)
{
	signed int i;
	signed char ani_index_ptr_index_1;
	signed char ani_index_ptr_index_2;
	int8_t *p_ani_clip_base;
	signed char viewdir_iter;
	int16_t *ani_index_ptr;

	g_fig_ani_tracks[FANI_TRACK_ID_ACTOR_0_BASE][0] = 0;
	g_fig_ani_tracks[FANI_TRACK_ID_ACTOR_0_BASE][242] = hero->actor_sprite_id;

	p_ani_clip_base = &g_fig_ani_tracks[FANI_TRACK_ID_ACTOR_0_BASE][1];
	ani_index_ptr = g_gfx_ani_index[hero->actor_sprite_id];

	i = 0;

	while (g_fig_move_pathdir[i] != -1) {

		/* Concerning the following code block: There are parallel code blocks in
		 * prepare_enemy_ani(...) in seg037.c
		 * FIG_prepare_hero_ani(...) in seg036.c
		 * FANI_prepare_fight_hero_ani(...) in seg044.c
		 * FANI_prepare_fight_enemy_ani(...) in seg044.c
		 * FANI_prepare_spell_hero(...) in seg044.c
		 * FANI_prepare_spell_enemy(...) in seg044.c
		 */
		if (hero->viewdir != g_fig_move_pathdir[i]) {

			ani_index_ptr_index_2 = ani_index_ptr_index_1 = FIG_VIEWDIR__NONE;
			viewdir_iter = hero->viewdir;
			ani_index_ptr_index_2 = viewdir_iter;

			viewdir_iter++;
			if (viewdir_iter == FIG_VIEWDIR__END) {
				viewdir_iter = FIG_VIEWDIR__BEGIN;
			}

			if (g_fig_move_pathdir[i] != viewdir_iter) {

				ani_index_ptr_index_1 = viewdir_iter;

				viewdir_iter++;
				if (viewdir_iter == FIG_VIEWDIR__END) {
					viewdir_iter = FIG_VIEWDIR__BEGIN;
				}

				if (g_fig_move_pathdir[i] != viewdir_iter) {

					ani_index_ptr_index_2 = hero->viewdir + 4;
					ani_index_ptr_index_1 = FIG_VIEWDIR__NONE;
				}
			}

			hero->viewdir = g_fig_move_pathdir[i];

			p_ani_clip_base += load_ani_clip_from_file(p_ani_clip_base, ani_index_ptr[ani_index_ptr_index_2], ANI_SRC_FILE_ID_ANI_DAT);

			if (ani_index_ptr_index_1 != FIG_VIEWDIR__NONE) {
				p_ani_clip_base += load_ani_clip_from_file(p_ani_clip_base, ani_index_ptr[ani_index_ptr_index_1], ANI_SRC_FILE_ID_ANI_DAT);
			}
		}

		if (g_fig_move_pathdir[i] == g_fig_move_pathdir[i + 1]) {

			p_ani_clip_base += load_ani_clip_from_file(p_ani_clip_base, ani_index_ptr[g_fig_move_pathdir[i] + 12], ANI_SRC_FILE_ID_ANI_DAT);
			i += 2;
			/* BP - 2 */
			hero->fight_bp_left = hero->fight_bp_left - 2;
		} else {
			p_ani_clip_base += load_ani_clip_from_file(p_ani_clip_base, ani_index_ptr[g_fig_move_pathdir[i] + 8], ANI_SRC_FILE_ID_ANI_DAT);
			i++;
			/* BP - 1 */
			hero->fight_bp_left--;
		}
	}

	*p_ani_clip_base = -1;
	FIG_call_draw_pic();
	FIG_remove_from_list(g_fig_cb_marker_id, 0);
	g_fig_cb_marker_id = -1;
	FIG_set_ani_track_id_base(hero->fighter_id, FANI_TRACK_ID_ACTOR_0_BASE);

	draw_fight_screen(0);
	memset(&g_fig_ani_tracks[FANI_TRACK_ID_ACTOR_0_BASE], -1, 243);
	FIG_init_list_elem(hero_pos + 1);
}

/**
 * \brief   changes the weapon of a hero
 *
 *          takes the first melee weapon from the knapsack into the right hand; called by
 *          AFIG_hero_turn() when the ranged weapon cannot be used (no ammunition or target
 *          adjacent). Without a melee weapon the old weapon is put away (bare hands). Costs 2 BP.
 *
 * \param   hero        pointer to the hero (former text "with a broken weapon" did not fit the callers)
 * \return              1 if a weapon was found, 0 if the hero fights now with bare hands
 */
signed int AFIG_switch_to_melee_weapon(struct struct_hero *hero)
{
	signed int pos;
	signed int has_new_weapon = 0;
	struct item_stats *item_p;
	signed int item_id;
	struct struct_fighter *p_fighter;

	for (pos = HERO_INVENTORY_SLOT_KNAPSACK_1; pos < NR_HERO_INVENTORY_SLOTS; pos++) {

		item_id = hero->inventory[pos].item_id;
		item_p = &g_itemsdat[item_id];

		/* grab the first melee weapon in the knapsack,
		 * and exchange it with the broken weapon. */
		if (item_p->flags.weapon &&
#if FIX_0316
			/* Fix 316: keine zerbrochene Waffe nehmen */
			!hero->inventory[pos].flags.broken &&
#endif
			(item_p->item_subtype_id != WEAPON_TYPE_SCHUSSWAFFE) &&
			(item_p->item_subtype_id != WEAPON_TYPE_WURFWAFFE) &&
			(item_p->item_subtype_id != WEAPON_TYPE_SPEER)) // Original-Bug: WEAPON_TYPE_SPEER is a melee weapon category.
		{
			move_item(HERO_INVENTORY_SLOT_RIGHT_HAND, pos, hero);
			has_new_weapon = 1;
			break;
		}
	}

	if (!has_new_weapon) {

		/* find a free slot, to get rid of the broken weapon */
		for (pos = HERO_INVENTORY_SLOT_KNAPSACK_1; pos < NR_HERO_INVENTORY_SLOTS; pos++) {

			if (hero->inventory[pos].item_id == ITEM_ID_NONE) {

				move_item(HERO_INVENTORY_SLOT_RIGHT_HAND, pos, hero);
				has_new_weapon = 2;
				break;
			}
		}

		/* if nothing helps, put it in the left hand */
		if (!has_new_weapon) {
			move_item(HERO_INVENTORY_SLOT_RIGHT_HAND, HERO_INVENTORY_SLOT_LEFT_HAND, hero);
		}

		has_new_weapon = 0;
	}

	p_fighter = FIG_get_fighter(hero->fighter_id);
	p_fighter->nvf_no = hero->viewdir;
	p_fighter->reload = -1;

	draw_fight_screen_pal(0);

	hero->fight_bp_left = hero->fight_bp_left - 2;

	return has_new_weapon;
}

/**
 * \brief   check if a neighbour can be attacked
 *
 * \param   start_x     X-Coordinate of the hero
 * \param   start_y     Y-Coordinate of the hero
 * \param   offset_x    X-Direction -1/0/+1
 * \param   offset_y    Y-Direction -1/0/+1
 * \param   mode        0 = living enemies (renegade ones, too) /
 *                      1 = living, conscious heroes and living renegade enemies (the heroes' side) /
 *                      2 = only living, conscious heroes (not passed by the callers in this file)
 * \return              1 if an attack is possible in that mode, else 0.
 */
signed int AFIG_can_attack_neighbour(const signed int start_x, const signed int start_y,
			const signed int offset_x, const signed int offset_y, const signed int mode)
{
	const signed char target_object_id = get_cb_val(start_x + offset_x, start_y + offset_y);

	/* REMARK: in this file the 'dead' flag of an enemy is read via the address of the flags
	 * (mov dx,offset; add dx,0x31; add ax,dx; mov bx,ax; mov al,[bx]), unlike in seg032-seg034,
	 * where the plain C form g_enemy_sheets[i].flags.dead gives mov al,[bx+offset+0x31].
	 * The cast to struct enemy_flags reproduces this in C; Attic probably used some other
	 * declaration or macro here. The 'renegade' flag (byte 0x32) needs no cast. */
	if (mode == 1) {

		/* target is hero or enemy */
		if (
			(
				 (target_object_id > 0) && (target_object_id < 10)
				 && !get_hero(target_object_id - 1)->flags.dead && !get_hero(target_object_id - 1)->flags.unconscious
			) || (
				(target_object_id >= 10) && (target_object_id < 30)
				&& !((struct enemy_flags)g_enemy_sheets[target_object_id - 10].flags).dead
				&& g_enemy_sheets[target_object_id - 10].flags.renegade
			)
		) {
			return 1;
		} else {
			return 0;
		}

	} else if (!mode) {

		/* target is an enemy.
		 * REMARK: renegade enemies (fighting for the heroes, e.g. Boeser Blick) are attacked, too.
		 * Original-Bug? */
		if (
			(target_object_id >= 10) && (target_object_id < 30)
			&& !((struct enemy_flags)g_enemy_sheets[target_object_id - 10].flags).dead
#if FIX_0018
			/* Fix 18: uebergelaufene Gegner (Boeser Blick) kaempfen auf der Seite der Helden */
			&& !g_enemy_sheets[target_object_id - 10].flags.renegade
#endif
		) {
			return 1;
		} else {
			return 0;
		}

	} else if (mode == 2) {

		/* target is a hero */
		if (
			(target_object_id > 0) && (target_object_id < 10)
			&& !get_hero(target_object_id - 1)->flags.dead
			&& !get_hero(target_object_id - 1)->flags.unconscious
		) {
			return 1;
		} else {
			return 0;
		}
	}

	return 0;
}

/**
 * \brief   searches the first target in a straight line from (x, y) in direction viewdir
 *
 *          Used for ranged attacks (AFIG_select_range_target) and ranged spells
 *          (AFIG_select_autospell). The search stops at the border, at obstacles and at
 *          living actors of the other kind.
 *
 * \param   x           x-coordinate of the hero
 * \param   y           y-coordinate of the hero
 * \param   viewdir     direction of the search
 * \param   target_mode 0 = search enemies, 1 = search heroes or renegade enemies
 *                      (hero->flags.renegade, or the mode from AFIG_get_spell_target_mode())
 * \return              0 = no target found, else the object id of the target
 */
signed int AFIG_search_line_target(const signed int x, const signed int y, const signed int viewdir, const signed int target_mode)
{
	signed int x_diff;
	signed int y_diff;
	signed char target_object_id;
	signed int done;
	signed int will_attack;

	done = 0;
	x_diff = 0;
	y_diff = 0;
	will_attack = 0;

	while (done == 0) {

		/* calculate the offset from direction */
		if (viewdir == FIG_VIEWDIR_RIGHT) {
			x_diff++;
		} else if (viewdir == FIG_VIEWDIR_DOWN) {
			y_diff--;
		} else if (viewdir == FIG_VIEWDIR_LEFT) {
			x_diff--;
		} else {
			y_diff++;
		}

		/* check whether the square is on the chessboard */
		if ((y + y_diff < 0) || (y + y_diff > 23) ||
			(x + x_diff < 0) || (x + x_diff > 23))
		{
			done = 1;
			continue;
		}

		/* get the fight object ID of the object on that square */
		target_object_id = get_cb_val(x + x_diff, y + y_diff);

		if (target_mode == 1) {

			/* attack only heroes and renegade enemies */
			if (
				(
					(target_object_id > 0) && (target_object_id < 10)
					 && !get_hero(target_object_id - 1)->flags.dead && !get_hero(target_object_id - 1)->flags.unconscious
				) || (
					(target_object_id >= 10) && (target_object_id < 30)
					&& !((struct enemy_flags)g_enemy_sheets[target_object_id - 10].flags).dead
					&& g_enemy_sheets[target_object_id - 10].flags.renegade
				)
			) {

				will_attack = 1;
				done = 1;

			} else if (
				(target_object_id != 0)
				&& (
					(
						(target_object_id >= 10) && (target_object_id < 30)
						&& !((struct enemy_flags)g_enemy_sheets[target_object_id - 10].flags).dead
					) || (
						(target_object_id >= 50)
						&& !is_in_int_array(target_object_id - 50, g_cb_obj_nonobstacle))
				)
			) {
				done = 1;
			}

		} else if (target_mode == 0) {

			/* attack only enemies (renegade ones, too) */
			if (
				(target_object_id >= 10) && (target_object_id < 30)
				&& !((struct enemy_flags)g_enemy_sheets[target_object_id - 10].flags).dead
#if FIX_0018
				/* Fix 18: uebergelaufene Gegner nicht angreifen */
				&& !g_enemy_sheets[target_object_id - 10].flags.renegade
#endif
			) {
				will_attack = 1;
				done = 1;
			} else if (
				(target_object_id != 0)
				&& (
#if FIX_0019
					/* Fix 19: Fluchtfelder (negative Ids) sind kein Held, sie beenden die Suche */
					(target_object_id < 0) ||
#endif
#if FIX_0018
					/* Fix 18: ein uebergelaufener Gegner steht in der Schusslinie wie ein Held */
					((target_object_id >= 10) && (target_object_id < 30)
						&& !((struct enemy_flags)g_enemy_sheets[target_object_id - 10].flags).dead) ||
#endif
					(
						((target_object_id < 10)
						&& !get_hero(target_object_id - 1)->flags.dead
						&& !get_hero(target_object_id - 1)->flags.unconscious
					) || (
						(target_object_id >= 50)
						&& !is_in_int_array(target_object_id - 50, g_cb_obj_nonobstacle)
					))
				)
			)
			{
				done = 1;
			}
		}
	}

	if (will_attack == 0) {
		return 0;
	} else {
		return target_object_id;
	}
}

/**
 * \brief   selects a target for a ranged weapon attack (autofight)
 *
 *          If no target is in a straight line, the hero moves towards a firing position.
 *          (former text "with a spell" was wrong: spells are handled by AFIG_select_autospell)
 *
 * \param   hero        pointer to the hero
 * \param   hero_pos    position of the hero in the party
 * \param   renegade    0 = hero normal, 1 = hero renegade
 * \param   x           x-coordinate of the hero
 * \param   y           y-coordinate of the hero
 * \return              0 = no target found, 1 = target found (distance >= 2),
 *                      2 = target adjacent (too close for a ranged attack, the hero changes the weapon)
 */
signed int AFIG_select_range_target(struct struct_hero *hero, const signed int hero_pos, const signed int renegade, signed int x, signed int y)
{
	signed int viewdir;
	signed int count;
	signed int done;
	signed int retval;
	signed int target_found;
	signed int target_x;
	signed int target_y;

	retval = 0;
	done = 0;

	while ((done == 0) && (hero->fight_bp_left > 0)) {

		/* reset target fight-id */
		hero->target_object_id = 0;

		/* REMARK: a ranged attack requires 3 BP (former remark: "spells usually require 5 BP") */
		if (hero->fight_bp_left >= 3) {

			viewdir = hero->viewdir;

			count = 0;

			/* try to find a target clockwise from current direction */
			while (!hero->target_object_id && (count < 4)) {

				hero->target_object_id = AFIG_search_line_target(x, y, viewdir, renegade);

				count++;

				if (++viewdir == FIG_VIEWDIR__END) {
					viewdir = FIG_VIEWDIR__BEGIN;
				}
			}
		}

		/* check if a target was found */
		if (hero->target_object_id != 0) {

			/* yes */

			FIG_search_obj_on_cb(hero->target_object_id, &target_x, &target_y);

			if (manhattan_distance(target_x, target_y, x, y) < 2) {
				retval = 2;
			} else {
				retval = 1;
			}

			done = 1;

		} else if (!hero->flags.tied) {
			/* try a more expensive search */

			if (renegade == 0) {
				target_found = FIG_find_path_to_target((uint8_t*)hero, hero_pos, x, y, 9);
			} else {
				target_found = FIG_find_path_to_target((uint8_t*)hero, hero_pos, x, y, 8);
			}

			if (target_found != -1) {

				FIG_prepare_hero_ani(hero, hero_pos);

				FIG_search_obj_on_cb(hero_pos + 1, &x, &y);

				/* REMARK: a ranged attack requires 3 BP (former remark: "spells usually require 5 BP") */
				if (hero->fight_bp_left < 3) {

					/* set BP to 0 */
					hero->fight_bp_left = 0;
				}
			} else {
				/* set BP to 0 */
				hero->fight_bp_left = 0;
			}
		} else {
			/* set BP to 0 */
			hero->fight_bp_left = 0;
		}
	}

	return retval;
}

/**
 * \brief   determines the target mode of a spell for the autofight
 *
 * \param   spell_id    spell index
 * \param   renegade    0 = hero normal, 1 = hero renegade
 * \return              -1 = spell not usable,
 *                      0 = target is an enemy (spell target_type 1 or 3),
 *                      1 = target is a hero or renegade enemy (normal hero: target_type 2,
 *                          renegade hero: target_type 3),
 *                      2 = no target / the caster himself
 */
signed int AFIG_get_spell_target_mode(const signed int spell_id, const signed int renegade)
{
	struct spell_descr *spell_description;
	signed int retval = -1;

	/* make a pointer to the spell description */
	spell_description = &g_spell_descriptions[spell_id];

	if (renegade == 0) {
		if (spell_description->target_type == 2) {
			retval = 1;
		} else if ((spell_description->target_type == 1) || (spell_description->target_type == 3)) {
			retval = 0;
		} else {
			retval = 2;
		}
	} else if (spell_description->target_type == 3) {
		retval = 1;
	} else if (spell_description->target_type == 0) {
		retval = 2;
	}

	return retval;
}

/**
 * \brief   autofight: selects a spell from g_af_spell_list and a target for it
 *
 *          The list is walked from the start: a spell with range 1 is taken with about 61%
 *          (49% + 51% * 24%), any other with 24%; if none is taken, a random one.
 *          If no target is found,
 *          the hero moves towards one.
 *
 * \param   hero        pointer to the hero
 * \param   hero_pos    position of the hero in the party
 * \param   renegade    0 = hero normal, 1 = hero renegade
 * \param   x           x-coordinate of the hero
 * \param   y           y-coordinate of the hero
 * \return              1 = spell (hero->spell_id) and target (hero->target_object_id) chosen, 0 = no spell
 */
signed int AFIG_select_autospell(struct struct_hero *hero, const signed int hero_pos, const signed int renegade, signed int x, signed int y)
{
	signed int i;
	signed int count;
	signed int spell_id;
	signed int done;
	signed int retval;
	signed int spell_mode;
	signed int target_found;
	signed int decided;

	struct viewdir_offsets a = g_fig_viewdir_offsets3;

	retval = 0;
	done = 0;

	while ((done == 0) && (hero->fight_bp_left > 0)) {

		decided = 0;

		for (i = 0; i <= 10; i++) {

			/* get a spell from an array */
			spell_id = g_af_spell_list[i];

			if ((g_spell_descriptions[spell_id].range == 1) && (random_schick(100) < 50))
			{
				decided = 1;
				break;

			} else if (random_schick(100) < 25) {
				decided = 1;
				break;
			}
		}

		if (decided == 0) {

			spell_id = g_af_spell_list[random_interval(0, 10)];
		}

		/* reset the target of the hero */
		hero->target_object_id = 0;

#if FIX_0319
		/* Fix 319: einen Zauber mit Zauberwert unter -5 kann der Held nicht wirken (test_spell() liefert 0,
		 * use_spell() zieht trotzdem die halben AE ab) - dann in diesem Zug nicht zaubern */
		if (hero->spells[spell_id] < -5) {
			done = 1;
			continue;
		}
#endif

		if ((spell_mode = AFIG_get_spell_target_mode(spell_id, renegade)) != -1) {

			if (spell_mode == 2) {

				/* set target to hero */
				hero->target_object_id = hero_pos + 1;

				/* set spell */
				hero->spell_id = spell_id;

				retval = 1;
				done = 1;

			} else if (!g_spell_descriptions[spell_id].range) {

				while ((hero->fight_bp_left != 0) && (done == 0)) {

					i = hero->viewdir;

					count = 0;
					while ((!hero->target_object_id) && (count < 4)) {

						if (AFIG_can_attack_neighbour(x, y, a.offset[i].x, a.offset[i].y, spell_mode)) {
							hero->target_object_id = get_cb_val(x + a.offset[i].x, y + a.offset[i].y);
						}

						count++;

						if (++i == 4) {
							i = 0;
						}
					}

					if (hero->target_object_id != 0) {

						if (hero->fight_bp_left >= 5) {

							/* enough BP */
							hero->spell_id = spell_id;
							retval = 1;
						} else {
							/* set BP to 0 */
							hero->fight_bp_left = 0;
						}

						done = 1;

					} else if (!hero->flags.tied) {

						if (spell_mode == 0) {
							target_found = FIG_find_path_to_target((uint8_t*)hero, hero_pos, x, y, 3);
						} else {
							target_found = FIG_find_path_to_target((uint8_t*)hero, hero_pos, x, y, 1);
						}

						if (target_found != -1) {

							FIG_prepare_hero_ani(hero, hero_pos);
							FIG_search_obj_on_cb(hero_pos + 1, &x, &y);

						} else {
							/* set BP to 0 */
							hero->fight_bp_left = 0;
						}

					} else {
						/* set BP to 0 */
						hero->fight_bp_left = 0;
					}
				}
			} else {
				while ((done == 0) && (hero->fight_bp_left > 0)) {

					i = hero->viewdir;

					count = 0;

					while (!hero->target_object_id && (count < 4)) {

						hero->target_object_id = AFIG_search_line_target(x, y, i, spell_mode);

						count++;

						if (++i == 4) {
							i = 0;
						}
					}

					if (hero->target_object_id != 0) {

						if (hero->fight_bp_left >= 5) {

							/* enough BP */
							hero->spell_id = spell_id;
							retval = 1;
						} else {
							/* set BP to 0 */
							hero->fight_bp_left = 0;
						}

						done = 1;

					} else if (!hero->flags.tied) {

						if (spell_mode == 0) {
							target_found = FIG_find_path_to_target((uint8_t*)hero, hero_pos, x, y, 9);
						} else {
							target_found = FIG_find_path_to_target((uint8_t*)hero, hero_pos, x, y, 8);
						}

						if (target_found != -1) {

							FIG_prepare_hero_ani(hero, hero_pos);
							FIG_search_obj_on_cb(hero_pos + 1, &x, &y);

						} else {
							/* set BP to 0 */
							hero->fight_bp_left = 0;
						}

					} else {
						/* set BP to 0 */
						hero->fight_bp_left = 0;
					}
				}
			}
		}
	}

	return retval;
}

/**
 * \brief   count the other heroes in the current group
 *
 *          REMARK: uses the first gs_group_member_counts[...] hero slots (GRP_sort_heroes()
 *          moves the active group to the front; an NPC in slot 6 is not covered).
 *
 * \param   hero_pos    position of the calling hero
 * \return              number of other heroes in the group which can act (check_hero())
 */
signed int AFIG_count_heroes(const signed int hero_pos)
{
	signed int cnt = 0;
	signed int i;

	/* for each hero in this group */
	for (i = 0; gs_group_member_counts[gs_active_group_id] > i; i++) {

		if ((i != hero_pos) && check_hero(get_hero(i))) {
			cnt++;
		}
	}

	return cnt;
}

/**
 * \brief   automatic turn of a hero (autofight, NPC or renegade hero)
 *
 *          NPCs flee by their own rules; then flight, spell, ranged or melee attack is
 *          chosen, otherwise the hero moves towards a target.
 *
 * \param   hero        pointer to the hero
 * \param   hero_pos    position of the hero in the party
 * \param   x           x-coordinate of the hero
 * \param   y           y-coordinate of the hero
 */
void AFIG_hero_turn(struct struct_hero *hero, const signed int hero_pos, signed int x, signed int y)
{
	signed int i;
	signed int cnt;
	signed int done;
	signed int target_found;
	signed int try_autospell;
	signed int x_bak;
	signed int y_bak;
	signed int range_type;
	signed int hero_x;
	signed int hero_y;

	struct viewdir_offsets a = g_fig_viewdir_offsets4;

	done = 0;
	try_autospell = 1;

	hero->action_id = FIG_ACTION_MOVE;

	if (hero->npc_id > 0) {

		if (hero->npc_id == NPC_NARIELL) {

			/* equip LONGBOW and ARROWS in the first round,
			 * if the hero has them in the inventory */
			if ((g_fight_round == 0) &&
				(hero->inventory[HERO_INVENTORY_SLOT_RIGHT_HAND].item_id != ITEM_ID_LANGBOGEN) &&
				(inv_slot_of_item(hero, ITEM_ID_PFEIL) != -1) &&
				(inv_slot_of_item(hero, ITEM_ID_LANGBOGEN) != -1))
			{
				move_item(HERO_INVENTORY_SLOT_RIGHT_HAND, inv_slot_of_item(hero, ITEM_ID_LANGBOGEN), hero);

				if (hero->inventory[HERO_INVENTORY_SLOT_LEFT_HAND].item_id != ITEM_ID_PFEIL) {
					move_item(HERO_INVENTORY_SLOT_LEFT_HAND, inv_slot_of_item(hero, ITEM_ID_PFEIL), hero);
				}
			}

		} else if (hero->npc_id == NPC_HARIKA) {

			if (hero->le <= 12) {

				/* equip LONGBOW and ARROWS in the first round,
				 * if the hero has them in the inventory */
				if ((g_fight_round == 0) &&
					(hero->inventory[HERO_INVENTORY_SLOT_RIGHT_HAND].item_id != ITEM_ID_LANGBOGEN) &&
					(inv_slot_of_item(hero, ITEM_ID_PFEIL) != -1) &&
					(inv_slot_of_item(hero, ITEM_ID_LANGBOGEN) != -1))
				{
					move_item(HERO_INVENTORY_SLOT_RIGHT_HAND, inv_slot_of_item(hero, ITEM_ID_LANGBOGEN), hero);

					if (hero->inventory[HERO_INVENTORY_SLOT_LEFT_HAND].item_id != ITEM_ID_PFEIL) {
						move_item(HERO_INVENTORY_SLOT_LEFT_HAND, inv_slot_of_item(hero, ITEM_ID_PFEIL), hero);
					}

				} else if (FIG_weapon_gfx_id_ranged(hero) == WEAPON_GFX_ID_NONE) {
					hero->action_id = FIG_ACTION_FLEE;
				}
			}

		} else if (hero->npc_id == NPC_CURIAN) {

			if ((hero->le < 10) && (hero->ae < 10))
			{
				hero->action_id = FIG_ACTION_FLEE;
			}

		} else if (hero->npc_id == NPC_ARDORA) {

			if (hero->le < 8)
			{
				hero->action_id = FIG_ACTION_FLEE;
			}

		} else if (hero->npc_id == NPC_GARSVIK) {

			if (!AFIG_count_heroes(hero_pos)) {
				hero->action_id = FIG_ACTION_FLEE;
			}

		} else if (hero->npc_id == NPC_ERWO) {

			if (hero->le < 15)
			{
				hero->action_id = FIG_ACTION_FLEE;
			}

		}

		/* the NPC (slot 6) is the only hero who can still fight -> flee */
		if (FIG_get_first_active_hero() == 6) {

			hero->action_id = FIG_ACTION_FLEE;
		}

		if (hero->action_id == FIG_ACTION_FLEE) {

			FIG_search_obj_on_cb(hero_pos + 1, &hero_x, &hero_y);

			/* if the hero is surrounded on all four sides, he cannot flee.
			 * REMARK: the bounds (hero_x + 1 <= 25) are too wide, the board is 0..23 */
			if (
#if FIX_0020
				/* Fix 20: Grenze 23 (das Feld ist 0..23) */
				(hero_x - 1 >= 0) && (hero_x + 1 <= 23)
				&& (hero_y -1 >= 0) && (hero_y + 1 <= 23)
#else
				(hero_x - 1 >= 0) && (hero_x + 1 <= 25)
				&& (hero_y -1 >= 0) && (hero_y + 1 <= 25)
#endif
				&& (get_cb_val(hero_x, hero_y + 1) != 0)
				&& (get_cb_val(hero_x, hero_y - 1) != 0)
				&& (get_cb_val(hero_x + 1, hero_y) != 0)
				&& (get_cb_val(hero_x - 1, hero_y) != 0)
			) {
				hero->action_id = FIG_ACTION_MOVE;
			}
		}
	}

	if (hero->fight_bp_left == 1) {
		/* set BP to 0 */
		hero->fight_bp_left = 0;
	}

	while ((done == 0) && (hero->fight_bp_left > 0)) {

		/* REMARK: should be done in the timer */
		CD_audio_repeat_check();

		if ((hero->action_id == FIG_ACTION_FLEE) && (hero->fight_bp_left > 0)) {

			if (!hero->flags.tied) {

				target_found = FIG_find_path_to_target((uint8_t*)hero, hero_pos, x, y, 5);

				if (target_found != -1) {

					x_bak = x;
					y_bak = y;

					FIG_prepare_hero_ani(hero, hero_pos);

					hero->target_object_id = 0;

					if (FIG_search_obj_on_cb(hero_pos + 1, &x, &y)) {

						hero->action_id = FIG_ACTION_MOVE;

						if ((x_bak == x) && (y_bak == y)) {

							hero->fight_bp_left = 0;
						}
					} else {
						hero->fight_bp_left = 0;
					}

					if (hero->fight_bp_left < 3) {
						hero->fight_bp_left = 0;
					}

				} else {
					hero->fight_bp_left = 0;
				}
			} else {
				hero->fight_bp_left = 0;
			}

		} else {

			/* REMARK: hero->bp should be >= 5 */
			if (
				(hero->typus >= HERO_TYPE_HEXE)		/* spellcaster */
				&& (hero->ae > 10)	/* AE > 10 */
				&& (try_autospell != 0)
				&& (g_current_fight_id != FIGHT_ID_F144)	/* not in the final fight */
#if FIX_0385
				/* Fix 385: ein NSC (Curian) zaubert auch im normalen Kampf, nicht nur im Computerkampf mit Magie */
				&& (g_autofight_magic || (hero->npc_id > 0))
#else
				&& g_autofight_magic /* magic activated in auto fight */
#endif
			) {
				if (AFIG_select_autospell(hero, hero_pos, hero->flags.renegade, x, y))
				{
					hero->action_id = FIG_ACTION_SPELL;
					hero->fight_bp_left = 0;

				} else {
					try_autospell = 0;
				}
			}

#if FIX_0316
			/* Fix 316: ist die Waffe zerbrochen und liegt eine heile Nahkampfwaffe im Rucksack, zu ihr greifen */
			if ((hero->action_id == FIG_ACTION_MOVE) && (hero->fight_bp_left > 2) &&
				g_itemsdat[hero->inventory[HERO_INVENTORY_SLOT_RIGHT_HAND].item_id].flags.weapon &&
				hero->inventory[HERO_INVENTORY_SLOT_RIGHT_HAND].flags.broken)
			{
				for (i = HERO_INVENTORY_SLOT_KNAPSACK_1; i < NR_HERO_INVENTORY_SLOTS; i++) {

					cnt = hero->inventory[i].item_id;

					if (g_itemsdat[cnt].flags.weapon && !hero->inventory[i].flags.broken &&
						(g_itemsdat[cnt].item_subtype_id != WEAPON_TYPE_SCHUSSWAFFE) &&
						(g_itemsdat[cnt].item_subtype_id != WEAPON_TYPE_WURFWAFFE) &&
						(g_itemsdat[cnt].item_subtype_id != WEAPON_TYPE_SPEER))
					{
						AFIG_switch_to_melee_weapon(hero);
						break;
					}
				}
			}
#endif

			if ((hero->action_id == FIG_ACTION_MOVE) && (hero->fight_bp_left > 0)) {

				if (FIG_weapon_gfx_id_ranged(hero) != WEAPON_GFX_ID_NONE) {

					if (range_attack_check_ammo(hero, 2)) {

						range_type = AFIG_select_range_target(hero, hero_pos, hero->flags.renegade, x, y);

						if (range_type != 0) {
							if (range_type == 2) {
								if (!AFIG_switch_to_melee_weapon(hero)) {
									done = 1;
								}
							} else {
								hero->action_id = FIG_ACTION_RANGE_ATTACK;
							}
						}

						/* set BP to 0 */
						hero->fight_bp_left = 0;
					} else if (!AFIG_switch_to_melee_weapon(hero)) {
						done = 1;
					}
				} else {
					hero->target_object_id = 0;

					if (hero->fight_bp_left >= 3) {

						i = hero->viewdir;
						cnt = 0;

						while (!hero->target_object_id && (cnt < 4)) {

							if (AFIG_can_attack_neighbour(x, y, a.offset[i].x, a.offset[i].y, hero->flags.renegade))
							{
								hero->target_object_id = get_cb_val(x + a.offset[i].x, y + a.offset[i].y);
							}

							cnt++;
							if (++i == 4) {
								i = 0;
							}
						}
					}

					if (hero->target_object_id != 0) {

						hero->action_id = FIG_ACTION_MELEE_ATTACK;
						hero->fight_bp_left = 0;

					} else if (!hero->flags.tied) {

						if (!hero->flags.renegade) {
							target_found = FIG_find_path_to_target((uint8_t*)hero, hero_pos, x, y, 3);
						} else {
							target_found = FIG_find_path_to_target((uint8_t*)hero, hero_pos, x, y, 1);
						}

						if (target_found != -1) {
							x_bak = x;
							y_bak = y;

							FIG_prepare_hero_ani(hero, hero_pos);

							hero->action_id = FIG_ACTION_MOVE;
							hero->target_object_id = 0;

							FIG_search_obj_on_cb(hero_pos + 1, &x, &y);

							if ((x_bak == x) && (y_bak == y)) {
								hero->fight_bp_left = 0;
							}

							if (hero->fight_bp_left < 3) {
								hero->fight_bp_left = 0;
							}
						} else {
							hero->fight_bp_left = 0;
						}
					} else {
						hero->fight_bp_left = 0;
					}
				}
			}
		}
	}
}
