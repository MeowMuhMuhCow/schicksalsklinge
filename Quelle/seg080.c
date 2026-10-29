/*
 * seg080 - Dungeons "Wolfshoehle" (DNG04) und "Goblinhoehle" (DNG05)
 *
 * Inhalt:    DNG04_handler(): Wolfsbau zwischen Ottarje und Orvil mit drei Leichen zum
 *            Durchsuchen, Spalt mit Rattenbiss und Travia-Amulett, Silberhelm, Laterne,
 *            einsturzgefaehrdete Decke, Kothaufen (CH -5), tiefer Spalt (der erste Held
 *            kann verschwinden), Ratten und Ausgang. DNG05_handler(): Goblinhoehle
 *            zwischen Felsteyn und Orkanger mit Abfallberg (20 Heller, CH -5), totem
 *            Wildschwein (10 Rationen), Fledermaeusen, wehrlosem Goblin (Travia +/-15),
 *            zufaelligem Kampf, Rattenloch (Wundfieber), Hoehlenbaer und Ausgang.
 * Aufrufer:  DNG04/05_handler() ueber g_dng_handlers (seg076), die Leichen ueber
 *            g_dng04_specialchests und loot_special_chest() (seg092), DNG05_fight_intro()
 *            aus GUI_print_fight_intro_msg() (seg097).
 * Daten:     g_dng04_specialchests[4] (drei Leichen), g_dng05_trash_flag (Abfallberg
 *            durchsucht; als g_-Variable vermutlich nicht im Spielstand).
 * Besonders: DNG04_handler(): tiefer Spalt ruft hero_disappear(hero, 0, ...) mit Position 0
 *            statt der echten Position; die zweite if-Kette (Leiche 2, Ratten, Ausgang)
 *            haengt ohne "else" an der ersten. Kothaufen/Abfallberg: Modifikator auf
 *            Held 0 bzw. mit who = 0 (Bright-Eyes-Hinweis).
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg080.c
 */
#include <stdio.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg003.h"
#include "seg004.h"
#include "seg007.h"
#include "seg025.h"
#include "seg027.h"
#include "seg032.h"
#include "seg047.h"
#include "seg080.h"
#include "seg092.h"
#include "seg096.h"
#include "seg097.h"
#include "seg103.h"
#include "seg105.h"
#include "seg113.h"

struct struct_chest g_dng04_specialchests[4] = {
	{ DNG_POS(0,6,7),	1, DNG04_chest00_open, NULL, DNG04_chest00_loot, 0, 0, 0 },
	{ DNG_POS(0,1,12),	1, DNG04_chest01_open, NULL, DNG04_chest01_loot, 0, 0, 0 },
	{ DNG_POS(0,6,1),	1, DNG04_chest02_open, NULL, DNG04_chest02_loot, 0, 0, 0 },
	{     -1,		0, NULL, 	       NULL, NULL,		0, 0, 0 }
}; // ds:0x95ba
int16_t g_dng05_trash_flag = 0; // ds:0x960e, 1 = heap of rubbish in DNG05 searched

/**
 * \brief   dungeon handler of the wolfcave
 */
