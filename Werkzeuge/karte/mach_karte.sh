#!/bin/bash
# mach_karte.sh - Quelltextkarte aus ~/ana/module.json und der Fehlerliste erzeugen -> ~/ana/Quelltextkarte.html
set -e
T=$(mktemp -d); K=$(dirname "$0")
python3 - "$T" <<'P'
import json,csv,re,sys
T=sys.argv[1]
M=json.load(open('/root/ana/module.json',encoding='utf-8'))
R=list(csv.DictReader(open('/root/ana/Fehler/Fehler_Raster.tsv',encoding='utf-8'),delimiter='\t'))
by={}
for r in R:
    for m in re.findall(r'seg\d{3}',(r.get('modul') or '').lower()):
        by.setdefault(m.upper(),[]).append(int(r['nr']))
for m in M: m['fehler']=sorted(set(by.get(m['modul'],[])))
json.dump(M,open(T+'/module.json','w',encoding='utf-8'),ensure_ascii=False)
json.dump(M,open('/root/ana/module.json','w',encoding='utf-8'),ensure_ascii=False,indent=1)
with open(T+'/fehler_raster.tsv','w',encoding='utf-8') as o:
    w=csv.writer(o,delimiter='\t'); w.writerow(['nr','sicherheit','auswirkung','begruendung','fix'])
    for r in R: w.writerow([r['nr'],r['sicherheit'],r['auswirkung'],r['begruendung'],r['fix']])
P
cp /root/ana/Fehler/Originalfehler.md $T/ORIGINALFEHLER.md
cd $T && python3 $K/gen_seite.py $K/seite_vorlage.html /root/ana/Quelltextkarte.html
rm -rf $T
