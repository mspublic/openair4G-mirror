#ifdef USER_MODE
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>	/* for atoi(3) */
#include <unistd.h>	/* for getopt(3) */
#include <string.h>	/* for strerror(3) */
#include <sysexits.h>	/* for EX_* exit codes */
#include <errno.h>	/* for errno */
#else
#include <linux/module.h>  /* Needed by all modules */
#endif

#include "RRC/LITE/defs.h"
#include "COMMON/mac_rrc_primitives.h"
#include <asn_application.h>
#include <asn_internal.h>	/* for _ASN_DEFAULT_STACK_MAX */
#include <per_encoder.h>

#include "RRCConnectionRequest.h"
#include "UL-CCCH-Message.h"
#include "UL-DCCH-Message.h"
#include "DL-CCCH-Message.h"
#include "DL-DCCH-Message.h"
#include "EstablishmentCause.h"
#include "RRCConnectionSetup.h"
#include "SRB-ToAddModList.h"
#include "DRB-ToAddModList.h"

#include "RRCConnectionSetupComplete.h"
#include "RRCConnectionReconfigurationComplete.h"
#include "RRCConnectionReconfiguration.h"
#include "MasterInformationBlock.h"
#include "SystemInformation.h"

#include "SystemInformationBlockType1.h"

#include "SIB-Type.h"

//#include "PHY/defs.h"
#ifndef USER_MODE
#define msg printk
#ifndef errno
int errno;
#endif
#else
#define msg printf
#endif

/*
void assign_enum(ENUMERATED_t *x,uint8_t val) {
  uint8_t *buf=(uint8_t *)malloc(4);
  x->buf = buf;
  *buf=val;
  x->size=1;
}
*/ 
/*
void FREEMEM(void *ptr) {

	if (ptr)
	  free(ptr);
	else {
	  msg("********************Trying to free a null ptr************************\n");
	}
}

void *REALLOC(void *oldptr,size_t size) {

printf("Realloc\n");
if (oldptr)
free(oldptr);
return(malloc(size));
}
*/

int transmission_mode_rrc;//

uint8_t do_SIB1(uint8_t *buffer,
		SystemInformationBlockType1_t *sib1) {
  

  PLMN_IdentityInfo_t PLMN_identity_info;
  MCC_MNC_Digit_t dummy;
  asn_enc_rval_t enc_rval;
  SchedulingInfo_t schedulingInfo;
  SIB_Type_t sib_type;

  memset(sib1,0,sizeof(SystemInformationBlockType1_t));
  memset(&PLMN_identity_info,0,sizeof(PLMN_IdentityInfo_t));
  memset(&schedulingInfo,0,sizeof(SchedulingInfo_t));
  memset(&sib_type,0,sizeof(SIB_Type_t));

  PLMN_identity_info.plmn_Identity.mcc = CALLOC(1,sizeof(*PLMN_identity_info.plmn_Identity.mcc));
  memset(PLMN_identity_info.plmn_Identity.mcc,0,sizeof(*PLMN_identity_info.plmn_Identity.mcc));

  asn_set_empty(&PLMN_identity_info.plmn_Identity.mcc->list);//.size=0;  

  dummy=2;ASN_SEQUENCE_ADD(&PLMN_identity_info.plmn_Identity.mcc->list,&dummy);
  dummy=6;ASN_SEQUENCE_ADD(&PLMN_identity_info.plmn_Identity.mcc->list,&dummy);
  dummy=2;ASN_SEQUENCE_ADD(&PLMN_identity_info.plmn_Identity.mcc->list,&dummy);

  PLMN_identity_info.plmn_Identity.mnc.list.size=0;
  PLMN_identity_info.plmn_Identity.mnc.list.count=0;
  dummy=8;ASN_SEQUENCE_ADD(&PLMN_identity_info.plmn_Identity.mnc.list,&dummy);
  dummy=0;ASN_SEQUENCE_ADD(&PLMN_identity_info.plmn_Identity.mnc.list,&dummy);
  //assign_enum(&PLMN_identity_info.cellReservedForOperatorUse,PLMN_IdentityInfo__cellReservedForOperatorUse_notReserved); 
  PLMN_identity_info.cellReservedForOperatorUse=PLMN_IdentityInfo__cellReservedForOperatorUse_notReserved;

  ASN_SEQUENCE_ADD(&sib1->cellAccessRelatedInfo.plmn_IdentityList.list,&PLMN_identity_info);


  // 16 bits
  sib1->cellAccessRelatedInfo.trackingAreaCode.buf = MALLOC(2);
  sib1->cellAccessRelatedInfo.trackingAreaCode.buf[0]=0x00;
  sib1->cellAccessRelatedInfo.trackingAreaCode.buf[1]=0x10;
  sib1->cellAccessRelatedInfo.trackingAreaCode.size=2;
  sib1->cellAccessRelatedInfo.trackingAreaCode.bits_unused=0;

  // 28 bits
  sib1->cellAccessRelatedInfo.cellIdentity.buf = MALLOC(8);
  sib1->cellAccessRelatedInfo.cellIdentity.buf[0]=0x01;
  sib1->cellAccessRelatedInfo.cellIdentity.buf[1]=0x48;
  sib1->cellAccessRelatedInfo.cellIdentity.buf[2]=0x0f;
  sib1->cellAccessRelatedInfo.cellIdentity.buf[3]=0x03;
  sib1->cellAccessRelatedInfo.cellIdentity.size=4;
  sib1->cellAccessRelatedInfo.cellIdentity.bits_unused=4;

  //  assign_enum(&sib1->cellAccessRelatedInfo.cellBarred,SystemInformationBlockType1__cellAccessRelatedInfo__cellBarred_notBarred);  
  sib1->cellAccessRelatedInfo.cellBarred=SystemInformationBlockType1__cellAccessRelatedInfo__cellBarred_notBarred;  

  //  assign_enum(&sib1->cellAccessRelatedInfo.intraFreqReselection,SystemInformationBlockType1__cellAccessRelatedInfo__intraFreqReselection_allowed);
  sib1->cellAccessRelatedInfo.intraFreqReselection=SystemInformationBlockType1__cellAccessRelatedInfo__intraFreqReselection_allowed;
  sib1->cellAccessRelatedInfo.csg_Indication=0;

  sib1->cellSelectionInfo.q_RxLevMin=-70;
  sib1->cellSelectionInfo.q_RxLevMinOffset=NULL;

  sib1->freqBandIndicator = 2;

  //  assign_enum(&schedulingInfo.si_Periodicity,SchedulingInfo__si_Periodicity_rf8);
  schedulingInfo.si_Periodicity=SchedulingInfo__si_Periodicity_rf8;

  //  assign_enum(&sib_type,SIB_Type_sibType3);
  sib_type=SIB_Type_sibType3;

  ASN_SEQUENCE_ADD(&schedulingInfo.sib_MappingInfo.list,&sib_type);
  ASN_SEQUENCE_ADD(&sib1->schedulingInfoList.list,&schedulingInfo);
 
  sib1->tdd_Config = CALLOC(1,sizeof(struct TDD_Config));
  
  //assign_enum(&sib1->tdd_Config->subframeAssignment,TDD_Config__subframeAssignment_sa3);
  sib1->tdd_Config->subframeAssignment=TDD_Config__subframeAssignment_sa3;

  //  assign_enum(&sib1->tdd_Config->specialSubframePatterns,TDD_Config__specialSubframePatterns_ssp0);
  sib1->tdd_Config->specialSubframePatterns=TDD_Config__specialSubframePatterns_ssp0;

  //  assign_enum(&sib1->si_WindowLength,SystemInformationBlockType1__si_WindowLength_ms10);
  sib1->si_WindowLength=SystemInformationBlockType1__si_WindowLength_ms10;
  sib1->systemInfoValueTag=0;
  //  sib1.nonCriticalExtension = calloc(1,sizeof(*sib1.nonCriticalExtension));

#ifdef USER_MODE
  xer_fprint(stdout, &asn_DEF_SystemInformationBlockType1, (void*)sib1);
#endif

  enc_rval = uper_encode_to_buffer(&asn_DEF_SystemInformationBlockType1,
				   (void*)sib1,
				   buffer,
				   200);
#ifdef USER_MODE
  printf("[RRC][eNB] SystemInformationBlockType1 Encoded %d bits (%d bytes)\n",enc_rval.encoded,(enc_rval.encoded+7)/8);
#endif

  if (enc_rval.encoded==-1)
    return(-1);
  return((enc_rval.encoded+7)/8);
}

