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

/*! \file rrc_UE.c
* \brief rrc procedures for UE
* \author Raymond Knopp and Navid Nikaein
* \date 2011
* \version 1.0
* \company Eurecom
* \email: raymond.knopp@eurecom.fr and  navid.nikaein@eurecom.fr
*/


#include "defs.h"
#include "extern.h"
#include "RRC/L2_INTERFACE/openair_rrc_L2_interface.h"
#include "LAYER2/RLC/rlc.h"
#include "COMMON/mac_rrc_primitives.h"
#include "UTIL/LOG/log.h"
//#include "RRC/LITE/MESSAGES/asn1_msg.h"
#include "RRCConnectionRequest.h"
#include "RRCConnectionReconfiguration.h"
#include "UL-CCCH-Message.h"
#include "DL-CCCH-Message.h"
#include "UL-DCCH-Message.h"
#include "DL-DCCH-Message.h"
#include "BCCH-DL-SCH-Message.h"
#include "MeasConfig.h"
#include "MeasGapConfig.h"
#include "MeasObjectEUTRA.h"
#include "TDD-Config.h"
//TCS LOLAmesh
#include "PHY/extern.h"
#ifdef PHY_ABSTRACTION
#include "OCG.h"
#include "OCG_extern.h"
#endif
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

void init_SI_UE(u8 Mod_id,u8 eNB_index) {

  int i;


  UE_rrc_inst[Mod_id].sizeof_SIB1[eNB_index] = 0;
  UE_rrc_inst[Mod_id].sizeof_SI[eNB_index] = 0;

  UE_rrc_inst[Mod_id].SIB1[eNB_index] = (u8 *)malloc16(32);
  UE_rrc_inst[Mod_id].sib1[eNB_index] = (SystemInformationBlockType1_t *)malloc16(sizeof(SystemInformationBlockType1_t));
  UE_rrc_inst[Mod_id].SI[eNB_index] = (u8 *)malloc16(64);

  for (i=0;i<NB_CNX_UE;i++) {
     UE_rrc_inst[Mod_id].si[eNB_index][i] = (SystemInformation_t *)malloc16(sizeof(SystemInformation_t));
  }

  UE_rrc_inst[Mod_id].Info[eNB_index].SIB1Status = 0;
  UE_rrc_inst[Mod_id].Info[eNB_index].SIStatus = 0;


}

/*------------------------------------------------------------------------------*/
char openair_rrc_lite_ue_init(u8 Mod_id, unsigned char eNB_index){
  /*-----------------------------------------------------------------------------*/

  LOG_D(RRC,"[UE %d] INIT State = RRC_IDLE (eNB %d)\n",Mod_id,eNB_index);
  LOG_D(RRC,"[MSC_NEW][FRAME 00000][RRC_UE][MOD %02d][]\n", Mod_id+NB_eNB_INST);

  UE_rrc_inst[Mod_id].Info[eNB_index].State=RRC_IDLE;
  UE_rrc_inst[Mod_id].Info[eNB_index].T300_active = 0;
  UE_rrc_inst[Mod_id].Info[eNB_index].T304_active = 0;
  UE_rrc_inst[Mod_id].Info[eNB_index].T310_active = 0;
  UE_rrc_inst[Mod_id].Info[eNB_index].UE_index=0xffff;
  UE_rrc_inst[Mod_id].Srb0[eNB_index].Active=0;
  UE_rrc_inst[Mod_id].Srb1[eNB_index].Active=0;
  UE_rrc_inst[Mod_id].Srb2[eNB_index].Active=0;

  init_SI_UE(Mod_id,eNB_index);
  LOG_D(RRC,"[UE %d] INIT: phy_sync_2_ch_ind\n", Mod_id);

#ifndef NO_RRM
  send_msg(&S_rrc,msg_rrc_phy_synch_to_CH_ind(Mod_id,eNB_index,UE_rrc_inst[Mod_id].Mac_id));
#endif

#ifdef NO_RRM //init ch SRB0, SRB1 & BDTCH
  openair_rrc_on(Mod_id,0);
#endif

  return 0;
}


/*------------------------------------------------------------------------------*/
void rrc_ue_generate_RRCConnectionRequest(u8 Mod_id, u32 frame, u8 eNB_index){
  /*------------------------------------------------------------------------------*/

  u8 i=0,rv[6];

  if(UE_rrc_inst[Mod_id].Srb0[eNB_index].Tx_buffer.payload_size ==0){

    // Get RRCConnectionRequest, fill random for now
    // Generate random byte stream for contention resolution
    for (i=0;i<6;i++) {
      rv[i]=taus()&0xff;
      LOG_T(RRC,"%x.",rv[i]);
    }
    LOG_T(RRC,"\n");
    UE_rrc_inst[Mod_id].Srb0[eNB_index].Tx_buffer.payload_size = do_RRCConnectionRequest((u8 *)UE_rrc_inst[Mod_id].Srb0[eNB_index].Tx_buffer.Payload,rv);

    LOG_I(RRC,"[UE %d] : Frame %d, Logical Channel UL-CCCH (SRB0), Generating RRCConnectionRequest (bytes %d, eNB %d)\n",
	  Mod_id, frame, UE_rrc_inst[Mod_id].Srb0[eNB_index].Tx_buffer.payload_size, eNB_index);

    for (i=0;i<UE_rrc_inst[Mod_id].Srb0[eNB_index].Tx_buffer.payload_size;i++) {
      LOG_T(RRC,"%x.",UE_rrc_inst[Mod_id].Srb0[eNB_index].Tx_buffer.Payload[i]);
    }
    LOG_T(RRC,"\n");
    /*
      UE_rrc_inst[Mod_id].Srb0[Idx].Tx_buffer.Payload[i] = taus()&0xff;

    UE_rrc_inst[Mod_id].Srb0[Idx].Tx_buffer.payload_size =i;
    */

  }
}


mui_t rrc_mui=0;


/*------------------------------------------------------------------------------*/
void rrc_ue_generate_RRCConnectionSetupComplete(u8 Mod_id, u32 frame, u8 eNB_index){
  /*------------------------------------------------------------------------------*/

  u8 buffer[32];
  u8 size;

  size = do_RRCConnectionSetupComplete(buffer);

  LOG_I(RRC,"[UE %d][RAPROC] Frame %d : Logical Channel UL-DCCH (SRB1), Generating RRCConnectionSetupComplete (bytes%d, eNB %d)\n",
	Mod_id,frame, size, eNB_index);

   LOG_D(RLC, "[MSC_MSG][FRAME %05d][RRC_UE][MOD %02d][][--- PDCP_DATA_REQ/%d Bytes (RRCConnectionSetupComplete to eNB %d MUI %d) --->][PDCP][MOD %02d][RB %02d]\n",
                                     frame, Mod_id+NB_eNB_INST, size, eNB_index, rrc_mui, Mod_id+NB_eNB_INST, (eNB_index * MAX_NUM_RB) + DCCH);

   //  rrc_rlc_data_req(Mod_id+NB_eNB_INST,frame, 0 ,DCCH,rrc_mui++,0,size,(char*)buffer);
   pdcp_data_req(Mod_id+NB_eNB_INST,frame, 0 ,(eNB_index * MAX_NUM_RB) + DCCH,rrc_mui++,0,size,(char*)buffer,1);

}



void rrc_ue_generate_RRCConnectionReconfigurationComplete(u8 Mod_id, u32 frame, u8 eNB_index, u8 isCODRB, u8 virtualLinkID) {

  u8 buffer[32], size;

  size = do_RRCConnectionReconfigurationComplete(buffer,isCODRB,virtualLinkID);

  LOG_I(RRC,"[UE %d] Frame %d : Logical Channel UL-DCCH (SRB1), Generating RRCConnectionReconfigurationComplete (bytes %d, eNB_index %d)\n",
	Mod_id,frame, size, eNB_index);
  LOG_D(RLC, "[MSC_MSG][FRAME %05d][RRC_UE][MOD %02d][][--- PDCP_DATA_REQ/%d Bytes (RRCConnectionReconfigurationComplete to eNB %d MUI %d) --->][PDCP][MOD %02d][RB %02d]\n",
	frame, Mod_id+NB_eNB_INST, size, eNB_index, rrc_mui, Mod_id+NB_eNB_INST, (eNB_index * MAX_NUM_RB) + DCCH);

  //rrc_rlc_data_req(Mod_id+NB_eNB_INST,frame, 0 ,DCCH,rrc_mui++,0,size,(char*)buffer);
  pdcp_data_req(Mod_id+NB_eNB_INST,frame, 0 ,(eNB_index * MAX_NUM_RB) + DCCH,rrc_mui++,0,size,(char*)buffer,1);
}


