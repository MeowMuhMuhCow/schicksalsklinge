#!/usr/bin/env python3
"""probenlog.py - zeigt das Probenlog der Bugfix-Fassung (PROBEN.CSV) als Klartext.
   python3 probenlog.py PROBEN.CSV          ganze Datei ausgeben
   python3 probenlog.py --live PROBEN.CSV   laufend mitlesen (wie ein Konsolenfenster neben dem Spiel)
Das Spiel schreibt nur Zahlen; dieses Skript macht daraus lesbare Zeilen."""
import os, sys, time

SPIEL = 'Schicksalsklinge'
EIG = ["MU", "KL", "CH", "FF", "GE", "IN", "KK", "AG", "HA", "RA", "GG", "TA", "NG", "JZ"]
TALENT = ["Waffenlos", "Hiebwaffen", "Stichwaffen", "Schwerter", "Äxte", "Speere", "Zweihänder", "Schusswaffen",
          "Wurfwaffen", "Akrobatik", "Klettern", "Körperbeherrschung", "Reiten", "Schleichen", "Schwimmen",
          "Selbstbeherrschung", "Tanzen", "Verstecken", "Zechen", "Bekehren", "Betören", "Feilschen", "Gassenwissen",
          "Lügen", "Menschenkenntnis", "Schätzen", "Fährtensuchen", "Fesseln", "Orientierung", "Pflanzenkunde",
          "Tierkunde", "Wildnisleben", "Alchimie", "Alte Sprachen", "Geographie", "Geschichte", "Götter/Kulte",
          "Kriegskunst", "Lesen", "Magiekunde", "Sprachen", "Abrichten", "Fahrzeuge", "Falschspiel", "Heilen Gift",
          "Heilen Krankheiten", "Heilen Wunden", "Musizieren", "Schlösser", "Taschendiebstahl", "Gefahrensinn",
          "Sinnesschärfe"]
ZAUBER = ["-", "Beherrschung brechen", "Destructibo", "Gardianum", "Illusionen zerstören", "Verwandlung beenden",
          "Band und Fessel", "Bannbaladin", "Böser Blick", "Große Gier", "Große Verwirrung", "Herr der Tiere",
          "Horriphobus", "Magischer Raub", "Respondami", "Sanftmut", "Somnigravis", "Zwingtanz", "Furor Blut",
          "Geister bannen", "Geister rufen", "Heptagon", "Krähenruf", "Skelettarius", "Elementare", "Nihilatio",
          "Solidirid", "Axxeleratus", "Foramen", "Motoricus", "Spurlos", "Transversalis", "Über Eis", "Balsam",
          "Hexenspeichel", "Klarum Purum", "Ruhe Körper", "Tiere heilen", "Adleraug", "Analüs", "Eigenschaften",
          "Exposami", "Odem Arcanum", "Penetrizzel", "Sensibar", "Chamaelioni", "Duplicatus", "Harmlos", "Hexenknoten",
          "Blitz", "Ecliptifactus", "Eisenrost", "Fulminictus", "Ignifaxius", "Plumbumbarum", "Radau",
          "Saft, Kraft, Monstermacht", "Scharfes Auge", "Hexenblick", "Nekropathia", "Adler, Wolf", "Arcano",
          "Armatrutz", "CH steigern", "Feuerbann", "FF steigern", "GE steigern", "IN steigern", "KK steigern",
          "KL steigern", "MU steigern", "Mutabili", "Paralü", "Salander", "See und Fluss", "Visibili", "Abvenenum",
          "Aeolitus", "Brenne", "Claudibus", "Dunkelheit", "Erstarre", "Flim Flam", "Schmelze", "Silentium",
          "Sturmgebrüll"]
GZAUBER = ["-", "Verwandlung beenden", "Bannbaladin", "Böser Blick", "Horriphobus", "Axxeleratus", "Balsam", "Blitz",
           "Eisenrost", "Fulminictus", "Ignifaxius", "Plumbumbarum", "Saft, Kraft, Monstermacht", "Armatrutz", "Paralü"]
