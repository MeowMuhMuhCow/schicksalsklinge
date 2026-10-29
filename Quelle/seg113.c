/*
 * seg113 - Reiseereignisse 5/10 (Ereignisse 80-89, 97-109) und hero_disappear()
 *
 * Inhalt:    Ereignisse zwischen Orkanger/Clanegh/Tyldon, Oberorken/Felsteyn/Einsiedlersee,
 *            Rybon/Thoss, Skjal/Orvil und Skelellen/Phexcaer: Tatzelwurm-Spur, Furten durch den
 *            Hjaldorval, Riesenlindwurm, Huetten, Klamm (verschwundene Helden landen im Tempel 33),
 *            Harpyien, Nebeltal, schmaler Grat mit Hoehenangst (Bannbaladin), Felswand und Zugang zur
 *            Orkbehausung. hero_disappear() entfernt einen Helden oder NSC endgueltig oder in einen Tempel.
 * Aufrufer:  TRV_event() (seg109); hero_disappear() auch aus seg002, seg032, seg072, seg078, seg080,
 *            seg086, seg090, seg114, seg117 und seg118.
 * Besonders: Original-Bug 53, tevent_098 (hero), tevent_099/101 (Flucht), tevent_103 (Funktionsadresse
 *            als Held). Neu: hero_disappear() benutzt fuer den NSC npc_id + 1 statt npc_id (falsche
 *            NSC-Datei, falscher Timer, bei Erwo ausserhalb von gs_npc_timers); tevent_098 uebergibt beim
 *            Seil-Weg die Optionszahl statt der Heldenposition; tevent_080 liest bei bekannter Spur
 *            hero->alias ueber einen NULL-Zeiger.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg113.c
 */

#include <stdio.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg004.h"
#include "seg007.h"
#include "seg025.h"
#include "seg026.h"
#include "seg027.h"
#include "seg028.h"
#include "seg029.h"
#include "seg032.h"
#include "seg047.h"
#include "seg097.h"
#include "seg098.h"
#include "seg103.h"
#include "seg105.h"
#include "seg109.h"
#include "seg113.h"

/* The Tatzelwurm */
/* Orkanger <-> Clanegh: Nahrungsplatz; der Jaeger (gs_main_acting_hero aus replenish_stocks()) findet
 * evtl. die Spur. gs_tevent080_tatzelwurm: 0 = unbekannt, 1 = Spur bekannt, 2 = Hoehle gefunden */
void tevent_080(void)
{
	signed int answer;
	struct struct_hero *hero;

	hero = get_first_hero_available_in_group();

	if ((!gs_tevent080_flag && (test_talent(hero, TA_WILDNISLEBEN, 4) > 0)) || gs_tevent080_flag != 0)
	{
		gs_tevent080_flag = 1;
		gs_main_acting_hero = NULL;

		TRV_found_replenish_place(0);

		hero = (struct struct_hero*)gs_main_acting_hero;

		/* Fehler im Original: bei gs_tevent080_tatzelwurm == 1 wird hero nicht geprueft. Wurde nicht
		 * gejagt (Rast abgelehnt, kein Held gewaehlt, schon gejagt), ist hero NULL und der Text liest
		 * hero->alias aus Adresse 0000:xxxx (Name aus der Interrupt-Vektortabelle). */
		if ((hero && !gs_tevent080_tatzelwurm && test_talent(hero, TA_FAEHRTENSUCHEN, 5) > 0) || gs_tevent080_tatzelwurm == 1)
		{
			gs_tevent080_tatzelwurm = 1;

			sprintf(g_dtp2, get_tx2(87), hero->alias); /* "WÄHREND DER JAGD SIND %s EINIGE SELTSAME SPUREN AUFGEFALLEN: UNGEWÖ..." */

			do {
				answer = GUI_radio(g_dtp2, 2, get_tx2(88), get_tx2(89)); /* "DIE NEUE SPUR VERFOLGEN?" / "EURE REISE FORTSETZEN?" */

			} while (answer == -1);

			if (answer == 1) {

				timewarp(HOURS(1));

				do {
					answer = GUI_radio(get_tx2(90), 2, get_tx2(91), get_tx2(92)); /* "DIE SPUR WINDET SICH TIEFER U..." / "DIE JAGD ABBRECHEN?" / "DEN DRACHEN ZUR STRECKE BRINGEN?" */

				} while (answer == -1);

				if (answer == 2) {

					timewarp(HOURS(1));

					do {
						answer = GUI_radio(get_tx2(93), 2, get_tx2(94),	get_tx2(95)); /* "SCHLIESSLICH ERREICHT IHR EIN..." / "DEN DRACHEN AUS DER HÖHLE LOC..." / "LIEBER DOCH DIE JAGD ABBRECHEN?" */

					} while (answer == -1);

					if (answer == 1) {

						GUI_output(get_tx2(96)); /* "ZU EURER GROSSEN ENTTÄUSCHUNG ERSCHEINT NACH MEHRMALIGEM RUFEN KEIN..." */

						gs_tevent080_tatzelwurm = 2;

						add_hero_ap_all(20);
					}
				}
			}
		}
	}
}

