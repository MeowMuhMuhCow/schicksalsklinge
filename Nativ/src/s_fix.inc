; s_fix.inc - Fehlerbehebungen per Schalter fuer TASM (erzeugt von Werkzeuge/mkfix.py, siehe s_fix.h)
ifndef FIX_BUGS
FIX_BUGS = 0
endif
ifndef FIX_EXPLOITS
if FIX_BUGS gt 0
FIX_EXPLOITS = 1
else
FIX_EXPLOITS = 0
endif
endif
