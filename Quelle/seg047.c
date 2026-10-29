/*
 * seg047 - Helden und Gruppe: Hilfsfunktionen
 *
 * Inhalt:    Kleine Helfer rund um die Helden: Held mit dem besten CH oder KK
 *            finden, Krankheit/Vergiftung abfragen und zufügen, KK-Probe der
 *            ersten beiden Helden (Türen einschlagen), Geldbetrag als Text
 *            "D S H", AT/PA-Basiswert neu berechnen, Auswahlmenüs für Helden
 *            (Gruppe, handlungsfähig, erzwungen, Tempelliste), Rausch und
 *            Ausnüchtern.
 * Aufrufer:  sehr breit: Städte, Dungeons, Reise, Kampf, Statusmenü;
 *            check_heroes_KK aus seg076 (Türen), menu_enter_delete aus seg061
 *            (Tempel), hero_get_sober aus der Zeitfortschreibung (seg002).
 * Daten:     g_hero_sel_exclude: Held, der in der nächsten Auswahl fehlt
 *            (danach wieder -1); drei Nullvorlagen für lokale Indexfelder.
 * Besonders: check_hero_KK_unused wird nie aufgerufen. Original-Bug 46
 *            (hero_gets_diseased). Neu: select_hero_ok_forced stellt ohne
 *            wählbaren Helden die Textboxbreite nicht zurück und liefert 0;
 *            menu_enter_delete zeigt bei 20, 30, ... Helden eine leere Seite;
 *            make_valuta_str läuft ab 32768 Dukaten über.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg047.c
 */
#include <stdio.h>
#include <stdlib.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg007.h"
#include "seg047.h"
#include "seg097.h"

signed char g_hero_sel_exclude = -1; // ds:0x64a2
static struct int16_t_7 g_seg047_init1 = { 0, 0, 0, 0, 0, 0, 0 }; // ds:0x64a3, filled with zeroes
static struct int16_t_7 g_seg047_init2 = { 0, 0, 0, 0, 0, 0, 0 }; // ds:0x64b1, filled with zeroes
static struct int16_t_7 g_seg047_init3 = { 0, 0, 0, 0, 0, 0, 0 }; // ds:0x64bf, filled with zeroes

/**
 * \brief   get index of the first hero with the highest CH value
 *
 * \return index of the first hero with the highest current CH value.
 * The hero must be alive and in the current group.
 * Remark: the return value is undefined if no hero qualifies.
 */
signed int get_hero_CH_best(void)
{
	signed int retval;
	struct struct_hero *hero_i;
	signed int i;
	signed int ch_val = -1;

	hero_i = get_hero(0);

	for (i = 0; i <= 6; i++, hero_i++) {

		if ((hero_i->typus != HERO_TYPE_NONE) && (hero_i->group_id == gs_active_group_id) &&
			!hero_i->flags.dead && (hero_i->attrib[ATTRIB_CH].current > ch_val))
		{
			/* check if CH value is higher */

			ch_val = hero_i->attrib[ATTRIB_CH].current;
			retval = i;
		}
	}

	return retval;
}

/**
 * \brief   get the index of the first hero with the highest KK value
 *
 * \return index of the first hero with the highest current KK value.
 * The hero must be alive and in the current group.
 */
signed int get_hero_KK_best(void)
{
	signed int retval;
	struct struct_hero *hero_i;
	signed int i;
	signed int kk_val = -1;

	hero_i = get_hero(0);

	for (i = 0; i <= 6; i++, hero_i++) {

		if ((hero_i->typus != HERO_TYPE_NONE) && (hero_i->group_id == gs_active_group_id) &&
			!hero_i->flags.dead && (hero_i->attrib[ATTRIB_KK].current > kk_val))
		{
			/* check if KK is higher */

			kk_val = hero_i->attrib[ATTRIB_KK].current;
			retval = i;
		}
	}

	return retval;
}

