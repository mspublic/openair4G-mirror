/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2011 Eurecom

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fsr/openairinterface
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/

/*! \file pdcp.c
* \brief pdcp interface with RLC
* \author  Lionel GAUTHIER and Navid Nikaein
* \date 2009-2012
* \version 1.0
*/

#define PDCP_C
#ifndef USER_MODE
  #include <rtai_fifos.h>
#endif
#include "pdcp.h"
#include "pdcp_util.h"
#include "pdcp_sequence_manager.h"
#include "LAYER2/RLC/rlc.h"
#include "LAYER2/MAC/extern.h"
#include "pdcp_primitives.h"
#include "OCG.h"
#include "OCG_extern.h"
#include "UTIL/LOG/log.h"
#include <inttypes.h>
#define PDCP_DATA_REQ_DEBUG 1
#define PDCP_DATA_IND_DEBUG 1

extern rlc_op_status_t rlc_data_req(module_id_t, u32_t, u8_t, rb_id_t, mui_t, confirm_t, sdu_size_t, mem_block_t*);
extern void rrc_lite_data_ind( u8 Mod_id, u32 frame, u8 eNB_flag, u32 Rb_id, u32 sdu_size,u8 *Buffer);
extern u8 mac_get_rrc_status(u8 Mod_id,u8 eNB_flag,u8 index);

//-----------------------------------------------------------------------------
/*
 * If PDCP_UNIT_TEST is set here then data flow between PDCP and RLC is broken
 * and PDCP has no longer anything to do with RLC. In this case, after it's handed
 * an SDU it appends PDCP header and returns (by filling in incoming pointer parameters)
 * this mem_block_t to be dissected for testing purposes. For further details see test
 * code at targets/TEST/PDCP/test_pdcp.c:test_pdcp_data_req()
 */
#ifdef PDCP_UNIT_TEST
BOOL pdcp_data_req(module_id_t module_id, u32_t frame, u8_t eNB_flag, rb_id_t rab_id, sdu_size_t sdu_buffer_size, \
                   unsigned char* sdu_buffer, pdcp_t* test_pdcp_entity, list_t* test_list)
#else
BOOL pdcp_data_req(module_id_t module_id, u32_t frame, u8_t eNB_flag, rb_id_t rab_id, u32 muiP, u32 confirmP, \
		   sdu_size_t sdu_buffer_size, unsigned char* sdu_buffer, u8 is_data_pdu)
