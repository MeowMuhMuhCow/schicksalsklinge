@echo off
rem ============================================================
rem  HIMMEL_AUS.bat - schaltet den lebendigen Stadthimmel der Bugfix-Fassung
rem  (ab 1.4) aus: das Spiel zeigt wieder den Original-Himmel.
rem  Gilt fuer alle BLADEM_FIX_..._STARTEN.bat ab dem naechsten Start.
rem  Gegenstueck: HIMMEL_AN.bat
rem ============================================================
echo Original-Himmel gewuenscht, wieder an mit HIMMEL_AN.bat> "%~dp0HIMMEL_AUS.txt"
echo Lebendiger Himmel ist AUS: das Spiel zeigt den Original-Himmel.
echo Gilt ab dem naechsten Start.
pause
