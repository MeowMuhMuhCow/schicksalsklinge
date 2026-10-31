/*
 * seg069 - Thorwal: Sondergebaeude (Teil 2)
 *
 * Inhalt:    THO_hetmann(): Audienz bei Hetmann Tronde, Vergabe des Hauptauftrags (Grimring)
 *            und spaeter das Empfehlungsschreiben; die Antworten bestimmen gs_subvention,
 *            das spaetere Zeughausbudget. Die Ottaskins der Windzwinger, Sturmkinder und
 *            Garaldssons lassen niemanden ein, beim dritten Einbruchsversuch kommt es zum
 *            Kampf. THO_tav_inn_combi() fuehrt durch die Gasthaeuser mit Taverne und Herberge
 *            unter einem Dach.
 * Aufrufer:  do_special_buildings() (seg066).
 * Daten:     g_combo_mode (Taverne/Herberge im selben Haus, auch von do_tavern/do_inn gelesen).
 * Besonders: THO_hetmann(): der Segen fuer alle Goetter laeuft ueber die Indizes 0..13 und
 *            laesst Ifirn (14) aus. THO_tav_inn_combi(): je nach Weg wird leave_location()
 *            drei- oder viermal aufgerufen, die Gruppe steht danach mal gewendet, mal nicht.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg069.c
 */
#include "v302de.h"
#include "common.h"

#include "seg004.h"
#include "seg025.h"
#include "seg026.h"
#include "seg027.h"
#include "seg032.h"
#include "seg054.h"
#include "seg059.h"
#include "seg097.h"
#include "seg105.h"

signed int g_combo_mode; // ds:0xe42c, 0 = normal, 1 = tavern/inn of a combined house, 2 = change to the other part

/**
 * \brief   the Ottaskin of the hetleute: audience with hetman Tronde (main quest, letter of recommendation)
 */
void THO_hetmann(void)
{
	signed int answer;
	signed int i;
	signed int map_parts;

	load_in_head(66);

	if (!gs_got_letter_het && gs_got_main_quest && (gs_jurge_awaits_letter || gs_need_letter))
	{

		/* count already collected parts of the map */
		for (i = map_parts = 0; i < 9; i++) {
			if (gs_treasure_maps[i] != 0) {
				map_parts++;
			}
		}

		load_in_head(61);

		GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(85), get_tx(82), 0); /* "HETMANN TRONDE" / "<SEID IHR FÜNDIG GEWORDEN??> ERSTAUNT SCHA..." */

		GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(85), get_tx(83), 0); /* "HETMANN TRONDE" / "NACHDEM IHR DEM HETMANN EUER BEGEHREN VORG..." */

		GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(85), gs_min_map_parts <= map_parts ? get_tx(84) : get_tx(85), 0); /* "HETMANN TRONDE" / "<IHR WISST, WELCHE VERANTWORT..." / "<DIE GÖTTER MÖGEN UNS GNÄDIG..." */

		answer = gs_min_map_parts <= map_parts ? 10 : 50;

		/* Original-Bug? index 0 is no god (GOD_ID_PRAIOS = 1), so Ifirn (14) is left out. */
