/*
 * seg083 - Dungeon "Orkbehausung" (DNG08)
 *
 * Inhalt:    DNG08_handler() steuert die Orkhoehle zwischen Phexcaer und Skelellen: zwoelf
 *            Strohlager zum Durchsuchen (DNG08_search_bed: Floehe CH -2, mit Glueck
 *            Schnaps, Dolch oder Heller), Wasserfass, zwei Kessel (satt, LE +2, CH -1),
 *            Ballista und Brandgeschosse zerstoeren (je ein Kampf), Latrine, Hebel, der
 *            festgehalten werden muss (Held bleibt als eigene Gruppe zurueck), Speerfalle,
 *            Geheimtuer, Goetzenstatuen und Ausgang. Dazu die Truhen: Proviant,
 *            Schnappfalle, Orkraetsel "TAIRACH" und drei Truhen mit Bronzeschluessel.
 * Aufrufer:  DNG08_handler() ueber g_dng_handlers (seg076), die Truhen ueber
 *            g_dng08_specialchests und loot_special_chest() (seg092).
 * Daten:     g_dng08_specialchests[7], Loesungswort TAIRACH.
 * Besonders: Kessel und Speerfalle: Heldenschleife beginnt beim ersten verfuegbaren
 *            Helden statt bei Held 0 (laeuft ueber das Feld hinaus, CH-Malus trifft
 *            get_hero(i)). Geheimtuer (5,8) verlangt Betreten UND Drehen im selben Schritt.
 *            gs_dng08_chest2_looted wird nie gesetzt.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg083.c
 */
#include <stdio.h>
#include <string.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg007.h"
#include "seg025.h"
#include "seg032.h"
#include "seg047.h"
#include "seg049.h"
#include "seg075.h"
#include "seg076.h"
#include "seg083.h"
#include "seg092.h"
#include "seg096.h"
#include "seg097.h"
#include "seg103.h"
#include "seg105.h"

struct struct_chest g_dng08_specialchests[7] = {
	{ DNG_POS(0,14,1), 0, NULL,			NULL,			DNG08_chest00_loot, 0, 0, 0 },
	{ DNG_POS(0,14,3), 4, use_lockpicks_on_chest,	DNG08_chest01_trap,	DNG08_chest01_loot, 0, 0, 0 },
	{ DNG_POS(0,9,7),  0, DNG08_chest02_open,	NULL,			DNG08_chest02_loot, 0, 0, 0 },
	{ DNG_POS(0,5,13), 5, DNG08_chest03_open,	chest_poisoned1,	NULL, 0, 0, 0 },
	{ DNG_POS(0,6,12), 5, DNG08_chest04_open,	chest_poisoned1,	DNG08_chest04_loot, 0, 0, 0 },
	{ DNG_POS(0,6,14), 5, DNG08_chest05_open,	chest_poisoned1,	DNG08_chest05_loot, 0, 0, 0 },
	{     -1, 0, NULL, NULL, NULL, 0, 0, 0 }
}; // ds:0x9692
static const char g_dng08_str_tairach[8] = "TAIRACH"; // ds:0x9725

/**
 * \brief   dungeon handler of the orc lair
 */
