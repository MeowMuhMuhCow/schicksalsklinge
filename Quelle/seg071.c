/*
 * seg071 - Phexcaer: Gebaeude (Teil 2) und Alrik Derondan
 *
 * Inhalt:    PHX_drogentempel(): Tempel des "neuen Gottes"; nachmittags verteilt ein
 *            Tempeldiener die Droge (gs_drug_timer), wer sie schon kennt, zahlt 20 D je Held;
 *            sonst trifft man Ektor Gremob (Spende, Einladung in seine Villa, nach der
 *            Einladung Kampf). PHX_phextempel(): Gespraech mit der Geweihten, danach der
 *            normale Tempel (typeindex 59). PHX_alrik_derondan(): Gespraech mit Alrik in der
 *            Taverne, Hinweis auf Hyggelik fuer 5 D oder mit Charisma.
 * Aufrufer:  do_special_buildings() (seg066); PHX_alrik_derondan() aus
 *            tavern_quest_infos() (seg073), also im Gespraech mit dem Wirt.
 * Daten:     keine eigenen.
 * Besonders: gs_ptemple_closed wird nie gesetzt, Text 60 ("DIE GEWEIHTE NICHT DA") ist
 *            unerreichbar. gs_alrik_derondan_insulted wird nur gesetzt, nie gelesen.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg071.c
 */
#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg004.h"
#include "seg026.h"
#include "seg027.h"
#include "seg028.h"
#include "seg032.h"
#include "seg047.h"
#include "seg061.h"
#include "seg097.h"

/**
 * \brief   the temple of the "new god": 15-19 o'clock a temple servant hands out the drug,
 *          otherwise the high priest Ektor Gremob is there
 *
 * With gs_drug_timer running the party pays automatically 20 D (servant) or 25 D (Gremob) per hero.
 */
