

#ifndef __CONFIG_VARS_H__
#define __CONFIG_VARS_H__
#include "config_proto.h"
#include "config_defs.h"
//#include "openair_defs.h"
#include "SIMULATION/PHY_EMULATION/spec_defs.h"

FILE *config, *scenario;

PHY_FRAMING phyFraming[MAX_CFG_SECTIONS];
PHY_CHBCH phyCHBCH[MAX_CFG_SECTIONS]; 
PHY_MRBCH phyMRBCH[MAX_CFG_SECTIONS]; 
PHY_CHSCH phyCHSCH[MAX_CFG_SECTIONS]; 
PHY_SCH phySCH[MAX_CFG_SECTIONS];
PHY_SACH phySACH[MAX_CFG_SECTIONS]; 

const cfg_Action Action[] = 
{
	{"PHY_FRAMING", phyFraming_ProcessInitReq}, // to do some manipulation of the raw data
	{"PHY_CHSCH",   phyCHSCH_ProcessInitReq},
	{"PHY_SCH",     phySCH_ProcessInitReq},
	{"PHY_CHBCH", phyCHBCH_ProcessInitReq},
	{"PHY_MRBCH", phyMRBCH_ProcessInitReq},
	{"PHY_SACH", phySACH_ProcessInitReq}
};

const cfg_Section Section[]=
{
	{"PHY_FRAMING", cfg_readPhyFraming},
	{"PHY_CHBCH", cfg_readPhyCHBCH},
	{"PHY_MRBCH", cfg_readPhyMRBCH},
	{"PHY_CHSCH", cfg_readPhyCHSCH},
	{"PHY_SCH",  cfg_readPhySCH},
	{"PHY_SACH", cfg_readPhySACH}
};

#endif /*__CONFIG_VARS_H__*/
