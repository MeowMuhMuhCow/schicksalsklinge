/*
 * seg084 - Dungeon "Kultstaette des Namenlosen" (DNG09)
 *
 * Inhalt:    DNG09_handler() steuert die Kultstaette auf der Insel Hjalland: Waschraum,
 *            Kessel, Latrinen, viele Statuen des Namenlosen (DNG09_statues, seg082),
 *            Bolzenfallen (teils per Hebel umgeschaltet), Leitern, Falltuer, die Fallgrube
 *            (DNG09_pitfall, seg081), ein Hebel, der festgehalten werden muss und eine Wand
 *            versetzt, der Altar des Namenlosen (Quest erfuellt, schwarze Statuette) und
 *            zwei Geheimtueren. DNG09_chest04_loot(): der schwarze Kristall, der nur mit
 *            DESTRUCTIBO oder IGNIFAXIUS zerstoert werden kann (AE-Verlust dauerhaft).
 * Aufrufer:  DNG09_handler() ueber g_dng_handlers (seg076), DNG09_fight_intro() aus
 *            GUI_print_fight_intro_msg() (seg097), die Truhen ueber g_dng09_specialchests
 *            (seg092).
 * Daten:     g_dng09_specialchests[8].
 * Besonders: Hilferuf aus der Grube: "gs_day_timer % 90 * 20" wird als (t % 90) * 20
 *            gerechnet, die Meldung kommt zu jeder vollen Minute statt alle 20 Minuten.
 *            Altar: Goetterschleife 0..13 statt 1..14 (Ifirn fehlt). Bolzenfalle 4 wird
 *            nie verbraucht (Bright-Eyes-TODO).
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg084.c
 */
#include <stdio.h>
#include <string.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg004.h"
#include "seg007.h"
#include "seg025.h"
#include "seg027.h"
#include "seg032.h"
#include "seg047.h"
#include "seg049.h"
#include "seg075.h"
#include "seg081.h"
#include "seg082.h"
#include "seg084.h"
#include "seg092.h"
#include "seg096.h"
#include "seg097.h"
#include "seg098.h"
#include "seg103.h"
#include "seg105.h"

struct struct_chest g_dng09_specialchests[8] = {
	{ DNG_POS(0,12,8),	0, NULL,			NULL,			DNG09_chest00_loot, 0, 0, 8 },
	{ DNG_POS(0,13,4),	0, NULL,			NULL,			DNG09_chest01_loot, 0, 0, 0 },
	{ DNG_POS(0,14,5),	4, use_lockpicks_on_chest,	NULL,			DNG09_chest02_loot, 0, 0, 0 },
	{ DNG_POS(1,12,11),	0, NULL,			NULL,			DNG09_chest03_loot, 0, 160, 0 },
	{ DNG_POS(1,9,8),	8, use_lockpicks_on_chest,	DNG09_chest04_trap,	DNG09_chest04_loot, 0, 0, 0 },
	{ DNG_POS(1,1,1),	4, use_lockpicks_on_chest,	NULL,			DNG09_chest05_loot, 0, 1500, 0 },
	{ DNG_POS(1,1,5),	8, use_lockpicks_on_chest,	chest_poisoned3,	DNG09_chest06_loot, 0, 25000, 0 },
	{     -1, 0, NULL, NULL, NULL, 0, 0, 0 }
}; // ds:0x972e

/**
 * \brief   dungeon handler of the temple of the Nameless God
 */
