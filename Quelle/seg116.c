/*
 * seg116 - Reiseereignisse 8/10 (Ereignisse 130-144, 136) und das Gespraech mit der Alten
 *
 * Inhalt:    Ereignisse zwischen Brendhil und Manrin (Jaegerhuette, Zugang zur Piratenhoehle), am
 *            Einsiedlersee (Sumpf, Kairanhalm, Monolith zum Erklettern mit Hinweis auf die Kartenteile,
 *            zweite Einhornbegegnung, Seeufer), Runin, auf der Querverbindung (Urwald, Hirschjagd) und
 *            das Heerlager der Orks mit dem Endkampf (tevent_144). TLK_old_woman() behandelt die
 *            Zustaende von F092.TLK (tevent_092).
 * Aufrufer:  TRV_event() (seg109); TLK_old_woman() aus talk_switch() in seg030; tevent_144_unused()
 *            wird nie aufgerufen.
 * Daten:     g_tevent135_climb_damage (Sturzschaden je Hoehe am Monolithen).
 * Besonders: TLK_old_woman(): Verstecken-Proben fuer alle sieben Plaetze (Original-Bug), Zustand 33 ist
 *            eine Endlosschleife. Kairanhalm/Jorugawurzel als Spezialkraut wirken nie (siehe seg112).
 *            tevent_144 laesst, wenn Grimring im Statusmenue abgelegt wird, Heldenplatz 0 kaempfen.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg116.c
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
#include "seg047.h"
#include "seg048.h"
#include "seg065.h"
#include "seg096.h"
#include "seg097.h"
#include "seg103.h"
#include "seg105.h"
#include "seg109.h"
#include "seg116.h"

static const struct climb_damage_range g_tevent135_climb_damage[5] = { { 1, 5 }, { 4, 13 }, { 7, 21 }, { 10, 32 }, { 15, 40 } }; // ds:0xb174

/* Brendhil <-> Manrin: Huette mit muerrischem Jaeger */
void tevent_130(void)
{
	signed int answer;

	if (TRV_enter_hut_question()) {

		gs_camp_incident = 0; /* Zwischenfall in der 1. Wache: der Jaeger kommt */

		gs_town_loc_type = LOCTYPE_WILDCAMP;
		do_location();
		gs_town_loc_type = LOCTYPE_NONE;

		TRV_load_textfile(-1);
		gs_camp_incident = -1;

		load_in_head(10);

		do {
			answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(19), 2, get_tx2(20), get_tx2(21)); /* "NACH ETWA EINER STUNDE ÖFFNET..." / "DEN JÄGER ANSPRECHEN?" / "DEN UNFREUNDLICHEN KERL IN RU..." */

		} while (answer == -1);

		if (answer == 1) {

			do {
				answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(22), 3, get_tx2(23), get_tx2(24), get_tx2(25)); /* "DER JÄGER STARRT EUCH AN, ALS..." / "<WIR SIND AUF DER SUCHE NACH..." / "<WIR WOLLEN EINEN BEKANNTEN I..." / "<WANDERN HALT SO RUM.>" */

			} while (answer == -1);

			if (answer == 1 || answer == 2) {
				GUI_dialog_na(0, answer == 1 ? get_tx2(26) : get_tx2(27)); /* "<ACH SO, NA DANN VIEL GLÜCK.> GRUMMELT DER..." / "<NA DANN PASST AUF, DASS IHR NICHT DEN PIR..." */
			} else {
				GUI_dialog_na(0, test_attrib(get_first_hero_available_in_group(), ATTRIB_CH, 0) > 0 ? get_tx2(27) : get_tx2(26)); /* "<NA DANN PASST AUF, DASS IHR NICHT DEN PIR..." / "<ACH SO, NA DANN VIEL GLÜCK.> GRUMMELT DER..." */
			}
		}

		gs_town_loc_type = LOCTYPE_WILDCAMP;
		do_location();
		gs_town_loc_type = LOCTYPE_NONE;
	}
}

