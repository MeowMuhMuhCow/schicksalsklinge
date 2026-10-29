/*
 * seg045 - Kampfanimation: Geschosse und Zauberwirkung
 *
 * Inhalt:    Legt Geschosse (Zauberkugel, -blitz, -stern, Pfeil/Bolzen, Wurfwaffe, Speer)
 *            als Kampffigur an und erzeugt ihre Flugbahn Feld fuer Feld bis zum Ziel (nur
 *            ab Entfernung 2), ebenso die Wirkungsgrafik von Zaubern (Kuppel, Flamme) am
 *            Zielfeld, und entfernt sie wieder.
 * Aufrufer:  FIG_do_hero_action() (seg042), FIG_do_enemy_action() und FIG_use_item()
 *            (seg043).
 * Daten:     Tabellen fuer Geschossgrafik, -groesse und -bahn je Richtung; g_anitab_spell_*
 *            fuer die Wirkungen.
 * Besonders: g_anitab_spell_* haben nur Eintraege fuer die Wirkungen 1 (Kuppel) und 2
 *            (Flamme); Hexenknoten (3) erreicht die Funktionen nicht, weil er ohne Ziel
 *            gesprochen wird.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg045.c
 */
#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg006.h"
#include "seg038.h"
#include "seg039.h"

static const signed int g_gfxtab_projectile_ox[6][FIG_VIEWDIR__END] = {
	{ -17, -3, -3, 3 },
	{ -17, -3, -3, 3 },
	{ -17, -3, -3, 3 },
	{ -17, -3, -3, 3 },
	{ -17, -3, -3, 3 },
	{ -17, -3, -3, 3 }
}; // ds:0x6198
static const signed int g_gfxtab_projectile_oy[6][FIG_VIEWDIR__END] = {
	{ -30, -20, -20, -20 },
	{ -30, -20, -20, -20 },
	{ -30, -20, -20, -20 },
	{ -30, -20, -20, -20 },
	{ -30, -20, -20, -20 },
	{ -30, -20, -20, -20 }
}; // ds:0x61c8
static const signed int g_gfxtab_projectile_nvfno[6] = { 0, 4, 8, 12, 16, 20 }; // ds:0x61f8
static const signed int g_gfxtab_projectile_height[6] = { 15, 15, 15, 5, 5, 8 }; // ds:0x6204
static const signed int g_gfxtab_projectile_width[6] = { 25, 25, 25, 9, 9, 16 }; // ds:0x6210
static const int8_t g_anitab_projectile_data[24][7] = {

	{-2,  1,  0,  0, 0, 0, -1},
	{-2,  0, -1,  1, 0, 0, -1},
	{-2, -1,  0,  2, 0, 0, -1},
	{-2,  0,  1,  3, 0, 0, -1},

	{-2,  1,  0,  4, 0, 0, -1},
	{-2,  0, -1,  5, 0, 0, -1},
	{-2, -1,  0,  6, 0, 0, -1},
	{-2,  0,  1,  7, 0, 0, -1},

	{-2,  1,  0,  8, 0, 0, -1},
	{-2,  0, -1,  8, 0, 0, -1},
	{-2, -1,  0,  8, 0, 0, -1},
	{-2,  0,  1,  8, 0, 0, -1},

	{-2,  1,  0, 12, 0, 0, -1},
	{-2,  0, -1, 13, 0, 0, -1},
	{-2, -1,  0, 14, 0, 0, -1},
	{-2,  0,  1, 15, 0, 0, -1},

	{-2,  1,  0, 16, 0, 0, -1},
	{-2,  0, -1, 17, 0, 0, -1},
	{-2, -1,  0, 18, 0, 0, -1},
	{-2,  0,  1, 19, 0, 0, -1},

	{-2,  1,  0, 20, 0, 0, -1},
	{-2,  0, -1, 21, 0, 0, -1},
	{-2, -1,  0, 22, 0, 0, -1},
	{-2,  0,  1, 23, 0, 0, -1}
}; // ds:0x621c, arrays, each terminated by -1
static const int8_t *g_anitab_projectile_index2[6][FIG_VIEWDIR__END] = {
	{g_anitab_projectile_data[0],  g_anitab_projectile_data[1],  g_anitab_projectile_data[2],  g_anitab_projectile_data[3]},
	{g_anitab_projectile_data[4],  g_anitab_projectile_data[5],  g_anitab_projectile_data[6],  g_anitab_projectile_data[7]},
	{g_anitab_projectile_data[8],  g_anitab_projectile_data[9],  g_anitab_projectile_data[10], g_anitab_projectile_data[11]},
	{g_anitab_projectile_data[12], g_anitab_projectile_data[13], g_anitab_projectile_data[14], g_anitab_projectile_data[15]},
	{g_anitab_projectile_data[16], g_anitab_projectile_data[17], g_anitab_projectile_data[18], g_anitab_projectile_data[19]},
	{g_anitab_projectile_data[20], g_anitab_projectile_data[21], g_anitab_projectile_data[22], g_anitab_projectile_data[23]}
}; // ds:0x62c4, 6 long arrays of length 4 (viewdir); uint8_t*
static const int8_t **g_anitab_projectile_index[6] = {
	g_anitab_projectile_index2[0],
	g_anitab_projectile_index2[1],
	g_anitab_projectile_index2[2],
	g_anitab_projectile_index2[3],
	g_anitab_projectile_index2[4],
	g_anitab_projectile_index2[5]
}; // ds:0x6324; uint8_t*

