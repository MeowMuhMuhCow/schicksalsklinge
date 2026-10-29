/*
 * seg104 - Alchimie, Krankheiten heilen, Heldenhilfen
 *
 * Inhalt:    Alchimie: Rezepttabelle, Zutatenprüfung, Brauen mit Wartezeit oder
 *            Abtrennen des Brauers von der Gruppe (lange Rezepte nur in der
 *            Herberge) und Abschluss des Brauvorgangs. Dazu Krankheiten mit dem
 *            Talent heilen (benötigte Kräuter), den schwersten Helden finden und
 *            den Helden mit den besten Talentchancen bestimmen.
 * Aufrufer:  plan_alchemy aus use_talent (seg103); do_alchemy aus GRP_hero_sleep
 *            (seg049) und der Herberge (seg054); talent_cure_disease aus seg103
 *            und dem Tollwut-Ereignis (seg120); get_heaviest_hero/get_hero_weight
 *            aus seg078; get_talented_hero_pos für Vorauswahlen in Menüs.
 * Daten:     13 Rezepte (Rezept, bis zu 10 Zutaten, Ergebnis, AE-Kosten,
 *            Probenaufschlag, Dauer in Stunden); g_alchemy_missing_item.
 * Besonders: SPELLTXT 49 "DA SCHEINT ABER WAS ZU FEHLEN!" hat kein %s, die fehlende
 *            Zutat wird nie genannt. Neu (talent_cure_disease): Wundfieber und
 *            Blaue Keuche lassen sich nie heilen, und beim Aufruf aus seg120 bleibt
 *            SPELLTXT.LTX geladen, wenn der Patient nicht behandelbar ist.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg104.c
 */
#include <stdio.h>
#include <string.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg026.h"
#include "seg047.h"
#include "seg049.h"
#include "seg053.h"
#include "seg096.h"
#include "seg097.h"
#include "seg103.h"
#include "seg104.h"
#include "seg105.h"