/* Manrek: entrance to the dungeon "pirate cave" */
/* (Brendhil <-> Manrin) */
void tevent_131(void)
{
	signed int answer;

	if (test_talent(get_first_hero_available_in_group(), TA_SINNESSCHAERFE, 8) > 0 && !gs_tevent131_flag) {

		gs_tevent131_flag = 1;

		GUI_output(get_tx2(28)); /* "ABSEITS DES WEGES KÖNNT IHR DIE SPUREN VON STIEFELN UND SANDALEN ER..." */

		do {
			answer = GUI_radio(get_tx2(29), 2, get_tx2(30), get_tx2(31)); /* "NACH EINIGEN DUTZEND SCHRITTE..." / "DIE HÖHLE UNTERSUCHEN?" / "UMKEHREN UND EUREN WEG FORTSE..." */

		} while (answer == -1);

		if (answer == 1) {
			gs_travel_detour = DUNGEON_ID_PIRATENHOEHLE;
		}

	} else {

		if (gs_tevent131_flag) {

			do {
				answer = GUI_radio(get_tx2(32), 2, get_tx2(33),	get_tx2(34)); /* "EIN VERSTECKTER PFAD FÜHRT VO..." / "DIE HÖHLE AUFSUCHEN?" / "DEN WEG FORTSETZEN?" */

			} while (answer == -1);

			if (answer == 1) {
				gs_travel_detour = DUNGEON_ID_PIRATENHOEHLE;
			}
		}
	}
}

/* Brendhil <-> Manrin: Rastplatz, evtl. mit Wirselkraut */
void tevent_132(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 1) > 0 && !gs_tevent132_flag) || gs_tevent132_flag)
	{

		/* set this camp place as known */
		gs_tevent132_flag = 1;

		if ((test_talent(get_first_hero_available_in_group(), TA_PFLANZENKUNDE, 1) > 0 && !gs_tevent132_herb_flag) || gs_tevent132_herb_flag) {

			gs_tevent132_herb_flag = 1;

			g_gather_herbs_special = ITEM_ID_WIRSELKRAUT;

			TRV_found_camp_place(2);

			g_gather_herbs_special = -1;
		} else {
			TRV_found_camp_place(0);
		}
	}
}

/* Einsiedlersee (Rundweg): Sumpf */
void tevent_133(void)
{
	signed int i;
	signed int answer;
	struct struct_hero *hero;

	do {
		answer = GUI_radio(get_tx2(35), 2, get_tx2(36), get_tx2(37)); /* "DAS GELÄNDE WIRD ZUSEHENDS SU..." / "UMKEHREN UND EINEN ANDEREN WE..." / "VORSICHTIG WEITERGEHEN?" */

	} while (answer == -1);

	if (answer == 1) {

#if !FIX_0150
		/* Original-Bug 53:
		 * If the party was already traveling backward, the new direction should be forward. */
		gs_journey_direction = JOURNEY_DIRECTION_CHANGE_TO_BACKWARD;
#else
		/* Fix 150: Umkehren kehrt die aktuelle Richtung um */
		gs_journey_direction = (gs_journey_direction == JOURNEY_DIRECTION_FORWARD ? JOURNEY_DIRECTION_CHANGE_TO_BACKWARD : JOURNEY_DIRECTION_CHANGE_TO_FORWARD);
#endif

	} else {

		GUI_output(get_tx2(38)); /* "DAS SUMPFLAND SCHEINT SICH ENDLOS DAHINZUZIEHEN..." */

		hero = get_hero(0);
		for (i = 0; i <= 6; i++, hero++) {

			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !hero->flags.dead)
			{
				timewarp(HOURS(1));

				if (test_attrib(hero, ATTRIB_GE, 4) <= 0) {

					/* test failed */
					timewarp(HOURS(1));

					sprintf(g_dtp2, get_tx2(39), hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_3RD), GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_4TH)); /* "%s SINKT PLÖTZLICH BIS ZU DEN HÜFTEN EIN UND DROHT, VOLLSTÄNDIG@UNT..." */
					GUI_output(g_dtp2);

					sub_hero_le(hero, random_schick(8));

					lose_random_item(hero, 50, get_ttx(506)); /* "OH VERDAMMT, %s HAT %s VERLOREN!" */
					lose_random_item(hero, 50, get_ttx(506)); /* "OH VERDAMMT, %s HAT %s VERLOREN!" */
					lose_random_item(hero, 50, get_ttx(506)); /* "OH VERDAMMT, %s HAT %s VERLOREN!" */
				}
			}
		}

		GUI_output(get_tx2(40)); /* "NACH FAST EINEM TAG - SO WILL ES EUCH SCHEINEN - ERREICHT IHR ENDLI..." */
	}
}

