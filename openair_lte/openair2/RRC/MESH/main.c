/*________________________openair_rrc_main.c________________________
  
  Authors : Hicham Anouar
  Company : EURECOM
  Emails  : anouar@eurecom.fr
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
  msg("rrc_rx_tx: Mod_id %d (CH status %d)\n",Mod_id,Mac_rlc_xface->Is_cluster_head[Mod_id]);
#endif DEBUG_RRC

  Rrc_xface->Frame_index=Mac_rlc_xface->frame;
  if(Mac_rlc_xface->Is_cluster_head[Mod_id] == 1){
#ifdef DEBUG_RRC
    msg("[eNB RRC] : Generating bcch_header\n");
#endif
    ch_rrc_generate_bcch_header(Mod_id);
  }
  else{
    for(i=0;i<NB_SIG_CNX_UE;i++) 
      ue_rrc_decode_bcch_header(Mod_id-NB_CH_INST,i);  
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
  DCCH_LCHAN_DESC.transport_block_size=30;
  DCCH_LCHAN_DESC.max_transport_blocks=20;
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
  Rlc_info_am_config.rlc.rlc_am_info.pdu_size              = 416; // in bits
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
    CH_rrc_inst[Mod_id].Info.UE_list[j].L2_id[0]=0xff;	
    CH_rrc_inst[Mod_id].Nb_rb[j]=0;
  }
  
  CH_rrc_inst[Mod_id].Srb0.Active=0;
  CH_rrc_inst[Mod_id].Srb1.Active=0;
  
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
  
  CH_rrc_inst[Mod_id].Def_meas[0]= &CH_mac_inst[Mod_id].Def_meas[0];
  CH_rrc_inst[Mod_id].Def_meas[0]->Status = RADIO_CONFIG_OK;
  CH_rrc_inst[Mod_id].Def_meas[0]->Forg_fact=1;
  CH_rrc_inst[Mod_id].Def_meas[0]->Rep_interval=50;
  CH_rrc_inst[Mod_id].Def_meas[0]->Last_report_frame=Rrc_xface->Frame_index;
    
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
  MAC_CONFIG_REQ Mac_config_req;
  MAC_MEAS_REQ Mac_meas_req;  

  msg("OPENAIR RRC IN....\n");
  if( Mac_rlc_xface->Is_cluster_head[Mod_id] == 1){
    
    for(i=0;i<NB_SIG_CNX_CH;i++){  
      
      Mac_config_req.Lchan_type = BCCH;
      memcpy(&Mac_config_req.Lchan_desc[0],(LCHAN_DESC*)&BCCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
      memcpy(&Mac_config_req.Lchan_desc[1],(LCHAN_DESC*)&BCCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
      Mac_config_req.UE_CH_index=i;
      Mac_config_req.Lchan_id.Index=(i << RAB_SHIFT2) + BCCH;
      msg("Calling Lchan_config\n");
      Index=Mac_rlc_xface->mac_config_req(Mod_id,ADD_LC,&Mac_config_req);
      msg("[OPENAIR][RRC][RRC_ON] NODE %d, Config BCCH %d done\n",CH_rrc_inst[Mod_id].Node_id,Index);
      CH_rrc_inst[Mod_id].Srb0.Srb_id = Index;
      memcpy(&CH_rrc_inst[Mod_id].Srb0.Lchan_desc[0],&BCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
      memcpy(&CH_rrc_inst[Mod_id].Srb0.Lchan_desc[1],&BCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
      rrc_config_buffer(&CH_rrc_inst[Mod_id].Srb0,BCCH,0);
      //      ((CH_BCCH_HEADER*)(&CH_rrc_inst[Mod_id].Srb0.Tx_buffer.Header[0]))->Rv_tb_idx=0;
      msg("[OPENAIR][RRC][RRC_ON] NODE %d, Config BCCH for TB_size %d\n",NODE_ID[Mod_id],
	     CH_rrc_inst[Mod_id].Srb0.Lchan_desc[1].transport_block_size);
      CH_rrc_inst[Mod_id].Srb0.Active=1;
      CH_rrc_inst[Mod_id].Srb0.Tx_buffer.generate_fun=ch_rrc_generate_bcch;
  
      Mac_config_req.Lchan_type = CCCH;
      memcpy(&Mac_config_req.Lchan_desc[0],(LCHAN_DESC*)&CCCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
      memcpy(&Mac_config_req.Lchan_desc[1],(LCHAN_DESC*)&CCCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
      Mac_config_req.UE_CH_index=i;
      Mac_config_req.Lchan_id.Index=(i << RAB_SHIFT2) + CCCH;
      Index=Mac_rlc_xface->mac_config_req(Mod_id,ADD_LC,&Mac_config_req);
      //msg("[OPENAIR][RRC][RRC_ON] NODE %d, Config BCCH %d done\n",NODE_ID[Mod_id],Index);
      CH_rrc_inst[Mod_id].Srb1.Srb_id = Index;
      memcpy(&CH_rrc_inst[Mod_id].Srb1.Lchan_desc[0],&CCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
      memcpy(&CH_rrc_inst[Mod_id].Srb1.Lchan_desc[1],&CCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
      rrc_config_buffer(&CH_rrc_inst[Mod_id].Srb1,CCCH,1);
      ((CH_CCCH_HEADER*)(&CH_rrc_inst[Mod_id].Srb0.Tx_buffer.Header[0]))->Rv_tb_idx=0;
      msg("[OPENAIR][RRC][RRC_ON] NODE %d, Config CCCH %d done, TB_size=%d,%d\n",NODE_ID[Mod_id],Index,
	     CH_rrc_inst[Mod_id].Srb1.Tx_buffer.Tb_size,CH_rrc_inst[Mod_id].Srb1.Rx_buffer.Tb_size);
      CH_rrc_inst[Mod_id].Srb1.Active=1;
      CH_rrc_inst[Mod_id].Srb1.Tx_buffer.generate_fun=ch_rrc_generate_ccch;
  
      Mac_config_req.Lchan_type = DTCH_BD;//DEFAULT BROADCAST DTCH for IP SIGNALLING// (only tx, no orrsponding RABs for rx) 
      Mac_config_req.UE_CH_index = 0;
      Mac_config_req.Lchan_id.Index=(i << RAB_SHIFT2)+DTCH_BD ;
      memcpy(&Mac_config_req.Lchan_desc[0],(LCHAN_DESC*)&DTCH_UL_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
      memcpy(&Mac_config_req.Lchan_desc[1],(LCHAN_DESC*)&DTCH_DL_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
      Index = Mac_rlc_xface->mac_config_req(Mod_id,ADD_LC,&Mac_config_req);
      msg("[OPENAIR][RRC][RRC_ON] NODE %d, Config DTCH BROADCAST %d done\n",NODE_ID[Mod_id],Index);
      CH_rrc_inst[Mod_id].Rab[0][i].Active = 1;
      CH_rrc_inst[Mod_id].Rab[0][i].Rb_info.Rb_id = Index;//(i << RAB_SHIFT)+DTCH ;
      memcpy(&CH_rrc_inst[Mod_id].Rab[0][i].Rb_info.Lchan_desc[0],&DTCH_UL_LCHAN_DESC,LCHAN_DESC_SIZE);
      memcpy(&CH_rrc_inst[Mod_id].Rab[0][i].Rb_info.Lchan_desc[1],&DTCH_UL_LCHAN_DESC,LCHAN_DESC_SIZE);
      CH_rrc_inst[Mod_id].Rab[0][i].Status=RADIO_CONFIG_OK;
      msg("[OPENAIR][RRC] CALLING RLC CONFIG RADIO BEARER %d\n",Index);
      Mac_rlc_xface->rrc_rlc_config_req(Mod_id,ACTION_ADD,Index,RADIO_ACCESS_BEARER,Rlc_info_um);
      
      CH_rrc_inst[Mod_id].Info.UE_list[i].L2_id[0]=i;	
    }
  }
  
  else{
  
    Mod_id-=NB_CH_INST;
    for(i=0;i<NB_SIG_CNX_UE;i++){  
      
      Mac_config_req.Lchan_type = BCCH;
      memcpy(&Mac_config_req.Lchan_desc[0],(LCHAN_DESC*)&BCCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
      memcpy(&Mac_config_req.Lchan_desc[1],(LCHAN_DESC*)&BCCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
      Mac_config_req.UE_CH_index=i;
      Mac_config_req.Lchan_id.Index=(i << RAB_SHIFT2) + BCCH;
      Index=Mac_rlc_xface->mac_config_req(Mod_id+NB_CH_INST,ADD_LC,&Mac_config_req);
      UE_rrc_inst[Mod_id].Srb0[i].Srb_id = Index;
      memcpy(&UE_rrc_inst[Mod_id].Srb0[i].Lchan_desc[0],&BCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
      memcpy(&UE_rrc_inst[Mod_id].Srb0[i].Lchan_desc[1],&BCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
      rrc_config_buffer(&UE_rrc_inst[Mod_id].Srb0[i],BCCH,0);
      //      ((CH_BCCH_HEADER*)(&UE_rrc_inst[Mod_id].Srb0[i].Rx_buffer.Header[0]))->Rv_tb_idx=0;
      UE_rrc_inst[Mod_id].Srb0[i].Active=1;

      Mac_meas_req.Lchan_id.Index = Index;
      Mac_meas_req.UE_CH_index = i;
      Mac_meas_req.Meas_trigger = BCCH_MEAS_TRIGGER;
      Mac_meas_req.Mac_avg = BCCH_MEAS_AVG;
      Mac_meas_req.Rep_amount = 0;
      Mac_meas_req.Rep_interval = 1000;
      UE_rrc_inst[Mod_id].Srb0[i].Meas_entry=Mac_rlc_xface->mac_meas_req(Mod_id+NB_CH_INST,&Mac_meas_req);
      UE_rrc_inst[Mod_id].Srb0[i].Meas_entry->Status=RADIO_CONFIG_OK;
      UE_rrc_inst[Mod_id].Srb0[i].Meas_entry->Last_report_frame=Rrc_xface->Frame_index;
      UE_rrc_inst[Mod_id].Srb0[i].Meas_entry->Next_check_frame=Rrc_xface->Frame_index+1000;
      
      Mac_config_req.Lchan_type = CCCH;
      memcpy(&Mac_config_req.Lchan_desc[0],(LCHAN_DESC*)&CCCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
      memcpy(&Mac_config_req.Lchan_desc[1],(LCHAN_DESC*)&CCCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
      Mac_config_req.UE_CH_index=i;
      Mac_config_req.Lchan_id.Index=(i << RAB_SHIFT2) + CCCH;
      Index=Mac_rlc_xface->mac_config_req(Mod_id+NB_CH_INST,ADD_LC,&Mac_config_req);
      UE_rrc_inst[Mod_id].Srb1[i].Srb_id = Index;
      memcpy(&UE_rrc_inst[Mod_id].Srb1[i].Lchan_desc[0],&CCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
      memcpy(&UE_rrc_inst[Mod_id].Srb1[i].Lchan_desc[1],&CCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
      rrc_config_buffer(&UE_rrc_inst[Mod_id].Srb1[i],CCCH,1);
      ((CH_CCCH_HEADER*)(&UE_rrc_inst[Mod_id].Srb1[i].Rx_buffer.Header[0]))->Rv_tb_idx=0;
      UE_rrc_inst[Mod_id].Srb1[i].Active=1;
      
      Mac_meas_req.Lchan_id.Index = Index;
      Mac_meas_req.Meas_trigger = CCCH_MEAS_TRIGGER;
      Mac_meas_req.Mac_avg = CCCH_MEAS_AVG;
      Mac_meas_req.Rep_amount = 0;
      Mac_meas_req.Rep_interval = 1000;
      UE_rrc_inst[Mod_id].Srb1[i].Meas_entry=Mac_rlc_xface->mac_meas_req(Mod_id+NB_CH_INST,&Mac_meas_req);
      UE_rrc_inst[Mod_id].Srb1[i].Meas_entry->Status=RADIO_CONFIG_OK;
      UE_rrc_inst[Mod_id].Srb1[i].Meas_entry->Last_report_frame=Rrc_xface->Frame_index;
      UE_rrc_inst[Mod_id].Srb1[i].Meas_entry->Next_check_frame=Rrc_xface->Frame_index+1000;
    }
  }
}
#endif //NO_RRM

/*------------------------------------------------------------------------------*/
void ch_rrc_generate_bcch_header(u8 Mod_id){
  /*------------------------------------------------------------------------------*/
 
  unsigned char k;
  
 
  if(CH_rrc_inst[Mod_id].Srb0.Active==0)
    return;
 

  ((CH_BCCH_HEADER*)(CH_rrc_inst[Mod_id].Srb0.Tx_buffer.Header))->CH_id = CH_rrc_inst[Mod_id].Node_id;
  
  for(k=0;k<NB_UE_BRDCAST;k++)
    ((CH_BCCH_HEADER*)CH_rrc_inst[Mod_id].Srb0.Tx_buffer.Header)->UE_list[k]=CH_rrc_inst[Mod_id].Info.UE_list[k].L2_id[0];
  
  //  ((CH_BCCH_HEADER*)(CH_rrc_inst[Mod_id].Srb0.Tx_buffer.Header))->Rach_time_alloc = RACH_TIME_ALLOC;
  //  ((CH_BCCH_HEADER*)(CH_rrc_inst[Mod_id].Srb0.Tx_buffer.Header))->Rach_freq_alloc = RACH_FREQ_ALLOC;
  CH_rrc_inst[Mod_id].Srb0.Header_tx=1;
  CH_rrc_inst[Mod_id].Srb0.Tx_buffer.W_idx=CH_BCCH_HEADER_SIZE;
}

