#!/usr/bin/env python3
"""umbenennen.py QUELLORDNER LISTE.tsv [--pruefen] - globale Umbenennungen (Spalten: alt, neu, ...).
Ersetzt ganze Bezeichner in *.c, *.h und (mit fuehrendem _) in *.asm. Prueft vorher:
neuer Name <= 31 Zeichen, nicht schon vorhanden, alt kommt vor. --pruefen: nur pruefen."""
import sys, re, glob, os, csv
D, L = sys.argv[1], sys.argv[2]; nur = '--pruefen' in sys.argv
files = sorted(glob.glob(os.path.join(D, '*.c')) + glob.glob(os.path.join(D, '*.h')) + glob.glob(os.path.join(D, '*.asm')))
txt = {f: open(f, encoding='latin1').read() for f in files}
alle = set()
for t in txt.values(): alle |= set(re.findall(r'\b[A-Za-z_]\w*\b', t))
rows = [r for r in csv.reader(open(L, encoding='utf-8'), delimiter='\t') if r and r[0] != 'alt' and not r[0].startswith('#')]
ok, fehler = [], []
neu_set = set()
for r in rows:
    alt, neu = r[0].strip(), r[1].strip()
    if alt == neu: continue
    if len(neu) > 31: fehler.append(f'{alt} -> {neu}: zu lang ({len(neu)})'); continue
    if alt not in alle: fehler.append(f'{alt}: kommt nicht vor'); continue
    if neu in alle or neu in neu_set: fehler.append(f'{alt} -> {neu}: Name schon vorhanden'); continue
    neu_set.add(neu); ok.append((alt, neu))
for f in fehler: print('UEBERSPRUNGEN', f)
print(len(ok), 'Umbenennungen gueltig')
if nur: sys.exit(0)
n = 0
for f, t in txt.items():
    o = t
    for alt, neu in ok:
        t = re.sub(r'\b' + re.escape(alt) + r'\b', neu, t)
        if f.endswith('.asm'): t = re.sub(r'\b_' + re.escape(alt) + r'\b', '_' + neu, t)
    if t != o: open(f, 'w', encoding='latin1').write(t); n += 1
print(n, 'Dateien geaendert')