void rrc_ue_generate_MeasurementReport(u8 Mod_id,u8 eNB_index) {

  u8 buffer[32], size;

  msg("[RRC][UE %d] Frame %d : Generating Measurement Report\n",Mod_id,Mac_rlc_xface->frame);

  size = do_MeasurementReport(buffer,1,0,3,4,5,6);

  LOG_D(RLC, "[MSC_MSG][FRAME %05d][RRC_UE][MOD %02d][][--- PDCP_DATA_REQ/%d Bytes (RRCConnectionReconfigurationComplete to eNB %d MUI %d) --->][PDCP][MOD %02d][RB %02d]\n",
    Mac_rlc_xface->frame, Mod_id+NB_eNB_INST, size, eNB_index, rrc_mui, Mod_id+NB_eNB_INST, DCCH);
  //rrc_rlc_data_req(Mod_id+NB_eNB_INST,DCCH,rrc_mui++,0,size,(char*)buffer);
  pdcp_data_req(Mod_id+NB_eNB_INST,(eNB_index * MAX_NUM_RB) + DCCH,rrc_mui++,0,size,(char*)buffer,1);
}

/*------------------------------------------------------------------------------*/
int rrc_ue_decode_ccch(u8 Mod_id, u32 frame, SRB_INFO *Srb_info, u8 eNB_index){
  /*------------------------------------------------------------------------------*/

  //DL_CCCH_Message_t dlccchmsg;
  DL_CCCH_Message_t *dl_ccch_msg=NULL;//&dlccchmsg;
  asn_dec_rval_t dec_rval;
  int i,rval=0;

  //memset(dl_ccch_msg,0,sizeof(DL_CCCH_Message_t));

  //  LOG_D(RRC,"[UE %d] Decoding DL-CCCH message (%d bytes), State %d\n",Mod_id,Srb_info->Rx_buffer.payload_size,
  //	UE_rrc_inst[Mod_id].Info[eNB_index].State);

  dec_rval = uper_decode(NULL,
			 &asn_DEF_DL_CCCH_Message,
			 (void**)&dl_ccch_msg,
	 		 (uint8_t*)Srb_info->Rx_buffer.Payload,
			 100,0,0);

  xer_fprint(stdout,&asn_DEF_DL_CCCH_Message,(void*)dl_ccch_msg);

  if ((dec_rval.code != RC_OK) && (dec_rval.consumed==0)) {
    LOG_E(RRC,"[UE %d] Frame %d : Failed to decode DL-CCCH-Message (%d bytes)\n",Mod_id,dec_rval.consumed);
    return -1;
  }

  if (dl_ccch_msg->message.present == DL_CCCH_MessageType_PR_c1) {

    if (UE_rrc_inst[Mod_id].Info[eNB_index].State == RRC_SI_RECEIVED) {

      switch (dl_ccch_msg->message.choice.c1.present) {

      case DL_CCCH_MessageType__c1_PR_NOTHING :

	LOG_I(RRC,"[UE%d] Frame %d : Received PR_NOTHING on DL-CCCH-Message\n",Mod_id,frame);
	rval= 0;
	break;
      case DL_CCCH_MessageType__c1_PR_rrcConnectionReestablishment:
          LOG_D(RRC, "[MSC_MSG][FRAME %05d][MAC_UE][MOD %02d][][--- MAC_DATA_IND (rrcConnectionReestablishment ENB %d) --->][RRC_UE][MOD %02d][]\n",
            frame,  Mod_id+NB_eNB_INST, eNB_index,  Mod_id+NB_eNB_INST);

	LOG_I(RRC,"[UE%d] Frame %d : Logical Channel DL-CCCH (SRB0), Received RRCConnectionReestablishment\n",Mod_id,frame);
	rval= 0;
	break;
      case DL_CCCH_MessageType__c1_PR_rrcConnectionReestablishmentReject:
          LOG_D(RRC, "[MSC_MSG][FRAME %05d][MAC_UE][MOD %02d][][--- MAC_DATA_IND (RRCConnectionReestablishmentReject ENB %d) --->][RRC_UE][MOD %02d][]\n",
            frame,  Mod_id+NB_eNB_INST, eNB_index,  Mod_id+NB_eNB_INST);
	LOG_I(RRC,"[UE%d] Frame %d : Logical Channel DL-CCCH (SRB0), Received RRCConnectionReestablishmentReject\n",Mod_id,frame);
	rval= 0;
	break;
      case DL_CCCH_MessageType__c1_PR_rrcConnectionReject:
          LOG_D(RRC, "[MSC_MSG][FRAME %05d][MAC_UE][MOD %02d][][--- MAC_DATA_IND (rrcConnectionReject ENB %d) --->][RRC_UE][MOD %02d][]\n",
            frame,  Mod_id+NB_eNB_INST, eNB_index,  Mod_id+NB_eNB_INST);

	LOG_I(RRC,"[UE%d] Frame %d : Logical Channel DL-CCCH (SRB0), Received RRCConnectionReject \n",Mod_id,frame);
	rval= 0;
	break;
      case DL_CCCH_MessageType__c1_PR_rrcConnectionSetup:
          LOG_D(RRC, "[MSC_MSG][FRAME %05d][MAC_UE][MOD %02d][][--- MAC_DATA_IND (rrcConnectionSetup ENB %d) --->][RRC_UE][MOD %02d][]\n",
            frame,  Mod_id+NB_eNB_INST, eNB_index,  Mod_id+NB_eNB_INST);

	LOG_I(RRC,"[UE%d][RAPROC] Frame %d : Logical Channel DL-CCCH (SRB0), Received RRCConnectionSetup \n",Mod_id,frame);
	// Get configuration

	// Release T300 timer
	UE_rrc_inst[Mod_id].Info[eNB_index].T300_active=0;
	rrc_ue_process_radioResourceConfigDedicated(Mod_id,frame,eNB_index,
						    &dl_ccch_msg->message.choice.c1.choice.rrcConnectionSetup.criticalExtensions.choice.c1.choice.rrcConnectionSetup_r8.radioResourceConfigDedicated);

	rrc_ue_generate_RRCConnectionSetupComplete(Mod_id,frame, eNB_index);

	rval= 0;
	break;
      default:
	LOG_I(RRC,"[UE%d] Frame %d : Unknown message\n",Mod_id,frame);
	rval= -1;
      }
    }
  }

  return rval;
}


