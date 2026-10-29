/*
 * seg078 - Dungeon "Verfallene Herberge" (DNG02)
 *
 * Inhalt:    DNG02_handler() steuert die Ereignisse der verfallenen Herberge zwischen
 *            Ottarje und Daspota: Zombies bei Nacht, Kuehlkeller, Klebefalle, Falltueren,
 *            Gewichtsplatten, die Waende auf (10,11) und (11,8) erscheinen bzw. oeffnen
 *            lassen, Sphaerenriss und Phantom, Rabenstatue mit Geheimtuer,
 *            Versteinerungsfalle, Aufzug, drei Geheimtueren und den Ausgang. Dazu die
 *            Truhen (u. a. die violette Kristall-Apparatur) und die Kampfeinleitungen.
 * Aufrufer:  DNG02_handler() ueber g_dng_handlers (seg076), die Truhenfunktionen ueber
 *            g_dng02_specialchests und loot_special_chest() (seg092),
 *            DNG02_fight_intro() aus GUI_print_fight_intro_msg() (seg097) beim Kampfbeginn.
 * Daten:     g_dng02_specialchests[8].
 * Besonders: Gewichtsplatte (11,6): weight_sum wird in der Gruppenschleife jedes Mal auf 0
 *            gesetzt, die "Gruppe steht auf (11,8)"-Sperre wirkt nur fuer Gruppe 5.
 *            Viele bekannte Fehler von Bright-Eyes kommentiert (Klebefalle Bug 65,
 *            Geheimtuer-Held Bug 66, Teleport wird nicht erkannt).
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg078.c
 */

#include <stdio.h>
#include <string.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg003.h"
#include "seg007.h"
#include "seg025.h"
#include "seg026.h"
#include "seg032.h"
#include "seg075.h"
#include "seg076.h"
#include "seg078.h"
#include "seg092.h"
#include "seg096.h"
#include "seg097.h"
#include "seg098.h"
#include "seg103.h"
#include "seg104.h"
#include "seg113.h"

struct struct_chest g_dng02_specialchests[8] = {
	{ DNG_POS(0,3,11),	0, NULL,			NULL,			DNG02_chest00_loot, 0, 0, 0 },
	{ DNG_POS(0,8,10),	0, NULL,			NULL,			DNG02_chest01_loot, 0, 0, 0 },
	{ DNG_POS(1,4,8),	4, use_lockpicks_on_chest,	chest_protected_heavy,	DNG02_chest02_loot, 0, 0, 0 },
	{ DNG_POS(1,4,10),	4, use_lockpicks_on_chest,	chest_protected_heavy,	DNG02_chest03_loot, 0, 0, 0 },
	{ DNG_POS(1,8,12),	0, NULL,			NULL,			DNG02_chest04_loot, 0, 0, 0 },
	{ DNG_POS(1,14,7),	6, use_lockpicks_on_chest,	chest_petrified,	DNG02_chest05_loot, 0, 0, 0 },
	{ DNG_POS(1,1,1),	1, DNG02_chest06_open,		NULL,			DNG02_chest06_loot, 0, 0, 0 },
	{     -1,		0, NULL,			NULL,			NULL,		0, 0, 0 }
}; // ds:0x93d8

/**
 * \brief   dungeon handler of the tumbledown inn
 */
