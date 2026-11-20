@echo off
rem ============================================================
rem  DREHEN_AUS.bat - schaltet den fluessigen Schwenk beim Drehen
rem  der Bugfix-Fassung (ab 1.5f) aus: das Spiel dreht wieder hart
rem  um 90 Grad wie im Original.
rem  Gilt fuer alle BLADEM_FIX_..._STARTEN.bat ab dem naechsten Start.
rem  Gegenstueck: DREHEN_AN.bat
rem ============================================================
echo Hartes Drehen gewuenscht, wieder an mit DREHEN_AN.bat> "%~dp0DREHEN_AUS.txt"
echo Fluessiges Drehen ist AUS: das Spiel dreht hart wie im Original.
echo Gilt ab dem naechsten Start.
pause
