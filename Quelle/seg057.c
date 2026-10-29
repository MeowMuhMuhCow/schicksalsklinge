/*
 * seg057 - Haendler: Verkauf (sell_screen)
 *
 * Inhalt:    sell_screen() zeigt das Gepaeck eines Helden mit den Ankaufpreisen des Haendlers.
 *            Gegenstaende (bei Stapeln eine Menge) werden fuer den Verkauf vorgemerkt, auch
 *            von mehreren Helden der Gruppe. Beim Abschluss kann um 1-50 % Aufschlag
 *            gefeilscht werden. Jede 10. verkaufte Ware einer Sorte senkt ihren Grundpreis
 *            in g_itemsdat um 10 %.
 * Aufrufer:  do_merchant() (seg055).
 * Daten:     nur lokale Formatstrings und Bildschirmpositionen.
 * Besonders: Der Erloes wird mit g_price_modificator / 4 multipliziert. Ein erfolgreiches
 *            Gespraech mit dem Haendler setzt den Wert auf 3 und senkt damit den Erloes auf
 *            75 %, statt den Einkauf zu verbilligen (TLK_khandel/TLK_whandel, seg055).
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg057.c
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
#include "seg029.h"
#include "seg047.h"
#include "seg056.h"
#include "seg096.h"
#include "seg097.h"
#include "seg103.h"
#include "seg104.h"
#include "seg105.h"

static struct c_str_5 g_sell_screen_str_money_h = { "H %d" }; // ds:0x6bf0
static struct c_str_5 g_sell_screen_str_money_s = { "S %d" }; // ds:0x6bf5
static struct c_str_5 g_sell_screen_str_money_d = { "D %d" }; // ds:0x6bfa
static struct int16_t_3 g_sell_screen_items_posx = { 30, 95, 160 }; // ds:0x6bff
static struct int16_t_5 g_sell_screen_items_posy = { 35, 55, 75, 95, 115 }; // ds:0x6c05

/**
 * \brief   shows the sell-screen and provides interaction
 *
 * \param   merchant_descr  the merchant (price_mod and type decide the price)
 *
 * Items of several heroes of the active group can be marked (item_table[hero_pos][inv_slot]
 * holds the number to sell). Sell price per item: g_item_selector_sell[].price * price_unit
 * * g_price_modificator / 4.
 */
