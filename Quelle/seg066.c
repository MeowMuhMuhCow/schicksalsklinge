/*
 * seg066 - Stadt: Bewegung, Orte betreten, 3D-Ansicht
 *
 * Inhalt:    do_town()/town_step() sind die Hauptschleife in einer Stadt: Menue (Gruppe
 *            teilen/vereinen/wechseln, Spielstand, Karte, Zauber, Lager, Markt), Drehen und
 *            Schritte, Zufallsereignisse in den vier grossen Staedten, Markttage.
 *            enter_location() prueft, ob die Gruppe auf einem Ort steht oder vor einer
 *            Haustuer (Buerger oder verschlossenes Haus); Daspota hat eine eigene Variante
 *            mit Raeuberhaeusern und Beute. do_special_buildings() verzweigt zu den
 *            Sondergebaeuden von Thorwal und Phexcaer und zum Einsiedler.
 *            town_update_view() und Helfer zeichnen die Strassenansicht aus 29 sichtbaren
 *            Feldern (Verdeckung, Texturwahl, Wasser/Gras, Gebaeude, Palette).
 * Aufrufer:  game_loop (seg002), Ortstabelle (seg025: do_special_buildings), TLK_eremit aus
 *            seg030, town_update_house_count() aus seg028, get_town_tile_type() auch aus seg074.
 * Daten:     g_market_descr_table (Markt je Stadt: Sortiment, Preis, Markttag),
 *            g_hyg_ani_x* (fuer seg065), Texturtabellen je Feldtyp und sichtbarem Feld,
 *            g_tex_descr_table (Lage und Bildnummern der Texturen).
 * Besonders: get_town_tile_type() liest bei Kartenwerten ab 0xf0 ueber das Tabellenende.
 *            load_town_texture(): bei y < 0 wird copy_height vergroessert statt verkleinert
 *            (mit den vorhandenen Daten vermutlich nie erreicht).
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg066.c
 */
#include <stdlib.h>
#include <string.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg003.h"
#include "seg004.h"
#include "seg007.h"
#include "seg008.h"
#include "seg024.h"
#include "seg025.h"
#include "seg026.h"
#include "seg027.h"
#include "seg028.h"
#include "seg029.h"
#include "seg030.h"
#include "seg032.h"
#include "seg049.h"
#include "seg066.h"
#include "seg067.h"
#include "seg068.h"
#include "seg069.h"
#include "seg070.h"
#include "seg071.h"
#include "seg074.h"
#include "seg092.h"
#include "seg096.h"
#include "seg097.h"
#include "seg098.h"

struct market g_market_descr_table[34] = {
	{  4,  40, -1, 0 },
	{ 10,   0, -1, 0 },
	{ 11,   0,  3, 0 },
	{ 10,  20,  5, 0 },
	{ 13, -30,  5, 0 },
	{  9,   0,  0, 0 },
	{ 14, -30,  3, 0 },
	{ 13,   0,  5, 0 },
	{ 19,   0,  3, 0 },
	{ 12, -10,  0, 0 },
	{  8,  10,  3, 0 },
	{ 17, -10,  4, 0 },
	{ 16,   0,  1, 0 },
	{ 14,  10,  3, 0 },
	{  6,  30,  5, 0 },
	{  9,  10, -1, 0 },
	{ 10,  40, -1, 0 },
	{  1, -50, -1, 0 },
	{  7,  10,  5, 0 },
	{ 10, -20,  3, 0 },
	{  9,  20,  3, 0 },
	{  8,  10,  2, 0 },
	{ 11,  20,  1, 0 },
	{ 10,   0,  0, 0 },
	{ 10,  20,  5, 0 },
	{  8,  20,  2, 0 },
	{ 14, -30,  5, 0 },
	{  5,   0, -1, 0 },
	{  9, -10,  5, 0 },
	{ 12,  30,  5, 0 },
	{  5,  20,  1, 0 },
	{ 12,   0,  3, 0 },
	{  4,  30,  4, 0 },
	{ -1,   0,  0, 0 }
}; // ds:0x70a8
/* positions for show_hyggelik_ani() (seg065). g_hyg_ani_y9 holds y coordinates. */
const uint8_t g_hyg_ani_x0[5] = { 0x52, 0x58, 0x59, 0x5a, 0x5a }; // ds:0x71b8
const uint8_t g_hyg_ani_x1 = 0x5a; // ds:0x71bd
const uint8_t g_hyg_ani_x2 = 0x5a; // ds:0x71be
const uint8_t g_hyg_ani_x3[3] = { 0x5a, 0x5a, 0x5a }; // ds:0x71bf
const uint8_t g_hyg_ani_x5 = 0xba; // ds:0x71c2
const uint8_t g_hyg_ani_x6 = 0xba; // ds:0x71c3
const uint8_t g_hyg_ani_x7[3] = { 0xb1, 0xa8, 0xa4 }; // ds:0x71c4
const uint8_t g_hyg_ani_y9[3] = { 0x30, 0x26, 0x26 }; // ds:0x71c7

/* The following table is unnecessary, basically.
 * It seems that its only purpose is the retrieval of the message when entering an ordinary house.
 * In this case, this info would also be available from the preceding loop, see the line
 * assert(g_town_num_locations_table[town_id - 1] == (locations_tab_ptr - g_locations_tab)
 * below */
static const uint8_t g_town_num_locations_table[TOWN_ID__END - 1] = {
/* note that the index is "off by 1" compared to TOWN_ID_... */
	54, // TOWN_ID_THORWAL
	 4, // TOWN_ID_SERSKE
	 8, // TOWN_ID_BREIDA
	 7, // TOWN_ID_PEILINEN
	10, // TOWN_ID_ROVAMUND
	 4, // TOWN_ID_NORDVEST
	 2, // TOWN_ID_KRAVIK
	 4, // TOWN_ID_SKELELLEN
	 4, // TOWN_ID_MERSKE
	12, // TOWN_ID_EFFERDUN
	 3, // TOWN_ID_TJOILA
	 4, // TOWN_ID_RUKIAN
	 6, // TOWN_ID_ANGBODIRTAL
	 3, // TOWN_ID_AUPLOG
	12, // TOWN_ID_VILNHEIM
	 2, // TOWN_ID_BODON
	18, // TOWN_ID_OBERORKEN
	24, // TOWN_ID_PHEXCAER
	 3, // TOWN_ID_GROENVELDEN
	 6, // TOWN_ID_FELSTEYN
	 1, // TOWN_ID_EINSIEDLERSEE
	 7, // TOWN_ID_ORKANGER
	10, // TOWN_ID_CLANEGH
	 7, // TOWN_ID_LISKOR
	 6, // TOWN_ID_THOSS
	 8, // TOWN_ID_TJANSET
	 3, // TOWN_ID_ALA
	11, // TOWN_ID_ORVIL
	10, // TOWN_ID_OVERTHORN
	 3, // TOWN_ID_ROVIK
	 8, // TOWN_ID_HJALSINGOR
	 4, // TOWN_ID_GUDDASUNDEN
	12, // TOWN_ID_KORD
	 3, // TOWN_ID_TREBAN
	 5, // TOWN_ID_ARYN
	 4, // TOWN_ID_RUNINSHAVEN
	 8, // TOWN_ID_OTTARJE
	 9, // TOWN_ID_SKJAL
	22, // TOWN_ID_PREM
	19, // TOWN_ID_DASPOTA
	 3, // TOWN_ID_RYBON
	10, // TOWN_ID_LJASDAHL
	10, // TOWN_ID_VARNHEIM
	 7, // TOWN_ID_VAERMHAG
	 6, // TOWN_ID_TYLDON
	10, // TOWN_ID_VIDSAND
	 3, // TOWN_ID_BRENDHIL
	12, // TOWN_ID_MANRIN
	 2, // TOWN_ID_FAEHRSTATION_TJOILA
	 1, // TOWN_ID_FAEHRE_ANGBODIRTAL
	 1, // TOWN_ID_HJALLANDER_HOF
	 3  // TOWN_ID_LEUCHTTURM_RUNIN
}; // ds:0x71ca

