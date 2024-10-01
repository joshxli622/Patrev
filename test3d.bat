@echo off

for %%A in (200000 250000 300000 350000 400000 450000 500000 600000 700000 800000 900000 1000000) do C:\home\joshxli\prod\strat\Patrev\sim\bin\msvc-14.0\release\Patrev-sim.exe -c etc\sim-SU.cfg -a Patrev3dTest --strat.riskFactor=0.004449 --strat.maxPosAge 3 --strat.nearPeriod 1 --strat.tpfMult 1.2 --debug 0 -m %%A -e c%%A
