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
#include "PHY/TOOLS/dB_routines.h"
#include "extern.h"
#include "RRC/L2_INTERFACE/openair_rrc_L2_interface.h"
#include "LAYER2/RLC/rlc.h"
#include "COMMON/mac_rrc_primitives.h"
#include "UTIL/LOG/log.h"
#include "UTIL/LOG/vcd_signal_dumper.h"
#ifndef CELLULAR
#include "RRC/LITE/MESSAGES/asn1_msg.h"
#endif
#include "RRCConnectionRequest.h"
#include "RRCConnectionReconfiguration.h"
#include "UL-CCCH-Message.h"
#include "DL-CCCH-Message.h"
#include "UL-DCCH-Message.h"
#include "DL-DCCH-Message.h"
#include "BCCH-DL-SCH-Message.h"
#ifdef Rel10
#include "MCCH-Message.h"
#endif
#include "MeasConfig.h"
#include "MeasGapConfig.h"
#include "MeasObjectEUTRA.h"
#include "TDD-Config.h"
#include "UECapabilityEnquiry.h"
#include "UE-CapabilityRequest.h"
#ifdef PHY_ABSTRACTION
#include "OCG.h"
#include "OCG_extern.h"
#endif
#ifdef USER_MODE
#include "RRC/NAS/nas_config.h"
#include "RRC/NAS/rb_config.h"
#endif

#if defined(ENABLE_SECURITY)
# include "UTIL/OSA/osa_defs.h"
#endif

#include "pdcp.h"

#if defined(ENABLE_ITTI)
# include "intertask_interface.h"
#endif

#ifdef PHY_EMUL
extern EMULATION_VARS *Emul_vars;
#endif
extern eNB_MAC_INST *eNB_mac_inst;
extern UE_MAC_INST *UE_mac_inst;
#ifdef BIGPHYSAREA
extern void *bigphys_malloc(int);
#endif

//#define XER_PRINT

extern inline unsigned int taus(void);
extern s8 dB_fixed2(u32 x,u32 y);

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

#ifdef Rel10
void init_MCCH_UE(u8 Mod_id, u8 eNB_index) {
  int i;
  UE_rrc_inst[Mod_id].sizeof_MCCH_MESSAGE[eNB_index] = 0;
  UE_rrc_inst[Mod_id].MCCH_MESSAGE[eNB_index] = (u8 *)malloc16(32);
  UE_rrc_inst[Mod_id].mcch_message[eNB_index] = (MBSFNAreaConfiguration_r9_t *)malloc16(sizeof(MBSFNAreaConfiguration_r9_t));
  for (i=0; i<8;i++) // MAX MBSFN Area
    UE_rrc_inst[Mod_id].Info[eNB_index].MCCHStatus[i] = 0;

}
#endif

static
void openair_rrc_lite_ue_init_security(u8 Mod_id)
{
#if defined(ENABLE_SECURITY)
    uint8_t *kRRCenc;
    uint8_t *kRRCint;
    char ascii_buffer[65];
    uint8_t i;

    memset(UE_rrc_inst[Mod_id].kenb, Mod_id, 32);

    for (i = 0; i < 32; i++) {
        sprintf(&ascii_buffer[2 * i], "%02X", UE_rrc_inst[Mod_id].kenb[i]);
    }

    LOG_T(RRC, "[OSA][UE %02d] kenb    = %s\n",
            Mod_id, ascii_buffer);
#endif
}

/*------------------------------------------------------------------------------*/
char openair_rrc_lite_ue_init(u8 Mod_id, unsigned char eNB_index){
  /*-----------------------------------------------------------------------------*/
  LOG_D(RRC,"[UE %d] INIT State = RRC_IDLE (eNB %d)\n",Mod_id,eNB_index);
  LOG_D(RRC,"[MSC_NEW][FRAME 00000][RRC_UE][MOD %02d][]\n", Mod_id+NB_eNB_INST);
  LOG_D(RRC, "[MSC_NEW][FRAME 00000][IP][MOD %02d][]\n", Mod_id+NB_eNB_INST);

  UE_rrc_inst[Mod_id].Info[eNB_index].State=RRC_IDLE;
  UE_rrc_inst[Mod_id].Info[eNB_index].T300_active = 0;
  UE_rrc_inst[Mod_id].Info[eNB_index].T304_active = 0;
  UE_rrc_inst[Mod_id].Info[eNB_index].T310_active = 0;
  UE_rrc_inst[Mod_id].Info[eNB_index].UE_index=0xffff;
  UE_rrc_inst[Mod_id].Srb0[eNB_index].Active=0;
  UE_rrc_inst[Mod_id].Srb1[eNB_index].Active=0;
  UE_rrc_inst[Mod_id].Srb2[eNB_index].Active=0;
  UE_rrc_inst[Mod_id].HandoverInfoUe.measFlag=1;

  UE_rrc_inst[Mod_id].ciphering_algorithm = SecurityAlgorithmConfig__cipheringAlgorithm_eea0;
#ifdef Rel10
  UE_rrc_inst[Mod_id].integrity_algorithm = SecurityAlgorithmConfig__integrityProtAlgorithm_eia0_v920;
#else
  UE_rrc_inst[Mod_id].integrity_algorithm = SecurityAlgorithmConfig__integrityProtAlgorithm_reserved;
#endif

  openair_rrc_lite_ue_init_security(Mod_id);

  init_SI_UE(Mod_id,eNB_index);
  LOG_D(RRC,"[UE %d] INIT: phy_sync_2_ch_ind\n", Mod_id);

#ifndef NO_RRM
  send_msg(&S_rrc,msg_rrc_phy_synch_to_CH_ind(Mod_id,eNB_index,UE_rrc_inst[Mod_id].Mac_id));
#endif

#ifdef NO_RRM //init ch SRB0, SRB1 & BDTCH
  openair_rrc_on(Mod_id,0);
#endif
#ifdef CBA 
  int j;
  for(j=0; j<NUM_MAX_CBA_GROUP; j++)
    UE_rrc_inst[Mod_id].cba_rnti[j] = 0x0000;
  UE_rrc_inst[Mod_id].num_active_cba_groups = 0;
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
#ifdef SMBV   
          // if SMBV is configured the contention resolution needs to be fix for the connection procedure to succeed
          rv[i]=i;
#else
          rv[i]=taus()&0xff;
#endif
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

/* NAS Attach request with IMSI */
static const char nas_attach_req_imsi[] =
{
    0x07, 0x41,
    /* EPS Mobile identity = IMSI */
    0x71, 0x08, 0x29, 0x80, 0x43, 0x21, 0x43, 0x65, 0x87,
    0xF9,
    /* End of EPS Mobile Identity */
    0x02, 0xE0, 0xE0, 0x00, 0x20, 0x02, 0x03,
    0xD0, 0x11, 0x27, 0x1A, 0x80, 0x80, 0x21, 0x10, 0x01, 0x00, 0x00,
    0x10, 0x81, 0x06, 0x00, 0x00, 0x00, 0x00, 0x83, 0x06, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x0D, 0x00, 0x00, 0x0A, 0x00, 0x52, 0x12, 0xF2,
    0x01, 0x27, 0x11,
};

/* NAS Attach request with GUTI */
static const char nas_attach_req_guti[] =
{
    0x07, 0x41,
    /* EPS Mobile identity = GUTI */
    0x71, 0x0B, 0xF6, 0x12, 0xF2, 0x01, 0x80, 0x00, 0x01, 0xE0, 0x00,
    0xDA, 0x1F,
    /* End of EPS Mobile Identity */
    0x02, 0xE0, 0xE0, 0x00, 0x20, 0x02, 0x03,
    0xD0, 0x11, 0x27, 0x1A, 0x80, 0x80, 0x21, 0x10, 0x01, 0x00, 0x00,
    0x10, 0x81, 0x06, 0x00, 0x00, 0x00, 0x00, 0x83, 0x06, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x0D, 0x00, 0x00, 0x0A, 0x00, 0x52, 0x12, 0xF2,
    0x01, 0x27, 0x11,
};

/*------------------------------------------------------------------------------*/
void rrc_ue_generate_RRCConnectionSetupComplete(u8 Mod_id, u32 frame, u8 eNB_index, uint8_t Transaction_id){
  /*------------------------------------------------------------------------------*/

  u8 buffer[100];
  u8 size;

  size = do_RRCConnectionSetupComplete(buffer, Transaction_id, sizeof(nas_attach_req_guti), nas_attach_req_guti);

  LOG_I(RRC,"[UE %d][RAPROC] Frame %d : Logical Channel UL-DCCH (SRB1), Generating RRCConnectionSetupComplete (bytes%d, eNB %d)\n",
	Mod_id,frame, size, eNB_index);

   LOG_D(RLC, "[MSC_MSG][FRAME %05d][RRC_UE][MOD %02d][][--- PDCP_DATA_REQ/%d Bytes (RRCConnectionSetupComplete to eNB %d MUI %d) --->][PDCP][MOD %02d][RB %02d]\n",
                                     frame, Mod_id+NB_eNB_INST, size, eNB_index, rrc_mui, Mod_id+NB_eNB_INST, DCCH);

   //  rrc_rlc_data_req(Mod_id+NB_eNB_INST,frame, 0 ,DCCH,rrc_mui++,0,size,(char*)buffer);
  pdcp_rrc_data_req (Mod_id + NB_eNB_INST, frame, 0, DCCH, rrc_mui++, 0, size, buffer, 1);

}



void rrc_ue_generate_RRCConnectionReconfigurationComplete(u8 Mod_id, u32 frame, u8 eNB_index, uint8_t Transaction_id) {

  u8 buffer[32], size;

  size = do_RRCConnectionReconfigurationComplete(buffer, Transaction_id);

  LOG_I(RRC,"[UE %d] Frame %d : Logical Channel UL-DCCH (SRB1), Generating RRCConnectionReconfigurationComplete (bytes %d, eNB_index %d)\n",
	Mod_id,frame, size, eNB_index);
  LOG_D(RLC, "[MSC_MSG][FRAME %05d][RRC_UE][MOD %02d][][--- PDCP_DATA_REQ/%d Bytes (RRCConnectionReconfigurationComplete to eNB %d MUI %d) --->][PDCP][MOD %02d][RB %02d]\n",
	frame, Mod_id+NB_eNB_INST, size, eNB_index, rrc_mui, Mod_id+NB_eNB_INST, DCCH);

  //rrc_rlc_data_req(Mod_id+NB_eNB_INST,frame, 0 ,DCCH,rrc_mui++,0,size,(char*)buffer);
  pdcp_rrc_data_req (Mod_id + NB_eNB_INST, frame, 0, DCCH, rrc_mui++, 0, size, buffer, 1);
}

/*------------------------------------------------------------------------------*/
int rrc_ue_decode_ccch(u8 Mod_id, u32 frame, SRB_INFO *Srb_info, u8 eNB_index){
  /*------------------------------------------------------------------------------*/

  //DL_CCCH_Message_t dlccchmsg;
  DL_CCCH_Message_t *dl_ccch_msg=NULL;//&dlccchmsg;
  asn_dec_rval_t dec_rval;
  int rval=0;

  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_DECODE_CCCH, VCD_FUNCTION_IN);

  //memset(dl_ccch_msg,0,sizeof(DL_CCCH_Message_t));

  //  LOG_D(RRC,"[UE %d] Decoding DL-CCCH message (%d bytes), State %d\n",Mod_id,Srb_info->Rx_buffer.payload_size,
  //	UE_rrc_inst[Mod_id].Info[eNB_index].State);

  dec_rval = uper_decode(NULL,
			 &asn_DEF_DL_CCCH_Message,
			 (void**)&dl_ccch_msg,
	 		 (uint8_t*)Srb_info->Rx_buffer.Payload,
			 100,0,0);

#ifdef XER_PRINT
  xer_fprint(stdout,&asn_DEF_DL_CCCH_Message,(void*)dl_ccch_msg);
#endif

#if defined(ENABLE_ITTI)
# if defined(DISABLE_ITTI_XER_PRINT)
  {
    MessageDef *message_p;

    message_p = itti_alloc_new_message (TASK_RRC_UE, RRC_DL_CCCH_MESSAGE);
    memcpy (&message_p->ittiMsg, (void *) dl_ccch_msg, sizeof(RrcDlCcchMessage));

    itti_send_msg_to_task (TASK_UNKNOWN, Mod_id + NB_eNB_INST, message_p);
  }
# else
  {
    char        message_string[10000];
    size_t      message_string_size;

    if ((message_string_size = xer_sprint(message_string, sizeof(message_string), &asn_DEF_DL_CCCH_Message, (void *)dl_ccch_msg)) > 0)
    {
      MessageDef *message_p;

      message_p = itti_alloc_new_message_sized (TASK_RRC_UE, GENERIC_LOG, message_string_size);
      memcpy(&message_p->ittiMsg.generic_log, message_string, message_string_size);

      itti_send_msg_to_task(TASK_UNKNOWN, Mod_id + NB_eNB_INST, message_p);
    }
  }
# endif
#endif

  if ((dec_rval.code != RC_OK) && (dec_rval.consumed==0)) {
    LOG_E(RRC,"[UE %d] Frame %d : Failed to decode DL-CCCH-Message (%d bytes)\n",Mod_id,dec_rval.consumed);
    vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_DECODE_CCCH, VCD_FUNCTION_OUT);
    return -1;
  }

  if (dl_ccch_msg->message.present == DL_CCCH_MessageType_PR_c1) {

    if (UE_rrc_inst[Mod_id].Info[eNB_index].State == RRC_SI_RECEIVED) {

      switch (dl_ccch_msg->message.choice.c1.present) {

        case DL_CCCH_MessageType__c1_PR_NOTHING:
          LOG_I(RRC, "[UE%d] Frame %d : Received PR_NOTHING on DL-CCCH-Message\n", Mod_id, frame);
          rval = 0;
          break;

        case DL_CCCH_MessageType__c1_PR_rrcConnectionReestablishment:
          LOG_D(RRC,
              "[MSC_MSG][FRAME %05d][MAC_UE][MOD %02d][][--- MAC_DATA_IND (rrcConnectionReestablishment ENB %d) --->][RRC_UE][MOD %02d][]\n", frame, Mod_id+NB_eNB_INST, eNB_index, Mod_id+NB_eNB_INST);

          LOG_I(RRC,
              "[UE%d] Frame %d : Logical Channel DL-CCCH (SRB0), Received RRCConnectionReestablishment\n", Mod_id, frame);
          rval = 0;
          break;

        case DL_CCCH_MessageType__c1_PR_rrcConnectionReestablishmentReject:
          LOG_D(RRC,
              "[MSC_MSG][FRAME %05d][MAC_UE][MOD %02d][][--- MAC_DATA_IND (RRCConnectionReestablishmentReject ENB %d) --->][RRC_UE][MOD %02d][]\n", frame, Mod_id+NB_eNB_INST, eNB_index, Mod_id+NB_eNB_INST);
          LOG_I(RRC,
              "[UE%d] Frame %d : Logical Channel DL-CCCH (SRB0), Received RRCConnectionReestablishmentReject\n", Mod_id, frame);
          rval = 0;
          break;

        case DL_CCCH_MessageType__c1_PR_rrcConnectionReject:
          LOG_D(RRC,
              "[MSC_MSG][FRAME %05d][MAC_UE][MOD %02d][][--- MAC_DATA_IND (rrcConnectionReject ENB %d) --->][RRC_UE][MOD %02d][]\n", frame, Mod_id+NB_eNB_INST, eNB_index, Mod_id+NB_eNB_INST);

          LOG_I(RRC,
              "[UE%d] Frame %d : Logical Channel DL-CCCH (SRB0), Received RRCConnectionReject \n", Mod_id, frame);
          rval = 0;
          break;

        case DL_CCCH_MessageType__c1_PR_rrcConnectionSetup:
          LOG_D(RRC,
              "[MSC_MSG][FRAME %05d][MAC_UE][MOD %02d][][--- MAC_DATA_IND (rrcConnectionSetup ENB %d) --->][RRC_UE][MOD %02d][]\n", frame, Mod_id+NB_eNB_INST, eNB_index, Mod_id+NB_eNB_INST);

          LOG_I(RRC,
              "[UE%d][RAPROC] Frame %d : Logical Channel DL-CCCH (SRB0), Received RRCConnectionSetup \n", Mod_id, frame);
          // Get configuration

          // Release T300 timer
          UE_rrc_inst[Mod_id].Info[eNB_index].T300_active = 0;
          rrc_ue_process_radioResourceConfigDedicated(Mod_id, frame, eNB_index,
              &dl_ccch_msg->message.choice.c1.choice.rrcConnectionSetup.criticalExtensions.choice.c1.choice.rrcConnectionSetup_r8.radioResourceConfigDedicated);

          rrc_ue_generate_RRCConnectionSetupComplete(Mod_id, frame, eNB_index, dl_ccch_msg->message.choice.c1.choice.rrcConnectionSetup.rrc_TransactionIdentifier);

          rval = 0;
          break;

        default:
          LOG_E(RRC, "[UE%d] Frame %d : Unknown message\n", Mod_id, frame);
          rval = -1;
          break;
      }
    }
  }

  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_DECODE_CCCH, VCD_FUNCTION_OUT);
  return rval;
}