static uint8_t* g_daspota_locloot_index[18] = {
	gs_daspota_location01_loot,
	gs_daspota_location02_loot,
	gs_daspota_location03_loot,
	gs_daspota_location04_loot,
	gs_daspota_location05_loot,
	gs_daspota_location06_loot,
	gs_daspota_location07_loot,
	gs_daspota_location08_loot,
	NULL,
	gs_daspota_location10_loot,
	NULL,
	gs_daspota_location12_loot,
	gs_daspota_location13_loot,
	gs_daspota_location14_loot,
	NULL,
	gs_daspota_location16_loot,
	NULL,
	gs_daspota_location18_loot
}; // ds:0x71fe; uint8_t*

/* upper bounds of the map values for each TOWN_TILE_* type, see get_town_tile_type() */
static const unsigned char g_mapval_to_town_tile[12] = { 0x00, 0x0f, 0x1f, 0x3f, 0x5f, 0x7f, 0x9f, 0xaf, 0xbf, 0xcf, 0xdf, 0xef }; // ds:0x7246
/* texture numbers (1-based index into g_tex_descr_table, -1 = none) for each of the 29 visual squares,
 * one table per TOWN_TILE_* type, see town_set_visual_square_tex() */
static const signed char g_visual_square_tex_house1[29] = { 0x01, -0x01, 0x02, -0x01, 0x03, 0x04, 0x05, -0x01, 0x06, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0a, -0x01, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, -0x01, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10 }; // ds:0x7252
static const signed char g_visual_square_tex_house2[29] = { 0x2a, -0x01, 0x2b, -0x01, 0x2c, 0x2d, 0x2e, -0x01, 0x2f, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x33, -0x01, 0x34, 0x35, 0x36, 0x37, 0x38, -0x01, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x39 }; // ds:0x726f
static const signed char g_visual_square_tex_house3[29] = { 0x3a, -0x01, 0x3b, -0x01, 0x3c, 0x3d, 0x3e, -0x01, 0x3f, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x43, -0x01, 0x44, 0x45, 0x46, 0x47, 0x48, -0x01, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49, 0x49 }; // ds:0x728c
static const signed char g_visual_square_tex_house4[29] = { 0x4e, -0x01, 0x4f, -0x01, 0x50, 0x51, 0x52, -0x01, 0x53, 0x53, 0x54, 0x55, 0x56, 0x57, 0x57, -0x01, 0x58, 0x59, 0x5a, 0x5b, 0x5c, -0x01, 0x5d, 0x5d, 0x5d, 0x5d, 0x5d, 0x5d, 0x5d }; // ds:0x72a9
static const signed char g_visual_square_tex_water_grass[29] = { 0x11, 0x12, 0x13, -0x01, 0x14, 0x15, 0x16, -0x01, -0x01, 0x17, 0x18, 0x19, 0x1a, 0x1b, -0x01, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29 }; // ds:0x72c6
static const signed char g_visual_square_tex_signpost[29] = { -0x01, -0x01, -0x01, -0x01, 0x4a, 0x4a, 0x4a, -0x01, -0x01, -0x01, 0x4b, 0x4b, 0x4b, -0x01, -0x01, -0x01, 0x4c, 0x4c, 0x4c, 0x4c, 0x4c, -0x01, 0x4d, 0x4d, 0x4d, 0x4d, 0x4d, 0x4d, 0x4d }; // ds:0x72e3
static const signed char g_visual_square_tex_inn[29] = { -0x01, -0x01, -0x01, -0x01, 0x5e, 0x5e, 0x5e, -0x01, -0x01, -0x01, 0x5f, 0x5f, 0x5f, -0x01, -0x01, -0x01, 0x60, 0x60, 0x60, 0x60, 0x60, -0x01, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61 }; // ds:0x7300
static const signed char g_visual_square_tex_merchant[29] = { -0x01, -0x01, -0x01, -0x01, 0x62, 0x62, 0x62, -0x01, -0x01, -0x01, 0x63, 0x63, 0x63, -0x01, -0x01, -0x01, 0x64, 0x64, 0x64, 0x64, 0x64, -0x01, 0x65, 0x65, 0x65, 0x65, 0x65, 0x65, 0x65 }; // ds:0x731d
static const struct struct_point g_visual_square_offsets_std[29] = {
	{ 0x0000, 0x0001 },
	{ -0x0001, -1 },
	{ 0x00bd, 0x0001 },
	{ -0x0141, 0x000e },
	{ -0x0097, 0x000e },
	{ 0x0013, 0x000e },
	{ 0x00bd, 0x000e },
	{ 0x0167, 0x000e },
	{ -0x00fd, 0x002e },
	{ -0x0097, 0x002e },
	{ -0x0031, 0x0024 },
	{ 0x0035, 0x0024 },
	{ 0x009b, 0x0024 },
	{ 0x0101, 0x002e },
	{ 0x0167, 0x002e },
	{ -0x005c, 0x0032 },
	{ -0x0024, 0x0032 },
	{ 0x0014, 0x0032 },
	{ 0x004c, 0x0032 },
	{ 0x0084, 0x0032 },
	{ 0x00bc, 0x0032 },
	{ 0x00f4, 0x0032 },
	{ -0x000e, 0x0039 },
	{ 0x0014, 0x0039 },
	{ 0x0036, 0x0039 },
	{ 0x0058, 0x0039 },
	{ 0x007a, 0x0039 },
	{ 0x009c, 0x0039 },
	{ 0x00be, 0x0039 }
}; // ds:0x733a
static const struct struct_point g_visual_square_offsets_sign[29] = {
	{ 0x0014, 0x0001 },
	{ -0x0001, -1 },
	{ 0x00a9, 0x0001 },
	{ -0x0141, 0x000e },
	{ -0x0065, 0x000e },
	{ 0x0013, 0x000e },
	{ 0x008b, 0x000e },
	{ 0x0167, 0x000e },
	{ -0x00fd, 0x002e },
	{ -0x0097, 0x002e },
	{ -0x001d, 0x0024 },
	{ 0x0035, 0x0024 },
	{ 0x0091, 0x0024 },
	{ 0x0101, 0x002e },
	{ 0x0167, 0x002e },
	{ -0x005c, 0x0032 },
	{ -0x0024, 0x0032 },
	{ 0x0014, 0x0032 },
	{ 0x004c, 0x0032 },
	{ 0x0084, 0x0032 },
	{ 0x00bc, 0x0032 },
	{ 0x00f4, 0x0032 },
	{ -0x000e, 0x0039 },
	{ 0x0014, 0x0039 },
	{ 0x0036, 0x0039 },
	{ 0x0058, 0x0039 },
	{ 0x007a, 0x0039 },
	{ 0x009c, 0x0039 },
	{ 0x00be, 0x0039 }
}; // ds:0x73ae
static const struct struct_point g_visual_square_offsets_inn[29] = {
	{ 0x0000, 0x0001 },
	{ -0x0001, -1 },
	{ 0x00bd, 0x0001 },
	{ -0x0141, 0x0000 },
	{ -0x0097, 0x0000 },
	{ 0x002d, 0x0000 },
	{ 0x00bd, 0x0000 },
	{ 0x0167, 0x0000 },
	{ -0x00fd, 0x0000 },
	{ -0x0097, 0x0000 },
	{ -0x0031, 0x0000 },
	{ 0x0046, 0x0000 },
	{ 0x009b, 0x0000 },
	{ 0x0101, 0x0000 },
	{ 0x0167, 0x0000 },
	{ -0x005c, 0x0000 },
	{ -0x0024, 0x0000 },
	{ 0x000e, 0x0000 },
	{ 0x0057, 0x0000 },
	{ 0x008a, 0x0000 },
	{ 0x00bc, 0x0000 },
	{ 0x00f4, 0x0000 },
	{ -0x000e, 0x000e },
	{ 0x0014, 0x000e },
	{ 0x0034, 0x000e },
	{ 0x005c, 0x000e },
	{ 0x007c, 0x000e },
	{ 0x009c, 0x000e },
	{ 0x00be, 0x000e }
}; // ds:0x7422
static const struct struct_point g_visual_square_offsets_grass[29] = {
	{ 0x0000, 0x007a },
	{ 0x0000, 0x007a },
	{ 0x00bd, 0x007a },
	{ 0x0000, 0x0064 },
	{ 0x0000, 0x0064 },
	{ 0x0014, 0x0064 },
	{ 0x009b, 0x0064 },
	{ 0x0167, 0x0064 },
	{ 0x0000, 0x0055 },
	{ 0x0000, 0x0055 },
	{ 0x0000, 0x0055 },
	{ 0x0036, 0x0055 },
	{ 0x0084, 0x0055 },
	{ 0x00be, 0x0055 },
	{ 0x00be, 0x0055 },
	{ 0x0000, 0x004d },
	{ 0x0000, 0x004d },
	{ 0x0017, 0x004d },
	{ 0x004d, 0x004d },
	{ 0x0078, 0x004d },
	{ 0x0099, 0x004d },
	{ 0x00ba, 0x004d },
	{ 0x0000, 0x0043 },
	{ 0x001e, 0x0043 },
	{ 0x003c, 0x0043 },
	{ 0x0059, 0x0043 },
	{ 0x0068, 0x0043 },
	{ 0x006b, 0x0043 },
	{ 0x006d, 0x0043 }
}; // ds:0x7496

