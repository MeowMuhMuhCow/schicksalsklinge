/*
 * seg034 - Kampf: Zielauswahl und Bewegung auf dem Schachbrett
 *
 * Inhalt:    FIG_cb_select_target() laesst den Spieler mit Maus oder Pfeiltasten ein Ziel
 *            waehlen, FIG_check_target_cursor_step() prueft dabei jeden Schritt des Auswahlcursors
 *            (Reichweite, Sichtlinie). FIG_move_hero() waehlt das Zielfeld einer Bewegung
 *            oder Flucht, zeigt die BP-Kosten an und fuehrt die Bewegung aus.
 *            FIG_latecomers() setzt Gegner, die erst in spaeteren Runden erscheinen, auf
 *            ein freies Feld nahe ihrer Startposition.
 * Aufrufer:  FIG_menu() (seg033), do_fight() (seg032), check_hero_range_attack() (seg035).
 * Daten:     g_cb_obj_nonobstacle (begehbare Objekte, Nummer - 50), Richtungstabellen.
 * Besonders: FIG_move_hero(): Flucht ueber den Rand y<0 oder y>23 braucht ein BP mehr als
 *            ueber x<0; im Fluchtfall wird object_id_bak einer frueheren Cursorposition
 *            geprueft. Bekannt (Bright-Eyes): "ZIEL: 99 BP" bei toten Figuren als Ziel.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg034.c
 */
#include <stdio.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg006.h"
#include "seg007.h"
#include "seg032.h"
#include "seg034.h"
#include "seg036.h"
#include "seg038.h"
#include "seg039.h"
#include "seg096.h"
#include "seg097.h"

const signed int g_cb_obj_nonobstacle[27] = {
       	0x0017, 0x0018, 0x0019, 0x001a, 0x001b,
	0x0024, 0x0025, 0x0026, 0x0027, 0x0028,
	0x0029, 0x002a, 0x002b, 0x002c, 0x002d,
	0x002e, 0x002f, 0x0030, 0x0031, 0x0032,
	0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
	0x0038, -1
}; // ds:0x5f46, { 23,24,25,26,27,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,-1 }
   //
static struct viewdir_offsets g_fig_viewdir_offsets1 = { { { 1, 0 }, { 0, -1 }, { -1, 0 }, { 0, 1 } } }; // ds:0x5f7c
static struct viewdir_offsets g_fig_viewdir_offsets2 = { { { 1, 0 }, { 0, -1 }, { -1, 0 }, { 0, 1 } } }; // ds:0x5f8c
static char g_string_14spaces[15] = "              "; // ds:0x5f9c

/**
 * \brief   checks if the target cursor may move one square, from (x, y) to (x + x_diff, y + y_diff)
 *
 *          Used by FIG_cb_select_target() (cursor movement) and check_hero_range_attack()
 *          (line of fire). Moving away from the actor is only possible along the row or column
 *          of the actor and across empty squares, dead actors or objects; a living actor
 *          blocks the line. Obstacles (objects >= 50 not in g_cb_obj_nonobstacle) are never entered.
 *
 * \param   x_hero      x-coordinate of the actor (start of the cursor)
 * \param   y_hero      y-coordinate of the actor (start of the cursor)
 * \param   x           x-coordinate of the current cursor position
 * \param   y           y-coordinate of the current cursor position
 * \param   x_diff      x-coordinate difference {-1, 0, 1}
 * \param   y_diff      y-coordinate difference {-1, 0, 1}
 * \param   max_range   maximal range for the weapon or spell
 * \return              1 = the cursor may move, 0 = it may not
 **/