#endif
{
//-----------------------------------------------------------------------------
#ifdef PDCP_UNIT_TEST
  pdcp_t* pdcp = test_pdcp_entity;
#else
  pdcp_t* pdcp = &pdcp_array[module_id][rab_id];
#endif
  u8 i;
  u8 pdcp_header_len=0, pdcp_tailer_len=0;
  u16 pdcp_pdu_size=0, current_sn;
  mem_block_t* pdcp_pdu = NULL;

  if (sdu_buffer_size == 0) {
    LOG_W(PDCP, "Handed SDU is of size 0! Ignoring...\n");
    return FALSE;
  }
  /*
   * XXX MAX_IP_PACKET_SIZE is 4096, shouldn't this be MAX SDU size, which is 8188 bytes?
   */

  if (sdu_buffer_size > MAX_IP_PACKET_SIZE) {
    LOG_E(PDCP, "Requested SDU size (%d) is bigger than that can be handled by PDCP!\n", sdu_buffer_size);
    // XXX What does following call do?
    mac_xface->macphy_exit("");
  }

  // calculate the pdcp header and trailer size
  if ((rab_id % MAX_NUM_RB) < DTCH) {
    pdcp_header_len = PDCP_CONTROL_PLANE_DATA_PDU_SN_SIZE;
    pdcp_tailer_len = PDCP_CONTROL_PLANE_DATA_PDU_MAC_I_SIZE;
  } else {
    pdcp_header_len = PDCP_USER_PLANE_DATA_PDU_LONG_SN_HEADER_SIZE;
    pdcp_tailer_len = 0;
  }
  pdcp_pdu_size= sdu_buffer_size + pdcp_header_len + pdcp_tailer_len;

  LOG_I(PDCP, "Data request notification for PDCP entity with module ID %d and radio bearer ID %d pdu size %d (header%d, trailer%d)\n", module_id, rab_id,pdcp_pdu_size, pdcp_header_len,pdcp_tailer_len);

  /*
   * Allocate a new block for the new PDU (i.e. PDU header and SDU payload)
   */
  LOG_D(PDCP, "Asking for a new mem_block of size %d\n", pdcp_pdu_size);
  pdcp_pdu = get_free_mem_block(pdcp_pdu_size);

  if (pdcp_pdu != NULL) {
    /*
     * Create a Data PDU with header and append data
     *
     * Place User Plane PDCP Data PDU header first
     */

    if ((rab_id % MAX_NUM_RB) < DTCH) { // this Control plane PDCP Data PDU
      pdcp_control_plane_data_pdu_header pdu_header;
      pdu_header.sn = pdcp_get_next_tx_seq_number(pdcp);
      current_sn = pdu_header.sn;
      memset(&pdu_header.mac_i[0],0,PDCP_CONTROL_PLANE_DATA_PDU_MAC_I_SIZE);
      if (pdcp_serialize_control_plane_data_pdu_with_SRB_sn_buffer((unsigned char*)pdcp_pdu->data, &pdu_header) == FALSE) {
	LOG_E(PDCP, "Cannot fill PDU buffer with relevant header fields!\n");
	return FALSE;
      }
    } else {
      pdcp_user_plane_data_pdu_header_with_long_sn pdu_header;
      pdu_header.dc = (is_data_pdu == 1) ? PDCP_DATA_PDU :  PDCP_CONTROL_PDU;
      pdu_header.sn = pdcp_get_next_tx_seq_number(pdcp);
      current_sn = pdu_header.sn ;
      if (pdcp_serialize_user_plane_data_pdu_with_long_sn_buffer((unsigned char*)pdcp_pdu->data, &pdu_header) == FALSE) {
	LOG_E(PDCP, "Cannot fill PDU buffer with relevant header fields!\n");
	return FALSE;
      }
    }
    /*
     * Validate incoming sequence number, there might be a problem with PDCP initialization
     */
    if (current_sn > pdcp_calculate_max_seq_num_for_given_size(pdcp->seq_num_size)) {
      LOG_E(PDCP, "Generated sequence number (%lu) is greater than a sequence number could ever be!\n", current_sn);
      LOG_E(PDCP, "There must be a problem with PDCP initialization, ignoring this PDU...\n");

      free_mem_block(pdcp_pdu);
      return FALSE;
    }

    LOG_I(PDCP, "Sequence number %d is assigned to current PDU\n", current_sn);

     /* Then append data... */
    memcpy(&pdcp_pdu->data[pdcp_header_len], sdu_buffer, sdu_buffer_size);
    //For control plane data that are not integrity protected,
    // the MAC-I field is still present and should be padded with padding bits set to 0.
    for (i=0;i<pdcp_tailer_len;i++)
      pdcp_pdu->data[pdcp_header_len + sdu_buffer_size + i] = 0x00;// pdu_header.mac_i[i];

    /* Print octets of outgoing data in hexadecimal form */
    LOG_D(PDCP, "Following content with size %d will be sent over RLC (PDCP PDU header is the first two bytes)\n",
	  pdcp_pdu_size);
    //util_print_hex_octets(PDCP, (unsigned char*)pdcp_pdu->data, pdcp_pdu_size);
    util_flush_hex_octets(PDCP, (unsigned char*)pdcp_pdu->data, pdcp_pdu_size);

#ifdef PDCP_UNIT_TEST
    /*
     * Here we add PDU to the list and return to test code without
     * handing it off to RLC
     */
    list_add_tail_eurecom(pdcp_pdu, test_list);

    return TRUE;
#else
    /*
     * Ask sublayer to transmit data and check return value
     * to see if RLC succeeded
     */
    rlc_op_status_t rlc_status = rlc_data_req(module_id, frame, eNB_flag, rab_id, muiP, confirmP, pdcp_pdu_size, pdcp_pdu);
    switch (rlc_status) {
      case RLC_OP_STATUS_OK:
        LOG_I(PDCP, "Data sending request over RLC succeeded!\n");
	break;

      case RLC_OP_STATUS_BAD_PARAMETER:
	LOG_W(PDCP, "Data sending request over RLC failed with 'Bad Parameter' reason!\n");
	return FALSE;

      case RLC_OP_STATUS_INTERNAL_ERROR:
	LOG_W(PDCP, "Data sending request over RLC failed with 'Internal Error' reason!\n");
	return FALSE;

      case RLC_OP_STATUS_OUT_OF_RESSOURCES:
	LOG_W(PDCP, "Data sending request over RLC failed with 'Out of Resources' reason!\n");
	return FALSE;

      default:
	LOG_W(PDCP, "RLC returned an unknown status code after PDCP placed the order to send some data (Status Code:%d)\n", rlc_status);
	return FALSE;
    }

    /*
     * Control arrives here only if rlc_data_req() returns RLC_OP_STATUS_OK
     * so we return TRUE afterwards
     */
    if (eNB_flag == 1) {
      Pdcp_stats_tx[module_id][(rab_id & RAB_OFFSET2 )>> RAB_SHIFT2][(rab_id & RAB_OFFSET)-DTCH]++;
      Pdcp_stats_tx_bytes[module_id][(rab_id & RAB_OFFSET2 )>> RAB_SHIFT2][(rab_id & RAB_OFFSET)-DTCH] += sdu_buffer_size;
    } else {
      Pdcp_stats_tx[module_id][(rab_id & RAB_OFFSET2 )>> RAB_SHIFT2][(rab_id & RAB_OFFSET)-DTCH]++;
      Pdcp_stats_tx_bytes[module_id][(rab_id & RAB_OFFSET2 )>> RAB_SHIFT2][(rab_id & RAB_OFFSET)-DTCH] += sdu_buffer_size;
    }

    return TRUE;
#endif // PDCP_UNIT_TEST
  } else {
    LOG_E(PDCP, "Cannot create a mem_block for a PDU!\n");

    return FALSE;
  }
}

