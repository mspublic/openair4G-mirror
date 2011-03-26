/*________________________openair_rrc_main.c________________________

  Authors : Hicham Anouar, Raymond Knopp
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
extern CH_MAC_INST *CH_mac_inst;
extern UE_MAC_INST *UE_mac_inst;
#ifdef BIGPHYSAREA
extern void *bigphys_malloc(int);
#endif

extern inline unsigned int taus(void);
/*------------------------------------------------------------------------------*/
void rrc_rx_tx(u8 Mod_id){
  /*------------------------------------------------------------------------------*/

  unsigned char i;
#ifdef DEBUG_RRC
  msg("rrc_rx_tx: Frame %d Mod_id %d (CH status %d),  CCCH TX buffer %d\n",Mac_rlc_xface->frame,Mod_id,Mac_rlc_xface->Is_cluster_head[Mod_id],CH_rrc_inst[0].Srb0.Tx_buffer.W_idx);
#endif DEBUG_RRC

  Rrc_xface->Frame_index=Mac_rlc_xface->frame;
  if(Mac_rlc_xface->Is_cluster_head[Mod_id] == 1){

  }
  else{

  }
}

/*------------------------------------------------------------------------------*/
int rrc_init_global_param(void){
  /*------------------------------------------------------------------------------*/

  //#ifdef USER_MODE
  Rrc_xface = (RRC_XFACE*)malloc16(sizeof(RRC_XFACE));
  //#endif //USRE_MODE
 
  Rrc_xface->openair_rrc_top_init = openair_rrc_top_init;
  Rrc_xface->openair_rrc_ch_init = openair_rrc_ch_init;
  Rrc_xface->openair_rrc_mr_init = openair_rrc_mr_init;
  Rrc_xface->mac_rrc_data_ind=mac_rrc_data_ind;
  Rrc_xface->mac_rrc_data_req=mac_rrc_data_req;
  Rrc_xface->rrc_data_indP=rlcrrc_data_ind;
  Rrc_xface->rrc_rx_tx=rrc_rx_tx;
  Rrc_xface->mac_rrc_meas_ind=mac_rrc_meas_ind;

  Mac_rlc_xface->mac_out_of_sync_ind=mac_out_of_sync_ind;

#ifndef NO_RRM
  Rrc_xface->fn_rrc=fn_rrc;
#endif
  msg("[RRC]INIT_GLOBAL_PARAM: Mac_rlc_xface %p, rrc_rlc_register %p,rlcrrc_data_ind %p\n",Mac_rlc_xface,Mac_rlc_xface->rrc_rlc_register_rrc,rlcrrc_data_ind);

  if(Mac_rlc_xface==NULL || Mac_rlc_xface->rrc_rlc_register_rrc==NULL||rlcrrc_data_ind==NULL)
    return -1;
  Mac_rlc_xface->rrc_rlc_register_rrc(rlcrrc_data_ind ,NULL); //register with rlc


  BCCH_LCHAN_DESC.transport_block_size=BCCH_PAYLOAD_SIZE_MAX;
  BCCH_LCHAN_DESC.max_transport_blocks=16;
  CCCH_LCHAN_DESC.transport_block_size=CCCH_PAYLOAD_SIZE_MAX;
  CCCH_LCHAN_DESC.max_transport_blocks=256;
  DCCH_LCHAN_DESC.transport_block_size=4;
  DCCH_LCHAN_DESC.max_transport_blocks=16;
  DCCH_LCHAN_DESC.Delay_class=1;
  DTCH_DL_LCHAN_DESC.transport_block_size=52;
  DTCH_DL_LCHAN_DESC.max_transport_blocks=20;
  DTCH_DL_LCHAN_DESC.Delay_class=1;
  DTCH_UL_LCHAN_DESC.transport_block_size=52;
  DTCH_UL_LCHAN_DESC.max_transport_blocks=20;
  DTCH_UL_LCHAN_DESC.Delay_class=1;

  Rlc_info_um.rlc_mode=RLC_UM;
  Rlc_info_um.rlc.rlc_um_info.timer_discard=0;
  Rlc_info_um.rlc.rlc_um_info.sdu_discard_mode=16;

  Rlc_info_am_config.rlc_mode=RLC_AM;
  Rlc_info_am_config.rlc.rlc_am_info.sdu_discard_mode      = SDU_DISCARD_MODE_RESET;//SDU_DISCARD_MODE_MAX_DAT_RETRANSMISSION;//
  Rlc_info_am_config.rlc.rlc_am_info.timer_poll            = 0;
  Rlc_info_am_config.rlc.rlc_am_info.timer_poll_prohibit   = 0;
  Rlc_info_am_config.rlc.rlc_am_info.timer_discard         = 500;
  Rlc_info_am_config.rlc.rlc_am_info.timer_poll_periodic   = 0;
  Rlc_info_am_config.rlc.rlc_am_info.timer_status_prohibit = 250;
  Rlc_info_am_config.rlc.rlc_am_info.timer_status_periodic = 500;
  Rlc_info_am_config.rlc.rlc_am_info.timer_rst             = 250;//250
  Rlc_info_am_config.rlc.rlc_am_info.max_rst               = 500;//500
  Rlc_info_am_config.rlc.rlc_am_info.timer_mrw             = 0;
  Rlc_info_am_config.rlc.rlc_am_info.pdu_size              = 32;//416; // in bits
  //Rlc_info_am.rlc.rlc_am_info.in_sequence_delivery  = 1;//boolean
  Rlc_info_am_config.rlc.rlc_am_info.max_dat               = 32;//127;
  Rlc_info_am_config.rlc.rlc_am_info.poll_pdu              = 0;
  Rlc_info_am_config.rlc.rlc_am_info.poll_sdu              = 0;//256;/
  Rlc_info_am_config.rlc.rlc_am_info.poll_window           = 80;//128
  Rlc_info_am_config.rlc.rlc_am_info.tx_window_size        = 512;
  Rlc_info_am_config.rlc.rlc_am_info.rx_window_size        = 512;
  Rlc_info_am_config.rlc.rlc_am_info.max_mrw               = 8;
  Rlc_info_am_config.rlc.rlc_am_info.last_transmission_pdu_poll_trigger   = 1;//boolean
  Rlc_info_am_config.rlc.rlc_am_info.last_retransmission_pdu_poll_trigger = 1;//boolean
  Rlc_info_am_config.rlc.rlc_am_info.send_mrw              = 1;//boolean*

#ifndef NO_RRM
  if(L3_xface_init())
    return(-1);
#endif

  return 0;
}


#ifndef NO_RRM
/*------------------------------------------------------------------------------*/
int L3_xface_init(void){
/*------------------------------------------------------------------------------*/

  int ret = 0;

#ifdef USER_MODE

  int sock ;
  msg("[RRC][L3_XFACE] init de l'interface \n");

  if(open_socket(&S_rrc, RRC_RRM_SOCK_PATH, RRM_RRC_SOCK_PATH,0)==-1)
    return (-1);

  if (S_rrc.s  == -1)
    {
      return (-1);
    }

  socket_setnonblocking(S_rrc.s);
  msg("Interface Connected... RRM-RRC\n");
  return 0 ;

#else

  ret=rtf_create(RRC2RRM_FIFO,32768);

  if (ret < 0) {
    msg("[openair][MAC][INIT] Cannot create RRC2RRM fifo %d (ERROR %d)\n",RRC2RRM_FIFO,ret);

    return(-1);
  }
  else{
    msg("[openair][MAC][INIT] Created RRC2RRM fifo %d\n",RRC2RRM_FIFO);
    rtf_reset(RRC2RRM_FIFO);
  }

  ret=rtf_create(RRM2RRC_FIFO,32768);

  if (ret < 0) {
    msg("[openair][MAC][INIT] Cannot create RRM2RRC fifo %d (ERROR %d)\n",RRM2RRC_FIFO,ret);

    return(-1);
  }
  else{
    msg("[openair][MAC][INIT] Created RRC2RRM fifo %d\n",RRM2RRC_FIFO);
    rtf_reset(RRM2RRC_FIFO);
  }

  return(0);

#endif
}
#endif


