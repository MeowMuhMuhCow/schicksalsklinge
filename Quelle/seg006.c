/*
 * seg006 - Kampf: Liste der Kaempfer-Objekte und Zeichnen der Figuren und Portraets
 *
 * Inhalt:    Verwaltet die doppelt verkettete Liste g_fig_list_head der Objekte auf dem Kampffeld
 *            (Helden, Gegner, Leichen, Zauberbilder), sortiert in Zeichenreihenfolge von hinten nach vorn.
 *            Suchen, Einfuegen, Entfernen, sichtbar/unsichtbar schalten, Animationsspuren zuweisen.
 *            Zeichnet alle Figuren, das Kampfhintergrundbild sowie Portraet und Namen des Helden
 *            bzw. Gegners, der gerade am Zug ist.
 * Aufrufer:  Kampfmodule (seg005, seg032 ff.).
 * Daten:     g_fight_figs_index (welches Gegnerbild zuletzt geladen wurde).
 * Besonders: FIG_get_fighter() liefert bei unbekannter ID den Listenkopf statt NULL;
 *            FIG_get_hero_ptr() liefert dann Held 0. FIG_draw_enemy_pic() legt das Gegnerbild
 *            in den letzten 0x508 Byte des Textpuffers g_buffer7_ptr ab.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Y seg006.c
 */

#include <stdlib.h>
#include <string.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg004.h"
#include "seg006.h"
#include "seg027.h"
#include "seg096.h"

signed int g_fight_figs_index = -1; // ds:0x4b9e

/**
 * \brief   get the pointer to the fighter with id fighter_id
 *
 * \param   fighter_id  id of the fighter
 * \return              a pointer to the fighter with id fighter_id
 */

struct struct_fighter* FIG_get_fighter(const signed char fighter_id)
{
	struct struct_fighter *p_fighter_iter = g_fig_list_head;

	while (p_fighter_iter->id != fighter_id) {

		if (p_fighter_iter->next == NULL) {
			return g_fig_list_head;	/* TODO: Should be NULL ? */
		}

		/* set fighter_ptr to the next element */
		p_fighter_iter = p_fighter_iter->next;
	}

	return p_fighter_iter;
}

/* sucht die erste freie Kaempfer-ID in g_fig_list_array, belegt sie und gibt sie zurueck
 * (hiess FIG_set_array; ohne Pruefung der Feldgrenze) */
static signed char FIG_alloc_fighter_id(void)
{
	signed char i = 0;

	/* find first element that is zero */
	while (g_fig_list_array[i]) {

		i++;
	}

	/* make it 1 */
	g_fig_list_array[i] = 1;

	/* return the number of the index */
	return i;
}

/* zeichnet alle sichtbaren Objekte (visible == 1) der Kaempferliste in den Renderpuffer */
void FIG_draw_figures(void)
{
	signed int l1 = 10;
	signed int l2 = 118;
	struct struct_fighter *p_fighter_iter;
	struct struct_rect rect_bak;
	uint8_t* gfx_dst_bak;

	/* TODO: potential bug: Just backup the pointer dst or the whole structure struct_pic_copy ? */
	gfx_dst_bak = g_pic_copy.dst;
	g_pic_copy.dst = g_renderbuf_ptr;

	/* backup a structure */
	rect_bak = g_pic_copy_rect;

	p_fighter_iter = g_fig_list_head;

	do {
		if (p_fighter_iter->visible == 1) {

			signed int x1 = (l1 - p_fighter_iter->width / 2) + 10 * (p_fighter_iter->cbx + p_fighter_iter->cby);
			signed int y1 = (l2 - p_fighter_iter->height) +	5 * (p_fighter_iter->cbx - p_fighter_iter->cby);

			x1 += p_fighter_iter->offsetx;
			y1 += p_fighter_iter->offsety;

			g_pic_copy.x1 = x1;
			g_pic_copy.y1 = y1;
			g_pic_copy.x2 = x1 + p_fighter_iter->width - 1;
			g_pic_copy.y2 = y1 + p_fighter_iter->height - 1;
			g_pic_copy.src = p_fighter_iter->gfxbuf;

			g_pic_copy_rect.y1 = y1 + p_fighter_iter->y1;
			if (g_pic_copy_rect.y1 < 0) g_pic_copy_rect.y1 = 0;

			g_pic_copy_rect.x1 = x1 + p_fighter_iter->x1;
			if (g_pic_copy_rect.x1 < 0) g_pic_copy_rect.x1 = 0;

			g_pic_copy_rect.y2 = y1 + p_fighter_iter->y2;
			if (g_pic_copy_rect.y2 > (200 - 1)) g_pic_copy_rect.y2 = (200 - 1);

			g_pic_copy_rect.x2 = x1 + p_fighter_iter->x2;
			if (g_pic_copy_rect.x2 > (320 - 1)) g_pic_copy_rect.x2 = (320 - 1);

			do_pic_copy(2);
		}

	} while ((p_fighter_iter = p_fighter_iter->next));

	/* restore two structures */
	g_pic_copy_rect = rect_bak;
	g_pic_copy.dst = gfx_dst_bak;
}