/* REMARK: this is int16_t: 3 parts of a texture, each { x offset, y offset, nvf_no (-1 = none, bit 15 = mirrored) } */
static const unsigned char g_tex_descr_table[101][18] = {
	{ 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x0e, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0xff, 0xff, 0x0a, 0x00, 0xaa, 0x00, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0xff, 0xff, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0xff, 0xff, 0x0a, 0x00, 0xde, 0xff, 0xff, 0xff, 0x0f, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x97, 0x00, 0xff, 0xff, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0xff, 0xff, 0x0b, 0x00, 0x66, 0x00, 0xff, 0xff, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0xff, 0xff, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0xff, 0xff, 0x0b, 0x00, 0xe9, 0xff, 0xff, 0xff, 0x10, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff, 0x12, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x38, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x38, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0xf5, 0xff, 0x00, 0x00, 0x11, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0xdf, 0xff, 0x00, 0x00, 0x13, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0a, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x09, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x08, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x05, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x04, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x02, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x06, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x0e, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0xff, 0xff, 0x0a, 0x00, 0xaa, 0x00, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0xff, 0xff, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0xff, 0xff, 0x0a, 0x00, 0xde, 0xff, 0xff, 0xff, 0x0f, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x97, 0x00, 0xff, 0xff, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0xff, 0xff, 0x0b, 0x00, 0x66, 0x00, 0xff, 0xff, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0xff, 0xff, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0xff, 0xff, 0x0b, 0x00, 0xe9, 0xff, 0xff, 0xff, 0x10, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff, 0x12, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x38, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x38, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0xf5, 0xff, 0x00, 0x00, 0x11, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0xdf, 0xff, 0x00, 0x00, 0x13, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x0e, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0xff, 0xff, 0x0a, 0x00, 0xaa, 0x00, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0xff, 0xff, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0xff, 0xff, 0x0a, 0x00, 0xde, 0xff, 0xff, 0xff, 0x0f, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x97, 0x00, 0xff, 0xff, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0xff, 0xff, 0x0b, 0x00, 0x66, 0x00, 0xff, 0xff, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0xff, 0xff, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0xff, 0xff, 0x0b, 0x00, 0xe9, 0xff, 0xff, 0xff, 0x10, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff, 0x12, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x38, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x38, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0xf5, 0xff, 0x00, 0x00, 0x11, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0xdf, 0xff, 0x00, 0x00, 0x13, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x31, 0x00, 0x1c, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x1c, 0x00, 0x0d, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x0f, 0x00, 0x07, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x09, 0x00, 0x05, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x0e, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0xff, 0xff, 0x0a, 0x00, 0xaa, 0x00, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0xff, 0xff, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0xff, 0xff, 0x0a, 0x00, 0xde, 0xff, 0xff, 0xff, 0x0f, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x97, 0x00, 0xff, 0xff, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0xff, 0xff, 0x0b, 0x00, 0x66, 0x00, 0xff, 0xff, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0xff, 0xff, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0xff, 0xff, 0x0b, 0x00, 0xe9, 0xff, 0xff, 0xff, 0x10, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff, 0x12, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x38, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x38, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0xf5, 0xff, 0x00, 0x00, 0x11, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0xdf, 0xff, 0x00, 0x00, 0x13, 0x80, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0xe8, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0xf2, 0xff, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0xf7, 0xff, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0xfa, 0xff, 0xf3, 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0xfe, 0xff, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff },
	{ 0x00, 0x00, 0xf3, 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff }
}; // ds:0x750a
static const signed char g_visual_square_draw_order[29] = {
	22, 23, 24, 25, 26, 27, 28, 15, 16, 21, 20, 17, 19, 18, 8, 9, 14, 13, 10, 12, 11, 3, 4, 7, 6, 5, 0, 2, 1
}; // ds:0x7c24
signed char g_can_merge_group = -1; // ds:0x7c41, {-1,1}


HugePt g_buffer9_ptr4;				// ds:0xe3fc, copy of BUFFER9_PTR
unsigned char g_town_house_count[4];		// ds:0xe400, counts of the four different kinds of houses on the current map
unsigned char *g_buffer11_ptr;			// ds:0xe404, to buffer of size 192, used for palettes
signed int g_town_refresh_direction;	// ds:0xe408
signed int g_town_refresh_y_target;	// ds:0xe40a
signed int g_town_refresh_x_target;	// ds:0xe40c
signed int g_always_zero1;		// ds:0xe40e, writeonly (0)
signed int g_always_zero2;		// ds:0xe410, writeonly (0)
signed int g_entrance_angle;		// ds:0xe412, side of the entrance relative to the view direction, 2 = facing the party
unsigned char *g_town_tex[6];			// ds:0xe414


/**
 * \brief   check if the party stands on a location or in front of a house door
 *
 * \param   town_id     the current town
 * \return              1 = location found (gs_town_loc_type etc. are set), 0 = nothing
 */
