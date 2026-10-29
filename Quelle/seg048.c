/*
 * seg048 - Statusmenü (Bedienung)
 *
 * Inhalt:    Steuert den Statusbildschirm: Blättern zwischen den Helden der
 *            Gruppe, Auswahl eines Inventarplatzes (roter Rahmen) und eines
 *            Zielplatzes (gelber Rahmen), Gegenstände umlegen, an einen
 *            anderen Helden übergeben ("Handelsmodus"), ansehen (Auge) oder
 *            essen/trinken (Mund), dazu das Kontextmenü: Namen ändern,
 *            Gegenstand benutzen/wegwerfen, Talent/Zauber anwenden, Seitenwechsel.
 * Aufrufer:  status_select_hero und status_menu aus der Hauptschleife (seg002)
 *            und aus Ereignissen (seg116, seg117).
 * Daten:     Rahmenkoordinaten der 23 Inventarplätze plus Auge (23) und
 *            Mund (24); g_forbid_ta_falschspiel sperrt "Falschspiel" in der
 *            Talentauswahl des Statusmenüs (seg103).
 * Besonders: Rahmenfarben: 9 = rot (Auswahl), 8 = gelb (Ziel), 30 = Hintergrund.
 *            Im Grundwerte-Menü fällt Fall 9 absichtlich in Fall 10 durch.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg048.c
 */
#include <stdio.h>
#include <string.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg004.h"
#include "seg007.h"
#include "seg026.h"
#include "seg046.h"
#include "seg047.h"
#include "seg048.h"
#include "seg096.h"
#include "seg097.h"
#include "seg098.h"
#include "seg103.h"
#include "seg105.h"
#include "seg106.h"
#include "seg107.h"
#include "seg108.h"

static const struct struct_point g_invslot_borderxy_table[25] = {
	{ 0x0044, 0x0037 },
	{ 0x003a, 0x0050 },
	{ 0x0091, 0x0050 },
	{ 0x0044, 0x006a },
	{ 0x0087, 0x006a },
	{ 0x0044, 0x008c },
	{ 0x0087, 0x00ae },
	{ 0x000f, 0x0037 },
	{ 0x0020, 0x0037 },
	{ 0x000f, 0x0048 },
	{ 0x0020, 0x0048 },
	{ 0x000f, 0x0059 },
	{ 0x0020, 0x0059 },
	{ 0x000f, 0x006a },
	{ 0x0020, 0x006a },
	{ 0x000f, 0x007b },
	{ 0x0020, 0x007b },
	{ 0x000f, 0x008c },
	{ 0x0020, 0x008c },
	{ 0x000f, 0x009d },
	{ 0x0020, 0x009d },
	{ 0x000f, 0x00ae },
	{ 0x0020, 0x00ae },
	{ 0x0080, 0x0037 },
	{ 0x0091, 0x0037 }
}; // ds:0x64ce
signed int g_forbid_ta_falschspiel = 0; // ds:0x6532
static char g_extraspace_separated_strings[7] = "%s %s "; // ds:0x6534
static char g_empty_string8[1] = ""; // ds:0x653b
static char g_extraspace_separated_strings2[7] = "%s %s "; // ds:0x653c
static char g_empty_string9[1] = ""; // ds:0x6543


struct mouse_cursor *g_current_cursor_bak; // ds:0xe3ae

/**
 * \brief   resets the inventory selection of the status screen
 *
 *          Moves the red frame back to the first knapsack slot (7), ends a pending
 *          target selection (yellow frame) and restores the default mouse cursor.
 */
