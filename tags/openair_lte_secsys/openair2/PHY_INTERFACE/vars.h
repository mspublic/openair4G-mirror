#ifndef __PHY_INTERFACE_VARS_H__
#define __PHY_INTERFACE_VARS_H__

//#include "SIMULATION/PHY_EMULATION/spec_defs.h"
#include "defs.h"

#ifdef PHY_EMUL
#include "SIMULATION/PHY_EMULATION/DEVICE_DRIVER/defs.h"
#include "SIMULATION/simulation_defs.h"
#endif


unsigned int frame;
unsigned int mac_debug;

//MAC_xface *mac_xface;

//MACPHY_PARAMS MACPHY_params;

unsigned int mac_registered;

MACPHY_DATA_REQ_TABLE Macphy_req_table[NB_MODULES_MAX];

#endif

#ifndef USER_MODE
EXPORT_SYMBOL(mac_xface);
#ifdef PHY_EMUL
EXPORT_SYMBOL(Emul_vars);
EXPORT_SYMBOL(Macphy_req_table);
#endif //PHY_EMUL

//EXPORT_SYMBOL(NB_INST);
//EXPORT_SYMBOL(NB_UE_INST);
//EXPORT_SYMBOL(NB_CH_INST);
//EXPORT_SYMBOL(NODE_ID); 
#endif //USER_MODE
