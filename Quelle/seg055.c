/*
 * seg055 - Haendler: Laden und Haendlergespraeche
 *
 * Inhalt:    do_merchant() ist der Laden eines Waffen-, Kraeuter- oder Kraemerhaendlers:
 *            prueft Oeffnungszeit (8-19 Uhr, ausser Markt), Rauswurf und Beleidigung,
 *            stellt das Angebot aus Sortimentsstufe und Haendlertyp zusammen (Waffen vor
 *            Ruestungen, je nach Preis sortiert) und verzweigt zu Kaufen (seg056),
 *            Verkaufen (seg057) und Reden. In Thorwal, Prem, Phexcaer und Oberorken wird
 *            bei mehr als 100 D Schulden bei Stoerrebrandt eine Zahlung von 50 D verlangt.
 *            TLK_ghandel/khandel/whandel steuern die drei Haendlergespraeche.
 * Aufrufer:  Ortstabelle g_location_handlers (seg025), do_market() (seg065), Stadtereignis 6
 *            (seg067), THO_arsenal() (seg068), INF_treborn_unicorn() (seg072); TLK_* aus
 *            seg030, talk_merchant() auch aus THO_arsenal().
 * Daten:     g_merchant_descr_table[95] (Preisaufschlag, Typ, Sortimentsstufe, 3 Sonderwaren;
 *            Eintraege 90-93 fuer Markt, Treborn Kolberg, Zeughaus, Strassenhaendler).
 * Besonders: gs_merchant_kicked2_flags wird nirgends gesetzt, TLK_ghandel() Zustand 1 fuehrt
 *            darum immer zu 3. Der "gute Preis" nach erfolgreichem Feilschen im Gespraech
 *            (g_price_modificator = 3) wirkt nur beim Verkaufen und dort zum Nachteil der
 *            Helden (75 %), siehe sell_screen() (seg057).
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg055.c
 */
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
#include "seg031.h"
#include "seg055.h"
#include "seg056.h"
#include "seg057.h"
#include "seg096.h"
#include "seg097.h"
#include "seg103.h"

enum {
	ITEM_ID_ARMOR_POS_HEAD = 70,
	ITEM_ID_POS_MAX = 100
	/* these settings rely on the number of armor items in the buy list being at most 30
	 * (weapons at slots 0..69, armor at slots 70..99 of g_item_selector_buy). */
};

