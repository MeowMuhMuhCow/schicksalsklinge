/*
 * seg089 - Dungeon "Hyggeliks Ruine" (DNG15), Burgruine mit dem Schwert Grimring
 *
 * Inhalt:    DNG15_handler() steuert die Burgruine zwischen Phexcaer und Vilnheim bzw.
 *            Skelellen: Truemmerfelder mit kleinen Wunden und einer versteckten
 *            Leiter-Truhe, einsturzgefaehrdete Decken (DNG15_collapsing_ceiling,
 *            DNG15_clear_way), die zwei Spiegel mit Durchgang, verfluchtes Gold (sperrt den
 *            Ausgang und verhindert das Gespraech mit Hyggelik), Untotenkaempfe und am Ende
 *            die Zombies und Hyggeliks Geist, der das Schwert Grimring uebergibt. Dazu 32
 *            meist leere oder harmlose Truhen.
 * Aufrufer:  DNG15_handler() ueber g_dng_handlers (seg076), die Truhen ueber
 *            g_dng15_specialchests und loot_special_chest() (seg092).
 * Daten:     g_dng15_specialchests[33]; Zustaende der 18 Deckenstellen in
 *            gs_dng15_ceilings (1 morsch, 2 bricht, 3 verschuettet, 4 freigeraeumt).
 * Besonders: DNG15_collapsing_ceiling(): kein "case 3", ein verschuetteter Gang hat keine
 *            Wirkung und Text 45 ("...KOENNT VERSUCHEN, DEN GANG WIEDER FREIZURAEUMEN")
 *            wird nie benutzt. DNG15_clear_way(): mit allen Werkzeugen dauert es 1 Stunde,
 *            ohne 30 Minuten (umgekehrt zum Kommentar). Feld (0,11,1) steht in zwei
 *            Zweigen, der zweite ist dafuer toter Code.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg089.c
 */

#include <stdio.h>
#include <string.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg003.h"
#include "seg004.h"
#include "seg007.h"
#include "seg025.h"
#include "seg026.h"
#include "seg027.h"
#include "seg029.h"
#include "seg030.h"
#include "seg032.h"
#include "seg065.h"
#include "seg075.h"
#include "seg076.h"
#include "seg089.h"
#include "seg092.h"
#include "seg096.h"
#include "seg097.h"
#include "seg103.h"
#include "seg105.h"
#include "seg113.h"

struct struct_chest g_dng15_specialchests[33] = {
	{ DNG_POS(0,4,11),	0, NULL,			NULL,			DNG15_empty_chest, 0, 0, 0 },
	{ DNG_POS(0,8,14),	0, NULL,			NULL,			DNG15_rotten_clothes_chest, 0, 0, 0 },
	{ DNG_POS(0,14,13),	0, NULL,			NULL,			DNG15_rotten_clothes_chest, 0, 0, 0 },
	{ DNG_POS(1,2,3),	0, NULL,			NULL,			DNG15_rotten_clothes_chest, 0, 0, 0 },
	{ DNG_POS(1,3,3),	3, use_lockpicks_on_chest,	NULL,			DNG15_empty_chest, 0, 0, 0 },
	{ DNG_POS(1,14,8),	0, NULL,			NULL,			DNG15_empty_chest, 0, 0, 0 },
	{ DNG_POS(1,14,9),	0, NULL,			NULL,			DNG15_empty_chest, 0, 0, 0 },
	{ DNG_POS(1,11,8),	0, NULL,			NULL,			DNG15_empty_chest, 0, 0, 0 },
	{ DNG_POS(1,11,9),	0, NULL,			NULL,			DNG15_empty_chest, 0, 0, 0 },
	{ DNG_POS(1,14,14),	8, use_lockpicks_on_chest,	chest_poisoned1,	DNG15_figures_chest, 0, 0, 0 },
	{ DNG_POS(1,5,10),	3, use_lockpicks_on_chest,	NULL,			DNG15_empty_chest, 0, 0, 0 },
	{ DNG_POS(1,6,14),	0, NULL,			NULL,			DNG15_empty_chest, 0, 0, 0 },
	{ DNG_POS(2,6,3),	3, use_lockpicks_on_chest,	NULL,			DNG15_empty_chest, 0, 0, 0 },
	{ DNG_POS(2,6,7),	0, NULL,			NULL,			DNG15_empty_chest, 0, 0, 0 },
	{ DNG_POS(2,14,12),	0, NULL,			NULL,			DNG15_cursed_money_chest, 0, 0, 0 },
	{ DNG_POS(2,14,9),	0, NULL,			NULL,			DNG15_cursed_money_chest, 0, 0, 0 },
	{ DNG_POS(2,13,7),	0, NULL,			NULL,			DNG15_empty_chest, 0, 0, 0 },
	{ DNG_POS(2,13,6),	0, NULL,			NULL,			DNG15_empty_chest, 0, 0, 0 },
	{ DNG_POS(2,13,5),	0, NULL,			NULL,			DNG15_empty_chest, 0, 0, 0 },
	{ DNG_POS(3,7,14),	3, use_lockpicks_on_chest,	NULL,			DNG15_empty_chest, 0, 0, 0 },
	{ DNG_POS(3,4,10),	0, NULL,			NULL,			DNG15_empty_chest, 0, 0, 0 },
	{ DNG_POS(3,5,10),	0, NULL,			NULL,			DNG15_empty_chest, 0, 0, 0 },
	{ DNG_POS(3,6,10),	0, NULL,			NULL,			DNG15_rotten_clothes_chest, 0, 0, 0 },
	{ DNG_POS(3,7,10),	3, use_lockpicks_on_chest,	NULL,			DNG15_empty_chest, 0, 0, 0 },
	{ DNG_POS(3,7,7),	0, NULL,			NULL,			DNG15_smelling_chest, 0, 0, 0 },
	{ DNG_POS(3,5,3),	0, NULL,			NULL,			DNG15_smelling_chest, 0, 0, 0 },
	{ DNG_POS(3,5,4),	0, NULL,			NULL,			DNG15_smelling_chest, 0, 0, 0 },
	{ DNG_POS(3,11,1),	0, NULL,			NULL,			DNG15_empty_chest, 0, 0, 0 },
	{ DNG_POS(3,9,6),	0, NULL,			NULL,			DNG15_empty_chest, 0, 0, 0 },
	{ DNG_POS(3,9,7),	0, NULL,			NULL,			DNG15_empty_chest, 0, 0, 0 },
	{ DNG_POS(3,14,6),	0, NULL,			NULL,			DNG15_empty_chest, 0, 0, 0 },
	{ DNG_POS(3,14,7),	0, NULL,			NULL,			DNG15_empty_chest, 0, 0, 0 },
	{     -1, 0, NULL, NULL, NULL, 0, 0, 0 }
}; // ds:0x9a4e

