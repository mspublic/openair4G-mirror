/***************************************************************************
                          rrc_ue_main.c  -
                          -------------------
    begin                : Tue Jan 15 2002
    copyright            : (C) 2002, 2010 by Eurecom
    created by           : michelle.wetterwald@eurecom.fr
 **************************************************************************
		This file contains the main function of the RRC module
 ***************************************************************************/
//#include "rtos_header.h"
//#include "platform.h"
//#include "protocol_vars_extern.h"
//#include "print.h"
//-----------------------------------------------------------------------------
/********************
//OpenAir definitions
 ********************/
#include "LAYER2/MAC/extern.h"
#include "UTIL/MEM/mem_block.h"
//-----------------------------------------------------------------------------
/********************
// RRC definitions
 ********************/
#include "rrc_ue_vars.h"
//#include "rrc_nas_sap.h"
//#include "rrc_messages.h"
//-----------------------------------------------------------------------------
#include "rrc_proto_int.h"
//#include "rrc_proto_fsm.h"
//#include "rrc_proto_intf.h"
//#include "rrc_proto_bch.h"
#include "rrc_proto_mbms.h"

#include "umts_timer_proto_extern.h"

//-----------------------------------------------------------------------------
// entry point for rrc-ue process
//void rrc_ue_main_scheduler (u8 Mod_id){
int rrc_ue_main_scheduler(u8 Mod_id,u32 frame, u8 eNB_flag,u8 index){
//-----------------------------------------------------------------------------
  mem_block_t *p;
  int Message_Id;

//     while(1)
//     {sleep(2);}

//  protocol_ms->rrc.current_SFN = Mac_rlc_xface->frame;
  protocol_ms->rrc.current_SFN = frame;
  /*************/
  // TODO TO BE REMOVED TEMP -- stop the loop 
  if (protocol_ms->rrc.current_SFN > 50000)
   exit(1);
  //if (Mac_rlc_xface->frame < 100)
  //s return;
  if (protocol_ms->rrc.current_SFN == 100)
   msg("\n\n[RRC] [TEMP-OPENAIR-DEBUG] RRC-UE resuming its operation at frame %d\n\n ", protocol_ms->rrc.current_SFN);

  /*************/

  #ifdef DEBUG_RRC_DETAILS
  if (protocol_ms->rrc.current_SFN % 5 == 0) {
     msg ("\n\n[RRC][MSG_TEST] System Time : %d\n", protocol_ms->rrc.current_SFN);
  }
  #endif

  #ifdef ALLOW_MBMS_PROTOCOL
    //     ATTENTION: This must be the first event of RRC process.
    //     Used to find the beginning of the modification period.
    rrc_ue_mbms_scheduling_check();
  #endif

  if (rrc_release_all_ressources) {
  #ifdef DEBUG_RRC_STATE
    msg ("[RRC_UE]rrc_ue_rxtx : release_radio_resources() \n");
  #endif
    //mac_remove_all ();
    //rb_remove_all ();
    // Set RRM Functions to remove RBs, TrChs, CCTrChs
    // Put UE in Cell-BCH mode
    /* BYPASS_L1 */

    rrc_release_all_ressources = 0;
  }
  // check L1
  rrc_ue_L1_check ();


  // check if there is some message to transmit to NAS and do it
  if ((p = protocol_ms->rrc.NASMessageToXmit) != NULL) {
    rrc_ue_write_FIFO (p);
  } else {
    if (protocol_ms->rrc.ue_broadcast_counter % 500 == 2) {
      RRC_UE_O_NAS_MEASUREMENT_IND ();
      rrc_ue_write_FIFO (protocol_ms->rrc.NASMessageToXmit);
    }
    // modulo to improve stability
    protocol_ms->rrc.ue_broadcast_counter = (protocol_ms->rrc.ue_broadcast_counter++) % 1000000000;
  }

  // time out for SIB14 - cf RG
  if (protocol_ms->rrc.ue_broadcast_counter % (protocol_ms->rrc.ue_bch_blocks.SIB14_timeout_value) == 2) {
    protocol_ms->rrc.ue_bch_blocks.SIB14_timeout = TRUE;
  }
  // Wait for message in DC FIFO
  rrc_ue_read_DCin_FIFO ();

  // check for a time-out event
  // umts_timer_check_time_out (&protocol_ms->rrc.rrc_timers, protocol_ms->frame_tick_milliseconds);
  umts_timer_check_time_out (&protocol_ms->rrc.rrc_timers, Mac_rlc_xface->frame/RRC_FRAME_DURATION);

  // Measurements
  //rrc_ue_meas_loop();   // for test only
  //check if report of measure needed in UE
  rrc_ue_sync_measures (protocol_ms->rrc.current_SFN, &Message_Id);

  //Force Uplink RLC communication
  rrc_ue_force_uplink ();
  // Test RLC interface
  //rrc_ue_test_lchannels ();
  return 0;
}
