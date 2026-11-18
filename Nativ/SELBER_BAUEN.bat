@echo off
REM Baut alle nativen Windows-EXEs (SCHICK_PORT*.EXE) und den Starter (NORDLANDTRILOGIE.EXE)
REM direkt aus dem C/C++-Quelltext - komplett OHNE Python!
cd /d "%~dp0"
if exist "selber_bauen.ps1" (
    powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0selber_bauen.ps1" %*
) else if exist "Nativ\selber_bauen.ps1" (
    powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Nativ\selber_bauen.ps1" %*
) else (
    echo FEHLER: selber_bauen.ps1 nicht gefunden.
    pause
)