s32 rrc_ue_establish_srb1(u8 Mod_id,u32 frame,u8 eNB_index,
			 struct SRB_ToAddMod *SRB_config) { // add descriptor from RRC PDU

  u8 lchan_id = DCCH;

  UE_rrc_inst[Mod_id].Srb1[eNB_index].Active = 1;
  UE_rrc_inst[Mod_id].Srb1[eNB_index].Status = RADIO_CONFIG_OK;//RADIO CFG
  UE_rrc_inst[Mod_id].Srb1[eNB_index].Srb_info.Srb_id = 1;

    // copy default configuration for now
  //  memcpy(&UE_rrc_inst[Mod_id].Srb1[eNB_index].Srb_info.Lchan_desc[0],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
  //  memcpy(&UE_rrc_inst[Mod_id].Srb1[eNB_index].Srb_info.Lchan_desc[1],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);


  LOG_I(RRC,"[UE %d], CONFIG_SRB1 %d corresponding to eNB_index %d\n", Mod_id,lchan_id,eNB_index);

  //rrc_pdcp_config_req (Mod_id+NB_eNB_INST, frame, 0, ACTION_ADD, lchan_id,UNDEF_SECURITY_MODE);
  //  rrc_rlc_config_req(Mod_id+NB_eNB_INST,frame,0,ACTION_ADD,lchan_id,SIGNALLING_RADIO_BEARER,Rlc_info_am_config);

  //  UE_rrc_inst[Mod_id].Srb1[eNB_index].Srb_info.Tx_buffer.payload_size=DEFAULT_MEAS_IND_SIZE+1;


  return(0);
}

s32 rrc_ue_establish_srb2(u8 Mod_id,u32 frame,u8 eNB_index,
			 struct SRB_ToAddMod *SRB_config) { // add descriptor from RRC PDU

  u8 lchan_id = DCCH1;

  UE_rrc_inst[Mod_id].Srb2[eNB_index].Active = 1;
  UE_rrc_inst[Mod_id].Srb2[eNB_index].Status = RADIO_CONFIG_OK;//RADIO CFG
  UE_rrc_inst[Mod_id].Srb2[eNB_index].Srb_info.Srb_id = 2;

    // copy default configuration for now
  //  memcpy(&UE_rrc_inst[Mod_id].Srb2[eNB_index].Srb_info.Lchan_desc[0],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
  //  memcpy(&UE_rrc_inst[Mod_id].Srb2[eNB_index].Srb_info.Lchan_desc[1],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);


  LOG_I(RRC,"[UE %d], CONFIG_SRB2 %d corresponding to eNB_index %d\n",Mod_id,lchan_id,eNB_index);

  //rrc_pdcp_config_req (Mod_id+NB_eNB_INST, frame, 0, ACTION_ADD, lchan_id, UNDEF_SECURITY_MODE);
  //  rrc_rlc_config_req(Mod_id+NB_eNB_INST,frame,0,ACTION_ADD,lchan_id,SIGNALLING_RADIO_BEARER,Rlc_info_am_config);

  //  UE_rrc_inst[Mod_id].Srb1[eNB_index].Srb_info.Tx_buffer.payload_size=DEFAULT_MEAS_IND_SIZE+1;


  return(0);
}

s32 rrc_ue_establish_drb(u8 Mod_id,u32 frame,u8 eNB_index,
			 struct DRB_ToAddMod *DRB_config) { // add descriptor from RRC PDU
#ifdef NAS_NETLINK
  int oip_ifup=0,ip_addr_offset3=0,ip_addr_offset4=0;
#endif

  LOG_I(RRC,"[UE] Frame %d: processing RRCConnectionReconfiguration: reconfiguring DRB %ld/LCID %d\n",
	frame,DRB_config->drb_Identity,(int)*DRB_config->logicalChannelIdentity);
  /*
  rrc_pdcp_config_req (Mod_id+NB_eNB_INST, frame, 0, ACTION_ADD,
		       (eNB_index * NB_RB_MAX) + *DRB_config->logicalChannelIdentity, UNDEF_SECURITY_MODE);
 
 rrc_rlc_config_req(Mod_id+NB_eNB_INST,frame,0,ACTION_ADD,
		     (eNB_index * NB_RB_MAX) + *DRB_config->logicalChannelIdentity,
		     RADIO_ACCESS_BEARER,Rlc_info_um);
  */
#ifdef NAS_NETLINK
#    ifdef OAI_EMU
    ip_addr_offset3 = oai_emulation.info.nb_enb_local;
    ip_addr_offset4 = NB_eNB_INST;
#    else
    ip_addr_offset3 = 0;
    ip_addr_offset4 = 8;
#    endif
#    if !defined(OAI_NW_DRIVER_TYPE_ETHERNET) && !defined(EXMIMO)
    LOG_I(OIP,"[UE %d] trying to bring up the OAI interface oai%d, IP 10.0.%d.%d\n", Mod_id, ip_addr_offset3+Mod_id,
	  ip_addr_offset3+Mod_id+1,ip_addr_offset4+Mod_id+1);
    oip_ifup=nas_config(ip_addr_offset3+Mod_id,   // interface_id
			ip_addr_offset3+Mod_id+1, // third_octet
			ip_addr_offset4+Mod_id+1); // fourth_octet
    if (oip_ifup == 0 ){ // interface is up --> send a config the DRB
#        ifdef OAI_EMU
      oai_emulation.info.oai_ifup[Mod_id]=1;
#        endif
      LOG_I(OIP,"[UE %d] Config the oai%d to send/receive pkt on DRB %d to/from the protocol stack\n",
	    Mod_id,
	    ip_addr_offset3+Mod_id,
	    (eNB_index * NB_RB_MAX) + *DRB_config->logicalChannelIdentity);

	    rb_conf_ipv4(0,//add
			 Mod_id,//cx align with the UE index
			 ip_addr_offset3+Mod_id,//inst num_enb+ue_index
			 (eNB_index * NB_RB_MAX) + *DRB_config->logicalChannelIdentity,//rb
			 0,//dscp
			 ipv4_address(ip_addr_offset3+Mod_id+1,ip_addr_offset4+Mod_id+1),//saddr
			 ipv4_address(ip_addr_offset3+Mod_id+1,eNB_index+1));//daddr
	    LOG_D(RRC,"[UE %d] State = Attached (eNB %d)\n",Mod_id,eNB_index);
    }
#    else
#        ifdef OAI_EMU
      oai_emulation.info.oai_ifup[Mod_id]=1;
#        endif
#    endif
#endif

  return(0);
}


