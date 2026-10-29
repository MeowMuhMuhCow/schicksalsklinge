/*
 * seg095 - NSC: Begegnung in der Taverne, Aufnahme und Abschied der sechs Mitreisenden
 *
 * Inhalt:    npc_meetings() prueft in sechs bestimmten Tavernen (Clanegh, Phexcaer, Overthorn,
 *            Thorwal, Prem, Manrin), ob dort ein NSC sitzt, und fuehrt dessen Anwerbegespraech
 *            (npc_nariell() ... npc_erwo(), Texte aus NSC.LTX). add_npc() laedt den NSC aus TEMP
 *            auf Heldenplatz 7 (Index 6), remove_npc() speichert ihn zurueck und loescht den Platz.
 *            npc_farewell() laesst den NSC nach seiner Mindestzeit in der Gruppe gehen.
 * Aufrufer:  Tavernengespraech TLK_tavern (seg060, npc_meetings), Spielschleife in seg002
 *            (npc_farewell, monatlich), seg061 (Entlassen), seg072/seg077 (remove_npc/add_npc).
 * Daten:     gs_npc_timers[7] (Index = NPC_ID; 0 = ansprechbar, -1 = nie wieder), gs_npc_months,
 *            gs_npc_meet_tavern.
 * Besonders: Original-Bug 42 (Harika). In remove_npc() wirkt der Parameter days nie: check_hero()
 *            liefert nach dem memset() immer 0, jeder entlassene NSC bekommt Timer -1 und ist danach
 *            nicht wieder anzutreffen (vermutlich nicht so gedacht, Timer zaehlen in seg002 taeglich).
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg095.c
 */

#include <string.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg026.h"
#include "seg028.h"
#include "seg029.h"
#include "seg050.h"
#include "seg095.h"
#include "seg097.h"
#include "seg120.h"

/**
 * \brief   prueft, ob in dieser Taverne ein NSC angetroffen wird, und fuehrt das Gespraech
 *
 * \param   type_index  typeindex der Taverne; dieselbe Taverne loest nicht zweimal hintereinander aus
 * \return  1 = NSC-Gespraech gefuehrt, 0 = keins
 */
signed int npc_meetings(const signed int type_index)
{
	/* check if an NPC is in the party and if we already had an NPC conversation here */
	if (!get_hero(6)->typus && (type_index != gs_npc_meet_tavern)) {

		gs_npc_meet_tavern = type_index;

		if ((gs_town_id == TOWN_ID_CLANEGH) && (gs_town_typeindex == 48) && !((uint8_t)gs_npc_timers[NPC_NARIELL])) {

			npc_nariell();

		} else if ((gs_town_id == TOWN_ID_PHEXCAER) && (gs_town_typeindex == 40) && !((uint8_t)gs_npc_timers[NPC_HARIKA])) {

			npc_harika();

		} else if ((gs_town_id == TOWN_ID_OVERTHORN) && (gs_town_typeindex == 60) && !((uint8_t)gs_npc_timers[NPC_CURIAN])) {

			npc_curian();

		} else if ((gs_town_id == TOWN_ID_THORWAL) && (gs_town_typeindex == 6) && !((uint8_t)gs_npc_timers[NPC_ARDORA]) && gs_dng01_ardora_freed) {

			npc_ardora();

		} else if ((gs_town_id == TOWN_ID_PREM) && (gs_town_typeindex == 74) && !((uint8_t)gs_npc_timers[NPC_GARSVIK])) {

			npc_garsvik();

		} else if ((gs_town_id == TOWN_ID_MANRIN) && (gs_town_typeindex == 87) && !((uint8_t)gs_npc_timers[NPC_ERWO])) {

			npc_erwo();

		} else {
			return 0;
		}

		return 1;
	} else {
		return 0;
	}
}

