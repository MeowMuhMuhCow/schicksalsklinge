/*
 * seg111 - Reiseereignisse 3/10 (Ereignisse 53-66)
 *
 * Inhalt:    Ereignisse zwischen Ottarje, Skjal, Prem, Kord, Orvil und Ala: Herbergen "Langschiff"
 *            und "Adlerfels", schlammiger Bergpfad, Erdrutsch, Jaegerhuette, Pass mit Klamm (Seile,
 *            Seil-Stabzauber), Zugang zur Wolfshoehle, toter Kadaver, Gorahs Lichtung (Druide mit
 *            Woelfen, Auftrag von Umbrik Siebenstein), Reiter mit Hinweis auf Umbrik und wilde Hunde.
 * Aufrufer:  TRV_event() (seg109) ueber g_travel_event_handlers.
 * Daten:     g_tevent064_chest_descr (Truhe in Gorahs Huette, Schloss und Giftfalle).
 * Besonders: Original-Bug (tevent_057 Kletterbedingung, tevent_065 Informantenflag). Neu in tevent_064:
 *            der Text "LEIDER MISSLINGT DER SILENTIUM" (102) ist unerreichbar und ein Nicht-Magier faellt
 *            nicht in die Auswahl zurueck; das Anschleichen scheitert erst bei zwei misslungenen Proben.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg111.c
 */

#include <stdio.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg007.h"
#include "seg025.h"
#include "seg026.h"
#include "seg032.h"
#include "seg047.h"
#include "seg092.h"
#include "seg096.h"
#include "seg097.h"
#include "seg098.h"
#include "seg103.h"
#include "seg105.h"
#include "seg109.h"
#include "seg111.h"

struct struct_chest g_tevent064_chest_descr = {0, 12, use_lockpicks_on_chest, chest_poisoned3, tevent_064_chest, 0, 0, 0}; // ds:0xb154

/* Ottarje <-> Skjal: alte Feuerstelle. idealer Rastplatz */
/* (Beschreibung zufaellig aus FEATURE.LTX 11..15, siehe TRV_found_camp_place()) */
void tevent_053(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 1) > 0 && !gs_tevent053_flag) ||
		gs_tevent053_flag != 0)
	{
		TRV_found_camp_place(0);
		gs_tevent053_flag = 1;
	}
}

/* Prem <-> Skjal: inn "Langschiff" */
void tevent_054(void)
{
	TRV_found_inn(TOWN_ID_BRENDHIL, 68); /* 47 = FEATURE.LTX "LANGSCHIFF" */
}

/* Path ??? */
/* Prem <-> Kord: im Fruehling und Herbst schlammiger Bergpfad, 3 Stunden Verzoegerung */
void tevent_055(void)
{
	signed int season;

	season = get_current_season();

	if (season == SEASON_SPRING || season == SEASON_AUTUMN) {

		load_in_head(52);

		GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(0), 0); /* "DER BERGPFAD, AUF DEM IHR EUCH GERADE BEFINDET, WIRD ZUSEHENDS SCHL..." */

		timewarp(HOURS(3));
	}
}

/* Prem <-> Kord: Herberge "Adlerfels" */
void tevent_056(void)
{
	TRV_found_inn(TOWN_ID_MANRIN, 69); /* 48 = FEATURE.LTX "ADLERFELS" */
}

