/*
 * seg002 - Kern: main(), Spielschleife, Zeit/Timer, Maus/Tastatur, Ton, Archivzugriff, Heldenhilfen
 *
 * Inhalt:    Enthaelt main() mit Start, Kopierschutzabfrage und Spielschleife (game_loop),
 *            den Zugriff auf SCHICK.DAT und TEMP-Dateien, Musik und Soundeffekte ueber AIL
 *            (Miles Sound System), Maustreiber und Tastaturabfrage (handle_input, handle_gui_input),
 *            die Spielzeit mit allen Timern (do_timers, timewarp*, Tageswechsel, Zinsen, Schiffspassagen,
 *            Hunger und Durst in herokeeping) sowie viele kleine Helfer fuer Helden
 *            (LE/AE/AP/Geld, Eigenschaftsproben, Gruppensuche).
 * Aufrufer:  praktisch alle Module; do_timers() laeuft im Timer-Interrupt (seg004, timer_isr).
 * Daten:     Hier wird datseg.c eingebunden, alle globalen Daten liegen also in diesem Modul.
 *            Dazu die BSS-Variablen des Datensegments ab ds:0xbc59.
 * Besonders: Viele bekannte Fehler bei der Zeitrechnung (Original-Bug 36-39, timewarp*).
 *            Neu: sub_mod_timers() setzt beim Aufraeumen ungueltiger Ziele immer nur den
 *            aktuellen Eintrag zurueck und benutzt h_index ohne Neuinitialisierung;
 *            draw_loc_icons() liest bei 9 Icons hinter das Feld; das AE-Bugfix in sub_ae_splash()
 *            prueft eine unpassende Bedingung.
 *            Musik laeuft in v3.02 immer ueber CD-Audio, der MIDI-Treiber wird nie geladen (read_sound_cfg).
 *            Braucht die Header von AIL (Miles Sound System).
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Y -IAIL seg002.c
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>

#if defined(__BORLANDC__)
#include <ALLOC.H>
#include <CONIO.H>
#include <DIR.H>
#include <DOS.H>
#include <IO.H>
#include "AIL/AIL.H"
#else
#include <unistd.h>
#include "seg011.h"
#endif

#include "v302de.h"
#include "common.h"

#if defined(__BORLANDC__)
/* all global variables are included here, since BCC.EXE would create another module */
#include "datseg.c"
#endif

#include "seg001.h"
#include "seg002.h"
#include "seg004.h"
#include "seg006.h"
#include "seg007.h"
#include "seg008.h"
#include "seg009.h"
#include "seg010.h"
#include "seg025.h"
#include "seg026.h"
#include "seg027.h"
#include "seg029.h"
#include "seg030.h"
#include "seg039.h"
#include "seg041.h"
#include "seg047.h"
#include "seg048.h"
#include "seg049.h"
#include "seg050.h"
#include "seg065.h"
#include "seg066.h"
#include "seg076.h"
#include "seg093.h"
#include "seg095.h"
#include "seg096.h"
#include "seg097.h"
#include "seg106.h"
#include "seg108.h"
#include "seg113.h"
#include "seg119.h"
#include "seg120.h"
#include "seg121.h"

#if !defined(__BORLANDC__)
#if defined(_WIN32)
#include <direct.h>
#include <io.h>
#define MKDIR_PORT_S2(p) _mkdir(p)
#else
#include <ctype.h>
#include <sys/stat.h>
#define MKDIR_PORT_S2(p) mkdir(p, 0755)
#endif
#include "dsa_platform.h"
#include "vgalib.h"
static int g_skip_copy_protection = 
#if defined(SKIP_COPY_PROTECTION)
    SKIP_COPY_PROTECTION;
#else
    1;
#endif
#endif

/* BSS Variables */
unsigned char dummy;	// ds:0xbc59
uint16_t g_sample_ad_length;		// ds:0xbc5a
struct sample_idx g_sample_ad_idx_entry;	// ds:0xbc5c
unsigned char g_playmask_us;			// ds:0xbc62, 1 = PLAYM_US, 0 = PLAYM_UK
unsigned char g_gfxbuf_wait_keypress[100];	// ds:0xbc63
unsigned char *g_splash_ae;			// ds:0xbcc7
unsigned char *g_splash_le;			// ds:0xbccb
uint8_t g_hero_splash_timer[7];			// ds:0xbccf
signed int g_spinlock_flag;		// ds:0xbcd6
signed int g_map_townmark_state;		// ds:0xbcd8
signed char g_freeze_timers;			// ds:0xbcda

#if defined(__BORLANDC__)
void interrupt far (*g_mouse_handler_bak)(__CPPARGS); // ds:0xbcdb; seg002
#endif

int32_t g_archive_file_offset;	// ds:0xbcdf, start offset in SCHICK.DAT
int32_t g_archive_file_remaining; // ds:0xbce3, flen - off
int32_t g_archive_file_length;	// ds:0xbce7

#if defined(__BORLANDC__)
uint8_t *g_ail_digi_driver_buf2;		// ds:0xbceb, to buffer of size 5016
uint8_t *g_ail_voc_buffer;		// ds:0xbcef
uint8_t *g_ail_digi_driver_buf;		// ds:0xbcf3
int16_t *g_ail_digi_driver_descr;	// ds:0xbcf7
signed int g_ail_digi_driver_id;		// ds:0xbcfb
#endif

signed int g_use_cdaudio_flag; // ds:0xbcfd

#if defined(__BORLANDC__)
signed int g_ail_music_inactive;		// ds:0xbcff
signed int g_sample_ad_handle;		// ds:0xbd01
uint16_t g_ail_timbre_cache_size;	// ds:0xbd03
int32_t g_ail_state_table_size;		// ds:0xbd05
uint8_t *g_ail_music_driver_buf2;	// ds:0xbd09
uint8_t *g_ail_midi_buffer;		// ds:0xbd0d
uint8_t *g_ail_timbre_cache;		// ds:0xbd11
uint8_t *g_ail_state_table;		// ds:0xbd15
uint8_t *g_ail_music_driver_buf;		// ds:0xbd19
int16_t *g_ail_music_driver_descr;	// ds:0xbd1d
signed int g_ail_sequence;		// ds:0xbd21
signed int g_ail_music_driver_id;	// ds:0xbd23
#endif

signed int g_pregame_state;			// ds:0xbd25
signed char g_area_camp_area_type;		// ds:0xbd27, {0 = camp takes place in a dungeon, 1 = camp takes place in a town}
struct fight *g_current_fight;		// ds:0xbd28
struct scenario *g_scenario_buf;		// ds:0xbd2c
unsigned char *g_fightobj_buf;		// ds:0xbd30
struct struct_hero *g_heroes;		// ds:0xbd34
signed char g_new_menu_icons[9];	// ds:0xbd38
unsigned char g_unkn_073[12];		// ds:0xbd41
unsigned char g_steptarget_front;	// ds:0xbd4d
unsigned char g_steptarget_back;	// ds:0xbd4e
signed char g_viewdir_flag_bogus;		// ds:0xbd4f, writeonly (1)
signed char g_visual_squares_tex[29];	// ds:0xbd50
unsigned char g_unkn_074[1];		// ds:0xbd6d
unsigned char g_visual_square_vals[29];	// ds:0xbd6e
unsigned char g_unkn_075[1];		// ds:0xbd8b
#if defined(__BORLANDC__)
struct ems_tab *g_ems_unused_tab;	// ds:0xbd8c
#endif
signed int g_ems_travelmap_handle;	// ds:0xbd90
#if defined(__BORLANDC__)
signed int g_ems_unused_handle;		// ds:0xbd92
#endif
unsigned char g_map_size_x;		// ds:0xbd94
unsigned char g_dng_map[512];		// ds:0xbd95
char *g_radio_name_list[25];		// ds:0xbf95, used for items, heroes, spells, talents, recipes
unsigned char *g_gui_box_bg_buf;	// ds:0xbff9
signed int g_textbox_width;		// ds:0xbffd
signed int g_textbox_pos_x;		// ds:0xbfff, coordinate of upper left corner
signed int g_textbox_pos_y;		// ds:0xc001, coordinate of upper left corner
signed int g_game_mode;			// ds:0xc003, {-1 = Input error, 1 = Beginner, 2 = Advanced }

struct item_selector_item *g_item_selector_sell; // ds:0xc005, also used for repair items
struct item_selector_item *g_item_selector_buy; // ds:0xc009, merchant's assortment
struct struct_pic_copy g_pic_copy;	// ds:0xc00d
struct location g_locations_tab[150]; // ds:0xc025
unsigned char *g_buffer8_ptr; // ds:0xc3a9, to buffer of size 12008
char **g_tx2_index; // ds:0xc3ad, to index table of secondary text file
char **g_tx_index; // ds:0xc3b1, to index table of primary text file
char **g_text_ltx_index; // ds:0xc3b5, to index table of TEXT.LTX
unsigned char g_unkn_077[6]; // ds:0xc3b9
int16_t g_random_schick_seed2; // ds:0xc3bf
signed int g_game_state; // ds:0xc3c1, see enum GAME_STATE_*
unsigned char g_unkn_078[2]; // ds:0xc3c3
signed int g_game_paused_flag; // ds:0xc3c5 // game paused from pressing Ctrl + P
signed int g_have_mouse; // ds:0xc3c7
signed int g_unused_spinlock_flag; // ds:0xc3c9
signed int g_update_statusline; // ds:0xc3cb
unsigned char g_unkn_079[2]; // ds:0xc3cd
signed int g_mouse1_doubleclick; // ds:0xc3cf
signed int g_mouse_leftclick_event; // ds:0xc3d1
signed int g_mouse_rightclick_event; // ds:0xc3d3
signed int g_mouse1_event2; // ds:0xc3d5
signed int g_keystroke_ascii_code; // ds:0xc3d7
signed int g_action_id; // ds:0xc3d9
HugePt g_buffer9_ptr; // ds:0xc3db, to buffer of size 180000 (or 203000 if LARGE_BUF), used for NVF
unsigned char g_unkn_080[8]; // ds:0xc3df
signed int g_ani_width; // ds:0xc3e7
signed int g_ani_unknown1; // ds:0xc3e9
signed int g_ani_unknown2; // ds:0xc3eb
unsigned char g_ani_height; // ds:0xc3ed
signed char g_ani_areacount; // ds:0xc3ee
struct ani_area g_ani_area_table[10]; // ds:0xc3ef
unsigned char *g_ani_main_ptr; // ds:0xce35
unsigned char g_ani_compr_flag; // ds:0xce39, {0,1 = compressed}
unsigned char g_ani_palette_size; // ds:0xce3a
HugePt g_ani_palette; // ds:0xce3b
signed int g_ani_posy; // ds:0xce3f
signed int g_ani_posx; // ds:0xce41
int32_t g_ani_unknown4; // ds:0xce43, writeonly (0)
unsigned char g_unkn_081[64]; // ds:0xce47
unsigned char g_gui_text_buffer[64]; // ds:0xce87
struct mouse_cursor *g_last_cursor; // ds:0xcec7
struct mouse_cursor *g_current_cursor; // ds:0xcecb
struct mouse_cursor g_ggst_cursor; // ds:0xcecf
unsigned char g_mouse_bg_bak[256]; // ds:0xcf0f

signed int g_dng_init_flag;		// ds:0xd00f
signed int g_dng_extra_action;		// ds:0xd011, {0 = warehouse,1 = open door,2 = open chest,3 = close door,4 = lever,5 = smash door}
signed int g_redraw_menuicons;		// ds:0xd013
unsigned char *g_buffer9_ptr2;		// ds:0xd015, copy of BUFFER9_PTR
HugePt g_buffer9_ptr3;			// ds:0xd019, copy of BUFFER9_PTR
uint32_t g_buffer_monster_tab[36];	// ds:0xd01d
uint32_t g_buffer_wfigs_tab[43];		// ds:0xd0ad
uint32_t g_buffer_mfigs_tab[43];		// ds:0xd159
int32_t g_buffer_anis_tab[37];		// ds:0xd205

unsigned char *g_trv_track_pixel_bak; // ds:0xd299, to buffer of size 500
unsigned char *g_fig_star_gfx; // ds:0xd29d
char *g_monnames_buffer; // ds:0xd2a1
char *g_buffer5_ptr; // ds:0xd2a5, to buffer of size 3880
unsigned char *g_buffer10_ptr; // ds:0xd2a9, to buffer of size 16771, used for NVF and text
unsigned char *g_popup; // ds:0xd2ad
unsigned char *g_buffer6_ptr; // ds:0xd2b1, to buffer of size 2200, used for NVF
char *g_buffer7_ptr; // ds:0xd2b5, to buffer of size 10000, used for NVF and text
char *g_text_ltx_buffer; // ds:0xd2b9, to buffer of size 30500
unsigned char *g_splash_buffer; // ds:0xd2bd, to buffer of size 1000, used for Splashes
unsigned char *g_buf_font6; // ds:0xd2c1
signed int g_textcolor_index; // ds:0xd2c5
signed int g_textcolor_bg; // ds:0xd2c7
signed int g_textcolor_fg[4]; // ds:0xd2c9
signed int g_gui_text_centered; // ds:0xd2d1
signed int g_textline_unknown; // ds:0xd2d3, writeonly (103)
signed int g_textline_maxlen; // ds:0xd2d5
signed int g_textline_posy; // ds:0xd2d7
signed int g_textline_posx; // ds:0xd2d9
unsigned char *g_fig_figure2_buf; // ds:0xd2db, to buffer of size 20000
unsigned char *g_fig_figure1_buf; // ds:0xd2df
unsigned char *g_objects_nvf_buf; // ds:0xd2e3, to buffer of size 3400
unsigned char *g_buf_icon; // ds:0xd2e7
char *g_text_output_buf; // ds:0xd2eb, to buffer of size 300
char *g_text_input_buf; // ds:0xd2ef, to buffer of size 24
char *g_dtp2; // ds:0xd2f3
unsigned char *g_icon; // ds:0xd2f7
unsigned char *g_vga_backbuffer; // ds:0xd2fb
unsigned char *g_vga_memstart; // ds:0xd2ff
unsigned char *g_renderbuf_ptr; // ds:0xd303, to buffer of size 65000


#if defined(__BORLANDC__)
unsigned char g_unkn_082[2]; // ds:0xd307
signed int g_gameinit_flag; // ds:0xd309, writeonly (1)
int16_t g_video_page_bak; // ds:0xd30b
int16_t g_video_mode_bak; // ds:0xd30d
unsigned char g_unkn_083[4]; // ds:0xd30f
#endif

signed int g_txt_tabpos[7]; // ds:0xd313
unsigned char *g_townpal_buf; // ds:0xd321
signed int g_fig_escape_position[4]; // ds:0xd325, see HERO_ESCAPE_POSITION
signed int g_wildcamp_sleep_quality; // ds:0xd32d
signed int g_gather_herbs_mod; // ds:0xd32f
signed int g_replenish_stocks_mod; // ds:0xd331
struct fight_msg g_fig_msg_data[6]; // ds:0xd333
struct enemy_sheet g_enemy_sheets[20]; // ds:0xd34b
#if defined(__BORLANDC__)
int8_t g_fig_move_pathdir[10]; // ds:0xd823
#else
int8_t g_fig_move_pathdir[20]; // ds:0xd823
uint8_t *g_ail_voc_buffer;
signed int g_ail_music_inactive;
uint8_t *g_ail_midi_buffer;
#endif /* TODO: 10 steps is too short */
signed char g_fig_enemy_parry_action_used[30]; // ds:0xd82d, see FIG_ACTION_PARRY
signed char g_fig_hero_parry_action_used[7]; // ds:0xd84b
signed char *g_chessboard;// ds:0xd852
uint8_t *g_fig_spellgfx_buf; // ds:0xd856
uint8_t *g_fig_projectile_buf; // ds:0xd85a
uint8_t *g_fig_cb_selector_buf; // ds:0xd85e
uint8_t *g_fig_cb_marker_buf; // ds:0xd862
uint8_t *g_spellobj_nvf_buf; // ds:0xd866, to buffer of size 0xf5f
uint8_t *g_weapons_nvf_buf; // ds:0xd86a, to buffer of size 0x1953
unsigned char *g_fightobj_buf_seek_ptr; // ds:0xd86e, points to end of FIGHTOBJ buffer
signed int g_nr_of_enemies; // ds:0xd872, ?
signed char g_fightobj_list[90]; // ds:0xd874
int8_t g_fig_ani_tracks[FANI_TRACK_ID__MAX][243]; // ds:0xd8ce

struct struct_fighter g_fig_list_elem; // ds:0xe066
signed char g_fig_list_array[127]; // ds:0xe089
struct struct_fighter *g_fig_list_head; // ds:0xe108, to a list
signed char g_location_market_flag; // ds:0xe10c
signed int g_wallclock_redraw; // ds:0xe10d
signed int g_wallclock_y; // ds:0xe10f
signed int g_wallclock_x; // ds:0xe111
signed int g_wallclock_update; // ds:0xe113, 0 = don't update the wallclock
struct struct_memslot_fig *g_memslots_mon; // ds:0xe115
struct struct_memslot_fig *g_memslots_wfig; // ds:0xe119
struct struct_memslot_fig *g_memslots_mfig; // ds:0xe11d
struct struct_memslot_ani *g_memslots_anis; // ds:0xe121, to ()[36]
struct monster *g_monster_dat_buf; // ds:0xe125
char **g_monnames_index; // ds:0xe129
signed char g_market_itemsaldo_table[254]; // ds:0xe12d
struct item_stats *g_itemsdat; // ds:0xe22b
char **g_itemsname; // ds:0xe22f


#if !defined(__BORLANDC__)
static inline uint8_t readb(uint8_t *p)
{
	return ((uint8_t)*p);
}

static inline uint16_t readws(uint8_t *p)
{
	return (uint16_t)((readb(p + 1) << 8) | readb(p));
}

static inline uint32_t readds(uint8_t *p)
{
	return ((uint32_t)readws(p + 2) << 16) | (uint32_t)readws(p);
}

#else

#define readb(p) (*(uint8_t*)(p))
#define readws(p) (*(uint16_t*)(p))
#define readds(p) (*(uint32_t*)(p))

#endif

/* static prototypes */
static signed int prepare_midi_playback(const signed int);
static uint8_t* prepare_timbre(const signed int, const signed int);
static signed int do_load_midi_file(const signed int);
static signed int load_music_driver(const char*, const signed int, int16_t);
static void do_play_music_file(const signed int);
static void stop_midi_playback(void);
static void free_voc_buffer(void);
static signed int read_voc_file(const signed int);
static void SND_play_voc(const signed int);
static void SND_stop_digi(void);
static void SND_set_volume(const uint16_t);
static signed int load_digi_driver(const char*, const signed int, int16_t, int16_t);
static uint8_t* read_digi_driver(const char*);

static signed int open_temp_file(const signed int);
static void copy_from_archive_to_temp(const signed int, const char*);

static void mouse_bg(void);
static void mouse_cursor(void);
static void game_loop(void);
static void magical_chainmail_damage(void);
static void check_level_up(void);
static void passages_recalc(void);
static void passages_reset(void);
static void pause_mark_show(void);
static void pause_mark_hide(void);
static signed int check_hero_no3(const struct struct_hero *hero);
static void do_starve_damage(struct struct_hero *hero, const signed int hero_pos, const signed int type);
static signed int copy_protection(void);

static void play_music_file(const signed int index)
{
	dsa_trace_enter(6, "seg002:play_music_file");
	if (g_music_enabled != 0) {
		do_play_music_file(index);
	}
}

void set_audio_track(const signed int index)
{
	dsa_trace_enter(7, "seg002:set_audio_track");
#if defined(__BORLANDC__)
	CD_check();
#endif

	/* only do something when index is not the current track */
	if (g_music_current_track != index) {

		g_music_current_track = index;

		if (g_use_cdaudio_flag) {
			/* we use CD */
			CD_set_track(index);
		} else {
			/* we use AIL */
			play_music_file(index);
		}
	}
}

void sound_menu(void)
{
	dsa_trace_enter(8, "seg002:sound_menu");
	const signed int answer = GUI_radio(g_snd_menu_title, 4,
				g_snd_menu_radio1, g_snd_menu_radio2,
				g_snd_menu_radio3, g_snd_menu_radio4);

	switch (answer - 1) {
		case 0: {
			g_music_enabled = 0;
			g_snd_effects_enabled = 0;
			break;
		}
		case 1: {
			g_music_enabled = 1;
			g_snd_effects_enabled = 0;
			break;
		}
		case 2: {
			g_music_enabled = 0;
			g_snd_effects_enabled = 1;
			break;
		}
		case 3: {
			g_music_enabled = 1;
			g_snd_effects_enabled = 1;
			break;
		}
	}

	if (g_music_enabled == 0) {
		/* music disabled */
		if (g_use_cdaudio_flag) {
			CD_audio_pause();
		} else {
			stop_midi_playback();
		}
	} else {
		if (g_music_current_track != -1) {
			/* music enabled */
			if (g_use_cdaudio_flag) {
				CD_audio_resume();
			} else {
				play_music_file(g_music_current_track);
			}
		}
	}
}

void read_sound_cfg(void)
{
	dsa_trace_enter(9, "seg002:read_sound_cfg");
#if defined(__BORLANDC__)
	int16_t midi_port;
	int16_t dummy; /* zweites Wort in SOUND.CFG, wird nicht ausgewertet (vermutlich MIDI-IRQ) */
	int16_t digi_port;
	int16_t digi_irq;
	signed int handle;

	/* try to open SOUND.CFG */
	if ( (handle = open(g_fname_sound_cfg, O_BINARY | O_RDONLY)) != -1) {

		_read(handle, (uint8_t*)&midi_port, 2);
		_read(handle, (uint8_t*)&dummy, 2);
		_read(handle, (uint8_t*)&digi_port, 2);
		_read(handle, (uint8_t*)&digi_irq, 2);
		_close(handle);

#if !defined(__BORLANDC__)
		/* menu to select the music source */
		const int16_t tw_bak = g_textbox_width;
		char question[] = "WIE SOLL DIE MUSIK WIEDERGEGEBEN WERDEN?";
		char opt1[] = "AUDIO-CD";
		char opt2[] = "MIDI";
		signed int answer;

		g_textbox_width = 3;
		do {
			answer = GUI_radio(question, 2, opt1, opt2);

		} while (answer == -1);
		g_textbox_width = tw_bak;

		if (answer == 1)
		{
			/* AUDIO-CD selected */
			g_use_cdaudio_flag = 1;
			g_ail_music_inactive = 1;

		} else if (answer == 2)
		{
			/* MIDI selected */
			g_use_cdaudio_flag = 0;
			g_ail_music_inactive = 0;
		}

		if (g_use_cdaudio_flag == 0)
#else
		/* enable usage of audio-CD */
		/* g_ail_music_inactive = 1 bedeutet hier: kein MIDI-Musiktreiber aktiv */
		g_use_cdaudio_flag = g_ail_music_inactive = 1;

#if !FIX_PLATZ_SEG002	/* nie ausgefuehrt (if (0)); entfaellt mit Behebungen in diesem residenten Modul (Platz) */
		/* disable loading of the music driver */
		/* in v3.02 laeuft die Musik also immer von CD, SOUND.ADV wird nie geladen */
		if (0)
#endif
#endif
#if !defined(__BORLANDC__) || !FIX_PLATZ_SEG002
		{
			if (midi_port) {
				load_music_driver(g_fname_sound_adv2, 3, midi_port);
			} else {

				/* music was disabled in SOUND.CFG */
				if (g_ail_midi_buffer) {
					free((void*)g_ail_midi_buffer);
				}

				g_ail_midi_buffer = NULL;
			}
		}
#endif

		if (digi_port) {

			if (g_snd_voc_enabled) {

				if (!load_digi_driver(g_fname_digi_adv, 2, digi_port, digi_irq))
				{
					g_snd_voc_enabled = 0;
				}
			} else {
				/* print that sound effects are disabled */
				GUI_output(g_snd_txt_disabled_mem);
				g_snd_voc_enabled = 0;
			}
		} else {
			g_snd_voc_enabled = 0;
		}

	}
#else
	/* Uebernommen aus MobileEyes (Commit 4638425f, GPL v3, Gero Ohrner):
	 * Standard = Musik ueber die Audio-CD (MUSIC/TrackNN.OGG der GOG-Fassung, g_use_cdaudio_flag = 1),
	 * bei Schalter --music=gm oder --music=opl hingegen XMIDI (g_use_cdaudio_flag = 0). */
	g_use_cdaudio_flag = (dsa_platform_get_music_mode() == DSA_MUSIC_MODE_CD) ? 1 : 0;
	g_ail_music_inactive = 0;
	g_snd_voc_enabled = 1;
	g_snd_effects_enabled = 1;
	{
		signed int handle = load_archive_file(ARCHIVE_FILE_SAMPLE_AD);
		if (handle != -1) {
			uint8_t *ad_buf = (uint8_t*)schick_alloc(0x4000);
			if (ad_buf) {
				uint16_t ad_len = read_archive_file(handle, ad_buf, 0x3fff);
				dsa_platform_load_adlib_bank(ad_buf, ad_len);
				free(ad_buf);
			}
			close(handle);
		}
	}
#endif
}

/* reserviert den MIDI-Puffer und startet AIL. g_ail_music_inactive = 1 heisst: Musiktreiber (noch) nicht geladen */
void init_AIL(const uint32_t size)
{
	dsa_trace_enter(10, "seg002:init_AIL");
#if defined(__BORLANDC__)
	if ((g_ail_midi_buffer = (uint8_t*)schick_alloc(size))) {
		AIL_startup();
		g_ail_music_inactive = 1;
	}
#else
	g_ail_midi_buffer = (uint8_t*)schick_alloc(size < 0x8000 ? 0x8000 : size);
	g_ail_music_inactive = 0;
#endif
}

void exit_AIL(void)
{
	dsa_trace_enter(11, "seg002:exit_AIL");
#if defined(__BORLANDC__)
	AIL_shutdown((uint8_t*)NULL);

	if (g_ail_timbre_cache) {
		free((void*)g_ail_timbre_cache);
	}

	if (g_ail_state_table) {
		free((void*)g_ail_state_table);
	}

	if (g_ail_midi_buffer) {
		free((void*)g_ail_midi_buffer);
	}

	if (g_ail_music_driver_buf2) {
		free((void*)g_ail_music_driver_buf2);
	}

	/* set all pointers to NULL */
	g_ail_timbre_cache = g_ail_state_table = g_ail_midi_buffer = g_ail_music_driver_buf2 = NULL;

	if (g_snd_voc_enabled) {
		free_voc_buffer();
	}
#else
	dsa_platform_stop_cd_audio();
	dsa_platform_stop_music();
	dsa_platform_stop_voc();
	if (g_ail_midi_buffer) {
		free((void*)g_ail_midi_buffer);
		g_ail_midi_buffer = NULL;
	}
	if (g_snd_voc_enabled) {
		free_voc_buffer();
	}
#endif
}

#if !FIX_PLATZ_SEG002	/* nie aufgerufen; entfaellt mit Behebungen in diesem residenten Modul (Platz) */
static uint8_t* read_music_driver(const char* fname)
{
#if defined(__BORLANDC__)
	uint32_t len;
	uint8_t* buf;
	uint32_t ptr;

	signed int handle;

	if ( (handle = open((char*)fname, O_BINARY | O_RDONLY)) != -1) {

		len = 16500L;

		g_ail_music_driver_buf2 = (uint8_t*)schick_alloc(len + 16L);
		/* insane pointer casting */
		/* = Adresse auf die naechste Paragraphengrenze (16 Byte) aufrunden; AIL-Treiber muessen dort beginnen */
		ptr = (uint32_t)g_ail_music_driver_buf2 + 15L;
		ptr &= 0xfffffff0;
		buf = EMS_norm_ptr((uint8_t*)ptr);
		_read(handle, buf, len);
		_close(handle);
		return buf;
	}
#endif
	return NULL;
}
#endif

static signed int prepare_midi_playback(const signed int sequence)
{
	dsa_trace_enter(12, "seg002:prepare_midi_playback");
#if defined(__BORLANDC__)
	uint16_t answer;
	signed int bank;
	signed int patch;
	uint8_t* ptr;

	if ((g_sample_ad_handle = load_archive_file(ARCHIVE_FILE_SAMPLE_AD)) != -1) {

		if ((g_ail_sequence = AIL_register_sequence(g_ail_music_driver_id, g_ail_midi_buffer, sequence, g_ail_state_table, 0)) != -1) {

			while ((answer = AIL_timbre_request(g_ail_music_driver_id, g_ail_sequence)) != (uint16_t)-1)
			{
				bank = answer >> 8;
				patch = answer & 0xff;

				if ( (ptr = prepare_timbre(bank, patch))) {
					AIL_install_timbre(g_ail_music_driver_id, bank, patch, ptr);
					free(ptr);
				}
			}

			close(g_sample_ad_handle);
			return 1;
		}

		close(g_sample_ad_handle);
	}
#endif
	return 0;
}

static uint32_t g_native_midi_len = 0;
static uint32_t g_native_voc_len = 0;

static signed int start_midi_playback(const signed int seq)
{
	dsa_trace_enter(13, "seg002:start_midi_playback");
#if defined(__BORLANDC__)
	if (prepare_midi_playback(seq)) {
		AIL_start_sequence(g_ail_music_driver_id, seq);
		return 1;
	}
#else
	(void)seq;
	if (g_ail_midi_buffer && g_native_midi_len > 0) {
		return dsa_platform_play_xmidi(g_ail_midi_buffer, g_native_midi_len, 1);
	}
#endif
	return 0;
}


static uint8_t* prepare_timbre(const signed int bank, const signed int patch)
{
#if defined(__BORLANDC__)
	char *buf;

	seek_archive_file(g_sample_ad_handle, 0, 0);

	do {
		read_archive_file(g_sample_ad_handle, (uint8_t*)&g_sample_ad_idx_entry, 6);

		if (g_sample_ad_idx_entry.bank == -1) {
			return NULL;
		}

	} while ((g_sample_ad_idx_entry.bank != bank) || (g_sample_ad_idx_entry.patch != patch));

	seek_archive_file(g_sample_ad_handle, g_sample_ad_idx_entry.offset, 0);

	read_archive_file(g_sample_ad_handle, (uint8_t*)&g_sample_ad_length, 2);

	buf = schick_alloc(g_sample_ad_length);

	*((uint16_t*)buf) = g_sample_ad_length;

	read_archive_file(g_sample_ad_handle, buf + 2, g_sample_ad_length - 2);

	return buf;
#else
	return 0;
#endif
}

static signed int load_midi_file(const signed int index)
{
	dsa_trace_enter(14, "seg002:load_midi_file");
	return do_load_midi_file(index);
}

static signed int do_load_midi_file(const signed int index)
{
	dsa_trace_enter(15, "seg002:do_load_midi_file");
#if defined(__BORLANDC__)
	signed int handle;

	if ((handle = load_archive_file(index)) != -1) {
		read_archive_file(handle, g_ail_midi_buffer, 0x7fff);
		close(handle);
		return 1;
	}
#else
	signed int handle;
	if (g_ail_midi_buffer && (handle = load_archive_file(index)) != -1) {
		g_native_midi_len = read_archive_file(handle, g_ail_midi_buffer, 0x7fff);
		close(handle);
		return (g_native_midi_len > 0) ? 1 : 0;
	}
#endif
	return 0;
}

#if !FIX_PLATZ_SEG002	/* nie aufgerufen (if (0) in read_sound_cfg); entfaellt mit Behebungen in diesem residenten Modul (Platz) */
/* laedt einen AIL-Musiktreiber (type 3 = XMIDI) und erkennt die Hardware.
 * Erfolg: g_ail_music_inactive = 0, Fehlschlag: g_ail_music_inactive = 1 */
static signed int load_music_driver(const char* fname, const signed int type, int16_t port)
{
	dsa_trace_enter(16, "seg002:load_music_driver");
#if defined(__BORLANDC__)
	if (port && (g_ail_music_driver_buf = (uint8_t*)read_music_driver(fname)) &&
		((g_ail_music_driver_id = AIL_register_driver(g_ail_music_driver_buf)) != -1))
	{
		g_ail_music_driver_descr = (int16_t*)AIL_describe_driver(g_ail_music_driver_id);

		if (g_ail_music_driver_descr[1] == type)
		{
			if (port == -1) {
				port = g_ail_music_driver_descr[6];
			}

			if (AIL_detect_device(g_ail_music_driver_id, port,
				g_ail_music_driver_descr[7],
				g_ail_music_driver_descr[8],
				g_ail_music_driver_descr[9]))
			{
				AIL_init_driver(g_ail_music_driver_id, port,
					g_ail_music_driver_descr[7],
					g_ail_music_driver_descr[8],
					g_ail_music_driver_descr[9]);

				if (type == 3) {
					g_ail_state_table_size = AIL_state_table_size(g_ail_music_driver_id);
					g_ail_state_table = (uint8_t*)schick_alloc(g_ail_state_table_size);
					g_ail_timbre_cache_size = AIL_default_timbre_cache_size(g_ail_music_driver_id);

					if (g_ail_timbre_cache_size) {

						g_ail_timbre_cache = (uint8_t*)schick_alloc(g_ail_timbre_cache_size);

						AIL_define_timbre_cache(g_ail_music_driver_id, g_ail_timbre_cache, g_ail_timbre_cache_size);
					}
				}

				g_ail_music_inactive = 0;

				return 1;
			} else {

				/* no sound hardware found */
				GUI_output(g_snd_txt_hw_not_found);
				exit_AIL();
			}
		}
	}

	g_ail_music_inactive = 1;
#endif
	return 0;
}
#endif

static void do_play_music_file(const signed int index)
{
	dsa_trace_enter(17, "seg002:do_play_music_file");
#if defined(__BORLANDC__)
	if (!g_ail_music_inactive && (g_ail_music_driver_descr[1] == 3)) {

		stop_midi_playback();
		load_midi_file(index);
		start_midi_playback(0);
	}
#else
	if (!g_ail_music_inactive) {
		stop_midi_playback();
		if (load_midi_file(index)) {
			start_midi_playback(0);
		}
	}
#endif
}

static void stop_midi_playback(void)
{
	dsa_trace_enter(18, "seg002:stop_midi_playback");
#if defined(__BORLANDC__)
	if (!g_ail_music_inactive && (g_ail_music_driver_descr[1] == 3))
	{
		AIL_stop_sequence(g_ail_music_driver_id, g_ail_sequence);
		AIL_release_sequence_handle(g_ail_music_driver_id, g_ail_sequence);
	}
#else
	dsa_platform_stop_music();
#endif
}

/* wird im Timer-Interrupt (seg004) aufgerufen: ist die Sequenz zu Ende (Status 2 = SEQ_DONE),
 * wird sie neu gestartet, die Musik laeuft also in Schleife */
void restart_midi_playback_IRQ(void)
{
	dsa_trace_enter(19, "seg002:restart_midi_playback_IRQ");
#if defined(__BORLANDC__)
	if (!g_ail_music_inactive && g_music_enabled && (g_ail_music_driver_descr[1] == 3))
	{
		if (AIL_sequence_status(g_ail_music_driver_id, g_ail_sequence) == 2) {
			AIL_start_sequence(g_ail_music_driver_id, g_ail_sequence);
		}
	}
#endif
}

#if defined(__BORLANDC__)
#if !FIX_PLATZ_SEG002	/* nie aufgerufen; entfaellt mit Behebungen in diesem residenten Modul (Platz) */
static void midi_stop_sequence_unused(void)
/* This function is never called */
/* (hiess cruft_1) haelt die MIDI-Sequenz an, ohne sie freizugeben */
{
	if (!g_ail_music_inactive && (g_ail_music_driver_descr[1] == 3))
	{
		AIL_stop_sequence(g_ail_music_driver_id, g_ail_sequence);
	}
}

static void midi_set_volume_unused(const signed int volume)
/* This function is never called */
/* (hiess cruft_2) setzt die MIDI-Lautstaerke, bei 0 wird die Musik gestoppt */
{
	if (!g_ail_music_inactive) {

		if (g_ail_music_driver_descr[1] == 3) {
			AIL_set_relative_volume(g_ail_music_driver_id, g_ail_sequence, volume, 0);
		}

		if (!volume) {
			stop_midi_playback();
		}
	}
}
#endif
#endif

/* prueft, ob genug Speicher fuer den Musiktreiber da ist (1 = ja) und schaltet bei
 * weiteren 25000 Byte freiem Speicher die Soundeffekte (g_snd_voc_enabled) ein */
signed int have_mem_for_sound(void)
{
	dsa_trace_enter(20, "seg002:have_mem_for_sound");
#if defined(__BORLANDC__)
	int32_t size;
	signed int retval;
	struct ffblk blk;

	if (!findfirst(g_fname_sound_adv, &blk, 0)) {

		/* SOUND.ADV was found */
		size = blk.ff_fsize;
		size += 4000L;

		if ((uint32_t)size < farcoreleft()) {
			retval = 1;

			if ((uint32_t)(size + 25000L) < farcoreleft()) {

				g_snd_voc_enabled = 1;
			}
		} else {
			retval = 0;
		}
	} else {
		/* SOUND.ADV was not found */
		retval = 1;

		if (25000L < farcoreleft()) {
			g_snd_voc_enabled = 1;
		}
	}

	return retval;
#else
	g_snd_voc_enabled = 1;
	return 1;
#endif
}

void play_voc(const signed int index)
{
	dsa_trace_enter(21, "seg002:play_voc");
	if (g_snd_voc_enabled && g_snd_effects_enabled) {
		SND_set_volume(90);
		SND_play_voc(index);
	}
}

void play_voc_delay(const signed int index)
{
	dsa_trace_enter(22, "seg002:play_voc_delay");
#if defined(__BORLANDC__)
	if (g_snd_voc_enabled && g_snd_effects_enabled) {
		SND_set_volume(90);
		SND_play_voc(index);

		while (AIL_VOC_playback_status(g_ail_digi_driver_id) == 2) {
			wait_for_vsync();
		}
	}
#else
	if (g_snd_voc_enabled && g_snd_effects_enabled) {
		SND_set_volume(90);
		SND_play_voc(index);
		if (!dsa_platform_is_fast_replay()) {
			while (dsa_platform_is_voc_playing()) {
				wait_for_vsync();
			}
		}
	}
#endif
}

void alloc_voc_buffer(uint32_t size)
{
	dsa_trace_enter(23, "seg002:alloc_voc_buffer");
#if defined(__BORLANDC__)
	if (g_snd_voc_enabled) {
		if ((g_ail_voc_buffer = (uint8_t*)schick_alloc(size))) ;
	}
#else
	if (g_snd_voc_enabled) {
		g_ail_voc_buffer = (uint8_t*)schick_alloc(size < 0x8000 ? 0x8000 : size);
	}
#endif
}

static void free_voc_buffer(void)
{
	dsa_trace_enter(24, "seg002:free_voc_buffer");
#if defined(__BORLANDC__)
	if (g_snd_voc_enabled != 0) {

		if (g_ail_voc_buffer) {
			free((void*)g_ail_voc_buffer);
		}

		if (g_ail_digi_driver_buf2) {
			free((void*)g_ail_digi_driver_buf2);
		}

		g_ail_voc_buffer = g_ail_digi_driver_buf2 = NULL;

	}
#else
	if (g_snd_voc_enabled != 0 && g_ail_voc_buffer) {
		free((void*)g_ail_voc_buffer);
		g_ail_voc_buffer = NULL;
	}
#endif
}

static signed int read_new_voc_file(const signed int index)
{
	dsa_trace_enter(25, "seg002:read_new_voc_file");
#if defined(__BORLANDC__)
	if (AIL_VOC_playback_status(g_ail_digi_driver_id) == 2) {
		SND_stop_digi();
	}

	if (read_voc_file(index)) {

		AIL_format_VOC_file(g_ail_digi_driver_id, g_ail_voc_buffer, -1);
		return 1;
	}
#else
	if (dsa_platform_is_voc_playing()) {
		SND_stop_digi();
	}
	if (read_voc_file(index)) {
		return 1;
	}
#endif
	return 0;
}

static signed int read_voc_file(const signed int index)
{
	dsa_trace_enter(26, "seg002:read_voc_file");
#if defined(__BORLANDC__)
	signed int handle;

	if ( (handle = load_archive_file(index)) != -1) {
		read_archive_file(handle, g_ail_voc_buffer, 0x7fff);
		close(handle);
		return 1;
	}
#else
	signed int handle;
	if (g_ail_voc_buffer && (handle = load_archive_file(index)) != -1) {
		g_native_voc_len = read_archive_file(handle, g_ail_voc_buffer, 0x7fff);
		close(handle);
		return (g_native_voc_len > 0) ? 1 : 0;
	}
#endif
	return 0;
}

static void SND_play_voc(const signed int index)
{
	dsa_trace_enter(27, "seg002:SND_play_voc");
#if defined(__BORLANDC__)
	if (g_snd_voc_enabled) {

		AIL_stop_digital_playback(g_ail_digi_driver_id);
		read_new_voc_file(index);
		AIL_play_VOC_file(g_ail_digi_driver_id, g_ail_voc_buffer, -1);
		AIL_start_digital_playback(g_ail_digi_driver_id);
	}
#else
	if (g_snd_voc_enabled) {
		SND_stop_digi();
		if (read_new_voc_file(index)) {
			dsa_platform_play_voc(g_ail_voc_buffer, g_native_voc_len);
		}
	}
#endif
}

static void SND_stop_digi(void)
{
	dsa_trace_enter(28, "seg002:SND_stop_digi");
#if defined(__BORLANDC__)
	if (g_snd_voc_enabled) {
		AIL_stop_digital_playback(g_ail_digi_driver_id);
	}
#else
	if (g_snd_voc_enabled) {
		dsa_platform_stop_voc();
	}
#endif
}

static void SND_set_volume(const uint16_t volume)
{
	dsa_trace_enter(29, "seg002:SND_set_volume");
#if defined(__BORLANDC__)
	if (g_snd_voc_enabled) {

		AIL_set_digital_playback_volume(g_ail_digi_driver_id, volume);

		if (!volume) {
			SND_stop_digi();
		}
	}
#else
	if (g_snd_voc_enabled) {
		dsa_platform_set_sfx_volume((int)volume);
		if (!volume) {
			SND_stop_digi();
		}
	}
#endif
}

static signed int load_digi_driver(const char* fname, const signed int type, int16_t io, int16_t irq)
{
	dsa_trace_enter(30, "seg002:load_digi_driver");
#if defined(__BORLANDC__)
	if (io && (g_ail_digi_driver_buf = read_digi_driver(fname)) &&
		((g_ail_digi_driver_id = AIL_register_driver(g_ail_digi_driver_buf)) != -1))
	{

		g_ail_digi_driver_descr = (int16_t*)AIL_describe_driver(g_ail_digi_driver_id);

		if (g_ail_digi_driver_descr[1] == type) {

			if (io == -1) {
				io = g_ail_digi_driver_descr[6];
				irq = g_ail_digi_driver_descr[7];
			}

			if (AIL_detect_device(g_ail_digi_driver_id, io, irq,
				g_ail_digi_driver_descr[8], g_ail_digi_driver_descr[9]))
			{
				AIL_init_driver(g_ail_digi_driver_id, io, irq,
					g_ail_digi_driver_descr[8],
					g_ail_digi_driver_descr[9]);

				g_snd_effects_enabled = 1;
				return 1;
			} else {
				/* no sound hardware found */
				GUI_output(g_snd_txt_hw_not_found2);
				free_voc_buffer();
			}
		}
	}
#endif

	return 0;
}

static uint8_t* read_digi_driver(const char *fname)
{
#if defined(__BORLANDC__)
	uint32_t len;
	uint8_t *buf;
	uint32_t ptr;

	signed int handle;

	if ( (handle = open(fname, O_BINARY | O_RDONLY)) != -1) {

		len = 5000L;

		/* Puffer wie in read_music_driver() auf eine Paragraphengrenze ausrichten */

		g_ail_digi_driver_buf2 = (uint8_t*)schick_alloc(len + 16L);
		ptr = (uint32_t)g_ail_digi_driver_buf2 + 15L;
		ptr &= 0xfffffff0;
		buf = EMS_norm_ptr((uint8_t*)ptr);
		_read(handle, buf, len);
		_close(handle);
		return buf;
	}
#endif
	return NULL;
}

/**
 * \brief   opens SCHICK.DAT and seeks to desired position
 *
 * \param   fileindex   the index of the file in SCHICK.DAT
 * \return              the filehandle or 0xffff (-1).
 */
static signed int open_and_seek_dat(const uint16_t fileindex)
{
	dsa_trace_enter(31, "seg002:open_and_seek_dat");
	uint32_t start;
	uint32_t end;
	signed int handle;

	/* open SCHICK.DAT */
#if defined(__BORLANDC__)
	if ( (handle = open(g_fname_schick_dat, O_BINARY | O_RDONLY)) != -1) {
#else
	char schick_dat_buf[512];
	const char *schick_dat_path = (access(g_fname_schick_dat, 0) == 0)
		? g_fname_schick_dat
		: dsa_platform_resolve_path(g_fname_schick_dat, schick_dat_buf, sizeof(schick_dat_buf));
	if ( (handle = _open(schick_dat_path, O_BINARY | O_RDONLY)) != -1) {
#endif

		/* seek to the fileindex position in the offset table */
		lseek(handle, fileindex * 4, SEEK_SET);

		/* read the start offset of the desired file */
		_read(handle, (uint8_t*)&start, 4);

		/* REMARK: could fail on the last file */
		/* read the start offset of the next file */
		_read(handle, (uint8_t*)&end, 4);

		/* seek to the desired file */
		lseek(handle, start, SEEK_SET);

		/* save the offset of the desired file */
		g_archive_file_offset = start;

		/* save the length of the desired file in 2 variables */
		g_archive_file_length = g_archive_file_remaining = end - start;
	}

	return handle;
}

/**
 * \brief   Laenge der zuletzt geoeffneten Archivdatei
 *
 * \param   handle      Dateihandle von load_archive_file(); bei -1 wird 0 geliefert
 *                      (hiess faelschlich index, alle Aufrufer uebergeben das Handle)
 */
uint32_t get_archive_file_length(const signed int handle)
{
	dsa_trace_enter(32, "seg002:get_archive_file_length");
	return handle != -1 ? g_archive_file_length : 0;
}

/**
 * \brief   reads len bytes from the current position in SCHICK.DAT
 *
 * \param   handle      handle returned by load_archive_file
 * \param   buffer      target buffer for the data
 * \param   len         number of bytes to read
 * \return              number of bytes read
 */
uint16_t read_archive_file(const signed int handle, uint8_t *buffer, uint16_t len)
{
	dsa_trace_enter(33, "seg002:read_archive_file");
	/* no need to read */
	if (g_archive_file_remaining != 0) {

		/* adjust number of bytes to read */
		if (len > g_archive_file_remaining)
			len = (uint16_t)g_archive_file_remaining;

		g_archive_file_remaining -= len;

		return _read(handle, buffer, len);
	} else {
		return 0;
	}
}

/**
 * \brief   seeks to a position in a file in SCHICK.DAT
 *
 * \param   handle      handle returned by load_archive_file
 * \param   off         position to seek for
 * \param   ...         ein dritter Parameter (whence, stets 0) wird uebergeben, aber nicht ausgewertet
 */
void seek_archive_file(const signed int handle, int32_t off, ...)
{
	dsa_trace_enter(34, "seg002:seek_archive_file");
	uint32_t file_off;

	g_archive_file_remaining = g_archive_file_length - off;

	file_off = g_archive_file_offset + off;

	lseek(handle, file_off, SEEK_SET);

	return;
}

/* oeffnet eine Datei aus dem Spielverzeichnis (nicht aus SCHICK.DAT); bei Fehler Meldung und -1 */
signed int load_regular_file(const signed int index)
{
	dsa_trace_enter(35, "seg002:load_regular_file");
	signed int handle;

#if defined(__BORLANDC__)
	if ( (handle = open(g_fnames_v302de[index], O_BINARY | O_RDWR)) == -1) {
#else
	handle = _open(g_fnames_v302de[index], O_BINARY | O_RDWR);
	if (handle == -1) {
		handle = _open(g_fnames_v302de[index], O_BINARY | O_RDONLY);
	}
	if (handle == -1) {
		char reg_buf[512];
		const char *reg_path = dsa_platform_resolve_path(g_fnames_v302de[index], reg_buf, sizeof(reg_buf));
		handle = _open(reg_path, O_BINARY | O_RDONLY);
	}
	if (handle == -1) {
#endif

		sprintf(g_dtp2, g_str_file_missing_ptr, g_fnames_v302de[index]);

		g_missing_file_guilock = 1;
		GUI_output(g_dtp2);
		g_missing_file_guilock = 0;
	}

	return handle;
}

/**
 * \brief   opens a file stored in temp or in SCHICK.DAT
 *
 * \param   index       index of the file in SCHICK.DAT or in temp (bitwise or 0x8000)
 * \return              a file handle that can be used with read_archive_file etc.
 */
signed int load_archive_file(const signed int index)
{
	dsa_trace_enter(36, "seg002:load_archive_file");
#if defined(__BORLANDC__)
	flushall();
#endif

	return (index & 0x8000) ? open_temp_file(index & 0x7fff) : open_and_seek_dat(index);
}

/* oeffnet TEMP\<name>; fehlt die Datei, wird sie vorher aus SCHICK.DAT nach TEMP kopiert */
static signed int open_temp_file(const signed int index)
{
	dsa_trace_enter(37, "seg002:open_temp_file");
	char tmppath[40];
	signed int handle;

	sprintf((char*)tmppath, g_str_temp_fmt_ptr, g_fnames_v302de[index]);

#if defined(__BORLANDC__)
	while ( (handle = open(tmppath, O_BINARY | O_RDWR)) == -1) {
#else
	while ( (handle = _open(tmppath, O_BINARY | O_RDWR)) == -1) {
#endif

		copy_from_archive_to_temp(index, tmppath);
	}

	/* get the length of the file */
	g_archive_file_length = g_archive_file_remaining = lseek(handle, 0, 2);
	/* seek to start */
	lseek(handle, 0, 0);

	g_archive_file_offset = 0;

	return handle;
}

static void copy_from_archive_to_temp(const signed int index, const char* fname)
{
	dsa_trace_enter(38, "seg002:copy_from_archive_to_temp");
	signed int handle1;
	signed int handle2;
	signed int len;

	if ( (handle1 = load_archive_file(index)) != -1) {

		/* create new file in TEMP */
		/* TODO: should be O_BINARY | O_WRONLY */
		handle2 = _creat(fname, 0);

		/* copy it */
		while ( (len = read_archive_file(handle1, g_renderbuf_ptr, 60000)) && (len != -1))
		{
			write(handle2, g_renderbuf_ptr, len);
		}

		close(handle1);
		close(handle2);
	}
}

void copy_file_to_temp(const char* src_file, const char* fname)
{
	dsa_trace_enter(39, "seg002:copy_file_to_temp");
	signed int handle1;
	signed int handle2;
	signed int len;

#if defined(__BORLANDC__)
	if ( (handle1 = open(src_file, O_BINARY | O_RDONLY)) != -1) {
#else
	char src_buf[512];
	const char *src_path = (access(src_file, 0) == 0)
		? src_file
		: dsa_platform_resolve_path(src_file, src_buf, sizeof(src_buf));
	if ( (handle1 = _open(src_path, O_BINARY | O_RDONLY)) != -1) {
#endif

		/* create new file in TEMP */
		/* TODO: should be O_BINARY | O_WRONLY */
		handle2 = _creat(fname, 0);

		/* copy it */
		while ( (len = _read(handle1, g_renderbuf_ptr, 60000)) && (len != -1))
		{
			write(handle2, g_renderbuf_ptr, len);
		}

		close(handle1);
		close(handle2);
	}
}

/**
 * \brief   holt ein Bild aus einer NVF-Datei und entpackt es nach nvf->dst
 *
 *          NVF-Typ im ersten Byte (Bit 7 = Groessenangabe am Ende der PP20-Daten),
 *          danach Bildanzahl und je nach Typ feste oder bildweise Breite/Hoehe/Laenge.
 *          compression_type: 0 = PP20, 2..5 = RLE, sonst unkomprimiert.
 *
 * \return  Groesse des entpackten Bildes in Byte (bei RLE im Original nicht gesetzt)
 */
int32_t process_nvf_extraction(struct nvf_extract_desc *nvf)
{
	dsa_trace_enter(40, "seg002:process_nvf_extraction");
	signed int i;
	uint32_t offs;
	signed int pics;
	signed int width;
	signed int height;
	signed int va;
	uint32_t p_size;
	uint32_t retval;
	signed char nvf_type;
#if !defined(__BORLANDC__)
	uint8_t *nvf_no;
	uint8_t *src;
#else
	uint8_t huge *nvf_no;
	uint8_t huge *src;
#endif
	uint8_t *dst;

	nvf_type = *(signed char*)(nvf->src);
	va = nvf_type & 0x80;
	nvf_type &= 0x7f;

	pics = readws(nvf->src + 1L);

	if (nvf->image_num < 0)
		nvf->image_num = 0;

	if (nvf->image_num > pics - 1)
		nvf->image_num = pics - 1;

	switch (nvf_type) {

	case 0x00:
		width = readws(nvf->src + 3L);
		height = readws(nvf->src + 5L);
		p_size = width * height;
		src =  nvf->src + nvf->image_num * p_size + 7L;
		break;

	case 0x01:
		offs = pics * 4 + 3L;
		/* Original-Bug: in der Borland-Fassung werden width/height in dieser Schleife nicht gelesen,
		 * der Versatz wird also mit nicht gesetzten Werten berechnet. Nur fuer image_num > 0 relevant;
		 * vermutlich kommt NVF-Typ 1 im Spiel nicht vor. */
		for (i = 0; i < nvf->image_num; i++) {
#if !defined(__BORLANDC__)
			width = readws(nvf->src + i * 4 + 3L);
			height = readws(nvf->src + i * 4 + 5L);
#endif
			offs += width * height;
		}

		width = readws(nvf->src + nvf->image_num * 4 + 3L);
		height = readws(nvf->src + nvf->image_num * 4 + 5L);
		p_size = width * height;
		src = nvf->src + offs;
		break;

	case 0x02: case 0x04:
		width = readws(nvf->src + 3L);
		height = readws(nvf->src + 5L);
		offs = pics * 4 + 7L;
		for (i = 0; i < nvf->image_num; i++) {
			offs += readds(nvf->src + (i * 4) + 7L);
		}

		p_size = readds(nvf->src + nvf->image_num * 4 + 7L);
		src = nvf->src + offs;
		break;

	case 0x03: case 0x05:
		offs = pics * 8 + 3L;
		for (i = 0; i < nvf->image_num; i++)
			offs += readds(nvf->src + (i * 8) + 7L);

		width = readws(nvf->src + nvf->image_num * 8 + 3L);
		height = readws(nvf->src +  nvf->image_num * 8 + 5L);
		p_size = readds(nvf->src + i * 8 + 7L);
		src = nvf->src + offs;
		break;
	}

	if (!nvf->compression_type) {

		/* PP20 decompression */

		if (va != 0) {

			/* get size from unpacked picture */
			/* (die letzten 4 Byte der PP20-Daten enthalten die entpackte Laenge, 24 Bit Big Endian) */
			retval = readds(src);
			nvf_no = src;
			nvf_no += (retval + (-4L));
			retval = readds(nvf_no);
			retval = swap_u32(retval) >> 8;

		} else {
			retval = width * height;
		}

#if !defined(__BORLANDC__)
		decomp_pp20(src, nvf->dst, src + (signed char)4, p_size);
#else
		decomp_pp20(src, nvf->dst, FP_OFF(src) + 4, FP_SEG(src), p_size);
#endif

	} else if (nvf->compression_type >= 2 && nvf->compression_type <= 5) {

		dst = nvf->dst;

		/* RLE decompression */
		decomp_rle(width, height, dst, src, (uint8_t*)g_text_output_buf, nvf->compression_type);
#ifdef M302de_ORIGINAL_BUGFIX
		/* retval was originally neither set nor used here.
			VC++2008 complains about an uninitialized variable
			on a Debug build, so we fix this for debuggings sake */
		/* Orig-Fix */
		retval = p_size;
#endif
	} else {
		/* No decompression, just copy */
		memmove(nvf->dst, src, (short)p_size);
		retval = p_size;
	}

	*nvf->width = width;
	*nvf->height = height;

	return retval;
}

/**
 * \brief   does mouse programming
 *
 * \param   p1          function AX
 * \param   p2          depends on AX
 * \param   p3          depends on AX
 * \param   p4          depends on AX
 * \param   p5          depends on AX
 *
 * This function differs a bit. The Borland C-Library has a special function
 * to call interrupts. We use the one of DOSBox, which means, that we
 * put the values in the emulated registers, instead in a structure.
 *
 * (Veraltet: die Borland-Fassung ruft INT 33h ueber int86x() auf. p1..p5 entsprechen AX, BX, CX, DX
 * und ES bzw. bei Funktion 10h CX, DX, SI, DI.)
 */
static void mouse_action(int16_t *p1, int16_t *p2, int16_t *p3, int16_t *p4, int16_t *p5)
{
	dsa_trace_enter(41, "seg002:mouse_action");
#if defined(__BORLANDC__)
	struct SREGS sregs;
	union REGS wregs;

	if (*p1 >= 0) {

		wregs.x.ax = *p1;
		wregs.x.bx = *p2;
		wregs.x.cx = *p3;

		switch (*p1) {
		case 0x9:	/* define Cursor in graphic mode */
		case 0xc:	/* install event handler */
		case 0x14:	/* swap event handler */
		case 0x16:	/* save mouse state */
		case 0x17:	/* load mouse state */
		{
			wregs.x.dx = *p4;
			sregs.es = *p5;
			break;
		}
		case 0x10: {
			wregs.x.cx = *p2;
			wregs.x.dx = *p3;
			wregs.x.si = *p4;
			wregs.x.di = *p5;
			break;
		}
		default : {
			wregs.x.dx = *p4;
		}
		}

		int86x(0x33, &wregs, &wregs, &sregs);

		if (*p1 == 0x14) {
			*p2 = sregs.es;
		} else {
			*p2 = wregs.x.bx;
		}

		*p1 = wregs.x.ax;
		*p3 = wregs.x.cx;
		*p4 = wregs.x.dx;
	}
#endif
}

#if defined(__BORLANDC__)
/* Maus-Ereignisroutine; AX = Ereignismaske des Maustreibers:
 * Bit 0 = Bewegung, Bit 1 = linke Taste gedrueckt, Bit 3 = rechte Taste gedrueckt.
 * Setzt im Stadt-/Dungeonbild den Pfeil-Cursor passend zur Mausposition. */
void interrupt mouse_isr(void)
{
	int16_t l_si = _AX;
	int16_t l1;
	int16_t l3;
	int16_t l4;
	int16_t l5;
	int16_t l6;

	if (!g_mouse_locked) {

		if (l_si & 0x2) {
			g_mouse1_event2 = 1;
			g_mouse_leftclick_event = 1;
		}

		if (l_si & 0x8) {
			g_mouse_rightclick_event = 1;
		}

		if (((gs_dungeon_id != DUNGEON_ID_NONE) || (gs_town_id != TOWN_ID_NONE)) &&
				!gs_town_loc_type && !g_dialogbox_lock && (g_pp20_index == ARCHIVE_FILE_PLAYM_UK))
		{
			g_current_cursor =	(is_mouse_in_rect( 68,  4, 171,  51) ? &g_cursor_arrow_up :
						(is_mouse_in_rect( 68, 89, 171, 136) ? &g_cursor_arrow_down :
						(is_mouse_in_rect( 16, 36,  67,  96) ? &g_cursor_arrow_left :
						(is_mouse_in_rect(172, 36, 223,  96) ? &g_cursor_arrow_right :
						(!is_mouse_in_rect(16,  4, 223, 138) ? &g_default_mouse_cursor : g_current_cursor)))));
		} else {
			if (g_dialogbox_lock) {
				g_current_cursor = &g_default_mouse_cursor;
			}
		}

		if (l_si & 1) {
			/* Funktion 3: Position abfragen, auf den erlaubten Bereich begrenzen, Funktion 4: Position setzen */
			l1 = 3;
			l4 = g_mouse_posx;
			l5 = g_mouse_posy;

			mouse_action(&l1, &l3, &l4, &l5, &l6);

			g_mouse_posx = l4;
			g_mouse_posy = l5;

			if (g_mouse_posx > g_mouse_posx_max) {
				g_mouse_posx = g_mouse_posx_max;
			}
			if (g_mouse_posx < g_mouse_posx_min) {
				g_mouse_posx = g_mouse_posx_min;
			}
			if (g_mouse_posy < g_mouse_posy_min) {
				g_mouse_posy = g_mouse_posy_min;
			}
			if (g_mouse_posy > g_mouse_posy_max) {
				g_mouse_posy = g_mouse_posy_max;
			}

			l1 = 4;
			l4 = g_mouse_posx;
			l5 = g_mouse_posy;

			mouse_action(&l1, &l3, &l4, &l5, &l6);

			g_mouse_moved = 1;
		}
	}
}
#endif

/**
 * \brief   checks if the mouse cursor is in a rectangle
 *
 * \param   x1          upper left x coordinate
 * \param   y1          upper left y coordinate
 * \param   x2          bottom right x coordinate
 * \param   y2          bottom right y coordinate
 * \return              1 if the pointer is in this rectangle, otherwise 0
 */
signed int is_mouse_in_rect(const signed int x1, const signed int y1, const signed int x2, const signed int y2)
{
	dsa_trace_enter(42, "seg002:is_mouse_in_rect");
	const signed int m_x = g_mouse_posx;
	const signed int m_y = g_mouse_posy;

	return ((m_x >= x1) && (m_x <= x2) && (m_y >= y1) && (m_y <= y2)) ? 1 : 0;
}

void mouse_init(void)
{
	dsa_trace_enter(43, "seg002:mouse_init");
	if (g_have_mouse == 2) {

#if defined(__BORLANDC__)
		int16_t l1, l2, l3, l4, l5;

		l1 = 0;

		mouse_action(&l1, &l2, &l3, &l4, &l5);

		if (l1 == 0) {
			g_have_mouse = 0;
		}
#endif

		g_current_cursor = &g_default_mouse_cursor;
		g_last_cursor = &g_default_mouse_cursor;

#if defined(__BORLANDC__)
		if (g_have_mouse == 2) {

			l1 = 4;
			l3 = g_mouse_posx;
			l4 = g_mouse_posy;

			mouse_action(&l1, &l2, &l3, &l4, &l5);

			mouse_install_handler(0x1f, (unsigned char*)&mouse_isr);
		}
#endif
	}
}

void disable_mouse(void)
{
	dsa_trace_enter(44, "seg002:disable_mouse");
	if (g_have_mouse == 2) {
		mouse_reset_ehandler();
	}
}

#if defined(__BORLANDC__)
#if !FIX_PLATZ_SEG002	/* nie aufgerufen; entfaellt mit Behebungen in diesem residenten Modul (Platz) */
void mouse_get_button_press_info(int16_t *a1, int16_t *a2, int16_t *a3, int16_t *a4)
{
	int16_t tmp;

	*a1 = 5;

	mouse_action(a1, a2, a3, a4, &tmp);
}
#endif

void call_mouse_isr(void)
{
	mouse_isr();
}

/* installiert call_mouse_isr() als Ereignisroutine des Maustreibers (INT 33h, Funktion 0Ch)
 * und biegt nebenbei INT 78h auf isr um.
 * event_mask (hiess irq_no) ist keine IRQ-Nummer, sondern die Ereignismaske fuer CX (0x1f = alle Ereignisse). */
void mouse_install_handler(const int16_t event_mask, void interrupt *(isr))
{
	int16_t l1;
	int16_t l3;
	int16_t l4;
	int16_t l5;
	int16_t l6;

	l1 = 12;
	l4 = event_mask;

	l5 = FP_OFF(call_mouse_isr);
	l6 = FP_SEG(call_mouse_isr);

	g_mouse_handler_bak = getvect(0x78);
	setvect(0x78, (void interrupt far (*)(__CPPARGS))isr);

	mouse_action(&l1, &l3, &l4, &l5, &l6);

	g_mouse_irq_init = 1;
}
#endif


/* stellt INT 78h wieder her und meldet die Ereignisroutine beim Maustreiber ab (Funktion 0Ch, Maske 0) */
void mouse_reset_ehandler(void)
{
	dsa_trace_enter(45, "seg002:mouse_reset_ehandler");
#if defined(__BORLANDC__)
	int16_t l1;
	int16_t l2;
	int16_t l3;
	int16_t l4;
	int16_t l5;

	setvect(0x78, g_mouse_handler_bak);

	l1 = 12;
	l3 = 0;
	l4 = 0;
	l5 = 0;

	mouse_action(&l1, &l2, &l3, &l4, &l5);

	g_mouse_irq_init = 0;
#endif
}

/**
 * \brief   move the mouse cursor to a position on the screen
 *
 * \param   x           X - coordinate
 * \param   y           Y - coordinate
 */
void mouse_move_cursor(const signed int x, const signed int y)
{
	dsa_trace_enter(46, "seg002:mouse_move_cursor");
#if defined(__BORLANDC__)
	int16_t l1 = 4;
	int16_t l3;
	int16_t l4 = x;
	int16_t l5 = y;
	int16_t l6;

	mouse_action(&l1, &l3, &l4, &l5, &l6);
#else
	dsa_platform_set_mouse_pos(x, y);
#endif
}

#if defined(__BORLANDC__)
/* unused */
/* INT 33h, Funktion 9: Grafik-Mauszeiger definieren (Hotspot, Maske bei mask_seg:mask_off) */
#if !FIX_PLATZ_SEG002	/* nie aufgerufen; entfaellt mit Behebungen in diesem residenten Modul (Platz) */
static void mouse_setGMask(const uint16_t h_spot, const uint16_t v_spot, uint16_t mask_off, uint16_t mask_seg)
{
	int16_t l1 = 9;
	int16_t l3 = h_spot;
	int16_t l4 = v_spot;
	int16_t l5 = mask_off;
	int16_t l6 = mask_seg;

	mouse_action(&l1, &l3, &l4, &l5, &l6);
}

/* unused */
/* INT 33h, Funktion 1Dh: Bildschirmseite fuer den Mauszeiger setzen */
static void mouse_setCRTpage(const uint16_t page)
{
	int16_t l1 = 29;
	int16_t l3 = page;
	int16_t l4;
	int16_t l5;
	int16_t l6;

	mouse_action(&l1, &l3, &l4, &l5, &l6);
}
#endif
#endif


/**
 * \brief   makes a mouse cursor from a selected item
 *
 * \param icon	pointer to the icon of the item 16x16
 */
void make_ggst_cursor(uint8_t *icon)
{
	dsa_trace_enter(47, "seg002:make_ggst_cursor");
	signed int y;
	signed int x;

	/* clear the bitmask */
	for (y = 0; y < 16; y++) {
		g_ggst_cursor.mask[y] = 0;
	}

	/* make a bitmask from the icon */
	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			/* if pixelcolor of the icon is not black */
			if (*icon++ != 0x40) {
				g_ggst_cursor.mask[y] |= (0x8000 >> x);
			}
		}
	}

	/* copy and negate the bitmask */
	for (y = 0; y < 16; y++) {
		g_ggst_cursor.data[y] = ~g_ggst_cursor.mask[y];
	}
}

#if !defined(__BORLANDC__)
const int g_port_fix_stufe = FIX_BUGS;
#endif
void call_mouse_bg(void)
{
	dsa_trace_enter(48, "seg002:call_mouse_bg");
	mouse_bg();
}

void call_mouse(void)
{
	dsa_trace_enter(49, "seg002:call_mouse");
	mouse_cursor();
}

static void mouse_bg(void)
{
	dsa_trace_enter(50, "seg002:mouse_bg");
	if (!g_mouse_locked) {

		if (!g_mouse_refresh_flag) {

			g_mouse_locked = 1;
			restore_mouse_bg();
			g_mouse_locked = 0;
		}

		g_mouse_refresh_flag--;
	}
}

static void mouse_cursor(void)
{
	dsa_trace_enter(51, "seg002:mouse_cursor");
	/* check lock */
	if (!g_mouse_locked) {

		g_mouse_refresh_flag++;

		if (!g_mouse_refresh_flag) {

			/* get lock */
			g_mouse_locked = 1;
#if FIX_ZEIGER && defined(__BORLANDC__)
			g_last_cursor = g_current_cursor;
			g_mouse_pointer_offsetx = g_mouse_pointer_offsety =
				(g_current_cursor == &g_default_mouse_cursor) ? 0 : 8;
#endif

			if (g_mouse_posx < g_mouse_pointer_offsetx)
				g_mouse_posx = g_mouse_pointer_offsetx;

			if (g_mouse_posx > 315)
				g_mouse_posx = 315;

			if (g_mouse_posy < g_mouse_pointer_offsety)
				g_mouse_posy = g_mouse_pointer_offsety;

			if (g_mouse_posy > 195)
				g_mouse_posy = 195;

			mouse_save_bg();

			g_mouse_posx_bak = g_mouse_posx;
			g_mouse_posy_bak = g_mouse_posy;
			g_mouse_pointer_offsetx_bak = g_mouse_pointer_offsetx;
			g_mouse_pointer_offsety_bak = g_mouse_pointer_offsety;

			mouse_cursor_draw();

			/* put lock */
			g_mouse_locked = 0;
		}
	}
}

static void mouse_motion(void)
{
	dsa_trace_enter(52, "seg002:mouse_motion");
#if !defined(__BORLANDC__)
	if (g_mouse_refresh_flag < 0) {
		return;
	}
#endif
	/* return if mouse was not moved and the cursor remains */
	if (g_mouse_moved || (g_last_cursor != g_current_cursor)) {

		/* set new cursor */
		g_last_cursor = g_current_cursor;

		/* check if the new cursor is the default cursor */
		if (g_current_cursor == &g_default_mouse_cursor) {
			/* set cursor size 0x0 */
			/* genauer: Hotspot oben links (0,0) */
			g_mouse_pointer_offsetx = g_mouse_pointer_offsety = 0;
		} else {
			/* set cursor size 8x8 */
			/* genauer: Hotspot in der Mitte des 16x16-Cursors (8,8) */
			g_mouse_pointer_offsetx = g_mouse_pointer_offsety = 8;
		}

		/* reset mouse was moved */
		g_mouse_moved = 0;
		mouse_bg();
		mouse_cursor();
	}
}

#if !defined(__BORLANDC__)
/**
 * Emulation des DOS-Timer-Interrupts (INT 8, 18,2 Hz).
 * Ruft timer_isr() (seg004.c) entsprechend der verstrichenen Echtzeit auf.
 */
static void sdl_timer_emulation(void)
{
	static double next = 0.0;
	static int in_timer = 0;
	const double period = 1000.0 / 18.2065;
	const double now = (double)dsa_platform_get_ticks_ms();
	int ticks = 0;

	if (!g_native_timer_active || in_timer) return;

	if (next == 0.0 || now - next > 10.0 * period) next = now;

	in_timer = 1;
	while (now >= next && ticks < 4) {
		timer_isr();
		next += period;
		ticks++;
	}
	in_timer = 0;
}

static void sdl_poll_input(void)
{
	int mx = g_mouse_posx, my = g_mouse_posy;
	int moved = 0, left_click = 0, right_click = 0;

	sdl_timer_emulation();

	dsa_platform_set_mouse_range_x(g_mouse_posx_min, g_mouse_posx_max);
	dsa_platform_set_mouse_range_y(g_mouse_posy_min, g_mouse_posy_max);
	dsa_platform_poll_mouse_events(&mx, &my, &moved, &left_click, &right_click);

	if (left_click) {
		g_mouse1_event2 = 1;
		g_mouse_leftclick_event = 1;
	}
	if (right_click) {
		g_mouse_rightclick_event = 1;
	}

	if (!g_mouse_locked) {
		if (moved || left_click || right_click) {
			g_mouse_posx = mx;
			g_mouse_posy = my;
			if (g_mouse_posx > g_mouse_posx_max) g_mouse_posx = g_mouse_posx_max;
			if (g_mouse_posx < g_mouse_posx_min) g_mouse_posx = g_mouse_posx_min;
			if (g_mouse_posy < g_mouse_posy_min) g_mouse_posy = g_mouse_posy_min;
			if (g_mouse_posy > g_mouse_posy_max) g_mouse_posy = g_mouse_posy_max;
			if (moved) {
				g_mouse_moved = 1;
			}
		}
		/* Wie in mouse_isr() des DOS-Originals: Richtungspfeil-Cursor im 3D-Sichtfenster */
		if (((gs_dungeon_id != DUNGEON_ID_NONE) || (gs_town_id != TOWN_ID_NONE)) &&
			!gs_town_loc_type && !g_dialogbox_lock && (g_pp20_index == ARCHIVE_FILE_PLAYM_UK))
		{
			g_current_cursor = (is_mouse_in_rect( 68,  4, 171,  51) ? &g_cursor_arrow_up :
					   (is_mouse_in_rect( 68, 89, 171, 136) ? &g_cursor_arrow_down :
					   (is_mouse_in_rect( 16, 36,  67,  96) ? &g_cursor_arrow_left :
					   (is_mouse_in_rect(172, 36, 223,  96) ? &g_cursor_arrow_right :
					   (!is_mouse_in_rect(16,  4, 223, 138) ? &g_default_mouse_cursor : g_current_cursor)))));
		} else if (g_dialogbox_lock) {
			g_current_cursor = &g_default_mouse_cursor;
		}
	}
}

void sdl_wait_vsync(void)
{
	sdl_poll_input();
	mouse_motion();
	sdl_present_if_changed();
	dsa_platform_delay_ms(14);
}

signed int bioskey(const signed int cmd)
{
	dsa_trace_enter(53, "seg002:bioskey");
	sdl_poll_input();
	mouse_motion();
	sdl_present_if_changed();
	if (cmd == 1) {
		unsigned short ready = dsa_platform_key_ready();
		if (!ready) {
			dsa_platform_delay_ms(1);
		}
		return ready ? 1 : 0;
	} else if (cmd == 0) {
		if (dsa_platform_key_ready()) {
			return (signed int)dsa_platform_key_read();
		}
		return 0;
	}
	return 0;
}
#endif

#if FIX_ZEIGER && defined(__BORLANDC__)
/* Zusatzfunktion FIX_ZEIGER: Prueft, ob das 16x16-Feld des aktuell gezeichneten Mauszeigers das Rechteck schneidet. */
signed int zeiger_ueber_rect(const signed int x1, const signed int y1, const signed int x2, const signed int y2)
{
	const signed int rx = g_mouse_posx_bak - g_mouse_pointer_offsetx_bak;
	const signed int ry = g_mouse_posy_bak - g_mouse_pointer_offsety_bak;

	return ((rx + 15 >= x1) && (rx <= x2) && (ry + 15 >= y1) && (ry <= y2)) ? 1 : 0;
}

/* Vor pic_copy (restore == 0): Neue Hintergrundpunkte unter dem Zeiger nach g_mouse_bg_bak uebernehmen
 * und die Zeigermaske (0xff) kurz in den Quellpuffer setzen. Nach pic_copy (restore == 1): Quellpuffer wiederherstellen. */
signed int zeiger_puffer(const signed int mode, uint8_t *bak, const signed int restore)
{
	signed int x1 = g_pic_copy.x1;
	signed int y1 = g_pic_copy.y1;
	signed int width = g_pic_copy.x2 - x1 + 1;
	signed int cx1 = (x1 > g_pic_copy_rect.x1) ? x1 : g_pic_copy_rect.x1;
	signed int cy1 = (y1 > g_pic_copy_rect.y1) ? y1 : g_pic_copy_rect.y1;
	signed int cx2 = (g_pic_copy.x2 < g_pic_copy_rect.x2) ? g_pic_copy.x2 : g_pic_copy_rect.x2;
	signed int cy2 = (g_pic_copy.y2 < g_pic_copy_rect.y2) ? g_pic_copy.y2 : g_pic_copy_rect.y2;
	signed int rx = g_mouse_posx_bak - g_mouse_pointer_offsetx_bak;
	signed int ry = g_mouse_posy_bak - g_mouse_pointer_offsety_bak;
	signed int dy;
	signed int dx;
	signed int x;
	signed int y;
	signed int idx;
	uint8_t *p;
	uint8_t orig;

	if (!restore) {
		if (g_mouse_locked || (g_mouse_refresh_flag != 0) || (mode < 0) || (mode > 2) ||
			(g_pic_copy.dst != g_vga_memstart))
		{
			return 0;
		}
		g_mouse_locked = 1;
	}

	for (dy = 0; dy < 16; dy++) {
		y = ry + dy;
		if ((y >= cy1) && (y <= cy2) && (y < 200)) {
			for (dx = 0; dx < 16; dx++) {
				x = rx + dx;
				if ((x >= cx1) && (x <= cx2) && (x < 320)) {
					idx = dy * 16 + dx;
					p = g_pic_copy.src + (y - y1) * width + (x - x1);
					if (!restore) {
						orig = *p;
						bak[idx] = orig;
						if ((mode == 0) ||
							((mode == 2) && (orig != 0)) ||
							((mode == 1) && ((g_mouse_bg_bak[idx] < 0xc8) ||
								(g_pic_copy_flag && (y >= cy2 - 38) && (x >= cx2 - 148) && (x <= cx2 - 75)))))
						{
							g_mouse_bg_bak[idx] = orig;
						}
						if ((0x8000 >> dx) & g_last_cursor->mask[dy]) {
							*p = 0xff;
						}
					} else {
						*p = bak[idx];
					}
				}
			}
		}
	}

	if (restore) {
		g_mouse_locked = 0;
	}
	return 1;
}
#endif

void handle_gui_input(void)
{
	dsa_trace_enter(54, "seg002:handle_gui_input");
	signed int action_id;
	signed int tw_bak;

	g_keystroke_ascii_code = g_action_id = action_id = ACTION_ID_NONE;

	herokeeping();

	if (CD_bioskey(1)) { /* check if a keystroke is waiting in the buffer. key stays in buffer. */

		/* bioskey(0) reads 2 byte BIOS scan key from the last keystroke and removes it from the buffer.
		 * action_id stores higher byte (scan code).
		 * g_keystroke_ascii_code stores lower byte (ASCII code).
		 */
		action_id = (g_keystroke_ascii_code = bioskey(0)) >> 8;
		g_keystroke_ascii_code &= 0xff;

		/* J is mapped to Y */
		if (action_id == KEY_SCAN_CODE_J) {
			action_id = KEY_SCAN_CODE_Y;
		}

		/* Ctrl + Q -> quit */
		if ((g_keystroke_ascii_code == ASCII_CODE_CTRL_Q) && !g_pregame_state) {
			cleanup_game();

			exit(0);
		}
	}

	if (g_mouse1_event2 == 0) {

		g_always_zero3 = 0;

		if (g_have_mouse == 0) {
		}

		/* Ctrl + E */
		/* -> Helden auswaehlen und dessen Charakterbogen zeigen */
		if (g_keystroke_ascii_code == ASCII_CODE_CTRL_E) {
			status_select_hero();
			action_id = ACTION_ID_NONE;
		}

		/* Ctrl + O -> swap heroes */
		if (g_keystroke_ascii_code == ASCII_CODE_CTRL_O) {
			GRP_swap_heroes();
			action_id = ACTION_ID_NONE;
		}

		/* Ctrl + S -> sound menu */
		if ((g_keystroke_ascii_code == ASCII_CODE_CTRL_S) && !g_dialogbox_lock) {
			sound_menu();
		}

		/* Ctrl + P -> pause game */
		if ((g_keystroke_ascii_code == ASCII_CODE_CTRL_P) && (g_game_paused_flag == 0) && !g_dialogbox_lock && !g_pregame_state)
		{
			g_game_paused_flag = 1;
			g_timers_disabled++;
			g_gui_text_centered = 1;
			tw_bak = g_textbox_width;
			g_textbox_width = 2;
			GUI_output(g_pause_string);		/* P A U S E */
			g_textbox_width = tw_bak;
			g_gui_text_centered = 0;
			g_game_paused_flag = action_id = g_keystroke_ascii_code = 0;
			g_timers_disabled--;
		}
	} else {
		play_voc(ARCHIVE_FILE_FX1_VOC);
		g_mouse1_event2 = 0;
		action_id = ACTION_ID_NONE;

		if (g_action_table_secondary) {
			action_id = get_mouse_action(g_mouse_posx, g_mouse_posy, g_action_table_secondary);
		}

		if (!action_id && g_action_table_primary) {
			action_id = get_mouse_action(g_mouse_posx, g_mouse_posy, g_action_table_primary);
		}

		if (g_have_mouse == 2) {

			signed int i;

			for (i = 0; i < 15; i++) {
				wait_for_vsync();
			}

			if (g_mouse1_event2 != 0) {
				g_mouse1_doubleclick = 1;
				g_mouse1_event2 = 0;
			}
		}

		if ((action_id >= ACTION_ID_HERO_1) && (action_id <= ACTION_ID_HERO_NPC + 1)) { /* Original-Bug? Should this be '<= ACTION_ID_HERO_NPC'? */
			/* ja, die Grenze ist um eins zu gross. Harmlos: keine Maustabelle liefert 0xf8. */

			if (g_mouse1_doubleclick) {

				/* open character screen by double click on hero picture */
				if ((get_hero(action_id - ACTION_ID_HERO_1)->typus != HERO_TYPE_NONE) && (get_hero(action_id - ACTION_ID_HERO_1)->group_id == gs_active_group_id))
				{
					status_menu(action_id - ACTION_ID_HERO_1);
					action_id = ACTION_ID_NONE;
					g_mouse1_doubleclick = 0;
					g_mouse1_event2 = 0;
				}

			} else {
				/* swap heroes by click - move mouse - click */
				if (g_heroswap_allowed &&
					(get_hero(action_id - ACTION_ID_HERO_1)->typus != HERO_TYPE_NONE) && (get_hero(action_id - ACTION_ID_HERO_1)->group_id == gs_active_group_id))
				{
					/* the destination will be selected by a mouse klick in the following function call */
					GRP_move_hero(action_id - ACTION_ID_HERO_1);
					action_id = ACTION_ID_NONE;
					g_mouse1_doubleclick = 0;
					g_mouse1_event2 = 0;
				}
			}

		} else if (action_id == ACTION_ID_CREDITS) {
			/* Credits */

			action_id = ACTION_ID_NONE;
			tw_bak = g_textbox_width;
			g_textbox_width = 5;
			g_gui_text_centered = 1;
			GUI_output(get_ttx(394)); /* "CREDITS@@PROGRAMM@HANS-JÜRGEN BRÄNDLE@GUIDO HENKEL@@@CD-VERSION@ULR..." */
			g_gui_text_centered = 0;
			g_textbox_width = tw_bak;

		} else if (action_id == ACTION_ID_CLOCK) {
			/* Clock */
			action_id = ACTION_ID_NONE;
			tw_bak = g_textbox_width;
			g_textbox_width = 5;
			g_gui_text_centered = 1;
			prepare_date_str();
			GUI_output(g_dtp2);
			g_gui_text_centered = 0;
			g_textbox_width = tw_bak;

		}
	}

	mouse_motion();
	g_action_id = action_id;
}

/**
 * \brief   finds a rectangle containing (x,y)
 *
 * \param   x the x-coordinate
 * \param   y the y-coordinate
 * \param   a pointer to a (-1)-terminated list of signed 2-byte-values.
 *          each block of 5 * 2 bytes is interpreted as the coordinates of a rectangle, and a return value,
 *          in the order lower-x-bound, upper-x-bound, lower-y-bound, upper-y-bound, return value
 * \return  if (x,y) is in one of the rectangles, return the return value of the first fitting rectangle.
 *          otherwise, return 0.
 */
signed int get_mouse_action(const signed int x, const signed int y, const struct mouse_action *act)
{
	dsa_trace_enter(55, "seg002:get_mouse_action");
	signed int i;

	for (i = 0; act[i].x1 != -1; i++) {

		if ((act[i].x1 <= x) && (act[i].x2 >= x) && (act[i].y1 <= y) && (act[i].y2 >= y))
		{
			return act[i].action;
		}
	}

	return 0;
}

void handle_input(void)
{
	dsa_trace_enter(56, "seg002:handle_input");
	signed int action_id;

	g_keystroke_ascii_code = g_action_id = action_id = ACTION_ID_NONE;

	herokeeping();

	if (CD_bioskey(1)) { /* check if a keystroke is waiting in the buffer. key stays in buffer. */

		/* bioskey(0) reads 2 byte BIOS scan key from the last keystroke and removes it from the buffer.
		 * action_id stores higher byte (scan code).
		 * g_keystroke_ascii_code stores lower byte (ASCII code).
		 */
		action_id = (g_keystroke_ascii_code = bioskey(0)) >> 8;
		g_keystroke_ascii_code &= 0xff;

		/* J is mapped to Y */
		if (action_id == KEY_SCAN_CODE_J) {
			action_id = KEY_SCAN_CODE_Y;
		}

		/* Ctrl + Q -> quit */
		if ((g_keystroke_ascii_code == ASCII_CODE_CTRL_Q) && !g_pregame_state) {
			cleanup_game();

			exit(0);
		}
	}

	if (g_mouse1_event2 == 0) {

		if (g_have_mouse == 0) {
		}

		/* Ctrl + S -> sound menu */
		if ((g_keystroke_ascii_code == ASCII_CODE_CTRL_S) && !g_dialogbox_lock) {
			sound_menu();
		}

		/* Ctrl + P -> pause game */
		/* TODO: use tw_bak here */
		if ((g_keystroke_ascii_code == ASCII_CODE_CTRL_P) && (g_game_paused_flag == 0) &&
			!g_dialogbox_lock && !g_pregame_state)
		{
			g_timers_disabled++;
			g_game_paused_flag = 1;
			g_gui_text_centered = 1;
			g_textbox_width = 2;
			GUI_output(g_pause_string);		/* P A U S E */
			g_textbox_width = 3;
			g_gui_text_centered = 0;
			g_timers_disabled--;

			g_game_paused_flag = action_id = g_keystroke_ascii_code = 0;
		}
	} else {
		play_voc(ARCHIVE_FILE_FX1_VOC);
		g_mouse1_event2 = 0;
		action_id = ACTION_ID_NONE;

		if (g_action_table_secondary) {
			action_id = get_mouse_action(g_mouse_posx, g_mouse_posy, g_action_table_secondary);
		}

		if (!action_id && g_action_table_primary) {
			action_id = get_mouse_action(g_mouse_posx, g_mouse_posy, g_action_table_primary);
		}

		if (g_have_mouse == 2) {

			signed int i;

			for (i = 0; i < 25; i++) {

				wait_for_vsync();

				if (g_mouse1_event2) {
					g_mouse1_doubleclick = 1;
					g_mouse1_event2 = 0;
					break;
				}
			}
		}
	}

	mouse_motion();
	g_action_id = action_id;
}

void flush_keyboard_queue_alt(void)
{
	dsa_trace_enter(57, "seg002:flush_keyboard_queue_alt");
	while (CD_bioskey(1)) {
		bioskey(0);
	}
}

static void game_loop(void)
{
	dsa_trace_enter(58, "seg002:game_loop");
	signed int answer;

	while (g_game_state == GAME_STATE_MAIN) {

		if (gs_town_loc_type != LOCTYPE_NONE) {
			do_location();
		} else if (gs_town_id != TOWN_ID_NONE) {
			do_town();
		} else if (gs_dungeon_id != DUNGEON_ID_NONE) {
			do_dungeon();
		} else if (gs_show_travel_map != 0) {
			do_travel_mode();
		}

		if (gs_datseg_status_start == 99) {
			/* gs_datseg_status_start = 99 markiert vermutlich das Ende des Spiels (Abspann) */
			g_game_state = (GAME_STATE_OUTRO);
		}

		if (g_check_disease) {
			disease_effect();
		}

		if (g_check_poison) {
			poison_effect();
		}

		if (g_check_party) {
			/* check the party for active heroes.
			 * If necessary, switch to another group or in fatal case: game over */

			g_check_party = 0;

			if (!count_heroes_available() || ((count_heroes_available() == 1) && check_hero(get_hero(6)))) // count_heroes_available_ignore_npc() == 0
			{
				/* no heroes or only the NPC can act => GAME OVER */
				g_game_state = GAME_STATE_DEAD;

			} else if (!count_heroes_available_in_group() || ((count_heroes_available_in_group() == 1) && is_hero_available_in_group(get_hero(6)))) // count_heroes_available_in_group_ignore_npc() == 0
			{
				/* no heroes or only the NPC in this group can act => switch to next */
				GRP_switch_to_next(2);
			}

		}

		if ((get_hero(6)->typus != HERO_TYPE_NONE) &&
			((gs_town_id != TOWN_ID_NONE) || (g_game_state == GAME_STATE_VICTORY)) &&
			(gs_npc_months >= 1) &&	(g_npc_last_farewellcheck != gs_npc_months))
		{
			npc_farewell();
			g_npc_last_farewellcheck = gs_npc_months;
		}

		if (!g_in_fight &&
			((g_game_state == GAME_STATE_MAIN) || (g_game_state == GAME_STATE_VICTORY)) &&
			!gs_town_loc_type)
		{
			check_level_up();
		}

		if (g_refresh_status_line) {

			g_refresh_status_line = 0;

			if (g_pp20_index == ARCHIVE_FILE_PLAYM_UK) {
				draw_status_line();
			}
		}

		if ((g_game_state != GAME_STATE_MAIN) && (g_fading_state != 0)) {
			refresh_colors();
		}

		if (g_game_state == GAME_STATE_DEAD) {
			game_over_screen();
		}

		if (g_game_state == GAME_STATE_TIMEUP) {
			show_times_up();
		}

		if ((g_game_state == GAME_STATE_DEAD) ||
			g_game_state == GAME_STATE_UNKNOWN ||
			g_game_state == GAME_STATE_TIMEUP ||
			g_game_state == GAME_STATE_OUTRO ||
			g_game_state == GAME_STATE_FIGQUIT)
		{
			gs_town_loc_type = LOCTYPE_NONE;

			do {
				answer = load_game_state();

			} while (answer == -1);

			if (answer) {
				g_game_state = (GAME_STATE_MAIN);
				refresh_colors();
			}
		}

		if (g_game_state == GAME_STATE_VICTORY) {
			show_outro();
			cleanup_game();

			exit(0);
		}
	}
}

/*
 *	This function does daily accounting stuff.
 *	The g_check_disease flag is set here, indicating that the disease status should be updated.
 **/
static void timers_daily(void)
{
	dsa_trace_enter(59, "seg002:timers_daily");
	struct struct_hero *hero_i;
	signed int i = 0;

	/* Smith / items to repair */
	for (i = 0; i < 50; i++) {

		if (gs_smith_repairitems[i].item_id) {
			/* set time to 6:00 am */
			gs_smith_repairitems[i].pickup_time = HOURS(6);
		}
	}

	/* reset offended merchants */
	for (i = 0; i < 94; i++) {
		gs_merchant_offended_flags[i] = 0;
	}

	hero_i = get_hero(0);
	for (i = 0; i <= 6; i++, hero_i++) {

		if ((get_hero(i)->typus != HERO_TYPE_NONE) && (hero_i->recipe_timer > 0))
		{
			hero_i->recipe_timer--;
		}
	}

	/* enable disease check */
	g_check_disease = 1;

#if FIX_0039
	/* Fix 39: Akademie identifiziert und bricht Flueche am naechsten Tag wieder */
	gs_academy_daily_ident = 0;
	gs_academy_daily_curse = 0;
#endif

	/* Decrase monthly credit cens timer (bank) */
	if (gs_days_to_cens > 0) {

		gs_days_to_cens--;

		if (!gs_days_to_cens) {
			gs_monthly_credit = 0;
		}
	}

	/* Days until you run in trouble, if you have more
		than 1000S debt at the bank */
	if (gs_debt_days > 0) {

		gs_debt_days--;

		if (!gs_debt_days) {
			gs_debt_days = -1;
		}
	}
}

static void set_market_size(void)
{
	dsa_trace_enter(60, "seg002:set_market_size");
	signed int i;

	for (i = 0; g_market_descr_table[i].min_size != -1; i++) {

		g_market_descr_table[i].size = random_interval(g_market_descr_table[i].min_size, 20);
	}
}

/**
 * \brief   bewegt 32 Farben der Palette dst um einen Schritt auf die Zielpalette target zu
 *
 * \param   dst         Palette, die veraendert wird (je 3 Byte R, G, B; Werte 0..0x3f)
 * \param   target      Zielpalette (hiess p2)
 */
void pal_fade(int8_t *dst, int8_t *target)
{
	dsa_trace_enter(61, "seg002:pal_fade");
	signed int i;

	for (i = 0; i < 32; i++) {

		if ((target[3 * i] < dst[3 * i]) && (dst[3 * i] > 0))
		{
			dst[i * 3]--;

		} else {
			if ((*(target + 3 * i) > *(dst + 3 * i)) && (*(dst + 3 * i) < 0x3f))
			{
				(*(dst + i * 3))++;
			}
		}

		if ((*((target + 1) + 3 * i) < *((dst + 1) + 3 * i)) && (*((dst + 1) + 3 * i) > 0))
		{

			(*((dst + 1) + i * 3))--;

		} else {
			if ((*((target + 1) + 3 * i) > *((dst + 1) + 3 * i)) &&	(*((dst + 1) + 3 * i) < 0x3f))
			{
				(*((dst + 1) + i * 3))++;
			}
		}

		if ((*((target + 2) + 3 * i) < *((dst + 2) + 3 * i)) && (*((dst + 2) + 3 * i) > 0))
		{

			(*((dst + 2) + i * 3))--;

		} else {
			if ((*((target + 2) + 3 * i) > *((dst + 2) + 3 * i)) &&	(*((dst + 2) + 3 * i) < 0x3f))
			{
				(*((dst + 2) + i * 3))++;
			}
		}
	}
}

/**
 * \brief   Einblenden: erhoeht jeden Farbanteil von dst um 1, wenn der Zielwert mindestens 0x40 - step ist
 *          und noch nicht erreicht wurde. Mit step = 1..0x40 aufgerufen ergibt das ein Einblenden.
 *
 * \param   dst         Palette, die veraendert wird
 * \param   target      Zielpalette (hiess p2)
 * \param   step        Schritt des Einblendens (hiess v3)
 * \param   colors      Anzahl der Farben
 */
void pal_fade_in(int8_t *dst, int8_t *target, const signed int step, const signed int colors)
{
	dsa_trace_enter(62, "seg002:pal_fade_in");
	signed int i;
	signed int si;

	si = 0x40 - step;

	for (i = 0; i < colors; i++) {

		if ((*((target + 0) + 3 * i) >= si) && (*((target + 0) + 3 * i) > *((dst + 0) + 3 * i)))
		{
			(*((dst + 0) + i * 3))++;
		}

		if ((*((target + 1) + 3 * i) >= si) && (*((target + 1) + 3 * i) > *((dst + 1) + 3 * i)))
		{
			(*((dst + 1) + i * 3))++;
		}

		if ((*((target + 2) + 3 * i) >= si) && (*((target + 2) + 3 * i) > *((dst + 2) + 3 * i)))
		{
			(*((dst + 2) + i * 3))++;
		}
	}
}

/**
 * \brief   adjusts palettes in the morning
 */
static void dawning(void)
{
	dsa_trace_enter(63, "seg002:dawning");
	/* Between 6 and 7, in 64 steps (i.e. each 56 seconds) */
	if ((gs_day_timer >= HOURS(6)) && (gs_day_timer <= HOURS(7)) && !((gs_day_timer - HOURS(6)) % SECONDS(56)))
	{
		/* floor */
		pal_fade((int8_t*)gs_palette_floor, (int8_t*)g_townpal_buf);

		/* buildings */
		pal_fade((int8_t*)gs_palette_buildings, (int8_t*)g_townpal_buf + 0x60);

		/* sky */
		pal_fade((int8_t*)gs_palette_sky, (int8_t*)g_townpal_buf + 0xc0);

		/* in a town */
		if (gs_town_id && !gs_dungeon_id && !gs_town_loc_type && !gs_show_travel_map &&
			!g_event_ani_busy && !g_special_screen && (g_pp20_index == ARCHIVE_FILE_PLAYM_UK))
		{
			wait_for_vsync();

			set_palette(gs_palette_floor, 0, 0x20);
#if defined(__BORLANDC__)
			/* 0x40 Farben: gs_palette_buildings und das direkt folgende gs_palette_sky in einem Rutsch */
			set_palette(gs_palette_buildings, 0x80, 0x40);
#else
			set_palette(gs_palette_buildings, 0x80, 0x20);
			set_palette(gs_palette_sky, 0x80 + 0x20, 0x20);
#endif
		}
	}
}

/**
 * \brief   adjusts palettes in the evening
 */
static void nightfall(void)
{
	dsa_trace_enter(64, "seg002:nightfall");
	/* Between 20 and 21, in 64 steps (i.e. each 56 seconds) */
	if ((gs_day_timer >= HOURS(20)) && (gs_day_timer <= HOURS(21)) && !((gs_day_timer - HOURS(20)) % SECONDS(56)))
	{
		/* floor */
		pal_fade((int8_t*)gs_palette_floor, (int8_t*)&g_floor_fade_palette[0][0]);

		/* buildings */
		pal_fade((int8_t*)gs_palette_buildings, (int8_t*)&g_building_fade_palette[0][0]);

		/* sky */
		pal_fade((int8_t*)gs_palette_sky, (int8_t*)&g_sky_fade_palette[0][0]);

		/* in a town */
		if (gs_town_id && !gs_dungeon_id && !gs_town_loc_type && !gs_show_travel_map &&
			!g_event_ani_busy && !g_special_screen && (g_pp20_index == ARCHIVE_FILE_PLAYM_UK))
		{
			wait_for_vsync();

			set_palette(gs_palette_floor, 0, 0x20);
#if defined(__BORLANDC__)
			set_palette(gs_palette_buildings, 0x80, 0x40);
#else
			set_palette(gs_palette_buildings, 0x80, 0x20);
			set_palette(gs_palette_sky, 0x80 + 0x20, 0x20);
#endif
		}
	}
}

/**
 * \brief   get season
 *
 * \return              number of the season {0 = WINTER, 1,2,3}
 */
signed int get_current_season(void)
{
	dsa_trace_enter(65, "seg002:get_current_season");
	if (is_in_byte_array(gs_month, g_months_winter)) {
		return SEASON_WINTER;
	} else if (is_in_byte_array(gs_month, g_months_summer)) {
		return SEASON_SUMMER;
	} else if (is_in_byte_array(gs_month, g_months_spring)) {
		return SEASON_SPRING;
	} else {
		return SEASON_AUTUMN;
	}
}


#if FIX_0079
/* Fix 79: die stuendlichen Sondertimer aus do_timers() (Einhorn, Sphaerenriss, Orkmist-Faesser),
 * auch fuer die Stundenschleife in timewarp() */
static void fix_hourly_timers(void)
{
	dsa_trace_enter(66, "seg002:fix_hourly_timers");
	/* decrement unicorn timer */
	if (gs_unicorn_get_map && gs_unicorn_timer_hours)
	{
		gs_unicorn_timer_hours--;
	}

	/* handle sphere timer */
	if (gs_dng02_sphaerenriss_timer_hours) {

		if (!(--gs_dng02_sphaerenriss_timer_hours)) {
			gs_dng02_sphaerenriss_activated = 1;
		}
	}

	/* barrels with orc muck in the orc dungeon */
	if (gs_dng08_timer1) {
		gs_dng08_timer1--;
	}
	if (gs_dng08_timer2) {
		gs_dng08_timer2--;
	}
}
#endif

/**
 * \brief   calc census for the bank depot
 *
 *  If you put money on the bank, you get 5%.
 *  If you borrowed money you pay 15%.
 *
 *  ("census" = Zins; Jahreszins, monatlich mit 1/12 verrechnet. Hiess do_census.)
 */
static void do_bank_interest(void)
{
	dsa_trace_enter(67, "seg002:do_bank_interest");
	signed int sign = 0;	/* {-1, 0, 1} */
	int32_t val;

	if (gs_bank_deposit > 0) {
		sign = 1;
	} else if (gs_bank_deposit < 0) {
		sign = -1;
	}

	/* bank transactions, no census */
	/* (kein Guthaben und keine Schulden: keine Zinsen) */
	if (sign == 0)
		return;

	/* convert to heller */
	val = gs_bank_deposit * 10L;
	val += g_bank_heller;

	if (val < 0) {
		/* 15% Interest for borrowed money */
		val += val * 15 / 100L / 12L;
	} else if (val > 0) {
		/* 5% Interest for deposited money */
		val += val * 5 / 100L / 12L;
	}

	/* remember the heller */
	g_bank_heller = val % 10;

	if (val < 0) {
		g_bank_heller = -__abs__(g_bank_heller);
	}

	/* save the new deposit */
	gs_bank_deposit = val / 10;

	/* fixup over- and underflows */
	if ((gs_bank_deposit < 0) && (sign == 1)) {
		gs_bank_deposit = 32760;
	} else if ((gs_bank_deposit > 0) && (sign == -1)) {
		gs_bank_deposit = -32760;
	}
}

/* called from timewarp(..), timewarp_until_time_of_day(..),
 * timewarp_until_midnight(..) and interrupt timer_isr() */
void do_timers(void)
{
	dsa_trace_enter(68, "seg002:do_timers");
	struct struct_hero *hero_i;
	signed char afternoon;
	struct struct_hero *ptr;
	signed int i;
	signed int group_id;

	afternoon = 0;

	/* TIMERS_DISABLED is set during a fight, a level-up or if
	 * the game is paused (Ctrl + P) */
	if (g_timers_disabled)
		return;

	dawning();

	nightfall();

	/* inc day timer */
	gs_day_timer += 1L;

	if (!g_freeze_timers) {
		/* g_freeze_timers is set in timewarp(..) and timewarp_until_time_of_day(..) for efficiency reasons,
		 *  where certain timers are updated separately in a single step (instead of many 1-tick update calls). */
		sub_ingame_timers(1);
		sub_mod_timers(1);
	}

	if (!g_freeze_timers) {

		/* set day timer to pm */
		/* TODO: afternoon is useless */
		if (gs_day_timer >= HOURS(12)) {
			gs_day_timer -= HOURS(12);
			afternoon = 1;
		}

		/* every 5 minutes ingame */
		if (!(gs_day_timer % MINUTES(5))) {
			sub_heal_staffspell_timers(1);
		}

		/* every 15 minutes ingame */
		if (!(gs_day_timer % MINUTES(15))) {
			sub_light_timers(1L);
		}

		/* every hour ingame */
		if (!(gs_day_timer % HOURS(1))) {

			magical_chainmail_damage();

			/* What about herokeeping()? Original-Bug? */
			/* (Nein: magical_chainmail_damage() setzt g_herokeeping_flag, herokeeping() selbst laeuft bei jeder
			 * Eingabeabfrage (handle_input/handle_gui_input) und isst/trinkt dann einmal pro Stunde.) */

#if FIX_0079
			fix_hourly_timers();
#else
			/* decrement unicorn timer */
			if (gs_unicorn_get_map && gs_unicorn_timer_hours)
			{
				gs_unicorn_timer_hours--;
			}

			/* handle sphere timer */
			if (gs_dng02_sphaerenriss_timer_hours) {

				if (!(--gs_dng02_sphaerenriss_timer_hours)) {
					gs_dng02_sphaerenriss_activated = 1;
				}
			}

			/* barrels with orc muck in the orc dungeon */
			if (gs_dng08_timer1) {
				gs_dng08_timer1--;
			}
			if (gs_dng08_timer2) {
				gs_dng08_timer2--;
			}
#endif
		}

		/* reset the day timer to 24h time */
		if (afternoon) {
			gs_day_timer += HOURS(12);
		}
	}

	/* at 6 o'clock in the morninig */
	if (gs_day_timer == HOURS(6)) {

		hero_i = get_hero(0);

		for (i = 0; i <= 6; i++, hero_i++) {

			if ((hero_i->typus != HERO_TYPE_NONE) && (hero_i->jail != 0))
			{
				hero_i->jail = 0;

				gs_town_groups_loctype[hero_i->group_id] =
					gs_town_groups_loctype_bak[hero_i->group_id];

				gs_groups_x[hero_i->group_id] =
					gs_groups_x_bak[hero_i->group_id];

				gs_groups_y[hero_i->group_id] =
					gs_groups_y_bak[hero_i->group_id];
			}
		}
	}

	/* at 10 o'clock */
	if (gs_day_timer == HOURS(10)) {

		hero_i = get_hero(0);

		for (i = 0; i <= 6; i++, hero_i++) {

			if ((hero_i->typus != HERO_TYPE_NONE) && (hero_i->drunk != 0))
			{
				hero_get_sober(hero_i);
			}
		}
	}

	/* poison timer in the mage dungeon */
	if (gs_dng07_poison_timer) {

		/* decrement the timer */
		gs_dng07_poison_timer -= 1L;

		/* every 15 minutes do damage */
		if (gs_dng07_poison_timer % MINUTES(15) == 0) {

			ptr = get_hero(0);

			for (i = 0; i <= 6; i++, ptr++) {

				if (ptr->typus != HERO_TYPE_NONE) {

					group_id = ptr->group_id;

					/* hero is in group and in mage dungeon */
					if ((gs_active_group_id == group_id) && (gs_dungeon_id == DUNGEON_ID_RUINE_DES_SCHWARZMAGIERS))
					{

						if (gs_dungeon_level == 1) {

							/* 1W6-1 */
							sub_hero_le(ptr, dice_roll(1, 6, -1));

						} else if (gs_dungeon_level == 2) {
							/* 1W6+1 */
							sub_hero_le(ptr, dice_roll(1, 6, 1));
						}

					} else {
						if (gs_groups_dungeon_id[group_id] == DUNGEON_ID_RUINE_DES_SCHWARZMAGIERS) {

							if (gs_groups_dng_level[group_id] == 1) {
								/* 1W6-1 */
								sub_hero_le(ptr, dice_roll(1, 6, -1));

							} else if (gs_groups_dng_level[group_id] == 2) {
								/* 1W6+1 */
								sub_hero_le(ptr, dice_roll(1, 6, 1));
							}
						}
					}
				}
			}
		}
	}

	if (gs_dng12_watertrap_timer) {
		gs_dng12_watertrap_timer -= 1L;
	}

	/* at 24 o'clock, daily stuff */
	if (gs_day_timer >= HOURS(24)) {

		timers_daily();

		set_market_size();

		/* reset day timer */
		gs_day_timer = 0L;

		/* inc DAY date */
		gs_day_of_week++;
		gs_day_of_month++;

		if (gs_day_of_week == 7) {
			gs_day_of_week = 0;
		}

		/* decrement NPC timers */
		for (i = 1; i < 7; i++) {

			if (gs_npc_timers[i] && (gs_npc_timers[i] != -1))
			{
				gs_npc_timers[i]--;
			}
		}

		/* drug timer (phexcaer) */
		if (gs_drug_timer) {
			gs_drug_timer--;
		}

		/* unknown timer */
		/* = Gewaltmarsch-Timer (gs_forcedmarch_timer), zaehlt taeglich herunter */
		if (gs_forcedmarch_timer) {
			gs_forcedmarch_timer--;
		}

		/* calendar */
		if (gs_day_of_month == 31) {

			/* new month */
			gs_day_of_month = 1;
			gs_month++;

			/* increment quested months counter */
			if (gs_got_main_quest) {
				gs_quested_months++;
			}

			/* increment the months the NPC is in the group */
			if (get_hero(6)->typus != HERO_TYPE_NONE) {
				gs_npc_months++;
			}

			do_bank_interest();

			/* set days of the nameless (negative) */
			if (gs_month == 13) {
				gs_day_of_month = -5;
			}
		} else {
			/* new year */
			if (gs_day_of_month == 0) {
				gs_month = 1;
				gs_year++;
				gs_day_of_month = 1;
			}
		}

		/* check if we have a special day */
		gs_special_date_id = SPECIAL_DATE_ID_NONE;

		for (i = 0; g_special_dates[i].month != -1; i++) {

			if ((g_special_dates[i].month == gs_month) && (g_special_dates[i].day == gs_day_of_month))
			{
				gs_special_date_id = g_special_dates[i].id;
				break;
			}
		}

		passages_recalc();

		/* roll out the weather, used for passages */
		gs_weather1 = random_schick(6);
		gs_weather2 = random_schick(7);
#ifndef __BORLANDC__
		i = (gs_weather2 + 6) * (gs_weather1 * 15 + 100); /* between 805 and 2470 */
		D1_INFO_VERBOSE("WEATHER1=%d, WEATHER2=%d -> ",gs_weather1, gs_weather2);
		D1_INFO("Heutige Wetter-Anpassung der Schiffs-Geschwindigkeiten: %d,%d%%.\n",i/10,i%10);
#endif

		/* check if times up */
		if ((gs_year == 17) && (gs_month >= 10) && (gs_day_of_month >= 17)) {
			g_game_state = (GAME_STATE_TIMEUP);
		}
	}

	/* at 9 o'clock */
	if (gs_day_timer == HOURS(9)) {
		/* ships leave the harbor at 9 o'clock */
		passages_reset();
	}
}

/**
 * \brief   subtracts val from the ingame timers
 *
 * \param   val         value to subtract from the ingame timers
 * \note improvable
 */
void sub_ingame_timers(const int32_t val)
{
	dsa_trace_enter(69, "seg002:sub_ingame_timers");
	signed int i = 0;

	if (g_timers_disabled) return;

	for (i = 0; i < 26; i++) {

		/* only subtract if greater than zero */
		if (gs_ingame_timers[i] > 0) {

			/* subtract val from timer*/
			gs_ingame_timers[i] -= val;

			/* if the timer is now lower than zero, set the timer to zero */
			if (gs_ingame_timers[i] < 0) {
				gs_ingame_timers[i] = 0;
			}
		}
	}
}

/**
 * \brief   subtracts val from the modification timers
 *
 * \param   val         value to subtract from the modification timers
 *
 *          Ist ein Timer abgelaufen, wird die Aenderung (modifier) am Helden bzw. am Spielstand
 *          wieder abgezogen. Ziel ist ein Held (target = timer_id des Helden) oder der Spielstand (target = 0).
 */
void sub_mod_timers(const int32_t val)
{
	dsa_trace_enter(70, "seg002:sub_mod_timers");
	signed int i;
	signed int j;
	signed int h_index;
	HugePt mp;
	signed char target;
	unsigned char reset_target;
	struct struct_modification_timer *sp;

	/* Original-Bug: h_index wird nur hier einmal auf -1 gesetzt. Laeuft in derselben Runde ein
	 * zweiter Timer ab, dessen Ziel nicht gefunden wird, gilt noch der Held aus dem vorigen Durchlauf,
	 * und die Aenderung wird bei diesem Helden abgezogen. */
	h_index = -1;

	if (g_timers_disabled)
		return;

	for (i = 0; i < 100; i++) {

		/* if timer is 0 continue */
		if (gs_modification_timers[i].time_left == 0)
			continue;

		/* subtract diff from timer */
		gs_modification_timers[i].time_left -= val;


		/* if timer > 0 continue */
		/* (umgekehrt: nur abgelaufene Timer, time_left <= 0, werden hier behandelt) */
		if (gs_modification_timers[i].time_left <= 0) {


#if !defined(__BORLANDC__)
			D1_LOG("Mod Timer %d rueckgesetzt\n", i);
#endif

			/* set timer to 0 */
			gs_modification_timers[i].time_left = 0;

			/* make a pointer to the slot */
			sp = &gs_modification_timers[i];

			if (sp->target) {
				/* target is a hero/npc */

#if FIX_0229
				/* Fix 229: Held fuer jeden Timer neu suchen */
				h_index = -1;
#endif
				/* get the hero index from the target */
				target = sp->target;
				for (j = 0; j <= 6; j++) {
					if (get_hero(j)->timer_id == target) {
						h_index = j;
						break;
					}
				}

				if (h_index != -1) {
					/* if a hero/npc is determined */

					mp = (uint8_t*)get_hero(h_index);
					/* make a pointer to the hero's attribute mod */
					mp += (uint32_t)sp->offset;
					/* subtract the mod */
					*mp -= sp->modifier;

					if (g_pp20_index == ARCHIVE_FILE_ZUSTA_UK) {
						g_request_refresh = 1;
					}

					/* reset target */
					sp->target = 0;

					/* reset target if no other slots of target */
					reset_target = 1;
					for (j = 0; j < 100; j++) {
						if (gs_modification_timers[j].target == target) {
							reset_target = 0;
							break;
						}
					}

					if (reset_target) {
						get_hero(h_index)->timer_id = 0;
					}
				} else {
#if !defined(__BORLANDC__)
					D1_ERR("Invalid Mod Timer Target %d\n", target);
#endif

					/* reset all slots of invalid target */
					/* Original-Bug: im Schleifenrumpf steht sp statt &gs_modification_timers[j];
					 * zurueckgesetzt wird also nur der aktuelle Eintrag, die anderen bleiben stehen. */
					for (j = 0; j < 100; j++) {
						if (gs_modification_timers[j].target == target) {
#if FIX_0230
							/* Fix 230: alle Eintraege des ungueltigen Ziels zuruecksetzen */
							gs_modification_timers[j].target = gs_modification_timers[j].modifier = 0;
							gs_modification_timers[j].offset = 0;
#else
							sp->target = sp->modifier = 0;
							sp->offset = 0;
#endif
						}
					}
				}

			} else {
				/* target affects the savegame */
				mp = (uint8_t*)&gs_datseg_status_start;
				mp += sp->offset;
				*mp -= sp->modifier;
			}

			/* reset offset, target, and modificator */
			sp->target = sp->modifier = 0;
			sp->offset = 0;
		}
	}
}

/**
 * \brief   get a free modification slot
 *
 * \return              number of the modification slot
 */
signed int get_free_mod_slot(void)
{
	dsa_trace_enter(71, "seg002:get_free_mod_slot");
	signed int i;

	for (i = 0; i < 100; i++) {

		if (gs_modification_timers[i].offset == 0) {
			break;
		}
	}

	if (i == 100) {
		/* all 100 mod timers are in use. apply hack to free timer in slot 0 */
		/* Nebenwirkung: alle anderen Timer werden dabei ebenfalls um 1 verkuerzt.
		 * Original-Bug (vermutlich selten): ist g_timers_disabled gesetzt (Kampf), kehrt sub_mod_timers()
		 * sofort zurueck, Slot 0 wird nicht frei und trotzdem neu vergeben; dessen alte Aenderung
		 * wird dann nie mehr zurueckgenommen. */

		/* set timer of slot 0 to 1 */
		gs_modification_timers[0].time_left = 1;
		/* subtract one from each mod timer -> timer in slot 0 will be freed. */
#if FIX_0231
		/* Fix 231: auch im Kampf (g_timers_disabled) wirklich freigeben */
		i = g_timers_disabled;
		g_timers_disabled = 0;
		sub_mod_timers(1);
		g_timers_disabled = i;
#else
		sub_mod_timers(1);
#endif

		/* now timer slot 0 can be used again. */
		return 0;
	}

	return i;
}

/**
 * \brief   aendert ein Byte zeitlich begrenzt und traegt die Ruecknahme in einen Modifikations-Timer ein
 *
 * \param   slot_no     Nummer des Timers (von get_free_mod_slot())
 * \param   timer_value Dauer in Ticks
 * \param   ptr         Zeiger auf das Byte im Helden bzw. im Spielstand
 * \param   mod         Aenderung, die sofort addiert und nach Ablauf wieder abgezogen wird
 * \param   who         Position des Helden oder -1 fuer den Spielstand
 */
void set_mod_slot(const signed int slot_no, const int32_t timer_value, const uint8_t *ptr, const signed char mod, const signed char who)
{
	dsa_trace_enter(72, "seg002:set_mod_slot");
	signed int j;

#if !defined (__BORLANDC__)
	uint8_t *mod_ptr;
#else
	uint8_t huge *mod_ptr;
#endif
	signed char target;
	signed int i;
	signed int new_target;


	if (who == -1) {
		/* mod slot is on savegame */
		mod_ptr = (uint8_t*)&gs_datseg_status_start;
	} else {
		/* mod slot is on a hero/npc */
		mod_ptr = (uint8_t*)get_hero(who);

		if (get_hero(who)->timer_id != 0) {
			/* hero/npc has a target number */
			target = get_hero(who)->timer_id;
		} else {
			/* hero/npc has no target number */

			for (i = 1; i < 8; i++) {

				new_target = 1;
				for (j = 0; j <= 6; j++) {
					if (get_hero(j)->timer_id == i) {
						new_target = 0;
						break;
					}
				}

				if (new_target != 0) {
					target = (signed char)i;
					break;
				}
			}

			get_hero(who)->timer_id = target;
		}

		gs_modification_timers[slot_no].target = target;
	}

	gs_modification_timers[slot_no].modifier = mod;
	gs_modification_timers[slot_no].offset = (HugePt)ptr - mod_ptr;
	gs_modification_timers[slot_no].time_left = timer_value;
	*((int8_t*)ptr) += mod;
}

/**
 * \param   fmin        five minutes
 *
 *	This function decrements the timers for the healing and staffspell timeouts.
 *	Furthermore, the g_check_poison flag is set.
 */
void sub_heal_staffspell_timers(const int32_t fmin)
{
	dsa_trace_enter(73, "seg002:sub_heal_staffspell_timers");
	signed int i;
	struct struct_hero *hero_i;

	if (g_timers_disabled)
		return;

	hero_i = get_hero(0);

	for (i = 0; i <= 6; i++, hero_i++) {

		if (hero_i->typus != HERO_TYPE_NONE) {

			/* Timer to the next healing attempt */
			if (hero_i->heal_timer > 0) {

				hero_i->heal_timer -= fmin * MINUTES(5);
#if !defined(__BORLANDC__)
				if (hero_i->heal_timer <= 0) {
					D1_INFO("%s kann wieder geheilt werden\n", hero_i->alias);
				}
#endif

				if (hero_i->heal_timer < 0) {
					hero_i->heal_timer = 0;
				}
			}

			/* Timer set after Staffspell */
			if (hero_i->staffspell_timer > 0) {

				hero_i->staffspell_timer -= fmin * MINUTES(5);
#if !defined(__BORLANDC__)
				if (hero_i->staffspell_timer <= 0) {
					D1_INFO("%s kann wieder einen Stabzauber versuchen\n", hero_i->alias);
				}

#endif
				if (hero_i->staffspell_timer < 0) {
					hero_i->staffspell_timer = 0;
				}
			}

			/* Original-Bug:
			 * Apparently, the following line is supposed to trigger poison_effect() every 5 minutes.
			 * This won't work if the game does not jump back to game_loop() where poison_effect() is called.
			 * Also, fmin > 1 triggers only one call of poison_effect().
			 * Solution could be to move the poison_effect() call from game_loop() to do_timers() and timewarp(..). */
			g_check_poison = 1;
		}
	}
}

/**
 * \brief   decrements the light timers
 *
 * \param   quarter     the time in quarters of an hour
 *
 *	This function decrements the timers of burning torches and lanterns.
 *	If the time of the lightsource is up the torch is removed from the
 *	inventory and the lantern is turned off.
 */
void sub_light_timers(const int32_t quarter)
{
	dsa_trace_enter(74, "seg002:sub_light_timers");
	signed int j;
	signed int i;

	struct struct_hero *hero_i;
	signed char tmp;

	if (g_timers_disabled)
		return;

	hero_i = get_hero(0);

	for (i = 0; i <= 6; i++, hero_i++) {
		if (hero_i->typus != HERO_TYPE_NONE) {

			if (quarter > 120) {
				tmp = 120;
			} else {
				tmp = quarter;
			}

			for (j = 0; j < NR_HERO_INVENTORY_SLOTS; j++) {

				if (hero_i->inventory[j].item_id == ITEM_ID_FACKEL__LIT) {

					/* Torch, burning */
					hero_i->inventory[j].lighting_timer -= tmp;

					if (hero_i->inventory[j].lighting_timer <= 0)
					{
						/* decrement item counter */
						hero_i->num_filled_inv_slots--;

						/* subtract weight of a torch */
						hero_i->load -= g_itemsdat[ITEM_ID_FACKEL__LIT].weight;

						/* Remove Torch from inventory */
						memset(&hero_i->inventory[j], 0, sizeof(struct inventory));
					}

				} else if (hero_i->inventory[j].item_id == ITEM_ID_LATERNE__LIT) {

					/* Lantern, burning */
					hero_i->inventory[j].lighting_timer -= tmp;

					if (hero_i->inventory[j].lighting_timer <= 0) {

						/* Set timer to 0 */
						hero_i->inventory[j].lighting_timer = 0;
						/* Set burning lantern to a not burning lantern */
						hero_i->inventory[j].item_id = ITEM_ID_LATERNE__UNLIT;
					}
				}
			}
		}
	}
}

/**
 * \brief   damage if a cursed chainmail is worn
 */
static void magical_chainmail_damage(void)
{
	dsa_trace_enter(75, "seg002:magical_chainmail_damage");
	signed int i;
	struct struct_hero *hero_i;

	if (g_timers_disabled) {
		return;
	}

	/* loest in herokeeping() das stuendliche Essen und Trinken aus (siehe dort) */
	g_herokeeping_flag = (gs_show_travel_map ? 1 : 2);

	for (i = 0; i <= 6; i++) {

		if (get_hero(i)->typus != HERO_TYPE_NONE) {

			hero_i = get_hero(i);

			if (!hero_i->flags.dead &&
				/* check if not in jail (the argument might be: heroes are forced to take off armor in jail) */
				!hero_i->jail &&
				/* check if cursed chainmail is equipped */
				(hero_i->inventory[HERO_INVENTORY_SLOT_BODY].item_id == ITEM_ID_KETTENHEMD__CURSED))
			{
				sub_hero_le(hero_i, 1);
			}
		}
	}
}

/**
 * \brief   consume food if needed and print warnings to the user
 */
void herokeeping(void)
{
	dsa_trace_enter(76, "seg002:herokeeping");
	signed int hero_pos;
	signed int inv_slot;
	struct struct_hero *hero;
	char buffer[100];

	if (g_game_state != GAME_STATE_MAIN)
		return;

	/* The actual food consumption is done only if g_herokeeping_flag is set.
	 * This happens hourly in magical_chainmail_damage()
	 * The flag is reset at the end of this function. */

	/* for each hero ..*/
	hero = get_hero(0);
	for (hero_pos = 0; hero_pos <= 6; hero_pos++, hero++) {

		/* consume food and set messages */
		/* must be vital */
		if ((hero->typus != HERO_TYPE_NONE) && g_herokeeping_flag &&
				check_hero_no3(hero) &&	!hero->jail && !g_travel_herokeeping)
		{
			/* Do the eating */

			/* check for magic bread bag in the group */
			if (get_first_hero_with_item_in_group(ITEM_ID_MAGISCHER_BROTBEUTEL, hero->group_id) == -1) {

				/* if not, check if the hero has the food amulet */
				if (inv_slot_of_item(hero, ITEM_ID_AMULETT__TRAVIA) == -1) {
					/* if not... */

					/* eat if hunger > 90 % */
					if (hero->hunger > 90) {

						/* search for Lunchpack */
						inv_slot = inv_slot_of_item(hero, ITEM_ID_PROVIANTPAKET);

						if (inv_slot != -1) {
							/* Lunchpack found, consume quiet */
							g_consume_quiet = 1;
							consume(hero, hero, inv_slot);
#if !defined(__BORLANDC__)
							D1_INFO("%s isst etwas\n", hero->alias);
#endif
							g_consume_quiet = 0;

							/* search for another Lunchpack */
							/* print last ration message */
							if (inv_slot_of_item(hero, ITEM_ID_PROVIANTPAKET) == -1) {
								gs_food_message[hero_pos] = 6;
							}
						} else {
							/* print ration warning */
							if (hero->hunger < 100) {
								gs_food_message[hero_pos] = 4;
							}
						}

					}

					if (hero->hunger < 100) {

						/* increase hunger value. FOOD_MOD is always 0 or 1 */
						if (hero->hunger_timer <= 0) {
							/* increase more (FOOD_MOD == 0 -> increase by 2. FOOD_MOD == 1 -> increase by 0.) */
							hero->hunger += 2 / (g_food_mod * 2 + 1);
						} else {
							/* increase less (FOOD_MOD == 0 -> increase by 1. FOOD_MOD == 1 -> increase by 0.) */
							hero->hunger += 1 / (g_food_mod * 2 + 1);
						}

						/* adjust hunger */
						if (hero->hunger > 100) {
							hero->hunger = 100;
						}
					} else {

						/* */
						if (hero->hunger_timer <= 0) {
							do_starve_damage(hero, hero_pos, 0);
						}
					}
				}
			} else {

				/* set hunger to 20 % */
				if (hero->hunger > 20) {
					hero->hunger = 20;
				}
			}

			/* Do the drinking */

			/* check if someone in the group of the hero has the magic bread bag */
			/* (kopierter Kommentar, falsch: hier geht es um den magischen Wasserschlauch) */
			/* check for magic waterskin in group */
			if ((get_first_hero_with_item_in_group(ITEM_ID_MAGISCHER_WASSERSCHLAUCH, hero->group_id) == -1) &&
				(((hero->group_id == gs_active_group_id) &&
					(!gs_town_id || (gs_town_id != TOWN_ID_NONE && gs_show_travel_map != 0))) ||
				((hero->group_id != gs_active_group_id) && !gs_groups_town_id[hero->group_id])))
			{
					/* check for food amulett */
					if (inv_slot_of_item(hero, ITEM_ID_AMULETT__TRAVIA) == -1) {

						/* hero should drink something */
						if (hero->thirst > 90) {

							g_consume_quiet = 1;

							/* first check for beer :) */
							inv_slot = inv_slot_of_item(hero, ITEM_ID_BIER);

							/* and then for water */
							if (inv_slot == -1) {
								inv_slot = get_full_waterskin_inv_slot(hero);
							}

							if (inv_slot != -1) {
								/* drink it */
								consume(hero, hero, inv_slot);
#if !defined(__BORLANDC__)
								D1_INFO("%s trinkt etwas\n", hero->alias);
#endif
								/* nothing to drink message */
								if ((inv_slot_of_item(hero, ITEM_ID_BIER) == -1)
									&& (get_full_waterskin_inv_slot(hero) == -1)) {
									gs_food_message[hero_pos] = 5;
								}

							} else {
								/* hero has nothing to drink */
								if (hero->thirst < 100) {
									gs_food_message[hero_pos] = 3;
								}
							}

							g_consume_quiet = 0;
						}

						if (hero->thirst < 100) {

							/* increase thirst counter food_mod is always 0 or 1 */
							if (hero->hunger_timer <= 0) {

								/* increase more (FOOD_MOD == 0 -> increase by 4. FOOD_MOD == 1 -> increase by 1.) */
								hero->thirst += 4 / (g_food_mod * 2 + 1);
							} else {

								/* increase less (FOOD_MOD == 0 -> increase by 2. FOOD_MOD == 1 -> increase by 0.) */
								hero->thirst += 2 / (g_food_mod * 2 + 1);
							}

							/* adjust thirst */
							if (hero->thirst > 100) {
								hero->thirst = 100;
							}

						} else {
							if (hero->hunger_timer <= 0) {
								do_starve_damage(hero, hero_pos, 1);
							}
						}
					}
			} else {

				/* set thirst to 20 % */
				if (hero->thirst > 20) {
					hero->thirst = 20;
				}
			}
		}

		/* print hero message */
		if (gs_food_message[hero_pos] && !g_dialogbox_lock && !g_in_fight && !g_freeze_timers)
		{
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !hero->flags.dead &&
				(!gs_show_travel_map || (g_food_message_shown[hero_pos] != gs_food_message[hero_pos]))) {

					sprintf(buffer,	 (gs_food_message[hero_pos] == 1) ? get_ttx(224): /* "UNTERNIMM ETWAS! %s IST AM VERDURSTEN!" */
							((gs_food_message[hero_pos] == 2) ? get_ttx(223) : /* "TU ETWAS! %s VERHUNGERT!" */
							((gs_food_message[hero_pos] == 3) ? get_ttx(797) : /* "%s HAT GROSSEN DURST!" */
							((gs_food_message[hero_pos] == 4) ? get_ttx(798) : /* "%s HAT GROSSEN HUNGER!" */
							((gs_food_message[hero_pos] == 5) ? get_ttx(799) : /* "%s HAT SOEBEN DEN LETZTEN TROPFEN TRINKBARES ZU SICH GENOMMEN." */
							get_ttx(800))))), /* "%s HAT SOEBEN DIE LETZTE RATION GEGESSEN." */
							hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_2ND));

					g_food_message_shown[hero_pos] = gs_food_message[hero_pos];

					GUI_output(buffer);

					if (g_pp20_index == ARCHIVE_FILE_ZUSTA_UK) {
						g_request_refresh = 1;
					}
			}

			gs_food_message[hero_pos] = 0;
		}


		/* print unconscious message */
		if (gs_unconscious_message[hero_pos] && !g_dialogbox_lock) {

			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !hero->flags.dead)
			{
					sprintf(buffer, get_ttx(789), hero->alias); /* "%s WIRD BEWUSSTLOS!" */
					GUI_output(buffer);

					if (g_pp20_index == ARCHIVE_FILE_ZUSTA_UK) {
						g_request_refresh = 1;
					}
			}

			/* reset condition */
			gs_unconscious_message[hero_pos] = 0;
		}
	}

	g_herokeeping_flag = 0;
}

static void check_level_up(void)
{
	dsa_trace_enter(77, "seg002:check_level_up");
	signed int hero_pos;
	signed int not_done;
	struct struct_hero *hero;

	if (g_timers_disabled) {
		return;
	}

	do {
		not_done = 0;
		hero = get_hero(0);
		for (hero_pos = 0; hero_pos <= 6; hero_pos++, hero++) {

			if ((hero->typus != HERO_TYPE_NONE) && !hero->flags.dead && (hero->level < 20) &&

#ifndef M302de_FEATURE_MOD
				/* Feature mod 8:
				 * Adjust AP requirement for level up to match the original DSA 2/3 rules.
				 * Without this mod, it is "off by one".
				 * For example, for level 2, 100 AP should be enough, but the game requires 101 AP. */
#if FIX_0403
				/* Fix 403: Aufstieg schon bei genau der geforderten AP-Zahl */
				(g_level_ap_tab[hero->level] <= hero->ap)
#else
				(g_level_ap_tab[hero->level] < hero->ap)
#endif
				/* could be easily done without accessing the data segment by the formula level_ap_tab[i] = 50 * i * (i+1) */
#else
				(50 * hero->level * (hero->level + 1) <= hero->ap)
				/* while we're at it, avoid accessing the data segment... */
#endif
			) {
				level_up(hero_pos);
				not_done = 1;
			}
		}

	} while (not_done);
}

void update_travelmap(void)
{
	dsa_trace_enter(78, "seg002:update_travelmap");
	signed int in_rect = 0;
	uint8_t* p1;
	uint8_t* p2;
	uint8_t* p3;
	struct struct_pic_copy pic_copy_bak = g_pic_copy; /* struct copy */

	p1 = g_buffer6_ptr + 2000;
	p2 = g_buffer6_ptr + 2100;
	p3 = g_buffer6_ptr + 1000;

#if !defined(__BORLANDC__)
	g_pic_copy.dst = g_vga_memstart;
#endif

	if (g_trv_menu_selection && gs_show_travel_map) {

		g_selected_town_anix = g_town_positions[gs_trv_signpost_menu_town_ids[g_trv_menu_selection - 1] - 1].x;
		g_selected_town_aniy = g_town_positions[gs_trv_signpost_menu_town_ids[g_trv_menu_selection - 1] - 1].y;

		g_pic_copy.x1 = g_selected_town_anix - 4;
		g_pic_copy.y1 = g_selected_town_aniy - 4;
		g_pic_copy.x2 = g_selected_town_anix + 4;
		g_pic_copy.y2 = g_selected_town_aniy + 4;
		g_pic_copy.src = p1;

		if (is_mouse_in_rect(g_pic_copy.x1 - 16, g_pic_copy.y1 - 16, g_pic_copy.x2 + 16, g_pic_copy.y2 + 16))
		{
			call_mouse_bg();
			in_rect = 1;
		}

		do_pic_copy(0);

		if (in_rect) {
			call_mouse();
		}

		g_trv_menu_selection = in_rect = 0;
	}

	if (g_current_town_over) {

		g_pic_copy.x1 = g_current_town_overx - 4;
		g_pic_copy.y1 = g_current_town_overy - 4;
		g_pic_copy.x2 = g_current_town_overx + 4;
		g_pic_copy.y2 = g_current_town_overy + 4;
		g_pic_copy.src = p2;

		if (is_mouse_in_rect(g_pic_copy.x1 - 16, g_pic_copy.y1 - 16, g_pic_copy.x2 + 16, g_pic_copy.y2 + 16))
		{
			call_mouse_bg();
			in_rect = 1;
		}

		if (g_current_town_over) {
			do_pic_copy(0);
		}

		if (in_rect) {
			call_mouse();
		}

		in_rect = g_current_town_over = 0;
	}

	if (g_current_town_anix) {

		g_pic_copy.x1 = g_current_town_anix - 4;
		g_pic_copy.y1 = g_current_town_aniy - 4;
		g_pic_copy.x2 = g_current_town_anix + 4;
		g_pic_copy.y2 = g_current_town_aniy + 4;
		g_pic_copy.src = p2;

		if (is_mouse_in_rect(g_pic_copy.x1 - 16, g_pic_copy.y1 - 16, g_pic_copy.x2 + 16, g_pic_copy.y2 + 16))
		{
			call_mouse_bg();
			in_rect = 1;
		}

		do_save_rect();

		if (in_rect) {
			call_mouse();
		}

		g_current_town_over = 1;
		g_current_town_overx = g_current_town_anix;
		g_current_town_overy = g_current_town_aniy;
		in_rect = 0;

		if (g_menu_input_busy && gs_show_travel_map) {

			g_selected_town_anix = g_town_positions[gs_trv_signpost_menu_town_ids[g_menu_selected - 1] - 1].x;
			g_selected_town_aniy = g_town_positions[gs_trv_signpost_menu_town_ids[g_menu_selected - 1] - 1].y;

			g_pic_copy.x1 = g_selected_town_anix - 4;
			g_pic_copy.y1 = g_selected_town_aniy - 4;
			g_pic_copy.x2 = g_selected_town_anix + 4;
			g_pic_copy.y2 = g_selected_town_aniy + 4;
			g_pic_copy.src = p1;

			if (is_mouse_in_rect(g_pic_copy.x1 - 16, g_pic_copy.y1 - 16, g_pic_copy.x2 + 16, g_pic_copy.y2 + 16))
			{
				call_mouse_bg();
				in_rect = 1;
			}

			do_save_rect();
			g_pic_copy.src = p3 + 100 * g_map_townmark_state;
			do_pic_copy(2);

			if (in_rect) {
				call_mouse();
			}

			g_trv_menu_selection = g_menu_selected;

			in_rect = 0;
		}

		g_pic_copy.x1 = g_current_town_anix - 4;
		g_pic_copy.y1 = g_current_town_aniy - 4;
		g_pic_copy.x2 = g_current_town_anix + 4;
		g_pic_copy.y2 = g_current_town_aniy + 4;
		g_pic_copy.src = p3 + 100 * (g_map_townmark_state + 5);

		if (is_mouse_in_rect(g_pic_copy.x1 - 16, g_pic_copy.y1 - 16, g_pic_copy.x2 + 16, g_pic_copy.y2 + 16))
		{
			call_mouse_bg();
			in_rect = 1;
		}

		do_pic_copy(2);

		if (in_rect) {
			call_mouse();
		}

		g_current_town_over = 1;
	}

	g_spinlock_flag = 0;

	g_map_townmark_state++;
	g_map_townmark_state %= 5;

	g_pic_copy = pic_copy_bak;
}

void set_and_spin_lock(void)
{
	dsa_trace_enter(79, "seg002:set_and_spin_lock");
	g_spinlock_flag = 1;

	while (g_spinlock_flag) {
#if !defined(__BORLANDC__)
		/* deadlock avoidance */
		static int cnt = 0;

		if (cnt % 256 == 0) {
			wait_for_vsync();
		}

		cnt++;
#else
	/* just spin */
#endif
	}
}

/**
 * \brief   called once every day at midnight
 */
static void passages_recalc(void)
{
	dsa_trace_enter(80, "seg002:passages_recalc");
	signed int i;
	signed int di;
	signed int frequency_modifier; /* passages are rarer in summer and still rarer in winter */
	struct sea_route *route = &g_sea_routes[0];

	i = get_current_season();

	frequency_modifier = (i == SEASON_SUMMER) ? 2 : ((i == SEASON_WINTER) ? 4 : 0);
		/* winter -> 4
		 * summer -> 2
		 * spring, autumn -> 0 */

	for (i = 0; i < (SEA_ROUTE_ID__END - 1); route++, i++) {

		if (route->passage_timer-- == -1) { /* note that dec_ptr_bs returns the old (still un-decremented) value */
			/* (heute: Post-Dekrement, verglichen wird der alte Wert) */
			/* ship of a sea passage has left yesterday -> set up a new ship of this passage */

			route->price_mod = random_interval(70, 130);
			/* random price modifier in the range 70% -- 130% */

			route->passage_timer = random_interval(0, route->frequency * 10 + route->frequency * frequency_modifier) / 10;
			/* setup timer: In how many days will a ship of this passage be available? */
			/* This results in a random number in the interval [ 0..(route->frequency + frequency_modifier) ], where
			 * all numbers have the same probabilty, except the upper end route->frequency + frequency_modifier
			 * of the interval which has only 1/10 of the probabilty of each other number. */
			/* Korrektur: die Obergrenze ist frequency * (10 + frequency_modifier) / 10, der Modifikator
			 * verlaengert die Wartezeit also um 20 % (Sommer) bzw. 40 % (Winter), er wird nicht addiert. */

			di = random_schick(100);

			route->ship_type =
				(!route->coastal_route ?
					((di <= 50) ? SHIP_TYPE_LANGSCHIFF_HIGH_SEAS : ((di <= 80) ? SHIP_TYPE_KARRACKE : (di <= 95) ? SHIP_TYPE_SCHNELLSEGLER : SHIP_TYPE_SCHNELLSEGLER_LUXURY)) :
					((di <= 10) ? SHIP_TYPE_LANGSCHIFF_COSTAL : ((di <= 40) ? SHIP_TYPE_KUESTENSEGLER : (di <= 80) ? SHIP_TYPE_KUTTER : SHIP_TYPE_FISCHERBOOT)));

#if !defined(__BORLANDC__)
			// TODO: Ausgabe fuer Schiffstyp verbessern (momentan nur Zahl)
			D1_INFO_VERBOSE("Neue Passage auf Seeroute ID %d (%s -- %s): Abfahrt in %d Tagen, Schiffstyp: %d, Preisanpassung: %d%%.\n",
					i, get_ttx(route->town_id_1 + 235), get_ttx(route->town_id_2 + 235), /* TEXT.LTX 236..287: Ortsname */
					route->passage_timer, route->ship_type,	route->price_mod);
#endif
		}
	}

	/* If a passage is hired decrement Passage days timer */
	if (gs_sea_travel_psgbooked_flag == 0xaa) {
		gs_sea_travel_psgbooked_timer--;
	}
}

/**
 * \brief   called once every day at 9 o'clock when the ships leave the harbors
 */
static void passages_reset(void)
{
	dsa_trace_enter(81, "seg002:passages_reset");
	signed int i;
	struct sea_route *route = &g_sea_routes[0];

#if !FIX_0127
	/* Original-Bug 36: the loop operates only on the first sea route (which is Thorwal-Prem) */
	/* Folge: auf allen anderen Routen faehrt das Schiff nicht um 9 Uhr ab, sondern bleibt bis Mitternacht. */
	for (i = 0; i < (SEA_ROUTE_ID__END - 1); i++)
#else
	/* Fix 127: alle Seerouten zuruecksetzen */
	for (i = 0; i < (SEA_ROUTE_ID__END - 1); route++, i++)
#endif
	{
		if (!route->passage_timer) {
			route->passage_timer = -1;
		}
	}

	/* If a passage is booked and the timer is zero, the ship leaves the harbor.
	 * Therefore, reset the flag for the passage (i.e., unbook the passage) */
	if ((gs_sea_travel_psgbooked_flag == 0xaa) && !gs_sea_travel_psgbooked_timer) {
		gs_sea_travel_psgbooked_flag = 0;
	}
}

/**
 * \brief   forwards the ingame time
 *
 * \param   time        ticks to forward
 */
void timewarp(const int32_t time)
{
	dsa_trace_enter(82, "seg002:timewarp");
	signed int hour_old;
	signed int hour_new;
	int32_t i;
	signed int td_bak;
	signed int hour_diff;
	int32_t timer_bak;

	timer_bak = gs_day_timer;
	td_bak = g_timers_disabled;
	g_timers_disabled = 0;

	g_freeze_timers = 1;
	/* this deactivates the function calls sub_ingame_timers(1); and sub_mod_timers(1); in do_timers(); within the following loop.
	 * these timers will be dealt with in a single call sub_ingame_timers(time); and sub_mod_timers(time); for efficiency reasons.
	 */

	for (i = 0; i < time; i++) {

		do_timers();

		/* REMARK: SPEEDFIX
		 if (i % 768 == 0) wait_for_vsync();
		 */
	}

	sub_ingame_timers(time);

	sub_mod_timers(time);

#if !FIX_0233
	/* Original-Bug 37:
	 * Healing time-offs, staffspell time-offs and timers for the consumption of burning lanterns and torches
	 * do not get updated in certain situations, like a step forward in a town or dungeon.
	 * Reason: Because of rounding down, time / MINUTES(5) and time / MINUTES(15) will be 0 in many situations.
	 *
	 * Not exactly a masterpiece in modular arithmetics...
	 * see https://www.crystals-dsa-foren.de/showthread.php?tid=5191&pid=146023#pid146023 */
	sub_heal_staffspell_timers(time / MINUTES(5));
	sub_light_timers(time / MINUTES(15));
#else
	/* Fix 233: den Rest der angebrochenen 5 bzw. 15 Minuten mitrechnen */
	sub_heal_staffspell_timers((time + (timer_bak % MINUTES(5))) / MINUTES(5));
	sub_light_timers((time + (timer_bak % MINUTES(15)))/ MINUTES(15));
#endif

#if !FIX_0234
	/* fixes Original-Bugs 38 and 39.
	 *
	 * Original-Bug 38:
	 * In some situations, the hourly damage from the magic chain mail is not correctly applied.
	 * For example, if the hero stays in an inn for from 8:30 till 8:00 the next day, he doesn't take any damage (while he should have suffered the damage 12 times).
	 *
	 * Below, the case (hour_old == hour_new) might also come from a difference of 23 hours (which has not been considered).
	 * (Genauer: gleiche Stunde entsteht bei 24 Stunden Differenz. Zusaetzlich muesste es beim Tageswechsel
	 * 24 - hour_old + hour_new heissen; mit 23 fehlt eine Stunde.)
	 * Furthermore, according to do_timers(..), magical_chainmail_damage() should be called at *full* hours.
	 * For example, hour_new - hour_old == 1 could mean that 0 or 1 full hours have been passed.
	 *
	 * Again, sloppy treatment of modular arithmetics. */
	hour_old = timer_bak / HOURS(1);
	hour_new = gs_day_timer / HOURS(1);

	if (hour_old != hour_new) {
		if (hour_new > hour_old) {
			hour_diff = hour_new - hour_old;
		} else {
			hour_diff = 23 - hour_old + hour_new;
		}

		for (i = 0; hour_diff > i; i++) {
			magical_chainmail_damage();
			herokeeping();
#if FIX_0079
			/* Fix 79: stuendliche Sondertimer mitzaehlen */
			fix_hourly_timers();
#endif
			/* Original-Bug 39: Timers for 2nd encounter of the unicorn, Sphaerenriss in verfallene Herberge
			 * and two timers for barrels with orc muck in the orc dungeon are not affected from passing in-game time.
			 * Reason: They are present in do_timers(), but missing at this place.
			 * See https://www.crystals-dsa-foren.de/showthread.php?tid=5191&pid=146026#pid146026 */
		}
	}
#else
	/* Fix 234: Zahl der ueberschrittenen vollen Stunden */
	for (i = 0; i < (time + (timer_bak % HOURS(1))) / HOURS(1); ++i) {
		magical_chainmail_damage();
		herokeeping();
#if FIX_0079
		/* Fix 79: stuendliche Sondertimer mitzaehlen */
		fix_hourly_timers();
#endif
	}
#endif

	/* restore variables */
	g_freeze_timers = 0;
	g_timers_disabled = td_bak;
}

/**
 * \brief   forwards the ingame time till the given time of the day.
 *
 * \param   time        ticks to forward to, e.g to 6 AM. If the passed time agrees with the current time of the day, 24 hours will be forwarded.
 */
void timewarp_until_time_of_day(const int32_t time)
{
	dsa_trace_enter(83, "seg002:timewarp_until_time_of_day");
#if FIX_0079 || FIX_0233 || FIX_0234
	/* Fixes 79, 233, 234: timewarp() benutzen, das die Behebungen schon enthaelt (gleich viele Takte) */
	/* The code of the function replicates the one of timewarp(..)
	 * Better call timewarp(..), such that we don't have to apply the same bugfixes twice.
	 * The bypassed code below suffers from Original-Bug 37 and 38. */
	if (gs_day_timer < time) {
		timewarp(time - gs_day_timer);
	} else {
		timewarp(DAYS(1) + time - gs_day_timer);
	}
#else
	signed int hour_old;
	signed int hour_new;
	int32_t i = 0;
	signed int td_bak;
	signed int j;
	signed int hour_diff;
	int32_t timer_bak = gs_day_timer;

	td_bak = g_timers_disabled;
	g_timers_disabled = 0;

	g_freeze_timers = 1;

	do {
		do_timers();
		i++;

		/* REMARK: SPEEDFIX
		 if (i % 768 == 0) wait_for_vsync();
		 */

	} while (gs_day_timer != time);

	sub_ingame_timers(i);

	sub_mod_timers(i);

	/* Original-Bug 37: see above. */
	sub_heal_staffspell_timers(i / MINUTES(5));
	sub_light_timers(i / MINUTES(15));

	hour_old = timer_bak / HOURS(1);
	hour_new = gs_day_timer / HOURS(1);

	/* Original-Bug 38: see above */
	if (hour_old != hour_new) {
		if (hour_new > hour_old) {
			hour_diff = hour_new - hour_old;
		} else {
			hour_diff = 23 - hour_old + hour_new;
		}

		for (j = 0; j < hour_diff; j++) {
			magical_chainmail_damage();
			herokeeping();
		}
	}

	/* Original-Bug:
	 * forgotten hourly timers: UNICORN_TIMER, gs_dng02_sphaerenriss_timer_hours, gs_dng08_timer1, gd_dng08_timer2
	 * see do_timers(..).
	 * For a bugfix either add code here (and in timewarp(..)), or modify do_timers(..)
	 * */

	/* restore variables */
	g_freeze_timers = 0;
	g_timers_disabled = td_bak;
#endif
}

/**
 * \brief   decrements splash timer and restores pictures
 */
void dec_splash(void)
{
	dsa_trace_enter(84, "seg002:dec_splash");
	signed int i;

	for (i = 0; i <= 6; i++) {

		if (!g_dialogbox_lock && g_hero_splash_timer[i] && !(--g_hero_splash_timer[i]) &&
			(g_pp20_index == ARCHIVE_FILE_PLAYM_UK) && !get_hero(i)->flags.dead) {

			restore_rect(g_vga_memstart, get_hero(i)->pic, g_hero_pic_posx[i], 157, 32, 32);
		}
	}
}

/**
 * \brief   draws a splash on a hero portrait
 *
 * \param   hero_pos    on which slot the splash is drawn
 * \param   type        kind of damage (0 = red,LE / !0 = yellow,AE)
 */
static void draw_splash(const signed int hero_pos, const signed int type)
{
	dsa_trace_enter(85, "seg002:draw_splash");
	if (g_pp20_index == ARCHIVE_FILE_PLAYM_UK) {

		uint8_t *splash = (type == 0 ? g_splash_le : g_splash_ae);

		restore_rect_rle(g_vga_memstart, splash, g_hero_pic_posx[hero_pos], 157, 32, 32, 2);

		/* how long the splash should be displayed */
		g_hero_splash_timer[hero_pos] = 10;
	}
}

/**
 * \brief   fast forward the ingame time to midnight
 *          If it is precicely midnight, 1 full day will be passing.
 */
void timewarp_until_midnight(void)
	/* only called at a single place, at the Swafnild encounter in seg072.cpp */
{
	/* TODO: This doesn't look all correct to me... Have all timers been considered? Why not call timewarp_until_time_of_day(..)? */
#if FIX_0128
	/* Fix 128: gewoehnlicher Zeitsprung bis Mitternacht (feste Uhrzeiten, Licht und Stunden werden richtig gezaehlt) */
	timewarp(HOURS(24) - gs_day_timer);
#else
	int32_t ticks_left;
	signed int td_bak;

	/* save the timers status */
	td_bak = g_timers_disabled;

	/* enable timers */
	g_timers_disabled = 0;

	/* calculate the ticks left on this day */
	ticks_left = (HOURS(24) - 1) - gs_day_timer;

	/* Set the day timer to one tick before midnight.
	 * Original-Bug: Hard setting the time skips events at a fixed time of the day, like getting sober at 10 o'clock. */
	gs_day_timer = HOURS(24) - 1L;

	do_timers(); /* now it is precisely midnight */
	sub_ingame_timers(ticks_left);
	sub_mod_timers(ticks_left);
	sub_heal_staffspell_timers(ticks_left / MINUTES(5)); /* I think the rounding effects are indeed correct this time. More luck than reason... */
	sub_light_timers(100); /* Original-Bug: why not sub_light_timers(ticks_left / MINUTES(15)) ?? */

	/* Original-Bug: missing: magical_chainmail_damage();
	 * Original-Bug 39: Timers for 2nd encounter of the unicorn, Sphaerenriss in verfallene Herberge
	 * and two timers for barrels with orc muck in the orc dungeon are not affected from passing in-game time. */

	/* restore the timer status */
	g_timers_disabled = td_bak;
#endif
}

void flush_keyboard_queue(void)
{
	dsa_trace_enter(86, "seg002:flush_keyboard_queue");
#if defined(__BORLANDC__)
	while (CD_bioskey(1)) {
		bioskey(0);
	}
#endif
}


/* unused */
/* zeigt msg als Kopfzeile und wartet je Zeile 150 Takte oder bis zum Tastendruck */
#if defined(__BORLANDC__)
#if !FIX_PLATZ_SEG002	/* nie aufgerufen; entfaellt mit Behebungen in diesem residenten Modul (Platz) */
static void error_msg(char *msg)
{
	vsync_or_key(150 * GUI_print_text(msg));
}
#endif
#endif

/* wartet auf Taste oder Mausklick; Leertaste pausiert (kleines "P" oben links) bis zum naechsten Tastendruck */
void wait_for_keypress(void)
{
	dsa_trace_enter(87, "seg002:wait_for_keypress");
	signed int scan_key;

#if defined(__BORLANDC__)
	flushall();
#endif

	g_mouse1_event2 = 0;

	do {
		if (CD_bioskey(1)) { /* check if a keystroke is waiting in the buffer. key stays in buffer. */

			/* bioskey(0) reads 2 byte BIOS scan key from the last keystroke and removes it from the buffer. */
			scan_key = bioskey(0);

			/* space bar pauses the game till next keypress */
			if (((scan_key & 0xff) == ' ') && (g_game_paused_flag == 0))
			{

				pause_mark_show();

				/* wait for next keypress; keycode stays in buffer */
				while (!CD_bioskey(1)) {;}

				pause_mark_hide();
				break;
			}
		}

	} while (!CD_bioskey(1) && g_mouse1_event2 == 0);

	/* remove keycode from buffer */
	if (CD_bioskey(1))
		scan_key = bioskey(0);

	g_mouse1_event2 = 0;
}

/**
 * \brief   wait until duration time is elapsed or a key is pressed
 *
 * \param   duration    the maximal time to wait (1 = 15ms, 66 = 1s)
 */
void vsync_or_key(const signed int duration)
{
	dsa_trace_enter(88, "seg002:vsync_or_key");
	signed int done = 0;
	signed int counter = 0;

	while (counter < duration) {

		g_delay_or_keypress_flag = 1;
		handle_input();
		g_delay_or_keypress_flag = 0;

		if (g_town_city_event_active) {

			if (g_action_id != ACTION_ID_NONE) {

				if (g_action_id == KEY_SCAN_CODE_SPACE_BAR) {

					pause_mark_show();
					while (!CD_bioskey(1)) { ; }
					pause_mark_hide();
					done = 1;

				} else {

					if ((g_action_id != KEY_SCAN_CODE_ARROW_UP) &&
						(g_action_id != KEY_SCAN_CODE_ARROW_DOWN) &&
						(g_action_id != KEY_SCAN_CODE_ARROW_RIGHT) &&
						(g_action_id != KEY_SCAN_CODE_ARROW_LEFT))
					{
						done = 1;
					}
				}
			} else {

				if (g_mouse_rightclick_event) {
					done = 1;
				}
			}
		} else {

			if (g_action_id != ACTION_ID_NONE) {

				if (g_action_id == KEY_SCAN_CODE_SPACE_BAR) {

					pause_mark_show();
					while (!CD_bioskey(1)) { ; }
					pause_mark_hide();
				}

				done = 1;

			} else {

				if (g_mouse_rightclick_event) {
					done = 1;
				}
			}
		}


		if (done) {
			g_mouse_rightclick_event = 0;
			g_action_id = KEY_SCAN_CODE_ENTER;
			break;
		}

		wait_for_vsync();

		counter++;
	}
}

#if defined(__BORLANDC__)
#if !FIX_PLATZ_SEG002	/* nie aufgerufen; entfaellt mit Behebungen in diesem residenten Modul (Platz) */
/* unused */
void unused_delay(const signed int no)
{
	signed int i = 0;

	while (i < no) {
		wait_for_vsync();
		i++;
	}
}

/* unused */
void unused_spinlock(void)
{
	g_unused_spinlock_flag = 1;

	while (g_unused_spinlock_flag) {
	}
}
#endif
#endif

/**
 * \brief   calculates a 32bit BigEndian value into LittleEndian
 *
 * \param   v           32bit BE value
 * \return              32bit LE value
 */
uint32_t swap_u32(uint32_t v)
{
	dsa_trace_enter(89, "seg002:swap_u32");
	register signed int tmp;
	int16_t a[2];
	int32_t *ptr = (int32_t*)(&a[0]);

	*ptr = readds((uint8_t*)&v);

	tmp = a[0];
	a[0] = swap_u16(a[1]);
	a[1] = swap_u16(tmp);

	return readds((uint8_t*)ptr);
}

#if defined(__BORLANDC__)
/* unused */
/* vertauscht nur die beiden 16-Bit-Haelften, nicht die Bytes */
#if !FIX_PLATZ_SEG002	/* nie aufgerufen; entfaellt mit Behebungen in diesem residenten Modul (Platz) */
uint32_t swap_u32_halves_unused(uint32_t v)
{
	int16_t a[2];
	signed int tmp;
	int32_t *ptr = (int32_t*)(&a[0]);

	tmp = (int16_t)(*ptr = readds((uint8_t*)&v));

	a[0] = a[1];
	a[1] = tmp;

	return readds((uint8_t*)ptr);
}
#endif
#endif

#if defined(__BORLANDC__)
/**
 * \brief   allocates EMS memory
 *
 * \param   bytes       bytes to allocate
 * \return              an EMS handle, to access the memory.
 */
signed int alloc_EMS(const int32_t bytes)
{
	/* calculate the number of needed EMS pages */
	signed int handle = bytes / 0x4000 + 1;

	/* check if enough EMS is free */
	/* (bei einem Vielfachen von 16 KB wird eine Seite zu viel angefordert) */
	if (EMS_get_num_pages_unalloced() >= handle) {

		/* if allocation was successful return the handle */
		if ((handle = EMS_alloc_pages(handle)) != 0) {
			return handle;
		}
	}

	return 0;
}

/* kopiert bytes Byte aus dem EMS-Block handle seitenweise (16 KB) nach dst */
void from_EMS(const uint8_t* dst, const signed int handle, int32_t bytes)
{
	signed int pages_copied;
	signed int pages_left;
	signed int pages_copied2;
	signed int len;
	uint8_t* ptr;

	pages_left = bytes / 0x4000 + 1;
	pages_copied2 = pages_copied = 0;

	do {
		EMS_map_memory(handle, pages_copied2++, 0);
		ptr = (uint8_t*)((HugePt)dst + (((int32_t)pages_copied) << 0x0e));
		pages_copied++;

		len = (bytes - 0x4000 > 0) ? 0x4000 : bytes;

		bytes -= 0x4000;

		memmove((void*)EMS_norm_ptr(ptr), (void*)g_ems_frame_ptr, len);

	} while (--pages_left != 0);
}

/* kopiert bytes Byte von src seitenweise (16 KB) in den EMS-Block handle */
void to_EMS(const signed int handle, const uint8_t* src, int32_t bytes)
{
	signed int pages_copied;
	signed int pages_left;
	signed int pages_copied2;
	signed int len;
	uint8_t* ptr;

	pages_left = bytes / 0x4000 + 1;
	pages_copied2 = pages_copied = 0;

	do {
		EMS_map_memory(handle, pages_copied2++, 0);
		ptr = (uint8_t*)((HugePt)src + (((int32_t)pages_copied) << 0x0e));
		pages_copied++;

		len = (bytes - 0x4000 > 0) ? 0x4000 : bytes;

		bytes -= 0x4000;

		memmove((void*)g_ems_frame_ptr, (void*)EMS_norm_ptr(ptr), len);

	} while (--pages_left != 0);
}
#endif

void clear_menu_icons(void)
{
	dsa_trace_enter(90, "seg002:clear_menu_icons");
	signed int i;

	for (i = 0; i < 9; i++) {
		g_new_menu_icons[i] = MENU_ICON_NONE;
	}
}

/**
 * \brief   draws the icons of locations
 *
 * \param   icons       number of icons
 * \param   ...         icon ids
 */
void draw_loc_icons(const signed int icons, ...)
{
	dsa_trace_enter(91, "seg002:draw_loc_icons");
	signed int icons_bak[9];
	va_list arguments;
	signed int i;
	signed int changed = 0;

	/* save icon ids in local variable */
	for (i = 0; i < 9; i++) {
		icons_bak[i] = g_new_menu_icons[i];
		g_new_menu_icons[i] = MENU_ICON_NONE;
	}

	va_start(arguments, icons);

	for (i = 0; i < icons; i++) {
		g_new_menu_icons[i] = va_arg(arguments, int);

		if (g_new_menu_icons[i] != icons_bak[i]) {
			changed = 1;
		}
	}

	va_end(arguments);

	/* war hinter den neuen Icons vorher noch ein Icon, hat sich ebenfalls etwas geaendert.
	 * Original-Bug (harmlos): bei icons == 9 wird icons_bak[9] gelesen, also hinter dem Feld. */
	if (icons_bak[i] != -1)
		changed = 1;

	if (changed && g_pp20_index == ARCHIVE_FILE_PLAYM_UK) {
		draw_icons();
	}
}

/* 1, wenn die Tageszeit ein Vielfaches von val ist */
signed int mod_day_timer(const signed int val)
{
	dsa_trace_enter(92, "seg002:mod_day_timer");
	return ((gs_day_timer % val) == 0) ? 1 : 0;
}

void draw_compass(void)
{
	dsa_trace_enter(93, "seg002:draw_compass");
	signed int width;
	signed int height;
	struct nvf_extract_desc nvf;

	/* No compass in a location */
	if (!gs_town_loc_type &&
		/* Has something to do with traveling */
		/* (kein Reiseereignis aktiv) */
		!g_travel_event_active &&
		/* Not in town or dungeon */
		/* (umgekehrt: nur in einer Stadt oder einem Dungeon) */
		((gs_dungeon_id != DUNGEON_ID_NONE) || (gs_town_id != TOWN_ID_NONE)) &&
		/* I have no clue */
		/* (vermutlich: nicht waehrend die Palette ausgeblendet ist, siehe copy_palette() in seg025) */
		(g_fading_state != 2))
	{

		/* set dst */
		nvf.dst = g_icon;
		/* set src */
		nvf.src = g_buffer6_ptr;
		/* set no */
		nvf.image_num = gs_viewdir;
		/* set type*/
		/* (0 = PP20) */
		nvf.compression_type = 0;

		nvf.width = &width;
		nvf.height = &height;

		/* process the nvf */
		process_nvf_extraction(&nvf);

		/* set x and y values */
		g_pic_copy.x1 = 94;
		g_pic_copy.y1 = 115;
		g_pic_copy.x2 = 145;
		g_pic_copy.y2 = 136;

		/* set source */
		g_pic_copy.src = g_icon;

#if !FIX_ZEIGER || !defined(__BORLANDC__)
		call_mouse_bg();
#endif
		do_pic_copy(2);
#if !FIX_ZEIGER || !defined(__BORLANDC__)
		call_mouse();
#endif
	}
}

/* liefert eine andere Gruppe auf demselben Feld (bei mehreren die letzte) oder -1 */
signed int can_merge_group(void)
{
	dsa_trace_enter(94, "seg002:can_merge_group");
	signed int group_id;
	signed int retval = -1;

	if (gs_group_member_counts[gs_active_group_id] == gs_total_hero_counter) {

		retval = -1;

	} else {

		for (group_id = 0; group_id < 6; group_id++)	{

			if ((group_id != gs_active_group_id) &&
				(0 != gs_group_member_counts[group_id]) &&
				(gs_groups_x[group_id] == gs_x) &&
				(gs_groups_y[group_id] == gs_y) &&
				(gs_town_groups_loctype[group_id] == gs_town_loc_type) &&
				(gs_groups_town_id[group_id] == gs_town_id) &&
				(gs_groups_dungeon_id[group_id] == gs_dungeon_id) &&
				(gs_groups_dng_level[group_id] == gs_dungeon_level))
			{
				retval = group_id;
			}
		}
	}

	return retval;
}

/* oberes Nibble eines Kartenbytes, bei Dungeonkarten der Feldtyp (DNG_TILE_*) */
uint16_t div16(const unsigned char val)
{
	dsa_trace_enter(95, "seg002:div16");
	return val >> 4;
}

void item_selector_mouse_select(signed int *result_ptr, const struct item_selector_item *item_selector)
/* This function is called in shops at sell/buy screens and at smiths */
{
	signed int item_selector_pos;

	if (g_have_mouse != 2) {
		return;
	}

	for (item_selector_pos = 0; item_selector_pos < 15; item_selector_pos++) {
		if ((g_merchant_items_posx[item_selector_pos] <= g_mouse_posx) &&
			(g_merchant_items_posx[item_selector_pos] + 50 >= g_mouse_posx) &&
			(g_merchant_items_posy[item_selector_pos] <= g_mouse_posy) &&
			(g_merchant_items_posy[item_selector_pos] + 17 >= g_mouse_posy) &&
			(item_selector[item_selector_pos].item_id != 0))
		{
			*result_ptr = item_selector_pos;
			return;
		}
	}
}

void item_selector_keyboard_select(signed int *result_ptr, const struct item_selector_item *item_selector)
/* This function is called in shops at sell/buy screens and at smiths */
{
	signed int item_selector_pos = *result_ptr;

	if (g_action_id == KEY_SCAN_CODE_ARROW_UP) {

		/* Key UP */
		if (item_selector_pos) {
			item_selector_pos--;
		} else {
			item_selector_pos = 14;
			while (item_selector[item_selector_pos].item_id == 0) {
				item_selector_pos--;
			}
		}

	} else if (g_action_id == KEY_SCAN_CODE_ARROW_DOWN) {

		/* Key DOWN */
		if (item_selector_pos < 14) {
			if (item_selector[item_selector_pos + 1].item_id != 0) {
				item_selector_pos++;
			} else {
				item_selector_pos = 0;
			}
		} else {
			item_selector_pos = 0;
		}

	} else if (g_action_id == KEY_SCAN_CODE_ARROW_RIGHT) {

		/* Key RIGHT */
		if (item_selector_pos < 10) {
			if (item_selector[item_selector_pos + 5].item_id != 0) {
				item_selector_pos += 5;
			}
		} else {
			item_selector_pos -= 10;
		}

	} else if (g_action_id == KEY_SCAN_CODE_ARROW_LEFT) {

		/* Key LEFT */
		if (item_selector_pos > 4) {
			item_selector_pos -= 5;
		} else {
			if (item_selector[item_selector_pos + 10].item_id != 0) {
				item_selector_pos += 10;
			}
		}
	}

	*result_ptr = item_selector_pos;
}

/**
 * \brief   marks a tile in the automap as seen
 *
 * \param   x           X coordinate
 * \param   y           Y coordinate
 */
void set_automap_tile(const signed int x, const signed int y)
{
	dsa_trace_enter(96, "seg002:set_automap_tile");
	g_automap_buf[4 * y + (x >> 3)] |= g_automap_bitmask[x & 0x07];
}

/**
 * \brief   marks all surrounding tiles in the automap as seen
 *
 * \param   x           X coordinate
 * \param   y           Y coordinate
 */
void set_automap_tiles(const signed int x, const signed int y)
{
	dsa_trace_enter(97, "seg002:set_automap_tiles");
	/* set upper line */
	if (y > 0) {

		if (x > 0) {
			set_automap_tile(x - 1, y - 1);
		}

		set_automap_tile(x, y - 1);

		if (g_map_size_x - 1 > x) {
			set_automap_tile(x + 1, y - 1);
		}
	}

	/* set middle line */
	if (x > 0) {
		set_automap_tile(x - 1, y);
	}

	set_automap_tile(x, y);

	if (g_map_size_x - 1 > x) {
		set_automap_tile(x + 1, y);
	}

	/* set lower line */
	if (y < 15) {
		if (x > 0) {
			set_automap_tile(x - 1, y + 1);
		}

		set_automap_tile(x, y + 1);

		if (g_map_size_x - 1 > x) {
			set_automap_tile(x + 1, y + 1);
		}
	}
}

/**
 * \brief   Pausenzeichen: sichert das 8x8-Feld oben links und schreibt dort ein "P"
 *
 *          (hiess ul_save, "ul" = upper left)
 */
static void pause_mark_show(void)
{
	dsa_trace_enter(98, "seg002:pause_mark_show");
	/* save gfx settings to stack */
	struct struct_pic_copy pic_copy_bak = g_pic_copy;

	/* set range (0,0) - (7,7), i.e. 8x8 square in upper left corner */
	g_pic_copy.x1 = 0;
	g_pic_copy.y1 = 0;
	g_pic_copy.x2 = 7;
	g_pic_copy.y2 = 7;

	/* set destination */
	g_pic_copy.dst = g_vga_memstart;
	/* set source */
	g_pic_copy.src = g_gfxbuf_wait_keypress;

	do_save_rect();

	GUI_print_char('P', 0, 0);

	/* restore gfx settings from stack */
	g_pic_copy = pic_copy_bak;
}

/**
 * \brief   entfernt das Pausenzeichen wieder, stellt das 8x8-Feld oben links her
 *
 *          (hiess ul_blit)
 */
static void pause_mark_hide(void)
{
	dsa_trace_enter(99, "seg002:pause_mark_hide");
	/* save gfx settings to stack */
	struct struct_pic_copy pic_copy_bak = g_pic_copy;

	/* set range (0,0) - (7,7), i.e. 8x8 square in upper left corner */
	g_pic_copy.x1 = 0;
	g_pic_copy.y1 = 0;
	g_pic_copy.x2 = 7;
	g_pic_copy.y2 = 7;

	/* set destination */
	g_pic_copy.dst = g_vga_memstart;
	/* set source */
	g_pic_copy.src = g_gfxbuf_wait_keypress;

	do_pic_copy(0);

	/* restore gfx settings from stack */
	g_pic_copy = pic_copy_bak;
}

/**
 * \brief   returns true if the hero is not sleeping, dead, petrified, unconscious, renegade or fleeing
 *
 * \param   hero        pointer to the hero
 * \return              {0, 1}
 */
signed int check_hero(const struct struct_hero *hero)
{
	dsa_trace_enter(100, "seg002:check_hero");
	if (!hero->typus || hero->flags.asleep || hero->flags.dead || hero->flags.petrified ||
		hero->flags.unconscious || hero->flags.renegade || (hero->action_id == FIG_ACTION_FLEE))
	{
		return 0;
	}

	return 1;
}

/**
 * \brief   returns true if the hero is not dead, petrified, unconscious or renegade
 *
 *          (anders als check_hero() zaehlen schlafende und fliehende Helden hier mit)
 */
static signed int check_hero_no2(const struct struct_hero *hero)
{
	dsa_trace_enter(101, "seg002:check_hero_no2");

	if (!hero->typus || hero->flags.dead || hero->flags.petrified || hero->flags.unconscious || hero->flags.renegade)
	{
		return 0;
	}

	return 1;
}

/**
 * \brief   check if hero is not dead, petrified or unconscious
 *
 * \param   hero        pointer to the hero
 * \return              {0, 1}
 */
static signed int check_hero_no3(const struct struct_hero *hero)
{
	dsa_trace_enter(102, "seg002:check_hero_no3");
	if (!hero->typus || hero->flags.dead || hero->flags.petrified || hero->flags.unconscious)
	{
		return 0;
	}

	return 1;
}

signed int is_hero_available_in_group(const struct struct_hero *hero)
{
	dsa_trace_enter(103, "seg002:is_hero_available_in_group");
	if (check_hero(hero) &&	(hero->group_id == gs_active_group_id)) {

		return 1;
	}

	return 0;
}

/**
 * \brief   subtract current ae with a splash
 *
 * \param   hero        the magicuser
 * \param   ae_cost     astralenergy to subtract
 */
void sub_ae_splash(struct struct_hero *hero, signed int ae_cost)
{
	dsa_trace_enter(104, "seg002:sub_ae_splash");
	if (!hero->flags.dead && (ae_cost > 0)) {

		signed int tmp = g_update_statusline;
		g_update_statusline = 0;

		if ((hero->typus == HERO_TYPE_MAGIER) && (hero->staff_level >= 4)) {
			/* 4th staff spell reduces AE cost by 2 */
			ae_cost -= 2;
			if (ae_cost < 0)
				ae_cost = 0;
		}

		/* Calc new AE */
		hero->ae -= ae_cost;

		/* Draw the splash */
		draw_splash(get_hero_pos(hero), 1);

		/* set AE to 0 if they have gotten lower than 0 */
		if (hero->ae < 0) {
			hero->ae = 0;
		}

		g_update_statusline = tmp;

#if FIX_0280
		/* Fix 280: AE-Leiste ausserhalb des Kampfes im 3D-Bildschirm neu zeichnen */
		if (!g_in_fight && (g_pp20_index == ARCHIVE_FILE_PLAYM_UK)) {
			draw_bar(1, get_hero_pos(hero), hero->ae, hero->ae_max, 0);
		}
#endif
#ifdef M302de_ORIGINAL_BUGFIX
		/* AE bar was not updated in pseudo 3D mode */
		/* Anmerkung: die Bedingung prueft g_mouse1_doubleclick, das mit dem Bildschirmmodus nichts zu tun hat;
		 * gemeint war vermutlich (g_pp20_index == ARCHIVE_FILE_PLAYM_UK). */
		if (!g_in_fight && g_mouse1_doubleclick) {
			/* redraw AE bar */
			draw_bar(1, get_hero_pos(hero), hero->ae, hero->ae_max, 0);
		}
#endif
	}
}

/**
 * \brief   add AE points to the current AE of a hero.
 */
void add_hero_ae(struct struct_hero* hero, const signed int ae)
{
	dsa_trace_enter(105, "seg002:add_hero_ae");
	/* dont add AE if hero is dead or ae = 0 */
	if (!hero->flags.dead && (ae > 0)) {

		signed int tmp = g_update_statusline;
		g_update_statusline = 0;

		/* add AE to hero's current AE */
		hero->ae += ae;

		/* if current AE is greater than AE maximum set current AE to AE maximum */
		if (hero->ae > hero->ae_max) {

			hero->ae = hero->ae_max;
		}

		g_update_statusline = tmp;
	}
}

/**
 * \brief   subtracts LE from a hero
 *
 * \param   hero        pointer to the hero
 * \param   le          LE the hero looses
 */
void sub_hero_le(struct struct_hero *hero, const signed int le)
{
	dsa_trace_enter(106, "seg002:sub_hero_le");
	signed int i; /* multi use: disease_id, poison_id, hero_pos */
	signed int bak;
	signed int old_le;
	struct struct_fighter *p_fighter;
	struct struct_hero *hero_i;

	if (!hero->flags.dead && (le > 0)) {

		bak = g_update_statusline;
		g_update_statusline = 0;

		/* do the damage */
		old_le = hero->le;
		hero->le -= le;

		if (hero->flags.asleep) {

			/* awake him/her */
			hero->flags.asleep = 0;

			/* in fight mode */
			if (g_in_fight) {

				p_fighter = FIG_get_fighter(hero->fighter_id);

				/* update viewing direction and more */
				p_fighter->nvf_no = hero->viewdir;
				p_fighter->reload = -1;
				p_fighter->offsetx = 0;
				p_fighter->offsety = 0;
			}
		}

		draw_splash(get_hero_pos(hero), 0);

		if (hero->le <= 0) {

			/* set LE to 0 */
			hero->le = 0;

			/* mark hero as dead */
			hero->flags.dead = 1;

			gs_unconscious_message[get_hero_pos(hero)] = 0;

			/* this is strange... */
			/* (vermutlich nur ein Wert ungleich FIG_ACTION_FLEE, damit der Tote nicht als Fliehender gilt) */
			hero->action_id = FIG_ACTION_PARRY;

			if (g_pp20_index == ARCHIVE_FILE_PLAYM_UK) {
				g_refresh_status_line = 1;
			}

			/* reset disease */
			for (i = 1; i <= 7; i++) {
				hero->disease[i].status = DISEASE_STATUS_HEALTHY;
				hero->disease[i].time_counter = 0;
			}

			/* reset poison */
			for (i = 1; i <= 9; i++) {
				hero->poison[i].status = POISON_STATUS_HEALTHY;
				hero->poison[i].time_counter = 0;
			}

			/* FINAL FIGHT */
			if (g_current_fight_id == FIGHT_ID_F144) {
				if (hero == gs_main_acting_hero) {
					g_game_state = GAME_STATE_DEAD;
					g_in_fight = 0;
				}
			}

			if (g_traveling	&& !g_in_fight &&
				(!count_heroes_available_in_group() || ((count_heroes_available_in_group() == 1) && is_hero_available_in_group(get_hero(6))))) /* count_heroes_available_in_group_ignore_npc() == 0 */
			{
				/* if traveling, not in a fight, and no hero in the group (except possibly the NPC) is available. */

				gs_travel_detour = 99;

				hero_i = get_hero(0);
				for (i = 0; i <= 6; i++, hero_i++) {

					if ((hero_i->typus != HERO_TYPE_NONE) && (hero_i->group_id == gs_active_group_id))
					{
						hero_disappear(hero_i, i, -1);
					}
				}
			}

		} else {
			if ((old_le >= 5) && (hero->le < 5)) {

				/* make hero unconscious */
				hero->flags.unconscious = 1;

				/* unknown yet */
				/* (der Bewusstlose handelt nicht mehr: Aktion "Warten") */
				hero->action_id = FIG_ACTION_WAIT;

				/* unknown yet */
				/* (loest in herokeeping() die Meldung "%s WIRD BEWUSSTLOS!" aus) */
				gs_unconscious_message[get_hero_pos(hero)] = 1;

				/* in fight mode */
				if (g_in_fight) {

					p_fighter = FIG_get_fighter(hero->fighter_id);

					p_fighter->nvf_no = g_nvftab_figures_unconscious[hero->actor_sprite_id] + hero->viewdir;

					p_fighter->reload = -1;

					p_fighter->offsetx = g_gfxtab_offsets_unconscious[hero->actor_sprite_id][hero->viewdir].x;
					p_fighter->offsety = g_gfxtab_offsets_unconscious[hero->actor_sprite_id][hero->viewdir].y;


					FIG_add_msg(7, 0);

					/* FINAL FIGHT */
					if (g_current_fight_id == FIGHT_ID_F144) {

						if (hero == gs_main_acting_hero) {

							g_game_state = GAME_STATE_DEAD;

							g_in_fight = 0;
						}
					}
				}
			}
		}

		g_update_statusline = bak;
	}

	if (!g_in_fight) {
		g_check_party = 1;
	}
}

/**
 * \brief   regenerates LE of a hero
 *
 *          This functions does some magic in fights, when a hero awakes.
 *
 * \param   hero        pointer to the hero
 * \param   le          LE to be regenerated
 */
void add_hero_le(struct struct_hero *hero, const signed int le)
{
	dsa_trace_enter(107, "seg002:add_hero_le");
	signed int val_bak;
	struct struct_fighter *p_fighter;
	signed int ret;

	/* dead heroes never get LE */
	if (!hero->flags.dead && (le > 0)) {

		val_bak = g_update_statusline;
		g_update_statusline = 0;

		/* add LE */
		hero->le += le;

		/* set LE to maximum if greater than maximum */
		if (hero->le > hero->le_max) {

			hero->le = hero->le_max;
		}

		/* if current LE is >= 5 and the hero is unconscious */
		if ((hero->le >= 5) && hero->flags.unconscious) {

			/* awake */
			hero->flags.unconscious = 0;

			/* maybe if we are in a fight */
			/* (im Kampf: Figur wieder aufrecht zeichnen) */
			if (g_in_fight) {

				p_fighter = FIG_get_fighter(hero->fighter_id);

				ret = FIG_weapon_gfx_id_ranged(hero);

				if (ret != WEAPON_GFX_ID_NONE) {

					p_fighter->nvf_no = g_nvftab_figures_rangeweapon[hero->actor_sprite_id - 1][ret][hero->viewdir];
				} else {
					p_fighter->nvf_no = hero->viewdir;
				}

				p_fighter->reload = -1;
				p_fighter->offsetx = 0;
				p_fighter->offsety = 0;
			}
		}

		g_update_statusline = val_bak;
	}
}

/**
 * \brief   regenerates LE of a group
 *
 * \param   le          LE to be regenerated
 */
void add_group_le(const signed int le)
{
	dsa_trace_enter(108, "seg002:add_group_le");
	struct struct_hero *hero;
	signed int i;

	hero = get_hero(0);
	for (i = 0; i <= 6; i++, hero++) {

		if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id))
		{
			add_hero_le(hero, le);
		}
	}
}

/**
 * \brief   damages starving heroes
 *
 * \param   hero        a pointer to the hero
 * \param   hero_pos    the index number of the hero
 * \param   type        the type of message which should be printed (0 = hunger / 1 = thirst)
 */
static void do_starve_damage(struct struct_hero *hero, const signed int hero_pos, const signed int type)
{
	dsa_trace_enter(109, "seg002:do_starve_damage");
	/* check if the hero is dead */
	if (!hero->flags.dead) {

		/* save this value locally */
		const int sl_bak = g_update_statusline;
		g_update_statusline = 0;

		/* decrement LE of the hero */
		hero->le--;

		/* set the critical message type for the hero */
		gs_food_message[hero_pos] = (type != 0 ? 1 : 2);

		if (hero->le <= 0) {

			/* don't let the hero die */
			hero->le = 1;

			/* decrement the max LE and save them at 0x7a */
			/* (gemerkt in hero->le_max_malus) */
			if (hero->le_max >= 2) {
				hero->le_max--;
				hero->le_max_malus++;
			}
		}

		/* restore the locally save value */
		g_update_statusline = sl_bak;
	}
}

#if defined(__BORLANDC__)
/* unused */
#if !FIX_PLATZ_SEG002	/* nie aufgerufen; entfaellt mit Behebungen in diesem residenten Modul (Platz) */
static signed int compare_name(const char *name)
{
	signed int i;

	for (i = 0; i < 6; i++) {

		if (!strcmp(get_hero(i)->alias, name)) {
			return 1;
		}
	}

	return 0;
}
#endif
#endif

#if FIX_PROBENLOG
/* Probenlog (Zusatzfunktion, keine Fehlerbehebung): haengt eine Zeile an PROBEN.CSV an, aber nur, wenn die Datei
 * schon da ist (anlegen = Log an, loeschen = aus). Kein neuer Text im Datensegment: der Dateiname entsteht Zeichen
 * fuer Zeichen, geschrieben werden nur Zahlen und der Heldenname.
 * Zeile: Art;Tag;Monat;Jahr;Minute des Tages;Name;Wert1;...;WertN
 * Art 1 Eigenschaftsprobe, 3 Dreifachprobe, 4 Talentprobe, 5 Zauberprobe, 6 Attacke, 7 Parade, 8 Stufenanstieg
 * (seg050; Bedeutung der Werte:
 * Probenlog.html bzw. Probenlog-Fenster) */
static char *fix_probenlog_zahl(char *p, const signed int v)
{
	dsa_trace_enter(110, "seg002:fix_probenlog_zahl");
	itoa(v, p, 10);
	p += strlen(p);
	*p++ = ';';
	return p;
}

void fix_probenlog(const signed int art, const char *name, const signed int n, const signed int *w)
{
	dsa_trace_enter(111, "seg002:fix_probenlog");
	char z[160];
	char *p;
	signed int i;
	signed int fd;

	z[0] = 'P'; z[1] = 'R'; z[2] = 'O'; z[3] = 'B'; z[4] = 'E'; z[5] = 'N';
	z[6] = '.'; z[7] = 'C'; z[8] = 'S'; z[9] = 'V'; z[10] = '\0';
	if ((fd = open(z, O_BINARY | O_WRONLY)) == -1) {
		return;
	}
	p = fix_probenlog_zahl(z, art);
	p = fix_probenlog_zahl(p, gs_day_of_month);
	p = fix_probenlog_zahl(p, gs_month);
	p = fix_probenlog_zahl(p, gs_year);
	p = fix_probenlog_zahl(p, (signed int)(gs_day_timer / MINUTES(1)));
	for (i = 0; name && (i < 16) && name[i]; i++) {
		*p++ = name[i];
	}
	*p++ = ';';
	for (i = 0; i < n; i++) {
		p = fix_probenlog_zahl(p, w[i]);
	}
	p[-1] = '\r';
	*p++ = '\n';
	lseek(fd, 0L, SEEK_END);
	write(fd, z, p - z);
	close(fd);
}
#endif

/**
 * \brief   make an attribute test
 *
 * \param   hero        pointer to the hero
 * \param   attrib_id   ID of the attribute
 * \param   handicap    may be positive or negative. The higher the value, the harder the test.
 * \return              the result of the test. > 0: success; <= 0: failure; -99: critical failure
 */
signed int test_attrib(const struct struct_hero* hero, const signed int attrib_id, const signed int handicap)
{
	dsa_trace_enter(112, "seg002:test_attrib");
#if FIX_PROBENLOG
	/* Probenlog: dieselbe Probe (gleich viele Wuerfe, gleiches Ergebnis), dazu eine Zeile Art 1:
	 * Eigenschaft, Eigenschaftswert, Zuschlag, W20, Ergebnis */
	signed int w[5];

	w[3] = random_schick(20);
	w[0] = attrib_id;
	w[1] = hero->attrib[attrib_id].current + hero->attrib[attrib_id].mod;
	w[2] = handicap;
	w[4] = (w[3] == 20) ? -99 : (w[1] - (w[3] + handicap) + 1);
	fix_probenlog(1, hero->alias, 5, w);
	return w[4];
#else
	signed int randval = random_schick(20);
	signed int att_val;

#if !defined(__BORLANDC__)
	D1_INFO("Eigenschaftsprobe %s auf %s %+d: W20 = %d", hero->alias, names_attrib[attrib_id], handicap, randval);
#endif

	if (randval == 20) {
#if !defined(__BORLANDC__)
		D1_INFO("Ungluecklich\n");
#endif
		return -99;
	} else {

		randval += handicap;
	}

	att_val = hero->attrib[attrib_id].current + hero->attrib[attrib_id].mod;

#if !defined(__BORLANDC__)
	D1_INFO(" -> %s mit %d\n", (att_val - randval + 1) > 0 ? "bestanden" : "nicht bestanden", (att_val - randval + 1));
#endif

	return att_val - randval + 1;
#endif
}

/**
 * \brief   make three attribute tests
 *
 * \param   hero          pointer to the hero
 * \param   attrib_id_1   ID of attribute 1
 * \param   attrib_id_2   ID of attribute 2
 * \param   attrib_id_3   ID of attribute 3
 * \param   handicap      may be positive or negative. The higher the value, the harder the test.
 * \return                a test is successful if the return value is greater than zero
 *                        in detail:
 *                        unlucky fail: -99
 *                        lucky success: +99 (only if M302de_FEATURE_MOD is activated)
 *                        ordinary fail: any value between -98 and 0.
 *                        ordinary success: any value between 1 and 98.
 */

signed int test_attrib3(const struct struct_hero* hero, const signed int attrib_id_1, const signed int attrib_id_2, const signed int attrib_id_3, signed char handicap)
{
	dsa_trace_enter(113, "seg002:test_attrib3");
#ifndef M302de_FEATURE_MOD
	/* Feature mod 6: The implementation of the talent test logic differs from the original DSA2/3 rules.
	 * It is sometimes called the 'pool' variant, where '3W20 + handicap' is compared to the sum of the attributes.
	 * It is significantly easier than the original rule, where each individuall roll must be at most the corresponding attribute,
	 * where positive handicap must be used up during the process, and negative handicap may be used for compensation. */

	/* in the original pool variant, the ramining pool, increased by 1, is returned. */

	/* in the modified variant, ordinary failure gives always return value 0.
	 * Ordinary success leads to return value 1, or, in the case of a negative handicap (in other words, a positive bonus),
	 * the number of remaining bonus points, increased by 1.  */

#if FIX_PROBENLOG
	/* Probenlog: dieselbe Probe (gleich viele Wuerfe in gleicher Reihenfolge, gleiches Ergebnis), dazu eine Zeile Art 3:
	 * drei Eigenschaften, ihre drei Werte, Zuschlag, drei W20 (0 = nicht mehr gewuerfelt), Ergebnis */
	signed int w[11];
	signed int i;
	signed int nr_rolls_20 = 0;

	w[0] = attrib_id_1; w[1] = attrib_id_2; w[2] = attrib_id_3;
	w[3] = hero->attrib[attrib_id_1].current + hero->attrib[attrib_id_1].mod;
	w[4] = hero->attrib[attrib_id_2].current + hero->attrib[attrib_id_2].mod;
	w[5] = hero->attrib[attrib_id_3].current + hero->attrib[attrib_id_3].mod;
	w[6] = handicap;
	w[7] = w[8] = w[9] = 0;
	for (i = 0; i < 3; i++) {
		if (((w[7 + i] = random_schick(20)) == 20) && (++nr_rolls_20 == 2)) {
			break;
		}
	}
	w[10] = (nr_rolls_20 == 2) ? -99 : (w[3] + w[4] + w[5] - (w[7] + w[8] + w[9] + handicap) + 1);
	fix_probenlog(3, hero->alias, 11, w);
	return w[10];
#else
	signed int i;
	signed int rolls_sum;
	signed int tmp;
	signed int nr_rolls_20 = 0;

	rolls_sum = 0;

#if !defined(__BORLANDC__)
	D1_INFO(" (%s/%s/%s) %+d -> ", names_attrib[attrib_id_1], names_attrib[attrib_id_2], names_attrib[attrib_id_3], handicap);
#endif

	for (i = 0; i < 3; i++) {

		tmp = random_schick(20);

#if !defined(__BORLANDC__)
		D1_INFO("%d ", tmp);
#endif

		if (tmp == 20) {
			if (++nr_rolls_20 == 2) {
#if !defined(__BORLANDC__)
				D1_INFO(" -> UNGLUECKLICH! nicht bestanden.\n");
#endif
				return -99;
			}
		}

		rolls_sum += tmp;
	}

	rolls_sum += handicap;

	tmp = hero->attrib[attrib_id_1].current + hero->attrib[attrib_id_1].mod +
		hero->attrib[attrib_id_2].current + hero->attrib[attrib_id_2].mod +
		hero->attrib[attrib_id_3].current + hero->attrib[attrib_id_3].mod;

#if !defined(__BORLANDC__)
	D1_INFO(" -> %s mit %d\n", (tmp - rolls_sum + 1) > 0 ? "bestanden" : "nicht bestanden", (tmp - rolls_sum + 1));
#endif
	return tmp - rolls_sum + 1; // in a nutshell: sum of the 3 attributes - 3*D20 - handicap + 1
#endif

#else
	/* Here, the original DSA2/3 talent test logic is implemented.
	 * WARNING: This makes talent tests, and thus the game, significantly harder!
	 * Note that we are not implementing the DSA4 rules, where tests with a positive handicap are yet harder. */
	signed int i;
	signed int tmp;
	signed int nr_rolls_1 = 0;
	signed int nr_rolls_20 = 0;
	signed int fail = 0;
	signed char attrib [3];

	attrib[0] = hero->attrib[attrib_id_1].current + hero->attrib[attrib_id_1].mod;
	attrib[1] = hero->attrib[attrib_id_2].current + hero->attrib[attrib_id_2].mod;
	attrib[2] = hero->attrib[attrib_id_3].current + hero->attrib[attrib_id_3].mod;

#if !defined(__BORLANDC__)
	D1_INFO(" (%s %d/%s %d/%s %d) ->",
			names_attrib[attrib_id_1], attrib[0],
			names_attrib[attrib_id_2], attrib[1],
			names_attrib[attrib_id_3], attrib[2]
	);
#endif

	for (i = 0; i < 3; i++) {

		tmp = random_schick(20);

#if !defined(__BORLANDC__)
		D1_INFO(" W20 = %d;", tmp);
#endif

		if (tmp == 20) {
			if (++nr_rolls_20 == 2) {
#if !defined(__BORLANDC__)
				D1_INFO(" -> UNGLUECKLICH nicht bestanden\n");
#endif
				return -99;
			}
		}

		if (tmp == 1) {
			if (++nr_rolls_1 == 2) {
#if !defined(__BORLANDC__)
				D1_INFO(" -> GLUECKLICH bestanden\n");
#endif
				return 99;
			}
		}

		if (!fail) {
			tmp -= attrib[i];
			if (handicap <= 0) {
				if (tmp > 0) {
					if (tmp > -handicap) {
						fail = 1;
#if !defined(__BORLANDC__)
						D1_INFO(" zu hoch!");
#endif
					} else  {
						handicap += tmp;
					}
				}
			}
			if (handicap > 0) {
				if (tmp > 0) {
					fail = 1;
#if !defined(__BORLANDC__)
					D1_INFO(" zu hoch!");
#endif
				} else {
					handicap += tmp;
					if (handicap < 0) {
						handicap = 0;
					}
				}
			}
		}
	}
	if (fail || (handicap > 0)) {
#if !defined(__BORLANDC__)
		D1_INFO(" -> nicht bestanden\n");
#endif
		return 0;
	} else {
#if !defined(__BORLANDC__)
		D1_INFO(" -> bestanden mit %d.\n",-handicap);
#endif
		return 1 - handicap;
	}
#endif
}

#if defined(__BORLANDC__)
/* nie aufgerufen: zufaelliger Held (Position 0..5) der aktiven Gruppe, ohne Pruefung auf Tod;
 * -1, wenn es keine Helden gibt. Vorlaeufer von get_random_hero(). */
#if !FIX_PLATZ_SEG002	/* nie aufgerufen; entfaellt mit Behebungen in diesem residenten Modul (Platz) */
signed int get_random_hero_unused(void)
{
	signed int hero_pos;

	if (!gs_total_hero_counter) {
		return -1;
	}

	do {
		hero_pos = random_schick(6) - 1;

	} while (!get_hero(hero_pos)->typus || (get_hero(hero_pos)->group_id != gs_active_group_id));

	return hero_pos;
}
#endif
#endif

/**
 * \brief   selects a hero randomly
 *
 * \return              position of a randomly choosen hero
 */
/* Original-Bug: can loop forever if the position is greater than the
	number of heroes in the group */
signed int get_random_hero(void)
{
	dsa_trace_enter(114, "seg002:get_random_hero");
#if FIX_0235
	/* Fix 235: alle 7 Plaetze wuerfeln (auch den NSC); ohne lebenden Helden der Gruppe keine Endlosschleife */
	signed int cur_hero;

	if (!count_heroes_in_group()) {
		return 0;
	}

	do {
		cur_hero = random_schick(7) - 1;

	} while (!get_hero(cur_hero)->typus || (get_hero(cur_hero)->group_id != gs_active_group_id) || get_hero(cur_hero)->flags.dead);

	return cur_hero;
#else
	signed int cur_hero;

	do {
		/* get number of current group */
		cur_hero = random_schick(gs_group_member_counts[gs_active_group_id]) - 1;

#ifdef M302de_ORIGINAL_BUGFIX
		signed int pos = 0;

		struct struct_hero *hero = get_hero(0);
		for (int i = 0; i <= 6; i++, hero++) {

			if (hero->typus == HERO_TYPE_NONE)
				continue;
			/* Check if in current group */
			if (hero->group_id != gs_active_group_id)
				continue;

			if (pos == cur_hero) {
				pos = i;
				break;
			}

			pos++;
		}
		cur_hero = pos;
#endif

	} while (!get_hero(cur_hero)->typus || (get_hero(cur_hero)->group_id != gs_active_group_id) || get_hero(cur_hero)->flags.dead);

	return cur_hero;
#endif
}

/**
 * \brief   get the money of the current group
 *
 * \return              the sum of the money of all heroes in the current group
 */
int32_t get_party_money(void)
{
	dsa_trace_enter(115, "seg002:get_party_money");
	signed int i;
	int32_t sum = 0;
	struct struct_hero *hero = get_hero(0);

	for (i = 0; i < 6; i++, hero++) {

		if (hero->typus && (hero->group_id == gs_active_group_id))
		{
			sum += hero->money;
		}
	}

	return sum;
}

/**
 * \brief   shares money between current party members
 *
 * \param   money       the money to share
 *
 *	If only a NPC is in that party, he gets all the money.
 *	If a hero is dead and in the current party, his money is set to 0.
 */
void set_party_money(int32_t money)
{
	dsa_trace_enter(116, "seg002:set_party_money");
	signed int heroes = 0;
	signed int i;
	int32_t hero_money;
	struct struct_hero *hero;

	if (money < 0)
		money = 0;

	heroes = count_heroes_in_group();

	/* set hero to NPC */
	hero = get_hero(6);

	/* if we have an NPC in current group and alive */
	if (hero->typus && (hero->group_id == gs_active_group_id) && !hero->flags.dead) {

		/* If only the NPC is in that group give him all the money */
		if (heroes > 1) {
			heroes--;
		} else {
#if FIX_0407
			/* Fix 407: der NSC allein bekommt den Betrag, nicht dazu */
			hero->money = money;
#else
			hero->money += money;
#endif
			heroes = 0;
		}
	}

	if (heroes != 0) {

		hero_money = money / heroes;
#if FIX_0407
		/* Fix 407: der Rest der Teilung geht an den ersten Helden */
		money -= hero_money * heroes;
#endif

		hero = get_hero(0);

		for (i = 0; i < 6; i++, hero++) {

			if (hero->typus && (hero->group_id == gs_active_group_id) && !hero->flags.dead) {

				/* account the money to hero */
#if FIX_0407
				hero->money = hero_money + money;
				money = 0;
#else
				hero->money = hero_money;
#endif
			} else {
				if (hero->group_id == gs_active_group_id) {
					hero->money = 0;
				}
			}
		}
	}
}

/**
 * \brief   adds money to the current group
 *
 * \param   money       money to add
 */
void add_party_money(const int32_t money)
{
	dsa_trace_enter(117, "seg002:add_party_money");
	set_party_money(get_party_money() + money);
}

/**
 * \brief   add AP to a hero
 *
 * \param   hero        pointer to the hero
 * \param   ap          AP the hero should get
 */
void add_hero_ap(struct struct_hero *hero, const int32_t ap)
{
	dsa_trace_enter(118, "seg002:add_hero_ap");
	hero->ap += ap;
}

/**
 * \brief   shares AP in the current group
 *
 * \param   ap          AP to share
 */
void add_group_ap(int32_t ap)
{
	dsa_trace_enter(119, "seg002:add_group_ap");
	signed int i;
	struct struct_hero *hero;

	if (ap < 0) {
		return;
	}

	ap = ap / count_heroes_in_group();

	hero = get_hero(0);

	for (i = 0; i <= 6; i++, hero++) {

		if (hero->typus && (hero->group_id == gs_active_group_id) && !hero->flags.dead)
		{
			add_hero_ap(hero, ap);
		}
	}
}

/**
 * \brief   add AP to every non-dead hero in the group
 *
 * \param   ap          AP to add
 */
void add_hero_ap_all(const signed int ap)
{
	dsa_trace_enter(120, "seg002:add_hero_ap_all");
	struct struct_hero *hero;
	signed int i;

	if (ap < 0)
		return;

	hero = get_hero(0);
	for (i = 0; i <= 6; i++, hero++) {

		if (hero->typus && (hero->group_id == gs_active_group_id) && !hero->flags.dead)
		{
#if !defined(__BORLANDC__)
			D1_INFO("%s erhaelt %d AP\n", hero->alias, ap);
#endif

			add_hero_ap(hero, ap);
		}
	}
}

/**
 * \brief   subtracts AP from every non-dead hero in the group
 *
 * \param   ap          AP to subtract
 */
void sub_hero_ap_all(const signed int ap)
{
	dsa_trace_enter(121, "seg002:sub_hero_ap_all");
	signed int i;
	struct struct_hero *hero;

	if (ap < 0)
		return;

	hero = get_hero(0);
	for (i = 0; i <= 6; i++, hero++) {

		if (hero->typus && (hero->group_id == gs_active_group_id) && !hero->flags.dead)
		{
			if ((uint32_t)ap <= hero->ap) {
#if !defined(__BORLANDC__)
				D1_INFO("%s erhaelt %+d AP\n", hero->alias, -ap);
#endif
				add_hero_ap(hero, -((int32_t)ap));
			} else {
#if !defined(__BORLANDC__)
				D1_INFO("%s wird auf 0 AP gesetzt\n", hero->alias);
#endif
				hero->ap = 0;
			}
		}
	}
}

/**
 * \brief   gets the position of a hero
 *
 * \param   hero        pointer to the hero
 * \return              position of the hero
 */
signed int get_hero_pos(const struct struct_hero *hero)
{
	dsa_trace_enter(122, "seg002:get_hero_pos");
	signed int i = 0;
	struct struct_hero *p;

	p = get_hero(i);
	while (hero != p) {
		i++;
		p = get_hero(i);
	}

	return i;
}

/**
 * \brief   checks if hero has the item and returns its inventory slot
 *
 * \param   hero        pointer to the hero
 * \param   item        item ID to look for
 * \return              -1 if the item is not in the inventory
 *			otherwise: first inventory slot holding item
 */
signed int inv_slot_of_item(const struct struct_hero *hero, const signed int item_id)
{
	dsa_trace_enter(123, "seg002:inv_slot_of_item");
	signed int i;

	for (i = 0; i < NR_HERO_INVENTORY_SLOTS; i++) {
		if (item_id == hero->inventory[i].item_id) {
			return i;
		}
	}

	return -1;
}

/**
 * \brief   gets the position of the first hero with an item
 *
 * \param   item_id     item ID to look for
 * \return              position of the hero or -1 if nobody of the group has this item
 */
signed int get_first_hero_with_item(const signed int item_id)
{
	dsa_trace_enter(124, "seg002:get_first_hero_with_item");
	signed int j;
	signed int i;
	struct struct_hero *hero_i = get_hero(0);

	for (i = 0; i <= 6; i++, hero_i++) {

		if ((hero_i->typus) && (hero_i->group_id == gs_active_group_id))
		{
			/* Search inventory */
			for (j = 0; j < NR_HERO_INVENTORY_SLOTS; j++) {
				if (hero_i->inventory[j].item_id == item_id) {
					return i;
				}
			}
		}
	}

	return -1;
}

/**
 * \brief   gets the position of the first hero with an item in a specified group
 *
 * \param   item_id     item ID to look for
 * \param   group_id    group number
 * \return              position of the hero or -1 if nobody in the specified group has this item
 */
signed int get_first_hero_with_item_in_group(const signed int item_id, const signed int group_id)
{
	dsa_trace_enter(125, "seg002:get_first_hero_with_item_in_group");
	signed int j;
	signed int i;
	struct struct_hero *hero_i = get_hero(0);

	for (i = 0; i <= 6; i++, hero_i++) {

		if (hero_i->typus && (hero_i->group_id == (signed char)group_id))
		{
			/* Search inventory */
			for (j = 0; j < NR_HERO_INVENTORY_SLOTS; j++) {
				if (hero_i->inventory[j].item_id == item_id) {
					return i;
				}
			}
		}
	}

	return -1;
}


/**
 * \brief   subtracts LE from every group member
 *
 * \param   le          LE to subtract
 */
void sub_group_le(const signed int le)
{
	dsa_trace_enter(126, "seg002:sub_group_le");
	signed int i;

	for (i = 0; i <= 6; i++) {

		struct struct_hero *hero_i = get_hero(i);

		if (hero_i->typus && (hero_i->group_id == gs_active_group_id))
		{
			sub_hero_le(hero_i, le);
		}
	}
}

/**
 * \brief   return a pointer to the first available hero
 *
 * \return a pointer to the first available hero. If none is available it returns a pointer to the first hero.
 */
struct struct_hero* get_first_hero_available_in_group(void)
{
	dsa_trace_enter(127, "seg002:get_first_hero_available_in_group");
	signed int i;
	struct struct_hero *hero_i = get_hero(0);

	for (i = 0; i <= 6; i++, hero_i++) {

		/* Check class, group, deadness and check_hero() */
		if (hero_i->typus && (hero_i->group_id == gs_active_group_id) &&
			!hero_i->flags.dead && check_hero(hero_i))
		{
			return hero_i;
		}
	}

	return get_hero(0);
}

/**
 * \brief   return a pointer to the second available hero in the group
 *
 * \return              a pointer to the second available hero in the group or NULL.
 */
struct struct_hero* get_second_hero_available_in_group(void)
{
	dsa_trace_enter(128, "seg002:get_second_hero_available_in_group");
	signed int i;
	signed int tmp;
	struct struct_hero *hero_i = get_hero(0);

	for (i = tmp = 0; i <= 6; i++, hero_i++) {

		/* Check class, group and check_hero() */
		if ((hero_i->typus) && (hero_i->group_id == gs_active_group_id) && check_hero(hero_i))
		{
			if (tmp) {
				return hero_i;
			}

			tmp++;
		}
	}

	return NULL;
}

/**
 * \brief   count available heroes
 *
 * \return              number of available heroes in all groups, including NPC
 */
signed int count_heroes_available(void)
{
	dsa_trace_enter(129, "seg002:count_heroes_available");
	signed int i;
	signed int retval = 0;
	struct struct_hero *hero = get_hero(0);

	for (i = 0; i <= 6; i++, hero++) {

		/* Check if hero is available */
		/* (check_hero() ist strenger als check_hero_no2(), die Bedingung ist also gleich check_hero_no2()) */
		if (hero->typus && (check_hero(hero) || check_hero_no2(hero)))
		{
			retval++;
		}
	}

	return retval;
}

#ifdef M302de_ORIGINAL_BUGFIX
/* this function allows a cleaner fix for Original-Bug 15 */
signed int count_heroes_available_ignore_npc(void)
{
	dsa_trace_enter(130, "seg002:count_heroes_available_ignore_npc");
	signed int i;
	signed int retval = 0;
	struct struct_hero *hero = get_hero(0);

	for (i = 0; i < 6; i++, hero++) {
		/* Check if hero is available */
		if (hero->typus && (check_hero(hero) || check_hero_no2(hero)))
		{
			retval++;
		}
	}

	return retval;
}
#endif

/**
 * \brief   count available (= not dead, petrified, unconscious, renegade) heroes in current group, including NPC
 *
 * \return   number of available (= not dead, petrified, unconscious, renegade) heroes in current group, including NPC
 */
signed int count_heroes_available_in_group(void)
{
	dsa_trace_enter(131, "seg002:count_heroes_available_in_group");
	signed int heroes = 0;
	signed int i;
	struct struct_hero *hero = get_hero(0);

	for (i = 0; i <= 6; i++, hero++) {
		if (hero->typus && (hero->group_id == gs_active_group_id) && check_hero_no2(hero))
		{
			/* hero not dead, petrified, unconscious or renegade */
			heroes++;
		}
	}

	return heroes;
}

#ifdef M302de_ORIGINAL_BUGFIX
/* this function allows cleaner fixes for Original-Bug 12, 13, 14 and 15 */
signed int count_heroes_available_in_group_ignore_npc(void)
{
	dsa_trace_enter(132, "seg002:count_heroes_available_in_group_ignore_npc");
	signed int heroes = 0;
	signed int i;
	struct struct_hero *hero = get_hero(0);

	for (i = 0; i < 6; i++, hero++) {

		if (hero->typus && (hero->group_id == gs_active_group_id) && check_hero_no2(hero))
		{
			/* hero not dead, petrified, unconscious or renegade */
			heroes++;
		}
	}

	return heroes;
}
#endif

void check_group(void)
/* called from only a single position, namely the petrification trap in 'Verfallene Herberge' in DNG02_handler in seg078.cpp */
{
#if FIX_0080
	/* Fix 80: die Pruefung von game_loop() anstossen, die den NSC allein nicht als handlungsfaehige
	 * Gruppe zaehlt (Spielende bzw. Gruppenwechsel beim naechsten Durchlauf) */
	g_check_party = 1;
#else

	/* Original-Bug 15:
	 * If the group steps into the petrification trap in "Verfallene Herberge" and all but the NPC get petrified
	 * (for example, Curian (MR 6) equipped with the red ring (MR +2) of Gorah's chest), the group is still active
	 * with the NPC as the only active member (which is impossible in other circumstances). If this is the only group,
	 * a click on "switch group" results in an infinite loop with the window "In dieser Gruppe ist niemand in der Lage etwas zu tun!". */

	if
#ifndef M302de_ORIGINAL_BUGFIX
		(!count_heroes_available())
#else
		(!count_heroes_available_ignore_npc())
#endif
	{
		/* game over */
		g_game_state = GAME_STATE_DEAD;

	} else if
#ifndef M302de_ORIGINAL_BUGFIX
		(!count_heroes_available_in_group())
#else
		(!count_heroes_available_in_group_ignore_npc())
#endif
	{
		GRP_switch_to_next(2);
	}
#endif
}

#if !defined(__BORLANDC__)
extern int ngen_run(int init_level, int called_from_game);
extern int ngen_run_intro(void);
static int g_run_intro = 1;
static int g_intro_explicit = 0;

extern void ngen_load_tables_from_exe(const unsigned char *exe_buf);
extern char g_fn_v302de_XXX[1];
extern char g_fn_v302de_000[9];
extern char g_fn_v302de_001[11];
extern char g_fn_v302de_002[11];
extern char g_fn_v302de_003[11];
extern char g_fn_v302de_004[11];
extern char g_fn_v302de_005[10];
extern char g_fn_v302de_006[7];
extern char g_fn_v302de_007[12];
extern char g_fn_v302de_008[10];
extern char g_fn_v302de_009[10];
extern char g_fn_v302de_010[11];
extern char g_fn_v302de_011[6];
extern char g_fn_v302de_012[6];
extern char g_fn_v302de_013[10];
extern char g_fn_v302de_014[9];
extern char g_fn_v302de_015[6];
extern char g_fn_v302de_016[8];
extern char g_fn_v302de_017[12];
extern char g_fn_v302de_018[8];
extern char g_fn_v302de_019[12];
extern char g_fn_v302de_020[9];
extern char g_fn_v302de_021[13];
extern char g_fn_v302de_022[5];
extern char g_fn_v302de_023[9];
extern char g_fn_v302de_024[6];
extern char g_fn_v302de_025[10];
extern char g_fn_v302de_026[12];
extern char g_fn_v302de_027[11];
extern char g_fn_v302de_028[11];
extern char g_fn_v302de_029[13];
extern char g_fn_v302de_030[13];
extern char g_fn_v302de_031[13];
extern char g_fn_v302de_032[11];
extern char g_fn_v302de_033[13];
extern char g_fn_v302de_034[11];
extern char g_fn_v302de_035[13];
extern char g_fn_v302de_036[11];
extern char g_fn_v302de_037[11];
extern char g_fn_v302de_038[16];
extern char g_fn_v302de_039[11];
extern char g_fn_v302de_040[13];
extern char g_fn_v302de_041[10];
extern char g_fn_v302de_042[14];
extern char g_fn_v302de_043[13];
extern char g_fn_v302de_044[13];
extern char g_fn_v302de_045[13];
extern char g_fn_v302de_046[13];
extern char g_fn_v302de_047[13];
extern char g_fn_v302de_048[12];
extern char g_fn_v302de_049[11];
extern char g_fn_v302de_050[10];
extern char g_fn_v302de_051[12];
extern char g_fn_v302de_052[8];
extern char g_fn_v302de_053[10];
extern char g_fn_v302de_054[14];
extern char g_fn_v302de_055[10];
extern char g_fn_v302de_056[13];
extern char g_fn_v302de_057[13];
extern char g_fn_v302de_058[9];
extern char g_fn_v302de_059[11];
extern char g_fn_v302de_060[9];
extern char g_fn_v302de_061[13];
extern char g_fn_v302de_062[12];
extern char g_fn_v302de_063[10];
extern char g_fn_v302de_064[9];
extern char g_fn_v302de_065[12];
extern char g_fn_v302de_066[10];
extern char g_fn_v302de_067[13];
extern char g_fn_v302de_068[13];
extern char g_fn_v302de_069[13];
extern char g_fn_v302de_070[11];
extern char g_fn_v302de_071[12];
extern char g_fn_v302de_072[13];
extern char g_fn_v302de_073[11];
extern char g_fn_v302de_074[12];
extern char g_fn_v302de_075[13];
extern char g_fn_v302de_076[13];
extern char g_fn_v302de_077[10];
extern char g_fn_v302de_078[12];
extern char g_fn_v302de_079[11];
extern char g_fn_v302de_080[11];
extern char g_fn_v302de_081[13];
extern char g_fn_v302de_082[13];
extern char g_fn_v302de_083[13];
extern char g_fn_v302de_084[11];
extern char g_fn_v302de_085[13];
extern char g_fn_v302de_086[11];
extern char g_fn_v302de_087[13];
extern char g_fn_v302de_088[11];
extern char g_fn_v302de_089[11];
extern char g_fn_v302de_090[13];
extern char g_fn_v302de_091[11];
extern char g_fn_v302de_092[13];
extern char g_fn_v302de_093[10];
extern char g_fn_v302de_094[13];
extern char g_fn_v302de_095[13];
extern char g_fn_v302de_096[13];
extern char g_fn_v302de_097[13];
extern char g_fn_v302de_098[13];
extern char g_fn_v302de_099[13];
extern char g_fn_v302de_100[12];
extern char g_fn_v302de_101[11];
extern char g_fn_v302de_102[10];
extern char g_fn_v302de_103[12];
extern char g_fn_v302de_104[8];
extern char g_fn_v302de_105[10];
extern char g_fn_v302de_106[13];
extern char g_fn_v302de_107[10];
extern char g_fn_v302de_108[13];
extern char g_fn_v302de_109[13];
extern char g_fn_v302de_110[9];
extern char g_fn_v302de_111[11];
extern char g_fn_v302de_112[9];
extern char g_fn_v302de_113[13];
extern char g_fn_v302de_114[12];
extern char g_fn_v302de_115[10];
extern char g_fn_v302de_116[9];
extern char g_fn_v302de_117[12];
extern char g_fn_v302de_118[10];
extern char g_fn_v302de_119[13];
extern char g_fn_v302de_120[13];
extern char g_fn_v302de_121[13];
extern char g_fn_v302de_122[11];
extern char g_fn_v302de_123[12];
extern char g_fn_v302de_124[13];
extern char g_fn_v302de_125[11];
extern char g_fn_v302de_126[13];
extern char g_fn_v302de_127[13];
extern char g_fn_v302de_128[13];
extern char g_fn_v302de_129[12];
extern char g_fn_v302de_130[11];
extern char g_fn_v302de_131[11];
extern char g_fn_v302de_132[12];
extern char g_fn_v302de_133[10];
extern char g_fn_v302de_134[9];
extern char g_fn_v302de_135[12];
extern char g_fn_v302de_136[13];
extern char g_fn_v302de_137[10];
extern char g_fn_v302de_138[11];
extern char g_fn_v302de_139[11];
extern char g_fn_v302de_140[11];
extern char g_fn_v302de_141[12];
extern char g_fn_v302de_142[12];
extern char g_fn_v302de_143[12];
extern char g_fn_v302de_144[11];
extern char g_fn_v302de_145[10];
extern char g_fn_v302de_146[10];
extern char g_fn_v302de_147[8];
extern char g_fn_v302de_148[11];
extern char g_fn_v302de_149[9];
extern char g_fn_v302de_150[12];
extern char g_fn_v302de_151[10];
extern char g_fn_v302de_152[6];
extern char g_fn_v302de_153[10];
extern char g_fn_v302de_154[1];
extern char g_fn_v302de_155[1];
extern char g_fn_v302de_156[9];
extern char g_fn_v302de_157[12];
extern char g_fn_v302de_158[1];
extern char g_fn_v302de_159[13];
extern char g_fn_v302de_160[13];
extern char g_fn_v302de_161[13];
extern char g_fn_v302de_162[10];
extern char g_fn_v302de_163[10];
extern char g_fn_v302de_164[10];
extern char g_fn_v302de_165[10];
extern char g_fn_v302de_166[10];
extern char g_fn_v302de_167[10];
extern char g_fn_v302de_168[10];
extern char g_fn_v302de_169[12];
extern char g_fn_v302de_170[12];
extern char g_fn_v302de_171[12];
extern char g_fn_v302de_172[12];
extern char g_fn_v302de_173[9];
extern char g_fn_v302de_174[11];
extern char g_fn_v302de_175[13];
extern char g_fn_v302de_176[12];
extern char g_fn_v302de_177[9];
extern char g_fn_v302de_178[13];
extern char g_fn_v302de_179[13];
extern char g_fn_v302de_180[9];
extern char g_fn_v302de_181[11];
extern char g_fn_v302de_182[12];
extern char g_fn_v302de_183[13];
extern char g_fn_v302de_184[12];
extern char g_fn_v302de_185[13];
extern char g_fn_v302de_186[11];
extern char g_fn_v302de_187[11];
extern char g_fn_v302de_188[11];
extern char g_fn_v302de_189[11];
extern char g_fn_v302de_190[12];
extern char g_fn_v302de_191[13];
extern char g_fn_v302de_192[13];
extern char g_fn_v302de_193[13];
extern char g_fn_v302de_194[13];
extern char g_fn_v302de_195[13];
extern char g_fn_v302de_196[13];
extern char g_fn_v302de_197[13];
extern char g_fn_v302de_198[13];
extern char g_fn_v302de_199[13];
extern char g_fn_v302de_200[13];
extern char g_fn_v302de_201[12];
extern char g_fn_v302de_202[12];
extern char g_fn_v302de_203[12];
extern char g_fn_v302de_204[12];
extern char g_fn_v302de_205[10];
extern char g_fn_v302de_206[11];
extern char g_fn_v302de_207[10];
extern char g_fn_v302de_208[9];
extern char g_fn_v302de_209[9];
extern char g_fn_v302de_210[13];
extern char g_fn_v302de_211[13];
extern char g_fn_v302de_212[12];
extern char g_fn_v302de_213[11];
extern char g_fn_v302de_214[9];
extern char g_fn_v302de_215[9];
extern char g_fn_v302de_216[8];
extern char g_fn_v302de_217[12];
extern char g_fn_v302de_218[12];
extern char g_fn_v302de_219[12];
extern char g_fn_v302de_220[10];
extern char g_fn_v302de_221[13];
extern char g_fn_v302de_222[13];
extern char g_fn_v302de_223[11];
extern char g_fn_v302de_224[12];
extern char g_fn_v302de_225[8];
extern char g_fn_v302de_226[12];
extern char g_fn_v302de_227[11];
extern char g_fn_v302de_228[11];
extern char g_fn_v302de_229[11];
extern char g_fn_v302de_230[12];
extern char g_fn_v302de_231[9];
extern char g_fn_v302de_232[11];
extern char g_fn_v302de_233[10];
extern char g_fn_v302de_234[6];
extern char g_fn_v302de_235[10];
extern char g_fn_v302de_236[13];
extern char g_fn_v302de_237[13];
extern char g_fn_v302de_238[10];
extern char g_fn_v302de_239[10];
extern char g_fn_v302de_240[9];
extern char g_fn_v302de_241[13];
extern char g_fn_v302de_242[9];
extern char g_fn_v302de_243[9];
extern char g_fn_v302de_244[9];
extern char g_fn_v302de_245[9];
extern char g_fn_v302de_246[9];
extern char g_fn_v302de_247[9];
extern char g_fn_v302de_248[9];
extern char g_fn_v302de_249[9];
extern char g_fn_v302de_250[9];
extern char g_fn_v302de_251[9];
extern char g_fn_v302de_252[9];
extern char g_fn_v302de_253[9];
extern char g_fn_v302de_254[9];
extern char g_fn_v302de_255[12];
extern char g_fn_v302de_256[10];
extern char g_fn_v302de_257[9];
extern char g_fn_v302de_258[9];
extern char g_fn_v302de_259[9];
extern char g_fn_v302de_260[9];
extern char g_fn_v302de_261[9];
extern char g_fn_v302de_262[9];
extern char g_fn_v302de_263[9];
extern char g_fn_v302de_264[9];
extern char g_fn_v302de_265[9];
extern char g_fn_v302de_266[9];
extern char g_fn_v302de_267[9];
extern char g_fn_v302de_268[9];
extern char g_fn_v302de_269[9];
extern char g_fn_v302de_270[12];
extern char g_fn_v302de_271[10];
extern char g_fn_v302de_272[9];
extern char g_fn_v302de_273[9];
extern char g_fn_v302de_274[9];
extern char g_fn_v302de_275[9];
extern char g_fn_v302de_276[9];
extern char g_fn_v302de_277[9];
extern char g_fn_v302de_278[9];
extern char g_fn_v302de_279[9];
extern char g_fn_v302de_280[9];
extern char g_fn_v302de_281[9];
extern char g_fn_v302de_282[9];
extern char g_fn_v302de_283[9];
extern char g_fn_v302de_284[9];
extern char g_fn_v302de_285[12];
extern char g_fn_v302de_286[10];
extern char g_fn_v302de_287[12];
extern char g_fn_v302de_288[10];
extern char g_fn_v302de_289[8];
extern char g_fn_v302de_290[8];
extern char g_fn_v302de_291[8];
extern char g_fn_v302de_292[8];
extern char g_fn_v302de_293[8];
extern char g_fn_v302de_294[8];
extern char g_fn_v302de_295[8];
extern char g_fn_v302de_296[8];
extern char g_fn_v302de_297[8];
extern char g_fn_v302de_298[9];
extern char g_fn_v302de_299[9];
extern char g_fn_v302de_300[9];
extern char g_fn_v302de_301[9];
extern char g_fn_v302de_302[9];
extern char g_fn_v302de_303[9];
extern char g_fn_v302de_304[9];
extern char g_fn_v302de_305[9];
extern char g_fn_v302de_306[9];
extern char g_fn_v302de_307[10];
extern unsigned char g_unkn_005[38];
extern short g_gfx_ani_descriptions_01[65];
extern short g_gfx_ani_descriptions_02[65];
extern short g_gfx_ani_descriptions_03[65];
extern short g_gfx_ani_descriptions_04[65];
extern short g_gfx_ani_descriptions_05[65];
extern short g_gfx_ani_descriptions_06[65];
extern short g_gfx_ani_descriptions_07[65];
extern short g_gfx_ani_descriptions_08[65];
extern short g_gfx_ani_descriptions_09[65];
extern short g_gfx_ani_descriptions_10[65];
extern short g_gfx_ani_descriptions_11[65];
extern short g_gfx_ani_descriptions_12[65];
extern short g_gfx_ani_descriptions_13[65];
extern short g_gfx_ani_descriptions_14[37];
extern short g_gfx_ani_descriptions_15[37];
extern short g_gfx_ani_descriptions_16[37];
extern short g_gfx_ani_descriptions_17[37];
extern short g_gfx_ani_descriptions_18[37];
extern short g_gfx_ani_descriptions_19[37];
extern short g_gfx_ani_descriptions_20[37];
extern short g_gfx_ani_descriptions_21[37];
extern short g_gfx_ani_descriptions_22[37];
extern short g_gfx_ani_descriptions_23[37];
extern short g_gfx_ani_descriptions_24[37];
extern short g_gfx_ani_descriptions_25[37];
extern short g_gfx_ani_descriptions_26[37];
extern short g_gfx_ani_descriptions_27[37];
extern short g_gfx_ani_descriptions_28[37];
extern int8_t g_wallclock_pos_y[81];
extern struct struct_informer_tab g_informer_tab[15];
extern int8_t g_anitab_projectile_data[24][7];
extern uint8_t g_statuspage_palette[32][3];
extern struct struct_point g_invslot_iconxy_table[23];
extern struct struct_point g_invslot_borderxy_table[25];
extern struct gather_herbs g_gather_herbs_table[13];
extern struct healer_descr g_healer_descr_table[41];
extern struct inn_descr g_inn_descr_table[77];
extern struct smith_descr g_smith_descr_table[42];
extern struct struct_point g_visual_square_offsets_std[29];
extern struct struct_point g_visual_square_offsets_sign[29];
extern struct struct_point g_visual_square_offsets_inn[29];
extern struct struct_point g_visual_square_offsets_grass[29];
extern unsigned char g_tex_descr_table[101][18];
extern struct struct_point g_dng_square_pos[22];
extern struct struct_point g_dng_square_pos_stairs[22];
extern struct struct_point g_dng_square_pos_pit[22];
extern struct struct_point g_dng_square_pos_pit_ceil[22];
extern int8_t g_signposts_linked_lroute_001[2];
extern int8_t g_signposts_linked_lroute_002[2];
extern int8_t g_signposts_linked_lroute_003[2];
extern int8_t g_signposts_linked_lroute_004[2];
extern int8_t g_signposts_linked_lroute_005[3];
extern int8_t g_signposts_linked_lroute_006[2];
extern int8_t g_signposts_linked_lroute_007[2];
extern int8_t g_signposts_linked_lroute_008[2];
extern int8_t g_signposts_linked_lroute_009[2];
extern int8_t g_signposts_linked_lroute_010[2];
extern int8_t g_signposts_linked_lroute_011[2];
extern int8_t g_signposts_linked_lroute_012[2];
extern int8_t g_signposts_linked_lroute_013[2];
extern int8_t g_signposts_linked_lroute_014[2];
extern int8_t g_signposts_linked_lroute_015[2];
extern int8_t g_signposts_linked_lroute_016[2];
extern int8_t g_signposts_linked_lroute_017[2];
extern int8_t g_signposts_linked_lroute_018[2];
extern int8_t g_signposts_linked_lroute_019[2];
extern int8_t g_signposts_linked_lroute_020[2];
extern int8_t g_signposts_linked_lroute_021[2];
extern int8_t g_signposts_linked_lroute_022[2];
extern int8_t g_signposts_linked_lroute_023[2];
extern int8_t g_signposts_linked_lroute_024[2];
extern int8_t g_signposts_linked_lroute_025[2];
extern int8_t g_signposts_linked_lroute_026[2];
extern int8_t g_signposts_linked_lroute_027[2];
extern int8_t g_signposts_linked_lroute_028[2];
extern int8_t g_signposts_linked_lroute_029[2];
extern int8_t g_signposts_linked_lroute_030[2];
extern int8_t g_signposts_linked_lroute_031[2];
extern int8_t g_signposts_linked_lroute_032[2];
extern int8_t g_signposts_linked_lroute_033[2];
extern int8_t g_signposts_linked_lroute_034[2];
extern int8_t g_signposts_linked_lroute_035[2];
extern int8_t g_signposts_linked_lroute_036[2];
extern int8_t g_signposts_linked_lroute_037[2];
extern int8_t g_signposts_linked_lroute_038[2];
extern int8_t g_signposts_linked_lroute_039[2];
extern int8_t g_signposts_linked_lroute_040[2];
extern int8_t g_signposts_linked_lroute_041[2];
extern int8_t g_signposts_linked_lroute_042[2];
extern int8_t g_signposts_linked_lroute_043[2];
extern int8_t g_signposts_linked_lroute_044[2];
extern int8_t g_signposts_linked_lroute_045[2];
extern int8_t g_signposts_linked_lroute_046[2];
extern int8_t g_signposts_linked_lroute_047[2];
extern int8_t g_signposts_linked_lroute_048[6];
extern int8_t g_signposts_linked_lroute_049[2];
extern int8_t g_signposts_linked_lroute_050[2];
extern int8_t g_signposts_linked_lroute_051[2];
extern int8_t g_signposts_linked_lroute_052[2];
extern int8_t g_signposts_linked_lroute_053[2];
extern int8_t g_signposts_linked_lroute_054[2];
extern int8_t g_signposts_linked_lroute_055[2];
extern int8_t g_signposts_linked_lroute_056[2];
extern int8_t g_signposts_linked_lroute_057[2];
extern int8_t g_signposts_linked_lroute_058[3];
extern int8_t g_signposts_linked_lroute_059[2];
extern int8_t g_signposts_linked_lroute_060[2];
extern int8_t g_signposts_linked_lroute_061[3];
extern int8_t g_signposts_linked_lroute_062[3];
extern int8_t g_signposts_linked_lroute_063[2];
extern int8_t g_signposts_linked_lroute_064[3];
extern int8_t g_signposts_linked_lroute_065[2];
extern int8_t g_signposts_linked_lroute_066[2];
extern int8_t g_signposts_linked_lroute_067[2];
extern int8_t g_signposts_linked_lroute_068[2];
extern int8_t g_signposts_linked_lroute_069[2];
extern int8_t g_signposts_linked_lroute_070[2];
extern int8_t g_signposts_linked_lroute_071[2];
extern int8_t g_signposts_linked_lroute_072[2];
extern int8_t g_signposts_linked_lroute_073[2];
extern int8_t g_signposts_linked_lroute_074[2];
extern int8_t g_signposts_linked_lroute_075[2];
extern int8_t g_signposts_linked_lroute_076[2];
extern int8_t g_signposts_linked_lroute_077[2];
extern int8_t g_signposts_linked_lroute_078[2];
extern int8_t g_signposts_linked_lroute_079[2];
extern int8_t g_signposts_linked_lroute_080[2];
extern int8_t g_signposts_linked_lroute_081[2];
extern int8_t g_signposts_linked_lroute_082[2];
extern int8_t g_signposts_linked_lroute_083[2];
extern int8_t g_signposts_linked_lroute_084[2];
extern int8_t g_signposts_linked_lroute_085[2];
extern int8_t g_signposts_linked_lroute_086[2];
extern int8_t g_signposts_linked_lroute_087[2];
extern int8_t g_signposts_linked_lroute_088[2];
extern int8_t g_signposts_linked_lroute_089[2];
extern int8_t g_signposts_linked_lroute_090[2];
extern int8_t g_signposts_linked_lroute_091[2];
extern int8_t g_signposts_linked_lroute_092[2];
extern int8_t g_signposts_linked_lroute_093[2];
extern int8_t g_signposts_linked_lroute_094[2];
extern int8_t g_signposts_linked_lroute_095[2];
extern int8_t g_signposts_linked_lroute_096[2];
extern int8_t g_signposts_linked_lroute_097[2];
extern int8_t g_signposts_linked_lroute_098[2];
extern int8_t g_signposts_linked_lroute_099[2];
extern int8_t g_signposts_linked_lroute_100[2];
extern int8_t g_signposts_linked_lroute_101[2];
extern int8_t g_signposts_linked_lroute_102[2];
extern int8_t g_signposts_linked_lroute_103[2];
extern int8_t g_signposts_linked_lroute_104[2];
extern int8_t g_signposts_linked_lroute_105[2];
extern int8_t g_harbors_linked_sea_routes_01[6];
extern int8_t g_harbors_linked_sea_routes_02[3];
extern int8_t g_harbors_linked_sea_routes_03[4];
extern int8_t g_harbors_linked_sea_routes_04[4];
extern int8_t g_harbors_linked_sea_routes_05[4];
extern int8_t g_harbors_linked_sea_routes_06[4];
extern int8_t g_harbors_linked_sea_routes_07[2];
extern int8_t g_harbors_linked_sea_routes_08[8];
extern int8_t g_harbors_linked_sea_routes_09[5];
extern int8_t g_harbors_linked_sea_routes_10[7];
extern int8_t g_harbors_linked_sea_routes_11[3];
extern int8_t g_harbors_linked_sea_routes_12[5];
extern int8_t g_harbors_linked_sea_routes_13[4];
extern int8_t g_harbors_linked_sea_routes_14[3];
extern int8_t g_harbors_linked_sea_routes_15[7];
extern int8_t g_harbors_linked_sea_routes_16[6];
extern int8_t g_harbors_linked_sea_routes_17[4];
extern int8_t g_harbors_linked_sea_routes_18[9];
extern int8_t g_harbors_linked_sea_routes_19[3];
extern int8_t g_harbors_linked_sea_routes_20[5];
extern int8_t g_harbors_linked_sea_routes_21[3];
extern int8_t g_harbors_linked_sea_routes_22[4];
extern int8_t g_harbors_linked_sea_routes_23[3];
extern int8_t g_harbors_linked_sea_routes_24[6];
extern int8_t g_harbors_linked_sea_routes_25[3];
extern struct struct_char_width g_gui_char_width[75];
extern struct struct_char_height g_gui_char_width_font8[67];
extern struct analues_item g_analues_items[28];
extern struct struct_recipe g_alchemy_recipes[13];
extern uint8_t g_trv_journey_random_encounter_stats[14][7];
extern uint8_t g_palette_unknown4[32][3];
extern uint8_t g_palette_general[32][3];

static void schick_md5_hex(const uint8_t *msg, uint32_t len, char out_hex[33])
{
	static const uint8_t s[64] = {
		7,12,17,22, 7,12,17,22, 7,12,17,22, 7,12,17,22,
		5, 9,14,20, 5, 9,14,20, 5, 9,14,20, 5, 9,14,20,
		4,11,16,23, 4,11,16,23, 4,11,16,23, 4,11,16,23,
		6,10,15,21, 6,10,15,21, 6,10,15,21, 6,10,15,21
	};
	static const uint32_t K[64] = {
		0xd76aa478u,0xe8c7b756u,0x242070dbu,0xc1bdceeeu,0xf57c0fafu,0x4787c62au,0xa8304613u,0xfd469501u,
		0x698098d8u,0x8b44f7afu,0xffff5bb1u,0x895cd7beu,0x6b901122u,0xfd987193u,0xa679438eu,0x49b40821u,
		0xf61e2562u,0xc040b340u,0x265e5a51u,0xe9b6c7aau,0xd62f105du,0x02441453u,0xd8a1e681u,0xe7d3fbc8u,
		0x21e1cde6u,0xc33707d6u,0xf4d50d87u,0x455a14edu,0xa9e3e905u,0xfcefa3f8u,0x676f02d9u,0x8d2a4c8au,
		0xfffa3942u,0x8771f681u,0x6d9d6122u,0xfde5380cu,0xa4beea44u,0x4bdecfa9u,0xf6bb4b60u,0xbebfbc70u,
		0x289b7ec6u,0xeaa127fau,0xd4ef3085u,0x04881d05u,0xd9d4d039u,0xe6db99e5u,0x1fa27cf8u,0xc4ac5665u,
		0xf4292244u,0x432aff97u,0xab9423a7u,0xfc93a039u,0x655b59c3u,0x8f0ccc92u,0xffeff47du,0x85845dd1u,
		0x6fa87e4fu,0xfe2ce6e0u,0xa3014314u,0x4e0811a1u,0xf7537e82u,0xbd3af235u,0x2ad7d2bbu,0xeb86d391u
	};
	uint32_t a0 = 0x67452301u, b0 = 0xefcdab89u, c0 = 0x98badcfeu, d0 = 0x10325476u;
	uint32_t new_len = ((len + 8u) / 64u + 1u) * 64u;
	uint8_t *buf = (uint8_t*)calloc(new_len, 1);
	uint64_t bits_len = (uint64_t)len * 8u;
	static const char hex[] = "0123456789abcdef";
	if (!buf) { out_hex[0] = '\0'; return; }
	memcpy(buf, msg, len);
	buf[len] = 0x80;
	for (int b = 0; b < 8; ++b) buf[new_len - 8 + b] = (uint8_t)((bits_len >> (b * 8)) & 0xffu);
	for (uint32_t off = 0; off < new_len; off += 64u) {
		uint32_t M[16];
		for (int j = 0; j < 16; ++j) {
			const uint8_t *p = buf + off + j * 4;
			M[j] = (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
		}
		uint32_t A = a0, B = b0, C = c0, D = d0;
		for (uint32_t i = 0; i < 64; ++i) {
			uint32_t F, g;
			if (i < 16)      { F = (B & C) | ((~B) & D); g = i; }
			else if (i < 32) { F = (D & B) | ((~D) & C); g = (5u * i + 1u) % 16u; }
			else if (i < 48) { F = B ^ C ^ D;            g = (3u * i + 5u) % 16u; }
			else             { F = C ^ (B | (~D));       g = (7u * i) % 16u; }
			F = F + A + K[i] + M[g];
			A = D; D = C; C = B;
			B = B + ((F << s[i]) | (F >> (32u - s[i])));
		}
		a0 += A; b0 += B; c0 += C; d0 += D;
	}
	free(buf);
	uint32_t st[4] = { a0, b0, c0, d0 };
	for (int w = 0; w < 4; ++w) {
		for (int b = 0; b < 4; ++b) {
			uint8_t v = (uint8_t)((st[w] >> (b * 8)) & 0xffu);
			out_hex[(w * 4 + b) * 2 + 0] = hex[v >> 4];
			out_hex[(w * 4 + b) * 2 + 1] = hex[v & 0x0f];
		}
	}
	out_hex[32] = '\0';
}

static void schick_load_tables_from_exe(int explicit_gamedir)
{
	char exe_path[512];
	const char *resolved = dsa_platform_resolve_path("SCHICKM.EXE", exe_path, sizeof(exe_path));
	int fd = _open(resolved, O_BINARY | O_RDONLY);
	if (fd == -1 && !explicit_gamedir) {
		fd = _open("SCHICKM.EXE", O_BINARY | O_RDONLY);
	}
	if (fd == -1) {
		fprintf(stderr,
			"FEHLER: Originaldatei SCHICKM.EXE nicht gefunden (%s) - deutsche GOG-Fassung benoetigt (V3.02de, 556.208 Bytes, MD5 4ce035f95ead01a8b2553b4487544e5d).\n",
			resolved ? resolved : "SCHICKM.EXE");
		fflush(stderr);
		exit(2);
	}
	uint8_t *exe_buf = (uint8_t*)malloc(556208 + 1);
	if (!exe_buf) {
		close(fd);
		fprintf(stderr, "FEHLER: Speicherreservierung fuer SCHICKM.EXE fehlgeschlagen - deutsche GOG-Fassung benoetigt.\n");
		fflush(stderr);
		exit(2);
	}
	int total = 0;
	while (total < 556209) {
		int r = _read(fd, exe_buf + total, 556209 - total);
		if (r <= 0) break;
		total += r;
	}
	close(fd);
	if (total != 556208) {
		free(exe_buf);
		fprintf(stderr,
			"FEHLER: Ungueltige Dateigroesse von SCHICKM.EXE (%d Bytes statt 556.208 Bytes) - deutsche GOG-Fassung benoetigt (V3.02de, MD5 4ce035f95ead01a8b2553b4487544e5d).\n",
			total);
		fflush(stderr);
		exit(2);
	}
	char md5_hex[33];
	schick_md5_hex(exe_buf, 556208u, md5_hex);
	if (strcmp(md5_hex, "4ce035f95ead01a8b2553b4487544e5d") != 0) {
		free(exe_buf);
		fprintf(stderr,
			"FEHLER: Ungueltige MD5-Pruefsumme von SCHICKM.EXE (%s statt 4ce035f95ead01a8b2553b4487544e5d) - deutsche GOG-Fassung benoetigt (V3.02de, 556.208 Bytes).\n",
			md5_hex);
		fflush(stderr);
		exit(2);
	}
	STATIC_ASSERT(sizeof(g_fn_v302de_XXX) <= 1, size_check_g_fn_v302de_XXX);
	memcpy(&g_fn_v302de_XXX, exe_buf + 0x01c526, sizeof(g_fn_v302de_XXX));
	STATIC_ASSERT(sizeof(g_fn_v302de_000) <= 9, size_check_g_fn_v302de_000);
	memcpy(&g_fn_v302de_000, exe_buf + 0x01c527, sizeof(g_fn_v302de_000));
	STATIC_ASSERT(sizeof(g_fn_v302de_001) <= 11, size_check_g_fn_v302de_001);
	memcpy(&g_fn_v302de_001, exe_buf + 0x01c530, sizeof(g_fn_v302de_001));
	STATIC_ASSERT(sizeof(g_fn_v302de_002) <= 11, size_check_g_fn_v302de_002);
	memcpy(&g_fn_v302de_002, exe_buf + 0x01c53b, sizeof(g_fn_v302de_002));
	STATIC_ASSERT(sizeof(g_fn_v302de_003) <= 11, size_check_g_fn_v302de_003);
	memcpy(&g_fn_v302de_003, exe_buf + 0x01c546, sizeof(g_fn_v302de_003));
	STATIC_ASSERT(sizeof(g_fn_v302de_004) <= 11, size_check_g_fn_v302de_004);
	memcpy(&g_fn_v302de_004, exe_buf + 0x01c551, sizeof(g_fn_v302de_004));
	STATIC_ASSERT(sizeof(g_fn_v302de_005) <= 10, size_check_g_fn_v302de_005);
	memcpy(&g_fn_v302de_005, exe_buf + 0x01c55c, sizeof(g_fn_v302de_005));
	STATIC_ASSERT(sizeof(g_fn_v302de_006) <= 7, size_check_g_fn_v302de_006);
	memcpy(&g_fn_v302de_006, exe_buf + 0x01c566, sizeof(g_fn_v302de_006));
	STATIC_ASSERT(sizeof(g_fn_v302de_007) <= 12, size_check_g_fn_v302de_007);
	memcpy(&g_fn_v302de_007, exe_buf + 0x01c56d, sizeof(g_fn_v302de_007));
	STATIC_ASSERT(sizeof(g_fn_v302de_008) <= 10, size_check_g_fn_v302de_008);
	memcpy(&g_fn_v302de_008, exe_buf + 0x01c579, sizeof(g_fn_v302de_008));
	STATIC_ASSERT(sizeof(g_fn_v302de_009) <= 10, size_check_g_fn_v302de_009);
	memcpy(&g_fn_v302de_009, exe_buf + 0x01c583, sizeof(g_fn_v302de_009));
	STATIC_ASSERT(sizeof(g_fn_v302de_010) <= 11, size_check_g_fn_v302de_010);
	memcpy(&g_fn_v302de_010, exe_buf + 0x01c58d, sizeof(g_fn_v302de_010));
	STATIC_ASSERT(sizeof(g_fn_v302de_011) <= 6, size_check_g_fn_v302de_011);
	memcpy(&g_fn_v302de_011, exe_buf + 0x01c598, sizeof(g_fn_v302de_011));
	STATIC_ASSERT(sizeof(g_fn_v302de_012) <= 6, size_check_g_fn_v302de_012);
	memcpy(&g_fn_v302de_012, exe_buf + 0x01c59e, sizeof(g_fn_v302de_012));
	STATIC_ASSERT(sizeof(g_fn_v302de_013) <= 10, size_check_g_fn_v302de_013);
	memcpy(&g_fn_v302de_013, exe_buf + 0x01c5a4, sizeof(g_fn_v302de_013));
	STATIC_ASSERT(sizeof(g_fn_v302de_014) <= 9, size_check_g_fn_v302de_014);
	memcpy(&g_fn_v302de_014, exe_buf + 0x01c5ae, sizeof(g_fn_v302de_014));
	STATIC_ASSERT(sizeof(g_fn_v302de_015) <= 6, size_check_g_fn_v302de_015);
	memcpy(&g_fn_v302de_015, exe_buf + 0x01c5b7, sizeof(g_fn_v302de_015));
	STATIC_ASSERT(sizeof(g_fn_v302de_016) <= 8, size_check_g_fn_v302de_016);
	memcpy(&g_fn_v302de_016, exe_buf + 0x01c5bd, sizeof(g_fn_v302de_016));
	STATIC_ASSERT(sizeof(g_fn_v302de_017) <= 12, size_check_g_fn_v302de_017);
	memcpy(&g_fn_v302de_017, exe_buf + 0x01c5c5, sizeof(g_fn_v302de_017));
	STATIC_ASSERT(sizeof(g_fn_v302de_018) <= 8, size_check_g_fn_v302de_018);
	memcpy(&g_fn_v302de_018, exe_buf + 0x01c5d1, sizeof(g_fn_v302de_018));
	STATIC_ASSERT(sizeof(g_fn_v302de_019) <= 12, size_check_g_fn_v302de_019);
	memcpy(&g_fn_v302de_019, exe_buf + 0x01c5d9, sizeof(g_fn_v302de_019));
	STATIC_ASSERT(sizeof(g_fn_v302de_020) <= 9, size_check_g_fn_v302de_020);
	memcpy(&g_fn_v302de_020, exe_buf + 0x01c5e5, sizeof(g_fn_v302de_020));
	STATIC_ASSERT(sizeof(g_fn_v302de_021) <= 13, size_check_g_fn_v302de_021);
	memcpy(&g_fn_v302de_021, exe_buf + 0x01c5ee, sizeof(g_fn_v302de_021));
	STATIC_ASSERT(sizeof(g_fn_v302de_022) <= 5, size_check_g_fn_v302de_022);
	memcpy(&g_fn_v302de_022, exe_buf + 0x01c5fb, sizeof(g_fn_v302de_022));
	STATIC_ASSERT(sizeof(g_fn_v302de_023) <= 9, size_check_g_fn_v302de_023);
	memcpy(&g_fn_v302de_023, exe_buf + 0x01c600, sizeof(g_fn_v302de_023));
	STATIC_ASSERT(sizeof(g_fn_v302de_024) <= 6, size_check_g_fn_v302de_024);
	memcpy(&g_fn_v302de_024, exe_buf + 0x01c609, sizeof(g_fn_v302de_024));
	STATIC_ASSERT(sizeof(g_fn_v302de_025) <= 10, size_check_g_fn_v302de_025);
	memcpy(&g_fn_v302de_025, exe_buf + 0x01c60f, sizeof(g_fn_v302de_025));
	STATIC_ASSERT(sizeof(g_fn_v302de_026) <= 12, size_check_g_fn_v302de_026);
	memcpy(&g_fn_v302de_026, exe_buf + 0x01c619, sizeof(g_fn_v302de_026));
	STATIC_ASSERT(sizeof(g_fn_v302de_027) <= 11, size_check_g_fn_v302de_027);
	memcpy(&g_fn_v302de_027, exe_buf + 0x01c625, sizeof(g_fn_v302de_027));
	STATIC_ASSERT(sizeof(g_fn_v302de_028) <= 11, size_check_g_fn_v302de_028);
	memcpy(&g_fn_v302de_028, exe_buf + 0x01c630, sizeof(g_fn_v302de_028));
	STATIC_ASSERT(sizeof(g_fn_v302de_029) <= 13, size_check_g_fn_v302de_029);
	memcpy(&g_fn_v302de_029, exe_buf + 0x01c63b, sizeof(g_fn_v302de_029));
	STATIC_ASSERT(sizeof(g_fn_v302de_030) <= 13, size_check_g_fn_v302de_030);
	memcpy(&g_fn_v302de_030, exe_buf + 0x01c648, sizeof(g_fn_v302de_030));
	STATIC_ASSERT(sizeof(g_fn_v302de_031) <= 13, size_check_g_fn_v302de_031);
	memcpy(&g_fn_v302de_031, exe_buf + 0x01c655, sizeof(g_fn_v302de_031));
	STATIC_ASSERT(sizeof(g_fn_v302de_032) <= 11, size_check_g_fn_v302de_032);
	memcpy(&g_fn_v302de_032, exe_buf + 0x01c662, sizeof(g_fn_v302de_032));
	STATIC_ASSERT(sizeof(g_fn_v302de_033) <= 13, size_check_g_fn_v302de_033);
	memcpy(&g_fn_v302de_033, exe_buf + 0x01c66d, sizeof(g_fn_v302de_033));
	STATIC_ASSERT(sizeof(g_fn_v302de_034) <= 11, size_check_g_fn_v302de_034);
	memcpy(&g_fn_v302de_034, exe_buf + 0x01c67a, sizeof(g_fn_v302de_034));
	STATIC_ASSERT(sizeof(g_fn_v302de_035) <= 13, size_check_g_fn_v302de_035);
	memcpy(&g_fn_v302de_035, exe_buf + 0x01c685, sizeof(g_fn_v302de_035));
	STATIC_ASSERT(sizeof(g_fn_v302de_036) <= 11, size_check_g_fn_v302de_036);
	memcpy(&g_fn_v302de_036, exe_buf + 0x01c692, sizeof(g_fn_v302de_036));
	STATIC_ASSERT(sizeof(g_fn_v302de_037) <= 11, size_check_g_fn_v302de_037);
	memcpy(&g_fn_v302de_037, exe_buf + 0x01c69d, sizeof(g_fn_v302de_037));
	STATIC_ASSERT(sizeof(g_fn_v302de_038) <= 16, size_check_g_fn_v302de_038);
	memcpy(&g_fn_v302de_038, exe_buf + 0x01c6a8, sizeof(g_fn_v302de_038));
	STATIC_ASSERT(sizeof(g_fn_v302de_039) <= 11, size_check_g_fn_v302de_039);
	memcpy(&g_fn_v302de_039, exe_buf + 0x01c6b8, sizeof(g_fn_v302de_039));
	STATIC_ASSERT(sizeof(g_fn_v302de_040) <= 13, size_check_g_fn_v302de_040);
	memcpy(&g_fn_v302de_040, exe_buf + 0x01c6c3, sizeof(g_fn_v302de_040));
	STATIC_ASSERT(sizeof(g_fn_v302de_041) <= 10, size_check_g_fn_v302de_041);
	memcpy(&g_fn_v302de_041, exe_buf + 0x01c6d0, sizeof(g_fn_v302de_041));
	STATIC_ASSERT(sizeof(g_fn_v302de_042) <= 14, size_check_g_fn_v302de_042);
	memcpy(&g_fn_v302de_042, exe_buf + 0x01c6da, sizeof(g_fn_v302de_042));
	STATIC_ASSERT(sizeof(g_fn_v302de_043) <= 13, size_check_g_fn_v302de_043);
	memcpy(&g_fn_v302de_043, exe_buf + 0x01c6e8, sizeof(g_fn_v302de_043));
	STATIC_ASSERT(sizeof(g_fn_v302de_044) <= 13, size_check_g_fn_v302de_044);
	memcpy(&g_fn_v302de_044, exe_buf + 0x01c6f5, sizeof(g_fn_v302de_044));
	STATIC_ASSERT(sizeof(g_fn_v302de_045) <= 13, size_check_g_fn_v302de_045);
	memcpy(&g_fn_v302de_045, exe_buf + 0x01c702, sizeof(g_fn_v302de_045));
	STATIC_ASSERT(sizeof(g_fn_v302de_046) <= 13, size_check_g_fn_v302de_046);
	memcpy(&g_fn_v302de_046, exe_buf + 0x01c70f, sizeof(g_fn_v302de_046));
	STATIC_ASSERT(sizeof(g_fn_v302de_047) <= 13, size_check_g_fn_v302de_047);
	memcpy(&g_fn_v302de_047, exe_buf + 0x01c71c, sizeof(g_fn_v302de_047));
	STATIC_ASSERT(sizeof(g_fn_v302de_048) <= 12, size_check_g_fn_v302de_048);
	memcpy(&g_fn_v302de_048, exe_buf + 0x01c729, sizeof(g_fn_v302de_048));
	STATIC_ASSERT(sizeof(g_fn_v302de_049) <= 11, size_check_g_fn_v302de_049);
	memcpy(&g_fn_v302de_049, exe_buf + 0x01c735, sizeof(g_fn_v302de_049));
	STATIC_ASSERT(sizeof(g_fn_v302de_050) <= 10, size_check_g_fn_v302de_050);
	memcpy(&g_fn_v302de_050, exe_buf + 0x01c740, sizeof(g_fn_v302de_050));
	STATIC_ASSERT(sizeof(g_fn_v302de_051) <= 12, size_check_g_fn_v302de_051);
	memcpy(&g_fn_v302de_051, exe_buf + 0x01c74a, sizeof(g_fn_v302de_051));
	STATIC_ASSERT(sizeof(g_fn_v302de_052) <= 8, size_check_g_fn_v302de_052);
	memcpy(&g_fn_v302de_052, exe_buf + 0x01c756, sizeof(g_fn_v302de_052));
	STATIC_ASSERT(sizeof(g_fn_v302de_053) <= 10, size_check_g_fn_v302de_053);
	memcpy(&g_fn_v302de_053, exe_buf + 0x01c75e, sizeof(g_fn_v302de_053));
	STATIC_ASSERT(sizeof(g_fn_v302de_054) <= 14, size_check_g_fn_v302de_054);
	memcpy(&g_fn_v302de_054, exe_buf + 0x01c768, sizeof(g_fn_v302de_054));
	STATIC_ASSERT(sizeof(g_fn_v302de_055) <= 10, size_check_g_fn_v302de_055);
	memcpy(&g_fn_v302de_055, exe_buf + 0x01c776, sizeof(g_fn_v302de_055));
	STATIC_ASSERT(sizeof(g_fn_v302de_056) <= 13, size_check_g_fn_v302de_056);
	memcpy(&g_fn_v302de_056, exe_buf + 0x01c780, sizeof(g_fn_v302de_056));
	STATIC_ASSERT(sizeof(g_fn_v302de_057) <= 13, size_check_g_fn_v302de_057);
	memcpy(&g_fn_v302de_057, exe_buf + 0x01c78d, sizeof(g_fn_v302de_057));
	STATIC_ASSERT(sizeof(g_fn_v302de_058) <= 9, size_check_g_fn_v302de_058);
	memcpy(&g_fn_v302de_058, exe_buf + 0x01c79a, sizeof(g_fn_v302de_058));
	STATIC_ASSERT(sizeof(g_fn_v302de_059) <= 11, size_check_g_fn_v302de_059);
	memcpy(&g_fn_v302de_059, exe_buf + 0x01c7a3, sizeof(g_fn_v302de_059));
	STATIC_ASSERT(sizeof(g_fn_v302de_060) <= 9, size_check_g_fn_v302de_060);
	memcpy(&g_fn_v302de_060, exe_buf + 0x01c7ae, sizeof(g_fn_v302de_060));
	STATIC_ASSERT(sizeof(g_fn_v302de_061) <= 13, size_check_g_fn_v302de_061);
	memcpy(&g_fn_v302de_061, exe_buf + 0x01c7b7, sizeof(g_fn_v302de_061));
	STATIC_ASSERT(sizeof(g_fn_v302de_062) <= 12, size_check_g_fn_v302de_062);
	memcpy(&g_fn_v302de_062, exe_buf + 0x01c7c4, sizeof(g_fn_v302de_062));
	STATIC_ASSERT(sizeof(g_fn_v302de_063) <= 10, size_check_g_fn_v302de_063);
	memcpy(&g_fn_v302de_063, exe_buf + 0x01c7d0, sizeof(g_fn_v302de_063));
	STATIC_ASSERT(sizeof(g_fn_v302de_064) <= 9, size_check_g_fn_v302de_064);
	memcpy(&g_fn_v302de_064, exe_buf + 0x01c7da, sizeof(g_fn_v302de_064));
	STATIC_ASSERT(sizeof(g_fn_v302de_065) <= 12, size_check_g_fn_v302de_065);
	memcpy(&g_fn_v302de_065, exe_buf + 0x01c7e3, sizeof(g_fn_v302de_065));
	STATIC_ASSERT(sizeof(g_fn_v302de_066) <= 10, size_check_g_fn_v302de_066);
	memcpy(&g_fn_v302de_066, exe_buf + 0x01c7ef, sizeof(g_fn_v302de_066));
	STATIC_ASSERT(sizeof(g_fn_v302de_067) <= 13, size_check_g_fn_v302de_067);
	memcpy(&g_fn_v302de_067, exe_buf + 0x01c7f9, sizeof(g_fn_v302de_067));
	STATIC_ASSERT(sizeof(g_fn_v302de_068) <= 13, size_check_g_fn_v302de_068);
	memcpy(&g_fn_v302de_068, exe_buf + 0x01c806, sizeof(g_fn_v302de_068));
	STATIC_ASSERT(sizeof(g_fn_v302de_069) <= 13, size_check_g_fn_v302de_069);
	memcpy(&g_fn_v302de_069, exe_buf + 0x01c813, sizeof(g_fn_v302de_069));
	STATIC_ASSERT(sizeof(g_fn_v302de_070) <= 11, size_check_g_fn_v302de_070);
	memcpy(&g_fn_v302de_070, exe_buf + 0x01c820, sizeof(g_fn_v302de_070));
	STATIC_ASSERT(sizeof(g_fn_v302de_071) <= 12, size_check_g_fn_v302de_071);
	memcpy(&g_fn_v302de_071, exe_buf + 0x01c82b, sizeof(g_fn_v302de_071));
	STATIC_ASSERT(sizeof(g_fn_v302de_072) <= 13, size_check_g_fn_v302de_072);
	memcpy(&g_fn_v302de_072, exe_buf + 0x01c837, sizeof(g_fn_v302de_072));
	STATIC_ASSERT(sizeof(g_fn_v302de_073) <= 11, size_check_g_fn_v302de_073);
	memcpy(&g_fn_v302de_073, exe_buf + 0x01c844, sizeof(g_fn_v302de_073));
	STATIC_ASSERT(sizeof(g_fn_v302de_074) <= 12, size_check_g_fn_v302de_074);
	memcpy(&g_fn_v302de_074, exe_buf + 0x01c84f, sizeof(g_fn_v302de_074));
	STATIC_ASSERT(sizeof(g_fn_v302de_075) <= 13, size_check_g_fn_v302de_075);
	memcpy(&g_fn_v302de_075, exe_buf + 0x01c85b, sizeof(g_fn_v302de_075));
	STATIC_ASSERT(sizeof(g_fn_v302de_076) <= 13, size_check_g_fn_v302de_076);
	memcpy(&g_fn_v302de_076, exe_buf + 0x01c868, sizeof(g_fn_v302de_076));
	STATIC_ASSERT(sizeof(g_fn_v302de_077) <= 10, size_check_g_fn_v302de_077);
	memcpy(&g_fn_v302de_077, exe_buf + 0x01c875, sizeof(g_fn_v302de_077));
	STATIC_ASSERT(sizeof(g_fn_v302de_078) <= 12, size_check_g_fn_v302de_078);
	memcpy(&g_fn_v302de_078, exe_buf + 0x01c87f, sizeof(g_fn_v302de_078));
	STATIC_ASSERT(sizeof(g_fn_v302de_079) <= 11, size_check_g_fn_v302de_079);
	memcpy(&g_fn_v302de_079, exe_buf + 0x01c88b, sizeof(g_fn_v302de_079));
	STATIC_ASSERT(sizeof(g_fn_v302de_080) <= 11, size_check_g_fn_v302de_080);
	memcpy(&g_fn_v302de_080, exe_buf + 0x01c896, sizeof(g_fn_v302de_080));
	STATIC_ASSERT(sizeof(g_fn_v302de_081) <= 13, size_check_g_fn_v302de_081);
	memcpy(&g_fn_v302de_081, exe_buf + 0x01c8a1, sizeof(g_fn_v302de_081));
	STATIC_ASSERT(sizeof(g_fn_v302de_082) <= 13, size_check_g_fn_v302de_082);
	memcpy(&g_fn_v302de_082, exe_buf + 0x01c8ae, sizeof(g_fn_v302de_082));
	STATIC_ASSERT(sizeof(g_fn_v302de_083) <= 13, size_check_g_fn_v302de_083);
	memcpy(&g_fn_v302de_083, exe_buf + 0x01c8bb, sizeof(g_fn_v302de_083));
	STATIC_ASSERT(sizeof(g_fn_v302de_084) <= 11, size_check_g_fn_v302de_084);
	memcpy(&g_fn_v302de_084, exe_buf + 0x01c8c8, sizeof(g_fn_v302de_084));
	STATIC_ASSERT(sizeof(g_fn_v302de_085) <= 13, size_check_g_fn_v302de_085);
	memcpy(&g_fn_v302de_085, exe_buf + 0x01c8d3, sizeof(g_fn_v302de_085));
	STATIC_ASSERT(sizeof(g_fn_v302de_086) <= 11, size_check_g_fn_v302de_086);
	memcpy(&g_fn_v302de_086, exe_buf + 0x01c8e0, sizeof(g_fn_v302de_086));
	STATIC_ASSERT(sizeof(g_fn_v302de_087) <= 13, size_check_g_fn_v302de_087);
	memcpy(&g_fn_v302de_087, exe_buf + 0x01c8eb, sizeof(g_fn_v302de_087));
	STATIC_ASSERT(sizeof(g_fn_v302de_088) <= 11, size_check_g_fn_v302de_088);
	memcpy(&g_fn_v302de_088, exe_buf + 0x01c8f8, sizeof(g_fn_v302de_088));
	STATIC_ASSERT(sizeof(g_fn_v302de_089) <= 11, size_check_g_fn_v302de_089);
	memcpy(&g_fn_v302de_089, exe_buf + 0x01c903, sizeof(g_fn_v302de_089));
	STATIC_ASSERT(sizeof(g_fn_v302de_090) <= 13, size_check_g_fn_v302de_090);
	memcpy(&g_fn_v302de_090, exe_buf + 0x01c90e, sizeof(g_fn_v302de_090));
	STATIC_ASSERT(sizeof(g_fn_v302de_091) <= 11, size_check_g_fn_v302de_091);
	memcpy(&g_fn_v302de_091, exe_buf + 0x01c91b, sizeof(g_fn_v302de_091));
	STATIC_ASSERT(sizeof(g_fn_v302de_092) <= 13, size_check_g_fn_v302de_092);
	memcpy(&g_fn_v302de_092, exe_buf + 0x01c926, sizeof(g_fn_v302de_092));
	STATIC_ASSERT(sizeof(g_fn_v302de_093) <= 10, size_check_g_fn_v302de_093);
	memcpy(&g_fn_v302de_093, exe_buf + 0x01c933, sizeof(g_fn_v302de_093));
	STATIC_ASSERT(sizeof(g_fn_v302de_094) <= 13, size_check_g_fn_v302de_094);
	memcpy(&g_fn_v302de_094, exe_buf + 0x01c93d, sizeof(g_fn_v302de_094));
	STATIC_ASSERT(sizeof(g_fn_v302de_095) <= 13, size_check_g_fn_v302de_095);
	memcpy(&g_fn_v302de_095, exe_buf + 0x01c94a, sizeof(g_fn_v302de_095));
	STATIC_ASSERT(sizeof(g_fn_v302de_096) <= 13, size_check_g_fn_v302de_096);
	memcpy(&g_fn_v302de_096, exe_buf + 0x01c957, sizeof(g_fn_v302de_096));
	STATIC_ASSERT(sizeof(g_fn_v302de_097) <= 13, size_check_g_fn_v302de_097);
	memcpy(&g_fn_v302de_097, exe_buf + 0x01c964, sizeof(g_fn_v302de_097));
	STATIC_ASSERT(sizeof(g_fn_v302de_098) <= 13, size_check_g_fn_v302de_098);
	memcpy(&g_fn_v302de_098, exe_buf + 0x01c971, sizeof(g_fn_v302de_098));
	STATIC_ASSERT(sizeof(g_fn_v302de_099) <= 13, size_check_g_fn_v302de_099);
	memcpy(&g_fn_v302de_099, exe_buf + 0x01c97e, sizeof(g_fn_v302de_099));
	STATIC_ASSERT(sizeof(g_fn_v302de_100) <= 12, size_check_g_fn_v302de_100);
	memcpy(&g_fn_v302de_100, exe_buf + 0x01c98b, sizeof(g_fn_v302de_100));
	STATIC_ASSERT(sizeof(g_fn_v302de_101) <= 11, size_check_g_fn_v302de_101);
	memcpy(&g_fn_v302de_101, exe_buf + 0x01c997, sizeof(g_fn_v302de_101));
	STATIC_ASSERT(sizeof(g_fn_v302de_102) <= 10, size_check_g_fn_v302de_102);
	memcpy(&g_fn_v302de_102, exe_buf + 0x01c9a2, sizeof(g_fn_v302de_102));
	STATIC_ASSERT(sizeof(g_fn_v302de_103) <= 12, size_check_g_fn_v302de_103);
	memcpy(&g_fn_v302de_103, exe_buf + 0x01c9ac, sizeof(g_fn_v302de_103));
	STATIC_ASSERT(sizeof(g_fn_v302de_104) <= 8, size_check_g_fn_v302de_104);
	memcpy(&g_fn_v302de_104, exe_buf + 0x01c9b8, sizeof(g_fn_v302de_104));
	STATIC_ASSERT(sizeof(g_fn_v302de_105) <= 10, size_check_g_fn_v302de_105);
	memcpy(&g_fn_v302de_105, exe_buf + 0x01c9c0, sizeof(g_fn_v302de_105));
	STATIC_ASSERT(sizeof(g_fn_v302de_106) <= 13, size_check_g_fn_v302de_106);
	memcpy(&g_fn_v302de_106, exe_buf + 0x01c9ca, sizeof(g_fn_v302de_106));
	STATIC_ASSERT(sizeof(g_fn_v302de_107) <= 10, size_check_g_fn_v302de_107);
	memcpy(&g_fn_v302de_107, exe_buf + 0x01c9d7, sizeof(g_fn_v302de_107));
	STATIC_ASSERT(sizeof(g_fn_v302de_108) <= 13, size_check_g_fn_v302de_108);
	memcpy(&g_fn_v302de_108, exe_buf + 0x01c9e1, sizeof(g_fn_v302de_108));
	STATIC_ASSERT(sizeof(g_fn_v302de_109) <= 13, size_check_g_fn_v302de_109);
	memcpy(&g_fn_v302de_109, exe_buf + 0x01c9ee, sizeof(g_fn_v302de_109));
	STATIC_ASSERT(sizeof(g_fn_v302de_110) <= 9, size_check_g_fn_v302de_110);
	memcpy(&g_fn_v302de_110, exe_buf + 0x01c9fb, sizeof(g_fn_v302de_110));
	STATIC_ASSERT(sizeof(g_fn_v302de_111) <= 11, size_check_g_fn_v302de_111);
	memcpy(&g_fn_v302de_111, exe_buf + 0x01ca04, sizeof(g_fn_v302de_111));
	STATIC_ASSERT(sizeof(g_fn_v302de_112) <= 9, size_check_g_fn_v302de_112);
	memcpy(&g_fn_v302de_112, exe_buf + 0x01ca0f, sizeof(g_fn_v302de_112));
	STATIC_ASSERT(sizeof(g_fn_v302de_113) <= 13, size_check_g_fn_v302de_113);
	memcpy(&g_fn_v302de_113, exe_buf + 0x01ca18, sizeof(g_fn_v302de_113));
	STATIC_ASSERT(sizeof(g_fn_v302de_114) <= 12, size_check_g_fn_v302de_114);
	memcpy(&g_fn_v302de_114, exe_buf + 0x01ca25, sizeof(g_fn_v302de_114));
	STATIC_ASSERT(sizeof(g_fn_v302de_115) <= 10, size_check_g_fn_v302de_115);
	memcpy(&g_fn_v302de_115, exe_buf + 0x01ca31, sizeof(g_fn_v302de_115));
	STATIC_ASSERT(sizeof(g_fn_v302de_116) <= 9, size_check_g_fn_v302de_116);
	memcpy(&g_fn_v302de_116, exe_buf + 0x01ca3b, sizeof(g_fn_v302de_116));
	STATIC_ASSERT(sizeof(g_fn_v302de_117) <= 12, size_check_g_fn_v302de_117);
	memcpy(&g_fn_v302de_117, exe_buf + 0x01ca44, sizeof(g_fn_v302de_117));
	STATIC_ASSERT(sizeof(g_fn_v302de_118) <= 10, size_check_g_fn_v302de_118);
	memcpy(&g_fn_v302de_118, exe_buf + 0x01ca50, sizeof(g_fn_v302de_118));
	STATIC_ASSERT(sizeof(g_fn_v302de_119) <= 13, size_check_g_fn_v302de_119);
	memcpy(&g_fn_v302de_119, exe_buf + 0x01ca5a, sizeof(g_fn_v302de_119));
	STATIC_ASSERT(sizeof(g_fn_v302de_120) <= 13, size_check_g_fn_v302de_120);
	memcpy(&g_fn_v302de_120, exe_buf + 0x01ca67, sizeof(g_fn_v302de_120));
	STATIC_ASSERT(sizeof(g_fn_v302de_121) <= 13, size_check_g_fn_v302de_121);
	memcpy(&g_fn_v302de_121, exe_buf + 0x01ca74, sizeof(g_fn_v302de_121));
	STATIC_ASSERT(sizeof(g_fn_v302de_122) <= 11, size_check_g_fn_v302de_122);
	memcpy(&g_fn_v302de_122, exe_buf + 0x01ca81, sizeof(g_fn_v302de_122));
	STATIC_ASSERT(sizeof(g_fn_v302de_123) <= 12, size_check_g_fn_v302de_123);
	memcpy(&g_fn_v302de_123, exe_buf + 0x01ca8c, sizeof(g_fn_v302de_123));
	STATIC_ASSERT(sizeof(g_fn_v302de_124) <= 13, size_check_g_fn_v302de_124);
	memcpy(&g_fn_v302de_124, exe_buf + 0x01ca98, sizeof(g_fn_v302de_124));
	STATIC_ASSERT(sizeof(g_fn_v302de_125) <= 11, size_check_g_fn_v302de_125);
	memcpy(&g_fn_v302de_125, exe_buf + 0x01caa5, sizeof(g_fn_v302de_125));
	STATIC_ASSERT(sizeof(g_fn_v302de_126) <= 13, size_check_g_fn_v302de_126);
	memcpy(&g_fn_v302de_126, exe_buf + 0x01cab0, sizeof(g_fn_v302de_126));
	STATIC_ASSERT(sizeof(g_fn_v302de_127) <= 13, size_check_g_fn_v302de_127);
	memcpy(&g_fn_v302de_127, exe_buf + 0x01cabd, sizeof(g_fn_v302de_127));
	STATIC_ASSERT(sizeof(g_fn_v302de_128) <= 13, size_check_g_fn_v302de_128);
	memcpy(&g_fn_v302de_128, exe_buf + 0x01caca, sizeof(g_fn_v302de_128));
	STATIC_ASSERT(sizeof(g_fn_v302de_129) <= 12, size_check_g_fn_v302de_129);
	memcpy(&g_fn_v302de_129, exe_buf + 0x01cad7, sizeof(g_fn_v302de_129));
	STATIC_ASSERT(sizeof(g_fn_v302de_130) <= 11, size_check_g_fn_v302de_130);
	memcpy(&g_fn_v302de_130, exe_buf + 0x01cae3, sizeof(g_fn_v302de_130));
	STATIC_ASSERT(sizeof(g_fn_v302de_131) <= 11, size_check_g_fn_v302de_131);
	memcpy(&g_fn_v302de_131, exe_buf + 0x01caee, sizeof(g_fn_v302de_131));
	STATIC_ASSERT(sizeof(g_fn_v302de_132) <= 12, size_check_g_fn_v302de_132);
	memcpy(&g_fn_v302de_132, exe_buf + 0x01caf9, sizeof(g_fn_v302de_132));
	STATIC_ASSERT(sizeof(g_fn_v302de_133) <= 10, size_check_g_fn_v302de_133);
	memcpy(&g_fn_v302de_133, exe_buf + 0x01cb05, sizeof(g_fn_v302de_133));
	STATIC_ASSERT(sizeof(g_fn_v302de_134) <= 9, size_check_g_fn_v302de_134);
	memcpy(&g_fn_v302de_134, exe_buf + 0x01cb0f, sizeof(g_fn_v302de_134));
	STATIC_ASSERT(sizeof(g_fn_v302de_135) <= 12, size_check_g_fn_v302de_135);
	memcpy(&g_fn_v302de_135, exe_buf + 0x01cb18, sizeof(g_fn_v302de_135));
	STATIC_ASSERT(sizeof(g_fn_v302de_136) <= 13, size_check_g_fn_v302de_136);
	memcpy(&g_fn_v302de_136, exe_buf + 0x01cb24, sizeof(g_fn_v302de_136));
	STATIC_ASSERT(sizeof(g_fn_v302de_137) <= 10, size_check_g_fn_v302de_137);
	memcpy(&g_fn_v302de_137, exe_buf + 0x01cb31, sizeof(g_fn_v302de_137));
	STATIC_ASSERT(sizeof(g_fn_v302de_138) <= 11, size_check_g_fn_v302de_138);
	memcpy(&g_fn_v302de_138, exe_buf + 0x01cb3b, sizeof(g_fn_v302de_138));
	STATIC_ASSERT(sizeof(g_fn_v302de_139) <= 11, size_check_g_fn_v302de_139);
	memcpy(&g_fn_v302de_139, exe_buf + 0x01cb46, sizeof(g_fn_v302de_139));
	STATIC_ASSERT(sizeof(g_fn_v302de_140) <= 11, size_check_g_fn_v302de_140);
	memcpy(&g_fn_v302de_140, exe_buf + 0x01cb51, sizeof(g_fn_v302de_140));
	STATIC_ASSERT(sizeof(g_fn_v302de_141) <= 12, size_check_g_fn_v302de_141);
	memcpy(&g_fn_v302de_141, exe_buf + 0x01cb5c, sizeof(g_fn_v302de_141));
	STATIC_ASSERT(sizeof(g_fn_v302de_142) <= 12, size_check_g_fn_v302de_142);
	memcpy(&g_fn_v302de_142, exe_buf + 0x01cb68, sizeof(g_fn_v302de_142));
	STATIC_ASSERT(sizeof(g_fn_v302de_143) <= 12, size_check_g_fn_v302de_143);
	memcpy(&g_fn_v302de_143, exe_buf + 0x01cb74, sizeof(g_fn_v302de_143));
	STATIC_ASSERT(sizeof(g_fn_v302de_144) <= 11, size_check_g_fn_v302de_144);
	memcpy(&g_fn_v302de_144, exe_buf + 0x01cb80, sizeof(g_fn_v302de_144));
	STATIC_ASSERT(sizeof(g_fn_v302de_145) <= 10, size_check_g_fn_v302de_145);
	memcpy(&g_fn_v302de_145, exe_buf + 0x01cb8b, sizeof(g_fn_v302de_145));
	STATIC_ASSERT(sizeof(g_fn_v302de_146) <= 10, size_check_g_fn_v302de_146);
	memcpy(&g_fn_v302de_146, exe_buf + 0x01cb95, sizeof(g_fn_v302de_146));
	STATIC_ASSERT(sizeof(g_fn_v302de_147) <= 8, size_check_g_fn_v302de_147);
	memcpy(&g_fn_v302de_147, exe_buf + 0x01cb9f, sizeof(g_fn_v302de_147));
	STATIC_ASSERT(sizeof(g_fn_v302de_148) <= 11, size_check_g_fn_v302de_148);
	memcpy(&g_fn_v302de_148, exe_buf + 0x01cba7, sizeof(g_fn_v302de_148));
	STATIC_ASSERT(sizeof(g_fn_v302de_149) <= 9, size_check_g_fn_v302de_149);
	memcpy(&g_fn_v302de_149, exe_buf + 0x01cbb2, sizeof(g_fn_v302de_149));
	STATIC_ASSERT(sizeof(g_fn_v302de_150) <= 12, size_check_g_fn_v302de_150);
	memcpy(&g_fn_v302de_150, exe_buf + 0x01cbbb, sizeof(g_fn_v302de_150));
	STATIC_ASSERT(sizeof(g_fn_v302de_151) <= 10, size_check_g_fn_v302de_151);
	memcpy(&g_fn_v302de_151, exe_buf + 0x01cbc7, sizeof(g_fn_v302de_151));
	STATIC_ASSERT(sizeof(g_fn_v302de_152) <= 6, size_check_g_fn_v302de_152);
	memcpy(&g_fn_v302de_152, exe_buf + 0x01cbd1, sizeof(g_fn_v302de_152));
	STATIC_ASSERT(sizeof(g_fn_v302de_153) <= 10, size_check_g_fn_v302de_153);
	memcpy(&g_fn_v302de_153, exe_buf + 0x01cbd7, sizeof(g_fn_v302de_153));
	STATIC_ASSERT(sizeof(g_fn_v302de_154) <= 1, size_check_g_fn_v302de_154);
	memcpy(&g_fn_v302de_154, exe_buf + 0x01cbe1, sizeof(g_fn_v302de_154));
	STATIC_ASSERT(sizeof(g_fn_v302de_155) <= 1, size_check_g_fn_v302de_155);
	memcpy(&g_fn_v302de_155, exe_buf + 0x01cbe2, sizeof(g_fn_v302de_155));
	STATIC_ASSERT(sizeof(g_fn_v302de_156) <= 9, size_check_g_fn_v302de_156);
	memcpy(&g_fn_v302de_156, exe_buf + 0x01cbe3, sizeof(g_fn_v302de_156));
	STATIC_ASSERT(sizeof(g_fn_v302de_157) <= 12, size_check_g_fn_v302de_157);
	memcpy(&g_fn_v302de_157, exe_buf + 0x01cbec, sizeof(g_fn_v302de_157));
	STATIC_ASSERT(sizeof(g_fn_v302de_158) <= 1, size_check_g_fn_v302de_158);
	memcpy(&g_fn_v302de_158, exe_buf + 0x01cbf8, sizeof(g_fn_v302de_158));
	STATIC_ASSERT(sizeof(g_fn_v302de_159) <= 13, size_check_g_fn_v302de_159);
	memcpy(&g_fn_v302de_159, exe_buf + 0x01cbf9, sizeof(g_fn_v302de_159));
	STATIC_ASSERT(sizeof(g_fn_v302de_160) <= 13, size_check_g_fn_v302de_160);
	memcpy(&g_fn_v302de_160, exe_buf + 0x01cc06, sizeof(g_fn_v302de_160));
	STATIC_ASSERT(sizeof(g_fn_v302de_161) <= 13, size_check_g_fn_v302de_161);
	memcpy(&g_fn_v302de_161, exe_buf + 0x01cc13, sizeof(g_fn_v302de_161));
	STATIC_ASSERT(sizeof(g_fn_v302de_162) <= 10, size_check_g_fn_v302de_162);
	memcpy(&g_fn_v302de_162, exe_buf + 0x01cc20, sizeof(g_fn_v302de_162));
	STATIC_ASSERT(sizeof(g_fn_v302de_163) <= 10, size_check_g_fn_v302de_163);
	memcpy(&g_fn_v302de_163, exe_buf + 0x01cc2a, sizeof(g_fn_v302de_163));
	STATIC_ASSERT(sizeof(g_fn_v302de_164) <= 10, size_check_g_fn_v302de_164);
	memcpy(&g_fn_v302de_164, exe_buf + 0x01cc34, sizeof(g_fn_v302de_164));
	STATIC_ASSERT(sizeof(g_fn_v302de_165) <= 10, size_check_g_fn_v302de_165);
	memcpy(&g_fn_v302de_165, exe_buf + 0x01cc3e, sizeof(g_fn_v302de_165));
	STATIC_ASSERT(sizeof(g_fn_v302de_166) <= 10, size_check_g_fn_v302de_166);
	memcpy(&g_fn_v302de_166, exe_buf + 0x01cc48, sizeof(g_fn_v302de_166));
	STATIC_ASSERT(sizeof(g_fn_v302de_167) <= 10, size_check_g_fn_v302de_167);
	memcpy(&g_fn_v302de_167, exe_buf + 0x01cc52, sizeof(g_fn_v302de_167));
	STATIC_ASSERT(sizeof(g_fn_v302de_168) <= 10, size_check_g_fn_v302de_168);
	memcpy(&g_fn_v302de_168, exe_buf + 0x01cc5c, sizeof(g_fn_v302de_168));
	STATIC_ASSERT(sizeof(g_fn_v302de_169) <= 12, size_check_g_fn_v302de_169);
	memcpy(&g_fn_v302de_169, exe_buf + 0x01cc66, sizeof(g_fn_v302de_169));
	STATIC_ASSERT(sizeof(g_fn_v302de_170) <= 12, size_check_g_fn_v302de_170);
	memcpy(&g_fn_v302de_170, exe_buf + 0x01cc72, sizeof(g_fn_v302de_170));
	STATIC_ASSERT(sizeof(g_fn_v302de_171) <= 12, size_check_g_fn_v302de_171);
	memcpy(&g_fn_v302de_171, exe_buf + 0x01cc7e, sizeof(g_fn_v302de_171));
	STATIC_ASSERT(sizeof(g_fn_v302de_172) <= 12, size_check_g_fn_v302de_172);
	memcpy(&g_fn_v302de_172, exe_buf + 0x01cc8a, sizeof(g_fn_v302de_172));
	STATIC_ASSERT(sizeof(g_fn_v302de_173) <= 9, size_check_g_fn_v302de_173);
	memcpy(&g_fn_v302de_173, exe_buf + 0x01cc96, sizeof(g_fn_v302de_173));
	STATIC_ASSERT(sizeof(g_fn_v302de_174) <= 11, size_check_g_fn_v302de_174);
	memcpy(&g_fn_v302de_174, exe_buf + 0x01cc9f, sizeof(g_fn_v302de_174));
	STATIC_ASSERT(sizeof(g_fn_v302de_175) <= 13, size_check_g_fn_v302de_175);
	memcpy(&g_fn_v302de_175, exe_buf + 0x01ccaa, sizeof(g_fn_v302de_175));
	STATIC_ASSERT(sizeof(g_fn_v302de_176) <= 12, size_check_g_fn_v302de_176);
	memcpy(&g_fn_v302de_176, exe_buf + 0x01ccb7, sizeof(g_fn_v302de_176));
	STATIC_ASSERT(sizeof(g_fn_v302de_177) <= 9, size_check_g_fn_v302de_177);
	memcpy(&g_fn_v302de_177, exe_buf + 0x01ccc3, sizeof(g_fn_v302de_177));
	STATIC_ASSERT(sizeof(g_fn_v302de_178) <= 13, size_check_g_fn_v302de_178);
	memcpy(&g_fn_v302de_178, exe_buf + 0x01cccc, sizeof(g_fn_v302de_178));
	STATIC_ASSERT(sizeof(g_fn_v302de_179) <= 13, size_check_g_fn_v302de_179);
	memcpy(&g_fn_v302de_179, exe_buf + 0x01ccd9, sizeof(g_fn_v302de_179));
	STATIC_ASSERT(sizeof(g_fn_v302de_180) <= 9, size_check_g_fn_v302de_180);
	memcpy(&g_fn_v302de_180, exe_buf + 0x01cce6, sizeof(g_fn_v302de_180));
	STATIC_ASSERT(sizeof(g_fn_v302de_181) <= 11, size_check_g_fn_v302de_181);
	memcpy(&g_fn_v302de_181, exe_buf + 0x01ccef, sizeof(g_fn_v302de_181));
	STATIC_ASSERT(sizeof(g_fn_v302de_182) <= 12, size_check_g_fn_v302de_182);
	memcpy(&g_fn_v302de_182, exe_buf + 0x01ccfa, sizeof(g_fn_v302de_182));
	STATIC_ASSERT(sizeof(g_fn_v302de_183) <= 13, size_check_g_fn_v302de_183);
	memcpy(&g_fn_v302de_183, exe_buf + 0x01cd06, sizeof(g_fn_v302de_183));
	STATIC_ASSERT(sizeof(g_fn_v302de_184) <= 12, size_check_g_fn_v302de_184);
	memcpy(&g_fn_v302de_184, exe_buf + 0x01cd13, sizeof(g_fn_v302de_184));
	STATIC_ASSERT(sizeof(g_fn_v302de_185) <= 13, size_check_g_fn_v302de_185);
	memcpy(&g_fn_v302de_185, exe_buf + 0x01cd1f, sizeof(g_fn_v302de_185));
	STATIC_ASSERT(sizeof(g_fn_v302de_186) <= 11, size_check_g_fn_v302de_186);
	memcpy(&g_fn_v302de_186, exe_buf + 0x01cd2c, sizeof(g_fn_v302de_186));
	STATIC_ASSERT(sizeof(g_fn_v302de_187) <= 11, size_check_g_fn_v302de_187);
	memcpy(&g_fn_v302de_187, exe_buf + 0x01cd37, sizeof(g_fn_v302de_187));
	STATIC_ASSERT(sizeof(g_fn_v302de_188) <= 11, size_check_g_fn_v302de_188);
	memcpy(&g_fn_v302de_188, exe_buf + 0x01cd42, sizeof(g_fn_v302de_188));
	STATIC_ASSERT(sizeof(g_fn_v302de_189) <= 11, size_check_g_fn_v302de_189);
	memcpy(&g_fn_v302de_189, exe_buf + 0x01cd4d, sizeof(g_fn_v302de_189));
	STATIC_ASSERT(sizeof(g_fn_v302de_190) <= 12, size_check_g_fn_v302de_190);
	memcpy(&g_fn_v302de_190, exe_buf + 0x01cd58, sizeof(g_fn_v302de_190));
	STATIC_ASSERT(sizeof(g_fn_v302de_191) <= 13, size_check_g_fn_v302de_191);
	memcpy(&g_fn_v302de_191, exe_buf + 0x01cd64, sizeof(g_fn_v302de_191));
	STATIC_ASSERT(sizeof(g_fn_v302de_192) <= 13, size_check_g_fn_v302de_192);
	memcpy(&g_fn_v302de_192, exe_buf + 0x01cd71, sizeof(g_fn_v302de_192));
	STATIC_ASSERT(sizeof(g_fn_v302de_193) <= 13, size_check_g_fn_v302de_193);
	memcpy(&g_fn_v302de_193, exe_buf + 0x01cd7e, sizeof(g_fn_v302de_193));
	STATIC_ASSERT(sizeof(g_fn_v302de_194) <= 13, size_check_g_fn_v302de_194);
	memcpy(&g_fn_v302de_194, exe_buf + 0x01cd8b, sizeof(g_fn_v302de_194));
	STATIC_ASSERT(sizeof(g_fn_v302de_195) <= 13, size_check_g_fn_v302de_195);
	memcpy(&g_fn_v302de_195, exe_buf + 0x01cd98, sizeof(g_fn_v302de_195));
	STATIC_ASSERT(sizeof(g_fn_v302de_196) <= 13, size_check_g_fn_v302de_196);
	memcpy(&g_fn_v302de_196, exe_buf + 0x01cda5, sizeof(g_fn_v302de_196));
	STATIC_ASSERT(sizeof(g_fn_v302de_197) <= 13, size_check_g_fn_v302de_197);
	memcpy(&g_fn_v302de_197, exe_buf + 0x01cdb2, sizeof(g_fn_v302de_197));
	STATIC_ASSERT(sizeof(g_fn_v302de_198) <= 13, size_check_g_fn_v302de_198);
	memcpy(&g_fn_v302de_198, exe_buf + 0x01cdbf, sizeof(g_fn_v302de_198));
	STATIC_ASSERT(sizeof(g_fn_v302de_199) <= 13, size_check_g_fn_v302de_199);
	memcpy(&g_fn_v302de_199, exe_buf + 0x01cdcc, sizeof(g_fn_v302de_199));
	STATIC_ASSERT(sizeof(g_fn_v302de_200) <= 13, size_check_g_fn_v302de_200);
	memcpy(&g_fn_v302de_200, exe_buf + 0x01cdd9, sizeof(g_fn_v302de_200));
	STATIC_ASSERT(sizeof(g_fn_v302de_201) <= 12, size_check_g_fn_v302de_201);
	memcpy(&g_fn_v302de_201, exe_buf + 0x01cde6, sizeof(g_fn_v302de_201));
	STATIC_ASSERT(sizeof(g_fn_v302de_202) <= 12, size_check_g_fn_v302de_202);
	memcpy(&g_fn_v302de_202, exe_buf + 0x01cdf2, sizeof(g_fn_v302de_202));
	STATIC_ASSERT(sizeof(g_fn_v302de_203) <= 12, size_check_g_fn_v302de_203);
	memcpy(&g_fn_v302de_203, exe_buf + 0x01cdfe, sizeof(g_fn_v302de_203));
	STATIC_ASSERT(sizeof(g_fn_v302de_204) <= 12, size_check_g_fn_v302de_204);
	memcpy(&g_fn_v302de_204, exe_buf + 0x01ce0a, sizeof(g_fn_v302de_204));
	STATIC_ASSERT(sizeof(g_fn_v302de_205) <= 10, size_check_g_fn_v302de_205);
	memcpy(&g_fn_v302de_205, exe_buf + 0x01ce16, sizeof(g_fn_v302de_205));
	STATIC_ASSERT(sizeof(g_fn_v302de_206) <= 11, size_check_g_fn_v302de_206);
	memcpy(&g_fn_v302de_206, exe_buf + 0x01ce20, sizeof(g_fn_v302de_206));
	STATIC_ASSERT(sizeof(g_fn_v302de_207) <= 10, size_check_g_fn_v302de_207);
	memcpy(&g_fn_v302de_207, exe_buf + 0x01ce2b, sizeof(g_fn_v302de_207));
	STATIC_ASSERT(sizeof(g_fn_v302de_208) <= 9, size_check_g_fn_v302de_208);
	memcpy(&g_fn_v302de_208, exe_buf + 0x01ce35, sizeof(g_fn_v302de_208));
	STATIC_ASSERT(sizeof(g_fn_v302de_209) <= 9, size_check_g_fn_v302de_209);
	memcpy(&g_fn_v302de_209, exe_buf + 0x01ce3e, sizeof(g_fn_v302de_209));
	STATIC_ASSERT(sizeof(g_fn_v302de_210) <= 13, size_check_g_fn_v302de_210);
	memcpy(&g_fn_v302de_210, exe_buf + 0x01ce47, sizeof(g_fn_v302de_210));
	STATIC_ASSERT(sizeof(g_fn_v302de_211) <= 13, size_check_g_fn_v302de_211);
	memcpy(&g_fn_v302de_211, exe_buf + 0x01ce54, sizeof(g_fn_v302de_211));
	STATIC_ASSERT(sizeof(g_fn_v302de_212) <= 12, size_check_g_fn_v302de_212);
	memcpy(&g_fn_v302de_212, exe_buf + 0x01ce61, sizeof(g_fn_v302de_212));
	STATIC_ASSERT(sizeof(g_fn_v302de_213) <= 11, size_check_g_fn_v302de_213);
	memcpy(&g_fn_v302de_213, exe_buf + 0x01ce6d, sizeof(g_fn_v302de_213));
	STATIC_ASSERT(sizeof(g_fn_v302de_214) <= 9, size_check_g_fn_v302de_214);
	memcpy(&g_fn_v302de_214, exe_buf + 0x01ce78, sizeof(g_fn_v302de_214));
	STATIC_ASSERT(sizeof(g_fn_v302de_215) <= 9, size_check_g_fn_v302de_215);
	memcpy(&g_fn_v302de_215, exe_buf + 0x01ce81, sizeof(g_fn_v302de_215));
	STATIC_ASSERT(sizeof(g_fn_v302de_216) <= 8, size_check_g_fn_v302de_216);
	memcpy(&g_fn_v302de_216, exe_buf + 0x01ce8a, sizeof(g_fn_v302de_216));
	STATIC_ASSERT(sizeof(g_fn_v302de_217) <= 12, size_check_g_fn_v302de_217);
	memcpy(&g_fn_v302de_217, exe_buf + 0x01ce92, sizeof(g_fn_v302de_217));
	STATIC_ASSERT(sizeof(g_fn_v302de_218) <= 12, size_check_g_fn_v302de_218);
	memcpy(&g_fn_v302de_218, exe_buf + 0x01ce9e, sizeof(g_fn_v302de_218));
	STATIC_ASSERT(sizeof(g_fn_v302de_219) <= 12, size_check_g_fn_v302de_219);
	memcpy(&g_fn_v302de_219, exe_buf + 0x01ceaa, sizeof(g_fn_v302de_219));
	STATIC_ASSERT(sizeof(g_fn_v302de_220) <= 10, size_check_g_fn_v302de_220);
	memcpy(&g_fn_v302de_220, exe_buf + 0x01ceb6, sizeof(g_fn_v302de_220));
	STATIC_ASSERT(sizeof(g_fn_v302de_221) <= 13, size_check_g_fn_v302de_221);
	memcpy(&g_fn_v302de_221, exe_buf + 0x01cec0, sizeof(g_fn_v302de_221));
	STATIC_ASSERT(sizeof(g_fn_v302de_222) <= 13, size_check_g_fn_v302de_222);
	memcpy(&g_fn_v302de_222, exe_buf + 0x01cecd, sizeof(g_fn_v302de_222));
	STATIC_ASSERT(sizeof(g_fn_v302de_223) <= 11, size_check_g_fn_v302de_223);
	memcpy(&g_fn_v302de_223, exe_buf + 0x01ceda, sizeof(g_fn_v302de_223));
	STATIC_ASSERT(sizeof(g_fn_v302de_224) <= 12, size_check_g_fn_v302de_224);
	memcpy(&g_fn_v302de_224, exe_buf + 0x01cee5, sizeof(g_fn_v302de_224));
	STATIC_ASSERT(sizeof(g_fn_v302de_225) <= 8, size_check_g_fn_v302de_225);
	memcpy(&g_fn_v302de_225, exe_buf + 0x01cef1, sizeof(g_fn_v302de_225));
	STATIC_ASSERT(sizeof(g_fn_v302de_226) <= 12, size_check_g_fn_v302de_226);
	memcpy(&g_fn_v302de_226, exe_buf + 0x01cef9, sizeof(g_fn_v302de_226));
	STATIC_ASSERT(sizeof(g_fn_v302de_227) <= 11, size_check_g_fn_v302de_227);
	memcpy(&g_fn_v302de_227, exe_buf + 0x01cf05, sizeof(g_fn_v302de_227));
	STATIC_ASSERT(sizeof(g_fn_v302de_228) <= 11, size_check_g_fn_v302de_228);
	memcpy(&g_fn_v302de_228, exe_buf + 0x01cf10, sizeof(g_fn_v302de_228));
	STATIC_ASSERT(sizeof(g_fn_v302de_229) <= 11, size_check_g_fn_v302de_229);
	memcpy(&g_fn_v302de_229, exe_buf + 0x01cf1b, sizeof(g_fn_v302de_229));
	STATIC_ASSERT(sizeof(g_fn_v302de_230) <= 12, size_check_g_fn_v302de_230);
	memcpy(&g_fn_v302de_230, exe_buf + 0x01cf26, sizeof(g_fn_v302de_230));
	STATIC_ASSERT(sizeof(g_fn_v302de_231) <= 9, size_check_g_fn_v302de_231);
	memcpy(&g_fn_v302de_231, exe_buf + 0x01cf32, sizeof(g_fn_v302de_231));
	STATIC_ASSERT(sizeof(g_fn_v302de_232) <= 11, size_check_g_fn_v302de_232);
	memcpy(&g_fn_v302de_232, exe_buf + 0x01cf3b, sizeof(g_fn_v302de_232));
	STATIC_ASSERT(sizeof(g_fn_v302de_233) <= 10, size_check_g_fn_v302de_233);
	memcpy(&g_fn_v302de_233, exe_buf + 0x01cf46, sizeof(g_fn_v302de_233));
	STATIC_ASSERT(sizeof(g_fn_v302de_234) <= 6, size_check_g_fn_v302de_234);
	memcpy(&g_fn_v302de_234, exe_buf + 0x01cf50, sizeof(g_fn_v302de_234));
	STATIC_ASSERT(sizeof(g_fn_v302de_235) <= 10, size_check_g_fn_v302de_235);
	memcpy(&g_fn_v302de_235, exe_buf + 0x01cf56, sizeof(g_fn_v302de_235));
	STATIC_ASSERT(sizeof(g_fn_v302de_236) <= 13, size_check_g_fn_v302de_236);
	memcpy(&g_fn_v302de_236, exe_buf + 0x01cf60, sizeof(g_fn_v302de_236));
	STATIC_ASSERT(sizeof(g_fn_v302de_237) <= 13, size_check_g_fn_v302de_237);
	memcpy(&g_fn_v302de_237, exe_buf + 0x01cf6d, sizeof(g_fn_v302de_237));
	STATIC_ASSERT(sizeof(g_fn_v302de_238) <= 10, size_check_g_fn_v302de_238);
	memcpy(&g_fn_v302de_238, exe_buf + 0x01cf7a, sizeof(g_fn_v302de_238));
	STATIC_ASSERT(sizeof(g_fn_v302de_239) <= 10, size_check_g_fn_v302de_239);
	memcpy(&g_fn_v302de_239, exe_buf + 0x01cf84, sizeof(g_fn_v302de_239));
	STATIC_ASSERT(sizeof(g_fn_v302de_240) <= 9, size_check_g_fn_v302de_240);
	memcpy(&g_fn_v302de_240, exe_buf + 0x01cf8e, sizeof(g_fn_v302de_240));
	STATIC_ASSERT(sizeof(g_fn_v302de_241) <= 13, size_check_g_fn_v302de_241);
	memcpy(&g_fn_v302de_241, exe_buf + 0x01cf97, sizeof(g_fn_v302de_241));
	STATIC_ASSERT(sizeof(g_fn_v302de_242) <= 9, size_check_g_fn_v302de_242);
	memcpy(&g_fn_v302de_242, exe_buf + 0x01cfa4, sizeof(g_fn_v302de_242));
	STATIC_ASSERT(sizeof(g_fn_v302de_243) <= 9, size_check_g_fn_v302de_243);
	memcpy(&g_fn_v302de_243, exe_buf + 0x01cfad, sizeof(g_fn_v302de_243));
	STATIC_ASSERT(sizeof(g_fn_v302de_244) <= 9, size_check_g_fn_v302de_244);
	memcpy(&g_fn_v302de_244, exe_buf + 0x01cfb6, sizeof(g_fn_v302de_244));
	STATIC_ASSERT(sizeof(g_fn_v302de_245) <= 9, size_check_g_fn_v302de_245);
	memcpy(&g_fn_v302de_245, exe_buf + 0x01cfbf, sizeof(g_fn_v302de_245));
	STATIC_ASSERT(sizeof(g_fn_v302de_246) <= 9, size_check_g_fn_v302de_246);
	memcpy(&g_fn_v302de_246, exe_buf + 0x01cfc8, sizeof(g_fn_v302de_246));
	STATIC_ASSERT(sizeof(g_fn_v302de_247) <= 9, size_check_g_fn_v302de_247);
	memcpy(&g_fn_v302de_247, exe_buf + 0x01cfd1, sizeof(g_fn_v302de_247));
	STATIC_ASSERT(sizeof(g_fn_v302de_248) <= 9, size_check_g_fn_v302de_248);
	memcpy(&g_fn_v302de_248, exe_buf + 0x01cfda, sizeof(g_fn_v302de_248));
	STATIC_ASSERT(sizeof(g_fn_v302de_249) <= 9, size_check_g_fn_v302de_249);
	memcpy(&g_fn_v302de_249, exe_buf + 0x01cfe3, sizeof(g_fn_v302de_249));
	STATIC_ASSERT(sizeof(g_fn_v302de_250) <= 9, size_check_g_fn_v302de_250);
	memcpy(&g_fn_v302de_250, exe_buf + 0x01cfec, sizeof(g_fn_v302de_250));
	STATIC_ASSERT(sizeof(g_fn_v302de_251) <= 9, size_check_g_fn_v302de_251);
	memcpy(&g_fn_v302de_251, exe_buf + 0x01cff5, sizeof(g_fn_v302de_251));
	STATIC_ASSERT(sizeof(g_fn_v302de_252) <= 9, size_check_g_fn_v302de_252);
	memcpy(&g_fn_v302de_252, exe_buf + 0x01cffe, sizeof(g_fn_v302de_252));
	STATIC_ASSERT(sizeof(g_fn_v302de_253) <= 9, size_check_g_fn_v302de_253);
	memcpy(&g_fn_v302de_253, exe_buf + 0x01d007, sizeof(g_fn_v302de_253));
	STATIC_ASSERT(sizeof(g_fn_v302de_254) <= 9, size_check_g_fn_v302de_254);
	memcpy(&g_fn_v302de_254, exe_buf + 0x01d010, sizeof(g_fn_v302de_254));
	STATIC_ASSERT(sizeof(g_fn_v302de_255) <= 12, size_check_g_fn_v302de_255);
	memcpy(&g_fn_v302de_255, exe_buf + 0x01d019, sizeof(g_fn_v302de_255));
	STATIC_ASSERT(sizeof(g_fn_v302de_256) <= 10, size_check_g_fn_v302de_256);
	memcpy(&g_fn_v302de_256, exe_buf + 0x01d025, sizeof(g_fn_v302de_256));
	STATIC_ASSERT(sizeof(g_fn_v302de_257) <= 9, size_check_g_fn_v302de_257);
	memcpy(&g_fn_v302de_257, exe_buf + 0x01d02f, sizeof(g_fn_v302de_257));
	STATIC_ASSERT(sizeof(g_fn_v302de_258) <= 9, size_check_g_fn_v302de_258);
	memcpy(&g_fn_v302de_258, exe_buf + 0x01d038, sizeof(g_fn_v302de_258));
	STATIC_ASSERT(sizeof(g_fn_v302de_259) <= 9, size_check_g_fn_v302de_259);
	memcpy(&g_fn_v302de_259, exe_buf + 0x01d041, sizeof(g_fn_v302de_259));
	STATIC_ASSERT(sizeof(g_fn_v302de_260) <= 9, size_check_g_fn_v302de_260);
	memcpy(&g_fn_v302de_260, exe_buf + 0x01d04a, sizeof(g_fn_v302de_260));
	STATIC_ASSERT(sizeof(g_fn_v302de_261) <= 9, size_check_g_fn_v302de_261);
	memcpy(&g_fn_v302de_261, exe_buf + 0x01d053, sizeof(g_fn_v302de_261));
	STATIC_ASSERT(sizeof(g_fn_v302de_262) <= 9, size_check_g_fn_v302de_262);
	memcpy(&g_fn_v302de_262, exe_buf + 0x01d05c, sizeof(g_fn_v302de_262));
	STATIC_ASSERT(sizeof(g_fn_v302de_263) <= 9, size_check_g_fn_v302de_263);
	memcpy(&g_fn_v302de_263, exe_buf + 0x01d065, sizeof(g_fn_v302de_263));
	STATIC_ASSERT(sizeof(g_fn_v302de_264) <= 9, size_check_g_fn_v302de_264);
	memcpy(&g_fn_v302de_264, exe_buf + 0x01d06e, sizeof(g_fn_v302de_264));
	STATIC_ASSERT(sizeof(g_fn_v302de_265) <= 9, size_check_g_fn_v302de_265);
	memcpy(&g_fn_v302de_265, exe_buf + 0x01d077, sizeof(g_fn_v302de_265));
	STATIC_ASSERT(sizeof(g_fn_v302de_266) <= 9, size_check_g_fn_v302de_266);
	memcpy(&g_fn_v302de_266, exe_buf + 0x01d080, sizeof(g_fn_v302de_266));
	STATIC_ASSERT(sizeof(g_fn_v302de_267) <= 9, size_check_g_fn_v302de_267);
	memcpy(&g_fn_v302de_267, exe_buf + 0x01d089, sizeof(g_fn_v302de_267));
	STATIC_ASSERT(sizeof(g_fn_v302de_268) <= 9, size_check_g_fn_v302de_268);
	memcpy(&g_fn_v302de_268, exe_buf + 0x01d092, sizeof(g_fn_v302de_268));
	STATIC_ASSERT(sizeof(g_fn_v302de_269) <= 9, size_check_g_fn_v302de_269);
	memcpy(&g_fn_v302de_269, exe_buf + 0x01d09b, sizeof(g_fn_v302de_269));
	STATIC_ASSERT(sizeof(g_fn_v302de_270) <= 12, size_check_g_fn_v302de_270);
	memcpy(&g_fn_v302de_270, exe_buf + 0x01d0a4, sizeof(g_fn_v302de_270));
	STATIC_ASSERT(sizeof(g_fn_v302de_271) <= 10, size_check_g_fn_v302de_271);
	memcpy(&g_fn_v302de_271, exe_buf + 0x01d0b0, sizeof(g_fn_v302de_271));
	STATIC_ASSERT(sizeof(g_fn_v302de_272) <= 9, size_check_g_fn_v302de_272);
	memcpy(&g_fn_v302de_272, exe_buf + 0x01d0ba, sizeof(g_fn_v302de_272));
	STATIC_ASSERT(sizeof(g_fn_v302de_273) <= 9, size_check_g_fn_v302de_273);
	memcpy(&g_fn_v302de_273, exe_buf + 0x01d0c3, sizeof(g_fn_v302de_273));
	STATIC_ASSERT(sizeof(g_fn_v302de_274) <= 9, size_check_g_fn_v302de_274);
	memcpy(&g_fn_v302de_274, exe_buf + 0x01d0cc, sizeof(g_fn_v302de_274));
	STATIC_ASSERT(sizeof(g_fn_v302de_275) <= 9, size_check_g_fn_v302de_275);
	memcpy(&g_fn_v302de_275, exe_buf + 0x01d0d5, sizeof(g_fn_v302de_275));
	STATIC_ASSERT(sizeof(g_fn_v302de_276) <= 9, size_check_g_fn_v302de_276);
	memcpy(&g_fn_v302de_276, exe_buf + 0x01d0de, sizeof(g_fn_v302de_276));
	STATIC_ASSERT(sizeof(g_fn_v302de_277) <= 9, size_check_g_fn_v302de_277);
	memcpy(&g_fn_v302de_277, exe_buf + 0x01d0e7, sizeof(g_fn_v302de_277));
	STATIC_ASSERT(sizeof(g_fn_v302de_278) <= 9, size_check_g_fn_v302de_278);
	memcpy(&g_fn_v302de_278, exe_buf + 0x01d0f0, sizeof(g_fn_v302de_278));
	STATIC_ASSERT(sizeof(g_fn_v302de_279) <= 9, size_check_g_fn_v302de_279);
	memcpy(&g_fn_v302de_279, exe_buf + 0x01d0f9, sizeof(g_fn_v302de_279));
	STATIC_ASSERT(sizeof(g_fn_v302de_280) <= 9, size_check_g_fn_v302de_280);
	memcpy(&g_fn_v302de_280, exe_buf + 0x01d102, sizeof(g_fn_v302de_280));
	STATIC_ASSERT(sizeof(g_fn_v302de_281) <= 9, size_check_g_fn_v302de_281);
	memcpy(&g_fn_v302de_281, exe_buf + 0x01d10b, sizeof(g_fn_v302de_281));
	STATIC_ASSERT(sizeof(g_fn_v302de_282) <= 9, size_check_g_fn_v302de_282);
	memcpy(&g_fn_v302de_282, exe_buf + 0x01d114, sizeof(g_fn_v302de_282));
	STATIC_ASSERT(sizeof(g_fn_v302de_283) <= 9, size_check_g_fn_v302de_283);
	memcpy(&g_fn_v302de_283, exe_buf + 0x01d11d, sizeof(g_fn_v302de_283));
	STATIC_ASSERT(sizeof(g_fn_v302de_284) <= 9, size_check_g_fn_v302de_284);
	memcpy(&g_fn_v302de_284, exe_buf + 0x01d126, sizeof(g_fn_v302de_284));
	STATIC_ASSERT(sizeof(g_fn_v302de_285) <= 12, size_check_g_fn_v302de_285);
	memcpy(&g_fn_v302de_285, exe_buf + 0x01d12f, sizeof(g_fn_v302de_285));
	STATIC_ASSERT(sizeof(g_fn_v302de_286) <= 10, size_check_g_fn_v302de_286);
	memcpy(&g_fn_v302de_286, exe_buf + 0x01d13b, sizeof(g_fn_v302de_286));
	STATIC_ASSERT(sizeof(g_fn_v302de_287) <= 12, size_check_g_fn_v302de_287);
	memcpy(&g_fn_v302de_287, exe_buf + 0x01d145, sizeof(g_fn_v302de_287));
	STATIC_ASSERT(sizeof(g_fn_v302de_288) <= 10, size_check_g_fn_v302de_288);
	memcpy(&g_fn_v302de_288, exe_buf + 0x01d151, sizeof(g_fn_v302de_288));
	STATIC_ASSERT(sizeof(g_fn_v302de_289) <= 8, size_check_g_fn_v302de_289);
	memcpy(&g_fn_v302de_289, exe_buf + 0x01d15b, sizeof(g_fn_v302de_289));
	STATIC_ASSERT(sizeof(g_fn_v302de_290) <= 8, size_check_g_fn_v302de_290);
	memcpy(&g_fn_v302de_290, exe_buf + 0x01d163, sizeof(g_fn_v302de_290));
	STATIC_ASSERT(sizeof(g_fn_v302de_291) <= 8, size_check_g_fn_v302de_291);
	memcpy(&g_fn_v302de_291, exe_buf + 0x01d16b, sizeof(g_fn_v302de_291));
	STATIC_ASSERT(sizeof(g_fn_v302de_292) <= 8, size_check_g_fn_v302de_292);
	memcpy(&g_fn_v302de_292, exe_buf + 0x01d173, sizeof(g_fn_v302de_292));
	STATIC_ASSERT(sizeof(g_fn_v302de_293) <= 8, size_check_g_fn_v302de_293);
	memcpy(&g_fn_v302de_293, exe_buf + 0x01d17b, sizeof(g_fn_v302de_293));
	STATIC_ASSERT(sizeof(g_fn_v302de_294) <= 8, size_check_g_fn_v302de_294);
	memcpy(&g_fn_v302de_294, exe_buf + 0x01d183, sizeof(g_fn_v302de_294));
	STATIC_ASSERT(sizeof(g_fn_v302de_295) <= 8, size_check_g_fn_v302de_295);
	memcpy(&g_fn_v302de_295, exe_buf + 0x01d18b, sizeof(g_fn_v302de_295));
	STATIC_ASSERT(sizeof(g_fn_v302de_296) <= 8, size_check_g_fn_v302de_296);
	memcpy(&g_fn_v302de_296, exe_buf + 0x01d193, sizeof(g_fn_v302de_296));
	STATIC_ASSERT(sizeof(g_fn_v302de_297) <= 8, size_check_g_fn_v302de_297);
	memcpy(&g_fn_v302de_297, exe_buf + 0x01d19b, sizeof(g_fn_v302de_297));
	STATIC_ASSERT(sizeof(g_fn_v302de_298) <= 9, size_check_g_fn_v302de_298);
	memcpy(&g_fn_v302de_298, exe_buf + 0x01d1a3, sizeof(g_fn_v302de_298));
	STATIC_ASSERT(sizeof(g_fn_v302de_299) <= 9, size_check_g_fn_v302de_299);
	memcpy(&g_fn_v302de_299, exe_buf + 0x01d1ac, sizeof(g_fn_v302de_299));
	STATIC_ASSERT(sizeof(g_fn_v302de_300) <= 9, size_check_g_fn_v302de_300);
	memcpy(&g_fn_v302de_300, exe_buf + 0x01d1b5, sizeof(g_fn_v302de_300));
	STATIC_ASSERT(sizeof(g_fn_v302de_301) <= 9, size_check_g_fn_v302de_301);
	memcpy(&g_fn_v302de_301, exe_buf + 0x01d1be, sizeof(g_fn_v302de_301));
	STATIC_ASSERT(sizeof(g_fn_v302de_302) <= 9, size_check_g_fn_v302de_302);
	memcpy(&g_fn_v302de_302, exe_buf + 0x01d1c7, sizeof(g_fn_v302de_302));
	STATIC_ASSERT(sizeof(g_fn_v302de_303) <= 9, size_check_g_fn_v302de_303);
	memcpy(&g_fn_v302de_303, exe_buf + 0x01d1d0, sizeof(g_fn_v302de_303));
	STATIC_ASSERT(sizeof(g_fn_v302de_304) <= 9, size_check_g_fn_v302de_304);
	memcpy(&g_fn_v302de_304, exe_buf + 0x01d1d9, sizeof(g_fn_v302de_304));
	STATIC_ASSERT(sizeof(g_fn_v302de_305) <= 9, size_check_g_fn_v302de_305);
	memcpy(&g_fn_v302de_305, exe_buf + 0x01d1e2, sizeof(g_fn_v302de_305));
	STATIC_ASSERT(sizeof(g_fn_v302de_306) <= 9, size_check_g_fn_v302de_306);
	memcpy(&g_fn_v302de_306, exe_buf + 0x01d1eb, sizeof(g_fn_v302de_306));
	STATIC_ASSERT(sizeof(g_fn_v302de_307) <= 10, size_check_g_fn_v302de_307);
	memcpy(&g_fn_v302de_307, exe_buf + 0x01d1f4, sizeof(g_fn_v302de_307));
	STATIC_ASSERT(sizeof(g_ranged_weapon_stats_table) <= 72, size_check_g_ranged_weapon_stats_table);
	memcpy(&g_ranged_weapon_stats_table, exe_buf + 0x017a28, sizeof(g_ranged_weapon_stats_table));
	STATIC_ASSERT(sizeof(g_weapon_stats_table) <= 455, size_check_g_weapon_stats_table);
	memcpy(&g_weapon_stats_table, exe_buf + 0x017a70, sizeof(g_weapon_stats_table));
	STATIC_ASSERT(sizeof(g_monster_name_genders) <= 78, size_check_g_monster_name_genders);
	memcpy(&g_monster_name_genders, exe_buf + 0x017ce5, sizeof(g_monster_name_genders));
	STATIC_ASSERT(sizeof(g_staffspell_descriptions) <= 42, size_check_g_staffspell_descriptions);
	memcpy(&g_staffspell_descriptions, exe_buf + 0x017d33, sizeof(g_staffspell_descriptions));
	STATIC_ASSERT(sizeof(g_spell_descriptions) <= 886, size_check_g_spell_descriptions);
	memcpy(&g_spell_descriptions, exe_buf + 0x017d5d, sizeof(g_spell_descriptions));
	STATIC_ASSERT(sizeof(g_mon_spell_descriptions) <= 120, size_check_g_mon_spell_descriptions);
	memcpy(&g_mon_spell_descriptions, exe_buf + 0x0182d3, sizeof(g_mon_spell_descriptions));
	STATIC_ASSERT(sizeof(g_talent_descriptions) <= 208, size_check_g_talent_descriptions);
	memcpy(&g_talent_descriptions, exe_buf + 0x0183be, sizeof(g_talent_descriptions));
	STATIC_ASSERT(sizeof(g_talentclasses) <= 14, size_check_g_talentclasses);
	memcpy(&g_talentclasses, exe_buf + 0x01848e, sizeof(g_talentclasses));
	STATIC_ASSERT(sizeof(g_nvftab_figures_rangeweapon) <= 264, size_check_g_nvftab_figures_rangeweapon);
	memcpy(&g_nvftab_figures_rangeweapon, exe_buf + 0x01849c, sizeof(g_nvftab_figures_rangeweapon));
	STATIC_ASSERT(sizeof(g_nvftab_figures_unconscious) <= 44, size_check_g_nvftab_figures_unconscious);
	memcpy(&g_nvftab_figures_unconscious, exe_buf + 0x0185a4, sizeof(g_nvftab_figures_unconscious));
	STATIC_ASSERT(sizeof(g_gfxtab_offsets_unconscious) <= 176, size_check_g_gfxtab_offsets_unconscious);
	memcpy(&g_gfxtab_offsets_unconscious, exe_buf + 0x0185d0, sizeof(g_gfxtab_offsets_unconscious));
	STATIC_ASSERT(sizeof(g_gfxtab_figures_main) <= 625, size_check_g_gfxtab_figures_main);
	memcpy(&g_gfxtab_figures_main, exe_buf + 0x018680, sizeof(g_gfxtab_figures_main));
	STATIC_ASSERT(sizeof(g_gfxtab_offsets_main) <= 1250, size_check_g_gfxtab_offsets_main);
	memcpy(&g_gfxtab_offsets_main, exe_buf + 0x0188f1, sizeof(g_gfxtab_offsets_main));
	STATIC_ASSERT(sizeof(g_unkn_005) <= 38, size_check_g_unkn_005);
	memcpy(&g_unkn_005, exe_buf + 0x018dff, sizeof(g_unkn_005));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_01) <= 130, size_check_g_gfx_ani_descriptions_01);
	memcpy(&g_gfx_ani_descriptions_01, exe_buf + 0x018e25, sizeof(g_gfx_ani_descriptions_01));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_02) <= 130, size_check_g_gfx_ani_descriptions_02);
	memcpy(&g_gfx_ani_descriptions_02, exe_buf + 0x018ea7, sizeof(g_gfx_ani_descriptions_02));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_03) <= 130, size_check_g_gfx_ani_descriptions_03);
	memcpy(&g_gfx_ani_descriptions_03, exe_buf + 0x018f29, sizeof(g_gfx_ani_descriptions_03));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_04) <= 130, size_check_g_gfx_ani_descriptions_04);
	memcpy(&g_gfx_ani_descriptions_04, exe_buf + 0x018fab, sizeof(g_gfx_ani_descriptions_04));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_05) <= 130, size_check_g_gfx_ani_descriptions_05);
	memcpy(&g_gfx_ani_descriptions_05, exe_buf + 0x01902d, sizeof(g_gfx_ani_descriptions_05));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_06) <= 130, size_check_g_gfx_ani_descriptions_06);
	memcpy(&g_gfx_ani_descriptions_06, exe_buf + 0x0190af, sizeof(g_gfx_ani_descriptions_06));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_07) <= 130, size_check_g_gfx_ani_descriptions_07);
	memcpy(&g_gfx_ani_descriptions_07, exe_buf + 0x019131, sizeof(g_gfx_ani_descriptions_07));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_08) <= 130, size_check_g_gfx_ani_descriptions_08);
	memcpy(&g_gfx_ani_descriptions_08, exe_buf + 0x0191b3, sizeof(g_gfx_ani_descriptions_08));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_09) <= 130, size_check_g_gfx_ani_descriptions_09);
	memcpy(&g_gfx_ani_descriptions_09, exe_buf + 0x019235, sizeof(g_gfx_ani_descriptions_09));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_10) <= 130, size_check_g_gfx_ani_descriptions_10);
	memcpy(&g_gfx_ani_descriptions_10, exe_buf + 0x0192b7, sizeof(g_gfx_ani_descriptions_10));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_11) <= 130, size_check_g_gfx_ani_descriptions_11);
	memcpy(&g_gfx_ani_descriptions_11, exe_buf + 0x019339, sizeof(g_gfx_ani_descriptions_11));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_12) <= 130, size_check_g_gfx_ani_descriptions_12);
	memcpy(&g_gfx_ani_descriptions_12, exe_buf + 0x0193bb, sizeof(g_gfx_ani_descriptions_12));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_13) <= 130, size_check_g_gfx_ani_descriptions_13);
	memcpy(&g_gfx_ani_descriptions_13, exe_buf + 0x01943d, sizeof(g_gfx_ani_descriptions_13));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_14) <= 74, size_check_g_gfx_ani_descriptions_14);
	memcpy(&g_gfx_ani_descriptions_14, exe_buf + 0x0194bf, sizeof(g_gfx_ani_descriptions_14));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_15) <= 74, size_check_g_gfx_ani_descriptions_15);
	memcpy(&g_gfx_ani_descriptions_15, exe_buf + 0x019509, sizeof(g_gfx_ani_descriptions_15));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_16) <= 74, size_check_g_gfx_ani_descriptions_16);
	memcpy(&g_gfx_ani_descriptions_16, exe_buf + 0x019553, sizeof(g_gfx_ani_descriptions_16));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_17) <= 74, size_check_g_gfx_ani_descriptions_17);
	memcpy(&g_gfx_ani_descriptions_17, exe_buf + 0x01959d, sizeof(g_gfx_ani_descriptions_17));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_18) <= 74, size_check_g_gfx_ani_descriptions_18);
	memcpy(&g_gfx_ani_descriptions_18, exe_buf + 0x0195e7, sizeof(g_gfx_ani_descriptions_18));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_19) <= 74, size_check_g_gfx_ani_descriptions_19);
	memcpy(&g_gfx_ani_descriptions_19, exe_buf + 0x019631, sizeof(g_gfx_ani_descriptions_19));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_20) <= 74, size_check_g_gfx_ani_descriptions_20);
	memcpy(&g_gfx_ani_descriptions_20, exe_buf + 0x01967b, sizeof(g_gfx_ani_descriptions_20));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_21) <= 74, size_check_g_gfx_ani_descriptions_21);
	memcpy(&g_gfx_ani_descriptions_21, exe_buf + 0x0196c5, sizeof(g_gfx_ani_descriptions_21));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_22) <= 74, size_check_g_gfx_ani_descriptions_22);
	memcpy(&g_gfx_ani_descriptions_22, exe_buf + 0x01970f, sizeof(g_gfx_ani_descriptions_22));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_23) <= 74, size_check_g_gfx_ani_descriptions_23);
	memcpy(&g_gfx_ani_descriptions_23, exe_buf + 0x019759, sizeof(g_gfx_ani_descriptions_23));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_24) <= 74, size_check_g_gfx_ani_descriptions_24);
	memcpy(&g_gfx_ani_descriptions_24, exe_buf + 0x0197a3, sizeof(g_gfx_ani_descriptions_24));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_25) <= 74, size_check_g_gfx_ani_descriptions_25);
	memcpy(&g_gfx_ani_descriptions_25, exe_buf + 0x0197ed, sizeof(g_gfx_ani_descriptions_25));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_26) <= 74, size_check_g_gfx_ani_descriptions_26);
	memcpy(&g_gfx_ani_descriptions_26, exe_buf + 0x019837, sizeof(g_gfx_ani_descriptions_26));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_27) <= 74, size_check_g_gfx_ani_descriptions_27);
	memcpy(&g_gfx_ani_descriptions_27, exe_buf + 0x019881, sizeof(g_gfx_ani_descriptions_27));
	STATIC_ASSERT(sizeof(g_gfx_ani_descriptions_28) <= 74, size_check_g_gfx_ani_descriptions_28);
	memcpy(&g_gfx_ani_descriptions_28, exe_buf + 0x0198cb, sizeof(g_gfx_ani_descriptions_28));
	STATIC_ASSERT(sizeof(g_palette_unknown1) <= 96, size_check_g_palette_unknown1);
	memcpy(&g_palette_unknown1, exe_buf + 0x019ae3, sizeof(g_palette_unknown1));
	STATIC_ASSERT(sizeof(g_palette_fight1) <= 96, size_check_g_palette_fight1);
	memcpy(&g_palette_fight1, exe_buf + 0x019b43, sizeof(g_palette_fight1));
	STATIC_ASSERT(sizeof(g_palette_special) <= 96, size_check_g_palette_special);
	memcpy(&g_palette_special, exe_buf + 0x019ba3, sizeof(g_palette_special));
	STATIC_ASSERT(sizeof(g_default_mouse_cursor) <= 64, size_check_g_default_mouse_cursor);
	memcpy(&g_default_mouse_cursor, exe_buf + 0x019c08, sizeof(g_default_mouse_cursor));
	STATIC_ASSERT(sizeof(g_cursor_arrow_up) <= 64, size_check_g_cursor_arrow_up);
	memcpy(&g_cursor_arrow_up, exe_buf + 0x019c48, sizeof(g_cursor_arrow_up));
	STATIC_ASSERT(sizeof(g_cursor_arrow_down) <= 64, size_check_g_cursor_arrow_down);
	memcpy(&g_cursor_arrow_down, exe_buf + 0x019c88, sizeof(g_cursor_arrow_down));
	STATIC_ASSERT(sizeof(g_cursor_arrow_left) <= 64, size_check_g_cursor_arrow_left);
	memcpy(&g_cursor_arrow_left, exe_buf + 0x019cc8, sizeof(g_cursor_arrow_left));
	STATIC_ASSERT(sizeof(g_cursor_arrow_right) <= 64, size_check_g_cursor_arrow_right);
	memcpy(&g_cursor_arrow_right, exe_buf + 0x019d08, sizeof(g_cursor_arrow_right));
	STATIC_ASSERT(sizeof(gs_daspota_location01_loot) <= 29, size_check_gs_daspota_location01_loot);
	memcpy(&gs_daspota_location01_loot, exe_buf + 0x01b08e, sizeof(gs_daspota_location01_loot));
	STATIC_ASSERT(sizeof(gs_daspota_location02_loot) <= 15, size_check_gs_daspota_location02_loot);
	memcpy(&gs_daspota_location02_loot, exe_buf + 0x01b0ab, sizeof(gs_daspota_location02_loot));
	STATIC_ASSERT(sizeof(gs_daspota_location03_loot) <= 15, size_check_gs_daspota_location03_loot);
	memcpy(&gs_daspota_location03_loot, exe_buf + 0x01b0ba, sizeof(gs_daspota_location03_loot));
	STATIC_ASSERT(sizeof(gs_daspota_location04_loot) <= 9, size_check_gs_daspota_location04_loot);
	memcpy(&gs_daspota_location04_loot, exe_buf + 0x01b0c9, sizeof(gs_daspota_location04_loot));
	STATIC_ASSERT(sizeof(gs_daspota_location05_loot) <= 31, size_check_gs_daspota_location05_loot);
	memcpy(&gs_daspota_location05_loot, exe_buf + 0x01b0d2, sizeof(gs_daspota_location05_loot));
	STATIC_ASSERT(sizeof(gs_daspota_location06_loot) <= 25, size_check_gs_daspota_location06_loot);
	memcpy(&gs_daspota_location06_loot, exe_buf + 0x01b0f1, sizeof(gs_daspota_location06_loot));
	STATIC_ASSERT(sizeof(gs_daspota_location07_loot) <= 7, size_check_gs_daspota_location07_loot);
	memcpy(&gs_daspota_location07_loot, exe_buf + 0x01b10a, sizeof(gs_daspota_location07_loot));
	STATIC_ASSERT(sizeof(gs_daspota_location08_loot) <= 15, size_check_gs_daspota_location08_loot);
	memcpy(&gs_daspota_location08_loot, exe_buf + 0x01b111, sizeof(gs_daspota_location08_loot));
	STATIC_ASSERT(sizeof(gs_daspota_location10_loot) <= 7, size_check_gs_daspota_location10_loot);
	memcpy(&gs_daspota_location10_loot, exe_buf + 0x01b120, sizeof(gs_daspota_location10_loot));
	STATIC_ASSERT(sizeof(gs_daspota_location12_loot) <= 21, size_check_gs_daspota_location12_loot);
	memcpy(&gs_daspota_location12_loot, exe_buf + 0x01b127, sizeof(gs_daspota_location12_loot));
	STATIC_ASSERT(sizeof(gs_daspota_location13_loot) <= 9, size_check_gs_daspota_location13_loot);
	memcpy(&gs_daspota_location13_loot, exe_buf + 0x01b13c, sizeof(gs_daspota_location13_loot));
	STATIC_ASSERT(sizeof(gs_daspota_location14_loot) <= 5, size_check_gs_daspota_location14_loot);
	memcpy(&gs_daspota_location14_loot, exe_buf + 0x01b145, sizeof(gs_daspota_location14_loot));
	STATIC_ASSERT(sizeof(gs_daspota_location16_loot) <= 5, size_check_gs_daspota_location16_loot);
	memcpy(&gs_daspota_location16_loot, exe_buf + 0x01b14a, sizeof(gs_daspota_location16_loot));
	STATIC_ASSERT(sizeof(gs_daspota_location18_loot) <= 19, size_check_gs_daspota_location18_loot);
	memcpy(&gs_daspota_location18_loot, exe_buf + 0x01b14f, sizeof(gs_daspota_location18_loot));
	STATIC_ASSERT(sizeof(gs_dng14_chest_x1) <= 7, size_check_gs_dng14_chest_x1);
	memcpy(&gs_dng14_chest_x1, exe_buf + 0x01b3a4, sizeof(gs_dng14_chest_x1));
	STATIC_ASSERT(sizeof(gs_dng14_chest_x2) <= 16, size_check_gs_dng14_chest_x2);
	memcpy(&gs_dng14_chest_x2, exe_buf + 0x01b3ab, sizeof(gs_dng14_chest_x2));
	STATIC_ASSERT(sizeof(gs_dng14_chest_x3) <= 11, size_check_gs_dng14_chest_x3);
	memcpy(&gs_dng14_chest_x3, exe_buf + 0x01b3bb, sizeof(gs_dng14_chest_x3));
	STATIC_ASSERT(sizeof(gs_dng14_chest_x4) <= 14, size_check_gs_dng14_chest_x4);
	memcpy(&gs_dng14_chest_x4, exe_buf + 0x01b3c6, sizeof(gs_dng14_chest_x4));
	STATIC_ASSERT(sizeof(gs_dng14_chest_x5) <= 10, size_check_gs_dng14_chest_x5);
	memcpy(&gs_dng14_chest_x5, exe_buf + 0x01b3d4, sizeof(gs_dng14_chest_x5));
	STATIC_ASSERT(sizeof(gs_dng14_chest_x6) <= 8, size_check_gs_dng14_chest_x6);
	memcpy(&gs_dng14_chest_x6, exe_buf + 0x01b3de, sizeof(gs_dng14_chest_x6));
	STATIC_ASSERT(sizeof(gs_dng14_chest_x7) <= 7, size_check_gs_dng14_chest_x7);
	memcpy(&gs_dng14_chest_x7, exe_buf + 0x01b3e6, sizeof(gs_dng14_chest_x7));
	STATIC_ASSERT(sizeof(gs_dng14_chest_x8) <= 13, size_check_gs_dng14_chest_x8);
	memcpy(&gs_dng14_chest_x8, exe_buf + 0x01b3ed, sizeof(gs_dng14_chest_x8));
	STATIC_ASSERT(sizeof(gs_dng02_chest00_content) <= 2, size_check_gs_dng02_chest00_content);
	memcpy(&gs_dng02_chest00_content, exe_buf + 0x01b3fb, sizeof(gs_dng02_chest00_content));
	STATIC_ASSERT(sizeof(gs_dng02_chest01_content) <= 4, size_check_gs_dng02_chest01_content);
	memcpy(&gs_dng02_chest01_content, exe_buf + 0x01b3fd, sizeof(gs_dng02_chest01_content));
	STATIC_ASSERT(sizeof(gs_dng02_chest06_content) <= 4, size_check_gs_dng02_chest06_content);
	memcpy(&gs_dng02_chest06_content, exe_buf + 0x01b401, sizeof(gs_dng02_chest06_content));
	STATIC_ASSERT(sizeof(gs_dng02_chest02_content) <= 8, size_check_gs_dng02_chest02_content);
	memcpy(&gs_dng02_chest02_content, exe_buf + 0x01b405, sizeof(gs_dng02_chest02_content));
	STATIC_ASSERT(sizeof(gs_dng02_chest03_content) <= 8, size_check_gs_dng02_chest03_content);
	memcpy(&gs_dng02_chest03_content, exe_buf + 0x01b40d, sizeof(gs_dng02_chest03_content));
	STATIC_ASSERT(sizeof(gs_dng02_chest05_content) <= 8, size_check_gs_dng02_chest05_content);
	memcpy(&gs_dng02_chest05_content, exe_buf + 0x01b415, sizeof(gs_dng02_chest05_content));
	STATIC_ASSERT(sizeof(gs_dng03_chest00_content) <= 7, size_check_gs_dng03_chest00_content);
	memcpy(&gs_dng03_chest00_content, exe_buf + 0x01b41d, sizeof(gs_dng03_chest00_content));
	STATIC_ASSERT(sizeof(gs_dng03_chest01_content) <= 8, size_check_gs_dng03_chest01_content);
	memcpy(&gs_dng03_chest01_content, exe_buf + 0x01b424, sizeof(gs_dng03_chest01_content));
	STATIC_ASSERT(sizeof(gs_dng03_chest02_content) <= 13, size_check_gs_dng03_chest02_content);
	memcpy(&gs_dng03_chest02_content, exe_buf + 0x01b42c, sizeof(gs_dng03_chest02_content));
	STATIC_ASSERT(sizeof(gs_dng03_chest03_content) <= 3, size_check_gs_dng03_chest03_content);
	memcpy(&gs_dng03_chest03_content, exe_buf + 0x01b439, sizeof(gs_dng03_chest03_content));
	STATIC_ASSERT(sizeof(gs_dng03_chest04_content) <= 9, size_check_gs_dng03_chest04_content);
	memcpy(&gs_dng03_chest04_content, exe_buf + 0x01b43c, sizeof(gs_dng03_chest04_content));
	STATIC_ASSERT(sizeof(gs_dng03_chest06_content) <= 8, size_check_gs_dng03_chest06_content);
	memcpy(&gs_dng03_chest06_content, exe_buf + 0x01b445, sizeof(gs_dng03_chest06_content));
	STATIC_ASSERT(sizeof(gs_dng03_chest07_content) <= 2, size_check_gs_dng03_chest07_content);
	memcpy(&gs_dng03_chest07_content, exe_buf + 0x01b44d, sizeof(gs_dng03_chest07_content));
	STATIC_ASSERT(sizeof(gs_dng03_chest08_content) <= 4, size_check_gs_dng03_chest08_content);
	memcpy(&gs_dng03_chest08_content, exe_buf + 0x01b44f, sizeof(gs_dng03_chest08_content));
	STATIC_ASSERT(sizeof(gs_dng03_chest09_content) <= 11, size_check_gs_dng03_chest09_content);
	memcpy(&gs_dng03_chest09_content, exe_buf + 0x01b453, sizeof(gs_dng03_chest09_content));
	STATIC_ASSERT(sizeof(gs_dng03_chest12_content) <= 7, size_check_gs_dng03_chest12_content);
	memcpy(&gs_dng03_chest12_content, exe_buf + 0x01b45e, sizeof(gs_dng03_chest12_content));
	STATIC_ASSERT(sizeof(gs_dng04_chest_corpse0) <= 4, size_check_gs_dng04_chest_corpse0);
	memcpy(&gs_dng04_chest_corpse0, exe_buf + 0x01b465, sizeof(gs_dng04_chest_corpse0));
	STATIC_ASSERT(sizeof(gs_dng04_chest_corpse1) <= 12, size_check_gs_dng04_chest_corpse1);
	memcpy(&gs_dng04_chest_corpse1, exe_buf + 0x01b469, sizeof(gs_dng04_chest_corpse1));
	STATIC_ASSERT(sizeof(gs_dng04_chest_corpse2) <= 10, size_check_gs_dng04_chest_corpse2);
	memcpy(&gs_dng04_chest_corpse2, exe_buf + 0x01b475, sizeof(gs_dng04_chest_corpse2));
	STATIC_ASSERT(sizeof(gs_dng11_chest3) <= 15, size_check_gs_dng11_chest3);
	memcpy(&gs_dng11_chest3, exe_buf + 0x01b52b, sizeof(gs_dng11_chest3));
	STATIC_ASSERT(sizeof(gs_dng11_chest4) <= 11, size_check_gs_dng11_chest4);
	memcpy(&gs_dng11_chest4, exe_buf + 0x01b53a, sizeof(gs_dng11_chest4));
	STATIC_ASSERT(sizeof(gs_dng11_chest5) <= 5, size_check_gs_dng11_chest5);
	memcpy(&gs_dng11_chest5, exe_buf + 0x01b545, sizeof(gs_dng11_chest5));
	STATIC_ASSERT(sizeof(gs_dng11_chest6) <= 7, size_check_gs_dng11_chest6);
	memcpy(&gs_dng11_chest6, exe_buf + 0x01b54a, sizeof(gs_dng11_chest6));
	STATIC_ASSERT(sizeof(gs_dng11_chest8) <= 6, size_check_gs_dng11_chest8);
	memcpy(&gs_dng11_chest8, exe_buf + 0x01b552, sizeof(gs_dng11_chest8));
	STATIC_ASSERT(sizeof(gs_dng11_chest9) <= 6, size_check_gs_dng11_chest9);
	memcpy(&gs_dng11_chest9, exe_buf + 0x01b558, sizeof(gs_dng11_chest9));
	STATIC_ASSERT(sizeof(gs_dng11_chest10) <= 11, size_check_gs_dng11_chest10);
	memcpy(&gs_dng11_chest10, exe_buf + 0x01b55e, sizeof(gs_dng11_chest10));
	STATIC_ASSERT(sizeof(gs_dng11_chest11) <= 4, size_check_gs_dng11_chest11);
	memcpy(&gs_dng11_chest11, exe_buf + 0x01b569, sizeof(gs_dng11_chest11));
	STATIC_ASSERT(sizeof(gs_dng11_chest12) <= 13, size_check_gs_dng11_chest12);
	memcpy(&gs_dng11_chest12, exe_buf + 0x01b56d, sizeof(gs_dng11_chest12));
	STATIC_ASSERT(sizeof(gs_dng11_chest13) <= 6, size_check_gs_dng11_chest13);
	memcpy(&gs_dng11_chest13, exe_buf + 0x01b57a, sizeof(gs_dng11_chest13));
	STATIC_ASSERT(sizeof(gs_dng11_chest14) <= 6, size_check_gs_dng11_chest14);
	memcpy(&gs_dng11_chest14, exe_buf + 0x01b580, sizeof(gs_dng11_chest14));
	STATIC_ASSERT(sizeof(g_floor_fade_palette) <= 96, size_check_g_floor_fade_palette);
	memcpy(&g_floor_fade_palette, exe_buf + 0x01b858, sizeof(g_floor_fade_palette));
	STATIC_ASSERT(sizeof(g_building_fade_palette) <= 96, size_check_g_building_fade_palette);
	memcpy(&g_building_fade_palette, exe_buf + 0x01b8b8, sizeof(g_building_fade_palette));
	STATIC_ASSERT(sizeof(g_sky_fade_palette) <= 96, size_check_g_sky_fade_palette);
	memcpy(&g_sky_fade_palette, exe_buf + 0x01b918, sizeof(g_sky_fade_palette));
	STATIC_ASSERT(sizeof(g_special_dates) <= 129, size_check_g_special_dates);
	memcpy(&g_special_dates, exe_buf + 0x01b979, sizeof(g_special_dates));
	STATIC_ASSERT(sizeof(g_level_ap_tab) <= 84, size_check_g_level_ap_tab);
	memcpy(&g_level_ap_tab, exe_buf + 0x01ba0e, sizeof(g_level_ap_tab));
	STATIC_ASSERT(sizeof(g_questions_handbook) <= 190, size_check_g_questions_handbook);
	memcpy(&g_questions_handbook, exe_buf + 0x01babc, sizeof(g_questions_handbook));
	STATIC_ASSERT(sizeof(g_questions_map) <= 30, size_check_g_questions_map);
	memcpy(&g_questions_map, exe_buf + 0x01bb7a, sizeof(g_questions_map));
	STATIC_ASSERT(sizeof(g_visual_square_north) <= 62, size_check_g_visual_square_north);
	memcpy(&g_visual_square_north, exe_buf + 0x01bd30, sizeof(g_visual_square_north));
	STATIC_ASSERT(sizeof(g_visual_square_east) <= 62, size_check_g_visual_square_east);
	memcpy(&g_visual_square_east, exe_buf + 0x01bd6e, sizeof(g_visual_square_east));
	STATIC_ASSERT(sizeof(g_visual_square_south) <= 62, size_check_g_visual_square_south);
	memcpy(&g_visual_square_south, exe_buf + 0x01bdac, sizeof(g_visual_square_south));
	STATIC_ASSERT(sizeof(g_visual_square_west) <= 62, size_check_g_visual_square_west);
	memcpy(&g_visual_square_west, exe_buf + 0x01bdea, sizeof(g_visual_square_west));
	STATIC_ASSERT(sizeof(g_wallclock_pos_y) <= 81, size_check_g_wallclock_pos_y);
	memcpy(&g_wallclock_pos_y, exe_buf + 0x01be60, sizeof(g_wallclock_pos_y));
	STATIC_ASSERT(sizeof(g_informer_tab) <= 60, size_check_g_informer_tab);
	memcpy(&g_informer_tab, exe_buf + 0x01d296, sizeof(g_informer_tab));
	STATIC_ASSERT(sizeof(g_anitab_projectile_data) <= 168, size_check_g_anitab_projectile_data);
	memcpy(&g_anitab_projectile_data, exe_buf + 0x01d5dc, sizeof(g_anitab_projectile_data));
	STATIC_ASSERT(sizeof(g_statuspage_palette) <= 96, size_check_g_statuspage_palette);
	memcpy(&g_statuspage_palette, exe_buf + 0x01d732, sizeof(g_statuspage_palette));
	STATIC_ASSERT(sizeof(g_invslot_iconxy_table) <= 92, size_check_g_invslot_iconxy_table);
	memcpy(&g_invslot_iconxy_table, exe_buf + 0x01d792, sizeof(g_invslot_iconxy_table));
	STATIC_ASSERT(sizeof(g_invslot_borderxy_table) <= 100, size_check_g_invslot_borderxy_table);
	memcpy(&g_invslot_borderxy_table, exe_buf + 0x01d88e, sizeof(g_invslot_borderxy_table));
	STATIC_ASSERT(sizeof(g_gather_herbs_table) <= 52, size_check_g_gather_herbs_table);
	memcpy(&g_gather_herbs_table, exe_buf + 0x01da5c, sizeof(g_gather_herbs_table));
	STATIC_ASSERT(sizeof(g_healer_descr_table) <= 82, size_check_g_healer_descr_table);
	memcpy(&g_healer_descr_table, exe_buf + 0x01daaa, sizeof(g_healer_descr_table));
	STATIC_ASSERT(sizeof(g_inn_descr_table) <= 308, size_check_g_inn_descr_table);
	memcpy(&g_inn_descr_table, exe_buf + 0x01dafc, sizeof(g_inn_descr_table));
	STATIC_ASSERT(sizeof(g_merchant_descr_table) <= 856, size_check_g_merchant_descr_table);
	memcpy(&g_merchant_descr_table, exe_buf + 0x01dc30, sizeof(g_merchant_descr_table));
	STATIC_ASSERT(sizeof(g_smith_descr_table) <= 84, size_check_g_smith_descr_table);
	memcpy(&g_smith_descr_table, exe_buf + 0x01dfd0, sizeof(g_smith_descr_table));
	STATIC_ASSERT(sizeof(g_tavern_descr_table) <= 356, size_check_g_tavern_descr_table);
	memcpy(&g_tavern_descr_table, exe_buf + 0x01e044, sizeof(g_tavern_descr_table));
	STATIC_ASSERT(sizeof(g_sea_routes) <= 368, size_check_g_sea_routes);
	memcpy(&g_sea_routes, exe_buf + 0x01e2c0, sizeof(g_sea_routes));
	STATIC_ASSERT(sizeof(g_market_descr_table) <= 272, size_check_g_market_descr_table);
	memcpy(&g_market_descr_table, exe_buf + 0x01e468, sizeof(g_market_descr_table));
	STATIC_ASSERT(sizeof(g_visual_square_offsets_std) <= 116, size_check_g_visual_square_offsets_std);
	memcpy(&g_visual_square_offsets_std, exe_buf + 0x01e6fa, sizeof(g_visual_square_offsets_std));
	STATIC_ASSERT(sizeof(g_visual_square_offsets_sign) <= 116, size_check_g_visual_square_offsets_sign);
	memcpy(&g_visual_square_offsets_sign, exe_buf + 0x01e76e, sizeof(g_visual_square_offsets_sign));
	STATIC_ASSERT(sizeof(g_visual_square_offsets_inn) <= 116, size_check_g_visual_square_offsets_inn);
	memcpy(&g_visual_square_offsets_inn, exe_buf + 0x01e7e2, sizeof(g_visual_square_offsets_inn));
	STATIC_ASSERT(sizeof(g_visual_square_offsets_grass) <= 116, size_check_g_visual_square_offsets_grass);
	memcpy(&g_visual_square_offsets_grass, exe_buf + 0x01e856, sizeof(g_visual_square_offsets_grass));
	STATIC_ASSERT(sizeof(g_tex_descr_table) <= 1818, size_check_g_tex_descr_table);
	memcpy(&g_tex_descr_table, exe_buf + 0x01e8ca, sizeof(g_tex_descr_table));
	STATIC_ASSERT(sizeof(g_dng_gfxtab_wood) <= 1584, size_check_g_dng_gfxtab_wood);
	memcpy(&g_dng_gfxtab_wood, exe_buf + 0x01f1aa, sizeof(g_dng_gfxtab_wood));
	STATIC_ASSERT(sizeof(g_dng_gfxtab_stone) <= 1584, size_check_g_dng_gfxtab_stone);
	memcpy(&g_dng_gfxtab_stone, exe_buf + 0x01f7da, sizeof(g_dng_gfxtab_stone));
	STATIC_ASSERT(sizeof(g_dng_gfxtab_marble) <= 1584, size_check_g_dng_gfxtab_marble);
	memcpy(&g_dng_gfxtab_marble, exe_buf + 0x01fe0a, sizeof(g_dng_gfxtab_marble));
	STATIC_ASSERT(sizeof(g_dng_square_pos) <= 88, size_check_g_dng_square_pos);
	memcpy(&g_dng_square_pos, exe_buf + 0x020516, sizeof(g_dng_square_pos));
	STATIC_ASSERT(sizeof(g_dng_square_pos_stairs) <= 88, size_check_g_dng_square_pos_stairs);
	memcpy(&g_dng_square_pos_stairs, exe_buf + 0x02056e, sizeof(g_dng_square_pos_stairs));
	STATIC_ASSERT(sizeof(g_dng_square_pos_pit) <= 88, size_check_g_dng_square_pos_pit);
	memcpy(&g_dng_square_pos_pit, exe_buf + 0x0205c6, sizeof(g_dng_square_pos_pit));
	STATIC_ASSERT(sizeof(g_dng_square_pos_pit_ceil) <= 88, size_check_g_dng_square_pos_pit_ceil);
	memcpy(&g_dng_square_pos_pit_ceil, exe_buf + 0x02061e, sizeof(g_dng_square_pos_pit_ceil));
	STATIC_ASSERT(sizeof(g_land_routes) <= 531, size_check_g_land_routes);
	memcpy(&g_land_routes, exe_buf + 0x021186, sizeof(g_land_routes));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_001) <= 2, size_check_g_signposts_linked_lroute_001);
	memcpy(&g_signposts_linked_lroute_001, exe_buf + 0x021399, sizeof(g_signposts_linked_lroute_001));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_002) <= 2, size_check_g_signposts_linked_lroute_002);
	memcpy(&g_signposts_linked_lroute_002, exe_buf + 0x02139b, sizeof(g_signposts_linked_lroute_002));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_003) <= 2, size_check_g_signposts_linked_lroute_003);
	memcpy(&g_signposts_linked_lroute_003, exe_buf + 0x02139d, sizeof(g_signposts_linked_lroute_003));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_004) <= 2, size_check_g_signposts_linked_lroute_004);
	memcpy(&g_signposts_linked_lroute_004, exe_buf + 0x02139f, sizeof(g_signposts_linked_lroute_004));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_005) <= 3, size_check_g_signposts_linked_lroute_005);
	memcpy(&g_signposts_linked_lroute_005, exe_buf + 0x0213a1, sizeof(g_signposts_linked_lroute_005));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_006) <= 2, size_check_g_signposts_linked_lroute_006);
	memcpy(&g_signposts_linked_lroute_006, exe_buf + 0x0213a4, sizeof(g_signposts_linked_lroute_006));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_007) <= 2, size_check_g_signposts_linked_lroute_007);
	memcpy(&g_signposts_linked_lroute_007, exe_buf + 0x0213a6, sizeof(g_signposts_linked_lroute_007));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_008) <= 2, size_check_g_signposts_linked_lroute_008);
	memcpy(&g_signposts_linked_lroute_008, exe_buf + 0x0213a8, sizeof(g_signposts_linked_lroute_008));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_009) <= 2, size_check_g_signposts_linked_lroute_009);
	memcpy(&g_signposts_linked_lroute_009, exe_buf + 0x0213aa, sizeof(g_signposts_linked_lroute_009));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_010) <= 2, size_check_g_signposts_linked_lroute_010);
	memcpy(&g_signposts_linked_lroute_010, exe_buf + 0x0213ac, sizeof(g_signposts_linked_lroute_010));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_011) <= 2, size_check_g_signposts_linked_lroute_011);
	memcpy(&g_signposts_linked_lroute_011, exe_buf + 0x0213ae, sizeof(g_signposts_linked_lroute_011));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_012) <= 2, size_check_g_signposts_linked_lroute_012);
	memcpy(&g_signposts_linked_lroute_012, exe_buf + 0x0213b0, sizeof(g_signposts_linked_lroute_012));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_013) <= 2, size_check_g_signposts_linked_lroute_013);
	memcpy(&g_signposts_linked_lroute_013, exe_buf + 0x0213b2, sizeof(g_signposts_linked_lroute_013));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_014) <= 2, size_check_g_signposts_linked_lroute_014);
	memcpy(&g_signposts_linked_lroute_014, exe_buf + 0x0213b4, sizeof(g_signposts_linked_lroute_014));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_015) <= 2, size_check_g_signposts_linked_lroute_015);
	memcpy(&g_signposts_linked_lroute_015, exe_buf + 0x0213b6, sizeof(g_signposts_linked_lroute_015));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_016) <= 2, size_check_g_signposts_linked_lroute_016);
	memcpy(&g_signposts_linked_lroute_016, exe_buf + 0x0213b8, sizeof(g_signposts_linked_lroute_016));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_017) <= 2, size_check_g_signposts_linked_lroute_017);
	memcpy(&g_signposts_linked_lroute_017, exe_buf + 0x0213ba, sizeof(g_signposts_linked_lroute_017));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_018) <= 2, size_check_g_signposts_linked_lroute_018);
	memcpy(&g_signposts_linked_lroute_018, exe_buf + 0x0213bc, sizeof(g_signposts_linked_lroute_018));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_019) <= 2, size_check_g_signposts_linked_lroute_019);
	memcpy(&g_signposts_linked_lroute_019, exe_buf + 0x0213be, sizeof(g_signposts_linked_lroute_019));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_020) <= 2, size_check_g_signposts_linked_lroute_020);
	memcpy(&g_signposts_linked_lroute_020, exe_buf + 0x0213c0, sizeof(g_signposts_linked_lroute_020));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_021) <= 2, size_check_g_signposts_linked_lroute_021);
	memcpy(&g_signposts_linked_lroute_021, exe_buf + 0x0213c2, sizeof(g_signposts_linked_lroute_021));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_022) <= 2, size_check_g_signposts_linked_lroute_022);
	memcpy(&g_signposts_linked_lroute_022, exe_buf + 0x0213c4, sizeof(g_signposts_linked_lroute_022));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_023) <= 2, size_check_g_signposts_linked_lroute_023);
	memcpy(&g_signposts_linked_lroute_023, exe_buf + 0x0213c6, sizeof(g_signposts_linked_lroute_023));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_024) <= 2, size_check_g_signposts_linked_lroute_024);
	memcpy(&g_signposts_linked_lroute_024, exe_buf + 0x0213c8, sizeof(g_signposts_linked_lroute_024));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_025) <= 2, size_check_g_signposts_linked_lroute_025);
	memcpy(&g_signposts_linked_lroute_025, exe_buf + 0x0213ca, sizeof(g_signposts_linked_lroute_025));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_026) <= 2, size_check_g_signposts_linked_lroute_026);
	memcpy(&g_signposts_linked_lroute_026, exe_buf + 0x0213cc, sizeof(g_signposts_linked_lroute_026));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_027) <= 2, size_check_g_signposts_linked_lroute_027);
	memcpy(&g_signposts_linked_lroute_027, exe_buf + 0x0213ce, sizeof(g_signposts_linked_lroute_027));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_028) <= 2, size_check_g_signposts_linked_lroute_028);
	memcpy(&g_signposts_linked_lroute_028, exe_buf + 0x0213d0, sizeof(g_signposts_linked_lroute_028));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_029) <= 2, size_check_g_signposts_linked_lroute_029);
	memcpy(&g_signposts_linked_lroute_029, exe_buf + 0x0213d2, sizeof(g_signposts_linked_lroute_029));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_030) <= 2, size_check_g_signposts_linked_lroute_030);
	memcpy(&g_signposts_linked_lroute_030, exe_buf + 0x0213d4, sizeof(g_signposts_linked_lroute_030));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_031) <= 2, size_check_g_signposts_linked_lroute_031);
	memcpy(&g_signposts_linked_lroute_031, exe_buf + 0x0213d6, sizeof(g_signposts_linked_lroute_031));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_032) <= 2, size_check_g_signposts_linked_lroute_032);
	memcpy(&g_signposts_linked_lroute_032, exe_buf + 0x0213d8, sizeof(g_signposts_linked_lroute_032));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_033) <= 2, size_check_g_signposts_linked_lroute_033);
	memcpy(&g_signposts_linked_lroute_033, exe_buf + 0x0213da, sizeof(g_signposts_linked_lroute_033));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_034) <= 2, size_check_g_signposts_linked_lroute_034);
	memcpy(&g_signposts_linked_lroute_034, exe_buf + 0x0213dc, sizeof(g_signposts_linked_lroute_034));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_035) <= 2, size_check_g_signposts_linked_lroute_035);
	memcpy(&g_signposts_linked_lroute_035, exe_buf + 0x0213de, sizeof(g_signposts_linked_lroute_035));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_036) <= 2, size_check_g_signposts_linked_lroute_036);
	memcpy(&g_signposts_linked_lroute_036, exe_buf + 0x0213e0, sizeof(g_signposts_linked_lroute_036));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_037) <= 2, size_check_g_signposts_linked_lroute_037);
	memcpy(&g_signposts_linked_lroute_037, exe_buf + 0x0213e2, sizeof(g_signposts_linked_lroute_037));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_038) <= 2, size_check_g_signposts_linked_lroute_038);
	memcpy(&g_signposts_linked_lroute_038, exe_buf + 0x0213e4, sizeof(g_signposts_linked_lroute_038));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_039) <= 2, size_check_g_signposts_linked_lroute_039);
	memcpy(&g_signposts_linked_lroute_039, exe_buf + 0x0213e6, sizeof(g_signposts_linked_lroute_039));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_040) <= 2, size_check_g_signposts_linked_lroute_040);
	memcpy(&g_signposts_linked_lroute_040, exe_buf + 0x0213e8, sizeof(g_signposts_linked_lroute_040));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_041) <= 2, size_check_g_signposts_linked_lroute_041);
	memcpy(&g_signposts_linked_lroute_041, exe_buf + 0x0213ea, sizeof(g_signposts_linked_lroute_041));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_042) <= 2, size_check_g_signposts_linked_lroute_042);
	memcpy(&g_signposts_linked_lroute_042, exe_buf + 0x0213ec, sizeof(g_signposts_linked_lroute_042));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_043) <= 2, size_check_g_signposts_linked_lroute_043);
	memcpy(&g_signposts_linked_lroute_043, exe_buf + 0x0213ee, sizeof(g_signposts_linked_lroute_043));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_044) <= 2, size_check_g_signposts_linked_lroute_044);
	memcpy(&g_signposts_linked_lroute_044, exe_buf + 0x0213f0, sizeof(g_signposts_linked_lroute_044));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_045) <= 2, size_check_g_signposts_linked_lroute_045);
	memcpy(&g_signposts_linked_lroute_045, exe_buf + 0x0213f2, sizeof(g_signposts_linked_lroute_045));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_046) <= 2, size_check_g_signposts_linked_lroute_046);
	memcpy(&g_signposts_linked_lroute_046, exe_buf + 0x0213f4, sizeof(g_signposts_linked_lroute_046));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_047) <= 2, size_check_g_signposts_linked_lroute_047);
	memcpy(&g_signposts_linked_lroute_047, exe_buf + 0x0213f6, sizeof(g_signposts_linked_lroute_047));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_048) <= 6, size_check_g_signposts_linked_lroute_048);
	memcpy(&g_signposts_linked_lroute_048, exe_buf + 0x0213f8, sizeof(g_signposts_linked_lroute_048));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_049) <= 2, size_check_g_signposts_linked_lroute_049);
	memcpy(&g_signposts_linked_lroute_049, exe_buf + 0x0213fe, sizeof(g_signposts_linked_lroute_049));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_050) <= 2, size_check_g_signposts_linked_lroute_050);
	memcpy(&g_signposts_linked_lroute_050, exe_buf + 0x021400, sizeof(g_signposts_linked_lroute_050));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_051) <= 2, size_check_g_signposts_linked_lroute_051);
	memcpy(&g_signposts_linked_lroute_051, exe_buf + 0x021402, sizeof(g_signposts_linked_lroute_051));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_052) <= 2, size_check_g_signposts_linked_lroute_052);
	memcpy(&g_signposts_linked_lroute_052, exe_buf + 0x021404, sizeof(g_signposts_linked_lroute_052));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_053) <= 2, size_check_g_signposts_linked_lroute_053);
	memcpy(&g_signposts_linked_lroute_053, exe_buf + 0x021406, sizeof(g_signposts_linked_lroute_053));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_054) <= 2, size_check_g_signposts_linked_lroute_054);
	memcpy(&g_signposts_linked_lroute_054, exe_buf + 0x021408, sizeof(g_signposts_linked_lroute_054));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_055) <= 2, size_check_g_signposts_linked_lroute_055);
	memcpy(&g_signposts_linked_lroute_055, exe_buf + 0x02140a, sizeof(g_signposts_linked_lroute_055));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_056) <= 2, size_check_g_signposts_linked_lroute_056);
	memcpy(&g_signposts_linked_lroute_056, exe_buf + 0x02140c, sizeof(g_signposts_linked_lroute_056));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_057) <= 2, size_check_g_signposts_linked_lroute_057);
	memcpy(&g_signposts_linked_lroute_057, exe_buf + 0x02140e, sizeof(g_signposts_linked_lroute_057));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_058) <= 3, size_check_g_signposts_linked_lroute_058);
	memcpy(&g_signposts_linked_lroute_058, exe_buf + 0x021410, sizeof(g_signposts_linked_lroute_058));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_059) <= 2, size_check_g_signposts_linked_lroute_059);
	memcpy(&g_signposts_linked_lroute_059, exe_buf + 0x021413, sizeof(g_signposts_linked_lroute_059));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_060) <= 2, size_check_g_signposts_linked_lroute_060);
	memcpy(&g_signposts_linked_lroute_060, exe_buf + 0x021415, sizeof(g_signposts_linked_lroute_060));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_061) <= 3, size_check_g_signposts_linked_lroute_061);
	memcpy(&g_signposts_linked_lroute_061, exe_buf + 0x021417, sizeof(g_signposts_linked_lroute_061));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_062) <= 3, size_check_g_signposts_linked_lroute_062);
	memcpy(&g_signposts_linked_lroute_062, exe_buf + 0x02141a, sizeof(g_signposts_linked_lroute_062));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_063) <= 2, size_check_g_signposts_linked_lroute_063);
	memcpy(&g_signposts_linked_lroute_063, exe_buf + 0x02141d, sizeof(g_signposts_linked_lroute_063));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_064) <= 3, size_check_g_signposts_linked_lroute_064);
	memcpy(&g_signposts_linked_lroute_064, exe_buf + 0x02141f, sizeof(g_signposts_linked_lroute_064));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_065) <= 2, size_check_g_signposts_linked_lroute_065);
	memcpy(&g_signposts_linked_lroute_065, exe_buf + 0x021422, sizeof(g_signposts_linked_lroute_065));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_066) <= 2, size_check_g_signposts_linked_lroute_066);
	memcpy(&g_signposts_linked_lroute_066, exe_buf + 0x021424, sizeof(g_signposts_linked_lroute_066));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_067) <= 2, size_check_g_signposts_linked_lroute_067);
	memcpy(&g_signposts_linked_lroute_067, exe_buf + 0x021426, sizeof(g_signposts_linked_lroute_067));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_068) <= 2, size_check_g_signposts_linked_lroute_068);
	memcpy(&g_signposts_linked_lroute_068, exe_buf + 0x021428, sizeof(g_signposts_linked_lroute_068));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_069) <= 2, size_check_g_signposts_linked_lroute_069);
	memcpy(&g_signposts_linked_lroute_069, exe_buf + 0x02142a, sizeof(g_signposts_linked_lroute_069));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_070) <= 2, size_check_g_signposts_linked_lroute_070);
	memcpy(&g_signposts_linked_lroute_070, exe_buf + 0x02142c, sizeof(g_signposts_linked_lroute_070));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_071) <= 2, size_check_g_signposts_linked_lroute_071);
	memcpy(&g_signposts_linked_lroute_071, exe_buf + 0x02142e, sizeof(g_signposts_linked_lroute_071));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_072) <= 2, size_check_g_signposts_linked_lroute_072);
	memcpy(&g_signposts_linked_lroute_072, exe_buf + 0x021430, sizeof(g_signposts_linked_lroute_072));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_073) <= 2, size_check_g_signposts_linked_lroute_073);
	memcpy(&g_signposts_linked_lroute_073, exe_buf + 0x021432, sizeof(g_signposts_linked_lroute_073));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_074) <= 2, size_check_g_signposts_linked_lroute_074);
	memcpy(&g_signposts_linked_lroute_074, exe_buf + 0x021434, sizeof(g_signposts_linked_lroute_074));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_075) <= 2, size_check_g_signposts_linked_lroute_075);
	memcpy(&g_signposts_linked_lroute_075, exe_buf + 0x021436, sizeof(g_signposts_linked_lroute_075));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_076) <= 2, size_check_g_signposts_linked_lroute_076);
	memcpy(&g_signposts_linked_lroute_076, exe_buf + 0x021438, sizeof(g_signposts_linked_lroute_076));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_077) <= 2, size_check_g_signposts_linked_lroute_077);
	memcpy(&g_signposts_linked_lroute_077, exe_buf + 0x02143a, sizeof(g_signposts_linked_lroute_077));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_078) <= 2, size_check_g_signposts_linked_lroute_078);
	memcpy(&g_signposts_linked_lroute_078, exe_buf + 0x02143c, sizeof(g_signposts_linked_lroute_078));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_079) <= 2, size_check_g_signposts_linked_lroute_079);
	memcpy(&g_signposts_linked_lroute_079, exe_buf + 0x02143e, sizeof(g_signposts_linked_lroute_079));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_080) <= 2, size_check_g_signposts_linked_lroute_080);
	memcpy(&g_signposts_linked_lroute_080, exe_buf + 0x021440, sizeof(g_signposts_linked_lroute_080));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_081) <= 2, size_check_g_signposts_linked_lroute_081);
	memcpy(&g_signposts_linked_lroute_081, exe_buf + 0x021442, sizeof(g_signposts_linked_lroute_081));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_082) <= 2, size_check_g_signposts_linked_lroute_082);
	memcpy(&g_signposts_linked_lroute_082, exe_buf + 0x021444, sizeof(g_signposts_linked_lroute_082));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_083) <= 2, size_check_g_signposts_linked_lroute_083);
	memcpy(&g_signposts_linked_lroute_083, exe_buf + 0x021446, sizeof(g_signposts_linked_lroute_083));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_084) <= 2, size_check_g_signposts_linked_lroute_084);
	memcpy(&g_signposts_linked_lroute_084, exe_buf + 0x021448, sizeof(g_signposts_linked_lroute_084));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_085) <= 2, size_check_g_signposts_linked_lroute_085);
	memcpy(&g_signposts_linked_lroute_085, exe_buf + 0x02144a, sizeof(g_signposts_linked_lroute_085));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_086) <= 2, size_check_g_signposts_linked_lroute_086);
	memcpy(&g_signposts_linked_lroute_086, exe_buf + 0x02144c, sizeof(g_signposts_linked_lroute_086));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_087) <= 2, size_check_g_signposts_linked_lroute_087);
	memcpy(&g_signposts_linked_lroute_087, exe_buf + 0x02144e, sizeof(g_signposts_linked_lroute_087));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_088) <= 2, size_check_g_signposts_linked_lroute_088);
	memcpy(&g_signposts_linked_lroute_088, exe_buf + 0x021450, sizeof(g_signposts_linked_lroute_088));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_089) <= 2, size_check_g_signposts_linked_lroute_089);
	memcpy(&g_signposts_linked_lroute_089, exe_buf + 0x021452, sizeof(g_signposts_linked_lroute_089));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_090) <= 2, size_check_g_signposts_linked_lroute_090);
	memcpy(&g_signposts_linked_lroute_090, exe_buf + 0x021454, sizeof(g_signposts_linked_lroute_090));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_091) <= 2, size_check_g_signposts_linked_lroute_091);
	memcpy(&g_signposts_linked_lroute_091, exe_buf + 0x021456, sizeof(g_signposts_linked_lroute_091));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_092) <= 2, size_check_g_signposts_linked_lroute_092);
	memcpy(&g_signposts_linked_lroute_092, exe_buf + 0x021458, sizeof(g_signposts_linked_lroute_092));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_093) <= 2, size_check_g_signposts_linked_lroute_093);
	memcpy(&g_signposts_linked_lroute_093, exe_buf + 0x02145a, sizeof(g_signposts_linked_lroute_093));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_094) <= 2, size_check_g_signposts_linked_lroute_094);
	memcpy(&g_signposts_linked_lroute_094, exe_buf + 0x02145c, sizeof(g_signposts_linked_lroute_094));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_095) <= 2, size_check_g_signposts_linked_lroute_095);
	memcpy(&g_signposts_linked_lroute_095, exe_buf + 0x02145e, sizeof(g_signposts_linked_lroute_095));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_096) <= 2, size_check_g_signposts_linked_lroute_096);
	memcpy(&g_signposts_linked_lroute_096, exe_buf + 0x021460, sizeof(g_signposts_linked_lroute_096));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_097) <= 2, size_check_g_signposts_linked_lroute_097);
	memcpy(&g_signposts_linked_lroute_097, exe_buf + 0x021462, sizeof(g_signposts_linked_lroute_097));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_098) <= 2, size_check_g_signposts_linked_lroute_098);
	memcpy(&g_signposts_linked_lroute_098, exe_buf + 0x021464, sizeof(g_signposts_linked_lroute_098));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_099) <= 2, size_check_g_signposts_linked_lroute_099);
	memcpy(&g_signposts_linked_lroute_099, exe_buf + 0x021466, sizeof(g_signposts_linked_lroute_099));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_100) <= 2, size_check_g_signposts_linked_lroute_100);
	memcpy(&g_signposts_linked_lroute_100, exe_buf + 0x021468, sizeof(g_signposts_linked_lroute_100));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_101) <= 2, size_check_g_signposts_linked_lroute_101);
	memcpy(&g_signposts_linked_lroute_101, exe_buf + 0x02146a, sizeof(g_signposts_linked_lroute_101));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_102) <= 2, size_check_g_signposts_linked_lroute_102);
	memcpy(&g_signposts_linked_lroute_102, exe_buf + 0x02146c, sizeof(g_signposts_linked_lroute_102));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_103) <= 2, size_check_g_signposts_linked_lroute_103);
	memcpy(&g_signposts_linked_lroute_103, exe_buf + 0x02146e, sizeof(g_signposts_linked_lroute_103));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_104) <= 2, size_check_g_signposts_linked_lroute_104);
	memcpy(&g_signposts_linked_lroute_104, exe_buf + 0x021470, sizeof(g_signposts_linked_lroute_104));
	STATIC_ASSERT(sizeof(g_signposts_linked_lroute_105) <= 2, size_check_g_signposts_linked_lroute_105);
	memcpy(&g_signposts_linked_lroute_105, exe_buf + 0x021472, sizeof(g_signposts_linked_lroute_105));
	STATIC_ASSERT(sizeof(g_harbors_linked_sea_routes_01) <= 6, size_check_g_harbors_linked_sea_routes_01);
	memcpy(&g_harbors_linked_sea_routes_01, exe_buf + 0x0216f0, sizeof(g_harbors_linked_sea_routes_01));
	STATIC_ASSERT(sizeof(g_harbors_linked_sea_routes_02) <= 3, size_check_g_harbors_linked_sea_routes_02);
	memcpy(&g_harbors_linked_sea_routes_02, exe_buf + 0x0216f6, sizeof(g_harbors_linked_sea_routes_02));
	STATIC_ASSERT(sizeof(g_harbors_linked_sea_routes_03) <= 4, size_check_g_harbors_linked_sea_routes_03);
	memcpy(&g_harbors_linked_sea_routes_03, exe_buf + 0x0216f9, sizeof(g_harbors_linked_sea_routes_03));
	STATIC_ASSERT(sizeof(g_harbors_linked_sea_routes_04) <= 4, size_check_g_harbors_linked_sea_routes_04);
	memcpy(&g_harbors_linked_sea_routes_04, exe_buf + 0x0216fd, sizeof(g_harbors_linked_sea_routes_04));
	STATIC_ASSERT(sizeof(g_harbors_linked_sea_routes_05) <= 4, size_check_g_harbors_linked_sea_routes_05);
	memcpy(&g_harbors_linked_sea_routes_05, exe_buf + 0x021701, sizeof(g_harbors_linked_sea_routes_05));
	STATIC_ASSERT(sizeof(g_harbors_linked_sea_routes_06) <= 4, size_check_g_harbors_linked_sea_routes_06);
	memcpy(&g_harbors_linked_sea_routes_06, exe_buf + 0x021705, sizeof(g_harbors_linked_sea_routes_06));
	STATIC_ASSERT(sizeof(g_harbors_linked_sea_routes_07) <= 2, size_check_g_harbors_linked_sea_routes_07);
	memcpy(&g_harbors_linked_sea_routes_07, exe_buf + 0x021709, sizeof(g_harbors_linked_sea_routes_07));
	STATIC_ASSERT(sizeof(g_harbors_linked_sea_routes_08) <= 8, size_check_g_harbors_linked_sea_routes_08);
	memcpy(&g_harbors_linked_sea_routes_08, exe_buf + 0x02170b, sizeof(g_harbors_linked_sea_routes_08));
	STATIC_ASSERT(sizeof(g_harbors_linked_sea_routes_09) <= 5, size_check_g_harbors_linked_sea_routes_09);
	memcpy(&g_harbors_linked_sea_routes_09, exe_buf + 0x021713, sizeof(g_harbors_linked_sea_routes_09));
	STATIC_ASSERT(sizeof(g_harbors_linked_sea_routes_10) <= 7, size_check_g_harbors_linked_sea_routes_10);
	memcpy(&g_harbors_linked_sea_routes_10, exe_buf + 0x021718, sizeof(g_harbors_linked_sea_routes_10));
	STATIC_ASSERT(sizeof(g_harbors_linked_sea_routes_11) <= 3, size_check_g_harbors_linked_sea_routes_11);
	memcpy(&g_harbors_linked_sea_routes_11, exe_buf + 0x02171f, sizeof(g_harbors_linked_sea_routes_11));
	STATIC_ASSERT(sizeof(g_harbors_linked_sea_routes_12) <= 5, size_check_g_harbors_linked_sea_routes_12);
	memcpy(&g_harbors_linked_sea_routes_12, exe_buf + 0x021722, sizeof(g_harbors_linked_sea_routes_12));
	STATIC_ASSERT(sizeof(g_harbors_linked_sea_routes_13) <= 4, size_check_g_harbors_linked_sea_routes_13);
	memcpy(&g_harbors_linked_sea_routes_13, exe_buf + 0x021727, sizeof(g_harbors_linked_sea_routes_13));
	STATIC_ASSERT(sizeof(g_harbors_linked_sea_routes_14) <= 3, size_check_g_harbors_linked_sea_routes_14);
	memcpy(&g_harbors_linked_sea_routes_14, exe_buf + 0x02172b, sizeof(g_harbors_linked_sea_routes_14));
	STATIC_ASSERT(sizeof(g_harbors_linked_sea_routes_15) <= 7, size_check_g_harbors_linked_sea_routes_15);
	memcpy(&g_harbors_linked_sea_routes_15, exe_buf + 0x02172e, sizeof(g_harbors_linked_sea_routes_15));
	STATIC_ASSERT(sizeof(g_harbors_linked_sea_routes_16) <= 6, size_check_g_harbors_linked_sea_routes_16);
	memcpy(&g_harbors_linked_sea_routes_16, exe_buf + 0x021735, sizeof(g_harbors_linked_sea_routes_16));
	STATIC_ASSERT(sizeof(g_harbors_linked_sea_routes_17) <= 4, size_check_g_harbors_linked_sea_routes_17);
	memcpy(&g_harbors_linked_sea_routes_17, exe_buf + 0x02173b, sizeof(g_harbors_linked_sea_routes_17));
	STATIC_ASSERT(sizeof(g_harbors_linked_sea_routes_18) <= 9, size_check_g_harbors_linked_sea_routes_18);
	memcpy(&g_harbors_linked_sea_routes_18, exe_buf + 0x02173f, sizeof(g_harbors_linked_sea_routes_18));
	STATIC_ASSERT(sizeof(g_harbors_linked_sea_routes_19) <= 3, size_check_g_harbors_linked_sea_routes_19);
	memcpy(&g_harbors_linked_sea_routes_19, exe_buf + 0x021748, sizeof(g_harbors_linked_sea_routes_19));
	STATIC_ASSERT(sizeof(g_harbors_linked_sea_routes_20) <= 5, size_check_g_harbors_linked_sea_routes_20);
	memcpy(&g_harbors_linked_sea_routes_20, exe_buf + 0x02174b, sizeof(g_harbors_linked_sea_routes_20));
	STATIC_ASSERT(sizeof(g_harbors_linked_sea_routes_21) <= 3, size_check_g_harbors_linked_sea_routes_21);
	memcpy(&g_harbors_linked_sea_routes_21, exe_buf + 0x021750, sizeof(g_harbors_linked_sea_routes_21));
	STATIC_ASSERT(sizeof(g_harbors_linked_sea_routes_22) <= 4, size_check_g_harbors_linked_sea_routes_22);
	memcpy(&g_harbors_linked_sea_routes_22, exe_buf + 0x021753, sizeof(g_harbors_linked_sea_routes_22));
	STATIC_ASSERT(sizeof(g_harbors_linked_sea_routes_23) <= 3, size_check_g_harbors_linked_sea_routes_23);
	memcpy(&g_harbors_linked_sea_routes_23, exe_buf + 0x021757, sizeof(g_harbors_linked_sea_routes_23));
	STATIC_ASSERT(sizeof(g_harbors_linked_sea_routes_24) <= 6, size_check_g_harbors_linked_sea_routes_24);
	memcpy(&g_harbors_linked_sea_routes_24, exe_buf + 0x02175a, sizeof(g_harbors_linked_sea_routes_24));
	STATIC_ASSERT(sizeof(g_harbors_linked_sea_routes_25) <= 3, size_check_g_harbors_linked_sea_routes_25);
	memcpy(&g_harbors_linked_sea_routes_25, exe_buf + 0x021760, sizeof(g_harbors_linked_sea_routes_25));
	STATIC_ASSERT(sizeof(g_town_positions) <= 208, size_check_g_town_positions);
	memcpy(&g_town_positions, exe_buf + 0x0217ff, sizeof(g_town_positions));
	STATIC_ASSERT(sizeof(g_tevents_tab) <= 469, size_check_g_tevents_tab);
	memcpy(&g_tevents_tab, exe_buf + 0x021a2d, sizeof(g_tevents_tab));
	STATIC_ASSERT(sizeof(g_gui_char_width) <= 225, size_check_g_gui_char_width);
	memcpy(&g_gui_char_width, exe_buf + 0x021e11, sizeof(g_gui_char_width));
	STATIC_ASSERT(sizeof(g_gui_char_width_font8) <= 201, size_check_g_gui_char_width_font8);
	memcpy(&g_gui_char_width_font8, exe_buf + 0x021f02, sizeof(g_gui_char_width_font8));
	STATIC_ASSERT(sizeof(g_analues_items) <= 140, size_check_g_analues_items);
	memcpy(&g_analues_items, exe_buf + 0x021ffc, sizeof(g_analues_items));
	STATIC_ASSERT(sizeof(g_alchemy_recipes) <= 364, size_check_g_alchemy_recipes);
	memcpy(&g_alchemy_recipes, exe_buf + 0x02209a, sizeof(g_alchemy_recipes));
	STATIC_ASSERT(sizeof(g_trv_journey_random_encounter_stats) <= 98, size_check_g_trv_journey_random_encounter_stats);
	memcpy(&g_trv_journey_random_encounter_stats, exe_buf + 0x022579, sizeof(g_trv_journey_random_encounter_stats));
	STATIC_ASSERT(sizeof(g_palette_unknown4) <= 96, size_check_g_palette_unknown4);
	memcpy(&g_palette_unknown4, exe_buf + 0x022611, sizeof(g_palette_unknown4));
	STATIC_ASSERT(sizeof(g_palette_general) <= 96, size_check_g_palette_general);
	memcpy(&g_palette_general, exe_buf + 0x022671, sizeof(g_palette_general));
	ngen_load_tables_from_exe(exe_buf);
	free(exe_buf);
}

static void schick_format_crash_state(char *buf, int buf_size)
{
	int n = snprintf(buf, (size_t)buf_size,
		"Spielort:        town_id=%d dungeon_id=%d loc_type=%d pos=(%d,%d) dir=%d\n"
		"Spielstand:      active_group=%d day_timer=%ld datum=%d.%d.%d\n",
		(int)gs_town_id, (int)gs_dungeon_id, (int)gs_town_loc_type,
		(int)gs_x, (int)gs_y, (int)gs_viewdir,
		(int)gs_active_group_id, (long)gs_day_timer,
		(int)gs_day_of_month, (int)gs_month, (int)gs_year
	);
	if (g_heroes != NULL && n > 0 && n < buf_size) {
		for (int hi = 0; hi <= 6 && n < buf_size - 64; ++hi) {
			struct struct_hero *h = get_hero(hi);
			if (h && h->typus != 0) {
				n += snprintf(buf + n, (size_t)(buf_size - n),
					"Held[%d]:         %s (Typ=%d, Grp=%d, LE=%d/%d, AE=%d/%d)\n",
					hi, h->alias, (int)h->typus, (int)h->group_id,
					(int)h->le, (int)h->le_max, (int)h->ae, (int)h->ae_max
				);
			}
		}
	}
}
#endif

int main(int argc, char** argv)
{
	dsa_trace_enter(133, "seg002:main");
	signed int argv1_len;
	signed int answer;
	int32_t diskspace;
	signed int savegame;
	signed int len;

#if !defined(__BORLANDC__)
	{
		int explicit_gamedir = (getenv("DSA_GAME_DIR") != NULL || getenv("DSA_GAMEDIR") != NULL) ? 1 : 0;
		dsa_platform_register_crash_state_cb(schick_format_crash_state);
		const char *env_cp = getenv("DSA_COPY_PROTECTION");
		if (env_cp && env_cp[0]) {
			g_skip_copy_protection = (env_cp[0] == '0') ? 1 : 0;
		}
		const char *env_intro = getenv("DSA_INTRO");
		if (env_intro && env_intro[0]) {
			if (env_intro[0] == '0') {
				g_run_intro = 0;
			} else if (env_intro[0] == '1') {
				g_run_intro = 1;
				g_intro_explicit = 1;
			}
		}
		for (int arg_i = 1; arg_i < argc; ++arg_i) {
			if (strcmp(argv[arg_i], "--copy-protection") == 0) {
				g_skip_copy_protection = 0;
			} else if (strcmp(argv[arg_i], "--skip-copy-protection") == 0) {
				g_skip_copy_protection = 1;
			} else if (strcmp(argv[arg_i], "--intro") == 0) {
				g_run_intro = 1;
				g_intro_explicit = 1;
			} else if (strcmp(argv[arg_i], "--kein-intro") == 0 || strcmp(argv[arg_i], "--no-intro") == 0) {
				g_run_intro = 0;
				g_intro_explicit = 0;
			} else if (strcmp(argv[arg_i], "--coverage") == 0) {
				g_dsa_coverage_enabled = 1;
			} else if (strcmp(argv[arg_i], "--fast-replay") == 0) {
				dsa_platform_set_fast_replay(1);
			} else if (strcmp(argv[arg_i], "--cd") == 0 || strcmp(argv[arg_i], "--music=cd") == 0 || strcmp(argv[arg_i], "--music=ogg") == 0) {
				dsa_platform_set_music_mode(DSA_MUSIC_MODE_CD);
			} else if (strcmp(argv[arg_i], "--gm") == 0 || strcmp(argv[arg_i], "--music=gm") == 0 || strcmp(argv[arg_i], "--music=midi") == 0) {
				dsa_platform_set_music_mode(DSA_MUSIC_MODE_GM);
			} else if (strcmp(argv[arg_i], "--opl") == 0 || strcmp(argv[arg_i], "--adlib") == 0 || strcmp(argv[arg_i], "--music=opl") == 0 || strcmp(argv[arg_i], "--music=adlib") == 0) {
				dsa_platform_set_music_mode(DSA_MUSIC_MODE_OPL);
			} else if (strcmp(argv[arg_i], "--music") == 0 && arg_i + 1 < argc) {
				const char *m = argv[++arg_i];
				if (strcmp(m, "opl") == 0 || strcmp(m, "adlib") == 0) {
					dsa_platform_set_music_mode(DSA_MUSIC_MODE_OPL);
				} else if (strcmp(m, "gm") == 0 || strcmp(m, "midi") == 0) {
					dsa_platform_set_music_mode(DSA_MUSIC_MODE_GM);
				} else {
					dsa_platform_set_music_mode(DSA_MUSIC_MODE_CD);
				}
			} else if (strcmp(argv[arg_i], "--dump-wav") == 0 && arg_i + 1 < argc) {
				dsa_platform_set_dump_wav(argv[++arg_i]);
			} else if (strcmp(argv[arg_i], "--record") == 0 && arg_i + 1 < argc) {
				dsa_platform_set_record_file(argv[++arg_i]);
			} else if (strcmp(argv[arg_i], "--replay") == 0 && arg_i + 1 < argc) {
				dsa_platform_set_replay_file(argv[++arg_i]);
				if (!g_intro_explicit) {
					g_run_intro = 0;
				}
			} else if (strcmp(argv[arg_i], "--gamedir") == 0 && arg_i + 1 < argc) {
				dsa_platform_set_game_dir(argv[++arg_i]);
				explicit_gamedir = 1;
			} else if (strcmp(argv[arg_i], "--savedir") == 0 && arg_i + 1 < argc) {
				dsa_platform_set_save_dir(argv[++arg_i]);
			} else if (argv[arg_i][0] != '-') {
				dsa_platform_set_game_dir(argv[arg_i]);
				explicit_gamedir = 1;
			}
		}
		{
			const char *sd = dsa_platform_get_save_dir();
			if (sd && sd[0]) {
				chdir(sd);
			} else if (access("SCHICK.DAT", 0) != 0) {
				const char *gd = dsa_platform_get_game_dir();
				if (gd && gd[0]) {
					chdir(gd);
				}
			}
		}
		MKDIR_PORT_S2("TEMP");
		schick_load_tables_from_exe(explicit_gamedir);
	}
#endif
	g_pregame_state = 1;

	g_playmask_us = 1;

	init_AIL(16000);

#if defined(__BORLANDC__)
	randomize();

	save_display_stat(&g_video_page_bak);
#endif

	if (!init_memory()) {

#if defined(__BORLANDC__)
		g_gameinit_flag = 1;
#endif

		schick_set_video();

		g_have_mouse = 2;

		mouse_init();

		if (g_have_mouse == 0) {
			g_mouse_refresh_flag = -10;
		}

		init_game_state();

		save_and_set_timer();

		init_common_buffers();

		g_textbox_width = 3;

		call_mouse();

		if (argc == 2) {

#if defined(__BORLANDC__)
			/* some trick to disable the cd check */
			/* g_cd_skipmagic = Produkt aller Zeichencodes von argv[1]; ergibt es 0x682772e4,
			 * entfaellt die CD-Pruefung (siehe CD_read_exe() in seg001) */

			len = strlen(argv[1]);

			argv1_len = 0;

			g_cd_skipmagic = 1;
			while (argv1_len < len) {


				g_cd_skipmagic = argv[1][0] * g_cd_skipmagic;
				argv[1]++;
				argv1_len++;
			}
#endif
		}

		prepare_dirs();

		if (have_mem_for_sound()) {

			read_sound_cfg();
			alloc_voc_buffer(20000);

		} else {
			/* disable sound */
			exit_AIL();
#if defined(__BORLANDC__)
			GUI_output(g_snd_txt_disabled_mem2);
#endif
		}

		CD_init();

#if defined(__BORLANDC__)
		if (!g_cd_init_successful) {

			/* CD init failed */
			cleanup_game();
			exit(0);
		}
#endif

#if !defined(__BORLANDC__)
		if (g_run_intro) {
			uint8_t pal_white[3] = { 0x3f, 0x3f, 0x3f };
			call_mouse_bg();
			if (!dsa_run_original_intro()) {
				ngen_run_intro();
			}
			set_color(pal_white, 0xff);
			refresh_colors();
			dsa_platform_set_mouse_range_x(0, 319);
			dsa_platform_set_mouse_range_y(0, 199);
			call_mouse();
			flush_keyboard_queue();
		}
#endif
		/* select game mode */
		g_game_mode = GAME_MODE_UNSPECIFIED;

		while (g_game_mode == GAME_MODE_UNSPECIFIED) {
			g_game_mode = GUI_radio(get_ttx(5), 2, get_ttx(6), get_ttx(7)); /* "WÄHLEN SIE DEN@SCHWIERIGKEITS..." / "ANFÄNGER" / "FORTGESCHRITTENE" */
		}

		if (copy_protection()) {

			g_textbox_width = 3;

			diskspace = get_diskspace();

			if (diskspace < 0) {

				sprintf(g_dtp2, get_ttx(807), -diskspace); /* "AUF IHRER FESTPLATTE FEHLEN %ld BYTES SPEICHERKAPAZITÄT, UM <DIE@SC..." */
				GUI_output(g_dtp2);
				cleanup_game();

			} else {

				/* ask for generation or game */
				do {
					answer = GUI_radio(get_ttx(820), 2, get_ttx(821), get_ttx(822)) - 1; /* "WAS SOLL GESTARTET WERDEN?" / "HAUPTPROGRAMM" / "GENERIERUNG" */

				} while (answer == -1);

				if (answer == 1) {
					call_gen();
				}

				flush_keyboard_queue();


				/* load a savegame */
				do {
					savegame = load_game_state();

				} while (savegame == -1);

				g_pregame_state = 0;

				/* start the game */
				game_loop();

				cleanup_game();
			}
		} else {
			cleanup_game();
		}

	} else {
		/* not enough memory */
		exit_AIL();
		schick_reset_video();
#if defined(__BORLANDC__)
		clrscr();
#endif
	}

#if !defined(__BORLANDC__)
	return 0;
#endif
}

uint8_t* schick_alloc(uint32_t size)
{
#if defined(__BORLANDC__)
	return (uint8_t*)farcalloc(size, 1);
#else
	return (uint8_t*)calloc(size, 1);
#endif
}

/* Kopierschutz: bis zu 3 Fragen nach einem Wort aus dem Handbuch oder einer Stadt auf der Karte.
 * 1 = richtig beantwortet */
static signed int copy_protection(void)
{
	dsa_trace_enter(134, "seg002:copy_protection");
#if !defined(__BORLANDC__)
	if (g_skip_copy_protection) {
		return 1;
	}
#endif
	signed int i;
	signed int randval;
	signed int tries;
	signed int len;

	load_tx(ARCHIVE_FILE_FIGHTTXT_LTX);

	g_textbox_width = 4;

	set_textcolor(0xff, 0);

#if FIX_KOPIERSCHUTZ && defined(__BORLANDC__)
	/* Zusatzfunktion FIX_KOPIERSCHUTZ: keine Kopierschutzabfrage in den DOS-Fix-Fassungen (wie im Port) */
	return 1;
#else
	tries = 0;

	while (tries < 3) {

		if (random_schick(100) <= 50) {
			/* handbook question */

			/* select a question */
			randval = random_schick(10) - 1;

			/* prepare the string */
			sprintf(g_dtp2, get_tx(39), /* "BITTE SCHLAGEN SIE IHR HANDBUCH AUF@SEITE %d@AUF UND GEBEN SIE DAS..." */
				g_questions_handbook[randval][3],
				g_questions_handbook[randval][2],
				g_questions_handbook[randval][1],
				g_questions_handbook[randval][0]);

			/* print version number */
			GUI_print_string(g_game_version, 290, 190);

			/* ask the question */
			GUI_input(g_dtp2, 20);

			len = strlen(g_text_input_buf);

			/* transform the input string in uppercase letters and bitwise invert them */
			for (i = 0; i < len; i++) {
				g_text_input_buf[i] = ~toupper(g_text_input_buf[i]);
			}

			if (!strcmp((char*)&g_questions_handbook[randval][4], g_text_input_buf)) {
				return 1;
			}
		} else {
			/* map question */

			/* select a question */
			randval = random_schick(10) - 1;

			/* prepare the string */
			sprintf(g_dtp2, get_tx(40), /* "WIE HEISST DIE STADT@%s VON %s@(AUS DER BEILIEGENDEN FARBKARTE ZU E..." */
				get_tx(41 + g_questions_map[randval][0]), /* FIGHTTXT.LTX 41..44: Himmelsrichtung */
				get_ttx(235 + g_questions_map[randval][1])); /* TEXT.LTX 236..287: Ortsname */

			/* print version number */
			GUI_print_string(g_game_version, 290, 190);

			/* ask the question */
			GUI_input(g_dtp2, 20);

			len = strlen(g_text_input_buf);

			/* transform the input string in uppercase letters */
			for (i = 0; i < len; i++) {
				g_text_input_buf[i] = toupper(g_text_input_buf[i]);
			}

			if (!strcmp(get_ttx(235 + g_questions_map[randval][2]), g_text_input_buf)) { /* TEXT.LTX 236..287: Ortsname */
				return 1;
			}
		}

		tries++;
	}

	return 0;
#endif
}
