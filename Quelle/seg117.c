/*
 * seg117 - Reiseereignisse 9/10: Zufallsbegegnungen, Jagd, Seeungeheuer und der Weg zu Hyggeliks Ruine
 *
 * Inhalt:    journey_random_encounter() waehlt nach der Landschaft des Wegs eine Zufallsbegegnung
 *            (Wildnis-Kaempfe WILD1-7, Jagd auf Karene, Wildschwein, Auerochse, Nashorn, Hoehlenbaer,
 *            Kvillotter). Auf See: Krakenmolch (octopus_attack) und Piraten (pirates_attack).
 *            do_wild8_fight() ist der Wegkampf von trv_do_journey(). TLK_way_to_ruin() fuehrt durch
 *            F115.TLK (Weg zu Hyggeliks Ruine, tevent_031/tevent_115); tevent_087 ist die erste
 *            Begegnung mit dem Einhorn (INFO6.TLK). Texte der Begegnungen in WILD.LTX.
 * Aufrufer:  trv_do_journey() (seg094), Seereise (seg063 ueber seg059), TRV_event() (seg109),
 *            talk_switch() (seg030), tevent_031 (seg110).
 * Daten:     g_trv_route_landscape_type_table (Landschaft 0..7 je Landweg),
 *            g_trv_journey_random_encounter_stats[14][7] (Schwellen je Begegnung und Landschaft).
 * Besonders: Original-Bug 21, 22, 60 und weitere markierte. Neu: journey_random_encounter() prueft
 *            "Schwelle <= W100" - es kann nur die erste Begegnung einer Spalte eintreten (Kampf WILD1);
 *            Landschaft 7 liest hinter die 7 Spalten (dann nur Karenjagd). hunt_cavebear() wertet
 *            Attacke und Parade verkehrt herum. hunt_viper(): die Schlange beisst nie.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg117.c
 */

#include <stdio.h>
#include <string.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg004.h"
#include "seg007.h"
#include "seg025.h"
#include "seg026.h"
#include "seg027.h"
#include "seg029.h"
#include "seg030.h"
#include "seg032.h"
#include "seg047.h"
#include "seg048.h"
#include "seg097.h"
#include "seg103.h"
#include "seg105.h"
#include "seg109.h"
#ifdef M302de_ORIGINAL_BUGFIX
/* Original-Bug 22: see below */
#include "seg113.h"
#endif

/* REMARK: valid values for this array are in {0, ..., 7} */
/* Fehler im Original: g_trv_journey_random_encounter_stats hat nur 7 Spalten (0..6); Wert 7 liest
 * stats[id][7] = stats[id + 1][0] (bei id 13 schon g_tlk_ruin_hero_counter). */
static unsigned char g_trv_route_landscape_type_table[59] = {
	1, // LROUTE_ID_THORWAL_VAERMHAG
	1, // LROUTE_ID_VAERMHAG_VARNHEIM
	1, // LROUTE_ID_VARNHEIM_DASPOTA
	1, // LROUTE_ID_THORWAL_SERSKE
	1, // LROUTE_ID_SERSKE_MERSKE
	1, // LROUTE_ID_MERSKE_EFFERDUN
	3, // LROUTE_ID_SERSKE_BREIDA
	0, // LROUTE_ID_F_TJOILA_TJOILA
	2, // LROUTE_ID_TJOILA_BREIDA
	3, // LROUTE_ID_BREIDA_PEILINEN
	3, // LROUTE_ID_PEILINEN_ROVAMUND
	4, // LROUTE_ID_ROVAMUND_NORDVEST
	4, // LROUTE_ID_NORDVEST_KRAVIK
	4, // LROUTE_ID_KRAVIK_SKELELLE
	1, // LROUTE_ID_THORWAL_F_TJOILA
	1, // LROUTE_ID_F_TJOILA_RUKIAN
	1, // LROUTE_ID_RUKIAN_F_ANGBOD
	3, // LROUTE_ID_F_ANGBOD_AUPLOG
	0, // LROUTE_ID_F_ANGBOD_ANGBODIR
	3, // LROUTE_ID_AUPLOG_VILNHEIM
	0, // LROUTE_ID_VILNHEIM_BODON
	5, // LROUTE_ID_VILNHEIM_PHEXCAER
	5, // LROUTE_ID_PHEXCAER_GROENVEL
	5, // LROUTE_ID_PHEXCAER_EINSIEDL
	4, // LROUTE_ID_VARNHEIM_AUPLOG
	4, // LROUTE_ID_DASPOTA_RYBON
	1, // LROUTE_ID_DASPOTA_OTTARJE
	1, // LROUTE_ID_OTTARJE_SKJAL
	1, // LROUTE_ID_SKJAL_PREM
	3, // LROUTE_ID_PREM_KORD
	7, // LROUTE_ID_OTTARJE_ORVIL
	4, // LROUTE_ID_ORVIL_ALA
	4, // LROUTE_ID_ALA_TJANSET
	1, // LROUTE_ID_TJANSET_LISKOR
	1, // LROUTE_ID_LISKOR_CLANEGH
	4, // LROUTE_ID_ALA_THOSS
	3, // LROUTE_ID_THOSS_LISKOR
	4, // LROUTE_ID_TJANSET_THOSS
	4, // LROUTE_ID_VILNHEIM_OBERORKE
	4, // LROUTE_ID_OBERORKE_FELSTEYN
	6, // LROUTE_ID_FELSTEYN_ORKANGER
	4, // LROUTE_ID_ORKANGER_CLANEGH
	3, // LROUTE_ID_CLANEGH_TYLDON
	1, // LROUTE_ID_TYLDON_VIDSAND
	5, // LROUTE_ID_OBERORKE_EINSIEDL
	5, // LROUTE_ID_FELSTEYN_EINSIEDL
	6, // LROUTE_ID_RYBON_THOSS
	7, // LROUTE_ID_SKJAL_ORVIL
	7, // LROUTE_ID_SKELELLE_PHEXCAER
	7, // LROUTE_ID_MERSKE_ROVAMUND
	1, // LROUTE_ID_ORVIL_ROVIK
	1, // LROUTE_ID_LJASDAHL__CIRCULAR
	1, // LROUTE_ID_LJASDAHL_HJALLA_H
	1, // LROUTE_ID_RUNINSHA_L_RUNIN__1
	1, // LROUTE_ID_RUNINSHA_L_RUNIN__MIDDLE
	7, // LROUTE_ID_BRENDHIL_MANRIN
	5, // LROUTE_ID_EINSIEDL__CIRCULAR
	7, // LROUTE_ID_L_RUNIN_RUNINSHA__2
	7  // LROUTE_ID__CROSSLINK
}; // ds:0xb17e