void npc_farewell(void)
{
	struct struct_hero *hero_i;
	signed int i;
	signed int tmp;

	/* no NPC there */
	if (get_hero(6)->typus == HERO_TYPE_NONE)
		return;

	/* no NPC in that group */
	if (get_hero(6)->group_id != gs_active_group_id)
		return;

	/* Unconscious or dead NPCs cannot be removed automatically (99 means manual). */
	if (check_hero(get_hero(6)) == 0 && (gs_npc_months < 99))
		return;

	tmp = g_tx_file_index;
	load_tx(ARCHIVE_FILE_NSC_LTX);

	switch (get_hero(6)->npc_id) {

		case NPC_NARIELL: {
			if (gs_npc_months >= 2)
				remove_npc(20, 31, ARCHIVE_FILE_NPCS + 0, get_ttx(753), get_tx(9)); /* "NARIELL" / "<MACHT'S GUT. ICH MUSS MAL WIEDER EIN WENI..." */
			break;
		}
		case NPC_HARIKA: {
			if (gs_npc_months >= 2) {
				if (gs_npc_months >= 99 ||
					gs_town_id == TOWN_ID_THORWAL ||
					gs_town_id == TOWN_ID_PHEXCAER ||
					gs_town_id == TOWN_ID_PREM ||
					gs_town_id == TOWN_ID_OBERORKEN) {

					remove_npc(22, 31, ARCHIVE_FILE_NPCS + 1, get_ttx(754), get_tx(19)); /* "HARIKA" / "<TSCHÜSS, LEUTE! ICH MACH DIE BIEGE. HIER..." */

					hero_i = get_hero(0);
					for (i = 0; i < 6; i++, hero_i++) {
						if ((hero_i->typus) && (hero_i->group_id == gs_active_group_id) && (!hero_i->flags.dead))
						{
							/* Original-Bug 42:
							 * When NPC Harika leaves the party, all non-dead heroes in the same group get up to
							 * three attempts for a talent increase in Schleichen. The idea is probably that the
							 * group members had a chance to learn something from the experienced stray Harika.
							 * However, these "free" attempts are missing at the next level-up.
							 * This does not make sense. */

#if FIX_0255
							/* Fix 255: Steigerungsversuche sichern, inc_talent_novice() zieht sie ab */
							char ta_rise_bak = hero_i->saved_talent_increases;
#endif
							/* All non-dead heroes in the same group get a chance to increase TA_SCHLEICHEN */
							inc_talent_novice(hero_i, TA_SCHLEICHEN);
#if FIX_0255
							/* The unwanted reduction is done within the function inc_talent_novice(). We revert it. */
							hero_i->saved_talent_increases = ta_rise_bak;
#endif
						}
					}
				}
			}
			break;
		}
		case NPC_CURIAN: {
			if (gs_npc_months >= 6)
				remove_npc(25, 64, ARCHIVE_FILE_NPCS + 2, get_ttx(755), get_tx(29)); /* "CURIAN" / "<ICH HABE SCHON ZU LANGE MEINE STUDIEN VER..." */
			break;
		}
		case NPC_ARDORA: {
			if (gs_npc_months >= 1)
				remove_npc(21, 31, ARCHIVE_FILE_NPCS + 3, get_ttx(756), get_tx(43)); /* "ARDORA VON GREIFENFURT" / "<TUT MIR LEID, KAMERADEN, DASS ICH EUCH JE..." */
			break;
		}
		case NPC_GARSVIK: {
			if (gs_npc_months >= 2)
				remove_npc(23, 31, ARCHIVE_FILE_NPCS + 4, get_ttx(757), get_tx(53)); /* "GARSVIK THORFINSSON" / "<BEI SWAFNIR! WAR 'NE TOLLE ZEIT MIT EUCH,..." */
			break;
		}
		case NPC_ERWO: {
			if (gs_npc_months >= 2)
				remove_npc(24, 31, ARCHIVE_FILE_NPCS + 5, get_ttx(758), get_tx(63)); /* "ERWO VON GOLLBRINCK" / "<WIEDERSEH'N, LEUTE. ICH GLAUBE, ICH HAB G..." */
			break;
		}
	}

	if ((tmp != -1) && (tmp != (ARCHIVE_FILE_NPCS - 1)))
		load_tx(tmp);
}