/**
 * \brief   returns the disease ID the hero has
 *
 *          Only the first disease is returned here.
 *
 * \param   hero        the hero which should be checked
 * \return  ID of the first disease the hero has
 */
signed int hero_is_diseased(const struct struct_hero *hero)
{
	signed int disease_id;

	for (disease_id = 1; disease_id <= 7; disease_id++) {

		if (hero->disease[disease_id].status == DISEASE_STATUS_DISEASED) {

			return disease_id;
		}
	}

	return 0;
}

/**
 * \brief   returns the poison ID the hero has
 *
 *          Only the first poison is returned here.
 *
 * \param   hero        the hero which should be checked
 * \return              ID of the first poisoning the hero has
 */
signed int hero_is_poisoned(const struct struct_hero *hero)
{
	signed int poison_id;

	for (poison_id = 1; poison_id <= 9; poison_id++) {

		if (hero->poison[poison_id].status == POISON_STATUS_POISONED) {

			return poison_id;
		}
	}

	return 0;
}

/**
 * \brief   poison a hero
 *
 * \param   hero        the hero which gets poisoned
 * \param   poison_id   the ID of the poison
 */
void hero_gets_poisoned(struct struct_hero *hero, const signed int poison_id)
{
	if (!hero->flags.dead) {
		/* TODO: Original-Bug?: What if the hero is already poisoned? */
		/* Remark: probably the same problem as Original-Bug 46: the log bytes of the suffered effects are reset. */
#if FIX_0238
		/* Fix 238: die Protokollbytes der erlittenen Verluste nur bei einem gesunden Helden loeschen,
		 * sonst gibt das Ende der Vergiftung sie nie zurueck */
		if (hero->poison[poison_id].status == POISON_STATUS_HEALTHY) {
			hero->poison[poison_id].log_1 = 0;
			hero->poison[poison_id].log_2 = 0;
			hero->poison[poison_id].log_3 = 0;
		}
		hero->poison[poison_id].status = POISON_STATUS_POISONED;
		hero->poison[poison_id].time_counter = 0;
#else

		hero->poison[poison_id].status = POISON_STATUS_POISONED;
		hero->poison[poison_id].time_counter = 0;
		hero->poison[poison_id].log_1 = 0;
		hero->poison[poison_id].log_2 = 0;
		hero->poison[poison_id].log_3 = 0;
#endif
	}
}

/**
 * \brief   diseases a hero
 *
 * \param   hero        the hero which gets diseased
 * \param   disease_id  the ID of the disease
 */
void hero_gets_diseased(struct struct_hero *hero, const signed int disease_id)
{
#ifdef M302de_ORIGINAL_BUGFIX
	/* not a real BUG, but very useless */
	if (hero->typus == HERO_TYPE_NONE)
		return;
#endif

	/* Original-Bug 46: If the hero is already has the disease,
	 * the log bytes containing the info about the suffered negative effects are erased.
	 * Hence, these negative effects will be permanent. */
#ifdef M302de_ORIGINAL_BUGFIX
	if ((hero->disease[disease_id].status != DISEASE_STATUS_HEALTHY)
		/* A hero cannot get the same disease twice. In the recovering phase, the hero is immune. */
		|| (disease_id == DISEASE_ID_DUMPFSCHAEDEL && (hero->disease[DISEASE_ID_BLAUE_KEUCHE].status != DISEASE_STATUS_HEALTHY)))
		/* Blaue Keuche is a worsening of DumpfschÃ¤del.
		 * So if a hero already suffers from Blaue Keuche (or is recovering from it),
		 * he is immune to the (weaker) disease DumpfschÃ¤del, too. */
	{
		return;
	}
#endif

	if (!hero->flags.dead) {
#ifdef M302de_ORIGINAL_BUGFIX
		if (disease_id == DISEASE_ID_BLAUE_KEUCHE) {
		/* Blaue Keuche is a worsening of DumpfschÃ¤del.
		 * Hence, the effect of DumpfschÃ¤del is removed. (But blaue Keuche will be worse...) */
			hero->disease[DISEASE_ID_DUMPFSCHAEDEL].time_counter = 0;
			hero->disease[DISEASE_ID_DUMPFSCHAEDEL].status = DISEASE_STATUS_RECOVER;
		}
#endif

#if !defined(__BORLANDC__)
		D1_INFO("%s erkrankt an %s\n", hero->alias, get_ttx(disease_id + 0x193)); /* TEXT.LTX 404..410: Krankheit */
#endif

#if FIX_0240
		/* Fix 240: die Protokollbytes der erlittenen Verluste nur bei einem gesunden Helden loeschen,
		 * sonst gibt die Genesung sie nie zurueck (Verluste wuerden dauerhaft) */
		if (hero->disease[disease_id].status == DISEASE_STATUS_HEALTHY) {
			hero->disease[disease_id].log_1 = 0;
			hero->disease[disease_id].log_2 = 0;
			hero->disease[disease_id].log_3 = 0;
		}
		hero->disease[disease_id].status = DISEASE_STATUS_DISEASED;
		hero->disease[disease_id].time_counter = 0;
#else
		hero->disease[disease_id].status = DISEASE_STATUS_DISEASED;
		hero->disease[disease_id].time_counter = 0;
		hero->disease[disease_id].log_1 = 0;
		hero->disease[disease_id].log_2 = 0;
		hero->disease[disease_id].log_3 = 0;
#endif
	}
}