signed int DNG02_handler(void)
{
	signed int target_pos;
	signed int mod_slot; // multi use
	signed int i; // multi use
	signed int hero_weight;
	int32_t weight_sum;
	struct struct_hero *hero;
	uint8_t *amap_ptr;
	signed char success_flag;

	target_pos = DNG_POS(gs_dungeon_level, gs_x, gs_y);

	amap_ptr = g_dng_map;

	if ((target_pos == DNG_POS(0,1,2) || target_pos == DNG_POS(0,2,1)))
	{
		// fight against two zombies. they only appear at night.
		if (gs_day_timer >= HOURS(8) && gs_day_timer <= HOURS(20))
		{
		} else {

			g_fig_escape_position[NORTH] = g_fig_escape_position[EAST] = g_fig_escape_position[SOUTH] = g_fig_escape_position[WEST] = DNG_POS_DIR(0,2,6,NORTH);

			do_fight(FIGHT_ID_F046_01);
		}

	} else if (target_pos == DNG_POS(0,3,6) && target_pos != gs_dng_pos_bak && gs_viewdir == SOUTH)
	{
		strcpy(g_dtp2, get_tx(4)); /* "DIES IST DER EHEMALIGE KÜHLKELLER. IHR MERKT AUCH, DASS DIE LUFT HI..." */
		// cooling cellar: room is much colder than the outside

		strcat(g_dtp2,(char*)((
			test_spell_group(SP_ODEM_ARCANUM, 0) == 0)
				? get_tx(5) /* "OB DIES AUFGRUND DER BESCHAFFENHEIT DER WÄNDE SO IST ODER OB VIELLE..." */
				// unclear whether this is caused by magic

				: get_tx(6) /* "DIE WÄNDE DES RAUMS SIND VON SCHWACHEM BLÄULICHEM FLIMMERN DURCHZOGEN." */
				// The walls show a bluish shimmering. (a hint to a magic cause)
			)
		);

		GUI_output(g_dtp2);

	} else if (target_pos == DNG_POS(0,4,10) && target_pos != gs_dng_pos_bak && gs_viewdir == SOUTH)
	{
		GUI_output(get_tx(7)); /* "DIES IST WOHL DER RAUM, IN DEM DER WIRT DER HERBERGE FRÜHER SEINEWE..." */
		// former storage room for valuables

	} else if ((target_pos == DNG_POS(0,7,13) || target_pos == DNG_POS(1,6,5) || target_pos == DNG_POS(1,4,14)) && target_pos != gs_dng_pos_bak)
	{
		if (test_talent((hero = get_first_hero_available_in_group()), TA_SINNESSCHAERFE, 6) > 0)
		{
			GUI_output(get_tx(8)); /* "HIER BEDECKT EINE KLEBRIGE FLÜSSIGKEIT DEN BODEN, ES IST JEDOCH EIN..." */
			// sticky liquid on the floor; crossing is easy.

		} else {

			sprintf(g_dtp2, get_tx(9), hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST)); /* "%s BLEIBT PLÖTZLICH WIE ANGEWURZELT STEHEN. %s KANN DIE FÜSSE NICHT..." */
			GUI_output(g_dtp2);
			// Group leader is stuck to the floor.

			success_flag = 0;

			while ((hero->le > 10) && !success_flag)
			{
				if (test_attrib(hero, ATTRIB_KK, 4) <= 0)
				{
					sprintf(g_dtp2, get_tx(10), hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST)); /* "%s ZERRT VERGEBLICH, KOMMT ABER NICHT LOS!" */
					GUI_output(g_dtp2);
					// Group leader fails to free himself. //

					sub_hero_le(hero, 1);

				} else {
					success_flag = 1;
				}
			}

#ifndef M302de_ORIGINAL_BUGFIX
			// Original-Bug 65:
			// verfallene Herberge: If the group leader becomes stuck in the sticky liquid on the floor,
			// the "freed himself" message only appears after a prior failed attempt that reduced his LE to 10 or below.

			if (!success_flag)
			{
				// Group leader was released "out of grace" to keep his LE from dropping to 10 or below.

				sprintf(g_dtp2,	get_tx(11), hero->alias); /* "%s GELINGT ES, SICH LOSZUREISSEN!" */
				GUI_output(g_dtp2);
				// Group leader succeeds in freeing himself. //

				// 1D6 extra damage to compensate for the out-of-grace release.
				sub_hero_le(hero, (hero->le > 6 ? random_schick(6) : hero->le - 1));

			}
#else
			sprintf(g_dtp2, get_tx(11), hero->alias); /* "%s GELINGT ES, SICH LOSZUREISSEN!" */
			GUI_output(g_dtp2);
			// Group leader succeeds in freeing himself.

			if (!success_flag)
			{
				// Group leader was released "out of grace" to keep his LE from dropping to 10 or below.
				// 1D6 extra damage to compensate for that action.

				sub_hero_le(hero, (hero->le > 6 ? random_schick(6) : hero->le - 1));

			}
#endif

		}

	} else if (target_pos == DNG_POS(0,6,10))
	{
		DNG_pitfall_add_pit(2);

	} else if (target_pos == DNG_POS(0,14,14) && target_pos != gs_dng_pos_bak)
	{
		/* If the combined weight of the first two heroes is at least 7000 ounces, a trapdoor is triggered. */

		hero = get_first_hero_available_in_group();
		weight_sum = get_hero_weight(hero);

		if ((hero = get_second_hero_available_in_group()))
		{
			weight_sum += get_hero_weight(hero);
		}

		if (weight_sum >= 7000L)
		{
			DNG_pitfall(6);
		}

	} else if ((target_pos == DNG_POS(0,10,12) || target_pos == DNG_POS(0,10,10)) && target_pos != gs_dng_pos_bak)
		// Besides the obvious main purpose of this code branch -- handling entry into squares (0,10,12) and (0,10,10) --
		// it also covers the effect of leaving square (0,10,11):
		// A group there might have prevented the wall at (0,10,11) from reappearing.
		//
		// Original-Bug: groups teleporting away from (0,10,11) are not detected.
	{
		hero = get_hero(0);
		for (i = (signed int)(weight_sum = 0); i <= 6; i++, hero++)
		{
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id))
			{
				weight_sum += get_hero_weight(hero);
			}
		}

		// Prevent the wall at (0,10,11) from appearing if there is a group on that square.
		for (i = 0; i < 6; i++) {

			if (
				(gs_groups_x[i] == 10 && gs_groups_y[i] == 11)
				&& (gs_groups_dng_level[i] == gs_dungeon_level)
				&& (gs_groups_dungeon_id[i] == gs_dungeon_id)
			) {
				weight_sum = 0L;
			}
		}

		hero_weight = get_hero_weight(get_heaviest_hero());

		amap_ptr[MAP_POS(10,11)] = (
			hero_weight >= weight_sum
			// By the above logic, this condition is true if the current group consists only of a single hero,
			// or if there is a group located on square (0,10,11).

				? (DNG_TILE_CORRIDOR << 4)
				: (DNG_TILE_WALL << 4)
		);

		play_voc(305);

	} else if ((target_pos == DNG_POS(0,10,13) || target_pos == DNG_POS(0,10,9)) && target_pos != gs_dng_pos_bak)
	{
		// Purpose: detect groups moving away from (0,10,12) or (0,10,10) to remove the wall at (0,10,11).
		// Original-Bug: groups teleporting away are not detected.
		// Original-Bug: no check is made for another group on (0,10,12) or (0,10,10) that would make the wall at (0,10,11) appear.

		amap_ptr[MAP_POS(10,11)] = DNG_TILE_CORRIDOR << 4;

		play_voc(305);

	} else if ((target_pos == DNG_POS(0,7,6) || target_pos == DNG_POS(0,8,6) || target_pos == DNG_POS(0,8,8)))
	{
		if (gs_dng02_sphaerenriss_activated)
		{
			/* Sphaerenriss */
			GUI_output(get_tx(41)); /* "IN DIESEM RAUM ENTSTAND DURCH MAGIE EIN RIESIGER RISS IMRAUM-ZEIT-K..." */

			hero = get_hero(0);
			for (i = 0; i <= 6; i++, hero++)
			{
				if ((hero->typus != HERO_TYPE_NONE) &&
					(hero->group_id == gs_active_group_id) &&
					!hero->flags.dead) /* TODO: Why !dead? */
				{
					hero_disappear(hero, i, -1);
				}
			}

		} else {
			hero = get_hero(0);
			for (i = mod_slot = 0; i <= 6; i++, hero++)
			{
				if ((hero->typus != HERO_TYPE_NONE) &&
					(hero->group_id == gs_active_group_id) &&
					!hero->flags.dead &&
					!hero->flags.seen_phantom)
				{
					mod_slot = get_free_mod_slot();
					set_mod_slot(mod_slot, HOURS(5), (uint8_t*)&hero->attrib[ATTRIB_MU].current, -3, (signed char)i);
					{
						/* mod_slot with different usage */
						hero->flags.seen_phantom = mod_slot = 1;
					}
				}
			}

			if (mod_slot != 0)
			{
				GUI_output(get_tx(12)); /* "DER RAUM, DEN IHR BETRETET, IST VOLLKOMMEN LEER. IN DER MITTE JEDOC..." */
				// A terrifying figure above a pentagram
			}
		}

	} else if (target_pos == DNG_POS(0,7,1) && !gs_dng02_raven_destroyed && target_pos != gs_dng_pos_bak)
	{
		if (GUI_bool(get_tx(13))) /* "HIER STEHT EINE STATUE, DIE EINEN GEKÖPFTEN RABEN DARSTELLT - OFFEN..." */
		// A raven statue mocking Boron. Destroy?
		{
			gs_dng02_raven_destroyed = 1;

			sub_group_le(random_schick(20));

			GUI_output(get_tx(14)); /* "ALS IHR AUF DIE STATUE EINSCHLAGT, DURCHZUCKT EIN GRELLER BLITZ DEN..." */
			// Statue destroyed in a blinding flash; secret door in the west wall opened.

			amap_ptr[MAP_POS(5,2)] = DNG_TILE_CORRIDOR << 4;
		}

	} else if (target_pos == DNG_POS(0,11,6) && target_pos != gs_dng_pos_bak)
	{
		hero = get_hero(0);
		for (i = (signed int)(weight_sum = 0); i <= 6; i++, hero++)
		{
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id))
			// Original-Bug: Heroes from other groups on the same square (0,11,6) should be included.
			// (Note that moving away from (0,11,6), heroes from other groups are included.)
			{
				weight_sum += get_hero_weight(hero);
			}
		}

		if (weight_sum >= 4000)
		{
			// Group heavy enough -> wall at (0,11,8) disappears

			GUI_output(get_tx(15)); /* "IHR HÖRT VON IRGENDWOHER EIN KNIRSCHEN." */
			// A crunching sound
			// Original-Bug: this message also appears when the wall has already been removed.

			amap_ptr[MAP_POS(11,8)] = DNG_TILE_CORRIDOR << 4;
		}

	} else if ((target_pos == DNG_POS(0,10,6) || target_pos == DNG_POS(0,11,5)) && target_pos != gs_dng_pos_bak)
	{
		// Purpose: detect groups moving away from (0,11,6) to determine whether the wall at (0,11,8) should reappear.
		// Original-Bug: Groups teleporting away from (0,11,6) are not detected.
		// (former comment said (0,11,8), but the weight plate is at (0,11,6))

		for (i = 0; i < 6; i++)
		{
			/* Original-Bug: weight_sum is reset for every group, so the dummy value 5000 set below
			 * for a group on (0,11,8) is lost unless that group is the last one (i == 5).
			 * The reset belongs before the loop. */
			weight_sum = 0;

			if (gs_groups_x[i] == 11 && gs_groups_y[i] == 6 &&
				gs_groups_dng_level[i] == gs_dungeon_level &&
				gs_groups_dungeon_id[i] == gs_dungeon_id)
			{
				hero = get_hero(0);
				for (mod_slot = 0; mod_slot <= 6; mod_slot++, hero++)
				{
					if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == i))
					{
						weight_sum += get_hero_weight(hero);
					}
				}

				break;

			} else if (gs_groups_x[i] == 11 &&
					gs_groups_y[i] == 8 &&
					gs_groups_dng_level[i] == gs_dungeon_level &&
					gs_groups_dungeon_id[i] == gs_dungeon_id)
			{
				// Prevent the wall from reappearing if there is a group on that square.
				// Original-Bug: if that group moves away later, the wall remains open.

				weight_sum = 5000L; // some dummy value >= 4000.
			}
		}

		amap_ptr[MAP_POS(11,8)] = (weight_sum >= 4000L ? (DNG_TILE_CORRIDOR << 4) : (DNG_TILE_WALL << 4));

	} else if (target_pos == DNG_POS(1,12,5) && gs_viewdir == NORTH &&
			 (target_pos != gs_dng_pos_bak || gs_viewdir != gs_viewdir_bak))
	{
		gs_viewdir_bak = gs_viewdir;

		GUI_output(get_tx(16)); /* "DIE WAND IM NORDEN TRÄGT EINE INSCHRIFT:@VORSICHT NACH DREI SCHRITT..." */
		// Inscription in the northern wall: "Attention after three steps to the left."
		// This is a hint for the secret passage at (1,8,4).
		// Original-Bug: Shouldn't this be "three steps to the right?"

	} else if (target_pos == DNG_POS(1,4,5) && gs_viewdir == SOUTH &&
			 (target_pos != gs_dng_pos_bak || gs_viewdir != gs_viewdir_bak))
	{
		gs_viewdir_bak = gs_viewdir;

		GUI_output(get_tx(17)); /* "DIE WAND IM SÜDEN TRÄGT EINE INSCHRIFT:@VORSICHT NACH DREI SCHRITTE..." */
		// Inscription in the southern wall: "Attention after three steps to the left."
		// This is a hint for the secret passage at (1,8,4).
		// Original-Bug: Shouldn't this be "three steps to the right?"

	} else if (target_pos == DNG_POS(1,7,1) && !gs_dng02_sphaerenriss_timer_activated)
	{
		load_in_head(43);

		GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx(18), 0); /* "IN DIESER NISCHE IST EIN MENSCH AN DIE WAND GEKETTET, DER OFFENSICH..." */
		// A chained, mad man babbles about the "Sphaerenriss".

		// The timer for the Sphaerenriss is activated!
		gs_dng02_sphaerenriss_timer_hours = 7; // 7 hours
		gs_dng02_sphaerenriss_timer_activated = 1;

	} else if ((target_pos == DNG_POS(1,1,11) || target_pos == DNG_POS(1,1,8)) && target_pos != gs_dng_pos_bak)
	{
		GUI_output(get_tx(22)); /* "ES SIEHT AUS, ALS WÜRDEN AN DIESEM TISCH ALCHIMISTISCHE EXPERIMENTE..." */
		// an alchemy table

	} else if (target_pos == DNG_POS(1,3,7) && target_pos != gs_dng_pos_bak)
	{
		// This lever is a red herring. It has no effect.

		if (GUI_bool(get_tx(23))) /* "HIER RAGT EIN HEBEL AUS DER WAND.@WOLLT IHR IHN BETÄTIGEN?" */
		// a lever...
		{
			GUI_output(get_tx(24)); /* "IHR HÖRT EIN LEISES KLICKEN." */
			// a clicking sound.
		}

	} else if (target_pos == DNG_POS(1,8,9) && target_pos != gs_dng_pos_bak && !gs_dng02_apparatus_destroyed)
	{
		/* petrification trap */

		GUI_output(get_tx(25)); /* "DIE LUFT HIER NIMMT EINEN SANFTEN VIOLETTEN TON AN." */
		// The air glows violet.

		hero = get_hero(0);
		for (i = 0; i <= 6; i++, hero++)
		{
			/* all heroes with MR < 8 are petrified */
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
				!hero->flags.dead && (hero->mr < 8))
			{
				hero->flags.petrified = 1;

				sprintf(
					g_dtp2,
					get_tx(42), /* "%s STÖHNT URPLÖTZLICH AUF. %s SPÜRT, WIE EINE BEKLEMMENDE, EISIGE K..." */ // within seconds, <hero> is petrified.
					hero->alias,
					GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST),
					GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST),
					GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST),
					hero->alias
				);

				GUI_output(g_dtp2);
			}
		}

		check_group();

	} else if (target_pos == DNG_POS(1,13,11) && target_pos != gs_dng_pos_bak)
	{
		do {
			i = GUI_radio(
				get_tx(26), /* "HIER FÜHRT EIN MECHANISCHER AUFZUG NACH OBEN.@WOLLT IHR" */ // A lift leads upward.
				2,
				get_tx(27), /* "DEN AUFZUG BENUTZEN?" */ // Use the lift.
				get_tx(28) /* "DEN RAUM WEITER UNTERSUCHEN?" */  // Investigate the room further.
			);

		} while (i == -1);

		if (i == 1)
		{
			// Use the lift

			DNG_dec_level();
			target_pos = DNG_POS(0,13,11);

			// Original-Bug: The following message is displayed whether or not the fight has already been triggered.

			do {
				i = GUI_radio(
					get_tx(29), /* "BIS AUF DIE BEIDEN ZAUBERER UND DIE MUMIEN, DIE HIER OFFENSICHTLICH..." */ // Two mages, guarded by two mummies, perform an invocation at a heptagon.
					2,
					get_tx(30), /* "DIE BESCHWÖRUNG UNTERBRECHEN?" */ // Interrupt the invocation.
					get_tx(31) /* "EUCH WIEDER VERDRÜCKEN?" */  // Leave.
				);

			} while (i == -1);

			if (i == 2)
			{
				// Leave

				DNG_inc_level();
				target_pos = DNG_POS(1,13,11);
			}
		}

	} else if (target_pos == DNG_POS(0,13,11) && target_pos != gs_dng_pos_bak)
	{
		do {
			i = GUI_radio(
				get_tx(43), /* "HIER FÜHRT EIN MECHANISCHER AUFZUG NACH UNTEN.@WOLLT IHR" */ // A lift leads downward.
				2,
				get_tx(27), /* "DEN AUFZUG BENUTZEN?" */ // Use the lift.
				get_tx(28) /* "DEN RAUM WEITER UNTERSUCHEN?" */  // Investigate the room further.
			);

		} while (i == -1);

		if (i == 1)
		{
			// Use the lift

			DNG_inc_level();
			target_pos = DNG_POS(1,13,11);

			do {
				i = GUI_radio(
					get_tx(26), /* "HIER FÜHRT EIN MECHANISCHER AUFZUG NACH OBEN.@WOLLT IHR" */ // A lift leads upward.
					2,
					get_tx(27), /* "DEN AUFZUG BENUTZEN?" */ // Use the lift.
					get_tx(28) /* "DEN RAUM WEITER UNTERSUCHEN?" */  // Investigate the room further.
				);

			} while (i == -1);

			if (i == 1)
			{
				// Use the lift

				DNG_dec_level();
				target_pos = DNG_POS(0,13,11);
			}
		}

	} else if ((target_pos == DNG_POS(0,12,11) || target_pos == DNG_POS(0,14,11) || target_pos == DNG_POS(0,13,10) || target_pos == DNG_POS(0,13,12)) && target_pos != gs_dng_pos_bak)
	{
		g_fig_escape_position[NORTH] = g_fig_escape_position[EAST] = g_fig_escape_position[SOUTH] = g_fig_escape_position[WEST] = DNG_POS_DIR(0,12,11,NORTH);

		if (!do_fight(FIGHT_ID_F046_31))
		{
#if defined(__BORLANDC__)
			gs_always_one2 = 1;
#endif

			add_hero_ap_all(30);
		}

	} else if (target_pos == DNG_POS(1,1,1) && target_pos != gs_dng_pos_bak)
	{
		loot_special_chest(0);

	} else if (target_pos == DNG_POS(1,9,14) && target_pos != gs_dng_pos_bak && gs_viewdir == EAST)
	{
		GUI_output(get_tx(35)); /* "DIES SCHEINT EINE FOLTERKAMMER ZU SEIN. AN DEN WÄNDEN HÄNGEN FOLTER..." */ // A torture chamber.

	} else if (target_pos == DNG_POS(0,1,8) &&
			(target_pos != gs_dng_pos_bak || gs_viewdir != gs_viewdir_bak) &&
			(gs_viewdir == SOUTH) &&
			(gs_dng02_secret_door_status_1 != DNG_SECRET_DOOR_STATUS_OPEN))
	{
#ifndef M302de_ORIGINAL_BUGFIX
		/* Original-Bug 66:
		 * This should be the leader of the active group, who is not necessarily at hero_pos 0. */
		hero = get_hero(0);
#else
		hero = get_first_hero_available_in_group();
#endif

		if (gs_dng02_secret_door_status_1 || test_talent(hero, TA_SINNESSCHAERFE, 6) > 0)
		{
			gs_dng02_secret_door_status_1 = DNG_SECRET_DOOR_STATUS_DISCOVERED;

			sprintf(g_dtp2, get_tx(37), hero->alias); /* "EIN GENAUER BLICK AUF DIE WAND ENTHÜLLT %s EINE GEHEIMTÜR." */
			// you find a secret door

			sprintf(g_text_output_buf,
				(char*)((i = test_talent(hero, TA_SCHLOESSER, 4)) > 0
					? get_tx(39) /* "@MIT EINEM POLTERN ÖFFNET SIE SICH, ALS %s SICH AN DEM SCHLOSS ZUSC..." */ // the secret door opens
					: get_tx(38) /* "@LEIDER KANN %s SIE NICHT ÖFFNEN!" */ // <hero> fails to open the secret door
				),
			GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST));

			strcat(g_dtp2, g_text_output_buf);

			GUI_output(g_dtp2);

			if (i > 0)
			{
				/* the secret door opens */

				/* unlike other similar code positions, the lower 4 bits of the map entry are preserved here. Is there a reason? */
				amap_ptr[MAP_POS(1,9)] &= (DNG_TILE_CORRIDOR << 4) + 0x0f;
				gs_dng02_secret_door_status_1 = DNG_SECRET_DOOR_STATUS_OPEN;
				DNG_update_pos();
			}

			gs_viewdir_bak = gs_viewdir;
		}

	} else if (target_pos == DNG_POS(0,4,8) &&
			(target_pos != gs_dng_pos_bak || gs_viewdir != gs_viewdir_bak) &&
			(gs_viewdir == SOUTH) &&
			(gs_dng02_secret_door_status_2 != DNG_SECRET_DOOR_STATUS_OPEN))
	{
#ifndef M302de_ORIGINAL_BUGFIX
		/* Original-Bug 66:
		 * This should be the leader of the active group, who is not necessarily at hero_pos 0. */
		hero = get_hero(0);
#else
		hero = get_first_hero_available_in_group();
#endif

		if (gs_dng02_secret_door_status_2 || test_talent(hero, TA_SINNESSCHAERFE, 2) > 0)
		{
			gs_dng02_secret_door_status_2 = DNG_SECRET_DOOR_STATUS_DISCOVERED;

			sprintf(g_dtp2,	get_tx(37), hero->alias); /* "EIN GENAUER BLICK AUF DIE WAND ENTHÜLLT %s EINE GEHEIMTÜR." */
			// you find a secret door

			sprintf(g_text_output_buf,
				(char*)((i = test_talent(hero, TA_SCHLOESSER, 2)) > 0
					? get_tx(39) /* "@MIT EINEM POLTERN ÖFFNET SIE SICH, ALS %s SICH AN DEM SCHLOSS ZUSC..." */ // the secret door opens
					: get_tx(38) /* "@LEIDER KANN %s SIE NICHT ÖFFNEN!" */ // <hero> fails to open the secret door
				),
			GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST));

			strcat(g_dtp2, g_text_output_buf);

			GUI_output(g_dtp2);

			if (i > 0)
			{
				/* the secret door opens */

				/* here the lower 4 bits of the map entry are cleared (unlike secret door 1 above, where the comment was copied from) */
				amap_ptr[MAP_POS(4,9)] = DNG_TILE_CORRIDOR << 4;
				gs_dng02_secret_door_status_2 = DNG_SECRET_DOOR_STATUS_OPEN;
				DNG_update_pos();
			}

			gs_viewdir_bak = gs_viewdir;
		}

	} else if (((target_pos == DNG_POS(1,4,9) && gs_viewdir == EAST) ||
			(target_pos == DNG_POS(1,6,9) && gs_viewdir == WEST)) &&
			(target_pos != gs_dng_pos_bak || gs_viewdir != gs_viewdir_bak) &&
			(gs_dng02_secret_door_status_3 != DNG_SECRET_DOOR_STATUS_OPEN))
	{
		hero = get_first_hero_available_in_group();

		if (gs_dng02_secret_door_status_3 || (test_talent(hero, TA_SINNESSCHAERFE, 5) > 0))
		{
			gs_dng02_secret_door_status_3 = DNG_SECRET_DOOR_STATUS_DISCOVERED;

			sprintf(g_dtp2,	get_tx(37), hero->alias); /* "EIN GENAUER BLICK AUF DIE WAND ENTHÜLLT %s EINE GEHEIMTÜR." */
			// you find a secret door

			sprintf(g_text_output_buf,
				(char*)((i = test_talent(hero, TA_SCHLOESSER, 4)) > 0
					? get_tx(39) /* "@MIT EINEM POLTERN ÖFFNET SIE SICH, ALS %s SICH AN DEM SCHLOSS ZUSC..." */ // the secret door opens
					: get_tx(38) /* "@LEIDER KANN %s SIE NICHT ÖFFNEN!" */ // <hero> fails to open the secret door
			       ),
			GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST));

			strcat(g_dtp2, g_text_output_buf);

			if (i > 0)
			{
				/* the secret door opens */

				/* here the lower 4 bits of the map entry are cleared (unlike secret door 1 above, where the comment was copied from) */
				amap_ptr[MAP_POS(5,9)] = DNG_TILE_CORRIDOR << 4;
				gs_dng02_secret_door_status_3 = DNG_SECRET_DOOR_STATUS_OPEN;
				DNG_update_pos();
			}

			/* the crossbow bolt hits every time the door is approached, until it is open */
			i = random_schick(6) + 4;

			sprintf(g_text_output_buf, get_tx(40), hero->alias, i); /* "@%s WIRD VON EINEM ARMBRUSTBOLZEN GETROFFEN." */
			// <hero> is hit by a crossbow bolt

			strcat(g_dtp2, g_text_output_buf);

			GUI_output(g_dtp2);

			sub_hero_le(hero, i);

			gs_viewdir_bak = gs_viewdir;
		}

	} else if (target_pos == DNG_POS(0,1,0) && target_pos != gs_dng_pos_bak)
	{
		/* exit */

		leave_dungeon();
		gs_town_id = gs_travel_destination_town_id;
		gs_x = gs_travel_destination_x;
		gs_y = gs_travel_destination_y;
		gs_town_loc_type = LOCTYPE_NONE;
		gs_viewdir = REVERSE_VIEWDIR(gs_travel_destination_viewdir);

		sprintf(g_dtp2, get_tx(44), get_ttx(gs_journey_destination_town_id + 0xeb)); /* "VOLLKOMMEN ERSCHÖPFT UND AUSGELAUGT VERLAS..." / TEXT.LTX 236..287: Ortsname */
		/* You leave the dungeon and reach your true destionation, <town>, a few hours later. */

		GUI_output(g_dtp2);

		timewarp(HOURS(3));

		g_fading_state = 3;
	}

	gs_dng_pos_bak = target_pos;

	return 0;
}

