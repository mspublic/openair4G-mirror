/***************************************************************************
                          rrc_ue_init.c
                          -------------------
    begin                : Someday 2008
    copyright            : (C) 2008 by Eurecom
    created by           : Michelle.Wetterwald@eurecom.fr
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
#include "rrc_ue_vars.h"
#include "rrc_L2_proto.h"
//-----------------------------------------------------------------------------
//#include "rrc_broadcast_proto_extern.h"
//#include "rrc_paging_proto_extern.h"
//#include "rrc_proto_extern.h"
//#include "rrc_srb_proto_extern.h"
//#include "rrc_sap.h"
//#include "rrc_bch_proto_extern.h"
//#include "rrc_mbms_proto.h"
//#include "rrc_ue_data.h"

int *pt_nas_ue_irq;
u8  nas_IMEI[14];
#ifndef  USER_MODE
EXPORT_SYMBOL(pt_nas_ue_irq);
EXPORT_SYMBOL(nas_IMEI);
#endif

//-----------------------------------------------------------------------------
void rrc_ue_init (u8 Mod_id){
//-----------------------------------------------------------------------------
//  int i;
  printk("[RRC CELL][INIT] Init UE function start\n");
  pool_buffer_init();


  protocol_ms = &prot_pool_ms;
  memset ((char *)protocol_ms, 0, sizeof (struct protocol_pool_ms));


  rrc_release_all_ressources = 0;

  //protocol_ms->frame_tick_milliseconds = 80;
  protocol_ms->rrc.ue_wait_establish_req = 0;
  protocol_ms->rrc.protocol_state = RRC_UE_IDLE;
  protocol_ms->rrc.u_rnti = 0;
  protocol_ms->rrc.cell_id = 0;
  printk("[RRC CELL][INIT] cell_id %d\n",protocol_ms->rrc.cell_id );
  //init_dbl_lk_up (&protocol_ms->rrc.rrc_timers, NULL);

  //init_dbl_lk (&prot->sched_rb_tx, NULL);

  /*
  rrc_mt_set_broadcast ();
  rrc_mt_set_paging ();
  rrc_mt_set_ccch_idle_mode ();
  rrc_mt_set_srb0 ();
  rrc_ue_bch_init ();
  rrc_rb_ue_init ();
  // initialise IRQ in case of Moby Dick
  pt_nas_ue_irq = &((&protocol_ms->rb_dispatch)->ip_rx_irq);
  protocol_ms->rrc.ue_initial_id = rrc_ue_get_initial_id ();
  memcpy (nas_IMEI, protocol_ms->rrc.IMEI, 14);
  rrc_ue_fsm_init (protocol_ms->rrc.ue_initial_id);
  protocol_ms->rrc.rrc_currently_updating = FALSE;

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
  rrc_ue_sap_init ();           // init FIFOs towards NAS
  qos_fifo_open ();
#endif
#endif
  //Initialise MBMS
  rrc_ue_mbms_init();
  */
  printk("[RRC CELL][INIT] Init UE function completed\n");

  //return NULL;
}