/**
 * \brief   the hero may get a disease
 *
 * \param   hero        the hero which may gets diseased
 * \param   disease_id  the ID of the disease
 * \param   probability the probability to get diseased in percent
 */
void hero_disease_test(struct struct_hero *hero, const signed int disease_id, const signed int probability)
{
#ifdef M302de_ORIGINAL_BUGFIX
	/* not a real BUG, but very useless */
	if (hero->typus == HERO_TYPE_NONE) {
		D1_ERR("WARNING: called %s with an invalid hero\n", __func__);
		return;
	}
#endif

	/* check the probability and if hero is diseased*/
	if ((random_schick(100) <= probability) && (hero->disease[disease_id].status != DISEASE_STATUS_DISEASED)) {

		hero_gets_diseased(hero, disease_id);
	}
}

/**
 *	check_hero_KK_unused
 *
 *	This function is not used in the game! (checked: no caller)
 *	Returns 1 if KK (current + mod) of the hero in slot 0 is >= val.
 */
signed int check_hero_KK_unused(const signed int val)
{
	return get_hero(0)->attrib[ATTRIB_KK].current + get_hero(0)->attrib[ATTRIB_KK].mod >= val ? 1 : 0;
}

/**
 * \brief check_heroes_KK
 *
 *          Sums up KK (current + mod) of the heroes in slots 0 and 1 and
 *          compares with val. Used for smashing doors (seg076).
 *
 * \param   val         value to compare KK with
 * \return  1 if the sum is >= val, else 0
 *
 *	This function, like check_hero_KK_unused, is buggy!
 *	It does not check if the first slot is a valid hero.
 */
signed int check_heroes_KK(const signed int val)
{
	struct struct_hero *hero;
	signed int sum;

#if FIX_0081
	/* Fix 81: die ersten beiden handlungsfaehigen Helden der aktiven Gruppe */
	hero = get_first_hero_available_in_group();

	sum = hero->attrib[ATTRIB_KK].current + hero->attrib[ATTRIB_KK].mod;

	if ((hero = get_second_hero_available_in_group()) != NULL) {
		sum += hero->attrib[ATTRIB_KK].current + hero->attrib[ATTRIB_KK].mod;
	}
#else
	hero = get_hero(0);

	/* Orig-BUG: not checked if hero is valid */
	sum = hero->attrib[ATTRIB_KK].current + hero->attrib[ATTRIB_KK].mod;

	hero = get_hero(1);

	/* check class, group and dead status of hero in slot 2*/
	if (hero->typus && (hero->group_id == gs_active_group_id) && !hero->flags.dead) {
		sum += hero->attrib[ATTRIB_KK].current + hero->attrib[ATTRIB_KK].mod;
	}
#endif

#if !defined(__BORLANDC__)
	D1_INFO("Pruefe KK der ersten beiden Helden (%d) >= %d: ", sum, val);
	D1_INFO("%s\n", sum >= val ? "gelungen" : "mislungen");
#endif

	return (sum >= val) ? 1 : 0;
}