void PHX_drogentempel(void)
{
	signed int answer;
	signed int nr_heroes;
	int32_t money;

	load_ani(3);

	init_ani(0);

	money = get_party_money();

	if ((gs_day_timer >= HOURS(15)) && (gs_day_timer <= HOURS(19))) {

		/* load the picture of the TEMPLE SERVANT */

		load_in_head(6);

		nr_heroes = count_heroes_in_group();

		if (gs_drug_timer && (nr_heroes * 2000 <= money)) {

			GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(15), get_tx2(14), 0); /* "TEMPELDIENER" / "<DER MANN NICKT EUCH ERKENNEND ZU UND GIBT..." */

			money -= nr_heroes * 2000;
			set_party_money(money);

		} else {

			do {
				answer = GUI_dialogbox((unsigned char*)g_dtp2,
						get_tx2(15), get_tx2(0), /* "TEMPELDIENER" / "<DER TEMPEL BESTEHT IM WESENTLICHEN AUS EI..." */
						3,
						get_tx2(1), /* "<DANN KANN DAS ZEUG JA AUCH NICHTS TAUGEN.>" */
						get_tx2(2), /* "<JA DANN, IMMER HER DAMIT!>" */
						get_tx2(3)); /* "<SAGT, WER IST DIESER ÜBERAUS GROSSZÜGIGE GOTT?>" */
			} while (answer == -1);

			if (answer == 1) {

				do {
					answer = GUI_dialogbox((unsigned char*)g_dtp2,
							get_tx2(15), get_tx2(4), /* "TEMPELDIENER" / "<DA IRRT IHR EUCH GANZ GEWALTIG, MEINE FRE..." */
							3,
							get_tx2(5), /* "<VERSUCHT EUREN TRICK WOANDERS.>" */
							get_tx2(6), /* "<VIELEN DANK, ABER DAS HABEN WIR SCHON.>" */
							get_tx2(7)); /* "<JA DANN PROBIEREN WIR DOCH MAL.>" */
				} while (answer == -1);

				if (answer == 3) {

					GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(15), get_tx2(8), 0); /* "TEMPELDIENER" / "<BITTE SEHR. IHR WERDET ES NICHT BEREUEN.>" */

					gs_drug_timer = 10;
				}

			} else if (answer == 2) {

				GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(15), get_tx2(8), 0); /* "TEMPELDIENER" / "<BITTE SEHR. IHR WERDET ES NICHT BEREUEN.>" */

				gs_drug_timer = 10;
			} else {

				do {
					answer = GUI_dialogbox((unsigned char*)g_dtp2,
							get_tx2(15), get_tx2(9), /* "TEMPELDIENER" / "<ES IST DER GÜLDENE, DER EINES TAGES MEINE..." */
							3,
							get_tx2(10), /* "<WIR WOLLEN DOCH ETWAS VON DIESEM BROT.>" */
							get_tx2(11), /* "<GEBT MIR ETWAS, ES KANN JA NICHT SCHADEN.>" */
							get_tx2(12)); /* "<ES IST UND BLEIBT BLASPHEMIE!>" */
				} while (answer == -1);

				if (answer == 3) {

					GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(15), get_tx2(13), 0); /* "TEMPELDIENER" / "<WENN DAS EURE AUFFASSUNG IST, GEHT IHR JE..." */

				} else {

					GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(15), get_tx2(8), 0); /* "TEMPELDIENER" / "<BITTE SEHR. IHR WERDET ES NICHT BEREUEN.>" */

					gs_drug_timer = 10;
				}
			}
		}
	} else {

		/* load the picture of EKTOR GREMOB */
		load_in_head(8);

		nr_heroes = count_heroes_in_group();

		if (gs_gremob_invited) {

			GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(16), get_tx2(29), 0); /* "EKTOR GREMOB" / "<DAS SIND SIE, JUNGS! MACHT SIE EIN!> BRÜL..." */

			do_fight(FIGHT_ID_PHEX3);
		} else {

			if (gs_drug_timer && (nr_heroes * 2500 <= money)) {

				GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(16), get_tx2(28), 0); /* "EKTOR GREMOB" / "<AHA, DIE NEUEN WIEDER. ICH KANN MIR SCHON..." */

				money -= nr_heroes * 2500;
				set_party_money(money);
			} else {

				do {
					answer = GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(16), get_tx2(17), /* "EKTOR GREMOB" / "IN DER GROSSEN, LEEREN HALLE MIT DEM OBELI..." */
							2, get_tx2(19), get_tx2(30)); /* "<JA, BITTE!>" / "<NEIN, DANKE!>" */

				} while (answer == -1);

				if (answer == 1) {

					do {
						answer = GUI_dialogbox((unsigned char*)g_dtp2,
							get_tx2(16), get_tx2(18), 3, /* "EKTOR GREMOB" / "ALS IHR DAS LEUCHTEN DIESER AUGEN SEHT, VE..." */
							get_tx2(19), get_tx2(20), get_tx2(21)); /* "<JA, BITTE!>" / "<ICH FLEHE EUCH AN, GEBT MIR..." / "<JA! JA! JA!>" */

					} while (answer == -1);

					GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(16), get_tx2(26), 0); /* "EKTOR GREMOB" / "<EINE ANGEMESSENE SPENDE WÄREN ZWANZIG DUK..." */

					if (nr_heroes * 2000 <= money) {

						if (GUI_bool(get_tx2(31))) { /* "WOLLT IHR SPENDEN?" */
							money -= nr_heroes * 2000;
							set_party_money(money);
						}
					}
				} else {
					do {
						answer = GUI_dialogbox((unsigned char*)g_dtp2,
							get_tx2(16), get_tx2(22), 3, /* "EKTOR GREMOB" / "ALS IHR DIESEM MANN IN DIE AUGEN SEHT, WIS..." */
							get_tx2(23), get_tx2(24), get_tx2(25)); /* "<VERGISS ES! SAG UNS, WAS DA..." / "<NA JA, MAN KANN ES JA MAL PR..." / "<NA GUT, IN DER GÖTTER NAMEN.>" */

					} while (answer == -1);

					if (answer == 1) {
						GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(16), get_tx2(27), 0); /* "EKTOR GREMOB" / "DER MANN SIEHT SICH UM UND FLÜSTERT@<NICHT..." */
						gs_gremob_invited = 1;
					} else {
						GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(16), get_tx2(8), 0); /* "EKTOR GREMOB" / "<BITTE SEHR. IHR WERDET ES NICHT BEREUEN.>" */
						gs_drug_timer = 10;
					}
				}
			}
		}
	}

	disable_ani();
}

/**
 * \brief   the Phextemple of Phexcaer
 *
 *          This function shows how to enter a temple manually.
 *          Also there is some code, which will never be executed,
 *          because PTEMPLE_CLOSED is initially 0 and never changes.
 *          (Probably it should be set after text 56 "VERSCHWINDET HIER...", then text 60
 *          "OFFENSICHTLICH IST DIE GEWEIHTE NICHT DA" would appear.)
 *          REMARK: gs_town_typeindex stays 59 after do_temple().
 */
