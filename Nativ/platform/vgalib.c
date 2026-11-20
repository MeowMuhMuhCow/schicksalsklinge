/*
 * vgalib.c - VGA-Framebuffer-Routinen und Bruecke zu dsa_platform (SDL2)
 *
 * Herkunft: Abgeleitet aus BrightEyes src/schick/vgalib.cpp (Copyright The BrightEyes Authors,
 * GPL v3) und angebunden an Port/platform/dsa_platform_sdl2.cpp.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dsa_platform.h"
#include "vgalib.h"

extern unsigned char *g_vga_memstart;

static unsigned char g_shown_vga[O_WIDTH * O_HEIGHT];
static int           g_pal_dirty = 1;
static int           g_pal_logic = 0;

void sdl_toggle_pal_logic(void)
{
	g_pal_logic = g_pal_logic ? 0 : 1;
}

void sdl_init_video(void)
{
	dsa_platform_init(
		"DSA 1: Die Schicksalsklinge (Windows Port - MooMoo)",
		O_WIDTH,
		O_HEIGHT,
		NULL
	);
}

void sdl_exit_video(void)
{
	dsa_platform_shutdown();
}

/* Mauszeiger-Flackern (ab Wichtige Fixes): Das Spiel blendet den Zeiger vor jedem Neuzeichnen kurz aus
 * (call_mouse_bg) und danach wieder ein (call_mouse). Wurde genau dazwischen ein Bild gezeigt, flackerte der Zeiger.
 * Jetzt wird ein Bild, das nur waehrend dieser kurzen Luecke entsteht, nicht gezeigt - man sieht erst das fertige Bild
 * mit Zeiger (auch bei gleichzeitigem Palettenwechsel, z. B. beim Verlassen eines Hauses). Bleibt der Zeiger laenger
 * aus (Zwischensequenzen, Ueberblendungen), wird nach 300 ms normal gezeigt.
 * g_port_fix_stufe kommt aus dem Spielcode (FIX_BUGS der jeweiligen Stufe), ORIG bleibt unveraendert. */
extern const int g_port_fix_stufe;
extern signed int g_mouse_refresh_flag;
static uint32_t g_zeiger_aus_seit = 0;

static int zeiger_luecke_aussetzen(void)
{
	uint32_t jetzt;
	if (g_port_fix_stufe < 1 || g_mouse_refresh_flag >= 0) {
		g_zeiger_aus_seit = 0;
		return 0;
	}
	jetzt = dsa_platform_get_ticks_ms();
	if (!g_zeiger_aus_seit) g_zeiger_aus_seit = jetzt ? jetzt : 1;
	return (jetzt - g_zeiger_aus_seit) < 300;
}

static void sync_and_present_now(void)
{
	if (zeiger_luecke_aussetzen()) {
		dsa_platform_poll_events_only();
		return;
	}
	unsigned char *vram = dsa_platform_get_vram();
	if (g_vga_memstart && vram && g_vga_memstart != vram) {
		memcpy(vram, g_vga_memstart, O_WIDTH * O_HEIGHT);
	}
	if (g_vga_memstart) {
		memcpy(g_shown_vga, g_vga_memstart, O_WIDTH * O_HEIGHT);
	}
	g_pal_dirty = 0;
	dsa_platform_present();
}

void sdl_update_rect_window(const int x_in, const int y_in, const int width_in, const int height_in)
{
	(void)x_in;
	(void)y_in;
	(void)width_in;
	(void)height_in;
	sync_and_present_now();
}

void sdl_forced_update(void)
{
	sync_and_present_now();
}

void sdl_present_if_changed(void)
{
	if (!g_vga_memstart) {
		dsa_platform_poll_events_only();
		return;
	}
	if (g_pal_dirty || memcmp(g_shown_vga, g_vga_memstart, O_WIDTH * O_HEIGHT) != 0) {
		sync_and_present_now();
	} else {
		unsigned char *vram = dsa_platform_get_vram();
		if (vram && g_vga_memstart != vram) {
			memcpy(vram, g_vga_memstart, O_WIDTH * O_HEIGHT);
		}
		dsa_platform_pump_and_present_if_due();
	}
}