signed int FIG_check_target_cursor_step(const signed int x_hero, const signed int y_hero,
			const signed int x, const signed int y,
			const signed int x_diff, const signed int y_diff,
			const signed int max_range)
{
	signed int target_object_id;
	signed int cursor_object_id;
	signed int distance;

	cursor_object_id = get_cb_val(x, y);
	target_object_id = get_cb_val(x + x_diff, y + y_diff);

	distance = manhattan_distance(x + x_diff, y + y_diff, x_hero, y_hero);

	/* distance actions */
	if ((cursor_object_id != 0) && (manhattan_distance(x, y, x_hero, y_hero) < distance) && (distance <= max_range)) {

		if ((x_hero == x) && (y_hero == y)) {

			if ((target_object_id < 50) || ((target_object_id >= 50) && is_in_int_array(target_object_id - 50, g_cb_obj_nonobstacle)))
			{
				return 1;
			} else {
				return 0;
			}

		} else if (
			(
				(cursor_object_id >= 50)
				|| ((cursor_object_id >= 10) && (cursor_object_id < 30) && g_enemy_sheets[cursor_object_id - 10].flags.dead)
				|| ((cursor_object_id >= 30) && (cursor_object_id < 50) && g_enemy_sheets[cursor_object_id - 30].flags.dead)
				|| ((cursor_object_id < 10) && get_hero(cursor_object_id - 1)->flags.dead)
			) && (
				(target_object_id >= 0)
				&& ((target_object_id < 50) || ((target_object_id >= 50) && is_in_int_array(target_object_id - 50, g_cb_obj_nonobstacle)))
			)
		) {

			if (
				((((x_diff == 1) || (x_diff == -1)) && (y_hero != y)))
				|| ((((y_diff == 1) || (y_diff == -1)) && (x_hero != x)))
			) {
				return 0;
			} else {
				return 1;
			}

		} else {
			return 0;
		}
	}

	/* 4 near actions */
	if (x_diff == 1) {
		if (
			(target_object_id >= 0)
			&& ((target_object_id < 50) || ((target_object_id >= 50) && is_in_int_array(target_object_id - 50, g_cb_obj_nonobstacle)))
			&& ((x < 23) && (y == y_hero) && (manhattan_distance(x_hero, y_hero, x + 1, y) <= max_range))
		) {
			return 1;
		} else {
			return 0;
		}

		return 0;
	}

	if (x_diff == -1) {
		if (
			(target_object_id >= 0)
			&& ((target_object_id < 50) || ((target_object_id >= 50) && is_in_int_array(target_object_id - 50, g_cb_obj_nonobstacle)))
			&& ((x > 0) && (y == y_hero) && (manhattan_distance(x_hero, y_hero, x - 1, y) <= max_range))
		) {
			return 1;
		} else {
			return 0;
		}

		return 0;

	}

	if (y_diff == 1) {
		if (
			(target_object_id >= 0)
			&& ((target_object_id < 50) || ((target_object_id >= 50) && is_in_int_array(target_object_id - 50, g_cb_obj_nonobstacle)))
			&& ((y < 23) && (x == x_hero) && (manhattan_distance(x_hero, y_hero, x, y + 1) <= max_range))
		) {
			return 1;
		} else {
			return 0;
		}

		return 0;

	}

	if (y_diff == -1) {
		if (
			(target_object_id >= 0)
			&& ((target_object_id < 50) || ((target_object_id >= 50) && is_in_int_array(target_object_id - 50, g_cb_obj_nonobstacle)))
			&& ((y > 0) && (x == x_hero) && (manhattan_distance(x_hero, y_hero, x, y - 1) <= max_range))
		) {
			return 1;
		} else {
			return 0;
		}
	}

	return 0;
}

/**
 * \brief   select a target for the fight action of the current actor
 *
 * \param   px          in: x-coordinate of actor, out: x-coordinate of the selected square
 * \param   py          in: y-coordinate of actor, out: y-coordinate of the selected square
 * \param   max_range   maximal range for the weapon or spell (1 = adjacent only, 99 = ranged)
 * \return              object id on the selected square (0 = empty), -99 if cancelled
 */
