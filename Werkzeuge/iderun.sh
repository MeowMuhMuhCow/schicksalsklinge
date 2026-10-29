#!/bin/bash
# iderun.sh <dos-dir> <befehl> <sek> [tastenskript]  - DOSBox mit Bildschirmfotos und Tastenskript
R=$HOME/sternenschweif; WD=$1; CMD=$2; T=${3:-60}; KS=$4
CONF=$(mktemp --suffix=.conf)
cat > $CONF <<C
[sdl]
output=surface
[dosbox]
memsize=16
captures=/tmp/claude-0/shots
[cpu]
core=normal
cycles=max
[mixer]
nosound=true
[autoexec]
mount c $R
c:
cd \\$WD
$CMD
exit
C
rm -rf /tmp/claude-0/shots; mkdir -p /tmp/claude-0/shots
BE_DATE=${BE_DATE:-} BE_KEYSCRIPT=$KS LD_LIBRARY_PATH=$R/sdlinst/lib SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy timeout -s KILL $T $R/be-build/src/dosbox -conf $CONF -noconsole > /tmp/claude-0/ide.log 2>&1
rm -f $CONF; grep -c SHOT /tmp/claude-0/ide.log; grep KEYSCRIPT /tmp/claude-0/ide.log
