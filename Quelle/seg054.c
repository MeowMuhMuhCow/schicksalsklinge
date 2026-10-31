/*
 * seg054 - Herberge
 *
 * Inhalt:    Ort "Herberge": Reden mit dem Wirt, Essen und Trinken, Zimmer
 *            mieten (Schlafsaal, Einzelzimmer, Suite), Talent oder Magie
 *            anwenden, für die gebuchten Nächte schlafen und, wenn mit Taverne
 *            kombiniert, in die Taverne wechseln. Dazu die Behandlung von
 *            Helden, die in der Herberge einen langen Trank brauen.
 * Aufrufer:  do_inn über do_location (seg025) und seg069; TLK_inn aus dem
 *            Dialogsystem (seg030).
 * Daten:     g_inn_descr_table je Herberge ("quality": je kleiner, desto besser;
 *            Preisaufschlag in Prozent); gebuchte Nächte, Schlafqualität.
 * Besonders: Original-Bugs 7, 8, 9, 10, 11, 20 (Brauen, NPC). Neu: Einzelzimmer
 *            und Suite bekommen den Preisaufschlag des Schlafsaals statt des
 *            eigenen Preises (do_inn).
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg054.c
 */
#include <stdio.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg004.h"
#include "seg025.h"
#include "seg027.h"
#include "seg029.h"
#include "seg031.h"
#include "seg047.h"
#include "seg049.h"
#include "seg054.h"
#include "seg096.h"
#include "seg097.h"
#include "seg098.h"
#include "seg103.h"
#include "seg104.h"

static const struct inn_descr g_inn_descr_table[77] = {
	{ 14,   0 },
	{  7,  30 },
	{ 12, -10 },
	{ 11,   0 },
	{  6,  50 },
	{ 11,   1 },
	{ 10,  30 },
	{  9,  40 },
	{  7, -20 },
	{  7,  40 },
	{ 10,  20 },
	{ 11,  10 },
	{  7,   0 },
	{  8,  30 },
	{ 14,   0 },
	{ 20, -30 },
	{  9, -20 },
	{  1,  10 },
	{  8,  10 },
	{ 10, -30 },
	{  7,  20 },
	{ 12,  50 },
	{ 12, -10 },
	{  4,  70 },
	{  9,  40 },
	{ 14, -10 },
	{  3,  60 },
	{ 17,   0 },
	{  9,  20 },
	{  8,  50 },
	{  2,  50 },
	{ 13,  10 },
	{ 12,  20 },
	{  8,  40 },
	{ 11,  40 },
	{  9,  20 },
	{ 14,  40 },
	{ 11,  30 },
	{ 12, -20 },
	{  7,  40 },
	{ 16,  40 },
	{  3,  60 },
	{ 13, -40 },
	{  8,  10 },
	{ 12,  10 },
	{  4,  10 },
	{  5,  30 },
	{ 12,  20 },
	{ 16, -10 },
	{ 13,   0 },
	{  8,  30 },
	{  5,  40 },
	{ 10,  70 },
	{  5,  20 },
	{ 13, -10 },
	{  3,  40 },
	{ 14,   0 },
	{  9, -20 },
	{ 11,  30 },
	{ 17,  50 },
	{ 11,  20 },
	{  2,  30 },
	{ 10, -10 },
	{ 12, -20 },
	{ 15, -10 },
	{  3,  50 },
	{ 10,   0 },
	{ 10,  10 },
	{ 12,   0 },
	{  7,  40 },
	{ 14,   0 },
	{  4,  50 },
	{ 12, -10 },
	{ 11,   0 },
	{  9,   0 },
	{ 10,   0 },
	{ -1,   0 }
}; // ds:0x673c

signed char g_booked_inn_days;	// ds:0xe3f0
signed char g_sleep_quality;		// ds:0xe3f1

/**
 * \brief   talk to the innkeeper (random talk)
 */
void talk_inn(void)
{
	do_random_talk(1, 0);
}

/**
 * \brief   returns a pointer to the first brewing hero, who is not in this group, but in this location
 */
struct struct_hero* get_first_brewing_hero(void)
{
	struct struct_hero *hero;
	signed int i;

