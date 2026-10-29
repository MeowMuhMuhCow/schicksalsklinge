#!/bin/bash
# build_fix.sh <quellordner> <wichtig|alles|FLAGS> [name]  - Fix-Modus der Werkbank (Cloud):
#   1. Standardbau mit b1.sh (FIX_BUGS=0) -> muss BYTEGLEICH sein, er liefert die Vergleichsfassung (OBJ, MAP)
#   2. alle Module mit den Schaltern neu uebersetzen (BCC -DFIX_BUGS=.., TASM /dFIX_BUGS=.. fuer seg008/009/010/013)
#   3. residente Module (SEG001..SEG013), die sich geaendert haben, auf ihre Originallaenge auffuellen (segpad.py);
#      ein residentes Modul, das laenger wird, ist ein Abbruchgrund (die Behebung passt nicht hinein)
#   4. in der IDE linken (BC.EXE /b BLADEM.PRJ, Datum 12.01.1994) und mit fixcheck.py pruefen
# Ergebnis: $R/fixexe/BLADEM_FIX_WICHTIG.EXE (FIX_BUGS=1) bzw. _ALLES (2) bzw. _<name>, dazu .MAP
# Beispiele:  bash build_fix.sh ~/q/Quelle wichtig
#             bash build_fix.sh ~/q/Quelle "-DFIX_BUGS=2 -DFIX_EXPLOITS=0" ALLES_OHNE_X
set -e
R=${Q005ROOT:-$HOME/q005}
T=$(cd "$(dirname "$0")" && pwd)
S=$(cd "${1:?Quellordner}" && pwd)
case "$2" in
  wichtig) FL="-DFIX_BUGS=1"; FN=WICHTIG; W=fw;;
  alles)   FL="-DFIX_BUGS=2"; FN=ALLES; W=fa;;
  *)       FL="${2:?wichtig|alles|FLAGS}"; FN=${3:-EIGEN}; W=fe;;
esac
[ -n "$3" ] && FN=$3
AFL=$(echo "$FL" | sed 's/-D/\/d/g')
echo "Fix-Modus: $FL -> BLADEM_FIX_$FN.EXE"
# 1. Standardbau
if [ -z "$OHNE_STANDARDBAU" ]; then
  bash $R/tools/b1.sh "$S" b1 || { echo "Fix-Modus: Standardbau nicht bytegleich - Abbruch"; exit 1; }
fi
cmp -s $R/b1/BLADEM.EXE $R/orig/SCHICKM.EXE || { echo "Fix-Modus: Vergleichsfassung b1 fehlt/nicht bytegleich"; exit 1; }
# 2. uebersetzen
rm -rf $R/$W && mkdir -p $R/$W && cp -r "$S/." $R/$W/ && cd $R/$W
rm -f *.OBJ BLADEM.EXE BLADEM.MAP
grep -vE '^(SETDATE|TLINK)' build.bat | sed 's/\.\.\\BIN\\/..\\bc\\BIN\\/g' | tr -d '\r' | while IFS= read -r l; do
  case "$l" in
    *BCC.EXE*) l=$(echo "$l" | sed "s/BCC\.EXE /BCC.EXE $FL /"); echo "$l >> UEBERS.LOG";;
    *TASM.EXE*SEG0*.asm*) a=$(ls | grep -ixF "$(echo "$l" | grep -oiE 'SEG0[0-9]+\.asm' | head -1)" | head -1)
                          if [ -n "$a" ] && grep -qi '^ *IF.*FIX_[0-9]' "$a"; then
                          l=$(echo "$l" | sed "s|TASM\.EXE |TASM.EXE $AFL |"); fi; echo "$l >> UEBERS.LOG";;
    *TASM.EXE*) echo "$l >> UEBERS.LOG";;
    *) echo "$l";;
  esac
done | sed 's/$/\r/' > BCF.BAT
DOSTIMEOUT=1500 bash $R/tools/dosrun.sh $W 'CALL BCF.BAT' > /dev/null || true
tr -d '\r' < UEBERS.LOG | grep -E '^(Error [^ ]+ [0-9]+:|Fatal|\*\*Error|\*\*Fatal)' && { echo "Fix-Modus: Uebersetzungsfehler - Abbruch"; exit 1; }
tr -d '\r' < UEBERS.LOG | grep -E '^Warning' | sort | uniq > warnungen.txt || true
n=$(ls *.OBJ | wc -l); [ $n -eq $(ls $R/b1/*.OBJ | wc -l) ] || { echo "Fix-Modus: nur $n OBJ - Abbruch"; exit 1; }
# 3. residente Module auffuellen
# (die OBJ unterscheiden sich immer in den Kommentarsaetzen mit Dateidatum; segpad fuellt nur, was kuerzer ist)
NR=0
for o in SEG00[1-9].OBJ SEG01[0-3].OBJ; do
  [ -f $o ] || continue
  python3 -I $T/segpad.py $o $R/b1/$o > ${o%.OBJ}.PAD || { cat ${o%.OBJ}.PAD; echo "Fix-Modus: $o passt nicht in sein residentes Segment - Abbruch"; exit 1; }
  grep -q 'Nullbytes' ${o%.OBJ}.PAD && NR=$((NR + 1))
done
echo "Fix-Modus: $NR residente Module aufgefuellt"
# 4. linken und pruefen
BE_DATE=1994-01-12 DOSTIMEOUT=300 bash $R/tools/dosrun.sh $W '..\bc\BIN\BC.EXE /b BLADEM.PRJ' > link.log || true
[ -s BLADEM.EXE ] || { cat link.log; echo "Fix-Modus: keine EXE - Abbruch"; exit 1; }
mkdir -p $R/fixexe
python3 -I $T/fixcheck.py BLADEM.EXE BLADEM.MAP $R/b1/BLADEM.EXE $R/b1/BLADEM.MAP || { echo "Fix-Modus: fixcheck FEHLER"; exit 1; }
cp BLADEM.EXE $R/fixexe/BLADEM_FIX_$FN.EXE; cp BLADEM.MAP $R/fixexe/BLADEM_FIX_$FN.MAP
ls -la $R/fixexe/BLADEM_FIX_$FN.EXE; md5sum $R/fixexe/BLADEM_FIX_$FN.EXE
