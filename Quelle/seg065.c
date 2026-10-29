/*
 * seg065 - Markt und Sonderszenen (Hyggelik, Endkampf, Spielende)
 *
 * Inhalt:    do_market() ist der Marktplatz: Waffen-, Kraeuter- oder Kraemerstand werden
 *            als Haendler 90 mit den Marktwerten der Stadt eingerichtet und ueber
 *            do_merchant() besucht. final_intro() zeigt vor dem Endkampf das Bild aus
 *            FACE.NVF, show_hyggelik_ani() die Animation von Hyggeliks Geist mit Grimring,
 *            show_times_up() das Ende bei Zeitablauf (Orks erobern Thorwal), show_outro()
 *            den Abspann nach dem Sieg: 50 D je Held, Heilung, Ruecksetzen von Zustaenden,
 *            Spielstand als "beendet" speichern (fuer den zweiten Teil).
 * Aufrufer:  Ortstabelle g_location_handlers (seg025); final_intro() aus seg116,
 *            show_hyggelik_ani() aus seg089 (Hyggeliks Ruine), show_times_up() und
 *            show_outro() aus seg002.
 * Daten:     g_hyg_ani_x* (Positionen der Animation, definiert in seg066.c).
 * Besonders: show_outro(): der NSC (Platz 6) bekommt nichts; le_max_malus wird ohne
 *            Rueckgabe der LE geloescht (Bright-Eyes-Fix im Code).
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg065.c
 */
#include <string.h>

#if !defined(__BORLANDC__)
#include <unistd.h>
#endif

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg004.h"
#include "seg008.h"
#include "seg025.h"
#include "seg026.h"
#include "seg027.h"
#include "seg029.h"
#include "seg055.h"
#include "seg097.h"
#include "seg120.h"

struct struct_ani {
	uint8_t* ptr;
	signed int width;
	signed int height;
};

/**
 * \brief   location handler of a market place: choose a stand (weapons, herbs, general goods)
 *
 * The answer 1..3 of the menu is the MERCHANT_* type of the stand. Merchant entry 90 gets the
 * price modifier and the assortment of this town's market (g_market_descr_table).
 */
void do_market(void)
{
	signed int done;
	signed int answer;
	signed int typeindex_bak;
	signed int viewdir_bak;
	signed int viewdir_bak_bak;

	done = 0;
	g_request_refresh = 1;
	viewdir_bak_bak = gs_viewdir_bak;
	viewdir_bak = gs_viewdir;

	do {

		if (g_request_refresh) {

			draw_main_screen();
			disable_ani();
			load_ani(16);
			init_ani(0);
			g_request_refresh = 0;
		}

		answer = GUI_radio(get_ttx(680), 4, get_ttx(676), get_ttx(677), get_ttx(678), get_ttx(613)); /* "WAS WOLLT IHR AUF DEM MARKTPLATZ" / "WAFFENSTAND" / "KRÄUTERSTAND" / "KRÄMERSTAND" / "ZURÜCK" */

		if (answer == 4 || answer == -1) {

			done = 1;

		} else {

			/* set up parameters for this merchant */
			g_merchant_descr_table[MERCHANT_TYPEINDEX_MARKET].price_mod = g_market_descr_table[gs_town_typeindex].price_mod;
			g_merchant_descr_table[MERCHANT_TYPEINDEX_MARKET].sortiment = g_market_descr_table[gs_town_typeindex].size;
			g_merchant_descr_table[MERCHANT_TYPEINDEX_MARKET].type = answer;
			typeindex_bak = gs_town_typeindex;
			gs_town_typeindex = MERCHANT_TYPEINDEX_MARKET;

			/* visit this merchant */
			do_merchant();

			/* change back to market */
			gs_town_loc_type = LOCTYPE_MARKET;

			/* clean up */
			gs_town_typeindex = typeindex_bak;
			gs_viewdir_bak = viewdir_bak_bak;
			gs_viewdir = viewdir_bak; /* by this line, the party will *not* be rotated after leaving the market */

			g_merchant_descr_table[MERCHANT_TYPEINDEX_MARKET].price_mod = 0;
			g_merchant_descr_table[MERCHANT_TYPEINDEX_MARKET].sortiment = 0;
			g_merchant_descr_table[MERCHANT_TYPEINDEX_MARKET].type = 0;
		}

	} while (!done);

	gs_town_loc_type = LOCTYPE_NONE;
	copy_palette();
}