	hero = get_hero(0);
# if !FIX_0048
	/* Original-Bug 11: If NPC Curian got separated from the group for brewing a recipe at an inn,
	 * he is stuck in the inn. When the group enters the inn where they left him, no dialog appears.
	 * Also, switch group does not help, as his group cannot be selected.
	 * ("In dieser Gruppe ist momentan niemand in der Lage, etwas zu tun.") */
	for (i = 0; i < 6; i++, hero++)
# else
	/* Fix 48: auch den NSC (Platz 6) finden (Bright-Eyes-Fix) */
	for (i = 0; i < 7; i++, hero++)
# endif
	{
		if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id != gs_active_group_id) &&
			hero->flags.brewing && (hero->alchemy_inn_typeindex == gs_town_typeindex))
		{
			return hero;
		}
	}

	return NULL;
}

#if FIX_0050
/* Fix 50: eine Gruppe mit der aktiven vereinen (der Rumpf von GRP_merge() fuer eine einzelne Gruppe) */
static void fix_merge_group(const signed int group_id)
{
	signed int hero_pos;

	gs_groups_viewdir[group_id] = gs_groups_x[group_id]
		= gs_groups_y[group_id] = gs_groups_town_id[group_id]
		= gs_groups_dungeon_id[group_id] = gs_groups_dng_level[group_id]
		= gs_groups_viewdir_bak[group_id] = gs_groups_x_bak[group_id]
		= gs_groups_y_bak[group_id] = gs_town_groups_loctype_bak[group_id]
		= gs_groups_town_id_bak[group_id] = gs_groups_dungeon_id_bak[group_id]
		= gs_groups_dng_level_bak[group_id] = 0;

	gs_group_member_counts[group_id] = 0;

	for (hero_pos = 0; hero_pos <= 6; hero_pos++) {

		if ((get_hero(hero_pos)->typus != HERO_TYPE_NONE) && (get_hero(hero_pos)->group_id == group_id))
		{
			get_hero(hero_pos)->group_id = gs_active_group_id;
			gs_group_member_counts[gs_active_group_id]++;
		}
	}

	GRP_sort_heroes();
}
#endif

/**
 * \brief   location handler for inns
 */