/* Clanegh <-> Tyldon: Rastplatz */
void tevent_081(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 2) > 0 && !gs_tevent081_flag) ||
		 gs_tevent081_flag != 0)
	{
		TRV_found_camp_place(0);
		gs_tevent081_flag = 1;
	}
}

/* Oberorken <-> Einsiedlersee: Furt durch den Hjaldorval */
void tevent_082(void)
{
	TRV_cross_a_ford(get_tx(51), 30, 1); /* "HJALDORVAL" */
}

void tevent_083(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_PFLANZENKUNDE, 6) > 0 && !gs_tevent083_flag) || gs_tevent083_flag != 0)
	{
		g_gather_herbs_special = ITEM_ID_WIRSELKRAUT;

		TRV_found_herb_place(0);

		g_gather_herbs_special = -1;

		gs_tevent083_flag = 1;
	}
}

/* Oberorken <-> Einsiedlersee: Kampf F084 (Fluchtversuch mit Kriegskunst-Probe) */
void tevent_084(void)
{
	signed int answer;

	if (!gs_tevent084_flag) {

		load_in_head(44);

		do {
			answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(97), 2, get_tx2(98), get_tx2(99)); /* "IHR ÜBERQUERT GERADE EINEN SA..." / "EUCH ZUM KAMPF STELLEN?" / "VERSUCHEN ZU FLIEHEN?" */

		} while (answer == -1);

		if (answer == 1) {

			if (!do_fight(FIGHT_ID_F084)) {
				gs_tevent084_flag = 1;
			}
		} else {

			if (test_talent(get_first_hero_available_in_group(), TA_KRIEGSKUNST, 5) <= 0)
			{
				g_fig_initiative = 1;

				if (!do_fight(FIGHT_ID_F084)) {
					gs_tevent084_flag = 1;
				}
			}
		}
	}
}

void tevent_085(void)
{
	struct struct_hero *hero = get_first_hero_available_in_group();

	if ((test_talent(hero, TA_WILDNISLEBEN, 4) > 0 && !gs_tevent085_flag) || gs_tevent085_flag) {

		gs_tevent085_flag = 1;

		if ((test_talent(hero, TA_PFLANZENKUNDE, 6) > 0 && !gs_tevent085_herb_flag) || gs_tevent085_herb_flag)
		{
			gs_tevent085_herb_flag = 1;

			g_gather_herbs_special = ITEM_ID_DONFSTENGEL;

			TRV_found_camp_place(2);

			g_gather_herbs_special = -1;

		} else {
			TRV_found_camp_place(0);
		}
	}
}

/* Oberorken <-> Einsiedlersee: Riesenlindwurm */
void tevent_086(void)
{
	signed int i;
	signed int answer;
	struct struct_hero *hero;

	load_in_head(58);

	do {
		answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(100), 2, /* "AM HIMMEL ÜBER EUCH KÖNNT IHR EINEN KLEINEN PUNKT ERKENNEN, DER SIC..." */
					get_tx2(101), get_tx2(102)); /* "DAS SCHAUSPIEL STEHEND WEITER BETRACHTEN?" / "EUCH VOLLKOMMEN UNHELDISCH IN DIE BÜSCHE W..." */

	} while (answer == -1);

	if (answer == 1) {

		hero = get_hero(0);

		for (i = 0; i <= 6; i++, hero++) {

			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id))
			{
				sub_hero_le(hero, 2);
			}
		}

		GUI_dialog_na(0, get_tx2(103)); /* "DER RIESENLINDWURM VERLIERT SCHNELL AN HÖHE UND JAGT KNAPP ÜBER DEM..." */

		if (!gs_tevent086_flag) {

			add_hero_ap_all((gs_tevent086_flag = 15));
		}

	} else {

		GUI_dialog_na(0, get_tx2(104)); /* "MIT INFERNALISCHEM GEHEUL JAGT DER LINDWURM ÜBER EUREN UNTERSCHLUPF..." */

		if (!gs_tevent086_flag) {

			add_hero_ap_all((gs_tevent086_flag = 5));
		}
	}
}

