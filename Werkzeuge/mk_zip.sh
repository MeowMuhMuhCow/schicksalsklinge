#!/bin/bash
# mk_zip.sh <ziel.zip> - Weitergabe-Paket bauen (ohne Compiler, ohne Original-EXE, ohne GOG-Daten)
set -e
Z=${1:?Ziel}; R=$HOME/sternenschweif; T=$(mktemp -d); P=$T/Schicksalsklinge
mkdir -p $P/{Quelle,Bauen,Analyse,Werkzeuge,Referenz,Compiler,Original,Build}
cp -r $R/BE2/src/schick_dos/. $P/Quelle/
cp $HOME/q005neu/Bauen/BAUEN.bat $HOME/q005neu/Bauen/bauen.py $HOME/q005neu/Bauen/BAUEN.md $P/Bauen/
cp $HOME/q005neu/WIEDERAUFBAU.md $P/Werkzeuge/
cp $HOME/q005neu/{START_HIER.md,LIESMICH.md,ABSCHLUSSBERICHT.md,BLADEM_STARTEN.bat,SPIELPFAD_SUCHEN.cmd} $P/
cp -r $HOME/ana/. $P/Analyse/
rm -rf $P/Analyse/Beschriftung/wellen/*.tsv.bak $P/Analyse/Steckbriefe 2>/dev/null || true
for f in setup_werkbank.sh dosbox_instrumentierung.patch dosrun.sh iderun.sh omfcmp.py; do cp $R/tools5/$f $P/Werkzeuge/ 2>/dev/null || true; done
for f in b1.sh cc_c.sh dcmp.py bssorder.py bsscheck.py pubs.py umbenennen.py sichern.sh mk_zip.sh spielerliste.py; do cp $R/tools/$f $P/Werkzeuge/; done
mkdir -p $P/Werkzeuge/kommentar $P/Werkzeuge/karte && cp $R/tools/kommentar/*.py $P/Werkzeuge/kommentar/ && cp $R/tools/karte/* $P/Werkzeuge/karte/
cp -r $HOME/w/q005/Quelle $P/Referenz/Bright-Eyes_df74edde
cp $HOME/w/q005/Quelle/COPYING_GPL.txt $P/Quelle/
printf 'Hierher Borland C++ 3.1 kopieren (BIN, INCLUDE, LIB), siehe START_HIER.md Schritt 3.\r\n' > $P/Compiler/LIESMICH.txt
printf 'Hierher SCHICKM.EXE aus der GOG-Installation kopieren, siehe START_HIER.md Schritt 4.\r\n' > $P/Original/LIESMICH.txt
printf 'Hier landet die gebaute BLADEM.EXE.\r\n' > $P/Build/LIESMICH.txt
find $P -name '__pycache__' -prune -exec rm -rf {} + 
rm -f $Z; (cd $T && zip -qr $Z Schicksalsklinge)
rm -rf $T; ls -la $Z; md5sum $Z
