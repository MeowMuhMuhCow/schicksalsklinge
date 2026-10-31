/*
 * seg060 - Taverne: Gespraech mit dem Wirt
 *
 * Inhalt:    talk_tavern() fuehrt den Dialog TAVERN.TLK und setzt dabei die variablen Teile
 *            der Texte ein (Zahl der Helden, Geruechte, Essensqualitaet, Name von Zechkumpan
 *            oder Informant). TLK_tavern() ist die Zustandslogik: Bier und Runden bezahlen,
 *            Charismaproben, Schlaegerei (LE-Verlust, Rauswurf, bei voller Taverne Aechtung
 *            in der Stadt), Zechen-Proben der Helden, Treffen mit NSC und Informanten.
 * Aufrufer:  do_tavern() (seg059); TLK_tavern() nur aus talk_tavern().
 * Daten:     g_tavern_descr_table[89] (Qualitaet 1..20, 1 = beste; Preisaufschlag),
 *            gs_tlk_tav_* (Zustand des Gespraechs).
 * Besonders: Runden "fuer alle" kosten laut Text 20/40/60 Silbertaler, abgezogen werden aber
 *            20/40/60 Heller; die Mahlzeit (Zustand 130) kostet ein Zehntel des Preises aus
 *            do_tavern(). Die Zechen-Probe geht Heldenplaetze 0..n-1 durch statt die Helden
 *            der Gruppe. gs_tlk_tav_firstinfo/_infopromise/_round werden nie zurueckgesetzt.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg060.c
 */
#include <stdio.h>
#include <string.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg004.h"
#include "seg007.h"
#include "seg026.h"
#include "seg027.h"
#include "seg028.h"
#include "seg031.h"
#include "seg047.h"
#include "seg060.h"
#include "seg073.h"
#include "seg095.h"
#include "seg096.h"
#include "seg097.h"
#include "seg103.h"

const struct inn_descr g_tavern_descr_table[89] = {
	{ 0x000c, -0x000a },
	{ 0x000b, 0x0000 },
	{ 0x0009, 0x0000 },
	{ 0x0008, 0x0028 },
	{ 0x000a, 0x000a },
	{ 0x000e, -0x0014 },
	{ 0x000a, 0x0000 },
	{ 0x0010, -0x0028 },
	{ 0x0012, -0x0028 },
	{ 0x000b, 0x000a },
	{ 0x0011, -0x0032 },
	{ 0x0009, 0x000a },
	{ 0x000f, -0x0028 },
	{ 0x000d, -0x000a },
	{ 0x0006, -0x000a },
	{ 0x0011, -0x001e },
	{ 0x0010, -0x001e },
	{ 0x000b, 0x000a },
	{ 0x0012, -0x000a },
	{ 0x0010, -0x0028 },
	{ 0x000e, 0x0000 },
	{ 0x000e, -0x0032 },
	{ 0x000b, 0x0000 },
	{ 0x0012, -0x0028 },
	{ 0x0012, -0x003c },
	{ 0x0006, 0x0000 },
	{ 0x0011, -0x000a },
	{ 0x0006, 0x0014 },
	{ 0x0011, -0x0014 },
	{ 0x0008, 0x0000 },
	{ 0x000d, 0x0000 },
	{ 0x0010, -0x0014 },
	{ 0x0003, 0x001e },
	{ 0x000c, -0x001e },
	{ 0x0004, 0x0000 },
	{ 0x0007, 0x001e },
	{ 0x000b, -0x001e },
	{ 0x0014, -0x0014 },
	{ 0x000a, 0x0000 },
	{ 0x0009, 0x001e },
	{ 0x000b, -0x000a },
	{ 0x0013, -0x0032 },
	{ 0x0006, -0x0014 },
	{ 0x000f, 0x0000 },
	{ 0x000d, -0x0014 },
	{ 0x0008, -0x0014 },
	{ 0x0005, 0x0014 },
	{ 0x000c, 0x0000 },
	{ 0x000f, -0x0014 },
	{ 0x0002, 0x001e },
	{ 0x0007, -0x000a },
	{ 0x0005, 0x0000 },
	{ 0x000f, -0x000a },
	{ 0x0011, -0x001e },
	{ 0x000d, -0x000a },
	{ 0x0006, 0x0000 },
	{ 0x000d, -0x0014 },
	{ 0x0009, 0x0014 },
	{ 0x0012, -0x000a },
	{ 0x0009, 0x0000 },
	{ 0x0006, 0x000a },
	{ 0x0007, 0x0000 },
	{ 0x0013, -0x0028 },
	{ 0x000c, -0x0032 },
	{ 0x0008, -0x000a },
	{ 0x000e, -0x0014 },
	{ 0x0009, -0x001e },
	{ 0x000c, 0x001e },
	{ 0x000f, -0x001e },
	{ 0x0007, 0x0014 },
	{ 0x0012, 0x0000 },
	{ 0x0006, 0x001e },
	{ 0x0010, -0x0014 },
	{ 0x0007, 0x0014 },
	{ 0x000e, 0x0032 },
	{ 0x000a, 0x0000 },
	{ 0x0012, -0x001e },
	{ 0x0008, 0x003c },
	{ 0x000b, -0x001e },
	{ 0x0009, -0x000a },
	{ 0x0007, -0x0014 },
	{ 0x000e, -0x000a },
	{ 0x0007, -0x000a },
	{ 0x0006, 0x000a },
	{ 0x0006, 0x000a },
	{ 0x000f, -0x001e },
	{ 0x000a, 0x0000 },
	{ 0x0013, -0x0032 },
	{ -0x0001, 0x0000 }
}; // ds:0x6c84


