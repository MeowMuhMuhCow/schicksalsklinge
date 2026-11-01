@echo off
rem ============================================================
rem  BLADEM_STARTEN.bat - startet die aus dem Quellcode gebaute
rem  Schicksalsklinge (Build\BLADEM.EXE) mit den GOG-Spieldaten.
rem  Das Original im Spielordner wird nicht angefasst.
rem  Spielstaende landen in Build\Testspielstaende, nicht bei
rem  deinen echten Spielstaenden.
rem  Den Spielordner findet SPIELPFAD_SUCHEN.cmd (oder fragt nach).
rem ============================================================

set "HIER=%~dp0"
set "BUILD=%HIER%Build"
set "SAVES=%BUILD%\Testspielstaende"
set "CONF=%TEMP%\dosbox_bladem_build.conf"

if not exist "%BUILD%\BLADEM.EXE" goto keinexe
call "%HIER%SPIELPFAD_SUCHEN.cmd"
if not defined SPIEL goto keinspiel
set "GOG=%SPIEL%"
if not exist "%SAVES%" mkdir "%SAVES%"
rem --- Spielordner in den Spielstand-Ordner spiegeln: nur fehlende Dateien, vorhandene (Spielstaende,
rem     korrigierte Spieldaten) bleiben. Die DOSBox 0.74 von GOG kennt keine Overlay-Laufwerke, deshalb ist
rem     dieser Ordner das ganze Laufwerk C:. Alte Spielstaende des Originals (Windows-VirtualStore) kommen mit.
rem     Nur die Dateien direkt im Spielordner, keine Unterordner: so kann ein Bugfix-Ordner, der im
rem     Spielordner liegt, nicht in sich selbst kopiert werden.
robocopy "%GOG%" "%SAVES%" /XC /XN /XO /R:1 /W:1 /XF game.gog game.inst *.pdf *.zip *.lnk *.ico *.dll *.ini *.info unins000.* goggame* >nul
set "VSTORE=%LOCALAPPDATA%\VirtualStore%GOG:~2%"
if exist "%VSTORE%" robocopy "%VSTORE%" "%SAVES%" /XC /XN /XO /R:1 /W:1 >nul
rem     Einzige Ausnahme: TEMP (dort liegen die Start-Helden, das Spiel legt den Ordner nicht selbst an)
if not exist "%SAVES%\TEMP" mkdir "%SAVES%\TEMP"
if exist "%GOG%\TEMP" robocopy "%GOG%\TEMP" "%SAVES%\TEMP" /XC /XN /XO /R:1 /W:1 >nul
if exist "%VSTORE%\TEMP" robocopy "%VSTORE%\TEMP" "%SAVES%\TEMP" /XC /XN /XO /R:1 /W:1 >nul

rem --- DOSBox-Startbefehle in eine kleine Zusatz-Konfiguration schreiben
rem     C: = Spielstand-Ordner (Kopie des Spielordners, dort landen auch die Spielstaende)
rem     D: = CD-Abbild von GOG (CD-Audio)
rem     E: = unser Build
> "%CONF%" echo [sdl]
>>"%CONF%" echo fullscreen=false
>>"%CONF%" echo [autoexec]
>>"%CONF%" echo @echo off
>>"%CONF%" echo mount C "%SAVES%"
if exist "%GOG%\game.inst" >>"%CONF%" echo imgmount D "%GOG%\game.inst" -t iso
>>"%CONF%" echo mount E "%BUILD%"
>>"%CONF%" echo C:
>>"%CONF%" echo cls
>>"%CONF%" echo intro
>>"%CONF%" echo E:\BLADEM.EXE
>>"%CONF%" echo echo.
>>"%CONF%" echo echo BLADEM.EXE (aus dem Quellcode gebaut) wurde beendet. Taste druecken zum Schliessen.
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
echo Build\BLADEM.EXE fehlt neben dieser Datei. Erst Bauen\BAUEN.bat ausfuehren.
pause
exit /b
