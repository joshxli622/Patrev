@echo off

set equity=
if "%1"=="" (
   set equity="" 
) else (  
  set equity= -e c%1
)

for %%A in (0.4 0.6 0.8 1 1.2 1.4 1.6 1.8 2) do C:\home\joshxli\prod\strat\Patrev\sim\bin\msvc-14.0\release\Patrev-sim.exe -m %%A -c etc\sim-BBAT.cfg -a Patrev3dSim_tpfMultWith250dAtr --strat.riskFactor=0.0045 --strat.maxPosAge 3 --strat.nearPeriod 1 --strat.numOfUnits 1 --strat.tpfMult %%A --debug 0 -e c10000000