signed int enter_location(const signed int town_id)
{
	signed int map_pos;
	signed int tile_type;
	struct location *locations_tab_ptr;

	if (town_id == TOWN_ID_DASPOTA) {
		return enter_location_daspota();
	}

	map_pos = 256 * gs_x + gs_y;
	locations_tab_ptr = &g_locations_tab[0];
	g_location_market_flag = 0;

	do {
		if (locations_tab_ptr->pos == map_pos) {

			/* found the location */
			gs_town_loc_type_bak = LOCTYPE_NONE;
			gs_town_loc_type = locations_tab_ptr->loctype;
			gs_town_typeindex = locations_tab_ptr->typeindex;
			gs_town_locdata = locations_tab_ptr->locdata;

			if (gs_town_loc_type == LOCTYPE_MARKET) {
				gs_town_loc_type = LOCTYPE_NONE;
				g_location_market_flag = 1;
			}

			return 1;
		}

		locations_tab_ptr++;

	} while (locations_tab_ptr->pos != -1);

	update_visual_squares();

	/* get_town_tile_type() hat hier noch keinen Prototyp (er steht erst vor town_set_visual_square_vals),
	 * deshalb wird das Argument auf int erweitert (mov ah,0), wie im Original. */
	if (((tile_type = get_town_tile_type(g_visual_square_vals[1])) >= 2) && (tile_type <= 5)) {

		gs_town_loc_type_bak = LOCTYPE_NONE;
		// assert(g_town_num_locations_table[town_id - 1] == (locations_tab_ptr - g_locations_tab)
		gs_town_locdata = g_town_num_locations_table[town_id - 1];

		if (!((gs_viewdir + gs_x + gs_y) & 1)) {
			gs_town_loc_type = LOCTYPE_CITIZEN;
		} else {
			gs_town_loc_type = LOCTYPE_HOUSE;

			gs_town_locdata++;
			// gs_town_locdata is now number_of_locations + 1.
			// It will be used in do_house() as index for the text retrieval for the status line from <TOWN>.LTX,
			// In this file, the first entries are the names of the locations, followed by the message entering a house.
			// Example for Thorwal: <TAGCHEN! IHR WOLLT SICHER NICHT ZU MIR. ALSO RAUS!>
		}

		return 1;
	}

	return 0;
}

/**
 * \brief   enter_location() for Daspota: talks with the robbers, fights and loot in their houses
 *
 * \return              1 = location handled, 0 = nothing
 */
signed int enter_location_daspota(void)
{
	signed int map_pos;
	signed int tile_type;
	struct location *locations_tab_ptr;

	if (g_game_state == GAME_STATE_FIGQUIT) {
		return 1;
	}

	map_pos = 256 * gs_x + gs_y;
	locations_tab_ptr = &g_locations_tab[0];
	g_location_market_flag = 0;

	do {

		if (locations_tab_ptr->pos == map_pos) {

			gs_town_typeindex = locations_tab_ptr->typeindex;

			if (locations_tab_ptr->loctype != LOCTYPE_SIGNPOST) {

				GUI_print_loc_line(get_tx(locations_tab_ptr->locdata));

				if (!gs_daspota_fightflags[locations_tab_ptr->locdata]) {

					do_talk(locations_tab_ptr->loctype, locations_tab_ptr->typeindex - 1);

					if (!gs_daspota_fightflags[locations_tab_ptr->locdata]) {

						leave_location();
						return 1;
					}
				}

				draw_main_screen();
				disable_ani();

				load_ani(10);
				GUI_print_loc_line(get_tx(locations_tab_ptr->locdata));
				init_ani(0);

				if (g_daspota_locloot_index[locations_tab_ptr->locdata - 1]) {

					loot_multi_chest(g_daspota_locloot_index[locations_tab_ptr->locdata - 1], get_tx(21)); /* "IN DIESEM HAUS FINDET IHR:" */

				} else {

					do {
						handle_gui_input();

					} while (g_action_id == 0 && g_mouse1_event2 == 0);

					g_mouse1_event2 = 0;
				}

				disable_ani();

				if (locations_tab_ptr->locdata == 6) {
					do_fight(FIGHT_ID_DASP6B);
				} else if (locations_tab_ptr->locdata == 12) {
					do_fight(FIGHT_ID_DASP12B);
				}

				leave_location();

			} else {
				gs_town_loc_type_bak = LOCTYPE_NONE;
				gs_town_loc_type = locations_tab_ptr->loctype;
				gs_town_locdata = locations_tab_ptr->locdata;
			}

			return 1;
		}

		locations_tab_ptr++;

	} while (locations_tab_ptr->pos != -1);

	update_visual_squares();

	/* unprototyped call, see enter_location() */
	if ((tile_type = get_town_tile_type(g_visual_square_vals[1])) >= 2 && tile_type <= 5) {

		gs_town_loc_type_bak = LOCTYPE_NONE;
		gs_town_loc_type = LOCTYPE_CITIZEN;
		gs_town_locdata = 19;
		return 1;
	}

	return 0;
}

/**
 * \brief   location handler of special buildings in Thorwal, Phexcaer and at the Einsiedlersee
 */
void do_special_buildings(void)
{
	signed int typeindex;
	const signed int tw_bak = g_textbox_width;

	typeindex = gs_town_typeindex;

	if (gs_town_id == TOWN_ID_THORWAL) {

		load_tx2(typeindex < 41 ? ARCHIVE_FILE_THORWAL1_LTX : ARCHIVE_FILE_THORWAL2_LTX);
		g_textbox_width = 9;

		if (typeindex == 28) {
			THO_hetmann();
		} else if (typeindex == 29) {
			THO_windriders();
		} else if (typeindex == 30) {
			THO_stormchildren();
		} else if (typeindex == 31) {
			THO_garaldsson();
		} else if (typeindex == 32) {
			THO_eisenhof();
		} else if (typeindex == 33) {
			THO_imman();
		} else if (typeindex == 34) {
			THO_botschaft();
		} else if (typeindex == 35) {
			GUI_output(get_tx2(70)); /* "UM ZUM HJALDINGARD-TURM ZU GELANGEN, MÜSSTET IHR EINE NATÜRLICHE BR..." */
		} else if (typeindex == 36) {
			GUI_output(get_tx2(71)); /* "IHR STEHT VOR DER BOTSCHAFT DES ALTEN REICHES, EINEM PRUNKVOLLEN BA..." */
		} else if (typeindex == 37) {
			THO_bank();
		} else if (typeindex == 38) {
			GUI_output(get_tx2(82)); /* "DIES IST DIE STATION DER BEILUNKER REITER. VON HIER AUS KÖNNTET IHR..." */
		} else if (typeindex == 39) {
			GUI_output(get_tx2(83)); /* "IHR STEHT VOR EINEM ETWA DREI STOCKWERKE HOHEN TURM, DER GANZ AUS@D..." */
		} else if (typeindex == 40) {
			GUI_output(get_tx2(84)); /* "DIES IST DER EINGANG ZUR WERFT VON ANSIR ARILSSON, DEM GRÖSSTEN THO..." */
		} else if (typeindex == 41) {
			THO_arsenal();
		} else if (typeindex == 42) {
			THO_magistracy();
		} else if (typeindex == 43) {
			THO_mueller();
		} else if (typeindex == 44) {
			THO_black_finger();
		} else if (typeindex == 45) {
			THO_ugdalf();
		} else if (typeindex == 46) {
			THO_academy();
		} else if (typeindex == 11 || typeindex == 14 || typeindex == 17 || typeindex == 21) {
			THO_tav_inn_combi();
		}

	} else if (gs_town_id == TOWN_ID_PHEXCAER) {

		load_tx2(typeindex <= 3 ? ARCHIVE_FILE_PHEX2_LTX : ARCHIVE_FILE_PHEX1_LTX);
		g_textbox_width = 9;

		if (typeindex == 1) {
			PHX_phextempel();
		} else if (typeindex == 3) {
			PHX_drogentempel();
		} else if (typeindex == 18) {
			PHX_apotheke();
		} else if (typeindex == 20) {
			PHX_healer();
		} else if (typeindex == 23) {
			PHX_bordell();
		} else if (typeindex == 24) {
			PHX_villa_gremob();
		} else if (typeindex == 25) {
			PHX_spielhaus();
		} else if (typeindex == 26) {
			PHX_fuhrhaus();
		} else if (typeindex == 27) {
			PHX_stadthaus();
		}


	} else if (gs_town_id == TOWN_ID_EINSIEDLERSEE) {
		/*  HERMITS LAKE / EINSIEDLERSEE */

		if (typeindex == 1) {
			load_ani(8);
			init_ani(1);
			do_talk(18, 0);
			disable_ani();
		}
	}

	g_textbox_width = tw_bak;
	leave_location();
}

