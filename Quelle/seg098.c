/*
 * seg098 - Magie: Zauber auswaehlen, pruefen und sprechen
 *
 * Inhalt:    Zaubermenue (erst Zauberklasse, dann Spruch), Zauberprobe test_spell()
 *            mit MR des Ziels, AE-Kosten, und use_spell(): ruft nach gelungener
 *            Probe den Handler aus g_spell_handlers (seg099-seg101) und zieht die
 *            AE ab. use_magic(): Meditation (LE in AE wandeln) und Stabzauber der
 *            Magier. Dazu Hilfen fuer Kampfzauber (Schaden, Parade des Ziels) und
 *            die Sternchen-Animation auf dem Heldenbild.
 * Aufrufer:  Magie-Menue im Lager/Gasthaus (seg051/052/054: use_magic), Helden-,
 *            Stadt- und Dungeonmenue (seg048/066/076: select_magic_user), Kampf
 *            (seg033/seg042), Steigern (seg050: select_spell mit show_vals 1),
 *            test_spell() auch aus Ereignissen (seg076, seg082 ...) und seg120.
 * Daten:     g_spell_special_aecost: Rueckmeldung des Handlers (-1 Standardkosten,
 *            0 keine Wirkung/keine Kosten, -2 misslungen = halbe Kosten, >0 eigene
 *            Kosten).
 * Besonders: Stabzauber: Probe prueft Eigenschaft 2 doppelt und 3 nie (Original-
 *            Bug 17), die Grenze staff_level == 7 wird nie erreicht. Meditation
 *            bei LE unter 8 kostet LE ohne AE-Gewinn. get_attackee_rs() unbenutzt.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg098.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(__BORLANDC__)
#include <unistd.h>
#endif

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg004.h"
#include "seg007.h"
#include "seg026.h"
#include "seg029.h"
#include "seg041.h"
#include "seg047.h"
#include "seg097.h"
#include "seg098.h"
#include "seg099.h"
#include "seg100.h"
#include "seg101.h"
#include "seg105.h"

signed int g_spell_special_aecost = -1; // ds:0xac0e
static struct int16_t_5 g_ani_heal_picstars = { 0, 1, 2, 1, 0 };	// ds:0xac10
static struct c_str_6 g_spell_select_str_keyval = { "%s~%d" };		// ds:0xac1a
static struct c_str_5 g_spell_select_str_hl = { "\xf2%s\xf0"};	// ds:0xac20
static struct c_str_3 g_spell_select_str_plain = { "%s" };		// ds:0xac25
static struct c_str_8 g_spell_select_str_hl_val = { "\xf2%s~%d\xf0" }; // ds:0xac28
static struct int8_t_12 g_spell_select_class_usable = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }; // ds:0xac30

/**
 * \brief   show some stars on the picture of a healed hero
 *
 * \param   hero        the hero who heals
 *
 * Wird nach jedem gelungenen Zauber auf einen Helden ausserhalb des Kampfes gezeigt
 * (use_spell), nicht nur bei Heilzaubern.
 */
void magic_target_stars_ani(const struct struct_hero *hero)
{
	signed int target_object_id;
	struct int16_t_5 a = g_ani_heal_picstars;
	//signed int a[5] = { 0, 1, 2, 1, 0 };

	struct struct_hero *target;
	signed int handle;
	signed int i;

	/* load SPSTAR.NVF */
	handle = load_archive_file(ARCHIVE_FILE_SPSTAR_NVF);
	read_archive_file(handle, g_buffer8_ptr, 0x400);
	read_archive_file(handle, g_buffer8_ptr + 0x400, 0x400);
	read_archive_file(handle, g_buffer8_ptr + 0x800, 0x400);
	close(handle);

	target_object_id = hero->target_object_id - 1;
	target = get_hero(target_object_id);

	g_pic_copy.v1 = 0;
	g_pic_copy.v2 = 0;
	g_pic_copy.v3 = 31;
	g_pic_copy.v4 = 31;

	for (i = 0; i < 5; i++) {

		/* copy hero picture into buffer */
		g_pic_copy.x1 = 0;
		g_pic_copy.y1 = 0;
		g_pic_copy.x2 = 31;
		g_pic_copy.y2 = 31;
		g_pic_copy.dst = g_renderbuf_ptr;
		g_pic_copy.src = target->pic;
		do_pic_copy(0);

		/* copy stars over it */
		g_pic_copy.src = g_buffer8_ptr + (a.a[i] * 1024);
		do_pic_copy(2);

		/* copy buffer content to screen */
		g_pic_copy.x1 = g_hero_pic_posx[target_object_id];
		g_pic_copy.y1 = 157;
		g_pic_copy.x2 = g_hero_pic_posx[target_object_id] + 31;
		g_pic_copy.y2 = 188;
		g_pic_copy.src = g_renderbuf_ptr;
		g_pic_copy.dst = g_vga_memstart;
		do_pic_copy(3);

		vsync_or_key(10);
	}

	draw_status_line();
}