/*------------------------------------------------------------------------------*/
void ue_rrc_decode_bcch(u8 Mod_id){
  /*------------------------------------------------------------------------------*/
}

/*------------------------------------------------------------------------------*/
void rrc_mac_association_req_tx(u8 Mod_id, unsigned char Idx){ 
  /*------------------------------------------------------------------------------*/

  u8 i=0;
  UE_rrc_inst[Mod_id].Rrc_dummy_pdu[i++] = RRC_RACH_ASS_REQ;
  memcpy(&UE_rrc_inst[Mod_id].Rrc_dummy_pdu[i],(char*)&UE_rrc_inst[Mod_id].Mac_id.L2_id[0],sizeof(L2_ID));
  i+=sizeof(L2_ID);
  UE_rrc_inst[Mod_id].Rrc_dummy_pdu[i++] = UE_rrc_inst[Mod_id].Info[Idx].CH_id;
  //  UE_rrc_inst[Mod_id].Rrc_dummy_pdu[i++] = UE_rrc_inst[Mod_id].Info[Idx].Rach_time_alloc;
  //  UE_rrc_inst[Mod_id].Rrc_dummy_pdu[i++] = UE_rrc_inst[Mod_id].Info[Idx].Rach_freq_alloc;
  if(UE_rrc_inst[Mod_id].Srb1[Idx].Tx_buffer.W_idx ==0){
    memcpy(&UE_rrc_inst[Mod_id].Srb1[Idx].Tx_buffer.Payload[0],UE_rrc_inst[Mod_id].Rrc_dummy_pdu,i);
    UE_rrc_inst[Mod_id].Srb1[Idx].Tx_buffer.W_idx =i;
   
    /*    msg("[OPENAIR][RRC]NODE %d, UE: Association Req TX from L2_id %d to CH_index %d (%d), W_idx %d done\n",
	NODE_ID[Mod_id+NB_CH_INST],
	UE_rrc_inst[Mod_id].Mac_id.L2_id[0],
	UE_rrc_inst[Mod_id].Info[Idx].CH_id,  
	Idx,
	UE_rrc_inst[Mod_id].Srb1[Idx].Tx_buffer.W_idx);
    */
  }
}

