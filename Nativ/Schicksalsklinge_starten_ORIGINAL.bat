@echo off
setlocal
REM ===========================================================================
REM Startskript fuer Das Schwarze Auge: Die Schicksalsklinge (Nativer Port)
REM ===========================================================================

set "SCRIPT_DIR=%~dp0"
if not defined PORT_EXE_NAME set "PORT_EXE_NAME=SCHICK_PORT_ORIG.EXE"
if not defined PROBENLOG set "PROBENLOG=0"

REM Optionaler Schalter --probenlog / log als erster Parameter
if /i "%~1"=="--probenlog" (
    set "PROBENLOG=1"
    shift
) else if /i "%~1"=="log" (
    set "PROBENLOG=1"
    shift
)

REM 1. SPIELPFAD aus erstem Parameter oder Umgebungsvariable uebernehmen
if not "%~1"=="" (
    if exist "%~1\SCHICK.DAT" (
        set "SPIELPFAD=%~1"
        shift
    )
)

REM 2. Gemerkten Pfad aus SPIELPFAD.txt pruefen
if "%SPIELPFAD%"=="" (
    if exist "%SCRIPT_DIR%SPIELPFAD.txt" (
        set /p SPIELPFAD=<"%SCRIPT_DIR%SPIELPFAD.txt"
    )
)
if not "%SPIELPFAD%"=="" (
    if not exist "%SPIELPFAD%\SCHICK.DAT" set "SPIELPFAD="
)

REM 3. Automatische Erkennung ueblicher GOG- und Projekt-Orte
if "%SPIELPFAD%"=="" (
    if exist "%CD%\SCHICK.DAT" (
        set "SPIELPFAD=%CD%"
    ) else if exist "%SCRIPT_DIR%SCHICK.DAT" (
        set "SPIELPFAD=%SCRIPT_DIR%"
    ) else if exist "%SCRIPT_DIR%..\Original\V3.02de_GOG\SCHICK.DAT" (
        set "SPIELPFAD=%SCRIPT_DIR%..\Original\V3.02de_GOG"
    ) else if exist "%SCRIPT_DIR%..\..\Original\V3.02de_GOG\SCHICK.DAT" (
        set "SPIELPFAD=%SCRIPT_DIR%..\..\Original\V3.02de_GOG"
    ) else if exist "%ProgramFiles(x86)%\GOG Galaxy\Games\Realms of Arkania\SCHICK.DAT" (
        set "SPIELPFAD=%ProgramFiles(x86)%\GOG Galaxy\Games\Realms of Arkania"
    ) else if exist "%ProgramFiles%\GOG Galaxy\Games\Realms of Arkania\SCHICK.DAT" (
        set "SPIELPFAD=%ProgramFiles%\GOG Galaxy\Games\Realms of Arkania"
    ) else if exist "C:\GOG Games\Realms of Arkania\SCHICK.DAT" (
        set "SPIELPFAD=C:\GOG Games\Realms of Arkania"
    ) else if exist "D:\GOG Games\Realms of Arkania\SCHICK.DAT" (
        set "SPIELPFAD=D:\GOG Games\Realms of Arkania"
    ) else if exist "C:\GOG Games\Realms of Arkania - Blade of Destiny\SCHICK.DAT" (
        set "SPIELPFAD=C:\GOG Games\Realms of Arkania - Blade of Destiny"
    ) else if exist "D:\GOG Games\Realms of Arkania - Blade of Destiny\SCHICK.DAT" (
        set "SPIELPFAD=D:\GOG Games\Realms of Arkania - Blade of Destiny"
    ) else if exist "C:\GOG Games\Realms of Arkania - Blade of Destiny German\SCHICK.DAT" (
        set "SPIELPFAD=C:\GOG Games\Realms of Arkania - Blade of Destiny German"
    ) else if exist "C:\Program Files (x86)\GOG Galaxy\Games\Realms of Arkania - Blade of Destiny German\SCHICK.DAT" (
        set "SPIELPFAD=C:\Program Files (x86)\GOG Galaxy\Games\Realms of Arkania - Blade of Destiny German"
    )
)

if not "%SPIELPFAD%"=="" goto spielpfad_ok

echo [FEHLER] SCHICK.DAT wurde nicht gefunden!
echo Bitte setzen Sie die Umgebungsvariable SPIELPFAD auf den Ordner Ihres Originalspiels
echo oder uebergeben Sie den Pfad als Parameter:
echo   start_schicksalsklinge.bat "C:\Pfad\zu\Schicksalsklinge"
exit /b 1

:spielpfad_ok
REM 4. Port-EXE suchen (lokal oder in build\)
set "PORT_EXE="
if exist "%SCRIPT_DIR%%PORT_EXE_NAME%" (
    set "PORT_EXE=%SCRIPT_DIR%%PORT_EXE_NAME%"
) else if exist "%SCRIPT_DIR%build\%PORT_EXE_NAME%" (
    set "PORT_EXE=%SCRIPT_DIR%build\%PORT_EXE_NAME%"
) else if exist "%SCRIPT_DIR%..\build\%PORT_EXE_NAME%" (
    set "PORT_EXE=%SCRIPT_DIR%..\build\%PORT_EXE_NAME%"
)

