/*******************************************************************************

  Eurecom OpenAirInterface 2
  Copyright(c) 1999 - 2010 Eurecom

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fsr/openairinterface
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/


/*! \file rrc_eNB.c
* \brief rrc procedures for eNB
* \author Raymond Knopp and Navid Nikaein
* \date 2011
* \version 1.0 
* \company Eurecom
* \email: raymond.knopp@eurecom.fr and navid.nikaein@eurecom.fr
*/ 


#include "defs.h"
#include "extern.h"
#include "RRC/L2_INTERFACE/openair_rrc_L2_interface.h"
#include "LAYER2/RLC/rlc.h"
#include "UTIL/LOG/log.h"
#include "COMMON/mac_rrc_primitives.h"
#include "RRC/LITE/MESSAGES/asn1_msg.h"
#include "RRCConnectionRequest.h"
#include "UL-CCCH-Message.h"
#include "DL-CCCH-Message.h"
#include "UL-DCCH-Message.h"
#include "DL-DCCH-Message.h"
#include "TDD-Config.h"
#include "rlc.h"
#include "SIMULATION/ETH_TRANSPORT/extern.h"
#ifdef USER_MODE
#include "RRC/NAS/nas_config.h"
#include "RRC/NAS/rb_config.h"
#endif
#ifdef PHY_EMUL
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
    eNB_rrc_inst[Mod_id].sizeof_SIB1 = do_SIB1(mac_xface->lte_frame_parms,
					       eNB_rrc_inst[Mod_id].SIB1,
					      &eNB_rrc_inst[Mod_id].sib1);
  else {
    LOG_E(RRC,"[eNB] init_SI: FATAL, no memory for SIB1 allocated\n");
    mac_xface->macphy_exit("");
  }

  if (eNB_rrc_inst[Mod_id].sizeof_SIB1 == 255)
    mac_xface->macphy_exit("");

  eNB_rrc_inst[Mod_id].SIB23 = (u8 *)malloc16(64);
  if (eNB_rrc_inst[Mod_id].SIB23) {
    eNB_rrc_inst[Mod_id].sizeof_SIB23 = do_SIB23(Mod_id,
						 eNB_rrc_inst[Mod_id].SIB23,
						&eNB_rrc_inst[Mod_id].systemInformation,
						&eNB_rrc_inst[Mod_id].sib2,
						&eNB_rrc_inst[Mod_id].sib3,
                                                &eNB_rrc_inst[Mod_id].sib13,
                                                eNB_rrc_inst[Mod_id].MBMS_flag);
    if (eNB_rrc_inst[Mod_id].sizeof_SIB23 == 255)
      mac_xface->macphy_exit("");

    LOG_D(RRC,"[eNB %d] SIB2/3 Contents (partial)\n", Mod_id);

    LOG_D(RRC,"[eNB %d] pusch_config_common.n_SB = %ld\n", Mod_id,eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.n_SB);


    LOG_D(RRC,"[eNB %d] pusch_config_common.hoppingMode = %ld\n", Mod_id, eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.hoppingMode);

    LOG_D(RRC,"[eNB %d] pusch_config_common.pusch_HoppingOffset = %ld\n", Mod_id,eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.pusch_HoppingOffset);

    LOG_D(RRC,"[eNB %d] pusch_config_common.enable64QAM = %d\n", Mod_id,(int)eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.enable64QAM);

    LOG_D(RRC,"[eNB %d] pusch_config_common.groupHoppingEnabled = %d\n", Mod_id,(int)eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupHoppingEnabled);


    LOG_D(RRC,"[eNB %d] pusch_config_common.groupAssignmentPUSCH = %ld\n", Mod_id,eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH);


    LOG_D(RRC,"[eNB %d] pusch_config_common.sequenceHoppingEnabled = %d\n", Mod_id,(int)eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled);


    LOG_D(RRC, "[eNB %d] pusch_config_common.cyclicShift  = %ld\n",Mod_id, eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.cyclicShift);

  LOG_D(RRC, "[MSC_MSG][FRAME unknown][RRC_UE][MOD %02d][][--- MAC_CONFIG_REQ (SIB1.tdd & SIB2 params) --->][MAC_UE][MOD %02d][]\n",
             Mod_id, Mod_id);
    Mac_rlc_xface->rrc_mac_config_req(Mod_id,1,0,0,
				      (RadioResourceConfigCommonSIB_t *)&eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon,
				      (struct PhysicalConfigDedicated *)NULL,
				      (MAC_MainConfig_t *)NULL,
				      0,
				      (struct LogicalChannelConfig *)NULL,
				      (MeasGapConfig_t *)NULL,
				      eNB_rrc_inst[Mod_id].sib1.tdd_Config,
				      &SIwindowsize,
				      &SIperiod);
  }
  else {
    LOG_E(RRC,"[eNB] init_SI: FATAL, no memory for SIB2/3 allocated\n");
    mac_xface->macphy_exit("");
  }
}