//-----------------------------------------------------------------------------
#ifdef PDCP_UNIT_TEST
BOOL pdcp_data_ind(module_id_t module_id, u32_t frame, u8_t eNB_flag, rb_id_t rab_id, sdu_size_t sdu_buffer_size, \
                   mem_block_t* sdu_buffer, pdcp_t* pdcp_test_entity, list_t* test_list)
#else
BOOL pdcp_data_ind(module_id_t module_id, u32_t frame, u8_t eNB_flag, rb_id_t rab_id, sdu_size_t sdu_buffer_size, \
                   mem_block_t* sdu_buffer, u8 is_data_plane)
#endif
{
//-----------------------------------------------------------------------------
#ifdef PDCP_UNIT_TEST
  pdcp_t* pdcp = pdcp_test_entity;
  list_t* sdu_list = test_list;
#else
  pdcp_t* pdcp = &pdcp_array[module_id][rab_id];
  list_t* sdu_list = &pdcp_sdu_list;
#endif
  mem_block_t *new_sdu = NULL;
  int src_id, dst_id,ctime; // otg param
  u8 pdcp_header_len=0, pdcp_tailer_len=0;
  u16 sequence_number;

  LOG_I(PDCP,"Data indication notification for PDCP entity with module ID %d and radio bearer ID %d rlc sdu size %d\n", module_id, rab_id, sdu_buffer_size);

  if (sdu_buffer_size == 0) {
    LOG_W(PDCP, "SDU buffer size is zero! Ignoring this chunk!");
    return FALSE;
  }

  /*
   * Check if incoming SDU is long enough to carry a PDU header
   */
  if ((rab_id % MAX_NUM_RB) < DTCH) {
    pdcp_header_len = PDCP_CONTROL_PLANE_DATA_PDU_SN_SIZE;
    pdcp_tailer_len = PDCP_CONTROL_PLANE_DATA_PDU_MAC_I_SIZE;
  } else {
    pdcp_header_len = PDCP_USER_PLANE_DATA_PDU_LONG_SN_HEADER_SIZE;
    pdcp_tailer_len = 0;
  }

  if (sdu_buffer_size < pdcp_header_len + pdcp_tailer_len ) {
    LOG_W(PDCP, "Incoming (from RLC) SDU is short of size (size:%d)! Ignoring...\n", sdu_buffer_size);
#ifndef PDCP_UNIT_TEST
    free_mem_block(sdu_buffer);
#endif
    return FALSE;
  }

  /*
   * Parse the PDU placed at the beginning of SDU to check
   * if incoming SN is in line with RX window
   */

  if (pdcp_header_len == PDCP_USER_PLANE_DATA_PDU_LONG_SN_HEADER_SIZE) { // DRB
    sequence_number =     pdcp_get_sequence_number_of_pdu_with_long_sn((unsigned char*)sdu_buffer->data);
    u8 dc = pdcp_get_dc_filed((unsigned char*)sdu_buffer->data);
  } else { //SRB1/2
    sequence_number =   pdcp_get_sequence_number_of_pdu_with_SRB_sn((unsigned char*)sdu_buffer->data);
  }
  if (pdcp_is_rx_seq_number_valid(sequence_number, pdcp) == TRUE) {
    LOG_I(PDCP, "Incoming PDU has a sequence number (%d) in accordance with RX window, yay!\n", sequence_number);
    /* if (dc == PDCP_DATA_PDU )
      LOG_D(PDCP, "Passing piggybacked SDU to NAS driver...\n");
    else
    LOG_D(PDCP, "Passing piggybacked SDU to RRC ...\n");*/
  } else {
    LOG_W(PDCP, "Incoming PDU has an unexpected sequence number (%d), RX window snychronisation have probably been lost!\n", sequence_number);
    /*
     * XXX Till we implement in-sequence delivery and duplicate discarding
     * mechanism all out-of-order packets will be delivered to RRC/IP
     */
#if 0
    LOG_D(PDCP, "Ignoring PDU...\n");
    free_mem_block(sdu_buffer);
    return FALSE;
#else
    LOG_W(PDCP, "Delivering out-of-order SDU to upper layer...\n");
#endif
  }
   // SRB1/2: control-plane data
  if ( (rab_id % MAX_NUM_RB) <  DTCH ){
    /*new_sdu = get_free_mem_block(sdu_buffer_size - pdcp_header_len - pdcp_tailer_len);
    if (new_sdu) {
      memcpy(new_sdu->data,
	     &sdu_buffer->data[pdcp_header_len],
	     sdu_buffer_size - pdcp_header_len - pdcp_tailer_len);
      rrc_lite_data_ind(module_id,
			frame,
			eNB_flag,
			rab_id,
			sdu_buffer_size - pdcp_header_len - pdcp_tailer_len,
			new_sdu->data);
			}*/
    rrc_lite_data_ind(module_id,
			frame,
			eNB_flag,
			rab_id,
			sdu_buffer_size - pdcp_header_len - pdcp_tailer_len,
			&sdu_buffer->data[pdcp_header_len]);
    free_mem_block(sdu_buffer);
    // free_mem_block(new_sdu);
    return TRUE;
  }
#if defined(USER_MODE) && defined(OAI_EMU)
  if (oai_emulation.info.otg_enabled ==1 ){
  src_id = (eNB_flag == 1) ? (rab_id - DTCH) / MAX_NUM_RB  /*- NB_eNB_INST */ + 1 :  ((rab_id - DTCH) / MAX_NUM_RB);
  dst_id = (eNB_flag == 1) ? module_id : module_id /*-  NB_eNB_INST*/;
  ctime = oai_emulation.info.time_ms; // avg current simulation time in ms : we may get the exact time through OCG?
  LOG_I(OTG,"Check received buffer : enb_flag %d mod id %d, rab id %d (src %d, dst %d)\n", eNB_flag, module_id, rab_id, src_id, dst_id);
  if (otg_rx_pkt(src_id, dst_id,ctime,&sdu_buffer->data[PDCP_USER_PLANE_DATA_PDU_LONG_SN_HEADER_SIZE],
		 sdu_buffer_size - PDCP_USER_PLANE_DATA_PDU_LONG_SN_HEADER_SIZE ) == 0 ) {
     free_mem_block(sdu_buffer);
     return TRUE;
  }
  }
#endif
  new_sdu = get_free_mem_block(sdu_buffer_size - PDCP_USER_PLANE_DATA_PDU_LONG_SN_HEADER_SIZE + sizeof (pdcp_data_ind_header_t));

  if (new_sdu) {
    /*
     * Prepend PDCP indication header which is going to be removed at pdcp_fifo_flush_sdus()
     */
    memset(new_sdu->data, 0, sizeof (pdcp_data_ind_header_t));
    ((pdcp_data_ind_header_t *) new_sdu->data)->rb_id     = rab_id;
    ((pdcp_data_ind_header_t *) new_sdu->data)->data_size = sdu_buffer_size - PDCP_USER_PLANE_DATA_PDU_LONG_SN_HEADER_SIZE;

    // Here there is no virtualization possible
#ifdef IDROMEL_NEMO
    if (eNB_flag == 0)
      ((pdcp_data_ind_header_t *) new_sdu->data)->inst = rab_id/8;
    else
      ((pdcp_data_ind_header_t *) new_sdu->data)->inst = 0;
#else
      ((pdcp_data_ind_header_t *) new_sdu->data)->inst = module_id;
#endif

    // XXX Decompression would be done at this point

    /*
     * After checking incoming sequence number PDCP header
     * has to be stripped off so here we copy SDU buffer starting
     * from its second byte (skipping 0th and 1st octets, i.e.
     * PDCP header)
     */
    memcpy(&new_sdu->data[sizeof (pdcp_data_ind_header_t)], \
           &sdu_buffer->data[PDCP_USER_PLANE_DATA_PDU_LONG_SN_HEADER_SIZE], \
           sdu_buffer_size - PDCP_USER_PLANE_DATA_PDU_LONG_SN_HEADER_SIZE);
    list_add_tail_eurecom (new_sdu, sdu_list);

    /* Print octets of incoming data in hexadecimal form */
    LOG_D(PDCP, "Following content has been received from RLC (%d,%d)(PDCP header has already been removed):\n", sdu_buffer_size  - PDCP_USER_PLANE_DATA_PDU_LONG_SN_HEADER_SIZE + sizeof(pdcp_data_ind_header_t),
	  sdu_buffer_size  - PDCP_USER_PLANE_DATA_PDU_LONG_SN_HEADER_SIZE);
    //util_print_hex_octets(PDCP, (unsigned char*)new_sdu->data, sdu_buffer_size  - PDCP_USER_PLANE_DATA_PDU_LONG_SN_HEADER_SIZE + sizeof(pdcp_data_ind_header_t));
    util_flush_hex_octets(PDCP, (unsigned char*)new_sdu->data, sdu_buffer_size  - PDCP_USER_PLANE_DATA_PDU_LONG_SN_HEADER_SIZE + sizeof(pdcp_data_ind_header_t));

    /*
     * Update PDCP statistics
     * XXX Following two actions are identical, is there a merge error?
     */
    if (eNB_flag == 1) {
      Pdcp_stats_rx[module_id][(rab_id & RAB_OFFSET2) >> RAB_SHIFT2][(rab_id & RAB_OFFSET) - DTCH]++;
      Pdcp_stats_rx_bytes[module_id][(rab_id & RAB_OFFSET2) >> RAB_SHIFT2][(rab_id & RAB_OFFSET) - DTCH] += sdu_buffer_size;
    } else {
      Pdcp_stats_rx[module_id][(rab_id & RAB_OFFSET2) >> RAB_SHIFT2][(rab_id & RAB_OFFSET) - DTCH]++;
      Pdcp_stats_rx_bytes[module_id][(rab_id & RAB_OFFSET2) >> RAB_SHIFT2][(rab_id & RAB_OFFSET) - DTCH] += sdu_buffer_size;
    }
  }

  free_mem_block(sdu_buffer);

  return TRUE;
}