static const int8_t g_anitab_spell_nvfno[2] = { 9, 27 }; // ds:0x633c
static const signed int g_anitab_spell_ox[2] = { 0, 0 }; // ds:0x633e
static const signed int g_anitab_spell_oy[2] = { 0, 0 }; // ds:0x6342
static const int8_t g_anitab_spell_data[2][16] = {
	{	 9, 0, 0,
		10, 0, 0,
		11, 0, 0,
		10, 0, 0, 
		9, 0, 0, -1},
	{	27, 0, 0,
		28, 0, 0,
		29, 0, 0,
		28, 0, 0,
		27, 0, 0, -1},
}; // ds:0x6346, 2 arrays, each terminated by -1
static const int8_t* g_anitab_spell_index[2] = {
	(const int8_t*)&g_anitab_spell_data[0],
	(const int8_t*)&g_anitab_spell_data[1],
}; // ds:0x6366; uint8_t*


/**
 * \brief load sprites of projectiles (from spell, missile weapon or throwing weapon)
 * \param[in] attacker_object_id         object_id of the attacker
 * \param[in] projectile_gfx_id
 * \param[in] viewdir		direction
 **/
void FANI_add_projectile(const signed int attacker_object_id, const signed int projectile_gfx_id, const signed int viewdir)
{
	signed int attacker_x;
	signed int attacker_y;
	struct nvf_extract_desc nvf;

	FIG_search_obj_on_cb(attacker_object_id, &attacker_x, &attacker_y);

	g_fig_list_elem.figure = 0;

	g_fig_list_elem.nvf_no = g_gfxtab_projectile_nvfno[projectile_gfx_id];

	if (projectile_gfx_id != PROJECTILE_GFX_ID_SPELLCAST_STAR) {
		g_fig_list_elem.nvf_no = g_fig_list_elem.nvf_no + viewdir;
	}

	if (viewdir == FIG_VIEWDIR_RIGHT) {
		attacker_x += 2;
	}

	g_fig_list_elem.cbx = attacker_x;
	g_fig_list_elem.cby = attacker_y;
	g_fig_list_elem.offsetx = g_gfxtab_projectile_ox[projectile_gfx_id][viewdir];
	g_fig_list_elem.offsety = g_gfxtab_projectile_oy[projectile_gfx_id][viewdir];
	g_fig_list_elem.height = g_gfxtab_projectile_height[projectile_gfx_id];
	g_fig_list_elem.width = g_gfxtab_projectile_width[projectile_gfx_id];

	g_fig_list_elem.x1 = 0;
	g_fig_list_elem.y1 = 0;
	g_fig_list_elem.x2 = g_gfxtab_projectile_width[projectile_gfx_id] - 1;
	g_fig_list_elem.y2 = g_gfxtab_projectile_height[projectile_gfx_id] - 1;

	g_fig_list_elem.is_enemy = 0;
	g_fig_list_elem.reload = 0;
	g_fig_list_elem.ani_track_id_weapon = FANI_TRACK_ID_NONE;
	g_fig_list_elem.ani_track_id_base = FANI_TRACK_ID_NONE;
	g_fig_list_elem.gfxbuf = g_fig_projectile_buf;
	g_fig_list_elem.z = 100;
	g_fig_list_elem.visible = 0;
	g_fig_list_elem.double_size = -1;

	nvf.dst = g_fig_projectile_buf;
	nvf.src = g_spellobj_nvf_buf;
	nvf.image_num = g_fig_list_elem.nvf_no;
	nvf.compression_type = 0;
	nvf.width = &attacker_x;
	nvf.height = &attacker_x;

	process_nvf_extraction(&nvf);

	g_fig_projectile_id = FIG_add_to_list(-1);
}