/*------------------------------------------------------------------------------*/
void openair_rrc_top_init(void){
  /*-----------------------------------------------------------------------------*/

  Rrc_xface->Frame_index=Mac_rlc_xface->frame;

  msg("[OPENAIR][RRC INIT] Init function start:Nb_INST=%d, NB_UE_INST=%d, NB_CH_INST=%d\n",NB_INST,NB_UE_INST,NB_CH_INST);
  msg("[OPENAIR][RRC INIT] Init function start:Nb_INST=%d\n",NB_INST);

  UE_rrc_inst = (UE_RRC_INST*)malloc16(NB_UE_INST*sizeof(UE_RRC_INST));

  msg("ALLOCATE %d Bytes for UE_RRC_INST @ %p\n",(unsigned int)(NB_UE_INST*sizeof(UE_RRC_INST)),UE_rrc_inst);

  CH_rrc_inst = (CH_RRC_INST*)malloc16(NB_CH_INST*sizeof(CH_RRC_INST));
  memset(CH_rrc_inst,0,NB_CH_INST*sizeof(CH_RRC_INST));
  msg("ALLOCATE %d Bytes for CH_RRC_INST @ %p\n",(unsigned int)(NB_CH_INST*sizeof(CH_RRC_INST)),CH_rrc_inst);

#ifndef NO_RRM
#ifndef USER_MODE

  Header_buf=(char*)malloc16(sizeof(msg_head_t));
  Data=(char*)malloc16(2400);
  Header_read_idx=0;
  Data_read_idx=0;
  Header_size=sizeof(msg_head_t);

#endif //NO_RRM
Data_to_read=0;
#endif //USER_MODE

}

void init_SI(u8 Mod_id) {

  u8 SIwindowsize=1;
  u16 SIperiod=8;

  CH_rrc_inst[Mod_id].sizeof_SIB1 = 0;  
  CH_rrc_inst[Mod_id].sizeof_SIB23 = 0;

  CH_rrc_inst[Mod_id].SIB1 = (u8 *)malloc16(32);
  
  if (CH_rrc_inst[Mod_id].SIB1)
    CH_rrc_inst[Mod_id].sizeof_SIB1 = do_SIB1(CH_rrc_inst[Mod_id].SIB1,
					      &CH_rrc_inst[Mod_id].sib1);
  
  CH_rrc_inst[Mod_id].SIB23 = (u8 *)malloc16(64);
  if (CH_rrc_inst[Mod_id].SIB23)
    CH_rrc_inst[Mod_id].sizeof_SIB23 = do_SIB23(CH_rrc_inst[Mod_id].SIB23,
						&CH_rrc_inst[Mod_id].systemInformation,
						&CH_rrc_inst[Mod_id].sib2,
						&CH_rrc_inst[Mod_id].sib3);

    Mac_rlc_xface->rrc_mac_config_req(Mod_id,1,0,0,
				      &CH_rrc_inst[Mod_id].sib2->radioResourceConfigCommon,
				      (struct PhysicalConfigDedicated_t *)NULL,
				      CH_rrc_inst[Mod_id].sib1.tdd_Config,
				      &SIwindowsize,
				      &SIperiod);
}

void init_SI_UE(u8 Mod_id,u8 CH_index) {

  UE_rrc_inst[Mod_id].sizeof_SIB1[CH_index] = 0;  
  UE_rrc_inst[Mod_id].sizeof_SI[CH_index] = 0;

  UE_rrc_inst[Mod_id].SIB1[CH_index] = (u8 *)malloc16(32);
  
  UE_rrc_inst[Mod_id].SI[CH_index] = (u8 *)malloc16(64);

  UE_rrc_inst[Mod_id].Info[CH_index].SIB1Status = 0;
  UE_rrc_inst[Mod_id].Info[CH_index].SIStatus = 0;


}

/*------------------------------------------------------------------------------*/
char openair_rrc_ch_init(u8 Mod_id){
  /*-----------------------------------------------------------------------------*/

  unsigned char i,j,k;
  CH_rrc_inst[Mod_id].Node_id=Mac_rlc_xface->Node_id[Mod_id];
  msg("[OPENAIR][RRC][INIT CH] Mod_id:%d, Node_id=%d\n",Mod_id,CH_rrc_inst[Mod_id].Node_id);
  CH_rrc_inst[Mod_id].Info.Status = CH_READY;
  CH_rrc_inst[Mod_id].Info.Nb_ue=0;

  
  CH_rrc_inst[Mod_id].Srb0.Active=0;

  for(j=0;j<(NB_CNX_CH+1);j++){
    CH_rrc_inst[Mod_id].Srb2[j].Active=0;
  }

  /// System Information INIT
  init_SI(Mod_id);

  msg("[OPENAIR][RRC][INIT] INIT OK for Mod_id:%d, Node_id=%d\n",Mod_id,CH_rrc_inst[Mod_id].Node_id);

#ifdef NO_RRM //init ch SRB0, SRB1 & BDTCH
  openair_rrc_on(Mod_id);
#else
  CH_rrc_inst[Mod_id].Last_scan_req=0;
   send_msg(&S_rrc,msg_rrc_phy_synch_to_MR_ind(Mod_id,CH_rrc_inst[Mod_id].Mac_id));
#endif
   msg("\nRRC: INIT CH %d Successful \n\n",NODE_ID[Mod_id]);


  return 0;

}

/*------------------------------------------------------------------------------*/
char openair_rrc_mr_init(u8 Mod_id, unsigned char CH_index){
  /*-----------------------------------------------------------------------------*/

  unsigned char i,j,k; 

  UE_rrc_inst[Mod_id-NB_CH_INST].Node_id=Mac_rlc_xface->Node_id[Mod_id];
  Mod_id-=NB_CH_INST;
  msg("[OPENAIR][RRC][INIT] Mod_id:%d, Node_id=%d\n",Mod_id,UE_rrc_inst[Mod_id].Node_id);
  
  UE_rrc_inst[Mod_id].Info[CH_index].Status=RRC_IDLE;
  UE_rrc_inst[Mod_id].Info[CH_index].Rach_tx_cnt=0;
  UE_rrc_inst[Mod_id].Info[CH_index].Nb_bcch_wait=0;
  UE_rrc_inst[Mod_id].Info[CH_index].UE_index=0xffff;
  UE_rrc_inst[Mod_id].Srb0[CH_index].Active=0; 
  UE_rrc_inst[Mod_id].Srb1[CH_index].Active=0;
  UE_rrc_inst[Mod_id].Srb2[CH_index].Active=0;

  init_SI_UE(Mod_id,CH_index);
  msg("[UE][RRC] INIT: phy_sync_2_ch_ind from Inst %d\n", Mod_id,Mod_id+NB_CH_INST);
  
#ifndef NO_RRM
  send_msg(&S_rrc,msg_rrc_phy_synch_to_CH_ind(Mod_id+NB_CH_INST,CH_index,UE_rrc_inst[Mod_id].Mac_id));
#endif

#ifdef NO_RRM //init ch SRB0, SRB1 & BDTCH
  openair_rrc_on(Mod_id+NB_CH_INST);
#endif
  msg("[OPENAIR][RRC][INIT] Init OK for Mod_id:%d, Node_id=%d\n",Mod_id,UE_rrc_inst[Mod_id].Node_id);
  
  return 0;
}