/* Borlandified and identical */
void reset_item_selector(void)
{
	if (g_status_page_mode < 3) {

		/* remove the previous border */
		do_border(g_vga_memstart,
				g_invslot_borderxy_table[g_statuspage_sel_slot].x,
				g_invslot_borderxy_table[g_statuspage_sel_slot].y,
				g_invslot_borderxy_table[g_statuspage_sel_slot].x + 17,
				g_invslot_borderxy_table[g_statuspage_sel_slot].y + 17,
				30);

		/* select the first knapsack slot */
		g_statuspage_sel_slot_drawn = g_statuspage_sel_slot = 7;

		/* set the new red border */
		do_border(g_vga_memstart,
				g_invslot_borderxy_table[g_statuspage_sel_slot].x,
				g_invslot_borderxy_table[g_statuspage_sel_slot].y,
				g_invslot_borderxy_table[g_statuspage_sel_slot].x + 17,
				g_invslot_borderxy_table[g_statuspage_sel_slot].y + 17,
				9);

		g_statuspage_target_slot = -1;
		g_statuspage_target_slot_drawn = HERO_INVENTORY_SLOT_EYE;
	}

	g_current_cursor = g_current_cursor_bak = &g_default_mouse_cursor;
}

/**
 * \brief   the status menu of a hero
 *
 *          Item handling: the red frame marks the selected slot, ENTER/click starts
 *          the target selection (yellow frame, starting at the eye). Switching to
 *          another hero during the target selection enables the trade mode:
 *          owner_hero gives, shown_hero receives.
 *
 * \param   hero_pos    position of the hero to be shown first
 */
