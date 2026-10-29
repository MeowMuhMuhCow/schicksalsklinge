/*
 * seg070 - Phexcaer: Gebaeude (Teil 1)
 *
 * Inhalt:    Fuhrhof (keine Fahrt zum Einsiedlersee, Hinweis auf Alrik Derondan), Stadthaus
 *            (Archiv fuer 10 D), Spielhaus (Gluecksspiel mit Falschspiel-Proben, Auskunft
 *            ueber Alrik oder fuer 5 D ueber Hyggelik), Villa des Ektor Gremob (Kampf nach
 *            Einladung), Bordell (Verabredung, Nacht +30 Rahja-Ansehen), Apotheke
 *            (Wirselkraut fuer 10 D, Auskunft fuer 5 D) und der Heiler (oft geschlossen,
 *            sonst Behandlung ueber do_healer oder Auskunft).
 * Aufrufer:  do_special_buildings() (seg066).
 * Daten:     keine eigenen.
 * Besonders: Bekannt (Bright-Eyes): Stadthaus ruft GUI_radio nach ESC mit -1 Optionen auf;
 *            Spielhaus: Gewinn/Verlust-Bedingung vertauscht (nur per #ifdef markiert).
 *            Die Texte des Spielhauses sprechen von Stunden, es vergeht aber keine Zeit.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg070.c
 */
#include <stdio.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg004.h"
#include "seg007.h"
#include "seg026.h"
#include "seg027.h"
#include "seg032.h"
#include "seg047.h"
#include "seg053.h"
#include "seg097.h"
#include "seg103.h"
#include "seg105.h"

/**
 * \brief   carriage station: no ride to the Einsiedlersee; asking for Alrik Derondan sets gs_alrik_derondan
 */
void PHX_fuhrhaus(void)
{
	signed int answer;

	load_in_head(42);

	do {
		answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(0), 3, /* "AUF DEM FUHRHOF STEHEN DREI LEICHTE GESPANNE HERUM. EINE FRAU FETTE..." */
				get_tx2(1), get_tx2(2), get_tx2(3)); /* "<JA, ZUM EINSIEDLERSEE.>" / "<WIR WOLLTEN NUR ETWAS FRAGEN.>" / "<MIT EUCH GERNE, SCHÖNE FRAU.>" */

	} while (answer == -1);

	if (answer == 1) {

		GUI_dialog_na(0, get_tx2(4)); /* "<DAHIN FAHRE ICH NICHT. ES IST MIR NICHT GEHEUER DORT. SUCHT EUCH J..." */

	} else if (answer == 2) {

		do {
			answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(5), 3, /* "<DANN BEEILT EUCH, ICH HABE NICHT DEN GANZEN TAG ZEIT.>" */
					get_tx2(6), get_tx2(7), get_tx2(8)); /* "<WO IST DER EINSIEDLERSEE?>" / "<KENNT IHR HYGGELIK?>" / "<WO FINDEN WIR ALRIK DERONDAN?>" */

		} while (answer == -1);

		GUI_dialog_na(0, answer == 1 ? get_tx2(10): (answer == 2 ? get_tx2(11) : get_tx2(12))); /* "<EINIGE TAGESREISEN SÜDÖSTLIC..." / "<NIE GEHÖRT.>" / "<KEINE AHNUNG. ER IST IRGENDW..." */

		if (answer == 3) {

			/* You are now able to talk to Alrik */
			gs_alrik_derondan = 1;
		}

		GUI_dialog_na(0, get_tx2(13)); /* "<UND JETZT VERZIEHT EUCH, ICH HABE ZU TUN.>" */

	} else {

		GUI_dialog_na(0, get_tx2(9)); /* "<VERZIEHT EUCH, SCHWACHKOPF, BEVOR ICH DIE HUNDE LOSLASSE.>" */
	}
}

/**
 * \brief   stadthaus of Phexcaer: the town archive costs 10 D (without any further effect)
 */