signed char FIG_cb_select_target(signed int *px, signed int *py, const signed int max_range)
{
	signed int x_diff;
	signed int y_diff;
	signed int x;
	signed int y;
	signed int cursor_object_id;
	signed int x_screen;
	signed int y_screen;
	signed int from_kbd;
	/* TODO: the next two variables are constants */
	signed int cb_base_x = 9;
	signed int cb_base_y = 116;

	g_mouse_leftclick_event = g_mouse_rightclick_event = 0;

	call_mouse_bg();

	g_mouse_posx_bak = g_mouse_posx = x_screen = cb_base_x + 10 * (*px + *py);
	g_mouse_posy_bak = g_mouse_posy = y_screen = cb_base_y + 5 * (*px - *py);


	mouse_move_cursor(g_mouse_posx, g_mouse_posy);

	call_mouse();

	x = *px;
	y = *py;

	if (g_fig_cb_selector_id[0] != -1) {

		FIG_remove_from_list(g_fig_cb_selector_id[0], 0);
		g_fig_cb_selector_id[0] = -1;
	}

	g_fig_list_elem.figure = 0;
	g_fig_list_elem.nvf_no = 0;
	g_fig_list_elem.cbx = *px;
	g_fig_list_elem.cby = *py;
	g_fig_list_elem.offsetx = 0;
	g_fig_list_elem.offsety = 4;
	g_fig_list_elem.height = 11;
	g_fig_list_elem.width = 22;
	g_fig_list_elem.x1 = 0;
	g_fig_list_elem.y1 = 0;
	g_fig_list_elem.x2 = 21;
	g_fig_list_elem.y2 = 10;
	g_fig_list_elem.is_enemy = 0;
	g_fig_list_elem.reload = 0;
	g_fig_list_elem.ani_track_id_weapon = FANI_TRACK_ID_NONE;
	g_fig_list_elem.ani_track_id_base = FANI_TRACK_ID_NONE;
	g_fig_list_elem.gfxbuf = g_fig_cb_selector_buf;
	g_fig_list_elem.z = 1;
	g_fig_list_elem.visible = 1;
	g_fig_list_elem.double_size = -1;

	g_fig_cb_selector_id[0] = FIG_add_to_list(-1);

	draw_fight_screen_pal(0);

	do {
		handle_input();

		if ((g_action_id == KEY_SCAN_CODE_ESC) || g_mouse_rightclick_event) {

			/* cancel */

			g_mouse_rightclick_event = 0;

			FIG_remove_from_list(g_fig_cb_selector_id[0], 0);

			g_fig_cb_selector_id[0] = -1;

			draw_fight_screen_pal(0);

			return -99;
		}

		from_kbd = 0;

		if ((g_action_id == KEY_SCAN_CODE_ARROW_UP) || (g_action_id == KEY_SCAN_CODE_ARROW_DOWN) ||
			(g_action_id == KEY_SCAN_CODE_ARROW_RIGHT) || (g_action_id == KEY_SCAN_CODE_ARROW_LEFT))
		{
			from_kbd = 1;

		} else {

			x_diff = g_mouse_posx - x_screen;
			y_diff = g_mouse_posy - y_screen;

			if (((y_diff > 0) && (x_diff <= -10)) || ((x_diff < 0) && (y_diff >= 5)))
			{
				g_action_id = KEY_SCAN_CODE_ARROW_DOWN;

			} else if (((y_diff < 0) && (x_diff >= 10)) || ((x_diff > 0) && (y_diff <= -5)))
			{
				g_action_id = KEY_SCAN_CODE_ARROW_UP;

			} else if (((y_diff > 0) && (x_diff >= 10)) || ((x_diff > 0) && (y_diff >= 5)))
			{
				g_action_id = KEY_SCAN_CODE_ARROW_RIGHT;

			} else if (((y_diff < 0) && (x_diff <= -10)) ||	((x_diff < 0) && (y_diff <= -5)))
			{
				g_action_id = KEY_SCAN_CODE_ARROW_LEFT;
			}
		}

		if (g_mouse_leftclick_event) {

			g_action_id = KEY_SCAN_CODE_ENTER;
			g_mouse_leftclick_event = 0;
		}

		if (g_action_id == KEY_SCAN_CODE_ARROW_RIGHT) {

			if (FIG_check_target_cursor_step(x, y, *px, *py, 1, 0, max_range)) {
				(*px)++;
			}

		} else if (g_action_id == KEY_SCAN_CODE_ARROW_LEFT) {

			if (FIG_check_target_cursor_step(x, y, *px, *py, -1, 0, max_range)) {
				(*px)--;
			}

		} else if (g_action_id == KEY_SCAN_CODE_ARROW_UP) {

			if (FIG_check_target_cursor_step(x, y, *px, *py, 0, 1, max_range)) {
				(*py)++;
			}

		} else if (g_action_id == KEY_SCAN_CODE_ARROW_DOWN) {

			if (FIG_check_target_cursor_step(x, y, *px, *py, 0, -1, max_range)) {
				(*py)--;
			}
		}

		if ((g_fig_list_elem.cbx != *px) || (g_fig_list_elem.cby != *py)) {

			call_mouse_bg();

			x_screen = cb_base_x + 10 * (*px + *py);
			y_screen = cb_base_y + 5 * (*px - *py);

			if (from_kbd != 0) {

				g_mouse_posx_bak = g_mouse_posx = x_screen;
				g_mouse_posy_bak = g_mouse_posy = y_screen;

				mouse_move_cursor(g_mouse_posx, g_mouse_posy);
			}

			call_mouse();
			FIG_call_draw_pic();
			cursor_object_id = get_cb_val(*px, *py);

			if ((cursor_object_id > 0) && (cursor_object_id < 50)) {

				if (cursor_object_id < 10) {
					FIG_draw_char_pic(1, cursor_object_id);
				} else {

					if (cursor_object_id >= 30) {
						FIG_draw_enemy_pic(1, cursor_object_id - 20);
					} else {
						FIG_draw_enemy_pic(1, cursor_object_id);
					}
				}
			}

			FIG_remove_from_list(g_fig_cb_selector_id[0], 1);

			g_fig_list_elem.cbx = *px;
			g_fig_list_elem.cby = *py;

			FIG_add_to_list(g_fig_cb_selector_id[0]);
			FIG_draw_figures();
			FIG_set_gfx();
		}

	} while (g_action_id != KEY_SCAN_CODE_ENTER);

	FIG_remove_from_list(g_fig_cb_selector_id[0], 0);

	g_fig_cb_selector_id[0] = -1;

	draw_fight_screen_pal(0);

	return get_cb_val(*px, *py);
}

