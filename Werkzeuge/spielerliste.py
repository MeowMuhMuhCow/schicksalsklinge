#!/usr/bin/env python3
# Erzeugt die Spieler-Fehlerliste (md + tsv) aus Fehler_Raster.tsv und netz_fehler.tsv
import csv,re,sys
A='/root/ana/Fehler/'
r=list(csv.DictReader(open(A+'Fehler_Raster.tsv',encoding='utf-8'),delimiter='\t'))
netz={x['nr']:x for x in csv.DictReader(open(A+'netz_fehler.tsv',encoding='utf-8'),delimiter='\t')}
BER={'Helden':'Helden & Werte','Gegenstände':'Gegenstände & Inventar','Stadt':'Städte & Orte','Reise':'Reise & Ereignisse',
     'Dungeon':'Dungeons','Kampf':'Kampf','Magie':'Magie','Technik':'Technik & Oberfläche'}
ORD=list(BER)
def nb(x):
    ns=[n.strip() for n in re.split(r'[,; ]+',x['netz']) if n.strip() in netz]
    if not ns: return 'nein'
    n=netz[ns[0]]; mehr=f' (und {len(ns)-1} weitere Meldung{"en" if len(ns)>2 else ""})' if len(ns)>1 else ''
    return f'ja: „{n["titel"]}“{mehr} ({n["url"]})'
kein=[x for x in r if x['be_urteil']=='kein Fehler']
echt=[x for x in r if x['be_urteil']!='kein Fehler']
ex=[x for x in echt if x['exploit']=='ja']
def mat(s,a): return sum(1 for x in echt if x['sicherheit']==s and x['auswirkung']==a)
bek=sum(1 for x in echt if nb(x)!='nein')
o=[]
w=o.append
w('# Schicksalsklinge: Fehler im Original\n')
w('**Das Schwarze Auge 1: Die Schicksalsklinge** (Attic, 1992), deutsche CD-Fassung 3.02. Alle Fehler, die bei der vollständigen '
  'Rekonstruktion des Quelltexts gefunden wurden. Jeder Eintrag ist am Quelltext geprüft. Stand 20.10.2026, MooMoo mit Claude.\n')
w('**Spoiler-Warnung:** Die Liste nennt Orte, Ereignisse und Lösungen aus dem ganzen Spiel.\n')
w('## Übersicht\n')
w(f'{len(echt)} Fehler, eingeordnet nach zwei Fragen: *Ist das sicher ein Fehler oder vielleicht Absicht?* und *Wie stark wirkt er sich im Spiel aus?*\n')
w('| | kritisch | mittel | egal |\n|---|---:|---:|---:|')
for s,l in (('sicher','sicher falsch'),('unsicher','unsicher falsch')):
    w(f'| **{l}** | {mat(s,"kritisch")} | {mat(s,"mittel")} | {mat(s,"egal")} |')
w('')
w('| Begriff | Bedeutung |\n|---|---|')
w('| sicher falsch | Der Code widerspricht sich selbst oder seiner offensichtlichen Absicht |')
w('| unsicher | Könnte Absicht von Attic sein (Spielbalance, Design). Die Begründung steht dabei |')
w('| kritisch | Absturz, Speicher überschrieben, Spielstand oder Ausrüstung verloren, Spieler betrogen oder plötzlich reich, Quest blockiert |')
w('| mittel | spürbar falsches Spielverhalten: Text erscheint nie, Ereignis falsch, Werte falsch |')
w('| egal | ohne sichtbare Folge, unerreichbar, toter Code |')
w('| Im Netz bekannt | *ja* = in den durchgesehenen Sammlungen gemeldet (erste Meldung mit Link), *nein* = dort nicht gefunden |')
w(f'| Exploit | Fehler, der dem Spieler nützt und gezielt ausnutzbar ist ({len(ex)} Stück, siehe Cheat-Liste) |')
w('')
w(f'**Im Netz bekannt:** {bek} ja, {len(echt)-bek} nein. Durchgesehen wurden vor allem die Crystals-DSA-Foren-Threads „Fehler und Bugs“ '
  '(2006–2025) und „Umfassender Bugfix-Patch“ von NRS (2016–2026) sowie einige Steam- und VOGONS-Threads (Liste im Anhang). '
  'Dazu kommen die Markierungen im Bright-Eyes-Projekt, die alle am Code nachgeprüft wurden.\n')
