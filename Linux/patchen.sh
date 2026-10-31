#!/usr/bin/env bash
# patchen.sh - erzeugt die Bugfix-Fassungen OHNE Compiler aus der eigenen Original-EXE (GOG).
# Linux-Gegenstueck zu PATCHEN.bat. Die Originaldateien werden nur gelesen, nie veraendert.
# Ergebnis in Build/. Danach spielen mit:  bash Linux/starten.sh wichtig   (oder: alles)
# Braucht: Python 3.
. "$(dirname "${BASH_SOURCE[0]}")/gemeinsam.sh"

command -v python3 >/dev/null 2>&1 || { echo "Python 3 fehlt (z. B. sudo apt install python3)."; exit 1; }
spielpfad_suchen || exit 1
echo
python3 "$REPO/Patch/patchen.py" "$SPIEL" "$ORIG_EXE" "$BUILD" "${EXE}_FIX_*.ndp" || { echo "Patch nicht moeglich (siehe oben)."; exit 1; }
if [ -n "$DATEN" ]; then
	echo
	echo "Spieldaten ($DATEN) mit den Korrekturen aus dem Patch von NRS:"
	python3 "$REPO/Patch/patchen.py" "$SPIEL" "$DATEN" "$BUILD" "$DATEN.ndp" || { echo "Patch nicht moeglich (siehe oben)."; exit 1; }
fi
echo
echo "Jetzt spielen:  bash Linux/starten.sh wichtig   (kritische + mittlere Fehler behoben)"
echo "          oder: bash Linux/starten.sh alles     (alle behebbaren Fehler)"