/* Einsiedlersee (Rundweg): Kraeuterplatz mit Kairanhalm (Bonus wirkungslos, siehe seg112 tevent_070) */
void tevent_134(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_PFLANZENKUNDE, 6) > 0 && !gs_tevent134_flag) ||
		gs_tevent134_flag)
	{
		g_gather_herbs_special = (int8_t)ITEM_ID_KAIRANHALM;	/* REMARK: item_id > 127 */

		TRV_found_herb_place(0);

		g_gather_herbs_special = -1;

		gs_tevent134_flag = 1;
	}
}

void tevent_135(void)
/* Einsiedlersee <-> Einsiedlersee: Monolith */
{
	signed int tmp; /* used for (1) fall_height; (2) LE of the hero after falling */
	int16_t tmp2;	/* used for (1) map tile position (2) falling damage */
			/* (Korrektur: (1) ist der Index der neun Kartenteile gs_treasure_maps[]) */
	signed int answer;
	signed int done;
	signed int count;
	struct struct_hero *hero;

	load_ani(1);
	draw_main_screen();
	init_ani(0);

	GUI_output(get_tx2(41)); /* "SCHON AUS GERAUMER ENTFERNUNG KÖNNT IHR ERKENNEN, DASS IHR EUCH AUF..." */

	do {
		answer = GUI_radio(get_tx2(42), 2, get_tx2(43), get_tx2(44)); /* "DER MONOLITH SIEHT NICHT EINF..." / "JA, DAS KLINGT VERLOCKEND." / "NEIN, WIR SIND DOCH NICHT VER..." */

	} while (answer == -1);

	if (answer == 1) {

		do {
			done = 0;

			hero = get_hero(select_hero_ok_forced(get_tx2(45))); /* "WER SOLL KLETTERN?" */

			tmp = 1;
			if (test_talent(hero, TA_KLETTERN, -1) > 0) {
				tmp = 2;
				GUI_output(get_tx2(46)); /* "DIE ERSTEN ZEHN SCHRITT SIND EIN KINDERSPIEL..." */

				if (test_talent(hero, TA_KLETTERN, 1) > 0) {
					tmp = 3;
					GUI_output(get_tx2(47)); /* "...DIE NÄCHSTEN ZEHN SCHON BEDEUTEND SCHWIERIGER..." */

					if (test_talent(hero, TA_KLETTERN, 0) > 0) {
						tmp = 4;
						GUI_output(get_tx2(48)); /* "...JETZT FOLGT EINE LEICHTERE ETAPPE..." */

						if (test_talent(hero, TA_KLETTERN, 2) > 0) {
							tmp = 5;
							GUI_output(get_tx2(49)); /* "...EIN HALSBRECHERISCHER ÜBERHANG..." */

							if (test_talent(hero, TA_KLETTERN, 1) > 0) {

								GUI_output(get_tx2(50)); /* "...UND SCHLIESSLICH EINE STEILE, ABER GRIFFIGE WAND." */

								sprintf(g_dtp2, get_tx2(54), hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_3RD)); /* "GESCHAFFT! %s ERREICHT EIN PLATEAU VON VIELLEICHT ZEHN MAL ZEHN SCH..." */
								GUI_output(g_dtp2);

								load_in_head(45);

								sprintf(g_dtp2 + 0x400, get_tx2(55), /* "IN DER MITTE DES PLATEAUS ENTDECKT %s EINEN KLEINEN SCHREIN, IN DEM..." */
									GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST),
									GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST));

								GUI_dialog_na(0, (char*)(g_dtp2 + 0x400));

								for (tmp2 = count = 0; tmp2 < 9; tmp2++) {
									if (gs_treasure_maps[tmp2] != 0) {
										count++;
									}
								}

								if (count < 5) {
									sprintf(g_dtp2 + 0x400, get_tx2(56), /* "GRELLBUNTE FARBEN UND FREMDARTIGE TÖNE WIRBELN %s DURCH DEN KOPF, U..." */
										hero->alias, get_tx2(57)); /* "<DIE KARTE, DIE IHR SUCHT, BESTEHT AUS NEUN TEILEN, DIE IN DEN HÄND..." */
								} else {
									sprintf(g_dtp2 + 0x400, get_tx2(56), /* "GRELLBUNTE FARBEN UND FREMDARTIGE TÖNE WIRBELN %s DURCH DEN KOPF, U..." */
										hero->alias, get_tx(random_interval(54, 67))); /* 54..67 zufällig, z.B. "JURGE THORFINSSON WOHNT IN BREIDA" */
								}

								GUI_dialog_na(0, (char*)(g_dtp2 + 0x400));

								if (!(hero->start_gear__monolith_reward & 0x2)) {

									hero->start_gear__monolith_reward |= 0x02;
									hero->attrib[ATTRIB_IN].normal++;
									hero->attrib[ATTRIB_IN].current++;
								}

								sprintf(g_dtp2 + 0x400, get_tx2(58), hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_4TH)); /* "VERWIRRT VERHARRT %s EINIGE MINUTEN AUF DER STELLE, BIS %s EURE RUF..." */
								GUI_dialog_na(0, (char*)(g_dtp2 + 0x400));

								tmp = 0;
								done = 1;
							}
						}
					}
				}
			}

			if (tmp) {

				sprintf(g_dtp2, get_tx2(51), hero->alias); /* "%s VERLIERT DEN HALT UND STÜRZT IN DIE TIEFE!" */
				GUI_output(g_dtp2);

				/* depending on fall height stored in tmp: damage in the interval [1..5], [4..13], [7..21], [10..32], [15..40] */
				tmp2 = random_interval(g_tevent135_climb_damage[tmp - 1].lo, g_tevent135_climb_damage[tmp - 1].hi);

				tmp = hero->le;
				tmp -= tmp2;

				sub_hero_le(hero, tmp2);

				if (tmp <= 0) {
					sprintf(g_dtp2, (char*)(!tmp ? get_tx2(53) : get_tx2(52)), hero->alias); /* "%s IST SCHWER VERLETZT! JETZT TUT SCHNELLE..." / "EXITUS! %s HAT SICH DAS GENICK GEBROCHEN." */
					GUI_output(g_dtp2);
				}

#if FIX_0080
				/* Fix 80: der NSC allein versucht es nicht noch einmal; die Spielschleife prueft die Gruppe */
				g_check_party = 1;
				if (count_heroes_available_in_group() > is_hero_available_in_group(get_hero(6))) {
#else
				if (count_heroes_available_in_group()) { /* potential Original-Bug: Does this make sense if there is only the NPC left? (Can it happen?) */
#endif

					if (!GUI_bool(get_tx2(59))) { /* "WOLLT IHR ES NOCH EINMAL VERSUCHEN?" */
						done = 1;
					}
				} else {
					done = 1;
				}
			}

		} while (done == 0);
	}
	/* potential Original-Bug: Can it happen that only the NPC survives? What then? */

	disable_ani();
	g_request_refresh = 1;
}

