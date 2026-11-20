/*
 * seg093 - Reisekarte: Tabellen des Wegenetzes und Reisemodus am Wegweiser
 *
 * Inhalt:    Datentabellen der Landreise: 59 Landwege (g_land_routes: Orte, Laenge in Meilen,
 *            Tempo, Begegnungs- und Kampfchancen u. a.), die Wegweiser jeder Stadt mit ihren
 *            Landwegen (g_signposts), die 25 Haefen mit ihren Seewegen (g_harbors), die
 *            Kartenkoordinaten der Orte, die Klickflaechen der Kartengebiete und die Tabelle der
 *            Reiseereignisse (g_tevents_tab: Landweg, Meile, Ereignis-Nr.). do_travel_mode() zeigt
 *            am Wegweiser die Reisekarte, beschreibt angeklickte Orte und Gebiete (MAPTEXT.LTX),
 *            laesst per Rechtsklick/Bild hoch das Ziel waehlen und startet trv_do_journey() (seg094).
 * Aufrufer:  game_loop() in seg002, sobald gs_show_travel_map gesetzt ist (Wegweiser, seg025).
 *            Die Tabellen lesen auch seg002 (Kartenmarken), seg063/seg064 (Seereisen) und seg094.
 * Daten:     g_land_routes, g_signposts(_linked_lroute_NNN), g_harbors(_linked_sea_routes_NN),
 *            g_town_positions, g_action_table_travelmap, g_tevents_tab.
 * Besonders: Im Winter sind fuenf Gebirgs-/Wildniswege gesperrt. Original-Bug 40/41: falsche
 *            Seeweg-Listen fuer Hjalsingor und Leuchtturm Runin. Ereignis 71 (Ork-Monolith am
 *            Thossel, Weg Thoss-Liskor) steht zusaetzlich auf dem Weg Oberorken-Felsteyn, der mit
 *            Thoss nichts gemein hat (vermutlich Datenfehler).
 *
 * Uebersetzen: BCC.EXE -mlarge -O- -c -1 -Yo seg093.c
 */

#include <stdio.h>
#include <string.h>

#if !defined(__BORLANDC__)
#include <unistd.h>
#endif

#include "v302de.h"
#include "common.h"

#include "seg002.h"
#include "seg004.h"
#include "seg008.h"
#include "seg025.h"
#include "seg028.h"
#include "seg075.h"
#include "seg094.h"
#include "seg097.h"

struct struct_land_route g_land_routes[LROUTE_ID__END - 1]; // ds:0x9dc6
// towns not connected to any land route: TOWN_ID_OVERTHORN, TOWN_ID_HJALSINGOR, TOWN_ID_GUDDASUNDEN, TOWN_ID_TREBAN, TOWN_ID_ARYN