void  rrc_ue_process_measConfig(u8 Mod_id,u32 frame, u8 eNB_index,MeasConfig_t *measConfig){

  // This is the procedure described in 36.331 Section 5.5.2.1
  int i;
  long ind;
  MeasObjectToAddMod_t *measObj;

  if (measConfig->measObjectToRemoveList != NULL) {
    for (i=0;i<measConfig->measObjectToRemoveList->list.count;i++) {
      ind   = *measConfig->measObjectToRemoveList->list.array[i];
      free(UE_rrc_inst[Mod_id].MeasObj[eNB_index][ind-1]);
    }
  }
  if (measConfig->measObjectToAddModList != NULL) {
    LOG_D(RRC,"Measurement Object List is present\n");
    for (i=0;i<measConfig->measObjectToAddModList->list.count;i++) {
      measObj = measConfig->measObjectToAddModList->list.array[i];
      ind   = measConfig->measObjectToAddModList->list.array[i]->measObjectId;

      if (UE_rrc_inst[Mod_id].MeasObj[eNB_index][ind-1]) {
	LOG_D(RRC,"Modifying measurement object %d\n",ind);
	memcpy((char*)UE_rrc_inst[Mod_id].MeasObj[eNB_index][ind-1],
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
		       (MobilityControlInfo_t *)NULL,
		       NULL,
		       NULL,
		       NULL,
		       NULL,
		       NULL,
		       NULL
#ifdef Rel10	       
		       ,
		       0,
		       (MBSFN_AreaInfoList_r9_t *)NULL,
		       (PMCH_InfoList_r9_t *)NULL
#endif
#ifdef CBA
		       ,
		       0,
		       0
#endif
		       );
  }
  if (measConfig->reportConfigToRemoveList != NULL) {
    for (i=0;i<measConfig->reportConfigToRemoveList->list.count;i++) {
      ind   = *measConfig->reportConfigToRemoveList->list.array[i];
      free(UE_rrc_inst[Mod_id].ReportConfig[eNB_index][ind-1]);
    }
  }
  if (measConfig->reportConfigToAddModList != NULL) {
    LOG_I(RRC,"Report Configuration List is present\n");
    for (i=0;i<measConfig->reportConfigToAddModList->list.count;i++) {
      ind   = measConfig->reportConfigToAddModList->list.array[i]->reportConfigId;
      if (UE_rrc_inst[Mod_id].ReportConfig[eNB_index][ind-1]) {
	LOG_I(RRC,"Modifying Report Configuration %d\n",ind-1);
	memcpy((char*)UE_rrc_inst[Mod_id].ReportConfig[eNB_index][ind-1],
	       (char*)measConfig->reportConfigToAddModList->list.array[i],
	       sizeof(ReportConfigToAddMod_t));
      }
      else {
	LOG_D(RRC,"Adding Report Configuration %d %p \n",ind-1,measConfig->reportConfigToAddModList->list.array[i]);
	UE_rrc_inst[Mod_id].ReportConfig[eNB_index][ind-1] = measConfig->reportConfigToAddModList->list.array[i];
      }
    }
  }

  if (measConfig->quantityConfig != NULL) {
    if (UE_rrc_inst[Mod_id].QuantityConfig[eNB_index]) {
      LOG_D(RRC,"Modifying Quantity Configuration \n");
      memcpy((char*)UE_rrc_inst[Mod_id].QuantityConfig[eNB_index],
	     (char*)measConfig->quantityConfig,
	     sizeof(QuantityConfig_t));
    }
    else {
      LOG_D(RRC,"Adding Quantity configuration\n");
      UE_rrc_inst[Mod_id].QuantityConfig[eNB_index] = measConfig->quantityConfig;
    }
  }

  if (measConfig->measIdToRemoveList != NULL) {
    for (i=0;i<measConfig->measIdToRemoveList->list.count;i++) {
      ind   = *measConfig->measIdToRemoveList->list.array[i];
      free(UE_rrc_inst[Mod_id].MeasId[eNB_index][ind-1]);
    }
  }

  if (measConfig->measIdToAddModList != NULL) {
    for (i=0;i<measConfig->measIdToAddModList->list.count;i++) {
      ind   = measConfig->measIdToAddModList->list.array[i]->measId;
      if (UE_rrc_inst[Mod_id].MeasId[eNB_index][ind-1]) {
	LOG_D(RRC,"Modifying Measurement ID %d\n",ind-1);
	memcpy((char*)UE_rrc_inst[Mod_id].MeasId[eNB_index][ind-1],
	       (char*)measConfig->measIdToAddModList->list.array[i],
	       sizeof(MeasIdToAddMod_t));
      }
      else {
	LOG_D(RRC,"Adding Measurement ID %d %p\n",ind-1,measConfig->measIdToAddModList->list.array[i]);
	UE_rrc_inst[Mod_id].MeasId[eNB_index][ind-1] = measConfig->measIdToAddModList->list.array[i];
      }
    }
  }

  if (measConfig->measGapConfig !=NULL) {
    if (UE_rrc_inst[Mod_id].measGapConfig[eNB_index]) {
      memcpy((char*)UE_rrc_inst[Mod_id].measGapConfig[eNB_index],
	     (char*)measConfig->measGapConfig,
	     sizeof(MeasGapConfig_t));
    }
    else {
      UE_rrc_inst[Mod_id].measGapConfig[eNB_index] = measConfig->measGapConfig;
    }
  }

  if (measConfig->quantityConfig != NULL) {
    if (UE_rrc_inst[Mod_id].QuantityConfig[eNB_index]) {
      LOG_I(RRC,"Modifying Quantity Configuration \n");
      memcpy((char*)UE_rrc_inst[Mod_id].QuantityConfig[eNB_index],
	     (char*)measConfig->quantityConfig,
	     sizeof(QuantityConfig_t));
    }
    else {
      LOG_I(RRC,"Adding Quantity configuration\n");
      UE_rrc_inst[Mod_id].QuantityConfig[eNB_index] = measConfig->quantityConfig;
    }
    
    UE_rrc_inst[Mod_id].filter_coeff_rsrp = 1./pow(2,(*UE_rrc_inst[Mod_id].QuantityConfig[eNB_index]->quantityConfigEUTRA->filterCoefficientRSRP)/4);
    UE_rrc_inst[Mod_id].filter_coeff_rsrq = 1./pow(2,(*UE_rrc_inst[Mod_id].QuantityConfig[eNB_index]->quantityConfigEUTRA->filterCoefficientRSRQ)/4);

    LOG_I(RRC,"[UE %d] set rsrp-coeff for eNB %d: %d rsrq-coeff: %d rsrp_factor: %f rsrq_factor: %f \n",
	  Mod_id, eNB_index, // UE_rrc_inst[Mod_id].Info[eNB_index].UE_index,
	  *UE_rrc_inst[Mod_id].QuantityConfig[eNB_index]->quantityConfigEUTRA->filterCoefficientRSRP,
	  *UE_rrc_inst[Mod_id].QuantityConfig[eNB_index]->quantityConfigEUTRA->filterCoefficientRSRQ,
	  UE_rrc_inst[Mod_id].filter_coeff_rsrp, UE_rrc_inst[Mod_id].filter_coeff_rsrp,
	  UE_rrc_inst[Mod_id].filter_coeff_rsrp, UE_rrc_inst[Mod_id].filter_coeff_rsrq);
  }

  if (measConfig->s_Measure != NULL) {
    UE_rrc_inst[Mod_id].s_measure = *measConfig->s_Measure;
  }
 
  if (measConfig->speedStatePars != NULL) {
    if (UE_rrc_inst[Mod_id].speedStatePars)
      memcpy((char*)UE_rrc_inst[Mod_id].speedStatePars,(char*)measConfig->speedStatePars,sizeof(struct MeasConfig__speedStatePars));
    else
      UE_rrc_inst[Mod_id].speedStatePars = measConfig->speedStatePars;
    LOG_I(RRC,"[UE %d] Configuring mobility optimization params for UE %d \n",
	  Mod_id,UE_rrc_inst[Mod_id].Info[0].UE_index);
  }
}


void	
rrc_ue_process_radioResourceConfigDedicated(u8 Mod_id,u32 frame, u8 eNB_index,
					    RadioResourceConfigDedicated_t *radioResourceConfigDedicated) {

  long SRB_id,DRB_id;
  int i,cnt;
  LogicalChannelConfig_t *SRB1_logicalChannelConfig,*SRB2_logicalChannelConfig;
#ifdef CBA  
  uint8_t cba_found = 0;
  uint16_t cba_RNTI;
#endif 

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
#ifdef CBA
  if (radioResourceConfigDedicated->cba_RNTI_vlola) {
    cba_RNTI = (uint16_t) (((radioResourceConfigDedicated->cba_RNTI_vlola->buf[1]&0xff) << 8) | 
			   (radioResourceConfigDedicated->cba_RNTI_vlola->buf[0]&0xff));
    for (i=0 ; i< NUM_MAX_CBA_GROUP; i++) {
      if (UE_rrc_inst[Mod_id].cba_rnti[i] == cba_RNTI ) {
	cba_found=1;
	break;
      } else if (UE_rrc_inst[Mod_id].cba_rnti[i] == 0 )
	break;
    }
    if (cba_found==0) {
      UE_rrc_inst[Mod_id].num_active_cba_groups++;
      UE_rrc_inst[Mod_id].cba_rnti[i]=cba_RNTI;
      LOG_D(RRC, "[UE %d] Frame %d: radioResourceConfigDedicated reveived CBA_RNTI = %x for group %d from eNB %d \n", 
	    Mod_id,frame, UE_rrc_inst[Mod_id].cba_rnti[i], i, eNB_index);
    }
  }
#endif 
  // Establish SRBs if present
  // loop through SRBToAddModList
  if (radioResourceConfigDedicated->srb_ToAddModList) {
    uint8_t *kRRCenc = NULL;
    uint8_t *kRRCint = NULL;

#if defined(ENABLE_SECURITY)
    derive_key_rrc_enc(UE_rrc_inst[Mod_id].ciphering_algorithm,
                       UE_rrc_inst[Mod_id].kenb, &kRRCenc);
    derive_key_rrc_int(UE_rrc_inst[Mod_id].integrity_algorithm,
                       UE_rrc_inst[Mod_id].kenb, &kRRCint);
#endif

// Refresh SRBs
    rrc_pdcp_config_asn1_req(NB_eNB_INST+Mod_id,frame,0,eNB_index,
                             radioResourceConfigDedicated->srb_ToAddModList,
                             (DRB_ToAddModList_t*)NULL,
                             (DRB_ToReleaseList_t*)NULL,
                             UE_rrc_inst[Mod_id].ciphering_algorithm |
                             (UE_rrc_inst[Mod_id].integrity_algorithm << 4),
                             kRRCenc,
                             kRRCint,
                             NULL
#ifdef Rel10
			    ,(PMCH_InfoList_r9_t *)NULL
#endif
			    );

    // Refresh SRBs
    rrc_rlc_config_asn1_req(NB_eNB_INST+Mod_id,frame,0,eNB_index,
			    radioResourceConfigDedicated->srb_ToAddModList,
			    (DRB_ToAddModList_t*)NULL,
			    (DRB_ToReleaseList_t*)NULL
#ifdef Rel10
			    ,(PMCH_InfoList_r9_t *)NULL
#endif
			    );

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
			     NULL,
			     NULL,
			     NULL,
			     NULL,
			     NULL,
			     NULL
#ifdef Rel10	       
			     ,
			     0,
			     (MBSFN_AreaInfoList_r9_t *)NULL,
			     (PMCH_InfoList_r9_t *)NULL
#endif
#ifdef CBA
			     ,
			     0,
			     0
#endif
			     );
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
			 (MobilityControlInfo_t *)NULL,
			 NULL,
			 NULL,
			 NULL,
			 NULL,
			 NULL,
			 NULL
#ifdef Rel10	       
			 ,
			 0,
			 (MBSFN_AreaInfoList_r9_t *)NULL,
			 (PMCH_InfoList_r9_t *)NULL
#endif
#ifdef CBA
			 ,
			 0,
			 0
#endif
			 );
	}
      }
    }
  }

  // Establish DRBs if present
  if (radioResourceConfigDedicated->drb_ToAddModList) {
    uint8_t *kUPenc = NULL;

#if defined(ENABLE_SECURITY)
    derive_key_up_enc(UE_rrc_inst[Mod_id].integrity_algorithm,
                      UE_rrc_inst[Mod_id].kenb, &kUPenc);
#endif

    // Refresh DRBs
    rrc_pdcp_config_asn1_req(NB_eNB_INST+Mod_id,frame,0,eNB_index,
                             (SRB_ToAddModList_t*)NULL,
                             radioResourceConfigDedicated->drb_ToAddModList,
                             (DRB_ToReleaseList_t*)NULL,
                             UE_rrc_inst[Mod_id].ciphering_algorithm |
                             (UE_rrc_inst[Mod_id].integrity_algorithm << 4),
                             NULL,
                             NULL,
                             kUPenc
#ifdef Rel10
			    ,(PMCH_InfoList_r9_t *)NULL
#endif
			    );
  
  // Refresh DRBs
    rrc_rlc_config_asn1_req(NB_eNB_INST+Mod_id,frame,0,eNB_index,
			    (SRB_ToAddModList_t*)NULL,
			    radioResourceConfigDedicated->drb_ToAddModList,
			    (DRB_ToReleaseList_t*)NULL
#ifdef Rel10
			    ,(PMCH_InfoList_r9_t *)NULL
#endif
			    );
    for (i=0;i<radioResourceConfigDedicated->drb_ToAddModList->list.count;i++) {
      DRB_id   = radioResourceConfigDedicated->drb_ToAddModList->list.array[i]->drb_Identity-1;
      if (UE_rrc_inst[Mod_id].DRB_config[eNB_index][DRB_id]) {
	memcpy(UE_rrc_inst[Mod_id].DRB_config[eNB_index][DRB_id],radioResourceConfigDedicated->drb_ToAddModList->list.array[i],
	       sizeof(struct DRB_ToAddMod));
      }
      else {
	UE_rrc_inst[Mod_id].DRB_config[eNB_index][DRB_id] = radioResourceConfigDedicated->drb_ToAddModList->list.array[i];

	rrc_ue_establish_drb(Mod_id,frame,eNB_index,radioResourceConfigDedicated->drb_ToAddModList->list.array[i]);
	// MAC/PHY Configuration
	LOG_D(RRC, "[MSC_MSG][FRAME %05d][RRC_UE][MOD %02d][][--- MAC_CONFIG_REQ (DRB %d eNB %d) --->][MAC_UE][MOD %02d][]\n",
	      frame, Mod_id, radioResourceConfigDedicated->drb_ToAddModList->list.array[i]->drb_Identity, eNB_index, Mod_id);
	rrc_mac_config_req(Mod_id,0,0,eNB_index,
			   (RadioResourceConfigCommonSIB_t *)NULL,
			   UE_rrc_inst[Mod_id].physicalConfigDedicated[eNB_index],
			   (MeasObjectToAddMod_t **)NULL,
			   UE_rrc_inst[Mod_id].mac_MainConfig[eNB_index],
			   *UE_rrc_inst[Mod_id].DRB_config[eNB_index][DRB_id]->logicalChannelIdentity,
			   UE_rrc_inst[Mod_id].DRB_config[eNB_index][DRB_id]->logicalChannelConfig,
			   UE_rrc_inst[Mod_id].measGapConfig[eNB_index],
			   (TDD_Config_t*)NULL,
			   (MobilityControlInfo_t *)NULL,
			   NULL,
			   NULL,
			   NULL,
			   NULL,
			   NULL,
			   NULL
#ifdef Rel10	       
			   ,
			   0,
			   (MBSFN_AreaInfoList_r9_t *)NULL,
			   (PMCH_InfoList_r9_t *)NULL
#endif
#ifdef CBA
			   ,
			   UE_rrc_inst[Mod_id].num_active_cba_groups, // 
			   UE_rrc_inst[Mod_id].cba_rnti[0]
#endif
			   );

      }
    }
  }

  UE_rrc_inst[Mod_id].Info[eNB_index].State = RRC_CONNECTED;
  LOG_I(RRC,"[UE %d] State = RRC_CONNECTED (eNB %d)\n",Mod_id,eNB_index);


}

