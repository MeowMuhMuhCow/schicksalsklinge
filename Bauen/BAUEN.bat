@echo off
rem BAUEN.bat - baut BLADEM.EXE aus dem rekonstruierten Quelltext (Projekt Schicksalsklinge)
rem Sucht Python und startet bauen.py. Ergebnis: ..\Build\BLADEM.EXE und ..\Build\bau.log
rem   BAUEN.bat wichtig   zusaetzlich ..\Build\BLADEM_FIX_WICHTIG.EXE (Fehlerbehebungen kritisch + mittel)
rem   BAUEN.bat alles     zusaetzlich ..\Build\BLADEM_FIX_ALLES.EXE   (alle Fehlerbehebungen)
rem   (alle Behebungen sind im Spiel ungetestet, siehe FIXES.md)
setlocal
cd /d "%~dp0"
title Schicksalsklinge - Bau von BLADEM.EXE %1

rem ---- Python: python\python.exe neben dieser Datei, sonst py -3, sonst python
set PY=
if exist "%~dp0python\python.exe" set PY="%~dp0python\python.exe"
if defined PY goto py_ok
py -3 -c "import sys; sys.exit(0 if sys.version_info >= (3, 8) else 1)" >nul 2>&1
if not errorlevel 1 set PY=py -3
if defined PY goto py_ok
python -c "import sys; sys.exit(0 if sys.version_info >= (3, 8) else 1)" >nul 2>&1
if not errorlevel 1 set PY=python
if defined PY goto py_ok
echo.
echo Python 3 wurde nicht gefunden.
echo Bitte Python fuer Windows von https://www.python.org/downloads/ installieren
echo (beim Installieren "Add python.exe to PATH" ankreuzen) und BAUEN.bat erneut starten.
echo.
pause
exit /b 1
:py_ok

echo Python: %PY%
echo.
%PY% -X utf8 "%~dp0bauen.py" %*
set RC=%errorlevel%
echo.
if "%RC%"=="0" echo Fertig. Ergebnis: ..\Build\BLADEM.EXE
if "%RC%"=="0" if /i "%~1"=="wichtig" echo Fix-EXE: ..\Build\BLADEM_FIX_WICHTIG.EXE - starten mit BLADEM_FIX_WICHTIG_STARTEN.bat
if "%RC%"=="0" if /i "%~1"=="alles" echo Fix-EXE: ..\Build\BLADEM_FIX_ALLES.EXE - starten mit BLADEM_FIX_ALLES_STARTEN.bat
if not "%RC%"=="0" echo Nicht bytegleich oder abgebrochen - Protokoll: ..\Build\bau.log
echo.
pause
exit /b %RC%
