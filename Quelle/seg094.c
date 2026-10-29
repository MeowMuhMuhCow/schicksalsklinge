/*
 * seg094 - Reisemodus: eine Landreise auf der Karte (trv_do_journey) und Hilfsfunktionen
 *
 * Inhalt:    trv_do_journey() fuehrt die Gruppe Schritt fuer Schritt ueber einen Landweg
 *            (Pixelspur auf der Karte), laesst die Zeit laufen und loest an den Meilenpunkten die
 *            Reiseereignisse aus (TRV_event, seg109), dazu Zufallsbegegnung, Wegkampf und das
 *            Treffen mit Olvir/Treborn. Rechtsklick bietet Gewaltmarsch und Lager, ab 20 Uhr wird ein
 *            Nachtlager erzwungen, danach kann umgekehrt werden. Weitere Funktionen bestimmen Ankunfts-
 *            feld und Blickrichtung im Zielort, laden die Kartenmarken und zeichnen die Vorstrecke der
 *            Querverbindung (Crosslink Peilinen/Rovamund <-> Kravik/Skelellen).
 * Aufrufer:  do_travel_mode() (seg093). Die Kartenmarken-Variablen (g_current_town_*, g_trv_menu_selection)
 *            liest die Markenanimation in seg002; g_traveling werten seg002, seg032 und seg063 aus.
 * Daten:     g_tevents_repeatable (je Ereignis: innerhalb einer Reise wiederholbar),
 *            g_journey_tevent_relative_position, Koordinaten der Kartenmarken, g_traveling.
 * Besonders: Original-Bug 50-52 (Richtungswechsel und Ereignisse). Gewaltmarsch: Tempo = 197 plus
 *            bisherige Schrittzahl (vermutlich Fehler). TM_draw_track() stellt die Pixel beim
 *            Loeschen um einen Schritt versetzt wieder her. TM_unused_continue_journey()/TM_unused2() werden nie
 *            aufgerufen.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg094.c
 */

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
#include "seg028.h"
#include "seg094.h"
#include "seg097.h"
#include "seg109.h"
#include "seg117.h"
#include "seg119.h"

signed char g_traveling = 0; // ds:0xa842

/* A list of flags, indicating if a tevent is repeatable *within the same journey* or not.
 * Note that tevents marked as non-repeatable can be triggered again in the following journeys (i.e., after reaching a town).
 */
static const signed char g_tevents_repeatable[145] = {
	1, // tevent_001
	0, // tevent_002
	1, // tevent_003
	1, // tevent_004
	1, // tevent_005
	1, // tevent_006
	0, // tevent_007
	1, // tevent_008
	1, // tevent_009
	1, // tevent_010
	1, // tevent_011
	1, // tevent_012
	1, // tevent_013
	1, // tevent_014
	1, // tevent_015
	1, // tevent_016
	1, // tevent_017
	1, // tevent_018
	1, // tevent_019
	1, // tevent_020
	1, // tevent_021
	1, // tevent_022
	1, // tevent_023
	1, // tevent_024
	1, // tevent_025
	1, // tevent_026
	1, // tevent_027
	1, // tevent_028
	1, // tevent_029
	1, // tevent_030
	0, // tevent_031
	1, // tevent_032
	0, // tevent_033
	1, // tevent_034
	1, // tevent_035
	1, // tevent_036
	0, // tevent_037
	1, // tevent_038
	1, // tevent_039
	1, // tevent_040
	0, // tevent_041
	1, // tevent_042
	1, // tevent_043
	0, // tevent_044
	0, // tevent_045
	1, // tevent_046
	0, // tevent_047
	1, // tevent_048
	1, // tevent_049
	1, // tevent_050
	1, // tevent_051
	0, // tevent_052
	1, // tevent_053
	1, // tevent_054
	1, // tevent_055
	1, // tevent_056
	1, // tevent_057
	1, // tevent_058
	0, // tevent_059
	1, // tevent_060
	1, // tevent_061
	1, // tevent_062
	1, // tevent_063
	1, // tevent_064
	0, // tevent_065
	0, // tevent_066
	1, // tevent_067
	1, // tevent_068
	1, // tevent_069
	1, // tevent_070
	1, // tevent_071
	1, // tevent_072
	1, // tevent_073
	1, // tevent_074
	0, // tevent_075
	1, // tevent_076
	0, // tevent_077
	1, // tevent_078
	1, // tevent_079
	1, // tevent_080
	1, // tevent_081
	1, // tevent_082
	1, // tevent_083
	1, // tevent_084
	1, // tevent_085
	1, // tevent_086
	1, // tevent_087
	1, // tevent_088
	1, // tevent_089
	0, // tevent_090
	1, // tevent_091
	1, // tevent_092
	1, // tevent_093
	1, // tevent_094
	1, // tevent_095
	1, // tevent_096
	1, // tevent_097
	1, // tevent_098
	0, // tevent_099
	1, // tevent_100
	1, // tevent_101
	1, // tevent_102
	1, // tevent_103
	1, // tevent_104
	1, // tevent_105
	1, // tevent_106
	1, // tevent_107
	1, // tevent_108
	1, // tevent_109
	1, // tevent_110
	1, // tevent_111
	1, // tevent_112
	1, // tevent_113
	1, // tevent_114
	1, // tevent_115
	1, // tevent_116
	0, // tevent_117
	1, // tevent_118
	1, // tevent_119
	1, // tevent_120
	1, // tevent_121
	0, // tevent_122
	1, // tevent_123
	1, // tevent_124
	0, // tevent_125
	1, // tevent_126
	0, // tevent_127
	1, // tevent_128
	1, // tevent_129
	0, // tevent_130
	1, // tevent_131
	1, // tevent_132
	1, // tevent_133
	1, // tevent_134
	1, // tevent_135
	1, // tevent_136
	1, // tevent_137
	1, // tevent_138
	1, // tevent_139
	1, // tevent_140
	1, // tevent_141
	1, // tevent_142
	1, // tevent_143
	1, // tevent_144
	0  // tevent_145
}; // ds:0xa843