signed int DNG04_handler(void)
{
	signed int target_pos;
	signed int i; /* dual use: mod_slot, hero_pos */
	signed int tw_bak;
	struct struct_hero *hero;

	tw_bak = g_textbox_width;
	g_textbox_width = 7;

        target_pos = DNG_POS(gs_dungeon_level, gs_x, gs_y);

	if (target_pos == DNG_POS(0,6,7) && target_pos != gs_dng_pos_bak && !gs_dng04_corpse0_flag)
	{
		/* DNG04_corpse0 */
		loot_special_chest(0);

	} else if (target_pos == DNG_POS(0,8,8) && target_pos != gs_dng_pos_bak && !gs_dng04_gap_flag)
	{
		/* do you want to grab into the gap ? */
		do {
			i = GUI_radio(get_tx(2), 2, get_tx(3), get_tx(4)); /* "IN DER RECHTEN HÖHLENWAND KÖN..." / "DEN SPALT SPALT SEIN LASSEN?" / "IN DEN SPALT GREIFEN?" */

		} while (i == -1);

		if (i == 2)
		{
			hero = get_first_hero_available_in_group();

			sprintf(g_dtp2,	get_tx(5), hero->alias); /* "%s WURDE VON EINER RATTE GEBISSEN, FINDET DAFÜR ABER EIN AMULETT." */
			GUI_output(g_dtp2);

			sub_hero_le(hero, 2);

			/* get a magic AMULET */
			give_new_item_to_group(ITEM_ID_AMULETT__TRAVIA, 1, 1);

			gs_dng04_gap_flag = 1;
		}

	} else if (target_pos == DNG_POS(0,5,11) && target_pos != gs_dng_pos_bak && !gs_dng04_helmet_flag)
	{
		/* do you want the SILVER HELMET? */
		if (GUI_bool(get_tx(7))) /* "HIER LIEGT EIN SILBERNER HELM.@WOLLT IHR IHN MITNEHMEN?" */
		{
			/* get a SILVER HELMET */
			if (give_new_item_to_group(ITEM_ID_SILBERHELM, 1, 1))
			{
				/* mark SILVER HELMET as taken */
				gs_dng04_helmet_flag = 1;
			}
		}

	} else if (target_pos == DNG_POS(0,1,12) && target_pos != gs_dng_pos_bak)
	{
		/* DNG04_corpse1 */
		loot_special_chest(0);

	} else if (target_pos == DNG_POS(0,8,3) && target_pos != gs_dng_pos_bak)
	{
		/* the ceiling may drop on your head */
		GUI_output(get_tx(9)); /* "IN DIESEM ABSCHNITT SCHEINT DIE DECKE DER HÖHLE EINSTURZGEFÄHRDET Z..." */

		hero = get_hero(0);
		for (i = 0; i <= 6 ; i++, hero++)
		{
			if ((hero->typus != HERO_TYPE_NONE) &&
				(hero->group_id == gs_active_group_id) &&
				!hero->flags.dead &&
				(test_talent(hero, TA_SCHLEICHEN, 2) <= 0))
			{
				sprintf(g_dtp2,	get_tx(10), hero->alias); /* "%s REMPELT EINEN FELSVORSPRUNG AN...@UND GROSSE STEINBROCKEN, STAUB..." */
				GUI_output(g_dtp2);

				sub_group_le(dice_roll(2, 6, 0));

				break;
			}
		}

	} else if (target_pos == DNG_POS(0,7,3) && target_pos != gs_dng_pos_bak)
	{
		GUI_output(get_tx(11)); /* "UNTER EINEM HAUFEN HERABGESTÜRZTER STEINE RAGT EINE SKELETTHAND HER..." */

	} else if (target_pos == DNG_POS(0,5,5) && target_pos != gs_dng_pos_bak)
	{
		GUI_output(get_tx(12)); /* "AUF DEM BODEN ERKENNT IHR EINE BLUTIGE SCHLEIFSPUR, DIE IN DIE WEST..." */

	} else if (target_pos == DNG_POS(0,10,13) && target_pos != gs_dng_pos_bak)
	{
		/* dig into a heap of dirt? */
		if (GUI_bool(get_tx(13)) && GUI_bool(get_tx(14))) /* "HIER LIEGT EIN GROSSER HAUFEN KOT UND ABFÄ..." / "WIRKLICH?" */
		{
			GUI_output(get_tx(15)); /* "BÄÄÄÄHHH!" */

			i = get_free_mod_slot();

			/* Original-BUG: assumption the leader is at pos 0 */
			/* CH-5 for 1 day */
			set_mod_slot(i, DAYS(1), (uint8_t*)&get_hero(0)->attrib[ATTRIB_CH].current, -5, 0);
		}

	} else if (target_pos == DNG_POS(0,14,14) && target_pos != gs_dng_pos_bak)
	{
		/* the ceiling may drop on your head */
		GUI_output(get_tx(9)); /* "IN DIESEM ABSCHNITT SCHEINT DIE DECKE DER HÖHLE EINSTURZGEFÄHRDET Z..." */

		hero = get_hero(0);
		for (i = 0; i <= 6 ; i++, hero++)
		{
			if ((hero->typus != HERO_TYPE_NONE) &&
				(hero->group_id == gs_active_group_id) &&
				!hero->flags.dead &&
				(test_talent(hero, TA_SCHLEICHEN, 4) <= 0))
			{
				sprintf(g_dtp2, get_tx(10), hero->alias); /* "%s REMPELT EINEN FELSVORSPRUNG AN...@UND GROSSE STEINBROCKEN, STAUB..." */
				GUI_output(g_dtp2);

				sub_group_le(dice_roll(1, 6, 2));

				break;
			}
		}

	} else if (target_pos == DNG_POS(0,10,6) && target_pos != gs_dng_pos_bak && !gs_dng04_lantern_flag)
	{
		/* do you want a LANTERN? */
		if (GUI_bool(get_tx(16))) /* "IN DER NISCHE FINDET IHR EINE ALTE, ABER NOCH FUNKTIONSFÄHIGELATERN..." */
		{
			/* get a LANTERN */
			if (give_new_item_to_group(ITEM_ID_LATERNE__UNLIT, 1, 1))
			{
				/* mark LANTERN as taken */
				gs_dng04_lantern_flag = 1;
			}
		}

	} else if (target_pos == DNG_POS(0,10,2) && target_pos != gs_dng_pos_bak)
	{
		/* a very deep gap: the first time the group is asked. Later the leader is only asked
		 * if he passes a GE test, otherwise he walks to the edge without asking.
		 * At the edge a GE test decides: success -> only a block falls, failure -> the hero is lost. */
		i = 1;
		hero = get_first_hero_available_in_group();

		if (!gs_dng04_deepgap_flag || (test_attrib(hero, ATTRIB_GE, 0) > 0))
		{
			gs_dng04_deepgap_flag = 1;

			i = GUI_bool(get_tx(17)); /* "EIN SPALT IN DER FELSWAND FÜHRT NACH UNTEN, WEITER, VIEL WEITER, AL..." */
		}

		if (i)
		{
			if (test_attrib(hero, ATTRIB_GE, 0) > 0)
			{
				sprintf(g_dtp2,	get_tx(18), hero->alias, /* "UNTER %s LÖST SICH PLÖTZLICH EIN FELSBLOCK UND STÜRZT POLTERND IN D..." */
					GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_3RD),
					GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST));
				GUI_output(g_dtp2);

			} else {

				sprintf(g_dtp2,	get_tx(19), hero->alias, /* "UNTER %s LÖST SICH PLÖTZLICH EIN FELSBLOCK UND STÜRZT IN DIE SCHWAR..." */
					GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST), hero->alias);

				GUI_output(g_dtp2);

				/* Original-Bug: pos must be the position of the hero (here 0 is passed). For a hero
				 * not at position 0 hero_disappear() writes the CHR file of hero 0 (and for the NPC
				 * save_npc() is not called). */
				hero_disappear(hero, 0, -1);
			}
		}
	}

	/* REMARK: this is a second if-chain (no "else" before it), so e.g. the rats below can also bite
	 * on a square where an event of the first chain has just happened. */
	if (target_pos == DNG_POS(0,6,1) && target_pos != gs_dng_pos_bak && !gs_dng04_corpse2_flag)
	{
		/* DNG04_corpse2 */
		loot_special_chest(0);

	} else if ((target_pos == DNG_POS(0,13,6) || target_pos == DNG_POS(0,14,6) || target_pos == DNG_POS(0,14,5)) && target_pos != gs_dng_pos_bak)
	{
		/* Probability of 5% to get bitten by a rat */
		if (mod_day_timer(MINUTES(10)))
		{
			hero = get_hero(0);
			for (i = 0; i <= 6 ; i++, hero++)
			{
				if ((random_schick(100) <= 5) &&
					(hero->typus != HERO_TYPE_NONE) &&
					(hero->group_id == gs_active_group_id) &&
					!hero->flags.dead)
				{
					sub_hero_le(hero, 2);

					sprintf(g_text_output_buf, get_tx(21), hero->alias); /* "%s WIRD VON EINER RATTE GEBISSEN." */
					GUI_output(g_text_output_buf);
				}
			}
		}

	} else if (g_visual_square_vals[1] == 2)
	{
		/* Probability of 10% to get bitten by a rat */
		if (mod_day_timer(MINUTES(10)))
		{
			hero = get_hero(0);
			for (i = 0; i <= 6 ; i++, hero++)
			{
				/* Original-Bug: forgot to check if the hero is dead */
				if ((random_schick(100) <= 10) &&
					(hero->typus != HERO_TYPE_NONE) &&
					(hero->group_id == gs_active_group_id))
				{
					sub_hero_le(hero, 2);

					sprintf(g_text_output_buf, get_tx(25), hero->alias); /* "%s WIRD VON EINER RATTE GEBISSEN!" */
					GUI_output(g_text_output_buf);
				}
			}
		}

	} else if (target_pos == DNG_POS(0,7,15) && target_pos != gs_dng_pos_bak)
	{
		/* the exit of this dungeon */
		leave_dungeon();

		gs_town_id = gs_travel_destination_town_id;
		gs_x = gs_travel_destination_x;
		gs_y = gs_travel_destination_y;
		gs_town_loc_type = LOCTYPE_NONE;
		gs_viewdir = REVERSE_VIEWDIR(gs_travel_destination_viewdir);

		sprintf(g_dtp2, get_tx(26), get_ttx(gs_journey_destination_town_id + 0xeb)); /* "ERSCHÖPFT UND MÜDE VERLASST IHR DEN WOLFSB..." / TEXT.LTX 236..287: Ortsname */
		GUI_output(g_dtp2);

		timewarp(HOURS(2));

		g_fading_state = 3;
	}

	g_textbox_width = tw_bak;
	gs_dng_pos_bak = target_pos;

	return 0;
}

