/*
 * seg059 - Taverne: Hauptmenue; Ereignisse der Seereise
 *
 * Inhalt:    do_tavern() ist die Taverne: Oeffnungszeiten (11 bzw. 16 Uhr bis 3 Uhr, je nach
 *            Qualitaet), Reden (talk_tavern, seg060, danach evtl. einem Informanten folgen),
 *            Essen und Trinken (Preis nach Qualitaet und Gruppengroesse, wer nicht zahlen
 *            kann, gilt als Zechpreller und ist dort unerwuenscht), Talent anwenden und
 *            beim kombinierten Haus (Thorwal) der Wechsel in die Herberge.
 *            Dazu die Ereignisse der Seereise: Krakenangriff, Piratenangriff und das
 *            Auftauchen des Totenschiffs (prolog_ghostship).
 * Aufrufer:  Ortstabelle g_location_handlers (seg025), THO_tav_inn_combi() (seg069);
 *            die drei Seereise-Funktionen aus sea_travel() (seg063).
 * Daten:     gs_tav_cheated_flags (Zechpreller je Taverne).
 * Besonders: Talentprobe in der Taverne: wurde in den letzten 2 Minuten gespeichert, ist sie
 *            um 50 erschwert (Schutz gegen Speichern/Laden).
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg059.c
 */

#include <stdio.h>
#include <time.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg004.h"
#include "seg025.h"
#include "seg026.h"
#include "seg027.h"
#include "seg029.h"
#include "seg047.h"
#include "seg060.h"
#include "seg073.h"
#include "seg096.h"
#include "seg097.h"
#include "seg103.h"
#include "seg117.h"

/**
 * \brief   location handler of a tavern
 *
 * Opening hours: taverns with quality 6..13 from 11:00, all others from 16:00, until 3:00.
 * g_combo_mode != 0: tavern and inn in one house (THO_tav_inn_combi(), seg069),
 * the 5th menu entry switches to the inn (g_combo_mode = 2).
 */