/**
 * \brief   account physical spell damage in fight
 *
 * \param   le          LE someone looses
 */
void FIG_do_spell_damage(const signed int le)
{

	if (le <= 0)
		return;

	if (get_spelluser()->target_object_id < 10) {
		/* attack hero */

		/* set pointer */
		g_spelltarget = get_hero(get_spelluser()->target_object_id - 1);

		/* ensure the spelluser does not attack himself */
		if (get_spelltarget() != get_spelluser()) {

			/* do the damage */
			sub_hero_le(get_spelltarget(), le);

			/* add a message (red star with le) */
			FIG_add_msg(0x08, le);

			/* set a variable if the hero died */
			if (get_spelltarget()->flags.dead) {
				g_fig_target_dead = 1;
			}
		}

	} else {
		/* attack enemy */

		/* set a pointer to the enemy */
		g_spelltarget_e = &g_enemy_sheets[get_spelluser()->target_object_id - 10];

		/* do the damage */
		FIG_damage_enemy(g_spelltarget_e, le, 0);

		/* add a message (green star with le) */
		FIG_add_msg(0x0b, le);

		/* set a variable if the enemy died */
		if (g_spelltarget_e->flags.dead) {
			g_fig_target_dead = 1;
		}
	}
}

/**
 * \brief   calculates the PA value of one who is attacked
 *
 *          This function is only used by the spell Kraehenruf
 *          (und damit auch von Radau, spell_radau() ruft spell_kraehenruf() auf).
 */
signed int get_attackee_parade(void)
{
	/* check if enemy or hero is attacked */
	if (get_spelluser()->target_object_id < 10) {

		/* attacked a hero */
		g_spelltarget = get_hero(get_spelluser()->target_object_id - 1);

		/* calculate PA  */

		/* PA = PA-Current-Weapon - AT-Modificator - 1/2 * RS-BE */

		return get_spelltarget()->pa_talent_bonus[get_spelltarget()->weapon_type]
			- get_spelltarget()->fight_atpa_mod
			- get_spelltarget()->rs_be / 2;
	} else {

		/* attacked an enemy */

		/* set a global pointer to the target */
		g_spelltarget_e = &g_enemy_sheets[get_spelluser()->target_object_id - 10];

		return g_spelltarget_e->pa;
	}
}

/**
 * \brief   calculates the RS value of one who is attacked
 *
 *          This function is unused.
 */
signed int get_attackee_rs(void)
{
	/* check if enemy or hero is attacked */
	if (get_spelluser()->target_object_id < 10) {

		/* attacked a hero */

		g_spelltarget = get_hero(get_spelluser()->target_object_id - 1);

		return get_spelltarget()->rs_bonus; /* why not also HERO_RS_BONUS2? Anyway, function is unused... */

	} else {

		/* attacked an enemy */

		/* set a global pointer to the target */
		g_spelltarget_e = &g_enemy_sheets[get_spelluser()->target_object_id - 10];

		return g_spelltarget_e->rs;
	}
}

/**
 * \brief   get the amount of AE-Points needed for a spell
 *
 * \param   spell_id	the spell ID
 * \param   half_cost	spellcaster needs only half AE
 * \return  AE-Kosten; -1 = variabel (der Handler setzt sie). Mit half_cost: Haelfte,
 *          mindestens 1; variable Kosten werden dann zu 5..10.
 */
signed int get_spell_cost(const signed int spell_id, const signed int half_cost)
{
	signed char ret = g_spell_descriptions[spell_id].ae_cost;

	if (half_cost != 0) {

		if (ret == -1) {
			ret = random_interval(5, 10);
		} else {
			ret /= 2;
		}

		if (!ret) {
			/* spell cost is at least 1 */
			ret = 1;
		}
	}

	return ret;
}

