/*
 * seg114 - Reiseereignisse 6/10 (Ereignisse 110-123)
 *
 * Inhalt:    Ereignisse auf dem langen Weg Skelellen-Phexcaer und zwischen Merske/Rovamund und
 *            Orvil/Rovik: Felswand, Nashornjagd, Firun-Tempel in der Wildnis (Spende verbessert den
 *            Schlaf), Hochmoor mit der Hexe Olimone (Rezept fuer starken Heiltrank), Steinlawine,
 *            Woelfe greifen einen Schaefer an, Haengebruecke mit Hoehenangst sowie Rast- und Kraeuterplaetze.
 * Aufrufer:  TRV_event() (seg109).
 * Besonders: Original-Bug 53 (Umkehren). Neu: in tevent_123 wird test_attrib() mit 99 verglichen, was es
 *            nie liefert - das Verschwinden von der Bruecke (Text 48) ist unerreichbar; tevent_113 setzt
 *            g_wildcamp_sleep_quality und setzt es nicht zurueck (Bonus fuer spaetere Nachtlager);
 *            Thonnysbluete als Spezialkraut wirkt nie (siehe seg112).
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg114.c
 */
#include <stdio.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg004.h"
#include "seg007.h"
#include "seg025.h"
#include "seg026.h"
#include "seg027.h"
#include "seg029.h"
#include "seg047.h"
#include "seg096.h"
#include "seg097.h"
#include "seg098.h"
#include "seg103.h"
#include "seg105.h"
#include "seg109.h"
#include "seg113.h"
#include "seg114.h"

/* wall of rock */
/* Skelellen <-> Phexcaer */
void tevent_110(void)
{
	signed int i;
	signed int answer;
	struct struct_hero *hero;

	do {
		answer = GUI_radio(get_tx2(79), 2, get_tx2(80),	get_tx2(81)); /* "HIER ENDET DER GANGBARE PFAD,..." / "DIE FELSWAND ERKLETTERN?" / "VERSUCHEN, DAS HINDERNIS ZU U..." */

	} while (answer == -1);

	if (answer == 1)
	{
		/* try to climb */
		hero = get_hero(0);
		for (i = 0; i <= 6; i++, hero++)
		{
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
				!hero->flags.dead && test_talent(hero, TA_KLETTERN, 0) <= 0)
			{
				/* talent test failed */

				if (get_first_hero_with_item(ITEM_ID_SEIL) != -1)
				{
					/* one hero in the group has a rope */
					sprintf(g_dtp2,	get_tx2(85), hero->alias); /* "%s STÜRZT AB, FÄNGT SICH ABER GLÜCKLICHERWEISE IM SEIL." */

					sub_hero_le(hero, random_schick(10));

				} else {
					/* all heroes in the group have no ropes */
					sprintf(g_dtp2,	get_tx2(86), hero->alias); /* "UNTER %s LÖST SICH EIN FELSBLOCK, WAS EINEN HEFTIGEN STURZ ZUR FOLG..." */

					sub_hero_le(hero, random_schick(16) + 4);
				}

				GUI_output(g_dtp2);
			}

		}

		GUI_output(get_tx2(84)); /* "UNTER GROSSEN MÜHEN ERREICHT IHR ENDLICH DAS PLATEAU." */

	} else {

		/* try to walk arround */

		if (test_talent(get_first_hero_available_in_group(), TA_ORIENTIERUNG, 0) > 0)
		{
			/* success */
			timewarp(HOURS(4));
			GUI_output(get_tx2(82)); /* "ES DAUERT MEHRERE STUNDEN, BIS IHR EINEN KAMIN GEFUNDEN HABT, IN DE..." */
		} else {
			/* fail */
			timewarp(DAYS(1) + HOURS(6));
			GUI_output(get_tx2(83)); /* "SO SEHR IHR AUCH SUCHT, ES IST KEIN VERNÜNFTIGER AUFSTIEG ZU ENTDEC..." */
		}
	}
}

