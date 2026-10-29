/*
 * seg079 - Dungeon "Spinnenhoehle" (DNG03)
 *
 * Inhalt:    DNG03_handler() steuert die Spinnenhoehle zwischen Ottarje und Skjal:
 *            Rutsche und Teleporter (kosten AE bzw. LE), Raetsel "SPINNENNETZ" und
 *            "MACTANS", Spinnenstatue, Speerfallgrube (per Hebel schaltbar), Saeuregruben
 *            (Ruestung verliert RS), die Kaempfe mit dem Hohepriester, der Altar fuer die
 *            Kristalle, das Spinnengelege (Verbrennen startet die Todesfalle mit 15
 *            Schritten) und die Ausgaenge. Dazu die Truhen: Leiter nach unten
 *            (chest05), Fallentruhe mit Speergrube (chest11, per Hebel abschaltbar),
 *            Kristalltruhe (chest12, nur mit genau 4 Kristallen).
 * Aufrufer:  DNG03_handler() ueber g_dng_handlers (seg076), die Truhenfunktionen ueber
 *            g_dng03_specialchests und loot_special_chest() (seg092).
 * Daten:     g_dng03_specialchests[14], Loesungswoerter SPINNENNETZ und MACTANS.
 * Besonders: Teleporter-Schaden trifft alle Helden, auch andere Gruppen. Hebel (0,9,11)
 *            verlangt neues Feld UND Drehung im selben Schritt, ist dadurch praktisch
 *            nicht bedienbar. DNG03_chest11_loot(): faellt die ganze Gruppe, ist das Spiel
 *            auch fuer andere Gruppen vorbei.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg079.c
 */

#include <stdio.h>
#include <string.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg007.h"
#include "seg025.h"
#include "seg032.h"
#include "seg075.h"
#include "seg079.h"
#include "seg092.h"
#include "seg096.h"
#include "seg097.h"
#include "seg103.h"
#include "seg105.h"

struct struct_chest g_dng03_specialchests[14] = {
	{ DNG_POS(0,5,3),	0, NULL,			NULL,			DNG03_chest00_loot, 15, 0, 0 },
	{ DNG_POS(0,9,1),	0, NULL,			NULL,			DNG03_chest01_loot, 0, 0, 0 },
	{ DNG_POS(0,11,1),	0, NULL,			NULL,			DNG03_chest02_loot, 0, 0, 48 },
	{ DNG_POS(0,10,6),	6, use_lockpicks_on_chest,	chest_protected_heavy,	DNG03_chest03_loot, 0, 0, 0 },
	{ DNG_POS(0,11,11),	4, use_lockpicks_on_chest,	chest_poisoned2,	DNG03_chest04_loot, 0, 0, 0 },
	{ DNG_POS(0,9,12),	5, use_lockpicks_on_chest,	chest_ignifax_normal,	DNG03_chest05_loot, 0, 0, 0 },
	{ DNG_POS(0,11,12),	4, use_lockpicks_on_chest,	chest_protected_heavy,	DNG03_chest06_loot, 0, 0, 0 },
	{ DNG_POS(1,14,14),	0, NULL,			NULL,			DNG03_chest07_loot, 0, 0, 0 },
	{ DNG_POS(1,1,14),	1, use_lockpicks_on_chest,	chest_ignifax_brutal,	DNG03_chest08_loot, 0, 0, 0 },
	{ DNG_POS(1,1,3),	6, use_lockpicks_on_chest,	chest_protected_brutal, DNG03_chest09_loot, 0, 20000, 0 },
	{ DNG_POS(1,14,0),	0, NULL,			NULL,			DNG03_chest10_loot, 0, 0, 0 },
	{ DNG_POS(1,9,11),	0, NULL,			NULL,			DNG03_chest11_loot, 0, 0, 0 },
	{ DNG_POS(1,10,7),	6, use_lockpicks_on_chest,	DNG03_chest12_trap,	DNG03_chest12_loot, 0, 0, 0 },
	{     -1,		0, NULL,			NULL,			NULL, 0, 0, 0 }
}; // ds:0x9480
const char g_dng03_str_spinnennetz[12] = "SPINNENNETZ"; // ds:0x95a6
const char g_dng03_str_mactans[8] = "MACTANS"; // ds:0x95b2


