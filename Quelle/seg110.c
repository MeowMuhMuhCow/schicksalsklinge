/*
 * seg110 - Reiseereignisse 2/10 (Ereignisse 11-50)
 *
 * Inhalt:    Ereignisse auf den Wegen um Peilinen/Rovamund, Kravik/Skelellen, Rukian/Angbodirtal,
 *            Auplog/Vilnheim/Phexcaer, Varnheim/Auplog und Daspota/Ottarje/Skjal: Pfad ueber den
 *            Roval zur Querverbindung (tevent_011, TRV_a_path), Pfad nach Westen (tevent_020), Leiche
 *            (tevent_014), Furten, Schilfmeer am Bodir (tevent_029), Orks und Raeuber, Abenteurer,
 *            sterbende Abenteurerin mit dem Hinweis auf den Daspota-Schatz (tevent_044), Bauer,
 *            Zugang zur Verfallenen Herberge (tevent_046) und viele Rast-, Kraeuter- und Jagdplaetze.
 * Aufrufer:  TRV_event() (seg109) ueber g_travel_event_handlers.
 * Daten:     g_tevent014_corpse (Truhenbeschreibung der Leiche).
 * Besonders: Neu: tevent_026/027 geben "DES BODIR" an "IHR STEHT AM UFER DES %s" (doppeltes DES);
 *            tevent_029 macht die Krankheitsprobe fuer alle sieben Heldenplaetze; tevent_041 setzt
 *            im Brueckentext das Pronomen im Dativ statt Akkusativ ein. tevent_020 setzt den
 *            Crosslink-Status falsch, was ohne Folgen bleibt (nur Bit 0 zaehlt).
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg110.c
 */

#include <stdio.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg003.h"
#include "seg004.h"
#include "seg007.h"
#include "seg025.h"
#include "seg026.h"
#include "seg027.h"
#include "seg029.h"
#include "seg032.h"
#include "seg047.h"
#include "seg092.h"
#include "seg096.h"
#include "seg097.h"
#include "seg103.h"
#include "seg105.h"
#include "seg109.h"
#include "seg110.h"
#include "seg117.h"

static struct struct_chest g_tevent014_corpse = {0, 1, NULL, NULL, tevent_014_chest, 0, 0, 0}; // ds:0xb13e

/* Peilinen <-> Rovamund: Spuren am anderen Ufer des Roval, Pfad zur Querverbindung (TRV_a_path) */
void tevent_011(void)
{
	signed int answer;
	struct struct_hero *hero;

	if ((test_talent((hero = get_first_hero_available_in_group()), TA_SINNESSCHAERFE, 8) > 0 && !gs_tevent011_flag) ||
		gs_tevent011_flag)
	{
		gs_tevent011_flag = 1;

		sprintf(g_dtp2,	get_tx2(24), hero->alias, hero->alias); /* "%s FORDERT EUCH AUF, STEHENZUBLEIBEN. AM ANDEREN UFER DES ROVAL SIN..." */

		do {
			answer = GUI_radio(g_dtp2, 2, get_tx2(25), get_tx2(26)); /* "IGNORIEREN UND WEITERMARSCHIEREN?" / "DER SACHE AUF DEN GRUND GEHEN?" */

		} while (answer == -1);

		if (answer == 2) {

			do {
				answer = GUI_radio(get_tx2(27), 3, get_tx2(28),	get_tx2(29), get_tx2(30)); /* "DAZU MÜSSTET IHR ALLERDINGS D..." / "MIT SACK UND PACK SCHWIMMEN?" / "VORHER EIN KLEINES FLOSS FÜR..." / "DIE SACHE DOCH LIEBER AUF SIC..." */

			} while (answer == -1);

			if (answer == 1) {

				/* just swim */
				TRV_swim2(0, 5);
				TRV_a_path();

			} else if (answer == 2) {

				/* build a raft */
				timewarp(HOURS(1));
				TRV_swim2(0, 0);
				TRV_a_path();
			}
		}
	}
}

/**
 * \brief   all members of the current group try to swim
 *
 *          In seg112.cpp exists a similiar function called TRV_swim() for that purpose.
 *
 * \param   handicap    handicap for the swim test
 * \param   percent     probability to loose an item
 */