/**
 * \brief   dungeon handler of Hyggelik's ruin
 */
signed int DNG15_handler(void)
{
	signed int target_pos;
	signed int i;
	signed int viewdir;
	signed int tmp; // multi use: random value, viewdir, group index
	signed int tw_bak;
	struct struct_hero *hero;

	tw_bak = g_textbox_width;
	g_textbox_width = 7;

	target_pos = DNG_POS(gs_dungeon_level, gs_x, gs_y);

	viewdir = gs_viewdir;

	hero = get_first_hero_available_in_group(); /* UNNEEDED */

	if ((((target_pos == DNG_POS(0,6,3) || target_pos == DNG_POS(0,6,6) || target_pos == DNG_POS(0,6,9)) && viewdir == EAST) ||
		((target_pos == DNG_POS(0,12,10) || target_pos == DNG_POS(0,8,10)) && viewdir == NORTH)) && target_pos != gs_dng_pos_bak)
	{
		/* INFO: a large hall */
		GUI_output(get_tx(1)); /* "IHR BETRETET EINEN GROSSEN SAAL." */

	} else if (((target_pos == DNG_POS(0,3,4) && viewdir == NORTH) || (target_pos == DNG_POS(0,4,3) && viewdir == WEST)) && target_pos != gs_dng_pos_bak)
	{
		/* INFO: entering the tower */
		GUI_output(get_tx(2)); /* "IHR BETRETET DEN TURM. DIE EHEMALIGEN TREPPEN UND ZWISCHENDECKEN SI..." */

	} else if ((target_pos == DNG_POS(0,1,1) || target_pos == DNG_POS(0,2,1) || target_pos == DNG_POS(0,3,1) || target_pos == DNG_POS(0,4,1) ||
			target_pos == DNG_POS(0,1,2) || target_pos == DNG_POS(0,2,2) || target_pos == DNG_POS(0,3,2) || target_pos == DNG_POS(0,4,2) ||
			target_pos == DNG_POS(0,1,3) || target_pos == DNG_POS(0,2,3) || target_pos == DNG_POS(0,3,3) || target_pos == DNG_POS(0,4,3) ||
			target_pos == DNG_POS(0,1,4) || target_pos == DNG_POS(0,2,4) || target_pos == DNG_POS(0,3,4) || target_pos == DNG_POS(0,4,4)) &&
			target_pos != gs_dng_pos_bak)
	{
		/* TRAP: light wounds */
		hero = get_hero(0);
		for (i = 0; i <= 6; i++, hero++)
		{
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
				!hero->flags.dead && test_attrib(hero, ATTRIB_GE, -3) <= 0)
			{
				tmp = random_schick(3);

				sprintf(g_dtp2,	(char*)(tmp == 3 ? get_tx(3) : (tmp == 2 ? get_tx(4) : get_tx(5))), hero->alias); /* "%s STOLPERT ÜBER EINEN BALKEN..." / "%s TRITT IN EINEN HOLZSPLITTER." / "%s RUTSCHT AUF DEN TRÜMMERN AUS." */
				GUI_output(g_dtp2);

				sub_hero_le(hero, 1);
			}
		}

	} else if ((target_pos == DNG_POS(0,10,1) || target_pos == DNG_POS(0,11,1) || target_pos == DNG_POS(0,11,2) ||
			target_pos == DNG_POS(0,12,2) || target_pos == DNG_POS(0,13,2) || target_pos == DNG_POS(0,14,3)) &&
			target_pos != gs_dng_pos_bak)
	{
		/* debris, with wounds and no hidden ladder */
		DNG15_wounds_and_ladders(get_tx(9), 1, 0); /* "DIESE ECKE DES RAUMES IST DICK MIT TRÜMMERN UND SCHUTT ÜBERSÄT. DIE..." */

	} else if ((target_pos == DNG_POS(0,11,0) || target_pos == DNG_POS(0,12,0) || target_pos == DNG_POS(0,13,0) ||
			target_pos == DNG_POS(0,15,0) || target_pos == DNG_POS(0,15,1) || target_pos == DNG_POS(0,14,0)) &&
			target_pos != gs_dng_pos_bak)
	{
		/* debris, no wounds and no hidden ladder */
		DNG15_wounds_and_ladders(get_tx(14), 0, 0); /* "HIER GEHT ES NICHT WEITER. DIE ÄUSSERE MAUER IST EINGESTÜRZT UND VO..." */

	} else if (target_pos == DNG_POS(0,14,2) && target_pos != gs_dng_pos_bak)
	{
		/* debris, with wounds and hidden ladder */
		DNG15_wounds_and_ladders(get_tx(15), 1, 1); /* "IHR LAUFT AUF DEN TRÜMMERN HERUM." */

	} else if ((target_pos == DNG_POS(0,11,1) || target_pos == DNG_POS(0,12,1) || target_pos == DNG_POS(0,13,1) ||
			target_pos == DNG_POS(0,14,1)) && target_pos != gs_dng_pos_bak)
	{
		/* REMARK: (0,11,1) is already caught by the first debris branch above */
		/* debris, with wounds and no hidden ladder */
		DNG15_wounds_and_ladders(get_tx(16), 1, 0); /* "IHR LAUFT AUF DEN DECKENTRÜMMERN HERUM." */

	} else if (target_pos == DNG_POS(0,9,1) && target_pos != gs_dng_pos_bak)
	{
		DNG15_collapsing_ceiling(&gs_dng15_ceilings[0]);

	} else if (target_pos == DNG_POS(0,14,4) && target_pos != gs_dng_pos_bak)
	{
		DNG15_collapsing_ceiling(&gs_dng15_ceilings[1]);

	} else if (target_pos == DNG_POS(1,14,2) && target_pos != gs_dng_pos_bak)
	{
		/* LADDER: upwards */
		if (GUI_bool(get_tx(17))) { /* "EINE LEITER FÜHRT DURCH EINEN SCHACHT NACH OBEN UND ENDET VOR EINER..." */

			DNG_dec_level();
			gs_viewdir = WEST;
			gs_x--;
		}

	} else if (target_pos == DNG_POS(1,8,1) &&
			(target_pos != gs_dng_pos_bak || gs_viewdir_bak != viewdir) &&
			(!gs_dng15_lever_south || !gs_dng15_lever_north))
	{
		tmp = viewdir;

		sprintf(g_dtp2, get_tx(23), /* "%s EUCH BEFINDET SICH EIN SPIEGEL IN DIE WAND EINGEARBEITET. AN DER..." */
			(char*)(tmp == NORTH ? get_tx(22) : /* "HINTER" */
				(tmp == SOUTH ? get_tx(21) : /* "VOR" */
				(tmp == WEST ? get_tx(20) : /* "LINKS VON" */
				get_tx(19))))); /* "RECHTS VON" */ // tmp == EAST

		for (i = tmp = 0; i < 6; i++)
		{
			if (gs_groups_x[i] == 8 && gs_groups_y[i] == 5 && gs_active_group_id != i)
			{
				tmp = 1;
			}
		}

		strcat(g_dtp2, (char*)(tmp == 0 ? get_tx(24) : get_tx(25))); /* "IHR SEID IM SPIEGEL NICHT ZU SEHEN, WOHL A..." / "IHR SEHT IM SPIEGEL EURE FREUNDE, DIE GERA..." */
		GUI_output(g_dtp2);

		gs_viewdir_bak = (signed char)viewdir;

	} else if (target_pos == DNG_POS(1,8,5) &&
			(target_pos != gs_dng_pos_bak || gs_viewdir_bak != viewdir) &&
			(!gs_dng15_lever_south || !gs_dng15_lever_north))
	{
		tmp = viewdir;

		sprintf(g_dtp2, get_tx(23), /* "%s EUCH BEFINDET SICH EIN SPIEGEL IN DIE WAND EINGEARBEITET. AN DER..." */
				(char*)(tmp == 0 ? get_tx(21) : /* "VOR" */
				(tmp == 2 ? get_tx(22) : /* "HINTER" */
				(tmp == 3 ? get_tx(19) : get_tx(20))))); /* "RECHTS VON" / "LINKS VON" */

		for (i = tmp = 0; i < 6; i++)
		{
			if (gs_groups_x[i] == 8 && gs_groups_y[i] == 1 && gs_active_group_id != i)
			{
				tmp = 1;
			}
		}

		strcat(g_dtp2, (char*)(tmp == 0 ? get_tx(24) : get_tx(25))); /* "IHR SEID IM SPIEGEL NICHT ZU SEHEN, WOHL A..." / "IHR SEHT IM SPIEGEL EURE FREUNDE, DIE GERA..." */

		GUI_output(g_dtp2);

		gs_viewdir_bak = (signed char)viewdir;

	} else if (target_pos == DNG_POS(1,8,2) && target_pos != gs_dng_pos_bak)
	{
		/* check if another group stands on the other side */
		for (i = tmp = 0; i < 6; i++)
		{
			if (gs_groups_x[i] == 8 && gs_groups_y[i] == 5 && gs_active_group_id != i)
			{
				tmp = 1;
			}
		}

		if (tmp == 0) {
			/* go through the mirror */
			GUI_output(get_tx(26)); /* "IHR TRETET DURCH DEN SPIEGEL HINDURCH. JETZT STEHT IHR VOR DEM HEBE..." */
			gs_y = 5;
			DNG_update_pos();
		} else {
			/* stay here */
			GUI_output(get_tx(27)); /* "IHR KÖNNT NICHT DURCH DEN SPIEGEL TRETEN!" */
			gs_y = gs_y_bak;
		}

	} else if (target_pos == DNG_POS(1,8,4) && target_pos != gs_dng_pos_bak)
	{

		/* check if another group stands on the other side */
		for (i = tmp = 0; i < 6; i++)
		{
			if (gs_groups_x[i] == 8 && gs_groups_y[i] == 1 && gs_active_group_id != i)
			{
				tmp = 1;
			}
		}

		if (tmp == 0)
		{
			/* go through the mirror */
			GUI_output(get_tx(26)); /* "IHR TRETET DURCH DEN SPIEGEL HINDURCH. JETZT STEHT IHR VOR DEM HEBE..." */
			gs_y = 1;
			DNG_update_pos();
		} else {
			/* stay here */
			GUI_output(get_tx(27)); /* "IHR KÖNNT NICHT DURCH DEN SPIEGEL TRETEN!" */
			gs_y = gs_y_bak;
		}

	} else if (target_pos == DNG_POS(1,1,14) && target_pos != gs_dng_pos_bak)
	{
		loot_multi_chest(gs_dng15_chest_equips, get_tx(38)); /* "IN DER ECKE STEHEN EIN PAAR ALTE, VERROSTETE GERÄTSCHAFTEN. DIE MEI..." */

	} else if (target_pos == DNG_POS(1,1,11) && target_pos != gs_dng_pos_bak)
	{
		/* INFO: wooden beams */
		GUI_output(get_tx(39)); /* "EINIGE SCHWERE HOLZBALKEN STEHEN IN DER ECKE. AUSSERDEM LIEGEN AUF..." */

	} else if (target_pos == DNG_POS(1,3,10) && target_pos != gs_dng_pos_bak)
	{
		DNG15_collapsing_ceiling(&gs_dng15_ceilings[2]);

	} else if (target_pos == DNG_POS(2,4,10) && target_pos != gs_dng_pos_bak && !gs_dng15_took_hoe)
	{
		/* ITEM: a HOE */
		if (GUI_bool(get_tx(50))) /* "EINE VERROSTET SPITZHACKE STEHT IN DER ECKE.@WOLLT IHR SIE MITNEHMEN?" */
		{
			GUI_output(get_tx(51)); /* "DIE SPITZHACKE IST ZWAR SEHR ROSTIG, ABER DURCHAUS NOCH ZU GEBRAUCHEN." */

			if (give_new_item_to_group(ITEM_ID_HACKE, 1, 1) != -1)
			{
				gs_dng15_took_hoe = 1;
			}
		}

	} else if ((target_pos == DNG_POS(2,1,9) || target_pos == DNG_POS(2,3,13)) && target_pos != gs_dng_pos_bak)
	{
		/* INFO: end of the corridor */
		GUI_output(get_tx(52)); /* "DER GANG ENDET HIER VOR GROB BEHAUENEM FELSEN." */

	} else if (target_pos == DNG_POS(2,3,12) && target_pos != gs_dng_pos_bak)
	{
		DNG15_collapsing_ceiling(&gs_dng15_ceilings[3]);

	} else if (target_pos == DNG_POS(2,3,11) && target_pos != gs_dng_pos_bak)
	{
		DNG15_collapsing_ceiling(&gs_dng15_ceilings[4]);

	} else if (target_pos == DNG_POS(2,2,9) && target_pos != gs_dng_pos_bak)
	{
		DNG15_collapsing_ceiling(&gs_dng15_ceilings[5]);

	} else if (target_pos == DNG_POS(2,7,3) && target_pos != gs_dng_pos_bak && !gs_dng15_cursed_money[0])
	{
		DNG15_cursed_money(&gs_dng15_cursed_money[0]);

	} else if (target_pos == DNG_POS(2,6,6) && target_pos != gs_dng_pos_bak && !gs_dng15_cursed_money[1])
	{
		DNG15_cursed_money(&gs_dng15_cursed_money[1]);

	} else if (target_pos == DNG_POS(2,8,11) && target_pos != gs_dng_pos_bak && !gs_dng15_cursed_money[2])
	{
		DNG15_cursed_money(&gs_dng15_cursed_money[2]);

	} else if (target_pos == DNG_POS(2,9,7) && target_pos != gs_dng_pos_bak && !gs_dng15_cursed_money[5])
	{
		DNG15_cursed_money(&gs_dng15_cursed_money[5]);

	} else if (target_pos == DNG_POS(2,5,1) && target_pos != gs_dng_pos_bak)
	{
		DNG15_collapsing_ceiling(&gs_dng15_ceilings[11]);

	} else if (target_pos == DNG_POS(2,9,3) && target_pos != gs_dng_pos_bak)
	{
		DNG15_collapsing_ceiling(&gs_dng15_ceilings[12]);

	} else if (target_pos == DNG_POS(2,9,2) && target_pos != gs_dng_pos_bak)
	{
		DNG15_collapsing_ceiling(&gs_dng15_ceilings[13]);

	} else if (target_pos == DNG_POS(2,3,2) && target_pos != gs_dng_pos_bak)
	{
		DNG15_collapsing_ceiling(&gs_dng15_ceilings[15]);

	} else if (target_pos == DNG_POS(2,3,3) && target_pos != gs_dng_pos_bak)
	{
		DNG15_collapsing_ceiling(&gs_dng15_ceilings[16]);

	} else if (target_pos == DNG_POS(2,3,4) && target_pos != gs_dng_pos_bak)
	{
		DNG15_collapsing_ceiling(&gs_dng15_ceilings[17]);

	} else if (target_pos == DNG_POS(3,1,12) && target_pos != gs_dng_pos_bak)
	{
		DNG15_collapsing_ceiling(&gs_dng15_ceilings[6]);

	} else if (target_pos == DNG_POS(3,1,13) && target_pos != gs_dng_pos_bak)
	{
		DNG15_collapsing_ceiling(&gs_dng15_ceilings[7]);

	} else if (target_pos == DNG_POS(3,1,14) && target_pos != gs_dng_pos_bak)
	{
		DNG15_collapsing_ceiling(&gs_dng15_ceilings[8]);

	} else if (target_pos == DNG_POS(3,2,13) && target_pos != gs_dng_pos_bak)
	{
		DNG15_collapsing_ceiling(&gs_dng15_ceilings[9]);

	} else if (target_pos == DNG_POS(3,2,14) && target_pos != gs_dng_pos_bak)
	{
		DNG15_collapsing_ceiling(&gs_dng15_ceilings[10]);

	} else if (target_pos == 0x3e02 && target_pos != gs_dng_pos_bak && !gs_dng15_cursed_money[3]) /* 0x3e02 == DNG_POS(3,14,2) */
	{
		DNG15_cursed_money(&gs_dng15_cursed_money[3]);

	} else if (target_pos == DNG_POS(3,11,5) && target_pos != gs_dng_pos_bak && !gs_dng15_cursed_money[4])
	{
		DNG15_cursed_money(&gs_dng15_cursed_money[4]);

	} else if (target_pos == DNG_POS(3,11,2) && target_pos != gs_dng_pos_bak)
	{
		/* FIGHT: */
		if (random_schick(100) < 10) /* 9% chance */
		{
			g_fig_escape_position[NORTH] = g_fig_escape_position[EAST] = g_fig_escape_position[WEST] = DNG_POS_DIR(3,9,2,WEST);
			g_fig_escape_position[SOUTH] = DNG_POS_DIR(3,13,5,SOUTH);

			if (!do_fight(FIGHT_ID_DFIN26))
			{
				gs_dng15_undead_fight = 1;
			}
		}

	} else if (target_pos == DNG_POS(3,13,3) && target_pos != gs_dng_pos_bak)
	{
		/* FIGHT: */
		if (random_schick(100) < 10)
		{
			g_fig_escape_position[NORTH] = g_fig_escape_position[EAST] = g_fig_escape_position[SOUTH] = g_fig_escape_position[WEST] = DNG_POS_DIR(3,13,5,SOUTH);

			if (!do_fight(FIGHT_ID_DFIN26))
			{
				gs_dng15_undead_fight = 1;
			}
		}

	} else if ((target_pos == DNG_POS(2,8,9) || target_pos == DNG_POS(2,9,9)) && target_pos != gs_dng_pos_bak)
	{
		/* FIGHT: */
		g_fig_escape_position[NORTH] = DNG_POS_DIR(2,9,7,NORTH);
		g_fig_escape_position[EAST] = g_fig_escape_position[SOUTH] = DNG_POS_DIR(2,12,11,EAST);
		g_fig_escape_position[WEST] = DNG_POS_DIR(2,6,9,WEST);

		do_fight(!gs_dng15_undead_fight ? 134 : 133); /* FIGHT_ID_DFIN18B : FIGHT_ID_DFIN18A */

	} else if ((target_pos == DNG_POS(3,13,5) || target_pos == DNG_POS(3,9,5)) && target_pos != gs_dng_pos_bak)
	{
		/* FIGHT: */
		if (gs_dng15_undead_fight != 2)
		{
			g_fig_escape_position[NORTH] = DNG_POS_DIR(3,13,3,NORTH);
			g_fig_escape_position[EAST] = DNG_POS_DIR(3,14,5,EAST);
			g_fig_escape_position[SOUTH] = DNG_POS_DIR(3,10,10,SOUTH);
			g_fig_escape_position[WEST] = DNG_POS_DIR(3,9,3,NORTH);

			if (!do_fight(!gs_dng15_undead_fight ? 137 : 136)) /* FIGHT_ID_DFIN27B : FIGHT_ID_DFIN27A */
			{
				gs_dng15_undead_fight = 2;
			}
		}

	} else if ((target_pos == DNG_POS(3,10,10) || target_pos == DNG_POS(3,13,10) || target_pos == DNG_POS(3,12,14)) &&
			target_pos != gs_dng_pos_bak)
	{
		/* QUEST: hyggelik */
		if (gs_dng15_took_cursed_money)
		{
			/* you are cursed */
			do_talk(19, 1);

		} else if (!gs_got_grimring) {

			/* fight the zombies */
			g_fig_escape_position[NORTH] = g_fig_escape_position[EAST] = g_fig_escape_position[SOUTH] = g_fig_escape_position[WEST] = DNG_POS_DIR(3,10,10,NORTH);

			if (!do_fight(FIGHT_ID_DFIN28))
			{
				/* talk with hyggelik */
				draw_main_screen();
				DNG_update_pos();
				do_talk(19, 0);
				call_mouse_bg();
				show_hyggelik_ani();

				if (1) { } else { }

				/* group gets GRIMRING (with full knapsacks give_new_item_to_group() asks to drop
				 * something; the loop repeats until the sword is taken) */
				do { ; } while (!give_new_item_to_group(ITEM_ID_DAS_SCHWERT_GRIMRING, 1, 1));

				gs_got_grimring = 1;

				add_hero_ap_all(50);

				g_dng_loaded_dungeon_id = g_area_prepared = -1;
				g_pp20_index = -2;

				draw_main_screen();
				call_mouse();
			}
		}

	} else if (target_pos == DNG_POS(0,0,11) && target_pos != gs_dng_pos_bak) {

		/* EXIT: blocked if cursed money has been taken (gs_dng15_took_cursed_money is never reset) */
		if (gs_dng15_took_cursed_money)
		{
			GUI_output(get_tx(6)); /* "IHR VERSPÜRT DEN STARKEN DRANG, NOCH EINMAL UMZUKEHREN. IHR HABT EI..." */

			gs_x = gs_x_bak;
			gs_y = gs_y_bak;
		} else {
			leave_dungeon();

			gs_town_id = gs_travel_destination_town_id;
			gs_x = gs_travel_destination_x;
			gs_y = gs_travel_destination_y;
			gs_town_loc_type = LOCTYPE_NONE;
			gs_viewdir = REVERSE_VIEWDIR(gs_travel_destination_viewdir);

			sprintf(g_dtp2, get_tx(55), get_ttx(gs_journey_destination_town_id + 0xeb)); /* "IHR HABT DAS GEFÜHL, ALS WÜRE EINE SCHWERE..." / TEXT.LTX 236..287: Ortsname */
			GUI_output(g_dtp2);

			timewarp(HOURS(2));

			g_fading_state = 3;
		}
	}

	g_textbox_width = tw_bak;
	gs_dng_pos_bak = target_pos;

	return 0;
}

