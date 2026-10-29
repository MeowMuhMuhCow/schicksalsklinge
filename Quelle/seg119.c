/*
 * seg119 - Krankheiten: taegliche Wirkung
 *
 * Inhalt:    disease_effect() laeuft einmal am Tag (nach Mitternacht) fuer alle
 *            lebenden Helden und spielt die sieben Krankheiten durch: Wundfieber,
 *            Dumpfschaedel, Blaue Keuche, Paralyse, Schlachtfeldfieber,
 *            Frostschaeden, Tollwut. Verschlimmerung (LE-Verlust, Abzuege auf
 *            Eigenschaften und AT/PA, Ansteckung der Gruppe), Uebergang in die
 *            Genesung und schrittweise Rueckgabe der Abzuege. Faellt KK oder GE
 *            auf 0, stirbt der Held.
 * Aufrufer:  Spielschleife (seg002, wenn g_check_disease gesetzt), Seereise
 *            (seg063) und Landreise (seg094).
 * Daten:     je Held disease[8] (status, time_counter = Tage, log_1..log_3 =
 *            Merker fuer die schon verursachten Abzuege).
 * Besonders: Paralyse/Frostschaeden: Genesung vertauscht (Original-Bug 44),
 *            Schlachtfeldfieber endet nie (Bug 45), Meldung bei Blauer Keuche
 *            (Bug 48; der BE-Fix nutzt log_3, das nur den voruebergehenden Abzug
 *            anzeigt). Frostschaeden gehen nie von selbst in die Genesung ueber.
 *            Dumpfschaedel: "5 %" sind 4 %. Tollwut ruft rabies_frenzy() (seg120).
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg119.c
 */

#include <stdio.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg007.h"
#include "seg047.h"
#include "seg096.h"
#include "seg097.h"
#include "seg120.h"

/*
 * Each disease is described by 5 consecutive bytes.
 * The first two bytes have for all diseases the same meaning.
 * Byte 0 means:	-1 = the disease has not been healed,
 *			 0 = not diseased,
 *			 1 = the disease has been healed and the hero gets better.
 *
 * Byte 1 means:	The number of days since the infection or the healing. Depends on Byte 0.
 */

/**
 * \brief   Effects of diseases
 */