/* should be static */
void TRV_swim2(const signed int handicap, const signed int percent)
{
	signed int i;
	struct struct_hero *hero;

	hero = get_hero(0);
	for (i = 0; i <= 6; i++, hero++)
	{
		if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !hero->flags.dead)
		{
			/* a swim test */
			if (test_talent(hero, TA_SCHWIMMEN, (signed char)handicap) > 0)
			{
				/* talent test succeeded */
				sprintf(g_dtp2, get_tx2(31), hero->alias); /* "%s ERREICHT SICHER DAS ANDERE UFER." */
				GUI_output(g_dtp2);

			} else {
				/* talent test failed */
				sprintf(g_dtp2, get_tx2(32), hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST)); /* "%s KANN SICH NUR MIT MÜH UND NOT ÜBER WASSER HALTEN. HUSTEND UND SP..." */
				GUI_output(g_dtp2);

				hero_disease_test(hero, DISEASE_ID_DUMPFSCHAEDEL, 20 - (hero->attrib[ATTRIB_KK].current + hero->attrib[ATTRIB_KK].mod));

				if (percent) {
					lose_random_item(hero, percent, get_ttx(506)); /* "OH VERDAMMT, %s HAT %s VERLOREN!" */
				}

				sub_hero_le(hero, random_schick(5));
			}
		}
	}
}

/* Path east from the route Rovamund <-> Peilinen. */
/* should be static */
void TRV_a_path(void)
{
	signed int answer;

	do {
		answer = GUI_radio(get_tx2(33), 2, get_tx2(34), get_tx2(35)); /* "NACHDEM IHR ALLE GLÜCKLICH UN..." / "DEM PFAD FOLGEN?" / "ODER ZURÜCKSCHWIMMEN?" */

	} while (answer == -1);

	if (answer == 1)
	{
		/* follow the path */
		g_journey_crosslink_status = (gs_town_id == TOWN_ID_PEILINEN ? CROSSLINK_STATUS_FROM_PEILINEN : CROSSLINK_STATUS_FROM_ROVAMUND);

		/* Original-Glitch:
		 * gs_travel_detour == 1 is indicating a detour to DNG_TOTENSCHIFF (which has the ID 1).
		 * Probably, this does not make a difference, but still, it would be better to use another number. */
		/* gs_travel_detour != 0 beendet hier nur trv_do_journey(); do_travel_mode() startet danach
		 * wegen g_journey_crosslink_status die Querverbindung, ein Dungeon wird nicht betreten. */
		gs_travel_detour = 1;
	} else {
		/* swim back */

		do {
			answer = GUI_radio(get_tx2(36), 2, get_tx2(28),	get_tx2(29)); /* "@WOLLT IHR" / "MIT SACK UND PACK SCHWIMMEN?" / "VORHER EIN KLEINES FLOSS FÜR..." */

		} while (answer == -1);

		if (answer == 1)
		{
			/* just swim */
			TRV_swim2(0, 5);

		/* TODO: this test is bogus, just an else case */
		/* (answer kann nach der Schleife nur 1 oder 2 sein) */
		} else if (answer == 2)
		{
			/* build a raft */
			timewarp(HOURS(1));
			TRV_swim2(0, 0);
		}

		GUI_output(get_tx2(37)); /* "WER SO HÄUFIG BADET, MUSS GANZ SCHÖN SCHMUTZIG SEIN!" */
	}
}

/* Rovamund <-> Nordvest: Kraeuterplatz (Belmart) */
void tevent_013(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_PFLANZENKUNDE, 3) > 0 && !gs_tevent013_flag) || gs_tevent013_flag)
	{
		g_gather_herbs_special = ITEM_ID_BELMART_BLATT;

		TRV_found_herb_place(0);

		g_gather_herbs_special = -1;

		gs_tevent013_flag = 1;
	}
}

/* Rovamund <-> Nordvest: Leiche eines Wanderers (Woelfe) */
void tevent_014(void)
{
	signed int answer;

	if (!gs_tevent014_flag)
	{
		load_in_head(55);

		do {
			answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(43), 2, get_tx2(44), get_tx2(45)); /* "NUR WENIGE SCHRITT VOM WEG EN..." / "DIE LEICHE GENAUER UNTERSUCHEN?" / "DEN GEIERN LASSEN, WAS IHNEN..." */

		} while (answer == -1);

		if (answer == 1)
		{
			/* examine the corpse */
			loot_corpse(&g_tevent014_corpse, get_tx2(46), (int8_t*)&gs_tevent014_flag); /* "OFFENSICHTLICH WURDE DER UNGLÜCKLICHE WANDERER DAS OPFER VON WÖLFEN..." */
		}
	}
}

