/*
 * seg077 - Dungeon "Totenschiff" (DNG01)
 *
 * Inhalt:    DNG01_handler() steuert die Ereignisfelder des Totenschiffs: Falltueren und
 *            Gruben, Saebel, Armbrust und Flim-Flam-Amulett zum Mitnehmen, Rattenbisse,
 *            die gefangene Ardora von Greifenfurt (NSC), der Spalt zum Hochklettern und
 *            das Verlassen des Schiffs (mit Untergang, wenn der Daemon besiegt ist).
 *            Dazu die Truhen: Raetseltruhe "MARBO" mit dem goldenen Schluessel
 *            (DNG01_chest06_*), Leiche, und die Truhe mit dem goldenen Schluessel, die den
 *            Kampf gegen den Daemon und die Todesfalle (30 Schritte) ausloest.
 * Aufrufer:  DNG01_handler() ueber g_dng_handlers (seg076), die Truhenfunktionen ueber
 *            g_dng01_specialchests und loot_special_chest() (seg092).
 * Daten:     g_dng01_specialchests[9] (Truhen mit Schloss, Falle, Inhalt), g_dng01_str_marbo.
 * Besonders: Spalt auf (5,14,7): Kartenindex "32 * y" statt "16 * y" (Bright-Eyes),
 *            Klettertest immer mit Held 0. DNG01_chest06_loot(): Text 14 bekommt eine Zahl
 *            statt eines Namens fuer %s. Untergangsmeldung fehlt (Original-Bug 30).
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg077.c
 */
#include <stdio.h>
#include <string.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg003.h"
#include "seg004.h"
#include "seg007.h"
#include "seg025.h"
#include "seg026.h"
#include "seg027.h"
#include "seg032.h"
#include "seg075.h"
#include "seg076.h"
#include "seg077.h"
#include "seg092.h"
#include "seg095.h"
#include "seg096.h"
#include "seg097.h"
#include "seg103.h"
#include "seg105.h"

struct struct_chest g_dng01_specialchests[9] = {
	{ DNG_POS(0,12,9),	(signed char)ITEM_ID_GOLDSCHLUESSEL,	use_key_on_chest,	chest_protected_normal, DNG01_chest00_loot,	 0,    0, 0 },
	{ DNG_POS(1,5,8),	2,					use_lockpicks_on_chest,	chest_closed,		DNG01_chest01_loot,	 0,    0, 0 },
	{ DNG_POS(1,14,8),    	3,					use_lockpicks_on_chest,	chest_protected_heavy,	DNG01_chest02_loot,	 0,    0, 0 },
	{ DNG_POS(3,5,5),    	0,					NULL,			NULL,			DNG01_chest03_loot,	 0,    0, 0 },
	{ DNG_POS(3,13,4),    	6,					use_lockpicks_on_chest,	chest_cursed, 		NULL,			 0, 5000, 0 },
	{ DNG_POS(3,14,6),    	3,					use_lockpicks_on_chest,	chest_fulminictus,	DNG01_chest05_loot,	 0,    0, 0 },
	{ DNG_POS(3,13,11),    	1,					DNG01_chest06_open,	DNG01_chest06_trap,	DNG01_chest06_loot,	10,    0, 0 },
	{ DNG_POS(4,13,6),    	1,					DNG01_chest07_open,	NULL,			DNG01_chest07_loot,	 0,    0, 0 },
	{ -1,			0,					NULL,			NULL,			NULL,			 0,    0, 0 }
}; // ds:0x9314
const char g_dng01_str_marbo[6] = "MARBO"; // ds:0x93d1

/**
 * \brief   dungeon handler of the death ship
 */
