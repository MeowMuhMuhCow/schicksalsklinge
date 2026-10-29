/*
 * seg068 - Thorwal: Sondergebaeude (Teil 1)
 *
 * Inhalt:    Eisenhof (Schmied, Auskunft eines Arbeiters), Immanstadion (Spiele im Fruehjahr
 *            und Herbst), Botschaft des Alten Reiches (ab Peraine 17 Hal geschlossen),
 *            Stoerrebrandts Kontor (Einzahlen, Abheben, Kredit bis 20 D im Monat),
 *            Zeughaus (Waffen fuer das vom Hetmann bewilligte Geld, nur mit Schreiben),
 *            Magistrat (Schreiben von Jadra), Muehle, Schwarzer Finger, Zwingfeste mit
 *            Meister Dramosch (Ugdalf-Auftrag) und die Akademie (Fluch aufheben fuer 20 D,
 *            Artefakt analysieren fuer 10 D, auch gegen einen Gegenstand in Zahlung).
 * Aufrufer:  do_special_buildings() (seg066).
 * Daten:     zwei feste Meldungstexte (Kontoobergrenze, "OFFENSICHTLICH GESCHLOSSEN!").
 * Besonders: THO_academy(): ein verfluchter Held auf Platz 0 wird nicht erkannt, und Text 63
 *            erscheint mit wortwoertlichem "%s". THO_arsenal(): im Zeughaus wird das Budget
 *            als Gruppengeld gesetzt, die Schuldenabfrage von do_merchant() kann daraus
 *            bezahlt werden. Bekannt (Bright-Eyes): 20 D bei jedem Betreten der Zwingfeste.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg068.c
 */
#include <stdio.h>
#include <string.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg007.h"
#include "seg026.h"
#include "seg047.h"
#include "seg055.h"
#include "seg058.h"
#include "seg068.h"
#include "seg075.h"
#include "seg096.h"
#include "seg097.h"
#include "seg099.h"
#include "seg105.h"

static const char g_str_bank_deposit_to_big[57] = "TUT MIR LEID, ICH AKZEPTIERE MAXIMAL 30000 SILBERM\x9aNZEN!"; // ds:0x7c44
static const char g_str_obviously_closed[28] = "OFFENSICHTLICH GESCHLOSSEN!"; // ds:0x7c7d

/**
 * \brief   the Eisenhof: visit a smith (typeindex 41) or ask a worker for 1 S (CH test, rumor)
 */
void THO_eisenhof(void)
{
	signed int answer;
	int32_t money;

	do {
		answer = GUI_radio(get_tx2(47), 3, get_tx2(48),	get_tx2(49), get_tx2(50)); /* "VOR EUCH ERSTRECKT SICH DAS G..." / "ZU EINER DER SCHMIEDEN GEHEN?" / "EINEN DER ARBEITER AUSFRAGEN?" / "DEN EISENHOF VERLASSEN?" */

	} while (answer == -1);

	if (answer == 1) {

		gs_town_typeindex = 41;
		do_smith();

	} else if (answer == 2) {

		GUI_input(get_tx2(51), 0); /* "IHR STREBT AUF EINEN DER BRONZEGIESSER ZU: <HEDA, GUTER MANN, KÖNNT..." */

		money = get_party_money();
		money -= 10;
		set_party_money(money);

		/* test for CH+0 */
		if (test_attrib(get_first_hero_available_in_group(), ATTRIB_CH, 0) > 0) {

			GUI_input(get_tx2(52), 0); /* "<NA JA, ICH BIN AUCH NICHT VON HIER, ABER VIELLEICHT INTERESSIERT E..." */

#if defined(__BORLANDC__)
			sprintf(g_dtp2, get_tx(random_schick(26) + 55)); /* 56..81 zufällig, z.B. "VIELE OTTAS SIND DAS GANZE JAHR ÜBER UNTER..." */
#else
			strcpy(g_dtp2, get_tx(random_schick(26) + 55)); /* 56..81 zufällig, z.B. "VIELE OTTAS SIND DAS GANZE JAHR ÜBER UNTER..." */
#endif
			GUI_input(g_dtp2, 0);
		} else {
			GUI_input(get_tx2(53), 0); /* "<TUT MIR LEID, DA KANN ICH EUCH AUCH NICHT WEITERHELFEN, HOHE HERREN!>" */
		}
	}
}

/**
 * \brief   the Imman stadium: games only in spring and autumn on day 5 of the week
 *
 * REMARK: text 55 mentions an entrance fee of one heller, which is not charged.
 */