void tevent_014_chest(struct struct_chest* chest)
{
	chest->loot.content = gs_tevent014_chest;

	loot_chest(chest, get_tx2(47), get_tx2(48)); /* "BEIM DURCHSTÖBERN DES GEPÄCKS FINDET IHR F..." / "AUSSER EIN PAAR VERMODERTEN GEBEINEN GIBT..." */
}

/* Rovamund <-> Nordvest: Herberge "Am Ingval" */
void tevent_015(void)
{
	TRV_found_inn(TOWN_ID_VAERMHAG, 67); /* 44 = FEATURE.LTX "AM INGVAL" */
}

void tevent_017(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 2) > 0 && !gs_tevent017_flag) ||
		gs_tevent017_flag)
	{
		TRV_found_replenish_place(0);
		gs_tevent017_flag = 1;
	}
}

/* Path west from the route Kravik <-> Skelellen. */
void tevent_020(void)
{
	signed int answer;
	struct struct_hero *hero;

	if ((test_talent((hero = get_first_hero_available_in_group()), TA_SINNESSCHAERFE, 7) > 0 && !gs_tevent020_flag) ||
		gs_tevent020_flag)
	{
		gs_tevent020_flag = 1;

		sprintf(g_dtp2, get_tx2(49), hero->alias); /* "%s IST AUFGEFALLEN, DASS HIER FRÜHER EINMAL EIN PFAD NACH WESTEN IN..." */

		do {
			answer = GUI_radio(g_dtp2, 2, get_tx2(50), get_tx2(51)); /* "DEN PFAD ERKUNDEN?" / "LIEBER WEITERMARSCHIEREN?" */

		} while (answer == -1);

		if (answer == 1)
		{
			do {
				answer = GUI_radio(get_tx2(52), 2, get_tx2(53), get_tx2(54)); /* "BEREITS NACH WENIGEN DUTZEND..." / "DEN WEG WEITER VERFOLGEN?" / "ZURÜCK AUF DIE STRASSE KRAVIK..." */

			} while (answer == -1);

			if (answer == 1)
			{
				/* TODO: Original-Bug: CURRENT_TOWN is either Kravik or Skelellen.
				 * So the following line should prabably be
				 * g_journey_crosslink_status = (gs_town_id == TOWN_ID_KRAVIK ? CROSSLINK_STATUS_FROM_KRAVIK : CROSSLINK_STATUS_FROM_SKELELLEN);
				 */
				/* Ohne Folgen: es wird immer 3 (FROM_SKELELLEN) gesetzt; ausgewertet wird nur Bit 0
				 * (TM_do_journey_crosslink, TM_draw_track_before_crosslink, tevent_145), das bei 1 und 3
				 * gleich ist. Die Vorstrecke waehlt TM_draw_track_before_crosslink() nach gs_town_id. */
				g_journey_crosslink_status = (gs_town_id == TOWN_ID_PEILINEN ? CROSSLINK_STATUS_FROM_KRAVIK : CROSSLINK_STATUS_FROM_SKELELLEN);

				/* Original-Glitch:
				 * gs_travel_detour == 1 is indicating a detour to DNG_TOTENSCHIFF (which had the ID 1).
				 * Probably, this does not make a difference, but still, it would be better to use another number. */
				gs_travel_detour = 1;
			}
		}

		timewarp(MINUTES(15));
	}
}

void tevent_021(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_PFLANZENKUNDE, 8) > 0 && !gs_tevent021_flag) || gs_tevent021_flag)
	{
		g_gather_herbs_special = ITEM_ID_ALRAUNE;

		TRV_found_herb_place(1);

		g_gather_herbs_special = -1;

		gs_tevent021_flag = 1;
	}
}

