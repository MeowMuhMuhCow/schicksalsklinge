/*
 * seg101 - Zauber-Handler 3/3: Verwandlung und Veraenderung
 *
 * Inhalt:    spell_*()-Handler fuer Arcano Psychostabilis, Armatrutz, die sieben
 *            Eigenschafts-Zauber (+2 auf CH/FF/GE/IN/KK/KL/MU fuer 2 Stunden, nicht
 *            auf sich selbst), Feuerbann, Paralue, Salander, Visibili, Brenne toter
 *            Stoff, Dunkelheit, Flim Flam und Silentium; viele Sprueche sind leer.
 * Aufrufer:  use_spell() (seg098) ueber g_spell_handlers; spell_brenne() auch beim
 *            Anzuenden von Fackel oder Laterne mit dem Zunderkaestchen (seg107,
 *            g_ignite_mode).
 * Daten:     keine eigenen.
 * Besonders: Armatrutz erlaubt einen Punkt mehr, als die AE hergeben (BE markiert).
 *            spell_paralue() prueft beim Heldenziel zuerst den alten Wert von
 *            g_spelltarget. Brenne entzuendet immer die erste Fackel (Bug 55).
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg101.c
 */

#include <stdio.h>
#include <string.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg007.h"
#include "seg096.h"
#include "seg097.h"
#include "seg105.h"

/* Transformation / Verwandlung */
void spell_adler(void)
{
	/* triggers the "spell failed" messages */
	g_spell_special_aecost = -2;

#if !defined(__BORLANDC__)
	D1_INFO("Zauberspruch \"Adler, Wolf und Hammerhai\" ist nicht implementiert\n");
#endif
}

void spell_arcano(void)
{
	signed int target_hero_pos;
	signed int mod_slot;

	/* get the spell target */
	target_hero_pos = get_spelluser()->target_object_id - 1;

	g_spelltarget = get_hero(target_hero_pos);

	/* get a free mod_slot */
	mod_slot = get_free_mod_slot();

	/* MR + 2 for 1 h */
	set_mod_slot(mod_slot, HOURS(1), (uint8_t*)&get_spelltarget()->mr, 2, target_hero_pos);

	/* "Die Magieresistenz von %s steigt um 2 Punkte." */
	sprintf(g_dtp2, get_tx(98), get_spelltarget()->alias); /* "DIE MAGIERESITENZ VON %s STEIGT UM 2 PUNKTE." */
}

void spell_armatrutz(void)
{
	signed int max_bonus = 0;
	signed int hero_pos;
	signed int bonus;
	signed int mod_slot;

	/* calc the maximal RS bonus */
#if !FIX_0207
	/* Original-Bug: you can get one RS point more that you have AE for */
	while (max_bonus * max_bonus < get_spelluser()->ae) {
		max_bonus++;
	}
#else
	/* Fix 207: groesster Bonus, dessen Quadrat die AE nicht uebersteigt */
	while (max_bonus * max_bonus <= get_spelluser()->ae) {
		max_bonus++;
	}
	--max_bonus;
#endif

	/* the maximum bonus is 9 */
	if (max_bonus > 9)
		max_bonus = 9;

	/* ask the user which bonus he wants */
	sprintf(g_dtp2,	get_tx(99), max_bonus); /* "UM WIE VIELE PUNKTE SOLL DER RÜSTUNGSSCHUTZ ERHÖHT WERDEN (MAX %d)?" */
	bonus = GUI_input(g_dtp2, 1);

	/* fix wrong input */
	if (bonus > max_bonus)
		bonus = max_bonus;

	if (bonus != -1) {

		hero_pos = get_hero_pos(get_spelluser());
		g_spell_special_aecost = bonus * bonus;
		mod_slot = get_free_mod_slot();
		set_mod_slot(mod_slot, MINUTES(5), (uint8_t*)&get_spelluser()->rs_bonus, (signed char)bonus, hero_pos);

		/* prepare output message */
		sprintf(g_dtp2,	get_tx(100), get_spelluser()->alias, bonus); /* "%sS RÜSTUNGSSCHUTZ STEIGT UM %d." */

	} else {
		/* spell canceled */
		/* set AE to 0 */
		g_spell_special_aecost = 0;
		/* avoid the textbox output */
		*g_dtp2 = '\0';
	}
}