/* Ottarje <-> Orvil: Erdrutsch (ueberklettern oder umgehen) */
void tevent_057(void)
{
	signed int i;
	signed int answer;
	struct struct_hero *hero;

	do {
		answer = GUI_radio(get_tx2(1), 2, get_tx2(2), get_tx2(3)); /* "HIER MUSS WOHL LETZTENS EIN B..." / "DEN SCHUTTHÜGEL ÜBERKLETTERN?" / "VERSUCHEN, DIE BARRIERE ZU UM..." */

	} while (answer == -1);

	if (answer == 2) {

		if (test_talent(get_first_hero_available_in_group(), TA_ORIENTIERUNG, 2) > 0) {

			timewarp(HOURS(2));

			GUI_output(get_tx2(4)); /* "NACH GERAUMER ZEIT FINDET IHR EIN KLEINES SEITENTAL, IN DEM ES SICH..." */

		} else {

			timewarp(HOURS(6));

			GUI_output(get_tx2(5)); /* "ES IST WIE VERHEXT! IHR FINDET EINFACH KEINEN VERNÜNFTIG GANGBAREN..." */

			hero = get_hero(0);

			for (i = 0; i <= 6; i++, hero++) {

				if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id))
				{
					sub_hero_le(hero, random_schick(3));
				}
			}
		}
	} else {

		hero = get_hero(0);

		for (i = 0; i <= 6; i++, hero++) {

			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
				!hero->flags.dead && test_talent(hero, TA_KLETTERN, 3) <= 0)
			{

				/* TODO: Original-Bug: that condition does not make sense */
				/* Vermutlich gemeint: kein Seil UND keine Strickleiter -> schwerer Sturz. So stuerzt
				 * schwer, wer eine Strickleiter hat oder kein Seil; leicht nur mit Seil ohne Strickleiter. */
#if FIX_0158
				/* Fix 158: schwerer Sturz nur ohne Seil und ohne Strickleiter */
				if (get_first_hero_with_item(ITEM_ID_SEIL) == -1 && get_first_hero_with_item(ITEM_ID_STRICKLEITER) == -1)
#else
				if (get_first_hero_with_item(ITEM_ID_SEIL) == -1 || get_first_hero_with_item(ITEM_ID_STRICKLEITER) != -1)
#endif
				{

					sprintf(g_dtp2, get_tx2(8), hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST)); /* "UNTER %s LÖST SICH EIN FELSBLOCK, UND IN EINEM HAUFEN GERÖLL GEHT %..." */
					GUI_output(g_dtp2);

					sub_hero_le(hero, random_schick(9) + 3);

				} else {
					sprintf(g_dtp2, get_tx2(7), hero->alias); /* "%s GERÄT INS RUTSCHEN, FÄNGT SICH ABER SCHNELL WIEDER." */
					GUI_output(g_dtp2);

					sub_hero_le(hero, random_schick(8));
				}
			}
		}

		GUI_output(get_tx2(6)); /* "SCHWITZEND UND SCHNAUFEND ERREICHT IHR DIE ANDERE SEITE DES ERDRUTS..." */
	}
}

/* Ottarje <-> Orvil: Nahrungsplatz */
void tevent_058(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 2) > 0 && !gs_tevent058_flag) || gs_tevent058_flag)
	{
		TRV_found_replenish_place(0);
		gs_tevent058_flag = 1;
	}

}