void THO_imman(void)
{
	signed int tmp = get_current_season();

	if ((tmp == SEASON_SPRING || tmp == SEASON_AUTUMN) && (gs_day_of_week == 5)) {

		/* ask to visit the game */
		if (GUI_bool(get_tx2(55)) != 0) { /* "IHR STEHT AM EINGANG DES IMMANSTADIONS. VON DRINNEN KÖNNT IHR DIE Z..." */

			tmp = random_schick(4) + 0x38;

			sprintf(g_dtp2, get_tx2(56), /* "HEUTE SPIELT %s GEGEN %s. DIE KEILEREI DAUERT EINE STUNDE, DANN HAT..." */
				/* winning team, losing team, winning team */
				get_tx2(tmp), get_tx2(random_schick(7) + 0x3c),	get_tx2(tmp), /* Sieger 57..60, Verlierer 61..67, z.B. "POTTWAL PREM" / "RUHMREICH NOSTRIA" */
				/* winning team score, losing team score */
				random_interval(15, 30), random_schick(14));

			GUI_input(g_dtp2, 0);
		}
	} else {
		/* no imman game at the moment */
		GUI_input(get_tx2(54), 0); /* "IHR STEHT AM EINGANG DES IMMANSTADIONS. HEUTE HERRSCHT HIER GÄHNEND..." */
	}

}

/**
 * \brief   the embassy of the old empire
 */
void THO_botschaft(void)
{
	int closed = 0;

	/* At the 6th month in year 17 Hal the embassy is closed and another message is shown */
	/* Reason:
	 * Diplomatische Probleme infolge des Bruchs des Garether Vertrags? (Die Thorwaler besetzen im Hesinde Salzerhaven) *
	 * https://www.crystals-dsa-foren.de/showthread.php?tid=700&pid=99706#pid99706 */

	if ((gs_year > 17) || ((gs_year == 17) && (gs_month > 5))) {

		closed = 1;
	}

	GUI_input(!closed ? get_tx2(68) : get_tx2(69), 0); /* "EIN SCHILD AN DER SCHWEREN EICHENTÜR VERKÜ..." / "EIN SCHILD AN DER SCHWEREN EICHENTÜR VERKÜ..." */
}

/**
 * \brief   Stoerrebrandt's office: deposit and withdraw money, credit of at most 20 D per month
 *
 * gs_bank_deposit is counted in silver (negative = debts).
 */
void THO_bank(void)
{
	signed int answer;
	signed int done;
	signed int excess;
	int32_t p_money;

	done = 0;

	if (gs_bank_deposit <= -1000) {

		GUI_input(get_tx2(77), 0); /* "EURE SCHULDEN BEI STOERREBRANDT BELAUFEN SICH AUF MEHR ALS HUNDERT..." */

		if (!gs_debt_days) {
			gs_debt_days = 7;
		}
	}

	load_in_head(16);

	do {

		sprintf(g_text_output_buf, get_tx2(72), gs_bank_deposit); /* "IHR BETRETET DAS PRACHTVOLL AUSGESTATTETE KONTOR DES FESTUMER HANDE..." */

		do {
			answer = GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(81), g_text_output_buf, 3, /* "STOVER STOERREBRANDT" */
						get_tx2(73), get_tx2(74), get_tx2(80)); /* "GOLD EINZAHLEN?" / "GOLD ABHEBEN?" / "GEHEN" */
		} while (answer == -1);


		if (answer == 2) {

			answer = GUI_input(get_tx2(75), 3); /* "WIEVIELE SILBERSTÜCKE?" */

			if (answer <= 0) {

				GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(81), get_tx2(79), 0); /* "STOVER STOERREBRANDT" / "<HAHAHA!> ANTWORTET STOERREBRANDT SARKASTI..." */

			} else {

				if (gs_days_to_cens
					|| ((gs_bank_deposit > 0) && (gs_bank_deposit + 200 < answer))
					|| ((gs_bank_deposit <= 0) && (answer > 200))) {

					GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(81), get_tx2(76), 0); /* "STOVER STOERREBRANDT" / "IHR KÖNNT NUR MAXIMAL ZWANZIG DUKATEN PRO..." */

				} else {

					if (gs_bank_deposit < answer) {

						if (gs_bank_deposit > 0) {
							answer -= gs_bank_deposit;
							p_money = get_party_money();
							p_money += 10 * gs_bank_deposit;
							set_party_money(p_money);
							gs_bank_deposit = 0;
						}

						gs_monthly_credit += answer;

						if (gs_monthly_credit > 200) {

							GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(81), get_tx2(76), 0); /* "STOVER STOERREBRANDT" / "IHR KÖNNT NUR MAXIMAL ZWANZIG DUKATEN PRO..." */

							excess = gs_monthly_credit - 200;
							answer -= excess;
							gs_monthly_credit = 200;

							if (answer < 0) {
								answer = 0;
							}
						}

						if (gs_monthly_credit >= 200) {
							gs_days_to_cens = 30;
						}
					}

					gs_bank_deposit -= answer;
					p_money = get_party_money();
					p_money += 10 * answer;
					set_party_money(p_money);
				}
			}

		} else if (answer == 1) {

			if (gs_bank_deposit >= 30000) {

				/* prevent overflow  at 32767 */
				GUI_output((char*)g_str_bank_deposit_to_big);

			} else {

				answer = GUI_input(get_tx2(75), 3); /* "WIEVIELE SILBERSTÜCKE?" */
				p_money = get_party_money();

				if (answer * 10 > p_money) {

					GUI_output(get_ttx(401)); /* "SOVIEL GELD HABT IHR DOCH GAR NICHT!" */

				} else {

					if (answer <= 0) {

						GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(81), get_tx2(79), 0); /* "STOVER STOERREBRANDT" / "<HAHAHA!> ANTWORTET STOERREBRANDT SARKASTI..." */

					} else {

						p_money -= 10 * answer;
						set_party_money(p_money);
						gs_days_to_cens = 0;
						gs_bank_deposit += answer;

						if (gs_monthly_credit) {

							gs_monthly_credit -= answer;

							if (gs_monthly_credit < 0) {

								gs_monthly_credit = 0;
							}
						}

						if (gs_bank_deposit > -1000) {
							gs_debt_days = 0;
						}
					}
				}
			}
		} else {
			done = 1;
		}

	} while (!done);
}