void PHX_phextempel(void)
{
	signed int answer;		/* the selected answer from the dialogs */
	signed int tw_bak;		/* saving the width of the textboxes */
	signed int loc_bak;		/* saving the type of the old location (gs_town_loc_type) */

	tw_bak = g_textbox_width;

	/* this variable is never != 0 */
	if (gs_ptemple_closed) {

		/* the cleric is gone */
		GUI_output(get_tx2(60)); /* "DER BETRAUM IST VESCHLOSSEN, KEIN LICHT IST ZU SEHEN UND ZU HÖREN I..." */

	} else {

		/* load the picture of THE FEMALE CLERIC */

		load_in_head(12);

		/* show introduction text */
		GUI_output(get_tx2(33)); /* "DER PHEXTEMPEL IST GRÖSSER ALS ALLE ANDEREN, DIE IHR JE GESEHEN HAB..." */

		do {
			answer = GUI_dialogbox((unsigned char*)g_dtp2,
					get_tx2(32), get_tx2(34), /* "EINE GEWEIHTE" / "GANZ AM ENDE DES@HAUPTGEBÄUDES SCHLIESSLIC..." */
					3,
					get_tx2(35), /* "<MIT EUCH WOHL NICHT, WIE ES HIER AUSSIEHT.>" */
					get_tx2(36), /* "<JA, WENN MAN IHN ERST GEFUNDEN HAT.>" */
					get_tx2(37)); /* "<UND MÖGE ER SICH GROSSZÜGIG ZEIGEN.>" */
		} while (answer == -1);

		if (answer == 1) {

			do {
				answer = GUI_dialogbox((unsigned char*)g_dtp2,
						get_tx2(32), get_tx2(38), /* "EINE GEWEIHTE" / "<IHR IRRT EUCH, FREMDE. IN DER TAT HAT MIC..." */
						3,
						get_tx2(39), /* "<NA DANN VIEL SPASS.>" */
						get_tx2(40), /* "<DAS GLAUBE ICH EUCH SOFORT.>" */
						get_tx2(41)); /* "<MÖGEN DIE GÖTTER EUREM VORHABEN GEWOGEN SEIN.>" */
			} while (answer == -1);

			if (answer == 1) {

				GUI_dialogbox((unsigned char*)g_dtp2,
						get_tx2(32), get_tx2(50), 0); /* "EINE GEWEIHTE" / "<IHR SEID HIER FEHL AM PLATZ. FÜR SOLCHE W..." */
			} else {

				/* enter the temple */

				GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(32), get_tx2(51), 0); /* "EINE GEWEIHTE" / "<DANN SOLLTET IHR EURER HOFFNUNG MIT EINER..." */

				gs_town_typeindex = 59;

				g_textbox_width = 3;

				loc_bak = gs_town_loc_type;

				gs_town_loc_type = LOCTYPE_TEMPLE;

				do_temple();

				gs_town_loc_type = loc_bak;

				g_textbox_width = tw_bak;
			}

		} else if (answer == 2) {

			do {
				answer = GUI_dialogbox((unsigned char*)g_dtp2,
						get_tx2(32), get_tx2(42), /* "EINE GEWEIHTE" / "DIE FRAU LÄCHELT EIN WENIG.@<IN DER TAT LI..." */
						3,
						get_tx2(43), /* "<LIEBT ER AUCH UNMÖGLICHE AUFGABEN?>" */
						get_tx2(44), /* "<HOFFENTLICH BRINGT IHR VIEL ZEIT MIT.>" */
						get_tx2(45)); /* "<WIR SIND ÜBERZEUGT, DASS EUCH DIES GELINGT.>" */
			} while (answer == -1);

			if (answer == 1) {

				GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(32), get_tx2(50), 0); /* "EINE GEWEIHTE" / "<IHR SEID HIER FEHL AM PLATZ. FÜR SOLCHE W..." */

			} else if (answer == 2) {

				do {
					answer = GUI_dialogbox((unsigned char*)g_dtp2,
							get_tx2(32), get_tx2(52), /* "EINE GEWEIHTE" / "<WAS SPIELT ZEIT SCHON FÜR EINE ROLLE? HIE..." */
							3,
							get_tx2(53), /* "<WÄR JA NICHT SCHADE DRUM.>" */
							get_tx2(54), /* "<IHR SEID JA FANATISCH!>" */
							get_tx2(55)); /* "<WAS GÄBE ICH FÜR SOLCHEN GLAUBEN!>" */
				} while (answer == -1);

				if (answer == 2) {

					GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(32), get_tx2(56), 0); /* "EINE GEWEIHTE" / "<DAS WERDET IHR BEREUEN, IHR SCHURKEN! ICH..." */
				}

			} else {

				/* enter the temple */

				GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(32), get_tx2(51), 0); /* "EINE GEWEIHTE" / "<DANN SOLLTET IHR EURER HOFFNUNG MIT EINER..." */

				gs_town_typeindex = 59;

				g_textbox_width = 3;

				loc_bak = gs_town_loc_type;

				gs_town_loc_type = LOCTYPE_TEMPLE;

				do_temple();

				gs_town_loc_type = loc_bak;

				g_textbox_width = tw_bak;
			}

		} else {

			do {
				answer = GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(32), get_tx2(46), /* "EINE GEWEIHTE" / "<DAS HAT ER BEREITS.>" */
						3, get_tx2(47), get_tx2(48), get_tx2(49)); /* "<DAVON SIEHT MAN ABER NICHTS.>" / "<DAS WAR ABER AUCH NÖTIG.>" / "<IHR SEID BENEIDENSWERT.>" */
			} while (answer == -1);

			if (answer == 1) {

				GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(32), get_tx2(57), 0); /* "EINE GEWEIHTE" / "<ZUGEGEBEN, DAS GEBÄUDE IST IN EINEM BEDAU..." */

			} else if (answer == 2) {

				GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(32), get_tx2(58), 0); /* "EINE GEWEIHTE" / "<IN DER TAT. SO IST DENN SEINE ERSCHEINUNG..." */

			} else if (answer == 3) {

				GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(32), get_tx2(59), 0); /* "EINE GEWEIHTE" / "<DA HABT IHR RECHT. ABER DIE WENIGSTEN BEM..." */
			}

			/* enter the temple */

			gs_town_typeindex = 59;

			g_textbox_width = 3;

			loc_bak = gs_town_loc_type;

			gs_town_loc_type = LOCTYPE_TEMPLE;

			do_temple();

			gs_town_loc_type = loc_bak;

			g_textbox_width = tw_bak;
		}
	}
}