/* hunt a rhino */
/* Skelellen <-> Phexcaer; gs_tevent111_flag: 1 = Spur bekannt, 2 = Nashorn erlegt */
void tevent_111(void)
{
	signed int counter;
	signed int answer;
	signed int i;
	signed int ret_talent_test2;
	signed int ret_talent_test3;
	signed int lucky_tests; /* Schuesse mit Ergebnis 99 (W20 = 1, gluecklich) */
	struct struct_hero *hero;

	if ((test_talent(get_first_hero_available_in_group(), TA_FAEHRTENSUCHEN, 1) > 0 && !gs_tevent111_flag) ||
		gs_tevent111_flag == 1)
	{
		gs_tevent111_flag = 1;
		g_event_ani_busy = 1;

		load_ani(33);
		draw_main_screen();
		init_ani(0);

		GUI_output(get_tx2(0)); /* "ES IST NICHT ZU ÜBERSEHEN, DASS DIESE SPUR VON EINEM SEHR GROSSEN U..." */

		hero = get_hero(0);
		for (i = counter = 0; i <= 6; i++, hero++)
		{
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
				!hero->flags.dead && test_talent(hero, TA_SCHLEICHEN, -5) <= 0)
			{
				counter++;
			}
		}

		if (counter)
		{
			do {
				answer = GUI_radio(get_tx2(1), 2, get_tx2(7), get_tx2(8)); /* "ABER ES SCHEINT EURE NÄHE ZU..." / "DAS TIER WEITER VERFOLGEN?" / "UMKEHREN UND DEN WEG FORTSETZEN?" */

			} while (answer == -1);

			if (answer == 1)
			{
				GUI_output(get_tx2(9)); /* "IHR KÖNNT ZWAR DEUTLICH DIE SPUR ERKENNEN, DIE DAS RIESIGE TIER HIN..." */

				add_hero_ap_all(7);

				timewarp(HOURS(2));
			} else {
				add_hero_ap_all(5);
			}

		} else {

			hero = get_hero(select_hero_ok_forced(get_tx2(2))); /* "DAS NASHORN SCHEINT EUCH IMMER NOCH NICHT BEMERKT ZU HABEN. WER SOL..." */

			if (test_talent(hero, TA_SCHLEICHEN, 0) <= 0)
			{
				/* talent test failed */
				do {
					answer = GUI_radio(get_tx2(1), 2, get_tx2(7), get_tx2(8)); /* "ABER ES SCHEINT EURE NÄHE ZU..." / "DAS TIER WEITER VERFOLGEN?" / "UMKEHREN UND DEN WEG FORTSETZEN?" */

				} while (answer == -1);

				if (answer == 1)
				{
					GUI_output(get_tx2(9)); /* "IHR KÖNNT ZWAR DEUTLICH DIE SPUR ERKENNEN, DIE DAS RIESIGE TIER HIN..." */

					add_hero_ap_all(7);

					timewarp(HOURS(2));
				} else {
					add_hero_ap_all(5);
				}

			} else {
				/* talent test succeeded */

				sprintf(g_dtp2,	get_tx2(3), hero->alias); /* "%s SCHLEICHT NÄHER HERAN, ERHEBT SICH AUS SEINER DECKUNG UND SCHIES..." */

				GUI_input(g_dtp2, counter = lucky_tests = 0);

				if ((i = test_talent(hero, TA_SCHUSSWAFFEN, 12)) > 0) counter++;
				if ((ret_talent_test2 = test_talent(hero, TA_SCHUSSWAFFEN, 12)) > 0) counter++;
				if ((ret_talent_test3 = test_talent(hero, TA_SCHUSSWAFFEN, 12)) > 0) counter++;

				if (i == 99) lucky_tests++;
				if (ret_talent_test2 == 99) lucky_tests++;
				if (ret_talent_test3 == 99) lucky_tests++;

				if (counter == 3 || lucky_tests >= 2)
				{
					sprintf(g_dtp2, get_tx2(4), hero->alias); /* "BLATTSCHUSS! TÖDLICH GETROFFEN BRICHT DAS SCHWERE VIEH ZUSAMMEN UND..." */

					GUI_output(g_dtp2);

					add_hero_ap_all(5);

					add_hero_ap(hero, 20);

					timewarp(HOURS(3));

					give_new_item_to_group(ITEM_ID_PROVIANTPAKET, 1, 200);

					gs_tevent111_flag = 2;

				} else if (counter == 2 || lucky_tests == 1 || counter == 1)
				{

					do {
						answer = GUI_radio(get_tx2(5), 2, get_tx2(10), get_tx2(11)); /* "EINER DER PFEILE TRIFFT DAS N..." / "ZUR SEITE SPRINGEN UND DEM TI..." / "DAS NASHORN MIT EUREN SCHWERT..." */

					} while (answer == -1);

					if (answer == 1)
					{
						GUI_output(get_tx2(12)); /* "MIT LAUTEM RUMPELN VERSCHWINDET DAS NASHORN IM WALD. IHR KLOPFT EUC..." */

						add_hero_ap_all(7);

						add_hero_ap(hero, 5);
					} else {

						GUI_output(get_tx2(13)); /* "NACH WENIGEN AUGENBLICKEN BLUTIGEN DURCHEINANDERS RÖCHELT DAS NASHO..." */

						add_hero_ap_all(15);

						add_hero_ap(hero, 5);

						timewarp(HOURS(4));

						hero = get_hero(0);
						for (i = 0; i <= 6; i++, hero++)
						{
							if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
								!hero->flags.dead && test_attrib(hero, ATTRIB_GE, 2) <= 0)
							{
								/* attribute test failed */
								sub_hero_le(hero, random_schick(10));
							}
						}

						give_new_item_to_group(ITEM_ID_PROVIANTPAKET, 1, 180);
						gs_tevent111_flag = 2;
					}

				} else {
					sprintf(g_dtp2, get_tx2(6), hero->alias); /* "FIRUN IST %s HEUTE NICHT GNÄDIG: ALLE DREI PFEILE VERFEHLEN IHR ZIE..." */

					do {
						answer = GUI_radio(g_dtp2, 2, get_tx2(7),get_tx2(8)); /* "DAS TIER WEITER VERFOLGEN?" / "UMKEHREN UND DEN WEG FORTSETZEN?" */

					} while (answer == -1);

					if (answer == 1)
					{
						GUI_output(get_tx2(9)); /* "IHR KÖNNT ZWAR DEUTLICH DIE SPUR ERKENNEN, DIE DAS RIESIGE TIER HIN..." */

						add_hero_ap_all(7);

						timewarp(HOURS(2));
					} else {
						add_hero_ap_all(5);
					}
				}
			}
		}

		disable_ani();
		g_request_refresh = 1;
	}

	g_event_ani_busy = 0;
}