/**
 * \brief   heroes may get small wounds
 *
 *          Each alive hero in the group makes a GE-3 test.
 *          If it fails he looses 1 LE.
 */
void DNG15_small_wounds(void)
{
	signed int i;
	signed int randval;
	struct struct_hero *hero = get_hero(0);

	for (i = 0; i <= 6; i++, hero++)
	{
		if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
			!hero->flags.dead && test_attrib(hero, ATTRIB_GE, -3) <= 0)
		{
			randval = random_schick(3);

			sprintf(g_dtp2, (char*)(randval == 3 ? get_tx(3) : (randval == 2 ? get_tx(7) : get_tx(8))), hero->alias); /* "%s STOLPERT ÜBER EINEN BALKEN..." / "%s HAT SICH DEN FUSS VERTRETEN." / "DAS GERÖLL GIBT NACH UND %s R..." */
			GUI_output(g_dtp2);

			sub_hero_le(hero, 1);
		}
	}
}

/**
 * \brief   group finds debris
 *
 *          The group may find a chest with a ladder to descend to the next dungeon level.
 *
 * \param   ladder      (0 = no ladder, 1 = ladder)
 */
void DNG15_debris(const signed int ladder)
{
	if (GUI_bool(get_tx(10))) /* "WOLLT IHR DEN SCHUTT GENAUER UNTERSUCHEN?" */
	{
		if (ladder == 0)
		{
			GUI_output(get_tx(11)); /* "IHR FINDET NICHTS WEITER ALS STEINBROCKEN UND HOLZSPLITTER." */
		} else {
			GUI_output(get_tx(12)); /* "IHR LEGT EINE TRUHE FREI!" */

			if (GUI_bool(get_tx(13))) /* "VORSICHTIG ÖFFNET IHR DIE UNVERSCHLOSSEN TRUHE UND STELLT FEST, DAS..." */
			{
				gs_viewdir = WEST;
				gs_x--;
				DNG_inc_level();
			}
		}
	}
}