void rrc_ue_process_securityModeCommand(uint8_t Mod_id,uint32_t frame,SecurityModeCommand_t *securityModeCommand,uint8_t eNB_index) {

  asn_enc_rval_t enc_rval;

  UL_DCCH_Message_t ul_dcch_msg;
  // SecurityModeCommand_t SecurityModeCommand;
  uint8_t buffer[200];
  int i, securityMode;
  
  LOG_I(RRC,"[UE %d] Frame %d: Receiving from SRB1 (DL-DCCH), Processing securityModeCommand (eNB %d)\n",
	Mod_id,frame,eNB_index);

  switch (securityModeCommand->criticalExtensions.choice.c1.choice.securityModeCommand_r8.securityConfigSMC.securityAlgorithmConfig.cipheringAlgorithm){
  case SecurityAlgorithmConfig__cipheringAlgorithm_eea0:
    LOG_I(RRC,"[UE %d] Security algorithm is set to eea0\n",Mod_id);
    securityMode= SecurityAlgorithmConfig__cipheringAlgorithm_eea0;
    break;
  case SecurityAlgorithmConfig__cipheringAlgorithm_eea1:
    LOG_I(RRC,"[UE %d] Security algorithm is set to eea1\n",Mod_id);
    securityMode= SecurityAlgorithmConfig__cipheringAlgorithm_eea1;
    break;
  case SecurityAlgorithmConfig__cipheringAlgorithm_eea2:
    LOG_I(RRC,"[UE %d] Security algorithm is set to eea2\n",Mod_id);
    securityMode = SecurityAlgorithmConfig__cipheringAlgorithm_eea2;
    break;
  default:
    LOG_I(RRC,"[UE %d] Security algorithm is set to none\n",Mod_id);
    securityMode = SecurityAlgorithmConfig__cipheringAlgorithm_spare1;
    break;
  }
  switch (securityModeCommand->criticalExtensions.choice.c1.choice.securityModeCommand_r8.securityConfigSMC.securityAlgorithmConfig.integrityProtAlgorithm){
  case SecurityAlgorithmConfig__integrityProtAlgorithm_eia1:
    LOG_I(RRC,"[UE %d] Integrity protection algorithm is set to eia1\n",Mod_id);
    securityMode |= 1 << 5;
    break;
  case SecurityAlgorithmConfig__integrityProtAlgorithm_eia2:
    LOG_I(RRC,"[UE %d] Integrity protection algorithm is set to eia2\n",Mod_id);
    securityMode |= 1 << 6;
    break;
  default:
    LOG_I(RRC,"[UE %d] Integrity protection algorithm is set to none\n",Mod_id);
    securityMode |= 0x70 ;
    break;
  }
  LOG_D(RRC,"[UE %d] security mode is %x \n",Mod_id, securityMode);

  /* Store the parameters received */
  UE_rrc_inst[Mod_id].ciphering_algorithm = securityModeCommand->criticalExtensions.choice.c1.choice.securityModeCommand_r8.securityConfigSMC.securityAlgorithmConfig.cipheringAlgorithm;
  UE_rrc_inst[Mod_id].integrity_algorithm = securityModeCommand->criticalExtensions.choice.c1.choice.securityModeCommand_r8.securityConfigSMC.securityAlgorithmConfig.integrityProtAlgorithm;

  memset((void *)&ul_dcch_msg,0,sizeof(UL_DCCH_Message_t));
  //memset((void *)&SecurityModeCommand,0,sizeof(SecurityModeCommand_t));

  ul_dcch_msg.message.present           = UL_DCCH_MessageType_PR_c1;
  if (securityMode >= NO_SECURITY_MODE)
    ul_dcch_msg.message.choice.c1.present = UL_DCCH_MessageType__c1_PR_securityModeComplete;
  else 
    ul_dcch_msg.message.choice.c1.present = UL_DCCH_MessageType__c1_PR_securityModeFailure;
  
  if (securityModeCommand->criticalExtensions.present == SecurityModeCommand__criticalExtensions_PR_c1) {
    if (securityModeCommand->criticalExtensions.choice.c1.present == SecurityModeCommand__criticalExtensions__c1_PR_securityModeCommand_r8) {
    
      ul_dcch_msg.message.choice.c1.choice.securityModeComplete.rrc_TransactionIdentifier = securityModeCommand->rrc_TransactionIdentifier;
      ul_dcch_msg.message.choice.c1.choice.securityModeComplete.criticalExtensions.present = SecurityModeCommand__criticalExtensions_PR_c1;
      ul_dcch_msg.message.choice.c1.choice.securityModeComplete.criticalExtensions.choice.securityModeComplete_r8.nonCriticalExtension =NULL; 

      LOG_I(RRC,"[UE %d] Frame %d: Receiving from SRB1 (DL-DCCH), encoding securityModeComplete (eNB %d)\n",
	    Mod_id,frame,eNB_index);

      enc_rval = uper_encode_to_buffer(&asn_DEF_UL_DCCH_Message,
				       (void*)&ul_dcch_msg,
				       buffer,
				       100);

#ifdef XER_PRINT
      xer_fprint(stdout, &asn_DEF_UL_DCCH_Message, (void*)&ul_dcch_msg);
#endif	  

#ifdef USER_MODE
      LOG_D(RRC, "securityModeComplete Encoded %d bits (%d bytes)\n", enc_rval.encoded, (enc_rval.encoded+7)/8);
#endif
      for (i = 0; i < (enc_rval.encoded + 7) / 8; i++)
        LOG_T(RRC, "%02x.", buffer[i]);
      LOG_T(RRC, "\n");
      pdcp_rrc_data_req (Mod_id + NB_eNB_INST, frame, 0, DCCH, rrc_mui++, 0, (enc_rval.encoded + 7) / 8, buffer, 1);
    }
  }
  
}
void rrc_ue_process_ueCapabilityEnquiry(uint8_t Mod_id,uint32_t frame,UECapabilityEnquiry_t *UECapabilityEnquiry,uint8_t eNB_index) {

  asn_enc_rval_t enc_rval;

  UL_DCCH_Message_t ul_dcch_msg;


  UE_CapabilityRAT_Container_t ue_CapabilityRAT_Container;

  uint8_t buffer[200];
  int i;

  LOG_I(RRC,"[UE %d] Frame %d: Receiving from SRB1 (DL-DCCH), Processing UECapabilityEnquiry (eNB %d)\n",
	Mod_id,frame,eNB_index);


  memset((void *)&ul_dcch_msg,0,sizeof(UL_DCCH_Message_t));
  memset((void *)&ue_CapabilityRAT_Container,0,sizeof(UE_CapabilityRAT_Container_t));

  ul_dcch_msg.message.present           = UL_DCCH_MessageType_PR_c1;
  ul_dcch_msg.message.choice.c1.present = UL_DCCH_MessageType__c1_PR_ueCapabilityInformation;
  ul_dcch_msg.message.choice.c1.choice.ueCapabilityInformation.rrc_TransactionIdentifier = UECapabilityEnquiry->rrc_TransactionIdentifier;

  ue_CapabilityRAT_Container.rat_Type = RAT_Type_eutra;
  OCTET_STRING_fromBuf(&ue_CapabilityRAT_Container.ueCapabilityRAT_Container,
		       (const char*)UE_rrc_inst[Mod_id].UECapability,
		       UE_rrc_inst[Mod_id].UECapability_size);
  //  ue_CapabilityRAT_Container.ueCapabilityRAT_Container.buf  = UE_rrc_inst[Mod_id].UECapability;
  // ue_CapabilityRAT_Container.ueCapabilityRAT_Container.size = UE_rrc_inst[Mod_id].UECapability_size;
  

  if (UECapabilityEnquiry->criticalExtensions.present == UECapabilityEnquiry__criticalExtensions_PR_c1) {
    if (UECapabilityEnquiry->criticalExtensions.choice.c1.present == UECapabilityEnquiry__criticalExtensions__c1_PR_ueCapabilityEnquiry_r8) {
      ul_dcch_msg.message.choice.c1.choice.ueCapabilityInformation.criticalExtensions.present           = UECapabilityInformation__criticalExtensions_PR_c1;
      ul_dcch_msg.message.choice.c1.choice.ueCapabilityInformation.criticalExtensions.choice.c1.present = UECapabilityInformation__criticalExtensions__c1_PR_ueCapabilityInformation_r8;
      ul_dcch_msg.message.choice.c1.choice.ueCapabilityInformation.criticalExtensions.choice.c1.choice.ueCapabilityInformation_r8.ue_CapabilityRAT_ContainerList.list.count=0;

      for (i=0;i<UECapabilityEnquiry->criticalExtensions.choice.c1.choice.ueCapabilityEnquiry_r8.ue_CapabilityRequest.list.count;i++) {

        if (*UECapabilityEnquiry->criticalExtensions.choice.c1.choice.ueCapabilityEnquiry_r8.ue_CapabilityRequest.list.array[i]
            == RAT_Type_eutra) {
          ASN_SEQUENCE_ADD(
              &ul_dcch_msg.message.choice.c1.choice.ueCapabilityInformation.criticalExtensions.choice.c1.choice.ueCapabilityInformation_r8.ue_CapabilityRAT_ContainerList.list,
              &ue_CapabilityRAT_Container);

          enc_rval = uper_encode_to_buffer(&asn_DEF_UL_DCCH_Message, (void*) &ul_dcch_msg, buffer, 100);

#ifdef XER_PRINT
          xer_fprint(stdout, &asn_DEF_UL_DCCH_Message, (void*)&ul_dcch_msg);
#endif	  

#ifdef USER_MODE
          LOG_D(RRC,"UECapabilityInformation Encoded %d bits (%d bytes)\n",enc_rval.encoded,(enc_rval.encoded+7)/8);
#endif 
          for (i = 0; i < (enc_rval.encoded + 7) / 8; i++)
            LOG_T(RRC, "%02x.", buffer[i]);
          LOG_T(RRC, "\n");
          pdcp_rrc_data_req (Mod_id + NB_eNB_INST, frame, 0, DCCH, rrc_mui++, 0, (enc_rval.encoded + 7) / 8, buffer, 1);
        }
      }
    }
  }
}


void rrc_ue_process_rrcConnectionReconfiguration(u8 Mod_id, u32 frame,
						 RRCConnectionReconfiguration_t *rrcConnectionReconfiguration,
						 u8 eNB_index) {

  LOG_I(RRC,"[UE %d] Frame %d: Receiving from SRB1 (DL-DCCH), Processing RRCConnectionReconfiguration (eNB %d)\n",
	Mod_id,frame,eNB_index);
  if (rrcConnectionReconfiguration->criticalExtensions.present == RRCConnectionReconfiguration__criticalExtensions_PR_c1) {
    if (rrcConnectionReconfiguration->criticalExtensions.choice.c1.present == RRCConnectionReconfiguration__criticalExtensions__c1_PR_rrcConnectionReconfiguration_r8) {

      if (rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.mobilityControlInfo) {
	LOG_I(RRC,"Mobility Control Information is present\n");
	rrc_ue_process_mobilityControlInfo(Mod_id,frame, eNB_index,
					   rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.mobilityControlInfo);

      }
      if (rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig != NULL) {
	LOG_I(RRC,"Measurement Configuration is present\n");
	rrc_ue_process_measConfig(Mod_id,frame, eNB_index,
				  rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.measConfig);
      }
      if (rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated) {
	LOG_I(RRC,"Radio Resource Configuration is present\n");
	rrc_ue_process_radioResourceConfigDedicated(Mod_id,frame,eNB_index,
                                                    rrcConnectionReconfiguration->criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.radioResourceConfigDedicated);
      }
    } // c1 present
  } // critical extensions present
}

/* 36.331, 5.3.5.4	Reception of an RRCConnectionReconfiguration including the mobilityControlInfo by the UE (handover) */
void   rrc_ue_process_mobilityControlInfo(u8 Mod_id,u32 frame, u8 eNB_index,struct MobilityControlInfo *mobilityControlInfo) {
  /*
  DRB_ToReleaseList_t*  drb2release_list;
  DRB_Identity_t *lcid;
  */
  LOG_N(RRC,"Note: This function needs some updates\n");
  if(UE_rrc_inst[Mod_id].Info[eNB_index].T310_active == 1)
    UE_rrc_inst[Mod_id].Info[eNB_index].T310_active = 0;
  UE_rrc_inst[Mod_id].Info[eNB_index].T304_active = 1;
  UE_rrc_inst[Mod_id].Info[eNB_index].T304_cnt = T304[mobilityControlInfo->t304];

  /*
  drb2release_list = CALLOC (1, sizeof (*drb2release_list));
  lcid= CALLOC (1, sizeof (DRB_Identity_t)); // long
  for (*lcid=0;*lcid<NB_RB_MAX;*lcid++)
  {
    ASN_SEQUENCE_ADD (&(drb2release_list)->list,lcid);
  }
  */

  //Removing SRB1 and SRB2 and DRB0
  LOG_N(RRC,"[UE %d] : Update needed for rrc_pdcp_config_req (deprecated) and rrc_rlc_config_req commands(deprecated)\n",Mod_id);
  rrc_pdcp_config_req (Mod_id+NB_eNB_INST, frame, 0, ACTION_REMOVE, Mod_id+DCCH,UNDEF_SECURITY_MODE);
  rrc_rlc_config_req(Mod_id+NB_eNB_INST,frame,0,ACTION_REMOVE,Mod_id+DCCH,SIGNALLING_RADIO_BEARER,Rlc_info_am_config);

  rrc_pdcp_config_req (Mod_id+NB_eNB_INST, frame, 0, ACTION_REMOVE, Mod_id+DCCH1,UNDEF_SECURITY_MODE);
  rrc_rlc_config_req(Mod_id+NB_eNB_INST,frame,0,ACTION_REMOVE,Mod_id+DCCH1,SIGNALLING_RADIO_BEARER,Rlc_info_am_config);

  rrc_pdcp_config_req (Mod_id+NB_eNB_INST, frame, 0, ACTION_REMOVE, Mod_id+DTCH,UNDEF_SECURITY_MODE);
  rrc_rlc_config_req(Mod_id+NB_eNB_INST,frame,0,ACTION_REMOVE,Mod_id+DTCH,RADIO_ACCESS_BEARER,Rlc_info_um);
  /*
  rrc_pdcp_config_asn1_req(NB_eNB_INST+Mod_id,frame, 0,eNB_index,
			   NULL, // SRB_ToAddModList
			   NULL, // DRB_ToAddModList
			   drb2release_list,
			   0, // security mode
			   NULL, // key rrc encryption
			   NULL, // key rrc integrity
			   NULL // key encryption
#ifdef Rel10
			   ,NULL
#endif
			   );
    
  rrc_rlc_config_asn1_req(NB_eNB_INST+Mod_id, frame,0,eNB_index,
			  NULL,// SRB_ToAddModList
			  NULL,// DRB_ToAddModList
			  drb2release_list // DRB_ToReleaseList
#ifdef Rel10
			  ,NULL
#endif 
			  );
   */
  

  //A little cleanup at RRC...
  //Copying current queue config to free RRC index
  /*
    memcpy((void *)UE_rrc_inst[Mod_id].SRB1_config[~(7<<eNB_index)],(void *)UE_rrc_inst[Mod_id].SRB1_config[7<<eNB_index],sizeof(SRB_ToAddMod_t));
    memcpy((void *)UE_rrc_inst[Mod_id].SRB2_config[~(7<<eNB_index)],(void *)UE_rrc_inst[Mod_id].SRB2_config[7<<eNB_index],sizeof(SRB_ToAddMod_t));
    memcpy((void *)UE_rrc_inst[Mod_id].DRB_config[~(7<<eNB_index)][0],(void *)UE_rrc_inst[Mod_id].DRB_config[7<<eNB_index][0],sizeof(DRB_ToAddMod_t));
  */
  /*
  LOG_N(RRC,"Not sure if Freeing the current queue config works properly: Fix me\n");
  free((void *)&UE_rrc_inst[Mod_id].SRB1_config[eNB_index]);
  free((void *)&UE_rrc_inst[Mod_id].SRB2_config[eNB_index]);
  free((void *)&UE_rrc_inst[Mod_id].DRB_config[eNB_index][0]);
 
  UE_rrc_inst[Mod_id].SRB1_config[eNB_index] = NULL;
  UE_rrc_inst[Mod_id].SRB2_config[eNB_index] = NULL;
  UE_rrc_inst[Mod_id].DRB_config[eNB_index][0] = NULL;
  */   
  //Synchronisation to DL of target cell
  LOG_D(RRC,"HO: Reset PDCP and RLC for configured RBs.. \n[MSC_MSG][FRAME %05d][RRC_UE][MOD %02d][][--- MAC_CONFIG_REQ  (SRB2 eNB %d) --->][MAC_UE][MOD %02d][]\n",
	frame, Mod_id, eNB_index, Mod_id);
  
    // Reset MAC and configure PHY
  rrc_mac_config_req(Mod_id,0,0,eNB_index,
		     (RadioResourceConfigCommonSIB_t *)NULL,
		     (struct PhysicalConfigDedicated *)NULL,
		     (MeasObjectToAddMod_t **)NULL,
		     (MAC_MainConfig_t *)NULL,
		     0,
		     (struct LogicalChannelConfig *)NULL,
		     (MeasGapConfig_t *)NULL,
		     (TDD_Config_t *)NULL,
		     mobilityControlInfo,
		     (u8 *)NULL,
		     (u16 *)NULL,
		     NULL,
		     NULL,
		     NULL,
		     NULL
#ifdef Rel10	       
		     ,0,
		     (MBSFN_AreaInfoList_r9_t *)NULL,
		     (PMCH_InfoList_r9_t *)NULL
#endif
#ifdef CBA
		     ,0,
		     0
#endif
		     );
  
  // Re-establish PDCP for all RBs that are established
  //	rrc_pdcp_config_req (Mod_id+NB_eNB_INST, frame, 0, ACTION_ADD, Mod_id+DCCH);
  //	rrc_pdcp_config_req (Mod_id+NB_eNB_INST, frame, 0, ACTION_ADD, Mod_id+DCCH1);
  //	rrc_pdcp_config_req (Mod_id+NB_eNB_INST, frame, 0, ACTION_ADD, Mod_id+DTCH);
  
  
  // Re-establish RLC for all RBs that are established
  //	rrc_rlc_config_req(Mod_id+NB_eNB_INST,frame,0,ACTION_ADD,Mod_id+DCCH,SIGNALLING_RADIO_BEARER,Rlc_info_am_config);
  //	rrc_rlc_config_req(Mod_id+NB_eNB_INST,frame,0,ACTION_ADD,Mod_id+DCCH1,SIGNALLING_RADIO_BEARER,Rlc_info_am_config);
  //	rrc_rlc_config_req(Mod_id+NB_eNB_INST,frame,0,ACTION_ADD,Mod_id+DTCH,RADIO_ACCESS_BEARER,Rlc_info_um);
  
  UE_rrc_inst[Mod_id].Info[eNB_index].State = RRC_SI_RECEIVED;

}
void rrc_detach_from_eNB(u8 Mod_id,u8 eNB_index) {
	//UE_rrc_inst[Mod_id].DRB_config[eNB_index]
}

