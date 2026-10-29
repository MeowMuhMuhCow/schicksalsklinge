/*
 * seg108 - Essen, Trinken, Kräuter und Tränke
 *
 * Inhalt:    consume: ein Held (consumer) verzehrt einen Gegenstand aus dem
 *            Inventar eines Helden (owner): Nahrung senkt Hunger, Getränke
 *            senken Durst (Wasserschlauch halb/leer, Wein und Schnaps machen
 *            betrunken), Kräuter (Gulmond, Einbeere, Belmart, Menchal, Atmon,
 *            Wirselkraut, Tarnele) und Tränke (Heil-, Zauber-, Schlaftrunk,
 *            Wunderkur, Gegengift, Elixiere) wirken sofort oder über
 *            Zeitmodifikatoren. Leere Flaschen kommen zurück ins Inventar.
 * Aufrufer:  Statusmenü (Mund, seg048), use_item (seg107), automatisches
 *            Essen und Trinken (seg002).
 * Daten:     keine eigenen.
 * Besonders: Tarnele gibt keinen Text aus (leere Meldung). Tränke ohne eigenen
 *            Fall geben einen alten Text in g_dtp2 aus (Bright-Eyes-Hinweis).
 *            Antikrankheitselixier ohne Krankheit ändert disease[0] (harmlos).
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg108.c
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg007.h"
#include "seg047.h"
#include "seg096.h"
#include "seg097.h"
#include "seg105.h"

#if FIX_0404
/* Fix 404: laufende Elixierwirkung (+5) auf derselben Eigenschaft des Helden suchen, -1 wenn keine */
static signed int fix_elixir_slot(const struct struct_hero *hero, const uint8_t *ptr)
{
	signed int i;

	if (hero->timer_id != 0) {
		for (i = 0; i < 100; i++) {
			if ((gs_modification_timers[i].time_left > 0) && (gs_modification_timers[i].target == hero->timer_id) &&
				(gs_modification_timers[i].modifier == 5) &&
				(gs_modification_timers[i].offset == (uint16_t)(ptr - (const uint8_t*)hero)))
			{
				return i;
			}
		}
	}
	return -1;
}
#endif

/**
 * \brief   consumer eats, drinks or takes an item from the inventory of owner
 *
 * \param   owner       the owner
 * \param   consumer    the consumer
 * \param   inv_slot    the inventory slot of the owner
 */
