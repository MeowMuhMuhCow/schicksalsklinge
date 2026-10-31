/*
 * seg087 - Dungeon "Alte Zwingfeste" in Thorwal (DNG14), Teil 1/2
 *
 * Inhalt:    DNG14_handler() steuert die Ereignisse der alten Zwingfeste unter Thorwal
 *            ueber fuenf Ebenen: Zufallskaempfe mit den Raeubern, Phex-Opferschrein
 *            (22,10 D, Phex-Ansehen -100), Alarm, Speisekammer und Ausruestungsdepot,
 *            Giftgasraum hinter Geheimtuer 2, Hauptkampf auf Ebene 2, Beutelager,
 *            Ugdalfs verrammelte Tuer (Quest), fuenf Geheimtueren, Schacht mit Seil,
 *            Geistererscheinung (MU -2), Feuerfalle, Tauchstrecke (DNG14_dive, seg088),
 *            Fundstuecke (Fackeln, Speer, Hacke, Ring, Orknase), der Ausgang durch den
 *            Zwergenkeller (Saufgelage) und der regulaere Ausgang.
 * Aufrufer:  DNG14_handler() ueber g_dng_handlers (seg076).
 * Daten:     keine eigenen; Truhen und Hilfsfunktionen in seg088.
 * Besonders: Kommentare "activate/disable trap" bei Geheimtuer 3-5 waren von Tuer 2
 *            kopiert (nur dort aktiviert ein Fehlschlag die Giftfalle). Schleimfalle
 *            (3,12,12) macht 3 LE, nicht 1W6.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg087.c
 */
#include <stdio.h>
#include <string.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg007.h"
#include "seg025.h"
#include "seg029.h"
#include "seg031.h"
#include "seg032.h"
#include "seg047.h"
#include "seg075.h"
#include "seg088.h"
#include "seg092.h"
#include "seg096.h"
#include "seg097.h"
#include "seg103.h"
#include "seg105.h"

/**
 * \brief   dungeon handler of the old Zwingfeste in Thorwal (levels 1-5)
 */
