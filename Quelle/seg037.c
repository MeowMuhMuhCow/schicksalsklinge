/*
 * seg037 - Kampf: Zug eines Gegners
 *
 * Inhalt:    FIG_enemy_turn() entscheidet fuer einen Gegner: Sonderregeln einzelner Kaempfe
 *            (Gorah F064, Harpyien F099, Woelfe F122, Endkampf F144), Flucht bei Erreichen
 *            der LE-Grenze, 4 % Chance, sich aus "Band und Fessel" zu befreien, dann Zauber
 *            (FIG_select_mspell()), Fernkampf (FIG_enemy_range_attack()) oder Nahkampf;
 *            findet er kein Ziel, geht er darauf zu. prepare_enemy_ani() baut die
 *            Bewegungsanimation.
 * Aufrufer:  FIG_do_round() (seg032); prepare_enemy_ani() auch aus seg036 und seg044
 *            (Verweise).
 * Besonders: FIG_select_mspell(): ein abtruenniger Gegner, dessen Zauberbuch keinen Spruch
 *            der Art 0 oder 3 enthaelt (Zauberbuch 8: Balsam, Verwandlung beenden,
 *            Eisenrost), findet nie einen Spruch -> Endlosschleife. Fernkampf auch gegen
 *            Nachbarfelder (Bright-Eyes). Kommentare zu Illusion und zum Zweck der
 *            Nachbarpruefung berichtigt.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg037.c
 */
#include <string.h>

#include "v302de.h"
#include "common.h"

#include "seg005.h"
#include "seg006.h"
#include "seg007.h"
#include "seg032.h"
#include "seg038.h"

/*
 * Note (C build): the enemy flags in this module are read via a pointer cast
 * ((struct enemy_flags*)&g_enemy_sheets[i].flags)->dead. With the plain form
 * g_enemy_sheets[i].flags.dead Borland C folds the field offset into the
 * address fixup ([bx+sym]), the original computes it separately
 * (mov dx,sym; add dx,0x31). For 'renegade' (byte 0x32) the extra (uint8_t*)
 * cast is needed to get 'add dx,0x32'.
 */

static struct viewdir_offsets g_fig_viewdir_offsets5 = { { { 1, 0 }, { 0, -1 }, { -1, 0 }, { 0, 1 } } };	// ds:0x5fd8
static struct viewdir_offsets g_fig_viewdir_offsets6 = { { { 1, 0 }, { 0, -1 }, { -1, 0 }, { 0, 1 } } };	// ds:0x5fe8

/**
 * \brief   copies an animation sequence from ANI.DAT (or WEAPANI.DAT)
 *
 *          REMARK: ani_num == ani_max_num is not rejected (valid are 0..ani_max_num-1)
 *
 * \param   dst         destination
 * \param[in]   ani_num animation seqence to copy
 * \param[in]   mode    3 for WEAPANI.DAT, anything else is ANI.DAT
 * \return              the number of copied bytes.
 */