/* In the following table, the columns correspond to the type of landscape (as stored in g_trv_route_landscape_type_table),
 * and the rows correspond to the random encounter events.
 * An entry 0 means that this encounter cannot happen in the given landscape.
 * Within each column, the nonzero entries are monotonically increasing.
 * They describe the chance that the specific random encounter happens in this landscape type.
 *
 * For example, look at the first column [40, 0, 0, 45, 60, 70, 85, 0, 0, 95, 0, 100, 0, 0].
 * From the zero entries: The random event IDs 1, 2, 7, 8, 10, 12, 13 cannot happen in the given landscape type.
 * Random event ID 0 happens if D100 <= 40, i.e., there is a chance of 40%.
 * Random event ID 3 happens if D100 <= 45, and additionally, no event with a smaller ID happens, i.e., there is a chance of 5%.
 * In this way, we get the following chances for the remaining random events:
 * ID 4: 15%; ID 5: 10%, ID 6: 15%, ID 9: 10%, ID 11: 5%
 */
/* Achtung: so ist die Tabelle gemeint, journey_random_encounter() prueft aber "Wert <= W100".
 * Tatsaechlich tritt nur die erste Begegnung einer Spalte ein (bei Spalte 0: ID 0 fuer W100 >= 40),
 * alle anderen nie. */
static const uint8_t g_trv_journey_random_encounter_stats[14][7] = {
	{  40,  40,  30,  25,  30,  25,  20 },
	{   0,  45,   0,   0,  40,  30,   0 },
	{   0,  50,   0,   0,  45,   0,   0 },
	{  45,  55,  35,  30,  55,  40,  30 },
	{  60,   0,  45,  40,   0,  45,  40 },
	{  70,  65,  50,  50,  65,  65,  45 },
	{  85,  70,  60,  65,   0,  80,  60 },
	{   0,  85,   0,   0,  85,   0,   0 },
	{   0,  95,   0,   0,  95,   0,   0 },
	{  95, 100,  85,  85,   0,  85,  80 },
	{   0,   0,  95,  95,   0,   0,  90 },
	{ 100,   0, 100, 100,   0,   0,  95 },
	{   0,   0,   0,   0, 100,  95,   0 },
	{   0,   0,   0,   0,   0, 100, 100 }
}; // ds:0xb1b9

static signed int g_tlk_ruin_hero_counter = 0; // ds:0xb21b
static unsigned char g_unkn_068[1] = { 0x00 }; // ds:0xb21d (nirgends benutzt)


unsigned char g_event_ani_busy;			//ds:0xe5d2
signed int g_wallclock_update_bak;	//ds:0xe5d3
signed int g_textbox_width_bak;		//ds:0xe5d5
signed int g_basepos_y_bak;		//ds:0xe5d7
signed int g_basepos_x_bak;		//ds:0xe5d9

/* Animation zeigen, WILD.LTX laden, Kartenmodus und Uhr sichern und abschalten */
static void pause_traveling(const signed int ani_id)
{
	g_event_ani_busy = 1;

	load_ani(ani_id);

	draw_main_screen();

	init_ani(0);

	load_tx2(ARCHIVE_FILE_WILD_LTX);

	g_basepos_x_bak = g_basepos_x;
	g_basepos_y_bak = g_basepos_y;

	g_textbox_width_bak = g_textbox_width;
	g_wallclock_update_bak = g_wallclock_update;

	g_travel_event_active = 1;

	/* c = b = a = 0 */
	gs_show_travel_map = g_basepos_x = g_wallclock_update = 0;

	g_basepos_y = (ani_id == 21 ? 60: 70);
	g_textbox_width = 9;
}

/* Gegenstueck zu pause_traveling(): gesicherte Werte zurueck, Karte neu zeichnen */
static void resume_traveling(void)
{
	g_basepos_x = g_basepos_x_bak;
	g_basepos_y = g_basepos_y_bak;

	g_textbox_width = g_textbox_width_bak;
	g_wallclock_update = g_wallclock_update_bak;

	disable_ani();

	g_request_refresh = gs_show_travel_map = 1;

	g_event_ani_busy = 0;
	g_travel_event_active = 0;
}

