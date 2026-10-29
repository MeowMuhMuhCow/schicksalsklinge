# Zuordnung Modul/Funktion -> Textdatei
# Eintrag: (datei-regex, funktions-regex, zugriff, textdatei, begruendung)
# Erste passende Regel gewinnt. textdatei None = bewusst nicht kommentieren.

DTX = {1: 'SHIP.DTX', 2: 'F046.DTX', 3: 'F051.DTX', 4: 'F061.DTX', 5: 'F076.DTX',
       6: 'F094.DTX', 7: 'F100.DTX', 8: 'F108.DTX', 9: 'F126.DTX', 10: 'F129.DTX',
       11: 'F131.DTX', 12: 'OBER.DTX', 13: 'PREM.DTX', 14: 'THORWAL.DTX', 15: 'FINAL.DTX'}

DNG_NAMES = {1: 'Totenschiff', 2: 'Verfallene Herberge', 3: 'Spinnenhoehle', 4: 'Wolfshoehle',
             5: 'Goblinhoehle', 6: 'Daspotaschatz', 7: 'Ruine des Schwarzmagiers', 8: 'Orkbehausung',
             9: 'Kultstaette des Namenlosen', 10: 'Drachenhort', 11: 'Piratenhoehle',
             12: 'Zwergenfeste Oberorken', 13: 'Verlassene Mine Prem', 14: 'Zwingfeste Thorwal',
             15: 'Hyggeliks Ruine'}


def feature_file(ev):
    """TRV_load_textfile() in seg109.c"""
    if ev in (37, 47, 100):
        return 'FEATURE9.LTX'
    if ev == 16:
        return 'FEATURE8.LTX'
    for lim, f in ((30, 1), (54, 2), (70, 3), (90, 4), (111, 5), (126, 6), (143, 7)):
        if ev < lim:
            return 'FEATURE%d.LTX' % f
    return 'FEATURE8.LTX'


