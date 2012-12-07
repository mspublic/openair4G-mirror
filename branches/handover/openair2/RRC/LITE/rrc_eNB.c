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
#include "HandoverCommand.h"
#include "HandoverCommand-r8-IEs.h"
#include "TDD-Config.h"
#include "rlc.h"
#include "SIMULATION/ETH_TRANSPORT/extern.h"
#ifdef Rel10
#include "MeasResults.h"
#endif
#ifdef USER_MODE
#include "RRC/NAS/nas_config.h"
#include "RRC/NAS/rb_config.h"
#include "OCG.h"
#include "OCG_extern.h"
#endif
#ifdef PHY_EMUL
extern EMULATION_VARS *Emul_vars;
#endif
extern eNB_MAC_INST *eNB_mac_inst;
extern UE_MAC_INST *UE_mac_inst;
#ifdef BIGPHYSAREA
extern void *bigphys_malloc(int);
#endif
extern uint16_t two_tier_hexagonal_cellIds[7];
extern int transmission_mode_rrc;

extern inline unsigned int taus(void);

mui_t rrc_eNB_mui=0;

void init_SI(u8 Mod_id) {

  u8 SIwindowsize=1;
  u16 SIperiod=8;

  eNB_rrc_inst[Mod_id].sizeof_SIB1 = 0;
  eNB_rrc_inst[Mod_id].sizeof_SIB23 = 0;

  eNB_rrc_inst[Mod_id].SIB1 = (u8 *)malloc16(32);

  if (eNB_rrc_inst[Mod_id].SIB1)
    eNB_rrc_inst[Mod_id].sizeof_SIB1 = do_SIB1(mac_xface->lte_frame_parms,
					       (uint8_t *)eNB_rrc_inst[Mod_id].SIB1,
					       &eNB_rrc_inst[Mod_id].siblock1,
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
						 &eNB_rrc_inst[Mod_id].sib3
#ifdef Rel10 
						 ,
						 &eNB_rrc_inst[Mod_id].sib13,
						 eNB_rrc_inst[Mod_id].MBMS_flag
#endif
						 );
    /*
      eNB_rrc_inst[Mod_id].sizeof_SIB23 = do_SIB2_AT4(Mod_id,
      eNB_rrc_inst[Mod_id].SIB23,
      &eNB_rrc_inst[Mod_id].systemInformation,
      &eNB_rrc_inst[Mod_id].sib2);
    */
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
    rrc_mac_config_req(Mod_id,1,0,0,
		       (RadioResourceConfigCommonSIB_t *)&eNB_rrc_inst[Mod_id].sib2->radioResourceConfigCommon,
		       (struct PhysicalConfigDedicated *)NULL,
		       (MeasObjectToAddMod_t **)NULL,
		       (MAC_MainConfig_t *)NULL,
		       0,
		       (struct LogicalChannelConfig *)NULL,
		       (MeasGapConfig_t *)NULL,
		       eNB_rrc_inst[Mod_id].sib1->tdd_Config,
		       (MobilityControlInfo_t *)NULL,
		       &SIwindowsize,
		       &SIperiod);
  }
  else {
    LOG_E(RRC,"[eNB] init_SI: FATAL, no memory for SIB2/3 allocated\n");
    mac_xface->macphy_exit("");
  }
}

/*------------------------------------------------------------------------------*/
char openair_rrc_lite_eNB_init(u8 Mod_id){
  /*-----------------------------------------------------------------------------*/

  unsigned char j;
  LOG_I(RRC,"[eNB %d] Init (UE State = RRC_IDLE)...\n",Mod_id);
  LOG_D(RRC, "[MSC_NEW][FRAME 00000][RRC_eNB][MOD %02d][]\n", Mod_id);

  for (j=0; j<NUMBER_OF_UE_MAX; j++)
    eNB_rrc_inst[Mod_id].Info.Status[j] = CH_READY;

  eNB_rrc_inst[Mod_id].Info.Nb_ue=0;

  eNB_rrc_inst[Mod_id].Srb0.Active=0;

  for(j=0;j<(NUMBER_OF_UE_MAX+1);j++){
    eNB_rrc_inst[Mod_id].Srb2[j].Active=0;
  }

#ifdef Rel10
  // This has to come from some top-level configuration
  eNB_rrc_inst[Mod_id].MBMS_flag = 0;
#endif
  /// System Information INIT
  init_SI(Mod_id);

#ifdef NO_RRM //init ch SRB0, SRB1 & BDTCH
  openair_rrc_on(Mod_id,1);
#else
  eNB_rrc_inst[Mod_id].Last_scan_req=0;
  send_msg(&S_rrc,msg_rrc_phy_synch_to_MR_ind(Mod_id,eNB_rrc_inst[Mod_id].Mac_id));
#endif

  return 0;

}


