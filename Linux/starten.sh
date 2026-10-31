#!/usr/bin/env bash
# starten.sh - startet eine Fassung des Spiels in DOSBox. Linux-Gegenstueck zu den *_STARTEN.bat.
#   bash Linux/starten.sh wichtig     Bugfix-Fassung: kritische + mittlere Fehler behoben (nach patchen.sh)
#   bash Linux/starten.sh alles       Bugfix-Fassung: alle behebbaren Fehler (nach patchen.sh)
#   bash Linux/starten.sh quelltext   die selbst gebaute, bytegleiche EXE (nach dem Bauen)
#   bash Linux/starten.sh wichtig log dazu das Probenlog: jede Probe erscheint live in diesem Terminal
#
# Der Spielordner wird nicht angefasst. Spielstaende und Probenlog liegen im Ablageordner der Fassung:
# Build/Linux_C_<Fassung>, oder mit ABLAGE=... in <ABLAGE>/<Fassung> (z. B. ausserhalb des Clones).
#
# Einstellungen als Umgebungsvariable oder dauerhaft in Linux/starten.conf (Zeilen SCHLUESSEL=wert):
#   SPIEL=pfad          Spielordner (sonst gesucht bzw. aus Linux/SPIELPFAD.txt)
#   CD=pfad             eigenes CD-Abbild, z. B. ~/Spiele/NLT_Images/dsa1.cue (Sternenschweif: mehrere Teile mit Leerzeichen)
#   ABLAGE=ordner       Ablage fuer Spielstaende und Probenlogs, z. B. ~/Spiele/Schicksalsklinge_Spielstaende
#   SPEICHERN=overlay   nur DOSBox Staging/DOSBox-X: Spielordner als C:, Geschriebenes in die Ablage (keine Kopie)
#   VOLLBILD=1          Vollbild
#   INTRO=0             Intro ueberspringen
#   CYCLES=3000         Tempo (DOSBox-Zyklen)
#   HIMMEL=0            Schicksalsklinge: Original-Himmel statt des lebendigen Himmels (ab 1.4)
#   DOSBOX=pfad         andere DOSBox
#   OHNE_GOGCONF=1      ohne die DOSBox-Einstellungen von GOG (dosbox_realms*.conf)
. "$(dirname "${BASH_SOURCE[0]}")/gemeinsam.sh"

case "${1:-}" in
	wichtig)   QUELLE="$BUILD/${EXE}_FIX_WICHTIG.EXE"; FASSUNG="Fix_Wichtig"; MITDATEN=1 ;;
	alles)     QUELLE="$BUILD/${EXE}_FIX_ALLES.EXE";   FASSUNG="Fix_Alles";   MITDATEN=1 ;;
	quelltext) QUELLE="$BUILD/${EXE}.EXE";             FASSUNG="Quelltext";   MITDATEN=0 ;;
	*) echo "Aufruf: bash Linux/starten.sh wichtig | alles | quelltext  [log]"; exit 1 ;;
esac
LOG=0
[ "${2:-}" = "log" ] && LOG=1
if [ "$LOG" = 1 ] && [ -z "$PROBENLOG" ]; then echo "Das Probenlog gibt es fuer $SPIELNAME noch nicht."; LOG=0; fi
if [ ! -f "$QUELLE" ]; then
	echo "$QUELLE fehlt."
	[ "$FASSUNG" = "Quelltext" ] && echo "Erst bauen (Bauen/BAUEN.md)." || echo "Erst patchen:  bash Linux/patchen.sh"
	exit 1
fi
spielpfad_suchen || exit 1
dosbox_finden || exit 1

# Art der DOSBox: Overlay-Laufwerke und cpu_cycles gibt es nur bei Staging/DOSBox-X
VER="$("${DOSBOX_CMD[@]}" --version 2>&1 | head -n 5 | tr 'A-Z' 'a-z')"
case "$VER" in *staging*) ART=staging ;; *dosbox-x*|*"dosbox x"*) ART=x ;; *) ART=klassisch ;; esac

# Ablage: Spielstaende, korrigierte Spieldaten, Probenlog
if [ -n "${ABLAGE:-}" ]; then
	ABLAGE="${ABLAGE/#\~/$HOME}"
	C="$ABLAGE/$FASSUNG"; LOGARCHIV="$ABLAGE/Probenlogs"
else
	C="$BUILD/Linux_C_$FASSUNG"; LOGARCHIV="$BUILD/Probenlogs"
fi
E="$BUILD/Linux_E_$FASSUNG"
mkdir -p "$C" "$E" || exit 1
cp -f "$QUELLE" "$E/$EXE.EXE"

OVERLAY=0
if [ "${SPEICHERN:-}" = "overlay" ]; then
	if [ "$ART" = klassisch ]; then
		echo "Hinweis: SPEICHERN=overlay geht nur mit DOSBox Staging oder DOSBox-X, diese DOSBox kopiert den Spielordner."
	else
		OVERLAY=1
	fi
