/*
 * seg091 - Dungeon "Verlassene Mine" in Prem (DNG13)
 *
 * Inhalt:    DNG13_handler() steuert die alte Mine in Prem: Ratten, Laterne, Sackgassen,
 *            Skelett, verschuettete Gaenge (DNG13_unblock_passage, mit allen vier Werkzeugen
 *            schneller), verzogene Tueren, Geraetelager, zwei Leichen, Katzengold, ein
 *            verlorener Geldbeutel, einsturzgefaehrdete Decken (DNG13_collapsing_ceiling und
 *            _easy, Schleichen-Probe) und der Ausgang mit AP je betretenem Deckenfeld.
 * Aufrufer:  DNG13_handler() ueber g_dng_handlers (seg076), DNG13_fight_intro() aus
 *            GUI_print_fight_intro_msg() (seg097), die Leichen ueber g_dng13_specialchests
 *            (seg092). gs_dng13_collapsecount/herocount setzt DNG_enter_dungeon() (seg075).
 * Daten:     g_dng13_specialchests[3] (zwei Leichen).
 * Besonders: DNG13_collapsing_ceiling(): mit Werkzeug dauert das Freiraeumen 1 Tag, ohne
 *            6 Stunden (umgekehrt zum Kommentar). Original-Bug 18 von Bright-Eyes in
 *            DNG13_collapsing_ceiling_hard() ist dagegen keiner: dort ist es richtig (mit
 *            Werkzeug 12, ohne 18 Stunden), der "Fix" dreht es falsch herum. Der Zaehler fuer
 *            die AP am Ausgang steigt bei jedem Betreten eines Deckenfeldes.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg091.c
 */

#include <stdio.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg003.h"
#include "seg007.h"
#include "seg025.h"
#include "seg047.h"
#include "seg091.h"
#include "seg092.h"
#include "seg097.h"
#include "seg103.h"
#include "seg105.h"

struct struct_chest g_dng13_specialchests[3] = {
	{ DNG_POS(0,11,5),	1, DNG13_chest00_open, NULL, DNG13_chest00_loot, 0, 0, 0 },
	{ DNG_POS(0,4,1),	1, DNG13_chest01_open, NULL, DNG13_chest01_loot, 0, 0, 0 },
	{     -1, 0, NULL, NULL, NULL, 0, 0, 0}
}; // ds:0x9d48

/**
 * \brief   dungeon handler of the abandoned mine in Prem
 */