void tevent_022(void)
	/* Rukian <-> Faehrstation Angbodirtal: windgeschuetzte Stelle */
	/* (Rastplatz; die Ortsbeschreibung waehlt TRV_found_camp_place() zufaellig aus FEATURE.LTX 11..15,
	 *  danach evtl. Wildschweinjagd) */
{
	signed int answer;

	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 0) > 0 && !gs_tevent022_flag) || gs_tevent022_flag)
	{
		gs_tevent022_flag = 1;

		if (TRV_found_camp_place(0))
		{
			if ((test_talent(get_first_hero_available_in_group(), TA_FAEHRTENSUCHEN, 5) > 0 && !gs_tevent022_track_flag) || gs_tevent022_track_flag)
			{
				do {
					answer = GUI_radio(get_tx2(59), 2, get_tx2(60), get_tx2(61)); /* "NACHDEM IHR GERASTET HABT, FA..." / "DIE SPUR AUFNEHMEN?" / "EURE SACHEN PACKEN UND WEITER..." */

				} while (answer == -1);

				if (answer == 1)
				{
					TRV_hunt_generic(25, 62, -2, 2, 8, 3, 7, 2, 4, 30, 6);
					gs_tevent022_track_flag = 1;
				}
			}
		}
	}
}

void tevent_024(void)
	/* Auplog <-> Vilnheim: windgeschuetzte Stelle. Idealer Rastplatz */
	/* (Beschreibung zufaellig, siehe tevent_022) */
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 2) > 0 && !gs_tevent024_flag) ||
		gs_tevent024_flag)
	{
		TRV_found_camp_place(0);
		gs_tevent024_flag = 1;
	}
}

void tevent_025(void)
	/* Auplog <-> Vilnheim: umgestuerzte, moosbewachsene Baumstaemme. idealer Rastplatz. */
	/* (Beschreibung zufaellig, siehe tevent_022) */
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 1) > 0 && !gs_tevent025_flag) ||
		gs_tevent025_flag)
	{
		TRV_found_camp_place(0);
		gs_tevent025_flag = 1;
	}
}

#if FIX_0156
/* Fix 156: fuehrendes "DES " des Flussnamens weglassen, der Furttext hat schon "AM UFER DES %s" */
static char* fix_skip_des(char *p)
{
	if ((p[0] == 'D') && (p[1] == 'E') && (p[2] == 'S') && (p[3] == ' ')) {
		p += 4;
	}
	return p;
}
#endif

/* Vilnheim <-> Bodon: Furt durch den Bodir */
void tevent_026(void)
{
	/* Fehler im Original: Text 72 ist "DES BODIR", der Furttext hat schon "AM UFER DES %s"
	 * -> "IHR STEHT AM UFER DES DES BODIR." (ebenso tevent_027 mit "DES HJALDORVAL") */
#if FIX_0156
	TRV_cross_a_ford(fix_skip_des(get_tx2(72)), 60, 0);
#else
	TRV_cross_a_ford(get_tx2(72), 60, 0); /* "DES BODIR" */
#endif
}

/* Vilnheim <-> Phexcaer: Furt durch den Hjaldorval ("DES DES HJALDORVAL", siehe tevent_026) */
void tevent_027(void)
{
#if FIX_0156
	TRV_cross_a_ford(fix_skip_des(get_tx2(73)), 30, 0);
#else
	TRV_cross_a_ford(get_tx2(73), 30, 0); /* "DES HJALDORVAL" */
#endif
}

void tevent_028(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 2) > 0 && !gs_tevent028_flag) ||
		gs_tevent028_flag)
	{
		TRV_found_camp_place(1);
		gs_tevent028_flag = 1;
	}
}