/**
 * \brief   dialog logic of the hermit at the Einsiedlersee (EREMIT.TLK)
 *
 * \param   state       the current dialog state
 */
void TLK_eremit(const signed int state)
{
	if (!state) {

		g_dialog_next_state = (gs_hermit_visited ? 1 : 2);

	} else if (state == 6) {

		signed int i;
		struct struct_hero *hero = get_hero(0);

		/* REMARK: all hero slots, without checking type or group */
		for (i = 0 ; i <= 6; i++, hero++) {

			/* remove hunger and thirst */
			hero->hunger = hero->thirst = 0;

			/* heal all wounds */
			add_hero_le(hero, hero->le_max);
		}

	} else if (state == 10) {

		/* group learns about two places to rest */
		gs_tevent137_flag = gs_tevent134_flag = 1;

	} else if (state == 13) {

		gs_hermit_visited = 1;

	} else if (state == 14) {

		timewarp(MINUTES(30));
	}
}

/**
 * \brief   one step of the town loop: prepare the town if needed, then town_step()
 */
void do_town(void)
{
	if ((g_town_loaded_town_id != gs_town_id) || (g_area_prepared != AREA_TYPE_TOWN))
	{
		prepare_area(1);

		set_audio_track(ARCHIVE_FILE_THORWAL_XMI);

		g_request_refresh = 1;

		diary_new_entry();
	}

	g_current_ani = -1;

	gs_town_id_bak = gs_town_id;

	town_step();
}

static void refresh_floor_and_sky(void)
{
	signed int width;
	signed int height;
	struct nvf_extract_desc nvf;

	nvf.dst = g_renderbuf_ptr;
	nvf.src = g_town_tex[1]; // tex_sky
	nvf.image_num = 0;
	nvf.compression_type = 3;
	nvf.width = &width;
	nvf.height = &height;

	process_nvf_extraction(&nvf);

	nvf.dst = ((uint8_t*)g_renderbuf_ptr) + 208 * height;
	nvf.src = g_town_tex[0];
	nvf.image_num = 0;
	nvf.compression_type = 3;
	nvf.width = &width;
	nvf.height = &height;

	process_nvf_extraction(&nvf);
}

/**
 * \brief   redraw the street view
 */
void town_update_view(void)
{
	refresh_floor_and_sky();
	update_visual_squares();
	town_set_visual_square_vals();
	town_set_visual_square_tex();

	/* TODO: these are write only variables */
	g_always_zero2 = g_always_zero1 = 0;

	town_water_and_grass();
	town_building_textures();
	town_fade_and_colors();
}

signed int get_town_tile_type(const unsigned char);

/**
 * \brief   clear the visual squares that are hidden behind houses (TOWN_TILE_HOUSE_1..4)
 */
void town_set_visual_square_vals(void)
{
	signed int bi;

	if ((bi = get_town_tile_type(g_visual_square_vals[3])) >= 2 && bi <= 5) {
		g_visual_square_vals[3] = 0;
	}

	if ((bi = get_town_tile_type(g_visual_square_vals[7])) >= 2 && bi <= 5) {
		g_visual_square_vals[7] = 0;
	}

	if ((bi = get_town_tile_type(g_visual_square_vals[8])) >= 2 && bi <= 5) {
		g_visual_square_vals[8] = 0;
	}

	if ((bi = get_town_tile_type(g_visual_square_vals[14])) >= 2 && bi <= 5) {
		g_visual_square_vals[14] = 0;
	}

	if ((bi = get_town_tile_type(g_visual_square_vals[15])) >= 2 && bi <= 5) {
		g_visual_square_vals[15] = 0;
	}

	if ((bi = get_town_tile_type(g_visual_square_vals[21])) >= 2 && bi <= 5) {
		g_visual_square_vals[21] = 0;
	}

	if ((bi = get_town_tile_type(g_visual_square_vals[0])) >= 2 && bi <= 5) {

		g_visual_square_vals[9] = g_visual_square_vals[15] = g_visual_square_vals[22] = 0;

		if ((bi = get_town_tile_type(g_visual_square_vals[5])) >= 2 && bi <= 5) {
			g_visual_square_vals[4] = g_visual_square_vals[10] = g_visual_square_vals[16] = g_visual_square_vals[23] = 0;
		}
	}

	if ((bi = get_town_tile_type(g_visual_square_vals[2])) >= 2 && bi <= 5) {

		g_visual_square_vals[13] = g_visual_square_vals[21] = g_visual_square_vals[28] = 0;

		if ((bi = get_town_tile_type(g_visual_square_vals[5])) >= 2 && bi <= 5) {
			g_visual_square_vals[6] = g_visual_square_vals[12] = g_visual_square_vals[20] = g_visual_square_vals[27] = 0;
		}
	}

	if ((bi = get_town_tile_type(g_visual_square_vals[4])) >= 2 && bi <= 5) {

		g_visual_square_vals[9] = g_visual_square_vals[16] = g_visual_square_vals[22] = g_visual_square_vals[23] = 0;

		if ((bi = get_town_tile_type(g_visual_square_vals[5])) >= 2 && bi <= 5) {
			g_visual_square_vals[10] = 0;
		}

		if ((bi = get_town_tile_type(g_visual_square_vals[11])) >= 2 && bi <= 5) {
			g_visual_square_vals[10] = g_visual_square_vals[17] = g_visual_square_vals[18] = 0;
			g_visual_square_vals[24] = g_visual_square_vals[25] = 0;
		}
	}

	if ((bi = get_town_tile_type(g_visual_square_vals[5])) >= 2 && bi <= 5) {

		g_visual_square_vals[11] = g_visual_square_vals[17] = g_visual_square_vals[18] = 0;

		g_visual_square_vals[19] = g_visual_square_vals[24] = g_visual_square_vals[25] = 0;

		g_visual_square_vals[26] = 0;
	}

	if ((bi = get_town_tile_type(g_visual_square_vals[6])) >= 2 && bi <= 5) {

		g_visual_square_vals[13] = g_visual_square_vals[20] = g_visual_square_vals[27] = g_visual_square_vals[28] = 0;

		if ((bi = get_town_tile_type(g_visual_square_vals[5])) >= 2 && bi <= 5) {

			g_visual_square_vals[11] = g_visual_square_vals[12] = g_visual_square_vals[17] = 0;

			g_visual_square_vals[18] = g_visual_square_vals[19] = g_visual_square_vals[24] = 0;

			g_visual_square_vals[25] = g_visual_square_vals[26] = 0;
		}

		if ((bi = get_town_tile_type(g_visual_square_vals[11])) >= 2 && bi <= 5) {

			g_visual_square_vals[12] = g_visual_square_vals[18] = g_visual_square_vals[19] = 0;

			g_visual_square_vals[25] = g_visual_square_vals[26] = 0;
		}
	}

	if ((bi = get_town_tile_type(g_visual_square_vals[9])) >= 2 && bi <= 5) {

		g_visual_square_vals[22] = 0;

		if ((bi = get_town_tile_type(g_visual_square_vals[17])) >= 2 && bi <= 5) {
			g_visual_square_vals[16] = g_visual_square_vals[23] = 0;
		}
	}

	if ((bi = get_town_tile_type(g_visual_square_vals[13])) >= 2 && bi <= 5) {

		g_visual_square_vals[28] = 0;

		if ((bi = get_town_tile_type(g_visual_square_vals[19])) >= 2 && bi <= 5) {
			g_visual_square_vals[20] = g_visual_square_vals[27] = 0;
		}
	}

	if ((bi = get_town_tile_type(g_visual_square_vals[10])) >= 2 && bi <= 5) {

		g_visual_square_vals[9] = g_visual_square_vals[16] = g_visual_square_vals[22] = g_visual_square_vals[23] = 0;

		if ((bi = get_town_tile_type(g_visual_square_vals[11])) >= 2 && bi <= 5) {
			g_visual_square_vals[18] = 0;
		}

		if (((bi = get_town_tile_type(g_visual_square_vals[11])) >= 2 && bi <= 5) ||
			((bi = get_town_tile_type(g_visual_square_vals[18])) >= 2 && bi <= 5)) {

			g_visual_square_vals[17] = g_visual_square_vals[24] = g_visual_square_vals[25] = 0;
		}
	}

	if ((bi = get_town_tile_type(g_visual_square_vals[12])) >= 2 && bi <= 5) {

		g_visual_square_vals[13] = g_visual_square_vals[20] = g_visual_square_vals[27] = g_visual_square_vals[28] = 0;

		if ((bi = get_town_tile_type(g_visual_square_vals[11])) >= 2 && bi <= 5) {
			g_visual_square_vals[18] = 0;
		}

		if (((bi = get_town_tile_type(g_visual_square_vals[11])) >= 2 && bi <= 5) ||
			((bi = get_town_tile_type(g_visual_square_vals[18])) >= 2 && bi <= 5)) {

			g_visual_square_vals[19] = g_visual_square_vals[25] = g_visual_square_vals[26] = 0;
		}
	}

	if ((bi = get_town_tile_type(g_visual_square_vals[11])) >= 2 && bi <= 5) {
		g_visual_square_vals[18] = g_visual_square_vals[25] = 0;
	}

	if ((bi = get_town_tile_type(g_visual_square_vals[16])) >= 2 && bi <= 5) {

		g_visual_square_vals[22] = 0;

		if ((bi = get_town_tile_type(g_visual_square_vals[24])) >= 2 && bi <= 5) {
			g_visual_square_vals[23] = 0;
		}
	}

	if ((bi = get_town_tile_type(g_visual_square_vals[20])) >= 2 && bi <= 5) {

		g_visual_square_vals[28] = 0;

		if ((bi = get_town_tile_type(g_visual_square_vals[26])) >= 2 && bi <= 5) {
			g_visual_square_vals[27] = 0;
		}
	}

	if ((bi = get_town_tile_type(g_visual_square_vals[18])) >= 2 && bi <= 5) {
		g_visual_square_vals[25] = 0;
	}

	if (((bi = get_town_tile_type(g_visual_square_vals[18])) >= 2 && bi <= 5) ||
		((bi = get_town_tile_type(g_visual_square_vals[25])) >= 2 && bi <= 5)) {

		if ((bi = get_town_tile_type(g_visual_square_vals[17])) >= 2 && bi <= 5) {
			g_visual_square_vals[24] = 0;
		}

		if ((bi = get_town_tile_type(g_visual_square_vals[19])) >= 2 && bi <= 5) {
			g_visual_square_vals[26] = 0;
		}
	}
}

