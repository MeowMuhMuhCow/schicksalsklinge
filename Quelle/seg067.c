/*
 * seg067 - Stadt: Zufallsereignisse auf der Strasse; Haendlerauskuenfte
 *
 * Inhalt:    city_event_switch() waehlt eines von neun Strassenereignissen (Texte aus
 *            STRASSE.LTX): Taschendiebe (Sinnesschaerfe-Probe), Bettler und falscher
 *            Bettler mit Geruecht, Kinder, Strassenhaendler (oeffnet den Laden von Haendler
 *            93) und harmlose Begebenheiten. waffinfo_*() liefern einen zufaelligen Text aus
 *            WAFFINFO.LTX fuer die Gespraeche der Haendler.
 * Aufrufer:  town_step() (seg066, 1 % pro Schritt in Thorwal, Prem, Phexcaer, Oberorken
 *            zwischen 8 und 20 Uhr); waffinfo_*() aus dem Dialogsystem (seg031).
 * Daten:     g_town_city_event_active (in vsync_or_key() ausgewertet).
 * Besonders: city_event_1(): bei der Verfolgung des Diebs erscheinen die Texte 27 und 28
 *            ohne ihren Zusammenhang, 29 und 30 nie. Bekannt (Bright-Eyes): Strassenhaendler
 *            ueberschreibt gs_town_typeindex (Bug 24), Drehung um 180 Grad (Bug 27).
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg067.c
 */
#include <stdio.h>

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg007.h"
#include "seg026.h"
#include "seg031.h"
#include "seg055.h"
#include "seg067.h"
#include "seg096.h"
#include "seg097.h"
#include "seg103.h"

signed char g_town_city_event_active = 0; // ds:0x7c42, 0 = default, 1 = city event active (vsync_or_key() allows a pause with the space bar)

/**
 * \brief   select a random city event type
 */
void city_event_switch(void)
{
	/* load STRASSE.LTX */
	load_tx(ARCHIVE_FILE_STRASSE_LTX);

	/* set city flag */
	g_town_city_event_active = 1;

	switch (random_schick(9)) {
		case 1: city_event_1(); break;
		case 2: city_event_2(); break;
		case 3: city_event_3(); break;
		case 4: city_event_4(); break;
		case 5: city_event_5(); break;
		case 6: city_event_6(); break;
		case 7: city_event_7(); break;
		case 8: city_event_8(); break;
		case 9: city_event_9(); break;
	}

	/* reset city flag */
	g_town_city_event_active = 0;

	/* load the LTX-file of the current town */
	load_tx(gs_town_id + 77);

	/* update the current position / make the step */
	gs_x = gs_x_bak;
	gs_y = gs_y_bak;
}

/**
 * \brief   random city event 1: a hero may lose W20 S to a pickpocket (if a Sinnesschaerfe+2 test fails)
 */