//static
void npc_nariell(void)
{
	signed int answer;

	/* load NSC.LTX */
	load_tx(ARCHIVE_FILE_NSC_LTX);

	/* load head */
	load_in_head(20);

	/* show dialog window */
	do {
		answer = GUI_dialogbox((uint8_t*)g_dtp2, get_ttx(753), get_tx(0), /* "NARIELL" / "DREI TISCHE WEITER - ALSO AM ANDEREN ENDE..." */
				3, get_tx(1), get_tx(2), get_tx(3)); /* "<HAU AB ZU DEINEN STINKENDEN..." / "<SEHEN WIR SELBST INKOMPETENT..." / "<ICH HOFFE, IHR HABT WIRKLICH..." */

	} while (answer == -1);

	/* process the answer */
	if (answer == 1) {

		GUI_dialogbox((uint8_t*)g_dtp2, get_ttx(753), get_tx(4), 0); /* "NARIELL" / "<WIE IHR MEINT. ABER SCHLAGT DEMNÄCHST EIN..." */

	} else {
		do {
			answer = GUI_dialogbox((uint8_t*)g_dtp2, get_ttx(753), /* "NARIELL" */
					(answer == 2) ? get_tx(5): get_tx(6), /* "<DAS NICHT, ABER HIER IN DER WILDNIS ZÄHLT..." / "<WILL ICH MEINEN. MEIN NAME IST NARIELL. N..." */
					2, get_tx(7), get_tx(8)); /* "<ACH NEIN, LIEBER NICHT.>" / "<SEI UNS WILLKOMMEN!>" */

		} while (answer == -1);

		/* add nariell */
		if (answer == 2)
			add_npc(ARCHIVE_FILE_NPCS + 0);
	}

	load_tlk(ARCHIVE_FILE_TAVERN_TLK);
}

//static
void npc_harika(void)
{
	signed long money;
	signed int answer;

	/* load NSC.LTX */
	load_tx(ARCHIVE_FILE_NSC_LTX);

	/* load head */
	load_in_head(22);

	/* show dialog window */
	do {
		answer = GUI_dialogbox((uint8_t*)g_dtp2, get_ttx(754), get_tx(10), /* "HARIKA" / "<IHR WOLLT DIESE SCHÖNE STADT SCHON WIEDER..." */
				3, get_tx(11), get_tx(12), get_tx(13)); /* "<MACH DIE BIEGE, MÄDEL. KEINE..." / "<HÖRT SICH GUT AN. ZU GUT. DE..." / "<UND VERSCHWIEGEN WIE EIN BOR..." */

	} while (answer == -1);

	if (answer == 1) {

		GUI_dialogbox((uint8_t*)g_dtp2, get_ttx(754), get_tx(14), 0); /* "HARIKA" / "<PASST AUF, DASS IHR EUCH NICHT IRGENDWANN..." */

	} else if (answer == 2) {

		money = get_party_money();

		answer = (money >= 2000) ? 2 : 1; /* ohne 20 D nur die Antwort "ACH NEIN" */

		do {
			answer = GUI_dialogbox((uint8_t*)g_dtp2, get_ttx(754), get_tx(15), /* "HARIKA" / "<FÜR ZWEIHUNDERT SILBER BIN ICH LOYAL WIE..." */
					answer,	get_tx(17), get_tx(18)); /* "<ACH NEIN, LIEBER NICHT.>" / "<SEI UNS WILLKOMMEN! BIER ODER WEIN?>" */

		} while (answer == -1);

		/* hire her for 20D */
		if (answer == 2) {
			/* subtract 20D */
			money -= 2000;
			set_party_money(money);

			/* add her to the party */
			add_npc(ARCHIVE_FILE_NPCS + 1);
		}
	} else {
		money = get_party_money();

		answer = (money >= 1500) ? 2 : 1; /* ohne 15 D nur die Antwort "ACH NEIN" */

		do {
			answer = GUI_dialogbox((uint8_t*)g_dtp2, get_ttx(754), get_tx(16), /* "HARIKA" / "<LUMPIGE HUNDERTFÜNFZIG IN BESTEM REICHSSI..." */
					answer,	get_tx(17), get_tx(18)); /* "<ACH NEIN, LIEBER NICHT.>" / "<SEI UNS WILLKOMMEN! BIER ODER WEIN?>" */

		} while (answer == -1);

		/* hire her for 15D */
		if (answer == 2) {
			/* subtract 15D */
			money -= 1500;
			set_party_money(money);

			/* add her to the party */
			add_npc(ARCHIVE_FILE_NPCS + 1);
		}
	}

	load_tlk(ARCHIVE_FILE_TAVERN_TLK);
}

