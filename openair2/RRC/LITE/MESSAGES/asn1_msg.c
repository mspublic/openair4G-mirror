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
#ifdef USER_MODE
//#include "RRC/LITE/defs.h"
//#include "COMMON/mac_rrc_primitives.h"
#include "UTIL/LOG/log.h"
#endif
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
#ifdef Rel10
#include "MCCH-Message.h"
#define MRB1 1
#endif

#include "RRC/LITE/defs.h"
#include "RRCConnectionSetupComplete.h"
#include "RRCConnectionReconfigurationComplete.h"
#include "RRCConnectionReconfiguration.h"
#include "MasterInformationBlock.h"
#include "SystemInformation.h"

#include "SystemInformationBlockType1.h"

#include "SIB-Type.h"

#include "BCCH-DL-SCH-Message.h"

#include "PHY/defs.h"

#include "MeasObjectToAddModList.h"
#include "ReportConfigToAddModList.h"
#include "MeasIdToAddModList.h"

//#include "PHY/defs.h"
#ifndef USER_MODE
#define msg printk
#ifndef errno
int errno;
#endif
#else
#define msg printf
#endif

uint16_t two_tier_hexagonal_cellIds[7] = {0,1,2,4,5,7,8};
uint16_t two_tier_hexagonal_adjacent_cellIds[7][6] = {{1,2,4,5,7,8},    // CellId 0
				                      {11,18,2,0,8,15}, // CellId 1
				                      {18,13,3,4,0,1},  // CellId 2
				                      {2,3,14,6,5,0},   // CellId 4
				                      {0,4,6,16,9,7},   // CellId 5
				                      {8,0,5,9,17,12},  // CellId 7
				                      {15,1,0,7,12,10}};// CellId 8


uint16_t get_adjacent_cell_id(uint8_t Mod_id,uint8_t index) {
  return(two_tier_hexagonal_adjacent_cellIds[Mod_id][index]);
}

/*
uint8_t do_SIB1(LTE_DL_FRAME_PARMS *frame_parms, uint8_t *buffer,
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
  sib1->tdd_Config->subframeAssignment=frame_parms->tdd_config; //TDD_Config__subframeAssignment_sa3;

  //  assign_enum(&sib1->tdd_Config->specialSubframePatterns,TDD_Config__specialSubframePatterns_ssp0);
  sib1->tdd_Config->specialSubframePatterns=frame_parms->tdd_config_S;//TDD_Config__specialSubframePatterns_ssp0;

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
  LOG_D(RRC,"[eNB] SystemInformationBlockType1 Encoded %d bits (%d bytes)\n",enc_rval.encoded,(enc_rval.encoded+7)/8);
#endif

  if (enc_rval.encoded==-1)
    return(-1);
  return((enc_rval.encoded+7)/8);
}
*/
// AT4 packet
uint8_t do_MIB(LTE_DL_FRAME_PARMS *frame_parms, uint32_t frame, uint8_t *buffer) {

  asn_enc_rval_t enc_rval;
  BCCH_BCH_Message_t mib;
  uint8_t sfn = (uint8_t)((frame>>2)&0xff);
  uint16_t spare=0;

  switch (frame_parms->N_RB_DL) {

  case 6:
    mib.message.dl_Bandwidth = MasterInformationBlock__dl_Bandwidth_n6;
    break;
  case 15:
    mib.message.dl_Bandwidth = MasterInformationBlock__dl_Bandwidth_n15;
    break;
  case 25:
    mib.message.dl_Bandwidth = MasterInformationBlock__dl_Bandwidth_n25;
    break;
  case 50:
    mib.message.dl_Bandwidth = MasterInformationBlock__dl_Bandwidth_n50;
    break;
  case 75:
    mib.message.dl_Bandwidth = MasterInformationBlock__dl_Bandwidth_n75;
    break;
  case 100:
    mib.message.dl_Bandwidth = MasterInformationBlock__dl_Bandwidth_n100;
    break;
  default:
    mib.message.dl_Bandwidth = MasterInformationBlock__dl_Bandwidth_n6;
    break;
  }
  switch (frame_parms->phich_config_common.phich_resource) {
  case oneSixth:
    mib.message.phich_Config.phich_Resource = 0;
    break;
  case half:
    mib.message.phich_Config.phich_Resource = 1;
    break;
  case one:
    mib.message.phich_Config.phich_Resource = 2;
    break;
  case two:
    mib.message.phich_Config.phich_Resource = 3;
    break;
  }

  printf("[MIB] systemBandwidth %x, phich_duration %x, phich_resource %x,sfn %x\n",  
	 (uint32_t)mib.message.dl_Bandwidth,
	 (uint32_t)frame_parms->phich_config_common.phich_duration,
	 (uint32_t)mib.message.phich_Config.phich_Resource,
	 (uint32_t)sfn);
  mib.message.phich_Config.phich_Duration = frame_parms->phich_config_common.phich_duration;
  mib.message.systemFrameNumber.buf = &sfn;
  mib.message.systemFrameNumber.size = 1;
  mib.message.systemFrameNumber.bits_unused=0;
  mib.message.spare.buf = (uint8_t *)&spare;
  mib.message.spare.size = 2;
  mib.message.spare.bits_unused = 6;  // This makes a spare of 10 bits

  enc_rval = uper_encode_to_buffer(&asn_DEF_BCCH_BCH_Message,
				   (void*)&mib,
				   buffer,
				   100);
  if (enc_rval.encoded==-1)
    return(-1);
  return((enc_rval.encoded+7)/8);
  /*
  printf("MIB: %x ((MIB>>10)&63)+(MIB&3<<6)=SFN %x, MIB>>2&3 = phich_resource %d, MIB>>4&1 = phich_duration %d, MIB>>5&7 = system_bandwidth %d)\n",*(uint32_t *)buffer,
	 (((*(uint32_t *)buffer)>>10)&0x3f)+(((*(uint32_t *)buffer)&3)<<6),
	 ((*(uint32_t *)buffer)>>2)&0x3,
	 ((*(uint32_t *)buffer)>>4)&0x1,
	 ((*(uint32_t *)buffer)>>5)&0x7
	 );
  */
}
uint8_t do_SIB1(LTE_DL_FRAME_PARMS *frame_parms, uint8_t *buffer,
		BCCH_DL_SCH_Message_t *bcch_message,
		SystemInformationBlockType1_t **sib1) {

  //  SystemInformation_t systemInformation;
  PLMN_IdentityInfo_t PLMN_identity_info;
  MCC_MNC_Digit_t dummy_mcc[3],dummy_mnc[2];
  asn_enc_rval_t enc_rval;
  SchedulingInfo_t schedulingInfo;
  SIB_Type_t sib_type;

  memset(bcch_message,0,sizeof(BCCH_DL_SCH_Message_t));
  bcch_message->message.present = BCCH_DL_SCH_MessageType_PR_c1;
  bcch_message->message.choice.c1.present = BCCH_DL_SCH_MessageType__c1_PR_systemInformationBlockType1;
  //  memcpy(&bcch_message.message.choice.c1.choice.systemInformationBlockType1,sib1,sizeof(SystemInformationBlockType1_t));

  *sib1 = &bcch_message->message.choice.c1.choice.systemInformationBlockType1;

  memset(&PLMN_identity_info,0,sizeof(PLMN_IdentityInfo_t));
  memset(&schedulingInfo,0,sizeof(SchedulingInfo_t));
  memset(&sib_type,0,sizeof(SIB_Type_t));



  PLMN_identity_info.plmn_Identity.mcc = CALLOC(1,sizeof(*PLMN_identity_info.plmn_Identity.mcc));
  memset(PLMN_identity_info.plmn_Identity.mcc,0,sizeof(*PLMN_identity_info.plmn_Identity.mcc));

  asn_set_empty(&PLMN_identity_info.plmn_Identity.mcc->list);//.size=0;

  dummy_mcc[0]=0;ASN_SEQUENCE_ADD(&PLMN_identity_info.plmn_Identity.mcc->list,&dummy_mcc[0]);
  dummy_mcc[1]=0;ASN_SEQUENCE_ADD(&PLMN_identity_info.plmn_Identity.mcc->list,&dummy_mcc[1]);
  dummy_mcc[2]=1;ASN_SEQUENCE_ADD(&PLMN_identity_info.plmn_Identity.mcc->list,&dummy_mcc[2]);

  PLMN_identity_info.plmn_Identity.mnc.list.size=0;
  PLMN_identity_info.plmn_Identity.mnc.list.count=0;
  dummy_mnc[0]=1;ASN_SEQUENCE_ADD(&PLMN_identity_info.plmn_Identity.mnc.list,&dummy_mnc[0]);
  dummy_mnc[1]=2;ASN_SEQUENCE_ADD(&PLMN_identity_info.plmn_Identity.mnc.list,&dummy_mnc[1]);
  //assign_enum(&PLMN_identity_info.cellReservedForOperatorUse,PLMN_IdentityInfo__cellReservedForOperatorUse_notReserved);
  PLMN_identity_info.cellReservedForOperatorUse=PLMN_IdentityInfo__cellReservedForOperatorUse_notReserved;

  ASN_SEQUENCE_ADD(&(*sib1)->cellAccessRelatedInfo.plmn_IdentityList.list,&PLMN_identity_info);


  asn_set_empty(&PLMN_identity_info.plmn_Identity.mcc->list);//.size=0;

  dummy_mcc[0]=0;ASN_SEQUENCE_ADD(&PLMN_identity_info.plmn_Identity.mcc->list,&dummy_mcc[0]);
  dummy_mcc[1]=0;ASN_SEQUENCE_ADD(&PLMN_identity_info.plmn_Identity.mcc->list,&dummy_mcc[1]);
  dummy_mcc[2]=1;ASN_SEQUENCE_ADD(&PLMN_identity_info.plmn_Identity.mcc->list,&dummy_mcc[2]);

  PLMN_identity_info.plmn_Identity.mnc.list.size=0;
  PLMN_identity_info.plmn_Identity.mnc.list.count=0;
  dummy_mnc[0]=0;ASN_SEQUENCE_ADD(&PLMN_identity_info.plmn_Identity.mnc.list,&dummy_mnc[0]);
  dummy_mnc[1]=1;ASN_SEQUENCE_ADD(&PLMN_identity_info.plmn_Identity.mnc.list,&dummy_mnc[1]);
  //assign_enum(&PLMN_identity_info.cellReservedForOperatorUse,PLMN_IdentityInfo__cellReservedForOperatorUse_notReserved);
  PLMN_identity_info.cellReservedForOperatorUse=PLMN_IdentityInfo__cellReservedForOperatorUse_notReserved;

  ASN_SEQUENCE_ADD(&(*sib1)->cellAccessRelatedInfo.plmn_IdentityList.list,&PLMN_identity_info);
  // 16 bits
  (*sib1)->cellAccessRelatedInfo.trackingAreaCode.buf = MALLOC(2);
  (*sib1)->cellAccessRelatedInfo.trackingAreaCode.buf[0]=0x00;
  (*sib1)->cellAccessRelatedInfo.trackingAreaCode.buf[1]=0x01;
  (*sib1)->cellAccessRelatedInfo.trackingAreaCode.size=2;
  (*sib1)->cellAccessRelatedInfo.trackingAreaCode.bits_unused=0;

  // 28 bits
  (*sib1)->cellAccessRelatedInfo.cellIdentity.buf = MALLOC(8);
  (*sib1)->cellAccessRelatedInfo.cellIdentity.buf[3]=0x00;
  (*sib1)->cellAccessRelatedInfo.cellIdentity.buf[2]=0x00;
  (*sib1)->cellAccessRelatedInfo.cellIdentity.buf[1]=0x00;
  (*sib1)->cellAccessRelatedInfo.cellIdentity.buf[0]=0x00;
  (*sib1)->cellAccessRelatedInfo.cellIdentity.size=4;
  (*sib1)->cellAccessRelatedInfo.cellIdentity.bits_unused=4;

  //  assign_enum(&(*sib1)->cellAccessRelatedInfo.cellBarred,SystemInformationBlockType1__cellAccessRelatedInfo__cellBarred_notBarred);
  (*sib1)->cellAccessRelatedInfo.cellBarred=SystemInformationBlockType1__cellAccessRelatedInfo__cellBarred_notBarred;

  //  assign_enum(&(*sib1)->cellAccessRelatedInfo.intraFreqReselection,SystemInformationBlockType1__cellAccessRelatedInfo__intraFreqReselection_allowed);
  (*sib1)->cellAccessRelatedInfo.intraFreqReselection=SystemInformationBlockType1__cellAccessRelatedInfo__intraFreqReselection_notAllowed;
  (*sib1)->cellAccessRelatedInfo.csg_Indication=0;

  (*sib1)->cellSelectionInfo.q_RxLevMin=-65;
  (*sib1)->cellSelectionInfo.q_RxLevMinOffset=NULL;

  if (frame_parms->frame_type == FDD)
    (*sib1)->freqBandIndicator = 1;  // FDD works on band1
  else
    (*sib1)->freqBandIndicator = 40; // TDD works on band38

  schedulingInfo.si_Periodicity=SchedulingInfo__si_Periodicity_rf8;

  // This is for SIB2/3
  sib_type=SIB_Type_sibType3;
  ASN_SEQUENCE_ADD(&schedulingInfo.sib_MappingInfo.list,&sib_type);
  ASN_SEQUENCE_ADD(&(*sib1)->schedulingInfoList.list,&schedulingInfo);

  //  ASN_SEQUENCE_ADD(&schedulingInfo.sib_MappingInfo.list,NULL);








  (*sib1)->tdd_Config = CALLOC(1,sizeof(struct TDD_Config));


  (*sib1)->tdd_Config->subframeAssignment=frame_parms->tdd_config; //TDD_Config__subframeAssignment_sa3;


  (*sib1)->tdd_Config->specialSubframePatterns=0;//frame_parms->tdd_config_S;//TDD_Config__specialSubframePatterns_ssp0;


  (*sib1)->si_WindowLength=SystemInformationBlockType1__si_WindowLength_ms20;
  (*sib1)->systemInfoValueTag=0;
  //  (*sib1).nonCriticalExtension = calloc(1,sizeof(*(*sib1).nonCriticalExtension));

#ifdef XER_PRINT
  xer_fprint(stdout, &asn_DEF_BCCH_DL_SCH_Message, (void*)bcch_message);
#endif
  enc_rval = uper_encode_to_buffer(&asn_DEF_BCCH_DL_SCH_Message,
				   (void*)bcch_message,
				   buffer,
				   100);
#ifdef USER_MODE
  LOG_D(RRC,"[eNB] SystemInformationBlockType1 Encoded %d bits (%d bytes)\n",enc_rval.encoded,(enc_rval.encoded+7)/8);
#endif

  if (enc_rval.encoded==-1)
    return(-1);
  return((enc_rval.encoded+7)/8);
}