/**
 * \brief   dialog with the innkeeper of a tavern (TAVERN.TLK)
 *
 * Like the generic dialog loop, but with special format arguments for many texts.
 */
void talk_tavern(void)
{
	signed int txt_id;
	signed int food_quality;
	signed int answer;
	signed int options;
	struct struct_dialog_state *state_ptr;
	struct struct_dialog_state *states_tab;
	char *text_buffer;
	char *answer1_buffer;
	char *answer2_buffer;
	char *answer3_buffer;
	char *format;
	struct struct_hero *hero;
	uint8_t *gossip_ptr1;
	uint8_t *gossip_ptr2;

	answer = 0;

	text_buffer = (char*)g_dtp2;
	answer1_buffer = text_buffer + 1000;
	answer2_buffer = answer1_buffer + 60;
	answer3_buffer = answer2_buffer + 60;
	g_textbox_width = 9;
	gs_tlk_tav_crowdedness = gs_tlk_tav_testdrunk = gs_current_informer
		= gs_tlk_tav_followinformer = 0;
	gs_tlk_tav_drinkcount = 0;

	disable_ani();
	load_ani(13);
	init_ani(0);
	load_tlk(ARCHIVE_FILE_TAVERN_TLK);
	g_dialog_state = g_dialog_done = 0;
	states_tab = &gs_dialog_states[0];

	do {
		answer = options = 0;

		state_ptr = &states_tab[g_dialog_state];

		if (state_ptr->txt_id1) {
			options++;
		}
		if (state_ptr->txt_id2) {
			options++;
		}
		if (state_ptr->txt_id3) {
			options++;
		}

		if (state_ptr->txt_id != -1) {

			txt_id = (state_ptr->txt_id & 0x7fff);
			format = get_tx(txt_id);
			hero = get_first_hero_available_in_group();

			if (txt_id == 52 || txt_id == 72 || txt_id == 78 || txt_id == 83 || txt_id == 89) {

				sprintf(text_buffer, format, count_heroes_in_group());

			} else if (txt_id == 16 || txt_id == 70) {

				sprintf(text_buffer, format, hero->alias);

			} else if (txt_id == 95) {

				hero = get_hero(gs_tlk_tav_testdrunk - 1);

				sprintf(text_buffer, format, hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_3RD), GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_2ND));
;
			} else if (txt_id == 18 || txt_id == 31) {

				sprintf(text_buffer, format, 20 * gs_tlk_tav_crowdedness);

			} else if (txt_id == 40 || txt_id == 46) {

				gossip_ptr1 = (uint8_t*)load_current_town_gossip();

				do {
					gossip_ptr2 = (uint8_t*)load_current_town_gossip();

				} while (gossip_ptr1 == gossip_ptr2);

				sprintf(text_buffer, format, (char*)gossip_ptr1, (char*)gossip_ptr2);

			} else if (txt_id == 66) {

				sprintf(text_buffer, format, get_informer_hint());

			} else if (txt_id == 105) {

				sprintf(text_buffer, format,
					random_interval(3, 6),
					random_schick(2) == 1 ? get_tx(145) : get_tx(146)); /* "BAUERN" / "REISENDE" */

			} else if (txt_id == 114) {

				food_quality = g_tavern_descr_table[gs_town_typeindex].quality;

				/* print quality [1..20]  1 = best, 20 = worst (-1 is the end marker of the table).
				 * REMARK: 15 matches both 13..15 and 15..18, the first one wins. */
				sprintf(text_buffer, format,
					food_quality >= 1 && food_quality <= 3 ? get_tx(136) : ( /* "GANZ HERVORRAGEND" */
					food_quality >= 4 && food_quality <= 6 ? get_tx(137) : ( /* "SEHR GUT" */
					food_quality >= 7 && food_quality <= 9 ? get_tx(138) : ( /* "GUT" */
					food_quality >= 10 && food_quality <= 12 ? get_tx(139) : ( /* "RECHT GUT" */
					food_quality >= 13 && food_quality <= 15 ? get_tx(140) : ( /* "ZIEMLICH DURCHSCHNITTLICH" */
					food_quality >= 15 && food_quality <= 18 ? get_tx(141) : get_tx(142))))))); /* "NICHT BESONDERS" / "FÜRCHTERLICH" */

			} else if (txt_id == 115) {

				gs_tlk_tav_informersex = random_schick(2);

				sprintf(text_buffer, format,
					gs_tlk_tav_informersex == 1 ? get_tx(143) : get_tx(144)); /* "EIN MANN" / "EINE FRAU" */

			} else if (txt_id == 119) {

				sprintf(text_buffer, format, (uint8_t*)get_random_tavern_message());

			} else if (txt_id == 122 || txt_id == 126 || txt_id == 127 || txt_id == 132) {

				sprintf(text_buffer, format,
					GUI_get_personal_pronoun(gs_tlk_tav_informersex - 1, GRAMMAR_CASE_1ST),
					GUI_get_personal_pronoun(gs_tlk_tav_informersex - 1, GRAMMAR_CASE_1ST));

			} else if (txt_id == 128) {

				sprintf(text_buffer, format, (!gs_current_informer ? get_drinkmate() : get_informer_name()));

			} else if (txt_id == 131) {

				sprintf(text_buffer, format,
					GUI_get_personal_pronoun(gs_tlk_tav_informersex - 1, GRAMMAR_CASE_1ST),
					GUI_get_personal_pronoun(gs_tlk_tav_informersex - 1, GRAMMAR_CASE_1ST),
					(uint8_t*)load_current_town_gossip());

			} else {

				strcpy(text_buffer, format);
			}

			txt_id = state_ptr->txt_id1;
			format = get_tx(txt_id);


			if (txt_id == 1 || txt_id == 19) {
				sprintf(answer1_buffer, format, count_heroes_in_group());
			} else {
				strcpy(answer1_buffer, format);
			}

			txt_id = state_ptr->txt_id2;
			format = get_tx(txt_id);

			if (txt_id == 13) {

				sprintf(answer2_buffer, format, hero->alias);

			} else if (txt_id == 61) {

				sprintf(answer2_buffer, format, count_heroes_in_group());

			} else if (txt_id == 39) {

				sprintf(answer2_buffer, format,
					!gs_informer_flags[informer_id_from_current_town()] ? get_drinkmate() :	get_town_informer_name());

			} else {
				strcpy(answer2_buffer, format);
			}

			txt_id = state_ptr->txt_id3;
			strcpy(answer3_buffer, get_tx(txt_id));

			do {
				answer = GUI_radio((char*)text_buffer, (signed char)options, answer1_buffer, answer2_buffer, answer3_buffer);

			} while (answer == -1);
		}

		g_dialog_next_state = -1;

		if ((state_ptr->txt_id & 0x8000) || (state_ptr->txt_id == -1)) {
			TLK_tavern(answer);
		}

		g_dialog_state = (g_dialog_next_state == -1 ? state_ptr->state1 : g_dialog_next_state);

		if (g_dialog_done == 0) {

			if (options != 0) {

				if (answer == -1) {
					g_dialog_done = 1;
				} else if (answer == 1) {
					g_dialog_state = state_ptr->state1;
				} else if (answer == 2) {
					g_dialog_state = state_ptr->state2;
				} else if (answer == 3) {
					g_dialog_state = state_ptr->state3;
				}
			}

			if (g_dialog_state == 255) {
				g_dialog_done = 1;
			}
		}

	} while (g_dialog_done == 0);

	gs_npc_meet_tavern = 0;
	g_textbox_width = 3;
	g_text_file_index = -1;
	load_tx(g_tx_file_index);
	disable_ani();
}