/*------------------------------------------------------------------------------*/
// Dummy function - to keep compatibility with RRC LITE
char rrc_ue_rglite_init(u8 Mod_id, unsigned char eNB_index){
/*------------------------------------------------------------------------------*/
  //#ifdef DEBUG_RRC_STATE
   msg ("\n[RRC CELL] Called rrc_ue_rglite_init - Dummy function - to keep compatibility with RRC LITE\n\n");
  //#endif
  return 0;
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
#endif //USER_MODE


  Rrc_xface->openair_rrc_top_init = rrc_ue_toplite_init;
  Rrc_xface->openair_rrc_eNB_init = rrc_ue_rglite_init;
  Rrc_xface->openair_rrc_UE_init = rrc_ue_init;
  Rrc_xface->mac_rrc_data_ind = mac_rrc_data_ind;
  Rrc_xface->mac_rrc_data_req = mac_rrc_data_req;
  Rrc_xface->rrc_data_indP    = rlcrrc_data_ind;
  Rrc_xface->rrc_rx_tx        = rrc_ue_main_scheduler;
  Rrc_xface->mac_rrc_meas_ind = mac_rrc_meas_ind;
  Rrc_xface->def_meas_ind     = rrc_L2_def_meas_ind_rx;
  Mac_rlc_xface->mac_out_of_sync_ind = mac_out_of_sync_ind;
  if(Mac_rlc_xface==NULL || Mac_rlc_xface->rrc_rlc_register_rrc==NULL||rlcrrc_data_ind==NULL)
    return -1;
  printk("[RRC]INIT_GLOBAL_PARAM: Mac_rlc_xface %p, rrc_rlc_register %p,rlcrrc_data_ind %p\n",Mac_rlc_xface,Mac_rlc_xface->rrc_rlc_register_rrc,rlcrrc_data_ind);
  Mac_rlc_xface->rrc_rlc_register_rrc(rlcrrc_data_ind ,NULL); //register with rlc
*/
  printk("[RRC]INIT_GLOBAL_PARAM: rrc_rlc_register_rrc %p,rlcrrc_data_ind %p\n",rrc_rlc_register_rrc,rlcrrc_data_ind);
  if( rrc_rlc_register_rrc==NULL||rlcrrc_data_ind==NULL)
    return -1;
  rrc_rlc_register_rrc(rlcrrc_data_ind ,NULL); //register with rlc

//   BCCH_LCHAN_DESC.transport_block_size=30;//+CH_BCCH_HEADER_SIZE;
//   BCCH_LCHAN_DESC.max_transport_blocks=15;//MAX 16 (4bits for Seq_id)
//   CCCH_LCHAN_DESC.transport_block_size=30;//+CH_CCCH_HEADER_SIZE;
//   CCCH_LCHAN_DESC.max_transport_blocks=15;//MAX 16 (4bits for Seq_id)
//   DCCH_LCHAN_DESC.transport_block_size=40;//+CH_BCCH_HEADER_SIZE;
//   DCCH_LCHAN_DESC.max_transport_blocks=20;
//   DTCH_LCHAN_DESC.transport_block_size=40;//120;//200;//+CH_BCCH_HEADER_SIZE;
//   DTCH_LCHAN_DESC.max_transport_blocks=20;
// 
//   Rlc_info_um.rlc_mode=RLC_UM;
//   Rlc_info_um.rlc.rlc_um_info.timer_discard=0;
//   Rlc_info_um.rlc.rlc_um_info.sdu_discard_mode=16;
// 
//   Rlc_info_am.rlc_mode=RLC_AM;	
//   Rlc_info_am.rlc.rlc_am_info.sdu_discard_mode      = SDU_DISCARD_MODE_RESET;
//   Rlc_info_am.rlc.rlc_am_info.timer_poll            = 0;
//   Rlc_info_am.rlc.rlc_am_info.timer_poll_prohibit   = 0;
//   Rlc_info_am.rlc.rlc_am_info.timer_discard         = 1000*10;
//   Rlc_info_am.rlc.rlc_am_info.timer_poll_periodic   = 0;
//   Rlc_info_am.rlc.rlc_am_info.timer_status_prohibit = 0;
//   Rlc_info_am.rlc.rlc_am_info.timer_status_periodic = 250*10;
//   Rlc_info_am.rlc.rlc_am_info.timer_rst             = 100*10;
//   Rlc_info_am.rlc.rlc_am_info.max_rst               = 8;
//   Rlc_info_am.rlc.rlc_am_info.timer_mrw             = 60*10;
// 
//   Rlc_info_am.rlc.rlc_am_info.pdu_size              = 320; // in bits
//   //Rlc_info_am.rlc.rlc_am_info.in_sequence_delivery  = 1;//boolean
//   Rlc_info_am.rlc.rlc_am_info.max_dat               = 63;
//   Rlc_info_am.rlc.rlc_am_info.poll_pdu              = 16;
//   Rlc_info_am.rlc.rlc_am_info.poll_sdu              = 1;
//   Rlc_info_am.rlc.rlc_am_info.poll_window           = 50;
//   Rlc_info_am.rlc.rlc_am_info.tx_window_size        = 128;
//   Rlc_info_am.rlc.rlc_am_info.rx_window_size        = 128;
//   Rlc_info_am.rlc.rlc_am_info.max_mrw               = 8;
// 	
//   Rlc_info_am.rlc.rlc_am_info.last_transmission_pdu_poll_trigger   = 1;//boolean
//   Rlc_info_am.rlc.rlc_am_info.last_retransmission_pdu_poll_trigger = 1;//boolean
//   Rlc_info_am.rlc.rlc_am_info.send_mrw              = 0;//boolean*

  return 0;
}