unsigned char g_journey_crosslink_status;	// ds:0xe4a2
/* according to enum CROSSLINK_STATUS_...: 0, 1 = from Kravik, 2 = from Peilinen, 3 = from Skelellen, 4 = from Rovamund */

signed int g_trv_menu_selection;		// ds:0xe4a3
signed int g_current_town_over;			// ds:0xe4a5
signed int g_current_town_overy;		// ds:0xe4a7
signed int g_current_town_overx;		// ds:0xe4a9
signed int g_selected_town_aniy;		// ds:0xe4ab
signed int g_selected_town_anix;		// ds:0xe4ad
signed int g_current_town_aniy;			// ds:0xe4af
signed int g_current_town_anix;			// ds:0xe4b1
unsigned char g_unkn_091[1];			// ds:0xe4b3 (nirgends benutzt)
unsigned char g_trv_crosslink_pixel_bak[20]; // ds:0xe4b4 (Kartenpixel unter der Vorstrecke zur Querverbindung, s. TM_draw_track())
unsigned char g_good_camp_place;		// ds:0xe4c8

unsigned char g_journey_tevent_relative_position[15];	// ds:0xe4c9
/* The position of the tevents relative to the party, viewed from the starting point of the journey. */

enum {
	TEVENT_POSITION_AHEAD = 0,
	TEVENT_POSITION_REMOVED = 1,
	TEVENT_POSITION_BEHIND = 2
};

/**
 * \brief   laedt OBJECTS.NVF und entpackt die 10 Kartenmarken nach g_buffer6_ptr + 1000;
 *          setzt danach Marke und Textfensterlage fuer den aktuellen Ort
 */
void prepare_map_marker(void)
{
	signed int i;
	signed int handle;
	signed int size_unused; /* nimmt Breite und Hoehe der Bilder auf, wird nicht gebraucht */
	struct nvf_extract_desc nvf;

	/* load OBJECTS.NVF */
	handle = load_archive_file(ARCHIVE_FILE_OBJECTS_NVF);

	read_archive_file(handle, g_buffer6_ptr, 2000);

	close(handle);

	for (i = 0; i < 10; i++) {
		nvf.dst = g_buffer6_ptr + 100 * i + 1000;
		nvf.src = g_buffer6_ptr;
		nvf.image_num = i;
		nvf.compression_type = 0;
		nvf.width = &size_unused;
		nvf.height = &size_unused;

		process_nvf_extraction(&nvf);
	}

	set_town_marker_and_textbox(gs_town_id);
}

/**
 * \brief   setzt die Marke des aktuellen Orts (g_current_town_anix/aniy) und legt das Textfenster
 *          (g_basepos_x/y) in das dem Ort gegenueberliegende Kartenviertel
 *
 * \param   town_id     Ort (TOWN_ID_...)
 */
void set_town_marker_and_textbox(const signed int town_id)
{
	signed int x;
	signed int quadrant; /* 0 = rechts unten, 1 = links unten, 2 = rechts oben, 3 = links oben */
	signed int y;

	/* zero some global variables */
	g_current_town_over = g_trv_menu_selection = g_selected_town_anix = g_selected_town_aniy = 0;


	x = g_current_town_anix = g_town_positions[town_id - 1].x;
	y = g_current_town_aniy = *(signed int*)((uint8_t*)g_town_positions + 4 * (town_id - 1) + 2); /* = g_town_positions[town_id - 1].y */

	quadrant = (x >= 0 && x <= 159) ? (y >= 0 && y <= 99 ? 3 : 1) : (y >= 0 && y <= 99 ? 2 : 0);

	g_basepos_x = ((!quadrant || quadrant == 2) ? -80 : 80);
	g_basepos_y = ((!quadrant || quadrant == 1) ? -40 : 40);
}

/**
 * \brief   Travel along a land route
 *
 * \param   land_route_id    number of the route
 * \param   reverse          0 = route is traveled in normal direction, i.e. from land_route_id.town_id_1 to land_route_id.town_id_2
 *                           1 = route is traveled in reverse direction, i.e. from land_route_id.town_id_2 to land_route_id.town_id_1
 */
