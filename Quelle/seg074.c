/*
 * seg074 - Automap (Karte von Stadt und Dungeon) und Zielwahl fuer TRANSVERSALIS
 *
 * Inhalt:    show_automap() zeigt die Karte der aktuellen Stadt bzw. des Dungeons mit
 *            allen schon betretenen Feldern (Bitfeld g_automap_buf), die Gruppe als gelben,
 *            andere Gruppen als violetten Pfeil. Grosse Staedte (32x16) werden in
 *            Schritten um je ein Feld seitlich geschoben. select_teleport_dest() nutzt
 *            dieselbe Darstellung, um mit einem Kreuz das Ziel des Zaubers TRANSVERSALIS
 *            zu waehlen, und liefert die AE-Kosten. get_maploc() bestimmt die Farbe eines
 *            Stadtfeldes aus der Ortstabelle.
 * Aufrufer:  show_automap() aus dem Stadt- (seg066) und Dungeonmenue (seg076),
 *            select_teleport_dest() aus spell_transversalis() (seg099). is_discovered()
 *            und die Zeichenfunktionen nur hier.
 * Daten:     g_palette_fight2 (Kartenfarben), g_automap_bitmask, die 7x7-Kacheln fuer
 *            Pfeile und Kreuz, g_automap_selx/sely (Kreuz der Zielwahl), g_automap_buf
 *            (64 Bytes = 16 Zeilen x 32 Bit "schon gesehen", gesetzt in seg002,
 *            gespeichert in seg028).
 * Besonders: render_automap(): bei zwei Gruppen auf einem Feld verdeckt der violette Pfeil
 *            den gelben (Original-Bug 31). Die Sichtpruefungen "<= 16" lassen eine 17.
 *            Spalte rechts neben der Karte zu. select_teleport_dest(): das Kreuz darf bis
 *            Zeile 16 wandern (eine Zeile unter der Karte), dabei liest is_discovered()
 *            hinter g_automap_buf.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg074.c
 */
#include <string.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg004.h"
#include "seg029.h"
#include "seg066.h"
#include "seg074.h"
#include "seg097.h"
#include "seg103.h"

const uint8_t g_palette_fight2[20][3] = {
	{ 0x00, 0x00, 0x00 },
	{ 0x3f, 0x00, 0x00 },
	{ 0x00, 0x3f, 0x00 },
	{ 0x00, 0x00, 0x3f },
	{ 0x3f, 0x3f, 0x00 },
	{ 0x3f, 0x00, 0x3f },
	{ 0x00, 0x3f, 0x3f },
	{ 0x3f, 0x1f, 0x1f },
	{ 0x1f, 0x1f, 0x3f },
	{ 0x0f, 0x1f, 0x0f },
	{ 0x0f, 0x0f, 0x1f },
	{ 0x20, 0x05, 0x00 },
	{ 0x3f, 0x1f, 0x00 },
	{ 0x3f, 0x00, 0x1f },
	{ 0x28, 0x22, 0x00 },
	{ 0x1f, 0x1f, 0x1f },
	{ 0x20, 0x00, 0x33 },
	{ 0x27, 0x13, 0x00 },
	{ 0x00, 0x30, 0x00 },
	{ 0x0a, 0x0a, 0x0a }
}; // ds:0x7d0e
const unsigned char g_automap_bitmask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 }; // ds:0x7d4a
static const unsigned char g_automap_tile_arrowup[49] = { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1 }; // ds:0x7d52
static const unsigned char g_automap_tile_arrowright[49] = { 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 }; // ds:0x7d83
static const unsigned char g_automap_tile_cross[49] = { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 }; // ds:0x7db4
signed int g_automap_selx = -1; // ds:0x7de5
signed int g_automap_sely = -1; // ds:0x7de7
unsigned char g_unkn_055[1] = { 0x00 }; // ds:0x7de9, no users (probably padding)