/**
 * \brief   the arsenal: with the letter of Jadra (or of the hetman) the party may choose weapons
 *          for a budget of 15 D per gs_subvention (at least 10 D), as merchant 92
 */
void THO_arsenal(void)
{
	/* ARSENAL / ZEUGHAUS */
	signed int answer;
	signed int options;
	signed int tw_bak;
	int32_t p_money;

	if (gs_day_timer < HOURS(8) || gs_day_timer > HOURS(19)) {

		GUI_output(get_ttx(482)); /* "GEÖFFNET VON@8.00 UHR BIS 19.00 UHR" */

	} else if (gs_merchant_kicked_flags[gs_town_typeindex]) {

		talk_merchant();

	} else if (gs_arsenal_money) {

		load_in_head(13);

		/* only show two options when the group has "LETTER FROM JADRA" or "LETTER OF INTRODUCTION" */
		options = get_first_hero_with_item(ITEM_ID_SCHREIBEN_VON_JADRA) != -1 || get_first_hero_with_item(ITEM_ID_EMPFEHLUNGSSCHREIBEN__HETMAN) != -1 ? 2 : 1;

		do {
			answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(0), options, get_tx2(2), get_tx2(1)); /* "<WAS WOLLT IHR HIER IM ZEUGHA..." / "<NEIN, BRAUCHT MAN HIER SO WAS?>" / "<JA, HABEN WIR. HIER.>" */

		} while (answer == -1);

		if (answer == 2) {

			if (gs_arsenal_money == -1) {

				/* calculate the maximal shopping price [10-60] D */
				if ((gs_arsenal_money = 15 * gs_subvention) < 10) {
					/* at least 10D */
					gs_arsenal_money = 10;
				}
			}

			sprintf(g_dtp2 + 0x400, get_tx2(3), gs_arsenal_money); /* "<HOLLA, DAS IST JA SCHON SEIT JAHREN NICHT MEHR VORGEKOMMEN. HABEN..." */

			gs_arsenal_money *= 100;
			GUI_dialog_na(0, (char*)(g_dtp2 + 0x400));
			/* REMARK: the budget replaces the party money during do_merchant(). There the debt check
			 * for Thorwal (Stoerrebrandt) still works, so 50 D of debts can be paid from the budget.
			 * The rest of the budget is stored in whole ducats, smaller amounts are lost. */
			p_money = get_party_money();
			set_party_money(gs_arsenal_money);

			gs_town_typeindex = MERCHANT_TYPEINDEX_THORWAL_ZEUGHAUS;
			tw_bak = g_textbox_width;
			g_textbox_width = 3;

			do_merchant();

			gs_arsenal_money = (signed int)get_party_money();
			gs_arsenal_money /= 100;
			g_textbox_width = tw_bak;
			set_party_money(p_money);


		} else {
			GUI_dialog_na(0, get_tx2(4)); /* "<JA, BRAUCHT MAN. OHNE BEGLAUBIGUNG KANN ICH EUCH KEINE WAFFEN AUSH..." */
			gs_need_letter = 1;
		}

	} else {

		GUI_output((char*)g_str_obviously_closed);
	}
}

