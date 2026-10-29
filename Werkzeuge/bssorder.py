#!/usr/bin/env python3
"""bssorder.py QUELLORDNER - BSS-Reihenfolge fuer den C-Modus herstellen.

Borland C 3.1 legt im C-Modus das BSS eines Moduls in umgekehrter Reihenfolge der ERSTEN Deklaration an
(gemessen, siehe Analyse). Damit das BSS wie im Original aufsteigend liegt, muss die erste Deklaration
absteigend sein. Das Werkzeug
  1. sammelt alle nicht vorbelegten Variablen auf Dateiebene (Definition ohne '=', mit // ds:0x...),
     die im Borland-Bau gelten,
  2. entfernt 'static' davor (sie werden in datseg.h deklariert),
  3. schreibt in datseg.h hinter die Marke '/* BSS starts at 0xbc38' einen Block mit allen
     extern-Deklarationen, absteigend nach Adresse, und loescht andere (auch auskommentierte)
     extern-Zeilen derselben Namen in datseg.h.
"""
import re, sys, glob, os
D = sys.argv[1]
MARK_A = '/* BSS: generiert von bssorder.py */'
MARK_E = '/* BSS: Ende des generierten Blocks */'

def active_lines(path):
    """liefert (zeilennr, zeile, aktiv) mit einfacher #if-Auswertung fuer den Borland-Bau"""
    st = []
    def ev(c):
        c = c.strip()
        if re.fullmatch(r'defined\s*\(?\s*__BORLANDC__\s*\)?', c): return True
        if re.fullmatch(r'!\s*defined\s*\(?\s*__BORLANDC__\s*\)?', c): return False
        if 'M302de_' in c: return c.lstrip().startswith('!')
        return True
    out = []
    for i, l in enumerate(open(path, encoding='latin1').read().split('\n')):
        s = l.strip()
        if s.startswith('#if'):
            if s.startswith('#ifdef'): v = ev('defined ' + s[6:])
            elif s.startswith('#ifndef'): v = not ev('defined ' + s[7:])
            else: v = ev(s[3:])
            st.append(v); out.append((i, l, False)); continue
        if s.startswith('#else'): st[-1] = not st[-1]; out.append((i, l, False)); continue
        if s.startswith('#endif'): st.pop(); out.append((i, l, False)); continue
        out.append((i, l, all(st)))
    return out

defs = []   # (adr, name, decl, modul, datei, zeile)
for f in sorted(glob.glob(os.path.join(D, 'seg*.c'))) + [os.path.join(D, 'datseg.c')]:
    mod = 'seg002' if f.endswith('datseg.c') else os.path.basename(f)[:6]
    for i, l, act in active_lines(f):
        if not act or not l or l[0] in ' \t/#{}*': continue
        code, _, com = l.partition('//')
        m = re.search(r'ds:0x([0-9a-fA-F]+)', com)
        if not m or '=' in code or code.startswith('extern') or not code.rstrip().endswith(';'): continue
        if '(' in code and '(*' not in code: continue      # Prototyp
        decl = re.sub(r'^static\s+', '', code.strip().rstrip(';').strip())
        nm = re.search(r'\(\s*(?:far\s*)?\*\s*(\w+)\s*\)' if '(*' in decl else r'(\w+)\s*(\[[^\]]*\]\s*)*$', decl)
        if not nm: print('?', f, i+1, l); continue
        defs.append((int(m[1], 16), nm[1], decl, mod, f, i))

# 2. static entfernen
byfile = {}
for d in defs: byfile.setdefault(d[4], []).append(d[5])
for f, rows in byfile.items():
    L = open(f, encoding='latin1').read().split('\n')
    for i in rows: L[i] = re.sub(r'^static\s+', '', L[i])
    open(f, 'w', encoding='latin1').write('\n'.join(L))

# 3. datseg.h
h = os.path.join(D, 'datseg.h')
H = open(h, encoding='latin1').read().split('\n')
names = {d[1] for d in defs}
old = {}
keep = []
inblock = False
for l in H:
    if l.strip() == MARK_A: inblock = True; continue
    if l.strip() == MARK_E: inblock = False; continue
    if inblock: continue
    m = re.match(r'\s*(//\s*)?extern\b(.*);\s*(//.*)?$', l)
    if m:
        code = m[2]
        nm = re.search(r'\(\s*(?:far\s*)?\*\s*(\w+)\s*\)', code) or re.search(r'(\w+)\s*(\[[^\]]*\]\s*)*$', code.strip())
        if nm and nm[1] in names:
            if m[3]: old[nm[1]] = m[3]
            continue
    keep.append(l)
k = next(i for i, l in enumerate(keep) if l.startswith('/* BSS starts at 0xbc38'))
blk = [MARK_A,
       '/* Alle nicht vorbelegten Variablen (BSS), je Modul ABSTEIGEND nach Adresse.',
       ' * Borland C legt das BSS eines Moduls in umgekehrter Reihenfolge der ersten Deklaration an.',
       ' * Diese extern-Zeilen sind die erste Deklaration, also liegt das BSS danach aufsteigend wie im',
       ' * Original. Attics eigener Header muss die Variablen ebenso absteigend gefuehrt haben. */']
cur = None
for adr, nm, decl, mod, f, i in sorted(defs, key=lambda d: (d[3], -d[0])):
    if mod != cur: blk.append(f'/* {mod} */'); cur = mod
    c = old.get(nm, f'// ds:0x{adr:04x}; {mod}')
    blk.append(f'extern {decl};\t{c}')
blk.append(MARK_E)
keep[k+1:k+1] = blk
open(h, 'w', encoding='latin1').write('\n'.join(keep))
print(len(defs), 'BSS-Variablen,', sum(1 for d in defs), 'Deklarationen erzeugt,', len(old), 'alte Kommentare uebernommen')