unsigned char g_automap_buf[64]; // ds:0xe442

/**
 * \brief   shows the automap of the current town or dungeon
 *
 *          Only possible if the dungeon is lit (gs_dungeon_light == 0) or, with
 *          gs_dungeon_light == 1, if the first available hero passes an Orientierung+6
 *          test. With gs_dungeon_light == 2 the heroes cannot orientate at all.
 */
void show_automap(void)
{
	signed int scroll_x;
	signed int answer;
	signed int loc_bak;
	signed int done;
	signed int dungeon_id;
	signed int town_id;
	signed int tw_bak;

	if (!gs_dungeon_light ||
		((gs_dungeon_light == 1) && (test_talent(get_first_hero_available_in_group(), TA_ORIENTIERUNG, 6) > 0)))
	{

		g_special_screen = 1;

		dungeon_id = gs_dungeon_id;
		town_id = gs_town_id;

		gs_town_id = gs_dungeon_id = 0;

		tw_bak = g_textbox_width;
		g_textbox_width = 3;

		scroll_x = (g_map_size_x == 16) ? 0 :
				((gs_x - 8 < 0) ? 0 :
				((gs_x - 8 > 15) ? 16 : gs_x - 8));

		gs_town_id = town_id;
		gs_dungeon_id = dungeon_id;

		g_request_refresh = 1;

		do {

			if (g_request_refresh != 0) {

				loc_bak = gs_town_loc_type;
				/* temporarily != LOCTYPE_NONE, so that draw_main_screen() and the town/dungeon
				 * routines treat the screen as a location screen (vermutlich; LOCTYPE_UNKN1 is used only here) */
				gs_town_loc_type = LOCTYPE_UNKN1;

				draw_main_screen();

				gs_town_loc_type = loc_bak;

				if (g_map_size_x == 16) {
					draw_loc_icons(1, MENU_ICON_LEAVE);
				} else {
					draw_loc_icons(3, MENU_ICON_SCROLL_LEFT, MENU_ICON_SCROLL_RIGHT, MENU_ICON_LEAVE);
				}

				render_automap(scroll_x);
				clear_ani_pal();
				draw_automap_to_screen();
				set_ani_pal((uint8_t*)g_palette_fight2);
				g_request_refresh = 0;
			}

			done = 0;

			handle_gui_input();

			if ((g_mouse_rightclick_event) || (g_action_id == KEY_SCAN_CODE_PAGE_UP)) {

				if (g_map_size_x == 16) {

					answer = GUI_radio(get_ttx(612), 1, get_ttx(613)) - 1; /* "AUTOMAP OPTIONEN" / "ZURÜCK" */

				} else {
					g_menu_default_select = 2;

					answer = GUI_radio(get_ttx(612), 3, get_ttx(614), get_ttx(615), get_ttx(613)) - 1; /* "AUTOMAP OPTIONEN" / "SCHIEBE RECHTS" / "SCHIEBE LINKS" / "ZURÜCK" */
				}

				if (answer != -2) {
					g_action_id = answer + ACTION_ID_ICON_1;
				}
			}

			if (g_map_size_x != 16) {

				if ((g_action_id == ACTION_ID_ICON_1) || (g_action_id == KEY_SCAN_CODE_ARROW_LEFT)) {

					if (scroll_x > 0) {
						render_automap(--scroll_x);
						draw_automap_to_screen();
					}
				}

				if ((g_action_id == ACTION_ID_ICON_2) || (g_action_id == KEY_SCAN_CODE_ARROW_RIGHT)) {

					if (scroll_x < 16) {
						render_automap(++scroll_x);
						draw_automap_to_screen();
					}
				}
			}

			if (((g_action_id == ACTION_ID_ICON_1) && (g_map_size_x == 16)) ||
				((g_action_id == ACTION_ID_ICON_3) && (g_map_size_x != 16)))
			{
				done = 1;
			}
		} while (done == 0);

		g_textbox_width = tw_bak;
		g_request_refresh = 1;

		clear_ani_pal();

	} else {
		GUI_output(get_ttx(772)); /* "IM MOMENT KÖNNT IHR EUCH ABSOLUT NICHT ORIENTIEREN!" */
	}
}