void trv_do_journey(const signed int land_route_id, const signed int reverse)
{
	uint8_t* fb_start;
	struct struct_hero *hero;
	struct struct_journey_tevent *tevent_ptr;
	signed int bak1;
	signed int bak2;
#ifndef M302de_ORIGINAL_BUGFIX
	/* After fixing Original-Bug 50, 51, and 52, this is not needed any more. */
	signed int last_tevent_id;
#endif
	signed int answer;

	g_traveling = 1;

#ifndef M302de_ORIGINAL_BUGFIX
	/* After fixing Original-Bug 50, 51, and 52, this is not needed any more. */
	last_tevent_id = -1;
#endif

	gs_travel_course_ptr = (int16_t*)((g_buffer9_ptr + *(int16_t*)((uint8_t*)g_buffer9_ptr + 4 * (land_route_id - 1))) + 0xecL);
	fb_start = g_vga_memstart;
	gs_travel_course_ptr += 2;

	memset((void*)g_trv_track_pixel_bak, 0xaa, 500);
	/* TODO: move this pointer out of the game state, verify if that works correctly.
	 * 		Can be replaced by a locvar! */
	gs_travel_route_ptr = &g_land_routes[land_route_id - 1];
	gs_travel_speed = 166; // unit: [10m per hour]. So standard speed is 1.66 km/h, pretty slow...
	gs_travel_total_steps = TM_get_track_length((struct struct_point*)gs_travel_course_ptr);
	gs_travel_total_distance = (gs_travel_route_ptr->distance * 100); // unit: [10m]
	gs_travel_total_duration = (gs_travel_total_distance / (gs_travel_speed + gs_travel_route_ptr->speed_mod * gs_travel_speed / 10) * 60); // unit: [minutes]
	gs_travel_duration_per_step = (gs_travel_total_duration / gs_travel_total_steps); /* duration of each step. unit: [minutes] */
	gs_travel_distance_per_step = gs_travel_total_distance / gs_travel_total_steps; /* length of a single step. unit: [10m] */

	if (gs_travel_distance_per_step == 0)
	{
		gs_travel_distance_per_step = 1;
	}

	if (reverse)
	{
		/* move gs_travel_course_ptr to the end of the route */
		while (gs_travel_course_ptr[0] != -1)
		{
			gs_travel_course_ptr += 2;
		}
		gs_travel_course_ptr -= 2;
	}

	memset((void*)gs_journey_tevents, (gs_travel_step_counter = 0), 15 * sizeof(struct struct_journey_tevent)); // gs_travel_step_counter is used in a different meaning
	memset(g_journey_tevent_relative_position, TEVENT_POSITION_AHEAD, 15);

	/* TODO: move this pointer out of the game state, verify if that works correctly.
	 * 		Can be replaced by a locvar! */
	gs_tevents_tab_ptr = &g_tevents_tab[0];
	/* Forward pointer to entries associated with current route. */
	while (((unsigned char)gs_tevents_tab_ptr->land_route_id != land_route_id) && (gs_tevents_tab_ptr->land_route_id != -1))
	{
		gs_tevents_tab_ptr++;
	}

	gs_journey_direction = JOURNEY_DIRECTION_FORWARD;
	gs_travel_course_start = gs_travel_course_ptr;
	gs_travel_distance_per_18_hours = (gs_travel_speed + gs_travel_route_ptr->speed_mod * gs_travel_speed / 10) * 18; /* distance after 18 hours of traveling. unit: [10m] */

	/* random section starts */
	if (gs_quested_months > 3)
	{
		if ((gs_journey_olvir_treborn_flag = (random_schick(100) <= 2) ? 1 : 0))
		{
			gs_journey_olvir_treborn_position = random_schick(gs_travel_distance_per_18_hours); /* unit: [10m] */
		}
	} else {
		gs_journey_olvir_treborn_flag = 0;
	}

	if ((gs_journey_random_encounter_flag = random_schick(100) <= gs_travel_route_ptr->encounters ? 1 : 0))
	{
		gs_journey_random_encounter_position = random_schick(gs_travel_distance_per_18_hours); /* unit: [10m] */
	}

	if ((gs_journey_fight_flag = (random_schick(100) <= gs_travel_route_ptr->fights / 3 ? 1 : 0)))
	{
		gs_journey_fight_position = random_schick(gs_travel_distance_per_18_hours); /* unit: [10m] */
	}

	gs_travel_distance_per_18_hours = 0;
	/* random section ends */

	while ((gs_tevents_tab_ptr->land_route_id != -1) && ((unsigned char)gs_tevents_tab_ptr->land_route_id == land_route_id)) // first condition can be dropped
	{
		tevent_ptr = &gs_journey_tevents[gs_travel_step_counter];
		tevent_ptr->position = gs_tevents_tab_ptr->position;
		tevent_ptr->tevent_id = gs_tevents_tab_ptr->tevent_id;

		if (reverse)
		{
			tevent_ptr->position = gs_travel_route_ptr->distance - tevent_ptr->position;
		}

		tevent_ptr->position *= 100; // unit is now [10m]
		gs_tevents_tab_ptr++;
		gs_travel_step_counter++;
	}

	gs_travel_step_counter = gs_travel_distance_made = gs_travel_detour = 0; // now gs_travel_step_counter has its normal meaning

	while (gs_travel_course_ptr[(gs_travel_mousehover = 0)] != -1 &&
		!gs_travel_detour &&
		g_game_state == GAME_STATE_MAIN)
	{
		if (is_mouse_in_rect(gs_travel_course_ptr[0] - 16, gs_travel_course_ptr[1] - 16,
					gs_travel_course_ptr[0] + 16, gs_travel_course_ptr[1] + 16))
		{
			call_mouse_bg();
			gs_travel_mousehover = 1;
		}

		if (gs_journey_direction == JOURNEY_DIRECTION_BACKWARD)
		{
			gs_travel_step_counter--;

			/* restore the pixel from the map */
			*(fb_start + gs_travel_course_ptr[1] * 320 + gs_travel_course_ptr[0]) =
				g_trv_track_pixel_bak[gs_travel_step_counter];

			g_trv_track_pixel_bak[gs_travel_step_counter] = 0xaa;
		} else {
			/* save the old pixel from the map */
			g_trv_track_pixel_bak[gs_travel_step_counter] =
				*(fb_start + gs_travel_course_ptr[1] * 320 + gs_travel_course_ptr[0]);

			gs_travel_step_counter++;

			/* write a new pixel */
			*(fb_start + gs_travel_course_ptr[1] * 320 + gs_travel_course_ptr[0]) = 0x1c;
		}

		if (gs_travel_mousehover) {
			call_mouse();
		}

		gs_trv_i = 0;
		while (gs_travel_duration_per_step / 2 > gs_trv_i)
		{
			handle_input();

			if (gs_trv_i % 2)
			{
				wait_for_vsync();
			}

			timewarp(MINUTES(2));

			gs_trv_i++;
		}

		gs_travel_distance_made += (gs_journey_direction == JOURNEY_DIRECTION_BACKWARD ? -gs_travel_distance_per_step : gs_travel_distance_per_step);
		gs_travel_distance_per_18_hours += gs_travel_distance_per_step;

		if (g_mouse_rightclick_event || g_action_id == KEY_SCAN_CODE_PAGE_UP)
		{
			if (!gs_forcedmarch_timer)
			{
				answer = GUI_radio(get_ttx(815), 3, get_tx(74), get_ttx(816), get_ttx(613)); /* "WAS WOLLT IHR TUN?" / "GEWALTMARSCH" / "KAMPIEREN" / "ZURÜCK" */
			} else {
				answer = GUI_radio(get_ttx(815), 2, get_ttx(816), get_ttx(613)); /* "WAS WOLLT IHR TUN?" / "KAMPIEREN" / "ZURÜCK" */
			}

			if (answer == 1 && !gs_forcedmarch_timer) /* Gewaltmarsch */
			{
				/* do forced march for 2 days */
				gs_journey_forced_march_le_cost = random_schick(10);

				gs_travel_speed = gs_travel_step_counter + 197;
				/* 197 is 1.97 km/h, still pretty slow.
				 * But WHY "+ gs_travel_step_counter"?? Original-Bug? */
				/* Vermutlich ja: das Tempo des Gewaltmarschs haengt davon ab, wie viele Schritte
				 * dieses Wegs schon gegangen sind (spaet gestartet = sehr schnell). */

				gs_forcedmarch_timer = 2;
				gs_travel_total_duration = (gs_travel_total_distance / (gs_travel_speed + (gs_travel_route_ptr->speed_mod * gs_travel_speed) / 10) * 60); // unit: [minutes]
				gs_travel_duration_per_step = (gs_travel_total_duration / gs_travel_total_steps); /* duration of each step. unit: [minutes] */

				/* The following looks like an a posteriori balancing adjustment */
				/* Remark: gs_journey_forced_march_le_cost = gs_journey_forced_march_le_cost / 2; */
				gs_journey_forced_march_le_cost >>= 1;

				hero = get_hero(0);
				for (gs_trv_i = 0; gs_trv_i <= 6; gs_trv_i++, hero++)
				{
					if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
						!hero->flags.dead)
					{
						/* This is kind of brutal and close to a bug:
						 * Switch to forced march, and a weak hero could be dead immediately.
						 * A much cleaner solution would be to gradually reduce the LE one-by-one, and stop the forced march if a hero is low on LE. */
						sub_hero_le(hero, gs_journey_forced_march_le_cost);
					}
				}

			} else if ((answer == 1 && gs_forcedmarch_timer != 0) ||
					(answer == 2 && !gs_forcedmarch_timer)) /* Kampieren */
				// Depending on gs_forcedmarch_timer, the "Kampieren" answer is on position 1 or 2.
			{
				/* make gather_herbs and replenish_stocks practically impossible */
				g_wildcamp_sleep_quality = -3;
				g_good_camp_place = 99;
				gs_town_loc_type = LOCTYPE_WILDCAMP;

				do_location();

				g_good_camp_place = gs_town_loc_type = g_wildcamp_sleep_quality = 0;
				g_wallclock_update = 0;
				g_request_refresh = 2;
			}
		}

		if (gs_journey_random_encounter_flag && gs_travel_distance_per_18_hours >= gs_journey_random_encounter_position && g_game_state == GAME_STATE_MAIN)
		{
			journey_random_encounter(land_route_id);
			gs_journey_random_encounter_flag = 0;

		} else if (gs_journey_fight_flag && gs_travel_distance_per_18_hours >= gs_journey_fight_position && g_game_state == GAME_STATE_MAIN)
		{
			do_wild8_fight();

		} else if (gs_journey_olvir_treborn_flag && gs_travel_distance_per_18_hours >= gs_journey_olvir_treborn_position && g_game_state == GAME_STATE_MAIN)
		{
			gs_town_typeindex = (random_schick(100) <= 50 ? INFORMER_ID_OLVIR + 1 : INFORMER_ID_TREBORN + 1);
			bak1 = g_basepos_x;
			bak2 = g_basepos_y;
			g_basepos_x = g_basepos_y = 0;
			do_informer();
			g_basepos_x = bak1;
			g_basepos_y = bak2;

		}

		if (g_game_state == GAME_STATE_MAIN)
		{
			for (gs_trv_i = 0; gs_trv_i < 15; gs_trv_i++)
			{
				if (((gs_journey_tevents[gs_trv_i].position <= gs_travel_distance_made) &&
					(gs_journey_direction == JOURNEY_DIRECTION_FORWARD) &&
					!g_journey_tevent_relative_position[gs_trv_i])
						||
					((gs_journey_tevents[gs_trv_i].position >= gs_travel_distance_made) &&
					(gs_journey_direction == JOURNEY_DIRECTION_BACKWARD) &&
					(g_journey_tevent_relative_position[gs_trv_i] == TEVENT_POSITION_BEHIND)))
				{
					if (gs_journey_tevents[gs_trv_i].tevent_id)
					{
#ifndef M302de_ORIGINAL_BUGFIX
						TRV_event(gs_journey_tevents[(last_tevent_id = gs_trv_i)].tevent_id);
#else

						/* Fix Original-Bug 50, 51, 52.
						 * Set the correct relative position of the last tevent. */
						TRV_event(gs_journey_tevents[gs_trv_i].tevent_id);

						if (gs_journey_direction != JOURNEY_DIRECTION_CHANGE_TO_FORWARD && gs_journey_direction != JOURNEY_DIRECTION_CHANGE_TO_BACKWARD) {
							/* travel event handler did not change the traveling direction.
							 * Hence, the event was settled and the group passed its position. */
#endif
							if (!g_tevents_repeatable[gs_journey_tevents[gs_trv_i].tevent_id - 1])
							{
								/* tevent is "used up" for this journey. */
								g_journey_tevent_relative_position[gs_trv_i] = TEVENT_POSITION_REMOVED;

							} else if (gs_journey_direction == JOURNEY_DIRECTION_FORWARD)
							{
								/* traveling forward => now the event can only be triggered moving in backward direction. */
								g_journey_tevent_relative_position[gs_trv_i] = TEVENT_POSITION_BEHIND;
							} else {
								/* traveling backward => now the event can only be triggered moving in forward direction. */
								g_journey_tevent_relative_position[gs_trv_i] = TEVENT_POSITION_AHEAD;
							}
#ifdef M302de_ORIGINAL_BUGFIX
						}
							/* Otherwise, the travel event handler wants to reverse the traveling direction.
							 * This means that the event was not really settled
							 * (so we don't remove it even when it is non-repeatable)
							 * and, moreover, the group has not passed the event position. */
#endif

						if (g_request_refresh != 0 && !gs_travel_detour)
						{
							g_wallclock_update = 0;
						}
					}
				}
			}
		}

	        /* night camp */
		if (gs_day_timer >= HOURS(20) && !gs_travel_detour && g_game_state == GAME_STATE_MAIN &&
			2 * gs_travel_distance_per_step < gs_travel_distance_made &&
			gs_travel_total_distance - 2 * gs_travel_distance_per_step > gs_travel_distance_made)
		{
			g_wallclock_update = 0;

			if (g_request_refresh != 0)
			{
				load_map();
			}

			GUI_input(get_tx(70), 0); /* "DEINE GRUPPE IST SEHR ERSCHÖPFT. DA ES BEREITS SPÄT AM ABEND IST, E..." */

			gs_town_loc_type = LOCTYPE_WILDCAMP;
			do_location();
			gs_town_loc_type = LOCTYPE_NONE;

			g_request_refresh = 2;
			g_wallclock_update = 0;

			if (g_game_state == GAME_STATE_MAIN)
			{
				/* figure out encounters etc. for next day */
				gs_travel_distance_per_18_hours = ((gs_travel_speed + (gs_travel_route_ptr->speed_mod * gs_travel_speed / 10)) * 18);

				if ((gs_journey_random_encounter_flag = (random_schick(100) <= gs_travel_route_ptr->encounters ? 1 : 0)))
				{
					gs_journey_random_encounter_position = random_schick(gs_travel_distance_per_18_hours);
				}

				if ((gs_journey_fight_flag = random_schick(100) <= gs_travel_route_ptr->fights / 3 ? 1 : 0) != 0)
				{
					gs_journey_fight_position = random_schick(gs_travel_distance_per_18_hours);
				}

				if (gs_quested_months > 3)
				{
					if ((gs_journey_olvir_treborn_flag = random_schick(100) <= 2 ? 1 : 0) != 0)
					{
						gs_journey_olvir_treborn_position = random_schick(gs_travel_distance_per_18_hours);
					}
				} else {
					gs_journey_olvir_treborn_flag = 0;
				}

				gs_travel_distance_per_18_hours = 0;
			}
		}

		if (g_check_disease && !gs_travel_detour && g_game_state == GAME_STATE_MAIN)
		{
			disease_effect();
		}

		if (g_request_refresh != 0 && !gs_travel_detour && g_game_state == GAME_STATE_MAIN)
		{
			call_mouse_bg();
			load_map();
			/* TODO: update window */
			memmove((void*)g_vga_memstart, (void*)gs_travel_map_ptr, 320 * 200);

			wait_for_vsync();
			set_palette(gs_travel_map_ptr + 64000 + 2, 0, 0x20);

			g_pp20_index = ARCHIVE_FILE_KARTE_DAT;
			gs_trv_i = 0;
			gs_travel_course_ptr2 = gs_travel_course_start;

			if (land_route_id == LROUTE_ID__CROSSLINK)
			{
				TM_draw_track_before_crosslink(0);
			}

			/* Redraw the track on the map */
			while (g_trv_track_pixel_bak[gs_trv_i++] != 0xaa)
			{
				*(fb_start + gs_travel_course_ptr2[1] * 320 + gs_travel_course_ptr2[0]) =  0x1c;
				gs_travel_course_ptr2 += (!reverse ? 2 : -2);
			}

			call_mouse();

			if (g_request_refresh == 2 && land_route_id != LROUTE_ID__CROSSLINK)
			{
				/* Return or continue? */
				if (GUI_radio(get_tx(71), 2, get_tx(72), get_tx(73)) == 2) /* "WOLLT IHR UMKEHREN ODER WEITE..." / "WEITERREISEN" / "LIEBER UMKEHREN" */
				{
					gs_journey_direction = (gs_journey_direction == JOURNEY_DIRECTION_FORWARD ? JOURNEY_DIRECTION_CHANGE_TO_BACKWARD : JOURNEY_DIRECTION_CHANGE_TO_FORWARD);

					/* The next code block adjusts the relative position of the last triggered travel event,
					 * which effectively means that the party did not pass it.
					 * However, this in only true in case that the direction change is done right at the last travel event.
					 *
					 * ---
					 *
					 * In all other cases, the effect is that the last travel event will not be triggered again when the
					 * party passes its position a second time. This leads to:
					 *
					 * Original-Bug 52:
					 * Repeatable travel events will not be triggered again if the party
					 * changes the traveling direction right after passing the event (this
					 * means, before the next travel event is triggered).
					 *
					 * ---
					 *
					 * Even if the direction change took place right at the last tevent, things can go wrong.
					 * If the party changes direction once again before another travel event is triggered,
					 * the relative position of the last travel event (which is still the same) is adjusted as if the
					 * direction change happens right at this travel event.
					 * Hence, the party can now pass the position of that travel event without triggering it.
					 * This leads to:
					 *
					 * Original-Bug 51:
					 * Travel events which give you the option to return can be circumvented by returning once again.
					 * This can be exploited to circumvent, for example, dangerous swamps.
					 *
					 * ---
					 *
					 * Moreover, the adjustment is also done for non-repeatable travel events,
					 * This can be utilized to reactivate such a travel event, like in this example:
					 *
					 * Consider a route with two tevents T1, T2, the first repeatable, the second not.
					 * Starting the journey, the party is heading forward, and accessibility for T1 and T2 is set forward, like this:
					 * [party: -->] [T1: ahead] [T2: ahead]
					 * Passing and thereby triggering both tevents results in:
					 *              [T1: behind] [T2: removed] [party: -->]
					 * T2 is the last triggered tevent. Now the party changes the direction, reviving T2:
					 *              [T1: behind] [T2: ahead]     [party: <--]
					 * Traveling back, we pass T2 (does not trigger) and T1 (does trigger) and have:
					 * [party: <--] [T1: ahead] [T2: ahead]
					 * The last triggered tevent is T1. Changing the direction again:
					 * [party: -->] [T1: ahead] [T2: ahead]
					 * This is the same state as the beginning. Traveling forward, T2 is triggered a second time.
					 * Hence we have:
					 *
					 * Original-Bug 50:
					 * Travel events marked as non-repeatable within the same journey can be reactivated by changing the direction at suitable positions.
					 *
					 * ---
					 *
					 * For the fix of Original-Bug 50, 51, and 52,
					 * we deactivate this a-posteriori adjustement of the relative position.
					 * Instead, the relative position is set to the correct state right after handling the travel event.
					 */
#ifndef M302de_ORIGINAL_BUGFIX
					/* Original-Bug 50, 51, and 52 */
					if ( last_tevent_id != -1) {
						g_journey_tevent_relative_position[last_tevent_id] = (gs_journey_direction == JOURNEY_DIRECTION_CHANGE_TO_BACKWARD ? TEVENT_POSITION_AHEAD : TEVENT_POSITION_BEHIND);
					}
#endif
				}
			}
			g_wallclock_x = g_basepos_x + 120;
			g_wallclock_y = g_basepos_y + 87;
			g_wallclock_update = 1;
			g_request_refresh = 0;
		}

		if (gs_journey_direction == JOURNEY_DIRECTION_CHANGE_TO_BACKWARD || gs_journey_direction == JOURNEY_DIRECTION_CHANGE_TO_FORWARD)
		{
			gs_journey_direction = (gs_journey_direction == JOURNEY_DIRECTION_CHANGE_TO_BACKWARD ? JOURNEY_DIRECTION_BACKWARD : JOURNEY_DIRECTION_FORWARD);

			gs_travel_course_ptr += ((!reverse && gs_journey_direction == JOURNEY_DIRECTION_FORWARD) || (reverse && gs_journey_direction != JOURNEY_DIRECTION_FORWARD) ? -2 : 2);
		}

		gs_travel_course_ptr += ((!reverse && gs_journey_direction == JOURNEY_DIRECTION_FORWARD) || (reverse && gs_journey_direction != JOURNEY_DIRECTION_FORWARD) ? 2 : -2);
	}

	if (g_game_state == GAME_STATE_MAIN && !gs_travel_detour && gs_journey_direction != JOURNEY_DIRECTION_BACKWARD)
	{
		call_mouse_bg();

		do {
			gs_travel_course_ptr += (!reverse ? -2 : 2);
			gs_travel_step_counter--;
			*(fb_start + gs_travel_course_ptr[1] * 320 + gs_travel_course_ptr[0]) =
				g_trv_track_pixel_bak[gs_travel_step_counter];

		} while (gs_travel_course_ptr[0] != -1);

		if (land_route_id == LROUTE_ID__CROSSLINK)
		{
			TM_draw_track_before_crosslink(1);
		}

		call_mouse();
	}

	g_traveling = 0;
}

