/*
 * seg103 - Talente anwenden
 *
 * Inhalt:    Talentproben (test_talent, mit Sonderfall Fernkampf), Auswahl
 *            eines Talents beim Stufenanstieg (LVL_select_talent) oder nach Ort
 *            (select_talent: Heiltalente, dazu Taverne/Markt/Händler/Lager/
 *            Herberge), Ausführen der anwendbaren Talente (Heilen von Gift,
 *            Krankheit und Wunden, Akrobatik, Musizieren, Falschspiel,
 *            Taschendiebstahl, Alchimie) und die Feilschen-Probe beim Handel.
 * Aufrufer:  test_talent aus sehr vielen Modulen (Städte, Dungeons, Reise);
 *            GUI_use_talent aus dem Statusmenü (seg048), GUI_use_talent2 aus
 *            Lager, Herberge, Taverne (seg051, seg052, seg054, seg059);
 *            bargain aus den Läden (seg056-seg058); LVL_select_talent aus seg050.
 * Daten:     Standardliste der Heiltalente, Formatstring "%s~%d".
 * Besonders: Fehlgeschlagenes Wundenheilen setzt beim Patienten
 *            staffspell_timer (wirkt als Regenerationssperre, siehe seg049).
 *            select_talent übergibt GUI_radio ein 7. Element außerhalb des
 *            Felds (harmlos). Misslungener Taschendiebstahl kostet das gesamte
 *            Geld der Gruppe.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg103.c
 */
#include <stdio.h>
#include <string.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg007.h"
#include "seg026.h"
#include "seg047.h"
#include "seg053.h"
#include "seg097.h"
#include "seg103.h"
#include "seg104.h"

/* helper type to copy a 6-byte string to the stack */
struct str6 {
	char a[6];
};

static char g_select_talent_lvlup[6] = "%s~%d"; // ds:0xacce
static signed char g_select_talent_defaults[6] = { TA_HEILEN_GIFT, TA_HEILEN_KRANKHEITEN, TA_HEILEN_WUNDEN, -1, -1, -1 }; // ds:0xacd4

/**
 * \brief   select a talent: first the talent class, then the talent
 *
 * \param   hero        the hero
 * \param   show_values 1 = show the talent values and the number of increases left (level up);
 *                      0 = names only (no caller uses 0)
 * \return              talent ID, -1 = ESC in the talent menu, -2 = ESC in the class menu
 */
signed int LVL_select_talent(const struct struct_hero *hero, const signed int show_values)
{
	signed int i;
	signed int answer;
	signed int first_talent;
	signed int retval = -1;
	/* string on stack "%s~%d" */
	struct str6 format_str = *(struct str6*)g_select_talent_lvlup;

	if (show_values != 0) {

		strcpy(g_text_output_buf, get_ttx(205)); /* "STEIGERUNGSVERSUCH" */

		if (hero->saved_talent_increases > 1) {
			strcat(g_text_output_buf, get_ttx(393)); /* "E" */
		}

		sprintf(g_dtp2, get_ttx(204), /* "NOCH %s %d %s ZU VERGEBEN!" */
			(hero->saved_talent_increases > 1) ? get_ttx(305) : get_ttx(304),	/* sind / ist */
			hero->saved_talent_increases, g_text_output_buf);
	} else {

		strcpy(g_dtp2, get_ttx(216)); /* "AUS WELCHER GRUPPE?" */
	}

	/* ask for the talentclass */
	answer = GUI_radio(g_dtp2, 7, get_ttx(100), get_ttx(101), get_ttx(102), get_ttx(105), get_ttx(103), get_ttx(104), get_ttx(106)) - 1; /* "KAMPF" / "KÖRPER" / "GESELLSCHAFT" / "NATUR" / "WISSEN" / "HANDWERK" / "INTUITION" */

	if (answer != -2) {

		first_talent = g_talentclasses[answer].first;

		if (show_values != 0) {

			for (i = 0; g_talentclasses[answer].length > i; i++) {

				sprintf(g_dtp2 + 50 * i, format_str.a, get_ttx(first_talent + i + 48), hero->talents[first_talent + i]); /* TEXT.LTX 48..99: Talentname */

				g_radio_name_list[i] = g_dtp2 + 50 * i;
			}
		} else {

			for (i = 0; g_talentclasses[answer].length > i; i++) {
				g_radio_name_list[i] = get_ttx(first_talent + i + 48); /* TEXT.LTX 48..99: Talentname */
			}
		}

		retval = GUI_radio(get_ttx(218), g_talentclasses[answer].length, /* "WELCHES TALENT?" */
				g_radio_name_list[0], g_radio_name_list[1],
				g_radio_name_list[2], g_radio_name_list[3],
				g_radio_name_list[4], g_radio_name_list[5],
				g_radio_name_list[6], g_radio_name_list[7],
				g_radio_name_list[8], g_radio_name_list[9],
				g_radio_name_list[10], g_radio_name_list[11],
				g_radio_name_list[12], g_radio_name_list[13],
				g_radio_name_list[14], g_radio_name_list[15]);

		if (retval != -1) {
			retval += first_talent - 1;
		}

	} else {
		retval = -2;
	}

	return retval;
}

