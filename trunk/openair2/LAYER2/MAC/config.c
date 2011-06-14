#include "COMMON/platform_types.h"
#include "COMMON/platform_constants.h"
#include "RadioResourceConfigCommonSIB.h"
#include "RadioResourceConfigDedicated.h"
#include "TDD-Config.h"
#include "defs.h"
#include "extern.h"

int rrc_mac_config_req(u8 Mod_id,u8 eNB_flag,u8 UE_id,u8 eNB_index, 
		       RadioResourceConfigCommonSIB_t *radioResourceConfigCommon,
		       struct PhysicalConfigDedicated *physicalConfigDedicated,
		       TDD_Config_t *tdd_Config,
		       u8 *SIwindowsize,
		       u16 *SIperiod) {
  
  if (eNB_flag==0) 
    msg("[MAC][UE %d] Frame %d: Configuring MAC/PHY from eNB %d\n",Mod_id,mac_xface->frame,eNB_index);
  else {
    if (physicalConfigDedicated == NULL)
      msg("[MAC][eNB %d] Frame %d: Configuring MAC/PHY\n",Mod_id,mac_xface->frame);
    else
      msg("[MAC][eNB %d] Frame %d: Configuring MAC/PHY for UE %d (%x)\n",Mod_id,mac_xface->frame,UE_id,find_UE_RNTI(Mod_id,UE_id));
  }

  if ((tdd_Config!=NULL)||
      (SIwindowsize!=NULL)||
      (SIperiod!=NULL)){

    if (eNB_flag==1)
      mac_xface->phy_config_sib1_eNB(Mod_id,tdd_Config,*SIwindowsize,*SIperiod);
    else
      mac_xface->phy_config_sib1_ue(Mod_id,eNB_index,tdd_Config,*SIwindowsize,*SIperiod);
  } 

  if (radioResourceConfigCommon) {
    if (eNB_flag==1) {
      msg("SIB2/3 Contents (partial)\n");
      
      msg("pusch_config_common.n_SB = %l\n",radioResourceConfigCommon->pusch_ConfigCommon.pusch_ConfigBasic.n_SB);
      
      
      msg("pusch_config_common.hoppingMode = %l\n",radioResourceConfigCommon->pusch_ConfigCommon.pusch_ConfigBasic.hoppingMode);
      
      msg("pusch_config_common.pusch_HoppingOffset = %l\n",  radioResourceConfigCommon->pusch_ConfigCommon.pusch_ConfigBasic.pusch_HoppingOffset);
      
      msg("pusch_config_common.enable64QAM = %d\n",radioResourceConfigCommon->pusch_ConfigCommon.pusch_ConfigBasic.enable64QAM);
      
      msg("pusch_config_common.groupHoppingEnabled = %d\n",radioResourceConfigCommon->pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupHoppingEnabled);
      
      
      msg("pusch_config_common.groupAssignmentPUSCH = %l\n",radioResourceConfigCommon->pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH);
      
      
      msg("pusch_config_common.sequenceHoppingEnabled = %d\n",radioResourceConfigCommon->pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled);
      
      
      msg("pusch_config_common.cyclicShift  = %l\n",radioResourceConfigCommon->pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.cyclicShift); 
      mac_xface->phy_config_sib2_eNB(Mod_id,radioResourceConfigCommon);
    }
    else
      mac_xface->phy_config_sib2_ue(Mod_id,eNB_index,radioResourceConfigCommon);

  }

  if (physicalConfigDedicated) {
    if (eNB_flag==1)
      mac_xface->phy_config_dedicated_eNB(Mod_id,find_UE_RNTI(Mod_id,UE_id),physicalConfigDedicated);
    else
      mac_xface->phy_config_dedicated_ue(Mod_id,eNB_index,physicalConfigDedicated);
  }
}