void hunt_karen(void)
{
	signed int answer;
	struct struct_hero *hero;
	signed int passed;
	signed int i;

	pause_traveling(21);

	do {
		answer = GUI_radio(get_tx2(0), 2, get_tx2(1), get_tx2(2)); /* "IN EINIGER ENTFERNUNG VOR EUC..." / "VERSUCHEN, EIN ODER MEHRERE T..." / "WEITERZIEHEN?" */

	} while (answer == -1);

	if (answer == 1) {
		/* check for a hunting weapon */
		if (
			(get_first_hero_with_item(ITEM_ID_KURZBOGEN) != -1)
			|| (get_first_hero_with_item(ITEM_ID_LANGBOGEN) != -1)
			|| (get_first_hero_with_item(ITEM_ID_ARMBRUST) != -1)
#ifndef M302de_ORIGINAL_BUGFIX
	/* Original-Bug 60:
	 * Speer is a melee weapon. */
			|| (get_first_hero_with_item(ITEM_ID_SPEER) != -1)
#endif
		) {

			hero = get_hero(0);
			/* make a STEALTH+2 test and count the heroes who passed it */
			for (i = passed = 0; i <= 6; i++, hero++) {

				if ((hero->typus != HERO_TYPE_NONE) &&
					(hero->group_id == gs_active_group_id) &&
					!hero->flags.dead &&
					(test_talent(hero, TA_SCHLEICHEN, 2) > 0))
				{
					passed++;
				}
			}

			if (count_heroes_in_group() <= passed) {
				/* all heroes passed STEALTH */

				GUI_output(get_tx2(5)); /* "IHR ERHEBT EUCH AUS EURER DECKUNG UND NEHMT DIE TIERE AUFS KORN..." */

				/* make a MISSLE WEAPON+0 test and count the heroes who passed it */
				hero = get_hero(0);
				for (i = passed = 0; i <= 6; i++, hero++) {

					if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
						!hero->flags.dead && (test_talent(hero, TA_SCHUSSWAFFEN, 0) > 0))
					{
						passed++;
					}
				}

#if FIX_0375
				/* Fix 375: mindestens die Haelfte (aufgerundet), ein Einzelheld muss treffen */
				if (((count_heroes_in_group() + 1) / 2) <= passed) {
#else
				if ((count_heroes_in_group() / 2) <= passed) {
#endif
					/* over the half of the group passed MISSLE WEAPON+0 */
					/* (genauer: mindestens die Haelfte, abgerundet) */

					GUI_output(get_tx2(8)); /* "BLATTSCHUSS! ZWEI KARENE BRECHEN WIE VOM BLITZ GETROFFEN ZUSAMMEN." */
					/* get 80 FOOD PACKAGES */
					give_new_item_to_group(ITEM_ID_PROVIANTPAKET, 1, 80);

				} else if (passed) {
					/* at least one of the group passed MISSLE WEAPON+0 */

					GUI_output(get_tx2(7)); /* "EINES DER TIERE BRICHT WAIDWUND GETROFFEN ZUSAMMEN." */
					/* get 40 FOOD PACKAGES */
					give_new_item_to_group(ITEM_ID_PROVIANTPAKET, 1, 40);
				} else {
					/* everybody failed MISSLE WEAPON+0 */
					GUI_output(get_tx2(6)); /* "FIRUN IST EUCH HEUTE NICHT GNÄDIG! ALLE PFEILE VERFEHLEN IHRE ZIELE!" */
				}
			} else {
				/* at least one hero failed STEALTH+2 */
				GUI_output(get_tx2(4)); /* "DIE KARENE HABEN EUCH GEWITTERT! IN WILDER FLUCHT SPRENGEN SIE DAVON!" */
			}
		} else {
			/* no hunting weapon in the group */
			GUI_output(get_tx2(9)); /* "IHR HABT KEINE ENTSPRECHENDEN WAFFEN!" */
		}
	}

	resume_traveling();
}

void hunt_wildboar(void)
{
	signed int answer;
	struct struct_hero *hero;
	signed int i;
	signed int passed;

	pause_traveling(25);

	do {
		answer = GUI_radio(get_tx2(10), 2, get_tx2(11), get_tx2(12)); /* "AUF EINER LICHTUNG KÖNNT IHR..." / "VERSUCHEN, EIN WILDSCHWEIN ZU..." / "WEITERZIEHEN?" */

	} while (answer == -1);

	if (answer == 1) {
		/* check for a hunting weapon */
		if (
			(get_first_hero_with_item(ITEM_ID_KURZBOGEN) != -1)
			|| (get_first_hero_with_item(ITEM_ID_LANGBOGEN) != -1)
			|| (get_first_hero_with_item(ITEM_ID_ARMBRUST) != -1)
#ifndef M302de_ORIGINAL_BUGFIX
	/* Original-Bug 60:
	 * Speer is a melee weapon. */
			|| (get_first_hero_with_item(ITEM_ID_SPEER) != -1)
#endif
		) {

			hero = get_hero(0);
			/* make a STEALTH+0 test and count the heroes who passed it */
			for (i = passed = 0; i <= 6; i++, hero++) {

				if ((hero->typus != HERO_TYPE_NONE) &&
					(hero->group_id == gs_active_group_id) &&
					!hero->flags.dead &&
					(test_talent(hero, TA_SCHLEICHEN, 0) > 0))
				{
					passed++;
				}
			}

			if (count_heroes_in_group() <= passed) {
				/* all heroes passed STEALTH */

				GUI_output(get_tx2(15)); /* "IHR ERHEBT EUCH AUS EURER DECKUNG UND NEHMT DIE TIERE AUFS KORN..." */

				/* make a MISSLE WEAPON+0 test and count the heroes who passed it */
				hero = get_hero(0);
				for (i = passed = 0; i <= 6; i++, hero++) {

					if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
						!hero->flags.dead && (test_talent(hero, TA_SCHUSSWAFFEN, 0) > 0))
					{
						passed++;
					}
				}

#if FIX_0375
				/* Fix 375: mindestens die Haelfte (aufgerundet), ein Einzelheld muss treffen */
				if (((count_heroes_in_group() + 1) / 2) <= passed) {
#else
				if ((count_heroes_in_group() / 2) <= passed) {
#endif
					/* over the half of the group passed MISSLE WEAPON+0 */

					GUI_output(get_tx2(17)); /* "EINES DER TIERE WIRD GETROFFEN, ZUCKT AUF UND BRICHT ZUSAMMEN." */
					/* get 30 FOOD PACKAGES */
					give_new_item_to_group(ITEM_ID_PROVIANTPAKET, 1, 30);

				} else {
					/* everybody failed MISSLE WEAPON+0 */
					/* (genauer: weniger als die Haelfte traf; der Text "ALLE PFEILE VERFEHLEN" kommt
					 *  auch, wenn einzelne trafen) */
					GUI_output(get_tx2(16)); /* "FIRUN IST EUCH HEUTE NICHT GNÄDIG! ALLE PFEILE VERFEHLEN IHRE ZIELE!" */
				}
			} else {
				/* at least one hero failed STEALTH+0 */
				GUI_output(get_tx2(14)); /* "DIE WILDSCHWEINE HABEN EUCH GEWITTERT! IN WILDER FLUCHT SPRENGEN DI..." */
			}
		} else {
			/* no hunting weapon in the group */
			GUI_output(get_tx2(18)); /* "IHR HABT KEINE ENTSPRECHENDEN WAFFEN!" */
		}
	}

	resume_traveling();
}

