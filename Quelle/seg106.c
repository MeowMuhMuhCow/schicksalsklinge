/*
 * seg106 - Inventar: Umlegen, Übergeben, Startausrüstung
 *
 * Inhalt:    Gegenstände im eigenen Inventar umlegen (mit Prüfung von
 *            Tragplatz, Zweihandwaffen und Stapeln), an einen anderen Helden
 *            übergeben, Beschreibung anzeigen ("DIES IST ..."), die Start-
 *            ausrüstung eines neuen Helden, die längste Brenndauer einer
 *            Lichtquelle, die Gürtel-Animation und die Suche nach einem vollen
 *            Wasserschlauch.
 * Aufrufer:  move_item/pass_item/print_item_description aus dem Statusmenü
 *            (seg048), move_item auch aus seg033, seg036, seg041;
 *            startup_equipment aus seg026; get_max_light_time aus seg075;
 *            get_full_waterskin_inv_slot aus seg002; equip_belt_ani aus seg105.
 * Daten:     Startgegenstände je Heldentyp und für alle.
 * Besonders: Original-Bug: get_full_waterskin_inv_slot sucht nur im Rucksack.
 *            Neu: pass_item zählt beim Tausch die belegten Plätze falsch und
 *            beim Übergeben eines Stapels in einen leeren Platz gar nicht;
 *            move_item zwischen zwei Körperplätzen prüft nur eine Richtung.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg106.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__BORLANDC__)
#include <IO.H>
#else
#include <unistd.h>
#endif

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg004.h"
#include "seg007.h"
#include "seg008.h"
#include "seg096.h"
#include "seg097.h"
#include "seg105.h"
#include "seg106.h"

static const signed int g_hero_startup_items[12][4] = {
	{ ITEM_ID_DOLCH, ITEM_ID_WURFMESSER, ITEM_ID_WURFMESSER, -1 }, /* Gaukler */
	{ ITEM_ID_LANGBOGEN, ITEM_ID_DOLCH, -1, -1 }, /* Jaeger */
	{ ITEM_ID_SCHWERT, ITEM_ID_DOLCH, ITEM_ID_LEDERHARNISCH, -1 }, /* Krieger */
	{ ITEM_ID_RAPIER, ITEM_ID_DOLCH, ITEM_ID_DIETRICHE, -1 }, /* Streuner */
	{ ITEM_ID_SKRAJA, ITEM_ID_SAEBEL, ITEM_ID_SCHNAPSFLASCHE, -1 }, /* Thorwaler */
	{ ITEM_ID_STREITKOLBEN, ITEM_ID_BRECHEISEN, ITEM_ID_HAMMER, -1 }, /* Zwerg */
	{ ITEM_ID_HEXENBESEN, ITEM_ID_EINBEERE, ITEM_ID_EINBEERE, -1 }, /* Hexe */
	{ ITEM_ID_VULKANGLASDOLCH, ITEM_ID_WIRSELKRAUT, ITEM_ID_JORUGAWURZEL, -1 }, /* Druide */
	{ ITEM_ID_ZAUBERSTAB, ITEM_ID_DOLCH, ITEM_ID_SCHREIBZEUG, ITEM_ID_ROBE__GREEN_1 }, /* Magier */
	{ ITEM_ID_LANGBOGEN, ITEM_ID_RAPIER, ITEM_ID_HARFE, -1 }, /* Auelf */
	{ ITEM_ID_ROBBENTOETER, ITEM_ID_SPEER, ITEM_ID_FLOETE, -1 }, /* Firnelf */
	{ ITEM_ID_LANGBOGEN, ITEM_ID_MESSER, ITEM_ID_FLOETE, -1 } /* Waldelf */
}; // ds:0xae48

static const signed int g_hero_startup_items_all[4] = {
	ITEM_ID_WASSERSCHLAUCH, ITEM_ID_PROVIANTPAKET, ITEM_ID_PROVIANTPAKET, ITEM_ID_HOSE
}; // ds:0xaea8

/**
 * \brief   check for a two hand collision
 *
 *          A hero can not equip a two-handed weapon if one hand is in use,
 *          nor can he equip something in his free hand if he has a two-handed weapon
 *          in the other.
 *          If such a collision is detected this function returns 1 else 0.
 *
 * \param   hero        the hero
 * \param   item_id     the item which should be equipped
 * \param   inv_slot    the inventory slot where the item should be placed
 */
