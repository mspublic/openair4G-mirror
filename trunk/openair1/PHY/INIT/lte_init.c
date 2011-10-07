#ifdef CBMIMO1
#include "ARCH/COMMON/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/from_grlib_softconfig.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_device.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_pci.h"
//#include "pci_commands.h"
#endif //CBMIMO1
#include "defs.h"
#include "SCHED/defs.h"
#include "PHY/extern.h"
#include "SIMULATION/TOOLS/defs.h"
#include "RadioResourceConfigCommonSIB.h"
#include "RadioResourceConfigDedicated.h"
#include "TDD-Config.h"

#define DEBUG_PHY

/*
void copy_lte_parms_to_phy_framing(LTE_DL_FRAME_PARMS *frame_parms, PHY_FRAMING *phy_framing) {

  //phy_framing->fc_khz;
  //phy_framing->fs_khz;
  msg("openair_lte: Copying to PHY Framing\n");
  phy_framing->Nsymb = frame_parms->symbols_per_tti*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME;
  msg("openair_lte: Nsymb %d\n",phy_framing->Nsymb);
  phy_framing->Nd = frame_parms->ofdm_symbol_size;     
  msg("openair_lte: Nd %d\n",phy_framing->Nd);

  phy_framing->Nc = frame_parms->nb_prefix_samples;    
  phy_framing->Nz = frame_parms->ofdm_symbol_size - frame_parms->N_RB_DL*12;    
  phy_framing->Nf = frame_parms->N_RB_DL;    
  phy_framing->Extension_type = CYCLIC_PREFIX;
  phy_framing->log2Nd = frame_parms->log2_symbol_size;
} 
*/

void phy_config_mib(LTE_DL_FRAME_PARMS *lte_frame_parms,
		    u8 N_RB_DL,
		    u8 Nid_cell,
		    u8 Ncp,
		    u8 frame_type,
		    u8 p_eNB,
		    PHICH_CONFIG_COMMON *phich_config) {

  lte_frame_parms->N_RB_DL                            = N_RB_DL;
  lte_frame_parms->Nid_cell                           = Nid_cell;
  lte_frame_parms->nushift                            = Nid_cell%6;
  lte_frame_parms->Ncp                                = Ncp;
  lte_frame_parms->frame_type                         = frame_type;
  lte_frame_parms->nb_antennas_tx                     = p_eNB;
  lte_frame_parms->phich_config_common.phich_resource = phich_config->phich_resource;
  lte_frame_parms->phich_config_common.phich_duration = phich_config->phich_duration;
}

void phy_config_sib1_eNB(u8 Mod_id,
			 TDD_Config_t *tdd_Config,
			 u8 SIwindowsize,
			 u16 SIPeriod) {
   
  LTE_DL_FRAME_PARMS *lte_frame_parms = &PHY_vars_eNB_g[Mod_id]->lte_frame_parms;

  lte_frame_parms->tdd_config    = tdd_Config->subframeAssignment;
  lte_frame_parms->tdd_config_S  = tdd_Config->specialSubframePatterns;  
  lte_frame_parms->SIwindowsize  = SIwindowsize;
  lte_frame_parms->SIPeriod      = SIPeriod;
}

void phy_config_sib1_ue(u8 Mod_id,u8 CH_index,
			 TDD_Config_t *tdd_Config,
			 u8 SIwindowsize,
			 u16 SIperiod) {

  LTE_DL_FRAME_PARMS *lte_frame_parms = &PHY_vars_UE_g[Mod_id]->lte_frame_parms;

  lte_frame_parms->tdd_config    = tdd_Config->subframeAssignment;
  lte_frame_parms->tdd_config_S  = tdd_Config->specialSubframePatterns;  
  lte_frame_parms->SIwindowsize  = SIwindowsize;  
  lte_frame_parms->SIPeriod      = SIperiod;
}

void phy_config_sib2_eNB(u8 Mod_id,
			 RadioResourceConfigCommonSIB_t *radioResourceConfigCommon) {

  LTE_DL_FRAME_PARMS *lte_frame_parms = &PHY_vars_eNB_g[Mod_id]->lte_frame_parms;

  msg("[PHY][eNB%d] Frame %d: Applying radioResourceConfigCommon\n",Mod_id,mac_xface->frame);

  lte_frame_parms->prach_config_common.rootSequenceIndex                           =radioResourceConfigCommon->prach_Config.rootSequenceIndex;
  lte_frame_parms->prach_config_common.prach_Config_enabled=1;
  lte_frame_parms->prach_config_common.prach_ConfigInfo.prach_ConfigIndex          =radioResourceConfigCommon->prach_Config.prach_ConfigInfo.prach_ConfigIndex;
  lte_frame_parms->prach_config_common.prach_ConfigInfo.highSpeedFlag              =radioResourceConfigCommon->prach_Config.prach_ConfigInfo.highSpeedFlag;
  lte_frame_parms->prach_config_common.prach_ConfigInfo.zeroCorrelationZoneConfig  =radioResourceConfigCommon->prach_Config.prach_ConfigInfo.zeroCorrelationZoneConfig;
  lte_frame_parms->prach_config_common.prach_ConfigInfo.prach_FreqOffset           =radioResourceConfigCommon->prach_Config.prach_ConfigInfo.prach_FreqOffset;
  


  lte_frame_parms->pucch_config_common.deltaPUCCH_Shift = 1+radioResourceConfigCommon->pucch_ConfigCommon.deltaPUCCH_Shift;
  lte_frame_parms->pucch_config_common.nRB_CQI          = radioResourceConfigCommon->pucch_ConfigCommon.nRB_CQI;
  lte_frame_parms->pucch_config_common.nCS_AN           = radioResourceConfigCommon->pucch_ConfigCommon.nCS_AN;
  lte_frame_parms->pucch_config_common.n1PUCCH_AN       = radioResourceConfigCommon->pucch_ConfigCommon.n1PUCCH_AN;
  


  lte_frame_parms->pdsch_config_common.referenceSignalPower = radioResourceConfigCommon->pdsch_ConfigCommon.referenceSignalPower;
  lte_frame_parms->pdsch_config_common.p_b                  = radioResourceConfigCommon->pdsch_ConfigCommon.p_b;
  

  lte_frame_parms->pusch_config_common.n_SB                                         = radioResourceConfigCommon->pusch_ConfigCommon.pusch_ConfigBasic.n_SB;
  msg("pusch_config_common.n_SB = %d\n",lte_frame_parms->pusch_config_common.n_SB );

  lte_frame_parms->pusch_config_common.hoppingMode                                  = radioResourceConfigCommon->pusch_ConfigCommon.pusch_ConfigBasic.hoppingMode;
  msg("pusch_config_common.hoppingMode = %d\n",lte_frame_parms->pusch_config_common.hoppingMode);

  lte_frame_parms->pusch_config_common.pusch_HoppingOffset                          = radioResourceConfigCommon->pusch_ConfigCommon.pusch_ConfigBasic.pusch_HoppingOffset;
  msg("pusch_config_common.pusch_HoppingOffset = %d\n",lte_frame_parms->pusch_config_common.pusch_HoppingOffset);

  lte_frame_parms->pusch_config_common.enable64QAM                                  = radioResourceConfigCommon->pusch_ConfigCommon.pusch_ConfigBasic.enable64QAM;
  msg("pusch_config_common.enable64QAM = %d\n",lte_frame_parms->pusch_config_common.enable64QAM );

  lte_frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.groupHoppingEnabled    = radioResourceConfigCommon->pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupHoppingEnabled;
  msg("pusch_config_common.groupHoppingEnabled = %d\n",lte_frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.groupHoppingEnabled);

  lte_frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH   = radioResourceConfigCommon->pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH;
  msg("pusch_config_common.groupAssignmentPUSCH = %d\n",lte_frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH);

  lte_frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled = radioResourceConfigCommon->pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled;
  msg("pusch_config_common.sequenceHoppingEnabled = %d\n",lte_frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled);

  lte_frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.cyclicShift            = radioResourceConfigCommon->pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.cyclicShift;
  msg("pusch_config_common.enable64QAM = %d\n",lte_frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.cyclicShift);  
  
  lte_frame_parms->soundingrs_ul_config_common.enabled_flag                        = 0;

  if (radioResourceConfigCommon->soundingRS_UL_ConfigCommon.present==SoundingRS_UL_ConfigCommon_PR_setup) {
    lte_frame_parms->soundingrs_ul_config_common.enabled_flag                        = 1;
    lte_frame_parms->soundingrs_ul_config_common.srs_BandwidthConfig                 = radioResourceConfigCommon->soundingRS_UL_ConfigCommon.choice.setup.srs_BandwidthConfig;
    lte_frame_parms->soundingrs_ul_config_common.srs_SubframeConfig                  = radioResourceConfigCommon->soundingRS_UL_ConfigCommon.choice.setup.srs_SubframeConfig;
    lte_frame_parms->soundingrs_ul_config_common.ackNackSRS_SimultaneousTransmission = radioResourceConfigCommon->soundingRS_UL_ConfigCommon.choice.setup.ackNackSRS_SimultaneousTransmission;
    if (radioResourceConfigCommon->soundingRS_UL_ConfigCommon.choice.setup.srs_MaxUpPts)
      lte_frame_parms->soundingrs_ul_config_common.srs_MaxUpPts                      = 1;
    else
      lte_frame_parms->soundingrs_ul_config_common.srs_MaxUpPts                      = 0;
  }


  
  lte_frame_parms->ul_power_control_config_common.p0_NominalPUSCH   = radioResourceConfigCommon->uplinkPowerControlCommon.p0_NominalPUSCH;
  lte_frame_parms->ul_power_control_config_common.alpha             = radioResourceConfigCommon->uplinkPowerControlCommon.alpha;
  lte_frame_parms->ul_power_control_config_common.p0_NominalPUCCH   = radioResourceConfigCommon->uplinkPowerControlCommon.p0_NominalPUCCH;
  lte_frame_parms->ul_power_control_config_common.deltaPreambleMsg3 = radioResourceConfigCommon->uplinkPowerControlCommon.deltaPreambleMsg3;
  
  lte_frame_parms->maxHARQ_Msg3Tx = radioResourceConfigCommon->rach_ConfigCommon.maxHARQ_Msg3Tx;


  // Now configure some of the Physical Channels

  // PUCCH

  init_ncs_cell(lte_frame_parms,PHY_vars_eNB_g[Mod_id]->ncs_cell);


}