/* Einsiedlersee (Rundweg): Seeufer, Vorraete auffuellen */
void tevent_137(void)
{
	signed int hero_pos;
	signed int answer;
	signed int inv_slot;
	struct struct_hero *hero;

	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 5) > 0 && !gs_tevent137_flag) ||
		gs_tevent137_flag)
	{
		gs_tevent137_flag = 1;

		do {
			answer = GUI_radio(get_tx2(60), 2, get_tx2(61),	get_tx2(62)); /* "IHR ERREICHT DAS UFER EINES K..." / "EURE BROTBEUTEL UND WASSERSCH..." / "WEITERREISEN?" */

		} while (answer == -1);

		if (answer == 1) {

			hero = get_hero(0);
			for (hero_pos = 0; hero_pos <= 6; hero_pos++, hero++) {

				if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !hero->flags.dead)
				{
					/* each hero gets five FOODPACKAGES */
					give_new_item_to_hero(hero, ITEM_ID_PROVIANTPAKET, 1, 5);

					/* each hero gets his first WATERSKIN filled */
					/* potential Original-Bug: Does it make sense that the further WATERSKINs are not filled? */

					if ((inv_slot = inv_slot_of_item(hero, ITEM_ID_WASSERSCHLAUCH)) != -1)
					{
						/* fill waterskin */
						hero->inventory[inv_slot].flags.half_empty = hero->inventory[inv_slot].flags.empty = 0;
					}

					hero->hunger = hero->thirst = 0;

					add_hero_le(hero, 2);
				}
			}
		}
	}
}