struct merchant_descr g_merchant_descr_table[95] = {
	{  50, MERCHANT_GENERAL,  1, 0, 0, 0 },
	{  40, MERCHANT_GENERAL,  2, 0, 0, 0 },
	{  30, MERCHANT_GENERAL,  8, 0, 0, 0 },
	{   0, MERCHANT_GENERAL, 14, 0, 0, 0 },
	{  50, MERCHANT_WEAPONS,  3, 0, 0, 0 },
	{  20, MERCHANT_WEAPONS, 14, 0, 0, 0 },
	{  20, MERCHANT_HERBS  , 10, 0, 0, 0 },
	{  20, MERCHANT_GENERAL, 18, 0, 0, 0 },
	{  10, MERCHANT_GENERAL, 16, 0, 0, 0 },
	{  40, MERCHANT_GENERAL, 12, 0, 0, 0 },
	{  50, MERCHANT_WEAPONS, 11, 0, 0, 0 },
	{  10, MERCHANT_GENERAL,  9, 0, 0, 0 },
	{  60, MERCHANT_WEAPONS,  4, 0, 0, 0 },
	{  50, MERCHANT_HERBS  ,  9, 0, 0, 0 },
	{ -20, MERCHANT_GENERAL, 10, 0, 0, 0 },
	{  40, MERCHANT_GENERAL,  3, 0, 0, 0 },
	{ -10, MERCHANT_GENERAL, 11, 0, 0, 0 },
	{ -10, MERCHANT_WEAPONS, 16, 0, 0, 0 },
	{  60, MERCHANT_HERBS  ,  8, 0, 0, 0 },
	{  10, MERCHANT_GENERAL,  7, 0, 0, 0 },
	{  40, MERCHANT_GENERAL,  2, 0, 0, 0 },
	{ -30, MERCHANT_GENERAL, 18, 0, 0, 0 },
	{  40, MERCHANT_HERBS  ,  7, 0, 0, 0 },
	{  40, MERCHANT_GENERAL,  2, 0, 0, 0 },
	{  30, MERCHANT_GENERAL,  7, 0, 0, 0 },
	{  80, MERCHANT_WEAPONS,  1, 0, 0, 0 },
	{  60, MERCHANT_WEAPONS,  8, 0, 0, 0 },
	{  10, MERCHANT_HERBS  , 11, 0, 0, 0 },
	{  10, MERCHANT_GENERAL,  4, 0, 0, 0 },
	{   0, MERCHANT_GENERAL, 12, 0, 0, 0 },
	{  50, MERCHANT_WEAPONS, 10, 0, 0, 0 },
	{  40, MERCHANT_WEAPONS, 15, 0, 0, 0 },
	{  40, MERCHANT_HERBS  ,  7, 0, 0, 0 },
	{  10, MERCHANT_GENERAL, 14, 0, 0, 0 },
	{   0, MERCHANT_GENERAL, 10, 0, 0, 0 },
	{   0, MERCHANT_GENERAL, 10, 0, 0, 0 },
	{  40, MERCHANT_GENERAL,  3, 0, 0, 0 },
	{  10, MERCHANT_HERBS  ,  8, 0, 0, 0 },
	{  20, MERCHANT_GENERAL,  4, 0, 0, 0 },
	{ -30, MERCHANT_GENERAL, 17, 0, 0, 0 },
	{  70, MERCHANT_WEAPONS,  2, 0, 0, 0 },
	{  10, MERCHANT_GENERAL, 12, 0, 0, 0 },
	{  40, MERCHANT_GENERAL,  2, 0, 0, 0 },
	{  40, MERCHANT_HERBS  ,  4, 0, 0, 0 },
	{ -30, MERCHANT_GENERAL, 18, 0, 0, 0 },
	{  50, MERCHANT_GENERAL,  1, 0, 0, 0 },
	{  10, MERCHANT_GENERAL, 14, 0, 0, 0 },
	{  20, MERCHANT_WEAPONS, 11, 0, 0, 0 },
	{  30, MERCHANT_GENERAL,  5, 0, 0, 0 },
	{  40, MERCHANT_WEAPONS,  2, 0, 0, 0 },
	{  10, MERCHANT_GENERAL,  7, 0, 0, 0 },
	{  20, MERCHANT_HERBS  , 14, 0, 0, 0 },
	{  30, MERCHANT_GENERAL,  5, 0, 0, 0 },
	{ -10, MERCHANT_GENERAL, 15, 0, 0, 0 },
	{   0, MERCHANT_HERBS  , 16, 0, 0, 0 },
	{  30, MERCHANT_WEAPONS, 10, 0, 0, 0 },
	{   0, MERCHANT_GENERAL, 10, 0, 0, 0 },
	{ -10, MERCHANT_GENERAL, 19, 0, 0, 0 },
	{  50, MERCHANT_GENERAL, 13, 0, 0, 0 },
	{  30, MERCHANT_GENERAL,  4, 0, 0, 0 },
	{  10, MERCHANT_GENERAL, 10, 0, 0, 0 },
	{   0, MERCHANT_WEAPONS,  7, 0, 0, 0 },
	{  10, MERCHANT_GENERAL,  5, 0, 0, 0 },
	{ -10, MERCHANT_GENERAL, 10, 0, 0, 0 },
	{  10, MERCHANT_HERBS  ,  9, 0, 0, 0 },
	{  20, MERCHANT_GENERAL,  1, 0, 0, 0 },
	{ -10, MERCHANT_GENERAL, 10, 0, 0, 0 },
	{ -20, MERCHANT_GENERAL, 16, 0, 0, 0 },
	{  40, MERCHANT_HERBS  ,  1, 0, 0, 0 },
	{  10, MERCHANT_WEAPONS,  5, 0, 0, 0 },
	{  20, MERCHANT_GENERAL,  2, 0, 0, 0 },
	{ -10, MERCHANT_WEAPONS,  7, 0, 0, 0 },
	{ -30, MERCHANT_HERBS  , 14, 0, 0, 0 },
	{   0, MERCHANT_GENERAL,  8, 0, 0, 0 },
	{ -10, MERCHANT_GENERAL, 12, 0, 0, 0 },
	{  20, MERCHANT_HERBS  , 15, 0, 0, 0 },
	{  50, MERCHANT_WEAPONS, 16, 0, 0, 0 },
	{ -10, MERCHANT_GENERAL, 10, 0, 0, 0 },
	{  10, MERCHANT_HERBS  ,  5, 0, 0, 0 },
	{   0, MERCHANT_GENERAL,  8, 0, 0, 0 },
	{  30, MERCHANT_WEAPONS,  4, 0, 0, 0 },
	{  40, MERCHANT_GENERAL,  1, 0, 0, 0 },
	{  10, MERCHANT_GENERAL,  7, 0, 0, 0 },
	{  10, MERCHANT_HERBS  , 13, 0, 0, 0 },
	{ -10, MERCHANT_GENERAL, 16, 0, 0, 0 },
	{  40, MERCHANT_GENERAL,  2, 0, 0, 0 },
	{  10, MERCHANT_GENERAL, 12, 0, 0, 0 },
	{  10, MERCHANT_HERBS  , 15, 0, 0, 0 },
	{  20, MERCHANT_WEAPONS, 18, 0, 0, 0 },
	{  40, MERCHANT_GENERAL, 17, 0, 0, 0 },
	{   0,                0,  0, 0, 0, 0 }, // market
	{  25, MERCHANT_GENERAL,  2, 0, 0, 0 }, // Treborn Kolberg
	{   0, MERCHANT_WEAPONS, 10, 0, 0, 0 }, // Thorwal, Zeughaus
	{   0, MERCHANT_GENERAL,  6, 0, 0, 0 }, // street merchant from city event 6
	{ -1, 0, 0, 0, 0, 0 }
}; // ds:0x6870