#if defined(__BORLANDC__)
/* continue travel after arrival while still on map */
/* Menue am Zielort "DEINE GRUPPE KOENNTE VON HIER AUS NOCH AN FOLGENDE ORTE WEITERREISEN."
 * mit allen anderen Wegen des dortigen Wegweisers; Rueckgabe: Nummer der Antwort, -1 = kein Wegweiser.
 * Wird nie aufgerufen.
 * old_route_idx ist der Index des befahrenen Wegs in der Liste des Start-Wegweisers (keine Weg-ID). */
signed int TM_unused_continue_journey(struct trv_start_point *signpost_ptr, const signed int old_route_idx)
{
	signed int route_id1;
	signed int route_id;
	signed int answer;
	signed int town_i;
	signed int route_id2;
	signed int town_id;
	signed int old_route_id2;
	char *destinations_tab[7];

	old_route_id2 = signpost_ptr->linked_travel_routes[old_route_idx] - 1;
	gs_town_id = town_id = gs_journey_destination_town_id;
	signpost_ptr = &g_signposts[0];

	do {
		if ((unsigned char)signpost_ptr->town_id == town_id)
		{
			route_id1 = 0;
			while (signpost_ptr->linked_travel_routes[route_id1] != (signed char)-1)
			{
				if (signpost_ptr->linked_travel_routes[route_id1] - 1 == old_route_id2 &&
					(route_id1 || signpost_ptr->linked_travel_routes[route_id1 + 1] != (signed char)-1))
				{
					town_i = route_id2 = 0;

					while ((route_id = signpost_ptr->linked_travel_routes[route_id2]) != 0xff)
					{
						if (route_id2 != route_id1)
						{
							destinations_tab[town_i++] = get_ttx(235 + (gs_trv_signpost_menu_town_ids[town_i] = /* TEXT.LTX 236..287: Ortsname */
								((answer = g_land_routes[route_id - 1].town_id_1) != gs_town_id ?
									(signed char)answer : g_land_routes[route_id - 1].town_id_2)));
						}
						route_id2++;
					}

					gs_trv_signpost_menu_town_ids[town_i] = town_id;
					destinations_tab[town_i] = get_ttx(547); /* "ENDE DER REISE" */
					town_i++;

					gs_tm_unused_signpost_ptr = signpost_ptr;

					set_town_marker_and_textbox(town_id);

					answer = GUI_radio(get_ttx(546), (signed char)town_i, /* "DEINE GRUPPE KÖNNTE VON HIER AUS NOCH AN FOLGENDE ORTE WEITERREISEN." */
								destinations_tab[0], destinations_tab[1],
								destinations_tab[2], destinations_tab[3],
								destinations_tab[4], destinations_tab[5]);

					if (answer == -1)
					{
						answer = town_i;
					}

					gs_journey_destination_town_id = gs_trv_signpost_menu_town_ids[answer - 1];
					return answer;
				}

				route_id1++;
			}
		}

		signpost_ptr++;

	} while (signpost_ptr->town_id != -1);

	return -1;
}
#endif

