/*________________________rrc_eNB.c________________________

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
#include "TDD-Config.h"
#define DEBUG_RRC 1
#ifdef PHY_EMUL
#include "SIMULATION/simulation_defs.h"
extern EMULATION_VARS *Emul_vars;
#endif
extern eNB_MAC_INST *eNB_mac_inst;
extern UE_MAC_INST *UE_mac_inst;
#ifdef BIGPHYSAREA
extern void *bigphys_malloc(int);
#endif

extern inline unsigned int taus(void);
void init_SI(u8 Mod_id) {

  u8 SIwindowsize=1;
  u16 SIperiod=8;

  eNB_rrc_inst[Mod_id].sizeof_SIB1 = 0;
  eNB_rrc_inst[Mod_id].sizeof_SIB23 = 0;

  eNB_rrc_inst[Mod_id].SIB1 = (u8 *)malloc16(32);

  if (eNB_rrc_inst[Mod_id].SIB1)
    eNB_rrc_inst[Mod_id].sizeof_SIB1 = do_SIB1(eNB_rrc_inst[Mod_id].SIB1,
					      &eNB_rrc_inst[Mod_id].sib1);
  else {
    msg("[RRC][eNB] init_SI: FATAL, no memory for SIB1 allocated\n");
    mac_xface->macphy_exit("");
  }

  if (eNB_rrc_inst[Mod_id].sizeof_SIB1 == -1)
    mac_xface->macphy_exit("");

  eNB_rrc_inst[Mod_id].SIB23 = (u8 *)malloc16(64);
  if (eNB_rrc_inst[Mod_id].SIB23) {
    eNB_rrc_inst[Mod_id].sizeof_SIB23 = do_SIB23(eNB_rrc_inst[Mod_id].SIB23,
						&eNB_rrc_inst[Mod_id].systemInformation,
						&eNB_rrc_inst[Mod_id].sib2,
						&eNB_rrc_inst[Mod_id].sib3);
    if (eNB_rrc_inst[Mod_id].sizeof_SIB23 == -1)
      mac_xface->macphy_exit("");

    msg("SIB2/3 Contents (partial)\n");
      
    msg("pusch_config_common.n_SB = %d\n",eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.n_SB);
    
    
    msg("pusch_config_common.hoppingMode = %d\n",eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.hoppingMode);
    
    msg("pusch_config_common.pusch_HoppingOffset = %d\n",  eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.pusch_HoppingOffset);
    
    msg("pusch_config_common.enable64QAM = %d\n",eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.enable64QAM);
    
    msg("pusch_config_common.groupHoppingEnabled = %d\n",eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupHoppingEnabled);
    
    
    msg("pusch_config_common.groupAssignmentPUSCH = %d\n",eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH);
    
    
    msg("pusch_config_common.sequenceHoppingEnabled = %d\n",eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled);
    
    
    msg("pusch_config_common.cyclicShift  = %d\n",eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.cyclicShift);  
    
    Mac_rlc_xface->rrc_mac_config_req(Mod_id,1,0,0,
				      (RadioResourceConfigCommonSIB_t *)&eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon,
				      (struct PhysicalConfigDedicated *)NULL,
				      eNB_rrc_inst[Mod_id].sib1.tdd_Config,
				      &SIwindowsize,
				      &SIperiod);
  }
  else {
    msg("[RRC][eNB] init_SI: FATAL, no memory for SIB2/3 allocated\n");
    mac_xface->macphy_exit("");
  }
}

/*------------------------------------------------------------------------------*/
char openair_rrc_eNB_init(u8 Mod_id){
  /*-----------------------------------------------------------------------------*/

  unsigned char j;
  msg("[OPENAIR][RRC][INIT eNB] Mod_id:%d\n",Mod_id);
  eNB_rrc_inst[Mod_id].Info.Status = CH_READY;
  eNB_rrc_inst[Mod_id].Info.Nb_ue=0;


  eNB_rrc_inst[Mod_id].Srb0.Active=0;

  for(j=0;j<(NB_CNX_eNB+1);j++){
    eNB_rrc_inst[Mod_id].Srb2[j].Active=0;
  }

  /// System Information INIT
  init_SI(Mod_id);

  msg("[OPENAIR][RRC][INIT] INIT OK for eNB %d\n",Mod_id);

#ifdef NO_RRM //init ch SRB0, SRB1 & BDTCH
  openair_rrc_on(Mod_id);
#else
  eNB_rrc_inst[Mod_id].Last_scan_req=0;
   send_msg(&S_rrc,msg_rrc_phy_synch_to_MR_ind(Mod_id,eNB_rrc_inst[Mod_id].Mac_id));
#endif
   msg("\nRRC: INIT eNB %d Successful \n\n",Mod_id);


  return 0;

}