void sell_screen(struct merchant_descr *merchant_descr)
{
	signed int tmp; /* multi use: inv_slot, y-coordinate for table, and others. */
	signed int j;
	signed int items_x;
	signed int fg_bak;
	signed int bg_bak;
	signed int answer;
	signed int num_items; /* multi use: quantity shown, quantity to sell, total number of items for bargain() */
	signed int percent;
	signed int percent_old = 100;
	signed int item_id;
	signed int item_selector_pos_old;
	signed int item_selector_pos;
	signed int done = 0;
	signed int item_selector_page_offset = 0;
	signed int refresh_items; /* redraw the item field */
	signed int num_filled_inv_slots;
	signed int choose_hero = 1; /* ask which hero wants to sell */
	signed int refresh_hero = 1; /* hero changed or items sold: rebuild g_item_selector_sell */
	signed int hero_pos_old = 1;
	signed int hero_pos = -1;
	struct c_str_5 fmt_h = g_sell_screen_str_money_h;
	struct c_str_5 fmt_s = g_sell_screen_str_money_s;
	struct c_str_5 fmt_d = g_sell_screen_str_money_d;
	struct int16_t_3 array3 = g_sell_screen_items_posx;
	//struct dummy3 array3 = { { 30, 95, 160 } };
	struct int16_t_5 array5 = g_sell_screen_items_posy;
	//struct dummy5 array5 = { { 35, 55, 75, 95, 115 } };
	int32_t p_money;
	int32_t price = 0;
	signed int deal = 0; /* 0 = no deal yet, 1 = bargaining successful, 2 = no surcharge wanted */
	struct struct_hero *hero1;
	struct struct_hero *hero2;
	struct struct_hero *hero3;
	signed int width;
	signed int height;
	signed int inv_slot;
	struct nvf_extract_desc nvf;
	signed int item_table[7][NR_HERO_INVENTORY_SLOTS];


	g_heroswap_allowed = 0;
	refresh_items = g_request_refresh = 1;
	g_item_selector_sell = (struct item_selector_item*)(g_fig_figure1_buf + 2100);

	while (done == 0) {

		if (g_request_refresh != 0) {

			disable_ani();
			g_pp20_index = -1;
			memset((uint8_t*)g_item_selector_sell, 0, 300 * sizeof(struct item_selector_item));

			for (items_x = 0; items_x <= 6; items_x++) {
				for (tmp = 0; tmp < NR_HERO_INVENTORY_SLOTS; tmp++) {
					item_table[items_x][tmp] = 0;
				}
			}

			price = 0;
			draw_loc_icons(5, MENU_ICON_BARGAIN, MENU_ICON_SCROLL_RIGHT, MENU_ICON_SCROLL_LEFT, MENU_ICON_HERO, MENU_ICON_LEAVE);
			draw_main_screen();

			/* ICONS */
			tmp = load_archive_file(ARCHIVE_FILE_ICONS);
			seek_archive_file(tmp, 24 * 576L);
			read_archive_file(tmp, g_icon, 576L);
			close(tmp);

			g_pic_copy.x1 = 108;
			g_pic_copy.y1 = 5;
			g_pic_copy.x2 = 131;
			g_pic_copy.y2 = 28;
			g_pic_copy.src = g_icon;
			do_pic_copy(0);

			get_textcolor(&fg_bak, &bg_bak);
			set_textcolor(255, 0);

			item_selector_pos_old = -1;
			item_selector_pos = 0;
			choose_hero = 1;
			g_request_refresh = 0;
		}

		if (refresh_items != 0 || choose_hero != 0 || refresh_hero != 0) {

			/* refresh goods */

			if (choose_hero != 0) {

				hero_pos = select_hero_ok(get_ttx(448)); /* "WER MÖCHTE ETWAS VERKAUFEN?" */

				if (hero_pos == -1 || (get_hero(hero_pos)->num_filled_inv_slots == 0)) {
					break;
				}

				hero1 = get_hero(hero_pos);

				deselect_hero_icon(hero_pos_old);
				select_hero_icon(hero_pos);
				hero_pos_old = hero_pos;
				choose_hero = 0;
				refresh_hero = 1;

			}

			if (refresh_hero != 0) {

				num_filled_inv_slots = 0;
				for (tmp = 0; tmp < NR_HERO_INVENTORY_SLOTS; tmp++) {
					if (hero1->inventory[tmp].item_id != ITEM_ID_NONE) {
						add_item_to_sell_selector(merchant_descr, hero1, tmp, num_filled_inv_slots++);
					}
				}
				// assert(num_filled_inv_slots == hero1->num_filled_inv_slots)

				for (tmp = num_filled_inv_slots; tmp < NR_HERO_INVENTORY_SLOTS; tmp++) {
					g_item_selector_sell[tmp].item_id = 0;
				}

				refresh_hero = 0;
				item_selector_pos_old = -1;
				item_selector_pos = item_selector_page_offset = deal = 0;
				percent_old = 100;
				p_money = get_party_money();

				do_fill_rect(g_vga_memstart, 26, 26, 105, 33, 0);

				make_valuta_str(g_dtp2, p_money);
				GUI_print_string(g_dtp2, 104 - GUI_get_space_for_string(g_dtp2, 0), 26);
			}

			call_mouse_bg();

			do_fill_rect(g_vga_memstart, 29, 34, 214, 133, 0);

			do_v_line(g_vga_memstart, 87, 35, 131, -1);
			do_v_line(g_vga_memstart, 152, 35, 131, -1);

			nvf.dst = g_renderbuf_ptr;
			nvf.src = g_buffer10_ptr;
			nvf.compression_type = 0;
			nvf.width =  &width;
			nvf.height = &height;

			for (items_x = 0; items_x < 3; items_x++) {

				for (tmp = 0; tmp < 5; tmp++) {

					answer = 5 * items_x + tmp + item_selector_page_offset;

					if ((j = g_item_selector_sell[answer].item_id)) {

						g_pic_copy.x1 = array3.a[items_x];
						g_pic_copy.y1 = array5.a[tmp];
						g_pic_copy.x2 = array3.a[items_x] + 15;
						g_pic_copy.y2 = array5.a[tmp] + 15;
						g_pic_copy.src = g_renderbuf_ptr;

						nvf.image_num = g_itemsdat[j].item_sprite_id;

						process_nvf_extraction(&nvf);

						do_pic_copy(0);

						if (g_itemsdat[j].flags.stackable) {

							if ((num_items = hero1->inventory[g_item_selector_sell[answer].inv_slot].quantity) > 1)
							{
								my_itoa(num_items, g_dtp2, 10);

								GUI_print_string(g_dtp2,
									array3.a[items_x] + 16 - GUI_get_space_for_string(g_dtp2, 0),
									array5.a[tmp] + 9);

								if (item_table[hero_pos][g_item_selector_sell[answer].inv_slot])
								{
									set_textcolor(201, 0);
									my_itoa(item_table[hero_pos][g_item_selector_sell[answer].inv_slot], g_dtp2, 10);

									GUI_print_string(g_dtp2,
										array3.a[items_x] + 16 - GUI_get_space_for_string(g_dtp2, 0),
										array5.a[tmp] + 1);

									set_textcolor(255, 0);
								}
							}
						}

						sprintf(g_dtp2,	g_item_selector_sell[answer].price_unit == 1 ? fmt_h.a :
								(g_item_selector_sell[answer].price_unit == 10 ? fmt_s.a : fmt_d.a),
									g_item_selector_sell[answer].price);

						if (item_table[hero_pos][g_item_selector_sell[answer].inv_slot]) {

							set_textcolor(201, 0);
						}

						GUI_print_string(g_dtp2, array3.a[items_x] + 20, array5.a[tmp] + 5);
						set_textcolor(255, 0);
					}
				}
			}

			do_fill_rect(g_vga_memstart, 135, 26, 214, 33, 0);

			make_valuta_str(g_dtp2, price);
			GUI_print_string(g_dtp2, 135, 26);

			item_selector_pos_old = -1;

			call_mouse();
			refresh_items = 0;
		}

		if (g_have_mouse == 2) {
			item_selector_mouse_select(&item_selector_pos, &g_item_selector_sell[item_selector_page_offset]);
		} else {
			item_selector_keyboard_select(&item_selector_pos, &g_item_selector_sell[item_selector_page_offset]);
		}

		g_action_table_secondary = &g_action_table_merchant[0];
		handle_gui_input();
		g_action_table_secondary = NULL;

		if (item_selector_pos_old != item_selector_pos) {

			if (item_selector_pos_old != -1) {
				do_border(g_vga_memstart,
					array3.a[item_selector_pos_old / 5] - 1,
					array5.a[item_selector_pos_old % 5] - 1,
					array3.a[item_selector_pos_old / 5] + 16,
					array5.a[item_selector_pos_old % 5] + 16,
					0);
			 }

			do_border(g_vga_memstart,
				array3.a[item_selector_pos / 5] - 1,
				array5.a[item_selector_pos % 5] - 1,
				array3.a[item_selector_pos / 5] + 16,
				array5.a[item_selector_pos % 5] + 16,
				-1);

			item_selector_pos_old = item_selector_pos;

			clear_loc_line();

			GUI_print_loc_line(GUI_name_base_form(g_itemsname[g_item_selector_sell[item_selector_pos + item_selector_page_offset].item_id]));
		}

		if (g_mouse_rightclick_event  || g_action_id == KEY_SCAN_CODE_PAGE_UP) {

			answer = GUI_radio(NULL, 5, get_ttx(433), get_ttx(435), get_ttx(436), get_ttx(446), get_ttx(437)) - 1; /* "FEILSCHEN" / "NÄCHSTE SEITE" / "VORHERIGE SEITE" / "ANDERER HELD" / "ABBRECHEN" */

			if (answer != -2) {
				g_action_id = answer + ACTION_ID_ICON_1;
			}
		}

		if (g_action_id == ACTION_ID_ICON_3 && item_selector_page_offset) {
			refresh_items = 1;
			item_selector_page_offset -= 15;
		} else if (g_action_id == ACTION_ID_ICON_2 && g_item_selector_sell[item_selector_page_offset + 15].item_id) {
			refresh_items = 1;
			item_selector_page_offset += 15;
		}

		if (g_action_id == ACTION_ID_DECREASE_ITEM_COUNT_BY_RIGHT_CLICK || g_action_id == KEY_SCAN_CODE_ENTER) {

			/* Is ACTION == ACTION_ID_DECREASE_ITEM_COUNT_BY_RIGHT_CLICK possible at all?
			 * ACTION_ID_DECREASE_ITEM_COUNT_BY_RIGHT_CLICK can be written to ACTION in buy_screen(), but where should it show up in sell_screen()??
			 * Answer: no. It is only set in buy_screen(), and handle_gui_input() resets g_action_id. */

			item_id = g_item_selector_sell[item_selector_pos + item_selector_page_offset].item_id;

			if (g_item_selector_sell[item_selector_pos + item_selector_page_offset].price == 0) {

				GUI_output(get_ttx(452)); /* "<TUT MIR LEID, SOWAS FÜHRE ICH IN MEINEM LADEN NICHT!>" */

			} else {

				if (g_itemsdat[item_id].flags.undroppable) {

					sprintf(g_dtp2,	get_ttx(454), (char*)GUI_name_inflect_with_article( /* "HEY, DU WIRST DOCH NICHT SO VERRÜCKT SEIN UND %s AUS DER HAND GEBEN..." */
						INFLECT_DEFINITE_ARTICLE | INFLECT_SINGULAR | INFLECT_4TH_CASE,
						item_id,
						INFLECT_NAME_TYPE_ITEM
					));
					GUI_output(g_dtp2);

				} else {

					/* Original-Bug? g_price_modificator is 3 after a successful talk with the merchant
					 * ("ICH MACHE EUCH EINEN BESONDERS GUTEN PREIS"), which lowers the sell price
					 * below to 75%. buy_screen() does not use it at all. */
					num_items = 1;
					inv_slot = g_item_selector_sell[item_selector_pos + item_selector_page_offset].inv_slot;

					if (item_table[hero_pos][inv_slot] != 0) {

						if (g_itemsdat[item_id].flags.stackable && (hero1->inventory[inv_slot].quantity > 1)) {

							sprintf(g_dtp2,	get_ttx(447), (char*)GUI_name_inflect_with_article( /* "WIE VIELE %s WOLLT IHR VERKAUFEN?" */
								INFLECT_INDEFINITE_ARTICLE | INFLECT_PLURAL | INFLECT_1ST_CASE,
								item_id,
								INFLECT_NAME_TYPE_ITEM
							));

							num_items = GUI_input(g_dtp2, 2);

							if (num_items < 0) {
								num_items = 0;
							}

							if (hero1->inventory[inv_slot].quantity < num_items) {
								num_items = hero1->inventory[inv_slot].quantity;
							}

							price -= ((int32_t)g_item_selector_sell[item_selector_pos + item_selector_page_offset].price *
									(int32_t)g_item_selector_sell[item_selector_pos + item_selector_page_offset].price_unit *
									item_table[hero_pos][inv_slot] * g_price_modificator) / 4L;

							item_table[hero_pos][inv_slot] = num_items;

							price += ((int32_t)g_item_selector_sell[item_selector_pos + item_selector_page_offset].price *
									(int32_t)g_item_selector_sell[item_selector_pos + item_selector_page_offset].price_unit *
									item_table[hero_pos][inv_slot] * g_price_modificator) / 4L;
						} else {
							item_table[hero_pos][inv_slot] = 0;

							price -= ((int32_t)g_item_selector_sell[item_selector_pos + item_selector_page_offset].price *
									(int32_t)g_item_selector_sell[item_selector_pos + item_selector_page_offset].price_unit *
									num_items * g_price_modificator) / 4L;
						}
					} else {
						if (g_itemsdat[item_id].flags.stackable && (hero1->inventory[inv_slot].quantity > 1)) {

							sprintf(g_dtp2,	get_ttx(447), (char*)GUI_name_inflect_with_article( /* "WIE VIELE %s WOLLT IHR VERKAUFEN?" */
								INFLECT_INDEFINITE_ARTICLE | INFLECT_PLURAL | INFLECT_1ST_CASE,
								item_id,
								INFLECT_NAME_TYPE_ITEM
							));

							num_items = GUI_input(g_dtp2, 2);

							if (num_items < 0) {
								num_items = 0;
							}

							if (hero1->inventory[inv_slot].quantity < num_items) {
								num_items = hero1->inventory[inv_slot].quantity;
							}

							price -= ((int32_t)g_item_selector_sell[item_selector_pos + item_selector_page_offset].price *
									(int32_t)g_item_selector_sell[item_selector_pos + item_selector_page_offset].price_unit *
									item_table[hero_pos][inv_slot] * g_price_modificator) / 4L;

							item_table[hero_pos][inv_slot] = num_items;

							price += ((int32_t)g_item_selector_sell[item_selector_pos + item_selector_page_offset].price *
									(int32_t)g_item_selector_sell[item_selector_pos + item_selector_page_offset].price_unit *
									item_table[hero_pos][inv_slot] * g_price_modificator) / 4L;
						} else {
							item_table[hero_pos][inv_slot] = 1;
							price += ((int32_t)g_item_selector_sell[item_selector_pos + item_selector_page_offset].price *
									(int32_t)g_item_selector_sell[item_selector_pos + item_selector_page_offset].price_unit *
									num_items * g_price_modificator) / 4L;
						}
					}
				}
			}
			refresh_items = 1;
		}


		if (g_action_id == ACTION_ID_ICON_1 && price) {

			j = num_items = 0;
			for (items_x = 0; items_x <= 6; items_x++) {
				for (tmp = 0; tmp < NR_HERO_INVENTORY_SLOTS; tmp++) {
					num_items += item_table[items_x][tmp];
				}
			}

			/* 0xe83 - 11ba */
			while (deal == 0 && j < 3) {

				make_valuta_str(g_text_output_buf, price);

				sprintf(g_dtp2, get_ttx(449), g_text_output_buf); /* "<ICH KÖNNTE EUCH@%s@FÜR ALLES ZAHLEN>.@WIEVIEL PROZENT (0-50) MEHR@..." */

				do {
					percent = GUI_input(g_dtp2, 2);

				} while (percent > 50);

				if (percent == 0) {
					GUI_output(get_ttx(440)); /* "<ES FREUT MICH IMMER WIEDER, MIT EUCH GESCHÄFTE ZU MACHEN!>" */
					deal = 2;
				} else if (percent >= percent_old) {
					j = 2;
				} else if (percent < 0) {
					break;
				} else {
					g_talented_hero_pos = get_talented_hero_pos(TA_FEILSCHEN);

					answer = select_hero_ok_forced(get_ttx(442)); /* "WER SOLL FEILSCHEN?" */

					hero2 = get_hero(answer);

					deal = bargain(hero2, num_items, price, percent, 0) > 0 ? 1 : 0;
				}

				if (deal > 0) {

					hero3 = get_hero(0);
					for (items_x = 0; items_x <= 6; items_x++, hero3++) {

						for (tmp = 0; tmp < NR_HERO_INVENTORY_SLOTS; tmp++) {

							if (item_table[items_x][tmp] != 0) {

								item_id = hero3->inventory[tmp].item_id;

								drop_item(hero3, tmp, item_table[items_x][tmp]);

								g_market_itemsaldo_table[item_id] = g_market_itemsaldo_table[item_id] - item_table[items_x][tmp];

								if (g_market_itemsaldo_table[item_id] <= -10) {
									g_market_itemsaldo_table[item_id] = 0;

									/* REMARK: write to g_itemsdat ? Yes: the base price drops by 10% for every 10 sold items
									 * (until the program ends, g_itemsdat is not saved). */
									g_itemsdat[item_id].price -= g_itemsdat[item_id].price * 10 / 100;
								}
							}
						}
					}

					price = price + percent * price / 100L;

					if (price > 0) {
						add_party_money(price);
					}

					if (deal != 2) {
						GUI_output(get_ttx(451)); /* "<BEI BORON, IHR BRINGT MICH NOCH INS GRAB.@ABER GUT, HIER HABT IHR..." */

						make_valuta_str(g_text_output_buf, price);

						sprintf(g_dtp2, get_ttx(819), g_text_output_buf); /* "DAS MACHT %s." */
						GUI_output(g_dtp2);
					}

					refresh_hero = 1;
					price = 0;

					for (items_x = 0; items_x <= 6; items_x++) {
						for (tmp = 0; tmp < NR_HERO_INVENTORY_SLOTS; tmp++) {
							item_table[items_x][tmp] = 0;
						}
					}
					continue;
				}

				if (j == 2) {
					GUI_output(get_ttx(453)); /* "<ZU SO EINEM WUCHERPREIS KÖNNT IHR GERNE JEMAND ANDEREN SUCHEN,@DER..." */
					gs_merchant_offended_flags[gs_town_typeindex] = 1;
					done = 1;
				} else {
					GUI_output(get_ttx(450)); /* "<PHEX SEI MIR GNÄDIG!@ICH LASSE MICH VON EUCH DOCH NICHT AUSPLÜNDER..." */
					percent_old = percent;
				}

				j++;
			}
		}

		if (g_action_id >= ACTION_ID_HERO_1 && g_action_id <= ACTION_ID_HERO_NPC) {

			hero_pos = g_action_id - ACTION_ID_HERO_1;
			hero3 = get_hero(hero_pos);

			if ((hero3->typus != HERO_TYPE_NONE) && (hero3->group_id == gs_active_group_id))
			{
				hero1 = get_hero(hero_pos);
				deselect_hero_icon(hero_pos_old);
				select_hero_icon(hero_pos);
				hero_pos_old = hero_pos;
				refresh_hero = 1;
			}
		}

		if (g_action_id == ACTION_ID_ICON_4) {
			choose_hero = 1;
		}

		if (g_action_id == ACTION_ID_ICON_5) {
			done = 1;
		}

		if (hero1->num_filled_inv_slots == 0) {
			done = 1;
		}
	}

	set_textcolor(fg_bak, bg_bak);
	g_request_refresh = 1;
	g_pp20_index = -1;
	g_heroswap_allowed = 1;
}
