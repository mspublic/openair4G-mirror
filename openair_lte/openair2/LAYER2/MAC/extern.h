/*________________________W3g4free_extern.h________________________

 Authors : Hicham Anouar, Raymond Knopp
 Company : EURECOM
 Emails  : anouar@eurecom.fr,  knopp@eurecom.fr
________________________________________________________________*/


#ifndef __MAC_EXTERN_H__
#define __MAC_EXTERN_H__


#ifdef USER_MODE
//#include "stdio.h"
#endif //USER_MODE
#include "COMMON/openair_defs.h"
#include "defs.h"
#include "COMMON/mac_rrc_primitives.h"
#ifdef PHY_EMUL
#include "SIMULATION/simulation_defs.h"
#endif //PHY_EMUL
#include "PHY_INTERFACE/defs.h"


extern PHY_RESOURCES CHBCH_PHY_RESOURCES[2];
extern PHY_RESOURCES DL_SCH_PHY_RESOURCES[2];
extern PHY_RESOURCES UL_SCH_PHY_RESOURCES[NB_UL_SCHED_MAX];
extern PHY_RESOURCES SACCH_PHY_RESOURCES;
extern MAC_MEAS_T MEAS_Trigger;

extern UE_MAC_INST *UE_mac_inst;
extern CH_MAC_INST *CH_mac_inst;
extern MAC_RLC_XFACE *Mac_rlc_xface;
extern u8 Is_rrc_registered;

#ifndef USER_MODE
extern MAC_xface *mac_xface;
extern RRC_XFACE *Rrc_xface;
#else
#include "PHY_INTERFACE/extern.h"
#ifndef CELLULAR
#include "RRC/MESH/extern.h"
#else
#ifdef NODE_RG
#include "RRC/CELLULAR/rrc_rg_vars_extern.h"
#endif
#ifdef NODE_MT
#include "RRC/CELLULAR/rrc_ue_vars_extern.h"
#endif
#endif //CELLULAR
#endif


extern u8 Is_rrc_registered;
extern char Mac_dummy_buffer[50]; 
//extern char crc[10];
extern char Sorted_index_table[MAX_NB_SCHED];  
#ifndef PHY_EMUL
#ifndef PHYSIM
#define NB_INST 1
#else
extern unsigned char NB_INST;
#endif
extern unsigned char NB_CH_INST;
extern unsigned char NB_UE_INST;
extern unsigned short NODE_ID[1];
extern void* bigphys_malloc(int); 
#else
extern EMULATION_VARS *Emul_vars;
#endif //PHY_EMUL



#endif //DEF_H


