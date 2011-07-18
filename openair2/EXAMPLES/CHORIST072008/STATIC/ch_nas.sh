#!/bin/bash
#Phil, 25 july 08

source params.sh
./ch_conf_nas.sh 
./ch_mpls_nas.sh $MR1_LABEL_OUT $MR2_IN_ADDR $MR2_LABEL_IN
./ch_mpls_nas.sh $MR2_LABEL_OUT $MR1_IN_ADDR $MR1_LABEL_IN
#echo /openair3/pmip6d/pmip6d -c -L $CH_ADDR
#/openair3/pmip6d/pmip6d -c -L $CH_ADDR
#./ch_del_mpls.sh