static struct struct_recipe g_alchemy_recipes[13] = {
	{
		ITEM_ID_REZEPT_FUER_HEILTRANK,
		ITEM_ID_GLASFLASCHE, ITEM_ID_EINBEERE, ITEM_ID_EINBEERE, ITEM_ID_TARNELE, ITEM_ID_SCHNAPSFLASCHE,
		-1, -1, -1, -1, -1,
		ITEM_ID_HEILTRANK,	5, 2, 8
	},
	{
		ITEM_ID_REZEPT_FUER_STARKEN_HEILTRANK,
		ITEM_ID_BRONZEFLASCHE, ITEM_ID_WIRSELKRAUT, ITEM_ID_WIRSELKRAUT, ITEM_ID_TARNELE, ITEM_ID_SCHNAPSFLASCHE,
		ITEM_ID_ALRAUNE, -1, -1, -1, -1,
		ITEM_ID_STARKER_HEILTRANK, 12, 5, 24
	},
	{
		ITEM_ID_REZEPT_FUER_WUNDERKUR,
		ITEM_ID_GLASFLASCHE, ITEM_ID_SCHNAPSFLASCHE, ITEM_ID_WIRSELKRAUT, ITEM_ID_WIRSELKRAUT, ITEM_ID_DONFSTENGEL,
		ITEM_ID_BELMART_BLATT, ITEM_ID_ALRAUNE, ITEM_ID_JORUGAWURZEL, -1, -1,
		ITEM_ID_WUNDERKUR,	25, 10, 3 * 24
	},
	{
		ITEM_ID_REZEPT_FUER_GEGENGIFT,
		ITEM_ID_GLASFLASCHE, ITEM_ID_SCHNAPSFLASCHE, ITEM_ID_MENCHALKAKTUS, ITEM_ID_MENCHALKAKTUS, ITEM_ID_ALRAUNE,
		ITEM_ID_SHURINKNOLLE, -1, -1, -1, -1,
		ITEM_ID_GEGENGIFT, 10, 5, 24
	},
	{
		ITEM_ID_REZEPT_FUER_SCHLAFTRUNK,
		ITEM_ID_GLASFLASCHE, ITEM_ID_WEINFLASCHE, ITEM_ID_GULMOND_BLATT, ITEM_ID_TARNELE, -1,
		-1, -1, -1, -1, -1,
		ITEM_ID_SCHLAFTRUNK, 2, 2, 8
	},
	{
		ITEM_ID_REZEPT_FUER_ZAUBERTRANK,
		ITEM_ID_GLASFLASCHE, ITEM_ID_SCHNAPSFLASCHE, ITEM_ID_ALRAUNE, ITEM_ID_ALRAUNE, ITEM_ID_KAIRANHALM,
		ITEM_ID_KAIRANHALM, ITEM_ID_THONNYSBLUETE, -1, -1, -1,
		ITEM_ID_ZAUBERTRANK,	7, 8, 2 * 24
	},
	{
		ITEM_ID_REZEPT_FUER_STARKER_ZAUBERTRANK,
		ITEM_ID_GLASFLASCHE, ITEM_ID_SCHNAPSFLASCHE, ITEM_ID_ALRAUNE, ITEM_ID_ALRAUNE, ITEM_ID_ALRAUNE,
		ITEM_ID_KAIRANHALM, ITEM_ID_KAIRANHALM, ITEM_ID_KAIRANHALM, ITEM_ID_THONNYSBLUETE, -1,
		ITEM_ID_ZAUBERTRANK_STARK, 15, 12, 5 * 24
	},
	{
		ITEM_ID_REZEPT_FUER_VOMICUM,
		ITEM_ID_BRONZEFLASCHE, ITEM_ID_OEL, ITEM_ID_SHURINKNOLLE, ITEM_ID_ALRAUNE, -1,
		-1, -1, -1, -1, -1,
		ITEM_ID_VOMICUM, 0, 5, 8
	},
	{
		ITEM_ID_REZEPT_FUER_EXPURGICUM,
		ITEM_ID_BRONZEFLASCHE, ITEM_ID_LAKRITZE, ITEM_ID_SHURINKNOLLE, ITEM_ID_SHURINKNOLLE, ITEM_ID_ALRAUNE,
		ITEM_ID_ALRAUNE, -1, -1, -1, -1,
		ITEM_ID_EXPURGICUM, 0, 8, 2 * 24
	},
	{
		ITEM_ID_REZEPT_FUER_MIASTHMATICUM,
		ITEM_ID_GLASFLASCHE, ITEM_ID_SCHNAPSFLASCHE, ITEM_ID_KAIRANHALM, ITEM_ID_ILMENBLATT, ITEM_ID_SHURINKNOLLE,
		ITEM_ID_ALRAUNE, -1, -1, -1, -1,
		ITEM_ID_MIASTHMATICUM, 0, 6, 2 * 24
	},
	{
		ITEM_ID_REZEPT_FUER_MUTELIXIER,
		ITEM_ID_GLASFLASCHE, ITEM_ID_SCHNAPSFLASCHE, ITEM_ID_FINAGEBAEMCHEN, ITEM_ID_ALRAUNE, ITEM_ID_DONFSTENGEL,
		ITEM_ID_ILMENBLATT, -1, -1, -1, -1,
		ITEM_ID_MU_ELIXIER, 12, 7, 2 * 24
	},
	{
		ITEM_ID_REZEPT_FUER_KRAFTELIXIER,
		ITEM_ID_GLASFLASCHE, ITEM_ID_SCHNAPSFLASCHE, ITEM_ID_FINAGEBAEMCHEN, ITEM_ID_ALRAUNE, ITEM_ID_GULMOND_BLATT,
		ITEM_ID_GULMOND_BLATT, -1, -1, -1, -1,
		ITEM_ID_KK_ELIXIER, 15, 8, 2 * 24
	},
	{
		ITEM_ID_REZEPT_FUER_HYLAILER_FEUER,
		ITEM_ID_GLASFLASCHE, ITEM_ID_SCHNAPSFLASCHE, ITEM_ID_SCHNAPSFLASCHE, ITEM_ID_LAKRITZE, ITEM_ID_OEL,
		ITEM_ID_OEL, -1, -1, -1, -1,
		ITEM_ID_HYLAILER_FEUER, 0, 5, 8
	}
}; // ds:0xacda


signed int g_alchemy_missing_item; // ds:0xe5c4

/**
 * \brief   checks if the hero has all ingredients of a recipe
 *
 *          The ingredients are dropped one after another (so that double
 *          ingredients are counted correctly) and given back afterwards.
 *          A missing ingredient is stored in g_alchemy_missing_item.
 *
 * \return              1 if all ingredients are present, else 0
 */