/*------------------------------------------------------------------------------------------*/
void  rrc_ue_decode_dcch(u8 Mod_id,u32 frame,u8 Srb_id, u8 *Buffer,u8 eNB_index){
  /*------------------------------------------------------------------------------------------*/

  //DL_DCCH_Message_t dldcchmsg;
  DL_DCCH_Message_t *dl_dcch_msg=NULL;//&dldcchmsg;
  //  asn_dec_rval_t dec_rval;
  // int i;
  u8 target_eNB_index=0xFF;
  if (Srb_id != 1) {
    LOG_E(RRC,"[UE %d] Frame %d: Received message on DL-DCCH (SRB1), should not have ...\n",Mod_id,frame);
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
	      RRC_BUF_SIZE,0,0);

#ifdef XER_PRINT
  xer_fprint(stdout,&asn_DEF_DL_DCCH_Message,(void*)dl_dcch_msg);
#endif

#if defined(ENABLE_ITTI)
# if defined(DISABLE_ITTI_XER_PRINT)
  {
    MessageDef *message_p;

    message_p = itti_alloc_new_message (TASK_RRC_UE, RRC_DL_DCCH_MESSAGE);
    memcpy (&message_p->ittiMsg, (void *) dl_dcch_msg, sizeof(RrcDlDcchMessage));

    itti_send_msg_to_task (TASK_UNKNOWN, Mod_id + NB_eNB_INST, message_p);
  }
# else
  {
    char        message_string[20000];
    size_t      message_string_size;

    if ((message_string_size = xer_sprint(message_string, sizeof(message_string), &asn_DEF_DL_DCCH_Message, (void *)dl_dcch_msg)) > 0)
    {
      MessageDef *message_p;

      message_p = itti_alloc_new_message_sized (TASK_RRC_UE, GENERIC_LOG, message_string_size);
      memcpy(&message_p->ittiMsg.generic_log, message_string, message_string_size);

      itti_send_msg_to_task(TASK_UNKNOWN, Mod_id + NB_eNB_INST, message_p);
    }
  }
# endif
#endif

  if (dl_dcch_msg->message.present == DL_DCCH_MessageType_PR_c1) {

    if (UE_rrc_inst[Mod_id].Info[eNB_index].State >= RRC_CONNECTED) {

      switch (dl_dcch_msg->message.choice.c1.present) {

        case DL_DCCH_MessageType__c1_PR_NOTHING:
          LOG_I(RRC, "[UE %d] Frame %d : Received PR_NOTHING on DL-DCCH-Message\n", Mod_id, frame);
          return;

        case DL_DCCH_MessageType__c1_PR_csfbParametersResponseCDMA2000:
          break;

        case DL_DCCH_MessageType__c1_PR_dlInformationTransfer: {
#if defined(ENABLE_ITTI)
          DLInformationTransfer_t *dlInformationTransfer = &dl_dcch_msg->message.choice.c1.choice.dlInformationTransfer;

          if ((dlInformationTransfer->criticalExtensions.present == DLInformationTransfer__criticalExtensions_PR_c1)
              && (dlInformationTransfer->criticalExtensions.choice.c1.present
                  == DLInformationTransfer__criticalExtensions__c1_PR_dlInformationTransfer_r8)
              && (dlInformationTransfer->criticalExtensions.choice.c1.choice.dlInformationTransfer_r8.dedicatedInfoType.present
                  == DLInformationTransfer_r8_IEs__dedicatedInfoType_PR_dedicatedInfoNAS)) {
            /* This message hold a dedicated info NAS payload, forward it to NAS */
            struct DLInformationTransfer_r8_IEs__dedicatedInfoType *dedicatedInfoType =
                &dlInformationTransfer->criticalExtensions.choice.c1.choice.dlInformationTransfer_r8.dedicatedInfoType;
            uint32_t pdu_length;
            uint8_t *pdu_buffer;
            MessageDef *msg_p;

            pdu_length = dedicatedInfoType->choice.dedicatedInfoNAS.size;
            pdu_buffer = dedicatedInfoType->choice.dedicatedInfoNAS.buf;

            msg_p = itti_alloc_new_message(TASK_RRC_UE, NAS_DOWNLINK_DATA_IND);
            NAS_DOWNLINK_DATA_IND(msg_p).UEid = Mod_id; // TODO set the UEid to something else ?
            NAS_DOWNLINK_DATA_IND(msg_p).nasMsg.length = pdu_length;
            NAS_DOWNLINK_DATA_IND(msg_p).nasMsg.data = pdu_buffer;

            itti_send_msg_to_task(TASK_NAS_UE, Mod_id + NB_eNB_INST, msg_p);
          }
#endif
          break;
        }

        case DL_DCCH_MessageType__c1_PR_handoverFromEUTRAPreparationRequest:
          break;

        case DL_DCCH_MessageType__c1_PR_mobilityFromEUTRACommand:
          break;

        case DL_DCCH_MessageType__c1_PR_rrcConnectionReconfiguration:
          // first check if mobilityControlInfo  is present
          if (dl_dcch_msg->message.choice.c1.choice.rrcConnectionReconfiguration.criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.mobilityControlInfo
              != NULL) {
            /* 36.331, 5.3.5.4 Reception of an RRCConnectionReconfiguration including the mobilityControlInfo by the UE (handover)*/
            if (UE_rrc_inst[Mod_id].HandoverInfoUe.targetCellId
                != dl_dcch_msg->message.choice.c1.choice.rrcConnectionReconfiguration.criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.mobilityControlInfo->targetPhysCellId) {
              LOG_W(RRC,
                  "[UE %d] Frame %d: Handover target (%d) is different from RSRP measured target (%d)..\n", Mod_id, frame, dl_dcch_msg->message.choice.c1.choice.rrcConnectionReconfiguration.criticalExtensions.choice.c1.choice.rrcConnectionReconfiguration_r8.mobilityControlInfo->targetPhysCellId, UE_rrc_inst[Mod_id].HandoverInfoUe.targetCellId);
              return;
            }
            else
              if ((target_eNB_index = get_adjacent_cell_mod_id(UE_rrc_inst[Mod_id].HandoverInfoUe.targetCellId))
                  == 0xFF) {
                LOG_W(RRC,
                    "[UE %d] Frame %d: Mod_id of the target eNB not found, check the network topology\n", Mod_id, frame);
                return;
              }
              else {
                LOG_I(RRC,
                    "[UE% d] Frame %d: Received rrcConnectionReconfiguration with mobilityControlInfo \n", Mod_id, frame);
                UE_rrc_inst[Mod_id].HandoverInfoUe.measFlag = 1; // Ready to send more MeasReports if required
              }
          }
          rrc_ue_process_rrcConnectionReconfiguration(Mod_id, frame,
              &dl_dcch_msg->message.choice.c1.choice.rrcConnectionReconfiguration, eNB_index);
          if (target_eNB_index != 0xFF) {
            rrc_ue_generate_RRCConnectionReconfigurationComplete(Mod_id, frame, target_eNB_index,
                dl_dcch_msg->message.choice.c1.choice.rrcConnectionReconfiguration.rrc_TransactionIdentifier);
            UE_rrc_inst[Mod_id].Info[eNB_index].State = RRC_HO_EXECUTION;
            UE_rrc_inst[Mod_id].Info[target_eNB_index].State = RRC_RECONFIGURED;
            LOG_I(RRC, "[UE %d] State = RRC_RECONFIGURED during HO (eNB %d)\n", Mod_id, target_eNB_index);
          }
          else {
            rrc_ue_generate_RRCConnectionReconfigurationComplete(Mod_id, frame, eNB_index,
                dl_dcch_msg->message.choice.c1.choice.rrcConnectionReconfiguration.rrc_TransactionIdentifier);
            UE_rrc_inst[Mod_id].Info[eNB_index].State = RRC_RECONFIGURED;
            LOG_I(RRC, "[UE %d] State = RRC_RECONFIGURED (eNB %d)\n", Mod_id, eNB_index);
          }
          break;

        case DL_DCCH_MessageType__c1_PR_rrcConnectionRelease:
          break;

        case DL_DCCH_MessageType__c1_PR_securityModeCommand:
          LOG_I(RRC, "[UE %d] Received securityModeCommand (eNB %d)\n", Mod_id, eNB_index);
          rrc_ue_process_securityModeCommand(Mod_id, frame, &dl_dcch_msg->message.choice.c1.choice.securityModeCommand,
              eNB_index);
          break;

        case DL_DCCH_MessageType__c1_PR_ueCapabilityEnquiry:
          LOG_I(RRC, "[UE %d] Received Capability Enquiry (eNB %d)\n", Mod_id, eNB_index);
          rrc_ue_process_ueCapabilityEnquiry(Mod_id, frame, &dl_dcch_msg->message.choice.c1.choice.ueCapabilityEnquiry,
              eNB_index);
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

        default:
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
int siPeriod_int[7] = {80,160,320,640,1280,2560,5120};

int decode_BCCH_DLSCH_Message(u8 Mod_id,u32 frame,u8 eNB_index,u8 *Sdu,u8 Sdu_len) {

  //BCCH_DL_SCH_Message_t bcch_message;
  BCCH_DL_SCH_Message_t *bcch_message=NULL;//_ptr=&bcch_message;
  SystemInformationBlockType1_t **sib1=&UE_rrc_inst[Mod_id].sib1[eNB_index];
  SystemInformation_t **si=UE_rrc_inst[Mod_id].si[eNB_index];
  asn_dec_rval_t dec_rval;
  uint32_t si_window;//, sib1_decoded=0, si_decoded=0;
  // int i;

  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_DECODE_BCCH, VCD_FUNCTION_IN);


  if ((UE_rrc_inst[Mod_id].Info[eNB_index].SIB1Status == 1) &&
      (UE_rrc_inst[Mod_id].Info[eNB_index].SIStatus == 1)) {
    // Avoid decoding to prevent memory bloating
    vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_DECODE_BCCH, VCD_FUNCTION_OUT);
    return 0;
  } 
  else {

    //memset(&bcch_message,0,sizeof(BCCH_DL_SCH_Message_t));
    //  LOG_D(RRC,"[UE %d] Decoding DL_BCCH_DLSCH_Message\n",Mod_id)
    /*
    for (i=0;i<Sdu_len;i++)
      printf("%x.",Sdu[i]);
      printf("\n");*/
    dec_rval = uper_decode_complete(NULL,
				    &asn_DEF_BCCH_DL_SCH_Message,
				    (void **)&bcch_message,
				    (const void *)Sdu,
				    Sdu_len);//,0,0);
    
    if ((dec_rval.code != RC_OK) && (dec_rval.consumed==0)) {
      LOG_E(RRC,"[UE %d] Failed to decode BCCH_DLSCH_MESSAGE (%d bits)\n",Mod_id,dec_rval.consumed);
      //free the memory
      SEQUENCE_free(&asn_DEF_BCCH_DL_SCH_Message, (void*)bcch_message, 1);
      vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_DECODE_BCCH, VCD_FUNCTION_OUT);
      return -1;
    }
    //  xer_fprint(stdout,  &asn_DEF_BCCH_DL_SCH_Message, (void*)&bcch_message);

#if defined(ENABLE_ITTI)
# if defined(DISABLE_ITTI_XER_PRINT)
  {
    MessageDef *message_p;

    message_p = itti_alloc_new_message (TASK_RRC_UE, RRC_DL_BCCH_MESSAGE);
    memcpy (&message_p->ittiMsg, (void *) bcch_message, sizeof(RrcDlBcchMessage));

    itti_send_msg_to_task (TASK_UNKNOWN, Mod_id + NB_eNB_INST, message_p);
  }
# else
  {
    char        message_string[15000];
    size_t      message_string_size;

    if ((message_string_size = xer_sprint(message_string, sizeof(message_string), &asn_DEF_BCCH_DL_SCH_Message, (void *)bcch_message)) > 0)
    {
      MessageDef *message_p;

      message_p = itti_alloc_new_message_sized (TASK_RRC_UE, GENERIC_LOG, message_string_size);
      memcpy(&message_p->ittiMsg.generic_log, message_string, message_string_size);

      itti_send_msg_to_task(TASK_UNKNOWN, Mod_id + NB_eNB_INST, message_p);
    }
  }
# endif
#endif

    if (bcch_message->message.present == BCCH_DL_SCH_MessageType_PR_c1) {
      switch (bcch_message->message.choice.c1.present) {
      case BCCH_DL_SCH_MessageType__c1_PR_systemInformationBlockType1:
	if ((frame %2) == 0) {
	  if (UE_rrc_inst[Mod_id].Info[eNB_index].SIB1Status == 0) {
	    memcpy((void*)*sib1,
		   (void*)&bcch_message->message.choice.c1.choice.systemInformationBlockType1,
		   sizeof(SystemInformationBlockType1_t));
	    LOG_D(RRC,"[UE %d] Decoding First SIB1\n",Mod_id);
	    decode_SIB1(Mod_id,eNB_index);
	    //mac_xface->macphy_exit("after decode_SIB1");

	  }
	}
	break;
      case BCCH_DL_SCH_MessageType__c1_PR_systemInformation:
	if ((UE_rrc_inst[Mod_id].Info[eNB_index].SIB1Status == 1) &&
	    (UE_rrc_inst[Mod_id].Info[eNB_index].SIStatus == 0)) {
	  //	  if ((frame %8) == 1) {  // check only in odd frames for SI
	    si_window = (frame%(UE_rrc_inst[Mod_id].Info[eNB_index].SIperiod/10))/(UE_rrc_inst[Mod_id].Info[eNB_index].SIwindowsize/10);
	    memcpy((void*)si[si_window],
		   (void*)&bcch_message->message.choice.c1.choice.systemInformation,
		   sizeof(SystemInformation_t));
	    LOG_D(RRC,"[UE %d] Decoding SI for frame %d, si_window %d\n",Mod_id,frame,si_window);
	    decode_SI(Mod_id,frame,eNB_index,si_window);
	    //mac_xface->macphy_exit("after decode_SI");
	    
	    //	  }
	}
	break;
      case BCCH_DL_SCH_MessageType__c1_PR_NOTHING:
      default:
	break;
      }
    }
  }
  /*  if ((UE_rrc_inst[Mod_id].Info[eNB_index].SIB1Status == 1) &&
      (UE_rrc_inst[Mod_id].Info[eNB_index].SIStatus == 1) && (frame >= Mod_id * 20 + 10))
      SEQUENCE_free(&asn_DEF_BCCH_DL_SCH_Message, (void*)bcch_message, 0);*/
  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_DECODE_BCCH, VCD_FUNCTION_OUT);

  return 0;
}