GEGNER = ["", "Oger", "Ork", "Goblin", "Heshthot", "Säbelzahntiger", "Skelett", "Waldschrat", "Mumie", "Harpyie",
          "Waldspinne", "Großer Schröter", "Pirat", "Steuermann", "Kapitän", "Streuner", "Zombie", "Heshthot", "Druide",
          "Druide", "Thorwaler", "Thorwaler", "Thorwaler", "Thorwaler", "Goblin", "Goblin", "Ork", "Ork", "Pirat",
          "Pirat", "Streuner", "Streuner", "Streuner", "Skelett", "Folterknecht", "Grimwolf", "Grimwolf",
          "Höhlenspinne", "Höhlenspinne", "Höhlenspinne", "Krieger", "Stadtgardist", "Stadtgardist", "Stadtgardist",
          "Kriegsveteran", "Mumie", "Magier", "Magier", "Zauberer", "Zauberer", "Zwerg", "Lindwurm", "Höhlendrache",
          "Kultist", "Kultist", "Kultist", "Kultist", "Priester", "Priester", "Hohepriester", "Estorik", "Piratenboss",
          "Pirat", "Magier", "Zwergenveteran", "Räuber", "Räuber", "Räuber", "Räuber", "Räuber", "Gorah",
          "Steppenhund", "Orkchampion", "Pirat", "Stadtgardist", "Waldlöwe", "Spinnendämon", "Heshthot"]
MONAT = ["", "Praios", "Rondra", "Efferd", "Travia", "Boron", "Hesinde", "Firun", "Tsa", "Phex", "Peraine",
         "Ingerimm", "Rahja", "Namenlose Tage"]
KRANKHEIT = ["keine", "Wundfieber", "Dumpfschädel", "Blaue Keuche", "Paralyse", "Schlachtenfieber", "Frostschäden", "Tollwut"]
BEGEGNUNG = ["Kampf WILD1 (2 bis 7 Goblins)", "Kampf WILD2 (1 bis 3 Oger)", "Karenjagd",
             "Kampf WILD3 (1 bis 2 Säbelzahntiger)", "Kvillotter", "Kampf WILD4 (2 bis 7 Grimwölfe)",
             "Kampf WILD4 (4 bis 6 Grimwölfe)", "Kampf WILD5 (2 bis 3 Steppenhunde)", "Nashornjagd",
             "Wildschweinjagd", "Kampf WILD6 (1 bis 3 Waldspinnen)", "Bisonjagd",
             "Kampf WILD7 (1 bis 3 Harpyien)", "Höhlenbär"]
BEGEGNUNG_KAMPF = {0, 1, 3, 5, 6, 7, 10, 12}