void do_inn(void)
{
	signed int i;
	signed int answer;
	signed int done;
	signed int redraw_locline;
	signed int portion_size;
# if !FIX_0049
	/* Original-Bug 7: NPC Curian cannot do magic actions in an inn.
	 * Problem: no space allocated for the NPC (= seventh hero). */
	signed char magic_act[6];
# else
	/* Fix 49: Platz fuer den NSC (Bright-Eyes-Fix) */
	signed char magic_act[7];
# endif
#if FIX_0050
	signed int finalize_alchemy;
#endif
	struct struct_hero* hero;
	struct struct_hero* hero2;
	const struct inn_descr *inn;
	int32_t party_money;
	int32_t price;
	signed char stay;
	signed int tw_bak;
	signed int nr_heroes;
	int32_t price_schlafsaal;
	int32_t price_einzelzimmer;
	int32_t price_suite;
	signed int booked_days;
	signed int rested_days;

	done = 0;
	stay = 0;
	g_sleep_quality = -1;

#if !FIX_0050
	/* Original-Bug 8:
	 * If a group of >=2 heroes enters an inn where another hero is brewing,
	 * a textbox is shown with the possibility to interrupt the brewing process.
	 * However, this textbox is not shown if a single hero enters the inn. */

	if (gs_group_member_counts[gs_active_group_id] == 1) {
		/* current group consists only of a single hero */

		hero = get_first_hero_available_in_group();

		if (hero->flags.brewing) {

			draw_status_line();

			if (hero->recipe_timer != 0) {

				sprintf(g_dtp2, get_ttx(733), hero->alias, hero->recipe_timer, /* "%s BEFINDET SICH INMITTEN EINES ALCHIMISTISCHEN VERSUCHS, DER WOHL@..." */
					(char*)(hero->recipe_timer < 2 ? get_ttx(735) : get_ttx(736))); /* "TAG" / "TAGE" */

				answer = GUI_radio(g_dtp2, 2, get_ttx(734), get_ttx(537)); /* "VERSUCH ABBRECHEN" / "NÄCHSTE GRUPPE" */
				/* <HERO> befindet sich inmitten eines alchimistischen Versuchs, der wohl noch <DAYS> Tage dauert.
				 * * Versuch abbrechen
				 * * Weiter brauen lassen
				 */

				if (answer == 1) {
					/* abort brewing */
					do_alchemy(hero, hero->recipe_id, 1);
				} else {
					done = 1;
					g_combo_mode = 0;
					stay = 1;
				}
			} else {
				/* hero brewing, HERO_RECIPE_TIMER == 0. STAFFSPELL_TIMER not checked */
				do_alchemy(hero, hero->recipe_id, 0);
			}
		}
	}
	else if ((hero = get_first_brewing_hero())) {
		/* Original-Bug 9:
		 * If a group of heroes enters an inn where more then one hero is brewing (each of them forms his own group),
		 * and the first brewing hero has not spent enough time for the brewing process yet,
		 * you get asked if his brewing process should be aborted.
		 * If you answer 'no', you are not asked for the second brewing hero. Similarly, the other brewing heroes are
		 * not checked for having spent enough time to finalize the brewing. */

		draw_status_line();

		if (hero->recipe_timer != 0) {

			sprintf(g_dtp2,	get_ttx(733), hero->alias, hero->recipe_timer, /* "%s BEFINDET SICH INMITTEN EINES ALCHIMISTISCHEN VERSUCHS, DER WOHL@..." */
				(char*)(hero->recipe_timer < 2 ? get_ttx(735) : get_ttx(736))); /* "TAG" / "TAGE" */

			tw_bak = g_textbox_width;
			g_textbox_width = 4;

			answer = GUI_radio(g_dtp2, 2, get_ttx(734), get_ttx(562)); /* "VERSUCH ABBRECHEN" / "WEITER BRAUEN LASSEN" */
			/* <HERO> befindet sich inmitten eines alchimistischen Versuchs, der wohl noch <DAYS> Tage dauert.
			 * * Versuch abbrechen
			 * * Weiter brauen lassen
			 */

			g_textbox_width = tw_bak;

			if (answer == 1) {
				/* abort brewing */
				do_alchemy(hero, hero->recipe_id, 1);

				/* Original-Bug 10:
				 * If a group of heroes enters an inn where more then one hero is brewing
				 * (each of them forms his own group), you get asked if the brewing process of the first hero
				 * should be aborted. If you answer yes, all groups in the inn are merged, which may include
				 * other size-one-groups containing a brewing hero. The brewing process of these other brewing
				 * heroes won't be finalized, no matter if they have spent enough time in the inn or not,
				 * so these heroes are still considered to be brewing. If such an interrupted hero is the
				 * group leader of a group, and that group is selected, strange effects occur. */
				GRP_merge();
			}
		} else {
			/* hero brewing, HERO_RECIPE_TIMER == 0. STAFFSPELL_TIMER not checked */
			do_alchemy(hero, hero->recipe_id, 0);

			/* Original-Bug 20:
			 * If a group of heroes enters an inn where more then one hero is brewing
			 * (each of them forms his own group), and one of the brewing heroes has spent enough time in the in
			 * to finish the brewing, his brewing process is finalized. Subsequently, all groups in the inn are
			 * merged, which may include other size-one-groups containing a brewing hero. The brewing process of
			 * these other brewing heroes won't be finalized, no matter if they have spent enough time in the inn
			 * or not, so these heroes are still considered to be brewing. If such an interrupted hero is the
			 * group leader of a group, and that group is selected, strange effects occur. */
			GRP_merge();
		}
	}
#else
	/* Fix 50 (und 51): jeden brauenden Helden im Haus einzeln behandeln, auch beim Einzelhelden; nur die
	 * Gruppe eines abgeschlossenen oder abgebrochenen Brauers wird aufgeloest (Bright-Eyes-Fix fuer
	 * Original-Bug 8, 9, 10, 20; hier statt GRP_merge(), das alle Gruppen im Haus vereint) */
	hero = get_first_hero_available_in_group();
	if (hero->flags.brewing) {
		/* 'switch groups' just switched to the group of a single hero brewing in this inn */
		draw_status_line();

		if (hero->recipe_timer != 0) {

			sprintf(g_dtp2, get_ttx(733), hero->alias, hero->recipe_timer, (char*)(hero->recipe_timer < 2 ? get_ttx(735) : get_ttx(736))); /* "%s BEFINDET SICH INMITTEN EIN..." / "TAG" / "TAGE" */

			answer = GUI_radio(g_dtp2, 2, get_ttx(734), get_ttx(537)); /* "VERSUCH ABBRECHEN" / "NÄCHSTE GRUPPE" */

			if (answer == 1) {
				/* abort brewing */
				do_alchemy(hero, hero->recipe_id, 1);
			} else {
				done = 1;
				g_combo_mode = 0;
				stay = 1;
			}
		} else {
			/* hero brewing, HERO_RECIPE_TIMER == 0. STAFFSPELL_TIMER not checked */
			do_alchemy(hero, hero->recipe_id, 0);
		}
	} else {
		hero = get_hero(0);
		for (i = 0; i < 7; i++, hero++) {
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id != gs_active_group_id) &&
				hero->flags.brewing && (hero->alchemy_inn_typeindex == gs_town_typeindex))
			{
				draw_status_line();

				finalize_alchemy = -1;

				if (hero->recipe_timer != 0) {

					sprintf(g_dtp2, get_ttx(733), hero->alias, hero->recipe_timer, (char*)(hero->recipe_timer < 2 ? get_ttx(735) : get_ttx(736))); /* "%s BEFINDET SICH INMITTEN EIN..." / "TAG" / "TAGE" */

					tw_bak = g_textbox_width;
					g_textbox_width = 4;

					answer = GUI_radio(g_dtp2, 2, get_ttx(734), get_ttx(562)); /* "VERSUCH ABBRECHEN" / "WEITER BRAUEN LASSEN" */

					g_textbox_width = tw_bak;

					if (answer == 1) {
						/* abort brewing */
						finalize_alchemy = 1;
					}
				} else {
					/* hero brewing, HERO_RECIPE_TIMER == 0. STAFFSPELL_TIMER not checked */
					finalize_alchemy = 0;
				}

				if (finalize_alchemy != -1) {

					do_alchemy(hero, hero->recipe_id, finalize_alchemy);

					/* nur diese Gruppe mit der aktiven vereinen (wie in GRP_merge()) */
					fix_merge_group(hero->group_id);
				}
			}
		}
	}
