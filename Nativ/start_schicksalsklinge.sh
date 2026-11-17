#!/usr/bin/env bash
# ===========================================================================
# Startskript fuer Das Schwarze Auge: Die Schicksalsklinge (Nativer Port)
# ===========================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

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

echo "Starte Schicksalsklinge mit Spieldaten aus: $SPIELPFAD"
exec "$PORT_BIN" "$SPIELPFAD" "$@"
