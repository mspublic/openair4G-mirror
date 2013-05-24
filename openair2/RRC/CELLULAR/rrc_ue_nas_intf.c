/***************************************************************************
                          rrc_ue_nas_intf.c  -
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
#include "rrc_ue_vars.h"
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
extern int rrc_ue_mobileId;

//-----------------------------------------------------------------------------
// Create and initialize FIFOs for UE RRC SAPs
void rrc_ue_sap_init (void){
//-----------------------------------------------------------------------------
  int  write_flag = O_WRONLY | O_NONBLOCK | O_NDELAY;
  int  read_flag = O_RDONLY | O_NONBLOCK | O_NDELAY;
  char gcsap[40], ntsap[40], dcsap_in[40], dcsap_out[40];

  sprintf (gcsap, "%s%d", RRC_SAPI_UE_GCSAP, rrc_ue_mobileId);
  sprintf (ntsap, "%s%d", RRC_SAPI_UE_NTSAP, rrc_ue_mobileId);
  sprintf (dcsap_in, "%s%d", RRC_SAPI_UE_DCSAP_IN, rrc_ue_mobileId);
  sprintf (dcsap_out, "%s%d", RRC_SAPI_UE_DCSAP_OUT, rrc_ue_mobileId);

  // Create FIFOs
  rrc_create_fifo (gcsap);
  rrc_create_fifo (ntsap);
  rrc_create_fifo (dcsap_in);
  rrc_create_fifo (dcsap_out);

  // Open FIFOs
  while ((protocol_ms->rrc.rrc_ue_GC_fifo = open (gcsap, write_flag)) < 0) {
    msg ("%s returned value %d\n", gcsap, protocol_ms->rrc.rrc_ue_GC_fifo);
    sleep (1);
  }

  while ((protocol_ms->rrc.rrc_ue_NT_fifo = open (ntsap, write_flag)) < 0) {
    msg ("%s returned value %d\n", ntsap, protocol_ms->rrc.rrc_ue_NT_fifo);
    sleep (1);
  }

  while ((protocol_ms->rrc.rrc_ue_DCIn_fifo = open (dcsap_in, read_flag)) < 0) {
    msg ("%s returned value %d\n", dcsap_in, protocol_ms->rrc.rrc_ue_DCIn_fifo);
    sleep (1);
  }

  while ((protocol_ms->rrc.rrc_ue_DCOut_fifo = open (dcsap_out, write_flag)) < 0) {
    msg ("%s returned value %d\n", dcsap_out, protocol_ms->rrc.rrc_ue_DCOut_fifo);
    sleep (1);
  }

  // Print result
  msg ("[RRC] %s returned value %d\n", gcsap, protocol_ms->rrc.rrc_ue_GC_fifo);
  msg ("[RRC] %s returned value %d\n", ntsap, protocol_ms->rrc.rrc_ue_NT_fifo);
  msg ("[RRC] %s returned value %d\n", dcsap_in, protocol_ms->rrc.rrc_ue_DCIn_fifo);
  msg ("[RRC] %s returned value %d\n", dcsap_out, protocol_ms->rrc.rrc_ue_DCOut_fifo);
}
#endif

//-----------------------------------------------------------------------------
// This function sends data from RRC to the NAS
void rrc_ue_write_FIFO (mem_block_t * p){
//-----------------------------------------------------------------------------
  int count = 0;
  int xmit_length;
  char *xmit_ptr;

  // transmit the primitive
  xmit_length = ((struct nas_ue_if_element *) p->data)->prim_length;
  xmit_ptr = (char *) &((struct nas_ue_if_element *) p->data)->nasUePrimitive;
  count = rtf_put (((struct nas_ue_if_element *) p->data)->xmit_fifo, xmit_ptr, xmit_length);

  if (count == xmit_length) {
  #ifdef DEBUG_RRC_STATE
    msg ("[RRC_UE][NAS] NAS primitive sent successfully, length %d \n", count);
    //msg("\n[RRC_UE][NAS] on FIFO, %d \n", ((struct nas_ue_if_element *) p->data)->xmit_fifo);
  #endif
    protocol_ms->rrc.NASMessageToXmit = p->next;        //Dequeue next message if any
    free_mem_block (p);
  #ifndef USER_MODE
    if (protocol_ms->rrc.ip_rx_irq > 0) {   //Temp - later a specific control irq
      rt_pend_linux_srq (protocol_ms->rrc.ip_rx_irq);
    } else {
      msg ("[RRC_UE] ERROR IF IP STACK WANTED NOTIF PACKET(S) ip_rx_irq not initialized\n");
    }
  #endif
  } else {
    msg ("[RRC_UE][NAS] transmission on FIFO failed, %d bytes sent\n", count);
  }
}

//-----------------------------------------------------------------------------
// Enqueue a message for NAS
void rrc_ue_nas_xmit_enqueue (mem_block_t * p){
//-----------------------------------------------------------------------------
  protocol_ms->rrc.NASMessageToXmit = p;
}