/* Borlandified and identical */
void status_menu(signed int hero_pos)
{
	signed int answer;
	signed int done;
	signed int force_redraw;
	signed int width;
	signed int height;
	signed int file_bak;
	struct struct_hero *shown_hero;
	struct struct_hero *owner_hero;
	signed int disease_id;
	signed int num_options;
	signed int tw_bak;
	signed int trade_mode;
	struct nvf_extract_desc nvf;

	done = 0;
	force_redraw = 1;
	disease_id = 0;
	trade_mode = 0;
	g_mouse1_doubleclick = 0;

	owner_hero = shown_hero = get_hero(hero_pos);

	if (!shown_hero->typus || !g_statusmenu_allowed) {
		return;
	}

	/* disable timers */
	g_timers_disabled++;

	/* set textbox width */
	tw_bak = g_textbox_width;
	g_textbox_width = 3;

	set_audio_track(ARCHIVE_FILE_SUMMARY_XMI);

	file_bak = g_text_file_index;

	load_tx2(ARCHIVE_FILE_CHARTEXT_LTX);

	load_ggsts_nvf();

	g_request_refresh = 1;
	g_action_id = 0;
	g_status_page_mode = 1;

	while (done == 0) {

		if (g_request_refresh != 0 || force_redraw != 0) {

			g_status_page_hero = hero_pos;

			call_mouse_bg();

			status_show(hero_pos);

			g_status_page_hunger = g_status_page_thirst = -1;

			update_status_bars();

			if (g_status_page_mode < 3) {

				if (owner_hero == shown_hero) {
					/* set the new red border */
					do_border(g_vga_memstart,
						g_invslot_borderxy_table[g_statuspage_sel_slot].x,
						g_invslot_borderxy_table[g_statuspage_sel_slot].y,
						g_invslot_borderxy_table[g_statuspage_sel_slot].x + 17,
						g_invslot_borderxy_table[g_statuspage_sel_slot].y + 17,
						9);
				}

				if (owner_hero->inventory[g_statuspage_sel_slot].item_id != ITEM_ID_NONE) {

					sprintf(g_dtp2, g_extraspace_separated_strings,
						(char*)GUI_name_base_form(g_itemsname[owner_hero->inventory[g_statuspage_sel_slot].item_id]),
						!is_in_int_array(owner_hero->inventory[g_statuspage_sel_slot].item_id,
							g_forbidden_item_ids_table[shown_hero->typus - 1]) ? g_empty_string8 : get_tx2(66)); /* "(KANN NICHT BENUTZT WERDEN)" */

					if (g_itemsdat[owner_hero->inventory[g_statuspage_sel_slot].item_id].flags.weapon) {
						strcat(g_dtp2, get_ttx(48 + g_itemsdat[owner_hero->inventory[g_statuspage_sel_slot].item_id].item_subtype_id)); /* TEXT.LTX 48..56: Waffentalent */
					}

					GUI_print_string(g_dtp2, 16, 192);
				}

				if (g_statuspage_target_slot != -1) {
					/* draw the yellow frame at the target slot */
					do_border(g_vga_memstart,
						g_invslot_borderxy_table[g_statuspage_target_slot].x,
						g_invslot_borderxy_table[g_statuspage_target_slot].y,
						g_invslot_borderxy_table[g_statuspage_target_slot].x + 17,
						g_invslot_borderxy_table[g_statuspage_target_slot].y + 17,
						8);
				}
			}

			call_mouse();

			g_request_refresh = force_redraw = 0;
		}

		handle_input();

		/* RIGHT_KEY */
		if ((g_action_id == KEY_SCAN_CODE_ARROW_RIGHT) && (gs_group_member_counts[gs_active_group_id] > 1)) {

			/* set hero_pos to the next possible hero */
			do {
				hero_pos++;

				if (hero_pos > 6) hero_pos = 0;

			} while (!(get_hero(hero_pos)->typus) ||
					(get_hero(hero_pos)->group_id != gs_active_group_id) ||
					((get_hero(hero_pos)->typus < HERO_TYPE_HEXE) && (g_status_page_mode > 3)));


			if (g_statuspage_target_slot != -1) {

				if (trade_mode == 0) {
					owner_hero = shown_hero;
				}

				shown_hero = get_hero(hero_pos);
				trade_mode = 1;
			} else {
				trade_mode = 0;

				owner_hero = shown_hero = get_hero(hero_pos);

				reset_item_selector();
			}

			g_request_refresh = 1;
		}

		/* LEFT_KEY */
		if ((g_action_id == KEY_SCAN_CODE_ARROW_LEFT) && (gs_group_member_counts[gs_active_group_id] > 1)) {

			/* set hero_pos to the previous possible hero */
			do {
				hero_pos--;

				if (hero_pos < 0) hero_pos = 6;

			} while (!(get_hero(hero_pos)->typus) ||
					(get_hero(hero_pos)->group_id != gs_active_group_id) ||
					((get_hero(hero_pos)->typus < HERO_TYPE_HEXE) && (g_status_page_mode > 3)));


			if (g_statuspage_target_slot != -1) {

				if (trade_mode == 0) {
					owner_hero = shown_hero;
				}

				shown_hero = get_hero(hero_pos);
				trade_mode = 1;
			} else {
				trade_mode = 0;

				owner_hero = shown_hero = get_hero(hero_pos);

				reset_item_selector();
			}

			g_request_refresh = 1;
		}

		if (g_status_page_mode < 3) {

			/* UP_KEY */
			if (g_action_id == KEY_SCAN_CODE_ARROW_UP) {

				if (g_statuspage_target_slot != -1) {

					if (!g_statuspage_target_slot) {
						g_statuspage_target_slot = HERO_INVENTORY_SLOT_MOUTH;
					} else {
						g_statuspage_target_slot--;
					}
				} else {
					if (g_statuspage_sel_slot == 0) {
						g_statuspage_sel_slot = HERO_INVENTORY_SLOT_KNAPSACK_16;
					} else {
						g_statuspage_sel_slot--;
					}
				}
			}

			/* DOWN_KEY */
			if (g_action_id == KEY_SCAN_CODE_ARROW_DOWN) {

				if (g_statuspage_target_slot != -1) {

					if (owner_hero != shown_hero) {

						if (g_statuspage_target_slot == HERO_INVENTORY_SLOT_MOUTH) {
							g_statuspage_target_slot = 0;
						} else {
							g_statuspage_target_slot++;
						}
					} else {
						if (g_statuspage_target_slot == HERO_INVENTORY_SLOT_MOUTH) {
							g_statuspage_target_slot = 0;
						} else {
							g_statuspage_target_slot++;
						}
					}
				} else {
					if (g_statuspage_sel_slot == HERO_INVENTORY_SLOT_KNAPSACK_16) {
						g_statuspage_sel_slot = 0;
					} else {
						g_statuspage_sel_slot++;
					}
				}
			}

			if ((g_action_id >= 128) && (g_action_id <= 152)) /* click on slot 0..22, eye (23) or mouth (24); slots 23/24 only as target */
			{
				if (g_statuspage_target_slot != -1) {
					g_statuspage_target_slot = g_action_id - 128;
					g_action_id = KEY_SCAN_CODE_ENTER;
				} else if (g_action_id <= 150) {
					g_statuspage_sel_slot = g_action_id - 128;
					g_action_id = KEY_SCAN_CODE_ENTER;
				}
			}

			if ((g_statuspage_sel_slot_drawn != g_statuspage_sel_slot) && (owner_hero == shown_hero)) {

				/* remove the old frame (background color 30) */
				do_border(g_vga_memstart,
					g_invslot_borderxy_table[g_statuspage_sel_slot_drawn].x,
					g_invslot_borderxy_table[g_statuspage_sel_slot_drawn].y,
					g_invslot_borderxy_table[g_statuspage_sel_slot_drawn].x + 17,
					g_invslot_borderxy_table[g_statuspage_sel_slot_drawn].y + 17,
					30);
				/* draw the red frame at the selected slot */
				do_border(g_vga_memstart,
					g_invslot_borderxy_table[g_statuspage_sel_slot].x,
					g_invslot_borderxy_table[g_statuspage_sel_slot].y,
					g_invslot_borderxy_table[g_statuspage_sel_slot].x + 17,
					g_invslot_borderxy_table[g_statuspage_sel_slot].y + 17,
					9);

				g_statuspage_sel_slot_drawn = g_statuspage_sel_slot;

				memset(g_dtp2, ' ', 60);
				*(g_dtp2 + 60) = '\0';
				GUI_print_string(g_dtp2, 16, 192);

				if (shown_hero->inventory[g_statuspage_sel_slot].item_id) {

					sprintf(g_dtp2, g_extraspace_separated_strings2,
						(char*)GUI_name_base_form(g_itemsname[shown_hero->inventory[g_statuspage_sel_slot].item_id]),
						!is_in_int_array(
						    shown_hero->inventory[g_statuspage_sel_slot].item_id,
						    g_forbidden_item_ids_table[shown_hero->typus - 1]) ? g_empty_string9 : get_tx2(66)); /* "(KANN NICHT BENUTZT WERDEN)" */

					if (g_itemsdat[owner_hero->inventory[g_statuspage_sel_slot].item_id].flags.weapon) {

						strcat(g_dtp2, get_ttx(48 + g_itemsdat[owner_hero->inventory[g_statuspage_sel_slot].item_id].item_subtype_id)); /* TEXT.LTX 48..56: Waffentalent */
					}

					GUI_print_string(g_dtp2, 16, 192);
				}
			}

			if (g_statuspage_target_slot_drawn != g_statuspage_target_slot && g_statuspage_target_slot != -1) {

				/* remove the old frame (background color 30) */
				do_border(g_vga_memstart,
					g_invslot_borderxy_table[g_statuspage_target_slot_drawn].x,
					g_invslot_borderxy_table[g_statuspage_target_slot_drawn].y,
					g_invslot_borderxy_table[g_statuspage_target_slot_drawn].x + 17,
					g_invslot_borderxy_table[g_statuspage_target_slot_drawn].y + 17,
					30);

				if (owner_hero == shown_hero) {

					/* draw the red frame at the selected slot */
					do_border(g_vga_memstart,
						g_invslot_borderxy_table[g_statuspage_sel_slot].x,
						g_invslot_borderxy_table[g_statuspage_sel_slot].y,
						g_invslot_borderxy_table[g_statuspage_sel_slot].x + 17,
						g_invslot_borderxy_table[g_statuspage_sel_slot].y + 17,
						9);
				}


				/* draw the yellow frame at the target slot */
				do_border(g_vga_memstart,
					g_invslot_borderxy_table[g_statuspage_target_slot].x,
					g_invslot_borderxy_table[g_statuspage_target_slot].y,
					g_invslot_borderxy_table[g_statuspage_target_slot].x + 17,
					g_invslot_borderxy_table[g_statuspage_target_slot].y + 17,
					8);

				g_statuspage_target_slot_drawn = g_statuspage_target_slot;
			}

			if (g_action_id == KEY_SCAN_CODE_ENTER) {
				if (g_statuspage_target_slot != -1) {

					if (trade_mode != 0) {

						if (g_statuspage_target_slot < 23) {
							pass_item(owner_hero, g_statuspage_sel_slot, shown_hero, g_statuspage_target_slot);
							g_request_refresh = 1;
						} else if (g_statuspage_target_slot == 23) { /* eye icon */
							print_item_description(owner_hero, g_statuspage_sel_slot);
						} else if (g_statuspage_target_slot == 24) { /* mouth icon */
							consume(owner_hero, shown_hero, g_statuspage_sel_slot);
						}

						/* remove the old frame (background color 30) */
						do_border(g_vga_memstart,
							g_invslot_borderxy_table[g_statuspage_target_slot].x,
							g_invslot_borderxy_table[g_statuspage_target_slot].y,
							g_invslot_borderxy_table[g_statuspage_target_slot].x + 17,
							g_invslot_borderxy_table[g_statuspage_target_slot].y + 17,
							30);

						trade_mode = 0;
						owner_hero = shown_hero;

					} else {
						if (g_statuspage_target_slot < 23) {
							move_item(g_statuspage_sel_slot, g_statuspage_target_slot, shown_hero);
							g_request_refresh = 1;
						} else if (g_statuspage_target_slot == HERO_INVENTORY_SLOT_EYE) { /* eye icon */
							print_item_description(shown_hero, g_statuspage_sel_slot);
						} else if (g_statuspage_target_slot == HERO_INVENTORY_SLOT_MOUTH) { /* mouth icon */
							consume(shown_hero, shown_hero, g_statuspage_sel_slot);
						}

						/* remove the old frame (background color 30) */
						do_border(g_vga_memstart,
							g_invslot_borderxy_table[g_statuspage_target_slot].x,
							g_invslot_borderxy_table[g_statuspage_target_slot].y,
							g_invslot_borderxy_table[g_statuspage_target_slot].x + 17,
							g_invslot_borderxy_table[g_statuspage_target_slot].y + 17,
							30);
					}

					reset_item_selector();
					force_redraw = 1;
				} else {

					g_statuspage_target_slot = HERO_INVENTORY_SLOT_EYE;

					/* draw the yellow frame at the target slot */
					do_border(g_vga_memstart,
						g_invslot_borderxy_table[g_statuspage_target_slot].x,
						g_invslot_borderxy_table[g_statuspage_target_slot].y,
						g_invslot_borderxy_table[g_statuspage_target_slot].x + 17,
						g_invslot_borderxy_table[g_statuspage_target_slot].y + 17,
						8);

					if (shown_hero->inventory[g_statuspage_sel_slot].item_id) {

						nvf.dst = g_icon;
						nvf.src = g_buffer10_ptr;
						nvf.compression_type = 0;
						nvf.width = &width;
						nvf.height = &height;
						nvf.image_num = g_itemsdat[shown_hero->inventory[g_statuspage_sel_slot].item_id].item_sprite_id;
						process_nvf_extraction(&nvf);

						make_ggst_cursor(g_icon);

						g_current_cursor = &g_ggst_cursor;
					}
				}
			}
		}

		/* check if the hero is diseased and print a message */
		if ((g_status_page_mode == 1) && (g_action_id == ACTION_ID_STATUS_DISEASE) && hero_is_diseased(shown_hero))
		{
			sprintf(g_dtp2, get_tx2(25 + hero_is_diseased(shown_hero)), shown_hero->alias, GUI_get_personal_pronoun(shown_hero->sex, GRAMMAR_CASE_2ND)); /* CHARTEXT.LTX 26..: Krankheitsbeschreibung */

			GUI_output(g_dtp2);
		}

		if (g_mouse_rightclick_event || g_action_id == KEY_SCAN_CODE_PAGE_UP) {

			g_current_cursor_bak = g_current_cursor;
			g_current_cursor = &g_default_mouse_cursor;

			switch (g_status_page_mode) {
			case 1: {
				/* from start-page */

				if (g_game_mode == GAME_MODE_ADVANCED) {

					g_radio_name_list[0] = get_tx2(19); /* "AT PA WERTE" */
					g_radio_name_list[1] = get_tx2(20); /* "TALENTE" */
					g_radio_name_list[2] = get_tx2(21); /* "ZAUBERFERT." */

					disease_id = hero_is_diseased(shown_hero);

					if (disease_id != 0) {
						g_radio_name_list[3] = get_tx2(25); /* "KRANKHEIT" */
						g_radio_name_list[4] = get_tx2(15); /* "ZURÜCK" */
						num_options = 10;
					} else {
						g_radio_name_list[3] = get_tx2(15); /* "ZURÜCK" */
						num_options = 9;
					}
				} else {

					if ((disease_id = hero_is_diseased(shown_hero))) {
						g_radio_name_list[0] = get_tx2(25); /* "KRANKHEIT" */
						g_radio_name_list[1] = get_tx2(15); /* "ZURÜCK" */
						num_options = 7;
					} else {
						g_radio_name_list[0] = get_tx2(15); /* "ZURÜCK" */
						num_options = 6;
					}
				}

				answer = GUI_radio(NULL, (signed char)num_options,
						get_tx2(69), get_tx2(16), get_tx2(17), get_ttx(212), get_ttx(213), /* "NAME ÄNDERN" / "GGST. BENUTZEN" / "GGST. WEGWERFEN" / "TALENT ANWENDEN" / "ZAUBER SPRECHEN" */
						g_radio_name_list[0], g_radio_name_list[1], g_radio_name_list[2],
						g_radio_name_list[3], g_radio_name_list[4]);

				if (answer != -1) {
					switch (answer) {
					case 1: {
						/* change name */
						if (shown_hero->npc_id != 0) {
							GUI_output(get_tx2(71)); /* "DEN NAMEN EINES NPC KANN MAN NICHT ÄNDERN!" */
						} else {
							GUI_input(get_tx2(70), 15); /* "NEUEN NAMEN EINGEBEN:" */
							strcpy(shown_hero->alias, g_text_input_buf);
							g_request_refresh = 1;
						}
						break;
					}
					case 2: {
						/* use item */
						if (owner_hero != shown_hero) {
							GUI_output(get_tx2(68)); /* "LEIDER NICHT IM HANDELSMODUS MÖGLICH!" */
						} else {
							use_item(g_statuspage_sel_slot, hero_pos);
							reset_item_selector();
							g_request_refresh = 1;
						}
						break;
					}
					case 3: {
						/* drop item */
						if (owner_hero != shown_hero) {
							GUI_output(get_tx2(68)); /* "LEIDER NICHT IM HANDELSMODUS MÖGLICH!" */
						} else {
							drop_item(shown_hero, g_statuspage_sel_slot, -1);
							reset_item_selector();
							g_request_refresh = 1;
						}
						break;
					}
					case 4: {
						/* use talent */
						g_forbid_ta_falschspiel = 1;
						GUI_use_talent(hero_pos, 0);
						g_forbid_ta_falschspiel = 0;

						if (g_tx_file_index == 19) { /* 19 = ARCHIVE_FILE_MAPTEXT_LTX */
							load_tx2(ARCHIVE_FILE_CHARTEXT_LTX);
						}
						break;
					}
					case 5: {
						/* cast spell */
						if (select_magic_user() != -2) {
							load_tx2(ARCHIVE_FILE_CHARTEXT_LTX);
							reset_item_selector();
						}
						g_request_refresh = 1;
						break;
					}
					case 6: {
						/* novice mode: ZURUECK or KRANKHEIT; advanced mode: show ATPA values */
						if (num_options == 6) {
							done = 1;
							reset_item_selector();
						} else if (num_options == 7) {
							sprintf(g_dtp2, get_tx2(25 + disease_id), shown_hero->alias, GUI_get_personal_pronoun(shown_hero->sex, GRAMMAR_CASE_2ND)); /* CHARTEXT.LTX 26..: Krankheitsbeschreibung */
							GUI_output(g_dtp2);
						} else {
							g_status_page_mode = 2;
							g_request_refresh = 1;
						}
						break;
					}
					case 7: {
						/* novice mode (with disease): ZURUECK; advanced mode: show talents */
						if (num_options == 7) {
							done = 1;
							reset_item_selector();
						} else {
							reset_item_selector();
							g_status_page_mode = 3;
							g_request_refresh = 1;
						}
						break;
					}
					case 8: {
						/* show spells */
						if (shown_hero->typus < HERO_TYPE_HEXE) {
							/* not a spellcaster */
							GUI_output(get_ttx(215)); /* "DIESER HELD KANN NICHT ZAUBERN!" */
						} else {
							reset_item_selector();
							g_status_page_mode = 4;
							g_request_refresh = 1;
						}
						break;
					}
					case 9: { /* advanced: KRANKHEIT if diseased, otherwise ZURUECK (falls through into case 10) */
						if (disease_id) {

							sprintf(g_dtp2, get_tx2(25 + disease_id), shown_hero->alias, GUI_get_personal_pronoun(shown_hero->sex, GRAMMAR_CASE_2ND)); /* CHARTEXT.LTX 26..: Krankheitsbeschreibung */
							GUI_output(g_dtp2);

							break;
						}
					}
					case 10: {
						done = 1;
						reset_item_selector();
						break;
					}
					}
				}

				break;
			}
			case 2: {
				/* from ATPA-page */

				answer = GUI_radio((char*)NULL, 8,
						get_tx2(16), get_tx2(17), get_ttx(212),	get_ttx(213), /* "GGST. BENUTZEN" / "GGST. WEGWERFEN" / "TALENT ANWENDEN" / "ZAUBER SPRECHEN" */
						get_tx2(24), get_tx2(20), get_tx2(21), get_tx2(15)); /* "GRUNDWERTE" / "TALENTE" / "ZAUBERFERT." / "ZURÜCK" */

				if (answer != -1) {
					switch (answer) {
					case 1: {
						/* use item */
						if (owner_hero != shown_hero) {
							GUI_output(get_tx2(68)); /* "LEIDER NICHT IM HANDELSMODUS MÖGLICH!" */
						} else {
							use_item(g_statuspage_sel_slot, hero_pos);
							reset_item_selector();
						}
						break;
					}
					case 2: {
						/* drop item */
						if (owner_hero != shown_hero) {
							GUI_output(get_tx2(68)); /* "LEIDER NICHT IM HANDELSMODUS MÖGLICH!" */
						} else {
							drop_item(shown_hero, g_statuspage_sel_slot, -1);
							reset_item_selector();
							g_request_refresh = 1;
						}
						break;
					}
					case 3: {
						/* use talent */
						g_forbid_ta_falschspiel = 1;
						GUI_use_talent(hero_pos, 0);
						g_forbid_ta_falschspiel = 0;

						if (g_tx_file_index == 19) { /* 19 = ARCHIVE_FILE_MAPTEXT_LTX */
							load_tx2(ARCHIVE_FILE_CHARTEXT_LTX);
						}
						break;
					}
					case 4: {
						/* cast spell */
						if (select_magic_user() != -2) {
							load_tx2(ARCHIVE_FILE_CHARTEXT_LTX);
							reset_item_selector();
							g_request_refresh = 1;
						}
						break;
					}
					case 5:
						g_status_page_mode = 1;
						goto refresh1;
					case 6:
						reset_item_selector();
						g_status_page_mode = 3;
refresh1:
						g_request_refresh = 1;
						break;
					case 7: {
						if (shown_hero->typus < HERO_TYPE_HEXE) {
							/* not a spellcaster */
							GUI_output(get_ttx(215)); /* "DIESER HELD KANN NICHT ZAUBERN!" */
						} else {
							reset_item_selector();
							g_status_page_mode = 4;
							g_request_refresh = 1;
						}
						break;
					}
					case 8: {
						done = 1;
						reset_item_selector();
						break;
					}
					}
				}
				break;
			}
			case 3: {
				/* from talents-page */
				answer = GUI_radio((char*)NULL, 6, get_ttx(212), get_tx2(24), get_tx2(19), get_tx2(21), get_ttx(213), get_tx2(15)); /* "TALENT ANWENDEN" / "GRUNDWERTE" / "AT PA WERTE" / "ZAUBERFERT." / "ZAUBER SPRECHEN" / "ZURÜCK" */

				if (answer != -1) {
					switch (answer) {
					case 1: {
						/* use talent */
						g_forbid_ta_falschspiel = 1;
						GUI_use_talent(hero_pos, 0);
						g_forbid_ta_falschspiel = 0;

						if (g_tx_file_index == 19) { /* 19 = ARCHIVE_FILE_MAPTEXT_LTX */
							load_tx2(ARCHIVE_FILE_CHARTEXT_LTX);
						}
						break;
					}
					case 2:
						g_status_page_mode = 1;
						goto refresh2;
					case 3:
						g_status_page_mode = 2;
refresh2:
						g_request_refresh = 1;
						break;
					case 4: {
						if (shown_hero->typus < HERO_TYPE_HEXE) {
							/* not a spellcaster */
							GUI_output(get_ttx(215)); /* "DIESER HELD KANN NICHT ZAUBERN!" */
						} else {
							g_status_page_mode = 4;
							g_request_refresh = 1;
						}
						break;
					}
					case 5: {
						/* cast spell */
						if (select_magic_user() != -2) {
							load_tx2(ARCHIVE_FILE_CHARTEXT_LTX);
							g_request_refresh = 1;
						}
						break;
					}
					case 6: {
						done = 1;
						break;
					}
					}
				}
				break;
			}
			case 4:
			case 5:{
				/* from spells-page */
				answer = GUI_radio((char*)NULL, 6,
						get_ttx(213),    // Zauber sprechen
						get_tx2(24),     // Grundwerte
						get_tx2(19),     // AT PA Werte
						get_tx2(20),     // Talente
						get_ttx(214),    // Mehr Zauber
						get_tx2(15));    // Zurueck

				if (answer != -1) {
					switch (answer) {
					case 1: {
						/* cast spell */
						if (select_magic_user() != -2) {
							load_tx2(ARCHIVE_FILE_CHARTEXT_LTX);
							g_request_refresh = 1;
						}
						break;
					}
					case 2:
						g_status_page_mode = 1;
						goto refresh3;
					case 3:
						g_status_page_mode = 2;
						goto refresh3;
					case 4:
						g_status_page_mode = 3;
						goto refresh3;
					case 5:
						if (g_status_page_mode == 4)
							g_status_page_mode = 5;
						else
							g_status_page_mode = 4;
refresh3:
						g_request_refresh = 1;
						break;
					case 6: {
						done = 1;
						break;
					}
					}
				}
				break;
			}
			}

			g_current_cursor = g_current_cursor_bak;
		}
	}

	/* restore text file except for CHARTEXT.LTX, TAVERN.TLK and except for dialogs */
	if ((file_bak != -1) &&	(file_bak != ARCHIVE_FILE_CHARTEXT_LTX) &&
		(file_bak != ARCHIVE_FILE_TAVERN_TLK) && (file_bak < 156 || file_bak > 176))
	{
		load_tx2(file_bak);
	}

	g_request_refresh = 1;
	g_textbox_width = tw_bak;
	g_timers_disabled--;

	if (gs_town_id != TOWN_ID_NONE) {
		g_fading_state = 3;
	}
}

/**
 * \brief   asks for a hero of the group and opens his status menu
 */
/* Borlandified and identical */
void status_select_hero(void)
{

	if (!g_statusmenu_allowed || !gs_group_member_counts[gs_active_group_id]) {
		/* Yes, it was written that way! */
	} else {
		const signed int hero_pos = select_hero_from_group(get_ttx(301)); /* "WELCHEN HELDEN ANSEHEN?" */

		if (hero_pos != -1) {
			status_menu(hero_pos);
		}
	}
}