void hunt_cavebear(void)
{
	struct struct_hero *hero;
	signed int answer;
	signed int i;

	pause_traveling(32);

	do {
		answer = GUI_radio(get_tx2(19), 2, get_tx2(20), get_tx2(21)); /* "IHR HABT EINEN HÖHLENBÄREN AU..." / "EUER HEIL IN DER FLUCHT SUCHEN?" / "EUCH DEM HÖHLENBÄREN STELLEN?" */

	} while (answer == -1);

	if (answer == 1) {

		/* print message */
		GUI_output(get_tx2(24)); /* "IHR ERGREIFT DIE FLUCHT, DEN HEISSEN ATEM DES BÄREN STÄNDIG IM GENI..." */

		/* time to flee = 1.5 hour */
		timewarp(MINUTES(90));

	} else {
		GUI_output(get_tx2(22)); /* "EIN KAMPF ENTBRENNT..." */

		hero = get_hero(0);
		for (i = 0; i <= 6; i++, hero++) {

			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !hero->flags.dead)
			{

				/* AT of the current weapon - (RS-BE / 2) <= 1W20 */
				/* Fehler im Original: Vergleiche verkehrt herum (wie im Krakenmolch-Kampf): die 5 AP gibt
				 * es bei misslungener Attacke (W20 >= AT), den Schaden bei GELUNGENER Parade (W20 < PA). */
#if FIX_0177
				/* Fix 177: Attacke gelingt bei W20 <= AT */
				if (random_schick(20) <= (hero->at_talent_bonus[hero->weapon_type] - (hero->rs_be / 2)))
#else
				if ((hero->at_talent_bonus[hero->weapon_type] - (hero->rs_be / 2)) <= random_schick(20))
#endif
				{
#if !defined(__BORLANDC__)
					D1_INFO("%-16s erhaelt 5 AP fuer eine gelungene Attacke.\n", hero->alias);
#endif
					add_hero_ap(hero, 5);
				}

				/* PA of the current weapon - (RS-BE / 2) <= 1W20 */
#if FIX_0177
				/* Fix 177: Schaden bei misslungener Parade (W20 > PA) */
				if (random_schick(20) > (hero->pa_talent_bonus[hero->weapon_type] - hero->rs_be / 2))
#else
				if ((hero->pa_talent_bonus[hero->weapon_type] - hero->rs_be / 2) > random_schick(20))
#endif
				{
#if !defined(__BORLANDC__)
					D1_INFO("%-16s erhaelt 3 AP fuer eine misslungene Parade.\n", hero->alias);
#endif

					add_hero_ap(hero, 3);
					sub_hero_le(hero, dice_roll(2, 6, 0));
				}
			}
		}

		GUI_output(get_tx2(23)); /* "NACH EINIGEN SCHLAGWECHSELN ZIEHT SICH DER BÄR OB EURER ÜBERLEGENHE..." */
	}

	resume_traveling();
}