#endif

#if FIX_0334
	/* Fix 334: hat der Wirt die Gruppe hinausgeworfen, spricht er sie an (Dialog im Rauswurf-Zustand) und sie muss gehen
	 * (ohne copy_palette(): die Herberge wurde nicht gezeichnet, Osterpatch NRS) */
	if ((done == 0) && gs_inn_kicked_flags[gs_town_typeindex]) {
		talk_inn();
		/* der Rauswurf gilt fuer diesen einen Besuch (wie in der Taverne); so sperren auch alte Spielstaende, in die
		 * der Fehler 425 Merker geschrieben hat, eine Herberge nicht fuer immer */
		gs_inn_kicked_flags[gs_town_typeindex] = 0;
		leave_location();
		return;
	}
#endif

	if (done == 0) {

		redraw_locline = g_request_refresh = 1;

		draw_loc_icons(g_combo_mode == 0 ? 7 : 8, MENU_ICON_TALK, MENU_ICON_ORDER_FOOD, MENU_ICON_BOOK_BED, MENU_ICON_APPLY_TALENT, MENU_ICON_MAGIC, MENU_ICON_SLEEP, MENU_ICON_LEAVE, MENU_ICON_TAVERN);

# if !FIX_0049
	/* Original-Bug 7: NPC Curian cannot do magic actions in an inn. */
		for (i = 0; i < 6; i++)
# else
		for (i = 0; i < 7; i++)
# endif
		{
			magic_act[i] = 0;
		}
	}

	while (done == 0) {

		if (g_request_refresh != 0) {

			draw_main_screen();

			disable_ani();

			load_ani(4);

			init_ani(0);

			GUI_print_loc_line(get_tx(gs_town_locdata));

			g_request_refresh = redraw_locline = 0;
		}

		if (redraw_locline != 0) {

			GUI_print_loc_line(get_tx(gs_town_locdata));

			redraw_locline = 0;
		}

		inn = &g_inn_descr_table[gs_town_typeindex];

		handle_gui_input();

		if (g_mouse_rightclick_event || g_action_id == KEY_SCAN_CODE_PAGE_UP) {

			answer = GUI_radio(get_ttx(345), g_combo_mode == 0 ? 7 : 8, /* "HERBEGSOPTIONEN" */
						get_ttx(343), get_ttx(470), /* "REDEN" / "ESSEN UND TRINKEN" */
						get_ttx(344), get_ttx(212), /* "ZIMMER MIETEN" / "TALENT ANWENDEN" */
						get_ttx(310), get_ttx(316), /* "MAGIE ANWENDEN" / "SCHLAFEN" */
						get_ttx(347), get_ttx(823)) - 1; /* "HERBERGE VERLASSEN" / "TAVERNE BESUCHEN" */

			if (answer != -2) {
				g_action_id = answer + ACTION_ID_ICON_1;
			}
		}

		if (g_action_id == ACTION_ID_ICON_1) {

			talk_inn();
			g_request_refresh = 1;
#if FIX_0334
			/* Fix 334: wirft der Wirt im Gespraech hinaus, verlaesst die Gruppe die Herberge */
			if (gs_inn_kicked_flags[gs_town_typeindex]) {
				done = 1;
			}
#endif

		} else if (g_action_id == ACTION_ID_ICON_2) { /* order food */

			price = count_heroes_in_group() * (6L - inn->quality / 4L);

			/* apply the price modifier of the inn (percent) */
			price += (price * inn->price_mod) / 100L;

			sprintf(g_dtp2,	get_ttx(473), (signed int)price); /* "<WIR HABEN ALLES DA, WAS EUER HERZ BEGEHRT.@FÜR %d SILBERSTÜCKE WER..." */

			if (GUI_bool(g_dtp2)) {

				party_money = get_party_money();
				price *= 10;

				if (price < 0) {
					price = 1;
				}

				if (party_money < price) {
					GUI_output(get_ttx(401)); /* "SOVIEL GELD HABT IHR DOCH GAR NICHT!" */
				} else {

					GUI_output(inn->quality < 5 ? get_ttx(475) : (inn->quality < 15 ? get_ttx(476) : get_ttx(477))); /* "EIN ERLESENES MENÜ MIT GUTEM..." / "ZWAR BEKOMMT IHR HIER NICHT G..." / "DAS BIER SCHMECKT ETWAS ABGES..." */

					for (i = 0, hero2 = get_hero(0); i <= 6; i++, hero2++) {

						if ((hero2->typus != HERO_TYPE_NONE) &&	(hero2->group_id == gs_active_group_id) && !hero2->flags.dead)
						{
							portion_size = (21 - inn->quality) * 20;
							if (portion_size > 100) {
								portion_size = 100;
							}

							hero2->hunger -= (unsigned char)portion_size;

							if (hero2->hunger < 0) {
								hero2->hunger = 0;
							}

							portion_size = (21 - inn->quality) * 30;
							if (portion_size > 100) {
								portion_size = 100;
							}

							hero2->thirst -= (unsigned char)portion_size;

							if (hero2->thirst < 0) {
								hero2->thirst = 0;
							}
						}
					}

					party_money -= price;
					set_party_money(party_money);
				}
			}

		} else if (g_action_id == ACTION_ID_ICON_3 && g_sleep_quality == -1) {

			price_schlafsaal = 5;
			price_einzelzimmer = 30;
			price_suite = 100;
			price_schlafsaal += price_schlafsaal * inn->price_mod / 100;
			/* Original-Bug: the modifier is taken from the (already modified) dormitory price
			 * instead of price_einzelzimmer / price_suite. */
#if FIX_0052
			/* Fix 52: Aufschlag vom eigenen Preis */
			price_einzelzimmer += price_einzelzimmer * inn->price_mod / 100;
			price_suite += price_suite * inn->price_mod / 100;
#else
			price_einzelzimmer += price_schlafsaal * inn->price_mod / 100;
			price_suite += price_schlafsaal * inn->price_mod / 100;
#endif

			tw_bak = g_textbox_width;
			g_textbox_width = 5;

			sprintf(g_dtp2,	get_ttx(397), (signed int)price_schlafsaal); /* "SCHLAFSAAL (%d H PRO P.)" */

			sprintf(g_dtp2 + 50, get_ttx(398), (signed int)price_einzelzimmer); /* "EINZELZIMMER (%d H PRO P.)" */

			sprintf(g_dtp2 + 100, get_ttx(399), (signed int)price_suite); /* "SUITE (%d H PRO P.)" */

			g_sleep_quality = GUI_radio(get_ttx(396), 3, g_dtp2, g_dtp2 + 50, g_dtp2 + 100); /* "<WAS FÜR EIN ZIMMER DARF ES DENN SEIN?>" */ /* 1 = Schlafsaal, 2 = Einzelzimmer, 3 = Suite, -1 = ESC */

			if (g_sleep_quality != -1) {
				g_booked_inn_days = GUI_input(get_ttx(826), 2); /* "<WIE VIELE NÄCHTE WOLLT IHR HIERBLEIBEN?>" */
			}

			if (g_booked_inn_days <= 0) {
				g_sleep_quality = -1;
			}

			g_textbox_width = tw_bak;

			if (g_sleep_quality != -1) {

				nr_heroes = count_heroes_in_group();

				price_schlafsaal *= nr_heroes;
				price_einzelzimmer *= nr_heroes;
				price_suite *= nr_heroes;
				party_money = get_party_money();

				price = g_sleep_quality == 1 ? price_schlafsaal : (g_sleep_quality == 2 ? price_einzelzimmer : price_suite);

				price *= g_booked_inn_days;

				if (price < 0) {
					price = 1;
				}

				if (party_money < price) {
					GUI_output(get_ttx(401)); /* "SOVIEL GELD HABT IHR DOCH GAR NICHT!" */
					g_sleep_quality = -1;
				} else {
					party_money -= price;
					set_party_money(party_money);
					GUI_output(get_ttx(402)); /* "<VIELEN DANK!>@IHR WERDET AUF EURE ZIMMER GEBRACHT." */
				}
			}

		} else if (g_action_id == ACTION_ID_ICON_4) {

			if (g_sleep_quality != -1) {
				GUI_use_talent2(0, get_ttx(395)); /* "WER SOLL EIN TALENT ANWENDEN?" */
				redraw_locline = 1;
			} else {
				GUI_output(get_ttx(346)); /* "IHR SOLLTET ZUERST EIN ZIMMER MIETEN." */
			}

		} else if (g_action_id == ACTION_ID_ICON_5) {

			if (g_sleep_quality != -1) {

				answer = select_hero_ok(get_ttx(317)); /* "WELCHER HELD SOLL MAGIE ANWENDEN?" */

				if (answer != -1) {

					hero = get_hero(answer);

					if (hero->typus >= HERO_TYPE_HEXE) {

						if (magic_act[answer] != 0) {

							GUI_output(get_ttx(334)); /* "DIE LETZTE MAGIEANWENDUNG HAT SCHON MEHR ALS GENUG AN DER KONSTITUT..." */

						} else {
							magic_act[answer] = (signed char)use_magic(hero);
						}
					} else {
						GUI_output(get_ttx(330)); /* "DIESE OPTION IST AUSSCHLIESSLICH MAGIEKUNDIGEN VORBEHALTEN!" */
					}
				}
			} else {
				GUI_output(get_ttx(346)); /* "IHR SOLLTET ZUERST EIN ZIMMER MIETEN." */
			}

		} else if (g_action_id == ACTION_ID_ICON_6) {

			if (g_sleep_quality != -1 && g_booked_inn_days > 0) {

				if (GUI_bool(get_ttx(318))) { /* "WIRKLICH SCHLAFEN?@KEINE WEITEREN OPTIONEN MEHR MÖGLICH!" */

					booked_days = g_booked_inn_days;

					/* good inns improve, bad inns worsen the sleep quality */
					if (inn->quality < 8) {

						g_sleep_quality++;

					} else if (inn->quality > 15) {

						g_sleep_quality--;
					}

					done = 1;
					g_combo_mode = 0;
					g_food_mod = 1;

					do {
						timewarp_until_time_of_day(HOURS(8));

					} while (--g_booked_inn_days);

					g_food_mod = 0;

					hero = get_hero(0);
					for (i = 0; i <= 6; i++, hero++) {

						if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id))
						{
							if (booked_days > 1) {
								/* Original-Bug 7:
								 * magic_act[6] = 0 writes out of boundary.
								 * Fixed above. */
								magic_act[i] = 0;
							}

							if (magic_act[i] != 1) {

								for (rested_days = 0; rested_days < booked_days; rested_days++)
								{
									GRP_hero_sleep(hero, g_sleep_quality);
								}
							}
						}
					}
				}
			} else {
				GUI_output(get_ttx(346)); /* "IHR SOLLTET ZUERST EIN ZIMMER MIETEN." */
			}
		} else if (g_action_id == ACTION_ID_ICON_7) {

			if (g_sleep_quality != -1) {

				if (GUI_bool(get_ttx(400))) { /* "<WOLLT IHR GEHEN, OBWOHL DANN DAS ZIMMER WIEDER WEG IST?>" */
					done = 1;
					g_combo_mode = 0;
				}
			} else {
				done = 1;
				g_combo_mode = 0;
			}

		} else if (g_action_id == ACTION_ID_ICON_8 && g_combo_mode != 0) {

			const struct inn_descr *tavern = &g_tavern_descr_table[gs_town_typeindex];

			if (tavern->quality >= 6 && tavern->quality <= 13 && gs_day_timer < HOURS(11) && gs_day_timer > HOURS(3)) {

				GUI_output(get_ttx(801)); /* "GEÖFFNET VON@11.00 UHR BIS 3.00 UHR" */

			} else if ((tavern->quality < 6 || tavern->quality > 13) && gs_day_timer < HOURS(16) && gs_day_timer > HOURS(3)) {

				GUI_output(get_ttx(481)); /* "GEÖFFNET VON@16.00 UHR BIS 3.00 UHR" */

			} else if (g_sleep_quality != -1) {

				if (GUI_bool(get_ttx(400))) { /* "<WOLLT IHR GEHEN, OBWOHL DANN DAS ZIMMER WIEDER WEG IST?>" */
					done = 1;
					g_combo_mode = 2;
				}
			} else {
				done = 1;
				g_combo_mode = 2;
			}
		}
	}

	if (!stay) {
		copy_palette();
		leave_location();
	} else {
		GRP_switch_to_next(1);
		draw_status_line();
	}
}