u8 get_next_UE_index(u8 Mod_id,u8 *UE_identity) {

  u8 i,first_index = 255,reg=0;

  for (i=0;i<NUMBER_OF_UE_MAX;i++) {


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

u8 rrc_find_free_ue_index(u8 Mod_id){
  //-------------------------------------------------------------------------------------------//
  u16 i;
  for(i=1;i<=NUMBER_OF_UE_MAX;i++)
    if ( (eNB_rrc_inst[Mod_id].Info.UE_list[i][0] == 0) &&
	 (eNB_rrc_inst[Mod_id].Info.UE_list[i][1] == 0) &&
	 (eNB_rrc_inst[Mod_id].Info.UE_list[i][2] == 0) &&
	 (eNB_rrc_inst[Mod_id].Info.UE_list[i][3] == 0) &&
	 (eNB_rrc_inst[Mod_id].Info.UE_list[i][4] == 0))
      return i;
  return 0xff;
}

/*------------------------------------------------------------------------------*/
int rrc_eNB_decode_dcch(u8 Mod_id, u32 frame, u8 Srb_id, u8 UE_index, u8 *Rx_sdu, u8 sdu_size) {
  /*------------------------------------------------------------------------------*/

  asn_dec_rval_t dec_rval;
  UL_DCCH_Message_t uldcchmsg;
  UL_DCCH_Message_t *ul_dcch_msg=&uldcchmsg;
  u8 buffer[120];
  int i;

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
			 120,0,0);
  for (i=0;i<sdu_size;i++)
    msg("%x.",Rx_sdu[i]);
  msg("\n");

  if ((dec_rval.code != RC_OK) && (dec_rval.consumed==0)) {
    LOG_E(RRC,"[UE %d] Frame %d : Failed to decode UL-DCCH (%d bytes)\n",Mod_id,frame,dec_rval.consumed);
    return -1;
  }

  //#ifdef X2_SIM
  //  for (i=0;i<NUMBER_OF_UE_MAX && eNB_rrc_inst[Mod_id].handover_info[i] != NULL;i++) {

  /*
    if(eNB_rrc_inst[Mod_id].handover_info[i]->ho_prepare == 0xFF) {
    LOG_D(RRC,"\n Incoming HO detected for new UE_idx %d eNB_mod_id: %d \n",i,Mod_id);
    rrc_eNB_process_handoverPreparationInformation(Mod_id,frame,i);
    }
  */

  /*
    if(eNB_rrc_inst[Mod_id].handover_info[i]->ho_complete == 0xFF) {
    LOG_D(RRC,"\n HO Command received for new UE_idx %d \n");
    //rrc_eNB_process_handoverPreparationInformation(Mod_id,frame,i);

    rrc_rlc_data_req(Mod_id,frame, 1,(i*MAX_NUM_RB)+DCCH,rrc_eNB_mui++,0,eNB_rrc_inst[Mod_id].handover_info[i]->size,(char*)eNB_rrc_inst[Mod_id].handover_info[i]->buf);

    pdcp_data_req(Mod_id,frame, 1,(i*MAX_NUM_RB)+DCCH,rrc_eNB_mui++,0,eNB_rrc_inst[Mod_id].handover_info[i]->size,(char*)eNB_rrc_inst[Mod_id].handover_info[i]->buf,1);
    }
  */
  //  }
  //#endif

  if (ul_dcch_msg->message.present == UL_DCCH_MessageType_PR_c1) {

    switch (ul_dcch_msg->message.choice.c1.present) {

    case UL_DCCH_MessageType__c1_PR_NOTHING:     /* No components present */
      break;
    case UL_DCCH_MessageType__c1_PR_csfbParametersRequestCDMA2000:
      break;
    case UL_DCCH_MessageType__c1_PR_measurementReport:
      LOG_D(RRC, "[MSC_MSG][FRAME %05d][RLC][MOD %02d][RB %02d][--- RLC_DATA_IND %d bytes (measurementReport) --->][RRC_eNB][MOD %02d][]\n",
	    frame, Mod_id, DCCH, sdu_size, Mod_id);
      rrc_eNB_process_MeasurementReport(Mod_id,UE_index,&ul_dcch_msg->message.choice.c1.choice.measurementReport.criticalExtensions.choice.c1.choice.measurementReport_r8.measResults);
      break;
    case UL_DCCH_MessageType__c1_PR_rrcConnectionReconfigurationComplete:
      LOG_D(RRC, "[MSC_MSG][FRAME %05d][RLC][MOD %02d][RB %02d][--- RLC_DATA_IND %d bytes (RRCConnectionReconfigurationComplete) --->][RRC_eNB][MOD %02d][]\n",
	    frame, Mod_id, DCCH, sdu_size, Mod_id);
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
#ifdef Rel10
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
#endif
    default:
      LOG_E(RRC,"[UE %d] Frame %d : Unknown message\n",Mod_id,frame);
      return -1;
    }
    return 0;
  }
  else {
    LOG_E(RRC,"[UE %d] Frame %d : Unknown error\n",Mod_id,frame);
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
  int i;


  memset(ul_ccch_msg,0,sizeof(UL_CCCH_Message_t));

  LOG_D(RRC,"[eNB %d] Frame %d: Decoding UL CCCH %x.%x.%x.%x.%x.%x (%p)\n", Mod_id,frame,
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
  for (i=0;i<8;i++)
    msg("%x.",((u8*)&ul_ccch_msg)[i]);
  if (dec_rval.consumed == 0) {
    LOG_E(RRC,"[eNB %d] FATAL Error in receiving CCCH\n", Mod_id);
    return -1; //mac_xface->macphy_exit(""); //exit(-1);
  }
  if (ul_ccch_msg->message.present == UL_CCCH_MessageType_PR_c1) {

    switch (ul_ccch_msg->message.choice.c1.present) {

    case UL_CCCH_MessageType__c1_PR_NOTHING :
      LOG_I(RRC,"[eNB %d] Frame %d : Received PR_NOTHING on UL-CCCH-Message\n",Mod_id,frame);
      break;

    case UL_CCCH_MessageType__c1_PR_rrcConnectionReestablishmentRequest :
      LOG_D(RRC, "[MSC_MSG][FRAME %05d][MAC_eNB][MOD %02d][][--- MAC_DATA_IND (rrcConnectionReestablishmentRequest on SRB0) -->][RRC_eNB][MOD %02d][]\n",
            frame, Mod_id, Mod_id);
      LOG_I(RRC,"[eNB %d] Frame %d : RRCConnectionReestablishmentRequest not supported yet\n",Mod_id,frame);
      break;

    case UL_CCCH_MessageType__c1_PR_rrcConnectionRequest :
      LOG_D(RRC, "[MSC_MSG][FRAME %05d][MAC_eNB][MOD %02d][][--- MAC_DATA_IND  (rrcConnectionRequest on SRB0) -->][RRC_eNB][MOD %02d][]\n",
            frame, Mod_id, Mod_id);

      rrcConnectionRequest = &ul_ccch_msg->message.choice.c1.choice.rrcConnectionRequest.criticalExtensions.choice.rrcConnectionRequest_r8;
      UE_index = get_next_UE_index(Mod_id,(u8 *)rrcConnectionRequest->ue_Identity.choice.randomValue.buf);

      if (UE_index!=255) {

	//	memcpy(&Rrc_xface->UE_id[Mod_id][UE_index],(u8 *)rrcConnectionRequest->ue_Identity.choice.randomValue.buf,5);
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
	rrc_pdcp_config_req (Mod_id, frame, 1, ACTION_ADD, Idx);
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
      }
      break;

    default:
      LOG_E(RRC,"[eNB %d] Frame %d : Unknown message\n",Mod_id,frame);
      return -1;
    }
    return 0;
  }
  else{
    LOG_E(RRC,"[eNB %d] Frame %d : Unknown error \n",Mod_id,frame);
    return -1;
  }

}



void rrc_eNB_process_RRCConnectionSetupComplete(u8 Mod_id, u32 frame, u8 UE_index,RRCConnectionSetupComplete_r8_IEs_t *rrcConnectionSetupComplete) {


  LOG_I(RRC,"[eNB %d][RAPROC] Frame %d : Logical Channel UL-DCCH, processing RRCConnectionSetupComplete from UE %d\n",Mod_id,frame,UE_index);

  rrc_eNB_generate_RRCConnectionReconfiguration(Mod_id,frame,UE_index);

}

void rrc_eNB_generate_RRCConnectionReconfiguration(u8 Mod_id,u32 frame,u16 UE_index) {

  u8 buffer[100];
  u8 size;
  int i;

  // configure SRB1/SRB2, PhysicalConfigDedicated, MAC_MainConfig for UE
  eNB_RRC_INST *rrc_inst = &eNB_rrc_inst[Mod_id];

  struct SRB_ToAddMod **SRB2_config                         = &rrc_inst->SRB2_config[UE_index];
  struct DRB_ToAddMod **DRB_config                          = &rrc_inst->DRB_config[UE_index][0];
  struct PhysicalConfigDedicated  **physicalConfigDedicated = &rrc_inst->physicalConfigDedicated[UE_index]; 


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
  MAC_MainConfig_t *mac_MainConfig;
  MeasObjectToAddModList_t *MeasObj_list;
  MeasObjectToAddMod_t *MeasObj;
  ReportConfigToAddModList_t *ReportConfig_list;
  ReportConfigToAddMod_t *ReportConfig_per,*ReportConfig_A1,*ReportConfig_A2,*ReportConfig_A3,*ReportConfig_A4,*ReportConfig_A5;
  MeasIdToAddModList_t *MeasId_list;
  MeasIdToAddMod_t *MeasId0,*MeasId1,*MeasId2,*MeasId3,*MeasId4,*MeasId5;
  QuantityConfig_t *quantityConfig;
#if Rel10
  long * sr_ProhibitTimer_r9;
  struct PUSCH_CAConfigDedicated_vlola  *pusch_CAConfigDedicated_vlola;
#endif

  long *logicalchannelgroup,*logicalchannelgroup_drb;
  long *maxHARQ_Tx, *periodicBSR_Timer;

  long *lcid;

  struct MeasConfig__speedStatePars *Sparams;
  CellsToAddMod_t *CellToAdd;
  CellsToAddModList_t *CellsToAddModList;

  // 
  // Configure SRB2

  SRB_list = CALLOC(1,sizeof(*SRB_list));

  /// SRB2
  SRB2_config2 = CALLOC(1,sizeof(*SRB2_config2));
  *SRB2_config = SRB2_config2;

  SRB2_config2->srb_Identity = 2;
  SRB2_rlc_config = CALLOC(1,sizeof(*SRB2_rlc_config));
  SRB2_config2->rlc_Config   = SRB2_rlc_config;

  SRB2_rlc_config->present = SRB_ToAddMod__rlc_Config_PR_explicitValue;
  SRB2_rlc_config->choice.explicitValue.present=RLC_Config_PR_am;
  SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.t_PollRetransmit = T_PollRetransmit_ms45;
  SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.pollPDU          = PollPDU_pInfinity;
  SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.pollByte         = PollPDU_pInfinity;
  SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.maxRetxThreshold = UL_AM_RLC__maxRetxThreshold_t4;
  SRB2_rlc_config->choice.explicitValue.choice.am.dl_AM_RLC.t_Reordering     = T_Reordering_ms35;
  SRB2_rlc_config->choice.explicitValue.choice.am.dl_AM_RLC.t_StatusProhibit = T_StatusProhibit_ms0;


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
  DRB_rlc_config->choice.um_Bi_Directional.ul_UM_RLC.sn_FieldLength=SN_FieldLength_size5;
  DRB_rlc_config->choice.um_Bi_Directional.dl_UM_RLC.sn_FieldLength=SN_FieldLength_size5;
  DRB_rlc_config->choice.um_Bi_Directional.dl_UM_RLC.t_Reordering=T_Reordering_ms35;
  DRB_lchan_config = CALLOC(1,sizeof(*DRB_lchan_config));
  DRB_config2->logicalChannelConfig   = DRB_lchan_config;
  DRB_ul_SpecificParameters = CALLOC(1,sizeof(*DRB_ul_SpecificParameters));
  DRB_lchan_config->ul_SpecificParameters = DRB_ul_SpecificParameters;


  DRB_ul_SpecificParameters->priority = 2; // lower priority than srb1, srb2
  DRB_ul_SpecificParameters->prioritisedBitRate=LogicalChannelConfig__ul_SpecificParameters__prioritisedBitRate_infinity;
  DRB_ul_SpecificParameters->bucketSizeDuration=LogicalChannelConfig__ul_SpecificParameters__bucketSizeDuration_ms50;

  logicalchannelgroup_drb = CALLOC(1,sizeof(long));
  *logicalchannelgroup_drb=0;
  DRB_ul_SpecificParameters->logicalChannelGroup = logicalchannelgroup_drb;


  ASN_SEQUENCE_ADD(&DRB_list->list,DRB_config2);

  mac_MainConfig = CALLOC(1,sizeof(*mac_MainConfig));
  eNB_rrc_inst[Mod_id].mac_MainConfig[UE_index] = mac_MainConfig;

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

#ifdef Rel10
  sr_ProhibitTimer_r9 = CALLOC(1,sizeof(long));
  *sr_ProhibitTimer_r9=0; // SR tx on PUCCH, Value in number of SR period(s). Value 0 = no timer for SR, Value 2= 2*SR 
  mac_MainConfig->sr_ProhibitTimer_r9=sr_ProhibitTimer_r9;
  sps_RA_ConfigList_rlola = NULL;
#endif


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

  //  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig->measIdToAddModList = MeasId_list;  

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
  //  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig->measObjectToAddModList = MeasObj_list;  

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
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA3.a3_Offset = 0.5;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA3.reportOnLeave = 1;

  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.triggerQuantity                       = ReportConfigEUTRA__triggerQuantity_rsrp;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.reportQuantity                        = ReportConfigEUTRA__reportQuantity_both;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.maxReportCells                        = 2;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.reportInterval                        = ReportInterval_ms120;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.reportAmount                          = ReportConfigEUTRA__reportAmount_infinity;

  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.hysteresis = 0.5;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.timeToTrigger = TimeToTrigger_ms40;

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
  //  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig->reportConfigToAddModList = ReportConfig_list;

  Sparams = CALLOC(1,sizeof(*Sparams));
  Sparams->present=MeasConfig__speedStatePars_PR_setup;
  Sparams->choice.setup.timeToTrigger_SF.sf_High=SpeedStateScaleFactors__sf_Medium_oDot75;
  Sparams->choice.setup.timeToTrigger_SF.sf_Medium=SpeedStateScaleFactors__sf_High_oDot5;
  Sparams->choice.setup.mobilityStateParameters.n_CellChangeHigh=10;
  Sparams->choice.setup.mobilityStateParameters.n_CellChangeMedium=5;
  Sparams->choice.setup.mobilityStateParameters.t_Evaluation=MobilityStateParameters__t_Evaluation_s60;
  Sparams->choice.setup.mobilityStateParameters.t_HystNormal=MobilityStateParameters__t_HystNormal_s120;

  quantityConfig = CALLOC(1,sizeof(*quantityConfig));
  memset((void *)quantityConfig,0,sizeof(*quantityConfig));
  quantityConfig->quantityConfigEUTRA = CALLOC(1,sizeof(struct QuantityConfigEUTRA));
  memset((void *)quantityConfig->quantityConfigEUTRA,0,sizeof(*quantityConfig->quantityConfigEUTRA));
  quantityConfig->quantityConfigCDMA2000 = NULL;
  quantityConfig->quantityConfigGERAN = NULL;
  quantityConfig->quantityConfigUTRA = NULL;
  quantityConfig->quantityConfigEUTRA->filterCoefficientRSRP = CALLOC(1,sizeof(*(quantityConfig->quantityConfigEUTRA->filterCoefficientRSRP)));
  quantityConfig->quantityConfigEUTRA->filterCoefficientRSRQ = CALLOC(1,sizeof(*(quantityConfig->quantityConfigEUTRA->filterCoefficientRSRQ)));
  *quantityConfig->quantityConfigEUTRA->filterCoefficientRSRP = FilterCoefficient_fc4;
  *quantityConfig->quantityConfigEUTRA->filterCoefficientRSRQ = FilterCoefficient_fc4;

  //rrc_inst->handover_info.as_config.sourceRadioResourceConfig.srb_ToAddModList = CALLOC(1,sizeof());
  rrc_inst->handover_info[UE_index] = CALLOC(1,sizeof(*(rrc_inst->handover_info[UE_index])));
  //memcpy((void *)rrc_inst->handover_info[UE_index]->as_config.sourceRadioResourceConfig.srb_ToAddModList,(void *)SRB_list,sizeof(SRB_ToAddModList_t));
  rrc_inst->handover_info[UE_index]->as_config.sourceRadioResourceConfig.srb_ToAddModList = SRB_list;
  //memcpy((void *)rrc_inst->handover_info[UE_index]->as_config.sourceRadioResourceConfig.drb_ToAddModList,(void *)DRB_list,sizeof(DRB_ToAddModList_t));
  rrc_inst->handover_info[UE_index]->as_config.sourceRadioResourceConfig.drb_ToAddModList = DRB_list;
  rrc_inst->handover_info[UE_index]->as_config.sourceRadioResourceConfig.drb_ToReleaseList = NULL;
  rrc_inst->handover_info[UE_index]->as_config.sourceRadioResourceConfig.mac_MainConfig = CALLOC(1, sizeof(*rrc_inst->handover_info[UE_index]->as_config.sourceRadioResourceConfig.mac_MainConfig));
  memcpy((void *)rrc_inst->handover_info[UE_index]->as_config.sourceRadioResourceConfig.mac_MainConfig,(void *)mac_MainConfig,sizeof(MAC_MainConfig_t));
  rrc_inst->handover_info[UE_index]->as_config.sourceRadioResourceConfig.physicalConfigDedicated = CALLOC(1,sizeof(PhysicalConfigDedicated_t));
  memcpy((void *)rrc_inst->handover_info[UE_index]->as_config.sourceRadioResourceConfig.physicalConfigDedicated,(void *)rrc_inst->physicalConfigDedicated[UE_index],sizeof(PhysicalConfigDedicated_t));
  rrc_inst->handover_info[UE_index]->as_config.sourceRadioResourceConfig.sps_Config = NULL;
  //memcpy((void *)rrc_inst->handover_info[UE_index]->as_config.sourceRadioResourceConfig.sps_Config,(void *)rrc_inst->sps_Config[UE_index],sizeof(SPS_Config_t));

  /*
   * MeasConfig_t	 sourceMeasConfig;
   RadioResourceConfigDedicated_t	 sourceRadioResourceConfig;
   SecurityAlgorithmConfig_t	 sourceSecurityAlgorithmConfig;
   C_RNTI_t	 sourceUE_Identity;
   MasterInformationBlock_t	 sourceMasterInformationBlock;
   SystemInformationBlockType1_t	 sourceSystemInformationBlockType1;
   SystemInformationBlockType2_t	 sourceSystemInformationBlockType2;
   AntennaInfoCommon_t	 antennaInfoCommon;
   ARFCN_ValueEUTRA_t	 sourceDl_CarrierFreq;
  */


  size = do_RRCConnectionReconfiguration(Mod_id,
					 buffer,
					 UE_index,
					 0,//Transaction_id,
					 SRB_list,
					 DRB_list,
					 NULL, // DRB2_list,
					 NULL, //*sps_Config,
					 physicalConfigDedicated,
					 MeasObj_list,
					 ReportConfig_list,
					 quantityConfig, //*QuantityConfig,
					 MeasId_list,
					 mac_MainConfig,
					 NULL, //*measGapConfig
					 (MobilityControlInfo_t *)NULL);  // *mobilityInfo;

  LOG_I(RRC,"[eNB %d] Frame %d, Logical Channel DL-DCCH, Generate RRCConnectionReconfiguration (bytes %d, UE id %d)\n",
	Mod_id,frame, size, UE_index);


  LOG_D(RLC, "[MSC_MSG][FRAME %05d][RRC_eNB][MOD %02d][][--- RLC_DATA_REQ/%d Bytes (rrcConnectionReconfiguration to UE %d MUI %d) --->][RLC][MOD %02d][RB %02d]\n",
	frame, Mod_id, size, UE_index, rrc_eNB_mui, Mod_id, (UE_index*MAX_NUM_RB)+DCCH);
  //rrc_rlc_data_req(Mod_id,frame, 1,(UE_index*MAX_NUM_RB)+DCCH,rrc_eNB_mui++,0,size,(char*)buffer);
  pdcp_data_req(Mod_id,frame, 1,(UE_index*MAX_NUM_RB)+DCCH,rrc_eNB_mui++,0,size,(char*)buffer,1);

}

//get_adjacent_cell_mod_id

void rrc_eNB_process_MeasurementReport(u8 Mod_id,u16 UE_index,MeasResults_t	 *measResults2) {


  LOG_I(RRC,"Received Measurement Report From UE %d (Measurement Id %d)\n",UE_index,(int)measResults2->measId);
  if (measResults2->measResultNeighCells->choice.measResultListEUTRA.list.count>0) {
    LOG_I(RRC,"Physical Cell Id %d\n",(int)measResults2->measResultNeighCells->choice.measResultListEUTRA.list.array[0]->physCellId);
    LOG_I(RRC,"RSRP of Target %d\n",(int)*(measResults2->measResultNeighCells->choice.measResultListEUTRA.list.array[0]->measResult.rsrpResult));
    LOG_I(RRC,"RSRQ of Target %d\n",(int)*(measResults2->measResultNeighCells->choice.measResultListEUTRA.list.array[0]->measResult.rsrqResult));
  }
#ifdef Rel10
  LOG_I(RRC,"RSRP of Source %d\n",measResults2->measResultPCell.rsrpResult);
  LOG_I(RRC,"RSRQ of Source %d\n",measResults2->measResultPCell.rsrqResult);
#else  
  LOG_I(RRC,"RSRP of Source %d\n",measResults2->measResultServCell.rsrpResult);
  LOG_I(RRC,"RSRQ of Source %d\n",measResults2->measResultServCell.rsrqResult);
#endif   
  
  //void fill_handover_info(u8 Mod_id, u8 UE_index, PhysCellId_t targetPhyId, eNB_RRC_INST *rrc_inst, HANDOVER_INFO *handover_info)
  //tart
  // if(eNB_rrc_inst[Mod_id]->handover_info[UE_index]) {
  //
  // }
  if(eNB_rrc_inst[Mod_id].handover_info[UE_index]->ho_prepare != 0xF0)
	  rrc_eNB_generate_HandoverPreparationInformation(Mod_id,UE_index,measResults2->measResultNeighCells->choice.measResultListEUTRA.list.array[0]->physCellId,&eNB_rrc_inst[Mod_id],&eNB_rrc_inst[Mod_id].handover_info[Mod_id]);
  else
	  LOG_D(RRC,"\neNB %d: Ignoring MeasReport from UE %d as Handover is in progress... \n",Mod_id,UE_index);
  //Look for IP address of the target eNB
  //Send Handover Request -> target eNB
  //Wait for Handover Acknowledgement <- target eNB
  //Send Handover Command
  
  //x2delay();
  //	handover_request_x2(UE_index,Mod_id,measResults2->measResultNeighCells->choice.measResultListEUTRA.list.array[0]->physCellId);
  
  //	u8 buffer[100];
  //    int size=rrc_eNB_generate_Handover_Command_TeNB(0,0,buffer);
  //
  //	  send_check_message((char*)buffer,size);
  //send_handover_command();

}



void rrc_eNB_generate_HandoverPreparationInformation (u8 Mod_id, u8 UE_index, PhysCellId_t targetPhyId, eNB_RRC_INST *rrc_inst, HANDOVER_INFO *handover_info) {
  u8 buffer[100];
  u8 size,UE_idx;
  u8 modid_target = get_adjacent_cell_mod_id(targetPhyId);

  HANDOVER_INFO *handoverInfo = CALLOC(1,sizeof(*handoverInfo));
  struct PhysicalConfigDedicated  **physicalConfigDedicated = &rrc_inst->physicalConfigDedicated[UE_index];
  RadioResourceConfigDedicated_t *radioResourceConfigDedicated = CALLOC(1,sizeof(RadioResourceConfigDedicated_t));

  handoverInfo->as_config.antennaInfoCommon.antennaPortsCount =  0; //Not used 0- but check value
  handoverInfo->as_config.sourceDl_CarrierFreq = 36090; //Verify!
  memcpy((void*) &handoverInfo->as_config.sourceMasterInformationBlock, (void*)&rrc_inst->mib,sizeof(MasterInformationBlock_t));
  memcpy((void*) &handoverInfo->as_config.sourceMeasConfig, (void*)&rrc_inst->measConfig[UE_index],sizeof(MeasConfig_t));
  //to be configured
  memset((void *)&rrc_inst->handover_info[UE_index]->as_config.sourceSecurityAlgorithmConfig,0,sizeof(SecurityAlgorithmConfig_t));

  memcpy((void *)&rrc_inst->handover_info[UE_index]->as_config.sourceSystemInformationBlockType1,(void *)&rrc_inst->SIB1, sizeof(SystemInformationBlockType1_t));
  memcpy((void *)&rrc_inst->handover_info[UE_index]->as_config.sourceSystemInformationBlockType2,(void *)&rrc_inst->SIB23, sizeof(SystemInformationBlockType2_t));

  rrc_inst->handover_info[UE_index]->as_context.reestablishmentInfo = CALLOC(1,sizeof(*rrc_inst->handover_info[UE_index]->as_context.reestablishmentInfo));
  rrc_inst->handover_info[UE_index]->as_context.reestablishmentInfo->sourcePhysCellId = rrc_inst->physCellId;
  rrc_inst->handover_info[UE_index]->as_context.reestablishmentInfo->targetCellShortMAC_I.buf = NULL; // Check values later
  rrc_inst->handover_info[UE_index]->as_context.reestablishmentInfo->targetCellShortMAC_I.size = 0;
  rrc_inst->handover_info[UE_index]->as_context.reestablishmentInfo->targetCellShortMAC_I.bits_unused = 0;
  rrc_inst->handover_info[UE_index]->as_context.reestablishmentInfo->additionalReestabInfoList = NULL;

  rrc_inst->handover_info[UE_index]->ho_prepare = 0xF0;
  rrc_inst->handover_info[UE_index]->ho_complete = 0;

  if (modid_target != 0xFF) {
    //UE_idx = rrc_find_free_ue_index(modid_target);
	UE_idx = get_next_UE_index(modid_target,(u8 *)eNB_rrc_inst[Mod_id].Info.UE_list[UE_index]); //this should return a new index

    if (UE_idx!=0xFF) {
      LOG_D(RRC,"\n Sending HandoverPreparationInformation msg from eNB %d to eNB %d source UE_idx %d target UE_idx %d source_modId: %d target_modId: %d\n", rrc_inst->physCellId,targetPhyId,UE_index,UE_idx,Mod_id,modid_target);
      eNB_rrc_inst[modid_target].handover_info[UE_idx] = CALLOC(1,sizeof(*(eNB_rrc_inst[modid_target].handover_info[UE_idx])));
      memcpy((void *)&eNB_rrc_inst[modid_target].handover_info[UE_idx]->as_context, (void *)&rrc_inst->handover_info[UE_index]->as_context, sizeof(AS_Context_t));
      memcpy((void *)&eNB_rrc_inst[modid_target].handover_info[UE_idx]->as_config, (void *)&rrc_inst->handover_info[UE_index]->as_config, sizeof(AS_Config_t));
      eNB_rrc_inst[modid_target].handover_info[UE_idx]->ho_prepare = 0xFF;
      eNB_rrc_inst[modid_target].handover_info[UE_idx]->ho_complete = 0;

      rrc_inst->handover_info[UE_index]->modid_t = modid_target;
      rrc_inst->handover_info[UE_index]->ueid_s = UE_index;
      rrc_inst->handover_info[UE_index]->modid_s = Mod_id;
      eNB_rrc_inst[modid_target].handover_info[UE_idx]->modid_t = modid_target;
      eNB_rrc_inst[modid_target].handover_info[UE_idx]->modid_s = Mod_id;
      eNB_rrc_inst[modid_target].handover_info[UE_idx]->ueid_t = UE_idx;
    }
    else
      LOG_E(RRC,"\nError in obtaining free UE id in target eNB %l for handover \n", targetPhyId);
  }
  else
    LOG_E(RRC,"\nError in obtaining Module ID of target eNB for handover \n");
}

void rrc_eNB_process_RRCConnectionReconfigurationComplete(u8 Mod_id,u32 frame,u8 UE_index,RRCConnectionReconfigurationComplete_r8_IEs_t *rrcConnectionReconfigurationComplete){
  int i;
  int oip_ifup=0;
  // Loop through DRBs and establish if necessary
  for (i=0;i<8;i++) { // num max DRB (11-3-8)
    if (eNB_rrc_inst[Mod_id].DRB_config[UE_index][i]) {
      LOG_I(RRC,"[eNB %d] Frame  %d : Logical Channel UL-DCCH, Received RRCConnectionReconfigurationComplete from UE %d, reconfiguring DRB %d/LCID %d\n",
	    Mod_id,frame, UE_index,
	    (int)eNB_rrc_inst[Mod_id].DRB_config[UE_index][0]->drb_Identity,
	    (UE_index * MAX_NUM_RB) + (int)*eNB_rrc_inst[Mod_id].DRB_config[UE_index][0]->logicalChannelIdentity);
      if (eNB_rrc_inst[Mod_id].DRB_active[UE_index][i] == 0) {
	rrc_pdcp_config_req (Mod_id, frame, 1, ACTION_ADD,  
			     (UE_index * MAX_NUM_RB) + (int)*eNB_rrc_inst[Mod_id].DRB_config[UE_index][i]->logicalChannelIdentity);
	rrc_rlc_config_req(Mod_id,frame,1,ACTION_ADD,
			   (UE_index * MAX_NUM_RB) + (int)*eNB_rrc_inst[Mod_id].DRB_config[UE_index][i]->logicalChannelIdentity,
			   RADIO_ACCESS_BEARER,Rlc_info_um);
	eNB_rrc_inst[Mod_id].DRB_active[UE_index][i] = 1;
	
	LOG_D(RRC,"[eNB %d] Frame %d: Establish RLC UM Bidirectional, DRB %d Active\n", 
	      Mod_id, frame, (int)eNB_rrc_inst[Mod_id].DRB_config[UE_index][0]->drb_Identity);
#ifdef NAS_NETLINK
	LOG_I(OIP,"[eNB %d] trying to bring up the OAI interface oai%d\n", Mod_id, Mod_id);
	oip_ifup = nas_config(Mod_id,// interface index
			      Mod_id+1, // thrid octet
			      Mod_id+1);// fourth octet

	if (oip_ifup == 0 ){ // interface is up --> send a config the DRB
	  oai_emulation.info.oai_ifup[Mod_id]=1;
	  LOG_I(OIP,"[eNB %d] Config the oai%d to send/receive pkt on DRB %d to/from the protocol stack\n",  
		Mod_id,
		Mod_id,
		(UE_index * MAX_NUM_RB) + *eNB_rrc_inst[Mod_id].DRB_config[UE_index][i]->logicalChannelIdentity);
	  rb_conf_ipv4(0,//add
		       UE_index, //cx
		       Mod_id,//inst
		       (UE_index * MAX_NUM_RB) + *eNB_rrc_inst[Mod_id].DRB_config[UE_index][i]->logicalChannelIdentity,
		       0,//dscp
		       ipv4_address(Mod_id+1,Mod_id+1),//saddr
		       ipv4_address(Mod_id+1,NB_eNB_INST+UE_index+1));//daddr
	   
	  LOG_D(RRC,"[eNB %d] State = Attached (UE %d)\n",Mod_id,UE_index);
	}
#endif
	LOG_D(RRC, "[MSC_MSG][FRAME %05d][RRC_eNB][MOD %02d][][--- MAC_CONFIG_REQ  (DRB UE %d) --->][MAC_eNB][MOD %02d][]\n",
	      frame, Mod_id, UE_index, Mod_id);
	DRB2LCHAN[i] = (u8)*eNB_rrc_inst[Mod_id].DRB_config[UE_index][i]->logicalChannelIdentity;
	rrc_mac_config_req(Mod_id,1,UE_index,0,
			   (RadioResourceConfigCommonSIB_t *)NULL,
			   eNB_rrc_inst[Mod_id].physicalConfigDedicated[UE_index],
			   (MeasObjectToAddMod_t **)NULL,
			   eNB_rrc_inst[Mod_id].mac_MainConfig[UE_index],
			   DRB2LCHAN[i],
			   eNB_rrc_inst[Mod_id].DRB_config[UE_index][i]->logicalChannelConfig,
			   eNB_rrc_inst[Mod_id].measGapConfig[UE_index],
			   (TDD_Config_t *)NULL,
			   (MobilityControlInfo_t *)NULL,
			   (u8 *)NULL,
			   (u16 *)NULL);
      }
      else { // remove LCHAN from MAC/PHY

	if (eNB_rrc_inst[Mod_id].DRB_active[UE_index][i] ==1) {
	  // DRB has just been removed so remove RLC + PDCP for DRB
	  rrc_pdcp_config_req (Mod_id, frame, 1, ACTION_REMOVE,  
			       (UE_index * MAX_NUM_RB) + DRB2LCHAN[i]);
	  rrc_rlc_config_req(Mod_id,frame,1,ACTION_REMOVE,
			     (UE_index * MAX_NUM_RB) + DRB2LCHAN[i],
			     RADIO_ACCESS_BEARER,Rlc_info_um);
	}
	eNB_rrc_inst[Mod_id].DRB_active[UE_index][i] = 0;
	LOG_D(RRC, "[MSC_MSG][FRAME %05d][RRC_eNB][MOD %02d][][--- MAC_CONFIG_REQ  (DRB UE %d) --->][MAC_eNB][MOD %02d][]\n",
	      frame, Mod_id, UE_index, Mod_id);

	rrc_mac_config_req(Mod_id,1,UE_index,0,
			   (RadioResourceConfigCommonSIB_t *)NULL,
			   eNB_rrc_inst[Mod_id].physicalConfigDedicated[UE_index],
			   (MeasObjectToAddMod_t **)NULL,
			   eNB_rrc_inst[Mod_id].mac_MainConfig[UE_index],
			   DRB2LCHAN[i],
			   (LogicalChannelConfig_t *)NULL,
			   (MeasGapConfig_t *)NULL,
			   (TDD_Config_t *)NULL,
			   (MobilityControlInfo_t *)NULL,
			   (u8 *)NULL,
			   (u16 *)NULL);
      }
    }
  }
}

void rrc_eNB_generate_RRCConnectionSetup(u8 Mod_id,u32 frame, u16 UE_index) {

  LogicalChannelConfig_t *SRB1_logicalChannelConfig;//,*SRB2_logicalChannelConfig;

  eNB_rrc_inst[Mod_id].Srb0.Tx_buffer.payload_size =
    do_RRCConnectionSetup((u8 *)eNB_rrc_inst[Mod_id].Srb0.Tx_buffer.Payload,
			  mac_xface->get_transmission_mode(Mod_id,find_UE_RNTI(Mod_id,UE_index)),
			  UE_index,0,
			  mac_xface->lte_frame_parms,
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
  rrc_mac_config_req(Mod_id,1,UE_index,0,
		     (RadioResourceConfigCommonSIB_t *)NULL,
		     eNB_rrc_inst[Mod_id].physicalConfigDedicated[UE_index],
		     (MeasObjectToAddMod_t **)NULL,
		     eNB_rrc_inst[Mod_id].mac_MainConfig[UE_index],
		     1,
		     SRB1_logicalChannelConfig,
		     eNB_rrc_inst[Mod_id].measGapConfig[UE_index],
		     (TDD_Config_t *)NULL,
		     (MobilityControlInfo_t *)NULL,
		     (u8 *)NULL,
		     (u16 *)NULL);

  LOG_I(RRC,"[eNB %d][RAPROC] Frame %d : Logical Channel DL-CCCH, Generating RRCConnectionSetup (bytes %d, UE %d)\n",
	Mod_id,frame,eNB_rrc_inst[Mod_id].Srb0.Tx_buffer.payload_size, UE_index);
  
}


void rrc_eNB_generate_RRCConnectionReconfiguration_handover(u8 Mod_id,u32 frame,u16 UE_index, LTE_DL_FRAME_PARMS *lte_frame_parms) {

  u8 buffer[RRC_BUF_SIZE];
  int size;
  int i;
  uint8_t rv[2];
  u16 Idx;
  struct SRB_ToAddMod__rlc_Config *SRB1_rlc_config;//,*SRB2_rlc_config;
  struct SRB_ToAddMod__logicalChannelConfig *SRB1_lchan_config;//,*SRB2_lchan_config;
  struct LogicalChannelConfig__ul_SpecificParameters *SRB1_ul_SpecificParameters;//,*SRB2_ul_SpecificParameters;
  LogicalChannelConfig_t *SRB1_logicalChannelConfig;//,*SRB2_logicalChannelConfig;
  PhysicalConfigDedicated_t *physicalConfigDedicated2;

  LOG_D(RRC,"\n HO newSourceUEIdentity (C-RNTI): ");
  for (i=0;i<2;i++) {
    rv[i]=taus()&0xff;
    LOG_D(RRC," %x.",rv[i]);
  }

  // configure SRB1/SRB2, PhysicalConfigDedicated, MAC_MainConfig for UE
  eNB_RRC_INST *rrc_inst = &eNB_rrc_inst[Mod_id];

  struct SRB_ToAddMod **SRB1_config                         = &rrc_inst->SRB1_config[UE_index];
  struct SRB_ToAddMod **SRB2_config                         = &rrc_inst->SRB2_config[UE_index];
  struct DRB_ToAddMod **DRB_config                          = &rrc_inst->DRB_config[UE_index][0];
  struct PhysicalConfigDedicated  **physicalConfigDedicated = &rrc_inst->physicalConfigDedicated[UE_index];

  struct SRB_ToAddMod *SRB1_config2;
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
  MAC_MainConfig_t *mac_MainConfig;
  MeasObjectToAddModList_t *MeasObj_list;
  MeasObjectToAddMod_t *MeasObj;
  ReportConfigToAddModList_t *ReportConfig_list;
  ReportConfigToAddMod_t *ReportConfig_per,*ReportConfig_A1,*ReportConfig_A2,*ReportConfig_A3,*ReportConfig_A4,*ReportConfig_A5;
  MeasIdToAddModList_t *MeasId_list;
  MeasIdToAddMod_t *MeasId0,*MeasId1,*MeasId2,*MeasId3,*MeasId4,*MeasId5;
  QuantityConfig_t *quantityConfig;
  MobilityControlInfo_t *mobilityInfo;

  //config rach preamble

  HandoverCommand_t handoverCommand;
  u8 sourceModId = get_adjacent_cell_mod_id(rrc_inst->handover_info[UE_index]->as_context.reestablishmentInfo->sourcePhysCellId);

#if Rel10
  long * sr_ProhibitTimer_r9;
  struct PUSCH_CAConfigDedicated_vlola  *pusch_CAConfigDedicated_vlola;
#endif

  long *logicalchannelgroup,*logicalchannelgroup_drb;
  long *maxHARQ_Tx, *periodicBSR_Timer;

  long *lcid;

  struct MeasConfig__speedStatePars *Sparams;
  CellsToAddMod_t *CellToAdd;
  CellsToAddModList_t *CellsToAddModList;



  SRB_list = CALLOC(1,sizeof(*SRB_list));

  // Configure SRB1 (as this is handover)
  /// SRB1
  SRB1_config2 = CALLOC(1,sizeof(*SRB1_config2));
  *SRB1_config = SRB1_config;

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

	/*
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
*/

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
	#ifdef Rel10
	  physicalConfigDedicated2->pusch_CAConfigDedicated_vlola = CALLOC(1,sizeof(*physicalConfigDedicated2->pusch_CAConfigDedicated_vlola));
	#endif
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
	  physicalConfigDedicated2->tpc_PDCCH_ConfigPUCCH->choice.setup.tpc_RNTI.buf[1]=0x34+UE_index;
	  physicalConfigDedicated2->tpc_PDCCH_ConfigPUCCH->choice.setup.tpc_RNTI.bits_unused=0;

	  physicalConfigDedicated2->tpc_PDCCH_ConfigPUSCH->present=TPC_PDCCH_Config_PR_setup;
	  physicalConfigDedicated2->tpc_PDCCH_ConfigPUSCH->choice.setup.tpc_Index.present = TPC_Index_PR_indexOfFormat3;
	  physicalConfigDedicated2->tpc_PDCCH_ConfigPUSCH->choice.setup.tpc_Index.choice.indexOfFormat3 = 1;
	  physicalConfigDedicated2->tpc_PDCCH_ConfigPUSCH->choice.setup.tpc_RNTI.buf=CALLOC(1,2);
	  physicalConfigDedicated2->tpc_PDCCH_ConfigPUSCH->choice.setup.tpc_RNTI.size=2;
	  physicalConfigDedicated2->tpc_PDCCH_ConfigPUSCH->choice.setup.tpc_RNTI.buf[0]=0x22;
	  physicalConfigDedicated2->tpc_PDCCH_ConfigPUSCH->choice.setup.tpc_RNTI.buf[1]=0x34+UE_index;
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
	  physicalConfigDedicated2->schedulingRequestConfig->choice.setup.sr_PUCCH_ResourceIndex = UE_index;

	  if (lte_frame_parms->frame_type == 0) // FDD
	    physicalConfigDedicated2->schedulingRequestConfig->choice.setup.sr_ConfigIndex = 5+(UE_index%10);  // Isr = 5 (every 10 subframes, offset=2+UE_id mod3)
	  else {
	    switch (lte_frame_parms->tdd_config) {
	    case 1:
	      physicalConfigDedicated2->schedulingRequestConfig->choice.setup.sr_ConfigIndex = 7+(UE_index&1)+((UE_index&3)>>1)*5;  // Isr = 5 (every 10 subframes, offset=2 for UE0, 3 for UE1, 7 for UE2, 8 for UE3 , 2 for UE4 etc..)
	      break;
	    case 3:
	      physicalConfigDedicated2->schedulingRequestConfig->choice.setup.sr_ConfigIndex = 7+(UE_index%3);  // Isr = 5 (every 10 subframes, offset=2 for UE0, 3 for UE1, 3 for UE2, 2 for UE3 , etc..)
	      break;
	    case 4:
	      physicalConfigDedicated2->schedulingRequestConfig->choice.setup.sr_ConfigIndex = 7+(UE_index&1);  // Isr = 5 (every 10 subframes, offset=2 for UE0, 3 for UE1, 3 for UE2, 2 for UE3 , etc..)
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


	// Mimicking RRCConnectionSetup behavior at eNB
	  LOG_D(RRC, "handover_config [MSC_MSG][FRAME %05d][RRC_eNB][MOD %02d][][--- MAC_CONFIG_REQ  (SRB1 UE %d) --->][MAC_eNB][MOD %02d][]\n",
		frame, Mod_id, UE_index, Mod_id);
	  rrc_mac_config_req(Mod_id,1,UE_index,0,
			     (RadioResourceConfigCommonSIB_t *)NULL,
			     eNB_rrc_inst[Mod_id].physicalConfigDedicated[UE_index],
			     (MeasObjectToAddMod_t **)NULL,
			     eNB_rrc_inst[Mod_id].mac_MainConfig[UE_index],
			     1,
			     SRB1_logicalChannelConfig,
			     eNB_rrc_inst[Mod_id].measGapConfig[UE_index],
			     (TDD_Config_t *)NULL,
			     (MobilityControlInfo_t *)NULL,
			     (u8 *)NULL,
			     (u16 *)NULL);

  // Configure SRB2
  /// SRB2
  SRB2_config2 = CALLOC(1,sizeof(*SRB2_config2));
  *SRB2_config = SRB2_config2;

  SRB2_config2->srb_Identity = 2;
  SRB2_rlc_config = CALLOC(1,sizeof(*SRB2_rlc_config));
  SRB2_config2->rlc_Config   = SRB2_rlc_config;

  SRB2_rlc_config->present = SRB_ToAddMod__rlc_Config_PR_explicitValue;
  SRB2_rlc_config->choice.explicitValue.present=RLC_Config_PR_am;
  SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.t_PollRetransmit = T_PollRetransmit_ms45;
  SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.pollPDU          = PollPDU_pInfinity;
  SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.pollByte         = PollPDU_pInfinity;
  SRB2_rlc_config->choice.explicitValue.choice.am.ul_AM_RLC.maxRetxThreshold = UL_AM_RLC__maxRetxThreshold_t4;
  SRB2_rlc_config->choice.explicitValue.choice.am.dl_AM_RLC.t_Reordering     = T_Reordering_ms35;
  SRB2_rlc_config->choice.explicitValue.choice.am.dl_AM_RLC.t_StatusProhibit = T_StatusProhibit_ms0;


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
  DRB_rlc_config->choice.um_Bi_Directional.ul_UM_RLC.sn_FieldLength=SN_FieldLength_size5;
  DRB_rlc_config->choice.um_Bi_Directional.dl_UM_RLC.sn_FieldLength=SN_FieldLength_size5;
  DRB_rlc_config->choice.um_Bi_Directional.dl_UM_RLC.t_Reordering=T_Reordering_ms35;
  DRB_lchan_config = CALLOC(1,sizeof(*DRB_lchan_config));
  DRB_config2->logicalChannelConfig   = DRB_lchan_config;
  DRB_ul_SpecificParameters = CALLOC(1,sizeof(*DRB_ul_SpecificParameters));
  DRB_lchan_config->ul_SpecificParameters = DRB_ul_SpecificParameters;


  DRB_ul_SpecificParameters->priority = 2; // lower priority than srb1, srb2
  DRB_ul_SpecificParameters->prioritisedBitRate=LogicalChannelConfig__ul_SpecificParameters__prioritisedBitRate_infinity;
  DRB_ul_SpecificParameters->bucketSizeDuration=LogicalChannelConfig__ul_SpecificParameters__bucketSizeDuration_ms50;

  logicalchannelgroup_drb = CALLOC(1,sizeof(long));
  *logicalchannelgroup_drb=0;
  DRB_ul_SpecificParameters->logicalChannelGroup = logicalchannelgroup_drb;


  ASN_SEQUENCE_ADD(&DRB_list->list,DRB_config2);

  mac_MainConfig = CALLOC(1,sizeof(*mac_MainConfig));
  eNB_rrc_inst[Mod_id].mac_MainConfig[UE_index] = mac_MainConfig;

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

#ifdef Rel10
  sr_ProhibitTimer_r9 = CALLOC(1,sizeof(long));
  *sr_ProhibitTimer_r9=0; // SR tx on PUCCH, Value in number of SR period(s). Value 0 = no timer for SR, Value 2= 2*SR
  mac_MainConfig->sr_ProhibitTimer_r9=sr_ProhibitTimer_r9;
  sps_RA_ConfigList_rlola = NULL;
#endif


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

  //  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig->measIdToAddModList = MeasId_list;

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
  //  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig->measObjectToAddModList = MeasObj_list;

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
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA3.a3_Offset = 0.5;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA3.reportOnLeave = 1;

  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.triggerQuantity                       = ReportConfigEUTRA__triggerQuantity_rsrp;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.reportQuantity                        = ReportConfigEUTRA__reportQuantity_both;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.maxReportCells                        = 2;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.reportInterval                        = ReportInterval_ms120;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.reportAmount                          = ReportConfigEUTRA__reportAmount_infinity;

  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.hysteresis = 0.5;
  ReportConfig_A3->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.timeToTrigger = TimeToTrigger_ms40;

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
  //  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig->reportConfigToAddModList = ReportConfig_list;

  Sparams = CALLOC(1,sizeof(*Sparams));
  Sparams->present=MeasConfig__speedStatePars_PR_setup;
  Sparams->choice.setup.timeToTrigger_SF.sf_High=SpeedStateScaleFactors__sf_Medium_oDot75;
  Sparams->choice.setup.timeToTrigger_SF.sf_Medium=SpeedStateScaleFactors__sf_High_oDot5;
  Sparams->choice.setup.mobilityStateParameters.n_CellChangeHigh=10;
  Sparams->choice.setup.mobilityStateParameters.n_CellChangeMedium=5;
  Sparams->choice.setup.mobilityStateParameters.t_Evaluation=MobilityStateParameters__t_Evaluation_s60;
  Sparams->choice.setup.mobilityStateParameters.t_HystNormal=MobilityStateParameters__t_HystNormal_s120;

  quantityConfig = CALLOC(1,sizeof(*quantityConfig));
  memset((void *)quantityConfig,0,sizeof(*quantityConfig));
  quantityConfig->quantityConfigEUTRA = CALLOC(1,sizeof(struct QuantityConfigEUTRA));
  memset((void *)quantityConfig->quantityConfigEUTRA,0,sizeof(*quantityConfig->quantityConfigEUTRA));
  quantityConfig->quantityConfigCDMA2000 = NULL;
  quantityConfig->quantityConfigGERAN = NULL;
  quantityConfig->quantityConfigUTRA = NULL;
  quantityConfig->quantityConfigEUTRA->filterCoefficientRSRP = CALLOC(1,sizeof(*(quantityConfig->quantityConfigEUTRA->filterCoefficientRSRP)));
  quantityConfig->quantityConfigEUTRA->filterCoefficientRSRQ = CALLOC(1,sizeof(*(quantityConfig->quantityConfigEUTRA->filterCoefficientRSRQ)));
  *quantityConfig->quantityConfigEUTRA->filterCoefficientRSRP = FilterCoefficient_fc4;
  *quantityConfig->quantityConfigEUTRA->filterCoefficientRSRQ = FilterCoefficient_fc4;

  /*
    PhysCellId_t	 targetPhysCellId;
    struct CarrierFreqEUTRA	*carrierFreq	/* OPTIONAL */;
  //	struct CarrierBandwidthEUTRA	*carrierBandwidth	/* OPTIONAL */;
  //	AdditionalSpectrumEmission_t	*additionalSpectrumEmission	/* OPTIONAL */;
  //	long	 t304;
  //  	C_RNTI_t	 newUE_Identity;
  // 	RadioResourceConfigCommon_t	 radioResourceConfigCommon;
  // 	struct RACH_ConfigDedicated	*rach_ConfigDedicated	/* OPTIONAL */;
  // */

  mobilityInfo = CALLOC(1,sizeof(*mobilityInfo));
  memset((void *)mobilityInfo,0,sizeof(*mobilityInfo));
  mobilityInfo->targetPhysCellId = (PhysCellId_t) two_tier_hexagonal_cellIds[Mod_id, rrc_inst->handover_info[UE_index]->modid_t];
  LOG_D(RRC,"\nHO: Process_handover_preparation_info: targetPhysCellId: %d mod_id: %d UE_index: %d \n",mobilityInfo->targetPhysCellId,Mod_id,UE_index);

  mobilityInfo->additionalSpectrumEmission = CALLOC(1,sizeof(*mobilityInfo->additionalSpectrumEmission));
  *mobilityInfo->additionalSpectrumEmission = 1; //Check this value!

  mobilityInfo->t304 = MobilityControlInfo__t304_ms50; // need to configure an appropriate value here

  // New UE Identity (C-RNTI) to identify an UE uniquely in a cell
  mobilityInfo->newUE_Identity.size = 2;
  mobilityInfo->newUE_Identity.bits_unused = 0;
  mobilityInfo->newUE_Identity.buf = rv;
  mobilityInfo->newUE_Identity.buf[0] = rv[0];
  mobilityInfo->newUE_Identity.buf[1] = rv[1];

  //memset((void *)&mobilityInfo->radioResourceConfigCommon,(void *)&rrc_inst->sib2->radioResourceConfigCommon,sizeof(RadioResourceConfigCommon_t));
  //memset((void *)&mobilityInfo->radioResourceConfigCommon,0,sizeof(RadioResourceConfigCommon_t));

  // Configuring radioResourceConfigCommon
  mobilityInfo->radioResourceConfigCommon.rach_ConfigCommon = CALLOC(1,sizeof(*mobilityInfo->radioResourceConfigCommon.rach_ConfigCommon));
  memcpy((void *)mobilityInfo->radioResourceConfigCommon.rach_ConfigCommon, (void *)&rrc_inst->sib2->radioResourceConfigCommon.rach_ConfigCommon,sizeof(RACH_ConfigCommon_t));
  mobilityInfo->radioResourceConfigCommon.prach_Config.prach_ConfigInfo = CALLOC(1,sizeof(*mobilityInfo->radioResourceConfigCommon.prach_Config.prach_ConfigInfo));
  memcpy((void *)mobilityInfo->radioResourceConfigCommon.prach_Config.prach_ConfigInfo, (void *)&rrc_inst->sib2->radioResourceConfigCommon.prach_Config.prach_ConfigInfo,sizeof(PRACH_ConfigInfo_t));
  mobilityInfo->radioResourceConfigCommon.prach_Config.rootSequenceIndex = rrc_inst->sib2->radioResourceConfigCommon.prach_Config.rootSequenceIndex;
  mobilityInfo->radioResourceConfigCommon.pdsch_ConfigCommon = CALLOC(1,sizeof(*mobilityInfo->radioResourceConfigCommon.pdsch_ConfigCommon));
  memcpy((void *)mobilityInfo->radioResourceConfigCommon.pdsch_ConfigCommon, (void *)&rrc_inst->sib2->radioResourceConfigCommon.pdsch_ConfigCommon,sizeof(PDSCH_ConfigCommon_t));
  memcpy((void *)&mobilityInfo->radioResourceConfigCommon.pusch_ConfigCommon,(void *)&rrc_inst->sib2->radioResourceConfigCommon.pusch_ConfigCommon,sizeof(PUSCH_ConfigCommon_t));
  mobilityInfo->radioResourceConfigCommon.phich_Config = NULL;
  mobilityInfo->radioResourceConfigCommon.pucch_ConfigCommon = CALLOC(1,sizeof(*mobilityInfo->radioResourceConfigCommon.pucch_ConfigCommon));
  memcpy((void *)mobilityInfo->radioResourceConfigCommon.pucch_ConfigCommon, (void *)&rrc_inst->sib2->radioResourceConfigCommon.pucch_ConfigCommon,sizeof(PUCCH_ConfigCommon_t));
  mobilityInfo->radioResourceConfigCommon.soundingRS_UL_ConfigCommon = CALLOC(1,sizeof(*mobilityInfo->radioResourceConfigCommon.soundingRS_UL_ConfigCommon));
  memcpy((void *)mobilityInfo->radioResourceConfigCommon.soundingRS_UL_ConfigCommon, (void *)&rrc_inst->sib2->radioResourceConfigCommon.soundingRS_UL_ConfigCommon,sizeof(SoundingRS_UL_ConfigCommon_t));
  mobilityInfo->radioResourceConfigCommon.uplinkPowerControlCommon = CALLOC(1,sizeof(*mobilityInfo->radioResourceConfigCommon.uplinkPowerControlCommon));
  memcpy((void *)mobilityInfo->radioResourceConfigCommon.uplinkPowerControlCommon, (void *)&rrc_inst->sib2->radioResourceConfigCommon.uplinkPowerControlCommon,sizeof(UplinkPowerControlCommon_t));
  mobilityInfo->radioResourceConfigCommon.antennaInfoCommon = NULL;
  mobilityInfo->radioResourceConfigCommon.p_Max = NULL; // CALLOC(1,sizeof(*mobilityInfo->radioResourceConfigCommon.p_Max));
  //memcpy((void *)mobilityInfo->radioResourceConfigCommon.p_Max,(void *)rrc_inst->sib1->p_Max,sizeof(P_Max_t));
  mobilityInfo->radioResourceConfigCommon.tdd_Config = NULL; //CALLOC(1,sizeof(TDD_Config_t));
  //memcpy((void *)mobilityInfo->radioResourceConfigCommon.tdd_Config,(void *)rrc_inst->sib1->tdd_Config,sizeof(TDD_Config_t));
  mobilityInfo->radioResourceConfigCommon.ul_CyclicPrefixLength = rrc_inst->sib2->radioResourceConfigCommon.ul_CyclicPrefixLength;
  //End of configuration of radioResourceConfigCommon

  mobilityInfo->carrierFreq = CALLOC(1,sizeof(*mobilityInfo->carrierFreq)); //CALLOC(1,sizeof(CarrierFreqEUTRA_t)); 36090
  mobilityInfo->carrierFreq->dl_CarrierFreq = 36090;
  mobilityInfo->carrierFreq->ul_CarrierFreq = NULL;

  mobilityInfo->carrierBandwidth = CALLOC(1,sizeof(*mobilityInfo->carrierBandwidth)); //CALLOC(1,sizeof(struct CarrierBandwidthEUTRA));  AllowedMeasBandwidth_mbw25
  mobilityInfo->carrierBandwidth->dl_Bandwidth = CarrierBandwidthEUTRA__dl_Bandwidth_n25;
  mobilityInfo->carrierBandwidth->ul_Bandwidth = NULL;
  mobilityInfo->rach_ConfigDedicated = NULL;


  // Check if below needs to be done at target eNB
  /*
    memcpy((void *)rrc_inst->handover_info[UE_index]->as_config.sourceRadioResourceConfig.srb_ToAddModList,(void *)SRB_list,sizeof(SRB_ToAddModList_t));
    memcpy((void *)rrc_inst->handover_info[UE_index]->as_config.sourceRadioResourceConfig.drb_ToAddModList,(void *)DRB_list,sizeof(DRB_ToAddModList_t));
    rrc_inst->handover_info[UE_index]->as_config.sourceRadioResourceConfig.drb_ToReleaseList = NULL;
    memcpy((void *)rrc_inst->handover_info[UE_index]->as_config.sourceRadioResourceConfig.mac_MainConfig,(void *)mac_MainConfig,sizeof(MAC_MainConfig_t));
    memcpy((void *)rrc_inst->handover_info[UE_index]->as_config.sourceRadioResourceConfig.physicalConfigDedicated,(void *)rrc_inst->physicalConfigDedicated[UE_index],sizeof(PhysicalConfigDedicated_t));
    memcpy((void *)rrc_inst->handover_info[UE_index]->as_config.sourceRadioResourceConfig.sps_Config,(void *)rrc_inst->sps_Config[UE_index],sizeof(SPS_Config_t));
  */

	/************************************* Adding new UE procedure begins ********************************************/
	Idx = (UE_index * MAX_NUM_RB) + DCCH;
	// SRB1
	eNB_rrc_inst[Mod_id].Srb1[UE_index].Active = 1;
	eNB_rrc_inst[Mod_id].Srb1[UE_index].Srb_info.Srb_id = Idx;
	memcpy(&eNB_rrc_inst[Mod_id].Srb1[UE_index].Srb_info.Lchan_desc[0],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
	memcpy(&eNB_rrc_inst[Mod_id].Srb1[UE_index].Srb_info.Lchan_desc[1],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);

	// SRB2
	eNB_rrc_inst[Mod_id].Srb2[UE_index].Active = 1;
	eNB_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.Srb_id = Idx+1; //Check this!
	memcpy(&eNB_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.Lchan_desc[0],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
	memcpy(&eNB_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.Lchan_desc[1],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);

	//rrc_eNB_generate_RRCConnectionSetup(Mod_id,frame,UE_index);
	//LOG_D(RRC, "[MSC_NBOX][FRAME %05d][RRC_eNB][MOD %02d][][Tx RRCConnectionSetup][RRC_eNB][MOD %02d][]\n",
	//      frame, Mod_id, Mod_id);

	//LOG_D(RRC,"[eNB %d] RLC AM allocation index@0 is %d\n",Mod_id,rlc[Mod_id].m_rlc_am_array[0].allocation);
	//LOG_D(RRC,"[eNB %d] RLC AM allocation index@1 is %d\n",Mod_id,rlc[Mod_id].m_rlc_am_array[1].allocation);
	LOG_I(RRC,"[eNB %d] CALLING RLC CONFIG SRB1 (rbid %d) for UE %d\n",
	      Mod_id,Idx,UE_index);
	rrc_pdcp_config_req (Mod_id, frame, 1, ACTION_ADD, Idx);
	rrc_rlc_config_req(Mod_id,frame,1,ACTION_ADD,Idx,SIGNALLING_RADIO_BEARER,Rlc_info_am_config);

	/************************************* Adding new UE procedure ends ********************************************/

  size = do_RRCConnectionReconfiguration(Mod_id,
					 buffer,
					 UE_index,
					 0,//Transaction_id,
					 SRB_list,
					 DRB_list,
					 NULL, // DRB2_list,
					 NULL, //*sps_Config,
					 physicalConfigDedicated,
					 MeasObj_list,
					 ReportConfig_list,
					 quantityConfig, //*QuantityConfig,
					 MeasId_list,
					 mac_MainConfig,
					 NULL,
					 mobilityInfo); //*measGapConfig

  LOG_I(RRC,"[eNB %d] Frame %d, Logical Channel DL-DCCH, Generate RRCConnectionReconfiguration HO (bytes %d, UE id %d)\n",
	Mod_id,frame, size, UE_index);

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

  LOG_D(RRC, "[MSC_MSG][FRAME %05d][RRC_eNB][MOD %02d][][--- MAC_CONFIG_REQ  (HO UE %d) --->][MAC_eNB][MOD %02d][]\n",
	frame, Mod_id, UE_index, Mod_id);

  rrc_mac_config_req(Mod_id,1,UE_index,1,
		     (RadioResourceConfigCommonSIB_t *)NULL,
		     eNB_rrc_inst[Mod_id].physicalConfigDedicated[UE_index],
		     (MeasObjectToAddMod_t **)NULL,
		     eNB_rrc_inst[Mod_id].mac_MainConfig[UE_index],
		     1,
		     SRB1_logicalChannelConfig,
		     eNB_rrc_inst[Mod_id].measGapConfig[UE_index],
		     (TDD_Config_t *)NULL,
		     (MobilityControlInfo_t *)mobilityInfo,
		     (u8 *)NULL,
		     (u16 *)NULL);

  handoverCommand.criticalExtensions.present = HandoverCommand__criticalExtensions_PR_c1;
  handoverCommand.criticalExtensions.choice.c1.present = HandoverCommand__criticalExtensions__c1_PR_handoverCommand_r8;
  handoverCommand.criticalExtensions.choice.c1.choice.handoverCommand_r8.handoverCommandMessage.buf = buffer;
  handoverCommand.criticalExtensions.choice.c1.choice.handoverCommand_r8.handoverCommandMessage.size = size;

  if (sourceModId != 0xFF) {
    memcpy(eNB_rrc_inst[sourceModId].handover_info[eNB_rrc_inst[Mod_id].handover_info[UE_index]->ueid_s]->buf,(void *)buffer,size);
    eNB_rrc_inst[sourceModId].handover_info[eNB_rrc_inst[Mod_id].handover_info[UE_index]->ueid_s]->size = size;
    eNB_rrc_inst[sourceModId].handover_info[eNB_rrc_inst[Mod_id].handover_info[UE_index]->ueid_s]->ho_complete = 0xF1;
    eNB_rrc_inst[Mod_id].handover_info[UE_index]->ho_complete = 0xFF;
  }
  else
    LOG_D(RRC,"\nError! rrc_eNB_generate_RRCConnectionReconfiguration_handover: Could not find source eNB mod_id. ");


}


void rrc_eNB_process_handoverPreparationInformation(u8 Mod_id,u32 frame, u16 UE_index) {

  LOG_I(RRC,"[eNB %d][RAPROC] Frame %d : Logical Channel UL-DCCH, processing RRCHandoverPreparationInformation from source eNB, sending RRCReconf to source eNB for user %d \n",Mod_id,frame,UE_index);


  //eNB_rrc_inst[Mod_id].Info.UE_list[UE_index]
  rrc_eNB_generate_RRCConnectionReconfiguration_handover(Mod_id,frame,UE_index,mac_xface->lte_frame_parms);

}

void check_handovers(u8 Mod_id,PHY_VARS_eNB *phy_vars_eNB) {
	u8 i;
	for (i=0;i<NUMBER_OF_UE_MAX;i++) {
	  if(eNB_rrc_inst[phy_vars_eNB->Mod_id].handover_info[i] != NULL) {

		  if(eNB_rrc_inst[phy_vars_eNB->Mod_id].handover_info[i]->ho_prepare == 0xFF) {
			  LOG_D(RRC,"\n Incoming HO detected for new UE_idx %d current eNB %d target eNB: %d \n",i,phy_vars_eNB->Mod_id, eNB_rrc_inst[phy_vars_eNB->Mod_id].handover_info[i]->modid_t);
			  rrc_eNB_process_handoverPreparationInformation(phy_vars_eNB->Mod_id,phy_vars_eNB->frame,i);
			  eNB_rrc_inst[phy_vars_eNB->Mod_id].handover_info[i]->ho_prepare = 0xF1;
		  }

		  if(eNB_rrc_inst[phy_vars_eNB->Mod_id].handover_info[i]->ho_complete == 0xF1) {
			  LOG_D(RRC,"\n HO Command received for new UE_idx %d current eNB %d target eNB: %d \n",i,phy_vars_eNB->Mod_id,eNB_rrc_inst[phy_vars_eNB->Mod_id].handover_info[i]->modid_t);
			  //rrc_eNB_process_handoverPreparationInformation(Mod_id,frame,i);
			  //rrc_rlc_data_req(phy_vars_eNB->Mod_id,phy_vars_eNB->frame, 1,(i*MAX_NUM_RB)+DCCH,rrc_eNB_mui++,0,eNB_rrc_inst[phy_vars_eNB->Mod_id].handover_info[i]->size,(char*)eNB_rrc_inst[phy_vars_eNB->Mod_id].handover_info[i]->buf);
			  pdcp_data_req(Mod_id,phy_vars_eNB->frame, 1,(i*MAX_NUM_RB)+DCCH,rrc_eNB_mui++,0,eNB_rrc_inst[phy_vars_eNB->Mod_id].handover_info[i]->size,(char*)eNB_rrc_inst[phy_vars_eNB->Mod_id].handover_info[i]->buf,1);
			  eNB_rrc_inst[phy_vars_eNB->Mod_id].handover_info[i]->ho_complete = 0xF2;
		  }
	  }
	}
}

/*
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
*/
#ifndef USER_MODE
EXPORT_SYMBOL(Rlc_info_am_config);
#endif