/* Borlandified and identical */
signed int two_hand_collision(struct struct_hero* hero, const signed int item_id, const signed int inv_slot)
{
	signed int retval = 0;
	signed int other_inv_slot;
	signed int other_item_id;

	if (inv_slot == HERO_INVENTORY_SLOT_RIGHT_HAND || inv_slot == HERO_INVENTORY_SLOT_LEFT_HAND) {

		other_inv_slot = HERO_INVENTORY_SLOT_RIGHT_HAND;

		if (inv_slot == HERO_INVENTORY_SLOT_RIGHT_HAND) {
			other_inv_slot = HERO_INVENTORY_SLOT_LEFT_HAND;
		}

		/* get the item in the other hand */
		other_item_id = hero->inventory[other_inv_slot].item_id;
		if (other_item_id) {

			/* check if one hand has a two-handed weapon */
			if ((g_itemsdat[item_id].flags.weapon && (g_itemsdat[item_id].item_subtype_id == WEAPON_TYPE_ZWEIHAENDER)) ||
			(g_itemsdat[other_item_id].flags.weapon && (g_itemsdat[other_item_id].item_subtype_id == WEAPON_TYPE_ZWEIHAENDER))) {
				retval = 1;
			}
		}
	}

	return retval;
}

/* Borlandified and identical */
/**
 * \brief   moves an item inside the inventory of a hero
 *
 *          The contents of the two slots are exchanged (or stacks merged), so the
 *          direction does not matter: the status menu passes (selected slot, target slot),
 *          startup_equipment passes (body slot, knapsack slot).
 *          Internally the slots are swapped so that inv_slot_1 is the body slot and
 *          item_id_2 is the item that gets equipped there.
 *          Remark: if both slots are body slots, only item_id_2 is checked and equipped
 *          at inv_slot_1. The displaced item_id_1 lands in inv_slot_2 without a check
 *          (e.g. a shield in the right hand) and without add_equip_boni(), while unequip()
 *          is done again when it is removed from there later (RS/BE drift).
 *
 * \param   inv_slot_1  first slot
 * \param   inv_slot_2  second slot
 * \param   hero        the hero
 */
void move_item(signed int inv_slot_1, signed int inv_slot_2, struct struct_hero *hero)
{
	signed int item_id_1;
	signed int item_id_2;
	signed int do_exchange = 0;
	signed int temp;
	struct inventory tmp;

	if (!check_hero(hero) || (inv_slot_1 == inv_slot_2)) {

	} else {

		if ((inv_slot_2 > HERO_INVENTORY_SLOT_KNAPSACK_1 - 1) && (inv_slot_1 > HERO_INVENTORY_SLOT_KNAPSACK_1 - 1)) {
			/* Both items are in knapsacks */
			do_exchange = 1;
			item_id_1 = hero->inventory[inv_slot_1].item_id;
			item_id_2 = hero->inventory[inv_slot_2].item_id;
		} else {
			item_id_1 = hero->inventory[inv_slot_1].item_id;
			item_id_2 = hero->inventory[inv_slot_2].item_id;

			if ((inv_slot_2 < inv_slot_1) || ((inv_slot_1 < HERO_INVENTORY_SLOT_KNAPSACK_1) && (inv_slot_2 < HERO_INVENTORY_SLOT_KNAPSACK_1))) {

				if (inv_slot_1 < HERO_INVENTORY_SLOT_KNAPSACK_1) {
					if (item_id_1 != ITEM_ID_NONE)
						do_exchange = 1;
				} else {
					do_exchange = 1;
				}

				if (do_exchange != 0) {
					/* exchange positions */
					temp = inv_slot_1;
					inv_slot_1 = inv_slot_2;
					inv_slot_2 = temp;

					/* exchange ids */
					temp = item_id_1;
					item_id_1 = item_id_2;
					item_id_2 = temp;
				}
			}

			do_exchange = 0;

			if ((item_id_1 == 0) && (item_id_2 == 0)) {

				GUI_output(get_ttx(209)); /* "ZUNÄCHST MUSS EIN ZULÄSSIGER GEGENSTAND AUSGEWÄHLT WERDEN." */

			} else {
				if (item_id_2 != ITEM_ID_NONE) {

					/* item have the same ids and are stackable */
					if ((item_id_2 == item_id_1) && g_itemsdat[item_id_1].flags.stackable) {
						/* merge them */

						/* add quantity of item at inv_slot_2 to item at inv_slot_1 */
						hero->inventory[inv_slot_1].quantity += hero->inventory[inv_slot_2].quantity;
						/* Original-Bug: Might result in a stack size bigger than STACK_SIZE_MAX */

						/* delete item at inv_slot_2 */
						memset((uint8_t*)&hero->inventory[inv_slot_2], 0, sizeof(struct inventory));
#ifdef M302de_ORIGINAL_BUGFIX
						/* Decrement the item counter */
						hero->num_filled_inv_slots--;
#endif
					} else {
						if (!can_hero_use_item(hero, item_id_2)) {

							sprintf(g_dtp2, get_ttx(221), hero->alias, /* "%s ALS %s KANN %s NICHT VERWENDEN!" */
								get_ttx((hero->sex != 0 ? 593 : 9) + hero->typus), /* TEXT.LTX 10..21 / 594..605: Heldentyp (m/w) */
								GUI_name_inflect_with_article(
									INFLECT_INDEFINITE_ARTICLE | INFLECT_SINGULAR | INFLECT_4TH_CASE,
									item_id_2,
									INFLECT_NAME_TYPE_ITEM
								));

							GUI_output(g_dtp2);

						} else {
							if (!can_hero_equip_item_at_slot(item_id_2, inv_slot_1)) {

								if (is_in_int_array(item_id_2, g_items_pluralwords))

									sprintf(g_dtp2, get_ttx(222), GUI_name_inflect_with_article( /* "%s %s AN DIESER POSITION NICHT GETRAGEN WERDEN!" */
										INFLECT_OMIT_ARTICLE | INFLECT_SINGULAR | INFLECT_1ST_CASE,
										item_id_2,
										INFLECT_NAME_TYPE_ITEM
									), get_ttx(557)); /* "KÖNNEN" */
								else
									sprintf(g_dtp2, get_ttx(222), GUI_name_inflect_with_article( /* "%s %s AN DIESER POSITION NICHT GETRAGEN WERDEN!" */
										INFLECT_INDEFINITE_ARTICLE | INFLECT_SINGULAR | INFLECT_1ST_CASE,
										item_id_2,
										INFLECT_NAME_TYPE_ITEM
									), get_ttx(556)); /* "KANN" */

								GUI_output(g_dtp2);
							} else {
								if (two_hand_collision(hero, item_id_2, inv_slot_1)) {

									sprintf(g_dtp2, get_ttx(829), hero->alias); /* "%s KANN KEINE ZWEIHÄNDIGE WAFFE IN DER EINEN UND ETWAS ANDERES IN D..." */
									GUI_output(g_dtp2);

								} else {
									if (item_id_1 != 0) {
										unequip(hero, item_id_1, inv_slot_1);
									}

									add_equip_boni(hero, hero, item_id_2, inv_slot_2, inv_slot_1);
									do_exchange = 1;
								}
							}
						}
					}
				} else {
					unequip(hero, item_id_1, inv_slot_1);
					do_exchange = 1;
				}
			}
		}

		if (do_exchange != 0) {

			/* item have the same ids and are stackable */
			if ((item_id_2 == item_id_1) && g_itemsdat[item_id_1].flags.stackable) {
				/* merge them */

				/* add quantity of item at inv_slot_2 to item at inv_slot_1 */
				hero->inventory[inv_slot_1].quantity += hero->inventory[inv_slot_2].quantity;

				/* delete item at inv_slot_2 */
				memset(&hero->inventory[inv_slot_2], 0, sizeof(struct inventory));
#ifdef M302de_ORIGINAL_BUGFIX
				/* Decrement the item counter */
				hero->num_filled_inv_slots--;
#endif
			} else {

				/* exchange the items */
				tmp = hero->inventory[inv_slot_1];
				hero->inventory[inv_slot_1] = hero->inventory[inv_slot_2];
				hero->inventory[inv_slot_2] = tmp;
			}
		}
	}
}