void spell_inc_ch(void)
{
	signed int target_hero_pos;
	signed int mod_slot;

	/* get the spell target */
	target_hero_pos = get_spelluser()->target_object_id - 1;

	g_spelltarget = get_hero(target_hero_pos);

	/* check if the target is the spelluser */
	if (get_spelltarget() == get_spelluser()) {

		/* set AE costs to 0 */
		g_spell_special_aecost = 0;

		/* copy message text */
		strcpy(g_dtp2, get_tx(112)); /* "DIESER SPRUCH IST NICHT AUF SICH SELBST ANWENDBAR!" */

		return;
	}

	/* check if CH was already increased */
	if (get_spelltarget()->attrib[ATTRIB_CH].current > get_spelltarget()->attrib[ATTRIB_CH].normal) {
		/* "Bei %s ist %s schon magisch gesteigert" */
		sprintf(g_dtp2,	get_tx(113), get_spelltarget()->alias, get_ttx(414)); /* "BEI %s IST %s SCHON MAGISCH ERHÖHT UND KAN..." / "CHARISMA" */
	} else {
		/* get a free mod_slot */
		mod_slot = get_free_mod_slot();

		/* CH + 2 for 2 hours */
		set_mod_slot(mod_slot, HOURS(2), (uint8_t*)&get_spelltarget()->attrib[ATTRIB_CH].current, 2, target_hero_pos);

		/* "Bei %s steigt %s um 2 Punkte" */
		sprintf(g_dtp2, get_tx(101), get_spelltarget()->alias, get_ttx(414)); /* "BEI %s STEIGT %s UM 2 PUNKTE!" / "CHARISMA" */
	}
}

void spell_feuerbann(void)
{
	signed int hero_pos;
	signed int mod_slot;

	/* check if spell is already activated */
	if (!get_spelluser()->fireban) {

		hero_pos = get_hero_pos(get_spelluser());

		mod_slot = get_free_mod_slot();

		/* Duration = Level * 5 min (berichtigt, vorher "12 min"; MINUTES(5)) */
		set_mod_slot(mod_slot, get_spelluser()->level * MINUTES(5), (uint8_t*)&get_spelluser()->fireban, 1, hero_pos);

		/* prepare message */
		sprintf(g_dtp2,	get_tx(102), get_spelluser()->alias); /* "%s NIMMT DURCH MAGISCHES FEUER KEINEN SCHADEN MEHR!" */
	} else {
		/* set AE costs to 0 */
		g_spell_special_aecost = 0;
	}
}

void spell_inc_ff(void)
{
	signed int target_hero_pos;
	signed int mod_slot;

	/* get the spell target */
	target_hero_pos = get_spelluser()->target_object_id - 1;

	g_spelltarget = get_hero(target_hero_pos);

	/* check if the target is the spelluser */
	if (get_spelltarget() == get_spelluser()) {

		/* set AE costs to 0 */
		g_spell_special_aecost = 0;

		/* copy message text */
		strcpy(g_dtp2, get_tx(112)); /* "DIESER SPRUCH IST NICHT AUF SICH SELBST ANWENDBAR!" */

		return;
	}

	/* check if FF was already increased */
	if (get_spelltarget()->attrib[ATTRIB_FF].current > get_spelltarget()->attrib[ATTRIB_FF].normal) {

		/* "Bei %s ist %s schon magisch gesteigert" */
		sprintf(g_dtp2,	get_tx(113), get_spelltarget()->alias, get_ttx(415)); /* "BEI %s IST %s SCHON MAGISCH ERHÖHT UND KAN..." / "FINGERFERTIGKEIT" */
	} else {
		/* get a free mod_slot */
		mod_slot = get_free_mod_slot();

		/* FF + 2 for 2 hours */
		set_mod_slot(mod_slot, HOURS(2), (uint8_t*)&get_spelltarget()->attrib[ATTRIB_FF].current, 2, target_hero_pos);

		/* "Bei %s steigt %s um 2 Punkte" */
		sprintf(g_dtp2,	get_tx(101), get_spelltarget()->alias, get_ttx(415)); /* "BEI %s STEIGT %s UM 2 PUNKTE!" / "FINGERFERTIGKEIT" */
	}
}