// ds:0x9fd9, 105 arrays, each terminated by -1
int8_t g_signposts_linked_lroute_001[2]; //ds:0x9fd9;
int8_t g_signposts_linked_lroute_002[2]; //ds:0x9fdb;
int8_t g_signposts_linked_lroute_003[2]; //ds:0x9fdd;
int8_t g_signposts_linked_lroute_004[2]; //ds:0x9fdf;
int8_t g_signposts_linked_lroute_005[3]; //ds:0x9fe1;
int8_t g_signposts_linked_lroute_006[2]; //ds:0x9fe4;
int8_t g_signposts_linked_lroute_007[2]; //ds:0x9fe6;
int8_t g_signposts_linked_lroute_008[2]; //ds:0x9fe8;
int8_t g_signposts_linked_lroute_009[2]; //ds:0x9fea;
int8_t g_signposts_linked_lroute_010[2]; //ds:0x9fec;
int8_t g_signposts_linked_lroute_011[2]; //ds:0x9fee;
int8_t g_signposts_linked_lroute_012[2]; //ds:0x9ff0;
int8_t g_signposts_linked_lroute_013[2]; //ds:0x9ff2;
int8_t g_signposts_linked_lroute_014[2]; //ds:0x9ff4;
int8_t g_signposts_linked_lroute_015[2]; //ds:0x9ff6;
int8_t g_signposts_linked_lroute_016[2]; //ds:0x9ff8;
int8_t g_signposts_linked_lroute_017[2]; //ds:0x9ffa;
int8_t g_signposts_linked_lroute_018[2]; //ds:0x9ffc;
int8_t g_signposts_linked_lroute_019[2]; //ds:0x9ffe;
int8_t g_signposts_linked_lroute_020[2]; //ds:0xa000;
int8_t g_signposts_linked_lroute_021[2]; //ds:0xa002;
int8_t g_signposts_linked_lroute_022[2]; //ds:0xa004;
int8_t g_signposts_linked_lroute_023[2]; //ds:0xa006;
int8_t g_signposts_linked_lroute_024[2]; //ds:0xa008;
int8_t g_signposts_linked_lroute_025[2]; //ds:0xa00a;
int8_t g_signposts_linked_lroute_026[2]; //ds:0xa00c;
int8_t g_signposts_linked_lroute_027[2]; //ds:0xa00e;
int8_t g_signposts_linked_lroute_028[2]; //ds:0xa010;
int8_t g_signposts_linked_lroute_029[2]; //ds:0xa012;
int8_t g_signposts_linked_lroute_030[2]; //ds:0xa014;
int8_t g_signposts_linked_lroute_031[2]; //ds:0xa016;
int8_t g_signposts_linked_lroute_032[2]; //ds:0xa018;
int8_t g_signposts_linked_lroute_033[2]; //ds:0xa01a;
int8_t g_signposts_linked_lroute_034[2]; //ds:0xa01c;
int8_t g_signposts_linked_lroute_035[2]; //ds:0xa01e;
int8_t g_signposts_linked_lroute_036[2]; //ds:0xa020;
int8_t g_signposts_linked_lroute_037[2]; //ds:0xa022;
int8_t g_signposts_linked_lroute_038[2]; //ds:0xa024;
int8_t g_signposts_linked_lroute_039[2]; //ds:0xa026;
int8_t g_signposts_linked_lroute_040[2]; //ds:0xa028;
int8_t g_signposts_linked_lroute_041[2]; //ds:0xa02a;
int8_t g_signposts_linked_lroute_042[2]; //ds:0xa02c;
int8_t g_signposts_linked_lroute_043[2]; //ds:0xa02e;
int8_t g_signposts_linked_lroute_044[2]; //ds:0xa030;
int8_t g_signposts_linked_lroute_045[2]; //ds:0xa032;
int8_t g_signposts_linked_lroute_046[2]; //ds:0xa034;
int8_t g_signposts_linked_lroute_047[2]; //ds:0xa036;
int8_t g_signposts_linked_lroute_048[6]; //ds:0xa038;
int8_t g_signposts_linked_lroute_049[2]; //ds:0xa03e;
int8_t g_signposts_linked_lroute_050[2]; //ds:0xa040;
int8_t g_signposts_linked_lroute_051[2]; //ds:0xa042;
int8_t g_signposts_linked_lroute_052[2]; //ds:0xa044;
int8_t g_signposts_linked_lroute_053[2]; //ds:0xa046;
int8_t g_signposts_linked_lroute_054[2]; //ds:0xa048;
int8_t g_signposts_linked_lroute_055[2]; //ds:0xa04a;
int8_t g_signposts_linked_lroute_056[2]; //ds:0xa04c;
int8_t g_signposts_linked_lroute_057[2]; //ds:0xa04e;
int8_t g_signposts_linked_lroute_058[3]; //ds:0xa050;
int8_t g_signposts_linked_lroute_059[2]; //ds:0xa053;
int8_t g_signposts_linked_lroute_060[2]; //ds:0xa055;
int8_t g_signposts_linked_lroute_061[3]; //ds:0xa057;
int8_t g_signposts_linked_lroute_062[3]; //ds:0xa05a;
int8_t g_signposts_linked_lroute_063[2]; //ds:0xa05d;
int8_t g_signposts_linked_lroute_064[3]; //ds:0xa05f;
int8_t g_signposts_linked_lroute_065[2]; //ds:0xa062;
int8_t g_signposts_linked_lroute_066[2]; //ds:0xa064;
int8_t g_signposts_linked_lroute_067[2]; //ds:0xa066;
int8_t g_signposts_linked_lroute_068[2]; //ds:0xa068;
int8_t g_signposts_linked_lroute_069[2]; //ds:0xa06a;
int8_t g_signposts_linked_lroute_070[2]; //ds:0xa06c;
int8_t g_signposts_linked_lroute_071[2]; //ds:0xa06e;
int8_t g_signposts_linked_lroute_072[2]; //ds:0xa070;
int8_t g_signposts_linked_lroute_073[2]; //ds:0xa072;
int8_t g_signposts_linked_lroute_074[2]; //ds:0xa074;
int8_t g_signposts_linked_lroute_075[2]; //ds:0xa076;
int8_t g_signposts_linked_lroute_076[2]; //ds:0xa078;
int8_t g_signposts_linked_lroute_077[2]; //ds:0xa07a;
int8_t g_signposts_linked_lroute_078[2]; //ds:0xa07c;
int8_t g_signposts_linked_lroute_079[2]; //ds:0xa07e;
int8_t g_signposts_linked_lroute_080[2]; //ds:0xa080;
int8_t g_signposts_linked_lroute_081[2]; //ds:0xa082;
int8_t g_signposts_linked_lroute_082[2]; //ds:0xa084;
int8_t g_signposts_linked_lroute_083[2]; //ds:0xa086;
int8_t g_signposts_linked_lroute_084[2]; //ds:0xa088;
int8_t g_signposts_linked_lroute_085[2]; //ds:0xa08a;
int8_t g_signposts_linked_lroute_086[2]; //ds:0xa08c;
int8_t g_signposts_linked_lroute_087[2]; //ds:0xa08e;
int8_t g_signposts_linked_lroute_088[2]; //ds:0xa090;
int8_t g_signposts_linked_lroute_089[2]; //ds:0xa092;
int8_t g_signposts_linked_lroute_090[2]; //ds:0xa094;
int8_t g_signposts_linked_lroute_091[2]; //ds:0xa096;
int8_t g_signposts_linked_lroute_092[2]; //ds:0xa098;
int8_t g_signposts_linked_lroute_093[2]; //ds:0xa09a;
int8_t g_signposts_linked_lroute_094[2]; //ds:0xa09c;
int8_t g_signposts_linked_lroute_095[2]; //ds:0xa09e;
int8_t g_signposts_linked_lroute_096[2]; //ds:0xa0a0;
int8_t g_signposts_linked_lroute_097[2]; //ds:0xa0a2;
int8_t g_signposts_linked_lroute_098[2]; //ds:0xa0a4;
int8_t g_signposts_linked_lroute_099[2]; //ds:0xa0a6;
int8_t g_signposts_linked_lroute_100[2]; //ds:0xa0a8;
int8_t g_signposts_linked_lroute_101[2]; //ds:0xa0aa;
int8_t g_signposts_linked_lroute_102[2]; //ds:0xa0ac;
int8_t g_signposts_linked_lroute_103[2]; //ds:0xa0ae;
int8_t g_signposts_linked_lroute_104[2]; //ds:0xa0b0;
int8_t g_signposts_linked_lroute_105[2]; //ds:0xa0b2;