u8 get_next_UE_index(u8 Mod_id,u8 *UE_identity) {

  u8 i,first_index = 255,reg=0;

  for (i=0;i<NB_CNX_eNB;i++) {


    if ((first_index == 255) && (*(unsigned int*)eNB_rrc_inst[Mod_id].Info.UE_list[i] == 0x00000000))
      first_index = i;  // save first free position

    if ((eNB_rrc_inst[Mod_id].Info.UE_list[i][0]==UE_identity[0]) &&
	(eNB_rrc_inst[Mod_id].Info.UE_list[i][1]==UE_identity[1]) &&
	(eNB_rrc_inst[Mod_id].Info.UE_list[i][2]==UE_identity[2]) &&
	(eNB_rrc_inst[Mod_id].Info.UE_list[i][3]==UE_identity[3]) &&
	(eNB_rrc_inst[Mod_id].Info.UE_list[i][4]==UE_identity[4]))      // UE_identity already registered
      reg=1;

  }

  if (reg==0)
    return(first_index);
  else
    return(255);
}

/*------------------------------------------------------------------------------*/
void rrc_eNB_decode_dcch(u8 Mod_id,  u8 Srb_id, u8 UE_index, u8 *Rx_sdu, u8 sdu_size) {
  /*------------------------------------------------------------------------------*/

  asn_dec_rval_t dec_rval;
  UL_DCCH_Message_t uldcchmsg;
  UL_DCCH_Message_t *ul_dcch_msg=&uldcchmsg;

  if (Srb_id != 1) {
    msg("[RRC][eNB %d] Frame %d: Received message on SRB%d, should not have ...\n",Mod_id,Mac_rlc_xface->frame,Srb_id);
  }

  memset(ul_dcch_msg,0,sizeof(UL_DCCH_Message_t));

  msg("[RRC][eNB %d] Frame %d: Decoding UL-DCCH Message\n",
      Mod_id,Mac_rlc_xface->frame);
  dec_rval = uper_decode(NULL,
			 &asn_DEF_UL_DCCH_Message,
			 (void**)&ul_dcch_msg,
			 Rx_sdu,
			 100,0,0);

  if (ul_dcch_msg->message.present == UL_DCCH_MessageType_PR_c1) {

    switch (ul_dcch_msg->message.choice.c1.present) {

    case UL_DCCH_MessageType__c1_PR_NOTHING:     /* No components present */
      break;
    case UL_DCCH_MessageType__c1_PR_csfbParametersRequestCDMA2000:
      break;
    case UL_DCCH_MessageType__c1_PR_measurementReport:
      break;
    case UL_DCCH_MessageType__c1_PR_rrcConnectionReconfigurationComplete:
      msg("[RRC][eNB %d] Processing RRCConnectionReconfigurationComplete message\n",Mod_id);
      if (ul_dcch_msg->message.choice.c1.choice.rrcConnectionReconfigurationComplete.criticalExtensions.present == RRCConnectionSetupComplete__criticalExtensions__c1_PR_rrcConnectionSetupComplete_r8)
	rrc_eNB_process_RRCConnectionReconfigurationComplete(Mod_id,UE_index,&ul_dcch_msg->message.choice.c1.choice.rrcConnectionReconfigurationComplete.criticalExtensions.choice.rrcConnectionReconfigurationComplete_r8);
      break;
    case UL_DCCH_MessageType__c1_PR_rrcConnectionReestablishmentComplete:
      break;
    case UL_DCCH_MessageType__c1_PR_rrcConnectionSetupComplete:
      msg("[RRC][eNB %d] Processing RRCConnectionSetupComplete message\n",Mod_id);
      if (ul_dcch_msg->message.choice.c1.choice.rrcConnectionSetupComplete.criticalExtensions.present == RRCConnectionSetupComplete__criticalExtensions__c1_PR_rrcConnectionSetupComplete_r8)
	rrc_eNB_process_RRCConnectionSetupComplete(Mod_id,UE_index,&ul_dcch_msg->message.choice.c1.choice.rrcConnectionSetupComplete.criticalExtensions.choice.c1.choice.rrcConnectionSetupComplete_r8);
      break;
    case UL_DCCH_MessageType__c1_PR_securityModeComplete:
      break;
    case UL_DCCH_MessageType__c1_PR_securityModeFailure:
      break;
    case UL_DCCH_MessageType__c1_PR_ueCapabilityInformation:
      break;
    case UL_DCCH_MessageType__c1_PR_ulHandoverPreparationTransfer:
      break;
    case UL_DCCH_MessageType__c1_PR_ulInformationTransfer:
      break;
    case UL_DCCH_MessageType__c1_PR_counterCheckResponse:
      break;
    case UL_DCCH_MessageType__c1_PR_spare1:
    case UL_DCCH_MessageType__c1_PR_spare2:
    case UL_DCCH_MessageType__c1_PR_spare3:
    case UL_DCCH_MessageType__c1_PR_spare4:
    case UL_DCCH_MessageType__c1_PR_spare5:
      break;

    }
  }
}