/**
 * \brief   dialog logic of the tavern (TAVERN.TLK)
 *
 * gs_tlk_tav_crowdedness: how crowded the tavern is (1..3), chosen at random in state 113.
 *
 * \param   answer      the answer chosen in the current state (1..3)
 */
void TLK_tavern(const signed int answer)
{
	signed int old_state;
	signed int tmp;
	int32_t p_money;
	struct struct_hero *hero;
	signed int hero_pos;

	old_state = g_dialog_state;
	hero = get_first_hero_available_in_group();

	if (!old_state) {

		if (gs_tav_kicked_flags[gs_town_typeindex]) {

			hero_pos = get_hero_CH_best();

			g_dialog_next_state = (test_attrib(get_hero(hero_pos), ATTRIB_CH, 0) <= 0 ? 112 : 113);

			gs_tav_kicked_flags[gs_town_typeindex] = 0;

		} else {
			g_dialog_next_state = 113;
		}

	} else if (old_state == 2) {

		if ((gs_tlk_tav_crowdedness == 1 || gs_tlk_tav_crowdedness == 2) && answer == 1) {
			g_dialog_next_state = 3;
		} else if ((gs_tlk_tav_crowdedness == 1 || gs_tlk_tav_crowdedness == 2) && answer == 2) {
			g_dialog_next_state = 5;
		} else {
			g_dialog_next_state = 4;
		}

	} else if (old_state == 5) {

		g_dialog_next_state = (test_attrib(hero, ATTRIB_CH, 3) > 0 ? 81 : 3);

	} else if (old_state == 9) {

		g_dialog_next_state = (gs_informer_flags[informer_id_from_current_town()] != 0 ? 10 : 11);

	} else if (old_state == 12 || old_state == 19 || old_state == 21) {

		gs_tlk_tav_firstinfo = 1;

	} else if (old_state == 14) {

		if (test_attrib(hero, ATTRIB_CH, 0) > 0) {
			g_dialog_next_state = (gs_tlk_tav_crowdedness == 1 ? 16 : 17);
		} else {
			g_dialog_next_state = 15;
		}

	} else if (old_state == 17 || old_state == 39 || old_state == 57) {

		gs_tlk_tav_infopromise = 1;

	} else if (old_state == 18) {

		g_dialog_next_state = (test_attrib(hero, ATTRIB_CH, 2) > 0 ? 19 : 20);

	} else if (old_state == 24) {

		/* Original-Bug? A round for the whole tavern costs "%d SILBERTALER" (20 * fullness, texts 18
		 * and 31), but the party money is counted in heller. So only a tenth is checked and paid
		 * (states 24, 27, 28, 29, 34, 40). */
		drink_while_drinking(10);

#if FIX_0059
		/* Fix 59: der Preis ist in Silbertalern genannt, das Geld wird in Hellern gefuehrt */
		tmp = 200 * gs_tlk_tav_crowdedness;
#else
		tmp = 20 * gs_tlk_tav_crowdedness;
#endif

		p_money = get_party_money();

		g_dialog_next_state = (tmp <= p_money ? 25 : 26);

	} else if (old_state == 27) {

		gs_tlk_tav_round = 1;

		drink_while_drinking(10);

#if FIX_0059
		/* Fix 59: der Preis ist in Silbertalern genannt, das Geld wird in Hellern gefuehrt */
		tmp = 200 * gs_tlk_tav_crowdedness;
#else
		tmp = 20 * gs_tlk_tav_crowdedness;
#endif

		p_money = get_party_money();

		p_money -= tmp;

		set_party_money(p_money);

	} else if (old_state == 28) {

		gs_tlk_tav_round = 1;

		drink_while_drinking(10);

#if FIX_0059
		/* Fix 59: der Preis ist in Silbertalern genannt, das Geld wird in Hellern gefuehrt */
		tmp = 200 * gs_tlk_tav_crowdedness;
#else
		tmp = 20 * gs_tlk_tav_crowdedness;
#endif

		p_money = get_party_money();

		p_money -= tmp + 10;

		set_party_money(p_money);

	} else if (old_state == 29) {

		drink_while_drinking(10);

#if FIX_0059
		/* Fix 59: der Preis ist in Silbertalern genannt, das Geld wird in Hellern gefuehrt */
		tmp = 200 * gs_tlk_tav_crowdedness;
#else
		tmp = 20 * gs_tlk_tav_crowdedness;
#endif

		p_money = get_party_money();

		g_dialog_next_state = (tmp <= p_money ? 30 : 31);

	} else if (old_state == 32) {

		sub_group_le(gs_tlk_tav_crowdedness);

		gs_tav_kicked_flags[gs_town_typeindex] = 1;

		if (gs_tlk_tav_crowdedness == 3) {
#if !(FIX_0425 || FIX_0334)
			gs_town_outlawed_flags[gs_town_typeindex] = 1;
#else
			/* Fix 425: Merker je Stadt wie in TLK_inn; mit dem Tavernen-Index (bis 87) schrieb die Schlaegerei ueber das Feld (52 Byte) hinaus in die Rauswurf-Merker der Herbergen (mit Fix 334 noetig) */
			gs_town_outlawed_flags[gs_town_id] = 1;
#endif
		}

	} else if (old_state == 33) {

		sub_group_le(2 * gs_tlk_tav_crowdedness);

		gs_tav_kicked_flags[gs_town_typeindex] = 1;

		if (gs_tlk_tav_crowdedness == 3) {
#if !(FIX_0425 || FIX_0334)
			gs_town_outlawed_flags[gs_town_typeindex] = 1;
#else
			/* Fix 425: wie oben (Zustand 33) */
			gs_town_outlawed_flags[gs_town_id] = 1;
#endif
		}

	} else if (old_state == 34) {

		drink_while_drinking(10);

#if FIX_0059
		/* Fix 59: der Preis ist in Silbertalern genannt, das Geld wird in Hellern gefuehrt */
		tmp = 200 * gs_tlk_tav_crowdedness;
#else
		tmp = 20 * gs_tlk_tav_crowdedness;
#endif

		p_money = get_party_money();

		p_money -= tmp;

		set_party_money(p_money);

	} else if (old_state == 40) {

		drink_while_drinking(10);

#if FIX_0059
		/* Fix 59: der Preis ist in Silbertalern genannt, das Geld wird in Hellern gefuehrt */
		tmp = 200 * gs_tlk_tav_crowdedness;
#else
		tmp = 20 * gs_tlk_tav_crowdedness;
#endif

		p_money = get_party_money();

		g_dialog_next_state = (tmp <= p_money ? 41 : 42);

	} else if (old_state == 43) {

		if (test_attrib(hero, ATTRIB_CH, 4) > 0 && (gs_tlk_tav_crowdedness == 1 || gs_tlk_tav_crowdedness == 2)) {
			g_dialog_next_state = 56;
		} else {
			g_dialog_next_state = 44;
		}

	} else if (old_state == 45) {

		g_dialog_next_state = (gs_tlk_tav_crowdedness == 3 ? 57 : 46);

	} else if (old_state == 46) {

		g_dialog_next_state = (gs_informer_flags[informer_id_from_current_town()] != 0 ? 47 : 48);

	} else if (old_state == 49) {

		g_dialog_next_state = (test_attrib(hero, ATTRIB_CH, 4) > 0 ? 19 : 20);

	} else if (old_state == 54) {

		gs_tav_kicked_flags[gs_town_typeindex] = 1;

	} else if (old_state == 55) {

		sub_group_le(1);

		gs_tav_kicked_flags[gs_town_typeindex] = 1;

	} else if (old_state == 56) {

		/* REMARK: gs_tlk_tav_freebeer is never read */
		gs_tlk_tav_freebeer = 1;

	} else if (old_state == 58) {

		add_party_money(10);

		g_dialog_next_state = (gs_informer_flags[informer_id_from_current_town()] != 0 ? 59 : 60);

	} else if (old_state == 61) {

		g_dialog_next_state = (gs_tlk_tav_crowdedness == 3 ? 63 : 77);

	} else if (old_state == 63) {

		g_dialog_next_state = (gs_tlk_tav_infopromise ? 64 : 65);

	} else if (old_state == 68) {

		g_dialog_next_state = (gs_tlk_tav_infopromise ? 69 : 36);

	} else if (old_state == 71) {

		gs_tlk_tav_infopromise = 0;
		gs_tlk_tav_firstinfo = 1;

	} else if (old_state == 73) {

		g_dialog_next_state = (gs_tlk_tav_firstinfo ? 74 : 4);

	} else if (old_state == 77) {

		g_dialog_next_state = (gs_tlk_tav_infopromise ? 78 : 82);

	} else if (old_state == 79) {

		g_dialog_next_state = (gs_informer_flags[informer_id_from_current_town()] != 0 ? 153 : 154);

	} else if (old_state == 80) {

		g_dialog_next_state = (test_attrib(hero, ATTRIB_CH, 0) <= 0 && gs_tlk_tav_round ? 20 : 19);

	} else if (old_state == 85) {

		g_dialog_next_state = (gs_tlk_tav_firstinfo ? 88 : 7);

	} else if (old_state == 86) {

		tmp = test_attrib(hero, ATTRIB_CH, 0);

		g_dialog_next_state = (gs_tlk_tav_firstinfo || (!gs_tlk_tav_firstinfo && !gs_tlk_tav_round && tmp <= 0) ? 84 : 81);

	} else if (old_state == 87) {

		g_dialog_next_state = (gs_tlk_tav_firstinfo ? 88 : 89);

	} else if (old_state == 88) {

		g_dialog_next_state = (gs_tlk_tav_round ? 92 : 97);

	} else if (old_state == 89) {

		g_dialog_next_state = (gs_tlk_tav_round ? 90 : 91);

	} else if (old_state == 93) {

		g_dialog_next_state = (gs_informer_flags[informer_id_from_current_town()] != 0 ? 94 : 95);

	} else if (old_state == 98) {

		g_dialog_next_state = (test_attrib(hero, ATTRIB_CH, 0) > 0 ? 99 : 102);

	} else if (old_state == 99) {

		g_dialog_next_state = (gs_informer_flags[informer_id_from_current_town()] != 0 ? 100 : 101);

	} else if (old_state == 103) {

		g_dialog_next_state = (gs_tlk_tav_crowdedness == 3 ? 131 : 132);

	} else if (old_state == 104) {

		g_dialog_next_state = (gs_day_timer >= HOURS(22) && gs_tlk_tav_crowdedness != 3 ? 117 : 119);

	} else if (old_state == 106) {

		g_dialog_next_state = (gs_tlk_tav_round ? 107 : 108);

	} else if (old_state == 108) {

		/* Original-Bug? The drinking test runs over the hero slots 0 .. count_heroes_in_group() - 1.
		 * count_heroes_in_group() counts only living heroes of the active group, but state 109
		 * does not check the group: heroes of another group are tested, heroes in higher slots
		 * of the active group (behind a dead hero or an empty slot, the NPC in slot 6) are skipped. */

#if FIX_0060
		/* Fix 60: alle 7 Plaetze durchgehen, Zustand 109 prueft die Gruppe */
		if (gs_tlk_tav_testdrunk < 7) {
#else
		if (gs_tlk_tav_testdrunk < count_heroes_in_group()) {
#endif
			g_dialog_next_state = (109);
		} else {
			g_dialog_next_state = (111);
			gs_tlk_tav_testdrunk = 0;
		}

	} else if (old_state == 109) {

		g_dialog_next_state = (108);

		if ((get_hero(gs_tlk_tav_testdrunk)->typus) && !get_hero(gs_tlk_tav_testdrunk)->flags.dead && gs_tlk_tav_drinkcount
#if FIX_0060
			/* Fix 60: nur Helden der aktiven Gruppe */
			&& (get_hero(gs_tlk_tav_testdrunk)->group_id == gs_active_group_id)
#endif
			)
		{
			g_dialog_next_state = (test_talent(get_hero(gs_tlk_tav_testdrunk), TA_ZECHEN, gs_tlk_tav_drinkcount - 8) > 0 ? 108 : 110);
		}

		/* TODO: this variable is unsigned (harmless, the values are 0..7) */
		gs_tlk_tav_testdrunk++;

	} else if (old_state == 110) {

		hero_get_drunken(get_hero(gs_tlk_tav_testdrunk - 1));

	} else if (old_state == 112) {

		sub_group_le(1);

	} else if (old_state == 113) {

		gs_tlk_tav_crowdedness = random_schick(3);

		g_dialog_next_state = (gs_tlk_tav_crowdedness == 1 ? 114 : (gs_tlk_tav_crowdedness == 2 ? 115 : 116));

	} else if (old_state == 119) {

		tmp = random_schick(3);

		g_dialog_next_state = (tmp == 1 ? 120 : (tmp == 2 ? 121 : 122));

	} else if (old_state == 120) {

		g_dialog_next_state = (random_schick(2) == 1 ? 123 : 124);

	} else if (old_state == 121) {

		tmp = random_schick(5);

		g_dialog_next_state = (tmp == 1 || tmp == 2 ? 125 : (tmp == 3 ? 126 : 127));

	} else if (old_state == 122) {

		g_dialog_next_state = (random_schick(3) == 1 ? 128 : 129);

	} else if (old_state == 124 || old_state == 128) {

		gs_tlk_tav_crowdedness = 2;

	} else if (old_state == 125) {

		gs_tlk_tav_crowdedness = 1;

	} else if (old_state == 127) {

		gs_tlk_tav_crowdedness = 3;

	} else if (old_state == 130) {

		/* Original-Bug? Same price formula as for a meal in do_tavern() (seg059), where the result is
		 * in silver and multiplied by 10. Here it is subtracted as heller: the meal costs a tenth. */

		tmp = count_heroes_in_group() * (6 - g_tavern_descr_table[gs_town_typeindex].quality / 4);
		tmp += (tmp * g_tavern_descr_table[gs_town_typeindex].price_mod) / 100;
#if FIX_0059
		/* Fix 59: Silberstuecke wie in do_tavern(), also mal 10 Heller */
		tmp *= 10;
#endif
		p_money = get_party_money();
		p_money -= tmp;
		set_party_money(p_money);

		drink_while_drinking(10);
		eat_while_drinking(100);

		timewarp(HOURS(1));

	} else if (old_state == 131) {

		timewarp(MINUTES(45));
		gs_tlk_tav_drinkcount++;

		p_money = get_party_money();
		p_money -= 10;
		set_party_money(p_money);

		drink_while_drinking(10);

		g_dialog_next_state = (p_money >= 10 ? 104 : 105);
	}

	if (old_state == 132) {

		g_dialog_next_state = (npc_meetings(gs_town_typeindex) ? 144 : (!tavern_quest_infos() ? 133 : 144));

	} else if (old_state == 133) {

		g_dialog_next_state = (random_schick(5) == 5 ? 152 : 131);

	} else if (old_state == 134) {

		gs_tlk_tav_informersex = 1;

	} else if (old_state == 138) {

		timewarp(MINUTES(5));

	} else if (old_state == 139) {

		g_dialog_next_state = (random_schick(4) == 1 ? 140 : 138);

	} else if (old_state == 140) {

		g_dialog_next_state = (random_schick(3) == 2 ? 142 : 141);

	} else if (old_state == 146) {

		if (!gs_current_informer) {
			get_drinkmate();
		} else {
			get_informer_name();
		}

		g_dialog_next_state = (!gs_current_informer ? 148 : 147);

	} else if (old_state == 151) {

		gs_tlk_tav_followinformer = 1;

	} else if (old_state == 152) {

		g_dialog_next_state = (random_schick(3) == 2 ? 134 : 135);

	}
}