/**
 * \brief   dungeon handler of the spider cave
 */
signed int DNG03_handler(void)
{
	signed int target_pos;
	signed int i;
	signed int j;
	signed int tw_bak;
	struct struct_hero *hero;
	signed int armor_rs;
	signed int armor_malus;

	tw_bak = g_textbox_width;
	g_textbox_width = 7;

	target_pos = DNG_POS(gs_dungeon_level, gs_x, gs_y);

	if ((target_pos == DNG_POS(0,4,7) || target_pos == DNG_POS(0,4,9) || target_pos == DNG_POS(0,6,8)) &&
		target_pos != gs_dng_pos_bak)
	{
		GUI_output(get_tx(1)); /* "UNTER EUCH GIBT DER BODEN NACH UND IN EINER RASANTEN FAHRT, STÜRZT..." */

		gs_x = gs_y = 1;
		DNG_inc_level();

	} else if (target_pos == DNG_POS(0,3,4) && target_pos != gs_dng_pos_bak && gs_viewdir == NORTH)
	{
		GUI_input(get_tx(2), 15); /* "WAS IST UNDURCHDRINGLICH WIE EINE EHERNE WAND UND DOCH DURCHSICHTIG..." */

		if (!strcmp(g_text_input_buf, g_dng03_str_spinnennetz))
		{
			GUI_output(get_tx(4)); /* "UM EUCH HERUM BEGINNT SICH ALLES ZU DREHEN, ALS IHR VON EINEM TELEP..." */

			gs_x = 9;
			gs_y = 8;
			DNG_inc_level();

			/* teleport: magic users lose 1W6 AE, the others 1W6 LE.
			 * Original-Bug: no check of group_id (and dead), heroes of other groups are hit as well.
			 * The same in the three other teleporters below. */
			hero = get_hero(0);
			for (i = 0; i <= 6; i++, hero++)
			{
				if ((hero->typus >= HERO_TYPE_HEXE) && hero->ae != 0) {

					sub_ae_splash(hero, random_schick(6));

				} else if (hero->typus != HERO_TYPE_NONE) {

					sub_hero_le(hero, random_schick(6));
				}
			}
		}

	} else if (target_pos == DNG_POS(0,11,6) && target_pos != gs_dng_pos_bak)
	{
		GUI_output(get_tx(4)); /* "UM EUCH HERUM BEGINNT SICH ALLES ZU DREHEN, ALS IHR VON EINEM TELEP..." */

		gs_x = 1;
		gs_y = 8;
		gs_viewdir = WEST;

		DNG_update_pos();

		hero = get_hero(0);
		for (i = 0; i <= 6; i++, hero++)
		{
			if ((hero->typus >= HERO_TYPE_HEXE) && hero->ae != 0) {

				sub_ae_splash(hero, random_schick(6));

			} else if (hero->typus != HERO_TYPE_NONE) {

				sub_hero_le(hero, random_schick(6));
			}
		}

	} else if ((target_pos == DNG_POS(0,6,10) || target_pos == DNG_POS(0,7,9)) &&
			(target_pos != gs_dng_pos_bak || gs_viewdir != gs_viewdir_bak))
	{
		if ((target_pos == DNG_POS(0,6,10) && gs_viewdir == EAST) ||
			(target_pos == DNG_POS(0,7,9) && gs_viewdir == SOUTH))
		{
			DNG_update_pos();
			GUI_output(get_tx(3)); /* "VOR DER WAND STEHT DIE STATUE EINER SPINNE MIT MENSCHLICHEM KOPF. A..." */

			gs_viewdir_bak = gs_viewdir;
		}

	} else if ((target_pos == DNG_POS(0,10,9) || target_pos == DNG_POS(0,9,14)) &&
			gs_dng03_lever_trap &&
			target_pos != gs_dng_pos_bak)
	{
		GUI_output(get_tx(5)); /* "VOR EUCH TUT SICH EINE SPEERGESPICKTE FALLGRUBE AUF. VORSICHTIG VER..." */

		hero = get_hero(0);
		for (i = 0; i <= 6; i++, hero++)
		{
			if ((hero->typus != HERO_TYPE_NONE) &&
				(hero->group_id == gs_active_group_id) &&
				!hero->flags.dead &&
				(test_talent(hero, TA_KLETTERN, 2) <= 0))
			{
				sprintf(g_dtp2,	get_tx(6), hero->alias,	GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST)); /* "%s RUTSCHT AUS, STÜRZT UND ZIEHT SICH EINIGE HÄSSLICH BLUTENDE WUND..." */
				GUI_output(g_dtp2);

				sub_hero_le(hero, dice_roll(1, 6, 3));
			}
		}

	} else if ((target_pos == DNG_POS(0,5,14)) && (gs_viewdir != gs_viewdir_bak) && (gs_viewdir == SOUTH))
	{
		GUI_input(get_tx(7), 15); /* "WER IST DER HERR ÜBER ALLE SPINNEN?" */

		if (!strcmp(g_text_input_buf, g_dng03_str_mactans))
		{
			GUI_output(get_tx(4)); /* "UM EUCH HERUM BEGINNT SICH ALLES ZU DREHEN, ALS IHR VON EINEM TELEP..." */

			gs_x = 3;
			gs_y = 4;
			gs_viewdir = SOUTH;
			DNG_update_pos();

			hero = get_hero(0);
			for (i = 0; i <= 6; i++, hero++)
			{
				if ((hero->typus >= HERO_TYPE_HEXE) && hero->ae != 0) {

					sub_ae_splash(hero, random_schick(6));

				} else if (hero->typus != HERO_TYPE_NONE) {

					sub_hero_le(hero, random_schick(6));
				}
			}
		} else {
			gs_viewdir = WEST;
		}

	} else if (target_pos == DNG_POS(0,9,11) && (gs_viewdir == WEST) &&
			target_pos != gs_dng_pos_bak && gs_viewdir != gs_viewdir_bak)
	{
		/* LEVER: switches the spear pit trap of chest 11 at (1,9,11) (gs_dng03_lever_chest11, initially 1 = armed).
		 * Original-Bug: "&&" instead of "||" (compare the lever at (1,3,11)). The group must enter
		 * the square and turn in the same step, which normal movement never does, so this lever
		 * can practically not be used (vermutlich, not tested). */
		if (GUI_bool(get_tx(8))) /* "EIN KLEINER, UNSCHEINBARER HEBEL RAGT AUS DER WAND.@WOLLT IHR IHNBE..." */
		{
			gs_dng03_lever_chest11 ^= 1;
			GUI_output(get_tx(9)); /* "IHR KÖNNT EIN LEISES KLICKEN VERNEHMEN." */
		}

	} else if (target_pos == DNG_POS(1,3,11) && (gs_viewdir == EAST) &&
			(target_pos != gs_dng_pos_bak || gs_viewdir != gs_viewdir_bak))
	{
		/* LEVER: switches the spear pits at (0,10,9) and (0,9,14) */
		if (GUI_bool(get_tx(8))) /* "EIN KLEINER, UNSCHEINBARER HEBEL RAGT AUS DER WAND.@WOLLT IHR IHNBE..." */
		{
			gs_dng03_lever_trap ^= 1;
			GUI_output(get_tx(9)); /* "IHR KÖNNT EIN LEISES KLICKEN VERNEHMEN." */
		}

	} else if ((target_pos == DNG_POS(1,14,13) || target_pos == DNG_POS(1,13,4) || target_pos == DNG_POS(1,11,2)) &&
		target_pos != gs_dng_pos_bak)
	{
		/* acid pit: j = number of failed Gefahrensinn tests of the first two heroes.
		 * If any test failed, the first hero falls; the second one only if both failed.
		 * Otherwise each of them falls on a failed GE test. */
		j = 0;

		if (test_talent(get_first_hero_available_in_group(), TA_GEFAHRENSINN, 4) <= 0)
		{
			j++;
		}

		if ((hero = get_second_hero_available_in_group()) &&
			(hero->typus != HERO_TYPE_NONE) &&
			(hero->group_id == gs_active_group_id) &&
			!hero->flags.dead &&
			(test_talent(hero, TA_GEFAHRENSINN, 4) <= 0))
		{
			j++;
		}

		hero = get_first_hero_available_in_group();

		if (j != 0 || test_attrib(hero, ATTRIB_GE, 2) <= 0)
		{
			sprintf(g_dtp2,	get_tx(13), hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_3RD)); /* "%s STÜRZT HALS ÜBER KOPF IN EINE SÄUREGEFÜLLTE GRUBE UND ZIEHT SICH..." */
			GUI_output(g_dtp2);

			sub_hero_le(hero, dice_roll(2, 6, 0));

			timewarp(MINUTES(20));

			if (hero->inventory[HERO_INVENTORY_SLOT_BODY].item_id != ITEM_ID_NONE)
			{
				/* RS of the equipped body armor gets degraded by 3, but not below 0 */
				armor_rs = g_armor_stats_table[g_itemsdat[hero->inventory[HERO_INVENTORY_SLOT_BODY].item_id].item_type_stats_id].rs
				    - hero->inventory[HERO_INVENTORY_SLOT_BODY].rs_lost;

				armor_malus = (armor_rs > 3 ? 3 : (armor_rs > 0 ? armor_rs : 0));

				hero->inventory[HERO_INVENTORY_SLOT_BODY].rs_lost += armor_malus;
				hero->rs_bonus -= armor_malus;
			}
		}

		if ((hero = get_second_hero_available_in_group()) &&
			(j == 2 ||
			(hero->typus != HERO_TYPE_NONE) &&
			(hero->group_id == gs_active_group_id) &&
			!hero->flags.dead &&
			(test_attrib(hero, ATTRIB_GE, 2) <= 0)))
		{

			sprintf(g_dtp2,	get_tx(13), hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_3RD)); /* "%s STÜRZT HALS ÜBER KOPF IN EINE SÄUREGEFÜLLTE GRUBE UND ZIEHT SICH..." */
			GUI_output(g_dtp2);

			sub_hero_le(hero, dice_roll(2, 6, 0));

			timewarp(MINUTES(20));

			if (hero->inventory[HERO_INVENTORY_SLOT_BODY].item_id != ITEM_ID_NONE)
			{
				/* RS of the equipped body armor gets degraded by 3, but not below 0 */
				armor_rs = g_armor_stats_table[g_itemsdat[hero->inventory[HERO_INVENTORY_SLOT_BODY].item_id].item_type_stats_id].rs
				    - hero->inventory[HERO_INVENTORY_SLOT_BODY].rs_lost;

				armor_malus = (armor_rs > 3 ? 3 : (armor_rs > 0 ? armor_rs : 0));

				hero->inventory[HERO_INVENTORY_SLOT_BODY].rs_lost += armor_malus;
				hero->rs_bonus -= armor_malus;
			}
		}

	} else if ((target_pos == DNG_POS(1,2,10) || target_pos == DNG_POS(1,3,13)) &&
		target_pos != gs_dng_pos_bak)
	{
		g_fig_escape_position[NORTH] = g_fig_escape_position[WEST] = DNG_POS_DIR(1,2,8,NORTH);
		g_fig_escape_position[EAST] = g_fig_escape_position[SOUTH] = DNG_POS_DIR(1,5,13,NORTH);

		/* the place visited first gets fight A, the other one fight B */
		if (!gs_dng03_priest_fight_place)
		{
			gs_dng03_priest_fight_place = 14;
		}

		do_fight(gs_dng03_priest_fight_place == 14 ? 224 : 225); /* FIGHT_ID_F051_14A : FIGHT_ID_F051_14B */

	} else if ((target_pos == DNG_POS(1,2,4) || target_pos == DNG_POS(1,5,2)) &&
		target_pos != gs_dng_pos_bak)
	{
		g_fig_escape_position[NORTH] = g_fig_escape_position[EAST] = DNG_POS_DIR(1,7,2,NORTH);
		g_fig_escape_position[SOUTH] = g_fig_escape_position[WEST] = DNG_POS_DIR(1,2,6,NORTH);

		if (!gs_dng03_priest_fight_place)
		{
			gs_dng03_priest_fight_place = 16;
		}

		do_fight(gs_dng03_priest_fight_place == 16 ? 222 : 223); /* FIGHT_ID_F051_16A : FIGHT_ID_F051_16B */

	} else if (target_pos == DNG_POS(1,1,12) && (target_pos != gs_dng_pos_bak) && !gs_dng03_sanctum_smashed)
	{

		/* check if a hero in this group has crystals (i = 0: yes, 1: no) */
		i = get_first_hero_with_item(ITEM_ID_KRISTALL) != -1 ? 0 : 1;

		do {
			j = GUI_radio(get_tx(14), 2, get_tx(15), !i ? get_tx(16) : get_tx(29)); /* "IHR STEHT VOR EINEM ALTAR, DE..." / "DEN ALTER ZERSCHLAGEN?" / "KRISTALLE AUF DEN ALTAR LEGEN?" / "SPÄTER NOCHMAL KOMMEN." */

		} while (j == -1);

		if (j == 1)
		{
			GUI_output(get_tx(18)); /* "IHR ZERSCHLAGT DIE KULTSTÄTTE DER SPINNENDIENER." */
			gs_dng03_sanctum_smashed = 1;

		} else if (j == 2 && !i)
		{
			GUI_output(get_tx(17)); /* "EIN TIEFES BRUMMEN ERFÜLLT DIE LUFT UND EINE GEISTERHAFTE FORM SCHÄ..." */

			g_fig_escape_position[NORTH] = g_fig_escape_position[EAST] = g_fig_escape_position[SOUTH] = g_fig_escape_position[WEST] = DNG_POS_DIR(1,5,13,NORTH);

			/* drop all crystals from the heroes of that group */
			i = get_first_hero_with_item(ITEM_ID_KRISTALL);

			do {
				hero = get_hero(i);

				drop_item(hero, inv_slot_of_item(hero, ITEM_ID_KRISTALL), 1);

				i = get_first_hero_with_item(ITEM_ID_KRISTALL);

			} while (i != -1);

			do_fight(FIGHT_ID_F051_14C);
		}

	} else if (target_pos == DNG_POS(1,1,1) && target_pos != gs_dng_pos_bak &&
			!gs_dng03_spidereggs_burned)
	{
		do {
			j = GUI_radio(get_tx(19), 2, get_tx(20), get_tx(21)); /* "AM BODEN KÖNNT IHR HUNDERTE U..." / "DAS GELEGE VERBRENNEN?" / "ZUERST DIE HÖHLE WEITER UNTER..." */

		} while (j == -1);

		if (j == 1) {

			GUI_output(get_tx(22)); /* "DIE SPINNENBRUT LÖST SICH UNTER PRASSELN UND ZISCHEN IN SCHWARZEN R..." */

			add_hero_ap_all(50);

			gs_dng03_spidereggs_burned = 1;

			/* activate DEATHTRAP #2 with 15 steps */
			gs_deathtrap_steps = 15;
			gs_deathtrap = 2;
		}

	} else if (target_pos == DNG_POS(1,10,4) && target_pos != gs_dng_pos_bak)
	{
		GUI_output(get_tx(24)); /* "IN DIESER NISCHE FINDET SICH EIN GROSSER HAUFEN KNOCHEN VON KLEINER..." */

	} else if (target_pos == DNG_POS(1,10,8) && target_pos != gs_dng_pos_bak)
	{
		GUI_output(get_tx(4)); /* "UM EUCH HERUM BEGINNT SICH ALLES ZU DREHEN, ALS IHR VON EINEM TELEP..." */
		gs_x = 5;
		gs_y = 14;
		DNG_dec_level();
		gs_viewdir = NORTH;

		hero = get_hero(0);
		for (i = 0; i <= 6; i++, hero++)
		{
			if ((hero->typus >= HERO_TYPE_HEXE) && hero->ae) {

				sub_ae_splash(hero, random_schick(6));

			} else if (hero->typus != HERO_TYPE_NONE) {

				sub_hero_le(hero, random_schick(6));
			}
		}

	} else if ((target_pos == DNG_POS(1,2,1) && target_pos != gs_dng_pos_bak && GUI_bool(get_tx(23))) || /* "EIN SCHMALER SPALT IN DER WAND SCHEINT INS FREIE ZU FÜHREN.@WOLLT I..." */
			target_pos == DNG_POS(0,0,8))
	{
		leave_dungeon();
		gs_town_id = gs_travel_destination_town_id;
		gs_x = gs_travel_destination_x;
		gs_y = gs_travel_destination_y;
		gs_town_loc_type = LOCTYPE_NONE;
		gs_viewdir = REVERSE_VIEWDIR(gs_travel_destination_viewdir);

		sprintf(g_dtp2, get_tx(30), get_ttx(gs_journey_destination_town_id + 0xeb)); /* "ERSCHÖPFT UND MÜDE VERLASST IHR DIESE SPIN..." / TEXT.LTX 236..287: Ortsname */

		GUI_output(g_dtp2);

		timewarp(HOURS(3));

		g_fading_state = 3;
	}

	g_textbox_width = tw_bak;
	gs_dng_pos_bak = target_pos;

	return 0;
}