int decode_SIB1(u8 Mod_id,u8 eNB_index) {
  SystemInformationBlockType1_t **sib1=&UE_rrc_inst[Mod_id].sib1[eNB_index];
  int i;

  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_RRC_UE_DECODE_SIB1, VCD_FUNCTION_IN);

  LOG_D(RRC,"[UE %d] : Dumping SIB 1\n",Mod_id);

  //  xer_fprint(stdout,&asn_DEF_SystemInformationBlockType1, (void*)*sib1);

  LOG_D(RRC,"cellAccessRelatedInfo.cellIdentity : %x.%x.%x.%x\n",
      (*sib1)->cellAccessRelatedInfo.cellIdentity.buf[0],
      (*sib1)->cellAccessRelatedInfo.cellIdentity.buf[1],
      (*sib1)->cellAccessRelatedInfo.cellIdentity.buf[2],
      (*sib1)->cellAccessRelatedInfo.cellIdentity.buf[3]);

  LOG_D(RRC,"cellSelectionInfo.q_RxLevMin       : %d\n",(int)(*sib1)->cellSelectionInfo.q_RxLevMin);
  LOG_D(RRC,"freqBandIndicator                  : %d\n",(int)(*sib1)->freqBandIndicator);
  LOG_D(RRC,"siWindowLength                     : %s\n",siWindowLength[(*sib1)->si_WindowLength]);
  if ((*sib1)->schedulingInfoList.list.count>0) {
    for (i=0;i<(*sib1)->schedulingInfoList.list.count;i++) {
      LOG_D(RRC,"siSchedulingInfoPeriod[%d]          : %s\n",i,SIBPeriod[(int)(*sib1)->schedulingInfoList.list.array[i]->si_Periodicity]);
      if ((*sib1)->schedulingInfoList.list.array[i]->sib_MappingInfo.list.count>0)
	LOG_D(RRC,"siSchedulingInfoSIBType[%d]         : %s\n",i,SIBType[(int)(*(*sib1)->schedulingInfoList.list.array[i]->sib_MappingInfo.list.array[0])]);
      else {
	LOG_W(RRC,"mapping list %d is null\n",i);
      }
    }
  }
  else {
   LOG_E(RRC,"siSchedulingInfoPeriod[0]          : PROBLEM!!!\n");
   return -1;
  }

  if ((*sib1)->tdd_Config) {
    LOG_D(RRC,"TDD subframe assignment            : %d\n",(int)(*sib1)->tdd_Config->subframeAssignment);
    LOG_D(RRC,"S-Subframe Config                  : %d\n",(int)(*sib1)->tdd_Config->specialSubframePatterns);
  }

  UE_rrc_inst[Mod_id].Info[eNB_index].SIperiod     = siPeriod_int[(*sib1)->schedulingInfoList.list.array[0]->si_Periodicity];
  UE_rrc_inst[Mod_id].Info[eNB_index].SIwindowsize = siWindowLength_int[(*sib1)->si_WindowLength];
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
		     (MobilityControlInfo_t *) NULL,
		     &UE_rrc_inst[Mod_id].Info[eNB_index].SIwindowsize,
		     &UE_rrc_inst[Mod_id].Info[eNB_index].SIperiod,
		     NULL,
		     NULL,
		     NULL,
		     (MBSFN_SubframeConfigList_t *)NULL
#ifdef Rel10	       
		     ,0,
		     (MBSFN_AreaInfoList_r9_t *)NULL,
		     (PMCH_InfoList_r9_t *)NULL
#endif
#ifdef CBA
		     ,
		     0,
		     0
#endif
		     );

  UE_rrc_inst[Mod_id].Info[eNB_index].SIB1Status = 1;
  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_RRC_UE_DECODE_SIB1, VCD_FUNCTION_OUT);
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
  LOG_D(RRC,"freqInfo.ul_CarrierFreq : %p\n", sib2->freqInfo.ul_CarrierFreq);
  LOG_D(RRC,"freqInfo.ul_Bandwidth : %p\n", sib2->freqInfo.ul_Bandwidth);
  LOG_D(RRC,"mbsfn_SubframeConfigList : %p\n", sib2->mbsfn_SubframeConfigList);
  LOG_D(RRC,"timeAlignmentTimerCommon : %ld\n", sib2->timeAlignmentTimerCommon);



}

void dump_sib3(SystemInformationBlockType3_t *sib3) {

}

#ifdef Rel10
void dump_sib13(SystemInformationBlockType13_r9_t *sib13) {

  LOG_D(RRC,"[RRC][UE] Dumping SIB13\n");
  LOG_D(RRC,"[RRC][UE] dumping sib13 second time\n");
  LOG_D(RRC,"[RRC][UE] NotificationRepetitionCoeff-r9 : %ld\n", sib13->notificationConfig_r9.notificationRepetitionCoeff_r9);
  LOG_D(RRC,"[RRC][UE] NotificationOffset-r9 : %d\n", (int)sib13->notificationConfig_r9.notificationOffset_r9);
  LOG_D(RRC,"[RRC][UE] NotificationSF-Index-r9 : %d\n", (int)sib13->notificationConfig_r9.notificationSF_Index_r9);

}
#endif

//const char SIBPeriod[7][7]= {"80ms\0","160ms\0","320ms\0","640ms\0","1280ms\0","2560ms\0","5120ms\0"};
int decode_SI(u8 Mod_id,u32 frame,u8 eNB_index,u8 si_window) {

  SystemInformation_t **si=&UE_rrc_inst[Mod_id].si[eNB_index][si_window];
  int i;
  struct SystemInformation_r8_IEs_sib_TypeAndInfo_Member *typeandinfo;

  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_RRC_UE_DECODE_SI  , VCD_FUNCTION_IN);

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
    LOG_D(RRC,"SI count %d\n",i);
    typeandinfo=(*si)->criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list.array[i];

    switch(typeandinfo->present) {
    case SystemInformation_r8_IEs_sib_TypeAndInfo_Member_PR_sib2:
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
			 (MobilityControlInfo_t *)NULL,
			 NULL,
			 NULL,
			 UE_rrc_inst[Mod_id].sib2[eNB_index]->freqInfo.ul_CarrierFreq,
			 UE_rrc_inst[Mod_id].sib2[eNB_index]->freqInfo.ul_Bandwidth,
			 &UE_rrc_inst[Mod_id].sib2[eNB_index]->freqInfo.additionalSpectrumEmission,
			 UE_rrc_inst[Mod_id].sib2[eNB_index]->mbsfn_SubframeConfigList
#ifdef Rel10	       
			 ,0,
			 (MBSFN_AreaInfoList_r9_t *)NULL,
			 (PMCH_InfoList_r9_t *)NULL
#endif
#ifdef CBA
			 ,0,
			 0
#endif
			 );
      UE_rrc_inst[Mod_id].Info[eNB_index].SIStatus = 1;
      // After SI is received, prepare RRCConnectionRequest
#ifdef Rel10
      if (UE_rrc_inst[Mod_id].MBMS_flag < 3) // see -Q option
#endif
	rrc_ue_generate_RRCConnectionRequest(Mod_id,frame,eNB_index);
      LOG_I(RRC, "not sending connection request\n");

      if (UE_rrc_inst[Mod_id].Info[eNB_index].State == RRC_IDLE) {
	LOG_I(RRC,"[UE %d] Received SIB1/SIB2/SIB3 Switching to RRC_SI_RECEIVED\n",Mod_id);
	UE_rrc_inst[Mod_id].Info[eNB_index].State = RRC_SI_RECEIVED;
      }
      break;
    case SystemInformation_r8_IEs_sib_TypeAndInfo_Member_PR_sib3:
      UE_rrc_inst[Mod_id].sib3[eNB_index] = &typeandinfo->choice.sib3;
      LOG_I(RRC,"[UE %d] Frame %d Found SIB3 from eNB %d\n",Mod_id,frame,eNB_index);
      dump_sib3(UE_rrc_inst[Mod_id].sib3[eNB_index]);
      UE_rrc_inst[Mod_id].Info[eNB_index].SIStatus = 1;
      break;
    case SystemInformation_r8_IEs_sib_TypeAndInfo_Member_PR_sib4:
      UE_rrc_inst[Mod_id].sib4[eNB_index] = &typeandinfo->choice.sib4;
      LOG_I(RRC,"[UE %d] Frame %d Found SIB4 from eNB %d\n",Mod_id,frame,eNB_index);
      break;
    case SystemInformation_r8_IEs_sib_TypeAndInfo_Member_PR_sib5:
      UE_rrc_inst[Mod_id].sib5[eNB_index] = &typeandinfo->choice.sib5;
      LOG_I(RRC,"[UE %d] Found SIB5 from eNB %d\n",Mod_id,eNB_index);
      break;
    case SystemInformation_r8_IEs_sib_TypeAndInfo_Member_PR_sib6:
      UE_rrc_inst[Mod_id].sib6[eNB_index] = &typeandinfo->choice.sib6;
      LOG_I(RRC,"[UE %d] Found SIB6 from eNB %d\n",Mod_id,eNB_index);
      break;
    case SystemInformation_r8_IEs_sib_TypeAndInfo_Member_PR_sib7:
      UE_rrc_inst[Mod_id].sib7[eNB_index] = &typeandinfo->choice.sib7;
      LOG_I(RRC,"[UE %d] Found SIB7 from eNB %d\n",Mod_id,eNB_index);
      break;
    case SystemInformation_r8_IEs_sib_TypeAndInfo_Member_PR_sib8:
      UE_rrc_inst[Mod_id].sib8[eNB_index] = &typeandinfo->choice.sib8;
      LOG_I(RRC,"[UE %d] Found SIB8 from eNB %d\n",Mod_id,eNB_index);
      break;
    case SystemInformation_r8_IEs_sib_TypeAndInfo_Member_PR_sib9:
      UE_rrc_inst[Mod_id].sib9[eNB_index] = &typeandinfo->choice.sib9;
      LOG_I(RRC,"[UE %d] Found SIB9 from eNB %d\n",Mod_id,eNB_index);
      break;
    case SystemInformation_r8_IEs_sib_TypeAndInfo_Member_PR_sib10:
      UE_rrc_inst[Mod_id].sib10[eNB_index] = &typeandinfo->choice.sib10;
      LOG_I(RRC,"[UE %d] Found SIB10 from eNB %d\n",Mod_id,eNB_index);
      break;
    case SystemInformation_r8_IEs_sib_TypeAndInfo_Member_PR_sib11:
      UE_rrc_inst[Mod_id].sib11[eNB_index] = &typeandinfo->choice.sib11;
      LOG_I(RRC,"[UE %d] Found SIB11 from eNB %d\n",Mod_id,eNB_index);
      break;
#ifdef Rel10
    case SystemInformation_r8_IEs_sib_TypeAndInfo_Member_PR_sib12_v920:
      UE_rrc_inst[Mod_id].sib12[eNB_index] = &typeandinfo->choice.sib12_v920;
      LOG_I(RRC,"[RRC][UE %d] Found SIB12 from eNB %d\n",Mod_id,eNB_index);

      break;
    case SystemInformation_r8_IEs_sib_TypeAndInfo_Member_PR_sib13_v920:
      UE_rrc_inst[Mod_id].sib13[eNB_index] = &typeandinfo->choice.sib13_v920;
      LOG_I(RRC,"[RRC][UE %d] Found SIB13 from eNB %d\n",Mod_id,eNB_index);
      dump_sib13(UE_rrc_inst[Mod_id].sib13[eNB_index]);
      // adding here function to store necessary parameters for using in decode_MCCH_Message + maybe transfer to PHY layer
      LOG_D(RRC, "[MSC_MSG][FRAME %05d][RRC_UE][MOD %02d][][--- MAC_CONFIG_REQ (SIB13 params eNB %d) --->][MAC_UE][MOD %02d][]\n",
            frame, Mod_id, eNB_index, Mod_id);
      rrc_mac_config_req(Mod_id,0,0,eNB_index,
			 (RadioResourceConfigCommonSIB_t *)NULL,
			 (struct PhysicalConfigDedicated *)NULL,
			 (MeasObjectToAddMod_t **)NULL,
			 (MAC_MainConfig_t *)NULL,
			 0,
			 (struct LogicalChannelConfig *)NULL,
			 (MeasGapConfig_t *)NULL,
			 (TDD_Config_t *)NULL,
			 (MobilityControlInfo_t *)NULL,
			 NULL,
			 NULL,
			 NULL,
			 NULL,
			 NULL,
			 (MBSFN_SubframeConfigList_t *)NULL
#ifdef Rel10	       
			 ,0,
			 &UE_rrc_inst[Mod_id].sib13[eNB_index]->mbsfn_AreaInfoList_r9,
			 (PMCH_InfoList_r9_t *)NULL
#endif
#ifdef CBA
			 ,0,
			 0
#endif
			 );
      UE_rrc_inst[Mod_id].Info[eNB_index].SIStatus = 1;
      break;
#endif
    default:
      break;
    }

  }

  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_RRC_UE_DECODE_SI  , VCD_FUNCTION_OUT);
  return 0;
}

// layer 3 filtering of RSRP (EUTRA) measurements: 36.331, Sec. 5.5.3.2
void ue_meas_filtering(u8 Mod_id,u32 frame,u8 eNB_index){
  float a = UE_rrc_inst[Mod_id].filter_coeff_rsrp; // 'a' in 36.331 Sec. 5.5.3.2
  float a1 = UE_rrc_inst[Mod_id].filter_coeff_rsrq;
  //float rsrp_db, rsrq_db;
  u8 eNB_offset;

  if(UE_rrc_inst[Mod_id].QuantityConfig[0] != NULL) { // Only consider 1 serving cell (index: 0)
    if (UE_rrc_inst[Mod_id].QuantityConfig[0]->quantityConfigEUTRA != NULL) {
      if(UE_rrc_inst[Mod_id].QuantityConfig[0]->quantityConfigEUTRA->filterCoefficientRSRP != NULL) {
	for (eNB_offset = 0;eNB_offset<1+mac_xface->get_n_adj_cells(Mod_id);eNB_offset++) {
	  //filter_factor = 1/power(2,*UE_rrc_inst[Mod_id].QuantityConfig[0]->quantityConfigEUTRA->filterCoefficientRSRP/4);
	  // LOG_N(RRC,"[UE %d] Frame %d : check proper operation in abstraction mode rsrp (%d), rx gain (%d) N_RB_DL (%d)\n",
	  //	Mod_id,frame,mac_xface->get_RSRP(Mod_id,eNB_offset),mac_xface->get_rx_total_gain_dB(Mod_id),mac_xface->lte_frame_parms->N_RB_DL);
	  UE_rrc_inst[Mod_id].rsrp_db[eNB_offset] = (dB_fixed_times10(mac_xface->get_RSRP(Mod_id,eNB_offset))/10.0)-mac_xface->get_rx_total_gain_dB(Mod_id)-dB_fixed(mac_xface->lte_frame_parms->N_RB_DL*12);
	  UE_rrc_inst[Mod_id].rsrp_db_filtered[eNB_offset] = (1.0-a)*UE_rrc_inst[Mod_id].rsrp_db_filtered[eNB_offset] +  a*UE_rrc_inst[Mod_id].rsrp_db[eNB_offset];
	  //mac_xface->set_RSRP_filtered(Mod_id,eNB_offset,UE_rrc_inst[Mod_id].rsrp_db_filtered[eNB_offset]);
	  
	  LOG_D(RRC,"[UE %d] Frame %d: Meas RSRP: eNB_offset: %d rsrp_coef: %3.2f filter_coef: %d before L3 filtering: rsrp: %3.1f after L3 filtering: rsrp: %3.1f \n ",
		Mod_id, frame, eNB_offset,a,
		*UE_rrc_inst->QuantityConfig[0]->quantityConfigEUTRA->filterCoefficientRSRP,
		UE_rrc_inst[Mod_id].rsrp_db[eNB_offset],
		UE_rrc_inst[Mod_id].rsrp_db_filtered[eNB_offset]);
	}
      }
    }
    else {
      for (eNB_offset = 0;eNB_offset<1+mac_xface->get_n_adj_cells(Mod_id);eNB_offset++) {
	UE_rrc_inst[Mod_id].rsrp_db_filtered[eNB_offset]= mac_xface->get_RSRP(Mod_id,eNB_offset);
	// phy_vars_ue->PHY_measurements.rsrp_filtered[eNB_offset]=UE_rrc_inst[Mod_id].rsrp_db_filtered[eNB_offset];
	//mac_xface->set_RSRP_filtered(Mod_id,eNB_offset,UE_rrc_inst[Mod_id].rsrp_db_filtered[eNB_offset]);
      }
    }
    if (UE_rrc_inst[Mod_id].QuantityConfig[0]->quantityConfigEUTRA != NULL) {
      if(UE_rrc_inst[Mod_id].QuantityConfig[0]->quantityConfigEUTRA->filterCoefficientRSRQ != NULL) {
	for (eNB_offset = 0;eNB_offset<1+mac_xface->get_n_adj_cells(Mod_id);eNB_offset++) {
	  // LOG_N(RRC,"[UE %d] Frame %d : check if this operation workes properly in abstraction mode\n",Mod_id,frame);
	  UE_rrc_inst[Mod_id].rsrq_db[eNB_offset] = (10*log10(mac_xface->get_RSRQ(Mod_id,eNB_offset)))-20;
	  UE_rrc_inst[Mod_id].rsrq_db_filtered[eNB_offset]=(1-a1)*UE_rrc_inst[Mod_id].rsrq_db_filtered[eNB_offset] + a1 *UE_rrc_inst[Mod_id].rsrq_db[eNB_offset];
	  //mac_xface->set_RSRP_filtered(Mod_id,eNB_offset,UE_rrc_inst[Mod_id].rsrp_db_filtered[eNB_offset]);
	  /*
	  LOG_D(RRC,"[UE %d] meas RSRQ: eNB_offset: %d rsrq_coef: %3.2f filter_coef: %d before L3 filtering: rsrq: %3.1f after L3 filtering: rsrq: %3.1f \n ",
		Mod_id, eNB_offset,
		a1,
		*UE_rrc_inst->QuantityConfig[0]->quantityConfigEUTRA->filterCoefficientRSRQ,
		mac_xface->get_RSRQ(Mod_id,eNB_offset),
		UE_rrc_inst[Mod_id].rsrq_db[eNB_offset],
		UE_rrc_inst[Mod_id].rsrq_db_filtered[eNB_offset]);
	  */
	}
      }
    }
    else{
      for (eNB_offset = 0;eNB_offset<1+mac_xface->get_n_adj_cells(Mod_id);eNB_offset++) {
	UE_rrc_inst[Mod_id].rsrq_db_filtered[eNB_offset]= mac_xface->get_RSRQ(Mod_id,eNB_offset);
      }
    } 
  }
}

