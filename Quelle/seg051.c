/*
 * seg051 - Lager in der Wildnis
 *
 * Inhalt:    Lagermenü bei Reisen: Wachen einteilen (3 Wachen), Vorräte
 *            auffüllen (Wildnisleben für Wasser, Fährtensuchen für Nahrung),
 *            Talent oder Magie anwenden, Kräuter suchen (Pflanzenkunde) und
 *            schlafen. Während der Nacht kann ein zufälliger Überfall (Kampf)
 *            oder ein vorher festgelegtes Ereignis eintreten.
 * Aufrufer:  do_wildcamp über do_location (seg025); gather_herbs und
 *            replenish_stocks auch aus Reiseereignissen (seg094, seg109).
 * Daten:     Kräutertabelle (Chance, Höchstzahl, Probenaufschlag), Lagerkämpfe;
 *            Status je Held (Wache, Magie, Jagd, Kräuter) und die drei Wachen,
 *            auch in seg049 (Heldentausch) und seg109 benutzt.
 * Besonders: Original-Bug (Wache einteilen): bei ESC wird g_wildcamp_herbstatus[-1]
 *            = g_wildcamp_guards[2] gelesen; da dieser -1 ist, erscheint "DU KANNST
 *            DOCH AUCH KEINE ZWEI DINGE GLEICHZEITIG MACHEN!". Ohne guten Lagerplatz
 *            (g_good_camp_place == 0) fehlt der Menüpunkt "WEITERREISEN".
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg051.c
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
#include "seg029.h"
#include "seg032.h"
#include "seg047.h"
#include "seg049.h"
#include "seg051.h"
#include "seg096.h"
#include "seg097.h"
#include "seg098.h"
#include "seg103.h"
#include "seg104.h"
#include "seg105.h"

static const signed int g_campfights[4] = {
	FIGHT_ID_CAMPFIGHT1,
	FIGHT_ID_CAMPFIGHT2,
	FIGHT_ID_CAMPFIGHT3,
	FIGHT_ID_CAMPFIGHT4
}; // ds:0x6694

struct gather_herbs g_gather_herbs_table[13] = {
	{ ITEM_ID_ALRAUNE,                  5, 1, 15 },
	{ ITEM_ID_BELMART_BLATT,           12, 3,  8 },
	{ ITEM_ID_DONFSTENGEL,             10, 2,  9 },
	{ ITEM_ID_GULMOND_BLATT,           15, 3,  8 },
	{ ITEM_ID_JORUGAWURZEL,            15, 1, 10 },
	{ ITEM_ID_KAIRANHALM,               1, 1, 18 },
	{ ITEM_ID_SHURINKNOLLE,            12, 2,  8 },
	{ ITEM_ID_TARNELE,                 45, 5,  4 },
	{ ITEM_ID_THONNYSBLUETE,            3, 1, 15 },
	{ ITEM_ID_EINBEERE,                40, 4,  8 },
	{ ITEM_ID_WIRSELKRAUT,             30, 2,  4 },
	{ ITEM_ID_EITRIGER_KROETENSCHEMEL, 20, 2,  4 },
	{ 0xff,                             0, 0,  0 }
}; // ds:0x669c
int8_t g_gather_herbs_special = -1; 			// ds:0x66d0	/* REMARK: should be signed int */ /* item ID of a herb with better chances at this place, -1 = none */
static char g_gather_herbs_str_found[6] = "%d^%s";	// ds:0x66d1
static char g_gather_herbs_str_comma[3] = ", ";		// ds:0x66d7
static char g_gather_herbs_str_and[6] = " UND ";	// ds:0x66da
static char g_gather_herbs_str_dot[2] = ".";		// ds:0x66e0


signed char g_wildcamp_guards[3];	// ds:0xe3be
signed char g_wildcamp_herbstatus[7];	// ds:0xe3c1
signed char g_wildcamp_replstatus[7];	// ds:0xe3c8
signed char g_wildcamp_magicstatus[7];	// ds:0xe3cf
signed char g_wildcamp_guardstatus[8];	// ds:0xe3d6