/**
 * \brief   the dialog with ALRIK DERONDAN (in a tavern, if gs_alrik_derondan is set)
 */
void PHX_alrik_derondan(void)
{
	signed int answer;
	signed int topic = 0; /* 1 = ask for Hyggelik (price), 2 = "who are you", 3 = ask for a hint, 0 = end */
	int32_t money;

	load_tx2(ARCHIVE_FILE_PHEX2_LTX);

	/* load the picture of ALRIK DERONDAN */
	load_in_head(3);

	do {
		answer = GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(61), get_tx2(62), /* "ALRIK DERONDAN" / "AN EUREN TISCH TRITT EIN MANN, DER EUCH VO..." */
				3, get_tx2(63), get_tx2(64), get_tx2(65)); /* "<DAS MUSS EIN IRRTUM SEIN.>" / "<SCHON MÖGLICH. WORUM GEHT ES?>" / "<JA! BITTE NEHMT PLATZ UND TR..." */

	} while (answer == -1);

	if (answer == 1) {

		do {
			answer = GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(61), get_tx2(66), /* "ALRIK DERONDAN" / "<VERZEIHT, ICH SCHEINE EUCH VERWECHSELT ZU..." */
					3, get_tx2(67), get_tx2(68), get_tx2(69)); /* "<DAS SCHEINT UNS AUCH SO.>" / "<DAS KANN JA MAL VORKOMMEN.>" / "<NEIN, IHR SEID SCHON RICHTIG..." */

		} while (answer == -1);

		if (answer == 3) {

			do {
				answer = GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(61), get_tx2(74), /* "ALRIK DERONDAN" / "<DAS HÖRT MAN GERNE. ALSO, WOMIT KANN ICH..." */
						3, get_tx2(75), get_tx2(76), get_tx2(77)); /* "<IHR KÖNNTET UNS EUER GALGENG..." / "<WO FINDEN WIR HYGGELIK?>" / "<KÖNNT IHR UNS EINEN HINWEIS..." */

			} while (answer == -1);

			if (answer == 1) {

				GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(61), get_tx2(81), 0); /* "ALRIK DERONDAN" / "<IHR SEID GANZ SCHÖN MUTIG, IN MEINEM REVI..." */

				/* REMARK: gs_alrik_derondan_insulted is never read */
				gs_alrik_derondan_insulted = 1;

			} else if (answer == 2) {
				topic = 1;
			} else {
				topic = 3;
			}
		} else {
			GUI_dialogbox((unsigned char*)g_dtp2,
					get_tx2(61), get_tx2(78), 0); /* "ALRIK DERONDAN" / "DER MANN SCHÜTTELT DEN KOPF, STEHT AUF UND..." */
		}

	} else if (answer == 2) {

		do {
			answer = GUI_dialogbox((unsigned char*)g_dtp2,
					get_tx2(61), get_tx2(70), /* "ALRIK DERONDAN" / "<ICH DACHTE, DAS SAGT IHR MIR?>" */
					3,
					get_tx2(71), /* "<WIE KÄMEN WIR DENN DAZU?>" */
					get_tx2(72), /* "<DAS BINDEN WIR DOCH NICHT JEDEM AUF DIE NASE.>" */
					get_tx2(73)); /* "<NATÜRLICH. WIR SUCHEN HYGGELIK.>" */
		} while (answer == -1);

		if (answer == 1) {

			GUI_dialogbox((unsigned char*)g_dtp2,
					get_tx2(61), get_tx2(78), 0); /* "ALRIK DERONDAN" / "DER MANN SCHÜTTELT DEN KOPF, STEHT AUF UND..." */

		} else if (answer == 2) {
			topic = 2;
		} else {
			topic = 1;
		}
	} else {
		do {
			answer = GUI_dialogbox((unsigned char*)g_dtp2,
					get_tx2(61), get_tx2(74), /* "ALRIK DERONDAN" / "<DAS HÖRT MAN GERNE. ALSO, WOMIT KANN ICH..." */
					3,
					get_tx2(75), /* "<IHR KÖNNTET UNS EUER GALGENGESICHT ERSPAREN.>" */
					get_tx2(76), /* "<WO FINDEN WIR HYGGELIK?>" */
					get_tx2(77)); /* "<KÖNNT IHR UNS EINEN HINWEIS GEBEN?>" */
		} while (answer == -1);

		if (answer == 1) {

			GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(61), get_tx2(81), 0); /* "ALRIK DERONDAN" / "<IHR SEID GANZ SCHÖN MUTIG, IN MEINEM REVI..." */

			gs_alrik_derondan_insulted = 1;

		} else if (answer == 2) {
			topic = 1;
		} else {
			topic = 3;
		}
	}

	if ((topic == 2) || (topic == 3)) {

		do {
			answer = GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(61), /* "ALRIK DERONDAN" */
					topic == 3 ? get_tx2(82) : get_tx2(79), /* "<KLAR KANN ICH DAS. DER PREIS HÄNGT VON DE..." / "<ICH BIN NICHT JEDER. ICH BIN DER, DEN IHR..." */
					3, get_tx2(84),	get_tx2(85), get_tx2(86)); /* "<DAS KANN JA JEDER BEHAUPTEN.>" / "<KÖNNT IHR DAS BEWEISEN?>" / "<SEI ES DRUM. WO FINDEN WIR H..." */
		} while (answer == -1);

		if (answer == 1) {

			GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(61), get_tx2(87), 0); /* "ALRIK DERONDAN" / "<ABER WARUM SOLLTE DAS JEMAND TUN? GLAUBT..." */

		} else if (answer == 2) {

			GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(61), get_tx2(88), 0); /* "ALRIK DERONDAN" / "<KÖNNTE ICH. HABE ICH ABER NICHT NÖTIG. PH..." */

		} else {

			topic = 1;
		}
	}

	if (topic == 1) {

		money = get_party_money();

		answer = money >= 500 ? 3 : 2;

		do {
			answer = GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(61), get_tx2(80), /* "ALRIK DERONDAN" / "<HYGGELIK? DER IST SCHON LANGE NICHT MEHR..." */
					answer, get_tx2(89), get_tx2(91), get_tx2(90)); /* "<DAS WÄRE SEHR NETT VON EUCH.>" / "<DAS WOLLEN WIR EUCH GERATEN..." / "<WIE WÄRE ES MIT FÜNF DUKATEN..." */

		} while (answer == -1);

		if (answer == 3) {

			money -= 500;
			set_party_money(money);

			GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(61), get_tx2(92), 0); /* "ALRIK DERONDAN" / "<WENN IHR MICH SO NETT BITTET: ICH HABE GE..." */

		} else if (answer == 2) {

			GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(61), get_tx2(78), 0); /* "ALRIK DERONDAN" / "DER MANN SCHÜTTELT DEN KOPF, STEHT AUF UND..." */

		} else {

			GUI_dialogbox((unsigned char*)g_dtp2, get_tx2(61), /* "ALRIK DERONDAN" */
				(test_attrib(get_first_hero_available_in_group(), ATTRIB_CH, 0) > 0) ? get_tx2(92) : get_tx2(78), 0); /* "<WENN IHR MICH SO NETT BITTET: ICH HABE GE..." / "DER MANN SCHÜTTELT DEN KOPF, STEHT AUF UND..." */
		}
	}

	/* reset the flag */
	gs_alrik_derondan = 0;

	load_tlk(ARCHIVE_FILE_TAVERN_TLK);
}
