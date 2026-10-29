/*
 * seg086 - Dungeon "Piratenhoehle" auf Manrek (DNG11)
 *
 * Inhalt:    DNG11_handler() steuert die Piratenhoehle: Suppenkessel (LE +1, manchmal
 *            kommen Piraten), zwei Wasserfaesser, Hebel mit fuenf Stellungen (bestimmt,
 *            welche Geheimtuer sich oeffnen laesst; Stellung 4 gibt die Tuer (12,8) frei,
 *            siehe DNG_door in seg076), Swafnir-Statue, die nach einem Opfer (Dreizack oder
 *            Netz, seg105) den Weg weist, zwei Feuerfallen, Lagerraeume, das Boot (die
 *            Gruppe ertrinkt im Sturm), drei Geheimtueren und der Ausgang. Dazu 14 Truhen.
 * Aufrufer:  DNG11_handler() ueber g_dng_handlers (seg076), DNG11_fight_intro() aus
 *            GUI_print_fight_intro_msg() (seg097), die Truhen ueber g_dng11_specialchests
 *            (seg092).
 * Daten:     g_dng11_specialchests[15] (Proviant der Truhen 0 und 1 im Feld food).
 * Besonders: Geheimtueren: bei falscher Hebelstellung wird test_result nicht gesetzt
 *            (Kurzschluss von "&&") und danach uninitialisiert abgefragt.
 *            gs_dng11_firetrap1_active wird nie gesetzt.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg086.c
 */

#include <stdio.h>
#include <string.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg004.h"
#include "seg007.h"
#include "seg025.h"
#include "seg027.h"
#include "seg032.h"
#include "seg075.h"
#include "seg076.h"
#include "seg086.h"
#include "seg092.h"
#include "seg096.h"
#include "seg097.h"
#include "seg103.h"
#include "seg113.h"

struct struct_chest g_dng11_specialchests[15] = {
	{ DNG_POS(0,11,12),	0, NULL,			NULL,			DNG11_chest00_loot, 0, 0, 50 },
	{ DNG_POS(0,12,12),	0, NULL,			NULL,			DNG11_chest01_loot, 0, 0, 21 },
	{ DNG_POS(0,14,12),	6, use_lockpicks_on_chest,	chest_crossbow_bolts,	DNG11_chest02_loot, 0, 0, 0 },
	{ DNG_POS(0,14,14),	6, use_lockpicks_on_chest,	chest_crossbow_bolts,	DNG11_chest03_loot, 0, 0, 0 },
	{ DNG_POS(0,1,13),	0, NULL,			NULL,			DNG11_chest04_loot, 0, 230, 0 },
	{ DNG_POS(0,1,10),	3, use_lockpicks_on_chest,	NULL,			DNG11_chest05_loot, 0, 680, 0 },
	{ DNG_POS(0,1,7),	0, NULL,			NULL,			DNG11_chest06_loot, 0, 0, 0 },
	{ DNG_POS(0,1,4),	4, use_lockpicks_on_chest,	NULL,			DNG11_chest07_loot, 0, 440, 0 },
	{ DNG_POS(0,1,1),	0, NULL,			NULL,			DNG11_chest08_loot, 0, 0, 0 },
	{ DNG_POS(0,1,2),	0, NULL,			NULL,			DNG11_chest09_loot, 0, 0, 0 },
	{ DNG_POS(0,6,10),	4, use_lockpicks_on_chest,	chest_ignifax_heavy,	DNG11_chest10_loot, 0, 2000, 0 },
	{ DNG_POS(0,11,5),	6, use_lockpicks_on_chest,	chest_ignifax_brutal,	DNG11_chest11_loot, 0, 10000, 0 },
	{ DNG_POS(0,12,5),	6, use_lockpicks_on_chest,	chest_ignifax_brutal,	DNG11_chest12_loot, 0, 0, 0 },
	{ DNG_POS(0,9,5),	4, use_lockpicks_on_chest,	NULL,			DNG11_chest13_loot, 0, 0, 0 },
	{     -1, 0, NULL, NULL, NULL, 0, 0, 0 }
}; // ds:0x9854

/**
 * \brief   dungeon handler of the pirate cave on Manrek
 */