//-----------------------------------------------------------------------------
void
pdcp_run (u32_t frame, u8 eNB_flag, u8 UE_index, u8 eNB_index) {
//-----------------------------------------------------------------------------

#ifndef NAS_NETLINK
  #ifdef USER_MODE
    #define PDCP_DUMMY_BUFFER_SIZE 38
    unsigned char pdcp_dummy_buffer[PDCP_DUMMY_BUFFER_SIZE];
  #endif
#endif
  unsigned int diff, i, k, j;
  char *otg_pkt=NULL;
  int src_id, module_id; // src for otg
  int dst_id, rab_id; // dst for otg
  int pkt_size=0;
  unsigned int ctime=0;
  /*
  if ((frame % 128) == 0) {
    for (i=0; i < NB_UE_INST; i++) {
      for (j=0; j < NB_CNX_CH; j++) {
        for (k=0; k < NB_RAB_MAX; k++) {
          diff = Pdcp_stats_tx_bytes[i][j][k];
          Pdcp_stats_tx_bytes[i][j][k] = 0;
          Pdcp_stats_tx_rate[i][j][k] = (diff*8) >> 7;

          diff = Pdcp_stats_rx_bytes[i][j][k];
          Pdcp_stats_rx_bytes[i][j][k] = 0;
          Pdcp_stats_rx_rate[i][j][k] = (diff*8) >> 7;
        }
      }
    }
  }
  */
  // we need to add conditions to avoid transmitting data when the UE is not RRC connected.
#if defined(USER_MODE) && defined(OAI_EMU)
  if (oai_emulation.info.otg_enabled ==1 ){
    module_id = (eNB_flag == 1) ?  eNB_index : /*NB_eNB_INST +*/ UE_index ;
    //rab_id    = (eNB_flag == 1) ? eNB_index * MAX_NUM_RB + DTCH : (NB_eNB_INST + UE_index -1 ) * MAX_NUM_RB + DTCH ;
    ctime = oai_emulation.info.time_ms; // current simulation time in ms
    if (eNB_flag == 1) { // search for DL traffic
      for (dst_id = NB_eNB_INST; dst_id < NB_UE_INST + NB_eNB_INST; dst_id++) {
	// generate traffic if the ue is rrc reconfigured state 
	if (mac_get_rrc_status(module_id, eNB_flag, dst_id - NB_eNB_INST ) > 2 /*RRC_CONNECTED*/ ) { 
	  otg_pkt=packet_gen(module_id, dst_id, ctime, &pkt_size);
	  if (otg_pkt != NULL) {
	    rab_id = (/*NB_eNB_INST +*/ dst_id -1 ) * MAX_NUM_RB + DTCH;
	    pdcp_data_req(module_id, frame, eNB_flag, rab_id, RLC_MUI_UNDEFINED, RLC_SDU_CONFIRM_NO, pkt_size, otg_pkt,PDCP_DATA_PDU);
	    LOG_I(OTG,"[eNB %d] send packet from module %d on rab id %d (src %d, dst %d) pkt size %d\n", eNB_index, module_id, rab_id, module_id, dst_id, pkt_size);
	    free(otg_pkt);
	  }
	} //else LOG_D(OTG,"frame %d enb %d-> ue %d link not yet established  \n", frame, eNB_index,dst_id - NB_eNB_INST);
      }
    }else {
      src_id = module_id+NB_eNB_INST;
      dst_id = eNB_index;	
      if (mac_get_rrc_status(module_id, eNB_flag, eNB_index ) > 2 /*RRC_CONNECTED*/ ) { 
	otg_pkt=packet_gen(src_id, dst_id, ctime, &pkt_size);
	if (otg_pkt != NULL){
	  rab_id= eNB_index * MAX_NUM_RB + DTCH;
	  pdcp_data_req(src_id, frame, eNB_flag, rab_id, RLC_MUI_UNDEFINED, RLC_SDU_CONFIRM_NO,pkt_size, otg_pkt, PDCP_DATA_PDU);
	  LOG_I(OTG,"[UE %d] send packet from module %d on rab id %d (src %d, dst %d) pkt size %d\n", UE_index, src_id, rab_id, src_id, dst_id, pkt_size);
	  free(otg_pkt);
	}
      } //else LOG_D(OTG,"frame %d ue %d-> enb %d link not yet established \n", frame, UE_index, eNB_index);
    }
  }
#endif
  // NAS -> PDCP traffic
  pdcp_fifo_read_input_sdus(frame,eNB_flag);

  // PDCP -> NAS traffic
  pdcp_fifo_flush_sdus(frame,eNB_flag);

//OTG
/*
  if ( eNB_flag == 0){
    char *rx_packet_out;
    rx_packet_out=check_packet(0, 0, frame, packet_gen(0, 0, 0, frame));
    if (rx_packet_out!=NULL){
      rx_packet_out=NULL;
      free(rx_packet_out);
    }
  }
*/

}