void phy_config_sib2_ue(u8 Mod_id,u8 CH_index,
			RadioResourceConfigCommonSIB_t *radioResourceConfigCommon) {

  LTE_DL_FRAME_PARMS *lte_frame_parms = &PHY_vars_UE_g[Mod_id]->lte_frame_parms;

  msg("[PHY][UE%d] Frame %d: Applying radioResourceConfigCommon from eNB%d\n",Mod_id,mac_xface->frame,CH_index);

  lte_frame_parms->prach_config_common.rootSequenceIndex                           =radioResourceConfigCommon->prach_Config.rootSequenceIndex;

  lte_frame_parms->prach_config_common.prach_Config_enabled=1;
  lte_frame_parms->prach_config_common.prach_ConfigInfo.prach_ConfigIndex          =radioResourceConfigCommon->prach_Config.prach_ConfigInfo.prach_ConfigIndex;
  lte_frame_parms->prach_config_common.prach_ConfigInfo.highSpeedFlag              =radioResourceConfigCommon->prach_Config.prach_ConfigInfo.highSpeedFlag;
  lte_frame_parms->prach_config_common.prach_ConfigInfo.zeroCorrelationZoneConfig  =radioResourceConfigCommon->prach_Config.prach_ConfigInfo.zeroCorrelationZoneConfig;
  lte_frame_parms->prach_config_common.prach_ConfigInfo.prach_FreqOffset           =radioResourceConfigCommon->prach_Config.prach_ConfigInfo.prach_FreqOffset;
  


  lte_frame_parms->pucch_config_common.deltaPUCCH_Shift = 1+radioResourceConfigCommon->pucch_ConfigCommon.deltaPUCCH_Shift;
  lte_frame_parms->pucch_config_common.nRB_CQI          = radioResourceConfigCommon->pucch_ConfigCommon.nRB_CQI;
  lte_frame_parms->pucch_config_common.nCS_AN           = radioResourceConfigCommon->pucch_ConfigCommon.nCS_AN;
  lte_frame_parms->pucch_config_common.n1PUCCH_AN       = radioResourceConfigCommon->pucch_ConfigCommon.n1PUCCH_AN;



  lte_frame_parms->pdsch_config_common.referenceSignalPower = radioResourceConfigCommon->pdsch_ConfigCommon.referenceSignalPower;
  lte_frame_parms->pdsch_config_common.p_b                  = radioResourceConfigCommon->pdsch_ConfigCommon.p_b;
  

  lte_frame_parms->pusch_config_common.n_SB                                         = radioResourceConfigCommon->pusch_ConfigCommon.pusch_ConfigBasic.n_SB;
  lte_frame_parms->pusch_config_common.hoppingMode                                  = radioResourceConfigCommon->pusch_ConfigCommon.pusch_ConfigBasic.hoppingMode;
  lte_frame_parms->pusch_config_common.pusch_HoppingOffset                          = radioResourceConfigCommon->pusch_ConfigCommon.pusch_ConfigBasic.pusch_HoppingOffset;
  lte_frame_parms->pusch_config_common.enable64QAM                                  = radioResourceConfigCommon->pusch_ConfigCommon.pusch_ConfigBasic.enable64QAM;
  lte_frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.groupHoppingEnabled    = radioResourceConfigCommon->pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupHoppingEnabled;
  lte_frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH   = radioResourceConfigCommon->pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH;
  lte_frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled = radioResourceConfigCommon->pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled;
  lte_frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.cyclicShift            = radioResourceConfigCommon->pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.cyclicShift;
  
  
  lte_frame_parms->soundingrs_ul_config_common.enabled_flag                        = 0;
  if (radioResourceConfigCommon->soundingRS_UL_ConfigCommon.present==SoundingRS_UL_ConfigCommon_PR_setup) {
    lte_frame_parms->soundingrs_ul_config_common.enabled_flag                        = 1;
    lte_frame_parms->soundingrs_ul_config_common.srs_BandwidthConfig                 = radioResourceConfigCommon->soundingRS_UL_ConfigCommon.choice.setup.srs_BandwidthConfig;
    lte_frame_parms->soundingrs_ul_config_common.srs_SubframeConfig                  = radioResourceConfigCommon->soundingRS_UL_ConfigCommon.choice.setup.srs_SubframeConfig;
    lte_frame_parms->soundingrs_ul_config_common.ackNackSRS_SimultaneousTransmission = radioResourceConfigCommon->soundingRS_UL_ConfigCommon.choice.setup.ackNackSRS_SimultaneousTransmission;
    if (radioResourceConfigCommon->soundingRS_UL_ConfigCommon.choice.setup.srs_MaxUpPts)
      lte_frame_parms->soundingrs_ul_config_common.srs_MaxUpPts                      = 1;
    else
      lte_frame_parms->soundingrs_ul_config_common.srs_MaxUpPts                      = 0;
  }
  


  lte_frame_parms->ul_power_control_config_common.p0_NominalPUSCH   = radioResourceConfigCommon->uplinkPowerControlCommon.p0_NominalPUSCH;
  lte_frame_parms->ul_power_control_config_common.alpha             = radioResourceConfigCommon->uplinkPowerControlCommon.alpha;
  lte_frame_parms->ul_power_control_config_common.p0_NominalPUCCH   = radioResourceConfigCommon->uplinkPowerControlCommon.p0_NominalPUCCH;
  lte_frame_parms->ul_power_control_config_common.deltaPreambleMsg3 = radioResourceConfigCommon->uplinkPowerControlCommon.deltaPreambleMsg3;
  

  lte_frame_parms->maxHARQ_Msg3Tx = radioResourceConfigCommon->rach_ConfigCommon.maxHARQ_Msg3Tx;
  
  // Now configure some of the Physical Channels

  // PUCCH
  init_ncs_cell(lte_frame_parms,PHY_vars_UE_g[Mod_id]->ncs_cell);

}


void phy_config_dedicated_eNB(u8 Mod_id,u16 rnti,
			      struct PhysicalConfigDedicated *physicalConfigDedicated) {

  PHY_VARS_eNB *phy_vars_eNB = PHY_vars_eNB_g[Mod_id];
  u8 UE_id = find_ue(rnti,phy_vars_eNB);
  

  
  if (physicalConfigDedicated) {
    msg("[PHY][eNB %d] Frame %d: Sent physicalConfigDedicated for UE %d (%x)\n",Mod_id, mac_xface->frame,UE_id,rnti);
    msg("------------------------------------------------------------------------\n");
    
    if (physicalConfigDedicated->pdsch_ConfigDedicated) {
      phy_vars_eNB->pdsch_config_dedicated[UE_id].p_a=physicalConfigDedicated->pdsch_ConfigDedicated->p_a;
      msg("pdsch_config_dedicated.p_a %d\n",phy_vars_eNB->pdsch_config_dedicated[UE_id].p_a);
      msg("\n");
    }
    
    if (physicalConfigDedicated->pucch_ConfigDedicated) {
      if (physicalConfigDedicated->pucch_ConfigDedicated->ackNackRepetition.present==PUCCH_ConfigDedicated__ackNackRepetition_PR_release)
	phy_vars_eNB->pucch_config_dedicated[UE_id].ackNackRepetition=0;
      else {
	phy_vars_eNB->pucch_config_dedicated[UE_id].ackNackRepetition=1;
      }

      if (physicalConfigDedicated->pucch_ConfigDedicated->tdd_AckNackFeedbackMode)
	phy_vars_eNB->pucch_config_dedicated[UE_id].tdd_AckNackFeedbackMode = *physicalConfigDedicated->pucch_ConfigDedicated->tdd_AckNackFeedbackMode;
      else
	phy_vars_eNB->pucch_config_dedicated[UE_id].tdd_AckNackFeedbackMode = bundling;

      if ( phy_vars_eNB->pucch_config_dedicated[UE_id].tdd_AckNackFeedbackMode == multiplexing)
	msg("pucch_config_dedicated.tdd_AckNackFeedbackMode = multiplexing\n");
      else
	msg("pucch_config_dedicated.tdd_AckNackFeedbackMode = bundling\n");
 
    }
    
    if (physicalConfigDedicated->pusch_ConfigDedicated) {
      phy_vars_eNB->pusch_config_dedicated[UE_id].betaOffset_ACK_Index = physicalConfigDedicated->pusch_ConfigDedicated->betaOffset_ACK_Index;
      phy_vars_eNB->pusch_config_dedicated[UE_id].betaOffset_RI_Index = physicalConfigDedicated->pusch_ConfigDedicated->betaOffset_RI_Index;
      phy_vars_eNB->pusch_config_dedicated[UE_id].betaOffset_CQI_Index = physicalConfigDedicated->pusch_ConfigDedicated->betaOffset_CQI_Index;
      
      msg("pusch_config_dedicated.betaOffset_ACK_Index %d\n",phy_vars_eNB->pusch_config_dedicated[UE_id].betaOffset_ACK_Index);
      msg("pusch_config_dedicated.betaOffset_RI_Index %d\n",phy_vars_eNB->pusch_config_dedicated[UE_id].betaOffset_RI_Index);
      msg("pusch_config_dedicated.betaOffset_CQI_Index %d\n",phy_vars_eNB->pusch_config_dedicated[UE_id].betaOffset_CQI_Index);
      msg("\n");
      
      
    }
    if (physicalConfigDedicated->uplinkPowerControlDedicated) {
      
      phy_vars_eNB->ul_power_control_dedicated[UE_id].p0_UE_PUSCH = physicalConfigDedicated->uplinkPowerControlDedicated->p0_UE_PUSCH;
      phy_vars_eNB->ul_power_control_dedicated[UE_id].deltaMCS_Enabled= physicalConfigDedicated->uplinkPowerControlDedicated->deltaMCS_Enabled;
      phy_vars_eNB->ul_power_control_dedicated[UE_id].accumulationEnabled= physicalConfigDedicated->uplinkPowerControlDedicated->accumulationEnabled;
      phy_vars_eNB->ul_power_control_dedicated[UE_id].p0_UE_PUCCH= physicalConfigDedicated->uplinkPowerControlDedicated->p0_UE_PUCCH;
      phy_vars_eNB->ul_power_control_dedicated[UE_id].pSRS_Offset= physicalConfigDedicated->uplinkPowerControlDedicated->pSRS_Offset;
      phy_vars_eNB->ul_power_control_dedicated[UE_id].filterCoefficient= *physicalConfigDedicated->uplinkPowerControlDedicated->filterCoefficient;
      msg("ul_power_control_dedicated.p0_UE_PUSCH %d\n",phy_vars_eNB->ul_power_control_dedicated[UE_id].p0_UE_PUSCH);
      msg("ul_power_control_dedicated.deltaMCS_Enabled %d\n",phy_vars_eNB->ul_power_control_dedicated[UE_id].deltaMCS_Enabled);
      msg("ul_power_control_dedicated.accumulationEnabled %d\n",phy_vars_eNB->ul_power_control_dedicated[UE_id].accumulationEnabled);
      msg("ul_power_control_dedicated.p0_UE_PUCCH %d\n",phy_vars_eNB->ul_power_control_dedicated[UE_id].p0_UE_PUCCH);
      msg("ul_power_control_dedicated.pSRS_Offset %d\n",phy_vars_eNB->ul_power_control_dedicated[UE_id].pSRS_Offset);
      msg("ul_power_control_dedicated.filterCoefficient %d\n",phy_vars_eNB->ul_power_control_dedicated[UE_id].filterCoefficient);
      msg("\n");
    }
    if (physicalConfigDedicated->antennaInfo) {
      phy_vars_eNB->transmission_mode[UE_id] = 1+(physicalConfigDedicated->antennaInfo->choice.explicitValue.transmissionMode);
      msg("Transmission Mode %d\n",phy_vars_eNB->transmission_mode[UE_id]);
      msg("\n");
    }
    
    if (physicalConfigDedicated->schedulingRequestConfig) {
      if (physicalConfigDedicated->schedulingRequestConfig->present == SchedulingRequestConfig_PR_setup) {
	phy_vars_eNB->scheduling_request_config[UE_id].sr_PUCCH_ResourceIndex = physicalConfigDedicated->schedulingRequestConfig->choice.setup.sr_PUCCH_ResourceIndex;
	phy_vars_eNB->scheduling_request_config[UE_id].sr_ConfigIndex=physicalConfigDedicated->schedulingRequestConfig->choice.setup.sr_ConfigIndex;  
	phy_vars_eNB->scheduling_request_config[UE_id].dsr_TransMax=physicalConfigDedicated->schedulingRequestConfig->choice.setup.dsr_TransMax;
	
	msg("scheduling_request_config.sr_PUCCH_ResourceIndex %d\n",phy_vars_eNB->scheduling_request_config[UE_id].sr_PUCCH_ResourceIndex);
	msg("scheduling_request_config.sr_ConfigIndex %d\n",phy_vars_eNB->scheduling_request_config[UE_id].sr_ConfigIndex);  
	msg("scheduling_request_config.dsr_TransMax %d\n",phy_vars_eNB->scheduling_request_config[UE_id].dsr_TransMax);
      }
      msg("------------------------------------------------------------\n");
      
    }
    
  }
  else {
    msg("[PHY][eNB %d] Frame %d: Received NULL radioResourceConfigDedicated from eNB %d\n",Mod_id, mac_xface->frame,UE_id);
    return;
  }
  
  
}

