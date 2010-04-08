/*________________________openair_rrc_extern.h________________________

 Authors : Hicham Anouar
 Company : EURECOM
 Emails  : anouar@eurecom.fr
________________________________________________________________*/

#ifndef __OPENAIR_RRC_EXTERN_H__
#define __OPENAIR_RRC_EXTERN_H__
#include "defs.h"
#include "COMMON/mac_rrc_primitives.h"
#include "LAYER2/MAC/defs.h"
#include "LAYER2/RLC/rlc.h"

extern UE_RRC_INST *UE_rrc_inst;
extern CH_RRC_INST *CH_rrc_inst;
extern RRC_XFACE *Rrc_xface;
#ifndef USER_MODE
extern MAC_RLC_XFACE *Mac_rlc_xface;
extern int S_rrc;
//extern int R_rrc;
#else
#include "LAYER2/MAC/extern.h"
#ifndef NO_RRM
extern sock_rrm_t S_rrc; 
#endif
#endif

#ifndef NO_RRM
#ifndef USER_MODE
extern char *Header_buf;
extern char *Data;
extern unsigned short Header_read_idx,Data_read_idx,Header_size;
#endif
extern unsigned short Data_to_read;
#endif //NO_RRM


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
#endif


//CONSTANTS
extern rlc_info_t Rlc_info_um,Rlc_info_am_config;
//u8 RACH_TIME_ALLOC;
extern u16 RACH_FREQ_ALLOC;
//u8 NB_RACH;
extern LCHAN_DESC BCCH_LCHAN_DESC,CCCH_LCHAN_DESC,DCCH_LCHAN_DESC,DTCH_DL_LCHAN_DESC,DTCH_UL_LCHAN_DESC;
extern MAC_MEAS_T BCCH_MEAS_TRIGGER,CCCH_MEAS_TRIGGER,DCCH_MEAS_TRIGGER,DTCH_MEAS_TRIGGER;
extern MAC_AVG_T BCCH_MEAS_AVG,CCCH_MEAS_AVG,DCCH_MEAS_AVG, DTCH_MEAS_AVG;
#endif