uint8_t do_SIB2_AT4(uint8_t Mod_id,
		    uint8_t *buffer,
		    BCCH_DL_SCH_Message_t *bcch_message,
		    SystemInformationBlockType2_t **sib2) {

  struct SystemInformation_r8_IEs__sib_TypeAndInfo__Member *sib2_part;

  asn_enc_rval_t enc_rval;

  memset(bcch_message,0,sizeof(BCCH_DL_SCH_Message_t));


  bcch_message->message.present = BCCH_DL_SCH_MessageType_PR_c1;
  bcch_message->message.choice.c1.present = BCCH_DL_SCH_MessageType__c1_PR_systemInformation;
  bcch_message->message.choice.c1.choice.systemInformation.criticalExtensions.present = SystemInformation__criticalExtensions_PR_systemInformation_r8;

  bcch_message->message.choice.c1.choice.systemInformation.criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list.count=0;

  sib2_part = CALLOC(1,sizeof(struct SystemInformation_r8_IEs__sib_TypeAndInfo__Member));
  memset(sib2_part,0,sizeof(struct SystemInformation_r8_IEs__sib_TypeAndInfo__Member));

  ASN_SEQUENCE_ADD(&bcch_message->message.choice.c1.choice.systemInformation.criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list,
		   sib2_part);

  sib2_part->present = SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib2;

  *sib2 = &sib2_part->choice.sib2;

  // sib2

  (*sib2)->ac_BarringInfo = NULL;
#ifdef Rel10
  (*sib2)->ssac_BarringForMMTEL_Voice_r9 = NULL;
  (*sib2)->ssac_BarringForMMTEL_Video_r9 = NULL;
  (*sib2)->ac_BarringForCSFB_r10 = NULL;
#endif

  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.preambleInfo.numberOfRA_Preambles=RACH_ConfigCommon__preambleInfo__numberOfRA_Preambles_n52;
  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.preambleInfo.preamblesGroupAConfig = NULL;

  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.powerRampingParameters.powerRampingStep=RACH_ConfigCommon__powerRampingParameters__powerRampingStep_dB2;


  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.powerRampingParameters.preambleInitialReceivedTargetPower=RACH_ConfigCommon__powerRampingParameters__preambleInitialReceivedTargetPower_dBm_104;

  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.preambleTransMax=RACH_ConfigCommon__ra_SupervisionInfo__preambleTransMax_n6;

  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.ra_ResponseWindowSize=RACH_ConfigCommon__ra_SupervisionInfo__ra_ResponseWindowSize_sf10;


  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.mac_ContentionResolutionTimer=RACH_ConfigCommon__ra_SupervisionInfo__mac_ContentionResolutionTimer_sf48;

  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.maxHARQ_Msg3Tx = 4;

  // BCCH-Config

  (*sib2)->radioResourceConfigCommon.bcch_Config.modificationPeriodCoeff=BCCH_Config__modificationPeriodCoeff_n4;

  // PCCH-Config

  (*sib2)->radioResourceConfigCommon.pcch_Config.defaultPagingCycle=PCCH_Config__defaultPagingCycle_rf128;


  (*sib2)->radioResourceConfigCommon.pcch_Config.defaultPagingCycle=PCCH_Config__defaultPagingCycle_rf128;
  (*sib2)->radioResourceConfigCommon.pcch_Config.nB=PCCH_Config__nB_oneT;

  // PRACH-Config
  (*sib2)->radioResourceConfigCommon.prach_Config.rootSequenceIndex=22;//0;//384;
  (*sib2)->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.prach_ConfigIndex = 3;//3;
  (*sib2)->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.highSpeedFlag = 0;
  (*sib2)->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.zeroCorrelationZoneConfig = 0;//12;
  (*sib2)->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.prach_FreqOffset = 0;

  // PDSCH-Config
  (*sib2)->radioResourceConfigCommon.pdsch_ConfigCommon.referenceSignalPower=24;
  (*sib2)->radioResourceConfigCommon.pdsch_ConfigCommon.p_b=0;

  // PUSCH-Config
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.n_SB=1;

  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.hoppingMode=PUSCH_ConfigCommon__pusch_ConfigBasic__hoppingMode_interSubFrame;
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.pusch_HoppingOffset=4;
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.enable64QAM=0;
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupHoppingEnabled=0;
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH=0;
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled=0;
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.cyclicShift=1;

  // PUCCH-Config


  (*sib2)->radioResourceConfigCommon.pucch_ConfigCommon.deltaPUCCH_Shift=PUCCH_ConfigCommon__deltaPUCCH_Shift_ds2;
  (*sib2)->radioResourceConfigCommon.pucch_ConfigCommon.nRB_CQI = 2;
  (*sib2)->radioResourceConfigCommon.pucch_ConfigCommon.nCS_AN = 0;
  (*sib2)->radioResourceConfigCommon.pucch_ConfigCommon.n1PUCCH_AN = 0;


  (*sib2)->radioResourceConfigCommon.soundingRS_UL_ConfigCommon.present=SoundingRS_UL_ConfigCommon_PR_release;
  (*sib2)->radioResourceConfigCommon.soundingRS_UL_ConfigCommon.choice.release=0;

  // uplinkPowerControlCommon

  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.p0_NominalPUSCH = -95;

  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.alpha=UplinkPowerControlCommon__alpha_al08;
  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.p0_NominalPUCCH = -117;

  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format1=DeltaFList_PUCCH__deltaF_PUCCH_Format1_deltaF0;

  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format1b=DeltaFList_PUCCH__deltaF_PUCCH_Format1b_deltaF3;


  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2=DeltaFList_PUCCH__deltaF_PUCCH_Format2_deltaF0;


  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2a=DeltaFList_PUCCH__deltaF_PUCCH_Format2a_deltaF0;


  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2b=DeltaFList_PUCCH__deltaF_PUCCH_Format2b_deltaF0;

  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaPreambleMsg3 = 4;


  (*sib2)->radioResourceConfigCommon.ul_CyclicPrefixLength=UL_CyclicPrefixLength_len1;


  (*sib2)->ue_TimersAndConstants.t300=UE_TimersAndConstants__t300_ms1000;


  (*sib2)->ue_TimersAndConstants.t301=UE_TimersAndConstants__t301_ms1000;


  (*sib2)->ue_TimersAndConstants.t310=UE_TimersAndConstants__t310_ms1000;


  (*sib2)->ue_TimersAndConstants.n310=UE_TimersAndConstants__n310_n1;


  (*sib2)->ue_TimersAndConstants.t311=UE_TimersAndConstants__t311_ms10000;


  (*sib2)->ue_TimersAndConstants.n311=UE_TimersAndConstants__n311_n1;

  (*sib2)->freqInfo.additionalSpectrumEmission = 1;
  (*sib2)->freqInfo.ul_Bandwidth = CALLOC(1,sizeof(long));
  (*sib2)->freqInfo.ul_CarrierFreq = CALLOC(1,sizeof(ARFCN_ValueEUTRA_t));
  *((*sib2)->freqInfo.ul_CarrierFreq) = 38050;
  *((*sib2)->freqInfo.ul_Bandwidth) = SystemInformationBlockType2__freqInfo__ul_Bandwidth_n50;
  (*sib2)->mbsfn_SubframeConfigList = NULL;
  (*sib2)->timeAlignmentTimerCommon= TimeAlignmentTimer_infinity;//TimeAlignmentTimer_sf10240;


#ifdef XER_PRINT
  xer_fprint(stdout, &asn_DEF_BCCH_DL_SCH_Message, (void*)bcch_message);
#endif
  enc_rval = uper_encode_to_buffer(&asn_DEF_BCCH_DL_SCH_Message,
				   (void*)bcch_message,
				   buffer,
				   100);
#ifdef USER_MODE
  LOG_D(RRC,"[eNB] SystemInformation Encoded %d bits (%d bytes)\n",enc_rval.encoded,(enc_rval.encoded+7)/8);
#endif

  if (enc_rval.encoded==-1) {
    msg("[RRC] ASN1 : SI encoding failed for SIB2\n");
    return(-1);
  }
  return((enc_rval.encoded+7)/8);

}

uint8_t do_SIB23(uint8_t Mod_id,
		 LTE_DL_FRAME_PARMS *frame_parms,
		 uint8_t *buffer,
		 BCCH_DL_SCH_Message_t *bcch_message,
		 SystemInformationBlockType2_t **sib2,
		 SystemInformationBlockType3_t **sib3
#ifdef Rel10
		 ,
                 SystemInformationBlockType13_r9_t **sib13,
		 uint8_t MBMS_flag
#endif
		 ) {


  struct SystemInformation_r8_IEs__sib_TypeAndInfo__Member *sib2_part,*sib3_part;
#ifdef Rel10
  struct SystemInformation_r8_IEs__sib_TypeAndInfo__Member *sib13_part;
  MBSFN_SubframeConfigList_t *MBSFNSubframeConfigList;
  MBSFN_AreaInfoList_r9_t *MBSFNArea_list;
  struct MBSFN_AreaInfo_r9 *MBSFN_Area1;
#endif
  asn_enc_rval_t enc_rval;

  if (bcch_message) 
    memset(bcch_message,0,sizeof(BCCH_DL_SCH_Message_t));
  else {
    LOG_E(RRC,"BCCH_MESSAGE is null, exiting\n");
    exit(-1);
  }

  if (!sib2) {
    LOG_E(RRC,"sib2 is null, exiting\n");
    exit(-1);
  }

  if (!sib3) {
    LOG_E(RRC,"sib3 is null, exiting\n");
    exit(-1);
  }
#ifdef Rel10
  LOG_I(RRC,"Configuration SIB2/3, MBMS = %d\n",MBMS_flag);
#else
  LOG_I(RRC,"Configuration SIB2/3\n");
#endif
  sib2_part = CALLOC(1,sizeof(struct SystemInformation_r8_IEs__sib_TypeAndInfo__Member));
  sib3_part = CALLOC(1,sizeof(struct SystemInformation_r8_IEs__sib_TypeAndInfo__Member));
  memset(sib2_part,0,sizeof(struct SystemInformation_r8_IEs__sib_TypeAndInfo__Member));
  memset(sib3_part,0,sizeof(struct SystemInformation_r8_IEs__sib_TypeAndInfo__Member));

  sib2_part->present = SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib2;
  sib3_part->present = SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib3;

  *sib2 = &sib2_part->choice.sib2;
  *sib3 = &sib3_part->choice.sib3;

#ifdef Rel10
  if (MBMS_flag == 1) {
    sib13_part = CALLOC(1,sizeof(struct SystemInformation_r8_IEs__sib_TypeAndInfo__Member));
    memset(sib13_part,0,sizeof(struct SystemInformation_r8_IEs__sib_TypeAndInfo__Member));
    sib13_part->present = SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib13_v920;
    *sib13 = &sib13_part->choice.sib13_v920;
  }
#endif

  // sib2

  (*sib2)->ac_BarringInfo = NULL;
#ifdef Rel10
  (*sib2)->ssac_BarringForMMTEL_Voice_r9 = NULL;
  (*sib2)->ssac_BarringForMMTEL_Video_r9 = NULL;
  (*sib2)->ac_BarringForCSFB_r10 = NULL;
#endif

  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.preambleInfo.numberOfRA_Preambles=RACH_ConfigCommon__preambleInfo__numberOfRA_Preambles_n64;
  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.preambleInfo.preamblesGroupAConfig = NULL;
  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.powerRampingParameters.powerRampingStep=RACH_ConfigCommon__powerRampingParameters__powerRampingStep_dB2;

#ifdef EXMIMO
  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.powerRampingParameters.preambleInitialReceivedTargetPower=RACH_ConfigCommon__powerRampingParameters__preambleInitialReceivedTargetPower_dBm_90;
#else
  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.powerRampingParameters.preambleInitialReceivedTargetPower=RACH_ConfigCommon__powerRampingParameters__preambleInitialReceivedTargetPower_dBm_108;
#endif
  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.preambleTransMax=RACH_ConfigCommon__ra_SupervisionInfo__preambleTransMax_n10;

  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.ra_ResponseWindowSize=RACH_ConfigCommon__ra_SupervisionInfo__ra_ResponseWindowSize_sf10;

  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.mac_ContentionResolutionTimer=RACH_ConfigCommon__ra_SupervisionInfo__mac_ContentionResolutionTimer_sf48;

  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.maxHARQ_Msg3Tx = 4;

  // BCCH-Config

  (*sib2)->radioResourceConfigCommon.bcch_Config.modificationPeriodCoeff=BCCH_Config__modificationPeriodCoeff_n2;

  // PCCH-Config

  (*sib2)->radioResourceConfigCommon.pcch_Config.defaultPagingCycle=PCCH_Config__defaultPagingCycle_rf128;


  (*sib2)->radioResourceConfigCommon.pcch_Config.nB=PCCH_Config__nB_oneT;

  // PRACH-Config
  (*sib2)->radioResourceConfigCommon.prach_Config.rootSequenceIndex=Mod_id;//0;//384;
  (*sib2)->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.prach_ConfigIndex = 0;//3;
  (*sib2)->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.highSpeedFlag = 0;
  (*sib2)->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.zeroCorrelationZoneConfig = 1;//12;
  (*sib2)->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.prach_FreqOffset = 0;

  // PDSCH-Config
#ifdef EXMIMO
  (*sib2)->radioResourceConfigCommon.pdsch_ConfigCommon.referenceSignalPower=0;
#else
  (*sib2)->radioResourceConfigCommon.pdsch_ConfigCommon.referenceSignalPower=15;
#endif
  if (frame_parms->mode1_flag==1)
    (*sib2)->radioResourceConfigCommon.pdsch_ConfigCommon.p_b=0;
  else
    (*sib2)->radioResourceConfigCommon.pdsch_ConfigCommon.p_b=1;


  // PUSCH-Config
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.n_SB=1;
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.hoppingMode=PUSCH_ConfigCommon__pusch_ConfigBasic__hoppingMode_interSubFrame;
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.pusch_HoppingOffset=0;
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.enable64QAM=0;
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupHoppingEnabled=0;
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH=0;
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled=0;
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.cyclicShift=0;

  // PUCCH-Config

  (*sib2)->radioResourceConfigCommon.pucch_ConfigCommon.deltaPUCCH_Shift=PUCCH_ConfigCommon__deltaPUCCH_Shift_ds1;
  (*sib2)->radioResourceConfigCommon.pucch_ConfigCommon.nRB_CQI = 1;
  (*sib2)->radioResourceConfigCommon.pucch_ConfigCommon.nCS_AN = 0;
  (*sib2)->radioResourceConfigCommon.pucch_ConfigCommon.n1PUCCH_AN = 32;


  (*sib2)->radioResourceConfigCommon.soundingRS_UL_ConfigCommon.present=SoundingRS_UL_ConfigCommon_PR_release;
  (*sib2)->radioResourceConfigCommon.soundingRS_UL_ConfigCommon.choice.release=0;

  // uplinkPowerControlCommon

#ifdef EXMIMO
  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.p0_NominalPUSCH = -90;
  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.p0_NominalPUCCH = -96;
#else
  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.p0_NominalPUSCH = -108;
  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.p0_NominalPUCCH = -108;
#endif
  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.alpha=UplinkPowerControlCommon__alpha_al1;
  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format1=DeltaFList_PUCCH__deltaF_PUCCH_Format1_deltaF2;
  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format1b=DeltaFList_PUCCH__deltaF_PUCCH_Format1b_deltaF3;

  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2=DeltaFList_PUCCH__deltaF_PUCCH_Format2_deltaF0;

  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2a=DeltaFList_PUCCH__deltaF_PUCCH_Format2a_deltaF0;

  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2b=DeltaFList_PUCCH__deltaF_PUCCH_Format2b_deltaF0;

  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaPreambleMsg3 = 6;

  (*sib2)->radioResourceConfigCommon.ul_CyclicPrefixLength=UL_CyclicPrefixLength_len1;

  (*sib2)->ue_TimersAndConstants.t300=UE_TimersAndConstants__t300_ms1000;

  (*sib2)->ue_TimersAndConstants.t301=UE_TimersAndConstants__t301_ms1000;

  (*sib2)->ue_TimersAndConstants.t310=UE_TimersAndConstants__t310_ms1000;

  (*sib2)->ue_TimersAndConstants.n310=UE_TimersAndConstants__n310_n20;

  (*sib2)->ue_TimersAndConstants.t311=UE_TimersAndConstants__t311_ms10000;

  (*sib2)->ue_TimersAndConstants.n311=UE_TimersAndConstants__n311_n1;

  (*sib2)->freqInfo.additionalSpectrumEmission = 1;
  (*sib2)->freqInfo.ul_CarrierFreq = NULL;
  (*sib2)->freqInfo.ul_Bandwidth = NULL;
  //  (*sib2)->mbsfn_SubframeConfigList = NULL;

#ifdef Rel10
  if (MBMS_flag == 1) {
    LOG_I(RRC,"Adding MBSFN Configuration to SIB2\n");
    MBSFN_SubframeConfig_t *sib2_mbsfn_SubframeConfig1;
    (*sib2)->mbsfn_SubframeConfigList = CALLOC(1,sizeof(struct MBSFN_SubframeConfigList));
    MBSFNSubframeConfigList = (*sib2)->mbsfn_SubframeConfigList;
    
    sib2_mbsfn_SubframeConfig1= CALLOC(1,sizeof(*sib2_mbsfn_SubframeConfig1));
    memset((void*)sib2_mbsfn_SubframeConfig1,0,sizeof(*sib2_mbsfn_SubframeConfig1));
    
    sib2_mbsfn_SubframeConfig1->radioframeAllocationPeriod= MBSFN_SubframeConfig__radioframeAllocationPeriod_n4;
    sib2_mbsfn_SubframeConfig1->radioframeAllocationOffset= 1;
    sib2_mbsfn_SubframeConfig1->subframeAllocation.present= MBSFN_SubframeConfig__subframeAllocation_PR_oneFrame;
    sib2_mbsfn_SubframeConfig1->subframeAllocation.choice.oneFrame.buf= MALLOC(1);
    sib2_mbsfn_SubframeConfig1->subframeAllocation.choice.oneFrame.size= 1;
    sib2_mbsfn_SubframeConfig1->subframeAllocation.choice.oneFrame.bits_unused= 2;
    if (frame_parms->frame_type == TDD) {// pattern 001110 for TDD
      sib2_mbsfn_SubframeConfig1->subframeAllocation.choice.oneFrame.buf[0]=0x0e<<2;// shift 2 cuz 2last bits are unused.
    } 
    else {   // pattern 101010 for FDD)
      sib2_mbsfn_SubframeConfig1->subframeAllocation.choice.oneFrame.buf[0]=0x2a<<2;
    }
    ASN_SEQUENCE_ADD(&MBSFNSubframeConfigList->list,sib2_mbsfn_SubframeConfig1);
  }
#else // no MBMS transmission
    (*sib2)->mbsfn_SubframeConfigList = NULL;
#endif
  (*sib2)->timeAlignmentTimerCommon=TimeAlignmentTimer_sf10240;
  /// (*SIB3)
#ifdef Rel10
  (*sib3)->s_IntraSearch_v920=NULL;
  (*sib3)->s_NonIntraSearch_v920=NULL;
  (*sib3)->q_QualMin_r9=NULL;
  (*sib3)->threshServingLowQ_r9=NULL;
#endif
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

  (*sib3)->intraFreqCellReselectionInfo.allowedMeasBandwidth=AllowedMeasBandwidth_mbw6;

  (*sib3)->intraFreqCellReselectionInfo.presenceAntennaPort1 = 0;
  (*sib3)->intraFreqCellReselectionInfo.neighCellConfig.buf = CALLOC(8,1);
  (*sib3)->intraFreqCellReselectionInfo.neighCellConfig.size = 1;
  (*sib3)->intraFreqCellReselectionInfo.neighCellConfig.buf[0] = 1;
  (*sib3)->intraFreqCellReselectionInfo.neighCellConfig.bits_unused = 6;
  (*sib3)->intraFreqCellReselectionInfo.t_ReselectionEUTRA = 1;
  (*sib3)->intraFreqCellReselectionInfo.t_ReselectionEUTRA_SF = (struct SpeedStateScaleFactors *)NULL;

  // SIB13
  // fill in all elements of SIB13 if present
#ifdef Rel10
  if (MBMS_flag == 1) {
    //  Notification for mcch change
    (*sib13)->notificationConfig_r9.notificationRepetitionCoeff_r9= MBMS_NotificationConfig_r9__notificationRepetitionCoeff_r9_n2;
    (*sib13)->notificationConfig_r9.notificationOffset_r9= 0;
    (*sib13)->notificationConfig_r9.notificationSF_Index_r9= 1;

    //  MBSFN-AreaInfoList
    MBSFNArea_list= &(*sib13)->mbsfn_AreaInfoList_r9;//CALLOC(1,sizeof(*MBSFNArea_list));
    memset(MBSFNArea_list,0,sizeof(*MBSFNArea_list));
    // MBSFN Area 1
    MBSFN_Area1= CALLOC(1, sizeof(*MBSFN_Area1));
    MBSFN_Area1->mbsfn_AreaId_r9= 1;
    MBSFN_Area1->non_MBSFNregionLength= MBSFN_AreaInfo_r9__non_MBSFNregionLength_s1;
    MBSFN_Area1->notificationIndicator_r9= 0;
    MBSFN_Area1->mcch_Config_r9.mcch_RepetitionPeriod_r9= MBSFN_AreaInfo_r9__mcch_Config_r9__mcch_RepetitionPeriod_r9_rf32;
    MBSFN_Area1->mcch_Config_r9.mcch_Offset_r9= 1; // in accordance with mbsfn subframe configuration in sib2
    MBSFN_Area1->mcch_Config_r9.mcch_ModificationPeriod_r9= MBSFN_AreaInfo_r9__mcch_Config_r9__mcch_ModificationPeriod_r9_rf512;
    //  Subframe Allocation Info
    MBSFN_Area1->mcch_Config_r9.sf_AllocInfo_r9.buf= MALLOC(1);
    MBSFN_Area1->mcch_Config_r9.sf_AllocInfo_r9.size= 1;
    if (frame_parms->frame_type == TDD) {//TDD: SF7
      MBSFN_Area1->mcch_Config_r9.sf_AllocInfo_r9.buf[0]=0x08<<2; 
    } 
    else {
      MBSFN_Area1->mcch_Config_r9.sf_AllocInfo_r9.buf[0]=0x20<<2;  // FDD: SF1
    }
    MBSFN_Area1->mcch_Config_r9.sf_AllocInfo_r9.bits_unused= 2;

    MBSFN_Area1->mcch_Config_r9.signallingMCS_r9= MBSFN_AreaInfo_r9__mcch_Config_r9__signallingMCS_r9_n7;

    ASN_SEQUENCE_ADD(&MBSFNArea_list->list,MBSFN_Area1);
    
    /*    //MBSFN Area 2
    MBSFN_Area2= CALLOC(1, sizeof(*MBSFN_Area2));
    MBSFN_Area2->mbsfn_AreaId_r9= 2;
    MBSFN_Area2->non_MBSFNregionLength= MBSFN_AreaInfo_r9__non_MBSFNregionLength_s1;
    MBSFN_Area2->notificationIndicator_r9= 0;
    MBSFN_Area2->mcch_Config_r9.mcch_RepetitionPeriod_r9= MBSFN_AreaInfo_r9__mcch_Config_r9__mcch_RepetitionPeriod_r9_rf32;
    MBSFN_Area2->mcch_Config_r9.mcch_Offset_r9= 0;
    MBSFN_Area2->mcch_Config_r9.mcch_ModificationPeriod_r9= MBSFN_AreaInfo_r9__mcch_Config_r9__mcch_ModificationPeriod_r9_rf512;
    // Subframe Allocation Info
    MBSFN_Area2->mcch_Config_r9.sf_AllocInfo_r9.buf= MALLOC(1);
    MBSFN_Area2->mcch_Config_r9.sf_AllocInfo_r9.size= 1;
    MBSFN_Area2->mcch_Config_r9.sf_AllocInfo_r9.buf[0]=0x8;
    MBSFN_Area2->mcch_Config_r9.sf_AllocInfo_r9.bits_unused= 2;

    MBSFN_Area2->mcch_Config_r9.signallingMCS_r9= MBSFN_AreaInfo_r9__mcch_Config_r9__signallingMCS_r9_n2;

    ASN_SEQUENCE_ADD(&MBSFNArea_list->list,MBSFN_Area2);*/
    //  end of adding for MBMS SIB13
  }
#endif


  bcch_message->message.present = BCCH_DL_SCH_MessageType_PR_c1;
  bcch_message->message.choice.c1.present = BCCH_DL_SCH_MessageType__c1_PR_systemInformation;

  /*  memcpy((void*)&bcch_message.message.choice.c1.choice.systemInformation,
	 (void*)systemInformation,
	 sizeof(SystemInformation_t));*/

  bcch_message->message.choice.c1.choice.systemInformation.criticalExtensions.present = SystemInformation__criticalExtensions_PR_systemInformation_r8;

  bcch_message->message.choice.c1.choice.systemInformation.criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list.count=0;

  //  asn_set_empty(&systemInformation->criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list);//.size=0;
  //  systemInformation->criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list.count=0;
  ASN_SEQUENCE_ADD(&bcch_message->message.choice.c1.choice.systemInformation.criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list,
		   sib2_part);
  ASN_SEQUENCE_ADD(&bcch_message->message.choice.c1.choice.systemInformation.criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list,
		   sib3_part);
#ifdef Rel10
  if (MBMS_flag == 1) {
    ASN_SEQUENCE_ADD(&bcch_message->message.choice.c1.choice.systemInformation.criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list,sib13_part);
  }
#endif


#ifdef XER_PRINT
  xer_fprint(stdout, &asn_DEF_BCCH_DL_SCH_Message, (void*)bcch_message);
#endif
  enc_rval = uper_encode_to_buffer(&asn_DEF_BCCH_DL_SCH_Message,
				   (void*)bcch_message,
				   buffer,
				   900);
#ifdef USER_MODE
  LOG_D(RRC,"[eNB] SystemInformation Encoded %d bits (%d bytes)\n",enc_rval.encoded,(enc_rval.encoded+7)/8);
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

  memset((void *)&ul_ccch_msg,0,sizeof(UL_CCCH_Message_t));

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


  rrcConnectionRequest->criticalExtensions.choice.rrcConnectionRequest_r8.establishmentCause = EstablishmentCause_mo_Signalling; //EstablishmentCause_mo_Data;

  rrcConnectionRequest->criticalExtensions.choice.rrcConnectionRequest_r8.spare.buf = &buf2;
  rrcConnectionRequest->criticalExtensions.choice.rrcConnectionRequest_r8.spare.size=1;
  rrcConnectionRequest->criticalExtensions.choice.rrcConnectionRequest_r8.spare.bits_unused = 7;


  enc_rval = uper_encode_to_buffer(&asn_DEF_UL_CCCH_Message,
				   (void*)&ul_ccch_msg,
				   buffer,
				   100);

#ifdef USER_MODE
  LOG_D(RRC,"[UE] RRCConnectionRequest Encoded %d bits (%d bytes), ecause %d\n",enc_rval.encoded,(enc_rval.encoded+7)/8,ecause);
#endif

  return((enc_rval.encoded+7)/8);

}

