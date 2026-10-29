/*
 * seg118 - Reiseereignisse 10/10 (Ereignisse 37, 38, 51, 52, 78, 79, 120, 121, 124, 125, 145)
 *
 * Inhalt:    Der Greif am Weg Phexcaer-Einsiedlersee (Pfand, Raetsel "RAD"), erste Einhornbegegnung,
 *            Schneewaechte im Winter (Orkanger-Clanegh), Zugang zur Spinnenhoehle und umgestuerzte
 *            Eiche (Ottarje-Skjal), Steilklippe und Furt (Orvil-Rovik), Hirte (Ljasdahl) und
 *            tevent_145: Wahl der Richtung am Ende der Querverbindung.
 * Aufrufer:  TRV_event() (seg109); tevent_145 aus TM_do_journey_crosslink() (seg094).
 * Daten:     g_str_rad1..3 (Loesung des Greifenraetsels).
 * Besonders: Original-Bug 16 (Schneewaechte: alle Helden sterben), 53, Klettersturz (tevent_124).
 *            Neu: tevent_078 zeigt den Text 82 nie (Bedingung !answer ist dort immer falsch).
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg118.c
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
#include "seg028.h"
#include "seg029.h"
#include "seg030.h"
#include "seg075.h"
#include "seg096.h"
#include "seg097.h"
#include "seg103.h"
#include "seg109.h"
#include "seg113.h"

static const char g_str_rad1[4] = "RAD"; // ds:0xb21e
static const char g_str_rad2[4] = "RAD"; // ds:0xb222
static const char g_str_rad3[4] = "RAD"; // ds:0xb226


/* The griffon */
/* Phexcaer <-> Einsiedlersee; gs_tevent037_flag wird nur gesetzt, wenn ein Pfand gegeben wurde */
void tevent_037(void)
{
	signed int answer;
	signed int done = 0;
	signed int hours;
	struct struct_hero *hero;

	if (!gs_tevent037_flag)
	{
		g_event_ani_busy = 1;

		load_ani(28);
		draw_main_screen();
		init_ani(0);

		do {
			answer = GUI_radio(get_tx2(26), 2, get_tx2(27), get_tx2(28)); /* "MIT BEINAHE LAUTLOSEN FLÜGELS..." / "IHR MACHT HALT UND WARTET AB." / "IHR GEHT WEITER." */

		} while (answer == -1);

		if (answer == 1) {
			/* wait */

			timewarp(MINUTES(15));

			do {
				answer = GUI_radio(get_tx2(29), 2, get_tx2(30),	get_tx2(28)); /* "NICHTS PASSIERT. IHR WARTET B..." / "IHR WARTET." / "IHR GEHT WEITER." */

			} while (answer == -1);

			if (answer == 1)
			{

				timewarp(MINUTES(15));

				/* loop until you decide to go on */
				do {
					answer = GUI_radio(get_tx2(31), 2, get_tx2(30), get_tx2(28)); /* "EINE WEITERE VIERTELSTUNDE VE..." / "IHR WARTET." / "IHR GEHT WEITER." */

				} while (answer == -1 || answer == 1);
			}
		}

		if (answer == 2) {
			/* go on */

			do {
				answer = GUI_radio(get_tx2(32), 2, get_tx2(33), get_tx2(34)); /* "ALS IHR EUCH DEN WESEN NÄHERT..." / "IHR BEFOLGT DIE WORTE DES GRE..." / "IHR VERSUCHT ZU VERHANDELN." */

			} while (answer == -1);
		}

		do {

			if (answer == 1)
			{
				/* turn around */
				/* (genauer: den Worten des Greifen folgen und sein Revier umgehen, mindestens 4 Stunden) */
				hours = GUI_input(get_tx2(35), 2); /* "ES BLEIBT EUCH NICHTS ANDERES ÜBRIG, ALS EINEN GROSSEN BOGEN UM DIE..." */

				if (hours >= 4)
				{
					sprintf(g_dtp2, get_tx2(36), hours); /* "IHR MACHT EINEN RECHT GROSSEN BOGEN UM DAS TERRITORIUM DER GREIFEN...." */
					GUI_output(g_dtp2);

					done = 1;

				} else {

					/* entered time was too short */
					do {
						answer = GUI_radio(get_tx2(37), 2, get_tx2(33), get_tx2(34)); /* "IHR NEHMT DIE FLÜGELSCHLÄGE D..." / "IHR BEFOLGT DIE WORTE DES GRE..." / "IHR VERSUCHT ZU VERHANDELN." */

					} while (answer == -1);

					if (answer == 1)
					{
						timewarp(HOURS(4));

						GUI_output(get_tx2(38)); /* "IHR BEMÜHT EUCH, EINEN GENÜGEND GROSSEN BOGEN UM DAS TERRITORIUM DE..." */

						done = 1;
					}
				}
			} else {
				do {
					answer = GUI_radio(get_tx2(39), 3, get_tx2(40), get_tx2(41), get_tx2(42)); /* "DER GREIF SCHLÄGT EINMAL GELA..." / "<WIR KÖNNTEN EIN PFAND GEBEN!>" / "<WIR SIND IM AUFTRAG DES HETM..." / "<WIR SIND AUF DER SUCHE NACH..." */

				} while (answer == -1);

				if (answer == 1)
				{
					/* offer a deposit */
					gs_tevent037_flag = 1;

					/* select a hero randomly */
					hero = get_hero(answer = get_random_hero());

					sprintf(g_dtp2, get_tx2(45), hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_4TH)); /* "DER GREIF DEUTET MIT EINER SEINER SCHWINGEN AUF %s@<GUT! LASST %s H..." */
					GUI_output(g_dtp2);

					add_hero_ap(hero, 20);

					add_hero_ap_all(20);

					GUI_output(get_tx2(46)); /* "<ABER IHR SOLLT EUER PFAND EINLÖSEN, WENN IHR ES VERMÖGT. ICH WERDE..." */

					GUI_input(get_tx2(47), 10); /* "<UNGEBROCHEN IST ES DAS ALLES BEWEGENDE, DOCH NEHMT DIE HÄLFTE FORT..." */

					/* check if the answer is "RAD" 1st */
					if (strcmp(g_text_input_buf, g_str_rad1))
					{
						GUI_input(get_tx2(48), 10); /* "<ICH SEHE SCHON: DAS PFAND IST SICHER IN MEINEN KLAUEN, VERSUCHT ES..." */

						/* check if the answer is "RAD" 2nd */
						if (strcmp(g_text_input_buf, g_str_rad2))
						{
							GUI_input(get_tx2(49), 10); /* "<MAN BRAUCHT GEDULD MIT EUCH ARMEN GESCHÖPFEN, IHR MÜSST WOHL TATSÄ..." */

							/* check if the answer is "RAD" 3rd */
							if (strcmp(g_text_input_buf, g_str_rad3))
							{
								sprintf(g_dtp2, get_tx2(50), hero->alias, GUI_get_definite_article(hero->sex, GRAMMAR_CASE_1ST), hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_4TH)); /* "<LASST ES BLEIBEN, UNSER TAUSCH IST JA LÄNGST BESCHLOSSEN. IHR HABT..." */

								GUI_output(g_dtp2);

								/* the hero is now lost completely */
								hero_disappear(hero, answer, -1);

								done = 1;
							} else {

								GUI_output(get_tx2(51)); /* "<DAS WEGRECHT FÜR DAS PFAND, UND DAS PFAND FÜR DAS RAD!>@MIT EIN PA..." */

								add_hero_ap_all(10);

								done = 1;
							}
						} else {

							GUI_output(get_tx2(51)); /* "<DAS WEGRECHT FÜR DAS PFAND, UND DAS PFAND FÜR DAS RAD!>@MIT EIN PA..." */

							add_hero_ap_all(10);

							done = 1;
						}
					} else {

						GUI_output(get_tx2(51)); /* "<DAS WEGRECHT FÜR DAS PFAND, UND DAS PFAND FÜR DAS RAD!>@MIT EIN PA..." */

						add_hero_ap_all(10);

						done = 1;
					}

				} else {
					/* talk of hetmann or hyggelik */

					do {
						answer = GUI_radio(get_tx2(43), 2, get_tx2(33), get_tx2(34)); /* "DAS BRÜLLEN DES GREIFEN IST W..." / "IHR BEFOLGT DIE WORTE DES GRE..." / "IHR VERSUCHT ZU VERHANDELN." */

					} while (answer == -1);

					if (answer == 2)
					{
						/* select a hero randomly */
						hero = get_hero(get_random_hero());

						timewarp(HOURS(1));

						sprintf(g_dtp2, get_tx2(44), hero->alias); /* "MIT EINEM MALE HOLT DER GREIF MIT SEINEN SCHWINGEN AUS UND HOLT %s..." */
						GUI_output(g_dtp2);
					}
				}
			}

		} while (!done);

		disable_ani();

		g_event_ani_busy = 0;
		g_request_refresh = 1;
	}
}