signed int DNG13_handler(void)
{
	signed int target_pos;
	signed int tw_bak;
	int32_t p_money;
	struct struct_hero *hero;

	tw_bak = g_textbox_width;
	g_textbox_width = 7;

	target_pos = DNG_POS(gs_dungeon_level, gs_x, gs_y);

	hero = get_first_hero_available_in_group();

	if (!(gs_day_timer % MINUTES(5)) &&
		(target_pos == DNG_POS(0,5,1) || target_pos == DNG_POS(0,6,1) || target_pos == DNG_POS(0,7,1) || target_pos == DNG_POS(0,8,1)) &&
		random_schick(100) <= 5)
	{
		hero = get_hero(get_random_hero());

		sprintf(g_dtp2,	get_tx(18), hero->alias); /* "%s WIRD VON EINER RATTE GEBISSEN." */
		GUI_output(g_dtp2);

		sub_hero_le(hero, 2);
	}

	if (target_pos == DNG_POS(0,9,13) && target_pos != gs_dng_pos_bak && !gs_dng13_lantern_flag)
	{
		if (GUI_bool(get_tx(1))) /* "HIER LIEGT EINE LATERNE AM BODEN.@WOLLT IHR SIE MITNEHMEN?" */
		{
			gs_dng13_lantern_flag = 1;

			give_new_item_to_group(ITEM_ID_LATERNE__UNLIT, 1, 1);
		}

	} else if ((target_pos == DNG_POS(0,1,12) || target_pos == DNG_POS(0,14,6) || target_pos == DNG_POS(0,14,13) || target_pos == DNG_POS(0,3,1)) &&
			target_pos != gs_dng_pos_bak)
	{
		GUI_output(get_tx(2)); /* "HIER ENDET DER GANG VOR MASSIVEM FELS. OFFENSICHTLICH WURDE HIER NI..." */

	} else if (target_pos == DNG_POS(0,2,13) && target_pos != gs_dng_pos_bak)
	{
		GUI_dialog_na(60, get_tx(3)); /* "HIER LIEGT EIN VERKRÜMMTES SKELETT, ANSCHEINEND EIN WEITERERABENTEU..." */

	} else if (target_pos == DNG_POS(0,3,12) && target_pos != gs_dng_pos_bak)
	{
		DNG13_unblock_passage(get_tx(4), &gs_dng13_passage1_flag); /* "HIER IST DER GANG EINGESTÜRZT. IM LICHTSCHEIN EURER FACKELN KÖNNT I..." */

	} else if (target_pos == DNG_POS(0,4,9) && (target_pos != gs_dng_pos_bak || gs_viewdir != gs_viewdir_bak) && gs_viewdir == WEST)
	{
		GUI_output(get_tx(6)); /* "DIE TÜR VOR EUCH SIEHT AUS, ALS WÄRE SIE VON DER KRAFT DES BERGES S..." */
		gs_viewdir_bak = gs_viewdir;

	} else if (target_pos == DNG_POS(0,10,2) &&
			 (target_pos != gs_dng_pos_bak || gs_viewdir != gs_viewdir_bak) &&
			gs_viewdir == EAST)
	{
		GUI_output(get_tx(6)); /* "DIE TÜR VOR EUCH SIEHT AUS, ALS WÄRE SIE VON DER KRAFT DES BERGES S..." */
		gs_viewdir_bak = gs_viewdir;

	} else if (target_pos == DNG_POS(0,5,9) && target_pos != gs_dng_pos_bak)
	{
		DNG13_unblock_passage(get_tx(7), &gs_dng13_passage2_flag); /* "HIER IST DER GANG EINGESTÜRZT. IM LICHTSCHEIN EURER FACKELN KÖNNT I..." */

	} else if (target_pos == DNG_POS(0,2,9) && target_pos != gs_dng_pos_bak)
	{
		loot_multi_chest(gs_dng13_chest_equips, get_tx(8)); /* "DAS HIER MUSS DAS EHEMALIGE GERÄTELAGER SEIN. IHR FINDET HIER" */

	} else if (target_pos == DNG_POS(0,10,6) && target_pos != gs_dng_pos_bak)
	{
		DNG13_unblock_passage(get_tx(7), &gs_dng13_passage3_flag); /* "HIER IST DER GANG EINGESTÜRZT. IM LICHTSCHEIN EURER FACKELN KÖNNT I..." */

	} else if (target_pos == DNG_POS(0,11,5) && target_pos != gs_dng_pos_bak)
	{
		loot_special_chest(0);

	} else if (target_pos == DNG_POS(0,10,10) && target_pos != gs_dng_pos_bak)
	{
		GUI_output(get_tx(12)); /* "HIER BEFINDET SICH EIN LOCH IM BODEN, ZU SCHMAL, UM EINEN VON EUCHD..." */

	} else if (target_pos == DNG_POS(0,14,9) && target_pos != gs_dng_pos_bak)
	{
		DNG13_unblock_passage(get_tx(7), &gs_dng13_passage4_flag); /* "HIER IST DER GANG EINGESTÜRZT. IM LICHTSCHEIN EURER FACKELN KÖNNT I..." */

	} else if (target_pos == DNG_POS(0,7,3) && target_pos != gs_dng_pos_bak)
	{
		DNG13_unblock_passage(get_tx(7), &gs_dng13_passage5_flag); /* "HIER IST DER GANG EINGESTÜRZT. IM LICHTSCHEIN EURER FACKELN KÖNNT I..." */

	} else if (target_pos == DNG_POS(0,4,1) && target_pos != gs_dng_pos_bak)
	{
		loot_special_chest(0);

	} else if (target_pos == DNG_POS(0,5,5) && target_pos != gs_dng_pos_bak)
	{
		GUI_output(get_tx(15)); /* "HIER DURCHZIEHEN EINIGE SCHIMMERNDE ADERN DIE WAND..." */

		GUI_output(get_tx(16)); /* "BEDAUERLICHERWEISE IST ES NUR KATZENGOLD!" */

	} else if (target_pos == DNG_POS(0,4,7) && target_pos != gs_dng_pos_bak && !gs_dng13_money_flag)
	{
		GUI_output(get_tx(17)); /* "HIER HAT EINER DER SCHÜRFER WOHL SEINEN GELDBEUTEL VERLOREN.@IHR FI..." */

		gs_dng13_money_flag = 1;

		p_money = get_party_money();
		p_money += 41L;
		set_party_money(p_money);

	} else if ((target_pos == DNG_POS(0,7,14) || target_pos == DNG_POS(0,5,12) || target_pos == DNG_POS(0,1,8) || target_pos == DNG_POS(0,4,3) || target_pos == DNG_POS(0,7,5) || target_pos == DNG_POS(0,14,1) || target_pos == DNG_POS(0,8,8)) &&
			target_pos != gs_dng_pos_bak)
	{
		DNG13_collapsing_ceiling();

	} else if ((target_pos == DNG_POS(0,14,12) || target_pos == DNG_POS(0,7,1)) && target_pos != gs_dng_pos_bak)
	{
		DNG13_collapsing_ceiling_hard();

	} else if (target_pos == DNG_POS(0,7,15) && target_pos != gs_dng_pos_bak)
	{
		/* leave dungeon */
		/* AP bonus for each collapsed ceiling tile, 8 AP if no hero lost, 5 AP otherwise */
		/* REMARK: gs_dng13_collapsecount counts every entry of a ceiling square (DNG13_collapsing_ceiling*),
		 * whether the ceiling collapsed or not, and also repeated entries of the same square. */
		add_hero_ap_all(gs_dng13_collapsecount * (gs_dng13_herocount == count_heroes_in_group() ? 8 : 5));

		leave_dungeon();

		gs_x = 9;
		gs_y = 1;
		gs_viewdir = EAST;
	}

	g_textbox_width = tw_bak;
	gs_dng_pos_bak = target_pos;

	return 0;
}

