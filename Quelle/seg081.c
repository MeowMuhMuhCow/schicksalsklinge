/*
 * seg081 - Dungeon "Daspota-Schatz" (DNG06) und Fallgrube der Kultstaette (DNG09)
 *
 * Inhalt:    DNG06_handler() steuert das Piratenversteck des Daspota-Schatzes zwischen
 *            Rybon und Thoss: Buch "Kaisersprüche Hals", Schlafraum mit Dukaten, Proviant,
 *            Armbrustfallen, Falltuer mit Sicherungsstein, Geheimfach mit goldenem
 *            Schluessel, Tuer-Countdown mit Bolzen, Speergrube, zwei Hebel, die zwei
 *            Gruppen gleichzeitig ziehen muessen, Klingenscheibe, Figurinen, der Schatz
 *            selbst (500 Dukaten und Gegenstaende), Kampf und Ausgang. DNG09_pitfall() ist
 *            die Fallgrube der Kultstaette des Namenlosen (1,3,12): wer hineinfaellt, wird
 *            eine eigene Gruppe, die mit zwei Seilen wieder herausgeholt werden kann.
 * Aufrufer:  DNG06_handler() ueber g_dng_handlers (seg076), DNG06_fight_intro() aus
 *            GUI_print_fight_intro_msg() (seg097), DNG09_pitfall() aus DNG09_handler()
 *            (seg084), die Truhen ueber g_dng06_specialchests (seg092).
 * Daten:     g_dng06_specialchests[3], Satzbausteine "%s WAR " / "%s UND %s WAREN ".
 * Besonders: Armbrustfalle (0,2,1) zaehlt nur die Heldenplaetze 0 und 1, bei 0 Treffern
 *            wird hero_second uninitialisiert benutzt. Die beiden Hebel pruefen die
 *            andere Gruppe nur nach x/y, nicht nach Dungeon und Ebene. DNG09_pitfall():
 *            die Gefahrensinn-Schleife beginnt beim ersten verfuegbaren Helden und laeuft
 *            7 Plaetze weit, also ueber das Heldenfeld hinaus. gs_estorik_known wird nie
 *            gesetzt.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg081.c
 */

#include <stdio.h>
#include <string.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg007.h"
#include "seg025.h"
#include "seg032.h"
#include "seg049.h"
#include "seg075.h"
#include "seg076.h"
#include "seg081.h"
#include "seg092.h"
#include "seg096.h"
#include "seg097.h"
#include "seg103.h"
#include "seg105.h"

struct struct_chest g_dng06_specialchests[3] = {
	{ DNG_POS(0,7,3), 0, NULL,			NULL,			DNG06_chest00_loot, 0, 0, 40, },
	{ DNG_POS(1,9,4), 0, use_lockpicks_on_chest,	chest_poisoned2,	DNG06_chest01_loot, 0, 0, 0 },
	{     -1, 0, NULL, NULL, NULL, 0, 0, 0 }
}; // ds:0x9610
static const char g_str_s_war[8] = "%s WAR "; // ds:0x964f
static const char g_str_s_und_s_waren[17] = "%s UND %s WAREN "; // ds:0x9657


/**
 * \brief   dungeon handler of the hideout with the treasure of Daspota
 */