/**
 * \brief copy the content of g_renderbuf to g_vga_memstart
 */
void FIG_set_gfx(void)
{
	/* TODO: potential bug: Just backup pointer dst or the whole struct_pic_copy ? */
	uint8_t* dst_bak = g_pic_copy.dst;

	g_pic_copy.x1 = 0;
	g_pic_copy.y1 = 0;
	g_pic_copy.x2 = 319;
	g_pic_copy.y2 = 199;
	g_pic_copy.src = g_renderbuf_ptr;
	g_pic_copy.dst = g_vga_memstart;
	call_mouse_bg();
	do_pic_copy(0);
	call_mouse();

	g_pic_copy.dst = dst_bak;
}

void FIG_call_draw_pic(void)
{
	FIG_draw_pic();
}

/* Kampfhintergrund (g_buffer8_ptr) in den Renderpuffer, dazu das Portraet des Helden bzw. Gegners am Zug */
void FIG_draw_pic(void)
{
	memcpy((void*)g_renderbuf_ptr, (void*)g_buffer8_ptr, 64000);

#if defined(__BORLANDC__)
	g_always_one = 1;
#endif

	if (g_fig_char_pic) {
		FIG_draw_char_pic(0, g_fig_char_pic);
	} else if (g_fig_enemy_pic) {
		FIG_draw_enemy_pic(0, g_fig_enemy_pic);
	}
}

/* Held zur Kaempfer-ID; gibt es keinen, wird Held 0 geliefert (nie NULL) */
struct struct_hero* FIG_get_hero_ptr(const signed int fighter_id)
{
	signed int i;

	for (i = 0; i <= 6; i++) {
		if (get_hero(i)->fighter_id == fighter_id)
			return get_hero(i);
	}

	return get_hero(0);
}

/* Gegnerblatt zur Kaempfer-ID oder NULL */
struct enemy_sheet* FIG_get_enemy_sheet(const signed int fighter_id)
{
	signed int i;

	for (i = 0; i < 20; i++) {
		if (fighter_id == g_enemy_sheets[i].fighter_id)
			return &g_enemy_sheets[i];
	}

	return (struct enemy_sheet*)NULL;
}

void FIG_set_ani_track_id_base(const signed char fighter_id, const signed char ani_track_id_base)
{
	struct struct_fighter *p_fighter_iter = g_fig_list_head;

	while (p_fighter_iter->id != fighter_id) {

		/* check for end of list */
		if (p_fighter_iter->next == NULL) {
			return;
		}

		/* set ptr to ptr->next */
		p_fighter_iter = p_fighter_iter->next;
	}

	p_fighter_iter->ani_track_id_base = ani_track_id_base;

	/* set presence flag */
	p_fighter_iter->visible = 1;
}