signed int hero_has_ingredients(struct struct_hero *hero, const signed int recipe_id)
{
	signed int i = 0;
	signed int retval = 1;
	struct struct_recipe *r_ptr = &g_alchemy_recipes[recipe_id];
	signed int inv_slot;

	/* loop over ingredients */
	while ((r_ptr->ingredients[i] != -1) && retval) {

		inv_slot = inv_slot_of_item(hero, r_ptr->ingredients[i]);

		if (inv_slot == -1) {

			/* needed item missing */
			retval = 0;

			/* TODO: g_alchemy_missing_item should not be a globvar */
			g_alchemy_missing_item = r_ptr->ingredients[i];
		} else {
			/* drop all needed items */
			drop_item(hero, inv_slot, 1);
		}

		i++;
	}

	i--;

	if (!retval) {
		i--;
	}

	while (i >= 0) {
		/* give all needed items back */
		give_new_item_to_hero(hero, r_ptr->ingredients[i], 1, 1);
		i--;
	}

	return retval;
}

/**
 * \brief   removes the ingredients of a recipe; wine/brandy bottles give a glass flask, oil a bronze flask
 */
void hero_use_ingredients(struct struct_hero *hero, const signed int recipe_id)
{
	signed int i = 0;
	struct struct_recipe *r_ptr = &g_alchemy_recipes[recipe_id];
	signed int inv_slot;

	/* loop over ingredients */
	while (r_ptr->ingredients[i] != -1) {

		inv_slot = inv_slot_of_item(hero, r_ptr->ingredients[i]);

		/* drop the needed item */
		drop_item(hero, inv_slot, 1);

		/* exchange wine- or brandybottles into glass flask */
		if ((r_ptr->ingredients[i] == ITEM_ID_WEINFLASCHE) || (r_ptr->ingredients[i] == ITEM_ID_SCHNAPSFLASCHE))
		{
			give_new_item_to_hero(hero, ITEM_ID_GLASFLASCHE, 1, 1);
		}

		/* exchange oil into bronze flask */
		if (r_ptr->ingredients[i] == ITEM_ID_OEL)
		{
			give_new_item_to_hero(hero, ITEM_ID_BRONZEFLASCHE, 1, 1);
		}

		i++;
	}
}

signed int do_alchemy(struct struct_hero* hero, const signed int recipe_id, const signed int flag_abort)
	/* flag_abort = 0: finalize running brewing process
	 * flag_abort = 1: abort running brewing process
	 * Ingredients and AE are used up in both cases. The talent test is also rolled on abort.
	 * Returns 1 on success, else 0. */
{
	struct struct_recipe *r_ptr = &g_alchemy_recipes[recipe_id];

	hero_use_ingredients(hero, recipe_id);

	sub_ae_splash(hero, r_ptr->ae_cost);

	hero->flags.brewing = 0;

	hero->recipe_timer = 0;
	/* reset the recipe of the hero */
	hero->recipe_id = 0;
	hero->alchemy_inn_typeindex = 0;

	if ((test_talent(hero, TA_ALCHIMIE, r_ptr->handicap) > 0) && (flag_abort == 0))
	{
		/* success */

		give_new_item_to_hero(hero, r_ptr->outcome, 1, 1);

		sprintf(g_dtp2, get_ttx(731), hero->alias, (char*)GUI_name_inflect_with_article( /* "%s GELINGT DAS BRAUEN %s." */
			INFLECT_INDEFINITE_ARTICLE | INFLECT_SINGULAR | INFLECT_2ND_CASE,
			r_ptr->outcome,
			INFLECT_NAME_TYPE_ITEM
		));
		GUI_output(g_dtp2);

		return 1;
	} else {
		/* failure */
		/* give first ingredient back, which is always the bottle (glass or bronze). */
		give_new_item_to_hero(hero, r_ptr->ingredients[0], 1, 1);

		sprintf(g_dtp2,	get_ttx(732), hero->alias, (char*)GUI_name_inflect_with_article( /* "%s GELINGT ES LEIDER NICHT %s HERZUSTELLEN." */
			INFLECT_INDEFINITE_ARTICLE | INFLECT_SINGULAR | INFLECT_4TH_CASE,
			r_ptr->outcome,
			INFLECT_NAME_TYPE_ITEM
		));
		GUI_output(g_dtp2);

		return 0;
	}

}