signed int DNG06_handler(void)
{
	signed int i;
	signed int target_pos;
	signed int l3; /* multi use: counter, flag, dice */
	signed int hero_pos;
	signed int tw_bak;
	int32_t p_money;
	struct struct_hero *hero;
	struct struct_hero *hero_first;
	struct struct_hero *hero_second;
	uint8_t *amap_ptr;

	amap_ptr = g_dng_map;

	tw_bak = g_textbox_width;
	g_textbox_width = 7;

	target_pos = DNG_POS(gs_dungeon_level, gs_x, gs_y);

	if (target_pos == DNG_POS(0,14,5) && target_pos != gs_dng_pos_bak && !gs_dng06_book_flag)
	{
		if (GUI_bool(get_tx(1)) && give_new_item_to_group(ITEM_ID_BUCH__KAISERSPRUECHE_HALS, 1, 1)) /* "OFFENSICHTLICH EINE BEDÜRFNISANSTALT. NEBEN DEM DONNERBALKEN LIEGT..." */
		{
			gs_dng06_book_flag = 1;
		}

	}

	if (target_pos == DNG_POS(0,9,9) && target_pos != gs_dng_pos_bak)
	{
		GUI_output(get_tx(35)); /* "HIER STEHT EIN GROSSES BIERFASS. LEIDER IST ES NOCH NICHT ANGEZAPFT..." */

	} else if (target_pos == DNG_POS(0,4,13) && target_pos != gs_dng_pos_bak)
	{
		if (GUI_bool(get_tx(2))) /* "DIES IST EIN SCHLAFRAUM MIT EIN PAAR SPINTEN UND EIN PAARPRITSCHEN...." */
		{
			strcpy(g_dtp2, get_tx(3)); /* "IHR FINDET EINIGE KLEIDUNGSSTÜCKE IN DEN GEWAGTESTENFARBZUSAMMENSTE..." */

			if (!gs_dng06_money_flag)
			{
				gs_dng06_money_flag = 1;

				hero = get_first_hero_available_in_group();

				i = dice_roll(3, 20, 0);

				add_party_money(100L * i);

				sprintf(g_text_output_buf, get_tx(4), hero->alias, i); /* "@AUSSERDEM ENTDECKT %s NOCH %d DUKATEN UNTER EINER DER PRITSCHEN." */

				strcat(g_dtp2, g_text_output_buf);
			}

			GUI_output(g_dtp2);
		}

	} else if (target_pos == DNG_POS(0,5,3) && target_pos != gs_dng_pos_bak && !gs_dng06_proviant_flag)
	{
		if (GUI_bool(get_tx(5))) /* "IN DEN FÄSSERN UND TRUHEN HIER SIND DIE UNTERSCHIEDLICHSTEN LEBENSM..." */
		{
			give_new_item_to_group(ITEM_ID_PROVIANTPAKET, 1, 40);
			gs_dng06_proviant_flag = 1;
		}

	} else if (target_pos == DNG_POS(0,2,1) && target_pos != gs_dng_pos_bak)
	{
		/* crossbow trap: hits the first one or two heroes.
		 * Original-Bug: the loop only checks the hero slots 0 and 1 instead of counting the heroes of
		 * the group. If neither slot belongs to the group, l3 == 0 and the "%s UND %s WAREN " text
		 * uses the uninitialized pointer hero_second; if only one of them does, only one hero is hit. */
#if FIX_0109
		/* Fix 109: die ersten beiden handlungsfaehigen Helden der Gruppe treffen */
		hero_first = get_first_hero_available_in_group();
		hero_second = (struct struct_hero*)get_second_hero_available_in_group();
		l3 = (hero_second != NULL ? 2 : 1);
#else
		hero = get_hero(0);
		for (i = l3 = 0; i < 2; i++, hero++)
		{
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !hero->flags.dead)
			{
				l3++;
			}
		}

		hero_first = get_first_hero_available_in_group();
		if (l3 > 1)
		{
			hero_second = (struct struct_hero*)get_second_hero_available_in_group();
		}
#endif

		sprintf(g_dtp2, (char*)(l3 == 1 ? g_str_s_war : g_str_s_und_s_waren),
			hero_first->alias, hero_second->alias);

		strcat(g_dtp2, get_tx(6)); /* "VON ARMBRUSTBOLZEN GETROFFEN. JETZT KÖNNT IHR AUCH DIE LÖCHER AN DE..." */

		GUI_output(g_dtp2);

		sub_hero_le(hero_first, random_schick(6));
		if (l3 > 1)
		{
			sub_hero_le(hero_second, random_schick(6));
		}

	} else if (target_pos == DNG_POS(0,9,7) && target_pos != gs_dng_pos_bak && !gs_dng06_pitdoor_flag)
	{
		/* trapdoor: only noticed with a successful Gefahrensinn test; a second successful test finds
		 * the loose stone that secures it (gs_dng06_pitdoor_flag, which also blocks the door at (0,11,1),
		 * see DNG_door() in seg076). Otherwise the group falls. */
		hero = get_hero(0);
		for (i = l3 = 0; i <= 6; i++, hero++)
		{
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
				!hero->flags.dead && (test_talent(hero, TA_GEFAHRENSINN, 5) > 0))
			{
				l3 = 1;
				break;
			}
		}

		if (l3 != 0)
		{
			if (GUI_bool(get_tx(7))) /* "VOR EUCH SCHEINT DER BODEN EINIGE ZU REGELMÄSSIGE RITZENAUFZUWEISEN..." */
			{
				GUI_output(get_tx(8)); /* "OFFENSICHTLICH IST VOR EUCH AUF DEM BODEN EINE FALLTÜR." */

				hero = get_hero(0);
				for (i = l3 = 0; i <= 6; i++, hero++)
				{
					if ((hero->typus != HERO_TYPE_NONE) &&
						(hero->group_id == gs_active_group_id) &&
						!hero->flags.dead &&
						(test_talent(hero, TA_GEFAHRENSINN, 5) > 0))
					{
						l3 = 1;
						break;
					}
				}

				if (l3 != 0 && GUI_bool(get_tx(9))) /* "AN DER WAND HINTER EUCH ENTDECKT IHR JETZT AUCH EINEN LOSEN STEIN,H..." */
				{
					gs_dng06_pitdoor_flag = 1;

					GUI_output(get_tx(10)); /* "IHR HÖRT EIN LEISES, SCHABENDES GERÄUSCH AN DER FALLTÜR." */
				}

				if (!gs_dng06_pitdoor_flag)
				{
					DNG_pitfall_add_pit(6);
				}
			} else {
				if (!gs_dng06_pitdoor_flag)
				{
					DNG_pitfall_add_pit(6);
				}
			}
		} else {
			if (!gs_dng06_pitdoor_flag)
			{
				DNG_pitfall_add_pit(6);
			}
		}

	} else if (target_pos == DNG_POS(0,8,7) && target_pos != gs_dng_pos_bak && gs_dng06_goldkey_flag != 2)
	{
		hero = get_hero(0);
		for (i = l3 = 0; i <= 6; i++, hero++)
		{
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
				!hero->flags.dead && (test_talent(hero, TA_SINNESSCHAERFE, 10) > 0))
			{
				l3++;
			}
		}

		/* secret compartment: gs_dng06_goldkey_flag 0 = not found, 1 = found, 2 = emptied */
		if (gs_dng06_goldkey_flag || l3 != 0)
		{
			gs_dng06_goldkey_flag = 1;

			if (GUI_bool(get_tx(11))) /* "MEHR DURCH ZUFALL, ALS ABSICHT, ENTDECKT IHR EIN GEHEIMFACH. ALS IH..." */
			{
				gs_dng06_goldkey_flag = 2;

				give_new_item_to_group(ITEM_ID_GOLDENER_SCHLUESSEL, 1, 1);

				/* TODO: This is not neccessary */
				hero = get_first_hero_available_in_group();

				add_party_money(3000L); /* 30 D */
			}
		}

	} else if (target_pos == DNG_POS(0,11,6) && target_pos != gs_dng_pos_bak && gs_dng06_countdown_timer)
	{
#if !defined(__BORLANDC__)
		if (div16(amap_ptr[MAP_POS(11,1)]) != DNG_TILE_CLOSED_DOOR ||
			div16(amap_ptr[MAP_POS(2,6)]) != DNG_TILE_CLOSED_DOOR ||
			div16(amap_ptr[MAP_POS(11,4)]) != DNG_TILE_CLOSED_DOOR)
#else
		/* REMARK: enforce cast to uint16_t */
		if (div16((_AL = amap_ptr[MAP_POS(11,1)], _AH = 0, _AX)) != DNG_TILE_CLOSED_DOOR ||
			div16((_AL = amap_ptr[MAP_POS(2,6)], _AH = 0, _AX)) != DNG_TILE_CLOSED_DOOR ||
			div16((_AL = amap_ptr[MAP_POS(11,4)], _AH = 0, _AX)) != DNG_TILE_CLOSED_DOOR)
#endif
		{
			gs_dng06_countdown_timer--;

			l3 = random_schick(5);

			hero = get_first_hero_available_in_group();

			sprintf(g_dtp2, (l3 == 1 ? get_tx(12) : get_tx(13)), hero->alias); /* "%s WIRD VON EINEM ARMBRUSTBOLZEN GETROFFEN." / "%s WIRD VON MEHREREN ARMBRUSTBOLZEN DURCHS..." */
			GUI_output(g_dtp2);

			sub_hero_le(hero, dice_roll(l3, 6, 0));
		}

	} else if (target_pos == DNG_POS(1,13,5) && target_pos != gs_dng_pos_bak)
	{
		do {
			i = GUI_radio(get_tx(14), 3, get_tx(15), get_tx(16), get_tx(17)); /* "VOR EUCH SCHEINT DER BODEN EI..." / "WEITERGEHEN?" / "DARÜBERSPRINGEN?" / "DEN BODEN UNTERSUCHEN?" */

		} while (i == -1);

		hero = get_first_hero_available_in_group();

		/* walking on and examining the floor both end in the spear pit (texts 18/19) */
		if (i == 1 || i == 3)
		{
			sprintf(g_dtp2,	(i == 1 ? get_tx(18) : get_tx(19)), hero->alias, /* "%s IST IN EINE MIT SPEEREN GESPICKTE GRUBE..." / "ALS IHR AUF DEN KNIEN HERUMRUTSCHT UND DAS..." */
				GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST),
				GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST),
				hero->alias,
				GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_2ND));

			GUI_output(g_dtp2);

			sub_hero_le(hero, dice_roll(3, 6, 0));
		} else {

			hero = get_hero(0);
			for (i = 0; i <= 6; i++, hero++)
			{
				if ((hero->typus != HERO_TYPE_NONE) &&
					(hero->group_id == gs_active_group_id) &&
					!hero->flags.dead &&
					(test_talent(hero, TA_KOERPERBEHERRSCHUNG, hero->rs_bonus) <= 0))
				{
					sprintf(g_dtp2,	get_tx(20), hero->alias, /* "%s IST ZU KURZ GESPRUNGEN. DABEI HAT DER BODEN NACHGEGEBEN UND %s I..." */
						GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST),
						GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_3RD));

					GUI_output(g_dtp2);

					sub_hero_le(hero, dice_roll(3, 6, 0));
				}
			}
		}

	} else if (target_pos == DNG_POS(1,1,2) && target_pos != gs_dng_pos_bak)
	{
		/* Hole in the wall: want to reach into it? want to move the lever?
		 * Works only if another group stands at the second lever (1,3,6) at the same time.
		 * Original-Bug: the other group is only checked by x/y, not by dungeon and level, so a group
		 * on square (3,6) anywhere else (other level, other dungeon, a town) counts as well.
		 * The same for the second lever below. */
		if (GUI_bool(get_tx(21)) && GUI_bool(get_tx(22))) /* "HIER IST EIN LOCH IN DER WAND.@WOLLT IHR H..." / "IN DEM LOCH IST EIN HEBEL ZU SPÜREN.@WOLLT..." */
		{
			for (i = l3 = 0; i < 6; i++)
			{
				if (gs_groups_x[i] == 3 && gs_groups_y[i] == 6 &&
#if FIX_0110
					/* Fix 110: die andere Gruppe muss in diesem Dungeon auf dieser Ebene stehen */
					gs_groups_dungeon_id[i] == gs_dungeon_id && gs_groups_dng_level[i] == gs_dungeon_level &&
#endif
					gs_active_group_id != i)
				{
					hero = get_hero(0);
					for (hero_pos = 0; hero_pos <= 6; hero_pos++, hero++)
					{
						if ((hero->typus != HERO_TYPE_NONE) &&
							(hero->group_id == i))
						{
							l3 = 1;
							break;
						}
					}
				}
			}

			if (l3 != 0)
			{
				/* some heroes are in the right position: hear gentle click */
				GUI_output(get_tx(24)); /* "EIN LEISES KLICKEN DRINGT AN EURE OHREN." */

				gs_dng06_lever_flag = 1;
			} else {
				/* liquid from ceiling causes damage */
				hero = get_first_hero_available_in_group();

				sprintf(g_dtp2, get_tx(23), hero->alias); /* "%s ZUCKT ZURÜCK. EINE FLÜSSIGKEIT SPRITZT PLÖTZLICH AUS DER DECKEUN..." */
				GUI_output(g_dtp2);

				sub_hero_le(hero, 2);
			}
		}

	} else if (target_pos == DNG_POS(1,3,6) && target_pos != gs_dng_pos_bak)
	{
		/* Hole in the wall: want to reach into it? want to move the lever? */
		if (GUI_bool(get_tx(21)) && GUI_bool(get_tx(22))) /* "HIER IST EIN LOCH IN DER WAND.@WOLLT IHR H..." / "IN DEM LOCH IST EIN HEBEL ZU SPÜREN.@WOLLT..." */
		{
			for (i = l3 = 0; i < 6; i++)
			{
				if (gs_groups_x[i] == 1 && gs_groups_y[i] == 2 &&
#if FIX_0110
					/* Fix 110: die andere Gruppe muss in diesem Dungeon auf dieser Ebene stehen */
					gs_groups_dungeon_id[i] == gs_dungeon_id && gs_groups_dng_level[i] == gs_dungeon_level &&
#endif
					gs_active_group_id != i)
				{
					hero = get_hero(0);
					for (hero_pos = 0; hero_pos <= 6; hero_pos++, hero++)
					{
						if ((hero->typus != HERO_TYPE_NONE) &&
							(hero->group_id == i))
						{
							l3 = 1;
							break;
						}
					}
				}
			}

			if (l3 != 0)
			{
				/* some heroes are in the right position: hear gentle click */
				GUI_output(get_tx(24)); /* "EIN LEISES KLICKEN DRINGT AN EURE OHREN." */

				gs_dng06_lever_flag = 1;
			} else {
				/* liquid from ceiling causes damage */
				hero = get_first_hero_available_in_group();

				sprintf(g_dtp2,	get_tx(23), hero->alias); /* "%s ZUCKT ZURÜCK. EINE FLÜSSIGKEIT SPRITZT PLÖTZLICH AUS DER DECKEUN..." */
				GUI_output(g_dtp2);

				sub_hero_le(hero, 2);
			}
		}

	} else if (target_pos == DNG_POS(1,5,8) && target_pos != gs_dng_pos_bak && !gs_dng06_lever_flag)
	{
		hero = get_first_hero_available_in_group();

		/* large disk comes down from ceiling */
		sprintf(g_dtp2,	get_tx(25), hero->alias); /* "PLÖTZLICH KOMMT MIT LAUTEM KREISCHEN EINE GROSSE BLITZENDE SCHEIBE..." */
		GUI_output(g_dtp2);

		if (test_talent(hero, TA_KOERPERBEHERRSCHUNG, 0) > 0 && test_talent(hero, TA_GEFAHRENSINN, 0) > 0)
		{
			/* evasion succeeds */
			sprintf(g_dtp2, get_tx(26), GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST)); /* "%s KANN SICH GERADE NOCH ZUR SEITE WERFEN, BEVOR DIE SCHEIBE KURZ V..." */
			GUI_output(g_dtp2);

		} else {
			/* disk causes heavy damage */
			sprintf(g_dtp2,	get_tx(27), GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_4TH)); /* "SIE VERLETZT %s SCHWER, BEVOR SIE WIEDER IN DER DECKE VERSCHWINDET." */
			GUI_output(g_dtp2);

			sub_hero_le(hero, dice_roll(3, 6, 0));
		}

	} else if (target_pos == DNG_POS(1,7,13) && target_pos != gs_dng_pos_bak && !gs_dng06_fight19_flag)
	{
		if (GUI_bool(get_tx(28))) /* "HIER STEHEN ZWEI FIGURINEN, DIE OFFENSICHTLICH THORWALERDARSTELLEN...." */
		{
			hero = get_first_hero_available_in_group();

			sprintf(g_dtp2,	get_tx(29), hero->alias, /* "%s FÜHLT EINEN STECHENDEN SCHMERZ IN DER HAND, DER JEDOCH VORÜBERIS..." */
				GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST),
				GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_1ST));

			GUI_output(g_dtp2);

			sub_hero_le(hero, 2);
		}

	} else if (target_pos == DNG_POS(1,6,11) && target_pos != gs_dng_pos_bak && !gs_dng06_dasptreas_found)
	{
		/* treasure of Daspota found */
		/* REMARK: should be modified, in a way that looting is still possible */
		GUI_output(get_tx(36)); /* "IHR HABT ENDLICH DEN BERÜHMTEN DASPOTA SCHATZ GEFUNDEN. EIN GANZERH..." */

		p_money = get_party_money();
		p_money += 50000L;
		set_party_money(p_money);

		loot_multi_chest(gs_dng06_chest_dasptreas, get_tx(37)); /* "DER DASPOTA SCHATZ" */

		gs_dng06_dasptreas_found = 1;

		add_hero_ap_all(30);

	} else if (target_pos == DNG_POS(1,8,14) && target_pos != gs_dng_pos_bak)
	{
		g_fig_escape_position[NORTH] = g_fig_escape_position[WEST] = DNG_POS_DIR(1,6,14,WEST);
		g_fig_escape_position[SOUTH] = g_fig_escape_position[EAST] = DNG_POS_DIR(1,13,13,NORTH);

		if (!do_fight(FIGHT_ID_F094_19))
		{
			gs_dng06_fight19_flag = 1;
		}

	} else if (target_pos == DNG_POS(0,13,15) && target_pos != gs_dng_pos_bak)
	{
		leave_dungeon();

		gs_town_id = gs_travel_destination_town_id;
		gs_x = gs_travel_destination_x;
		gs_y = gs_travel_destination_y;
		gs_town_loc_type = LOCTYPE_NONE;
		gs_viewdir = REVERSE_VIEWDIR(gs_travel_destination_viewdir);

		sprintf(g_dtp2,	get_tx(38), get_ttx(gs_journey_destination_town_id + 0xeb)); /* "IHR VERLASST DIESEN FINSTEREN UNTERSCHLUPF..." / TEXT.LTX 236..287: Ortsname */
		GUI_output(g_dtp2);

		timewarp(HOURS(2));

		g_fading_state = 3;
	}

	g_textbox_width = tw_bak;
	gs_dng_pos_bak = target_pos;

	return 0;
}