struct trv_start_point g_signposts[106] = {
	{ TOWN_ID_THORWAL            , 1, (uint8_t*)g_signposts_linked_lroute_001 }, //  1
	{ TOWN_ID_THORWAL            , 2, (uint8_t*)g_signposts_linked_lroute_002 }, //  2
	{ TOWN_ID_THORWAL            , 4, (uint8_t*)g_signposts_linked_lroute_003 }, //  3
	{ TOWN_ID_SERSKE             , 1, (uint8_t*)g_signposts_linked_lroute_004 }, //  4
	{ TOWN_ID_SERSKE             , 2, (uint8_t*)g_signposts_linked_lroute_005 }, //  5
	{ TOWN_ID_BREIDA             , 1, (uint8_t*)g_signposts_linked_lroute_006 }, //  6
	{ TOWN_ID_BREIDA             , 2, (uint8_t*)g_signposts_linked_lroute_007 }, //  7
	{ TOWN_ID_BREIDA             , 3, (uint8_t*)g_signposts_linked_lroute_008 }, //  8
	{ TOWN_ID_PEILINEN           , 1, (uint8_t*)g_signposts_linked_lroute_009 }, //  9
	{ TOWN_ID_PEILINEN           , 2, (uint8_t*)g_signposts_linked_lroute_010 }, // 10
	{ TOWN_ID_ROVAMUND           , 1, (uint8_t*)g_signposts_linked_lroute_011 }, // 11
	{ TOWN_ID_ROVAMUND           , 2, (uint8_t*)g_signposts_linked_lroute_012 }, // 12
	{ TOWN_ID_ROVAMUND           , 3, (uint8_t*)g_signposts_linked_lroute_013 }, // 13
	{ TOWN_ID_NORDVEST           , 1, (uint8_t*)g_signposts_linked_lroute_014 }, // 14
	{ TOWN_ID_NORDVEST           , 2, (uint8_t*)g_signposts_linked_lroute_015 }, // 15
	{ TOWN_ID_KRAVIK             , 1, (uint8_t*)g_signposts_linked_lroute_016 }, // 16
	{ TOWN_ID_KRAVIK             , 2, (uint8_t*)g_signposts_linked_lroute_017 }, // 17
	{ TOWN_ID_SKELELLEN          , 1, (uint8_t*)g_signposts_linked_lroute_018 }, // 18
	{ TOWN_ID_SKELELLEN          , 2, (uint8_t*)g_signposts_linked_lroute_019 }, // 19
	{ TOWN_ID_MERSKE             , 2, (uint8_t*)g_signposts_linked_lroute_020 }, // 20
	{ TOWN_ID_MERSKE             , 3, (uint8_t*)g_signposts_linked_lroute_021 }, // 21
	{ TOWN_ID_MERSKE             , 4, (uint8_t*)g_signposts_linked_lroute_022 }, // 22
	{ TOWN_ID_EFFERDUN           , 2, (uint8_t*)g_signposts_linked_lroute_023 }, // 23
	{ TOWN_ID_TJOILA             , 1, (uint8_t*)g_signposts_linked_lroute_024 }, // 24
	{ TOWN_ID_TJOILA             , 2, (uint8_t*)g_signposts_linked_lroute_025 }, // 25
	{ TOWN_ID_RUKIAN             , 1, (uint8_t*)g_signposts_linked_lroute_026 }, // 26
	{ TOWN_ID_RUKIAN             , 2, (uint8_t*)g_signposts_linked_lroute_027 }, // 27
	{ TOWN_ID_ANGBODIRTAL        , 1, (uint8_t*)g_signposts_linked_lroute_028 }, // 28
	{ TOWN_ID_AUPLOG             , 1, (uint8_t*)g_signposts_linked_lroute_029 }, // 29
	{ TOWN_ID_AUPLOG             , 2, (uint8_t*)g_signposts_linked_lroute_030 }, // 30
	{ TOWN_ID_AUPLOG             , 3, (uint8_t*)g_signposts_linked_lroute_031 }, // 31
	{ TOWN_ID_VILNHEIM           , 1, (uint8_t*)g_signposts_linked_lroute_032 }, // 32
	{ TOWN_ID_VILNHEIM           , 2, (uint8_t*)g_signposts_linked_lroute_033 }, // 33
	{ TOWN_ID_VILNHEIM           , 3, (uint8_t*)g_signposts_linked_lroute_034 }, // 34
	{ TOWN_ID_VILNHEIM           , 4, (uint8_t*)g_signposts_linked_lroute_035 }, // 35
	{ TOWN_ID_BODON              , 1, (uint8_t*)g_signposts_linked_lroute_036 }, // 36
	{ TOWN_ID_OBERORKEN          , 1, (uint8_t*)g_signposts_linked_lroute_037 }, // 37
	{ TOWN_ID_OBERORKEN          , 2, (uint8_t*)g_signposts_linked_lroute_038 }, // 38
	{ TOWN_ID_OBERORKEN          , 3, (uint8_t*)g_signposts_linked_lroute_039 }, // 39
	{ TOWN_ID_PHEXCAER           , 1, (uint8_t*)g_signposts_linked_lroute_040 }, // 40
	{ TOWN_ID_PHEXCAER           , 2, (uint8_t*)g_signposts_linked_lroute_041 }, // 41
	{ TOWN_ID_PHEXCAER           , 3, (uint8_t*)g_signposts_linked_lroute_042 }, // 42
	{ TOWN_ID_PHEXCAER           , 4, (uint8_t*)g_signposts_linked_lroute_043 }, // 43
	{ TOWN_ID_GROENVELDEN        , 1, (uint8_t*)g_signposts_linked_lroute_044 }, // 44
	{ TOWN_ID_FELSTEYN           , 1, (uint8_t*)g_signposts_linked_lroute_045 }, // 45
	{ TOWN_ID_FELSTEYN           , 2, (uint8_t*)g_signposts_linked_lroute_046 }, // 46
	{ TOWN_ID_FELSTEYN           , 3, (uint8_t*)g_signposts_linked_lroute_047 }, // 47
	{ TOWN_ID_EINSIEDLERSEE      , 1, (uint8_t*)g_signposts_linked_lroute_048 }, // 48
	{ TOWN_ID_ORKANGER           , 1, (uint8_t*)g_signposts_linked_lroute_049 }, // 49
	{ TOWN_ID_ORKANGER           , 2, (uint8_t*)g_signposts_linked_lroute_050 }, // 50
	{ TOWN_ID_CLANEGH            , 1, (uint8_t*)g_signposts_linked_lroute_051 }, // 51
	{ TOWN_ID_CLANEGH            , 2, (uint8_t*)g_signposts_linked_lroute_052 }, // 52
	{ TOWN_ID_CLANEGH            , 3, (uint8_t*)g_signposts_linked_lroute_053 }, // 53
	{ TOWN_ID_LISKOR             , 1, (uint8_t*)g_signposts_linked_lroute_054 }, // 54
	{ TOWN_ID_LISKOR             , 2, (uint8_t*)g_signposts_linked_lroute_055 }, // 55
	{ TOWN_ID_LISKOR             , 3, (uint8_t*)g_signposts_linked_lroute_056 }, // 56
	{ TOWN_ID_THOSS              , 1, (uint8_t*)g_signposts_linked_lroute_057 }, // 57
	{ TOWN_ID_THOSS              , 2, (uint8_t*)g_signposts_linked_lroute_058 }, // 58
	{ TOWN_ID_THOSS              , 3, (uint8_t*)g_signposts_linked_lroute_059 }, // 59
	{ TOWN_ID_TJANSET            , 1, (uint8_t*)g_signposts_linked_lroute_060 }, // 60
	{ TOWN_ID_TJANSET            , 2, (uint8_t*)g_signposts_linked_lroute_061 }, // 61
	{ TOWN_ID_ALA                , 1, (uint8_t*)g_signposts_linked_lroute_062 }, // 62
	{ TOWN_ID_ALA                , 2, (uint8_t*)g_signposts_linked_lroute_063 }, // 63
	{ TOWN_ID_ORVIL              , 1, (uint8_t*)g_signposts_linked_lroute_064 }, // 64
	{ TOWN_ID_ORVIL              , 2, (uint8_t*)g_signposts_linked_lroute_065 }, // 65
	{ TOWN_ID_ORVIL              , 3, (uint8_t*)g_signposts_linked_lroute_066 }, // 66
	{ TOWN_ID_ROVIK              , 1, (uint8_t*)g_signposts_linked_lroute_067 }, // 67
	{ TOWN_ID_KORD               , 1, (uint8_t*)g_signposts_linked_lroute_068 }, // 68
	{ TOWN_ID_RUNINSHAVEN        , 1, (uint8_t*)g_signposts_linked_lroute_069 }, // 69
	{ TOWN_ID_RUNINSHAVEN        , 2, (uint8_t*)g_signposts_linked_lroute_070 }, // 70
	{ TOWN_ID_RUNINSHAVEN        , 4, (uint8_t*)g_signposts_linked_lroute_071 }, // 71
	{ TOWN_ID_OTTARJE            , 1, (uint8_t*)g_signposts_linked_lroute_072 }, // 72
	{ TOWN_ID_OTTARJE            , 2, (uint8_t*)g_signposts_linked_lroute_073 }, // 73
	{ TOWN_ID_OTTARJE            , 3, (uint8_t*)g_signposts_linked_lroute_074 }, // 74
	{ TOWN_ID_SKJAL              , 1, (uint8_t*)g_signposts_linked_lroute_075 }, // 75
	{ TOWN_ID_SKJAL              , 2, (uint8_t*)g_signposts_linked_lroute_076 }, // 76
	{ TOWN_ID_SKJAL              , 3, (uint8_t*)g_signposts_linked_lroute_077 }, // 77
	{ TOWN_ID_PREM               , 1, (uint8_t*)g_signposts_linked_lroute_078 }, // 78
	{ TOWN_ID_PREM               , 2, (uint8_t*)g_signposts_linked_lroute_079 }, // 79
	{ TOWN_ID_DASPOTA            , 1, (uint8_t*)g_signposts_linked_lroute_080 }, // 80
	{ TOWN_ID_DASPOTA            , 2, (uint8_t*)g_signposts_linked_lroute_081 }, // 81
	{ TOWN_ID_DASPOTA            , 3, (uint8_t*)g_signposts_linked_lroute_082 }, // 82
	{ TOWN_ID_RYBON              , 1, (uint8_t*)g_signposts_linked_lroute_083 }, // 83
	{ TOWN_ID_RYBON              , 2, (uint8_t*)g_signposts_linked_lroute_084 }, // 84
	{ TOWN_ID_LJASDAHL           , 2, (uint8_t*)g_signposts_linked_lroute_085 }, // 85
	{ TOWN_ID_LJASDAHL           , 3, (uint8_t*)g_signposts_linked_lroute_086 }, // 86
	{ TOWN_ID_LJASDAHL           , 4, (uint8_t*)g_signposts_linked_lroute_087 }, // 87
	{ TOWN_ID_VARNHEIM           , 1, (uint8_t*)g_signposts_linked_lroute_088 }, // 88
	{ TOWN_ID_VARNHEIM           , 2, (uint8_t*)g_signposts_linked_lroute_089 }, // 89
	{ TOWN_ID_VARNHEIM           , 3, (uint8_t*)g_signposts_linked_lroute_090 }, // 90
	{ TOWN_ID_VAERMHAG           , 1, (uint8_t*)g_signposts_linked_lroute_091 }, // 91
	{ TOWN_ID_VAERMHAG           , 2, (uint8_t*)g_signposts_linked_lroute_092 }, // 92
	{ TOWN_ID_TYLDON             , 1, (uint8_t*)g_signposts_linked_lroute_093 }, // 93
	{ TOWN_ID_TYLDON             , 2, (uint8_t*)g_signposts_linked_lroute_094 }, // 94
	{ TOWN_ID_VIDSAND            , 1, (uint8_t*)g_signposts_linked_lroute_095 }, // 95
	{ TOWN_ID_BRENDHIL           , 1, (uint8_t*)g_signposts_linked_lroute_096 }, // 96
	{ TOWN_ID_MANRIN             , 1, (uint8_t*)g_signposts_linked_lroute_097 }, // 97
	{ TOWN_ID_FAEHRSTATION_TJOILA, 1, (uint8_t*)g_signposts_linked_lroute_098 }, // 98
	{ TOWN_ID_FAEHRSTATION_TJOILA, 2, (uint8_t*)g_signposts_linked_lroute_099 }, // 99
	{ TOWN_ID_FAEHRSTATION_TJOILA, 3, (uint8_t*)g_signposts_linked_lroute_100 }, // 100
	{ TOWN_ID_FAEHRE_ANGBODIRTAL , 1, (uint8_t*)g_signposts_linked_lroute_101 }, // 101
	{ TOWN_ID_FAEHRE_ANGBODIRTAL , 2, (uint8_t*)g_signposts_linked_lroute_102 }, // 102
	{ TOWN_ID_FAEHRE_ANGBODIRTAL , 3, (uint8_t*)g_signposts_linked_lroute_103 }, // 103
	{ TOWN_ID_HJALLANDER_HOF     , 1, (uint8_t*)g_signposts_linked_lroute_104 }, // 104
	{ TOWN_ID_LEUCHTTURM_RUNIN   , 2, (uint8_t*)g_signposts_linked_lroute_105 }, // 105
	{ -1                         , 0, NULL                                    }
}; // ds:0xa0b4; uint8_t*

