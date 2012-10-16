/***************************************************************************
                          rrc_rg_L2_interface.c  -
                          -------------------
    begin                : Sept 9, 2008
    copyright            : (C) 2008, 2010 by Eurecom
    created by           : michelle.wetterwald@eurecom.fr
    description
 **************************************************************************
    Entry point for L2 interfaces
 ***************************************************************************/
/********************
//OpenAir definitions
 ********************/
#include "LAYER2/MAC/extern.h"
#include "COMMON/openair_defs.h"
#include "RRC/L2_INTERFACE/openair_rrc_L2_interface.h"
#include "LAYER2/RLC/rlc.h"
#include "COMMON/mac_rrc_primitives.h"
#include "UTIL/MEM/mem_block.h"

//#include "SIMULATION/simulation_defs.h"
//extern EMULATION_VARS *Emul_vars;
//extern CH_MAC_INST *CH_mac_inst;
//extern UE_MAC_INST *UE_mac_inst;
/********************
// RRC definitions
 ********************/
#include "rrc_rg_vars.h"
//-----------------------------------------------------------------------------
#include "rrc_L2_proto.h"
#include "rrc_proto_bch.h"
//-----------------------------------------------------------------------------
extern LCHAN_DESC BCCH_LCHAN_DESC,CCCH_LCHAN_DESC, DCCH_LCHAN_DESC, DTCH_LCHAN_DESC;
extern rlc_info_t Rlc_info_um, Rlc_info_am_config;
//-----------------------------------------------------------------------------
s8 rrc_L2_data_req_rx (unsigned char Mod_id, unsigned short Srb_id, unsigned char Nb_tb,char *Buffer,u8 CH_index){
//-----------------------------------------------------------------------------
  unsigned char br_size=0;

#ifdef DEBUG_RRC_BROADCAST
  msg ("\n[RRC CELL][L2_INTF] rrc_L2_data_req_rx - begin\n");
  //msg ("Received parameters Mod_id %d, Srb_id %d, nb_tb %d, CH_index %d\n",Mod_id, Srb_id,Nb_tb,CH_index);
#endif
  if ( Mac_rlc_xface->Is_cluster_head[Mod_id]){
    if ((Srb_id & RAB_OFFSET) == BCCH){
      br_size = rrc_broadcast_tx(Buffer);
#ifdef DEBUG_RRC_BROADCAST_DETAILS
      msg ("[RRC CELL][L2_INTF] rrc_L2_data_req_rx - end BCCH,  br_size %d\n",br_size);
#endif
      //return br_size;
    }
  }
#ifdef DEBUG_RRC_BROADCAST_DETAILS
  msg ("\n[RRC CELL][L2_INTF] rrc_L2_data_req_rx - end\n");
#endif
  return br_size;
}

//-----------------------------------------------------------------------------
s8 rrc_L2_mac_data_ind_rx (u8 Mod_id, u16 Srb_id, char *Sdu, u8 CH_index){
//-----------------------------------------------------------------------------
#ifdef DEBUG_RRC_BROADCAST
  msg ("\n[RRC CELL][L2_INTF] rrc_L2_mac_data_ind_rx - begin - RG => Xmit only in BCCH\n");
  //msg ("Received parameters Mod_id %d, Srb_id %d, CH_index %d\n",Mod_id, Srb_id,CH_index);
#endif
#ifdef DEBUG_RRC_BROADCAST_DETAILS
  msg ("\n[RRC CELL][L2_INTF] rrc_L2_mac_data_ind_rx - end\n");
#endif
return 0;
}

//-----------------------------------------------------------------------------
void rrc_L2_rlc_data_ind_rx (unsigned char Mod_id, unsigned int Srb_id, unsigned int Sdu_size, unsigned char *Buffer){
//-----------------------------------------------------------------------------
#ifdef DEBUG_RRC_DETAILS
  msg ("\n[RRC][L2_INTF] rrc_L2_rlc_data_ind_rx - begin\n");
#endif
  //rrc_rg_test_rlc_intf_rcve (Buffer, Srb_id); // Dummy version
  rrc_rg_srb_rx (Buffer, Srb_id);
}

