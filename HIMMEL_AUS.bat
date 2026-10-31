@echo off
rem ============================================================
rem  HIMMEL_AUS.bat - schaltet den lebendigen Stadthimmel der Bugfix-Fassung
rem  aus: das Spiel zeigt den Original-Himmel. Das ist die Vorgabe.
rem  Gilt fuer alle BLADEM_FIX_..._STARTEN.bat ab dem naechsten Start.
rem  Gegenstueck: HIMMEL_AN.bat
rem ============================================================
if exist "%~dp0HIMMEL_AN.txt" del "%~dp0HIMMEL_AN.txt"
if exist "%~dp0HIMMEL_AUS.txt" del "%~dp0HIMMEL_AUS.txt"
echo Lebendiger Himmel ist AUS: das Spiel zeigt den Original-Himmel.
echo Gilt ab dem naechsten Start.
pause