void DNG03_chest00_loot(struct struct_chest* chest)
{
        uint8_t* ptr_bak = chest->loot.content;

        chest->loot.content = gs_dng03_chest00_content;

        loot_simple_chest(chest);

        chest->loot.content = ptr_bak;
}

void DNG03_chest01_loot(struct struct_chest* chest)
{
        uint8_t* ptr_bak = chest->loot.content;

        chest->loot.content = gs_dng03_chest01_content;

        loot_simple_chest(chest);

        chest->loot.content = ptr_bak;
}

void DNG03_chest02_loot(struct struct_chest* chest)
{
        uint8_t* ptr_bak = chest->loot.content;

        chest->loot.content = gs_dng03_chest02_content;

        loot_simple_chest(chest);

        chest->loot.content = ptr_bak;
}

void DNG03_chest03_loot(struct struct_chest* chest)
{
        uint8_t* ptr_bak = chest->loot.content;

        chest->loot.content = gs_dng03_chest03_content;

        loot_simple_chest(chest);

        chest->loot.content = ptr_bak;
}

void DNG03_chest04_loot(struct struct_chest* chest)
{
        uint8_t* ptr_bak = chest->loot.content;

        chest->loot.content = gs_dng03_chest04_content;

        loot_simple_chest(chest);

        chest->loot.content = ptr_bak;
}