/* a camp place */
/* Skelellen <-> Phexcaer, mit Thonnysbluete (Bonus wirkungslos, siehe seg112 tevent_070) */
void tevent_112(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 2) > 0 && !gs_tevent112_flag) || gs_tevent112_flag)
	{
		gs_tevent112_flag = 1;

		if ((test_talent(get_first_hero_available_in_group(), TA_PFLANZENKUNDE, 2) > 0 && !gs_tevent112_herb_flag) || gs_tevent112_herb_flag)
		{
			gs_tevent112_herb_flag = 1;

			g_gather_herbs_special = (int8_t)ITEM_ID_THONNYSBLUETE;	/* REMARK: item_id > 127 */

			TRV_found_camp_place(2);

			g_gather_herbs_special = -1;
		} else {
			TRV_found_camp_place(0);
		}
	}
}

/* FIRUN-temple in the wilderness */
/* Skelellen <-> Phexcaer */
void tevent_113(void)
{
	signed int answer;
	int32_t est_old;
	int32_t est_diff;

	do {
		answer = GUI_radio(get_tx2(14), 2, get_tx2(15), get_tx2(16)); /* "AUF EINER KLEINEN LICHTUNG VO..." / "DEN TEMPEL BETRETEN?" / "LIEBER WEITERREISEN?" */

	} while (answer == -1);

	if (answer == 1) {

		GUI_output(get_tx2(17)); /* "IHR BETRETET DEN FIRUNTEMPEL..." */

		/* enter the temple */
		gs_town_typeindex = 58;
		est_old = gs_gods_estimation[GOD_ID_FIRUN];
		gs_town_loc_type = LOCTYPE_TEMPLE;
		do_location();
		TRV_load_textfile(-1);

		GUI_output(get_tx2(18)); /* "IHR VERLASST DEN TEMPEL UND ENTSCHEIDET EUCH, HIER AUF DER LICHTUNG..." */

		/* calculate the difference of the estimation */
		est_diff = gs_gods_estimation[GOD_ID_FIRUN] - est_old;

		g_textbox_width = 3;

		/* the more money you spend, the better the group will sleep */
		/* Fehler im Original: g_wildcamp_sleep_quality wird danach nicht auf 0 zurueckgesetzt (anders als
		 * in TRV_found_camp_place() und tevent_078); der Schlafbonus gilt auch fuer spaetere Nachtlager,
		 * bis ein Rastplatz oder das Rechtsklick-Lager ihn loescht. */
		g_wildcamp_sleep_quality = (est_diff <= 10 ? 1 : (est_diff <= 50 ? 2 : (est_diff <= 100 ? 3 : 4)));

		gs_town_loc_type = LOCTYPE_WILDCAMP;
		do_location();
		gs_town_loc_type = LOCTYPE_NONE;
#if FIX_0170
		/* Fix 170: Schlafbonus gilt nur fuer dieses Lager */
		g_wildcamp_sleep_quality = 0;
#endif

		g_textbox_width = 9;
		g_request_refresh = 2;
	}
}

