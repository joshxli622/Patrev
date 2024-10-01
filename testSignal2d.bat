@echo on

set equity=
if "%1"=="" (
   set equity="" 
) else (  
  set equity= -e c%1
)

C:\home\joshxli\prod\strat\Patrev\sim\bin\msvc-14.0\release\Patrev-sim.exe -c etc\sim-research.cfg -a Patrev2dTest --strat.riskFactor=0.00369 --strat.maxPosAge 2 --strat.farPeriod 3 --strat.nearPeriod 1 --strat.ind1UBPct 20