void npc_curian(void)
{
	signed int answer;

	/* load NSC.LTX */
	load_tx(ARCHIVE_FILE_NSC_LTX);

	/* load head */
	load_in_head(25);

	/* show dialog window */
	do {
		answer = GUI_dialogbox((uint8_t*)g_dtp2, get_ttx(755), get_tx(20), /* "CURIAN" / "WÄHREND IHR NOCH GRÜBELT, WAS IHR IN DIESE..." */
				3, get_tx(21), get_tx(22), get_tx(23)); /* "<VERPISS DICH, DÄMONENBESCHWÖ..." / "<DA IST DOCH EIN HAKEN DRAN...." / "<WIR KÖNNEN KOMPETENTE MAGIER..." */

	} while (answer == -1);

	if (answer == 1) {

		GUI_dialogbox((uint8_t*)g_dtp2, get_ttx(755), get_tx(24), 0); /* "CURIAN" / "<WIE IHR MEINT. ABER SCHLAGT DEMNÄCHST EIN..." */

	} else {

		do {
			answer = GUI_dialogbox((uint8_t*)g_dtp2, get_ttx(755), /* "CURIAN" */
					(answer == 2) ? get_tx(25): get_tx(26), /* "<KEIN HAKEN, MEINE FREUNDE. UND AUCH KEIN..." / "<DA TREFFT IHR MIT MIR DIE BESTE WAHL. DER..." */
					2, get_tx(27), get_tx(28)); /* "<ACH NEIN, LIEBER NICHT.>" / "<SEI UNS WILLKOMMEN!>" */
		} while (answer == -1);

		/* add NPC */
		if (answer == 2)
			add_npc(ARCHIVE_FILE_NPCS + 2);
	}

	load_tlk(ARCHIVE_FILE_TAVERN_TLK);
}

//static
void npc_ardora(void)
{
	signed int answer;

	/* load NSC.LTX */
	load_tx(ARCHIVE_FILE_NSC_LTX);

	/* load head */
	load_in_head(21);

	/* show dialog window */
	do {
		answer = GUI_dialogbox((uint8_t*)g_dtp2, get_ttx(756), get_tx(30), 3, /* "ARDORA VON GREIFENFURT" / "ALS IHR NOCH ÜBERLEGT, OB IHR EINE WEITERE..." */
					get_tx(31), get_tx(32),	get_tx(33)); /* "<ICH WÜSSTE NICHT, DASS WIR U..." / "<VERDAMMT, IRGENDWOHER KENNE..." / "<ARDORA VON GREIFENFURT! KOMM..." */

	} while (answer == -1);

	if (answer == 1) {
		do {
			answer = GUI_dialogbox((uint8_t*)g_dtp2, get_ttx(756), get_tx(34), 2, /* "ARDORA VON GREIFENFURT" / "<ERINNERT IHR EUCH DENN NICHT? DAS TOTENSC..." */
					get_tx(35), get_tx(36)); /* "<ACH, NATÜRLICH, WIE KONNTEN WIR DAS VERGE..." / "<WIR HABEN SCHON ETWA DREI DUTZEND LEUTE G..." */

		} while (answer == -1);

		if (answer == 2) {

			GUI_dialogbox((uint8_t*)g_dtp2, get_ttx(756), get_tx(37), 0); /* "ARDORA VON GREIFENFURT" / "<WENN IHR MEINT. DANN MUSS ICH MICH WOHL G..." */

		} else {
			do {
				answer = GUI_dialogbox((uint8_t*)g_dtp2, get_ttx(756), get_tx(39), 2, /* "ARDORA VON GREIFENFURT" / "DIE KRIEGERIN SETZT SICH ZU EUCH AN DEN TI..." */
					get_tx(40), get_tx(41)); /* "<GERNE. SCHLIESS DICH UNS AN, WENN DU WILL..." / "<BEDAURE, WIR MÜSSEN ALLEINE WEITERREISEN.>" */

			} while (answer == -1);

			if (answer == 1) {

				add_npc(ARCHIVE_FILE_NPCS + 3);

			} else {

				GUI_dialogbox((uint8_t*)g_dtp2, get_ttx(756), get_tx(42), 0); /* "ARDORA VON GREIFENFURT" / "<WIE IHR MEINT. ABER WENN IHR MICH BRAUCHT..." */
			}
		}
	} else {
		if (answer == 2) {

			GUI_dialogbox((uint8_t*)g_dtp2, get_ttx(756), get_tx(38), 0); /* "ARDORA VON GREIFENFURT" / "<ARDORA VON GREIFENFURT. DAS TOTENSCHIFF...>" */
		}

		do {
			answer = GUI_dialogbox((uint8_t*)g_dtp2, get_ttx(756), get_tx(39), 2, /* "ARDORA VON GREIFENFURT" / "DIE KRIEGERIN SETZT SICH ZU EUCH AN DEN TI..." */
					get_tx(40), get_tx(41)); /* "<GERNE. SCHLIESS DICH UNS AN, WENN DU WILL..." / "<BEDAURE, WIR MÜSSEN ALLEINE WEITERREISEN.>" */

		} while (answer == -1);

		if (answer == 1) {

			add_npc(ARCHIVE_FILE_NPCS + 3);

		} else {
			GUI_dialogbox((uint8_t*)g_dtp2, get_ttx(756), get_tx(42), 0); /* "ARDORA VON GREIFENFURT" / "<WIE IHR MEINT. ABER WENN IHR MICH BRAUCHT..." */
		}
	}

	load_tlk(ARCHIVE_FILE_TAVERN_TLK);
}