/**
 * \brief   TOWN_TILE_* type (0..10) of a town map value; also sets g_entrance_angle
 *
 * \param   val         map value (bits 0..1: direction of the entrance)
 * \return              TOWN_TILE_* (0 = street, 1 = temple, 2..5 = houses, 6 = water, 7 = grass,
 *                      8 = signpost, 9 = inn/tavern, 10 = merchant)
 *
 * REMARK: for val >= 0xf0 the loop reads beyond g_mapval_to_loctype[] (into the next table).
 */
signed int get_town_tile_type(const unsigned char val)
{
	signed int i;

	i = 0;
	while (g_mapval_to_town_tile[i] < val) {
		i++;
	}

	g_entrance_angle = ((((val & 3) + 4) - gs_viewdir) & 3);

	if (i == 0)
		i = 1;

	i--;

	return i;
}

/**
 * \brief   choose the texture of each visual square by its TOWN_TILE_* type
 *
 * REMARK: get_town_tile_type() never returns -1, so the first branch is never taken.
 * Temples use the textures of house 3.
 */
void town_set_visual_square_tex(void)
{
	signed int i;
	signed int bi;

	for (i = 28; i >= 0; i--) {

		bi = get_town_tile_type(g_visual_square_vals[i]);

		if (bi == -1) {
			g_visual_squares_tex[i] = -1;
		} else {
			g_visual_squares_tex[i] = (
						bi == 2 ? g_visual_square_tex_house1[i] : (
						bi == 3 ? g_visual_square_tex_house2[i] : (
						bi == 4 ? g_visual_square_tex_house3[i] : (
						bi == 5 ? g_visual_square_tex_house4[i] : (
						bi == 6 ? g_visual_square_tex_water_grass[i] : (
						bi == 7 ? g_visual_square_tex_water_grass[i] : (
						bi == 8 ? g_visual_square_tex_signpost[i] : (
						bi == 9 ? g_visual_square_tex_inn[i] : (
						bi == 10 ? g_visual_square_tex_merchant[i] : (
						bi == 1 ? g_visual_square_tex_house3[i] : -1))))))))));
		}
	}
}

/**
 * \brief   draws water and grass textures
 */
void town_water_and_grass(void)
{
	signed int i;
	signed int nvf_no;
	signed int x;
	signed int y;
	signed char c1;
	signed char bi;
	unsigned char c2;
	int16_t *ptr;

	for (i = 0; i < 29; i++) {

		c1 = g_visual_square_draw_order[i];
		c2 = g_visual_square_vals[c1];

		if (c2 != 0) {

			bi = get_town_tile_type(c2);

			if (bi == 6 || bi == 7) {

				/* water or grass */
				ptr = (int16_t*)&g_visual_square_offsets_grass[c1];

				x = ptr[0];
				y = ptr[1];

				c1 = g_visual_squares_tex[c1];

				if (c1 != -1) {

					ptr = (int16_t*)&g_tex_descr_table[c1 - 1][0];

					if ((nvf_no = ptr[2]) != -1) {

						if (bi == 7) {
							nvf_no += 15;
						}

						load_town_texture(x + ptr[0], y + ptr[1], nvf_no, 184);
					}
				}
			}
		}
	}
}

/**
 * \brief   draws building textures
 */
void town_building_textures(void)
{
	signed int nvf_no;
	signed int i;
	signed int x;
	signed int y;
	signed int tex_set;
	signed char c1;
	signed char bi;
	unsigned char c2;
	int16_t *ptr;

	for (i = 0; i < 29; i++) {

		c1 = g_visual_square_draw_order[i];
		c2 = g_visual_square_vals[c1];

		if (c2 != 0) {

			bi = get_town_tile_type(c2);

			if (bi != TOWN_TILE_GRASS && bi != TOWN_TILE_WATER) {

				ptr = (int16_t*)&g_visual_square_offsets_std[c1];

				if (bi == TOWN_TILE_SIGNPOST) {
					ptr = (int16_t*)&g_visual_square_offsets_sign[c1];
				} else if (bi == TOWN_TILE_INN_OR_TAVERN || bi == TOWN_TILE_MERCHANT) {
					ptr = (int16_t*)&g_visual_square_offsets_inn[c1];
				}

				x = ptr[0];
				y = ptr[1];

				c1 = g_visual_squares_tex[c1];

				if (c1 != -1) {

					ptr = (int16_t*)&g_tex_descr_table[c1 - 1][0];

					tex_set =	bi == TOWN_TILE_HOUSE_1 ? 186 : (
						bi == TOWN_TILE_HOUSE_2 ? 187 : (
						bi == TOWN_TILE_HOUSE_3 ? 188 : (
						bi == TOWN_TILE_HOUSE_4 ? 189 : (
						bi == TOWN_TILE_TEMPLE ? 188 : (
						bi == TOWN_TILE_INN_OR_TAVERN ? 232 : (
						bi == TOWN_TILE_MERCHANT ? 233 : 185))))));

					if ((nvf_no = ptr[2]) != -1) {

						if (g_entrance_angle == 2 && bi >= TOWN_TILE_TEMPLE && bi <= TOWN_TILE_HOUSE_4) {

							if (bi == TOWN_TILE_TEMPLE) {
								nvf_no -= 5;
								tex_set = 185;
							} else {
								nvf_no -= 10;
							}
						}

						if (bi == TOWN_TILE_INN_OR_TAVERN || bi == TOWN_TILE_MERCHANT) {
							load_special_textures(bi);
						}

						load_town_texture(x + ptr[0], y + ptr[1], nvf_no, tex_set);

						if (bi == TOWN_TILE_INN_OR_TAVERN || bi == TOWN_TILE_MERCHANT) {
							call_load_buffer();
						}
					}

					if ((nvf_no = ptr[5]) != -1) {

						if (bi == TOWN_TILE_TEMPLE) {
							tex_set = 188;
						}

						if (g_entrance_angle == 1 && !(nvf_no & 0x8000) &&
							bi >= TOWN_TILE_TEMPLE && bi <= TOWN_TILE_HOUSE_4)
						{
							nvf_no -= 10;

						} else if (g_entrance_angle == 3 && (nvf_no & 0x8000))
						{
							nvf_no = ((uint16_t)(nvf_no & 0x7fff) - 10) | 0x8000;
						}

						load_town_texture(x + ptr[3], y + ptr[4], nvf_no, tex_set);
					}

					if ((nvf_no = ptr[8]) != -1) {
						load_town_texture(x + ptr[6], y + ptr[7], nvf_no, tex_set);
					}
				}
			}
		}
	}
}