/* Leuchtturm Runin <-> Runinshaven: Rastplatz mit guter Jagd */
void tevent_138(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 2) > 0 && !gs_tevent138_flag) || gs_tevent138_flag)
	{
		gs_tevent138_flag = 1;
		TRV_found_camp_place(1);
	}
}

/* Querverbindung: Urwald (mit dem Saebel durch oder Umweg) */
void tevent_139(void)
{
	signed int i;
	signed int answer;
	struct struct_hero *hero;

	do {
		answer = GUI_radio(get_tx2(63), 2, get_tx2(64), get_tx2(65)); /* "DER PFAD IST NUR AUSGESPROCHE..." / "EUCH MIT DEM SÄBEL DURCH DAS..." / "VERSUCHEN, EINEN BESSEREN WEG..." */

	} while (answer == -1);

	if (answer == 1) {

		hero = get_hero(0);
		for (i = 0; i <= 6; i++, hero++) {

			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !hero->flags.dead)
			{
				sub_hero_le(hero, random_schick(2));
			}
		}

		timewarp(HOURS(6));

		GUI_output((i = test_talent(get_first_hero_available_in_group(), TA_ORIENTIERUNG, 3)) > 0 ? get_tx2(66) : get_tx2(68)); /* "IHR HACKT EUCH SO DURCH...@UND NACH SECHS..." / "ALS IHR NACH EINIGEN STUNDEN DAS DICHTE UN..." */
	} else {
		timewarp(HOURS(2));

		GUI_output((i = test_talent(get_first_hero_available_in_group(), TA_ORIENTIERUNG, 5)) > 0 ? get_tx2(67) : get_tx2(69)); /* "SICH IM WALD ZU ORIENTIEREN, IST ZWAR SCHW..." / "IRGENDWIE SIEHT HIER JEDER BAUM AUS WIE DE..." */
	}

	if (i <= 0) {
		if (test_talent(get_first_hero_available_in_group(), TA_ORIENTIERUNG, 3) > 0) {

			timewarp(HOURS(2));

			GUI_output(get_tx2(70)); /* "IHR IRRT NOCH EINE GERAUME ZEIT UMHER, BIS IHR SCHLIESSLICH EINEN P..." */

		} else {

			timewarp(HOURS(3));

			GUI_output(get_tx2(71)); /* "SO SEHR IHR EUCH AUCH BEMÜHT, IHR HABT VOLLSTÄNDIG DIE ORIENTIERUNG..." */

			gs_town_loc_type = LOCTYPE_WILDCAMP;
			do_location();
			gs_town_loc_type = LOCTYPE_NONE;
		}
	}
}

/* Querverbindung: Rastplatz, evtl. mit Jorugawurzel (Bonus wirkungslos) */
void tevent_140(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 2) > 0 && !gs_tevent140_flag) || gs_tevent140_flag) {

		/* set this camp place as known */
		gs_tevent140_flag = 1;

		if ((test_talent(get_first_hero_available_in_group(), TA_PFLANZENKUNDE, 4) > 0 && !gs_tevent140_herb_flag) || gs_tevent140_herb_flag) {

			gs_tevent140_herb_flag = 1;

			g_gather_herbs_special = (int8_t)ITEM_ID_JORUGAWURZEL;	/* REMARK: item_id > 127 */

			TRV_found_camp_place(2);

			g_gather_herbs_special = -1;
		} else {
			TRV_found_camp_place(0);
		}
	}
}

/* Querverbindung: Hirschfaehrte, Jagd */
void tevent_141(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_FAEHRTENSUCHEN, 0) > 0 && !gs_tevent141_flag) || gs_tevent141_flag)
	{
		/* set this camp place as known */
		/* (Korrektur: kein Rastplatz, sondern die Hirschfaehrte ist nun bekannt) */
		gs_tevent141_flag = 1;

		if (!TRV_follow_trail_question()) {
			TRV_hunt_generic(21, 73, -1, 5, 10, 15, 5, 5, 10, 50, 0);
		}
	}
}