/* For each of the 25 harbors (see g_harbors), a -1 terminated list of the IDs of the linked sea routes */
int8_t g_harbors_linked_sea_routes_01[6]; //ds:0xa330
int8_t g_harbors_linked_sea_routes_02[3]; //ds:0xa336
int8_t g_harbors_linked_sea_routes_03[4]; //ds:0xa339
int8_t g_harbors_linked_sea_routes_04[4]; //ds:0xa33d
int8_t g_harbors_linked_sea_routes_05[4]; //ds:0xa341
int8_t g_harbors_linked_sea_routes_06[4]; //ds:0xa345
int8_t g_harbors_linked_sea_routes_07[2]; //ds:0xa349
int8_t g_harbors_linked_sea_routes_08[8]; //ds:0xa34b
int8_t g_harbors_linked_sea_routes_09[5]; //ds:0xa353
int8_t g_harbors_linked_sea_routes_10[7]; //ds:0xa358
int8_t g_harbors_linked_sea_routes_11[3]; //ds:0xa35f
int8_t g_harbors_linked_sea_routes_12[5]; //ds:0xa362
int8_t g_harbors_linked_sea_routes_13[4]; //ds:0xa367
int8_t g_harbors_linked_sea_routes_14[3]; //ds:0xa36b
int8_t g_harbors_linked_sea_routes_15[7]; //ds:0xa36e
int8_t g_harbors_linked_sea_routes_16[6]; //ds:0xa375
int8_t g_harbors_linked_sea_routes_17[4]; //ds:0xa37b
int8_t g_harbors_linked_sea_routes_18[9]; //ds:0xa37f
int8_t g_harbors_linked_sea_routes_19[3]; //ds:0xa388
int8_t g_harbors_linked_sea_routes_20[5]; //ds:0xa38b
int8_t g_harbors_linked_sea_routes_21[3]; //ds:0xa390
int8_t g_harbors_linked_sea_routes_22[4]; //ds:0xa393
int8_t g_harbors_linked_sea_routes_23[3]; //ds:0xa397
int8_t g_harbors_linked_sea_routes_24[6]; //ds:0xa39a
int8_t g_harbors_linked_sea_routes_25[3]; //ds:0xa3a0