/**
 * \brief   returns hero which seems best for a talent
 *
 * \param   talent_id       talent
 */
struct struct_hero* get_best_hero_for_talent(const signed int talent_id)
/* called from only a single position, namely test_talent((struct struct_hero*)..), and only if game is in 'easy' mode and the tested talent is TA_SINNESSCHAERFE */
{
	signed int i;
	signed int cur;

	signed int max = -1;
	struct struct_hero *hero_i;
	struct struct_hero *retval;

#if !defined(__BORLANDC__)
	retval = NULL;
#endif

	hero_i = get_hero(0);

	for (i = 0; i <= 6; i++, hero_i++) {

		if ((hero_i->typus != HERO_TYPE_NONE) &&
			(hero_i->group_id == gs_active_group_id) &&
			/* TODO: potential Original-Bug: What if petrified / unconscious etc.? */
			!hero_i->flags.dead) {

			/* add the three attributes of the talent test (current + mod) and the talent value */
			cur =	hero_i->attrib[g_talent_descriptions[talent_id].attrib1].current +
				hero_i->attrib[g_talent_descriptions[talent_id].attrib1].mod +
				hero_i->attrib[g_talent_descriptions[talent_id].attrib2].current +
				hero_i->attrib[g_talent_descriptions[talent_id].attrib2].mod +
				hero_i->attrib[g_talent_descriptions[talent_id].attrib3].current +
				hero_i->attrib[g_talent_descriptions[talent_id].attrib3].mod +
				hero_i->talents[talent_id];

			if (cur > max) {

				/* if the sum is greater than the current best */
				max = cur;
				retval = hero_i;
			}
		}
	}

#if !defined(__BORLANDC__)
	/* sanity check for Original Bug hunting */
	if (retval == 0)
		D1_ERR("Original-Bug: %s undefinierter RÃ¼ckgabewert\n",	__func__);
#endif

	return retval;
}

/**
 * \brief   performs a talent test
 *
 * \param   hero        hero which should be tested
 * \param   talent_id   the talent to test
 * \param   handicap    may be positive or negative. The higher the value, the harder the test.
 * \return              > 0: success (remaining points), 0: failed, -1: unlucky (W20 = 20, ranged only),
 *                      99: lucky (W20 = 1, ranged only); 0 for melee weapon talents.
 *                      Other talents: result of test_attrib3().
 */