/**
 * \brief   the magistracy: Jadra gives the party her letter (after the main quest was given)
 */
void THO_magistracy(void)
{
	signed int answer;

	if (!gs_got_main_quest) {

		GUI_output(get_tx2(5)); /* "DIES IST DAS GEBÄUDE DES MAGISTRATS DER STADT. IN DER EINGANGSHALLE..." */
		GUI_output(get_tx2(6)); /* "<WIR WOLLTEN UNS ERKUNDIGEN, OB DIE STADT FÄHIGE KÄMPEN FÜR GEFÄHRL..." */
		GUI_output(get_tx2(7)); /* "<WIR SIND DOCH HIER KEINE SÖLDNERVERMITTLUNG. FRAGT DOCH IM HAFEN N..." */

	} else {

		do {
			answer = GUI_radio(get_tx2(5), 3, get_tx2(8), get_tx2(9), get_tx2(10)); /* "DIES IST DAS GEBÄUDE DES MAGI..." / "<DER HETMANN SCHICKT UNS.>" / "<WIR WOLLEN BÜRGER VON THORWA..." / "<WIR BRAUCHEN GELD FÜR EINEN..." */

		} while (answer == -1);

		if (answer == 1) {

			if (!gs_got_letter_jad) {

				gs_got_letter_jad = 1;

				GUI_output(get_tx2(14)); /* "WENIGE MINUTEN SPÄTER BETRITT EINE ABENTEUERLICH AUSSEHENDE THORWAL..." */

				/* get "LETTER FROM JADRA" */
				give_new_item_to_group(ITEM_ID_SCHREIBEN_VON_JADRA, 1, 1);

			} else {
				GUI_output(get_tx2(15)); /* "<IHR HABT DOCH SCHON ALLES, WAS IHR BRAUCHT.>" */
			}

		} else if (answer == 2) {

			GUI_output(get_tx2(12)); /* "<IHR SEID DOCH DIE ABENTEURER, DIE DER HETMANN MIT EINER WICHTIGEN..." */

		} else {
			GUI_output(get_tx2(13)); /* "<DAS SAGT HIER JEDER. WENN IHR GELD FÜR'S ESSEN BRAUCHT, DANN FRAGT..." */
		}
	}
}

/**
 * \brief   the windmill
 */
void THO_mueller(void)
{
	if (GUI_bool(get_tx2(16))) { /* "IHR STEHT VOR EINER WINDMÜHLE, EINEM WUNDERWERK DER TECHNIK.@WOLLT..." */

		/* first visit ? */
		GUI_output(!gs_visited_miller ? get_tx2(17) : get_tx2(18)); /* "NACH MEHRMALIGEM KLOPFEN ÖFFNET EUCH EIN S..." / "<ICH SAGTE DOCH, DASS IHR MICH IN RUHE LAS..." */

		/* mark the miller as visited */
		gs_visited_miller = 1;
	}
}

/**
 * \brief   the black finger (tower of the historian Doran von Honingen, who wants no visitors)
 */
void THO_black_finger(void)
{
	if (GUI_radio(get_tx2(19), 2, get_tx2(20), get_tx2(21)) == 1) { /* "DER SCHWARZE FINGER IST EIN D..." / "ANKLOPFEN?" / "WEITERZIEHEN?" */

		GUI_output(get_tx2(22)); /* "EIN VERHUTZELTES MÄNNCHEN ÖFFNET DIE TÜR - <DER MEISTER WÜNSCHT KEI..." */
	}
}

/**
 * \brief   show a message of master Dramosch (only used in THO_ugdalf())
 */
/* static */
void dramosch_says(char *msg)
{
	GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(48), msg, 0); /* "MEISTER DRAMOSCH" */
}