uint8_t do_SIB23(uint8_t *buffer,  
		 SystemInformation_t *systemInformation,
		 SystemInformationBlockType2_t **sib2,
		 SystemInformationBlockType3_t **sib3) {


  //  SystemInformationBlockType2_t *sib2;
  //  SystemInformationBlockType3_t *sib3;


  struct SystemInformation_r8_IEs__sib_TypeAndInfo__Member *sib2_part,*sib3_part;

  asn_enc_rval_t enc_rval;


  sib2_part = CALLOC(1,sizeof(struct SystemInformation_r8_IEs__sib_TypeAndInfo__Member));
  sib3_part = CALLOC(1,sizeof(struct SystemInformation_r8_IEs__sib_TypeAndInfo__Member));
  memset(sib2_part,0,sizeof(struct SystemInformation_r8_IEs__sib_TypeAndInfo__Member));
  memset(sib3_part,0,sizeof(struct SystemInformation_r8_IEs__sib_TypeAndInfo__Member));

  sib2_part->present = SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib2;
  sib3_part->present = SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib3;

  *sib2 = &sib2_part->choice.sib2;
  *sib3 = &sib3_part->choice.sib3;


  // sib2

  (*sib2)->ac_BarringInfo = NULL;
  (*sib2)->ssac_BarringForMMTEL_Voice_r9 = NULL;
  (*sib2)->ssac_BarringForMMTEL_Video_r9 = NULL;
  (*sib2)->ac_BarringForCSFB_r10 = NULL;
   
  //assign_enum(&(*sib2)->radioResourceConfigCommon.rach_ConfigCommon.preambleInfo.numberOfRA_Preambles,RACH_ConfigCommon__preambleInfo__numberOfRA_Preambles_n64);
  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.preambleInfo.numberOfRA_Preambles=RACH_ConfigCommon__preambleInfo__numberOfRA_Preambles_n64;
  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.preambleInfo.preamblesGroupAConfig = NULL; 
  //calloc(1,sizeof(*(*sib2)->rach_ConfigCommon.preambleInfo.preamblesGroupAConfig));
  //  assign_enum(&(*sib2)->rach_ConfigCommon.preambleInfo.preamblesGroupAConfig->sizeOfRA_PreamblesGroupA,
  //assign_enum(&(*sib2)->radioResourceConfigCommon.rach_ConfigCommon.powerRampingParameters.powerRampingStep, RACH_ConfigCommon__powerRampingParameters__powerRampingStep_dB2);
  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.powerRampingParameters.powerRampingStep=RACH_ConfigCommon__powerRampingParameters__powerRampingStep_dB2;

  //assign_enum(&(*sib2)->radioResourceConfigCommon.rach_ConfigCommon.powerRampingParameters.preambleInitialReceivedTargetPower,RACH_ConfigCommon__powerRampingParameters__preambleInitialReceivedTargetPower_dBm_116 );
  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.powerRampingParameters.preambleInitialReceivedTargetPower=RACH_ConfigCommon__powerRampingParameters__preambleInitialReceivedTargetPower_dBm_116;

  //assign_enum(&(*sib2)->radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.preambleTransMax,RACH_ConfigCommon__ra_SupervisionInfo__preambleTransMax_n10);
  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.preambleTransMax=RACH_ConfigCommon__ra_SupervisionInfo__preambleTransMax_n10;

  //assign_enum(&(*sib2)->radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.ra_ResponseWindowSize,RACH_ConfigCommon__ra_SupervisionInfo__ra_ResponseWindowSize_sf4);
  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.ra_ResponseWindowSize=RACH_ConfigCommon__ra_SupervisionInfo__ra_ResponseWindowSize_sf4;

  //assign_enum(&(*sib2)->radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.mac_ContentionResolutionTimer,RACH_ConfigCommon__ra_SupervisionInfo__mac_ContentionResolutionTimer_sf48);
  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.mac_ContentionResolutionTimer=RACH_ConfigCommon__ra_SupervisionInfo__mac_ContentionResolutionTimer_sf48;

  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.maxHARQ_Msg3Tx = 4;

  // BCCH-Config
  //assign_enum(&(*sib2)->radioResourceConfigCommon.bcch_Config.modificationPeriodCoeff,BCCH_Config__modificationPeriodCoeff_n2);
  (*sib2)->radioResourceConfigCommon.bcch_Config.modificationPeriodCoeff=BCCH_Config__modificationPeriodCoeff_n2;

  // PCCH-Config
  //assign_enum(&(*sib2)->radioResourceConfigCommon.pcch_Config.defaultPagingCycle,PCCH_Config__defaultPagingCycle_rf128);
  (*sib2)->radioResourceConfigCommon.pcch_Config.defaultPagingCycle=PCCH_Config__defaultPagingCycle_rf128;

  //  assign_enum(&(*sib2)->radioResourceConfigCommon.pcch_Config.nB,PCCH_Config__nB_oneT);
  (*sib2)->radioResourceConfigCommon.pcch_Config.nB=PCCH_Config__nB_oneT;

  // PRACH-Config
  (*sib2)->radioResourceConfigCommon.prach_Config.rootSequenceIndex=384;
  (*sib2)->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.prach_ConfigIndex = 3;
  (*sib2)->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.highSpeedFlag = 0;
  (*sib2)->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.zeroCorrelationZoneConfig = 12;
  (*sib2)->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.prach_FreqOffset = 2;

  // PDSCH-Config
  (*sib2)->radioResourceConfigCommon.pdsch_ConfigCommon.referenceSignalPower=15;
  (*sib2)->radioResourceConfigCommon.pdsch_ConfigCommon.p_b=0;

  // PUSCH-Config
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.n_SB=1;
  //assign_enum(&(*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.hoppingMode,PUSCH_ConfigCommon__pusch_ConfigBasic__hoppingMode_interSubFrame);
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.hoppingMode=PUSCH_ConfigCommon__pusch_ConfigBasic__hoppingMode_interSubFrame;
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.pusch_HoppingOffset=0;
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.enable64QAM=0;
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupHoppingEnabled=1;
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH=0;
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled=0;
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.cyclicShift=0;

  // PUCCH-Config

  //assign_enum(&(*sib2)->radioResourceConfigCommon.pucch_ConfigCommon.deltaPUCCH_Shift,PUCCH_ConfigCommon__deltaPUCCH_Shift_ds1);
  (*sib2)->radioResourceConfigCommon.pucch_ConfigCommon.deltaPUCCH_Shift=PUCCH_ConfigCommon__deltaPUCCH_Shift_ds1;
  (*sib2)->radioResourceConfigCommon.pucch_ConfigCommon.nRB_CQI = 1;
  (*sib2)->radioResourceConfigCommon.pucch_ConfigCommon.nCS_AN = 0;
  (*sib2)->radioResourceConfigCommon.pucch_ConfigCommon.n1PUCCH_AN = 32;


  (*sib2)->radioResourceConfigCommon.soundingRS_UL_ConfigCommon.present=SoundingRS_UL_ConfigCommon_PR_release;
  (*sib2)->radioResourceConfigCommon.soundingRS_UL_ConfigCommon.choice.release=0;

  // uplinkPowerControlCommon

  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.p0_NominalPUSCH = -96;
  //assign_enum(&(*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.alpha,UplinkPowerControlCommon__alpha_al1);
  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.alpha=UplinkPowerControlCommon__alpha_al1;
  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.p0_NominalPUCCH = -117;
  //assign_enum(&(*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format1, DeltaFList_PUCCH__deltaF_PUCCH_Format1_deltaF0);
  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format1=DeltaFList_PUCCH__deltaF_PUCCH_Format1_deltaF0;
  //  assign_enum(&(*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format1b, DeltaFList_PUCCH__deltaF_PUCCH_Format1b_deltaF3);
  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format1b=DeltaFList_PUCCH__deltaF_PUCCH_Format1b_deltaF3;

  //assign_enum(&(*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2, DeltaFList_PUCCH__deltaF_PUCCH_Format2_deltaF0);
  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2=DeltaFList_PUCCH__deltaF_PUCCH_Format2_deltaF0;
  
  //assign_enum(&(*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2a, DeltaFList_PUCCH__deltaF_PUCCH_Format2a_deltaF0);
  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2a=DeltaFList_PUCCH__deltaF_PUCCH_Format2a_deltaF0;

  //assign_enum(&(*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2b, DeltaFList_PUCCH__deltaF_PUCCH_Format2b_deltaF0);
  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2b=DeltaFList_PUCCH__deltaF_PUCCH_Format2b_deltaF0;

  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaPreambleMsg3 = 6;

  //assign_enum(&(*sib2)->radioResourceConfigCommon.ul_CyclicPrefixLength, UL_CyclicPrefixLength_len1);
  (*sib2)->radioResourceConfigCommon.ul_CyclicPrefixLength=UL_CyclicPrefixLength_len1;

  //assign_enum(&(*sib2)->ue_TimersAndConstants.t300, UE_TimersAndConstants__t300_ms1000);
  (*sib2)->ue_TimersAndConstants.t300=UE_TimersAndConstants__t300_ms1000;
 
  //assign_enum(&(*sib2)->ue_TimersAndConstants.t301, UE_TimersAndConstants__t301_ms1000);
  (*sib2)->ue_TimersAndConstants.t301=UE_TimersAndConstants__t301_ms1000;

  //assign_enum(&(*sib2)->ue_TimersAndConstants.t310, UE_TimersAndConstants__t310_ms1000);
  (*sib2)->ue_TimersAndConstants.t310=UE_TimersAndConstants__t310_ms1000;

  //assign_enum(&(*sib2)->ue_TimersAndConstants.n310, UE_TimersAndConstants__n310_n20);
  (*sib2)->ue_TimersAndConstants.n310=UE_TimersAndConstants__n310_n20;

  //assign_enum(&(*sib2)->ue_TimersAndConstants.t311, UE_TimersAndConstants__t311_ms1000);
  (*sib2)->ue_TimersAndConstants.t311=UE_TimersAndConstants__t311_ms1000;

  //assign_enum(&(*sib2)->ue_TimersAndConstants.n311, UE_TimersAndConstants__n311_n1);
  (*sib2)->ue_TimersAndConstants.n311=UE_TimersAndConstants__n311_n1;

  (*sib2)->freqInfo.additionalSpectrumEmission = 1;
  (*sib2)->freqInfo.ul_CarrierFreq = NULL;
  (*sib2)->freqInfo.ul_Bandwidth = NULL;
  (*sib2)->mbsfn_SubframeConfigList = NULL;
  //assign_enum(&(*sib2)->timeAlignmentTimerCommon,TimeAlignmentTimer_sf5120);
  (*sib2)->timeAlignmentTimerCommon=TimeAlignmentTimer_sf5120;

  /// (*SIB3)
  (*sib3)->s_IntraSearch_v920=NULL;
  (*sib3)->s_NonIntraSearch_v920=NULL;
  (*sib3)->q_QualMin_r9=NULL;
  (*sib3)->threshServingLowQ_r9=NULL; 
  //  assign_enum(&(*sib3)->cellReselectionInfoCommon.q_Hyst,SystemInformationBlockType3__cellReselectionInfoCommon__q_Hyst_dB4);
  (*sib3)->cellReselectionInfoCommon.q_Hyst=SystemInformationBlockType3__cellReselectionInfoCommon__q_Hyst_dB4;

  (*sib3)->cellReselectionInfoCommon.speedStateReselectionPars=NULL;

  (*sib3)->cellReselectionServingFreqInfo.s_NonIntraSearch=NULL;
  (*sib3)->cellReselectionServingFreqInfo.threshServingLow=31;
  (*sib3)->cellReselectionServingFreqInfo.cellReselectionPriority=7;

  (*sib3)->intraFreqCellReselectionInfo.q_RxLevMin = -70;
  (*sib3)->intraFreqCellReselectionInfo.p_Max = NULL;
  (*sib3)->intraFreqCellReselectionInfo.s_IntraSearch = CALLOC(1,sizeof((*sib3)->intraFreqCellReselectionInfo.s_IntraSearch));
  *(*sib3)->intraFreqCellReselectionInfo.s_IntraSearch = 31;
  (*sib3)->intraFreqCellReselectionInfo.allowedMeasBandwidth=CALLOC(1,sizeof(*(*sib3)->intraFreqCellReselectionInfo.allowedMeasBandwidth));

  //  assign_enum((*sib3)->intraFreqCellReselectionInfo.allowedMeasBandwidth,AllowedMeasBandwidth_mbw6);
  (*sib3)->intraFreqCellReselectionInfo.allowedMeasBandwidth=AllowedMeasBandwidth_mbw6;

  (*sib3)->intraFreqCellReselectionInfo.presenceAntennaPort1 = 0;
  (*sib3)->intraFreqCellReselectionInfo.neighCellConfig.buf = CALLOC(8,1);
  (*sib3)->intraFreqCellReselectionInfo.neighCellConfig.size = 1;
  (*sib3)->intraFreqCellReselectionInfo.neighCellConfig.buf[0] = 1;
  (*sib3)->intraFreqCellReselectionInfo.neighCellConfig.bits_unused = 6;
  (*sib3)->intraFreqCellReselectionInfo.t_ReselectionEUTRA = 1;
  (*sib3)->intraFreqCellReselectionInfo.t_ReselectionEUTRA_SF = (struct SpeedStateScaleFactors *)NULL;


  memset((void*)systemInformation,0,sizeof(SystemInformation_t));

  systemInformation->criticalExtensions.present = SystemInformation__criticalExtensions_PR_systemInformation_r8;

  systemInformation->criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list.count=0;

  //  asn_set_empty(&systemInformation->criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list);//.size=0;  
  //  systemInformation->criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list.count=0;
  ASN_SEQUENCE_ADD(&systemInformation->criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list,sib2_part);
  ASN_SEQUENCE_ADD(&systemInformation->criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list,sib3_part);

#ifdef USER_MODE
  xer_fprint(stdout, &asn_DEF_SystemInformation, (void*)systemInformation);
#endif
  enc_rval = uper_encode_to_buffer(&asn_DEF_SystemInformation,
				   (void*)systemInformation,
				   buffer,
				   100);
#ifdef USER_MODE
  printf("[RRC][eNB] SystemInformation Encoded %d bits (%d bytes)\n",enc_rval.encoded,(enc_rval.encoded+7)/8);
#endif

  if (enc_rval.encoded==-1) {
    msg("[RRC] ASN1 : SI encoding failed for SIB23\n");
    return(-1);
  }
  return((enc_rval.encoded+7)/8);
}