/* Phexcaer <-> Skelellen: a swamp */
void tevent_114(void)
{
	signed int i;
	signed int answer;
	signed int done;
	signed int j;
	struct struct_hero *hero;

	done = 0;

	do {

		do {
			answer = GUI_radio(get_tx2(19), 2, get_tx2(20), get_tx2(21)); /* "IHR BEFINDET EUCH JETZT ZWISC..." / "UMKEHREN UND EINEN ANDEREN WE..." / "VORSICHTIG WEITERGEHEN?" */

		} while (answer == -1);

		if (answer == 1)
		{
			do {
				answer = GUI_radio(get_tx2(28), 2, get_tx2(29), get_tx2(30)); /* "SO SEHR IHR AUCH SUCHT, IHR K..." / "UMKEHREN?" / "DURCH DEN SUMPF?" */

			} while (answer == -1);

			if (answer == 1)
			{
				/* turn arround */

#if !FIX_0150
				/* Original-Bug 53:
				 * If the party was already traveling backward, the new direction should be forward. */
				gs_journey_direction = done = JOURNEY_DIRECTION_CHANGE_TO_BACKWARD; // this is value 1
#else
				/* Fix 150: Umkehren kehrt die aktuelle Richtung um */
				gs_journey_direction = (gs_journey_direction == JOURNEY_DIRECTION_FORWARD ? JOURNEY_DIRECTION_CHANGE_TO_BACKWARD : JOURNEY_DIRECTION_CHANGE_TO_FORWARD);
				done = 1;
#endif
			}

			if (answer == 2)
			{
				/* go on careful */
				GUI_output(get_tx2(22)); /* "DAS HOCHMOOR SCHEINT SICH ENDLOS DAHINZUZIEHEN..." */
			}

		} else {
			GUI_output(get_tx2(22)); /* "DAS HOCHMOOR SCHEINT SICH ENDLOS DAHINZUZIEHEN..." */
		}

		if (done == 0)
		{
			/* walk through the swamp */
			for (j = 0; j < 2; j++)
			{
				for (i = 0, hero = get_hero(0); i <= 6; i++, hero++)
				{
					if ((hero->typus != HERO_TYPE_NONE) &&
						(hero->group_id == gs_active_group_id) &&
						!hero->flags.dead &&
						test_attrib(hero, ATTRIB_GE, 4) <= 0)
					{
						/* attrib test failed */
						timewarp(MINUTES(30));

						sprintf(g_dtp2,	get_tx2(23), hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_4TH)); /* "%s SINKT PLÖTZLICH BIS ZU DEN HÜFTEN EIN UND DROHT, VOLLSTÄNDIG@UNT..." */
						GUI_output(g_dtp2);

						sub_hero_le(hero, random_schick(8));

						lose_random_item(hero, 50, get_ttx(506)); /* "OH VERDAMMT, %s HAT %s VERLOREN!" */
						lose_random_item(hero, 50, get_ttx(506)); /* "OH VERDAMMT, %s HAT %s VERLOREN!" */
						lose_random_item(hero, 50, get_ttx(506)); /* "OH VERDAMMT, %s HAT %s VERLOREN!" */
					}
				}
			}

			if (!gs_tevent114_olimone_flag)
			{
				/* meet OLIMONE */
				GUI_output(get_tx2(24)); /* "NACH FAST ACHT STUNDEN ERREICHT IHR WIEDER TROCKENEN BODEN. IHR SEI..." */

				load_in_head(12);

				GUI_dialogbox((unsigned char*)g_dtp2, get_tx(52), get_tx2(25), 0); /* "OLIMONE" / "<HEDA! SUMPFSCHRATE! AUCH SCHON DA?> ES IS..." */
				GUI_dialogbox((unsigned char*)g_dtp2, get_tx(52), get_tx2(26), 0); /* "OLIMONE" / "<ICH BEOBACHTE EUCH SCHON GERAUME ZEIT. IH..." */

				/* get RECIPE FOR POTENT HEALING POTION */
				give_new_item_to_group(ITEM_ID_REZEPT_FUER_STARKEN_HEILTRANK, 1, 1);

				/* TODO: ORIGINAL-BUG: this item could have been not taken => get_hero(-1) => SEGFAULT */
				/* (unter DOS kein Absturz: get_hero(-1) zeigt vor das Heldenfeld, der Text bekommt einen
				 *  Muellnamen; ausserdem kann ein Held gefunden werden, der das Rezept schon vorher hatte) */
#if FIX_0171
				/* Fix 171: hat niemand das Rezept, den ersten verfuegbaren Helden nennen statt get_hero(-1) */
				if ((i = get_first_hero_with_item(ITEM_ID_REZEPT_FUER_STARKEN_HEILTRANK)) != -1) {
					hero = get_hero(i);
				} else {
					hero = get_first_hero_available_in_group();
				}
#else
				hero = get_hero(get_first_hero_with_item(ITEM_ID_REZEPT_FUER_STARKEN_HEILTRANK));
#endif

				sprintf(g_dtp2,	get_tx2(27), hero->alias); /* "IHR PLAUDERT NOCH EINE WEILE, FROH ÜBER DIESE UNERWARTETE GESELLSCH..." */
				GUI_output(g_dtp2);

				timewarp(HOURS(8));

				for (i = 0, hero = get_hero(0); i <= 6; i++, hero++)
				{
					if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !hero->flags.dead)
					{
						add_hero_le(hero, 7);
					}
				}

				gs_tevent114_olimone_flag = done = 1;

			} else {
				GUI_output(get_tx2(64)); /* "NACH FAST ACHT STUNDEN ERREICHT IHR WIEDER TROCKENEN BODEN. IHR SEI..." */

				timewarp(HOURS(8));

				for (i = 0, hero = get_hero(0); i <= 6; i++, hero++)
				{
					if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !hero->flags.dead)
					{
						add_hero_le(hero, 4);
					}
				}

				done = 1;
			}
		}

	} while (done == 0);
}