/**
 * \brief   hides an element from the chessboard without removing it
 * \param   fighter_id  identificates the element to hide
 */
void FIG_make_invisible(const signed char fighter_id)
{
	struct struct_fighter *p_fighter_iter;
	struct struct_fighter *p_fighter_iter_ds;

	p_fighter_iter = g_fig_list_head;

	while (p_fighter_iter->id != fighter_id) {

		if (p_fighter_iter->next == NULL) {
			return;
		}

		p_fighter_iter = p_fighter_iter->next;
	}

	p_fighter_iter->visible = 0;

	if (p_fighter_iter->double_size != -1) {

		p_fighter_iter_ds = g_fig_list_head;

		while (g_fig_double_size_fighter_id_table[p_fighter_iter->double_size] != p_fighter_iter_ds->id) {
			p_fighter_iter_ds = p_fighter_iter_ds->next;
		}
		p_fighter_iter_ds->visible = 0;
	}
}

/**
 * \brief   unhides an element from the chessboard
 *
 * \param   fighter_id  identificates the element to unhide
 */
void FIG_make_visible(const signed int fighter_id)
{
	struct struct_fighter *p_fighter_iter;
	struct struct_fighter *p_fighter_iter_ds;

	p_fighter_iter = g_fig_list_head;

	while (p_fighter_iter->id != (signed char)fighter_id) {

		if (p_fighter_iter->next == NULL) {
			return;
		}

		p_fighter_iter = p_fighter_iter->next;
	}

	p_fighter_iter->visible = 1;

	if (p_fighter_iter->double_size != -1) {

		p_fighter_iter_ds = g_fig_list_head;

		while (g_fig_double_size_fighter_id_table[p_fighter_iter->double_size] != p_fighter_iter_ds->id) {

			p_fighter_iter_ds = p_fighter_iter_ds->next;
		}

		p_fighter_iter_ds->visible = 1;
	}
}

void FIG_set_ani_track_id_weapon(const signed char fighter_id, const signed char ani_track_id_weapon)
{
	struct struct_fighter *p_fighter_iter = g_fig_list_head;

	while (p_fighter_iter->id != fighter_id) {

		if (p_fighter_iter->next == NULL) {
			return;
		}

		p_fighter_iter = p_fighter_iter->next;
	}

	p_fighter_iter->ani_track_id_weapon = ani_track_id_weapon;
}

/**
 * \brief   removes an element from the FIG_LIST
 *
 * \param   fighter_id  the element to remove
 * \param   keep_in_memory whether to save the removed element in g_fig_list_elem, useful for moving element to end of list
 */
void FIG_remove_from_list(const signed char fighter_id, const signed char keep_in_memory)
{
	struct struct_fighter *p_fighter_iter = g_fig_list_head;

	if (!p_fighter_iter) return;

	while (p_fighter_iter->id != fighter_id) {

		if (p_fighter_iter->next == NULL) {
			return;
		}

		/* ptr = ptr->next; */
		p_fighter_iter = p_fighter_iter->next;
	}

	if (!keep_in_memory) {
		g_fig_list_array[fighter_id] = 0;
	} else {
		g_fig_list_elem = *p_fighter_iter;
	}

	/* check if p == HEAD */
	if (p_fighter_iter == g_fig_list_head) {
		/* Set HEAD: head = p->next;*/
		g_fig_list_head = p_fighter_iter->next;

		if (g_fig_list_head) {
			g_fig_list_head->prev = NULL;
		}
	} else {
		/* check if p == tail */
		if (p_fighter_iter->next == NULL) {
			/* p_fighter_iter->prev->next = NULL */
			p_fighter_iter->prev->next = NULL;
		} else {
			/* remove ptr from list */
			p_fighter_iter->prev->next = p_fighter_iter->next;
			p_fighter_iter->next->prev = p_fighter_iter->prev;
		}
	}

	memset(p_fighter_iter, 0, sizeof(struct struct_fighter));

	p_fighter_iter->id = -1;
}

