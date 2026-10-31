@echo off
rem ============================================================
rem  BLADEM_FIX_WICHTIG_MIT_PROBENLOG_STARTEN.bat - wie BLADEM_FIX_WICHTIG_STARTEN.bat,
rem  dazu das Probenlog: jede Probe (Eigenschaft, Talent, Zauber, Attacke,
rem  Parade) erscheint live in einem zweiten Fenster und wird in
rem  Build\Testspielstaende_Fix_Wichtig\PROBEN.CSV mitgeschrieben.
rem  Hinterher ansehen: Probenlog\Probenlog_Schicksalsklinge.html
rem ============================================================
set "PROBENLOG=1"
call "%~dp0BLADEM_FIX_WICHTIG_STARTEN.bat"