KAMPF = ["ZUFALL1_LAND", "LAND_FIGHT1_1", "ZUFALL2_LAND", "ZUFALL3_LAND", "ZUFALL4_LAND", "SHIP3", "SHIP4", "SHIP5", "SHIP6", "SHIP8", "SHIP9", "SHIP12", "SHIP14", "SHIP15", "SHIP17", "SHIP18", "SHIP19", "SHIP21", "SHIP22", "SHIP23A", "SHIP23B", "SHIP24", "SHIP10", "F061_2", "F061_3", "F061_4B", "F061_4A", "F061_5", "F061_6A", "F108_1", "F108_2", "F108_3A", "F108_3B", "F108_3C", "F108_4", "F108_6", "F108_9", "F108_7", "F051_05_4", "F051_05_3", "F051_05_2", "F051_05_1", "F051_07", "F051_09", "F051_13", "F108_15", "F051_15", "F051_16", "F051_16_1", "F051_03", "F051_04", "F051_17", "F051_18", "F051_19", "F051_20_2", "F051_18_3", "F046_01", "F046_04", "F046_06", "F046_07", "F046_10", "F046_12", "F046_13", "F046_15", "F046_16", "F046_18", "F046_22", "F046_24", "F046_25", "F046_26", "F046_26_5", "F046_27", "F046_28", "F046_31", "F076_04", "F076_06", "F076_07", "F076_10", "F076_11", "F076_13", "F094_02", "F094_05", "F094_10", "F094_13", "F094_17", "F094_19", "F094_22", "F100_01", "F100_03", "F100_05", "F100_12", "F100_13", "F126_03", "F126_07", "F126_08", "F126_09", "F126_11", "F126_12", "F126_13", "F126_17", "F126_18", "F126_20", "F126_22", "F126_23", "F126_25", "F126_27", "F126_28", "F129_05", "F129_08", "F129_09", "F129_18", "F129_21", "F129_23", "F129_24", "F129_25", "F129_27", "F129_28", "F129_29", "F129_30", "F131_01A", "F131_01B", "F131_04", "F131_05", "F131_06", "F131_07", "F131_08", "F131_10", "F131_11_1", "F131_14A", "F131_14B", "F131_16", "DFIN12", "DFIN16", "DFIN18A", "DFIN18B", "DFIN26", "DFIN27A", "DFIN27B", "DFIN28", "DFIN30", "DPRE10_1", "DOBE07", "DOBE09", "DOBE11", "DOBE20", "DOBE19", "DOBE22", "DTHO03", "DTHO05", "DTHO06", "DTHO09", "DTHO10", "DTHO13", "DTHO15", "DTHO16", "DTHO18", "DTHO19", "DTHO20", "DTHO20_1", "DTHO21A", "DTHO21B", "DTHO23", "DTHO25", "DTHO27", "DTHO43", "DTHO48_1", "DTHO49", "DTHO50", "DTHO53", "DTHO55", "DTHO56", "DTHO57", "DTHO57_1", "DTHO58", "DTHO59", "DTHO60", "DTHO61", "F031", "F035", "F046", "F064", "F066", "F074", "F075_A", "F075_B", "F075_C", "F077", "F080", "F084", "F099", "F101", "F122", "F144", "DASP1A", "DASP1B", "DASP2", "DASP3", "DASP4", "DASP5", "DASP6A", "DASP6B", "DASP7", "DASP8", "DASP9", "DASP10", "DASP11", "DASP12A", "DASP12B", "DASP13", "DASP14", "DASP16", "DASP17", "DASP18", "THOR8", "CITYFIGHT1", "CITYFIGHT2", "CITYFIGHT3", "CAMPFIGHT1", "CAMPFIGHT2", "CAMPFIGHT3", "SHIP10_1", "F061_6B", "F051_16A", "F051_16B", "F051_14A", "F051_14B", "F051_117", "F129_124", "F131_08A", "F131_08B", "F031_14A", "DTH021A", "CAMPFIGHT4", "SHIP10_2", "F051_02", "F051_5", "PHEX3", "PHEX22", "PHEX23", "PHEX24", "F131_01", "F129_08A", "DTHO14", "WILD1", "WILD2", "WILD3", "WILD4", "WILD5", "WILD6", "WILD7", "WILD8", "S001", "F051_14C", "F129_17", "DCAMPFIGHT1", "DCAMPFIGHT2", "DCAMPFIGHT3", "DCAMPFIGHT4"]


def nm(liste, i):
    return liste[i] if 0 <= i < len(liste) else f'Nr. {i}'


def gegner(i):
    return f'Gegner Nr. {i}' if not (0 < i < len(GEGNER)) else GEGNER[i]


def zuschlag(z):
    return '' if z == 0 else (f' erschwert um {z}' if z > 0 else f' erleichtert um {-z}')


def ergebnis3(r):
    if r == -99:
        return 'PATZER (zweimal 20)'
    return f'gelungen, {r - 1} Punkte übrig' if r > 0 else f'misslungen, {1 - r} Punkte zu wenig'


