/*
 * seg097 - Textboxen, Auswahlmenues und Texteingabe
 *
 * Inhalt:    Zeichnet die Popup-Textboxen und wertet sie aus: GUI_output und
 *            GUI_input (Meldung, Zahlen- oder Texteingabe), GUI_bool (Ja/Nein,
 *            auch mit den Tasten J und N), GUI_radio (Auswahlliste), GUI_dialogbox
 *            (Box mit Bild und Namen, z. B. Gespraeche), dazu die Menuesteuerung
 *            per Maus und Pfeiltasten. Ausserdem die Kampf-Einleitungstexte der
 *            Dungeons und die Breitentabelle einer zweiten, 8 Pixel hohen Schrift.
 * Aufrufer:  fast alle Module (jede Meldung, jede Frage im Spiel).
 * Daten:     g_gui_char_width_font8 (in Wahrheit Zeichenbreiten der 8-Pixel-Schrift),
 *            g_gui_bool_flag (Ja/Nein-Modus), g_menu_default_select/_selected.
 * Besonders: GUI_print_string_font8, GUI_print_char_font8 und GUI_write_font8_to_screen werden nie aufgerufen; sie
 *            holen ihre Glyphen aus g_splash_buffer, in dem heute die Splash-
 *            Grafiken liegen (FONT8 wird nirgends geladen). GUI_enter_string()
 *            setzt beim Bearbeiten eines Spielstandnamens die Schreibstelle eine
 *            Position zu weit (vermutlich Original-Fehler, siehe dort).
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg097.c
 */

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "v302de.h"
#include "common.h"

#include "seg001.h"
#include "seg002.h"
#include "seg004.h"
#include "seg026.h"

#include "seg078.h"
#include "seg080.h"
#include "seg081.h"
#include "seg084.h"
#include "seg086.h"
#include "seg088.h"
#include "seg090.h"
#include "seg091.h"

#include "seg096.h"
#include "seg097.h"

static const struct struct_char_height g_gui_char_width_font8[67] = {
	{        ' ',  0, 8 },
	{        'A',  1, 8 },
	{        'B',  2, 8 },
	{        'C',  3, 7 },
	{        'D',  4, 7 },
	{        'E',  5, 7 },
	{        'F',  6, 7 },
	{        'G',  7, 8 },
	{        'H',  8, 8 },
	{        'I',  9, 6 },
	{        'J', 10, 8 },
	{        'K', 11, 8 },
	{        'L', 12, 6 },
	{        'M', 13, 8 },
	{        'N', 14, 8 },
	{        'O', 15, 8 },
	{        'P', 16, 7 },
	{        'Q', 17, 8 },
	{        'R', 18, 8 },
	{        'S', 19, 7 },
	{        'T', 20, 7 },
	{        'U', 21, 8 },
	{        'V', 22, 8 },
	{        'W', 23, 8 },
	{        'X', 24, 7 },
	{        'Y', 25, 7 },
	{        'Z', 26, 7 },
	{ (char)0x8e, 27, 8 },
	{ (char)0x99, 28, 8 },
	{ (char)0x9a, 29, 8 },
	{        ',', 30, 5 },
	{        '.', 31, 4 },
	{        '-', 32, 6 },
	{        '(', 33, 6 },
	{        ')', 34, 6 },
	{        '!', 35, 4 },
	{        '?', 36, 8 },
	{        '0', 37, 8 },
	{        '1', 38, 6 },
	{        '2', 39, 8 },
	{        '3', 40, 8 },
	{        '4', 41, 8 },
	{        '5', 42, 8 },
	{        '6', 43, 8 },
	{        '7', 44, 7 },
	{        '8', 45, 8 },
	{        '9', 46, 8 },
	{       '\'', 47, 5 },
	{        ':', 48, 5 },
	{ (char)0xc8, 49, 8 },
	{ (char)0xc9, 50, 8 },
	{ (char)0xca, 51, 6 },
	{ (char)0xcb, 52, 5 },
	{ (char)0xcc, 53, 6 },
	{ (char)0xcd, 54, 6 },
	{ (char)0xce, 55, 7 },
	{ (char)0xcf, 56, 6 },
	{ (char)0xd0, 57, 3 },
	{ (char)0xd1, 58, 8 },
	{ (char)0xd2, 59, 8 },
	{ (char)0xd3, 60, 6 },
	{ (char)0xd4, 61, 8 },
	{ (char)0xd5, 62, 5 },
	{ (char)0xd6, 63, 8 },
	{ (char)0xd7, 64, 6 },
	{ (char)0xd8, 65, 8 },
	{ (char)0xd9, 66, 6 }
}; // ds:0xab42
/* Name irrefuehrend: die 3. Spalte ist die Zeichenbreite (nicht Hoehe) einer zweiten,
 * 8 Pixel hohen Schrift (vgl. g_gui_char_width in seg096). Nur ueber
 * GUI_lookup_char_width_font8() von totem Code benutzt (GUI_print_string_font8, dir = 1). */
