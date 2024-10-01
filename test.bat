@echo on

for %%A in (5 4 3 2 1) do for %%B in (5 4 3 2 1) do for %%C in (100 5 4 3 2 1) do C:\home\joshxli\prod\strat\Patrev\sim\bin\msvc-14.0\release\Patrev-sim.exe -c etc\sim-SU.cfg -a Patrev3dTest --strat.riskFactor=0.00369 --strat.maxPosAge %%C --strat.farPeriod %%A --strat.nearPeriod %%B --strat.ind1UBPct %1