void city_event_1(void)
{
	signed int randval;
	signed int answer;
	struct struct_hero *hero;

	hero = get_hero(get_random_hero());

	randval = random_schick(20);

	if (test_talent(hero, TA_SINNESSCHAERFE, 2) <= 0) {

		/* TODO:	check the format strings in the english version
		 *		they may be broken.
		 *		Or, at least some changes on the code are neccessary.
		 */
		sprintf(g_dtp2, get_tx(random_schick(4) - 1), hero->alias, randval); /* 0..3 zufällig, z.B. "%s STELLT FEST, DASS PLÖTZLICH %d SILBERTA..." */

		GUI_dialogbox(hero->pic, hero->alias, g_dtp2, 0);

		randval *= 10;

		hero->money -= randval;

		if (hero->money < 0) {
			hero->money = 0;
		}
	} else {

		sprintf(g_dtp2, get_tx(random_schick(4) + 3), hero->alias, GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_2ND)); /* 4..7 zufällig, z.B. "HAT DA NICHT IRGENDWAS AM GELDBEUTEL GEZOG..." */

		answer = GUI_dialogbox(hero->pic, hero->alias, g_dtp2, 3,
				get_tx(random_schick(4) + 7), /* 8..11 zufällig, z.B. "<ÄH, WAS?>" */
				get_tx(random_schick(4) + 11), /* 12..15 zufällig, z.B. "<HALTET DEN DIEB!!!>" */
				get_tx(random_schick(4) + 15)); /* 16..19 zufällig, z.B. "<HINTERHER!>" */

		if (answer == 1) {

			GUI_output(get_tx(random_schick(4) + 19)); /* 20..23 zufällig, z.B. "TJA, NUN IST ER WEG. DEN SEHT IHR WOHL NIC..." */

		} else {

			randval = random_schick(5) - 1;

			if (randval == 2) {
				GUI_output(get_tx(26)); /* "DER MANN IST FIX, DAS MUSS DER NEID IHM LASSEN. ABER EURE WUT SPORN..." */
				GUI_output(get_tx(27)); /* "IN EINER GIGANTISCHEN LEISTUNG SCHWINGT SICH DER DIEB ÜBER DIE HOHE..." */
				GUI_output(get_tx(28)); /* "IHR HABT ETWAS SCHWIERIGKEITEN MIT DER@MAUER, UND ALS IHR AUF DER A..." */
			} else {
				/* Original-Bug? randval is 0, 1, 3 or 4, so the texts 24, 25, 27 and 28 are shown. 27 and 28 are the
				 * 2nd and 3rd part of the story above and make no sense alone, 29 and 30 are never shown.
				 * Probably get_tx(randval < 2 ? randval + 24 : randval + 26) was intended. */
				sprintf(g_dtp2,	get_tx(randval + 24), hero->alias); /* 24, 25, 27 oder 28, z.B. "IHR RENNT HINTER IHM HER UND BRÜLLT AUS LEIBESKRÄFTEN..." */
				GUI_output(g_dtp2);
			}
		}
	}
}

/**
 * \brief   random city event 2: a hero may lose all money to a pickpocket (if a Sinnesschaerfe+2 test fails)
 */
void city_event_2(void)
{
	signed int answer;
	struct struct_hero *hero;

	hero = get_hero(get_random_hero());

	if (test_talent(hero, TA_SINNESSCHAERFE, 2) <= 0) {

		/* hero looses all money */
		hero->money = 0;

		sprintf(g_dtp2, get_tx(random_schick(4) + 30), hero->alias); /* 31..34 zufällig, z.B. "MEHR ZUFÄLLIG STELLT %s FEST, DASS DER GEL..." */

		GUI_dialogbox(hero->pic, hero->alias, g_dtp2, 0);

	} else {

		sprintf(g_dtp2,	get_tx(random_schick(4) + 34), hero->alias); /* 35..38 zufällig, z.B. "ES DAUERT EINE ZEIT, BIS %s ETWAS MERKT, U..." */

		answer = GUI_dialogbox(hero->pic, hero->alias, g_dtp2, 3,
				get_tx(random_schick(4) + 38), /* 39..42 zufällig, z.B. "<VIEL SPASS DAMIT!>" */
				get_tx(random_schick(4) + 42), /* 43..46 zufällig, z.B. "<DAS IST UNGLAUBLICH.>" */
				get_tx(random_schick(4) + 46)); /* 47..50 zufällig, z.B. "<DEN PACKEN WIR NOCH!>" */

		if (answer == 1) {

			GUI_output(get_tx(random_schick(4) + 50)); /* 51..54 zufällig, z.B. "VON EINIGEN PASSANTEN ERNTET IHR ZUSTIMMEN..." */

		} else {

			sprintf(g_dtp2,	get_tx(random_schick(4) + 54), hero->alias); /* 55..58 zufällig, z.B. "VERGESST ES." */
			GUI_output(g_dtp2);
		}
	}
}

/**
 * \brief   random city event 3: a beggar asks for 1D
 */