void FANI_remove_projectile(void)
{
	FIG_remove_from_list(g_fig_projectile_id, 0);
	g_fig_projectile_id = -1;
}

/**
 * \brief   copies chunks of 3 bytes
 *
 * \param   dst         pointer to destination
 * \param   src         pointer to sources
 * \param   term        termination sign
 */
signed int FANI_copy_sequence(int8_t *dst, const int8_t *src, const signed char term)
{
	signed int i;

	for (i = 0; *src != term; i += 3) {

		*dst = *src;
		src++;
		dst++;

		*dst = *src;
		src++;
		dst++;

		*dst = *src;
		src++;
		dst++;
	}

	return i;
}

/**
 * \brief   prepares the flight of a projectile from the attacker to the target
 *
 * \param   ani_track_id        animation track for the projectile
 * \param   projectile_gfx_id   kind of projectile (PROJECTILE_GFX_ID_*)
 * \param   attacker_object_id  object id of the attacker
 * \param   target_object_id    object id of the target
 * \param   viewdir             direction of the flight
 * \return                      0 = target adjacent, no projectile; 1 = projectile prepared
 */
signed int FANI_prepare_projectile_ani(const signed int ani_track_id, const signed int projectile_gfx_id, const signed int attacker_object_id, const signed int target_object_id, const signed int viewdir)
{
	signed int i;
	int8_t *p_ani_clip_projectile;
	signed int attacker_x;
	signed int attacker_y;
	signed int target_x;
	signed int target_y;
	signed int distance;

	FIG_search_obj_on_cb(target_object_id, &target_x, &target_y);
	FIG_search_obj_on_cb(attacker_object_id, &attacker_x, &attacker_y);

	distance = manhattan_distance(attacker_x, attacker_y, target_x, target_y);

	if (distance <= 1) {
		return 0;
	}

	p_ani_clip_projectile = &g_fig_ani_tracks[ani_track_id][1];
	g_fig_ani_tracks[ani_track_id][0] = 0;
	g_fig_ani_tracks[ani_track_id][242] = 0;

	for (i = 0; distance - 1 > i; i++) {
		p_ani_clip_projectile += FANI_copy_sequence(p_ani_clip_projectile, g_anitab_projectile_index[projectile_gfx_id][viewdir], -1);
	}
	*p_ani_clip_projectile = -1;

	FANI_add_projectile(attacker_object_id, projectile_gfx_id, viewdir);

	return 1;
}

struct int8_pair {
	signed char a[2];
};

struct int16_pair {
	signed int a[2];
};

/**
 * \brief   creates the fighter for the impact graphic of a spell on square (x, y)
 *
 *          REMARK: only spell_impact_gfx_id 1 (DOME) and 2 (FLAME) have table entries.
 */
