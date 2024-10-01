import os
import pandas as pd

bdates = pd.bdate_range('20150101', '20200331')
#os.system('time 20:01:01')
for dt in bdates:
    os.system('time 20:01:01')
    print 'run bat file on %s'%dt
    os.system(r'date %02d-%02d-%02d'%(dt.year%100, dt.month, dt.day))
    os.system(r'C:\\home\\joshxli\\prod\\qbot\\strat\\Patrev\\genProd3dBB_ho.bat %04d%02d%02d'
              %(dt.year, dt.month, dt.day))