//static
void npc_garsvik(void)
{
	signed int answer;

	/* load NSC.LTX */
	load_tx(ARCHIVE_FILE_NSC_LTX);

	/* load head */
	load_in_head(23);

	/* show dialog window */
	do {
		answer = GUI_dialogbox((uint8_t*)g_dtp2, get_ttx(757), get_tx(44), 3, /* "GARSVIK THORFINSSON" / "AN DER THEKE LEHNT EIN THORWALER IM BESTEN..." */
				get_tx(45), get_tx(46),	get_tx(47)); /* "<SAUF WEITER, UND LASS UNS IN..." / "<OHA, OHA! DANN SEID IHR ALSO..." / "<IN DER TAT, IHR SCHEINT MIR..." */

	} while (answer == -1);

	if (answer == 1) {

		GUI_dialogbox((uint8_t*)g_dtp2, get_ttx(757), get_tx(48), 0); /* "GARSVIK THORFINSSON" / "<SELBER SÄUFER. UND PASST AUF, DASS EUCH H..." */

	} else {
		do {
			answer = GUI_dialogbox((uint8_t*)g_dtp2, get_ttx(757), /* "GARSVIK THORFINSSON" */
					(answer == 2) ? get_tx(49) : get_tx(50), /* "<KEINE REDE! GEBT MIR EIN SCHIFF UNTER DIE..." / "<DAS WOHL! ABGEMACHT UND EIN FEUER DRAUF?>" */
					2, get_tx(51), get_tx(52)); /* "<ACH NEIN, LIEBER NICHT.>" / "<SEI UNS WILLKOMMEN! WIRT, EINE RUNDE FEUER!>" */

		} while (answer == -1);

		/* add NPC */
		if (answer == 2)
			add_npc(ARCHIVE_FILE_NPCS + 4);
	}

	load_tlk(ARCHIVE_FILE_TAVERN_TLK);
}

