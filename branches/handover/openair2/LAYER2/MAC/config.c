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

int rrc_mac_config_req(u8 Mod_id,u8 eNB_flag,u8 UE_id,u8 eNB_index, 
		       RadioResourceConfigCommonSIB_t *radioResourceConfigCommon,
		       PhysicalConfigDedicated_t *physicalConfigDedicated,
		       MeasObjectToAddMod_t **measObj,
		       MAC_MainConfig_t *mac_MainConfig,
		       long logicalChannelIdentity,
		       LogicalChannelConfig_t *logicalChannelConfig,
		       MeasGapConfig_t *measGapConfig,
		       TDD_Config_t *tdd_Config,
		       MobilityControlInfo_t *mobilityControlInfo,
		       u8 *SIwindowsize,
		       u16 *SIperiod) {

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
      
      mac_xface->phy_config_sib2_eNB(Mod_id,radioResourceConfigCommon);
    }
    else {
      UE_mac_inst[Mod_id].radioResourceConfigCommon = radioResourceConfigCommon;
      mac_xface->phy_config_sib2_ue(Mod_id,eNB_index,radioResourceConfigCommon);
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

    if(mobilityControlInfo != NULL) {
    	if(mobilityControlInfo->radioResourceConfigCommon.rach_ConfigCommon) {
    		memcpy((void *)&UE_mac_inst[Mod_id].radioResourceConfigCommon->rach_ConfigCommon, (void *)mobilityControlInfo->radioResourceConfigCommon.rach_ConfigCommon,sizeof(RACH_ConfigCommon_t));
    	}

        memcpy((void *)&UE_mac_inst[Mod_id].radioResourceConfigCommon->prach_Config.prach_ConfigInfo, (void *)mobilityControlInfo->radioResourceConfigCommon.prach_Config.prach_ConfigInfo,sizeof(PRACH_ConfigInfo_t));
        UE_mac_inst[Mod_id].radioResourceConfigCommon->prach_Config.rootSequenceIndex = mobilityControlInfo->radioResourceConfigCommon.prach_Config.rootSequenceIndex;

    	if(mobilityControlInfo->radioResourceConfigCommon.pdsch_ConfigCommon) {
    		memcpy((void *)&UE_mac_inst[Mod_id].radioResourceConfigCommon->pdsch_ConfigCommon, (void *)mobilityControlInfo->radioResourceConfigCommon.pdsch_ConfigCommon,sizeof(PDSCH_ConfigCommon_t));
    	}

    	memcpy((void *)&UE_mac_inst[Mod_id].radioResourceConfigCommon->pusch_ConfigCommon, (void *)&mobilityControlInfo->radioResourceConfigCommon.pusch_ConfigCommon,sizeof(PUSCH_ConfigCommon_t));

    	if(mobilityControlInfo->radioResourceConfigCommon.phich_Config) {
    		//fill this when HICH is implemented..comes from the MIB
    	}
    	if(mobilityControlInfo->radioResourceConfigCommon.pucch_ConfigCommon) {
    		memcpy((void *)&UE_mac_inst[Mod_id].radioResourceConfigCommon->pucch_ConfigCommon, (void *)mobilityControlInfo->radioResourceConfigCommon.pucch_ConfigCommon,sizeof(PUCCH_ConfigCommon_t));
    	}
    	if(mobilityControlInfo->radioResourceConfigCommon.soundingRS_UL_ConfigCommon) {
    		memcpy((void *)&UE_mac_inst[Mod_id].radioResourceConfigCommon->soundingRS_UL_ConfigCommon, (void *)mobilityControlInfo->radioResourceConfigCommon.soundingRS_UL_ConfigCommon,sizeof(SoundingRS_UL_ConfigCommon_t));
    	}
    	if(mobilityControlInfo->radioResourceConfigCommon.uplinkPowerControlCommon) {
    		memcpy((void *)&UE_mac_inst[Mod_id].radioResourceConfigCommon->uplinkPowerControlCommon, (void *)mobilityControlInfo->radioResourceConfigCommon.uplinkPowerControlCommon,sizeof(UplinkPowerControlCommon_t));
    	}
    	//configure antennaInfoCommon somewhere here..
    	if(mobilityControlInfo->radioResourceConfigCommon.p_Max) {
    		//to be configured
    	}
    	if(mobilityControlInfo->radioResourceConfigCommon.tdd_Config) {
    		//to be configured
    	}
    	if(mobilityControlInfo->radioResourceConfigCommon.ul_CyclicPrefixLength) {
	  memcpy((void *)&UE_mac_inst[Mod_id].radioResourceConfigCommon->ul_CyclicPrefixLength, (void *)mobilityControlInfo->radioResourceConfigCommon.ul_CyclicPrefixLength,sizeof(UL_CyclicPrefixLength_t));
    	}
	
	UE_mac_inst[Mod_id].crnti = 0;
	for (i=0;i<15;i++) {
	  UE_mac_inst[Mod_id].crnti |= (mobilityControlInfo->newUE_Identity.buf[i]<<i);
	}
	UE_mac_inst[Mod_id].rach_ConfigDedicated = malloc(sizeof(*mobilityControlInfo->rach_ConfigDedicated));
	if (mobilityControlInfo->rach_ConfigDedicated)
	  memcpy((void*)UE_mac_inst[Mod_id].rach_ConfigDedicated,
		 (void*)mobilityControlInfo->rach_ConfigDedicated,
		 sizeof(*mobilityControlInfo->rach_ConfigDedicated));
	
    	mac_xface->phy_config_afterHO_ue(Mod_id,eNB_index,mobilityControlInfo);
    }


    /*
    if (quantityConfig != NULL) {
    	if (quantityConfig[0] != NULL) {
    		UE_mac_inst[Mod_id].quantityConfig = quantityConfig[0];
    		LOG_I(MAC,"UE %d configured filterCoeff.",UE_mac_inst[Mod_id].crnti);
    		mac_xface->phy_config_meas_ue
    	}
    }
    */
  }
  else {  // This is to configure eNB PHY and MAC for new UE

    // configure PHY as above (without the cell specific stuff)
    // configure MAC as above

    // save the rnti etc.



  }
  return(0);
}