/**
 * \brief   print introductions of dungeon #6 fights
 *
 * \param   fight_id    ID of the fight
 */
void DNG06_fight_intro(const signed int fight_id)
{
	if (fight_id == FIGHT_ID_F094_22)
	{
		/* this is true all the time: (!gs_estorik_known), since gs_estorik_known is never set */
		sprintf(g_dtp2,	get_tx(30), (!gs_estorik_known ? get_tx(32) : get_tx(31))); /* "DREI PIRATEN KOMMEN AUF EUCH..." / "SCHEINT DER ANFÜHRER ZU SEIN." / "IST ESTORIK, DER ANFÜHRER DER..." */
		GUI_output(g_dtp2);
	}
}


void DNG06_chest00_loot(struct struct_chest* chest)
{
	uint8_t* ptr_bak = chest->loot.content;

	chest->loot.content = gs_dng06_chest1_content;

	loot_simple_chest(chest);

	chest->loot.content = ptr_bak;
}


/**
 * \brief   chest with books; with a successful Lesen test they are recognized as the loot accounts
 *          of the Daspota pirates (only a message, no item)
 */
void DNG06_chest01_loot(struct struct_chest* chest)
{
	signed int i;
	struct struct_hero *hero;

#if defined(__BORLANDC__)
	sprintf(g_dtp2, get_tx(33)); /* "IN DIESER TRUHE HAT ES NUR EINEN BERG BÜCHER." */
#else
	strcpy(g_dtp2, get_tx(33)); /* "IN DIESER TRUHE HAT ES NUR EINEN BERG BÜCHER." */
#endif

	hero = get_hero(0);
	for (i = 0; i <= 6; i++, hero++)
	{
		if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
			!hero->flags.dead && (test_talent(hero, TA_LESEN, 0) > 0))
		{
			strcat(g_dtp2, get_tx(34)); /* "@ES HANDELT SICH HIERBEI UM AUFZEICHNUNGEN DER BEUTEEINNAHMEN VONPI..." */
			break;
		}
	}

	GUI_output(g_dtp2);
}