static signed int g_gui_bool_flag = 0; // ds:0xac0b


signed int g_menu_default_select;	// ds:0xe5ac
signed int g_menu_input_busy;		// ds:0xe5ae
signed int g_menu_selected;		// ds:0xe5b0

#if defined(__BORLANDC__)
/* wie GUI_print_string(), aber mit der 8-Pixel-Schrift (Zeilenabstand 10); wird nie aufgerufen */
void GUI_print_string_font8(char *str, signed int x, signed int y)
{
	signed int i = 0;
	signed int x_bak;
	signed char c;

	call_mouse_bg();

	if (g_gui_text_centered == 1) {
		x = GUI_get_first_pos_centered(str, x, g_textline_maxlen, 1);
	}

	x_bak = x;

	while ((c = str[i++])) {

		if ((c == 0x0d) || (c == '@')) {

			y += 10;

			x = (g_gui_text_centered == 1) ?
				GUI_get_first_pos_centered(str + i, g_textline_posx, g_textline_maxlen, 1) : x_bak;

		} else if (c == '~') {
			if (x < g_txt_tabpos[0]) {
				x = g_txt_tabpos[0];
			} else if (x < g_txt_tabpos[1]) {
				x = g_txt_tabpos[1];
			} else if (x < g_txt_tabpos[2]) {
				x = g_txt_tabpos[2];
			} else if (x < g_txt_tabpos[3]) {
				x = g_txt_tabpos[3];
			} else if (x < g_txt_tabpos[4]) {
				x = g_txt_tabpos[4];
			} else if (x < g_txt_tabpos[5]) {
				x = g_txt_tabpos[5];
			} else if (x < g_txt_tabpos[6]) {
				x = g_txt_tabpos[6];
			}
		} else if ((c == (signed char)0xf0) || (c == (signed char)0xf1) || (c == (signed char)0xf2) || (c == (signed char)0xf3)) {
			g_textcolor_index = (unsigned char)c + 0xff10;
		} else {
			x += GUI_print_char_font8(c, x, y);
		}
	}

	call_mouse();
}

/* wie GUI_print_char(), aber mit der 8-Pixel-Schrift; nur von GUI_print_string_font8() aufgerufen */
signed int GUI_print_char_font8(const int c, const int x, const int y)
{
	signed int char_width;
	const signed int font_index = GUI_lookup_char_width_font8((signed char)c, &char_width);

	GUI_write_font8_to_screen(font_index, char_width, x, y);

	return char_width;
}
#endif

/* wie GUI_lookup_char_width(), aber fuer die 8-Pixel-Schrift: liefert die Glyphennummer,
 * *p = Zeichenbreite (nicht Hoehe; unbekannte Zeichen 8). */
signed int GUI_lookup_char_width_font8(const signed char c, signed int *p)
{
	signed int i;

	for (i = 0; i != 201; i += 3) {
		if (c == ((signed char*)g_gui_char_width_font8)[i]) {

			*p = ((signed char*)g_gui_char_width_font8)[i + 2] & 0xff;
			return ((signed char*)g_gui_char_width_font8)[i + 1] & 0xff;
		}
	}

	if ((c == '~') || (c == (signed char)0xf0) || (c == (signed char)0xf1) || (c == (signed char)0xf2) || (c == (signed char)0xf3))
	{
		return *p = 0;
	}

	*p = 8;

	return 0;
}

#if defined(__BORLANDC__)
/* zeichnet eine Glyphe der 8-Pixel-Schrift (8 Zeilen hoch). Die Glyphen werden aus
 * g_splash_buffer gelesen, wo aber die Splash-Grafiken liegen - toter Code. */
