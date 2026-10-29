/*
 * seg053 - Heiler
 *
 * Inhalt:    Ort "Heiler" in der Stadt: Wunden behandeln (auch dauerhaften
 *            LE-Verlust), Krankheiten heilen und Gift neutralisieren, jeweils
 *            gegen Bezahlung. Preis und Erfolg hängen vom Heiler ab; zwischen
 *            21 und 6 Uhr ist er verärgert und verlangt das Doppelte.
 *            is_hero_healable prüft, ob ein Held überhaupt behandelt werden kann.
 * Aufrufer:  do_healer über do_location (seg025) und aus seg070;
 *            is_hero_healable auch aus den Heiltalenten (seg103, seg104).
 * Daten:     g_healer_descr_table: je Heiler Preisaufschlag in Prozent und
 *            "quality" (je kleiner, desto besser).
 * Besonders: Misslingt eine Krankheits- oder Giftbehandlung, kostet sie die
 *            Hälfte. Der Erfolg ist 120 - 10*quality (Gift: 5*quality) plus
 *            Bonus der Krankheit/des Gifts in Prozent.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg053.c
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
#include "seg047.h"
#include "seg053.h"
#include "seg096.h"
#include "seg097.h"

static const struct healer_descr g_healer_descr_table[41] = {
	{  30,  2 },
	{  50, 10 },
	{ -10, 10 },
	{  10, 12 },
	{  30, 12 },
	{  30,  9 },
	{   0, 12 },
	{   0,  4 },
	{ -10,  6 },
	{  40, 15 },
	{   0,  7 },
	{   0, 16 },
	{  20,  3 },
	{  40,  5 },
	{  20, 13 },
	{ -10,  4 },
	{  20, 10 },
	{   0, 14 },
	{  30,  1 },
	{  20,  8 },
	{   0, 12 },
	{  10, 10 },
	{  50,  6 },
	{  20,  2 },
	{ -10,  7 },
	{   0,  7 },
	{  10, 14 },
	{   0, 12 },
	{  20,  7 },
	{  40,  4 },
	{ -20,  8 },
	{  10, 15 },
	{ -50, 12 },
	{  10,  3 },
	{ -20, 10 },
	{ -20, 11 },
	{  30, 12 },
	{  20, 17 },
	{  20,  6 },
	{ -10,  3 },
	{  -1,  0 }
}; // ds:0x66ea

/**
 * \brief   check if hero can be healed (by a healer or a healing talent)
 *
 * \param   hero        pointer to hero
 * \return              1 if hero can be healed, else 0.
 */
signed int is_hero_healable(const struct struct_hero *hero)
{
	if (hero->flags.dead || hero->flags.petrified) {

		/* this hero can not be helped */
		GUI_output(get_ttx(778)); /* "HIER VERSAGEN LEIDER ALLE MEDIZINISCHEN KENNTNISSE!" */

		return 0;
	} else {
		return 1;
	}
}

/**
 * \brief   location handler for healers
 */