//Below routine implements Measurement Reporting procedure from 36.331 Section 5.5.5
void rrc_ue_generate_MeasurementReport(u8 Mod_id, u32 frame,u8 eNB_index) {

  u8 buffer[32], size;
  u8 i;
  u8 target_eNB_offset;
  MeasId_t measId;
  PhysCellId_t cellId, targetCellId;
  long rsrq_s,rsrp_t,rsrq_t;
  long rsrp_s, nElem, nElem1;
  float rsrp_filtered, rsrq_filtered;
  nElem = 100;
  nElem1 = 33;
  static u32 pframe=0;
  target_eNB_offset = UE_rrc_inst[Mod_id].Info[0].handoverTarget; // eNB_offset of target eNB: used to obtain the mod_id of target eNB

  for (i=0;i<MAX_MEAS_ID;i++) {
    if (UE_rrc_inst[Mod_id].measReportList[0][i] != NULL) {
      measId = UE_rrc_inst[Mod_id].measReportList[0][i]->measId;

      // Note: Values in the meas report have to be the mapped values...to implement binary search for LUT
      rsrp_filtered = UE_rrc_inst[Mod_id].rsrp_db_filtered[eNB_index];//nid_cell];
      rsrp_s = binary_search_float(RSRP_meas_mapping,nElem, rsrp_filtered); //mapped RSRP of serving cell

      rsrq_filtered = UE_rrc_inst[Mod_id].rsrq_db_filtered[eNB_index];//nid_cell]; //RSRQ of serving cell
      rsrq_s = binary_search_float(RSRQ_meas_mapping,nElem1,rsrp_filtered);//mapped RSRQ of serving cell

      LOG_D(RRC,"[UE %d] Frame %d: source eNB %d :rsrp_s: %d rsrq_s: %d tmp: %f tmp1: %f \n",
            Mod_id,frame,eNB_index, rsrp_s,rsrq_s,rsrp_filtered,rsrq_filtered);

      rsrp_t = binary_search_float(RSRP_meas_mapping,nElem,UE_rrc_inst[Mod_id].rsrp_db_filtered[target_eNB_offset]); //RSRP of target cell
      rsrq_t = binary_search_float(RSRQ_meas_mapping,nElem1,UE_rrc_inst[Mod_id].rsrq_db_filtered[target_eNB_offset]); //RSRQ of target cell

      //  if (measFlag == 1) {
      cellId = get_adjacent_cell_id(Mod_id,eNB_index); //PhycellId of serving cell
      targetCellId = UE_rrc_inst[Mod_id].HandoverInfoUe.targetCellId ;//get_adjacent_cell_id(Mod_id,target_eNB_offset); //PhycellId of target cell

      if (pframe!=frame){
        pframe=frame;
        size = do_MeasurementReport(buffer,measId,targetCellId,rsrp_s,rsrq_s,rsrp_t,rsrq_t);
        LOG_D(RRC,"[UE %d] Frame %d: Sending MeasReport: servingCell(%d) targetCell(%d) rsrp_s(%d) rsrq_s(%d) rsrp_t(%d) rsrq_t(%d) \n",
              Mod_id, frame, cellId,targetCellId,rsrp_s,rsrq_s,rsrp_t,rsrq_t);
        LOG_I(RRC,"[UE %d] Frame %d : Generating Measurement Report for eNB %d\n",Mod_id,frame,eNB_index);
        LOG_D(RLC,"[MSC_MSG][FRAME %05d][RRC_UE][MOD %02d][][--- PDCP_DATA_REQ/%d Bytes (MeasurementReport to eNB %d MUI %d) --->][PDCP][MOD %02d][RB %02d]\n",
              frame, Mod_id+NB_eNB_INST, size, eNB_index, rrc_mui, Mod_id+NB_eNB_INST, DCCH);
        pdcp_data_req(Mod_id+NB_eNB_INST,frame,0,DCCH,rrc_mui++,0,size,buffer,1);
        //LOG_D(RRC, "[UE %d] Frame %d Sending MeasReport (%d bytes) through DCCH%d to PDCP \n",Mod_id,frame, size, DCCH);
      }
      //          measFlag = 0; //re-setting measFlag so that no more MeasReports are sent in this frame
      //          }
    }
  }
}

// Measurement report triggering, described in 36.331 Section 5.5.4.1: called periodically 
void ue_measurement_report_triggering(u8 Mod_id, u32 frame,u8 eNB_index) {
  u8 i,j;
  Hysteresis_t	 hys;
  TimeToTrigger_t	 ttt_ms;
  Q_OffsetRange_t ofn;
  Q_OffsetRange_t ocn;
  Q_OffsetRange_t ofs = 0;
  Q_OffsetRange_t ocs = 0;
  long			a3_offset;
  MeasObjectId_t	 measObjId;
  ReportConfigId_t	 reportConfigId;
  
  for(i=0 ; i<NB_CNX_UE ; i++) {
    for(j=0 ; j<MAX_MEAS_ID ; j++) {
      if(UE_rrc_inst[Mod_id].MeasId[i][j] != NULL) {
	measObjId = UE_rrc_inst[Mod_id].MeasId[i][j]->measObjectId;
	reportConfigId = UE_rrc_inst[Mod_id].MeasId[i][j]->reportConfigId;
	if( /*UE_rrc_inst[Mod_id].MeasId[i][j] != NULL && */ UE_rrc_inst[Mod_id].MeasObj[i][measObjId-1] != NULL) {
	  if(UE_rrc_inst[Mod_id].MeasObj[i][measObjId-1]->measObject.present == MeasObjectToAddMod__measObject_PR_measObjectEUTRA) {
	    /* consider any neighboring cell detected on the associated frequency to be
	     * applicable when the concerned cell is not included in the blackCellsToAddModList
	     * defined within the VarMeasConfig for this measId */
	    //    LOG_I(RRC,"event %d %d %p \n", measObjId,reportConfigId, UE_rrc_inst[Mod_id].ReportConfig[i][reportConfigId-1]);
	    if((UE_rrc_inst[Mod_id].ReportConfig[i][reportConfigId-1] != NULL) &&
	       (UE_rrc_inst[Mod_id].ReportConfig[i][reportConfigId-1]->reportConfig.present==ReportConfigToAddMod__reportConfig_PR_reportConfigEUTRA) &&
	       (UE_rrc_inst[Mod_id].ReportConfig[i][reportConfigId-1]->reportConfig.choice.reportConfigEUTRA.triggerType.present == ReportConfigEUTRA__triggerType_PR_event)) {
	      hys = UE_rrc_inst[Mod_id].ReportConfig[i][reportConfigId-1]->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.hysteresis;
	      //LOG_N(RRC,"[UE%d] Frame %d Check below lines for segfault :), Fix me \n",Mod_id, frame);
	      ttt_ms = timeToTrigger_ms[UE_rrc_inst[Mod_id].ReportConfig[i][reportConfigId-1]->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.timeToTrigger];
	      // Freq specific offset of neighbor cell freq
	      ofn = ((UE_rrc_inst[Mod_id].MeasObj[i][measObjId-1]->measObject.choice.measObjectEUTRA.offsetFreq != NULL) ? 
		     *UE_rrc_inst[Mod_id].MeasObj[i][measObjId-1]->measObject.choice.measObjectEUTRA.offsetFreq : 15); //  /* 15 is the Default */
	      // cellIndividualOffset of neighbor cell - not defined yet
	      ocn = 0;
	      a3_offset = UE_rrc_inst[Mod_id].ReportConfig[i][reportConfigId-1]->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.choice.eventA3.a3_Offset;
	      
	      switch (UE_rrc_inst[Mod_id].ReportConfig[i][reportConfigId-1]->reportConfig.choice.reportConfigEUTRA.triggerType.choice.event.eventId.present) {
	      case ReportConfigEUTRA__triggerType__event__eventId_PR_eventA1:
		LOG_D(RRC,"[UE %d] Frame %d : A1 event: check if serving becomes better than threshold\n",Mod_id, frame);
		break;
	      case ReportConfigEUTRA__triggerType__event__eventId_PR_eventA2:
		LOG_D(RRC,"[UE %d] Frame %d : A2 event, check if serving becomes worse than a threshold\n",Mod_id, frame);
		break;
	      case ReportConfigEUTRA__triggerType__event__eventId_PR_eventA3:
		LOG_D(RRC,"[UE %d] Frame %d : A3 event: check if a neighboring cell becomes offset better than serving to trigger a measurement event \n",Mod_id, frame);
		if ((check_trigger_meas_event(Mod_id,frame,eNB_index,i,j,ofn,ocn,hys,ofs,ocs,a3_offset,ttt_ms)) && 
		    (UE_rrc_inst[Mod_id].Info[0].State >= RRC_CONNECTED) && 
		    (UE_rrc_inst[Mod_id].Info[0].T304_active == 0 )      && 
		    (UE_rrc_inst[Mod_id].HandoverInfoUe.measFlag == 1)) {
		  //trigger measurement reporting procedure (36.331, section 5.5.5)
		  if (UE_rrc_inst[Mod_id].measReportList[i][j] == NULL) {
		    UE_rrc_inst[Mod_id].measReportList[i][j] = malloc(sizeof(MEAS_REPORT_LIST));
		  }
		  UE_rrc_inst[Mod_id].measReportList[i][j]->measId = UE_rrc_inst[Mod_id].MeasId[i][j]->measId;
		  UE_rrc_inst[Mod_id].measReportList[i][j]->numberOfReportsSent = 0;
		  rrc_ue_generate_MeasurementReport(Mod_id,frame,eNB_index);
		  UE_rrc_inst[Mod_id].HandoverInfoUe.measFlag = 1;
		  LOG_I(RRC,"[UE %d] Frame %d: A3 event detected, state: %d \n", Mod_id, frame, UE_rrc_inst[Mod_id].Info[0].State);
		}
		else {
		  if(UE_rrc_inst[Mod_id].measReportList[i][j] != NULL){
		    free(UE_rrc_inst[Mod_id].measReportList[i][j]);
		  }
		  UE_rrc_inst[Mod_id].measReportList[i][j] = NULL;
		}
		break;
	      case ReportConfigEUTRA__triggerType__event__eventId_PR_eventA4:
		LOG_D(RRC,"[UE %d] Frame %d : received an A4 event, neighbor becomes offset better than a threshold\n",Mod_id, frame);
		break;
	      case ReportConfigEUTRA__triggerType__event__eventId_PR_eventA5:
		LOG_D(RRC,"[UE %d] Frame %d: received an A5 event, serving becomes worse than threshold 1 and neighbor becomes better than threshold 2\n",Mod_id, frame);
		break;
	      default:
		LOG_D(RRC,"Invalid ReportConfigEUTRA__triggerType__event__eventId: %d", 
		      UE_rrc_inst[Mod_id].ReportConfig[i][j]->reportConfig.choice.reportConfigEUTRA.triggerType.present);
		break;
	      }
	    }
	  }
	}
      }
    }
  }
}