/* should be static */
void hunt_viper(void)
{
	signed int choosen_hero;
	struct struct_hero *hero_i;
	signed int min_result; /* bleibt immer 0 (Original-Bug, siehe unten) */
	signed int i;
#if FIX_0178
	signed int fix_result;
#endif

	/* load snake picture */
	pause_traveling(30);

	GUI_output(get_tx2(25)); /* "IHR HABT OFFENSICHTLICH EINE KVILLOTTER AUFGESCHRECKT..." */

	hero_i = get_hero(0);

	for (i = min_result = 0; i <= 6; i++, hero_i++) {

		/* hero is valid */
		/* hero is in current group */
		/* hero is not dead */
		/* check GE+0 */
		/* Original-Bug: something was forgotten */
		/* min_result wird nie veraendert: "if (min_result)" ist immer falsch, die Kvillotter beisst
		 * nie. Vermutlich sollte min_result das kleinste Ergebnis aufnehmen (vgl. tevent_063). */
#if FIX_0178
		/* Fix 178: das schlechteste misslungene Ergebnis merken, sein Held wird gebissen */
		if ((hero_i->typus != HERO_TYPE_NONE) && (hero_i->group_id == gs_active_group_id) &&
			(!hero_i->flags.dead) && ((fix_result = test_attrib(hero_i, ATTRIB_GE, 0)) < min_result))
		{
			min_result = fix_result;
			choosen_hero = i;
		}
#else
		if ((hero_i->typus != HERO_TYPE_NONE) && (hero_i->group_id == gs_active_group_id) &&
			(!hero_i->flags.dead) && (test_attrib(hero_i, ATTRIB_GE, 0) < min_result))
		{
			/* remember the hero */
			choosen_hero = i;
		}
#endif
	}

	if (min_result) {

		/* select the chosen hero */
		hero_i = get_hero(choosen_hero);

		/* print a message */
		sprintf(g_dtp2, get_tx2(26), hero_i->alias); /* "%s WIRD INS BEIN GEBISSEN! MIT EINEM WÜTENDEN ZÜNGELN VERSCHWINDET..." */
		GUI_output(g_dtp2);

		/* hero gets 2 AP */
		add_hero_ap(hero_i, 2);

		if (random_schick(100) <= 50) {
			/* hero gets bitten */

			/* hero gets 3 AP */
			add_hero_ap(hero_i, 3);

			/* and 2 * 3W6 damage */
			sub_hero_le(hero_i, dice_roll(3, 6, 0) * 2);
		}
	} else {
		GUI_output(get_tx2(27)); /* "MIT EINEM WÜTENDEN ZISCHEN VERSCHWINDET DIE SCHLANGE IM HOHEN GRAS." */
	}

	resume_traveling();
}

void octopus_attack(void)
{
	signed int i;
	signed int hits;
	signed int tmp;
	char overboard[7];
	struct struct_hero *hero;
#if FIX_0180
	/* Fix 180: Kampf endet, wenn kein Held mehr kaempft (sonst Endlosschleife) */
	char any_hero_active;
#endif

	hits = 0;
	pause_traveling(31);
	memset(overboard, 0, 7);

	GUI_output(get_tx2(28)); /* "PLÖTZLICH FÄHRT EIN RUCK DURCH DAS SCHIFF. AN DER BORDWAND SCHLÄNGE..." */
	GUI_output(get_tx2(29)); /* "IHR ERGREIFT EURE WAFFEN UND HACKT WILD AUF DIE TENTAKEL EIN!" */

	do {
#if FIX_0180
		any_hero_active = 0;
#endif
		hero = get_hero(0);
		for (i = 0; i <= 6; i++, hero++) {

#if FIX_0086
			/* Fix 86: versteinerte, bewusstlose usw. Helden kaempfen nicht mit */
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
				check_hero(hero) && !overboard[i])
#else
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
				!hero->flags.dead && !overboard[i])
#endif
			{
#if FIX_0180
				any_hero_active = 1;
#endif
#if !FIX_0179
				/* Original-Bug 21:
				 * Apparently, the outcome of the following GE-attrib-test is misinterpreted.
				 * For GE <= 17, its always 5% strangling attack, 5% grabbing attack, 90% hero attacks octopus.
				 * For GE = 18 its 5% strangling attack, 95% hero attacks octopus.
				 * For GE <= 19 it is 100% hero attacks octopus.
				 * (Korrektur: gemeint ist GE >= 19)
				 * The intended behavior probably was: critical failure: grabbing attack; normal failure: strangling attack; success: hero attacks octopus. */
				if (!(tmp = test_attrib(hero, ATTRIB_GE, 0)))
#else
				/* Fix 179: misslungene GE-Probe = Wuergen, Patzer (-99) = Packen */
				tmp = test_attrib(hero, ATTRIB_GE, 0);
				if (tmp <= 0 && tmp != -99)
#endif
				{
					/* strangling attack */
					/* <hero> GERAET IN DEN WUERGEGRIFF DES KRAKENMOLCHS UND KANN SICH NUR UNTER MUEHEN WIEDER BEFREIEN. */

					add_hero_ap(hero, 5);
					sub_hero_le(hero, random_schick(6));
					sprintf(g_dtp2, get_tx2(30), hero->alias); /* "%s GERÄT IN DEN WÜRGEGRIFF DES KRAKENMOLCHS UND KANN SICH NUR UNTER..." */
					GUI_output(g_dtp2);
				}

#if !FIX_0179
				/* Original-Bug 21: see above */
				else if (tmp == -1)
#else
				else if (tmp == -99)
#endif
				{
					/* grabbing attack */
					/* <hero> WIRD VON EINEM TENTAKEL GEPACKT UND UEBER BORD GERISSEN! */

					add_hero_ap(hero, 20);
					sub_hero_le(hero, random_schick(6));
					sprintf(g_dtp2, get_tx2(31), hero->alias); /* "%s WIRD VON EINEM TENTAKEL GEPACKT UND ÜBER BORD GERISSEN!" */
					GUI_output(g_dtp2);

					if (test_talent(hero, TA_SCHWIMMEN, 0) <= 0) {
						sub_hero_le(hero, random_schick(6));
						overboard[i] = 1;
					}

				} else {
					/* chance to hit the beast */
					/* Original-Bug: Petrified, sleeping etc. heroes may hit the octopus (found by siebenstreich 2021-08-12 https://www.crystals-dsa-foren.de/showthread.php?tid=4589&pid=167425#pid167425) */

					/* Original-Bug: The following is the wrong way round, should be ">= random_schick(20)" (found by siebenstreich 2021-08-12 https://www.crystals-dsa-foren.de/showthread.php?tid=4589&pid=167430#pid167430) */
					/* Further dubiosity: Only the bare HERO_AT value of weapon type of the equipped weapon is taken into account. HERO_AT_MOD (modifier of the current weapon) and HERO_RS_BE are ignored. */
#if FIX_0177
					/* Fix 177: Treffer bei W20 <= AT */
					if (random_schick(20) <= hero->at_talent_bonus[hero->weapon_type]) {
#else
					if (hero->at_talent_bonus[hero->weapon_type] <= random_schick(20)) {
#endif
						hits++;
					}
				}
			}
		}
	}
