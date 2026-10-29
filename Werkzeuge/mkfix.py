#!/usr/bin/env python3
"""mkfix.py - erzeugt die Schalter fuer die Fehlerbehebungen (Projekt Schicksalsklinge, nach dem Vorbild Sternenschweif).

Aufruf: python3 mkfix.py [PROJEKTORDNER]     (Vorgabe: Ordner ueber Werkzeuge/)

Quellen:
  Analyse/Fehler/Fehler_Raster.tsv   Nr, Titel, Sicherheit, Auswirkung (kritisch/mittel/egal), Exploit (ja/nein), Fix-Vorschlag
  Werkzeuge/fix_status.tsv           Nr <TAB> umgesetzt (ja/teilweise/nein) <TAB> Grund/Hinweis (von Hand gepflegt)
  Quelle/*.c, Quelle/*.asm           wo ein Schalter benutzt wird (#if FIX_nnnn / IF FIX_nnnn) und die Kommentare
                                     "/* Fix nnn: ... */" bzw. "; Fix nnn: ..." direkt dahinter (Spalte "Aenderung")
Ausgabe:
  Quelle/s_fix.h                     C-Header (in v302de.h eingebunden): Tabelle als Kommentar, Vorgaben je Schalter
  Quelle/s_fix.inc                   dasselbe fuer TASM (seg008/009/010/013, falls ein Schalter dort benutzt wird)
  Werkzeuge/fix_schalter.tsv         maschinenlesbare Gesamttabelle
  Bauen/FIXES.md                     Tabelle fuer Menschen

Stufen: FIX_BUGS 0 = Original (bytegleich), 1 = "wichtig" (Auswirkung kritisch oder mittel), 2 = "alles".
        FIX_EXPLOITS 1 = auch Fehler beheben, die dem Spieler nuetzen (Vorgabe 1, sobald FIX_BUGS > 0).
Jeder Schalter FIX_nnnn laesst sich einzeln uebersteuern (BCC -DFIX_0040=0, TASM /dFIX_0040=0).
"Kein Fehler" laut Raster (36 Nummern): kein Schalter.
"""
import csv, glob, os, re, sys

P = os.path.abspath(sys.argv[1]) if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
KEIN = {6, 23, 75, 103, 118, 125, 144, 154, 176, 219, 228, 236, 239, 241, 247, 257, 267, 272, 283, 290, 301, 307, 328,
        329, 330, 344, 363, 364, 376, 377, 378, 387, 388, 389, 408, 417}
ALIAS = {51: 50}     # Nr -> Nr, wenn zwei Nummern dieselbe Stelle meinen (Schalter folgt dem anderen)


def ascii_(t):
    """Kommentartexte in den DOS-Quellen nur ASCII (Umlaute umschreiben, typografische Zeichen ersetzen)."""
    for a, b in (('ä', 'ae'), ('ö', 'oe'), ('ü', 'ue'), ('Ä', 'Ae'), ('Ö', 'Oe'), ('Ü', 'Ue'), ('ß', 'ss'),
                 ('„', '"'), ('“', '"'), ('”', '"'), ('‚', "'"), ('‘', "'"), ('’', "'"), ('–', '-'), ('—', '-'),
                 ('…', '...'), ('×', 'x'), ('→', '->'), ('−', '-'), ('≥', '>='), ('≤', '<='), ('²', '^2')):
        t = t.replace(a, b)
    return t.encode('ascii', 'replace').decode('ascii').replace('*/', '* /')


def tsv(p):
    if not os.path.exists(p): return []
    return list(csv.reader(open(p, encoding='utf-8'), delimiter='\t', quoting=csv.QUOTE_NONE))


