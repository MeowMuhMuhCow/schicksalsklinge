#!/bin/bash
# sichern.sh - Arbeitsstand fuer MooMoos Rechner verpacken (-> /mnt/user-data/outputs/sicherung.tgz)
# Inhalt: Quelle/ (aktueller Stand attic), Werkzeuge/ (neue Werkzeuge), Analyse/ (~/ana), Werkzeuge/werkbank_git.bundle
set -e
T=$(mktemp -d); R=$HOME/sternenschweif
mkdir -p $T/Quelle $T/Werkzeuge $T/Analyse
cd $R/BE2 && git bundle create $T/Werkzeuge/werkbank_git.bundle --all >/dev/null 2>&1
cp -r $R/BE2/src/schick_dos/. $T/Quelle/ && rm -rf $T/Quelle/build $T/Quelle/orig
for f in b1.sh cc_c.sh dcmp.py sichern.sh bssorder.py bsscheck.py pubs.py umbenennen.py; do cp $R/tools/$f $T/Werkzeuge/; done
[ -d $HOME/ana ] && cp -r $HOME/ana/. $T/Analyse/
mkdir -p $T/Werkzeuge/kommentar && cp $R/tools/kommentar/*.py $T/Werkzeuge/kommentar/
[ -d $HOME/q005neu ] && cp -r $HOME/q005neu/. $T/
[ -n "$1" ] && cp "$@" $T/ 2>/dev/null || true
mkdir -p /mnt/user-data/outputs && S=/mnt/user-data/outputs/sicherung_$(date +%H%M%S).tgz; tar czf $S -C $T . && rm -rf $T; echo $S
md5sum /mnt/user-data/outputs/sicherung_*.tgz | tail -1