//-----------------------------------------------------------------------------
void rrc_L2_rlc_confirm_ind_rx (unsigned char Mod_id, unsigned int Srb_id, unsigned int mui){
//-----------------------------------------------------------------------------
#ifdef DEBUG_RRC_DETAILS
  msg ("\n[RRC][L2_INTF] rrc_L2_rlc_confirm_ind_rx - begin\n");
#endif
 //void* rrc_srb_confirm (u32 muiP, u8 rb_idP, u8 statusP);
  rrc_rg_srb_confirm (mui, Srb_id, 0);
}


/* 
public_rlc_rrc( void   rrc_rlc_register_rrc ( void (*rrc_data_indP)  (module_id_t , rb_id_t , sdu_size_t , char*),
 void            (*rrc_data_conf) (module_id_t , rb_id_t , mui_t) );)
*/
//-----------------------------------------------------------------------------
void rrc_L2_mac_meas_ind_rx (void){
//-----------------------------------------------------------------------------
#ifdef DEBUG_RRC_DETAILS
  msg ("\n[RRC][L2_INTF] rrc_L2_mac_meas_ind_rx - begin\n");
#endif
}

//-----------------------------------------------------------------------------
void rrc_L2_def_meas_ind_rx (unsigned char Mod_id, unsigned char Idx2){
//-----------------------------------------------------------------------------
#ifdef DEBUG_RRC_DETAILS
  msg ("\n[RRC][L2_INTF] rrc_L2_def_meas_ind_rx - begin\n");
#endif
}

//-----------------------------------------------------------------------------
void rrc_L2_sync_ind_rx (void){
//-----------------------------------------------------------------------------
#ifdef DEBUG_RRC_DETAILS
  msg ("\n[RRC][L2_INTF] rrc_L2_sync_ind_rx - begin\n");
#endif
}

//-----------------------------------------------------------------------------
void rrc_L2_out_sync_ind_rx (void){
//-----------------------------------------------------------------------------
#ifdef DEBUG_RRC_DETAILS
  msg ("\n[RRC][L2_INTF] rrrc_L2_out_sync_ind_rx - begin\n");
#endif
}

//-----------------------------------------------------------------------------
int rrc_L2_get_rrc_status(u8 Mod_id,u8 eNB_flag,u8 index){
//-----------------------------------------------------------------------------
#ifdef DEBUG_RRC_DETAILS
    msg ("\n[RRC][L2_INTF] rrc_L2_get_rrc_status - begin\n");
#endif
/*
  if(eNB_flag == 1)
    return(eNB_rrc_inst[Mod_id].Info.Status[index]);
  else
    return(UE_rrc_inst[Mod_id].Info[index].Status);
*/
   return 0;
}

//-----------------------------------------------------------------------------
char rrc_L2_ue_init(u8 Mod_id, unsigned char eNB_index){
//-----------------------------------------------------------------------------
#ifdef DEBUG_RRC_DETAILS
    msg ("\n[RRC][L2_INTF] rrc_L2_ue_init - begin\n");
#endif
    rrc_rg_uelite_init(Mod_id, eNB_index);
    return 0;
}


//-----------------------------------------------------------------------------
char rrc_L2_eNB_init(u8 Mod_id){
//-----------------------------------------------------------------------------
#ifdef DEBUG_RRC_DETAILS
    msg ("\n[RRC][L2_INTF] rrc_L2_eNB_init - begin\n");
#endif
    rrc_rg_init (Mod_id);
    return 0;
}

//-----------------------------------------------------------------------------
// Out of openair_rrc_L2_interface.c
void openair_rrc_lite_top_init(void){
//-----------------------------------------------------------------------------
  //#ifdef DEBUG_RRC_STATE
   msg ("\n[RRC CELL] [L2_INTF] openair_rrc_lite_top_init - Empty function to keep compatibility with RRC LITE\n\n");
  //#endif
}