void spell_inc_ge(void)
{
	signed int target_hero_pos;
	signed int mod_slot;

	/* get the spell target */
	target_hero_pos = get_spelluser()->target_object_id - 1;

	g_spelltarget = get_hero(target_hero_pos);

	/* check if the target is the spelluser */
	if (get_spelltarget() == get_spelluser()) {

		/* set AE costs to 0 */
		g_spell_special_aecost = 0;

		/* copy message text */
		strcpy(g_dtp2, get_tx(112)); /* "DIESER SPRUCH IST NICHT AUF SICH SELBST ANWENDBAR!" */

		return;
	}

	/* check if GE was already increased */
	if (get_spelltarget()->attrib[ATTRIB_GE].current > get_spelltarget()->attrib[ATTRIB_GE].normal) {
		/* "Bei %s ist %s schon magisch gesteigert" */
		sprintf(g_dtp2, get_tx(113), get_spelltarget()->alias, get_ttx(416)); /* "BEI %s IST %s SCHON MAGISCH ERHÖHT UND KAN..." / "GEWANDTHEIT" */
	} else {
		/* get a free mod_slot */
		mod_slot = get_free_mod_slot();

		/* GE + 2 for 2 hours */
		set_mod_slot(mod_slot, HOURS(2), (uint8_t*)&get_spelltarget()->attrib[ATTRIB_GE].current, 2, target_hero_pos);

		/* "Bei %s steigt %s um 2 Punkte" */
		sprintf(g_dtp2,	get_tx(101), get_spelltarget()->alias, get_ttx(416)); /* "BEI %s STEIGT %s UM 2 PUNKTE!" / "GEWANDTHEIT" */
	}
}

void spell_inc_in(void)
{
	signed int target_hero_pos;
	signed int mod_slot;

	/* get the spell target */
	target_hero_pos = get_spelluser()->target_object_id - 1;

	g_spelltarget = get_hero(target_hero_pos);

	/* check if the target is the spelluser */
	if (get_spelltarget() == get_spelluser()) {

		/* set AE costs to 0 */
		g_spell_special_aecost = 0;

		/* copy message text */
		strcpy(g_dtp2, get_tx(112)); /* "DIESER SPRUCH IST NICHT AUF SICH SELBST ANWENDBAR!" */

		return;
	}

	/* check if IN was already increased */
	if (get_spelltarget()->attrib[ATTRIB_IN].current > get_spelltarget()->attrib[ATTRIB_IN].normal) {
		/* "Bei %s ist %s schon magisch gesteigert" */
		sprintf(g_dtp2, get_tx(113), get_spelltarget()->alias, get_ttx(417)); /* "BEI %s IST %s SCHON MAGISCH ERHÖHT UND KAN..." / "INTUITION" */
	} else {
		/* get a free mod_slot */
		mod_slot = get_free_mod_slot();

		/* IN + 2 for 2 hours */
		set_mod_slot(mod_slot, HOURS(2), (uint8_t*)&get_spelltarget()->attrib[ATTRIB_IN].current, 2, target_hero_pos);

		/* "Bei %s steigt %s um 2 Punkte" */
		sprintf(g_dtp2,	get_tx(101), get_spelltarget()->alias, get_ttx(417)); /* "BEI %s STEIGT %s UM 2 PUNKTE!" / "INTUITION" */
	}
}