/**
 * \brief   shows the description of an item: name, broken, magic, worn out, poisoned, staff spell level
 *
 * \param   hero        the hero
 * \param   pos         inventory slot
 */
/* Borlandified and identical */
void print_item_description(struct struct_hero *hero, const signed int pos)
{
	/* create pointer to the item in the inventory */
	struct inventory *inventory_p = &hero->inventory[pos];

	if (inventory_p->item_id != ITEM_ID_NONE) {

		/* normal item */

		if (((inventory_p->quantity > 1) && g_itemsdat[inventory_p->item_id].flags.stackable) ||
			is_in_int_array(inventory_p->item_id, g_items_pluralwords)) {

			/* more than one item or special */
			sprintf(g_dtp2, get_tx2(72), get_ttx(305), (uint8_t*)GUI_name_inflect_with_article( /* "DIESE %s %s" / "SIND" */
				INFLECT_OMIT_ARTICLE | INFLECT_PLURAL | INFLECT_1ST_CASE,
				inventory_p->item_id,
				INFLECT_NAME_TYPE_ITEM
			));
		} else {
			/* one item */
			sprintf(g_dtp2, get_tx2(11), get_ttx(304), (uint8_t*)GUI_name_inflect_with_article( /* "DIES %s %s" / "IST" */
				INFLECT_INDEFINITE_ARTICLE | INFLECT_SINGULAR | INFLECT_1ST_CASE,
				inventory_p->item_id,
				INFLECT_NAME_TYPE_ITEM
			));
		}
	} else {
		/* no item */
		strcpy(g_dtp2, get_ttx(209)); /* "ZUNÄCHST MUSS EIN ZULÄSSIGER GEGENSTAND AUSGEWÄHLT WERDEN." */
	}


	/* broken */
	if (inventory_p->flags.broken) {
		strcat(g_dtp2, get_ttx(478)); /* "@(ZERBROCHEN)" */
	}

	/* magic */
	if (inventory_p->flags.magic && inventory_p->flags.magic_revealed) {
		strcat(g_dtp2, get_ttx(479)); /* "@(MAGISCH)" */
	}

	/* RS degraded */
	if (inventory_p->rs_lost != 0) {
		strcat(g_dtp2, get_ttx(480)); /* "@(ABGENUTZT)" */
	}

	/* poisoned */
	if (inventory_p->item_id == ITEM_ID_KUKRISDOLCH || inventory_p->item_id == ITEM_ID_KUKRIS_MENGBILAR ||
		inventory_p->flags.poison_expurgicum || inventory_p->flags.poison_vomicum ||
		hero->inventory[pos].poison_id != POISON_ID_NONE) {

		strcat(g_dtp2, get_ttx(548)); /* "@(VERGIFTET)" */
	}

	/* magic wand */
	if (inventory_p->item_id == ITEM_ID_ZAUBERSTAB) {
		sprintf(g_text_output_buf, get_tx2(53), hero->staff_level); /* "@AUF IHM LIEGT DER %d.STABZAUBER!" */
		strcat(g_dtp2, g_text_output_buf);
	}

	GUI_output(g_dtp2);
}