/**
 * \brief   extract a town texture and copy it into the street view (208 x 135)
 *
 * \param   x, y        position in the view
 * \param   nvf_no      image number, bit 15 = mirrored
 * \param   tex_set     texture set: 184 + index into g_town_tex (185 = TDIVERSE, 186..189 = houses),
 *                      232/233 = special textures of inn/merchant in g_buffer7_ptr
 */
void load_town_texture(signed int x, signed int y, signed int nvf_no, signed int tex_set)
{
	signed int width;
	signed int height;
	signed int copy_width;
	signed int copy_height;
	signed int direction;
	uint8_t *src;
	uint8_t *dst;
	struct nvf_extract_desc nvf;

	direction = (nvf_no & 0x8000 ? 1: 0);
	nvf_no &= 0x3fff;

	tex_set -= 184;

	nvf.dst = src = g_renderbuf_ptr + 30000;

	/*
	 * the following line accesses memory outside of the
	 * texture array if tex_set is 48 or 49!?
	 * Yes, but only reads a pointer, which is replaced in the next line (harmless).
	 */
	nvf.src = g_town_tex[tex_set];

	if (tex_set == 48 || tex_set == 49) {
		nvf.src = (uint8_t*)g_buffer7_ptr;
	}

	nvf.image_num = nvf_no;
	nvf.compression_type = (direction == 0 ? 3 : 5);
	nvf.width = &width;
	nvf.height = &height;
	process_nvf_extraction(&nvf);

	copy_width = width;
	copy_height = height;

	if (x < 0) {
		if (x + copy_width > 0) {
			copy_width += x;
			src += __abs__(x);
			x = 0;
		}
	}

	if (y < 0) {
		if (y + copy_height > 0) {
			/* Original-Bug? should be copy_height += y (like copy_width += x); with y < 0
			 * this copies |y| rows too many. Probably never reached with the given tables. */
			copy_height -= y;
			src += __abs__(y) * width;
			y = 0;
		}
	}

	if ((x < 208) && (y < 135) && (x >= 0) && (y >= 0)) {

		if (x + copy_width > 208) {
			copy_width = 208 - x;
		}

		if (y + copy_height > 135) {
			copy_height = 135 - y;
		}

		dst = ((uint8_t*)g_renderbuf_ptr) + y * 208 + x;

		copy_solid(dst, src, copy_width, copy_height, 208, width, tex_set == 0 ? 0 : 128);
	}
}

/**
 * \brief   redraw the view and remember position and direction
 */
void town_direction_change(void)
{
	disable_ani();
	town_update_view();
	g_town_refresh_x_target = gs_x;
	g_town_refresh_y_target = gs_y;
	g_town_refresh_direction = gs_viewdir;
}

/**
 * \brief   menu, movement and events in a town
 *
 * \return              always 0
 */
signed int town_step(void)
{
	signed int i;
	signed int bi;
	signed int options;
	signed int icon1_bak;

	g_new_menu_icons[0] = MENU_ICON_SPLIT_GROUP;
	icon1_bak = g_new_menu_icons[1];
	g_new_menu_icons[1] = (g_can_merge_group == -1 ? MENU_ICON_MERGE_GROUP_GRAYED : MENU_ICON_MERGE_GROUP);

	if (g_new_menu_icons[1] != icon1_bak) {
		g_redraw_menuicons = 1;
	}

	g_new_menu_icons[2] = MENU_ICON_SWITCH_GROUP;
	g_new_menu_icons[3] = MENU_ICON_INFO;
	g_new_menu_icons[4] = MENU_ICON_MAP;
	g_new_menu_icons[5] = MENU_ICON_MAGIC;
	g_new_menu_icons[6] = MENU_ICON_CAMP;

	if (g_request_refresh != 0) {

		draw_main_screen();
		GUI_print_loc_line(get_tx(0)); /* Stadt-LTX[0]: Ortsname */

		g_request_refresh = g_redraw_menuicons = 0;
		g_town_refresh_x_target = -1;
	}

	if (g_redraw_menuicons && (g_pp20_index == ARCHIVE_FILE_PLAYM_UK)) {
		draw_icons();
		g_redraw_menuicons = 0;
	}

	/* check if position or direction has changed */
	if (gs_viewdir != g_town_refresh_direction ||
		gs_x != g_town_refresh_x_target ||
		gs_y != g_town_refresh_y_target)
	{
		town_direction_change();
	}

	if ((gs_x != gs_x_bak) || (gs_y != gs_y_bak))
	{
		g_can_merge_group = can_merge_group();
		set_automap_tiles(gs_x, gs_y);
	}

	gs_x_bak = gs_x;
	gs_y_bak = gs_y;

	handle_gui_input();

	if (g_mouse_rightclick_event || g_action_id == KEY_SCAN_CODE_PAGE_UP) {

		for (i = options = 0; i < 9; i++) {
			if (g_new_menu_icons[i] != MENU_ICON_NONE) {
				options++;
			}
		}

		i = GUI_radio(get_ttx(570), (signed char)options, /* "STADT OPTIONEN" */
				get_ttx(535), get_ttx(536), get_ttx(537), /* "GRUPPE TEILEN" / "GRUPPEN VEREINEN" / "NÄCHSTE GRUPPE" */
				get_ttx(538), get_ttx(539), get_ttx(213), /* "SPIELSTAND" / "KARTE" / "ZAUBER SPRECHEN" */
				get_ttx(306), get_ttx(569)) - 1; /* "LAGER" / "MARKT BESUCHEN" */

		if (i != -2) {
			g_action_id = i + ACTION_ID_ICON_1;
		}
	}

	i = 0;

	if (g_action_id == ACTION_ID_ICON_1) {

		GRP_split();
		g_can_merge_group = can_merge_group();

	} else if (g_action_id == ACTION_ID_ICON_2) {

		GRP_merge();
		g_can_merge_group = -1;

	} else if (g_action_id == ACTION_ID_ICON_3) {

		GRP_switch_to_next(0);
		i = 1;

	} else if (g_action_id == ACTION_ID_ICON_4) {

		game_options();

	} else if (g_action_id == ACTION_ID_ICON_5) {

		show_automap();

	} else if (g_action_id == ACTION_ID_ICON_6) {

		select_magic_user();

	} else if (g_action_id == ACTION_ID_ICON_7) {

		gs_town_loc_type = LOCTYPE_AREA_CAMP;
		g_area_camp_area_type = AREA_TYPE_TOWN; /* AREA_CAMP takes place in a town */
		i = 1;

	} else if (g_action_id == ACTION_ID_ICON_8 && g_new_menu_icons[7] != MENU_ICON_NONE) {

		gs_town_loc_type = LOCTYPE_MARKET;
		i = 1;

	} else if (g_action_id == KEY_SCAN_CODE_ARROW_LEFT) {

		update_direction(3);

	} else if (g_action_id == KEY_SCAN_CODE_ARROW_RIGHT) {

		update_direction(1);

	} else if (g_action_id == KEY_SCAN_CODE_ARROW_UP) {

		bi = get_town_tile_type(g_steptarget_front);

		if (!bi || bi == 7 || bi == 8) {
			town_do_step(1);
		} else if (bi >= 1 && bi <= 5 && g_entrance_angle == 2) {
			town_do_step(1);
		} else {
			no_way();
		}

	} else if (g_action_id == KEY_SCAN_CODE_ARROW_DOWN) {

		bi = get_town_tile_type(g_steptarget_back);

		if (!bi || bi == 7 || bi == 8) {
			town_do_step(-1);
		} else {
			no_way();
		}
	}

	if ((gs_town_id != TOWN_ID_NONE) && (g_town_loaded_town_id != -1)) {

		if (!i) {
			options = enter_location(gs_town_id);
		}

		/* random city event? */
		/* check if the party has moved to another square */
		if (((gs_y != gs_y_bak) || (gs_x != gs_x_bak)) &&

			/* only in big town */
			(gs_town_id == TOWN_ID_THORWAL || gs_town_id == TOWN_ID_PREM ||
			gs_town_id == TOWN_ID_PHEXCAER || gs_town_id == TOWN_ID_OBERORKEN))
		{

		       	/* 1% chance only between 8:00 and 20:00 o'clock */
			if ((random_schick(100) <= 1) && (gs_day_timer > HOURS(8)) && (gs_day_timer < HOURS(20)))
			{
				city_event_switch();
			}
		}

		if (g_location_market_flag && g_new_menu_icons[7] != MENU_ICON_MARKET) {

			if (((i = g_market_descr_table[gs_town_typeindex].market_day) == -1 || gs_day_of_week == i) &&
				gs_day_timer >= HOURS(6) && gs_day_timer <= HOURS(16))
			{
				g_new_menu_icons[7] = MENU_ICON_MARKET;
				draw_icons();
			}

		} else if (!g_location_market_flag && (g_new_menu_icons[7] == MENU_ICON_MARKET)) {

			g_new_menu_icons[7] = MENU_ICON_NONE;
			draw_icons();
		}
	}

	return 0;
}