void DNG15_empty_chest(struct struct_chest* chest)
{
	GUI_output(get_ttx(522)); /* "DIE TRUHE IST LEER!" */
}

void DNG15_rotten_clothes_chest(struct struct_chest* chest)
{
	GUI_output(get_tx(34)); /* "IN DIESER TRUHE LIEGEN EIN PAAR VERROTTETE KLEIDUNGSSTÜCKE, DIE NIE..." */
}

void DNG15_smelling_chest(struct struct_chest* chest)
{
	struct struct_hero *hero;

	if (GUI_bool(get_tx(35))) { /* "ES STINKT HÖLLISCH, ALS IHR DIE TRUHE ÖFFNET. SCHNELL LASST IHR DEN..." */

		hero = get_hero(get_random_hero());

		sprintf(g_dtp2,	get_tx(36), hero->alias); /* "%s WIRD ES ÜBEL. DER INHALT DIESER TRUHE MUSS VOR EIN PAAR HUNDERT..." */
		GUI_output(g_dtp2);

		sub_hero_le(hero, 4);
	}
}

void DNG15_figures_chest(struct struct_chest* chest)
{
	signed int tw_bak;

	tw_bak = g_textbox_width;
	g_textbox_width = 7;
	GUI_output(get_tx(37)); /* "ALS IHR DIE TRUHE ÖFFNET, ERBLICKT IHR 12 STATUEN, DIE IM KREIS ANG..." */
	g_textbox_width = tw_bak;
}