signed int DNG08_handler(void)
{
	signed int target_pos;
	signed int i;
	signed int tmp; /* multiple use: mod_slot, group_id */
	signed int tw_bak;
	struct struct_hero *hero;
	uint8_t *amap_ptr;

	amap_ptr = g_dng_map;

	tw_bak = g_textbox_width;
	g_textbox_width = 7;

	target_pos = DNG_POS(gs_dungeon_level, gs_x, gs_y);

	hero = get_first_hero_available_in_group();

	if (target_pos == DNG_POS(0,1,10) && target_pos != gs_dng_pos_bak && !gs_dng08_bed_00)
	{
		if (GUI_bool(get_tx(1))) /* "EINIGE SCHMUTZIGE DECKEN UND ANGESAMMELTES STROH BILDEN HIER EINPRI..." */
		{
			gs_dng08_bed_00 = 1;
			DNG08_search_bed();
		}

	} else if (target_pos == DNG_POS(0,1,9) && target_pos != gs_dng_pos_bak && !gs_dng08_bed_01)
	{
		if (GUI_bool(get_tx(1))) /* "EINIGE SCHMUTZIGE DECKEN UND ANGESAMMELTES STROH BILDEN HIER EINPRI..." */
		{
			gs_dng08_bed_01 = 1;
			DNG08_search_bed();
		}

	} else if (target_pos == DNG_POS(0,1,8) && target_pos != gs_dng_pos_bak && !gs_dng08_bed_02)
	{
		if (GUI_bool(get_tx(1))) /* "EINIGE SCHMUTZIGE DECKEN UND ANGESAMMELTES STROH BILDEN HIER EINPRI..." */
		{
			gs_dng08_bed_02 = 1;
			DNG08_search_bed();
		}

	} else if (target_pos == DNG_POS(0,1,7) && target_pos != gs_dng_pos_bak && !gs_dng08_bed_03)
	{
		if (GUI_bool(get_tx(1))) /* "EINIGE SCHMUTZIGE DECKEN UND ANGESAMMELTES STROH BILDEN HIER EINPRI..." */
		{
			gs_dng08_bed_03 = 1;
			DNG08_search_bed();
		}

	} else if (target_pos == DNG_POS(0,1,6) && target_pos != gs_dng_pos_bak && !gs_dng08_bed_04)
	{
		if (GUI_bool(get_tx(1))) /* "EINIGE SCHMUTZIGE DECKEN UND ANGESAMMELTES STROH BILDEN HIER EINPRI..." */
		{
			gs_dng08_bed_04 = 1;
			DNG08_search_bed();
		}

	} else if (target_pos == DNG_POS(0,6,6) && target_pos != gs_dng_pos_bak && !gs_dng08_bed_05)
	{
		if (GUI_bool(get_tx(1))) /* "EINIGE SCHMUTZIGE DECKEN UND ANGESAMMELTES STROH BILDEN HIER EINPRI..." */
		{
			gs_dng08_bed_05 = 1;
			DNG08_search_bed();
		}

	} else if (target_pos == DNG_POS(0,6,10) && target_pos != gs_dng_pos_bak && !gs_dng08_bed_06)
	{
		if (GUI_bool(get_tx(1))) /* "EINIGE SCHMUTZIGE DECKEN UND ANGESAMMELTES STROH BILDEN HIER EINPRI..." */
		{
			gs_dng08_bed_06 = 1;
			DNG08_search_bed();
		}

	} else if (target_pos == DNG_POS(0,7,7) && target_pos != gs_dng_pos_bak && !gs_dng08_bed_07)
	{
		if (GUI_bool(get_tx(1))) /* "EINIGE SCHMUTZIGE DECKEN UND ANGESAMMELTES STROH BILDEN HIER EINPRI..." */
		{
			gs_dng08_bed_07 = 1;
			DNG08_search_bed();
		}

	} else if (target_pos == DNG_POS(0,7,1) && target_pos != gs_dng_pos_bak && !gs_dng08_bed_08)
	{
		if (GUI_bool(get_tx(1))) /* "EINIGE SCHMUTZIGE DECKEN UND ANGESAMMELTES STROH BILDEN HIER EINPRI..." */
		{
			gs_dng08_bed_08 = 1;
			DNG08_search_bed();
		}

	} else if (target_pos == DNG_POS(0,11,1) && target_pos != gs_dng_pos_bak && !gs_dng08_bed_09)
	{
		if (GUI_bool(get_tx(1))) /* "EINIGE SCHMUTZIGE DECKEN UND ANGESAMMELTES STROH BILDEN HIER EINPRI..." */
		{
			gs_dng08_bed_09 = 1;
			DNG08_search_bed();
		}

	} else if (target_pos == DNG_POS(0,7,4) && target_pos != gs_dng_pos_bak && !gs_dng08_bed_10)
	{
		if (GUI_bool(get_tx(1))) /* "EINIGE SCHMUTZIGE DECKEN UND ANGESAMMELTES STROH BILDEN HIER EINPRI..." */
		{
			gs_dng08_bed_10 = 1;
			DNG08_search_bed();
		}

	} else if (target_pos == DNG_POS(0,10,5) && target_pos != gs_dng_pos_bak && !gs_dng08_bed_11)
	{
		if (GUI_bool(get_tx(1))) /* "EINIGE SCHMUTZIGE DECKEN UND ANGESAMMELTES STROH BILDEN HIER EINPRI..." */
		{
			gs_dng08_bed_11 = 1;
			DNG08_search_bed();
		}

	} else if (target_pos == DNG_POS(0,8,5) && target_pos != gs_dng_pos_bak && gs_dng08_waterbarrel)
	{
		DNG_waterbarrel(&gs_dng08_waterbarrel);

	} else if (target_pos == DNG_POS(0,3,6) && target_pos != gs_dng_pos_bak && !gs_dng08_timer1)
	{
		if (GUI_bool(get_tx(5))) /* "ÜBER EINER FEUERSTELLE HÄNGT EIN KESSEL MIT UNDEFINIERBAREM INHALT...." */
		{
			GUI_output(get_tx(6)); /* "SCHMECKT ZUM KOTZEN, FÜLLT ABER WENIGSTENS DEN MAGEN." */

			/* Original-Bug: hero still points to the first available hero, not to hero 0. If that hero
			 * is not at position 0, the loop checks and feeds the wrong slots (up to behind the last
			 * hero), while the CH modification goes to get_hero(i). Same in the second cauldron and
			 * the spear trap below. The cauldron can be used again after 24 hours (gs_dng08_timer1/2). */
#if FIX_0111
			/* Fix 111: bei Platz 0 beginnen (hero zeigte noch auf den ersten handlungsfaehigen Helden) */
			hero = get_hero(0);
#endif
			for (i = 0; i <= 6; i++, hero++)
			{
				if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
					!hero->flags.dead)
				{
					gs_dng08_timer1 = 24;

					hero->hunger = hero->thirst = 0;

					add_hero_le(hero, 2);

					tmp = get_free_mod_slot();

					set_mod_slot(tmp, DAYS(1), (uint8_t*)&get_hero(i)->attrib[ATTRIB_CH].current, -1, (signed char)i);
				}
			}
		}

	} else if (target_pos == DNG_POS(0,8,9) && target_pos != gs_dng_pos_bak && !gs_dng08_timer2)
	{
		if (GUI_bool(get_tx(5))) /* "ÜBER EINER FEUERSTELLE HÄNGT EIN KESSEL MIT UNDEFINIERBAREM INHALT...." */
		{
			GUI_output(get_tx(6)); /* "SCHMECKT ZUM KOTZEN, FÜLLT ABER WENIGSTENS DEN MAGEN." */

#if FIX_0111
			/* Fix 111: bei Platz 0 beginnen (hero zeigte noch auf den ersten handlungsfaehigen Helden) */
			hero = get_hero(0);
#endif
			for (i = 0; i <= 6; i++, hero++)
			{
				if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !hero->flags.dead)
				{
					gs_dng08_timer2 = 24;

					hero->hunger = hero->thirst = 0;

					add_hero_le(hero, 2);

					tmp = get_free_mod_slot();

					set_mod_slot(tmp, DAYS(1), (uint8_t*)&get_hero(i)->attrib[ATTRIB_CH].current, -1, (signed char)i);
				}
			}
		}

	} else if (target_pos == DNG_POS(0,1,1) && target_pos != gs_dng_pos_bak && !gs_dng08_ballista_done)
	{
		if (GUI_bool(get_tx(7))) /* "VOR EUCH LIEGT EIN HAUFEN BEHAUENES HOLZ, EINIGE TAUSENDE EISENTEIL..." */
		{
			gs_dng08_ballista_done = 1;

			GUI_output(get_tx(8)); /* "UNTER EUREN WUCHTIGEN SCHLÄGEN ZERBIRST DAS HOLZ. DIE TRÜMMER SIND..." */

			add_hero_ap_all(10);

			g_fig_escape_position[NORTH] = g_fig_escape_position[SOUTH] = g_fig_escape_position[WEST] = DNG_POS_DIR(0,2,7,SOUTH);
			g_fig_escape_position[EAST] = DNG_POS_DIR(0,6,3,EAST);

			do_fight(FIGHT_ID_F108_3B);
		}

	} else if (target_pos == DNG_POS(0,1,4) && target_pos != gs_dng_pos_bak && !gs_dng08_bundles_done)
	{
		if (GUI_bool(get_tx(9))) /* "IN DER ECKE VOR EUCH LIEGEN DUTZENDE VON BALLEN ÖLGETRÄNKTENHANFSEI..." */
		{
			gs_dng08_bundles_done = 1;

			GUI_output(get_tx(10)); /* "IN EINER STICHFLAMME VERGEHEN DIE GESCHOSSE, DICHTER RAUCH STEIGT A..." */

			add_hero_ap_all(10);

			g_fig_escape_position[NORTH] = (g_fig_escape_position[SOUTH] = ( g_fig_escape_position[WEST] = (DNG_POS_DIR(0,2,7,SOUTH))));
			g_fig_escape_position[EAST] = (DNG_POS_DIR(0,6,3,EAST));

			do_fight(FIGHT_ID_F108_3B);
		}


	} else if (target_pos == DNG_POS(0,11,14) && target_pos != gs_dng_pos_bak)
	{
		/* latrine */
		GUI_output(get_tx(12)); /* "EINE ÜBELRIECHENDE ORKISCHE LATRINE." */

	} else if (target_pos == DNG_POS(0,11,11) &&
			 (target_pos != gs_dng_pos_bak || gs_viewdir != gs_viewdir_bak) &&
			gs_viewdir == WEST)
	{
		/* lever, opens door at (8,13)  */
		do {
			i = GUI_radio(get_tx(13), 3, get_tx(14), get_tx(15), get_tx(16)); /* "AUS DER WAND RAGT EIN HEBEL.@..." / "DEN HEBEL IN RUHE LASSEN?" / "DEN HEBEL KURZ NIEDERDRÜCKEN?" / "DEN HEBEL NIEDERDRÜCKEN UND H..." */

		} while (i == -1);

		if (i == 2)
		{
			GUI_output(get_tx(17)); /* "DER HEBEL SCHNAPPT IN SEINE AUSGANGSLAGE ZURÜCK." */

		} else if (i == 3)
		{
			hero = get_hero(select_hero_ok_forced(get_tx(18))); /* "WER SOLL DEN HEBEL FESTHALTEN?" */

			sprintf(g_dtp2, get_tx(19), hero->alias); /* "MIT MÜHE HÄLT %s DEN HEBEL IN POSITION." */

			GUI_output(g_dtp2);

			/* open door at (8,13) */
			*(amap_ptr + MAP_POS(8,13)) &= 0x0f;
			*(amap_ptr + MAP_POS(8,13)) |= DNG_TILE_OPEN_DOOR << 4;

			if (gs_group_member_counts[gs_active_group_id] > 1) {

				gs_viewdir_bak = gs_viewdir;

				for (tmp = 0; gs_group_member_counts[tmp] != 0; tmp++); /* find empty group */

				hero->group_id = (signed char)tmp;
				gs_group_member_counts[tmp]++;
				gs_group_member_counts[gs_active_group_id]--;

				GRP_save_pos(tmp);
			}

		} else {
			gs_viewdir_bak = gs_viewdir;
		}

	} else if ((target_pos == DNG_POS(0,11,10) || target_pos == DNG_POS(0,11,12)) && target_pos != gs_dng_pos_bak)
	{
		/* squares next to lever: check if group at the lever moved away and the door must be closed */
		/* TODO: potential Original-Bug: What if the group at the lever to use Transversalis to teleport away? I guess the door is still open */
		/* REMARK: also, the other group is only checked by x/y, not by dungeon and level. */

		/* check if there is still another group on the square of the lever */
		for (i = tmp = 0; i < 6; i++)
		{
			if (gs_groups_x[i] == 11 && gs_groups_y[i] == 11 &&
#if FIX_0110
				/* Fix 110: die andere Gruppe muss in diesem Dungeon auf dieser Ebene stehen */
				gs_groups_dungeon_id[i] == gs_dungeon_id && gs_groups_dng_level[i] == gs_dungeon_level &&
#endif
				gs_active_group_id != i)
			{
				tmp = 1;
			}
		}

		/* if not, close door at (8,13) */
		if (tmp == 0)
		{
			*(amap_ptr + MAP_POS(8,13)) &= 0x0f;
			*(amap_ptr + MAP_POS(8,13)) |= DNG_TILE_CLOSED_DOOR << 4;
		}

	} else if (target_pos == DNG_POS(0,10,12) && target_pos != gs_dng_pos_bak)
	{
		GUI_output(get_tx(20)); /* "VOR EUCH TUT SICH EINE SPEERGESPICKTE FALLE AUF." */

#if FIX_0111
		/* Fix 111: bei Platz 0 beginnen (hero zeigte noch auf den ersten handlungsfaehigen Helden) */
		hero = get_hero(0);
#endif
		for (i = 0; i <= 6; i++, hero++)
		{
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
				!hero->flags.dead && test_talent(hero, TA_KLETTERN, 2) <= 0)
			{
				sprintf(g_dtp2, get_tx(21), hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST)); /* "%s RUTSCHT AUS, STÜRZT UND ZIEHT SICH EINIGE HÄSSLICH BLUTENDE WUND..." */
				GUI_output(g_dtp2);

				sub_hero_le(hero, dice_roll(1, 6, 4));
			}
		}

