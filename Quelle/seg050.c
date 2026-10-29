/*
 * seg050 - Stufenanstieg
 *
 * Inhalt:    Führt den Stufenaufstieg eines Helden durch: eine positive
 *            Eigenschaft +1, Versuch eine negative zu senken, MR neu berechnen,
 *            LE (bei Magiekundigen LE und AE) würfeln und verteilen, danach
 *            Talent- und Zaubersteigerungen. Im Anfängermodus steigert das
 *            Programm automatisch (Tabellen aus BSKILLS.DAT und Hauszauber-
 *            listen), im Fortgeschrittenenmodus wählt der Spieler, Magiekundige
 *            können Steigerungen zwischen Zaubern und Talenten umwandeln.
 * Aufrufer:  level_up aus der Hauptschleife (seg002); inc_talent_novice auch
 *            aus seg095.
 * Daten:     automatisch gesteigerte Zauber je Heldentyp und Magierakademie,
 *            Spruchbereiche der Akademien; Steigerungszähler (Versuche/Erfolge)
 *            liegen im Puffer g_buffer8_ptr.
 * Besonders: Neu (level_up): ESC/rechte Maustaste bei der positiven Eigenschaft
 *            erhöht Bytes von hero->money (ca. +168427 D), bei der negativen
 *            Eigenschaft wird stattdessen IN gesenkt; random_schick(SP__END)
 *            trifft auch Index 86 (= saved_spell_increases); die MR-Neuberechnung
 *            kennt den Ring nur in der linken Hand und vergleicht mit dem Wert
 *            inkl. Gegenstandsboni. Steigerungswurf gleichverteilt statt 2W6/3W6.
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg050.c
 */
#include <stdio.h>
#if !defined(__BORLANDC__)
#include <unistd.h>
#endif

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg004.h"
#include "seg007.h"
#include "seg026.h"
#include "seg046.h"
#include "seg047.h"
#include "seg050.h"
#include "seg097.h"
#include "seg098.h"
#include "seg103.h"
#include "seg120.h"

const signed int g_autoinc_spells_witch[3] = { SP_ODEM_ARCANUM, SP_BLITZ_DICH_FIND, -1 }; // ds:25924
const signed int g_autoinc_spells_druid[4] = { SP_ODEM_ARCANUM, SP_VERWANDLUNG_BEENDEN, SP_EXPOSAMI_CREATUR, -1 }; // ds:25930

static const signed int g_autoinc_spells_mage00[] = {
	SP_GEISTER_AUSTREIBEN, SP_BALSAM_SALABUNDE, SP_ARCANO_PSYCHOSTABILIS,
	SP_IGNIFAXIUS_FLAMMENSTRAHL, SP_KLARUM_PURUM_KRAEUTERSUD, SP_ANALUES_ARCANSTRUKTUR,
	SP_FLIM_FLAM_FUNKEL, SP_ARMATRUTZ, SP_HEPTAGON_UND_KROETENEI, SP_ABVENENUM_PEST_UND_GALLE, -1};
static const signed int g_autoinc_spells_mage01[] = {
	SP_ODEM_ARCANUM, SP_SENSIBAR_WAHR_UND_KLAR, SP_IGNIFAXIUS_FLAMMENSTRAHL,
	SP_BAND_UND_FESSEL, SP_DUPLICATUS_DOPPELPEIN, SP_GROSSE_VERWIRRUNG, SP_BALSAM_SALABUNDE,
	SP_HARMLOSE_GESTALT, SP_FLIM_FLAM_FUNKEL, SP_HERR_UEBER_DAS_TIERREICH, SP_EIGENSCHAFTEN_SEID_GELESEN, -1};
static const signed int g_autoinc_spells_mage02[] = {
	SP_NIHILATIO_GRAVITAS, SP_ODEM_ARCANUM, SP_SOLIDIRID_FARBENSPIEL,
	SP_FLIM_FLAM_FUNKEL, SP_BALSAM_SALABUNDE, SP_IGNIFAXIUS_FLAMMENSTRAHL, SP_DUNKELHEIT,
	SP_GARDIANUM_PARADEI, SP_ECLIPTIFACTUS, SP_GEISTER_BESCHWOEREN, SP_ANALUES_ARCANSTRUKTUR, -1};
static const signed int g_autoinc_spells_mage03[] = {
	SP_BALSAM_SALABUNDE, SP_IGNIFAXIUS_FLAMMENSTRAHL, SP_FLIM_FLAM_FUNKEL, SP_PENETRIZZEL_HOLZ_UND_STEIN,
	SP_CLAUDIBUS_CLAVISTIBOR, SP_SOLIDIRID_FARBENSPIEL, SP_ODEM_ARCANUM, SP_PLUMBUMBARUM_UND_NARRETEI,
	SP_IN_SEE_UND_FLUSS, SP_VISIBILI_VANITAR, SP_UEBER_EIS_UND_UEBER_SCHNEE, -1};
static const signed int g_autoinc_spells_mage04[] = {
	SP_ABVENENUM_PEST_UND_GALLE, SP_ODEM_ARCANUM, SP_EIGENSCHAFTEN_SEID_GELESEN, SP_ARMATRUTZ,
	SP_GARDIANUM_PARADEI, SP_BLITZ_DICH_FIND, SP_TIERE_BESPRECHEN, SP_KK_STEIGERN, SP_FLIM_FLAM_FUNKEL, SP_EISENROST_UND_GRUENER_SPAN, -1};