void city_event_3(void)
{
	signed int answer;
	int32_t money;

	load_in_head(43);

	money = get_party_money();

	answer = money >= 100 ? 3 : 2;

	answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx(random_schick(4) + 58), answer, /* 59..62 zufällig, z.B. "AN EINER STRASSENECKE SITZT EIN EINBEINIGE..." */
			get_tx(random_schick(4) + 62), /* 63..66 zufällig, z.B. "<AUS DEM WEG,GESINDEL!>" */
			get_tx(random_schick(4) + 66), /* 67..70 zufällig, z.B. "<WIR HABEN SELBER NICHTS.>" */
			get_tx(random_schick(4) + 70)); /* 71..74 zufällig, z.B. "<NEHMT DIESES GOLDSTÜCK UND GEHT IN FRIEDEN.>" */

	if (answer == 3) {
		GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx(random_schick(4) + 74), 0); /* 75..78 zufällig, z.B. "<VIELEN HERZLICHEN DANK, UND MÖGEN DIE GÖT..." */
		money -= 100;
		set_party_money(money);
	}
}

/**
 * \brief   random city event 4: a false beggar asks for 1D and tells some gossip in return
 */
void city_event_4(void)
{
	signed int answer;
	int32_t money;

	load_in_head(43);

	money = get_party_money();

	answer = money >= 100 ? 3 : 2;

	answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx(random_schick(4) + 58), answer, /* 59..62 zufällig, z.B. "AN EINER STRASSENECKE SITZT EIN EINBEINIGE..." */
			get_tx(random_schick(4) + 78), /* 79..82 zufällig, z.B. "<VERZIEHT EUCH!>" */
			get_tx(random_schick(4) + 82), /* 83..86 zufällig, z.B. "<MEIN GELD IST MIR GERADE GESTOHLEN WORDEN.>" */
			get_tx(random_schick(4) + 86)); /* 87..90 zufällig, z.B. "<IHR SOLLT EUREN DUKATEN HABEN.>" */

	if (answer == 3) {

		money -= 100;
		set_party_money(money);

		GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx(random_schick(4) + 90), 0); /* 91..94 zufällig, z.B. "WIE VON WUNDERHAND GESUNDET, MACHT DER BET..." */


		sprintf(g_dtp2 + 0x400,	get_tx(random_schick(4) + 94), (char*)load_current_town_gossip()); /* 95..98 zufällig, z.B. "<ABER NICHTS IST UMSONST, UND IHR SEID OFF..." */

		GUI_dialogbox((unsigned char*)g_dtp2, NULL, (char*)(g_dtp2 + 0x400), 0);
	}
}

/**
 * \brief   random city event 5: children (one of the texts 99..102, harmless)
 */
void city_event_5(void)
{
	signed int randval;
	signed int tw_bak;
	struct struct_hero* hero;

	load_in_head(48);

	tw_bak = g_textbox_width;
	g_textbox_width = 5;

	randval = random_schick(4) - 1;

	if (randval == 1) {

		hero = get_first_hero_available_in_group();

		sprintf(g_dtp2 + 0x400,	get_tx(randval + 99), hero->alias); /* "EIN KIND BLEIBT VOR EUCH STEHEN UND S..." (randval == 1) */

		GUI_dialogbox((unsigned char*)g_dtp2, NULL, (char*)(g_dtp2 + 0x400), 0);
	} else {
		GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx(randval + 99), 0); /* 99/101/102 zufällig, z.B. "EIN KIND BLEIBT VOR EUCH STEHEN UND STARRT MIT RIESIGEN AUGEN..." */
	}

	g_textbox_width = tw_bak;
}

/**
 * \brief   random city event 6: meet a street merchant, the 3rd answer opens his shop (merchant 93)
 */