/**
 * \brief   talent Alchimie: choose a recipe and brew it
 *
 *          Recipes longer than 8 hours only in an inn (room booked). There the
 *          group may wait, or the brewing hero is separated into a new group
 *          (he is then asked again when the group enters the inn, see seg054).
 *
 * \return              1 = brewed or started, 0 = not possible / failed
 */
signed int plan_alchemy(struct struct_hero *hero)
{
	signed int retval;
	signed int inv_slot;
	signed int recipes;
	signed int answer;
	signed int decision;
	signed int new_group_id;
	signed int i;
	signed char recipe_index;
	struct struct_hero *hero_p;
	signed int tw_bak;
	signed char recipe_ids[13];


	retval = 1;
	recipes = 0;
	inv_slot = inv_slot_of_item(hero, ITEM_ID_ALCHIMIESET);

	if (inv_slot == -1) {

		/* no alchemy kit */
		GUI_output(get_tx(42)); /* "LEIDER GAR KEINE ALCHIMIE GRUNDAUSRÜSTUNG DABEI!" */
		retval = 0;
	} else {

		/* count all recipes and prepare the menu */
		for (i = 0; i <= 12; i++) {
			if (inv_slot_of_item(hero, g_alchemy_recipes[i].item_id) != -1) {

				strcpy(g_dtp2 + recipes * 50, GUI_name_base_form(g_itemsname[g_alchemy_recipes[i].outcome]));

				g_radio_name_list[recipes] = g_dtp2 + recipes * 50;
				recipe_ids[recipes] = (signed char)i;
				recipes++;
			}
		}

		if (recipes != 0) {

			/* ask which recipe should be used */
			tw_bak = g_textbox_width;
			g_textbox_width = 7;

			answer = GUI_radio(get_tx(43), (signed char)recipes, /* "WAS SOLL DENN GEBRAUT WERDEN?" */
						g_radio_name_list[0], g_radio_name_list[1],
						g_radio_name_list[2], g_radio_name_list[3],
						g_radio_name_list[4], g_radio_name_list[5],
						g_radio_name_list[6], g_radio_name_list[7],
						g_radio_name_list[8], g_radio_name_list[9],
						g_radio_name_list[10], g_radio_name_list[11],
						g_radio_name_list[12], g_radio_name_list[13]);

			g_textbox_width = tw_bak;

			if (answer != -1) {

				recipe_index = recipe_ids[answer -1];

				if (hero_has_ingredients(hero, recipe_index)) {

					if (g_alchemy_recipes[recipe_index].ae_cost > hero->ae) {

						/* AE not sufficient => brewing not possible */
						sprintf(g_dtp2,	get_ttx(607), hero->alias); /* "%s HAT NICHT GENÜGEND ASTRALENERGIE FÜR DIESEN ZAUBER." */
						GUI_output(g_dtp2);

						retval = 0;
					} else {
						/* AE sufficient */

						if ((gs_town_loc_type == LOCTYPE_INN) && (g_sleep_quality == -1)) {

							/* no room booked => brewing not possible */
							GUI_output(get_ttx(346)); /* "IHR SOLLTET ZUERST EIN ZIMMER MIETEN." */

							return 0;
						}

						if ((g_alchemy_recipes[recipe_index].duration > 8) && (gs_town_loc_type != LOCTYPE_INN)) {

							/* recipes with durations > 8 hours have to be done in a inn. */
							sprintf(g_dtp2,	get_tx(44), g_alchemy_recipes[recipe_index].duration); /* "DAFÜR IST NICHT GENÜGEND ZEIT (ES WÄREN %d STUNDEN NÖTIG)!" */
							GUI_output(g_dtp2);

							retval = 0;
						} else {
							if (
#ifndef M302de_ORIGINAL_BUGFIX
								/* Original-Bug 12: If the total number of heroes is >= 2, and the current group consists only of a single available hero (+possibly the non-brewing NPC),
								 * the option to separate that hero from the group is shown, resulting in an active group which is empty or contains only the NPC and dead/petrified/unconscious/renegade heroes
								 * See https://www.crystals-dsa-foren.de/showthread.php?tid=98&pid=166399#pid166399 and the following posts. */
								(gs_total_hero_counter > 1) &&
#else
								((hero == get_hero(6)) || (count_heroes_available_in_group_ignore_npc() > 1)) && /* still allow to single out the NPC if he is the brewing hero */
#endif
								(gs_town_loc_type != LOCTYPE_WILDCAMP) &&
								(g_alchemy_recipes[recipe_index].duration > 8)
							) {

								sprintf(g_dtp2,	get_tx(45), g_alchemy_recipes[recipe_index].duration); /* "DAS DAUERT %d STUNDEN.@WOLLT IHR" */

								sprintf(g_text_output_buf, get_tx(47), hero->alias); /* "%s VON DER GRUPPE TRENNEN?" */

								g_textbox_width = 7;

								do {
									decision = GUI_radio(g_dtp2, 3,
											get_tx(46), /* Die Zeit einfach verstreichen lassen */
											g_text_output_buf, /* <Held> von der Gruppe trennen */
											get_tx(48)); /* Lieber doch nicht brauen */
								} while (decision == -1);

								g_textbox_width = 3;

							} else {
								decision = 1;
							}

							if (decision == 1) {
								/* rest of group waits */
								timewarp(HOURS(g_alchemy_recipes[recipe_index].duration));

								if (gs_town_loc_type != LOCTYPE_WILDCAMP) {

									hero_p = get_hero(0);
									for (i = 0; i <= 6; i++, hero_p++) {
										if ((hero_p->typus != HERO_TYPE_NONE) && (hero_p->group_id == gs_active_group_id))
										{
											GRP_hero_sleep(hero_p, g_sleep_quality);
										}
									}
								} else {
									hero->staffspell_timer = DAYS(1); /* TODO: Why STAFFSPELL ?? */
									/* Remark: blocks the regeneration of the brewer in the next night (only read in GRP_hero_sleep, seg049). */
								}

								retval = do_alchemy(hero, recipe_index, 0);

								g_sleep_quality = -1;

							} else if (decision == 2) {
								/* split group */

								/* find an empty group */
								/* Original-Bug: only 6 groups, but 7 heroes might cause an 'out of boundary' here */
								for (new_group_id = 0; gs_group_member_counts[new_group_id] != 0; new_group_id++);

								hero->group_id = (signed char)new_group_id;
								gs_group_member_counts[new_group_id]++;
								gs_group_member_counts[gs_active_group_id]--;

								/* time in days, rounded down */
								hero->recipe_timer = g_alchemy_recipes[recipe_index].duration / 24;

								hero->recipe_id = recipe_index;
								hero->alchemy_inn_typeindex = gs_town_typeindex;

								hero->flags.brewing = 1;

								GRP_save_pos(new_group_id);
							} else {
								/* decision == 3, abort brewing process */
								retval = 0;
							}
						}
					}
				} else {
					/* not all ingredients */
					sprintf(g_dtp2, get_tx(49), GUI_name_base_form(g_itemsname[g_alchemy_missing_item])); /* "DA SCHEINT ABER WAS ZU FEHLEN!" */ /* the text has no %s, so the name of the missing item is never shown */
					GUI_output(g_dtp2);
				}
			}
		} else {
			/* no recipes */
			sprintf(g_dtp2, get_tx(50), hero->alias); /* "%s HAT GAR KEINE REZEPTE DABEI!" */
			GUI_output(g_dtp2);
		}
	}

	return retval;
}

