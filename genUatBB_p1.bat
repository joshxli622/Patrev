@echo off

set equity=
if "%1"=="" (
   set equity="" 
 ) else (  
  set equity= -e c%1
 )

REM target 10 yr vol 10% ==> riskFactor: 0.0026

%~dp0\sim\bin\msvc-14.0\release\Patrev-sim.exe ^
-c etc\uat-BBAT.cfg ^
-a Patrev ^
-t 20000101, ^
-p etc/pf-mlp-Patrev.tsv ^
-f ../univ/live/ref-mlp-daily-prod.tsv ^
--strat.riskFactor=0.0016 ^
--strat.maxPosAge 300 ^
--strat.nearPeriod 1 ^
--strat.medPeriod 1 ^
--strat.farPeriod 7 ^
--strat.ind1LBPct 0 ^
--strat.ind1UBPct 100 ^
--strat.ind2LBPct 99 ^
--strat.ind2UBPct 100 ^
--strat.ind3LBPct 99 ^
--strat.ind3UBPct 100 ^
--strat.extrDistance 1 ^
--strat.tickMult 1 ^
--strat.numOfUnits 1 ^
--strat.tpfMult 1.4 ^
--strat.alpha 1.1 ^
--strat.beta 1.1 ^
--strat.refAtr 250 ^
--debug 0 ^
%equity%


