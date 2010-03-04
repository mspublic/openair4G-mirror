/*________________________W3g4free_vars.h________________________

 Authors : Hicham Anouar, Raymond Knopp
 Company : EURECOM
 Emails  : anouar@eurecom.fr,  knopp@eurecom.fr
________________________________________________________________*/


#ifndef __MAC_VARS_H__
#define __MAC_VARS_H__
#ifdef USER_MODE
//#include "stdio.h"
#endif //USER_MODE
#include "COMMON/openair_defs.h"
#include "defs.h"
#include "PHY_INTERFACE/defs.h"
#include "COMMON/mac_rrc_primitives.h"



PHY_RESOURCES CHBCH_PHY_RESOURCES[2];
PHY_RESOURCES DL_SCH_PHY_RESOURCES[2];
PHY_RESOURCES UL_SCH_PHY_RESOURCES[NB_UL_SCHED_MAX];

PHY_RESOURCES SACCH_PHY_RESOURCES;
MAC_MEAS_T MEAS_Trigger;

UE_MAC_INST *UE_mac_inst; //[NB_MODULE_MAX]; 
CH_MAC_INST *CH_mac_inst; //[NB_MODULE_MAX]; 
MAC_RLC_XFACE *Mac_rlc_xface;

#ifndef USER_MODE
RRC_XFACE *Rrc_xface;
MAC_xface *mac_xface;
#else
#include "PHY_INTERFACE/extern.h"
#include "RRC/MESH/extern.h"
#endif

//u8 CH_ID[NB_MODULES_MAX][NB_CNX];
 
//u8 Nb_inst;
u8 Is_rrc_registered;
char Mac_dummy_buffer[50]; 
//char crc[MAX];
char Sorted_index_table[MAX_NB_SCHED]; 

//unsigned char *Sched_rssi_meas_matrix[NB_CNX_CH+1][NB_CNX_CH+1][NUMBER_OF_FREQUENCY_GROUPS][NB_TIME_ALLOC];//NB_of_CH per machine
//unsigned int *Last_sched_frame[NUMBER_OF_FREQUENCY_GROUPS][NB_TIME_ALLOC];//NB_of_CH per machine
//MEAS_INFO Meas_info_matrix[NB_MODULES_MAX][NUMBER_OF_FREQUENCY_GROUPS][NB_TIME_ALLOC];


#ifndef PHY_EMUL
unsigned char NB_CH_INST;
unsigned char NB_UE_INST;
#endif
#endif