/**
 * \brief   checks if the healer has the herbs needed against a disease
 *
 * \return              0 = no herb, item ID of the herb to use, 999 = Joruga + Gulmond,
 *                      99 = no herb needed (Wundfieber, Blaue Keuche; but see talent_cure_disease)
 */
signed int has_herb_for_disease(const struct struct_hero *hero, const signed int disease_id)
{
	signed int retval = 0;

	switch (disease_id) {

		case DISEASE_ID_WUNDFIEBER:
		case DISEASE_ID_BLAUE_KEUCHE:
			retval = 99;
			break;

		case DISEASE_ID_DUMPFSCHAEDEL:
			/* any single one of the following herbs is sufficient */
			if (inv_slot_of_item(hero, ITEM_ID_BELMART_BLATT)          != -1) retval = ITEM_ID_BELMART_BLATT;
			else if (inv_slot_of_item(hero, ITEM_ID_WIRSELKRAUT)   != -1) retval = ITEM_ID_WIRSELKRAUT;
			else if (inv_slot_of_item(hero, ITEM_ID_EINBEERE)    != -1) retval = ITEM_ID_EINBEERE;
			else if (inv_slot_of_item(hero, ITEM_ID_TARNELE)     != -1) retval = ITEM_ID_TARNELE;
			else if (inv_slot_of_item(hero, ITEM_ID_DONFSTENGEL) != -1) retval = ITEM_ID_DONFSTENGEL;
			else if (inv_slot_of_item(hero, ITEM_ID_FINAGEBAEMCHEN) != -1) retval = ITEM_ID_FINAGEBAEMCHEN;
			else if (inv_slot_of_item(hero, ITEM_ID_MENCHALKAKTUS)     != -1) retval = ITEM_ID_MENCHALKAKTUS;
			else if (inv_slot_of_item(hero, ITEM_ID_OLGINWURZEL)  != -1) retval = ITEM_ID_OLGINWURZEL;
			else if (inv_slot_of_item(hero, ITEM_ID_JORUGAWURZEL) != -1) retval = ITEM_ID_JORUGAWURZEL;
			break;

		case DISEASE_ID_PARALYSE:
			if (inv_slot_of_item(hero, ITEM_ID_DONFSTENGEL) != -1)
				retval = ITEM_ID_DONFSTENGEL;
			break;

		case DISEASE_ID_SCHLACHTENFIEBER:
			if ( (inv_slot_of_item(hero, ITEM_ID_JORUGAWURZEL) != -1) && (inv_slot_of_item(hero, ITEM_ID_GULMOND_BLATT) != -1)) retval = 999;
			break;

		case DISEASE_ID_FROSTSCHAEDEN:
			if (inv_slot_of_item(hero, ITEM_ID_WIRSELKRAUT) != -1) retval = ITEM_ID_WIRSELKRAUT;
			break;

		case DISEASE_ID_TOLLWUT:
			if (inv_slot_of_item(hero, ITEM_ID_JORUGAWURZEL) != -1) retval = ITEM_ID_JORUGAWURZEL;
			break;
	}

	return retval;
}