def stufe(name, w):
    """Art 8: Wurf beim Stufenanstieg (Typ, Nr., alter Wert, Wurf, Ergebnis)"""
    typ, nr, alt = w[0], w[1], w[2]
    if typ in (1, 2):
        was = f'{nm(TALENT, nr)} (TaW {alt})' if typ == 1 else f'{nm(ZAUBER, nr)} (ZfW {alt})'
        t = f'{name}: Stufenanstieg, {was}: Steigerungswurf {w[3]} ({"3–18" if alt >= 11 else "2–12"}), muss über {alt} liegen'
        return t + (f' → gelungen, jetzt {alt + 1}' if w[4] else ' → misslungen')
    if typ == 3:
        return f'{name}: Stufenanstieg, {nm(EIG, nr)} +1 ({alt} → {alt + 1})'
    if typ == 4 and len(w) >= 7:
        e = nm(EIG, nr)
        t = f'{name}: Stufenanstieg, {e} senken ({e} {alt}): W20 = {w[3]}, {w[4]}, {w[5]}, einer muss höchstens {alt} sein'
        return t + (f' → gelungen, jetzt {alt - 1}' if w[6] else ' → misslungen')
    if typ == 5:
        if w[4] > w[3]:
            return f'{name}: Stufenanstieg, W6 = {w[3]} + 2 → {w[4]} Punkte für LE und AE (LE vorher {alt})'
        return f'{name}: Stufenanstieg, W6 = {w[3]} → LE {alt} → {alt + w[4]}'
    if typ == 6:
        return f'{name}: Stufenanstieg, 10 Zaubersteigerungen gegen AE getauscht: Wurf {w[3]} (3–8) → AE {alt} → {alt + w[4]}'
    return f'{name}: Stufenanstieg (Typ {typ})'


def wache(i):
    return f'in der {i + 1}. Wache' if i >= 0 else ''