/**
 * \brief   the Zwingfeste: master Dramosch and the Ugdalf quest
 *
 * gs_quest_ugdalf: 0 = not offered, 1 = accepted, 3 = solved (set in the dungeon), 4 = rewarded.
 */
void THO_ugdalf(void)
{
	signed int answer;
	signed int randval;

	load_in_head(gs_quest_ugdalf == 0 ? 0 : 14);

	if (gs_quest_ugdalf == 0) {

		/* talk to the guards */
		randval = random_schick(10) - 1;

		answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL,
					get_tx2(23), 3, /* "HOCH ÜBER DER STADT STEHT DIE ALTE KAISERLICHE FESTUNG, VON DEN THO..." */
					get_tx2(randval + 38), /* 38..47 zufällig: Losungswort, z.B. "GRÜNKOHL MIT SPECK" */
					get_tx2(24), /* "RONDRA ZUM GRUSS! BESPRECHUNG MIT MEISTER DRAMOSCH!" */
					get_tx2(25)); /* "<WIR GEHEN JA SCHON WIEDER!>" */

		if (answer == 1) {

			GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(27), 0); /* "<EINEN MOMENT! ICH FRAGE NACH!>" */

		} else if (answer == 2) {

			/* talk to DRAMOSCH */
			GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(28), 0); /* "NACH EINIGER ZEIT ERSCHEINT EIN ZWERG IN DEN BESTEN JAHREN, ANGETAN..." */

			load_in_head(14);

			dramosch_says(get_tx2(29)); /* "<...KÖNNTET IHR MIR BEI EINEM PROBLEM ZUR SEITE STEHEN. WIE IHR VIE..." */

			do {
				answer = GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(48), get_tx2(30), 2, get_tx2(31), get_tx2(32)); /* "MEISTER DRAMOSCH" / "<ZWANZIG DUKATEN VORKASSE, FÜ..." / "<GEMACHT, WIR SIND DABEI!>" / "<ACH NEIN, DAS IST UNS EINE S..." */

			} while (answer == -1);

			if (answer == 1) {
				/* take the quest */
				dramosch_says(get_tx2(33)); /* "<GUT. BESORGT EUCH DIE ENTSPRECHENDE AUSRÜSTUNG UND KOMMT DANN@WIED..." */
				gs_quest_ugdalf = 1;

			} else {

				dramosch_says(get_tx2(34)); /* "<SCHADE, ICH HÄTTE GEDACHT, DASS IHR DEN NÖTIGEN MUMM AUFWEIST. SO..." */
			}
		}
	} else if (gs_quest_ugdalf == 1 || !gs_dng14_ugdalf_done) {

		dramosch_says(get_tx2(35)); /* "MEISTER DRAMOSCH ERWARTET EUCH SCHON AM TOR, ALS IHR EINTREFFT.@<IH..." */

		/* enter the dungeon */
		DNG_enter_dungeon(DUNGEON_ID_ZWINGFESTE);

		gs_x_bak = gs_x;
		gs_y_bak = gs_y;

		if (gs_quest_ugdalf == 1) {
			add_party_money(2000L);
#if FIX_0067
			/* Fix 67: das Geld gibt es nur einmal (Bright-Eyes-Fix); der Zugang haengt weiter an !gs_dng14_ugdalf_done */
			gs_quest_ugdalf = 2;
#endif

		/* Original-Bug:	Everytime the heroes enter the dungeon they get 20D.
					Why this fix works is not seen that easy.
					As long as gs_dng14_ugdalf_done is 0 this block is executed.
		 */
		}

	} else if (gs_quest_ugdalf == 3) {

		/* talk with DRAMOSCH for 8 h */
		dramosch_says(get_tx2(36)); /* "<ICH WUSSTE DOCH, DASS MAN SICH AUF EUCH VERLASSEN KANN. HIER SIND..." */
		timewarp(HOURS(8));

		/* mark this quest as done */
		gs_quest_ugdalf = 4;

		/* get the reward */
		add_hero_ap_all(25);
		add_party_money(5000);
	} else {

		dramosch_says(get_tx2(37)); /* "<AH, IHR SEID ES. EINE LAGE AMBOSSKRONEN? PRIMA. IHR SEID SICHERLIC..." */

#if defined(__BORLANDC__)
		sprintf(g_dtp2 + 0x400, get_tx(random_schick(26) + 55)); /* 56..81 zufällig, z.B. "VIELE OTTAS SIND DAS GANZE JAHR ÜBER UNTER..." */
#else
		strcpy(g_dtp2 + 0x400, get_tx(random_schick(26) + 55)); /* 56..81 zufällig, z.B. "VIELE OTTAS SIND DAS GANZE JAHR ÜBER UNTER..." */
#endif

		dramosch_says((char*)(g_dtp2 + 0x400));

		/* enter the dungeon */
		DNG_enter_dungeon(DUNGEON_ID_ZWINGFESTE);

		gs_x_bak = gs_x;
		gs_y_bak = gs_y;
	}
}