/**
 * \brief   add an item to the buy item selector of a merchant
 *
 * \param   merchant    pointer to the merchant description
 * \param   item_id     item_id
 * \param   pos         position in the item selector
 *
 * The buy price is the base price of the item plus merchant->price_mod percent.
 */
void add_item_to_buy_selector(const struct merchant_descr *merchant, const signed int item_id, const signed int pos)
{
	g_item_selector_buy[pos].item_id = item_id;

	g_item_selector_buy[pos].price = g_itemsdat[item_id].price + g_itemsdat[item_id].price * merchant->price_mod / 100;

	g_item_selector_buy[pos].price_unit = g_itemsdat[item_id].price_unit;
}

/**
 * \brief   the shop of a merchant (weapons, herbs or general goods)
 *
 * Opening hours 8:00-19:00 (not for market stands). Builds the item list for
 * buy_screen() from g_itemsdat (all items with commonness >= merchant->sortiment
 * that fit the merchant type, plus the 3 extra items), then shows the menu
 * buy / sell / talk / leave.
 */
void do_merchant(void)
{
	signed int item_selector_pos; /* dual use */
	signed int item_selector_pos_tail;
	signed int done = 0;
	signed int answer;
	signed int refresh;
	signed int item_selector_pos_tail_2;
	int32_t party_money;
	struct merchant_descr *merchant;

#if !defined(__BORLANDC__)
	/* Print merchant values */
	const uint8_t typi = gs_town_typeindex;
	const int8_t price = g_merchant_descr_table[typi].price_mod;
	const int8_t merchant_type = g_merchant_descr_table[typi].type;
	const int8_t sortiment = g_merchant_descr_table[typi].sortiment;
	const char type_string_invalid[] = "UNGUELTIG";
	const char type_string_weapons[] = "Waffen";
	const char type_string_herbs[] = "Kraeuter";
	const char type_string_general[] = "Kraemer";
	const char *const h_str = (merchant_type == MERCHANT_WEAPONS ? type_string_weapons :
					(merchant_type == MERCHANT_HERBS ? type_string_herbs :
					(merchant_type == MERCHANT_GENERAL ? type_string_general : type_string_invalid)));

	D1_INFO("Haendler-no: %2d / Haendlertyp: %s\n", typi, h_str);
	D1_INFO("\tPreise: %3d%% [70, 180]\n", 100 + price);
	D1_INFO("\tAuswahl: %2d [0, 18] (je kleiner der Wert, desto groesser die Auswahl)\n", sortiment);
#endif

	if ((gs_day_timer < HOURS(8) || gs_day_timer > HOURS(19)) && gs_town_loc_type != LOCTYPE_MARKET)
	{
		/* shop closed */

		GUI_output(get_ttx(482)); /* "GEÖFFNET VON@8.00 UHR BIS 19.00 UHR" */
		leave_location();
		return;
	}

	if (gs_merchant_kicked_flags[gs_town_typeindex]) {

		if (g_merchant_descr_table[gs_town_typeindex].type != MERCHANT_GENERAL) {
			talk_merchant();
			leave_location();
			return;
		}

	} else if (gs_merchant_offended_flags[gs_town_typeindex]) {

		GUI_output(get_ttx(507)); /* "<SOLCHEN HALSABSCHNEIDERN WIE EUCH VERKAUFE ICH NICHTS!>" */
		leave_location();
		return;
	}

	load_ggsts_nvf();
	refresh = g_request_refresh = 1;

	g_item_selector_buy = (struct item_selector_item*)g_fig_figure1_buf;
	memset((uint8_t*)g_item_selector_buy, 0, 500 * sizeof(struct item_selector_item));

	g_price_modificator = 4;

	merchant = &g_merchant_descr_table[gs_town_typeindex];

	/* redundant by memset() */
	for (item_selector_pos = 0; item_selector_pos < ITEM_ID_POS_MAX; item_selector_pos++) {
		g_item_selector_buy[item_selector_pos].item_id = 0;
	}

	item_selector_pos = 1;
	item_selector_pos_tail = 0;
	item_selector_pos_tail_2 = ITEM_ID_ARMOR_POS_HEAD;

	while (g_itemsdat[item_selector_pos].item_sprite_id != -1) {

		if (merchant->sortiment <= g_itemsdat[item_selector_pos].commonness) {

			if (g_itemsdat[item_selector_pos].flags.armor || g_itemsdat[item_selector_pos].flags.weapon) {

				if (merchant->type == MERCHANT_WEAPONS) {

					add_item_to_buy_selector(merchant, item_selector_pos,
						g_itemsdat[item_selector_pos].flags.weapon ? item_selector_pos_tail++ : item_selector_pos_tail_2++);
				}

			} else if (g_itemsdat[item_selector_pos].flags.herb_potion) {

				if (merchant->type == MERCHANT_HERBS) {

					add_item_to_buy_selector(merchant, item_selector_pos, item_selector_pos_tail++);
				}

			} else {

				if (merchant->type == MERCHANT_GENERAL) {

					add_item_to_buy_selector(merchant, item_selector_pos, item_selector_pos_tail++);
				}
			}
		}

		item_selector_pos ++;
	}

	/* new scope for item_selector_pos, should be sth. like extra_items_pos */
	for (item_selector_pos = 0; item_selector_pos < 3; item_selector_pos++) {

		if (merchant->extra_items[item_selector_pos]) {
			add_item_to_buy_selector(merchant, merchant->extra_items[item_selector_pos], item_selector_pos_tail++);
		}
	}

	if (merchant->type == MERCHANT_WEAPONS) {

		qsort((uint8_t*)g_item_selector_buy, item_selector_pos_tail, sizeof(struct item_selector_item), item_selector_item_compare);
		qsort((uint8_t*)g_item_selector_buy + sizeof(struct item_selector_item) * ITEM_ID_ARMOR_POS_HEAD, item_selector_pos_tail_2 - ITEM_ID_ARMOR_POS_HEAD, sizeof(struct item_selector_item), item_selector_item_compare);

		/* copy the rest */
		for (item_selector_pos = 0; item_selector_pos_tail_2 - ITEM_ID_ARMOR_POS_HEAD > item_selector_pos; item_selector_pos++) {

			g_item_selector_buy[item_selector_pos_tail + item_selector_pos] = g_item_selector_buy[item_selector_pos + ITEM_ID_ARMOR_POS_HEAD];

		}

		/* cleanup */
		for (item_selector_pos = item_selector_pos_tail + item_selector_pos_tail_2 - ITEM_ID_ARMOR_POS_HEAD; item_selector_pos < ITEM_ID_POS_MAX; item_selector_pos++) {
			g_item_selector_buy[item_selector_pos].item_id = 0;
		}

	} else {
		qsort((uint8_t*)g_item_selector_buy, item_selector_pos_tail, sizeof(struct item_selector_item), item_selector_item_compare);
	}

	while (done == 0 && !gs_merchant_offended_flags[gs_town_typeindex]) {

		if (g_request_refresh) {

			draw_loc_icons(4, MENU_ICON_BUY, MENU_ICON_SELL, MENU_ICON_TALK, MENU_ICON_LEAVE);

			draw_main_screen();

			disable_ani();

			load_ani(merchant->type == MERCHANT_WEAPONS ? 15 : (merchant->type == MERCHANT_HERBS ? 22 : 14));

			init_ani(0);

			set_audio_track(ARCHIVE_FILE_TERMS_XMI);

			GUI_print_loc_line(gs_town_loc_type == LOCTYPE_MARKET ? get_ttx(679) : (gs_town_typeindex == MERCHANT_TYPEINDEX_STREET_MERCHANT ?  get_ttx(46) : get_tx(gs_town_locdata))); /* "EIN STAND AUF DEM MARKTPLATZ" / "STRASSENHÄNDLER" */
			// typeindex MERCHANT_TYPEINDEX_STREET_MERCHANT == 93 is the street merchant from random city event 6.

			g_request_refresh = refresh = 0;
		}

		if (refresh != 0) {
			GUI_print_loc_line(get_tx(gs_town_locdata));
			refresh = 0;
		}

		handle_gui_input();

		if (gs_town_id == TOWN_ID_THORWAL ||
			gs_town_id == TOWN_ID_PREM ||
			gs_town_id == TOWN_ID_PHEXCAER ||
			gs_town_id == TOWN_ID_OBERORKEN)
		{

			/* debt logic for merchants.
			 * gs_bank_deposit is counted in silver: -1000 = 100 D debts, +500 = 50 D = 5000 H paid */

			if ((gs_bank_deposit <= -1000) && !gs_debt_days) {

				GUI_output(get_ttx(761)); /* "EURE SCHULDEN BEI STOERREBRANDT BELAUFEN SICH AUF MEHR ALS HUNDERT@..." */
				gs_debt_days = 7;
			}

			if (gs_debt_days == -1) {

				if (GUI_bool(get_ttx(762))) { /* "EURE SCHULDEN ... IHR MÜSST 50 DUKATEN EINZAHLEN, ODER IHR BEKOMMT KEINE WARE MEHR. IHR KÖNNT AUCH DIREKT IN DIESEM LADEN BEZAHLEN. WOLLT IHR DAS?" */

					party_money = get_party_money();

					if (party_money < 5000) {

						GUI_output(get_ttx(763)); /* "WER NICHT ZAHLEN WILL, ODER KANN, HAT HIER NICHTS VERLOREN!" */
						g_mouse_rightclick_event = g_action_id = 0;
						done = 1;

					} else {
						gs_bank_deposit += 500;
						if (gs_bank_deposit > -1000) {
							gs_debt_days = 0;
						}

						party_money -= 5000;
						set_party_money(party_money);
					}
				} else {

					GUI_output(get_ttx(763)); /* "WER NICHT ZAHLEN WILL, ODER KANN, HAT HIER NICHTS VERLOREN!" */
					g_mouse_rightclick_event = g_action_id = 0;
					done = 1;
				}
			}
		}

		if (g_mouse_rightclick_event || g_action_id == KEY_SCAN_CODE_PAGE_UP) {

			answer = GUI_radio(get_ttx(430), 4, get_ttx(431), get_ttx(432),	get_ttx(343), get_ttx(434)) - 1; /* "HÄNDLEROPTIONEN" / "WAREN KAUFEN" / "WAREN VERKAUFEN" / "REDEN" / "LADEN VERLASSEN" */

			if (answer != -2) {
				g_action_id = answer + ACTION_ID_ICON_1;
			}
		}

		if (g_action_id == ACTION_ID_ICON_4) {

			done = 1;

		} else if (g_action_id == ACTION_ID_ICON_1) {


			buy_screen();

		} else if (g_action_id == ACTION_ID_ICON_2) {

			sell_screen(merchant);

		} else if (g_action_id == ACTION_ID_ICON_3) {

			talk_merchant();

			if (gs_merchant_kicked_flags[gs_town_typeindex]) {
				done = 1;
			}
		}
	}

	copy_palette();

	leave_location();
}