void tevent_088(void)
{
	if (((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 3) > 0) && (!gs_tevent088_flag)) ||
		 gs_tevent088_flag != 0)
	{
		TRV_found_camp_place(0);
		gs_tevent088_flag = 1;
	}
}

/* Felsteyn <-> Einsiedlersee: Furt durch den Hjaldorval */
void tevent_089(void)
{
	TRV_cross_a_ford(get_tx(51), 40, 2); /* "HJALDORVAL" */
}

/* Rybon <-> Thoss: Huette */
void tevent_097(void)
{
	if (TRV_enter_hut_question()) {

		gs_town_loc_type = LOCTYPE_WILDCAMP;
		do_location();
		gs_town_loc_type = LOCTYPE_NONE;
	}
}

/**
 * \brief   travelevent 98: a gap
 *
 *          disappeared heroes can be found in the Efferd temple in Liskor
 *          (Rybon <-> Thoss; hero_disappear(..., 33): Tempel-Nr. 33)
 */
void tevent_098(void)
{
	signed int answer;
	signed int repeat;
	signed int i;
	signed int hero_pos;
	struct struct_hero *hero;

	/* TODO: this may not be correct */
	/* i = Anzahl der Antworten: das Hinueberhangeln gibt es nur mit Seil oder Strickleiter */
	i = get_first_hero_with_item(ITEM_ID_SEIL) != -1 || get_first_hero_with_item(ITEM_ID_STRICKLEITER) != -1 ? 3 : 2; /* TODO: STAFFSPELL? */

	do {
		answer = GUI_radio(get_tx2(27), (signed char)i,	get_tx2(28), get_tx2(29), get_tx2(30)); /* "HIER SCHEINT EUER WEG ZU ENDE..." / "DIE KLAMM ÜBERSPRINGEN?" / "HINABSTEIGEN UND DURCH DEN BA..." / "AN EINEM SEIL HINÜBERHANGELN?" */

	} while (answer == -1);

	do {
		repeat = 0;

		if (answer == 1) {

			hero = get_hero(0);

			for (i = 0; i <= 6; i++, hero++) {

				if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
					!hero->flags.dead)
				{

					if (test_talent(hero, TA_AKROBATIK, 0) > 0) {

						sprintf(g_dtp2,	get_tx2(31), hero->alias); /* "%s LANDET SICHER AUF DER ANDEREN SEITE." */
						GUI_output(g_dtp2);

					} else {

						sprintf(g_dtp2,	get_tx2(32), hero->alias); /* "%s ERREICHT DIE GEGENÜBERLIEGENDE SEITE NICHT UND VERSCHWINDET IN D..." */
						GUI_output(g_dtp2);

						hero_disappear(hero, i, 33);
					}
				}
			}

		} else if (answer == 2) {

			GUI_output(get_tx2(33)); /* "DER BACH IST BITTERKALT UND MEHR ALS EINMAL NEHMT IHR EIN UNFREIWIL..." */

			/* Original-Bug: hero not initialized */
#ifdef M302de_ORIGINAL_BUGFIX
			/* Original-Bugfix: take the leader of the group */
			hero = get_first_hero_available_in_group();
#endif
			hero_disease_test(hero, DISEASE_ID_DUMPFSCHAEDEL, 20 - (hero->attrib[ATTRIB_KK].current + hero->attrib[ATTRIB_KK].mod));

			lose_random_item(hero, 1, get_ttx(506)); /* "OH VERDAMMT, %s HAT %s VERLOREN!" */

			gs_town_loc_type = LOCTYPE_WILDCAMP;
			do_location();
			gs_town_loc_type = LOCTYPE_NONE;

			TRV_load_textfile(-1);
		} else {

			hero = get_hero(hero_pos = select_hero_ok_forced(get_tx2(34))); /* "DAZU MÜSSTE ZUERST EINER VON EUCH HINÜBERSPRINGEN. WER IST DER GLÜC..." */

			if (test_talent(hero, TA_AKROBATIK, 0) > 0) {

				sprintf(g_dtp2,	get_tx2(35), hero->alias); /* "GESCHAFFT! IHR WERFT %s EIN SEIL ZU UND HANGELT EUCH LANGSAM HINÜBER." */
				GUI_output(g_dtp2);

				hero = get_hero(0);
				for (i = 0; i <= 6; i++, hero++) {

					if ((i != hero_pos) && (hero->typus != HERO_TYPE_NONE) &&
						(hero->group_id == gs_active_group_id) &&	!hero->flags.dead)
					{
						if (test_attrib(hero, ATTRIB_GE, -4) > 0) {

							sprintf(g_dtp2,	get_tx2(36), hero->alias); /* "%s ERREICHT SICHER DAS ANDERE UFER." */
							GUI_output(g_dtp2);

						} else {

							sprintf(g_dtp2,	get_tx2(32), hero->alias); /* "%s ERREICHT DIE GEGENÜBERLIEGENDE SEITE NICHT UND VERSCHWINDET IN D..." */
							GUI_output(g_dtp2);

							hero_disappear(hero, i, 33);
						}
					}
				}
			} else {

				sprintf(g_dtp2,	get_tx2(32), hero->alias); /* "%s ERREICHT DIE GEGENÜBERLIEGENDE SEITE NICHT UND VERSCHWINDET IN D..." */
				GUI_output(g_dtp2);

				/* Fehler im Original: i ist hier noch die Zahl der Antworten (3), nicht hero_pos.
				 * hero_disappear() schreibt so die CHR-Datei von Platz 3 statt die des Springers
				 * (dessen temple_id geht verloren); ein springender NSC wird nicht als NSC behandelt. */
				hero_disappear(hero, i, 33);

				do {
					answer = GUI_radio(get_tx2(37), 3, get_tx2(28),	get_tx2(29), get_tx2(30)); /* "WIE WEITER NACH DIESEM UNGLÜCK?" / "DIE KLAMM ÜBERSPRINGEN?" / "HINABSTEIGEN UND DURCH DEN BA..." / "AN EINEM SEIL HINÜBERHANGELN?" */

				} while (answer == -1);

				repeat = 1;
			}
		}

	} while (repeat);
}