/* Vilnheim <-> Phexcaer: Grenzpfahl des Orklands und Schilfmeer am Bodir */
void tevent_029(void)
{
	signed int hero_pos;
	signed int i; /* dual use: inv_slot and item_count */
	struct struct_hero *hero;

	load_in_head(51);

#if !FIX_0371
	GUI_dialog_na(0, get_tx2(74)); /* "HIER ENDET DIE EIGENTLICHE STRASSE. IHR BEFINDET EUCH NUN IM ORKLAN..." */
#else
	/* Fix 371: Strassenende/Orkland-Text 74 nur auf dem Weg nach Phexcaer (Ziel Phexcaer vorwaerts oder anderes Ziel nach Umkehr) */
	if ((gs_journey_destination_town_id == TOWN_ID_PHEXCAER) == (gs_journey_direction == JOURNEY_DIRECTION_FORWARD)) {
		GUI_dialog_na(0, get_tx2(74)); /* "HIER ENDET DIE EIGENTLICHE STRASSE. IHR BEFINDET EUCH NUN IM ORKLAN..." */
	}
#endif
	GUI_dialog_na(0, get_tx2(75)); /* "VOR EUCH LIEGT EIN GROSSES SCHILFGEBIET, DAS DIE EHEMALIGE STRASSE..." */

	hero = get_hero(0);
	/* Fehler im Original: keine Pruefung von typus, Gruppe und Tod - auch Helden anderer Gruppen und
	 * tote Helden koennen Dumpfschaedel bekommen, leere Plaetze werden ebenfalls "angesteckt". */
	for (hero_pos = 0; hero_pos <= 6; hero_pos++, hero++)
	{
#if FIX_0105
		/* Fix 105: nur lebende Helden der aktiven Gruppe */
		if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !hero->flags.dead)
#endif
		hero_disease_test(hero, DISEASE_ID_DUMPFSCHAEDEL, 20 - (hero->attrib[ATTRIB_KK].current + hero->attrib[ATTRIB_KK].mod));
	}

	if (test_talent(get_first_hero_available_in_group(), TA_ORIENTIERUNG, 3) > 0)
	{
		/* talent test succeeded */
		timewarp(HOURS(3));

		GUI_dialog_na(0, get_tx2(76)); /* "SCHLIESSLICH FINDET IHR DEN RAND DES SCHILFMEERES. IHR SEID ZWAR EI..." */
	} else {
		/* talent test failed */
		hero = get_hero(0);
		for (hero_pos = 0; hero_pos <= 6; hero_pos++, hero++)
		{
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !hero->flags.dead)
			{
				sub_hero_le(hero, 2);

				i = inv_slot_of_item(hero, ITEM_ID_PROVIANTPAKET);

				if (i != -1) {
					/* hero looses the first set of FOOD PACKAGES */
					drop_item(hero, i, hero->inventory[i].quantity);
				}

				i = hero_count_item(hero, ITEM_ID_WASSERSCHLAUCH);

				if (i) {
					/* hero looses the first WATERSKIN */
					drop_item(hero, inv_slot_of_item(hero, ITEM_ID_WASSERSCHLAUCH), i - 1);
					/* effect of i - 1:
					 * if i==0 (hero doesn't have a waterskin), don't drop anything.
					 * otherwise (hero has one ore more waterskins), drop a single one.
					 */
					/* (i == 0 kommt hier wegen if (i) nicht vor; drop_item() wirft bei nicht stapelbaren
					 *  Gegenstaenden fuer jede Menge >= 0 genau einen weg) */
				}
			}
		}

		timewarp(HOURS(5));

		GUI_dialog_na(0, get_tx2(77)); /* "DAS SCHILF WIRD IMMER DICHTER, ABER IHR SEID DER FESTEN MEINUNG, DA..." */
	}
}

void tevent_030(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 0) > 0 && !gs_tevent030_flag) ||
		gs_tevent030_flag)
	{
		TRV_found_camp_place(0);
		gs_tevent030_flag = 1;
	}
}

/* Vilnheim <-> Phexcaer: Ueberfall (Kampf F031); mit Hyggelik-Auftrag stattdessen der Weg zur Ruine */
void tevent_031(void)
{
	signed int answer;

	if (gs_find_hyggelik) {

		search_ruin1();

	} else if (!gs_tevent031_flag) {

		load_in_head(49);

		do {
			answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(0), 2, get_tx2(1), get_tx2(2)); /* "PLÖTZLICH SPRINGEN HINTER EIN..." / "EUCH ZUM KAMPF STELLEN?" / "FLIEHEN?" */

		} while (answer == -1);

		if (answer == 2)
		{
			if (test_talent(get_first_hero_available_in_group(), TA_KRIEGSKUNST, 7) <= 0)
			{
				/* talent test failed */
				g_fig_initiative = 1;
				do_fight(FIGHT_ID_F031);
			}
		} else {
			do_fight(FIGHT_ID_F031);
		}

		gs_tevent031_flag = 1;
	}
}

void tevent_032(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 2) > 0 && !gs_tevent032_flag) || gs_tevent032_flag)
	{
		gs_tevent032_flag = 1;

		if ((test_talent(get_first_hero_available_in_group(), TA_PFLANZENKUNDE, 5) > 0 && !gs_tevent032_herb_flag) || gs_tevent032_herb_flag)
		{
			gs_tevent032_herb_flag = 1;

			g_gather_herbs_special = ITEM_ID_EITRIGER_KROETENSCHEMEL;

			TRV_found_camp_place(2);

			g_gather_herbs_special = -1;
		} else {
			TRV_found_camp_place(0);
		}
	}
}

