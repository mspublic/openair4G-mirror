/*________________________rrc_UE.c________________________

  Authors : Raymond Knopp
  Company : EURECOM
  Emails  : knopp@eurecom.fr
  ________________________________________________________________*/

#include "defs.h"
#include "extern.h"
#include "RRC/L2_INTERFACE/openair_rrc_L2_interface.h"
#include "LAYER2/RLC/rlc.h"
#include "COMMON/mac_rrc_primitives.h"
#include "RRC/LITE/MESSAGES/asn1_msg.h"
#include "RRCConnectionRequest.h"
#include "UL-CCCH-Message.h"
#include "DL-CCCH-Message.h"
#include "UL-DCCH-Message.h"
#include "DL-DCCH-Message.h"
#include "MeasGapConfig.h"
#include "TDD-Config.h"
#include "UTIL/OCG/OCG.h"
#include "UTIL/OCG/OCG_extern.h"
#ifdef PHY_EMUL
#include "RRC/NAS/nas_config.h"
extern EMULATION_VARS *Emul_vars;
#endif
extern eNB_MAC_INST *eNB_mac_inst;
extern UE_MAC_INST *UE_mac_inst;
#ifdef BIGPHYSAREA
extern void *bigphys_malloc(int);
#endif

extern inline unsigned int taus(void);

void init_SI_UE(u8 Mod_id,u8 eNB_index) {

  int i;


  UE_rrc_inst[Mod_id].sizeof_SIB1[eNB_index] = 0;
  UE_rrc_inst[Mod_id].sizeof_SI[eNB_index] = 0;

  UE_rrc_inst[Mod_id].SIB1[eNB_index] = (u8 *)malloc16(32);
  UE_rrc_inst[Mod_id].sib1[eNB_index] = (SystemInformationBlockType1_t *)malloc16(sizeof(SystemInformationBlockType1_t));
  UE_rrc_inst[Mod_id].SI[eNB_index] = (u8 *)malloc16(64);

  for (i=0;i<8;i++) {
     UE_rrc_inst[Mod_id].si[eNB_index][i] = (SystemInformation_t *)malloc16(sizeof(SystemInformation_t));
  }

  UE_rrc_inst[Mod_id].Info[eNB_index].SIB1Status = 0;
  UE_rrc_inst[Mod_id].Info[eNB_index].SIStatus = 0;


}

/*------------------------------------------------------------------------------*/
char openair_rrc_ue_init(u8 Mod_id, unsigned char eNB_index){
  /*-----------------------------------------------------------------------------*/


  msg("[OPENAIR][RRC] INIT UE %d (eNB %d)\n",Mod_id,eNB_index);

  UE_rrc_inst[Mod_id].Info[eNB_index].Status=RRC_IDLE;
  UE_rrc_inst[Mod_id].Info[eNB_index].Rach_tx_cnt=0;
  UE_rrc_inst[Mod_id].Info[eNB_index].Nb_bcch_wait=0;
  UE_rrc_inst[Mod_id].Info[eNB_index].UE_index=0xffff;
  UE_rrc_inst[Mod_id].Srb0[eNB_index].Active=0;
  UE_rrc_inst[Mod_id].Srb1[eNB_index].Active=0;
  UE_rrc_inst[Mod_id].Srb2[eNB_index].Active=0;

  init_SI_UE(Mod_id,eNB_index);
  msg("[UE][RRC] INIT: phy_sync_2_ch_ind from Inst %d\n", Mod_id);

#ifndef NO_RRM
  send_msg(&S_rrc,msg_rrc_phy_synch_to_CH_ind(Mod_id,eNB_index,UE_rrc_inst[Mod_id].Mac_id));
#endif

#ifdef NO_RRM //init ch SRB0, SRB1 & BDTCH
  openair_rrc_on(Mod_id);
#endif
  msg("[OPENAIR][RRC] Init OK for UE %d\n",Mod_id);

  return 0;
}


/*------------------------------------------------------------------------------*/
void rrc_ue_generate_RRCConnectionRequest(u8 Mod_id, u8 eNB_index){
  /*------------------------------------------------------------------------------*/

  u8 i=0,rv[6];

  if(UE_rrc_inst[Mod_id].Srb0[eNB_index].Tx_buffer.payload_size ==0){

    // Get RRCConnectionRequest, fill random for now


    // Generate random byte stream for contention resolution
    for (i=0;i<6;i++)
      rv[i]=taus()&0xff;

    UE_rrc_inst[Mod_id].Srb0[eNB_index].Tx_buffer.payload_size = do_RRCConnectionRequest((u8 *)UE_rrc_inst[Mod_id].Srb0[eNB_index].Tx_buffer.Payload,rv);

    /*
      UE_rrc_inst[Mod_id].Srb0[Idx].Tx_buffer.Payload[i] = taus()&0xff;

    UE_rrc_inst[Mod_id].Srb0[Idx].Tx_buffer.payload_size =i;
    */

  }
}


mui_t rrc_mui=0;


/*------------------------------------------------------------------------------*/
void rrc_ue_generate_RRCConnectionSetupComplete(u8 Mod_id, u8 eNB_index){
  /*------------------------------------------------------------------------------*/

  u8 buffer[32];
  u8 size;

  msg("[RRC][UE %d] Frame %d : Generating RRCConnectionSetupComplete\n",Mod_id,Mac_rlc_xface->frame);

  size = do_RRCConnectionSetupComplete(buffer);

  Mac_rlc_xface->rrc_rlc_data_req(Mod_id+NB_eNB_INST,DCCH,rrc_mui++,0,size,(char*)buffer);

}