void spell_inc_kk(void)
{
	signed int target_hero_pos;
	signed int mod_slot;

	/* get the spell target */
	target_hero_pos = get_spelluser()->target_object_id - 1;

	g_spelltarget = get_hero(target_hero_pos);

	/* check if the target is the spelluser */
	if (get_spelltarget() == get_spelluser()) {

		/* set AE costs to 0 */
		g_spell_special_aecost = 0;

		/* copy message text */
		strcpy(g_dtp2, get_tx(112)); /* "DIESER SPRUCH IST NICHT AUF SICH SELBST ANWENDBAR!" */

		return;
	}

	/* check if KK was already increased */
	if (get_spelltarget()->attrib[ATTRIB_KK].current > get_spelltarget()->attrib[ATTRIB_KK].normal) {
		/* "Bei %s ist %s schon magisch gesteigert" */
		sprintf(g_dtp2,	get_tx(113), get_spelltarget()->alias, get_ttx(418)); /* "BEI %s IST %s SCHON MAGISCH ERHÖHT UND KAN..." / "KÖRPERKRAFT" */
	} else {
		/* get a free mod_slot */
		mod_slot = get_free_mod_slot();

		/* KK + 2 for 2 hours (berichtigt, vorher "IN") */
		set_mod_slot(mod_slot, HOURS(2), (uint8_t*)&get_spelltarget()->attrib[ATTRIB_KK].current, 2, target_hero_pos);

		/* "Bei %s steigt %s um 2 Punkte" */
		sprintf(g_dtp2,	get_tx(101), get_spelltarget()->alias, get_ttx(418)); /* "BEI %s STEIGT %s UM 2 PUNKTE!" / "KÖRPERKRAFT" */
	}
}

void spell_inc_kl(void)
{
	signed int target_hero_pos;
	signed int mod_slot;

	/* get the spell target */
	target_hero_pos = get_spelluser()->target_object_id - 1;

	g_spelltarget = get_hero(target_hero_pos);

	/* check if the target is the spelluser */
	if (get_spelltarget() == get_spelluser()) {

		/* set AE costs to 0 */
		g_spell_special_aecost = 0;

		/* copy message text */
		strcpy(g_dtp2, get_tx(112)); /* "DIESER SPRUCH IST NICHT AUF SICH SELBST ANWENDBAR!" */

		return;
	}

	/* check if KL was already increased */
	if (get_spelltarget()->attrib[ATTRIB_KL].current > get_spelltarget()->attrib[ATTRIB_KL].normal) {
		/* "Bei %s ist %s schon magisch gesteigert" */
		sprintf(g_dtp2,	get_tx(113), get_spelltarget()->alias, get_ttx(413)); /* "BEI %s IST %s SCHON MAGISCH ERHÖHT UND KAN..." / "KLUGHEIT" */
	} else {
		/* get a free mod_slot */
		mod_slot = get_free_mod_slot();

		/* KL + 2 for 2 hours */
		set_mod_slot(mod_slot, HOURS(2), (uint8_t*)&get_spelltarget()->attrib[ATTRIB_KL].current, 2, target_hero_pos);

		/* "Bei %s steigt %s um 2 Punkte" */
		sprintf(g_dtp2, get_tx(101), get_spelltarget()->alias, get_ttx(413)); /* "BEI %s STEIGT %s UM 2 PUNKTE!" / "KLUGHEIT" */
	}
}