#if !FIX_0106
	} else if (target_pos == DNG_POS(0,5,8) && target_pos != gs_dng_pos_bak &&
			/* Original-Bug: "&&" instead of "||" (compare the secret doors in seg078). The group must
			 * enter the square and turn west in the same step, which normal movement never does, so the
			 * secret door at (4,8) can practically not be found (vermutlich, not tested). */
			gs_viewdir != gs_viewdir_bak &&
#else
	} else if (target_pos == DNG_POS(0,5,8) &&
			/* Fix 106: Betreten ODER Drehen (wie bei den Geheimtueren in seg078) */
			(target_pos != gs_dng_pos_bak || gs_viewdir != gs_viewdir_bak) &&
#endif
			gs_viewdir == WEST &&
			gs_dng08_secret_door_status != DNG_SECRET_DOOR_STATUS_OPEN)
	{
		if (gs_dng08_secret_door_status ||
			test_talent((hero = get_first_hero_available_in_group()), TA_SINNESSCHAERFE, 1) > 0)
		{
			gs_dng08_secret_door_status = DNG_SECRET_DOOR_STATUS_DISCOVERED;

			sprintf(g_dtp2, get_tx(29), hero->alias); /* "BEI GENAUEM HINSEHEN, ENTDECKT %s EIN GEHEIMTÜR IN DER WAND." */

			sprintf(g_text_output_buf,
				(char*)((tmp = test_talent(hero, TA_SCHLOESSER, 2)) > 0 ? get_tx(30): get_tx(31)), /* "@MIT EINEM LEISEN SCHNAPPEN SCHWINGT DIE T..." / "@LEIDER SCHAFFT %s ES ALLERDINGS NICHT, SI..." */
				GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST));

			strcat(g_dtp2, g_text_output_buf);

			GUI_output(g_dtp2);

			if (tmp > 0)
			{
				/* open the secret door */
				amap_ptr[MAP_POS(4,8)] = 0;
				gs_dng08_secret_door_status = DNG_SECRET_DOOR_STATUS_OPEN;
				DNG_update_pos();
			}

			gs_viewdir_bak = gs_viewdir;
		}

	} else if (target_pos == DNG_POS(0,5,7) && target_pos != gs_dng_pos_bak)
	{
		sprintf(g_dtp2, get_tx(22), /* "VOR EUCH STEHT EINE GROB AUS HOLZ GEHAUENE STATUE, DIE EINEN ORKISC..." */
			(char*)(test_talent(hero, TA_GOETTER_KULTE, 4) <= 0 ? get_tx(23) : get_tx(24))); /* "DEN IHR NICHT ERKENNT" / "OFFENSICHTLICH BRAZORAGH, DEN KRIEGSGOTT" */

		GUI_output(g_dtp2);

	} else if (target_pos == DNG_POS(0,5,9) && target_pos != gs_dng_pos_bak)
	{
		sprintf(g_dtp2,	get_tx(22), /* "VOR EUCH STEHT EINE GROB AUS HOLZ GEHAUENE STATUE, DIE EINEN ORKISC..." */
			(char*)(test_talent(hero, TA_GOETTER_KULTE, 6) <= 0 ? get_tx(23) : get_tx(25))); /* "DEN IHR NICHT ERKENNT" / "OFFENSICHTLICH TAIRACH, DEN JENSEITSHERRSCHER" */

		GUI_output(g_dtp2);

	} else if (target_pos == DNG_POS(0,0,14) && target_pos != gs_dng_pos_bak)
	{
		leave_dungeon();

		gs_town_id = gs_travel_destination_town_id;
		gs_x = gs_travel_destination_x;
		gs_y = gs_travel_destination_y;
		gs_town_loc_type = LOCTYPE_NONE;
		gs_viewdir = REVERSE_VIEWDIR(gs_travel_destination_viewdir);

		sprintf(g_dtp2,	get_tx(33), get_ttx(gs_journey_destination_town_id + 0xeb)); /* "ERSCHÖPFT UND AUSGELAUGT VERLASST IHR DIES..." / TEXT.LTX 236..287: Ortsname */
		GUI_output(g_dtp2);

		timewarp(HOURS(3));

		g_fading_state = 3;
	}


	g_textbox_width = tw_bak;

	gs_dng_pos_bak = target_pos;

	return 0;

}

