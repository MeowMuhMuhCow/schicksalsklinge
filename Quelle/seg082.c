/*
 * seg082 - Dungeon "Ruine des Schwarzmagiers" (DNG07) und Statuen der Kultstaette (DNG09)
 *
 * Inhalt:    DNG07_handler() steuert die Ruine zwischen Rybon und Thoss: verschlossene
 *            Tuer zur Treppe (Dietrich oder FORAMEN, bei Misserfolg ein Kampf), Gelaechter,
 *            Elfengeist mit MU-Elixier (MU +3 fuer die Gruppe bis zum Ausgang), falsches
 *            MU-Elixier, Endlosgang, Giftflasche (Gifttimer 1 Tag, Wirkung in seg002),
 *            Kampf um das Kartenstueck und der Ausgang. DNG09_statues() sind die Statuen
 *            des Namenlosen in der Kultstaette: beruehren (Eigenschaft +1, aber keine
 *            Wunder mehr, oder 1W6 LE) oder zerschlagen (Goetteransehen + bonus).
 * Aufrufer:  DNG07_handler() ueber g_dng_handlers (seg076), DNG09_statues() aus
 *            DNG09_handler() (seg084), die Truhe ueber g_dng07_specialchests (seg092).
 * Daten:     g_dng07_specialchests[2] (eine Truhe mit 500 D).
 * Besonders: MU-Elixier und Ausgang: Heldenschleife beginnt beim ersten verfuegbaren
 *            Helden (Bright-Eyes: "forgot to set hero"). DNG09_statues(): Schleife ueber
 *            die Goetter 0..13 statt 1..14, Ifirn bekommt keinen Bonus.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg082.c
 */
#include <stdio.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg007.h"
#include "seg025.h"
#include "seg032.h"
#include "seg047.h"
#include "seg075.h"
#include "seg076.h"
#include "seg092.h"
#include "seg097.h"
#include "seg098.h"
#include "seg103.h"
#include "seg105.h"

/* REMARK: These 500 D might result in an integer overflow. */
/* REMARK: no overflow: money is unsigned int (50000 < 65535) and loot_special_chest() adds it as int32_t. */
struct struct_chest g_dng07_specialchests[2] = {
	{ DNG_POS(2,8,5), 5, use_lockpicks_on_chest, chest_ignifax_normal, NULL, 0, 50000L, 0 },
	{     -1, 0, NULL, NULL, NULL, 0, 0, 0 }
}; // ds:0x9668

/**
 * \brief   dungeon handler of the ruin of the black mage
 */