/**
 * \brief   picture before the final fight (FACE.NVF, 2 images with map effect), then fade out
 */
void final_intro(void)
{
	signed int i;
	signed int handle;
	signed int width;
	signed int height;
	uint32_t len;
	uint8_t *ptr1;
	uint8_t *ptr2;
	struct nvf_extract_desc nvf;

	g_pp20_index = -2;

	call_mouse_bg();

	g_wallclock_update = 0;

	/* load FACE.NVF */
	handle = load_archive_file(ARCHIVE_FILE_FACE_NVF);
	len = read_archive_file(handle, g_buffer9_ptr, 64000);
	close(handle);

	ptr1 = (g_buffer9_ptr + len) - (96 * 3);

	do_fill_rect(g_vga_memstart, 0, 0, 319, 199, 0);

	wait_for_vsync();

	set_palette(ptr1, 0, 0x60);

	ptr2 = g_buffer9_ptr + 80000L;

	nvf.dst = g_renderbuf_ptr;
	nvf.src = (uint8_t*)g_buffer9_ptr;
	nvf.image_num = 0;
	nvf.compression_type = 3;
	nvf.width = &width;
	nvf.height = &height;
	process_nvf_extraction(&nvf);

	map_effect(g_renderbuf_ptr);

	nvf.dst = ptr2;
	nvf.src = (uint8_t*)g_buffer9_ptr;
	nvf.image_num = 1;
	nvf.compression_type = 3;
	nvf.width = &width;
	nvf.height = &height;
	process_nvf_extraction(&nvf);

	g_pic_copy.x1 = 0;
	g_pic_copy.y1 = 20;
	g_pic_copy.x2 = 319;
	g_pic_copy.y2 = 39;
	g_pic_copy.src = ptr2;
	g_pic_copy.dst = g_renderbuf_ptr;

	do_pic_copy(2);

	vsync_or_key(100);

	map_effect(g_renderbuf_ptr);

	g_pic_copy.dst = g_vga_memstart;

	vsync_or_key(250);

	memset(g_renderbuf_ptr, 0, 96 * 3);

	for (i = 0; i < 0x40; i++) {

		pal_fade((int8_t*)ptr1, (int8_t*)g_renderbuf_ptr);
		pal_fade((int8_t*)ptr1 + 0x60, (int8_t*)g_renderbuf_ptr + 0x60);
		pal_fade((int8_t*)ptr1 + 0xc0, (int8_t*)g_renderbuf_ptr + 0xc0);

		wait_for_vsync();

		set_palette(ptr1, 0, 0x60);
	}

	/* TODO: update window */
	memset((void*)g_vga_memstart, 0, 320 * 200);

	refresh_colors();
	call_mouse();
}

/**
 * \brief   extract image nvf_no (from g_renderbuf_ptr) to ani->ptr
 *
 * \return              pointer behind the extracted image (place for the next one)
 */
static uint8_t* hyg_ani_extract(const signed int nvf_no, struct struct_ani *ani)
{
	HugePt retval;
	struct nvf_extract_desc nvf;

	nvf.dst = ani->ptr;
	nvf.src = g_renderbuf_ptr;
	nvf.image_num = nvf_no;
	nvf.compression_type = 3;
	nvf.width = &ani->width;
	nvf.height = &ani->height;

	process_nvf_extraction(&nvf);

	retval = (HugePt)ani->ptr + ani->width * ani->height;

	return (uint8_t*)retval;
}

/**
 * \brief   draw an animation image at (x, y) into g_renderbuf_ptr (transparent)
 */
static void hyg_ani_draw(struct struct_ani *ani, const signed int x, const signed int y)
{
	g_pic_copy.x1 = x;
	g_pic_copy.y1 = y;
	g_pic_copy.x2 = x + ani->width - 1;
	g_pic_copy.y2 = y + ani->height - 1;

	g_pic_copy.src = ani->ptr;
	g_pic_copy.dst = g_renderbuf_ptr;

	do_pic_copy(2);
}