/**
 * \brief   searching a straw bed: CH -2 for 6 hours (fleas) for the first hero,
 *          10% each for a brandy bottle, a dagger and 1W6 Heller
 */
/* static */
void DNG08_search_bed(void)
{
	signed int counter;
	signed int money;
	signed int mod_slot;
	struct struct_hero *hero;

	money = counter = 0;

	hero = get_first_hero_available_in_group();

	mod_slot = get_free_mod_slot();

	set_mod_slot(mod_slot, HOURS(6), (uint8_t*)&hero->attrib[ATTRIB_CH].current, -2, (signed char)get_hero_pos(hero));

	if (random_schick(100) <= 10)
	{
		sprintf(g_dtp2, get_tx(3), hero->alias, (char*)GUI_name_inflect_with_article( /* "%s FINDET %s" */
			INFLECT_INDEFINITE_ARTICLE | INFLECT_SINGULAR | INFLECT_4TH_CASE,
			ITEM_ID_SCHNAPSFLASCHE,
			INFLECT_NAME_TYPE_ITEM
		));

		/* a BRANDY BOTTLE */
		give_new_item_to_group(ITEM_ID_SCHNAPSFLASCHE, 1, 1);

		GUI_output(g_dtp2);

		counter++;
	}

	if (random_schick(100) <= 10)
	{
		sprintf(g_dtp2, get_tx(3), hero->alias, (char*)GUI_name_inflect_with_article( /* "%s FINDET %s" */
			INFLECT_INDEFINITE_ARTICLE | INFLECT_SINGULAR | INFLECT_4TH_CASE,
			ITEM_ID_DOLCH,
			INFLECT_NAME_TYPE_ITEM
		));

		/* a DAGGER */
		give_new_item_to_group(ITEM_ID_DOLCH, 1, 1);

		GUI_output(g_dtp2);

		counter++;
	}

	if (random_schick(100) <= 10)
	{
		money = random_schick(6);

		sprintf(g_text_output_buf, get_tx(4), money); /* "%d HELLER" */

		sprintf(g_dtp2, get_tx(3), hero->alias, g_text_output_buf); /* "%s FINDET %s" */

		GUI_output(g_dtp2);

		add_party_money(money);

		counter++;
	}

	if (!counter)
	{
		GUI_output(get_tx(2)); /* "ES GIBT HIER NICHTS ZU FINDEN!" */
	}
}

