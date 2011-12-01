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
#include "PHY/defs.h"
#include "defs.h"
#include "PHY_INTERFACE/defs.h"
#include "COMMON/mac_rrc_primitives.h"



u32 RRC_CONNECTION_FLAG;

UE_MAC_INST *UE_mac_inst; //[NB_MODULE_MAX]; 
eNB_MAC_INST *eNB_mac_inst; //[NB_MODULE_MAX]; 
MAC_RLC_XFACE *Mac_rlc_xface;

eNB_ULSCH_INFO eNB_ulsch_info[NUMBER_OF_eNB_MAX][NUMBER_OF_UE_MAX]; // eNBxUE = 8x8 
eNB_DLSCH_INFO eNB_dlsch_info[NUMBER_OF_eNB_MAX][NUMBER_OF_UE_MAX]; // eNBxUE = 8x8 

#ifndef USER_MODE
RRC_XFACE *Rrc_xface;
MAC_xface *mac_xface;
#else
#include "PHY_INTERFACE/extern.h"
#include "RRC/LITE/extern.h"
#endif

u8 Is_rrc_registered;

#ifndef PHY_EMUL
unsigned char NB_eNB_INST;
unsigned char NB_UE_INST;
#endif


DCI0_5MHz_TDD_1_6_t       UL_alloc_pdu;

DCI1A_5MHz_TDD_1_6_t      DLSCH_alloc_pdu1A;
DCI1A_5MHz_TDD_1_6_t      RA_alloc_pdu;
DCI1A_5MHz_TDD_1_6_t      BCCH_alloc_pdu;

DCI1A_5MHz_TDD_1_6_t      CCCH_alloc_pdu;
DCI1_5MHz_TDD_t           DLSCH_alloc_pdu;
DCI2_5MHz_2A_L10PRB_TDD_t DLSCH_alloc_pdu1;
DCI2_5MHz_2A_M10PRB_TDD_t DLSCH_alloc_pdu2;

DCI2_5MHz_2D_M10PRB_TDD_t DLSCH_alloc_pdu2D;

#endif