void city_event_6(void)
{
	signed int answer;
	signed int loc_bak;
#ifdef M302de_ORIGINAL_BUGFIX
	/* Original-Bug 24 */
	signed int type_bak;
#endif

	if (gs_day_timer >= HOURS(8) && gs_day_timer <= HOURS(20)) {

		load_in_head(4);

		sprintf(g_dtp2 + 0x400, get_tx(random_schick(4) + 102), get_hero(get_random_hero())->alias); /* 103..106 zufällig, z.B. "EIN MANN MIT EINEM GROSSEN BAUCHLADEN KOMM..." */

		answer = GUI_dialogbox((unsigned char*)g_dtp2, NULL, (char*)(g_dtp2 + 0x400), 3,
			get_tx(random_schick(4) + 106), /* 107..110 zufällig, z.B. "<VERSTEHE ICH NICHT.>" */
			get_tx(random_schick(4) + 110), /* 111..114 zufällig, z.B. "<BRINGT ES SCHNELL IN DEN HESINDETEMPEL!>" */
			get_tx(random_schick(4) + 114)); /* 115..118 zufällig, z.B. "<SUPER, HER DAMIT!>" */

		if (answer == 1 || answer == 2) {
			GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx(random_schick(4) + 118), 0); /* 119..122 zufällig, z.B. "DER MANN SIEHT EUCH FASSUNGSLOS AN UND VRS..." */
		} else if (answer == 3) {
			loc_bak = gs_town_loc_type;
			gs_town_loc_type = LOCTYPE_MERCHANT;
#ifdef M302de_ORIGINAL_BUGFIX
	/* Original-Bug 24:
	 * When entering a building in Thorwal, Prem, Phexcaer or Oberorken between 8:00 and 20:00 o'clock, the street merchant (random city event) shows up with a chance 1:900. Selecting the third answer in the text box, the merchant screen appears. After leaving the street merchant, the entered building is corrupted. For example, an entered temple will be a Praios temple (which otherwise does not exist in the game), or an entered tavern may offer negative food prices.
	 */
			type_bak = gs_town_typeindex;
#endif
			gs_town_typeindex = MERCHANT_TYPEINDEX_STREET_MERCHANT;
			do_merchant();
			gs_town_loc_type = loc_bak;
#ifdef M302de_ORIGINAL_BUGFIX
	/* Original-Bug 24 */
			gs_town_typeindex = type_bak;
#endif
	/* Original-Bug 27: After leaving the street merchant (random city event), the party is rotated by 180 degrees. This doesn't make too much sense, and it is inconsistent to the similar situation of visiting a merchant at a market, where no rotation is performed. If moreover the street merchant happens to appear when the party is about to enter some building, after leaving the street merchant and then leaving the building the party will *not* be rotated. */
#ifdef M302de_ORIGINAL_BUGFIX
	/* fix analogous to to Original-Bug 26.
	 * The rotation is performed in the function leave_location(), which has been called in do_merchant() above.
	 * We fix the bug in a hacky way by simply correcting the rotation afterwards. */
			gs_viewdir = REVERSE_VIEWDIR(gs_viewdir); /* rotate by 180 degrees */
#endif
		}
	}
}

/**
 * \brief   random city event 7: some harmless events
 */
void city_event_7(void)
{
	signed int randval;
	struct struct_hero *hero;

	randval = random_schick(4) - 1;
	hero = get_hero(get_random_hero());

	if (!randval) {

		sprintf(g_dtp2 + 0x400, get_tx(123), hero->alias); /* "%s TRITT IN EINEN HUNDEHAUFEN." */

		GUI_dialogbox(hero->pic, hero->alias, (char*)(g_dtp2 + 0x400), 0);

	} else if (randval == 1) {

		load_in_head(12);

		sprintf(g_dtp2 + 0x400, get_tx(124), hero->alias); /* "EUCH KOMMT EINE JUNGE FRAU ENTGEGEN. AUF DEM GESICHT TRÄGT SIE DEN..." */

		GUI_dialogbox((unsigned char*)g_dtp2, NULL, (char*)(g_dtp2 + 0x400), 0);

		sprintf(g_dtp2 + 0x400, get_tx(125), GUI_get_personal_pronoun(hero->sex, GRAMMAR_CASE_3RD)); /* "ALS SIE %s INS GESICHT SIEHT, WIRD SIE BLEICH UND STAMMELT@<ICH ESS..." */

		GUI_dialogbox((unsigned char*)g_dtp2, NULL, (char*)(g_dtp2 + 0x400), 0);

	} else if (randval == 2) {

		load_in_head(47);

		GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx(126), 0); /* "VON WEITEM KOMMT IN MÖRDERISCHEN TEMPO EINE KUTSCHE ANGERAST. SIE N..." */

		sprintf(g_dtp2 + 0x400, get_tx(127), hero->alias); /* "IHR KÖNNT GERADE NOCH RECHTZEITIG ZUR SEITE SPRINGEN, BEVOR@DIE KUT..." */

		GUI_dialogbox((unsigned char*)g_dtp2, NULL, (char*)(g_dtp2 + 0x400), 0);

		sprintf(g_dtp2 + 0x400, get_tx(128), hero->alias); /* "ABER WIE VON ZAUBERHAND GELENKT RAST DAS GESPANN UM HAARESBREITE AN..." */

		GUI_dialogbox((unsigned char*)g_dtp2, NULL, (char*)(g_dtp2 + 0x400), 0);
	} else {
		GUI_output(get_tx(129)); /* "AUF MARASKAN FÄLLT EIN SACK REIS UM." */
	}
}