void disease_effect(void)
{
	signed int hero_pos;
	signed int j; /* multi use: hero_pos (Ansteckung), Waffentalent-Index, Zufallswert, disease_id */
	struct struct_hero *hero;
	struct struct_hero *hero2;
	struct hero_affliction *disease_ptr;

	g_check_disease = 0;

	for (hero_pos = 0; hero_pos <= 6; hero_pos++) {

		if ((get_hero(hero_pos)->typus != HERO_TYPE_NONE) && !get_hero(hero_pos)->flags.dead) {

			hero = get_hero(hero_pos);

			disease_ptr = &hero->disease[DISEASE_ID_WUNDFIEBER];

			/* TETANUS / WUNDFIEBER: get worse */
			if (disease_ptr->status == DISEASE_STATUS_DISEASED) {

				if (disease_ptr->time_counter > 13) {
					/* Hero feels better after more than 13 days (time_counter > 13) */

					disease_ptr->time_counter = 0;
					disease_ptr->status = DISEASE_STATUS_RECOVER;

					sprintf(g_dtp2, get_ttx(561), hero->alias); /* "%s GEHT ES ENDLICH WIEDER ETWAS BESSER!" */
					// "hero finally feels better"
					GUI_output(g_dtp2);

				} else {
					/* LE loss of 2D6 + 1 - (number of days the hero is diseased) */
					/* so the effect is worst at the beginning and gets better over time */
					sub_hero_le(hero, dice_roll(2, 6, -(disease_ptr->time_counter - 1)));

					/* Strength is fading, but only to 1 */
					/* (berichtigt: geprueft wird != 0, KK sinkt also bis 0; dann stirbt der Held
					 *  am Ende der Funktion) */
					if (hero->attrib[ATTRIB_KK].current != 0) {

						sprintf(g_dtp2, get_ttx(572), hero->alias); /* "%s SCHWINDEN DIE KRÄFTE!" */
						// "hero is losing some KK"
						GUI_output(g_dtp2);

						disease_ptr->log_1++;
						hero->attrib[ATTRIB_KK].current--;
					}
				}
			}

			/* TETANUS / WUNDFIEBER: get better */
			if (disease_ptr->status == DISEASE_STATUS_RECOVER) {

				if (!disease_ptr->log_1) {
					/* regeneration complete */
					disease_ptr->time_counter = 0;
					disease_ptr->status = DISEASE_STATUS_HEALTHY;
				} else {
					sprintf(g_dtp2, get_ttx(573), hero->alias); /* "LANGSAM SCHAFFT ES %s WIEDER, KRAFT ZU SCHÖPFEN." */
					// "hero regains some KK"
					GUI_output(g_dtp2);

					disease_ptr->log_1--;
					hero->attrib[ATTRIB_KK].current++;
				}
			}


			disease_ptr = &hero->disease[DISEASE_ID_DUMPFSCHAEDEL];

			/* NUMBSKULL / DUMPFSCHAEDEL: get worse */
			if (disease_ptr->status == DISEASE_STATUS_DISEASED) {

				if (!disease_ptr->log_3) {

					disease_ptr->log_3 = 1;

					for (j = 0; j < 7; j++) {
						hero->at_talent_bonus[j] -= 2;
						hero->pa_talent_bonus[j] -= 2;
					}

					hero->attrib[ATTRIB_GE].current -= 2;
					hero->attrib[ATTRIB_KK].current -= 5;

					sprintf(g_dtp2, get_ttx(574), hero->alias); /* "%s FÜHLT SICH PLÖTZLICH MATT UND HAT LEICHTES FIEBER." */
					// "hero has slight fever"
					GUI_output(g_dtp2);
				}

				if (disease_ptr->time_counter > 2) {
					/* after two days, each other hero in the group
					 * can be infectect by a chance of 20 % */

					hero2 = get_hero(0);

					for (j = 0; j <= 6; j++, hero2++) {
						if ((hero2->typus != HERO_TYPE_NONE) &&
							(hero2->group_id == gs_active_group_id) &&
							!hero2->flags.dead &&
							(hero2 != hero) &&
							(random_schick(100) <= 20))
						{
							hero_gets_diseased(hero2, DISEASE_ID_DUMPFSCHAEDEL);
						}
					}
				}

				if (random_schick(100) < 5) {
					/* 5% chance of worsening to Blaue Keuche */
					/* (tatsaechlich 4 %: random_schick(100) < 5 heisst 1..4; anderswo steht "<=") */

					/* remove the effect of Dumpfschaedel */
					disease_ptr->time_counter = 0;
					disease_ptr->status = DISEASE_STATUS_HEALTHY;

					if (disease_ptr->log_3 != 0) {

						disease_ptr->log_3 = 0;

						for (j = 0; j < 7; j++) {
							hero->at_talent_bonus[j] += 2;
							hero->pa_talent_bonus[j] += 2;
						}

						hero->attrib[ATTRIB_GE].current += 2;
						hero->attrib[ATTRIB_KK].current += 5;
					}

					/* infect with the worse Blaue Keuche */
					hero_gets_diseased(hero, DISEASE_ID_BLAUE_KEUCHE);
					/* danach laeuft der Block weiter: time_counter ist 0, es gibt heute
					 * also noch D6-1 LE Verlust durch den eben beendeten Dumpfschaedel */
				}

				if (disease_ptr->time_counter > dice_roll(1, 3, 4)) { /* number of days infected > D3 + 4 */
					// Original-Bug? forgot? disease_ptr->time_counter = 0; -- harmlos: der Genesungsblock direkt darunter setzt time_counter = 0
					disease_ptr->status = DISEASE_STATUS_RECOVER;
				} else {
					sub_hero_le(hero, dice_roll(1, 6, -1)); /* LE loss of D6 - 1 */
				}
			}

			/* NUMBSKULL / DUMPFSCHAEDEL: get better */
			if (disease_ptr->status == DISEASE_STATUS_RECOVER) {

				disease_ptr->time_counter = 0;
				disease_ptr->status = DISEASE_STATUS_HEALTHY;

				if (disease_ptr->log_3 != 0) {

					disease_ptr->log_3 = 0;

					for (j = 0; j < 7; j++) {
						hero->at_talent_bonus[j] += 2;
						hero->pa_talent_bonus[j] += 2;
					}

					hero->attrib[ATTRIB_GE].current += 2;
					hero->attrib[ATTRIB_KK].current += 5;
				}

				/* regeneration complete */
				sprintf(g_dtp2, get_ttx(575), hero->alias); /* "%sS KOPF WIRD LANGSAM WIEDER KLARER." */
				// "hero's head feals clearer"
				GUI_output(g_dtp2);
			}

			disease_ptr = &hero->disease[DISEASE_ID_BLAUE_KEUCHE];

			/* BLUE COUGH / BLAUE KEUCHE: get worse */
			if (disease_ptr->status == DISEASE_STATUS_DISEASED) {

				if (!disease_ptr->log_3) {

					disease_ptr->log_3 = 1;

					for (j = 0; j < 7; j++) {
						hero->at_talent_bonus[j] -= 4;
						hero->pa_talent_bonus[j] -= 4;
					}

					disease_ptr->log_1 = hero->attrib[ATTRIB_KK].current / 2;
					disease_ptr->log_2 = hero->attrib[ATTRIB_GE].current / 2;


					hero->attrib[ATTRIB_GE].current -= disease_ptr->log_2;
					hero->attrib[ATTRIB_KK].current -= disease_ptr->log_1;

					sprintf(g_dtp2, get_ttx(577), hero->alias); /* "%sS FIEBER VERSCHLIMMERT SICH DRASTISCH." */
					// "hero's fever gets worse"
					GUI_output(g_dtp2);

				}

				if ((disease_ptr->time_counter > 3) && (random_schick(100) <= 25)) {

					hero->attrib[ATTRIB_KK].normal--;
					hero->attrib[ATTRIB_KK].current--;
					hero->le_max -= disease_ptr->time_counter / 3;
					sub_hero_le(hero, disease_ptr->time_counter / 3);
					// Original-Bug? forgot? disease_ptr->time_counter = 0; -- harmlos: der Genesungsblock direkt darunter setzt time_counter = 0
					disease_ptr->status = DISEASE_STATUS_RECOVER;

				} else {

					hero2 = get_hero(0);

					for (j = 0; j <= 6; j++, hero2++) {
						if ((hero2->typus != HERO_TYPE_NONE) &&	(hero2->group_id == gs_active_group_id) &&
							!hero2->flags.dead && (hero2 != hero) && (random_schick(100) <= 20))
						{
							hero_gets_diseased(hero2, DISEASE_ID_BLAUE_KEUCHE);
						}
					}

					sub_hero_le(hero, dice_roll(1, 6, 2));
				}
			}

			/* BLUE COUGH / BLAUE KEUCHE: get better */
			if (disease_ptr->status == DISEASE_STATUS_RECOVER) {

				/* Original-Bug 48:
				 * When recovering from blaue Keuche, the message
				 * "Das Leben wird wohl nie wieder sein, wie es war!"
				 * ("Life will probably never be the same!")
				 * is shown whether or not a permanent attribute loss actually occurred.
				 */
#ifndef M302de_ORIGINAL_BUGFIX
				/* regeneration complete */
				sprintf(g_dtp2, get_ttx(576), hero->alias); /* "%s HAT DIE KRANKHEIT ÜBERWUNDEN, DOCH DAS LEBEN WIRD WOHL NIE WIEDE..." */
				// "hero has overcome the illness, but life will probably never be the same"
				GUI_output(g_dtp2);
#endif

				disease_ptr->time_counter = 0;
				disease_ptr->status = DISEASE_STATUS_HEALTHY;

				if (disease_ptr->log_3 != 0) {
					/* permanent attribute loss happened. */
					/* Berichtigt: log_3 zeigt nur, dass der VORUEBERGEHENDE Abzug (AT/PA -4,
					 * KK/GE halbiert) angewandt wurde. Den dauerhaften Verlust (KK -1, LE max)
					 * gibt es nur im 25-%-Zweig oben. Der Fix fuer Original-Bug 48 trifft
					 * deshalb nicht genau: nach Heilung durch Heiler/Talent/Kraut ist log_3
					 * meist schon gesetzt, die "nie wieder"-Meldung kommt trotzdem. */

					disease_ptr->log_3 = 0;

					for (j = 0; j < 7; j++) {
						hero->at_talent_bonus[j] += 4;
						hero->pa_talent_bonus[j] += 4;
					}

					hero->attrib[ATTRIB_GE].current += disease_ptr->log_2;
					hero->attrib[ATTRIB_KK].current += disease_ptr->log_1;
					disease_ptr->log_1 = disease_ptr->log_2 = 0;
#ifdef M302de_ORIGINAL_BUGFIX
					/* Original-Bug 48, see above */
					sprintf(g_dtp2, get_ttx(576), hero->alias); /* "%s HAT DIE KRANKHEIT ÜBERWUNDEN, DOCH DAS LEBEN WIRD WOHL NIE WIEDE..." */
					// %s HAT DIE KRANKHEIT ÃœBERWUNDEN, DOCH DAS LEBEN WIRD WOHL NIE WIEDER SEIN, WIE ES WAR!
				} else {
					/* permanent attribute loss did not happen. */

					sprintf(g_dtp2, "%s HAT DIE KRANKHEIT ""\x9a""BERWUNDEN.", hero->alias);
#endif

				}

#ifdef M302de_ORIGINAL_BUGFIX
				GUI_output(g_dtp2);
#endif
			}


			disease_ptr = &hero->disease[DISEASE_ID_PARALYSE];

			/* PARALYSIS / PARALYSE: get worse */
			if (disease_ptr->status == DISEASE_STATUS_DISEASED) {

				if (disease_ptr->time_counter > dice_roll(1, 3, 4)) {
					disease_ptr->time_counter = 0;
					disease_ptr->status = DISEASE_STATUS_RECOVER;
				} else {
					j = random_schick(6);
					disease_ptr->log_2 += j;
					hero->attrib[ATTRIB_GE].current -= j;

					j = random_schick(6);
					disease_ptr->log_1 += j;
					hero->attrib[ATTRIB_KK].current -= j;

					sprintf(g_dtp2, get_ttx(579), hero->alias); /* "DIE PARALYSE BEI %s NIMMT IMMER GRÖSSERE AUSMASSE AN." */
					// "hero's paralysis is worsening"
					GUI_output(g_dtp2);
				}
			}

			/* PARALYSIS / PARALYSE: get better */
			if (disease_ptr->status == DISEASE_STATUS_RECOVER) {

				if (!disease_ptr->log_1 && !disease_ptr->log_2) {

					disease_ptr->time_counter = 0;
					disease_ptr->status = DISEASE_STATUS_HEALTHY;

				} else {

					if
#ifndef M302de_ORIGINAL_BUGFIX
					/* Original-Bug 44:
					 * Recover from Paralyse and Frostschaden does not work, GE and KK are not recovered.
					 * In the two if statements below, the condition is reversed. */
					(!disease_ptr->log_1)
#else
					(disease_ptr->log_1)
#endif
					{

						sprintf(g_dtp2, get_ttx(573), hero->alias); /* "LANGSAM SCHAFFT ES %s WIEDER, KRAFT ZU SCHÖPFEN." */
						// "hero regains some KK"
						GUI_output(g_dtp2);

						disease_ptr->log_1--;
						hero->attrib[ATTRIB_KK].current++;
					}

					if
#ifndef M302de_ORIGINAL_BUGFIX
					/* Original-Bug 44, see above. */
					(!disease_ptr->log_2)
#else
					(disease_ptr->log_2)
#endif
					{

						sprintf(g_dtp2, get_ttx(578), hero->alias); /* "LANGSAM ERHOLT SICH DIE GEWANDTHEIT VON %s." */
						// "hero regains some GE"
						GUI_output(g_dtp2);

						disease_ptr->log_2--;
						hero->attrib[ATTRIB_GE].current++;
					}
				}
			}


			disease_ptr = &hero->disease[DISEASE_ID_SCHLACHTENFIEBER];

			/* BATTLEFIELD FEVER / SCHLACHTFELDFIEBER: get worse */
			if (disease_ptr->status == DISEASE_STATUS_DISEASED) {

				if (disease_ptr->time_counter > 7) {

					/* 30 % for elves, 20% for the all other types */
					if (random_schick(100) <= (hero->typus >= HERO_TYPE_AUELF ? 30 : 20)) {

						sprintf(g_dtp2, get_ttx(580), hero->alias); /* "%s ERLIEGT DEN FOLGEN DES SCHLACHTFELDFIEBERS." */
						// "hero dies"
						GUI_output(g_dtp2);

						sub_hero_le(hero, 1000);

					} else {

						if (disease_ptr->log_3) {
							// Original-Bug? forgot? disease_ptr->time_counter = 0; -- harmlos: der Genesungsblock direkt darunter setzt time_counter = 0
							disease_ptr->status = DISEASE_STATUS_RECOVER;
						}
					}
				} else {

					if (disease_ptr->time_counter > 3) {

						if (!disease_ptr->log_3) {

							disease_ptr->log_3 = 1;
							hero->attrib[ATTRIB_KK].current -= 5;

							sprintf(g_dtp2, get_ttx(581), hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST)); /* "%sS LIPPEN VERFÄRBEN SICH GELBLICH UND ES SETZEN HEFTIGE LEIBSCHMER..." */
							// "hero's lips turn yellow and he feels intense pain"
							GUI_output(g_dtp2);
						}

						for (j = 0; j <= 6; j++) {

							hero2 = get_hero(j);

							if ((hero2->typus != HERO_TYPE_NONE) &&	(hero2->group_id == gs_active_group_id) &&
								!hero2->flags.dead && (hero2 != hero) && (random_schick(100) <= 5))
							{
								hero_gets_diseased(hero2, DISEASE_ID_SCHLACHTENFIEBER);
							}
						}

						sub_hero_le(hero, dice_roll(hero->typus >= HERO_TYPE_AUELF ? 2 : 1, 6, disease_ptr->time_counter - 1));
					}
				}
			}

			/* BATTLEFIELD FEVER / SCHLACHTFELDFIEBER: get better */
			if (disease_ptr->status == DISEASE_STATUS_RECOVER) {

				/* Original-Bug 45: It is impossible to fully recover from Schlachtenfieber.
				 * There is no way to get the status back to DISEASE_STATUS_HEALTHY. */
#ifndef M302de_ORIGINAL_BUGFIX
				if (disease_ptr->log_3) {

					disease_ptr->log_3 = 0;
					disease_ptr->time_counter = 0;
					hero->attrib[ATTRIB_KK].current += 5;
				}

				sprintf(g_dtp2, get_ttx(582), hero->alias); /* "DIE GELBLICHE FÄRBUNG VON %sS LIPPEN GEHT ZURÜCK." */
				// "hero's lips do not look yellow any more"
				GUI_output(g_dtp2);
#else
				if (disease_ptr->log_3) {
					disease_ptr->log_3 = 0;
					hero->attrib[ATTRIB_KK].current += 5;
				} else {
					disease_ptr->time_counter = 0;
					disease_ptr->status = DISEASE_STATUS_HEALTHY;
					sprintf(g_dtp2, get_ttx(582), hero->alias); /* "DIE GELBLICHE FÄRBUNG VON %sS LIPPEN GEHT ZURÜCK." */
					// "hero's lips do not look yellow any more"
					GUI_output(g_dtp2);
				}
#endif
			}

			disease_ptr = &hero->disease[DISEASE_ID_FROSTSCHAEDEN];

			/* FROSTBITE / FROSTSCHAEDEN: get worse */
			if (disease_ptr->status == DISEASE_STATUS_DISEASED) {

				j = 0;

				/* (number of days * 5)% chance for *permanent* GE-1 (berichtigt, kein "+ 1") */
				/* Frostschaeden gehen nie von selbst in die Genesung ueber, nur durch Heilung. */
				if (random_schick(100) <= disease_ptr->time_counter * 5) {

					hero->attrib[ATTRIB_GE].current--;
					hero->attrib[ATTRIB_GE].normal--;

					j = 1;

					sprintf(g_dtp2, get_ttx(583), hero->alias); /* "%s NIMMT DURCH DIE FROSTSCHÄDEN PERMANENTEN SCHADEN!" */
					// "hero takes permanent damage"
					GUI_output(g_dtp2);
				}

				sub_hero_le(hero, dice_roll(1, 6, 0));

				if (hero->attrib[ATTRIB_KK].current != 0) {

					disease_ptr->log_1++;
					hero->attrib[ATTRIB_KK].current--;
				}

				if (hero->attrib[ATTRIB_GE].current != 0) {

					disease_ptr->log_2++;
					hero->attrib[ATTRIB_GE].current--;
				}

				if (j == 0) {

					sprintf(g_dtp2, get_ttx(740), hero->alias); /* "%s ZITTERT AM GANZEN LEIB. DIE FROSTSCHÄDEN TUN NOCH IMMER IHR WERK." */
					// "hero shakes"
					GUI_output(g_dtp2);
				}
			}

			/* FROSTBITE / FROSTSCHAEDEN: get better */
			if (disease_ptr->status == DISEASE_STATUS_RECOVER) {

				if (!disease_ptr->log_1 && !disease_ptr->log_2) {
					disease_ptr->time_counter = 0;
					disease_ptr->status = DISEASE_STATUS_HEALTHY;
				} else {
					if
#ifndef M302de_ORIGINAL_BUGFIX
					/* Original-Bug 44:
					 * Recover from Paralyse and Frostschaeden does not work, GE and KK are not recovered.
					 * In the two if statements below, the condition is reversed. */
					(!disease_ptr->log_1)
#else
					(disease_ptr->log_1)
#endif
					{

						sprintf(g_dtp2, get_ttx(573), hero->alias); /* "LANGSAM SCHAFFT ES %s WIEDER, KRAFT ZU SCHÖPFEN." */
						// "hero regains some KK"
						GUI_output(g_dtp2);

						disease_ptr->log_1--;
						hero->attrib[ATTRIB_KK].current++;
					}

					if
#ifndef M302de_ORIGINAL_BUGFIX
					/* Original-Bug 44, see above. */
					(!disease_ptr->log_2)
#else
					(disease_ptr->log_2)
#endif
					{

						sprintf(g_dtp2, get_ttx(578), hero->alias); /* "LANGSAM ERHOLT SICH DIE GEWANDTHEIT VON %s." */
						// "hero regains some GE"
						GUI_output(g_dtp2);

						disease_ptr->log_2--;
						hero->attrib[ATTRIB_GE].current++;
					}
				}
			}

			disease_ptr = &hero->disease[DISEASE_ID_TOLLWUT];

			/* RABIES / TOLLWUT: get worse */
			if (disease_ptr->status == DISEASE_STATUS_DISEASED) {

				if (disease_ptr->time_counter > dice_roll(1, 6, 6)) {

					disease_ptr->time_counter = 0;
					disease_ptr->status = DISEASE_STATUS_RECOVER;

				} else {

					hero2 = get_hero(0);

					for (j = 0; j <= 6; j++, hero2++) {


						if ((hero2->typus != HERO_TYPE_NONE) &&
							(hero2->group_id == gs_active_group_id) &&
							!hero2->flags.dead &&
							(hero2 != hero) &&
							(random_schick(100) <= 10))
						{
							hero_gets_diseased(hero2, DISEASE_ID_TOLLWUT);
						}
					}

					sub_hero_le(hero, dice_roll((disease_ptr->time_counter > 3 ? 3 : disease_ptr->time_counter), 6, 0));

					disease_ptr->log_1 += 2;
					hero->attrib[ATTRIB_KK].current -= 2;

					if (disease_ptr->time_counter > 2) {

						rabies_frenzy(hero, hero_pos);

					} else {

						sprintf(g_dtp2, get_ttx(572), hero->alias); /* "%s SCHWINDEN DIE KRÄFTE!" */
						// "hero is losing some KK"
						GUI_output(g_dtp2);
					}
				}
			}

			/* RABIES / TOLLWUT: get better */
			if (disease_ptr->status == DISEASE_STATUS_RECOVER) {

				if (!disease_ptr->log_1) {

					disease_ptr->time_counter = 0;
					disease_ptr->status = DISEASE_STATUS_HEALTHY;

				} else {
					sprintf(g_dtp2, get_ttx(573), hero->alias); /* "LANGSAM SCHAFFT ES %s WIEDER, KRAFT ZU SCHÖPFEN." */
					// "hero regains some KK"
					GUI_output(g_dtp2);

					disease_ptr->log_1--;
					hero->attrib[ATTRIB_KK].current++;
				}
			}

			/* increment day timer for all diseases */
			/* (Frostschaeden und die Genesung vom Schlachtfeldfieber enden nie von selbst;
			 *  nach 127 Tagen wird time_counter negativ) */
			for (j = 1; j <= 7; j++) {

				if (hero->disease[j].status != DISEASE_STATUS_HEALTHY) {
					/* Original-Bug? I think for some of the diseases, an overflow could theoretically happen. */
					hero->disease[j].time_counter++;
				}
			}

			/* kill hero if KK <= 0 */
			if (hero->attrib[ATTRIB_KK].current <= 0) {

				hero->attrib[ATTRIB_KK].current = hero->attrib[ATTRIB_KK].normal;

				sub_hero_le(hero, 5000);
			}

			/* kill hero if GE <= 0 */
			if (hero->attrib[ATTRIB_GE].current <= 0) {

				hero->attrib[ATTRIB_GE].current = hero->attrib[ATTRIB_GE].normal;

				sub_hero_le(hero, 5000);
			}
		}
	}
}