/**
 * \brief   trap of chest 1: iron jaws, 3W6 LE for the first hero
 */
void DNG08_chest01_trap(void)
{
	struct struct_hero *hero = get_first_hero_available_in_group();

	sprintf(g_dtp2, get_tx(11), hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_2ND)); /* "PLÖTZLICH SCHNAPPEN ZWEI EISERNE KIEFER ZU UND TRENNEN %s FAST DIE..." */
	GUI_output(g_dtp2);

	/* 3W6 damage */
	sub_hero_le(hero, dice_roll(3, 6, 0));
}

void DNG08_chest00_loot(struct struct_chest* chest)
{
	if (!gs_dng08_chest0_looted)
	{
		gs_dng08_chest0_looted = 1;

		GUI_output(get_tx(32)); /* "IN DER TRUHE HAT ES 40 RATIONEN, DIE IHR SOFORT EINPACKT UND MITNEHMT." */

		/* get 40 FOOD PACKAGES */
		give_new_item_to_group(ITEM_ID_PROVIANTPAKET, 1, 40);
	}
}

void DNG08_chest01_loot(struct struct_chest* chest)
{
	loot_multi_chest(gs_dng08_chest1_content, get_tx(34)); /* "IN DIESER TRUHE FINDET IHR" */
}

void DNG08_chest02_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng08_chest2_content;

	loot_simple_chest(chest);

	chest->loot.content = ptr_bak;
}

