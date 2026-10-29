/*
 * seg052 - Lager in Stadt oder Dungeon
 *
 * Inhalt:    Lagermenü für das Rasten in einer Stadt oder einem Dungeon: Wachen
 *            einteilen, Talent oder Magie anwenden, eine wählbare Zahl Stunden
 *            schlafen. Je Stunde steigt die Gefahr eines Zwischenfalls: im
 *            Dungeon ein Kampf, in der Stadt ein Diebstahl.
 * Aufrufer:  do_area_camp über do_location (seg025).
 * Daten:     vier Dungeon-Lagerkämpfe, Status je Held (Wache, Magie) und die
 *            drei Wachen.
 * Besonders: Original-Bug 26: nach dem Verlassen ist die Gruppe um 180 Grad
 *            gedreht (leave_location). Bei weniger als 3 Stunden Schlaf ist die
 *            Wachdauer 0, die Wachen wechseln nie.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg052.c
 */
#include <stdio.h>

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
#include "seg096.h"
#include "seg097.h"
#include "seg098.h"
#include "seg103.h"
#include "seg105.h"

static const signed int g_dcampfights[4] = {
	FIGHT_ID_DCAMPFIGHT1,
	FIGHT_ID_DCAMPFIGHT2,
	FIGHT_ID_DCAMPFIGHT3,
	FIGHT_ID_DCAMPFIGHT4
}; // ds:0x66e2

signed char g_area_camp_guards[3];	// ds:0xe3de
signed char g_area_camp_magicstatus[7];	// ds:0xe3e1
signed char g_area_camp_guardstatus[8];	// ds:0xe3e8

/**
 * \brief   camp in a town or a dungeon
 */