void DNG04_chest00_open(struct struct_chest* chest)
{
	loot_corpse(chest, get_tx(1), (int8_t*)&gs_dng04_corpse0_flag); /* "AN DER WAND LEHNT EIN IN LUMPEN GEKLEIDETES SKELETT. DIE GRÖSSE LÄS..." */
}

void DNG04_chest01_open(struct struct_chest* chest)
{
	loot_corpse(chest, get_tx(8), (int8_t*)&gs_dng04_corpse1_flag); /* "IN DER ECKE LIEGEN MEHRERE RECHT FRISCHE LEICHEN, OFFENSICHTLICH DI..." */
}

void DNG04_chest02_open(struct struct_chest* chest)
{
	loot_corpse(chest, get_tx(20), (int8_t*)&gs_dng04_corpse2_flag); /* "AM ENDE DES GANGES LIEGT DIE ZUSAMMENGEKRÜMMT GESTALT EINES ELFEN,..." */
}

void DNG04_chest00_loot(struct struct_chest* chest)
{
	uint8_t* bak = chest->loot.content;

	chest->loot.content = gs_dng04_chest_corpse0;

	loot_chest(chest, get_tx(22), get_tx(6)); /* "DAS SKELETT TRÄGT NOCH FOLGENDE GEGENSTÄND..." / "HIER GIBT'S NICHTS MEHR ZU HOLEN." */

	chest->loot.content = bak;
}