/* Ottarje <-> Orvil: Berghuette mit grummelndem Jaeger */
void tevent_059(void)
{
	signed int answer;

	if (TRV_enter_hut_question())
	{
		gs_camp_incident = 0; /* Zwischenfall in der 1. Wache: der Jaeger kommt */
		gs_town_loc_type = LOCTYPE_WILDCAMP;
		do_location();
		gs_town_loc_type = LOCTYPE_NONE;
		gs_camp_incident = -1;

		TRV_load_textfile(-1);

		load_in_head(10);

		do {
			answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(9), 2, get_tx2(10), get_tx2(11)); /* "NACHDEM IHR BEREITS ZWEI STUN..." / "DEN JÄGER ANSPRECHEN?" / "DEN GRUMMLER SCHLAFEN LASSEN?" */

		} while (answer == -1);

		if (answer == 1) {

			if (get_first_hero_with_item(ITEM_ID_SCHNAPSFLASCHE) != -1 || get_first_hero_with_item(ITEM_ID_WEINFLASCHE) != -1)
			{
				do {
					answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(12), /* "DER JÄGER SCHEINT EUCH ERST JETZT RICHTIG ZU BEMERKEN.@<TAG AUCH. B..." */
								3, get_tx2(13), get_tx2(14), get_tx2(15)); /* "<ES IST NICHT RECHT, SICH DEM..." / "<NEIN, LEIDER NUR 97ER YAQUIR..." / "<KLAR, MANN.>" */
				} while (answer == -1);

				if (answer == 1) {
					GUI_dialog_na(0, get_tx2(16)); /* "DER JÄGER GRUMMELT ETWAS WIE <BLÖDE MORALPREDIGER> UND DREHT SICH Z..." */
				} else if (answer == 2) {
					GUI_dialog_na(0, get_tx2(17)); /* "<NA, DANN HER DAMIT. AH, DAS SCHMECKT! KOMMTNIHRHER? NICHDIEGEGENDF..." */
				} else if (test_attrib(get_first_hero_available_in_group(), ATTRIB_CH, 0) > 0) {
					GUI_dialog_na(0, get_tx2(17)); /* "<NA, DANN HER DAMIT. AH, DAS SCHMECKT! KOMMTNIHRHER? NICHDIEGEGENDF..." */
				} else {
					GUI_dialog_na(0, get_tx2(16)); /* "DER JÄGER GRUMMELT ETWAS WIE <BLÖDE MORALPREDIGER> UND DREHT SICH Z..." */
				}

			} else {
				GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(12), 1, get_tx2(18)); /* "DER JÄGER SCHEINT EUCH ERST JETZT RICHTIG..." / "<NEIN, LEIDER NICHT. ABER GUTE HARTWURST.>" */
				GUI_dialog_na(0, get_tx2(17)); /* "<NA, DANN HER DAMIT. AH, DAS SCHMECKT! KOMMTNIHRHER? NICHDIEGEGENDF..." */
			}
		}

		gs_town_loc_type = LOCTYPE_WILDCAMP;
		do_location();
		gs_town_loc_type = LOCTYPE_NONE;
	}
}