void GUI_write_font8_to_screen(const signed int index, const signed int char_width, const signed int x, const signed int y)
{

	GUI_blank_char();
	GUI_font_to_buf(g_splash_buffer + index * 8);
	GUI_write_char_to_screen_xy(x, y, 8, char_width);
}
#endif

/**
 * \brief   Texteingabe in ein Feld fester Laenge (6 Pixel je Zeichen)
 *
 * \param   dst         Zielpuffer; beim Spielstandnamen (g_gui_entering_savegame)
 *                      mit dem alten Namen vorbelegt
 * \param   x, y        Bildschirmposition des Feldes
 * \param   max_len     Feldlaenge in Zeichen (vorher num)
 * \param   cursor_mode 0: ganzes Feld mit '_' vorzeichnen; 1: nur ein '_' an der
 *                      Schreibstelle (vorher zero; wird nie benutzt, der einzige
 *                      Aufrufer GUI_input() uebergibt 0)
 * \return  0 = mit Enter beendet, -1 = ESC (dst wird geleert)
 */
//should be static
signed int GUI_enter_string(char* dst, signed int x, const signed int y, const signed int max_len, const signed int cursor_mode)
{
	signed int x_pos;
	register signed int si;
	signed int pos;
	signed int c;
	char *dst_start = dst;
	signed int length;

	call_mouse_bg();

	x_pos = x;

	pos = 0;

	length = strlen((char*)dst);

	if (cursor_mode == 0) {
		for (si = 0; si < max_len; si++) {
			/* vermutlich Original-Fehler: '>=' statt '>' gibt auch das Endezeichen 0 aus
			 * (als Leerstelle) und setzt pos und dst eine Stelle hinter das Namensende.
			 * Danach getippte Zeichen landen hinter der 0 und fehlen im Ergebnis,
			 * solange man nicht zuerst die Ruecktaste drueckt. */
			if ((g_gui_entering_savegame) && (length >= si)) {
				GUI_print_char(' ', x_pos, y);
				GUI_print_char(*dst, x_pos, y);
				pos++;
				dst++;
				x_pos += 6;
				x = x_pos;
			} else {
				GUI_print_char(' ', x_pos, y);
				GUI_print_char('_', x_pos, y);
				x_pos += 6;
			}
		}
		x_pos = x;
	} else {
		GUI_print_char(' ', x_pos, y);
		GUI_print_char('_', x_pos, y);
	}

	flush_keyboard_queue();
	g_mouse_leftclick_event = 0;

	c = ASCII_CODE_NONE;
	while ((c != ASCII_CODE_ENTER) || (pos == 0)) {
		do {
			goto dummy;
dummy:

			/* wait till keypress or left mouse button */
			/* This loop is evil */
			do {;} while ((CD_bioskey(1) == 0) && !g_mouse_leftclick_event);

			if (g_mouse_leftclick_event) {
				g_keystroke_ascii_code = ASCII_CODE_ENTER; /* 0x0d is ASCII code of [Enter] key */
				g_mouse_leftclick_event = g_mouse1_event2 = 0;
			} else {
				/* bioskey(0) writes 2 byte keystroke code to g_keystroke_ascii_code and removes it from the buffer.
				 * g_action_id stores higher byte   (hardware keyscan code)
				 * g_keystroke_ascii_code stores lower byte (ASCII code)
				 */
				g_action_id = (g_keystroke_ascii_code = bioskey(0)) >> 8;
				g_keystroke_ascii_code &= 0xff;
			}

		} while ((g_action_id == ACTION_ID_NONE) && (g_keystroke_ascii_code == ASCII_CODE_NONE));

		c = g_keystroke_ascii_code;

		if (c == ASCII_CODE_ENTER) {

		} else if (g_action_id == KEY_SCAN_CODE_ESC) {
			*dst_start = 0;
			call_mouse();
			g_action_id = 0;
			return -1;
		} else if (c == ASCII_CODE_BACKSPACE) {
			if (pos > 0) {

				if ((cursor_mode == 1) && (pos != max_len)) {
					GUI_print_char(' ', x_pos, y);
				}

				x_pos -= 6;
				pos--;
				dst--;
				GUI_print_char(' ', x_pos, y);
				GUI_print_char('_', x_pos, y);
			}
		} else

		/* check if character is valid */
		if (
			((c >= ' ') && (c <= 'z'))
			|| (c == ASCII_CODE_LOWERCASE_UE) || (c == ASCII_CODE_LOWERCASE_AE) || (c == ASCII_CODE_LOWERCASE_OE)
		) {
			if (isalpha(c)) c = toupper(c);

			/* ae */
			if (c == ASCII_CODE_LOWERCASE_AE)
				c = (signed char)ASCII_CODE_UPPERCASE_AE;
			/* oe */
			if (c == ASCII_CODE_LOWERCASE_OE)
				c = (signed char)ASCII_CODE_UPPERCASE_OE;
			/* ue */
			if (c == ASCII_CODE_LOWERCASE_UE)
				c = (signed char)ASCII_CODE_UPPERCASE_UE;

			/* are we at the end of the input field ? */
			if (pos == max_len) {
				dst--;
				x_pos -= 6;
				pos--;
			}

			*dst++ = (unsigned char)c;
			GUI_print_char(' ', x_pos, y);
			GUI_print_char((unsigned char)c, x_pos, y);
			x_pos += 6;
			pos++;

			if ((cursor_mode == 1) && (pos != max_len)) {
				GUI_print_char(' ', x_pos, y);
				GUI_print_char('_', x_pos, y);
			}
		}
	}

	if (cursor_mode == 0) {
		while (pos < max_len) {
			GUI_print_char(' ', x_pos, y);
			x_pos += 6;
			pos++;
		}
	}

	*dst = 0;
	call_mouse();

	return 0;
}