/* Querverbindung: Rastplatz mit guter Jagd */
void tevent_142(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 2) > 0 && !gs_tevent142_flag) ||
		gs_tevent142_flag)
	{
		/* set this camp place as known */
		gs_tevent142_flag = 1;
		TRV_found_camp_place(1);
	}
}

/* Querverbindung: Dickicht (Texte FEATURE8.LTX) */
void tevent_143(void)
{
	signed int i;
	signed int answer;
	struct struct_hero *hero;

	do {
		answer = GUI_radio(get_tx2(0), 2, get_tx2(1), get_tx2(2)); /* "NACH EINER WEILE WIRD DER WEG..." / "EUCH MIT DEM SÄBEL EINEN WEG..." / "ODER VERSUCHEN, DAS DICKICHT..." */

	} while (answer == -1);

	if (answer == 1) {

		hero = get_hero(0);
		for (i = 0; i <= 6; i++, hero++) {

			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id))
			{
				sub_hero_le(hero, random_schick(2) + 1);
			}
		}

		timewarp(HOURS(4));

		GUI_output((i = test_talent(get_first_hero_available_in_group(), TA_ORIENTIERUNG, 2)) > 0 ? get_tx2(3) : get_tx2(5)); /* "NACH VIER STUNDEN MÜHSELIGEN GEKRAUCHES KÖ..." / "ALS IHR NACH EINIGEN STUNDEN DAS DICHTE UN..." */
	} else {
		timewarp(HOURS(1));

		GUI_output((i = test_talent(get_first_hero_available_in_group(), TA_ORIENTIERUNG, 4)) > 0 ? get_tx2(4) : get_tx2(6)); /* "IHR HABT ZWAR EINIGE MÜHE, DIE RICHTUNG ZU..." / "IHR KOMMT GLÄNZEND VORAN, ABER ALS IHR AUF..." */
	}

	if (i <= 0) {

		if (test_talent(get_first_hero_available_in_group(), TA_ORIENTIERUNG, 3) > 0) {

			timewarp(HOURS(1));

			GUI_output(get_tx2(7)); /* "SO IRRT IHR NOCH GERAUME ZEIT UMHER, BIS EUCH SCHLIESSLICH DER WILL..." */

		} else {

			timewarp(HOURS(3));

			GUI_output(get_tx2(8)); /* "SO SEHR IHR EUCH AUCH BEMÜHT, IHR HABT VOLLSTÄNDIG DIE ORIENTIERUNG..." */

			gs_town_loc_type = LOCTYPE_WILDCAMP;
			do_location();
			gs_town_loc_type = LOCTYPE_NONE;
		}
	}
}