/**
 * \brief   passes an item from hero1 to hero2 (trade mode of the status menu)
 *
 *          An item at pos2 is exchanged, equal stackable items are merged, part of a
 *          stack can be passed. Checks: undroppable, usable and wearable at a body slot,
 *          two-handed weapon, carrying capacity of hero2.
 *
 * \param   hero1       the giving hero
 * \param   old_pos1    inventory slot of hero1
 * \param   hero2       the receiving hero
 * \param   pos2        inventory slot of hero2
 */
/* Borlandified and identical */
void pass_item(struct struct_hero *hero1, const signed int old_pos1, struct struct_hero *hero2, const signed int pos2)
{
	signed int stackable_quant;
	register signed int pos1 = old_pos1;

	const signed int item_id1 = hero1->inventory[pos1].item_id;
	signed int item_id2;
	struct item_stats *item1_desc;
	struct item_stats *item2_desc;
	signed int do_swap;
	signed int item_weight1;
	signed int item_weight2;
	struct inventory tmp;

	/* check if item_id1 is an item */
	if (item_id1 == 0) {

		GUI_output(get_ttx(209)); /* "ZUNÄCHST MUSS EIN ZULÄSSIGER GEGENSTAND AUSGEWÄHLT WERDEN." */
		return;
	}

	item_id2 = hero2->inventory[pos2].item_id;

	item1_desc = &g_itemsdat[item_id1];
	item2_desc = &g_itemsdat[item_id2];

	if (item1_desc->flags.undroppable) {

		sprintf(g_dtp2, get_ttx(454), GUI_name_inflect_with_article( /* "HEY, DU WIRST DOCH NICHT SO VERRÜCKT SEIN UND %s AUS DER HAND GEBEN..." */
			INFLECT_DEFINITE_ARTICLE | INFLECT_SINGULAR | INFLECT_4TH_CASE,
			item_id1,
			INFLECT_NAME_TYPE_ITEM
		));

		GUI_output(g_dtp2);
		return;
	}

	if (item2_desc->flags.undroppable) {

		sprintf(g_dtp2, get_ttx(454), GUI_name_inflect_with_article( /* "HEY, DU WIRST DOCH NICHT SO VERRÜCKT SEIN UND %s AUS DER HAND GEBEN..." */
			INFLECT_DEFINITE_ARTICLE | INFLECT_SINGULAR | INFLECT_4TH_CASE,
			item_id2,
			INFLECT_NAME_TYPE_ITEM
		));
		GUI_output(g_dtp2);
		return;

	}

	/* identical until here */
	if (pos2 < 7) {

		if (!can_hero_use_item(hero2, item_id1)) {

			sprintf(g_dtp2,	get_ttx(221), hero2->alias, get_ttx((hero2->sex ? 593 : 9) + hero2->typus), /* "%s ALS %s KANN %s NICHT VERWENDEN!" / TEXT.LTX 10..21 / 594..605: Heldentyp (m/w) */
				GUI_name_inflect_with_article(
					INFLECT_INDEFINITE_ARTICLE | INFLECT_SINGULAR | INFLECT_4TH_CASE,
					item_id1,
					INFLECT_NAME_TYPE_ITEM
				)
			);

			/* REMARK: an unused label. BCC merges identical code tails ("cross jumping"),
			 * a label in between stops that. Without it, this message would jump into the
			 * identical tail of the message for hero1 below. */
pass_item_label1:
			GUI_output(g_dtp2);
			return;

		} else if (!can_hero_equip_item_at_slot(item_id1, pos2)) {

			if (is_in_int_array(item_id1, g_items_pluralwords)) {

				sprintf(g_dtp2, get_ttx(222), GUI_name_inflect_with_article( /* "%s %s AN DIESER POSITION NICHT GETRAGEN WERDEN!" */
					INFLECT_OMIT_ARTICLE | INFLECT_SINGULAR | INFLECT_1ST_CASE,
					item_id1,
					INFLECT_NAME_TYPE_ITEM),
				get_ttx(557)); /* "KÖNNEN" */
			} else {
				sprintf(g_dtp2, get_ttx(222), GUI_name_inflect_with_article( /* "%s %s AN DIESER POSITION NICHT GETRAGEN WERDEN!" */
					INFLECT_INDEFINITE_ARTICLE | INFLECT_SINGULAR | INFLECT_1ST_CASE,
					item_id1,
					INFLECT_NAME_TYPE_ITEM),
				get_ttx(556)); /* "KANN" */
			}

			GUI_output(g_dtp2);
			return;

		} else if (two_hand_collision(hero2, item_id1, pos2)) {

			sprintf(g_dtp2, get_tx2(67), hero2->alias); /* "%s KANN KEINE ZWEIHÄNDIGE WAFFE IN DER EINEN UND ETWAS ANDERES IN D..." */

			/* REMARK: unused label, see above */
pass_item_label2:
			GUI_output(g_dtp2);
			return;
		}
	}

/* 0x8ff */

	if ((item_id2 != 0) && (pos1 < 7)) {

		if (!can_hero_use_item(hero1, item_id2)) {

			sprintf(g_dtp2,	get_ttx(221), hero1->alias, get_ttx((hero1->sex ? 593 : 9) + hero1->typus), /* "%s ALS %s KANN %s NICHT VERWENDEN!" / TEXT.LTX 10..21 / 594..605: Heldentyp (m/w) */
				GUI_name_inflect_with_article(
					INFLECT_INDEFINITE_ARTICLE | INFLECT_SINGULAR | INFLECT_4TH_CASE,
					item_id2,
					INFLECT_NAME_TYPE_ITEM
				)
			);

			GUI_output(g_dtp2);
			return;

		} else if (!can_hero_equip_item_at_slot(item_id2, pos1)) {

			if (is_in_int_array(item_id2, g_items_pluralwords)) {

				sprintf(g_dtp2, get_ttx(222), GUI_name_inflect_with_article( /* "%s %s AN DIESER POSITION NICHT GETRAGEN WERDEN!" */
					INFLECT_OMIT_ARTICLE | INFLECT_SINGULAR | INFLECT_1ST_CASE,
					item_id2,
					INFLECT_NAME_TYPE_ITEM
				), get_ttx(557)); /* "KÖNNEN" */
			} else {
				sprintf(g_dtp2, get_ttx(222), GUI_name_inflect_with_article( /* "%s %s AN DIESER POSITION NICHT GETRAGEN WERDEN!" */
					INFLECT_INDEFINITE_ARTICLE | INFLECT_SINGULAR | INFLECT_1ST_CASE,
					item_id2,
					INFLECT_NAME_TYPE_ITEM
				), get_ttx(556)); /* "KANN" */
			}

			GUI_output(g_dtp2);
			return;
		}
	}

/* 0xa14 */

	/* identical from here */
	if (item_id2 != ITEM_ID_NONE) {

		do_swap = 1;
		if ((item_id2 == item_id1) && item2_desc->flags.stackable) {

			do_swap = 0;
			stackable_quant = 1;

			if (hero1->inventory[pos1].quantity > 1) {

				sprintf(g_dtp2,	get_ttx(210), hero1->inventory[pos1].quantity, (char*)GUI_name_inflect_with_article( /* "WIE VIELE DER %d %s SOLLEN AN %s ÜBERGEBEN WERDEN?" */
				/* get_ttx(210): "WIE VIELE DER %d^%s SOLLEN an %s ÃœBERGEBEN WERDEN" */
#ifndef M302de_ORIGINAL_BUGFIX
					INFLECT_INDEFINITE_ARTICLE | INFLECT_PLURAL | INFLECT_4TH_CASE,
#else
					// this is 2nd case, not 4th
					INFLECT_INDEFINITE_ARTICLE | INFLECT_PLURAL | INFLECT_2ND_CASE,
#endif
					item_id1,
					INFLECT_NAME_TYPE_ITEM
				), hero2->alias);

				stackable_quant = GUI_input(g_dtp2, 2);
			}

			if (hero1->inventory[pos1].quantity < stackable_quant) {
				stackable_quant = hero1->inventory[pos1].quantity;
			}

			if ((stackable_quant > 0) && (hero2->inventory[pos2].quantity < STACK_SIZE_MAX)) {

				if (hero2->inventory[pos2].quantity + stackable_quant > STACK_SIZE_MAX) {
					stackable_quant = STACK_SIZE_MAX - hero2->inventory[pos2].quantity;
				}

				while ((hero2->attrib[ATTRIB_KK].current * 100 <= hero2->load + item1_desc->weight * stackable_quant) && (stackable_quant > 0)) {
					stackable_quant--;
				}

				if (stackable_quant > 0) {
					hero2->load += item1_desc->weight * stackable_quant;
					hero2->inventory[pos2].quantity += stackable_quant;
					drop_item(hero1, pos1, stackable_quant);
				} else {
					sprintf(g_dtp2,	get_ttx(779), hero2->alias); /* "%s HAT DAS ZUMUTBARE LIMIT AN GEWICHT BEREITS ERREICHT!" */
					GUI_output(g_dtp2);
				}
			}
		}

		if (do_swap != 0) {

			item_weight1 = item1_desc->flags.stackable ?
				hero1->inventory[pos1].quantity * item1_desc->weight :
				item1_desc->weight;

			item_weight2 = item2_desc->flags.stackable ?
				hero2->inventory[pos2].quantity * item2_desc->weight :
				item2_desc->weight;

			if (hero2->attrib[ATTRIB_KK].current * 100 <= hero2->load + item_weight1 - item_weight2) {

				sprintf(g_dtp2,	get_ttx(779), hero2->alias); /* "%s HAT DAS ZUMUTBARE LIMIT AN GEWICHT BEREITS ERREICHT!" */
				GUI_output(g_dtp2);

			} else {
				if (pos1 < 7) {
					unequip(hero1, item_id1, pos1);
					add_equip_boni(hero2, hero1, item_id2, pos2, pos1);
				}

				if (pos2 < 7) {
					unequip(hero2, item_id2, pos2);
					add_equip_boni(hero1, hero2, item_id1, pos1, pos2);
				}

				/* exchange two items */
				tmp = hero2->inventory[pos2]; /* struct_copy */

				hero2->load -= item_weight2;

				hero2->inventory[pos2] = hero1->inventory[pos1]; /* struct_copy */

				hero2->load += item_weight1;
				hero1->load -= item_weight1;

				hero1->inventory[pos1] = tmp; /* struct_copy */

				hero1->load += item_weight2;

				/* item counter */
				/* Original-Bug: this is an exchange, both heroes keep their number of filled slots.
				 * hero1's counter gets too low, hero2's too high (cf. the weak check in give_new_item_to_hero). */
				hero1->num_filled_inv_slots--;
				hero2->num_filled_inv_slots++;

				/* special items */
				if (item_id2 == ITEM_ID_SICHEL__MAGIC) {
					hero1->talents[TA_PFLANZENKUNDE] = hero1->talents[TA_PFLANZENKUNDE] + 3;
					hero2->talents[TA_PFLANZENKUNDE] = hero2->talents[TA_PFLANZENKUNDE] - 3;
				}
				if (item_id2 == ITEM_ID_AMULETT__MR_BONUS) {
					hero1->mr = hero1->mr + 5;
					hero2->mr = hero2->mr - 5;
				}
				if (item_id1 == ITEM_ID_SICHEL__MAGIC) {
					hero1->talents[TA_PFLANZENKUNDE] = hero1->talents[TA_PFLANZENKUNDE] - 3;
					hero2->talents[TA_PFLANZENKUNDE] = hero2->talents[TA_PFLANZENKUNDE] + 3;
				}
				if (item_id1 == ITEM_ID_AMULETT__MR_BONUS) {
					hero1->mr = hero1->mr - 5;
					hero2->mr = hero2->mr + 5;
				}
			}
		}

	} else if (item1_desc->flags.stackable) {

		stackable_quant = 1;

		if (hero1->inventory[pos1].quantity > 1) {

			sprintf(g_dtp2,	get_ttx(210), hero1->inventory[pos1].quantity, (char*)GUI_name_inflect_with_article( /* "WIE VIELE DER %d %s SOLLEN AN %s ÜBERGEBEN WERDEN?" */
			/* get_ttx(210): "WIE VIELE DER %d^%s SOLLEN an %s ÃœBERGEBEN WERDEN" */
#ifndef M302de_ORIGINAL_BUGFIX
				INFLECT_INDEFINITE_ARTICLE | INFLECT_PLURAL | INFLECT_4TH_CASE,
#else
				// this is 2nd case, not 4th
				INFLECT_INDEFINITE_ARTICLE | INFLECT_PLURAL | INFLECT_2ND_CASE,
#endif
				item_id1,
				INFLECT_NAME_TYPE_ITEM
			), hero2->alias);

			stackable_quant = GUI_input(g_dtp2, 2);
		}

		if (hero1->inventory[pos1].quantity < stackable_quant) {
			stackable_quant = hero1->inventory[pos1].quantity;
		}

		while ((hero2->attrib[ATTRIB_KK].current * 100 <= hero2->load + item1_desc->weight * stackable_quant) && (stackable_quant > 0)) {
			stackable_quant--;
		}

		if (stackable_quant > 0) {

			hero2->inventory[pos2] = hero1->inventory[pos1]; /* struct_copy */

			hero2->load += item1_desc->weight * stackable_quant;
			hero2->inventory[pos2].quantity = stackable_quant;
			drop_item(hero1, pos1, stackable_quant);
			/* Original-Bug: hero2->num_filled_inv_slots is not incremented for the new stack */

		} else {
			sprintf(g_dtp2,	get_ttx(779), hero2->alias); /* "%s HAT DAS ZUMUTBARE LIMIT AN GEWICHT BEREITS ERREICHT!" */
			GUI_output(g_dtp2);
		}

	} else if (hero2->attrib[ATTRIB_KK].current * 100 <= hero2->load + item1_desc->weight) {

		sprintf(g_dtp2,	get_ttx(779), hero2->alias); /* "%s HAT DAS ZUMUTBARE LIMIT AN GEWICHT BEREITS ERREICHT!" */
		GUI_output(g_dtp2);

	} else {

		/* do the change */

		if (pos1 < 7) {
			unequip(hero1, item_id1, pos1);
		}

		if (pos2 < 7) {
			add_equip_boni(hero1, hero2, item_id1, pos1, pos2);
		}

		hero2->inventory[pos2] = hero1->inventory[pos1]; /* struct_copy */

		/* adjust weight */
		hero2->load += item1_desc->weight;
		hero1->load -= item1_desc->weight;

		/* adjust item counter */
		hero1->num_filled_inv_slots--;
		hero2->num_filled_inv_slots++;

		/* clear slot */
		memset(&hero1->inventory[pos1], 0, sizeof(struct inventory));

		/* special items */
		if (item_id1 == ITEM_ID_SICHEL__MAGIC) {
			hero1->talents[TA_PFLANZENKUNDE] = hero1->talents[TA_PFLANZENKUNDE] - 3;
			hero2->talents[TA_PFLANZENKUNDE] = hero2->talents[TA_PFLANZENKUNDE] + 3;
		}
		if (item_id1 == ITEM_ID_AMULETT__MR_BONUS) {
			hero1->mr = hero1->mr - 5;
			hero2->mr = hero2->mr + 5;
		}
	}
}