/**
 * \brief   sichert den Hintergrund nach g_gui_box_bg_buf und zeichnet den Popup-Rahmen
 *
 * \param   header      Zahl der Textzeilen
 * \param   options     Zahl der Auswahlzeilen (mit Radio-Knopf)
 */
//static
void GUI_draw_popup_box(const signed int header, const signed int options, const signed int width, const signed int height)
{
	signed int i;

	/* set upper left coordinates */
	g_pic_copy.x1 = g_textbox_pos_x;
	g_pic_copy.y1 = g_textbox_pos_y;
	/* set lower right coordinates */
	g_pic_copy.x2 = g_textbox_pos_x + width - 1;
	g_pic_copy.y2 = g_textbox_pos_y + height - 1;
	/* set pointer */
	g_pic_copy.src = g_gui_box_bg_buf;
	do_save_rect();

	/* play FX3.VOC */
	if (!g_pregame_state)
		play_voc(ARCHIVE_FILE_FX3_VOC);

	GUI_draw_popup_line(0, 0);

	for (i = 0; i < header; i++)
		GUI_draw_popup_line(i + 1, 1);

	if (options != 0)
		for (i = 0; i < options; i++)
			GUI_draw_popup_line(header + i + 1, 2);

	GUI_draw_popup_line(header + options + 1, 3);

	set_textcolor(0xff, 0xdf);
	flush_keyboard_queue_alt();
}

/* stellt den von GUI_draw_popup_box() gesicherten Hintergrund wieder her (schliesst die Box) */
void GUI_restore_box_bg(const signed int width, const signed int height)
{
	g_pic_copy.x1 = g_textbox_pos_x;
	g_pic_copy.y1 = g_textbox_pos_y;
	g_pic_copy.x2 = g_textbox_pos_x + width - 1;
	g_pic_copy.y2 = g_textbox_pos_y + height - 1;
	g_pic_copy.src = g_gui_box_bg_buf;
	do_pic_copy(0);
}

void GUI_output(char *str)
{
	GUI_input(str, 0);
}

/**
 * \brief   Textbox mit Meldung, optional mit Eingabefeld
 *
 * \param   digits  0: nur Meldung (wartet auf Taste bzw. Zeit); >0: Eingabefeld mit so
 *                  vielen Zeichen, Rueckgabe atol() der Eingabe (-1 bei ESC). Auch fuer
 *                  Texteingaben benutzt (Spielstandname), Ergebnis dann in g_text_input_buf.
 */