signed int DNG01_handler(void)
{
	signed int target_pos;
	signed int i;
	struct struct_hero *hero;


	target_pos = DNG_POS(gs_dungeon_level, gs_x, gs_y);

	if (target_pos == DNG_POS(0,1,8))
	{
		DNG_pitfall_add_pit(6);

	} else if (target_pos == DNG_POS(1,3,9))
	{
		gs_x--;
		DNG_pitfall_add_pit(6);

	} else if (target_pos == DNG_POS(1,14,5))
	{
		gs_y++;
		DNG_pitfall_add_pit(6);

	} else if (target_pos == DNG_POS(2,7,6) || target_pos == DNG_POS(2,7,9))
	{
		DNG01_pitfall();

	} else if (target_pos == DNG_POS(3,14,7))
	{
		DNG_pitfall_add_pit(6);

	} else if (target_pos == DNG_POS(4,7,6))
	{
		DNG_pitfall_add_pit(6);
		gs_y++;

	} else if ((target_pos == DNG_POS(1,13,5)) && (target_pos != gs_dng_pos_bak) && !gs_dng01_sabre_taken)
	{
		sprintf(g_text_output_buf, get_ttx(528), GUI_name_inflect_with_article( /* "HIER LIEGT %s.@@WOLLT IHR %s MITNEHMEN?" */
			INFLECT_INDEFINITE_ARTICLE | INFLECT_SINGULAR | INFLECT_1ST_CASE,
			ITEM_ID_SAEBEL,
			INFLECT_NAME_TYPE_ITEM
		), (char*)GUI_grammar_name_to_personal_pronoun(
			INFLECT_SINGULAR | INFLECT_4TH_CASE,
			ITEM_ID_SAEBEL,
			INFLECT_NAME_TYPE_ITEM
		));

		/* ITEM: get a SABRE */
		if (GUI_bool(g_text_output_buf) && give_new_item_to_group(ITEM_ID_SAEBEL, 1, 1)) {
			gs_dng01_sabre_taken = 1;
		}

	} else if ((target_pos == DNG_POS(3,2,9)) && (target_pos != gs_dng_pos_bak) && !gs_dng01_crossbow_taken)
	{
		sprintf(g_text_output_buf, get_ttx(528), GUI_name_inflect_with_article( /* "HIER LIEGT %s.@@WOLLT IHR %s MITNEHMEN?" */
			INFLECT_INDEFINITE_ARTICLE | INFLECT_SINGULAR | INFLECT_1ST_CASE,
			ITEM_ID_ARMBRUST,
			INFLECT_NAME_TYPE_ITEM
		), (char*)GUI_grammar_name_to_personal_pronoun(
			INFLECT_SINGULAR | INFLECT_4TH_CASE,
			ITEM_ID_ARMBRUST,
			INFLECT_NAME_TYPE_ITEM
		));

		/* ITEM: get a CROSSBOW */
		if (GUI_bool(g_text_output_buf) && give_new_item_to_group(ITEM_ID_ARMBRUST, 1, 1)) {

			gs_dng01_crossbow_taken = 1;
		}

	} else if ((target_pos == DNG_POS(4,2,9)) && (target_pos != gs_dng_pos_bak) && !gs_dng01_amulet_taken)
	{
		/* ITEM: a magic AMULET */
		if (GUI_bool(get_tx(7)) && give_new_item_to_group(ITEM_ID_AMULETT__FLIM_FLAM, 1, 1)) /* "ALS IHR DEN RAUM BETRETET, SCHLÄGT EUCH FAULIGER GESTANKENTGEGEN. I..." */
		{
			gs_dng01_amulet_taken = 1;
			gs_gods_estimation[GOD_ID_BORON] -= 100L;
		}

	} else if ((target_pos == DNG_POS(4,13,6)) && (target_pos != gs_dng_pos_bak))
	{
		loot_special_chest(0);

	} else if (g_visual_square_vals[1] == 2)
	{
		/* TRAP: rats; on squares with value 2 (corridor with flag bit 1), every 10 minutes
		 * each hero of the group is bitten with probability 10% and loses 2 LE */
		if (mod_day_timer(MINUTES(10)))
		{
			hero = get_hero(0);
			for (i = 0; i <= 6; i++, hero++)
			{
				if ((random_schick(100) <= 10) &&
					(hero->typus != HERO_TYPE_NONE) &&
					(hero->group_id == gs_active_group_id))
				{
					sub_hero_le(hero, 2);

					sprintf(g_text_output_buf, get_tx(12), hero->alias); /* "%s WURDE VON EINER RATTE GEBISSEN." */

					GUI_output(g_text_output_buf);
				}
			}
		}

	} else if ((target_pos == DNG_POS(5,1,8)) && !gs_dng01_ardora_freed)
	{
		/* NPC: ARDORA */
		g_textbox_width = 7;

		load_in_head(21);

		if (GUI_dialogbox((unsigned char*)g_dtp2, get_ttx(756), get_tx(8), 2, get_ttx(2), get_ttx(3)) == 1) /* "ARDORA VON GREIFENFURT" / "DEN ZWÖLFEN SEI DANK! MEIN NA..." / "JA" / "NEIN" */
		{
			g_textbox_width = 3;

			GUI_dialogbox((unsigned char*)g_dtp2, get_ttx(756), get_tx(9), 0); /* "ARDORA VON GREIFENFURT" / "DANN LASST UNS ANS WERK GEHEN!" */

			if (get_hero(6)->typus != HERO_TYPE_NONE)
			{
				remove_npc(get_hero(6)->npc_id + 19, 31, get_hero(6)->npc_id + ARCHIVE_FILE_NSC_LTX, NULL, NULL);
			}

			add_npc(ARCHIVE_FILE_NSC_LTX + 4);
		} else {
			g_textbox_width = 3;

			GUI_dialogbox((unsigned char*)g_dtp2, get_ttx(756), get_tx(10), 0); /* "ARDORA VON GREIFENFURT" / "WIE IHR MEINT. SOLLTET IHR JEMALS MEINE DI..." */
		}

		gs_dng01_ardora_freed = 1;

		add_hero_ap_all(20);

	} else if ((target_pos == DNG_POS(5,14,7)) && (target_pos != gs_dng_pos_bak))
	{
		if (GUI_bool(get_tx(11))) /* "ÜBER EUCH KÖNNT IHR EINEN SCHMALEN SPALT ENTDECKEN.@@WOLLTIHR DEN S..." */
		{
			/* check if a ROPE LADDER or a ROPE is available */
			/* Original-Bug: Why not check for a mage with staffspell level >= 3? */
			if (get_first_hero_with_item(ITEM_ID_SEIL) != -1 || get_first_hero_with_item(ITEM_ID_STRICKLEITER) != -1)
			{
				/* Original-Bug: better get_first_hero_available_in_group() */
#if FIX_0093
				/* Fix 93: der erste handlungsfaehige Held klettert */
				if (test_talent(get_first_hero_available_in_group(), TA_KLETTERN, 0) > 0)
#else
				if (test_talent(get_hero(0), TA_KLETTERN, 0) > 0)
#endif
				{
					/* Original-Bug: '32 * ' should probably be '16 *'. */
#if FIX_0094
					/* Fix 94: Dungeonkarten sind 16 Felder breit */
					g_dng_map[16 * gs_y + gs_x] = DNG_TILE_PIT_IN_CEILING << 4;
#else
					g_dng_map[32 * gs_y + gs_x] = DNG_TILE_PIT_IN_CEILING << 4;
#endif

					DNG_dec_level();

					/* Original-Bug: '32 * ' should probably be '16 *'. */
#if FIX_0094
					g_dng_map[16 * gs_y + gs_x] = DNG_TILE_PIT << 4;
#else
					g_dng_map[32 * gs_y + gs_x] = DNG_TILE_PIT << 4;
#endif

					gs_y++;
				}
			} else {
				GUI_output(get_ttx(529)); /* "DIE GRUPPE HAT LEIDER KEIN SEIL DABEI!" */
			}
		}

	} else if ((target_pos == DNG_POS(2,8,11)) && (target_pos != gs_dng_pos_bak))
	{
		if (GUI_bool(get_tx(24))) /* "WOLLT IHR DAS TOTENSCHIFF WIEDER VERLASSEN?" */
		{
			leave_dungeon();
			gs_town_id = gs_travel_destination_town_id;
			gs_x = gs_travel_destination_x;
			gs_y = gs_travel_destination_y;
			gs_town_loc_type = LOCTYPE_NONE;
			gs_viewdir = REVERSE_VIEWDIR(gs_travel_destination_viewdir);

			GUI_output(get_tx(15)); /* "MIT TIEFEN ZÜGEN ATMET IHR DIE SEELUFT EIN, ALS IHR DAS SCHIFF WIED..." */

			if
#if !FIX_0095
			/* Original-Bug 30:
			 * After killing the demon on the death ship, it begins to sink. The party has 30 remaining steps to leave the ship. Succeeding in this, the corresponding message of the sinking death ship (present in the data) is not displayed.
			 * Apparently, the bug is not present in the floppy version. However, there was the bug that an activated dungeon death trap will not be deactivated when leaving the dungeon. Fixing this bug in the CD-version,  Attic has introduced this new (and much less critical) bug.
			 * reported 2014-04-15 by Alrik at https://www.crystals-dsa-foren.de/showthread.php?tid=4589&pid=131934#pid131934 and https://www.crystals-dsa-foren.de/showthread.php?tid=4589&pid=131938#pid131938
			 * fixed 2016-03-06 by NRS https://www.crystals-dsa-foren.de/showthread.php?tid=4589&pid=145465#pid145465

			 * The problem is that 'gs_deathtrap' has been resetted to 0 in the function leave_dungeon() called above. */

				(gs_deathtrap == 1 && gs_deathtrap_steps != 0)
#else
				/* Fix 95: leave_dungeon() hat gs_deathtrap schon geloescht (Bright-Eyes-Fix zu Original-Bug 30) */
				(gs_quest_deadship_done == 1)
#endif
			{
				load_ani(18);
				init_ani(1);
				disable_ani();
				GUI_output(get_tx(16)); /* "PLÖTZLICH BEGINNT DAS SCHIFF HEFTIG ZU KNARREN UND SCHLIESSLICH KÖN..." */
			} else {
				GUI_output(get_tx(17)); /* "ERSCHÖPFT SCHLAFT IHR SOFORT AN DECK DECK EURES SCHIFFES EIN UND WA..." */
			}

			g_special_screen = 1;
			timewarp(HOURS(6));
			g_fading_state = 3;
		}
	}

	gs_dng_pos_bak = target_pos;

	return 0;
}

