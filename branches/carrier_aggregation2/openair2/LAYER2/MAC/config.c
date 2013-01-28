#include "COMMON/platform_types.h"
#include "COMMON/platform_constants.h"
#include "RadioResourceConfigCommonSIB.h"
#include "RadioResourceConfigDedicated.h"
#include "MeasGapConfig.h"
#include "MeasObjectToAddModList.h"
#include "TDD-Config.h"
#include "defs.h"
#include "extern.h"
#include "UTIL/LOG/log.h"
#ifdef Rel10
#include "MBSFN-AreaInfoList-r9.h"
#include "MBSFN-AreaInfo-r9.h"
#include "MBSFN-SubframeConfigList.h"
#endif

int rrc_mac_config_req(u8 Mod_id,u8 eNB_flag,u8 UE_id,u8 eNB_index, 
		       RadioResourceConfigCommonSIB_t *radioResourceConfigCommon,
		       struct PhysicalConfigDedicated *physicalConfigDedicated,
#ifdef Rel10
		       struct PhysicalConfigDedicatedSCell_r10 *physicalConfigDedicatedSCell_r10,
#endif
		       MeasObjectToAddMod_t **measObj,
		       MAC_MainConfig_t *mac_MainConfig,
		       long logicalChannelIdentity,
		       LogicalChannelConfig_t *logicalChannelConfig,
		       MeasGapConfig_t *measGapConfig,
		       TDD_Config_t *tdd_Config,
		       u8 *SIwindowsize,
		       u16 *SIperiod
#ifdef Rel10
		       ,
		       u8 MBMS_Flag,
		       struct MBSFN_SubframeConfigList *mbsfn_SubframeConfigList,
		       MBSFN_AreaInfoList_r9_t *mbsfn_AreaInfoList
#endif 
		       ) {

  int i,CC_id;

  if (eNB_flag==0) {
    LOG_I(MAC,"[CONFIG][UE %d] Configuring MAC/PHY from eNB %d\n",Mod_id,eNB_index);
    if (tdd_Config != NULL)
      UE_mac_inst[Mod_id].tdd_Config = tdd_Config;
  }else {
    if (physicalConfigDedicated == NULL){
      LOG_I(MAC,"[CONFIG][eNB %d] Configuring MAC/PHY\n",Mod_id);
    } else{
      LOG_I(MAC,"[CONFIG][eNB %d] Configuring MAC/PHY for UE %d (%x)\n",Mod_id,UE_id,find_UE_RNTI(Mod_id,UE_id));
    }
  }
  
  if ((tdd_Config!=NULL)||
      (SIwindowsize!=NULL)||
      (SIperiod!=NULL)){

    if (eNB_flag==1)
      mac_xface->phy_config_sib1_eNB(Mod_id,0,tdd_Config,*SIwindowsize,*SIperiod);
    else
      mac_xface->phy_config_sib1_ue(Mod_id,0,eNB_index,tdd_Config,*SIwindowsize,*SIperiod);
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
      
      mac_xface->phy_config_sib2_eNB(Mod_id,0,radioResourceConfigCommon);
    }
    else {
      UE_mac_inst[Mod_id].radioResourceConfigCommon = radioResourceConfigCommon;

      mac_xface->phy_config_sib2_ue(Mod_id,0,eNB_index,radioResourceConfigCommon);
    }
  }
  
  if (logicalChannelConfig!= NULL) {
    if (eNB_flag==0){
      LOG_I(MAC,"[CONFIG][UE %d] Applying RRC logicalChannelConfig from eNB%d\n",Mod_id,eNB_index);
      UE_mac_inst[Mod_id].logicalChannelConfig[logicalChannelIdentity]=logicalChannelConfig;
      UE_mac_inst[Mod_id].scheduling_info.Bj[logicalChannelIdentity]=0; // initilize the bucket for this lcid
      if (logicalChannelConfig->ul_SpecificParameters)
	UE_mac_inst[Mod_id].scheduling_info.bucket_size[logicalChannelIdentity]=logicalChannelConfig->ul_SpecificParameters->prioritisedBitRate *
	  logicalChannelConfig->ul_SpecificParameters->bucketSizeDuration; // set the max bucket size
      else {
	LOG_E(MAC,"[CONFIG][UE %d] LCID %d NULL ul_SpecificParameters\n",Mod_id,logicalChannelIdentity);
	mac_xface->macphy_exit("");
      }
    } 
  }

  if (mac_MainConfig != NULL){
    if (eNB_flag==0){
      LOG_I(MAC,"[CONFIG][UE%d] Applying RRC macMainConfig from eNB%d\n",Mod_id,eNB_index);
      UE_mac_inst[Mod_id].macConfig=mac_MainConfig;
      UE_mac_inst[Mod_id].measGapConfig=measGapConfig;
      
      if (mac_MainConfig->ul_SCH_Config->periodicBSR_Timer)
	UE_mac_inst[Mod_id].scheduling_info.periodicBSR_Timer = (u16) *mac_MainConfig->ul_SCH_Config->periodicBSR_Timer;
      else
	UE_mac_inst[Mod_id].scheduling_info.periodicBSR_Timer = (u16) MAC_MainConfig__ul_SCH_Config__periodicBSR_Timer_infinity;

      if (mac_MainConfig->ul_SCH_Config->maxHARQ_Tx)
	UE_mac_inst[Mod_id].scheduling_info.maxHARQ_Tx     = (u16) *mac_MainConfig->ul_SCH_Config->maxHARQ_Tx;
      else
	UE_mac_inst[Mod_id].scheduling_info.maxHARQ_Tx     = (u16) MAC_MainConfig__ul_SCH_Config__maxHARQ_Tx_n5;

      UE_mac_inst[Mod_id].scheduling_info.retxBSR_Timer     = (u16) mac_MainConfig->ul_SCH_Config->retxBSR_Timer;
#ifdef Rel10   
      if (mac_MainConfig->sr_ProhibitTimer_r9) 
	UE_mac_inst[Mod_id].scheduling_info.sr_ProhibitTimer  = (u16) *mac_MainConfig->sr_ProhibitTimer_r9;
      else
	UE_mac_inst[Mod_id].scheduling_info.sr_ProhibitTimer  = (u16) 0;
#endif
      UE_mac_inst[Mod_id].scheduling_info.periodicBSR_SF  = get_sf_periodicBSRTimer(UE_mac_inst[Mod_id].scheduling_info.periodicBSR_Timer);
      UE_mac_inst[Mod_id].scheduling_info.retxBSR_SF     = get_sf_retxBSRTimer(UE_mac_inst[Mod_id].scheduling_info.retxBSR_Timer);
      
      UE_mac_inst[Mod_id].scheduling_info.drx_config     = mac_MainConfig->drx_Config;
      UE_mac_inst[Mod_id].scheduling_info.phr_config     = mac_MainConfig->phr_Config;
      if (mac_MainConfig->phr_Config){
	UE_mac_inst[Mod_id].PHR_state = mac_MainConfig->phr_Config->present;
	UE_mac_inst[Mod_id].PHR_reconfigured = 1;
	UE_mac_inst[Mod_id].scheduling_info.periodicPHR_Timer = mac_MainConfig->phr_Config->choice.setup.periodicPHR_Timer;
	UE_mac_inst[Mod_id].scheduling_info.prohibitPHR_Timer = mac_MainConfig->phr_Config->choice.setup.prohibitPHR_Timer;
	UE_mac_inst[Mod_id].scheduling_info.PathlossChange = mac_MainConfig->phr_Config->choice.setup.dl_PathlossChange;
      } else {
	UE_mac_inst[Mod_id].PHR_reconfigured = 0;
	UE_mac_inst[Mod_id].PHR_state = MAC_MainConfig__phr_Config_PR_setup;
	UE_mac_inst[Mod_id].scheduling_info.periodicPHR_Timer = MAC_MainConfig__phr_Config__setup__periodicPHR_Timer_sf20;
	UE_mac_inst[Mod_id].scheduling_info.prohibitPHR_Timer = MAC_MainConfig__phr_Config__setup__prohibitPHR_Timer_sf20;
	UE_mac_inst[Mod_id].scheduling_info.PathlossChange = MAC_MainConfig__phr_Config__setup__dl_PathlossChange_dB1;
      }	
      UE_mac_inst[Mod_id].scheduling_info.periodicPHR_SF =  get_sf_perioidicPHR_Timer(UE_mac_inst[Mod_id].scheduling_info.periodicPHR_Timer);
      UE_mac_inst[Mod_id].scheduling_info.prohibitPHR_SF =  get_sf_prohibitPHR_Timer(UE_mac_inst[Mod_id].scheduling_info.prohibitPHR_Timer);
      UE_mac_inst[Mod_id].scheduling_info.PathlossChange_db =  get_db_dl_PathlossChange(UE_mac_inst[Mod_id].scheduling_info.PathlossChange);
      LOG_D(MAC,"[UE %d] config PHR (%d): periodic %d (SF) prohibit %d (SF)  pathlosschange %d (db) \n",
	    Mod_id,mac_MainConfig->phr_Config->present, 
	    UE_mac_inst[Mod_id].scheduling_info.periodicPHR_SF,
	    UE_mac_inst[Mod_id].scheduling_info.prohibitPHR_SF,
	    UE_mac_inst[Mod_id].scheduling_info.PathlossChange_db);
    }
  }

  if (physicalConfigDedicated != NULL) {
    if (eNB_flag==1){
      mac_xface->phy_config_dedicated_eNB(Mod_id,0,find_UE_RNTI(Mod_id,UE_id),physicalConfigDedicated);
    }else{

      mac_xface->phy_config_dedicated_ue(Mod_id,0,eNB_index,physicalConfigDedicated);
      UE_mac_inst[Mod_id].physicalConfigDedicated=physicalConfigDedicated; // for SR proc
      //memcpy(UE_mac_inst[Mod_id].physicalConfigDedicated,physicalConfigDedicated,sizeof(PhysicalConfigDedicated_t));
    }
  }