void rrc_ue_generate_RRCConnectionReconfigurationComplete(u8 Mod_id,u8 eNB_index) {

  u8 buffer[32], size;

  msg("[RRC][UE %d] Frame %d : Generating RRCConnectionReconfigurationComplete\n",Mod_id,Mac_rlc_xface->frame);

  size = do_RRCConnectionReconfigurationComplete(buffer);

  Mac_rlc_xface->rrc_rlc_data_req(Mod_id+NB_eNB_INST,DCCH,rrc_mui++,0,size,(char*)buffer);
}


/*------------------------------------------------------------------------------*/
int rrc_ue_decode_ccch(u8 Mod_id, SRB_INFO *Srb_info, u8 eNB_index){
  /*------------------------------------------------------------------------------*/

  DL_CCCH_Message_t dlccchmsg;
  DL_CCCH_Message_t *dl_ccch_msg=&dlccchmsg;
  asn_dec_rval_t dec_rval;
  int i;

  memset(dl_ccch_msg,0,sizeof(DL_CCCH_Message_t));
  msg("[RRC][UE %d] Decoding DL-CCCH message (%d bytes)\n",Mod_id,Srb_info->Rx_buffer.payload_size);
  for (i=0;i<Srb_info->Rx_buffer.payload_size;i++)
    msg("%2x.",Srb_info->Rx_buffer.Payload[i]);
  msg("\n");

  dec_rval = uper_decode(NULL,
			 &asn_DEF_DL_CCCH_Message,
			 (void**)&dl_ccch_msg,
	 		 (uint8_t*)Srb_info->Rx_buffer.Payload,
			 100,0,0);

  if ((dec_rval.code != RC_OK) && (dec_rval.consumed==0)) {
    msg("[RRC][UE %d] Frame %d : Failed to decode SIB 1 (%d bytes)\n",Mod_id,Mac_rlc_xface->frame,dec_rval.consumed);
    return -1;
  }
  

  if (dl_ccch_msg->message.present == DL_CCCH_MessageType_PR_c1) {

    if (UE_rrc_inst[Mod_id].Info[eNB_index].Status == RRC_PRE_SYNCHRO) {

      switch (dl_ccch_msg->message.choice.c1.present) {

      case DL_CCCH_MessageType__c1_PR_NOTHING :
	msg("[RRC][UE%d] Frame %d : Received PR_NOTHING on DL-CCCH-Message\n",Mod_id,Mac_rlc_xface->frame);
	return 0;
	break;
      case DL_CCCH_MessageType__c1_PR_rrcConnectionReestablishment:
	msg("[RRC][UE%d] Frame %d : Received RRCConnectionReestablishment on DL-CCCH-Message\n",Mod_id,Mac_rlc_xface->frame);
	return 0;
	break;
      case DL_CCCH_MessageType__c1_PR_rrcConnectionReestablishmentReject:
	msg("[RRC][UE%d] Frame %d : Received RRCConnectionReestablishmentReject on DL-CCCH-Message\n",Mod_id,Mac_rlc_xface->frame);
	return 0;
	break;
      case DL_CCCH_MessageType__c1_PR_rrcConnectionReject:
	msg("[RRC][UE%d] Frame %d : Received RRCConnectionReject on DL-CCCH-Message\n",Mod_id,Mac_rlc_xface->frame);
	return 0;
	break;
      case DL_CCCH_MessageType__c1_PR_rrcConnectionSetup:
	msg("[RRC][UE%d] Frame %d : Received RRCConnectionSetup on DL-CCCH-Message\n",Mod_id,Mac_rlc_xface->frame);
	// Get configuration


	rrc_ue_process_radioResourceConfigDedicated(Mod_id,eNB_index,
						    &dl_ccch_msg->message.choice.c1.choice.rrcConnectionSetup.criticalExtensions.choice.c1.choice.rrcConnectionSetup_r8.radioResourceConfigDedicated);

	rrc_ue_generate_RRCConnectionSetupComplete(Mod_id,eNB_index);

	return 0;
	break;
      default:
	msg("[RRC][UE%d] Frame %d : Unknown message\n",Mod_id,Mac_rlc_xface->frame);
	return -1;
      }
    }
  }

  return 0;
}