/**
 * \brief   the camp menu in the wilderness
 *
 *          g_good_camp_place: 0 = no "WEITERREISEN" in the menu,
 *          1 = rest place of a route (seg109), 99 = camp on the road (seg094):
 *          then replenish_stocks and gather_herbs get a handicap of +99.
 */
void do_wildcamp(void)
{
	signed int watch;
	signed int redraw_locline;
	signed int i;
	signed int done = 0;
	signed int answer;
	signed char stock_tries = 0;
	signed char herb_tries = 0;
	signed char herb_hours;
	struct struct_hero* hero;
	signed char sleep_hours;
	signed int watch_hours;
	signed int watch_hours_left;
	signed int hours_left;
	signed char incident_preset; /* 1: gs_camp_incident was set before by an event (seg110, seg111, seg116) */
	signed char hours_slept; /* counted, but never read */

	redraw_locline = g_request_refresh = 1;

	for (i = 0; i <= 6; i++) {

		g_wildcamp_herbstatus[i] = g_wildcamp_replstatus[i] =
			g_wildcamp_magicstatus[i] = g_wildcamp_guardstatus[i] = 0;
	}

	for (i = 0; i < 3; i++) {
		g_wildcamp_guards[i] = -1;
	}

	i = !g_good_camp_place ? 6 : 7;
	draw_loc_icons(i, MENU_ICON_GUARDS, MENU_ICON_REPLENISH_SUPPLIES, MENU_ICON_APPLY_TALENT, MENU_ICON_MAGIC, MENU_ICON_GATHER_HERBS, MENU_ICON_SLEEP, MENU_ICON_QUIT_CAMP);

	while (done == 0) {

		if (g_request_refresh) {
			draw_main_screen();
			disable_ani();
			load_ani(2);
			init_ani(0);
			GUI_print_loc_line(get_ttx(306)); /* "LAGER" */
			set_audio_track(ARCHIVE_FILE_CAMP_XMI);
			g_request_refresh = redraw_locline = 0;
		}

		if (redraw_locline) {
			GUI_print_loc_line(get_ttx(306)); /* "LAGER" */
			redraw_locline = 0;
		}

		handle_gui_input();

		if (g_mouse_rightclick_event || g_action_id == KEY_SCAN_CODE_PAGE_UP) {

			i = !g_good_camp_place ? 6 : 7;

			answer = GUI_radio(get_ttx(307), (signed char)i, /* "LAGEROPTIONEN" */
						get_ttx(308), get_ttx(309), /* "WACHE EINTEILEN" / "VORRÄTE AUFFÜLLEN" */
						get_ttx(212), get_ttx(310), /* "TALENT ANWENDEN" / "MAGIE ANWENDEN" */
						get_ttx(315), get_ttx(316), /* "KRÄUTER SUCHEN" / "SCHLAFEN" */
						get_ttx(814)) -1; /* "WEITERREISEN" */

			if (answer != -2) {
				g_action_id = answer + ACTION_ID_ICON_1;
			}
		}

		if (g_action_id == ACTION_ID_ICON_1) {

			answer = -1;

			for (i = 0; i <= 6; i++) {

				if (!g_wildcamp_magicstatus[i] && !g_wildcamp_herbstatus[i] &&
					!g_wildcamp_replstatus[i] && is_hero_available_in_group(get_hero(i)))
				{
					g_wildcamp_guardstatus[i] = 0;
					answer = 0;
				}
			}

			if (answer == -1) {
				GUI_output(get_ttx(332)); /* "KEIN MITGLIED DER GRUPPE IST MEHR IN DER LAGE, WACHE ZU HALTEN!" */
			} else {

				for (i = 0; i < 3; i++) {

					sprintf(g_dtp2, get_ttx(321), i + 1); /* "WER SOLL DIE %d.WACHE ÜBERNEHMEN?" */

					do {
						answer = select_hero_ok(g_dtp2);

						/* Original-Bug: not checked answer for following options */
						/* With answer == -1 (ESC) g_wildcamp_herbstatus[-1] == g_wildcamp_guards[2] (still -1)
						 * is read, so the message below appears. The selection cannot be cancelled anyway. */
						if ((answer != -1 &&
							g_wildcamp_magicstatus[answer] != 0) ||
							g_wildcamp_herbstatus[answer] != 0 ||
							g_wildcamp_replstatus[answer] != 0)
						{
							GUI_output(get_ttx(331)); /* "DU KANNST DOCH AUCH KEINE ZWEI DINGE GLEICHZEITIG MACHEN!" */
							answer = -1;
						}

						if (answer != -1) {
							if (get_hero(answer)->flags.brewing) {
								GUI_output(get_ttx(730)); /* "WÄHREND EINES ALCHIMISTISCHEN VERSUCHS GEHT DAS NICHT!" */
								answer = -1;
							}
						}

					} while (answer == -1);

					g_wildcamp_guardstatus[answer]++;
					g_wildcamp_guards[i] = answer;
				}
			}

		} else if (g_action_id == ACTION_ID_ICON_2) {

			if (g_good_camp_place == 99) {
				redraw_locline = replenish_stocks(g_replenish_stocks_mod + 99, stock_tries);
			} else {
				redraw_locline = replenish_stocks(g_replenish_stocks_mod, stock_tries);
			}

			if (redraw_locline) {
				stock_tries++;
			}

		} else if (g_action_id == ACTION_ID_ICON_3) {

			GUI_use_talent2(0, get_ttx(395)); /* "WER SOLL EIN TALENT ANWENDEN?" */

		} else if (g_action_id == ACTION_ID_ICON_4) {

			answer = select_hero_ok(get_ttx(317)); /* "WELCHER HELD SOLL MAGIE ANWENDEN?" */

			if (answer != -1) {

				if (get_hero(answer)->flags.brewing) {
					GUI_output(get_ttx(730)); /* "WÄHREND EINES ALCHIMISTISCHEN VERSUCHS GEHT DAS NICHT!" */
					answer = -1;
				}
			}

			if (answer != -1) {

				hero = get_hero(answer);

				if (hero->typus >= HERO_TYPE_HEXE) {

					if (g_wildcamp_guardstatus[answer] != 0 ||
						g_wildcamp_herbstatus[answer] != 0 ||
						g_wildcamp_replstatus[answer] != 0)
					{
						GUI_output(get_ttx(331)); /* "DU KANNST DOCH AUCH KEINE ZWEI DINGE GLEICHZEITIG MACHEN!" */

					} else {

						if (g_wildcamp_magicstatus[answer] != 0) {

							GUI_output(get_ttx(334)); /* "DIE LETZTE MAGIEANWENDUNG HAT SCHON MEHR ALS GENUG AN DER KONSTITUT..." */

						} else {
							g_wildcamp_magicstatus[answer] = use_magic(hero);
						}
					}
				} else {
					GUI_output(get_ttx(330)); /* "DIESE OPTION IST AUSSCHLIESSLICH MAGIEKUNDIGEN VORBEHALTEN!" */
				}
			}

		} else if (g_action_id == ACTION_ID_ICON_5) {
			/* COLLECT HERBS */

			g_talented_hero_pos = get_talented_hero_pos(TA_PFLANZENKUNDE);

			answer = select_hero_ok(get_ttx(326)); /* "WER SOLL AUF KRÄUTERSUCHE GEHEN?" */

			if (answer != -1 && get_hero(answer)->flags.brewing) {
				GUI_output(get_ttx(730)); /* "WÄHREND EINES ALCHIMISTISCHEN VERSUCHS GEHT DAS NICHT!" */
				answer = -1;
			}

			if (answer != -1) {

				if (g_wildcamp_herbstatus[answer])
				{
					sprintf(g_dtp2,	get_ttx(803), get_hero(answer)->alias); /* "%s WAR SCHON KRÄUTER SUCHEN." */
					GUI_output(g_dtp2);

				} else if (g_wildcamp_guardstatus[answer] != 0 ||
						g_wildcamp_replstatus[answer] != 0 ||
						g_wildcamp_magicstatus[answer] != 0)
				{
					GUI_output(get_ttx(331)); /* "DU KANNST DOCH AUCH KEINE ZWEI DINGE GLEICHZEITIG MACHEN!" */

				} else {

					if (herb_tries < 1)
					{
						hero = get_hero(answer);

						herb_hours = GUI_input(get_ttx(327), 1); /* "WIE VIELE STUNDEN SOLL GESUCHT WERDEN?" */

						if (herb_hours > 0)
						{
							g_wildcamp_herbstatus[answer] = herb_tries = (signed char)(redraw_locline = 1);

							if (g_good_camp_place == 99) {
								gather_herbs(hero, herb_hours - 1, g_gather_herbs_mod + 99);
							} else {
								gather_herbs(hero, herb_hours - 1, g_gather_herbs_mod);
							}
						}
					} else {
						GUI_output(get_ttx(336)); /* "DIE GEGEND IST VOLLSTÄNDIG ABGEGRAST!" */
					}
				}
			}
		} else if (g_action_id == ACTION_ID_ICON_6) {
			/* Sleep */

			if (GUI_bool(get_ttx(318))) { /* "WIRKLICH SCHLAFEN?@KEINE WEITEREN OPTIONEN MEHR MÖGLICH!" */

				sleep_hours = gs_day_timer / HOURS(1);

				sleep_hours = gs_day_timer < HOURS(8) ? 8 - sleep_hours : 24 - sleep_hours + 8;

				watch_hours = sleep_hours / 3;
				watch_hours_left = watch_hours;
				watch = 0;
				incident_preset = 0;

				if (gs_camp_incident == -1) {

					if (((g_wildcamp_guards[0] == -1 ? 60 : 10) > random_schick(100)) && !gs_ingame_timers[INGAME_TIMER_TRAVIA_SAFE_REST]) /* 59 % without guards, 9 % with guards */
					{
						gs_camp_incident = random_schick(3) - 1; /* the watch (0..2) of the incident */
					}
				} else {
					incident_preset = 1;
				}

				hours_slept = 0;
				hours_left = sleep_hours;

				if (g_wildcamp_guards[watch] != -1) {

					sprintf(g_dtp2, get_ttx(774), get_hero(g_wildcamp_guards[watch])->alias); /* "%s HÄLT WACHE" */

					GUI_print_loc_line(g_dtp2);
				}

				do {

					g_food_mod = 1;
					timewarp(HOURS(1));
					g_food_mod = 0;
					watch_hours_left--;
					hours_slept++;
					hours_left--;

					if ((watch == gs_camp_incident) && (watch_hours / 2 >= watch_hours_left)) {
						done = 1;
					}

					if (watch_hours_left == 0 && watch < 2) {

						watch_hours_left = watch_hours;
						watch++;

						if (g_wildcamp_guards[watch] != -1) {

							sprintf(g_dtp2,	get_ttx(774), get_hero(g_wildcamp_guards[watch])->alias); /* "%s HÄLT WACHE" */
							GUI_print_loc_line(g_dtp2);
						}
					}

				} while (hours_left > 0 && done == 0);

				if (done == 0) {

					hero = get_hero(0);

					for (i = 0; i <= 6; i++, hero++) {

						if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
							g_wildcamp_guardstatus[i] < 2 && g_wildcamp_magicstatus[i] != 1) /* no regeneration for a hero with two or more watches or after using magic */
						{
							GRP_hero_sleep(hero, g_wildcamp_sleep_quality);
						}
					}

				} else if (!incident_preset) {

					gs_camp_incident = -1;
					g_fig_initiative = 1;
					g_fig_discard = 1;

					/* pick a random campfight out of 4 possibilities */
					do_fight(g_campfights[random_schick(4) - 1]);

					if (gs_travel_detour != 99 && g_game_state == GAME_STATE_MAIN) {

						draw_main_screen();
						disable_ani();
						load_ani(2);
						init_ani(0);
						GUI_print_loc_line(get_ttx(306)); /* "LAGER" */
						set_audio_track(ARCHIVE_FILE_CAMP_XMI);

						g_request_refresh = redraw_locline = 0;

						if (hours_left > 0) {
							g_food_mod = 1;
							timewarp_until_time_of_day(HOURS(8));
							g_food_mod = 0;
						}
					}
				} else {
					/* event incident: replace the watch by the hero on guard (or -1), evaluated by the event */
					gs_camp_incident = g_wildcamp_guards[gs_camp_incident];
				}

				done = 1;
			}
		} else if (g_action_id == ACTION_ID_ICON_7) {
			done = 1;
		}
	}

	for (i = 0; i <= 6; i++) {
		g_wildcamp_herbstatus[i] = g_wildcamp_replstatus[i] = 0;
	}

	leave_location();
}

