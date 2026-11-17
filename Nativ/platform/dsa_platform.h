/*
 * dsa_platform.h - Gemeinsame Plattformschnittstelle fuer die Nordlandtrilogie
 *   - DSA 1: Die Schicksalsklinge (320x200, 256 Farben)
 *   - DSA 2: Sternenschweif       (320x200 / SVGA, 256 Farben)
 *   - DSA 3: Schatten ueber Riva  (320x200 & 640x480/640x400 SVGA, 256 Farben)
 *
 * Kapselt Grafik (8-Bit-Framebuffer + 256-Farben-VGA-DAC-Palette -> SDL2-Textur),
 * Maus- und Tastatureingaben (DOS Int 0x33 / BIOS Int 0x16 / Scancode-Matrix),
 * Zeitgeber (PIT-Ticks) sowie Dateipfad-Aufloesung relativ zum Spielverzeichnis.
 */

#ifndef DSA_PLATFORM_H
#define DSA_PLATFORM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DSA_MAX_VRAM_WIDTH   640
#define DSA_MAX_VRAM_HEIGHT  480
#define DSA_VRAM_SIZE        (DSA_MAX_VRAM_WIDTH * DSA_MAX_VRAM_HEIGHT)

/* Initialisierung & Lebenszyklus */
int  dsa_platform_init(const char *window_title, int width, int height, const char *game_dir);
void dsa_platform_shutdown(void);
void dsa_platform_pump_and_present_if_due(void);
void dsa_platform_poll_events_only(void);

/* Grafikmodus & 8-Bit-Framebuffer */
void           dsa_platform_set_mode(int dos_mode, int width, int height);
int            dsa_platform_get_width(void);
int            dsa_platform_get_height(void);
unsigned char *dsa_platform_get_vram(void);
void           dsa_platform_present(void);
void           dsa_platform_wait_vsync(void);
int            dsa_platform_get_scale(void);
void           dsa_platform_cycle_scale(void);

/* 256-Farben VGA-DAC-Palette (6-Bit-Komponenten 0..63) */
void dsa_platform_set_palette_entry(int index, unsigned char r6, unsigned char g6, unsigned char b6);
void dsa_platform_set_palette_range(int first, int count, const unsigned char *rgb6_triplets);
void dsa_platform_get_palette_entry(int index, unsigned char *r6, unsigned char *g6, unsigned char *b6);
void dsa_platform_get_palette_range(int first, int count, unsigned char *rgb6_triplets);

/* Maus (DOS Int 0x33 kompatibel) */
void dsa_platform_get_mouse(int *x, int *y, int *buttons);
void dsa_platform_poll_mouse_events(int *x, int *y, int *moved, int *left_click, int *right_click);
void dsa_platform_set_mouse_pos(int x, int y);
void dsa_platform_set_mouse_range_x(int min_x, int max_x);
void dsa_platform_set_mouse_range_y(int min_y, int max_y);

/* Tastatur (BIOS Int 0x16 & Int 0x09 Scancode-Tabelle kompatibel) */
unsigned short dsa_platform_key_ready(void);
unsigned short dsa_platform_key_read(void);
unsigned short dsa_platform_key_shift_status(void);
unsigned char  dsa_platform_get_scancode_state(int dos_scancode);

/* Zeitgeber */
uint32_t dsa_platform_get_ticks_ms(void);
void     dsa_platform_delay_ms(uint32_t ms);

/* Dateipfad-Aufloesung relativ zum konfigurierbaren Spielordner */
void        dsa_platform_set_game_dir(const char *path);
const char *dsa_platform_get_game_dir(void);
const char *dsa_platform_resolve_path(const char *dos_path, char *out_buf, size_t out_size);

/* Audio: CD-Audio (MUSIC/Track02..15.OGG wie MobileEyes/GOG) + General MIDI + DOSBox OPL2 & VOC-Effekte */
#define DSA_MUSIC_MODE_CD  0
#define DSA_MUSIC_MODE_GM  1
#define DSA_MUSIC_MODE_OPL 2

void dsa_platform_set_music_mode(int mode);
int  dsa_platform_get_music_mode(void);
int  dsa_platform_play_cd_track(int track_no);
void dsa_platform_pause_cd_audio(int pause_on);
void dsa_platform_stop_cd_audio(void);
void dsa_platform_load_adlib_bank(const unsigned char *data, uint32_t len);
int  dsa_platform_play_xmidi(const unsigned char *xmi_data, uint32_t xmi_len, int loop);
void dsa_platform_stop_music(void);
void dsa_platform_pause_music(int pause_on);
void dsa_platform_set_music_volume(int vol_0_127);
int  dsa_platform_get_music_volume(void);

int  dsa_platform_play_voc(const unsigned char *voc_data, uint32_t voc_len);
void dsa_platform_stop_voc(void);
int  dsa_platform_is_voc_playing(void);
void dsa_platform_set_sfx_volume(int vol_0_127);
int  dsa_platform_get_sfx_volume(void);

void dsa_platform_set_dump_wav(const char *path);
void dsa_platform_get_audio_stats(uint32_t *out_mixed_frames, uint32_t *out_music_notes, uint32_t *out_voc_plays, int *out_peak_sample);

/* Werkzeuge W1 (Absturz-Logger), W2 (Stub-Waechter), W3 (Abdeckung), W4 (Record/Replay) */
extern const char *g_dsa_last_func;
extern int         g_dsa_coverage_enabled;

void dsa_platform_register_crash_state_cb(void (*cb)(char *buf, int buf_size));
void dsa_platform_log_stub(const char *func_name);
void dsa_platform_trace_func_hit(int func_id, const char *seg_and_func);
void dsa_platform_set_record_file(const char *path);
void dsa_platform_set_replay_file(const char *path);
void dsa_platform_set_fast_replay(int enabled);
int  dsa_platform_is_fast_replay(void);
void dsa_platform_save_screenshot_bmp(const char *path);
int  dsa_run_original_intro(void);

static inline void dsa_trace_enter(int func_id, const char *seg_and_func) {
    g_dsa_last_func = seg_and_func;
    if (g_dsa_coverage_enabled) {
        dsa_platform_trace_func_hit(func_id, seg_and_func);
    }
}

#define DSA_STUB(name) dsa_platform_log_stub(name)

#ifdef __cplusplus
}
#endif

#endif /* DSA_PLATFORM_H */
