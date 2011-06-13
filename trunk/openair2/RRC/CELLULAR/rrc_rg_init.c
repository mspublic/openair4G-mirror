/***************************************************************************
                          rrc_rg_init.c
                          -------------------
    begin                : Someday 2008
    copyright            : (C) 2008 by Eurecom
		created by					 : Michelle.Wetterwald@eurecom.fr
 **************************************************************************
  Initialization of RRC protocol entity for User Equipment
 ***************************************************************************/
//#include "rtos_header.h"
//#include "platform.h"
//#include "protocol_vars_extern.h"
//#include "print.h"

/********************
// OpenAir includes
 ********************/
#include "LAYER2/MAC/extern.h"
#include "COMMON/openair_defs.h"
#include "RRC/L2_INTERFACE/openair_rrc_L2_interface.h"
#include "LAYER2/RLC/rlc.h"
#include "COMMON/mac_rrc_primitives.h"
  
//#include "SIMULATION/simulation_defs.h"
//extern EMULATION_VARS *Emul_vars;
//extern CH_MAC_INST *CH_mac_inst;
//extern UE_MAC_INST *UE_mac_inst;
//-----------------------------------------------------------------------------
 
/********************
// RRC includes
 ********************/
#include "rrc_rg_vars.h"
#include "rrc_L2_proto.h"
//-----------------------------------------------------------------------------
//#include "rrc_broadcast_proto_extern.h"
//#include "rrc_paging_proto_extern.h"
//#include "rrc_proto_extern.h"
//#include "rrc_srb_proto_extern.h"
//#include "rrc_sap.h"
//#include "rrc_bch_proto_extern.h"
//#include "rrc_mbms_proto.h"
//#include "rrc_rg_data.h"

int *pt_nas_rg_irq;
u8  nas_IMEI[14];
#ifndef  USER_MODE
EXPORT_SYMBOL(pt_nas_rg_irq);
EXPORT_SYMBOL(nas_IMEI);
#endif
LCHAN_DESC BCCH_LCHAN_DESC,CCCH_LCHAN_DESC;
//-----------------------------------------------------------------------------
void rrc_rg_init (u8 Mod_id){
//-----------------------------------------------------------------------------
//  int i;

/*
  init_uniform ();
#ifdef BYPASS_L1
  init_up (&trch_tx_L1H, NULL);
  init_list (&trch_rx_L1H, NULL);
#endif
*/
  printk("[RRC CELL][INIT] Init BS function start\n");
  pool_buffer_init();


  protocol_bs = &prot_pool_bs;
  memset ((char *)protocol_bs, 0, sizeof (struct protocol_pool_bs));


  rrc_release_all_ressources = 0;

  //protocol_bs->frame_tick_milliseconds = 80;
  protocol_bs->rrc.rg_wait_establish_req = 0;
  protocol_bs->rrc.protocol_state = RRC_RG_IDLE;
  protocol_bs->rrc.u_rnti = 0;
  protocol_bs->rrc.cell_id = 0;
  printk("[RRC CELL][INIT] cell_id %d\n",protocol_bs->rrc.cell_id );
  //init_dbl_lk_up (&protocol_bs->rrc.rrc_timers, NULL);

  //init_dbl_lk (&prot->sched_rb_tx, NULL);

  /*
  rrc_mt_set_broadcast ();
  rrc_mt_set_paging ();
  rrc_mt_set_ccch_idle_mode ();
  rrc_mt_set_srb0 ();
  rrc_rg_bch_init ();
  rrc_rb_rg_init ();
  // initialise IRQ in case of Moby Dick
  pt_nas_rg_irq = &((&protocol_bs->rb_dispatch)->ip_rx_irq);
  protocol_bs->rrc.ue_initial_id = rrc_rg_get_initial_id ();
  memcpy (nas_IMEI, protocol_bs->rrc.IMEI, 14);
  rrc_rg_fsm_init (protocol_bs->rrc.ue_initial_id);
  protocol_bs->rrc.rrc_currently_updating = FALSE;

  */
  /*
  rrm_config->outer_loop_vars.IBTS_averaged = -105;
  rrm_config->outer_loop_vars.PRACH_CNST = 3;
  rrm_config->outer_loop_vars.DPCH_CNST = 3;
  rrm_config->outer_loop_vars.alpha = 40;
  rrm_config->outer_loop_vars.L0 = 100;
  rrm_config->outer_loop_vars.RF_OFFSET = 10;
  rrm_config->outer_loop_vars.SIR_Target[0] = 10;

  rrm_config->prach.RG_RX_GAIN = 39 + 100;
  rrm_config->power_control_ul_received = 0;

  // Set IBTS values to default:  will be replaced by RRC using BCH
  for (i = 0; i < 15; i++)
    rrm_config->outer_loop_vars.IBTS[i] = -110;
  */

//  rrc_mt_set_mcch_rb();
  /*
#ifdef USER_MODE
#ifndef ONLY_L1
  rrc_rg_sap_init ();           // init FIFOs towards NAS
  qos_fifo_open ();
#endif
#endif
  //Initialise MBMS
  rrc_rg_mbms_init();
  */
  rrc_rg_init_mac_config();
  printk("[RRC CELL][INIT] Init RG function completed\n");

  //return NULL;
}

