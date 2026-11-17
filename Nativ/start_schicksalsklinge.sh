#!/usr/bin/env bash
# ===========================================================================
# Startskript fuer Das Schwarze Auge: Die Schicksalsklinge (Nativer Port)
# ===========================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROBENLOG="${PROBENLOG:-0}"

if [[ $# -ge 1 && ( "$1" == "--probenlog" || "$1" == "log" ) ]]; then
    PROBENLOG=1
    shift
fi

# 1. SPIELPFAD aus erstem Parameter oder Umgebungsvariable uebernehmen
if [[ $# -ge 1 && -f "$1/SCHICK.DAT" ]]; then
    SPIELPFAD="$1"
    shift
fi

# 2. Automatische Erkennung, falls SPIELPFAD noch nicht gesetzt ist
if [[ -z "${SPIELPFAD:-}" ]]; then
    for cand in \
        "$PWD" \
        "$SCRIPT_DIR" \
        "$SCRIPT_DIR/../../Original/V3.02de_GOG" \
        "$HOME/GOG Games/Realms of Arkania - Blade of Destiny German" \
        "$HOME/.local/share/schicksalsklinge"; do
        if [[ -f "$cand/SCHICK.DAT" ]]; then
            SPIELPFAD="$(cd "$cand" && pwd)"
            break
        fi
    done
fi

if [[ -z "${SPIELPFAD:-}" || ! -f "$SPIELPFAD/SCHICK.DAT" ]]; then
    echo "[FEHLER] SCHICK.DAT wurde nicht gefunden!" >&2
    echo "Bitte setzen Sie SPIELPFAD auf das Verzeichnis Ihres Originalspiels (V3.02de):" >&2
    echo "  SPIELPFAD=/pfad/zu/Schicksalsklinge ./start_schicksalsklinge.sh" >&2
    exit 1
fi

# 3. Port-Binary suchen (nativ unter Linux oder via Wine/WSL)
PORT_BIN=""
for bin in \
    "$SCRIPT_DIR/SCHICK_PORT" \
    "$SCRIPT_DIR/build/SCHICK_PORT" \
    "$SCRIPT_DIR/../build/SCHICK_PORT" \
    "$SCRIPT_DIR/SCHICK_PORT.EXE" \
    "$SCRIPT_DIR/build/SCHICK_PORT.EXE" \
    "$SCRIPT_DIR/../build/SCHICK_PORT.EXE"; do
    if [[ -f "$bin" ]]; then
        PORT_BIN="$bin"
        break
    fi
done

if [[ -z "$PORT_BIN" ]]; then
    echo "[HINWEIS] Port-Binary noch nicht gebaut. Baue mit CMake ..."
    cmake -S "$SCRIPT_DIR" -B "$SCRIPT_DIR/build" -DCMAKE_BUILD_TYPE=Release
    cmake --build "$SCRIPT_DIR/build" -j4
    PORT_BIN="$SCRIPT_DIR/build/SCHICK_PORT"
fi

# 4. Pruefen, ob Spielordner beschreibbar ist; sonst nach Spielstaende/ neben der EXE umleiten
SAVEDIR="$SPIELPFAD"
if [[ "${DSA_FORCE_READONLY_GAMEDIR:-0}" == "1" ]] || ! touch "$SPIELPFAD/.schick_wtest_$$" 2>/dev/null; then
    SAVEDIR="$(dirname "$PORT_BIN")/Spielstaende"
    export DSA_SAVE_DIR="$SAVEDIR"
    mkdir -p "$SAVEDIR/TEMP"
    echo "[HINWEIS] Der Spielordner '$SPIELPFAD' ist nicht beschreibbar."
    echo "          Spielstaende und PROBEN.CSV werden nach '$SAVEDIR' umgeleitet."
else
    rm -f "$SPIELPFAD/.schick_wtest_$$"
fi

# 5. Probenlog: altes PROBEN.CSV mit Datum nach Probenlogs/ verschieben
if [[ -f "$SAVEDIR/PROBEN.CSV" ]]; then
    if [[ ! -s "$SAVEDIR/PROBEN.CSV" ]]; then
        rm -f "$SAVEDIR/PROBEN.CSV"
    else
        mkdir -p "$SCRIPT_DIR/Probenlogs"
        TS="$(date +%Y-%m-%d_%H-%M-%S)"
        mv -f "$SAVEDIR/PROBEN.CSV" "$SCRIPT_DIR/Probenlogs/PROBEN_Fix_Alles_${TS}.CSV"
    fi
fi

LOG_PID=""
cleanup_log() {
    if [[ -n "$LOG_PID" ]]; then
        kill "$LOG_PID" 2>/dev/null || true
    fi
}
trap cleanup_log EXIT

if [[ "$PROBENLOG" == "1" ]]; then
    : > "$SAVEDIR/PROBEN.CSV"
    if [[ -f "$SCRIPT_DIR/Probenlog/probenlog.py" && "${DSA_NO_PROBENLOG_WINDOW:-0}" != "1" ]]; then
        python3 "$SCRIPT_DIR/Probenlog/probenlog.py" "$SAVEDIR/PROBEN.CSV" &
        LOG_PID="$!"
    fi
fi

echo "Starte Schicksalsklinge mit Spieldaten aus: $SPIELPFAD"
"$PORT_BIN" "$SPIELPFAD" "$@"