/**
 * \brief   a hero searches for herbs
 *
 *          For every herb of the table: chance roll, then a Pflanzenkunde test
 *          (handicap of the herb - hours + handicap). Takes hours + 1 hours.
 *
 * \param   hero        the hero
 * \param   hours       number of hours - 1 (more time makes the test easier)
 * \param   handicap    additional handicap of the place
 * \return              always 0
 */
signed int gather_herbs(struct struct_hero *hero, const signed int hours, const signed int handicap)
{
	signed int herb_index;
	signed int unique_herbs_count;
	struct gather_herbs *ptr;
	signed char herb_count[12];

	memset(herb_count, 0 , 12);

	timewarp(HOURS(hours + 1));

	ptr = &g_gather_herbs_table[0];

	for (unique_herbs_count = herb_index = 0; herb_index < 12; herb_index++, ptr++) {

		/* check if this is a special place for collecting the considered herb.. */
		if (ptr->item_id == g_gather_herbs_special) {

			/* dirty code follows. The original herbs table is modified. */
			ptr->chance_max += 10; // 10% higher chance to find the herb
			ptr->max_count++;  // increase maximum count of single herbs by 1.
		}

		if ((random_schick(100) <= ptr->chance_max) &&
			test_talent(hero, TA_PFLANZENKUNDE, ptr->handicap - hours + handicap) > 0) {

			herb_count[herb_index] = give_new_item_to_hero(hero, ptr->item_id, 0, random_schick(ptr->max_count)); // collect a random amount between 1 and max_count herbs.

			if (herb_count[herb_index]) {
				unique_herbs_count++;
			}
		}

		if (ptr->item_id == g_gather_herbs_special) {

			/* The herbs table is reverted to original state. */
			ptr->chance_max -= 10;
			ptr->max_count--;
		}
	}

	if (unique_herbs_count) {

		/* print a sentence with all the herb names */
		sprintf(g_dtp2,	get_ttx(328), hero->alias); /* "%s FINDET" */

		for (herb_index = 0; herb_index < 12; herb_index++) {

			if (herb_count[herb_index]) {

				sprintf(g_text_output_buf, g_gather_herbs_str_found, herb_count[herb_index],
					(char*)GUI_name_inflect_with_article((herb_count[herb_index] > 1 ? INFLECT_PLURAL : INFLECT_SINGULAR)
						+ (INFLECT_OMIT_ARTICLE | INFLECT_4TH_CASE),
						g_gather_herbs_table[herb_index].item_id, INFLECT_NAME_TYPE_ITEM
					)
				);

				strcat(g_dtp2, g_text_output_buf);

				if (--unique_herbs_count > 1) {

					/* add a comma ", " */
					strcat(g_dtp2, g_gather_herbs_str_comma);

				} else if (unique_herbs_count == 1) {

					/* add an and " UND " */
					strcat(g_dtp2, g_gather_herbs_str_and);
				}
			}
		}

		/* add a dot "." */
		strcat(g_dtp2, g_gather_herbs_str_dot);

	} else {

		/* no herbs found */
		sprintf(g_dtp2, get_ttx(342), hero->alias); /* "%s ENTDECKT LEIDER KEINE KRÄUTER." */
	}

	GUI_output(g_dtp2);

	return 0;
}