fi
if [ "$OVERLAY" = 0 ]; then
	# Spielordner in die Ablage kopieren, ohne vorhandene Dateien zu ueberschreiben (Spielstaende bleiben erhalten)
	( cd "$SPIEL" && tar cf - --exclude=./DOSBOX --exclude=./dosbox --exclude=./DOSBox --exclude=./MUSIC --exclude=./music \
		--exclude='*.gog' --exclude='*.GOG' --exclude='*.inst' --exclude='*.ins' --exclude='*.INS' --exclude='*.ogg' . ) \
		| ( cd "$C" && tar xf - --skip-old-files ) || { echo "Kopieren des Spielordners fehlgeschlagen."; exit 1; }
	chmod -R u+w "$C"   # GOG-Dateien koennen schreibgeschuetzt sein, das Spiel schreibt aber Spielstaende
fi

# korrigierte Spieldaten (von patchen.sh) einsetzen; im Overlay-Modus liegen sie in der Ablage ueber dem Spielordner
if [ "$MITDATEN" = 1 ] && [ -n "$DATEN" ] && [ -f "$BUILD/$DATEN" ]; then
	ZIEL="$(datei_in "$C" "$DATEN")"
	if [ -z "$ZIEL" ]; then
		ORIGNAME="$(datei_in "$SPIEL" "$DATEN")"
		ZIEL="$C/$(basename "${ORIGNAME:-$DATEN}")"
	fi
	cp -f "$BUILD/$DATEN" "$ZIEL"
fi

# Probenlog: das Spiel schreibt nur, wenn PROBEN.CSV da ist. Ein altes Log wandert ins Archiv.
P="$(datei_in "$C" PROBEN.CSV)"
if [ -n "$P" ]; then
	if [ -s "$P" ]; then mkdir -p "$LOGARCHIV"; mv -f "$P" "$LOGARCHIV/PROBEN_${FASSUNG}_$(date +%Y-%m-%d_%H-%M-%S).CSV"; else rm -f "$P"; fi
fi
[ "$LOG" = 1 ] && : > "$C/PROBEN.CSV"

# Lebendiger Stadthimmel (nur Schicksalsklinge, ab 1.4, Vorgabe an): HIMMEL=0 legt HIMMEL.AUS an, sonst wird sie entfernt
if [ "$EXE" = BLADEM ]; then
	H="$(datei_in "$C" HIMMEL.AUS)"
	if [ "${HIMMEL:-1}" = 0 ]; then [ -z "$H" ] && : > "$C/HIMMEL.AUS"; else [ -n "$H" ] && rm -f "$H"; fi
fi

# CD-Abbild: eigenes ueber CD=..., sonst das von GOG im Spielordner (CD-Audio bzw. CD-Pruefung).
# "-t cdrom" verstehen DOSBox 0.74, DOSBox Staging und DOSBox-X.
CDZEILE=""
CDTEILE=()
if [ -n "${CD:-}" ]; then
	if [ -f "${CD/#\~/$HOME}" ]; then TEILE=("$CD"); else read -r -a TEILE <<< "$CD"; fi
	for f in "${TEILE[@]}"; do
		f="${f/#\~/$HOME}"
		[ -f "$f" ] || { echo "CD-Abbild nicht gefunden: $f"; exit 1; }
		CDTEILE+=("\"$(cd "$(dirname "$f")" && pwd)/$(basename "$f")\"")
	done
	CDZEILE="imgmount D ${CDTEILE[*]} -t cdrom"
	echo "CD-Abbild: $CD"
else
	for t in $CDBILD; do
		f="$(datei_in "$SPIEL" "$t")"
		[ -n "$f" ] && CDTEILE+=("\"$f\"")
	done
	[ -n "$CDBILD" ] && [ "${#CDTEILE[@]}" -eq "$(echo $CDBILD | wc -w)" ] && CDZEILE="imgmount D ${CDTEILE[*]} -t cdrom"
fi

CONF="$E/dosbox_linux.conf"
{
	echo "[sdl]"
	[ "${VOLLBILD:-0}" = 1 ] && echo "fullscreen=true" || echo "fullscreen=false"
	if [ -n "${CYCLES:-}" ]; then
		echo "[cpu]"
		[ "$ART" = staging ] && echo "cpu_cycles=$CYCLES" || echo "cycles=$CYCLES"
	fi
	echo "[autoexec]"
	echo "@echo off"
	if [ "$OVERLAY" = 1 ]; then
		echo "mount C \"$SPIEL\""
		echo "mount C \"$C\" -t overlay"
	else
		echo "mount C \"$C\""
	fi
	[ -n "$CDZEILE" ] && echo "$CDZEILE"
	echo "mount E \"$E\""
	echo "C:"
	echo "cls"
	[ -n "$INTROPROG" ] && [ "${INTRO:-1}" != 0 ] && echo "$INTROPROG"
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
echo "Spielstaende: $C$([ "$OVERLAY" = 1 ] && echo '  (Overlay ueber dem Spielordner)')"
if [ "$LOG" = 1 ]; then
	"${DOSBOX_CMD[@]}" "${ARGS[@]}" >/dev/null 2>&1 &
	DPID=$!
	python3 "$REPO/Probenlog/probenlog.py" --live "$C/PROBEN.CSV" &
	VPID=$!
	wait $DPID
	kill $VPID 2>/dev/null
	echo; echo "Spiel beendet. Das Log liegt in $C/PROBEN.CSV (ansehen: Probenlog/Probenlog_$SPIELNAME.html)."
else
	exec "${DOSBOX_CMD[@]}" "${ARGS[@]}"
fi