void consume(struct struct_hero *owner, struct struct_hero *consumer, const signed int inv_slot)
{
	struct item_stats *item_desc;
	signed int item_id;

	signed int bad_elixir_item_id;
	signed int le_diff;

	signed int disease_id;
	signed int poison_id;
	signed int tw_bak;
	signed int consumer_hero_pos;

	signed int tmp; /* multiple use: mod_slot, random_le_gain */
	signed int tmp2; /* multiple use: talent_id, flag, attrib_id, etc. */

	/* this hero cannot consume */
	if (check_hero(consumer) == 0) {
		return;
	}

	consumer_hero_pos = get_hero_pos(consumer);

	/* get item id */
	item_id = owner->inventory[inv_slot].item_id;

	/* get pointer to ITEMS.DAT */
	item_desc = &g_itemsdat[item_id];

	/* is nutrition */
	if (item_desc->flags.nutrition) {

		if (item_desc->item_subtype_id == NUTRITION_TYPE_FOOD) {
			/* eating */

#if !defined(__BORLANDC__)
				int diff = consumer->hunger - item_desc->item_type_stats_id;
				/* note that, for nutrition, item_type_stats_id contains the nutrition value */

				D1_INFO("%s isst %s mit Naehrwert %d. Der Hunger sinkt von %d auf %d\n",
					consumer->alias, GUI_name_base_form(g_itemsname[item_id]),
					item_desc->item_type_stats_id,
					consumer->hunger, (diff >= 0) ? diff : 0);
#endif

			/* subtract from hunger value */
			consumer->hunger -= item_desc->item_type_stats_id;

			/* adjust hunger value */
			if (consumer->hunger < 0) {
				consumer->hunger = 0;
			}

			/* message unless g_consume_quiet is set */
			if (!g_consume_quiet) {
				GUI_output(get_ttx(207)); /* "MMMHH, DAS STILLT DEN HUNGER." */
			}

			/* drop one unit of that item */
			drop_item(owner, inv_slot, 1);
		} else {
			/* drinking */

			/* check if item is not empty */
			if (!owner->inventory[inv_slot].flags.empty) {

#if !defined(__BORLANDC__)
				int diff = consumer->thirst - item_desc->item_type_stats_id;
				D1_INFO("%s trinkt aus %s mit Naehrwert %d. Der Durst sinkt von %d auf %d\n",
					consumer->alias, GUI_name_base_form(g_itemsname[item_id]), item_desc->item_type_stats_id,
					consumer->thirst, (diff >= 0) ? diff : 0);
#endif

				/* subtract from thirst value */
				consumer->thirst -= item_desc->item_type_stats_id;

				/* adjust thirst value */
				if (consumer->thirst < 0) {
					consumer->thirst = 0;
				}

				/* message unless g_consume_quiet is set */
				if (!g_consume_quiet) {
					GUI_output(get_ttx(208)); /* "KÖSTLICH ERFRISCHEND." */
				}


				if (item_id == ITEM_ID_WASSERSCHLAUCH) {
					/* water */

					if (owner->inventory[inv_slot].flags.half_empty) {
						owner->inventory[inv_slot].flags.empty = 1;
					} else {
						owner->inventory[inv_slot].flags.half_empty = 1;
					}

				} else if (item_id == ITEM_ID_SCHNAPSFLASCHE || item_id == ITEM_ID_WEINFLASCHE) {
					/* wine or snaps */
					hero_get_drunken(consumer);
					drop_item(owner, inv_slot, 1);
				} else {
					/* everything else: Beer */
					drop_item(owner, inv_slot, 1);

					/* That does not happen (only beer gets here) */
					if (item_id != ITEM_ID_BIER) {
						/* get an empty glass bottle */
						give_new_item_to_hero(owner, ITEM_ID_GLASFLASCHE, 2, 1);
					}
				}
			} else {
				/* print empty message */
				GUI_output(get_ttx(429)); /* "LEIDER NICHTS MEHR DRIN!" */
			}
		}

		g_request_refresh = 1;

	} else if (item_desc->flags.herb_potion) {

		if (item_desc->item_subtype_id == HERB_POTION_TYPE_HERB_AND_POISON) {

			if (is_in_int_array(item_id, g_herbs_uneatable)) {

				GUI_output(get_ttx(499)); /* "DIESES KRAUT IST IN UNBEHANDELTEM ZUSTAND UNGENIESSBAR!" */

			} else if (is_in_int_array(item_id, g_herbs_toxic) || is_in_int_array(item_id, g_weapon_poisons)) {
				/* herbs and poisons */
				GUI_output(get_ttx(500)); /* "WILLST DU MICH VERGIFTEN?@DAS ZEUG NEHME ICH SICHER NICHT IN DEN MUND." */

			} else {
				/* consume with effects */

				/* drop one entity */
				drop_item(owner, inv_slot, 1);

				/* terminate output string */
				*g_dtp2 = '\0';

				switch (item_id) {
				case ITEM_ID_GULMOND_BLATT: {
					/* Gulmond Blatt */

					/* KK+2 for 12h */
					tmp = get_free_mod_slot();
					set_mod_slot(tmp, HOURS(12), (uint8_t*)(&consumer->attrib[ATTRIB_KK].current), 2, (signed char)consumer_hero_pos);

					/* LE + 2 */
					add_hero_le(consumer, 2);

					/* prepare output */
					strcpy(g_dtp2, get_ttx(501)); /* "HERRLICH!@NACH SO EINER PORTION GULMOND FÜHLT MAN SICH GLEICH, ALS..." */
					break;
				}
				case ITEM_ID_EINBEERE: {
					/* Vierblaettrige Einbeere */

					tmp = random_schick(6);
					le_diff = consumer->le_max - consumer->le;
					if (tmp > le_diff)
						tmp = le_diff;

					add_hero_le(consumer, tmp);

					/* prepare output */
					sprintf(g_dtp2, get_ttx(502), tmp); /* "DIE VIERBLÄTTRIGE EINBEERE HAT %d LE-PUNKTE GEBRACHT." */

					break;
				}
				case ITEM_ID_BELMART_BLATT: {
					/* Belmart */
					poison_id = hero_is_poisoned(consumer);

					if (poison_id != POISON_ID_NONE && poison_id <= 5) {

						/* cure the first poison */
						consumer->poison[poison_id].time_counter = 0;
						consumer->poison[poison_id].status = POISON_STATUS_RECOVER;
					}

					/* dummy: consumer->herbs is write-only */
					tmp = get_free_mod_slot();
					set_mod_slot(tmp, DAYS(1), (uint8_t*)(&consumer->herbs), 1, (signed char)consumer_hero_pos);

					sprintf(g_dtp2, get_ttx(503), consumer->alias); /* "%s IST JETZT DEUTLICH WIDERSTANDSFÄHIGER GEGENÜBER KRANKHEITEN UND..." */
					// %s IST JETZT DEUTLICH WIDERSTANDSFÃ„HIGER GEGENÃœBER KRANKHEITEN UND GIFTEN.
					break;
				}
				case ITEM_ID_MENCHALKAKTUS: {
					/* Menchalkaktus */
					poison_id = hero_is_poisoned(consumer);

					if (poison_id != POISON_ID_NONE && poison_id <= 10) {
						/* cure the first poison */
						consumer->poison[poison_id].time_counter = 0;
						consumer->poison[poison_id].status = POISON_STATUS_RECOVER;
					}

					/* dummy: consumer->herbs is write-only */
					tmp = get_free_mod_slot();
					set_mod_slot(tmp, DAYS(1), (uint8_t*)(&consumer->herbs), 2, (signed char)consumer_hero_pos);

					sprintf(g_dtp2, get_ttx(503), consumer->alias); /* "%s IST JETZT DEUTLICH WIDERSTANDSFÄHIGER GEGENÜBER KRANKHEITEN UND..." */
					// %s IST JETZT DEUTLICH WIDERSTANDSFÃ„HIGER GEGENÃœBER KRANKHEITEN UND GIFTEN.
					break;
				}
				case ITEM_ID_ATMONBLUETE: {
					/* Atmon */
					for (tmp2 = 9; tmp2 < 19; tmp2++) {
						/* All body talents + 2 for 5h */
						tmp = get_free_mod_slot();
						set_mod_slot(tmp, HOURS(5), (uint8_t*)(&consumer->talents[tmp2]), 2, (signed char)consumer_hero_pos);
					}

					/* prepare output */
					sprintf(g_dtp2, get_ttx(504), consumer->alias); /* "%s FÜHLT SICH NACH DER EINNAHME DES ATMON WIE EIN AKROBAT!" */
					break;
				}
				case ITEM_ID_WIRSELKRAUT: {
					/* Wirselkraut */
					tmp = 10;
					le_diff = consumer->le_max - consumer->le;
					if (le_diff < tmp)
						tmp = le_diff;

					add_hero_le(consumer, tmp);
					/* prepare output */

					sprintf(g_dtp2, get_ttx(505), tmp); /* "DAS WIRSELKRAUT HAT %d LE-PUNKTE GEBRACHT." */
					break;
				}
				case ITEM_ID_TARNELE: {
					/* Tarnelle: doubled regeneration in the next night */
					/* Remark: no text is prepared, GUI_output() below gets the empty string */
					consumer->ruhe_koerper = 1;
#if FIX_0226
					/* Fix 226 (teilweise): keinen alten Text zeigen; ein leerer Text oeffnet kein Fenster */
					*g_dtp2 = '\0';
#endif
					break;
				}
				}

				/* print the output */
				GUI_output(g_dtp2);
			}
		} else {

			/* check if item is an elixir */
			tmp2 = is_in_int_array(item_id, g_elixir_potions);
			bad_elixir_item_id = is_in_int_array(item_id, g_bad_elixirs);

			if (tmp2 != 0) {
				/* handle good elixirs */

				/* drop elixir */
				drop_item(owner, inv_slot, 1);

				/* get glassbottle */
				give_new_item_to_hero(owner, ITEM_ID_GLASFLASCHE, 2, 1);

				/* Attribute +5 for 1h */
#if FIX_0404
				/* Fix 404: wirkt schon ein Elixier auf diese Eigenschaft, nur seine Dauer erneuern (kein weiteres +5) */
				if ((tmp = fix_elixir_slot(consumer, (uint8_t*)(&consumer->attrib[tmp2 - 1].current))) != -1) {
					gs_modification_timers[tmp].time_left = HOURS(1);
				} else
#endif
				{
				tmp = get_free_mod_slot();
				set_mod_slot(tmp, HOURS(1), (uint8_t*)(&consumer->attrib[tmp2 - 1].current), 5, (signed char)consumer_hero_pos);
				}

				/* prepare output */
				sprintf(g_dtp2, get_ttx(509), consumer->alias, get_ttx(411 + tmp2), 5); /* "BEI %s SCHNELLT %s UM %d PUNKTE IN DIE HÖHE!" / TEXT.LTX 412..: Eigenschaft */

				/* print output */
				GUI_output(g_dtp2);

			} else if (bad_elixir_item_id != 0) {
				/* handle bad elixires */

				/* drop elixir */
				drop_item(owner, inv_slot, 1);

				/* get glassbottle */
				give_new_item_to_hero(owner, ITEM_ID_GLASFLASCHE, 2, 1);

				/* Attribute -7 for 1h */
				tmp = get_free_mod_slot();
				set_mod_slot(tmp, HOURS(1), (uint8_t*)(&consumer->attrib[bad_elixir_item_id - 1].current), -7, (signed char)consumer_hero_pos);

				/* prepare output */
				sprintf(g_dtp2, get_ttx(656), consumer->alias, get_ttx(411 + bad_elixir_item_id), 7); /* "BEI %s SINKT %s UM %d PUNKTE!" / TEXT.LTX 412..: Eigenschaft */

				/* print output */
				GUI_output(g_dtp2);

			} else {
				/* everything else */

				/* drop the item */
				drop_item(owner, inv_slot, 1);

				switch (item_id) {
				case ITEM_ID_HEILTRANK : {
					/* Heiltrank */

					tmp2 = consumer->le_max - consumer->le;
					if (tmp2 > 10)
						tmp2 = 10;

					/* singular POINT/ PUNKT */
					strcpy(g_text_output_buf, get_ttx(392)); /* "PUNKT" */

					if (tmp2 > 1) {
						/* make plural POINTS/PUNKTE */
						strcat(g_text_output_buf, get_ttx(393)); /* "E" */
					}

					/* add LE */
					add_hero_le(consumer, tmp2);

					/* give owner a glassbottle */
					give_new_item_to_hero(owner, ITEM_ID_GLASFLASCHE, 2, 1);

					/* prepare output */
					sprintf(g_dtp2, get_ttx(510), consumer->alias, tmp2, g_text_output_buf); /* "DER HEILTRANK GIBT %s %d LE-%s ZURÜCK." */
					break;
				}
				case  ITEM_ID_STARKER_HEILTRANK : {
					/* Starker Heiltrank */

					/* 1W20+10 */
					tmp2 = dice_roll(1, 20, 10);
					if (consumer->le_max - consumer->le < tmp2)
						tmp2 = consumer->le_max - consumer->le;

					/* add LE */
					add_hero_le(consumer, tmp2);

					/* give owner a bronze flask */
					give_new_item_to_hero(owner, ITEM_ID_BRONZEFLASCHE, 2, 1);

					/* singular POINT/ PUNKT */
					strcpy(g_text_output_buf, get_ttx(392)); /* "PUNKT" */

					if (tmp2 > 1) {
						/* make plural POINTS/PUNKTE */
						strcat(g_text_output_buf, get_ttx(393)); /* "E" */
					}

					/* prepare output */
					sprintf(g_dtp2, get_ttx(510), consumer->alias, tmp2, g_text_output_buf); /* "DER HEILTRANK GIBT %s %d LE-%s ZURÜCK." */
					break;
				}
				case ITEM_ID_WUNDERKUR: {
					/* Wunderkur */

					/* undo starvation damage */
					consumer->le_max += consumer->le_max_malus;
					consumer->le_max_malus = 0;

					/* fill up LE */
					if (consumer->le < consumer->le_max) {
						add_hero_le(consumer, consumer->le_max);
					}

					/* diseases, not all */
					disease_id = hero_is_diseased(consumer);

					if (disease_id == DISEASE_ID_DUMPFSCHAEDEL || disease_id == DISEASE_ID_BLAUE_KEUCHE) {
						consumer->disease[disease_id].status = DISEASE_STATUS_RECOVER;
						consumer->disease[disease_id].time_counter = 0;
					}

					/* poison */
					poison_id = hero_is_poisoned(consumer);

					if (poison_id != POISON_ID_NONE && poison_id < 7) {
						consumer->poison[poison_id].time_counter = 0;
						consumer->poison[poison_id].status = POISON_STATUS_RECOVER;
					}

					/* give owner a glasbottle */
					give_new_item_to_hero(owner, ITEM_ID_GLASFLASCHE, 2, 1);

					/* prepare output */
					sprintf(g_dtp2, get_ttx(511), consumer->alias, GUI_get_personal_pronoun(consumer->sex, GRAMMAR_CASE_1ST)); /* "DIESE WUNDERKUR LÄSST %sS LE-PUNKTE WIEDER AUF DEN MAXIMALWERT SCHN..." */

					break;
				}
				case ITEM_ID_SCHLAFTRUNK: {
					/* Schlaftrunk */

					/* doubled regeneration for the next 3 nights (see GRP_hero_sleep) */
					consumer->ruhe_koerper = 3;

					/* give owner a glasbottle */
					give_new_item_to_hero(owner, ITEM_ID_GLASFLASCHE, 2, 1);

					/* prepare output */
					sprintf(g_dtp2, get_ttx(738), consumer->alias); /* "MMMHHH - %s WIRD DIE NÄCHSTEN DREI NÄCHTE SICHER SCHLAFEN WIE EIN B..." */
					break;
				}
				case ITEM_ID_ZAUBERTRANK: {
					/* Zaubertrank */

					if (consumer->typus >= HERO_TYPE_HEXE) {
						/* Magicuser */

						tmp2 = consumer->ae_max - consumer->ae;

						if (tmp2 > 10)
							tmp2 = 10;

						/* add AE */
						add_hero_ae(consumer, tmp2);

						/* give hero a glassbottle */
						give_new_item_to_hero(owner, ITEM_ID_GLASFLASCHE, 2, 1);

						/* prepare output */
						strcpy(g_text_output_buf, get_ttx(392)); /* "PUNKT" */

						if (tmp2 > 1) {
							strcat(g_text_output_buf, get_ttx(393)); /* "E" */
						}

						sprintf(g_dtp2, get_ttx(512), tmp2, g_text_output_buf, consumer->alias); /* "DER ZAUBERTRANK REGENERIERT %d AE-%s BEI %s." */
					} else {
						/* Not a magicuser */

						strcpy(g_dtp2, get_ttx(804)); /* "DAS HATTE WOHL WENIG SINN!" */

						/* give owner a glassbottle */
						give_new_item_to_hero(owner, ITEM_ID_GLASFLASCHE, 2, 1);
					}

					break;
				}
				case ITEM_ID_ZAUBERTRANK_STARK: {
					/* Zaubertrank (stark) */

					if (consumer->typus >= HERO_TYPE_HEXE) {
						/* Magicuser */

						tmp2 = consumer->ae_max - consumer->ae;

						if (tmp2 > 30)
							tmp2 = 30;

						/* add AE */
						add_hero_ae(consumer, tmp2);

						/* give owner a glassbottle */
						give_new_item_to_hero(owner, ITEM_ID_GLASFLASCHE, 2, 1);

						/* prepare output */
						if (consumer->ae >= consumer->ae_max) {
							sprintf(g_dtp2, get_ttx(513), consumer->alias); /* "DIESES ZAUBERTRANKKONZENTRAT LÄSST %sS AE-PUNKTE WIEDER AUF@DEN MAX..." */
						} else {
							strcpy(g_text_output_buf, get_ttx(392)); /* "PUNKT" */

							if (tmp2 > 1) {
								strcat(g_text_output_buf, get_ttx(393)); /* "E" */
							}

							sprintf(g_dtp2, get_ttx(512), tmp2, g_text_output_buf, consumer->alias); /* "DER ZAUBERTRANK REGENERIERT %d AE-%s BEI %s." */
						}
					} else {
						/* Not a magicuser */
						strcpy(g_dtp2, get_ttx(804)); /* "DAS HATTE WOHL WENIG SINN!" */

						/* give owner a glassbottle */
						give_new_item_to_hero(owner, ITEM_ID_GLASFLASCHE, 2, 1);
					}
					break;
				}
				case ITEM_ID_GEGENGIFT: {
					/* Gegengift */

					poison_id = hero_is_poisoned(consumer);

					if (poison_id != POISON_ID_NONE && poison_id < 5) {
						consumer->poison[poison_id].time_counter = 0;
						consumer->poison[poison_id].status = POISON_STATUS_RECOVER;
					}

					/* give owner a glassbottle */
					give_new_item_to_hero(owner, ITEM_ID_GLASFLASCHE, 2, 1);

					sprintf(g_dtp2, get_ttx(467), consumer->alias); /* "<TSA SEI GEDANKT. %s IST WIEDER VÖLLIG IN ORDNUNG!>" */
					break;
				}
				case ITEM_ID_ANTIKRANKHEITSELIXIER: {
					/* Antikrankheitselixier */
					/* Remark: if the hero is not diseased, disease[0] (unused) is changed */

					disease_id = hero_is_diseased(consumer);

#if FIX_0227
					/* Fix 227: ohne Krankheit nichts aendern (disease[0] ist unbenutzt) */
					if (disease_id != 0)
#endif
					{
					consumer->disease[disease_id].status = DISEASE_STATUS_RECOVER;
					consumer->disease[disease_id].time_counter = 0;
					}

					/* give owner a glassbottle */
					give_new_item_to_hero(owner, ITEM_ID_GLASFLASCHE, 2, 1);

					sprintf(g_dtp2, get_ttx(467), consumer->alias); /* "<TSA SEI GEDANKT. %s IST WIEDER VÖLLIG IN ORDNUNG!>" */
					break;

				}
				}

				/* print output: maybe garbage - string not set to 0 */
				tw_bak = g_textbox_width;
				g_textbox_width = 5;
				GUI_output(g_dtp2);
				g_textbox_width = tw_bak;
			}
		}

		g_request_refresh = 1;
	} else {
		/* this item cannot be consumed */
		GUI_output(get_ttx(206)); /* "DAS KANN MAN WEDER ESSEN NOCH TRINKEN!" */
	}
}
