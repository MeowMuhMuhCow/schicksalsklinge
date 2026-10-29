#!/usr/bin/env python3
# Erzeugt die Übersichtsseite (eine HTML-Datei) aus module.json und ORIGINALFEHLER.md
import json, re, sys, html

M = json.load(open('module.json'))
fehler = []
for l in open('ORIGINALFEHLER.md'):
    if not re.match(r'^\| \d+ \|', l):
        continue
    c = [x.strip().replace('\\|', '|') for x in re.split(r'(?<!\\)\|', l.strip().strip('|'))]
    fehler.append({'nr': int(c[0]), 'ort': c[1], 'fehler': c[2], 'folge': c[3], 'schwere': c[4]})

import csv
R={}
try:
    for r in csv.DictReader(open('fehler_raster.tsv',encoding='utf-8'),delimiter='\t'):
        R[int(r['nr'])]=r
except FileNotFoundError: pass
for f in fehler:
    r=R.get(f['nr'])
    if r: f.update(sicher=r['sicherheit'], ausw=r['auswirkung'], begr=r['begruendung'], fix=r['fix'])
stand = {
    'c_module': sum(1 for m in M if m['sprache'] != 'ASM'),
    'asm_module': sum(1 for m in M if m['sprache'] == 'ASM'),
    'c_zeilen': sum(m['zeilen'] for m in M if m['sprache'] != 'ASM'),
    'fehler': len(fehler),
    'hoch': sum(1 for f in fehler if f.get('ausw') == 'kritisch'),
}
data = json.dumps({'module': M, 'fehler': fehler, 'stand': stand}, ensure_ascii=False)
data = data.replace('</', '<\\/')
tpl = open(sys.argv[1]).read()
open(sys.argv[2], 'w').write(tpl.replace('/*__DATEN__*/null', data))
print('ok', len(M), len(fehler), stand)