/**
 * \brief   use magic menu, meditation and staffspell logic
 *
 * \param   hero        pointer to the hero
 * \return              {0, 1, 2}
 */
signed int use_magic(struct struct_hero *hero)
{
	signed int ae;
	signed int retval = 0;
	const signed int answer = GUI_radio(get_ttx(329), 3, get_ttx(311), get_ttx(312), get_ttx(213)); /* "WAS GENAU SOLL VERSUCHT WERDEN?" / "MEDITIEREN" / "STABZAUBER" / "ZAUBER SPRECHEN" */

	if (answer != -1) {

		switch (answer) {

		case 1: {
			/* Meditation */

			signed int thonnys_inv_slot;

			if (hero->typus != HERO_TYPE_MAGIER) {
				/* not a mage, need thonnys */


				if ((thonnys_inv_slot = inv_slot_of_item(hero, ITEM_ID_THONNYSBLUETE)) == -1) {
					GUI_output(get_ttx(790)); /* "LEIDER HABT IHR KEINE THONNYS DABEI, UM DEN VORGANG ZU STARTEN!" */
					return 0;
				}
			} else {
				/* a mage */
				thonnys_inv_slot = -1;
			}

			/* Ask how many AE should be generated */
			ae = GUI_input(get_ttx(333), 2); /* "WIE VIELE LE SOLLEN IN AE UMGEWANDELT WERDEN?" */

			if (ae != -1) {
				retval = 2;

				if (thonnys_inv_slot != -1) {
					/* drop a thonny */
					drop_item(hero, thonnys_inv_slot, 1);
				}

				/* cap the converted AE such that the hero has at most HERO_AE_ORIG in the end. */
				if ((hero->ae_max - hero->ae)  < ae) {
					ae = hero->ae_max - hero->ae;
				}

				/* spend one AE point */
				sub_ae_splash(hero, 1);

#if !defined(__BORLANDC__)
				D1_INFO("%s Meditationsprobe +0 ", hero->alias);
#endif
				if (test_attrib3(hero, ATTRIB_MU, ATTRIB_CH, ATTRIB_KK, 0) > 0) {
					/* Success */

					/* cap the converted AE such that at least 5 LE remain .*/
					/* bei LE < 8 wird ae <= 0: add_hero_ae() gibt dann nichts, sub_hero_le()
					 * zieht aber noch bis zu 2 LE ab (LE 6/7 -> 5) */
					if (hero->le <= ae + 8) {
						ae = hero->le - 8;
					}

#if FIX_0188
					/* Fix 188: ohne umwandelbare LE (ae <= 0) keine LE abziehen */
					if (ae > 0) {
						sub_hero_le(hero, ae + 3);
						add_hero_ae(hero, ae);
					}
#else
					sub_hero_le(hero, ae + 3);
					add_hero_ae(hero, ae);
#endif
				} else {
					/* Failed, print only a message */
					sprintf(g_dtp2, get_ttx(795), hero->alias); /* "LEIDER KANN SICH %s NICHT GENUG KONZENTRIEREN, UM DIE UMWANDLUNG ZU..." */
					GUI_output(g_dtp2);
				}
			}
			break;
		}
		case 2: {
			/* Staffspell */

			if (hero->typus != HERO_TYPE_MAGIER) {
				/* only for mages */
				GUI_output(get_ttx(403)); /* "DAS KÖNNEN AUSSCHLIESSLICH MAGIER!" */
				return 0;
			}

#if FIX_0189
			/* Fix 189: ab dem 5. Stabzauber (Zuschlag 99) gibt es keine mehr */
			if (hero->staff_level >= 4) {
#else
			if (hero->staff_level == 7) {
#endif
				/* Original-Bug: This never happens.
				 * The highest possible staff spell is the 4th one, since the 5th staff spell has handicap 99.
				 * The check should ask for '== 4' instead. */
				GUI_output(get_ttx(335)); /* "ES GIBT KEINE STABZAUBER MEHR!" */
			} else {

				if (g_staffspell_descriptions[hero->staff_level].ae_cost <= hero->ae) {

					/* check AE */

					retval = 1;

#if !defined(__BORLANDC__)
					D1_INFO("%s Probe fuer Stabzauber Nr. %d (%+d)", hero->alias, hero->staff_level + 1,
						       	g_staffspell_descriptions[hero->staff_level].handicap);
#endif
					if (test_attrib3(hero,
							g_staffspell_descriptions[hero->staff_level].attrib1,
							g_staffspell_descriptions[hero->staff_level].attrib2,
#if !FIX_0190
						/* Original-Bug 17: the second attribute is tested twice, the third one is left out */
						/* (berichtigt: vorher hiess es "first ... second"; uebergeben werden attrib1, attrib2, attrib2) */
							g_staffspell_descriptions[hero->staff_level].attrib2,
#else
							/* Fix 190: dritte Eigenschaft statt der zweiten ein zweites Mal */
							g_staffspell_descriptions[hero->staff_level].attrib3,
#endif
							g_staffspell_descriptions[hero->staff_level].handicap) > 0)
					{
						/* Success */

						/* print a message */
						sprintf(g_dtp2,	get_ttx(339), hero->staff_level + 1); /* "GESCHAFFT!@AUF DEM ZAUBERSTAB LIEGT JETZT DER %d.STABZAUBER." */
						GUI_output(g_dtp2);

						sub_ae_splash(hero, g_staffspell_descriptions[hero->staff_level].ae_cost);

						/* REMARK: ae_max or ae */
						hero->ae_max -= g_staffspell_descriptions[hero->staff_level].ae_mod;

						/* Staffspell level +1 */
						hero->staff_level++;

						/* set the timer */
						hero->staffspell_timer = HOURS(12);

						timewarp(HOURS(5));
					} else {
						/* Failed */
						GUI_output(get_ttx(338)); /* "LEIDER MISSLUNGEN!" */

						/* only half of the AE costs */
						sub_ae_splash(hero, g_staffspell_descriptions[hero->staff_level].ae_cost / 2);

						timewarp(HOURS(2));
					}
				} else {
					/* not enough AE */
					GUI_output(get_ttx(337)); /* "DAZU FEHLT LEIDER DIE ASTRALENERGIE!" */
				}
			}

			break;
		}
		case 3: {
			/* Cast Spell */
			use_spell(hero, 1, 0);
			break;
		}
		}
	}

	return retval;
}

/**
 * \brief   check if a spellclass can be used
 *
 * \param   hero        pointer to the hero
 * \param   spellclass_id the ID of the spellclass
 * \return              0 = can't be used, 1 = can be used
 */
signed int can_use_spellclass(const struct struct_hero *hero, const signed int spellclass_id)
{
	signed int i;
	const signed int first_spell = g_spellclasses[spellclass_id].first;

	for (i = 0; g_spellclasses[spellclass_id].length > i; i++) {

		if ((hero->spells[first_spell + i] >= -5) &&
			((g_in_fight && (g_spell_descriptions[first_spell + i].where_to_use == 1)) ||
			(!g_in_fight && (g_spell_descriptions[first_spell + i].where_to_use != 1))))
		{
			return 1;
		}

	}

	return 0;
}

/**
 * \brief   Zaubermenue: erst die Zauberklasse, dann den Spruch waehlen
 *
 * \param   hero        der Zauberer
 * \param   show_vals   0: Zaubern - nicht anwendbare Klassen/Sprueche in Farbe 2 (\xf2);
 *                      1: Steigern - alle Klassen, Sprueche mit Wert;
 *                      2: wie 0, aber mit Werten (bei 0 im Spielmodus "Fortgeschritten")
 * \return  Zauber-ID; -1 oder -2 = abgebrochen bzw. nicht moeglich
 */
signed int select_spell(struct struct_hero *hero, signed int show_vals)
{
	signed int i;
	signed int answer1;
	signed int first_spell;
	signed int retval = -1;
	struct c_str_6 fmt_val = g_spell_select_str_keyval;
	struct c_str_5 fmt_hl = g_spell_select_str_hl;
	struct c_str_3 fmt = g_spell_select_str_plain;
	struct c_str_8 fmt_hl_val = g_spell_select_str_hl_val;
	struct int8_t_12 class_usable = g_spell_select_class_usable;

	if ((show_vals == 0) && (g_game_mode == GAME_MODE_ADVANCED)) {
		show_vals = 2;
	}

	/* only for magic users */
	if (hero->typus < HERO_TYPE_HEXE) {
		GUI_output(get_ttx(330)); /* "DIESE OPTION IST AUSSCHLIESSLICH MAGIEKUNDIGEN VORBEHALTEN!" */
		return -2;
	}

	if (show_vals == 1) {

		strcpy(g_text_output_buf, get_ttx(205)); /* "STEIGERUNGSVERSUCH" */

		if (hero->saved_spell_increases > 1) {
			strcat(g_text_output_buf, get_ttx(393)); /* "E" */
		}

		sprintf(g_dtp2,	get_ttx(204), /* "NOCH %s %d %s ZU VERGEBEN!" */
			(hero->saved_spell_increases > 1) ? get_ttx(305) : get_ttx(304), /* "SIND" / "IST" */
			hero->saved_spell_increases, g_text_output_buf);

		answer1 = GUI_radio(g_dtp2, 12,
					get_ttx(192), get_ttx(193), /* "ANTIMAGIE" / "BEHERRSCHUNG" */
					get_ttx(194), get_ttx(195), /* "DÄMONOLOGIE" / "ELEMENTE" */
					get_ttx(196), get_ttx(197), /* "BEWEGUNG" / "HEILUNG" */
					get_ttx(198), get_ttx(199), /* "HELLSICHT" / "ILLUSION" */
					get_ttx(100), get_ttx(201), /* "KAMPF" / "VERSTÄNDIGUNG" */
					get_ttx(202), get_ttx(203)) - 1; /* "VERWANDLUNG" / "VERÄNDERUNG" */
	} else {

		strcpy(g_dtp2, get_ttx(216)); /* "AUS WELCHER GRUPPE?" */

		for (i = 0; i < 12; i++) {

			g_radio_name_list[i] = g_dtp2 + 50 * (i + 1);

			class_usable.a[i] = (signed char)can_use_spellclass(hero, i);

			sprintf(g_radio_name_list[i], (class_usable.a[i] != 0) ? fmt.a : fmt_hl.a, get_ttx(i + 192)); /* TEXT.LTX 192..203: Zauberklasse */
		}

		answer1 = GUI_radio(g_dtp2, 12,
				g_radio_name_list[0], g_radio_name_list[1],
				g_radio_name_list[2], g_radio_name_list[3],
				g_radio_name_list[4], g_radio_name_list[5],
				g_radio_name_list[6], g_radio_name_list[7],
				g_radio_name_list[8], g_radio_name_list[9],
				g_radio_name_list[10], g_radio_name_list[11]) - 1;
	}

	if (answer1 != -2) {

		if (!class_usable.a[answer1]) {

			/* this cant use any spells of this class */

			sprintf(g_dtp2, get_ttx(559), hero->alias); /* "%s KANN IM MOMENT KEINEN ZAUBER AUS DIESER MAGIEGRUPPE ANWENDEN!" */
			GUI_output(g_dtp2);

			retval = -2;
		} else {

			first_spell = g_spellclasses[answer1].first;

			for (i = 0; i < g_spellclasses[answer1].length; i++) {

				g_radio_name_list[i] = g_dtp2 + 50 * i;

				if (show_vals == 1) {

					sprintf(g_radio_name_list[i], (char*)fmt_val.a,
						get_ttx(first_spell + i + 106), /* TEXT.LTX 107..: Zaubername */
						hero->spells[first_spell + i]);
				} else if (
					((g_in_fight && (g_spell_descriptions[first_spell + i].where_to_use == 1)) ||
					(!g_in_fight && (g_spell_descriptions[first_spell + i].where_to_use != 1))) &&
					(hero->spells[first_spell + i] >= -5))
				{

					if (show_vals == 2) {
						sprintf(g_radio_name_list[i], (char*)fmt_val.a,
							get_ttx(first_spell + i + 106), /* TEXT.LTX 107..: Zaubername */
							hero->spells[first_spell + i]);
					} else {
						sprintf(g_radio_name_list[i], (char*)fmt.a, get_ttx(first_spell + i + 106)); /* TEXT.LTX 107..: Zaubername */
					}
				} else if (show_vals == 2) {

					sprintf(g_radio_name_list[i], (char*)fmt_hl_val.a,
						get_ttx(first_spell + i + 106), /* TEXT.LTX 107..: Zaubername */
						hero->spells[first_spell + i]);
				} else {
					sprintf(g_radio_name_list[i], (char*)fmt_hl.a, get_ttx(first_spell + i + 106)); /* TEXT.LTX 107..: Zaubername */
				}
			}

			retval = GUI_radio(get_ttx(217), g_spellclasses[answer1].length, /* "WELCHEN ZAUBER?" */
					g_radio_name_list[0], g_radio_name_list[1],
					g_radio_name_list[2], g_radio_name_list[3],
					g_radio_name_list[4], g_radio_name_list[5],
					g_radio_name_list[6], g_radio_name_list[7],
					g_radio_name_list[8], g_radio_name_list[9],
					g_radio_name_list[10], g_radio_name_list[11],
					g_radio_name_list[12], g_radio_name_list[13],
					g_radio_name_list[14], g_radio_name_list[15]);

			if (retval != -1) {

				if ((hero->spells[retval + first_spell - 1] < -5) && (show_vals == 0))
				{
					sprintf(g_dtp2,	get_ttx(560), hero->alias); /* "%s IST NICHT IN DER LAGE DIESEN ZAUBER ZU SPRECHEN!" */
					GUI_output(g_dtp2);
					retval = -1;

				} else {
					retval += first_spell -1;
				}
			}
		}

		if (retval > 0) {
			if (!g_in_fight && (g_spell_descriptions[retval].where_to_use == 1) && (show_vals == 0))
			{
				GUI_output(get_ttx(591)); /* "DIESER ZAUBER IST NUR IM KAMPF ANWENDBAR!" */
				retval = -2;

			} else {
				if (g_in_fight && (g_spell_descriptions[retval].where_to_use == -1))
				{
					GUI_output(get_ttx(592)); /* "DIES IST KEIN ZAUBER, DER IM KAMPF ANWENDBAR WÄRE!" */
					retval = -2;
				}
			}
		}
	} else {
		retval = -2;
	}

	return retval;
}

/**
 * \brief   makes a spell test. no AE deduction in this function.
 */
signed int test_spell(struct struct_hero *hero, const signed int spell_id, signed char handicap)
{
	signed int retval;
	struct spell_descr *spell_desc;

	/* check if class is magic user */
	if ((hero->typus < HERO_TYPE_HEXE) || (check_hero(hero) == 0)) {
		return 0;
	}

	/* check if spell talent >= -5 */
	if (hero->spells[spell_id] < -5)
		return 0;

	/* check if hero has enough AE */
	if (get_spell_cost(spell_id, 0) > hero->ae)
		return -99;

	spell_desc = &g_spell_descriptions[spell_id];

	if (spell_desc->fight) {

		if (hero->target_object_id >= 10) {

			handicap += g_enemy_sheets[hero->target_object_id - 10].mr;

			//if (g_enemy_sheets[hero->target_object_id - 10].flags.mushroom) {	/* BAE-TODO: different code */
			if (g_enemy_sheets[hero->target_object_id - 10].flags.mushroom) { /* BAE-TODO: good */ /* C: natural form is identical, C++ needed a (struct enemy_flags) cast */
				return 0;
			}
		} else {
			handicap += get_hero(hero->target_object_id - 1)->mr;
		}
	}

	if ((spell_id >= 1) && (spell_id <= 85)) {

#if !defined(__BORLANDC__)
		D1_INFO("%s Zauberprobe %s %+d (TaW %d)", hero->alias, names_spell[spell_id], handicap, hero->spells[spell_id]);
#endif

#if FIX_PROBENLOG
		/* Probenlog: Zeile Art 5 vor der Dreifachprobe (Wuerfe und Ergebnis in der folgenden Zeile Art 3):
		 * Zauber, Zauberfertigkeit, Zuschlag (mit Magieresistenz des Ziels) */
		{
			signed int w[3];

			w[0] = spell_id;
			w[1] = hero->spells[spell_id];
			w[2] = handicap;
			fix_probenlog(5, hero->alias, 3, w);
		}
#endif
		handicap -= hero->spells[spell_id];

		retval = test_attrib3(hero, spell_desc->attrib1, spell_desc->attrib2, spell_desc->attrib3, handicap);

		if (retval == -99) {
			retval = -1;
		}

		return retval;
	}

	return 0;
}

/**
 * \brief   makes a spell test for all magic users in the current group
 */
signed int test_spell_group(const signed int spell_id, const signed char handicap)
{
	struct struct_hero *hero_i = get_hero(0);
	signed int i;

	for (i = 0; i <= 6; i++, hero_i++) {

		/* Check class is magicuser */
		if ((hero_i->typus >= HERO_TYPE_HEXE) &&
			/* Check class  BOGUS */
			(hero_i->typus != HERO_TYPE_NONE) &&
			/* Check in group */
			(hero_i->group_id == gs_active_group_id) &&
			/* Check if dead */
#if FIX_0086
			/* Fix 86: nur handlungsfaehige Zauberkundige */
			check_hero(hero_i))
#else
			!hero_i->flags.dead)
#endif
			/* Original-Bug: what if petrified, sleeping, unconcious etc. */
		{

			if (test_spell(hero_i, spell_id, handicap) > 0) {
				return 1;
			}
		}
	}

	return 0;
}

