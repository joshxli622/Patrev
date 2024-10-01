@echo off

set equity=
if "%1"=="" (
   set equity="" 
 ) else (  
  set equity= -e c%1
 )

REM target 10 yr vol 10% ==> riskFactor: 0.0026

%~dp0\sim\bin\msvc-14.0\release\Patrev-sim.exe ^
-c etc\prod-BBAT.cfg ^
-a Patrev3d ^
-p ../univ/notc/pf-mlp-Patrev-notc.tsv ^
-f ../univ/notc/ref-mlp-daily-ct2.tsv ^
-r C:/home/joshxli/prod/mlp/report/bb-notc ^
--strat.riskFactor=0.0026 ^
--strat.maxPosAge 3 ^
--strat.nearPeriod 1 ^
--strat.medPeriod 3 ^
--strat.farPeriod 3 ^
--strat.ind1LBPct 0 ^
--strat.ind1UBPct 20 ^
--strat.tickMult=-2 ^
--strat.numOfUnits 1 ^
--strat.tpfMult 1.2 ^
--strat.alpha 1.1 ^
--strat.beta 1.1 ^
--strat.refAtr 250 ^
--debug 0 ^
%equity%


