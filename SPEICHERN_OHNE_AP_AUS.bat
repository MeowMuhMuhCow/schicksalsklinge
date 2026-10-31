@echo off
rem ============================================================
rem  SPEICHERN_OHNE_AP_AUS.bat - Speichern ausserhalb eines Tempels kostet wieder
rem  jeden Helden 1 Abenteuerpunkt, wie im Original. Das ist die Vorgabe.
rem  Gegenstueck: SPEICHERN_OHNE_AP_AN.bat
rem ============================================================
if exist "%~dp0SPEICHERN_OHNE_AP.txt" del "%~dp0SPEICHERN_OHNE_AP.txt"
echo Speichern ohne AP-Verlust ist AUS: wie im Original kostet Speichern ausserhalb von Tempeln 1 AP je Held.
echo Gilt ab dem naechsten Start.
pause
