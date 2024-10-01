@echo off

set equity=
if "%1"=="" (
   set equity="" 
) else (  
  set equity= -e c%1
)

C:\home\joshxli\prod\strat\Patrev\sim\bin\msvc-14.0\release\Patrev-sim.exe ^
-c etc\prod-BBAT.cfg ^
-a Patrev3dSim ^
-p etc/pf-mlp-Patrev-debug.tsv ^
-f ../univ/live/ref-mlp-daily-prod.tsv ^
--strat.riskFactor=0.0045 ^
--strat.maxPosAge 3 ^
--strat.nearPeriod 1 ^
--strat.medPeriod 3 ^
--strat.farPeriod 3 ^
--strat.tickMult=-2 ^
--strat.numOfUnits 1 ^
--strat.tpfMult 1.2 ^
--debug 1 ^
%equity%