if not "%PORT_EXE%"=="" goto exe_ok
echo [HINWEIS] %PORT_EXE_NAME% noch nicht gebaut. Starte Build via bauen_port.py ...
python "%SCRIPT_DIR%bauen_port.py"
if exist "%SCRIPT_DIR%build\%PORT_EXE_NAME%" (
    set "PORT_EXE=%SCRIPT_DIR%build\%PORT_EXE_NAME%"
    goto exe_ok
)
echo [FEHLER] %PORT_EXE_NAME% konnte nicht gefunden oder gebaut werden.
exit /b 1

:exe_ok
REM 5. Pruefen, ob der Spielordner beschreibbar ist; sonst Spielstaende/PROBEN.CSV
REM    in den Ordner "Spielstaende" neben der EXE umleiten und einmal melden.
set "SAVEDIR=%SPIELPFAD%"
if "%DSA_FORCE_READONLY_GAMEDIR%"=="1" goto umleiten_spielstaende
set "WTEST=%SPIELPFAD%\.schick_wtest_%RANDOM%.tmp"
type nul > "%WTEST%" 2>nul
if not exist "%WTEST%" goto umleiten_spielstaende
del "%WTEST%" >nul 2>nul
goto ordner_bereit

:umleiten_spielstaende
for %%D in ("%PORT_EXE%") do set "EXE_DIR=%%~dpD"
set "SAVEDIR=%EXE_DIR%Spielstaende"
set "DSA_SAVE_DIR=%SAVEDIR%"
if not exist "%SAVEDIR%" mkdir "%SAVEDIR%"
if not exist "%SAVEDIR%\TEMP" mkdir "%SAVEDIR%\TEMP"
if exist "%SPIELPFAD%\GAMES.NAM" if not exist "%SAVEDIR%\GAMES.NAM" copy /y "%SPIELPFAD%\GAMES.NAM" "%SAVEDIR%\GAMES.NAM" >nul 2>nul
for %%F in ("%SPIELPFAD%\*.GAM" "%SPIELPFAD%\*.CHR") do (
    if not exist "%SAVEDIR%\%%~nxF" copy /y "%%~fF" "%SAVEDIR%\%%~nxF" >nul 2>nul
)
echo [HINWEIS] Der Spielordner "%SPIELPFAD%" ist nicht beschreibbar.
echo           Spielstaende und PROBEN.CSV werden in den Ordner "Spielstaende" neben der EXE umgeleitet:
echo           "%SAVEDIR%"

:ordner_bereit
REM 6. Probenlog (Zusatzfunktion): alte PROBEN.CSV mit Datum nach Probenlogs\ verschieben.
REM    Bei PROBENLOG=1 neue PROBEN.CSV anlegen und das Live-Fenster (probenlog_fenster.ps1) oeffnen.
set "LOGTAG=Fix_Alles"
if /i "%PORT_EXE_NAME%"=="SCHICK_PORT_WICHTIG.EXE" set "LOGTAG=Fix_Wichtig"

if exist "%SAVEDIR%\PROBEN.CSV" for %%A in ("%SAVEDIR%\PROBEN.CSV") do if %%~zA==0 del "%SAVEDIR%\PROBEN.CSV" >nul 2>nul
if not exist "%SAVEDIR%\PROBEN.CSV" goto nach_archiv_probenlog

if not exist "%SCRIPT_DIR%Probenlogs" mkdir "%SCRIPT_DIR%Probenlogs"
set "ZEITSTEMPEL="
for /f %%t in ('powershell -NoProfile -Command "Get-Date -Format yyyy-MM-dd_HH-mm-ss"') do set "ZEITSTEMPEL=%%t"
if not defined ZEITSTEMPEL set "ZEITSTEMPEL=alt"
if /i not "%SAVEDIR%\Probenlogs"=="%SCRIPT_DIR%Probenlogs" (
    if not exist "%SAVEDIR%\Probenlogs" mkdir "%SAVEDIR%\Probenlogs" 2>nul
    copy /y "%SAVEDIR%\PROBEN.CSV" "%SAVEDIR%\Probenlogs\PROBEN_%LOGTAG%_%ZEITSTEMPEL%.CSV" >nul 2>nul
)
move /y "%SAVEDIR%\PROBEN.CSV" "%SCRIPT_DIR%Probenlogs\PROBEN_%LOGTAG%_%ZEITSTEMPEL%.CSV" >nul 2>nul

:nach_archiv_probenlog
if not "%PROBENLOG%"=="1" goto start_spiel
type nul > "%SAVEDIR%\PROBEN.CSV"
set "PS1_PFAD="
if exist "%SCRIPT_DIR%Probenlog\probenlog_fenster.ps1" set "PS1_PFAD=%SCRIPT_DIR%Probenlog\probenlog_fenster.ps1"
if not defined PS1_PFAD if exist "%SCRIPT_DIR%..\Probenlog\probenlog_fenster.ps1" set "PS1_PFAD=%SCRIPT_DIR%..\Probenlog\probenlog_fenster.ps1"
if not defined PS1_PFAD if exist "%SCRIPT_DIR%probenlog_fenster.ps1" set "PS1_PFAD=%SCRIPT_DIR%probenlog_fenster.ps1"
if defined PS1_PFAD if not "%DSA_NO_PROBENLOG_WINDOW%"=="1" (
    start "Probenlog" powershell -NoProfile -ExecutionPolicy Bypass -File "%PS1_PFAD%" -Datei "%SAVEDIR%\PROBEN.CSV"
)

:start_spiel
echo Starte Schicksalsklinge (%PORT_EXE_NAME%) mit Spieldaten aus: "%SPIELPFAD%"
"%PORT_EXE%" "%SPIELPFAD%" %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