/**
 * \brief   copy the background (HYGBACK.NVF, at g_buffer9_ptr + 130000) to g_renderbuf_ptr
 */
static void hyg_ani_draw_background(void)
{
	g_pic_copy.x1 = 0;
	g_pic_copy.y1 = 0;
	g_pic_copy.x2 = 319;
	g_pic_copy.y2 = 199;
	g_pic_copy.src = g_buffer9_ptr + 130000L;
	g_pic_copy.dst = g_renderbuf_ptr;

	do_pic_copy(0);
}

/**
 * \brief   copy g_renderbuf_ptr to the screen
 */
static void hyg_ani_show(void)
{
	g_pic_copy.x1 = 0;
	g_pic_copy.y1 = 0;
	g_pic_copy.x2 = 319;
	g_pic_copy.y2 = 199;
	g_pic_copy.src = g_renderbuf_ptr;
	g_pic_copy.dst = g_vga_memstart;

	do_pic_copy(0);
}

/**
 * \brief   animation of Hyggelik's ghost handing over Grimring (HYGBACK.NVF, HYGGELIK.NVF)
 */
void show_hyggelik_ani(void)
{
	signed int i;
	signed int handle;
	int32_t filelen;
	uint8_t *src;
	uint8_t* ptr1;
	uint8_t* ptr2;
	struct struct_ani ani[30];

	g_wallclock_update = 0;
	ptr1 = g_buffer9_ptr;
	ptr2 = g_buffer9_ptr + 130000L;

	handle = load_archive_file(ARCHIVE_FILE_HYGBACK_NVF);
	filelen = read_archive_file(handle, g_renderbuf_ptr, 64000);
	close(handle);
	src = &(g_renderbuf_ptr[filelen - 0xc0]);

	do_fill_rect(g_vga_memstart, 0, 0, 319, 199, 0);
	memcpy((void*)g_dtp2, src, 192);
	src = (uint8_t*)g_dtp2;

	wait_for_vsync();
	set_palette(src, 0 , 0x40);
	ani[0].ptr = ptr2;

	hyg_ani_extract(0, ani);

	handle = load_archive_file(ARCHIVE_FILE_HYGGELIK_NVF);
	filelen = read_archive_file(handle, g_renderbuf_ptr, 64000);
	close(handle);

	ani[0].ptr = ptr1;

	for (i = 0; i < 26; i++) {
		ani[i + 1].ptr = hyg_ani_extract(i, &ani[i]);
	}

	call_mouse_bg();

	hyg_ani_draw_background();
	hyg_ani_draw(&ani[0], 145, 39);
	hyg_ani_draw(&ani[7], 142, 86);
	hyg_ani_draw(&ani[10], 82, 67);
	hyg_ani_draw(&ani[20], 186, 67);

	map_effect(g_renderbuf_ptr);

	for (i = 0; i < 7; i++) {
		hyg_ani_draw_background();
		hyg_ani_draw(&ani[7], 142, 86);
		hyg_ani_draw(&ani[i], 145, 39);
		hyg_ani_draw(&ani[10], 82, 67);
		hyg_ani_draw(&ani[20], 186, 67);
		hyg_ani_show();
		vsync_or_key(3);
	}

	vsync_or_key(6);

	for (i = 0; i < 5; i++) {
		hyg_ani_draw_background();
		hyg_ani_draw(&ani[6], 145, 39);
		hyg_ani_draw(&ani[7], 142, 86);
		hyg_ani_draw(&ani[20], 186, 67);
		hyg_ani_draw(&ani[10 + i], g_hyg_ani_x0[i], 67);
		hyg_ani_show();
		vsync_or_key(3);
	}

	hyg_ani_draw_background();
	hyg_ani_draw(&ani[5], 145, 39);
	hyg_ani_draw(&ani[7], 142, 86);
	hyg_ani_draw(&ani[15], g_hyg_ani_x1, 67);
	hyg_ani_draw(&ani[20], g_hyg_ani_x5, 67);
	hyg_ani_show();
	vsync_or_key(3);

	hyg_ani_draw_background();
	hyg_ani_draw(&ani[5], 145, 39);
	hyg_ani_draw(&ani[7], 142, 86);
	hyg_ani_draw(&ani[16], g_hyg_ani_x2, 67);
	hyg_ani_draw(&ani[21], g_hyg_ani_x6, 67);
	hyg_ani_show();
	vsync_or_key(3);

	for (i = 0; i < 3; i++) {
		hyg_ani_draw_background();
		hyg_ani_draw(&ani[6], 145, 39);
		hyg_ani_draw(&ani[8], 144, g_hyg_ani_y9[i]); /* REMARK: g_hyg_ani_y9 holds y coordinates */
		hyg_ani_draw(&ani[17 + i], g_hyg_ani_x3[i], 67);
		hyg_ani_draw(&ani[22 + i], g_hyg_ani_x7[i], 67);
		hyg_ani_show();
		vsync_or_key(3);
	}

	hyg_ani_draw_background();
	hyg_ani_draw(&ani[6], 145, 39);
	hyg_ani_draw(&ani[9], 125, 104);
	hyg_ani_draw(&ani[19], g_hyg_ani_x3[2], 67);
	hyg_ani_draw(&ani[24], g_hyg_ani_x7[2], 67);
	hyg_ani_show();
	vsync_or_key(100);

	/* clear the screen */
	do_fill_rect(g_renderbuf_ptr, 0, 0, 319, 199, 0);

	hyg_ani_draw(&ani[25], 100, 0);
	g_pic_copy.dst = g_vga_memstart;
	map_effect(g_renderbuf_ptr);
	vsync_or_key(500);

	memset(g_renderbuf_ptr, 0, 0xc0);

	for (i = 0; i < 64; i++) {
		pal_fade((int8_t*)src, (int8_t*)g_renderbuf_ptr);
		pal_fade((int8_t*)src + 0x60, (int8_t*)g_renderbuf_ptr + 0x60);
		wait_for_vsync();
		set_palette(src, 0, 0x40);
	}

	call_mouse();
	/* TODO: update window */
	memset((void*)g_vga_memstart, 0, 320 * 200);
	refresh_colors();
}