/**
 * \brief   adds g_fig_list_elem to FIG_LIST
 *
 * \param   fighter_id  id to assign to the new element (-1 = assign a new id)
 * \return  the new element's fighter_id (position in g_fig_list_array)
 */
signed char FIG_add_to_list(const signed char fighter_id)
{
	struct struct_fighter* p_fighter_1 = g_fig_list_buffer;
	struct struct_fighter* p_fighter_2;
	signed int x = g_fig_list_elem.cbx;
	signed int y = g_fig_list_elem.cby;

	if (g_fig_list_head == NULL) {

		g_fig_list_head = g_fig_list_buffer;

		*g_fig_list_head = g_fig_list_elem;

		if (fighter_id == -1) {
			g_fig_list_head->id = FIG_alloc_fighter_id();
		}
		/* (bei leerer Liste und fighter_id != -1 bleibt die ID aus g_fig_list_elem stehen; sie ist dann gleich) */

		g_fig_list_head->prev = NULL;
		g_fig_list_head->next = NULL;

#if !defined(__BORLANDC__)
		D1_LOG("\tlist created x = %d, y = %d\n", x, y);
#endif

		return g_fig_list_head->id;
	}

	while (p_fighter_1->id != -1) {
		p_fighter_1++;
	}

	*p_fighter_1 = g_fig_list_elem;

	if (fighter_id == -1) {
		p_fighter_1->id = FIG_alloc_fighter_id();
	} else {
		p_fighter_1->id = fighter_id;
	}

	p_fighter_2 = g_fig_list_head;

	/* The list is filled in the order of rendering, i.e. from rear to front:
	 * (x1,y1) is rendered before (x2,y2) if (x1 < x2) || (x1 == x2 && y1 > y2)
	 * On the same chessboard square, lower z is rendered before larger z.
	 */
	if (g_fig_list_elem.z != -1) {

		while ((p_fighter_2->cbx <= x) && (p_fighter_2->cbx != x || p_fighter_2->cby >= y) &&
		((p_fighter_2->cbx != x) || (p_fighter_2->cby != y) || (p_fighter_2->z <= g_fig_list_elem.z)))
		{
			if (p_fighter_2->next == 0) {

				/* append to end of the list */

				p_fighter_2->next = p_fighter_1;
				p_fighter_1->prev = p_fighter_2;
				p_fighter_1->next = NULL;

#if !defined(__BORLANDC__)
				D1_LOG("\tlist appended x = %d, y = %d\n", x, y);
#endif
				return p_fighter_1->id;
			}

			p_fighter_2 = p_fighter_2->next;
		}
	}

	p_fighter_1->prev = p_fighter_2->prev;

	if (p_fighter_2->prev != 0)
		p_fighter_2->prev->next = p_fighter_1;
	else
		g_fig_list_head = p_fighter_1;

	p_fighter_2->prev = p_fighter_1;
	p_fighter_1->next = p_fighter_2;

#if !defined(__BORLANDC__)
	D1_LOG("\tlist insert x = %d, y = %d\n", x, y);
#endif

	return p_fighter_1->id;
}

/**
 * \brief   draws the heroes picture to the fight screen
 *
 * \param   loc         0 = upper left, 1 = lower left
 * \param   hero_pos    position of the hero, hier 1-basiert (gezeichnet wird get_hero(hero_pos - 1))
 */