/*------------------------------------------------------------------------------*/
void rrc_eNB_decode_ccch(u8 Mod_id, SRB_INFO *Srb_info){
  /*------------------------------------------------------------------------------*/

  u16 Idx,UE_index;

  asn_dec_rval_t dec_rval;
  UL_CCCH_Message_t ulccchmsg;
  UL_CCCH_Message_t *ul_ccch_msg=&ulccchmsg;
  RRCConnectionRequest_r8_IEs_t *rrcConnectionRequest;



  memset(ul_ccch_msg,0,sizeof(UL_CCCH_Message_t));

  msg("[RRC][eNB %d] Frame %d: Decoding CCCH %x.%x.%x.%x.%x.%x (%p)\n", Mod_id,Mac_rlc_xface->frame,
      ((uint8_t*)Srb_info->Rx_buffer.Payload)[0],
      ((uint8_t*)Srb_info->Rx_buffer.Payload)[1],
      ((uint8_t*)Srb_info->Rx_buffer.Payload)[2],
      ((uint8_t*)Srb_info->Rx_buffer.Payload)[3],
      ((uint8_t*)Srb_info->Rx_buffer.Payload)[4],
      ((uint8_t*)Srb_info->Rx_buffer.Payload)[5],
      (uint8_t*)Srb_info->Rx_buffer.Payload);
  dec_rval = uper_decode(NULL,
			 &asn_DEF_UL_CCCH_Message,
			 (void**)&ul_ccch_msg,
			 (uint8_t*)Srb_info->Rx_buffer.Payload,
			 100,0,0);
  if (dec_rval.consumed == 0) {
    msg("[RRC][eNB] FATAL Error in receiving CCCH\n");
    exit(-1);
  }
  if (ul_ccch_msg->message.present == UL_CCCH_MessageType_PR_c1) {

    switch (ul_ccch_msg->message.choice.c1.present) {

    case UL_CCCH_MessageType__c1_PR_NOTHING :
      msg("[RRC][eNB %d] Frame %d : Received PR_NOTHING on UL-CCCH-Message\n",Mod_id,Mac_rlc_xface->frame);
      return;
      break;

    case UL_CCCH_MessageType__c1_PR_rrcConnectionRequest :

      rrcConnectionRequest = &ul_ccch_msg->message.choice.c1.choice.rrcConnectionRequest.criticalExtensions.choice.rrcConnectionRequest_r8;
      UE_index = get_next_UE_index(Mod_id,(u8 *)rrcConnectionRequest->ue_Identity.choice.randomValue.buf);

      if (UE_index!=255) {

	memcpy(&Rrc_xface->UE_id[Mod_id][UE_index],(u8 *)rrcConnectionRequest->ue_Identity.choice.randomValue.buf,5);
	memcpy(&eNB_rrc_inst[Mod_id].Info.UE_list[UE_index],(u8 *)rrcConnectionRequest->ue_Identity.choice.randomValue.buf,5);

	msg("_______________________[OPENAIR][RRC] eNB %d, Frame %d : Accept New connexion from UE %x%x%x%x%x (UE_index %d)____________\n",Mod_id,Rrc_xface->Frame_index,
	    eNB_rrc_inst[Mod_id].Info.UE_list[UE_index][0],
	    eNB_rrc_inst[Mod_id].Info.UE_list[UE_index][1],
	    eNB_rrc_inst[Mod_id].Info.UE_list[UE_index][2],
	    eNB_rrc_inst[Mod_id].Info.UE_list[UE_index][3],
	    eNB_rrc_inst[Mod_id].Info.UE_list[UE_index][4],
	    UE_index);

	//CONFIG SRB2  (DCCHs, ONE per User)  //meas && lchan Cfg
	//eNB_rrc_inst[Mod_id].Info.Dtch_bd_config[UE_index].Status=NEED_RADIO_CONFIG;
	//eNB_rrc_inst[Mod_id].Info.Dtch_bd_config[UE_index].Next_eNBeck_frame=Rrc_xface->Frame_index+1;
	eNB_rrc_inst[Mod_id].Info.Nb_ue++;

#ifndef NO_RRM
	send_msg(&S_rrc,msg_rrc_MR_attach_ind(Mod_id,Mac_id));
#else


	Idx = (UE_index * MAX_NUM_RB) + DCCH;
	// SRB1
	eNB_rrc_inst[Mod_id].Srb1[UE_index].Active = 1;
	eNB_rrc_inst[Mod_id].Srb1[UE_index].Srb_info.Srb_id = Idx;
	memcpy(&eNB_rrc_inst[Mod_id].Srb1[UE_index].Srb_info.Lchan_desc[0],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
	memcpy(&eNB_rrc_inst[Mod_id].Srb1[UE_index].Srb_info.Lchan_desc[1],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);

	// SRB2
	eNB_rrc_inst[Mod_id].Srb2[UE_index].Active = 1;
	eNB_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.Srb_id = Idx;
	memcpy(&eNB_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.Lchan_desc[0],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
	memcpy(&eNB_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.Lchan_desc[1],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);

	rrc_eNB_generate_RRCConnectionSetup(Mod_id,UE_index);

	msg("[OPENAIR][RRC] CALLING RLC CONFIG SRB1 (rbid %d) for UE %d\n",
	    Idx,UE_index);
	Mac_rlc_xface->rrc_rlc_config_req(Mod_id,ACTION_ADD,Idx,SIGNALLING_RADIO_BEARER,Rlc_info_am_config);
	/*
	msg("[OPENAIR][RRC] CALLING RLC CONFIG SRB2 (rbid %d) for UE %d\n",
	    Idx,UE_index);
	Mac_rlc_xface->rrc_rlc_config_req(Mod_id,ACTION_ADD,Idx+1,SIGNALLING_RADIO_BEARER,Rlc_info_am_config);
	*/

#endif //NO_RRM
	break;

    case UL_CCCH_MessageType__c1_PR_rrcConnectionReestablishmentRequest :
      msg("[RRC][eNB %d] Frame %d : RRCConnectionReestablishmentRequest not supported yet\n",Mod_id,Mac_rlc_xface->frame);
      break;
      }
    }


  }

}


mui_t rrc_eNB_mui=0;

void rrc_eNB_generate_RRCConnectionReconfiguration(u8 Mod_id,u16 UE_index) {

  u8 buffer[100];
  u8 size;

  size = do_RRCConnectionReconfiguration(buffer,
					 UE_index,
					 0,
					 &eNB_rrc_inst[Mod_id].SRB2_config[UE_index],
					 &eNB_rrc_inst[Mod_id].DRB_config[UE_index][0],
					 &eNB_rrc_inst[Mod_id].physicalConfigDedicated[UE_index]);

  msg("[RRC][eNB %d] Generate %d bytes (RRCConnectionReconfiguration) for DCCH UE %d: 1 ",Mod_id,size,UE_index);

  Mac_rlc_xface->rrc_rlc_data_req(Mod_id,(UE_index*MAX_NUM_RB)+DCCH,rrc_eNB_mui++,0,size,(char*)buffer);



}

void rrc_eNB_process_RRCConnectionSetupComplete(u8 Mod_id, u8 UE_index,RRCConnectionSetupComplete_r8_IEs_t *rrcConnectionSetupComplete) {



  // initiate RRCConnectionReconfiguration on SRB1
  rrc_eNB_generate_RRCConnectionReconfiguration(Mod_id,UE_index);

  // process information

  Mac_rlc_xface->rrc_mac_config_req(Mod_id,1,UE_index,0,NULL,
				    eNB_rrc_inst[Mod_id].physicalConfigDedicated[UE_index],NULL,NULL,NULL);

}

void rrc_eNB_process_RRCConnectionReconfigurationComplete(u8 Mod_id,u8 UE_index,RRCConnectionReconfigurationComplete_r8_IEs_t *rrcConnectionReconfigurationComplete){

    //Establish DRB (DTCH)
  msg("[RRC][eNB %d] Received RRCConnectionReconfigurationComplete from UE %d, configuring DRB %d/LCID %d\n",Mod_id,UE_index,
      (int)eNB_rrc_inst[Mod_id].DRB_config[UE_index][0]->drb_Identity,
      (int)*eNB_rrc_inst[Mod_id].DRB_config[UE_index][0]->logicalChannelIdentity);
  Mac_rlc_xface->rrc_rlc_config_req(Mod_id,ACTION_ADD,
				    (UE_index * MAX_NUM_RB) + *eNB_rrc_inst[Mod_id].DRB_config[UE_index][0]->logicalChannelIdentity,
				    RADIO_ACCESS_BEARER,Rlc_info_um);

}

void rrc_eNB_generate_RRCConnectionSetup(u8 Mod_id,u16 UE_index) {


  eNB_rrc_inst[Mod_id].Srb0.Tx_buffer.payload_size = do_RRCConnectionSetup((u8 *)eNB_rrc_inst[Mod_id].Srb0.Tx_buffer.Payload,
								   UE_index,0,
								   &eNB_rrc_inst[Mod_id].SRB1_config[UE_index],
								   &eNB_rrc_inst[Mod_id].SRB2_config[UE_index],
								   &eNB_rrc_inst[Mod_id].physicalConfigDedicated[UE_index]);

  msg("[RRC][eNB %d] Generate %d bytes (RRCConnectionSetup for UE %d) for CCCH : 0 ",Mod_id,eNB_rrc_inst[Mod_id].Srb0.Tx_buffer.payload_size,UE_index);




}


void ue_rrc_process_rrcConnectionReconfiguration(u8 Mod_id,
						 RRCConnectionReconfiguration_t *rrcConnectionReconfiguration,
						 u8 CH_index) {

  if (rrcConnectionReconfiguration->criticalExtensions.present == RRCConnectionReconfiguration__criticalExtensions__c1_PR_rrcConnectionReconfiguration_r8) {

    if (rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated) {
      rrc_ue_process_radioResourceConfigDedicated(Mod_id,CH_index,
						  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated);


    }

    // check other fields for
  }
}

#ifndef USER_MODE
EXPORT_SYMBOL(Rlc_info_am_config);
#endif