void phy_config_dedicated_ue(u8 Mod_id,u8 CH_index,
			     struct PhysicalConfigDedicated *physicalConfigDedicated ) {

  PHY_VARS_UE *phy_vars_ue = PHY_vars_UE_g[Mod_id];

  

    
    if (physicalConfigDedicated) {
      msg("[PHY][UE %d] Frame %d: Received physicalConfigDedicated from eNB %d\n",Mod_id, mac_xface->frame,CH_index);
      msg("------------------------------------------------------------------------\n");

      if (physicalConfigDedicated->pdsch_ConfigDedicated) {
	phy_vars_ue->pdsch_config_dedicated[CH_index].p_a=physicalConfigDedicated->pdsch_ConfigDedicated->p_a;
	msg("pdsch_config_dedicated.p_a %d\n",phy_vars_ue->pdsch_config_dedicated[CH_index].p_a);
	msg("\n");
      }

      if (physicalConfigDedicated->pucch_ConfigDedicated) {
	if (physicalConfigDedicated->pucch_ConfigDedicated->ackNackRepetition.present==PUCCH_ConfigDedicated__ackNackRepetition_PR_release)
	  phy_vars_ue->pucch_config_dedicated[CH_index].ackNackRepetition=0;
	else {
	  phy_vars_ue->pucch_config_dedicated[CH_index].ackNackRepetition=1;
	}
	if (physicalConfigDedicated->pucch_ConfigDedicated->tdd_AckNackFeedbackMode)
	  phy_vars_ue->pucch_config_dedicated[CH_index].tdd_AckNackFeedbackMode = *physicalConfigDedicated->pucch_ConfigDedicated->tdd_AckNackFeedbackMode;
	else
	  phy_vars_ue->pucch_config_dedicated[CH_index].tdd_AckNackFeedbackMode = bundling;

	if ( phy_vars_ue->pucch_config_dedicated[CH_index].tdd_AckNackFeedbackMode == multiplexing)
	  msg("pucch_config_dedicated.tdd_AckNackFeedbackMode = multiplexing\n");
	else
	  msg("pucch_config_dedicated.tdd_AckNackFeedbackMode = bundling\n");
      }

      if (physicalConfigDedicated->pusch_ConfigDedicated) {
	phy_vars_ue->pusch_config_dedicated[CH_index].betaOffset_ACK_Index = physicalConfigDedicated->pusch_ConfigDedicated->betaOffset_ACK_Index;
	phy_vars_ue->pusch_config_dedicated[CH_index].betaOffset_RI_Index = physicalConfigDedicated->pusch_ConfigDedicated->betaOffset_RI_Index;
	phy_vars_ue->pusch_config_dedicated[CH_index].betaOffset_CQI_Index = physicalConfigDedicated->pusch_ConfigDedicated->betaOffset_CQI_Index;


	msg("pusch_config_dedicated.betaOffset_ACK_Index %d\n",phy_vars_ue->pusch_config_dedicated[CH_index].betaOffset_ACK_Index);
	msg("pusch_config_dedicated.betaOffset_RI_Index %d\n",phy_vars_ue->pusch_config_dedicated[CH_index].betaOffset_RI_Index);
	msg("pusch_config_dedicated.betaOffset_CQI_Index %d\n",phy_vars_ue->pusch_config_dedicated[CH_index].betaOffset_CQI_Index);
	msg("\n");
	
	
      }
      if (physicalConfigDedicated->uplinkPowerControlDedicated) {
	
	phy_vars_ue->ul_power_control_dedicated[CH_index].p0_UE_PUSCH = physicalConfigDedicated->uplinkPowerControlDedicated->p0_UE_PUSCH;
	phy_vars_ue->ul_power_control_dedicated[CH_index].deltaMCS_Enabled= physicalConfigDedicated->uplinkPowerControlDedicated->deltaMCS_Enabled;
	phy_vars_ue->ul_power_control_dedicated[CH_index].accumulationEnabled= physicalConfigDedicated->uplinkPowerControlDedicated->accumulationEnabled;
	phy_vars_ue->ul_power_control_dedicated[CH_index].p0_UE_PUCCH= physicalConfigDedicated->uplinkPowerControlDedicated->p0_UE_PUCCH;
	phy_vars_ue->ul_power_control_dedicated[CH_index].pSRS_Offset= physicalConfigDedicated->uplinkPowerControlDedicated->pSRS_Offset;
	phy_vars_ue->ul_power_control_dedicated[CH_index].filterCoefficient= *physicalConfigDedicated->uplinkPowerControlDedicated->filterCoefficient;
	msg("ul_power_control_dedicated.p0_UE_PUSCH %d\n",phy_vars_ue->ul_power_control_dedicated[CH_index].p0_UE_PUSCH);
	msg("ul_power_control_dedicated.deltaMCS_Enabled %d\n",phy_vars_ue->ul_power_control_dedicated[CH_index].deltaMCS_Enabled);
	msg("ul_power_control_dedicated.accumulationEnabled %d\n",phy_vars_ue->ul_power_control_dedicated[CH_index].accumulationEnabled);
	msg("ul_power_control_dedicated.p0_UE_PUCCH %d\n",phy_vars_ue->ul_power_control_dedicated[CH_index].p0_UE_PUCCH);
	msg("ul_power_control_dedicated.pSRS_Offset %d\n",phy_vars_ue->ul_power_control_dedicated[CH_index].pSRS_Offset);
	msg("ul_power_control_dedicated.filterCoefficient %d\n",phy_vars_ue->ul_power_control_dedicated[CH_index].filterCoefficient);
	msg("\n");
      }
      if (physicalConfigDedicated->antennaInfo) {
	phy_vars_ue->transmission_mode[CH_index] = 1+(physicalConfigDedicated->antennaInfo->choice.explicitValue.transmissionMode);
	msg("Transmission Mode %d\n",phy_vars_ue->transmission_mode[CH_index]);
	msg("\n");
      }

      if (physicalConfigDedicated->schedulingRequestConfig) {
	if (physicalConfigDedicated->schedulingRequestConfig->present == SchedulingRequestConfig_PR_setup) {
	  phy_vars_ue->scheduling_request_config[CH_index].sr_PUCCH_ResourceIndex = physicalConfigDedicated->schedulingRequestConfig->choice.setup.sr_PUCCH_ResourceIndex;
	  phy_vars_ue->scheduling_request_config[CH_index].sr_ConfigIndex=physicalConfigDedicated->schedulingRequestConfig->choice.setup.sr_ConfigIndex;  
	  phy_vars_ue->scheduling_request_config[CH_index].dsr_TransMax=physicalConfigDedicated->schedulingRequestConfig->choice.setup.dsr_TransMax;

	  msg("scheduling_request_config.sr_PUCCH_ResourceIndex %d\n",phy_vars_ue->scheduling_request_config[CH_index].sr_PUCCH_ResourceIndex);
	  msg("scheduling_request_config.sr_ConfigIndex %d\n",phy_vars_ue->scheduling_request_config[CH_index].sr_ConfigIndex);  
	  msg("scheduling_request_config.dsr_TransMax %d\n",phy_vars_ue->scheduling_request_config[CH_index].dsr_TransMax);
	}
	msg("------------------------------------------------------------\n");

      }

    }
    else {
      msg("[PHY][UE %d] Frame %d: Received NULL radioResourceConfigDedicated from eNB %d\n",Mod_id, mac_xface->frame,CH_index);
      return;
    }
    
}


void phy_init_lte_top(LTE_DL_FRAME_PARMS *lte_frame_parms) {

  crcTableInit();
  
  ccodedot11_init();
  ccodedot11_init_inv();

  ccodelte_init();
  ccodelte_init_inv();

#ifndef EXPRESSMIMO_TARGET
  phy_generate_viterbi_tables();
  phy_generate_viterbi_tables_lte();
#endif //EXPRESSMIMO_TARGET



#ifdef USER_MODE
  lte_sync_time_init(lte_frame_parms);
#else
  // lte_sync_time_init(lte_frame_parms) has to be called from the real-time thread, since it uses SSE instructions.
#endif

  generate_ul_ref_sigs();
  generate_ul_ref_sigs_rx();

  generate_64qam_table();
  generate_16qam_table();
  generate_RIV_tables();
  

  //set_taus_seed(1328);
  
}