/* Anzahl der Punkte eines Kartenpfads bis zur Endmarke x == -1 */
signed int TM_get_track_length(struct struct_point *track)
{
	signed int length = 0;

	while (track->x != -1)
	{
		track++;
		length++;
	}

	return length;
}

/**
 * \brief   sucht im Zielort den Wegweiser, an dem der befahrene Weg endet, und setzt daraus
 *          Ankunftsfeld und Blickrichtung (gs_travel_destination_x/_y/_viewdir)
 *
 *          Betritt den Ort nicht selbst; das macht do_travel_mode() mit diesen Werten.
 * \return  immer 0
 */
signed int trv_find_dest_town_signpost(void)
{
	signed int tmp; /* multi use */
	signed int signpost_typeindex;
	signed int tmp2; /* multi use */
	struct trv_start_point *signpost_ptr;
	struct location *locations_tab_ptr;

	signpost_typeindex = 0;
	gs_travel_destination_town_id = gs_journey_destination_town_id;
#ifndef M302de_ORIGINAL_BUGFIX
	// not a bug, but useless
	signpost_typeindex = 1;

	if (signpost_typeindex)
	{
#endif
		signpost_ptr = &g_signposts[0];
#ifndef M302de_ORIGINAL_BUGFIX
		signpost_typeindex = 0;
#endif
		do {
			if ((unsigned char)signpost_ptr->town_id == gs_travel_destination_town_id)
			{
				tmp = 0;

				do {
					tmp2 = signpost_ptr->linked_travel_routes[tmp] - 1;

					if ((g_land_routes[tmp2].town_id_1 == gs_town_id) || (g_land_routes[tmp2].town_id_2 == gs_town_id))
					{
						signpost_typeindex = signpost_ptr->typeindex;
						break;
					}

					tmp++;

				} while (signpost_ptr->linked_travel_routes[tmp] != 0xff);
			}

			signpost_ptr++;

		} while (!signpost_typeindex && signpost_ptr->town_id != -1);

		if (signpost_typeindex)
		{
			/* set the target town as current town */
			tmp2 = gs_town_id;
			gs_town_id = gs_travel_destination_town_id;

			/* load the map */
			call_load_area(1);

			locations_tab_ptr = &g_locations_tab[0];
			while (locations_tab_ptr->loctype != LOCTYPE_SIGNPOST || locations_tab_ptr->typeindex != signpost_typeindex)
			{
				locations_tab_ptr++;
			}

			tmp = locations_tab_ptr->locdata;
			gs_travel_destination_x = (tmp >> 8) & 0xff; /* bits 8..15 */
			gs_travel_destination_y = tmp & 0x0f;        /* bits 0..3 */

			/* In principle, the viewdir is stored in bits 4..7 of locdata.
			 * But strangely, this information is ignored. Instead, a heuristic based on
			 * the signpost's coordinates and the arrival position is used.
			 */
			gs_travel_destination_viewdir = trv_journey_dest_town_viewdir(locations_tab_ptr->pos);

			gs_town_id = tmp2;

			/* load the map */
			call_load_area(1);
		}
#ifndef M302de_ORIGINAL_BUGFIX
	}
#endif

	return 0;
}