signed int DNG07_handler(void)
{
	signed int target_pos;
	signed int i;
	signed int tw_bak;
	struct struct_hero *hero;
	uint8_t *amap_ptr;
	signed int spell_result;
	signed int lockpick_inv_slot;
	signed int talent_result;

	amap_ptr = g_dng_map;
	tw_bak = g_textbox_width;
	g_textbox_width = 7;

	target_pos = DNG_POS(gs_dungeon_level, gs_x, gs_y);

	hero = get_first_hero_available_in_group();

	if (target_pos == DNG_POS(0,13,2) && target_pos != gs_dng_pos_bak)
	{
		/* locked door in front of the stairs down: the square becomes a stair once the door is
		 * opened. A failed attempt (lockpicks or spell) calls the guards (FIGHT_ID_F100_03).
		 * In any case the group steps back. */
#if !defined(__BORLANDC__)
		if (div16((uint16_t)amap_ptr[MAP_POS(13,2)]) != DNG_TILE_STAIR_DOWN)
#else
		/* REMARK: enforce a cast to uint16_t */
		if (div16((_AL = amap_ptr[MAP_POS(13,2)], _AH = 0, _AX)) != DNG_TILE_STAIR_DOWN)
#endif
		{
			do {
				i = GUI_radio(get_tx(8), 3, get_tx(9), get_tx(10), get_tx(11)); /* "VOR EUCH BEFINDET SICH EINE V..." / "DAS SCHLOSS KNACKEN?" / "EINEN FORAMEN SPRECHEN?" / "DIE TÜR SEIN LASSEN?" */

			} while (i == -1);

			if (i == 1)
			{
				if ((lockpick_inv_slot = hero_has_lockpicks(hero)) != -1)
				{
					if (lockpick_inv_slot != -2) // !=-1 and !=-2 means: hero has at least one non-broken set of lockpicks
					{
						talent_result = test_talent(hero, TA_SCHLOESSER, 7);

						if (talent_result == -99) {

							print_msg_with_first_hero(get_ttx(533)); /* "BEI DEM VERSUCH, DAS SCHLOSS ZU KNACKEN, ZERBRECHEN %sS@DIETRICHE....." */

							hero->inventory[lockpick_inv_slot].flags.broken = 1;

							g_fig_escape_position[NORTH] = g_fig_escape_position[EAST] = g_fig_escape_position[SOUTH] = g_fig_escape_position[WEST] = target_pos;

							do_fight(FIGHT_ID_F100_03);

						} else if (talent_result <= 0) {

							print_msg_with_first_hero(get_ttx(532)); /* "LEIDER MISSLUNGEN!@%s SCHAFFT ES NICHT, DAS SCHLOSS ZU KNACKEN." */

							g_fig_escape_position[NORTH] = g_fig_escape_position[EAST] = g_fig_escape_position[SOUTH] = g_fig_escape_position[WEST] = target_pos;

							do_fight(FIGHT_ID_F100_03);

						} else {

							*(amap_ptr + MAP_POS(13,2)) &= 0x0f;
							*(amap_ptr + MAP_POS(13,2)) |= DNG_TILE_STAIR_DOWN << 4;

							add_hero_ap(hero, 1L);
						}

					} else {
						// all lockpicks are broken
						print_msg_with_first_hero(get_ttx(531)); /* "%sS DIETRICHE SIND LEIDER ZERBROCHEN UND SOMIT NUTZLOS!" */
					}
				} else {
					// hero does not have lockpicks
					print_msg_with_first_hero(get_ttx(530)); /* "%s HAT KEINEN DIETRICH, UM DAS SCHLOSS ZU KNACKEN!" */
				}

			} else if (i == 2)
			{

				hero = get_hero(select_hero_ok_forced(get_ttx(317))); /* "WELCHER HELD SOLL MAGIE ANWENDEN?" */

				if (hero->typus < HERO_TYPE_HEXE)
				{
					GUI_output(get_ttx(330)); /* "DIESE OPTION IST AUSSCHLIESSLICH MAGIEKUNDIGEN VORBEHALTEN!" */
				} else {
					spell_result = test_spell(hero, SP_FORAMEN_FORAMINOR, 5);

					if (spell_result > 0) {

						sub_ae_splash(hero, get_spell_cost(SP_FORAMEN_FORAMINOR, 0));

						*(amap_ptr + MAP_POS(13,2)) &= 0x0f;
						*(amap_ptr + MAP_POS(13,2)) |= DNG_TILE_STAIR_DOWN << 4;

						add_hero_ap(hero, 1L);

					} else if (spell_result != -99) {

						sub_ae_splash(hero, get_spell_cost(SP_FORAMEN_FORAMINOR, 1));

						g_fig_escape_position[NORTH] = g_fig_escape_position[EAST] = g_fig_escape_position[SOUTH] = g_fig_escape_position[WEST] = target_pos;

						do_fight(FIGHT_ID_F100_03);

					} else {

						sprintf(g_dtp2,	get_ttx(607), hero->alias); /* "%s HAT NICHT GENÜGEND ASTRALENERGIE FÜR DIESEN ZAUBER." */
						GUI_output(g_dtp2);
					}
				}
			}

			gs_x = gs_x_bak;
			gs_y = gs_y_bak;
			g_dng_refresh_direction = -1;

		} else {

			gs_x++;
			DNG_inc_level();
		}

	} else if (target_pos == DNG_POS(1,12,3) && target_pos != gs_dng_pos_bak && gs_treasure_maps[1] != 1)
	{
		GUI_output(get_tx(1)); /* "EIN HOHLES UND SEHR, SEHR HÄMISCHES GELÄCHTER ERKLINGT, DAS EUCH DA..." */

	} else if (target_pos == DNG_POS(1,13,2) && target_pos != gs_dng_pos_bak)
	{

		gs_x++;

		DNG_dec_level();

	} else if (target_pos == DNG_POS(1,6,1) && target_pos != gs_dng_pos_bak && (gs_dng07_mu_elixir_flag != 2))
	{
		if (!gs_dng07_mu_elixir_flag)
		{
			gs_dng07_mu_elixir_flag = 1;

			GUI_output(get_tx(2)); /* "VOR EUCH MANIFESTIERT SICH DIE GESTALT EINER JUNGEN ELFE. SIE MUSTE..." */

			if (GUI_bool(get_tx(3))) /* "WOLLT IHR DIE PHIOLE MITNEHMEN?" */
			{
				give_new_item_to_group(ITEM_ID_MU_ELIXIER, 1, 1);

				gs_dng07_mu_elixir_flag = 2;
			}

			/* ORIGINAL-BUG: forgot to set hero */
			/* REMARK: hero is still the first available hero, so the loop runs behind the last hero
			 * slot if that hero is not at position 0. Should start with get_hero(0). */
#if FIX_0111
			/* Fix 111: bei Platz 0 beginnen (hero zeigte noch auf den ersten handlungsfaehigen Helden) */
			hero = get_hero(0);
#endif
			for (i = 0; i <= 6; i++, hero++)
			{
				if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !hero->flags.dead)
				{
					hero->attrib[ATTRIB_MU].current += 3;

					hero->flags.encouraged = 1;
				}
			}
		} else {
			if (GUI_bool(get_tx(5))) /* "HIER LIEGT EINE KLEINE PHIOLE AM BODEN.@WOLLT IHR SIE MITNEHMEN?" */
			{
				give_new_item_to_group(ITEM_ID_MU_ELIXIER, 1, 1);

				gs_dng07_mu_elixir_flag = 2;
			}
		}

	} else if ((target_pos == DNG_POS(2,3,13) || target_pos == DNG_POS(2,2,13)) && target_pos != gs_dng_pos_bak && !gs_dng07_flicker_flag)
	{
		gs_dng07_flicker_flag = 1;

		GUI_output(get_tx(4)); /* "ALS IHR DEN RAUM BETRETET, MEINT IHR, GERADE NOCH EIN FLACKERN IN E..." */

	} else if (target_pos == DNG_POS(2,1,11) && target_pos != gs_dng_pos_bak && !gs_dng07_anti_mu_elixir_flag)
	{
		if (GUI_bool(get_tx(5))) /* "HIER LIEGT EINE KLEINE PHIOLE AM BODEN.@WOLLT IHR SIE MITNEHMEN?" */
		{
			give_new_item_to_group(ITEM_ID_MU_ELIXIER__BAD, 1, 1);

			gs_dng07_anti_mu_elixir_flag = 1;
		}

	} else if (target_pos == DNG_POS(2,10,1) && target_pos != gs_dng_pos_bak)
	{
		if (!do_fight(FIGHT_ID_F100_13))
		{
			if (gs_treasure_maps[1] == 2)
			{
				g_tmap_double2 = 1;
			}

			gs_treasure_maps[1] = 1;

			add_hero_ap_all(10L);

			show_treasure_map();
		}

	} else if (target_pos == DNG_POS(2,14,13) && target_pos != gs_dng_pos_bak)
	{
		/* endless corridor (vermutlich): at (14,13) and (8,13) the group is set back and turned around */
		gs_x = 7;

		gs_viewdir = REVERSE_VIEWDIR(gs_viewdir);

	} else if (target_pos == DNG_POS(2,8,13) && target_pos != gs_dng_pos_bak)
	{
		gs_x = 13;

		gs_viewdir = REVERSE_VIEWDIR(gs_viewdir);

	} else if (target_pos == DNG_POS(2,1,2) && target_pos != gs_dng_pos_bak && !gs_dng07_poison_flag)
	{
		if (GUI_bool(get_tx(6))) /* "AUF DEN REGALEN STEHEN EINE UNMENGE VON FLASCHEN UND KRÜGEN.@WOLLT..." */
		{
			sprintf(g_dtp2, get_tx(7), hero->alias); /* "%s STÖSST EINE KLEINE FLASCHE UM. DER INHALT VERMISCHT SICH MIT DEM..." */
			GUI_output(g_dtp2);

			/* poisonous vapour: damage every 15 minutes for one day while in this dungeon (seg002) */
			gs_dng07_poison_flag = 1;
			gs_dng07_poison_timer = DAYS(1);
		}

	} else if (target_pos == DNG_POS(0,1,14) && target_pos != gs_dng_pos_bak)
	{
		if (GUI_bool(get_tx(15))) /* "WOLLT IHR WIRKLICH ZURÜCK IN DIE BERGE?" */
		{
			/* ORIGINAL-BUG: forgot to set hero */
			/* REMARK: as above; additionally heroes of other groups who lose the flag here are not
			 * checked, and leaving the dungeon any other way keeps MU +3. */
#if FIX_0111
			/* Fix 111: bei Platz 0 beginnen (hero zeigte noch auf den ersten handlungsfaehigen Helden) */
			hero = get_hero(0);
#endif
			for (i = 0; i <= 6; i++, hero++)
			{
				if ((hero->typus != HERO_TYPE_NONE) && hero->flags.encouraged)
				{
					hero->attrib[ATTRIB_MU].current -= 3;

					hero->flags.encouraged = 0;
				}
			}

			leave_dungeon();

			gs_town_id = gs_travel_destination_town_id;
			gs_x = gs_travel_destination_x;
			gs_y = gs_travel_destination_y;
			gs_town_loc_type = LOCTYPE_NONE;
			gs_viewdir = REVERSE_VIEWDIR(gs_travel_destination_viewdir);

			sprintf(g_dtp2, get_tx(14), get_ttx(gs_journey_destination_town_id + 0xeb)); /* "IHR VERLASST DIE VERTRACKTEN GÄNGE DIESER..." / TEXT.LTX 236..287: Ortsname */

			GUI_output(g_dtp2);

			timewarp(HOURS(3));

			g_fading_state = 3;
		} else
		{
			gs_y = gs_y_bak;
			gs_x = gs_x_bak;
		}

	}

	g_textbox_width = tw_bak;
	gs_dng_pos_bak = target_pos;

	return 0;
}