/*------------------------------------------------------------------------------*/
//Entry function for RRC init - Copied from RRC MESH (MW 09/09/2008)
int rrc_init_global_param(void){
  /*------------------------------------------------------------------------------*/
  //  Nb_mod=0;
#ifdef USER_MODE
  Rrc_xface = (RRC_XFACE*)malloc16(sizeof(RRC_XFACE));
#endif //USRE_MODE
  Rrc_xface->openair_rrc_eNB_init = rrc_rg_init;
  Rrc_xface->mac_rrc_data_ind = mac_rrc_data_ind;
  Rrc_xface->mac_rrc_data_req = mac_rrc_data_req;
  Rrc_xface->rrc_data_indP    = rlcrrc_data_ind;
  Rrc_xface->rrc_rx_tx        = rrc_rg_main_scheduler;
  Rrc_xface->mac_rrc_meas_ind = mac_rrc_meas_ind;
  Rrc_xface->def_meas_ind     = rrc_L2_def_meas_ind_rx;
  Mac_rlc_xface->mac_out_of_sync_ind = mac_out_of_sync_ind;
  printk("[RRC]INIT_GLOBAL_PARAM: Mac_rlc_xface %p, rrc_rlc_register %p,rlcrrc_data_ind %p\n",Mac_rlc_xface,Mac_rlc_xface->rrc_rlc_register_rrc,rlcrrc_data_ind);
  if(Mac_rlc_xface==NULL || Mac_rlc_xface->rrc_rlc_register_rrc==NULL||rlcrrc_data_ind==NULL)
    return -1;
  Mac_rlc_xface->rrc_rlc_register_rrc(rlcrrc_data_ind ,NULL); //register with rlc
  
  BCCH_LCHAN_DESC.transport_block_size=BCCH_PAYLOAD_SIZE_MAX;
  BCCH_LCHAN_DESC.max_transport_blocks=15;//MAX 16 (4bits for Seq_id)

  CCCH_LCHAN_DESC.transport_block_size=CCCH_PAYLOAD_SIZE_MAX;
  CCCH_LCHAN_DESC.max_transport_blocks=15;//MAX 16 (4bits for Seq_id)
/*
  DCCH_LCHAN_DESC.transport_block_size=40;//+CH_BCCH_HEADER_SIZE;
  DCCH_LCHAN_DESC.max_transport_blocks=20;
  DTCH_LCHAN_DESC.transport_block_size=40;//120;//200;//+CH_BCCH_HEADER_SIZE;
  DTCH_LCHAN_DESC.max_transport_blocks=20;
  
  Rlc_info_um.rlc_mode=RLC_UM;
  Rlc_info_um.rlc.rlc_um_info.timer_discard=0;
  Rlc_info_um.rlc.rlc_um_info.sdu_discard_mode=16;

  Rlc_info_am.rlc_mode=RLC_AM;	
  Rlc_info_am.rlc.rlc_am_info.sdu_discard_mode      = SDU_DISCARD_MODE_RESET;
  Rlc_info_am.rlc.rlc_am_info.timer_poll            = 0;
  Rlc_info_am.rlc.rlc_am_info.timer_poll_prohibit   = 0;
  Rlc_info_am.rlc.rlc_am_info.timer_discard         = 1000*10;
  Rlc_info_am.rlc.rlc_am_info.timer_poll_periodic   = 0;
  Rlc_info_am.rlc.rlc_am_info.timer_status_prohibit = 0;
  Rlc_info_am.rlc.rlc_am_info.timer_status_periodic = 250*10;
  Rlc_info_am.rlc.rlc_am_info.timer_rst             = 100*10;
  Rlc_info_am.rlc.rlc_am_info.max_rst               = 8;
  Rlc_info_am.rlc.rlc_am_info.timer_mrw             = 60*10;

  Rlc_info_am.rlc.rlc_am_info.pdu_size              = 320; // in bits
  //Rlc_info_am.rlc.rlc_am_info.in_sequence_delivery  = 1;//boolean
  Rlc_info_am.rlc.rlc_am_info.max_dat               = 63;
  Rlc_info_am.rlc.rlc_am_info.poll_pdu              = 16;
  Rlc_info_am.rlc.rlc_am_info.poll_sdu              = 1;
  Rlc_info_am.rlc.rlc_am_info.poll_window           = 50;
  Rlc_info_am.rlc.rlc_am_info.tx_window_size        = 128;
  Rlc_info_am.rlc.rlc_am_info.rx_window_size        = 128;
  Rlc_info_am.rlc.rlc_am_info.max_mrw               = 8;
	
  Rlc_info_am.rlc.rlc_am_info.last_transmission_pdu_poll_trigger   = 1;//boolean
  Rlc_info_am.rlc.rlc_am_info.last_retransmission_pdu_poll_trigger = 1;//boolean
  Rlc_info_am.rlc.rlc_am_info.send_mrw              = 0;//boolean*
  */
  return 0;
}

