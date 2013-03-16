/*________________________MAC/extern.h________________________

 Authors : Hicham Anouar, Raymond Knopp
 Company : EURECOM
 Emails  : anouar@eurecom.fr,  knopp@eurecom.fr
________________________________________________________________*/


#ifndef __MAC_EXTERN_H__
#define __MAC_EXTERN_H__


#ifdef USER_MODE
//#include "stdio.h"
#endif //USER_MODE
#include "PHY/defs.h"
#include "defs.h"
#include "COMMON/mac_rrc_primitives.h"
#ifdef PHY_EMUL
//#include "SIMULATION/simulation_defs.h"
#endif //PHY_EMUL
#include "PHY_INTERFACE/defs.h"

extern const u32 BSR_TABLE[BSR_TABLE_SIZE];
//extern u32 EBSR_Level[63];

extern UE_MAC_INST *UE_mac_inst;
extern eNB_MAC_INST *eNB_mac_inst;
extern MAC_RLC_XFACE *Mac_rlc_xface;
extern u8 Is_rrc_registered;

extern eNB_ULSCH_INFO eNB_ulsch_info[NUMBER_OF_eNB_MAX][NUMBER_OF_UE_MAX]; // eNBxUE = 8x8 
extern eNB_DLSCH_INFO eNB_dlsch_info[NUMBER_OF_eNB_MAX][NUMBER_OF_UE_MAX]; // eNBxUE = 8x8 



//#ifndef USER_MODE
extern MAC_xface *mac_xface;
extern RRC_XFACE *Rrc_xface;

extern u8 Is_rrc_registered;

#ifndef PHY_EMUL
#ifndef PHYSIM
#define NB_INST 1
#else
extern unsigned char NB_INST;
#endif
extern unsigned char NB_eNB_INST;
extern unsigned char NB_UE_INST;
extern unsigned short NODE_ID[1];
extern void* bigphys_malloc(int); 
#else
extern EMULATION_VARS *Emul_vars;
#endif //PHY_EMUL


extern u32 RRC_CONNECTION_FLAG;


extern DCI0_5MHz_TDD_1_6_t       UL_alloc_pdu;

extern DCI1A_5MHz_TDD_1_6_t      RA_alloc_pdu;
extern DCI1A_5MHz_TDD_1_6_t      DLSCH_alloc_pdu1A;
extern DCI1A_5MHz_TDD_1_6_t      BCCH_alloc_pdu;

extern DCI1A_5MHz_TDD_1_6_t      CCCH_alloc_pdu;
extern DCI1_5MHz_TDD_t           DLSCH_alloc_pdu;

extern DCI0_5MHz_FDD_t       UL_alloc_pdu_fdd;

extern DCI1A_5MHz_FDD_t      DLSCH_alloc_pdu1A_fdd;
extern DCI1A_5MHz_FDD_t      RA_alloc_pdu_fdd;
extern DCI1A_5MHz_FDD_t      BCCH_alloc_pdu_fdd;

extern DCI1A_5MHz_FDD_t      CCCH_alloc_pdu_fdd;
extern DCI1_5MHz_FDD_t       DLSCH_alloc_pdu_fdd;

extern DCI2_5MHz_2A_L10PRB_TDD_t DLSCH_alloc_pdu1;
extern DCI2_5MHz_2A_M10PRB_TDD_t DLSCH_alloc_pdu2;
extern DCI1E_5MHz_2A_M10PRB_TDD_t DLSCH_alloc_pdu1E;


extern struct virtual_links vlinksTable[NB_MAX_CH];
// mac layer forwarding table
extern struct forwarding_Table forwardingTable[NB_MAX_CH];
// CO-RNTIs of the UE
//extern struct cornti_array corntis;
#endif //DEF_H


