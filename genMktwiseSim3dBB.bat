@echo off

REM  stratName default
set a=R2NEX3dSim
REM equity default
set e=c10000000
REM portfolioFile default
set p=etc\pf-rintra.tsv
REM report root
set r=
REM debug
set d=0
REM trade ricci
set t=20000101,21160617

:initial
if "%1"=="" goto done
echo %1
set aux=%1
if "%aux:~0,1%"=="-" (
   set nome=%aux:~1,250%
) else (
   set "%nome%=%1"
   set nome=
)
shift
goto initial
:done

C:\home\joshxli\prod\strat\Patrev\sim\bin\msvc-14.0\release\Patrev-sim.exe ^
-c etc\prod-BBAT.cfg ^
-a %a% ^
-p %p% ^
-r %r% ^
-e %e% ^
-f ../univ/live/ref-mlp-daily-ct2.tsv ^
--strat.riskFactor=0.0045 ^
--strat.maxPosAge 3 ^
--strat.nearPeriod 1 ^
--strat.medPeriod 3 ^
--strat.farPeriod 3 ^
--strat.tickMult=-2 ^
--strat.numOfUnits 1 ^
--strat.tpfMult 1.2 ^
--debug 0