/**
 * \brief   random city event 8: some harmless events
 */
void city_event_8(void)
{
	signed int randval;
	struct struct_hero *hero;

	randval = random_schick(4) - 1;
	hero = get_hero(get_random_hero());

	if (!randval) {

		GUI_output(get_tx(130)); /* "AUF MARASKAN FÄLLT SCHON WIEDER EIN SACK REIS UM." */

	} else if (randval == 1) {

		load_in_head(12);

		GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx(131), 0); /* "AUF EUCH KOMMT EINE FRAU ZU, DIE NEUGIERIG EURE WAFFEN MUSTERT. NAC..." */

		GUI_dialogbox((unsigned char*)g_dtp2, NULL, get_tx(132), 0); /* "JA? DAS DACHTE ICH MIR, DAS WOLLEN NÄMLICH ALLE, DIE HIER HERUMLAUF..." */

	} else if (randval == 3) {

		GUI_output(get_tx(133)); /* "IHR SEHT EINE STERNSCHNUPPE UND DÜRFT EUCH ETWAS WÜNSCHEN." */

	} else {

		sprintf(g_dtp2, get_tx(134), hero->alias); /* "JEMAND KIPPT SEINEN NACHTTOPF AUS DEM FENSTER, UNTER DEM GERADE %s..." */
		GUI_output(g_dtp2);
	}
}

/**
 * \brief   random city event 9: some harmless events
 */
void city_event_9(void)
{
	signed int randval = random_schick(4) - 1;

	if (!randval) {

		GUI_output(get_tx(135)); /* "AUF MARASKAN BLEIBEN ALLE REISSÄCKE STEHEN." */

	} else if (randval == 1) {

		GUI_output(get_tx(136)); /* "ÜBER EUCH ZIEHT URPLÖTZLICH EIN GOSSER WANDERSTERN DAHIN. KURZ DARA..." */

	} else if (randval == 2) {

		GUI_output(get_tx(137)); /* "EIN KLEINES MÄNNLEIN TANZT UM EINEN TISCH, AUF DEM EINIGE GEHÄUFTE@..." */

	} else {

		GUI_output(get_tx(138)); /* "HIER STINKT DIE STRASSE GANZ BESONDERS." */
		GUI_output(get_tx(139)); /* "ODER SEID IHR DAS?" */
	}
}

/**
 * \brief   return a pointer to a weapon related information (WAFFINFO.LTX 0..18, weapon merchant dialog)
 */
char* waffinfo_weapons(void)
{
	signed int randval;
	char **ptr;

	load_ltx(ARCHIVE_FILE_WAFFINFO_LTX);

	ptr = (char**)g_buffer9_ptr3;

	randval = random_schick(19) - 1;

	return ptr[randval];
}

/**
 * \brief   return a pointer to a herb related information (WAFFINFO.LTX 19..58, herb merchant dialog)
 */
char* waffinfo_herbs(void)
{
	signed int randval;
	char **ptr;

	load_ltx(ARCHIVE_FILE_WAFFINFO_LTX);

	ptr = ((char**)g_buffer9_ptr3) + 19;

	randval = random_schick(40) - 1;

	return ptr[randval];
}

/**
 * \brief   return a pointer to a general information (WAFFINFO.LTX 59..125, general merchant dialog)
 */
char* waffinfo_general(void)
{
	signed int randval;
	char **ptr;

	load_ltx(ARCHIVE_FILE_WAFFINFO_LTX);

	ptr = ((char**)g_buffer9_ptr3) + 59;

	randval = random_schick(67) - 1;

	return ptr[randval];
}
