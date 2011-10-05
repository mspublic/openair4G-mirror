#include "COMMON/platform_types.h"
#include "COMMON/platform_constants.h"
#include "RadioResourceConfigCommonSIB.h"
#include "RadioResourceConfigDedicated.h"
#include "MeasGapConfig.h"
#include "TDD-Config.h"
#include "defs.h"
#include "extern.h"
#include "UTIL/LOG/log.h"

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
  
  if (eNB_flag==0) {
    LOG_I(MAC,"[CONFIG][UE %d] Frame %d: Configuring MAC/PHY from eNB %d\n",Mod_id,mac_xface->frame,eNB_index);
  }else {
    if (physicalConfigDedicated == NULL){
      LOG_I(MAC,"[CONFIG][eNB %d] Frame %d: Configuring MAC/PHY\n",Mod_id,mac_xface->frame);
   } else{
      LOG_I(MAC,"[CONFIG][eNB %d] Frame %d: Configuring MAC/PHY for UE %d (%x)\n",Mod_id,mac_xface->frame,UE_id,find_UE_RNTI(Mod_id,UE_id));
  }
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
      LOG_I(MAC,"[CONFIG]SIB2/3 Contents (partial)\n");
      
      LOG_I(MAC,"[CONFIG]pusch_config_common.n_SB = %ld\n",radioResourceConfigCommon->pusch_ConfigCommon.pusch_ConfigBasic.n_SB);
      
      
      LOG_I(MAC,"[CONFIG]pusch_config_common.hoppingMode = %ld\n",radioResourceConfigCommon->pusch_ConfigCommon.pusch_ConfigBasic.hoppingMode);
      
      LOG_I(MAC,"[CONFIG]pusch_config_common.pusch_HoppingOffset = %ld\n",  radioResourceConfigCommon->pusch_ConfigCommon.pusch_ConfigBasic.pusch_HoppingOffset);
      
      LOG_I(MAC,"[CONFIG]pusch_config_common.enable64QAM = %d\n",radioResourceConfigCommon->pusch_ConfigCommon.pusch_ConfigBasic.enable64QAM);
      
      LOG_I(MAC,"[CONFIG]pusch_config_common.groupHoppingEnabled = %d\n",radioResourceConfigCommon->pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupHoppingEnabled);
      
      
      LOG_I(MAC,"[CONFIG]pusch_config_common.groupAssignmentPUSCH = %ld\n",radioResourceConfigCommon->pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH);
      
      
      LOG_I(MAC,"[CONFIG]pusch_config_common.sequenceHoppingEnabled = %d\n",radioResourceConfigCommon->pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled);
      
      
      LOG_I(MAC,"[CONFIG]pusch_config_common.cyclicShift  = %ld\n",radioResourceConfigCommon->pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.cyclicShift); 
      mac_xface->phy_config_sib2_eNB(Mod_id,radioResourceConfigCommon);
    }
    else
      mac_xface->phy_config_sib2_ue(Mod_id,eNB_index,radioResourceConfigCommon);

  }
  
  if (logicalChannelConfig!= NULL) {
    if (eNB_flag==0){
      UE_mac_inst[Mod_id].scheduling_info.logicalChannelConfig[logicalChannelIdentity]=logicalChannelConfig;
      UE_mac_inst[Mod_id].scheduling_info.Bj[logicalChannelIdentity]=0; // initilize the bucket for this lcid
      UE_mac_inst[Mod_id].scheduling_info.bucket_size[logicalChannelIdentity]=logicalChannelConfig->ul_SpecificParameters->prioritisedBitRate *
	logicalChannelConfig->ul_SpecificParameters->bucketSizeDuration; // set the max bucket size 
    } 
  }

  if (mac_MainConfig != NULL){
    if (eNB_flag==0){
      LOG_I(MAC,"[CONFIG][UE%d] Applying RRC macMainConfig from eNB%d\n",Mod_id,eNB_index);
      UE_mac_inst[Mod_id].scheduling_info.macConfig=mac_MainConfig;
      UE_mac_inst[Mod_id].scheduling_info.measGapConfig=measGapConfig;
      UE_mac_inst[Mod_id].scheduling_info.periodicBSR_Timer = mac_MainConfig->ul_SCH_Config->periodicBSR_Timer;
      UE_mac_inst[Mod_id].scheduling_info.retxBSR_Timer     = mac_MainConfig->ul_SCH_Config->retxBSR_Timer;
      UE_mac_inst[Mod_id].scheduling_info.sr_ProhibitTimer  = mac_MainConfig->sr_ProhibitTimer_r9;
      UE_mac_inst[Mod_id].scheduling_info.periodicBSR_SF  = get_sf_periodicBSRTimer(UE_mac_inst[Mod_id].scheduling_info.periodicBSR_Timer);
      UE_mac_inst[Mod_id].scheduling_info.retxBSR_SF     = get_sf_retxBSRTimer(UE_mac_inst[Mod_id].scheduling_info.retxBSR_Timer);
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
