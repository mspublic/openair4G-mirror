#include "COMMON/platform_types.h"
#include "COMMON/platform_constants.h"
#include "RadioResourceConfigCommonSIB.h"
#include "RadioResourceConfigDedicated.h"
#include "TDD-Config.h"
#include "defs.h"
#include "extern.h"

int rrc_mac_config_req(u8 Mod_id,u8 CH_flag,u8 UE_id,u8 CH_index, 
		       RadioResourceConfigCommonSIB_t *radioResourceConfigCommon,
		       struct PhysicalConfigDedicated *physicalConfigDedicated,
		       TDD_Config_t *tdd_Config,
		       u8 *SIwindowsize,
		       u16 *SIperiod) {
  
  if (CH_flag==0) 
    msg("[MAC][UE %d] Frame %d: Configuring MAC/PHY from eNB %d\n",Mod_id,mac_xface->frame,CH_index);
  else
    msg("[MAC][eNB %d] Frame %d: Configuring MAC/PHY for UE %d (%x)\n",Mod_id,mac_xface->frame,UE_id,find_UE_RNTI(Mod_id,UE_id));

  if ((tdd_Config!=NULL)||
      (SIwindowsize!=NULL)||
      (SIperiod!=NULL)){

    if (CH_flag==1)
      mac_xface->phy_config_sib1_eNB(Mod_id,tdd_Config,*SIwindowsize,*SIperiod);
    else
      mac_xface->phy_config_sib1_ue(Mod_id,CH_index,tdd_Config,*SIwindowsize,*SIperiod);
  } 

  if (radioResourceConfigCommon) {
    if (CH_flag==1)
      mac_xface->phy_config_sib2_eNB(Mod_id,radioResourceConfigCommon);
    else
      mac_xface->phy_config_sib2_ue(Mod_id,CH_index,radioResourceConfigCommon);

  }

  if (physicalConfigDedicated) {
    if (CH_flag==1)
      mac_xface->phy_config_dedicated_eNB(Mod_id,find_UE_RNTI(Mod_id,UE_id),physicalConfigDedicated);
    else
      mac_xface->phy_config_dedicated_ue(Mod_id,CH_index,physicalConfigDedicated);
  }
}