/**
 * \brief   makes a valuta string
 *
 * \param   dst         the destination
 * \param   money       the money in Heller
 *
 *	This function is buggy (see the overflow below).
 */
void make_valuta_str(char *dst, int32_t money) {

	/* Orig-BUG: d is a signed int and overflows for more than 32767 D (the old comment said 65536) */
	signed int d = 0;
	signed int s = 0;
#if FIX_0296
	int32_t money_d = money / 100;
#endif

	/*	These loops are not very performant.
		They take longer the more money you have.
		Here is a much better solution.
	*/

	/*
	d = money / 100;
	money -= d * 100;

	s = money / 10;
	money -= s * 10;
	*/
	while (money / 100) {
		d++;
		money -= 100;
	}

	while (money / 10) {
		s++;
		money -= 10;
	}

#if FIX_0296
	/* Fix 296: ab 32768 Dukaten die Dukaten als long ausgeben (Format "%dD %dS %dH": ab Zeichen 2 folgt "D ...") */
	if ((money_d > 32767L) && (get_ttx(748)[0] == '%') && (get_ttx(748)[1] == 'd')) {
		ltoa(money_d, dst, 10);
		sprintf(dst + strlen(dst), get_ttx(748) + 2, s, (signed int)money);
	} else
#endif
	sprintf(dst, get_ttx(748), d, s, (signed int)money); /* "%dD %dS %dH" */
}

/**
 * \brief   recalculates the AT PA values
 *
 *          Computes the AT/PA base value from the normal values of IN, KK and GE.
 *          On a change, a message is shown and the difference is added to the
 *          AT and PA values of all 7 melee weapon types.
 *
 * \param   hero        the hero
 */
void update_atpa(struct struct_hero *hero)
{
	div_t erg;
	signed int diff;
	signed int i;

	/* ATPA base = (IN + KK + GE) / 5 rounded */
	erg = div(hero->attrib[ATTRIB_IN].normal + hero->attrib[ATTRIB_KK].normal + hero->attrib[ATTRIB_GE].normal, 5);
	/* Original-Bug:
	 * According to DSA 3 rules, AT basis value is (MU + KK + GE) / 5
	 * (PA basis (IN + KK + GE) / 5 is correct
	 * Remark: DSA1 knows only a single AT/PA base value, so this is rather a deviation
	 * from the DSA 3 rules than a programming error. */

	/* round up */
	if (erg.rem >= 3) {
		erg.quot++;
	}

	/* calculate difference */
	diff = erg.quot - hero->atpa_base;

	if (diff != 0) {

		/* update atpa base value */
		hero->atpa_base = erg.quot;

		/* prepare message */
		sprintf(g_dtp2,	get_ttx(8), hero->atpa_base); /* "DER AT/PA BASISWERT HAT SICH AUF %d VERÄNDERT!" */

		/* print message */
		GUI_output(g_dtp2);

		for (i = 0; i < 7; i++) {
			/* add diff to AT value */
			hero->at_talent_bonus[i] = hero->at_talent_bonus[i] + diff;
			/* add diff to PA value */
			hero->pa_talent_bonus[i] = hero->pa_talent_bonus[i] + diff;
		}
	}
}

/**
 * \brief   selects a hero to enter / get deleted
 *
 * \param   ptr         list of the heroes in the temple: 32-byte entries, the name starts at offset 16
 * \param   entries     how many heroes
 * \param   mode        1 = enter / -1 = delete
 * \return              the number of the selected hero (0-based list index) or -1 (ESC).
 *                      Pages of 10 heroes, the entry "MEHR HELDEN" switches to the next page.
 * Used only in temples (seg061).
 */