/**
 * \brief   statue of the Nameless God in the Kultstaette des Namenlosen (DNG09)
 *
 *          The square value 4 (corridor with flag bit 2) marks a statue.
 *
 * \param   prob        probability in % that touching the statue has an effect
 * \param   bonus       gods estimation bonus for destroying the statue
 */
void DNG09_statues(const signed int prob, const signed int bonus)
{
	signed int i;
	signed int randval;
	struct struct_hero *hero;
	uint8_t *amap_ptr;

	hero = get_first_hero_available_in_group();

	amap_ptr = g_dng_map;

	if (*(amap_ptr + MAP_POS(gs_x, gs_y)) == 4)
	{
		/* TODO: no forced decision here ? */
		i = GUI_radio(get_tx(4), 3, get_tx(5), get_tx(6), get_tx(7)); /* "IHR STEHT VOR EINER STATUE AU..." / "DIE STATUE BERÜHREN?" / "DIE STATUE ZERSCHLAGEN?" / "WEITERGEHEN?" */

		if (i == 1)
		{
			/* praise the nameless god */
			if (random_schick(100) <= prob)
			{
				/* 49% (random_schick(100) < 50) */
				if (random_schick(100) < 50 && !hero->flags.gods_pissed && !gs_nameless_destroyed)
				{
					/* increase one attribute of the leader permanently */
					randval = random_schick(7) - 1;

					hero->attrib[randval].normal++;
					hero->attrib[randval].current++;

					/* ... but the twelve won't grant miracles any more */
					hero->flags.gods_pissed = 1;

					sprintf(g_dtp2, get_tx(8), hero->alias); /* "%s FÜHLT SICH IRGENDWIE SELTSAM UND BEMERKT, DASS SICH ETWASVERÄNDE..." */
					GUI_output(g_dtp2);

				} else {
					/* loose 1W6 LE */
					sub_hero_le(hero, random_schick(6));

					sprintf(g_dtp2,	get_tx(9), hero->alias); /* "%s SPÜRT EIN HEFTIGES STECHEN UND REISST ERSCHROCKEN DIE HAND ZURÜCK." */
					GUI_output(g_dtp2);
				}
			}

		} else if (i == 2) {

			/* destroy the statue */

			/* remove the statue from the map */
			*(amap_ptr + MAP_POS(gs_x, gs_y)) &= 0xfb; /* clear flag 2 */

			GUI_output(get_tx(10)); /* "MIT WUCHTIGEN HIEBEN ZERSÖRT IHR DAS BLASPHEMISCHE ABBILD DES NAMEN..." */

			/* increase estimation */
			/* Original-Bug: the god ids are 1..14 (GOD_ID_PRAIOS..GOD_ID_IFIRN). The loop changes the
			 * unused entry 0 and skips Ifirn (14). Compare seg112: for (i = 1; i <= 14; i++). */
#if FIX_0070
			/* Fix 70: Goetter 1..14 (Praios bis Ifirn) */
			for (i = 1; i <= 14; i++) {
#else
			for (i = 0; i < 14; i++) {
#endif
				gs_gods_estimation[i] += bonus;
			}

			hero = get_hero(0);
			for (i = 0; i <= 6; i++, hero++)
			{
				if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !hero->flags.dead)
				{
					/* the twelve will grant miracles again */
					hero->flags.gods_pissed = 0;
				}
			}

			/* no statue can be praised any longer */
			gs_nameless_destroyed = 1;
		}
	}
}
