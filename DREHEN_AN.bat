@echo off
rem ============================================================
rem  DREHEN_AN.bat - schaltet den fluessigen Schwenk beim Drehen
rem  der Bugfix-Fassung (ab 1.5f) an: weicher 90-Grad-Schwenk in
rem  Stadt und Dungeon. Das ist die Vorgabe.
rem  Gilt fuer alle BLADEM_FIX_..._STARTEN.bat ab dem naechsten Start.
rem  Gegenstueck: DREHEN_AUS.bat
rem ============================================================
if exist "%~dp0DREHEN_AUS.txt" del "%~dp0DREHEN_AUS.txt"
echo Fluessiges Drehen ist AN: weicher Schwenk in Stadt und Dungeon.
echo Gilt ab dem naechsten Start.
pause
