#!/usr/bin/env bash
# ===========================================================================
# Startskript fuer Das Schwarze Auge: Die Schicksalsklinge (Nativer Linux/macOS-Port)
# ===========================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PORT_EXE_NAME="${PORT_EXE_NAME:-SCHICK_PORT}"
PROBENLOG="${PROBENLOG:-0}"

if [[ -d "$SCRIPT_DIR/lib" ]]; then
    export LD_LIBRARY_PATH="$SCRIPT_DIR/lib:${LD_LIBRARY_PATH:-}"
fi

if [[ $# -ge 1 && ( "$1" == "--probenlog" || "$1" == "log" ) ]]; then
    PROBENLOG=1
    shift
fi

has_schick_dat() {
    local d="$1"
    [[ -d "$d" ]] && ( find "$d" -maxdepth 1 -iname "SCHICK.DAT" -print -quit 2>/dev/null | grep -q . || \
                       find "$d/DATA" -maxdepth 1 -iname "SCHICK.DAT" -print -quit 2>/dev/null | grep -q . )
}

# 1. SPIELPFAD aus erstem Parameter, SPIELPFAD.txt oder Umgebungsvariable uebernehmen
if [[ $# -ge 1 ]] && has_schick_dat "$1"; then
    SPIELPFAD="$(cd "$1" && pwd)"
    shift
fi

for sp_file in "$SCRIPT_DIR/SPIELPFAD.txt" "$SCRIPT_DIR/../SPIELPFAD.txt" "$PWD/SPIELPFAD.txt"; do
    if [[ -z "${SPIELPFAD:-}" && -f "$sp_file" ]]; then
        CAND_FILE="$(tr -d '\r\n' < "$sp_file")"
        if [[ "$CAND_FILE" =~ ^([A-Za-z]):[\\/](.*)$ ]]; then
            DRV="$(echo "${BASH_REMATCH[1]}" | tr 'A-Z' 'a-z')"
            REL="${BASH_REMATCH[2]//\\//}"
            CAND_FILE="/mnt/$DRV/$REL"
        fi
        if [[ -n "$CAND_FILE" ]] && has_schick_dat "$CAND_FILE"; then
            SPIELPFAD="$CAND_FILE"
            break
        fi
    fi
done

# 2. Automatische Erkennung (GOG, Steam, Lutris, Heroic, lokal)
if [[ -z "${SPIELPFAD:-}" ]] || ! has_schick_dat "${SPIELPFAD:-}"; then
    for cand in \
        "$PWD" \
        "$SCRIPT_DIR" \
        "$SCRIPT_DIR/.." \
        "$SCRIPT_DIR/../../Original/V3.02de_GOG" \
        "$SCRIPT_DIR/../../../Original/V3.02de_GOG" \
        "$HOME/GOG Games/Realms of Arkania - Blade of Destiny German" \
        "$HOME/GOG Games/Realms of Arkania" \
        "$HOME/GOG Games/Realms of Arkania 1" \
        "$HOME/GOG Games/Das Schwarze Auge - Die Schicksalsklinge" \
        "$HOME/GOG Games/Schicksalsklinge" \
        "$HOME/.local/share/Steam/steamapps/common/Realms of Arkania - Blade of Destiny Classic" \
        "$HOME/.local/share/Steam/steamapps/common/Realms of Arkania 1" \
        "$HOME/Games/gog/realms-of-arkania-blade-of-destiny" \
        "$HOME/Games/Heroic/Realms of Arkania 1" \
        "$HOME/.local/share/schicksalsklinge"; do
        if has_schick_dat "$cand"; then
            SPIELPFAD="$(cd "$cand" && pwd)"
            break
        fi
    done
fi

if [[ -z "${SPIELPFAD:-}" ]] || ! has_schick_dat "$SPIELPFAD"; then
    echo "[FEHLER] SCHICK.DAT wurde nicht gefunden!" >&2
    echo "Bitte Pfad in $SCRIPT_DIR/SPIELPFAD.txt eintragen oder als ersten Parameter uebergeben:" >&2
    echo "  ./start_schicksalsklinge.sh \"/pfad/zu/Schicksalsklinge\"" >&2
    exit 1
fi

# 3. Port-Binary suchen (4 Stufen: SCHICK_PORT_ORIG, SCHICK_PORT_WICHTIG, SCHICK_PORT_ALLES, SCHICK_PORT)
PORT_BIN=""
BASE_NAME="${PORT_EXE_NAME%.EXE}"
for bin in \
    "$SCRIPT_DIR/$BASE_NAME" \
    "$SCRIPT_DIR/build/$BASE_NAME" \
    "$SCRIPT_DIR/../build/$BASE_NAME" \
    "$SCRIPT_DIR/SCHICK_PORT" \
    "$SCRIPT_DIR/build/SCHICK_PORT" \
    "$SCRIPT_DIR/../build/SCHICK_PORT"; do
    if [[ -f "$bin" ]]; then
        PORT_BIN="$bin"
        break
    fi
done

if [[ -z "$PORT_BIN" ]]; then
    echo "[FEHLER] Port-Binary '$BASE_NAME' nicht gefunden." >&2
    exit 1
fi

# 4. Pruefen, ob Spielordner beschreibbar ist; sonst nach Spielstaende/ neben dem Binary umleiten
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
        mv -f "$SAVEDIR/PROBEN.CSV" "$SCRIPT_DIR/Probenlogs/PROBEN_${BASE_NAME}_${TS}.CSV"
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
    if [[ "${DSA_NO_PROBENLOG_WINDOW:-0}" != "1" ]]; then
        if [[ -f "$SCRIPT_DIR/Probenlog/probenlog_fenster.py" ]]; then
            python3 "$SCRIPT_DIR/Probenlog/probenlog_fenster.py" "$SAVEDIR/PROBEN.CSV" &
            LOG_PID="$!"
        elif [[ -f "$SCRIPT_DIR/Probenlog/probenlog.py" ]]; then
            python3 "$SCRIPT_DIR/Probenlog/probenlog.py" --live "$SAVEDIR/PROBEN.CSV" &
            LOG_PID="$!"
        fi
    fi
fi

echo "Starte Schicksalsklinge ($BASE_NAME) mit Spieldaten aus: $SPIELPFAD"
"$PORT_BIN" --gamedir "$SPIELPFAD" "$@"