signed int test_talent(const struct struct_hero* hero, const signed int talent_id, signed char handicap)
{
	signed int randval;
	signed int e_talentval;

	/* only talents from Schusswaffen on: no test for melee weapon talents */
	if ((talent_id >= TA_SCHUSSWAFFEN) && (talent_id <= TA_SINNESSCHAERFE)) {

#if !defined(__BORLANDC__)
		D1_INFO("%s Talentprobe %s %+d (TaW %d)", hero->alias, names_talent[talent_id], handicap, hero->talents[talent_id]);
#endif

		/* special test if talent is a range weapon talent */
		if ((talent_id == TA_SCHUSSWAFFEN) || (talent_id == TA_WURFWAFFEN)) {

			/* calculate range weapon base value */
			e_talentval = (	hero->attrib[ATTRIB_KL].current +
					hero->attrib[ATTRIB_KL].mod +
					hero->attrib[ATTRIB_GE].current +
					hero->attrib[ATTRIB_GE].mod +
					hero->attrib[ATTRIB_KK].current +
					hero->attrib[ATTRIB_KK].mod) / 4;

			e_talentval += hero->talents[talent_id];

			e_talentval -= handicap;

			randval = random_schick(20);

			/* Unlucky */
			if (randval == 20) {
#if !defined(__BORLANDC__)
				D1_INFO("Ungluecklich\n");
#endif
				return -1;
			}
			/* Lucky */
			if (randval == 1) {
#if !defined(__BORLANDC__)
				D1_INFO("Gluecklich\n");
#endif
				return 99;
			}
			if (randval <= e_talentval) {
				/* test successful */
#if !defined(__BORLANDC__)
				D1_INFO(" (%d) -> bestanden\n", randval);
#endif
				return e_talentval - randval + 1;
			}

			/* test unsuccessful */
#if !defined(__BORLANDC__)
			D1_INFO(" (%d) -> nicht bestanden\n", randval);
#endif
			return 0;
		}

		/* automatically get hero with best senses in beginner mode */
		if ((talent_id == TA_SINNESSCHAERFE) && (g_game_mode == GAME_MODE_BEGINNER)) {

			hero = get_best_hero_for_talent(TA_SINNESSCHAERFE);

#if defined(__BORLANDC__)
			/* seems to have been debug stuff with conditional compilation */
			if (0) return 0;
#endif
		}

		/* do the test */
		handicap -= hero->talents[talent_id];

		return test_attrib3(hero, g_talent_descriptions[talent_id].attrib1,
						g_talent_descriptions[talent_id].attrib2,
						g_talent_descriptions[talent_id].attrib3, handicap);
	}

	return 0;
}

/* helper type to copy the 6 talent IDs to the stack */
struct talent_ids6 {
	signed char a[6];
};

/**
 * \brief   select one of the talents usable at the current location
 *
 *          Always the three healing talents; plus Akrobatik, Falschspiel (not from
 *          the status menu) and Musizieren in a tavern, Alchimie in wild camp or inn,
 *          Akrobatik and Taschendiebstahl on the market, Taschendiebstahl at a merchant.
 *
 * \return              talent ID or -1
 */
signed int select_talent(void)
{
	signed int retval = -1;
	signed int nr_talents = 3;

	/* available talents {TA_HEILEN_GIFT, TA_HEILEN_KRANKHEITEN, TA_HEILEN_WUNDEN, -1, -1, -1} */
	struct talent_ids6 a = *(struct talent_ids6*)g_select_talent_defaults;

	/* add talents for special location */
	if (gs_town_loc_type == LOCTYPE_TAVERN) {
		a.a[nr_talents] = TA_AKROBATIK;
		nr_talents++;

		if (!g_forbid_ta_falschspiel) {
			a.a[nr_talents] = TA_FALSCHSPIEL;
			nr_talents++;
		}

		a.a[nr_talents] = TA_MUSIZIEREN;
		nr_talents++;
	} else if ((gs_town_loc_type == LOCTYPE_WILDCAMP) || (gs_town_loc_type == LOCTYPE_INN)) {
		a.a[nr_talents] = TA_ALCHIMIE;
		nr_talents++;
	} else if (gs_town_loc_type == LOCTYPE_MARKET) {
		a.a[nr_talents] = TA_AKROBATIK;
		nr_talents++;
		a.a[nr_talents] = TA_TASCHENDIEBSTAHL;
		nr_talents++;
	} else if (gs_town_loc_type == LOCTYPE_MERCHANT) {
		a.a[nr_talents] = TA_TASCHENDIEBSTAHL;
		nr_talents++;
	}

	retval = GUI_radio(get_ttx(218), (signed char)nr_talents, /* "WELCHES TALENT?" */
				get_ttx(a.a[0] + 48), /* TEXT.LTX 48..99: Talentname */
				get_ttx(a.a[1] + 48), /* TEXT.LTX 48..99: Talentname */
				get_ttx(a.a[2] + 48), /* TEXT.LTX 48..99: Talentname */
				get_ttx(a.a[3] + 48), /* TEXT.LTX 48..99: Talentname */
				get_ttx(a.a[4] + 48), /* TEXT.LTX 48..99: Talentname */
				get_ttx(a.a[5] + 48), /* TEXT.LTX 48..99: Talentname */
				get_ttx(a.a[6] + 48)); /* TEXT.LTX 48..99: Talentname */ /* a.a[6] is outside of the array; never shown, since nr_talents <= 6 */

	if (retval != -1) {
		return a.a[retval - 1];
	}

	return retval;
}