void spell_inc_mu(void)
{
	signed int target_hero_pos;
	signed int mod_slot;

	/* get the spell target */
	target_hero_pos = get_spelluser()->target_object_id - 1;

	g_spelltarget = get_hero(target_hero_pos);

	/* check if the target is the spelluser */
	if (get_spelltarget() == get_spelluser()) {

		/* set AE costs to 0 */
		g_spell_special_aecost = 0;

		/* copy message text */
		strcpy(g_dtp2, get_tx(112)); /* "DIESER SPRUCH IST NICHT AUF SICH SELBST ANWENDBAR!" */

		return;
	}

	/* check if MU was already increased */
	if (get_spelltarget()->attrib[ATTRIB_MU].current > get_spelltarget()->attrib[ATTRIB_MU].normal) {
		/* "Bei %s ist %s schon magisch gesteigert" */
		sprintf(g_dtp2,	get_tx(113), get_spelltarget()->alias, get_ttx(412)); /* "BEI %s IST %s SCHON MAGISCH ERHÖHT UND KAN..." / "MUT" */
	} else {
		/* get a free mod_slot */
		mod_slot = get_free_mod_slot();

		/* MU + 2 for 2 hours */
		set_mod_slot(mod_slot, HOURS(2), (uint8_t*)&get_spelltarget()->attrib[ATTRIB_MU].current, 2, target_hero_pos);

		/* "Bei %s steigt %s um 2 Punkte" */
		sprintf(g_dtp2, get_tx(101), get_spelltarget()->alias, get_ttx(412)); /* "BEI %s STEIGT %s UM 2 PUNKTE!" / "MUT" */
	}
}

void spell_mutabili(void)
{
	/* triggers the "spell failed" messages */
	g_spell_special_aecost = -2;
#if !defined(__BORLANDC__)
	D1_INFO("Zauberspruch \"Mutabili\" ist nicht implementiert\n");
#endif
}

void spell_paralue(void)
{
	if (get_spelluser()->target_object_id >= 10) {
		/* cast at an enemy */

		/* BC-TODO: calculation of ptr could be better */
		g_spelltarget_e = &g_enemy_sheets[get_spelluser()->target_object_id - 10];

		/* set 'petrified' flag */
		g_spelltarget_e->flags.petrified = 1;

		sprintf(g_dtp2,	get_tx(103), (char*)GUI_name_inflect_with_article( /* "%s IST VERSTEINERT!" */
			INFLECT_DEFINITE_ARTICLE | INFLECT_SINGULAR | INFLECT_1ST_CASE,
			g_spelltarget_e->monster_id,
			INFLECT_NAME_TYPE_MONSTER
		));
	} else {
		/* cast a hero */
		/* TODO: the first check can be removed, cause it would not give a message */
		/* Achtung: geprueft wird hier der ALTE Wert von g_spelltarget (vom letzten Zauber),
		 * er wird erst danach gesetzt. War das alte Ziel der Zauberer selbst, scheitert
		 * Paralue still mit 0 AE, egal auf wen er jetzt gesprochen wird. */
#if FIX_0208
		/* Fix 208: erst das neue Ziel setzen, dann pruefen */
		g_spelltarget = get_hero(get_spelluser()->target_object_id - 1);
#endif
		if (get_spelltarget() != get_spelluser()) {

			/* set the target  */
			g_spelltarget = get_hero(get_spelluser()->target_object_id - 1);

			/* check again */
			if (get_spelltarget() == get_spelluser()) {

				/* never cast yourself */
				g_spell_special_aecost = 0;

				strcpy(g_dtp2, get_tx(112)); /* "DIESER SPRUCH IST NICHT AUF SICH SELBST ANWENDBAR!" */
			} else {
				get_spelltarget()->flags.petrified = 1;

				/* prepare message */
				sprintf(g_dtp2, get_tx(103), get_spelltarget()->alias); /* "%s IST VERSTEINERT!" */
			}
		} else {
			/* set AE to 0 */
			g_spell_special_aecost = 0;
		}
	}
}

/**
 * \brief   hero spell 'Salander Mutanderer'
 *
 * Turns the target into a mushroom
 */
