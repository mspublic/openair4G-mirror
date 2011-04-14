#ifndef __SIMULATION_DEFS_H__
#define __SIMULATION_DEFS_H__

#include "COMMON/platform_constants.h"

#include "SIMULATION/PHY_EMULATION/impl_defs.h"
#include "SIMULATION/PHY_EMULATION/ABSTRACTION/complex.h"

typedef struct {
  unsigned short NB_NODE;
  unsigned short NODE_LIST[NB_NODE_MAX]; 
  int RSSI[NB_NODE_MAX][NB_NODE_MAX]; 
  complex16 KH[NUMBER_OF_FREQUENCY_GROUPS][NUMBER_OF_FREQUENCY_GROUPS];
  unsigned short Emul_idx[NB_MODULES_MAX]; 
  unsigned short NB_MASTER;
  unsigned short Master_list;
  unsigned short NODE_ID[NB_MODULES_MAX];
  unsigned short NB_INST;
  unsigned short NB_UE_INST;
  unsigned short NB_CH_INST;
  unsigned short Master_id;
//unsigned short NODE_ID[NB_MODULES_MAX]; 

} EMULATION_VARS;

#define NB_NODE Emul_vars->NB_NODE
#define NODE_LIST Emul_vars->NODE_LIST
#define RSSI Emul_vars->RSSI
#define KH Emul_vars->KH
#define Emul_idx Emul_vars->Emul_idx
#define NB_MASTER Emul_vars->NB_MASTER
#define Master_list Emul_vars->Master_list
#define NODE_ID Emul_vars->NODE_ID
#define NB_INST Emul_vars->NB_INST
#define NB_UE_INST Emul_vars->NB_UE_INST
#define NB_CH_INST Emul_vars->NB_CH_INST
#define Master_id Emul_vars->Master_id
//unsigned short NODE_ID[NB_MODULES_MAX]; 


#endif