uint8_t do_RRCConnectionRequest(uint8_t *buffer,uint8_t *rv) {

  asn_enc_rval_t enc_rval;
  uint8_t buf[5],buf2=0;
  uint8_t ecause=0;

  UL_CCCH_Message_t ul_ccch_msg;

  RRCConnectionRequest_t *rrcConnectionRequest;

  ul_ccch_msg.message.present           = UL_CCCH_MessageType_PR_c1;
  ul_ccch_msg.message.choice.c1.present = UL_CCCH_MessageType__c1_PR_rrcConnectionRequest;
  rrcConnectionRequest          = &ul_ccch_msg.message.choice.c1.choice.rrcConnectionRequest;
  
  rrcConnectionRequest->criticalExtensions.present = RRCConnectionRequest__criticalExtensions_PR_rrcConnectionRequest_r8;
  rrcConnectionRequest->criticalExtensions.choice.rrcConnectionRequest_r8.ue_Identity.present = InitialUE_Identity_PR_randomValue;
  rrcConnectionRequest->criticalExtensions.choice.rrcConnectionRequest_r8.ue_Identity.choice.randomValue.size = 5;
  rrcConnectionRequest->criticalExtensions.choice.rrcConnectionRequest_r8.ue_Identity.choice.randomValue.bits_unused = 0;
  rrcConnectionRequest->criticalExtensions.choice.rrcConnectionRequest_r8.ue_Identity.choice.randomValue.buf = buf;
  rrcConnectionRequest->criticalExtensions.choice.rrcConnectionRequest_r8.ue_Identity.choice.randomValue.buf[0] = rv[0];
  rrcConnectionRequest->criticalExtensions.choice.rrcConnectionRequest_r8.ue_Identity.choice.randomValue.buf[1] = rv[1];
  rrcConnectionRequest->criticalExtensions.choice.rrcConnectionRequest_r8.ue_Identity.choice.randomValue.buf[2] = rv[2];
  rrcConnectionRequest->criticalExtensions.choice.rrcConnectionRequest_r8.ue_Identity.choice.randomValue.buf[3] = rv[3];
  rrcConnectionRequest->criticalExtensions.choice.rrcConnectionRequest_r8.ue_Identity.choice.randomValue.buf[4] = rv[4];


  rrcConnectionRequest->criticalExtensions.choice.rrcConnectionRequest_r8.establishmentCause = EstablishmentCause_mo_Data;

  rrcConnectionRequest->criticalExtensions.choice.rrcConnectionRequest_r8.spare.buf = &buf2;
  rrcConnectionRequest->criticalExtensions.choice.rrcConnectionRequest_r8.spare.size=1;
  rrcConnectionRequest->criticalExtensions.choice.rrcConnectionRequest_r8.spare.bits_unused = 7;

  
  enc_rval = uper_encode_to_buffer(&asn_DEF_UL_CCCH_Message,
				   (void*)&ul_ccch_msg,
				   buffer,
				   100);

#ifdef USER_MODE
  printf("[RRC][UE] RRCConnectionRequest Encoded %d bits (%d bytes), ecause %d\n",enc_rval.encoded,(enc_rval.encoded+7)/8,ecause);
#endif

  return((enc_rval.encoded+7)/8);

}

