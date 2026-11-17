@echo off
rem ===========================================================================
rem  Schicksalsklinge_starten_ALLE_FIXES_MIT_PROBENLOG.bat - wie
rem  Schicksalsklinge_starten_ALLE_FIXES.bat, dazu das Probenlog:
rem  jede Probe (Eigenschaft, Talent, Zauber, Attacke, Parade, Stufenanstieg,
rem  Ereignis) erscheint live in einem zweiten Fenster und wird in PROBEN.CSV
rem  im Spielordner (bzw. bei Schreibschutz in Spielstaende\PROBEN.CSV)
rem  mitgeschrieben. Alte Logs wandern mit Datum nach Probenlogs\.
rem  Hinterher ansehen: Probenlog\Probenlog_Schicksalsklinge.html
rem ===========================================================================
setlocal
set "PROBENLOG=1"
set "PORT_EXE_NAME=SCHICK_PORT.EXE"
if exist "%~dp0Schicksalsklinge_starten_ALLE_FIXES.bat" (
    call "%~dp0Schicksalsklinge_starten_ALLE_FIXES.bat" %*
) else (
    call "%~dp0start_schicksalsklinge.bat" %*
)
endlocal