/**
 * \brief   the magician Aleya identifies an artifact (spell_analues() with the academy behavior)
 */
/* should be static */
void academy_analues(void)
{
	signed int tx_file_index_bak;
	signed int hero_pos;

	GUI_input(get_tx2(62), 0); /* "<GUT, DANN LASST UNS ZUR SACHE KOMMEN.>" */

	/* change behavior of analues spell */
	gs_in_academy = 99;

	/* select a hero (does not need to be a magic user here) */
	hero_pos = select_hero_ok(get_ttx(794)); /* "WER SOLL ANALYSIEREN?" */

	if (hero_pos != -1) {

		g_spelluser = get_hero(hero_pos);

		tx_file_index_bak = g_tx_file_index;

		load_tx(ARCHIVE_FILE_SPELLTXT_LTX);

		sprintf(g_dtp2, get_tx2(64), (char*)spell_analues()); /* "DER MAGUS BETRACHTET DAS ARTEFAKT EINIGE ZEITLANG.@%s@ICH HOFFE,@IC..." */

		if (tx_file_index_bak != -1 && tx_file_index_bak != 222) { /* 222 = ARCHIVE_FILE_SPELLTXT_LTX */

			load_tx(tx_file_index_bak);
		}

		GUI_input(g_dtp2, 0);

		gs_academy_daily_ident = 1;
	}

	/* change behaviour of analues spell */
	gs_in_academy = 0;
}

#if FIX_0339
/* Fix 339: das Stueck, das academy_get_equal_item() angeboten hat: erstes heiles Stueck dieser Art,
 * gesucht in derselben Reihenfolge (Platz 0..5 der aktiven Gruppe, lebend) */
static signed int fix_academy_pay_slot(const signed int item_id, struct struct_hero **p_hero)
{
	signed int i;
	signed int inv_slot;
	struct struct_hero *hero = get_hero(0);

	for (i = 0; i < 6; i++, hero++) {

		if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !hero->flags.dead)
		{
			for (inv_slot = 0; inv_slot < NR_HERO_INVENTORY_SLOTS; inv_slot++) {

				if ((hero->inventory[inv_slot].item_id == item_id) && !hero->inventory[inv_slot].flags.broken) {
					*p_hero = hero;
					return inv_slot;
				}
			}
		}
	}

	/* nicht gefunden: wie bisher */
	*p_hero = get_hero(get_first_hero_with_item(item_id));
	return inv_slot_of_item(*p_hero, item_id);
}
#endif

/**
 * \brief   the academy (Schule der Hellsicht): remove a curse, analyse an artifact, no potions
 */