/* determine free position (*px,*py) for new enemy to appear on chessboard
 * if the desired position (x,y) is occupied, a free position is determined as
 * close as possible to this position.
 * double_size is 1 if the enemy has a double-size sprite (such as wolves)
 */
void FIG_find_latecomer_position(const signed int x, const signed int y, signed int *px, signed int *py, const signed int viewdir, const signed int double_size)
{
	signed int new_viewdir;
	signed int dist;
	signed int new_x;
	signed int new_y;
	signed char done;

	struct viewdir_offsets a = g_fig_viewdir_offsets1;

	done = 0;

	*px = x;
	*py = y;

	if (get_cb_val(x, y) == 0) {

		if (double_size == 0) {
			return;
		}

		if (get_cb_val(x - a.offset[viewdir].x, y - a.offset[viewdir].y) == 0) {
			return;
		}
	}

	dist = 1;

	while (!done) {

		for (new_viewdir = FIG_VIEWDIR__BEGIN; new_viewdir < FIG_VIEWDIR__END; new_viewdir++) {

			new_x = *px + a.offset[new_viewdir].x * dist;
			new_y = *py + a.offset[new_viewdir].y * dist;

			if ((new_x >= 0) && (new_x < 24) && (new_y >= 0) && (new_y < 24) && !get_cb_val(new_x, new_y))
			{

				if ((double_size == 0) || (!get_cb_val(new_x - a.offset[viewdir].x, new_y - a.offset[viewdir].y)))
				{
					done = 1;
					*px = new_x;
					*py = new_y;
					break;
				}
			}
		}

		dist++;
	}
}

/**
 * \brief   add enemies to the fight, which appear in later rounds
 */
void FIG_latecomers(void)
{
	signed int i;
	signed int x;
	signed int y;
	signed int tail_fighter_id;
	struct enemy_sheet *p_enemy;
	struct struct_fighter *p_fighter;
	struct struct_fighter *p_fighter_add;

	struct viewdir_offsets a = g_fig_viewdir_offsets2;

	/* for all enemies in this fight */
	for (i = 0; i < g_nr_of_enemies; i++) {

		p_enemy = &g_enemy_sheets[i];

		/* if enemy has not appeared yet */
		if (p_enemy->round_appear > 0) {

			/* decrement counter */
			p_enemy->round_appear--;

			if (!p_enemy->round_appear) {

				/* let enemy enter the fight */

				if (!p_enemy->flags.scared) {

					if (is_in_byte_array(p_enemy->actor_sprite_id, g_double_size_actor_sprite_id_table)) {

						FIG_find_latecomer_position(g_current_fight->enemies[i].x, g_current_fight->enemies[i].y,
								&x, &y,	g_current_fight->enemies[i].viewdir, 1);

						p_fighter = FIG_get_fighter(p_enemy->fighter_id);

						p_fighter->cbx = x;
						p_fighter->cby = y;

						tail_fighter_id = g_fig_double_size_fighter_id_table[p_fighter->double_size];

						p_fighter_add = FIG_get_fighter((signed char)tail_fighter_id);

						p_fighter_add->cbx = x - a.offset[g_current_fight->enemies[i].viewdir].x;
						p_fighter_add->cby = y - a.offset[g_current_fight->enemies[i].viewdir].y;

						FIG_remove_from_list(p_enemy->fighter_id, 1);

						FIG_add_to_list(p_enemy->fighter_id);

						FIG_remove_from_list((signed char)tail_fighter_id, 1);

						FIG_add_to_list((signed char)tail_fighter_id);
					} else {
						FIG_find_latecomer_position(g_current_fight->enemies[i].x, g_current_fight->enemies[i].y,
								&x, &y, g_current_fight->enemies[i].viewdir, 0);

						p_fighter = FIG_get_fighter(p_enemy->fighter_id);

						p_fighter->cbx = x;
						p_fighter->cby = y;

						FIG_remove_from_list(p_enemy->fighter_id, 1);

						FIG_add_to_list(p_enemy->fighter_id);
					}

#if !defined(__BORLANDC__)
					place_obj_on_cb(x, y, i + 10, p_enemy->actor_sprite_id, g_current_fight->enemies[i].viewdir);
#else
					place_obj_on_cb(x, y, i + 10, (_AX = (int16_t)p_enemy->actor_sprite_id), (_AX = (int16_t)g_current_fight->enemies[i].viewdir));
#endif
					FIG_make_visible(p_enemy->fighter_id);

				} else {
					/* scared enemies entering a fight are marked as dead. does this ever happen? */
					p_enemy->flags.dead = 1;
				}
			}
		}
	}
}