signed int menu_enter_delete(char* ptr, const signed int entries, const signed int mode)
{
	signed int i;
	signed int answer = 0;
	signed int i_max;
	signed int i_min = 0;

	while (answer != -1) {

		i_max = (i_min + 10 < entries) ? 10 : entries - i_min;

		/* fill a pointer array with the pointer to the names */
		for (i = 0; i < i_max; i++) {
			g_radio_name_list[i] = ((i + i_min) * 32 + ptr + 16);
		}

		i = i_max;
		if (entries > 10) {
			g_radio_name_list[i] = get_ttx(291); /* "MEHR HELDEN" */
			i++;
		}

		answer = GUI_radio( (mode == -1) ? get_ttx(567) : get_ttx(292), (signed char)i, /* "WELCHEN HELD LÖSCHEN?" / "WELCHEN HELD AUFNEHMEN?" */
				g_radio_name_list[0], g_radio_name_list[1],
				g_radio_name_list[2], g_radio_name_list[3],
				g_radio_name_list[4], g_radio_name_list[5],
				g_radio_name_list[6], g_radio_name_list[7],
				g_radio_name_list[8], g_radio_name_list[9],
				g_radio_name_list[10], g_radio_name_list[11]);

		if ((entries > 10) && (answer == i)) {
			i_min += 10;

			/* Original-Bug: should be '>=': with 20, 30, ... heroes the last page
			 * contains no hero, only "MEHR HELDEN". */
#if FIX_0046
			/* Fix 46: auch bei genau 20, 30, ... Helden zurueck zur ersten Seite */
			if (i_min >= entries) {
#else
			if (i_min > entries) {
#endif
				i_min = 0;
			}

		} else {

			if (answer != -1) {
				return i_min + answer - 1;
			}
		}
	}

	return -1;
}

/**
 * \brief   menu for selecting a hero
 *
 * \param   title       titlestring for the menu
 * \return              index of the hero or -1 (ESC).
 * Remark: The available heroes must be in the group only.
 */
signed int select_hero_from_group(char *title)
{
	signed int i;
	signed int answer;
	struct int16_t_7 dst = g_seg047_init1;
	/* Hack for
	signed int dst[7] = {0, 0, 0, 0, 0, 0, 0};
	*/

	signed int cnt;
	signed int tw_bak;
	signed int basepos_x_bak;
	signed int basepos_y_bak;
	struct struct_hero *hero;

	tw_bak = g_textbox_width;
	g_textbox_width = 3;
	cnt = 0;

	for (i = 0; i <= 6; i++) {

		hero = get_hero(i);

		if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && (g_hero_sel_exclude != i)) {
			/* g_hero_sel_exclude: hero who must not be offered (e.g. the healer himself
			 * in seg103, the NPC in GRP_split). It is reset to -1 below. */

			/* save pointer to the name of the hero */
			g_radio_name_list[cnt] = hero->alias;
			dst.a[cnt] = i;
			cnt++;
		}
	}


	g_hero_sel_exclude = -1;

	if (cnt != 0) {

		basepos_x_bak = g_basepos_x;
		basepos_y_bak = g_basepos_y;

		g_basepos_x = g_basepos_y = 0;

		answer = GUI_radio(title, (signed char)cnt,
				g_radio_name_list[0], g_radio_name_list[1],
				g_radio_name_list[2], g_radio_name_list[3],
				g_radio_name_list[4], g_radio_name_list[5],
				g_radio_name_list[6]) - 1;

		g_basepos_x = basepos_x_bak;
		g_basepos_y = basepos_y_bak;
		g_textbox_width = tw_bak;

		if (answer != -2)
			return dst.a[answer];
		else
			return -1;
	}

	g_textbox_width = tw_bak;

	return -1;
}