/**
 * \brief   Richtung vom Ankunftsfeld zum Wegweiser (locdata.pos); do_travel_mode() dreht sie um,
 *          so dass die Gruppe vom Wegweiser weg in den Ort schaut
 *
 * \param   coordinates Position des Wegweisers (x in Bits 8..15, y in Bits 0..3)
 */
signed int trv_journey_dest_town_viewdir(const signed int coordinates)
{
	const signed int x = (coordinates >> 8) & 0xff;
	const signed int y = coordinates & 0xf;
	const signed int retval = (gs_travel_destination_x < x ? EAST :
				(gs_travel_destination_x > x ? WEST :
				(gs_travel_destination_y < y ? SOUTH : NORTH)));

	return retval;
}

/**
 * \brief   zeichnet die ersten length Punkte eines Landwegs auf die Karte bzw. stellt sie wieder her
 *
 * \param   route_id    Landweg
 * \param   length      Anzahl Punkte (hoechstens 20, Groesse von g_trv_crosslink_pixel_bak)
 * \param   direction   0 = vom Anfang des Wegs, 1 = vom Ende her
 * \param   restore     0 = zeichnen und alte Pixel merken, 1 = gemerkte Pixel zurueckschreiben
 */
void TM_draw_track(const signed int route_id, const signed int length, const signed int direction, const signed int restore)
{
	signed int i;
	int16_t *ptr;
	uint8_t* fb_start;

	fb_start = g_vga_memstart;
	ptr = (int16_t*)((g_buffer9_ptr + *(int16_t*)((uint8_t*)g_buffer9_ptr + 4 * (route_id - 1))) + 0xecL);
	ptr += 2;

	if (direction)
	{
		/* move ptr to the last valid value */
		while (ptr[0] != -1)
		{
			ptr += 2;
		}

		ptr -= 2;
	}

	for (i = 0; i < length; i++)
	{
		if (restore == 0)
		{
			/* save the old pixel from the map */
			g_trv_crosslink_pixel_bak[i] = *(fb_start + ptr[1] * 320 + ptr[0]);

			/* write a new one */
			*(fb_start + ptr[1] * 320 + ptr[0]) = 0x1c;

			/* move the pointer */
			ptr += (!direction ? 2 : -2);
		} else {
			/* move the pointer */
			/* Fehler im Original: der Zeiger wird hier VOR dem Zurueckschreiben weitergesetzt.
			 * Punkt 0 bleibt daher in Wegfarbe stehen, Punkt i+1 bekommt das Pixel von Punkt i. */
			ptr += (!direction ? 2 : -2);

			/* restore the pixel from the map */
			*(fb_start + ptr[1] * 320 + ptr[0]) = g_trv_crosslink_pixel_bak[i];
		}
	}
}