def ereignis(name, w):
    """Art 9: Zufallswurf des Spiels (Kampf/Gegnerzahl, Überfall im Lager, Land-/Seereise, Krankheitsprobe)"""
    typ = w[0]
    if typ == 1 and len(w) >= 5:
        k = nm(KAMPF, w[1]) or f'Nr. {w[1]}'
        if w[2] > 0:
            return f'Kampf {k}: Gegnerzahl gewürfelt, höchstens {w[2]} → {w[4]} von {w[3]} Gegnern der Kampfliste kämpfen mit'
        return f'Kampf {k}: feste Kampfliste → {w[3]} Gegner'
    if typ == 2 and len(w) >= 5:
        t = f'Nachtlager: Überfallwurf W100 = {w[1]}, Überfall bei weniger als {w[2]} ({"keine Wache" if w[2] == 60 else "mit Wache"})'
        if w[4] >= 0: return t + f' → ÜBERFALL {wache(w[4])}'
        if w[3] and w[1] < w[2]: return t + ' → kein Überfall, Travias Segen schützt'
        return t + ' → kein Überfall'
    if typ == 3 and len(w) >= 5:
        t = f'Lager im Dungeon ({w[3]} Std.): Überfallwurf W100 = {w[1]}, Überfall bei weniger als {w[2]}' + \
            (' (ohne Wache: 4 × Stunden)' if w[3] and w[2] == 4 * w[3] else '')
        return t + (f' → ÜBERFALL {wache(w[4])}' if w[4] >= 0 else ' → kein Überfall')
    if typ in (4, 5) and len(w) >= 4:
        was = 'Zufallsbegegnung' if typ == 4 else 'Kampf'
        return f'Reise: {was}{"s" if typ == 4 else ""}wurf W100 = {w[1]}, {was} bei höchstens {w[2]} → {"JA" if w[3] else "nein"}'
    if typ in (6, 7, 8) and len(w) >= 4:
        was = {6: 'Totenschiff', 7: 'Krakenmolch', 8: 'Piratenangriff'}[typ]
        return f'Seereise: Wurf auf {was} W100 = {w[1]}, {was} bei höchstens {w[2]} → {"JA" if w[3] else "nein"}'
    if typ == 9 and len(w) >= 5:
        kr = nm(KRANKHEIT, w[1])
        wer = f'{name}: ' if name else ''
        return f'{wer}Krankheitsprobe auf {kr}: W100 = {w[2]}, Ansteckung bei höchstens {w[3]} → {"ANGESTECKT" if w[4] else "nicht angesteckt"}'
    if typ == 10 and len(w) >= 4:
        wer = f'{name}: ' if name else ''
        return f'{wer}Gardianum-Schutzschirm ({w[1]} AE eingesetzt, +{w[2]}) → fängt bis zu {w[3]} SP ab'
    if typ == 11 and len(w) >= 6:
        wer = f'{name}: ' if name else ''
        if w[1] > 0:
            return f'{wer}Sturz in eine Grube – Wurf W{w[2]} = {w[1]} → {w[3]} von {w[2]} Helden stürzen, verliert {w[4]} LE (1W{w[5]})'
        return f'{wer}Sturz im Dunkeln – die ganze Gruppe ({w[3]}) stürzt, verliert {w[4]} LE (1W{w[5]})'
    if typ == 12 and len(w) >= 6:
        if w[3] < 0:
            return f'Reise: Art der Begegnung (Landschaftstyp {w[1]}): W100 = {w[2]} → keine'
        t = f'Reise: Art der Begegnung (Landschaftstyp {w[1]}): W100 = {w[2]} → {nm(BEGEGNUNG, w[3])}'
        if w[3] in BEGEGNUNG_KAMPF:
            t += f', Kampfwurf W100 = {w[4]}, Kampf bei höchstens 33 → {"entfällt" if w[5] else "KAMPF"}'
        return t
    if typ == 13 and len(w) >= 4:
        return f'Olvir/Treborn: W100 {w[1]} (bis {w[2]}) → Treffen {"ja" if w[3] else "nein"} (ab dem 4. Monat der Queste)'
    if typ == 14 and len(w) >= 6:
        wer = f'{name}: ' if name else ''
        return (f'{wer}Regeneration: Lager {w[1]}, W6 LE {w[2]}, W6 AE {w[3]} → LE {w[4]}, AE {w[5]}'
                ' (vor Tausch/Verdopplung)')
    if typ == 15 and len(w) >= 4:
        wer = f'{name}: ' if name else ''
        vorher = {-1: 'krank', 0: 'gesund', 1: 'Genesung'}.get(w[2], str(w[2]))
        return (f'{wer}Wundfieber nach Wundheilung: W20 {w[1]} (bis 7), vorher {vorher}'
                f' → neu begonnen {"ja" if w[3] else "nein"}')
    if typ == 16 and len(w) >= 10:
        return zauberschaden(name, w)
    return f'Ereignis (Typ {typ})'


def zauberschaden(name, w):
    """Art 9, Typ 16: Schaden durch einen Zauber (Held oder Gegner zaubert; Feuerbann/Gardianum fangen ab)"""
    zp = nm(GZAUBER, w[1]) if w[2] else nm(ZAUBER, w[1])
    wer = f'{zp} ({gegner(w[3])})' if w[2] else f'{name}: {zp}'
    if w[4] == 1:
        ziel = (name or 'einen Helden') if w[2] else f'Held Nr. {w[5]}'
    else:
        ziel = gegner(w[5])
    t = f'{wer} auf {ziel}: {w[6]} SP'
    if w[7] > 0: t += f', Feuerbann halbiert (−{w[7]})'
    if w[8] > 0: t += f', Gardianum fängt {w[8]} ab'
    return t + f' → {w[9]} SP am Ziel'