signed int DNG09_handler(void)
{
	signed int target_pos;
	signed int i;
	signed int l3;
	signed int tw_bak;
	struct struct_hero *hero;
	uint8_t *amap_ptr;

	amap_ptr = g_dng_map;
	tw_bak = g_textbox_width;
	g_textbox_width = 7;

	target_pos = DNG_POS(gs_dungeon_level, gs_x, gs_y);

	/* TODO: not the leader ? */
	hero = get_hero(0);

	/* a hero is in the pit (DNG09_pitfall()): call for help.
	 * Original-Bug: operator precedence. This is (gs_day_timer % 90) * 20, i.e. true at every full
	 * minute (MINUTES(1) == 90), not gs_day_timer % MINUTES(20) as probably intended. */
#if FIX_0113
	/* Fix 113: alle 20 Minuten */
	if (gs_dng09_pit_flag && !(gs_day_timer % MINUTES(20)))
#else
	if (gs_dng09_pit_flag && !(gs_day_timer % 90 * 20))
#endif
	{
		GUI_output(get_tx(32)); /* "IN DER FERNE KÖNNT IHR JEMANDEN VERZWEIFELT UM HILFE RUFEN HÖREN..." */
	}

	if (target_pos == DNG_POS(0,11,14) && target_pos != gs_dng_pos_bak && gs_viewdir == EAST)
	{
		GUI_output(get_tx(1)); /* "DER VOLLKOMMEN KAHLE RAUM IST OFFENSICHTLICH FÜR RITUELLE WASCHUNGE..." */

	} else if (target_pos == DNG_POS(0,10,8) && target_pos != gs_dng_pos_bak)
	{
		GUI_output(get_tx(2)); /* "ÜBER EINER ERKALTETEN FEUERSTELLE HÄNGT EIN GROSSER KESSEL MIT DEN..." */

	} else if ((target_pos == DNG_POS(0,14,14) || target_pos == DNG_POS(0,14,8)) && target_pos != gs_dng_pos_bak)
	{
		GUI_output(get_tx(3)); /* "DAS LOCH IM BODEN UND DER TYPISCHE GERUCH VERRATEN, DASS ES SICH HI..." */

	} else if ((target_pos == DNG_POS(0,3,14) || target_pos == DNG_POS(0,5,14) ||
				target_pos == DNG_POS(0,1,14) ||target_pos == DNG_POS(0,1,1) ||
				target_pos == DNG_POS(0,1,3) ||target_pos == DNG_POS(0,6,1) ||
				target_pos == DNG_POS(0,9,1) ||target_pos == DNG_POS(0,6,4) ||
				target_pos == DNG_POS(0,9,4) ||target_pos == DNG_POS(0,12,6) ||
				target_pos == DNG_POS(1,12,7) ||target_pos == DNG_POS(1,14,4) ||
				target_pos == DNG_POS(1,14,6) ||target_pos == DNG_POS(1,13,14) ||
				target_pos == DNG_POS(1,10,14) ||target_pos == DNG_POS(1,1,14) ||
				target_pos == DNG_POS(1,4,8) ||target_pos == DNG_POS(1,11,1) ||
				target_pos == DNG_POS(1,6,1) ||target_pos == DNG_POS(1,8,1) ||
				target_pos == DNG_POS(1,6,3) ||target_pos == DNG_POS(1,8,3)) &&
			target_pos != gs_dng_pos_bak)
	{
		DNG09_statues(10, 20);

	} else if (target_pos == DNG_POS(0,4,11) && target_pos != gs_dng_pos_bak &&
			gs_dng09_bolttrap1_flag && !gs_dng09_lever1_flag)
	{
		GUI_output(get_tx(11)); /* "AUS DEM BODEN SCHIESSEN PLÖTZLICH DUTZENDE KLEINER ARMBRUSTBOLZEN E..." */

		gs_dng09_bolttrap1_flag--;

		for (i = 0; i <= 6; i++, hero++)
		{
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
				!hero->flags.dead && test_attrib(hero, ATTRIB_GE, 4) <= 0)
			{
				/* failed GE+4 test (former comment: FF+4) */
				sub_hero_le(hero, dice_roll(2, 6, 4));

				sprintf(g_dtp2, get_tx(12), hero->alias); /* "%s WIRD VON DEN BOLZEN GETROFFEN UND TRÄGT MITTELSCHWERE VERWUNDUNG..." */
				GUI_output(g_dtp2);
			}
		}

	} else if (target_pos == DNG_POS(0,1,8) && target_pos != gs_dng_pos_bak)
	{
		if (GUI_bool(get_tx(14))) /* "HIER FÜHRT EINE STEILE LEITER DURCH EINEN ENGEN SCHACHT NACH UNTEN...." */
		{
			GUI_output(get_tx(15)); /* "VORSICHTIG KLETTERT IHR, EINER NACH DEM ANDEREN, IN DIE TIEFE." */

			gs_x += 2;
			gs_viewdir = EAST;
			DNG_inc_level();
		}

	} else if (target_pos == DNG_POS(0,4,5) && target_pos != gs_dng_pos_bak &&
			gs_dng09_bolttrap2_flag && gs_dng09_lever1_flag)
	{
		gs_dng09_bolttrap2_flag--;

		GUI_output(get_tx(11)); /* "AUS DEM BODEN SCHIESSEN PLÖTZLICH DUTZENDE KLEINER ARMBRUSTBOLZEN E..." */

		for (i = 0; i <= 6; i++, hero++)
		{
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
				!hero->flags.dead && test_attrib(hero, ATTRIB_GE, 4) <= 0)
			{
				/* failed GE+4 test (former comment: FF+4) */
				sub_hero_le(hero, dice_roll(2, 6, 4));

				sprintf(g_dtp2,	get_tx(12), hero->alias); /* "%s WIRD VON DEN BOLZEN GETROFFEN UND TRÄGT MITTELSCHWERE VERWUNDUNG..." */
				GUI_output(g_dtp2);
			}
		}

	} else if (target_pos == DNG_POS(0,2,2) && target_pos != gs_dng_pos_bak &&
			gs_dng09_bolttrap3_flag)
	{
		gs_dng09_bolttrap3_flag--;

		GUI_output(get_tx(11)); /* "AUS DEM BODEN SCHIESSEN PLÖTZLICH DUTZENDE KLEINER ARMBRUSTBOLZEN E..." */

		for (i = 0; i <= 6; i++, hero++)
		{
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
				!hero->flags.dead && test_attrib(hero, ATTRIB_GE, 4) <= 0)
			{
				/* failed GE+4 test (former comment: FF+4) */
				sub_hero_le(hero, dice_roll(2, 6, 4));

				sprintf(g_dtp2,	get_tx(12), hero->alias); /* "%s WIRD VON DEN BOLZEN GETROFFEN UND TRÄGT MITTELSCHWERE VERWUNDUNG..." */
				GUI_output(g_dtp2);
			}
		}

	} else if (target_pos == DNG_POS(0,4,3) && target_pos != gs_dng_pos_bak)
	{
		if (GUI_bool(get_tx(16))) /* "HIER BEFINDET SICH EIN HEBEL IN DER WAND.@WOLLT IHR DEN HEBEL UMLEGEN?" */
		{
			GUI_output(get_tx(17)); /* "IHR KÖNNT KEIN GERÄUSCH VERNEHMEN, ALS IHR DEN HEBEL UMLEGT.@SELTSAM!" */

			/* switches between bolt trap 1 at (0,4,11) and bolt trap 2 at (0,4,5) */
			gs_dng09_lever1_flag ^= 1;
		}

	} else if (target_pos == DNG_POS(0,8,6) && target_pos != gs_dng_pos_bak)
	{
		for (i = l3 = 0; i <= 6; i++, hero++)
		{
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
				!hero->flags.dead && test_talent(hero, TA_GEFAHRENSINN, 2) > 0)
			{
				l3 = 1;
			}
		}

		if (l3 != 0)
		{
			GUI_output(get_tx(18)); /* "HOPPLA!@DA WÄRT IHR DOCH FAST AUF EINE FALLTÜRE GETRETEN." */
		} else {
			GUI_output(get_tx(19)); /* "UNTER EUREN FÜSSEN TUT SICH URPLÖTZLICH EINE FALLGRUBE AUF. MIT HOH..." */

			gs_x--;
			gs_y += 2;
			DNG_inc_level();
		}

	} else if (target_pos == DNG_POS(1,13,6) && target_pos != gs_dng_pos_bak &&
			gs_dng09_cultist_flag && !gs_dng09_lever2_flag)
	{
		GUI_output(get_tx(11)); /* "AUS DEM BODEN SCHIESSEN PLÖTZLICH DUTZENDE KLEINER ARMBRUSTBOLZEN E..." */

		for (i = 0; i <= 6; i++, hero++)
		{
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
				!hero->flags.dead && test_attrib(hero, ATTRIB_GE, 4) <= 0)
			{
				/* failed GE+4 test (former comment: FF+4) */
				sub_hero_le(hero, dice_roll(2, 6, 4));

				sprintf(g_dtp2,	get_tx(12), hero->alias); /* "%s WIRD VON DEN BOLZEN GETROFFEN UND TRÄGT MITTELSCHWERE VERWUNDUNG..." */
				GUI_output(g_dtp2);
			}
		}

	} else if (target_pos == DNG_POS(1,14,9) && target_pos != gs_dng_pos_bak)
	{
		if (GUI_bool(get_tx(16))) /* "HIER BEFINDET SICH EIN HEBEL IN DER WAND.@WOLLT IHR DEN HEBEL UMLEGEN?" */
		{
			GUI_output(get_tx(28)); /* "EIN LEISES KLICKEN VERRÄT EUCH, DASS DER HEBEL IRGENDEINE WIRKUNG HAT." */
			gs_dng09_lever2_flag ^= 1;
		}

	} else if (target_pos == DNG_POS(1,9,11) && target_pos != gs_dng_pos_bak &&
			gs_dng09_bolttrap4_flag && gs_dng09_secretdoor2 == 2)
	{
		GUI_output(get_tx(11)); /* "AUS DEM BODEN SCHIESSEN PLÖTZLICH DUTZENDE KLEINER ARMBRUSTBOLZEN E..." */

		/* TODO: add missing gs_dng09_bolttrap4_flag--; */
#if FIX_0114
		/* Fix 114: wie die Fallen 1 bis 3 herunterzaehlen */
		gs_dng09_bolttrap4_flag--;
#endif

		for (i = 0; i <= 6; i++, hero++)
		{
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
				!hero->flags.dead && test_attrib(hero, ATTRIB_GE, 4) <= 0)
			{
				/* failed GE+4 test (former comment: FF+4) */
				sub_hero_le(hero, dice_roll(2, 6, 4));

				sprintf(g_dtp2,	get_tx(12), hero->alias); /* "%s WIRD VON DEN BOLZEN GETROFFEN UND TRÄGT MITTELSCHWERE VERWUNDUNG..." */
				GUI_output(g_dtp2);
			}
		}

	} else if ((target_pos == DNG_POS(1,6,7) || target_pos == DNG_POS(1,4,1) ||
			target_pos == DNG_POS(1,4,3)) && target_pos != gs_dng_pos_bak)
       	{

		DNG09_statues(20, 50);

	} else if (target_pos == DNG_POS(1,3,12) && target_pos != gs_dng_pos_bak) {

		DNG09_pitfall();

	} else if (target_pos == DNG_POS(1,2,8) && target_pos != gs_dng_pos_bak) {

		if (GUI_bool(get_tx(38))) { /* "HIER FÜHRT EINE LEITER NACH OBEN.@WOLLT IHR SIE BENUTZEN?" */
			gs_x--;
			gs_y++;
			gs_viewdir = SOUTH;
			DNG_dec_level();
		}

	} else if (target_pos == DNG_POS(1,7,13) && target_pos != gs_dng_pos_bak &&	!gs_dng09_lever_fast) {
		/* lever, removes wall at (4,5) and puts a wall at (3,5), level 2 */
		do {
			i = GUI_radio(get_tx(39), 3, get_tx(40), get_tx(41), get_tx(42)); /* "HIER RAGT EIN HEBEL AUS DER W..." / "DEN HEBEL IN RUHE LASSEN" / "DEN HEBEL KURZ NIEDERDRÜCKEN?" / "DEN HEBEL NIEDERDRÜCKEN UND H..." */

		} while (i == -1);

		if (i == 2)
		{
			GUI_output(get_tx(43)); /* "DER HEBEL SCHNAPPT IN SEINE AUSGANGSLAGE ZURÜCK." */

		} else if (i == 3)
		{
			if ((i = select_hero_ok(get_tx(44))) != -1) /* "WER SOLL DEN HEBEL FESTHALTEN?" */
			{
				hero = get_hero(i);

				sprintf(g_dtp2, get_tx(45), hero->alias); /* "MIT MÜHE HÄLT %s DEN HEBEL IN POSITION." */
				GUI_output(g_dtp2);

				/* remove wall at (4,5), lvl 2 (MAP_POS(15,0) is just 0x0f: keep the flags) */
				*(amap_ptr + MAP_POS(4,5)) &= (DNG_TILE_CORRIDOR << 4) + MAP_POS(15,0);
				*(amap_ptr + MAP_POS(3,5)) |= 0xf0; /* tile type 15 = wall, flags kept (former comment: "clear flags") */

				if (gs_group_member_counts[gs_active_group_id] > 1)
				{
					gs_viewdir_bak = (gs_viewdir);
					l3 = 0;
					while (gs_group_member_counts[l3])
					{
						l3++;
					}

					hero->group_id = (signed char)l3;
					gs_group_member_counts[l3]++;
					gs_group_member_counts[gs_active_group_id]--;
					GRP_save_pos(l3);
					gs_dng09_lever_fast = 1;
				}
			} else {
				gs_viewdir_bak = gs_viewdir;
			}
		} else
		{
			gs_viewdir_bak = gs_viewdir;
		}

	} else if ((target_pos == DNG_POS(1,8,13) || target_pos == DNG_POS(1,7,14)) && target_pos != gs_dng_pos_bak)
	{
		/* squares next to lever: check if group at the lever moved away and the wall must be closed */
		/* TODO: potential Original-Bug: What if the group at the lever to use Transversalis to teleport away? I guess the wall is still open */

		/* check if there is still another group on the square of the lever */
		for (i = l3 = 0; i < 6; i++)
		{
			if (gs_groups_x[i] == 7 && gs_groups_y[i] == 13 &&
				gs_groups_dng_level[i] == 1 && gs_active_group_id != i)
			{
				l3 = 1;
			}
		}

		/* if not, close wall at (4,5), lvl 2 */
		if (l3 == 0)
		{
			*(amap_ptr + MAP_POS(4,5)) |= 0xf0; /* tile type 15 = wall, flags kept (former comment: "clear flags") */
			*(amap_ptr + MAP_POS(3,5)) &= (DNG_TILE_CORRIDOR << 4) + 0x0f;
			gs_dng09_lever_fast = 0;
		}

	} else if (target_pos == DNG_POS(1,5,2) && target_pos != gs_dng_pos_bak && !gs_dng09_altar_flag)
	{
		load_ani(26);
		init_ani(0);
		vsync_or_key(100);

		if (GUI_bool(get_tx(46))) { /* "IHR STEHT VOR EINEM FINSTEREN ALTAR, DER ÜBER UND ÜBER MIT KRYPTISC..." */

			gs_dng09_altar_flag = 1;

			add_hero_ap_all(60);

			/* Original-Bug: god ids are 1..14, the loop changes entry 0 and skips Ifirn (see DNG09_statues()) */
#if FIX_0070
			/* Fix 70: Goetter 1..14 (Praios bis Ifirn) */
			for (i = 1; i <= 14; i++) {
#else
			for (i = 0; i < 14; i++) {
#endif
				gs_gods_estimation[i] += 100L;
			}

			/* mark quest as done */
			gs_quest_nameless_done = 1;

			do {
				i = GUI_radio(get_tx(47), 2, get_tx(48), get_tx(49)); /* "ES DAUERT EINE WEILE, BIS IHR..." / "JA" / "NEIN, WIR VERNICHTEN SIE" */

			} while (i == -1);

			if (i == 1) {

				give_new_item_to_group(ITEM_ID_SCHWARZE_STATUETTE, 1, 1);

			} else {

				GUI_output(get_tx(50)); /* "MIT MÄCHTIGEN HIEBEN ZERSTÖRT IHR AUCH DIESES ABBILD DES NAMENLOSEN..." */
			}
		}

		disable_ani();
		g_area_prepared = AREA_TYPE_NONE;

	} else if (target_pos == DNG_POS(1,4,2) && (target_pos != gs_dng_pos_bak || gs_viewdir != gs_viewdir_bak) &&
			gs_viewdir == WEST && gs_dng09_secretdoor1 != 2)
	{
		if (gs_dng09_secretdoor1 || test_talent((hero = get_first_hero_available_in_group()), TA_SINNESSCHAERFE, 6) > 0)
		{
			gs_dng09_secretdoor1 = 1;

			sprintf(g_dtp2,	get_tx(51), hero->alias); /* "%s ENTDECKT PLÖTZLICH EINE GEHEIMTÜRE IN DER WAND." */

			sprintf(g_text_output_buf,
				(char*)(((l3 = test_talent(hero, TA_SCHLOESSER, 4)) > 0) ? get_tx(52) : get_tx(54)), /* "@NACH KURZER ZEIT GELINGT ES %s SOGAR, SIE..." / "@ES GELINGT %s ALLERDINGS NICHT, SIE ZU ÖF..." */
				GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_3RD), GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_4TH), GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_4TH));

			strcat(g_dtp2, g_text_output_buf);

			GUI_output(g_dtp2);

			if (l3 > 0)
			{
				*(amap_ptr + MAP_POS(3,2)) &= (DNG_TILE_CORRIDOR << 4) + 0x0f;

				gs_dng09_secretdoor1 = 2;

				DNG_update_pos();

			} else {
				sub_hero_le(hero, dice_roll(2, 6, 0));
			}

			gs_viewdir_bak = gs_viewdir;
		}
	} else if (target_pos == DNG_POS(1,5,11) && (target_pos != gs_dng_pos_bak || gs_viewdir != gs_viewdir_bak) &&
			gs_viewdir == EAST && gs_dng09_secretdoor2 != 2)
	{
		if (gs_dng09_secretdoor2 || test_talent((hero = get_first_hero_available_in_group()), TA_SINNESSCHAERFE, 8) > 0)
		{
			gs_dng09_secretdoor2 = 1;

			sprintf(g_dtp2,	get_tx(51), hero->alias); /* "%s ENTDECKT PLÖTZLICH EINE GEHEIMTÜRE IN DER WAND." */

			sprintf(g_text_output_buf,
				(char*)(((l3 = test_talent(hero, TA_SCHLOESSER, 6)) > 0) ? get_tx(52) : get_tx(53)), /* "@NACH KURZER ZEIT GELINGT ES %s SOGAR, SIE..." / "@LEIDER GELINGT ES %s ALLERDINGS NICHT, SI..." */
				GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_3RD));

			strcat(g_dtp2, g_text_output_buf);

			GUI_output(g_dtp2);

			if (l3 > 0)
			{
				*(amap_ptr + MAP_POS(6,11)) &= (DNG_TILE_CORRIDOR << 4) + 0x0f;

				gs_dng09_secretdoor2 = 2;

				DNG_update_pos();
			}

			gs_viewdir_bak = gs_viewdir;
		}

	} else if (target_pos == DNG_POS(0,7,15) && target_pos != gs_dng_pos_bak)
	{
		leave_dungeon();
		gs_town_id = gs_travel_destination_town_id;
		gs_x = gs_travel_destination_x;
		gs_y = gs_travel_destination_y;
		gs_town_loc_type = LOCTYPE_NONE;
		gs_viewdir = REVERSE_VIEWDIR(gs_travel_destination_viewdir);

		sprintf(g_dtp2,	get_tx(55), get_ttx(gs_journey_destination_town_id + 0xeb)); /* "IHR VERLASST DIESEN UNGEMÜTLICHEN, ORT UND..." / TEXT.LTX 236..287: Ortsname */

		GUI_output(g_dtp2);

		timewarp(HOURS(3));

		g_fading_state = 3;
	}

	g_textbox_width = tw_bak;
	gs_dng_pos_bak = target_pos;

	return 0;
}

