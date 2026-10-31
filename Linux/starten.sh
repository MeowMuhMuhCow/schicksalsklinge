#!/usr/bin/env bash
# starten.sh - startet eine Fassung des Spiels in DOSBox. Linux-Gegenstueck zu den *_STARTEN.bat.
#   bash Linux/starten.sh wichtig     Bugfix-Fassung: kritische + mittlere Fehler behoben (nach patchen.sh)
#   bash Linux/starten.sh alles       Bugfix-Fassung: alle behebbaren Fehler (nach patchen.sh)
#   bash Linux/starten.sh quelltext   die selbst gebaute, bytegleiche EXE (nach dem Bauen)
# Der Spielordner wird nicht angefasst: Beim ersten Start wird er nach Build/Linux_C_<Fassung> kopiert
# (ohne CD-Abbild und DOSBox), dort landen auch die Spielstaende dieser Fassung. Spaetere Starts
# ueberschreiben dort nichts ausser der EXE und den korrigierten Spieldaten.
# Eigene DOSBox:  DOSBOX=/pfad/zu/dosbox bash Linux/starten.sh wichtig
# Ohne die DOSBox-Einstellungen von GOG:  OHNE_GOGCONF=1 bash Linux/starten.sh wichtig
. "$(dirname "${BASH_SOURCE[0]}")/gemeinsam.sh"

case "${1:-}" in
	wichtig)   QUELLE="$BUILD/${EXE}_FIX_WICHTIG.EXE"; FASSUNG="Fix_Wichtig"; MITDATEN=1 ;;
	alles)     QUELLE="$BUILD/${EXE}_FIX_ALLES.EXE";   FASSUNG="Fix_Alles";   MITDATEN=1 ;;
	quelltext) QUELLE="$BUILD/${EXE}.EXE";             FASSUNG="Quelltext";   MITDATEN=0 ;;
	*) echo "Aufruf: bash Linux/starten.sh wichtig | alles | quelltext"; exit 1 ;;
esac
if [ ! -f "$QUELLE" ]; then
	echo "$QUELLE fehlt."
	[ "$FASSUNG" = "Quelltext" ] && echo "Erst bauen (Bauen/BAUEN.md)." || echo "Erst patchen:  bash Linux/patchen.sh"
	exit 1
fi
spielpfad_suchen || exit 1
dosbox_finden || exit 1

C="$BUILD/Linux_C_$FASSUNG"
E="$BUILD/Linux_E_$FASSUNG"
mkdir -p "$C" "$E"
cp -f "$QUELLE" "$E/$EXE.EXE"

# Spielordner kopieren, ohne vorhandene Dateien zu ueberschreiben (Spielstaende bleiben erhalten)
( cd "$SPIEL" && tar cf - --exclude=./DOSBOX --exclude=./dosbox --exclude=./DOSBox --exclude=./MUSIC --exclude=./music \
	--exclude='*.gog' --exclude='*.GOG' --exclude='*.inst' --exclude='*.ins' --exclude='*.INS' --exclude='*.ogg' . ) \
	| ( cd "$C" && tar xf - --skip-old-files ) || { echo "Kopieren des Spielordners fehlgeschlagen."; exit 1; }
chmod -R u+w "$C"   # GOG-Dateien koennen schreibgeschuetzt sein, das Spiel schreibt aber Spielstaende

# korrigierte Spieldaten (von patchen.sh) einsetzen
if [ "$MITDATEN" = 1 ] && [ -n "$DATEN" ] && [ -f "$BUILD/$DATEN" ]; then
	ZIEL="$(datei_in "$C" "$DATEN")"
	cp -f "$BUILD/$DATEN" "${ZIEL:-$C/$DATEN}"
fi

# CD-Abbild von GOG (CD-Audio bzw. CD-Pruefung)
CDZEILE=""
CDTEILE=()
for t in $CDBILD; do
	f="$(datei_in "$SPIEL" "$t")"
	[ -n "$f" ] && CDTEILE+=("\"$f\"")
done
[ -n "$CDBILD" ] && [ "${#CDTEILE[@]}" -eq "$(echo $CDBILD | wc -w)" ] && CDZEILE="imgmount D ${CDTEILE[*]} -t iso"

CONF="$E/dosbox_linux.conf"
{
	echo "[sdl]"
	echo "fullscreen=false"
	echo "[autoexec]"
	echo "@echo off"
	echo "mount C \"$C\""
	[ -n "$CDZEILE" ] && echo "$CDZEILE"
	echo "mount E \"$E\""
	echo "C:"
	echo "cls"
	[ -n "$INTRO" ] && echo "$INTRO"
	echo "E:\\$EXE.EXE"
	echo "echo."
	echo "echo $EXE ($FASSUNG) wurde beendet. Taste druecken zum Schliessen."
	echo "pause"
	echo "exit"
} > "$CONF"

ARGS=()
GOGC="$(datei_in "$SPIEL" "$GOGCONF")"
[ -n "$GOGC" ] && [ -z "${OHNE_GOGCONF:-}" ] && ARGS+=(-conf "$GOGC")
ARGS+=(-conf "$CONF")
echo "Starte $SPIELNAME ($FASSUNG) mit ${DOSBOX_CMD[*]} ..."
exec "${DOSBOX_CMD[@]}" "${ARGS[@]}"