/**
 * \brief   makes a hero disappear
 *
 * \param   hero        the hero
 * \param   pos         the position of the hero
 * \param   temple_id	value = -2, -1: hero disappears completely; value >= 0: hero can be found in the temple with that id.
 */
void hero_disappear(struct struct_hero *hero, const signed int pos, const signed int temple_id)
{
	/* decrement the number of heroes */
	gs_total_hero_counter--;

	/* load a new savegame if no hero is present */
	/* TODO: potential Original-Bug: What if only the NPC is left? */
	/* (gs_total_hero_counter zaehlt den NSC mit; bleibt nur er uebrig, laeuft das Spiel weiter) */
	if (!gs_total_hero_counter) {
		g_game_state = GAME_STATE_DEAD;
	}

	/* decrement group counter */
	gs_group_member_counts[gs_active_group_id]--;

	/* write temple_id to character sheet */
	hero->temple_id = temple_id;

	/* reset position in group */
	hero->slot_pos = 0;

	if (pos == 6) {
		/* NPC */
		/* Fehler im Original: npc_id ist 1..6, die Datei des NSC ist ARCHIVE_FILE_NPCS + npc_id - 1 und sein
		 * Timer gs_npc_timers[npc_id] (vgl. add_npc()/remove_npc() in seg095). Hier wird die Datei des
		 * naechsten NSC ueberschrieben (bei Erwo: ARCHIVE_FILE_FINGER_NVF) und der Timer des naechsten
		 * NSC auf -1 gesetzt (bei Erwo: gs_npc_timers[7] = gs_tlk_tav_firstinfo); der verschwundene NSC
		 * selbst bleibt in seiner Taverne anwerbbar. */
		save_npc(ARCHIVE_FILE_NPCS + get_hero(6)->npc_id);

		/* reset NPC timer */
		gs_npc_timers[get_hero(6)->npc_id + 1] = -1;
	} else {
		/* Regular Hero */
		write_chr_temp(pos);
	}

	/* set typus to 0 */
	hero->typus = 0;

	if (temple_id != -2) {
		draw_main_screen();
		init_ani(2);
		g_request_refresh = 1;
	}

	/* set flag to check all heroes */
	g_check_party = 1;
}