/* Ottarje <-> Orvil: Pass mit Abstieg in eine Klamm (rechter/linker Pfad, Abseilen) */
void tevent_060(void)
{
	signed int answer;
	signed int i;
	signed int done;
	signed int nr_items;
	signed int has_magic_rope;
	struct struct_hero *hero;

	done = 0;
	load_in_head(57);

	do {
		do {
			answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(19), 2, get_tx2(20), get_tx2(21)); /* "MÜHSELIG QUÄLT IHR EUCH SCHRI..." / "DEN RECHTEN PFAD NEHMEN?" / "ODER ES LIEBER LINKS VERSUCHEN?" */

		} while (answer == -1);

		if (answer == 1) {

			if (test_talent(get_first_hero_available_in_group(), TA_ORIENTIERUNG, 4) > 0) {

				sub_group_le(1);

				timewarp(HOURS(3));

				GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(22), 0); /* "DER ABSTIEG IST SCHWIERIGER ALS ERWARTET, ABER SCHLIESSLICH@BEFINDE..." */

				done = 1;
			} else {

				sub_group_le(1);

				timewarp(HOURS(4));

				do {
					answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(24), 2, get_tx2(26), get_tx2(27)); /* "DER PFAD FÜHRT EUCH IMMER WEI..." / "DIE KLIPPE HINABKLETTERN?" / "ZUR PASSHÖHE UMKEHREN?" */

				} while (answer == -1);

				if (answer == 1) {

					timewarp(HOURS(1));

					hero = get_hero(0);

					for (i = 0; i <= 6; i++, hero++) {

						if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
							!hero->flags.dead && test_talent(hero, TA_KLETTERN, 0) <= 0)
						{
							sub_hero_le(hero, random_schick(10));
						}
					}

					GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(29), 0); /* "ES GELINGT EUCH, IN DIE KLAMM HINABZUSTEIGEN.@EINIGE VON EUCH TRAGE..." */

					done = 1;
				} else {
					timewarp(HOURS(2));
				}
			}

		} else {


			if (test_talent(get_first_hero_available_in_group(), TA_ORIENTIERUNG, 2) > 0) {

				timewarp(HOURS(3));

				GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(23), 0); /* "DER WEG SCHEINT AM ANFANG SCHWIERIG ZU SEIN, DOCH NACH WENIGER ALS..." */

				done = 1;
			} else {

				sub_group_le(1);

				timewarp(HOURS(4));

				do {
					answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(25), 2, get_tx2(27), get_tx2(28)); /* "IHR KOMMT GLÄNZEND VORAN, ABE..." / "ZUR PASSHÖHE UMKEHREN?" / "EUCH ABSEILEN?" */
				} while (answer == -1);

				if (answer == 2) {

					hero = get_hero(0);

					for (i = has_magic_rope = nr_items = 0; i <= 6 ; i++, hero++){

						if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
							!hero->flags.dead)
						{
							nr_items += hero_count_item(hero, ITEM_ID_SEIL);
							nr_items += hero_count_item(hero, ITEM_ID_STRICKLEITER);

							if (hero->staff_level >= 3) /* 3. Stabzauber: Seil des Stabs */
							{
								has_magic_rope = 1;
							}
						}
					}

					if (nr_items >= 3) {

						sub_group_le(3);

						for (i = 0; i < 3; i++) {

							answer = inv_slot_of_item((hero = get_hero(get_first_hero_with_item(ITEM_ID_SEIL))), ITEM_ID_SEIL);
							if (answer == -1) {
								answer = inv_slot_of_item((hero = get_hero(get_first_hero_with_item(ITEM_ID_STRICKLEITER))), ITEM_ID_STRICKLEITER);
							}

							drop_item(hero, answer, 1);
						}

						timewarp(HOURS(1));

						GUI_dialog_na(0, get_tx2(32)); /* "DER ABSTIEG GELINGT EUCH OHNE GRÖSSERE MÜHE. DIE GANZE AKTION HAT E..." */

						done = 1;

					} else if (nr_items == 2 && has_magic_rope != 0) {

						sub_group_le(3);

						timewarp(HOURS(1));

						GUI_dialog_na(0, get_tx2(31)); /* "HESINDE SEI DANK FÜR DEN DRITTEN STABZAUBER! IHR ERREICHT DEN WEG O..." */

						done = 1;
					} else {

						GUI_dialog_na(0, get_tx2(30)); /* "IHR HABT NICHT GENÜGEND SEIL ZUR VERFÜGUNG." */

						timewarp(HOURS(2));

						GUI_dialog_na(0, get_tx2(33)); /* "IHR MÜSST ZUR PASSHÖHE UMKEHREN." */
					}
				} else {
					timewarp(HOURS(2));
				}
			}
		}

	} while (done == 0);
}

/* Ottarje <-> Orvil: entrance to the dungeon "wolf cave" */
void tevent_061(void)
{
	signed int answer;

	/* TODO: not needed here */
	answer = 0;

	if (test_talent(get_first_hero_available_in_group(), TA_SINNESSCHAERFE, 6) > 0 &&
		test_talent(get_first_hero_available_in_group(), TA_FAEHRTENSUCHEN, 3) > 0 &&
		!gs_tevent061_flag)
	{
		gs_tevent061_flag = 1;

		/* TODO: is this correct ? */
		/* ja: TRV_follow_trail_question() liefert 0 fuer "DIE FAEHRTE GENAUER UNTERSUCHEN" */
		if (!TRV_follow_trail_question()) {

			GUI_output(get_tx2(34)); /* "DEN ABDRÜCKEN NACH ZU URTEILEN, MUSS ES SICH UM WÖLFE HANDELN, ABER..." */
			GUI_output(get_tx2(35)); /* "DIE SPUREN WERDEN DEUTLICHER UND FÜHREN JETZT IN EIN SEITENTAL..." */

			load_in_head(53);

			do {
				answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(36), /* "AM ENDE EINES TALES KÖNNT IHR DEN EINGANG EINER HÖHLE ERKENNEN. DOR..." */
							2, get_tx2(37), get_tx2(38)); /* "IN DIE HÖHLE EINDRINGEN?" / "UMKEHREN UND DEN WEG FORTSETZEN?" */

			} while (answer == -1);

			if (answer == 1) {
				gs_travel_detour = DUNGEON_ID_WOLFSHOEHLE;
			}
		}

	} else if (gs_tevent061_flag != 0) {

		load_in_head(53);

		do {
			answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(39), /* "HIER FÜHRT EIN WEG ZU EINEM WOLFSBAU. @WOLLT IHR" */
						2, get_tx2(40),	get_tx2(41)); /* "DIE HÖHLE DER WÖLFE BESUCHEN?" / "DEN WEG FORTSETZEN" */

		} while (answer == -1);

		if (answer == 1) {
			gs_travel_detour = DUNGEON_ID_WOLFSHOEHLE;
		}
	}
}