void PHX_stadthaus(void)
{
	signed int answer;
	int32_t money;

	do {
		answer = GUI_radio(get_tx2(14), 3, get_tx2(15), get_tx2(16), get_tx2(17)); /* "DAS STADTHAUS, EIN GROSSER BA..." / "<NICHTS FÜR UNGUT, WAR NUR SO..." / "<WAS WISST IHR ÜBER HYGGELIK?>" / "<WIR MÖCHTEN DAS STADTARCHIV..." */

	} while (answer == -1);

	if (answer == 1) {

		GUI_output(get_tx2(18)); /* "IHR SEHT NUR NOCH, WIE DER SCHREIBER DEN KOPF SCHÜTTELT, ALS IHR DA..." */

	} else {

#ifdef M302de_ORIGINAL_BUGFIX
		signed int answer2;
#endif
		money = get_party_money();

		/* reuse answer as number of options */
		answer = money >= 1000 ? 3 : 1;

#ifdef M302de_ORIGINAL_BUGFIX

		do {
			answer2 = GUI_radio(get_tx2(19), (signed char)answer, get_tx2(21), get_tx2(20),	get_tx2(22)); /* "<FÜR NACHFORSCHUNGEN JEDER AR..." / "<KOMMT GAR NICHT IN FRAGE!>" / "<ABER SELBSTVERSTÄNDLICH.>" / "<ABER NUR ZÄHNEKNIRSCHEND.>" */

		} while (answer2 == -1);

		answer = answer2;

#else
		/* Original-Bug: pressing ESC calls GUI_radio() again with -1 options */
		do {
			answer = GUI_radio(get_tx2(19), answer, get_tx2(21), get_tx2(20), get_tx2(22)); /* "<FÜR NACHFORSCHUNGEN JEDER AR..." / "<KOMMT GAR NICHT IN FRAGE!>" / "<ABER SELBSTVERSTÄNDLICH.>" / "<ABER NUR ZÄHNEKNIRSCHEND.>" */

		} while (answer == -1);
#endif

		if (answer == 1) {

			GUI_output(get_tx2(23)); /* "<DANN AUF WIEDERSEHEN.>" */

		} else {

			/* the Party loses 10D */
			money -= 1000;

			set_party_money(money);

			GUI_output(get_tx2(24)); /* "<DANKE. WENN IHR JETZT BITTE DIESES FORMULAR AUSFÜLLEN WÜRDET? DEN@..." */
		}
	}
}

/**
 * \brief   spielhaus of Phexcaer: gambling (win or lose 1..20 D), information for 5 D
 */
void PHX_spielhaus(void)
{
	signed int answer;
	signed int num_passed;
	signed int pos; /* multi use: hero position, then the amount won or lost in D */
	int32_t money;
	struct struct_hero *hero;

	do {
		answer = GUI_radio(get_tx2(25), 3, get_tx2(26), get_tx2(27), get_tx2(28)); /* "DAS SPIELHAUS IST ERFÜLLT VON..." / "<EIGENTLICH WOLLTEN WIR SPIEL..." / "<WIR SIND AUF DER SUCHE NACH..." / "<WIR SUCHEN JEMANDEN, DER SIC..." */

	} while (answer == -1);

	if (answer == 1) {

		GUI_output(get_tx2(29)); /* "<DANN MACHT DAS DOCH EINFACH. DORT HINTEN AM TISCH WERDEN GERADE EI..." */

		hero = get_hero(0);

		for (pos = num_passed = answer = 0; pos <= 6; pos++, hero++) {

			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
				!hero->flags.dead && (test_talent(hero, TA_FALSCHSPIEL, 3) > 0))
			{
				/* Falschspiel +3 (handicap) */
				num_passed++;
			}
		}

		pos = random_schick(20);

		money = get_party_money();

		/* Original-Bug: the condition is inverted, the group loses if more than half of the
		 * heroes passed the Falschspiel test. The fix lets it lose if at most half passed.
		 * REMARK: texts 30/31 speak of some hours, but no time passes. */
#ifdef M302de_ORIGINAL_BUGFIX
		if ((count_heroes_in_group() >> 1) >= num_passed) {
#else
		if ((count_heroes_in_group() >> 1) < num_passed) {
#endif

			/* you lose: at least the half of your group have not passed the test (only with the fix) */

			sprintf(g_dtp2,	get_tx2(30), pos); /* "NACH EINIGEN STUNDEN HABT IHR %d DUKATEN VERLOREN. DABEI HATTET IHR..." */

			money -= pos * 100;

			if (money < 0) {
				money = 0;
			}
		} else {

			/* you win */
			sprintf(g_dtp2,	get_tx2(31), pos); /* "NACH EINIGEN STUNDEN HABT IHR %d DUKATEN GEWONNEN. DABEI HATTET IHR..." */

			money += pos * 100;
		}

		set_party_money(money);

		GUI_output(g_dtp2);

	} else if (answer == 2) {

		GUI_output(get_tx2(32)); /* "<DER TREIBT SICH IRGENDWO IN DER STADT HERUM. AN EURER STELLE WÜRDE..." */

		/* You are now able to talk to Alrik */
		gs_alrik_derondan = 1;

	} else {

		money = get_party_money();

		pos = money >= 500 ? 3 : 2;

		do {
			answer = GUI_radio(get_tx2(33), (signed char)pos, get_tx2(34), get_tx2(36), get_tx2(35)); /* "<ICH LEBE SCHON EINIGE JAHRE..." / "<JA, DA HABT IHR RECHT.>" / "<NEIN! JETZT KAUFEN WIR KEINE..." / "<KÖNNTEN EUCH FÜNF DUKATEN ÜB..." */

		} while (answer == -1);

		GUI_output(answer == 3 ? get_tx2(38): get_tx2(37)); /* "<DAS HABEN SIE SCHON, EDLE HERRSCHAFTEN. W..." / "<JA, ICH MUSS DANN MAL WIEDER> SAGT DER MA..." */

		if (answer == 3) {

			money -= 500;

			if (money < 0) {
				money = 0;
			}
		}

		set_party_money(money);
	}
}