void DNG04_chest01_loot(struct struct_chest* chest)
{
	uint8_t* bak = chest->loot.content;

	chest->loot.content = gs_dng04_chest_corpse1;

	loot_chest(chest, get_tx(23), get_tx(6)); /* "BEGRABEN UNTER DEN LEIBERN KÖNNT IHR MIT M..." / "HIER GIBT'S NICHTS MEHR ZU HOLEN." */

	chest->loot.content = bak;
}

void DNG04_chest02_loot(struct struct_chest* chest)
{
	uint8_t* bak = chest->loot.content;

	chest->loot.content = gs_dng04_chest_corpse2;

	loot_chest(chest, get_tx(24), get_tx(6)); /* "DER ELF HAT NOCH IMMER SEINE AUSRÜSTUNG BE..." / "HIER GIBT'S NICHTS MEHR ZU HOLEN." */

	chest->loot.content = bak;
}

/**
 * \brief   dungeon handler of a cave
 */
signed int DNG05_handler(void)
{
	signed int target_pos;
	signed int tmp; /* multiple use: mod_slot, answer */
	signed int tw_bak;
	struct struct_hero *hero;

	tw_bak = g_textbox_width;
	g_textbox_width = 7;

	target_pos = DNG_POS(gs_dungeon_level, gs_x, + gs_y);

	if (target_pos == DNG_POS(0,7,14) && target_pos != gs_dng_pos_bak && !g_dng05_trash_flag)
	{
		if (GUI_bool(get_tx(1)) && GUI_bool(get_tx(15))) /* "HIER LIEGT EIN GROSSER BERG VON ABFÄLLEN.@..." / "WIRKLICH?" */
		{
			g_dng05_trash_flag = 1;

			hero = get_first_hero_available_in_group();

			GUI_output(get_tx(16)); /* "BÄÄÄÄHHH!" */

			tmp = get_free_mod_slot();
			/* Original-Bug: who = 0, but hero is the first available hero (compare DNG04 above).
			 * The modification is stored relative to hero 0 and is undone wrongly if hero 0 changes. */
			set_mod_slot(tmp, DAYS(1), (uint8_t*)&hero->attrib[ATTRIB_CH].current, -5, 0);

			add_party_money(20L);
		}

	} else if (target_pos == DNG_POS(0,5,12) && target_pos != gs_dng_pos_bak && !gs_dng05_proviant_flag)
	{
		if (GUI_bool(get_tx(2))) /* "HIER LIEGT EIN TOTES WILDSCHWEIN, OFFENSICHTLICH FRISCH AUSGEWEIDET..." */
		{
			GUI_output(get_tx(3)); /* "IHR KÖNNT NOCH ZEHN RATIONEN AUS DEM SCHWEIN HERAUSSCHNEIDEN, BEVOR..." */

			give_new_item_to_group(ITEM_ID_PROVIANTPAKET, 1, 10);

			gs_dng05_proviant_flag = 1;
		}

	} else if (target_pos == DNG_POS(0,9,7) && target_pos != gs_dng_pos_bak && !gs_dng05_bats_flag)
	{
		if (GUI_bool(get_tx(4))) /* "AN DER DECKE HÄNGEN MEHRERE SELTSAME GEBILDE.@WOLLT IHR SIE EUCH AN..." */
		{
			GUI_output(get_tx(5)); /* "10 WÜTENDE, AUFGESCHRECKTE FLEDERMÄUSE; ALS DIE SICH DIE GEBILDEENT..." */

			gs_dng05_bats_flag = 1;
		}

	} else if (target_pos == DNG_POS(0,3,9) && target_pos != gs_dng_pos_bak && !gs_dng05_god_flag)
	{
		do {
			tmp = GUI_radio(get_tx(6), 2, get_tx(7), get_tx(8)); /* "DER GOBLIN VOR EUCH KANN SICH..." / "KÄMPFEN" / "IHN STEHEN LASSEN?" */

		} while (tmp == -1);

		gs_gods_estimation[GOD_ID_TRAVIA] += (tmp == 1 ? -15 : 15);

		GUI_output(get_tx(9)); /* "DIE SACHE ERLEDIGT SICH OHNEHIN IN JUST DIESEM MOMENT, DENN DER GOB..." */

		gs_dng05_god_flag = 1;

	} else if (target_pos == DNG_POS(0,8,5) && target_pos != gs_dng_pos_bak)
	{
		/* random fight (29%). REMARK: uses a fight of the wolf cave list (F061) in the goblin cave */
		if (random_schick(100) < 30)
		{
			g_max_enemies = (random_schick(100) < 10 ? 3 : 2);
			g_fig_escape_position[NORTH] = g_fig_escape_position[EAST] = g_fig_escape_position[SOUTH] = g_fig_escape_position[WEST] = DNG_POS_DIR(0,8,7,SOUTH);

			do_fight(FIGHT_ID_F061_4B);
		}

	} else if (target_pos == DNG_POS(0,11,1) && target_pos != gs_dng_pos_bak)
	{
		GUI_output(get_tx(11)); /* "HIER LIEGEN ZWEI GOBLIN-LEICHEN, DIE ALLERDINGS NUR IHRE KLEIDER AM..." */

	} else if (target_pos == DNG_POS(0,6,1) && target_pos != gs_dng_pos_bak)
	{
		if (GUI_bool(get_tx(12))) /* "VOR EUCH IN DER WAND IST EIN LOCH.@WOLLT IHR HINEINGREIFEN?" */
		{
			hero = get_first_hero_available_in_group();

			sprintf(g_dtp2,	get_tx(13), hero->alias); /* "%s WURDE VON EINER RATTE GEBISSEN. DIE WUNDE SIEHT ZIEMLICH TIEF AUS." */
			GUI_output(g_dtp2);

			sub_hero_le(hero, dice_roll(1, 3, 2));

			hero_disease_test(hero, DISEASE_ID_WUNDFIEBER, 65);
		}

	} else if (target_pos == DNG_POS(0,3,14) && target_pos != gs_dng_pos_bak)
	{
		load_ani(32);
		init_ani(1);

		GUI_output(get_tx(14)); /* "IHR STÖBERT EINEN DÖSENDEN HÖHLENBÄREN AUF, DER BEI EUREM NÄHERKOMM..." */

		disable_ani();

		/* the bear chases the group back to (5,14) */
		gs_x = 5;
		g_area_prepared = AREA_TYPE_NONE;

	} else if (target_pos == DNG_POS(0,6,15) && target_pos != gs_dng_pos_bak)
	{
		/* the exit of this dungeon */
		leave_dungeon();

		gs_town_id = gs_travel_destination_town_id;
		gs_x = gs_travel_destination_x;
		gs_y = gs_travel_destination_y;
		gs_town_loc_type = LOCTYPE_NONE;
		gs_viewdir = REVERSE_VIEWDIR(gs_travel_destination_viewdir);

		sprintf(g_dtp2, get_tx(17), get_ttx(gs_journey_destination_town_id + 0xeb)); /* "IHR VERLASST MÜDE UND SCHLAPP DIESE HÖHLE..." / TEXT.LTX 236..287: Ortsname */
		GUI_output(g_dtp2);

		timewarp(HOURS(2));

		g_fading_state = 3;
	}

	g_textbox_width = tw_bak;
	gs_dng_pos_bak = target_pos;

	return 0;
}

/**
 * \brief   print introductions of dungeon #5 fights
 *
 * \param   fight_id    ID of the fight
 */
void DNG05_fight_intro(const signed int fight_id)
{
	if (fight_id == 76) { /* FIGHT_ID_F076_07 */
		GUI_output(get_tx(10)); /* "DIES IST OFFENSICHTLICH DIE SCHATZKAMMER DER GOBLINS. IN EINER ECKE..." */
	}
}
