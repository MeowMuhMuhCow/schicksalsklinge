/*
 * seg044 - Kampfanimation: Figuren
 *
 * Inhalt:    Baut die Animationsspuren (g_fig_ani_tracks) fuer Angriff, Parade, Gegenstand,
 *            Zauber und Tod von Helden und Gegnern: Drehung zur Gegenseite, Grundanimation
 *            aus ANI.DAT, Waffenanimation aus WEAPANI.DAT, bei doppelt grossen Gegnern eine
 *            Kopie fuer das Schwanzteil. Merkt sich in g_fig_hero_parry_action_used /
 *            g_fig_enemy_parry_action_used, wer in dieser Runde schon pariert hat.
 * Aufrufer:  FIG_do_hero_action() (seg042), FIG_do_enemy_action() und FIG_use_item()
 *            (seg043).
 * Besonders: Die Parameter von FANI_prepare_spell_enemy() waren vertauscht benannt (zuerst
 *            kommt die animierte Figur selbst, dann die Gegenseite); "max_range" ist in
 *            Wahrheit die Art der Animation (4 = zaubern, 99 = getroffen werden).
 *            Kommentare zu FIG_ACTION_USE_HERB_POTION/4 berichtigt (Gegenstand, nicht Zauber).
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg044.c
 */
#include <string.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg006.h"
#include "seg007.h"
#include "seg036.h"
#include "seg038.h"

/**
 * \brief   copies an animation sequence
 *
 * \param   dst         where to copy the sequence
 * \param   ani_num     the number of the animation sequence
 * \param   type        type of animation (3 = WEAPANI.DAT else ANI.DAT)
 * \return              the number of copied bytes
 */
/* Borlandified and identical */
static signed int copy_ani_seq(int8_t *dst, const signed int ani_num, const signed int type)
{
	int8_t *p_datbuffer;
	int8_t *p_datitem;
	signed int ani_max_num;
	signed int i;
	signed char len;

	/* get pointer from ANI.DAT */
	p_datbuffer = (int8_t*)g_buffer_anidat;

	/* check if we must use WEAPANI.DAT */
	if (type == 3)
		p_datbuffer = (int8_t*)g_buffer_weapanidat;

	/* get number of animation sequences */
	ani_max_num = *(int16_t*)p_datbuffer;

	/* sanity check */
	if (ani_num < 0)
		return 0;

#if FIX_0294
	/* Fix 294: gueltig sind 0..ani_max_num-1 */
	if (ani_num >= ani_max_num)
#else
	if (ani_num > ani_max_num)
#endif
		return 0;

	/* set p_datitem to the first sequence */
	p_datitem = p_datbuffer;
	p_datitem += ani_max_num + 2;

	/* set length to the length of the first sequence */
	len = p_datbuffer[2];

	/* fast forward to the requestet sequence */
	for (i = 1; i <= ani_num; i++) {
		p_datitem += len;
		len = *(p_datbuffer + i + 2);
	}

	/* skip the first byte */
	p_datitem++;
	/* calc the length of the sequence */
	len -= 2;
	/* REMARK: the first and the last byte of the sequence are skipped */

	/* copy them */
	for (i = 0; i < len; i++)
		*dst++ = *p_datitem++;

	return len;
}

/**
 * \brief   returns the header byte (first byte) of an animation sequence in ANI.DAT
 *
 * \param[in]  ani_num the number of the animation sequence
 * \return  the first byte of the sequence from ANI.DAT {0,1,2,3,4}
 */