/**
 * \brief   chest with cursed gold: 18928 Heller and 50 gold jewelry, but the group is cursed
 */
void DNG15_cursed_money_chest(struct struct_chest* chest)
{
	int32_t p_money;
	signed int tw_bak;

	tw_bak = g_textbox_width;
	g_textbox_width = 7;

	if (GUI_bool(get_tx(54))) /* "IN DER TRUHE LIEGEN UNMENGEN VON URALTEN GOLDMÜNZEN, POKALE UND STA..." */
	{
		/* You are cursed ... */
		gs_dng15_took_cursed_money = 1;

		/* ... but get much money and ... */
		p_money = get_party_money();
		p_money += 18928L;
		set_party_money(p_money);

		/* ... 50 GOLD JEWELRY. */
		give_new_item_to_group(ITEM_ID_GOLDSCHMUCK, 1, 50);
	}
	g_textbox_width = tw_bak;
}

/**
 * \brief   a place with a ceiling in danger of collapsing
 *
 * \param   ptr         state of this place in gs_dng15_ceilings:
 *                      1 = rotting beams, 2 = about to collapse, 3 = blocked, 4 = cleared
 */
void DNG15_collapsing_ceiling(uint8_t* ptr)
{
	signed int i;
	signed int cnt;
	struct struct_hero *hero;

	hero = get_hero(0);

	switch (*ptr)
	{
		case 1:
		{
			/* rotting beams */
			GUI_output(get_tx(40)); /* "VON WÄNDEN UND DECKEN IST SCHUTT IN DEN GANG GEFALLEN. DIE DECKENBA..." */

			/* count failed GE-3 test */
			for (i = cnt = 0; i <= 6; i++, hero++)
			{
				if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
					!hero->flags.dead && test_attrib(hero, ATTRIB_GE, -3) <= 0)
				{
					cnt++;
				}
			}

			if (cnt >= 2)
			{
				/* if more than one hero failed, the ceiling cracks */
				(*ptr)++;

				GUI_output(get_tx(41)); /* "ALS IHR DIE STELLE PASSIERT, LÖSEN SICH STEINBROCKEN AUS DEN WÄNDEN..." */
			}
			break;
		}
		case 2:
		{
			/* the beams crash imediately */
			GUI_output(get_tx(42)); /* "DER GANG IST BIS AUF KNIEHÖHE MIT GERÖLL BEDECKT, DAS SICH AUS DEN..." */
			(*ptr)++;
			GUI_output(get_tx(43)); /* "KRACHEND STÜRZEN DIE BALKEN HERUNTER, GEFOLGT VON EINER LAWINE AUS..." */

			/* each hero gets 1W6 damage on a failed GE test */
			for (i = cnt = 0; i <= 6; i++, hero++)
			{
				if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
					!hero->flags.dead && test_attrib(hero, ATTRIB_GE, 0) <= 0)
				{
					sprintf(g_dtp2, get_tx(44), hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST)); /* "%s IST VON EINEM DECKENBALKEN GETROFFEN WORDEN. ZUM GLÜCK HAT %s NU..." */
					GUI_output(g_dtp2);

					sub_hero_le(hero, random_schick(6));
				}
			}

			/* way is blocked */
			gs_x = gs_x_bak;
			gs_y = gs_y_bak;
			break;
		}
		/* Original-Bug: "case 3" (blocked) is missing. A blocked place (also the initial state of two
		 * places) has no effect, the group just walks through, and text 45 of FINAL.DTX ("DER GANG
		 * IST ... BLOCKIERT. IHR KOENNT VERSUCHEN, DEN GANG WIEDER FREIZURAEUMEN.") is never shown.
		 * Probably it should block the way and offer DNG15_clear_way() like case 4. */