/**
 * \brief   a hero uses a talent
 *
 * \param   hero_pos    position of the hero
 * \param   handicap    handicap of the tests
 * \param   talent_id   the talent (-1 = nothing)
 * \return              1 by default, the LE healed (wounds/poison), 0 on a failed
 *                      healing, -1 on a failed performance, cheating or pickpocketing
 */
signed int use_talent(const signed int hero_pos, signed char handicap, const signed int talent_id)
{
	signed int retval = 1;
	signed int le_damage;

	signed int patient_pos;
	signed int le;
	struct struct_hero *hero = get_hero(hero_pos);
	struct struct_hero *patient;
	int32_t money;
	signed int poison_id;
	signed int tx_file_bak;

	if (talent_id != -1) {

		tx_file_bak = g_tx_file_index;

		load_tx(ARCHIVE_FILE_SPELLTXT_LTX);

		switch (talent_id) {

		case TA_HEILEN_GIFT: {

			g_hero_sel_exclude = (signed char)hero_pos;

			patient_pos = select_hero_from_group(get_ttx(460)); /* "WER SOLL BEHANDELT WERDEN?" */

			if (patient_pos != -1) {

				patient = get_hero(patient_pos);

				if (is_hero_healable(patient)) {

					poison_id = hero_is_poisoned(patient);

					if (poison_id == POISON_ID_NONE) {

						/* patient is not poisoned */
						sprintf(g_dtp2,	get_ttx(463), patient->alias); /* "<%s IST DOCH GAR NICHT VERGIFTET.>" */
						GUI_output(g_dtp2);

					} else if (patient->heal_timer > 0) {

						/* patient timer is not zero */
						sprintf(g_dtp2,	get_ttx(697), patient->alias); /* "DER LETZTE HEILVERSUCH DURCH EIN TALENT IST BEI %s NOCH NICHT LANGE..." */
						GUI_output(g_dtp2);

					} else {
						/* set patient timer */
						patient->heal_timer = HOURS(4); /* 4 hours */

						if (test_talent(hero, TA_HEILEN_GIFT, handicap) > 0) {

							timewarp(MINUTES(20));

							if (test_talent(hero, TA_HEILEN_GIFT, g_poison_strength[poison_id] + handicap) > 0) {
								/* success */
								sprintf(g_dtp2, get_ttx(690), hero->alias, patient->alias); /* "TSA SEI DANK!@%s KANN DAS GIFT IN %sS ADERN STOPPEN." */
								GUI_output(g_dtp2);

								patient->poison[poison_id].time_counter = 0;
								patient->poison[poison_id].status = POISON_STATUS_RECOVER;

								sprintf(g_dtp2,	get_ttx(692), hero->alias, patient->alias); /* "SOLL %s VERSUCHEN %s ZU HEILEN?" */

								if (GUI_bool(g_dtp2)) {

									do {
										le = GUI_input(get_ttx(693), 2); /* "WIE VIELE LEBENSPUNKTE SOLLEN GEHEILT WERDEN?" */

									} while (le <= 0);

									if ((retval = test_talent(hero, TA_HEILEN_GIFT, le + handicap)) > 0) {

										sprintf(g_dtp2,	get_ttx(691), hero->alias, patient->alias, le); /* "%s VERHILFT %s ZU %d LE." */

										add_hero_le(patient, le);

										GUI_output(g_dtp2);
									} else {
										/* talent test failed */
										le_damage = 3;

										if (patient->le <= le_damage) {

											/* don't kill the patient: at least 1 LE should remain */
											le_damage = patient->le - 1;
										}

										sub_hero_le(patient, le_damage);

										sprintf(g_dtp2,	get_ttx(694), patient->alias, le_damage); /* "DAS HEILEN MISSLINGT!@%s ERLEIDET %d ZUSÄTZLICHE SCHADENSPUNKTE." */
										GUI_output(g_dtp2);

										retval = 0;
									}
								}
							} else {
								/* healing failed */
								sprintf(g_dtp2,	get_ttx(689), hero->alias, patient->alias); /* "%s KANN DAS GIFT IN %sS ADERN NICHT STOPPEN." */
								GUI_output(g_dtp2);
							}
						} else {
							/* recognizing the poison failed */
							sprintf(g_dtp2,	get_ttx(688), hero->alias, patient->alias); /* "%s KANN NICHT HERAUSFINDEN, WELCHES GIFT IN %sS ADERN FLIESST." */
							GUI_output(g_dtp2);
						}
					}
				}
			}
			break;
		}
		case  TA_HEILEN_KRANKHEITEN : {

			g_hero_sel_exclude = (signed char)hero_pos;

			patient_pos = select_hero_from_group(get_ttx(460)); /* "WER SOLL BEHANDELT WERDEN?" */

			if (patient_pos != -1) {

				patient = get_hero(patient_pos);

				talent_cure_disease(hero, patient, handicap, 0);
			}
			break;
		}
		case TA_HEILEN_WUNDEN : {

			g_hero_sel_exclude = (signed char)hero_pos;

			patient_pos = select_hero_from_group(get_ttx(460)); /* "WER SOLL BEHANDELT WERDEN?" */

			if (patient_pos != -1) {

				patient = get_hero(patient_pos);

				if (is_hero_healable(patient)) {

					if (patient->le >= patient->le_max) {

						/* no need to heal */
						sprintf(g_dtp2, get_ttx(461), patient->alias); /* "<%s SCHEINT MIR VÖLLIG IN ORDNUNG ZU SEIN.>" */
						GUI_output(g_dtp2);

					} else if (patient->heal_timer > 0) {

						/* timer is still running */
						sprintf(g_dtp2, get_ttx(697), patient->alias); /* "DER LETZTE HEILVERSUCH DURCH EIN TALENT IST BEI %s NOCH NICHT LANGE..." */
						GUI_output(g_dtp2);

					} else {
						patient->heal_timer = DAYS(1);

						if (test_talent(hero, TA_HEILEN_WUNDEN, handicap) > 0) {

							if (test_talent(hero, TA_HEILEN_WUNDEN, handicap) > 0) {

								retval = hero->talents[TA_HEILEN_WUNDEN] > 1 ? hero->talents[TA_HEILEN_WUNDEN] : 1;

								add_hero_le(patient, retval);

								sprintf(g_dtp2, get_ttx(691), hero->alias, patient->alias, retval); /* "%s VERHILFT %s ZU %d LE." */
								GUI_output(g_dtp2);

							} else {
								/* talent test failed */
								le_damage = 3;

								if (patient->le <= le_damage) {

									/* don't kill the patient: at least 1 LE should remain */
									le_damage = patient->le - 1;
								}

								sub_hero_le(patient, le_damage);

								sprintf(g_dtp2,	get_ttx(694), patient->alias, le_damage); /* "DAS HEILEN MISSLINGT!@%s ERLEIDET %d ZUSÄTZLICHE SCHADENSPUNKTE." */
								GUI_output(g_dtp2);

								retval = 0;

								patient->staffspell_timer = DAYS(1); /* TODO: Why STAFFSPELL ?? BUG! */
								/* Remark: probably intended: staffspell_timer is only read in GRP_hero_sleep (seg049) and blocks
								 * the regeneration of the patient for a day. heal_timer was already set above. */
								//patient->heal_timer = DAYS(1);
							}
						} else {

							if (random_schick(20) <= 7) {
								/* TODO Original-Bug? What if already diseased or recovering? */

								/* 35% chance: infected with Wundfieber disease */
								sprintf(g_dtp2, get_ttx(699), hero->alias, patient->alias); /* "DIE GÖTTER SEIEN VERFLUCHT.@HÄTTE %s DAS NUR NICHT VERSUCHT!@DAS KU..." */

								patient->disease[DISEASE_ID_WUNDFIEBER].status = DISEASE_STATUS_DISEASED;
								patient->disease[DISEASE_ID_WUNDFIEBER].time_counter = 0;

							} else {
								/* 65% chance: just failed, no infection */
								sprintf(g_dtp2,	get_ttx(698), hero->alias, patient->alias); /* "%s KANN DIE WUNDE VON %s NICHT RICHTIG VERSORGEN." */
							}

							GUI_output(g_dtp2);
						}
					}
				}
			}
			break;
		}
		case TA_AKROBATIK : {

			if (gs_ingame_timers[INGAME_TIMER_AKROBATIK] > 0) {

				GUI_output(get_tx(34)); /* "ICH DENKE IM MOMENT SOLLTE NICHT SCHON WIEDER VERSUCHT WERDEN, DEN..." */

			} else {

				if (test_talent(hero, TA_AKROBATIK, handicap) > 0) {

					money = random_interval(10, 200);

					make_valuta_str(g_text_output_buf, money);

					sprintf(g_dtp2, get_tx(35), hero->alias, g_text_output_buf); /* "%s KANN DIE LEUTE BEGEISTERN!@SIE LASSEN %s SPRINGEN." */

					GUI_output(g_dtp2);

					add_party_money(money);

					gs_ingame_timers[INGAME_TIMER_AKROBATIK] = HOURS(8);

					g_request_refresh = 1;
				} else {
					GUI_output(get_tx(36)); /* "ACH HERJE, DAS GING WOHL DANEBEN. DIE LEUTE HIER SCHEINEN GENUG VON..." */

					gs_ingame_timers[INGAME_TIMER_AKROBATIK] = HOURS(8);

					retval = -1;
				}
			}
			break;
		}
		case TA_MUSIZIEREN : {

			if (gs_ingame_timers[INGAME_TIMER_MUSIZIEREN] > 0) {

				GUI_output(get_tx(37)); /* "SCHON WIEDER DIE STIMMBÄNDER KLIRREN LASSEN?@LIEBER NICHT!" */

			} else {

				if (test_talent(hero, TA_MUSIZIEREN, handicap) > 0) {

					money = random_interval(100, 300);

					make_valuta_str(g_text_output_buf, money);

					sprintf(g_dtp2, get_tx(35), hero->alias, g_text_output_buf); /* "%s KANN DIE LEUTE BEGEISTERN!@SIE LASSEN %s SPRINGEN." */

					GUI_output(g_dtp2);

					add_party_money(money);

					gs_ingame_timers[INGAME_TIMER_MUSIZIEREN] = HOURS(8);

					g_request_refresh = 1;
				} else {
					GUI_output(get_tx(36)); /* "ACH HERJE, DAS GING WOHL DANEBEN. DIE LEUTE HIER SCHEINEN GENUG VON..." */

					gs_ingame_timers[INGAME_TIMER_MUSIZIEREN] = HOURS(8);

					retval = -1;
				}
			}
			break;
		}
		case TA_FALSCHSPIEL : {

			if (test_talent(hero, TA_FALSCHSPIEL, handicap) > 0) {

				money = random_interval(500, 1000);

				make_valuta_str(g_text_output_buf, money);

				sprintf(g_dtp2, get_tx(38), g_text_output_buf, hero->alias); /* "MAN SOLLTE ES NICHT FÜR MÖGLICH HALTEN ABER TATSÄCHLICH GELINGT DAS..." */

				GUI_output(g_dtp2);

				add_party_money(money);

				g_request_refresh = 1;
			} else {
				GUI_output(get_tx(39)); /* "ERTAPPT!@ALLE GÄSTE GEHEN WILD AUF EUCH LOS, UND EUCH BLEIBT NICHTS..." */

				gs_tav_cheated_flags[gs_town_typeindex] = 1;

				retval = -1;
			}

			break;
		}
		case TA_TASCHENDIEBSTAHL : {

			if (test_talent(hero, TA_TASCHENDIEBSTAHL, handicap) > 0) {

				money = random_interval(500, 1000);

				make_valuta_str(g_text_output_buf, money);

				sprintf(g_dtp2, get_tx(40), g_text_output_buf, hero->alias); /* "UNBEMERKT WANDERN %s IN DIE TASCHEN VON %s." */

				GUI_output(g_dtp2);

				add_party_money(money);

				g_request_refresh = 1;
			} else {
				sprintf(g_dtp2, get_tx(41), hero->alias); /* "<DIEBE - HALTET DIE DIEBE>, SCHALLT ES, ALS %s ERWISCHT WIRD.@NUR M..." */

				GUI_output(g_dtp2);

				set_party_money(0);

				g_request_refresh = 1;

				retval = -1;
			}

			break;
		}
		case TA_ALCHIMIE : {
			retval = plan_alchemy(hero);
			break;
		}
		}

		if ((tx_file_bak != -1) && (tx_file_bak != ARCHIVE_FILE_SPELLTXT_LTX)) {
			load_tx(tx_file_bak);
		}
	}

	return retval;
}