/*------------------------------------------------------------------------------*/
// Send config to the MAC Layer
void rrc_rg_init_mac_config(void){
/*------------------------------------------------------------------------------*/
  MAC_CONFIG_REQ Mac_config_req;

  // Configure BCCH
  //  Mac_config_req.Lchan_type = BCCH;
  //  memcpy(&Mac_config_req.Lchan_desc[0],(LCHAN_DESC*)&BCCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
  //  memcpy(&Mac_config_req.Lchan_desc[1],(LCHAN_DESC*)&BCCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
  //  Mac_config_req.UE_CH_index=0;
  //  Mac_config_req.Lchan_id.Index=(0 << RAB_SHIFT2) + BCCH;
  printk("Calling mac_config_req for BCCH\n");
  //  Mac_rlc_xface->mac_config_req(0,ADD_LC,&Mac_config_req);

  // Configure CCCH
  //  Mac_config_req.Lchan_type = CCCH;
  //  memcpy(&Mac_config_req.Lchan_desc[0],(LCHAN_DESC*)&CCCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
  //  memcpy(&Mac_config_req.Lchan_desc[1],(LCHAN_DESC*)&CCCH_LCHAN_DESC,LCHAN_DESC_SIZE); //0 rx, 1 tx
  //  Mac_config_req.UE_CH_index=1;
  //  Mac_config_req.Lchan_id.Index=(0 << RAB_SHIFT2) + CCCH;
  printk("Calling mac_config_req for CCCH\n");
  //  Mac_rlc_xface->mac_config_req(0,ADD_LC,&Mac_config_req);      
}
