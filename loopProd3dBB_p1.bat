@echo off

set equity=
if "%1"=="" (
   set equity="" 
) else (  
  set equity= -e c%1
)

REM target 10 yr vol 10% ==> riskFactor: 0.0026

for %%A in (0.5 0.6 0.7 0.8 0.9 1.0 1.1 1.2 1.3 1.4 1.5) do (
for %%B in (0.5 0.6 0.7 0.8 0.9 1.0 1.1 1.2 1.3 1.4 1.5) do (
C:\home\joshxli\prod\strat\Patrev\sim\bin\msvc-14.0\release\Patrev-sim.exe ^
-c etc\prod-BBAT.cfg ^
-a Patrev3d_loop_alpha_beta ^
-p ../univ/live/pf-mlp-Patrev-prod.tsv ^
-f ../univ/live/ref-mlp-daily-ct2.tsv ^
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
--strat.alpha %%A ^
--strat.beta %%B ^
--strat.refAtr 250 ^
--debug 0 ^
%equity%
))