void do_tavern(void)
{
	signed int i;
	signed int done = 0;
	signed int answer;
	signed int reduction; /* reduction of hunger and thirst */
	int32_t p_money_before;
	int32_t p_money_after; /* multi use: also the price of the meal (first in silver, then in heller) */
	const struct inn_descr *tavern = &g_tavern_descr_table[gs_town_typeindex];
	signed int handicap;
	time_t timeval;


	GUI_print_loc_line(get_tx(gs_town_locdata));

	g_request_refresh = 1;

	if (tavern->quality >= 6 && tavern->quality <= 13) {

		if (gs_day_timer < HOURS(11) && gs_day_timer > HOURS(3)) {

			GUI_output(get_ttx(801)); /* "GEÖFFNET VON@11.00 UHR BIS 3.00 UHR" */
			leave_location();
			return;
		}

	} else if (gs_day_timer < HOURS(16) && gs_day_timer > HOURS(3)) {

			GUI_output(get_ttx(481)); /* "GEÖFFNET VON@16.00 UHR BIS 3.00 UHR" */
			leave_location();
			return;
	}

	draw_loc_icons(g_combo_mode == 0 ? 4 : 5, MENU_ICON_TALK, MENU_ICON_ORDER_FOOD, MENU_ICON_APPLY_TALENT, MENU_ICON_LEAVE, MENU_ICON_INN);

	while (!done) {

		if (g_request_refresh != 0) {

			draw_main_screen();
			disable_ani();
			load_ani(27);
			init_ani(0);
			GUI_print_loc_line(get_tx(gs_town_locdata));
			set_audio_track(ARCHIVE_FILE_INN_XMI);
			g_request_refresh = 0;
		}

		handle_gui_input();

		if (gs_tav_cheated_flags[gs_town_typeindex]) {

			GUI_output(get_ttx(472)); /* "<BETRÜGER!@IHR HABT JA WIRKLICH NERVEN HIER NOCH MAL AUFZUTAUCHEN.>..." */
			done = 1;
			g_mouse_rightclick_event = g_action_id = 0;
		}

		if (gs_day_timer < HOURS(11) && gs_day_timer > HOURS(3)) {

			GUI_output(get_ttx(9)); /* "WIR SCHLIESSEN JETZT!@ALLE BITTE RAUS HIER, BEVOR DIE GARDISTEN WIE..." */
			done = 1;
			g_mouse_rightclick_event = g_action_id = 0;
		}

		if (g_mouse_rightclick_event || g_action_id == KEY_SCAN_CODE_PAGE_UP) {

			answer = GUI_radio(get_ttx(469), g_combo_mode == 0 ? 4 : 5, /* "HIER KÖNNT IHR" */
					get_ttx(343), get_ttx(470), get_ttx(212), /* "REDEN" / "ESSEN UND TRINKEN" / "TALENT ANWENDEN" */
					get_ttx(471), get_ttx(824)) - 1; /* "VERLASSEN" / "HERBERGE BESUCHEN" */

			if (answer != -2) {
				g_action_id = answer + ACTION_ID_ICON_1;
			}
		}

		if (g_action_id == ACTION_ID_ICON_1) {
			/* TALK */

			p_money_before = get_party_money();
			talk_tavern();
			p_money_after = get_party_money();

#if FIX_0343
			/* Fix 343: nur eine Ausgabe anzeigen, keinen Gewinn */
			if (p_money_before > p_money_after) {
#else
			if (p_money_before != p_money_after) {
#endif

				make_valuta_str(g_text_output_buf, p_money_before - p_money_after);
				sprintf(g_dtp2, get_ttx(825), g_text_output_buf); /* "IHR HABT %s IN DER TAVERNE AUSGEGEBEN!" */
				GUI_output(g_dtp2);
			}

			if (gs_tlk_tav_followinformer) {

				tavern_follow_informer();
			}

			g_request_refresh = done = 1;
			g_combo_mode = 0;

		} else if (g_action_id == ACTION_ID_ICON_2) {
			/* EAT AND DRINK */

			p_money_after = count_heroes_in_group() * (6 - tavern->quality / 4);

			p_money_after += tavern->price_mod * p_money_after / 100;
			sprintf(g_dtp2, get_ttx(473), (signed int)p_money_after); /* "<WIR HABEN ALLES DA, WAS EUER HERZ BEGEHRT.@FÜR %d SILBERSTÜCKE WER..." */

			if (GUI_bool(g_dtp2)) {

				GUI_output(tavern->quality < 5 ? get_ttx(475) : (tavern->quality < 15 ? get_ttx(476) : get_ttx(477))); /* "EIN ERLESENES MENÜ MIT GUTEM..." / "ZWAR BEKOMMT IHR HIER NICHT G..." / "DAS BIER SCHMECKT ETWAS ABGES..." */

#if !FIX_0336
				timewarp(HOURS(1));
#endif

				for (i = 0; i <= 6; i++) {

					gs_food_message[i] = g_food_message_shown[i] = 0;

					if ((get_hero(i)->typus != HERO_TYPE_NONE) && (get_hero(i)->group_id == gs_active_group_id))
					{

						reduction = (21 - tavern->quality) * 20;
						if (reduction > 100) {
							reduction = 100;
						}

						get_hero(i)->hunger -= (unsigned char)reduction;

						if (get_hero(i)->hunger < 0) {
							get_hero(i)->hunger = 0;
						}

						reduction = (21 - tavern->quality) * 30;
						if (reduction > 100) {
							reduction = 100;
						}

						get_hero(i)->thirst -= (unsigned char)reduction;

						if (get_hero(i)->thirst < 0) {
							get_hero(i)->thirst = 0;
						}
					}
				}
#if FIX_0336
				/* Fix 336: erst essen, dann die Stunde vergehen lassen (sonst isst herokeeping() vorher Proviant) */
				timewarp(HOURS(1));
#endif


				p_money_before = get_party_money();
				p_money_after *= 10L;

				if (p_money_before < p_money_after) {

					GUI_output(get_ttx(474)); /* "<DAS HABEN WIR GERN, SICH ERST DEN BAUCH VOLLSCHLAGEN UND DANN NICH..." */

					gs_tav_cheated_flags[gs_town_typeindex] = 1;
					done = 1;
					g_combo_mode = 0;

				} else {

					p_money_before -= p_money_after;
					set_party_money(p_money_before);
				}
			}

		} else if (g_action_id == ACTION_ID_ICON_3) {
			/* USE TALENT */

			time(&timeval);

			/* the talent test gets a handicap of +50 (i.e. it practically fails) if the game was
			 * saved up to 2 minutes ago. probably to prevent excessive save & reload */
			handicap = (timeval - g_last_save_time) > 120 ? 0 : 50;

			if (GUI_use_talent2(handicap, get_ttx(395)) == -1) { /* "WER SOLL EIN TALENT ANWENDEN?" */
				done = 1;
				g_combo_mode = 0;
			}

		} else if (g_action_id == ACTION_ID_ICON_4) {
			/* LEAVE */

			done = 1;
			g_combo_mode = 0;

		} else if (g_action_id == ACTION_ID_ICON_5) {
			/* VISIT INN */

			if (g_combo_mode != 0) {

				done = 1;
				g_combo_mode = 2;
			}
		}
	}

	copy_palette();
	leave_location();
}

/**
 * \brief   sea travel event: attack of a kraken (called by sea_travel(), seg063)
 */
void octopus_attack_wrapper(void)
{
	octopus_attack();
	g_request_refresh = 1;
}

/**
 * \brief   sea travel event: attack of pirates (called by sea_travel(), seg063)
 */
void pirates_attack_wrapper(void)
{
	pirates_attack();
	g_request_refresh = 1;
}

/**
 * \brief   sea travel event: the ghost ship appears out of the fog (texts from SHIP.DTX)
 *
 * If the party decides to enter the unknown ship, gs_travel_detour is set to the
 * Totenschiff dungeon, which sea_travel() (seg063) then enters.
 */
void prolog_ghostship(void)
{
	signed int answer;
	signed int tw_bak;
	signed int basepos_x_bak;
	signed int basepos_y_bak;

	tw_bak = g_textbox_width;
	basepos_x_bak = g_basepos_x;
	basepos_y_bak = g_basepos_y;
	g_textbox_width = 7;
	g_basepos_x = g_basepos_y = 0;

	load_ani(17);
	draw_main_screen();
	init_ani(1);

	load_tx(ARCHIVE_FILE_DNGS_DTX + 1);

	GUI_output(get_tx(18)); /* "URPLÖTZLICH KÖNNT IHR IN DER FERNE EINE DICHTE NEBELWAND ENTDECKEN,..." */
	GUI_output(get_tx(19)); /* "BINNEN WENIGER MINUTEN ERREICHT SIE EUCH UND HÜLLT DAS GESAMTE SCHI..." */

	do {
		answer = GUI_radio(get_tx(20), 2, get_tx(21), get_tx(22)); /* "AUF EIN MAL TAUCHT AUS DEM WE..." / "DAS UNBEKANNTE SCHIFF BETRETEN?" / "ES PASSIEREN LASSEN?" */

	} while (answer == -1);

	if (answer == 1) {
		g_request_refresh = 0;
		gs_travel_detour = (DUNGEON_ID_TOTENSCHIFF);
	} else {
		g_request_refresh = 1;
	}

	disable_ani();
	g_textbox_width = tw_bak;
	g_basepos_x = basepos_x_bak;
	g_basepos_y = basepos_y_bak;
}
