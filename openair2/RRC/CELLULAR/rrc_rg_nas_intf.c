/***************************************************************************
                          rrc_rg_nas_intf.c  -
                          -------------------
    begin                : Tue Jan 15 2002
    copyright            : (C) 2002, 2010 by Eurecom
    created by           : michelle.wetterwald@eurecom.fr
 **************************************************************************
      This file contains the functions used to interface the NAS
 ***************************************************************************/
/********************
//OpenAir definitions
 ********************/
#include "LAYER2/MAC/extern.h"
#include "UTIL/MEM/mem_block.h"
#include "rtos_header.h"

/********************
// RRC definitions
 ********************/
#include "rrc_rg_vars.h"
#include "rrc_nas_sap.h"
#include "rrc_messages.h"
//-----------------------------------------------------------------------------
#include "rrc_proto_int.h"
//-----------------------------------------------------------------------------
// For FIFOS interface
#ifdef USER_MODE
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#ifdef USER_MODE
//-----------------------------------------------------------------------------
// Create and initialize FIFOs for UE RRC SAPs
void rrc_rg_sap_init (void){
//-----------------------------------------------------------------------------
  int write_flag = O_WRONLY | O_NONBLOCK | O_NDELAY;
  int read_flag = O_RDONLY | O_NONBLOCK | O_NDELAY;

  // Create FIFOs
  rrc_create_fifo (RRC_SAPI_RG_GCSAP);
  rrc_create_fifo (RRC_SAPI_RG_NTSAP);
  rrc_create_fifo (RRC_SAPI_RG_DCSAP0_IN);
  rrc_create_fifo (RRC_SAPI_RG_DCSAP0_OUT);
  rrc_create_fifo (RRC_SAPI_RG_DCSAP1_IN);
  rrc_create_fifo (RRC_SAPI_RG_DCSAP1_OUT);
  rrc_create_fifo (RRC_SAPI_RG_DCSAP2_IN);
  rrc_create_fifo (RRC_SAPI_RG_DCSAP2_OUT);

  // Open FIFOs
  while ((protocol_bs->rrc.rrc_rg_GC_fifo = open (RRC_SAPI_RG_GCSAP, read_flag)) < 0) {
    msg ("%s returned value %d\n", RRC_SAPI_RG_GCSAP, protocol_bs->rrc.rrc_rg_GC_fifo);
    perror("RRC_SAPI_RG_GCSAP - open failed: ");
    sleep (1);
  }
  msg("FIFO opened %s\n", RRC_SAPI_RG_GCSAP);

  while ((protocol_bs->rrc.rrc_rg_NT_fifo = open (RRC_SAPI_RG_NTSAP, read_flag)) < 0) {
    msg ("%s returned value %d\n", RRC_SAPI_RG_NTSAP, protocol_bs->rrc.rrc_rg_NT_fifo);
    sleep (1);
  }
  msg("FIFO opened %s\n", RRC_SAPI_RG_NTSAP);

// Currently 3 MT supported -- will become a loop ???
  while ((protocol_bs->rrc.rrc_rg_DCIn_fifo[0] = open (RRC_SAPI_RG_DCSAP0_IN, read_flag)) < 0) {
    msg ("%s returned value %d\n", RRC_SAPI_RG_DCSAP0_IN, protocol_bs->rrc.rrc_rg_DCIn_fifo[0]);
    sleep (1);
  }
  msg("FIFO opened %s\n", RRC_SAPI_RG_DCSAP0_IN);

  while ((protocol_bs->rrc.rrc_rg_DCOut_fifo[0] = open (RRC_SAPI_RG_DCSAP0_OUT, write_flag)) < 0) {
    msg ("%s returned value %d\n", RRC_SAPI_RG_DCSAP0_OUT, protocol_bs->rrc.rrc_rg_DCOut_fifo[0]);
    perror("RRC_SAPI_RG_DCSAP0_OUT - open failed: ");
    sleep (1);
  }
  msg("FIFO opened %s\n", RRC_SAPI_RG_DCSAP0_OUT);

  while ((protocol_bs->rrc.rrc_rg_DCIn_fifo[1] = open (RRC_SAPI_RG_DCSAP1_IN, read_flag)) < 0) {
    msg ("%s returned value %d\n", RRC_SAPI_RG_DCSAP1_IN, protocol_bs->rrc.rrc_rg_DCIn_fifo[1]);
    sleep (1);
  }
  msg("FIFO opened %s\n", RRC_SAPI_RG_DCSAP1_IN);

  while ((protocol_bs->rrc.rrc_rg_DCOut_fifo[1] = open (RRC_SAPI_RG_DCSAP1_OUT, write_flag)) < 0) {
    msg ("%s returned value %d\n", RRC_SAPI_RG_DCSAP1_OUT, protocol_bs->rrc.rrc_rg_DCOut_fifo[1]);
    sleep (1);
  }
  msg("FIFO opened %s\n", RRC_SAPI_RG_DCSAP1_OUT);

  while ((protocol_bs->rrc.rrc_rg_DCIn_fifo[2] = open (RRC_SAPI_RG_DCSAP2_IN, read_flag)) < 0) {
    msg ("%s returned value %d\n", RRC_SAPI_RG_DCSAP2_IN, protocol_bs->rrc.rrc_rg_DCIn_fifo[2]);
    sleep (1);
  }
  msg("FIFO opened %s\n", RRC_SAPI_RG_DCSAP2_IN);

  while ((protocol_bs->rrc.rrc_rg_DCOut_fifo[2] = open (RRC_SAPI_RG_DCSAP2_OUT, write_flag)) < 0) {
    msg ("%s returned value %d\n", RRC_SAPI_RG_DCSAP2_OUT, protocol_bs->rrc.rrc_rg_DCOut_fifo[2]);
    sleep (1);
  }
  msg("FIFO opened %s\n", RRC_SAPI_RG_DCSAP2_OUT);

  // Print result
  msg ("%s returned value %d\n", "RRC_SAPI_RG_GCSAP", protocol_bs->rrc.rrc_rg_GC_fifo);
  msg ("%s returned value %d\n", "RRC_SAPI_RG_NTSAP", protocol_bs->rrc.rrc_rg_NT_fifo);
  msg ("%s returned value %d\n", "RRC_SAPI_RG_DCSAP0_IN", protocol_bs->rrc.rrc_rg_DCIn_fifo[0]);
  msg ("%s returned value %d\n", "RRC_SAPI_RG_DCSAP0_OUT", protocol_bs->rrc.rrc_rg_DCOut_fifo[0]);
  msg ("%s returned value %d\n", "RRC_SAPI_RG_DCSAP1_IN", protocol_bs->rrc.rrc_rg_DCIn_fifo[1]);
  msg ("%s returned value %d\n", "RRC_SAPI_RG_DCSAP1_OUT", protocol_bs->rrc.rrc_rg_DCOut_fifo[1]);
  msg ("%s returned value %d\n", "RRC_SAPI_RG_DCSAP2_IN", protocol_bs->rrc.rrc_rg_DCIn_fifo[2]);
  msg ("%s returned value %d\n", "RRC_SAPI_RG_DCSAP2_OUT", protocol_bs->rrc.rrc_rg_DCOut_fifo[2]);
}
#endif

