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

# \file openair.py
# \brief class that define the oaisim class and its attributes
# \author Navid Nikaein
# \date 2013
# \version 0.1
# @ingroup _test

import pexpect
import time
import os
import array
import shutil
from subprocess import call

from core import *

SHELL = '/bin/bash'

class openair(core):
    def __init__(self, hostname, address):
        self.error = '% '
        self.hostname = hostname
        self.address = address
        self.localhost = None
        self.shell_prompt = '$'
        core.__init__(self)

    def get_shell(self):
        print 'get the bash \n'
        self.prompt1 = self.shell_prompt
        self.prompt2 = prompt
        try:
            self.sh = pexpect.spawn(SHELL)
            index = self.sh.expect([self.prompt1, pexpect.TIMEOUT], timeout=10)
            if index != 0:
                print 'unable to spawn shell'   
            
        except Exception, val:
            print "Error:", val
              
    @property        
    def localhost(self):
        if self.localhost :
            return self.localhost 
        elif self.hostname in ['localhost', '127.0.0.7', '::1'] :
            self.localhost = self.hostname
        return self.localhost

    @localhost.setter
    def localhost(self,localhost):
        self.localhost = localhost

    def shcmd(self,cmd,sudo=False):
        
        if sudo:
            cmd = "sudo %s" % command
        
        proc = subprocess.Popen(command, shell=True, 
                             stdout = subprocess.PIPE, 
                             stderr = subprocess.PIPE)
                       
        stdout, stderr = proc.communicate()
        return (stdout, stderr)

    def connect(self, username, password, prompt):
        self.prompt1 = self.shell_prompt
        if not prompt :
            self.prompt2 = self.prompt1
        else :
            self.prompt2 = prompt 
        while 1:
            try:
                if  not username:
                    username = root 
                if  not password:
                    password = username 
                    
                self.oai = pexpect.spawn('ssh -o "UserKnownHostsFile=/dev/null" -o "StrictHostKeyChecking=no" -o "ConnectionAttempts=1" ' \
                                             + username + '@' + self.address)
                
                index = self.oai.expect([re.escape(self.prompt1), re.escape(self.prompt2), pexpect.TIMEOUT], timeout=40)
                if index == 0 :
                    return 'Ok'
                else :
                    index = self.oai.expect(['password:', pexpect.TIMEOUT], timeout=40)
                    if index == 0 : 
                        self.oai.sendline(password)
                        index = self.oai.expect([re.escape(self.prompt1), re.escape(self.prompt2), pexpect.TIMEOUT], timeout=10)
                        if index != 0:
                            print 'ERROR! could not login with SSH.'
                            print 'Expected ' + self.prompt1 + ', received >>>>' + self.oai.before + '<<<<'
                            sys.exit(1) 
                    return 'Ok'
                        
            except Exception, val:
                time.sleep(5)
                print "Error:", val
                
                    
    def disconnect(self):
        print 'disconnecting the ssh connection to ' + self.address + '\n'
        self.oai.send('exit')
#        self.cancel()

    def kill(self, user, pw):
        try:
            if user == 'root' :
                os.system('pkill oaisim')
                os.system('pkill cc1') 
                time.sleep(1)
                os.system('pkill oaisim')
            else :
                os.system('echo '+pw+' | sudo -S pkill oaisim')
                os.system('echo '+pw+' | sudo -S pkill cc1') 
                time.sleep(1)
                os.system('echo '+pw+' | sudo -S pkill oaisim')
        except Error, val:
            print "Error:", val
            
    def rm_driver(self,oai,user, pw):
        try:
            if user == 'root' : 
                #oai.send_nowait('rmmod nasmesh;')
                os.system('rmmod nasmesh;')
            else :
                #oai.send_nowait('echo '+pw+ ' | sudo -S rmmod nasmesh;')
                os.system('echo '+pw+ ' | sudo -S rmmod nasmesh;')
        except Error, val:
            print "Error removing oai network driver module:", val
   
    def driver(self,oai,user,pw):
        pwd = oai.send_recv('pwd') 
        oai.send('cd $OPENAIR_TARGETS;')   
        oai.send('cd SIMU/USER;')   
        try:
            if user == 'root' : 
                oai.send_nowait('insmod ./nasmesh.ko;')
            else :
                oai.send_nowait('echo '+pw+ ' | sudo -S insmod ./nasmesh.ko;')
                
            oai.send_nowait('cd '+ pwd)  
        except Error, val:
            print "Error inserting oai network driver module:", val