static signed int copy_ani_sequence(int8_t *dst, const signed int ani_num, const signed int mode)
{
	uint8_t *p_datbuffer;
	int8_t *p_datitem;
	signed char len;
	signed int i;
	signed int ani_max_num;

	/* ANI.DAT */
	p_datbuffer = g_buffer_anidat;

	/* WEAPANI.DAT */
	if (mode == 3)
		p_datbuffer = g_buffer_weapanidat;

	ani_max_num = *(int16_t*)p_datbuffer;

	/* Sanity check of no */
	if (ani_num < 0)
		return 0;

	if (ani_num > ani_max_num)
		return 0;

	/* set p_datitem to the requested data entry */
	p_datitem = (int8_t*)p_datbuffer;
	p_datitem += ani_max_num + 2;

	len = (int8_t)p_datbuffer[2];

	for (i = 1; i <= ani_num; i++) {
		p_datitem += len;
		len = *(int8_t*)(p_datbuffer + i + 2);
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
 * \brief   builds the movement animation of an enemy along the path in g_fig_move_pathdir,
 *          plays it and subtracts the BP (also for the tail of a double-size enemy)
 *
 * \param   enemy       pointer to the enemy sheet
 * \param   enemy_id    index of the enemy (object id = enemy_id + 10)
 */
void prepare_enemy_ani(struct enemy_sheet *enemy, const signed int enemy_id)
{
	signed char ani_index_ptr_index_1;
	signed char ani_index_ptr_index_2;
	signed char viewdir_iter;
	int8_t *p_ani_clip_base;
	struct struct_fighter *p_fighter;
	int16_t *ani_index_ptr;

	signed int i;

	g_fig_ani_tracks[FANI_TRACK_ID_ACTOR_1_BASE][0] = 0;
	g_fig_ani_tracks[FANI_TRACK_ID_ACTOR_1_BASE][242] = enemy->actor_sprite_id;

	p_ani_clip_base = &g_fig_ani_tracks[FANI_TRACK_ID_ACTOR_1_BASE][1];
	i = 0;
	ani_index_ptr = g_gfx_ani_index[enemy->actor_sprite_id];

	while (g_fig_move_pathdir[i] != -1) {

		/* Concerning the following code block: There are parallel code blocks in
		 * prepare_enemy_ani(...) in seg037.c
		 * FIG_prepare_hero_ani(...) in seg036.c
		 * FANI_prepare_fight_hero_ani(...) in seg044.c
		 * FANI_prepare_fight_enemy_ani(...) in seg044.c
		 * FANI_prepare_spell_hero(...) in seg044.c
		 * FANI_prepare_spell_enemy(...) in seg044.c
		 */
		if (enemy->viewdir != g_fig_move_pathdir[i]) {

			ani_index_ptr_index_2 = ani_index_ptr_index_1 = FIG_VIEWDIR__NONE;
			viewdir_iter = enemy->viewdir;
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

					ani_index_ptr_index_2 = enemy->viewdir + 4;
					ani_index_ptr_index_1 = FIG_VIEWDIR__NONE;
				}

			}

			enemy->viewdir = g_fig_move_pathdir[i];

			p_ani_clip_base += copy_ani_sequence(p_ani_clip_base, ani_index_ptr[ani_index_ptr_index_2], 1);

			if (ani_index_ptr_index_1 != FIG_VIEWDIR__NONE) {

				p_ani_clip_base += copy_ani_sequence(p_ani_clip_base, ani_index_ptr[ani_index_ptr_index_1], 1);
			}
		}

		if (g_fig_move_pathdir[i] == g_fig_move_pathdir[i + 1]) {

			p_ani_clip_base += copy_ani_sequence(p_ani_clip_base, ani_index_ptr[g_fig_move_pathdir[i] + 12], 1);
			i += 2;
			/* BP - 2 */
			enemy->bp = enemy->bp - 2;

		} else {
			p_ani_clip_base += copy_ani_sequence(p_ani_clip_base, ani_index_ptr[g_fig_move_pathdir[i] + 8], 1);
			i++;
			/* BP - 1 */
			enemy->bp--;
		}
	}

	/* terminate array */
	*p_ani_clip_base = -1;
	FIG_call_draw_pic();
	FIG_remove_from_list(g_fig_cb_marker_id, 0);
	g_fig_cb_marker_id = -1;
	FIG_set_ani_track_id_base(enemy->fighter_id, FANI_TRACK_ID_ACTOR_1_BASE);

	if (is_in_byte_array(enemy->actor_sprite_id, g_double_size_actor_sprite_id_table)) {

		memcpy(&g_fig_ani_tracks[FANI_TRACK_ID_ACTOR_1_TAIL], &g_fig_ani_tracks[FANI_TRACK_ID_ACTOR_1_BASE], 243);

		p_fighter = FIG_get_fighter(enemy->fighter_id);

		FIG_set_ani_track_id_base(g_fig_double_size_fighter_id_table[p_fighter->double_size], FANI_TRACK_ID_ACTOR_1_TAIL);
	}

	/* draw_fight_screen */
	draw_fight_screen(0);
	memset(&g_fig_ani_tracks[FANI_TRACK_ID_ACTOR_1_BASE], -1, 243);
	memset(&g_fig_ani_tracks[FANI_TRACK_ID_ACTOR_1_TAIL], -1, 243);
	FIG_init_list_elem(enemy_id + 10);
}


/**
 * \brief   checks if the neighbouring square (x + dx, y + dy) holds a target for a melee
 *          attack or touch spell of an enemy (former text "range attack" was wrong)
 *
 * \param   x           x - coordinate of attacker
 * \param   y           y - coordinate of attacker
 * \param   dx          delta to x (viewing direction)
 * \param   dy          delta to y (viewing direction)
 * \param   mode        0 = living, conscious hero or living renegade enemy (normal enemy),
 *                      1 = living enemy (renegade enemy), 2 = living, conscious hero only
 * \return              0 if theres nothing to attack else 1
 */
