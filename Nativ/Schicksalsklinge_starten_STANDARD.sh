#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export PORT_EXE_NAME="SCHICK_PORT"
export PROBENLOG="0"
exec "$SCRIPT_DIR/start_schicksalsklinge.sh" "$@"