signed int DNG14_handler(void)
{
	signed int target_pos;
	signed int tmp; /* multiple use: mod_slot, item_count, result of talent test */
	signed int hero_pos;
	signed int tw_bak;
	int32_t p_money;
	struct struct_hero *hero;
	uint8_t *amap_ptr;

	amap_ptr = g_dng_map;

	tw_bak = g_textbox_width;
	g_textbox_width = 7;

	target_pos = DNG_POS(gs_dungeon_level, gs_x, gs_y);

	hero = get_first_hero_available_in_group();

	/* poison gas room: every 10 minutes 1 LE for each hero while the trap is active (gs_dng14_poisontrap == 2).
	 * REMARK: gs_day_timer advances in the timer interrupt, so the condition may hold for several
	 * passes of the dungeon loop in a row (vermutlich more than 1 LE per 10 minutes). */
	if (!(gs_day_timer % MINUTES(10)) &&
		(gs_dng14_poisontrap == 2) &&
		(target_pos == DNG_POS(0,13,6) || target_pos == DNG_POS(0,14,6) || target_pos == DNG_POS(0,13,7) || target_pos == DNG_POS(0,14,7)))
	{
		sub_group_le(1);
	}

	if (target_pos == DNG_POS(0,2,11) && target_pos != gs_dng_pos_bak && random_schick(100) <= 10) {

		g_fig_escape_position[NORTH] = g_fig_escape_position[EAST] = DNG_POS_DIR(0,3,8,EAST);
		g_fig_escape_position[SOUTH] = g_fig_escape_position[WEST] = DNG_POS_DIR(0,2,14,SOUTH);

		do_fight(FIGHT_ID_DTHO03);

	} else if (target_pos == DNG_POS(0,2,4) && target_pos != gs_dng_pos_bak && !gs_dng14_money_flag) {

		if (GUI_bool(get_tx(1))) { /* "IN DIESER NISCHE STEHT EIN KLEINER, DEM PHEX GEWEIHTER SCHREIN, DER..." */

			/* mark as visited */
			gs_dng14_money_flag = 1;

			/* add money */
			p_money = get_party_money();
			p_money += 2210L;
			set_party_money(p_money);

			/* set gods estimation for PHEX to -100 */
			gs_gods_estimation[GOD_ID_PHEX] = -100L;

			GUI_output(get_tx(2)); /* "ICH WEISS JA NICHT, OB ES EINE SO GUTE IDEE IST, EINEN OPFERSCHREIN..." */
		}

	} else if (target_pos == DNG_POS(0,5,3) && target_pos != gs_dng_pos_bak && !gs_dng14_alarm_flag) {

		if (test_talent(hero, TA_SINNESSCHAERFE, 6) <= 0) {

			GUI_output(get_tx(3)); /* "IHR HABT OFFENSICHTLICH EINEN ALARM AUSGELÖST..." */

			gs_dng14_alarm_flag = 1;
		}

	} else if (target_pos == DNG_POS(0,6,2) && target_pos != gs_dng_pos_bak && gs_dng14_alarm_flag) {

		g_fig_escape_position[NORTH] = g_fig_escape_position[EAST] = g_fig_escape_position[SOUTH] = g_fig_escape_position[WEST] = DNG_POS_DIR(0,5,4,SOUTH);

		do_fight(FIGHT_ID_DTHO06);

	} else if (target_pos == DNG_POS(0,7,6) && target_pos != gs_dng_pos_bak && gs_viewdir == NORTH) {

		loot_multi_chest(gs_dng14_chest_pantry, get_tx(4)); /* "DIES MUSS DIE SPEISEKAMMER SEIN, DEM ZEUG NACH ZU URTEILEN, DAS HIE..." */

	} else if (target_pos == DNG_POS(0,6,11) && target_pos != gs_dng_pos_bak && gs_viewdir == EAST) {

		loot_multi_chest(gs_dng14_chest_gear, get_tx(5)); /* "HIER SEID IHR IM AUSRÜSTUNGSDEPOT GELANDET. EINE MENGE ZEUG LIEGT H..." */

	} else if ((target_pos == DNG_POS(0,9,13) || target_pos == DNG_POS(0,9,9)) && target_pos != gs_dng_pos_bak && random_schick(100) <= 20) {

		g_fig_escape_position[NORTH] = DNG_POS_DIR(0,9,8,NORTH);
		g_fig_escape_position[EAST] = DNG_POS_DIR(0,11,11,EAST);
		g_fig_escape_position[SOUTH] = g_fig_escape_position[WEST] = DNG_POS_DIR(0,9,14,SOUTH);

		do_fight(FIGHT_ID_DTHO09);

	} else if (target_pos == DNG_POS(0,11,14) && target_pos != gs_dng_pos_bak && random_schick(100) <= 10) {

		g_fig_escape_position[NORTH] = g_fig_escape_position[EAST] = g_fig_escape_position[SOUTH] = g_fig_escape_position[WEST] = DNG_POS_DIR(0,8,14,WEST);

		do_fight(FIGHT_ID_DTHO10);

	} else if (target_pos == DNG_POS(0,11,14) && target_pos != gs_dng_pos_bak && gs_viewdir == EAST) {

		GUI_output(get_tx(6)); /* "DIESER RAUM IST ALS BADERAUM EINGERICHTET. HIER KANN AUFGEFANGENESR..." */

	} else if (target_pos == DNG_POS(0,13,7) && target_pos != gs_dng_pos_bak && (gs_dng14_poisontrap != 0)) {

		GUI_output(get_tx(10)); /* "IHR HÖRT EIN SCHARFES KLICKEN, ALS DIE TÜR HINTER EUCH WIEDER INS S..." */

		GUI_output(get_tx(11)); /* "AUS FEINEN DÜSEN IN DER DECKE SENKT SICH EIN GELBLICHER NEBEL AUF E..." */

		gs_dng14_poisontrap = 2;

		amap_ptr[MAP_POS(12,7)] |= DNG_TILE_REMOVABLE_WALL << 4;

	} else if (target_pos == DNG_POS(0,13,5) && target_pos != gs_dng_pos_bak) {

		gs_dng14_poisontrap = 0;

	} else if (target_pos == DNG_POS(0,11,10) && target_pos != gs_dng_pos_bak && random_schick(100) <= 10) {

		g_fig_escape_position[NORTH] = g_fig_escape_position[EAST] = g_fig_escape_position[SOUTH] = g_fig_escape_position[WEST] = DNG_POS_DIR(0,9,11,WEST);

		do_fight(FIGHT_ID_DTHO13);

	} else if (target_pos == DNG_POS(0,10,3) && target_pos != gs_dng_pos_bak) {

		GUI_output(get_tx(12)); /* "HIER DREHT SICH GERADE EIN FETTER HASE ÜBER DEM FEUER." */

	} else if (target_pos == DNG_POS(0,4,11) && (target_pos != gs_dng_pos_bak || gs_viewdir != gs_viewdir_bak) &&
			gs_viewdir == EAST && gs_dng14_secretdoor1 != 2) {

		if (gs_dng14_secretdoor1 != 0 || test_talent(hero, TA_SINNESSCHAERFE, 4) > 0) {
			gs_dng14_secretdoor1 = 1;

			sprintf(g_dtp2, get_tx(7), hero->alias); /* "BEI GENAUEM HINSEHEN, ENTDECKT %s EINE GEHEIMTÜRE IN DER WAND." */

			sprintf(g_text_output_buf,
				(char*)((tmp = test_talent(hero, TA_SCHLOESSER, -6)) > 0 ? get_tx(8): get_tx(9)), /* "@NACH KURZER ZEIT SCHAFFT %s ES SOGAR, SIE..." / "@LEIDER SCHAFFT %s ES ALLERDINGS NICHT, SI..." */
				GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST));

			strcat(g_dtp2, g_text_output_buf);

			GUI_output(g_dtp2);

			if (tmp > 0) {
				amap_ptr[MAP_POS(5,11)] &= (DNG_TILE_CORRIDOR << 4) + 0x0f;
				gs_dng14_secretdoor1 = 2;
				DNG_update_pos();
			}

			gs_viewdir_bak = gs_viewdir;
		}

	} else if (target_pos == DNG_POS(0,11,7) &&
			(target_pos != gs_dng_pos_bak || gs_viewdir != gs_viewdir_bak) &&
			gs_viewdir == EAST &&
			gs_dng14_secretdoor2 != 2) {

		if (gs_dng14_secretdoor2 != 0 || test_talent(hero, TA_SINNESSCHAERFE, 4) > 0) {
			gs_dng14_secretdoor2 = 1;

			sprintf(g_dtp2, get_tx(7), hero->alias); /* "BEI GENAUEM HINSEHEN, ENTDECKT %s EINE GEHEIMTÜRE IN DER WAND." */

			sprintf(g_text_output_buf,
				(char*)((tmp = test_talent(hero, TA_SCHLOESSER, 6)) > 0 ? get_tx(8): get_tx(9)), /* "@NACH KURZER ZEIT SCHAFFT %s ES SOGAR, SIE..." / "@LEIDER SCHAFFT %s ES ALLERDINGS NICHT, SI..." */
				GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST));

			strcat(g_dtp2, g_text_output_buf);

			GUI_output(g_dtp2);

			if (tmp > 0) {
				amap_ptr[MAP_POS(12,7)] &= (DNG_TILE_CORRIDOR << 4) + 0x0f;
				gs_dng14_secretdoor2 = 2;
				DNG_update_pos();
			} else {
				/* a failed lock test arms the poison gas trap at (0,13,7) */
				gs_dng14_poisontrap = 1;
			}

			gs_viewdir_bak = gs_viewdir;
		}

	} else if (target_pos == DNG_POS(1,1,8) && target_pos != gs_dng_pos_bak && random_schick(100) <= 30) {

		g_fig_escape_position[NORTH] = g_fig_escape_position[EAST] = g_fig_escape_position[SOUTH] = g_fig_escape_position[WEST] = DNG_POS_DIR(1,1,6,NORTH);

		do_fight(FIGHT_ID_DTHO18);

	} else if (target_pos == DNG_POS(1,1,11) && target_pos != gs_dng_pos_bak && random_schick(100) <= 5) {

		g_fig_escape_position[NORTH] = g_fig_escape_position[EAST] = DNG_POS_DIR(1,1,9,NORTH);
		g_fig_escape_position[SOUTH] = g_fig_escape_position[WEST] = DNG_POS_DIR(1,1,14,SOUTH);

		do_fight(FIGHT_ID_DTHO19);

	} else if (target_pos == DNG_POS(1,1,14) && target_pos != gs_dng_pos_bak && random_schick(100) <= 10) {

		g_fig_escape_position[NORTH] = DNG_POS_DIR(1,1,12,NORTH);
		g_fig_escape_position[EAST] = g_fig_escape_position[SOUTH] = DNG_POS_DIR(1,5,14,EAST);
		g_fig_escape_position[WEST] = DNG_POS_DIR(1,1,14,NORTH);

		do_fight(FIGHT_ID_DTHO20);

	} else if (target_pos == DNG_POS(1,13,13) && target_pos != gs_dng_pos_bak) {

		/* a Ladder */

		if (GUI_bool(get_tx(13))) { /* "HIER BEFINDET SICH EIN LOCH IM BODEN, DAS IN EINEN SCHACHT MIT EINE..." */

			GUI_output(get_tx(14)); /* "NACH ETWA ZWEI DUTZEND SCHRITT HÖHE ERREICHT IHR EINE KLEINE HÖHLE,..." */

			gs_y++;
			gs_viewdir = SOUTH;
			DNG_update_pos();
		}

	} else if ((target_pos == DNG_POS(1,10,8) || target_pos == DNG_POS(1,11,10) || target_pos == DNG_POS(1,7,10) || target_pos == DNG_POS(1,9,12)) &&
		 target_pos != gs_dng_pos_bak && !gs_dng14_lvl2_fight) {

		/* the main fight in Level 2 */

		g_fig_escape_position[NORTH] = DNG_POS_DIR(1,10,6,NORTH);
		g_fig_escape_position[EAST] = DNG_POS_DIR(1,11,10,EAST);
		g_fig_escape_position[SOUTH] = g_fig_escape_position[WEST] = DNG_POS_DIR(1,9,14,SOUTH);

		if (!do_fight(random_schick(100) <= 50 ? FIGHT_ID_DTHO21A : FIGHT_ID_DTHO21B))
		{
			gs_dng14_lvl2_fight = 1;
		}

	} else if (target_pos == DNG_POS(1,8,9) && target_pos != gs_dng_pos_bak) {

		GUI_output(get_tx(15)); /* "ÜBER EINEM OFFENEN FEUER HÄNGT EIN GROSSER KESSEL MIT EINTOPF." */

	} else if (target_pos == DNG_POS(1,10,6) && target_pos != gs_dng_pos_bak && random_schick(100) <= 50) {

		g_fig_escape_position[NORTH] = DNG_POS_DIR(1,10,1,NORTH);
		g_fig_escape_position[EAST] = DNG_POS_DIR(1,12,6,EAST);
		g_fig_escape_position[SOUTH] = g_fig_escape_position[WEST] = DNG_POS_DIR(1,10,9,SOUTH);

		do_fight(FIGHT_ID_DTHO23);

	} else if ((target_pos == DNG_POS(1,8,2) || target_pos == DNG_POS(1,8,5)) && target_pos != gs_dng_pos_bak && gs_viewdir == WEST) {

		GUI_output(get_tx(16)); /* "HIER BEWAHREN DIE RÄUBER OFFENSICHTLICH IHR PLÜNDERGUT AUF. KISTEN..." */

	} else if (target_pos == DNG_POS(1,12,2) && target_pos != gs_dng_pos_bak && gs_viewdir == EAST) {

		GUI_output(get_tx(16)); /* "HIER BEWAHREN DIE RÄUBER OFFENSICHTLICH IHR PLÜNDERGUT AUF. KISTEN..." */

	} else if (target_pos == DNG_POS(1,12,6) && target_pos != gs_dng_pos_bak) {

		GUI_output(get_tx(16)); /* "HIER BEWAHREN DIE RÄUBER OFFENSICHTLICH IHR PLÜNDERGUT AUF. KISTEN..." */

#if !FIX_0355
		if (!gs_dng14_booty_flag) {
#else
		/* Fix 355: das Beutelager bleibt pluenderbar (der Rest liegt in gs_dng14_chest_barrel im Spielstand) */
		{
#endif

			loot_multi_chest(gs_dng14_chest_barrel, get_tx(17)); /* "NACHDEM IHR EUCH ETWAS UMGESCHAUT HABT, ENTDECKT IHR SCHLIESSLICH D..." */

			gs_dng14_booty_flag = 1;
		}

	} else if (target_pos == DNG_POS(1,12,10) && target_pos != gs_dng_pos_bak) {

		GUI_output(get_tx(18)); /* "DIESES GANGSTÜCK WIRD WOHL SEIT LANGER ZEIT NICHT MEHR BENUTZT." */

	} else if (target_pos == DNG_POS(1,13,10) && target_pos != gs_dng_pos_bak && !gs_dng14_ugdalf_done) {

		GUI_output(get_tx(19)); /* "DIE TÜR VOR EUCH IST MIT SCHWEREN BALKEN VERRAMELT, AUSSERDEM SIND..." */

		gs_dng14_ugdalf_done = 1;

		gs_quest_ugdalf = 3;

	} else if (target_pos == DNG_POS(1,1,9) &&
		(target_pos != gs_dng_pos_bak || gs_viewdir != gs_viewdir_bak) &&
		gs_viewdir == SOUTH && gs_dng14_secretdoor3 != 2) {

		if (gs_dng14_secretdoor3 != 0 || test_talent(hero, TA_SINNESSCHAERFE, 4) > 0) {

			/* secret door discovered (former comment: "acticate trap", copied from secret door 2) */
			gs_dng14_secretdoor3 = 1;

			sprintf(g_dtp2, get_tx(7), hero->alias); /* "BEI GENAUEM HINSEHEN, ENTDECKT %s EINE GEHEIMTÜRE IN DER WAND." */

			sprintf(g_text_output_buf,
				(char*)((tmp = test_talent(hero, TA_SCHLOESSER, -6)) > 0 ? get_tx(8) : get_tx(9)), /* "@NACH KURZER ZEIT SCHAFFT %s ES SOGAR, SIE..." / "@LEIDER SCHAFFT %s ES ALLERDINGS NICHT, SI..." */
				GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST));

			strcat(g_dtp2, g_text_output_buf);

			GUI_output(g_dtp2);

			if (tmp > 0) {
				/* test was successful => open the secret door (former comment: "disable trap") */
				amap_ptr[MAP_POS(1,10)] &= (DNG_TILE_CORRIDOR << 4) + 0x0f;
				gs_dng14_secretdoor3 = 2;

				DNG_update_pos();
			}

			gs_viewdir_bak = gs_viewdir;
		}

	} else if (target_pos == DNG_POS(1,7,10) &&
		(target_pos != gs_dng_pos_bak || gs_viewdir != gs_viewdir_bak) &&
		gs_viewdir == WEST && gs_dng14_secretdoor4 != 2) {

		if (gs_dng14_secretdoor4 != 0 || test_talent(hero, TA_SINNESSCHAERFE, 8) > 0) {

			/* secret door discovered (former comment: "acticate trap", copied from secret door 2) */
			gs_dng14_secretdoor4 = 1;

			sprintf(g_dtp2, get_tx(7), hero->alias); /* "BEI GENAUEM HINSEHEN, ENTDECKT %s EINE GEHEIMTÜRE IN DER WAND." */

			sprintf(g_text_output_buf,
				(char*)((tmp = test_talent(hero, TA_SCHLOESSER, 7)) > 0 ? get_tx(8) : get_tx(9)), /* "@NACH KURZER ZEIT SCHAFFT %s ES SOGAR, SIE..." / "@LEIDER SCHAFFT %s ES ALLERDINGS NICHT, SI..." */
				GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST));

			strcat(g_dtp2, g_text_output_buf);

			GUI_output(g_dtp2);

			if (tmp > 0) {
				/* test was successful => open the secret door (former comment: "disable trap") */
				amap_ptr[MAP_POS(6,10)] &= (DNG_TILE_CORRIDOR << 4) + 0x0f;
				gs_dng14_secretdoor4 = 2;

				DNG_update_pos();
			}

			gs_viewdir_bak = gs_viewdir;
		}

	} else if (target_pos == DNG_POS(2,3,6) && target_pos != gs_dng_pos_bak) {

		GUI_output(get_tx(20)); /* "HIER FÜHRT EIN SCHACHT IN BODENLOSE TIEFEN. VON UNTEN IST EIN SELTS..." */


	} else if (target_pos == DNG_POS(2,4,13) && target_pos != gs_dng_pos_bak) {

		if (GUI_bool(get_tx(21))) { /* "AM BODEN DER OSTWAND BEFINDET SICH EIN LOCH VON VIELLEICHT 4 SPANND..." */

			tmp = group_count_item(ITEM_ID_SEIL);
			tmp += group_count_item(ITEM_ID_STRICKLEITER);

			hero = get_hero(0);
			for (hero_pos = 0; hero_pos <= 6; hero_pos++, hero++) {

				if ((hero->typus != HERO_TYPE_NONE) &&
					(hero->group_id == gs_active_group_id) &&
					!hero->flags.dead &&
					(hero->typus == HERO_TYPE_MAGIER) &&
					(hero->staff_level > 2))
				{
					/* mage with staffspell-level > 2 => can transform staff to rope */
					tmp++;
				}
			}

			do {
				/* "climb roped together" is only offered with at least 5 ropes/rope ladders/staff mages */
			hero_pos = GUI_radio(get_tx(22), (tmp >= 5 ? 3 : 2), get_tx(23), get_tx(24), get_tx(25)); /* "UM ANGESEILT ZU KLETTERN BRÄU..." / "OHNE SEIL KLETTERN?" / "EUCH NACH SEIL UMSEHEN?" / "EUCH ANSEILEN?" */

			} while (hero_pos == -1);

			if (hero_pos == 3) {

				hero = get_hero(0);
				for (hero_pos = 0; hero_pos <= 6; hero_pos++, hero++) {

					if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
						!hero->flags.dead && (test_talent(hero, TA_KLETTERN, 0) <= 0))
					{
						sprintf(g_dtp2, get_tx(26), hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_4TH)); /* "%s GLEITET AUS UND STÜRZT SCHWER INS HALTESEIL. IHR KÖNNT %s JEDOCH..." */
						GUI_output(g_dtp2);

						/* 1W6 damage */
						sub_hero_le(hero, random_schick(6));
					}
				}

				/* drop down to level 4 */
				gs_y--;
				DNG_inc_level();

			} else if (hero_pos == 1) {

				hero = get_hero(0);
				for (hero_pos = 0; hero_pos <= 6; hero_pos++, hero++) {

					if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
						!hero->flags.dead && (test_talent(hero, TA_KLETTERN, 4) <= 0))
					{
						sprintf(g_dtp2, get_tx(27), hero->alias); /* "%s GLEITET AN EINER SCHLEIMBESCHMIERTEN WANDSTELLE AUS UND STÜRZT I..." */
						GUI_output(g_dtp2);

						/* 2W6+4 damage */
						sub_hero_le(hero, dice_roll(2, 6, 4));
					}
				}

				/* drop down to level 4 */
				gs_y--;
				DNG_inc_level();
			}
		}

	} else if (target_pos == DNG_POS(2,2,14) && target_pos != gs_dng_pos_bak && !gs_dng14_torches_flag) {

		if (GUI_bool(get_tx(28))) { /* "IN DIESER NISCHE LIEGEN 4 FACKELN.@WOLLT IHR SIE MITNEHMEN?" */

			gs_dng14_torches_flag = 1;

			give_new_item_to_group(ITEM_ID_FACKEL__UNLIT, 1, 4);
		}

	} else if (target_pos == DNG_POS(2,3,1) && target_pos != gs_dng_pos_bak && !gs_dng14_spooky_flag) {
		gs_dng14_spooky_flag = 1;

		GUI_output(get_tx(29)); /* "AUS DER LUFT VOR EUCH SCHÄLEN SICH GEISTERHAFTE SZENEN.@OFFENSICHTL..." */

		hero = get_hero(0);
		for (hero_pos = 0; hero_pos <= 6; hero_pos++, hero++) {

			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
				!hero->flags.dead && test_attrib(hero, ATTRIB_TA, 0) <= 0)
			{
				tmp = get_free_mod_slot();

				set_mod_slot(tmp, HOURS(6), (uint8_t*)&hero->attrib[ATTRIB_MU].current, -2, (signed char)hero_pos);
			}
		}

	} else if (target_pos == DNG_POS(2,1,2) && target_pos != gs_dng_pos_bak && !gs_dng14_spear_flag) {

		if (GUI_bool(get_tx(30))) { /* "IN DIESER NISCHE LIEGT EIN SPEER.@WOLLT IHR IHN MITNEHMEN?" */

			gs_dng14_spear_flag = 1;

			give_new_item_to_group(ITEM_ID_SPEER, 1, 1);
		}


	} else if (target_pos == DNG_POS(2,9,7) && target_pos != gs_dng_pos_bak) {

		if (GUI_bool(get_tx(31))) { /* "HIER IST DER GANG EINGESTÜRZT. IM LICHTSCHEIN EURER FACKELN KÖNNT I..." */

			timewarp(HOURS(12));

			GUI_output(get_tx(32)); /* "OHNE SCHWERES GERÄT IST HIER SCHEINBAR NICHTS ZU MACHEN. ES SIEHT A..." */
		}

		gs_x = gs_x_bak;
		gs_y = gs_y_bak;

	} else if (target_pos == DNG_POS(2,7,7) && target_pos != gs_dng_pos_bak) {

		GUI_dialog_na(60, get_tx(33)); /* "HIER LIEGT DAS SKELETT EINES THORWALERS, WIE MAN AN SEINER BEWAFFNU..." */

	} else if (target_pos == DNG_POS(2,13,7) && target_pos != gs_dng_pos_bak) {

		GUI_dialog_na(60, get_tx(34)); /* "HIER LIEGEN MEHRERE SKELETTE ÜBEREINANDER. ES SIEHT AUS, ALS HÄTTE..." */

	} else if (target_pos == DNG_POS(2,14,5) && target_pos != gs_dng_pos_bak) {

		GUI_output(get_tx(36)); /* "PLÖTZLICH SCHIESSEN UM EUCH HERUM FLAMMEN AUS DEM BODEN! IHR MACHT..." */

		/* each hero gets 2W6 damage */
		sub_group_le(dice_roll(2, 6, 0));

	} else if (target_pos == DNG_POS(2,14,3) && target_pos != gs_dng_pos_bak && gs_viewdir == NORTH) {

		GUI_output(get_tx(37)); /* "ES SCHEINT SICH BEI DIESEM RAUM UM EIN GEHEIMES LAGER ZU HANDELN. I..." */

	} else if (target_pos == DNG_POS(2,14,5) &&
			/* REMARK: entering (2,14,5) is caught by the fire trap above, so this branch only runs
			 * when the group turns north on that square */
			(target_pos != gs_dng_pos_bak || gs_viewdir != gs_viewdir_bak) &&
			gs_viewdir == NORTH &&
			gs_dng14_secretdoor5 != 2 &&
			(gs_dng14_secretdoor5 != 0 || test_talent(hero, TA_SINNESSCHAERFE, 6) > 0))
	{
			gs_dng14_secretdoor5 = 1;

			sprintf(g_dtp2,	get_tx(7), hero->alias); /* "BEI GENAUEM HINSEHEN, ENTDECKT %s EINE GEHEIMTÜRE IN DER WAND." */

			sprintf(g_text_output_buf,
				(char*)((tmp = test_talent(hero, TA_SCHLOESSER, 4)) > 0 ? get_tx(8) : get_tx(9)), /* "@NACH KURZER ZEIT SCHAFFT %s ES SOGAR, SIE..." / "@LEIDER SCHAFFT %s ES ALLERDINGS NICHT, SI..." */
				GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST));

			strcat(g_dtp2, g_text_output_buf);

			GUI_output(g_dtp2);

			if (tmp > 0) {
				/* open the secret door (former comment: "disable trap") */
				amap_ptr[MAP_POS(14,4)] &= (DNG_TILE_CORRIDOR << 4) + 0x0f;

				gs_dng14_secretdoor5 = 2;

				DNG_update_pos();
			}

			gs_viewdir_bak = gs_viewdir;
	}

	/* second if-chain (levels 4 and 5) */
	if (target_pos == DNG_POS(3,4,12) && target_pos != gs_dng_pos_bak) {

		GUI_output(get_tx(35)); /* "HIER FÜHRT EIN STEILER SCHACHT NACH OBEN, WEITER ALS DAS LICHT EURE..." */

	} else if (((target_pos == DNG_POS(3,10,10) && gs_viewdir == EAST) || (target_pos == DNG_POS(3,14,10) && gs_viewdir == WEST)) &&
			target_pos != gs_dng_pos_bak)
	{
		if (GUI_bool(get_tx(38))) { /* "AUF EINER LÄNGE VON MEHREREN SCHRITT IST HIER DIE DECKE EINGESTÜRZT..." */

			if (get_first_hero_with_item(ITEM_ID_SEIL) != - 1 || get_first_hero_with_item(ITEM_ID_STRICKLEITER) != -1) /* TODO: why not staffspell >= 3 ? */
			{

				if ((hero_pos = select_hero_ok(get_tx(39))) != -1) { /* "WER SOLL ZUERST TAUCHEN?" */

					hero = get_hero(hero_pos);

					if (test_talent(hero, TA_SCHWIMMEN, 8) <= 0) {

						sprintf(g_dtp2, get_tx(40), hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST)); /* "%s GEHT UNTERWEGS DIE LUFT AUS. %s MUSS UMKEHREN!" */
						GUI_output(g_dtp2);

						/* 1W6 damage */
						sub_hero_le(hero, random_schick(6));

						gs_x = (target_pos == DNG_POS(3,10,10) ? 9 : 13);

						if (hero->flags.dead) {

							sprintf(g_dtp2,	get_tx(63), hero->alias); /* "%s ERTRINKT BEI DEM VERSUCH, DEN KANAL ZU DURCHSCHWIMMEN." */
							GUI_output(g_dtp2);

							draw_status_line();
						}
					} else {
						sprintf(g_dtp2, get_tx(41), hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST)); /* "%s GELANGT HUSTEND UND SPUCKEND AUF DIE ANDERE SEITE. AN DEM SEIL,..." */
						GUI_output(g_dtp2);

						DNG14_dive(hero_pos, 2, (target_pos == DNG_POS(3,10,10) ? 13 : 9));
					}
				}
			} else {
				/* no tools for diving */
				DNG14_dive(-1, 8, (target_pos == DNG_POS(3,10,10) ? 13 : 9));
			}
		}

	} else if (target_pos == DNG_POS(3,6,11) && target_pos != gs_dng_pos_bak) {

		GUI_dialog_na(55, get_tx(44)); /* "IN DER NISCHE KÖNNT IHR DIE VERWESTE LEICHE EINES ZWERGES ERKENNEN...." */

	} else if ((target_pos == DNG_POS(3,4,1) || target_pos == DNG_POS(3,12,7) || target_pos == DNG_POS(3,1,3) || target_pos == DNG_POS(3,6,7)) && target_pos != gs_dng_pos_bak) {

		GUI_output(get_tx(45)); /* "HIER IST DER GANG EINGESTÜRZT. AN EIN WEITERKOMMEN IST NICHT ZU DEN..." */

	} else if (target_pos == DNG_POS(3,4,13) && target_pos != gs_dng_pos_bak) {

		GUI_output(get_tx(47)); /* "HIER IST DER GANG EINGESTÜRZT. AN EIN WEITERKOMMEN IST NICHT ZU DEN..." */

	} else if (target_pos == DNG_POS(3,3,3) && target_pos != gs_dng_pos_bak && !gs_dng14_hatchet_flag) {

		if (GUI_bool(get_tx(46))) { /* "IN DIESER NISCHE LIEGT EINE HACKE.@WOLLT IHR SIE MITNEHMEN?" */

			gs_dng14_hatchet_flag = 1;

			give_new_item_to_group(ITEM_ID_HACKE, 1, 1);
		}

	} else if (target_pos == DNG_POS(3,11,13) && target_pos != gs_dng_pos_bak && gs_viewdir == EAST) {

		GUI_output(get_tx(48)); /* "DER RAUM IST HÜFTTIEF MIT BRACKIGEM WASSER GEFÜLLT, VON DER DECKE U..." */

	} else if (target_pos == DNG_POS(3,12,12) && target_pos != gs_dng_pos_bak) {

		sprintf(g_dtp2, get_tx(49), hero->alias, hero->alias); /* "IRGENDETWAS SCHLEIMIGES GREIFT NACH %sS FUSS! IHR SCHLAGT UND STECH..." */
		GUI_output(g_dtp2);

		/* 3 LE damage (former comment: 1W6) */
		sub_hero_le(hero, 3);

	} else if (target_pos == DNG_POS(3,7,3) && target_pos != gs_dng_pos_bak && !gs_dng14_ring_flag) {

		if (GUI_bool(get_tx(50))) { /* "HIER LIEGT EIN RING AM BODEN.@WOLLT IHR IHN MITNEHMEN?" */

			gs_dng14_ring_flag = 1;

			give_new_item_to_group(ITEM_ID_RING__BLUE, 1, 1);
		}

	} else if (target_pos == DNG_POS(3,11,1) && target_pos != gs_dng_pos_bak) {

		if (GUI_bool(get_tx(51))) { /* "ÜBER EUCH KÖNNT IHR EINEN SPALT IN DER DECKE ERKENNEN UND AUCH EINE..." */

			GUI_output(get_tx(52)); /* "ES DAUERT ZWAR EINIGE ZEIT, ABER SCHLIESSLICH HABT IHR ES GESCHAFFT..." */

			if (!gs_dng14_cellarexit_flag) {

				GUI_output(get_tx(53)); /* "PLÖTZLICH HÖRT IHR GERÄUSCHE AN DER TÜR - UND NOCH EHE IHR REAGIERE..." */
				GUI_output(get_tx(54)); /* "<AHA! WEN HABEN WIR DENN HIER? GROTTENSCHRATE WOHL! GURTHAG, SOHN D..." */
				GUI_output(get_tx(55)); /* "IM LAUF DER NÄCHSTEN STUNDEN NEHMT IHR EINIGE ZWERGISCHE BIERE ZU E..." */

				drink_while_drinking(100);

				gs_dng14_cellarexit_flag = 1;
			}

			leave_dungeon();
			gs_x = 22;
			gs_y = 5;
			gs_town_loc_type = LOCTYPE_NONE;
			gs_viewdir = SOUTH;
		}

	} else if (target_pos == DNG_POS(4,5,13) && target_pos != gs_dng_pos_bak && !gs_dng14_orknase_flag) {

		if (GUI_bool(get_tx(57))) { /* "HIER LIEGT EINE ORKNASE AM BODEN.@WOLLT IHR SIE MITNEHMEN?" */

			gs_dng14_orknase_flag = 1;

			give_new_item_to_group(ITEM_ID_ORKNASE, 1, 1);
		}

	} else if (target_pos == DNG_POS(4,12,9) && target_pos != gs_dng_pos_bak && gs_viewdir == SOUTH) {

		GUI_output(get_tx(58)); /* "DIES SCHEINT EINER DER EHEMALIGEN VORRATSKELLER ZU SEIN. REGALE AN..." */

	} else if (target_pos == DNG_POS(4,5,9) && target_pos != gs_dng_pos_bak && gs_viewdir == NORTH) {

		if (!GUI_bool(get_tx(59))) { /* "HIER FÜHRT EIN GANG INS DUNKEL. AUS DEM RAUM VOR EUCH KÖNNT IHR DAS..." */

			gs_x = gs_x_bak;
			gs_y = gs_y_bak;

			GUI_output(get_tx(60)); /* "DIESE ENTSCHEIDUNG WAR VIELLEICHT AUCH BESSER." */
		}

	} else if (target_pos == DNG_POS(0,0,14) && target_pos != gs_dng_pos_bak) {
		/* regular exit */
		leave_dungeon();
		gs_x = 2;
		gs_y = 13;
		gs_town_loc_type = LOCTYPE_NONE;
		gs_viewdir = NORTH;
	}

	g_textbox_width = tw_bak;
	gs_dng_pos_bak = target_pos;

	return 0;
}