//-----------------------------------------------------------------------------
void
rrc_pdcp_config_req (module_id_t module_id, u32 frame, u8_t eNB_flag, u32  action, rb_id_t rab_id){
//-----------------------------------------------------------------------------
  /*
   * Initialize sequence number state variables of relevant PDCP entity
   */
  switch (action) {
  case ACTION_ADD:
    pdcp_array[module_id][rab_id].next_pdcp_tx_sn = 0;
    pdcp_array[module_id][rab_id].next_pdcp_rx_sn = 0;
    pdcp_array[module_id][rab_id].tx_hfn = 0;
    pdcp_array[module_id][rab_id].rx_hfn = 0;
    /* SN of the last PDCP SDU delivered to upper layers */
    pdcp_array[module_id][rab_id].last_submitted_pdcp_rx_sn = 4095;

    if ( (rab_id % MAX_NUM_RB) < DTCH) // SRB
      pdcp_array[module_id][rab_id].seq_num_size = 5;
    else // DRB
      pdcp_array[module_id][rab_id].seq_num_size = 12;
    pdcp_array[module_id][rab_id].first_missing_pdu = -1;
    LOG_I(PDCP,"[%s %d] Config request : Action ADD: Frame %d radio bearer id %d configured\n",
	  (eNB_flag) ? "eNB" : "UE", module_id, frame, rab_id);
    LOG_D(PDCP,  "[MSC_NEW][FRAME %05d][PDCP][MOD %02d][RB %02d]\n", frame, module_id,rab_id);
    break;
  case ACTION_MODIFY:
    break;
  case ACTION_REMOVE:
    pdcp_array[module_id][rab_id].next_pdcp_tx_sn = 0;
  pdcp_array[module_id][rab_id].next_pdcp_rx_sn = 0;
  pdcp_array[module_id][rab_id].tx_hfn = 0;
  pdcp_array[module_id][rab_id].rx_hfn = 0;
  pdcp_array[module_id][rab_id].last_submitted_pdcp_rx_sn = 4095;
  pdcp_array[module_id][rab_id].seq_num_size = 0;
  pdcp_array[module_id][rab_id].first_missing_pdu = -1;
  LOG_I(PDCP,"[%s %d] Config request : ACTION_REMOVE: Frame %d radio bearer id %d configured\n",
	  (eNB_flag) ? "eNB" : "UE", module_id, frame, rab_id);

    break;
  default:
    break;
  }

}