/* fight with 4 harpyes */
/* Rybon <-> Thoss */
void tevent_099(void)
{
	signed int answer;

	if (!gs_tevent099_flag) {

		do {
			answer = GUI_radio(get_tx2(38), 2, get_tx2(39),	get_tx2(40)); /* "MIT IRREM KREISCHEN STÜRZEN S..." / "EUCH ZUM KAMPF STELLEN?" / "VERSUCHEN ZU FLIEHEN?" */

		} while (answer == -1);

		/* Original-Bug: The 2nd option "try to flee (or avoid the fight)" is not a try.
		 *		 To trigger the code you have to flee from the fight mode,
		 *		 then the talent test is done, and if this one fails you reenter this fight.
		*/


		if (answer == 1) {

			if (!do_fight(FIGHT_ID_F099)) {
				gs_tevent099_flag = 1;
#ifdef M302de_ORIGINAL_BUGFIX
			}
#endif
		} else {

			if (test_talent(get_first_hero_available_in_group(), TA_KRIEGSKUNST, 4) <= 0) {

				g_fig_initiative = 1;

				if (!do_fight(FIGHT_ID_F099)) {
					gs_tevent099_flag = 1;
				}
			}
#ifndef M302de_ORIGINAL_BUGFIX
			}
#endif
		}
	}
}

/* fight with 2-6 harpyes */
/* Korrektur: 3-6 Harpyien (random_schick(4) + 2); Rybon <-> Thoss */
void tevent_101(void)
{
	signed int answer;
	signed int handicap; /* Zahl der Harpyien (g_max_enemies) und zugleich Erschwernis der Kriegskunst-Probe */

	if (!gs_tevent101_flag) {

		sprintf(g_dtp2,	get_tx2(41), (handicap = random_schick(4) + 2)); /* "AUF DEN FELSKLIPPEN ÜBER EUCH SITZEN WOHL EIN DUTZEND SCHEUSSLICH@A..." */


		do {
			answer = GUI_radio(g_dtp2, 2, get_tx2(42), get_tx2(43)); /* "EUCH ZUM KAMPF STELLEN?" / "VERSUCHEN ZU FLIEHEN?" */

		} while (answer == -1);

		g_max_enemies = handicap;

		/* Original-Bugfix: see description in tevent_099() */
		if (answer == 1) {

			if (!do_fight(FIGHT_ID_F101)) {

				gs_tevent101_flag = 1;
#ifdef M302de_ORIGINAL_BUGFIX
			}
#endif
		} else {

			if (test_talent(get_first_hero_available_in_group(), TA_KRIEGSKUNST, (signed char)handicap) <= 0) {

				g_fig_initiative = 1;

				if (!do_fight(FIGHT_ID_F101)) {
					gs_tevent101_flag = 1;
				}
			}
#ifndef M302de_ORIGINAL_BUGFIX
			}
#endif
		}
	}
}

/* Skjal <-> Orvil: Huette */
void tevent_102(void)
{
	if (TRV_enter_hut_question())
	{
		gs_town_loc_type = LOCTYPE_WILDCAMP;
		do_location();
		gs_town_loc_type = LOCTYPE_NONE;
	}
}