/* Vilnheim <-> Phexcaer: Abenteurer am Wegrand */
void tevent_033(void)
{
	signed int answer;

	load_in_head(9);

	do {
		answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(3), 3, /* "EINE ABENTEUERLICHE GESTALT HAT ES SICH AM WEGRAND BEQUEM GEMACHT,..." */
					get_tx2(4), get_tx2(5), get_tx2(6)); /* "DEN ABENTEURER NACH DEM WEG F..." / "EUCH ÜBER DIE ORKS UND PHEXCA..." / "EUREN <KOLLEGEN> GRÜSSEN UND..." */

	} while (answer == -1);

	if (answer == 1 || answer == 2)
	{
		timewarp(MINUTES(15));

		GUI_dialog_na(0, (answer == 1 ? get_tx2(7) : get_tx2(8))); /* "<VON HIER AUS IST ES HÖCHSTENS EINEN TAGES..." / "<DIE ORKS? DARÜBER KANN ICH EUCH KAUM WAS..." */
	}
}

void tevent_034(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 2) > 0 && !gs_tevent034_flag) ||
		gs_tevent034_flag)
	{
		TRV_found_camp_place(0);
		gs_tevent034_flag = 1;
	}
}

/* Phexcaer <-> Einsiedlersee: Kampf F035 (Fluchtversuch mit Kriegskunst-Probe) */
void tevent_035(void)
{
	signed int answer;

	if (!gs_tevent035_flag)
	{
		do {
			answer = GUI_radio(get_tx2(9), 2, get_tx2(10), get_tx2(11)); /* "MIT LEISEM SIRREN FLIEGEN SCH..." / "EUCH ZUM KAMPF STELLEN?" / "FLIEHEN?" */

		} while (answer == -1);

		if (answer == 2)
		{
			if (test_talent(get_first_hero_available_in_group(), TA_KRIEGSKUNST, 8) <= 0)
			{
				/* talent test failed */
				g_fig_initiative = 1;
				do_fight(FIGHT_ID_F035);
			}
		} else {
			do_fight(FIGHT_ID_F035);
		}

		gs_tevent035_flag = 1;
	}
}

void tevent_036(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 3) > 0 && !gs_tevent036_flag) || gs_tevent036_flag)
	{
		gs_tevent036_flag = 1;

		if ((test_talent(get_first_hero_available_in_group(), TA_PFLANZENKUNDE, 6) > 0 && !gs_tevent036_herb_flag) || gs_tevent036_herb_flag)
		{
			gs_tevent036_herb_flag = 1;

			g_gather_herbs_special = ITEM_ID_GULMOND_BLATT;

			TRV_found_camp_place(2);

			g_gather_herbs_special = -1;
		} else {
			TRV_found_camp_place(0);
		}
	}
}

void tevent_039(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 4) > 0 && !gs_tevent039_flag) ||
		gs_tevent039_flag)
	{
		TRV_found_camp_place(0);
		gs_tevent039_flag = 1;
	}
}

void tevent_040(void)
/* Auplog <-> Varnheim: alte Feuerstelle. idealer Rastplatz */
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 4) > 0 && !gs_tevent040_flag) ||
		gs_tevent040_flag)
	{
		TRV_found_camp_place(0);
		gs_tevent040_flag = 1;
	}
}

/* Auplog <-> Varnheim: kleiner aber reissender Bach, ueber den sich urspruenglich einmal eine Haengebruecke spannte. bridge building */
void tevent_041(void)
{
	signed int answer;

	/* Fehler im Original (Text): FEATURE2.LTX 19 hat nur zwei %s ("... KOENNT IHR %s VOR DEM ERTRINKEN
	 * RETTEN"), TRV_barrier() uebergibt Name, Dativ, Akkusativ -> "IHM"/"IHR" statt "IHN"/"SIE".
	 * Bei tevent_007 (FEATURE1.LTX 23, drei %s) stimmt es. */
	TRV_barrier(12);

	if (test_talent(get_first_hero_available_in_group(), TA_FAEHRTENSUCHEN, 0) > 0 && !gs_tevent041_flag)
	{
		do {
			/* Spuren von Karenen */
			answer = GUI_radio(get_tx2(20), 2, get_tx2(21), get_tx2(22)); /* "NACHDEM IHR DIE SCHLUCHT ÜBER..." / "DIE SPUR AUFNEHMEN?" / "EUREN WEG FORTSETZEN?" */

		} while (answer == -1);

		if (answer == 1)
		{
			TRV_hunt_generic(21, 23, -2, 2, 8, 3, 7, 2, 4, 15, 8);

			gs_tevent041_flag = 1;
		}
	}
}