/**
 * \brief   riddle chest in orcish: needs a Sprachen test to understand the question, answer TAIRACH
 */
void DNG08_chest02_open(struct struct_chest* chest)
{
	/* REMARK: gs_dng08_chest2_looted is never set, so the else branch is never reached */
	if (!gs_dng08_chest2_looted)
	{
		if (test_talent(get_first_hero_available_in_group(), TA_SPRACHEN, 2) > 0)
		{
			GUI_input(get_tx(27), 10); /* "<WER IST DER HERR ÜBER DEN TOD, ÜBER BLUT UND RACHE?> FRAGT EUCH DI..." */

			if (!strcmp(g_text_input_buf, g_dng08_str_tairach)) {

				chest->loot.func(chest);

			} else if (chest->trap) {

				chest->trap();
			}

		} else {
			GUI_output(get_tx(26)); /* "DIE TRUHE STELLT EUCH AUF ORKISCH EINE RÄTSELFRAGE." */
		}
	} else {
		GUI_input(get_ttx(522), 0); /* "DIE TRUHE IST LEER!" */
	}
}

/**
 * \brief   chests 3..5 open with the bronze key or a Schloesser+5 test, otherwise poison trap
 *          (chest_poisoned1); chest 3 contains 853 silver coins
 */
void DNG08_chest03_open(struct struct_chest* chest)
{
	struct struct_hero *hero = get_first_hero_available_in_group();

	if (get_first_hero_with_item(ITEM_ID_BRONZESCHLUESSEL) != -1 || test_talent(hero, TA_SCHLOESSER, 5) > 0)
	{
		if (!(gs_dng08_chest35_looted & 1)) {

			GUI_output(get_tx(28)); /* "IHR FINDET 853 SILBERTALER IN DER TRUHE." */

			add_party_money(8530L);

			gs_dng08_chest35_looted |= 1;
		} else {
			GUI_output(get_ttx(522)); /* "DIE TRUHE IST LEER!" */
		}
	} else {
		chest->trap();
	}
}