/*
 * \brief execute a step if possible
 * \param[in] forward {1 = forward, -1 = backward}
 */
void town_do_step(const signed int forward)
{
	signed int viewdir;

	timewarp(MINUTES(2));

	viewdir = gs_viewdir;

	if (forward == 1) {

		if (!viewdir) { // viewdir == NORTH
			gs_y--;
		} else if (viewdir == EAST) {
			gs_x++;
		} else if (viewdir == SOUTH) {
			gs_y++;
		} else { // viewdir == WEST
			gs_x--;
		}

	} else {

		if (!viewdir) { // viewdir == NORTH
			gs_y++;
		} else if (viewdir == EAST) {
			gs_x--;
		} else if (viewdir == SOUTH) {
			gs_y--;
		} else { // viewdir == WEST
			gs_x++;
		}
	}

	if (gs_x < 0) {

		gs_x = 0;
		no_way();

	} else if ((g_map_size_x - 1) < gs_x) {

		gs_x = g_map_size_x - 1;
		no_way();
	}

	if (gs_y < 0) {

		gs_y = 0;
		no_way();

	} else if (gs_y > 15) {

		gs_y = 15;
		no_way();
	}
}

/**
 * \brief   show the street view, fade in if requested and set the palettes
 */
void town_fade_and_colors(void)
{
	signed int i;
	uint8_t *dst;
	uint8_t *pal_ptr;

	if (g_fading_state == 2) {

		fade_into();
		g_fading_state = 1;

	}

	if (g_fading_state == 3) {

		set_palette(g_palette_allblack2, 0x00, 0x20);
		set_palette(g_palette_allblack2, 0x80, 0x20);
		set_palette(g_palette_allblack2, 0xa0, 0x20);

		g_fading_state = 1;
	}

	draw_compass();

	g_pic_copy.x1 = g_ani_posx;
	g_pic_copy.y1 = g_ani_posy;
	g_pic_copy.x2 = g_ani_posx + 207;
	g_pic_copy.y2 = g_ani_posy + 134;
	g_pic_copy.src = g_renderbuf_ptr;

	g_special_screen = 0;

	call_mouse_bg();
	wait_for_vsync();

	do_pic_copy(1);

	call_mouse();

	if (g_fading_state != 0) {

		dst = ((uint8_t*)g_renderbuf_ptr) + 500;
		pal_ptr = g_renderbuf_ptr;

#if defined(__BORLANDC__)
		/* 0x120 = gs_palette_floor, gs_palette_buildings and gs_palette_sky, which lie one after another */
		memset(g_renderbuf_ptr, 0, 0x120);
		memcpy(dst, gs_palette_floor, 0x120);
#else
		memset(g_renderbuf_ptr, 0, 3 * 0x20);
		memcpy(dst, gs_palette_floor, 3 * 0x20);
#endif

		for (i = 0; i < 64; i += 2) {

			pal_fade_in((int8_t*)pal_ptr, (int8_t*)dst, i, 0x60);
			pal_fade_in((int8_t*)pal_ptr, (int8_t*)dst, i + 1, 0x60);

			wait_for_vsync();

			set_palette(pal_ptr, 0x00, 0x20);
			set_palette(pal_ptr + 0x60, 0x80, 0x40);
		}

		g_fading_state = 0;

	} else {

		wait_for_vsync();

		set_palette(gs_palette_floor, 0x00, 0x20);
		set_palette(gs_palette_buildings, 0x80, 0x40);
	}
}

/**
 * \brief   count the four kinds of houses on the town map; the rarest kind is set to 0
 *          and its textures are not loaded (seg028)
 */
void town_update_house_count(void)
{
	signed int l_si;
	signed int i;
	uint8_t *map_ptr = g_dng_map;

	g_town_house_count[0] = g_town_house_count[1]
				= g_town_house_count[2] = g_town_house_count[3] = 0;

	for (i = 0; g_map_size_x * 16 > i; i++) {

		l_si = get_town_tile_type(map_ptr[i]);

		/* count number of houses of certain kind */
		if (l_si == 2) {
			g_town_house_count[0]++;
		} else if (l_si == 3) {
			g_town_house_count[1]++;
		} else if ((l_si == 4) || (l_si == 1)) {
			g_town_house_count[2]++;
		} else if (l_si == 5) {
			g_town_house_count[3]++;
		}
	}

	i = 5;
	l_si = 2000;

	/* find house with lowest count on current map */
	if (g_town_house_count[0] < l_si) {
		l_si = g_town_house_count[(i = 0)];
	}

	if (g_town_house_count[1] < l_si) {
		l_si = g_town_house_count[(i = 1)];
	}

	if (g_town_house_count[2] < l_si) {
		l_si = g_town_house_count[(i = 2)];
	}

	if (g_town_house_count[3] < l_si) {
		l_si = g_town_house_count[(i = 3)];
	}

	/* the kind of house with lowest count is deactivated, i.e. it's texture is
	 * not loaded and replaced by another texture in prepare_town_area */
	g_town_house_count[i] = 0;
}
