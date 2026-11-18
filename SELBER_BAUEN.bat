@echo off
REM Baut alle DOS-Bugfix-EXEs (Patch\patchen.ps1), alle nativen Windows-EXEs (SCHICK_PORT*.EXE)
REM und den Starter (NORDLANDTRILOGIE.EXE) direkt aus dem Quelltext - komplett OHNE Python!
cd /d "%~dp0"
if exist "Nativ\selber_bauen.ps1" (
    powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Nativ\selber_bauen.ps1" %*
) else if exist "Port\release\selber_bauen.ps1" (
    powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Port\release\selber_bauen.ps1" %*
) else (
    echo FEHLER: Nativ\selber_bauen.ps1 nicht gefunden.
    pause
)
