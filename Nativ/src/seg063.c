/*
 * seg063 - Hafen: Passage buchen, Hafenmeister, Seereise
 *
 * Inhalt:    do_harbor() ist der Hafen: Passage buchen (Angebote aus prepare_passages,
 *            seg064), Hafenmeister (fuer 1 Silberling: ankommende Schiffe oder alle Routen),
 *            an Bord gehen. Bei Abfahrt wird bis 9 Uhr gewartet, dann laeuft sea_travel():
 *            die Route wird Punkt fuer Punkt auf der Karte gezeichnet, unterwegs koennen
 *            Totenschiff, Krakenmolch oder Piraten auftauchen; nachts wird an Bord
 *            geschlafen. passages_init() wuerfelt Abfahrten, Preise und Schiffstypen aller
 *            Seerouten aus.
 * Aufrufer:  Ortstabelle g_location_handlers (seg025); passages_init() beim Spielstart
 *            (seg120); mod_clock_pos(), sea_travel() und get_srout_len() nur intern.
 * Daten:     g_sea_routes[46] (Seerouten), g_ship_table (Passageart, Preis, Tempo je
 *            Schiffstyp), Zuordnungen zu den Namen in HAFEN.LTX, Schlafbonus je Passageart.
 * Besonders: Bekannt (Bright-Eyes): Prem-Manrin mit 54 km viel zu kurz (Bug 25), Efferds
 *            Schutz hilft nicht gegen Piraten (Bug 34). Schnellsegler-Tempo 150 als
 *            signed char = -106. Der Hafenmeister schimpft ueber fehlendes Geld auch, wenn
 *            man ihn nur verlassen will.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg063.c
 */

#include <stdio.h>
#include <string.h>

#if !defined(__BORLANDC__)
#include <unistd.h>
#endif

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg004.h"
#include "seg007.h"
#include "seg008.h"
#include "seg025.h"
#include "seg026.h"
#include "seg027.h"
#include "seg028.h"
#include "seg029.h"
#include "seg030.h"
#include "seg049.h"
#include "seg059.h"
#include "seg063.h"
#include "seg064.h"
#include "seg075.h"
#include "seg097.h"
#include "seg119.h"

static const signed int g_trv_passage_type_to_hafen_ltx[7] = {
	/* maps PASSAGE_TYPE -> index of its name (Begleitschutzfahrt, Deckpassage etc.) in HAFEN.LTX */
	29, /* PASSAGE_TYPE_HEUER */
	30, /* PASSAGE_TYPE_BEGLEITSCHUTZFAHRT */
	31, /* PASSAGE_TYPE_LUXUSPASSAGE */
	32, /* PASSAGE_TYPE_KOMFORTABLE PASSAGE */
	33, /* PASSAGE_TYPE_KABINENPASSAGE */
	34, /* PASSAGE_TYPE_DECKPASSAGE */
	35  /* PASSAGE_TYPE_MITFAHRGELEGENHEIT */
}; // ds:0x6ec2/*

static const struct ship g_ship_table[8] = {
	{ PASSAGE_TYPE_HEUER,               1, 0,  120 },
	{ PASSAGE_TYPE_KOMFORTABLE_PASSAGE, 1, 35, 100 },
	{ PASSAGE_TYPE_BEGLEITSCHUTZFAHRT,  1,  0, (signed char)150 }, //BUG: base_speed is int8_t, 150 is read as -106
	{ PASSAGE_TYPE_LUXUSPASSAGE,        1, 45, (signed char)150 }, //BUG: base_speed is int8_t, 150 is read as -106
	{ PASSAGE_TYPE_HEUER,               1,  0, 90 },
	{ PASSAGE_TYPE_KABINENPASSAGE,      1, 20, 80 },
	{ PASSAGE_TYPE_DECKPASSAGE,         0, 10, 60 },
	{ PASSAGE_TYPE_MITFAHRGELEGENHEIT,  0,  0, 40 }
}; // ds:0x6ed0

static const signed int g_trv_ship_to_hafen_ltx[8] = {
	/* maps SHIP_TYPE -> index of its name (Langschiff, Karracke etc.) in HAFEN.LTX */
	36, // SHIP_TYPE_LANGSCHIFF_HIGH_SEAS
	37, // SHIP_TYPE_KARRACKE
	38, // SHIP_TYPE_SCHNELLSEGLER
	38, // SHIP_TYPE_SCHNELLSEGLER_LUXURY
	36, // SHIP_TYPE_LANGSCHIFF_COSTAL
	39, // SHIP_TYPE_KUESTENSEGLER
	40, // SHIP_TYPE_KUTTER
	41  // SHIP_TYPE_FISCHERBOOT
}; // ds:0x6ef0

struct sea_route g_sea_routes[SEA_ROUTE_ID__END]; // ds:0x6f00