void do_healer(void)
{
	signed char motivation = 0; /* 0 = not yet greeted, 1 = normal, 2 = annoyed (night): double price */
	signed int leave_healer = 0;
	signed int redraw_locline;
	struct struct_hero *hero;
	signed long money;
	signed long price;
	const struct healer_descr *healer;
	signed int poison_id;
	signed int answer;
	signed int disease_id;

#if !defined(__BORLANDC__)
	const uint8_t typi = gs_town_typeindex;
	const int8_t h_price = g_healer_descr_table[typi].price_mod;
	const uint8_t h_qual = g_healer_descr_table[typi].quality;

	D1_INFO("Heiler: %2d\n", typi);
	D1_INFO("\tPreis: %d%% [50,150]\n", h_price + 100);
	D1_INFO("\tQualitaet: %2d [1,15]\n", h_qual);
#endif

	redraw_locline = g_request_refresh = 1;
	healer = &g_healer_descr_table[gs_town_typeindex];
	draw_loc_icons(4, MENU_ICON_HEAL_WOUNDS, MENU_ICON_HEAL_DISEASE, MENU_ICON_HEAL_POISON, MENU_ICON_LEAVE);

	while (leave_healer == 0) {

		if (g_request_refresh != 0) {
			draw_main_screen();
			disable_ani();
			load_ani(23);
			init_ani(0);

			GUI_print_loc_line(get_tx(gs_town_locdata));

			set_audio_track(ARCHIVE_FILE_HEALER_XMI);

			g_request_refresh = redraw_locline = 0;

			if (!motivation) {

				motivation = 1;

				/* from 9.00 pm to 6.00 am the healer gets unkind */
				if (gs_day_timer > HOURS(21) || gs_day_timer < HOURS(6)) {
					GUI_output(get_ttx(484)); /* "<GANZ SCHÖN UNVERSCHÄMT, UM DIESE ZEIT EINEN ALTEN MANN ZU STÖREN!>" */
					motivation = 2;
				}
			}
		}

		if (redraw_locline != 0) {
			GUI_print_loc_line(get_tx(gs_town_locdata));
			redraw_locline = 0;
		}

		handle_gui_input();

		if (g_mouse_rightclick_event || g_action_id == KEY_SCAN_CODE_PAGE_UP) {

			g_textbox_width = 4;

			answer = GUI_radio(get_ttx(459), 4, get_ttx(455), get_ttx(456), get_ttx(457), get_ttx(458)) - 1; /* "HEILER OPTIONEN" / "WUNDEN BEHANDELN" / "KRANKHEITEN BEHANDELN" / "GIFT NEUTRALISIEREN" / "HEILER VERLASSEN" */

			g_textbox_width = 3;

			if (answer != -2) {
				g_action_id = answer + ACTION_ID_ICON_1;
			}
		}

		if (g_action_id == ACTION_ID_ICON_4) {
			leave_healer = 1;
			continue;
		}

		if (g_action_id == ACTION_ID_ICON_1) {

			/* Heal Wounds */

			money = get_party_money();
			answer = select_hero_from_group(get_ttx(460)); /* "WER SOLL BEHANDELT WERDEN?" */
			if (answer != - 1) {

				hero = get_hero(answer);

				if (is_hero_healable(hero)) {

					/* LEmax >= LE and no permanent LEdamage */
					if ((hero->le >= hero->le_max) && !hero->le_max_malus) {

						/* Hero seems OK */
						sprintf(g_dtp2,	get_ttx(461), hero->alias); /* "<%s SCHEINT MIR VÖLLIG IN ORDNUNG ZU SEIN.>" */
						GUI_output(g_dtp2);
					} else {

						/* calculate price */
						price = hero->le_max_malus * 50;
						price += (hero->le_max + hero->le_max_malus - hero->le) * 5;
						price += healer->price_mod * price / 100;

						if (motivation == 2)
							price *= 2;

						/* ask */
						sprintf(g_dtp2,	get_ttx(464), hero->alias, price); /* "<DIE LE-REGENERATION VON %s WÜRDE EUCH %ld SILBERSTÜCKE KOSTEN.>" */

						if (GUI_bool(g_dtp2)) {

							price *= 10;

							if (money < price) {

								GUI_output(get_ttx(401)); /* "SOVIEL GELD HABT IHR DOCH GAR NICHT!" */

							} else {
								/* remove money */
								money -= price;
								set_party_money(money);

								/* heal permanent damage TODO:LE += */
								hero->le_max += hero->le_max_malus;
								hero->le_max_malus = 0;

								/* time passes by (number of missing LE) minutes */
								timewarp(MINUTES((signed long)(hero->le_max - hero->le)));

								/* heal LE */
								add_hero_le(hero, hero->le_max);

								/* prepare output */
								sprintf(g_dtp2,	get_ttx(467), hero->alias); /* "<TSA SEI GEDANKT. %s IST WIEDER VÖLLIG IN ORDNUNG!>" */
								GUI_output(g_dtp2);
							}
						}
					}
				}
			}

		} else if (g_action_id == ACTION_ID_ICON_2) {

			/* Cure Disease */
			money = get_party_money();
			answer = select_hero_from_group(get_ttx(460)); /* "WER SOLL BEHANDELT WERDEN?" */
			if (answer != -1) {

				hero = get_hero(answer);

				if (is_hero_healable(hero)) {

					disease_id = hero_is_diseased(hero);

					if (!disease_id) {

						/* Hero is not diseased */
						sprintf(g_dtp2,	get_ttx(462), hero->alias); /* "<ICH KANN BEI %s KEINE KRANKHEIT DIAGNOSTIZIEREN!>" */
						GUI_output(g_dtp2);

					} else {
						/* calculate price */
						price = g_disease_strength[disease_id] * 10;

						price += healer->price_mod * price / 100;

						if (motivation == 2)
							price *= 2;

						/* prepare output */
						sprintf(g_dtp2, get_ttx(465), hero->alias, get_ttx(disease_id + 0x193), price); /* "<%s HAT %s. ICH KÖNNTE FÜR %ld SILBERSTÜCK..." / TEXT.LTX 404..410: Krankheit */

						if (GUI_bool(g_dtp2)) {

							price *= 10;

							if (money < price) {
								GUI_output(get_ttx(401)); /* "SOVIEL GELD HABT IHR DOCH GAR NICHT!" */
							} else {
								timewarp(HOURS(1));

								if (random_schick(100) <= (120 - 10 * healer->quality) + g_diseases_healer_bonus[disease_id]) {
									/* heal the disease */
									hero->disease[disease_id].status = DISEASE_STATUS_RECOVER;
									hero->disease[disease_id].time_counter = 0;

									/* prepare output */
									sprintf(g_dtp2,	get_ttx(467), hero->alias); /* "<TSA SEI GEDANKT. %s IST WIEDER VÖLLIG IN ORDNUNG!>" */
									GUI_output(g_dtp2);
								} else {
									price /= 2;
									GUI_output(get_ttx(468)); /* "<BEI PERAINE! IRGEND ETWAS IST SCHIEFGEGANGEN.@TUT MIR WIRKLICH LEI..." */
								}

								/* remove money */
								money -= price;
								set_party_money(money);
							}
						}
					}
				}
			}

		} else if (g_action_id == ACTION_ID_ICON_3) {

			/* Heal Poison */
			money = get_party_money();
			answer = select_hero_from_group(get_ttx(460)); /* "WER SOLL BEHANDELT WERDEN?" */
			if (answer != -1) {

				hero = get_hero(answer);

				if (is_hero_healable(hero)) {

					poison_id = hero_is_poisoned(hero);

					if (poison_id == POISON_ID_NONE) {

						/* Hero is not poisoned */
						sprintf(g_dtp2,	get_ttx(463), hero->alias); /* "<%s IST DOCH GAR NICHT VERGIFTET.>" */
						GUI_output(g_dtp2);
					} else {
						/* calculate price */
						price = g_poison_strength[poison_id] * 20;
						price += healer->price_mod * price / 100;
						if (motivation == 2)
							price *= 2;

						/* prepare output */
						sprintf(g_dtp2, get_ttx(466), price, hero->alias); /* "<MAL RECHNEN...@ICH MÜSSTE %ld SILBERSTÜCKE FÜR EINEN VERSUCH, DAS..." */

						if (GUI_bool(g_dtp2)) {

							price *= 10;

							if (money < price) {

								GUI_output(get_ttx(401)); /* "SOVIEL GELD HABT IHR DOCH GAR NICHT!" */

							} else {
								timewarp(HOURS(1));

								if (random_schick(100) <= (120 - 5 * healer->quality) + g_poisons_healer_bonus[poison_id]) {

									/* cure the poison */
									hero->poison[poison_id].time_counter = 0;
									hero->poison[poison_id].status = POISON_STATUS_RECOVER;

									/* prepare output */
									sprintf(g_dtp2,	get_ttx(467), hero->alias); /* "<TSA SEI GEDANKT. %s IST WIEDER VÖLLIG IN ORDNUNG!>" */
									GUI_output(g_dtp2);

								} else {
									price /= 2;
									GUI_output(get_ttx(468)); /* "<BEI PERAINE! IRGEND ETWAS IST SCHIEFGEGANGEN.@TUT MIR WIRKLICH LEI..." */
								}

								money -= price;
								set_party_money(money);
							}
						}
					}
				}
			}
		}
	}

	copy_palette();
	leave_location();

}