void DNG02_chest00_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = (gs_dng02_chest00_content);

	loot_simple_chest(chest);

	chest->loot.content = ptr_bak;
}

void DNG02_chest01_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng02_chest01_content;

	loot_simple_chest(chest);

	chest->loot.content = ptr_bak;
}

void DNG02_chest02_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng02_chest02_content;

	loot_simple_chest(chest);

	chest->loot.content = ptr_bak;
}

void DNG02_chest03_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng02_chest03_content;

	loot_simple_chest(chest);

	chest->loot.content = ptr_bak;
}

/**
 * \brief   chest with the violet crystal apparatus (source of the petrification trap at (1,8,9))
 *
 *          taking it petrifies (chest_petrified()), destroying it costs every hero 1W6 LE
 *          and switches the trap off (gs_dng02_apparatus_destroyed)
 */
void DNG02_chest04_loot(struct struct_chest* chest)
{
	signed int i;
	signed int answer;
	signed int tw_bak;
	struct struct_hero *hero;

	tw_bak = g_textbox_width;
	g_textbox_width = 7;

	if (!gs_dng02_apparatus_destroyed)
	{
		do {
			answer = GUI_radio(
				get_tx(1), /* "IN DIESER TRUHE LAGERT, AUF SAMT GEBETTET, EINE SELTSAME APPARATUR..." */ // In the chest lies a strange crystal apparatus, glowing violet.

				2,
				get_tx(2), /* "DIE APPARATUR MITNEHMEN?" */ // Take it with you.
				get_tx(3) /* "DIE APPARATUR ZERSCHLAGEN?" */  // Destroy it.
			);

		} while (answer == -1);

		if (answer == 1)
		{
			/* take it -> the first hero is petrified (chest_petrified() shows the poison needle text) */
			chest_petrified();
		} else {

			gs_dng02_apparatus_destroyed = 1;

			hero = get_hero(0);
			for (i = 0; i <= 6; i++, hero++)
			{
				if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id))
				{
					sub_hero_le(hero, random_schick(6));
				}
			}
		}
	} else {
		GUI_output(get_ttx(522)); /* "DIE TRUHE IST LEER!" */ // Chest is empty.
	}

	g_textbox_width = tw_bak;
}