signed char g_travel_by_ship = 0; // ds:0x7070, 0 = on land, 1 = at the ship

/* sleep bonus on board for each PASSAGE_TYPE, used in do_harbor() (table1) and sea_travel() (table2), identical */
static struct int16_t_7 g_sea_travel_sleepbonus_table1 = {
	-2, // PASSAGE_TYPE_HEUER
	 0, // PASSAGE_TYPE_BEGLEITSCHUTZFAHRT
	 5, // PASSAGE_TYPE_LUXUSPASSAGE
	 4, // PASSAGE_TYPE_KOMFORTABLE_PASSAGE
	 3, // PASSAGE_TYPE_KABINENPASSAGE
	 1, // PASSAGE_TYPE_DECKPASSAGE
	 0  // PASSAGE_TYPE_MITFAHRGELEGENHEIT
}; // ds:0x7071

static struct int16_t_7 g_sea_travel_sleepbonus_table2 = {
	-2, // PASSAGE_TYPE_HEUER
	 0, // PASSAGE_TYPE_BEGLEITSCHUTZFAHRT
	 5, // PASSAGE_TYPE_LUXUSPASSAGE
	 4, // PASSAGE_TYPE_KOMFORTABLE_PASSAGE
	 3, // PASSAGE_TYPE_KABINENPASSAGE
	 1, // PASSAGE_TYPE_DECKPASSAGE
	 0  // PASSAGE_TYPE_MITFAHRGELEGENHEIT
}; // ds:0x707f

static char g_sea_travel_str_t[2] = "T"; // ds:0x708d
static char g_sea_travel_str_en[3] = "EN"; // ds:0x708f
static char g_sea_travel_str_comma[3] = ", "; // ds:0x7092

signed char g_sea_travel_passage_type; // ds:0xe3fa, the PASSAGE_TYPE of the booked ship (index into the sleep bonus tables)

/**
 * \brief   initialize all sea routes: days until the next passage, price modifier (70..130%)
 *          and ship type (high seas: Langschiff/Karracke/Schnellsegler, coastal: Langschiff/
 *          Kuestensegler/Kutter/Fischerboot)
 */
void passages_init(void)
{
	dsa_trace_enter(485, "seg063:passages_init");
	signed int rnd;
	signed int i;
	struct sea_route *route = &g_sea_routes[0];


	for (i = 0; i < (SEA_ROUTE_ID__END - 1); route++, i++) {

		route->passage_timer = (unsigned char)random_interval(0, route->frequency);
		route->price_mod = (unsigned char)random_interval(70, 130);

		rnd = random_schick(100);

		if (!route->coastal_route) {
			/* high seas route */
			route->ship_type = rnd <= 50 ? SHIP_TYPE_LANGSCHIFF_HIGH_SEAS : (rnd <= 80 ? SHIP_TYPE_KARRACKE : (rnd <= 95 ? SHIP_TYPE_SCHNELLSEGLER : SHIP_TYPE_SCHNELLSEGLER_LUXURY));

		} else {

			/* costal route */
			route->ship_type = rnd <= 10 ? SHIP_TYPE_LANGSCHIFF_COSTAL : (rnd <= 40 ? SHIP_TYPE_KUESTENSEGLER : (rnd <= 80 ? SHIP_TYPE_KUTTER : SHIP_TYPE_FISCHERBOOT));
		}

#if !defined(__BORLANDC__)
	D1_LOG("%16s - %16s: %d %d %d %d %d %d\n",
		get_ttx(route->town_id_1 + 0xeb), /* TEXT.LTX 236..287: Ortsname */
		get_ttx(route->town_id_2 + 0xeb), /* TEXT.LTX 236..287: Ortsname */
		route->distance,
		route->frequency,
		route->passage_timer,
		route->coastal_route,
		route->ship_type,
		route->price_mod);
#endif
	}
}

/**
 * \brief   location handler of a harbor
 */