static const signed int g_autoinc_spells_mage05[] = {
	SP_BALSAM_SALABUNDE, SP_FLIM_FLAM_FUNKEL, SP_RESPONDAMI_VERITAR,
	SP_IGNIFAXIUS_FLAMMENSTRAHL, SP_BANNBALADIN, SP_ARMATRUTZ,
	SP_ARCANO_PSYCHOSTABILIS, SP_GARDIANUM_PARADEI, SP_HORRIPHOBUS, -1};
static const signed int g_autoinc_spells_mage06[] = {
	SP_GARDIANUM_PARADEI, SP_BALSAM_SALABUNDE, SP_PARALUE_PARALEIN,
	SP_FLIM_FLAM_FUNKEL, SP_ECLIPTIFACTUS, SP_HORRIPHOBUS, SP_DUNKELHEIT,
	SP_SAFT_KRAFT_MONSTERMACHT, SP_ODEM_ARCANUM, SP_FUROR_BLUT_UND_SULPHURDAMPF,
	SP_ARCANO_PSYCHOSTABILIS, SP_KK_STEIGERN, SP_GE_STEIGERN, SP_MU_STEIGERN, -1};
static const signed int g_autoinc_spells_mage07[] = {
	SP_BALSAM_SALABUNDE, SP_KLARUM_PURUM_KRAEUTERSUD, SP_IGNIFAXIUS_FLAMMENSTRAHL,
	SP_FLIM_FLAM_FUNKEL, SP_KK_STEIGERN, SP_ARCANO_PSYCHOSTABILIS, SP_GE_STEIGERN,
	SP_ECLIPTIFACTUS, SP_IN_SEE_UND_FLUSS, SP_ODEM_ARCANUM, SP_EIGENSCHAFTEN_SEID_GELESEN, -1};
static const signed int g_autoinc_spells_mage08[] = {
	SP_PARALUE_PARALEIN, SP_ARMATRUTZ, SP_GARDIANUM_PARADEI,
	SP_BALSAM_SALABUNDE, SP_IGNIFAXIUS_FLAMMENSTRAHL, SP_PENETRIZZEL_HOLZ_UND_STEIN,
	SP_FORAMEN_FORAMINOR, SP_WEICHES_ERSTARRE, SP_AEOLITUS_WINDGEBRAUS,
	SP_STURMGEBRUELL_BESAENFTGE_DICH, SP_PLUMBUMBARUM_UND_NARRETEI, SP_DESTRUCTIBO_ARCANITAS, -1};

const signed int* g_autoinc_spells_mage_index[9] = {
	g_autoinc_spells_mage00,
	g_autoinc_spells_mage01,
	g_autoinc_spells_mage02,
	g_autoinc_spells_mage03,
	g_autoinc_spells_mage04,
	g_autoinc_spells_mage05,
	g_autoinc_spells_mage06,
	g_autoinc_spells_mage07,
	g_autoinc_spells_mage08
}; // ds:0x662a, by magic school; uint8_t*
const signed int g_autoinc_spells_gelf[7] = { SP_FLIM_FLAM_FUNKEL, SP_BLITZ_DICH_FIND, SP_AXXELERATUS_BLITZGESCHWIND, SP_KLARUM_PURUM_KRAEUTERSUD, SP_TIERE_BESPRECHEN, SP_DUNKELHEIT, -1 }; // ds:26190
const signed int g_autoinc_spells_self[7] = { SP_SILENTIUM_SILENTILLE, SP_ARMATRUTZ, SP_FULMINICTUS_DONNERKEIL, SP_ADLERAUG_UND_LUCHSENOHR, SP_ODEM_ARCANUM, SP_FLIM_FLAM_FUNKEL, -1 }; // ds:26204
const signed int g_autoinc_spells_ielf[12] = { SP_BALSAM_SALABUNDE, SP_CHAMAELIONI_MIMIKRY, SP_BLITZ_DICH_FIND, SP_ADLERAUG_UND_LUCHSENOHR, SP_FULMINICTUS_DONNERKEIL, SP_ARMATRUTZ, SP_SILENTIUM_SILENTILLE, SP_ODEM_ARCANUM, SP_BAND_UND_FESSEL, SP_AXXELERATUS_BLITZGESCHWIND, SP_EXPOSAMI_CREATUR, -1 }; // ds:26218
const struct mssr g_magic_school_spellranges = { {
	{ SP_BEHERRSCHUNG_BRECHEN, SP_VERWANDLUNG_BEENDEN },			/* Antimagie */
	{ SP_BAND_UND_FESSEL, SP_ZWINGTANZ },					/* Beherrschung */
	{ SP_FUROR_BLUT_UND_SULPHURDAMPF, SP_SOLIDIRID_FARBENSPIEL },		/* Daemonologie + Elemente */
	{ SP_AXXELERATUS_BLITZGESCHWIND, SP_UEBER_EIS_UND_UEBER_SCHNEE },	/* Bewegung */
	{ SP_BALSAM_SALABUNDE, SP_TIERE_BESPRECHEN },				/* Heilung */
	{ SP_ADLERAUG_UND_LUCHSENOHR, SP_SENSIBAR_WAHR_UND_KLAR },		/* Hellsicht (former comment: Wahrnehmung) */
	{ SP_BLITZ_DICH_FIND, SP_SCHARFES_AUGE_UND_SICHRE_HAND },		/* Kampf */
	{ SP_ADLER_WOLF_UND_HAMMERHAI, SP_VISIBILI_VANITAR },			/* Verwandlung */
	{ SP_ABVENENUM_PEST_UND_GALLE, SP_STURMGEBRUELL_BESAENFTGE_DICH }	/* Veraenderung */
	/* Note: There are no schools for the spell groups "Illusion" (SP 45..48) and "Verstaendigung" (SP 58..59). */
} }; // ds:0x6682