uint8_t do_RRCConnectionSetupComplete(uint8_t *buffer) {


  asn_enc_rval_t enc_rval;

  UL_DCCH_Message_t ul_dcch_msg;

  RRCConnectionSetupComplete_t *rrcConnectionSetupComplete;

  memset((void *)&ul_dcch_msg,0,sizeof(UL_DCCH_Message_t));

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
  LOG_D(RRC,"RRCConnectionSetupComplete Encoded %d bits (%d bytes)\n",enc_rval.encoded,(enc_rval.encoded+7)/8);
#endif

  return((enc_rval.encoded+7)/8);

}

uint8_t do_DLInformationTransfer(uint8_t *buffer, void *nas_buf, int nas_size)
{
  asn_enc_rval_t enc_rval;
  DL_DCCH_Message_t dl_dcch_msg;
  
  DLInformationTransfer_t *dlInformationTransfer; 
  DLInformationTransfer_r8_IEs_t *dlInformationTransferR8;

  memset((void*)&dl_dcch_msg,0,sizeof(DL_DCCH_Message_t));

  dl_dcch_msg.message.present           = DL_DCCH_MessageType_PR_c1;
  dl_dcch_msg.message.choice.c1.present = DL_DCCH_MessageType__c1_PR_dlInformationTransfer;
  dlInformationTransfer                 = &dl_dcch_msg.message.choice.c1.choice.dlInformationTransfer;

  dlInformationTransfer->rrc_TransactionIdentifier            = 0x0; //TODO:????
  dlInformationTransfer->criticalExtensions.present           = DLInformationTransfer__criticalExtensions_PR_c1;
  dlInformationTransfer->criticalExtensions.choice.c1.present = DLInformationTransfer__criticalExtensions__c1_PR_dlInformationTransfer_r8;
  
  dlInformationTransferR8 = &dlInformationTransfer->criticalExtensions.choice.c1.choice.dlInformationTransfer_r8; 
  dlInformationTransferR8->dedicatedInfoType.present  = DLInformationTransfer_r8_IEs__dedicatedInfoType_PR_dedicatedInfoNAS;
  memset(&dlInformationTransferR8->dedicatedInfoType.choice.dedicatedInfoNAS, 0, sizeof(OCTET_STRING_t));
  OCTET_STRING_fromBuf(&dlInformationTransferR8->dedicatedInfoType.choice.dedicatedInfoNAS, nas_buf, nas_size);

  enc_rval = uper_encode_to_buffer(&asn_DEF_DL_DCCH_Message,
                                  (void*)&dl_dcch_msg,
                                  buffer, 
                                  100);

  LOG_D(RRC, "DLInformationTransfer Encoded %d bits (%d bytes)\n", enc_rval.encoded, (enc_rval.encoded+7)/8);

  return ((enc_rval.encoded+7)/8);
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
 LOG_D(RRC,"RRCConnectionReconfigurationComplete Encoded %d bits (%d bytes)\n",enc_rval.encoded,(enc_rval.encoded+7)/8);
#endif

  return((enc_rval.encoded+7)/8);
}