//------------------------------------------------------------------------------------------------//
void rrc_config_buffer(SRB_INFO *Srb_info, u8 Lchan_type, u8 Role){//role: 0 CH, 1 UE
  //--------------------------------------------------------------------------------------------------//

  Srb_info->Rx_buffer.R_idx = 0;
  Srb_info->Rx_buffer.W_idx = 0;
  Srb_info->Rx_buffer.Tb_size = Srb_info->Lchan_desc[0].transport_block_size;
  Srb_info->Rx_buffer.Nb_tb_max = Srb_info->Lchan_desc[0].max_transport_blocks;

  Srb_info->Tx_buffer.R_idx = 0;
  Srb_info->Tx_buffer.W_idx = 0;
  Srb_info->Tx_buffer.Tb_size = Srb_info->Lchan_desc[1].transport_block_size;
  Srb_info->Tx_buffer.Nb_tb_max = (Srb_info->Lchan_desc[1].max_transport_blocks);
}

#ifdef NO_RRM
/*------------------------------------------------------------------------------*/
void openair_rrc_on(u8 Mod_id){//configure  BCCH & CCCH Logical Channels and associated rrc_buffers,
  //configure associated SRBs
  /*------------------------------------------------------------------------------*/

  unsigned short i,Role,Nb_sig;
  u16 Index;

  msg("OPENAIR RRC IN....\n");


  if( Mac_rlc_xface->Is_cluster_head[Mod_id] == 1){

    for(i=0;i<NB_SIG_CNX_CH;i++){

      memcpy(&CH_rrc_inst[Mod_id].Srb3.Lchan_desc[0],&BCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
      memcpy(&CH_rrc_inst[Mod_id].Srb3.Lchan_desc[1],&BCCH_LCHAN_DESC,LCHAN_DESC_SIZE);

      rrc_config_buffer(&CH_rrc_inst[Mod_id].Srb3,BCCH,1);

      msg("[OPENAIR][RRC][RRC_ON] NODE %d, Config BCCH for TB_size %d\n",NODE_ID[Mod_id],
	  CH_rrc_inst[Mod_id].Srb3.Lchan_desc[0].transport_block_size);
      CH_rrc_inst[Mod_id].Srb3.Active=1;




      memcpy(&CH_rrc_inst[Mod_id].Srb0.Lchan_desc[0],&CCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
      memcpy(&CH_rrc_inst[Mod_id].Srb0.Lchan_desc[1],&CCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
      rrc_config_buffer(&CH_rrc_inst[Mod_id].Srb0,CCCH,1);

      msg("[OPENAIR][RRC][RRC_ON] NODE %d, Config CCCH %d done, TB_size=%d,%d\n",NODE_ID[Mod_id],Index,
	     CH_rrc_inst[Mod_id].Srb0.Tx_buffer.Tb_size,CH_rrc_inst[Mod_id].Srb0.Rx_buffer.Tb_size);
      CH_rrc_inst[Mod_id].Srb0.Active=1;


    }
  }

  else{

    Mod_id-=NB_CH_INST;
    for(i=0;i<NB_SIG_CNX_UE;i++){

      msg("[RRC][UE %d] Activating CCCH (eNB %d)\n",Mod_id,i);
      UE_rrc_inst[Mod_id].Srb0[i].Srb_id = CCCH;
      memcpy(&UE_rrc_inst[Mod_id].Srb0[i].Lchan_desc[0],&CCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
      memcpy(&UE_rrc_inst[Mod_id].Srb0[i].Lchan_desc[1],&CCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
      rrc_config_buffer(&UE_rrc_inst[Mod_id].Srb0[i],CCCH,1);

      UE_rrc_inst[Mod_id].Srb0[i].Active=1;

    }
  }


}
#endif //NO_RRM


/*------------------------------------------------------------------------------*/
void rrc_ue_generate_RRCConnectionRequest(u8 Mod_id, u8 CH_index){
  /*------------------------------------------------------------------------------*/

  u8 i=0,rv[6];

  if(UE_rrc_inst[Mod_id].Srb0[CH_index].Tx_buffer.W_idx ==0){

    // Get RRCConnectionRequest, fill random for now


    // Generate random byte stream for contention resolution
    for (i=0;i<6;i++)
      rv[i]=taus()&0xff;

    UE_rrc_inst[Mod_id].Srb0[CH_index].Tx_buffer.W_idx = do_RRCConnectionRequest(UE_rrc_inst[Mod_id].Srb0[CH_index].Tx_buffer.Payload,rv);

    /*
      UE_rrc_inst[Mod_id].Srb0[Idx].Tx_buffer.Payload[i] = taus()&0xff;

    UE_rrc_inst[Mod_id].Srb0[Idx].Tx_buffer.W_idx =i;
    */

  }
}


mui_t rrc_mui=0;


/*------------------------------------------------------------------------------*/
void rrc_ue_generate_RRCConnectionSetupComplete(u8 Mod_id, u8 CH_index){
  /*------------------------------------------------------------------------------*/

  u8 buffer[32];
  u8 size;

  msg("[RRC][UE %d] Frame %d : Generating RRCConnectionSetupComplete\n",Mod_id,Mac_rlc_xface->frame);

  size = do_RRCConnectionSetupComplete(buffer);

  Mac_rlc_xface->rrc_rlc_data_req(Mod_id+NB_CH_INST,DCCH,rrc_mui++,0,size,(char*)buffer);

}



void rrc_ue_generate_RRCConnectionReconfigurationComplete(u8 Mod_id,u8 CH_index) {

  u8 buffer[32], size,i;

  msg("[RRC][UE %d] Frame %d : Generating RRCConnectionReconfigurationComplete\n",Mod_id,Mac_rlc_xface->frame);

  size = do_RRCConnectionReconfigurationComplete(buffer);

  Mac_rlc_xface->rrc_rlc_data_req(Mod_id+NB_CH_INST,DCCH,rrc_mui++,0,size,(char*)buffer);
}


/*------------------------------------------------------------------------------*/
void rrc_ue_decode_ccch(u8 Mod_id, SRB_INFO *Srb_info, u8 CH_index){
  /*------------------------------------------------------------------------------*/

  unsigned char ret;
  DL_CCCH_Message_t dlccchmsg;
  DL_CCCH_Message_t *dl_ccch_msg=&dlccchmsg;
  asn_dec_rval_t dec_rval;
  int i;

  memset(dl_ccch_msg,0,sizeof(DL_CCCH_Message_t));
  msg("[RRC][UE %d] Decoding DL-CCCH message (%d bytes)\n",Mod_id,Srb_info->Rx_buffer.W_idx);
  for (i=0;i<Srb_info->Rx_buffer.W_idx;i++)
    msg("%x.",Srb_info->Rx_buffer.Payload[i]);
  msg("\n");

  dec_rval = uper_decode(NULL,
			 &asn_DEF_DL_CCCH_Message,
			 (void**)&dl_ccch_msg,
			 (uint8_t*)Srb_info->Rx_buffer.Payload,
			 100,0,0);

  if (dl_ccch_msg->message.present == DL_CCCH_MessageType_PR_c1) {

    if (UE_rrc_inst[Mod_id].Info[CH_index].Status == RRC_PRE_SYNCHRO) {

      switch (dl_ccch_msg->message.choice.c1.present) {
      
      case DL_CCCH_MessageType__c1_PR_NOTHING :
	msg("[RRC][eNB %d] Frame %d : Received PR_NOTHING on DL-CCCH-Message\n",Mod_id,Mac_rlc_xface->frame);
	return;
	break;
      case DL_CCCH_MessageType__c1_PR_rrcConnectionReestablishment:
	msg("[RRC][eNB %d] Frame %d : Received RRCConnectionReestablishment on DL-CCCH-Message\n",Mod_id,Mac_rlc_xface->frame);
	return;
	break;
      case DL_CCCH_MessageType__c1_PR_rrcConnectionReestablishmentReject:
	msg("[RRC][eNB %d] Frame %d : Received RRCConnectionReestablishmentReject on DL-CCCH-Message\n",Mod_id,Mac_rlc_xface->frame);
	return;
	break;
      case DL_CCCH_MessageType__c1_PR_rrcConnectionReject:
	msg("[RRC][eNB %d] Frame %d : Received RRCConnectionReject on DL-CCCH-Message\n",Mod_id,Mac_rlc_xface->frame);
	return;
	break;
      case DL_CCCH_MessageType__c1_PR_rrcConnectionSetup:
	msg("[RRC][eNB %d] Frame %d : Received RRCConnectionSetup on DL-CCCH-Message\n",Mod_id,Mac_rlc_xface->frame);
	// Get configuration
	

	rrc_ue_process_radioResourceConfigDedicated(Mod_id,CH_index,
						    &dl_ccch_msg->message.choice.c1.choice.rrcConnectionSetup.criticalExtensions.choice.c1.choice.rrcConnectionSetup_r8.radioResourceConfigDedicated);

	rrc_ue_generate_RRCConnectionSetupComplete(Mod_id,CH_index);

	return;
	break;
      }       
    }
  }
}


s32 rrc_ue_establish_srb1(u8 Mod_id,u8 CH_index,
			 struct SRB_ToAddMod *SRB_config) { // add descriptor from RRC PDU

  u8 lchan_id = DCCH;

  UE_rrc_inst[Mod_id].Srb1[CH_index].Active = 1;
  UE_rrc_inst[Mod_id].Srb1[CH_index].Status = RADIO_CONFIG_OK;//RADIO CFG
  UE_rrc_inst[Mod_id].Srb1[CH_index].Srb_info.Srb_id = 1;

    // copy default configuration for now
  memcpy(&UE_rrc_inst[Mod_id].Srb1[CH_index].Srb_info.Lchan_desc[0],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
  memcpy(&UE_rrc_inst[Mod_id].Srb1[CH_index].Srb_info.Lchan_desc[1],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);


  msg("[RRC][UE %d], CONFIG_SRB1 %d corresponding to CH_index %d\n",
      Mod_id,
      lchan_id,
      CH_index);

  Mac_rlc_xface->rrc_rlc_config_req(Mod_id+NB_CH_INST,ACTION_ADD,lchan_id,SIGNALLING_RADIO_BEARER,Rlc_info_am_config);
  UE_rrc_inst[Mod_id].Srb1[CH_index].Srb_info.Tx_buffer.W_idx=DEFAULT_MEAS_IND_SIZE+1;
  UE_rrc_inst[Mod_id].Srb1[CH_index].Srb_info.Tx_buffer.R_idx=0;

  return(0);
}


s32 rrc_ue_establish_drb(u8 Mod_id,u8 CH_index,
			 struct DRB_ToAddMod *DRB_config) { // add descriptor from RRC PDU


  msg("[RRC][UE] Frame %d: Configuring DRB %d\n",
      Mac_rlc_xface->frame,*DRB_config->logicalChannelIdentity);

  switch (DRB_config->rlc_Config->present) {
  case RLC_Config_PR_NOTHING:
    msg("[RRC][UE] Frame %d: Received RLC_Config_PR_NOTHING!! for DRB Configuration\n");
    return(-1);
    break;
  case RLC_Config_PR_um_Bi_Directional :
    Mac_rlc_xface->rrc_rlc_config_req(Mod_id+NB_CH_INST,ACTION_ADD,
				      (CH_index * MAX_NUM_RB) + *DRB_config->logicalChannelIdentity,
				      RADIO_ACCESS_BEARER,Rlc_info_um);
    break;
  case RLC_Config_PR_um_Uni_Directional_UL :
  case RLC_Config_PR_um_Uni_Directional_DL :
  case RLC_Config_PR_am:
    msg("[RRC][UE] Frame %d: Illegal RLC mode for DRB\n");
    return(-1);
    break;
  }

  return(0);
}

void	rrc_ue_process_radioResourceConfigDedicated(u8 Mod_id,u8 CH_index,
						    RadioResourceConfigDedicated_t *radioResourceConfigDedicated) {

  long SRB_id,DRB_id;
  int i,ret,cnt;

  // Save physicalConfigDedicated if present
  if (radioResourceConfigDedicated->physicalConfigDedicated) {
    if (UE_rrc_inst[Mod_id].physicalConfigDedicated[CH_index]) {
      memcpy(UE_rrc_inst[Mod_id].physicalConfigDedicated[CH_index],radioResourceConfigDedicated->physicalConfigDedicated,
	     sizeof(struct PhysicalConfigDedicated));
      
    }
    else {
      UE_rrc_inst[Mod_id].physicalConfigDedicated[CH_index] = radioResourceConfigDedicated->physicalConfigDedicated;
    }
  }
  // Apply macMainConfig if present
  if (radioResourceConfigDedicated->mac_MainConfig) {
    if (radioResourceConfigDedicated->mac_MainConfig->present == RadioResourceConfigDedicated__mac_MainConfig_PR_explicitValue) 
      memcpy(&UE_rrc_inst[Mod_id].mac_MainConfig[CH_index],&radioResourceConfigDedicated->mac_MainConfig->choice.explicitValue,
	     sizeof(MAC_MainConfig_t));
  }

  // Apply spsConfig if present
  if (radioResourceConfigDedicated->sps_Config) {
    if (UE_rrc_inst[Mod_id].sps_Config[CH_index]) {
      memcpy(UE_rrc_inst[Mod_id].sps_Config[CH_index],radioResourceConfigDedicated->sps_Config,
	     sizeof(struct SPS_Config));
    }
    else {
      UE_rrc_inst[Mod_id].sps_Config[CH_index] = radioResourceConfigDedicated->sps_Config;
    }
  }
  // Establish SRBs if present
  // loop through SRBToAddModList
  if (radioResourceConfigDedicated->srb_ToAddModList) {
    
    for (cnt=0;cnt<radioResourceConfigDedicated->srb_ToAddModList->list.count;cnt++) {
      SRB_id = radioResourceConfigDedicated->srb_ToAddModList->list.array[cnt]->srb_Identity;
      if (SRB_id == 1) {
	if (UE_rrc_inst[Mod_id].SRB1_config[CH_index]) {
	  memcpy(UE_rrc_inst[Mod_id].SRB1_config[CH_index],radioResourceConfigDedicated->srb_ToAddModList->list.array[cnt],
		 sizeof(struct SRB_ToAddMod));
	}
	else {
	  UE_rrc_inst[Mod_id].SRB1_config[CH_index] = radioResourceConfigDedicated->srb_ToAddModList->list.array[cnt];
	  
	  ret = rrc_ue_establish_srb1(Mod_id,CH_index,radioResourceConfigDedicated->srb_ToAddModList->list.array[cnt]);
	}
      }
      else {
	if (UE_rrc_inst[Mod_id].SRB2_config[CH_index]) {
	  memcpy(UE_rrc_inst[Mod_id].SRB2_config[CH_index],radioResourceConfigDedicated->srb_ToAddModList->list.array[cnt],
		 sizeof(struct SRB_ToAddMod));
	}
	else {
	  UE_rrc_inst[Mod_id].SRB1_config[CH_index] = radioResourceConfigDedicated->srb_ToAddModList->list.array[cnt];
	  
	  //	  ret = rrc_ue_establish_srb2(Mod_id,CH_index,radioResourceConfigDedicated->srb_ToAddModList->list.array[cnt]);
	  msg("[RRC][UE] Can't establish SRB2 yet\n");
	}	
	
      }
    }
  }

  // Establish DRBs if present
  if (radioResourceConfigDedicated->drb_ToAddModList) {

    for (i=0;i<radioResourceConfigDedicated->drb_ToAddModList->list.count;i++) {
      DRB_id = radioResourceConfigDedicated->drb_ToAddModList->list.array[i]->drb_Identity-1;
      if (UE_rrc_inst[Mod_id].DRB_config[CH_index][DRB_id]) {
	memcpy(UE_rrc_inst[Mod_id].DRB_config[CH_index][DRB_id],radioResourceConfigDedicated->drb_ToAddModList->list.array[i],
	       sizeof(struct DRB_ToAddMod));
      }
      else {
	UE_rrc_inst[Mod_id].DRB_config[CH_index][DRB_id] = radioResourceConfigDedicated->drb_ToAddModList->list.array[i];
	
	ret = rrc_ue_establish_drb(Mod_id,CH_index,radioResourceConfigDedicated->drb_ToAddModList->list.array[i]);
      }
    }
  }

  UE_rrc_inst[Mod_id].Info[CH_index].Status = RRC_CONNECTED;

  Mac_rlc_xface->rrc_mac_config_req(Mod_id,0,0,CH_index,NULL,
				    radioResourceConfigDedicated->physicalConfigDedicated,NULL,NULL,NULL);

}


/*
s32 rrc_ue_establish_drbs(u8 Mod_id,u8 CH_index) {

  u8 lchan_id = DTCH;

  UE_rrc_inst[Mod_id].Srb1[CH_index].Active = 1;
  UE_rrc_inst[Mod_id].Srb1[CH_index].Status = RADIO_CONFIG_OK;//RADIO CFG
  UE_rrc_inst[Mod_id].Srb1[CH_index].Srb_info.Srb_id = 1;

    // copy default configuration for now
  memcpy(&UE_rrc_inst[Mod_id].Srb1[CH_index].Srb_info.Lchan_desc[0],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
  memcpy(&UE_rrc_inst[Mod_id].Srb1[CH_index].Srb_info.Lchan_desc[1],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);


  msg("[RRC][UE %d], CONFIG_SRB1 (%d) corresponding to CH_index %d\n",
      Mod_id,
      lchan_id,
      CH_index);

  Mac_rlc_xface->rrc_rlc_config_req(Mod_id+NB_CH_INST,ACTION_ADD,lchan_id,SIGNALLING_RADIO_BEARER,Rlc_info_am_config);
  UE_rrc_inst[Mod_id].Srb1[CH_index].Srb_info.Tx_buffer.W_idx=DEFAULT_MEAS_IND_SIZE+1;
  UE_rrc_inst[Mod_id].Srb1[CH_index].Srb_info.Tx_buffer.R_idx=0;

  return(0);
}
*/

u8 get_next_UE_index(u8 Mod_id,u8 *UE_identity) {

  u8 i,first_index = 255,reg=0;

  for (i=0;i<NB_CNX_CH;i++) {


    if ((first_index == 255) && (*(unsigned int*)CH_rrc_inst[Mod_id].Info.UE_list[i] == 0x00000000))
      first_index = i;  // save first free position

    if ((CH_rrc_inst[Mod_id].Info.UE_list[i][0]==UE_identity[0]) &&
	(CH_rrc_inst[Mod_id].Info.UE_list[i][1]==UE_identity[1]) &&
	(CH_rrc_inst[Mod_id].Info.UE_list[i][2]==UE_identity[2]) &&
	(CH_rrc_inst[Mod_id].Info.UE_list[i][3]==UE_identity[3]) &&
	(CH_rrc_inst[Mod_id].Info.UE_list[i][4]==UE_identity[4]))      // UE_identity already registered
      reg=1;

  }

  if (reg==0)
    return(first_index);
  else
    return(255);
}

/*------------------------------------------------------------------------------*/
void rrc_ch_decode_dcch(u8 Mod_id,  u8 UE_index, u8 *Rx_sdu, u8 sdu_size) {
  /*------------------------------------------------------------------------------*/

  u16 Idx,In_idx;

  asn_dec_rval_t dec_rval;
  UL_DCCH_Message_t *ul_dcch_msg;

  int i;

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
	rrc_ch_process_RRCConnectionReconfigurationComplete(Mod_id,UE_index,&ul_dcch_msg->message.choice.c1.choice.rrcConnectionReconfigurationComplete.criticalExtensions.choice.rrcConnectionReconfigurationComplete_r8);
      break;
    case UL_DCCH_MessageType__c1_PR_rrcConnectionReestablishmentComplete:
      break;
    case UL_DCCH_MessageType__c1_PR_rrcConnectionSetupComplete:
      msg("[RRC][eNB %d] Processing RRCConnectionSetupComplete message\n",Mod_id);
      if (ul_dcch_msg->message.choice.c1.choice.rrcConnectionSetupComplete.criticalExtensions.present == RRCConnectionSetupComplete__criticalExtensions__c1_PR_rrcConnectionSetupComplete_r8)
	rrc_ch_process_RRCConnectionSetupComplete(Mod_id,UE_index,&ul_dcch_msg->message.choice.c1.choice.rrcConnectionSetupComplete.criticalExtensions.choice.c1.choice.rrcConnectionSetupComplete_r8);
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
      


    }
  }
}


/*------------------------------------------------------------------------------*/
void rrc_ch_decode_ccch(u8 Mod_id, SRB_INFO *Srb_info){
  /*------------------------------------------------------------------------------*/

  u16 Idx,UE_index,In_idx;

  asn_dec_rval_t dec_rval;
  UL_CCCH_Message_t *ul_ccch_msg;
  RRCConnectionRequest_r8_IEs_t *rrcConnectionRequest;


  int i;

  msg("[RRC][eNB %d] Frame %d: Decoding CCCH %x.%x.%x.%x.%x.%x (%p)\n", Mod_id,Mac_rlc_xface->frame,
	(uint8_t*)Srb_info->Rx_buffer.Payload[0],
    (uint8_t*)Srb_info->Rx_buffer.Payload[1],
    (uint8_t*)Srb_info->Rx_buffer.Payload[2],
    (uint8_t*)Srb_info->Rx_buffer.Payload[3],
    (uint8_t*)Srb_info->Rx_buffer.Payload[4],
    (uint8_t*)Srb_info->Rx_buffer.Payload[5],
	(uint8_t*)Srb_info->Rx_buffer.Payload);
  dec_rval = uper_decode(NULL,
			 &asn_DEF_UL_CCCH_Message,
			 (void**)&ul_ccch_msg,
			 (uint8_t*)Srb_info->Rx_buffer.Payload,
			 100,0,0);

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
	memcpy(&CH_rrc_inst[Mod_id].Info.UE_list[UE_index],(u8 *)rrcConnectionRequest->ue_Identity.choice.randomValue.buf,5);
	
	msg("_______________________[OPENAIR][RRC]CH %d, Frame %d : Accept New connexion from UE %x%x%x%x%x (UE_index %d)____________\n",CH_rrc_inst[Mod_id].Node_id,Rrc_xface->Frame_index,
	    CH_rrc_inst[Mod_id].Info.UE_list[UE_index][0],
	    CH_rrc_inst[Mod_id].Info.UE_list[UE_index][1],
	    CH_rrc_inst[Mod_id].Info.UE_list[UE_index][2],
	    CH_rrc_inst[Mod_id].Info.UE_list[UE_index][3],
	    CH_rrc_inst[Mod_id].Info.UE_list[UE_index][4],
	    UE_index);
	
	//CONFIG SRB2  (DCCHs, ONE per User)  //meas && lchan Cfg
	//CH_rrc_inst[Mod_id].Info.Dtch_bd_config[UE_index].Status=NEED_RADIO_CONFIG;
	//CH_rrc_inst[Mod_id].Info.Dtch_bd_config[UE_index].Next_check_frame=Rrc_xface->Frame_index+1;
	CH_rrc_inst[Mod_id].Info.Nb_ue++;
	
#ifndef NO_RRM
	send_msg(&S_rrc,msg_rrc_MR_attach_ind(Mod_id,Mac_id));
#else
	
	
	Idx = (UE_index * MAX_NUM_RB) + DCCH;
	CH_rrc_inst[Mod_id].Srb1[UE_index].Active = 1;
	CH_rrc_inst[Mod_id].Srb1[UE_index].Srb_info.Srb_id = Idx;
	memcpy(&CH_rrc_inst[Mod_id].Srb1[UE_index].Srb_info.Lchan_desc[0],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
	memcpy(&CH_rrc_inst[Mod_id].Srb1[UE_index].Srb_info.Lchan_desc[1],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
	
	
	
	rrc_ch_generate_RRCConnectionSetup(Mod_id,UE_index);

	msg("[OPENAIR][RRC] CALLING RLC CONFIG SRB1 (rbid %d) for UE %d\n",
	    Idx,UE_index);
	Mac_rlc_xface->rrc_rlc_config_req(Mod_id,ACTION_ADD,Idx,SIGNALLING_RADIO_BEARER,Rlc_info_am_config);
	
	
#endif //NO_RRM      
	break;
      
    case UL_CCCH_MessageType__c1_PR_rrcConnectionReestablishmentRequest : 
      msg("[RRC][eNB %d] Frame %d : RRCConnectionReestablishmentRequest not supported yet\n",Mod_id,Mac_rlc_xface->frame);
      break;
      }
    }
    
    
  }
  
}



void rrc_ch_generate_RRCConnectionReconfiguration(u8 Mod_id,u16 UE_index) {

  u8 buffer[100];
  u8 size,i;

  //  CH_rrc_inst[Mod_id].Srb1.Tx_buffer.W_idx=0;


  // Get RRCConnectionSetup message and size (here 9)
  //  CH_rrc_inst[Mod_id].Srb1.Tx_buffer.W_idx = size;

  size = do_RRCConnectionReconfiguration(buffer,
					 UE_index,
					 0,
					 &CH_rrc_inst[Mod_id].SRB2_config[UE_index],
					 &CH_rrc_inst[Mod_id].DRB_config[UE_index][0],
					 &CH_rrc_inst[Mod_id].physicalConfigDedicated[UE_index]);

  msg("[RRC][eNB %d] Generate %d bytes (RRCConnectionReconfiguration) for DCCH UE %d: 1 ",Mod_id,size,UE_index);

  Mac_rlc_xface->rrc_rlc_data_req(Mod_id,(UE_index*MAX_NUM_RB)+DCCH,rrc_mui++,0,size,(char*)buffer);


}

void rrc_ch_process_RRCConnectionSetupComplete(u8 Mod_id, u8 UE_index,RRCConnectionSetupComplete_r8_IEs_t *rrcConnectionSetupComplete) {

  
  // process information

  Mac_rlc_xface->rrc_mac_config_req(Mod_id,1,UE_index,0,NULL,
				    CH_rrc_inst[Mod_id].physicalConfigDedicated[UE_index],NULL,NULL,NULL);

  // initiate RRCConnectionReconfiguration on SRB1
  rrc_ch_generate_RRCConnectionReconfiguration(Mod_id,UE_index);

}

void rrc_ch_process_RRCConnectionReconfigurationComplete(u8 Mod_id,u8 UE_index,RRCConnectionReconfigurationComplete_r8_IEs_t *rrcConnectionReconfigurationComplete){

    //Establish DRB (DTCH)
  msg("[RRC][eNB %d] Received RRCConnectionReconfigurationComplete from UE %d, configuring DRB %d/LCID %d\n",Mod_id,UE_index,
      CH_rrc_inst[Mod_id].DRB_config[UE_index][0]->drb_Identity,
      *CH_rrc_inst[Mod_id].DRB_config[UE_index][0]->logicalChannelIdentity);
  Mac_rlc_xface->rrc_rlc_config_req(Mod_id,ACTION_ADD,
				    (UE_index * MAX_NUM_RB) + *CH_rrc_inst[Mod_id].DRB_config[UE_index][0]->logicalChannelIdentity,
				    RADIO_ACCESS_BEARER,Rlc_info_um);

}

void rrc_ch_generate_RRCConnectionSetup(u8 Mod_id,u16 UE_index) {

  //  u8 size=9;
  //  u8 RRCConnectionSetup[size];
  u8 i;

  CH_rrc_inst[Mod_id].Srb0.Tx_buffer.W_idx=0;


  // Get RRCConnectionSetup message and size (here 9)

  /*
  RRCConnectionSetup[0]=0;
  for (i=1;i<size;i++) {
    RRCConnectionSetup[i] = (u8)(taus()&0xff);
    msg("%x ",RRCConnectionSetup[i]);
  }
  msg("\n");
  */

  CH_rrc_inst[Mod_id].Srb0.Tx_buffer.W_idx = do_RRCConnectionSetup((u8 *)CH_rrc_inst[Mod_id].Srb0.Tx_buffer.Payload,
								   UE_index,0,
								   &CH_rrc_inst[Mod_id].SRB1_config[UE_index],
								   &CH_rrc_inst[Mod_id].physicalConfigDedicated[UE_index]);

  msg("[RRC][eNB %d] Generate %d bytes (RRCConnectionSetup for UE %d) for CCCH : 0 ",Mod_id,CH_rrc_inst[Mod_id].Srb0.Tx_buffer.W_idx,UE_index);




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

/*------------------------------------------------------------------------------------------*/
void  rrc_ue_decode_dcch(u8 Mod_id,u8 *Buffer,u8 CH_index){
  /*------------------------------------------------------------------------------------------*/
  
  DL_DCCH_Message_t dldcchmsg;
  DL_DCCH_Message_t *dl_dcch_msg=&dldcchmsg;
  asn_dec_rval_t dec_rval;
  int i;
  
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

    if (UE_rrc_inst[Mod_id].Info[CH_index].Status == RRC_CONNECTED) {

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
	    Mac_rlc_xface->frame,CH_index);
	ue_rrc_process_rrcConnectionReconfiguration(Mod_id,&dl_dcch_msg->message.choice.c1.choice.rrcConnectionReconfiguration,CH_index);
	rrc_ue_generate_RRCConnectionReconfigurationComplete(Mod_id,CH_index);
	break;
      case DL_DCCH_MessageType__c1_PR_rrcConnectionRelease:
	break;
      case DL_DCCH_MessageType__c1_PR_securityModeCommand:
	break;
      case DL_DCCH_MessageType__c1_PR_ueCapabilityEnquiry:
	break;
      case DL_DCCH_MessageType__c1_PR_counterCheck:
	break;
      }
    }
  }
#ifndef NO_RRM
    send_msg(&S_rrc,msg_rrc_end_scan_req(Mod_id+NB_CH_INST,CH_index));
#endif
}

const char siWindowLength[7][5] = {"1ms\0","2ms\0","5ms\0","10ms\0","15ms\0","20ms\0","40ms\0"};
const char siWindowLength_int[7] = {1,2,5,10,15,20,40};

const char SIBType[16][6] ={"SIB3\0","SIB4\0","SIB5\0","SIB6\0","SIB7\0","SIB8\0","SIB9\0","SIB10\0","SIB11\0","Sp0\0","Sp1\0","Sp2\0","Sp3\0","Sp4\0"};
const char SIBPeriod[7][7]= {"80ms\0","160ms\0","320ms\0","640ms\0","1280ms\0","2560ms\0","5120ms\0"};

void decode_SIB1(u8 Mod_id,u8 CH_index) {
  asn_dec_rval_t dec_rval;
  SystemInformationBlockType1_t **sib1=&UE_rrc_inst[Mod_id].sib1[CH_index];
  int i;

  dec_rval = uper_decode(NULL,
			 &asn_DEF_SystemInformationBlockType1,
			 (void**)sib1,
			 (uint8_t*)UE_rrc_inst[Mod_id].SIB1[CH_index],
			 100,0,0);
  msg("[RRC][UE %d] Frame %d : Dumping SIB 1 (%d bytes)\n",Mod_id,Mac_rlc_xface->frame,dec_rval.consumed);
  for (i=0;i<18;i++)
    msg("%x.",UE_rrc_inst[Mod_id].SIB1[CH_index][i]);
  msg("\n");
  
  msg("cellAccessRelatedInfo.cellIdentity : %x.%x.%x.%x\n",
      (*sib1)->cellAccessRelatedInfo.cellIdentity.buf[0],
      (*sib1)->cellAccessRelatedInfo.cellIdentity.buf[1],
      (*sib1)->cellAccessRelatedInfo.cellIdentity.buf[2],
      (*sib1)->cellAccessRelatedInfo.cellIdentity.buf[3]);

  msg("cellSelectionInfo.q_RxLevMin       : %d\n",(int)(*sib1)->cellSelectionInfo.q_RxLevMin);
  msg("freqBandIndicator                  : %d\n",(int)(*sib1)->freqBandIndicator);
  msg("siWindowLength                     : %s\n",siWindowLength[(*sib1)->si_WindowLength]);
  msg("siSchedulingInfoSIBType[0]         : %s\n",SIBType[(int)(*sib1)->schedulingInfoList.list.array[0]->sib_MappingInfo.list.array[0]->buf[0]]);
  msg("siSchedulingInfoPeriod[0]          : %s\n",SIBPeriod[(int)(*sib1)->schedulingInfoList.list.array[0]->si_Periodicity]);

  if ((*sib1)->tdd_Config)
    msg("TDD subframe assignment            : %d\nS-Subframe Config                  : %d\n",(*sib1)->tdd_Config->subframeAssignment,(*sib1)->tdd_Config->specialSubframePatterns);

  UE_rrc_inst[Mod_id].Info[CH_index].SIperiod    =8<<((int)(*sib1)->schedulingInfoList.list.array[0]->si_Periodicity);
  UE_rrc_inst[Mod_id].Info[CH_index].SIwindowsize=siWindowLength_int[(int)(*sib1)->schedulingInfoList.list.array[0]->sib_MappingInfo.list.array[0]->buf[0]];

  Mac_rlc_xface->rrc_mac_config_req(Mod_id,0,0,CH_index,
				    (RadioResourceConfigCommonSIB_t *)NULL,
				    (struct PhysicalConfigDedicated *)NULL,
				    UE_rrc_inst[Mod_id].sib1[CH_index]->tdd_Config,
				    &UE_rrc_inst[Mod_id].Info[CH_index].SIwindowsize,
				    &UE_rrc_inst[Mod_id].Info[CH_index].SIperiod);
}


void dump_sib2(SystemInformationBlockType2_t *sib2) {

  msg("radioResourceConfigCommon.rach_ConfigCommon.preambleInfo.numberOfRA_Preambles : %d\n",
      sib2->radioResourceConfigCommon.rach_ConfigCommon.preambleInfo.numberOfRA_Preambles);

  //  if (radioResourceConfigCommon.rach_ConfigCommon.preambleInfo.preamblesGroupAConfig)
  //msg("radioResourceConfigCommon.rach_ConfigCommon.preambleInfo.preamblesGroupAConfig ",sib2->radioResourceConfigCommon.rach_ConfigCommon.preambleInfo.preamblesGroupAConfig = NULL; 

  msg("radioResourceConfigCommon.rach_ConfigCommon.powerRampingParameters.powerRampingStep : %d\n",sib2->radioResourceConfigCommon.rach_ConfigCommon.powerRampingParameters.powerRampingStep);

  msg("radioResourceConfigCommon.rach_ConfigCommon.powerRampingParameters.preambleInitialReceivedTargetPower : %d\n",sib2->radioResourceConfigCommon.rach_ConfigCommon.powerRampingParameters.preambleInitialReceivedTargetPower);
  
  msg("radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.preambleTransMax  : %d\n",sib2->radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.preambleTransMax);
  
  msg("radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.ra_ResponseWindowSize : %d\n",sib2->radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.ra_ResponseWindowSize);
  
  msg("radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.mac_ContentionResolutionTimer : %d\n",sib2->radioResourceConfigCommon.rach_ConfigCommon.ra_SupervisionInfo.mac_ContentionResolutionTimer);

  msg("radioResourceConfigCommon.rach_ConfigCommon.maxHARQ_Msg3Tx : %d\n",
      sib2->radioResourceConfigCommon.rach_ConfigCommon.maxHARQ_Msg3Tx);

  msg("radioResourceConfigCommon.prach_Config.rootSequenceIndex : %d\n",sib2->radioResourceConfigCommon.prach_Config.rootSequenceIndex);
  msg("radioResourceConfigCommon.prach_Config.prach_ConfigInfo.prach_ConfigIndex : %d\n",sib2->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.prach_ConfigIndex);
  msg("radioResourceConfigCommon.prach_Config.prach_ConfigInfo.highSpeedFlag : %d\n",  sib2->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.highSpeedFlag);
  msg("radioResourceConfigCommon.prach_Config.prach_ConfigInfo.zeroCorrelationZoneConfig : %d\n",  sib2->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.zeroCorrelationZoneConfig);
  msg("radioResourceConfigCommon.prach_Config.prach_ConfigInfo.prach_FreqOffset %d\n",  sib2->radioResourceConfigCommon.prach_Config.prach_ConfigInfo.prach_FreqOffset);

  // PDSCH-Config
  msg("radioResourceConfigCommon.pdsch_ConfigCommon.referenceSignalPower  : %d\n",sib2->radioResourceConfigCommon.pdsch_ConfigCommon.referenceSignalPower);
  msg("radioResourceConfigCommon.pdsch_ConfigCommon.p_b : %d\n",sib2->radioResourceConfigCommon.pdsch_ConfigCommon.p_b);

  // PUSCH-Config
  msg("radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.n_SB  : %d\n",sib2->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.n_SB);
  msg("radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.hoppingMode  : %d\n",sib2->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.hoppingMode);
  msg("radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.pusch_HoppingOffset : %d\n",sib2->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.pusch_HoppingOffset);
  msg("radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.enable64QAM : %d\n",sib2->radioResourceConfigCommon.pusch_ConfigCommon.pusch_ConfigBasic.enable64QAM);
  msg("radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupHoppingEnabled : %d\n",sib2->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupHoppingEnabled);
  msg("radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH : %d\n",sib2->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.groupAssignmentPUSCH);
  msg("radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled : %d\n",sib2->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.sequenceHoppingEnabled);
  msg("radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.cyclicShift : %d\n",sib2->radioResourceConfigCommon.pusch_ConfigCommon.ul_ReferenceSignalsPUSCH.cyclicShift);

  // PUCCH-Config

  msg("radioResourceConfigCommon.pucch_ConfigCommon.deltaPUCCH_Shift : %d\n",sib2->radioResourceConfigCommon.pucch_ConfigCommon.deltaPUCCH_Shift);
  msg("radioResourceConfigCommon.pucch_ConfigCommon.nRB_CQI : %d\n",sib2->radioResourceConfigCommon.pucch_ConfigCommon.nRB_CQI);
  msg("radioResourceConfigCommon.pucch_ConfigCommon.nCS_AN : %d\n",sib2->radioResourceConfigCommon.pucch_ConfigCommon.nCS_AN);
  msg("radioResourceConfigCommon.pucch_ConfigCommon.n1PUCCH_AN : %d\n",sib2->radioResourceConfigCommon.pucch_ConfigCommon.n1PUCCH_AN);

  msg("radioResourceConfigCommon.soundingRS_UL_ConfigCommon.present : %d\n",sib2-> radioResourceConfigCommon.soundingRS_UL_ConfigCommon.present);


  // uplinkPowerControlCommon

  msg("radioResourceConfigCommon.uplinkPowerControlCommon.p0_NominalPUSCH : %d\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.p0_NominalPUSCH);
  msg("radioResourceConfigCommon.uplinkPowerControlCommon.alpha : %d\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.alpha);

  msg("radioResourceConfigCommon.uplinkPowerControlCommon.p0_NominalPUCCH : %d\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.p0_NominalPUCCH);
  msg("radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format1 : %d\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format1);
  msg("radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format1b :%d\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format1b);
  msg("radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2  :%d\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2);
  msg("radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2a :%d\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2a);
  msg("radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2b :%d\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.deltaFList_PUCCH.deltaF_PUCCH_Format2b);

  msg("radioResourceConfigCommon.uplinkPowerControlCommon.deltaPreambleMsg3 : %d\n",sib2->radioResourceConfigCommon.uplinkPowerControlCommon.deltaPreambleMsg3);

  msg("radioResourceConfigCommon.ul_CyclicPrefixLength : %d\n", sib2->radioResourceConfigCommon.ul_CyclicPrefixLength);

  msg("ue_TimersAndConstants.t300 : %d\n", sib2->ue_TimersAndConstants.t300);
  msg("ue_TimersAndConstants.t301 : %d\n", sib2->ue_TimersAndConstants.t301);
  msg("ue_TimersAndConstants.t310 : %d\n", sib2->ue_TimersAndConstants.t310);
  msg("ue_TimersAndConstants.n310 : %d\n", sib2->ue_TimersAndConstants.n310);
  msg("ue_TimersAndConstants.t311 : %d\n", sib2->ue_TimersAndConstants.t311);
  msg("ue_TimersAndConstants.n311 : %d\n", sib2->ue_TimersAndConstants.n311);

  msg("freqInfo.additionalSpectrumEmission : %d\n",sib2->freqInfo.additionalSpectrumEmission);
  msg("freqInfo.ul_CarrierFreq : %d\n",sib2->freqInfo.ul_CarrierFreq);
  msg("freqInfo.ul_Bandwidth : %d\n",sib2->freqInfo.ul_Bandwidth);
  msg("mbsfn_SubframeConfigList : %d\n",sib2->mbsfn_SubframeConfigList);
  msg("timeAlignmentTimerCommon : %d\n",sib2->timeAlignmentTimerCommon);



}

void dump_sib3(SystemInformationBlockType3_t *sib3) {

}

//const char SIBPeriod[7][7]= {"80ms\0","160ms\0","320ms\0","640ms\0","1280ms\0","2560ms\0","5120ms\0"};
void decode_SI(u8 Mod_id,u8 CH_index,u8 si_window) {

  asn_dec_rval_t dec_rval;
  SystemInformation_t **si=&UE_rrc_inst[Mod_id].si[CH_index][si_window];
  int i;
  struct SystemInformation_r8_IEs__sib_TypeAndInfo__Member *typeandinfo;

  if (si_window>8) {
    msg("[RRC][UE], not enough windows (%d>8)\n",si_window);
    return;
  }

  dec_rval = uper_decode(NULL,
			 &asn_DEF_SystemInformation,
			 (void**)si,
			 (uint8_t*)UE_rrc_inst[Mod_id].SI[CH_index],
			 100,0,0);
  msg("[RRC][UE %d] Frame %d : Dumping SI from window %d (%d bytes)\n",Mod_id,Mac_rlc_xface->frame,si_window,dec_rval.consumed);
  for (i=0;i<30;i++)
    msg("%x.",UE_rrc_inst[Mod_id].SI[CH_index][i]);
  msg("\n");

  // Dump contents
  if ((*si)->criticalExtensions.present==SystemInformation__criticalExtensions_PR_systemInformation_r8)
    typeandinfo=(*si)->criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list.array[0];
  else {
    msg("[RRC][UE] Unknown criticalExtension version (not Rel8)\n");
    return;
  }
  for (i=0;(typeandinfo!=NULL);i++) {
    switch(typeandinfo->present) {
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib2:
      UE_rrc_inst[Mod_id].sib2[CH_index] = &typeandinfo->choice.sib2;
      msg("[RRC][UE] Found SIB2\n");
      dump_sib2(UE_rrc_inst[Mod_id].sib2[CH_index]);
      Mac_rlc_xface->rrc_mac_config_req(Mod_id,0,0,CH_index,
					&UE_rrc_inst[Mod_id].sib2[CH_index]->radioResourceConfigCommon,
					(struct PhysicalConfigDedicated_t *)NULL,
					(TDD_Config_t *)NULL,
					NULL,
					NULL);
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib3:
      UE_rrc_inst[Mod_id].sib3[CH_index] = &typeandinfo->choice.sib3;
      msg("[RRC][UE] Found SIB3\n");
      dump_sib3(UE_rrc_inst[Mod_id].sib3[CH_index]);
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib4:
      UE_rrc_inst[Mod_id].sib4[CH_index] = &typeandinfo->choice.sib4;
      msg("[RRC][UE] Found SIB4\n");
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib5:
      UE_rrc_inst[Mod_id].sib5[CH_index] = &typeandinfo->choice.sib5;
      msg("[RRC][UE] Found SIB5\n");
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib6:
      UE_rrc_inst[Mod_id].sib6[CH_index] = &typeandinfo->choice.sib6;
      msg("[RRC][UE] Found SIB6\n");
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib7:
      UE_rrc_inst[Mod_id].sib7[CH_index] = &typeandinfo->choice.sib7;
      msg("[RRC][UE] Found SIB7\n");
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib8:
      UE_rrc_inst[Mod_id].sib8[CH_index] = &typeandinfo->choice.sib8;
      msg("[RRC][UE] Found SIB8\n");
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib9:
      UE_rrc_inst[Mod_id].sib9[CH_index] = &typeandinfo->choice.sib9;
      msg("[RRC][UE] Found SIB9\n");
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib10:
      UE_rrc_inst[Mod_id].sib10[CH_index] = &typeandinfo->choice.sib10;
      msg("[RRC][UE] Found SIB10\n");
      break;
    case SystemInformation_r8_IEs__sib_TypeAndInfo__Member_PR_sib11:
      UE_rrc_inst[Mod_id].sib11[CH_index] = &typeandinfo->choice.sib11;
      msg("[RRC][UE] Found SIB11\n");
      break;
    default:
      break;
    }

    typeandinfo=(*si)->criticalExtensions.choice.systemInformation_r8.sib_TypeAndInfo.list.array[1+i];
    
  } 
}



#ifndef USER_MODE
EXPORT_SYMBOL(Rlc_info_am_config);
#endif