#ifdef Rel10
  if (physicalConfigDedicatedSCell_r10 != NULL) {

	if (eNB_flag==1){
	  mac_xface->phy_config_dedicated_scell_eNB(Mod_id,find_UE_RNTI(Mod_id,UE_id),physicalConfigDedicatedSCell_r10,1);
	}
	else {
	  mac_xface->phy_config_dedicated_scell_ue(Mod_id,eNB_index,physicalConfigDedicatedSCell_r10,1);
	  UE_mac_inst[Mod_id].physicalConfigDedicatedSCell_r10=physicalConfigDedicatedSCell_r10; // using SCell index 0
	}
  }
#endif


  if (eNB_flag == 0) {
    if (measObj!= NULL) 
      if (measObj[0]!= NULL){
	UE_mac_inst[Mod_id].n_adj_cells = measObj[0]->measObject.choice.measObjectEUTRA.cellsToAddModList->list.count;
	LOG_I(MAC,"Number of adjacent cells %d\n",UE_mac_inst[Mod_id].n_adj_cells);
	for (i=0;i<UE_mac_inst[Mod_id].n_adj_cells;i++) {
	  UE_mac_inst[Mod_id].adj_cell_id[i] = measObj[0]->measObject.choice.measObjectEUTRA.cellsToAddModList->list.array[i]->physCellId;
	  LOG_I(MAC,"Cell %d : Nid_cell %d\n",i,UE_mac_inst[Mod_id].adj_cell_id[i]);
	}
	for (CC_id=0;CC_id<MAX_NUM_CCs;CC_id++)
	  mac_xface->phy_config_meas_ue(Mod_id,CC_id,eNB_index,UE_mac_inst[Mod_id].n_adj_cells,UE_mac_inst[Mod_id].adj_cell_id);
      }
  }