signed int FIG_enemy_can_attack_neighbour(const signed int x, const signed int y, const signed int dx, const signed int dy, const signed int mode)
{
	const signed char target_object_id = get_cb_val(x + dx, y + dy);

	if (mode == 0) {

		if (
			(
				(target_object_id > 0) && (target_object_id < 10)
				&& !get_hero(target_object_id - 1)->flags.dead && !get_hero(target_object_id - 1)->flags.unconscious
			) || (
				(target_object_id >= 10) && (target_object_id < 30)
				&& !((struct enemy_flags*)&g_enemy_sheets[target_object_id - 10].flags)->dead
			//	&& g_enemy_sheets[target_object_id - 10].flags.renegade
				&& ((struct enemy_flags*)(uint8_t*)&g_enemy_sheets[target_object_id - 10].flags)->renegade
			)
		) {
			return 1;
		} else {
			return 0;
		}

	} else if (mode == 1) {

		/* is a living enemy */
		if (
			(target_object_id >= 10) && (target_object_id < 30)
			&& !((struct enemy_flags*)&g_enemy_sheets[target_object_id - 10].flags)->dead
		) {
			return 1;
		} else {
			return 0;
		}

	} else if (mode == 2) {

		/* is a living, conscious hero */
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
 * \brief   checks if range attack is possible
 *
 * \param   x           x - coordinate of attacker
 * \param   y           y - coordinate of attacker
 * \param   viewdir     viewing direction
 * \param   mode        0 = hero or renegade enemy (normal enemy), 1 = enemy (renegade enemy),
 *                      2 = hero only
 * \return              0 if theres nothing to attack in that direction
 *                      or the ID of the attackee.
 */
/*
 * Original-Bug: range attack of enemies is possible with direct contact
 */
signed int FIG_search_range_target(const signed int x, const signed int y, const signed int viewdir, const signed int mode)
{
	signed int done;
	signed int x_diff;	/* run variables in viewdir */
	signed char target_object_id;
	signed char target; /* unused */
	signed int y_diff;	/* run variables in viewdir */
	signed int can_attack;

	done = 0;
	x_diff = 0;
	y_diff = 0;
	can_attack = 0;

	while (!done) {

		/* move one square forward */
		if (viewdir == FIG_VIEWDIR_RIGHT) {
			x_diff++;
		} else if (viewdir == FIG_VIEWDIR_DOWN) {
			y_diff--;
		} else if (viewdir == FIG_VIEWDIR_LEFT) {
			x_diff--;
		} else {
			y_diff++;
		}

		/* out of chessboard */
		if (y + y_diff < 0 || y + y_diff > 23 || x + x_diff < 0 || x + x_diff > 23) {
			done = 1;
		} else {

			target_object_id = get_cb_val(x + x_diff, y + y_diff);

			if (mode == 0) {
				/* hero or enemy reacheable from enemies position */
				if (
					(
						(target_object_id > 0) && (target_object_id < 10)
						&& !get_hero(target_object_id - 1)->flags.dead
						&& !get_hero(target_object_id - 1)->flags.unconscious
					) || (
						(target_object_id >= 10) && (target_object_id < 30)
						&& !((struct enemy_flags*)&g_enemy_sheets[target_object_id - 10].flags)->dead
					//	&& g_enemy_sheets[target_object_id - 10].flags.renegade
						&& ((struct enemy_flags*)(uint8_t*)&g_enemy_sheets[target_object_id - 10].flags)->renegade
					)
				) {
					can_attack = 1;
					done = 1;

				} else

				/* if square is not empty */
				if (target_object_id != 0) {

					/* an enemy or another object */
					if (
						(
							(target_object_id >= 10) && (target_object_id < 30)
							&& !((struct enemy_flags*)&g_enemy_sheets[target_object_id - 10].flags)->dead
						) || (
							(target_object_id >= 50)
							&& !is_in_int_array(target_object_id - 50, g_cb_obj_nonobstacle)
						)
					) {
							done = 1;
					}
				}

			} else if (mode == 1) {
				/* attack foe first */
				if (
					(target_object_id >= 10) && (target_object_id < 30)
					&& !((struct enemy_flags*)&g_enemy_sheets[target_object_id - 10].flags)->dead
				) {
					can_attack = 1;
					done = 1;
				} else
				/* skip zeros */
				if (target_object_id != 0) {

#ifdef M302de_ORIGINAL_BUGFIX
					/* Original-Bugfix: the next if assumes
						that a negative target_object_id is a hero -> SEGFAULT*/
					if (target_object_id < 0) {
						done = 1;
					} else
#endif

					/* handle heroes or walls */
					if (
						(
							(target_object_id < 10)
							&& !get_hero(target_object_id - 1)->flags.dead
							&& !get_hero(target_object_id - 1)->flags.unconscious
						) || (
							(target_object_id >= 50)
							&& !is_in_int_array(target_object_id - 50, g_cb_obj_nonobstacle)
						)
					) {
						done = 1;
					}
				}

			} else if (mode == 2) {
				/* attack hero */
				if (
					(target_object_id > 0) && (target_object_id < 10)
					&& !get_hero(target_object_id - 1)->flags.dead
					&& !get_hero(target_object_id - 1)->flags.unconscious
				) {
					can_attack = 1;
					done = 1;
				} else if (target_object_id != 0) {

#ifdef M302de_ORIGINAL_BUGFIX
					if (target_object_id < 0) {
						done = 1;
					} else
#endif

					if (
						(
							(target_object_id < 10)
							&& !get_hero(target_object_id - 1)->flags.dead
							&& !get_hero(target_object_id - 1)->flags.unconscious
						) || (
							(target_object_id >= 50)
							&& !is_in_int_array(target_object_id - 50, g_cb_obj_nonobstacle)
						) || (
							(target_object_id >= 10)
							&& (target_object_id < 30)
							&& !((struct enemy_flags*)&g_enemy_sheets[target_object_id - 10].flags)->dead
						)
					) {
						done = 1;
					}
				}
			}
		}
	}

	if (can_attack == 0)
		return 0;
	else
		return target_object_id;
}


/**
 * \brief   determines the target mode of a monster spell
 *
 *          desc->mode: 0 = caster himself, 1 = own side (enemies), 2 = heroes, 3 = anyone
 *
 * \param   mspell_id   monster spell id
 * \param   attack_foe  0 = normal enemy, 1 = renegade enemy (former name: mode)
 * \return              0 = spell not usable, 1 = target: an enemy, 2 = target: a hero,
 *                      3 = target: the caster himself
 */
signed int FIG_get_mspell_target_mode(const signed int mspell_id, const signed int attack_foe)
{
	signed int retval = 0;
	struct mon_spell_description *desc = &g_mon_spell_descriptions[mspell_id];

	if (attack_foe == 0) {

		if ((desc->mode == 3) || (desc->mode == 2)) {
			retval = 2;
		} else {
			if (desc->mode == 1) {
				retval = 1;
			} else {
				retval = 3;
			}
		}

	} else {
		if (desc->mode == 3) {
			retval = 1;
		} else if (desc->mode == 0) {
			retval = 3;
		}
	}

	return retval;
}

/**
 * \brief   selects a spell from the spellbook of an enemy and a target for it;
 *          moves the enemy towards a target if needed
 *
 * \param   enemy       pointer to the enemy sheet
 * \param   enemy_id    index of the enemy (object id = enemy_id + 10)
 * \param   attack_foe  0 = normal enemy, 1 = renegade enemy
 * \param   x           x-coordinate of the enemy
 * \param   y           y-coordinate of the enemy
 * \return              1 = spell (enemy->mspell_id) and target chosen, 0 = no spell
 */
signed int FIG_select_mspell(struct enemy_sheet* enemy, const signed int enemy_id, const signed int attack_foe, signed int x, signed int y)
{
	signed int available_spells;
	signed int mspell_id;
	signed int done;
	signed int retval;
	signed int mode;
	signed int target_found;
	signed int decided;
	struct viewdir_offsets diff = g_fig_viewdir_offsets5;

	signed int i;
	signed int cnt;

	retval = 0;

	available_spells = i = 0;
	while ((i < 5) && (g_mon_spellbooks[enemy->spellbook_id][i++] != -1))
	{
		available_spells++;
	}

	/* REMARK: if FIG_get_mspell_target_mode() returns 0 for every spell of the book (renegade enemy, book
	 * without spells of mode 0 or 3, e.g. g_mon_spellbooks[8]), neither done nor bp changes:
	 * endless loop. Original-Bug? */
	done = 0;
	while ((done == 0) && (enemy->bp > 0)) {

		decided = 0;

		for (i = 0; i < available_spells; i++) {

			mspell_id = g_mon_spellbooks[enemy->spellbook_id][i];

			/* is_ranged == 1: ranged spell (Blitz, Eisenrost, Fulminictus, Ignifaxius, Plumbumbarum).
			 * Each ranged spell of the book is taken with 74% */
			if (g_mon_spell_descriptions[mspell_id].is_ranged == 1) {

				if (random_schick(100) < 75) {
					decided = 1;
					break;
				}
			}
		}

		if (decided == 0) {
			mspell_id = g_mon_spellbooks[enemy->spellbook_id][random_interval(0, available_spells - 1)];
		}

		enemy->target_object_id = 0;

		if ( (mode = FIG_get_mspell_target_mode(mspell_id, attack_foe)) > 0) {

			if (mode == 3) {
				enemy->target_object_id = enemy_id + 10;
 				enemy->mspell_id = mspell_id;
				retval = 1;
				done = 1;
			} else if (!g_mon_spell_descriptions[mspell_id].is_ranged) {
				/* touch spell: target must be adjacent */

				while (enemy->bp && (done == 0)) {

					i = enemy->viewdir;
					cnt = 0;

					while (!enemy->target_object_id && (cnt < 4)) {

						if (FIG_enemy_can_attack_neighbour(x, y, diff.offset[i].x, diff.offset[i].y, mode)) {
							enemy->target_object_id = get_cb_val(x + diff.offset[i].x, y + diff.offset[i].y);
						}

						cnt++;
						if (++i == 4) {
							i = 0;
						}
					}

					if (enemy->target_object_id) {

						enemy->mspell_id = mspell_id;
						retval = 1;
						done = 1;

					} else if (enemy->bp > 0) {

						if (!enemy->flags.tied) {

							if (mode == 1) {
								target_found = FIG_find_path_to_target((uint8_t*)enemy, enemy_id, x, y, 2);
							} else {
								target_found = FIG_find_path_to_target((uint8_t*)enemy, enemy_id, x, y, 0);
							}

							if (target_found != -1) {
								prepare_enemy_ani(enemy, enemy_id);
								FIG_search_obj_on_cb(enemy_id + 10, &x, &y);

								if (enemy->bp < 3) {
									enemy->bp = 0;
								}
							} else {
								enemy->bp = 0;
							}
						} else {
							enemy->bp = 0;
						}
					}
				}
			} else {

				while ((done == 0) && (enemy->bp > 0)) {

					i = enemy->viewdir;
					cnt = 0;

					while (!enemy->target_object_id && (cnt < 4)) {

						enemy->target_object_id = FIG_search_range_target(x, y, i, mode);

						cnt++;
						if (++i == 4) {
							i = 0;
						}
					}

					if (enemy->target_object_id) {

						enemy->mspell_id = mspell_id;
						retval = 1;
						done = 1;

					} else if (enemy->bp > 0) {

						if (!enemy->flags.tied) {

							if (mode == 1) {
								target_found = FIG_find_path_to_target((uint8_t*)enemy, enemy_id, x, y, 7);
							} else {
								target_found = FIG_find_path_to_target((uint8_t*)enemy, enemy_id, x, y, 6);
							}

							if (target_found != -1) {
								prepare_enemy_ani(enemy, enemy_id);
								FIG_search_obj_on_cb(enemy_id + 10, &x, &y);

								if (enemy->bp < 5) {
									enemy->bp = 0;
								}
							} else {
								enemy->bp = 0;
							}
						} else {
							enemy->bp = 0;
						}
					}
				}
			}
		}
	}

#if !defined(__BORLANDC__)
	if (retval)
		D1_INFO("Zauberspruch %d\n", enemy->mspell_id);
	else
		D1_INFO("Kein Zauberspruch\n");
#endif

	return retval;
}


/**
 * \brief   searches a target in a straight line for a ranged attack of an enemy;
 *          moves the enemy towards a firing position if needed
 *
 * \param   enemy       pointer to the enemy sheet
 * \param   enemy_id    index of the enemy (object id = enemy_id + 10)
 * \param   attack_foe  0 = normal enemy, 1 = renegade enemy
 * \param   x           x-coordinate of the enemy
 * \param   y           y-coordinate of the enemy
 * \return              1 = target found (enemy->target_object_id), 0 = none
 */
signed int FIG_enemy_range_attack(struct enemy_sheet *enemy, const signed int enemy_id, const signed int attack_foe, signed int x, signed int y)
{

	signed int cnt;
	signed int done;
	signed int retval;
	signed int target_found;
	signed int viewdir;

	retval = 0;

	done = 0;

	while ((done == 0) && (enemy->bp > 0)) {

		/* reset the attackee ID */
		enemy->target_object_id = 0;

		while ( (done == 0) && (enemy->bp > 0) ) {

			viewdir = enemy->viewdir;
			cnt = 0;

			/* check clockwise for someone to attack */
			while (!enemy->target_object_id && (cnt < 4)) {

				enemy->target_object_id = FIG_search_range_target(x, y, viewdir, attack_foe);
				cnt++;
				if (++viewdir == FIG_VIEWDIR__END) {
					viewdir = FIG_VIEWDIR__BEGIN;
				}
			}

			if (enemy->target_object_id) {

				/* found someone to attack */
				retval = 1;
				done = 1;

			} else if (enemy->bp > 0) {

				if (!enemy->flags.tied) {

					if (attack_foe == 0) {
						target_found = FIG_find_path_to_target((uint8_t*)enemy, enemy_id, x, y, 6);
					} else {
						target_found = FIG_find_path_to_target((uint8_t*)enemy, enemy_id, x, y, 7);
					}

					if (target_found != -1) {
						prepare_enemy_ani(enemy, enemy_id);
						FIG_search_obj_on_cb(enemy_id + 10, &x, &y);

						if (enemy->bp < 3) {
							enemy->bp = 0;
						}
					} else {
						enemy->bp = 0;
					}
				} else {
					enemy->bp = 0;
				}
			}
		}
	}

	return retval;
}


/**
 * \brief   decides the action of an enemy in its action phase
 *
 * \param   enemy       pointer to the enemy sheet
 * \param   enemy_id    index of the enemy (object id = enemy_id + 10)
 * \param   x           x-coordinate of the enemy
 * \param   y           y-coordinate of the enemy
 */
void FIG_enemy_turn(struct enemy_sheet *enemy, const signed int enemy_id, signed int x, signed int y)
{
	signed int target_reachable;
	signed int attack_foe;
	signed int viewdir;
	signed int cnt;
	signed int done = 0;
	signed int flag;
	signed int x_bak;
	signed int y_bak;
	signed int rear_object_id;

	struct viewdir_offsets diff = g_fig_viewdir_offsets6;

	/* check if we are in a special fight */

	if (g_current_fight_id == FIGHT_ID_F064) {

		/* F064: fight against GORAH */

		if (enemy->monster_id == MONSTER_ID_GORAH) {
			/* GORAH waits the first 5 rounds */

			if (g_fight_round < 5) {
				enemy->bp = 0;
			}
		}

	} else if ((g_current_fight_id == FIGHT_ID_F099) && (random_interval(8, 12) <= g_fight_round)) {

		/* F099: fight against four HARPIES */

		/* after 8-12 rounds, the enemies flee */
		enemy->flags.scared = 1;

	} else if ((g_current_fight_id == FIGHT_ID_F122) && (FIG_count_active_enemies() <= 3)) {

		/* F122: fight against 13 WOLVES */

		/* if at most 3 wolves are left, all enemies flee */
		enemy->flags.scared = 1;

	} else if (g_current_fight_id == FIGHT_ID_F144) {

		/* F144: final fight */

		if (enemy->flags.tied) {
			enemy->bp = 0;
		}
	}

	while ( (done == 0) && (enemy->bp > 0)) {

		if (g_fig_cb_marker_id != -1) {
			FIG_remove_from_list(g_fig_cb_marker_id, 0);
			g_fig_cb_marker_id = -1;
		}

		FIG_init_list_elem(enemy_id + 10);

		draw_fight_screen_pal(0);

		enemy->target_object_id = 0;
		enemy->action_id = 1; /* FIG_ACTION_MOVE */

		/* LE threshold reached */
		if (enemy->le_flee >= enemy->le) {

			enemy->flags.scared = 1;
		}

		/* chance of 4% that a tied enemy breaks free ("Band und Fessel"). The former comment
		 * "an illusion enemy disappears" did not fit the code: only 'tied' is reset. */
		if (random_schick(100) < 5) {
#if !defined(__BORLANDC__)
			if (enemy->flags.illusion) {
				D1_INFO("Feind %d verliert seinen Illusionszauber\n", enemy_id);
			}
#endif
			/* Original-Bug? Why unset 'tied' and not 'illusion'??
			 * REMARK: probably intended as a chance to break free, see above (vermutlich). */
			enemy->flags.tied = 0;
		}

		if (!enemy->flags.scared) {

			attack_foe = 0;

			if (enemy->flags.renegade) {
				attack_foe = 1;
			}

			/* enemy can cast spells and has AE >= 5 left */
			if (
				(enemy->spellbook_id != -1) && (enemy->ae >= 5)
				&& FIG_select_mspell(enemy, enemy_id, attack_foe, x, y)
			) {
				/* REMARK: enemy can still cast a spell with less than 5 BP. */
				enemy->action_id = FIG_ACTION_SPELL;

				/* adjust BP */
				enemy->bp = enemy->bp - 5;

				if (enemy->bp < 0) {
					enemy->bp = 0;
				}
				return;
			}

			/* enemy has range weapons */
			if (
				((enemy->shots > 0) || (enemy->throws > 0))
				&& FIG_enemy_range_attack(enemy, enemy_id, attack_foe, x, y)
			) {
				/* REMARK: enemy can still attack with less than 3 BP. */
				enemy->action_id = FIG_ACTION_RANGE_ATTACK;

				/* adjust BP */
				enemy->bp = enemy->bp - 3;

				if (enemy->bp < 0) {
					enemy->bp = 0;
				}
				return;
			}

			enemy->target_object_id = 0;
			viewdir = enemy->viewdir;
			cnt = 0;
			while (!enemy->target_object_id && (cnt < 4)) {

				if (FIG_enemy_can_attack_neighbour(x, y, diff.offset[viewdir].x, diff.offset[viewdir].y, attack_foe)) {

					/* flag: a double-size enemy may only attack in this direction if the square behind it
					 * (where its tail goes) is free, its own tail, or holds something dead */
					flag = 1;

					if (is_in_byte_array(enemy->actor_sprite_id, g_double_size_actor_sprite_id_table)) {

						rear_object_id = get_cb_val(x - diff.offset[viewdir].x, y - diff.offset[viewdir].y);

						if (rear_object_id && (enemy_id + 30 != rear_object_id)) {

							if (
								(rear_object_id < 0) || (rear_object_id >= 50) || (rear_object_id >= 30)
								// (rear_object_id >= 50) is redundant and could be removed

								|| (
									(rear_object_id > 0) && (rear_object_id < 10)
									&& !get_hero(rear_object_id - 1)->flags.dead
								) || (
									(rear_object_id < 30) && (rear_object_id >= 10)
									&& !((struct enemy_flags*)&g_enemy_sheets[rear_object_id - 10].flags)->dead)
								)
							{
								flag = 0;
							}
						}
					}

					if (flag != 0) {
						enemy->target_object_id = get_cb_val(x + diff.offset[viewdir].x, y + diff.offset[viewdir].y);
					}
				}

				cnt++;
				if (++viewdir == FIG_VIEWDIR__END) {
					viewdir = FIG_VIEWDIR__BEGIN;
				}
			}
		}

		if (enemy->target_object_id != 0) {

			enemy->action_id = 2; /* FIG_ACTION_MELEE_ATTACK */
			enemy->bp = enemy->bp - 3;
			done = 1;

		} else if (enemy->bp > 0) {

			if (!enemy->flags.tied) {

				if (enemy->flags.scared) {
					target_reachable = FIG_find_path_to_target((uint8_t*)enemy, enemy_id, x, y, 4);
					enemy->bp = 0;
				} else if (enemy->flags.renegade) {
					target_reachable = FIG_find_path_to_target((uint8_t*)enemy, enemy_id, x, y, 2);
				} else {
					target_reachable = FIG_find_path_to_target((uint8_t*)enemy, enemy_id, x, y, 0);
				}

				if (target_reachable != -1) {

					x_bak = x;
					y_bak = y;
					prepare_enemy_ani(enemy, enemy_id);
					FIG_search_obj_on_cb(enemy_id + 10, &x, &y);

					if ((x_bak == x) && (y_bak == y)) {
						enemy->bp = 0;
					}

					if (enemy->bp < 3) {
						enemy->bp = 0;
					}

				} else {
					enemy->bp = 0;
				}
			} else {
				enemy->bp = 0;
			}

			enemy->action_id = 1; /* FIG_ACTION_MOVE */
		}
	}
}