/*------------------------------------------------------------------------------*/
void ue_rrc_decode_bcch_header(u8 Mod_id , u8 Idx){
  /*------------------------------------------------------------------------------*/
  u8 i;
  CH_BCCH_HEADER *Header,Header1;

 #ifdef DEBUG_MAC_XFACE
  msg("[OPENAIR][RRC] NODE %d, FRAME %d: UE: CH_INDEX %d: Status=%d, Header Rx=%d, NB_MISS %d: Decode bcch ...\n",
      NODE_ID[Mod_id+NB_CH_INST],
      Rrc_xface->Frame_index,
      Idx,
      UE_rrc_inst[Mod_id].Info[Idx].Status,
      UE_rrc_inst[Mod_id].Srb0[Idx].Header_rx,
      UE_rrc_inst[Mod_id].Info[Idx].Nb_bcch_miss);
#endif
  
  if(UE_rrc_inst[Mod_id].Srb0[Idx].Active==0) return;
  switch (UE_rrc_inst[Mod_id].Info[Idx].Status){    
    
  case RRC_IDLE:
    if( UE_rrc_inst[Mod_id].Srb0[Idx].Header_rx !=0 ){
      memcpy(&Header1,((CH_BCCH_HEADER*) (UE_rrc_inst[Mod_id].Srb0[Idx].Rx_buffer.Header)),CH_BCCH_HEADER_SIZE);
      //      UE_rrc_inst[Mod_id].Info[Idx].Nb_rach_res     = Header1.Nb_rach_res;
      //      UE_rrc_inst[Mod_id].Info[Idx].Rach_time_alloc = Header1.Rach_time_alloc;
      //      UE_rrc_inst[Mod_id].Info[Idx].Rach_freq_alloc = Header1.Rach_freq_alloc;
      UE_rrc_inst[Mod_id].Info[Idx].Status = RRC_PRE_SYNCHRO;
      UE_rrc_inst[Mod_id].Info[Idx].CH_id = Header1.CH_id;
      Rrc_xface->CH_id[Mod_id][Idx]= Header1.CH_id;
      UE_rrc_inst[Mod_id].Info[Idx].Rach_tx_cnt = 0;
      UE_rrc_inst[Mod_id].Info[Idx].Nb_bcch_miss=0;

#ifndef NO_RRM
      //    if(Mac_rlc_xface->Is_cluster_head[Mod_id+NB_CH_INST]==0)
      //send_msg(&S_rrc,msg_rrc_phy_synch_to_CH_ind(Mod_id+NB_CH_INST,Idx,UE_rrc_inst[Mod_id].Mac_id));
#endif

      msg("[OPANIR][RRC] NODE %d, PRE_SYNCHRONIZED to CH %d(%d) (CH_index =%d) \n",
	  NODE_ID[Mod_id+NB_CH_INST],
	  Header1.CH_id,
	  UE_rrc_inst[Mod_id].Info[Idx].CH_id,
	  Idx);
      
      rrc_mac_association_req_tx(Mod_id,Idx);
    }
    break;
    
  case RRC_PRE_SYNCHRO:
    if( UE_rrc_inst[Mod_id].Srb0[Idx].Header_rx !=0 ){
      Header=(CH_BCCH_HEADER*)(&UE_rrc_inst[Mod_id].Srb0[Idx].Rx_buffer.Header[0]);
      for( i=0;i<NB_UE_BRDCAST;i++){ //LOOK IF PRE_ASSOCIATED //(CH TX ASSOCIATION_RESP())
	if(UE_rrc_inst[Mod_id].Node_id == Header->UE_list[i]){
	  UE_rrc_inst[Mod_id].Info[Idx].UE_index=i;
	  Rrc_xface->UE_index[Mod_id+NB_CH_INST][Idx]=i;
	  msg("[OPENAIR][RRC] Frame %d: NODE %d: PRE_ASSOCIATED to CH %d with index %d\n",
	      Rrc_xface->Frame_index,
	      NODE_ID[Mod_id+NB_CH_INST],
	      UE_rrc_inst[Mod_id].Info[Idx].CH_id,
	      UE_rrc_inst[Mod_id].Info[Idx].UE_index);
	  UE_rrc_inst[Mod_id].Info[Idx].Status = RRC_PRE_ASSOCIATED;
	  UE_rrc_inst[Mod_id].Info[Idx].Nb_bcch_miss=0;
	  break;
	}
      }
      if( UE_rrc_inst[Mod_id].Info[Idx].Status != RRC_PRE_ASSOCIATED) {
	if(++UE_rrc_inst[Mod_id].Info[Idx].Rach_tx_cnt % (UE_rrc_inst[Mod_id].Node_id) ==0){
	  rrc_mac_association_req_tx(Mod_id,Idx);
	}
      }
    }
    break;
    
  case RRC_PRE_ASSOCIATED:
    if( UE_rrc_inst[Mod_id].Srb0[Idx].Header_rx !=0 ){
      Header=(CH_BCCH_HEADER*)UE_rrc_inst[Mod_id].Srb0[Idx].Rx_buffer.Header;
      for( i=0 ; i<NB_UE_BRDCAST ; i++ ) 
	if(UE_rrc_inst[Mod_id].Node_id == Header->UE_list[i])   //Check if am still Pre_associated 
	  break;
      if( i == NB_UE_BRDCAST ){
	
	UE_rrc_inst[Mod_id].Info[Idx].Nb_bcch_miss++;
	if(UE_rrc_inst[Mod_id].Info[Idx].Nb_bcch_miss == MAX_ALLOWED_BCCH_MISS){
	  msg("[OPANIR][RRC] Frame %d: MR %d: NO_MORE PRE_ASSOCIATED with CH %d, Retry!!! \n",Rrc_xface->Frame_index,
	      NODE_ID[Mod_id+NB_CH_INST],UE_rrc_inst[Mod_id].Info[Idx].CH_id);
	  UE_rrc_inst[Mod_id].Info[Idx].Status = RRC_PRE_SYNCHRO;
	  rrc_mesh_out_of_sync_ind(Mod_id+NB_CH_INST, Idx);
	  if(UE_rrc_inst[Mod_id].Info[Idx].Rach_tx_cnt % (UE_rrc_inst[Mod_id].Node_id) ==0)
	    rrc_mac_association_req_tx(Mod_id,Idx);
	}
      }
      else UE_rrc_inst[Mod_id].Info[Idx].Nb_bcch_miss=0;
    }
    break;
  
  case RRC_ASSOCIATED:
    if(UE_rrc_inst[Mod_id].Srb0[Idx].Header_rx !=0 ){
      
      Header=(CH_BCCH_HEADER*)UE_rrc_inst[Mod_id].Srb0[Idx].Rx_buffer.Header;
      for( i=0 ; i<NB_UE_BRDCAST ; i++ ){ 
	if( UE_rrc_inst[Mod_id].Node_id == Header->UE_list[i])   //Check if am still Pre_associated 
	  break;
      }
      
      if( i == NB_UE_BRDCAST ){
	UE_rrc_inst[Mod_id].Info[Idx].Nb_bcch_miss++;
	if(UE_rrc_inst[Mod_id].Info[Idx].Nb_bcch_miss == MAX_ALLOWED_BCCH_MISS){
	  
	  msg("[OPANIR][RRC] Frame %d: MR %d: NO_MORE ASSOCIATED with CH %d, Retry!!! \n",Rrc_xface->Frame_index,
	      NODE_ID[Mod_id+NB_CH_INST],UE_rrc_inst[Mod_id].Info[Idx].CH_id);
	  UE_rrc_inst[Mod_id].Info[Idx].Status = RRC_PRE_SYNCHRO;
	  rrc_mesh_out_of_sync_ind(Mod_id+NB_CH_INST, Idx);
	  if(UE_rrc_inst[Mod_id].Info[Idx].Rach_tx_cnt % (UE_rrc_inst[Mod_id].Node_id) ==0)
	    rrc_mac_association_req_tx(Mod_id,Idx);
	}
      }
      else
	UE_rrc_inst[Mod_id].Info[Idx].Nb_bcch_miss=0;
    }
    break;

  case RRC_CONNECTED:
    if( UE_rrc_inst[Mod_id].Srb0[Idx].Header_rx !=0 ){
      Header=(CH_BCCH_HEADER*)UE_rrc_inst[Mod_id].Srb0[Idx].Rx_buffer.Header;
      for( i=0 ; i<NB_UE_BRDCAST ; i++ ){ 
	if( UE_rrc_inst[Mod_id].Node_id == Header->UE_list[i])   //Check if am still Pre_associated 
	  break;
      }
      
      if( i == NB_UE_BRDCAST ){
	UE_rrc_inst[Mod_id].Info[Idx].Nb_bcch_miss++;
	if(UE_rrc_inst[Mod_id].Info[Idx].Nb_bcch_miss == MAX_ALLOWED_BCCH_MISS){	
	  msg("[OPANIR][RRC] Frame %d: MR %d: NO_MORE CONNECTED with CH %d, NB_MISS %d: Retry!!! \n",Rrc_xface->Frame_index,
	      NODE_ID[Mod_id+NB_CH_INST],UE_rrc_inst[Mod_id].Info[Idx].CH_id,UE_rrc_inst[Mod_id].Info[Idx].Nb_bcch_miss);
	  UE_rrc_inst[Mod_id].Info[Idx].Status = RRC_PRE_SYNCHRO;
	  rrc_mesh_out_of_sync_ind(Mod_id+NB_CH_INST, Idx);
	  if(UE_rrc_inst[Mod_id].Info[Idx].Rach_tx_cnt % (UE_rrc_inst[Mod_id].Node_id) ==0)
	    rrc_mac_association_req_tx(Mod_id,Idx);
	}             
      }
      else UE_rrc_inst[Mod_id].Info[Idx].Nb_bcch_miss=0;   
    }
    break;
  default:
    msg("[OPENAIR][RRC] IN DECODE_BCCH(): MODULE_ID %d: FATAL ERROR: UNKOWN RRC STATE %d\n",Mod_id,UE_rrc_inst[Mod_id].Info[Idx].Status);
    Mac_rlc_xface->macphy_exit("");
  }
  UE_rrc_inst[Mod_id].Srb0[Idx].Header_rx=0;
}

/*------------------------------------------------------------------------------*/
void ue_rrc_decode_ccch(u8 Mod_id, SRB_INFO *Srb_info, u8 CH_index){
  /*------------------------------------------------------------------------------*/
  msg("UE DECODE CCCH\n");
  unsigned char ret;
  switch (UE_rrc_inst[Mod_id].Info[CH_index].Status){    
  case RRC_PRE_ASSOCIATED:    
    ret=rrc_read_ccch_config_req(Mod_id,Srb_info,CH_index);
    if( ret == 1 ){
      UE_rrc_inst[Mod_id].Info[CH_index].Status = RRC_ASSOCIATED;
      msg("[OPANIR][RRC] Frame %d: NODE %d: ASSOCIATED to CH %d \n",Rrc_xface->Frame_index,
	  UE_rrc_inst[Mod_id].Node_id,UE_rrc_inst[Mod_id].Info[CH_index].CH_id);    	
    }
    else if( ret == 2 ){
      UE_rrc_inst[Mod_id].Info[CH_index].Status = RRC_CONNECTED;
      msg("_____________[OPANIR][RRC] Frame %d: NODE %d CONNECTED to CH %d with index %d______________ \n",
	  Rrc_xface->Frame_index, UE_rrc_inst[Mod_id].Node_id,UE_rrc_inst[Mod_id].Info[CH_index].CH_id,
	  UE_rrc_inst[Mod_id].Info[CH_index].UE_index);    	
    }
    break;
  case RRC_ASSOCIATED:
    if( (rrc_read_ccch_config_req(Mod_id,Srb_info,CH_index)) == 2 ){
      UE_rrc_inst[Mod_id].Info[CH_index].Status=RRC_CONNECTED;   
      msg("______________[OPANIR][RRC] Frame %d: NODE %d CONNECTED to CH %d with index %d _____________ \n",
	  Rrc_xface->Frame_index, UE_rrc_inst[Mod_id].Node_id,UE_rrc_inst[Mod_id].Info[CH_index].CH_id,
	  UE_rrc_inst[Mod_id].Info[CH_index].UE_index);    	
    }
    break;
  case RRC_CONNECTED:
    rrc_read_ccch_config_req(Mod_id,Srb_info,CH_index);
    break;
  default:
    msg("[OPENAIR][RRC] IN DECODE_CCCH(): Module_id %d, COULD NOT DECODE MESSAGE YET %d\n",Mod_id,
	UE_rrc_inst[Mod_id].Info[CH_index].Status);
  }
}