signed int GUI_input(char *str, const signed int digits)
{
	signed int retval = 0;
	signed int height;
	signed int fg_bak;
	signed int bg_bak;
	signed int tlx_bak;
	signed int tly_bak;
	signed int ml_bak;
	signed int wc_bak;
	signed int usl_bak;
	signed int lines;
	signed int width;

	usl_bak = g_update_statusline;
	g_update_statusline = 0;

	if (!str || !(*str) || g_autofight != 0)
		return -1;

	wc_bak = g_wallclock_update;
	g_wallclock_update = 0;
	g_dialogbox_lock = 1;
	g_gui_text_centered = 1;

	tlx_bak = g_textline_posx;
	tly_bak = g_textline_posy;
	ml_bak = g_textline_maxlen;

	width = (g_textbox_width * 32) + 32;
	g_textbox_pos_x = ((signed int)(320u - width) >> 1) + g_basepos_x;

	g_textline_posx = g_textbox_pos_x + 5;
	g_textline_maxlen = width - 8;

	lines = GUI_count_lines(str);

	if (digits != 0)
		lines += 2;

	height = (lines + 2) * 8;

	g_textbox_pos_y = ((signed int)(200u - height) >> 1) + g_basepos_y;
	g_textline_posy = g_textbox_pos_y + 7;

	get_textcolor(&fg_bak, &bg_bak);

	call_mouse_bg();

	GUI_draw_popup_box(lines, 0, width, height);

	GUI_print_text(str);

	g_mouse_rightclick_event = 0;

	call_mouse();

	if (digits != 0) {
		if (GUI_enter_string(g_text_input_buf, g_textbox_pos_x + ((signed int)(width - digits * 6) >> 1), g_textbox_pos_y + lines * 8 -2, digits, 0) != -1) {
			retval = (signed int)atol(g_text_input_buf);
		} else {
			retval = -1;
		}
	} else {
		/* set action table */
		g_action_table_secondary = &g_action_table_menu[0];

		if (g_game_paused_flag != 0) {
			wait_for_keypress();
		} else {
			vsync_or_key(lines * g_textbox_width * 50);
		}

		/* delete action table */
		g_action_table_secondary = NULL;
	}

	set_textcolor(fg_bak, bg_bak);

	call_mouse_bg();

	GUI_restore_box_bg(width, height);

	call_mouse();

	g_textline_posx = tlx_bak;
	g_textline_posy = tly_bak;
	g_textline_maxlen = ml_bak;

	g_action_id = 0;
	g_dialogbox_lock = 0;

	g_wallclock_update = wc_bak;
	g_gui_text_centered = 0;
	g_update_statusline = usl_bak;

	return retval;
}

/**
 * \brief   shows a text box with two radio buttons "Ja" (yes) and "Nein" (no).
 *
 * \param   header	the displayed text (Parametername berichtigt, vorher "text")
 * \return  1 = Ja, 0 = Nein oder Abbruch
 */
signed int GUI_bool(char *header)
{
	signed int ret_radio;

	g_gui_bool_flag = 1;

	ret_radio = GUI_radio(header, 2, get_ttx(2), get_ttx(3)); /* "JA" / "NEIN" */

	g_gui_bool_flag = 0;

	return (ret_radio == 1) ? 1 : 0;
}


/**
 * \brief   fuellt den Radio-Knopf der Auswahl new_pos (4x4 Pixel) und loescht den von old_pos
 *
 * \param   offset  Zeile der ersten Auswahl in der Box (in 8-Pixel-Zeilen)
 *
 * Die lokalen Namen x und y waren vertauscht (y war die x-Koordinate fuer do_v_line()).
 */
//static
void GUI_fill_radio_button(const signed int old_pos, const signed int new_pos, const signed int offset)
{
	signed int i;
	signed int y;
	signed int x;

	call_mouse_bg();

	x = g_textbox_pos_x + 6;

	if (old_pos != -1) {

		y = g_textbox_pos_y + (offset + old_pos) * 8 + 2;

		/* clear the old button */
		for (i = 0; i < 4; i++)
			do_v_line(g_vga_memstart, x + i, y, y + 3, (signed char)0xd8);
	}

	y = g_textbox_pos_y + (offset + new_pos) * 8 + 2;

	/* fill the new button */
	for (i = 0; i < 4; i++)
		do_v_line(g_vga_memstart, x + i, y, y + 3, (signed char)0xd9);

	call_mouse();
}

/**
 * \brief   Dialogbox mit Bild (32x32), Namen, Text und options Antworten (variable Argumente)
 *
 * \return  gewaehlte Antwort 1..options, -1 bei Abbruch oder ohne Antworten
 */