/* Skjal <-> Orvil: Nebeltal */
void tevent_103(void)
{
	signed int answer;
	struct struct_hero *hero;

	/* Original-Bug: save the address instead the return value of a function as a hero */
#ifdef M302de_ORIGINAL_BUGFIX
	hero = get_first_hero_available_in_group();
#else
	hero = (struct struct_hero*)(get_first_hero_available_in_group);
#endif

	do {
		answer = GUI_radio(get_tx2(44), 2, get_tx2(45),	get_tx2(46)); /* "VOR EUCH LIEGT EIN OVALES TAL..." / "VERSUCHEN, GERADEAUS DURCH DE..." / "ODER VERSUCHEN, DAS TAL AM RA..." */

	} while (answer == -1);

	if (answer == 1) {

		if (test_talent(hero, TA_ORIENTIERUNG, 2) > 0) {

			timewarp(HOURS(4));

			GUI_output(get_tx2(47)); /* "IHR KÖNNT DIE HAND KAUM VOR AUGEN SEHEN, DER KALTE NEBEL DRINGT EUC..." */
		} else {

			timewarp(HOURS(6));

			GUI_output(get_tx2(49)); /* "IHR TAPPT BLIND DURCH DEN NEBEL UND HABT BALD JEGLICHE ORIENTIERUNG..." */

			answer = -1;
		}
	} else {

		if (test_talent(hero, TA_ORIENTIERUNG, 3) > 0) {

			timewarp(HOURS(6));

			GUI_output(get_tx2(48)); /* "IHR TASTET EUCH ENTLANG DER FELSEN VORWÄRTS, GERATET HIN UND WIEDER..." */
		} else {

			timewarp(HOURS(8));

			GUI_output(get_tx2(50)); /* "IRGENDWIE SCHEINT DAS NICHT DIE BESTE WAHL GEWESEN ZU SEIN, DENN IH..." */

			answer = -1;
		}
	}

	if (answer == -1) {

		if (test_talent(hero, TA_ORIENTIERUNG, 4) > 0) {

			timewarp(HOURS(4));

			GUI_output(get_tx2(51)); /* "SO IRRT IHR NOCH EINIGE ZEIT UMHER, BIS IHR MIT MEHR GLÜCK ALS VERS..." */
		} else {

			timewarp(HOURS(6));

			GUI_output(get_tx2(52)); /* "SO SEHR IHR EUCH AUCH BEMÜHT, IHR HABT VOLLSTÄNDIG DIE ORIENTIERUNG..." */

			gs_town_loc_type = LOCTYPE_WILDCAMP;
			do_location();
			gs_town_loc_type = LOCTYPE_NONE;

			TRV_load_textfile(-1);

			GUI_output(get_tx2(53)); /* "NACHDEM IHR EINIGE ZEIT GERUHT HABT, GELINGT ES EUCH WUNDERSAMERWEI..." */
		}
	}
}

