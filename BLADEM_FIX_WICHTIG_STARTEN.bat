@echo off
rem ============================================================
rem  BLADEM_FIX_WICHTIG_STARTEN.bat - startet die Fix-Fassung
rem  Build\BLADEM_FIX_WICHTIG.EXE (Fehlerbehebungen kritisch + mittel)
rem  mit den GOG-Spieldaten. Das Original im Spielordner wird nicht
rem  angefasst. Die EXE wird als BLADEM.EXE nach Build\Fix_Wichtig
rem  kopiert (DOS kennt nur 8.3-Namen). Spielstaende landen in
rem  Build\Testspielstaende_Fix_Wichtig, getrennt von den echten
rem  Spielstaenden und von den Tests der anderen Fassungen.
rem  Gebaut wird sie mit Bauen\BAUEN.bat wichtig.
rem  ALLE BEHEBUNGEN SIND IM SPIEL UNGETESTET - Tester willkommen
rem  (Liste: Bauen\FIXES.md).
rem  Den Spielordner findet SPIELPFAD_SUCHEN.cmd (oder fragt nach).
rem ============================================================

set "HIER=%~dp0"
set "BUILD=%HIER%Build"
set "SAVES=%BUILD%\Testspielstaende_Fix_Wichtig"
set "FIXEXE=%BUILD%\BLADEM_FIX_WICHTIG.EXE"
set "RUN=%BUILD%\Fix_Wichtig"
set "CONF=%TEMP%\dosbox_bladem_fix_wichtig.conf"

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
rem --- korrigierte Spieldaten (von PATCHEN.bat) in den Spielstand-Ordner legen
if exist "%BUILD%\SCHICK.DAT" copy /y "%BUILD%\SCHICK.DAT" "%SAVES%\SCHICK.DAT" >nul
rem --- Probenlog (Zusatzfunktion): an mit BLADEM_FIX_WICHTIG_MIT_PROBENLOG_STARTEN.bat, sonst aus.
rem     Das Spiel schreibt nur, wenn PROBEN.CSV da ist. Ein altes Log wandert nach Build\Probenlogs.
if exist "%SAVES%\PROBEN.CSV" for %%A in ("%SAVES%\PROBEN.CSV") do if %%~zA==0 del "%SAVES%\PROBEN.CSV"
if exist "%SAVES%\PROBEN.CSV" (
  if not exist "%BUILD%\Probenlogs" mkdir "%BUILD%\Probenlogs"
  for /f %%t in ('powershell -NoProfile -Command "Get-Date -Format yyyy-MM-dd_HH-mm-ss"') do move /y "%SAVES%\PROBEN.CSV" "%BUILD%\Probenlogs\PROBEN_Fix_Wichtig_%%t.CSV" >nul
)
if "%PROBENLOG%"=="1" (
  type nul > "%SAVES%\PROBEN.CSV"
  start "Probenlog" powershell -NoProfile -ExecutionPolicy Bypass -File "%HIER%Probenlog\probenlog_fenster.ps1" -Datei "%SAVES%\PROBEN.CSV"
)

rem --- Zusatzfunktionen (Vorgabe: beide aus), die Schalter liegen neben den Startdateien:
rem     HIMMEL_AN.bat / HIMMEL_AUS.bat                   lebendiger Stadthimmel  -> HIMMEL.AN im Spielstand-Ordner
rem     SPEICHERN_OHNE_AP_AN.bat / _AUS.bat              Speichern ohne AP-Verlust -> APFREI.AN im Spielstand-Ordner
if exist "%SAVES%\HIMMEL.AUS" del "%SAVES%\HIMMEL.AUS"
if exist "%HIER%HIMMEL_AN.txt" type nul > "%SAVES%\HIMMEL.AN"
if not exist "%HIER%HIMMEL_AN.txt" if exist "%SAVES%\HIMMEL.AN" del "%SAVES%\HIMMEL.AN"
if exist "%HIER%SPEICHERN_OHNE_AP.txt" type nul > "%SAVES%\APFREI.AN"
if not exist "%HIER%SPEICHERN_OHNE_AP.txt" if exist "%SAVES%\APFREI.AN" del "%SAVES%\APFREI.AN"

rem --- DOSBox-Startbefehle in eine kleine Zusatz-Konfiguration schreiben
rem     C: = Spielstand-Ordner (Kopie des Spielordners, dort landen auch die Spielstaende)
rem     D: = CD-Abbild von GOG (CD-Audio)
rem     E: = Build\Fix_Wichtig (die Fix-EXE als BLADEM.EXE)
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
>>"%CONF%" echo echo BLADEM_FIX_WICHTIG.EXE (Fix-Fassung, im Spiel ungetestet) wurde beendet. Taste druecken zum Schliessen.
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
echo Build\BLADEM_FIX_WICHTIG.EXE fehlt. Erst PATCHEN.bat ausfuehren (ohne Compiler) oder Bauen\BAUEN.bat wichtig.
pause
exit /b