/* unicorn 1st time */
/* Phexcaer <-> Einsiedlersee (INFO6.TLK); gleiche Begegnung wie tevent_087 */
void tevent_038(void)
{
	if (!gs_met_unicorn_flag && gs_got_main_quest)
	{
		do_talk(11, 2);

		gs_met_unicorn_flag = 1;
	}
}

/* Orkanger <-> Clanegh: Schneewaechte (nur im Winter) */
void tevent_078(void)
{
	/* This event happens only in winter */
	if (!get_current_season())
	{
		signed int tmp;
		signed int answer;
		signed int found_path;
		signed int days;
		struct struct_hero *hero;

		do {
			answer = GUI_radio(get_tx2(78), 2, get_tx2(79), get_tx2(80)); /* "VOR EUCH IST EINE SCHNEEWÄCHT..." / "VERSUCHEN, EUCH EINEN WEG FRE..." / "DIE VERSCHÜTTETE STELLE WEITR..." */

		} while (answer == -1);

		load_in_head(57);

		if (answer == 1)
		{
			/* try to clean the path */

			/* with SHOVEL/SCHAUFEL 5 Hours, without 8 Hours */
			if (get_first_hero_with_item(ITEM_ID_SCHAUFEL) != -1) {
				tmp = 5;
			} else {
				tmp = 8;
			}

			timewarp(HOURS(tmp));

			GUI_dialog_na(0, get_tx2(81)); /* "SCHRITT FÜR SCHRITT SCHAUFELT IHR EUCH VORWÄRTS. ES IST BITTERKALT...." */

			/* each hero in the group looses tmp / 2 LE */
			sub_group_le(tmp >> 1);

			g_wildcamp_sleep_quality = -6;

			gs_town_loc_type = LOCTYPE_WILDCAMP;
			do_location();
			gs_town_loc_type = LOCTYPE_NONE;

			TRV_load_textfile(-1);

#if FIX_0168
			/* Fix 168: Kopfbild nach dem Lager neu laden */
			/* if a campfight occurs, the picture needs to be reloaded */
			load_in_head(57);
#endif
			GUI_dialog_na(0, get_tx2(84)); /* "NACH DER RAST GEHT DIE ARBEIT ETWAS SCHNELLER VONSTATTEN, UND NACH..." */

		} else {
			/* try to find another path */
			found_path = days = 0;

			do {
				/* ORIENTATION + 8 */
				if (test_talent(get_first_hero_available_in_group(), TA_ORIENTIERUNG, 8) > 0)
				{
					/* success, you found a way */
					timewarp(HOURS(5));

					sub_group_le(2);

					/* Fehler im Original: answer ist hier 2 (erster Versuch) oder 1 (nach einem Fehlversuch),
					 * nie 0; Text 82 ("... BIS IHR ENDLICH AUF DEN GEWUENSCHTEN WEG ZURUECKFINDET") erscheint
					 * nie. Gemeint war vermutlich answer == 2. */
#if FIX_0183
					/* Fix 183: beim ersten Versuch (answer == 2) der Text fuer den schnellen Erfolg */
					GUI_dialog_na(0, (answer == 2) ? get_tx2(82) : get_tx2(85));
#else
					GUI_dialog_na(0, !answer ? get_tx2(82) : get_tx2(85));
#endif /* "STUNDE UM STUNDE MARSCHIERT IHR, EURE AUGE..." / "ENDLICH ERREICHT IHR EINE STELLE, DIE IHR..." */

					found_path = 1;
				} else {
					/* fail */
					timewarp(HOURS(8));

					GUI_dialog_na(0, get_tx2(83)); /* "STUNDE UM STUNDE MARSCHIERT IHR DURCH DIE ENDLOSE SCHNEEWÜSTE, EURE..." */

					sub_group_le(4);

					g_wildcamp_sleep_quality = -6;
					/* make a camp */
					gs_town_loc_type = LOCTYPE_WILDCAMP;
					do_location();
					gs_town_loc_type = LOCTYPE_NONE;

					TRV_load_textfile(-1);

					answer = 1;
				}
#if FIX_0168
				/* Fix 168: Kopfbild nach dem Lager neu laden */
				/* if a campfight occurs, the picture needs to be reloaded */
				load_in_head(57);
#endif

			} while (found_path == 0 && ++days < 6);

			if (found_path == 0)
			{
				GUI_dialog_na(0, get_tx2(86)); /* "DAS IST DAS ENDE! NACH FÜNF TAGEN HABT IHR NICHT MEHR DIE KRAFT,@WE..." */

				timewarp(HOURS(8));

				/* TODO: Original-Bug: all heroes die, even if they are not in the current group */
			/* (ausserdem wird hero_disappear() auch fuer leere Plaetze aufgerufen und zaehlt
			 *  gs_total_hero_counter und gs_group_member_counts entsprechend oft herunter) */
				hero = get_hero(0);
				for (tmp = 0; tmp <= 6; tmp++, hero++)
				{
#if !FIX_0184
					/* Original-Bug 16: all heroes die, even if they are not in the current group */
					hero_disappear(hero, tmp, -1);
#else
					/* Fix 184: nur die Helden der verirrten Gruppe */
					if ((hero->typus != HERO_TYPE_NONE) &&
						(hero->group_id == gs_active_group_id))
					{
						hero_disappear(hero, tmp, -1);
					}
#endif
				}
			}
		}

		g_wildcamp_sleep_quality = 0;
	}
}

