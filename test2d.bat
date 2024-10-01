@echo off

set equity=
if "%1"=="" (
   set equity="" 
) else (  
  set equity= -e c%1
)

C:\home\joshxli\prod\strat\Patrev\sim\bin\msvc-14.0\release\Patrev-sim.exe -c etc\sim-SU.cfg -a Patrev2dTest --strat.riskFactor=0.004589 --strat.maxPosAge 2  --debug 0 %equity%
REM C:\home\joshxli\prod\strat\Patrev\sim\bin\msvc-14.0\debug\Patrev-sim_d.exe -c etc\sim-SU.cfg -a Patrev2dTest --strat.riskFactor=0.004589 --strat.maxPosAge 2 %equity%