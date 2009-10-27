
#ifndef __CONFIG_EXTERN_H__
#define __CONFIG_EXTERN_H__

#include <stdio.h>
#include "PHY/defs.h"
#include "defs.h"


extern FILE *config, *scenario;
extern PHY_FRAMING phyFraming[MAX_CFG_SECTIONS];
#ifndef OPENAIR_LTE
extern PHY_CHBCH phyCHBCH[MAX_CFG_SECTIONS]; 
extern PHY_MRBCH phyMRBCH[MAX_CFG_SECTIONS]; 
extern PHY_CHSCH phyCHSCH[MAX_CFG_SECTIONS]; 
extern PHY_SCH phySCH[MAX_CFG_SECTIONS];
extern PHY_SACH phySACH[MAX_CFG_SECTIONS]; 
#endif //OPENAIR_LTE
extern const cfg_Action Action[];
extern const cfg_Section Section[];



#endif /* __CONFIG_EXTERN_H__*/