void do_area_camp(void)
{
	signed int l_si;
	signed int watch;
	signed int done;
	signed int answer;
	signed int redraw_locline;
	struct struct_hero* hero;
	signed char hours;
	signed int watch_hours;
	signed int watch_hours_left;
	signed int hours_left;
	signed char hours_slept; /* counted, but never read */

	done = 0;

	redraw_locline = g_request_refresh = 1;

	for (l_si = 0; l_si <= 6; l_si++) {
		g_area_camp_magicstatus[l_si] = g_area_camp_guardstatus[l_si] = 0;
	}

	for (l_si = 0; l_si < 3; l_si++) {
		g_area_camp_guards[l_si] = -1;
	}

	draw_loc_icons(5, MENU_ICON_GUARDS, MENU_ICON_APPLY_TALENT, MENU_ICON_MAGIC, MENU_ICON_SLEEP, MENU_ICON_LEAVE);

	while (done == 0) {

		if (g_request_refresh != 0) {
			draw_main_screen();
			disable_ani();
			load_ani(36);
			init_ani(0);
			GUI_print_loc_line(get_ttx(306)); /* "LAGER" */
			set_audio_track(ARCHIVE_FILE_CAMP_XMI);
			g_request_refresh = redraw_locline = 0;
		}

		if (redraw_locline != 0) {
			GUI_print_loc_line(get_ttx(306)); /* "LAGER" */
			redraw_locline = 0;
		}

		handle_gui_input();

		if (g_mouse_rightclick_event || g_action_id == KEY_SCAN_CODE_PAGE_UP) {

			/* open area_camp radio menu */
			answer = GUI_radio(get_ttx(307), 5, /* "LAGEROPTIONEN" */
						get_ttx(308), get_ttx(212), /* "WACHE EINTEILEN" / "TALENT ANWENDEN" */
						get_ttx(310), get_ttx(316), /* "MAGIE ANWENDEN" / "SCHLAFEN" */
						get_ttx(471)) - 1; /* "VERLASSEN" */

			/* set action on a valid answer */
			if (answer != -2) {
				g_action_id = answer + ACTION_ID_ICON_1;
			}
		}

		if (g_action_id == ACTION_ID_ICON_1) {

			answer = -1;

			for (l_si = 0; l_si <= 6; l_si++) {

				if (!g_area_camp_magicstatus[l_si] && check_hero(get_hero(l_si))) {
					g_area_camp_guardstatus[l_si] = 0;
					answer = 0;
				}
			}

			if (answer == -1) {
				GUI_output(get_ttx(332)); /* "KEIN MITGLIED DER GRUPPE IST MEHR IN DER LAGE, WACHE ZU HALTEN!" */
			} else {

				for (l_si = 0; l_si < 3; l_si++) {

					sprintf(g_dtp2,	get_ttx(321), l_si + 1); /* "WER SOLL DIE %d.WACHE ÜBERNEHMEN?" */

					do {

						answer = select_hero_ok(g_dtp2);

						if (answer != -1 && g_area_camp_magicstatus[answer] != 0) {
							GUI_output(get_ttx(331)); /* "DU KANNST DOCH AUCH KEINE ZWEI DINGE GLEICHZEITIG MACHEN!" */
							answer = -1;
						}

						if (answer != -1 && get_hero(answer)->flags.brewing) {
							GUI_output(get_ttx(730)); /* "WÄHREND EINES ALCHIMISTISCHEN VERSUCHS GEHT DAS NICHT!" */
							answer = -1;
						}

					} while (answer == -1);

					g_area_camp_guardstatus[answer]++;
					g_area_camp_guards[l_si] = answer;
				}
			}

		} else if (g_action_id == ACTION_ID_ICON_2) {

			GUI_use_talent2(0, get_ttx(395)); /* "WER SOLL EIN TALENT ANWENDEN?" */

		} else if (g_action_id == ACTION_ID_ICON_3) {

			answer = select_hero_ok(get_ttx(317)); /* "WELCHER HELD SOLL MAGIE ANWENDEN?" */

			if (answer != -1 && get_hero(answer)->flags.brewing) {

				GUI_output(get_ttx(730)); /* "WÄHREND EINES ALCHIMISTISCHEN VERSUCHS GEHT DAS NICHT!" */
				answer = -1;
			}

			if (answer != -1) {

				hero = get_hero(answer);

				if (hero->typus >= HERO_TYPE_HEXE) {

					if (g_area_camp_guardstatus[answer]) {

						GUI_output(get_ttx(331)); /* "DU KANNST DOCH AUCH KEINE ZWEI DINGE GLEICHZEITIG MACHEN!" */

					} else {
						if (g_area_camp_magicstatus[answer]) {

							GUI_output(get_ttx(334)); /* "DIE LETZTE MAGIEANWENDUNG HAT SCHON MEHR ALS GENUG AN DER KONSTITUT..." */

						} else {
							g_area_camp_magicstatus[answer] = use_magic(hero);
						}
					}
				} else {
					GUI_output(get_ttx(330)); /* "DIESE OPTION IST AUSSCHLIESSLICH MAGIEKUNDIGEN VORBEHALTEN!" */
				}
			}
		} else if (g_action_id == ACTION_ID_ICON_4) {

			if (GUI_bool(get_ttx(318))) { /* "WIRKLICH SCHLAFEN?@KEINE WEITEREN OPTIONEN MEHR MÖGLICH!" */

				hours = GUI_input(get_ttx(833), 1); /* "WIE VIELE STUNDEN WOLLT IHR SCHLAFEN?" */

				if (hours > 0) {

					watch_hours = hours / 3;
#if FIX_0130
					/* Fix 130: mindestens 1 Stunde je Wache, sonst wechselt die Wache nie */
					if (watch_hours == 0) {
						watch_hours = 1;
					}
#endif
					watch_hours_left = watch_hours;
					watch = 0;

					if (gs_camp_incident == -1) {
						/* with guards: (hours - 1) % chance for an incident */
						/* without guards: (4*hours - 1) % chance for an incident */
						/* For a 1 hour rest with guards this will be 0% chance! */
						/* TODO: maybe change it to random_schick(100) - 1 to fix that */
						if ((g_area_camp_guards[0] == -1 ? 4 * hours : hours) > random_schick(100)) {
							gs_camp_incident = random_schick(3) - 1;
						}
					}

					hours_slept = 0;
					hours_left = hours;

					if (g_area_camp_guards[watch] != -1) {

						sprintf(g_dtp2,	get_ttx(774), get_hero(g_area_camp_guards[watch])->alias); /* "%s HÄLT WACHE" */

						GUI_print_loc_line(g_dtp2);
					}

					do {
						g_food_mod = 1;
						timewarp(HOURS(1));
						g_food_mod = 0;

						watch_hours_left--;
						hours_slept++;
						hours_left--;

						if (watch == gs_camp_incident && (watch_hours / 2) >= watch_hours_left) {
							done = 1;
						}

						if (watch_hours_left == 0 && watch < 2) {

							watch_hours_left = watch_hours;
							watch++;

							if (g_area_camp_guards[watch] != -1) {

								sprintf(g_dtp2,	get_ttx(774), get_hero(g_area_camp_guards[watch])->alias); /* "%s HÄLT WACHE" */

								GUI_print_loc_line(g_dtp2);
							}
						}

					} while (hours_left > 0 && done == 0);

					if (done != 0) {

#if FIX_0313
						/* Fix 313: im Dungeon erst nach dem Kampf loeschen, FIG_init_heroes() weckt die Wache */
						if (g_area_camp_area_type != AREA_TYPE_DUNGEON)
#endif
						gs_camp_incident = -1;

						if (g_area_camp_area_type == AREA_TYPE_DUNGEON) {
							/* in a dungeon */

							g_fig_initiative = 1;
							g_fig_discard = 1;

							do_fight(g_dcampfights[random_schick(4) - 1]);
#if FIX_0313
							gs_camp_incident = -1;
#endif

							if (g_game_state == GAME_STATE_MAIN) {
								draw_main_screen();
								disable_ani();
								load_ani(36);
								init_ani(0);
								GUI_print_loc_line(get_ttx(306)); /* "LAGER" */
								set_audio_track(ARCHIVE_FILE_CAMP_XMI);
								g_request_refresh = redraw_locline = 0;
							}

						} else {
							/* in a town */
							done = 0;
							lose_random_item(get_hero(get_random_hero()), 100, get_ttx(832)); /* "OH VERDAMMT! %s HAT DEN DIEB NICHT GESEHEN, ABER %s WURDE GESTOHLEN!" */
						}

						if (hours_left > 0) {
							g_food_mod = 1;
							timewarp(HOURS(hours_left));
							g_food_mod = 0;
						}
					}

					if (done == 0) {

						hero = get_hero(0);
						for (l_si = 0; l_si <= 6; l_si++, hero++) {

							if ((hero->typus != HERO_TYPE_NONE) &&
								(hero->group_id == gs_active_group_id) &&
								(g_area_camp_guardstatus[l_si] < 2) &&
								(g_area_camp_magicstatus[l_si] != 1))
							{
								GRP_hero_sleep(hero, hours - 10); /* the sleep quality depends on the duration: 8 hours give -2 */
							}
						}
					}
				}
				done = 1;
			}

		} else if (g_action_id == ACTION_ID_ICON_5) {
			done = 1;
		}
	}

	gs_town_loc_type_bak = LOCTYPE_NONE;

	/* Original-Bug 26: After leaving a camp in town/dungeon-mode, the party is rotated by 180 degrees. This does not make sense. */
	leave_location();
#if FIX_0047
	/* Fix 47 (teilweise): Drehung nach dem Lager zuruecknehmen (Bright-Eyes-Fix zu Original-Bug 26)
	 * The rotation is performed in the function leave_location().
	 * We fix the bug in a hacky way by simply correcting the rotation afterwards. */
	gs_viewdir = REVERSE_VIEWDIR(gs_viewdir); /* rotate by 180 degrees */
#endif

}
