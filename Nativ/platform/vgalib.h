/*
 * vgalib.h - VGA- und SDL2-Bruecke fuer DSA 1: Die Schicksalsklinge
 * Basierend auf BrightEyes (GPL v3), angebunden an die gemeinsame
 * Nordlandtrilogie-Plattformschicht (dsa_platform.h).
 */

#ifndef VGALIB_H
#define VGALIB_H

#ifdef __cplusplus
extern "C" {
#endif

enum FB_VALUES {
	O_WIDTH = 320,
	O_HEIGHT = 200,
	MAX_RATIO = 10
};

void set_palette(const unsigned char *pointer, const unsigned char first_color, const unsigned short colors);

unsigned short swap_u16(const unsigned short val);

void vgalib_fill_rect(unsigned char *dst_in, const int color, const int width, const int height);
void vgalib_copy_to_screen(unsigned char *dst_in, unsigned char *src_in, const int width, const int height);
void vgalib_copy_to_screen_bounded(unsigned char *dst_in, unsigned char *src_in, const int width, const int height);
void vgalib_copy_to_screen_nonzero(unsigned char *dst_in, unsigned char *src_in, const int width, const int height);
void vgalib_screen_copy(unsigned char *dst_in, unsigned char *src_in, const int width, const int height);
void vgalib_copy_from_screen(unsigned char *dst_in, unsigned char *src_in, const int width, const int height);

#if defined(__BORLANDC__)
void set_video_mode(unsigned short);
void set_video_page(unsigned short);
void save_display_stat(signed short*);
#define sdl_toggle_pal_logic()
#else
void sdl_init_video(void);
void sdl_exit_video(void);
void sdl_update_rect_window(const int x_in, const int y_in, const int width_in, const int height_in);
void sdl_forced_update(void);
void sdl_present_if_changed(void);
int  sdl_get_ratio(void);
void sdl_toggle_pal_logic(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* VGALIB_H */
