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
