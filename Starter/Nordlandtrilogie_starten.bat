@echo off
setlocal
set "SCRIPT_DIR=%~dp0"
set "STARTER_EXE=%SCRIPT_DIR%build\NORDLANDTRILOGIE.EXE"

if not exist "%STARTER_EXE%" (
    echo [HINWEIS] NORDLANDTRILOGIE.EXE noch nicht gebaut. Starte bauen_starter.py ...
    python "%SCRIPT_DIR%bauen_starter.py"
)

if not exist "%STARTER_EXE%" (
    echo [FEHLER] %STARTER_EXE% konnte nicht gefunden oder gebaut werden.
    exit /b 1
)

start "" "%STARTER_EXE%" %*
endlocal
