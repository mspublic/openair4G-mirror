

#ifndef __CONFIG_VARS_H__
#define __CONFIG_VARS_H__
#include "defs.h"
#include "PHY/defs.h"

FILE *config, *scenario;

PHY_FRAMING phyFraming[MAX_CFG_SECTIONS];
#ifndef OPENAIR_LTE
PHY_CHBCH phyCHBCH[MAX_CFG_SECTIONS]; 
PHY_MRBCH phyMRBCH[MAX_CFG_SECTIONS]; 
PHY_CHSCH phyCHSCH[MAX_CFG_SECTIONS]; 
PHY_SCH phySCH[MAX_CFG_SECTIONS];
PHY_SACH phySACH[MAX_CFG_SECTIONS]; 
#endif //OPENAIR_LTE

const cfg_Action Action[] = 
{
	{"PHY_FRAMING", phyFraming_ProcessInitReq}
#ifndef OPENAIR_LTE
, // to do some manipulation of the raw data

	{"PHY_CHSCH",   phyCHSCH_ProcessInitReq},
	{"PHY_SCH",     phySCH_ProcessInitReq},
	{"PHY_CHBCH", phyCHBCH_ProcessInitReq},
	{"PHY_MRBCH", phyMRBCH_ProcessInitReq},
	{"PHY_SACH", phySACH_ProcessInitReq}
#endif //OPENAIR_LTE
};

const cfg_Section Section[]=
{
  {"PHY_FRAMING", cfg_readPhyFraming},
#ifndef OPENAIR_LTE
	{"PHY_CHBCH", cfg_readPhyCHBCH},
	{"PHY_MRBCH", cfg_readPhyMRBCH},
	{"PHY_CHSCH", cfg_readPhyCHSCH},
	{"PHY_SCH",  cfg_readPhySCH},
	{"PHY_SACH", cfg_readPhySACH}
#endif //OPENAIR_LTE
};

#endif /*__CONFIG_VARS_H__*/