/**
 * \brief   start the merchant dialog (WHANDEL / KHANDEL / GHANDEL.TLK)
 */
void talk_merchant(void)
{
	signed int tlk_id;

	switch (g_merchant_descr_table[gs_town_typeindex].type) {
		case MERCHANT_WEAPONS: tlk_id = 16; break;
		case MERCHANT_HERBS: tlk_id = 15; break;
		case MERCHANT_GENERAL: tlk_id = 14; break;
	}

	do_random_talk(tlk_id, 0);
}

void TLK_ghandel(const signed int state)
{
	if (!state) {
		g_dialog_next_state = (gs_merchant_kicked_flags[gs_town_typeindex] ? 1 : 4);
	} else if (state == 1) {
		/* REMARK: gs_merchant_kicked2_flags[] is never written anywhere, so this always leads to state 3 */
		g_dialog_next_state = (gs_merchant_kicked2_flags[gs_town_typeindex] ? 2 : 3);
	} else if (state == 6 && gs_town_typeindex != MERCHANT_TYPEINDEX_MARKET) {
		gs_merchant_kicked_flags[gs_town_typeindex] = 1;
	} else if (state == 10) {
		/* test CH+0 */
		g_dialog_next_state = (test_attrib(get_first_hero_available_in_group(), ATTRIB_CH, 0) > 0 ? 11 : 12);
	}
}