#if FIX_0070
		/* Fix 70: Goetter 1..14 (Praios bis Ifirn) */
		for (i = 1; i <= 14; i++) {
#else
		for (i = 0; i < 14; i++) {
#endif
			gs_gods_estimation[i] += answer;
		}

		GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(85), get_tx(86), 0); /* "HETMANN TRONDE" / "IHR VERABSCHIEDET EUCH, UND MACHT EUCH SCH..." */

		gs_got_letter_het = 1;
		give_new_item_to_group(ITEM_ID_EMPFEHLUNGSSCHREIBEN__HETMAN, 1, 1);

	} else if (!gs_heard_announce || gs_got_main_quest != 0) {

		GUI_output(get_tx2(0)); /* "HOCH ÜBER DER STADT ERHEBT SICH DER OTTASKIN DER HETLEUTE, WO SICH..." */

	} else {
		gs_subvention = 0;

		do {
			answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL,
					get_tx2(1), 3, /* "HOCH ÜBER DER STADT ERHEBT SICH DER OTTASKIN DER HETLEUTE, WO SICH..." */
					get_tx2(2), /* "<WIR KOMMEN VON WEGEN DEM AUFRUF.>" */
					get_tx2(3), /* "<WIR SIND DIE HELDEN, DIE DER HETMANN SUCHT!>" */
					get_tx2(4)); /* "<DAS SAGEN WIR NUR DEM HETMANN!>" */
		} while (answer == -1);

		if (answer == 1) {

			GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(5), 0); /* "<VON WEGEN DEM AUFRUF, AHA. DANN WOLLEN WIR MAL SEHEN, OB ES ÜBERHA..." */

		} else if (answer == 2) {

			GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(6), 0); /* "<DAS WOHL! SO EIN SELBSTVERTRAUEN LOB' ICH MIR! NA, DANN KOMMT MAL..." */

			gs_subvention += 2;

		} else {

			GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(7), 0); /* "<AHA! EIN PAAR GANZ SCHLAUE! NA, DANN KOMMT MAL MIT!>" */

			gs_subvention++;
		}

		load_in_head(61);

		GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(85), get_tx2(8), 0); /* "HETMANN TRONDE" / "EINE DER WACHEN FÜHRT EUCH IN DAS GROSSE L..." */

		gs_heard_announce = 3;

		GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(85), get_tx2(9), 0); /* "HETMANN TRONDE" / "UND WENN DIE@WEISSAGUNGEN NICHT TÄUSCHEN,..." */

		do {
			answer = GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(85), /* "HETMANN TRONDE" */
					get_tx2(10), 3, /* "ES GIBT DA EINE WAFFE, DIE SEIT MEHREREN DUTZEND JAHREN IM ORKLAND..." */
					get_tx2(11), /* "<JAWOLL, HETMANN! WANN SOLLEN WIR LOS?>" */
					get_tx2(12), /* "<SCHON, ABER DA WÄRE NOCH EINE FRAGE ZU KLÄREN.>" */
					get_tx2(13)); /* "<ICH FÜRCHTE, DAS IST FÜR UNS EINE NUMMER ZU GROSS.>" */
		} while (answer == -1);

		if (answer == 1) {

			GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(85), get_tx2(14), 0); /* "HETMANN TRONDE" / "<SOBALD WIE MÖGLICH! PACKT EURE SACHEN, RÜ..." */

#if !FIX_0076
			gs_informer_flags[INFORMER_ID_ISLEIF] = 1;
#else
			/* Fix 76: Isleif nur von 0 auf 1 setzen; wer ihn vor dem Auftrag schon besucht hat (2), behaelt "erledigt" */
			if (!gs_informer_flags[INFORMER_ID_ISLEIF]) gs_informer_flags[INFORMER_ID_ISLEIF] = 1;
#endif
			gs_subvention++;
			gs_got_main_quest = 1;
			gs_quested_months = 0;

		} else if (answer == 2) {

			GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(85), get_tx2(15), 0); /* "HETMANN TRONDE" / "<ICH KANN EUCH EINE VOLLMACHT ERTEILEN, DA..." */

#if !FIX_0076
			gs_informer_flags[INFORMER_ID_ISLEIF] = 1;
#else
			/* Fix 76: wie oben, "erledigt" (2) nicht zuruecksetzen */
			if (!gs_informer_flags[INFORMER_ID_ISLEIF]) gs_informer_flags[INFORMER_ID_ISLEIF] = 1;
