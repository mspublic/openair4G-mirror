/***************************************************************************
                          rrc_ue_main.c  -
                          -------------------
    begin                : Tue Jan 15 2002
    copyright            : (C) 2002, 2008 by Eurecom
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
//#include "rrc_sap.h"
/*
#include "rrc_messages.h"
#include "rrc_proto_extern.h"
#include "rrc_fsm_proto_extern.h"
#include "rrc_rrm_proto.h"
#include "rrc_bch_proto_extern.h"
#include "rrc_mbms_proto.h"
#include "rrc_control_proto.h"

#include "umts_timer_proto_extern.h"
*/

//-----------------------------------------------------------------------------
// This function sends data from RRC to the NAS
void rrc_ue_write_FIFO (mem_block * p){
//-----------------------------------------------------------------------------
/*  int             count = 0;
  int             xmit_length;
//  int message_type;
  char           *xmit_ptr;

  // transmit the primitive

  xmit_length = ((struct nas_ue_if_element *) p->data)->prim_length;
  xmit_ptr = (char *) &((struct nas_ue_if_element *) p->data)->nasUePrimitive;
  count = rtf_put (((struct nas_ue_if_element *) p->data)->xmit_fifo, xmit_ptr, xmit_length);

  if (count == xmit_length) {
#ifdef DEBUG_RRC_STATE
    //msg ("[RRC_UE][NAS] NAS primitive sent successfully, length %d \n", count);
    //msg("\n[RRC_UE][NAS] on FIFO, %d \n", ((struct nas_ue_if_element *) p->data)->xmit_fifo);
#endif
    protocol_ms->rrc.NASMessageToXmit = p->next;        //Dequeue next message if any
    free_mem_block (p);
#ifndef USER_MODE
    if ((&protocol_ms->rb_dispatch)->ip_rx_irq > 0) {   //Temp - later a specific control irq
      rt_pend_linux_srq ((&protocol_ms->rb_dispatch)->ip_rx_irq);
    } else {
#ifdef DEBUG_RRC_STATE
      msg ("[RRC_UE] ERROR IF IP STACK WANTED NOTIF PACKET(S) ip_rx_irq not initialized\n");
#endif
    }
#endif
  } else {
#ifdef DEBUG_RRC_STATE
    msg ("[RRC_UE][NAS] transmission on FIFO failed, %d bytes sent\n", count);
#endif
  }
*/
}

//-----------------------------------------------------------------------------
// entry point for rrc-ue process
void rrc_ue_main_scheduler (u8 Mod_id){
//-----------------------------------------------------------------------------
  mem_block *p;
  int Message_Id;

/*
#ifdef ALLOW_MBMS_PROTOCOL
    //HNN: ATTENTION: This must be the first event of RRC process.
    //     Used to find the beginning of the modification period.
    rrc_ue_mbms_scheduling_check();
#endif

  if (rrc_release_all_ressources) {
#ifdef DEBUG_RRC_STATE
    msg ("[RRC_UE]rrc_ue_rxtx : release_radio_resources() \n");
#endif
    mac_remove_all ();
    rb_remove_all ();
    // Set RRM Functions to remove RBs, TrChs, CCTrChs
    // Put UE in Cell-BCH mode
#ifndef BYPASS_L1
    CPHY_release_UE_resources ();
#endif
    // BYPASS_L1
    rrc_release_all_ressources = 0;
  }
  // check L1
  rrc_ue_L1_check ();

#ifndef BYPASS_L1
  if (protocol_ms->rrc.protocol_state == RRC_UE_IDLE) {
    if ((frame % 800) == 0)
      msg ("[RRC][IDLE] frame %d \n", frame);
  }

  if (protocol_ms->rrc.protocol_state != RRC_UE_IDLE) {
    rrc_ue_tick += 1;
    if ((frame % 800) == 0) {
      msg ("[RRC][KEEP-CX-ALIVE] TICK %d, at frame %d \n", rrc_ue_tick, frame);
    }
  }
#endif

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
  umts_timer_check_time_out (&protocol_ms->rrc.rrc_timers, protocol_ms->frame_tick_milliseconds);
  // Measurements
  //rrc_ue_meas_loop();   // for test only
*/
  protocol_ms->rrc.current_SFN = Mac_rlc_xface->frame;
//  if (protocol_ms->rrc.current_SFN % 50 == 0) {
#ifdef DEBUG_RRC_DETAILS
    msg ("\n[RRC][MSG_TEST] System Measurement Time : %d\n", protocol_ms->rrc.current_SFN);
#endif
//  }
/*
  //check if report of measure needed in UE
  rrc_ue_sync_measures (protocol_ms->rrc.current_SFN, &Message_Id);
*/
}