struct items_all {
	signed int a[4];
};

/**
 * \brief   gives a new hero his starting equipment
 *
 *          All: waterskin, 2 food packages, trousers (worn); females except warriors and
 *          mages: a shirt (worn); then the items of his type, the first one in the right
 *          hand; warrior: leather armor, mage: robe, hunter and elves: 20 arrows in the left hand.
 */
/* Borlandified and identical */
void startup_equipment(struct struct_hero *hero)
{
	signed int i;
	struct items_all all;

	*(struct items_all*)&all = *(struct items_all*)g_hero_startup_items_all; /* struct copy */

	for (i = 0; i < 4; i++) {
		give_new_item_to_hero(hero, all.a[i], 1, 1);
	}

	move_item(HERO_INVENTORY_SLOT_LEGS, HERO_INVENTORY_SLOT_KNAPSACK_3, hero);

	if ((hero->sex != 0) && (hero->typus != HERO_TYPE_KRIEGER) && (hero->typus != HERO_TYPE_MAGIER))
       	{
		/* female non-warriors and non-mages get a free shirt */
		give_new_item_to_hero(hero, ITEM_ID_HEMD, 1, 1);
		move_item(HERO_INVENTORY_SLOT_BODY, HERO_INVENTORY_SLOT_KNAPSACK_3, hero);
	}

	i = 0;
	while ((g_hero_startup_items[hero->typus - 1][i] != -1) && (i < 4)) { /* for the mage (4 items) the element [4] behind the row is read before i < 4 is checked (harmless) */

		give_new_item_to_hero(hero, g_hero_startup_items[hero->typus - 1][i++], 1, 1);

		if (i == 1) {
			move_item(HERO_INVENTORY_SLOT_RIGHT_HAND, HERO_INVENTORY_SLOT_KNAPSACK_3, hero);
		}
	}

	if (hero->typus == HERO_TYPE_KRIEGER) {
		move_item(HERO_INVENTORY_SLOT_BODY, inv_slot_of_item(hero, ITEM_ID_LEDERHARNISCH), hero);
	}

	if (hero->typus == HERO_TYPE_MAGIER) {
		move_item(HERO_INVENTORY_SLOT_BODY, inv_slot_of_item(hero, ITEM_ID_ROBE__GREEN_1), hero);
	}

	if ((hero->typus == HERO_TYPE_JAEGER) ||
		(hero->typus == HERO_TYPE_AUELF) ||
		(hero->typus == HERO_TYPE_WALDELF))
	{
		give_new_item_to_hero(hero, ITEM_ID_PFEIL, 1, 20);
		move_item(HERO_INVENTORY_SLOT_LEFT_HAND, inv_slot_of_item(hero, ITEM_ID_PFEIL), hero);
	}
}

