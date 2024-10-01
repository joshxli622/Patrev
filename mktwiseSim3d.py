#!/usr/bin/env python

import subprocess
import sys
import time
import pprint
import os.path 

from os import listdir
from datetime import date, datetime, timedelta
from optparse import OptionParser

parser = OptionParser()
parser.add_option('-c', '--config', dest = 'config', default = 'tradetool/etc/tradetool.cfg')
parser.add_option('-d', '--stratRoot', dest = 'stratRoot', 
                  default = r'C:\home\joshxli\prod\strat')

(opts, args) = parser.parse_args()

pp = pprint.PrettyPrinter(indent=2)

pfRoot = "etc/mktwise"
pfFiles = [f for f in listdir("etc/mktwise")
           if os.path.isfile(os.path.join(pfRoot, f))]

for pfFile in pfFiles:
    (pfFileRoot, pfFileExt) = os.path.splitext(pfFile)
    mktID = pfFileRoot.split('_')[1]
    pfFile = os.path.join(pfRoot, pfFile)
    batFile = 'genSim3d.bat -p %s -a %s' %(pfFile, "Patrev3d_"+mktID)
    print 'batFile=' + batFile

    try:
        retcode = subprocess.call(batFile, shell=True)
        if retcode < 0:
            print >>sys.stderr, "Child was terminated by signal (mktID %s)" %mktID, -retcode
        else:
            print >>sys.stderr, "Child returned (mktID %s)" %mktID, retcode
    except OSError as e:
        print >>sys.stderr, "Execution failed (mktID %s):" %mktID, e



