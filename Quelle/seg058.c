/*
 * seg058 - Schmied: Reparatur und Schmiedgespraech
 *
 * Inhalt:    do_smith() ist die Schmiede (6-20 Uhr) mit Reden, Reparieren und Verlassen.
 *            repair_screen() zeigt das Gepaeck eines Helden mit Reparaturpreisen (zerbrochene
 *            Waffe/Ruestung halber Kaufpreis, beschaedigte Ruestung ein Viertel), laesst
 *            um 0-50 % Rabatt feilschen und nimmt den Gegenstand an; er ist nach 6 Stunden
 *            bzw. am naechsten Morgen abholbar. Jeder Schmied repariert nur ein Stueck zur Zeit.
 *            TLK_schmied() steuert das Gespraech, darunter den Weg vom Thorwaler Schmied in
 *            die Zwingfeste.
 * Aufrufer:  Ortstabelle g_location_handlers (seg025); TLK_schmied() aus seg030.
 * Daten:     g_smith_descr_table[42] (Preisaufschlag, Qualitaet - unbenutzt),
 *            g_price_modificator (4 = normal, 3 = guter Preis; auch fuer seg055/seg057).
 * Besonders: repair_screen() zeigt das Geld des gewaehlten Helden, bezahlt wird aber aus der
 *            Gruppenkasse. Bei 0 % Rabatt und genau passendem Geld wird trotzdem
 *            gefeilscht (p_money > price statt >=).
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg058.c
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
#include "seg025.h"
#include "seg026.h"
#include "seg027.h"
#include "seg029.h"
#include "seg031.h"
#include "seg047.h"
#include "seg056.h"
#include "seg058.h"
#include "seg075.h"
#include "seg096.h"
#include "seg097.h"
#include "seg103.h"
#include "seg104.h"
#include "seg105.h"

static struct smith_descr g_smith_descr_table[42] = { /* 'quality' is apparently unused. */
	{  50,  1 },
	{  20,  4 },
	{   0, 10 },
	{ -20, 15 },
	{   0, 10 },
	{  30, 12 },
	{ -10,  9 },
	{  30, 10 },
	{  20,  2 },
	{   0,  9 },
	{ -40, 16 },
	{   0,  9 },
	{   0, 11 },
	{  50,  1 },
	{  70,  1 },
	{  10,  4 },
	{ -20,  9 },
	{  40,  6 },
	{ -10, 15 },
	{   0,  5 },
	{  50,  4 },
	{  10, 16 },
	{  30,  3 },
	{ -30, 11 },
	{  20,  1 },
	{ -50, 14 },
	{   0,  9 },
	{   0,  5 },
	{  20, 17 },
	{  10, 13 },
	{   0,  3 },
	{  10, 12 },
	{  30,  4 },
	{   0, 11 },
	{  10,  6 },
	{ -20, 13 },
	{  20,  2 },
	{   0, 13 },
	{ -10, 15 },
	{  30,  4 },
	{  30,  4 },
	{  -1,  0 }
}; // ds:0x6c10
static struct c_str_5 g_smith_str_money_h = { "H %d" }; // ds:0x6c64
static struct c_str_5 g_smith_str_money_s = { "S %d" }; // ds:0x6c69
static struct c_str_5 g_smith_str_money_d = { "D %d" }; // ds:0x6c6e
static struct int16_t_3 g_smith_items_posx = { 30, 95, 160 }; // ds:0x6c73
static struct int16_t_5 g_smith_items_posy = { 35, 55, 75, 95, 115 }; // ds:0x6c79

signed int g_price_modificator; // ds:0xe3f6, price modificator for smith and sell

/**
 * \brief   add one item of a hero to the repair list, with its repair price
 *
 * \param   smith       pointer to the smith description
 * \param   hero        pointer to the hero
 * \param   inv_slot    the position of the item in the inventory
 * \param   item_selector_pos   the position of the item in the repair list
 */