/**
 * \brief   "chest" (0,9,12): behind the lock there is a ladder down to (1,10,12)
 */
void DNG03_chest05_loot(struct struct_chest* chest)
{
	signed int answer;

	do {
		answer = GUI_radio(get_tx(10), 2, get_tx(11), get_tx(12)); /* "HIER FÜHRT EINE LEITER IN DIE..." / "HINABSTEIGEN?" / "HIER BLEIBEN?" */

	} while (answer == -1);

	if (answer == 1) {

		gs_x = 10;
		gs_y = 12;
		DNG_inc_level();
	}
}

void DNG03_chest06_loot(struct struct_chest* chest)
{
        uint8_t* ptr_bak = chest->loot.content;

        chest->loot.content = gs_dng03_chest06_content;

        loot_simple_chest(chest);

        chest->loot.content = ptr_bak;
}

void DNG03_chest07_loot(struct struct_chest* chest)
{
        uint8_t* ptr_bak = chest->loot.content;

        chest->loot.content = gs_dng03_chest07_content;

        loot_simple_chest(chest);

        chest->loot.content = ptr_bak;
}

void DNG03_chest08_loot(struct struct_chest* chest)
{
        uint8_t* ptr_bak = chest->loot.content;

        chest->loot.content = gs_dng03_chest08_content;

        loot_simple_chest(chest);

        chest->loot.content = ptr_bak;
}