#ifdef Rel10

  if (mbsfn_SubframeConfigList != NULL) {
    if (eNB_flag == 1) {
      for (i=0; i<mbsfn_SubframeConfigList->list.count; i++) {
	eNB_mac_inst[Mod_id].mbsfn_SubframeConfig[i] = mbsfn_SubframeConfigList->list.array[i];
	LOG_I(MAC, "[CONFIG] MBSFN_SubframeConfig[%d] pattern is  %ld\n", i, 
	      eNB_mac_inst[Mod_id].mbsfn_SubframeConfig[i]->subframeAllocation.choice.oneFrame.buf[0]); 
      }
      eNB_mac_inst[Mod_id].MBMS_flag = MBMS_Flag;
    }
    else {
      // UE
    }
  }

  if (mbsfn_AreaInfoList != NULL) {
    if (eNB_flag == 1) {
      LOG_I(MAC, "[CONFIG] SIB13 Contents (partial)\n");
      LOG_I(MAC, "[CONFIG] Number of Area Info list %d\n", mbsfn_AreaInfoList->list.count);
      for (i =0; i< mbsfn_AreaInfoList->list.count; i++) {
	eNB_mac_inst[Mod_id].mbsfn_AreaInfo[i] = mbsfn_AreaInfoList->list.array[i];
	LOG_I(MAC, "[CONFIG] MBSFN_AreaInfo[%d]: MCCH Repetition Period = %ld\n", i, 
	      eNB_mac_inst[Mod_id].mbsfn_AreaInfo[i]->mcch_Config_r9.mcch_RepetitionPeriod_r9); 
      }
    }
    else { 
      // UE
    }
  }
  
#endif
  return(0);
}
