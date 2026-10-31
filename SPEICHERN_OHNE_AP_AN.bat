@echo off
rem ============================================================
rem  SPEICHERN_OHNE_AP_AN.bat - Speichern ausserhalb eines Tempels kostet keine
rem  Abenteuerpunkte mehr (Bugfix-Fassung ab 1.4a). Das ist eine Regelaenderung,
rem  keine Fehlerbehebung: Attic wollte damit haeufiges Speichern bremsen.
rem  Vorgabe: aus. Gilt fuer alle BLADEM_FIX_..._STARTEN.bat ab dem naechsten Start.
rem  Gegenstueck: SPEICHERN_OHNE_AP_AUS.bat
rem ============================================================
echo Speichern ohne AP-Verlust gewuenscht, wieder aus mit SPEICHERN_OHNE_AP_AUS.bat> "%~dp0SPEICHERN_OHNE_AP.txt"
echo Speichern ohne AP-Verlust ist AN: Speichern kostet auch ausserhalb von Tempeln keine AP.
echo Gilt ab dem naechsten Start.
pause