void DNG03_chest09_loot(struct struct_chest* chest)
{
        uint8_t* ptr_bak = chest->loot.content;

        chest->loot.content = gs_dng03_chest09_content;

        loot_simple_chest(chest);

        chest->loot.content = ptr_bak;
}

void DNG03_chest12_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak;
	signed char crystals = 0;
	signed char i;
	struct struct_hero *hero = get_first_hero_available_in_group();

	/* count the crystals in the knapsack of the leader */
	for (i = HERO_INVENTORY_SLOT_KNAPSACK_1; i < NR_HERO_INVENTORY_SLOTS; i++)
	{
		if (hero->inventory[i].item_id == ITEM_ID_KRISTALL)
		{
			crystals++;
		}
	}

	/* this chest can only be opened with a leader with exactly 4 crystals in the knapsack (crystals don't stack) */
	if (crystals == 4)
	{
		ptr_bak = chest->loot.content;

		chest->loot.content = gs_dng03_chest12_content;

		loot_simple_chest(chest);

		chest->loot.content = ptr_bak;

	} else {
		DNG03_chest12_trap();
	}
}

/**
 * \brief   chest (1,14,0), always empty
 */
void DNG03_chest10_loot(struct struct_chest* chest)
{
	GUI_output(get_ttx(522)); /* "DIE TRUHE IST LEER!" */
}