//-----------------------------------------------------------------------------
RRC_status_t rrc_rx_tx(u8 Mod_id,u32 frame, u8 eNB_flag,u8 index){
//-----------------------------------------------------------------------------
#ifdef DEBUG_RRC_DETAILS
    msg ("\n[RRC][L2_INTF] rrc_L2_eNB_init - begin\n");
#endif
    rrc_rg_main_scheduler(Mod_id, frame, 0, 0);
    return 0;
}

/*------------------------------------------------------------------------------*/
// Dummy function - to keep compatibility with RRC LITE
char rrc_rg_uelite_init(u8 Mod_id, unsigned char eNB_index){
/*------------------------------------------------------------------------------*/
  //#ifdef DEBUG_RRC_STATE
   msg ("\n[RRC CELL] Called rrc_rg_ue_init - Dummy function - to keep compatibility with RRC LITE\n\n");
  //#endif
  return 0;
}


/*------------------------------------------------------------------------------*/
// Send config to the MAC Layer
void rrc_init_mac_config(void){
/*------------------------------------------------------------------------------*/
  MAC_CONFIG_REQ Mac_config_req;
  int UE_index,Idx;

  // The content of this function has been commented on 23/03/2012
  printk("\n rrc_init_mac_config -- COMMENTED\n");

  /*
  //#ifdef DEBUG_RRC_STATE
   msg ("\n[RRC CELL] Called rrc_init_mac_config - Begin \n\n");
  //#endif

  UE_index=1;

  // Configure BCCH
  Mac_config_req.Lchan_type = BCCH;
  memcpy(&Mac_config_req.Lchan_desc[0],(LCHAN_DESC*)&BCCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
  memcpy(&Mac_config_req.Lchan_desc[1],(LCHAN_DESC*)&BCCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
  Mac_config_req.UE_CH_index=0;
  Mac_config_req.Lchan_id.Index=(0 << RAB_SHIFT2) + BCCH;
  printk("Calling mac_config_req for BCCH\n");
  Mac_rlc_xface->mac_config_req(0,ADD_LC,&Mac_config_req);

  // Configure CCCH
  Mac_config_req.Lchan_type = CCCH;
  memcpy(&Mac_config_req.Lchan_desc[0],(LCHAN_DESC*)&CCCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
  memcpy(&Mac_config_req.Lchan_desc[1],(LCHAN_DESC*)&CCCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
  Mac_config_req.UE_CH_index=0;
  Mac_config_req.Lchan_id.Index=(0 << RAB_SHIFT2) + CCCH;
  printk("Calling mac_config_req for CCCH\n");
  Mac_rlc_xface->mac_config_req(0,ADD_LC,&Mac_config_req);
	
 // Configure DCCH (LTE ACCESS)
  Mac_config_req.Lchan_type = DCCH;
  Mac_config_req.UE_CH_index = UE_index;
  memcpy(&Mac_config_req.Lchan_desc[0],(LCHAN_DESC*)&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx 
  memcpy(&Mac_config_req.Lchan_desc[1],(LCHAN_DESC*)&DCCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
  Mac_config_req.Lchan_id.Index=(UE_index << RAB_SHIFT2) + DCCH;
  Idx = Mac_rlc_xface->mac_config_req(0,ADD_LC,&Mac_config_req);
  Mac_rlc_xface->rrc_rlc_config_req(0,ACTION_ADD,Idx,SIGNALLING_RADIO_BEARER,Rlc_info_um);

// Configure DTCH (SRB0)
  Mac_config_req.Lchan_type = DTCH;
  Mac_config_req.UE_CH_index = UE_index;
  memcpy(&Mac_config_req.Lchan_desc[0],(LCHAN_DESC*)&DTCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx 
  memcpy(&Mac_config_req.Lchan_desc[1],(LCHAN_DESC*)&DTCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
  Mac_config_req.Lchan_id.Index=(UE_index << RAB_SHIFT2) + DTCH_BD + 1;
  Idx = Mac_rlc_xface->mac_config_req(0,ADD_LC,&Mac_config_req);
  Mac_rlc_xface->rrc_rlc_config_req(0,ACTION_ADD,Idx,SIGNALLING_RADIO_BEARER,Rlc_info_um);

// Configure DTCH (SRB1)
  Mac_config_req.Lchan_type = DTCH;
  Mac_config_req.UE_CH_index = UE_index;
  memcpy(&Mac_config_req.Lchan_desc[0],(LCHAN_DESC*)&DTCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx 
  memcpy(&Mac_config_req.Lchan_desc[1],(LCHAN_DESC*)&DTCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
  Mac_config_req.Lchan_id.Index=(UE_index << RAB_SHIFT2) + DTCH_BD + 2;
  Idx = Mac_rlc_xface->mac_config_req(0,ADD_LC,&Mac_config_req);
  Mac_rlc_xface->rrc_rlc_config_req(0,ACTION_ADD,Idx,SIGNALLING_RADIO_BEARER,Rlc_info_um);

// Configure DTCH (SRB2)
  Mac_config_req.Lchan_type = DTCH;
  Mac_config_req.UE_CH_index = UE_index;
  memcpy(&Mac_config_req.Lchan_desc[0],(LCHAN_DESC*)&DTCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx 
  memcpy(&Mac_config_req.Lchan_desc[1],(LCHAN_DESC*)&DTCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
  Mac_config_req.Lchan_id.Index=(UE_index << RAB_SHIFT2) + DTCH_BD + 3;
  Idx = Mac_rlc_xface->mac_config_req(0,ADD_LC,&Mac_config_req);
  Mac_rlc_xface->rrc_rlc_config_req(0,ACTION_ADD,Idx,SIGNALLING_RADIO_BEARER,Rlc_info_am);

// Configure DTCH (SRB3)
  Mac_config_req.Lchan_type = DTCH;
  Mac_config_req.UE_CH_index = UE_index;
  memcpy(&Mac_config_req.Lchan_desc[0],(LCHAN_DESC*)&DTCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx 
  memcpy(&Mac_config_req.Lchan_desc[1],(LCHAN_DESC*)&DTCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
  Mac_config_req.Lchan_id.Index=(UE_index << RAB_SHIFT2) + DTCH_BD + 4;
  Idx = Mac_rlc_xface->mac_config_req(0,ADD_LC,&Mac_config_req);
  Mac_rlc_xface->rrc_rlc_config_req(0,ACTION_ADD,Idx,SIGNALLING_RADIO_BEARER,Rlc_info_am);

  */

}