/* Skjal <-> Orvil: schmaler Grat an einer Klamm (Hoehenangst) */
void tevent_104(void)
{
	signed int answer; /* resused variable: answer and number of heroes with a failed HA test */
	/* (genauer: Helden, bei denen die HA-Probe GELINGT, d. h. die Hoehenangst zuschlaegt) */
	signed int done = 0;
	signed int i;
	signed int nr_heroes; /* spaeter: 1 = Bannbaladin misslungen (Text 87 statt 59) */
	struct struct_hero *hero;
	signed int spell_result;

	load_in_head(57);

	do {

		GUI_dialog_na(0, get_tx2(54)); /* "DER BERGPFAD FÜHRT NUN SCHON GERAUME ZEIT AN EINER KLAMM ENTLANG UN..." */

		hero = get_hero(0);

		for (i = answer = nr_heroes = 0; i <= 6; i++, hero++)
		{
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !hero->flags.dead)
			{
				nr_heroes++;

				/* test for HA+0 */

				if (test_attrib(hero, ATTRIB_HA, 0) > 0)
				{

					timewarp(MINUTES(30));

					sprintf(g_dtp2 + 0x400,	get_tx2(55), hero->alias); /* "%s WEIGERT SICH, WEITERZUGEHEN!" */

					GUI_dialog_na(0, (char*)(g_dtp2 + 0x400));

					answer++;
				}
			}
		}

		if (!answer) {

			/* everything is fine */

			GUI_dialog_na(0, get_tx2(63)); /* "IHR ERREICHT WOHLBEHALTEN DAS ENDE DES SCHMALEN GRATES. HIER BIEGT..." */
			done = 1;

		} else if (answer == nr_heroes) {

			/* all heroes have failed the test */

			do {
				answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(56), 2, get_tx2(57), get_tx2(58)); /* "KEINER TRAUT SICH WEITERZUGEH..." / "UMKEHREN." / "RAST MACHEN." */

			} while (answer == -1);

			if (answer == 2) {
				/* make a camp */

				gs_town_loc_type = LOCTYPE_WILDCAMP;
				do_location();
				gs_town_loc_type = LOCTYPE_NONE;

				TRV_load_textfile(-1);
#ifdef M302de_ORIGINAL_BUGFIX
				/* Original-Bug: The dialog picture is lost when a campfight occurs. */
				load_in_head(57);
#endif

			} else {
				/* turn around */

#ifndef M302de_ORIGINAL_BUGFIX
				/* Original-Bug 53:
				 * If the party was already traveling backward, the new direction should be forward. */
				gs_journey_direction = done = JOURNEY_DIRECTION_CHANGE_TO_BACKWARD; // this is value 1
#else
				gs_journey_direction = (gs_journey_direction == JOURNEY_DIRECTION_FORWARD ? JOURNEY_DIRECTION_CHANGE_TO_BACKWARD : JOURNEY_DIRECTION_CHANGE_TO_FORWARD);
				done = 1;
#endif
			}

		} else {

			/* some heroes, but not all, have failed the test */

			nr_heroes = 0;

			do {

				do {
					answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL,
								(nr_heroes == 0 ? get_tx2(59) : get_tx2(87)), 3, /* "@WOLLT IHR" / "LEIDER MISSLINGT DER VERSUCH, EINEN BANNBA..." */
								get_tx2(60), get_tx2(61), get_tx2(62)); /* "EUREN KAMERADEN DIE AUGEN VER..." / "EINEN <BANNBALADIN> ANWENDEN?" / "ODER HOFFEN, DASS SICH DIE AN..." */
				} while (answer == -1);

				if (answer == 1) {

					timewarp(HOURS(2));

					GUI_dialog_na(0, get_tx2(63)); /* "IHR ERREICHT WOHLBEHALTEN DAS ENDE DES SCHMALEN GRATES. HIER BIEGT..." */

					done = 1;

				} else if (answer == 2) {

					hero = get_hero(select_hero_ok_forced(get_ttx(317))); /* "WELCHER HELD SOLL MAGIE ANWENDEN?" */

					if (hero->typus < HERO_TYPE_HEXE) {

						/* hero is not a spell user */
						GUI_output(get_ttx(330)); /* "DIESE OPTION IST AUSSCHLIESSLICH MAGIEKUNDIGEN VORBEHALTEN!" */
					} else {

						spell_result = test_spell(hero, SP_BANNBALADIN, 0);

						if (spell_result > 0) {

							sub_ae_splash(hero, get_spell_cost(SP_BANNBALADIN, 0));

							GUI_output(get_tx2(16)); /* "IHR ERREICHT WOHLBEHALTEN DAS ENDE DES GRATES. AB JETZT KÖNNT IHR E..." */

							done = 1;

						} else if (spell_result != -99) {

							sub_ae_splash(hero, get_spell_cost(SP_BANNBALADIN, 1));

							nr_heroes = 1;

						} else {

							sprintf(g_dtp2,	get_ttx(607), hero->alias); /* "%s HAT NICHT GENÜGEND ASTRALENERGIE FÜR DIESEN ZAUBER." */
							GUI_output(g_dtp2);
						}

						timewarp(MINUTES(30));
					}
				} else {

					timewarp(HOURS(4));

					GUI_dialog_na(0, get_tx2(63)); /* "IHR ERREICHT WOHLBEHALTEN DAS ENDE DES SCHMALEN GRATES. HIER BIEGT..." */

					done = 1;
				}

			} while (!done);
		}

	} while (!done);
}

/* Skjal <-> Orvil: Huette */
void tevent_105(void)
{
	if (TRV_enter_hut_question()) {

		gs_town_loc_type = LOCTYPE_WILDCAMP;
		do_location();
		gs_town_loc_type = LOCTYPE_NONE;
	}
}

/* Skelellen <-> Phexcaer: Rastplatz mit guter Jagd */
void tevent_106(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 4) > 0 && !gs_tevent106_flag) ||
		 gs_tevent106_flag)
	{
		gs_tevent106_flag = 1;

		TRV_found_camp_place(1);
	}
}