#endif
			gs_got_main_quest = 1;
			gs_quested_months = 0;
			gs_got_letter_het = 1;
			give_new_item_to_group(ITEM_ID_EMPFEHLUNGSSCHREIBEN__HETMAN, 1, 1);

		} else if (answer == 3) {

			gs_subvention += 2;

			do {
				answer = GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(85), /* "HETMANN TRONDE" */
						get_tx2(16), 2, /* "<SEID IHR HELDEN ODER HÜHNER? ABER WENN IHR MEINT...@DANN MUSS ICH..." */
						get_tx2(17), /* "<NEIN, NEIN! WIR NEHMEN DEN AUFTRAG!>" */
						get_tx2(18)); /* "<GUT, WIR BRECHEN AUF, SOBALD ES GEHT.>" */
			} while (answer == -1);

			if (answer == 1) {
				GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(85), get_tx2(19), 0); /* "HETMANN TRONDE" / "<DER SEGEN SWAFNIRS UND DER ZWÖLFE MÖGEN E..." */

				gs_got_main_quest = 1;
				gs_quested_months = 0;
			}
		}
	}
}

/**
 * \brief   the Ottaskin of the Windzwinger: no entry, a fight at the third attempt to get in
 */
void THO_windriders(void)
{
	const signed int answer = GUI_radio(get_tx2(20), 3, get_tx2(21), get_tx2(22), get_tx2(23)); /* "IHR STEHT VOR EINEM ETWA DREI..." / "HÖFLICH UM EINLASS BITTEN?" / "IN EINEM UNBEOBACHTETEN MOMEN..." / "VERSUCHEN, DIE TORWACHEN ZU B..." */

	if (answer != -1) {

		if (answer == 1) {

			GUI_output(get_tx2(24)); /* "<TUT UNS LEID, DIES IST DIE OTTASKIN DER WINDZWINGER, UND WIR DÜRFE..." */

		} else if (answer == 2 || answer == 3) {

			if (!gs_otta_windriders) {

				GUI_output(answer == 2 ? get_tx2(25): get_tx2(26)); /* "DAS HIER IST EINE WEHRANLAGE! DA DENKT IHR..." / "<WIR SIND DOCH HIER NICHT IM MITTELREICH,..." */

			} else if (gs_otta_windriders == 1) {

				GUI_output(get_tx2(27)); /* "<IHR SCHON WIEDER? ZUTRITT VERBOTEN, SAGTE ICH!>" */

			} else {

				GUI_output(get_tx2(28)); /* "<JARVE, DIESE SPINNER SIND SCHON WIEDER DA. ICH GLAUBE, ES SIND SPI..." */

				g_fig_discard = 1;
				do_fight(FIGHT_ID_THOR8);
			}

			gs_otta_windriders++;
		}
	}
}

/**
 * \brief   a house with tavern and inn: typeindex 11, 14, 17, 21 -> tavern 0, 1, 2, 6 / inn 70, 71, 72, 73
 *
 * REMARK: do_tavern() and do_inn() call leave_location(), this function and do_special_buildings()
 * once more. Each call turns the party by 180 degrees, so after tavern and inn the party is not turned.
 */
void THO_tav_inn_combi(void)
{
	signed int answer;
	signed int typeindex_bak;

	disable_ani();
	load_ani(4);
	init_ani(0);

	answer = GUI_radio(get_ttx(673), 3, get_ttx(674), get_ttx(675), get_ttx(347)); /* "IN DIESEM GASTHAUS SIND TAVER..." / "DIE TAVERNE BESUCHEN?" / "EIN ZIMMER MIETEN?" / "HERBERGE VERLASSEN" */

	/* save the combo typeindex */
	typeindex_bak = gs_town_typeindex;

	do {

		/* restore the combo typeindex */
		gs_town_typeindex = typeindex_bak;

		if (answer == 1) {

			/* enter TAVERN */

			/* set combo_mode active */
			g_combo_mode = 1;

			/* set the typeindex of the corresponding tavern */
			answer = gs_town_typeindex;
			gs_town_typeindex = (answer == 11 ? 0 : (answer == 14 ? 1 : (answer == 17 ? 2 : 6)));

			g_textbox_width = 3;

			gs_town_loc_type = LOCTYPE_TAVERN;
			do_tavern();

			/* leave the loop or enter the inn in the next iteration */
			answer = 0;
			if (g_combo_mode == 2) {
				answer = 2;
			}

		} else if (answer == 2) {

			/* enter INN */

			/* set combo_mode active */
			g_combo_mode = 1;

			/* set the typeindex of the corresponding inn */
			answer = gs_town_typeindex;
			gs_town_typeindex = (answer == 11 ? 70 : (answer == 14 ? 71 : (answer == 17 ? 72 : 73)));

			g_textbox_width = 3;

			gs_town_loc_type = LOCTYPE_INN;
			do_inn();

			/* leave the loop or enter the tavern in the next iteration */
			answer = 0;
			if (g_combo_mode == 2) {
				answer = 1;
			}

		} else {
			disable_ani();
			return;
		}

	} while (answer != 0);

	disable_ani();
	leave_location();
}