/*------------------------------------------------------------------------------*/
unsigned char rrc_read_ccch_config_req(u8 Mod_id, SRB_INFO *Srb_info, u8 CH_index){
  /*------------------------------------------------------------------------------*/

  MAC_CONFIG_REQ Mac_config_req;
  MAC_MEAS_REQ Mac_meas_req;
  unsigned char i,ret=0;
  unsigned int R_idx=0;
  #ifndef NO_RRM
  rrm_init_scan_req_t Rrm_init_scan_req;
  #endif
  u16 UE_index= UE_rrc_inst[Mod_id].Info[CH_index].UE_index, Index,UE_index12[2],Idx1,In_idx;
  CH_CCCH_HEADER *Header=(CH_CCCH_HEADER *)(&Srb_info->Rx_buffer.Header[0]);
  
  //   msg("[OPENAIR][RRC]___________________NODE %d: frame %d: READ CCCH_CONFIG from CH %d: Nb_lc_cfg %d, Nb_ms_cfg %d _____________\n",
  //  NODE_ID[Mod_id+NB_CH_INST],Rrc_xface->Frame_index,CH_index,Header->Nb_cfg_req,Header->Nb_meas_req);
  
  msg("NB_SENS_REQ %d, NB_CFG_REQ %d\n",Header->Nb_sens_req,Header->Nb_cfg_req);
 #ifndef NO_RRM
  for( i=0 ; i < Header->Nb_sens_req ; i++ ){//Look for any MAC config_request
    memcpy(&Rrm_init_scan_req,(rrm_init_scan_req_t*)&Srb_info->Rx_buffer.Payload[R_idx],sizeof(rrm_init_scan_req_t));
    msg("UE inst %d, RX init_scna_req FROM FC %d, sending to RRM\n",Mod_id,CH_index);
    send_msg(&S_rrc,msg_rrc_init_scan_req(Mod_id+NB_CH_INST,CH_rrc_inst[0].Mac_id,Rrm_init_scan_req.interv));
    R_idx+=sizeof(rrm_init_scan_req_t);
  }
#endif
  for( i=0 ; i < Header->Nb_cfg_req ; i++ ){//Look for any MAC config_request
    memcpy(&Mac_config_req,(MAC_CONFIG_REQ*)&Srb_info->Rx_buffer.Payload[R_idx],MAC_CONFIG_REQ_SIZE);
    UE_index12[0]=( ( Mac_config_req.Lchan_id.Index & RAB_OFFSET1) >> RAB_SHIFT1 );
    UE_index12[1]=( ( Mac_config_req.Lchan_id.Index & RAB_OFFSET2 ) >> RAB_SHIFT2 );
    Idx1=0xffff;
    In_idx = (Mac_config_req.Lchan_id.Index & RAB_OFFSET);
    if(UE_index12[0] == UE_index) Idx1=UE_index12[1];
    if(UE_index12[1] == UE_index) Idx1=UE_index12[0];
    if( Idx1 !=0xffff){
      msg("[RRC][ UE %d]TTI %d: GET LCHAN_CONFIG Command %d for LCHAN_ID %d (tb_size RX %d, tb_size TX %d), CH_index %d\n",
	  NODE_ID[Mod_id+NB_CH_INST],
	  Rrc_xface->Frame_index,
	  i,
	  Mac_config_req.Lchan_id.Index,
	  Mac_config_req.Lchan_desc[0].transport_block_size,
	  Mac_config_req.Lchan_desc[1].transport_block_size,
	  Idx1);
      Mac_config_req.UE_CH_index=CH_index;
      if(Mac_config_req.Lchan_type == DCCH){
	Mac_config_req.Lchan_id.Index = (CH_index << RAB_SHIFT2) + DCCH;
	Index=Mac_rlc_xface->mac_config_req(Mod_id+NB_CH_INST,ADD_LC,&Mac_config_req);
	if(Index){
	  UE_rrc_inst[Mod_id].Srb2[CH_index].Active = 1;
	  UE_rrc_inst[Mod_id].Srb2[CH_index].Status = RADIO_CONFIG_OK;//RADIO CFG
	  UE_rrc_inst[Mod_id].Srb2[CH_index].Srb_info.Srb_id = Index;
	  
	  memcpy(&UE_rrc_inst[Mod_id].Srb2[CH_index].Srb_info.Lchan_desc[0],(LCHAN_DESC*)&Mac_config_req.Lchan_desc[0],LCHAN_DESC_SIZE);
	  memcpy(&UE_rrc_inst[Mod_id].Srb2[CH_index].Srb_info.Lchan_desc[2],(LCHAN_DESC*)&Mac_config_req.Lchan_desc[2],LCHAN_DESC_SIZE);
	  
	  ret=1;
	  msg("[RRC][UE %d], UE_index %d,CONFIG_SRB2 %d(%d) corresponding to CH_index %d\n",
	      UE_rrc_inst[Mod_id].Node_id,
	      UE_index,
	      Index,
	      Mac_config_req.Lchan_id.Index,
	      CH_index);
	  Mac_rlc_xface->rrc_rlc_config_req(Mod_id+NB_CH_INST,ACTION_ADD,Index,SIGNALLING_RADIO_BEARER,Rlc_info_um);
	  UE_rrc_inst[Mod_id].Srb2[CH_index].Srb_info.Tx_buffer.W_idx=DEFAULT_MEAS_IND_SIZE+1;
	  UE_rrc_inst[Mod_id].Srb2[CH_index].Srb_info.Tx_buffer.R_idx=0;
	  
	  if(Mac_config_req.L3_info_type == MAC_ADDR){
	    memcpy(&UE_rrc_inst[Mod_id].Info[CH_index].CH_mac_id,(L2_ID*)Mac_config_req.L3_info,sizeof(L2_ID));
	    
	    
	   //CONFIG of DTCH_BD & Defaut IP signaling RAB must be done before DCCH
#ifndef NO_RRM 
	    send_msg(&S_rrc, msg_rrc_cx_establish_ind(Mod_id+NB_CH_INST,
						      UE_rrc_inst[Mod_id].Info[CH_index].CH_mac_id,
						      UE_rrc_inst[Mod_id].Srb2[CH_index].Srb_info.Trans_id++,
						      UE_rrc_inst[Mod_id].Srb2[CH_index].Srb_info.IP_addr,
						      UE_rrc_inst[Mod_id].Srb2[CH_index].Srb_info.IP_addr_type,
						      UE_rrc_inst[Mod_id].Rab[0][CH_index].Rb_info.Rb_id,
						      UE_rrc_inst[Mod_id].Rab[1][CH_index].Rb_info.Rb_id));
#endif
	    
	  }
	}
      }
      else if( (Mac_config_req.Lchan_type != DTCH_DIL) && (Mac_config_req.Lchan_type > CCCH) ){
	Mac_config_req.Lchan_id.Index = (CH_index << RAB_SHIFT2) +  In_idx;
	Index=Mac_rlc_xface->mac_config_req(Mod_id+NB_CH_INST,ADD_LC,&Mac_config_req);
	if(Index!=0){
	  UE_rrc_inst[Mod_id].Rab[In_idx-DTCH_BD][CH_index].Active = 1;
	  UE_rrc_inst[Mod_id].Rab[In_idx-DTCH_BD][CH_index].Status = RADIO_CONFIG_OK;
	  UE_rrc_inst[Mod_id].Rab[In_idx-DTCH_BD][CH_index].Rb_info.Rb_id =Mac_config_req.Lchan_id.Index ;

	  memcpy(&UE_rrc_inst[Mod_id].Rab[In_idx-DTCH_BD][CH_index].Rb_info.Lchan_desc[0],
		 (LCHAN_DESC*)&Mac_config_req.Lchan_desc[0],LCHAN_DESC_SIZE);
	  memcpy(&UE_rrc_inst[Mod_id].Rab[In_idx-DTCH_BD][CH_index].Rb_info.Lchan_desc[1],
		 (LCHAN_DESC*)&Mac_config_req.Lchan_desc[1],LCHAN_DESC_SIZE);

	  ret=2;
	  msg("[OPENAIR][RRC][UE %d] GET Lchan Config Command  %d:CONFIG RADIO BEARER %d(%d) ASS to CH Index %d\n",
	      NODE_ID[Mod_id+NB_CH_INST],
	      i,
	      Index, 
	      Mac_config_req.Lchan_id.Index,
	      CH_index);
	  
	  if(Mac_config_req.Lchan_type == DTCH_BD)
	    Mac_rlc_xface->rrc_rlc_config_req(Mod_id+NB_CH_INST,ACTION_ADD,Index,RADIO_ACCESS_BEARER,Rlc_info_um);
	  else
	    Mac_rlc_xface->rrc_rlc_config_req(Mod_id+NB_CH_INST,ACTION_ADD,Index,RADIO_ACCESS_BEARER,Rlc_info_am_config);

	  if(Mac_config_req.Lchan_type == DTCH_BD){
	    UE_rrc_inst[Mod_id].Srb2[CH_index].Srb_info.IP_addr_type=Mac_config_req.L3_info_type;
	    if(Mac_config_req.L3_info_type == IPv4_ADDR){
	      memcpy(UE_rrc_inst[Mod_id].Srb2[CH_index].Srb_info.IP_addr,Mac_config_req.L3_info,4);
	    }
	    else
	      memcpy(UE_rrc_inst[Mod_id].Srb2[CH_index].Srb_info.IP_addr,Mac_config_req.L3_info,16);
	  }
	}
      }
      
      else if(Mac_config_req.Lchan_type == DTCH_DIL){
	Index=Mac_rlc_xface->mac_config_req(Mod_id+NB_CH_INST,ADD_LC,&Mac_config_req);
	UE_rrc_inst[Mod_id].Rab_dil[In_idx][CH_index][Idx1].Active = 1;
	UE_rrc_inst[Mod_id].Rab_dil[In_idx][CH_index][Idx1].Status = RADIO_CONFIG_OK;
	UE_rrc_inst[Mod_id].Rab_dil[In_idx][CH_index][Idx1].Rb_info.Rb_id =Mac_config_req.Lchan_id.Index ;
	ret=2;
      }
      else{
	msg("FATAL: CORRUPTED LCHAN TYPE\n");
	Mac_rlc_xface->macphy_exit("");
      }
    }
    
    R_idx+=MAC_CONFIG_REQ_SIZE ;
  }
  
  DEFAULT_MEAS_REQ Def_meas_req;
  
  for( i=0 ; i < Header->Nb_def_req ; i++ ){//Look for any DEFAULT meas_request
    memcpy(&Def_meas_req,(DEFAULT_MEAS_REQ*)&Srb_info->Rx_buffer.Payload[R_idx],DEFAULT_MEAS_REQ_SIZE);
    if(Def_meas_req.UE_index == UE_index) {
      UE_rrc_inst[Mod_id].Def_meas[CH_index]=&UE_mac_inst[Mod_id].Def_meas[CH_index];
      UE_rrc_inst[Mod_id].Def_meas[CH_index]->Status = RADIO_CONFIG_OK;
      UE_rrc_inst[Mod_id].Def_meas[CH_index]->Active = 1;
      UE_rrc_inst[Mod_id].Def_meas[CH_index]->Forg_fact = Def_meas_req.Forg_fact;
      UE_rrc_inst[Mod_id].Def_meas[CH_index]->Rep_interval = Def_meas_req.Rep_interval;
      UE_rrc_inst[Mod_id].Def_meas[CH_index]->Last_report_frame = Rrc_xface->Frame_index;
      msg("[RRC][UE %d] Default Meas Config OK for CH %d\n ",NODE_ID[Mod_id+NB_CH_INST],CH_index);
    }
    R_idx+=DEFAULT_MEAS_REQ_SIZE;
  }

  for( i=0 ; i < Header->Nb_meas_req ; i++ ){//Look for any MAC meas_request
    memcpy(&Mac_meas_req,(MAC_MEAS_REQ*)&Srb_info->Rx_buffer.Payload[R_idx],MAC_MEAS_REQ_SIZE);
    //    msg("[RRC][NODE %d] MEASUREMENT RADIO CONFIG for LCHAN (%d), Req_size%d, CH_index %d\n",UE_rrc_inst[Mod_id].Node_id,Mac_meas_req.Lchan_id.Index,MAC_MEAS_REQ_SIZE,CH_index);
    UE_index12[0]=( ( Mac_meas_req.Lchan_id.Index & RAB_OFFSET1) >> RAB_SHIFT1 );
    UE_index12[1]=( ( Mac_meas_req.Lchan_id.Index & RAB_OFFSET2 ) >> RAB_SHIFT2 );
    Idx1=0xffff;
    In_idx = (Mac_meas_req.Lchan_id.Index & RAB_OFFSET);
    if(UE_index12[0] == UE_index) Idx1=UE_index12[1];
    if(UE_index12[1] == UE_index) Idx1=UE_index12[0];
    if( Idx1 !=0xffff){
      if( Idx1 == 0 ){ //CH_RAB
	Mac_meas_req.Lchan_id.Index=In_idx+(CH_index << RAB_SHIFT2);	
	Mac_meas_req.UE_CH_index=CH_index;
	msg("[RRC] NODE %d: MEASUREMENT_RADIO CONFIG for LCHAN %d (CH_index %d)\n",
	    NODE_ID[Mod_id+NB_CH_INST],
	    Mac_meas_req.Lchan_id.Index,
	    CH_index);
	if(In_idx==DCCH){
	  UE_rrc_inst[Mod_id].Srb2[CH_index].Srb_info.Meas_entry=Mac_rlc_xface->mac_meas_req(Mod_id+NB_CH_INST,&Mac_meas_req);
	  UE_rrc_inst[Mod_id].Srb2[CH_index].Srb_info.Meas_entry->Last_report_frame=Rrc_xface->Frame_index;
        }
	else{
	  UE_rrc_inst[Mod_id].Rab[In_idx-DTCH_BD][CH_index].Rb_info.Meas_entry=Mac_rlc_xface->mac_meas_req(Mod_id+NB_CH_INST,&Mac_meas_req);
	  UE_rrc_inst[Mod_id].Rab[In_idx-DTCH_BD][CH_index].Rb_info.Meas_entry->Last_report_frame=Rrc_xface->Frame_index;
	}
      }	
      else{
	Mac_meas_req.UE_CH_index=CH_index;
	UE_rrc_inst[Mod_id].Rab_dil[In_idx][CH_index][Idx1].Rb_info.Meas_entry=Mac_rlc_xface->mac_meas_req(Mod_id+NB_CH_INST,&Mac_meas_req);
	UE_rrc_inst[Mod_id].Rab_dil[In_idx][CH_index][Idx1].Rb_info.Meas_entry->Last_report_frame=Rrc_xface->Frame_index;
      }
    }
    R_idx+=MAC_MEAS_REQ_SIZE;
  }
    return (ret);
}