void FIG_draw_char_pic(const signed int loc, const signed int hero_pos)
{
	struct struct_hero *hero;
	signed int fg_bak;
	signed int bg_bak;

	hero = get_hero(hero_pos - 1);
	g_pic_copy.src = hero->pic;

	get_textcolor(&fg_bak, &bg_bak);
	set_textcolor(0xff, 0);

	g_pic_copy.dst = g_renderbuf_ptr;
	g_vga_backbuffer = g_renderbuf_ptr;

	if (loc == 0) {

		do_border(g_renderbuf_ptr, 1, 9, 34, 42, 29);
		g_pic_copy.x1 = 2;
		g_pic_copy.y1 = 10;
		g_pic_copy.x2 = 33;
		g_pic_copy.y2 = 41;
		GUI_print_string(hero->alias, 1, 1);

		draw_bar(0, 0, hero->le, hero->le_max, 1);

		draw_bar(1, 0, hero->ae, hero->ae_max, 1);
	} else {
		do_border(g_renderbuf_ptr, 1, 157, 34, 190, 29);
		g_pic_copy.x1 = 2;
		g_pic_copy.y1 = 158;
		g_pic_copy.x2 = 33;
		g_pic_copy.y2 = 189;
		GUI_print_string(hero->alias, 1, 193);
	}

	do_pic_copy(0);

	g_pic_copy.dst = g_vga_memstart;
	g_vga_backbuffer = g_vga_memstart;

	set_textcolor(fg_bak, bg_bak);
}

/**
 * \brief   draws a picture of the enemy, when on turn
 *
 * \param   loc         0 = left side, 1 = right side
 *                      (genauer: 0 = oben links, 1 = unten links)
 * \param   enemy_id    ID of the enemy
 *                      (Objekt-ID auf dem Kampffeld, Gegnerblatt = enemy_id - 10)
 */
void FIG_draw_enemy_pic(const signed int loc, const signed int enemy_id)
{
	signed int height_width;
	struct enemy_sheet *p_enemy;
	signed int fg_bak;
	signed int bg_bak;
	uint8_t* p1;
	struct nvf_extract_desc nvf;

	/* Ablage fuer das Gegnerbild: die 1288 (0x508) Byte vor g_buffer8_ptr,
	 * also das Ende des Textpuffers g_buffer7_ptr (10000 Byte, g_buffer8_ptr = g_buffer7_ptr + 10000) */
	p1 = ((HugePt)g_buffer8_ptr) - 1288L;

	p_enemy = &g_enemy_sheets[enemy_id - 10];

	if (g_gfxtab_figures_main[p_enemy->actor_sprite_id][0] != g_fight_figs_index) {

		nvf.src = (uint8_t*)load_fight_figs(g_gfxtab_figures_main[p_enemy->actor_sprite_id][0]);
		nvf.dst = p1;
		nvf.image_num = 1;
		nvf.compression_type = 0;
		nvf.width = &height_width;
		nvf.height = &height_width;

		process_nvf_extraction(&nvf);

		g_fight_figs_index = g_gfxtab_figures_main[p_enemy->actor_sprite_id][0];
	}

	/* save and set text colors */
	get_textcolor(&fg_bak, &bg_bak);
	set_textcolor(0xff, 0);

	/* set gfx pointers */
	g_pic_copy.dst = g_renderbuf_ptr;
	g_vga_backbuffer = g_renderbuf_ptr;

	if (loc == 0) {
		do_border(g_renderbuf_ptr, 1, 9, 34, 50, 0x1d);
		g_pic_copy.x1 = 2;
		g_pic_copy.y1 = 10;
		g_pic_copy.x2 = 33;
		g_pic_copy.y2 = 49;
		g_pic_copy.src = p1;
		do_pic_copy(0);
		GUI_print_string(GUI_name_base_form(g_monnames_index[p_enemy->monster_id]), 1, 1);
	} else {
		do_border(g_renderbuf_ptr, 1, 149, 34, 190, 0x1d);
		g_pic_copy.x1 = 2;
		g_pic_copy.y1 = 150;
		g_pic_copy.x2 = 33;
		g_pic_copy.y2 = 189;
		g_pic_copy.src = p1;
		do_pic_copy(0);
		GUI_print_string(GUI_name_base_form(g_monnames_index[p_enemy->monster_id]), 1, 193);
	}

	g_pic_copy.dst = g_vga_memstart;
	g_vga_backbuffer = g_vga_memstart;

	set_textcolor(fg_bak, bg_bak);
}