uint8_t do_RRCConnectionSetup(uint8_t *buffer,
			      u8 transmission_mode,
			      uint8_t UE_id,
			      uint8_t Transaction_id,
			      LTE_DL_FRAME_PARMS *frame_parms,
			      SRB_ToAddModList_t  **SRB_configList,
			      struct PhysicalConfigDedicated  **physicalConfigDedicated) {


  asn_enc_rval_t enc_rval;
  uint8_t ecause=0;

  long *logicalchannelgroup;

  struct SRB_ToAddMod *SRB1_config;
  struct SRB_ToAddMod__rlc_Config *SRB1_rlc_config;
  struct SRB_ToAddMod__logicalChannelConfig *SRB1_lchan_config;
  struct LogicalChannelConfig__ul_SpecificParameters *SRB1_ul_SpecificParameters;

#ifdef CBA
  struct PUSCH_CBAConfigDedicated_vlola	*pusch_CBAConfigDedicated_vlola;
  long * betaOffset_CBA_Index;
  long * cShift_CBA;
#endif 
  PhysicalConfigDedicated_t *physicalConfigDedicated2;

  DL_CCCH_Message_t dl_ccch_msg;

  RRCConnectionSetup_t *rrcConnectionSetup;

  memset((void *)&dl_ccch_msg,0,sizeof(DL_CCCH_Message_t));
  dl_ccch_msg.message.present           = DL_CCCH_MessageType_PR_c1;
  dl_ccch_msg.message.choice.c1.present = DL_CCCH_MessageType__c1_PR_rrcConnectionSetup;
  rrcConnectionSetup          = &dl_ccch_msg.message.choice.c1.choice.rrcConnectionSetup;

  // RRCConnectionSetup
  // Configure SRB1

  *SRB_configList = CALLOC(1,sizeof(*SRB_configList));

  /// SRB1
  SRB1_config = CALLOC(1,sizeof(*SRB1_config));

  SRB1_config->srb_Identity = 1;
  SRB1_rlc_config = CALLOC(1,sizeof(*SRB1_rlc_config));
  SRB1_config->rlc_Config   = SRB1_rlc_config;

  SRB1_rlc_config->present = SRB_ToAddMod__rlc_Config_PR_explicitValue;
  SRB1_rlc_config->choice.explicitValue.present=RLC_Config_PR_am;
  SRB1_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.t_PollRetransmit=T_PollRetransmit_ms15;
  SRB1_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.pollPDU=PollPDU_p8;
  SRB1_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.pollByte=PollByte_kB1000;
  SRB1_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.maxRetxThreshold=UL_AM_RLC__maxRetxThreshold_t16;
  SRB1_rlc_config->choice.explicitValue.choice.am.dl_AM_RLC.t_Reordering=T_Reordering_ms50;
  SRB1_rlc_config->choice.explicitValue.choice.am.dl_AM_RLC.t_StatusProhibit=T_StatusProhibit_ms10;

  SRB1_lchan_config = CALLOC(1,sizeof(*SRB1_lchan_config));
  SRB1_config->logicalChannelConfig   = SRB1_lchan_config;

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


  ASN_SEQUENCE_ADD(&(*SRB_configList)->list,SRB1_config);

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
#ifdef CBA
  physicalConfigDedicated2->pusch_CBAConfigDedicated_vlola = CALLOC(1,sizeof(*physicalConfigDedicated2->pusch_CBAConfigDedicated_vlola));
#endif
  // PDSCH
  //assign_enum(&physicalConfigDedicated2->pdsch_ConfigDedicated->p_a,
  //	      PDSCH_ConfigDedicated__p_a_dB0);
  physicalConfigDedicated2->pdsch_ConfigDedicated->p_a = PDSCH_ConfigDedicated__p_a_dB0;

  // PUCCH
  physicalConfigDedicated2->pucch_ConfigDedicated->ackNackRepetition.present=PUCCH_ConfigDedicated__ackNackRepetition_PR_release;
  physicalConfigDedicated2->pucch_ConfigDedicated->ackNackRepetition.choice.release=0;
  if(frame_parms->frame_type == FDD) {
  physicalConfigDedicated2->pucch_ConfigDedicated->tdd_AckNackFeedbackMode= NULL;//CALLOC(1,sizeof(long));
  } else { //TDD
  physicalConfigDedicated2->pucch_ConfigDedicated->tdd_AckNackFeedbackMode= CALLOC(1,sizeof(long));
  *(physicalConfigDedicated2->pucch_ConfigDedicated->tdd_AckNackFeedbackMode) = PUCCH_ConfigDedicated__tdd_AckNackFeedbackMode_bundling;//PUCCH_ConfigDedicated__tdd_AckNackFeedbackMode_multiplexing;
  }

  // Pusch_config_dedicated
  physicalConfigDedicated2->pusch_ConfigDedicated->betaOffset_ACK_Index = 0; // 2.00
  physicalConfigDedicated2->pusch_ConfigDedicated->betaOffset_RI_Index  = 0; // 1.25
  physicalConfigDedicated2->pusch_ConfigDedicated->betaOffset_CQI_Index = 8; // 2.25

  // UplinkPowerControlDedicated
  physicalConfigDedicated2->uplinkPowerControlDedicated->p0_UE_PUSCH = 0; // 0 dB
  //assign_enum(&physicalConfigDedicated2->uplinkPowerControlDedicated->deltaMCS_Enabled,
  // UplinkPowerControlDedicated__deltaMCS_Enabled_en1);
  physicalConfigDedicated2->uplinkPowerControlDedicated->deltaMCS_Enabled= UplinkPowerControlDedicated__deltaMCS_Enabled_en1;
  physicalConfigDedicated2->uplinkPowerControlDedicated->accumulationEnabled = 1;  // FALSE
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

  switch (transmission_mode){
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
  
  if (frame_parms->frame_type == 0) // FDD
    physicalConfigDedicated2->schedulingRequestConfig->choice.setup.sr_ConfigIndex = 5+(UE_id%10);  // Isr = 5 (every 10 subframes, offset=2+UE_id mod3)
  else {
    switch (frame_parms->tdd_config) {
    case 1:
      physicalConfigDedicated2->schedulingRequestConfig->choice.setup.sr_ConfigIndex = 7+(UE_id&1)+((UE_id&3)>>1)*5;  // Isr = 5 (every 10 subframes, offset=2 for UE0, 3 for UE1, 7 for UE2, 8 for UE3 , 2 for UE4 etc..)
      break;
    case 3:
      physicalConfigDedicated2->schedulingRequestConfig->choice.setup.sr_ConfigIndex = 7+(UE_id%3);  // Isr = 5 (every 10 subframes, offset=2 for UE0, 3 for UE1, 3 for UE2, 2 for UE3 , etc..)
      break;
    case 4:
      physicalConfigDedicated2->schedulingRequestConfig->choice.setup.sr_ConfigIndex = 7+(UE_id&1);  // Isr = 5 (every 10 subframes, offset=2 for UE0, 3 for UE1, 3 for UE2, 2 for UE3 , etc..)
      break;
    default:
      physicalConfigDedicated2->schedulingRequestConfig->choice.setup.sr_ConfigIndex = 7;  // Isr = 5 (every 10 subframes, offset=2 for all UE0 etc..)
      break;
    }
  }
      
  //  assign_enum(&physicalConfigDedicated2->schedulingRequestConfig->choice.setup.dsr_TransMax,
  //SchedulingRequestConfig__setup__dsr_TransMax_n4);
  //  assign_enum(&physicalConfigDedicated2->schedulingRequestConfig->choice.setup.dsr_TransMax = SchedulingRequestConfig__setup__dsr_TransMax_n4;
  physicalConfigDedicated2->schedulingRequestConfig->choice.setup.dsr_TransMax = SchedulingRequestConfig__setup__dsr_TransMax_n4;

  rrcConnectionSetup->rrc_TransactionIdentifier = Transaction_id;
  rrcConnectionSetup->criticalExtensions.present = RRCConnectionSetup__criticalExtensions_PR_c1;
  rrcConnectionSetup->criticalExtensions.choice.c1.present =RRCConnectionSetup__criticalExtensions__c1_PR_rrcConnectionSetup_r8 ;
  rrcConnectionSetup->criticalExtensions.choice.c1.choice.rrcConnectionSetup_r8.radioResourceConfigDedicated.srb_ToAddModList = *SRB_configList;
  rrcConnectionSetup->criticalExtensions.choice.c1.choice.rrcConnectionSetup_r8.radioResourceConfigDedicated.drb_ToAddModList = NULL;
  rrcConnectionSetup->criticalExtensions.choice.c1.choice.rrcConnectionSetup_r8.radioResourceConfigDedicated.drb_ToReleaseList = NULL;
  rrcConnectionSetup->criticalExtensions.choice.c1.choice.rrcConnectionSetup_r8.radioResourceConfigDedicated.sps_Config = NULL;
  rrcConnectionSetup->criticalExtensions.choice.c1.choice.rrcConnectionSetup_r8.radioResourceConfigDedicated.physicalConfigDedicated = physicalConfigDedicated2;
  rrcConnectionSetup->criticalExtensions.choice.c1.choice.rrcConnectionSetup_r8.radioResourceConfigDedicated.mac_MainConfig = NULL;
#ifdef CBA
  betaOffset_CBA_Index = CALLOC(1,sizeof(long));
  cShift_CBA = CALLOC(1,sizeof(long));
  *betaOffset_CBA_Index=10; // need to be changed by Kaijie
  *cShift_CBA=4;
  physicalConfigDedicated2->pusch_CBAConfigDedicated_vlola->betaOffset_CBA_Index=betaOffset_CBA_Index;
  physicalConfigDedicated2->pusch_CBAConfigDedicated_vlola->cShift_CBA=cShift_CBA;
  rrcConnectionSetup->criticalExtensions.choice.c1.choice.rrcConnectionSetup_r8.radioResourceConfigDedicated.sps_CBA_ConfigList_vlola = NULL;
#endif


  enc_rval = uper_encode_to_buffer(&asn_DEF_DL_CCCH_Message,
				   (void*)&dl_ccch_msg,
				   buffer,
				   100);

#ifdef USER_MODE
  LOG_D(RRC,"RRCConnectionSetup Encoded %d bits (%d bytes), ecause %d\n",enc_rval.encoded,(enc_rval.encoded+7)/8,ecause);
#endif

  //  FREEMEM(SRB_list);
  //  free(SRB1_config);
  //  free(SRB1_rlc_config);
  //  free(SRB1_lchan_config);
  //  free(SRB1_ul_SpecificParameters);

  return((enc_rval.encoded+7)/8);
}
uint8_t do_SecurityModeCommand(uint8_t Mod_id,
				 uint8_t *buffer,
				 uint8_t UE_id,
				 uint8_t Transaction_id) {
 DL_DCCH_Message_t dl_dcch_msg;
 asn_enc_rval_t enc_rval;

 memset(&dl_dcch_msg,0,sizeof(DL_DCCH_Message_t));

 dl_dcch_msg.message.present           = DL_DCCH_MessageType_PR_c1;
 dl_dcch_msg.message.choice.c1.present = DL_DCCH_MessageType__c1_PR_securityModeCommand;
 
 dl_dcch_msg.message.choice.c1.choice.securityModeCommand.rrc_TransactionIdentifier = Transaction_id;
 dl_dcch_msg.message.choice.c1.choice.securityModeCommand.criticalExtensions.present = SecurityModeCommand__criticalExtensions_PR_c1;
 
 dl_dcch_msg.message.choice.c1.choice.securityModeCommand.criticalExtensions.choice.c1.present = SecurityModeCommand__criticalExtensions__c1_PR_securityModeCommand_r8;
 // the two following information could be based on the mod_id
 dl_dcch_msg.message.choice.c1.choice.securityModeCommand.criticalExtensions.choice.c1.choice.securityModeCommand_r8.securityConfigSMC.securityAlgorithmConfig.cipheringAlgorithm=SecurityAlgorithmConfig__cipheringAlgorithm_eea0;
  dl_dcch_msg.message.choice.c1.choice.securityModeCommand.criticalExtensions.choice.c1.choice.securityModeCommand_r8.securityConfigSMC.securityAlgorithmConfig.integrityProtAlgorithm=SecurityAlgorithmConfig__integrityProtAlgorithm_spare1;


#ifdef XER_PRINT
  xer_fprint(stdout, &asn_DEF_DL_DCCH_Message, (void*)&dl_dcch_msg);
#endif
  enc_rval = uper_encode_to_buffer(&asn_DEF_DL_DCCH_Message,
				   (void*)&dl_dcch_msg,
				   buffer,
				   100);
#ifdef USER_MODE
  LOG_D(RRC,"[eNB %d] securityModeCommand for UE %d Encoded %d bits (%d bytes)\n",Mod_id,UE_id,enc_rval.encoded,(enc_rval.encoded+7)/8);
#endif

  if (enc_rval.encoded==-1) {
    LOG_E(RRC,"[eNB %d] ASN1 : securityModeCommand encoding failed for UE %d\n",Mod_id,UE_id);
    return(-1);
  }

  //  rrc_ue_process_ueCapabilityEnquiry(0,1000,&dl_dcch_msg.message.choice.c1.choice.ueCapabilityEnquiry,0);
  //  exit(-1);
  return((enc_rval.encoded+7)/8);
}
uint8_t do_UECapabilityEnquiry(uint8_t Mod_id,
			       uint8_t *buffer,
			       uint8_t UE_id,
			       uint8_t Transaction_id) {

  DL_DCCH_Message_t dl_dcch_msg;

  RAT_Type_t rat=RAT_Type_eutra;
  asn_enc_rval_t enc_rval;
 
  memset(&dl_dcch_msg,0,sizeof(DL_DCCH_Message_t));

  dl_dcch_msg.message.present           = DL_DCCH_MessageType_PR_c1;
  dl_dcch_msg.message.choice.c1.present = DL_DCCH_MessageType__c1_PR_ueCapabilityEnquiry;

  dl_dcch_msg.message.choice.c1.choice.ueCapabilityEnquiry.rrc_TransactionIdentifier = Transaction_id;

  dl_dcch_msg.message.choice.c1.choice.ueCapabilityEnquiry.criticalExtensions.present = UECapabilityEnquiry__criticalExtensions_PR_c1;
  dl_dcch_msg.message.choice.c1.choice.ueCapabilityEnquiry.criticalExtensions.choice.c1.present = UECapabilityEnquiry__criticalExtensions__c1_PR_ueCapabilityEnquiry_r8;
  dl_dcch_msg.message.choice.c1.choice.ueCapabilityEnquiry.criticalExtensions.choice.c1.choice.ueCapabilityEnquiry_r8.ue_CapabilityRequest.list.count=0;
  ASN_SEQUENCE_ADD(&dl_dcch_msg.message.choice.c1.choice.ueCapabilityEnquiry.criticalExtensions.choice.c1.choice.ueCapabilityEnquiry_r8.ue_CapabilityRequest.list,
		   &rat);

#ifdef XER_PRINT
  xer_fprint(stdout, &asn_DEF_DL_DCCH_Message, (void*)&dl_dcch_msg);
#endif
  enc_rval = uper_encode_to_buffer(&asn_DEF_DL_DCCH_Message,
				   (void*)&dl_dcch_msg,
				   buffer,
				   100);
#ifdef USER_MODE
  LOG_D(RRC,"[eNB %d] UECapabilityRequest for UE %d Encoded %d bits (%d bytes)\n",Mod_id,UE_id,enc_rval.encoded,(enc_rval.encoded+7)/8);
#endif

  if (enc_rval.encoded==-1) {
    LOG_E(RRC,"[eNB %d] ASN1 : UECapabilityRequest encoding failed for UE %d\n",Mod_id,UE_id);
    return(-1);
  }

  //  rrc_ue_process_ueCapabilityEnquiry(0,1000,&dl_dcch_msg.message.choice.c1.choice.ueCapabilityEnquiry,0);
  //  exit(-1);
  return((enc_rval.encoded+7)/8);
}

uint8_t do_RRCConnectionReconfiguration(uint8_t                           Mod_id,
                                        uint8_t                          *buffer,
                                        uint8_t                           UE_id,
                                        uint8_t                           Transaction_id,
                                        SRB_ToAddModList_t                *SRB_list,
                                        DRB_ToAddModList_t                *DRB_list,
                                        DRB_ToReleaseList_t               *DRB_list2,
                                        struct SPS_Config                 *sps_Config,
                                        struct PhysicalConfigDedicated    *physicalConfigDedicated,
                                        MeasObjectToAddModList_t          *MeasObj_list,
                                        ReportConfigToAddModList_t        *ReportConfig_list,
                                        QuantityConfig_t                  *QuantityConfig,
                                        MeasIdToAddModList_t              *MeasId_list,
                                        MAC_MainConfig_t                  *mac_MainConfig,
                                        MeasGapConfig_t                   *measGapConfig,
                                        C_RNTI_t                          *cba_rnti, 
					uint8_t                           *nas_pdu,
                                        uint32_t                           nas_length
                                       ) {

  asn_enc_rval_t enc_rval;
  
  DL_DCCH_Message_t dl_dcch_msg;
  RRCConnectionReconfiguration_t *rrcConnectionReconfiguration;

  //  int i;

  memset(&dl_dcch_msg,0,sizeof(DL_DCCH_Message_t));

  dl_dcch_msg.message.present           = DL_DCCH_MessageType_PR_c1;
  dl_dcch_msg.message.choice.c1.present = DL_DCCH_MessageType__c1_PR_rrcConnectionReconfiguration;
  rrcConnectionReconfiguration          = &dl_dcch_msg.message.choice.c1.choice.rrcConnectionReconfiguration;

  // RRCConnectionReconfiguration
  rrcConnectionReconfiguration->rrc_TransactionIdentifier = Transaction_id;
  rrcConnectionReconfiguration->criticalExtensions.present = RRCConnectionReconfiguration__criticalExtensions_PR_c1;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.present =RRCConnectionReconfiguration__criticalExtensions__c1_PR_rrcConnectionReconfiguration_r8 ;

  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated = CALLOC(1,sizeof(*rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated));
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->srb_ToAddModList = SRB_list;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->drb_ToAddModList = DRB_list;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->drb_ToReleaseList = NULL;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->sps_Config = NULL;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->physicalConfigDedicated = physicalConfigDedicated;
#ifdef CBA
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->cba_RNTI_vlola= cba_rnti;
#endif 
  if (mac_MainConfig) {
    rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig = CALLOC(1,sizeof(*rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig));
    rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig->present =RadioResourceConfigDedicated__mac_MainConfig_PR_explicitValue;
    memcpy(&rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig->choice.explicitValue,
           mac_MainConfig,
           sizeof(*mac_MainConfig));
  }
  else {
    rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig=NULL;
  }

  if (MeasId_list) {
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig           = CALLOC(1,sizeof(*rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig));
  memset((void*)rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig,
         0, sizeof(*rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig));

  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig->reportConfigToAddModList = ReportConfig_list;  
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig->measIdToAddModList       = MeasId_list;  
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig->measObjectToAddModList   = MeasObj_list;  
  } 
  else {
    rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig = NULL;
  }


  // Note: RK, I'm not sure this is ok, we have to use ASN_SEQ_ADD
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.mobilityControlInfo  = NULL;
  if ((nas_pdu == NULL) || (nas_length == 0)) {
    rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.dedicatedInfoNASList = (DedicatedInfoNAS_t*)NULL;
  } else {
      DedicatedInfoNAS_t *dedicatedInfoNAS = CALLOC(1, sizeof(DedicatedInfoNAS_t));
      memset(dedicatedInfoNAS, 0, sizeof(OCTET_STRING_t));
      
      OCTET_STRING_fromBuf(dedicatedInfoNAS, nas_pdu, nas_length);

      rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.dedicatedInfoNASList = CALLOC(1, 
          sizeof(*rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.dedicatedInfoNASList));
      ASN_SEQUENCE_ADD(&rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.dedicatedInfoNASList->list, dedicatedInfoNAS);

      //DedicatedInfoNAS_t *dedicatedInfoNAS;
      //dedicatedInfoNAS = &rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.dedicatedInfoNASList;
      //dedicatedInfoNAS->buf = nas_pdu;
      //dedicatedInfoNAS->size = nas_length;
  }
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.securityConfigHO     = NULL;

  enc_rval = uper_encode_to_buffer(&asn_DEF_DL_DCCH_Message,
                                   (void*)&dl_dcch_msg,
                                   buffer,
                                   200);
#ifdef XER_PRINT
  xer_fprint(stdout,&asn_DEF_DL_DCCH_Message,(void*)&dl_dcch_msg);
#endif
  //#ifdef USER_MODE
  LOG_I(RRC,"RRCConnectionReconfiguration Encoded %d bits (%d bytes)\n",enc_rval.encoded,(enc_rval.encoded+7)/8);
  // for (i=0;i<30;i++)
  //    msg("%x.",buffer[i]);
  // msg("\n");

  //#endif

  return((enc_rval.encoded+7)/8);

}

uint8_t TMGI[5] = {5,4,0,0,1};//TMGI is a string of octet, ref. TS 24.008 fig. 10.5.4a

#ifdef Rel10
uint8_t do_MBSFNAreaConfig(LTE_DL_FRAME_PARMS *frame_parms,
			   uint8_t *buffer,
			   MCCH_Message_t *mcch_message,
			   MBSFNAreaConfiguration_r9_t **mbsfnAreaConfiguration) {

  asn_enc_rval_t enc_rval;
  MBSFN_SubframeConfig_t *mbsfn_SubframeConfig1;
  PMCH_Info_r9_t *pmch_Info_1;
  MBMS_SessionInfo_r9_t *mbms_Session_1, *mbms_Session_2;

  memset(mcch_message,0,sizeof(MCCH_Message_t));
  mcch_message->message.present	= MCCH_MessageType_PR_c1;
  mcch_message->message.choice.c1.present = MCCH_MessageType__c1_PR_mbsfnAreaConfiguration_r9;
  *mbsfnAreaConfiguration = &mcch_message->message.choice.c1.choice.mbsfnAreaConfiguration_r9;

  // Common Subframe Allocation (CommonSF-Alloc-r9)

    mbsfn_SubframeConfig1= CALLOC(1,sizeof(*mbsfn_SubframeConfig1));
    memset((void*)mbsfn_SubframeConfig1,0,sizeof(*mbsfn_SubframeConfig1));

    mbsfn_SubframeConfig1->radioframeAllocationPeriod= MBSFN_SubframeConfig__radioframeAllocationPeriod_n4;
    mbsfn_SubframeConfig1->radioframeAllocationOffset= 1;
    mbsfn_SubframeConfig1->subframeAllocation.present= MBSFN_SubframeConfig__subframeAllocation_PR_oneFrame;
    mbsfn_SubframeConfig1->subframeAllocation.choice.oneFrame.buf= MALLOC(1);
    mbsfn_SubframeConfig1->subframeAllocation.choice.oneFrame.size= 1;
    mbsfn_SubframeConfig1->subframeAllocation.choice.oneFrame.bits_unused= 2;
    if (frame_parms->frame_type == TDD) {// pattern 001110 for TDD
      mbsfn_SubframeConfig1->subframeAllocation.choice.oneFrame.buf[0]=0x0e<<2;// shift 2bits cuz 2last bits are unused.
    } 
    else {   // pattern 101010 for FDD)
      mbsfn_SubframeConfig1->subframeAllocation.choice.oneFrame.buf[0]=0x2a<<2;
    }
    ASN_SEQUENCE_ADD(&(*mbsfnAreaConfiguration)->commonSF_Alloc_r9.list,mbsfn_SubframeConfig1);

  //  commonSF-AllocPeriod-r9
  (*mbsfnAreaConfiguration)->commonSF_AllocPeriod_r9= MBSFNAreaConfiguration_r9__commonSF_AllocPeriod_r9_rf16;

  // PMCHs Information List (PMCH-InfoList-r9)
  // PMCH_1  Config
  pmch_Info_1 = CALLOC(1,sizeof(PMCH_Info_r9_t));
  memset((void*)pmch_Info_1,0,sizeof(PMCH_Info_r9_t));
  
  pmch_Info_1->pmch_Config_r9.sf_AllocEnd_r9= 11;//take the value of last mbsfn subframe in this CSA period because there is only one PMCH in this mbsfn area
  pmch_Info_1->pmch_Config_r9.dataMCS_r9= 15;
  pmch_Info_1->pmch_Config_r9.mch_SchedulingPeriod_r9= PMCH_Config_r9__mch_SchedulingPeriod_r9_rf16;

  // MBMSs-SessionInfoList-r9
  //  pmch_Info_1->mbms_SessionInfoList_r9 = CALLOC(1,sizeof(struct MBMS_SessionInfoList_r9));
  //  Session 1
  mbms_Session_1 = CALLOC(1,sizeof(MBMS_SessionInfo_r9_t));
  memset(mbms_Session_1,0,sizeof(MBMS_SessionInfo_r9_t));
  // TMGI value 
  mbms_Session_1->tmgi_r9.plmn_Id_r9.present= TMGI_r9__plmn_Id_r9_PR_plmn_Index_r9;
  mbms_Session_1->tmgi_r9.plmn_Id_r9.choice.plmn_Index_r9= 1;
  // Service ID 
  memset(&mbms_Session_1->tmgi_r9.serviceId_r9,0,sizeof(OCTET_STRING_t));// need to check
  OCTET_STRING_fromBuf(&mbms_Session_1->tmgi_r9.serviceId_r9,(const char*)&TMGI[2],3);
  // Session ID is still missing here, it can be used as an rab id or mrb id
  mbms_Session_1->sessionId_r9 = CALLOC(1,sizeof(OCTET_STRING_t));
  mbms_Session_1->sessionId_r9->buf= MALLOC(1);
  mbms_Session_1->sessionId_r9->size= 1;
  mbms_Session_1->sessionId_r9->buf[0]= MRB1; 
  // Logical Channel ID
  mbms_Session_1->logicalChannelIdentity_r9= MTCH;
  ASN_SEQUENCE_ADD(&pmch_Info_1->mbms_SessionInfoList_r9.list,mbms_Session_1);
  
  /*    //  Session 2
  //mbms_Session_2 = CALLOC(1,sizeof(MBMS_SessionInfo_r9_t));
  memset(mbms_Session_2,0,sizeof(MBMS_SessionInfo_r9_t));
  // TMGI value  
  mbms_Session_2->tmgi_r9.plmn_Id_r9.present= TMGI_r9__plmn_Id_r9_PR_plmn_Index_r9;
  mbms_Session_2->tmgi_r9.plmn_Id_r9.choice.plmn_Index_r9= 1;
  // Service ID
  memset(&mbms_Session_2->tmgi_r9.serviceId_r9,0,sizeof(OCTET_STRING_t));// need to check
  OCTET_STRING_fromBuf(&mbms_Session_2->tmgi_r9.serviceId_r9,(const char*)&TMGI[3],3);
  // Session ID is still missing here
  mbms_Session_2->sessionID_r9->buf= MALLOC(1);
  mbms_Session_2->sessionID_r9->size= 1;
  mbms_Session_2->sessionID_r9->buf[0]= 0x11; 
  // Logical Channel ID
  mbms_Session_2->logicalChannelIdentity_r9= 2;
  ASN_SEQUENCE_ADD(&pmch_Info_1->mbms_SessionInfoList_r9.list,mbms_Session_2);
 */ 
  ASN_SEQUENCE_ADD(&(*mbsfnAreaConfiguration)->pmch_InfoList_r9.list,pmch_Info_1);

#ifdef XER_PRINT
  xer_fprint(stdout,&asn_DEF_MCCH_Message,(void*)mcch_message);
#endif
  enc_rval = uper_encode_to_buffer(&asn_DEF_MCCH_Message,
				   (void*)mcch_message,
				   buffer,
				   100);
#ifdef USER_MODE
  LOG_D(RRC,"[eNB] MCCH Message Encoded %d bits (%d bytes)\n",enc_rval.encoded,(enc_rval.encoded+7)/8);
#endif
  if (enc_rval.encoded==-1) {
    msg("[RRC] ASN1 : MCCH  encoding failed for MBSFNAreaConfiguration\n");
    return(-1);
  }

  return((enc_rval.encoded+7)/8);
}
#endif

uint8_t do_MeasurementReport(uint8_t *buffer,int measid,int phy_id,int rsrp_s,int rsrq_s,int rsrp_t,int rsrq_t) {


  asn_enc_rval_t enc_rval;

  UL_DCCH_Message_t ul_dcch_msg;

  MeasurementReport_t	 *measurementReport;

  ul_dcch_msg.message.present                     = UL_DCCH_MessageType_PR_c1;
  ul_dcch_msg.message.choice.c1.present           = UL_DCCH_MessageType__c1_PR_measurementReport;
  measurementReport            = &ul_dcch_msg.message.choice.c1.choice.measurementReport;

  measurementReport->criticalExtensions.present=MeasurementReport__criticalExtensions_PR_c1;
  measurementReport->criticalExtensions.choice.c1.present=MeasurementReport__criticalExtensions__c1_PR_measurementReport_r8;
  measurementReport->criticalExtensions.choice.c1.choice.measurementReport_r8.nonCriticalExtension=CALLOC(1,sizeof(*measurementReport->criticalExtensions.choice.c1.choice.measurementReport_r8.nonCriticalExtension));


  measurementReport->criticalExtensions.choice.c1.choice.measurementReport_r8.measResults.measId=measid;
#ifdef Rel10
  measurementReport->criticalExtensions.choice.c1.choice.measurementReport_r8.measResults.measResultPCell.rsrpResult=rsrp_s;
  measurementReport->criticalExtensions.choice.c1.choice.measurementReport_r8.measResults.measResultPCell.rsrqResult=rsrq_s;
#else
  measurementReport->criticalExtensions.choice.c1.choice.measurementReport_r8.measResults.measResultServCell.rsrpResult=rsrp_s;
  measurementReport->criticalExtensions.choice.c1.choice.measurementReport_r8.measResults.measResultServCell.rsrqResult=rsrq_s;
#endif
  measurementReport->criticalExtensions.choice.c1.choice.measurementReport_r8.measResults.measResultNeighCells=CALLOC(1,sizeof(*measurementReport->criticalExtensions.choice.c1.choice.measurementReport_r8.measResults.measResultNeighCells));
  measurementReport->criticalExtensions.choice.c1.choice.measurementReport_r8.measResults.measResultNeighCells->present=MeasResults__measResultNeighCells_PR_measResultListEUTRA;

  MeasResultListEUTRA_t	 *measResultListEUTRA2;
  measResultListEUTRA2 = CALLOC(1,sizeof(*measResultListEUTRA2));

  struct MeasResultEUTRA *measresulteutra2;
  measresulteutra2 = CALLOC(1,sizeof(*measresulteutra2));
  measresulteutra2->physCellId=phy_id;//1;

  struct MeasResultEUTRA__cgi_Info *measresult_cgi2;
  measresult_cgi2 = CALLOC(1,sizeof(*measresult_cgi2));


  memset(&measresult_cgi2->cellGlobalId.plmn_Identity,0,sizeof(measresult_cgi2->cellGlobalId.plmn_Identity));

  measresult_cgi2->cellGlobalId.plmn_Identity.mcc=CALLOC(1,sizeof(measresult_cgi2->cellGlobalId.plmn_Identity.mcc));

  asn_set_empty(&measresult_cgi2->cellGlobalId.plmn_Identity.mcc->list);//.size=0;

    MCC_MNC_Digit_t dummy;
    dummy=2;ASN_SEQUENCE_ADD(&measresult_cgi2->cellGlobalId.plmn_Identity.mcc->list,&dummy);
    dummy=6;ASN_SEQUENCE_ADD(&measresult_cgi2->cellGlobalId.plmn_Identity.mcc->list,&dummy);
    dummy=2;ASN_SEQUENCE_ADD(&measresult_cgi2->cellGlobalId.plmn_Identity.mcc->list,&dummy);

    measresult_cgi2->cellGlobalId.plmn_Identity.mnc.list.size=0;
    measresult_cgi2->cellGlobalId.plmn_Identity.mnc.list.count=0;
    dummy=8;ASN_SEQUENCE_ADD(&measresult_cgi2->cellGlobalId.plmn_Identity.mnc.list,&dummy);
    dummy=0;ASN_SEQUENCE_ADD(&measresult_cgi2->cellGlobalId.plmn_Identity.mnc.list,&dummy);


    measresult_cgi2->cellGlobalId.cellIdentity.buf=MALLOC(8);
    measresult_cgi2->cellGlobalId.cellIdentity.buf[0]=0x01;
    measresult_cgi2->cellGlobalId.cellIdentity.buf[1]=0x48;
    measresult_cgi2->cellGlobalId.cellIdentity.buf[2]=0x0f;
    measresult_cgi2->cellGlobalId.cellIdentity.buf[3]=0x03;
    measresult_cgi2->cellGlobalId.cellIdentity.size=4;
    measresult_cgi2->cellGlobalId.cellIdentity.bits_unused=4;

    measresult_cgi2->trackingAreaCode.buf = MALLOC(2);
    measresult_cgi2->trackingAreaCode.buf[0]=0x00;
    measresult_cgi2->trackingAreaCode.buf[1]=0x10;
    measresult_cgi2->trackingAreaCode.size=2;
    measresult_cgi2->trackingAreaCode.bits_unused=0;


    measresulteutra2->cgi_Info=measresult_cgi2;
    struct MeasResultEUTRA__measResult meas2;
    //    int rsrp_va=10;
    meas2.rsrpResult=&rsrp_t;
    		//&rsrp_va;
    meas2.rsrqResult=&rsrq_t;

    measresulteutra2->measResult=meas2;

    ASN_SEQUENCE_ADD(&measResultListEUTRA2->list,measresulteutra2);

    measurementReport->criticalExtensions.choice.c1.choice.measurementReport_r8.measResults.measResultNeighCells->choice.measResultListEUTRA=*(measResultListEUTRA2);


  enc_rval = uper_encode_to_buffer(&asn_DEF_UL_DCCH_Message,
				   (void*)&ul_dcch_msg,
				   buffer,
				   100);

#ifdef USER_MODE
  printf("Measurement Report Encoded %d bits (%d bytes)\n",enc_rval.encoded,(enc_rval.encoded+7)/8);
#endif

  return((enc_rval.encoded+7)/8);
}

OAI_UECapability_t UECapability;
SupportedBandEUTRA_t Bandlist[4];
BandInfoEUTRA_t BandInfo_meas[4];
InterFreqBandInfo_t InterFreqBandInfo[4][4];
BandInfoEUTRA_t BandInfoEUTRA[4];

OAI_UECapability_t *fill_ue_capability() {

  UE_EUTRA_Capability_t *UE_EUTRA_Capability;
  asn_enc_rval_t enc_rval;
  long maxNumberROHC_ContextSessions = PDCP_Parameters__maxNumberROHC_ContextSessions_cs16;
  int i;

  Bandlist[0].bandEUTRA  = 33;  // 1900-1920 TDD
  Bandlist[0].halfDuplex = 0;
  Bandlist[1].bandEUTRA  = 38;  // 2570-2620 TDD 

  Bandlist[1].halfDuplex = 0;
  Bandlist[2].bandEUTRA  = 5;   // 824-849 , 869-894 FDD
  Bandlist[2].halfDuplex = 0;
  Bandlist[3].bandEUTRA  = 7;   // 2500-2570, 2620-2690 FDD
  Bandlist[3].halfDuplex = 0;

  memset((void*)&InterFreqBandInfo,0,sizeof(InterFreqBandList_t));

  memset((void*)&BandInfoEUTRA[0],0,sizeof(BandInfoEUTRA_t));
  memset((void*)&BandInfoEUTRA[1],0,sizeof(BandInfoEUTRA_t));
  memset((void*)&BandInfoEUTRA[2],0,sizeof(BandInfoEUTRA_t));
  memset((void*)&BandInfoEUTRA[3],0,sizeof(BandInfoEUTRA_t));

  InterFreqBandInfo[0][0].interFreqNeedForGaps = 0;
  InterFreqBandInfo[0][1].interFreqNeedForGaps = 1;
  InterFreqBandInfo[0][2].interFreqNeedForGaps = 1;
  InterFreqBandInfo[0][3].interFreqNeedForGaps = 1;
  InterFreqBandInfo[1][0].interFreqNeedForGaps = 1;
  InterFreqBandInfo[1][1].interFreqNeedForGaps = 0;
  InterFreqBandInfo[1][2].interFreqNeedForGaps = 1;
  InterFreqBandInfo[1][3].interFreqNeedForGaps = 1;
  InterFreqBandInfo[2][0].interFreqNeedForGaps = 1;
  InterFreqBandInfo[2][1].interFreqNeedForGaps = 1;
  InterFreqBandInfo[2][2].interFreqNeedForGaps = 0;
  InterFreqBandInfo[2][3].interFreqNeedForGaps = 1;
  InterFreqBandInfo[3][0].interFreqNeedForGaps = 1;
  InterFreqBandInfo[3][1].interFreqNeedForGaps = 1;
  InterFreqBandInfo[3][2].interFreqNeedForGaps = 1;
  InterFreqBandInfo[3][3].interFreqNeedForGaps = 0;


  
  LOG_I(RRC,"Allocating %d bytes for UE_EUTRA_Capability\n",sizeof(*UE_EUTRA_Capability));
  UE_EUTRA_Capability = CALLOC(1,sizeof(*UE_EUTRA_Capability));
  memset(UE_EUTRA_Capability,0,sizeof(*UE_EUTRA_Capability));

  //  UE_EUTRA_Capability->accessStratumRelease = 0;//AccessStratumRelease_rel8;
  UE_EUTRA_Capability->ue_Category          = 4;
  UE_EUTRA_Capability->pdcp_Parameters.supportedROHC_Profiles.profile0x0001=0;
  UE_EUTRA_Capability->pdcp_Parameters.supportedROHC_Profiles.profile0x0002=0;
  UE_EUTRA_Capability->pdcp_Parameters.supportedROHC_Profiles.profile0x0003=0;
  UE_EUTRA_Capability->pdcp_Parameters.supportedROHC_Profiles.profile0x0004=0;
  UE_EUTRA_Capability->pdcp_Parameters.supportedROHC_Profiles.profile0x0006=0;
  UE_EUTRA_Capability->pdcp_Parameters.supportedROHC_Profiles.profile0x0101=0;    
  UE_EUTRA_Capability->pdcp_Parameters.supportedROHC_Profiles.profile0x0102=0;    
  UE_EUTRA_Capability->pdcp_Parameters.supportedROHC_Profiles.profile0x0103=0;    
  UE_EUTRA_Capability->pdcp_Parameters.supportedROHC_Profiles.profile0x0104=0;    

  UE_EUTRA_Capability->pdcp_Parameters.maxNumberROHC_ContextSessions = &maxNumberROHC_ContextSessions;

  UE_EUTRA_Capability->phyLayerParameters.ue_TxAntennaSelectionSupported = 0;
  UE_EUTRA_Capability->phyLayerParameters.ue_SpecificRefSigsSupported    = 0;
  UE_EUTRA_Capability->rf_Parameters.supportedBandListEUTRA.list.count                          = 0;
  ASN_SEQUENCE_ADD(&UE_EUTRA_Capability->rf_Parameters.supportedBandListEUTRA.list,(void*)&Bandlist[0]);
  ASN_SEQUENCE_ADD(&UE_EUTRA_Capability->rf_Parameters.supportedBandListEUTRA.list,(void*)&Bandlist[1]);
  ASN_SEQUENCE_ADD(&UE_EUTRA_Capability->rf_Parameters.supportedBandListEUTRA.list,(void*)&Bandlist[2]);
  ASN_SEQUENCE_ADD(&UE_EUTRA_Capability->rf_Parameters.supportedBandListEUTRA.list,(void*)&Bandlist[3]);

  ASN_SEQUENCE_ADD(&UE_EUTRA_Capability->measParameters.bandListEUTRA.list,(void*)&BandInfoEUTRA[0]);
  ASN_SEQUENCE_ADD(&UE_EUTRA_Capability->measParameters.bandListEUTRA.list,(void*)&BandInfoEUTRA[1]);
  ASN_SEQUENCE_ADD(&UE_EUTRA_Capability->measParameters.bandListEUTRA.list,(void*)&BandInfoEUTRA[2]);
  ASN_SEQUENCE_ADD(&UE_EUTRA_Capability->measParameters.bandListEUTRA.list,(void*)&BandInfoEUTRA[3]);

  ASN_SEQUENCE_ADD(&UE_EUTRA_Capability->measParameters.bandListEUTRA.list.array[0]->interFreqBandList.list,(void*)&InterFreqBandInfo[0][0]);
  ASN_SEQUENCE_ADD(&UE_EUTRA_Capability->measParameters.bandListEUTRA.list.array[0]->interFreqBandList.list,(void*)&InterFreqBandInfo[0][1]);
  ASN_SEQUENCE_ADD(&UE_EUTRA_Capability->measParameters.bandListEUTRA.list.array[0]->interFreqBandList.list,(void*)&InterFreqBandInfo[0][2]);
  ASN_SEQUENCE_ADD(&UE_EUTRA_Capability->measParameters.bandListEUTRA.list.array[0]->interFreqBandList.list,(void*)&InterFreqBandInfo[0][3]);

  ASN_SEQUENCE_ADD(&UE_EUTRA_Capability->measParameters.bandListEUTRA.list.array[1]->interFreqBandList.list,(void*)&InterFreqBandInfo[1][0]);
  ASN_SEQUENCE_ADD(&UE_EUTRA_Capability->measParameters.bandListEUTRA.list.array[1]->interFreqBandList.list,(void*)&InterFreqBandInfo[1][1]);
  ASN_SEQUENCE_ADD(&UE_EUTRA_Capability->measParameters.bandListEUTRA.list.array[1]->interFreqBandList.list,(void*)&InterFreqBandInfo[1][2]);
  ASN_SEQUENCE_ADD(&UE_EUTRA_Capability->measParameters.bandListEUTRA.list.array[1]->interFreqBandList.list,(void*)&InterFreqBandInfo[1][3]);

  ASN_SEQUENCE_ADD(&UE_EUTRA_Capability->measParameters.bandListEUTRA.list.array[2]->interFreqBandList.list,(void*)&InterFreqBandInfo[2][0]);
  ASN_SEQUENCE_ADD(&UE_EUTRA_Capability->measParameters.bandListEUTRA.list.array[2]->interFreqBandList.list,(void*)&InterFreqBandInfo[2][1]);
  ASN_SEQUENCE_ADD(&UE_EUTRA_Capability->measParameters.bandListEUTRA.list.array[2]->interFreqBandList.list,(void*)&InterFreqBandInfo[2][2]);
  ASN_SEQUENCE_ADD(&UE_EUTRA_Capability->measParameters.bandListEUTRA.list.array[2]->interFreqBandList.list,(void*)&InterFreqBandInfo[2][3]);

  ASN_SEQUENCE_ADD(&UE_EUTRA_Capability->measParameters.bandListEUTRA.list.array[3]->interFreqBandList.list,(void*)&InterFreqBandInfo[3][0]);
  ASN_SEQUENCE_ADD(&UE_EUTRA_Capability->measParameters.bandListEUTRA.list.array[3]->interFreqBandList.list,(void*)&InterFreqBandInfo[3][1]);
  ASN_SEQUENCE_ADD(&UE_EUTRA_Capability->measParameters.bandListEUTRA.list.array[3]->interFreqBandList.list,(void*)&InterFreqBandInfo[3][2]);
  ASN_SEQUENCE_ADD(&UE_EUTRA_Capability->measParameters.bandListEUTRA.list.array[3]->interFreqBandList.list,(void*)&InterFreqBandInfo[3][3]);

  // UE_EUTRA_Capability->measParameters.bandListEUTRA.list.count                         = 0;  // no measurements on other bands
  // UE_EUTRA_Capability->featureGroupIndicators  // null
  // UE_EUTRA_Capability->interRAT_Parameters     // null
  
  UECapability.UE_EUTRA_Capability = UE_EUTRA_Capability;
#ifdef XER_PRINT
  xer_fprint(stdout,&asn_DEF_UE_EUTRA_Capability,(void *)UE_EUTRA_Capability);
#endif
  enc_rval = uper_encode_to_buffer(&asn_DEF_UE_EUTRA_Capability,
				   (void*)UE_EUTRA_Capability,
				   &UECapability.sdu[0],
  				   MAX_UE_CAPABILITY_SIZE);
  UECapability.sdu_size = (enc_rval.encoded+7)/8;
  LOG_I(PHY,"[RRC]UE Capability encoded, %d bytes (%d bits)\n",UECapability.sdu_size,enc_rval.encoded+7);
  for (i=0;i<UECapability.sdu_size;i++)
    printf("%02x.",UECapability.sdu[i]);
  printf("\n");
  return(&UECapability);
}

/************************************
 CELLULAR ADAPTED FUNCTION
 ***********************************/
//-----------------------------------------------------------------------------
uint8_t do_SIB1_TDD_config_cell (LTE_DL_FRAME_PARMS *frame_parms,TDD_Config_t *tdd_Config){
//-----------------------------------------------------------------------------
  tdd_Config->subframeAssignment=frame_parms->tdd_config; //TDD_Config__subframeAssignment_sa3;
  tdd_Config->specialSubframePatterns=0;//frame_parms->tdd_config_S;//TDD_Config__specialSubframePatterns_ssp0;
  //msg ("ASN1 TDD_config %d\n", frame_parms->tdd_config);
return 0;
}

//-----------------------------------------------------------------------------
// Build SIB2 for RRC Cellular (extracted from do_SIB23)
uint8_t do_SIB2_cell(uint8_t Mod_id, LTE_DL_FRAME_PARMS *frame_parms, uint8_t *buffer,
                  BCCH_DL_SCH_Message_t *bcch_message, SystemInformationBlockType2_t **sib2) {
//-----------------------------------------------------------------------------
  struct SystemInformation_r8_IEs__sib_TypeAndInfo__Member *sib2_part;
  asn_enc_rval_t enc_rval;

  if (bcch_message) 
    memset(bcch_message,0,sizeof(BCCH_DL_SCH_Message_t));
  else {
    LOG_E(RRC,"BCCH_MESSAGE is null, exiting\n");
    exit(-1);
  }

  if (!sib2) {
    LOG_E(RRC,"sib2 is null, exiting\n");
    exit(-1);
  }
  LOG_I(RRC,"Configuration SIB2 CELLULAR\n");

  sib2_part = CALLOC(1,sizeof(struct SystemInformation_r8_IEs__sib_TypeAndInfo__Member));
  memset(sib2_part,0,sizeof(struct SystemInformation_r8_IEs__sib_TypeAndInfo__Member));
  sib2_part->present = SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib2;
  *sib2 = &sib2_part->choice.sib2;

  // sib2

  (*sib2)->ac_BarringInfo = NULL;
#ifdef Rel10
  (*sib2)->ssac_BarringForMMTEL_Voice_r9 = NULL;
  (*sib2)->ssac_BarringForMMTEL_Video_r9 = NULL;
  (*sib2)->ac_BarringForCSFB_r10 = NULL;
#endif

  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.preambleInfo.numberOfRA_Preambles=RACH_ConfigCommon__preambleInfo__numberOfRA_Preambles_n64;
  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.preambleInfo.preamblesGroupAConfig = NULL;
  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.powerRampingParameters.powerRampingStep=RACH_ConfigCommon__powerRampingParameters__powerRampingStep_dB2;

#ifdef EXMIMO
  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.powerRampingParameters.preambleInitialReceivedTargetPower=RACH_ConfigCommon__powerRampingParameters__preambleInitialReceivedTargetPower_dBm_100;
#else
  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.powerRampingParameters.preambleInitialReceivedTargetPower=RACH_ConfigCommon__powerRampingParameters__preambleInitialReceivedTargetPower_dBm_108;
#endif
  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.preambleTransMax=RACH_ConfigCommon__ra_SupervisionInfo__preambleTransMax_n10;

  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.ra_ResponseWindowSize=RACH_ConfigCommon__ra_SupervisionInfo__ra_ResponseWindowSize_sf10;

  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.mac_ContentionResolutionTimer=RACH_ConfigCommon__ra_SupervisionInfo__mac_ContentionResolutionTimer_sf48;

  (*sib2)->radioResourceConfigCommon.rach_ConfigCommon.maxHARQ_Msg3Tx = 4;

  // BCCH-Config

  (*sib2)->radioResourceConfigCommon.bcch_Config.modificationPeriodCoeff=BCCH_Config__modificationPeriodCoeff_n2;

  // PCCH-Config

  (*sib2)->radioResourceConfigCommon.pcch_Config.defaultPagingCycle=PCCH_Config__defaultPagingCycle_rf128;


  (*sib2)->radioResourceConfigCommon.pcch_Config.nB=PCCH_Config__nB_oneT;

  // PRACH-Config
  (*sib2)->radioResourceConfigCommon.prach_Config.rootSequenceIndex=Mod_id;//0;//384;
  (*sib2)->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.prach_ConfigIndex = 0;//3;
  (*sib2)->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.highSpeedFlag = 0;
  (*sib2)->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.zeroCorrelationZoneConfig = 1;//12;
  (*sib2)->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.prach_FreqOffset = 2;

  // PDSCH-Config
#ifdef EXMIMO
  (*sib2)->radioResourceConfigCommon.pdsch_ConfigCommon.referenceSignalPower=0;
#else
  (*sib2)->radioResourceConfigCommon.pdsch_ConfigCommon.referenceSignalPower=15;
#endif
  if (frame_parms->mode1_flag==1)
    (*sib2)->radioResourceConfigCommon.pdsch_ConfigCommon.p_b=0;
  else
    (*sib2)->radioResourceConfigCommon.pdsch_ConfigCommon.p_b=1;


  // PUSCH-Config
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.n_SB=1;
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.hoppingMode=PUSCH_ConfigCommon__pusch_ConfigBasic__hoppingMode_interSubFrame;
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.pusch_HoppingOffset=0;
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.enable64QAM=0;
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupHoppingEnabled=1;
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH=0;
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled=0;
  (*sib2)->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.cyclicShift=0;

  // PUCCH-Config

  (*sib2)->radioResourceConfigCommon.pucch_ConfigCommon.deltaPUCCH_Shift=PUCCH_ConfigCommon__deltaPUCCH_Shift_ds1;
  (*sib2)->radioResourceConfigCommon.pucch_ConfigCommon.nRB_CQI = 1;
  (*sib2)->radioResourceConfigCommon.pucch_ConfigCommon.nCS_AN = 0;
  (*sib2)->radioResourceConfigCommon.pucch_ConfigCommon.n1PUCCH_AN = 32;


  (*sib2)->radioResourceConfigCommon.soundingRS_UL_ConfigCommon.present=SoundingRS_UL_ConfigCommon_PR_release;
  (*sib2)->radioResourceConfigCommon.soundingRS_UL_ConfigCommon.choice.release=0;

  // uplinkPowerControlCommon

#ifdef EXMIMO
  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.p0_NominalPUSCH = -90;
  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.p0_NominalPUCCH = -96;
#else
  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.p0_NominalPUSCH = -108;
  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.p0_NominalPUCCH = -108;
#endif
  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.alpha=UplinkPowerControlCommon__alpha_al1;
  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format1=DeltaFList_PUCCH__deltaF_PUCCH_Format1_deltaF2;
  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format1b=DeltaFList_PUCCH__deltaF_PUCCH_Format1b_deltaF3;

  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2=DeltaFList_PUCCH__deltaF_PUCCH_Format2_deltaF0;

  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2a=DeltaFList_PUCCH__deltaF_PUCCH_Format2a_deltaF0;

  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2b=DeltaFList_PUCCH__deltaF_PUCCH_Format2b_deltaF0;

  (*sib2)->radioResourceConfigCommon.uplinkPowerControlCommon.deltaPreambleMsg3 = 6;

  (*sib2)->radioResourceConfigCommon.ul_CyclicPrefixLength=UL_CyclicPrefixLength_len1;

  (*sib2)->ue_TimersAndConstants.t300=UE_TimersAndConstants__t300_ms1000;

  (*sib2)->ue_TimersAndConstants.t301=UE_TimersAndConstants__t301_ms1000;

  (*sib2)->ue_TimersAndConstants.t310=UE_TimersAndConstants__t310_ms1000;

  (*sib2)->ue_TimersAndConstants.n310=UE_TimersAndConstants__n310_n20;

  (*sib2)->ue_TimersAndConstants.t311=UE_TimersAndConstants__t311_ms10000;

  (*sib2)->ue_TimersAndConstants.n311=UE_TimersAndConstants__n311_n1;

  (*sib2)->freqInfo.additionalSpectrumEmission = 1;
  (*sib2)->freqInfo.ul_CarrierFreq = NULL;
  (*sib2)->freqInfo.ul_Bandwidth = NULL;
  //  (*sib2)->mbsfn_SubframeConfigList = NULL;

  (*sib2)->mbsfn_SubframeConfigList = NULL;
  (*sib2)->timeAlignmentTimerCommon=TimeAlignmentTimer_sf5120;
  // end fill sib2

  bcch_message->message.present = BCCH_DL_SCH_MessageType_PR_c1;
  bcch_message->message.choice.c1.present = BCCH_DL_SCH_MessageType__c1_PR_systemInformation;

  bcch_message->message.choice.c1.choice.systemInformation.criticalExtensions.present = SystemInformation__criticalExtensions_PR_systemInformation_r8;

  bcch_message->message.choice.c1.choice.systemInformation.criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list.count=0;

  //  asn_set_empty(&systemInformation->criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list);//.size=0;
  //  systemInformation->criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list.count=0;
  ASN_SEQUENCE_ADD(&bcch_message->message.choice.c1.choice.systemInformation.criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list,
		   sib2_part);

#ifdef XER_PRINT
  xer_fprint(stdout, &asn_DEF_BCCH_DL_SCH_Message, (void*)bcch_message);
#endif
  enc_rval = uper_encode_to_buffer(&asn_DEF_BCCH_DL_SCH_Message,
				   (void*)bcch_message,
				   buffer,
				   900);
#ifdef USER_MODE
  LOG_D(RRC,"[eNB] SystemInformation Encoded %d bits (%d bytes)\n",enc_rval.encoded,(enc_rval.encoded+7)/8);
#endif

  if (enc_rval.encoded==-1) {
    msg("[RRC] ASN1 : SI encoding failed for SIB2 Cell\n");
    return(-1);
  }
  return((enc_rval.encoded+7)/8);
}

//-----------------------------------------------------------------------------
uint8_t do_RRCConnReconf_defaultCELL(uint8_t Mod_id,
                                             uint8_t *buffer,
                                             uint8_t UE_id,
                                             uint8_t Transaction_id,
 /*                                            struct SRB_ToAddMod **SRB2_config,
                                             struct DRB_ToAddMod **DRB_config,*/
                                             SRB_ToAddModList_t  **SRB_configList,
                                             DRB_ToAddModList_t  **DRB_configList,
                                             struct PhysicalConfigDedicated  **physicalConfigDedicated) {
//-----------------------------------------------------------------------------

  asn_enc_rval_t enc_rval;
  /*
  struct SRB_ToAddMod **SRB2_config                         = &rrc_inst->SRB2_config[UE_id];
  struct DRB_ToAddMod **DRB_config                          = &rrc_inst->DRB_config[UE_id][0];
  struct PhysicalConfigDedicated  **physicalConfigDedicated = &rrc_inst->physicalConfigDedicated[UE_id]; 
  */

  struct SRB_ToAddMod *SRB2_config2;
  struct SRB_ToAddMod__rlc_Config *SRB2_rlc_config;
  struct SRB_ToAddMod__logicalChannelConfig *SRB2_lchan_config;
  struct LogicalChannelConfig__ul_SpecificParameters *SRB2_ul_SpecificParameters;
  SRB_ToAddModList_t *SRB_configList2;

  //struct DRB_ToAddMod **DRB_config;
  struct DRB_ToAddMod *DRB_config2;
  struct RLC_Config *DRB_rlc_config;
  struct PDCP_Config *DRB_pdcp_config;
  struct PDCP_Config__rlc_UM *PDCP_rlc_UM;
  struct LogicalChannelConfig *DRB_lchan_config;
  struct LogicalChannelConfig__ul_SpecificParameters *DRB_ul_SpecificParameters;
  //   DRB_ToAddModList_t *DRB_list;

  MeasObjectToAddModList_t *MeasObj_list;
  MeasObjectToAddMod_t *MeasObj;
  ReportConfigToAddModList_t *ReportConfig_list;
  ReportConfigToAddMod_t *ReportConfig_per,*ReportConfig_A1,*ReportConfig_A2,*ReportConfig_A3,*ReportConfig_A4,*ReportConfig_A5;
  MeasIdToAddModList_t *MeasId_list;
  MeasIdToAddMod_t *MeasId0,*MeasId1,*MeasId2,*MeasId3,*MeasId4,*MeasId5;
#ifdef Rel10
  long * sr_ProhibitTimer_r9;
#endif
#if CBA
  struct PUSCH_CBAConfigDedicated_vlola  *pusch_CBAConfigDedicated_vlola;
#endif

  long *logicalchannelgroup,*logicalchannelgroup_drb;
  long *maxHARQ_Tx, *periodicBSR_Timer;
  DL_DCCH_Message_t dl_dcch_msg;

  RRCConnectionReconfiguration_t *rrcConnectionReconfiguration;
//  long *lcid;

//  RSRP_Range_t *rsrp;
//  struct MeasConfig__speedStatePars *Sparams;
  CellsToAddMod_t *CellToAdd;
  CellsToAddModList_t *CellsToAddModList;
  int i;

  memset(&dl_dcch_msg,0,sizeof(DL_DCCH_Message_t));

  dl_dcch_msg.message.present           = DL_DCCH_MessageType_PR_c1;
  dl_dcch_msg.message.choice.c1.present = DL_DCCH_MessageType__c1_PR_rrcConnectionReconfiguration;
  rrcConnectionReconfiguration          = &dl_dcch_msg.message.choice.c1.choice.rrcConnectionReconfiguration;

  // RRCConnectionReconfiguration
  // Configure SRB2

  //SRB_list = CALLOC(1,sizeof(*SRB_list));

  // SRB2
  SRB2_config2 = CALLOC(1,sizeof(*SRB2_config2));
  //*SRB2_config = SRB2_config2;
  SRB_configList2 = CALLOC(1,sizeof(*SRB_configList2));
  memset(SRB_configList2,0,sizeof(*SRB_configList2));

  SRB2_config2->srb_Identity = 2;
  SRB2_rlc_config = CALLOC(1,sizeof(*SRB2_rlc_config));
  SRB2_config2->rlc_Config   = SRB2_rlc_config;

  SRB2_rlc_config->present = SRB_ToAddMod__rlc_Config_PR_explicitValue;
  /* Used for standard srb2
  SRB2_rlc_config->choice.explicitValue.present=RLC_Config_PR_am;
  SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.t_PollRetransmit = T_PollRetransmit_ms15;
  SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.pollPDU          = PollPDU_p8;
  SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.pollByte         = PollByte_kB1000;
  SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.maxRetxThreshold = UL_AM_RLC__maxRetxThreshold_t32;
  SRB2_rlc_config->choice.explicitValue.choice.am.dl_AM_RLC.t_Reordering     = T_Reordering_ms50;
  SRB2_rlc_config->choice.explicitValue.choice.am.dl_AM_RLC.t_StatusProhibit = T_StatusProhibit_ms10;
  */
  SRB2_rlc_config->choice.explicitValue.present=RLC_Config_PR_um_Bi_Directional;
  SRB2_rlc_config->choice.explicitValue.choice.um_Bi_Directional.ul_UM_RLC.sn_FieldLength   = SN_FieldLength_size10;
  SRB2_rlc_config->choice.explicitValue.choice.um_Bi_Directional.dl_UM_RLC.sn_FieldLength   = SN_FieldLength_size10;
  SRB2_rlc_config->choice.explicitValue.choice.um_Bi_Directional.dl_UM_RLC.t_Reordering     = T_Reordering_ms5;


  SRB2_lchan_config = CALLOC(1,sizeof(*SRB2_lchan_config));
  SRB2_config2->logicalChannelConfig   = SRB2_lchan_config;

  SRB2_lchan_config->present                                    = SRB_ToAddMod__logicalChannelConfig_PR_explicitValue;


  SRB2_ul_SpecificParameters = CALLOC(1,sizeof(*SRB2_ul_SpecificParameters));

  SRB2_ul_SpecificParameters->priority           = 1;
  SRB2_ul_SpecificParameters->prioritisedBitRate = LogicalChannelConfig__ul_SpecificParameters__prioritisedBitRate_infinity;
  SRB2_ul_SpecificParameters->bucketSizeDuration = LogicalChannelConfig__ul_SpecificParameters__bucketSizeDuration_ms50;

  logicalchannelgroup = CALLOC(1,sizeof(long));
  *logicalchannelgroup=0;

  SRB2_ul_SpecificParameters->logicalChannelGroup = logicalchannelgroup;

  SRB2_lchan_config->choice.explicitValue.ul_SpecificParameters = SRB2_ul_SpecificParameters;
  //ASN_SEQUENCE_ADD(&SRB_list->list,SRB2_config2);
  //ASN_SEQUENCE_ADD(&SRB_configList->list,SRB2_config2);
  ASN_SEQUENCE_ADD(&((*SRB_configList)->list),SRB2_config2);
  ASN_SEQUENCE_ADD(&SRB_configList2->list,SRB2_config2);

  // Configure DRB

  //DRB_list = CALLOC(1,sizeof(*DRB_list));
  *DRB_configList = CALLOC(1,sizeof(*DRB_configList));

  /// DRB
  DRB_config2 = CALLOC(1,sizeof(*DRB_config2));
  //*DRB_config = DRB_config2;

  //DRB_config2->drb_Identity = 1;
  DRB_config2->drb_Identity = (DRB_Identity_t) (UE_id+1); //allowed values 1..32
  //lcid = CALLOC(1,sizeof(*lcid));
  //*lcid = 3;
  //DRB_config2->logicalChannelIdentity = lcid;
  DRB_config2->logicalChannelIdentity = CALLOC(1,sizeof(long));
  *(DRB_config2->logicalChannelIdentity) = (long) 3;

  DRB_rlc_config = CALLOC(1,sizeof(*DRB_rlc_config));
  DRB_config2->rlc_Config   = DRB_rlc_config;
  DRB_rlc_config->present=RLC_Config_PR_um_Bi_Directional;
  DRB_rlc_config->choice.um_Bi_Directional.ul_UM_RLC.sn_FieldLength=SN_FieldLength_size10;
  DRB_rlc_config->choice.um_Bi_Directional.dl_UM_RLC.sn_FieldLength=SN_FieldLength_size10;
  DRB_rlc_config->choice.um_Bi_Directional.dl_UM_RLC.t_Reordering=T_Reordering_ms5;

  DRB_pdcp_config = CALLOC(1,sizeof(*DRB_pdcp_config));
  DRB_config2->pdcp_Config   = DRB_pdcp_config;
  DRB_pdcp_config->discardTimer=NULL;
  DRB_pdcp_config->rlc_AM = NULL;
  PDCP_rlc_UM = CALLOC(1, sizeof(*PDCP_rlc_UM));
  DRB_pdcp_config->rlc_UM =PDCP_rlc_UM;  
  PDCP_rlc_UM->pdcp_SN_Size = PDCP_Config__rlc_UM__pdcp_SN_Size_len12bits;
  DRB_pdcp_config->headerCompression.present = PDCP_Config__headerCompression_PR_notUsed;

  DRB_lchan_config = CALLOC(1,sizeof(*DRB_lchan_config));
  DRB_config2->logicalChannelConfig   = DRB_lchan_config;
  DRB_ul_SpecificParameters = CALLOC(1,sizeof(*DRB_ul_SpecificParameters));
  DRB_lchan_config->ul_SpecificParameters = DRB_ul_SpecificParameters;


  DRB_ul_SpecificParameters->priority = 2; // lower priority than srb1, srb2
  DRB_ul_SpecificParameters->prioritisedBitRate=LogicalChannelConfig__ul_SpecificParameters__prioritisedBitRate_infinity;
  DRB_ul_SpecificParameters->bucketSizeDuration=LogicalChannelConfig__ul_SpecificParameters__bucketSizeDuration_ms50;

 // LCG for DTCH can take the value from 1 to 3 as defined in 36331: normally controlled by upper layers (like RRM)
  logicalchannelgroup_drb = CALLOC(1,sizeof(long));
  *logicalchannelgroup_drb=1;
  DRB_ul_SpecificParameters->logicalChannelGroup = logicalchannelgroup_drb;


  //ASN_SEQUENCE_ADD(&DRB_list->list,DRB_config2);
  ASN_SEQUENCE_ADD(&(*DRB_configList)->list,DRB_config2);

  rrcConnectionReconfiguration->rrc_TransactionIdentifier = Transaction_id;
  rrcConnectionReconfiguration->criticalExtensions.present = RRCConnectionReconfiguration__criticalExtensions_PR_c1;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.present =RRCConnectionReconfiguration__criticalExtensions__c1_PR_rrcConnectionReconfiguration_r8 ;

  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated = CALLOC(1,sizeof(*rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated));
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->srb_ToAddModList = (SRB_ToAddModList_t *)*SRB_configList;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->drb_ToAddModList = (DRB_ToAddModList_t *)*DRB_configList;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->drb_ToReleaseList = NULL;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->sps_Config = NULL;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->physicalConfigDedicated = NULL;
  //rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig = NULL;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig = CALLOC(1,sizeof(*rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig));

  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig->present =RadioResourceConfigDedicated__mac_MainConfig_PR_explicitValue;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig->choice.explicitValue.ul_SCH_Config = CALLOC(1,sizeof(*rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig->choice.explicitValue.ul_SCH_Config));
  
  maxHARQ_Tx = CALLOC(1,sizeof(long));
  *maxHARQ_Tx=MAC_MainConfig__ul_SCH_Config__maxHARQ_Tx_n5;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig->choice.explicitValue.ul_SCH_Config->maxHARQ_Tx = maxHARQ_Tx;
  
  periodicBSR_Timer = CALLOC(1,sizeof(long));
  *periodicBSR_Timer = MAC_MainConfig__ul_SCH_Config__periodicBSR_Timer_sf64;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig->choice.explicitValue.ul_SCH_Config->periodicBSR_Timer =  periodicBSR_Timer;

  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig->choice.explicitValue.ul_SCH_Config->retxBSR_Timer =  MAC_MainConfig__ul_SCH_Config__retxBSR_Timer_sf320;

  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig->choice.explicitValue.ul_SCH_Config->ttiBundling=0; // FALSE

  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig->choice.explicitValue.drx_Config = NULL;

  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig->choice.explicitValue.phr_Config = CALLOC(1,sizeof(*rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig->choice.explicitValue.phr_Config));
  
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig->choice.explicitValue.phr_Config->present = MAC_MainConfig__phr_Config_PR_setup;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig->choice.explicitValue.phr_Config->choice.setup.periodicPHR_Timer= MAC_MainConfig__phr_Config__setup__periodicPHR_Timer_sf20; // sf20 = 20 subframes

  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig->choice.explicitValue.phr_Config->choice.setup.prohibitPHR_Timer=MAC_MainConfig__phr_Config__setup__prohibitPHR_Timer_sf20; // sf20 = 20 subframes

  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig->choice.explicitValue.phr_Config->choice.setup.dl_PathlossChange=MAC_MainConfig__phr_Config__setup__dl_PathlossChange_dB1; // Value dB1 =1 dB, dB3 = 3 dB

#ifdef Rel10
  sr_ProhibitTimer_r9 = CALLOC(1,sizeof(long));
  *sr_ProhibitTimer_r9=0; // SR tx on PUCCH, Value in number of SR period(s). Value 0 = no timer for SR, Value 2= 2*SR 
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->mac_MainConfig->choice.explicitValue.sr_ProhibitTimer_r9=sr_ProhibitTimer_r9;
#endif
#ifdef CBA  
 rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated->sps_CBA_ConfigList_vlola = NULL;
#endif
  //  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig           = NULL;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig           = CALLOC(1,sizeof(*rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig));
  memset((void*)rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig,
	 0,sizeof(*rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig));


  // Measurement ID list
  MeasId_list       = CALLOC(1,sizeof(*MeasId_list));
  memset((void *)MeasId_list,0,sizeof(*MeasId_list));

  MeasId0            = CALLOC(1,sizeof(*MeasId0));
  MeasId0->measId = 1;
  MeasId0->measObjectId = 1;
  MeasId0->reportConfigId = 1;
  ASN_SEQUENCE_ADD(&MeasId_list->list,MeasId0);

  MeasId1            = CALLOC(1,sizeof(*MeasId1));
  MeasId1->measId = 2;
  MeasId1->measObjectId = 1;
  MeasId1->reportConfigId = 2;
  ASN_SEQUENCE_ADD(&MeasId_list->list,MeasId1);

  MeasId2            = CALLOC(1,sizeof(*MeasId2));
  MeasId2->measId = 3;
  MeasId2->measObjectId = 1;
  MeasId2->reportConfigId = 3;
  ASN_SEQUENCE_ADD(&MeasId_list->list,MeasId2);

  MeasId3            = CALLOC(1,sizeof(*MeasId3));
  MeasId3->measId = 4;
  MeasId3->measObjectId = 1;
  MeasId3->reportConfigId = 4;
  ASN_SEQUENCE_ADD(&MeasId_list->list,MeasId3);

  MeasId4            = CALLOC(1,sizeof(*MeasId4));
  MeasId4->measId = 5;
  MeasId4->measObjectId = 1;
  MeasId4->reportConfigId = 5;
  ASN_SEQUENCE_ADD(&MeasId_list->list,MeasId4);

  MeasId5            = CALLOC(1,sizeof(*MeasId5));
  MeasId5->measId = 6;
  MeasId5->measObjectId = 1;
  MeasId5->reportConfigId = 6;
  ASN_SEQUENCE_ADD(&MeasId_list->list,MeasId5);

  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig->measIdToAddModList = MeasId_list;  

  // Add one EUTRA Measurement Object
  MeasObj_list      = CALLOC(1,sizeof(*MeasObj_list));
  memset((void *)MeasObj_list,0,sizeof(*MeasObj_list));

  // Configure MeasObject
  
  MeasObj           = CALLOC(1,sizeof(*MeasObj));
  memset((void *)MeasObj,0,sizeof(*MeasObj));
  
  MeasObj->measObjectId           = 1;
  MeasObj->measObject.present                = MeasObjectToAddMod__measObject_PR_measObjectEUTRA;
  MeasObj->measObject.choice.measObjectEUTRA.carrierFreq                 = 36090;
  MeasObj->measObject.choice.measObjectEUTRA.allowedMeasBandwidth        = AllowedMeasBandwidth_mbw25;
  MeasObj->measObject.choice.measObjectEUTRA.presenceAntennaPort1        = 1;
  MeasObj->measObject.choice.measObjectEUTRA.neighCellConfig.buf         = CALLOC(1,sizeof(uint8_t));
  MeasObj->measObject.choice.measObjectEUTRA.neighCellConfig.buf[0]      = 0;
  MeasObj->measObject.choice.measObjectEUTRA.neighCellConfig.size        = 1;
  MeasObj->measObject.choice.measObjectEUTRA.neighCellConfig.bits_unused = 6;
  MeasObj->measObject.choice.measObjectEUTRA.offsetFreq                  = NULL; // Default is 15 or 0dB

  MeasObj->measObject.choice.measObjectEUTRA.cellsToAddModList = (CellsToAddModList_t *)CALLOC(1,sizeof(*CellsToAddModList));

  CellsToAddModList  = MeasObj->measObject.choice.measObjectEUTRA.cellsToAddModList;
  
  // Add adjacent cell lists (6 per eNB)
  for (i=0;i<6;i++) {
    CellToAdd                       = (CellsToAddMod_t *)CALLOC(1,sizeof(*CellToAdd));
    CellToAdd->cellIndex            = i+1;
    CellToAdd->physCellId           = get_adjacent_cell_id(Mod_id,i);
    CellToAdd->cellIndividualOffset = Q_OffsetRange_dB0;

    ASN_SEQUENCE_ADD(&CellsToAddModList->list,CellToAdd);
  } 
  
  ASN_SEQUENCE_ADD(&MeasObj_list->list,MeasObj);
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig->measObjectToAddModList = MeasObj_list;  

  // Report Configurations for periodical, A1-A5 events
  ReportConfig_list = CALLOC(1,sizeof(*ReportConfig_list));
  memset((void *)ReportConfig_list,0,sizeof(*ReportConfig_list));

  ReportConfig_per  = CALLOC(1,sizeof(*ReportConfig_per));
  memset((void *)ReportConfig_per,0,sizeof(*ReportConfig_per));

  ReportConfig_A1   = CALLOC(1,sizeof(*ReportConfig_A1));
  memset((void *)ReportConfig_A1,0,sizeof(*ReportConfig_A1));

  ReportConfig_A2   = CALLOC(1,sizeof(*ReportConfig_A2));
  memset((void *)ReportConfig_A2,0,sizeof(*ReportConfig_A2));

  ReportConfig_A3   = CALLOC(1,sizeof(*ReportConfig_A3));
  memset((void *)ReportConfig_A3,0,sizeof(*ReportConfig_A3));

  ReportConfig_A4   = CALLOC(1,sizeof(*ReportConfig_A4));
  memset((void *)ReportConfig_A4,0,sizeof(*ReportConfig_A4));

  ReportConfig_A5   = CALLOC(1,sizeof(*ReportConfig_A5));
  memset((void *)ReportConfig_A5,0,sizeof(*ReportConfig_A5));

  ReportConfig_per->reportConfigId                                                              = 1;
  ReportConfig_per->reportConfig.present                                                        = ReportConfigToAddMod__reportConfig_PR_reportConfigEUTRA;
  ReportConfig_per->reportConfig.choice.reportConfigEUTRA.triggerType.present                   = ReportConfigEUTRA__triggerType_PR_periodical;
  ReportConfig_per->reportConfig.choice.reportConfigEUTRA.triggerType.choice.periodical.purpose = ReportConfigEUTRA__triggerType__periodical__purpose_reportStrongestCells;
  ReportConfig_per->reportConfig.choice.reportConfigEUTRA.triggerQuantity                       = ReportConfigEUTRA__triggerQuantity_rsrp;
  ReportConfig_per->reportConfig.choice.reportConfigEUTRA.reportQuantity                        = ReportConfigEUTRA__reportQuantity_both;
  ReportConfig_per->reportConfig.choice.reportConfigEUTRA.maxReportCells                        = 2;
  ReportConfig_per->reportConfig.choice.reportConfigEUTRA.reportInterval                        = ReportInterval_ms120;
  ReportConfig_per->reportConfig.choice.reportConfigEUTRA.reportAmount                          = ReportConfigEUTRA__reportAmount_infinity;

  ASN_SEQUENCE_ADD(&ReportConfig_list->list,ReportConfig_per);

  ReportConfig_A1->reportConfigId                                                              = 2;
  ReportConfig_A1->reportConfig.present                                                        = ReportConfigToAddMod__reportConfig_PR_reportConfigEUTRA;
  ReportConfig_A1->reportConfig.choice.reportConfigEUTRA.triggerType.present                                    = ReportConfigEUTRA__triggerType_PR_event;
  ReportConfig_A1->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.present              = ReportConfigEUTRA__triggerType__event__eventId_PR_eventA1;
  ReportConfig_A1->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA1.a1_Threshold.present = ThresholdEUTRA_PR_threshold_RSRP;
  ReportConfig_A1->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA1.a1_Threshold.choice.threshold_RSRP = 10;

  ReportConfig_A1->reportConfig.choice.reportConfigEUTRA.triggerQuantity                       = ReportConfigEUTRA__triggerQuantity_rsrp;
  ReportConfig_A1->reportConfig.choice.reportConfigEUTRA.reportQuantity                        = ReportConfigEUTRA__reportQuantity_both;
  ReportConfig_A1->reportConfig.choice.reportConfigEUTRA.maxReportCells                        = 2;
  ReportConfig_A1->reportConfig.choice.reportConfigEUTRA.reportInterval                        = ReportInterval_ms120;
  ReportConfig_A1->reportConfig.choice.reportConfigEUTRA.reportAmount                          = ReportConfigEUTRA__reportAmount_infinity;

  ASN_SEQUENCE_ADD(&ReportConfig_list->list,ReportConfig_A1);

  ReportConfig_A2->reportConfigId                                                              = 3;
  ReportConfig_A2->reportConfig.present                                                        = ReportConfigToAddMod__reportConfig_PR_reportConfigEUTRA;
  ReportConfig_A2->reportConfig.choice.reportConfigEUTRA.triggerType.present                                    = ReportConfigEUTRA__triggerType_PR_event;
  ReportConfig_A2->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.present              = ReportConfigEUTRA__triggerType__event__eventId_PR_eventA2;
  ReportConfig_A2->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA2.a2_Threshold.present = ThresholdEUTRA_PR_threshold_RSRP;
  ReportConfig_A2->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA2.a2_Threshold.choice.threshold_RSRP = 10;

  ReportConfig_A2->reportConfig.choice.reportConfigEUTRA.triggerQuantity                       = ReportConfigEUTRA__triggerQuantity_rsrp;
  ReportConfig_A2->reportConfig.choice.reportConfigEUTRA.reportQuantity                        = ReportConfigEUTRA__reportQuantity_both;
  ReportConfig_A2->reportConfig.choice.reportConfigEUTRA.maxReportCells                        = 2;
  ReportConfig_A2->reportConfig.choice.reportConfigEUTRA.reportInterval                        = ReportInterval_ms120;
  ReportConfig_A2->reportConfig.choice.reportConfigEUTRA.reportAmount                          = ReportConfigEUTRA__reportAmount_infinity;

  ASN_SEQUENCE_ADD(&ReportConfig_list->list,ReportConfig_A2);

  ReportConfig_A3->reportConfigId                                                              = 4;
  ReportConfig_A3->reportConfig.present                                                        = ReportConfigToAddMod__reportConfig_PR_reportConfigEUTRA;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.triggerType.present                                    = ReportConfigEUTRA__triggerType_PR_event;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.present              = ReportConfigEUTRA__triggerType__event__eventId_PR_eventA3;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA3.a3_Offset = 10;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA3.reportOnLeave = 1;

  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.triggerQuantity                       = ReportConfigEUTRA__triggerQuantity_rsrp;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.reportQuantity                        = ReportConfigEUTRA__reportQuantity_both;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.maxReportCells                        = 2;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.reportInterval                        = ReportInterval_ms120;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.reportAmount                          = ReportConfigEUTRA__reportAmount_infinity;

  ASN_SEQUENCE_ADD(&ReportConfig_list->list,ReportConfig_A3);

  ReportConfig_A4->reportConfigId                                                              = 5;
  ReportConfig_A4->reportConfig.present                                                        = ReportConfigToAddMod__reportConfig_PR_reportConfigEUTRA;
  ReportConfig_A4->reportConfig.choice.reportConfigEUTRA.triggerType.present                                    = ReportConfigEUTRA__triggerType_PR_event;
  ReportConfig_A4->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.present              = ReportConfigEUTRA__triggerType__event__eventId_PR_eventA4;
  ReportConfig_A4->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA4.a4_Threshold.present = ThresholdEUTRA_PR_threshold_RSRP;
  ReportConfig_A4->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA4.a4_Threshold.choice.threshold_RSRP = 10;

  ReportConfig_A4->reportConfig.choice.reportConfigEUTRA.triggerQuantity                       = ReportConfigEUTRA__triggerQuantity_rsrp;
  ReportConfig_A4->reportConfig.choice.reportConfigEUTRA.reportQuantity                        = ReportConfigEUTRA__reportQuantity_both;
  ReportConfig_A4->reportConfig.choice.reportConfigEUTRA.maxReportCells                        = 2;
  ReportConfig_A4->reportConfig.choice.reportConfigEUTRA.reportInterval                        = ReportInterval_ms120;
  ReportConfig_A4->reportConfig.choice.reportConfigEUTRA.reportAmount                          = ReportConfigEUTRA__reportAmount_infinity;

  ASN_SEQUENCE_ADD(&ReportConfig_list->list,ReportConfig_A4);

  ReportConfig_A5->reportConfigId                                                              = 6;
  ReportConfig_A5->reportConfig.present                                                        = ReportConfigToAddMod__reportConfig_PR_reportConfigEUTRA;
  ReportConfig_A5->reportConfig.choice.reportConfigEUTRA.triggerType.present                                    = ReportConfigEUTRA__triggerType_PR_event;
  ReportConfig_A5->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.present              = ReportConfigEUTRA__triggerType__event__eventId_PR_eventA5;
  ReportConfig_A5->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA5.a5_Threshold1.present = ThresholdEUTRA_PR_threshold_RSRP;
  ReportConfig_A5->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA5.a5_Threshold2.present = ThresholdEUTRA_PR_threshold_RSRP;
  ReportConfig_A5->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA5.a5_Threshold1.choice.threshold_RSRP = 10;
  ReportConfig_A5->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA5.a5_Threshold2.choice.threshold_RSRP = 10;

  ReportConfig_A5->reportConfig.choice.reportConfigEUTRA.triggerQuantity                       = ReportConfigEUTRA__triggerQuantity_rsrp;
  ReportConfig_A5->reportConfig.choice.reportConfigEUTRA.reportQuantity                        = ReportConfigEUTRA__reportQuantity_both;
  ReportConfig_A5->reportConfig.choice.reportConfigEUTRA.maxReportCells                        = 2;
  ReportConfig_A5->reportConfig.choice.reportConfigEUTRA.reportInterval                        = ReportInterval_ms120;
  ReportConfig_A5->reportConfig.choice.reportConfigEUTRA.reportAmount                          = ReportConfigEUTRA__reportAmount_infinity;

  ASN_SEQUENCE_ADD(&ReportConfig_list->list,ReportConfig_A5);
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig->reportConfigToAddModList = ReportConfig_list;  
 
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.mobilityControlInfo  = NULL;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.dedicatedInfoNASList = NULL;
  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.securityConfigHO     = NULL;

  enc_rval = uper_encode_to_buffer(&asn_DEF_DL_DCCH_Message,
				   (void*)&dl_dcch_msg,
				   buffer,
				   100);
  
#ifdef XER_PRINT
  xer_fprint(stdout,&asn_DEF_DL_DCCH_Message,(void*)&dl_dcch_msg);
#endif
  LOG_I(RRC,"RRCConnectionReconfiguration Encoded %d bits (%d bytes)\n",enc_rval.encoded,(enc_rval.encoded+7)/8);

  //FREEMEM(SRB_list);
  //FREEMEM(DRB_list);

  return((enc_rval.encoded+7)/8);
}

//-----------------------------------------------------------------------------
uint8_t do_DRB_UE_MAC_config_cell (MAC_MainConfig_t *mac_MainConfig){
//-----------------------------------------------------------------------------
/*
Typedef struct MAC_MainConfig {
	struct MAC_MainConfig__ul_SCH_Config {
		long	*maxHARQ_Tx	// OPTIONAL ;
		long	*periodicBSR_Timer	// OPTIONAL ;
		long	 retxBSR_Timer;
		BOOLEAN_t	 ttiBundling;
		
		// Context for parsing across buffer boundaries
		asn_struct_ctx_t _asn_ctx;
	} *ul_SCH_Config;
	struct DRX_Config	*drx_Config	// OPTIONAL ;
	TimeAlignmentTimer_t	 timeAlignmentTimerDedicated;
	struct MAC_MainConfig__phr_Config {
		MAC_MainConfig__phr_Config_PR present;
		union MAC_MainConfig__phr_Config_u {
			NULL_t	 release;
			struct MAC_MainConfig__phr_Config__setup {
				long	 periodicPHR_Timer;
				long	 prohibitPHR_Timer;
				long	 dl_PathlossChange;
				
				// Context for parsing across buffer boundaries 
				asn_struct_ctx_t _asn_ctx;
			} setup;
		} choice;
		
		// Context for parsing across buffer boundaries 
		asn_struct_ctx_t _asn_ctx;
	} *phr_Config;
	
	//Context for parsing across buffer boundaries 
	asn_struct_ctx_t _asn_ctx;
} MAC_MainConfig_t;
*/
//*************************

  //mac_MainConfig->present =RadioResourceConfigDedicated__mac_MainConfig_PR_explicitValue;
  //mac_MainConfig->choice.explicitValue.ul_SCH_Config = CALLOC(1,sizeof(*mac_MainConfig->choice.explicitValue.ul_SCH_Config));
  //mac_MainConfig->ul_SCH_Config = CALLOC(1,sizeof(*mac_MainConfig->choice.explicitValue.ul_SCH_Config));

  long *maxHARQ_Tx;
  long *periodicBSR_Timer;

  mac_MainConfig->ul_SCH_Config = CALLOC(1,sizeof(*mac_MainConfig->ul_SCH_Config));

  maxHARQ_Tx = CALLOC(1,sizeof(long));
  *maxHARQ_Tx=MAC_MainConfig__ul_SCH_Config__maxHARQ_Tx_n5;
  mac_MainConfig->ul_SCH_Config->maxHARQ_Tx = maxHARQ_Tx;
  
  periodicBSR_Timer = CALLOC(1,sizeof(long));
  *periodicBSR_Timer = MAC_MainConfig__ul_SCH_Config__periodicBSR_Timer_sf64;
  mac_MainConfig->ul_SCH_Config->periodicBSR_Timer =  periodicBSR_Timer;

  mac_MainConfig->ul_SCH_Config->retxBSR_Timer =  MAC_MainConfig__ul_SCH_Config__retxBSR_Timer_sf320;

  mac_MainConfig->ul_SCH_Config->ttiBundling=0; // FALSE

  mac_MainConfig->drx_Config = NULL;

  mac_MainConfig->phr_Config = CALLOC(1,sizeof(*mac_MainConfig->phr_Config));
  
  mac_MainConfig->phr_Config->present = MAC_MainConfig__phr_Config_PR_setup;
  mac_MainConfig->phr_Config->choice.setup.periodicPHR_Timer= MAC_MainConfig__phr_Config__setup__periodicPHR_Timer_sf20; // sf20 = 20 subframes

  mac_MainConfig->phr_Config->choice.setup.prohibitPHR_Timer=MAC_MainConfig__phr_Config__setup__prohibitPHR_Timer_sf20; // sf20 = 20 subframes

  mac_MainConfig->phr_Config->choice.setup.dl_PathlossChange=MAC_MainConfig__phr_Config__setup__dl_PathlossChange_dB1; // Value dB1 =1 dB, dB3 = 3 dB
//*************************

return 0;
}

/************************************
 END CELLULAR ADAPTED FUNCTIONS
 ***********************************/

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
EXPORT_SYMBOL(transmission_mode_rrc);
#endif