/* Borlandified and identical */
static signed char get_seq_header(const signed int ani_num)
{
	int8_t *p_datbuffer;
	int8_t *p_datitem;
	int8_t length;
	signed int ani_max_num;
	signed int i;

	/* get pointer from ANI.DAT */
	p_datbuffer = (int8_t*)g_buffer_anidat;

	/* get number of ani seqences in ANI.DAT */
	ani_max_num = *(int16_t*)p_datbuffer;

	if (ani_num < 0) {
		return 0;
	}

#if FIX_0294
	/* Fix 294: gueltig sind 0..ani_max_num-1 */
	if (ani_num >= ani_max_num) {
#else
	if (ani_num > ani_max_num) {
#endif
		return 0;
	}

	p_datitem = p_datbuffer;
	p_datitem += ani_max_num + 2;

	length = p_datbuffer[2];

	for (i = 1; i <= ani_num; i++) {
		/* set pointer to the start of the next sequence */
		p_datitem += length;
		/* set length to the length of the next sequence */
		length = *(p_datbuffer + i + 2);
	}

	return *p_datitem;
}

/**
 * \brief   prepares the animation sequence of a hero in fights
 *
 * \param   ani_track_id       [0, 1]
 * \param   hero            pointer to hero
 * \param   weapon_gfx_id   the type of weapon for the animation [-1, 5], 3,4,5 are range weapons
 * \param   f_action        {FIG_ACTION_NONE = 0, FIG_ACTION_MELEE_ATTACK = 2, FIG_ACTION_RANGE_ATTACK = 15, FIG_ACTION_PARRY = 100,
 *                          FIG_ACTION_USE_HERB_POTION = 102 (use herb/potion), FIG_ACTION_USE_OTHER_ITEM = 103 (use other item)}
 * \param   actor_object_id     object id of the animated hero himself
 * \param   opponent_object_id  object id of the other side (0 = none, keep viewdir)
 * \param   is_defender     0 = the hero is the active side, 1 = the hero is attacked
 */
/* Borlandified and identical */
void FANI_prepare_fight_hero_ani(const signed int ani_track_id, struct struct_hero *hero, const signed int weapon_gfx_id, const signed int f_action, const signed int actor_object_id, const signed int opponent_object_id, const signed int is_defender)
{
	signed int ani_index_ptr_index_0;
	signed int attacker_x;
	signed int attacker_y;
	signed int target_x;
	signed int target_y;
	signed int viewdir_of_action;
	signed int ani_index_ptr_index_1;
	signed int ani_index_ptr_index_2;
	signed int viewdir_iter;
	signed int i;
	int8_t *p_ani_clip_base;
	int8_t *p_ani_clip_weapon;
	signed int weapon_id;
	int16_t *ani_index_ptr;

	ani_index_ptr = g_gfx_ani_index[hero->actor_sprite_id];
	weapon_id = hero->inventory[HERO_INVENTORY_SLOT_RIGHT_HAND].item_id;

	if ((signed char)opponent_object_id) {

		FIG_search_obj_on_cb((signed char)opponent_object_id, &target_x, &target_y);
		FIG_search_obj_on_cb((signed char)actor_object_id, &attacker_x, &attacker_y);

		if (attacker_x == target_x) {
			if (target_y < attacker_y) {
				viewdir_of_action = FIG_VIEWDIR_DOWN;
			} else {
				viewdir_of_action = FIG_VIEWDIR_UP;
			}
		} else {
			if (target_x < attacker_x) {
				viewdir_of_action = FIG_VIEWDIR_LEFT;
			} else {
				viewdir_of_action = FIG_VIEWDIR_RIGHT;
			}
		}
	} else {
		viewdir_of_action = hero->viewdir;
	}

	if (
		(weapon_gfx_id == WEAPON_GFX_ID_NONE)
		|| ((hero->typus == HERO_TYPE_MAGIER) && (weapon_id == ITEM_ID_ZAUBERSTAB))
	) {

		ani_index_ptr_index_0 = (f_action == FIG_ACTION_MELEE_ATTACK) ? 45 :		/* melee attack */
			(f_action == FIG_ACTION_USE_HERB_POTION) ? 41 :		/* use herb/potion (FIG_use_item) */
			(f_action == FIG_ACTION_USE_OTHER_ITEM) ? 53 :		/* use other item (FIG_use_item) */
			49;

	} else {
		ani_index_ptr_index_0 = (f_action == FIG_ACTION_MELEE_ATTACK) ?  21:		/* melee attack */
			(f_action == FIG_ACTION_USE_HERB_POTION) ? 41 :		/* use herb/potion (FIG_use_item) */
			(f_action == FIG_ACTION_USE_OTHER_ITEM) ? 53 :		/* use other item (FIG_use_item) */
			(f_action != FIG_ACTION_RANGE_ATTACK) ? 25 :
			(weapon_gfx_id == WEAPON_GFX_ID_RANGED_MISSILE) ? 33 :
			(weapon_gfx_id == WEAPON_GFX_ID_RANGED_THROW) ? 57 :
			61;
	}

	ani_index_ptr_index_0 += viewdir_of_action;
	p_ani_clip_base = &g_fig_ani_tracks[ani_track_id][1];
	p_ani_clip_weapon = &g_fig_ani_tracks[FANI_TRACK_ID_BASE_TO_WEAPON(ani_track_id)][1];

	g_fig_ani_tracks[ani_track_id][0] = get_seq_header(ani_index_ptr[ani_index_ptr_index_0]);
	g_fig_ani_tracks[ani_track_id][242] = hero->actor_sprite_id;

	/* Concerning the following code block: There are parallel code blocks in
	 * prepare_enemy_ani(...) in seg037.c
	 * FIG_prepare_hero_ani(...) in seg036.c
	 * FANI_prepare_fight_hero_ani(...) in seg044.c
	 * FANI_prepare_fight_enemy_ani(...) in seg044.c
	 * FANI_prepare_spell_hero(...) in seg044.c
	 * FANI_prepare_spell_enemy(...) in seg044.c
	 */
	if (
		check_hero(hero)
		&& (hero->viewdir != viewdir_of_action)
		&& (
			(f_action == FIG_ACTION_MELEE_ATTACK)
			|| (f_action == FIG_ACTION_RANGE_ATTACK)
			|| (f_action == FIG_ACTION_USE_OTHER_ITEM)
			|| ((f_action == FIG_ACTION_PARRY) && !g_fig_hero_parry_action_used[(signed char)actor_object_id - 1])
			|| ((g_fig_critical_fail_backfire_2 != 0) && (is_defender == 0))
			|| ((g_fig_critical_fail_backfire_1 != 0) && (is_defender == 1))
		)
	) {
			g_fig_ani_tracks[ani_track_id][0] = 0;

			ani_index_ptr_index_2 = ani_index_ptr_index_1 = FIG_VIEWDIR__NONE;
			viewdir_iter = hero->viewdir;
			ani_index_ptr_index_2 = viewdir_iter;

			viewdir_iter++;
			if (viewdir_iter == FIG_VIEWDIR__END) {
				viewdir_iter = FIG_VIEWDIR__BEGIN;
			}

			if (viewdir_iter != viewdir_of_action) {
				ani_index_ptr_index_1 = viewdir_iter;

				viewdir_iter++;
				if (viewdir_iter == FIG_VIEWDIR__END) {
					viewdir_iter = FIG_VIEWDIR__BEGIN;
				}

				if (viewdir_iter != viewdir_of_action) {
					ani_index_ptr_index_2 = hero->viewdir + 4;
					ani_index_ptr_index_1 = FIG_VIEWDIR__NONE;
				}
			}

			hero->viewdir = viewdir_of_action;

			if (ani_index_ptr_index_1 == FIG_VIEWDIR__NONE) {
				for (i = 0; i < 2; i++) {
					*p_ani_clip_base++ = -5;
					*p_ani_clip_base++ = 0;
					*p_ani_clip_base++ = 0;
				}
			}

			p_ani_clip_base += copy_ani_seq(p_ani_clip_base, ani_index_ptr[ani_index_ptr_index_2], ANI_SRC_FILE_ID_ANI_DAT);

			if (ani_index_ptr_index_1 != FIG_VIEWDIR__NONE) {
				p_ani_clip_base += copy_ani_seq(p_ani_clip_base, ani_index_ptr[ani_index_ptr_index_1], ANI_SRC_FILE_ID_ANI_DAT);
			}

			*p_ani_clip_base++ = -4;
			*p_ani_clip_base++ = get_seq_header(ani_index_ptr[ani_index_ptr_index_0]);
			*p_ani_clip_base++ = 0;
	} else {
		for (i = 0; i < 5; i++) {
			*p_ani_clip_base++ = -5;
			*p_ani_clip_base++ = 0;
			*p_ani_clip_base++ = 0;
		}
	}

	if (
		(
			check_hero(hero)
			&& (f_action == FIG_ACTION_MELEE_ATTACK)
		) || (
			(f_action == FIG_ACTION_RANGE_ATTACK)
			|| (f_action == FIG_ACTION_USE_HERB_POTION)
			|| (f_action == FIG_ACTION_USE_OTHER_ITEM)
			|| ((f_action == FIG_ACTION_PARRY) && !g_fig_hero_parry_action_used[(signed char)actor_object_id - 1])
		)
	) {
		p_ani_clip_base += copy_ani_seq(p_ani_clip_base, ani_index_ptr[ani_index_ptr_index_0], ANI_SRC_FILE_ID_ANI_DAT);

		if (
			(weapon_gfx_id != WEAPON_GFX_ID_NONE)
			&& (weapon_gfx_id < WEAPON_GFX_ID_RANGED_MISSILE)
			&& (hero->typus != HERO_TYPE_MAGIER)
			&& (hero->typus != HERO_TYPE_DRUIDE)
		) {
			for (i = 0; i < 5; i++) {
				*p_ani_clip_weapon++ = -5;
				*p_ani_clip_weapon++ = 0;
				*p_ani_clip_weapon++ = 0;
			}

			p_ani_clip_weapon += copy_ani_seq(p_ani_clip_weapon,
				*(int16_t*)((uint8_t*)g_weaponani_table + (g_weaponani_types[hero->actor_sprite_id] * 48 + weapon_gfx_id * 16) +
				((f_action == FIG_ACTION_MELEE_ATTACK) ? 0 : 1) * 8 + hero->viewdir * 2), ANI_SRC_FILE_ID_WEAPANI_DAT);
		}
	}

	if (
		(
			check_hero(hero)
			&& g_fig_critical_fail_backfire_2
			&& !is_defender
		) || (
			g_fig_critical_fail_backfire_1
			&& (is_defender == 1)
		)
	) {

			p_ani_clip_base += copy_ani_seq(p_ani_clip_base, ani_index_ptr[ani_index_ptr_index_0], ANI_SRC_FILE_ID_ANI_DAT);

			if (
				(weapon_gfx_id != WEAPON_GFX_ID_NONE)
				&& (weapon_gfx_id < WEAPON_GFX_ID_RANGED_MISSILE)
				&& (hero->typus != HERO_TYPE_MAGIER)
				&& (hero->typus != HERO_TYPE_DRUIDE)
			) {
				p_ani_clip_weapon += copy_ani_seq(p_ani_clip_weapon,
					*(int16_t*)((uint8_t*)g_weaponani_table + (g_weaponani_types[hero->actor_sprite_id] * 48 + weapon_gfx_id * 16) +
					((f_action == FIG_ACTION_MELEE_ATTACK) ? 0 : 1) * 8 + hero->viewdir * 2), 3);
			}
	}

	if (
		(g_fig_attacker_dead && !is_defender)
		|| (g_fig_target_dead && (is_defender == 1))
	) {
		*p_ani_clip_base++ = -4;
		*p_ani_clip_base++ = get_seq_header(ani_index_ptr[20]);
		*p_ani_clip_base++ = 0;

		p_ani_clip_base += copy_ani_seq(p_ani_clip_base, ani_index_ptr[20], ANI_SRC_FILE_ID_ANI_DAT);
	}

	if (
		check_hero(hero)
		|| (g_fig_attacker_dead && !is_defender)
		|| (g_fig_target_dead && (is_defender == 1))
	) {
		FIG_set_ani_track_id_base(hero->fighter_id, (signed char)ani_track_id);
		*p_ani_clip_base = -1;

		if (
			(weapon_gfx_id != WEAPON_GFX_ID_NONE)
			&& (weapon_gfx_id < WEAPON_GFX_ID_RANGED_MISSILE)
			&& (hero->typus != HERO_TYPE_MAGIER)
			&& (hero->typus != HERO_TYPE_DRUIDE)
		) {
			FIG_set_ani_track_id_weapon(hero->fighter_id, FANI_TRACK_ID_BASE_TO_WEAPON(ani_track_id));

			*p_ani_clip_weapon = -1;
		}
	}

	*p_ani_clip_base = -1;
	if (f_action == FIG_ACTION_PARRY) {
		g_fig_hero_parry_action_used[(signed char)actor_object_id - 1] = 1;
	}
}

/**
 * \brief   prepares the animation sequence of an enemy in fights
 *
 * \param   ani_track_id        [0, 1]
 * \param   enemy               pointer to the enemy sheet
 * \param   f_action            {FIG_ACTION_NONE, FIG_ACTION_MELEE_ATTACK, FIG_ACTION_RANGE_ATTACK, FIG_ACTION_PARRY}
 * \param   actor_object_id     object id of the animated enemy itself
 * \param   opponent_object_id  object id of the other side
 * \param   is_defender         0 = the enemy is the active side, 1 = the enemy is attacked
 */
/* Borlandified and identical */
void FANI_prepare_fight_enemy_ani(const signed int ani_track_id, struct enemy_sheet *enemy, const signed int f_action, const signed int actor_object_id, const signed int opponent_object_id, const signed int is_defender)
{
	signed int ani_index_ptr_index_0;
	signed int attacker_x;
	signed int attacker_y;
	signed int target_x;
	signed int target_y;
	signed int viewdir_of_action;
	signed int ani_index_ptr_index_1;
	signed int ani_index_ptr_index_2;
	signed int viewdir_iter;
	signed int i;
	int8_t *p_ani_clip_base;
	int8_t *p_ani_clip_weapon;
	struct struct_fighter *p_fighter;	/* only user for double-size enemies */
	int16_t *ani_index_ptr;			/* read only */
	signed int weapon_gfx_id;

	/* initialize with bare hands */
	weapon_gfx_id = WEAPON_GFX_ID_NONE;

	/* every enemy with the actor_sprite_id < 22 has a sword type weapon */
	if (enemy->actor_sprite_id < ACTOR_SPRITE_ID_ELF__FEMALE + 1) {
		weapon_gfx_id = WEAPON_GFX_ID_MELEE_BLADE;
	}

	if (enemy->weapon_broken) {
		weapon_gfx_id = WEAPON_GFX_ID_NONE;
	}

	/* read a pointer from an array with the actor_sprite_id as offset, read only */
	ani_index_ptr = g_gfx_ani_index[enemy->actor_sprite_id];

	/* find both actors on the chessboard */
	FIG_search_obj_on_cb((signed char)opponent_object_id, &target_x, &target_y);
	FIG_search_obj_on_cb((signed char)actor_object_id, &attacker_x, &attacker_y);

	/* find out which direction the action will have */
	if (attacker_x == target_x) {
		if (target_y < attacker_y) {
			viewdir_of_action = FIG_VIEWDIR_DOWN;
		} else {
			viewdir_of_action = FIG_VIEWDIR_UP;
		}
	} else {
		if (target_x < attacker_x) {
			viewdir_of_action = FIG_VIEWDIR_LEFT;
		} else {
			viewdir_of_action = FIG_VIEWDIR_RIGHT;
		}
	}

	/* melee attack */
	ani_index_ptr_index_0 = (f_action == FIG_ACTION_MELEE_ATTACK) ? 21 : 25;

	if (
		(enemy->actor_sprite_id == ACTOR_SPRITE_ID_DRUIDE__MALE)
		|| (enemy->actor_sprite_id == ACTOR_SPRITE_ID_MAGIER__MALE)
		|| (enemy->actor_sprite_id == ACTOR_SPRITE_ID_DRUIDE__FEMALE)
		|| (enemy->actor_sprite_id == ACTOR_SPRITE_ID_MAGIER__FEMALE)
	) {

		weapon_gfx_id = WEAPON_GFX_ID_NONE;
		ani_index_ptr_index_0 = (f_action == FIG_ACTION_MELEE_ATTACK) ? 45 : 49;
	}

	if (f_action == FIG_ACTION_RANGE_ATTACK) {
		ani_index_ptr_index_0 = 33;
		weapon_gfx_id = WEAPON_GFX_ID_NONE;
	}

	ani_index_ptr_index_0 += viewdir_of_action;

	p_ani_clip_base = &g_fig_ani_tracks[ani_track_id][1];
	p_ani_clip_weapon = &g_fig_ani_tracks[FANI_TRACK_ID_BASE_TO_WEAPON(ani_track_id)][1];


	g_fig_ani_tracks[ani_track_id][0] = get_seq_header(ani_index_ptr[ani_index_ptr_index_0]);
	g_fig_ani_tracks[ani_track_id][242] = enemy->actor_sprite_id;

	/* Concerning the following code block: There are parallel code blocks in
	 * prepare_enemy_ani(...) in seg037.c
	 * FIG_prepare_hero_ani(...) in seg036.c
	 * FANI_prepare_fight_hero_ani(...) in seg044.c
	 * FANI_prepare_fight_enemy_ani(...) in seg044.c
	 */

	/* first the enemy may turn */
	if (
		(enemy->viewdir != viewdir_of_action)
		&& (
			(
				 (f_action == FIG_ACTION_MELEE_ATTACK)
				 || (f_action == FIG_ACTION_RANGE_ATTACK)
				 || ((f_action == FIG_ACTION_PARRY) && !g_fig_enemy_parry_action_used[(signed char)actor_object_id])
			)
			|| (g_fig_critical_fail_backfire_2 && !is_defender)
			|| (g_fig_critical_fail_backfire_1 && (is_defender == 1))
		)
	) {

		g_fig_ani_tracks[ani_track_id][0] = 0;

		/* find out the new direction */
		ani_index_ptr_index_2 = ani_index_ptr_index_1 = FIG_VIEWDIR__NONE;

		viewdir_iter = enemy->viewdir;
		ani_index_ptr_index_2 = viewdir_iter;

		/* turn right by 90 degrees */
		viewdir_iter++;
		if (viewdir_iter == FIG_VIEWDIR__END) {
			viewdir_iter = FIG_VIEWDIR__BEGIN;
		}

		if (viewdir_iter != viewdir_of_action) {
			/* still not the same direction */
			ani_index_ptr_index_1 = viewdir_iter; // misleading, setting ani_index_ptr_index_1 to any value != FIG_VIEWDIR__NONE would do.

			/* turn right by 90 degrees for the second time */
			viewdir_iter++;
			if (viewdir_iter == FIG_VIEWDIR__END) {
				viewdir_iter = FIG_VIEWDIR__BEGIN;
			}

			if (viewdir_iter != viewdir_of_action) {
				/* still not the same direction */
				/* So: correct direction is turning left by 90 degrees from the starting position. */
				ani_index_ptr_index_2 = enemy->viewdir + 4;
				ani_index_ptr_index_1 = FIG_VIEWDIR__NONE;
			}
		}

		/* outcome:                   ani_index_ptr_index_1                             ani_index_ptr_index_2
		 * turn right by 90 degrees   FIG_VIEWDIR__NONE                                 initial viewdir
		 * turn by 180 degrees        initial viewdir, turned 90 degrees to the right   initial viewdir
		 * turn left by 90 degrees    FIG_VIEWDIR__NONE                                 (initial viewdir) + 4
		 */

		/* set the new direction in enemy sheet */
		enemy->viewdir = viewdir_of_action;

		if (ani_index_ptr_index_1 == FIG_VIEWDIR__NONE) {
			/* attacking enemy turns by 90 degrees (left or right) */
			/* do not move for 2 frames */
			for (i = 0; i < 2; i++) {
				*p_ani_clip_base++ = -5;
				*p_ani_clip_base++ = 0;
				*p_ani_clip_base++ = 0;
			}
		}

		p_ani_clip_base += copy_ani_seq(p_ani_clip_base, ani_index_ptr[ani_index_ptr_index_2], 1);

		if (ani_index_ptr_index_1 != FIG_VIEWDIR__NONE) {
			/* attacking enemy turned by 180 degrees */
			/* show sprite turned 90 degrees to the right (intermediate turning state) */
			p_ani_clip_base += copy_ani_seq(p_ani_clip_base, ani_index_ptr[ani_index_ptr_index_1], 1);
		}

		*p_ani_clip_base++ = -4;
		*p_ani_clip_base++ = get_seq_header(ani_index_ptr[ani_index_ptr_index_0]);
		*p_ani_clip_base++ = 0;
	} else {
		/* attacking enemy is already heading toward the target and does not turn. */
		for (i = 0; i < 5; i++) {
			/* do not move for 5 frames */
			*p_ani_clip_base++ = -5;
			*p_ani_clip_base++ = 0;
			*p_ani_clip_base++ = 0;
		}
	}

	if (
		(f_action == FIG_ACTION_MELEE_ATTACK)
		|| (f_action == FIG_ACTION_RANGE_ATTACK)
		|| ((f_action == FIG_ACTION_PARRY) && !g_fig_enemy_parry_action_used[(signed char)actor_object_id])
	) {
		p_ani_clip_base += copy_ani_seq(p_ani_clip_base, ani_index_ptr[ani_index_ptr_index_0], 1);

		if (weapon_gfx_id != WEAPON_GFX_ID_NONE) {

			/* do not move for 5 frames */
			for (i = 0; i < 5; i++) {
				*p_ani_clip_weapon++ = -5;
				*p_ani_clip_weapon++ = 0;
				*p_ani_clip_weapon++ = 0;
			}

			/* copy the weapon ani */
			p_ani_clip_weapon += copy_ani_seq(p_ani_clip_weapon,
				*(int16_t*)((uint8_t*)g_weaponani_table + g_weaponani_types[enemy->actor_sprite_id] * 48 + weapon_gfx_id * 16 + ((f_action == FIG_ACTION_MELEE_ATTACK) ? 0 : 1) * 8 + enemy->viewdir * 2), 3);
		}
	}

	if (
		((g_fig_critical_fail_backfire_2 != 0) && (is_defender == 0))
		|| ((g_fig_critical_fail_backfire_1 != 0) && (is_defender == 1))
	) {

			p_ani_clip_base += copy_ani_seq(p_ani_clip_base, ani_index_ptr[ani_index_ptr_index_0], 1);

			if (weapon_gfx_id != WEAPON_GFX_ID_NONE) {

				/* copy the weapon ani */
				p_ani_clip_weapon += copy_ani_seq(p_ani_clip_weapon,
					*(int16_t*)((uint8_t*)g_weaponani_table + g_weaponani_types[enemy->actor_sprite_id] * 48 + weapon_gfx_id * 16 + ((f_action == FIG_ACTION_MELEE_ATTACK) ? 0 : 1) * 8 + enemy->viewdir * 2), 3);
			}
	}

	if (
		((g_fig_attacker_dead != 0) && (is_defender == 0))
		|| ((g_fig_target_dead != 0) && (is_defender == 1))
	) {
		*p_ani_clip_base++ = -4;
		*p_ani_clip_base++ = get_seq_header(ani_index_ptr[20]);
		*p_ani_clip_base++ = 0;

		p_ani_clip_base += copy_ani_seq(p_ani_clip_base, ani_index_ptr[20], 1);
	}

	FIG_set_ani_track_id_base(enemy->fighter_id, (signed char)ani_track_id);

	/* terminate figure animation array */
	*p_ani_clip_base = -1;

	/* does this sprite need two squares? */
	if (is_in_byte_array(enemy->actor_sprite_id, g_double_size_actor_sprite_id_table))	{

		memcpy(&g_fig_ani_tracks[FANI_TRACK_ID_BASE_TO_TAIL(ani_track_id)], &g_fig_ani_tracks[ani_track_id], 243);

		p_fighter = FIG_get_fighter(enemy->fighter_id);

		FIG_set_ani_track_id_base(g_fig_double_size_fighter_id_table[p_fighter->double_size], FANI_TRACK_ID_BASE_TO_TAIL(ani_track_id));
	}

	if (weapon_gfx_id != WEAPON_GFX_ID_NONE) {

		FIG_set_ani_track_id_weapon(enemy->fighter_id, FANI_TRACK_ID_BASE_TO_WEAPON(ani_track_id));

		/* terminate weapon animation array */
		*p_ani_clip_weapon = -1;
	}

	if (f_action == FIG_ACTION_PARRY) {
		g_fig_enemy_parry_action_used[(signed char)actor_object_id] = 1;
	}
}

/**
 * \brief   prepares the animation of a hero casting a spell or being hit by a spell
 *
 * \param[in] ani_track_id  0 or 1
 * \param   hero        pointer to a hero
 * \param   ani_mode    4 (FIG_ACTION_SPELL) = casting, the hero turns to the target;
 *                      99 = the hero is hit by a spell (former name: max_range)
 * \param   actor_object_id     object id of the animated hero himself
 * \param   opponent_object_id  object id of the other side (0 when hit)
 * \param   projectile_gfx_id   casting: PROJECTILE_GFX_ID_SPELLCAST_BOLT (1) selects another clip; -1 when hit
 * \param   is_target   0 = the hero casts, 1 = the hero is hit
 */

/* Borlandified and identical */
void FANI_prepare_spell_hero(const signed int ani_track_id, struct struct_hero *hero, const signed int ani_mode, const signed int actor_object_id, const signed int opponent_object_id, const signed int projectile_gfx_id, const signed int is_target)
{
	signed int x_actor;
	signed int y_actor;
	signed int x_opponent;
	signed int y_opponent;
	signed int viewdir_of_action;
	signed int ani_index_ptr_index_1;
	signed int ani_index_ptr_index_2;
	int8_t *p_ani_clip_base;
	int16_t *ani_index_ptr;
	signed int ani_index_ptr_index_0;

	/* get a pointer from an array where the Monster-ID serves as index */
	ani_index_ptr = g_gfx_ani_index[hero->actor_sprite_id];

	FIG_search_obj_on_cb((signed char)opponent_object_id, &x_opponent, &y_opponent);
	FIG_search_obj_on_cb((signed char)actor_object_id, &x_actor, &y_actor);

	if (x_actor == x_opponent) {
		if (y_opponent < y_actor)
			viewdir_of_action = FIG_VIEWDIR_DOWN;
		else
			viewdir_of_action = FIG_VIEWDIR_UP;
	} else {
		if (x_opponent < x_actor)
			viewdir_of_action = FIG_VIEWDIR_LEFT;
		else
			viewdir_of_action = FIG_VIEWDIR_RIGHT;
	}

	if ((signed char)opponent_object_id == (signed char)actor_object_id)
		viewdir_of_action = hero->viewdir;


	ani_index_ptr_index_0 = (ani_mode == 4) ? ((projectile_gfx_id == 1) ? 37 : 29) : 16;

	ani_index_ptr_index_0 += (ani_mode == 4) ? viewdir_of_action : hero->viewdir;

	p_ani_clip_base = &g_fig_ani_tracks[ani_track_id][1];

	g_fig_ani_tracks[ani_track_id][0] = get_seq_header(ani_index_ptr[ani_index_ptr_index_0]);

	g_fig_ani_tracks[ani_track_id][242] = hero->actor_sprite_id;

	/* Concerning the following code block: There are parallel code blocks in
	 * prepare_enemy_ani(...) in seg037.c
	 * FIG_prepare_hero_ani(...) in seg036.c
	 * FANI_prepare_fight_hero_ani(...) in seg044.c
	 * FANI_prepare_fight_enemy_ani(...) in seg044.c
	 * FANI_prepare_spell_hero(...) in seg044.c
	 * FANI_prepare_spell_enemy(...) in seg044.c
	 */
	if ((hero->viewdir != viewdir_of_action) && (ani_mode == 4)) {

		signed int viewdir_iter;

		g_fig_ani_tracks[ani_track_id][0] = 0;

		ani_index_ptr_index_2 = ani_index_ptr_index_1 = FIG_VIEWDIR__NONE;
		viewdir_iter = hero->viewdir;
		ani_index_ptr_index_2 = viewdir_iter;

		viewdir_iter++;
		if (viewdir_iter == FIG_VIEWDIR__END) {
			viewdir_iter = FIG_VIEWDIR__BEGIN;
		}

		if (viewdir_iter != viewdir_of_action) {
			ani_index_ptr_index_1 = viewdir_iter;

			viewdir_iter++;
			if (viewdir_iter == FIG_VIEWDIR__END) {
				viewdir_iter = FIG_VIEWDIR__BEGIN;
			}

			if (viewdir_iter != viewdir_of_action) {
				ani_index_ptr_index_2 = hero->viewdir + 4;
				ani_index_ptr_index_1 = FIG_VIEWDIR__NONE;
			}
		}

		hero->viewdir = viewdir_of_action;
		p_ani_clip_base += copy_ani_seq(p_ani_clip_base, ani_index_ptr[ani_index_ptr_index_2], ANI_SRC_FILE_ID_ANI_DAT);

		if (ani_index_ptr_index_1 != FIG_VIEWDIR__NONE) {
			p_ani_clip_base += copy_ani_seq(p_ani_clip_base, ani_index_ptr[ani_index_ptr_index_1], ANI_SRC_FILE_ID_ANI_DAT);
		}

		*p_ani_clip_base = -4;
		p_ani_clip_base++;

		*p_ani_clip_base = get_seq_header(ani_index_ptr[ani_index_ptr_index_0]);
		p_ani_clip_base++;

		*p_ani_clip_base = 0x00;
		p_ani_clip_base++;
	}

	if (
		(ani_mode == 4)
		|| check_hero(hero)
		|| ((g_fig_attacker_dead != 0) && (is_target == 0))
		|| ((g_fig_target_dead != 0) && (is_target == 1))
	) {

		p_ani_clip_base += copy_ani_seq(p_ani_clip_base, ani_index_ptr[ani_index_ptr_index_0], ANI_SRC_FILE_ID_ANI_DAT);
	}

	if (
		((g_fig_attacker_dead != 0) && (is_target == 0))
		|| ((g_fig_target_dead != 0) && (is_target == 1))
	) {

		*p_ani_clip_base = -4;
		p_ani_clip_base++;

		*p_ani_clip_base = get_seq_header(ani_index_ptr[20]);
		p_ani_clip_base++;

		*p_ani_clip_base = 0;
		p_ani_clip_base++;

		p_ani_clip_base += copy_ani_seq(p_ani_clip_base, ani_index_ptr[20], ANI_SRC_FILE_ID_ANI_DAT);
	}

	*p_ani_clip_base = -1;
}

/**
 * \brief   prepares a spell animation
 *
 *          of an enemy casting a spell (FIG_do_enemy_action) or being hit by a spell
 *          (FIG_do_hero_action, FIG_do_enemy_action); not only for "Blitz", "Fulminictus", "Ignifaxius"
 *
 * \param[in] ani_track_id  0 or 1
 * \param   enemy       pointer to an entry of g_enemy_sheets
 * \param   ani_mode    4 (FIG_ACTION_SPELL) = casting, the enemy turns to the target;
 *                      99 = the enemy is hit (former name: max_range)
 * \param   actor_object_id     object id of the animated enemy itself
 *                              (former name target_object_id: the parameter names were swapped)
 * \param   opponent_object_id  object id of the other side (former name: caster_object_id)
 * \param   is_target   0 = the enemy casts, 1 = the enemy is hit
 */
/* Borlandified and identical */
void FANI_prepare_spell_enemy(const signed int ani_track_id, struct enemy_sheet *enemy, const signed int ani_mode, const signed int actor_object_id, const signed int opponent_object_id, const signed int is_target)
{
	signed int ani_index_ptr_index_0;
	signed int x_actor;
	signed int y_actor;
	signed int x_opponent;
	signed int y_opponent;
	signed int viewdir_of_action;
	signed int ani_index_ptr_index_1;
	signed int ani_index_ptr_index_2;		/* indicees to ani_index_ptr */
	int8_t *p_ani_clip_base;	/* mostly written */
	int16_t *ani_index_ptr;	/* read only */
	signed int viewdir_iter;


	/* get a pointer from an array where the actor_sprite_id of the enemy serves as index */
	ani_index_ptr = g_gfx_ani_index[enemy->actor_sprite_id];

	FIG_search_obj_on_cb((signed char)opponent_object_id, &x_opponent, &y_opponent);
	FIG_search_obj_on_cb((signed char)actor_object_id, &x_actor, &y_actor);

	if (x_actor == x_opponent) {
		if (y_opponent < y_actor)
			viewdir_of_action = FIG_VIEWDIR_DOWN;
		else
			viewdir_of_action = FIG_VIEWDIR_UP;
	} else {
		if (x_opponent < x_actor)
			viewdir_of_action = FIG_VIEWDIR_LEFT;
		else
			viewdir_of_action = FIG_VIEWDIR_RIGHT;
	}

	if ((signed char)opponent_object_id == (signed char)actor_object_id)
		viewdir_of_action = enemy->viewdir;

	/* ani_mode 4: the enemy casts a spell (former comment: "this is true if an enemy attacks a hero") */
	ani_index_ptr_index_0 = (ani_mode == 4) ? 29 : 16;

	p_ani_clip_base = &g_fig_ani_tracks[ani_track_id][1];

	/* ani_mode 4: the enemy casts a spell (former comment: "this is true if an enemy attacks a hero") */
	ani_index_ptr_index_0 += (ani_mode == 4) ? viewdir_of_action : enemy->viewdir;

	g_fig_ani_tracks[ani_track_id][0] = get_seq_header(ani_index_ptr[ani_index_ptr_index_0]);

	g_fig_ani_tracks[ani_track_id][242] = enemy->actor_sprite_id;

	/* Concerning the following code block: There are parallel code blocks in
	 * prepare_enemy_ani(...) in seg037.c
	 * FIG_prepare_hero_ani(...) in seg036.c
	 * FANI_prepare_fight_hero_ani(...) in seg044.c
	 * FANI_prepare_fight_enemy_ani(...) in seg044.c
	 * FANI_prepare_spell_hero(...) in seg044.c
	 * FANI_prepare_spell_enemy(...) in seg044.c
	 */
	if ((enemy->viewdir != viewdir_of_action) && (ani_mode == 4)) {

		g_fig_ani_tracks[ani_track_id][0] = 0;

		ani_index_ptr_index_2 = ani_index_ptr_index_1 = FIG_VIEWDIR__NONE;

		viewdir_iter = enemy->viewdir;
		ani_index_ptr_index_2 = viewdir_iter;

		viewdir_iter++;
		if (viewdir_iter == FIG_VIEWDIR__END) {
			viewdir_iter = FIG_VIEWDIR__BEGIN;
		}

		if (viewdir_iter != viewdir_of_action) {

			ani_index_ptr_index_1 = viewdir_iter;

			viewdir_iter++;
			if (viewdir_iter == FIG_VIEWDIR__END) {
				viewdir_iter = FIG_VIEWDIR__BEGIN;
			}

			if (viewdir_iter != viewdir_of_action) {
				ani_index_ptr_index_2 = enemy->viewdir + 4;
				ani_index_ptr_index_1 = FIG_VIEWDIR__NONE;
			}
		}

		enemy->viewdir = viewdir_of_action;

		p_ani_clip_base += copy_ani_seq(p_ani_clip_base, ani_index_ptr[ani_index_ptr_index_2], 1);

		if (ani_index_ptr_index_1 != FIG_VIEWDIR__NONE)
			p_ani_clip_base += copy_ani_seq(p_ani_clip_base, ani_index_ptr[ani_index_ptr_index_1], 1);

		*p_ani_clip_base = -4;
		p_ani_clip_base++;

		*p_ani_clip_base = get_seq_header(ani_index_ptr[ani_index_ptr_index_0]);
		p_ani_clip_base++;

		*p_ani_clip_base = 0;
		p_ani_clip_base++;
	}

	p_ani_clip_base += copy_ani_seq(p_ani_clip_base, ani_index_ptr[ani_index_ptr_index_0], 1);

	if (
		(g_fig_attacker_dead && !is_target)
		|| (g_fig_target_dead && (is_target == 1))
	) {

		*p_ani_clip_base = -4;
		p_ani_clip_base++;

		*p_ani_clip_base = get_seq_header(ani_index_ptr[20]);
		p_ani_clip_base++;

		*p_ani_clip_base = 0;
		p_ani_clip_base++;

		p_ani_clip_base += copy_ani_seq(p_ani_clip_base, ani_index_ptr[20], 1);
	}

	*p_ani_clip_base = -1;

	/* check if the moster sprite ID needs two squares */
	if (is_in_byte_array(enemy->actor_sprite_id, g_double_size_actor_sprite_id_table)) {
		memcpy(&g_fig_ani_tracks[FANI_TRACK_ID_BASE_TO_TAIL(ani_track_id)], &g_fig_ani_tracks[ani_track_id], 243);
	}
}