void tevent_042(void)
	/* Auplog <-> Varnheim: (weiter von Auplog weg als tevent_043) kleiner Bachlauf. Nahrung. */
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 2) > 0 && !gs_tevent042_flag) ||
		gs_tevent042_flag)
	{
		TRV_found_replenish_place(0);
		gs_tevent042_flag = 1;
	}
}

void tevent_043(void)
	/* Auplog <-> Varnheim: (nahe an Auplog) kleiner Bachlauf. Nahrung. */
	/* Korrektur: es ist ein Rastplatz (TRV_found_camp_place), kein Nahrungsplatz. */
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 3) > 0 && !gs_tevent043_flag) ||
		gs_tevent043_flag)
	{
		TRV_found_camp_place(0);
		gs_tevent043_flag = 1;
	}
}

/* dying adventuress */
/* Daspota <-> Rybon: sie verraet "ZEHN MEILEN HINTER RYBON ... DER DASPOTA-SCHATZ"; das Flag
 * gs_daspota_treasure_hint schaltet den Hoehleneingang in tevent_094 (Rybon <-> Thoss) frei. */
void tevent_044(void)
{
	signed int answer;
	struct struct_hero *hero;

	if (!gs_daspota_treasure_hint) {

		gs_daspota_treasure_hint = 1;

		load_in_head(1);

		hero = get_first_hero_available_in_group();

		sprintf((char*)(g_dtp2 + 0x400), get_tx2(33), hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_3RD)); /* "AUF DEM SCHMALEN PFAD KOMMT EUCH EINE GESTALT ENTGEGENGETORKELT. IH..." */

		do {
			answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, (char*)(g_dtp2 + 0x400), 2, get_tx2(34), get_tx2(35)); /* "DIE TOTE DEN AASKRÄHEN ÜBERLASSEN?" / "DIE ABENTEURERIN BEERDIGEN?" */

		} while (answer == -1);

		if (answer == 2) {

			/* bury her */
			gs_gods_estimation[GOD_ID_BORON] += 50L;

			timewarp(HOURS(1));

			add_hero_ap_all(10);
		}
	}
}

/* Ottarje -> Daspota: a farmer */
void tevent_045(void)
{
	signed int answer;

	if (gs_journey_destination_town_id == TOWN_ID_DASPOTA)
	{
		load_in_head(11);

		do {
			answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(36), 3, /* "EUCH KOMMT EIN EINSAMER WANDERER ENTGEGEN, DER KLEIDUNG NACH OFFENS..." */
						get_tx2(37), get_tx2(38), get_tx2(39)); /* "DEN BAUERN NACH DEM WEG FRAGEN?" / "EUCH ÜBER EVENTUELLE GEFAHREN..." / "DEN LANDMANN GRÜSSEN UND DANN..." */

		} while (answer == -1);

		if (answer == 1 || answer == 2)
		{
			timewarp(MINUTES(15));

			GUI_dialog_na(0, (answer == 1 ? get_tx2(40) : get_tx2(41))); /* "<DER WEG FÜHRT VON OTTARJE NACH RYBON, ODE..." / "<HUM, JA, ALSO DIESES PIRATENNEST, WO MAN..." */
		}
	}
}