void spell_salander(void)
{
	/* TODO: Original-Bug: Strange effect when cast on a double-size enemy */
	signed int ae_cost;

	g_spelltarget_e = &g_enemy_sheets[get_spelluser()->target_object_id - 10];

	/* AE-Kosten: MR des Ziels * 3 */
	ae_cost = g_spelltarget_e->mr * 3;

	/* set the minimal astral cost to 25 AE */
	if (ae_cost < 25)
		ae_cost = 25;

	if (get_spelluser()->ae >= ae_cost) {

		/* sets 'mushroom' flag */
		g_spelltarget_e->flags.mushroom = 1;

		/* prepare message */
		sprintf(g_dtp2, get_tx(104), (char*)GUI_name_inflect_with_article( /* "%s WURDE SOEBEN IN EINEN FLIEGENPILZ VERWANDELT!" */
			INFLECT_DEFINITE_ARTICLE | INFLECT_SINGULAR | INFLECT_1ST_CASE,
			g_spelltarget_e->monster_id,
			INFLECT_NAME_TYPE_MONSTER
		));

		/* set AE cost */
		g_spell_special_aecost = ae_cost;
	} else {
		/* prepare message */
		sprintf(g_dtp2, get_ttx(607), get_spelluser()->alias); /* "%s HAT NICHT GENÜGEND ASTRALENERGIE FÜR DIESEN ZAUBER." */

		/* no AE cost */
		g_spell_special_aecost = 0;
	}
}

void spell_see(void)
{
#if !defined(__BORLANDC__)
	D1_INFO("Zauberspruch \"See und Fluss\" ist nicht implementiert\n");
#endif
}

/**
 * \brief   hero spell 'Visibili Vanitar'
 *
 * Makes the spell caster invisible for a certain amount of time. This allows him to skip fights in dungeons.
 */
void spell_visibili(void)
{
	signed int mod_slot;
	signed int hero_pos;
	signed int rounds;

	/* ask the user how many rounds he wants to be invisible */
	sprintf(g_dtp2, get_tx(105), get_spelluser()->alias); /* "WIE LANGE (SPIELRUNDEN) SOLL %s UNSICHTBAR WERDEN?" */
	rounds = GUI_input(g_dtp2, 2);

	/* the spell has also no effect if it is already active */
	if ((rounds <= 0) || (get_spelluser()->invisible != 0)) {

		/* set AE to 0 */
		g_spell_special_aecost = 0;
		/* clear output string */
		*g_dtp2 = '\0';

		return;
	}

	/* check if the hero has enough AE */
	if (rounds * 5 <= get_spelluser()->ae) {

		g_spell_special_aecost = rounds * 5;
		hero_pos = get_hero_pos(get_spelluser());
		mod_slot = get_free_mod_slot();
		set_mod_slot(mod_slot, (int32_t)rounds * MINUTES(5), (uint8_t*)&get_spelluser()->invisible, 1, hero_pos);

		sprintf(g_dtp2,	get_tx(106), get_spelluser()->alias, GUI_get_personal_pronoun(get_spelluser()->sex, GRAMMAR_CASE_1ST)); /* "%s WIRD UNSICHTBAR, D.H. %s UMGEHT ALLE MÖGLICHEN KÄMPFE!" */
	} else {
		sprintf(g_dtp2,	get_ttx(607), get_spelluser()->alias); /* "%s HAT NICHT GENÜGEND ASTRALENERGIE FÜR DIESEN ZAUBER." */

		g_spell_special_aecost = 0;
	}

}

/* Transmutation / Veraenderung */

void spell_abvenenum(void)
{
#if !defined(__BORLANDC__)
	D1_INFO("Zauberspruch \"Abvenenum\" ist nicht implementiert\n");
#endif
}

void spell_aeolitus(void)
{
#if !defined(__BORLANDC__)
	D1_INFO("Zauberspruch \"Aeolitus\" ist nicht implementiert\n");
#endif
}