/**
 * \brief   print introductions of dungeon #9 fights
 *
 * \param   fight_id    ID of the fight
 */
void DNG09_fight_intro(const signed int fight_id)
{
	if (fight_id == FIGHT_ID_F126_08)
	{
		GUI_output(get_tx(13)); /* "ENTLANG DER WAND DES GANGES SITZEN EIN HALBES DUTZEND IN KUTTEN GEK..." */
	}
}

void DNG09_chest00_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng09_chest0;

	loot_simple_chest(chest);

	chest->loot.content = ptr_bak;
}

void DNG09_chest01_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng09_chest1;

	loot_simple_chest(chest);

	chest->loot.content = ptr_bak;
}

void DNG09_chest02_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng09_chest2;

	loot_simple_chest(chest);

	chest->loot.content = ptr_bak;
}

void DNG09_chest03_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng09_chest3;

	loot_simple_chest(chest);

	chest->loot.content = ptr_bak;
}

/**
 * \brief   chest with the black crystal: smashing it costs 2W20 LE, DESTRUCTIBO (20 AE) or
 *          IGNIFAXIUS (35 AE) destroy it with a permanent loss of 3 AE; a failed spell costs 10/20 AE
 */
void DNG09_chest04_loot(struct struct_chest* chest)
{
	signed int answer;
	struct struct_hero *hero;

	if (!gs_dng09_crystal_flag)
	{
		if (GUI_bool(get_tx(20))) /* "IN DER TRUHE LIEGT EIN GROSSER, AUF NACHTSCHWARZEN SAMT GEBETTETER,..." */
		{
			do {
				answer = GUI_radio(get_tx(21), 3, get_tx(22), get_tx(23), get_tx(24)); /* "WIE SOLL DER KRISTALL VERNICH..." / "ZERSCHLAGEN" / "DESTRUCTIBO" / "IGNIFAXIUS" */

			} while (answer == -1);

			hero = get_first_hero_available_in_group();

			if (answer == 1)
			{
				sprintf(g_dtp2, get_tx(25), hero->alias); /* "DER KRISTALL WIDERSTEHT DEM HIEB OHNE AUCH NUR EINEN KRATZER DAVONZ..." */
				GUI_output(g_dtp2);

				sub_hero_le(hero, dice_roll(2, 20, 0));

			} else if (answer == 2)
			{
				hero = get_hero(select_hero_ok_forced(get_ttx(317))); /* "WELCHER HELD SOLL MAGIE ANWENDEN?" */
				if (test_spell(hero, SP_DESTRUCTIBO_ARCANITAS, 0) > 0)
				{
					sub_ae_splash(hero, 20);

					/* permanent AE loss of 3 */
					hero->ae_max -= 3;

					GUI_output(get_tx(27)); /* "LANGSAM BILDEN SICH HAARFEINE RISSE AUF DER OBERFLÄCHE DES KRISTALL..." */

					add_hero_ap_all(50);

					gs_dng09_crystal_flag = 1;

				} else {
					sprintf(g_dtp2, get_tx(26), hero->alias); /* "LEIDER VERSAGT DER ZAUBER UND ALLES BLEIBT BEIM ALTEN... BIS AUF DI..." */
					GUI_output(g_dtp2);

					sub_ae_splash(hero, 10);
				}

			} else if (answer == 3)
			{
				hero = get_hero(select_hero_ok_forced(get_ttx(317))); /* "WELCHER HELD SOLL MAGIE ANWENDEN?" */
				if (test_spell(hero, SP_IGNIFAXIUS_FLAMMENSTRAHL, 0) > 0)
				{
					sub_ae_splash(hero, 35);

					/* permanent AE loss of 3 */
					hero->ae_max -= 3;

					GUI_output(get_tx(27)); /* "LANGSAM BILDEN SICH HAARFEINE RISSE AUF DER OBERFLÄCHE DES KRISTALL..." */

					add_hero_ap_all(50);

					gs_dng09_crystal_flag = 1;

				} else {
					sprintf(g_dtp2,	get_tx(26), hero->alias); /* "LEIDER VERSAGT DER ZAUBER UND ALLES BLEIBT BEIM ALTEN... BIS AUF DI..." */
					GUI_output(g_dtp2);

					sub_ae_splash(hero, 20);
				}
			}
		}
	} else {
		GUI_output(get_ttx(522)); /* "DIE TRUHE IST LEER!" */
	}
}

void DNG09_chest05_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng09_chest5;

	loot_simple_chest(chest);

	chest->loot.content = ptr_bak;
}

void DNG09_chest06_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng09_chest6;

	loot_simple_chest(chest);

	chest->loot.content = ptr_bak;
}

/**
 * \brief   trap of the crystal chest: a fight starts (FIGHT_ID_F126_22)
 */
void DNG09_chest04_trap(void)
{
	g_fig_escape_position[NORTH] = g_fig_escape_position[EAST] = g_fig_escape_position[SOUTH] = g_fig_escape_position[WEST] = DNG_POS_DIR(1,10,11,SOUTH);

	do_fight(FIGHT_ID_F126_22);
}