void add_item_to_repair_selector(struct smith_descr *smith, struct struct_hero *hero, const signed int inv_slot, const signed int item_selector_pos)
{
	const signed int item_id = hero->inventory[inv_slot].item_id;

	g_item_selector_sell[item_selector_pos].item_id = item_id;

	if (g_itemsdat[item_id].flags.armor || g_itemsdat[item_id].flags.weapon) {

		if (((struct struct_hero*)hero)->inventory[inv_slot].flags.broken) {

			g_item_selector_sell[item_selector_pos].price =
				(g_itemsdat[item_id].price + g_itemsdat[item_id].price * smith->price_mod / 100) / 2;

			if (g_item_selector_sell[item_selector_pos].price == 0) {

				g_item_selector_sell[item_selector_pos].price = 1;
			}

			g_item_selector_sell[item_selector_pos].price_unit = g_itemsdat[item_id].price_unit;

		} else {

			if (hero->inventory[inv_slot].rs_lost) {

				/* armor has degraded RS */

				g_item_selector_sell[item_selector_pos].price =
					(g_itemsdat[item_id].price + g_itemsdat[item_id].price * smith->price_mod / 100) / 4;

				if (g_item_selector_sell[item_selector_pos].price == 0) {

					g_item_selector_sell[item_selector_pos].price = 1;
				}

				g_item_selector_sell[item_selector_pos].price_unit = g_itemsdat[item_id].price_unit;

			} else {
				/* not damaged: price 0 => "ICH REPARIERE NUR WAFFEN UND RÜSTUNGEN, DIE IRGENDWIE BESCHÄDIGT SIND" */
				g_item_selector_sell[item_selector_pos].price = 0;
				g_item_selector_sell[item_selector_pos].price_unit = 1;
			}
		}
	} else {
		/* neither weapon nor armor: price 0 => not repairable (text 487) */
		g_item_selector_sell[item_selector_pos].price = 0;
		g_item_selector_sell[item_selector_pos].price_unit = 1;
	}

	g_item_selector_sell[item_selector_pos].inv_slot = inv_slot;
}

/**
 * \brief   shows the repair-screen and provides interaction
 *
 * \param   smith       pointer to the smith descriptor
 * \param   smith_id    ID of the smith (= gs_town_typeindex) [0,...,49]
 *
 * If the smith still has an item of the party, it is handed out when it is ready.
 */