/**
 * \brief   get the maximum time of a burning lightsource
 * \return {-1 = no lightsource, otherwise a value in {0, ..., 10}
 * \todo: rethink this function with (groups, dead heroes, empty slots, ...) in mind
 */
/* Borlandified and identical */
signed int get_max_light_time(void)
{
	struct struct_hero *hero;
	signed int i;
	signed int j;
	signed int retval = -1;

	hero = get_hero(0);
	for (i = 0; i <= 6; i++, hero++) {

#ifdef M302de_ORIGINAL_BUGFIX
		if (!hero->typus)
			continue;
#endif

		for (j = 0; j < NR_HERO_INVENTORY_SLOTS; j++) {

			/* search for a burning torch */
			if (hero->inventory[j].item_id == ITEM_ID_FACKEL__LIT) {

				if (hero->inventory[j].lighting_timer > retval) {
					retval = hero->inventory[j].lighting_timer;
				}

			} else if (hero->inventory[j].item_id == ITEM_ID_LATERNE__LIT) {

				/* search for a burning lantern */

				if (hero->inventory[j].lighting_timer / 10 > retval) {
					retval = hero->inventory[j].lighting_timer / 10;
				}
			}
		}
	}

	return retval;
}

/**
 * \brief   shows an animation of a man equipping a belt
 *
 *          This is only executed when equipping a skullbelt.
 */