#if defined(__BORLANDC__)
void TM_unused2(void)
{

}
#endif

/**
 * \brief   Draw the track from the last town to the start of the crosslink on the travel map.
 *          Used when traveling the crosslink, to display the complete track all the way back from the last town.
 *
 * \param   restore     0 = zeichnen, 1 = wieder loeschen (siehe TM_draw_track())
 */
void TM_draw_track_before_crosslink(const signed int restore)
{
	if (!(g_journey_crosslink_status & 1))
	{
		if (gs_town_id == TOWN_ID_PEILINEN)
		{
			// draw from Peilinen to start of crosslink
			TM_draw_track(LROUTE_ID_PEILINEN_ROVAMUND, 9, 0, restore);
		} else {
			// draw from Rovamund to start of crosslink
			TM_draw_track(LROUTE_ID_PEILINEN_ROVAMUND, 17, 1, restore);
		}
	} else {
		if (gs_town_id == TOWN_ID_KRAVIK)
		{
			TM_draw_track(LROUTE_ID_KRAVIK_SKELELLE, 8, 0, restore);
			// draw from Kravik to start of crosslink
		} else {
			TM_draw_track(LROUTE_ID_KRAVIK_SKELELLE, 17, 1, restore);
			// draw from Skelellen to start of crosslink
		}
	}
}

void TM_do_journey_crosslink(void)
{
	trv_do_journey(LROUTE_ID__CROSSLINK, g_journey_crosslink_status & 1);
	/* result of (g_journey_crosslink_status & 1) :
	 * g_journey_crosslink_status 2 (from Peilinen) or 4 (from Rovamund) --> 0 (normal direction)
	 * g_journey_crosslink_status 1 (from Kravik) or 3 (from Skelellen) --> 1 (reverse direction)
	 */

	TRV_event(145);
}