void DNG02_chest05_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng02_chest05_content;

	loot_simple_chest(chest);

	chest->loot.content = ptr_bak;
}

void DNG02_chest06_open(struct struct_chest* chest)
{
	loot_corpse(
		chest,
		get_tx(19), /* "HIER LIEGT DIE NOCH RECHT FRISCHE LEICHE EINES JUNGEN WANDERERS, DE..." */ // corpse of a wanderer, slain by zombies.
		(int8_t*)&gs_dng02_wanderer_corpse_looted
	);
}

void DNG02_chest06_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng02_chest06_content;

	loot_chest(
		chest,
		get_tx(20), /* "DER WANDERER HAT NOCH FOLGENDE GEGENSTÄNDE BEI SICH" */ // There are these items...
		get_tx(21) /* "DA GIBT'S NICHTS MEHR ZU HOLEN!" */  // There is nothing left
	);

	chest->loot.content = ptr_bak;
}

/**
 * \brief   print introductions of dungeon #2 fights
 *
 * \param   fight_id    ID of the fight
 */
void DNG02_fight_intro(const signed int fight_id)
{
	if (fight_id == FIGHT_ID_F046_13) {
		GUI_output(get_tx(32)); /* "BIS AUF DIE VIER ZOMBIES, DIE SICH EUCH BEDROHLICH NÄHERN, IST DER..." */ // Four approaching zombies.
	} else if (fight_id == FIGHT_ID_F046_22) {
		GUI_output(get_tx(33)); /* "EIN SKELETT SCHÜTTELT SEINE KETTEN AB UND GREIFT EUCH AN!" */ // A skeleton attacks.
	} else if (fight_id == FIGHT_ID_F046_25) {
		GUI_output(get_tx(34)); /* "DER GROSSE RAUM IST VON BEISSENDEM GESTANK ERFÜLLT. DURCH DIE SCHWA..." */ // You and an alchemist notice each other; at his gesture, five shadowy figures emerge and attack.
	} else if (fight_id == FIGHT_ID_F046_28) {
		GUI_output(get_tx(36)); /* "EIN MUSKELBEPACKTER, KLEINWÜCHSIGER MANN SÄUBERT GERADE EINE EISERN..." */ // A man cleans an iron maiden; spotting you, he shouts an alarm and attacks.
	}
}