/**
 * \brief   dialog handler for the innkeeper (HERBERG.TLK)
 *
 * \param   state       the current dialog state
 */
void TLK_inn(const signed int state)
{
	struct struct_hero *hero = get_first_hero_available_in_group();

	if (!state) {
		g_dialog_next_state = (gs_inn_kicked_flags[gs_town_typeindex] ? 1 : 2);
	} else if (state == 1 || state == 14) {
		gs_inn_kicked_flags[gs_town_typeindex] = 1;
	} else if (state == 11) {
		tumult();
		gs_town_outlawed_flags[gs_town_id] = 1;
		gs_inn_kicked_flags[gs_town_typeindex] = 1;
	} else if (state == 12) {
		/* CH + 5 */
		g_dialog_next_state = (test_attrib(hero, ATTRIB_CH, 5) > 0 ? 14 : 11);
	} else if (state == 13) {
		/* CH + 0 */
		g_dialog_next_state = (test_attrib(hero, ATTRIB_CH, 0) > 0 ? 14 : 7);
	} else if (state == 15) {
		/* CH - 3 */
		g_dialog_next_state = (test_attrib(hero, ATTRIB_CH, -3) > 0 ? 16 : 17);
	} else if (state == 17) {
		g_action_id = ACTION_ID_ICON_2;
	}
}
