# gemeinsam.sh - wird von patchen.sh und starten.sh eingelesen (nicht direkt aufrufen).
# Findet den Spielordner und DOSBox. Die Angaben zum Spiel stehen in spiel.conf.

LINUX="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(dirname "$LINUX")"
BUILD="$REPO/Build"
. "$LINUX/spiel.conf"
MERK="$LINUX/SPIELPFAD.txt"

# Ordner mit der Original-EXE unterhalb von $1 (Gross-/Kleinschreibung egal), sonst nichts
_spiel_in() {
	[ -d "$1" ] || return 1
	local f
	f="$(find "$1" -maxdepth 4 -type f -iname "$ORIG_EXE" -not -path "*/Linux_C_*" 2>/dev/null | sort | head -n 1)"
	[ -n "$f" ] && ( cd "$(dirname "$f")" && pwd )
}

# Ergebnis in SPIEL. Reihenfolge: Umgebungsvariable SPIEL, gemerkter Pfad, uebliche Orte, Nachfrage.
spielpfad_suchen() {
	local s b
	if [ -n "${SPIEL:-}" ]; then
		s="$(_spiel_in "$SPIEL")" && { SPIEL="$s"; echo "$SPIEL" > "$MERK"; echo "Spielordner: $SPIEL"; return 0; }
		echo "In SPIEL=$SPIEL liegt keine $ORIG_EXE."; SPIEL=""
	fi
	if [ -f "$MERK" ]; then
		s="$(_spiel_in "$(head -n 1 "$MERK")")" && { SPIEL="$s"; return 0; }
	fi
	for b in "$HOME/GOG Games" "$HOME/Games" "$HOME/games" "$HOME/Spiele" "$HOME/.wine/drive_c/GOG Games" \
	         "$HOME/.local/share/Steam/steamapps/common" "$HOME/.steam/steam/steamapps/common"; do
		[ -d "$b" ] || continue
		s="$(_spiel_in "$b")" && { SPIEL="$s"; break; }
	done
	if [ -z "${SPIEL:-}" ]; then
		echo
		echo "$SPIELNAME wurde nicht automatisch gefunden."
		echo "Bitte den Spielordner angeben (der Ordner mit $ORIG_EXE, z. B. aus der GOG-Installation"
		echo "oder mit innoextract aus dem GOG-Installer entpackt):"
		if command -v zenity >/dev/null 2>&1 && [ -n "${DISPLAY:-}${WAYLAND_DISPLAY:-}" ]; then
			b="$(zenity --file-selection --directory --title="$SPIELNAME-Spielordner waehlen (mit $ORIG_EXE)" 2>/dev/null)"
		else
			read -r -p "> " b
		fi
		b="${b/#\~/$HOME}"
		s="$(_spiel_in "$b")" || { echo "Dort liegt keine $ORIG_EXE."; return 1; }
		SPIEL="$s"
	fi
	echo "$SPIEL" > "$MERK"
	echo "Spielordner: $SPIEL   (zum Neu-Waehlen Linux/SPIELPFAD.txt loeschen)"
}

# Ergebnis in DOSBOX_CMD (Array). Eigene Wahl: DOSBOX=/pfad/zu/dosbox
dosbox_finden() {
	local d
	if [ -n "${DOSBOX:-}" ]; then DOSBOX_CMD=("$DOSBOX"); return 0; fi
	for d in dosbox-staging dosbox dosbox-x; do
		command -v "$d" >/dev/null 2>&1 && { DOSBOX_CMD=("$d"); return 0; }
	done
	if command -v flatpak >/dev/null 2>&1 && flatpak info io.github.dosbox-staging >/dev/null 2>&1; then
		DOSBOX_CMD=(flatpak run io.github.dosbox-staging); return 0
	fi
	echo "DOSBox nicht gefunden. Bitte DOSBox Staging oder DOSBox installieren"
	echo "(z. B. sudo apt install dosbox  bzw.  flatpak install flathub io.github.dosbox-staging)."
	return 1
}

# Datei im Ordner $1 (nur oberste Ebene) mit dem Namen $2, Gross-/Kleinschreibung egal
datei_in() {
	find "$1" -maxdepth 1 -type f -iname "$2" 2>/dev/null | sort | head -n 1
}