int sdl_get_ratio(void)
{
	return dsa_platform_get_scale();
}

void set_palette(const unsigned char *pointer, const unsigned char first_color, const unsigned short colors)
{
	if (!pointer || colors == 0) return;
	dsa_platform_set_palette_range((int)first_color, (int)colors, pointer);
	g_pal_dirty = 1;
	if (g_pal_logic) {
		sync_and_present_now();
	}
}

void vgalib_fill_rect(unsigned char *dst_in, const int color, const int width, const int height)
{
	unsigned char *dst;
	int h;

	if ((width == O_WIDTH) && (height == O_HEIGHT)) {
		memset(dst_in, color, O_WIDTH * O_HEIGHT);
	} else {
		dst = dst_in;
		for (h = height; h > 0; h--) {
			memset(dst, color, width);
			dst += O_WIDTH;
		}
	}

	if (g_vga_memstart) {
		const ptrdiff_t offset = dst_in - g_vga_memstart;
		if ((0 <= offset) && (offset <= O_WIDTH * O_HEIGHT)) {
			sync_and_present_now();
		}
	}
}

void vgalib_copy_to_screen(unsigned char *dst_in, unsigned char *src_in, const int width, const int height)
{
	unsigned char *src = src_in;
	unsigned char *dst = dst_in;
	int h;

	if ((width == O_WIDTH) && (height == O_HEIGHT)) {
		memcpy(dst_in, src_in, O_WIDTH * O_HEIGHT);
	} else {
		for (h = height; h > 0; h--) {
			memcpy(dst, src, width);
			dst += O_WIDTH;
			src += width;
		}
	}

	if (g_vga_memstart) {
		const ptrdiff_t offset = dst_in - g_vga_memstart;
		if ((0 <= offset) && (offset <= O_WIDTH * O_HEIGHT)) {
			sync_and_present_now();
		}
	}
}

void vgalib_copy_to_screen_bounded(unsigned char *dst_in, unsigned char *src_in, const int width, const int height)
{
	unsigned char *src = src_in;
	unsigned char *dst = dst_in;
	int h, j;

	for (h = height; h > 0; h--) {
		for (j = 0; j < width; j++) {
			if (dst[j] < 0xc8) dst[j] = src[j];
		}
		dst += O_WIDTH;
		src += width;
	}

	if (g_vga_memstart) {
		const ptrdiff_t offset = dst_in - g_vga_memstart;
		if ((0 <= offset) && (offset <= O_WIDTH * O_HEIGHT)) {
			sync_and_present_now();
		}
	}
}

void vgalib_copy_to_screen_nonzero(unsigned char *dst_in, unsigned char *src_in, const int width, const int height)
{
	unsigned char *src = src_in;
	unsigned char *dst = dst_in;
	int h, j;

	for (h = height; h > 0; h--) {
		for (j = 0; j < width; j++) {
			if (src[j]) dst[j] = src[j];
		}
		dst += O_WIDTH;
		src += width;
	}

	if (g_vga_memstart) {
		const ptrdiff_t offset = dst_in - g_vga_memstart;
		if ((0 <= offset) && (offset <= O_WIDTH * O_HEIGHT)) {
			sync_and_present_now();
		}
	}
}

void vgalib_screen_copy(unsigned char *dst_in, unsigned char *src_in, const int width, const int height)
{
	unsigned char *dst = dst_in;
	unsigned char *src = src_in;
	int i;

	for (i = height; i > 0; i--) {
		memcpy(dst, src, width);
		dst += O_WIDTH;
		src += O_WIDTH;
	}

	if (g_vga_memstart) {
		const ptrdiff_t offset = dst_in - g_vga_memstart;
		if ((0 <= offset) && (offset <= O_WIDTH * O_HEIGHT)) {
			sync_and_present_now();
		}
	}
}

void vgalib_copy_from_screen(unsigned char *dst_in, unsigned char *src_in, const int width, const int height)
{
	unsigned char *src = src_in;
	unsigned char *dst = dst_in;
	int h;

	for (h = height; h > 0; h--) {
		memcpy(dst, src, width);
		dst += width;
		src += O_WIDTH;
	}
}