void TLK_khandel(const signed int state)
{
	if (!state) {
		g_dialog_next_state = (gs_merchant_kicked_flags[gs_town_typeindex] ? 1 : 2);
	} else if (state == 5) {
		tumult();
		if (gs_town_typeindex != MERCHANT_TYPEINDEX_MARKET) {
			gs_merchant_kicked_flags[gs_town_typeindex] = 1;
		}

	} else if (state == 7 && gs_town_typeindex != MERCHANT_TYPEINDEX_MARKET) {
		gs_merchant_kicked_flags[gs_town_typeindex] = 1;
	} else if (state == 8) {
		g_dialog_next_state = (random_schick(20) <= 3 ? 9 : -1);
	} else if (state == 11) {
		g_price_modificator = 3;
	} else if (state == 12) {
		/* test CH+4 */
		g_dialog_next_state = (test_attrib(get_first_hero_available_in_group(), ATTRIB_CH, 4) > 0 ? 13 : 10);
	}
}

void TLK_whandel(const signed int state)
{
	if (!state) {

		g_dialog_next_state = (gs_merchant_kicked_flags[gs_town_typeindex] ? 26 : 1);

	} else if (state == 7 || state == 13) {

		tumult();

		if (gs_town_typeindex != MERCHANT_TYPEINDEX_MARKET) {
			gs_merchant_kicked_flags[gs_town_typeindex] = 1;
		}

	} else if ((state == 8 || state == 16) && gs_town_typeindex != MERCHANT_TYPEINDEX_MARKET) {

		gs_merchant_kicked_flags[gs_town_typeindex] = 1;

	} else if (state == 18) {

		/* test CH+0 */
		g_dialog_next_state = (test_attrib(get_first_hero_available_in_group(), ATTRIB_CH, 0) > 0 ? 19 : -1);

	} else if (state == 25) {

		if (test_talent(get_first_hero_available_in_group(), TA_FEILSCHEN, 0) > 0) {

			g_dialog_next_state = 23;
			g_price_modificator = 3;
		} else {
			g_dialog_next_state = 24;
		}
	}
}