/**
 * \brief   hero spell 'Brenne, toter Stoff!'
 *
 * Ignites a lantern (uses up 1 oil) or a torch in the inventory.
 * Used for hero spell, but also for manual igniting based on a tinderbox.
 */
void spell_brenne(void)
{
	signed int oil_inv_slot;
	signed int answer;

	signed int torch_inv_slot = -1;
	signed int lantern_inv_slot = -1;

	g_spell_special_aecost = 0;

#if !FIX_0210
	/* Original-Bug 55:
	 * Using the second (third, firth...) un-lit torch in the inventory to ignite it,
	 * actually the first torch will be ignited. (provided the hero also has a tinder box)
	 * The same for lanterns instead of torches, provided the hero also has a tinder
	 * box and a bottle of oil.
	 */
	if (g_ignite_mode == IGNITE_MODE_USE_TORCH) {
		torch_inv_slot = inv_slot_of_item(get_spelluser(), ITEM_ID_FACKEL__UNLIT);
	} else {
		if (g_ignite_mode == IGNITE_MODE_USE_LANTERN) {
		} else {
			// assert(g_ignite_mode == IGNITE_MODE_SPELL_OR_USE_TINDER);
			torch_inv_slot = inv_slot_of_item(get_spelluser(), ITEM_ID_FACKEL__UNLIT);
		}

		lantern_inv_slot = inv_slot_of_item(get_spelluser(), ITEM_ID_LATERNE__UNLIT);
	}
#else
	/* Fix 210: den benutzten Inventarplatz anzuenden, nicht die erste Fackel/Laterne */
	/* For the fix, we rely on the stored inventory slot of the used item in g_used_item_inv_slot */
	if (g_ignite_mode == IGNITE_MODE_USE_TORCH) {
		torch_inv_slot = g_used_item_inv_slot;
	} else if (g_ignite_mode == IGNITE_MODE_USE_LANTERN) {
		lantern_inv_slot = g_used_item_inv_slot;
	} else {
		// assert(g_ignite_mode == IGNITE_MODE_SPELL_OR_USE_TINDER);
		torch_inv_slot = inv_slot_of_item(get_spelluser(), ITEM_ID_FACKEL__UNLIT);
		lantern_inv_slot = inv_slot_of_item(get_spelluser(), ITEM_ID_LATERNE__UNLIT);
	}
#endif

	if (torch_inv_slot != -1 && lantern_inv_slot != -1) {

		/* both torch and lantern are available, must decide */
		sprintf(g_dtp2,	get_tx(107), get_spelluser()->alias); /* "WAS SOLL %s ENTZÜNDEN?" */

		/* 0x4000 = INFLECT_OMIT_ARTICLE, 0 = INFLECT_NAME_TYPE_ITEM */
		answer = GUI_radio(g_dtp2, 2,
				(char*)GUI_name_inflect_with_article(0x4000, ITEM_ID_FACKEL__UNLIT, 0),
				(char*)GUI_name_inflect_with_article(0x4000, ITEM_ID_LATERNE__UNLIT, 0));

		if (answer == -1) {

			/* abort */
			torch_inv_slot = lantern_inv_slot = -1;

		} else if (answer == 1) {

			lantern_inv_slot = -1;

		} else {

			torch_inv_slot = -1;
		}
	}

	if (torch_inv_slot != -1) {

		/* change torch to burning torch */
		get_spelluser()->inventory[torch_inv_slot].item_id = ITEM_ID_FACKEL__LIT;

		/* set timer to 10 */
		get_spelluser()->inventory[torch_inv_slot].lighting_timer = 10;

		/* set AE cost */
		g_spell_special_aecost = random_schick(20);

		sprintf(g_dtp2,	get_tx(108), get_spelluser()->alias); /* "%s ENTZÜNDET DIE FACKEL." */
		/* "torch is lit *" */

	} else if (lantern_inv_slot != -1) {

		/* get position of oil */
		oil_inv_slot = inv_slot_of_item(get_spelluser(), ITEM_ID_OEL);

		if (oil_inv_slot != -1) {

			/* change lantern to burning lantern */
			get_spelluser()->inventory[lantern_inv_slot].item_id = ITEM_ID_LATERNE__LIT;

			/* set counter to 100 */
			get_spelluser()->inventory[lantern_inv_slot].lighting_timer = 100;

			/* drop one oil flask */
			drop_item(get_spelluser(), oil_inv_slot, 1);

			/* give bronze flask */
			give_new_item_to_hero(get_spelluser(), ITEM_ID_BRONZEFLASCHE, 0, 1);

			/* set AE cost */
			g_spell_special_aecost = random_schick(20);

			sprintf(g_dtp2,	get_tx(119), get_spelluser()->alias); /* "%s ENTZÜNDET DIE LATERNE." */
			/* "lantern is lit" */
		} else {
			/* prepare message */
			sprintf(g_dtp2, get_tx(120), get_spelluser()->alias); /* "%s HAT KEIN LAMPENÖL DABEI!" */
			/* "hero does not have oil" */
		}
	} else {
		sprintf(g_dtp2, get_tx(121), get_spelluser()->alias); /* "%s HAT WEDER EINE FACKEL NOCH EINE LATERNE DABEI!" */
		/* "hero has neither torch nor lantern" */
	}
}

