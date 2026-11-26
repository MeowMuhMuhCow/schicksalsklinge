@echo off
rem ============================================================
rem  BLADEM_ORIGINAL_MIT_PROBENLOG_STARTEN.bat - startet das
rem  Original ohne jede Fehlerbehebung (Build\BLADEM_ORIGINAL_PROBENLOG.EXE,
rem  FIX_BUGS 0), nur mit dem Probenlog: jede Probe erscheint live in
rem  einem zweiten Fenster und wird in
rem  Build\Testspielstaende_Original_Probenlog\PROBEN.CSV mitgeschrieben.
rem  Wuerfelgleich mit BLADEM.EXE: dieselben Zufallswerte in derselben
rem  Reihenfolge. Keine Zusatzfunktionen, keine korrigierten Spieldaten,
rem  die Handbuchfrage (Kopierschutz) bleibt wie im Original.
rem  Gebaut wird sie mit Bauen\BAUEN.bat probenlog (oder PATCHEN.bat).
rem  Hinterher ansehen: Probenlog\Probenlog_Schicksalsklinge.html
rem  Den Spielordner findet SPIELPFAD_SUCHEN.cmd (oder fragt nach).
rem ============================================================

set "HIER=%~dp0"
set "BUILD=%HIER%Build"
set "SAVES=%BUILD%\Testspielstaende_Original_Probenlog"
set "FIXEXE=%BUILD%\BLADEM_ORIGINAL_PROBENLOG.EXE"
set "RUN=%BUILD%\Original_Probenlog"
set "CONF=%TEMP%\dosbox_bladem_original_probenlog.conf"

if not exist "%FIXEXE%" goto keinexe
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
if not exist "%RUN%" mkdir "%RUN%"
copy /y "%FIXEXE%" "%RUN%\BLADEM.EXE" >nul
rem --- KEINE korrigierten Spieldaten: das Original spielt mit der unveraenderten SCHICK.DAT aus dem Spielordner.
rem     (Eine Build\SCHICK.DAT von PATCHEN.bat wird hier bewusst nicht benutzt.)
rem --- Probenlog: immer an. Das Spiel schreibt nur, wenn PROBEN.CSV da ist. Ein altes Log wandert nach Build\Probenlogs.
if exist "%SAVES%\PROBEN.CSV" for %%A in ("%SAVES%\PROBEN.CSV") do if %%~zA==0 del "%SAVES%\PROBEN.CSV"
if exist "%SAVES%\PROBEN.CSV" (
  if not exist "%BUILD%\Probenlogs" mkdir "%BUILD%\Probenlogs"
  for /f %%t in ('powershell -NoProfile -Command "Get-Date -Format yyyy-MM-dd_HH-mm-ss"') do move /y "%SAVES%\PROBEN.CSV" "%BUILD%\Probenlogs\PROBEN_Original_%%t.CSV" >nul
)
type nul > "%SAVES%\PROBEN.CSV"
start "Probenlog" powershell -NoProfile -ExecutionPolicy Bypass -File "%HIER%Probenlog\probenlog_fenster.ps1" -Datei "%SAVES%\PROBEN.CSV"

rem --- DOSBox-Startbefehle in eine kleine Zusatz-Konfiguration schreiben
rem     C: = Spielstand-Ordner (Kopie des Spielordners, dort landen auch die Spielstaende)
rem     D: = CD-Abbild von GOG (CD-Audio)
rem     E: = Build\Original_Probenlog (die EXE als BLADEM.EXE)
> "%CONF%" echo [sdl]
>>"%CONF%" echo fullscreen=false
>>"%CONF%" echo [autoexec]
>>"%CONF%" echo @echo off
>>"%CONF%" echo mount C "%SAVES%"
if exist "%GOG%\game.inst" >>"%CONF%" echo imgmount D "%GOG%\game.inst" -t iso
>>"%CONF%" echo mount E "%RUN%"
>>"%CONF%" echo C:
>>"%CONF%" echo cls
>>"%CONF%" echo intro
>>"%CONF%" echo E:\BLADEM.EXE
>>"%CONF%" echo echo.
>>"%CONF%" echo echo BLADEM_ORIGINAL_PROBENLOG.EXE (Original mit Probenlog) wurde beendet. Taste druecken zum Schliessen.
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
echo Build\BLADEM_ORIGINAL_PROBENLOG.EXE fehlt. Erst PATCHEN.bat ausfuehren (ohne Compiler) oder Bauen\BAUEN.bat probenlog.
pause
exit /b