/* Daspota <-> Ottarje: entrance to the dungeon "verlassene Herberge"("abandoned inn") */
/* (im Spiel: "Verfallene Herberge", DUNGEON_ID_VERFALLENE_HERBERGE) */
void tevent_046(void)
{
	signed int answer;
	signed int enter_inn;
	struct struct_hero *hero;

	enter_inn = 0;

	/* pause traveling */
	g_event_ani_busy = 1;

	load_ani(12);
	draw_main_screen();
	init_ani(0);

	do {
		answer = GUI_radio(get_tx2(42), 2, get_tx2(43),	get_tx2(44)); /* "VOR EUCH LIEGT EINE VERLASSEN..." / "IN DEM GEMÄUER EINKEHREN?" / "LIEBER WEITERREISEN?" */

	} while (answer == -1);

	if (answer == 1)
	{
		/* make a camp */
		gs_camp_incident = 1; /* Zwischenfall in der 2. Wache; do_wildcamp() ersetzt den Wert durch den Wachhabenden */
		gs_town_loc_type = LOCTYPE_WILDCAMP;
		do_location();
		gs_town_loc_type = LOCTYPE_NONE;
		TRV_load_textfile(-1);

		hero = (struct struct_hero*)(gs_camp_incident != -1 ? (uint8_t*)get_hero(gs_camp_incident) : (uint8_t*)get_first_hero_available_in_group());
		gs_camp_incident = -1;

		if (test_talent(hero, TA_SINNESSCHAERFE, 0) > 0)
		{
			sprintf(g_dtp2,	get_tx2(45), hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST)); /* "%s SCHRECKT WÄHREND DER WACHE PLÖTZLICH AUF. IRGENDWO AUS DEM@UNTER..." */

			do {
				answer = GUI_radio(g_dtp2, 2, get_tx2(46), get_tx2(47)); /* "DIE KLAPPE ÖFFNEN UND NACHSEHEN?" / "DIE RATTEN WEITER IM VORRATSKELLER RUMOREN..." */

			} while (answer == -1);

			if (answer == 1)
			{
				gs_travel_detour = DUNGEON_ID_VERFALLENE_HERBERGE;
				enter_inn = 1;
			}
		}

		if (!enter_inn)
		{
			sprintf(g_dtp2, get_tx2(48), hero->alias); /* "INSTINKTIV FAHRT IHR HOCH, WEIL IHR EUCH IN GEFAHR GLAUBT. IN DER T..." */
			GUI_output(g_dtp2);

			g_fig_discard = 1;

			TRV_fight_event(FIGHT_ID_F046, 46);

			do {
				answer = GUI_radio(get_tx2(49), 2, get_tx2(50), get_tx2(51)); /* "VON IRGENDWOHER MÜSSEN DIESE..." / "IN DEN KELLER EINDINGEN?" / "EURE SACHEN PACKEN UND VERSCH..." */

			} while (answer == -1);

			if (answer == 1)
			{
				gs_travel_detour = DUNGEON_ID_VERFALLENE_HERBERGE;
			}
		}
	}

	disable_ani();
	/* resume traveling */
	g_event_ani_busy = 0;
	g_request_refresh = 1;
}

/* Daspota <-> Ottarje: alte Feuerstelle. idealer Rastplatz. */
/* (Beschreibung zufaellig, siehe tevent_022; danach evtl. Karenjagd) */
void tevent_048(void)
{
	signed int answer;

	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 3) > 0 && !gs_tevent048_flag) ||
		gs_tevent048_flag)
	{
		gs_tevent048_flag = 1;

		if (TRV_found_camp_place(0))
		{
			if ((test_talent(get_first_hero_available_in_group(), TA_FAEHRTENSUCHEN, 3) > 0 && !gs_tevent048_track_flag) ||
				gs_tevent048_track_flag)
			{
				/* Spuren einer kleinen Karenherde */
				do {
					answer = GUI_radio(get_tx2(52), 2, get_tx2(53), get_tx2(54)); /* "NACH EURER RAST BEMERKT IHR E..." / "DIE SPUR AUFNEHMEN?" / "EURE SACHEN PACKEN UND WEITER..." */

				} while (answer == -1);

				if (answer == 1)
				{
					TRV_hunt_generic(21, 55, -2, 2, 8, 3, 5, 2, 4, 25, 12);
					gs_tevent048_track_flag = 1;
				}
			}
		}
	}
}

/* Ottarje <-> Skjal: Rastplatz */
void tevent_049(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 2) > 0 && !gs_tevent049_flag) ||
		gs_tevent049_flag)
	{
		TRV_found_camp_place(0);
		gs_tevent049_flag = 1;
	}
}

/* Ottarje <-> Skjal: Nahrungsplatz, ggf. mit Kraeuterhinweis (der wegen Text 16 nie erscheint) */
void tevent_050(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 3) > 0 && !gs_tevent050_flag) ||
		gs_tevent050_flag)
	{
		gs_tevent050_flag = 1;

		if ((test_talent(get_first_hero_available_in_group(), TA_PFLANZENKUNDE, 4) > 0 && !gs_tevent050_herb_flag) ||
			gs_tevent050_herb_flag)
		{
			TRV_found_replenish_place(gs_tevent050_herb_flag = 1);
		} else {
			TRV_found_replenish_place(0);
		}
	}
}