/*------------------------------------------------------------------------------*/
// Send config to the MAC Layer
void rrc_init_mac_default_param(void){
/*------------------------------------------------------------------------------*/
  // The following code has been commented on 23/03/2012
  printk("\n rrc_init_mac_default_param -- COMMENTED\n");

  /*
  BCCH_LCHAN_DESC.transport_block_size=BCCH_PAYLOAD_SIZE_MAX;
  BCCH_LCHAN_DESC.max_transport_blocks=15;//MAX 16 (4bits for Seq_id)

  CCCH_LCHAN_DESC.transport_block_size=CCCH_PAYLOAD_SIZE_MAX;
  CCCH_LCHAN_DESC.max_transport_blocks=15;//MAX 16 (4bits for Seq_id)

//   BCCH_LCHAN_DESC.transport_block_size=30;//+CH_BCCH_HEADER_SIZE;
//   BCCH_LCHAN_DESC.max_transport_blocks=15;//MAX 16 (4bits for Seq_id)
//   CCCH_LCHAN_DESC.transport_block_size=30;//+CH_CCCH_HEADER_SIZE;
//   CCCH_LCHAN_DESC.max_transport_blocks=15;//MAX 16 (4bits for Seq_id)
   DCCH_LCHAN_DESC.transport_block_size=40;//+CH_BCCH_HEADER_SIZE;
   DCCH_LCHAN_DESC.max_transport_blocks=20;
   DTCH_LCHAN_DESC.transport_block_size=40;//120;//200;//+CH_BCCH_HEADER_SIZE;
   DTCH_LCHAN_DESC.max_transport_blocks=20;

*/

  // Test config on 02/04/2012
  BCCH_LCHAN_DESC.transport_block_size=BCCH_PAYLOAD_SIZE_MAX;
  BCCH_LCHAN_DESC.max_transport_blocks=15;//MAX 16 (4bits for Seq_id)


  // Config copied from RRC LITE on 02/04/2012
  Rlc_info_um.rlc_mode=RLC_UM;
  Rlc_info_um.rlc.rlc_um_info.timer_reordering=0;
  Rlc_info_um.rlc.rlc_um_info.sn_field_length=10;
  Rlc_info_um.rlc.rlc_um_info.is_mXch=0;
  //Rlc_info_um.rlc.rlc_um_info.sdu_discard_mode=16;

  Rlc_info_am_config.rlc_mode=RLC_AM;
  Rlc_info_am_config.rlc.rlc_am_info.max_retx_threshold = 255;
  Rlc_info_am_config.rlc.rlc_am_info.poll_pdu           = 8;
  Rlc_info_am_config.rlc.rlc_am_info.poll_byte          = 1000;
  Rlc_info_am_config.rlc.rlc_am_info.t_poll_retransmit  = 15;
  Rlc_info_am_config.rlc.rlc_am_info.t_reordering       = 5000;
  Rlc_info_am_config.rlc.rlc_am_info.t_status_prohibit  = 10;


}