signed int GUI_dialogbox(uint8_t* picture, char *name, char *text, signed int options, ...)
{
	va_list arguments;
	signed int i;
	signed int opt_x;
	signed int opt_y;
	signed int lines_total;
	signed int height;
	signed int tw_bak;
	signed int fg_bak;
	signed int bg_bak;
	signed int x_bak;
	signed int y_bak;
	signed int ml_bak;
	signed int tab0_bak;
	signed int retval;
	signed int wc_bak;
	signed int usl_bak;
	const signed int ani_bak = g_ani_enabled;
	signed int lines;
	signed int width;

	usl_bak = g_update_statusline;
	g_update_statusline = 0;

	disable_ani();

	wc_bak = g_wallclock_update;
	g_wallclock_update = 0;
	g_dialogbox_lock = 1;
	x_bak = g_textline_posx;
	y_bak = g_textline_posy;
	ml_bak = g_textline_maxlen;
	tw_bak = g_textbox_width;
	g_textbox_width = 9;

	width = g_textbox_width * 32 + 32;
	g_textbox_pos_x = ((signed int)(320 - width) >> 1) + g_basepos_x;
	g_textline_posx = g_textbox_pos_x + 5;
	g_textline_maxlen = width - 8;
	tab0_bak = g_txt_tabpos[0];
	g_txt_tabpos[0] = (g_textline_posx + g_textline_maxlen - 24);
	g_dialogbox_indent_width = 40;
	g_dialogbox_indent_height = 5;

	lines = GUI_count_lines(text) - 1;

	if (name)
		lines += 2;

	if (lines < g_dialogbox_indent_height)
		lines = g_dialogbox_indent_height - 1;

	lines_total = lines + (signed char)options;
	height = (lines_total + 2) * 8;
	g_textbox_pos_y = (200 - (height + 2)) >> 1;
	g_textline_posy = g_textbox_pos_y + 5;

	call_mouse_bg();
	get_textcolor(&fg_bak, &bg_bak);

	GUI_draw_popup_box(lines, (signed char)options, width, height);

	if (picture != 0) {

		/* draw a frame */
		do_border(g_vga_memstart, g_textbox_pos_x + 5, g_textbox_pos_y + 6, g_textbox_pos_x + 38, g_textbox_pos_y + 39, (signed char)0xff);

		/* set the coordinates */
		g_pic_copy.x1 = g_textbox_pos_x + 6;
		g_pic_copy.y1 = g_textbox_pos_y + 7;
		g_pic_copy.x2 = g_textbox_pos_x + 37;
		g_pic_copy.y2 = g_textbox_pos_y + 38;
		g_pic_copy.src = picture;

		do_pic_copy(0);
	}

	if (name) {
		/* set text color */
		g_textcolor_index = 1;

		GUI_print_string(name, g_textline_posx, g_textline_posy);

		/* set text color */
		g_textcolor_index = 0;

		g_textline_posy += 14;
		g_dialogbox_indent_height -= 2;
	}

	if (lines != 0) {
		GUI_print_text(text);
	}

	g_dialogbox_indent_width = g_dialogbox_indent_height = 0;

	if ((signed char)options != 0) {

		opt_x = g_textline_posx + 8;
		opt_y = g_textbox_pos_y + (lines + 1) * 8;

		va_start(arguments, options);
		for (i = 0; i < (signed char)options; opt_y += 8, i++) {
			GUI_print_string((char*)va_arg(arguments, char*), opt_x, opt_y);
		}
	}

	va_end(arguments);

	retval = GUI_menu_input((signed char)options, lines + 1, width);

	GUI_restore_box_bg(width, height);

	call_mouse();
	set_textcolor(fg_bak, bg_bak);

	g_textline_posx = x_bak;
	g_textline_posy = y_bak;
	g_textline_maxlen = ml_bak;

	g_textbox_width = tw_bak;

	g_txt_tabpos[0] = tab0_bak;

	g_wallclock_update = wc_bak;

	g_action_id = g_dialogbox_lock = 0;

	g_update_statusline = usl_bak;

	if (ani_bak != 0)
		init_ani(2);

	return retval;
}