//-----------------------------------------------------------------------------
// This function sends data from RRC to the NAS
void rrc_rg_write_FIFO (mem_block_t *p){
//-----------------------------------------------------------------------------
  int count = 0;
  int xmit_length;
//  int message_type;
  char *xmit_ptr;

  // transmit the primitive
  #ifdef DEBUG_RRC_STATE
    msg ("[RRC_DEBUG] Message to transmit in DC FIFO\n");
  #endif
  xmit_length = ((struct nas_rg_if_element *) p->data)->prim_length;
  xmit_ptr = (char *) &((struct nas_rg_if_element *) p->data)->nasRgPrimitive;
  if (xmit_ptr != NULL) {
    count = rtf_put (((struct nas_rg_if_element *) p->data)->xmit_fifo, xmit_ptr, xmit_length);
  }else{
    count = 0;
  }
  if (count == xmit_length) {
    #ifdef DEBUG_RRC_DETAILS
     msg ("[RRC_RG][NAS] NAS primitive sent successfully, length %d \n", count);
     msg("\n[RRC_RG][NAS] on FIFO, %d \n", ((struct nas_rg_if_element *) p->data)->xmit_fifo);
    #endif
    protocol_bs->rrc.NASMessageToXmit = p->next;        //Dequeue next message if any
    free_mem_block (p);
    #ifndef USER_MODE
    if (protocol_bs->rrc.ip_rx_irq > 0) {
      rt_pend_linux_srq (protocol_bs->rrc.ip_rx_irq);
    }else{
      msg ("[RRC_RG] ERROR IF IP STACK WANTED NOTIF PACKET(S) ip_rx_irq not initialized\n");
    }
    #endif
  }else{
    msg ("[RRC_RG][NAS] transmission on FIFO failed, %d bytes sent\n", count);
  }
}

//-----------------------------------------------------------------------------
// Enqueue a message for NAS
void rrc_rg_nas_xmit_enqueue (mem_block_t * p){
//-----------------------------------------------------------------------------
  protocol_bs->rrc.NASMessageToXmit = p;
}