/**
 * \brief   chest (1,9,11), a trap without loot: while gs_dng03_lever_chest11 is set (initially 1)
 *          a spear pit opens, otherwise the chest cannot be opened. The lever at (0,9,11) toggles it.
 *
 *          Every hero of the group needs a GE test (the first +6, the 2nd/3rd +2, the others -2).
 *          If all fail, the game ends.
 */
void DNG03_chest11_loot(struct struct_chest* chest)
{
	signed int heroes_num;
	signed int i;
	signed int test_failed_num;
	signed int handicap;

	if (!gs_dng03_lever_chest11)
	{
		GUI_output(get_tx(25)); /* "NICHTS PASSIERT. DIE TRUHE SCHEINT SICH NICHT ÖFFNEN ZU LASSEN." */

	} else {
		struct struct_hero *hero = get_hero(0);

		for (i = test_failed_num = heroes_num = 0; i <= 6; i++, hero++)
		{
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !hero->flags.dead)
			{
				heroes_num++;

				handicap = heroes_num < 2 ? 6 : (heroes_num < 4 ? 2 : -2);

				if (test_attrib(hero, ATTRIB_GE, handicap) <= 0) {

					test_failed_num++;

					sprintf(g_dtp2, get_tx(27), hero->alias); /* "%s STÜRZT IN EINE SPEERGESPICKTE FALLGRUBE." */
					GUI_output(g_dtp2);

					sub_hero_le(hero, dice_roll(2, 6, 4));
				}
			}
		}

		if (test_failed_num == heroes_num)
		{
			/* end of game */
			GUI_output(get_tx(26)); /* "SÄMTLICHE MITGLIEDER DER GRUPPE STÜRZTEN IN DIE GRUBE UND WERDEN GE..." */

			g_game_state = GAME_STATE_DEAD;
		} else {
			timewarp(HOURS(1));

			GUI_output(get_tx(28)); /* "NACH EINER STUNDE HABT IHR ES GESCHAFFT, WIEDER FESTEN BODEN UNTER..." */
		}
	}
}

/**
 * \brief   trap of chest 12: poison (chest_poisoned2()) while gs_dng03_chest12_loads > 0 (6 loads)
 */
void DNG03_chest12_trap(void)
{
	if (gs_dng03_chest12_loads) {

		gs_dng03_chest12_loads--;

		chest_poisoned2();
	}
}