/* Orkanger <-> Clanegh: Rastplatz */
void tevent_079(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 4) > 0 && !gs_tevent079_flag) ||
		gs_tevent079_flag != 0)
	{
		TRV_found_camp_place(0);
		gs_tevent079_flag = 1;
	}
}

/* Ottarje <-> Skal: entrance to spider cave */
/* (Ottarje <-> Skjal) */
void tevent_051(void)
{
	signed int answer;

	if (test_talent(get_first_hero_available_in_group(), TA_FAEHRTENSUCHEN, 4) > 0 && !gs_tevent051_flag)
	{
		gs_tevent051_flag = 1;

		if (!TRV_follow_trail_question())
		{

			GUI_output(get_tx2(65)); /* "DIE SPUR IST NICHT SEHR DEUTLICH. ES SIEHT AUS, ALS WÄRE HIER EIN G..." */
			GUI_output(get_tx2(66)); /* "NACH EINIGER ZEIT HÄUFEN SICH AM BODEN UND IN DEN ZWEIGEN KLEBRIGE..." */

			load_in_head(53);

			do {
				answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(67), 2, get_tx2(68), get_tx2(69)); /* "IHR ERREICHT EIN KLEINES, TIE..." / "DAS NETZ DURCHTRENNEN UND DIE..." / "UMKEHREN UND DEN WEG FORTSETZEN?" */

			} while (answer == -1);

			if (answer == 1)
			{
				gs_travel_detour = DUNGEON_ID_SPINNENHOEHLE;
			}
		}

	} else if (gs_tevent051_flag != 0) {

		load_in_head(53);

		do {
			answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(70), 2, get_tx2(71), get_tx2(72)); /* "HIER FÜHRT EIN PFAD IN EIN VO..." / "DAS TAL AUFSUCHEN?" / "DEN WEG FORTSETZEN" */

		} while (answer == -1);

		if (answer == 1)
		{
			gs_travel_detour = DUNGEON_ID_SPINNENHOEHLE;
		}
	}
}