/* Orcs' military camp */
/* Phexcaer <-> Einsiedlersee: Heerlager der Orks und Zweikampf mit Grimring (ab 10. Peraine 17 Hal) */
void tevent_144(void)
{
	signed int grimring_hero_pos;
	signed int right_time_flag;
	struct struct_hero *hero;

	right_time_flag = 0;

	if ((grimring_hero_pos = get_first_hero_with_item(ITEM_ID_DAS_SCHWERT_GRIMRING)) != -1) {

		if ((gs_year == 17) && (gs_month == 10) && (gs_day_of_month >= 10)) {
			right_time_flag = 1;
		} else {
			if (gs_orcdocument_read_flag && GUI_bool(get_tx2(46))) { /* "IHR SEID ZU FRÜH HIER EINGETROFFEN.@DIE ORKS SIND NOCH NIRGENDS ZU..." */
			/* the heroes are too early and they know about it since they have read the orc document.
			 * they decided to wait...forward time  */

				GUI_output(get_tx2(47)); /* "DER AUFMARSCHPLAN WAR KORREKT!@AM 10.PERAINE IM JAHRE 17 N.HAL WIRD..." */

				/* set date */
				gs_year = 17;
				gs_month = 10;
				gs_day_of_month = 10;

				/* seems like the timers are simply forwarded by an arbitrary time span. */
				sub_ingame_timers(MONTHS(1));
				sub_mod_timers(MONTHS(1));
				sub_heal_staffspell_timers(MONTHS(1)/MINUTES(5)); /* note that the argument is given in units of 5 minutes */
				/* BAE-TODO: argument ist given here as unsigned char + not linked correctly */
				/* Der Header deklariert sub_light_timers() ohne Prototyp, das Argument geht als int (push 64h).
				 * 100 Viertelstunden = 25 Stunden */
				sub_light_timers(100);
				right_time_flag = 1;
			}
		}

		if (right_time_flag) {

			load_in_head(44);
			memmove(g_buffer10_ptr, g_dtp2, 0x400);

			hero = get_hero(grimring_hero_pos);

			sprintf(g_dtp2, get_tx2(10), hero->alias); /* "VOR EUCH LIEGT EIN RIESIGES HEERLAGER - UND IHR KÖNNTET SCHWÖREN, A..." */

			GUI_output(g_dtp2);

			sprintf(g_dtp2, get_tx2(11), hero->alias); /* "EIN STATTLICHER, MIT EINER GROSSEN KUPFERSCHEIBE GESCHMÜCKTER@ORK -..." */

			GUI_dialogbox(g_buffer10_ptr, NULL, g_dtp2, 0);
			GUI_dialogbox(g_buffer10_ptr, NULL, get_tx2(12), 0); /* "<IM NAMEN RONDRAS UND IM NAMEN@SWAFNIRS UND AUCH IM NAMEN DES BRAZO..." */
			GUI_dialogbox(g_buffer10_ptr, NULL, get_tx2(13), 0); /* "EIN ORK, STATTLICHER NOCH ALS DER SCHAMANE, TRITT@VOR DAS ZELT, BET..." */

			sprintf(g_dtp2, get_tx2(14), hero->alias); /* "OHNE GROSS NACHZUDENKEN WENDET@SICH %s IN EINER FLIESSENDEN BEWEGUN..." */

			GUI_dialogbox(g_buffer10_ptr, NULL, g_dtp2, 0);
			GUI_dialogbox(g_buffer10_ptr, NULL, get_tx2(15), 0); /* "NACH GERAUMER ZEIT HEBT DER@HÄUPTLING DIE HAND UND, SCHLAGARTIG VER..." */

			do {
				status_menu(grimring_hero_pos);

				grimring_hero_pos = get_first_hero_with_item(ITEM_ID_DAS_SCHWERT_GRIMRING);

				if (grimring_hero_pos == -1) {
					/* Grimring wurde abgelegt: dann kaempft Platz 0, auch wenn dieser Held nicht in
					 * der Gruppe ist oder nicht kaempfen kann (vermutlich nicht bedacht) */
#if FIX_0038
					/* Fix 38: dann den ersten handlungsfaehigen Helden der Gruppe nehmen */
					grimring_hero_pos = get_hero_pos(get_first_hero_available_in_group());
#else
					grimring_hero_pos = 0;
#endif
				}

				if (grimring_hero_pos == 6) {

					sprintf(g_dtp2, get_tx2(38), get_hero(6)->alias); /* "<ICH LASSE MICH VON EUCH NICHT SO TIEF IN DIE SACHE HINEINZIEHEN>,..." */

					GUI_dialogbox(get_hero(6)->pic, get_hero(6)->alias, g_dtp2, 0);
				}

			} while (grimring_hero_pos == 6);

			gs_main_acting_hero = get_hero(grimring_hero_pos);

			final_intro();
			if (!TRV_fight_event(FIGHT_ID_F144, 144)) {
				/* you won the final fight */
				GUI_output(get_tx2(17)); /* "<DIE GÖTTER HABEN GESPROCHEN: IHR SEID DIE SIEGER UND ICH MUSS DIES..." */

				g_game_state = GAME_STATE_VICTORY;

				add_hero_ap_all(500);

				gs_town_loc_type = LOCTYPE_NONE;
				gs_npc_months = 200;
			} else {
				/* you lost the final fight */
				GUI_output(get_tx2(18)); /* "<BRAZORAGHS WILLE HAT TRIUMPHIERT! NICHTS KANN UNS AUFHALTEN! UND I..." */
				g_game_state = GAME_STATE_DEAD;
			}

#if !defined(__BORLANDC__)
			gs_main_acting_hero = NULL;
#endif
		}
	}
}

/* TODO: this function is unused */
void tevent_144_unused(void)
{
	GUI_output(get_tx2(16)); /* "<DER KAMPF MÖGE BEGINNEN!>" */
}