/* Orvil <-> Skjal: dummy */
/* (steht auch auf Ottarje <-> Orvil; im Original leer) */
void tevent_062(void)
{
#if !defined(__BORLANDC__)
	D1_INFO("Reiseevent %d ist nicht implementiert\n", 62);
#endif
}

/* Orvil <-> Skjal: dead animal (auch Ottarje <-> Orvil) */
void tevent_063(void)
{
	signed int i; /* multiple use: hero_pos, mod_slot */
	signed int max; /* kleinstes bisheriges MU-Ergebnis (Startwert 9999) */
	signed int proof;
	signed int vomiter;
	struct struct_hero *hero;

	max = 9999;

	/* intro message */
	GUI_output(get_tx2(46)); /* "SCHON AUF EINIGE ENTFERNUNG KÖNNT IHR DEN SÜSSLICHEN GERUCH VON VER..." */

	hero = get_hero(0);

	for (i = 0; i <= 6; i++, hero++) {

		if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !hero->flags.dead)
		{
			/* MU+0 */
			if ((proof = test_attrib(hero, ATTRIB_MU, 0)) < max) {
				max = proof;
				vomiter = i;
			}
		}
	}

	hero = get_hero(vomiter);

	sprintf(g_dtp2, get_tx2(47), hero->alias); /* "%s MUSS SICH ÜBERGEBEN!" */

	/* print who vomits */
	GUI_output(g_dtp2);

	/* LE - 2 */
	sub_hero_le(hero, 2);

	i = get_free_mod_slot();

	/* MU -2 for 24 hours */
	set_mod_slot(i, DAYS(1), (uint8_t*)&hero->attrib[ATTRIB_MU].current, -2, (signed char)vomiter);

	/* outro message */
	GUI_output(get_tx2(48)); /* "AUCH EINE GENAUERE UNTERSUCHUNG DES KADAVERS BRINGT KEINE WEITEREN@..." */
}