// TODO PDCP module initialization code might be removed
int
pdcp_module_init ()
{
//-----------------------------------------------------------------------------
#ifdef NAS_FIFO
  int ret;

  ret=rtf_create(PDCP2NAS_FIFO,32768);

  if (ret < 0) {
    LOG_E(PDCP, "Cannot create PDCP2NAS fifo %d (ERROR %d)\n", PDCP2NAS_FIFO, ret);

    return -1;
  } else {
    LOG_I(PDCP, "Created PDCP2NAS fifo %d\n", PDCP2NAS_FIFO);
    rtf_reset(PDCP2NAS_FIFO);
  }

  ret=rtf_create(NAS2PDCP_FIFO,32768);

  if (ret < 0) {
    LOG_E(PDCP, "Cannot create NAS2PDCP fifo %d (ERROR %d)\n", NAS2PDCP_FIFO, ret);

    return -1;
  } else {
    LOG_I(PDCP, "Created NAS2PDCP fifo %d\n", NAS2PDCP_FIFO);
    rtf_reset(NAS2PDCP_FIFO);
  }

  pdcp_2_nas_irq = 0;
  pdcp_input_sdu_remaining_size_to_read=0;
  pdcp_input_sdu_size_read=0;
#endif

  return 0;

}

//-----------------------------------------------------------------------------
void
pdcp_module_cleanup ()
//-----------------------------------------------------------------------------
{
#ifdef NAS_FIFO
  rtf_destroy(NAS2PDCP_FIFO);
  rtf_destroy(PDCP2NAS_FIFO);
#endif
}