uint8_t do_RRCConnectionSetupComplete(uint8_t *buffer) {

  
  asn_enc_rval_t enc_rval;

  UL_DCCH_Message_t ul_dcch_msg;

  RRCConnectionSetupComplete_t *rrcConnectionSetupComplete;

  ul_dcch_msg.message.present           = UL_DCCH_MessageType_PR_c1;
  ul_dcch_msg.message.choice.c1.present = UL_DCCH_MessageType__c1_PR_rrcConnectionSetupComplete;
  rrcConnectionSetupComplete            = &ul_dcch_msg.message.choice.c1.choice.rrcConnectionSetupComplete;
  
  rrcConnectionSetupComplete->rrc_TransactionIdentifier = 0x2; 
  rrcConnectionSetupComplete->criticalExtensions.present = RRCConnectionSetupComplete__criticalExtensions_PR_c1;
  rrcConnectionSetupComplete->criticalExtensions.choice.c1.present = RRCConnectionSetupComplete__criticalExtensions__c1_PR_rrcConnectionSetupComplete_r8;

  rrcConnectionSetupComplete->criticalExtensions.choice.c1.choice.rrcConnectionSetupComplete_r8.nonCriticalExtension=CALLOC(1,sizeof(*rrcConnectionSetupComplete->criticalExtensions.choice.c1.choice.rrcConnectionSetupComplete_r8.nonCriticalExtension));

  rrcConnectionSetupComplete->criticalExtensions.choice.c1.choice.rrcConnectionSetupComplete_r8.selectedPLMN_Identity=2;

  rrcConnectionSetupComplete->criticalExtensions.choice.c1.choice.rrcConnectionSetupComplete_r8.registeredMME = NULL;//calloc(1,sizeof(*rrcConnectionSetupComplete->criticalExtensions.choice.c1.choice.rrcConnectionSetupComplete_r8.registeredMME));
/*
  rrcConnectionSetupComplete->criticalExtensions.choice.c1.choice.rrcConnectionSetupComplete_r8.registeredMME->plmn_Identity=NULL;
  rrcConnectionSetupComplete->criticalExtensions.choice.c1.choice.rrcConnectionSetupComplete_r8.registeredMME->mmegi.buf = calloc(2,1);
  rrcConnectionSetupComplete->criticalExtensions.choice.c1.choice.rrcConnectionSetupComplete_r8.registeredMME->mmegi.buf[0] = 0x0;
  rrcConnectionSetupComplete->criticalExtensions.choice.c1.choice.rrcConnectionSetupComplete_r8.registeredMME->mmegi.buf[1] = 0x1;
  rrcConnectionSetupComplete->criticalExtensions.choice.c1.choice.rrcConnectionSetupComplete_r8.registeredMME->mmegi.size=2;
  rrcConnectionSetupComplete->criticalExtensions.choice.c1.choice.rrcConnectionSetupComplete_r8.registeredMME->mmegi.bits_unused=0;
*/
  memset(&rrcConnectionSetupComplete->criticalExtensions.choice.c1.choice.rrcConnectionSetupComplete_r8.dedicatedInfoNAS,0,sizeof(OCTET_STRING_t));
  OCTET_STRING_fromBuf(&rrcConnectionSetupComplete->criticalExtensions.choice.c1.choice.rrcConnectionSetupComplete_r8.dedicatedInfoNAS,"NAS Info",8);

/*
  rrcConnectionSetupComplete->criticalExtensions.choice.c1.choice.rrcConnectionSetupComplete_r8.registeredMME->mmec.buf = calloc(1,1);
  rrcConnectionSetupComplete->criticalExtensions.choice.c1.choice.rrcConnectionSetupComplete_r8.registeredMME->mmec.buf[0] = 0x98;
  rrcConnectionSetupComplete->criticalExtensions.choice.c1.choice.rrcConnectionSetupComplete_r8.registeredMME->mmec.size=1;
  rrcConnectionSetupComplete->criticalExtensions.choice.c1.choice.rrcConnectionSetupComplete_r8.registeredMME->mmec.bits_unused=0;
*/



  
  enc_rval = uper_encode_to_buffer(&asn_DEF_UL_DCCH_Message,
				   (void*)&ul_dcch_msg,
				   buffer,
				   100);


#ifdef USER_MODE
  printf("RRCConnectionSetupComplete Encoded %d bits (%d bytes)\n",enc_rval.encoded,(enc_rval.encoded+7)/8);
#endif

  return((enc_rval.encoded+7)/8);

}

uint8_t do_RRCConnectionReconfigurationComplete(uint8_t *buffer) {

  
  asn_enc_rval_t enc_rval;

  UL_DCCH_Message_t ul_dcch_msg;

  RRCConnectionReconfigurationComplete_t *rrcConnectionReconfigurationComplete;

  memset((void *)&ul_dcch_msg,0,sizeof(UL_DCCH_Message_t));

  ul_dcch_msg.message.present                     = UL_DCCH_MessageType_PR_c1;
  ul_dcch_msg.message.choice.c1.present           = UL_DCCH_MessageType__c1_PR_rrcConnectionReconfigurationComplete;
  rrcConnectionReconfigurationComplete            = &ul_dcch_msg.message.choice.c1.choice.rrcConnectionReconfigurationComplete;
  
  rrcConnectionReconfigurationComplete->rrc_TransactionIdentifier = 0x2; 
  rrcConnectionReconfigurationComplete->criticalExtensions.present = RRCConnectionReconfigurationComplete__criticalExtensions_PR_rrcConnectionReconfigurationComplete_r8;
  rrcConnectionReconfigurationComplete->criticalExtensions.choice.rrcConnectionReconfigurationComplete_r8.nonCriticalExtension=NULL;

 enc_rval = uper_encode_to_buffer(&asn_DEF_UL_DCCH_Message,
				   (void*)&ul_dcch_msg,
				   buffer,
				   100);

#ifdef USER_MODE
  printf("RRCConnectionReconfigurationComplete Encoded %d bits (%d bytes)\n",enc_rval.encoded,(enc_rval.encoded+7)/8);
#endif

  return((enc_rval.encoded+7)/8);
}


