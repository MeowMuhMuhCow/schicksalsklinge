#!/bin/bash
# b1.sh <quellordner> [arbeitsname]  - Schicksalsklinge komplett bauen und gegen das Original pruefen
# Quellordner = BrightEyes-Stil (seg*.cpp, *.asm, AIL/, BLADEM.PRJ, build.bat)
# Arbeitsordner: ~/sternenschweif/<arbeitsname> (max. 8 Zeichen, Standard b1)
set -e
R=$HOME/sternenschweif
S=$(cd "${1:?Quellordner}" && pwd); N=${2:-b1}
W=$R/$N
O=$R/orig/SCHICKM.EXE
rm -rf $W && mkdir -p $W && cp -r "$S/." $W/ && cd $W
rm -f *.OBJ BLADEM.EXE
grep -vE '^(SETDATE|TLINK)' build.bat | sed 's/\.\.\\BIN\\/..\\bc\\BIN\\/g' > BCR.BAT
DOSTIMEOUT=1500 bash $R/tools/dosrun.sh $N 'CALL BCR.BAT' | tr -d '\r' | grep -iE '^(error|fatal)|^Warning' > compile.log || true
n=$(ls *.OBJ 2>/dev/null | wc -l)
BE_DATE=1994-01-12 DOSTIMEOUT=300 bash $R/tools/dosrun.sh $N '..\bc\BIN\BC.EXE /b BLADEM.PRJ' > link.log || true
if [ -f BLADEM.EXE ] && cmp -s BLADEM.EXE $O; then
  echo "BYTEGLEICH mit dem Original ($n OBJ)"
else
  echo "NICHT GLEICH ($n OBJ)"; head -20 compile.log; tail -5 link.log
  [ -f BLADEM.EXE ] && cmp -l BLADEM.EXE $O | wc -l | sed 's/$/ Bytes verschieden/'
  exit 1
fi
