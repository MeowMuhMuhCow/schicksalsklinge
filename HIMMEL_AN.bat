@echo off
rem ============================================================
rem  HIMMEL_AN.bat - schaltet den lebendigen Stadthimmel der Bugfix-Fassung
rem  (ab 1.4) an: ziehende Wolken, Sonne, Mond und Sterne. Vorgabe: aus.
rem  Gilt fuer alle BLADEM_FIX_..._STARTEN.bat ab dem naechsten Start.
rem  Gegenstueck: HIMMEL_AUS.bat
rem ============================================================
echo Lebendiger Himmel gewuenscht, wieder aus mit HIMMEL_AUS.bat> "%~dp0HIMMEL_AN.txt"
echo Lebendiger Himmel ist AN: ziehende Wolken, Sonne, Mond und Sterne.
echo Gilt ab dem naechsten Start.
pause