/**
 * \brief   unblock a passage if its blocked
 *
 * \param   text        text for the output
 * \param   flag        pointer to the flag (0 = blocked / 1 = free)
 */
void DNG13_unblock_passage(char* text, uint8_t* flag)
{
	signed int has_items;

	/* check if passage is blocked */
	if (!(*flag))
	{
		/* ask if the heroes want to try */
		if (GUI_bool(text))
		{
			/* check if the group has ALL of these items:
			 * 	Schaufel, Hacke, Brecheisen, Wurfbeil
			 * 	(= shovel, pickaxe, crowbar, francesca) */

			has_items = 0;

			if (get_first_hero_with_item(ITEM_ID_SCHAUFEL) != -1 &&
				get_first_hero_with_item(ITEM_ID_HACKE) != -1 &&
				get_first_hero_with_item(ITEM_ID_BRECHEISEN) != -1 &&
				get_first_hero_with_item(ITEM_ID_WURFBEIL) != -1)
			{
				has_items = 1;
			}

			/* if all items are available, it takes 2 hours instead of 6 */
			timewarp(!has_items ? HOURS(6) : HOURS(2));

			GUI_output(get_tx(5)); /* "ES GELINGT EUCH NACH EINIGER ZEIT, DEN GANG FREIZULEGEN." */

			/* mark this passage as free */
			*flag = 1;

		} else {

			gs_x = gs_x_bak;
			gs_y = gs_y_bak;
		}
	}
}

/**
 * \brief   corpse of an adventurer gnawed to the bone, looted only once (gs_dng13_corpse0_flag)
 */
void DNG13_chest00_open(struct struct_chest* chest)
{
	loot_corpse(chest, get_tx(9), &gs_dng13_corpse0_flag); /* "HIER LIEGT DIE BIS AUF DIE KNOCHEN ABGENAGTE LEICHE EINES ABENTEURE..." */
}

void DNG13_chest00_loot(struct struct_chest* chest)
{
	uint8_t* bak = chest->loot.content;

	chest->loot.content = gs_dng13_chest0_content;

	loot_chest(chest, get_tx(10), get_tx(11)); /* "DER VERBLICHENE ABENTEURER HAT FOLGENDES G..." / "DA GIBT'S NICHTS MEHR ZU HOLEN." */

	chest->loot.content = bak;
}

/**
 * \brief   mummified corpse of an adventurer, looted only once (gs_dng13_corpse1_flag)
 */
void DNG13_chest01_open(struct struct_chest* chest)
{
	loot_corpse(chest, get_tx(14), &gs_dng13_corpse1_flag); /* "HIER LIEGT DIE MUMIFIZIERTE LEICHE EINES ABENTEURERS, DER ANSCHEINE..." */
}

void DNG13_chest01_loot(struct struct_chest* chest)
{
	uint8_t* bak = chest->loot.content;

	chest->loot.content = gs_dng13_chest1_content;

	loot_chest(chest, get_tx(10), get_tx(11)); /* "DER VERBLICHENE ABENTEURER HAT FOLGENDES G..." / "DA GIBT'S NICHTS MEHR ZU HOLEN." */

	chest->loot.content = bak;
}

/**
 * \brief   print introductions of dungeon #13 fights
 *
 * \param   fight_id    ID of the fight
 */
void DNG13_fight_intro(const signed int fight_id)
{
	if (fight_id == FIGHT_ID_DPRE10_1)
	{
		GUI_output(get_tx(13)); /* "DER RAUM, DEN IHR BETRETET IST EIN EINZIGES CHAOS. OFFENSICHTLICH D..." */
	}
}

/**
 * \brief   a ceiling in danger of collapsing (Schleichen-4 for every hero)
 *
 *          If at least two heroes fail, every hero loses 1W6+4 LE and the way has to be cleared.
 */