signed int DNG11_handler(void)
{
	signed int target_pos;
	signed int test_result;
	signed int answer;
	signed int tw_bak;
	struct struct_hero *hero;
	uint8_t *amap_ptr;

	amap_ptr = g_dng_map;

	tw_bak = g_textbox_width;
	g_textbox_width = 7;

	target_pos = DNG_POS(gs_dungeon_level, gs_x, gs_y);

	hero = get_first_hero_available_in_group();

	if (target_pos == DNG_POS(0,6,12) && target_pos != gs_dng_pos_bak && gs_dng11_soup_supply)
	{
		if (GUI_radio(get_tx(1), 2, get_tx(2), get_tx(3)) == 1) /* "ÜBER EINEM GROSSEN FEUER HÄNG..." / "EUCH DEN MAGEN VOLLSCHLAGEN?" / "DAS ESSEN IGNORIEREN?" */
		{
			add_group_le(1);

			gs_dng11_soup_supply--;

			if (random_schick(100) <= 50)
			{
				g_fig_escape_position[NORTH] = g_fig_escape_position[EAST] = g_fig_escape_position[SOUTH] = g_fig_escape_position[WEST] = DNG_POS_DIR(0,8,15,SOUTH);

				do_fight(FIGHT_ID_F131_01B);
			}
		}

	} else if (target_pos == DNG_POS(0,11,14) && target_pos != gs_dng_pos_bak)
	{
		DNG_waterbarrel(&gs_dng11_waterbarrel1);

	} else if (target_pos == DNG_POS(0,12,14) && target_pos != gs_dng_pos_bak)
	{
		DNG_waterbarrel(&gs_dng11_waterbarrel2);

	} else if (target_pos == DNG_POS(0,6,9) && target_pos != gs_dng_pos_bak) {

		answer = GUI_radio(get_tx(4), 6, get_tx(5), get_tx(6), get_tx(7), /* "VOR EUCH RAGT EIN HEBEL AUS D..." / "POSITION 1" / "POSITION 2" / "POSITION 3" */
						 get_tx(8), get_tx(9), get_tx(10)); /* "POSITION 4" / "POSITION 5" / "HEBEL IN RUHE LASSEN" */

		/* position 1: secret door (5,7), 2: (8,11), 3: all three secret doors, 4: door (12,8)
		 * (seg076), 5: secret door (10,8). Initial value 2. */
		gs_dng11_lever_flag = 0;

		if (answer >= 1 && answer <= 5)
		{
			gs_dng11_lever_flag = (signed char)answer;

			GUI_output(get_tx(11)); /* "IHR BRINGT DEN HEBEL IN POSITION, ABER NICHTS GESCHIEHT." */
		}

	} else if (target_pos == DNG_POS(0,9,9))
	{
		/* statue of Swafnir: after a sacrifice (trident or net dropped here, see seg105) it tells
		 * the way; this switches the fire trap at (0,11,8) off and the one at (0,12,10) on */
		if (target_pos != gs_dng_pos_bak && !gs_dng11_swafnir_hint)
		{
			GUI_output(get_tx(12)); /* "HIER STEHT EINE STATUE AUF EINEM KLEINEN SOCKEL. DIE STATUE STELLTE..." */

		} else if (gs_dng11_swafnir_sacrifice && !gs_dng11_swafnir_hint)
		{
			gs_dng11_swafnir_hint = 1;
			gs_dng11_firetrap2_active = gs_dng11_swafnir_sacrifice = 0;

			GUI_output(get_tx(13)); /* "<GEHT DEN GANG ZU MEINER RECHTEN, UM ZUM SCHATZ ZU GELANGEN UND DEN..." */
		}

	} else if (target_pos == DNG_POS(0,12,10) && target_pos != gs_dng_pos_bak && gs_dng11_firetrap1_flag)
	{
		/* REMARK: gs_dng11_firetrap1_active is never set, so this trap is active only after the hint */
		if (gs_dng11_swafnir_hint || gs_dng11_firetrap1_active)
		{
			GUI_output(get_tx(15)); /* "PLÖTZLICH SCHIESST UM EUCH HERUM FLÜSSIGES FEUER AUS DEM BODEN!" */

			gs_dng11_firetrap1_flag--;

			hero = get_hero(0);
			for (answer = 0; answer <= 6; answer++, hero++)
			{
				if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
					!hero->flags.dead)
				{
					sub_hero_le(hero, test_attrib(hero, ATTRIB_GE, 4) <= 0 ? dice_roll(6, 6, 6) : dice_roll(3, 6, 3));
				}
			}
		}

	} else if (target_pos == DNG_POS(0,11,8) && target_pos != gs_dng_pos_bak && gs_dng11_firetrap2_flag)
	{
		/* gs_dng11_firetrap2_active: 1 (initial) = trap active, set to 0 by the statue's hint */
		if (gs_dng11_firetrap2_active)
		{
			GUI_output(get_tx(15)); /* "PLÖTZLICH SCHIESST UM EUCH HERUM FLÜSSIGES FEUER AUS DEM BODEN!" */

			gs_dng11_firetrap2_flag--;

			hero = get_hero(0);
			for (answer = 0; answer <= 6; answer++, hero++)
			{
				if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !hero->flags.dead)
				{
					sub_hero_le(hero, test_attrib(hero, ATTRIB_GE, 4) <= 0 ? dice_roll(6, 6, 6) : dice_roll(3, 6, 3));
				}
			}
		}

	} else if (target_pos == DNG_POS(0,11,3) && target_pos != gs_dng_pos_bak && gs_viewdir == EAST)
	{
		GUI_output(get_tx(17)); /* "DIES HIER IST EIN LAGER FÜR ALLERLEI SCHIFFSBEDARF." */

	} else if (((target_pos == DNG_POS(0,12,3) && gs_viewdir == EAST) ||
			(target_pos == DNG_POS(0,11,2) && gs_viewdir == NORTH)) &&
			target_pos != gs_dng_pos_bak)
	{
			GUI_output(get_tx(16)); /* "IN MEHREREN TRUHEN UND FÄSSERN, AUF REGALEN UND IN SCHRÄNKEN STAPEL..." */

	} else if ((target_pos == DNG_POS(0,6,0) || target_pos == DNG_POS(0,7,0) || target_pos == DNG_POS(0,8,0) || target_pos == DNG_POS(0,9,0)) &&
			target_pos != gs_dng_pos_bak)
	{
		/* boat. if entered, the complete group is drowned */
		if (GUI_radio(get_tx(18), 2, get_tx(19), get_tx(20)) == 1) /* "HIER ÖFFNET SICH DIE HÖHLE ZU..." / "DIE HÖHLE VERLASSEN?" / "DAS BOOT WEITER DÜMPELN LASSEN?" */
		{
			GUI_output(get_tx(21)); /* "NACH EINIGEN MINUTEN ERREICHT IHR DIE OFFENE SEE." */
			GUI_output(get_tx(22)); /* "NACH EINIGEN WEITEREN MINUTEN WIRD DIE SEE NOCH OFFENER..." */

			load_ani(18);
			init_ani(0);

			GUI_output(get_tx(23)); /* "UND SCHLIESSLICH TREIBT EURE NUSSSCHALE GENAU IN DAS AUGE EINESHEFT..." */

			hero = get_hero(0);
			for (answer = 0; answer <= 6; answer++, hero++)
			{
				if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id))
				{
					hero_disappear(hero, answer, -1);
				}
			}

			disable_ani();

		} else {
			gs_x = gs_x_bak;
			gs_y = gs_y_bak;
		}

	} else if (target_pos == DNG_POS(0,8,12) &&
			// possibly a bug. position (8,12) might indeed be (8,11)
			// see https://www.crystals-dsa-foren.de/showthread.php?tid=1373&pid=96541#pid96541
			// REMARK: (8,12) is consistent: facing north from (8,12) the group looks at the secret door
			// (8,11), which is opened below. The other two doors work the same way ((4,7)->(5,7), (9,8)->(10,8)).
			(target_pos != gs_dng_pos_bak || gs_viewdir != gs_viewdir_bak) &&
			gs_viewdir == NORTH &&
			gs_dng11_secretdoor1_flag != 2)
	{
		if (gs_dng11_secretdoor1_flag || test_talent(hero, TA_SINNESSCHAERFE, 8) > 0)
		{
			gs_dng11_secretdoor1_flag = 1;

			sprintf(g_dtp2,	get_tx(26), hero->alias); /* "BEI GENAUEM HINSEHEN, ENTDECKT %s EINE GEHEIMTÜRE IN DER WAND." */

			/* Original-Bug: with a wrong lever position the "&&" skips the assignment of test_result,
			 * which is then read uninitialized below (same for the other two secret doors). */
			sprintf(g_text_output_buf,
				(char*)((gs_dng11_lever_flag == 2 || gs_dng11_lever_flag == 3) && (test_result = test_talent(hero, TA_SCHLOESSER, 5)) > 0 ? get_tx(27) : get_tx(28)), /* "@NACH KURZER ZEIT GELINGT ES %s SOGAR, SIE..." / "@LEIDER GELINGT ES %s ALLERDINGS NICHT, SI..." */
				GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_3RD));

			strcat(g_dtp2, g_text_output_buf);
			GUI_output(g_dtp2);

			if (test_result > 0)
			{
				amap_ptr[MAP_POS(8,11)] &= (DNG_TILE_CORRIDOR << 4) + 0x0f;

				gs_dng11_secretdoor1_flag = 2;

				DNG_update_pos();
			}

			gs_viewdir_bak = gs_viewdir;
		}

	} else if (target_pos == DNG_POS(0,4,7) &&
			(target_pos != gs_dng_pos_bak || gs_viewdir != gs_viewdir_bak) &&
			gs_viewdir == EAST &&
			gs_dng11_secretdoor2_flag != 2)
	{
		if (gs_dng11_secretdoor2_flag != 0 || test_talent(hero, TA_SINNESSCHAERFE, 4) > 0)
		{
			gs_dng11_secretdoor2_flag = 1;

			sprintf(g_dtp2, get_tx(26), hero->alias); /* "BEI GENAUEM HINSEHEN, ENTDECKT %s EINE GEHEIMTÜRE IN DER WAND." */

			sprintf(g_text_output_buf,
				(char*)((gs_dng11_lever_flag == 1 || gs_dng11_lever_flag == 3) && (test_result = test_talent(hero, TA_SCHLOESSER, 3)) > 0 ? get_tx(27) : get_tx(28)), /* "@NACH KURZER ZEIT GELINGT ES %s SOGAR, SIE..." / "@LEIDER GELINGT ES %s ALLERDINGS NICHT, SI..." */
				GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_3RD));

			strcat(g_dtp2, g_text_output_buf);

			GUI_output(g_dtp2);

			if (test_result > 0)
			{
				amap_ptr[MAP_POS(5,7)] &= (DNG_TILE_CORRIDOR << 4) + 0x0f;

				gs_dng11_secretdoor2_flag = 2;

				DNG_update_pos();
			}

			gs_viewdir_bak = gs_viewdir;
		}

	} else if (target_pos == DNG_POS(0,9,8) &&
			(target_pos != gs_dng_pos_bak || gs_viewdir != gs_viewdir_bak) &&
			gs_viewdir == EAST &&
			gs_dng11_secretdoor3_flag != 2)
	{
		if (gs_dng11_secretdoor3_flag != 0 || test_talent(hero, TA_SINNESSCHAERFE, 6) > 0)
		{
			gs_dng11_secretdoor3_flag = 1;

			sprintf(g_dtp2,	get_tx(26), hero->alias); /* "BEI GENAUEM HINSEHEN, ENTDECKT %s EINE GEHEIMTÜRE IN DER WAND." */

			sprintf(g_text_output_buf,
				(char*)((gs_dng11_lever_flag == 5 || gs_dng11_lever_flag == 3) && (test_result = test_talent(hero, TA_SCHLOESSER, 5)) > 0 ? get_tx(27) : get_tx(28)), /* "@NACH KURZER ZEIT GELINGT ES %s SOGAR, SIE..." / "@LEIDER GELINGT ES %s ALLERDINGS NICHT, SI..." */
				GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_3RD));

			strcat(g_dtp2, g_text_output_buf);

			GUI_output(g_dtp2);

			if (test_result > 0)
			{
				amap_ptr[MAP_POS(10,8)] &= (DNG_TILE_CORRIDOR << 4) + 0x0f;

				gs_dng11_secretdoor3_flag = 2;

				DNG_update_pos();
			}

			gs_viewdir_bak = gs_viewdir;
		}

	} else if (target_pos == DNG_POS(0,8,15) && target_pos != gs_dng_pos_bak)
	{
		leave_dungeon();

		gs_town_id = gs_travel_destination_town_id;
		gs_x = gs_travel_destination_x;
		gs_y = gs_travel_destination_y;
		gs_town_loc_type = LOCTYPE_NONE;
		gs_viewdir = REVERSE_VIEWDIR(gs_travel_destination_viewdir);

		sprintf(g_dtp2, get_tx(29), get_ttx(gs_journey_destination_town_id + 0xeb)); /* "KÜHLE LUFT SCHLÄGT EUCH ENTGEGEN, ALS IHR..." / TEXT.LTX 236..287: Ortsname */
		GUI_output(g_dtp2);

		timewarp(HOURS(3));

		g_fading_state = 3;
	}

	g_textbox_width = tw_bak;
	gs_dng_pos_bak = target_pos;

	return 0;
}