/**
 * \brief   the villa of Ektor Gremob
 */
void PHX_villa_gremob(void)
{
	GUI_output(!gs_gremob_invited ? get_tx2(40) : get_tx2(39)); /* "DIE PROTZIGE VILLA DES EKTOR GREMOB WIRD V..." / "VOR DER PRACHTVOLLEN VILLA STEHEN VIER BEW..." */

	if (gs_gremob_invited) {
		do_fight(FIGHT_ID_PHEX24);
	}
}

/**
 * \brief   the brothel: a date (gs_harlot_date) or a night for free (+30 estimation of Rahja)
 */
void PHX_bordell(void)
{
	signed int answer;

	load_ani(24);

	init_ani(0);

	do {
		answer = GUI_radio(get_tx2(41), 3, get_tx2(42), get_tx2(43), get_tx2(44)); /* "DAS ROSAFARBENE GEBÄUDE VERMI..." / "<WEDER NOCH. WIR SUCHEN HYGGE..." / "<OCH, WIR NEHMEN BEIDES.>" / "<WIE BITTE? HALTET IHR UNS FÜ..." */

	} while (answer == -1);

	if (answer == 1) {

		do {
			answer = GUI_radio(get_tx2(45), 3, get_tx2(46), get_tx2(47), get_tx2(48)); /* "<NICHT SO LAUT! DIE CHEFIN HÖ..." / "<WIR KÖNNEN SEHR GROSSZÜGUG S..." / "<ES SOLLTE REICHEN.>" / "<NEIN, SO HABEN WIR NICHT GEW..." */

		} while (answer == -1);

		GUI_input(answer == 3 ? get_tx2(51) : get_tx2(52), 0); /* "<NA, DANN EBEN NICHT!>" / "<BIS SPÄTER DANN, IHR SÜSSEN. KUSCHT EUCH..." */

		if ((answer == 1) || (answer == 2)) {
			gs_harlot_date = 1;
		}

	} else if (answer == 2) {

		/* increase RAHJA's estimation */
		gs_gods_estimation[GOD_ID_RAHJA] += 30L;

		timewarp_until_time_of_day(HOURS(7));

		GUI_input(get_tx2(49), 0); /* "<DANN FOLGT MIR BITTE.> NACH EINER AUSGESPROCHEN ANGENEHMEN NACHT S..." */

	} else if (answer == 3) {	/* this check is bogus: answer is always 3 here */

		GUI_input(get_tx2(50), 0); /* "<MEINE GÜTE, SEID IHR SPIESSIG. ICH GLAUBE, DANN GEHT IHR SCHNELL W..." */
	}

	disable_ani();
}

/**
 * \brief   the pharmacy: Wirselkraut for 10 D, information about Hyggelik for 5 D
 */
