/*
 * seg088 - Dungeon "Alte Zwingfeste" (DNG14), Teil 2/2, und Spiegelraetsel in Hyggeliks Ruine
 *
 * Inhalt:    DNG14_dive() laesst die Gruppe durch den ueberfluteten Gang tauchen (jeder
 *            ausser dem Vortaucher braucht eine Schwimmen-Probe). Dazu die
 *            Kampfeinleitung, die Truhen der Zwingfeste (eine enthaelt den Plan der
 *            Ebene, der die ganze Automap aufdeckt) und die Schnappeisenfalle.
 *            DNG15_riddle() ist das Hebelraetsel an den zwei Spiegeln in Hyggeliks Ruine:
 *            zwei Gruppen muessen sich durch die Spiegel die Hand gereicht haben und je den
 *            Hebel im Gang der anderen ziehen, dann oeffnet sich die Tuer (1,9,3).
 * Aufrufer:  DNG14_dive() aus DNG14_handler() (seg087), DNG14_fight_intro() aus
 *            GUI_print_fight_intro_msg() (seg097), DNG15_riddle() aus DNG_step() (seg076),
 *            die Truhen ueber g_dng14_specialchests (seg092).
 * Daten:     g_dng14_specialchests[9].
 * Besonders: DNG15_riddle() prueft die andere Gruppe nur nach x/y, nicht nach Dungeon und
 *            Ebene.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg088.c
 */
#include <stdio.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg007.h"
#include "seg076.h"
#include "seg088.h"
#include "seg092.h"
#include "seg096.h"
#include "seg097.h"
#include "seg103.h"

struct struct_chest g_dng14_specialchests[9] = {
	{ DNG_POS(0,1,1),	0, NULL,			NULL,			DNG14_chest00_loot, 0, 0, 0 },
	{ DNG_POS(0,10,2),	0, NULL,			NULL,			DNG14_chest01_loot, 0, 0, 0 },
	{ DNG_POS(1,3,6),	4, use_lockpicks_on_chest,	chest_protected_normal,	DNG14_chest02_loot, 0, 0, 0 },
	{ DNG_POS(1,4,6),	4, use_lockpicks_on_chest,	chest_protected_normal,	DNG14_chest03_loot, 0, 0, 0 },
	{ DNG_POS(1,10,11),	0, NULL,			NULL,			DNG14_chest04_loot, 0, 0, 0 },
	{ DNG_POS(1,5,8),	6, use_lockpicks_on_chest,	chest_protected_heavy,	DNG14_chest05_loot, 0, 10300, 0 },
	{ DNG_POS(2,13,2),	8, use_lockpicks_on_chest,	NULL,			DNG14_chest06_loot, 0, 0, 0 },
	{ DNG_POS(2,13,12),	6, use_lockpicks_on_chest,	DNG14_chest07_trap,	DNG14_chest07_loot, 0, 0, 0 },
	{     -1, 0, NULL, NULL, NULL, 0, 0, 0 }
}; // ds:0x9990


/**
 * \brief   the group dives through the flooded passage on level 4 of the Zwingfeste
 *
 *          Every hero except the first diver needs a Schwimmen test; on failure he loses
 *          2W6 LE but makes it on the second try. Then the group is moved to dest_x.
 *
 * \param   diver_pos   position of the hero who already dived with the rope, -1 if none
 * \param   handicap    handicap of the Schwimmen test (2 with rope, 8 without)
 * \param   dest_x      x-coordinate on the other side
 */
void DNG14_dive(const signed int diver_pos, const signed char handicap, const signed int dest_x)
{
	signed int i;
	struct struct_hero *hero;

	hero = get_hero(0);
	for (i = 0; i <= 6; i++, hero++)
	{
		if ((i != diver_pos) && (hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !hero->flags.dead)
		{

			if (test_talent(hero, TA_SCHWIMMEN, handicap) <= 0) {

				/* swimming failed */
				sprintf(g_dtp2,	get_tx(42), hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST), GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST)); /* "%s GEHT UNTERWEGS DIE LUFT AUS. %s MUSS UMKEHREN!@BEIM ZWEITEN ANLA..." */
				GUI_output(g_dtp2);

				/* loose 2W6 LE */
				sub_hero_le(hero, dice_roll(2, 6, 0));
			} else {

				/* swimming succeeded */
				sprintf(g_dtp2, get_tx(43), hero->alias); /* "%s GELANGT HUSTEND UND SPUCKEND AUF DIE ANDERE SEITE." */
				GUI_output(g_dtp2);
			}
		}
	}

	gs_x = dest_x;
	g_dng_refresh_direction = -1;
}

/**
 * \brief   print introductions of dungeon #14 fights
 *
 * \param   fight_id    ID of the fight
 */
void DNG14_fight_intro(const signed int fight_id)
{
	if ((fight_id == 172) || (fight_id == 174)) { /* FIGHT_ID_DTHO57_1, FIGHT_ID_DTHO59 */
		GUI_output(get_tx(56)); /* "DIES SCHEINT EIN EHEMALIGER ZUFLUCHTSRAUM ZU SEIN, ZUMINDEST SCHEIN..." */
	}
}

void DNG14_chest00_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng14_chest_x1;

	loot_simple_chest(chest);

	chest->loot.content = ptr_bak;
}

void DNG14_chest01_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng14_chest_x2;

	loot_simple_chest(chest);

	chest->loot.content = ptr_bak;
}