/**
 * \brief   menu for selecting a hero
 *
 * \param   title       titlestring for the menu
 * \return              index of the hero or -1 (ESC).
 * Remark: The available heroes must be in the group and pass check_hero().
 */
signed int select_hero_ok(char *title)
{
	signed int i;
	signed int answer;
	struct int16_t_7 dst = g_seg047_init2;
	/* Hack for
	signed int dst[7] = {0, 0, 0, 0, 0, 0, 0};
	*/

	signed int cnt;
	signed int tw_bak;
	signed int basepos_x_bak;
	signed int basepos_y_bak;
	struct struct_hero *hero;

	tw_bak = g_textbox_width;
	g_textbox_width = 3;
	cnt = 0;

	for (hero = get_hero(0), i = 0; i <= 6; i++, hero++) {

		if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && check_hero(hero) && (g_hero_sel_exclude != i)) {

			/* save pointer to the name of the hero */
			g_radio_name_list[cnt] = hero->alias;
			dst.a[cnt] = i;
			cnt++;
		}
	}

	g_hero_sel_exclude = -1;

	if (cnt != 0) {
		basepos_x_bak = g_basepos_x;
		basepos_y_bak = g_basepos_y;

		g_basepos_x = g_basepos_y = 0;

		answer = GUI_radio(title, (signed char)cnt,
				g_radio_name_list[0], g_radio_name_list[1],
				g_radio_name_list[2], g_radio_name_list[3],
				g_radio_name_list[4], g_radio_name_list[5],
				g_radio_name_list[6]) - 1;

		g_basepos_x = basepos_x_bak;
		g_basepos_y = basepos_y_bak;
		g_textbox_width = tw_bak;
		g_talented_hero_pos = -1;

		if (answer != -2)
			return dst.a[answer];
		else
			return -1;
	}

	g_textbox_width = tw_bak;
	g_talented_hero_pos = -1;

	return -1;
}

/**
 * \brief   menu for selecting a hero
 *
 * \param   title       titlestring for the menu
 * \return              index of the hero or -1 (ESC).
 * Remark: The available heroes must be in the group, pass check_hero() and
 *		you are forced to select a hero.
 */
signed int select_hero_ok_forced(char *title)
{
	signed int i;
	signed int answer;
	struct int16_t_7 dst = g_seg047_init3;
	/* Hack for
	signed int dst[7] = {0, 0, 0, 0, 0, 0, 0};
	*/

	signed int cnt;
	signed int tw_bak;
	signed int basepos_x_bak;
	signed int basepos_y_bak;
	struct struct_hero *hero;

	tw_bak = g_textbox_width;
	g_textbox_width = 3;
	cnt = 0;

	for (hero = get_hero(0), i = 0; i <= 6; i++, hero++) {

		if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && check_hero(hero) && (g_hero_sel_exclude != i)) {

			/* save pointer to the name of the hero */
			g_radio_name_list[cnt] = hero->alias;
			dst.a[cnt] = i;
			cnt++;
		}
	}

	g_hero_sel_exclude = -1;

	if (cnt != 0) {
		do {
			basepos_x_bak = g_basepos_x;
			basepos_y_bak = g_basepos_y;

			g_basepos_x = g_basepos_y = 0;

			answer = GUI_radio(title, (signed char)cnt,
					g_radio_name_list[0], g_radio_name_list[1],
					g_radio_name_list[2], g_radio_name_list[3],
					g_radio_name_list[4], g_radio_name_list[5],
					g_radio_name_list[6]) - 1;

			g_basepos_x = basepos_x_bak;
			g_basepos_y = basepos_y_bak;

		} while (answer == -2);

		g_talented_hero_pos = -1;
		g_textbox_width = tw_bak;

		return dst.a[answer];
	} else {

		/* Original-Bug: g_textbox_width is not restored (stays 3), and 0 (the hero
		 * in the first slot) is returned instead of -1. Callers use the result
		 * unchecked with get_hero(). */
		g_talented_hero_pos = -1;
#if FIX_0242
		/* Fix 242 (teilweise): Textbreite zuruecksetzen; die Rueckgabe bleibt 0 (die Aufrufer pruefen nicht auf -1) */
		g_textbox_width = tw_bak;
#endif
		return 0;
	}
}