/**
 * \brief   the game is lost: time is up, the orcs conquer Thorwal (CHARTEXT.LTX 55..57)
 */
void show_times_up(void)
{
	signed int fi_bak;
	signed int tw_bak;
	signed int basepos_x_bak;
	signed int basepos_y_bak;

	fi_bak = g_text_file_index;
	load_tx2(ARCHIVE_FILE_CHARTEXT_LTX);
	set_audio_track(ARCHIVE_FILE_VICTORY_XMI);
	disable_ani();
	draw_main_screen();
	load_ani(35);
	init_ani(0);

	vsync_or_key(200);

	tw_bak = g_textbox_width;
	basepos_x_bak = g_basepos_x;
	basepos_y_bak = g_basepos_y;

	g_textbox_width = 7;
	g_basepos_x = 0;
	g_basepos_y = 55;

	GUI_output(get_tx2(55)); /* "AM 27. PERAINE DES JAHRES 17 STÜRMEN TAUSENDE UND ABERTAUSENDE@SCHW..." */
	GUI_output(get_tx2(56)); /* "MITTE INGERIMM ERSTÜRMEN DIE ORKS DIE MAUERN VON THORWAL, DAS GEFEC..." */
	GUI_output(get_tx2(57)); /* "ABER THORWAL IST NICHT MEHR, UND MIT DER STADT VERSINKT AUCH DIE LE..." */

	g_basepos_x = basepos_x_bak;
	g_basepos_y = basepos_y_bak;
	g_textbox_width = tw_bak;

	/* restore text file except for CHARTEXT.LTX, TAVERN.TLK and except for dialogs */
	if (fi_bak != -1 && fi_bak != ARCHIVE_FILE_CHARTEXT_LTX
	    && fi_bak != ARCHIVE_FILE_TAVERN_TLK
	    && !(fi_bak >= 156 && fi_bak <= 176)) {
		load_tx2(fi_bak);
	}
}

/**
 * \brief   the game is won: outro pictures and texts, reward (50 D per hero), heal and reset
 *          the heroes (slots 0..5), mark the game as done and save it
 */