/*------------------------------------------------------------------------------*/
void ch_rrc_decode_ccch(u8 Mod_id, SRB_INFO *Srb_info){
  /*------------------------------------------------------------------------------*/
  unsigned char i=0;
  MAC_CONFIG_REQ Mac_config_req;
  MAC_MEAS_REQ Mac_meas_req;
  u16 Idx,UE_index,In_idx; 
  L2_ID Mac_id;
  memcpy(&Mac_id.L2_id[0],&Srb_info->Rx_buffer.Payload[i],sizeof(L2_ID));
  i+=sizeof(L2_ID);
  
  if((Srb_info->Rx_buffer.Payload[i] !=0) && (Srb_info->Rx_buffer.Payload[i] !=1)) {
    msg("FATAL: RACH for CH %d!!!!\n",Srb_info->Rx_buffer.Payload[i]);
    Mac_rlc_xface->macphy_exit("");  
  }
  if((Srb_info->Rx_buffer.Payload[i] == CH_rrc_inst[Mod_id].Node_id )&& (!rrc_is_mobile_already_associated(Mod_id,Mac_id))){
    
    //until dynamic classification
    UE_index=Mac_id.L2_id[0]-NB_CH_MAX+1;
    if(UE_index!=0xffff){
      
      memcpy(&Rrc_xface->UE_id[Mod_id][UE_index],&Mac_id,sizeof(L2_ID));
      memcpy(&CH_rrc_inst[Mod_id].Info.UE_list[UE_index],&Mac_id,sizeof(L2_ID));
      msg("_______________________[OPENAIR][RRC]CH %d, Frame %d : Accept New connexion from UE %d (UE_index %d)____________\n",CH_rrc_inst[Mod_id].Node_id,Rrc_xface->Frame_index,CH_rrc_inst[Mod_id].Info.UE_list[UE_index].L2_id[0],UE_index);

      //CONFIG SRB2  (DCCHs, ONE per User)  //meas && lchan Cfg 
      CH_rrc_inst[Mod_id].Info.Dtch_bd_config[UE_index].Status=NEED_RADIO_CONFIG;
      CH_rrc_inst[Mod_id].Info.Dtch_bd_config[UE_index].Next_check_frame=Rrc_xface->Frame_index+1;
      CH_rrc_inst[Mod_id].Info.Nb_ue++;	
      
#ifndef NO_RRM
      send_msg(&S_rrc,msg_rrc_MR_attach_ind(Mod_id,Mac_id));
#else
      
      Mac_config_req.Lchan_type = DCCH;
      Mac_config_req.UE_CH_index = UE_index; 
      memcpy(&Mac_config_req.Lchan_desc[0],(LCHAN_DESC*)&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx 
      memcpy(&Mac_config_req.Lchan_desc[1],(LCHAN_DESC*)&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
      Mac_config_req.Lchan_id.Index=(UE_index << RAB_SHIFT2) + DCCH;
      Idx = Mac_rlc_xface->mac_config_req(Mod_id,ADD_LC,&Mac_config_req);
            
      CH_rrc_inst[Mod_id].Srb2[UE_index].Active = 1;
      CH_rrc_inst[Mod_id].Srb2[UE_index].Next_check_frame = Rrc_xface->Frame_index + 250;
      CH_rrc_inst[Mod_id].Srb2[UE_index].Status = NEED_RADIO_CONFIG;//RADIO CFG
      CH_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.Srb_id = Idx;
      memcpy(&CH_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.Lchan_desc[0],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
      memcpy(&CH_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.Lchan_desc[1],&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE);
      
      msg("[OPENAIR][RRC] NODE=%d, CALLING RLC CONFIG SIGNALLING RADIO BEARER %d, %d\n",
	  CH_rrc_inst[Mod_id].Node_id,Idx,Mac_config_req.Lchan_id.Index);
      Mac_rlc_xface->rrc_rlc_config_req(Mod_id,ACTION_ADD,Idx,SIGNALLING_RADIO_BEARER,Rlc_info_um);
      
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
    
#endif //NO_RRM 
    }  
    
  }
}

/*------------------------------------------------------------------------------*/
#define NB_MEAS_MAX 3
void ch_rrc_decode_dcch(u8 Mod_id, char *Rx_sdu){  //measurement
  /*------------------------------------------------------------------------------*/	

  unsigned char i,R_idx=0,In_idx,Nb_meas;
  u16 UE_index;
  DEFAULT_MEAS_IND Def_meas_ind;
  DEFAULT_CH_MEAS *Def_meas_entry=NULL;
  MAC_MEAS_IND Mac_meas_ind;
  MAC_MEAS_REQ_ENTRY *Meas_entry;

  Nb_meas=Rx_sdu[R_idx++];
  if (Nb_meas > NB_MEAS_MAX) {
    msg("[RRC][CH] TTI: %d, decode_dcch, nb_meas too big %d\n",
	Rrc_xface->Frame_index,Nb_meas);
    return;
  }
  
  memcpy(&Def_meas_ind,(DEFAULT_MEAS_IND*)&Rx_sdu[R_idx],DEFAULT_MEAS_IND_SIZE);
  UE_index=Def_meas_ind.UE_index;
  
  if (UE_index > NB_CNX_CH) {
    msg("[RRC][CH] TTI: %d, decode_dcch, UE_index too big %d\n",
	Rrc_xface->Frame_index,UE_index);
    return;
  }
  
  Def_meas_entry =CH_rrc_inst[Mod_id].Def_meas[UE_index];
  //       msg("\n[OPENAIR][RRC CH %d] Frame :%d CH_DECODE_DCCH : Receive DEF Measurement from UE_index%d UE Fully Connected\n",Mod_id,Rrc_xface->Frame_index,UE_index);
    
  if(Def_meas_entry!=NULL){
      if(Def_meas_entry->Status == IDLE){// from mac
	msg("[RRC][CH %d] DECODE DCCH: Warning: Rx Measurement for undefined Default meas process \n",NODE_ID[Mod_id]);
	Mac_rlc_xface->macphy_exit("");  
      }
      else{
	Def_meas_entry->Last_report_frame= Rrc_xface->Frame_index;
	Def_meas_entry->Status = RADIO_CONFIG_OK;
	Def_meas_entry->Next_check_frame = 0;
	CH_rrc_inst[Mod_id].Srb2[UE_index].Status = RADIO_CONFIG_OK;
	CH_rrc_inst[Mod_id].Srb2[UE_index].Next_check_frame = 0;
	CH_rrc_inst[Mod_id].Srb2_meas[UE_index].Status = RADIO_CONFIG_OK; 
	for(i=0;i<NB_RAB_MAX;i++)
	  if((CH_rrc_inst[Mod_id].Rab[i][UE_index].Active==1) && (Def_meas_ind.Rb_active[i]==1)
	     &&(CH_rrc_inst[Mod_id].Rab[i][UE_index].Status != RADIO_CONFIG_OK)) {
	    CH_rrc_inst[Mod_id].Rab[i][UE_index].Status = RADIO_CONFIG_OK;
	    CH_rrc_inst[Mod_id].Rab[i][UE_index].Next_check_frame = 0;
	    CH_rrc_inst[Mod_id].Rab_meas[i][UE_index].Status = RADIO_CONFIG_OK;
	    CH_mac_inst[Mod_id].Dtch_lchan[i][UE_index].Active=1;
	    msg("[RRC]CH %d: decode dcch: Rx ack for DTCH %d CONFIG from user index %d\n",
		NODE_ID[Mod_id],
		CH_rrc_inst[Mod_id].Rab[i][UE_index].Rb_info.Rb_id,
		UE_index);
	  }
      }
      R_idx+=DEFAULT_MEAS_IND_SIZE;

      for(i=0;i<Nb_meas;i++){ 
	Meas_entry=NULL;
	memcpy(&Mac_meas_ind,(MAC_MEAS_IND*)&Rx_sdu[R_idx],MAC_MEAS_IND_SIZE);
	In_idx=(Mac_meas_ind.Lchan_id.Index & RAB_OFFSET);
	if (In_idx==DCCH)
	  Meas_entry=&CH_rrc_inst[Mod_id].Srb2_meas[UE_index];
	else if (In_idx > DCCH)
	  Meas_entry=&CH_rrc_inst[Mod_id].Rab_meas[In_idx-DTCH_BD][UE_index];
	if(Meas_entry!=NULL){
	  msg("[RRC][CH %d] Rx meas for Lchan_id %d\n",Mod_id,Mac_meas_ind.Lchan_id.Index);
	  memcpy(&Meas_entry->Mac_meas_req.Mac_meas,(MAC_MEAS_T*)&Mac_meas_ind.Meas,MAC_MEAS_T_SIZE);
	}
	R_idx+=MAC_MEAS_IND_SIZE;
      }
      if( (Rx_sdu[R_idx]!=0) && (CH_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.IP_addr_type==NONE_L3) ){//GOT MR IP ADDR
	CH_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.IP_addr_type=Rx_sdu[R_idx++];
	if(CH_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.IP_addr_type==IPv4_ADDR)
	  memcpy(CH_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.IP_addr,&Rx_sdu[R_idx],4);
	else
	  memcpy(CH_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.IP_addr,&Rx_sdu[R_idx],16);
	//msg("[RRC] Inst %d: send rrc_cx_establish_ind Mr %d,ip@ %d \n",Mod_id,UE_index,
	//  CH_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.MR_ip_addr);

	//	L2_ID L2_id;
	//L2_id.L2_id[0]=CH_rrc_inst[Mod_id].Info.UE_list[UE_index];
#ifndef NO_RRM
	send_msg(&S_rrc,msg_rrc_cx_establish_ind(Mod_id,CH_rrc_inst[Mod_id].Info.UE_list[UE_index],0,
						 CH_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.IP_addr,
						 CH_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.IP_addr_type,
						 CH_rrc_inst[Mod_id].Rab[0][0].Rb_info.Rb_id,
						 CH_rrc_inst[Mod_id].Rab[1][UE_index].Rb_info.Rb_id));
#endif
      }
      
  }
    else{
              msg("[FRAME %d]NODEB %d,DEF_MEAS IS NULL, UE_index %d, Nb_meas=%d, MSG_DUMP!!!\n",Rrc_xface->Frame_index,
          NODE_ID[Mod_id],UE_index,Nb_meas);
	//for(i=0;i<DEFAULT_MEAS_IND_SIZE+1;i++)
	//	  msg("%x.",Rx_sdu[i]);
	//msg("\n");

	Mac_rlc_xface->macphy_exit("");

    }
  //exit(0);
}



/*------------------------------------------------------------------------------*/
void ch_rrc_generate_bcch(u8 Mod_id){
  /*------------------------------------------------------------------------------*/
  //NOTHING TO DO YET ( segmentation)  
}      
/*------------------------------------------------------------------------------*/
char ch_rrc_generate_ccch(u8 Mod_id){//FILL CCCH_TX_BUFFER every X frames
  /*------------------------------------------------------------------------------*/
  CH_rrc_inst[Mod_id].Srb1.Tx_buffer.W_idx=0;//CH_CCCH_HEADER_SIZE;
  ((CH_CCCH_HEADER*)(CH_rrc_inst[Mod_id].Srb1.Tx_buffer.Header))->Nb_cfg_req=0;
  ((CH_CCCH_HEADER*)(CH_rrc_inst[Mod_id].Srb1.Tx_buffer.Header))->Nb_meas_req=0;
  ((CH_CCCH_HEADER*)(CH_rrc_inst[Mod_id].Srb1.Tx_buffer.Header))->Nb_def_req=0;
  ((CH_CCCH_HEADER*)(CH_rrc_inst[Mod_id].Srb1.Tx_buffer.Header))->Nb_sens_req=0;
  //     msg("[OPENAIR][RRC]________________________CH: GENERATE CCCH, NB_ue=%d__________________________________\n",
  //  Rrc_inst[Mod_id].Rrc_info[0].Info.CH_info.Nb_ue);
  unsigned char i,j;
  MAC_CONFIG_REQ Mac_config_req;
  MAC_MEAS_REQ Mac_meas_req;
  MAC_MEAS_REQ_ENTRY *Meas_entry;	
  DEFAULT_MEAS_REQ Def_meas_req;
#ifndef NO_RRM
  if(CH_rrc_inst[Mod_id].Last_scan_req >0){
    if(!rrc_fill_buffer(&CH_rrc_inst[Mod_id].Srb1.Tx_buffer,(char*)&CH_rrc_inst[Mod_id].Rrm_init_scan_req,sizeof(rrm_init_scan_req_t))){
      msg("ERROR sending INIT_SCAN_REQ\n");
      return 0;
    }
    
    ((CH_CCCH_HEADER*)(CH_rrc_inst[Mod_id].Srb1.Tx_buffer.Header))->Nb_sens_req++;
    msg("RRC:CH: send init scan req on CCCH\n ");
    CH_rrc_inst[Mod_id].Last_scan_req=0;
  }
  #endif
  for(i=1 ; i<= NB_CNX_CH; i++){
    if(CH_rrc_inst[Mod_id].Info.UE_list[i].L2_id[0]!=0xff){
      if((CH_rrc_inst[Mod_id].Info.Dtch_bd_config[i].Status==NEED_RADIO_CONFIG) 
	 ||( (CH_rrc_inst[Mod_id].Info.Dtch_bd_config[i].Status==NEED_RADIO_CONFIG) && (CH_rrc_inst[Mod_id].Info.Dtch_bd_config[i].Next_check_frame < Rrc_xface->Frame_index)) ){
	Mac_config_req.Lchan_id.Index = ( i << RAB_SHIFT2 )+(CH_rrc_inst[Mod_id].Rab[0][0].Rb_info.Rb_id);
	Mac_config_req.Lchan_type = DTCH_BD;
	memcpy(&Mac_config_req.Lchan_desc[0],(LCHAN_DESC*)&CH_rrc_inst[Mod_id].Rab[0][0].Rb_info.Lchan_desc[1],LCHAN_DESC_SIZE);
	memcpy(&Mac_config_req.Lchan_desc[1],(LCHAN_DESC*)&CH_rrc_inst[Mod_id].Rab[0][0].Rb_info.Lchan_desc[0],LCHAN_DESC_SIZE);
	Mac_config_req.L3_info_type=CH_rrc_inst[Mod_id].IP_addr_type;
	memcpy(Mac_config_req.L3_info,CH_rrc_inst[Mod_id].IP_addr,16);
	
	
	if(!rrc_fill_buffer(&CH_rrc_inst[Mod_id].Srb1.Tx_buffer,(char*)&Mac_config_req,MAC_CONFIG_REQ_SIZE))
	  return 0;
	//msg("[OPENAIR][RRC]CH %d: frame %d: GENERATE CCCH: Lchan_config_req Id %d UE_index %d, W_idx %d (Rx %d, Tx %d)\n",
	//  Rrc_xface->Frame_index,
	//  NODE_ID[Mod_id], Mac_config_req.Lchan_id.Index,i,CH_rrc_inst[Mod_id].Srb1.Tx_buffer.W_idx,
	//  Mac_config_req.Lchan_desc[0].transport_block_size,
	//  Mac_config_req.Lchan_desc[1].transport_block_size);
	  //usleep(1000000);
	((CH_CCCH_HEADER*)(CH_rrc_inst[Mod_id].Srb1.Tx_buffer.Header))->Nb_cfg_req++;
	if(CH_rrc_inst[Mod_id].Info.Dtch_bd_config[i].Status == RADIO_CONFIG_TX)
	  CH_rrc_inst[Mod_id].Info.Dtch_bd_config[i].Next_check_frame = Rrc_xface->Frame_index + 100;    
	else CH_rrc_inst[Mod_id].Info.Dtch_bd_config[i].Status=RADIO_CONFIG_TX;  
	
      }
      if( (CH_rrc_inst[Mod_id].Srb2[i].Active == 1) && 
	  ( (CH_rrc_inst[Mod_id].Srb2[i].Status == NEED_RADIO_CONFIG) || ((CH_rrc_inst[Mod_id].Srb2[i].Status == RADIO_CONFIG_TX)  && (CH_rrc_inst[Mod_id].Srb2[i].Next_check_frame < Rrc_xface->Frame_index ) ))){
	Mac_config_req.Lchan_id.Index = CH_rrc_inst[Mod_id].Srb2[i].Srb_info.Srb_id;
	Mac_config_req.Lchan_type = DCCH;
	memcpy(&Mac_config_req.Lchan_desc[0],(LCHAN_DESC*)&CH_rrc_inst[Mod_id].Srb2[i].Srb_info.Lchan_desc[0],LCHAN_DESC_SIZE);
	memcpy(&Mac_config_req.Lchan_desc[1],(LCHAN_DESC*)&CH_rrc_inst[Mod_id].Srb2[i].Srb_info.Lchan_desc[1],LCHAN_DESC_SIZE);
	
	Mac_config_req.L3_info_type= MAC_ADDR;
	  memcpy(Mac_config_req.L3_info,&CH_rrc_inst[Mod_id].Mac_id,sizeof(L2_ID));
	  
	  if(!rrc_fill_buffer(&CH_rrc_inst[Mod_id].Srb1.Tx_buffer,(char*)&Mac_config_req,MAC_CONFIG_REQ_SIZE))
	    return 0;
	  //msg("[OPENAIR][RRC]CH: Frame %d: GENERATE CCCH: Lchan_config_req Id %d UE_index %d, W_idx=%d\n", 
	  //  Rrc_xface->Frame_index,
	  //  Mac_config_req.Lchan_id.Index,i,CH_rrc_inst[Mod_id].Srb1.Tx_buffer.W_idx);
	  //usleep(1000000);
	  ((CH_CCCH_HEADER*)(CH_rrc_inst[Mod_id].Srb1.Tx_buffer.Header))->Nb_cfg_req++;
	  if(CH_rrc_inst[Mod_id].Srb2[i].Status == RADIO_CONFIG_TX)
	    CH_rrc_inst[Mod_id].Srb2[i].Next_check_frame = Rrc_xface->Frame_index + 50;    
	  else CH_rrc_inst[Mod_id].Srb2[i].Status=RADIO_CONFIG_TX;  
	  
      } 
      for(j=1; j<NB_RAB_MAX;j++ ){
	if( (CH_rrc_inst[Mod_id].Rab[j][i].Active == 1) && 
	    ((CH_rrc_inst[Mod_id].Rab[j][i].Status == NEED_RADIO_CONFIG) ||((CH_rrc_inst[Mod_id].Rab[j][i].Status == RADIO_CONFIG_TX) && (CH_rrc_inst[Mod_id].Rab[j][i].Next_check_frame < Rrc_xface->Frame_index)) )){
	  Mac_config_req.Lchan_id.Index = CH_rrc_inst[Mod_id].Rab[j][i].Rb_info.Rb_id;
	  Mac_config_req.Lchan_type = DTCH;
	  memcpy(&Mac_config_req.Lchan_desc[0],(LCHAN_DESC*)&CH_rrc_inst[Mod_id].Rab[j][i].Rb_info.Lchan_desc[1],LCHAN_DESC_SIZE);
	  memcpy(&Mac_config_req.Lchan_desc[1],(LCHAN_DESC*)&CH_rrc_inst[Mod_id].Rab[j][i].Rb_info.Lchan_desc[0],LCHAN_DESC_SIZE);
	  
	  
	  
	  if(!rrc_fill_buffer(&CH_rrc_inst[Mod_id].Srb1.Tx_buffer,(char*)&Mac_config_req,MAC_CONFIG_REQ_SIZE))
	    return 0;
	  //msg("[OPENAIR][RRC]CH %d: frame %d: GENERATE CCCH: Lchan_config_req Id %d UE_index %d, W_idx %d (Rx %d, Tx %d)\n",
	  //  Rrc_xface->Frame_index,
	  //  NODE_ID[Mod_id], Mac_config_req.Lchan_id.Index,i,CH_rrc_inst[Mod_id].Srb1.Tx_buffer.W_idx,
	  //  Mac_config_req.Lchan_desc[0].transport_block_size,
	  //  Mac_config_req.Lchan_desc[1].transport_block_size);
	  //usleep(1000000);
	  ((CH_CCCH_HEADER*)(CH_rrc_inst[Mod_id].Srb1.Tx_buffer.Header))->Nb_cfg_req++;
	  if(CH_rrc_inst[Mod_id].Rab[j][i].Status == RADIO_CONFIG_TX)
            CH_rrc_inst[Mod_id].Rab[j][i].Next_check_frame = Rrc_xface->Frame_index + 100;    
	  else CH_rrc_inst[Mod_id].Rab[j][i].Status=RADIO_CONFIG_TX;  
	  
	}
	//DTCH_DIL
      }
      
      
      
    }
  }

  for(i=1 ; i<= NB_CNX_CH; i++){
    if((CH_rrc_inst[Mod_id].Info.UE_list[i].L2_id[0]!=0xff) && 
       (CH_rrc_inst[Mod_id].Def_meas[i]!=NULL)) {
      if( (CH_rrc_inst[Mod_id].Def_meas[i]->Status !=IDLE) && 
	  ( (Rrc_xface->Frame_index - CH_rrc_inst[Mod_id].Def_meas[i]->Last_report_frame) >  10*CH_rrc_inst[Mod_id].Def_meas[i]->Rep_interval) ){
	msg("DISCONNECT UE %d,LAST REPORT %d, REP_INTERVAL %d, TTI %d\n",
	    i,
	    CH_rrc_inst[Mod_id].Def_meas[i]->Last_report_frame,
	    CH_rrc_inst[Mod_id].Def_meas[i]->Rep_interval,
	    Rrc_xface->Frame_index);	ch_disconnect_ue(Mod_id,i);
	return 1;
      }
      if( (CH_rrc_inst[Mod_id].Def_meas[i]->Status == NEED_RADIO_CONFIG) || 
	  ((CH_rrc_inst[Mod_id].Def_meas[i]->Status !=IDLE) && 
	   (CH_rrc_inst[Mod_id].Def_meas[i]->Next_check_frame == Rrc_xface->Frame_index ) &&
	   (((CH_rrc_inst[Mod_id].Def_meas[i]->Next_check_frame - CH_rrc_inst[Mod_id].Def_meas[i]->Last_report_frame) >=  CH_rrc_inst[Mod_id].Def_meas[i]->Rep_interval)))){
	Def_meas_req.UE_index = i;//CH_rrc_inst[Mod_id].Info.UE_list[i];
	Def_meas_req.Forg_fact = CH_rrc_inst[Mod_id].Def_meas[i]->Forg_fact;
	Def_meas_req.Rep_interval = CH_rrc_inst[Mod_id].Def_meas[i]->Rep_interval; 
	if(!rrc_fill_buffer(&CH_rrc_inst[Mod_id].Srb1.Tx_buffer,(char*)&Def_meas_req,DEFAULT_MEAS_REQ_SIZE))
	  return 0;
	((CH_CCCH_HEADER*)(CH_rrc_inst[Mod_id].Srb1.Tx_buffer.Header))->Nb_def_req++;
	
	
	if(CH_rrc_inst[Mod_id].Def_meas[i]->Status == NEED_RADIO_CONFIG)
	  CH_rrc_inst[Mod_id].Def_meas[i]->Status = RADIO_CONFIG_TX;
        else
          CH_rrc_inst[Mod_id].Def_meas[i]->Next_check_frame+=CH_rrc_inst[Mod_id].Def_meas[i]->Rep_interval;	
        //CH_rrc_inst[Mod_id].Def_meas[i]->Last_report_frame
	//msg("[OPENAIR][RRC]CH %d: Frame %d: GENERATE CCCH: Default Measurement config UE_index %d; Frame %d, Next_check %d, Last_rep %d,Rep_interval %d\n",
	//  NODE_ID[Mod_id], 
	//  Rrc_xface->Frame_index,
	//  i,Rrc_xface->Frame_index,CH_rrc_inst[Mod_id].Def_meas[i]->Next_check_frame,CH_rrc_inst[Mod_id].Def_meas[i]->Last_report_frame,CH_rrc_inst[Mod_id].Def_meas[i]->Rep_interval);        
      }
    }
  }

  /*
  for(i=1 ; i<= NB_CNX_CH; i++){
    if(CH_rrc_inst[Mod_id].Info.UE_list[i].L2_id[0]!=0xff){
      Meas_entry=&CH_rrc_inst[Mod_id].Srb2_meas[i];
      if( 
	 ( Meas_entry->Status == NEED_RADIO_CONFIG ) ){//|| (
	//(Meas_entry->Status != IDLE ) && (Meas_entry->Next_check_frame == Rrc_xface->Frame_index ) 
	//					 && ( (Meas_entry->Next_check_frame - Meas_entry->Last_report_frame) >= Meas_entry->Mac_meas_req.Rep_interval) )
	//						 ){
	
	Mac_meas_req.Lchan_id.Index=Meas_entry->Mac_meas_req.Lchan_id.Index;
	Mac_meas_req.Meas_trigger=Meas_entry->Mac_meas_req.Meas_trigger;
	Mac_meas_req.Mac_avg=Meas_entry->Mac_meas_req.Mac_avg;
	Mac_meas_req.Process_id=Meas_entry->Mac_meas_req.Process_id;
	Mac_meas_req.Rep_amount=Meas_entry->Mac_meas_req.Rep_amount;
	Mac_meas_req.Rep_interval=Meas_entry->Mac_meas_req.Rep_interval;
	if(!rrc_fill_buffer(&CH_rrc_inst[Mod_id].Srb1.Tx_buffer,(char*)&Mac_meas_req,MAC_MEAS_REQ_SIZE))
	  return 0;
	((CH_CCCH_HEADER*)(CH_rrc_inst[Mod_id].Srb1.Tx_buffer.Header))->Nb_meas_req++;
	//msg("Frame %d, Next %d, last %d, interval %d, W_idx %d \n",Rrc_xface->Frame_index,Meas_entry->Next_check_frame,Meas_entry->Last_report_frame,Meas_entry->Mac_meas_req.Rep_interval,CH_rrc_inst[Mod_id].Srb1.Tx_buffer.W_idx);
        if(Meas_entry->Status == NEED_RADIO_CONFIG)
	  Meas_entry->Status = RADIO_CONFIG_TX;
        else{
          Meas_entry->Next_check_frame+=Meas_entry->Mac_meas_req.Rep_interval;
          //exit(0);
	}
           

      }
      for(j=0; j<NB_RAB_MAX;j++ ){
	Meas_entry=&CH_rrc_inst[Mod_id].Rab_meas[j][i];
	if( 
	   ( Meas_entry->Status == NEED_RADIO_CONFIG )){//|| ((Meas_entry->Status != IDLE ) &&(Meas_entry->Next_check_frame == Rrc_xface->Frame_index) && (Meas_entry->Next_check_frame - Meas_entry->Last_report_frame >= Meas_entry->Mac_meas_req.Rep_interval) )){
	  Mac_meas_req.Lchan_id.Index=Meas_entry->Mac_meas_req.Lchan_id.Index;
	  Mac_meas_req.Meas_trigger=Meas_entry->Mac_meas_req.Meas_trigger;
	  Mac_meas_req.Mac_avg=Meas_entry->Mac_meas_req.Mac_avg;
	  Mac_meas_req.Process_id=Meas_entry->Mac_meas_req.Process_id;
	  Mac_meas_req.Rep_amount=Meas_entry->Mac_meas_req.Rep_amount;
	  Mac_meas_req.Rep_interval=Meas_entry->Mac_meas_req.Rep_interval;
	  if(!rrc_fill_buffer(&CH_rrc_inst[Mod_id].Srb1.Tx_buffer,(char*)&Mac_meas_req,MAC_MEAS_REQ_SIZE))
	    return 0;
	  ((CH_CCCH_HEADER*)(CH_rrc_inst[Mod_id].Srb1.Tx_buffer.Header))->Nb_meas_req++;
	  //msg("11  Frame %d, Next %d, last %d, interval %d, W_idx %d\n",Rrc_xface->Frame_index,Meas_entry->Next_check_frame,Meas_entry->Last_report_frame,Meas_entry->Mac_meas_req.Rep_interval,CH_rrc_inst[Mod_id].Srb1.Tx_buffer.W_idx);

	  if(Meas_entry->Status == NEED_RADIO_CONFIG)
	    Meas_entry->Status = RADIO_CONFIG_TX;
	  else{
	    Meas_entry->Next_check_frame+=Meas_entry->Mac_meas_req.Rep_interval;
	    //exit(0);
          }


	}
      }
    }
  }
*/
  return 1;
}

/*------------------------------------------------------------------------------------------*/
void ch_disconnect_ue(unsigned char Mod_id,unsigned char UE_index){
  /*------------------------------------------------------------------------------------------*/
  unsigned char i;
  MAC_CONFIG_REQ Mac_config_req;

  Mac_config_req.UE_CH_index=UE_index;
  msg("______________[RRC_XFACE] FRAME %d: CH %d,NODE %d(UE_index %d) OUT OF SYNC, DISCONNECT!______________\n ",Rrc_xface->Frame_index,CH_rrc_inst[Mod_id].Node_id,CH_rrc_inst[Mod_id].Info.UE_list[UE_index].L2_id[0],UE_index);

  CH_rrc_inst[Mod_id].Info.Nb_ue--;

  CH_rrc_inst[Mod_id].Def_meas[UE_index]->Active = 0;
  CH_rrc_inst[Mod_id].Def_meas[UE_index]->Status = IDLE;
  CH_rrc_inst[Mod_id].Def_meas[UE_index]->Last_report_frame=0;
  CH_rrc_inst[Mod_id].Def_meas[UE_index]->Next_check_frame=0;

  CH_rrc_inst[Mod_id].Info.UE_list[UE_index].L2_id[0]=0xff;
  CH_rrc_inst[Mod_id].Srb2[UE_index].Active = 0;
  CH_rrc_inst[Mod_id].Srb2[UE_index].Status = IDLE;//RADIO CFG
  CH_rrc_inst[Mod_id].Srb2[UE_index].Next_check_frame = 0;
  CH_rrc_inst[Mod_id].Srb2_meas[UE_index].Status = IDLE;
  CH_rrc_inst[Mod_id].Srb2[UE_index].Srb_info.Meas_entry->Status = IDLE;
  CH_rrc_inst[Mod_id].Def_meas[UE_index]->Status = IDLE;
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
#ifndef NO_RRM
    send_msg(&S_rrc,msg_rrc_end_scan_req(Mod_id+NB_CH_INST,CH_index));
#endif
}

/*------------------------------------------------------------------------------------------*/
void  rrc_process_radio_meas(u8 Mod_id,MAC_MEAS_IND Mac_meas_ind,MAC_MEAS_REQ_ENTRY * Meas_entry){
  /*------------------------------------------------------------------------------------------*/

}


/*------------------------------------------------------------------------------------------*/
void mac_rrc_radio_meas_resp(MAC_MEAS_T *Mac_meas, MAC_MEAS_REQ_ENTRY * Meas_req_table_entry){
  /*---------------------------------------------------------------------------------------*/
}


#ifndef USER_MODE
EXPORT_SYMBOL(Rlc_info_am_config);
#endif