/**
 * \brief   replenish the stocks (water and food)
 *
 * \param   handicap    handicap for the talent test
 * \param   tries       how often was tried to replenish stocks
 * \return              0 if replenish was not possible or 1 if replenish was possible
 */
signed int replenish_stocks(signed int handicap, const signed int tries)
{
	signed int hero_pos;
	signed int i;
	signed int retval = 0;
	signed int j;
	struct struct_hero* hero;
	struct struct_hero* hero2;

	handicap += 5;

	g_talented_hero_pos = get_talented_hero_pos(TA_WILDNISLEBEN);
	hero_pos = select_hero_ok(get_ttx(322)); /* "WER SOLL VERSUCHEN DIE VORRÄTE AUFZUFÜLLEN?" */

	if (hero_pos != -1 && get_hero(hero_pos)->flags.brewing) {

		GUI_output(get_ttx(730)); /* "WÄHREND EINES ALCHIMISTISCHEN VERSUCHS GEHT DAS NICHT!" */
		hero_pos = -1;
	}

	if (hero_pos != -1) {

		if (g_wildcamp_replstatus[hero_pos] != 0) {

			sprintf(g_dtp2, get_ttx(802), get_hero(hero_pos)->alias); /* "%s WAR SCHON AUF DER JAGD." */
			GUI_output(g_dtp2);

		} else {

			if (g_wildcamp_herbstatus[hero_pos] != 0 ||
				g_wildcamp_magicstatus[hero_pos] != 0 ||
				g_wildcamp_guardstatus[hero_pos] != 0)
			{
				GUI_output(get_ttx(331)); /* "DU KANNST DOCH AUCH KEINE ZWEI DINGE GLEICHZEITIG MACHEN!" */

			} else {

				if (tries < 2) {

					timewarp(HOURS(1));
					gs_main_acting_hero = hero = get_hero(hero_pos);
					g_wildcamp_replstatus[hero_pos] = 1;
					retval = 1;

					/* search for water */
					if ((test_talent(hero, TA_WILDNISLEBEN, handicap) > 0) || gs_ingame_timers[INGAME_TIMER_EFFERD_FIND_WATER]) {

						/* found water */
						sprintf(g_dtp2, get_ttx(324), hero->alias); /* "%s FINDET WASSER!" */

						/* fill up all waterskins and remove thirst of all living heroes in the current group */
						hero2 = get_hero(0);

						for (i = 0; i <= 6; i++, hero2++) {

							if ((hero2->typus != HERO_TYPE_NONE) && (hero2->group_id == gs_active_group_id) &&
								!hero2->flags.dead)
							{
								hero2->thirst = 0;

								for (j = 0; j < NR_HERO_INVENTORY_SLOTS; j++) {

									if (hero2->inventory[j].item_id == ITEM_ID_WASSERSCHLAUCH) {

										hero2->inventory[j].flags.empty = 0;
										hero2->inventory[j].flags.half_empty = 0;
									}
								}
							}
						}
					} else {

						sprintf(g_dtp2, get_ttx(340), hero->alias); /* "%s FINDET NIRGENDS WASSER!" */
					}

					GUI_print_loc_line(g_dtp2);
					vsync_or_key(200);

					/* search for food */
					if ((test_talent(hero, TA_FAEHRTENSUCHEN, handicap) > 0) || gs_ingame_timers[INGAME_TIMER_FIRUN_HUNT]) {

						/* remove hunger of all living heroes in the current group */
						hero2 = get_hero(0);
						for (i = 0; i <= 6; i++, hero2++) {

							if ((hero2->typus != HERO_TYPE_NONE) && (hero2->group_id == gs_active_group_id) &&
								!hero2->flags.dead)
							{
								hero2->hunger = 0;
							}
						}

						/* the group may get three food packages */
						if (!give_new_item_to_group(ITEM_ID_PROVIANTPAKET, 1, 3)) {
							strcpy(g_dtp2, get_ttx(306)); /* "LAGER" */
							g_request_refresh = 1;
						} else {
							sprintf(g_dtp2,	get_ttx(325), hero->alias); /* "%s KANN ETWAS ESSBARES ERLEGEN!" */
						}

					} else {

						sprintf(g_dtp2, get_ttx(341), hero->alias); /* "%s KANN KEINE TIERE ENTDECKEN!" */
					}

					GUI_print_loc_line(g_dtp2);
					vsync_or_key(200);

				} else {
					GUI_output(get_ttx(323)); /* "FÜR MEHR ALS 2 VERSUCHE IST KEINE ZEIT!" */
				}
			}
		}
	}

	return retval;
}
