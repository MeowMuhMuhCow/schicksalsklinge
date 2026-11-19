#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export PORT_EXE_NAME="SCHICK_PORT_ALLES"
export PROBENLOG="1"
exec "$SCRIPT_DIR/start_schicksalsklinge.sh" "$@"