struct trv_start_point g_harbors[26] = { // a list of the available sea routes for each of the 25 harbors in the game.
	{ TOWN_ID_THORWAL         , 3, (uint8_t*)g_harbors_linked_sea_routes_01 }, //  1
	{ TOWN_ID_SERSKE          , 3, (uint8_t*)g_harbors_linked_sea_routes_02 }, //  2
	{ TOWN_ID_MERSKE          , 1, (uint8_t*)g_harbors_linked_sea_routes_03 }, //  3
	{ TOWN_ID_EFFERDUN        , 1, (uint8_t*)g_harbors_linked_sea_routes_04 }, //  4
	{ TOWN_ID_LISKOR          , 4, (uint8_t*)g_harbors_linked_sea_routes_05 }, //  5
	{ TOWN_ID_TJANSET         , 3, (uint8_t*)g_harbors_linked_sea_routes_06 }, //  6
	{ TOWN_ID_ORVIL           , 4, (uint8_t*)g_harbors_linked_sea_routes_07 }, //  7
	{ TOWN_ID_OVERTHORN       , 1, (uint8_t*)g_harbors_linked_sea_routes_08 }, //  8
	{ TOWN_ID_ROVIK           , 2, (uint8_t*)g_harbors_linked_sea_routes_09 }, //  9
	{ TOWN_ID_HJALSINGOR      , 1, (uint8_t*)g_harbors_linked_sea_routes_10 }, // 10
	{ TOWN_ID_GUDDASUNDEN     , 1, (uint8_t*)g_harbors_linked_sea_routes_11 }, // 11
	{ TOWN_ID_KORD            , 2, (uint8_t*)g_harbors_linked_sea_routes_12 }, // 12
	{ TOWN_ID_TREBAN          , 1, (uint8_t*)g_harbors_linked_sea_routes_13 }, // 13
	{ TOWN_ID_ARYN            , 1, (uint8_t*)g_harbors_linked_sea_routes_14 }, // 14
	{ TOWN_ID_RUNINSHAVEN     , 3, (uint8_t*)g_harbors_linked_sea_routes_15 }, // 15
	{ TOWN_ID_OTTARJE         , 4, (uint8_t*)g_harbors_linked_sea_routes_16 }, // 16
	{ TOWN_ID_SKJAL           , 4, (uint8_t*)g_harbors_linked_sea_routes_17 }, // 17
	{ TOWN_ID_PREM            , 3, (uint8_t*)g_harbors_linked_sea_routes_18 }, // 18
	{ TOWN_ID_LJASDAHL        , 1, (uint8_t*)g_harbors_linked_sea_routes_19 }, // 19
	{ TOWN_ID_VARNHEIM        , 4, (uint8_t*)g_harbors_linked_sea_routes_20 }, // 20
	{ TOWN_ID_VAERMHAG        , 3, (uint8_t*)g_harbors_linked_sea_routes_21 }, // 21
	{ TOWN_ID_VIDSAND         , 2, (uint8_t*)g_harbors_linked_sea_routes_22 }, // 22
	{ TOWN_ID_BRENDHIL        , 2, (uint8_t*)g_harbors_linked_sea_routes_23 }, // 23
	{ TOWN_ID_MANRIN          , 2, (uint8_t*)g_harbors_linked_sea_routes_24 }, // 24
#ifndef M302de_ORIGINAL_BUGFIX
	/* Original-Bug 41: After doing a ship passage to Leuchtturm Runin and leaving the harbor,
	 * the party is positioned in a remote town square. */
	/* Reason: The following line is incorrect; it is a duplicate of the corresponding entry in g_signposts. */
//	{ TOWN_ID_LEUCHTTURM_RUNIN, 2, (int8_t*)0x14fca0b2 }, // 25 links to land route 105
	{ TOWN_ID_LEUCHTTURM_RUNIN, 2, (int8_t*)g_signposts_linked_lroute_105 }, // 25 links to land route 105
#else
	{ TOWN_ID_LEUCHTTURM_RUNIN, 1, (uint8_t*)g_harbors_linked_sea_routes_25 }, // 25
#endif
	{ -1                 , 0, NULL }
}; // ds:0xa3a3; uint8_t*
struct struct_point g_town_positions[TOWN_ID__END - 1]; // ds:0xa43f
static struct mouse_action g_action_table_travelmap[35] = {
	{ 0x55,  0xa4, 0x65, 0xb6, 0x01},
	{ 0x6d,  0x70, 0x88, 0x7e, 0x02},
	{ 0x60,  0x80, 0x6a, 0x86, 0x03},
	{ 0x5c,  0x86, 0x61, 0x8a, 0x03},
	{ 0x5e,  0x9b, 0x62, 0xa0, 0x03},
	{ 0x00,  0x00, 0x2e, 0x15, 0x04},
	{ 0x45,  0x7f, 0xb9, 0xa6, 0x05},
	{ 0x64,  0xa6, 0xbb, 0xbb, 0x05},
	{ 0x75,  0x58, 0x91, 0x70, 0x05},
	{ 0x91,  0x70, 0xa7, 0x7e, 0x05},
	{ 0x2f,  0x00, 0x4a, 0x09, 0x06},
	{ 0x4b,  0x00, 0x72, 0x1a, 0x06},
	{ 0x00,  0x16, 0x39, 0x42, 0x07},
	{ 0x00,  0x06, 0x14, 0x14, 0x07},
	{ 0x3a,  0x26, 0x44, 0x3e, 0x07},
	{ 0x41,  0x4b, 0x64, 0x76, 0x08},
	{ 0x52,  0x32, 0x6c, 0x4a, 0x08},
	{ 0x6d,  0x23, 0xaa, 0x4b, 0x08},
	{ 0xab,  0x20, 0xd7, 0x36, 0x08},
	{ 0xe1,  0x57, 0xfa, 0x6e, 0x09},
	{ 0xfb,  0x57, 0x117, 0x7a, 0x09},
	{ 0x118, 0x60, 0x13f, 0x82, 0x09},
	{ 0xd6,  0x6f, 0xfa, 0x82, 0x0a},
	{ 0xfb,  0x7b, 0x117, 0x92, 0x0a},
	{ 0x118, 0x83, 0x13f, 0x99, 0x0a},
	{ 0xa8,  0x00, 0xdb, 0x1f, 0x0b},
	{ 0xee,  0x0c, 0xfa, 0x12, 0x0d},
	{ 0xdc,  0x00, 0x13f, 0x41, 0x0c},
	{ 0x128, 0x41, 0x13f, 0x54, 0x0e},
	{ 0x11a, 0x00, 0x138, 0x35, 0x0f},
	{ 0xd8,  0x36, 0x125, 0x4b, 0x0f},
	{ 0xb9,  0x4a, 0xd7, 0xa0, 0x0f},
	{ 0xd8,  0x8c, 0xf0, 0xaf, 0x10},
	{ 0x95,  0x49, 0xbd, 0x6a, 0x11},
	{ -1, 0x00, 0x00, 0x00, 0x00 }
}; // ds:0xa50f

