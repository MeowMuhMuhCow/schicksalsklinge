#!/bin/bash
# cc_c.sh <quellordner> [arbeitsname] [modul...] - alle (oder genannte) Module als C uebersetzen und
# je Modul gegen das Original vergleichen. Ergebnis: <arbeit>/cstand.txt (Modul Status Fehlerzahl)
R=$HOME/sternenschweif
S=$(cd "${1:?Quellordner}" && pwd); N=${2:-c1}; shift 2 2>/dev/null
W=$R/$N; O=$R/orig/SCHICKM.EXE
rm -rf $W && mkdir -p $W && cp -r "$S/." $W/ && cd $W
rm -f *.OBJ
for f in seg*.cpp; do [ -f "$f" ] && b=$(basename $f .cpp) && mv $f ${b^^}.C; done
SEL="$*"
grep -E 'BCC.EXE.*SEG[0-9]+\.(CPP|C)\r?$' build.bat | sed 's/\.\.\\BIN\\/..\\bc\\BIN\\/; s/\.CPP\r\?$/.C/' | while read -r l; do
  m=$(echo "$l" | grep -oE 'SEG[0-9]+'); [ -n "$SEL" ] && ! echo " $SEL " | grep -q " $m " && continue
  echo "$l > $m.ERR"; done > BCE.BAT
DOSTIMEOUT=1500 bash $R/tools/dosrun.sh $N 'CALL BCE.BAT' >/dev/null
: > cstand.txt
for e in SEG*.ERR; do m=${e%.ERR}; ne=$(tr -d '\r' < $e | grep -cE '^(Error|Fatal)')
  if [ -f $m.OBJ ]; then st=$(python3 -I $R/tools/omfcmp.py $O $m.OBJ 2>&1 | awk '{print $5}'); else st=FEHLER; fi
  echo "$m $st $ne" >> cstand.txt; done
awk '{print $2}' cstand.txt | sort | uniq -c