/* Ottarje <-> Skjal: narrow pass blocked by a tree */
void tevent_052(void)
{
	signed int done = 0;
	signed int i;
	signed int answer;

	load_in_head(52);

	do {

		do {
			answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(73), 2, get_tx2(74), get_tx2(75)); /* "MITTEN AUF DEM HOHLWEG, AUF D..." / "EIN STÜCK ZURÜCKGEHEN UND EIN..." / "DIE AXT ZUR HAND NEHMEN UND E..." */

		} while (answer == -1);

		if (answer == 1)
		{
			timewarp(HOURS(1));

			GUI_dialog_na(0, get_tx2(76)); /* "NACH ETWA EINER STUNDE FINDET IHR EINEN GANGBAREN WEG, DER ETWA IN..." */

			done = 1;
		} else {

			i = 0;

			do {
				/* check for a chopping tool */
				if (get_first_hero_with_item(g_travel_event_axes[i]) != -1)
				{
					done = 1;
				}

			} while (!done && (g_travel_event_axes[++i] != 0xff));

			if (done)
			{
				timewarp(HOURS(3));

				GUI_dialog_na(0, get_tx2(77)); /* "ES DAUERT ZWAR GERAUME ZEIT, BIS IHR DIE RIESIGE EICHE IN HANDLICHE..." */
			} else {
				GUI_dialog_na(0, get_tx2(78)); /* "IHR HABT LEIDER KEINE ÄXTE ODER BEILE DABEI!" */
			}
		}

	} while (!done);
}