void DNG11_chest00_loot(struct struct_chest* chest)
{
	if (!gs_dng11_proviant1_flag)
	{
		/* a chest with 50 food rations (given by loot_special_chest() from the food field of the chest table) */
		GUI_output(get_tx(24)); /* "DIESE TRUHE ENTHÄLT 50 RATIONEN, DIE IHR NATÜRLICH SOFORT AN EUCHNE..." */

		gs_dng11_proviant1_flag = 1;

	} else {

		/* an empty chest */
		GUI_output(get_ttx(522)); /* "DIE TRUHE IST LEER!" */
	}
}

void DNG11_chest01_loot(struct struct_chest* chest)
{
	if (!gs_dng11_proviant2_flag) {

		/* a chest with 21 food rations (given by loot_special_chest() from the food field of the chest table) */
		GUI_output(get_tx(25)); /* "DIESE TRUHE ENTHÄLT 21 RATIONEN, DIE IHR NATÜRLICH SOFORT AN EUCHNE..." */

		gs_dng11_proviant2_flag = 1;

	} else {

		/* an empty chest */
		GUI_output(get_ttx(522)); /* "DIE TRUHE IST LEER!" */
	}
}

void DNG11_chest02_loot(struct struct_chest* chest)
{
        uint8_t* ptr_bak = chest->loot.content;

        chest->loot.content = gs_dng11_chest3;

        loot_simple_chest(chest);

        chest->loot.content = ptr_bak;
}