/**
 * \brief   Auswahl in einer offenen Box per Maus, Pfeiltasten, Enter/ESC (bei GUI_bool auch J/N)
 *
 * \param   positions   Zahl der Auswahlzeilen (0: nur auf Taste warten)
 * \param   h_lines     Zeile der ersten Auswahl in der Box
 * \return  1..positions, -1 bei Abbruch
 */
//static
signed int GUI_menu_input(const signed int positions, const signed int h_lines, const signed int width)
{
	signed int opt_y0;
	signed int mouse_y_last;
	signed int mouse_x_bak;
	signed int mouse_y_bak;
	signed int sel_drawn = -1;
	signed int height;
	signed int done = 0;
	signed int retval;

	g_menu_selected = g_menu_input_busy = 1;
	g_menu_selected += g_menu_default_select;

	if (positions != 0) {
		height = h_lines * 8;
		mouse_x_bak = g_mouse_posx;
		mouse_y_bak = g_mouse_posy;
		g_mouse_posx_bak = g_mouse_posx = g_textbox_pos_x + 90;
		opt_y0 = g_textbox_pos_y + height;

		g_mouse_posy_bak = g_mouse_posy = mouse_y_last = opt_y0 + 8 * g_menu_default_select;

		mouse_move_cursor(g_mouse_posx, g_mouse_posy);

		g_mouse_posx_max = g_textbox_pos_x + width - 16;
		g_mouse_posx_min = g_textbox_pos_x;
		g_mouse_posy_min = g_textbox_pos_y + height;

		g_mouse_posy_max = height + g_textbox_pos_y - 1 + positions * 8;
		call_mouse();

		g_mouse1_event2 = g_mouse_leftclick_event = g_mouse_rightclick_event = 0;

		while (!done) {
			g_action_table_secondary = &g_action_table_menu[0];
			handle_input();
			g_action_table_secondary = NULL;

			if (sel_drawn != g_menu_selected) {
				GUI_fill_radio_button(sel_drawn, g_menu_selected, h_lines - 1);
				sel_drawn = g_menu_selected;
			}

			if (g_mouse_rightclick_event || g_action_id == KEY_SCAN_CODE_ESC || g_action_id == KEY_SCAN_CODE_PAGE_DOWN) {
				/* close menu */

				retval = -1;
				done = 1;
				g_mouse_rightclick_event = 0;
			}

			if (g_action_id == KEY_SCAN_CODE_ENTER) {
				retval = g_menu_selected;
				done = 1;
			}

			/* TODO: ... */
			if (g_action_id == KEY_SCAN_CODE_ARROW_UP) {
				if (g_menu_selected-- == 1)
					g_menu_selected = positions;
			}

			if (g_action_id == KEY_SCAN_CODE_ARROW_DOWN) {
				if (g_menu_selected++ == positions)
					g_menu_selected = 1;
			}

			if (g_mouse_posy != mouse_y_last) {
				mouse_y_last = g_mouse_posy;
				g_menu_selected = ((mouse_y_last - opt_y0) >> 3) + 1;
			}

			if (g_gui_bool_flag) {

				/* in yes-no-mode, answer "Ja" (yes) can be selected with the 'J' key,
				 * and answer "Nein" (no) can be selected with the 'N' key. */
				if (g_action_id == KEY_SCAN_CODE_Y) {

					retval = done = 1;

				} else if (g_action_id == KEY_SCAN_CODE_N) {

					retval = 2;
					done = 1;
				}
			}
		}

		call_mouse_bg();

		g_mouse_posx_bak = g_mouse_posx = mouse_x_bak;
		g_mouse_posy_bak = g_mouse_posy = mouse_y_bak;
		g_mouse_posx_max = 319;
		g_mouse_posx_min = 0;
		g_mouse_posy_min = 0;
		g_mouse_posy_max = 199;

		mouse_move_cursor(g_mouse_posx, g_mouse_posy);

	} else {
		do {
			vsync_or_key(10000);

		} while (g_action_id == 0);

		retval = -1;
	}

	g_menu_default_select = g_menu_input_busy = 0;

	return retval;
}