signed int select_magic_user(void)
{
	/* select the hero who should cast a spell */
	const signed int answer = select_hero_ok(get_ttx(317)); /* "WELCHER HELD SOLL MAGIE ANWENDEN?" */

	if (answer != -1) {
		/* valid answer => cast spell */
		return use_spell(get_hero(answer), 1, 0);
	}

	/* abort (berichtigt: hier wird keine Meldung ausgegeben) */
	return -2;
}

/**
 * \brief   casts a spell
 *
 * \param   hero        	the hero who casts the spell
 * \param   selection_menu	1: select spell from menu / 0: spell is preselected in HERO_SPELL_ID
 * \param   handicap		modifier for the spell cast
 */
signed int use_spell(struct struct_hero* hero, const signed int selection_menu, signed char handicap)
{
	signed int retval = 1;
	signed int spell_id;
	signed int ae_cost;
	signed int tw_bak;
	struct spell_descr *spell_description;
	void (*func)(void);
	signed int tx_file_bak;
#if FIX_0191
	/* Original-Bug 29: see below */
	signed int x;
	signed int y;
	signed int pos;
	struct dungeon_door *ptr_doors;
#endif

	if (!check_hero(hero) && !hero->flags.renegade) {

		return 0;
	}

	tw_bak = g_textbox_width;
	g_textbox_width = 3;

	if (selection_menu == 1) {

		spell_id = select_spell(hero, 0);

		if (spell_id > SP_NONE) {

			/* pointer to the spell description */
			spell_description = &g_spell_descriptions[spell_id];

			/* reset the spelltarget of the hero */
			hero->target_object_id = 0;

			if (spell_description->target_type && (spell_description->target_type != 4)) {

				if (spell_description->target_type == 1) {
					/* Ziel ist ein Gegner. Der Zauber laeuft danach trotzdem weiter, scheitert
					 * aber an where_to_use == 1 (alle Sprueche mit target_type 1 sind reine
					 * Kampfzauber): es erscheinen zwei Meldungen, ohne Folgen. */

					GUI_output(get_ttx(234)); /* "DIESER SPRUCH KANN JETZT UND HIER NICHT SINNVOLL EINGESETZT WERDEN!" */

				} else {

					hero->target_object_id = select_hero_from_group(get_ttx(47)) + 1; /* "AUF WEN SOLL DER ZAUBER GESPROCHEN WERDEN?" */

					if (hero->target_object_id <= 0) {
						spell_id = -1;
					}
				}
			}
		}

	} else {
		spell_id = hero->spell_id;
	}

	if (spell_id > 0) {

		/* pointer to the spell description */
		spell_description = &g_spell_descriptions[spell_id];

		if (!g_in_fight && (spell_description->where_to_use == 1)) {

			/* only in fight spell */
			GUI_output(get_ttx(591)); /* "DIESER ZAUBER IST NUR IM KAMPF ANWENDBAR!" */
			retval = 0;

		} else if (g_in_fight && (spell_description->where_to_use == -1)) {

			/* spell not usable in fight (Kommentar berichtigt, vorher "only in fight spell") */
			GUI_output(get_ttx(592)); /* "DIES IST KEIN ZAUBER, DER IM KAMPF ANWENDBAR WÄRE!" */
			retval = 0;
		}

		if (retval) {

#if FIX_0191
			/* Fix 191: Zuschlag der Tuer vor der Gruppe auch beim freien Foramen (Suche endet am Listenende -1) */
			/* Original-Bug 29: door-specific spell handicap is not considered in a free Foramen spell (from the spellcast menu). */
			if (spell_id == SP_FORAMEN_FORAMINOR && gs_dungeon_id != DUNGEON_ID_NONE && (g_dng_extra_action == DNG_MENU_MODE_OPEN_DOOR || g_dng_extra_action == DNG_MENU_MODE_UNLOCK_DOOR)) {
				x = gs_x;
				y = gs_y;
				ptr_doors = g_dungeon_doors_buf;

				switch (gs_viewdir)
				{
					case NORTH: y--; break;
					case EAST:  x++; break;
					case SOUTH: y++; break;
					case WEST:  x--; break;
				}

				pos = DNG_POS(gs_dungeon_level, x, y);

				if ((*(g_dng_map_ptr + MAP_POS(x,y)) & 0x02) == 0) {

					/* flag 1 'unlocked' is not set -> door is locked  */
					while ((ptr_doors->pos != pos) && (ptr_doors->pos != -1)) {
						ptr_doors++;
					}
					if (ptr_doors->pos == pos) {
						handicap += ptr_doors->foramen_handicap;
					}
				}
			}
#endif

			g_spelltest_result = test_spell(hero, spell_id, handicap);

			if (g_spelltest_result == -99) {

				/* prepare output */
				sprintf(g_dtp2, get_ttx(607), hero->alias); /* "%s HAT NICHT GENÜGEND ASTRALENERGIE FÜR DIESEN ZAUBER." */

				if (!g_in_fight) {
					GUI_output(g_dtp2);
				}

				retval = -1;

			} else if ((g_spelltest_result <= 0) || (gs_ingame_timers[INGAME_TIMER_RONDRA_NO_SPELLS] > 0)) {

				strcpy(g_dtp2, get_ttx(606)); /* "ZAUBER LEIDER MISSLUNGEN!" */

				sub_ae_splash(hero, get_spell_cost(spell_id, 1)); /* spell failed -> half AE cost */

				if (!g_in_fight) {
					GUI_output(g_dtp2);
				}

				retval = 0;
			} else {
				/* set global spelluser variable */
				g_spelluser = hero;

				ae_cost = get_spell_cost(spell_id, 0); /* spell successful -> full AE cost */
				g_spell_special_aecost = -1;

				*g_dtp2 = '\0';

				tx_file_bak = g_tx_file_index;

				load_tx(ARCHIVE_FILE_SPELLTXT_LTX);

				func = g_spell_handlers[spell_id];
				func();

				/* 222 = ARCHIVE_FILE_SPELLTXT_LTX: war schon geladen */
				if ((tx_file_bak != -1) && (tx_file_bak != 222)) {
					load_tx(tx_file_bak);
				}

				retval = 1;

				if (g_spell_special_aecost == 0) {

					retval = -1;

					if (!(*g_dtp2)) {
						strcpy(g_dtp2, get_ttx(606)); /* "ZAUBER LEIDER MISSLUNGEN!" */
					}

				} else if (g_spell_special_aecost == -2) {

					strcpy(g_dtp2, get_ttx(606)); /* "ZAUBER LEIDER MISSLUNGEN!" */
					sub_ae_splash(hero, get_spell_cost(spell_id, 1));
					retval = 0;

				} else if (g_spell_special_aecost != -1) {
					sub_ae_splash(hero, g_spell_special_aecost);
				} else {
					sub_ae_splash(hero, ae_cost);
				}

				if (!g_in_fight) {

					GUI_output(g_dtp2);

					if (retval > 0) {
						play_voc(ARCHIVE_FILE_FX17_VOC);

						if ((hero->target_object_id < 10) && (hero->target_object_id > 0) && (g_pp20_index == ARCHIVE_FILE_PLAYM_UK))
						{
							magic_target_stars_ani(hero);
						}
					}
				}
			}
		} else {
			retval = 0;
		}
	} else {
		retval = -1;
	}

	g_textbox_width = tw_bak;

	return retval;
}