int phy_init_lte_ue(LTE_DL_FRAME_PARMS *frame_parms,
		    LTE_UE_COMMON *ue_common_vars,
		    LTE_UE_DLSCH **ue_dlsch_vars,
		    LTE_UE_DLSCH **ue_dlsch_vars_cntl,
		    LTE_UE_DLSCH **ue_dlsch_vars_ra,
		    LTE_UE_PBCH **ue_pbch_vars,
		    LTE_UE_PDCCH **ue_pdcch_vars,
		    PHY_VARS_UE *phy_vars_ue,
		    u8 abstraction_flag) {

  int i,j;
  unsigned char eNB_id;

  msg("Initializing UE vars (abstraction %d) for eNB TXant %d, UE RXant %d\n",abstraction_flag,frame_parms->nb_antennas_tx,frame_parms->nb_antennas_rx);
  if (abstraction_flag == 0) {

    ue_common_vars->txdata = (int **)malloc16(frame_parms->nb_antennas_tx*sizeof(int*));
    for (i=0; i<frame_parms->nb_antennas_tx; i++) {
#ifdef USER_MODE
      ue_common_vars->txdata[i] = (int *)malloc16(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(int));
      bzero(ue_common_vars->txdata[i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(int));
#else //USER_MODE
#ifdef IFFT_FPGA
      ue_common_vars->txdata[i] = NULL;
#else //IFFT_FPGA
      ue_common_vars->txdata[i] = TX_DMA_BUFFER[0][i];
#endif //IFFT_FPGA
#endif //USER_MODE
    }

    ue_common_vars->txdataF = (mod_sym_t **)malloc16(frame_parms->nb_antennas_tx*sizeof(mod_sym_t*));
    for (i=0; i<frame_parms->nb_antennas_tx; i++) {
#ifdef USER_MODE
      ue_common_vars->txdataF[i] = (mod_sym_t *)malloc16(FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX*sizeof(mod_sym_t));
      bzero(ue_common_vars->txdataF[i],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX*sizeof(mod_sym_t));
#else //USER_MODE
#ifdef IFFT_FPGA
      ue_common_vars->txdataF[i] = (mod_sym_t*) TX_DMA_BUFFER[0][i];
#else //IFFT_FPGA
#error "IFFT_FPGA and USER_MODE cannot be undefined at the same time"
#endif //IFFT_FPGA
#endif //USER_MODE
    }
    
    // RX buffers
    ue_common_vars->rxdata = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
    if (ue_common_vars->rxdata) {
#ifdef DEBUG_PHY
      msg("[openair][LTE_PHY][INIT] ue_common_vars->rxdata allocated at %p\n", ue_common_vars->rxdata);
#endif
    }
    else {
      msg("[openair][LTE_PHY][INIT] ue_common_vars->rxdata not allocated\n");
      return(-1);
    }
    
    for (i=0; i<frame_parms->nb_antennas_rx; i++) {
#ifndef USER_MODE
      ue_common_vars->rxdata[i] = (int*) RX_DMA_BUFFER[0][i];
#else //USER_MODE
      ue_common_vars->rxdata[i] = (int*) malloc16(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(int));
#endif //USER_MODE
      if (ue_common_vars->rxdata[i]) {
#ifdef DEBUG_PHY
	msg("[openair][LTE_PHY][INIT] ue_common_vars->rxdata[%d] allocated at %p\n",i,ue_common_vars->rxdata[i]);
#endif
      }
      else {
	msg("[openair][LTE_PHY][INIT] ue_common_vars->rxdata[%d] not allocated\n",i);
	return(-1);
      }
    }
    
    ue_common_vars->rxdataF = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
    if (ue_common_vars->rxdataF) {
#ifdef DEBUG_PHY
      msg("[openair][LTE_PHY][INIT] ue_common_vars->rxdataF allocated at %p\n", ue_common_vars->rxdataF);
#endif
    }
    else {
      msg("[openair][LTE_PHY][INIT] ue_common_vars->rxdataF not allocated\n");
      return(-1);
    }
    
    for (i=0; i<frame_parms->nb_antennas_rx; i++) {
      //RK 2 times because of output format of FFT!  We should get rid of this
      ue_common_vars->rxdataF[i] = (int *)malloc16(2*sizeof(int)*(frame_parms->ofdm_symbol_size*14));
      if (ue_common_vars->rxdataF[i]) {
#ifdef DEBUG_PHY
	msg("[openair][LTE_PHY][INIT] ue_common_vars->rxdataF[%d] allocated at %p\n",i,ue_common_vars->rxdataF[i]);
#endif
      }
      else {
	msg("[openair][LTE_PHY][INIT] ue_common_vars->rxdataF[%d] not allocated\n",i);
	return(-1);
      }
    }

    ue_common_vars->rxdataF2 = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
    if (ue_common_vars->rxdataF2) {
#ifdef DEBUG_PHY
      msg("[openair][LTE_PHY][INIT] ue_common_vars->rxdataF2 allocated at %p\n", ue_common_vars->rxdataF2);
#endif
    }
    else {
      msg("[openair][LTE_PHY][INIT] ue_common_vars->rxdataF2 not allocated\n");
      return(-1);
    }

    
    for (i=0; i<frame_parms->nb_antennas_rx; i++) {
      //RK 2 times because of output format of FFT!  We should get rid of this
      ue_common_vars->rxdataF2[i] = (int *)malloc16(2*sizeof(int)*(frame_parms->ofdm_symbol_size*frame_parms->symbols_per_tti*10));
      if (ue_common_vars->rxdataF2[i]) {
#ifdef DEBUG_PHY
	msg("[openair][LTE_PHY][INIT] ue_common_vars->rxdataF2[%d] allocated at %p\n",i,ue_common_vars->rxdataF2[i]);
#endif
      }
      else {
	msg("[openair][LTE_PHY][INIT] ue_common_vars->rxdataF2[%d] not allocated\n",i);
	return(-1);
      }
    }
  }

    
  // Channel estimates  
  for (eNB_id=0;eNB_id<3;eNB_id++) {
    ue_common_vars->dl_ch_estimates[eNB_id] = (int **)malloc16(8*sizeof(int*));
    if (ue_common_vars->dl_ch_estimates[eNB_id]) {
#ifdef DEBUG_PHY
      msg("[openair][LTE_PHY][INIT] ue_common_vars->dl_ch_estimates (eNB %d) allocated at %p\n",
	  eNB_id,ue_common_vars->dl_ch_estimates[eNB_id]);
#endif
    }
    else {
      msg("[openair][LTE_PHY][INIT] ue_common_vars->dl_ch_estimates not allocated\n");
      return(-1);
    }
    
    
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<4; j++) { //frame_parms->nb_antennas_tx; j++) {
	ue_common_vars->dl_ch_estimates[eNB_id][(j<<1) + i] = (int *)malloc16(frame_parms->symbols_per_tti*sizeof(int)*(frame_parms->ofdm_symbol_size)+LTE_CE_FILTER_LENGTH);
	if (ue_common_vars->dl_ch_estimates[eNB_id][(j<<1)+i]) {
#ifdef DEBUG_PHY
	  msg("[openair][LTE_PHY][INIT] ue_common_vars->dl_ch_estimates[%d][%d] allocated at %p\n",eNB_id,(j<<1)+i,
	      ue_common_vars->dl_ch_estimates[eNB_id][(j<<1)+i]);
#endif
	  
	  memset(ue_common_vars->dl_ch_estimates[eNB_id][(j<<1)+i],0,frame_parms->symbols_per_tti*sizeof(int)*(frame_parms->ofdm_symbol_size));
	}
	else {
	  msg("[openair][LTE_PHY][INIT] ue_common_vars->dl_ch_estimates[%d] not allocated\n",i);
	  return(-1);
	}
      }
  }
    
    
  ue_common_vars->dl_ch_estimates_time = (int **)malloc16(8*sizeof(int*));
  if (ue_common_vars->dl_ch_estimates_time) {
#ifdef DEBUG_PHY
    msg("[openair][LTE_PHY][INIT] ue_common_vars->dl_ch_estimates_time allocated at %p\n",
	ue_common_vars->dl_ch_estimates_time);
#endif
  }
  else {
    msg("[openair][LTE_PHY][INIT] ue_common_vars->dl_ch_estimates_time not allocated_time\n");
    return(-1);
  }
  
  for (i=0; i<frame_parms->nb_antennas_rx; i++)
    for (j=0; j<4; j++) {//frame_parms->nb_antennas_tx; j++) {
      ue_common_vars->dl_ch_estimates_time[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->ofdm_symbol_size)*2);
      if (ue_common_vars->dl_ch_estimates_time[(j<<1)+i]) {
#ifdef DEBUG_PHY
	msg("[openair][LTE_PHY][INIT] ue_common_vars->dl_ch_estimates_time[%d] allocated at %p\n",i,
	    ue_common_vars->dl_ch_estimates_time[(j<<1)+i]);
#endif
	
	memset(ue_common_vars->dl_ch_estimates_time[(j<<1)+i],0,sizeof(int)*(frame_parms->ofdm_symbol_size)*2);
      }
      else {
	msg("[openair][LTE_PHY][INIT] ue_common_vars->dl_ch_estimates_time[%d] not allocated\n",i);
	return(-1);
      }
    }    
    
  //  lte_ue_dlsch_vars = (LTE_UE_DLSCH **)malloc16(3*sizeof(LTE_UE_DLSCH*));
  //  lte_ue_pbch_vars = (LTE_UE_PBCH **)malloc16(3*sizeof(LTE_UE_PBCH*));

  // DLSCH
  for (eNB_id=0;eNB_id<NUMBER_OF_eNB_MAX;eNB_id++) {
    ue_dlsch_vars[eNB_id] = (LTE_UE_DLSCH *)malloc16(sizeof(LTE_UE_DLSCH));
    ue_dlsch_vars_cntl[eNB_id] = (LTE_UE_DLSCH *)malloc16(sizeof(LTE_UE_DLSCH));
    ue_dlsch_vars_ra[eNB_id] = (LTE_UE_DLSCH *)malloc16(sizeof(LTE_UE_DLSCH));
    ue_pdcch_vars[eNB_id] = (LTE_UE_PDCCH *)malloc16(sizeof(LTE_UE_PDCCH));

#ifdef DEBUG_PHY
    msg("[OPENAIR][LTE PHY][INIT] ue_dlsch_vars[%d] = %p\n",eNB_id,ue_dlsch_vars[eNB_id]);
    msg("[OPENAIR][LTE PHY][INIT] ue_dlsch_vars_cntl[%d] = %p\n",eNB_id,ue_dlsch_vars_cntl[eNB_id]);
    msg("[OPENAIR][LTE PHY][INIT] ue_dlsch_vars_ra[%d] = %p\n",eNB_id,ue_dlsch_vars_ra[eNB_id]);
    msg("[OPENAIR][LTE PHY][INIT] ue_pdcch_vars[%d] = %p\n",eNB_id,ue_pdcch_vars[eNB_id]);

#endif

    if (abstraction_flag == 0) {
      ue_dlsch_vars[eNB_id]->rxdataF_ext    = (int **)malloc16(8*sizeof(int*));
      for (i=0; i<frame_parms->nb_antennas_rx; i++)
	for (j=0; j<4; j++) //frame_parms->nb_antennas_tx; j++)
	  ue_dlsch_vars[eNB_id]->rxdataF_ext[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*14));
      
      ue_dlsch_vars[eNB_id]->rxdataF_comp    = (int **)malloc16(8*sizeof(int*));
      for (i=0; i<frame_parms->nb_antennas_rx; i++)
	for (j=0; j<4;j++)//frame_parms->nb_antennas_tx; j++)
	  ue_dlsch_vars[eNB_id]->rxdataF_comp[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*14));
      //    printf("rxdataF_comp[0] %p\n",ue_dlsch_vars[eNB_id]->rxdataF_comp[0]);
      
      ue_pdcch_vars[eNB_id]->rxdataF_comp    = (int **)malloc16(8*sizeof(int*));
      for (i=0; i<frame_parms->nb_antennas_rx; i++)
	for (j=0; j<4;j++)//frame_parms->nb_antennas_tx; j++)
	  ue_pdcch_vars[eNB_id]->rxdataF_comp[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*4));
      
      ue_pdcch_vars[eNB_id]->dl_ch_rho_ext = (int **)malloc16(8*sizeof(short*));
      for (i=0; i<frame_parms->nb_antennas_rx; i++)
	for (j=0; j<4;j++)//frame_parms->nb_antennas_tx; j++)
	  ue_pdcch_vars[eNB_id]->dl_ch_rho_ext[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
      
      ue_pdcch_vars[eNB_id]->rho = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
      for (i=0;i<frame_parms->nb_antennas_rx;i++)
	ue_pdcch_vars[eNB_id]->rho[i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*7*2));

      ue_dlsch_vars[eNB_id]->dl_ch_estimates_ext = (int **)malloc16(8*sizeof(int*));
      for (i=0; i<frame_parms->nb_antennas_rx; i++)
	for (j=0; j<4;j++)//frame_parms->nb_antennas_tx; j++)
	  ue_dlsch_vars[eNB_id]->dl_ch_estimates_ext[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));

           
      ue_dlsch_vars[eNB_id]->dl_ch_rho_ext = (int **)malloc16(8*sizeof(short*));
      for (i=0; i<frame_parms->nb_antennas_rx; i++)
	for (j=0; j<4;j++)//frame_parms->nb_antennas_tx; j++)
	  ue_dlsch_vars[eNB_id]->dl_ch_rho_ext[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
      
      ue_dlsch_vars[eNB_id]->pmi_ext = (unsigned char *)malloc16(frame_parms->N_RB_DL);

            
      ue_dlsch_vars[eNB_id]->dl_ch_mag = (int **)malloc16(8*sizeof(short*));
      for (i=0; i<frame_parms->nb_antennas_rx; i++)
	for (j=0; j<4;j++)//frame_parms->nb_antennas_tx; j++) 
	  ue_dlsch_vars[eNB_id]->dl_ch_mag[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
      ue_dlsch_vars[eNB_id]->dl_ch_magb = (int **)malloc16(8*sizeof(short*));
      for (i=0; i<frame_parms->nb_antennas_rx; i++)
	for (j=0; j<4;j++)//frame_parms->nb_antennas_tx; j++)
	  ue_dlsch_vars[eNB_id]->dl_ch_magb[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
      
      ue_dlsch_vars[eNB_id]->rho = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
      for (i=0;i<frame_parms->nb_antennas_rx;i++)
	ue_dlsch_vars[eNB_id]->rho[i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*7*2));
      
      ue_dlsch_vars[eNB_id]->llr[0] = (short *)malloc16((8*((3*8*6144)+12))*sizeof(short));
      ue_dlsch_vars[eNB_id]->llr[1] = (short *)malloc16((8*((3*8*6144)+12))*sizeof(short));
      
      ue_dlsch_vars[eNB_id]->llr128 = (short **)malloc16(sizeof(short **));
      
     
      ue_dlsch_vars_cntl[eNB_id]->rxdataF_ext    = (int **)malloc16(8*sizeof(int*));
      for (i=0; i<frame_parms->nb_antennas_rx; i++)
	for (j=0; j<4; j++) //frame_parms->nb_antennas_tx; j++)
	  ue_dlsch_vars_cntl[eNB_id]->rxdataF_ext[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*14));
      
      ue_dlsch_vars_cntl[eNB_id]->rxdataF_comp    = (int **)malloc16(8*sizeof(int*));
      
      ue_dlsch_vars_cntl[eNB_id]->pmi_ext = (unsigned char *)malloc16(frame_parms->N_RB_DL);
      
      for (i=0; i<frame_parms->nb_antennas_rx; i++)
	for (j=0; j<4;j++)//frame_parms->nb_antennas_tx; j++)
	  ue_dlsch_vars_cntl[eNB_id]->rxdataF_comp[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*14));
      
      ue_dlsch_vars_cntl[eNB_id]->rho = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
      for (i=0;i<frame_parms->nb_antennas_rx;i++)
	ue_dlsch_vars_cntl[eNB_id]->rho[i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*7*2));
      
      ue_dlsch_vars_cntl[eNB_id]->llr[0] = (short *)malloc16((8*((3*8*6144)+12))*sizeof(short));
      
      
      /***/
      
      ue_dlsch_vars_ra[eNB_id]->rxdataF_ext    = (int **)malloc16(8*sizeof(int*));
      for (i=0; i<frame_parms->nb_antennas_rx; i++)
	for (j=0; j<4;j++)//frame_parms->nb_antennas_tx; j++)
	  ue_dlsch_vars_ra[eNB_id]->rxdataF_ext[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*14));
      
      ue_dlsch_vars_ra[eNB_id]->rxdataF_comp    = (int **)malloc16(8*sizeof(int*));
      for (i=0; i<frame_parms->nb_antennas_rx; i++)
	for (j=0; j<4;j++)//frame_parms->nb_antennas_tx; j++)
	  ue_dlsch_vars_ra[eNB_id]->rxdataF_comp[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*14));
      
      ue_dlsch_vars_ra[eNB_id]->rho = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
      
      ue_dlsch_vars_ra[eNB_id]->pmi_ext = (unsigned char *)malloc16(frame_parms->N_RB_DL);
      
      for (i=0;i<frame_parms->nb_antennas_rx;i++)
	ue_dlsch_vars_ra[eNB_id]->rho[i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*7*2));
      
      ue_dlsch_vars_ra[eNB_id]->llr[0] = (short *)malloc16((8*((3*8*6144)+12))*sizeof(short));
      
      /***/
      
      
      ue_pdcch_vars[eNB_id]->rxdataF_comp    = (int **)malloc16(8*sizeof(int*));
      for (i=0; i<frame_parms->nb_antennas_rx; i++)
	for (j=0; j<4;j++)//frame_parms->nb_antennas_tx; j++)
	  ue_pdcch_vars[eNB_id]->rxdataF_comp[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*4));
      
      /***/
      
      ue_dlsch_vars_cntl[eNB_id]->dl_ch_estimates_ext = (int **)malloc16(8*sizeof(short*));
      for (i=0; i<frame_parms->nb_antennas_rx; i++)
	for (j=0; j<4;j++)//frame_parms->nb_antennas_tx; j++)
	  ue_dlsch_vars_cntl[eNB_id]->dl_ch_estimates_ext[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
      
      ue_dlsch_vars_cntl[eNB_id]->dl_ch_rho_ext = (int **)malloc16(8*sizeof(short*));
      for (i=0; i<frame_parms->nb_antennas_rx; i++)
	for (j=0; j<4;j++)//frame_parms->nb_antennas_tx; j++)
	  ue_dlsch_vars_cntl[eNB_id]->dl_ch_rho_ext[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
      
      ue_dlsch_vars_cntl[eNB_id]->dl_ch_mag = (int **)malloc16(8*sizeof(short*));
      for (i=0; i<frame_parms->nb_antennas_rx; i++)
	for (j=0; j<4;j++)//frame_parms->nb_antennas_tx; j++) 
	  ue_dlsch_vars_cntl[eNB_id]->dl_ch_mag[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
    
      ue_dlsch_vars_cntl[eNB_id]->dl_ch_magb = (int **)malloc16(8*sizeof(short*));
      for (i=0; i<frame_parms->nb_antennas_rx; i++)
	for (j=0; j<4;j++)//frame_parms->nb_antennas_tx; j++)
	  ue_dlsch_vars_cntl[eNB_id]->dl_ch_magb[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
      ue_dlsch_vars_cntl[eNB_id]->llr[0] = (short *)malloc16((8*((3*8*6144)+12))*sizeof(short));
      
      /***/
      
      ue_dlsch_vars_cntl[eNB_id]->llr128 = (short **)malloc16(sizeof(short **));
      
      ue_dlsch_vars_ra[eNB_id]->dl_ch_estimates_ext = (int **)malloc16(8*sizeof(short*));
      for (i=0; i<frame_parms->nb_antennas_rx; i++)
	for (j=0; j<4;j++)//frame_parms->nb_antennas_tx; j++)
	  ue_dlsch_vars_ra[eNB_id]->dl_ch_estimates_ext[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
      
      ue_dlsch_vars_ra[eNB_id]->dl_ch_rho_ext = (int **)malloc16(8*sizeof(short*));
      for (i=0; i<frame_parms->nb_antennas_rx; i++)
	for (j=0; j<4;j++)//frame_parms->nb_antennas_tx; j++)
	  ue_dlsch_vars_ra[eNB_id]->dl_ch_rho_ext[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
      
      ue_dlsch_vars_ra[eNB_id]->dl_ch_mag = (int **)malloc16(8*sizeof(short*));
      for (i=0; i<frame_parms->nb_antennas_rx; i++)
	for (j=0; j<4;j++)//frame_parms->nb_antennas_tx; j++) 
	  ue_dlsch_vars_ra[eNB_id]->dl_ch_mag[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
      
      ue_dlsch_vars_ra[eNB_id]->dl_ch_magb = (int **)malloc16(8*sizeof(short*));
      for (i=0; i<frame_parms->nb_antennas_rx; i++)
	for (j=0; j<4;j++)//frame_parms->nb_antennas_tx; j++)
	  ue_dlsch_vars_ra[eNB_id]->dl_ch_magb[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
      //    ue_dlsch_vars_ra[eNB_id]->llr[0] = (short *)malloc16((8*((3*8*6144)+12))*sizeof(short));
      
      ue_dlsch_vars_ra[eNB_id]->llr128 = (short **)malloc16(sizeof(short **));
      /***/
      
      ue_pdcch_vars[eNB_id]->rxdataF_ext    = (int **)malloc16(8*sizeof(int*));
      for (i=0; i<frame_parms->nb_antennas_rx; i++)
	for (j=0; j<4;j++)//frame_parms->nb_antennas_tx; j++)
	  ue_pdcch_vars[eNB_id]->rxdataF_ext[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*14));
      
      ue_pdcch_vars[eNB_id]->dl_ch_estimates_ext = (int **)malloc16(8*sizeof(short*));
      for (i=0; i<frame_parms->nb_antennas_rx; i++)
	for (j=0; j<4;j++)//frame_parms->nb_antennas_tx; j++)
	  ue_pdcch_vars[eNB_id]->dl_ch_estimates_ext[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
      ue_pdcch_vars[eNB_id]->llr = (unsigned short *)malloc16(4*frame_parms->N_RB_DL*12*sizeof(unsigned short));
      ue_pdcch_vars[eNB_id]->llr16 = (unsigned short *)malloc16(2*4*frame_parms->N_RB_DL*12*sizeof(unsigned short));
      ue_pdcch_vars[eNB_id]->wbar = (unsigned short *)malloc16(4*frame_parms->N_RB_DL*12*sizeof(unsigned short));
      
      ue_pdcch_vars[eNB_id]->e_rx = (char *)malloc16(4*2*frame_parms->N_RB_DL*12*sizeof(unsigned char));
      
      // PBCH
      ue_pbch_vars[eNB_id] = (LTE_UE_PBCH *)malloc16(sizeof(LTE_UE_PBCH));
      ue_pbch_vars[eNB_id]->rxdataF_ext    = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
      for (i=0;i<frame_parms->nb_antennas_rx;i++)
	ue_pbch_vars[eNB_id]->rxdataF_ext[i] = (int *)malloc16(sizeof(int)*(6*12*4));
      
      ue_pbch_vars[eNB_id]->rxdataF_comp    = (int **)malloc16(8*sizeof(int*));
      ue_pbch_vars[eNB_id]->dl_ch_estimates_ext = (int **)malloc16(8*sizeof(short*));
      
      for (i=0;i<frame_parms->nb_antennas_rx;i++)
	for (j=0;j<4;j++){//frame_parms->nb_antennas_tx;j++) {
	  ue_pbch_vars[eNB_id]->rxdataF_comp[(j<<1)+i]        = (int *)malloc16(sizeof(int)*(6*12*4));
	  ue_pbch_vars[eNB_id]->dl_ch_estimates_ext[(j<<1)+i] = (int *)malloc16(sizeof(int)*6*12*4);
	}    
      ue_pbch_vars[eNB_id]->llr = (char *)malloc16(1920*sizeof(char));
      
      //    ue_pbch_vars[eNB_id]->channel_output = (short *)malloc16(*sizeof(short));
      
      ue_pbch_vars[eNB_id]->decoded_output = (unsigned char *)malloc16(64*sizeof(unsigned char));
      
      ue_pbch_vars[eNB_id]->pdu_errors_conseq=0;
      ue_pbch_vars[eNB_id]->pdu_errors=0;
      ue_pbch_vars[eNB_id]->pdu_errors_last=0;
      ue_pbch_vars[eNB_id]->pdu_fer=0;
    
  
      // Initialize Gold sequence table
      // lte_gold(frame_parms); --> moved to phy_init_lte_top
      
      // Initialize Sync
      // lte_sync_time_init(frame_parms); --> moved to phy_init_lte_top
      
#ifndef NO_UL_REF 
      // generate_ul_ref_sigs(); --> moved to phy_init_lte_top
#endif
      
      
      if (phy_vars_ue->is_secondary_ue) {
	phy_vars_ue->ul_precoder_S_UE = (int **)malloc16(4*sizeof(int*));
	if (phy_vars_ue->ul_precoder_S_UE) {
#ifdef DEBUG_PHY
	  msg("[openair][SECSYS_PHY][INIT] phy_vars_ue->ul_precoder_S_UE allocated at %p\n",phy_vars_ue->ul_precoder_S_UE);
#endif
	}
	else {
	  msg("[openair][SECSYS_PHY][INIT] phy_vars_ue->ul_precoder_S_UE not allocated\n");
	  return(-1);
	}
	
	for (j=0; j<phy_vars_ue->lte_frame_parms.nb_antennas_tx; j++) {
	  phy_vars_ue->ul_precoder_S_UE[j] = (int *)malloc16(2*sizeof(int)*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size)); // repeated format (hence the '2*')
	  if (phy_vars_ue->ul_precoder_S_UE[j]) {
#ifdef DEBUG_PHY
	    msg("[openair][SECSYS_PHY][INIT] phy_vars_ue->ul_precoder_S_UE[%d] allocated at %p\n",j,
		phy_vars_ue->ul_precoder_S_UE[j]);
#endif
	    memset(phy_vars_ue->ul_precoder_S_UE[j],0,2*sizeof(int)*(phy_vars_ue->lte_frame_parms.ofdm_symbol_size));
	  }
	  else {
	    msg("[openair][SECSYS_PHY][INIT] phy_vars_ue->ul_precoder_S_UE[%d] not allocated\n",j);
	    return(-1);
	  }
	} //for(j=...nb_antennas_tx
      }
    }
    else {
      ue_pbch_vars[eNB_id] = (LTE_UE_PBCH *)malloc16(sizeof(LTE_UE_PBCH));
      ue_pbch_vars[eNB_id]->pdu_errors_conseq=0;
      ue_pbch_vars[eNB_id]->pdu_errors=0;
      ue_pbch_vars[eNB_id]->pdu_errors_last=0;
      ue_pbch_vars[eNB_id]->pdu_fer=0;
      ue_pbch_vars[eNB_id]->decoded_output = (unsigned char *)malloc16(64*sizeof(unsigned char));
    } 
  }
  //initialization for the last instance of ue_dlsch_vars (used for MU-MIMO)
  ue_dlsch_vars[NUMBER_OF_eNB_MAX] = (LTE_UE_DLSCH *)malloc16(sizeof(LTE_UE_DLSCH));
#ifdef DEBUG_PHY
  msg("[OPENAIR][LTE PHY][INIT] ue_dlsch_vars[%d] = %p\n",NUMBER_OF_eNB_MAX,ue_dlsch_vars[NUMBER_OF_eNB_MAX]);
#endif
  if(abstraction_flag == 0){
    
    ue_dlsch_vars[NUMBER_OF_eNB_MAX]->rxdataF_ext    = (int **)malloc16(8*sizeof(int*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<4; j++) //frame_parms->nb_antennas_tx; j++)
	ue_dlsch_vars[NUMBER_OF_eNB_MAX]->rxdataF_ext[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*14));
    
    ue_dlsch_vars[NUMBER_OF_eNB_MAX]->rxdataF_comp    = (int **)malloc16(8*sizeof(int*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<4;j++)//frame_parms->nb_antennas_tx; j++)
	ue_dlsch_vars[NUMBER_OF_eNB_MAX]->rxdataF_comp[(j<<1)+i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*14));
    //    printf("rxdataF_comp[0] %p\n",ue_dlsch_vars[eNB_id]->rxdataF_comp[0]);
    
    ue_dlsch_vars[NUMBER_OF_eNB_MAX]->dl_ch_estimates_ext = (int **)malloc16(8*sizeof(int*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<4;j++)//frame_parms->nb_antennas_tx; j++)
	ue_dlsch_vars[NUMBER_OF_eNB_MAX]->dl_ch_estimates_ext[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
    
    
    ue_dlsch_vars[NUMBER_OF_eNB_MAX]->dl_ch_rho_ext = (int **)malloc16(8*sizeof(short*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<4;j++)//frame_parms->nb_antennas_tx; j++)
	ue_dlsch_vars[NUMBER_OF_eNB_MAX]->dl_ch_rho_ext[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
    
    ue_dlsch_vars[NUMBER_OF_eNB_MAX]->pmi_ext = (unsigned char *)malloc16(frame_parms->N_RB_DL);
    
    
    ue_dlsch_vars[NUMBER_OF_eNB_MAX]->dl_ch_mag = (int **)malloc16(8*sizeof(short*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<4;j++)//frame_parms->nb_antennas_tx; j++) 
	ue_dlsch_vars[NUMBER_OF_eNB_MAX]->dl_ch_mag[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
    ue_dlsch_vars[NUMBER_OF_eNB_MAX]->dl_ch_magb = (int **)malloc16(8*sizeof(short*));
    for (i=0; i<frame_parms->nb_antennas_rx; i++)
      for (j=0; j<4;j++)//frame_parms->nb_antennas_tx; j++)
	ue_dlsch_vars[NUMBER_OF_eNB_MAX]->dl_ch_magb[(j<<1)+i] = (int *)malloc16(7*2*sizeof(int)*(frame_parms->N_RB_DL*12));
    
    ue_dlsch_vars[NUMBER_OF_eNB_MAX]->rho = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
    for (i=0;i<frame_parms->nb_antennas_rx;i++)
      ue_dlsch_vars[NUMBER_OF_eNB_MAX]->rho[i] = (int *)malloc16(sizeof(int)*(frame_parms->N_RB_DL*12*7*2));
    
    ue_dlsch_vars[NUMBER_OF_eNB_MAX]->llr[0] = (short *)malloc16((8*((3*8*6144)+12))*sizeof(short));
    ue_dlsch_vars[NUMBER_OF_eNB_MAX]->llr[1] = (short *)malloc16((8*((3*8*6144)+12))*sizeof(short));
    
    ue_dlsch_vars[NUMBER_OF_eNB_MAX]->llr128 = (short **)malloc16(sizeof(short **));
    
  }
  else { //abstraction == 1
    phy_vars_ue->sinr_dB = (double*) malloc16(frame_parms->N_RB_DL*2*sizeof(double));
  }

  phy_vars_ue->init_averaging = 1;

  return(0);
}

int phy_init_lte_eNB(LTE_DL_FRAME_PARMS *frame_parms,
		     LTE_eNB_COMMON *eNB_common_vars,
		     LTE_eNB_ULSCH **eNB_ulsch_vars,
		     unsigned char is_secondary_eNB,
		     PHY_VARS_eNB *phy_vars_eNB,
		     u8 cooperation_flag,// 0 for no cooperation,1 for Delay Diversity and 2 for Distributed Alamouti
		     unsigned char abstraction_flag)
{

  LTE_eNB_SRS *eNB_srs_vars = phy_vars_eNB->lte_eNB_srs_vars;

  int i, j, eNB_id, UE_id;


  lte_gold(frame_parms,phy_vars_eNB->lte_gold_table,0);
  generate_pcfich_reg_mapping(frame_parms);
  generate_phich_reg_mapping(frame_parms);

  for (UE_id=0; UE_id<NUMBER_OF_UE_MAX; UE_id++) {
    phy_vars_eNB->first_run_timing_advance[UE_id] = 1; ///This flag used to be static. With multiple eNBs this does no longer work, hence we put it in the structure. However it has to be initialized with 1, which is performed here.

    memset(&phy_vars_eNB->eNB_UE_stats[UE_id],0,sizeof(LTE_eNB_UE_stats));
  }
  phy_vars_eNB->first_run_I0_measurements = 1; ///This flag used to be static. With multiple eNBs this does no longer work, hence we put it in the structure. However it has to be initialized with 1, which is performed here.

  for (eNB_id=0; eNB_id<3; eNB_id++) {

    if (abstraction_flag==0) {
      // TX vars
      eNB_common_vars->txdata[eNB_id] = (int **)malloc16(frame_parms->nb_antennas_tx*sizeof(int*));
      if (eNB_common_vars->txdata[eNB_id]) {
#ifdef DEBUG_PHY
	msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->txdata[%d] allocated at %p\n",eNB_id,
	    eNB_common_vars->txdata[eNB_id]);
#endif
      }
      else {
	msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->txdata[%d] not allocated\n",eNB_id);
	return(-1);
      }
      
      for (i=0; i<frame_parms->nb_antennas_tx; i++) {
#ifdef USER_MODE
	eNB_common_vars->txdata[eNB_id][i] = (int *)malloc16(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(int));
	bzero(eNB_common_vars->txdata[eNB_id][i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(int));
#else // USER_MODE
#ifdef IFFT_FPGA
	eNB_common_vars->txdata[eNB_id][i] = NULL;
#else //IFFT_FPGA
	eNB_common_vars->txdata[eNB_id][i] = TX_DMA_BUFFER[eNB_id][i];
#endif //IFFT_FPGA
#endif //USER_MODE
#ifdef DEBUG_PHY
	msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->txdata[%d][%d] = %p\n",eNB_id,i,eNB_common_vars->txdata[eNB_id][i]);
#endif
      }
      
      eNB_common_vars->txdataF[eNB_id] = (mod_sym_t **)malloc16(frame_parms->nb_antennas_tx*sizeof(mod_sym_t*));
      if (eNB_common_vars->txdataF[eNB_id]) {
#ifdef DEBUG_PHY
	msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->txdataF[%d] allocated at %p\n",eNB_id,
	    eNB_common_vars->txdataF[eNB_id]);
#endif
      }
      else {
	msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->txdataF[%d] not allocated\n",eNB_id);
	return(-1);
      }

      for (i=0; i<frame_parms->nb_antennas_tx; i++) {
#ifdef USER_MODE
	eNB_common_vars->txdataF[eNB_id][i] = (mod_sym_t *)malloc16(FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX*sizeof(mod_sym_t));
	bzero(eNB_common_vars->txdataF[eNB_id][i],FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX*sizeof(mod_sym_t));
#else //USER_MODE
#ifdef IFFT_FPGA
	eNB_common_vars->txdataF[eNB_id][i] = (mod_sym_t *)TX_DMA_BUFFER[eNB_id][i];
#else
#error "IFFT_FPGA and USER_MODE cannot be undefined at the same time"
#endif //IFFT_FPGA
#endif //USER_MODE
#ifdef DEBUG_PHY
	msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->txdataF[%d][%d] = %p (%d bytes)\n",
	    eNB_id,i,eNB_common_vars->txdataF[eNB_id][i],
	    FRAME_LENGTH_COMPLEX_SAMPLES_NO_PREFIX*sizeof(mod_sym_t));
#endif
      }

      //RX vars
      eNB_common_vars->rxdata[eNB_id] = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
      if (eNB_common_vars->rxdata[eNB_id]) {
#ifdef DEBUG_PHY
	msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->rxdata[%d] allocated at %p\n",eNB_id,
	    eNB_common_vars->rxdata[eNB_id]);
#endif
      }
      else {
	msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->rxdata[%d] not allocated\n",eNB_id);
	return(-1);
      }
      
      for (i=0; i<frame_parms->nb_antennas_rx; i++) {
#ifndef USER_MODE
	eNB_common_vars->rxdata[eNB_id][i] = (int *)RX_DMA_BUFFER[eNB_id][i];
#else //USER_MODE
	eNB_common_vars->rxdata[eNB_id][i] = (int *)malloc16(FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(int));
	bzero(eNB_common_vars->rxdata[eNB_id][i],FRAME_LENGTH_COMPLEX_SAMPLES*sizeof(int));
#endif //USER_MODE
#ifdef DEBUG_PHY
	msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->rxdata[%d][%d] = %p\n",eNB_id,i,eNB_common_vars->rxdata[eNB_id][i]);
#endif
      }
      
      eNB_common_vars->rxdataF[eNB_id] = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
      if (eNB_common_vars->rxdataF[eNB_id]) {
#ifdef DEBUG_PHY
	msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->rxdataF[%d] allocated at %p\n",eNB_id,
	    eNB_common_vars->rxdataF[eNB_id]);
#endif
      }
      else {
	msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->rxdataF[%d] not allocated\n",eNB_id);
	return(-1);
      }
      
      for (i=0; i<frame_parms->nb_antennas_rx; i++) {
	//RK 2 times because of output format of FFT!  We should get rid of this
	eNB_common_vars->rxdataF[eNB_id][i] = (int *)malloc16(2*sizeof(int)*(frame_parms->ofdm_symbol_size*frame_parms->symbols_per_tti));
	if (eNB_common_vars->rxdataF[eNB_id][i]) {
#ifdef DEBUG_PHY
	  msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->rxdataF[%d][%d] allocated at %p\n",eNB_id,i,
	      eNB_common_vars->rxdataF[eNB_id][i]);
#endif
	}
	else {
	  msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->rxdataF[%d][%d] not allocated\n",eNB_id,i);
	  return(-1);
	}
      }
      
      // Channel estimates for SRS
      for (UE_id=0;UE_id<NUMBER_OF_UE_MAX;UE_id++) {
	
	eNB_srs_vars[UE_id].srs_ch_estimates[eNB_id] = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
	if (eNB_srs_vars[UE_id].srs_ch_estimates[eNB_id]) {
#ifdef DEBUG_PHY
	  msg("[openair][LTE_PHY][INIT] lte_eNB_srs_vars[%d].srs_ch_estimates[%d] allocated at %p\n",UE_id,eNB_id,
	      eNB_srs_vars[UE_id].srs_ch_estimates[eNB_id]);
#endif
	}
	else {
	  msg("[openair][LTE_PHY][INIT] lte_eNB_srs_vars[%d].srs_ch_estimates[%d] not allocated\n",UE_id,eNB_id);
	  return(-1);
	}
	
	for (i=0; i<frame_parms->nb_antennas_rx; i++) {
	  eNB_srs_vars[UE_id].srs_ch_estimates[eNB_id][i] = (int *)malloc16(sizeof(int)*(frame_parms->ofdm_symbol_size));
	  if (eNB_srs_vars[UE_id].srs_ch_estimates[eNB_id][i]) {
#ifdef DEBUG_PHY
	    msg("[openair][LTE_PHY][INIT] lte_eNB_srs_vars[%d]->srs_ch_estimates[%d][%d] allocated at %p\n",UE_id,eNB_id,i,
		eNB_srs_vars[UE_id].srs_ch_estimates[eNB_id][i]);
#endif
	    
	    memset(eNB_srs_vars[UE_id].srs_ch_estimates[eNB_id][i],0,sizeof(int)*(frame_parms->ofdm_symbol_size));
	  }
	  else {
	    msg("[openair][LTE_PHY][INIT] lte_eNB_srs_vars[%d]->srs_ch_estimates[%d][%d] not allocated\n",UE_id,eNB_id,i);
	    return(-1);
	  }
	}
	
	// Channel estimates for SRS (time)
	eNB_srs_vars[UE_id].srs_ch_estimates_time[eNB_id] = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
	if (eNB_srs_vars[UE_id].srs_ch_estimates_time[eNB_id]) {
#ifdef DEBUG_PHY
	  msg("[openair][LTE_PHY][INIT] lte_eNB_srs_vars[%d]->srs_ch_estimates_time[%d] allocated at %p\n",UE_id,eNB_id,
	      eNB_srs_vars[UE_id].srs_ch_estimates_time[eNB_id]);
#endif
	}
	else {
	  msg("[openair][LTE_PHY][INIT] lte_eNB_srs_vars[%d]->srs_ch_estimates_time[%d] not allocated\n",UE_id,eNB_id);
	  return(-1);
	}
	
	for (i=0; i<frame_parms->nb_antennas_rx; i++) {
	  eNB_srs_vars[UE_id].srs_ch_estimates_time[eNB_id][i] = (int *)malloc16(sizeof(int)*(frame_parms->ofdm_symbol_size)*2);
	  if (eNB_srs_vars[UE_id].srs_ch_estimates_time[eNB_id][i]) {
#ifdef DEBUG_PHY
	    msg("[openair][LTE_PHY][INIT] lte_eNB_srs_vars[%d].srs_ch_estimates_time[%d][%d] allocated at %p\n",UE_id,eNB_id,i,
		eNB_srs_vars[UE_id].srs_ch_estimates_time[eNB_id][i]);
#endif
	    
	    memset(eNB_srs_vars[UE_id].srs_ch_estimates_time[eNB_id][i],0,sizeof(int)*(frame_parms->ofdm_symbol_size)*2);
	  }
	  else {
	    msg("[openair][LTE_PHY][INIT] lte_eNB_srs_vars[%d]->srs_ch_estimates_time[%d][%d] not allocated\n",UE_id,eNB_id,i);
	    return(-1);
	  }
	}
      } //UE_id
      
      eNB_common_vars->sync_corr[eNB_id] = (unsigned int *)malloc16(LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*sizeof(int)*frame_parms->samples_per_tti);
      if (eNB_common_vars->sync_corr[eNB_id]) {
	bzero(eNB_common_vars->sync_corr[eNB_id],LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*sizeof(int)*frame_parms->samples_per_tti);
#ifdef DEBUG_PHY
	msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->sync_corr[%d] allocated at %p\n", eNB_id, eNB_common_vars->sync_corr[eNB_id]);
#endif
      }
      else {
	msg("[openair][LTE_PHY][INIT] lte_eNB_common_vars->sync_corr[%d] not allocated\n", eNB_id);
	return(-1);
      }
    }
  } //eNB_id
    
  
#ifndef NO_UL_REF 
  if (abstraction_flag==0) {
    generate_ul_ref_sigs_rx();
    
    // SRS
    for (UE_id=0;UE_id<NUMBER_OF_UE_MAX;UE_id++) {
      eNB_srs_vars[UE_id].srs = (int *)malloc16(2*frame_parms->ofdm_symbol_size*sizeof(int*));
      if (eNB_srs_vars[UE_id].srs) { 
#ifdef DEBUG_PHY
	msg("[openair][LTE_PHY][INIT] eNB_srs_vars[%d].srs allocated at %p\n",UE_id,eNB_srs_vars[UE_id].srs);
#endif
      }
      else {
	msg("[openair][LTE_PHY][INIT] eNB_srs_vars[%d].srs not allocated\n",UE_id);
	return(-1);
      }
    }
  }
#endif
    
    // ULSCH VARS

  for (UE_id=0; UE_id<NUMBER_OF_UE_MAX; UE_id++) {

    eNB_ulsch_vars[UE_id] = (LTE_eNB_ULSCH *)malloc16(NUMBER_OF_UE_MAX*sizeof(LTE_eNB_ULSCH));
    if (eNB_ulsch_vars[UE_id]) {
#ifdef DEBUG_PHY
      msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d] allocated at %p\n",UE_id,eNB_ulsch_vars[UE_id]);
#endif
    }
    else {
      msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d] not allocated\n",UE_id);
      return(-1);
    }

    if (abstraction_flag==0) {
      for (eNB_id=0; eNB_id<3; eNB_id++) {
	
	eNB_ulsch_vars[UE_id]->rxdataF_ext[eNB_id] = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
	if (eNB_ulsch_vars[UE_id]->rxdataF_ext[eNB_id]) {
#ifdef DEBUG_PHY
	  msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_ext[%d] allocated at %p\n",UE_id,eNB_id,
	      eNB_ulsch_vars[UE_id]->rxdataF_ext[eNB_id]);
#endif
	}
	else {
	  msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_ext[%d] not allocated\n",UE_id,eNB_id);
	  return(-1);
	}
	
	for (i=0; i<frame_parms->nb_antennas_rx; i++) {
	  //RK 2 times because of output format of FFT!  We should get rid of this
	  eNB_ulsch_vars[UE_id]->rxdataF_ext[eNB_id][i] = 
	    (int *)malloc16(2*sizeof(int)*(frame_parms->N_RB_UL*12*frame_parms->symbols_per_tti));
	  if (eNB_ulsch_vars[UE_id]->rxdataF_ext[eNB_id][i]) {
#ifdef DEBUG_PHY
	    msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_ext[%d][%d] allocated at %p\n",UE_id,eNB_id,i,
		eNB_ulsch_vars[UE_id]->rxdataF_ext[eNB_id][i]);
#endif
	  }
	  else {
	    msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_ext[%d][%d] not allocated\n",UE_id,eNB_id,i);
	    return(-1);
	  }
	}
	
	eNB_ulsch_vars[UE_id]->rxdataF_ext2[eNB_id] = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
	if (eNB_ulsch_vars[UE_id]->rxdataF_ext2[eNB_id]) {
#ifdef DEBUG_PHY
	  msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_ext2[%d] allocated at %p\n",UE_id,eNB_id,
	      eNB_ulsch_vars[UE_id]->rxdataF_ext2[eNB_id]);
#endif
	}
	else {
	  msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_ext[%d] not allocated\n",UE_id,eNB_id);
	  return(-1);
	}
	
	for (i=0; i<frame_parms->nb_antennas_rx; i++) {
	  eNB_ulsch_vars[UE_id]->rxdataF_ext2[eNB_id][i] = 
	    (int *)malloc16(sizeof(int)*(frame_parms->N_RB_UL*12*frame_parms->symbols_per_tti));
	  if (eNB_ulsch_vars[UE_id]->rxdataF_ext2[eNB_id][i]) {
#ifdef DEBUG_PHY
	    msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_ext2[%d][%d] allocated at %p\n",UE_id,eNB_id,i,
		eNB_ulsch_vars[UE_id]->rxdataF_ext2[eNB_id][i]);
#endif
	  }
	  else {
	    msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_ext[%d][%d] not allocated\n",UE_id,eNB_id,i);
	    return(-1);
	  }
	}
	
	// Channel estimates for DRS
	eNB_ulsch_vars[UE_id]->drs_ch_estimates[eNB_id] = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
	if (eNB_ulsch_vars[UE_id]->drs_ch_estimates[eNB_id]) {
#ifdef DEBUG_PHY
	  msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->drs_ch_estimates[%d] allocated at %p\n",UE_id,eNB_id,
	      eNB_ulsch_vars[UE_id]->drs_ch_estimates[eNB_id]);
#endif
	}
	else {
	  msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->drs_ch_estimates[%d] not allocated\n",UE_id,eNB_id);
	  return(-1);
	}
	
	
	for (i=0; i<frame_parms->nb_antennas_rx; i++) {
	  eNB_ulsch_vars[UE_id]->drs_ch_estimates[eNB_id][i] = 
	    (int *)malloc16(frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	  if (eNB_ulsch_vars[UE_id]->drs_ch_estimates[eNB_id][i]) {
#ifdef DEBUG_PHY
	    msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->drs_ch_estimates[%d][%d] allocated at %p\n",UE_id,eNB_id,i,
		eNB_ulsch_vars[UE_id]->drs_ch_estimates[eNB_id][i]);
#endif
	    
	    memset(eNB_ulsch_vars[UE_id]->drs_ch_estimates[eNB_id][i],0,frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	  }
	  else {
	    msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->drs_ch_estimates[%d][%d] not allocated\n",UE_id,eNB_id,i);
	    return(-1);
	  }
	}
	
	// Channel estimates for time domain DRS
	eNB_ulsch_vars[UE_id]->drs_ch_estimates_time[eNB_id] = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
	if (eNB_ulsch_vars[UE_id]->drs_ch_estimates_time[eNB_id]) {
#ifdef DEBUG_PHY
	  msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->drs_ch_estimates_time[%d] allocated at %p\n",UE_id,eNB_id,
	      eNB_ulsch_vars[UE_id]->drs_ch_estimates_time[eNB_id]);
#endif
	}
	else {
	  msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->drs_ch_estimates_time[%d] not allocated\n",UE_id,eNB_id);
	  return(-1);
	}
	
	
	for (i=0; i<frame_parms->nb_antennas_rx; i++) {
	  eNB_ulsch_vars[UE_id]->drs_ch_estimates_time[eNB_id][i] = 
	    (int *)malloc16(2*sizeof(int)*frame_parms->ofdm_symbol_size);
	  if (eNB_ulsch_vars[UE_id]->drs_ch_estimates_time[eNB_id][i]) {
#ifdef DEBUG_PHY
	    msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->drs_ch_estimates_time[%d][%d] allocated at %p\n",UE_id,eNB_id,i,
		eNB_ulsch_vars[UE_id]->drs_ch_estimates_time[eNB_id][i]);
#endif
	    
	    memset(eNB_ulsch_vars[UE_id]->drs_ch_estimates_time[eNB_id][i],0,sizeof(int)*frame_parms->ofdm_symbol_size);
	  }
	  else {
	    msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->drs_ch_estimates_time[%d][%d] not allocated\n",UE_id,eNB_id,i);
	    return(-1);
	  }
	}
	
	
	// In case of Distributed Alamouti Collabrative scheme separate channel estimates are required for both the UEs
	if(cooperation_flag == 2)
	  {
	    //UE 0 DRS estimates
	    eNB_ulsch_vars[UE_id]->drs_ch_estimates_0[eNB_id] = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
	    if (eNB_ulsch_vars[UE_id]->drs_ch_estimates_0[eNB_id]) {
#ifdef DEBUG_PHY
	      msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->drs_ch_estimates_0[%d] allocated at %p\n",UE_id,eNB_id,
		  eNB_ulsch_vars[UE_id]->drs_ch_estimates_0[eNB_id]);
#endif
	    }
	    else {
	      msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->drs_ch_estimates_0[%d] not allocated\n",UE_id,eNB_id);
	      return(-1);
	    }
	    
	    
	    for (i=0; i<frame_parms->nb_antennas_rx; i++) {
	      eNB_ulsch_vars[UE_id]->drs_ch_estimates_0[eNB_id][i] = 
		(int *)malloc16(frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	      if (eNB_ulsch_vars[UE_id]->drs_ch_estimates_0[eNB_id][i]) {
#ifdef DEBUG_PHY
		msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->drs_ch_estimates_0[%d][%d] allocated at %p\n",UE_id,eNB_id,i,
		    eNB_ulsch_vars[UE_id]->drs_ch_estimates_0[eNB_id][i]);
#endif
		
		memset(eNB_ulsch_vars[UE_id]->drs_ch_estimates_0[eNB_id][i],0,frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	      }
	      else {
		msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->drs_ch_estimates_0[%d][%d] not allocated\n",UE_id,eNB_id,i);
		return(-1);
	      }
	    }
	    
	    //UE 1 DRS estimates
	    eNB_ulsch_vars[UE_id]->drs_ch_estimates_1[eNB_id] = (int **)malloc16(frame_parms->nb_antennas_rx*sizeof(int*));
	    if (eNB_ulsch_vars[UE_id]->drs_ch_estimates_1[eNB_id]) {
#ifdef DEBUG_PHY
	      msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->drs_ch_estimates_1[%d] allocated at %p\n",UE_id,eNB_id,
		  eNB_ulsch_vars[UE_id]->drs_ch_estimates_1[eNB_id]);
#endif
	    }
	    else {
	      msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->drs_ch_estimates_1[%d] not allocated\n",UE_id,eNB_id);
	      return(-1);
	    }
	    
	    
	    for (i=0; i<frame_parms->nb_antennas_rx; i++) {
	      eNB_ulsch_vars[UE_id]->drs_ch_estimates_1[eNB_id][i] = 
		(int *)malloc16(frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	      if (eNB_ulsch_vars[UE_id]->drs_ch_estimates_1[eNB_id][i]) {
#ifdef DEBUG_PHY
		msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->drs_ch_estimates_1[%d][%d] allocated at %p\n",UE_id,eNB_id,i,
		    eNB_ulsch_vars[UE_id]->drs_ch_estimates_1[eNB_id][i]);
#endif
		
		memset(eNB_ulsch_vars[UE_id]->drs_ch_estimates_1[eNB_id][i],0,frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	      }
	      else {
		msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->drs_ch_estimates_1[%d][%d] not allocated\n",UE_id,eNB_id,i);
		return(-1);
	      }
	    }
	  }// cooperation_flag

	
	eNB_ulsch_vars[UE_id]->rxdataF_comp[eNB_id] = malloc16(frame_parms->nb_antennas_rx*sizeof(int**));
	if (eNB_ulsch_vars[UE_id]->rxdataF_comp[eNB_id]) {
#ifdef DEBUG_PHY
	  msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_comp[%d] allocated at %p\n",UE_id,eNB_id,
	      eNB_ulsch_vars[UE_id]->rxdataF_comp[eNB_id]);
#endif
	}
	else {
	  msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_comp[%d] not allocated\n",UE_id,eNB_id);
	  return(-1);
	}
	
	for (i=0; i<frame_parms->nb_antennas_rx; i++) {
	  eNB_ulsch_vars[UE_id]->rxdataF_comp[eNB_id][i] = 
	    (int *)malloc16(frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	  if (eNB_ulsch_vars[UE_id]->rxdataF_comp[eNB_id][i]) {
#ifdef DEBUG_PHY
	    msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_comp[%d][%d] allocated at %p\n",UE_id,eNB_id,i,
		eNB_ulsch_vars[UE_id]->rxdataF_comp[eNB_id][i]);
#endif
	    
	    memset(eNB_ulsch_vars[UE_id]->rxdataF_comp[eNB_id][i],0,frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	  }
	  else {
	    msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_comp[%d][%d] not allocated\n",UE_id,eNB_id,i);
	    return(-1);
	  }
	}
	
	
	
	// Compensated data for the case of Distributed Alamouti Scheme
	if(cooperation_flag == 2)
	  {
	    
	    // it will contain(y)*(h0*)
	    eNB_ulsch_vars[UE_id]->rxdataF_comp_0[eNB_id] = malloc16(frame_parms->nb_antennas_rx*sizeof(int**));
	    if (eNB_ulsch_vars[UE_id]->rxdataF_comp_0[eNB_id]) {
#ifdef DEBUG_PHY
	      msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_comp_0[%d] allocated at %p\n",UE_id,eNB_id,
		  eNB_ulsch_vars[UE_id]->rxdataF_comp_0[eNB_id]);
#endif
	    }
	    else {
	      msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_comp_0[%d] not allocated\n",UE_id,eNB_id);
	      return(-1);
	    }
	    
	    for (i=0; i<frame_parms->nb_antennas_rx; i++) {
	      eNB_ulsch_vars[UE_id]->rxdataF_comp_0[eNB_id][i] = 
		(int *)malloc16(frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	      if (eNB_ulsch_vars[UE_id]->rxdataF_comp_0[eNB_id][i]) {
#ifdef DEBUG_PHY
		msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_comp_0[%d][%d] allocated at %p\n",UE_id,eNB_id,i,
		    eNB_ulsch_vars[UE_id]->rxdataF_comp_0[eNB_id][i]);
#endif
		
		memset(eNB_ulsch_vars[UE_id]->rxdataF_comp_0[eNB_id][i],0,frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	      }
	      else {
		msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_comp_0[%d][%d] not allocated\n",UE_id,eNB_id,i);
		return(-1);
	      }
	    }
	    
	    
	    // it will contain(y*)*(h1)
	    eNB_ulsch_vars[UE_id]->rxdataF_comp_1[eNB_id] = malloc16(frame_parms->nb_antennas_rx*sizeof(int**));
	    if (eNB_ulsch_vars[UE_id]->rxdataF_comp_1[eNB_id]) {
#ifdef DEBUG_PHY
	      msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_comp_1[%d] allocated at %p\n",UE_id,eNB_id,
		  eNB_ulsch_vars[UE_id]->rxdataF_comp_1[eNB_id]);
#endif
	    }
	    else {
	      msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_comp_1[%d] not allocated\n",UE_id,eNB_id);
	      return(-1);
	    }
	    
	    for (i=0; i<frame_parms->nb_antennas_rx; i++) {
	      eNB_ulsch_vars[UE_id]->rxdataF_comp_1[eNB_id][i] = 
		(int *)malloc16(frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	      if (eNB_ulsch_vars[UE_id]->rxdataF_comp_1[eNB_id][i]) {
#ifdef DEBUG_PHY
		msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_comp_1[%d][%d] allocated at %p\n",UE_id,eNB_id,i,
		    eNB_ulsch_vars[UE_id]->rxdataF_comp_1[eNB_id][i]);
#endif
		
		memset(eNB_ulsch_vars[UE_id]->rxdataF_comp_1[eNB_id][i],0,frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	      }
	      else {
		msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->rxdataF_comp_1[%d][%d] not allocated\n",UE_id,eNB_id,i);
		return(-1);
	      }
	    }
	  }// cooperation_flag

	
	
	
	eNB_ulsch_vars[UE_id]->ul_ch_mag[eNB_id] = malloc16(frame_parms->nb_antennas_rx*sizeof(int**));
	if (eNB_ulsch_vars[UE_id]->ul_ch_mag[eNB_id]) {
#ifdef DEBUG_PHY
	  msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_mag[%d] allocated at %p\n",UE_id,eNB_id,
	      eNB_ulsch_vars[UE_id]->ul_ch_mag[eNB_id]);
#endif
	}
	else {
	  msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_mag[%d] not allocated\n",UE_id,eNB_id);
	  return(-1);
	}
	
	for (i=0; i<frame_parms->nb_antennas_rx; i++) {
	  eNB_ulsch_vars[UE_id]->ul_ch_mag[eNB_id][i] = 
	    (int *)malloc16(frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	  if (eNB_ulsch_vars[UE_id]->ul_ch_mag[eNB_id][i]) {
#ifdef DEBUG_PHY
	    msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_mag[%d][%d] allocated at %p\n",UE_id,eNB_id,i,
		eNB_ulsch_vars[UE_id]->ul_ch_mag[eNB_id][i]);
#endif
	    
	    memset(eNB_ulsch_vars[UE_id]->ul_ch_mag[eNB_id][i],0,frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	  }
	  else {
	    msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_mag[%d][%d] not allocated\n",UE_id,eNB_id,i);
	    return(-1);
	  }
	}
	
	eNB_ulsch_vars[UE_id]->ul_ch_magb[eNB_id] = malloc16(frame_parms->nb_antennas_rx*sizeof(int**));
	if (eNB_ulsch_vars[UE_id]->ul_ch_magb[eNB_id]) {
#ifdef DEBUG_PHY
	  msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_magb[%d] allocated at %p\n",UE_id,eNB_id,
	      eNB_ulsch_vars[UE_id]->ul_ch_magb[eNB_id]);
#endif
	}
	else {
	  msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_magb[%d] not allocated\n",UE_id,eNB_id);
	  return(-1);
	}
	
	for (i=0; i<frame_parms->nb_antennas_rx; i++) {
	  eNB_ulsch_vars[UE_id]->ul_ch_magb[eNB_id][i] = 
	    (int *)malloc16(frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	  if (eNB_ulsch_vars[UE_id]->ul_ch_magb[eNB_id][i]) {
#ifdef DEBUG_PHY
	    msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_magb[%d][%d] allocated at %p\n",UE_id,eNB_id,i,
		eNB_ulsch_vars[UE_id]->ul_ch_magb[eNB_id][i]);
#endif
	    
	    memset(eNB_ulsch_vars[UE_id]->ul_ch_magb[eNB_id][i],0,frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	  }
	  else {
	    msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_magb[%d][%d] not allocated\n",UE_id,eNB_id,i);
	    return(-1);
	  }
	}
	
	if(cooperation_flag == 2) // for Distributed Alamouti Scheme
	  {
	    // UE 0
	    eNB_ulsch_vars[UE_id]->ul_ch_mag_0[eNB_id] = malloc16(frame_parms->nb_antennas_rx*sizeof(int**));
	    if (eNB_ulsch_vars[UE_id]->ul_ch_mag_0[eNB_id]) {
#ifdef DEBUG_PHY
	      msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_mag_0[%d] allocated at %p\n",UE_id,eNB_id,
		  eNB_ulsch_vars[UE_id]->ul_ch_mag_0[eNB_id]);
#endif
	    }
	    else {
	      msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_mag_0[%d] not allocated\n",UE_id,eNB_id);
	      return(-1);
	    }
	    
	    for (i=0; i<frame_parms->nb_antennas_rx; i++) {
	      eNB_ulsch_vars[UE_id]->ul_ch_mag_0[eNB_id][i] = 
		(int *)malloc16(frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	      if (eNB_ulsch_vars[UE_id]->ul_ch_mag_0[eNB_id][i]) {
#ifdef DEBUG_PHY
		msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_mag_0[%d][%d] allocated at %p\n",UE_id,eNB_id,i,
		    eNB_ulsch_vars[UE_id]->ul_ch_mag_0[eNB_id][i]);
#endif
		
		memset(eNB_ulsch_vars[UE_id]->ul_ch_mag_0[eNB_id][i],0,frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	      }
	      else {
		msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_mag_0[%d][%d] not allocated\n",UE_id,eNB_id,i);
		return(-1);
	      }
	    }
	    
	    eNB_ulsch_vars[UE_id]->ul_ch_magb_0[eNB_id] = malloc16(frame_parms->nb_antennas_rx*sizeof(int**));
	    if (eNB_ulsch_vars[UE_id]->ul_ch_magb_0[eNB_id]) {
#ifdef DEBUG_PHY
	      msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_magb_0[%d] allocated at %p\n",UE_id,eNB_id,
		  eNB_ulsch_vars[UE_id]->ul_ch_magb_0[eNB_id]);
#endif
	    }
	    else {
	      msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_magb_0[%d] not allocated\n",UE_id,eNB_id);
	      return(-1);
	    }
	    
	    for (i=0; i<frame_parms->nb_antennas_rx; i++) {
	      eNB_ulsch_vars[UE_id]->ul_ch_magb_0[eNB_id][i] = 
		(int *)malloc16(frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	      if (eNB_ulsch_vars[UE_id]->ul_ch_magb_0[eNB_id][i]) {
#ifdef DEBUG_PHY
		msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_magb_0[%d][%d] allocated at %p\n",UE_id,eNB_id,i,
		    eNB_ulsch_vars[UE_id]->ul_ch_magb_0[eNB_id][i]);
#endif
		
		memset(eNB_ulsch_vars[UE_id]->ul_ch_magb_0[eNB_id][i],0,frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	      }
	      else {
		msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_magb_0[%d][%d] not allocated\n",UE_id,eNB_id,i);
		return(-1);
	      }
	    }
	    
	    
	    
	    // UE 1
	    eNB_ulsch_vars[UE_id]->ul_ch_mag_1[eNB_id] = malloc16(frame_parms->nb_antennas_rx*sizeof(int**));
	    if (eNB_ulsch_vars[UE_id]->ul_ch_mag_1[eNB_id]) {
#ifdef DEBUG_PHY
	      msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_mag_1[%d] allocated at %p\n",UE_id,eNB_id,
		  eNB_ulsch_vars[UE_id]->ul_ch_mag_1[eNB_id]);
#endif
	    }
	    else {
	      msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_mag_1[%d] not allocated\n",UE_id,eNB_id);
	      return(-1);
	    }
	    
	    for (i=0; i<frame_parms->nb_antennas_rx; i++) {
	      eNB_ulsch_vars[UE_id]->ul_ch_mag_1[eNB_id][i] = 
		(int *)malloc16(frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	      if (eNB_ulsch_vars[UE_id]->ul_ch_mag_1[eNB_id][i]) {
#ifdef DEBUG_PHY
		msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_mag_1[%d][%d] allocated at %p\n",UE_id,eNB_id,i,
		    eNB_ulsch_vars[UE_id]->ul_ch_mag_1[eNB_id][i]);
#endif
		
		memset(eNB_ulsch_vars[UE_id]->ul_ch_mag_1[eNB_id][i],0,frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	      }
	      else {
		msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_mag_1[%d][%d] not allocated\n",UE_id,eNB_id,i);
		return(-1);
	      }
	    }
	    
	    eNB_ulsch_vars[UE_id]->ul_ch_magb_1[eNB_id] = malloc16(frame_parms->nb_antennas_rx*sizeof(int**));
	    if (eNB_ulsch_vars[UE_id]->ul_ch_magb_1[eNB_id]) {
#ifdef DEBUG_PHY
	      msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_magb_1[%d] allocated at %p\n",UE_id,eNB_id,
		  eNB_ulsch_vars[UE_id]->ul_ch_magb_1[eNB_id]);
#endif
	    }
	    else {
	      msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_magb_1[%d] not allocated\n",UE_id,eNB_id);
	      return(-1);
	    }
	    
	    for (i=0; i<frame_parms->nb_antennas_rx; i++) {
	      eNB_ulsch_vars[UE_id]->ul_ch_magb_1[eNB_id][i] = 
		(int *)malloc16(frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	      if (eNB_ulsch_vars[UE_id]->ul_ch_magb_1[eNB_id][i]) {
#ifdef DEBUG_PHY
		msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_magb_1[%d][%d] allocated at %p\n",UE_id,eNB_id,i,
		    eNB_ulsch_vars[UE_id]->ul_ch_magb_1[eNB_id][i]);
#endif
		
		memset(eNB_ulsch_vars[UE_id]->ul_ch_magb_1[eNB_id][i],0,frame_parms->symbols_per_tti*sizeof(int)*frame_parms->N_RB_UL*12);
	      }
	      else {
		msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->ul_ch_magb_1[%d][%d] not allocated\n",UE_id,eNB_id,i);
		return(-1);
	      }
	    }
	  }//cooperation_flag 
	
      

      } //eNB_id
    
      eNB_ulsch_vars[UE_id]->llr = (short *)malloc16((8*((3*8*6144)+12))*sizeof(short));
      if (! eNB_ulsch_vars[UE_id]->llr) {
	msg("[openair][LTE_PHY][INIT] lte_eNB_ulsch_vars[%d]->llr not allocated\n",UE_id);
	return(-1);
      }
    } // abstraction_flag
  } //UE_id 

  if (abstraction_flag==0) {
    if (is_secondary_eNB) {
      for (eNB_id=0; eNB_id<3; eNB_id++) {
	phy_vars_eNB->dl_precoder_SeNB[eNB_id] = (int **)malloc16(4*sizeof(int*));
	if (phy_vars_eNB->dl_precoder_SeNB[eNB_id]) {
#ifdef DEBUG_PHY
	  msg("[openair][SECSYS_PHY][INIT] phy_vars_eNB->dl_precoder_SeNB[%d] allocated at %p\n",eNB_id,
	      phy_vars_eNB->dl_precoder_SeNB[eNB_id]);
#endif
	}
	else {
	  msg("[openair][SECSYS_PHY][INIT] phy_vars_eNB->dl_precoder_SeNB[%d] not allocated\n",eNB_id);
	  return(-1);
	}
	
	for (j=0; j<phy_vars_eNB->lte_frame_parms.nb_antennas_tx; j++) {
	  phy_vars_eNB->dl_precoder_SeNB[eNB_id][j] = (int *)malloc16(2*sizeof(int)*(phy_vars_eNB->lte_frame_parms.ofdm_symbol_size)); // repeated format (hence the '2*')
	  if (phy_vars_eNB->dl_precoder_SeNB[eNB_id][j]) {
#ifdef DEBUG_PHY
	    msg("[openair][LTE_PHY][INIT] phy_vars_eNB->dl_precoder_SeNB[%d][%d] allocated at %p\n",eNB_id,j,
		phy_vars_eNB->dl_precoder_SeNB[eNB_id][j]);
#endif
	    memset(phy_vars_eNB->dl_precoder_SeNB[eNB_id][j],0,2*sizeof(int)*(phy_vars_eNB->lte_frame_parms.ofdm_symbol_size));
	  }
	  else {
	    msg("[openair][LTE_PHY][INIT] phy_vars_eNB->dl_precoder_SeNB[%d][%d] not allocated\n",eNB_id,j);
	    return(-1);
	  }
	} //for(j=...nb_antennas_tx
	
      } //for(eNB_id...
    }
  }

  for (UE_id=0;UE_id<NUMBER_OF_UE_MAX;UE_id++)
    phy_vars_eNB->eNB_UE_stats_ptr[UE_id] = &phy_vars_eNB->eNB_UE_stats[UE_id];

  return (0);  
}
    
