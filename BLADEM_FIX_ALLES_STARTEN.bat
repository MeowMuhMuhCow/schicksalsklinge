@echo off
rem ============================================================
rem  BLADEM_FIX_ALLES_STARTEN.bat - startet die Fix-Fassung
rem  Build\BLADEM_FIX_ALLES.EXE (alle Fehlerbehebungen)
rem  mit den GOG-Spieldaten. Das Original im Spielordner wird nicht
rem  angefasst. Die EXE wird als BLADEM.EXE nach Build\Fix_Alles
rem  kopiert (DOS kennt nur 8.3-Namen). Spielstaende landen in
rem  Build\Testspielstaende_Fix_Alles, getrennt von den echten
rem  Spielstaenden und von den Tests der anderen Fassungen.
rem  Gebaut wird sie mit Bauen\BAUEN.bat alles.
rem  ALLE BEHEBUNGEN SIND IM SPIEL UNGETESTET - Tester willkommen
rem  (Liste: Bauen\FIXES.md).
rem  Den Spielordner findet SPIELPFAD_SUCHEN.cmd (oder fragt nach).
rem ============================================================

set "HIER=%~dp0"
set "BUILD=%HIER%Build"
set "SAVES=%BUILD%\Testspielstaende_Fix_Alles"
set "FIXEXE=%BUILD%\BLADEM_FIX_ALLES.EXE"
set "RUN=%BUILD%\Fix_Alles"
set "CONF=%TEMP%\dosbox_bladem_fix_alles.conf"

if not exist "%FIXEXE%" goto keinexe
call "%HIER%SPIELPFAD_SUCHEN.cmd"
if not defined SPIEL goto keinspiel
set "GOG=%SPIEL%"
if not exist "%SAVES%" mkdir "%SAVES%"
if not exist "%RUN%" mkdir "%RUN%"
copy /y "%FIXEXE%" "%RUN%\BLADEM.EXE" >nul

rem --- DOSBox-Startbefehle in eine kleine Zusatz-Konfiguration schreiben
rem     C: = Spielordner (nur lesen), Aenderungen gehen nach Testspielstaende_Fix_Alles
rem     D: = CD-Abbild von GOG (CD-Audio)
rem     E: = Build\Fix_Alles (die Fix-EXE als BLADEM.EXE)
> "%CONF%" echo [sdl]
>>"%CONF%" echo fullscreen=false
>>"%CONF%" echo [autoexec]
>>"%CONF%" echo @echo off
>>"%CONF%" echo mount C "%GOG%"
>>"%CONF%" echo mount C "%SAVES%" -t overlay
if exist "%GOG%\game.inst" >>"%CONF%" echo imgmount D "%GOG%\game.inst" -t iso
>>"%CONF%" echo mount E "%RUN%"
>>"%CONF%" echo C:
>>"%CONF%" echo cls
>>"%CONF%" echo intro
>>"%CONF%" echo E:\BLADEM.EXE
>>"%CONF%" echo echo.
>>"%CONF%" echo echo BLADEM_FIX_ALLES.EXE (Fix-Fassung, im Spiel ungetestet) wurde beendet. Taste druecken zum Schliessen.
>>"%CONF%" echo pause
>>"%CONF%" echo exit

rem --- Einstellungen des Spielordners (Grafik, Sound), falls vorhanden, plus unsere Startbefehle
cd /d "%GOG%\DOSBOX"
if exist "..\dosbox_realms1.conf" start "" DOSBox.exe -conf "..\dosbox_realms1.conf" -conf "%CONF%" -noconsole
if not exist "..\dosbox_realms1.conf" start "" DOSBox.exe -conf "%CONF%" -noconsole
exit /b

:keinspiel
echo Ohne den Spielordner der Schicksalsklinge (GOG) kann das Spiel nicht starten.
pause
exit /b

:keinexe
echo Build\BLADEM_FIX_ALLES.EXE fehlt. Erst PATCHEN.bat ausfuehren (ohne Compiler) oder Bauen\BAUEN.bat alles.
pause
exit /b