/* increase counters of one talent/spell during a level up */
struct inc_states {
	signed char tries; /* failed tries since the last success */
	signed char incs; /* successful increases */
};

struct inc_states *g_inc_spells_counter;	// ds:0xe3b2
struct inc_states *g_inc_talents_counter;	// ds:0xe3b6
unsigned char *g_talents_buffer;		// ds:0xe3ba


/**
 * \brief   tries to increase a spell in advanced mode
 *
 * \param   hero        pointer to the hero
 * \param   spell_id    ID of the spell
 */
void inc_spell_advanced(struct struct_hero *hero, const signed int spell_id)
{
	signed int max_incs = 1;
	signed int randval;
	struct mssr a = g_magic_school_spellranges;

	if ((hero->typus == HERO_TYPE_HEXE) && (g_spell_descriptions[spell_id].origin == SPELL_ORIGIN_HEXE))
	{
		/* witch spell */
		max_incs = 2;
	}

	if ((hero->typus >= HERO_TYPE_AUELF) && /* hero is one of the three elven types */
		((g_spell_descriptions[spell_id].origin == SPELL_ORIGIN_AUELF) ||
			(g_spell_descriptions[spell_id].origin == SPELL_ORIGIN_WALDELF) ||
			(g_spell_descriptions[spell_id].origin == SPELL_ORIGIN_FIRNELF)))
	{
		/* elven spell */
		max_incs = 2;
	}

	if ((hero->typus == HERO_TYPE_DRUIDE) &&
		(g_spell_descriptions[spell_id].origin == SPELL_ORIGIN_DRUIDE))
	{
		/* druid spell */
		max_incs = 2;
	}

	if (hero->typus == HERO_TYPE_MAGIER) {

		/* mages */
		if (g_spell_descriptions[spell_id].origin == SPELL_ORIGIN_MAGIER) {
			/* spell is a mage spell */
			max_incs = 2;
		}

		/* check if spell is of the specialized group of the mage (according to his HERO_MAGIC_SCHOOL) */
		if ((a.a[hero->spell_school].first <= spell_id) &&
			(a.a[hero->spell_school].last >= spell_id)) {
			max_incs = 2;
		}

		/* depending on the HERO_MAGIC_SCHOOL, the mage will get 3 possible increases on certain spells */
		if (is_in_int_array(spell_id, g_magic_schools_table[hero->spell_school]))
		{
			max_incs = 3;
		}
	}

	if (g_inc_spells_counter[spell_id].incs >= max_incs) {

		/* no increase is possible */

		GUI_output(get_tx2(43)); /* "LEIDER KEINE STEIGERUNG MEHR MÖGLICH!" */

	} else if (g_inc_spells_counter[spell_id].tries == 3) {

		/* 3 increase failures -> no further increase try allowed */

		GUI_output(get_tx2(44)); /* "MEHR STEIGERUNGSVERSUCHE SIND NICHT MEHR ERLAUBT!" */
	} else {
		/* try to increase */

		hero->saved_spell_increases--;

#ifndef M302de_FEATURE_MOD
		if (hero->spells[spell_id] >= 11) {
			randval = random_interval(3, 18);
		} else {
			randval = random_interval(2, 12);
		}
#else
		/* Feature mod 2:
		 * use the exact talent/spell increase mechanism as in DSA 2/3 */
		if (hero->spells[spell_id] >= 10) {
			randval = dice_roll(3,6,0);
		} else {
			randval = dice_roll(2,6,0);
		}
#endif

		if (hero->spells[spell_id] < randval) {

			/* success */
			GUI_output(get_tx2(37)); /* "HURRA, GESCHAFFT!" */

			/* increment spell value */
			hero->spells[spell_id]++;

			/* set the try counter to 0 */
			g_inc_spells_counter[spell_id].tries = 0;
			/* increment the increase counter */
			g_inc_spells_counter[spell_id].incs++;

		} else {
			/* fail */
			GUI_output(get_ttx(338)); /* "LEIDER MISSLUNGEN!" */

			/* increment the try counter */
			g_inc_spells_counter[spell_id].tries++;
		}
	}
}

/**
 * \brief   tries to increase a talent in advanced mode
 *
 * \param   hero        pointer to the hero
 * \param   talent_id   ID of the talent
 */