#if !FIX_0180
	/* Original-Bug 22: infinite loop if all heroes are overboard/dead */
	while (hits <= 5);
#else
	/* (anders als Bright-Eyes verschwinden die Helden dabei nicht: ueber Bord Gerissene bleiben wie im
	 *  Original in der Gruppe) */
	while ((hits <= 5) && any_hero_active);
#endif

	sub_group_le(3);
	add_hero_ap_all(5);
	GUI_output(get_tx2(32)); /* "NACH EINIGEN HEFTIGEN SCHLÄGEN FLIEHT DER KRAKENMOLCH INS OFFENE MEER." */

	g_basepos_x = g_basepos_y = 0;
	status_menu(get_hero_pos(get_first_hero_available_in_group()));
	resume_traveling();
}

void hunt_bison(void)
{
	signed int answer;
	struct struct_hero *hero;

	pause_traveling(29);

	do {
		answer = GUI_radio(get_tx2(33), 2, get_tx2(34), get_tx2(35)); /* "AUF EINER KLEINEN LICHTUNG VO..." / "DAS OFFENSICHTLICH NERVÖSE TI..." / "DEN AUEROCHSEN ERLEGEN?" */

	} while (answer == -1);

	if (answer == 1) {

		timewarp(MINUTES(30));
		GUI_output(get_tx2(38)); /* "NACH ETWA EINER HALBEN STUNDE GELANGT IHR WIEDER AUF DEN WEG." */

	} else {
		GUI_output(get_tx2(36)); /* "ALS IHR EUCH NÄHERT, SCHRECKT DER AUEROCHSE AUF UND ERGREIFT DIE FL..." */

		hero = get_hero(get_random_hero());
		sprintf(g_dtp2, get_tx2(37), hero->alias); /* "DABEI RENNT ER EUCH FAST ÜBER DEN HAUFEN. %s KANN SICH NUR MIT EINE..." */
		GUI_output(g_dtp2);

		sub_hero_le(hero, random_schick(6));
		add_hero_ap(hero, 2);
	}

	resume_traveling();
}

void hunt_rhino(void)
{
	signed int answer;
	struct struct_hero *hero;

	pause_traveling(33);

	do {
		answer = GUI_radio(get_tx2(39), 2, get_tx2(40), get_tx2(41)); /* "VOR EUCH GRAST FRIEDLICH EIN..." / "DEN MASSIVEN FLEISCHBERG UMGE..." / "DAS NASHORN NIEDERMACHEN?" */

	} while (answer == -1);

	if (answer == 1) {

		timewarp(MINUTES(30));
		GUI_output(get_tx2(44)); /* "NACH ETWA EINER HALBEN STUNDE GELANGT IHR WIEDER AUF DEN WEG." */

	} else {
		GUI_output(get_tx2(42)); /* "ALS IHR EUCH NÄHERT, ZUCKT DER KOPF DES NASHORNS HOCH. KLEINE AUGEN..." */

		hero = get_hero(get_random_hero());
		sprintf(g_dtp2, get_tx2(43), hero->alias); /* "URPLÖTZLICH STÜRMT DAS TIER AUF EUCH ZU! IHR HECHTET IN DIE BÜSCHE,..." */
		GUI_output(g_dtp2);
		sub_hero_le(hero, dice_roll(2, 6, 0));
		add_hero_ap(hero, 6);
	}

	resume_traveling();
}

void pirates_attack(void)
{
	pause_traveling(9);

	GUI_output(get_tx2(45)); /* "AM HORIZONT KÖNNT IHR EIN WEISS-ROT GESTREIFTES SEGEL ENTDECKEN!" */
	GUI_output(get_tx2(46)); /* "DAS SCHIFF KOMMT LANGSAM NÄHER. ES NIMMT KURS AUF EUCH!" */
	GUI_output(get_tx2(47)); /* "DANN ENTDECKT IHR MIT SCHRECKEN, DASS ES SICH UM PIRATEN HANDELT, D..." */

	g_max_enemies = random_interval(3, 8);
	g_fig_discard = 1;

#if FIX_0327
	/* Fix 327 (teilweise): nach verlorenem oder per VERLASSEN/LADEN beendetem Kampf kein Ausruestungsbildschirm */
	if (do_fight(FIGHT_ID_S001) < 2) {
		g_basepos_x = g_basepos_y = 0;

		status_menu(get_hero_pos(get_first_hero_available_in_group()));
	} else {
		g_basepos_x = g_basepos_y = 0;
	}
#else
	do_fight(FIGHT_ID_S001);

	g_basepos_x = g_basepos_y = 0;

	status_menu(get_hero_pos(get_first_hero_available_in_group()));
#endif

	resume_traveling();
}