void spell_claudibus(void)
{
#if !defined(__BORLANDC__)
	D1_INFO("Zauberspruch \"Claudibus\" ist nicht implementiert\n");
#endif
}

void spell_dunkelheit(void)
{
	/* set dunkelheit duration (level + 3) * hours */
	gs_ingame_timers[INGAME_TIMER_DARKNESS] = (int32_t)(get_spelluser()->level + 3) * HOURS(1);

	/* copy message text */
	strcpy(g_dtp2, get_tx(109)); /* "DUNKELHEIT NIMMT DIE UMGEBUNG IN BESCHLAG." */
}

void spell_erstarre(void)
{
#if !defined(__BORLANDC__)
	D1_INFO("Zauberspruch \"Erstarre\" ist nicht implementiert\n");
#endif
}

void spell_flim_flam(void)
{
	/* set flim flam duration (level + 3) hours */
	gs_ingame_timers[INGAME_TIMER_FLIM_FLAM] = (int32_t)(get_spelluser()->level + 3) * HOURS(1);

	/* copy message text */
	strcpy(g_dtp2, get_tx(110)); /* "UND ES WARD LICHT!" */
}

void spell_schmelze(void)
{
#if !defined(__BORLANDC__)
	D1_INFO("Zauberspruch \"Schmelze\" ist nicht implementiert\n");
#endif
}

void spell_silentium(void)
{
	signed int i;
	signed int mod_slot;
	struct struct_hero *hero = get_hero(0);

	for (i = 0; i <= 6; i++, hero++) {

		if ((hero->typus != HERO_TYPE_NONE) && (hero->group_id == gs_active_group_id) && !(hero->flags.dead))
		{
			/* get a free mod_slot */
			mod_slot = get_free_mod_slot();

			/* talent stealth + 10 for 5 minutes */
			set_mod_slot(mod_slot, MINUTES(5), (uint8_t*)&hero->talents[TA_SCHLEICHEN], 10, (signed char)i);
		}
	}

	/* set AE cost */
	g_spell_special_aecost = 5;

	/* copy message text */
	strcpy(g_dtp2, get_tx(111)); /* "DIE GRUPPE SCHLEICHT JETZT WIE AUF KATZENPFOTEN." */
}

void spell_sturmgebr(void)
{
#if !defined(__BORLANDC__)
	D1_INFO("Zauberspruch \"Sturmgebruell\" ist nicht implementiert\n");
#endif
}
