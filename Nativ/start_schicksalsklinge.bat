@echo off
setlocal EnableDelayedExpansion
REM ===========================================================================
REM Startskript fuer Das Schwarze Auge: Die Schicksalsklinge (Nativer Port)
REM ===========================================================================

set "SCRIPT_DIR=%~dp0"

REM 1. SPIELPFAD aus erstem Parameter oder Umgebungsvariable uebernehmen
if not "%~1"=="" (
    if exist "%~1\SCHICK.DAT" (
        set "SPIELPFAD=%~1"
        shift
    )
)

REM 2. Automatische Erkennung, falls SPIELPFAD noch nicht gesetzt ist
if "%SPIELPFAD%"=="" (
    if exist "%CD%\SCHICK.DAT" (
        set "SPIELPFAD=%CD%"
    ) else if exist "%SCRIPT_DIR%SCHICK.DAT" (
        set "SPIELPFAD=%SCRIPT_DIR%"
    ) else if exist "%SCRIPT_DIR%..\Original\V3.02de_GOG\SCHICK.DAT" (
        set "SPIELPFAD=%SCRIPT_DIR%..\Original\V3.02de_GOG"
    ) else if exist "%SCRIPT_DIR%..\..\Original\V3.02de_GOG\SCHICK.DAT" (
        set "SPIELPFAD=%SCRIPT_DIR%..\..\Original\V3.02de_GOG"
    ) else if exist "C:\Program Files (x86)\GOG Galaxy\Games\Realms of Arkania\SCHICK.DAT" (
        set "SPIELPFAD=C:\Program Files (x86)\GOG Galaxy\Games\Realms of Arkania"
    ) else if exist "C:\GOG Games\Realms of Arkania - Blade of Destiny German\SCHICK.DAT" (
        set "SPIELPFAD=C:\GOG Games\Realms of Arkania - Blade of Destiny German"
    ) else if exist "C:\Program Files (x86)\GOG Galaxy\Games\Realms of Arkania - Blade of Destiny German\SCHICK.DAT" (
        set "SPIELPFAD=C:\Program Files (x86)\GOG Galaxy\Games\Realms of Arkania - Blade of Destiny German"
    )
)

if "%SPIELPFAD%"=="" (
    echo [FEHLER] SCHICK.DAT wurde nicht gefunden!
    echo Bitte setzen Sie die Umgebungsvariable SPIELPFAD auf den Ordner Ihres Originalspiels
    echo oder uebergeben Sie den Pfad als Parameter:
    echo   start_schicksalsklinge.bat "C:\Pfad\zu\Schicksalsklinge"
    exit /b 1
)

REM 3. Port-EXE suchen (lokal oder in build\)
set "PORT_EXE="
if exist "%SCRIPT_DIR%SCHICK_PORT.EXE" (
    set "PORT_EXE=%SCRIPT_DIR%SCHICK_PORT.EXE"
) else if exist "%SCRIPT_DIR%build\SCHICK_PORT.EXE" (
    set "PORT_EXE=%SCRIPT_DIR%build\SCHICK_PORT.EXE"
) else if exist "%SCRIPT_DIR%..\build\SCHICK_PORT.EXE" (
    set "PORT_EXE=%SCRIPT_DIR%..\build\SCHICK_PORT.EXE"
)

if "%PORT_EXE%"=="" (
    echo [HINWEIS] SCHICK_PORT.EXE noch nicht gebaut. Starte Build via bauen_port.py ...
    python "%SCRIPT_DIR%bauen_port.py"
    if exist "%SCRIPT_DIR%build\SCHICK_PORT.EXE" (
        set "PORT_EXE=%SCRIPT_DIR%build\SCHICK_PORT.EXE"
    ) else (
        echo [FEHLER] SCHICK_PORT.EXE konnte nicht gebaut werden.
        exit /b 1
    )
)

echo Starte Schicksalsklinge mit Spieldaten aus: "%SPIELPFAD%"
"%PORT_EXE%" "%SPIELPFAD%" %*
endlocal