/*------------------------------------------------------------------------------*/
//Entry function for RRC init - Copied from RRC MESH (MW 09/09/2008)
int rrc_init_global_param(void){
  /*------------------------------------------------------------------------------*/
  //  Nb_mod=0;
  //#ifdef DEBUG_RRC_STATE
   msg ("\n[RRC CELL] Called rrc_init_global_param - Begin \n\n");
  //#endif

 /*
#ifdef USER_MODE
  Rrc_xface = (RRC_XFACE*)malloc16(sizeof(RRC_XFACE));
#endif
  Rrc_xface->openair_rrc_top_init = rrc_rg_toplite_init;
  Rrc_xface->openair_rrc_eNB_init = rrc_rg_init;
  Rrc_xface->openair_rrc_UE_init = rrc_rg_uelite_init;
  Rrc_xface->mac_rrc_data_ind = mac_rrc_data_ind;
  Rrc_xface->mac_rrc_data_req = mac_rrc_data_req;
  Rrc_xface->rrc_data_indP    = rlcrrc_data_ind;
  Rrc_xface->rrc_rx_tx        = rrc_rg_main_scheduler;
  Rrc_xface->mac_rrc_meas_ind = mac_rrc_meas_ind;
  Rrc_xface->def_meas_ind     = rrc_L2_def_meas_ind_rx;
  Mac_rlc_xface->mac_out_of_sync_ind = mac_out_of_sync_ind;
  Rrc_xface->get_rrc_status= rrc_L2_get_rrc_status;
  printk("[RRC]INIT_GLOBAL_PARAM: Mac_rlc_xface %p, rrc_rlc_register %p,rlcrrc_data_ind %p\n",Mac_rlc_xface,Mac_rlc_xface->rrc_rlc_register_rrc,rlcrrc_data_ind);
  if (Mac_rlc_xface==NULL || Mac_rlc_xface->rrc_rlc_register_rrc==NULL||rlcrrc_data_ind==NULL)
    return -1;
  //register with rlc -1st function= data_ind/srb_rx, 2nd function = srb_confirm
  Mac_rlc_xface->rrc_rlc_register_rrc(rlcrrc_data_ind , rrc_L2_rlc_confirm_ind_rx); 
  */

  //register with rlc -1st function= data_ind/srb_rx, 2nd function = srb_confirm
  printk("[RRC]INIT_GLOBAL_PARAM: rrc_rlc_register_rrc %p,rlcrrc_data_ind %p, rrc_L2_rlc_confirm_ind_rx %p\n", rrc_rlc_register_rrc, rlcrrc_data_ind, rrc_L2_rlc_confirm_ind_rx );
  if( rrc_rlc_register_rrc==NULL||rlcrrc_data_ind==NULL|| rrc_L2_rlc_confirm_ind_rx==NULL)
    return -1;

  rrc_rlc_register_rrc(rlcrrc_data_ind , rrc_L2_rlc_confirm_ind_rx); 

  rrc_init_mac_default_param();

  return 0;
}