struct struct_land_route_tevent g_tevents_tab[156]; // ds:0xa66d

/**
 * \brief   Reisemodus am Wegweiser: Reisekarte zeigen und eine Landreise starten
 *
 *          Rechtsklick/Bild hoch: Zielauswahl aus den Wegen des Wegweisers, dann trv_do_journey().
 *          Linksklick auf einen Ort oder ein Kartengebiet: Beschreibung aus MAPTEXT.LTX.
 * \return  immer 0
 */
signed int do_travel_mode(void)
{
	dsa_trace_enter(780, "seg093:do_travel_mode");
	signed int tmp1; /* multiple use: route_id, screen coordinate, etc. */
	signed int i;
	signed int tmp2; /* multiple use: town_id, screen coordinate, menu answer, saved marker x */
	signed int town_id;
	signed int route_id;
	struct trv_start_point *signpost_ptr;
	signed int x_bak;
	signed int y_bak;
	signed int wallclock_update_bak;
	signed int tw_bak;
	char *destinations_tab[6];

	wallclock_update_bak = g_wallclock_update;
	g_journey_crosslink_status = g_wallclock_update = gs_travel_detour = 0;
	gs_town_id = gs_town_typeindex;

	call_mouse_bg();

	for (i = 0; i < 6; i++)
	{
		g_food_message_shown[i] = 0;
	}

	if (g_travel_map_loaded != gs_show_travel_map)
	{
		load_map();
	}

	memmove((void*)g_renderbuf_ptr, (void*)gs_travel_map_ptr, 64000);

	map_effect((uint8_t*)g_renderbuf_ptr);

	wait_for_vsync();

	set_palette(gs_travel_map_ptr + 64000 + 2, 0, 0x20);

	call_mouse();

	prepare_map_marker();

	g_mouse_leftclick_event = 0;

	signpost_ptr = &g_signposts[0];

	do {
		if (g_request_refresh)
		{
			call_mouse_bg();

			if (g_travel_map_loaded != gs_show_travel_map)
			{
				load_map();
			}

			wait_for_vsync();

			/* TODO: update window */
			memmove((void*)g_vga_memstart, (void*)gs_travel_map_ptr, 320 * 200);

			wait_for_vsync();

			set_palette(gs_travel_map_ptr + 64000 + 2, 0, 0x20);

			call_mouse();

			g_request_refresh = 0;
		}

		if ((signpost_ptr->town_id == gs_town_id) && (signpost_ptr->typeindex == gs_current_signpost_typeindex))
		{
			while (1) {
				handle_input();

				if (g_mouse_rightclick_event || g_action_id == KEY_SCAN_CODE_PAGE_UP)
				{
					i = 0;
					while ((tmp1 = signpost_ptr->linked_travel_routes[i]) != 0xff)
					{
						destinations_tab[i] = get_ttx(235 + (gs_trv_signpost_menu_town_ids[i] = ( /* TEXT.LTX 236..287: Ortsname */
							(tmp2 = g_land_routes[tmp1 - 1].town_id_1) != gs_town_id ?
							(signed char) tmp2 : g_land_routes[tmp1 - 1].town_id_2)));

						i++;
					}

					gs_trv_signpost_menu_town_ids[i] = gs_town_id;
					destinations_tab[i] = get_ttx(613); /* "ZURÜCK" */
					i++;

					sprintf(g_text_output_buf, get_ttx(545), get_ttx(235 + gs_town_id)); /* "WOHIN WILLST DU DEINE GRUPPE VON %s AUS SC..." / TEXT.LTX 236..287: Ortsname */

					tw_bak = g_textbox_width;
					g_textbox_width = 4;

					tmp2 = GUI_radio(g_text_output_buf, i,
								destinations_tab[0], destinations_tab[1], destinations_tab[2],
								destinations_tab[3], destinations_tab[4], destinations_tab[5]) - 1;

					g_textbox_width = tw_bak;

					g_current_town_anix = 0;

					set_and_spin_lock();

					if (i - 1 == tmp2 || tmp2 == -2)
					{
						gs_show_travel_map = 0;
						gs_viewdir = REVERSE_VIEWDIR(gs_viewdir);
						break;
					}

					route_id = signpost_ptr->linked_travel_routes[tmp2];
					gs_journey_destination_town_id = gs_trv_signpost_menu_town_ids[tmp2];

					if (!get_current_season() && (
						route_id == LROUTE_ID_OTTARJE_ORVIL
						|| route_id == LROUTE_ID_FELSTEYN_ORKANGER
						|| route_id == LROUTE_ID_RYBON_THOSS
						|| route_id == LROUTE_ID_SKJAL_ORVIL
						|| route_id == LROUTE_ID_SKELELLE_PHEXCAER
					)) {
						/* routes not available in winter */
						GUI_input(get_tx(69), 0); /* "IM WINTER IST DIESE STRECKE LEBENSGEFÄHRLICH. IHR KÖNNT SIE NICHT B..." */
						break;
					}

					g_wallclock_x = g_basepos_x + 120;
					g_wallclock_y = g_basepos_y + 87;

					g_wallclock_update = 1;

					trv_do_journey(signpost_ptr->linked_travel_routes[tmp2],
						(g_land_routes[signpost_ptr->linked_travel_routes[tmp2] - 1].town_id_1 == gs_town_id ? 0 : 1));
					g_wallclock_update = 0;

					if (g_journey_crosslink_status)
					{
						TM_do_journey_crosslink();
					}

					if (gs_journey_direction == JOURNEY_DIRECTION_BACKWARD)
					{
						if (gs_travel_detour && gs_travel_detour != 99)
						{
							DNG_enter_dungeon(gs_travel_detour);
						}
						break;
					}

					trv_find_dest_town_signpost();

					if (!gs_travel_detour && g_game_state == GAME_STATE_MAIN)
					{
						gs_town_id = gs_travel_destination_town_id;
						gs_x_bak = gs_travel_destination_x;
						gs_y_bak = gs_travel_destination_y;
						gs_viewdir = REVERSE_VIEWDIR(gs_travel_destination_viewdir);

					} else if (g_game_state == GAME_STATE_MAIN && gs_travel_detour != 99)
					{
						DNG_enter_dungeon(gs_travel_detour);
					}

					break;

				} else if (g_mouse_leftclick_event)
				{

					for (i = 0, town_id = -1; i < TOWN_ID__END - 1; i++)
					{
#if defined(__BORLANDC__)
						/* Borland wertet die Argumente von rechts nach links aus: tmp1/tmp2 sind schon
						 * auf die Ortskoordinaten gesetzt, wenn tmp1 - 4 und tmp2 - 4 berechnet werden. */
						if (is_mouse_in_rect(tmp1 - 4, tmp2 - 4,
								(tmp1 = g_town_positions[i].x) + 4,
								(tmp2 = g_town_positions[i].y) + 4))
#else
						const signed int x = g_town_positions[i].x;
						const signed int y = g_town_positions[i].y;

						if (is_mouse_in_rect(x - 4, y - 4, x + 4, y + 4))
#endif
						{
							town_id = i;
							break;
						}
					}

					if (town_id == -1 && (tmp1 = get_mouse_action(g_mouse_posx, g_mouse_posy, g_action_table_travelmap)))
					{
						town_id = tmp1 + 51; /* Kartengebiete: MAPTEXT.LTX 52..68 */
					}

					if (town_id != -1)
					{
						tmp2 = g_current_town_anix;
						g_current_town_anix = 0;
						x_bak = g_basepos_x;
						y_bak = g_basepos_y;
						g_basepos_y = 0;
						g_basepos_x = (g_mouse_posx >= 0 && g_mouse_posx <= 159 ? 80 : -80);

						set_and_spin_lock();

						GUI_input(get_tx(town_id), 0); /* MAPTEXT.LTX 0..51: Ortsname und Beschreibung, 52..68: Gebiet */

						g_basepos_x = x_bak;
						g_basepos_y = y_bak;
						g_current_town_anix = tmp2;
					}

					g_mouse_leftclick_event = 0;
				}
			}
			break;
		}

		signpost_ptr++;

	} while (signpost_ptr->town_id != -1);

	g_current_town_anix = g_current_town_aniy = g_selected_town_anix = g_selected_town_aniy = 0;

	i = load_archive_file(ARCHIVE_FILE_COMPASS);
	read_archive_file(i, g_buffer6_ptr, 5000);
	close(i);

	gs_show_travel_map = g_basepos_x = g_basepos_y = g_current_town_over = g_trv_menu_selection = 0;

	if (!gs_travel_detour)
	{
		g_wallclock_update = 0;
		leave_location();

	} else if (gs_travel_detour != 99)
	{
		gs_town_id = TOWN_ID_NONE;
	}

	if (g_pp20_index == 5)
	{
		memset((void*)g_dtp2, 0, 0xc0);

		memcpy((void*)(g_dtp2 + 0xc0), (void*)(gs_travel_map_ptr + 64000 + 2), 0x60);

		memcpy((void*)(g_dtp2 + 0x120),	(void*)g_palette_special, 0x60);

		for (i = 0; i < 64; i++)
		{
			pal_fade((int8_t*)g_dtp2 + 0xc0, (int8_t*)g_dtp2);
			pal_fade((int8_t*)g_dtp2 + 0x120, (int8_t*)g_dtp2 + 0x60);
			wait_for_vsync();
			set_palette((uint8_t*)g_dtp2 + 0xc0, 0x00, 0x20);
			set_palette((uint8_t*)g_dtp2 + 0x120, 0xe0, 0x20);
		}

		wait_for_vsync();
		set_palette((uint8_t*)g_dtp2, 0x80, 0x40);
		set_palette((uint8_t*)g_dtp2, 0x00, 0x20);

		do_fill_rect(g_vga_memstart, 0, 0, 319, 199, 0);
	}

	g_current_ani = g_town_loaded_town_id = g_pp20_index = -1;
	g_request_refresh = g_fading_state = 1;
	g_wallclock_update = wallclock_update_bak;

	return 0;
}
