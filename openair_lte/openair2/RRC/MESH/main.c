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
  Rrc_xface->def_meas_ind=def_meas_ind;  
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

/*------------------------------------------------------------------------------*/
char openair_rrc_ch_init(u8 Mod_id){
  /*-----------------------------------------------------------------------------*/

  unsigned char i,j,k;
  CH_rrc_inst[Mod_id].Node_id=Mac_rlc_xface->Node_id[Mod_id];
  msg("[OPENAIR][RRC][INIT CH] Mod_id:%d, Node_id=%d\n",Mod_id,CH_rrc_inst[Mod_id].Node_id);
  CH_rrc_inst[Mod_id].Info.Status = CH_READY;
  CH_rrc_inst[Mod_id].Info.Nb_ue=0;
  CH_rrc_inst[Mod_id].Mac_id.L2_id[0]=CH_rrc_inst[Mod_id].Node_id;
  
  for(j=1;j<sizeof(L2_ID);j++)
    CH_rrc_inst[Mod_id].Mac_id.L2_id[j]=0;
  
  memcpy(&CH_rrc_inst[Mod_id].Info.UE_list[0],&CH_rrc_inst[Mod_id].Mac_id,sizeof(L2_ID));
  
  for(j=1;j<=NB_CNX_CH;j++){	
    memset(CH_rrc_inst[Mod_id].Info.UE_list[j],0,5);	
    CH_rrc_inst[Mod_id].Nb_rb[j]=0;
  }
  
  CH_rrc_inst[Mod_id].Srb0.Active=0;
  
  for(j=0;j<(NB_CNX_CH+1);j++){	
    CH_rrc_inst[Mod_id].Srb2[j].Active=0;
    CH_rrc_inst[Mod_id].Srb2[j].Srb_info.Meas_entry=NULL;
    CH_rrc_inst[Mod_id].Srb2_meas[j].Status=IDLE;
  }
  
  for(j=0;j<NB_RAB_MAX;j++)
    for(i=0;i<(NB_CNX_CH+1);i++){
      CH_rrc_inst[Mod_id].Rab[j][i].Active=0;
      CH_rrc_inst[Mod_id].Rab[j][i].Rb_info.Meas_entry=NULL;
      CH_rrc_inst[Mod_id].Rab_meas[j][i].Status=IDLE;
    }	
  
  for(j=0;j<NB_RAB_MAX;j++)
    for(i=0;i<(NB_CNX_CH+1);i++)
      for(k=0;k<(NB_CNX_CH-1);k++){
	CH_rrc_inst[Mod_id].Rab_dil[j][i][k].Active=0;
	CH_rrc_inst[Mod_id].Rab_dil[j][i][k].Rb_info.Meas_entry=NULL;
	CH_rrc_inst[Mod_id].Rab_dil_meas[j][i][k].Status=IDLE;
      }
  
  //  CH_rrc_inst[Mod_id].Def_meas[0]= &CH_mac_inst[Mod_id].Def_meas[0];
  //  CH_rrc_inst[Mod_id].Def_meas[0]->Status = RADIO_CONFIG_OK;
  //  CH_rrc_inst[Mod_id].Def_meas[0]->Forg_fact=1;
  //  CH_rrc_inst[Mod_id].Def_meas[0]->Rep_interval=50;
  //  CH_rrc_inst[Mod_id].Def_meas[0]->Last_report_frame=Rrc_xface->Frame_index;
    
  for(i=1;i<(NB_CNX_CH+1);i++){
    CH_rrc_inst[Mod_id].Def_meas[i]=NULL;
    CH_rrc_inst[Mod_id].Info.Dtch_bd_config[i].Status=IDLE;
    CH_rrc_inst[Mod_id].Info.Dtch_bd_config[i].Next_check_frame=0;
  }   
  
  msg("[OPENAIR][RRC][INIT] INIT OK for Mod_id:%d, Node_id=%d\n",Mod_id,CH_rrc_inst[Mod_id].Node_id);
  
  //To be modified
  CH_rrc_inst[Mod_id].Mac_id.L2_id[0]= Mac_rlc_xface->Node_id[Mod_id];
  
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
char openair_rrc_mr_init(u8 Mod_id, unsigned char CH_IDX){
  /*-----------------------------------------------------------------------------*/

  unsigned char i,j,k,CH_index;
    UE_rrc_inst[Mod_id-NB_CH_INST].Node_id=Mac_rlc_xface->Node_id[Mod_id];
    Mod_id-=NB_CH_INST;
    msg("[OPENAIR][RRC][INIT] Mod_id:%d, Node_id=%d\n",Mod_id,UE_rrc_inst[Mod_id].Node_id);

    for(CH_index =0; CH_index < NB_CNX_UE;CH_index++){
      for(j=0;j<NB_RAB_MAX;j++){
	UE_rrc_inst[Mod_id].Rab[j][CH_index].Active=0;
	UE_rrc_inst[Mod_id].Rab[j][CH_index].Status=IDLE;
	UE_rrc_inst[Mod_id].Rab[j][CH_index].Rb_info.Meas_entry=NULL;
	for(k=0;k<(NB_CNX_CH-1);k++){
	  UE_rrc_inst[Mod_id].Rab_dil[j][k][CH_index].Active=0;
	  UE_rrc_inst[Mod_id].Rab_dil[j][k][CH_index].Rb_info.Meas_entry=NULL;
	}
      }
      
      UE_rrc_inst[Mod_id].Mac_id.L2_id[0]=UE_rrc_inst[Mod_id].Node_id;
      for(j=1;j<sizeof(L2_ID);j++)
	UE_rrc_inst[Mod_id].Mac_id.L2_id[j]=0;
      
      UE_rrc_inst[Mod_id].Info[CH_index].Status=RRC_IDLE;
      UE_rrc_inst[Mod_id].Info[CH_index].Rach_tx_cnt=0;	
      UE_rrc_inst[Mod_id].Info[CH_index].Nb_bcch_wait=0;	
      UE_rrc_inst[Mod_id].Info[CH_index].UE_index=0xffff;
      UE_rrc_inst[Mod_id].Srb0[CH_index].Active=0;
      UE_rrc_inst[Mod_id].Srb1[CH_index].Active=0;
      UE_rrc_inst[Mod_id].Srb2[CH_index].Active=0;
      UE_rrc_inst[Mod_id].Srb2[CH_index].Srb_info.Meas_entry=NULL;
      UE_rrc_inst[Mod_id].Def_meas[CH_index]=NULL;
      
      msg("RRC: INIT: UE %d phy_sync_2_ch_ind from Inst %d\n", UE_rrc_inst[Mod_id].Mac_id.L2_id[0],Mod_id+NB_CH_INST);

#ifndef NO_RRM
      send_msg(&S_rrc,msg_rrc_phy_synch_to_CH_ind(Mod_id+NB_CH_INST,CH_index,UE_rrc_inst[Mod_id].Mac_id));
#endif
  }
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
  Srb_info->Header_rx = 0;
  Srb_info->Rx_buffer.Nb_tb_max = Srb_info->Lchan_desc[0].max_transport_blocks;
  
  Srb_info->Tx_buffer.R_idx = 0;
  Srb_info->Tx_buffer.W_idx = 0;
  Srb_info->Tx_buffer.Tb_size = Srb_info->Lchan_desc[1].transport_block_size;
  Srb_info->Header_tx = 0;
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

                
      msg("[OPENAIR][RRC][RRC_ON] NODE %d, Config BCCH for TB_size %d\n",NODE_ID[Mod_id],
	     CH_rrc_inst[Mod_id].Srb0.Lchan_desc[1].transport_block_size);
      CH_rrc_inst[Mod_id].Srb0.Active=1;

      

      
      memcpy(&CH_rrc_inst[Mod_id].Srb0.Lchan_desc[0],&CCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
      memcpy(&CH_rrc_inst[Mod_id].Srb0.Lchan_desc[1],&CCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
      rrc_config_buffer(&CH_rrc_inst[Mod_id].Srb0,CCCH,1);
      ((CH_CCCH_HEADER*)(&CH_rrc_inst[Mod_id].Srb0.Tx_buffer.Header[0]))->Rv_tb_idx=0;
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
      ((CH_CCCH_HEADER*)(&UE_rrc_inst[Mod_id].Srb0[i].Rx_buffer.Header[0]))->Rv_tb_idx=0;
      UE_rrc_inst[Mod_id].Srb0[i].Active=1;

    }
  }
  

}
#endif //NO_RRM


/*------------------------------------------------------------------------------*/
void ue_rrc_decode_bcch(u8 Mod_id){
  /*------------------------------------------------------------------------------*/
}

/*------------------------------------------------------------------------------*/
void rrc_ue_generate_RRCConnectionRequest(u8 Mod_id, unsigned char Idx){ 
  /*------------------------------------------------------------------------------*/

  u8 i=0;

  if(UE_rrc_inst[Mod_id].Srb0[Idx].Tx_buffer.W_idx ==0){

    // Get RRCConnectionRequest, fill random for now
    for (i=0;i<6;i++)
      UE_rrc_inst[Mod_id].Srb0[Idx].Tx_buffer.Payload[i] = taus()&0xff;

    UE_rrc_inst[Mod_id].Srb0[Idx].Tx_buffer.W_idx =i;
   
  }
}

mui_t rrc_mui=0;

void rrc_ue_generate_RRCConnectionSetupComplete(u8 Mod_id,u8 CH_index) {

  u8 RRCConnectionSetupComplete[32], size=5,i;

  msg("[RRC][UE %d] Generating RRCConnectionSetupComplete : 0 ",Mod_id);
  RRCConnectionSetupComplete[0]=0;
  for (i=1;i<size;i++) {
    RRCConnectionSetupComplete[i] = (u8)(taus()&0xff);
    msg("%x ",RRCConnectionSetupComplete[i]);
  }
  msg("\n");
  Mac_rlc_xface->rrc_rlc_data_req(Mod_id+NB_CH_INST,DCCH,rrc_mui++,0,size,(char*)RRCConnectionSetupComplete);
  UE_rrc_inst[Mod_id].Info[0].Status = RRC_ASSOCIATED;
}

void rrc_ue_generate_RRCConnectionReconfigurationComplete(u8 Mod_id,u8 CH_index) {

  u8 RRCConnectionReconfigurationComplete[32], size=5,i;

  msg("[RRC][UE %d] Generating RRCConnectionReconfigurationComplete : 1 ",Mod_id);
  RRCConnectionReconfigurationComplete[0]=1;
  for (i=1;i<size;i++) {
    RRCConnectionReconfigurationComplete[i] = (u8)(taus()&0xff);
    msg("%x ",RRCConnectionReconfigurationComplete[i]);
  }
  msg("\n");
  Mac_rlc_xface->rrc_rlc_data_req(Mod_id+NB_CH_INST,DCCH,rrc_mui++,0,size,(char*)RRCConnectionReconfigurationComplete);
  UE_rrc_inst[Mod_id].Info[0].Status = RRC_CONNECTED;
}


/*------------------------------------------------------------------------------*/
void ue_rrc_decode_ccch(u8 Mod_id, SRB_INFO *Srb_info, u8 CH_index){
  /*------------------------------------------------------------------------------*/

  unsigned char ret;
  msg("[RRC][UE %d] Decoding CCCH\n",Mod_id);
  // Decode packet
  switch (Srb_info->Rx_buffer.Payload[0]) {
  case 0: // ConnectionSetup
    if (UE_rrc_inst[Mod_id].Info[CH_index].Status == RRC_PRE_SYNCHRO) {
      ret = rrc_ue_establish_srb1(Mod_id,CH_index);
      if (ret==0) {
	UE_rrc_inst[Mod_id].Info[CH_index].Status == RRC_PRE_ASSOCIATED;
	rrc_ue_generate_RRCConnectionSetupComplete(Mod_id,CH_index);
      } 
    }
    break;
  case 1: // ConnectionReject
  case 2: // ConnectionReestablishment
  case 3: // ConnectionReestablishmentReject
    break;

  }
}


s32 rrc_ue_establish_srb1(u8 Mod_id,u8 CH_index) { // add descriptor from RRC PDU

  u8 lchan_id = DCCH;

  UE_rrc_inst[Mod_id].Srb1[CH_index].Active = 1;
  UE_rrc_inst[Mod_id].Srb1[CH_index].Status = RADIO_CONFIG_OK;//RADIO CFG
  UE_rrc_inst[Mod_id].Srb1[CH_index].Srb_info.Srb_id = 1;
  
    // copy default configuration for now
  memcpy(&UE_rrc_inst[Mod_id].Srb1[CH_index].Srb_info.Lchan_desc[0],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
  memcpy(&UE_rrc_inst[Mod_id].Srb1[CH_index].Srb_info.Lchan_desc[1],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
    

  msg("[RRC][UE %d], CONFIG_SRB1 %d(%d) corresponding to CH_index %d\n",
      Mod_id,
      lchan_id,
      CH_index);

  Mac_rlc_xface->rrc_rlc_config_req(Mod_id+NB_CH_INST,ACTION_ADD,lchan_id,SIGNALLING_RADIO_BEARER,Rlc_info_am_config);
  UE_rrc_inst[Mod_id].Srb1[CH_index].Srb_info.Tx_buffer.W_idx=DEFAULT_MEAS_IND_SIZE+1;
  UE_rrc_inst[Mod_id].Srb1[CH_index].Srb_info.Tx_buffer.R_idx=0;
  
  return(0);
}

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
void ch_rrc_decode_ccch(u8 Mod_id, SRB_INFO *Srb_info){
  /*------------------------------------------------------------------------------*/
  u8 i=0;
  MAC_CONFIG_REQ Mac_config_req;
  MAC_MEAS_REQ Mac_meas_req;
  u16 Idx,UE_index,In_idx; 

  //  L2_ID Mac_id;
  //  memcpy(&Mac_id.L2_id[0],&Srb_info->Rx_buffer.Payload[i],sizeof(L2_ID));
  //  i+=sizeof(L2_ID);
  
  //  if((Srb_info->Rx_buffer.Payload[i] !=0) && (Srb_info->Rx_buffer.Payload[i] !=1)) {
  //    msg("FATAL: RACH for CH %d!!!!\n",Srb_info->Rx_buffer.Payload[i]);
  //    Mac_rlc_xface->macphy_exit("");  
  //  }

  // Decode RRCConnectionRequest

  UE_index = get_next_UE_index(Mod_id,(u8 *)&Srb_info->Rx_buffer.Payload);

  if (UE_index!=255) {
  
  //  if((Srb_info->Rx_buffer.Payload[i] == CH_rrc_inst[Mod_id].Node_id )&& (!rrc_is_mobile_already_associated(Mod_id,Mac_id))){
    
    //until dynamic classification
  //    UE_index=Mac_id.L2_id[0]-NB_CH_MAX+1;
  //    if(UE_index!=0xffff){
      
  //      memcpy(&Rrc_xface->UE_id[Mod_id][UE_index],&Mac_id,sizeof(L2_ID));
      memcpy(&Rrc_xface->UE_id[Mod_id][UE_index],Srb_info->Rx_buffer.Payload,5);
      //      memcpy(&CH_rrc_inst[Mod_id].Info.UE_list[UE_index],&Mac_id,sizeof(L2_ID));
      memcpy(&CH_rrc_inst[Mod_id].Info.UE_list[UE_index],&Srb_info->Rx_buffer.Payload,5);

      msg("_______________________[OPENAIR][RRC]CH %d, Frame %d : Accept New connexion from UE %x%x%x%x%x (UE_index %d)____________\n",CH_rrc_inst[Mod_id].Node_id,Rrc_xface->Frame_index,
	  CH_rrc_inst[Mod_id].Info.UE_list[UE_index][0],
	  CH_rrc_inst[Mod_id].Info.UE_list[UE_index][1],
	  CH_rrc_inst[Mod_id].Info.UE_list[UE_index][2],
	  CH_rrc_inst[Mod_id].Info.UE_list[UE_index][3],
	  CH_rrc_inst[Mod_id].Info.UE_list[UE_index][4],
	  UE_index);

      //CONFIG SRB2  (DCCHs, ONE per User)  //meas && lchan Cfg 
      CH_rrc_inst[Mod_id].Info.Dtch_bd_config[UE_index].Status=NEED_RADIO_CONFIG;
      CH_rrc_inst[Mod_id].Info.Dtch_bd_config[UE_index].Next_check_frame=Rrc_xface->Frame_index+1;
      CH_rrc_inst[Mod_id].Info.Nb_ue++;	
      
#ifndef NO_RRM
      send_msg(&S_rrc,msg_rrc_MR_attach_ind(Mod_id,Mac_id));
#else
      
      //      Mac_config_req.Lchan_type = DCCH;
      //      Mac_config_req.UE_CH_index = UE_index; 
      //      memcpy(&Mac_config_req.Lchan_desc[0],(LCHAN_DESC*)&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx 
      //      memcpy(&Mac_config_req.Lchan_desc[1],(LCHAN_DESC*)&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
      //      Mac_config_req.Lchan_id.Index=(UE_index << RAB_SHIFT2) + DCCH;
      //      Idx = Mac_rlc_xface->mac_config_req(Mod_id,ADD_LC,&Mac_config_req);

      Idx = (UE_index * MAX_NUM_RB) + DCCH;
      CH_rrc_inst[Mod_id].Srb1[UE_index].Active = 1;
      CH_rrc_inst[Mod_id].Srb1[UE_index].Next_check_frame = Rrc_xface->Frame_index + 250;
      CH_rrc_inst[Mod_id].Srb1[UE_index].Status = NEED_RADIO_CONFIG;//RADIO CFG
      CH_rrc_inst[Mod_id].Srb1[UE_index].Srb_info.Srb_id = Idx;
      memcpy(&CH_rrc_inst[Mod_id].Srb1[UE_index].Srb_info.Lchan_desc[0],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
      memcpy(&CH_rrc_inst[Mod_id].Srb1[UE_index].Srb_info.Lchan_desc[1],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
      
      msg("[OPENAIR][RRC] NODE=%d, CALLING RLC CONFIG SRB1\n",
	  CH_rrc_inst[Mod_id].Node_id,Idx,Mac_config_req.Lchan_id.Index);
      Mac_rlc_xface->rrc_rlc_config_req(Mod_id,ACTION_ADD,Idx,SIGNALLING_RADIO_BEARER,Rlc_info_am_config);

      /*      
      //Configure a correponding measurement process
      Mac_meas_req.Lchan_id.Index = Idx;
      Mac_meas_req.Meas_trigger = DCCH_MEAS_TRIGGER;
      Mac_meas_req.Mac_avg = DCCH_MEAS_AVG;
      Mac_meas_req.Rep_amount = 0;
      Mac_meas_req.Rep_interval = 2000;
      CH_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.Meas_entry = Mac_rlc_xface->mac_meas_req(Mod_id,&Mac_meas_req);
      CH_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.Meas_entry->Status=RADIO_CONFIG_OK;	
      CH_rrc_inst[Mod_id].Srb2_meas[UE_index].Status=NEED_RADIO_CONFIG;
      CH_rrc_inst[Mod_id].Srb2_meas[UE_index].Last_report_frame=Rrc_xface->Frame_index;
      CH_rrc_inst[Mod_id].Srb2_meas[UE_index].Mac_meas_req.Rep_interval=2000;
      CH_rrc_inst[Mod_id].Srb2_meas[UE_index].Mac_meas_req.Lchan_id.Index=Idx;
      CH_rrc_inst[Mod_id].Srb2_meas[UE_index].Next_check_frame=Rrc_xface->Frame_index+2000;
      
      //CONFIG RAB0 (DTCH_broadcast)
      Mac_config_req.Lchan_type = DTCH;
      Mac_config_req.UE_CH_index = UE_index;
      memcpy(&Mac_config_req.Lchan_desc[0],(LCHAN_DESC*)&DTCH_UL_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
      memcpy(&Mac_config_req.Lchan_desc[1],(LCHAN_DESC*)&DTCH_DL_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
      In_idx=find_free_dtch_position(Mod_id,UE_index);
      Mac_config_req.Lchan_id.Index=(UE_index << RAB_SHIFT2) + DTCH_BD + In_idx;
      Idx = Mac_rlc_xface->mac_config_req(Mod_id,ADD_LC,&Mac_config_req);
      CH_rrc_inst[Mod_id].Rab[In_idx][UE_index].Active = 1;
      CH_rrc_inst[Mod_id].Rab[In_idx][UE_index].Next_check_frame = Rrc_xface->Frame_index + 250;
      CH_rrc_inst[Mod_id].Rab[In_idx][UE_index].Status = NEED_RADIO_CONFIG;//RADIO CFG
      CH_rrc_inst[Mod_id].Rab[In_idx][UE_index].Rb_info.Rb_id = Idx;
      memcpy(&CH_rrc_inst[Mod_id].Rab[In_idx][UE_index].Rb_info.Lchan_desc[0],&DTCH_UL_LCHAN_DESC,LCHAN_DESC_SIZE);
      memcpy(&CH_rrc_inst[Mod_id].Rab[In_idx][UE_index].Rb_info.Lchan_desc[1],&DTCH_UL_LCHAN_DESC,LCHAN_DESC_SIZE);
      
      
      //Configure a correponding measurement process
      msg("Programing RADIO CONFIG of DTCH LCHAN %d\n",Idx); 
      Mac_rlc_xface->rrc_rlc_config_req(Mod_id,ACTION_ADD,Idx,RADIO_ACCESS_BEARER,Rlc_info_am_config);
      Mac_meas_req.Lchan_id.Index = Idx;
      Mac_meas_req.Meas_trigger = DTCH_MEAS_TRIGGER;
      Mac_meas_req.Mac_avg = DTCH_MEAS_AVG;
      Mac_meas_req.Rep_amount = 0;
      Mac_meas_req.Rep_interval = 2000;
      CH_rrc_inst[Mod_id].Rab[In_idx][UE_index].Rb_info.Meas_entry=Mac_rlc_xface->mac_meas_req(Mod_id,&Mac_meas_req);
      CH_rrc_inst[Mod_id].Rab[In_idx][UE_index].Rb_info.Meas_entry->Status=RADIO_CONFIG_OK;
      CH_rrc_inst[Mod_id].Rab_meas[In_idx][UE_index].Status=NEED_RADIO_CONFIG;  
      CH_rrc_inst[Mod_id].Rab_meas[In_idx][UE_index].Last_report_frame=Rrc_xface->Frame_index;
      CH_rrc_inst[Mod_id].Rab_meas[In_idx][UE_index].Mac_meas_req.Rep_interval=2000;
      CH_rrc_inst[Mod_id].Rab_meas[In_idx][UE_index].Mac_meas_req.Lchan_id.Index=Idx;
      CH_rrc_inst[Mod_id].Rab_meas[In_idx][UE_index].Next_check_frame=Rrc_xface->Frame_index+2000;   //CONFIG RAB0 (DTCH_broadcast)


      Mac_config_req.Lchan_type = DTCH;
      Mac_config_req.UE_CH_index = UE_index;
      memcpy(&Mac_config_req.Lchan_desc[0],(LCHAN_DESC*)&DTCH_UL_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
      memcpy(&Mac_config_req.Lchan_desc[1],(LCHAN_DESC*)&DTCH_DL_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
      In_idx=find_free_dtch_position(Mod_id,UE_index);
      Mac_config_req.Lchan_id.Index=(UE_index << RAB_SHIFT2) + DTCH_BD + In_idx;
      Idx = Mac_rlc_xface->mac_config_req(Mod_id,ADD_LC,&Mac_config_req);
      CH_rrc_inst[Mod_id].Rab[In_idx][UE_index].Active = 1;
      CH_rrc_inst[Mod_id].Rab[In_idx][UE_index].Next_check_frame = Rrc_xface->Frame_index + 250;
      CH_rrc_inst[Mod_id].Rab[In_idx][UE_index].Status = NEED_RADIO_CONFIG;//RADIO CFG
      CH_rrc_inst[Mod_id].Rab[In_idx][UE_index].Rb_info.Rb_id = Idx;
      memcpy(&CH_rrc_inst[Mod_id].Rab[In_idx][UE_index].Rb_info.Lchan_desc[0],&DTCH_UL_LCHAN_DESC,LCHAN_DESC_SIZE);
      memcpy(&CH_rrc_inst[Mod_id].Rab[In_idx][UE_index].Rb_info.Lchan_desc[1],&DTCH_UL_LCHAN_DESC,LCHAN_DESC_SIZE);

      
      //Configure a correponding measurement process
      msg("Programing RADIO CONFIG of DTCH LCHAN %d\n",Idx); 
      Mac_rlc_xface->rrc_rlc_config_req(Mod_id,ACTION_ADD,Idx,RADIO_ACCESS_BEARER,Rlc_info_am_config);
      Mac_meas_req.Lchan_id.Index = Idx;
      Mac_meas_req.Meas_trigger = DTCH_MEAS_TRIGGER;
      Mac_meas_req.Mac_avg = DTCH_MEAS_AVG;
      Mac_meas_req.Rep_amount = 0;
      Mac_meas_req.Rep_interval = 2000;
      CH_rrc_inst[Mod_id].Rab[In_idx][UE_index].Rb_info.Meas_entry=Mac_rlc_xface->mac_meas_req(Mod_id,&Mac_meas_req);
      CH_rrc_inst[Mod_id].Rab[In_idx][UE_index].Rb_info.Meas_entry->Status=RADIO_CONFIG_OK;
      CH_rrc_inst[Mod_id].Rab_meas[In_idx][UE_index].Status=NEED_RADIO_CONFIG;  
      CH_rrc_inst[Mod_id].Rab_meas[In_idx][UE_index].Last_report_frame=Rrc_xface->Frame_index;
      CH_rrc_inst[Mod_id].Rab_meas[In_idx][UE_index].Mac_meas_req.Rep_interval=2000;
      CH_rrc_inst[Mod_id].Rab_meas[In_idx][UE_index].Mac_meas_req.Lchan_id.Index=Idx;
      CH_rrc_inst[Mod_id].Rab_meas[In_idx][UE_index].Next_check_frame=Rrc_xface->Frame_index+2000;

      // Configure default measurement process 
      CH_rrc_inst[Mod_id].Def_meas[UE_index]= &CH_mac_inst[Mod_id].Def_meas[UE_index];
      CH_rrc_inst[Mod_id].Def_meas[UE_index]->Active = 1;
      CH_rrc_inst[Mod_id].Def_meas[UE_index]->Status = NEED_RADIO_CONFIG;
      CH_rrc_inst[Mod_id].Def_meas[UE_index]->Forg_fact=1;
      CH_rrc_inst[Mod_id].Def_meas[UE_index]->Rep_interval=50;
      CH_rrc_inst[Mod_id].Def_meas[UE_index]->Last_report_frame=Rrc_xface->Frame_index;
      CH_rrc_inst[Mod_id].Def_meas[UE_index]->Next_check_frame=Rrc_xface->Frame_index + 200;
      */

      ch_rrc_generate_RRCConnectionSetup(Mod_id,UE_index);


#endif //NO_RRM 
  }  
    
}



void ch_rrc_generate_RRCConnectionReconfiguration(u8 Mod_id,u16 UE_index) {

  u8 size=8;
  u8 RRCConnectionReconfiguration[size];
  u8 i;

  //  CH_rrc_inst[Mod_id].Srb1.Tx_buffer.W_idx=0;


  // Get RRCConnectionSetup message and size (here 9)
  //  CH_rrc_inst[Mod_id].Srb1.Tx_buffer.W_idx = size;
  msg("[RRC][eNB %d] Generate %d bytes (RRCConnectionReconfiguration) for DCCH : 1 ",Mod_id,size);//CH_rrc_inst[Mod_id].Srb1.Tx_buffer.W_idx);
  RRCConnectionReconfiguration[0]=1;
  for (i=1;i<size;i++) {
    RRCConnectionReconfiguration[i] = (u8)(taus()&0xff);
    msg("%x ",RRCConnectionReconfiguration[i]);
  }
  msg("\n");

  Mac_rlc_xface->rrc_rlc_data_req(Mod_id,DCCH,rrc_mui++,0,size,(char*)RRCConnectionReconfiguration);

  //  memcpy((void *)CH_rrc_inst[Mod_id].Srb1.Tx_buffer.Payload,
  //	 RRCConnectionSetup,9);

}

void ch_rrc_process_connectionsetupcomplete(u8 Mod_id, u8 UE_index, u8 *Rx_sdu, u8 sdu_size) {

  // process information

  // initiate RRCConnectionReconfiguration on SRB1
  ch_rrc_generate_RRCConnectionReconfiguration(Mod_id,UE_index);  

}

void ch_rrc_process_RRCConnectionReconfigurationComplete(Mod_id,UE_index,Rx_sdu,sdu_size){

    //Establish DRB (DTCH)
  msg("[RRC][eNB %d] Received RRCConnectionReconfigurationComplete from UE %d, configuring DRB\n",Mod_id,UE_index);
  Mac_rlc_xface->rrc_rlc_config_req(Mod_id,ACTION_ADD,
				    (UE_index * MAX_NUM_RB) + DTCH,
				    RADIO_ACCESS_BEARER,Rlc_info_um);

}

void ch_rrc_decode_dcch(u8 Mod_id, u8 UE_index, u8 *Rx_sdu, u8 sdu_size){  
  u8 sdu_type;

  msg("[RRC][eNB %d] Received message on dcch for UE_index %d, size %d bytes\n", Mod_id,UE_index, sdu_size);

  // check sdu_type
  switch (Rx_sdu[0]) {
  case 0 : // ConnectionSetupComplete
    msg("[RRC][eNB %d] Processing RRCConnectionSetupComplete message\n",Mod_id);
    ch_rrc_process_connectionsetupcomplete(Mod_id,UE_index,Rx_sdu,sdu_size);
    break;
  case 1 : // RRCConnectionReconfigurationComplete
    msg("[RRC][eNB %d] Processing RRCConnectionReconfigurationComplete message\n",Mod_id);
    ch_rrc_process_RRCConnectionReconfigurationComplete(Mod_id,UE_index,Rx_sdu,sdu_size);
    break;
  case 2 : // RRCConnectionRestablishmentComplete
  case 3:  // RRCSecurityModeComplete
  case 4:  // CSFBParametersRequestCDMA2000
  case 5:  // MeasurementReport
  case 6 : // SecurityModeFailure
  case 7 : // ueCapabilityInformation
  case 8 : // ulHandoverPreparationTransfer
  case 9 : // ulInformationTransfer
  case 10: // counterCheckResponse
  default:
    break;
  }

}
/*------------------------------------------------------------------------------*/
void ch_rrc_generate_bcch(u8 Mod_id){
  /*------------------------------------------------------------------------------*/
  //NOTHING TO DO YET ( segmentation)  
}      

void ch_rrc_generate_RRCConnectionSetup(u8 Mod_id,u16 UE_index) {

  u8 size=9;
  u8 RRCConnectionSetup[size];
  u8 i;

  CH_rrc_inst[Mod_id].Srb0.Tx_buffer.W_idx=0;


  // Get RRCConnectionSetup message and size (here 9)
  CH_rrc_inst[Mod_id].Srb0.Tx_buffer.W_idx = size;
  msg("[RRC][eNB %d] Generate %d bytes (RRCConnectionSetup for UE %d) for CCCH : 0 ",Mod_id,CH_rrc_inst[Mod_id].Srb0.Tx_buffer.W_idx,UE_index);
  RRCConnectionSetup[0]=0;
  for (i=1;i<size;i++) {
    RRCConnectionSetup[i] = (u8)(taus()&0xff);
    msg("%x ",RRCConnectionSetup[i]);
  }
  msg("\n");


  memcpy((void *)CH_rrc_inst[Mod_id].Srb0.Tx_buffer.Payload,
	 RRCConnectionSetup,9);

}

/*------------------------------------------------------------------------------------------*/
void ch_disconnect_ue(u8 Mod_id,u8 UE_index){
  /*------------------------------------------------------------------------------------------*/
  u8 i;
  MAC_CONFIG_REQ Mac_config_req;

  Mac_config_req.UE_CH_index=UE_index;
  msg("______________[RRC_XFACE] FRAME %d: CH %d,NODE %d(UE_index %d) OUT OF SYNC, DISCONNECT!______________\n ",Rrc_xface->Frame_index,CH_rrc_inst[Mod_id].Node_id,
      CH_rrc_inst[Mod_id].Info.UE_list[UE_index][0],
      CH_rrc_inst[Mod_id].Info.UE_list[UE_index][1],
      CH_rrc_inst[Mod_id].Info.UE_list[UE_index][2],
      CH_rrc_inst[Mod_id].Info.UE_list[UE_index][3],
      CH_rrc_inst[Mod_id].Info.UE_list[UE_index][4],
      UE_index);
  
  CH_rrc_inst[Mod_id].Info.Nb_ue--;

  //  CH_rrc_inst[Mod_id].Def_meas[UE_index]->Active = 0;
  //  CH_rrc_inst[Mod_id].Def_meas[UE_index]->Status = IDLE;
  //  CH_rrc_inst[Mod_id].Def_meas[UE_index]->Last_report_frame=0;
  //  CH_rrc_inst[Mod_id].Def_meas[UE_index]->Next_check_frame=0;

  memset(CH_rrc_inst[Mod_id].Info.UE_list[UE_index],0,5);
  CH_rrc_inst[Mod_id].Srb2[UE_index].Active = 0;
  CH_rrc_inst[Mod_id].Srb2[UE_index].Status = IDLE;//RADIO CFG
  CH_rrc_inst[Mod_id].Srb2[UE_index].Next_check_frame = 0;
  CH_rrc_inst[Mod_id].Srb2_meas[UE_index].Status = IDLE;
  CH_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.Meas_entry->Status = IDLE;
  //  CH_rrc_inst[Mod_id].Def_meas[UE_index]->Status = IDLE;
  Mac_rlc_xface->rrc_rlc_config_req(Mod_id,ACTION_REMOVE,CH_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.Srb_id,SIGNALLING_RADIO_BEARER,Rlc_info_um);
  Mac_config_req.Lchan_id.Index=CH_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.Srb_id;
  Mac_config_req.Lchan_type=DCCH;
  Mac_rlc_xface->mac_config_req(Mod_id,REMOVE_LC,&Mac_config_req);
  //     mac_release_req(Mod_id,CH_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.Srb_id,UE_index);
  //   CH_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.Meas_entry.Active=0;

  for (i=1;i<NB_RAB_MAX;i++)
    if(CH_rrc_inst[Mod_id].Rab[i][UE_index].Active==1){
      // msg("[RRC] RELEASE RB %d\n",Rrc_inst[Mod_id].Rab[i][UE_index].Rb_info.Rb_id);

      Mac_rlc_xface->rrc_rlc_config_req(Mod_id,ACTION_REMOVE,CH_rrc_inst[Mod_id].Rab[i][UE_index].Rb_info.Rb_id,RADIO_ACCESS_BEARER,Rlc_info_am_config);

      CH_rrc_inst[Mod_id].Rab[i][UE_index].Active=0;
      CH_rrc_inst[Mod_id].Rab[i][UE_index].Status=IDLE;
      CH_rrc_inst[Mod_id].Rab[i][UE_index].Next_check_frame = 0;
      CH_rrc_inst[Mod_id].Rab[i][UE_index].Rb_info.Meas_entry->Status=IDLE;
      CH_rrc_inst[Mod_id].Rab_meas[i][UE_index].Status=IDLE;
      Mac_config_req.Lchan_id.Index=CH_rrc_inst[Mod_id].Rab[i][UE_index].Rb_info.Rb_id;
      Mac_config_req.Lchan_type=DTCH;
      Mac_rlc_xface->mac_config_req(Mod_id,REMOVE_LC,&Mac_config_req);

      //mac_release_req(Mod_id,Rrc_inst[Mod_id].Rab[i][UE_index].Rb_info.Rb_id,UE_index);
      //H_rrc_inst[Mod_id].Rab[i][UE_index].Rb_info.Meas_entry.Active=0;
    }
    CH_rrc_inst[Mod_id].Info.Dtch_bd_config[UE_index].Status=IDLE;
    CH_rrc_inst[Mod_id].Info.Dtch_bd_config[UE_index].Next_check_frame=0;
  //Rrc_inst[Mod_id].Nb_rb[UE_index]=1;//DTCH BROADCAST
  
     
  
  // exit(0);
}
 
/*------------------------------------------------------------------------------------------*/
void  ue_rrc_decode_dcch(u8 Mod_id,char *Buffer,u8 CH_index){
  /*------------------------------------------------------------------------------------------*/

  // decode messages 
  switch (Buffer[0]) {
  case 1: // RRCConnectionReconfiguration

    //Establish DRB (DTCH)
    Mac_rlc_xface->rrc_rlc_config_req(Mod_id,ACTION_ADD,
				      DTCH,
				      RADIO_ACCESS_BEARER,Rlc_info_um);

    rrc_ue_generate_RRCConnectionReconfigurationComplete(Mod_id,CH_index);
    break;
  }
#ifndef NO_RRM
    send_msg(&S_rrc,msg_rrc_end_scan_req(Mod_id+NB_CH_INST,CH_index));
#endif
}

/*------------------------------------------------------------------------------------------*/
void  rrc_process_radio_meas(u8 Mod_id,MAC_MEAS_IND Mac_meas_ind,MAC_MEAS_REQ_ENTRY * Meas_entry){
  /*------------------------------------------------------------------------------------------*/

}


#ifndef USER_MODE
EXPORT_SYMBOL(Rlc_info_am_config);
#endif