/**
 * \brief   talent Heilkunde Krankheiten
 *
 * \param   healer      the healing hero
 * \param   patient     the diseased hero
 * \param   handicap    handicap of the tests
 * \param   from_event  1 = called from the rabies event (seg120): SPELLTXT.LTX is loaded here and
 *                      the first test (finding out the disease) is skipped; 0 = from use_talent (seg103)
 * \return              1 on success, else 0 or the (positive) test result, see the remark below
 */
signed int talent_cure_disease(struct struct_hero *healer, struct struct_hero *patient, const signed int handicap, const signed int from_event)
{
	signed int disease_id;
	signed int retval;

	signed int damage;
	signed int tx_file_bak;
	signed int herb;

	retval = 0;

	if (from_event) {
		tx_file_bak = g_tx_file_index;
		load_tx(ARCHIVE_FILE_SPELLTXT_LTX);
	}

	if (is_hero_healable(patient)) {

		disease_id = hero_is_diseased(patient);

		if (!disease_id) {

			/* not diseased */
			sprintf(g_dtp2,	get_ttx(462), patient->alias); /* "<ICH KANN BEI %s KEINE KRANKHEIT DIAGNOSTIZIEREN!>" */
			GUI_output(g_dtp2);

		} else if (patient->heal_timer > 0) {

			/* recently tried to cure with talent */
			sprintf(g_dtp2,	get_ttx(697), patient->alias); /* "DER LETZTE HEILVERSUCH DURCH EIN TALENT IST BEI %s NOCH NICHT LANGE..." */
			GUI_output(g_dtp2);

		} else if (!(herb = has_herb_for_disease(healer, disease_id))) {

			/* not the needed herbs for healing this disease */
			sprintf(g_dtp2, get_tx(118), healer->alias); /* "%s HAT NICHT DIE ZUR BEHANDLUNG NÖTIGEN KRÄUTER!" */
			GUI_output(g_dtp2);

		} else {
			timewarp(MINUTES(5));

			/* set timer */
			patient->heal_timer = HOURS(4);

			if ((from_event != 0) || (test_talent(healer, TA_HEILEN_KRANKHEITEN, (signed char)handicap) > 0)) {

				if (((retval = test_talent(healer, TA_HEILEN_KRANKHEITEN, g_disease_strength[disease_id] + handicap)) > 0) &&
					(disease_id != DISEASE_ID_WUNDFIEBER) && (disease_id != DISEASE_ID_BLAUE_KEUCHE)) /* Wundfieber and Blaue Keuche: always "DAS HEILEN MISSLINGT" with 3 damage, even after a successful test; then retval > 0 is returned */
				{

					add_hero_le(patient, retval);

					sprintf(g_dtp2,	get_ttx(695), healer->alias, patient->alias, GUI_get_personal_pronoun(patient->sex, GRAMMAR_CASE_3RD), retval); /* "TSA SEI DANK!@%s KANN %sS KRANKHEIT STOPPEN UND VERHILFT %s AUSSERD..." */
					GUI_output(g_dtp2);

					/* cure the disease */
					patient->disease[disease_id].status = DISEASE_STATUS_RECOVER;
					patient->disease[disease_id].time_counter = 0;

					if (herb == 999) {
						/* drop JORUGA & GULMOND LEAF */
						drop_item(healer, inv_slot_of_item(healer, ITEM_ID_JORUGAWURZEL), 1);
						drop_item(healer, inv_slot_of_item(healer, ITEM_ID_GULMOND_BLATT), 1);
					} else {
						/* drop the herb */
						drop_item(healer, inv_slot_of_item(healer, herb), 1);
					}

					retval = 1;
				} else {
					/* talent test failed */
					damage = 3;

					if (patient->le <= damage) {
						/* don't kill the patient: at least 1 LE should remain */
						damage = patient->le - 1;
					}

					sub_hero_le(patient, damage);

					sprintf(g_dtp2,	get_ttx(694), patient->alias, damage); /* "DAS HEILEN MISSLINGT!@%s ERLEIDET %d ZUSÄTZLICHE SCHADENSPUNKTE." */
					GUI_output(g_dtp2);
				}

			} else {
				/* failed to heal */
				sprintf(g_dtp2,	get_ttx(696), healer->alias, patient->alias); /* "%s KANN NICHT HERAUSFINDEN, VON WELCHER KRANKHEIT %s BEFALLEN IST." */
				GUI_output(g_dtp2);
			}
		}


		/* Original-Bug: this is inside 'if (is_hero_healable(patient))', so the text file is not
		 * restored if the patient cannot be healed (dead or petrified). */
		if ((from_event != 0) && (tx_file_bak != -1) && (tx_file_bak != ARCHIVE_FILE_SPELLTXT_LTX)) {
			load_tx(tx_file_bak);
		}
	}

	return retval;
}