/* Gorahs place */
/* Ottarje <-> Orvil (Meile 58) und Skjal <-> Orvil (Meile 61): nur mit Umbriks Auftrag */
void tevent_064(void)
{
	signed int answer;
	signed int result;
	signed int i;
	signed int approach_flag;
	signed int silentium_flag; /* wird nie gesetzt (siehe unten) */
	struct struct_hero *hero;

	approach_flag = 0;
	silentium_flag = 0;

	if (!gs_tevent064_flag && gs_umbrik_quest_gorah) {

		do {

			do {
				answer = GUI_radio(silentium_flag != 0 ? get_tx2(102) : get_tx2(49), 3, /* "LEIDER MISSLINGT DER SILENTIUM.@WAS WOLLT..." / "HÄTTE EUCH UMBRIK SIEBENSTEIN NICHT DAS GE..." */
							get_tx2(50), get_tx2(51), get_tx2(52)); /* "IM SCHUTZE EINES <SILENTIUM>..." / "EUCH OHNE HILFSMITTEL ANSCHLE..." / "DEN FINSTEREN DRUIDEN FORDERN?" */

			} while (answer == -1);

			if (answer == 1) {

				hero = get_hero(select_hero_ok_forced(get_ttx(317))); /* "WELCHER HELD SOLL MAGIE ANWENDEN?" */

#if FIX_0159
				/* Fix 159: Nicht-Magier oder misslungener Silentium fuehren zurueck in die Auswahl
				 * (bei Misserfolg mit Text 102 "LEIDER MISSLINGT DER SILENTIUM") */
				if (hero->typus < HERO_TYPE_HEXE) {
					GUI_output(get_ttx(330));
				} else if (test_spell(hero, SP_SILENTIUM_SILENTILLE, 0) > 0) {

					approach_flag = 1;

					sub_ae_splash(hero, 2);

					gs_tevent064_ambush_flag = 1;

					GUI_output(get_tx2(53));
				} else {
					sub_ae_splash(hero, 1);

					silentium_flag = 1;
				}
#else
				if (hero->typus < HERO_TYPE_HEXE) {
					GUI_output(get_ttx(330)); /* "DIESE OPTION IST AUSSCHLIESSLICH MAGIEKUNDIGEN VORBEHALTEN!" */
				} else {
					approach_flag = 1;
				}

				/* Fehler im Original: hier wird approach_flag immer 1, die Auswahl wird nie wiederholt.
				 * Ein Nicht-Magier "schleicht" danach ohne Wirkung weiter, ein misslungener Silentium
				 * setzt silentium_flag nicht; der Text 102 "LEIDER MISSLINGT DER SILENTIUM" erscheint nie. */
				if (approach_flag == 0) {
					approach_flag = 1;
				} else {
					if (test_spell(hero, SP_SILENTIUM_SILENTILLE, 0) > 0) {

						sub_ae_splash(hero, 2);

						gs_tevent064_ambush_flag = 1; /* mit Silentium hoert ihr selbst nichts: Hinterhalt */

						GUI_output(get_tx2(53)); /* "DIESER ZAUBER HAT SEINE VOR- UND NACHTEILE. EINER DER NACHTEILE IST..." */
					} else {
						sub_ae_splash(hero, 1);

						GUI_output(get_ttx(606)); /* "ZAUBER LEIDER MISSLUNGEN!" */
					}
				}
#endif

			} else if (answer == 2) {

				approach_flag = 1;

				GUI_output(get_tx2(54)); /* "IHR BEMÜHT EUCH, IM DICHTEN WALD KEIN GERÄUSCH ZU MACHEN..." */

				hero = get_hero(0);

				for (i = result = 0; i <= 6; i++, hero++) {

					if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
						!hero->flags.dead && test_talent(hero, TA_SCHLEICHEN, 0) <= 0)
					{
						result++;
					}
				}

				if (result > 1) {
					/* test failed for at least one hero */
					/* Korrektur: erst ab ZWEI misslungenen Proben (result > 1); ein Held darf patzen */
					gs_tevent064_ambush_flag = 1;
				}

			} else {

				approach_flag = 1;

				do {
					answer = GUI_radio(get_tx2(55), 2, get_tx2(56), get_tx2(57)); /* "HÖHNISCHES GELÄCHTER SCHALLT..." / "DEM DRUIDEN GEGENÜBERTRETEN?" / "ODER LIEBER UMKEHREN?" */

				} while (answer == -1);

				if (answer == 2)  return;
			}

		} while (approach_flag == 0);

		do {
			answer = GUI_radio(get_tx2(58), 2, get_tx2(59), get_tx2(60)); /* "IHR ERREICHT DEN RAND EINER W..." / "DEN DRUIDEN MIT GEZIELTEN SCH..." / "AUF DIE LICHTUNG TRETEN UND K..." */

		} while (answer == -1);

		/* gs_tevent064_ambush_flag = 1 bedeutet: die Woelfe umgehen euch (Text 62), d. h. ihr wurdet
		 * bemerkt oder habt wegen des Silentium nichts gehoert */
		if (answer == 1) {

			if (!gs_tevent064_ambush_flag) {
				GUI_output(get_tx2(61)); /* "SELTSAMERWEISE VERFEHLEN ALLE EURE SCHÜSSE DAS ZIEL." */
			}

			GUI_output(get_tx2(62)); /* "IHR SPÜRT INSTINKTIV, DASS SICH HINTER EUCH ETWAS BEWEGT. WENN IHR..." */

			result = TRV_fight_event(FIGHT_ID_F064, 64);

			gs_tevent064_ambush_flag = 1;

		} else {
			GUI_output(gs_tevent064_ambush_flag != 0 ? get_tx2(62) : get_tx2(63)); /* "IHR SPÜRT INSTINKTIV, DASS SICH HINTER EUC..." / "IHR MACHT EUCH ZUM KAMPF BEREIT UND BETRET..." */
			result = TRV_fight_event(FIGHT_ID_F064, 64);
		}

		if (!result) {

			gs_tevent064_flag = 1;

			add_hero_ap_all(100);

			do {
				do {
					answer = GUI_radio(get_tx2(64), 2, get_tx2(65), get_tx2(66)); /* "DER KAMPF IST VORÜBER - UND I..." / "DIE HÜTTE DURCHSUCHEN" / "UMKEHREN UND UMBRIK SIEBENSTE..." */

				} while (answer == -1);

				if (answer == 1) {

					loot_multi_chest(gs_tevent064_hut_content, get_tx2(71)); /* "DIE HÜTTE BESTEHT AUS EINEM EINZIGEN RAUM, DER INTENSIV NACH KRÄUTE..." */

					do {
						answer = GUI_radio(get_tx2(67), 2, get_tx2(68),	get_tx2(69)); /* "DIE HÜTTE BESTEHT AUS EINEM E..." / "DIE TRUHE ÖFFNEN?" / "LIEBER DIE FINGER DAVON LASSEN?" */

					} while (answer == -1);

					if (answer == 1) {
						use_lockpicks_on_chest(&g_tevent064_chest_descr);
					}
				}

			} while (answer != 2);

			GUI_output(get_tx2(70)); /* "IHR FINDET NICHTS WEITER VON INTERESSE UND BESCHLIESST EUREN WEG FO..." */

			/* zurueck nach Orvil zu Umbrik (Text 66) */
#if !FIX_0369
			gs_journey_direction = (gs_journey_direction == JOURNEY_DIRECTION_FORWARD ? JOURNEY_DIRECTION_CHANGE_TO_BACKWARD : JOURNEY_DIRECTION_CHANGE_TO_FORWARD);
#else
			/* Fix 369: nur umkehren, wenn die Gruppe nicht schon Richtung Orvil reist (Richtung Orvil = Ziel Orvil vorwaerts
			 * oder anderes Ziel rueckwaerts); die Umkehr laeuft ueber CHANGE_TO_..., auch nach einer Umkehr unterwegs */
			if ((gs_journey_destination_town_id == TOWN_ID_ORVIL) != (gs_journey_direction == JOURNEY_DIRECTION_FORWARD)) {
				gs_journey_direction = (gs_journey_direction == JOURNEY_DIRECTION_FORWARD ? JOURNEY_DIRECTION_CHANGE_TO_BACKWARD : JOURNEY_DIRECTION_CHANGE_TO_FORWARD);
			}
#endif
		}

	} else if (gs_tevent064_flag != 0) {

		do {
			do {
				answer = GUI_radio(get_tx2(64), 2, get_tx2(65),	get_ttx(814)); /* "DER KAMPF IST VORÜBER - UND I..." / "DIE HÜTTE DURCHSUCHEN" / "WEITERREISEN" */

			} while (answer == -1);

			if (answer == 1) {

				loot_multi_chest(gs_tevent064_hut_content, get_tx2(71)); /* "DIE HÜTTE BESTEHT AUS EINEM EINZIGEN RAUM, DER INTENSIV NACH KRÄUTE..." */

				if ((signed char)gs_tevent064_chest[0] != -1) {

					do {
						answer = GUI_radio(get_tx2(67), 2, get_tx2(68),	get_ttx(814)); /* "DIE HÜTTE BESTEHT AUS EINEM E..." / "DIE TRUHE ÖFFNEN?" / "WEITERREISEN" */

					} while (answer == -1);

					if (answer == 1) {
						use_lockpicks_on_chest(&g_tevent064_chest_descr);
					}
				}
			}

		} while (answer != 2);
	}
}