/**
 * \brief   checks if a square has already been seen by the heroes
 *
 * \param   x           x-coordinate (0..31)
 * \param   y           y-coordinate (0..15)
 * \return              != 0 if the bit of square (x,y) in g_automap_buf is set
 */
signed int is_discovered(const signed int x, const signed int y)
{
	return g_automap_buf[(4 * y) + (x >> 3)] & g_automap_bitmask[(x & 7)];
}

/**
 * \brief   get value of a square from a small (16x16) map (dungeon or small town) in g_dng_map
 *
 * \param   x           x-coordinate
 * \param   y           y-coordinate
 * \return              map value of square (x,y)
 */
uint16_t get_mapval_small(const signed int x, const signed int y)
{
	uint8_t *map = g_dng_map;

	return *(map + 16 * y + x);
}

/**
 * \brief   get value of a square from a large (32x16) town map in g_dng_map
 *
 * \param   x           x-coordinate
 * \param   y           y-coordinate
 * \return              map value of square (x,y)
 */
uint16_t get_mapval_large(const signed int x, const signed int y)
{
	uint8_t *map = g_dng_map;

	return *(map + 32 * y + x);
}

/**
 * \brief   checks if the group is in prison
 *
 * \param   group_id    number of the group
 * \return              the value of the "in_prison" flag (hero->jail) of the first hero of that group
 */
signed int is_group_in_prison(const signed int group_id)
{
	struct struct_hero *hero = get_hero(0);
	signed int i;

	for (i = 0; i < 6; i++, hero++) {

		if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == group_id))
		{
			return hero->jail;
		}
	}

	return 0;
}

/**
 * \brief   render the automap in g_renderbuf
 *
 *          Shows 16 columns starting at column x_off. Only squares already seen
 *          (is_discovered()) are drawn.
 *
 * \param   x_off           x offset for horizontal scroll (0..16, only != 0 on large 32x16 town maps)
 */