//check_trigger_meas_event(Mod_id, frame, eNB_index, i,j,ofn,ocn,hys,ofs,ocs,a3_offset,ttt_ms)
u8 check_trigger_meas_event(u8 Mod_id,u32 frame, u8 eNB_index, u8 ue_cnx_index, u8 meas_index, 
			    Q_OffsetRange_t ofn, Q_OffsetRange_t ocn, Hysteresis_t hys, 
			    Q_OffsetRange_t ofs, Q_OffsetRange_t ocs, long a3_offset, TimeToTrigger_t ttt) {
  u8 eNB_offset;
  u8 currentCellIndex = mac_xface->lte_frame_parms->Nid_cell;
  
  LOG_I(RRC,"ofn(%d) ocn(%d) hys(%d) ofs(%d) ocs(%d) a3_offset(%d) ttt(%d) rssi %3.1f\n", \
	ofn,ocn,hys,ofs,ocs,a3_offset,ttt,10*log10(mac_xface->get_RSSI(Mod_id))-mac_xface->get_rx_total_gain_dB(Mod_id));
  
  //  for (eNB_offset = 0;(eNB_offset<1+mac_xface->get_n_adj_cells(Mod_id))&& (eNB_offset!=eNB_index);eNB_offset++) {
  for (eNB_offset = 1;(eNB_offset<1+mac_xface->get_n_adj_cells(Mod_id));eNB_offset++) {
    /* RHS: Verify that idx 0 corresponds to currentCellIndex in rsrp array */
    if(UE_rrc_inst[Mod_id].rsrp_db_filtered[eNB_offset]+ofn+ocn-hys > UE_rrc_inst[Mod_id].rsrp_db_filtered[0/*eNB_index*/]+ofs+ocs - 1 /*+a3_offset*/) {
      UE_rrc_inst->measTimer[ue_cnx_index][meas_index][eNB_offset-1] += 2; //Called every subframe = 2ms
      LOG_D(RRC,"[UE %d] Frame %d: Entry measTimer[%d][%d]: %d currentCell: %d betterCell: %d \n", 
	    Mod_id, frame, ue_cnx_index,meas_index,UE_rrc_inst->measTimer[ue_cnx_index][meas_index][eNB_offset-1],currentCellIndex,eNB_offset);
    }
    else {
      UE_rrc_inst->measTimer[ue_cnx_index][meas_index][eNB_offset-1] = 0; //Exit condition: Resetting the measurement timer
      LOG_D(RRC,"[UE %d] Frame %d: Exit measTimer[%d][%d]: %d currentCell: %d betterCell: %d \n", 
	    Mod_id, frame, ue_cnx_index,meas_index,UE_rrc_inst->measTimer[ue_cnx_index][meas_index][eNB_offset-1],currentCellIndex,eNB_offset);
    }
    if (UE_rrc_inst->measTimer[ue_cnx_index][meas_index][eNB_offset-1] >= ttt) {
      UE_rrc_inst->HandoverInfoUe.targetCellId = get_adjacent_cell_id(Mod_id,eNB_offset-1); //check this!
      LOG_D(RRC,"[UE %d] Frame %d eNB %d: Handover triggered: targetCellId: %d currentCellId: %d eNB_offset: %d rsrp source: %3.1f rsrp target: %3.1f\n", \
	    Mod_id, frame, eNB_index, 
	    UE_rrc_inst->HandoverInfoUe.targetCellId,ue_cnx_index,eNB_offset,
	    (dB_fixed_times10(UE_rrc_inst[Mod_id].rsrp_db[0])/10.0)-mac_xface->get_rx_total_gain_dB(Mod_id)-dB_fixed(mac_xface->lte_frame_parms->N_RB_DL*12),
	    (dB_fixed_times10(UE_rrc_inst[Mod_id].rsrp_db[eNB_offset])/10.0)-mac_xface->get_rx_total_gain_dB(Mod_id)-dB_fixed(mac_xface->lte_frame_parms->N_RB_DL*12));
      UE_rrc_inst->Info[0].handoverTarget = eNB_offset;
      return 1;
    }
  }
  return 0;
}

#ifdef Rel10
int decode_MCCH_Message(u8 Mod_id, u32 frame, u8 eNB_index, u8 *Sdu, u8 Sdu_len,u8 mbsfn_sync_area) {
  
  MCCH_Message_t *mcch=NULL;
  MBSFNAreaConfiguration_r9_t **mcch_message=&UE_rrc_inst[Mod_id].mcch_message[eNB_index];
  asn_dec_rval_t dec_rval;
  
  if (UE_rrc_inst[Mod_id].Info[eNB_index].MCCHStatus[mbsfn_sync_area] == 1) {
    LOG_D(RRC,"[UE %d] Frame %d: MCCH MESSAGE for MBSFN sync area %d has been already received!\n",
	  Mod_id, frame, mbsfn_sync_area);
    return 0; // avoid decoding to prevent memory bloating
  }
  else {
    dec_rval = uper_decode_complete(NULL,
				     &asn_DEF_MCCH_Message,
				     (void **)&mcch,
				     (const void *)Sdu,
				     Sdu_len);
    if ((dec_rval.code != RC_OK) && (dec_rval.consumed==0)) {
      LOG_E(RRC,"[UE %d] Failed to decode MCCH__MESSAGE (%d bits)\n",Mod_id,dec_rval.consumed);
      //free the memory
      SEQUENCE_free(&asn_DEF_MCCH_Message, (void*)mcch, 1);
      return -1;
    }
#ifdef XER_PRINT
    xer_fprint(stdout, &asn_DEF_MCCH_Message, (void*)mcch);
#endif

    if (mcch->message.present == MCCH_MessageType_PR_c1) {
      LOG_D(RRC,"[UE %d] Found mcch message \n",Mod_id);
      if(mcch->message.choice.c1.present == MCCH_MessageType__c1_PR_mbsfnAreaConfiguration_r9) {
	/*	
	memcpy((void*)*mcch_message,
	       (void*)&mcch->message.choice.c1.choice.mbsfnAreaConfiguration_r9,
	       sizeof(MBSFNAreaConfiguration_r9_t)); */
	*mcch_message = &mcch->message.choice.c1.choice.mbsfnAreaConfiguration_r9;
	LOG_I(RRC,"[UE %d] Frame %d : Found MBSFNAreaConfiguration from eNB %d \n",Mod_id, frame, eNB_index);
	decode_MBSFNAreaConfiguration(Mod_id,eNB_index,frame, mbsfn_sync_area);

      }
    }
  }
  return 0;
}

void decode_MBSFNAreaConfiguration(u8 Mod_id, u8 eNB_index, u32 frame,u8 mbsfn_sync_area) {
  LOG_D(RRC,"[UE %d] Frame %d : Number of MCH(s) in the MBSFN Sync Area %d  is %d\n", 
	Mod_id, frame, mbsfn_sync_area, UE_rrc_inst[Mod_id].mcch_message[eNB_index]->pmch_InfoList_r9.list.count);
  //  store to MAC/PHY necessary parameters for receiving MTCHs
  rrc_mac_config_req(Mod_id,0,0,eNB_index,
		     (RadioResourceConfigCommonSIB_t *)NULL,
		     (struct PhysicalConfigDedicated *)NULL,
		     (MeasObjectToAddMod_t **)NULL,
		     (MAC_MainConfig_t *)NULL,
		     0,
		     (struct LogicalChannelConfig *)NULL,
		     (MeasGapConfig_t *)NULL,
		     (TDD_Config_t *)NULL,
		     (MobilityControlInfo_t *)NULL,
		     NULL,
		     NULL,
		     NULL,
		     NULL,
		     NULL,
		     (MBSFN_SubframeConfigList_t *)NULL
#ifdef Rel10	       
		     ,
		     0,
		     (MBSFN_AreaInfoList_r9_t *)NULL,
		     &UE_rrc_inst[Mod_id].mcch_message[eNB_index]->pmch_InfoList_r9
#endif
#ifdef CBA
		     ,
		     0,
		     0
#endif
		     );

  UE_rrc_inst[Mod_id].Info[eNB_index].MCCHStatus[mbsfn_sync_area] = 1;
  
  // Config Radio Bearer for MBMS user data (similar way to configure for eNB side in init_MBMS function)
  rrc_pdcp_config_asn1_req(NB_eNB_INST+Mod_id,frame,0,eNB_index,
			   NULL, // SRB_ToAddModList
			   NULL, // DRB_ToAddModList
			   (DRB_ToReleaseList_t*)NULL,
			   0, // security mode
			   NULL, // key rrc encryption
			   NULL, // key rrc integrity
			   NULL // key encryption
#ifdef Rel10
			   ,&(UE_rrc_inst[Mod_id].mcch_message[eNB_index]->pmch_InfoList_r9)
#endif
			   );
    
  rrc_rlc_config_asn1_req(NB_eNB_INST+Mod_id, frame,0,eNB_index,
			  NULL,// SRB_ToAddModList
			  NULL,// DRB_ToAddModList
			  NULL,// DRB_ToReleaseList
#ifdef Rel10
			  &(UE_rrc_inst[Mod_id].mcch_message[eNB_index]->pmch_InfoList_r9)
#endif 
			  );
  // */
  
}

#endif // rel10

#ifndef USER_MODE
EXPORT_SYMBOL(Rlc_info_am_config);
#endif

#if defined(ENABLE_ITTI)
void *rrc_ue_task(void *args_p) {
  MessageDef *msg_p;
  const char *msg_name;
  instance_t instance;
  unsigned int Mod_id;
  SRB_INFO *srb_info_p;

  itti_mark_task_ready (TASK_RRC_UE);

  while(1) {
    // Wait for a message
    itti_receive_msg (TASK_RRC_UE, &msg_p);

    msg_name = ITTI_MSG_NAME (msg_p);
    instance = ITTI_MSG_INSTANCE (msg_p);
    Mod_id = instance - NB_eNB_INST;

    switch (ITTI_MSG_ID(msg_p)) {
      case TERMINATE_MESSAGE:
        itti_exit_task ();
        break;

      case MESSAGE_TEST:
        LOG_I(RRC, "[UE %d] Received %s\n", Mod_id, msg_name);
        break;

      /* MAC messages */
      case RRC_MAC_IN_SYNC_IND:
        LOG_I(RRC, "[UE %d] Received %s: frame %d, eNB %d\n", Mod_id, msg_name,
              RRC_MAC_IN_SYNC_IND (msg_p).frame, RRC_MAC_IN_SYNC_IND (msg_p).enb_index);

        UE_rrc_inst[Mod_id].Info[RRC_MAC_IN_SYNC_IND (msg_p).enb_index].N310_cnt = 0;
        if (UE_rrc_inst[Mod_id].Info[RRC_MAC_IN_SYNC_IND (msg_p).enb_index].T310_active == 1)
          UE_rrc_inst[Mod_id].Info[RRC_MAC_IN_SYNC_IND (msg_p).enb_index].N311_cnt++;
        break;

      case RRC_MAC_OUT_OF_SYNC_IND:
        LOG_I(RRC, "[UE %d] Received %s: frame %d, eNB %d\n", Mod_id, msg_name,
              RRC_MAC_OUT_OF_SYNC_IND (msg_p).frame, RRC_MAC_OUT_OF_SYNC_IND (msg_p).enb_index);

        UE_rrc_inst[Mod_id].Info[RRC_MAC_OUT_OF_SYNC_IND (msg_p).enb_index].N310_cnt ++;
        break;

      case RRC_MAC_BCCH_DATA_IND:
        LOG_I(RRC, "[UE %d] Received %s: frame %d, eNB %d\n", Mod_id, msg_name,
              RRC_MAC_BCCH_DATA_IND (msg_p).frame, RRC_MAC_BCCH_DATA_IND (msg_p).enb_index);

        decode_BCCH_DLSCH_Message (Mod_id, RRC_MAC_BCCH_DATA_IND (msg_p).frame,
                                   RRC_MAC_BCCH_DATA_IND (msg_p).enb_index, RRC_MAC_BCCH_DATA_IND (msg_p).sdu,
                                   RRC_MAC_BCCH_DATA_IND (msg_p).sdu_size);
        break;

      case RRC_MAC_CCCH_DATA_CNF:
        LOG_I(RRC, "[UE %d] Received %s: eNB %d\n", Mod_id, msg_name,
              RRC_MAC_CCCH_DATA_CNF (msg_p).enb_index);

        // reset the tx buffer to indicate RRC that ccch was successfully transmitted (for example if contention resolution succeeds)
        UE_rrc_inst[Mod_id].Srb0[RRC_MAC_CCCH_DATA_CNF (msg_p).enb_index].Tx_buffer.payload_size = 0;
        break;

      case RRC_MAC_CCCH_DATA_IND:
        LOG_I(RRC, "[UE %d] Received %s: frame %d, eNB %d\n", Mod_id, msg_name,
              RRC_MAC_CCCH_DATA_IND (msg_p).frame, RRC_MAC_CCCH_DATA_IND (msg_p).enb_index);

        srb_info_p = &UE_rrc_inst[Mod_id].Srb0[RRC_MAC_CCCH_DATA_IND (msg_p).enb_index];

        memcpy (srb_info_p->Rx_buffer.Payload, RRC_MAC_CCCH_DATA_IND (msg_p).sdu,
                RRC_MAC_CCCH_DATA_IND (msg_p).sdu_size);
        srb_info_p->Rx_buffer.payload_size = RRC_MAC_CCCH_DATA_IND (msg_p).sdu_size;
        rrc_ue_decode_ccch (Mod_id, RRC_MAC_CCCH_DATA_IND (msg_p).frame, srb_info_p,
                            RRC_MAC_CCCH_DATA_IND (msg_p).enb_index);
        break;

#ifdef Rel10
      case RRC_MAC_MCCH_DATA_IND:
        LOG_I(RRC, "[UE %d] Received %s: frame %d, eNB %d, mbsfn SA %d\n", Mod_id, msg_name,
              RRC_MAC_MCCH_DATA_IND (msg_p).frame, RRC_MAC_MCCH_DATA_IND (msg_p).enb_index, RRC_MAC_MCCH_DATA_IND (msg_p).mbsfn_sync_area);

        decode_MCCH_Message (Mod_id, RRC_MAC_MCCH_DATA_IND (msg_p).frame, RRC_MAC_MCCH_DATA_IND (msg_p).enb_index,
                             RRC_MAC_MCCH_DATA_IND (msg_p).sdu, RRC_MAC_MCCH_DATA_IND (msg_p).sdu_size,
                             RRC_MAC_MCCH_DATA_IND (msg_p).mbsfn_sync_area);
        break;
#endif

        /* PDCP messages */
      case RRC_DCCH_DATA_IND:
        LOG_I(RRC, "[UE %d] Received %s: frame %d, DCCH %d, UE %d\n", Mod_id, msg_name,
              RRC_DCCH_DATA_IND (msg_p).frame, RRC_DCCH_DATA_IND (msg_p).dcch_index, RRC_DCCH_DATA_IND (msg_p).ue_index);

        rrc_ue_decode_dcch (Mod_id, RRC_DCCH_DATA_IND (msg_p).frame,
                            RRC_DCCH_DATA_IND (msg_p).dcch_index, RRC_DCCH_DATA_IND (msg_p).sdu_p,
                            RRC_DCCH_DATA_IND (msg_p).ue_index);

        // Message buffer has been processed, free it now.
        free (RRC_DCCH_DATA_IND (msg_p).sdu_p);
        break;

      /* NAS messages */
      case NAS_UPLINK_DATA_REQ:
      {
        uint32_t length;
        uint8_t *buffer;

        LOG_I(RRC, "[UE %d] Received %s: UEid %d\n", Mod_id, msg_name, NAS_UPLINK_DATA_REQ (msg_p).UEid);

        /* Create message for PDCP (ULInformationTransfer_t) */
        length = do_ULInformationTransfer(&buffer, NAS_UPLINK_DATA_REQ (msg_p).nasMsg.length, NAS_UPLINK_DATA_REQ (msg_p).nasMsg.data);

        /* Transfer data to PDCP */
        pdcp_rrc_data_req (instance, 0 /* TODO put frame number ! */, 0, DCCH, rrc_mui++, 0, length, buffer, 1);
        break;
      }

      default:
        LOG_E(RRC, "[UE %d] Received unexpected message %s\n", Mod_id, msg_name);
        break;
    }

    free (msg_p);
    msg_p = NULL;
  }
}
#endif