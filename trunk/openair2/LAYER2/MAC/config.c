#include "COMMON/platform_types.h"
#include "COMMON/platform_constants.h"
#include "SystemInformationBlockType2.h"
//#include "RadioResourceConfigCommonSIB.h"
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
#include "PMCH-InfoList-r9.h"
#endif
int rrc_mac_config_req(u8 Mod_id,u8 eNB_flag,u8 UE_id,u8 eNB_index, 
		       RadioResourceConfigCommonSIB_t *radioResourceConfigCommon,
		       struct PhysicalConfigDedicated *physicalConfigDedicated,
		       MeasObjectToAddMod_t **measObj,
		       MAC_MainConfig_t *mac_MainConfig,
		       long logicalChannelIdentity,
		       LogicalChannelConfig_t *logicalChannelConfig,
		       MeasGapConfig_t *measGapConfig,
		       TDD_Config_t *tdd_Config,
		       u8 *SIwindowsize,
		       u16 *SIperiod,
		       ARFCN_ValueEUTRA_t *ul_CarrierFreq,
		       long *ul_Bandwidth,
		       AdditionalSpectrumEmission_t *additionalSpectrumEmission,
		       struct MBSFN_SubframeConfigList *mbsfn_SubframeConfigList
#ifdef Rel10
		       ,
		       u8 MBMS_Flag,
		       MBSFN_AreaInfoList_r9_t *mbsfn_AreaInfoList,
		       PMCH_InfoList_r9_t *pmch_InfoList
#endif 
#ifdef CBA
		       ,
		       u8 num_active_cba_groups,
		       u16 CBA_RNTI
#endif
		       ) {

  int i;

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
      
      mac_xface->phy_config_sib2_eNB(Mod_id,radioResourceConfigCommon,ul_CarrierFreq,ul_Bandwidth,additionalSpectrumEmission,mbsfn_SubframeConfigList);
    }
    else {
      UE_mac_inst[Mod_id].radioResourceConfigCommon = radioResourceConfigCommon;
      mac_xface->phy_config_sib2_ue(Mod_id,eNB_index,radioResourceConfigCommon,ul_CarrierFreq,ul_Bandwidth,additionalSpectrumEmission,mbsfn_SubframeConfigList);
    }
  }
  // SRB2_lchan_config->choice.explicitValue.ul_SpecificParameters->logicalChannelGroup
  if (logicalChannelConfig!= NULL) {
    if (eNB_flag==0){
      LOG_I(MAC,"[CONFIG][UE %d] Applying RRC logicalChannelConfig from eNB%d\n",Mod_id,eNB_index);
      UE_mac_inst[Mod_id].logicalChannelConfig[logicalChannelIdentity]=logicalChannelConfig;
      UE_mac_inst[Mod_id].scheduling_info.Bj[logicalChannelIdentity]=0; // initilize the bucket for this lcid
      if (logicalChannelConfig->ul_SpecificParameters) {
	UE_mac_inst[Mod_id].scheduling_info.bucket_size[logicalChannelIdentity]=logicalChannelConfig->ul_SpecificParameters->prioritisedBitRate *
	  logicalChannelConfig->ul_SpecificParameters->bucketSizeDuration; // set the max bucket size
	UE_mac_inst[Mod_id].scheduling_info.LCGID[logicalChannelIdentity]=*logicalChannelConfig->ul_SpecificParameters->logicalChannelGroup;
	LOG_D(MAC,"[CONFIG][UE %d] LCID %d is attached to the LCGID %d\n",Mod_id,logicalChannelIdentity,*logicalChannelConfig->ul_SpecificParameters->logicalChannelGroup);
	} else {
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
      if (mac_MainConfig->ul_SCH_Config) {

	if (mac_MainConfig->ul_SCH_Config->periodicBSR_Timer)
	  UE_mac_inst[Mod_id].scheduling_info.periodicBSR_Timer = (u16) *mac_MainConfig->ul_SCH_Config->periodicBSR_Timer;
	else
	  UE_mac_inst[Mod_id].scheduling_info.periodicBSR_Timer = (u16) MAC_MainConfig__ul_SCH_Config__periodicBSR_Timer_infinity;
	
	if (mac_MainConfig->ul_SCH_Config->maxHARQ_Tx)
	  UE_mac_inst[Mod_id].scheduling_info.maxHARQ_Tx     = (u16) *mac_MainConfig->ul_SCH_Config->maxHARQ_Tx;
	else
	  UE_mac_inst[Mod_id].scheduling_info.maxHARQ_Tx     = (u16) MAC_MainConfig__ul_SCH_Config__maxHARQ_Tx_n5;
	if (mac_MainConfig->ul_SCH_Config->retxBSR_Timer)
	  UE_mac_inst[Mod_id].scheduling_info.retxBSR_Timer     = (u16) mac_MainConfig->ul_SCH_Config->retxBSR_Timer;
	else 
	  UE_mac_inst[Mod_id].scheduling_info.retxBSR_Timer     = (u16)MAC_MainConfig__ul_SCH_Config__retxBSR_Timer_sf2560;
      }
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
      LOG_D(MAC,"[UE %d] config PHR : periodic %d (SF) prohibit %d (SF)  pathlosschange %d (db) \n",
	    Mod_id, 
	    UE_mac_inst[Mod_id].scheduling_info.periodicPHR_SF,
	    UE_mac_inst[Mod_id].scheduling_info.prohibitPHR_SF,
	    UE_mac_inst[Mod_id].scheduling_info.PathlossChange_db);
    }
  }

  if (physicalConfigDedicated != NULL) {
    if (eNB_flag==1){
      mac_xface->phy_config_dedicated_eNB(Mod_id,find_UE_RNTI(Mod_id,UE_id),physicalConfigDedicated);
    }else{
      mac_xface->phy_config_dedicated_ue(Mod_id,eNB_index,physicalConfigDedicated);
      UE_mac_inst[Mod_id].physicalConfigDedicated=physicalConfigDedicated; // for SR proc
    }
  }

  if (eNB_flag == 0) {
    if (measObj!= NULL) 
      if (measObj[0]!= NULL){
	UE_mac_inst[Mod_id].n_adj_cells = measObj[0]->measObject.choice.measObjectEUTRA.cellsToAddModList->list.count;
	LOG_I(MAC,"Number of adjacent cells %d\n",UE_mac_inst[Mod_id].n_adj_cells);
	for (i=0;i<UE_mac_inst[Mod_id].n_adj_cells;i++) {
	  UE_mac_inst[Mod_id].adj_cell_id[i] = measObj[0]->measObject.choice.measObjectEUTRA.cellsToAddModList->list.array[i]->physCellId;
	  LOG_I(MAC,"Cell %d : Nid_cell %d\n",i,UE_mac_inst[Mod_id].adj_cell_id[i]);
	}
	mac_xface->phy_config_meas_ue(Mod_id,eNB_index,UE_mac_inst[Mod_id].n_adj_cells,UE_mac_inst[Mod_id].adj_cell_id);
      }
  }


  if (mbsfn_SubframeConfigList != NULL) {
    if (eNB_flag == 1) {
      for (i=0; i<mbsfn_SubframeConfigList->list.count; i++) {
	eNB_mac_inst[Mod_id].mbsfn_SubframeConfig[i] = mbsfn_SubframeConfigList->list.array[i];
	LOG_I(MAC, "[CONFIG] MBSFN_SubframeConfig[%d] pattern is  %ld\n", i, 
	      eNB_mac_inst[Mod_id].mbsfn_SubframeConfig[i]->subframeAllocation.choice.oneFrame.buf[0]); 
      }
#ifdef Rel10
      eNB_mac_inst[Mod_id].MBMS_flag = MBMS_Flag;
#endif
    }
    else { // UE
      for (i=0; i<mbsfn_SubframeConfigList->list.count; i++) {
	LOG_I(MAC, "[UE %d] Configuring MBSFN_SubframeConfig from received SIB2 \n", Mod_id); 
	UE_mac_inst[Mod_id].mbsfn_SubframeConfig[i] = mbsfn_SubframeConfigList->list.array[i];
	//	LOG_I("[UE %d] MBSFN_SubframeConfig[%d] pattern is  %ld\n", Mod_id, 
	//    UE_mac_inst[Mod_id].mbsfn_SubframeConfig[i]->subframeAllocation.choice.oneFrame.buf[0]); 
      }
    }
  }

#ifdef Rel10
  if (mbsfn_AreaInfoList != NULL) {
    if (eNB_flag == 1) {
     LOG_I(MAC, "[CONFIG] Number of MBSFN Area Info in the list %d\n", mbsfn_AreaInfoList->list.count);
      for (i =0; i< mbsfn_AreaInfoList->list.count; i++) {
	eNB_mac_inst[Mod_id].mbsfn_AreaInfo[i] = mbsfn_AreaInfoList->list.array[i];
	LOG_I(MAC, "[CONFIG] MBSFN_AreaInfo[%d]: MCCH Repetition Period = %ld\n", i, 
	      eNB_mac_inst[Mod_id].mbsfn_AreaInfo[i]->mcch_Config_r9.mcch_RepetitionPeriod_r9); 
	mac_xface->phy_config_sib13_eNB(Mod_id,i,eNB_mac_inst[Mod_id].mbsfn_AreaInfo[i]->mbsfn_AreaId_r9);
      }
    }
    else {  // UE
      LOG_I(MAC, "[UE %d] Configuring mbsfn_AreaInfo from received SIB13 \n", Mod_id);
      for (i =0; i< mbsfn_AreaInfoList->list.count; i++) {
	UE_mac_inst[Mod_id].mbsfn_AreaInfo[i] = mbsfn_AreaInfoList->list.array[i];
	LOG_I(MAC, "[UE %d] MBSFN_AreaInfo[%d]: MCCH Repetition Period = %ld\n",Mod_id, i, 
	      UE_mac_inst[Mod_id].mbsfn_AreaInfo[i]->mcch_Config_r9.mcch_RepetitionPeriod_r9); 
	mac_xface->phy_config_sib13_ue(Mod_id,eNB_index,i,UE_mac_inst[Mod_id].mbsfn_AreaInfo[i]->mbsfn_AreaId_r9);
      }
    }
  }

  
  if (pmch_InfoList != NULL) {
    if (eNB_flag == 1) {
      LOG_I(MAC, "[CONFIG] Number of PMCH in this MBSFN Area %d\n", pmch_InfoList->list.count);
      for (i =0; i< pmch_InfoList->list.count; i++) {
	eNB_mac_inst[Mod_id].pmch_Config[i] = &pmch_InfoList->list.array[i]->pmch_Config_r9;

	LOG_I(MAC, "[CONFIG] PMCH[%d]: This PMCH stop at subframe  %ldth\n", i, 
	      eNB_mac_inst[Mod_id].pmch_Config[i]->sf_AllocEnd_r9); 
	LOG_I(MAC, "[CONFIG] PMCH[%d]: mch_Scheduling_Period = %ld\n", i, 
	      eNB_mac_inst[Mod_id].pmch_Config[i]->mch_SchedulingPeriod_r9); 
	LOG_I(MAC, "[CONFIG] PMCH[%d]: dataMCS = %ld\n", i, 
	      eNB_mac_inst[Mod_id].pmch_Config[i]->dataMCS_r9); 

	// MBMS session info list in each MCH
	//	for (j=0;j< pmch_InfoList->list.array[i]->mbms_SessionInfoList_r9.list.count;j++) {
	  eNB_mac_inst[Mod_id].mbms_SessionList[i] = &pmch_InfoList->list.array[i]->mbms_SessionInfoList_r9;
	  LOG_I(MAC, "PMCH[%d] Number of session (MTCH) is: %d\n",i, eNB_mac_inst[Mod_id].mbms_SessionList[i]->list.count);
	  //	}
      }
    }
    else { // UE  
      LOG_I(MAC, "[UE %d] Configuring PMCH_config from MCCH MESSAGE \n",Mod_id);
      for (i =0; i< pmch_InfoList->list.count; i++) {
	UE_mac_inst[Mod_id].pmch_Config[i] = &pmch_InfoList->list.array[i]->pmch_Config_r9;
	LOG_I(MAC, "[UE %d] PMCH[%d]: MCH_Scheduling_Period = %ld\n", Mod_id, 
	      UE_mac_inst[Mod_id].pmch_Config[i]->mch_SchedulingPeriod_r9); 
      }
      UE_mac_inst[Mod_id].mcch_status = 1;
    }
  }
 
#endif
#ifdef CBA
  if (eNB_flag == 0){
    if (CBA_RNTI) {
      UE_mac_inst[Mod_id].CBA_RNTI[num_active_cba_groups] = CBA_RNTI;
      LOG_D(MAC,"[UE %d] configure the CBA group %d RNTI %x for\n", 
	    Mod_id, num_active_cba_groups, UE_mac_inst[Mod_id].CBA_RNTI[num_active_cba_groups]);
    }
  }else {
    if (CBA_RNTI) {
      for (i=0; i < num_active_cba_groups; i ++){
	eNB_mac_inst[Mod_id].CBA_RNTI[i] = CBA_RNTI + i;
	LOG_D(MAC,"[eNB %d] configure CBA groups %d with RNTI %x \n", Mod_id, i, eNB_mac_inst[Mod_id].CBA_RNTI[i]);
      }
    }
  }

#endif

  return(0);
}