/* a camp place */
/* Skelellen <-> Phexcaer, guter Jagdplatz */
void tevent_116(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 6) > 0 && !gs_tevent116_flag) ||
		gs_tevent116_flag)
	{
		gs_tevent116_flag = 1;
		TRV_found_camp_place(1);
	}
}

/* an avalance */
/* Skelellen <-> Phexcaer: Steinschlag, der Weg zurueck ist versperrt */
void tevent_117(void)
{
	signed int i;
	struct struct_hero *hero;

	GUI_output(get_tx2(31)); /* "OHNE GROSSE VORWARNUNG LÖST SICH ÜBER EUCH PLÖTZLICH EIN FELSBROCKE..." */
	GUI_output(get_tx2(32)); /* "IHR HECHTET IN DIE DECKUNG EINES ÜBERHANGS..." */

	for (hero = get_hero(0), i = 0; i <= 6; i++, hero++)
	{
		if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
			!hero->flags.dead && test_attrib(hero, ATTRIB_GE, 0) <= 0)
		{
			/* attrib test failed */
			sub_hero_le(hero, random_schick(11) + 1);

			lose_random_item(hero, 15, get_ttx(506)); /* "OH VERDAMMT, %s HAT %s VERLOREN!" */
			lose_random_item(hero, 15, get_ttx(506)); /* "OH VERDAMMT, %s HAT %s VERLOREN!" */
			lose_random_item(hero, 15, get_ttx(506)); /* "OH VERDAMMT, %s HAT %s VERLOREN!" */
		}
	}

	GUI_output(get_tx2(33)); /* "DER WEG HINTER EUCH IST - ZUMINDEST FÜR GERAUME ZEIT- VERSPERRT. UN..." */
}

/* a herb place */
/* Merske <-> Rovamund (Einbeere) */
void tevent_118(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_PFLANZENKUNDE, 3) > 0 && !gs_tevent118_flag) ||	gs_tevent118_flag)
	{
		g_gather_herbs_special = ITEM_ID_EINBEERE;

		TRV_found_herb_place(0);

		g_gather_herbs_special = -1;

		gs_tevent118_flag = 1;
	}
}