#if FIX_0121
		case 3:
		{
			/* Fix 121: der verschuettete Gang sperrt den Weg; Freiraeumen anbieten (Text 45) */
			if (GUI_bool(get_tx(45))) /* "DER GANG IST ... BLOCKIERT. IHR KOENNT VERSUCHEN, DEN GANG WIEDER FREIZURAEUMEN." */
			{
				DNG15_clear_way(ptr);
			} else {
				gs_x = gs_x_bak;
				gs_y = gs_y_bak;
			}
			break;
		}
#endif
		case 4:
		{
			if (random_schick(100) >= 65)
			{
				/* the way is already cleared */
				GUI_output(get_tx(48)); /* "DIESEN ABSCHNITT DES GANGS HABT IHR EBEN NOCH FREIGELEGT." */
			} else {
				(*ptr)--;

				if (GUI_bool(get_tx(49))) /* "DIESEN ABSCHNITT DES GANGS HABT IHR EBEN NOCH FREIGERÄUMT, DOCH INZ..." */
				{
					DNG15_clear_way(ptr);
				}
			}
				break;
		}
	}
}

/**
 * \brief   the group tries to clear a blocked passage
 *
 * \param   ptr         state of the place (3 -> 4 on success)
 */
void DNG15_clear_way(uint8_t* ptr)
{
	signed int i;
	struct struct_hero *hero = get_hero(0);
	i = 0;

	/* With all of the following items SHOVEL, HOE, CROWBAR, FRANCESCA ...*/
	if ((get_first_hero_with_item(ITEM_ID_SCHAUFEL) != -1) &&
		(get_first_hero_with_item(ITEM_ID_HACKE) != -1) &&
		(get_first_hero_with_item(ITEM_ID_BRECHEISEN) != -1) &&
		(get_first_hero_with_item(ITEM_ID_WURFBEIL) != -1))
	{
		i = 1;
	}

	/* ... the group saves 30 minutes ingame time. */
	/* Original-Bug: the code does the opposite: with all four tools (i == 1) it takes 1 hour,
	 * without them 30 minutes. */
#if FIX_0122
	/* Fix 122: mit Werkzeug 30 Minuten, ohne eine Stunde */
	timewarp(i ? MINUTES(30) : HOURS(1));
#else
	timewarp(!i ? MINUTES(30) : HOURS(1));
#endif

	/* With a probability of 60% the ceiling crashes down. */
	if (random_schick(100) <= 60)
	{
		GUI_output(get_tx(46)); /* "ALS IHR DEN GANG BEINAHE FREIGERÄUMT HABT, STÜRZT VON OBEN WIEDER G..." */

		for (i = 0; i <= 6; i++, hero++)
		{
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
				!hero->flags.dead && test_attrib(hero, ATTRIB_GE, 0) <= 0)
			{
				sprintf(g_dtp2, get_tx(44), hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST)); /* "%s IST VON EINEM DECKENBALKEN GETROFFEN WORDEN. ZUM GLÜCK HAT %s NU..." */
				GUI_output(g_dtp2);

				sub_hero_le(hero, random_schick(6));
			}
		}

		gs_x = gs_x_bak;
		gs_y = gs_y_bak;
	} else {
		(*ptr)++;
		GUI_output(get_tx(47)); /* "ES IST ZWAR EIN HARTES STÜCK ARBEIT, ABER SCHLIESSLICH HABT IHR DEN..." */
	}
}

/**
 * \brief   a few old gold coins on the floor (10..25 D), taking them curses the group
 *
 * \param   ptr         flag of this place in gs_dng15_cursed_money[]
 */
void DNG15_cursed_money(uint8_t* ptr)
{
	int32_t p_money;

	if (GUI_bool(get_tx(53))) /* "EIN PAAR URALTE GOLDSTÜCKE LIEGEN VERSTREUT AUF DEM BODEN HERUM.@WO..." */
	{
		*ptr = gs_dng15_took_cursed_money = 1;

		p_money = get_party_money();
		p_money += random_interval(10, 25) * 100;
		set_party_money(p_money);
	}
}

/**
 * \brief   shows a text about debris, maybe small wounds, then the debris can be examined
 * \param text		pointer to the output text
 * \param wounds      group may get wounds (0/1)
 * \param ladder      here is a ladder to the next level
 */
void DNG15_wounds_and_ladders(char* text, const signed int wounds, const signed int ladder)
{
	GUI_output(text);

	if (wounds != 0)
	{
		DNG15_small_wounds();
	}

	DNG15_debris(ladder);
}