RULES = [
    # ---------------- get_tx ----------------
    (r'seg002', r'copy_protection', 'get_tx', 'FIGHTTXT.LTX', 'load_tx(ARCHIVE_FILE_FIGHTTXT_LTX) am Funktionsanfang'),
    (r'seg005', r'.*', 'get_tx', 'FIGHTTXT.LTX', 'Kampf: do_fight() laedt FIGHTTXT.LTX (seg032)'),
    (r'seg025', r'game_options', 'get_tx', 'MAPTEXT.LTX', 'load_tx(ARCHIVE_FILE_MAPTEXT_LTX) direkt davor'),
    (r'seg025', r'show_storytext', 'get_tx', 'STORY.LTX', 'load_tx(ARCHIVE_FILE_STORY_LTX)'),
    (r'seg030', r'talk_switch', 'get_tx', 'DASPOTA.LTX', 'nur Zweig g_tlk_id==3 (DASPOTA1.TLK) -> Stadt-LTX von Daspota'),
    (r'seg031', r'.*', 'get_tx', 'TAVERN.TLK', 'Kneipengespraech: load_tlk(ARCHIVE_FILE_TAVERN_TLK) (seg060)'),
    (r'seg03[3-5]|seg04[13]', r'.*', 'get_tx', 'FIGHTTXT.LTX', 'Kampfsystem: do_fight() laedt FIGHTTXT.LTX'),
    (r'seg059', r'prolog_ghostship', 'get_tx', 'SHIP.DTX', 'load_tx(ARCHIVE_FILE_DNGS_DTX + 1) = SHIP.DTX'),
    (r'seg060', r'.*', 'get_tx', 'TAVERN.TLK', 'load_tlk(ARCHIVE_FILE_TAVERN_TLK) -> g_tx_index'),
    (r'seg06[34]', r'.*', 'get_tx', 'HAFEN.LTX', 'load_tx(ARCHIVE_FILE_HAFEN_LTX) in do_harbor'),
    (r'seg066', r'enter_location_daspota', 'get_tx', 'DASPOTA.LTX', 'Stadt Daspota, Stadt-LTX geladen'),
    (r'seg067', r'city_event_\d', 'get_tx', 'STRASSE.LTX', 'city_event_switch(): load_tx(ARCHIVE_FILE_STRASSE_LTX)'),
    (r'seg068', r'THO_', 'get_tx', 'THORWAL.LTX', 'Spezialgebaeude in Thorwal: Stadt-LTX THORWAL.LTX'),
    (r'seg069', r'THO_', 'get_tx', 'THORWAL.LTX', 'Spezialgebaeude in Thorwal: Stadt-LTX THORWAL.LTX'),
    (r'seg073', r'.*', 'get_tx', 'TAVERN.TLK', 'Kneipe: TAVERN.TLK geladen (seg060)'),
    (r'seg09[34]', r'.*', 'get_tx', 'MAPTEXT.LTX', 'Reisemodus: MAPTEXT.LTX (TRV_event laedt es danach wieder)'),
    (r'seg095', r'npc_', 'get_tx', 'NSC.LTX', 'load_tx(ARCHIVE_FILE_NSC_LTX)'),
    (r'seg099|seg10[0-4]|seg107', r'.*', 'get_tx', 'SPELLTXT.LTX', 'Zauber/Talente: load_tx(ARCHIVE_FILE_SPELLTXT_LTX) vor dem Handler'),
    (r'seg109|seg11[0-8]', r'.*', 'get_tx', 'FEATURE.LTX', 'TRV_load_textfile(): load_tx(ARCHIVE_FILE_FEATURE_LTX)'),
    # ---------------- get_tx2 ----------------
    (r'seg04[68]|seg050|seg065|seg106', r'.*', 'get_tx2', 'CHARTEXT.LTX', 'load_tx2(ARCHIVE_FILE_CHARTEXT_LTX) (Statusmenue/Stufenanstieg/Outro)'),
    (r'seg062', r'.*', 'get_tx2', 'WONDER.LTX', 'load_tx2(ARCHIVE_FILE_WONDER_LTX)'),
    (r'seg066', r'do_special_buildings', 'get_tx2', 'THORWAL1.LTX', 'Thorwal, Gebaeudetyp < 41 -> THORWAL1.LTX'),
    (r'seg068', r'THO_(eisenhof|imman|botschaft|bank)$', 'get_tx2', 'THORWAL1.LTX', 'Gebaeudetyp 32..37 < 41 -> THORWAL1.LTX'),
    (r'seg068', r'.*', 'get_tx2', 'THORWAL2.LTX', 'Gebaeudetyp >= 41 (Zeughaus, Magistrat, Akademie ...) -> THORWAL2.LTX'),
    (r'seg069', r'.*', 'get_tx2', 'THORWAL1.LTX', 'Gebaeudetyp 11..31 < 41 -> THORWAL1.LTX'),
    (r'seg070', r'.*', 'get_tx2', 'PHEX1.LTX', 'Phexcaer, Gebaeudetyp > 3 -> PHEX1.LTX'),
    (r'seg071', r'.*', 'get_tx2', 'PHEX2.LTX', 'Phexcaer, Gebaeudetyp <= 3 bzw. load_tx2(ARCHIVE_FILE_PHEX2_LTX) -> PHEX2.LTX'),
    (r'seg117', r'.*', 'get_tx2', 'WILD.LTX', 'pause_traveling(): load_tx2(ARCHIVE_FILE_WILD_LTX)'),
    (r'seg1(09|1[0-8])', r'TRV_swim2|TRV_a_path', 'get_tx2', 'FEATURE1.LTX', 'nur aus tevent_011 aufgerufen'),
    (r'seg1(09|1[0-8])', r'TRV_swimm|tevent_unused01', 'get_tx2', 'FEATURE4.LTX', 'aus tevent_071 (und tevent_unused01) aufgerufen, Texte passen'),
    # tevent_NNN -> dynamisch (siehe resolve)
]

# Dungeon-Module: Funktionspraefix DNGnn -> DTX
DNG_EXTRA = {
    'DNG_clear_corridor': 12,  # nur aus DNG12_handler
}