/**
 * \brief   the Ottaskin of the Sturmkinder: no entry, a fight at the third attempt to get in
 */
void THO_stormchildren(void)
{
	const signed int answer = GUI_radio(get_tx2(29), 3, get_tx2(30), get_tx2(31), get_tx2(32)); /* "IHR STEHT VOR EINEM ETWA DREI..." / "HÖFLICH UM EINLASS BITTEN?" / "IN EINEM UNBEOBACHTETEN MOMEN..." / "VERSUCHEN, DIE TORWACHEN ZU B..." */

	if (answer != -1) {

		if (answer == 1) {

			GUI_output(get_tx2(33)); /* "<TUT UNS LEID, ABER FREMDEN IST ES NICHT ERLAUBT, DIE OTTASKIN DER@..." */

		} else if (answer == 2 || answer == 3) {

			if (gs_otta_stormchildren == 0) {

				GUI_output(answer == 2 ? get_tx2(34): get_tx2(35)); /* "EINE OTTASKIN - ZUMAL DIE DER STURMKINDER..." / "<WER GLAUBT IHR EIGENTLICH, WER IHR SEID!..." */

			} else if (gs_otta_stormchildren == 1) {

				GUI_output(get_tx2(36)); /* "<MUSS ICH MICH WIEDERHOLEN? ZUTRITT VERBOTEN!>" */

			} else {

				GUI_output(get_tx2(37)); /* "<EILIF, KOMM MAL HER! DIESES RÄUBERPACK VERSUCHT SCHON ZUM DRITTEN..." */

				g_fig_discard = 1;
				do_fight(FIGHT_ID_THOR8);
			}

			gs_otta_stormchildren++;
		}
	}
}

/**
 * \brief   the Ottaskin of Garald Garaldsson: no entry, a fight at the third attempt to get in
 */
void THO_garaldsson(void)
{
	const signed int answer = GUI_radio(get_tx2(38), 3, get_tx2(39), get_tx2(40), get_tx2(41)); /* "IHR STEHT VOR EINEM ETWA DREI..." / "HÖFLICH UM EINLASS BITTEN?" / "IN EINEM UNBEOBACHTETEN MOMEN..." / "VERSUCHEN, DIE TORWACHEN ZU B..." */

	if (answer != -1) {

		if (answer == 1) {

			GUI_output(get_tx2(42)); /* "<ZUTRITT VERBOTEN! IHR SEID DOCH NICHT IN IRGENDEINEM AUFTRAG HIER,..." */

		} else if (answer == 2 || answer == 3) {

			if (gs_otta_garaldsson == 0) {

				GUI_output(answer == 2 ? get_tx2(43): get_tx2(44)); /* "IHR KÖNNT KEINEN WEG ENTDECKEN, WIE IHR IN..." / "<MERKT EUCH EINS, IHR ROTZIGEN DIEBE! NICH..." */

			} else if (gs_otta_garaldsson == 1) {

				GUI_output(get_tx2(45)); /* "<KENNEN WIR UNS NICHT? ICH SAGTE DOCH SCHON MAL, DASS IHR EUCH VERZ..." */

			} else {

				GUI_output(get_tx2(46)); /* "<IDRA, LAUF UND SAG DER WACHE BESCHEID! TORSTOR! ZU MIR! ALARM!>" */

				g_fig_discard = 1;
				do_fight(FIGHT_ID_THOR8);
			}

			gs_otta_garaldsson++;
		}
	}
}
