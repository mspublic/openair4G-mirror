#!/bin/bash

MR1_IN_ADDR=10.0.1.3
MR1_IN6_ADDR=2001:10:0:1:7856:3412:0:2
MR1_EG_ADDR=192.168.9.1
MR1_EG6_ADDR=2001:660:5502::100
# Flow CH1->MR1
MR1_LABEL_IN=1001
# Flow MR1->CH1
MR1_LABEL_OUT=1000
ETH_MR1=eth0

MR2_IN_ADDR=10.0.1.4
MR2_IN6_ADDR1=2001:10:0:1:7856:3412:0:3
MR2_IN6_ADDR2=2001:10:0:2:7856:3412:0:3
MR2_EG6_ADDR=2001:660:5502::300
ETH_MR2=eth0
# Flow CH1->MR2
MR2_CH1_LABEL_IN=2000
# Flow MR2->CH1
MR2_CH1_LABEL_OUT=2001
# Flow CH2->MR2
MR2_CH2_LABEL_IN=3001
# Flow MR2->CH2
MR2_CH2_LABEL_OUT=3000

MR3_IN_ADDR=10.0.1.5
MR3_IN6_ADDR=2001:10:0:2:7856:3412:0:2
MR3_EG_ADDR=192.168.10.1
MR3_EG6_ADDR=2001:660:5502::200
# Flow CH2->MR3
MR3_LABEL_IN=4000
# Flow MR3->CH2
MR3_LABEL_OUT=4001
ETH_MR3=eth2

CH1_IN_ADDR=10.0.1.1
CH2_IN_ADDR=10.0.1.2

CH1_IN6_ADDR=2001:10:0:1:7856:3412:0:1
CH2_IN6_ADDR=2001:10:0:2:7856:3412:0:1

## Communication between the CHs
CH1_MR2_CH2_LABEL_IN=8001
CH1_MR2_CH2_LABEL_OUT=8000
CH2_MR2_CH1_LABEL_IN=9001
CH2_MR2_CH1_LABEL_OUT=9000

## Com between MN1 and MN3
MN1_MR1_CH1_MN3=1500
MN3_CH1_MR1_MN1=1501
MN1_CH1_MR2_MN3=2500
MN3_MR2_CH1_MN1=2501

## Com between MN2 and MN3
MN3_MR2_CH2_MN2=3500
MN2_CH2_MR2_MN3=3501
MN3_CH2_MR3_MN2=4500
MN2_MR3_CH2_MN3=4501

#MN1_IN6_ADDR=2001:660:5502::20
#MN2_IN6_ADDR=2001:660:5502::25
MN1_IN6_ADDR=2001:660:5502::110
MN1_IN_ADDR=192.168.9.2

MN2_IN6_ADDR=2001:660:5502::210
MN3_IN6_ADDR=2001:660:5502::310
MN2_IN_ADDR=192.168.10.2

# Put the right OPENAIR3 path here
OPENAIR3_HOME=/root/openair_trials_2/openair3
OPENAIR3_PMIP6D_PATH=$OPENAIR3_HOME/pmip6d
OPENAIR3_SCRIPTS_PATH=$OPENAIR3_HOME/scripts
REFLECTOR_DIR=$OPENAIR3_HOME/EXAMPLES/BARCA_TRIALS/REFLECTOR

# Put the right OPENAIR2 path here
#OPENAIR2_DIR=~/openair2_r234/openair2/
OPENAIR2_DIR=/root/openair_trials_2/openair2