w(f'{len(kein)} Einträge haben sich bei der Prüfung als Absicht oder als kein Fehler erwiesen. Sie stehen am Ende der Liste.\n')
w('## Cheat-Liste: Exploits\n')
w('Fehler, die dem Spieler nützen. Was im Spiel passiert und warum. Schritt-für-Schritt-Anleitungen zum Nachstellen folgen mit den Tests im Spiel.\n')
def eintrag(x,lang=False):
    t=f'**Nr. {x["nr"]}: {x["titel"]}**'+(' · **Exploit**' if x['exploit']=='ja' and not lang else '')+'  '
    w(t); w(x['spieler']+'  ')
    if x['sicherheit']=='unsicher' or lang: w(f'*Warum:* {x["begruendung"]}  ')
    w(f'*Im Netz bekannt:* {nb(x)}  \n')
for x in sorted(ex,key=lambda x:int(x['nr'])):
    w(f'### Nr. {x["nr"]}: {x["titel"]}\n')
    w(f'*{BER[x["bereich"]]} · {x["sicherheit"]}, {x["auswirkung"]}*\n')
    w(x['spieler']+'\n')
    w(f'**Warum:** {x["begruendung"]}\n')
    w(f'**Im Netz bekannt:** {nb(x)}\n')
for s in ('sicher','unsicher'):
    for a in ('kritisch','mittel','egal'):
        g=[x for x in echt if x['sicherheit']==s and x['auswirkung']==a]
        if not g: continue
        w(f'## {s.capitalize()} falsch, {a} ({len(g)})\n')
        for b in ORD:
            gb=sorted([x for x in g if x['bereich']==b],key=lambda x:int(x['nr']))
            if not gb: continue
            w(f'### {BER[b]}\n')
            for x in gb: eintrag(x)
w(f'## Geprüft: kein Fehler ({len(kein)})\n')
w('Markierungen von Bright-Eyes und Meldungen aus dem Netz, die sich am Code als Absicht oder als harmlos erwiesen haben.\n')
for x in sorted(kein,key=lambda x:int(x['nr'])):
    w(f'**Nr. {x["nr"]}: {x["titel"]}**  '); w(x['spieler']+'  '); w(f'*Warum:* {x["begruendung"]}  \n')
w('## Anhang: Durchgesehene Quellen im Netz\n')
q=open(A+'netz_quellen.md',encoding='utf-8').read()
tab=[l for l in q.splitlines() if l.startswith('|')]
w('\n'.join(tab)+'\n')
w('## Technische Details\n')
w('Codestelle, genaue Beschreibung, Begründung der Einordnung und Fix-Vorschlag je Nummer: `Analyse/Fehler/Originalfehler.md` und '
  '`Analyse/Fehler/Fehler_Raster.tsv` im Quelltext-Paket. Die Fehler sind im rekonstruierten Quelltext bewusst so nachgebaut, damit er '
  'Byte für Byte das Original ergibt. Behoben werden sie später über Schalter, wenn auch Schatten über Riva fertig ist.\n')
open(A+'Spieler/Fehlerliste_Schicksalsklinge.md','w',encoding='utf-8').write('\n'.join(o))
with open(A+'Spieler/Fehler_Liste_Spieler.tsv','w',encoding='utf-8') as f:
    f.write('nr\ttitel\tbereich\tsicherheit\tauswirkung\texploit\tspieler\tim_netz\n')
    for x in sorted(r,key=lambda x:int(x['nr'])):
        f.write('\t'.join([x['nr'],x['titel'],BER[x['bereich']],x['sicherheit'] if x['be_urteil']!='kein Fehler' else 'kein Fehler',
          x['auswirkung'],x['exploit'],x['spieler'],nb(x)]).replace('\n',' ')+'\n')
print(len(echt),len(kein),len(ex),bek, [ (s,a,mat(s,a)) for s in ('sicher','unsicher') for a in ('kritisch','mittel','egal')])