void repair_screen(struct smith_descr *smith, const signed int smith_id)
{
	signed int i; /* REMARK: also used as handle */
	signed int j;
	signed int items_x;
	signed int fg_bak;
	signed int bg_bak;
	signed int answer;
	signed int num_items;
	signed int percent;
	signed int percent_old = 100;
	signed int item_id;
	signed int item_selector_pos_old;
	signed int item_selector_pos;
	signed int done = 0;
	signed int item_selector_page_offset = 0;
	signed int refresh_items;
	signed int num_filled_inv_slots;
	signed int choose_hero = 1; /* ask which hero wants something repaired */
	signed int refresh_hero = 1; /* hero changed: rebuild the repair list */
	signed int hero_pos_old = 1;
	signed int hero_pos = -1;
	struct c_str_5 fmt_h = g_smith_str_money_h;
	struct c_str_5 fmt_s = g_smith_str_money_s;
	struct c_str_5 fmt_d = g_smith_str_money_d;
	struct int16_t_3 array3 = g_smith_items_posx;
	//signed int array3[3] = { { 30, 95, 160 } };
	struct int16_t_5 array5 = g_smith_items_posy;
	//signed int array5[5] = { { 35, 55, 75, 95, 115 } };

	int32_t price;
	int32_t p_money;
	signed int deal = 0; /* 0 = no deal yet, 1 = bargaining successful, 2 = no discount wanted */
	struct struct_hero *hero2;
	struct struct_hero *hero_bargain;
	signed int width;
	signed int height;
	struct nvf_extract_desc nvf;

	/* check if this smith has an item in repair */
	if (gs_smith_repairitems[smith_id].item_id) {

		if (gs_smith_repairitems[smith_id].pickup_time > gs_day_timer) {

			/* not ready yet */
			GUI_output(get_ttx(485)); /* "<WOLLT IHR QUALITÄTSARBEIT ODER NICHT?@DANN LASST MIR BITTE AUCH DI..." */

		} else if (give_new_item_to_group(gs_smith_repairitems[smith_id].item_id, 1, 1)) {

			sprintf(g_dtp2, get_ttx(486), GUI_name_inflect_with_article( /* "<IHR KOMMT GENAU RECHT. ICH BIN FERTIG!>@MIT DIESEN WORTEN@ÜBERREIC..." */
				INFLECT_DEFINITE_ARTICLE | INFLECT_SINGULAR | INFLECT_4TH_CASE,
				gs_smith_repairitems[smith_id].item_id,
				INFLECT_NAME_TYPE_ITEM
			));

			GUI_output(g_dtp2);

			gs_smith_repairitems[smith_id].pickup_time = 0;
			gs_smith_repairitems[smith_id].item_id = 0;
		}
	} else {

		disable_ani();
		g_pp20_index = -1;

		draw_loc_icons(5, MENU_ICON_BARGAIN, MENU_ICON_SCROLL_RIGHT, MENU_ICON_SCROLL_LEFT, MENU_ICON_HERO, MENU_ICON_LEAVE);
		draw_main_screen();

		/* ICONS */
		i = load_archive_file(ARCHIVE_FILE_ICONS);
		seek_archive_file(i, 18 * 576L);
		read_archive_file(i, g_icon, 576L);
		close(i);

		g_pic_copy.x1 = 108;
		g_pic_copy.y1 = 5;
		g_pic_copy.x2 = 131;
		g_pic_copy.y2 = 28;
		g_pic_copy.src = g_icon;
		do_pic_copy(0);

		g_item_selector_sell = (struct item_selector_item*)g_fig_figure1_buf;
		memset((uint8_t*)g_item_selector_sell, 0, 50 * sizeof(struct item_selector_item));

		get_textcolor(&fg_bak, &bg_bak);
		set_textcolor(255, 0);

		refresh_items = 1;

		while (done == 0) {

			if (refresh_items != 0 || choose_hero != 0 || refresh_hero != 0) {

				/* refresh goods */

				if (choose_hero != 0) {

					hero_pos = select_hero_ok_forced(get_ttx(495)); /* "WER WILL ETWAS REPARIEREN LASSEN?" */

					hero2 = get_hero(hero_pos);

					deselect_hero_icon(hero_pos_old);
					select_hero_icon(hero_pos);
					hero_pos_old = hero_pos;
					choose_hero = 0;
					refresh_hero = 1;
				}

				if (refresh_hero != 0) {

					num_filled_inv_slots = 0;
					for (i = 0; i < NR_HERO_INVENTORY_SLOTS; i++) {
						if (hero2->inventory[i].item_id != ITEM_ID_NONE) {
							add_item_to_repair_selector(smith, hero2, i, num_filled_inv_slots++);
						}
					}

					for (i = num_filled_inv_slots; i < NR_HERO_INVENTORY_SLOTS; i++) {
						g_item_selector_sell[i].item_id = 0;
					}
					// assert(num_filled_inv_slots == hero2->num_filled_inv_slots)

					refresh_hero = 0;
					item_selector_pos_old = 1;
					item_selector_pos = item_selector_page_offset = deal = 0;
					percent_old = 100;

					do_fill_rect(g_vga_memstart, 26, 26, 105, 33, 0);

					/* REMARK: shows the money of this hero, but the repair is paid from the party money */
					make_valuta_str(g_dtp2, hero2->money);
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

					for (i = 0; i < 5; i++) {

						answer = 5 * items_x + i + item_selector_page_offset;

						if ((j = g_item_selector_sell[answer].item_id)) {

							g_pic_copy.x1 = array3.a[items_x];
							g_pic_copy.y1 = array5.a[i];
							g_pic_copy.x2 = array3.a[items_x] + 15;
							g_pic_copy.y2 = array5.a[i] + 15;
							g_pic_copy.src = g_renderbuf_ptr;

							nvf.image_num = g_itemsdat[j].item_sprite_id;

							process_nvf_extraction(&nvf);

							do_pic_copy(0);

							if (g_itemsdat[j].flags.stackable) {

								if ((num_items = hero2->inventory[g_item_selector_sell[answer].inv_slot].quantity) > 1)
								{
									my_itoa(num_items, g_dtp2, 10);

									GUI_print_string(g_dtp2,
										array3.a[items_x] + 16 - GUI_get_space_for_string(g_dtp2, 0),
										array5.a[i] + 9);

								}
							}

							sprintf(g_dtp2,	g_item_selector_sell[answer].price_unit == 1 ? fmt_h.a :
										(g_item_selector_sell[answer].price_unit == 10 ? fmt_s.a : fmt_d.a),
									g_item_selector_sell[answer].price);


							GUI_print_string(g_dtp2, array3.a[items_x] + 20, array5.a[i] + 5);
						}
					}
				}

				call_mouse();
				item_selector_pos_old = 1;
				item_selector_pos = 0;
				refresh_items = 0;

			}

			g_action_table_secondary = &g_action_table_merchant[0];
			handle_input();
			g_action_table_secondary = NULL;

			if (g_have_mouse == 2) {
				item_selector_mouse_select(&item_selector_pos, &g_item_selector_sell[item_selector_page_offset]);
			} else {
				item_selector_keyboard_select(&item_selector_pos, &g_item_selector_sell[item_selector_page_offset]);
			}

			if (item_selector_pos_old != item_selector_pos) {

				do_border(g_vga_memstart,
					array3.a[item_selector_pos_old / 5] - 1,
					array5.a[item_selector_pos_old % 5] - 1,
					array3.a[item_selector_pos_old / 5] + 16,
					array5.a[item_selector_pos_old % 5] + 16,
					0);

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

			if (g_action_id == ACTION_ID_DECREASE_ITEM_COUNT_BY_RIGHT_CLICK || g_action_id == ACTION_ID_ICON_1 || g_action_id == KEY_SCAN_CODE_ENTER) {
				/* Is ACTION == ACTION_ID_DECREASE_ITEM_COUNT_BY_RIGHT_CLICK possible at all?
				 * ACTION_ID_DECREASE_ITEM_COUNT_BY_RIGHT_CLICK can be written to ACTION in buy_screen(), but where should it show up in repair_screen()?? */

				item_id = g_item_selector_sell[item_selector_pos + item_selector_page_offset].item_id;

				p_money = get_party_money();

				if (g_item_selector_sell[item_selector_pos + item_selector_page_offset].price == 0) {

					GUI_output(get_ttx(487)); /* "<TUT MIR LEID, ICH REPARIERE NUR WAFFEN UND RÜSTUNGEN, DIE IRGENDWI..." */

				} else {

					j = 0;

					while (deal == 0 && j < 3) {


						price = (g_item_selector_sell[item_selector_pos + item_selector_page_offset].price
							* g_item_selector_sell[item_selector_pos + item_selector_page_offset].price_unit * g_price_modificator) / 4;

						make_valuta_str(g_text_output_buf, price);

						sprintf(g_dtp2, get_ttx(488), GUI_name_inflect_with_article( /* "<TJA, UM %s ZU REPARIEREN MÜSSTET IHR SCHON %s RAUSRÜCKEN!>@WIEVIEL..." */
							INFLECT_DEFINITE_ARTICLE | INFLECT_SINGULAR | INFLECT_4TH_CASE,
							item_id,
							INFLECT_NAME_TYPE_ITEM
						), g_text_output_buf);

						do {
							percent = GUI_input(g_dtp2, 2);

						} while (percent > 50);

						price -= (percent * price) / 100L;

						/* REMARK: with exactly enough money (p_money == price) or too little money a
						 * bargain test for 0% follows; with too little money it ends with text 401. */
						if (percent == 0 && p_money > price) {

							GUI_output(get_ttx(489)); /* "<SOLCHE KUNDEN SOLLTE ICH ÖFTERS MAL HABEN!>, MEINT DER SCHMIED." */
							deal = 2;

						} else if (percent >= percent_old) {

							j = 2;

						} else if (percent < 0) {

							break;

						} else {

							answer = select_hero_ok_forced(get_ttx(442)); /* "WER SOLL FEILSCHEN?" */

							hero_bargain = get_hero(answer);

							deal = bargain(hero_bargain, 1, price, percent, 2) > 0 ? 1 : 0;
						}

						if (deal > 0) {

							if (p_money < price) {

								GUI_output(get_ttx(401)); /* "SOVIEL GELD HABT IHR DOCH GAR NICHT!" */

							} else {

								if (deal != 2) {

									GUI_output(get_ttx(492)); /* "<INGERIMM MÖGE MIR VERZEIHEN!@ICH NEHME EUER ANGEBOT AN!>" */
								}

								gs_smith_repairitems[smith_id].item_id = item_id;

								if (gs_day_timer > HOURS(14)) {

									gs_smith_repairitems[smith_id].pickup_time = HOURS(23);
									GUI_output(get_ttx(490)); /* "<ALLES KLAR. ABER HEUTE WIRD DAS NICHTS MEHR.@KOMMT MORGEN WIEDER!>" */

								} else {

									gs_smith_repairitems[smith_id].pickup_time = gs_day_timer + HOURS(6);
									GUI_output(get_ttx(491)); /* "<ICH MACH MICH GLEICH AN DIE ARBEIT.@KOMMT IN 6 STUNDEN WIEDER VORB..." */
								}

								drop_item(hero2, g_item_selector_sell[item_selector_pos + item_selector_page_offset].inv_slot, 1);
								p_money -= price;
								set_party_money(p_money);

							}
							done = 1;
							continue;

						}
						 if (j == 2) {
							GUI_output(get_ttx(493)); /* "<MEINE GEDULD IST AM ENDE! SCHERT EUCH RAUS HIER.>" */
							done = 1;
						} else {
							GUI_output(get_ttx(494)); /* "<IHR DENKT WOHL, MIT EINEM EINFACHEN SCHMIED KÖNNE MAN ES MACHEN?@N..." */
							percent_old = percent;
						}

						j++;
					}
				}
			}

			if ((g_action_id >= ACTION_ID_HERO_1) && (g_action_id <= ACTION_ID_HERO_NPC)) {

				hero_pos = g_action_id - ACTION_ID_HERO_1;
				hero2 = get_hero(hero_pos);
				deselect_hero_icon(hero_pos_old);
				select_hero_icon(hero_pos);
				hero_pos_old = hero_pos;
				refresh_hero = 1;
			}

			if (g_action_id == ACTION_ID_ICON_4) {
				choose_hero = 1;
			}

			if (g_action_id == ACTION_ID_ICON_5) {
				done = 1;
			}

		}

		set_textcolor(fg_bak, bg_bak);
		g_request_refresh = 1;
		g_pp20_index = -1;
	}
}

/**
 * \brief   handler for entering smith location
 */
void do_smith(void)
{
	signed int done = 0;
	signed int answer;
	struct smith_descr *smith;

	if (gs_day_timer < HOURS(6) || gs_day_timer > HOURS(20)) {

		GUI_output(get_ttx(483)); /* "GEÖFFNET VON@6.00 UHR BIS 20.00 UHR" */
		leave_location();
		return;
	}

	if (gs_smith_kicked_flags[gs_town_typeindex] ||
		gs_smith_flogged_flags[gs_town_typeindex] ||
		(gs_town_typeindex == 1 && gs_dng14_cellarexit_flag)) {

		talk_smith();
		leave_location();
		return;
	}

	load_ggsts_nvf();
	g_request_refresh = 1;
	smith = &g_smith_descr_table[gs_town_typeindex];
	g_price_modificator = 4;

	while (!done) {

		if (g_request_refresh != 0) {

			draw_loc_icons(3, MENU_ICON_TALK, MENU_ICON_REPAIR, MENU_ICON_LEAVE);
			draw_main_screen();
			disable_ani();
			load_ani(5);
			init_ani(0);
			GUI_print_loc_line(get_tx(gs_town_locdata));
			set_audio_track(ARCHIVE_FILE_SMITH_XMI);
			g_request_refresh = 0;
		}

		handle_gui_input();

		if (g_mouse_rightclick_event || g_action_id == KEY_SCAN_CODE_PAGE_UP) {

			g_textbox_width = 4;

			answer = GUI_radio(get_ttx(496), 3, get_ttx(343), get_ttx(497), get_ttx(498)) - 1; /* "BEIM SCHMIED KÖNNT IHR" / "REDEN" / "ETWAS REPARIEREN LASSEN" / "DEN SCHMIED VERLASSEN" */

			/* TODO: why should it be 3??? Better make a backup
			 * 3 is the default width, which is restored the same way in many places (seg002, seg025). */
			g_textbox_width = 3;

			if (answer != -2) {
				g_action_id = answer + ACTION_ID_ICON_1;
			}
		}

		if (g_action_id == ACTION_ID_ICON_3) {
			done = 1;
		} else if (g_action_id == ACTION_ID_ICON_1) {

			talk_smith();
			g_request_refresh = 1;

			if (gs_smith_kicked_flags[gs_town_typeindex] ||
				gs_smith_flogged_flags[gs_town_typeindex] ||
				gs_dungeon_id != DUNGEON_ID_NONE)
			{
				done = 1;
			}
		} else if (g_action_id == ACTION_ID_ICON_2) {
			repair_screen(smith, gs_town_typeindex);
		}
	}

	leave_location();
	copy_palette();
}

/**
 * \brief   start the smith dialog (SCHMIED.TLK)
 */
void talk_smith(void)
{
	do_random_talk(13, 0);
}

/**
 * \brief   dialog logic of the smith (SCHMIED.TLK)
 *
 * State 0 branches to: kicked out before (1), smith with typeindex 17 (27),
 * Thorwal smith after the party left the Zwingfeste through his cellar (28), normal (4).
 * State 30 leads into the Zwingfeste (gs_dungeon_level = 3, x = 11, y = 2).
 *
 * \param   state       the current dialog state
 */
void TLK_schmied(const signed int state)
{
	if (!state) {
		g_dialog_next_state = (gs_smith_kicked_flags[gs_town_typeindex] ? 1 :
					(gs_town_typeindex == 17 ? 27 :
					(gs_town_typeindex == 1 && gs_dng14_cellarexit_flag ? 28 : 4)));
	} else if (state == 1) {
		g_dialog_next_state = (gs_smith_flogged_flags[gs_town_typeindex] ? 2 : 3);
	} else if (state == 3) {
		gs_smith_flogged_flags[gs_town_typeindex] = 1;
	} else if (state == 6 || state == 26) {
		tumult();
		gs_smith_kicked_flags[gs_town_typeindex] = gs_smith_flogged_flags[gs_town_typeindex] = 1;
	} else if (state == 11 || state == 14 || state == 16 || state == 23) {
		gs_smith_kicked_flags[gs_town_typeindex] = 1;
	} else if (state == 19 || state == 31) {
		g_price_modificator = 3;
	} else if (state == 30) {

		DNG_enter_dungeon(DUNGEON_ID_ZWINGFESTE);

		gs_dungeon_level = 3;
		gs_x_bak = gs_x = 11;
		gs_y_bak = gs_y = 2;
		gs_viewdir = SOUTH;
	}
}