void render_automap(const signed int x_off)
{
	signed int tile_type;
	signed int group_i;
	signed int x;
	signed int y;
	signed int viewdir_entrance_door;

	g_pic_copy.x1 = 0;
	g_pic_copy.y1 = 0;
	g_pic_copy.x2 = 6;
	g_pic_copy.y2 = 6;

	/* set buffer to 0 */
	memset(g_renderbuf_ptr, 0, 64000);

	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			if (is_discovered(x + x_off, y)) {

				if (gs_dungeon_id != DUNGEON_ID_NONE) {
					/* in dungeon */

					tile_type = div16(get_mapval_small(x, y));

					draw_automap_square(x, y,
						(tile_type <= DNG_TILE_CORRIDOR)? MAP_TILE_DARK_GREY :
							(((tile_type == DNG_TILE_CLOSED_DOOR) || (tile_type == DNG_TILE_SMASHED_DOOR) || tile_type == DNG_TILE_OPEN_DOOR) ? MAP_TILE_DARK_RED :
							((tile_type == DNG_TILE_STAIR_UP) ? MAP_TILE_LIGHT_BLUE :
							((tile_type == DNG_TILE_STAIR_DOWN) ? MAP_TILE_BLUE :
							((tile_type == DNG_TILE_PIT_IN_CEILING) ? MAP_TILE_BRIGHT_GREEN :
							((tile_type == DNG_TILE_CHEST) ? MAP_TILE_BROWN :
							((tile_type == DNG_TILE_PIT) ? MAP_TILE_DARK_GREEN : MAP_TILE_RED)))))), -1);

				} else {
					/* in a town */

					if (!(tile_type = get_maploc(x + x_off, y))) {
						tile_type = get_town_tile_type((g_map_size_x == 16 ? get_mapval_small(x, y) : get_mapval_large(x + x_off, y)));
					}

					draw_automap_square(x, y,
						(tile_type <= TOWN_TILE_STREET)? MAP_TILE_DARK_GREY :
							((tile_type == TOWN_TILE_WATER) ? MAP_TILE_BLUE :
							((tile_type == TOWN_TILE_GRASS) ? MAP_TILE_GREEN :
							((tile_type == TOWN_TILE_SIGNPOST) ? MAP_TILE_RED :
							((tile_type == TOWN_TILE_TEMPLE) ? MAP_TILE_ORANGE :
							((tile_type == TOWN_TILE_INN_OR_TAVERN) ? MAP_TILE_LIGHT_BLUE :
							((tile_type == TOWN_TILE_MERCHANT) ? MAP_TILE_GREY : /* The lightouse on Runin is also displayed in Gray */
							((tile_type == TOWN_TILE_SMITH) ? MAP_TILE_DARK_GREEN :
							((tile_type == TOWN_TILE_HEALER) ? MAP_TILE_PINK :
							((tile_type == TOWN_TILE_BLACK_FINGER) ? MAP_TILE_DARK_PURPLE :
							(((tile_type >= TOWN_TILE_HOUSE_1) && (tile_type <= TOWN_TILE_HOUSE_4)) ? MAP_TILE_DARK_RED : MAP_TILE_BLACK)))))))))), -1);

					if ((tile_type != TOWN_TILE_STREET) && (tile_type != TOWN_TILE_GRASS) && (tile_type != TOWN_TILE_WATER) && (tile_type != TOWN_TILE_SIGNPOST)) {

						viewdir_entrance_door = (g_map_size_x == 16) ?
										get_mapval_small(x, y) :
										get_mapval_large(x + x_off, y);

						viewdir_entrance_door &= 3;
						draw_automap_entrance(x, y, viewdir_entrance_door);
					}
				}
			}
		}
	}

#if !FIX_0082
	/* draw yellow arrow at the position of the active group */

	/* Original-Bug 31: If there is another group on the same map square as the active group, the automap will mark that square by a purple arrow (for the other group) instead of a yellow one (for the active group).
	 * The reason is that the yellow arrow will be overdrawn by the purple ones, which are drawn later. */
	/* REMARK: not always true: the player can scroll the map so that the group is off screen.
	 * The visible columns are 0..15, so "<= 16" should be "< 16": a group in column 16 is drawn
	 * right of the map (same for the purple arrows and the cross below). */
#if FIX_0083
	/* Fix 83: sichtbar sind die Spalten 0..15 */
	if (((gs_x - x_off) >= 0) && ((gs_x - x_off) < 16)) {
#else
	if (((gs_x - x_off) >= 0) && ((gs_x - x_off) <= 16)) { /* shouldn't this always be true? */
#endif

		draw_automap_square(gs_x - x_off, gs_y, MAP_TILE_YELLOW_ARROW, gs_viewdir);
	}
#endif

	/* draw purple arrows at the positions of other groups */
	for (group_i = 0; group_i < 6; group_i++) {

		if ((gs_active_group_id != group_i) &&
			(gs_group_member_counts[group_i] > 0) &&
			(gs_groups_dng_level[group_i] == gs_dungeon_level) &&
			(gs_groups_town_id[group_i] == gs_town_id) &&
			(gs_groups_dungeon_id[group_i] == gs_dungeon_id) &&
			!is_group_in_prison(group_i) &&
			(gs_groups_x[group_i] - x_off >= 0) &&
#if FIX_0083
			(gs_groups_x[group_i] - x_off < 16))
#else
			(gs_groups_x[group_i] - x_off <= 16))
#endif
		{
			draw_automap_square(gs_groups_x[group_i] - x_off,
					gs_groups_y[group_i],
					MAP_TILE_PURPLE_ARROW,
					gs_groups_viewdir[group_i]);
		}
	}