/**
 * \brief   handles the pit in Kultstaette des Namenlosen, level 2 (DNG_POS(1,3,12)), square (3,12)
 *
 *          gs_dng09_pit_flag: 0 = nobody in the pit, 1 = one hero, 2 = two heroes in the pit.
 *          Rescue needs two ropes/rope ladders or one plus a mage with staff level >= 3.
 *
 * 	heroes in the pit are separated into a new group.
 * 	This group will have an entry GROUPS_DNG_LEVEL = 2,
 * 	which indicates that it is in the pit.
 */

void DNG09_pitfall(void)
{
	signed int i;
	signed int l3;
	struct struct_hero *hero;
	struct struct_hero *hero_first;
	struct struct_hero *hero_second;

	hero = get_first_hero_available_in_group();

	if (!gs_dng09_pit_flag)
	{
		/* Original-Bug: the loop starts at the first available hero, not at hero 0, but still runs over
		 * 7 slots. If that hero is not at position 0, it reads behind the last hero slot (and may call
		 * test_talent() on it). Should start with get_hero(0). */
#if FIX_0111
		/* Fix 111: bei Platz 0 beginnen */
		hero = get_hero(0);
#endif
		for (i = l3 = 0; i <= 6; i++, hero++)
		{
			if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) &&
#if FIX_0086
				/* Fix 86: nur handlungsfaehige Helden */
				check_hero(hero) &&
#else
				!hero->flags.dead &&
#endif
				test_talent(hero, TA_GEFAHRENSINN, 4) > 0)
				/* TODO: potential Original-Bug: Why should 'petrified' or 'uncouscious' (or maybe other properties ) be o.k. here?? */
			{
				l3 = 1;
			}
		}

		if (l3 != 0)
		{
			GUI_output(get_tx(29)); /* "IHR HABT ES EUREM AUSGEPRÄGTEN GEFAHRENINSTINKT ZU VERDANKEN, DASS..." */

		} else {
			hero_first = get_first_hero_available_in_group();
			hero_second = get_second_hero_available_in_group();

			if (gs_group_member_counts[gs_active_group_id] >= 2)
			{
				/* the current group has at least two heroes */

				/* print message */
				sprintf(g_dtp2, get_tx(30), hero_first->alias, hero_second->alias); /* "%s UND %s STÜRZEN IN EINE FALLGRUBE. DIE ENGEN SCHACHTWÄNDE DÄMPFEN..." */
				GUI_output(g_dtp2);

				/* each of these two heroes looses 3W6+3 LE */
				sub_hero_le(hero_first, dice_roll(3, 6, 3));
				sub_hero_le(hero_second, dice_roll(3, 6, 3));

				/* find an empty group */
				l3 = 0;
				while (gs_group_member_counts[l3] != 0) l3++;

				/* put these heroes in empty group */
				hero_first->group_id = l3;
				hero_second->group_id = l3;
				gs_group_member_counts[l3] += 2;
				gs_group_member_counts[gs_active_group_id] -= 2;

				GRP_save_pos(l3);

				gs_groups_dng_level[l3] = gs_dungeon_level + 1;

				gs_dng09_pit_flag = 2;
			} else {
				/* the current group has only one hero */

				/* print message */
				sprintf(g_dtp2, get_tx(31), hero_first->alias, GUI_get_personal_pronoun(hero_first->sex, GRAMMAR_CASE_1ST)); /* "%s STÜRZT IN EINE FALLGRUBE. DIE ENGEN SCHACHTWÄNDE DÄMPFEN ZWARDEN..." */
				GUI_output(g_dtp2);

				/* this hero looses 3W6+3 LE */
				sub_hero_le(hero_first, dice_roll(3, 6, 3));

				/* find an empty group */
				l3 = 0;
				while (gs_group_member_counts[l3] != 0) l3++;

				/* put this hero in an empty group */
				hero_first->group_id = l3;
				gs_group_member_counts[l3]++;
				gs_group_member_counts[gs_active_group_id]--;

				GRP_save_pos(l3);

				gs_groups_dng_level[l3] = gs_dungeon_level + 1;

				gs_dng09_pit_flag = 1;
			}
		}

	} else if (GUI_bool(get_tx(33))) { /* "VOR EUCH SEHT IHR EINE DUNKLE FALLGRUBE. LEISES WIMMERN DRINGT AN E..." */

		if ((i = DNG_check_climb_tools()) != -1)
		{
			l3 = group_count_item(ITEM_ID_SEIL);
			l3 += group_count_item(ITEM_ID_STRICKLEITER);

			if (l3 >= 2 || (l3 == 1 && i))
			{
				*g_dtp2 = '\0';

				if (i) {
					sprintf(g_dtp2, get_ttx(768), get_hero(i - 1)->alias); /* "%s VERWANDELT DEN ZAUBERSTAB IN EIN SEIL.@" */
				}

				strcat(g_dtp2, (char*)(gs_dng09_pit_flag == 2 ? get_tx(36) : get_tx(37))); /* "IHR LASST DIE SEILE IN DIE GRUBE HINAB, BI..." / "IHR LASST DIE SEILE IN DIE GRUBE HINAB, BI..." */

				GUI_output(g_dtp2);

				for (i = 0; i < 6; i++)	{

					if ((gs_groups_dungeon_id[i] == DUNGEON_ID_KULTSTAETTE_DES_NAMENLOSEN) && (gs_groups_dng_level[i] == 2)) {
						gs_groups_dng_level[i] = 1;
					}
				}

				GRP_merge();

				gs_dng09_pit_flag = 0;

			} else {
				GUI_output(get_tx(35)); /* "FÜR 15 SCHRITT TIEFE IST EIN SEIL LEIDER ZU WENIG. EIN ZWEITES SEIL..." */
			}
		} else {
			GUI_output(get_tx(34)); /* "LEIDER HABT IHR KEIN SEIL DABEI UND SOMIT IST EIN RETTUNGSVERSUCH I..." */
		}
	}
}