/*------------------------------------------------------------------------------*/
char openair_rrc_eNB_init(u8 Mod_id){
  /*-----------------------------------------------------------------------------*/

  unsigned char j;
  LOG_I(RRC,"[eNB %d] Init (UE State = RRC_IDLE)...\n",Mod_id);
  LOG_D(RRC, "[MSC_NEW][FRAME 00000][RRC_eNB][MOD %02d][]\n", Mod_id);

  for (j=0; j<NB_CNX_eNB; j++)
    eNB_rrc_inst[Mod_id].Info.Status[j] = CH_READY;

  eNB_rrc_inst[Mod_id].Info.Nb_ue=0;

  eNB_rrc_inst[Mod_id].Srb0.Active=0;

  for(j=0;j<(NB_CNX_eNB+1);j++){
    eNB_rrc_inst[Mod_id].Srb2[j].Active=0;
  }

  // This has to come from some top-level configuration
  eNB_rrc_inst[Mod_id].MBMS_flag = 0;

  /// System Information INIT
  init_SI(Mod_id);

#ifdef NO_RRM //init ch SRB0, SRB1 & BDTCH
  openair_rrc_on(Mod_id);
#else
  eNB_rrc_inst[Mod_id].Last_scan_req=0;
   send_msg(&S_rrc,msg_rrc_phy_synch_to_MR_ind(Mod_id,eNB_rrc_inst[Mod_id].Mac_id));
#endif

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
int rrc_eNB_decode_dcch(u8 Mod_id, u32 frame, u8 Srb_id, u8 UE_index, u8 *Rx_sdu, u8 sdu_size) {
  /*------------------------------------------------------------------------------*/

  asn_dec_rval_t dec_rval;
  UL_DCCH_Message_t uldcchmsg;
  UL_DCCH_Message_t *ul_dcch_msg=&uldcchmsg;

  if (Srb_id != 1) {
    LOG_E(RRC,"[eNB %d] Frame %d: Received message on SRB%d, should not have ...\n",Mod_id,frame,Srb_id);
  }

  memset(ul_dcch_msg,0,sizeof(UL_DCCH_Message_t));

  LOG_D(RRC,"[eNB %d] Frame %d: Decoding UL-DCCH Message\n",
      Mod_id,frame);
  dec_rval = uper_decode(NULL,
			 &asn_DEF_UL_DCCH_Message,
			 (void**)&ul_dcch_msg,
			 Rx_sdu,
			 100,0,0);

  if ((dec_rval.code != RC_OK) && (dec_rval.consumed==0)) {
    LOG_E(RRC,"[UE %d] Frame %d : Failed to decode UL-DCCH (%d bytes)\n",Mod_id,frame,dec_rval.consumed);
    return -1;
  }

  if (ul_dcch_msg->message.present == UL_DCCH_MessageType_PR_c1) {

    switch (ul_dcch_msg->message.choice.c1.present) {

    case UL_DCCH_MessageType__c1_PR_NOTHING:     /* No components present */
      break;
    case UL_DCCH_MessageType__c1_PR_csfbParametersRequestCDMA2000:
      break;
    case UL_DCCH_MessageType__c1_PR_measurementReport:
      LOG_D(RRC, "[MSC_MSG][FRAME %05d][RLC][MOD %02d][RB %02d][--- RLC_DATA_IND %d bytes (measurementReport) --->][RRC_eNB][MOD %02d][]\n",
                                     frame, Mod_id, DCCH, sdu_size, Mod_id);
      break;
    case UL_DCCH_MessageType__c1_PR_rrcConnectionReconfigurationComplete:
      LOG_D(RRC, "[MSC_MSG][FRAME %05d][RLC][MOD %02d][RB %02d][--- RLC_DATA_IND %d bytes (RRCConnectionReconfigurationComplete) --->][RRC_eNB][MOD %02d][]\n",
                                     frame, Mod_id, DCCH, sdu_size, Mod_id);
      LOG_I(RRC,"[eNB %d] Processing RRCConnectionReconfigurationComplete message\n",Mod_id);
      if (ul_dcch_msg->message.choice.c1.choice.rrcConnectionReconfigurationComplete.criticalExtensions.present == RRCConnectionReconfigurationComplete__criticalExtensions_PR_rrcConnectionReconfigurationComplete_r8)
        rrc_eNB_process_RRCConnectionReconfigurationComplete(Mod_id,frame,UE_index,&ul_dcch_msg->message.choice.c1.choice.rrcConnectionReconfigurationComplete.criticalExtensions.choice.rrcConnectionReconfigurationComplete_r8);
      break;
    case UL_DCCH_MessageType__c1_PR_rrcConnectionReestablishmentComplete:
      LOG_D(RRC, "[MSC_MSG][FRAME %05d][RLC][MOD %02d][RB %02d][--- RLC_DATA_IND %d bytes (rrcConnectionReestablishmentComplete) --->][RRC_eNB][MOD %02d][]\n",
                                     frame, Mod_id, DCCH, sdu_size, Mod_id);
      break;
    case UL_DCCH_MessageType__c1_PR_rrcConnectionSetupComplete:
      LOG_D(RRC, "[MSC_MSG][FRAME %05d][RLC][MOD %02d][RB %02d][--- RLC_DATA_IND %d bytes (RRCConnectionSetupComplete) --->][RRC_eNB][MOD %02d][]\n",
                                     frame, Mod_id, DCCH, sdu_size, Mod_id);

      if (ul_dcch_msg->message.choice.c1.choice.rrcConnectionSetupComplete.criticalExtensions.present == RRCConnectionSetupComplete__criticalExtensions_PR_c1)
	if (ul_dcch_msg->message.choice.c1.choice.rrcConnectionSetupComplete.criticalExtensions.choice.c1.present == RRCConnectionSetupComplete__criticalExtensions__c1_PR_rrcConnectionSetupComplete_r8) {
	  rrc_eNB_process_RRCConnectionSetupComplete(Mod_id,frame,UE_index,&ul_dcch_msg->message.choice.c1.choice.rrcConnectionSetupComplete.criticalExtensions.choice.c1.choice.rrcConnectionSetupComplete_r8);
	  eNB_rrc_inst[Mod_id].Info.Status[UE_index] = RRC_CONNECTED;
	  LOG_D(RRC,"[eNB %d] UE %d State = RRC_CONNECTED \n",Mod_id,UE_index);
	  LOG_D(RRC,"[MSC_NBOX][FRAME %05d][RRC_eNB][MOD %02d][][Rx RRCConnectionSetupComplete\nNow CONNECTED with UE %d][RRC_eNB][MOD %02d][]\n",
            frame, Mod_id, UE_index, Mod_id);
	}
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
    case UL_DCCH_MessageType__c1_PR_ueInformationResponse_r9:
      break;
    case UL_DCCH_MessageType__c1_PR_proximityIndication_r9:
      break;
    case UL_DCCH_MessageType__c1_PR_rnReconfigurationComplete_r10:
      break;
    case UL_DCCH_MessageType__c1_PR_mbmsCountingResponse_r10:
      break;
    case UL_DCCH_MessageType__c1_PR_interFreqRSTDMeasurementIndication_r10:
      break;
    default:
     LOG_E(RRC,"[UE %d] Frame %d : Unkown message\n",Mod_id,frame);
     return -1;
    }
    return 0;
  }
  else {
    LOG_E(RRC,"[UE %d] Frame %d : Unkown error\n",Mod_id,frame);
    return -1;
  }
}


/*------------------------------------------------------------------------------*/
int rrc_eNB_decode_ccch(u8 Mod_id, u32 frame, SRB_INFO *Srb_info){
  /*------------------------------------------------------------------------------*/

  u16 Idx,UE_index;

  asn_dec_rval_t dec_rval;
  UL_CCCH_Message_t ulccchmsg;
  UL_CCCH_Message_t *ul_ccch_msg=&ulccchmsg;
  RRCConnectionRequest_r8_IEs_t *rrcConnectionRequest;



  memset(ul_ccch_msg,0,sizeof(UL_CCCH_Message_t));

  LOG_D(RRC,"[eNB %d] Frame %d: Decoding CCCH %x.%x.%x.%x.%x.%x (%p)\n", Mod_id,frame,
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
    LOG_E(RRC,"[eNB] FATAL Error in receiving CCCH\n");
    return -1; //mac_xface->macphy_exit(""); //exit(-1);
  }
  if (ul_ccch_msg->message.present == UL_CCCH_MessageType_PR_c1) {

    switch (ul_ccch_msg->message.choice.c1.present) {

    case UL_CCCH_MessageType__c1_PR_NOTHING :
      LOG_I(RRC,"[eNB %d] Frame %d : Received PR_NOTHING on UL-CCCH-Message\n",Mod_id,frame);
      break;

    case UL_CCCH_MessageType__c1_PR_rrcConnectionRequest :
      LOG_D(RRC, "[MSC_MSG][FRAME %05d][MAC_eNB][MOD %02d][][--- MAC_DATA_IND  (rrcConnectionRequest on SRB0) -->][RRC_eNB][MOD %02d][]\n",
            frame, Mod_id, Mod_id);

      rrcConnectionRequest = &ul_ccch_msg->message.choice.c1.choice.rrcConnectionRequest.criticalExtensions.choice.rrcConnectionRequest_r8;
      UE_index = get_next_UE_index(Mod_id,(u8 *)rrcConnectionRequest->ue_Identity.choice.randomValue.buf);

      if (UE_index!=255) {

	memcpy(&Rrc_xface->UE_id[Mod_id][UE_index],(u8 *)rrcConnectionRequest->ue_Identity.choice.randomValue.buf,5);
	memcpy(&eNB_rrc_inst[Mod_id].Info.UE_list[UE_index],(u8 *)rrcConnectionRequest->ue_Identity.choice.randomValue.buf,5);

	LOG_I(RRC,"[eNB %d] Frame %d : Accept new connection from UE %d (%x%x%x%x%x)\n",Mod_id,frame,UE_index,
	    eNB_rrc_inst[Mod_id].Info.UE_list[UE_index][0],
	    eNB_rrc_inst[Mod_id].Info.UE_list[UE_index][1],
	    eNB_rrc_inst[Mod_id].Info.UE_list[UE_index][2],
	    eNB_rrc_inst[Mod_id].Info.UE_list[UE_index][3],
	    eNB_rrc_inst[Mod_id].Info.UE_list[UE_index][4]);

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

	rrc_eNB_generate_RRCConnectionSetup(Mod_id,frame,UE_index);
      //LOG_D(RRC, "[MSC_NBOX][FRAME %05d][RRC_eNB][MOD %02d][][Tx RRCConnectionSetup][RRC_eNB][MOD %02d][]\n",
      //      frame, Mod_id, Mod_id);

	//LOG_D(RRC,"[eNB %d] RLC AM allocation index@0 is %d\n",Mod_id,rlc[Mod_id].m_rlc_am_array[0].allocation);
	//LOG_D(RRC,"[eNB %d] RLC AM allocation index@1 is %d\n",Mod_id,rlc[Mod_id].m_rlc_am_array[1].allocation);
	LOG_I(RRC,"[eNB %d] CALLING RLC CONFIG SRB1 (rbid %d) for UE %d\n",
	    Mod_id,Idx,UE_index);
	rrc_rlc_config_req(Mod_id,frame,1,ACTION_ADD,Idx,SIGNALLING_RADIO_BEARER,Rlc_info_am_config);


	//LOG_D(RRC,"[eNB %d] RLC AM allocation index@0 is %d\n",Mod_id,rlc[Mod_id].m_rlc_am_array[0].allocation);
	//LOG_D(RRC,"[eNB %d] RLC AM allocation index@1 is %d\n",Mod_id,rlc[Mod_id].m_rlc_am_array[1].allocation);

	/*

	LOG_D(RRC,"[eNB %d] CALLING RLC CONFIG SRB2 (rbid %d) for UE %d\n",
	    Mod_id,Idx+1,UE_index);
	Mac_rlc_xface->rrc_rlc_config_req(Mod_id,ACTION_ADD,Idx+1,SIGNALLING_RADIO_BEARER,Rlc_info_am_config);
	LOG_D(RRC,"[eNB %d] RLC AM allocation index@0 is %d\n",Mod_id,rlc[Mod_id].m_rlc_am_array[0].allocation);
	LOG_D(RRC,"[eNB %d] RLC AM allocation index@1 is %d\n",rlc[Mod_id].m_rlc_am_array[1].allocation);
	*/
#endif //NO_RRM
	break;

    case UL_CCCH_MessageType__c1_PR_rrcConnectionReestablishmentRequest :
      LOG_D(RRC, "[MSC_MSG][FRAME %05d][MAC_eNB][MOD %02d][][--- MAC_DATA_IND (rrcConnectionReestablishmentRequest on SRB0) -->][RRC_eNB][MOD %02d][]\n",
            frame, Mod_id, Mod_id);
      LOG_I(RRC,"[eNB %d] Frame %d : RRCConnectionReestablishmentRequest not supported yet\n",Mod_id,frame);
      break;
      }
    default:
      LOG_E(RRC,"[eNB %d] Frame %d : Unkown message\n",Mod_id,frame);
      return -1;
    }
    return 0;
  }
  else{
    LOG_E(RRC,"[eNB %d] Frame %d : Unkown error \n",Mod_id,frame);
      return -1;
  }

}


mui_t rrc_eNB_mui=0;

void rrc_eNB_generate_RRCConnectionReconfiguration(u8 Mod_id,u32 frame,u16 UE_index) {

  u8 buffer[100];
  u8 size;



  size = do_RRCConnectionReconfiguration(buffer,
					 UE_index,
					 0,
					 &eNB_rrc_inst[Mod_id].SRB2_config[UE_index],
					 &eNB_rrc_inst[Mod_id].DRB_config[UE_index][0],
					 &eNB_rrc_inst[Mod_id].physicalConfigDedicated[UE_index]);

  LOG_I(RRC,"[eNB %d] Generate %d bytes (RRCConnectionReconfiguration) for DCCH UE %d\n",Mod_id,size,UE_index);

  LOG_D(RLC, "[MSC_MSG][FRAME %05d][RRC_eNB][MOD %02d][][--- RLC_DATA_REQ/%d Bytes (rrcConnectionReconfiguration to UE %d MUI %d) --->][RLC][MOD %02d][RB %02d]\n",
                                     frame, Mod_id, size, UE_index, rrc_eNB_mui, Mod_id, (UE_index*MAX_NUM_RB)+DCCH);
  rrc_rlc_data_req(Mod_id,frame, 1,(UE_index*MAX_NUM_RB)+DCCH,rrc_eNB_mui++,0,size,(char*)buffer);



}

void rrc_eNB_process_RRCConnectionSetupComplete(u8 Mod_id, u32 frame, u8 UE_index,RRCConnectionSetupComplete_r8_IEs_t *rrcConnectionSetupComplete) {


  LOG_I(RRC,"[eNB %d][RAPROC] Frame %d : processing RRCConnectionSetupComplete from UE %d\n",Mod_id,frame,UE_index);

  // configure SRB1/SRB2, PhysicalConfigDedicated, MAC_MainConfig for UE



  rrc_eNB_generate_RRCConnectionReconfiguration(Mod_id,frame,UE_index);


}

void rrc_eNB_process_RRCConnectionReconfigurationComplete(u8 Mod_id,u32 frame,u8 UE_index,RRCConnectionReconfigurationComplete_r8_IEs_t *rrcConnectionReconfigurationComplete){
  int i;

  // Loop through DRBs and establish if necessary
  for (i=0;i<8;i++) { // num max DRB (11-3-8)
    if (eNB_rrc_inst[Mod_id].DRB_config[UE_index][i]) {
      LOG_I(RRC,"[eNB %d] Received RRCConnectionReconfigurationComplete from UE %d, reconfiguring DRB %d/LCID %d\n",
	  Mod_id,UE_index,
	  (int)eNB_rrc_inst[Mod_id].DRB_config[UE_index][0]->drb_Identity,
	  (UE_index * MAX_NUM_RB) + (int)*eNB_rrc_inst[Mod_id].DRB_config[UE_index][0]->logicalChannelIdentity);
      if (eNB_rrc_inst[Mod_id].DRB_active[UE_index][i] == 0) {
	rrc_rlc_config_req(Mod_id,frame,1,ACTION_ADD,
			   (UE_index * MAX_NUM_RB) + (int)*eNB_rrc_inst[Mod_id].DRB_config[UE_index][i]->logicalChannelIdentity,
			   RADIO_ACCESS_BEARER,Rlc_info_um);
	eNB_rrc_inst[Mod_id].DRB_active[UE_index][i] = 1;
	
	LOG_D(RRC,"[eNB %d] Frame %d: Establish RLC UM Bidirectional, DRB %d Active\n", 
	      Mod_id, frame, (int)eNB_rrc_inst[Mod_id].DRB_config[UE_index][0]->drb_Identity);
#ifdef NAS_NETLINK
	nas_config(Mod_id,// interface index
		   Mod_id+1, // thrid octet
		   Mod_id+1);// fourth octet

	rb_conf_ipv4(0,//add
		     UE_index, //cx
		     Mod_id,//inst
		     (UE_index * MAX_NUM_RB) + *eNB_rrc_inst[Mod_id].DRB_config[UE_index][i]->logicalChannelIdentity,
		     0,//dscp
		     ipv4_address(Mod_id+1,Mod_id+1),//saddr
		     ipv4_address(Mod_id+1,NB_eNB_INST+UE_index+1));//daddr
#endif

	LOG_D(RRC,"[eNB %d] State = Attached (UE %d)\n",Mod_id,UE_index);	 
	LOG_D(RRC, "[MSC_MSG][FRAME %05d][RRC_eNB][MOD %02d][][--- MAC_CONFIG_REQ  (DRB UE %d) --->][MAC_eNB][MOD %02d][]\n",
	      frame, Mod_id, UE_index, Mod_id);
	DRB2LCHAN[i] = (u8)*eNB_rrc_inst[Mod_id].DRB_config[UE_index][i]->logicalChannelIdentity;
	Mac_rlc_xface->rrc_mac_config_req(Mod_id,1,UE_index,0,
					  (RadioResourceConfigCommonSIB_t *)NULL,
					  eNB_rrc_inst[Mod_id].physicalConfigDedicated[UE_index],
					  eNB_rrc_inst[Mod_id].mac_MainConfig[UE_index],
					  DRB2LCHAN[i],
					  eNB_rrc_inst[Mod_id].DRB_config[UE_index][i]->logicalChannelConfig,
					  eNB_rrc_inst[Mod_id].measGapConfig[UE_index],
					  (TDD_Config_t *)NULL,
					  (u8 *)NULL,
					  (u16 *)NULL);
      }
      else { // remove LCHAN from MAC/PHY

	if (eNB_rrc_inst[Mod_id].DRB_active[UE_index][i] ==1) {
	  // DRB has just been removed so remove RLC + PDCP for DRB
	  rrc_rlc_config_req(Mod_id,frame,1,ACTION_REMOVE,
			     (UE_index * MAX_NUM_RB) + DRB2LCHAN[i],
			     RADIO_ACCESS_BEARER,Rlc_info_um);
	}
	eNB_rrc_inst[Mod_id].DRB_active[UE_index][i] = 0;
    LOG_D(RRC, "[MSC_MSG][FRAME %05d][RRC_eNB][MOD %02d][][--- MAC_CONFIG_REQ  (DRB UE %d) --->][MAC_eNB][MOD %02d][]\n",
            frame, Mod_id, UE_index, Mod_id);

	Mac_rlc_xface->rrc_mac_config_req(Mod_id,1,UE_index,0,
					  (RadioResourceConfigCommonSIB_t *)NULL,
					  eNB_rrc_inst[Mod_id].physicalConfigDedicated[UE_index],
					  eNB_rrc_inst[Mod_id].mac_MainConfig[UE_index],
					  DRB2LCHAN[i],
					  (LogicalChannelConfig_t *)NULL,
					  (MeasGapConfig_t *)NULL,
					  (TDD_Config_t *)NULL,
					  (u8 *)NULL,
					  (u16 *)NULL);
      }
    }
  }
}

void rrc_eNB_generate_RRCConnectionSetup(u8 Mod_id,u32 frame, u16 UE_index) {

  LogicalChannelConfig_t *SRB1_logicalChannelConfig;//,*SRB2_logicalChannelConfig;

  LOG_I(RRC,"[eNB %d][RAPROC] Frame %d : Generating RRCConnectionSetup for UE %d \n",Mod_id,frame,UE_index);

  eNB_rrc_inst[Mod_id].Srb0.Tx_buffer.payload_size =
    do_RRCConnectionSetup((u8 *)eNB_rrc_inst[Mod_id].Srb0.Tx_buffer.Payload,
			  mac_xface->get_transmission_mode(Mod_id,find_UE_RNTI(Mod_id,UE_index)),
			  UE_index,0,
			  &eNB_rrc_inst[Mod_id].SRB1_config[UE_index],
			  &eNB_rrc_inst[Mod_id].SRB2_config[UE_index],
			  &eNB_rrc_inst[Mod_id].physicalConfigDedicated[UE_index]);

    // configure SRB1/SRB2, PhysicalConfigDedicated, MAC_MainConfig for UE

  if (eNB_rrc_inst[Mod_id].SRB1_config[UE_index]->logicalChannelConfig) {
    if (eNB_rrc_inst[Mod_id].SRB1_config[UE_index]->logicalChannelConfig->present == SRB_ToAddMod__logicalChannelConfig_PR_explicitValue) {
      SRB1_logicalChannelConfig = &eNB_rrc_inst[Mod_id].SRB1_config[UE_index]->logicalChannelConfig->choice.explicitValue;
    }
    else {
      SRB1_logicalChannelConfig = &SRB1_logicalChannelConfig_defaultValue;
    }
  }
  else {
    SRB1_logicalChannelConfig = &SRB1_logicalChannelConfig_defaultValue;
  }

  /*
  if (eNB_rrc_inst[Mod_id].SRB2_config[UE_index]->logicalChannelConfig) {
    if (eNB_rrc_inst[Mod_id].SRB2_config[UE_index]->logicalChannelConfig->present == SRB_ToAddMod__logicalChannelConfig_PR_explicitValue) {
      SRB2_logicalChannelConfig = &eNB_rrc_inst[Mod_id].SRB2_config[UE_index]->logicalChannelConfig->choice.explicitValue;
    }
    else {
      SRB2_logicalChannelConfig = &SRB2_logicalChannelConfig_defaultValue;
    }
  }
  else {
    SRB2_logicalChannelConfig  = &SRB2_logicalChannelConfig_defaultValue;
  }
  */

  LOG_D(RRC, "[MSC_MSG][FRAME %05d][RRC_eNB][MOD %02d][][--- MAC_CONFIG_REQ  (SRB1 UE %d) --->][MAC_eNB][MOD %02d][]\n",
            frame, Mod_id, UE_index, Mod_id);
  Mac_rlc_xface->rrc_mac_config_req(Mod_id,1,UE_index,0,
				    (RadioResourceConfigCommonSIB_t *)NULL,
				    eNB_rrc_inst[Mod_id].physicalConfigDedicated[UE_index],
				    eNB_rrc_inst[Mod_id].mac_MainConfig[UE_index],
				    1,
				    SRB1_logicalChannelConfig,
				    eNB_rrc_inst[Mod_id].measGapConfig[UE_index],
				    (TDD_Config_t *)NULL,
				    (u8 *)NULL,
				    (u16 *)NULL);

  LOG_I(RRC,"[eNB %d][RAPROC] Generate %d bytes (RRCConnectionSetup for UE %d) for CCCH \n",Mod_id,eNB_rrc_inst[Mod_id].Srb0.Tx_buffer.payload_size,UE_index);




}


void ue_rrc_process_rrcConnectionReconfiguration(u8 Mod_id,u32 frame,
						 RRCConnectionReconfiguration_t *rrcConnectionReconfiguration,
						 u8 CH_index) {

  if (rrcConnectionReconfiguration->criticalExtensions.present == RRCConnectionReconfiguration__criticalExtensions_PR_c1)
    if (rrcConnectionReconfiguration->criticalExtensions.choice.c1.present == RRCConnectionReconfiguration__criticalExtensions__c1_PR_rrcConnectionReconfiguration_r8) {

      if (rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated) {
	rrc_ue_process_radioResourceConfigDedicated(Mod_id,frame,CH_index,
						    rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated);


      }

      // check other fields for
    }
}

#ifndef USER_MODE
EXPORT_SYMBOL(Rlc_info_am_config);
#endif