#if defined(__BORLANDC__)
signed int GUI_radio(char *header, const signed char options, ...)
#else
signed int GUI_radio(char *header, const signed int options, ...)
#endif
{
	signed int i;
	signed int lines_header;

	va_list arguments;
	signed int opt_x;
	signed int opt_y;
	signed int lines;
	signed int height;
	signed int fg_bak;
	signed int bg_bak;
	signed int x_bak;
	signed int y_bak;
	signed int ml_bak;
	signed int tab0_bak;
	signed int width;
	signed int retval;
	const signed int usl_bak = g_update_statusline;

	g_update_statusline = 0;

	if (!options) {
		GUI_output(header);
		return 0;
	}

	g_dialogbox_lock = 1;
	x_bak = g_textline_posx;
	y_bak = g_textline_posy;
	ml_bak = g_textline_maxlen;

	width = g_textbox_width * 32 + 32;
	g_textbox_pos_x = ((320 - width) >> 1) + g_basepos_x;
	g_textline_posx = g_textbox_pos_x + 5;
	g_textline_maxlen = width - 8;

	tab0_bak = g_txt_tabpos[0];
	g_txt_tabpos[0] = g_textline_posx + g_textline_maxlen - 24;

	lines_header = GUI_count_lines(header);
	lines = lines_header + options;
	height = (lines + 2) * 8;
	g_textbox_pos_y = ((200 - height + 2) >> 1) + g_basepos_y;
	g_textline_posy = g_textbox_pos_y + 7;

	call_mouse_bg();
	get_textcolor(&fg_bak, &bg_bak);

	GUI_draw_popup_box(lines_header, options, width, height);

	if (lines_header != 0)
		GUI_print_text(header);

	opt_x = g_textline_posx + 8;
	opt_y = g_textbox_pos_y + (lines_header + 1) * 8;

	va_start(arguments, options);
	for (i = 0; i < options; opt_y += 8, i++) {

		/* highlight special option */
		if ((g_game_mode == GAME_MODE_BEGINNER) && (g_talented_hero_pos == i))
			set_textcolor(0xc9, 0xdf);

		GUI_print_string((char*)va_arg(arguments, char*), opt_x, opt_y);

		/* reset highlight special option */
		if ((g_game_mode == GAME_MODE_BEGINNER) && (g_talented_hero_pos == i))
			set_textcolor(0xff, 0xdf);
	}

	retval = GUI_menu_input(options, lines_header + 1, width);

	GUI_restore_box_bg(width, height);
	call_mouse();
	set_textcolor(fg_bak, bg_bak);

	g_textline_posx = x_bak;
	g_textline_posy = y_bak;
	g_textline_maxlen = ml_bak;
	g_txt_tabpos[0] = tab0_bak;
	g_action_id = g_dialogbox_lock = 0;
	g_update_statusline = usl_bak;

	return retval;
}

/**
 * \brief   shows a fight intro message
 *
 * \param   fight_id    ID of the fight
 */
void GUI_print_fight_intro_msg(const signed int fight_id)
{
	const signed int tw_bak = g_textbox_width;
	g_textbox_width = 7;

	if (gs_dungeon_id == DUNGEON_ID_VERFALLENE_HERBERGE) {
		DNG02_fight_intro(fight_id);
	}
	if (gs_dungeon_id == DUNGEON_ID_GOBLINHOEHLE) {
		DNG05_fight_intro(fight_id);
	}
	if (gs_dungeon_id == DUNGEON_ID_DASPOTASCHATZ) {
		DNG06_fight_intro(fight_id);
	}
	if (gs_dungeon_id == DUNGEON_ID_KULTSTAETTE_DES_NAMENLOSEN) {
		DNG09_fight_intro(fight_id);
	}
	if (gs_dungeon_id == DUNGEON_ID_PIRATENHOEHLE) {
		DNG11_fight_intro(fight_id);
	}
	if (gs_dungeon_id == DUNGEON_ID_ZWERGENFESTE) {
		DNG12_fight_intro(fight_id);
	}
	if (gs_dungeon_id == DUNGEON_ID_VERLASSENE_MINE) {
		DNG13_fight_intro(fight_id);
	}
	if (gs_dungeon_id == DUNGEON_ID_ZWINGFESTE) {
		DNG14_fight_intro(fight_id);
	}

	g_textbox_width = tw_bak;
}

/**
 * \brief   print a dialog window without answer options
 *
 * \param   head_index  the number of a head, if another should be loaded
 * \param   text        the text
 */
void GUI_dialog_na(const signed int head_index, char *text)
{

	if (head_index != 0)
		load_in_head(head_index);

	GUI_dialogbox((unsigned char*)g_dtp2, NULL, text, 0);

}