uint8_t do_RRCConnectionSetup(uint8_t *buffer,
			      uint8_t UE_id,
			      uint8_t Transaction_id,
			      struct SRB_ToAddMod **SRB1_config,
			      struct SRB_ToAddMod **SRB2_config,
			      struct PhysicalConfigDedicated  **physicalConfigDedicated) {


  asn_enc_rval_t enc_rval;
  uint8_t ecause=0;
  


  long *logicalchannelgroup;
  
  struct SRB_ToAddMod *SRB1_config2,*SRB2_config2;
  struct SRB_ToAddMod__rlc_Config *SRB1_rlc_config,*SRB2_rlc_config;
  struct SRB_ToAddMod__logicalChannelConfig *SRB1_lchan_config,*SRB2_lchan_config;
  struct LogicalChannelConfig__ul_SpecificParameters *SRB1_ul_SpecificParameters,*SRB2_ul_SpecificParameters;
  SRB_ToAddModList_t *SRB_list;

  PhysicalConfigDedicated_t *physicalConfigDedicated2;

  DL_CCCH_Message_t dl_ccch_msg;

  RRCConnectionSetup_t *rrcConnectionSetup;

  memset((void *)&dl_ccch_msg,0,sizeof(DL_CCCH_Message_t));
  dl_ccch_msg.message.present           = DL_CCCH_MessageType_PR_c1;
  dl_ccch_msg.message.choice.c1.present = DL_CCCH_MessageType__c1_PR_rrcConnectionSetup;
  rrcConnectionSetup          = &dl_ccch_msg.message.choice.c1.choice.rrcConnectionSetup;

  // RRCConnectionSetup
  // Configure SRB1

  SRB_list = CALLOC(1,sizeof(*SRB_list));

  /// SRB1
  SRB1_config2 = CALLOC(1,sizeof(*SRB1_config2));
  *SRB1_config = SRB1_config2;

  SRB1_config2->srb_Identity = 1;
  SRB1_rlc_config = CALLOC(1,sizeof(*SRB1_rlc_config));
  SRB1_config2->rlc_Config   = SRB1_rlc_config;
  
  SRB1_rlc_config->present = SRB_ToAddMod__rlc_Config_PR_explicitValue;
  SRB1_rlc_config->choice.explicitValue.present=RLC_Config_PR_am;
  //assign_enum(&SRB1_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.t_PollRetransmit,T_PollRetransmit_ms45);
  SRB1_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.t_PollRetransmit=T_PollRetransmit_ms45;
 
  //assign_enum(&SRB1_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.pollPDU,PollPDU_pInfinity);
  SRB1_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.pollPDU=PollPDU_pInfinity;

  //assign_enum(&SRB1_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.pollByte,PollPDU_pInfinity);
  SRB1_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.pollByte=PollPDU_pInfinity;

  //assign_enum(&SRB1_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.maxRetxThreshold,UL_AM_RLC__maxRetxThreshold_t4);
  SRB1_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.maxRetxThreshold=UL_AM_RLC__maxRetxThreshold_t4;

  //assign_enum(&SRB1_rlc_config->choice.explicitValue.choice.am.dl_AM_RLC.t_Reordering,T_Reordering_ms35);
  SRB1_rlc_config->choice.explicitValue.choice.am.dl_AM_RLC.t_Reordering=T_Reordering_ms35;

  //assign_enum(&SRB1_rlc_config->choice.explicitValue.choice.am.dl_AM_RLC.t_StatusProhibit,T_StatusProhibit_ms0);
  SRB1_rlc_config->choice.explicitValue.choice.am.dl_AM_RLC.t_StatusProhibit=T_StatusProhibit_ms0;
  
  SRB1_lchan_config = CALLOC(1,sizeof(*SRB1_lchan_config));
  SRB1_config2->logicalChannelConfig   = SRB1_lchan_config;

  SRB1_lchan_config->present = SRB_ToAddMod__logicalChannelConfig_PR_explicitValue;
  SRB1_ul_SpecificParameters = CALLOC(1,sizeof(*SRB1_ul_SpecificParameters));

  SRB1_lchan_config->choice.explicitValue.ul_SpecificParameters = SRB1_ul_SpecificParameters;


  SRB1_ul_SpecificParameters->priority = 1;

  //assign_enum(&SRB1_ul_SpecificParameters->prioritisedBitRate,LogicalChannelConfig__ul_SpecificParameters__prioritisedBitRate_infinity);
  SRB1_ul_SpecificParameters->prioritisedBitRate=LogicalChannelConfig__ul_SpecificParameters__prioritisedBitRate_infinity;

  //assign_enum(&SRB1_ul_SpecificParameters->bucketSizeDuration,LogicalChannelConfig__ul_SpecificParameters__bucketSizeDuration_ms50);
  SRB1_ul_SpecificParameters->bucketSizeDuration=LogicalChannelConfig__ul_SpecificParameters__bucketSizeDuration_ms50;

  logicalchannelgroup = CALLOC(1,sizeof(long));
  *logicalchannelgroup=0;
  SRB1_ul_SpecificParameters->logicalChannelGroup = logicalchannelgroup;


  ASN_SEQUENCE_ADD(&SRB_list->list,SRB1_config2);

  /// SRB2
  SRB2_config2 = CALLOC(1,sizeof(*SRB2_config2));
  *SRB2_config = SRB2_config2;

  SRB2_config2->srb_Identity = 2;
  SRB2_rlc_config = CALLOC(1,sizeof(*SRB2_rlc_config));
  SRB2_config2->rlc_Config   = SRB2_rlc_config;
  
  SRB2_rlc_config->present = SRB_ToAddMod__rlc_Config_PR_explicitValue;
  SRB2_rlc_config->choice.explicitValue.present=RLC_Config_PR_am;
  //assign_enum(&SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.t_PollRetransmit,T_PollRetransmit_ms45);
  SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.t_PollRetransmit=T_PollRetransmit_ms45;
 
  //assign_enum(&SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.pollPDU,PollPDU_pInfinity);
  SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.pollPDU=PollPDU_pInfinity;

  //assign_enum(&SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.pollByte,PollPDU_pInfinity);
  SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.pollByte=PollPDU_pInfinity;

  //assign_enum(&SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.maxRetxThreshold,UL_AM_RLC__maxRetxThreshold_t4);
  SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.maxRetxThreshold=UL_AM_RLC__maxRetxThreshold_t4;

  //assign_enum(&SRB2_rlc_config->choice.explicitValue.choice.am.dl_AM_RLC.t_Reordering,T_Reordering_ms35);
  SRB2_rlc_config->choice.explicitValue.choice.am.dl_AM_RLC.t_Reordering=T_Reordering_ms35;

  //assign_enum(&SRB2_rlc_config->choice.explicitValue.choice.am.dl_AM_RLC.t_StatusProhibit,T_StatusProhibit_ms0);
  SRB2_rlc_config->choice.explicitValue.choice.am.dl_AM_RLC.t_StatusProhibit=T_StatusProhibit_ms0;
  
  SRB2_lchan_config = CALLOC(1,sizeof(*SRB2_lchan_config));
  SRB2_config2->logicalChannelConfig   = SRB2_lchan_config;

  SRB2_lchan_config->present = SRB_ToAddMod__logicalChannelConfig_PR_explicitValue;
  SRB2_ul_SpecificParameters = CALLOC(1,sizeof(*SRB2_ul_SpecificParameters));

  SRB2_lchan_config->choice.explicitValue.ul_SpecificParameters = SRB2_ul_SpecificParameters;


  SRB2_ul_SpecificParameters->priority = 1;

  //assign_enum(&SRB2_ul_SpecificParameters->prioritisedBitRate,LogicalChannelConfig__ul_SpecificParameters__prioritisedBitRate_infinity);
  SRB2_ul_SpecificParameters->prioritisedBitRate=LogicalChannelConfig__ul_SpecificParameters__prioritisedBitRate_infinity;

  //assign_enum(&SRB2_ul_SpecificParameters->bucketSizeDuration,LogicalChannelConfig__ul_SpecificParameters__bucketSizeDuration_ms50);
  SRB2_ul_SpecificParameters->bucketSizeDuration=LogicalChannelConfig__ul_SpecificParameters__bucketSizeDuration_ms50;

  logicalchannelgroup = CALLOC(1,sizeof(long));
  *logicalchannelgroup=0;
  SRB2_ul_SpecificParameters->logicalChannelGroup = logicalchannelgroup;


  ASN_SEQUENCE_ADD(&SRB_list->list,SRB2_config2);

  // PhysicalConfigDedicated

  physicalConfigDedicated2 = CALLOC(1,sizeof(*physicalConfigDedicated2));
  *physicalConfigDedicated = physicalConfigDedicated2;

  physicalConfigDedicated2->pdsch_ConfigDedicated         = CALLOC(1,sizeof(*physicalConfigDedicated2->pdsch_ConfigDedicated));
  physicalConfigDedicated2->pucch_ConfigDedicated         = CALLOC(1,sizeof(*physicalConfigDedicated2->pucch_ConfigDedicated));
  physicalConfigDedicated2->pusch_ConfigDedicated         = CALLOC(1,sizeof(*physicalConfigDedicated2->pusch_ConfigDedicated));
  physicalConfigDedicated2->uplinkPowerControlDedicated   = CALLOC(1,sizeof(*physicalConfigDedicated2->uplinkPowerControlDedicated));  
  physicalConfigDedicated2->tpc_PDCCH_ConfigPUCCH         = CALLOC(1,sizeof(*physicalConfigDedicated2->tpc_PDCCH_ConfigPUCCH));
  physicalConfigDedicated2->tpc_PDCCH_ConfigPUSCH         = CALLOC(1,sizeof(*physicalConfigDedicated2->tpc_PDCCH_ConfigPUSCH));
  physicalConfigDedicated2->cqi_ReportConfig              = NULL;//CALLOC(1,sizeof(*physicalConfigDedicated2->cqi_ReportConfig));
  physicalConfigDedicated2->soundingRS_UL_ConfigDedicated = NULL;//CALLOC(1,sizeof(*physicalConfigDedicated2->soundingRS_UL_ConfigDedicated));
  physicalConfigDedicated2->antennaInfo                   = CALLOC(1,sizeof(*physicalConfigDedicated2->antennaInfo));
  physicalConfigDedicated2->schedulingRequestConfig       = CALLOC(1,sizeof(*physicalConfigDedicated2->schedulingRequestConfig));
  physicalConfigDedicated2->pusch_CAConfigDedicated_vlola = NULL;
  // PDSCH
  //assign_enum(&physicalConfigDedicated2->pdsch_ConfigDedicated->p_a,
  //	      PDSCH_ConfigDedicated__p_a_dB0);
  physicalConfigDedicated2->pdsch_ConfigDedicated->p_a=   PDSCH_ConfigDedicated__p_a_dB0;
  // PUCCH
  physicalConfigDedicated2->pucch_ConfigDedicated->ackNackRepetition.present=PUCCH_ConfigDedicated__ackNackRepetition_PR_release;
  physicalConfigDedicated2->pucch_ConfigDedicated->ackNackRepetition.choice.release=0;
  physicalConfigDedicated2->pucch_ConfigDedicated->tdd_AckNackFeedbackMode=NULL;//PUCCH_ConfigDedicated__tdd_AckNackFeedbackMode_multiplexing;

  // Pusch_config_dedicated
  physicalConfigDedicated2->pusch_ConfigDedicated->betaOffset_ACK_Index = 0; // 2.00
  physicalConfigDedicated2->pusch_ConfigDedicated->betaOffset_RI_Index  = 0; // 1.25
  physicalConfigDedicated2->pusch_ConfigDedicated->betaOffset_CQI_Index = 8; // 2.25

  // UplinkPowerControlDedicated
  physicalConfigDedicated2->uplinkPowerControlDedicated->p0_UE_PUSCH = 0; // 0 dB
  //assign_enum(&physicalConfigDedicated2->uplinkPowerControlDedicated->deltaMCS_Enabled,
  // UplinkPowerControlDedicated__deltaMCS_Enabled_en1);
  physicalConfigDedicated2->uplinkPowerControlDedicated->deltaMCS_Enabled= UplinkPowerControlDedicated__deltaMCS_Enabled_en1;
  physicalConfigDedicated2->uplinkPowerControlDedicated->accumulationEnabled = 0;  // FALSE
  physicalConfigDedicated2->uplinkPowerControlDedicated->p0_UE_PUCCH = 0; // 0 dB
  physicalConfigDedicated2->uplinkPowerControlDedicated->pSRS_Offset = 0; // 0 dB
  physicalConfigDedicated2->uplinkPowerControlDedicated->filterCoefficient = CALLOC(1,sizeof(*physicalConfigDedicated2->uplinkPowerControlDedicated->filterCoefficient));
  //  assign_enum(physicalConfigDedicated2->uplinkPowerControlDedicated->filterCoefficient,FilterCoefficient_fc4); // fc4 dB
  *physicalConfigDedicated2->uplinkPowerControlDedicated->filterCoefficient=FilterCoefficient_fc4; // fc4 dB

  // TPC-PDCCH-Config

  physicalConfigDedicated2->tpc_PDCCH_ConfigPUCCH->present=TPC_PDCCH_Config_PR_setup;
  physicalConfigDedicated2->tpc_PDCCH_ConfigPUCCH->choice.setup.tpc_Index.present = TPC_Index_PR_indexOfFormat3;
  physicalConfigDedicated2->tpc_PDCCH_ConfigPUCCH->choice.setup.tpc_Index.choice.indexOfFormat3 = 1;
  physicalConfigDedicated2->tpc_PDCCH_ConfigPUCCH->choice.setup.tpc_RNTI.buf=CALLOC(1,2);
  physicalConfigDedicated2->tpc_PDCCH_ConfigPUCCH->choice.setup.tpc_RNTI.size=2;
  physicalConfigDedicated2->tpc_PDCCH_ConfigPUCCH->choice.setup.tpc_RNTI.buf[0]=0x12;
  physicalConfigDedicated2->tpc_PDCCH_ConfigPUCCH->choice.setup.tpc_RNTI.buf[1]=0x34+UE_id;
  physicalConfigDedicated2->tpc_PDCCH_ConfigPUCCH->choice.setup.tpc_RNTI.bits_unused=0;

  physicalConfigDedicated2->tpc_PDCCH_ConfigPUSCH->present=TPC_PDCCH_Config_PR_setup;
  physicalConfigDedicated2->tpc_PDCCH_ConfigPUSCH->choice.setup.tpc_Index.present = TPC_Index_PR_indexOfFormat3;
  physicalConfigDedicated2->tpc_PDCCH_ConfigPUSCH->choice.setup.tpc_Index.choice.indexOfFormat3 = 1;
  physicalConfigDedicated2->tpc_PDCCH_ConfigPUSCH->choice.setup.tpc_RNTI.buf=CALLOC(1,2);
  physicalConfigDedicated2->tpc_PDCCH_ConfigPUSCH->choice.setup.tpc_RNTI.size=2;
  physicalConfigDedicated2->tpc_PDCCH_ConfigPUSCH->choice.setup.tpc_RNTI.buf[0]=0x22;
  physicalConfigDedicated2->tpc_PDCCH_ConfigPUSCH->choice.setup.tpc_RNTI.buf[1]=0x34+UE_id;
  physicalConfigDedicated2->tpc_PDCCH_ConfigPUSCH->choice.setup.tpc_RNTI.bits_unused=0;

  // CQI ReportConfig
  /*
  physicalConfigDedicated2->cqi_ReportConfig->cqi_ReportModeAperiodic=CALLOC(1,sizeof(*physicalConfigDedicated2->cqi_ReportConfig->cqi_ReportModeAperiodic));
  assign_enum(physicalConfigDedicated2->cqi_ReportConfig->cqi_ReportModeAperiodic,
	      CQI_ReportConfig__cqi_ReportModeAperiodic_rm30); // HLC CQI, no PMI
  physicalConfigDedicated2->cqi_ReportConfig->nomPDSCH_RS_EPRE_Offset = 0; // 0 dB
  physicalConfigDedicated2->cqi_ReportConfig->cqi_ReportPeriodic=CALLOC(1,sizeof(*physicalConfigDedicated2->cqi_ReportConfig->cqi_ReportPeriodic));
  physicalConfigDedicated2->cqi_ReportConfig->cqi_ReportPeriodic->present =  CQI_ReportPeriodic_PR_setup;
  physicalConfigDedicated2->cqi_ReportConfig->cqi_ReportPeriodic->choice.setup.cqi_PUCCH_ResourceIndex = 0;  // n2_pucch
  physicalConfigDedicated2->cqi_ReportConfig->cqi_ReportPeriodic->choice.setup.cqi_pmi_ConfigIndex = 0;  // Icqi/pmi
  physicalConfigDedicated2->cqi_ReportConfig->cqi_ReportPeriodic->choice.setup.cqi_FormatIndicatorPeriodic.present = CQI_ReportPeriodic__setup__cqi_FormatIndicatorPeriodic_PR_subbandCQI;  // subband CQI 
  physicalConfigDedicated2->cqi_ReportConfig->cqi_ReportPeriodic->choice.setup.cqi_FormatIndicatorPeriodic.choice.subbandCQI.k=4;

  physicalConfigDedicated2->cqi_ReportConfig->cqi_ReportPeriodic->choice.setup.ri_ConfigIndex=NULL;
  physicalConfigDedicated2->cqi_ReportConfig->cqi_ReportPeriodic->choice.setup.simultaneousAckNackAndCQI=0;
  */

  //soundingRS-UL-ConfigDedicated
  /*
  physicalConfigDedicated2->soundingRS_UL_ConfigDedicated->present = SoundingRS_UL_ConfigDedicated_PR_setup;
  assign_enum(&physicalConfigDedicated2->soundingRS_UL_ConfigDedicated->choice.setup.srs_Bandwidth,
	      SoundingRS_UL_ConfigDedicated__setup__srs_Bandwidth_bw0);
  assign_enum(&physicalConfigDedicated2->soundingRS_UL_ConfigDedicated->choice.setup.srs_HoppingBandwidth,
	      SoundingRS_UL_ConfigDedicated__setup__srs_HoppingBandwidth_hbw0);
  physicalConfigDedicated2->soundingRS_UL_ConfigDedicated->choice.setup.freqDomainPosition=0;
  physicalConfigDedicated2->soundingRS_UL_ConfigDedicated->choice.setup.duration=1;
  physicalConfigDedicated2->soundingRS_UL_ConfigDedicated->choice.setup.srs_ConfigIndex=1;
  physicalConfigDedicated2->soundingRS_UL_ConfigDedicated->choice.setup.transmissionComb=0;
  assign_enum(&physicalConfigDedicated2->soundingRS_UL_ConfigDedicated->choice.setup.cyclicShift,
	      SoundingRS_UL_ConfigDedicated__setup__cyclicShift_cs0);
  */


  //AntennaInfoDedicated
  physicalConfigDedicated2->antennaInfo = CALLOC(1,sizeof(*physicalConfigDedicated2->antennaInfo));
  physicalConfigDedicated2->antennaInfo->present = PhysicalConfigDedicated__antennaInfo_PR_explicitValue;
  //assign_enum(&physicalConfigDedicated2->antennaInfo->choice.explicitValue.transmissionMode,
  //     AntennaInfoDedicated__transmissionMode_tm2);

  // TODO: set transmission mode based on some external config
  // for the moment use transmission_mode_rrc
  //physicalConfigDedicated2->antennaInfo->choice.explicitValue.transmissionMode=     AntennaInfoDedicated__transmissionMode_tm2;
  
  switch (transmission_mode_rrc){
  case 1:
    physicalConfigDedicated2->antennaInfo->choice.explicitValue.transmissionMode=     AntennaInfoDedicated__transmissionMode_tm1;
    break;
  case 2:
    physicalConfigDedicated2->antennaInfo->choice.explicitValue.transmissionMode=     AntennaInfoDedicated__transmissionMode_tm2;
    break;
  case 4:
    physicalConfigDedicated2->antennaInfo->choice.explicitValue.transmissionMode=     AntennaInfoDedicated__transmissionMode_tm4;
    break;
  case 5:
    physicalConfigDedicated2->antennaInfo->choice.explicitValue.transmissionMode=     AntennaInfoDedicated__transmissionMode_tm5;
    break;
  case 6:
    physicalConfigDedicated2->antennaInfo->choice.explicitValue.transmissionMode=     AntennaInfoDedicated__transmissionMode_tm6;
    break;
  }
  
 
  physicalConfigDedicated2->antennaInfo->choice.explicitValue.ue_TransmitAntennaSelection.present = AntennaInfoDedicated__ue_TransmitAntennaSelection_PR_release;
  physicalConfigDedicated2->antennaInfo->choice.explicitValue.ue_TransmitAntennaSelection.choice.release = 0;

  // SchedulingRequestConfig

  physicalConfigDedicated2->schedulingRequestConfig->present = SchedulingRequestConfig_PR_setup;
  physicalConfigDedicated2->schedulingRequestConfig->choice.setup.sr_PUCCH_ResourceIndex = UE_id;
  physicalConfigDedicated2->schedulingRequestConfig->choice.setup.sr_ConfigIndex = 7+(UE_id%3);  // Isr = 5 (every 10 subframes, offset=2+UE_id mod3)
  //  assign_enum(&physicalConfigDedicated2->schedulingRequestConfig->choice.setup.dsr_TransMax,
  //SchedulingRequestConfig__setup__dsr_TransMax_n4);
  //  assign_enum(&physicalConfigDedicated2->schedulingRequestConfig->choice.setup.dsr_TransMax = SchedulingRequestConfig__setup__dsr_TransMax_n4;
  physicalConfigDedicated2->schedulingRequestConfig->choice.setup.dsr_TransMax = SchedulingRequestConfig__setup__dsr_TransMax_n4;
 
  rrcConnectionSetup->rrc_TransactionIdentifier = Transaction_id;
  rrcConnectionSetup->criticalExtensions.present = RRCConnectionSetup__criticalExtensions_PR_c1;
  rrcConnectionSetup->criticalExtensions.choice.c1.present =RRCConnectionSetup__criticalExtensions__c1_PR_rrcConnectionSetup_r8 ;
  rrcConnectionSetup->criticalExtensions.choice.c1.choice.rrcConnectionSetup_r8.radioResourceConfigDedicated.srb_ToAddModList = SRB_list;
  rrcConnectionSetup->criticalExtensions.choice.c1.choice.rrcConnectionSetup_r8.radioResourceConfigDedicated.drb_ToAddModList = NULL;
  rrcConnectionSetup->criticalExtensions.choice.c1.choice.rrcConnectionSetup_r8.radioResourceConfigDedicated.drb_ToReleaseList = NULL;
  rrcConnectionSetup->criticalExtensions.choice.c1.choice.rrcConnectionSetup_r8.radioResourceConfigDedicated.sps_Config = NULL;
  rrcConnectionSetup->criticalExtensions.choice.c1.choice.rrcConnectionSetup_r8.radioResourceConfigDedicated.physicalConfigDedicated = physicalConfigDedicated2;
  rrcConnectionSetup->criticalExtensions.choice.c1.choice.rrcConnectionSetup_r8.radioResourceConfigDedicated.mac_MainConfig = NULL;
  rrcConnectionSetup->criticalExtensions.choice.c1.choice.rrcConnectionSetup_r8.radioResourceConfigDedicated.sps_RA_ConfigList_rlola = NULL;



  enc_rval = uper_encode_to_buffer(&asn_DEF_DL_CCCH_Message,
				   (void*)&dl_ccch_msg,
				   buffer,
				   100);

#ifdef USER_MODE
  printf("RRCConnectionSetup Encoded %d bits (%d bytes), ecause %d\n",enc_rval.encoded,(enc_rval.encoded+7)/8,ecause);
#endif

  FREEMEM(SRB_list);
  //  free(SRB1_config);
  //  free(SRB1_rlc_config);
  //  free(SRB1_lchan_config);
  //  free(SRB1_ul_SpecificParameters);

  return((enc_rval.encoded+7)/8);
}