void DNG01_chest01_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng01_chest1_content;

	loot_simple_chest(chest);

	chest->loot.content = ptr_bak;
}

void DNG01_chest02_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng01_chest2_content;

	loot_simple_chest(chest);

	chest->loot.content = ptr_bak;
}

void DNG01_chest03_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng01_chest3_content;

	loot_simple_chest(chest);

	chest->loot.content = ptr_bak;
}

void DNG01_chest05_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng01_chest5_content;

	loot_simple_chest(chest);

	chest->loot.content = ptr_bak;
}

/**
 * \brief   the remains of an adventurer (corpse), looted only once (gs_dng01_corpse_looted)
 */
void DNG01_chest07_open(struct struct_chest* chest)
{
	loot_corpse(chest, get_tx(4), &gs_dng01_corpse_looted); /* "IN DER ECKE LIEGEN DIE TRAURIGEN ÜBERRESTE EINES EHEMALSSTOLZEN ABE..." */
}

void DNG01_chest07_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng01_chest7_content;

	loot_chest(chest, get_tx(5), get_tx(6)); /* "BEIM UNTERSUCHEN DER ÜBERRESTE FINDET IHR..." / "AUSSER EINIGEN BLANKEN KNOCHEN GIBT ES DA..." */

	chest->loot.content = ptr_bak;
}