/* Borlandified and identical */
void equip_belt_ani(void)
{
	signed int i;
	signed int handle;

	signed int width;
	signed int height;
	int32_t nvf_length;
	uint8_t *p_pal;
	struct nvf_extract_desc nvf;

	/* open GUERTEL.NVF */
	handle = load_archive_file(ARCHIVE_FILE_GUERTEL_NVF);

	/* read NVF part 1 */
	nvf_length = read_archive_file(handle, (uint8_t*)g_buffer9_ptr, 64000);
	/* read NVF part 2 */
	nvf_length += read_archive_file(handle, (uint8_t*)(g_buffer9_ptr + 64000L), 64000);

	close(handle);

	/* calculate palette pointer */
	p_pal = (uint8_t*)((g_buffer9_ptr + nvf_length) -0x60L);

	wait_for_vsync();

	set_palette(p_pal, 0x80, 0x20);

	do_border(g_vga_memstart, 209, 79, 215, 89, 9);

	do_fill_rect(g_vga_memstart, 209, 79, 215, 89, 0);

	wait_for_vsync();
	wait_for_vsync();
	wait_for_vsync();

	do_fill_rect(g_vga_memstart, 189, 69, 235, 99, 0);

	do_border(g_vga_memstart, 189, 69, 235, 99, 9);

	wait_for_vsync();
	wait_for_vsync();
	wait_for_vsync();

	do_fill_rect(g_vga_memstart, 169, 59, 255, 109, 0);

	do_border(g_vga_memstart, 169, 59, 255, 109, 9);

	wait_for_vsync();
	wait_for_vsync();
	wait_for_vsync();

	do_fill_rect(g_vga_memstart, 164, 54, 260, 114, 0);

	do_border(g_vga_memstart, 164, 54, 260, 114, 9);

	wait_for_vsync();
	wait_for_vsync();
	wait_for_vsync();

	do_fill_rect(g_vga_memstart, 159, 49, 263, 117, 0);

	do_border(g_vga_memstart, 159, 49, 263, 117, 9);

	for (i = 0; i < 12; i++) {

		nvf.dst = g_renderbuf_ptr;
		nvf.src = (uint8_t*)g_buffer9_ptr;
		nvf.image_num = i;
		nvf.compression_type = 3;
		nvf.width = &width;
		nvf.height = &height;

		process_nvf_extraction(&nvf);

		g_pic_copy.x1 = 160;
		g_pic_copy.y1 = 50;
		g_pic_copy.x2 = width + 159;
		g_pic_copy.y2 = height + 49;
		g_pic_copy.src = g_renderbuf_ptr;

		wait_for_vsync();
		wait_for_vsync();
		wait_for_vsync();
		call_mouse_bg();
		wait_for_vsync();

		do_pic_copy(0);

		call_mouse();
	}

	g_current_ani = g_dng_loaded_dungeon_id = g_town_loaded_town_id = -1;
}

/**
 * \brief   search for a full waterskin
 *
 * \param   hero        pointer to the hero
 * \return              position of a non-empty waterskin
 */
/* Borlandified and identical */
signed int get_full_waterskin_inv_slot(const struct struct_hero *hero)
{
	signed int inv_pos = -1;
	signed int i;

	/* Original-BUG: should start from inventory pos 0 */
	for (i = HERO_INVENTORY_SLOT_KNAPSACK_1; i < NR_HERO_INVENTORY_SLOTS; i++) {

		/* look for a non-empty waterskin */
		if ((hero->inventory[i].item_id == ITEM_ID_WASSERSCHLAUCH) && !hero->inventory[i].flags.empty)
		{
			inv_pos = i;
			break;
		}
	}

	return inv_pos;
}