def main():
    raster = {}
    rows = tsv(os.path.join(P, 'Analyse', 'Fehler', 'Fehler_Raster.tsv'))
    if not rows:
        raise SystemExit('mkfix: Analyse/Fehler/Fehler_Raster.tsv fehlt')
    h = rows[0]
    for f in rows[1:]:
        if not f or not f[0].isdigit(): continue
        d = dict(zip(h, f))
        raster[int(f[0])] = dict(titel=d['titel'], sich=d['sicherheit'], ausw=d['auswirkung'],
                                 expl=d['exploit'].strip().lower() == 'ja', vorschlag=d['fix'])
    status = {}
    for f in tsv(os.path.join(P, 'Werkzeuge', 'fix_status.tsv')):
        if f and f[0].strip().isdigit():
            f = f + [''] * 3
            status[int(f[0])] = (f[1].strip(), f[2].strip())
    used = {}; aend = {}
    srcs = glob.glob(os.path.join(P, 'Quelle', '*.c')) + glob.glob(os.path.join(P, 'Quelle', '*.h')) \
        + glob.glob(os.path.join(P, 'Quelle', '*.asm'))
    for p in sorted(srcs):
        if os.path.basename(p).lower() in ('s_fix.h',): continue
        txt = open(p, encoding='latin1').read()
        for m in re.finditer(r'^\s*#?\s*(?:el)?if\b[^\n]*?\bFIX_(\d{4})\b', txt, re.M | re.I):
            for n in re.findall(r'\bFIX_(\d{4})\b', m.group(0)):
                used.setdefault(int(n), set()).add(os.path.basename(p).upper())
        for m in re.finditer(r'(?:/\*|;)\s*Fix (\d+)(?: \([^)]*\))?:\s*(.*?)\s*(?:\*/|$)', txt, re.M | re.S):
            t = ' '.join(m.group(2).replace('*', ' ').split())
            l = aend.setdefault(int(m.group(1)), [])
            if t and t not in l: l.append(t)
    out = []
    for nr in sorted(raster):
        r = raster[nr]
        if nr in KEIN:
            out.append(dict(nr=nr, titel=r['titel'], kat=r['ausw'], sich=r['sich'], expl=r['expl'], stufe='-',
                            um='kein Fehler', grund=status.get(nr, ('', 'laut Raster kein Fehler'))[1] or 'laut Raster kein Fehler',
                            aend='', dat=''))
            continue
        stufe = 'wichtig' if r['ausw'] in ('kritisch', 'mittel') else 'alles'
        src = used.get(ALIAS.get(nr, nr), set())
        um, grund = status.get(nr, ('', ''))
        um = um or ('ja' if src else 'nein')
        if not src and um != 'nein':
            print(f'mkfix: WARNUNG Nr. {nr} als "{um}" gefuehrt, aber kein #if FIX_{nr:04d} im Quelltext')
            um = 'nein'
        if src and um == 'nein':
            print(f'mkfix: WARNUNG Nr. {nr} als "nein" gefuehrt, aber FIX_{nr:04d} steht in {sorted(src)}')
        out.append(dict(nr=nr, titel=r['titel'], kat=r['ausw'], sich=r['sich'], expl=r['expl'], stufe=stufe, um=um,
                        grund=grund, aend='; '.join(aend.get(nr, []) or aend.get(ALIAS.get(nr, nr), [])), dat=' '.join(sorted(src))))
    # Gesamttabelle
    with open(os.path.join(P, 'Werkzeuge', 'fix_schalter.tsv'), 'w', encoding='utf-8', newline='\n') as f:
        f.write('nr\tschalter\ttitel\tsicherheit\tkategorie\texploit\tstufe\tumgesetzt\taenderung\tgrund\tdateien\n')
        for o in out:
            sw = '-' if o['stufe'] == '-' else 'FIX_%04d' % o['nr']
            f.write('\t'.join([str(o['nr']), sw, o['titel'], o['sich'], o['kat'], 'ja' if o['expl'] else 'nein',
                               o['stufe'], o['um'], o['aend'], o['grund'], o['dat']]) + '\n')
    # C-Header
    L = ['/* s_fix.h - Fehlerbehebungen per Schalter (Projekt Schicksalsklinge, erzeugt von Werkzeuge/mkfix.py - nicht von Hand aendern)',
         ' *',
         ' * ALLE BEHEBUNGEN SIND IM SPIEL UNGETESTET. Tester willkommen (siehe Bauen/FIXES.md).',
         ' *',
         ' * Stufenschalter (BCC -D..., Vorgabe: Original):',
         ' *   FIX_BUGS 0      Original, bytegleich mit BLADEM.EXE CD 3.02 de (Standard)',
         ' *   FIX_BUGS 1      "wichtig": alle Fehler mit Auswirkung kritisch oder mittel',
         ' *   FIX_BUGS 2      "alles": dazu alle Fehler mit Auswirkung egal',
         ' *   FIX_EXPLOITS 1  auch Fehler beheben, die dem Spieler nuetzen (Vorgabe 1, sobald FIX_BUGS > 0); 0 = drinlassen',
         ' * Einzelschalter: jeder FIX_nnnn laesst sich uebersteuern, z. B. -DFIX_0040=0 (aus) oder -DFIX_0004=1 (an).',
         ' * Keine Behebung aendert Strukturen, das Spielstandformat, das Datensegment oder die Lage der residenten Segmente.',
         ' *',
         ' * Schalter   Stufe    Kategorie Exploit umgesetzt  Titel',
         ' * ---------  -------  --------- ------- ---------  -----']
    for o in out:
        sw = 'FIX_%04d' % o['nr'] if o['stufe'] != '-' else '(%d)' % o['nr']
        L.append(ascii_(f" * {sw:<10} {o['stufe']:<8} {o['kat']:<9} {'ja' if o['expl'] else '-':<7} {o['um']:<10} {o['titel']}"))
    L += [' */', '#ifndef S_FIX_H', '#define S_FIX_H', '',
          '#ifndef FIX_BUGS', '#define FIX_BUGS 0', '#endif',
          '#ifndef FIX_EXPLOITS', '#if FIX_BUGS > 0', '#define FIX_EXPLOITS 1', '#else', '#define FIX_EXPLOITS 0', '#endif', '#endif',
          '', '/* Vorgaben je Stufe (X = Exploit: nur mit FIX_EXPLOITS) */',
          '#define FIX_STUFE_W  (FIX_BUGS >= 1)', '#define FIX_STUFE_A  (FIX_BUGS >= 2)',
          '#define FIX_STUFE_WX (FIX_BUGS >= 1 && FIX_EXPLOITS)', '#define FIX_STUFE_AX (FIX_BUGS >= 2 && FIX_EXPLOITS)', '']
    A = ['; s_fix.inc - Fehlerbehebungen per Schalter fuer TASM (erzeugt von Werkzeuge/mkfix.py, siehe s_fix.h)',
         'ifndef FIX_BUGS', 'FIX_BUGS = 0', 'endif', 'ifndef FIX_EXPLOITS', 'if FIX_BUGS gt 0', 'FIX_EXPLOITS = 1', 'else',
         'FIX_EXPLOITS = 0', 'endif', 'endif']
    asm_used = {n for n, s in used.items() if any(x.endswith('.ASM') for x in s)}
    for o in out:
        if o['stufe'] == '-': continue
        n = o['nr']; sw = 'FIX_%04d' % n
        if n in ALIAS:
            L += [f'#ifndef {sw}', f'#define {sw} FIX_{ALIAS[n]:04d}\t/* dieselbe Stelle */', '#endif']
            continue
        st = ('W' if o['stufe'] == 'wichtig' else 'A') + ('X' if o['expl'] else '')
        L += [f'#ifndef {sw}', f'#define {sw} FIX_STUFE_{st}', '#endif']
        if n in asm_used:
            cond = ('FIX_BUGS ge 1' if st[0] == 'W' else 'FIX_BUGS ge 2') + (' and FIX_EXPLOITS ne 0' if 'X' in st else '')
            A += [f'ifndef {sw}', f'if {cond}', f'{sw} = 1', 'else', f'{sw} = 0', 'endif', 'endif']
    # Platz in residenten Modulen: nie aufgerufene Funktionen entfallen, sobald eine Behebung im Modul aktiv ist
    # (residente Segmente duerfen nicht wachsen; Schalter FIX_PLATZ_SEGnnn, nur wenn das Modul ihn benutzt)
    for p in sorted(srcs):
        b = os.path.basename(p).upper()
        m = re.match(r'(SEG0\d\d)\.C$', b)
        if not m or ('FIX_PLATZ_' + m.group(1)) not in open(p, encoding='latin1').read(): continue
        nrs = sorted(n for n, s in used.items() if b in s)
        L += ['', f'/* {m.group(1)} ist resident: ohne Platz fuer die Behebungen entfallen dort nie aufgerufene Funktionen */',
              f'#ifndef FIX_PLATZ_{m.group(1)}',
              f'#define FIX_PLATZ_{m.group(1)} (' + ' || '.join('FIX_%04d' % n for n in nrs) + ')', '#endif']
    L += ['', '#endif', '']
    open(os.path.join(P, 'Quelle', 's_fix.h'), 'w', newline='\n').write('\n'.join(L))
    open(os.path.join(P, 'Quelle', 's_fix.inc'), 'w', newline='\r\n').write('\n'.join(A) + '\n')
    # FIXES.md fuer Menschen
    st = {}
    for o in out:
        st.setdefault(o['stufe'], {}).setdefault(o['um'], 0)
        st[o['stufe']][o['um']] += 1
    def zelle(t):
        return (t or '').replace('|', '/').replace('\n', ' ')
    M = ['# Fehlerbehebungen per Schalter (FIXES) - Die Schicksalsklinge', '',
         '**ALLE BEHEBUNGEN SIND IM SPIEL UNGETESTET.** Sie sind am Quelltext entworfen, uebersetzt, statisch geprueft '
         '(Datensegment, Lage der residenten Segmente, Overlay-Einspruenge: `Werkzeuge/fixcheck.py`) und starten in DOSBox, '
         'sind aber noch in keinem Spiel ausprobiert worden. **Tester willkommen** - Rueckmeldungen bitte mit Nummer.', '',
         'Quelle der Tabelle: `Werkzeuge/fix_schalter.tsv`, erzeugt von `Werkzeuge/mkfix.py` aus '
         '`Analyse/Fehler/Fehler_Raster.tsv`, `Werkzeuge/fix_status.tsv` und den Kommentaren `/* Fix n: ... */` im Quelltext.', '',
         '**Stufen:** `FIX_BUGS=1` ("wichtig") schaltet alle Behebungen der Kategorien *kritisch* und *mittel* ein, '
         '`FIX_BUGS=2` ("alles") zusaetzlich die Kategorie *egal*. `FIX_EXPLOITS=0` laesst die als Exploit markierten Fehler '
         '(die dem Spieler nuetzen) im Spiel; Vorgabe ist `FIX_EXPLOITS=1`, sobald `FIX_BUGS > 0`. Jeder Schalter `FIX_nnnn` '
         'laesst sich einzeln uebersteuern (`-DFIX_0040=0`). `FIX_BUGS=0` (Vorgabe) ergibt das bytegleiche Original '
         '(BLADEM.EXE CD 3.02 de, MD5 4ce035f95ead01a8b2553b4487544e5d).', '',
         '**Bauen:** `Bauen\\BAUEN.bat wichtig` bzw. `Bauen\\BAUEN.bat alles` -> `Build\\BLADEM_FIX_WICHTIG.EXE` bzw. '
         '`Build\\BLADEM_FIX_ALLES.EXE`; starten mit `BLADEM_FIX_WICHTIG_STARTEN.bat` / `BLADEM_FIX_ALLES_STARTEN.bat` '
         '(eigener Ordner, eigene Testspielstaende).', '',
         '**Grenzen:** Keine Behebung aendert Strukturen, das Spielstandformat, das Datensegment, die Spieldaten oder die Lage '
         'der residenten Segmente. Fehler, die das braeuchten, und Fehler mit unklarer Absicht (Balance) sind mit Grund '
         'als "nein" gefuehrt. Der Kopierschutz ist unberuehrt.', '',
         '**Spalten:** Stufe = ab welcher Stufe der Schalter an ist; umgesetzt = ja / teilweise / nein; "Aenderung / Grund" '
         'nennt die Kommentare an den `#if FIX_nnnn`-Stellen bzw. warum nicht oder nur teilweise behoben wurde.', '',
         '## Uebersicht', '', '| Stufe | ja | teilweise | nein | kein Fehler |', '|---|---|---|---|---|']
    for k, nm in (('wichtig', 'wichtig (kritisch + mittel)'), ('alles', 'alles (zusaetzlich egal)')):
        v = st.get(k, {})
        M.append(f"| {nm} | {v.get('ja', 0)} | {v.get('teilweise', 0)} | {v.get('nein', 0)} | - |")
    M.append(f"| - (laut Raster kein Fehler) | - | - | - | {st.get('-', {}).get('kein Fehler', 0)} |")
    M += ['', 'Nr. 51 ist dieselbe Stelle wie Nr. 50 (`FIX_0051` folgt `FIX_0050`). In den residenten Modulen (seg001..seg007) '
          'darf kein Segment laenger werden; in seg002 entfallen dafuer mit den Behebungen nie aufgerufene Funktionen '
          '(Schalter `FIX_PLATZ_SEG002`, aus den Schaltern des Moduls abgeleitet).', '',
          '## Tabelle', '', '| Nr | Titel | Kategorie | Exploit | Stufe | umgesetzt | Aenderung / Grund | Dateien |',
          '|---|---|---|---|---|---|---|---|']
    for o in out:
        txt = o['aend']
        if o['grund']: txt = (txt + ' - ' if txt and o['um'] != 'nein' else '') + o['grund'] if (o['um'] != 'ja' or not txt) else txt + ' (' + o['grund'] + ')'
        M.append(f"| {o['nr']} | {zelle(o['titel'])} | {o['kat']} | {'ja' if o['expl'] else 'nein'} | {o['stufe']} | {o['um']} | "
                 f"{zelle(txt)} | {o['dat']} |")
    os.makedirs(os.path.join(P, 'Bauen'), exist_ok=True)
    open(os.path.join(P, 'Bauen', 'FIXES.md'), 'w', encoding='utf-8', newline='\n').write('\n'.join(M) + '\n')
    # Statistik
    st = {}
    for o in out:
        st.setdefault(o['stufe'], {}).setdefault(o['um'], 0)
        st[o['stufe']][o['um']] += 1
    print('mkfix:', len(out), 'Nummern;', ', '.join(f"{k}: {v}" for k, v in sorted(st.items())))
    return out


if __name__ == '__main__':
    main()