void do_harbor(void)
{
	dsa_trace_enter(486, "seg063:do_harbor");
	signed int i; /* diverse usage */
	signed int done;
	signed int answer;
	struct harbor_option_obsolete *psg_ptr;
	int32_t p_money;
	struct struct_hero *hero;
	signed char first_refresh; /* Swafnild may be met only when the harbor is entered */
	int32_t money;
	struct int16_t_7 sleepbonus;

	done = 0;
	sleepbonus = g_sea_travel_sleepbonus_table1;
	//struct dummy7 a = { { -2, 0, 5, 4, 3, 1, 0 } };

	first_refresh = 1;

	gs_current_signpost_typeindex = gs_town_typeindex;

	draw_loc_icons(4, MENU_ICON_BOOK_SHIP_PASSAGE, MENU_ICON_HARBOR_MASTER, MENU_ICON_BOARD_SHIP, MENU_ICON_LEAVE);
	g_request_refresh = 1;
	gs_travel_detour = 0;

	do {
		if (g_request_refresh) {

			draw_main_screen();
			disable_ani();
			load_ani(6);
			init_ani(0);
			g_request_refresh = 0;

			load_tx(ARCHIVE_FILE_HAFEN_LTX);

			if (first_refresh != 0) {

				first_refresh = 0;

				if ((gs_quested_months > 2) && !gs_treasure_maps[6] &&
					gs_informer_flags[INFORMER_ID_SWAFNILD] && (random_schick(100) <= gs_quested_months + 4))
				{
					/* meet SWAFNILD */
					do_talk(12, 0);

					if (g_swafnild_traveled) {

						g_swafnild_traveled = 0;

						leave_location();

						return;
					}
				}
			}
		}

		handle_gui_input();

		if (g_mouse_rightclick_event || g_action_id == KEY_SCAN_CODE_PAGE_UP) {

			answer = GUI_radio(get_tx(9), 4,get_tx(10), get_tx(11), get_tx(12), get_tx(13)) - 1; /* "HAFEN OPTIONEN" / "PASSAGE BUCHEN" / "HAFENMEISTEREI" / "AN BORD GEHEN" / "HAFEN VERLASSEN" */

			if (answer != -2) {
				g_action_id = answer + ACTION_ID_ICON_1;
			}
		}

		if (g_action_id == ACTION_ID_ICON_1) {
			/* book passage */

			answer = prepare_passages();

			if (answer == 0) {
				GUI_output(get_tx(15)); /* "ES LIEGEN MOMENTAN LEIDER KEINE SCHIFFE IM HAFEN." */
			} else {

				/* select a destination */
				answer = GUI_radio(get_tx(14), (signed char)answer, /* "IM HAFEN LIEGEN SCHIFFE NACH" */
						get_ttx(gs_harbor_options[0].destination + 235), /* TEXT.LTX 236..287: Ortsname */
						get_ttx(gs_harbor_options[1].destination + 235), /* TEXT.LTX 236..287: Ortsname */
						get_ttx(gs_harbor_options[2].destination + 235), /* TEXT.LTX 236..287: Ortsname */
						get_ttx(gs_harbor_options[3].destination + 235), /* TEXT.LTX 236..287: Ortsname */
						get_ttx(gs_harbor_options[4].destination + 235), /* TEXT.LTX 236..287: Ortsname */
						get_ttx(gs_harbor_options[5].destination + 235), /* TEXT.LTX 236..287: Ortsname */
						get_ttx(gs_harbor_options[6].destination + 235), /* TEXT.LTX 236..287: Ortsname */
						get_ttx(gs_harbor_options[7].destination + 235), /* TEXT.LTX 236..287: Ortsname */
						get_ttx(gs_harbor_options[8].destination + 235), /* TEXT.LTX 236..287: Ortsname */
						get_ttx(gs_harbor_options[9].destination + 235)) - 1; /* TEXT.LTX 236..287: Ortsname */

				if (answer != -2) {

					psg_ptr = &gs_harbor_options[answer];

					sprintf(g_dtp2,	get_tx(16), /* "%s %s BIETET EUCH FÜR %s EINE %s NACH %s AN. EINE REISE, DIE BEI@GL..." */

						get_tx(g_trv_ship_to_hafen_ltx[psg_ptr->ship_type]), /* Fischerboot, Schnellsegler etc. */
						psg_ptr->ship_name_ptr,
						(!psg_ptr->ship_timer ? get_tx(5) : get_tx(6)), /* "HEUTE" / "MORGEN" */ /* today or tomorrow */

						get_tx(g_trv_passage_type_to_hafen_ltx[g_ship_table[psg_ptr->ship_type].passage_type]), /* Kabinenpassage etc. */
						get_ttx(psg_ptr->destination + 235), /* TEXT.LTX 236..287: Ortsname */
#ifdef __BORLANDC__
						get_passage_travel_hours(psg_ptr->sea_route_ptr->distance, g_ship_table[psg_ptr->ship_type].base_speed),
#else
						/* when compiled with gcc, occasionally passage times of 0 hours do show up. (which does not happen in the original game!!)
						 * I observed that within the function get_passage_travel_hours(..), computations with negative numbers might happen and lead to this bug.
						 * The following line fixes this. However, it will lead to incompatible binaries when compiled with the original 1992 BCC compiler
						 * This incompatibility of the behavior gcc vs. BCC is a bit scary.
						 * A better understanding is urgently needed... */
						/* REMARK: The base speed should be an unsigned char,
						 * since a speed of 150 would result in a negative speed, which ssomehow happened.
						 * Changing just this type to unsigned would hurt the binary aequivalency.
						 * */
						get_passage_travel_hours(psg_ptr->sea_route_ptr->distance, g_ship_table[psg_ptr->ship_type].base_speed),
#endif
						print_passage_price(g_ship_table[psg_ptr->ship_type].base_price, psg_ptr->sea_route_ptr));

					i = g_textbox_width;
					g_textbox_width = 5;

					answer = GUI_bool(g_dtp2);

					g_textbox_width = i;

					if (answer != 0) {

						money = get_party_money();

						if (gs_sea_travel_passage_price > money) {
							/* party does not have enough money to pay the ship passage */

							GUI_output(get_ttx(401)); /* "SOVIEL GELD HABT IHR DOCH GAR NICHT!" */

						} else {

							g_sea_travel_passage_type = g_ship_table[psg_ptr->ship_type].passage_type;
							money -= gs_sea_travel_passage_price;
							set_party_money(money);

							gs_sea_travel_psgbooked_timer = psg_ptr->ship_timer;
							gs_sea_travel_psgbooked_flag = 0xaa;

							gs_sea_travel_passage_speed1 = gs_sea_travel_passage_speed2; /* speed in [100m per hour] */
							/* Now ..._SPEED1 is the lower byte of ..._SPEED2 */
							/* not clear why two variables ..._SPEED1 and ..._SPEED2 are used. */
							/* In my opinion, a single variable would be enough (and then there would not be the need to copy the value around) */

							gs_current_sea_route_id = psg_ptr->route_id;

							GUI_output(psg_ptr->ship_timer ? get_tx(18) : get_tx(17)); /* "<KOMMT MORGEN VOR 9.00 UHR WIEDER, DA WIR..." / "DER KAPITÄN BEDANKT SICH UND ERINNERT EUCH..." */
							/* ship leaving tomorrow or today */
						}
					}
				}
			}

		} else if (g_action_id == ACTION_ID_ICON_2) {
			/* Hafenmeister */

			if (gs_day_timer <= HOURS(6) || gs_day_timer >= HOURS(21)) {

				GUI_output(get_ttx(568)); /* "GEÖFFNET VON 6.00 UHR BIS 21.00 UHR" */

			} else {

				load_in_head(11);

				do {
					answer = GUI_dialogbox((unsigned char*)g_dtp2, get_tx(4), /* "DER HAFENMEISTER" */
									get_tx(0), 3, /* "<HALLO REISENDE> GRÜSST EUCH EIN MANN, OFFENSICHTLICH DER HAFENMEIS..." */
									get_tx(1), /* Ankommende Schiffe erfragen */
									get_tx(2), /* Saemtliche Routen erfragen */
									get_tx(3)  /* Den Geldhai verlassen */
					);
				} while (answer == -1);

				p_money = get_party_money();

#if FIX_0132
				/* Fix 132: Geld nur pruefen, wenn die Gruppe etwas wissen will (nicht beim Verlassen) */
				if ((answer == 1 || answer == 2) && (p_money < 10L)) {
#else
				if (p_money < 10L) {
#endif

					/* REMARK: also shown if the party only wanted to leave (answer 3) */
					GUI_output(get_tx(8)); /* "<WAS?> BRÜLLT DER HAFENMEISTER ERBÖST.@<IHR HABT NOCH NICHT EINMAL..." */

				} else if (answer == 1 || answer == 2) {
					/* answer == 1: Ankommende Schiffe erfragen (next 2 days)
					 * answer == 2: Saemtliche Routen erfragen */

					p_money -= 10L;
					set_party_money(p_money);

					i = answer;
					answer = get_next_passages(answer);
					/* answer now: number of passages reported by Hafenmeister */

					if (answer != 0) {

						sprintf(g_dtp2,	(i == 1 ? get_tx(28) : get_tx(22)), /* "<IM LAUF DER NÄCHSTEN TAGE KOMM%s %s NACH" / "<VON DIESEM HAFEN AUS FAHREN IN MEHR ODER..." */
							(answer == 1 ? g_sea_travel_str_t : g_sea_travel_str_en),
							(answer == 1 ? get_tx(23) : get_tx(24))); /* "EIN SCHIFF" / "SCHIFFE" */

						i = 0;

						do {

							strcat(g_dtp2, get_ttx(gs_harbor_options[i++].destination + 235)); /* TEXT.LTX 236..287: Ortsname */

							if (--answer) {

								strcat(g_dtp2, (answer >= 2 ? g_sea_travel_str_comma : get_tx(7))); /* "UND NACH" */
							}

						} while (answer);

						strcat(g_dtp2, get_tx(25)); /* ".> MEINT DER HAFENMEISTER FREUNDLICH UND ZIEHT LANGE AN SEINER LANG..." */

						GUI_output(g_dtp2);

					} else {

						GUI_output(get_tx(26)); /* "<DIE NÄCHSTEN ZWEI TAGE WIRD WOHL KEIN SCHIFF KOMMEN!> ANTWORTET DE..." */
						/* "In den naechsten Tagen wird wohl kein Schiff kommen [...]" */
					}
				}
			}

		} else if (g_action_id == ACTION_ID_ICON_3) {
			/* enter booked ship */

			if (gs_sea_travel_psgbooked_flag != 0xaa) {
				/* no ship booked... */

				GUI_output(get_tx(19)); /* "IHR HABT AUF KEINEM DER SCHIFFE ETWAS VERLOREN!" */

			} else if (gs_sea_travel_psgbooked_timer) {

				/* REMARK: cast is important, since var is uint8_t */
				GUI_output((signed char)gs_sea_travel_psgbooked_timer == -1 ?
					get_tx(27) : /* SEA_TRAVEL_PSGBOOKED_TIMER == -1 -> "Zu spaet! Das Schiff, fuer das ihr gebucht wart ist leider ohne euch losgefahren!" */
					get_tx(20)   /* SEA_TRAVEL_PSGBOOKED_TIMER == +1 -> "Die Matrosen lassen euch das Schiff noch nicht besteigen. Kommt morgen wieder..." */
				);

			} else {

				disable_ani();

				GUI_output(get_tx(21)); /* "IHR BEGEBT EUCH AN BORD. <MACHT'S EUCH GEMÜTLICH!> BRUMMELT DER KAP..." */
				/* "Ihr begebt euch an Bord. <Macht's euch gemuetlich!> brummelt der Kapitaen. <Wir legen bald ab!>" */

				g_travel_herokeeping = 1;

				timewarp_until_time_of_day(HOURS(9));

				/* g_check_disease is set to 1 each midnight in timers_daily() */
				if (g_check_disease) {
					/* effectively, this branch is reached if the ship was entered before midnight. */

					disease_effect();
					/* in this call, g_check_disease is reset to 0 */

					hero = get_hero(0);
					for (i = 0; i <= 6; i++, hero++) {

						if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id))
						{
							GRP_hero_sleep(hero, sleepbonus.a[g_sea_travel_passage_type]);

							hero->hunger = hero->thirst = 0;
							/* on the ship, food and drinks are served. */
						}
					}
				}

				g_travel_herokeeping = 0;

				/* REMARK: the NPC (slot 6) is left out, unlike in do_tavern() */
#if FIX_0133
				/* Fix 133: auch den NSC */
				for (i = 0; i <= 6; i++) {
#else
				for (i = 0; i < 6; i++) {
#endif
					g_food_message_shown[i] = 0;
				}

				load_map();

				g_current_ani = -1;
				g_wallclock_update = 0;

				memmove(g_renderbuf_ptr, gs_travel_map_ptr, 64000);
				map_effect(g_renderbuf_ptr);

				wait_for_vsync();

				set_palette(gs_travel_map_ptr + 64002, 0, 0x20);

				mod_clock_pos(gs_town_id);

				set_audio_track(ARCHIVE_FILE_TERMS_XMI);

				g_wallclock_x = g_basepos_x + 120;
				g_wallclock_y = g_basepos_y + 87;
				g_wallclock_update = 1;

				sea_travel(gs_current_sea_route_id, (g_sea_routes[gs_current_sea_route_id].town_id_1 == gs_town_id ? 0 : 1));
				passage_arrival();

				g_wallclock_update = g_basepos_x = g_basepos_y = gs_sea_travel_psgbooked_flag = 0;
				g_current_ani = g_town_loaded_town_id = g_pp20_index = -1;
				g_request_refresh = 1;
				gs_show_travel_map = 0;

				if (!gs_travel_detour) {

					gs_town_id = (signed char)gs_travel_destination_town_id;
					gs_x_bak = gs_travel_destination_x;
					gs_y_bak = gs_travel_destination_y;
					gs_viewdir = ((gs_travel_destination_viewdir + 2) & 3);

#if FIX_0372
				/* Fix 372: nur der Umweg ins Totenschiff fuehrt dorthin; bei 99 (Gruppe tot, Kampf verlassen)
				 * ist schon eine andere Gruppe aktiv, sie bleibt, wo sie ist (wie bei der Landreise, seg093) */
				} else if (gs_travel_detour != DUNGEON_ID_TOTENSCHIFF) {
					done = 1;
#endif
				} else {
					done = 1;
					DNG_enter_dungeon(DUNGEON_ID_TOTENSCHIFF);
				}
			}

		} else if (g_action_id == ACTION_ID_ICON_4) {
			done = 1;
		}

	} while (!done);

	i = load_archive_file(ARCHIVE_FILE_COMPASS);
	read_archive_file(i, g_buffer6_ptr, 5000);
	close(i);

	disable_ani();

	if (!gs_travel_detour) {

		copy_palette();
		leave_location();

#if FIX_0372
	/* Fix 372: bei 99 ist eine andere Gruppe aktiv, ihr Ort bleibt (wie bei der Landreise, seg093) */
	} else if (gs_travel_detour != 99) {
#else
	} else {
#endif

		gs_town_id = TOWN_ID_NONE;
	}
}