void DNG13_collapsing_ceiling(void)
{
	signed int i;
	signed int fails;
	signed int has_items;
	struct struct_hero *hero;

	gs_dng13_collapsecount++;

	hero = get_hero(0);
	for (i = fails = 0; i <= 6; i++, hero++)
	{
		if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
			!hero->flags.dead && test_talent(hero, TA_SCHLEICHEN, -4) <= 0)
		{
			fails++;
		}
	}

	if (fails > 1)
	{
		/* at least two heroes failed in the talent test */
		GUI_output(get_tx(19)); /* "UNTER LAUTEM GETÖSE STÜRZT ÜBER EUCH PLÖTZLICH DIE DECKE EIN UND BE..." */

		sub_group_le(dice_roll(1, 6, 4)); /* 1D6 + 4 */

		GUI_output(get_tx(20)); /* "NACH KURZEM KÖNNT IHR EUCH WIEDER AUFRAPPELN UND STELLT FEST, DASS..." */

		/* check if the group has ALL of these items:
		 * 	Schaufel, Hacke, Brecheisen, Wurfbeil
		 * 	(= shovel, pickaxe, crowbar, francesca) */

		has_items = 0;

		if (get_first_hero_with_item(ITEM_ID_SCHAUFEL) != -1 &&
			get_first_hero_with_item(ITEM_ID_HACKE) != -1 &&
			get_first_hero_with_item(ITEM_ID_BRECHEISEN) != -1 &&
			get_first_hero_with_item(ITEM_ID_WURFBEIL) != -1)
		{
			has_items = 1;
		}

		/* if all items are available, it takes 6 hours instead of one day */
		/* Original-Bug: the code does the opposite: without the tools 6 hours, with them one day
		 * (compare DNG13_unblock_passage() and DNG13_collapsing_ceiling_hard(), which are right). */
#if FIX_0122
		/* Fix 122: mit Werkzeug 6 Stunden, ohne einen Tag */
		timewarp(has_items ? HOURS(6) : DAYS(1));
#else
		timewarp(has_items == 0 ? HOURS(6) : DAYS(1));
#endif
	}
}

/**
 * \brief   a ceiling in danger of collapsing, second variant
 *
 *          Schleichen-1 (a harder test than the -4 of DNG13_collapsing_ceiling(), despite the
 *          name "easy"), 1W6+5 LE, clearing takes 12 or 18 hours.
 */
void DNG13_collapsing_ceiling_hard(void)
{
	signed int i;
	signed int fails;
	signed int has_items;
	struct struct_hero *hero;

	gs_dng13_collapsecount++;

	hero = get_hero(0);
	for (i = fails = 0; i <= 6; i++, hero++)
	{
		if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
			!hero->flags.dead && test_talent(hero, TA_SCHLEICHEN, -1) <= 0)
		{
			fails++;
		}
	}

	if (fails > 1)
	{
		/* at least two heroes failed in the talent test */
		GUI_output(get_tx(19)); /* "UNTER LAUTEM GETÖSE STÜRZT ÜBER EUCH PLÖTZLICH DIE DECKE EIN UND BE..." */

		sub_group_le(dice_roll(1, 6, 5)); /* 1D6 + 5 */

		GUI_output(get_tx(20)); /* "NACH KURZEM KÖNNT IHR EUCH WIEDER AUFRAPPELN UND STELLT FEST, DASS..." */

		/* check if the group has ALL of these items:
		 * 	Schaufel, Hacke, Brecheisen, Wurfbeil
		 * 	(= shovel, pickaxe, crowbar, francesca) */

		has_items = 0;

		if (get_first_hero_with_item(ITEM_ID_SCHAUFEL) != -1 &&
			get_first_hero_with_item(ITEM_ID_HACKE) != -1 &&
			get_first_hero_with_item(ITEM_ID_BRECHEISEN) != -1 &&
			get_first_hero_with_item(ITEM_ID_WURFBEIL) != -1)
		{
			has_items = 1;
		}

#ifndef M302de_ORIGINAL_BUGFIX
		/* Original-Bug 18 */
		/* if all items are available, it takes 18 hours instead of 12.
		 * So in the presence of tools, it takes longer to clear the way. This doesn't make sense at all. */
		/* REMARK: this is a misreading: has_items == 0 (no tools) gives 18 hours, with the tools it takes
		 * 12 hours, which is sensible. The "fix" below reverses it into the wrong order. The real bug
		 * of this kind is in DNG13_collapsing_ceiling() above (6 hours vs. one day). */
		timewarp(has_items == 0 ? HOURS(18) : HOURS(12));
#else
		/* Fix: Reverse the two cases */
		timewarp(has_items == 0 ? HOURS(12) : HOURS(18));
#endif
	}
}