void inc_talent_advanced(struct struct_hero *hero, const signed int talent_id)
{
	signed int randval;
	signed int max_incs;

	max_incs = g_talent_descriptions[talent_id].max_incs;

	if (g_inc_talents_counter[talent_id].incs >= max_incs) {

		/* no increase is possible */
		GUI_output(get_tx2(43)); /* "LEIDER KEINE STEIGERUNG MEHR MÖGLICH!" */

	} else if (g_inc_talents_counter[talent_id].tries == 3) {

		/* 3 failed tries -> no further increase try allowed */
		GUI_output(get_tx2(44)); /* "MEHR STEIGERUNGSVERSUCHE SIND NICHT MEHR ERLAUBT!" */

	} else {

		/* try to increase */
		hero->saved_talent_increases--;

#ifndef M302de_FEATURE_MOD
		if (hero->talents[talent_id] >= 11) {
			randval = random_interval(3, 18);
		} else {
			randval = random_interval(2, 12);
		}
#else
		/* Feature mod 2:
		 * use the exact spell/talent increase mechanism as in DSA 2/3 */
		if (hero->talents[talent_id] >= 10) {
			randval = dice_roll(3,6,0);
		} else {
			randval = dice_roll(2,6,0);
		}
#endif

		if (hero->talents[talent_id] < randval) {

			/* success */
			GUI_output(get_tx2(37)); /* "HURRA, GESCHAFFT!" */

			/* increment talent value */
			hero->talents[talent_id]++;

			/* set the try counter to 0 */
			g_inc_talents_counter[talent_id].tries = 0;
			/* increment the increase counter */
			g_inc_talents_counter[talent_id].incs++;

			if (talent_id <= TA_ZWEIHAENDER) {
				/* increment a melee weapon talent */

				sprintf(g_dtp2,	get_ttx(426), get_ttx(48 + talent_id)); /* "WAS WOLLEN SIE IN DER ZUGEHÖRIGEN KAMPFART..." / TEXT.LTX 48..99: Talentname */

				randval = -1;

				/* AT - value */
				sprintf(g_text_output_buf, get_ttx(427), hero->at_talent_bonus[talent_id]); /* "ATTACKE~%d" */

				/* PA - value */
				sprintf(g_text_output_buf + 50,	get_ttx(428), hero->pa_talent_bonus[talent_id]); /* "PARADE~%d" */

				do {
					randval = GUI_radio(g_dtp2, 2, g_text_output_buf, g_text_output_buf + 50);

				} while (randval == -1);

				if (randval == 1) {
					hero->at_talent_bonus[talent_id]++;
				} else {
					hero->pa_talent_bonus[talent_id]++;
				}
			}

		} else {
			/* fail */
			GUI_output(get_ttx(338)); /* "LEIDER MISSLUNGEN!" */

			/* increment the try counter */
			g_inc_talents_counter[talent_id].tries++;
		}
	}
}

/**
 * \brief   tries to increase a talent in novice mode
 *
 *          This function is quiet and does not check how many times
 *          a talent can be increased. So the correct rules come
 *          from the array which contain the talents.
 *
 * \param   hero        pointer to the hero
 * \param   talent_id   ID of the talent
 */
void inc_talent_novice(struct struct_hero *hero, const signed int talent_id)
{
	signed int done = 0;
	signed int randval;

	while (!done) {

		/* leave the loop if 3 tries failed or the talent value is 18 */
		if ((g_inc_talents_counter[talent_id].tries == 3) || (hero->talents[talent_id] == 18)) {

			done = 1;
#if !defined(__BORLANDC__)
			D1_INFO("%s kann Talent nicht weiter steigern (3 Fehlversuche oder Talentwert 18)\n", hero->alias);
#endif
		} else {

			/* dec available talent incs */
			hero->saved_talent_increases--;

			/* check if available talent incs are 0 */
			if (!hero->saved_talent_increases) {
				done = 1;
			}

#ifndef M302de_FEATURE_MOD
			/* talent value < 11: uniform random number 2..12, else 3..18 (not 2W6/3W6, see Feature mod 2) */
			if (hero->talents[talent_id] >= 11) {
				randval = random_interval(3, 18);
			} else {
				randval = random_interval(2, 12);
			}
#else
		/* Feature mod 2:
		 * use the exact talent/spell increase mechanism as in DSA 2/3 */
			if (hero->talents[talent_id] >= 10) {
				randval = dice_roll(3,6,0);
			} else {
				randval = dice_roll(2,6,0);
			}
#endif

			/* check if increase success */
			if (hero->talents[talent_id] < randval) {

				/* inc talent value */
				hero->talents[talent_id]++;

				/* reset failed counter */
				g_inc_talents_counter[talent_id].tries = 0;

				done = 1;

				/* adjust AT PA values */
				if (talent_id <= TA_ZWEIHAENDER) {
					if (hero->at_talent_bonus[talent_id] > hero->pa_talent_bonus[talent_id]) {
						hero->pa_talent_bonus[talent_id]++;
					} else {
						hero->at_talent_bonus[talent_id]++;
					}
				}

			} else {
				/* inc failed counter */
				g_inc_talents_counter[talent_id].tries++;
			}
		}
	}
}