/**
 * \brief   length of the path in g_fig_move_pathdir
 *
 * \return              number of steps before the end marker -1, or 99 if the path is
 *                      marked (-2) as not reachable with the available BP
 */
signed int FIG_move_pathlen(void)
{
	signed int i = 0;

	/* count everything till the end marker -1 of the path */
	while (g_fig_move_pathdir[i] != -1) {
		i++;
	}

	/* if the end marker -1 is followed by a -2, the available BP are not sufficient */
	if (g_fig_move_pathdir[i + 1] == -2) {
		return 99;
	}

	return i;
}

/**
 * \brief   Moves a hero on the chessboard
 *
 * \param   hero        pointer to the hero
 * \param   hero_pos    position of the hero in the group (object id = hero_pos + 1)
 * \param   px          pointer to the x-coordinate on the chessboard
 * \param   py          pointer to the y-coordinate on the chessboard
 */
void FIG_move_hero(struct struct_hero *hero, const signed int hero_pos, signed int *px, signed int *py)
{
	signed int problem;
	signed int path_end;
	signed int sel_x;
	signed int sel_y;
	signed int target_reachable;
	signed int curr_x;
	signed int curr_y;
	signed int fg_bak;
	signed int bg_bak;
	signed int x_screen;
	signed int y_screen;
	signed int from_kbd;
	signed int base_x = 9;
	signed int base_y = 116;
	signed char object_id_bak;
	signed char bp_cost;
	signed char object_id_bak_escape;
	signed int escape_dir; /* 0: no escape; escape over the border 1: x < 0; 2: y < 0; 3: y > 23.
				 * Not compatible with the directions in the path (1 -> path dir 2, 2 -> 1, 3 -> 3)! */
	signed int mouse_cb_x;
	signed int mouse_cb_y;

	call_mouse_bg();

	g_mouse_posx_bak = g_mouse_posx = x_screen = base_x + 10 * (*px + *py);

	g_mouse_posy_bak = g_mouse_posy = y_screen = base_y + 5 * (*px - *py);

	mouse_move_cursor(g_mouse_posx, g_mouse_posy);

	call_mouse();

	g_mouse_leftclick_event = g_mouse_rightclick_event = 0;

	sel_x = *px;
	sel_y = *py;
	curr_x = sel_x;
	curr_y = sel_y;

	if (g_fig_cb_selector_id[0] != -1) {

		FIG_remove_from_list(g_fig_cb_selector_id[0], 0);
		g_fig_cb_selector_id[0] = -1;
	}

	g_fig_list_elem.figure = 0;
	g_fig_list_elem.nvf_no = 0;
	g_fig_list_elem.cbx = sel_x;
	g_fig_list_elem.cby = sel_y;
	g_fig_list_elem.offsetx = 0;
	g_fig_list_elem.offsety = 4;
	g_fig_list_elem.x1 = 0;
	g_fig_list_elem.y1 = 0;
	g_fig_list_elem.x2 = 21;
	g_fig_list_elem.y2 = 10;
	g_fig_list_elem.height = 11;
	g_fig_list_elem.width = 22;
	g_fig_list_elem.is_enemy = 0;
	g_fig_list_elem.reload = 0;
	g_fig_list_elem.ani_track_id_weapon = FANI_TRACK_ID_NONE;
	g_fig_list_elem.ani_track_id_base = FANI_TRACK_ID_NONE;
	g_fig_list_elem.gfxbuf = g_fig_cb_selector_buf;
	g_fig_list_elem.z = 1;
	g_fig_list_elem.visible = 1;
	g_fig_list_elem.double_size = -1;

	g_fig_cb_selector_id[0] = FIG_add_to_list(-1);

	draw_fight_screen_pal(0);

	do {
		handle_input();

		curr_x = sel_x;
		curr_y = sel_y;
		from_kbd = 0;

		if ((g_action_id == KEY_SCAN_CODE_ARROW_UP) || (g_action_id == KEY_SCAN_CODE_ARROW_DOWN) ||
				(g_action_id == KEY_SCAN_CODE_ARROW_RIGHT) || (g_action_id == KEY_SCAN_CODE_ARROW_LEFT))
		{
			from_kbd = 1;

		} else {

			mouse_cb_x = ((g_mouse_posx - base_x) / 10 + (g_mouse_posy - base_y) / 5) / 2;
			mouse_cb_y = -((g_mouse_posy - base_y) / 5 - mouse_cb_x);

			if ((mouse_cb_x != sel_x) || (mouse_cb_y != sel_y)) {

				if ((mouse_cb_x >= -1) && (mouse_cb_x <= 24) && (mouse_cb_y >= -1) && (mouse_cb_y <= 24)) {

					g_action_id = ACTION_ID_VOID;
				}
			}
		}

		if (g_mouse_leftclick_event) {

			g_mouse_leftclick_event = 0;
			g_action_id = KEY_SCAN_CODE_ENTER;
		}

		if ((g_action_id == KEY_SCAN_CODE_ARROW_RIGHT) && (sel_x < 23)) {
			sel_x++;
		} else if ((g_action_id == KEY_SCAN_CODE_ARROW_LEFT) && (sel_x >= 0)) {
			sel_x--;
		} else if ((g_action_id == KEY_SCAN_CODE_ARROW_UP) && (sel_y <= 23)) {
			sel_y++;
		} else if ((g_action_id == KEY_SCAN_CODE_ARROW_DOWN) && (sel_y >= 0)) {
			sel_y--;
		} else if (g_action_id == ACTION_ID_VOID) {
			sel_x = mouse_cb_x;
			sel_y = mouse_cb_y;
		}

		/* treatment of the two visible corners of the map */
		if ((sel_x < 0) && (sel_y < 0)) {
			sel_x = 0;
		}
		if ((sel_x < 0) && (sel_y > 23)) {
			sel_x = 0;
		}

		if ((sel_x != curr_x) || (sel_y != curr_y)) {

			call_mouse_bg();

			x_screen = base_x + 10 * (sel_x + sel_y);
			y_screen = base_y + 5 * (sel_x - sel_y);

			if (from_kbd != 0) {
				g_mouse_posx_bak = g_mouse_posx = x_screen;
				g_mouse_posy_bak = g_mouse_posy = y_screen;

				mouse_move_cursor(g_mouse_posx, g_mouse_posy);
			}

			call_mouse();

			FIG_call_draw_pic();

			FIG_remove_from_list(g_fig_cb_selector_id[0], 0);

			g_fig_list_elem.cbx = sel_x;
			g_fig_list_elem.cby = sel_y;
			g_fig_list_elem.gfxbuf = g_fig_cb_selector_buf;
			g_fig_list_elem.double_size = -1;
			g_fig_cb_selector_id[0] = FIG_add_to_list(-1);

			FIG_draw_figures();
			FIG_set_gfx();

			problem = 0;
			bp_cost = 0;

			if ((*px != sel_x) || (*py != sel_y)) { /* selected square is different from active hero position */

				escape_dir = 0;

				/* (sel_x > 23) is missing as the right border of the map is not visible, so it cannot be used for escape (I guess...) */
				if ((sel_x < 0) || (sel_y < 0) || (sel_y > 23)) {

					/* active hero wants to escape over the border of the map */

					/* reset (sel_x, sel_y) to the square at the border of the map. remember the direction of the escape */
					if (sel_x < 0) {

						sel_x = 0;
						escape_dir = 1;

					} else if (sel_y < 0) {

						sel_y = 0;
						escape_dir = 2;

					} else {

						sel_y = 23;
						escape_dir = 3;
					}

					if ((get_cb_val(sel_x, sel_y) != 0) && ((*px != sel_x) || (*py != sel_y)))
					{

						/* the reset square (sel_x, sel_y) is blocked and different from the position of the active hero */

						problem = 3; /* target square blocked */

						/* again reset (sel_x, sel_y) to the square beyond the border */
						if (escape_dir == 1) {
							sel_x = -1;
						} else if (escape_dir == 2) {
							sel_y = -1;
						} else {
							/*  escape_dir == 3 */
							sel_y = 24;
						}

					} else {
						object_id_bak_escape = get_cb_val(sel_x, sel_y);
					}
				} else {
					object_id_bak = get_cb_val(sel_x, sel_y);
				}

				if (problem != 3) {

					if ((escape_dir != 0) && (*px == sel_x) && (*py == sel_y))
					{
						/* active hero wants to escape over the border of the map and is already at the border. */
						g_fig_move_pathdir[0] = -1;
						bp_cost = 0;

					} else {

						FIG_set_cb_object(sel_y, sel_x, 124); /* target marker for FIG_find_path_to_target. The original content of this square has been backuped before in 'object_id_bak' or 'object_id_bak_escape'. */
						target_reachable = FIG_find_path_to_target((uint8_t*)hero, hero_pos, *px, *py, 10);
						/* target_reachable = 1: there is a path of length < 50 to the target square; target_reachable = -1: there is no such path */
						bp_cost = FIG_move_pathlen();
#if !defined(__BORLANDC__)
						D1_INFO("x: %d, y: %d, target id: %d, reachable: %d, distance: %d\n",
								escape_dir?object_id_bak_escape:sel_x, sel_y,
								object_id_bak, target_reachable, bp_cost);
#endif
					}

					if (escape_dir != 0) {

						/* restore the original entry of the target square, which has been overwritten by the target marker. */
						FIG_set_cb_object(sel_y, sel_x, object_id_bak_escape);

						path_end = 0;
						while (g_fig_move_pathdir[path_end] != -1) {
							path_end++;
						}

						/* add the last escape step to the path */
						if (escape_dir == 1) {

							sel_x = -1;

							/* REMARK: here one BP more than the path is enough, for escape_dir 2 and 3 below
							 * two more BP are needed (bp_cost < fight_bp_left - 1). Original-Bug? */
							if (hero->fight_bp_left > bp_cost) {

								g_fig_move_pathdir[path_end] = 2;
								g_fig_move_pathdir[path_end + 1] = -1;
							}

						} else if (escape_dir == 2) {

							sel_y = -1;

							if (bp_cost < (hero->fight_bp_left - 1)) {

								g_fig_move_pathdir[path_end] = 1;
								g_fig_move_pathdir[path_end + 1] = -1;
							}

						} else {

							sel_y = 24;

							if (bp_cost < (hero->fight_bp_left - 1)) {

								g_fig_move_pathdir[path_end] = 3;
								g_fig_move_pathdir[path_end + 1] = -1;
							}
						}

						bp_cost++;

					} else {
						/* restore the original entry of the target square, which has been overwritten by the target marker. */
						FIG_set_cb_object(sel_y, sel_x, object_id_bak);
					}

					/* REMARK: in the escape case object_id_bak has not been set in this pass (only
					 * object_id_bak_escape), so the following checks use the value of an earlier cursor
					 * position (uninitialized on the first pass). An escape square can thus be shown
					 * as "ZIEL: BLOCKIERT". Original-Bug? */
					if (object_id_bak >= 50) {

						problem = 3; /* target square blocked */
#ifndef M302de_ORIGINAL_BUGFIX
					} else if (object_id_bak >= 10) {

						/* target square contains an enemy (including the tail of a double-size enemy) */
						if (!g_enemy_sheets[(object_id_bak - 10 - (object_id_bak >= 30 ? 20 : 0))].flags.dead) /* check 'dead' flag */
						{
							/* enemy is not dead */
							problem = 3;
						}

						/* Original-Bug:
						 * If the target square contains a dead enemy,
						 * the checks for target_reachable and bp_cost below are not executed because of the "nested if" structure of the code.
						 * This results in the incorrect message "ZIEL: 99 BP" if the target is pointing at a dead enemy
						 * which is not reachable within the available BP or not reachable at all. */

					} else if (object_id_bak > 0) {

						/* target square contains a hero */
						if (!get_hero(object_id_bak - 1)->flags.dead &&
							!get_hero(object_id_bak - 1)->flags.unconscious &&
							(object_id_bak != hero_pos + 1))
						{
							/* hero is not dead, not unconscious, and not the active hero */
							problem = 3;
						}

						/* Original-Bug:
						 * If the target square contains a dead or unconscious hero different from the active hero,
						 * the checks for target_reachable and bp_cost below are not executed because of the "nested if" structure of the code.
						 * This results in the incorrect message "ZIEL: 99 BP" if the target is pointing at a dead/unconscious hero
						 * which is not reachable within the available BP or not reachable at all. */
#else
						/* Bug fix:
						 * flatten the nested if branches. */

					} else if ((object_id_bak >= 10) && !g_enemy_sheets[(object_id_bak - 10 - (object_id_bak >= 30 ? 20 : 0))].flags.dead) { /* check 'dead' flag */
						/* target square contains a non-dead enemy (including the tail of a double-size enemy) */
						problem = 3;

					} else if ((object_id_bak > 0) && (object_id_bak < 10) && !get_hero(object_id_bak - 1)->flags.dead && !get_hero(object_id_bak - 1)->flags.unconscious && (object_id_bak != hero_pos + 1)) {

						/* target square contains a non-dead and non-unconscious hero different from the active hero */
						problem = 3;
#endif
					} else if (target_reachable == -1) {

						problem = 4;

					} else if (hero->fight_bp_left < bp_cost) {

						problem = 2;

					} else if ((sel_x > 23) || (sel_x < 0) || (sel_y > 23) || (sel_y < 0) || (get_cb_val(sel_x, sel_y) < 0)) {
						if ((g_scenario_buf->fig_background_id > FIG_BACKGROUND_ID_SHIP_BORDERED) || (sel_x >= 0))
						{
							problem = 1;
						}
					}
				}
			}

			get_textcolor(&fg_bak, &bg_bak);
			set_textcolor(255, 0);

			if (!problem) {
				sprintf(g_dtp2,	get_tx(12), bp_cost); /* "ZIEL: %d BP" */ /* Target: %d BP */
			}


			GUI_print_string(problem == 1 ? get_tx(13) : /* "ZIEL: FLUCHT" */ /* Target: escape */
						(problem == 3 ? get_tx(30) : /* "ZIEL: BLOCKIERT" */ /* Target: blocked */
						(problem == 2 ? get_tx(32) : /* "ZIEL: ZU WEIT" */ /* Target: too far */
						(problem == 4 ? get_tx(48) : /* "ZIEL: KEIN WEG" */ /* Target: no way */
						g_dtp2))), /* Target: %d BP */
					 5, 190);

			set_textcolor(fg_bak, bg_bak);
		}

		if ((g_mouse_rightclick_event) || (g_action_id == KEY_SCAN_CODE_ESC)) {

			g_mouse_rightclick_event = 0;
			g_action_id = KEY_SCAN_CODE_ENTER;
			problem = 5;
		}

	} while (g_action_id != KEY_SCAN_CODE_ENTER);

	get_textcolor(&fg_bak, &bg_bak);
	set_textcolor(255, 0);
	GUI_print_string(g_string_14spaces, 5, 190);
	set_textcolor(fg_bak, bg_bak);

	if ((*px != sel_x) || (*py != sel_y)) {

		if (!problem || (problem == 1) || (problem == 2)) {

			if (problem == 1) {

				if (GUI_bool(get_tx(35))) { /* "WILLST DU FLIEHEN?" */

					hero->action_id = FIG_ACTION_FLEE;
					problem = 0;

				} else {

					FIG_remove_from_list(g_fig_cb_selector_id[0], 0);
					g_fig_cb_selector_id[0] = -1;
				}
			}

			if (!problem || (problem == 2)) {

				FIG_remove_from_list(g_fig_cb_selector_id[0], 0);
				g_fig_cb_selector_id[0] = -1;

				FIG_prepare_hero_ani(hero, hero_pos);

				if (hero->action_id == FIG_ACTION_FLEE) {

					hero->fight_bp_left = 0;

				} else {
					FIG_search_obj_on_cb(hero_pos + 1, px, py);
				}
			}

		} else if (problem == 3) {

			GUI_output(get_tx(49)); /* "DAS ZIELFELD IST BLOCKIERT" */

		} else if (problem == 4) {

			GUI_output(get_tx(50)); /* "DAS ZIELFELD IST UNERREICHBAR" */
		}
	}

	if (g_fig_cb_selector_id[0] != -1) {
		FIG_remove_from_list(g_fig_cb_selector_id[0], 0);
		g_fig_cb_selector_id[0] = -1;
	}
}