void show_outro(void)
{
	signed int j; /* multi use: disease_id, poison_id, attribute index */
	signed int handle;
	signed int width;
	signed int height;
	uint16_t len;		/* REMARK: check if read/writes with BCC can read more than 32k bytes at once */
	uint8_t *pal_ptr;
	struct struct_hero *hero;
	signed int hero_pos;
	struct nvf_extract_desc nvf;

	g_textbox_width = 7;
	g_basepos_x = 0;
	g_basepos_y = 60;

	load_tx2(ARCHIVE_FILE_CHARTEXT_LTX);
	set_audio_track(ARCHIVE_FILE_VICTORY_XMI);
	disable_ani();

	/* load OUTRO1.NVF */
	handle = load_archive_file(ARCHIVE_FILE_OUTRO1_NVF);
	len = read_archive_file(handle, (uint8_t*)g_buffer9_ptr, 64000);
	close(handle);

	pal_ptr = (g_buffer9_ptr + len) - 0xc0L;
	do_fill_rect(g_vga_memstart, 0, 0, 319, 199, 0);
	wait_for_vsync();
	set_palette(pal_ptr, 0, 0x40);

	nvf.dst = g_renderbuf_ptr;
	nvf.src = (uint8_t*)g_buffer9_ptr;
	nvf.image_num = 0;
	nvf.compression_type = 0;
	nvf.width = &width;
	nvf.height = &height;
	process_nvf_extraction(&nvf);

	g_pic_copy.x1 = (320 - width) / 2;
	g_pic_copy.y1 = 0;
	g_pic_copy.x2 = (320 - width) / 2 + width - 1;
	g_pic_copy.y2 = height - 1;
	g_pic_copy.src = g_renderbuf_ptr;
	do_pic_copy(0);

	vsync_or_key(200);

	GUI_output(get_tx2(58)); /* "KANN ES WAHR SEIN? IST DIES DAS ENDE? ES SCHEINT SO, KEINER DER ORK..." */

	/* load OUTRO2.NVF */
	handle = load_archive_file(ARCHIVE_FILE_OUTRO2_NVF);
	len = read_archive_file(handle, (uint8_t*)g_buffer9_ptr, 64000);
	close(handle);

	pal_ptr = (g_buffer9_ptr + len) - 0xc0L;
	do_fill_rect(g_vga_memstart, 0, 0, 319, 199, 0);
	wait_for_vsync();
	set_palette(pal_ptr, 0, 0x40);

	nvf.dst = g_renderbuf_ptr;
	nvf.src = (uint8_t*)g_buffer9_ptr;
	nvf.image_num = 0;
	nvf.compression_type = 0;
	nvf.width = &width;
	nvf.height = &height;
	process_nvf_extraction(&nvf);

	g_pic_copy.x1 = (320 - width) / 2;
	g_pic_copy.y1 = 0;
	g_pic_copy.x2 = (320 - width) / 2 + width - 1;
	g_pic_copy.y2 = height - 1;
	g_pic_copy.src = g_renderbuf_ptr;
	do_pic_copy(0);

	vsync_or_key(200);

	GUI_output(get_tx2(59)); /* "WENIGE TAGE SPÄTER BRECHEN DIE ORKS IHRE ZELTE AB UND ZIEHEN WEITER..." */

	/* load OUTRO3.NVF */
	handle = load_archive_file(ARCHIVE_FILE_OUTRO3_NVF);
	len = read_archive_file(handle, (uint8_t*)g_buffer9_ptr, 64000);
	close(handle);

	pal_ptr = (g_buffer9_ptr + len) - 0xc0L;
	do_fill_rect(g_vga_memstart, 0, 0, 319, 199, 0);
	wait_for_vsync();
	set_palette(pal_ptr, 0, 0x40);

	nvf.dst = g_renderbuf_ptr;
	nvf.src = (uint8_t*)g_buffer9_ptr;
	nvf.image_num = 0;
	nvf.compression_type = 0;
	nvf.width = &width;
	nvf.height = &height;
	process_nvf_extraction(&nvf);

	g_pic_copy.x1 = (320 - width) / 2;
	g_pic_copy.y1 = 0;
	g_pic_copy.x2 = (320 - width) / 2 + width - 1;
	g_pic_copy.y2 = height - 1;
	g_pic_copy.src = g_renderbuf_ptr;
	do_pic_copy(0);

	vsync_or_key(200);

	GUI_output(get_tx2(60)); /* "ZWEI WOCHEN SPÄTER ERREICHT IHR THORWAL. DIE KUNDE VON EUREN TATEN..." */
	GUI_output(get_tx2(61)); /* "NOCH AM GLEICHEN ABEND FINDET BEI HETMANN TRONDE EIN FEST ZU EUREN..." */
	GUI_output(get_tx2(62)); /* "DEM WILLEN HYGGELIKS SEI GENÜGE GETAN. HIERMIT ÜBERREICHEN WIR EUCH..." */
	GUI_output(get_tx2(63)); /* "IHR ERHALTET DEN SILBERNEN POTTWAL, DIE HÖCHSTE AUSZEICHNUNG, DIE D..." */
	GUI_output(get_tx2(64)); /* "IHR VERLASST THORWAL UND DEN TRUBEL DER STADT, UM RUHE ZU FINDEN UN..." */
	GUI_output(get_tx2(65)); /* "ZU NEUEN ABENTEUERN?@NEUE ABENTEUER ERWARTEN SIE IM ZWEITEN TEIL DE..." */

	/* reset the timers */
	sub_ingame_timers(DAYS(30));
	sub_mod_timers(DAYS(30));
	/* Der Header deklariert sub_light_timers() ohne Prototyp, das Argument geht als int (push 64h).
	 * 100 Viertelstunden = 25 Stunden */
	sub_light_timers(100);

	/* give the heroes the reward and restore them */
	hero = get_hero(0);
	for (hero_pos = 0; hero_pos < 6; hero_pos++, hero++) {

		if (hero->typus) {

			/* get 50D */
			add_party_money(5000);

			hero->flags.asleep = 0;
			hero->flags.petrified = 0;
			hero->flags.brewing = 0;
			hero->flags.renegade = 0;

			/* reset every disease */
			for (j = 0; j < 8; j++) {

				hero->disease[j].status = DISEASE_STATUS_HEALTHY;
				hero->disease[j].time_counter = 0;
				hero->disease[j].log_1 = 0;
				hero->disease[j].log_2 = 0;
				hero->disease[j].log_3 = 0;
			}

			/* reset every poison */
			for (j = 0; j < 10; j++) {

				hero->poison[j].status = POISON_STATUS_HEALTHY;
				hero->poison[j].time_counter = 0;
				hero->poison[j].log_1 = 0;
				hero->poison[j].log_2 = 0;
				hero->poison[j].log_3 = 0;
			}

#ifdef M302de_ORIGINAL_BUGFIX
			/* Original-Bug: restore permanent LE-damage at the end of game */
			/* Famous heroes get healed for free */
			if (hero->le_max_malus > 0)
			{
				hero->le_max += hero->le_max_malus;
				hero->le_max_malus = 0;
			}
#endif

			/* set LE to the max */
			add_hero_le(hero, hero->le_max);
			/* set AE to the max */
			add_hero_ae(hero, hero->ae_max);

			/* ??? the permanent LE loss is forgotten, but le_max is not restored (see the fix above) */
#ifndef M302de_ORIGINAL_BUGFIX
			hero->le_max_malus = 0;
#endif
			hero->hunger = 0;
			hero->thirst = 0;
			hero->temple_id = 0;
			hero->recipe_id = 0;
			hero->recipe_timer = 0;
			hero->alchemy_inn_typeindex = 0;
			hero->jail = 0;

			/* Remark: better use get_sober() */
			hero->drunk = 0;

			hero->ruhe_koerper = 0;

			for (j = 0; j <= 13; j++) {

				hero->attrib[j].current = hero->attrib[j].normal;
				hero->attrib[j].mod = 0;
			}

			hero->heal_timer = 0;
			hero->staffspell_timer = 0;

			hero->slot_pos = hero_pos + 1;
		}
	}

	/* share all money */
	set_party_money(get_party_money());

	/* mark the game as done */
	gs_datseg_status_start = 99;

	g_basepos_x = 0;
	g_basepos_y = 0;

	/* save the game */
	save_game_state();
}