void PHX_apotheke(void)
{
	signed int answer;
	signed int options;
	int32_t money;

	load_ani(23);

	init_ani(0);

	do {
		answer = GUI_radio(get_tx2(53), 3, get_tx2(54),	get_tx2(55), get_tx2(56)); /* "DER KLEINE VERKAUFSRAUM RIECH..." / "<WIR SUCHEN EINIGE HEILKRÄUTER.>" / "<KENNT IHR DEN NAMEN HYGGELIK?>" / "<NICHTS, WENN EUER LADEN IST,..." */

	} while (answer == -1);

	if (answer == 1) {

		money = get_party_money();

		options = money >= 1000 ? 3 : 2;


		do {
			answer = GUI_radio(get_tx2(57), (signed char)options, get_tx2(62), get_tx2(61), get_tx2(60)); /* "<HEILKRÄUTER? DA MUSS ICH SUC..." / "<DAS IST JA WUCHER! BEHALTET..." / "<NICHT MEHR GANZ FRISCH? DAS..." / "<DAS NENNE ICH EIN FAIRES ANG..." */

		} while (answer == -1);

		if (answer == 3) {

			money -= 1000;

			set_party_money(money);

			give_new_item_to_group(ITEM_ID_WIRSELKRAUT, 1, 1);
		} else {
			GUI_input(get_tx2(69), 0); /* "DER MANN ZUCKT LEDIGLICH MIT DEN ACHSELN UND VERSCHWINDET WIEDER IM..." */
		}

	} else if (answer == 2) {

		money = get_party_money();

		answer = money >= 500 ? 3 : 2;

		do {
			answer = GUI_radio(get_tx2(58), (signed char)answer, get_tx2(63), get_tx2(65), get_tx2(64)); /* "<HYGGELIK? DOCH, DEN KENNE IC..." / "<NUN STRENGT EUCH DOCH AN.>" / "<WOLLT IHR UNS DAS GELD AUS D..." / "<VIELLEICHT HELFEN EUCH FÜNF..." */

		} while (answer == -1);


		if (answer == 3) {

			money -= 500;

			set_party_money(money);

			GUI_input(get_tx2(71), 0); /* "<VIELEN DANK. JA, DIE HELFEN IN DER TAT WEITER. ALSO, HYGGELIK IST..." */
		} else {

			GUI_input(answer == 1 ? get_tx2(70) : get_tx2(69), 0); /* "<ICH KOMME BEIM BESTEN WILLEN NICHT DRAUF...." / "DER MANN ZUCKT LEDIGLICH MIT DEN ACHSELN U..." */

		}
	} else {

		do {
			answer = GUI_radio(get_tx2(59), 3, get_tx2(66), get_tx2(67), get_tx2(68)); /* "<VIEL BESSER.>" / "<DAS GLAUBE ICH NICHT.>" / "<WIR LASSEN UNS ÜBERRASCHEN.>" / "<DANN HABT IHR BESTIMMT AUCH..." */

		} while (answer == -1);

		if (answer == 1) {

			GUI_input(get_tx2(69), 0); /* "DER MANN ZUCKT LEDIGLICH MIT DEN ACHSELN UND VERSCHWINDET WIEDER IM..." */

		} else if (answer == 2) {

			do {
				answer = GUI_radio(get_tx2(72), 3, get_tx2(55), get_tx2(73), get_tx2(74)); /* "<DAZU MÜSSTET IHR MIR ALLERDI..." / "<KENNT IHR DEN NAMEN HYGGELIK?>" / "<EINFACH NUR RAUS HIER.>" / "<GIFTE. SCHÖNE, WIRKSAME GIFTE.>" */

			} while (answer == -1);

			if (answer == 1) {
				money = get_party_money();

				answer = money >= 500 ? 3 : 2;

				do {
					answer = GUI_radio(get_tx2(58), (signed char)answer, get_tx2(63), get_tx2(65), get_tx2(64)); /* "<HYGGELIK? DOCH, DEN KENNE IC..." / "<NUN STRENGT EUCH DOCH AN.>" / "<WOLLT IHR UNS DAS GELD AUS D..." / "<VIELLEICHT HELFEN EUCH FÜNF..." */

				} while (answer == -1);

				if (answer == 3) {

					money -= 500;

					set_party_money(money);

					GUI_input(get_tx2(71), 0); /* "<VIELEN DANK. JA, DIE HELFEN IN DER TAT WEITER. ALSO, HYGGELIK IST..." */
				} else {
					GUI_input(answer == 1 ? get_tx2(70) : get_tx2(69), 0); /* "<ICH KOMME BEIM BESTEN WILLEN NICHT DRAUF...." / "DER MANN ZUCKT LEDIGLICH MIT DEN ACHSELN U..." */
				}
			} else {
				GUI_input(get_tx2(75), 0); /* "<WAS DENKT IHR DENN VON MIR? MACHT, DASS IHR RAUSKOMMT!>" */
			}

		} else {

			money = get_party_money();

			answer = money >= 1000 ? 3 : 2;

			do {
				answer = GUI_radio(get_tx2(57), (signed char)answer, get_tx2(62), get_tx2(61), get_tx2(60)); /* "<HEILKRÄUTER? DA MUSS ICH SUC..." / "<DAS IST JA WUCHER! BEHALTET..." / "<NICHT MEHR GANZ FRISCH? DAS..." / "<DAS NENNE ICH EIN FAIRES ANG..." */

			} while (answer == -1);

			if (answer == 3) {

				money -= 1000;

				set_party_money(money);

				give_new_item_to_group(ITEM_ID_WIRSELKRAUT, 1, 1);
			} else {
				GUI_input(get_tx2(69), 0); /* "DER MANN ZUCKT LEDIGLICH MIT DEN ACHSELN UND VERSCHWINDET WIEDER IM..." */
			}
		}
	}

	disable_ani();
}