/**
 * \brief   move the wall clock on the travel map into the quarter diagonally opposite to the town
 *
 * \param   town_id     ID of the town
 */
void mod_clock_pos(const signed int town_id)
{
	dsa_trace_enter(487, "seg063:mod_clock_pos");
	signed int quarter;
	signed int map_x;
	signed int map_y;

	map_x = g_town_positions[town_id - 1].x;
	map_y = g_town_positions[town_id - 1].y;

	quarter = map_x >= 0 && map_x <= 159 ?
		(map_y >= 0 && map_y <= 99 ? 3 : 1) :
		(map_y >= 0 && map_y <= 99 ? 2 : 0);

	g_basepos_x = ((!quarter || quarter == 2) ? -80 : 80);
	g_basepos_y = ((!quarter || quarter == 1) ? -40 : 40);
}

/**
 * \brief   the sea passage: draws the route on the map step by step, passes the time,
 *          triggers ghost ship, kraken and pirates and lets the heroes sleep on board
 *
 * \param   route_id    index into g_sea_routes (high seas 0..6, coastal 7..44)
 * \param   reverse     0 = from town_id_1 to town_id_2, 1 = the other way
 */
void sea_travel(const signed int route_id, const signed int reverse)
{
	dsa_trace_enter(488, "seg063:sea_travel");
	signed int i;
	struct struct_hero *hero;
	uint8_t *ptr;
	int32_t off;

	struct int16_t_7 sleepbonus = g_sea_travel_sleepbonus_table2;
	//struct dummy7 a = { { -2, 0, 5, 4, 3, 1, 0 } };

	g_traveling = 1;

	gs_sea_travel_courses = (uint8_t*)(route_id < 7 ? (g_buffer9_ptr + 7600L) : (g_buffer9_ptr + 11400L));

	/* high seas routes have id 0..6, costal routes id 7..44 */

	/* if high seas route, write 7 (total number of high seas routes)
	 * if costal route, write 38 (total number of costal routes) */
	gs_travel_mousehover = (route_id < 7 ? 7 : 38);

	/* convert costal route ids to range 0..37 */
	gs_sea_travel_passage_id = route_id < 7 ? route_id : route_id - 7;

	off = ((int32_t*)gs_sea_travel_courses)[gs_sea_travel_passage_id];
	gs_travel_course_ptr = (int16_t*)(gs_sea_travel_courses + off + 4 * gs_travel_mousehover);
	ptr = g_vga_memstart;

	gs_travel_course_ptr += 2;

	memset(g_trv_track_pixel_bak, 0xaa, 500);
	gs_travel_speed = 10 * gs_sea_travel_passage_speed1; /* speed [unit: 10m per hour] */
	gs_travel_total_steps = get_srout_len((struct struct_point*)gs_travel_course_ptr); /* a step for each pixel on the map. */
	gs_travel_total_distance = 100 * g_sea_routes[route_id].distance; /* length of sea route [unit: 10m] */
	gs_travel_total_duration = gs_travel_total_distance / gs_travel_speed * 60; /* duration [unit: minutes] */
	gs_travel_duration_per_step = gs_travel_total_duration / gs_travel_total_steps; /* duration of each step [unit: minutes] */
	gs_travel_distance_per_step = gs_travel_total_distance / gs_travel_total_steps; /* length of a single step [unit: 10m] */

	if (gs_travel_distance_per_step == 0) {
		gs_travel_distance_per_step = 1;
	}
#if !defined(__BORLANDC__)
	D1_INFO("Schiffspassage gestartet. Entfernung: %d0 Schritt. Geschwindigkeit: %d0 Schritt/h. Dauer (lt. Hafen): %d min. Dauer (real): %d min.\n", gs_travel_total_distance, gs_travel_speed, gs_travel_total_duration,gs_travel_total_steps*2*(gs_travel_duration_per_step/2));
	D1_INFO_VERBOSE("#Pixel = %d, Entfernung/Pixel: %d0 Schritt, Dauer/Pixel: %d min\n", gs_travel_total_steps, gs_travel_distance_per_step, gs_travel_duration_per_step);
#endif

	if (reverse) {
		/* for reverse direction, point gs_travel_course_ptr to end of route */

		while (gs_travel_course_ptr[0] != -1) {
			gs_travel_course_ptr += 2;
		}

		gs_travel_course_ptr -= 2;
	}

	gs_travel_course_start = gs_travel_course_ptr;

	gs_travel_distance_per_18_hours = (18 * (gs_travel_speed + gs_travel_speed / 10));
	/* this is 19.8h * gs_travel_speed, which is the distance [unit: 10m] the ship travels in 19.8 h.
	 * I (siebenstreich) think that the original idea was 18h * gs_travel_speed, copying the
	 * corresponding part from trv_do_journey, and then messing up the removal of speed_mod */

#if FIX_PROBENLOG
	/* Probenlog (Art 9, Typ 6..8): Zufallswuerfe bei Beginn einer Seereise (W100 <= Schwelle).
	 * Werte: Typ (6 = Totenschiff, 7 = Krakenmolch, 8 = Piraten), W100, Schwelle, Ereignis (1/0). Gleiche Wuerfe wie im Original. */
	{
		signed int w[4];

		if (route_id <= 6 && gs_quest_deadship && !gs_quest_deadship_done) {
			w[0] = 6;
			w[1] = random_schick(100);
			w[2] = 20;
			w[3] = gs_passage_deadship_flag = (w[1] <= 20 ? 1 : 0);
			if (w[3]) {
				gs_passage_deadship_position = random_schick(gs_travel_distance_per_18_hours);
			}
			fix_probenlog(9, NULL, 4, w);
		} else {
			gs_passage_deadship_flag = 0;
		}

		w[0] = 7;
		w[1] = random_schick(100);
		w[2] = 5;
		w[3] = gs_passage_octopus_flag = (w[1] <= 5 ? 1 : 0);
		if (w[3]) {
			gs_passage_octopus_position = random_schick(gs_travel_distance_per_18_hours);
		}
		fix_probenlog(9, NULL, 4, w);

		w[0] = 8;
		w[1] = random_schick(100);
		w[2] = 10;
		w[3] = gs_passage_pirates_flag = (w[1] <= 10 ? 1 : 0);
		if (w[3]) {
			gs_passage_pirates_position = random_schick(gs_travel_distance_per_18_hours);
		}
		fix_probenlog(9, NULL, 4, w);
	}
#else
	if (route_id <= 6 && gs_quest_deadship && !gs_quest_deadship_done) {
		/* only on high seas routes */

		if ((gs_passage_deadship_flag = random_schick(100) <= 20 ? 1 : 0)) {
			gs_passage_deadship_position = random_schick(gs_travel_distance_per_18_hours);
#if !defined(__BORLANDC__)
			D1_INFO("Totenschiff wurde bei %o0 Schritt aktiviert!\n", gs_passage_deadship_position);
#endif
		}
	} else {
		gs_passage_deadship_flag = 0;
	}

	if ((gs_passage_octopus_flag = random_schick(100) <= 5 ? 1 : 0)) {

		gs_passage_octopus_position = random_schick(gs_travel_distance_per_18_hours);
#if !defined(__BORLANDC__)
		D1_INFO("Krakenmolch wurde bei %o0 Schritt aktiviert!\n", gs_passage_octopus_position);
#endif
	}

	if ((gs_passage_pirates_flag = random_schick(100) <= 10 ? 1 : 0)) {

		gs_passage_pirates_position = random_schick(gs_travel_distance_per_18_hours);
#if !defined(__BORLANDC__)
		D1_INFO("Piratenangriff wurde bei %o0 Schritt aktiviert!\n", gs_passage_pirates_position);
#endif
	}
#endif

	/* from now on gs_travel_distance_per_18_hours is the distance travelled so far, like gs_travel_distance_made */
	gs_travel_step_counter = gs_travel_distance_made = gs_travel_distance_per_18_hours = gs_travel_detour = 0;
	g_travel_herokeeping = 1;

	while (gs_travel_course_ptr[(gs_travel_mousehover = 0)] != -1 && !gs_travel_detour)
	{

		if (is_mouse_in_rect(gs_travel_course_ptr[0] - 16, gs_travel_course_ptr[1] - 16,
					gs_travel_course_ptr[0] + 16, gs_travel_course_ptr[1] + 16))
		{
			call_mouse_bg();
			gs_travel_mousehover = 1;
		}

		g_trv_track_pixel_bak[gs_travel_step_counter] =
			*(ptr + gs_travel_course_ptr[1] * 320 + gs_travel_course_ptr[0]);

		gs_travel_step_counter++;

		*(ptr + gs_travel_course_ptr[1] * 320 + gs_travel_course_ptr[0]) = 0x1f;

		if (gs_travel_mousehover) {
			call_mouse();
		}

		/* the following loop will be executed Floor(gs_travel_duration_per_step / 2) times.
		 * therefore, 2 * Floor(gs_travel_duration_per_step / 2) minutes ingame times will pass. */
		for (gs_trv_i = 0; gs_travel_duration_per_step / 2 > gs_trv_i; gs_trv_i++) {

			handle_input();

			wait_for_vsync();

			timewarp(MINUTES(2));
		}

		gs_travel_distance_made += gs_travel_distance_per_step;
		gs_travel_distance_per_18_hours += gs_travel_distance_per_step;

#if !defined(__BORLANDC__)
		D1_LOG("%d0 Schritt zurueckgelegt.\n",gs_travel_distance_per_18_hours);
#endif

		if (gs_passage_deadship_flag != 0 && gs_travel_distance_per_18_hours >= gs_passage_deadship_position && !gs_quest_deadship_done) {

			/* within the call prolog_ghostship(), the party can decide if they enter the Totenschiff.
			 * In that case, gs_travel_detour is set to DUNGEON_ID_TOTENSCHIFF (instead of 0) */
			prolog_ghostship();
			gs_passage_deadship_flag = 0;

		} else if (gs_passage_octopus_flag != 0 && gs_travel_distance_per_18_hours >= gs_passage_octopus_position && !gs_ingame_timers[INGAME_TIMER_EFFERD_SAFE_PASSAGE]) {

			octopus_attack_wrapper();
			gs_passage_octopus_flag = 0;

		} else if
#ifndef M302de_ORIGINAL_BUGFIX
			/* Original-Bug 34:
			 * There is an Efferd miracle with the text "Efferd gewaehrt euch seinen Schutz auf Wasser.".
			 * For sea traveling, it prevents octopus encounters. However, pirate encounters are still possible, which feels wrong. */
			(gs_passage_pirates_flag != 0 && gs_travel_distance_per_18_hours >= gs_passage_pirates_position)
#else
			(gs_passage_pirates_flag != 0 && gs_travel_distance_per_18_hours >= gs_passage_pirates_position && !gs_ingame_timers[INGAME_TIMER_EFFERD_SAFE_PASSAGE])
#endif
		{
			pirates_attack_wrapper();
			gs_passage_pirates_flag = 0;
		}

		/* This looks dirty.
		 * g_check_disease is set to 1 each midnight in timers_daily()
		 * Usually, the disease_effect() function is called within game_loop(). But the game does not jump back to that function at this point,
		 * which is probably the reason to replicate the disease_effect() call here.
		 * Why the dependence on g_check_party? */
		if (g_check_disease && !g_check_party) {

			disease_effect();

			g_travel_by_ship = 1;

			hero = get_hero(0);
			for (i = 0; i <= 6; i++, hero++) {

				if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id))
				{
					GRP_hero_sleep(hero, sleepbonus.a[g_sea_travel_passage_type]);
					hero->hunger = hero->thirst = 0;
				}
			}

			g_travel_by_ship = 0;
		}

		if (g_request_refresh && !gs_travel_detour) {

			call_mouse_bg();

			load_map();

			/* TODO: update window */
			memmove((void*)g_vga_memstart, (void*)gs_travel_map_ptr, 320 * 200);

			wait_for_vsync();

			set_palette(gs_travel_map_ptr + 64002, 0, 0x20);

			set_audio_track(ARCHIVE_FILE_TERMS_XMI);

			gs_trv_i = 0;

			for (gs_travel_course_ptr2 = gs_travel_course_start;
					g_trv_track_pixel_bak[gs_trv_i++] != 0xaa;
					gs_travel_course_ptr2 += (!reverse ? 2 : -2))
			{
				*(ptr + gs_travel_course_ptr2[1] * 320 + gs_travel_course_ptr2[0]) = 0x1f;
			}

			call_mouse();

			g_wallclock_x = g_basepos_x + 120;
			g_wallclock_y = g_basepos_y + 87;
			g_wallclock_update = 1;
			g_request_refresh = 0;
		}

		gs_travel_course_ptr += (!reverse ? 2 : -2);
	}

	g_travel_herokeeping = 0;

	if (!gs_travel_detour) {

		call_mouse_bg();

		do {
			if (!reverse) {
				gs_travel_course_ptr -= 2;
			} else {
				gs_travel_course_ptr += 2;
			}
			gs_travel_step_counter--;

			*(ptr + gs_travel_course_ptr[1] * 320 + gs_travel_course_ptr[0]) =
				g_trv_track_pixel_bak[gs_travel_step_counter];

#if defined(__BORLANDC__)
		} while (gs_travel_course_ptr[0] != -1);
#else
		} while ((gs_travel_course_ptr[0] != -1) && (gs_travel_step_counter != 0));
#endif

		call_mouse();
	}

	g_traveling = 0;
}

/**
 * \brief   number of points of a route on the travel map (terminated by x == -1)
 *
 * \param   ptr         first point of the route
 * \return              number of points = number of steps
 */
signed int get_srout_len(struct struct_point *ptr)
{
	dsa_trace_enter(489, "seg063:get_srout_len");
	signed int i = 0;

	while (ptr->x != -1) {
		ptr++;
		i++;
	}

	return i;
}