void tevent_064_chest(struct struct_chest* chest)
{
	loot_multi_chest(gs_tevent064_chest, get_tx2(74)); /* "IN DER TRUHE FINDET IHR FOLGENDES:" */
}

/* Orvil <-> Ala: rider */
void tevent_065(void)
{
	signed int answer;

	/* load dialog head */
	load_in_head(56);

	/* show dialogbox */
	do {
		answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(75), /* "EIN REITER AUF DEM WEG NACH ORVIL - FÜRWAHR EINE SELTENE ERSCHEINUN..." */
				3, get_tx2(76),	get_tx2(77), get_tx2(78)); /* "DEN REITERSMANN NACH DEM WEG..." / "EUCH ÜBER EVENTUELLE GEFAHREN..." / "DEN BERITTENEN GRÜSSEN UND DA..." */

	} while (answer == -1);

	/* talk to the rider for 15 min */
	if (answer == 1 || answer == 2) {

		timewarp(MINUTES(15));

		GUI_dialog_na(0, answer == 1 ? get_tx2(79): get_tx2(80)); /* "<VON ORVIL NACH ALA. EIN STRAMMER TAGESMAR..." / "<IN DER NÄHE VON ORVIL SOLL ES EINEN DRUID..." */

		if (answer == 2) {
			/* Make Umbrik Siebenstein known */

			/* Original-Bug: if Umbrik is done the value 2 is overwritten with 1 */
			update_informer_cond(INFORMER_ID_UMBRIK);
		}
	}
}