void THO_academy(void)
{
	signed int answer;
	signed int item_id;
	signed int i; /* dual use as inv_slot and hero_pos */
	signed int cursed_hero_pos;
	int32_t p_money;
	struct struct_hero *hero;

	/* find the position of the first cursed (=renegade) hero.
	 * Original-Bug? 0 also means "nobody is cursed", so a cursed hero in slot 0 is not recognized
	 * (text 67: "KEINER VON EUCH HAT EINEN FLUCH AUF SICH"). */
	hero = get_hero(0);
#if FIX_0068
	/* Fix 68: -1 = niemand verflucht, damit auch Platz 0 erkannt wird */
	for (i = 0, cursed_hero_pos = -1; i <= 6; i++, hero++) {
#else
	for (i = cursed_hero_pos = 0; i <= 6; i++, hero++) {
#endif

		if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && hero->flags.renegade)
		{
			cursed_hero_pos = i;
			break;
		}
	}

	do {
		answer = GUI_radio(get_tx2(49), 3, get_tx2(50), get_tx2(51), get_tx2(52)); /* "IHR STEHT VOR DER <SCHULE DER..." / "<EINER VON UNS IST VERFLUCHT...." / "<WIR MÖCHTEN EIN ARTEFAKT ANA..." / "<VERKAUFT IHR WUNDERTINKTUREN?>" */

	} while (answer == -1);

	if (answer == 1) {

		/* remove curse */

#if FIX_0068
		if (cursed_hero_pos == -1) {
#else
		if (cursed_hero_pos == 0) {
#endif

			GUI_input(get_tx2(67), 0); /* "DER ADEPT FÜHRT EUCH IN EIN HINTERZIMMER.@<WARTET HIER.> NACH EINIG..." */

		} else if (gs_academy_daily_curse != 0) {

			GUI_input(get_tx2(65), 0); /* "<TUT MIR LEID. ARANDA IST LEIDER NICHT ZU SPRECHEN. VERSUCHT ES MOR..." */

		} else {

			sprintf(g_dtp2, get_tx2(53), hero->alias); /* "DER ADEPT FÜHRT EUCH IN EIN HINTERZIMMER.@<WARTET HIER.> NACH EINIG..." */

			do {
				answer = GUI_radio(g_dtp2, 2, get_tx2(68), get_tx2(69)); /* "DAS GOLD RAUSRÜCKEN" / "LIEBER WIEDER GEHEN" */

			} while (answer == -1);

			if (answer == 1) {

				item_id = academy_get_equal_item(2000);

				if (item_id >= 0) {

					sprintf(g_dtp2, get_tx2(56), GUI_name_inflect_with_article( /* "<IHR HABT DAS GELD NICHT? NA, DANN NEHME ICH %s IN ZAHLUNG, WENN IH..." */
						INFLECT_DEFINITE_ARTICLE | INFLECT_SINGULAR | INFLECT_4TH_CASE,
						item_id,
						INFLECT_NAME_TYPE_ITEM
					));

					do {
						answer = GUI_radio(g_dtp2, 4, get_tx2(57), get_tx2(58), get_tx2(59), get_tx2(60)); /* "<DOCH, HABEN WIR!>" / "<BITTE, BEDIENT EUCH!>" / "<DAS IST UNS ZU TEUER!>" / "<NA GUT, ALLES HAT HALT SEINE..." */

					} while (answer == -1);

					if (answer == 1 || answer == 3) {

						GUI_input(get_tx2(61), 0); /* "<TJA, DANN KANN ICH EUCH NICHT HELFEN. SEHT ZU, DASS IHR GELD AUFTR..." */

					} else {

#if FIX_0339
						/* Fix 339: mit dem angebotenen (heilen) Stueck bezahlen, nicht mit dem ersten gleichen */
						i = fix_academy_pay_slot(item_id, &hero);
#else
						hero = get_hero(get_first_hero_with_item(item_id));
						i = inv_slot_of_item(hero, item_id);
#endif

						if (drop_item(hero, i, 1)) {

							GUI_input(get_tx2(62), 0); /* "<GUT, DANN LASST UNS ZUR SACHE KOMMEN.>" */
							/* Original-Bug? text 63 contains %s (the hero), but is shown without sprintf() */
#if FIX_0069
							/* Fix 69: den Namen des verfluchten Helden einsetzen */
							sprintf(g_dtp2, get_tx2(63), get_hero(cursed_hero_pos)->alias);
							GUI_input(g_dtp2, 0);
#else
							GUI_input(get_tx2(63), 0);
#endif /* "NACH ETWA EINER HALBEN STUNDE BEMERKT %s, DASS ES IHM BEREITS DEUTL..." */

							gs_academy_daily_curse = 1;

							get_hero(cursed_hero_pos)->flags.renegade = 0;

						} else {
							GUI_input(get_tx2(70), 0); /* "<DANN STEHLT NICHT MEINE ZEIT! HESINDE ZUM GRUSSE!>" */
						}
					}

				} else if (item_id == -2) {

					p_money = get_party_money();
					p_money -= 2000;
					set_party_money(p_money);

#if FIX_0069
					/* Fix 69: den Namen des verfluchten Helden einsetzen */
					sprintf(g_dtp2, get_tx2(63), get_hero(cursed_hero_pos)->alias);
					GUI_input(g_dtp2, 0);
#else
					GUI_input(get_tx2(63), 0);
#endif /* "NACH ETWA EINER HALBEN STUNDE BEMERKT %s, DASS ES IHM BEREITS DEUTL..." */

					gs_academy_daily_curse = 1;

					get_hero(cursed_hero_pos)->flags.renegade = 0;

				} else {
					GUI_input(get_ttx(401), 0); /* "SOVIEL GELD HABT IHR DOCH GAR NICHT!" */
				}
			} else {
				GUI_input(get_tx2(70), 0); /* "<DANN STEHLT NICHT MEINE ZEIT! HESINDE ZUM GRUSSE!>" */
			}
		}

	} else if (answer == 2) {

		/* identify item */

		if (gs_academy_daily_ident != 0) {

			GUI_input(get_tx2(66), 0); /* "<TUT MIR LEID. ALEYA IST LEIDER NICHT ZU SPRECHEN. VERSUCHT ES MORG..." */

		} else {

			do {
				answer = GUI_radio(get_tx2(54), 2, get_tx2(68), get_tx2(69)); /* "DER ADEPT FÜHRT EUCH IN EIN H..." / "DAS GOLD RAUSRÜCKEN" / "LIEBER WIEDER GEHEN" */

			} while (answer == -1);

			if (answer == 1) {

				item_id = academy_get_equal_item(1000);

				if (item_id >= 0) {

					sprintf(g_dtp2, get_tx2(56), GUI_name_inflect_with_article( /* "<IHR HABT DAS GELD NICHT? NA, DANN NEHME ICH %s IN ZAHLUNG, WENN IH..." */
						INFLECT_DEFINITE_ARTICLE | INFLECT_SINGULAR | INFLECT_4TH_CASE,
						item_id,
						INFLECT_NAME_TYPE_ITEM
					));

					do {
						answer = GUI_radio(g_dtp2, 4, get_tx2(57), get_tx2(58), get_tx2(59), get_tx2(60)); /* "<DOCH, HABEN WIR!>" / "<BITTE, BEDIENT EUCH!>" / "<DAS IST UNS ZU TEUER!>" / "<NA GUT, ALLES HAT HALT SEINE..." */

					} while (answer == -1);

					if (answer == 1 || answer == 3) {

						GUI_input(get_tx2(61), 0); /* "<TJA, DANN KANN ICH EUCH NICHT HELFEN. SEHT ZU, DASS IHR GELD AUFTR..." */

					} else {

#if FIX_0339
						/* Fix 339: mit dem angebotenen (heilen) Stueck bezahlen, nicht mit dem ersten gleichen */
						i = fix_academy_pay_slot(item_id, &hero);
#else
						hero = get_hero(get_first_hero_with_item(item_id));
						i = inv_slot_of_item(hero, item_id);
#endif

						if (drop_item(hero, i, 1)) {

							academy_analues();

						} else {

							GUI_input(get_tx2(70), 0); /* "<DANN STEHLT NICHT MEINE ZEIT! HESINDE ZUM GRUSSE!>" */
						}
					}

				} else if (item_id == -2) {

					p_money = get_party_money();
					p_money -= 1000;
					set_party_money(p_money);

					academy_analues();
				} else {
					GUI_input(get_ttx(401), 0); /* "SOVIEL GELD HABT IHR DOCH GAR NICHT!" */
				}
			} else {

				GUI_input(get_tx2(70), 0); /* "<DANN STEHLT NICHT MEINE ZEIT! HESINDE ZUM GRUSSE!>" */

			}
		}
	} else {

		GUI_input(get_tx2(55), 0); /* "<DANN WENDET IHR EUCH AM BESTEN AN EINEN APOTHEKARIUS. HESINDE ZUM@..." */
	}
}

/**
 * \brief   check if you can pay the price with an item
 *
 * \param   price       price you want to pay
 * \return              -2 = enough money, -1 no item found or the item_id
 *
 * The item must be worth at least the price (base price, not broken), heroes in slots 0..5.
 */
/* should be static */
signed int academy_get_equal_item(const signed int price)
{
	signed int inv_slot;
	signed int retval;
	signed int i;
	int32_t p_money;
	struct struct_hero *hero;
	struct item_stats *p_item;

	retval = -2;
	p_money = get_party_money();

	if (price > p_money) {

		retval = -1;
		hero = get_hero(0);
		for (i = 0; i < 6; i++, hero++) {

			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !hero->flags.dead)
			{
				for (inv_slot = 0; inv_slot < NR_HERO_INVENTORY_SLOTS; inv_slot++) {

					if ((hero->inventory[inv_slot].item_id != ITEM_ID_NONE) && !hero->inventory[inv_slot].flags.broken)
						/* remark: armor with degraded RS is accepted */
					{
						p_item = &g_itemsdat[hero->inventory[inv_slot].item_id];

						if (p_item->price * p_item->price_unit >= price)
						{
							retval = hero->inventory[inv_slot].item_id;
							break;
						}
					}
				}
			}

			if (retval != -1) break;
		}
	}

	return retval;
}