void DNG11_chest03_loot(struct struct_chest* chest)
{
        uint8_t* ptr_bak = chest->loot.content;

        chest->loot.content = gs_dng11_chest4;

        loot_simple_chest(chest);

        chest->loot.content = ptr_bak;
}

void DNG11_chest04_loot(struct struct_chest* chest)
{
        uint8_t* ptr_bak = chest->loot.content;

        chest->loot.content = gs_dng11_chest5;

        loot_simple_chest(chest);

        chest->loot.content = ptr_bak;
}

void DNG11_chest05_loot(struct struct_chest* chest)
{
        uint8_t* ptr_bak = chest->loot.content;

        chest->loot.content = gs_dng11_chest6;

        loot_simple_chest(chest);

        chest->loot.content = ptr_bak;
}

void DNG11_chest06_loot(struct struct_chest* chest)
{
        uint8_t* ptr_bak = chest->loot.content;

        chest->loot.content = &gs_dng11_chest7;

        loot_simple_chest(chest);

        chest->loot.content = ptr_bak;
}

void DNG11_chest07_loot(struct struct_chest* chest)
{
        uint8_t* ptr_bak = chest->loot.content;

        chest->loot.content = gs_dng11_chest8;

        loot_simple_chest(chest);

        chest->loot.content = ptr_bak;
}