/* Orvil <-> Ala: Nahrungsplatz, danach evtl. Spur wilder Hunde (Kampf F066) */
void tevent_066(void)
{
	signed int answer;
	signed int i;
	signed int count;
	struct struct_hero *hero;

	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 2) > 0 && !gs_tevent066_flag) || gs_tevent066_flag)
	{
		TRV_found_replenish_place(0);
		gs_tevent066_flag = 1;

		if ((test_talent(get_first_hero_available_in_group(), TA_FAEHRTENSUCHEN, 4) > 0 && !gs_tevent066_track_flag))
		{

			do {
				answer = GUI_radio(get_tx2(81), 2, get_tx2(82), get_tx2(83)); /* "ALS IHR GERADE WEITERGEHEN WO..." / "DIE SPUR AUFNEHMEN?" / "EURE SACHEN PACKEN UND WEITER..." */

			} while (answer == -1);

			if (answer == 1) {

				GUI_output(get_tx2(84)); /* "DIE SPUR FÜHRT IN RICHTUNG KÜSTE. HIN UND WIEDER TRENNEN SICH EINZE..." */

				hero = get_hero(0);
				for (i = count = 0; i <= 6; i++, hero++)
				{
					if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
						!hero->flags.dead && test_talent(hero, TA_SCHLEICHEN, -2) <= 0)
					{
						count++;
					}
				}

				if (count != 0) {

					GUI_output(get_tx2(85)); /* "ALS IHR EUCH NÄHERT, SPRINGEN DIE TIERE AUS IHRER DECKUNG HERVOR UN..." */

				} else {
					do {
						answer = GUI_radio(get_tx2(86), 2, get_tx2(87), get_tx2(88)); /* "IHR KÖNNT EUCH DEN TIEREN UNB..." / "EUCH STILL UND HEIMLICH VERDR..." / "VERSUCHEN, EINIGE TIERE ZU ER..." */

					} while (answer == -1);

					if (answer == 1) {

						GUI_output(get_tx2(89)); /* "NACH EINIGEN BANGEN MINUTEN SEID IHR AUSSERHALB DER REICHWEITE DES..." */
						return;
					}
				}

				if (!do_fight(FIGHT_ID_F066)) {
					gs_tevent066_track_flag = 1;
					add_hero_ap_all(50);
				}
			}
		}
	}
}