def gegnerzauber(name, w):
    """Art 10: Gegnerzauber im Kampf (Gegner, Zauber, Zielart, Zielgegner, 3 Eig, 3 Werte, Zuschlag, 3 W20, Ergebnis, Wirkung)"""
    mon, sp, ztyp, zid = w[0], w[1], w[2], w[3]
    a, wv, z, d, r = w[4:7], w[7:10], w[10], w[11:14], w[14]
    w1, w2, w3, w4, w5 = w[15:20]
    ziel = 'auf sich selbst' if ztyp == 0 else (f'auf {name or "einen Helden"}' if ztyp == 1 else f'auf {gegner(zid)}')
    eig = '/'.join(nm(EIG, x) for x in a) + ' (' + '/'.join(str(x) for x in wv) + ')'
    wuerfe = ', '.join(str(x) for x in d if x)
    zs = zuschlag(z) or ' ohne Zuschlag'
    t = f'{gegner(mon)} zaubert {nm(GZAUBER, sp)} {ziel} → Probe auf {eig},{zs}: {wuerfe} → {ergebnis3(r)}'
    if r <= 0:
        return t
    if w1 == -9:
        return t + ' → Magie durch Rondra gesperrt'
    if sp in (9, 10):
        abg = []
        if w3 > 0: abg.append(f'{w3} durch Feuerbann')
        if w4 > 0: abg.append(f'{w4} durch Gardianum (Schild noch {w5})')
        if abg:
            return t + f' → {w1} SP, {" und ".join(abg)} abgefangen → {w2} SP'
        return t + f' → {w2} SP'
    if sp == 1:
        wk = {1: 'Versteinerung aufgehoben', 2: 'Pilzverwandlung aufgehoben', -1: 'nicht genug AE (wirkungslos)'}.get(w1, 'Ziel nicht verwandelt (wirkungslos)')
    elif sp == 2: wk = 'Ziel greift nicht mehr an'
    elif sp == 3: wk = 'Ziel kämpft gegen die eigene Gruppe'
    elif sp == 4: wk = 'Ziel flieht verängstigt'
    elif sp == 5: wk = 'Attacken +1, AT +1, PA +1, BP ×2'
    elif sp == 6: wk = f'+{w1} LE geheilt' if w1 > 0 else 'keine Heilung (0 LE)'
    elif sp == 7: wk = f'{w1} Runden geblendet'
    elif sp == 8: wk = 'Waffe zerbrochen' if w1 > 0 else 'keine zerbrechliche Waffe (wirkungslos)'
    elif sp == 11: wk = f'AT -{w1}'
    elif sp == 12: wk = f'AT +{w1}, PA -{w1}, TP +{w1} ({w2} AE)'
    elif sp == 13: wk = f'RS +{w1} ({w2} AE)'
    elif sp == 14: wk = 'Ziel versteinert'
    else: wk = ''
    return t + (f' → {wk}' if wk else '')