/**
 * \brief   returns the hero of the group with the highest weight + load (undefined if there is none)
 */
struct struct_hero* get_heaviest_hero(void)
{
	struct struct_hero *hero;
	struct struct_hero *retval;

	signed int weight;
	signed int w_max;
	signed int i;

	w_max = 0;
	hero = get_hero(0);
	for (i = 0; i <= 6; i++, hero++) {

		if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id))
		{
			weight = hero->weight + hero->load;

			if (weight > w_max) {
				w_max = weight;
				retval = hero;
			}
		}
	}

	return retval;
}

/**
 * \brief   returns the weight of a hero including his load (ounces)
 */
signed int get_hero_weight(const struct struct_hero *hero)
{
	return hero->weight + hero->load;
}

/**
 * \brief   position of the hero of the group with the best chance for a talent test
 *
 *          Sum of the three attributes of the talent (current + mod) and the talent value.
 *          Dead or unconscious heroes are not skipped. Used as preselection in hero menus.
 */
signed int get_talented_hero_pos(const signed int talent_id)
{
	signed int pos;
	signed int cur;

	signed int max = -100;
	signed int return_hero_pos;
	struct struct_hero *hero = get_hero(0);

	for (pos = 0; pos <= 6; pos++, hero++) {

		if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id))
		{

			cur =	hero->attrib[g_talent_descriptions[talent_id].attrib1].current +
				hero->attrib[g_talent_descriptions[talent_id].attrib1].mod +
				hero->attrib[g_talent_descriptions[talent_id].attrib2].current +
				hero->attrib[g_talent_descriptions[talent_id].attrib2].mod +
				hero->attrib[g_talent_descriptions[talent_id].attrib3].current +
				hero->attrib[g_talent_descriptions[talent_id].attrib3].mod +
				hero->talents[talent_id];

			if (cur > max) {
				max = cur;
				return_hero_pos = pos;
			}
		}
	}

	return return_hero_pos;
}
