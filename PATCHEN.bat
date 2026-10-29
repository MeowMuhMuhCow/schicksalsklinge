@echo off
rem ============================================================
rem  PATCHEN.bat - erzeugt die Bugfix-Fassungen OHNE Compiler aus
rem  der eigenen SCHICKM.EXE der GOG-Fassung (Schicksalsklinge).
rem  Die Original-EXE wird nur gelesen und NICHT veraendert.
rem  Ergebnis: Build\BLADEM_FIX_WICHTIG.EXE und Build\BLADEM_FIX_ALLES.EXE
rem  Danach spielen mit BLADEM_FIX_WICHTIG_STARTEN.bat bzw. _ALLES_STARTEN.bat
rem  Den Spielordner findet SPIELPFAD_SUCHEN.cmd (oder fragt nach).
rem ============================================================
setlocal
title Schicksalsklinge - Bugfix-Patch
call "%~dp0SPIELPFAD_SUCHEN.cmd"
if not defined SPIEL goto keinspiel
echo Spielordner: %SPIEL%
echo.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Patch\patchen.ps1" -Spiel "%SPIEL%" -Name SCHICKM.EXE -Ziel "%~dp0Build"
if errorlevel 1 goto fehler
echo.
echo Jetzt spielen: BLADEM_FIX_WICHTIG_STARTEN.bat (kritische + mittlere Fehler behoben)
echo          oder: BLADEM_FIX_ALLES_STARTEN.bat   (alle behebbaren Fehler)
echo Alle Behebungen sind im Spiel noch ungetestet - Rueckmeldungen willkommen.
echo.
pause
exit /b 0
:fehler
echo.
echo Patch nicht moeglich (siehe Meldung oben).
pause
exit /b 1
:keinspiel
echo Ohne den GOG-Spielordner geht es nicht.
pause
exit /b 1