uint8_t do_RRCConnectionReconfiguration(uint8_t *buffer,
			      uint8_t UE_id,
			      uint8_t Transaction_id,
			      struct SRB_ToAddMod **SRB2_config,
			      struct DRB_ToAddMod **DRB_config,
			      struct PhysicalConfigDedicated  **physicalConfigDedicated) {

  asn_enc_rval_t enc_rval;
  struct SRB_ToAddMod *SRB2_config2;
  struct SRB_ToAddMod__rlc_Config *SRB2_rlc_config;
  struct SRB_ToAddMod__logicalChannelConfig *SRB2_lchan_config;
  struct LogicalChannelConfig__ul_SpecificParameters *SRB2_ul_SpecificParameters;
  SRB_ToAddModList_t *SRB_list;

  struct DRB_ToAddMod *DRB_config2;
  struct RLC_Config *DRB_rlc_config;
  struct LogicalChannelConfig *DRB_lchan_config;
  struct LogicalChannelConfig__ul_SpecificParameters *DRB_ul_SpecificParameters;
  DRB_ToAddModList_t *DRB_list;

  long *logicalchannelgroup,*logicalchannelgroup_drb;

  DL_DCCH_Message_t dl_dcch_msg;

  RRCConnectionReconfiguration_t *rrcConnectionReconfiguration;
  long *lcid;

  //  int i;

  dl_dcch_msg.message.present           = DL_DCCH_MessageType_PR_c1;
  dl_dcch_msg.message.choice.c1.present = DL_DCCH_MessageType__c1_PR_rrcConnectionReconfiguration;
  rrcConnectionReconfiguration          = &dl_dcch_msg.message.choice.c1.choice.rrcConnectionReconfiguration;

  // RRCConnectionReconfiguration
  // Configure SRB2

  SRB_list = CALLOC(1,sizeof(*SRB_list));

  /// SRB2
  SRB2_config2 = CALLOC(1,sizeof(*SRB2_config2));
  *SRB2_config = SRB2_config2;

  SRB2_config2->srb_Identity = 1;
  SRB2_rlc_config = CALLOC(1,sizeof(*SRB2_rlc_config));
  SRB2_config2->rlc_Config   = SRB2_rlc_config;
  
  SRB2_rlc_config->present = SRB_ToAddMod__rlc_Config_PR_explicitValue;
  SRB2_rlc_config->choice.explicitValue.present=RLC_Config_PR_am;
  /*  assign_enum(&SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.t_PollRetransmit,T_PollRetransmit_ms45);
  assign_enum(&SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.pollPDU,PollPDU_pInfinity);
  assign_enum(&SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.pollByte,PollPDU_pInfinity);
  assign_enum(&SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.maxRetxThreshold,UL_AM_RLC__maxRetxThreshold_t4);
  assign_enum(&SRB2_rlc_config->choice.explicitValue.choice.am.dl_AM_RLC.t_Reordering,T_Reordering_ms35);
  assign_enum(&SRB2_rlc_config->choice.explicitValue.choice.am.dl_AM_RLC.t_StatusProhibit,T_StatusProhibit_ms0);
  */
  SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.t_PollRetransmit=T_PollRetransmit_ms45;
  SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.pollPDU=PollPDU_pInfinity;
  SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.pollByte=PollPDU_pInfinity;
  SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.maxRetxThreshold=UL_AM_RLC__maxRetxThreshold_t4;
  SRB2_rlc_config->choice.explicitValue.choice.am.dl_AM_RLC.t_Reordering=T_Reordering_ms35;
  SRB2_rlc_config->choice.explicitValue.choice.am.dl_AM_RLC.t_StatusProhibit=T_StatusProhibit_ms0;
  

  SRB2_lchan_config = CALLOC(1,sizeof(*SRB2_lchan_config));
  SRB2_config2->logicalChannelConfig   = SRB2_lchan_config;

  SRB2_lchan_config->present = SRB_ToAddMod__logicalChannelConfig_PR_explicitValue;
  SRB2_ul_SpecificParameters = CALLOC(1,sizeof(*SRB2_ul_SpecificParameters));

  SRB2_lchan_config->choice.explicitValue.ul_SpecificParameters = SRB2_ul_SpecificParameters;


  SRB2_ul_SpecificParameters->priority = 1;

  //assign_enum(&SRB2_ul_SpecificParameters->prioritisedBitRate,LogicalChannelConfig__ul_SpecificParameters__prioritisedBitRate_infinity);
  SRB2_ul_SpecificParameters->prioritisedBitRate=LogicalChannelConfig__ul_SpecificParameters__prioritisedBitRate_infinity;
  //  assign_enum(&SRB2_ul_SpecificParameters->bucketSizeDuration,LogicalChannelConfig__ul_SpecificParameters__bucketSizeDuration_ms50);
  SRB2_ul_SpecificParameters->bucketSizeDuration=LogicalChannelConfig__ul_SpecificParameters__bucketSizeDuration_ms50;

  logicalchannelgroup = CALLOC(1,sizeof(long));
  *logicalchannelgroup=0;

  SRB2_ul_SpecificParameters->logicalChannelGroup = logicalchannelgroup;


  ASN_SEQUENCE_ADD(&SRB_list->list,SRB2_config2);

  // Configure DRB

  DRB_list = CALLOC(1,sizeof(*DRB_list));

  /// DRB
  DRB_config2 = CALLOC(1,sizeof(*DRB_config2));
  *DRB_config = DRB_config2;

  DRB_config2->drb_Identity = 1;
  lcid = CALLOC(1,sizeof(*lcid));
  *lcid = 3;
  DRB_config2->logicalChannelIdentity = lcid;
  DRB_rlc_config = CALLOC(1,sizeof(*DRB_rlc_config));
  DRB_config2->rlc_Config   = DRB_rlc_config;
  
  DRB_rlc_config->present=RLC_Config_PR_um_Bi_Directional;
  /*
  assign_enum(&DRB_rlc_config->choice.um_Bi_Directional.ul_UM_RLC.sn_FieldLength,SN_FieldLength_size5);
  assign_enum(&DRB_rlc_config->choice.um_Bi_Directional.dl_UM_RLC.sn_FieldLength,SN_FieldLength_size5);
  assign_enum(&DRB_rlc_config->choice.um_Bi_Directional.dl_UM_RLC.t_Reordering,T_Reordering_ms35);
  */
  DRB_rlc_config->choice.um_Bi_Directional.ul_UM_RLC.sn_FieldLength=SN_FieldLength_size5;
  DRB_rlc_config->choice.um_Bi_Directional.dl_UM_RLC.sn_FieldLength=SN_FieldLength_size5;
  DRB_rlc_config->choice.um_Bi_Directional.dl_UM_RLC.t_Reordering=T_Reordering_ms35;
  DRB_lchan_config = CALLOC(1,sizeof(*DRB_lchan_config));
  DRB_config2->logicalChannelConfig   = DRB_lchan_config;

  DRB_ul_SpecificParameters = CALLOC(1,sizeof(*DRB_ul_SpecificParameters));

  DRB_lchan_config->ul_SpecificParameters = DRB_ul_SpecificParameters;


  DRB_ul_SpecificParameters->priority = 2; // lower priority than srb1, srb2

  //  assign_enum(&DRB_ul_SpecificParameters->prioritisedBitRate,LogicalChannelConfig__ul_SpecificParameters__prioritisedBitRate_infinity);
  DRB_ul_SpecificParameters->prioritisedBitRate=LogicalChannelConfig__ul_SpecificParameters__prioritisedBitRate_infinity;
  //  assign_enum(&DRB_ul_SpecificParameters->bucketSizeDuration,LogicalChannelConfig__ul_SpecificParameters__bucketSizeDuration_ms50);
  DRB_ul_SpecificParameters->bucketSizeDuration=LogicalChannelConfig__ul_SpecificParameters__bucketSizeDuration_ms50;

  logicalchannelgroup_drb = CALLOC(1,sizeof(long));
  *logicalchannelgroup_drb=0;
  DRB_ul_SpecificParameters->logicalChannelGroup = logicalchannelgroup_drb;


  ASN_SEQUENCE_ADD(&DRB_list->list,DRB_config2);


  
  rrcConnectionReconfiguration->rrc_TransactionIdentifier = Transaction_id;
  rrcConnectionReconfiguration->criticalExtensions.present = RRCConnectionReconfiguration__criticalExtensions_PR_c1;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.present =RRCConnectionReconfiguration__criticalExtensions__c1_PR_rrcConnectionReconfiguration_r8 ;

  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated = CALLOC(1,sizeof(*rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated));
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->srb_ToAddModList = SRB_list;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->drb_ToAddModList = DRB_list;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->drb_ToReleaseList = NULL;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->sps_Config = NULL;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->physicalConfigDedicated = NULL;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig = NULL;
  /*  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig = CALLOC(1,sizeof(*rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig));
  
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig->present =RadioResourceConfigDedicated__mac_MainConfig_PR_explicitValue;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig->choice.explicitValue.ul_SCH_Config = CALLOC(1,sizeof(*rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig->choice.explicitValue.ul_SCH_Config));
  
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig->choice.explicitValue.ul_SCH_Config->maxHARQ_Tx = MAC_MainConfig__ul_SCH_Config__maxHARQ_Tx_n5;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig->choice.explicitValue.ul_SCH_Config->periodicBSR_Timer =  MAC_MainConfig__ul_SCH_Config__periodicBSR_Timer_sf64;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig->choice.explicitValue.ul_SCH_Config->retxBSR_Timer =  MAC_MainConfig__ul_SCH_Config__retxBSR_Timer_sf320;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig->choice.explicitValue.ul_SCH_Config->ttiBundling=0;
  */
 rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->sps_RA_ConfigList_rlola = NULL;

  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig           = NULL;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.mobilityControlInfo  = NULL;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.dedicatedInfoNASList = NULL;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.securityConfigHO     = NULL;

  enc_rval = uper_encode_to_buffer(&asn_DEF_DL_DCCH_Message,
				   (void*)&dl_dcch_msg,
				   buffer,
				   100);

  //#ifdef USER_MODE
  msg("RRCConnectionReconfiguration Encoded %d bits (%d bytes)\n",enc_rval.encoded,(enc_rval.encoded+7)/8);
  // for (i=0;i<30;i++)
  //    msg("%x.",buffer[i]);
  // msg("\n");
  
  //#endif

  FREEMEM(SRB_list);
  FREEMEM(DRB_list);

  return((enc_rval.encoded+7)/8);
}