/**
 * \brief   counts the heroes in the current group
 *
 * \return  number of living heroes in the current group
 */
signed int count_heroes_in_group(void)
{
	struct struct_hero *hero_i;
	signed int i;
	signed int retval = 0;

	for (hero_i = get_hero(0), i = 0; i <= 6; i++, hero_i++) {

		/* Check class, group and dead */
		if ((hero_i->typus != HERO_TYPE_NONE) && (hero_i->group_id == gs_active_group_id) && !hero_i->flags.dead) {

			retval++;
		}
	}

	return retval;
}

/**
 * \brief   let the hero feel the result of too much alcohol
 *
 * \param   hero        the hero
 */
void hero_get_drunken(struct struct_hero *hero)
{
	if (!hero->drunk) {

		/* set the hero drunken */
		hero->drunk = 1;

		/* change good attributes */
		hero->attrib[ATTRIB_MU].current += 1;
		hero->attrib[ATTRIB_KL].current -= 1;
		hero->attrib[ATTRIB_CH].current -= 1;
		hero->attrib[ATTRIB_FF].current -= 1;
		hero->attrib[ATTRIB_GE].current -= 1;
		hero->attrib[ATTRIB_IN].current += 1;
		hero->attrib[ATTRIB_KK].current += 1;

		/* change bad attributes */
		hero->attrib[ATTRIB_AG].current += 1;
		hero->attrib[ATTRIB_HA].current -= 1;
		hero->attrib[ATTRIB_RA].current -= 1;
		hero->attrib[ATTRIB_GG].current += 1;
		hero->attrib[ATTRIB_TA].current -= 1;
		hero->attrib[ATTRIB_NG].current += 1;
		hero->attrib[ATTRIB_JZ].current += 1;

		/* refresh the status screen if it is shown, then burp (FX2.VOC) */
		if (g_pp20_index == ARCHIVE_FILE_ZUSTA_UK) {
			g_request_refresh = 1;
		}

		play_voc_delay(ARCHIVE_FILE_FX2_VOC);
	}
}

/**
 * \brief   makes a drunken hero sober
 *
 * \param   hero        pointer to the hero
 */
void hero_get_sober(struct struct_hero *hero)
{
	/* This is checked twice: the caller (seg002, at 10 o'clock) checks hero->drunk, too */
	/* Is hero drunken ? */
	if (hero->drunk == 0)
		return;

#if !defined(__BORLANDC__)
	D1_INFO("%s ist wieder nuechtern\n", hero->alias);
#endif

	/* set hero sober */
	hero->drunk = 0;

	/* Reset good attributes */
	hero->attrib[ATTRIB_MU].current -= 1;
	hero->attrib[ATTRIB_KL].current += 1;
	hero->attrib[ATTRIB_CH].current += 1;
	hero->attrib[ATTRIB_FF].current += 1;
	hero->attrib[ATTRIB_GE].current += 1;
	hero->attrib[ATTRIB_IN].current -= 1;
	hero->attrib[ATTRIB_KK].current -= 1;

	/* Reset bad attributes */
	hero->attrib[ATTRIB_AG].current -= 1;
	hero->attrib[ATTRIB_HA].current += 1;
	hero->attrib[ATTRIB_RA].current += 1;
	hero->attrib[ATTRIB_GG].current -= 1;
	hero->attrib[ATTRIB_TA].current += 1;
	hero->attrib[ATTRIB_NG].current -= 1;
	hero->attrib[ATTRIB_JZ].current -= 1;

	if (g_pp20_index == ARCHIVE_FILE_ZUSTA_UK)
		g_request_refresh = 1;
}
