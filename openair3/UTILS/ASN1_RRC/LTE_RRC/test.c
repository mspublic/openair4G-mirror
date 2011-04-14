#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>	/* for atoi(3) */
#include <unistd.h>	/* for getopt(3) */
#include <string.h>	/* for strerror(3) */
#include <sysexits.h>	/* for EX_* exit codes */
#include <errno.h>	/* for errno */

#include <asn_application.h>
#include <asn_internal.h>	/* for _ASN_DEFAULT_STACK_MAX */
#include <per_encoder.h>

#include "RRCConnectionRequest.h"
#include "EstablishmentCause.h"
#include "RRCConnectionSetup.h"
#include "SRB-ToAddModList.h"

#include "RRCConnectionSetupComplete.h"

void assign_enum(ENUMERATED_t *x,uint8_t val) {
  uint8_t *buf=(uint8_t *)malloc(1);
  x->buf = buf;
  *buf=val;
  x->size=1;
}

main () {

  char *buffer;
  
  RRCConnectionRequest_t rrcconnectionrequest;
  RRCConnectionSetup_t rrcconnectionsetup;
  RRCConnectionSetupComplete_t rrcconnectionsetupcomplete;

  asn_enc_rval_t enc_rval;
  uint8_t buf[5],buf2=0;
  uint8_t ecause;
  long logicalchannelgroup=0;

  struct SRB_ToAddMod SRB1_config;
  struct SRB_ToAddMod__rlc_Config SRB1_rlc_config;
  struct SRB_ToAddMod__logicalChannelConfig SRB1_lchan_config;
  struct LogicalChannelConfig__ul_SpecificParameters SRB1_ul_SpecificParameters;


  SRB_ToAddModList_t SRB_list;

  rrcconnectionrequest.criticalExtensions.present = RRCConnectionRequest__criticalExtensions_PR_rrcConnectionRequest_r8;
  rrcconnectionrequest.criticalExtensions.choice.rrcConnectionRequest_r8.ue_Identity.present = InitialUE_Identity_PR_randomValue;
  rrcconnectionrequest.criticalExtensions.choice.rrcConnectionRequest_r8.ue_Identity.choice.randomValue.size = 5;
  rrcconnectionrequest.criticalExtensions.choice.rrcConnectionRequest_r8.ue_Identity.choice.randomValue.bits_unused = 0;
rrcconnectionrequest.criticalExtensions.choice.rrcConnectionRequest_r8.ue_Identity.choice.randomValue.buf = buf;
  rrcconnectionrequest.criticalExtensions.choice.rrcConnectionRequest_r8.ue_Identity.choice.randomValue.buf[0] = 0;
  rrcconnectionrequest.criticalExtensions.choice.rrcConnectionRequest_r8.ue_Identity.choice.randomValue.buf[1] = 1;
  rrcconnectionrequest.criticalExtensions.choice.rrcConnectionRequest_r8.ue_Identity.choice.randomValue.buf[2] = 2;
  rrcconnectionrequest.criticalExtensions.choice.rrcConnectionRequest_r8.ue_Identity.choice.randomValue.buf[3] = 3;
  rrcconnectionrequest.criticalExtensions.choice.rrcConnectionRequest_r8.ue_Identity.choice.randomValue.buf[4] = 4;

  ecause = EstablishmentCause_mo_Data;

  rrcconnectionrequest.criticalExtensions.choice.rrcConnectionRequest_r8.establishmentCause.buf = (uint8_t*)&ecause;
  rrcconnectionrequest.criticalExtensions.choice.rrcConnectionRequest_r8.establishmentCause.size = 1;

  rrcconnectionrequest.criticalExtensions.choice.rrcConnectionRequest_r8.spare.buf = &buf2;
  rrcconnectionrequest.criticalExtensions.choice.rrcConnectionRequest_r8.spare.size=1;
  rrcconnectionrequest.criticalExtensions.choice.rrcConnectionRequest_r8.spare.bits_unused = 7;

  buffer = malloc(100);
  
  enc_rval = uper_encode_to_buffer(&asn_DEF_RRCConnectionRequest,
				   (void*)&rrcconnectionrequest,
				   buffer,
				   100);


  printf("RRCConnectionRequest Encoded %d bits (%d bytes), ecause %d\n",enc_rval.encoded,(enc_rval.encoded+7)/8,ecause);

  rrcconnectionsetup.rrc_TransactionIdentifier = 0x1;
  rrcconnectionsetup.criticalExtensions.present = RRCConnectionSetup__criticalExtensions_PR_c1;
  rrcconnectionsetup.criticalExtensions.choice.c1.present = RRCConnectionSetup__criticalExtensions__c1_PR_rrcConnectionSetup_r8;
  rrcconnectionsetup.criticalExtensions.choice.c1.choice.rrcConnectionSetup_r8.radioResourceConfigDedicated.srb_ToAddModList = &SRB_list;
  rrcconnectionsetup.criticalExtensions.choice.c1.choice.rrcConnectionSetup_r8.radioResourceConfigDedicated.drb_ToAddModList = NULL;
  rrcconnectionsetup.criticalExtensions.choice.c1.choice.rrcConnectionSetup_r8.radioResourceConfigDedicated.drb_ToReleaseList = NULL;
  rrcconnectionsetup.criticalExtensions.choice.c1.choice.rrcConnectionSetup_r8.radioResourceConfigDedicated.sps_Config = NULL;
  rrcconnectionsetup.criticalExtensions.choice.c1.choice.rrcConnectionSetup_r8.radioResourceConfigDedicated.physicalConfigDedicated = NULL;
  rrcconnectionsetup.criticalExtensions.choice.c1.choice.rrcConnectionSetup_r8.radioResourceConfigDedicated.mac_MainConfig = NULL;

  asn_sequence_empty(&SRB_list);
  ASN_SEQUENCE_ADD(&SRB_list,&SRB1_config);

  SRB1_config.srb_Identity = 1;
  SRB1_config.rlc_Config   = &SRB1_rlc_config;

  SRB1_rlc_config.present = SRB_ToAddMod__rlc_Config_PR_explicitValue;
  SRB1_rlc_config.choice.explicitValue.present=RLC_Config_PR_am;
  assign_enum(&SRB1_rlc_config.choice.explicitValue.choice.am.ul_AM_RLC.t_PollRetransmit,T_PollRetransmit_ms45);
  assign_enum(&SRB1_rlc_config.choice.explicitValue.choice.am.ul_AM_RLC.pollPDU,PollPDU_pInfinity);
  assign_enum(&SRB1_rlc_config.choice.explicitValue.choice.am.ul_AM_RLC.pollByte,PollPDU_pInfinity);
  assign_enum(&SRB1_rlc_config.choice.explicitValue.choice.am.ul_AM_RLC.maxRetxThreshold,UL_AM_RLC__maxRetxThreshold_t4);
  assign_enum(&SRB1_rlc_config.choice.explicitValue.choice.am.dl_AM_RLC.t_Reordering,T_Reordering_ms35);
  assign_enum(&SRB1_rlc_config.choice.explicitValue.choice.am.dl_AM_RLC.t_StatusProhibit,T_StatusProhibit_ms0);


  SRB1_config.logicalChannelConfig   = &SRB1_lchan_config;

  SRB1_lchan_config.present = SRB_ToAddMod__logicalChannelConfig_PR_explicitValue;
  SRB1_lchan_config.choice.explicitValue.ul_SpecificParameters = &SRB1_ul_SpecificParameters;
  SRB1_ul_SpecificParameters.priority = 1;

  assign_enum(&SRB1_ul_SpecificParameters.prioritisedBitRate,LogicalChannelConfig__ul_SpecificParameters__prioritisedBitRate_infinity);
  assign_enum(&SRB1_ul_SpecificParameters.bucketSizeDuration,LogicalChannelConfig__ul_SpecificParameters__bucketSizeDuration_ms50);
  SRB1_ul_SpecificParameters.logicalChannelGroup = &logicalchannelgroup;




  printf("/n/n/nRunning per encoder for RRCConnectionSetup\n");
  enc_rval = uper_encode_to_buffer(&asn_DEF_RRCConnectionSetup,
				   (void*)&rrcconnectionsetup,
				   buffer,
				   100);


  printf("RRCConnectionSetup Encoded %d bits (%d bytes), ecause %d\n",enc_rval.encoded,(enc_rval.encoded+7)/8,ecause);


  rrcconnectionsetupcomplete.rrc_TransactionIdentifier = 0x1;
  rrcconnectionsetupcomplete.criticalExtensions.present = RRCConnectionSetupComplete__criticalExtensions_PR_c1;
  rrcconnectionsetupcomplete.criticalExtensions.choice.c1.present = RRCConnectionSetupComplete__criticalExtensions__c1_PR_rrcConnectionSetupComplete_r8;

}
