/*
 * seg035 - Kampf: Aufraeumen, Beute, AP, Schusslinie
 *
 * Inhalt:    Nach dem Kampf entfernt FIG_tidy_enemies() ausgeschaltete Gegner aus der
 *            Kampfliste, FIG_loot_enemies() bietet die Beute zur Auswahl an (Seiten zu je
 *            15 Eintraegen mit "MEHR") und verteilt das Geld, FIG_split_ap() verteilt die
 *            Abenteuerpunkte gleichmaessig (Erstbegegnung mit einer Gegnerart zaehlt voll,
 *            sonst ein Zehntel). check_hero_range_attack() prueft, ob ein gespeichertes
 *            Fernkampf- oder Zauberziel noch in gerader, freier Schusslinie liegt.
 * Aufrufer:  do_fight() (seg032), FIG_menu() (seg033, "ALTE OPTIONEN").
 * Besonders: FIG_loot_enemies(): bei genau 30 Beutestuecken wird auf Seite 2 in
 *            loot_names[30] geschrieben, einen nicht gesetzten Zeiger. Der Knochen mit Rune
 *            beendet die Beuteliste (Behelfsloesung fuer THOR8). check_hero_range_attack()
 *            kann die Meldung zweimal zeigen.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg035.c
 */
#include <stdio.h>
#include <string.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg032.h"
#include "seg034.h"
#include "seg038.h"
#include "seg039.h"
#include "seg047.h"
#include "seg096.h"
#include "seg097.h"
#include "seg105.h"

/**
 * \brief   compress enemies
 *
 *          removes enemies which are dead, petrified or a mushroom (and enemies which have
 *          not appeared yet, if no active enemy is left) from g_current_fight->enemies and
 *          g_enemy_sheets by moving the following entries to the front.
 */
void FIG_tidy_enemies(void)
{
	signed int i;
	signed int j;
	signed int enemies_num = FIG_count_active_enemies();

	i = 0;
	while (i < 20) {

		/* if the enemy is not able to fight anymore ... */
		if (g_enemy_sheets[i].monster_id &&
			(g_enemy_sheets[i].flags.dead || g_enemy_sheets[i].flags.mushroom || g_enemy_sheets[i].flags.petrified ||
			(g_current_fight->enemies[i].round_appear && (enemies_num == 0))))
		{
			if (i == 19) {
				/* just clear the last one */
				memset(&g_current_fight->enemies[i], 0, sizeof(struct fight_enemy));
				break;
			} else {
				/* move the next enemies one position to the front */
				for (j = i; j < 19; j++) {

					g_current_fight->enemies[j] = g_current_fight->enemies[j + 1];

					memset(&g_current_fight->enemies[j + 1], 0, sizeof(struct fight_enemy));

					g_enemy_sheets[j] = g_enemy_sheets[j + 1];

					memset(&g_enemy_sheets[j + 1], 0, sizeof(struct enemy_sheet));

					g_enemy_sheets[j + 1].flags.dead = 1;
				}
			}

		} else {
			i++;
		}
	}
}

/**
 * \brief   lets the group pick items from the loot of the fight and gives them the money
 */