/**
 * \brief   the healer: 70% closed, 15% nobody opens, 15% open (do_healer() as healer 13,
 *          an examination for 2 S, information about Hyggelik for 1 S or 5 D)
 */
void PHX_healer(void)
{
	signed int answer;
	int32_t money;

	answer = random_schick(100);

	if (answer <= 70) {

		GUI_output(get_tx2(76)); /* "EIN SCHILD AN DER TÜR WEIST DIESES GEBÄUDE EINDEUTIG ALS DIE PRAXIS..." */

	} else if (answer <= 85) {

		GUI_output(get_tx2(77)); /* "ALS IHR AN DER TÜR KLOPFT, RÜHRT SICH NICHTS. ERST NACH EINIGEN MIN..." */

	} else {

		load_ani(23);

		init_ani(0);

		do {
			answer = GUI_radio(get_tx2(78), 3, get_tx2(79), get_tx2(80), get_tx2(81)); /* "DIE EINGANGSTÜR STEHT OFFEN...." / "<SIND WIR ABER.>" / "<WIR SUCHEN AUCH KEINEN MEDIZ..." / "<IST JA NICHT SEHR SAUBER HIER.>" */

		} while (answer == -1);

		if (answer == 1) {

			GUI_output(get_tx2(82)); /* "<NA, DANN WOLLEN WIR DOCH MAL SEHEN.>" */

			/* enter the healer */
			gs_town_typeindex = 13;
			gs_town_locdata = (15);

			do_healer();

		} else if (answer == 3) {

			GUI_output(get_tx2(84)); /* "<IHR KÖNNT JA GEHEN, WENN EUCH ETWAS NICHT PASST. DAS SCHEINT MIR@Ü..." */

		} else {
			/* answer == 2 */
			do {
				answer = GUI_radio(get_tx2(83), 3, get_tx2(85), get_tx2(86), get_tx2(87)); /* "<WAS BRINGT EUCH DANN HIERHER?>" / "<NUR DIE NEUGIERDE.>" / "<WÜRDET IHR UNS VORSICHTSHALB..." / "<WIR SUCHEN HYGGELIK. KENNT I..." */

			} while (answer == -1);

			if (answer == 1) {

				do {
					answer = GUI_radio(get_tx2(88), 3, get_tx2(89), get_tx2(90), get_tx2(91)); /* "<HIER GIBT ES NICHTS ZU GAFFEN.>" / "<DAS HABEN WIR UNS GEDACHT.>" / "<WIR WOLLTEN AUCH NUR NACH JE..." / "<SEID IHR EUCH DAS GANZ SICHER?>" */

				} while (answer == -1);

			} else if (answer == 2) {

				money = get_party_money();

				if (money >= 200) {

					GUI_output(get_tx2(92)); /* "<NA GUT, DANN WOLLEN WIR MAL SEHEN. WÜRDET IHR BITTE DEN OBERKÖRPER..." */

					money -= 200;

					set_party_money(money);
				} else {
					GUI_output(get_tx2(99)); /* "<OHNE KRANKENSCHEIN GEHT DA GAR NICHTS...>" */
				}
			} else {

				money = get_party_money();

				answer = money >= 500 ? 3 : (money >= 10 ? 2 : 1);

				do {
					answer = GUI_radio(get_tx2(93), (signed char)answer, get_tx2(96), get_tx2(94), get_tx2(95)); /* "<JA, DEN KENNE ICH. WAS IST E..." / "<WIE BITTE? WIR KOMMEN AUCH O..." / "<SAGEN WIR, EINEN SILBERTALER?>" / "<GENÜGEN EUCH FÜNF DUKATEN?>" */

				} while (answer == -1);

				if (answer == 2) {

					money -= 10;

					set_party_money(money);

					GUI_output(get_tx2(97)); /* "<HYGGELIK IST TOT. ABER ICH WEISS ZUFÄLLIG, WER SEINE HINTERLASSENS..." */

				} else if (answer == 3) {

					money -= 500;

					set_party_money(money);

					GUI_output(get_tx2(98)); /* "<HM. SOWEIT ICH WEISS, TREIBT ER SICH AM EINSIEDLERSEE EINIGE TAGE@..." */
				}
			}
		}
	}

	disable_ani();
}