/* a camp place */
/* Merske <-> Rovamund */
void tevent_119(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 2) > 0 && !gs_tevent119_flag) || gs_tevent119_flag)
	{
		gs_tevent119_flag = 1;

		TRV_found_camp_place(0);
	}
}

/* Orvil - Rovik: wolves threaten a sheperd */
void tevent_122(void)
{
	signed int answer;

	if (!gs_tevent122_flag)
	{
		do {
			answer = GUI_radio(get_tx2(34), 2, get_tx2(35),	get_tx2(36)); /* "SCHON AUS GERAUMER ENTFERNUNG..." / "IN DEN KAMPF EINGREIFEN?" / "EUCH STILL UND HEIMLICH VERKR..." */

		} while (answer == -1);

		if (answer == 1)
		{
			/* attack wolves */
			if (!TRV_fight_event(FIGHT_ID_F122, 122))
			{
				gs_tevent122_flag = 1;

				add_hero_ap_all(10);

				load_in_head(4);

				GUI_dialogbox((unsigned char*)g_dtp2, get_tx(68), get_tx2(37), 0); /* "SCHÄFER" / "ERLEICHTERT STEIGT DER SCHÄFER VON SEINEM..." */
				GUI_dialogbox((unsigned char*)g_dtp2, get_tx(68), get_tx2(63), 0); /* "SCHÄFER" / "<ABER ES MAG EUCH INTERESSIEREN, DASS DIES..." */
			}
		} else {
			/* decline to help */
			sub_hero_ap_all(20);
		}
	}
}

