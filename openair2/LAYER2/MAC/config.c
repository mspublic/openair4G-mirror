#include "COMMON/platform_types.h"
#include "COMMON/platform_constants.h"
#include "RadioResourceConfigCommonSIB.h"
#include "RadioResourceConfigDedicated.h"
#include "MeasGapConfig.h"
#include "TDD-Config.h"
#include "defs.h"
#include "extern.h"
#include "UTIL/LOG/log_if.h"

int rrc_mac_config_req(u8 Mod_id,u8 eNB_flag,u8 UE_id,u8 eNB_index, 
		       RadioResourceConfigCommonSIB_t *radioResourceConfigCommon,
		       struct PhysicalConfigDedicated *physicalConfigDedicated,
		       MAC_MainConfig_t *mac_MainConfig,
		       long logicalChannelIdentity,
		       LogicalChannelConfig_t *logicalChannelConfig,
		       MeasGapConfig_t *measGapConfig,
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
      
      msg("pusch_config_common.n_SB = %ld\n",radioResourceConfigCommon->pusch_ConfigCommon.pusch_ConfigBasic.n_SB);
      
      
      msg("pusch_config_common.hoppingMode = %ld\n",radioResourceConfigCommon->pusch_ConfigCommon.pusch_ConfigBasic.hoppingMode);
      
      msg("pusch_config_common.pusch_HoppingOffset = %ld\n",  radioResourceConfigCommon->pusch_ConfigCommon.pusch_ConfigBasic.pusch_HoppingOffset);
      
      msg("pusch_config_common.enable64QAM = %d\n",radioResourceConfigCommon->pusch_ConfigCommon.pusch_ConfigBasic.enable64QAM);
      
      msg("pusch_config_common.groupHoppingEnabled = %d\n",radioResourceConfigCommon->pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupHoppingEnabled);
      
      
      msg("pusch_config_common.groupAssignmentPUSCH = %ld\n",radioResourceConfigCommon->pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH);
      
      
      msg("pusch_config_common.sequenceHoppingEnabled = %d\n",radioResourceConfigCommon->pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled);
      
      
      msg("pusch_config_common.cyclicShift  = %ld\n",radioResourceConfigCommon->pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.cyclicShift); 
      mac_xface->phy_config_sib2_eNB(Mod_id,radioResourceConfigCommon);
    }
    else
      mac_xface->phy_config_sib2_ue(Mod_id,eNB_index,radioResourceConfigCommon);

  }
  
  if (logicalChannelConfig!= NULL) {
    if (eNB_flag==0)
      UE_mac_inst[Mod_id].scheduling_info.logicalChannelConfig[logicalChannelIdentity]=logicalChannelConfig;
  }

  if (eNB_flag==0){
    UE_mac_inst[Mod_id].scheduling_info.macConfig=mac_MainConfig;
    UE_mac_inst[Mod_id].scheduling_info.measGapConfig=measGapConfig;
    if (mac_MainConfig!= NULL) {
      LOG_I(MAC,"[UE%d] Applying RRC macMainConfig from eNB%d\n",Mod_id,eNB_index);
      //UE_mac_inst[Mod_id].scheduling_info.macConfig=mac_MainConfig;
    }else{ // default values as deined in 36.331 sec 9.2.2
      LOG_I(MAC,"[UE%d] Applying default macMainConfig\n",Mod_id);
      //UE_mac_inst[Mod_id].scheduling_info.macConfig=NULL;
      UE_mac_inst[Mod_id].scheduling_info.retxBSR_Timer= MAC_MainConfig__ul_SCH_Config__retxBSR_Timer_sf2560;
      UE_mac_inst[Mod_id].scheduling_info.periodicBSR_Timer=MAC_MainConfig__ul_SCH_Config__periodicBSR_Timer_infinity;
      UE_mac_inst[Mod_id].scheduling_info.sr_ProhibitTimer=0;
      UE_mac_inst[Mod_id].scheduling_info.maxHARQ_tx=MAC_MainConfig__ul_SCH_Config__maxHARQ_Tx_n5;
      UE_mac_inst[Mod_id].scheduling_info.ttiBundling=0;
      UE_mac_inst[Mod_id].scheduling_info.drx_config=DRX_Config_PR_release;
      UE_mac_inst[Mod_id].scheduling_info.phr_config=MAC_MainConfig__phr_Config_PR_release;
    }
  }

  if (physicalConfigDedicated != NULL) {
    if (eNB_flag==1)
      mac_xface->phy_config_dedicated_eNB(Mod_id,find_UE_RNTI(Mod_id,UE_id),physicalConfigDedicated);
    else{
      mac_xface->phy_config_dedicated_ue(Mod_id,eNB_index,physicalConfigDedicated);
      UE_mac_inst[Mod_id].scheduling_info.physicalConfigDedicated=physicalConfigDedicated; // for SR proc
    }
  }
}