void FIG_loot_enemies(void)
{
	signed int page_offset = 0;
	signed int loot_num;
	signed int item_id;
	signed int item_cnt;
	signed int selected;
	signed int more_entry_pos;
	signed int radio_count;
	int32_t money;
	const signed int autofight_bak = g_autofight;
	signed int tw_bak;

	char *loot_names[31];

	g_autofight = 0;

#if FIX_0015
	/* Fix 15: auch loot_names[30] belegen, dorthin schreibt "MEHR" auf Seite 2 bei genau 30 Stueck */
	for (loot_num = 0; loot_num < 31; loot_num++) {
#else
	for (loot_num = 0; loot_num < 30; loot_num++) {
#endif

		loot_names[loot_num] = (char*)(g_buffer9_ptr + 40 * loot_num);
	}

#if FIX_0016
	/* Fix 16: den Knochen mit Rune aus der Beuteliste nehmen, statt die Liste an ihm abzubrechen
	 * (die Gegenstaende dahinter werden sonst nie angeboten) */
	for (loot_num = 0; loot_num < 30; loot_num++) {

		while (g_current_fight->loot[loot_num] == ITEM_ID_KNOCHEN_MIT_RUNE) {

			for (item_cnt = loot_num; item_cnt < 29; item_cnt++) {
				g_current_fight->loot[item_cnt] = g_current_fight->loot[item_cnt + 1];
			}
			g_current_fight->loot[29] = 0;
		}
	}
#endif

	do {
		loot_num = item_cnt = 0;

		/* REMARK: loot[loot_num] is read before loot_num < 30 is checked (loot[30] is the ducats field).
		 * With exactly 30 items the second page writes "MEHR" to loot_names[30], which has not
		 * been initialized (only 0..29 are set above). Original-Bug? */

		while ((item_id = g_current_fight->loot[loot_num]) && (loot_num < 30) && (item_id != ITEM_ID_KNOCHEN_MIT_RUNE))
			/* Apparently a quick "fix" for an unwanted bone with runes in fight THOR8,
			 * see https://www.crystals-dsa-foren.de/showthread.php?tid=453&pid=172221#pid172221 */
		{
			strcpy(loot_names[loot_num++],
				GUI_name_inflect(INFLECT_GENDER_UNSPECIFIED | INFLECT_SINGULAR | INFLECT_1ST_CASE, g_itemsname[item_id])
			);
			item_cnt++;
		}

		if (item_cnt != 0) {

			if (item_cnt > 15) {

				if (page_offset == 0) {
					more_entry_pos = 15;
					radio_count = 16;
				} else {
					more_entry_pos = item_cnt;
					radio_count = item_cnt - 14;
				}

				strcpy(loot_names[more_entry_pos], get_ttx(751)); /* "MEHR" */

			} else {
				page_offset = 0;
				more_entry_pos = 0;
				radio_count = item_cnt;
			}

			tw_bak = g_textbox_width;
			g_textbox_width = 6;

			selected = GUI_radio(get_tx(14), radio_count, /* "WELCHE DER ERBEUTETEN GEGENSTÄNDE WOLLT IHR AN EUCH NEHMEN?" */
				loot_names[0 + page_offset], loot_names[1 + page_offset],
				loot_names[2 + page_offset], loot_names[3 + page_offset],
				loot_names[4 + page_offset], loot_names[5 + page_offset],
				loot_names[6 + page_offset], loot_names[7 + page_offset],
				loot_names[8 + page_offset], loot_names[9 + page_offset],
				loot_names[10 + page_offset], loot_names[11 + page_offset],
				loot_names[12 + page_offset], loot_names[13 + page_offset],
				loot_names[14 + page_offset], loot_names[15 + page_offset]) - 1;

			g_textbox_width = tw_bak;

			if ((more_entry_pos != 0) && (radio_count - 1 == selected)) {
				if (!page_offset) {
					page_offset = 15;
				} else {
					page_offset = 0;
				}
			}

			if ((selected != -2) && ((more_entry_pos == 0) || ((more_entry_pos != 0) && (radio_count - 1 != selected)))) {

				if (!give_new_item_to_group(g_current_fight->loot[selected + page_offset], 1, 1))
				{
					selected = -2;

				} else {
					g_current_fight->loot[selected + page_offset] = 0;

					for (loot_num = selected + page_offset; loot_num < 29; loot_num++) {

						g_current_fight->loot[loot_num] = g_current_fight->loot[loot_num + 1];
						g_current_fight->loot[loot_num + 1] = 0;
					}
				}
			}

		} else {
			selected = -2;
		}

	} while (selected != -2);

	money = g_current_fight->ducats * 100;
	money += g_current_fight->silver * 10;
	money += g_current_fight->heller;

	if (money > 0) {

		make_valuta_str(g_text_output_buf, money);

		sprintf((char*)g_dtp2, get_tx(15), g_text_output_buf); /* "IHR ERBEUTET AUSSERDEM %s" */
		GUI_output(g_dtp2);

		set_party_money(get_party_money() + money);
	}

	g_autofight = autofight_bak;
}

/**
 * \brief   give the group the AP after a fight
 */
void FIG_split_ap(void)
{
	signed int i;
	signed int ap = 0;
	signed int known_ap;
	const signed int autofight_bak = g_autofight;

	g_autofight = 0;

	/* calculate ap for that fight */
	for (i = 0; i < 20; i++) {

		if (g_enemy_sheets[i].monster_id) {

			if (gs_known_monsters[g_enemy_sheets[i].monster_id]) {

				/* monster type is already known */
				known_ap = g_enemy_sheets[i].first_ap / 10;
				ap += (known_ap == 0 ? 1 : known_ap);
			} else {
				/* first time bonus */
				ap += g_enemy_sheets[i].first_ap;
			}
		}
	}

	/* mark each monster type from that fight in the game state */
	for (i = 0; i < 20; i++) {

		if (g_enemy_sheets[i].monster_id) {

			gs_known_monsters[g_enemy_sheets[i].monster_id] = 1;
		}
	}

	if (count_heroes_available_in_group() > ap) {
		/* every hero gets at least 1 AP */

		ap = count_heroes_available_in_group();

	} else {
		/* every hero should get the same AP */
		while ((ap % count_heroes_available_in_group())) {
			ap++;
		}
	}

	/* prepare output */
	sprintf(g_dtp2, get_tx(33), ap); /* "DIE GRUPPE ERHÄLT %d ABENTEUERPUNKTE." */
	GUI_output(g_dtp2);

	/* give AP to the group */
	add_group_ap(ap);

	g_autofight = autofight_bak;
}

/**
 * \brief   test if a range attack is possible
 *
 * \param   hero        pointer to the hero
 * \param   hero_pos    position of the hero
 * \return              1 = range attack is possible, 0 = ... not possible
 */
signed int check_hero_range_attack(const struct struct_hero *hero, const signed int hero_pos)
{
	signed int i;
	signed int retval;	/* retval */
	signed int target_x;
	signed int target_y;
	signed int hero_x;
	signed int hero_y;

	retval = 1;

	/* get position of the target */
	FIG_search_obj_on_cb(hero->target_object_id, &target_x, &target_y);

	/* get position of the hero */
	FIG_search_obj_on_cb(hero_pos + 1, &hero_x, &hero_y);

	/* check that the range attack is in the same line.
	 * REMARK: if not, the check of the line below runs anyway and may show the message a second time */
	if ((hero_x != target_x) && (hero_y != target_y)) {
		GUI_output(get_tx(45)); /* "ZIEL IST NICHT MEHR IN GÜLTIGER SCHUSSPOSITION!" */
		retval = 0;
	}

#if FIX_0017
	/* Fix 17: Schusslinie nur pruefen, wenn die Meldung nicht schon kam */
	if (retval && (manhattan_distance(hero_x, hero_y, target_x, target_y) != 1)) {
#else
	if (manhattan_distance(hero_x, hero_y, target_x, target_y) != 1) {
#endif

		/* exchange coordinates if needed */
		if (hero_x > target_x) {
			i = hero_x;
			hero_x = target_x;
			target_x = i;
		}

		/* exchange coordinates if needed */
		if (hero_y > target_y) {
			i = hero_y;
			hero_y = target_y;
			target_y = i;
		}

		if (hero_x != target_x) {

			for (i = hero_x; i < target_x; i++) {

				if (!FIG_check_target_cursor_step(hero_x, hero_y, i, hero_y, 1, 0, 99)) {
					GUI_output(get_tx(45)); /* "ZIEL IST NICHT MEHR IN GÜLTIGER SCHUSSPOSITION!" */
					retval = 0;
					break;
				}
			}
		} else {
			for (i = hero_y; i < target_y; i++) {

				if (!FIG_check_target_cursor_step(hero_x, hero_y, hero_x, i, 0, 1, 99)) {
					GUI_output(get_tx(45)); /* "ZIEL IST NICHT MEHR IN GÜLTIGER SCHUSSPOSITION!" */
					retval = 0;
					break;
				}
			}
		}
	} else {

		if (hero->action_id == FIG_ACTION_RANGE_ATTACK) {
			GUI_output(get_ttx(508)); /* "ZU NAH!" */
			retval = 0;
		}
	}

	return retval;
}