/* a bridge */
/* Orvil <-> Rovik: Haengebruecke. test_attrib(..., ATTRIB_HA, ...) > 0 heisst: Hoehenangst schlaegt zu */
void tevent_123(void)
{
	signed int i;
	signed int counter;
	signed int answer;
	signed int done;
	signed int attrib_result;
	signed int talent_result;
	struct struct_hero *hero;

	load_in_head(54);

	do {
		done = 0;

		do {
			answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(38), 3, get_tx2(39), get_tx2(40), get_tx2(41)); /* "DER WEG MACHT HIER EINE BIEGU..." / "DIE BRÜCKE ÜBERQUEREN?" / "UMKEHREN UND EINEN ANDEREN WE..." / "EURE HÖHENANGST BEKÄMPFEN?" */

		} while (answer == -1);

		if (answer == 1)
		{
			/* go over the bridge */
			hero = get_hero(0);
			for (i = counter = 0; i <= 6; i++, hero++)
			{
				if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
					!hero->flags.dead && test_attrib(hero, ATTRIB_HA, 0) > 0)
				{
					/* attrib test failed */
					/* (genauer: HA-Probe gelungen = der Held hat Hoehenangst) */
					counter++;

					sprintf(g_dtp2 + 0x400,	get_tx2(42), hero->alias); /* "%s WEIGERT SICH, DIE BRÜCKE ZU BETRETEN." */

					GUI_dialog_na(0, (char*)(g_dtp2 + 0x400));
				}
			}

			if (!counter)
			{
				GUI_dialog_na(0, get_tx2(43)); /* "IHR ERREICHT SCHWEISSGEBADET ABER GLÜCKLICH DAS ANDERE UFER." */
				done = 1;
			}

		} else if (answer == 2)
		{
			/* turn arround */

#if !FIX_0150
			/* Original-Bug 53:
			 * If the party was already traveling backward, the new direction should be forward. */
			gs_journey_direction = done = JOURNEY_DIRECTION_CHANGE_TO_BACKWARD; // this is value 1
#else
			/* Fix 150: Umkehren kehrt die aktuelle Richtung um */
			gs_journey_direction = (gs_journey_direction == JOURNEY_DIRECTION_FORWARD ? JOURNEY_DIRECTION_CHANGE_TO_BACKWARD : JOURNEY_DIRECTION_CHANGE_TO_FORWARD);
			done = 1;
#endif

		} else {
			/* fight acrophobia */
			do {
				answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(44), 3, /* "@WOLLT IHR" */
							get_tx2(45), get_tx2(46), get_tx2(47)); /* "EUCH BETRINKEN, UM DIE HÖHENA..." / "MAGISCH EUREN MUT ERHÖHEN?" / "EUCH ANDERWEITIG MUT ZUSPRECHEN?" */

			} while (answer == -1);

			if (answer == 1)
			{
				hero = get_hero(0);
				for (i = counter = 0; i <= 6; i++, hero++)
				{
					if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !hero->flags.dead)
					{
						attrib_result = test_attrib(hero, ATTRIB_HA, 4);
						talent_result = test_talent(hero, TA_KLETTERN, 0);

						/* Fehler im Original: test_attrib() liefert nie 99 (nur -99 bei W20 = 20); dieser
						 * Zweig und Text 48 (Held verschwindet) sind unerreichbar, ebenso unten. */
						if (attrib_result == 99 && talent_result == -1)
						{
							sprintf(g_dtp2 + 0x400, get_tx2(48), hero->alias); /* "%s GREIFT PLÖTZLICH IN DIE LEERE LUFT UND VERSCHWINDET MIT EINEM@GR..." */

							GUI_dialog_na(0, (char*)g_dtp2 + 0x400);

							hero_disappear(hero, i, -1);

							counter++;

						} else if (attrib_result > 0 || talent_result <= 0)
						{
							sprintf(g_dtp2 + 0x400,	get_tx2(42), hero->alias); /* "%s WEIGERT SICH, DIE BRÜCKE ZU BETRETEN." */

							GUI_dialog_na(0, (char*)g_dtp2 + 0x400);

							counter++;
						}
					}
				}

				if (!counter)
				{
					GUI_dialog_na(0, get_tx2(43)); /* "IHR ERREICHT SCHWEISSGEBADET ABER GLÜCKLICH DAS ANDERE UFER." */
					done = 1;
				}

			} else if (answer == 2)
			{
				/* cast a spell */
				hero = get_hero(select_hero_ok_forced(get_ttx(317))); /* "WELCHER HELD SOLL MAGIE ANWENDEN?" */

				if (test_spell(hero, SP_MU_STEIGERN, 0) > 0)
				{
					/* success */
					sub_ae_splash(hero, get_spell_cost(SP_MU_STEIGERN, 0));

					GUI_dialog_na(0, get_tx2(43)); /* "IHR ERREICHT SCHWEISSGEBADET ABER GLÜCKLICH DAS ANDERE UFER." */

					done = 1;
				} else {
					/* failed */
					sub_ae_splash(hero, get_spell_cost(SP_MU_STEIGERN, 1));

					hero = get_hero(0);
					for (i = counter = 0; i <= 6; i++, hero++)
					{
						if ((hero->typus != HERO_TYPE_NONE) &&
							(hero->group_id == gs_active_group_id) &&
							!hero->flags.dead)
						{
							sprintf(g_dtp2 + 0x400, get_tx2(42), hero->alias); /* "%s WEIGERT SICH, DIE BRÜCKE ZU BETRETEN." */

							GUI_dialog_na(0, (char*)g_dtp2 + 0x400);
						}
					}
				}
			} else {
				hero = get_hero(0);
				for (i = counter = 0; i <= 6; i++, hero++)
				{
					if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !hero->flags.dead)
					{
						attrib_result = test_attrib(hero, ATTRIB_HA, 2);

						if (attrib_result == 99)
						{
							/* unlucky */
							/* (unerreichbar, siehe oben) */
							sprintf(g_dtp2 + 0x400,	get_tx2(48), hero->alias); /* "%s GREIFT PLÖTZLICH IN DIE LEERE LUFT UND VERSCHWINDET MIT EINEM@GR..." */

							GUI_dialog_na(0, (char*)g_dtp2 + 0x400);

							hero_disappear(hero, i, -1);

							counter++;

						} else if (attrib_result > 0)
						{
							/* succeeded */
							sprintf(g_dtp2 + 0x400,	get_tx2(42), hero->alias); /* "%s WEIGERT SICH, DIE BRÜCKE ZU BETRETEN." */

							GUI_dialog_na(0, (char*)g_dtp2 + 0x400);

							counter++;
						}
					}
				}

				if (!counter)
				{
					GUI_dialog_na(0, get_tx2(43)); /* "IHR ERREICHT SCHWEISSGEBADET ABER GLÜCKLICH DAS ANDERE UFER." */
					done = 1;
				}
			}
		}


	} while (done == 0);
}