//-----------------------------------------------------------------------------
void
pdcp_layer_init ()
{
//-----------------------------------------------------------------------------
  unsigned int i, j, k;

  /*
   * Initialize SDU list
   */
  list_init(&pdcp_sdu_list, NULL);

  LOG_I(PDCP, "PDCP layer has been initialized\n");

  pdcp_output_sdu_bytes_to_write=0;
  pdcp_output_header_bytes_to_write=0;
  pdcp_input_sdu_remaining_size_to_read=0;
  /*
   * Initialize PDCP entities (see pdcp_t at pdcp.h)
   */
  // set RB for eNB : this is now down by RRC for each mod id and rab id when needed.
  /*  for (i=0;i  < NB_eNB_INST; i++) {
    for (j=NB_eNB_INST; j < NB_eNB_INST+NB_UE_INST; j++ ) {
      pdcp_config_req(i, (j-NB_eNB_INST) * MAX_NUM_RB + DCCH, DCCH  ); // default DRB
      pdcp_config_req(i, (j-NB_eNB_INST) * MAX_NUM_RB + DCCH1, DCCH1  ); // default DRB
      pdcp_config_req(i, (j-NB_eNB_INST) * MAX_NUM_RB + DTCH, DTCH  ); // default DRB
    }
  }
  // set RB for UE
  for (i=NB_eNB_INST;i<NB_eNB_INST+NB_UE_INST; i++) {
    for (j=0;j<NB_eNB_INST; j++) {
      pdcp_config_req(i, j * MAX_NUM_RB + DCCH, DCCH ); // default DRB
      pdcp_config_req(i, j * MAX_NUM_RB + DCCH1, DCCH1 ); // default DRB
      pdcp_config_req(i, j * MAX_NUM_RB + DTCH, DTCH ); // default DRB
    }
    }*/

  for (i=0;i<NB_UE_INST;i++) { // ue
    for (k=0;k<NB_eNB_INST;k++) { // enb
      for(j=0;j<NB_RAB_MAX;j++) {//rb
        Pdcp_stats_tx[i][k][j]=0;
        Pdcp_stats_tx_bytes[i][k][j]=0;
        Pdcp_stats_tx_bytes_last[i][k][j]=0;
        Pdcp_stats_tx_rate[i][k][j]=0;

        Pdcp_stats_rx[i][k][j]=0;
        Pdcp_stats_rx_bytes[i][k][j]=0;
        Pdcp_stats_rx_bytes_last[i][k][j]=0;
        Pdcp_stats_rx_rate[i][k][j]=0;
      }
    }
  }
}

//-----------------------------------------------------------------------------
void
pdcp_layer_cleanup ()
//-----------------------------------------------------------------------------
{
  list_free (&pdcp_sdu_list);
}

#ifdef NAS_FIFO
  EXPORT_SYMBOL(pdcp_2_nas_irq);
#endif //NAS_FIFO