/**
 * \brief   tries to increase a spell in novice mode
 *
 *          This function is quiet and does not check how many times
 *          a spell can be increased. So the correct rules come
 *          from the array which contain the spells.
 *
 * \param   hero        pointer to the hero
 * \param   spell_id    ID of the spell
 */
void inc_spell_novice(struct struct_hero *hero, const signed int spell_id)
{
	signed int done = 0;
	signed int randval;

	while (!done) {
		/* leave the loop if 3 tries to increase have failed or if the spell is already at the max value 18 */
		if ((g_inc_spells_counter[spell_id].tries == 3) ||
			(hero->spells[spell_id] == 18)) {
			done = 1;
#if !defined(__BORLANDC__)
			D1_INFO("%s kann Zauber nicht weiter steigern (3 Fehlversuche oder Talentwert 18)\n", hero->alias);
#endif
		} else {

			/* dec available spell incs */
			hero->saved_spell_increases--;

			/* check if available spell incs are 0 */
			if (!hero->saved_spell_increases) {
				done = 1;
			}

#ifndef M302de_FEATURE_MOD
			/* spell value < 11: uniform random number 2..12, else 3..18 (not 2W6/3W6, see Feature mod 2) */
			if (hero->spells[spell_id] >= 11) {
				randval = random_interval(3, 18);
			} else {
				randval = random_interval(2, 12);
			}
#else
		/* Feature mod 2:
		 * use the exact talent/spell increase mechanism as in DSA 2/3 */
			if (hero->spells[spell_id] >= 10) {
				randval = dice_roll(3,6,0);
			} else {
				randval = dice_roll(2,6,0);
			}
#endif

			/* check if increase success */
			if (hero->spells[spell_id] < randval) {

				/* inc spell value */
				hero->spells[spell_id]++;

				/* reset failed counter */
				g_inc_spells_counter[spell_id].tries = 0;

				done = 1;

			} else {
				/* inc failed counter */
				g_inc_spells_counter[spell_id].tries++;
			}
		}
	}
}

/**
 * \brief   brings your hero to the next level
 *
 *          Order: positive attribute +1, try to lower a negative attribute,
 *          new MR, LE (and AE) points, update AT/PA base, talent and spell increases
 *          (automatic in novice mode, by the player in advanced mode).
 *
 * \param   hero_pos    the position of the hero
 */