class Leser:
    """macht aus Zahlenzeilen Klartext; eine Talent- oder Zauberzeile (Art 4/5) kuendigt die folgende Dreifachprobe an"""
    def __init__(self):
        self.vorspann = None   # (name, text) der angekuendigten Probe

    def zeile(self, roh):
        f = roh.strip().split(';')
        if len(f) < 6 or not f[0].lstrip('-').isdigit():
            return None
        try:
            art, tag, mon, jahr, minute = (int(x) for x in f[:5])
            name = f[5]
            w = [int(x) for x in f[6:] if x.strip() != '']
        except ValueError:
            return None
        zeit = f'{tag:2d}. {nm(MONAT, mon)} {jahr} Hal, {minute // 60:02d}:{minute % 60:02d}'
        txt = self.text(art, name, w)
        return None if txt is None else f'{zeit} | {txt}'

    def text(self, art, name, w):
        if art == 1 and len(w) >= 5:
            e, wert, z, wurf, r = w[:5]
            t = f'{name}: {nm(EIG, e)}-Probe{zuschlag(z)}: W20 = {wurf} gegen {nm(EIG, e)} {wert}'
            return t + (' → PATZER (20)' if r == -99 else (f' → gelungen ({r - 1} übrig)' if r > 0 else ' → misslungen'))
        if art == 3 and len(w) >= 11:
            a, wv, z, d, r = w[0:3], w[3:6], w[6], w[7:10], w[10]
            eig = '/'.join(nm(EIG, x) for x in a) + ' (' + '/'.join(str(x) for x in wv) + ')'
            wuerfe = ', '.join(str(x) for x in d if x)
            if self.vorspann and self.vorspann[0] == name:
                t = f'{self.vorspann[1]} → Probe auf {eig}, gesamt{zuschlag(z) or " ohne Zuschlag"}: {wuerfe} → {ergebnis3(r)}'
            else:
                t = f'{name}: Probe auf {eig}{zuschlag(z)}: {wuerfe} → {ergebnis3(r)}'
            self.vorspann = None
            return t
        if art == 4 and len(w) >= 6:
            ta, wert, z, fern, wurf, r = w[:6]
            if fern:
                self.vorspann = None
                t = f'{name}: {nm(TALENT, ta)}{zuschlag(z)}: W20 = {wurf} gegen Zielwert {wert}'
                if r == -1: return t + ' → PATZER (20)'
                if r == 99: return t + ' → Glückswurf (1)'
                return t + (f' → getroffen ({r - 1} übrig)' if r > 0 else ' → daneben')
            self.vorspann = (name, f'{name}: {nm(TALENT, ta)} (Talentwert {wert}){zuschlag(z)}')
            return None
        if art == 5 and len(w) >= 3:
            sp, zfw, z = w[:3]
            self.vorspann = (name, f'{name} zaubert {nm(ZAUBER, sp)} (Zauberfertigkeit {zfw}){zuschlag(z)}')
            return None
        if art in (6, 7) and len(w) >= 4:
            wert, wurf, mon, rtg = w[:4]
            self.vorspann = None
            kurz = 'AT' if art == 6 else 'PA'
            erf = ('trifft' if art == 6 else 'pariert') if (wurf <= wert and wurf != 20) else \
                  ('verfehlt' if art == 6 else 'pariert nicht')
            if wurf == 20: erf += ' (20: Patzer möglich)'
            probe = f'W20 = {wurf} gegen {kurz} {wert} → {erf}'
            if art == 6:
                if rtg == 0: wer = f'{name} attackiert {"einen Helden" if mon < 0 else gegner(mon)}'
                elif rtg == 1: wer = f'{gegner(mon)} attackiert {name}'
                else: wer = f'{gegner(mon)} attackiert einen anderen Gegner'
            else:
                if rtg == 0: wer = f'{name} pariert' if mon < 0 else f'{gegner(mon)} pariert gegen {name}'
                elif rtg == 1: wer = f'{name} pariert gegen {gegner(mon)}'
                else: wer = f'Ein Gegner pariert gegen {gegner(mon)}'
            return f'{wer}: {probe}'
        if art == 8 and len(w) >= 5:
            self.vorspann = None
            return stufe(name, w)
        if art == 9 and len(w) >= 4:
            self.vorspann = None
            return ereignis(name, w)
        if art == 10 and len(w) >= 20:
            self.vorspann = None
            return gegnerzauber(name, w)
        return f'{name}: unbekannte Zeile (Art {art})'


def main():
    a = sys.argv[1:]
    live = '--live' in a
    a = [x for x in a if x != '--live']
    if not a:
        sys.exit(__doc__)
    pfad = a[0]
    L = Leser()
    if not live:
        for roh in open(pfad, encoding='latin-1'):
            t = L.zeile(roh)
            if t: print(t)
        return
    print(f'Probenlog {SPIEL} - liest {pfad} mit (Strg+C beendet)\n')
    while not os.path.exists(pfad):
        time.sleep(0.5)
    with open(pfad, encoding='latin-1') as f:
        rest = ''
        while True:
            s = f.readline()
            if not s:
                time.sleep(0.3)
                continue
            rest += s
            if not rest.endswith('\n'):
                continue
            t = L.zeile(rest); rest = ''
            if t:
                print(t, flush=True)


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        pass