/**
 * \brief   the hero hero_pos selects a talent and uses it (status menu)
 *
 * \return              result of use_talent() or -1 if the hero cannot act
 */
signed int GUI_use_talent(const signed int hero_pos, const signed char handicap)
{
	signed int talent_id;
	const struct struct_hero *hero = get_hero(hero_pos);

	if (!check_hero(hero)) {
		return -1;
	}

	talent_id = select_talent();
	return use_talent(hero_pos, handicap, talent_id);
}

/**
 * \brief   select a talent first, then the hero who uses it (camp, inn, tavern)
 *
 * \param   handicap    handicap of the tests
 * \param   msg         question for the hero selection
 * \return              result of use_talent() or 1 if nothing was done
 */
signed int GUI_use_talent2(const signed int handicap, char *msg)
{
	signed int hero_pos;
	const signed int talent_id = select_talent();

	if (talent_id != -1) {

		g_talented_hero_pos = get_talented_hero_pos(talent_id);

		hero_pos = select_hero_ok(msg);

		if ((hero_pos != -1) && get_hero(hero_pos)->flags.brewing) {

			GUI_output(get_ttx(730)); /* "WÄHREND EINES ALCHIMISTISCHEN VERSUCHS GEHT DAS NICHT!" */
			hero_pos = -1;
		}
		if (hero_pos != -1) {
			return use_talent(hero_pos, (signed char)handicap, talent_id);
		}
	}

	return 1;
}

