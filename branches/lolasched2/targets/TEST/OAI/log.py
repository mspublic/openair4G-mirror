#******************************************************************************

#  Eurecom OpenAirInterface
#  Copyright(c) 1999 - 2013 Eurecom

#  This program is free software; you can redistribute it and/or modify it
#  under the terms and conditions of the GNU General Public License,
#  version 2, as published by the Free Software Foundation.

#  This program is distributed in the hope it will be useful, but WITHOUT
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
#  more details.

#  You should have received a copy of the GNU General Public License along with
#  this program; if not, write to the Free Software Foundation, Inc.,
#  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

#  The full GNU General Public License is included in this distribution in
#  the file called "COPYING".

#  Contact Information
#  Openair Admin: openair_admin@eurecom.fr
#  Openair Tech : openair_tech@eurecom.fr
#  Forums       : http://forums.eurecom.fsr/openairinterface
#  Address      : Eurecom, Compus SophiaTech 450, route des chappes, 06451 Biot, France

#*****************************************************************************

# \file log.py
# \brief provides primitives and defines how the logs and statistics are generated
# \author Navid Nikaein
# \date 2013
# \version 0.1
# @ingroup _test

import sys
import re
import time
import datetime
import array


debug = False
docfile = ''
start_time = time.time()
debug = 0
stats = {'passed':0, 'failed':0, 'skipped':0, 'internal_errors':0, 'cmd':0}

class bcolors:
    header = '\033[95m'
    okblue = '\033[94m'
    okgreen = '\033[92m'
    warning = '\033[93m'
    fail = '\033[91m'
    normal = '\033[0m'
    
    def disable(self):
        self.header = ''
        self.okblue = ''
        self.okgreen = ''
        self.warning = ''
        self.fail = ''
        self.normal = ''

class err(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

def writefile(logfile, message):   
    F_testlog = open(logfile, 'a')
    F_testlog.write(message + '\n')
    F_testlog.close()


def sleep(seconds):
        time.sleep(seconds)

def set_debug_level(level):
    debug = level

def statistics(logfile):
    global start_time
    
    #if stats['passed'] == 0:
     #   print "no test executed...exiting"
      #  sys.exit()
        
    total_tests = stats['passed'] + stats['failed'] + stats['skipped']
    total_ex_tests = stats['passed'] + stats['failed']
    elapsed_time = time.gmtime(time.time() - start_time)
    print '\n'
    log_record('info', '===============================================')
    log_record('info', 'Total tests performed                ' + repr(total_tests))
    log_record('info', 'Tests passed                         ' + repr(stats['passed']))
    log_record('info', 'Tests failed                         ' + repr(stats['failed']))
    log_record('info', 'Tests skipped                        ' + repr(stats['skipped']))
    log_record('info', '')
    log_record('info', 'Total commands sent                  ' + repr(stats['cmd']))
    log_record('info', 'Total elapsed time (h:m:s)           ' + time.strftime('%H:%M:%S', elapsed_time))
    log_record('info', '===============================================')
    log_record('info', 'Testing pass rate                    ' + repr((stats['passed'] * 100) / total_tests) + '%')
    log_record('info', '===============================================')
    
    writefile(logfile, '\n=====================Results===================')
    writefile(logfile, 'Total tests performed                ' + repr(total_tests))
    writefile(logfile, 'Tests passed                         ' + repr(stats['passed']))
    writefile(logfile, 'Tests failed                         ' + repr(stats['failed']))
    writefile(logfile, 'Tests skipped                        ' + repr(stats['skipped']))
    writefile(logfile, '')
    writefile(logfile, 'Total commands sent                  ' + repr(stats['cmd']))
    writefile(logfile, 'Total elapsed time (h:m:s)           ' + time.strftime('%H:%M:%S', elapsed_time))
    writefile(logfile, '===============================================')
    writefile(logfile, 'Testing pass rate                    ' + repr((stats['passed'] * 100) / total_tests) + '%')
    writefile(logfile, '===============================================\n')
    
def log_record(level, message):
    ts = time.strftime('%d %b %Y %H:%M')
    message = ts + ' [' + level + '] ' + message
    if level == 'passed' : 
        print bcolors.okgreen + message + bcolors.normal
    elif   level == 'failed' :   
        print bcolors.fail + message  + bcolors.normal
    elif   level == 'skipped' :   
        print bcolors.warning + message  + bcolors.normal
    else : 
        print message

def fail(case, testnum, testname, conf,  message, diag, output):
#    report(case, testnum, testname, conf, 'failed', output, diag, message)
    report(case, testnum, testname, conf, 'failed', output, diag)
    log_record('failed', case + testnum + ' : ' + testname  + ' ('+ conf+')')
    if message :
        log_record('failed', "Output follows:\n" + message )  
    stats['failed'] += 1

def failquiet(case, testnum, testname, conf):
    log_record('failed', case + testnum + ' :' + testname + ' ('+ conf+')')
    stats['failed'] += 1
    
def ok(case, testnum, testname, conf, message, output):
    report(case, testnum, testname, conf, 'passed', output)
    log_record('passed', case + testnum + ' : ' + testname + ' ('+ conf+')')
    if message :
        log_record('passed', "Output follows:\n" + message )
    stats['passed'] += 1
    
        
def skip(case, testnum, testname, conf, message=None, diag=None, output=None):
    log_record('skipped', case + testnum + ' :' + testname + ' ('+ conf+')')
    report(case, testnum, testname, conf, 'skipped', output, diag)
    if message :
        log_record('skipped', "Output follows:\n" + message )
    if diag : 
        log_record('skipped', "Diagnostic: \n" + diag )
    stats['skipped'] += 1

    
def report(case, test, name, conf, status, output, diag=None, desc=None):
    writefile (output, '[' +status+ '] ' + case + test + ' : ' + name + ' ('+ conf+')')
    if diag : 
        writefile (output, '-------> ' + diag)
    if desc:
        writefile(output, desc)
    #log_record('report', + case + test + ' documented')
  
    