void DNG01_chest06_loot(struct struct_chest* chest)
{
#if FIX_0097
	/* Fix 97 (Teil 2): den Goldschluessel nur einmal (die Truhe gilt nach Fix 97 als offen)
	 * Enabling the extra loot in function DNG01_chest06_open() (formerly called DNG01_chest6_x1) also causes the special chest handler to call this function instead of DNG01_chest06_open().
	   Without this additional check the player will always get another golden key and the same text as the first time the chest is successfully opened.*/
	if (!gs_dng01_key_taken)
	{
#endif
	/* ITEM: the GOLDEN KEY */
	give_new_item_to_group(ITEM_ID_GOLDSCHLUESSEL, 1, 1);

	/* Original-Bug: The string 14 from SHIP.DTX needs a pointer to the name of the hero, not an integer.
	 */
#if FIX_0096
	/* Fix 96: den Namen statt der Zahl 10 einsetzen (Bright-Eyes-Fix) */
	sprintf(g_text_output_buf, get_tx(14), (get_first_hero_available_in_group())->alias); /* "LEISE KNARREND ÖFFNET SICH, WIE VON GEISTERHAND,DIE TRUHE. %s ENTNI..." */
#else
	sprintf(g_text_output_buf, get_tx(14), 10); /* "LEISE KNARREND ÖFFNET SICH, WIE VON GEISTERHAND,DIE TRUHE. %s ENTNI..." */
#endif
	print_msg_with_first_hero(g_text_output_buf);

	gs_dng01_key_taken = 1;

#if FIX_0097
	/* We also need to close the if-block...*/
	}
	else 
	{
		/* ... and tell the player that the chest is now empty.*/
		GUI_output(get_ttx(522)); /* "DIE TRUHE IST LEER!" */
	}
#endif
}

