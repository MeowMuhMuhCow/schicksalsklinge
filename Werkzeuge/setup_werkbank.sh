#!/bin/bash
# setup_werkbank.sh - baut Claudes Werkbank in einer frischen Cloud-Sitzung neu auf.
# Dauer ca. 3-5 Minuten. Ergebnis unter ~/sternenschweif:
#   Bright-Eyes/   Referenz-Repo (github.com/Henne/Bright-Eyes)
#   sdlinst/       SDL 1.2 ohne X11 (Dummy-Video/-Audio)
#   be-build/      DOSBox (Bright-Eyes-Fork), headless lauffaehig
#   bc/            Borland C++ 3.1 (BIN, INCLUDE, LIB) - muss vom Rechner gestaged werden
#   tools/         alle Werkzeuge (aus dem Ordner dieses Skripts kopiert)
# Aufruf: bash setup_werkbank.sh   (aus Q 005 .../Werkzeuge, dort liegen auch die Patches)
set -e
T=$(cd "$(dirname "$0")" && pwd)
R=$HOME/sternenschweif
mkdir -p $R && cd $R

[ -d Bright-Eyes ] || git clone -q https://github.com/Henne/Bright-Eyes.git
git -C Bright-Eyes checkout -q df74edde   # Stand, auf dem Q 005 beruht
mkdir -p tools && cp "$T"/*.py "$T"/*.sh tools/ 2>/dev/null || true

if [ ! -f sdlinst/lib/libSDL.so ]; then
  [ -d sdl12 ] || git clone -q --depth 1 https://github.com/libsdl-org/SDL-1.2.git sdl12
  cd sdl12
  ./configure --prefix=$R/sdlinst --disable-video-x11 --disable-pulseaudio --disable-alsa \
    --disable-esd --disable-arts --disable-nas --disable-oss --disable-video-fbcon \
    --disable-video-directfb --disable-video-opengl -q
  make -j16 >/dev/null && make install >/dev/null
  cd $R
fi

if [ ! -x be-build/src/dosbox ]; then
  rm -rf be-build && cp -r Bright-Eyes be-build && rm -rf be-build/.git
  cd be-build
  # Messfuehler: BE_NOHOOK, BE_FILELOG, BE_AUTOKEY, BE_SHOTS, BE_TRACEIP ([FAR]/[DIV0]),
  # BE_WATCH, BE_INT21LOG, BE_DATE  (siehe PROJEKT.md in Q 005)
  [ -f "$T/dosbox_instrumentierung.patch" ] && patch -s -p1 < "$T/dosbox_instrumentierung.patch"
  ./autogen.sh >/dev/null 2>&1
  PATH=$R/sdlinst/bin:$PATH ./configure --with-sdl-prefix=$R/sdlinst --disable-opengl \
    --disable-alsa-midi --disable-debug -q
  sed -i 's/#define C_X11_XKB 1/\/* no xkb *\//' config.h   # SDL ohne X11
  make -j16 >/dev/null
  cd $R
fi

# Borland-Konfiguration auf Laufwerk C: der Werkbank umbiegen
if [ -d bc/BIN ]; then
  printf -- '-IC:\\BC\\INCLUDE\r\n-LC:\\BC\\LIB\r\n' > bc/BIN/TURBOC.CFG
  printf -- '-LC:\\BC\\LIB\r\n' > bc/BIN/TLINK.CFG
else
  echo "HINWEIS: bc/ fehlt - Compiler/BIN, INCLUDE, LIB vom Rechner stagen und nach $R/bc kopieren"
fi
echo "Werkbank bereit."
