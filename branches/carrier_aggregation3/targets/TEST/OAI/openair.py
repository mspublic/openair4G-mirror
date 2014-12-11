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
import pxssh
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
        core.__init__(self)
              
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

    def connect(self, username, password, prompt='PEXPECT_OAI'):
        self.prompt1 = prompt
        self.prompt2 = prompt

        try:
            if  not username:
                username = root 
            if  not password:
                password = username 
            self.oai = pxssh.pxssh()
            self.oai.login(self.address,username,password)
            self.oai.sendline('PS1='+self.prompt1)
            self.oai.PROMPT='PEXPECT_OAI'
            # need to look for twice the string of the prompt
            self.oai.prompt()
            self.oai.prompt()
            self.oai.sendline('uptime')
            self.oai.prompt()
            print self.oai.before
                              
        except Error, val :
                print "Error: can't connect to"+username+"@"+self.address
                
                    
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
                oai.send_nowait('echo '+pw+ ' | sudo -S rmmod nasmesh;')
                #os.system('echo '+pw+ ' | sudo -S rmmod nasmesh;')
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
                oai.send('echo '+pw+ ' | sudo -S insmod ./nasmesh.ko;')
                
        except Error, val:
            print "Error inserting oai network driver module:", val
    
    def cleandir (self, logdir,debug) :
        
        for filename in os.listdir(logdir):
            filepath = os.path.join(logdir, filename)
            if debug == 2 :
                print 'logdir is ' + logdir
                print 'filepath is ' + filepath 
            try:
                shutil.rmtree(filepath)
            except OSError:
                os.remove(filepath)
                #print 'Could not remove the filepath'+ filepath + ' with error ' + OSError
    
    def create_dir(self,dirname,debug) :
        if not os.path.exists(dirname) :
            try:
                os.makedirs(dirname,0755)
            except OSError:
                # There was an error on creation, so make sure we know about it
                raise            