/**
 * \brief   does a bargain throw
 *
 * \param   hero             the hero who bargain
 * \param   items            the number of different goods
 * \param   price            the total price
 * \param   percent          how many percent the player wants to get
 * \param   handicap_init    initial value for the modificator
 * \return                   the result of the throw. A value greater than zero
 *                           means success, less or equal zero means failed.
 */
signed int bargain(const struct struct_hero *hero, const signed int items, const int32_t price,
	const signed int percent, const signed char handicap_init)
{

	signed char handicap = handicap_init;

	/* NPC Harika gets a bonus on bargain */
	if (get_hero(6)->npc_id == NPC_HARIKA) { /* Remark: not checked whether Harika is in the active group or alive */
		handicap -= 2;
	}

	/* the more different items you buy, the easier the bargain */
	handicap += (items == 1) ? 2 :
			((items == 2) ?  1 :
			((items < 5)  ?  0 :
			((items < 9)  ? -1 : -2)));

	/* the higher the price, the easier the bargain */
	handicap += (price < 100) ? 2 :
			((price < 500)  ?  1 :
			((price < 1000) ?  0 :
			((price < 2000) ? -1 : -2 )));

	/* the lower the percent, the easier the bargain */
	handicap += percent / 5 + 1;

	return test_talent(hero, TA_FEILSCHEN, handicap);
}