//static
void npc_erwo(void)
{
	signed int answer;

	/* load NSC.LTX */
	load_tx(ARCHIVE_FILE_NSC_LTX);

	/* load head */
	load_in_head(24);

	/* show dialog window */
	do {
		answer = GUI_dialogbox((uint8_t*)g_dtp2, get_ttx(758), get_tx(54), /* "ERWO VON GOLLBRINCK" / "UNVERMITTELT TRITT EIN UNSCHEINBARER MANN..." */
				3, get_tx(55), get_tx(56), get_tx(57)); /* "<SIEH ZU, DASS DU LAND GEWINN..." / "<UND DU VERKAUFST DEINE INFOR..." / "<GEGEN VERSTÄRKUNG HABEN WIR..." */

	} while (answer == -1);

	if (answer == 1) {

		GUI_dialogbox((uint8_t*)g_dtp2, get_ttx(758), get_tx(58), 0); /* "ERWO VON GOLLBRINCK" / "<WIE IHR MEINT. ABER WENN IHR LEUTE BEFRAG..." */

	} else {
		do {
			answer = GUI_dialogbox((uint8_t*)g_dtp2, get_ttx(758), /* "ERWO VON GOLLBRINCK" */
					(answer == 2) ? get_tx(59): get_tx(60), /* "<ABER NEIN! ICH HÄTTE LUST, EIN WENIG MIT..." / "<NA PRIMA, ICH PACK SCHNELL MEINE SACHEN U..." */
					2, get_tx(61), get_tx(62)); /* "<ACH NEIN, LIEBER NICHT.>" / "<SEI UNS WILLKOMMEN!>" */

		} while (answer == -1);

		/* add NPC */
		if (answer == 2)
			add_npc(ARCHIVE_FILE_NPCS + 5);

		}

	load_tlk(ARCHIVE_FILE_TAVERN_TLK);
}

/**
 * \brief   entfernt den NSC aus der Gruppe (speichert ihn vorher nach TEMP)
 *
 * \param   head_index  Kopfbild fuer den Abschiedstext
 * \param   days        gedacht: Tage bis zur naechsten Begegnung; wirkungslos (siehe unten)
 * \param   index       Archivdatei des NSC (ARCHIVE_FILE_NPCS + 0..5)
 * \param   name        Name fuer den Dialog
 * \param   text        Abschiedstext oder NULL
 */
void remove_npc(const signed int head_index, const signed char days,
		const signed int index, char *name, char *text)
{
#if FIX_0256
	/* Fix 256: Zustand des NSC vor dem Loeschen merken, sonst wird der Timer immer -1 */
	signed int npc_ok;
#endif

	if (g_fading_state != 0)
		refresh_colors();

	/* reset NPCs groups position */
	/* TODO: this is bogus, since memset() will come */
	get_hero(6)->slot_pos = 0;

	/* save the NPC */
	save_npc(index);

	/* print farewell message if the NPC has and can */
        if (text && check_hero(get_hero(6))) {

		load_in_head(head_index);
		GUI_dialogbox((uint8_t*)g_dtp2, name, text, 0);
	}

#if FIX_0256
	npc_ok = check_hero(get_hero(6));
#endif
	/* clear the NPC from memory */
	memset((void*)get_hero(6), 0, sizeof(struct struct_hero));

	/* dec group counter */
	gs_group_member_counts[gs_active_group_id]--;

	/* dec global hero counter */
	gs_total_hero_counter--;

	g_refresh_status_line = 1;

	/* TODO:	check_hero() will now, after memset() return 0,
			so the parameter days is useless */
	/* Folge: gs_npc_timers[] wird immer -1, der NSC ist nie wieder anzutreffen
	 * (npc_meetings() verlangt Timer 0, seg002 zaehlt nur Timer != -1 herunter). */
#if FIX_0256
	if (npc_ok)
#else
	if (check_hero(get_hero(6)))
#endif
		gs_npc_timers[index - ARCHIVE_FILE_NPCS + 1] = days;
	else
		gs_npc_timers[index - ARCHIVE_FILE_NPCS + 1] = -1;

}

/**
 * \brief   nimmt einen NSC auf Heldenplatz 7 (Index 6) in die aktive Gruppe auf
 *
 * \param   index       Archivdatei des NSC (ARCHIVE_FILE_NPCS + 0..5); npc_id = index - ARCHIVE_FILE_NPCS + 1
 */
void add_npc(const signed int index)
{
	load_npc(index);

	/* overwrite the picture of the NPC with one from IN_HEAD.NVF */
	memcpy(get_hero(6)->pic, g_dtp2, 0x400);

	/* increment heroes in that group */
	gs_group_member_counts[gs_active_group_id]++;

	/* increment heroes */
	gs_total_hero_counter++;

	/* reset the months the NPC is in the group */
	gs_npc_months = 0;

	/* set a number to decide between the NPCs (1-6) */
	get_hero(6)->npc_id = index - ARCHIVE_FILE_NPCS + 1;

	/* set the group the NPC contains in */
	get_hero(6)->group_id = gs_active_group_id;

	draw_status_line();
}