/* Merske <-> Rovamund: Rastplatz mit guter Jagd */
void tevent_120(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 3) > 0 && !gs_tevent120_flag) || gs_tevent120_flag)
	{
		gs_tevent120_flag = 1;
		TRV_found_camp_place(1);
	}
}

/* Orvil <-> Rovik: Furt durch den Ingval */
void tevent_121(void)
{
	TRV_cross_a_ford(get_tx(53), 30, 0); /* "INGVAL" */
}

/* Orvil <-> Rovik: Weg an der Steilklippe */
void tevent_124(void)
{
	signed int i;
	signed int counter;
	signed int answer;
	signed int have_climb_tools;
	signed int talent_ret;
	struct struct_hero *hero;

	have_climb_tools = 0;

	GUI_output(get_tx2(49)); /* "DER WEG WINDET SICH AN DER STEILKLIPPE ENTLANG. UNTER EUCH KÖNNT IH..." */

	if (DNG_check_climb_tools() != -1)
	{
		have_climb_tools = 1;

		GUI_output(get_tx2(50)); /* "VORSICHTSHALBER SEILT IHR EUCH AN." */
	}

	do {
		answer = GUI_radio(get_tx2(51), 2, get_tx2(52), get_tx2(53)); /* "JETZT KOMMT EINE STELLE, AN D..." / "DAS WAGNIS AUF EUCH NEHMEN?" / "ODER LIEBER UMKEHREN?" */

	} while (answer == -1);

	if (answer == 2)
	{

#if !FIX_0150
		/* Original-Bug 53:
		 * If the party was already traveling backward, the new direction should be forward. */
		gs_journey_direction = JOURNEY_DIRECTION_CHANGE_TO_BACKWARD;
#else
		/* Fix 150: Umkehren kehrt die aktuelle Richtung um */
		gs_journey_direction = (gs_journey_direction == JOURNEY_DIRECTION_FORWARD ? JOURNEY_DIRECTION_CHANGE_TO_BACKWARD : JOURNEY_DIRECTION_CHANGE_TO_FORWARD);
#endif

	} else {

		hero = get_hero(0);
		for (i = counter = 0; i <= 6; i++, hero++)
		{
			if ((hero->typus != HERO_TYPE_NONE) &&
				(hero->group_id == gs_active_group_id) &&
				!hero->flags.dead &&
				(talent_ret = test_talent(hero, TA_KLETTERN, -2)) <= 0)
			{
				/* the climb test failed */
				counter++;

				/* Original-Bug: a talent test with a fatal result returns -99, not -1 */
				/* (Folge im Original: der toedliche Sturz trifft, wer die Kletterprobe um genau 1 verfehlt) */
#if FIX_0185
				/* Fix 185: toedlich ist nur der Patzer (-99) */
				if (talent_ret == -99 && have_climb_tools == 0)
#else
				if (talent_ret == -1 && have_climb_tools == 0)
#endif
				{
					/* fatal */
					sprintf(g_dtp2, get_tx2(56), hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST)); /* "UNTER DEN HÄNDEN VON %s LÖST SICH PLÖTZLICH EIN SICHER GEGLAUBTER@F..." */
					GUI_output(g_dtp2);

					hero_disappear(hero, i, -1);

				} else {

					sprintf(g_dtp2, get_tx2(55), hero->alias); /* "%s RUTSCHT AB, SCHLITTERT EINIGE METER DEN HANG HINAB UND KANN SICH..." */
					GUI_output(g_dtp2);

					sub_hero_le(hero, random_schick(5));
				}
			}
		}

		if (!counter) {
			GUI_output(get_tx2(54)); /* "IHR HANGELT EUCH AN DER WAND ENTLANG, UND MEHR ALS EINMAL RUTSCHT E..." */
		}
	}
}