/**
 * \brief   Sonderbehandlung der Zustaende von F092.TLK (die Alte, tevent_092)
 *
 *          3/23: Verstecken-Proben der Gruppe, 37/41: Hinweise auf Rast- und Kraeuterplaetze am
 *          Einsiedlersee (tevent_088/091/093) und auf Isleif.
 * \param   state       Dialogzustand
 */
void TLK_old_woman(const signed int state)
{
	signed int hero_pos;
	signed int counter;
	struct struct_hero *hero;

	if (state == 3) {

		hero = get_hero(0);
		for (hero_pos = counter = 0; hero_pos <= 6; hero_pos++, hero++) {
			/* Original-Bug: check if this is really a hero in the current group and alive before test_talent((struct struct_hero*)) */
#if FIX_0153
			/* Fix 153: nur lebende Helden der aktiven Gruppe (wie count_heroes_in_group()) */
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !hero->flags.dead &&
				(test_talent(hero, TA_VERSTECKEN, -5) > 0)) {
#else
			if (test_talent(hero, TA_VERSTECKEN, -5) > 0) {
#endif
				counter++;
			}
		}

		g_dialog_next_state = (count_heroes_in_group() == counter ? 4 : 5);

	} else if (state == 4 || state == 14 || state == 21) {

		timewarp(MINUTES(15));

	} else if (state == 6 || state == 30) {

		timewarp(MINUTES(30));

	} else if (state == 23) {

		hero = get_hero(0);
		for (hero_pos = counter = 0; hero_pos <= 6; hero_pos++, hero++) {
			/* Original-Bug: check if this is really a hero in the current group and alive before test_talent((struct struct_hero*)) */
#if FIX_0153
			/* Fix 153: nur lebende Helden der aktiven Gruppe (wie count_heroes_in_group()) */
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !hero->flags.dead &&
				(test_talent(hero, TA_VERSTECKEN, -5) > 0)) {
#else
			if (test_talent(hero, TA_VERSTECKEN, -5) > 0) {
#endif
				counter++;
			}
		}

		gs_random_tlk_hero = get_hero(get_random_hero()); /* TODO: can be removed here ? */

		g_dialog_next_state = (count_heroes_in_group() == counter ? 24 : 25);

	} else if (state == 33) {

#if FIX_0174
		/* Fix 174: statt Endlosschleife das Gespraech beenden */
		g_dialog_done = 1;
#else
		/* Infinity-Loop */
		/* (das Spiel haengt, falls F092.TLK diesen Zustand je erreicht; vermutlich nie benutzt) */
		do {
		} while (1);
#endif

	} else if (state == 34) {
		g_dialog_next_state = (gs_town_id == TOWN_ID_FELSTEYN ? 35 : 39);
	} else if (state == 37) {
		gs_tevent093_flag = gs_tevent088_flag = 1;
	} else if (state == 38) {
		timewarp(HOURS(1));
	} else if (state == 41) {
#if !FIX_0076
		gs_tevent093_flag = gs_tevent091_flag = gs_informer_flags[INFORMER_ID_ISLEIF] = gs_isleif_jandas_regards = 1;
#else
		/* Fix 76: Isleif nur von unbekannt (0) auf bekannt (1) setzen, "erledigt" (2) bleibt erhalten */
		gs_tevent093_flag = gs_tevent091_flag = gs_isleif_jandas_regards = 1;
		if (!gs_informer_flags[INFORMER_ID_ISLEIF]) gs_informer_flags[INFORMER_ID_ISLEIF] = 1;
#endif
	}
}

/* Einsiedlersee (Rundweg): zweite Begegnung mit dem Einhorn (INFO7.TLK), wenn es die Karte bringen soll */
void tevent_136(void)
{
	if (gs_met_unicorn_flag && gs_got_main_quest && gs_unicorn_get_map && !gs_unicorn_timer_hours) {

#if FIX_0414
		/* Fix 414: Heldenzeiger aus der gespeicherten Heldenposition neu setzen, der Speicherzeiger im Spielstand gilt nach dem Laden nicht mehr */
		gs_unicorn_hero_ptr = (uint8_t*)get_hero(gs_unicorn_hero_pos);
#endif
		do_talk(12, 1);
		gs_met_unicorn_flag = 1;
		gs_unicorn_get_map = 0;
	}
}
