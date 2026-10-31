@echo off
rem ============================================================
rem  HIMMEL_AN.bat - schaltet den lebendigen Stadthimmel der Bugfix-Fassung
rem  (ab 1.4) an: ziehende Wolken, Sonne, Mond und Sterne. Das ist die Vorgabe.
rem  Gilt fuer alle BLADEM_FIX_..._STARTEN.bat ab dem naechsten Start.
rem  Gegenstueck: HIMMEL_AUS.bat
rem ============================================================
if exist "%~dp0HIMMEL_AUS.txt" del "%~dp0HIMMEL_AUS.txt"
echo Lebendiger Himmel ist AN: ziehende Wolken, Sonne, Mond und Sterne.
echo Gilt ab dem naechsten Start.
pause
