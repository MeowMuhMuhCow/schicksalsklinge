#!/bin/bash
# dosrun.sh <dos-workdir relativ zu ~/sternenschweif> <befehl...>
# Fuehrt einen DOS-Befehl headless in DOSBox aus, Ausgabe nach OUT.TXT
ROOT=$HOME/sternenschweif
WD="$1"; shift
CMD="$*"
CONF=$(mktemp --suffix=.conf)
cat > $CONF <<C
[sdl]
output=surface
[dosbox]
memsize=16
[cpu]
core=normal
cycles=max
[mixer]
nosound=true
[speaker]
pcspeaker=false
[sblaster]
sbtype=none
[gus]
gus=false
[autoexec]
mount c $ROOT
c:
cd \\$WD
$CMD > DOSOUT.TXT
exit
C
BE_DATE=$BE_DATE LD_LIBRARY_PATH=$ROOT/sdlinst/lib SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy timeout ${DOSTIMEOUT:-300} $ROOT/be-build/src/dosbox -conf $CONF -noconsole >/tmp/claude-0/dosbox.log 2>&1
rm -f $CONF
cat "$ROOT/$(echo $WD | tr '\\' '/')/DOSOUT.TXT" 2>/dev/null | tr -d '\r'