s32 rrc_ue_establish_srb1(u8 Mod_id,u32 frame,u8 eNB_index,
			 struct SRB_ToAddMod *SRB_config) { // add descriptor from RRC PDU

  u8 lchan_id = (eNB_index * MAX_NUM_RB) + DCCH;

  UE_rrc_inst[Mod_id].Srb1[eNB_index].Active = 1;
  UE_rrc_inst[Mod_id].Srb1[eNB_index].Status = RADIO_CONFIG_OK;//RADIO CFG
  UE_rrc_inst[Mod_id].Srb1[eNB_index].Srb_info.Srb_id = 1;

    // copy default configuration for now
  memcpy(&UE_rrc_inst[Mod_id].Srb1[eNB_index].Srb_info.Lchan_desc[0],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
  memcpy(&UE_rrc_inst[Mod_id].Srb1[eNB_index].Srb_info.Lchan_desc[1],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);


  LOG_I(RRC,"[UE %d], CONFIG_SRB1 %d corresponding to eNB_index %d\n", Mod_id,lchan_id,eNB_index);

  rrc_pdcp_config_req (Mod_id+NB_eNB_INST, frame, 0, ACTION_ADD, lchan_id);
  rrc_rlc_config_req(Mod_id+NB_eNB_INST,frame,0,ACTION_ADD,lchan_id,SIGNALLING_RADIO_BEARER,Rlc_info_am_config);
  //  UE_rrc_inst[Mod_id].Srb1[eNB_index].Srb_info.Tx_buffer.payload_size=DEFAULT_MEAS_IND_SIZE+1;


  return(0);
}

s32 rrc_ue_establish_srb2(u8 Mod_id,u32 frame,u8 eNB_index,
			 struct SRB_ToAddMod *SRB_config) { // add descriptor from RRC PDU

  u8 lchan_id = (eNB_index * MAX_NUM_RB) + DCCH1;

  UE_rrc_inst[Mod_id].Srb2[eNB_index].Active = 1;
  UE_rrc_inst[Mod_id].Srb2[eNB_index].Status = RADIO_CONFIG_OK;//RADIO CFG
  UE_rrc_inst[Mod_id].Srb2[eNB_index].Srb_info.Srb_id = 2;

    // copy default configuration for now
  memcpy(&UE_rrc_inst[Mod_id].Srb2[eNB_index].Srb_info.Lchan_desc[0],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
  memcpy(&UE_rrc_inst[Mod_id].Srb2[eNB_index].Srb_info.Lchan_desc[1],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);


  LOG_I(RRC,"[UE %d], CONFIG_SRB2 %d corresponding to eNB_index %d\n",Mod_id,lchan_id,eNB_index);

  rrc_pdcp_config_req (Mod_id+NB_eNB_INST, frame, 0, ACTION_ADD, lchan_id);
  rrc_rlc_config_req(Mod_id+NB_eNB_INST,frame,0,ACTION_ADD,lchan_id,SIGNALLING_RADIO_BEARER,Rlc_info_am_config);

  //  UE_rrc_inst[Mod_id].Srb1[eNB_index].Srb_info.Tx_buffer.payload_size=DEFAULT_MEAS_IND_SIZE+1;


  return(0);
}

//TCS LOLAmesh
s32 rrc_ue_establish_drb(u8 Mod_id,u32 frame,u8 eNB_index,struct DRB_ToAddMod *DRB_config) { // add descriptor from RRC PDU

  int oip_ifup=0,ip_addr_offset3=0,ip_addr_offset4=0;
  int logical_channel_id;
  int DRB_id;

  DRB_id = (int)DRB_config->drb_Identity - 1;

  switch (DRB_config->rlc_Config->present) {
  case RLC_Config_PR_NOTHING:
    LOG_D(RRC,"[UE] Frame %d: Received RLC_Config_PR_NOTHING!! for DRB Configuration\n",frame);
    return(-1);
    break;
  case RLC_Config_PR_um_Bi_Directional :
  	// We need to configure a collaborative DRB
  	if ((DRB_config->co_RNTI != NULL)&&(DRB_config->virtualLinkID != NULL)) {
  		logical_channel_id = ((int)*DRB_config->logicalChannelIdentity) + CO_LCID_SHIFT;
  		LOG_D(RRC,"[TCS DEBUG][UE] Frame %d: RRCConnectionReconfiguration Configuring CODRB %ld/LCID %d\n",frame,DRB_id,logical_channel_id);
			LOG_D(RRC,"[TCS DEBUG][UE %d] Frame %d: Establish RLC UM Bidirectional, CODRB %d Active\n",Mod_id,frame,DRB_id);
			//Collaborative DRB are stored at the end of the DRB list /collaborative logicalChannelIdentity = collaborative rab_id = 64 65 66 67 ...
			rrc_pdcp_config_req (Mod_id+NB_eNB_INST, frame, 0, ACTION_ADD,logical_channel_id);
			rrc_rlc_config_req(Mod_id+NB_eNB_INST,frame,0,ACTION_ADD,logical_channel_id,RADIO_ACCESS_BEARER,Rlc_info_um);
  	}
  	// We need to configure a DRB
  	else {
  		LOG_D(RRC,"[TCS DEBUG][UE] Frame %d: RRCConnectionReconfiguration Configuring DRB %ld/LCID %d\n",frame,DRB_id,(int)*DRB_config->logicalChannelIdentity);
			LOG_D(RRC,"[TCS DEBUG][UE %d] Frame %d: Establish RLC UM Bidirectional, DRB %d Active\n",Mod_id,frame,DRB_id);
			//pdcp_array size is MAX_eNB * MAX_RAB or MAX_UE * MAX_RAB where MAX_RAB = 8 (8*8 = 64), so it includes only DRBs (one UE can have 8 DRBs towards 8 eNB)
			//DRB0 starts at index 0 for UE0, 8 fort UE1...
			rrc_pdcp_config_req (Mod_id+NB_eNB_INST, frame, 0, ACTION_ADD,(eNB_index * MAX_NUM_RB) + *DRB_config->logicalChannelIdentity);
			rrc_rlc_config_req(Mod_id+NB_eNB_INST,frame,0,ACTION_ADD,(eNB_index * MAX_NUM_RB) + *DRB_config->logicalChannelIdentity,RADIO_ACCESS_BEARER,Rlc_info_um);
  	}
#ifdef NAS_NETLINK
#    ifdef OAI_EMU
    ip_addr_offset3 = oai_emulation.info.nb_enb_local;
    ip_addr_offset4 = NB_eNB_INST;
#    else  // for testing with HW: fixme
    ip_addr_offset3 = 0;
    ip_addr_offset4 = 8;
#    endif
#    ifndef NAS_DRIVER_TYPE_ETHERNET
    if (UE_rrc_inst[Mod_id].Info[eNB_index].oai_ifup == 0){
      LOG_I(OIP,"[UE %d] trying to bring up the OAI interface oai%d, IP 10.0.%d.%d\n", Mod_id, ip_addr_offset3+Mod_id,
	    ip_addr_offset3+Mod_id+1,ip_addr_offset4+Mod_id+1);
      oip_ifup=nas_config(ip_addr_offset3+Mod_id,   // interface_id
			  ip_addr_offset3+Mod_id+1, // third_octet
			  ip_addr_offset4+Mod_id+1); // fourth_octet
      UE_rrc_inst[Mod_id].Info[eNB_index].oai_ifup = ( oip_ifup == 0 ) ? 1 : 0;
    }

    if (UE_rrc_inst[Mod_id].Info[eNB_index].oai_ifup ==1 ){ // interface is up --> send a config the DRB
      
      LOG_I(OIP,"[UE %d] Config the oai%d to send/receive pkt on DRB %d to/from the protocol stack\n",
	    Mod_id,
	    ip_addr_offset3+Mod_id,
	    (eNB_index * MAX_NUM_RB) + *DRB_config->logicalChannelIdentity);
      rb_conf_ipv4(0,//add
		   Mod_id,//cx align with the UE index
		   ip_addr_offset3+Mod_id,//inst num_enb+ue_index
		   (eNB_index * MAX_NUM_RB) + *DRB_config->logicalChannelIdentity,//rb
		   0,//dscp
		   ipv4_address(ip_addr_offset3+Mod_id+1,ip_addr_offset4+Mod_id+1),//saddr
		   ipv4_address(ip_addr_offset3+Mod_id+1,eNB_index+1));//daddr
      LOG_D(RRC,"[UE %d] State = Attached (eNB %d)\n",Mod_id,eNB_index);
      UE_rrc_inst[Mod_id].Info[eNB_index].rbconfig[*DRB_config->logicalChannelIdentity]=1; 
    }
#endif
#endif
    break;
  case RLC_Config_PR_um_Uni_Directional_UL :
  case RLC_Config_PR_um_Uni_Directional_DL :
  case RLC_Config_PR_am:
    LOG_E(RRC,"[UE] Frame %d: Illegal RLC mode for DRB\n",frame);
    return(-1);
    break;
  }

  return(0);
}


void  rrc_ue_process_measConfig(u8 Mod_id,u8 eNB_index,MeasConfig_t *measConfig){

  // This is the procedure described in 36.331 Section 5.5.2.1
  int i;
  long ind;
  MeasObjectToAddMod_t *measObj;
  MeasObjectEUTRA_t *measObjd;

  if (measConfig->measObjectToRemoveList != NULL) {
    for (i=0;i<measConfig->measObjectToRemoveList->list.count;i++) {
      ind   = *measConfig->measObjectToRemoveList->list.array[i];
      free(UE_rrc_inst[Mod_id].MeasObj[eNB_index][ind]);
    }
  }
  if (measConfig->measObjectToAddModList != NULL) {
    LOG_I(RRC,"Measurement Object List is present\n");
    for (i=0;i<measConfig->measObjectToAddModList->list.count;i++) {
      measObj = measConfig->measObjectToAddModList->list.array[i];
      ind   = measConfig->measObjectToAddModList->list.array[i]->measObjectId;

      if (UE_rrc_inst[Mod_id].MeasObj[eNB_index][ind-1]) {
	LOG_I(RRC,"Modifying measurement object %d\n",ind);
	memcpy((char*)UE_rrc_inst[Mod_id].MeasObj[eNB_index][ind],
	       (char*)measObj,
	       sizeof(MeasObjectToAddMod_t));
      }
      else {
	LOG_I(RRC,"Adding measurement object %d\n",ind);
	if (measObj->measObject.present == MeasObjectToAddMod__measObject_PR_measObjectEUTRA) {
	  LOG_I(RRC,"EUTRA Measurement : carrierFreq %d, allowedMeasBandwidth %d,presenceAntennaPort1 %d, neighCellConfig %d\n",
		measObj->measObject.choice.measObjectEUTRA.carrierFreq,
		measObj->measObject.choice.measObjectEUTRA.allowedMeasBandwidth,
		measObj->measObject.choice.measObjectEUTRA.presenceAntennaPort1,
		measObj->measObject.choice.measObjectEUTRA.neighCellConfig.buf[0]);
	  UE_rrc_inst[Mod_id].MeasObj[eNB_index][ind-1]=measObj;


	}
      }
    }
    rrc_mac_config_req(Mod_id,0,0,eNB_index,
		       (RadioResourceConfigCommonSIB_t *)NULL,
		       (struct PhysicalConfigDedicated *)NULL,
		       UE_rrc_inst[Mod_id].MeasObj[eNB_index],
		       (MAC_MainConfig_t *)NULL,
		       0,
		       (struct LogicalChannelConfig *)NULL,
		       (MeasGapConfig_t *)NULL,
		       (TDD_Config_t *)NULL,
		       NULL,
		       NULL);
  }
  if (measConfig->reportConfigToRemoveList != NULL) {
    for (i=0;i<measConfig->reportConfigToRemoveList->list.count;i++) {
      ind   = *measConfig->reportConfigToRemoveList->list.array[i];
      free(UE_rrc_inst[Mod_id].ReportConfig[eNB_index][ind]);
    }
  }
  if (measConfig->reportConfigToAddModList != NULL) {
    LOG_I(RRC,"Report Configuration List is present\n");
    for (i=0;i<measConfig->reportConfigToAddModList->list.count;i++) {
      ind   = measConfig->reportConfigToAddModList->list.array[i]->reportConfigId;
      if (UE_rrc_inst[Mod_id].ReportConfig[eNB_index][ind-1]) {
	LOG_I(RRC,"Modifying Report Configuration %d\n",ind);
	memcpy((char*)UE_rrc_inst[Mod_id].ReportConfig[eNB_index][ind-1],
	       (char*)measConfig->reportConfigToAddModList->list.array[i],
	       sizeof(ReportConfigToAddMod_t));
      }
      else {
	LOG_I(RRC,"Adding Report Configuration %d\n",ind);
	UE_rrc_inst[Mod_id].ReportConfig[eNB_index][ind] = measConfig->reportConfigToAddModList->list.array[i];
      }
    }
  }

  if (measConfig->quantityConfig != NULL) {
    if (UE_rrc_inst[Mod_id].QuantityConfig[eNB_index]) {
      LOG_I(RRC,"Modifying Quantity Configuration \n");
      memcpy((char*)UE_rrc_inst[Mod_id].QuantityConfig[eNB_index],(char*)measConfig->quantityConfig,
	     sizeof(QuantityConfig_t));
    }
    else {
      LOG_I(RRC,"Adding Quantity configuration\n");
      UE_rrc_inst[Mod_id].QuantityConfig[eNB_index] = measConfig->quantityConfig;
    }
  }

  if (measConfig->measIdToRemoveList != NULL) {
    for (i=0;i<measConfig->measIdToRemoveList->list.count;i++) {
      ind   = *measConfig->measIdToRemoveList->list.array[i];
      free(UE_rrc_inst[Mod_id].MeasId[eNB_index][ind]);
    }
  }

  if (measConfig->measIdToAddModList != NULL) {
    for (i=0;i<measConfig->measIdToAddModList->list.count;i++) {
      ind   = measConfig->measIdToAddModList->list.array[i]->measId;
      if (UE_rrc_inst[Mod_id].MeasId[eNB_index][ind]) {
	LOG_I(RRC,"Modifying Measurement ID %d\n",ind);
	memcpy((char*)UE_rrc_inst[Mod_id].MeasId[eNB_index][ind-1],
	       (char*)measConfig->measIdToAddModList->list.array[i],
	       sizeof(MeasIdToAddMod_t));
      }
      else {
	LOG_I(RRC,"Adding Measurement ID %d\n",ind);
	UE_rrc_inst[Mod_id].MeasId[eNB_index][ind] = measConfig->measIdToAddModList->list.array[i];
      }
    }
  }

  if (measConfig->measGapConfig !=NULL) {
    if (UE_rrc_inst[Mod_id].measGapConfig[eNB_index]) {
      memcpy((char*)UE_rrc_inst[Mod_id].measGapConfig[eNB_index],(char*)measConfig->measGapConfig,
	     sizeof(MeasGapConfig_t));
    }
    else {
      UE_rrc_inst[Mod_id].measGapConfig[eNB_index] = measConfig->measGapConfig;
    }
  }

  if (measConfig->s_Measure != NULL) {
    UE_rrc_inst[Mod_id].s_measure = *measConfig->s_Measure;
  }

}


int	rrc_ue_process_radioResourceConfigDedicated(u8 Mod_id,u32 frame, u8 eNB_index,RadioResourceConfigDedicated_t *radioResourceConfigDedicated) {

  long SRB_id,DRB_id;
  int i,cnt;
  //TCS LOLAmesh
	u8 vlid;
	u16 cornti;
	u8 nb_corntis;
	int ret;
	int collaborative_link = 0;
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
      if (UE_rrc_inst[Mod_id].mac_MainConfig[eNB_index]) {
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
      LOG_D(RRC,"[UE %d]: Frame %d SRB config cnt %d (SRB%ld)\n",Mod_id,frame,cnt,SRB_id);
      if (SRB_id == 1) {
	if (UE_rrc_inst[Mod_id].SRB1_config[eNB_index]) {
	  memcpy(UE_rrc_inst[Mod_id].SRB1_config[eNB_index],radioResourceConfigDedicated->srb_ToAddModList->list.array[cnt],
		 sizeof(struct SRB_ToAddMod));
	}
	else {
	  UE_rrc_inst[Mod_id].SRB1_config[eNB_index] = radioResourceConfigDedicated->srb_ToAddModList->list.array[cnt];

	  rrc_ue_establish_srb1(Mod_id,frame,eNB_index,radioResourceConfigDedicated->srb_ToAddModList->list.array[cnt]);
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


      LOG_D(RRC, "[MSC_MSG][FRAME %05d][RRC_UE][MOD %02d][][--- MAC_CONFIG_REQ  (SRB1 eNB %d) --->][MAC_UE][MOD %02d][]\n",
            frame, Mod_id, eNB_index, Mod_id);
	  rrc_mac_config_req(Mod_id,0,0,eNB_index,
			     (RadioResourceConfigCommonSIB_t *)NULL,
			     UE_rrc_inst[Mod_id].physicalConfigDedicated[eNB_index],
			     (MeasObjectToAddMod_t **)NULL,
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

	  rrc_ue_establish_srb2(Mod_id,frame,eNB_index,radioResourceConfigDedicated->srb_ToAddModList->list.array[cnt]);
	  if (UE_rrc_inst[Mod_id].SRB2_config[eNB_index]->logicalChannelConfig) {
	    if (UE_rrc_inst[Mod_id].SRB2_config[eNB_index]->logicalChannelConfig->present == SRB_ToAddMod__logicalChannelConfig_PR_explicitValue){
	      LOG_I(RRC,"Applying Explicit SRB2 logicalChannelConfig\n");
	      SRB2_logicalChannelConfig = &UE_rrc_inst[Mod_id].SRB2_config[eNB_index]->logicalChannelConfig->choice.explicitValue;
	    }
	    else {
	      LOG_I(RRC,"Applying default SRB2 logicalChannelConfig\n");
	      SRB2_logicalChannelConfig = &SRB2_logicalChannelConfig_defaultValue;
	    }
	  }
	  else {
	    SRB2_logicalChannelConfig = &SRB2_logicalChannelConfig_defaultValue;
	  }

      LOG_D(RRC, "[MSC_MSG][FRAME %05d][RRC_UE][MOD %02d][][--- MAC_CONFIG_REQ  (SRB2 eNB %d) --->][MAC_UE][MOD %02d][]\n",
            frame, Mod_id, eNB_index, Mod_id);
      rrc_mac_config_req(Mod_id,0,0,eNB_index,
			 (RadioResourceConfigCommonSIB_t *)NULL,
			 UE_rrc_inst[Mod_id].physicalConfigDedicated[eNB_index],
			 (MeasObjectToAddMod_t **)NULL,
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

      // We need to configure a collaborative DRB
      if ((radioResourceConfigDedicated->drb_ToAddModList->list.array[i]->co_RNTI != NULL)&&(radioResourceConfigDedicated->drb_ToAddModList->list.array[i]->virtualLinkID != NULL)) {

      	//Store the CODRB configuration
      	UE_rrc_inst[Mod_id].CODRB_config[DRB_id] = radioResourceConfigDedicated->drb_ToAddModList->list.array[i];

      	//Establish the collaborative drb
      	rrc_ue_establish_drb(Mod_id,frame,eNB_index,radioResourceConfigDedicated->drb_ToAddModList->list.array[i]);

      	// MAC/PHY Configuration
      	LOG_D(RRC, "[MSC_MSG][FRAME %05d][RRC_UE][MOD %02d][][--- MAC_CONFIG_REQ (CODRB %d eNB %d) --->][MAC_UE][MOD %02d][]\n",frame, Mod_id, DRB_id, eNB_index, Mod_id);
      	rrc_mac_config_req(Mod_id,0,0,eNB_index,(RadioResourceConfigCommonSIB_t *)NULL,UE_rrc_inst[Mod_id].physicalConfigDedicated[eNB_index],(MeasObjectToAddMod_t **)NULL,UE_rrc_inst[Mod_id].mac_MainConfig[eNB_index],*UE_rrc_inst[Mod_id].CODRB_config[DRB_id]->logicalChannelIdentity,UE_rrc_inst[Mod_id].CODRB_config[DRB_id]->logicalChannelConfig,UE_rrc_inst[Mod_id].measGapConfig[eNB_index],(TDD_Config_t*)NULL,(u8 *)NULL,(u16 *)NULL);

    	  // MAC/PHY Configuration
    	  cornti = (u16)*radioResourceConfigDedicated->drb_ToAddModList->list.array[i]->co_RNTI;
    	  vlid = (u8)*radioResourceConfigDedicated->drb_ToAddModList->list.array[i]->virtualLinkID;
    	  LOG_D(RRC, "[MSC_MSG][FRAME %05d][RRC_UE][MOD %02d][--- MAC_CONFIG_CO_REQ (CODRB %d eNB %d) --->][MAC_UE][MOD %02d][] Configuring DRB %d for collabortaive communications with CORNTI = %u and VLID = %u\n",frame,Mod_id, DRB_id, eNB_index, Mod_id, DRB_id, cornti, vlid);

    	  /* Configure the forwarding table with the given vlid and cornti */
    	  ret=rrc_mac_config_co_req(Mod_id,eNB_index,cornti,vlid);
    	  if (ret < 0) {
    	    LOG_D(RRC, "[TCS DEBUG][MSC_MSG][FRAME %05d][RRC_UE][MOD %02d][CODRB %d eNB %d][MAC_UE][MOD %02d] MAC layer forwarding table configuration failed\n",frame,Mod_id, DRB_id, eNB_index, Mod_id);
    	  } else {
    	    LOG_D(RRC, "[TCS DEBUG][MSC_MSG][FRAME %05d][RRC_UE][MOD %02d][CODRB %d eNB %d][MAC_UE][MOD %02d] MAC layer forwarding table configuration succeeded\n",frame,Mod_id, DRB_id, eNB_index, Mod_id);
    	  }
    	  //UE_rrc_inst[Mod_id].State_CoLink[vlid]= RRC_RECONFIGURED;
    	  //LOG_D(RRC,"[TCS DEBUG][UE %d] State = RRC_RECONFIGURED for vlid %u (eNB %d)\n",Mod_id,eNB_index,vlid);
    	  collaborative_link = 1;

    	  /* Keep track of the CORNTI */
    	  //MAC layer structures
    	  nb_corntis = UE_mac_inst[Mod_id].corntis.count;
    	  UE_mac_inst[Mod_id].corntis.array[nb_corntis] = cornti;
    	  UE_mac_inst[Mod_id].corntis.count++;
    	  //PHY layer structures
    	  nb_corntis = PHY_vars_UE_g[Mod_id]->dlsch_ue[eNB_index][0]->corntis.count;
    	  PHY_vars_UE_g[Mod_id]->dlsch_ue[eNB_index][0]->corntis.array[nb_corntis] = cornti;
    	  PHY_vars_UE_g[Mod_id]->dlsch_ue[eNB_index][0]->corntis.count++;

      }// end if ((radioResourceConfigDedicated->drb_ToAddModList->list.array[i]->co_RNTI != NULL)&&(radioResourceConfigDedicated->drb_ToAddModList->list.array[i]->virtualLinkID != NULL))

      // We need to configure a DRB
      else {

        if (UE_rrc_inst[Mod_id].DRB_config[eNB_index][DRB_id]) {
        	memcpy(UE_rrc_inst[Mod_id].DRB_config[eNB_index][DRB_id],radioResourceConfigDedicated->drb_ToAddModList->list.array[i],sizeof(struct DRB_ToAddMod));
        }

        else {

        	UE_rrc_inst[Mod_id].DRB_config[eNB_index][DRB_id] = radioResourceConfigDedicated->drb_ToAddModList->list.array[i];

        	rrc_ue_establish_drb(Mod_id,frame,eNB_index,radioResourceConfigDedicated->drb_ToAddModList->list.array[i]);

  				// MAC/PHY Configuration
  				LOG_D(RRC, "[MSC_MSG][FRAME %05d][RRC_UE][MOD %02d][][--- MAC_CONFIG_REQ (DRB %d eNB %d) --->][MAC_UE][MOD %02d][]\n",frame, Mod_id, DRB_id, eNB_index, Mod_id);
  				rrc_mac_config_req(Mod_id,0,0,eNB_index,(RadioResourceConfigCommonSIB_t *)NULL,UE_rrc_inst[Mod_id].physicalConfigDedicated[eNB_index],(MeasObjectToAddMod_t **)NULL,UE_rrc_inst[Mod_id].mac_MainConfig[eNB_index],*UE_rrc_inst[Mod_id].DRB_config[eNB_index][DRB_id]->logicalChannelIdentity,UE_rrc_inst[Mod_id].DRB_config[eNB_index][DRB_id]->logicalChannelConfig,UE_rrc_inst[Mod_id].measGapConfig[eNB_index],(TDD_Config_t*)NULL,(u8 *)NULL,(u16 *)NULL);
        }

      }// end else

    }//end for (i=0;i<radioResourceConfigDedicated->drb_ToAddModList->list.count;i++)

  }//end if (radioResourceConfigDedicated->drb_ToAddModList)
  
  //TCS LOLAmesh
  if (collaborative_link == 0) {
  	UE_rrc_inst[Mod_id].Info[eNB_index].State = RRC_CONNECTED;
  	LOG_D(RRC,"[UE %d] State = RRC_CONNECTED (eNB %d)\n",Mod_id,eNB_index);
  	ret = 0;
  }
  //If this is a collaborative link, we return the id of the link
  else {
  	ret = vlid;
  }

  return ret;

}


int rrc_ue_process_rrcConnectionReconfiguration(u8 Mod_id, u32 frame,
						 RRCConnectionReconfiguration_t *rrcConnectionReconfiguration,
						 u8 eNB_index) {

  int ret = 0;

	LOG_I(RRC,"[UE %d] Frame %d: Receiving from SRB1 (DL-DCCH), Processing RRCConnectionReconfiguration (eNB %d)\n",
	Mod_id,frame,eNB_index);
  if (rrcConnectionReconfiguration->criticalExtensions.present == RRCConnectionReconfiguration__criticalExtensions_PR_c1) {
    if (rrcConnectionReconfiguration->criticalExtensions.choice.c1.present == RRCConnectionReconfiguration__criticalExtensions__c1_PR_rrcConnectionReconfiguration_r8) {

      if (rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.mobilityControlInfo) {
      	LOG_I(RRC,"Mobility Control Information is present\n");
      	rrc_ue_process_mobilityControlInfo(Mod_id,eNB_index,rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.mobilityControlInfo);
      }
      if (rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig != NULL) {
      	LOG_I(RRC,"Measurement Configuration is present\n");
      	rrc_ue_process_measConfig(Mod_id,eNB_index,rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig);
      }
      if (rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated) {
      	LOG_I(RRC,"Radio Resource Configuration is present\n");
      	ret = rrc_ue_process_radioResourceConfigDedicated(Mod_id,frame,eNB_index,rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated);

      }
    } // c1 present
  } // critical extensions present

  return ret;
}

void	rrc_ue_process_mobilityControlInfo(u8 Mod_id,u8 eNB_index,struct MobilityControlInfo *mobilityControlInfo) {


}

/*------------------------------------------------------------------------------------------*/
void  rrc_ue_decode_dcch(u8 Mod_id,u32 frame,u8 Srb_id, u8 *Buffer,u8 eNB_index){
  /*------------------------------------------------------------------------------------------*/

  //DL_DCCH_Message_t dldcchmsg;
  DL_DCCH_Message_t *dl_dcch_msg=NULL;//&dldcchmsg;
  //  asn_dec_rval_t dec_rval;
  int i;
  int ret;

  if (Srb_id != 1) {
    LOG_D(RRC,"[UE %d] Frame %d: Received message on DL-DCCH (SRB1), should not have ...\n",Mod_id,frame);
    return;
  }

  //memset(dl_dcch_msg,0,sizeof(DL_DCCH_Message_t));

  // decode messages
  //  LOG_D(RRC,"[UE %d] Decoding DL-DCCH message\n",Mod_id);
  /*
  for (i=0;i<30;i++)
    LOG_T(RRC,"%x.",Buffer[i]);
  LOG_T(RRC, "\n");
  */
  uper_decode(NULL,
	      &asn_DEF_DL_DCCH_Message,
	      (void**)&dl_dcch_msg,
	      (uint8_t*)Buffer,
	      100,0,0);

  xer_fprint(stdout,&asn_DEF_DL_DCCH_Message,(void*)dl_dcch_msg);

  if (dl_dcch_msg->message.present == DL_DCCH_MessageType_PR_c1) {

    if ((UE_rrc_inst[Mod_id].Info[eNB_index].State == RRC_CONNECTED)||(UE_rrc_inst[Mod_id].Info[eNB_index].State == RRC_RECONFIGURED)) {

      switch (dl_dcch_msg->message.choice.c1.present) {

      case DL_DCCH_MessageType__c1_PR_NOTHING :
	LOG_I(RRC,"[UE %d] Frame %d : Received PR_NOTHING on DL-DCCH-Message\n",Mod_id,frame);
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
				ret = rrc_ue_process_rrcConnectionReconfiguration(Mod_id,frame,&dl_dcch_msg->message.choice.c1.choice.rrcConnectionReconfiguration,eNB_index);
				//TCS LOLAmesh
				// If this is not a cooperative link ret == 0
				if (ret == 0) {
					rrc_ue_generate_RRCConnectionReconfigurationComplete(Mod_id,frame,eNB_index,0,0);
					UE_rrc_inst[Mod_id].Info[eNB_index].State = RRC_RECONFIGURED;
					LOG_D(RRC,"[UE %d] State = RRC_RECONFIGURED (eNB %d)\n",Mod_id,eNB_index);
				}
				// If this is a cooperative link ret == vlid
				else {
					rrc_ue_generate_RRCConnectionReconfigurationComplete(Mod_id,frame,eNB_index,1,ret);
					UE_rrc_inst[Mod_id].State_CoLink[ret] = RRC_RECONFIGURED;
					LOG_D(RRC,"[TCS DEBUG][UE %d] State = RRC_RECONFIGURED for vlid %u (eNB %d)\n",Mod_id,ret,eNB_index);
				}
	break;
      case DL_DCCH_MessageType__c1_PR_rrcConnectionRelease:
	break;
      case DL_DCCH_MessageType__c1_PR_securityModeCommand:
	break;
      case DL_DCCH_MessageType__c1_PR_ueCapabilityEnquiry:
	break;
      case DL_DCCH_MessageType__c1_PR_counterCheck:
	break;
#ifdef Rel10
      case DL_DCCH_MessageType__c1_PR_ueInformationRequest_r9:
	break;
      case DL_DCCH_MessageType__c1_PR_loggedMeasurementConfiguration_r10:
	break;
      case DL_DCCH_MessageType__c1_PR_rnReconfiguration_r10:
	break;
#endif
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

const char SIBType[16][6] ={"SIB3\0","SIB4\0","SIB5\0","SIB6\0","SIB7\0","SIB8\0","SIB9\0","SIB10\0","SIB11\0","SIB12\0","SIB13\0","Sp2\0","Sp3\0","Sp4\0"};
const char SIBPeriod[7][7]= {"80ms\0","160ms\0","320ms\0","640ms\0","1280ms\0","2560ms\0","5120ms\0"};
const char siPeriod_int[7] = {80,160,320,640,1280,2560,5120};

int decode_BCCH_DLSCH_Message(u8 Mod_id,u32 frame,u8 eNB_index,u8 *Sdu,u8 Sdu_len) {

  //  printf("Mod_id=%d, frame=%d, eNB_index=%d, sdu_len=%d, SI_period=%d, SI_WindowSize=%d\n",Mod_id,frame,eNB_index,Sdu_len,
  //	 UE_rrc_inst[Mod_id].Info[eNB_index].SIperiod,
  //	 UE_rrc_inst[Mod_id].Info[eNB_index].SIwindowsize);

  //BCCH_DL_SCH_Message_t bcch_message;
  BCCH_DL_SCH_Message_t *bcch_message=NULL;//_ptr=&bcch_message;
  SystemInformationBlockType1_t **sib1=&UE_rrc_inst[Mod_id].sib1[eNB_index];
  SystemInformation_t **si= UE_rrc_inst[Mod_id].si[eNB_index];
  asn_dec_rval_t dec_rval;
  uint32_t si_window;//, sib1_decoded=0, si_decoded=0;

  if ((UE_rrc_inst[Mod_id].Info[eNB_index].SIB1Status == 1) &&
      (UE_rrc_inst[Mod_id].Info[eNB_index].SIStatus == 1)) {
    // Avoid decoding to prevent memory bloating
    return 0;
  } 
  else {

    //memset(&bcch_message,0,sizeof(BCCH_DL_SCH_Message_t));
    //  LOG_D(RRC,"[UE %d] Decoding DL_BCCH_DLSCH_Message\n",Mod_id)
    dec_rval = uper_decode_complete(NULL,
				    &asn_DEF_BCCH_DL_SCH_Message,
				    (void **)&bcch_message,
				    (const void *)Sdu,
				    Sdu_len);//,0,0);
    
    if ((dec_rval.code != RC_OK) && (dec_rval.consumed==0)) {
      LOG_E(RRC,"[UE %d] Failed to decode BCCH_DLSCH_MESSAGE (%d bits)\n",Mod_id,dec_rval.consumed);
      //free the memory
      SEQUENCE_free(&asn_DEF_BCCH_DL_SCH_Message, (void*)bcch_message, 1);
      return -1;
    }
    //  xer_fprint(stdout,  &asn_DEF_BCCH_DL_SCH_Message, (void*)&bcch_message);
    
    if (bcch_message->message.present == BCCH_DL_SCH_MessageType_PR_c1) {
      switch (bcch_message->message.choice.c1.present) {
      case BCCH_DL_SCH_MessageType__c1_PR_systemInformationBlockType1:
	if ((frame %2) == 0) {
	  if (UE_rrc_inst[Mod_id].Info[eNB_index].SIB1Status == 0) {
	    memcpy((void*)*sib1,
		   (void*)&bcch_message->message.choice.c1.choice.systemInformationBlockType1,
		   sizeof(SystemInformationBlockType1_t));
	    LOG_D(RRC,"[UE %d] Decoding First SIB1 from eNB %d \n",Mod_id, eNB_index);
	    decode_SIB1(Mod_id,eNB_index);
	  }
	}
	break;
      case BCCH_DL_SCH_MessageType__c1_PR_systemInformation:
	if ((UE_rrc_inst[Mod_id].Info[eNB_index].SIB1Status == 1) &&
	    (UE_rrc_inst[Mod_id].Info[eNB_index].SIStatus == 0)) {
	  if ((frame %8) == 1) {  // check only in odd frames for SI
	    si_window = (frame%(UE_rrc_inst[Mod_id].Info[eNB_index].SIperiod/10))/(UE_rrc_inst[Mod_id].Info[eNB_index].SIwindowsize/10);
	    memcpy((void*)si[si_window],
		   (void*)&bcch_message->message.choice.c1.choice.systemInformation,
		   sizeof(SystemInformation_t));
	    LOG_D(RRC,"[UE %d] Decoding SI from eNB %d for frame %d, si_window %d\n",Mod_id,eNB_index, frame,si_window);
	    decode_SI(Mod_id,frame,eNB_index,si_window);
	  }
	}
	break;
      case BCCH_DL_SCH_MessageType__c1_PR_NOTHING:
	break;
      }
    }
  }
  /*  if ((UE_rrc_inst[Mod_id].Info[eNB_index].SIB1Status == 1) &&
      (UE_rrc_inst[Mod_id].Info[eNB_index].SIStatus == 1) && (frame >= Mod_id * 20 + 10))
      SEQUENCE_free(&asn_DEF_BCCH_DL_SCH_Message, (void*)bcch_message, 0);*/
}



int decode_SIB1(u8 Mod_id,u8 eNB_index) {
  asn_dec_rval_t dec_rval;
  SystemInformationBlockType1_t **sib1=&UE_rrc_inst[Mod_id].sib1[eNB_index];
  int i;
  int siWindowLength_index, siPeriod_index, siType_index;

  LOG_D(RRC,"[UE %d] : Dumping SIB 1\n",Mod_id);

  //  xer_fprint(stdout,&asn_DEF_SystemInformationBlockType1, (void*)*sib1);

  msg("cellAccessRelatedInfo.cellIdentity : %x.%x.%x.%x\n",
      (*sib1)->cellAccessRelatedInfo.cellIdentity.buf[0],
      (*sib1)->cellAccessRelatedInfo.cellIdentity.buf[1],
      (*sib1)->cellAccessRelatedInfo.cellIdentity.buf[2],
      (*sib1)->cellAccessRelatedInfo.cellIdentity.buf[3]);
  siWindowLength_index= (*sib1)->si_WindowLength;
  msg("cellSelectionInfo.q_RxLevMin       : %d\n",(int)(*sib1)->cellSelectionInfo.q_RxLevMin);
  msg("freqBandIndicator                  : %d\n",(int)(*sib1)->freqBandIndicator);
  msg("siWindowLength                     : %s\n",siWindowLength[siWindowLength_index]);
  if ((*sib1)->schedulingInfoList.list.count>0) {
    for (i=0;i<(*sib1)->schedulingInfoList.list.count;i++) {
      siPeriod_index = (int)(*sib1)->schedulingInfoList.list.array[i]->si_Periodicity;
      msg("siSchedulingInfoPeriod[%d]          : %s\n",i,SIBPeriod[siPeriod_index ]);
      if ((*sib1)->schedulingInfoList.list.array[i]->sib_MappingInfo.list.count>0){
	siType_index= (int)(*(*sib1)->schedulingInfoList.list.array[i]->sib_MappingInfo.list.array[0]);
	msg("siSchedulingInfoSIBType[%d]         : %s\n",i,SIBType[siType_index]);
      }else {
	msg("mapping list %d is null\n",i);
      }
    }
  }
  else {
    msg("siSchedulingInfoPeriod[0]          : PROBLEM!!!\n");
   return -1;
  }

  if ((*sib1)->tdd_Config)
    msg("TDD subframe assignment            : %d\nS-Subframe Config                  : %d\n",(int)(*sib1)->tdd_Config->subframeAssignment,(int)(*sib1)->tdd_Config->specialSubframePatterns);
  // msg("SIperiod index %d value %d\n", (*sib1)->schedulingInfoList.list.array[i]->si_Periodicity, siPeriod_int[(*sib1)->schedulingInfoList.list.array[i]->si_Periodicity]);

  UE_rrc_inst[Mod_id].Info[eNB_index].SIperiod     = siPeriod_int[siPeriod_index];
  UE_rrc_inst[Mod_id].Info[eNB_index].SIwindowsize = siWindowLength_int[siWindowLength_index];
  LOG_D(RRC, "[MSC_MSG][FRAME unknown][RRC_UE][MOD %02d][][--- MAC_CONFIG_REQ (SIB1 params eNB %d) --->][MAC_UE][MOD %02d][]\n",
             Mod_id, eNB_index, Mod_id);
  rrc_mac_config_req(Mod_id,0,0,eNB_index,
		     (RadioResourceConfigCommonSIB_t *)NULL,
		     (struct PhysicalConfigDedicated *)NULL,
		     (MeasObjectToAddMod_t **)NULL,
		     (MAC_MainConfig_t *)NULL,
		     0,
		     (struct LogicalChannelConfig *)NULL,
		     (MeasGapConfig_t *)NULL,
		     UE_rrc_inst[Mod_id].sib1[eNB_index]->tdd_Config,
		     &UE_rrc_inst[Mod_id].Info[eNB_index].SIwindowsize,
		     &UE_rrc_inst[Mod_id].Info[eNB_index].SIperiod);

  UE_rrc_inst[Mod_id].Info[eNB_index].SIB1Status = 1;
  return 0;

}


void dump_sib2(SystemInformationBlockType2_t *sib2) {

  LOG_D(RRC,"radioResourceConfigCommon.rach_ConfigCommon.preambleInfo.numberOfRA_Preambles : %ld\n",
      sib2->radioResourceConfigCommon.rach_ConfigCommon.preambleInfo.numberOfRA_Preambles);

  //  if (radioResourceConfigCommon.rach_ConfigCommon.preambleInfo.preamblesGroupAConfig)
  //msg("radioResourceConfigCommon.rach_ConfigCommon.preambleInfo.preamblesGroupAConfig ",sib2->radioResourceConfigCommon.rach_ConfigCommon.preambleInfo.preamblesGroupAConfig = NULL;

  LOG_D(RRC,"[UE]radioResourceConfigCommon.rach_ConfigCommon.powerRampingParameters.powerRampingStep : %ld\n",sib2->radioResourceConfigCommon.rach_ConfigCommon.powerRampingParameters.powerRampingStep);

  LOG_D(RRC,"[UE]radioResourceConfigCommon.rach_ConfigCommon.powerRampingParameters.preambleInitialReceivedTargetPower : %ld\n",sib2->radioResourceConfigCommon.rach_ConfigCommon.powerRampingParameters.preambleInitialReceivedTargetPower);

  LOG_D(RRC,"radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.preambleTransMax  : %ld\n",sib2->radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.preambleTransMax);

  LOG_D(RRC,"radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.ra_ResponseWindowSize : %ld\n",sib2->radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.ra_ResponseWindowSize);

  LOG_D(RRC,"radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.mac_ContentionResolutionTimer : %ld\n",sib2->radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.mac_ContentionResolutionTimer);

  LOG_D(RRC,"radioResourceConfigCommon.rach_ConfigCommon.maxHARQ_Msg3Tx : %ld\n",
      sib2->radioResourceConfigCommon.rach_ConfigCommon.maxHARQ_Msg3Tx);

  LOG_D(RRC,"radioResourceConfigCommon.prach_Config.rootSequenceIndex : %ld\n",sib2->radioResourceConfigCommon.prach_Config.rootSequenceIndex);
  LOG_D(RRC,"radioResourceConfigCommon.prach_Config.prach_ConfigInfo.prach_ConfigIndex : %ld\n",sib2->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.prach_ConfigIndex);
  LOG_D(RRC,"radioResourceConfigCommon.prach_Config.prach_ConfigInfo.highSpeedFlag : %d\n",  (int)sib2->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.highSpeedFlag);
  LOG_D(RRC,"radioResourceConfigCommon.prach_Config.prach_ConfigInfo.zeroCorrelationZoneConfig : %ld\n",  sib2->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.zeroCorrelationZoneConfig);
  LOG_D(RRC,"radioResourceConfigCommon.prach_Config.prach_ConfigInfo.prach_FreqOffset %ld\n",  sib2->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.prach_FreqOffset);

  // PDSCH-Config
  LOG_D(RRC,"radioResourceConfigCommon.pdsch_ConfigCommon.referenceSignalPower  : %ld\n",sib2->radioResourceConfigCommon.pdsch_ConfigCommon.referenceSignalPower);
  LOG_D(RRC,"radioResourceConfigCommon.pdsch_ConfigCommon.p_b : %ld\n",sib2->radioResourceConfigCommon.pdsch_ConfigCommon.p_b);

  // PUSCH-Config
  LOG_D(RRC,"radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.n_SB  : %ld\n",sib2->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.n_SB);
  LOG_D(RRC,"radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.hoppingMode  : %ld\n",sib2->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.hoppingMode);
  LOG_D(RRC,"radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.pusch_HoppingOffset : %ld\n",sib2->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.pusch_HoppingOffset);
  LOG_D(RRC,"radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.enable64QAM : %d\n",(int)sib2->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.enable64QAM);
  LOG_D(RRC,"radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupHoppingEnabled : %d\n",(int)sib2->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupHoppingEnabled);
  LOG_D(RRC,"radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH : %ld\n",sib2->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH);
  LOG_D(RRC,"radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled : %d\n",(int)sib2->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled);
  LOG_D(RRC,"radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.cyclicShift : %ld\n",sib2->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.cyclicShift);

  // PUCCH-Config

  LOG_D(RRC,"radioResourceConfigCommon.pucch_ConfigCommon.deltaPUCCH_Shift : %ld\n",sib2->radioResourceConfigCommon.pucch_ConfigCommon.deltaPUCCH_Shift);
  LOG_D(RRC,"radioResourceConfigCommon.pucch_ConfigCommon.nRB_CQI : %ld\n",sib2->radioResourceConfigCommon.pucch_ConfigCommon.nRB_CQI);
  LOG_D(RRC,"radioResourceConfigCommon.pucch_ConfigCommon.nCS_AN : %ld\n",sib2->radioResourceConfigCommon.pucch_ConfigCommon.nCS_AN);
  LOG_D(RRC,"radioResourceConfigCommon.pucch_ConfigCommon.n1PUCCH_AN : %ld\n",sib2->radioResourceConfigCommon.pucch_ConfigCommon.n1PUCCH_AN);

  LOG_D(RRC,"radioResourceConfigCommon.soundingRS_UL_ConfigCommon.present : %d\n",sib2-> radioResourceConfigCommon.soundingRS_UL_ConfigCommon.present);


  // uplinkPowerControlCommon

  LOG_D(RRC,"radioResourceConfigCommon.uplinkPowerControlCommon.p0_NominalPUSCH : %ld\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.p0_NominalPUSCH);
  LOG_D(RRC,"radioResourceConfigCommon.uplinkPowerControlCommon.alpha : %ld\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.alpha);

  LOG_D(RRC,"radioResourceConfigCommon.uplinkPowerControlCommon.p0_NominalPUCCH : %ld\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.p0_NominalPUCCH);
  LOG_D(RRC,"radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format1 : %ld\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format1);
  LOG_D(RRC,"radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format1b :%ld\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format1b);
  LOG_D(RRC,"radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2  :%ld\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2);
  LOG_D(RRC,"radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2a :%ld\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2a);
  LOG_D(RRC,"radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2b :%ld\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2b);

  LOG_D(RRC,"radioResourceConfigCommon.uplinkPowerControlCommon.deltaPreambleMsg3 : %ld\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.deltaPreambleMsg3);

  LOG_D(RRC,"radioResourceConfigCommon.ul_CyclicPrefixLength : %ld\n", sib2->radioResourceConfigCommon.ul_CyclicPrefixLength);

  LOG_D(RRC,"ue_TimersAndConstants.t300 : %ld\n", sib2->ue_TimersAndConstants.t300);
  LOG_D(RRC,"ue_TimersAndConstants.t301 : %ld\n", sib2->ue_TimersAndConstants.t301);
  LOG_D(RRC,"ue_TimersAndConstants.t310 : %ld\n", sib2->ue_TimersAndConstants.t310);
  LOG_D(RRC,"ue_TimersAndConstants.n310 : %ld\n", sib2->ue_TimersAndConstants.n310);
  LOG_D(RRC,"ue_TimersAndConstants.t311 : %ld\n", sib2->ue_TimersAndConstants.t311);
  LOG_D(RRC,"ue_TimersAndConstants.n311 : %ld\n", sib2->ue_TimersAndConstants.n311);

  LOG_D(RRC,"freqInfo.additionalSpectrumEmission : %ld\n",sib2->freqInfo.additionalSpectrumEmission);
  LOG_D(RRC,"freqInfo.ul_CarrierFreq : %d\n",(int)sib2->freqInfo.ul_CarrierFreq);
  LOG_D(RRC,"freqInfo.ul_Bandwidth : %d\n",(int)sib2->freqInfo.ul_Bandwidth);
  LOG_D(RRC,"mbsfn_SubframeConfigList : %d\n",(int)sib2->mbsfn_SubframeConfigList);
  LOG_D(RRC,"timeAlignmentTimerCommon : %ld\n",sib2->timeAlignmentTimerCommon);



}

void dump_sib3(SystemInformationBlockType3_t *sib3) {

}

#ifdef Rel10
void dump_sib13(SystemInformationBlockType13_r9_t *sib13) {

  LOG_D(RRC,"[RRC][UE] Dumping SIB13\n");
  LOG_D(RRC,"[RRC][UE] dumping sib13 second time\n");
  LOG_D(RRC,"NotificationRepetitionCoeff-r9 : %ld\n", sib13->notificationConfig_r9.notificationRepetitionCoeff_r9);
  LOG_D(RRC,"NotificationOffset-r9 : %d\n", (int)sib13->notificationConfig_r9.notificationOffset_r9);
  LOG_D(RRC,"notificationSF-Index-r9 : %d\n", (int)sib13->notificationConfig_r9.notificationSF_Index_r9);

}
#endif
//const char SIBPeriod[7][7]= {"80ms\0","160ms\0","320ms\0","640ms\0","1280ms\0","2560ms\0","5120ms\0"};
int decode_SI(u8 Mod_id,u32 frame,u8 eNB_index,u8 si_window) {

  SystemInformation_t **si=&UE_rrc_inst[Mod_id].si[eNB_index][si_window];
  int i;
  struct SystemInformation_r8_IEs__sib_TypeAndInfo__Member *typeandinfo;

  /*
  LOG_D(RRC,"[UE %d] Frame %d : Dumping SI from window %d (%d bytes)\n",Mod_id,Mac_rlc_xface->frame,si_window,dec_rval.consumed);
  for (i=0;i<30;i++)
    LOG_D(RRC,"%x.",UE_rrc_inst[Mod_id].SI[eNB_index][i]);
  LOG_D(RRC,"\n");
  */
  // Dump contents
  if ((*si)->criticalExtensions.present==SystemInformation__criticalExtensions_PR_systemInformation_r8) {
    LOG_D(RRC,"(*si)->criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list.count %d\n",
       (*si)->criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list.count);
  }
  else {
    LOG_D(RRC,"[UE] Unknown criticalExtension version (not Rel8)\n");
    return -1;
  }

  for (i=0;i<(*si)->criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list.count;i++) {
    printf("SI count %d\n",i);
    typeandinfo=(*si)->criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list.array[i];

    switch(typeandinfo->present) {
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib2:
      UE_rrc_inst[Mod_id].sib2[eNB_index] = &typeandinfo->choice.sib2;
      LOG_D(RRC,"[UE %d] Frame %d Found SIB2 from eNB %d\n",Mod_id,frame,eNB_index);
      dump_sib2(UE_rrc_inst[Mod_id].sib2[eNB_index]);
      LOG_D(RRC, "[MSC_MSG][FRAME %05d][RRC_UE][MOD %02d][][--- MAC_CONFIG_REQ (SIB2 params  eNB %d) --->][MAC_UE][MOD %02d][]\n",
            frame, Mod_id, eNB_index, Mod_id);
      rrc_mac_config_req(Mod_id,0,0,eNB_index,
			 &UE_rrc_inst[Mod_id].sib2[eNB_index]->radioResourceConfigCommon,
			 (struct PhysicalConfigDedicated *)NULL,
			 (MeasObjectToAddMod_t **)NULL,
			 (MAC_MainConfig_t *)NULL,
			 0,
			 (struct LogicalChannelConfig *)NULL,
			 (MeasGapConfig_t *)NULL,
			 (TDD_Config_t *)NULL,
			 NULL,
			 NULL);
      UE_rrc_inst[Mod_id].Info[eNB_index].SIStatus = 1;
      // After SI is received, prepare RRCConnectionRequest
      rrc_ue_generate_RRCConnectionRequest(Mod_id,frame,eNB_index);

      if (UE_rrc_inst[Mod_id].Info[eNB_index].State == RRC_IDLE) {
	LOG_I(RRC,"[UE %d] Received SIB1/SIB2/SIB3 from eNB %d Switching to RRC_SI_RECEIVED\n",Mod_id, eNB_index);
	UE_rrc_inst[Mod_id].Info[eNB_index].State = RRC_SI_RECEIVED;
      }
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib3:
      UE_rrc_inst[Mod_id].sib3[eNB_index] = &typeandinfo->choice.sib3;
      LOG_I(RRC,"[UE %d] Frame %d Found SIB3 from eNB %d\n",Mod_id,frame,eNB_index);
      dump_sib3(UE_rrc_inst[Mod_id].sib3[eNB_index]);
      UE_rrc_inst[Mod_id].Info[eNB_index].SIStatus = 1;
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib4:
      UE_rrc_inst[Mod_id].sib4[eNB_index] = &typeandinfo->choice.sib4;
      LOG_I(RRC,"[UE %d] Frame %d Found SIB4 from eNB %d\n",Mod_id,frame,eNB_index);
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib5:
      UE_rrc_inst[Mod_id].sib5[eNB_index] = &typeandinfo->choice.sib5;
      LOG_I(RRC,"[UE %d] Found SIB5 from eNB %d\n",Mod_id,eNB_index);
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib6:
      UE_rrc_inst[Mod_id].sib6[eNB_index] = &typeandinfo->choice.sib6;
      LOG_I(RRC,"[UE %d] Found SIB6 from eNB %d\n",Mod_id,eNB_index);
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib7:
      UE_rrc_inst[Mod_id].sib7[eNB_index] = &typeandinfo->choice.sib7;
      LOG_I(RRC,"[UE %d] Found SIB7 from eNB %d\n",Mod_id,eNB_index);
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib8:
      UE_rrc_inst[Mod_id].sib8[eNB_index] = &typeandinfo->choice.sib8;
      LOG_I(RRC,"[UE %d] Found SIB8 from eNB %d\n",Mod_id,eNB_index);
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib9:
      UE_rrc_inst[Mod_id].sib9[eNB_index] = &typeandinfo->choice.sib9;
      LOG_I(RRC,"[UE %d] Found SIB9 from eNB %d\n",Mod_id,eNB_index);
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib10:
      UE_rrc_inst[Mod_id].sib10[eNB_index] = &typeandinfo->choice.sib10;
      LOG_I(RRC,"[UE %d] Found SIB10 from eNB %d\n",Mod_id,eNB_index);
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib11:
      UE_rrc_inst[Mod_id].sib11[eNB_index] = &typeandinfo->choice.sib11;
      LOG_I(RRC,"[UE %d] Found SIB11 from eNB %d\n",Mod_id,eNB_index);
      break;
#ifdef Rel10
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib12_v920:
      UE_rrc_inst[Mod_id].sib12[eNB_index] = &typeandinfo->choice.sib12_v920;
      LOG_I(RRC,"[RRC][UE %d] Found SIB12 from eNB %d\n",Mod_id,eNB_index);

      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib13_v920:
      UE_rrc_inst[Mod_id].sib13[eNB_index] = &typeandinfo->choice.sib13_v920;
      LOG_I(RRC,"[RRC][UE %d] Found SIB13 from eNB %d\n",Mod_id,eNB_index);
      dump_sib13(UE_rrc_inst[Mod_id].sib13[eNB_index]);
      /*
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
      */
      break;
#endif
    default:
      break;
    }

  }

  return 0;
}



#ifndef USER_MODE
EXPORT_SYMBOL(Rlc_info_am_config);
#endif