void FANI_add_spell(const signed int x, const signed int y, const signed int spell_impact_gfx_id)
{
	signed int height;
	signed int width;

	/* TODO: some kind of initialized structure */
	struct int8_pair a = *(struct int8_pair*)&g_anitab_spell_nvfno;	/* struct copy */
	struct int16_pair b = *(struct int16_pair*)&g_anitab_spell_ox;		/* struct copy */
	struct int16_pair c = *(struct int16_pair*)&g_anitab_spell_oy;		/* struct copy */

	struct nvf_extract_desc nvf;

	nvf.dst = g_fig_spellgfx_buf;
	nvf.src = g_spellobj_nvf_buf;

	nvf.image_num = a.a[spell_impact_gfx_id - 1];
	nvf.compression_type = 0;
	nvf.width = &width;
	nvf.height = &height;
	process_nvf_extraction(&nvf);

	g_fig_list_elem.figure = 0;
	g_fig_list_elem.nvf_no = a.a[spell_impact_gfx_id - 1];
	g_fig_list_elem.cbx = x;
	g_fig_list_elem.cby = y;

	g_fig_list_elem.offsetx = (unsigned char)b.a[spell_impact_gfx_id - 1];
	g_fig_list_elem.offsety = (unsigned char)c.a[spell_impact_gfx_id - 1];

	g_fig_list_elem.height = height;
	g_fig_list_elem.width = width;
	g_fig_list_elem.x1 = 0;
	g_fig_list_elem.y1 = 0;
	g_fig_list_elem.x2 = width - 1;
	g_fig_list_elem.y2 = height - 1;
	g_fig_list_elem.is_enemy = 0;
	g_fig_list_elem.reload = 0;
	g_fig_list_elem.ani_track_id_weapon = FANI_TRACK_ID_NONE;
	g_fig_list_elem.ani_track_id_base = FANI_TRACK_ID_NONE;
	g_fig_list_elem.gfxbuf = g_fig_spellgfx_buf;
	g_fig_list_elem.z = 99;
	g_fig_list_elem.visible = 0;
	g_fig_list_elem.double_size = -1;
	g_fig_spellgfx_id = FIG_add_to_list(-1);
}

void FANI_remove_spell(void)
{
	FIG_remove_from_list(g_fig_spellgfx_id, 0);
	g_fig_spellgfx_id = -1;
}

/**
 * \brief   prepares the impact animation of a spell (or Hylailer Feuer) of a hero at its target
 */
void FANI_prepare_hero_spell_ani(const signed int ani_track_id, const struct struct_hero *hero, const signed int spell_impact_gfx_id)
{
	int8_t *p_ani_clip_spell_impact;
	signed int x;
	signed int y;

	/* search the target on the chessboard */
	FIG_search_obj_on_cb(hero->target_object_id, &x, &y);

	p_ani_clip_spell_impact = &g_fig_ani_tracks[ani_track_id][1];

	g_fig_ani_tracks[ani_track_id][0] = 0;
	g_fig_ani_tracks[ani_track_id][242] = -1;

	/* copy the ani sequence and terminate it */
	p_ani_clip_spell_impact += FANI_copy_sequence(p_ani_clip_spell_impact, g_anitab_spell_index[spell_impact_gfx_id - 1], -1);
	*p_ani_clip_spell_impact = -1;

	FANI_add_spell(x, y, spell_impact_gfx_id);
}

/**
 * \brief   prepares the impact animation of a spell of an enemy at its target
 */
void FANI_prepare_enemy_spell_ani(const signed int ani_track_id, const struct enemy_sheet *enemy, const signed int spell_impact_gfx_id)
{
	int8_t *p_ani_clip_spell_impact;
	signed int x;
	signed int y;

	/* search the target on the chessboard */
	FIG_search_obj_on_cb(enemy->target_object_id, &x, &y);

	p_ani_clip_spell_impact = &g_fig_ani_tracks[ani_track_id][1];

	g_fig_ani_tracks[ani_track_id][0] = 0;
	g_fig_ani_tracks[ani_track_id][242] = -1;

	/* copy the ani sequence and terminate it */
	p_ani_clip_spell_impact += FANI_copy_sequence(p_ani_clip_spell_impact, g_anitab_spell_index[spell_impact_gfx_id - 1], -1);
	*p_ani_clip_spell_impact = -1;

	FANI_add_spell(x, y, spell_impact_gfx_id);
}