void DNG08_chest04_open(struct struct_chest* chest)
{
	struct struct_hero *hero = get_first_hero_available_in_group();

	if (get_first_hero_with_item(ITEM_ID_BRONZESCHLUESSEL) != -1 || test_talent(hero, TA_SCHLOESSER, 5) > 0) {

		chest->loot.func(chest);

	} else {

		chest->trap();
	}
}

void DNG08_chest05_open(struct struct_chest* chest)
{
	struct struct_hero *hero = get_first_hero_available_in_group();

	if (get_first_hero_with_item(ITEM_ID_BRONZESCHLUESSEL) != -1 || test_talent(hero, TA_SCHLOESSER, 5) > 0) {

		chest->loot.func(chest);

	} else {

		chest->trap();
	}
}

void DNG08_chest04_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng08_chest4_content;

	loot_simple_chest(chest);

	chest->loot.content = ptr_bak;
}

void DNG08_chest05_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng08_chest5_content;

	loot_simple_chest(chest);

	chest->loot.content = ptr_bak;

	if (!(gs_dng08_chest35_looted & 2))
	{
		GUI_output(get_tx(35)); /* "AUSSERDEM FINDET IHR IN DER TRUHE 289 SILBERTALER, DIE IHR NATÜRLIC..." */

		add_party_money(2890L);

		gs_dng08_chest35_looted |= 2;
	}
}