void DNG11_chest08_loot(struct struct_chest* chest)
{
        uint8_t* ptr_bak = chest->loot.content;

        chest->loot.content = gs_dng11_chest9;

        loot_simple_chest(chest);

        chest->loot.content = ptr_bak;
}

void DNG11_chest09_loot(struct struct_chest* chest)
{
        uint8_t* ptr_bak = chest->loot.content;

        chest->loot.content = gs_dng11_chest10;

        loot_simple_chest(chest);

        chest->loot.content = ptr_bak;
}

void DNG11_chest10_loot(struct struct_chest* chest)
{
        uint8_t* ptr_bak = chest->loot.content;

        chest->loot.content = gs_dng11_chest11;

        loot_simple_chest(chest);

        chest->loot.content = ptr_bak;
}

void DNG11_chest11_loot(struct struct_chest* chest)
{
        uint8_t* ptr_bak = chest->loot.content;

        chest->loot.content = gs_dng11_chest12;

        loot_simple_chest(chest);

        chest->loot.content = ptr_bak;
}

void DNG11_chest12_loot(struct struct_chest* chest)
{
        uint8_t* ptr_bak = chest->loot.content;

        chest->loot.content = gs_dng11_chest13;

        loot_simple_chest(chest);

        chest->loot.content = ptr_bak;
}

void DNG11_chest13_loot(struct struct_chest* chest)
{
        uint8_t* ptr_bak = chest->loot.content;

        chest->loot.content = gs_dng11_chest14;

        loot_simple_chest(chest);

        chest->loot.content = ptr_bak;
}

/**
 * \brief   print introductions of dungeon #11 fights
 *
 * \param   fight_id    ID of the fight
 */
void DNG11_fight_intro(const signed int fight_id)
{
	if (fight_id == FIGHT_ID_F131_11_1) {
		GUI_output(get_tx(14)); /* "HIER FÜHRT EINE SCHMALE TREPPE NACH OBEN. VORSICHTIG STEIGT IHR DIE..." */
	}
}
