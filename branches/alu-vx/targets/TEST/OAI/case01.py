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

# \file case01.py
# \brief test case 01 for OAI: compilations
# \author Navid Nikaein
# \date 2013 - 2014
# \version 0.1
# @ingroup _test

import log
import openair
import core

makerr1 = '***'
makerr2 = 'Error 1'

def execute(oai, user, pw, host, logfile,logdir,debug):
    
    case = '01'
    rv = 1
    oai.send('cd $OPENAIR_TARGETS;')   
 
    try:
        test = '00'
        name = 'Check oai.svn.add'
        conf = 'svn st -q | grep makefile'
        diag = 'Makefile(s) changed. If you are adding a new file, make sure that it is added to the svn'
        rsp = oai.send_recv('svn st -q | grep -i makefile;') 
        for item in rsp.split("\n"):
            if "Makefile" in item:
                rsp2=item.strip() + '\n'
        oai.find_false_re(rsp,'Makefile')
    except log.err, e:
        diag = diag + "\n" + rsp2  
               #log.skip(case, test, name, conf, e.value, logfile)
        log.skip(case, test, name, conf, '', diag, logfile)
    else:
        log.ok(case, test, name, conf, '', logfile)
    
    oai.send('cd SIMU/USER;')   
    oai.send('mkdir ' + logdir + ';')
    
    try:
        test = '01'
        name = 'Compile oai.rel8.make'
        conf = 'make'
        trace = logdir + '/log_' + case + test + '.txt;'
        tee = ' 2>&1 | tee ' + trace
        diag = "check the compilation errors for oai"
        oai.send('make cleanall;')
        oai.send('make cleanasn1;')
        oai.send('rm -f ./oaisim.rel8.'+host)
        oai.send_expect_false('make -j4 JF=1' + tee, makerr1,  1500)
        oai.send('cp ./oaisim ./oaisim.rel8.'+host)
    except log.err, e:
        log.fail(case, test, name, conf, e.value, diag, logfile,trace)
        rv = 0
    else:
        log.ok(case, test, name, conf, '', logfile)
        
    try:
        test = '02'
        name = 'Compile oai.rel8.nas.make'
        conf = 'make nasmesh_fix; make NAS=1'
        diag = 'check the compilation errors for oai and nas driver'
        oai.send('make cleanall;')
        oai.send('rm -f ./oaisim.rel8.nas'+host)
        oai.send('rm -f ./nasmesh;')
        oai.send('make nasmesh_clean;')
        trace = logdir + '/log_' + case + test + '_1.txt;'
        tee = ' 2>&1 | tee ' + trace
        oai.send_expect_false('make nasmesh_fix' + tee, makerr1,  60)
        trace = logdir + '/log_' + case + test + '_2.txt;'
        tee = ' 2>&1 | tee ' + trace
        oai.send_expect_false('make NAS=1 JF=1 -j4' + tee, makerr1,  1500)
        oai.send('cp ./oaisim ./oaisim.rel8.nas.'+host)
        
    except log.err, e:
        log.fail(case, test, name, conf, e.value, diag, logfile,trace)
        rv = 0
    else:
        log.ok(case, test, name, conf, '', logfile)
    
    oai.send('cd $OPENAIR_TARGETS;')     
    oai.send('cd RT/USER;')   

    try:
        test = '03'
        name = 'Compile oai.rel8.rf.make' 
        conf = 'make RTAI=0 EXMIMO=1 Rel8=1'
        trace = logdir + '/log_' + case + test + '.txt;'
        tee = ' 2>&1 | tee ' + trace
        diag = 'check the compilation errors for Rel8'
        oai.send('make cleanall;')
        oai.send('rm -f ./oaisim.rel8.rf.'+host)
        oai.send_expect_false('make RTAI=0 EXMIMO=1 -j4' + tee, makerr1,  1500)
        oai.send('cp ./oaisim ./oaisim.rel8.rf.'+host)
    except log.err, e:
        log.fail(case, test, name, conf, e.value, diag, logfile,trace)
    else:
        log.ok(case, test, name, conf, '', logfile)
        
    oai.send('cd $OPENAIR1_DIR;')     
    oai.send('cd SIMULATION/LTE_PHY;')   

    try:
        test = '04'
        name = 'Compile oai.rel8.phy.dlsim.make' 
        conf = 'make dlsim'
        trace = logdir + '/log_' + case + test + '.txt;'
        tee = ' 2>&1 | tee ' + trace
        diag = 'check the compilation errors for dlsim in $OPENAIR1_DIR/SIMULATION/LTE_PHY'
        oai.send('make clean;')
        oai.send('rm -f ./dlsim.rel8.'+host)
        oai.send_expect_false('make dlsim -j4' + tee, makerr1,  1500)
        oai.send('cp ./dlsim ./dlsim.rel8.'+host)
    except log.err, e:
        log.fail(case, test, name, conf, e.value, diag, logfile,trace)
        rv = 0
    else:
        log.ok(case, test, name, conf, '', logfile)

    try:
        test = '05'
        name = 'Compile oai.rel8.phy.ulsim.make' 
        conf = 'make ulsim'
        trace = logdir + '/log_' + case + test + '.txt;'
        tee = ' 2>&1 | tee ' + trace
        diag = 'check the compilation errors for ulsim in $OPENAIR1_DIR/SIMULATION/LTE_PHY'
        oai.send('make clean;')
        oai.send('rm -f ./ulsim.rel8.'+host)
        oai.send_expect_false('make ulsim -j4' + tee, makerr1,  1500)
        oai.send('cp ./ulsim ./ulsim.rel8.'+host)
    except log.err, e:
        log.fail(case, test, name, conf, e.value, diag, logfile,trace)
        rv = 0
    else:
        log.ok(case, test, name, conf, '', logfile)
    
    oai.send('cd $OPENAIR_TARGETS;')     
    oai.send('cd SIMU/USER;')   
    
    try:
        test = '06'
        name = 'Compile oai.rel8.itti.make' 
        conf = 'make DISABLE_XER_PRINT=1 ENABLE_ITTI=1 Rel8=1'
        trace = logdir + '/log_' + case + test + '.txt;'
        tee = ' 2>&1 | tee ' + trace
        diag = 'check the compilation errors for ITTI Rel8'
        oai.send('make clean;')
        oai.send('rm -f ./oaisim.rel8.itti.'+host)
        oai.send_expect_false('make DISABLE_XER_PRINT=1 ENABLE_ITTI=1 Rel8=1 -j4' + tee, makerr1,  1500)
        oai.send('cp ./oaisim ./oaisim.rel8.itti.'+host)
    except log.err, e:
        log.fail(case, test, name, conf, e.value, diag, logfile,trace)
        rv = 0
    else:
        log.ok(case, test, name, conf, '', logfile)

    try:
        test = '07'
        name = 'Compile oai.rel10.make' 
        conf = 'make RLC_STOP_ON_LOST_PDU=1 Rel10=1'
        trace = logdir + '/log_' + case + test + '.txt;'
        tee = ' 2>&1 | tee ' + trace
        diag = 'check the compilation errors for Rel10'
        oai.send('make clean;')
        oai.send('make cleanall;')
        oai.send('make cleanasn1;')
        oai.send('rm -f ./oaisim.rel10.'+host)
        oai.send_expect_false('make RLC_STOP_ON_LOST_PDU=1 Rel10=1 -j4' + tee, makerr1,  1500)
        oai.send('cp ./oaisim ./oaisim.rel10.'+host)
    except log.err, e:
        log.fail(case, test, name, conf, e.value, diag, logfile,trace)
        rv = 0
    else:
        log.ok(case, test, name, conf, '', logfile)

    try:
        test = '08'
        name = 'Compile oai.rel10.itti.make' 
        conf = 'make DISABLE_XER_PRINT=1 ENABLE_ITTI=1 RLC_STOP_ON_LOST_PDU=1 Rel10=1'
        trace = logdir + '/log_' + case + test + '.txt;'
        tee = ' 2>&1 | tee ' + trace
        diag = 'check the compilation errors for ITTI Rel10'
        oai.send('make cleanall;')
        oai.send('rm -f ./oaisim.rel10.itti.'+host)
        oai.send_expect_false('make DISABLE_XER_PRINT=1 ENABLE_ITTI=1 RLC_STOP_ON_LOST_PDU=1 Rel10=1 -j4' + tee, makerr1,  1500)
        oai.send('cp ./oaisim ./oaisim.rel10.itti.'+host)
    except log.err, e:
        log.fail(case, test, name, conf, e.value, diag, logfile,trace)
        rv = 0
    else:
        log.ok(case, test, name, conf, '', logfile)
    try:
        test = '13'
        name = 'Compile oai_nw_ether IP driver' 
        conf = 'make oai_nw_drv'
        trace = logdir + '/log_' + case + test + '.txt;'
        tee = ' 2>&1 | tee ' + trace
        diag = 'check the compilation errors for ITTI Rel8'
        oai.send('make clean;')
        oai.send('make cleanall;')
        oai.send('make cleanasn1;')
        oai.send('rm -f ./oai_nw_drv;')
        oai.send('make oai_nw_drv_clean;')
        tee = ' 2>&1 | tee ' + trace
        oai.send_expect_false('make oai_nw_drv' + tee, makerr1,  60)
    except log.err, e:
        log.fail(case, test, name, conf, e.value, diag, logfile,trace)
    else:
        log.ok(case, test, name, conf, '', logfile)
    try:
        test = '14'
        name = 'Compile oai.rel8.itti.ral.make' 
        conf = 'make DISABLE_XER_PRINT=1 NAS=1 OAI_NW_DRIVER_TYPE_ETHERNET=1 ENABLE_ITTI=1 USER_MODE=1 OPENAIR2=1 ENABLE_RAL=1 MIH_C_MEDIEVAL_EXTENSIONS=1 RLC_STOP_ON_LOST_PDU=1 Rel8=1'
        trace = logdir + '/log_' + case + test + '.txt;'
        tee = ' 2>&1 | tee ' + trace
        diag = 'check the compilation errors for ITTI Rel8'
        oai.send('make clean;')
        oai.send('make cleanall;')
        oai.send('make cleanasn1;')
        oai.send('rm -f ./oaisim.rel8.itti.ral.'+host)
        oai.send_expect_false('make DISABLE_XER_PRINT=1 NAS=1 OAI_NW_DRIVER_TYPE_ETHERNET=1 ENABLE_ITTI=1 USER_MODE=1 OPENAIR2=1 ENABLE_RAL=1 MIH_C_MEDIEVAL_EXTENSIONS=1 RLC_STOP_ON_LOST_PDU=1 Rel8=1 -j4' + tee, makerr1,  1500)
        oai.send('cp ./oaisim ./oaisim.rel8.itti.ral.'+host)
    except log.err, e:
        log.fail(case, test, name, conf, e.value, diag, logfile,trace)
    else:
        log.ok(case, test, name, conf, '', logfile)
    try:
        test = '15'
        name = 'Compile oai.rel10.itti.ral.make' 
        conf = 'make DISABLE_XER_PRINT=1 NAS=1 OAI_NW_DRIVER_TYPE_ETHERNET=1 ENABLE_ITTI=1 USER_MODE=1 OPENAIR2=1 ENABLE_RAL=1 MIH_C_MEDIEVAL_EXTENSIONS=1 RLC_STOP_ON_LOST_PDU=1 Rel10=1'
        trace = logdir + '/log_' + case + test + '.txt;'
        tee = ' 2>&1 | tee ' + trace
        diag = 'check the compilation errors for ITTI Rel10'
        oai.send('make clean;')
        oai.send('make cleanall;')
        oai.send('make cleanasn1;')
        oai.send('rm -f ./oaisim.rel10.itti.ral.'+host)
        oai.send_expect_false('make DISABLE_XER_PRINT=1 NAS=1 OAI_NW_DRIVER_TYPE_ETHERNET=1 ENABLE_ITTI=1 USER_MODE=1 OPENAIR2=1 ENABLE_RAL=1 MIH_C_MEDIEVAL_EXTENSIONS=1 RLC_STOP_ON_LOST_PDU=1 Rel10=1 -j4' + tee, makerr1,  1500)
        oai.send('cp ./oaisim ./oaisim.rel10.itti.ral.'+host)
    except log.err, e:
        log.fail(case, test, name, conf, e.value, diag, logfile,trace)
    else:
        log.ok(case, test, name, conf, '', logfile)

    return rv


