#!/bin/bash
MR1_IN_ADDR=10.0.1.3
MR1_IN6_ADDR=2001:10:0:1:7856:3412:0:2
MR1_EG_ADDR=10.0.2.3
MR1_EG6_ADDR=2001:660:5502::10
# Flow CH1->MR1
MR1_LABEL_IN=1001
# Flow MR1->CH1
MR1_LABEL_OUT=1000

MR2_IN_ADDR=10.0.1.4
MR2_IN6_ADDR1=2001:10:0:1:7856:3412:0:3
MR2_IN6_ADDR2=2001:10:0:2:7856:3412:0:3
MR2_EG6_ADDR=2001:660:5502::30
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
MR3_EG6_ADDR=2001:660:5502::15
# Flow CH2->MR3
MR3_LABEL_IN=4000
# Flow MR3->CH2
MR3_LABEL_OUT=4001

CH1_IN_ADDR=10.0.1.1
CH2_IN_ADDR=10.0.1.2

CH1_IN6_ADDR=2001:10:0:1:7856:3412:0:1
CH2_IN6_ADDR=2001:10:0:2:7856:3412:0:1

MN1_IN6_ADDR=2001:660:5502::20
MN2_IN6_ADDR=2001:660:5502::25

# Put the right OPENAIR3 path here
OPENAIR3_HOME=~/openair3
OPENAIR3_PMIP6D_PATH=$OPENAIR3_HOME/pmip6d
OPENAIR3_SCRIPTS_PATH=$OPENAIR3_HOME/scripts

# Put the right OPENAIR2 path here
OPENAIR2_DIR=~/openair2_r220/