/* Wegkampf aus trv_do_journey() (gs_journey_fight_flag): 5-10 Gegner, Kampf WILD8 */
void do_wild8_fight(void)
{
	signed int x_bak;
	signed int y_bak;

	x_bak = g_basepos_x;
	y_bak = g_basepos_y;
	g_wallclock_update_bak = g_wallclock_update;
	g_basepos_x = 0;
	g_basepos_y = 0;
	gs_show_travel_map = 0;

	g_max_enemies = random_interval(5, 10);
	g_fig_discard = 1;

	do_fight(FIGHT_ID_WILD8);

	gs_journey_fight_flag = 0;
	gs_show_travel_map = 1;
	g_basepos_x = x_bak;
	g_basepos_y = y_bak;
}

/**
 * \brief   random encounter traveling a land route
 *
 * \param   tmp   ID of the traveled land route
 */
void journey_random_encounter(signed int tmp)
	/* tmp is dual use for land_route_id and as index for g_trv_journey_random_encounter_stats entries */
{
	signed int skip_fight = 0;
	signed int random_encounter_id;
	signed int roll_d100;
	signed int x_bak;
	signed int y_bak;
	signed int wallclock_update_bak;

	/* fights happen only with a chance of 33% */
	if (random_schick(100) > 33) {
		skip_fight = 1;
	}

	x_bak = g_basepos_x;
	y_bak = g_basepos_y;
	wallclock_update_bak = g_wallclock_update;
	g_basepos_x = 0;
	g_basepos_y = 0;

	tmp = g_trv_route_landscape_type_table[tmp - 1];
	/* now tmp is the entry in g_trv_route_landscape_type_table belonging to the traveled land_route_id */

	roll_d100 = random_schick(100);

	/* Fehler im Original: Bedingung "Wert <= W100" statt "W100 <= Wert". Weil die Werte je Spalte steigen,
	 * trifft immer nur die erste Zeile mit Wert != 0 zu (fuer Landschaft 0..6: Kampf WILD1 bei W100 >= 20..40,
	 * der in 67 % der Faelle wegen skip_fight entfaellt); Jagden, Schlange, Baer usw. treten nie ein,
	 * ausser bei Landschaft 7 (siehe g_trv_route_landscape_type_table). */
	for (random_encounter_id = 0; random_encounter_id < 14; random_encounter_id++) {

#if FIX_0181
		/* Fix 181: aufsteigende Schwellen: W100 <= Wert */
		if ((roll_d100 <= g_trv_journey_random_encounter_stats[random_encounter_id][tmp]) && g_trv_journey_random_encounter_stats[random_encounter_id][tmp]) {
#else
		if ((g_trv_journey_random_encounter_stats[random_encounter_id][tmp] <= roll_d100) && g_trv_journey_random_encounter_stats[random_encounter_id][tmp]) {
#endif

			gs_show_travel_map = g_wallclock_update = 0;
			g_travel_event_active = 1;
			g_fig_discard = 1;

			switch (random_encounter_id) {
				case 0: {
					if (!skip_fight) {
						g_max_enemies = random_schick(6) + 1;
						do_fight(FIGHT_ID_WILD1);
					}
					break;
				}
				case 1: {
					if (!skip_fight) {
						g_max_enemies = random_schick(3);
						do_fight(FIGHT_ID_WILD2);
					}
					break;
				}
				case 2: {
					hunt_karen();
					break;
				}
				case 3: {
					if (!skip_fight) {
						g_max_enemies = random_schick(2);
						do_fight(FIGHT_ID_WILD3);
					}
					break;
				}
				case 4: {
					hunt_viper();
					break;
				}
				case 5: {
					if (!skip_fight) {
						g_max_enemies = random_schick(6) + 1;
						do_fight(FIGHT_ID_WILD4);
					}
					break;
				}
				case 6: {
					if (!skip_fight) {
						g_max_enemies = random_schick(3) + 3;
						do_fight(FIGHT_ID_WILD4);
					}
					break;
				}
				case 7: {
					if (!skip_fight) {
						g_max_enemies = random_schick(2) + 1;
						do_fight(FIGHT_ID_WILD5);
					}
					break;
				}
				case 8: {
					hunt_rhino();
					break;
				}
				case 9: {
					hunt_wildboar();
					break;
				}
				case 10: {
					if (!skip_fight) {
						g_max_enemies = random_schick(3);
						do_fight(FIGHT_ID_WILD6);
					}
					break;
				}
				case 11: {
					hunt_bison();
					break;
				}
				case 12: {
					if (!skip_fight) {
						g_max_enemies = random_schick(3);
						do_fight(FIGHT_ID_WILD7);
					}
					break;
				}
				case 13: {
					hunt_cavebear();
					break;
				}
			}

			g_fig_discard = 0;
			gs_show_travel_map = 1;
			g_travel_event_active = 0;
			break;
		}
	}

	g_basepos_x = x_bak;
	g_basepos_y = y_bak;
	g_wallclock_update = wallclock_update_bak;
	load_tx(ARCHIVE_FILE_MAPTEXT_LTX);
}

/* tevent_031 mit Hyggelik-Auftrag: Weg zur Ruine (F115.TLK, Einstieg Zustand 66) */
void search_ruin1(void)
{
	do_talk(17, 0);
	disable_ani();
}

/* Skelellen <-> Phexcaer: Weg zu Hyggeliks Ruine (F115.TLK, Einstieg Zustand 45) */
void tevent_115(void)
{
	if (gs_find_hyggelik) {

		gs_tevent115_flag = 1;

		do_talk(17, 0);

		disable_ani();
	}
}

void TLK_way_to_ruin(const signed int state)
{
	signed int i;
	struct struct_hero *hero;
	struct struct_hero *hero2 = get_first_hero_available_in_group();

	if (!state) {

		g_dialog_next_state = (gs_tevent115_flag ? 45 : 66);
		g_tlk_ruin_hero_counter = 0;

	} else if (state == 66 || state == 45) {
		show_treasure_map();
	} else if (state == 4 || state == 7) {
		timewarp(HOURS(1));
	} else if (state == 6) {
		hero = get_hero(get_random_hero());
		g_dialog_next_state = (test_talent(hero, TA_WILDNISLEBEN, 6) > 0 ? 8 : 7);
	} else if (state == 8) {
		timewarp(HOURS(1));
		TRV_ford_test(0, 30);
	} else if (state == 9) {

		do {
			hero = get_hero(g_tlk_ruin_hero_counter);
			g_tlk_ruin_hero_counter++;

			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
				!hero->flags.dead) {

				gs_ruin_hero = hero;

				break;
			}

		} while (g_tlk_ruin_hero_counter != 7);

		g_dialog_next_state = (g_tlk_ruin_hero_counter == 7 ? 13 : 10);

	} else if (state == 10) {

		g_dialog_next_state = (test_talent(gs_ruin_hero, TA_SCHWIMMEN, 5) > 0 ? 11 : 12);

	} else if (state == 12) {

		sub_hero_le(gs_ruin_hero, random_schick(4) + 1);

		/* Original-Bug: hero != RUIN_HERO */
		/* (hero ist in diesem Aufruf nicht gesetzt - die KK kommt aus einem uninitialisierten Zeiger) */
#if FIX_0182
		/* Fix 182: KK des betroffenen Helden */
		hero_disease_test(gs_ruin_hero, DISEASE_ID_DUMPFSCHAEDEL, 25 - (gs_ruin_hero->attrib[ATTRIB_KK].current + gs_ruin_hero->attrib[ATTRIB_KK].mod));
#else
		hero_disease_test(gs_ruin_hero, DISEASE_ID_DUMPFSCHAEDEL, 25 - (hero->attrib[ATTRIB_KK].current + hero->attrib[ATTRIB_KK].mod));
#endif

		lose_random_item(gs_ruin_hero, 10, get_ttx(506)); /* "OH VERDAMMT, %s HAT %s VERLOREN!" */

	} else if (state == 13 || state == 25 || state == 34 || state == 59 || state == 62) {
		timewarp(MINUTES(30));
	} else if (state == 14 || state == 64) {
		timewarp(MINUTES(10));
	} else if (state == 15 || state == 16) {
		timewarp(MINUTES(20));
	} else if (state == 17) {
		g_dialog_next_state = (test_talent(hero2, TA_ORIENTIERUNG, 5) > 0 ? 18 : 19);
	} else if (state == 19) {
		timewarp(MINUTES(20));
		gs_ruin_hero = get_hero(get_random_hero());
		g_dialog_next_state = (test_attrib(gs_ruin_hero, ATTRIB_GE, 2) > 0 ? 20 : 21);
	} else if (state == 20) {
		lose_random_item(get_hero(get_random_hero()), 5, get_ttx(506)); /* "OH VERDAMMT, %s HAT %s VERLOREN!" */
	} else if (state == 21) {
		timewarp(MINUTES(10));
		lose_random_item(hero2, 10, get_ttx(506)); /* "OH VERDAMMT, %s HAT %s VERLOREN!" */
		sub_hero_le(hero2, random_schick(4) + 2);
	} else if (state == 22 || state == 30 || state == 52 || state == 53) {
		timewarp(HOURS(3));
	} else if (state == 26 || state == 36 || state == 37 || state == 38 || state == 39) {
		timewarp(HOURS(1));
	} else if (state == 27 || state == 29 || state == 65) {
		timewarp(HOURS(2));
	} else if (state == 54) {
		timewarp(HOURS(5));
	} else if (state == 28) {

		hero = get_hero(0);

		for (i = g_tlk_ruin_hero_counter = 0; i <= 6; i++, hero++) {

			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
				!hero->flags.dead && (test_talent(hero, TA_ORIENTIERUNG, 0) > 0))
		       	{
				g_tlk_ruin_hero_counter++;
			}
		}

		g_dialog_next_state = ((count_heroes_in_group() >> 1) < g_tlk_ruin_hero_counter ? 29 : 30);

	} else if (state == 41) {
		g_event_ani_busy = 1;
		load_ani(11);
		draw_main_screen();
		init_ani(0);
		timewarp(MINUTES(90));
		g_request_refresh = 1;
	} else if (state == 42 || state == 60) {
		timewarp(MINUTES(150));
	} else if (state == 67 || state == 44) {
		gs_travel_detour = DUNGEON_ID_HYGGELIKS_RUINE;

	} else if (state == 48) {

		hero = get_hero(0);

		for (i = g_tlk_ruin_hero_counter = 0; i <= 6; i++, hero++) {

			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
				!hero->flags.dead && (test_talent(hero, TA_ORIENTIERUNG, 0) > 0))
			{
				g_tlk_ruin_hero_counter++;
			}
		}

		g_dialog_next_state = ((count_heroes_in_group() >> 1) < g_tlk_ruin_hero_counter ? 49 : 50);
	}

	g_event_ani_busy = 0;
}

/* Oberorken <-> Einsiedlersee / Felsteyn <-> Einsiedlersee: erste Begegnung mit dem Einhorn (INFO6.TLK) */
void tevent_087(void)
{
	if (!gs_met_unicorn_flag && gs_got_main_quest) {

		do_talk(11, 2);

		gs_met_unicorn_flag = 1;
	}
}