void DNG14_chest02_loot(struct struct_chest* chest)
{
	loot_multi_chest(gs_dng14_chest_x3, get_tx(62)); /* "IN DIESER TRUHE ENTDECKT IHR EINE MENGE WAFFEN." */
}

void DNG14_chest03_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng14_chest_x4;

	loot_simple_chest(chest);

	chest->loot.content = ptr_bak;
}

void DNG14_chest04_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng14_chest_x5;

	loot_simple_chest(chest);

	chest->loot.content = ptr_bak;
}

void DNG14_chest05_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak;
	unsigned char x;
	unsigned char y;

	ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng14_chest_x6;

	loot_simple_chest(chest);

	chest->loot.content = ptr_bak;

	/* the plan of this level (once): reveals the whole automap */
	if (gs_dng14_fullmap_flag) {

		gs_dng14_fullmap_flag = 0;

		GUI_output(get_tx(64)); /* "AUSSERDEM FINDET IHR EINEN PLAN DIESER HÖHLENETAGE." */

		for (x = 0; x < 16; x++) {
			for (y = 0; y < 16; y++) {
				set_automap_tile(x, y);
			}
		}
	}
}

void DNG14_chest06_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng14_chest_x7;

	loot_simple_chest(chest);

	chest->loot.content = ptr_bak;
}

void DNG14_chest07_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng14_chest_x8;

	loot_simple_chest(chest);

	chest->loot.content = ptr_bak;
}

void DNG14_chest07_trap(void)
{
	struct struct_hero *hero = get_first_hero_available_in_group();

	print_msg_with_first_hero(get_tx(61)); /* "DUMMERWEISE IST DIESE TRUHE MIT EINEM SCHNAPPEISEN GESICHERT, DAS %..." */

	sub_hero_le(hero, dice_roll(2, 6, 0));
}

/**
 * \brief   lever riddle at the two mirrors (1,8,1) and (1,8,5) in Hyggelik's ruin
 *
 *          Works only if another group stands at the other mirror and the groups have
 *          reached their hands through the mirrors (gs_dng15_reached_hands, set in DNG_step()).
 *          Each group must pull the lever in the corridor of the other group; when both
 *          levers are pulled, the door at (1,9,3) opens.
 */
void DNG15_riddle(void)
{
	signed int i;
	signed int other_group_in_position;
	signed int pos;
	signed int tw_bak;
	uint8_t *ptr;

	tw_bak = g_textbox_width;
	g_textbox_width = 8;

	ptr = g_dng_map;

	pos = DNG_POS(gs_dungeon_level, gs_x, gs_y);

	/* check if the other group is in position.
	 * REMARK: only x/y of the other group are compared, not dungeon and level. */
	for (i = other_group_in_position = 0; i < 6; i++) {

		if (pos == DNG_POS(1,8,1) && gs_groups_x[i] == 8 &&
#if FIX_0110
			/* Fix 110: die andere Gruppe muss in diesem Dungeon auf dieser Ebene stehen */
			gs_groups_dungeon_id[i] == gs_dungeon_id && gs_groups_dng_level[i] == gs_dungeon_level &&
#endif
			gs_groups_y[i] == 5 && gs_active_group_id != i)
		{
			other_group_in_position = 1;
		}

		if (pos == DNG_POS(1,8,5) && gs_groups_x[i] == 8 &&
#if FIX_0110
			gs_groups_dungeon_id[i] == gs_dungeon_id && gs_groups_dng_level[i] == gs_dungeon_level &&
#endif
			gs_groups_y[i] == 1 && gs_active_group_id != i)
		{
			other_group_in_position = 1;
		}
	}

	if (!other_group_in_position || !gs_dng15_reached_hands) {

		/* INFO: you hear a soft cracking noise */
		GUI_output(get_tx(28)); /* "IHR HÖRT EIN LEISES KNACKEN, ABER NICHTS WEITER PASSIERT." */

	} else {

		/* ask which lever should be pulled */
		do {
			i = GUI_radio(get_tx(29), 2, get_tx(30), get_tx(31)); /* "WELCHEN HEBEL WOLLT IHR ZIEHEN?" / "DEN HEBEL IM EIGENEN GANG?" / "DEN HEBEL IM GANG HINTER DEM..." */

		} while (i == -1);

		if (i == 2) {
			/* pull on the other side */

			/* set the corresponding lever */
			if (gs_y == 1) {
				gs_dng15_lever_south = 1;
			} else if (gs_y == 5) {
				gs_dng15_lever_north = 1;
			}

			if (gs_dng15_lever_south && gs_dng15_lever_north)
			{
				/* riddle solved: remove the door from the map */
				GUI_output(get_tx(32)); /* "IHR HÖRT EIN LEISES KNACKEN, DANN ERTÖNT VON IRGENDWOHER EIN RUMPELN." */

				ptr[MAP_POS(9,3)] = DNG_TILE_OPEN_DOOR << 4;

				add_hero_ap_all(10);
			} else {
				/* INFO: you hear a soft cracking noise */
				GUI_output(get_tx(28)); /* "IHR HÖRT EIN LEISES KNACKEN, ABER NICHTS WEITER PASSIERT." */
			}
		} else {
			/* pull on this side */
			/* INFO: you hear a soft cracking noise */
			GUI_output(get_tx(28)); /* "IHR HÖRT EIN LEISES KNACKEN, ABER NICHTS WEITER PASSIERT." */
		}
	}

	g_textbox_width = tw_bak;
}