#if FIX_0082
	/* draw yellow arrow at the position of the active group */

	/* Fix 82: gelben Pfeil zuletzt zeichnen (Bright-Eyes-Fix zu Original-Bug 31)
	 * Fix: Move the code block drawing the yellow arrow after the one drawing the purple arrows. */
#if FIX_0083
	/* Fix 83: sichtbar sind die Spalten 0..15 */
	if (((gs_x - x_off) >= 0) && ((gs_x - x_off) < 16)) {
#else
	if (((gs_x - x_off) >= 0) && ((gs_x - x_off) <= 16)) { /* shouldn't this always be true? */
#endif

		draw_automap_square(gs_x - x_off, gs_y, MAP_TILE_YELLOW_ARROW, gs_viewdir);
	}
#endif

	/* In the target selector screen of the Transversalis spell, mark the target with a cross */
#if FIX_0083
	if (((g_automap_selx - x_off) >= 0) && ((g_automap_selx - x_off) < 16)) {
#else
	if (((g_automap_selx - x_off) >= 0) && ((g_automap_selx - x_off) <= 16)) {
#endif

		draw_automap_square(g_automap_selx - x_off, g_automap_sely, MAP_TILE_CROSS, -1);
	}
}

/**
 * \brief   draws one square (7x7 pixels) on the automap: a colored tile, an arrow or the cross
 *          (former brief said "draws a building", but it draws every kind of square)
 *
 * \param   x                  x-coordinate on the automap
 * \param   y                  y-coordiante on the automap
 * \param   map_tile_id        the id of the map tile, which also encodes the color
 * \param   viewdir_arrow      for arrows: the direction
 */
void draw_automap_square(const signed int x, const signed int y, const signed int map_tile_id, const signed int viewdir_arrow)
{
	signed int i;
	signed int offset_y;
	uint8_t* p_img_tile;
	signed char tile[50];

	offset_y = y;
	offset_y <<= 3;
	offset_y *= 320;

	p_img_tile = (uint8_t*)g_renderbuf_ptr + offset_y + 8 * x + 0xca8;

	for (i = 0; i < 49; i++) {
		tile[i] = map_tile_id;
	}

	if ((map_tile_id == MAP_TILE_YELLOW_ARROW) || (map_tile_id == MAP_TILE_PURPLE_ARROW)) {

		if (viewdir_arrow == NORTH) {

			memcpy(g_text_output_buf, g_automap_tile_arrowup, 49);

		} else if (viewdir_arrow == SOUTH) {

			for (i = 0; i < 49; i++) {
				g_text_output_buf[i] = g_automap_tile_arrowup[48 - i];
			}

		} else if (viewdir_arrow == EAST) {

			memcpy(g_text_output_buf, g_automap_tile_arrowright, 49);

		} else {
			// viewdir_arrow == WEST

			for (i = 0; i < 49; i++) {
				g_text_output_buf[i] = g_automap_tile_arrowright[48 - i];
			}
		}

		for (i = 0; i < 49; i++) {
			if (!g_text_output_buf[i]) {
				tile[i] = 0;
			}
		}
	}

	if (map_tile_id == MAP_TILE_CROSS) {

		for (i = 0; i < 49; i++) {
			if (!g_automap_tile_cross[i]) {
				tile[i] = 0;
			} else {
				tile[i] = map_tile_id;
			}
		}
	}

	g_pic_copy.dst = p_img_tile;
	g_pic_copy.src = (uint8_t*)tile;

	do_pic_copy(0);
}

/**
 * \brief   draws the entrance of a automap building
 *
 * \param   x           x-coordinate of the building
 * \param   y           y-coordinate of the building
 * \param   viewdir     direction of the entrance, 0 = NORTH, 1 = EAST,...
 */
void draw_automap_entrance(const signed int x, const signed int y, const signed int viewdir)
{
	signed int offset_y = y;
	signed int d = viewdir;
	signed int skipsize;
	uint8_t *p_img_tile;

	offset_y <<= 3;
	offset_y *= 320;

	p_img_tile = ((uint8_t*)g_renderbuf_ptr) + offset_y + 8 * x + 0xca8;

	if (!d) {
		p_img_tile += 2;
		skipsize = 1;
	} else if (d == 1) {
		p_img_tile += 646;
		skipsize = 320;
	} else if (d == 2) {
		p_img_tile += 1922;
		skipsize = 1;
	} else {
		p_img_tile += 640;
		skipsize = 320;
	}

	/* set 3 pixel to black */
	*p_img_tile = 0;
	*(p_img_tile + skipsize) = 0;
	*(p_img_tile + skipsize + skipsize) = 0;
}

/**
 * \brief   copies the rendered automap from g_renderbuf into the picture area of the screen
 */
void draw_automap_to_screen(void)
{
	/* save screen coordinates */
	struct struct_rect rect_bak = g_pic_copy_rect;

	/* set the screen coordinates */
	g_pic_copy_rect.x1 = g_pic_copy_rect.y1 = 0;
	g_pic_copy_rect.x2 = g_ani_posx + 208;
	g_pic_copy_rect.y2 = g_ani_posy + 135;

	g_pic_copy.src = g_renderbuf_ptr;

	g_pic_copy.x1 = 0;
	g_pic_copy.y1 = 0;
	g_pic_copy.x2 = 320 - 1;
	g_pic_copy.y2 = 135 - 1;

	g_pic_copy.dst = g_vga_memstart + g_ani_posx + 320 * g_ani_posy;

	call_mouse_bg();

	g_pic_copy_flag = 1;
	do_pic_copy(1);
	g_pic_copy_flag = 0;

	call_mouse();

	g_pic_copy.dst = g_vga_memstart;

	/* restore screen coordinates */
	g_pic_copy_rect = rect_bak;
}

/**
 * \brief   select a teleport destination (spell TRANSVERSALIS)
 *
 *          The cross (g_automap_selx/sely) can only be moved onto squares already seen.
 *
 * \return              costs for teleport in AE: 15 for a free square, 0 if the current
 *                      position was kept, -2 for a forbidden target (dungeon wall, town
 *                      house or water; message in g_dtp2)
 */
signed int select_teleport_dest(void)
{
	signed int scroll_x;
#if !defined(__BORLANDC__)
	signed int tile_type;
#endif

	signed int answer;
	signed int done;
	signed int dungeon;
	signed int town;
	signed int ae_costs;
	signed int tw_bak;

	draw_main_screen();

	dungeon = gs_dungeon_id;
	town = gs_town_id;
	gs_town_id = gs_dungeon_id = 0;

	scroll_x = ((g_map_size_x == 16) ? 0 :
			((gs_x - 8 < 0) ? 0 :
			((gs_x - 8 > 15) ? 16 : gs_x - 8)));

	g_automap_selx = gs_x;
	g_automap_sely = gs_y;
	gs_dungeon_id = dungeon;
	gs_town_id = town;
	tw_bak = g_textbox_width;
	g_textbox_width = 3;

	render_automap(scroll_x);

	clear_ani_pal();

	draw_automap_to_screen();

	set_ani_pal((uint8_t*)g_palette_fight2);

	if (g_map_size_x == 16) {
		draw_loc_icons(1, MENU_ICON_MAGIC);
	} else {
		draw_loc_icons(3, MENU_ICON_SCROLL_LEFT, MENU_ICON_SCROLL_RIGHT, MENU_ICON_MAGIC);
	}

	done = 0;

	do {
		handle_input();

		if ((g_mouse_rightclick_event) || (g_action_id == KEY_SCAN_CODE_PAGE_UP)) {

			if (g_map_size_x == 16) {
				answer = GUI_radio(get_ttx(616), 1, get_ttx(617)) - 1; /* "TELEPORTATION" / "DURCHFÜHREN" */
			} else {
				answer = GUI_radio(get_ttx(616), 3, get_ttx(614), get_ttx(615), get_ttx(617)) - 1; /* "TELEPORTATION" / "SCHIEBE RECHTS" / "SCHIEBE LINKS" / "DURCHFÜHREN" */
			}

			if (answer != -2) {
				g_action_id = (answer + ACTION_ID_ICON_1);
			}
		}

		if ((g_action_id == KEY_SCAN_CODE_ARROW_LEFT) &&
			(g_automap_selx > 0) &&
			is_discovered(g_automap_selx - 1, g_automap_sely))
		{
			g_automap_selx--;
			render_automap(scroll_x);
			draw_automap_to_screen();

		} else if ((g_action_id == KEY_SCAN_CODE_ARROW_UP) &&
			(g_automap_sely > 0) &&
			is_discovered(g_automap_selx, g_automap_sely - 1))
		{
			g_automap_sely--;
			render_automap(scroll_x);
			draw_automap_to_screen();

		} else if ((g_action_id == KEY_SCAN_CODE_ARROW_RIGHT) &&
			(g_map_size_x - 1 > g_automap_selx) &&
			is_discovered(g_automap_selx + 1, g_automap_sely))
		{
			g_automap_selx++;
			render_automap(scroll_x);
			draw_automap_to_screen();

		} else if ((g_action_id == KEY_SCAN_CODE_ARROW_DOWN) &&
			/* Original-Bug: should be "< 15". From row 15 is_discovered(x, 16) reads behind
			 * g_automap_buf[64], so the cross may move one row below the map. */
#if FIX_0187
			/* Fix 187: das Kreuz bleibt auf der Karte (Zeilen 0..15) */
			(g_automap_sely < 15) &&
#else
			(g_automap_sely < 16) &&
#endif
			is_discovered(g_automap_selx, g_automap_sely + 1))
		{
			g_automap_sely++;
			render_automap(scroll_x);
			draw_automap_to_screen();
		}

		if (g_map_size_x != 16) {

			if ((g_action_id == ACTION_ID_ICON_1) && (scroll_x > 0)) {
				render_automap(--scroll_x);
				draw_automap_to_screen();
			}

			if ((g_action_id == ACTION_ID_ICON_2) && (scroll_x < 16)) {
				render_automap(++scroll_x);
				draw_automap_to_screen();
			}
		}

		if (((g_action_id == ACTION_ID_ICON_1) && (g_map_size_x == 16)) ||
			((g_action_id == ACTION_ID_ICON_3) && (g_map_size_x != 16)))
		{
			done = 1;
		}


	} while (done == 0);

#if !defined(__BORLANDC__)
	tile_type = (g_map_size_x == 16 ? get_mapval_small(g_automap_selx, g_automap_sely) : get_mapval_large(g_automap_selx, g_automap_sely));

	if (gs_town_id != TOWN_ID_NONE) {
		tile_type = get_town_tile_type(tile_type);
	} else {
		tile_type = div16(tile_type);
	}
#else
	/* REMARK: enforce the use of register DI */
	_DI = (g_map_size_x == 16 ? get_mapval_small(g_automap_selx, g_automap_sely) : get_mapval_large(g_automap_selx, g_automap_sely));

	if (gs_town_id != TOWN_ID_NONE) {
		_DI = get_town_tile_type(_DI);
	} else {
		_DI = div16(_DI);
	}
#endif

	ae_costs = 0;

	/* forbidden: dungeon wall (15) or town tiles 2..6 (houses TOWN_TILE_HOUSE_1..4 and TOWN_TILE_WATER) */
	if ((g_automap_selx == gs_x) && (g_automap_sely == gs_y))
	{
		ae_costs = 0;
		*g_dtp2 = '\0';

	} else
#if !defined(__BORLANDC__)
		if ((gs_dungeon_id && (tile_type == 15)) || ((gs_town_id != TOWN_ID_NONE) && (((tile_type >= 2) && (tile_type <= 5)) || (tile_type == 6))))
#else
		/* REMARK: enforce the use of register DI */
		if ((gs_dungeon_id && (_DI == 15)) || ((gs_town_id != TOWN_ID_NONE) && ((((int16_t)_DI >= 2) && ((int16_t)_DI <= 5)) || (_DI == 6))))
#endif
	{
		strcpy(g_dtp2, get_ttx(611)); /* "IHR KÖNNT NUR AUF FREIE FELDER TELEPORTIEREN!" */
		ae_costs = -2;
	} else {
		*g_dtp2 = '\0';
		ae_costs = 15;
	}

	g_textbox_width = tw_bak;
	g_request_refresh = 1;

	return ae_costs;
}

/**
 * \brief   get location type at given position on map
 *
 * \param   x           x-coordinate of the building
 * \param   y           y-coordinate of the building
 * \return              type of location as TOWN_TILE_* (for the automap color),
 *                      TOWN_TILE_STREET (0) if there is no special location
 */
signed int get_maploc(const signed int x, const signed int y)
{
	struct location *locations_tab_ptr;
	const signed int pos_xy = TOWN_POS(x,y);

	// Wow. Original game has these hard-coded manipulation of the data.
	if (gs_town_id == TOWN_ID_THORWAL) {

		if (pos_xy == TOWN_POS(4,13)) {
			// schwarzer Finger
			return TOWN_TILE_BLACK_FINGER;
		} else if (pos_xy == TOWN_POS(5,2)) {
			// town exit to Vaermhag, which is located in a building (not a signpost).
			return TOWN_TILE_SIGNPOST;
		} else if ((pos_xy == TOWN_POS(5,1))
		        || (pos_xy == TOWN_POS(5,4) || (pos_xy == TOWN_POS(3,6)))
		        || (pos_xy == TOWN_POS(13,8) || (pos_xy == TOWN_POS(20,11)))
		        || (pos_xy == TOWN_POS(5,5) || (pos_xy == TOWN_POS(3,10))))
		{
			return TOWN_TILE_INN_OR_TAVERN;
		}
	} else if (gs_town_id == TOWN_ID_PREM) {
		if (pos_xy == TOWN_POS(28,9)) {
			// Inn "Zur Trutz". Why is this a special case here?
			return TOWN_TILE_INN_OR_TAVERN;
		}
	} else if (gs_town_id == TOWN_ID_GUDDASUNDEN) {
		if (pos_xy == TOWN_POS(1,14)) {
			// Harbor, which is located in a building (not a signpost).
			return TOWN_TILE_SIGNPOST;
		}
	}

	locations_tab_ptr = &g_locations_tab[0];

	do {

		if (locations_tab_ptr->pos == pos_xy) {

			if (locations_tab_ptr->loctype == LOCTYPE_TEMPLE) {
				return TOWN_TILE_TEMPLE;
			}
			if (locations_tab_ptr->loctype == LOCTYPE_MERCHANT) {
				return TOWN_TILE_MERCHANT;
			}
			if (locations_tab_ptr->loctype == LOCTYPE_SMITH) {
				return TOWN_TILE_SMITH;
			}
			if ((locations_tab_ptr->loctype == LOCTYPE_TAVERN) || (locations_tab_ptr->loctype == LOCTYPE_INN)) {
				return TOWN_TILE_INN_OR_TAVERN;
			}
			if (locations_tab_ptr->loctype == LOCTYPE_HEALER) {
				return TOWN_TILE_HEALER;
			}
		}

		locations_tab_ptr++;

	} while (locations_tab_ptr->pos != -1);

	return TOWN_TILE_STREET;
}