#ifndef USER_MODE
int init_module(void)
{
   printk("Init asn1_msg module\n");
        
   // A non 0 return means init_module failed; module can't be loaded.
   return 0;
}


void cleanup_module(void)
{
  printk("Stopping asn1_msg module\n");
} 

EXPORT_SYMBOL(do_SIB1);
EXPORT_SYMBOL(do_SIB23);
EXPORT_SYMBOL(do_RRCConnectionRequest);
EXPORT_SYMBOL(do_RRCConnectionSetupComplete);
EXPORT_SYMBOL(do_RRCConnectionReconfigurationComplete);
EXPORT_SYMBOL(do_RRCConnectionSetup);
EXPORT_SYMBOL(do_RRCConnectionReconfiguration);
EXPORT_SYMBOL(asn_DEF_UL_DCCH_Message);
EXPORT_SYMBOL(asn_DEF_UL_CCCH_Message);
EXPORT_SYMBOL(asn_DEF_SystemInformation);
EXPORT_SYMBOL(asn_DEF_DL_DCCH_Message);
EXPORT_SYMBOL(asn_DEF_SystemInformationBlockType1);
EXPORT_SYMBOL(asn_DEF_DL_CCCH_Message);
EXPORT_SYMBOL(uper_decode_complete);
EXPORT_SYMBOL(uper_decode);
#endif