/* Ljasdahl (Rundweg): Hirte */
void tevent_125(void)
{
	signed int answer;

	load_in_head(17);

	do {
		answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(57), 3, get_tx2(58), get_tx2(59), get_tx2(60)); /* "EIN HIRTE HÜTET HIER AUF EINE..." / "DEN HIRTEN NACH DEM WEG FRAGEN?" / "EUCH ÜBER EVENTUELLE GEFAHREN..." / "DEN HIRTEN GRÜSSEN UND DANN W..." */

	} while (answer == -1);

	if (answer == 1 || answer == 2)
	{
		GUI_dialog_na(0, answer == 1 ? get_tx2(61) : get_tx2(62)); /* "<DIESER WEG FÜHRT VON LJASDAHL ENTLANG DER..." / "<NEIN, HIER WAR ES IMMER RUHIG. AUSSER UNS..." */

		timewarp(MINUTES(30));
	}
}

/* How to proceed after travelling the crosslink land route?
 * The reached endpoint of the crosslink is either the land route Kravik-Skelellen or theland  route Peilinen-Rovamund. */
void tevent_145(void)
{
	signed int answer;

	load_area_description(0);

	if (!(g_journey_crosslink_status & 1))
	{
		/* The crosslink was either travelled from Peilinen (g_journey_crosslink_status == 2)
		 * or from Rovamund (g_journey_crosslink_status == 4).
		 * Hence, the reached land route is Kravik-Skelellen.
		 */
		do {
			answer = GUI_radio(get_tx2(42), 2, get_tx2(43), get_tx2(44)); /* "IHR GELANGT WIEDER AUF EINEN..." / "NACH KRAVIK" / "NACH SKELELLEN" */

		} while (answer == -1);

		if (answer == 1)
		{
			gs_journey_destination_town_id = TOWN_ID_KRAVIK;
			gs_town_id = TOWN_ID_SKELELLEN;

		} else {
			gs_journey_destination_town_id = TOWN_ID_SKELELLEN;
			gs_town_id = TOWN_ID_KRAVIK;
		}

	} else {
		/* The crosslink was either travelled from Kravik (g_journey_crosslink_status == 1)
		 * or from Skelellen (g_journey_crosslink_status == 3).
		 * Hence, the reached land route is Peilinen-Rovamund.
		 */
		do {
			answer = GUI_radio(get_tx2(39), 2, get_tx2(40),	get_tx2(41)); /* "IHR GELANGT WIEDER AUF EINEN..." / "NACH PEILINEN" / "NACH ROVAMUND" */

		} while (answer == -1);

		if (answer == 1) {

			gs_journey_destination_town_id = TOWN_ID_PEILINEN;
			gs_town_id = TOWN_ID_ROVAMUND;

		} else {
			gs_journey_destination_town_id = TOWN_ID_ROVAMUND;
			gs_town_id = TOWN_ID_PEILINEN;
		}
	}

	sprintf(g_dtp2, get_tx2(45), get_ttx(gs_journey_destination_town_id + 0xeb)); /* "PROBLEMLOS SETZT IHR EUREN WEG NACH %s FOR..." / TEXT.LTX 236..287: Ortsname */
	GUI_output(g_dtp2);
}