/* Skelellen <-> Phexcaer: steiler Hang (klettern oder Umweg) */
void tevent_107(void)
{
	signed int i;
	signed int answer;
	struct struct_hero *hero;

	load_ani(1);
	draw_main_screen();
	init_ani(0);

	do {
		answer = GUI_radio(get_tx2(64), 2, get_tx2(65), get_tx2(66)); /* "ES SIEHT AUS, ALS MÜSSTET IHR..." / "KLETTERN?" / "DOCH EINE BESSERE STRECKE SUC..." */

	} while (answer == -1);

	if (answer == 1) {

		hero = get_hero(0);
		for (i = 0; i <= 6; i++, hero++) {
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
				!hero->flags.dead && test_talent(hero, TA_KLETTERN, 1) <= 0)
			{

				if (get_first_hero_with_item(ITEM_ID_SEIL) != -1) { /* TODO: ROPE_LADDER? STAFFSPELL? */

					sprintf(g_dtp2, get_tx2(70), hero->alias); /* "%s STÜRZT AB, KANN SICH ABER NOCH IM SEIL FANGEN." */

					sub_hero_le(hero, random_schick(11) + 1);

				} else {

					sprintf(g_dtp2,	get_tx2(71), hero->alias); /* "PLÖTZLICH LÖST SICH EIN FELSBLOCK UND %s STÜRZT UNTER GROSSEM GETÖS..." */

					sub_hero_le(hero, random_schick(11) + 4);
				}

				GUI_output(g_dtp2);
			}
		}

		GUI_output(get_tx2(69)); /* "DIE KLETTEREI IST AUSSERGEWÖHNLICH MÜHSELIG, ABER IHR ERREICHT SCHL..." */

	} else {

		if (test_talent(get_first_hero_available_in_group(), TA_ORIENTIERUNG, 1) > 0) {

			timewarp(HOURS(3));

			GUI_output(get_tx2(67)); /* "ES DAUERT GERAUME ZEIT, BIS IHR EINEN FLACHEREN HANG GEFUNDEN HABT,..." */

		} else {

			timewarp(HOURS(12)); /* Text 68 spricht von "GUT ACHT STUNDEN" */

			GUI_output(get_tx2(68)); /* "MIT MÜH UND NOT FINDET IHR NACH GUT ACHT STUNDEN EINE STELLE, AN DE..." */

		}
	}

	disable_ani();
	g_request_refresh = 1;
}

/* Phexcaer <-> Skelellen: entrance to the dungeon 'orc cave' */
void tevent_108(void)
{
	signed int answer;

	if ((test_talent(get_first_hero_available_in_group(), TA_SINNESSCHAERFE, 3) > 0) && !gs_tevent108_flag)
	{
		gs_tevent108_flag = 1;

		GUI_dialog_na(53, get_tx2(72)); /* "HOCH ÜBER EUCH KÖNNT IHR DEN EINGANG EINER HÖHLE ERKENNEN, DIE@OFFE..." */

		do {
			answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL,
						get_tx2(73), 2, get_tx2(74), get_tx2(75)); /* "IHR STEHT VOR EINER HÖHLE, DI..." / "DIE HÖHLE ERKUNDEN?" / "UMKEHREN UND EUREN WEG FORTSE..." */

		} while (answer == -1);

		if (answer == 1) {
			gs_travel_detour = DUNGEON_ID_ORKBEHAUSUNG;
		}

	} else if (gs_tevent108_flag) {

		load_in_head(53);

		do {
			answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx2(76), 2,	get_tx2(77), get_tx2(78)); /* "HIER FÜHRT EIN PFAD ZU EINER..." / "DIE HÖHLE AUFSUCHEN?" / "DEN WEG FORTSETZEN?" */

		} while (answer == -1);

		if (answer == 1) {
			gs_travel_detour = DUNGEON_ID_ORKBEHAUSUNG;
		}
	}
}

/* Skelellen <-> Phexcaer: Rastplatz */
void tevent_109(void)
{
	if ((test_talent(get_first_hero_available_in_group(), TA_WILDNISLEBEN, 6) > 0 && !gs_tevent109_flag) || gs_tevent109_flag)
	{
		gs_tevent109_flag = 1;

		TRV_found_camp_place(0);
	}
}