s32 rrc_ue_establish_srb1(u8 Mod_id,u8 eNB_index,
			 struct SRB_ToAddMod *SRB_config) { // add descriptor from RRC PDU

  u8 lchan_id = DCCH;

  UE_rrc_inst[Mod_id].Srb1[eNB_index].Active = 1;
  UE_rrc_inst[Mod_id].Srb1[eNB_index].Status = RADIO_CONFIG_OK;//RADIO CFG
  UE_rrc_inst[Mod_id].Srb1[eNB_index].Srb_info.Srb_id = 1;

    // copy default configuration for now
  memcpy(&UE_rrc_inst[Mod_id].Srb1[eNB_index].Srb_info.Lchan_desc[0],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
  memcpy(&UE_rrc_inst[Mod_id].Srb1[eNB_index].Srb_info.Lchan_desc[1],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);


  msg("[RRC][UE %d], CONFIG_SRB1 %d corresponding to eNB_index %d\n",
      Mod_id,
      lchan_id,
      eNB_index);

  Mac_rlc_xface->rrc_rlc_config_req(Mod_id+NB_eNB_INST,ACTION_ADD,lchan_id,SIGNALLING_RADIO_BEARER,Rlc_info_am_config);
  //  UE_rrc_inst[Mod_id].Srb1[eNB_index].Srb_info.Tx_buffer.payload_size=DEFAULT_MEAS_IND_SIZE+1;


  return(0);
}

s32 rrc_ue_establish_srb2(u8 Mod_id,u8 eNB_index,
			 struct SRB_ToAddMod *SRB_config) { // add descriptor from RRC PDU

  u8 lchan_id = DCCH1;

  UE_rrc_inst[Mod_id].Srb2[eNB_index].Active = 1;
  UE_rrc_inst[Mod_id].Srb2[eNB_index].Status = RADIO_CONFIG_OK;//RADIO CFG
  UE_rrc_inst[Mod_id].Srb2[eNB_index].Srb_info.Srb_id = 2;

    // copy default configuration for now
  memcpy(&UE_rrc_inst[Mod_id].Srb2[eNB_index].Srb_info.Lchan_desc[0],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
  memcpy(&UE_rrc_inst[Mod_id].Srb2[eNB_index].Srb_info.Lchan_desc[1],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);

  
  msg("[RRC][UE %d], CONFIG_SRB2 %d corresponding to eNB_index %d\n",
      Mod_id,
      lchan_id,
      eNB_index);

  Mac_rlc_xface->rrc_rlc_config_req(Mod_id+NB_eNB_INST,ACTION_ADD,lchan_id,SIGNALLING_RADIO_BEARER,Rlc_info_am_config);
  
  //  UE_rrc_inst[Mod_id].Srb1[eNB_index].Srb_info.Tx_buffer.payload_size=DEFAULT_MEAS_IND_SIZE+1;


  return(0);
}

s32 rrc_ue_establish_drb(u8 Mod_id,u8 eNB_index,
			 struct DRB_ToAddMod *DRB_config) { // add descriptor from RRC PDU

    msg("[RRC][UE] Frame %d: Configuring DRB %ld/LCID %d\n",
      Mac_rlc_xface->frame,DRB_config->drb_Identity,(int)*DRB_config->logicalChannelIdentity);

  switch (DRB_config->rlc_Config->present) {
  case RLC_Config_PR_NOTHING:
    msg("[RRC][UE] Frame %d: Received RLC_Config_PR_NOTHING!! for DRB Configuration\n",Mac_rlc_xface->frame);
    return(-1);
    break;
  case RLC_Config_PR_um_Bi_Directional :
    Mac_rlc_xface->rrc_rlc_config_req(Mod_id+NB_eNB_INST,ACTION_ADD,
				      (eNB_index * MAX_NUM_RB) + *DRB_config->logicalChannelIdentity,
				      RADIO_ACCESS_BEARER,Rlc_info_um);
#ifdef NAS_NETLINK
    nas_config(oai_emulation.info.nb_enb_local+Mod_id,// interface index
	       oai_emulation.info.nb_enb_local+Mod_id+1, 
	       NB_eNB_INST+Mod_id+1);
    printf ( "rb_conf_ipv4: Mod_id %d emu_info.nb_enb_local+Mod_id %d\n",  Mod_id,  oai_emulation.info.nb_enb_local+Mod_id);
    rb_conf_ipv4(0,//add
		 Mod_id,//cx align with the UE index 
		 oai_emulation.info.nb_enb_local+Mod_id,//inst num_ue+ue_index
 		 (eNB_index * MAX_NUM_RB) + *DRB_config->logicalChannelIdentity,//rb
		 0,//dscp
		 ipv4_address(oai_emulation.info.nb_enb_local+Mod_id+1,NB_eNB_INST+Mod_id+1),//saddr
		 ipv4_address(oai_emulation.info.nb_enb_local+Mod_id+1,eNB_index+1));//daddr
		 
#endif 
    break;
  case RLC_Config_PR_um_Uni_Directional_UL :
  case RLC_Config_PR_um_Uni_Directional_DL :
  case RLC_Config_PR_am:
    msg("[RRC][UE] Frame %d: Illegal RLC mode for DRB\n",Mac_rlc_xface->frame);
    return(-1);
    break;
  }

  return(0);
}


void	rrc_ue_process_measConfig(u8 Mod_id,u8 eNB_index,MeasConfig_t *measConfig){

  if (measConfig->measGapConfig !=NULL) {
    if (UE_rrc_inst[Mod_id].measGapConfig[eNB_index]) {
      memcpy((char*)UE_rrc_inst[Mod_id].measGapConfig[eNB_index],(char*)measConfig->measGapConfig,
	     sizeof(MeasGapConfig_t));
    }
    else {
      UE_rrc_inst[Mod_id].measGapConfig[eNB_index] = measConfig->measGapConfig;
    }
  }
}


void	rrc_ue_process_radioResourceConfigDedicated(u8 Mod_id,u8 eNB_index,
						    RadioResourceConfigDedicated_t *radioResourceConfigDedicated) {

  long SRB_id,DRB_id;
  int i,ret,cnt;
  LogicalChannelConfig_t *SRB1_logicalChannelConfig,*SRB2_logicalChannelConfig;

  // Save physicalConfigDedicated if present
  if (radioResourceConfigDedicated->physicalConfigDedicated) {
    if (UE_rrc_inst[Mod_id].physicalConfigDedicated[eNB_index]) {
      memcpy((char*)UE_rrc_inst[Mod_id].physicalConfigDedicated[eNB_index],(char*)radioResourceConfigDedicated->physicalConfigDedicated,
	     sizeof(struct PhysicalConfigDedicated));

    }
    else {
      UE_rrc_inst[Mod_id].physicalConfigDedicated[eNB_index] = radioResourceConfigDedicated->physicalConfigDedicated;
    }
  }
  // Apply macMainConfig if present
  if (radioResourceConfigDedicated->mac_MainConfig) {
    if (radioResourceConfigDedicated->mac_MainConfig->present == RadioResourceConfigDedicated__mac_MainConfig_PR_explicitValue) {
      if (UE_rrc_inst[Mod_id].physicalConfigDedicated[eNB_index]) {
	memcpy((char*)UE_rrc_inst[Mod_id].mac_MainConfig[eNB_index],(char*)&radioResourceConfigDedicated->mac_MainConfig->choice.explicitValue,
	       sizeof(MAC_MainConfig_t));
      }
      else
	UE_rrc_inst[Mod_id].mac_MainConfig[eNB_index] = &radioResourceConfigDedicated->mac_MainConfig->choice.explicitValue;
    }
  }

  // Apply spsConfig if present
  if (radioResourceConfigDedicated->sps_Config) {
    if (UE_rrc_inst[Mod_id].sps_Config[eNB_index]) {
      memcpy(UE_rrc_inst[Mod_id].sps_Config[eNB_index],radioResourceConfigDedicated->sps_Config,
	     sizeof(struct SPS_Config));
    }
    else {
      UE_rrc_inst[Mod_id].sps_Config[eNB_index] = radioResourceConfigDedicated->sps_Config;
    }
  }
  // Establish SRBs if present
  // loop through SRBToAddModList
  if (radioResourceConfigDedicated->srb_ToAddModList) {
  
    for (cnt=0;cnt<radioResourceConfigDedicated->srb_ToAddModList->list.count;cnt++) {

      SRB_id = radioResourceConfigDedicated->srb_ToAddModList->list.array[cnt]->srb_Identity;
      msg("[RRC][UE%d]: SRB config cnt %d (SRB%ld)\n",Mod_id,cnt,SRB_id);
      if (SRB_id == 1) {
	if (UE_rrc_inst[Mod_id].SRB1_config[eNB_index]) {
	  memcpy(UE_rrc_inst[Mod_id].SRB1_config[eNB_index],radioResourceConfigDedicated->srb_ToAddModList->list.array[cnt],
		 sizeof(struct SRB_ToAddMod));
	}
	else {
	  UE_rrc_inst[Mod_id].SRB1_config[eNB_index] = radioResourceConfigDedicated->srb_ToAddModList->list.array[cnt];

	  ret = rrc_ue_establish_srb1(Mod_id,eNB_index,radioResourceConfigDedicated->srb_ToAddModList->list.array[cnt]);
	  if (UE_rrc_inst[Mod_id].SRB1_config[eNB_index]->logicalChannelConfig) {
	    if (UE_rrc_inst[Mod_id].SRB1_config[eNB_index]->logicalChannelConfig->present == SRB_ToAddMod__logicalChannelConfig_PR_explicitValue) {
	      SRB1_logicalChannelConfig = &UE_rrc_inst[Mod_id].SRB1_config[eNB_index]->logicalChannelConfig->choice.explicitValue;
	    }
	    else {
	      SRB1_logicalChannelConfig = &SRB1_logicalChannelConfig_defaultValue;
	    }
	  }
	  else {
	    SRB1_logicalChannelConfig = &SRB1_logicalChannelConfig_defaultValue;
	  }
	  
	  Mac_rlc_xface->rrc_mac_config_req(Mod_id,0,0,eNB_index,
					    (RadioResourceConfigCommonSIB_t *)NULL,
					    UE_rrc_inst[Mod_id].physicalConfigDedicated[eNB_index],
					    UE_rrc_inst[Mod_id].mac_MainConfig[eNB_index],
					    1,
					    SRB1_logicalChannelConfig,
					    (MeasGapConfig_t *)NULL,
					    NULL,
					    NULL,
					    NULL);
	}
      }
      else {
	if (UE_rrc_inst[Mod_id].SRB2_config[eNB_index]) {
	  memcpy(UE_rrc_inst[Mod_id].SRB2_config[eNB_index],radioResourceConfigDedicated->srb_ToAddModList->list.array[cnt],
		 sizeof(struct SRB_ToAddMod));
	}
	else {
	  
	  UE_rrc_inst[Mod_id].SRB2_config[eNB_index] = radioResourceConfigDedicated->srb_ToAddModList->list.array[cnt];

	  ret = rrc_ue_establish_srb2(Mod_id,eNB_index,radioResourceConfigDedicated->srb_ToAddModList->list.array[cnt]);
	  if (UE_rrc_inst[Mod_id].SRB2_config[eNB_index]->logicalChannelConfig) {
	    if (UE_rrc_inst[Mod_id].SRB2_config[eNB_index]->logicalChannelConfig->present == SRB_ToAddMod__logicalChannelConfig_PR_explicitValue){
	      SRB2_logicalChannelConfig = &UE_rrc_inst[Mod_id].SRB2_config[eNB_index]->logicalChannelConfig->choice.explicitValue;
	    }
	    else {
	      SRB2_logicalChannelConfig = &SRB2_logicalChannelConfig_defaultValue;
	    }
	  }
	  else {
	    SRB2_logicalChannelConfig = &SRB2_logicalChannelConfig_defaultValue;
	  }
	  
	  Mac_rlc_xface->rrc_mac_config_req(Mod_id,0,0,eNB_index,
					    (RadioResourceConfigCommonSIB_t *)NULL,
					    UE_rrc_inst[Mod_id].physicalConfigDedicated[eNB_index],
					    UE_rrc_inst[Mod_id].mac_MainConfig[eNB_index],
					    2,
					    SRB2_logicalChannelConfig,
					    UE_rrc_inst[Mod_id].measGapConfig[eNB_index],
					    (TDD_Config_t *)NULL,
					    (u8 *)NULL,
					    (u16 *)NULL);
	}
      }
    }
  }

  // Establish DRBs if present
  if (radioResourceConfigDedicated->drb_ToAddModList) {

    for (i=0;i<radioResourceConfigDedicated->drb_ToAddModList->list.count;i++) {
      DRB_id   = radioResourceConfigDedicated->drb_ToAddModList->list.array[i]->drb_Identity-1;
      if (UE_rrc_inst[Mod_id].DRB_config[eNB_index][DRB_id]) {
	memcpy(UE_rrc_inst[Mod_id].DRB_config[eNB_index][DRB_id],radioResourceConfigDedicated->drb_ToAddModList->list.array[i],
	       sizeof(struct DRB_ToAddMod));
      }
      else {
	UE_rrc_inst[Mod_id].DRB_config[eNB_index][DRB_id] = radioResourceConfigDedicated->drb_ToAddModList->list.array[i];

	ret = rrc_ue_establish_drb(Mod_id,eNB_index,radioResourceConfigDedicated->drb_ToAddModList->list.array[i]);
	// MAC/PHY Configuration
	Mac_rlc_xface->rrc_mac_config_req(Mod_id,0,0,eNB_index,
					  (RadioResourceConfigCommonSIB_t *)NULL,
					  UE_rrc_inst[Mod_id].physicalConfigDedicated[eNB_index],
					  UE_rrc_inst[Mod_id].mac_MainConfig[eNB_index],
					  *UE_rrc_inst[Mod_id].DRB_config[eNB_index][DRB_id]->logicalChannelIdentity,
					  UE_rrc_inst[Mod_id].DRB_config[eNB_index][DRB_id]->logicalChannelConfig,
					  UE_rrc_inst[Mod_id].measGapConfig[eNB_index],
					  (TDD_Config_t*)NULL,
					  (u8 *)NULL,
					  (u16 *)NULL);

      }
    }
  }

  UE_rrc_inst[Mod_id].Info[eNB_index].Status = RRC_CONNECTED;


}
  

void rrc_ue_process_rrcConnectionReconfiguration(u8 Mod_id,
						 RRCConnectionReconfiguration_t *rrcConnectionReconfiguration,
						 u8 eNB_index) {

  if (rrcConnectionReconfiguration->criticalExtensions.present == RRCConnectionReconfiguration__criticalExtensions__c1_PR_rrcConnectionReconfiguration_r8) {

    if (rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig != NULL) {
      rrc_ue_process_measConfig(Mod_id,eNB_index,
				rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig);
    }
    if (rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated) {
      rrc_ue_process_radioResourceConfigDedicated(Mod_id,eNB_index,
						  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated);
      
    }

    // check other fields for
  }
}
  
/*------------------------------------------------------------------------------------------*/
void  rrc_ue_decode_dcch(u8 Mod_id,u8 Srb_id, u8 *Buffer,u8 eNB_index){
  /*------------------------------------------------------------------------------------------*/

  DL_DCCH_Message_t dldcchmsg;
  DL_DCCH_Message_t *dl_dcch_msg=&dldcchmsg;
  asn_dec_rval_t dec_rval;
  int i;

  if (Srb_id != 1) {
    msg("[RRC][eNB %d] Frame %d: Received message on SRB2, should not have ...\n",Mod_id,Mac_rlc_xface->frame);
    return;
  }

  memset(dl_dcch_msg,0,sizeof(DL_DCCH_Message_t));

  // decode messages
  msg("[RRC][UE %d] Decoding DL-DCCH message\n",Mod_id);
  for (i=0;i<30;i++)
    msg("%x.",Buffer[i]);
  msg("\n");

  dec_rval = uper_decode(NULL,
			 &asn_DEF_DL_DCCH_Message,
			 (void**)&dl_dcch_msg,
			 (uint8_t*)Buffer,
			 100,0,0);

  if (dl_dcch_msg->message.present == DL_DCCH_MessageType_PR_c1) {

    if (UE_rrc_inst[Mod_id].Info[eNB_index].Status == RRC_CONNECTED) {

      switch (dl_dcch_msg->message.choice.c1.present) {

      case DL_DCCH_MessageType__c1_PR_NOTHING :
	msg("[RRC][eNB %d] Frame %d : Received PR_NOTHING on DL-DCCH-Message\n",Mod_id,Mac_rlc_xface->frame);
	return;
	break;
      case DL_DCCH_MessageType__c1_PR_csfbParametersResponseCDMA2000:
	break;
      case DL_DCCH_MessageType__c1_PR_dlInformationTransfer:
	break;
      case DL_DCCH_MessageType__c1_PR_handoverFromEUTRAPreparationRequest:
	break;
      case DL_DCCH_MessageType__c1_PR_mobilityFromEUTRACommand:
	break;
      case DL_DCCH_MessageType__c1_PR_rrcConnectionReconfiguration:

	msg("[RRC][UE] Frame %d: Processing RRCConnectionReconfiguration from eNB %d\n",
	    Mac_rlc_xface->frame,eNB_index);
	rrc_ue_process_rrcConnectionReconfiguration(Mod_id,&dl_dcch_msg->message.choice.c1.choice.rrcConnectionReconfiguration,eNB_index);
	rrc_ue_generate_RRCConnectionReconfigurationComplete(Mod_id,eNB_index);
	break;
      case DL_DCCH_MessageType__c1_PR_rrcConnectionRelease:
	break;
      case DL_DCCH_MessageType__c1_PR_securityModeCommand:
	break;
      case DL_DCCH_MessageType__c1_PR_ueCapabilityEnquiry:
	break;
      case DL_DCCH_MessageType__c1_PR_counterCheck:
	break;
      case DL_DCCH_MessageType__c1_PR_ueInformationRequest_r9:
	break;
      case DL_DCCH_MessageType__c1_PR_loggedMeasurementConfiguration_r10:
	break;
      case DL_DCCH_MessageType__c1_PR_rnReconfiguration_r10:
	break;
      case DL_DCCH_MessageType__c1_PR_spare1:
      case DL_DCCH_MessageType__c1_PR_spare2:
      case DL_DCCH_MessageType__c1_PR_spare3:
      case DL_DCCH_MessageType__c1_PR_spare4:
	break;
      }
    }
  }
#ifndef NO_RRM
    send_msg(&S_rrc,msg_rrc_end_scan_req(Mod_id,eNB_index));
#endif
}
  
const char siWindowLength[7][5] = {"1ms\0","2ms\0","5ms\0","10ms\0","15ms\0","20ms\0","40ms\0"};
const char siWindowLength_int[7] = {1,2,5,10,15,20,40};

const char SIBType[16][6] ={"SIB3\0","SIB4\0","SIB5\0","SIB6\0","SIB7\0","SIB8\0","SIB9\0","SIB10\0","SIB11\0","Sp0\0","Sp1\0","Sp2\0","Sp3\0","Sp4\0"};
const char SIBPeriod[7][7]= {"80ms\0","160ms\0","320ms\0","640ms\0","1280ms\0","2560ms\0","5120ms\0"};

int decode_SIB1(u8 Mod_id,u8 eNB_index) {
  asn_dec_rval_t dec_rval;
  SystemInformationBlockType1_t **sib1=&UE_rrc_inst[Mod_id].sib1[eNB_index];
  int i;

  memset(*sib1,0,sizeof(SystemInformationBlockType1_t));
  dec_rval = uper_decode(NULL,
			 &asn_DEF_SystemInformationBlockType1,
			 (void**)sib1,
			 (uint8_t*)UE_rrc_inst[Mod_id].SIB1[eNB_index],
			 100,0,0);

  if ((dec_rval.code != RC_OK) && (dec_rval.consumed==0)) {
    msg("[RRC][UE %d] Frame %d : Failed to decode SIB 1 (%d bytes)\n",Mod_id,Mac_rlc_xface->frame,dec_rval.consumed);
    return -1;
  }

  msg("[RRC][UE %d] Frame %d : Dumping SIB 1 (%d bytes)\n",Mod_id,Mac_rlc_xface->frame,dec_rval.consumed);
  for (i=0;i<18;i++)
    msg("%x.",UE_rrc_inst[Mod_id].SIB1[eNB_index][i]);
  msg("\n");

  msg("cellAccessRelatedInfo.cellIdentity : %x.%x.%x.%x\n",
      (*sib1)->cellAccessRelatedInfo.cellIdentity.buf[0],
      (*sib1)->cellAccessRelatedInfo.cellIdentity.buf[1],
      (*sib1)->cellAccessRelatedInfo.cellIdentity.buf[2],
      (*sib1)->cellAccessRelatedInfo.cellIdentity.buf[3]);

  msg("cellSelectionInfo.q_RxLevMin       : %d\n",(int)(*sib1)->cellSelectionInfo.q_RxLevMin);
  msg("freqBandIndicator                  : %d\n",(int)(*sib1)->freqBandIndicator);
  msg("siWindowLength                     : %s\n",siWindowLength[(*sib1)->si_WindowLength]);
  if ((*sib1)->schedulingInfoList.list.count) {
    msg("siSchedulingInfoPeriod[0]          : %s\n",SIBPeriod[(int)(*sib1)->schedulingInfoList.list.array[0]->si_Periodicity]);
    if ((*sib1)->schedulingInfoList.list.array[0]->sib_MappingInfo.list.count)
      msg("siSchedulingInfoSIBType[0]         : %s\n",SIBType[(int)(*(*sib1)->schedulingInfoList.list.array[0]->sib_MappingInfo.list.array[0])]);
    else {
      msg("siSchedulingInfoSIBType[0]         : PROBLEM!!!\n");
      return -1;
    }
  }
  else {
    msg("siSchedulingInfoPeriod[0]          : PROBLEM!!!\n");
   return -1;
  }

  if ((*sib1)->tdd_Config)
    msg("TDD subframe assignment            : %d\nS-Subframe Config                  : %d\n",(int)(*sib1)->tdd_Config->subframeAssignment,(int)(*sib1)->tdd_Config->specialSubframePatterns);

  UE_rrc_inst[Mod_id].Info[eNB_index].SIperiod    =8<<((int)(*sib1)->schedulingInfoList.list.array[0]->si_Periodicity);
  UE_rrc_inst[Mod_id].Info[eNB_index].SIwindowsize=siWindowLength_int[(int)*(*sib1)->schedulingInfoList.list.array[0]->sib_MappingInfo.list.array[0]];

  Mac_rlc_xface->rrc_mac_config_req(Mod_id,0,0,eNB_index,
				    (RadioResourceConfigCommonSIB_t *)NULL,
				    (struct PhysicalConfigDedicated *)NULL,
				    (MAC_MainConfig_t *)NULL,
				    0,
				    (struct LogicalChannelConfig *)NULL,
				    (MeasGapConfig_t *)NULL,
				    UE_rrc_inst[Mod_id].sib1[eNB_index]->tdd_Config,
				    &UE_rrc_inst[Mod_id].Info[eNB_index].SIwindowsize,
				    &UE_rrc_inst[Mod_id].Info[eNB_index].SIperiod);

  return 0;

}

  
void dump_sib2(SystemInformationBlockType2_t *sib2) {

  msg("radioResourceConfigCommon.rach_ConfigCommon.preambleInfo.numberOfRA_Preambles : %ld\n",
      sib2->radioResourceConfigCommon.rach_ConfigCommon.preambleInfo.numberOfRA_Preambles);

  //  if (radioResourceConfigCommon.rach_ConfigCommon.preambleInfo.preamblesGroupAConfig)
  //msg("radioResourceConfigCommon.rach_ConfigCommon.preambleInfo.preamblesGroupAConfig ",sib2->radioResourceConfigCommon.rach_ConfigCommon.preambleInfo.preamblesGroupAConfig = NULL;

  msg("radioResourceConfigCommon.rach_ConfigCommon.powerRampingParameters.powerRampingStep : %ld\n",sib2->radioResourceConfigCommon.rach_ConfigCommon.powerRampingParameters.powerRampingStep);

  msg("radioResourceConfigCommon.rach_ConfigCommon.powerRampingParameters.preambleInitialReceivedTargetPower : %ld\n",sib2->radioResourceConfigCommon.rach_ConfigCommon.powerRampingParameters.preambleInitialReceivedTargetPower);

  msg("radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.preambleTransMax  : %ld\n",sib2->radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.preambleTransMax);

  msg("radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.ra_ResponseWindowSize : %ld\n",sib2->radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.ra_ResponseWindowSize);

  msg("radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.mac_ContentionResolutionTimer : %ld\n",sib2->radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.mac_ContentionResolutionTimer);

  msg("radioResourceConfigCommon.rach_ConfigCommon.maxHARQ_Msg3Tx : %ld\n",
      sib2->radioResourceConfigCommon.rach_ConfigCommon.maxHARQ_Msg3Tx);

  msg("radioResourceConfigCommon.prach_Config.rootSequenceIndex : %ld\n",sib2->radioResourceConfigCommon.prach_Config.rootSequenceIndex);
  msg("radioResourceConfigCommon.prach_Config.prach_ConfigInfo.prach_ConfigIndex : %ld\n",sib2->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.prach_ConfigIndex);
  msg("radioResourceConfigCommon.prach_Config.prach_ConfigInfo.highSpeedFlag : %d\n",  (int)sib2->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.highSpeedFlag);
  msg("radioResourceConfigCommon.prach_Config.prach_ConfigInfo.zeroCorrelationZoneConfig : %ld\n",  sib2->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.zeroCorrelationZoneConfig);
  msg("radioResourceConfigCommon.prach_Config.prach_ConfigInfo.prach_FreqOffset %ld\n",  sib2->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.prach_FreqOffset);

  // PDSCH-Config
  msg("radioResourceConfigCommon.pdsch_ConfigCommon.referenceSignalPower  : %ld\n",sib2->radioResourceConfigCommon.pdsch_ConfigCommon.referenceSignalPower);
  msg("radioResourceConfigCommon.pdsch_ConfigCommon.p_b : %ld\n",sib2->radioResourceConfigCommon.pdsch_ConfigCommon.p_b);

  // PUSCH-Config
  msg("radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.n_SB  : %ld\n",sib2->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.n_SB);
  msg("radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.hoppingMode  : %ld\n",sib2->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.hoppingMode);
  msg("radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.pusch_HoppingOffset : %ld\n",sib2->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.pusch_HoppingOffset);
  msg("radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.enable64QAM : %d\n",(int)sib2->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.enable64QAM);
  msg("radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupHoppingEnabled : %d\n",(int)sib2->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupHoppingEnabled);
  msg("radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH : %ld\n",sib2->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH);
  msg("radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled : %d\n",(int)sib2->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled);
  msg("radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.cyclicShift : %ld\n",sib2->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.cyclicShift);

  // PUCCH-Config

  msg("radioResourceConfigCommon.pucch_ConfigCommon.deltaPUCCH_Shift : %ld\n",sib2->radioResourceConfigCommon.pucch_ConfigCommon.deltaPUCCH_Shift);
  msg("radioResourceConfigCommon.pucch_ConfigCommon.nRB_CQI : %ld\n",sib2->radioResourceConfigCommon.pucch_ConfigCommon.nRB_CQI);
  msg("radioResourceConfigCommon.pucch_ConfigCommon.nCS_AN : %ld\n",sib2->radioResourceConfigCommon.pucch_ConfigCommon.nCS_AN);
  msg("radioResourceConfigCommon.pucch_ConfigCommon.n1PUCCH_AN : %ld\n",sib2->radioResourceConfigCommon.pucch_ConfigCommon.n1PUCCH_AN);

  msg("radioResourceConfigCommon.soundingRS_UL_ConfigCommon.present : %d\n",sib2-> radioResourceConfigCommon.soundingRS_UL_ConfigCommon.present);


  // uplinkPowerControlCommon

  msg("radioResourceConfigCommon.uplinkPowerControlCommon.p0_NominalPUSCH : %ld\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.p0_NominalPUSCH);
  msg("radioResourceConfigCommon.uplinkPowerControlCommon.alpha : %ld\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.alpha);

  msg("radioResourceConfigCommon.uplinkPowerControlCommon.p0_NominalPUCCH : %ld\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.p0_NominalPUCCH);
  msg("radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format1 : %ld\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format1);
  msg("radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format1b :%ld\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format1b);
  msg("radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2  :%ld\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2);
  msg("radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2a :%ld\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2a);
  msg("radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2b :%ld\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2b);

  msg("radioResourceConfigCommon.uplinkPowerControlCommon.deltaPreambleMsg3 : %ld\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.deltaPreambleMsg3);

  msg("radioResourceConfigCommon.ul_CyclicPrefixLength : %ld\n", sib2->radioResourceConfigCommon.ul_CyclicPrefixLength);

  msg("ue_TimersAndConstants.t300 : %ld\n", sib2->ue_TimersAndConstants.t300);
  msg("ue_TimersAndConstants.t301 : %ld\n", sib2->ue_TimersAndConstants.t301);
  msg("ue_TimersAndConstants.t310 : %ld\n", sib2->ue_TimersAndConstants.t310);
  msg("ue_TimersAndConstants.n310 : %ld\n", sib2->ue_TimersAndConstants.n310);
  msg("ue_TimersAndConstants.t311 : %ld\n", sib2->ue_TimersAndConstants.t311);
  msg("ue_TimersAndConstants.n311 : %ld\n", sib2->ue_TimersAndConstants.n311);

  msg("freqInfo.additionalSpectrumEmission : %ld\n",sib2->freqInfo.additionalSpectrumEmission);
  msg("freqInfo.ul_CarrierFreq : %d\n",(int)sib2->freqInfo.ul_CarrierFreq);
  msg("freqInfo.ul_Bandwidth : %d\n",(int)sib2->freqInfo.ul_Bandwidth);
  msg("mbsfn_SubframeConfigList : %d\n",(int)sib2->mbsfn_SubframeConfigList);
  msg("timeAlignmentTimerCommon : %ld\n",sib2->timeAlignmentTimerCommon);



}

void dump_sib3(SystemInformationBlockType3_t *sib3) {

}

//const char SIBPeriod[7][7]= {"80ms\0","160ms\0","320ms\0","640ms\0","1280ms\0","2560ms\0","5120ms\0"};
int decode_SI(u8 Mod_id,u8 eNB_index,u8 si_window) {

  asn_dec_rval_t dec_rval;
  SystemInformation_t **si=&UE_rrc_inst[Mod_id].si[eNB_index][si_window];
  int i;
  struct SystemInformation_r8_IEs__sib_TypeAndInfo__Member *typeandinfo;

  if (si_window>8) {
    msg("[RRC][UE], not enough windows (%d>8)\n",si_window);
    return -1;
  }
  memset(*si,0,sizeof(SystemInformation_t));
  dec_rval = uper_decode(NULL,
			 &asn_DEF_SystemInformation,
			 (void**)si,
			 (uint8_t*)UE_rrc_inst[Mod_id].SI[eNB_index],
			 100,0,0);

  if ((dec_rval.code != RC_OK) || (dec_rval.consumed==0)) {
    msg("[RRC][UE %d] Frame %d : Failed to decode SI (%d bytes)\n",Mod_id,Mac_rlc_xface->frame,dec_rval.consumed);
    return -1;
  }


  msg("[RRC][UE %d] Frame %d : Dumping SI from window %d (%d bytes)\n",Mod_id,Mac_rlc_xface->frame,si_window,dec_rval.consumed);
  for (i=0;i<30;i++)
    msg("%x.",UE_rrc_inst[Mod_id].SI[eNB_index][i]);
  msg("\n");

  // Dump contents
  if ((*si)->criticalExtensions.present==SystemInformation__criticalExtensions_PR_systemInformation_r8) {
    msg("(*si)->criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list.count %d\n",
       (*si)->criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list.count);
  }
  else {
    msg("[RRC][UE] Unknown criticalExtension version (not Rel8)\n");
    return -1;
  }

  for (i=0;i<(*si)->criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list.count;i++) {

    typeandinfo=(*si)->criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list.array[i];

    switch(typeandinfo->present) {
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib2:
      UE_rrc_inst[Mod_id].sib2[eNB_index] = &typeandinfo->choice.sib2;
      msg("[RRC][UE %d] Found SIB2 from eNB %d\n",Mod_id,eNB_index);
      dump_sib2(UE_rrc_inst[Mod_id].sib2[eNB_index]);
      Mac_rlc_xface->rrc_mac_config_req(Mod_id,0,0,eNB_index,
					&UE_rrc_inst[Mod_id].sib2[eNB_index]->radioResourceConfigCommon,
					(struct PhysicalConfigDedicated *)NULL,
					(MAC_MainConfig_t *)NULL,
					0,
					(struct LogicalChannelConfig *)NULL,
					(MeasGapConfig_t *)NULL,
					(TDD_Config_t *)NULL,
					NULL,
					NULL);
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib3:
      UE_rrc_inst[Mod_id].sib3[eNB_index] = &typeandinfo->choice.sib3;
      msg("[RRC][UE %d] Found SIB3 from eNB %d\n",Mod_id,eNB_index);
      dump_sib3(UE_rrc_inst[Mod_id].sib3[eNB_index]);
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib4:
      UE_rrc_inst[Mod_id].sib4[eNB_index] = &typeandinfo->choice.sib4;
      msg("[RRC][UE %d] Found SIB4 from eNB %d\n",Mod_id,eNB_index);
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib5:
      UE_rrc_inst[Mod_id].sib5[eNB_index] = &typeandinfo->choice.sib5;
      msg("[RRC][UE %d] Found SIB5 from eNB %d\n",Mod_id,eNB_index);
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib6:
      UE_rrc_inst[Mod_id].sib6[eNB_index] = &typeandinfo->choice.sib6;
      msg("[RRC][UE %d] Found SIB6 from eNB %d\n",Mod_id,eNB_index);
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib7:
      UE_rrc_inst[Mod_id].sib7[eNB_index] = &typeandinfo->choice.sib7;
      msg("[RRC][UE %d] Found SIB7 from eNB %d\n",Mod_id,eNB_index);
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib8:
      UE_rrc_inst[Mod_id].sib8[eNB_index] = &typeandinfo->choice.sib8;
      msg("[RRC][UE %d] Found SIB8 from eNB %d\n",Mod_id,eNB_index);
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib9:
      UE_rrc_inst[Mod_id].sib9[eNB_index] = &typeandinfo->choice.sib9;
      msg("[RRC][UE %d] Found SIB9 from eNB %d\n",Mod_id,eNB_index);
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib10:
      UE_rrc_inst[Mod_id].sib10[eNB_index] = &typeandinfo->choice.sib10;
      msg("[RRC][UE %d] Found SIB10 from eNB %d\n",Mod_id,eNB_index);
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib11:
      UE_rrc_inst[Mod_id].sib11[eNB_index] = &typeandinfo->choice.sib11;
      msg("[RRC][UE %d] Found SIB11 from eNB %d\n",Mod_id,eNB_index);
      break;
    default:
      break;
    }
    
  }

  return 0;
}



#ifndef USER_MODE
EXPORT_SYMBOL(Rlc_info_am_config);
#endif