void level_up(const signed int hero_pos)
{
	signed int l_si; /* multi use */
	signed char mr;
	signed char rounds;
	signed char lowered;
	signed int i;
	struct struct_hero *hero;
	signed int text_file_index_bak;

	hero = get_hero(hero_pos);

	if (g_fading_state) {
		refresh_colors();
	}

	g_timers_disabled = 1;

	text_file_index_bak = g_text_file_index;

	load_tx2(ARCHIVE_FILE_CHARTEXT_LTX);

	g_inc_talents_counter = (struct inc_states*)(g_buffer8_ptr + 4500);
	g_inc_spells_counter = (struct inc_states*)((uint8_t*)g_inc_talents_counter + 208);
	g_talents_buffer = (unsigned char*)g_buffer9_ptr;

	l_si = load_archive_file(ARCHIVE_FILE_BSKILLS_DAT);

	read_archive_file(l_si, g_talents_buffer, 1300);

	close(l_si);


	sprintf(g_dtp2, get_ttx(411), hero->alias); /* "%s HAT SOEBEN EINE NEUE STUFE ERREICHT.@DANN WOLLEN WIR MAL SEHEN." */
	GUI_output(g_dtp2);

	g_action_id = 0;
	g_status_page_mode = 1;

	for (i = 0; i < SP__END; i++) {
		g_inc_spells_counter[i].tries = g_inc_spells_counter[i].incs = 0;
	}

	for (i = 0; i < TA__END; i++) {
		g_inc_talents_counter[i].tries = g_inc_talents_counter[i].incs = 0;
	}

	load_ggsts_nvf();

	/* increment level */
	hero->level++;

	g_status_page_hero = hero_pos;

	status_show(hero_pos);

	g_status_page_hunger = g_status_page_thirst = -1;

	update_status_bars();

	/* new context for l_si
	 * -1: cannot increase because all attributes are already >= 20,
	 * -2: no increase done yet,
	 * 0: increase done */
	l_si = -1;

	for (i = 0; i <= 6; i++) {
		if (hero->attrib[i].normal < 20) {
			l_si = 0;
		}
	}

	if (l_si != -1) {

		l_si = -2;


		do {

			g_basepos_x = -30;

			l_si = GUI_radio(get_tx2(35), 7, /* "WELCHE POSITIVE EIGENSCHAFT SOLL ERHÖHT WERDEN?" */
					get_ttx(412), get_ttx(413), /* "MUT" / "KLUGHEIT" */
					get_ttx(414), get_ttx(415), /* "CHARISMA" / "FINGERFERTIGKEIT" */
					get_ttx(416), get_ttx(417), /* "GEWANDTHEIT" / "INTUITION" */
					get_ttx(418)) - 1; /* "KÖRPERKRAFT" */

			/* Original-Bug: ESC or the right mouse button gives l_si = -2. attrib[-2] are the
			 * two upper bytes of hero->money. Unless they are >= 20, the loop ends and both bytes
			 * are incremented below: money += 65536 + 16777216 H (about 168427 D), no attribute rises. */
			if (hero->attrib[l_si].normal >= 20) {

				l_si = -2;
				GUI_output(get_tx2(43)); /* "LEIDER KEINE STEIGERUNG MEHR MÖGLICH!" */
			}

		} while (l_si == -2);

		hero->attrib[l_si].normal++;
		hero->attrib[l_si].current++;

		status_show(hero_pos);

		g_status_page_hunger = g_status_page_thirst = -1;

		update_status_bars();
	}

	l_si = -1;

	for (i = 7; i <= 13; i++) {

		if (hero->attrib[i].normal > 2) {
			l_si = 0;
		}
	}

	if (l_si != -1) {

		l_si = -2;

		do {

			l_si = GUI_radio(get_tx2(36), 7, /* "WELCHE NEGATIVE EIGENSCHAFT SOLL VERSUCHT WERDEN ZU ERNIEDRIGEN?" */
					get_ttx(419), get_ttx(420), /* "ABERGLAUBE" / "HÖHENANGST" */
					get_ttx(421), get_ttx(422), /* "RAUMANGST" / "GOLDGIER" */
					get_ttx(423), get_ttx(424), /* "TOTENANGST" / "NEUGIER" */
					get_ttx(425)) - 1; /* "JÄHZORN" */

			/* Original-Bug: ESC or the right mouse button gives l_si = -2, i.e. attrib[5] = IN.
			 * The loop ends and the hero tries to lower his IN (success is very likely). */
			if (hero->attrib[l_si + 7].normal <= 2) {

				l_si = -2;
				GUI_output(get_tx2(48)); /* "LEIDER KEINE ERNIEDRIGUNG MEHR MÖGLICH!" */
			}

		} while (l_si == -2);

		lowered = 0;

		for (i = 0; i < 3; i++) {

			if (random_schick(20) <= hero->attrib[l_si + 7].normal) {

				lowered = 1;
			}
		}

		if (lowered != 0) {

			hero->attrib[l_si + 7].normal--;
			hero->attrib[l_si + 7].current--;

			GUI_output(get_tx2(37)); /* "HURRA, GESCHAFFT!" */

			status_show(hero_pos);

			g_status_page_hunger = g_status_page_thirst = -1;

			update_status_bars();
		} else {

			GUI_output(get_ttx(338)); /* "LEIDER MISSLUNGEN!" */
		}
	}

	/* check changes in MR */

	mr = (hero->attrib[ATTRIB_KL].normal + hero->attrib[ATTRIB_MU].normal + hero->level) / 3 - 2 * hero->attrib[ATTRIB_AG].normal;
	mr += g_mr_modificators[hero->typus];

	/* Original-Bug: hero->mr contains the boni of MR items, mr does not. With such an item
	 * the message appears although the base MR did not change; if the base MR rises by
	 * exactly the bonus, nothing is updated and the bonus is lost (unequip subtracts it later). */
	if (hero->mr != mr) {

		/* set new basic MR */
		hero->mr = mr;

		/* check for STIRNREIF [blau] / CORONET [blue] equipped => MR + 2 */
		if (hero->inventory[HERO_INVENTORY_SLOT_HEAD].item_id == ITEM_ID_STIRNREIF__MR_BONUS) {
			hero->mr += 2;
		}
		/* check for RING / RING equipped => MR + 2 */
		/* Original-Bug: only the left hand is checked, but the ring gives MR + 2 in the right hand, too (add_equip_boni) */
		if (hero->inventory[HERO_INVENTORY_SLOT_LEFT_HAND].item_id == ITEM_ID_RING__RED) {
			hero->mr += 2;
		}
		/* check for AMULETT / in inventory => MR + 5 */
		if (inv_slot_of_item(hero, ITEM_ID_AMULETT__MR_BONUS) != -1) {
			hero->mr += 5;
		}

		/* show the user the new MR value */
		sprintf(g_dtp2, get_tx2(41), hero->mr); /* "DIE MAGIERESISTENZ HAT SICH AUF %d VERÄNDERT!" */
		GUI_output(g_dtp2);

		/* update status background */
		status_show(hero_pos);

		/* update_the bars of the hero */
		g_status_page_hunger = g_status_page_thirst = -1;

		update_status_bars();
	}


	/* add talent increasements */
	hero->saved_talent_increases += g_levelup_ta_rise[hero->typus - 1];

	/* roll how many LE points the hero may get */
	i = random_schick(6);

	if (hero->typus >= HERO_TYPE_HEXE) {
		/* a magic user */

		/* add spell increasements */
		hero->saved_spell_increases += g_levelup_sp_rise[hero->typus - 7];

		i += 2;

		/* show how many LE the hero may get */
		sprintf(g_dtp2, get_tx2(39), i); /* "ES STEHEN %d PUNKTE ZUR VERTEILUNG AUF LE UND AE ZUR VERFÜGUNG.@WIE..." */

		do {
			l_si = GUI_input(g_dtp2, 1);

		} while (l_si < 0);

		if (l_si > i) {
			l_si = i;
		}


		/* add LE and fill them up */
		hero->le_max += l_si;
		add_hero_le(hero, l_si);
		/* add AE and fill them up */
		hero->ae_max += i - l_si;
		add_hero_ae(hero, i - l_si);

		/* mages in advanced mode: trade 10 spell increases into AE */
		if ((hero->typus == HERO_TYPE_MAGIER) && (g_game_mode == GAME_MODE_ADVANCED)) {

			if (GUI_bool(get_tx2(40))) { /* "WOLLEN SIE ALS MAGIER(IN) ZUGUNSTEN IHRER ASTRALENERGIE AUF 10@STEI..." */
				/* trade 10 spell increases into 3..8 AE (uniform, like 1W6+2) */
				hero->saved_spell_increases += -10;
				i = random_interval(3, 8);
				hero->ae_max += i;
				hero->ae += i;
			}
		}

	} else {
		/* not a magic user */

		/* show how many LE the hero gets */
		sprintf(g_dtp2, get_tx2(38), i); /* "DIE LE-PUNKTE STEIGEN UM %d." */
		GUI_output(g_dtp2);

		/* add LE and fill them up */
		hero->le_max += i;
		add_hero_le(hero, i);
	}

	status_show(hero_pos);

	/* update_the bars of the hero */
	g_status_page_hunger = g_status_page_thirst = -1;

	update_status_bars();

	update_atpa(hero);

	if (g_game_mode == GAME_MODE_BEGINNER) {

		i = rounds = 0;

		while (hero->saved_talent_increases > 0) {

			l_si = *(int16_t*)(g_talents_buffer + 100 * hero->typus + 4 * i);

			if (hero->talents[l_si] < *(int16_t*)(g_talents_buffer + 100 * hero->typus + 4 * i + 2))
			{
				inc_talent_novice(hero, l_si);
			}

			i++;

			if (i > 24) {

				i = 0;

				rounds++;

				if (rounds > 5) {
					hero->saved_talent_increases = 0;
				}

			}
		}

		if (hero->typus >= HERO_TYPE_HEXE) {
			/* hero has magic type */

			i = 1;
			rounds = 0;

			/* up to 3 rounds. Remark: i is not reset at the start of a round, so the first
			 * loop of the cases HEXE, AUELF, FIRNELF and WALDELF only runs in the first round. */
			while (hero->saved_spell_increases != 0 && rounds < 3) {

				rounds++;

				switch (hero->typus) {

					case HERO_TYPE_HEXE: {

						/* first try to increase all witch-specific spells by 1,
						 * while the spell value is below 11. */
						while (hero->saved_spell_increases != 0 && i < SP__END) {

							if (g_spell_descriptions[i].origin == SPELL_ORIGIN_HEXE && hero->spells[i] < 11) {
								inc_spell_novice(hero, i);
							}
							i++;
						}

						/* if there are attempts left, try to increase spells from AUTOINC_SPELLS_WITCH list by 1,
						 * up to max value 18 */
						i = 0;

						while (hero->saved_spell_increases != 0 && (g_autoinc_spells_witch[i] != -1)) {
							inc_spell_novice(hero, g_autoinc_spells_witch[i]);
							i++;
						}

						break;
					}
					case HERO_TYPE_DRUIDE: {

						while (hero->saved_spell_increases != 0 && i < SP__END) {

							if (g_spell_descriptions[i].origin == SPELL_ORIGIN_DRUIDE && (hero->spells[i] < 11)) {
								inc_spell_novice(hero, i);
							}
							i++;
						}

						i = 0;

						while (hero->saved_spell_increases != 0 && (g_autoinc_spells_druid[i] != -1)) {
							inc_spell_novice(hero, g_autoinc_spells_druid[i]);
							i++;
						}

						break;
					}
					case HERO_TYPE_MAGIER: {

						i = 0;

						while (hero->saved_spell_increases != 0 && (g_magic_schools_table[hero->spell_school][i] != -1)) {

							if (hero->spells[g_magic_schools_table[hero->spell_school][i]] < 11)
							{
								inc_spell_novice(hero, g_magic_schools_table[hero->spell_school][i]);
							}
							i++;
						}

						i = 0;
						while (hero->saved_spell_increases != 0 && (g_autoinc_spells_mage_index[hero->spell_school][i] != -1)) {

							if (hero->spells[g_autoinc_spells_mage_index[hero->spell_school][i]] < 11)
							{
								inc_spell_novice(hero, g_autoinc_spells_mage_index[hero->spell_school][i]);
							}
							i++;
						}

						i = 0;
						while ((hero->saved_spell_increases) != 0 && (g_magic_schools_table[hero->spell_school][i] != -1)) {

							if (hero->spells[g_magic_schools_table[hero->spell_school][i]] < 11)
							{
								inc_spell_novice(hero, g_magic_schools_table[hero->spell_school][i]);
							}
							i++;
						}

						break;
					}
					case HERO_TYPE_AUELF: {

						while (hero->saved_spell_increases != 0 && i < SP__END) {

							if ((g_spell_descriptions[i].origin == SPELL_ORIGIN_AUELF) && (hero->spells[i] < 11)) {
								inc_spell_novice(hero, i);
							}
							i++;
						}

						i = 0;

						while (hero->saved_spell_increases != 0 && (g_autoinc_spells_gelf[i] != -1)) {
							inc_spell_novice(hero, g_autoinc_spells_gelf[i]);
							i++;
						}

						i = 1;
						while (hero->saved_spell_increases != 0 && i < SP__END) {

							if (g_spell_descriptions[i].origin == SPELL_ORIGIN_AUELF && (hero->spells[i] < 11)) {
								inc_spell_novice(hero, i);
							}
							i++;
						}

						break;
					}
					case HERO_TYPE_FIRNELF: {

						while (hero->saved_spell_increases != 0 && i < SP__END) {

							if (g_spell_descriptions[i].origin == SPELL_ORIGIN_FIRNELF && (hero->spells[i] < 11)) {
								inc_spell_novice(hero, i);
							}
							i++;
						}

						i = 0;

						while (hero->saved_spell_increases != 0 && (g_autoinc_spells_ielf[i] != -1)) {
							inc_spell_novice(hero, g_autoinc_spells_ielf[i]);
							i++;
						}

						i = 1;
						while (hero->saved_spell_increases != 0 && i < SP__END) {

							if (g_spell_descriptions[i].origin == SPELL_ORIGIN_FIRNELF && (hero->spells[i] < 11)) {
								inc_spell_novice(hero, i);
							}
							i++;
						}
						break;
					}
					case HERO_TYPE_WALDELF: {

						while (hero->saved_spell_increases != 0 && i < SP__END) {

							if ((g_spell_descriptions[i].origin == SPELL_ORIGIN_WALDELF) && (hero->spells[i] < 11)) {
								inc_spell_novice(hero, i);
							}
							i++;
						}

						i = 0;

						while ((hero->saved_spell_increases != 0) && (g_autoinc_spells_self[i] != -1)) {
							inc_spell_novice(hero, g_autoinc_spells_self[i]);
							i++;
						}

						i = 1;
						while ((hero->saved_spell_increases != 0) && (i < SP__END)) {

							if ((g_spell_descriptions[i].origin == SPELL_ORIGIN_WALDELF) && (hero->spells[i] < 11)) {
								inc_spell_novice(hero, i);
							}
							i++;
						}
						break;
					}
				}
			}

			while (hero->saved_spell_increases != 0) {
				/* Original-Bug: random_schick(SP__END) gives 1..86, but 85 is the last spell.
				 * Index 86 is hero->saved_spell_increases, that try is wasted. */
				inc_spell_novice(hero, random_schick(SP__END));
			}
		}

	} else {
		signed int spta_convs;

		if ((hero->typus >= HERO_TYPE_HEXE) && (spta_convs = g_levelup_spta_conv[hero->typus - 7]) && GUI_bool(get_tx2(45))) /* "MÖCHTEN SIE STEIGERUNGSVERSUCHE UMWANDELN?" */
		{

			sprintf(g_dtp2, get_tx2(46), spta_convs); /* "WIE VIELE ZAUBERSTEIGERUNGEN WOLLEN SIE IN TALENTSTEIGERUNGEN UMWAN..." */

			i = GUI_input(g_dtp2, 1);

			if (i > 0) {

				if (i > spta_convs) {
					i = spta_convs;
				}
				spta_convs -= i;

				hero->saved_spell_increases -= i;
				hero->saved_talent_increases += i;

			} else {

				sprintf(g_dtp2, get_tx2(47), spta_convs); /* "WIE VIELE TALENTSTEIGERUNGEN WOLLEN SIE IN ZAUBERSTEIGERUNGEN UMWAN..." */

				i = GUI_input(g_dtp2, 1);

				if (i > 0) {

					if (i > spta_convs) {
						i = spta_convs;
					}

					hero->saved_spell_increases += i;
					hero->saved_talent_increases -= i;
				}
			}
		}

		while (hero->saved_talent_increases != 0) {

			l_si = LVL_select_talent(hero, 1);

			if (l_si >= 0) {

				inc_talent_advanced(hero, l_si);

			} else if (l_si == -2) {

				if (GUI_bool(get_tx2(42))) { /* "STEIGERUNGEN DER TALENTE UND ZAUBER SIND NUR BEIM STUFENANSTIEG@MÖG..." */
					break;
				}
			}
		}

		while (hero->saved_spell_increases != 0) {

			l_si = select_spell(hero, 1);

			if (l_si > 0) {

				inc_spell_advanced(hero, l_si);

			} else if (l_si == -2) {

				if (GUI_bool(get_tx2(42))) { /* "STEIGERUNGEN DER TALENTE UND ZAUBER SIND NUR BEIM STUFENANSTIEG@MÖG..." */
					break;
				}
			}
		}
	}

	g_basepos_x = 0;
	g_request_refresh = 1;
	g_timers_disabled = 0;

	g_current_ani = -1;

	/* restore text file except for CHARTEXT.LTX, TAVERN.TLK and except for dialogs */
	if (text_file_index_bak != -1 && text_file_index_bak != ARCHIVE_FILE_CHARTEXT_LTX
	    && text_file_index_bak != ARCHIVE_FILE_TAVERN_TLK
	    && (text_file_index_bak < 156 || text_file_index_bak > 176)) {
		load_tx2(text_file_index_bak);
	}
}