void DNG01_chest06_trap(void)
{
	GUI_output(get_tx(3)); /* "<DIESE ANTWORT WAR LEIDER FALSCH! KOMMT SPÄTER WIEDER, WENNIHR DIE..." */
}

/**
 * \brief   riddle chest: the heroes must type the name of the one who cursed the ship (MARBO)
 *
 *          correct answer -> DNG01_chest06_loot() (golden key), wrong answer -> trap (message)
 */
void DNG01_chest06_open(struct struct_chest* chest)
{
	if (!gs_dng01_key_taken)
	{
		GUI_input(get_tx(2), 10); /* "<SPRICH DEN NAMEN DESJENIGEN, DER UNS VERFLUCHT HAT, UND DUWIRST DE..." */

		/* compare if the user wrote MARBO */
		if (!strcmp(g_text_input_buf, g_dng01_str_marbo))
		{
			// correct answer
			chest->loot.func(chest);

			/* Original-Bug: enable the extra loot, here 10 AP */
#if FIX_0097
			/* Fix 97: die Zusatzbelohnung (10 AP) freischalten (Bright-Eyes-Fix) */
			g_get_extra_loot = 1;
#endif
		} else if (chest->trap){

			// wrong answer
			chest->trap();
		}

	} else {
		GUI_output(get_ttx(522)); /* "DIE TRUHE IST LEER!" */
	}
}

/**
 * \brief   chest opened with the golden key: the demon appears, the death trap starts
 *          (30 steps to leave the ship) and the fight FIGHT_ID_SHIP10_1 begins
 */
void DNG01_chest00_loot(struct struct_chest* chest)
{
	if (!gs_deadship_final && GUI_bool(get_tx(1))) /* "UNTER ÜBLEM GESTANK LÖST SICH EINE GEISTERHAFTE ERSCHEINUNGAUS DER..." */
	{
		gs_deadship_final = 1;

		/* enable deathtrap. the ship begins to sink... */
		gs_deathtrap_steps = 30;
		gs_deathtrap = 1;

		/* fight the demon */
		g_fig_escape_position[NORTH] = g_fig_escape_position[EAST] = g_fig_escape_position[SOUTH] = g_fig_escape_position[WEST] = DNG_POS_DIR(1,13,10,WEST);

		if (!do_fight(FIGHT_ID_SHIP10_1))
		{
			add_hero_ap_all(50);
		}

		/* this quest is done */
		gs_quest_deadship_done = 1;
	}
}
