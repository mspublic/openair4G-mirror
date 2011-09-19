#define RLC_UM_MODULE
#define RLC_UM_C
//-----------------------------------------------------------------------------
#include "rtos_header.h"
#include "platform_types.h"
#include "platform_constants.h"
//-----------------------------------------------------------------------------
#include "rlc_um.h"
#include "list.h"
#include "rlc_primitives.h"
#include "mac_primitives.h"
#include "LAYER2/MAC/extern.h"

//#define RLC_UM_TEST_TRAFFIC

//#define DEBUG_RLC_UM_DATA_REQUEST
//#define DEBUG_RLC_UM_MAC_DATA_REQUEST
//#define DEBUG_RLC_UM_MAC_DATA_INDICATION
//#define DEBUG_RLC_UM_TX_STATUS
//#define DEBUG_RLC_UM_DISCARD_SDU
//-----------------------------------------------------------------------------
void
rlc_um_stat_req     (rlc_um_entity_t *rlcP,
							  unsigned int* tx_pdcp_sdu,
							  unsigned int* tx_pdcp_sdu_discarded,
							  unsigned int* tx_data_pdu,
							  unsigned int* rx_sdu,
							  unsigned int* rx_error_pdu,
							  unsigned int* rx_data_pdu,
							  unsigned int* rx_data_pdu_out_of_window) {
//-----------------------------------------------------------------------------
  *tx_pdcp_sdu               = rlcP->tx_pdcp_sdu;
  *tx_pdcp_sdu_discarded     = rlcP->tx_pdcp_sdu_discarded;
  *tx_data_pdu               = rlcP->tx_data_pdu;
  *rx_sdu                    = rlcP->rx_sdu;
  *rx_error_pdu              = rlcP->rx_error_pdu;
  *rx_data_pdu               = rlcP->rx_data_pdu;
  *rx_data_pdu_out_of_window = rlcP->rx_data_pdu_out_of_window;
}
//-----------------------------------------------------------------------------
u32_t
rlc_um_get_buffer_occupancy (rlc_um_entity_t *rlcP)
{
//-----------------------------------------------------------------------------
    if (rlcP->buffer_occupancy > 0) {
        return rlcP->buffer_occupancy;
    } else {
        return 0;
    }
}
//-----------------------------------------------------------------------------
void
rlc_um_get_pdus (void *argP)
{
//-----------------------------------------------------------------------------
  rlc_um_entity_t *rlc = (rlc_um_entity_t *) argP;

  switch (rlc->protocol_state) {

      case RLC_NULL_STATE:
        // from 3GPP TS 25.322 V9.2.0 p43
        // In the NULL state the RLC entity does not exist and therefore it is
        // not possible to transfer any data through it.
        // Upon reception of a CRLC-CONFIG-Req from upper layer indicating
        // establishment, the RLC entity:
        //   - is created; and
        //   - enters the DATA_TRANSFER_READY state.
        break;

      case RLC_DATA_TRANSFER_READY_STATE:
        // from 3GPP TS 25.322 V9.2.0 p43-44
        // In the DATA_TRANSFER_READY state, unacknowledged mode data can be
        // exchanged between the entities according to subclause 11.2.
        // Upon reception of a CRLC-CONFIG-Req from upper layer indicating
        // release, the RLC entity:
        // -enters the NULL state; and
        // -is considered as being terminated.
        // Upon reception of a CRLC-CONFIG-Req from upper layer indicating
        // modification, the RLC entity:
        // - stays in the DATA_TRANSFER_READY state;
        // - modifies only the protocol parameters and timers as indicated by
        // upper layers.
        // Upon reception of a CRLC-SUSPEND-Req from upper layers, the RLC
        // entity:
        // - enters the LOCAL_SUSPEND state.

        // SEND DATA TO MAC
        rlc_um_segment_10 (rlc);
        break;

      case RLC_LOCAL_SUSPEND_STATE:
        // from 3GPP TS 25.322 V9.2.0 p44
        // In the LOCAL_SUSPEND state, the RLC entity is suspended, i.e. it does
        // not send UMD PDUs with "Sequence Number" greater than or equal to a
        // certain specified value (see subclause 9.7.5).
        // Upon reception of a CRLC-CONFIG-Req from upper layer indicating
        // release, the RLC entity:
        // - enters the NULL state; and
        // - is considered as being terminated.
        // Upon reception of a CRLC-RESUME-Req from upper layers, the RLC entity:
        // - enters the DATA_TRANSFER_READY state; and
        // - resumes the data transmission.
        // Upon reception of a CRLC-CONFIG-Req from upper layer indicating
        // modification, the RLC entity:
        // - stays in the LOCAL_SUSPEND state;
        // - modifies only the protocol parameters and timers as indicated by
        //   upper layers.

        // TO DO TAKE CARE OF SN : THE IMPLEMENTATION OF THIS FUNCTIONNALITY IS NOT CRITICAL
        break;

      default:
        msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] MAC_DATA_REQ UNKNOWN PROTOCOL STATE %02X hex\n", rlc->module_id, rlc->rb_id, mac_xface->frame, rlc->protocol_state);
  }
}

//-----------------------------------------------------------------------------
void
rlc_um_rx (void *argP, struct mac_data_ind data_indP)
{
//-----------------------------------------------------------------------------
  rlc_um_entity_t *rlc = (rlc_um_entity_t *) argP;

  switch (rlc->protocol_state) {

      case RLC_NULL_STATE:
        // from 3GPP TS 25.322 V9.2.0 p43
        // In the NULL state the RLC entity does not exist and therefore it is
        // not possible to transfer any data through it.
        // Upon reception of a CRLC-CONFIG-Req from upper layer indicating
        // establishment, the RLC entity:
        //   - is created; and
        //   - enters the DATA_TRANSFER_READY state.
        msg ("[RLC_UM][MOD %d] ERROR MAC_DATA_IND IN RLC_NULL_STATE\n", rlc->module_id);
        list_free (&data_indP.data);
        break;

      case RLC_DATA_TRANSFER_READY_STATE:
        // from 3GPP TS 25.322 V9.2.0 p43-44
        // In the DATA_TRANSFER_READY state, unacknowledged mode data can be
        // exchanged between the entities according to subclause 11.2.
        // Upon reception of a CRLC-CONFIG-Req from upper layer indicating
        // release, the RLC entity:
        // -enters the NULL state; and
        // -is considered as being terminated.
        // Upon reception of a CRLC-CONFIG-Req from upper layer indicating
        // modification, the RLC entity:
        // - stays in the DATA_TRANSFER_READY state;
        // - modifies only the protocol parameters and timers as indicated by
        // upper layers.
        // Upon reception of a CRLC-SUSPEND-Req from upper layers, the RLC
        // entity:
        // - enters the LOCAL_SUSPEND state.
        data_indP.tb_size = data_indP.tb_size >> 3;
        rlc_um_receive (rlc, data_indP);
        break;

      case RLC_LOCAL_SUSPEND_STATE:
        // from 3GPP TS 25.322 V9.2.0 p44
        // In the LOCAL_SUSPEND state, the RLC entity is suspended, i.e. it does
        // not send UMD PDUs with "Sequence Number" greater than or equal to a
        // certain specified value (see subclause 9.7.5).
        // Upon reception of a CRLC-CONFIG-Req from upper layer indicating
        // release, the RLC entity:
        // - enters the NULL state; and
        // - is considered as being terminated.
        // Upon reception of a CRLC-RESUME-Req from upper layers, the RLC entity:
        // - enters the DATA_TRANSFER_READY state; and
        // - resumes the data transmission.
        // Upon reception of a CRLC-CONFIG-Req from upper layer indicating
        // modification, the RLC entity:
        // - stays in the LOCAL_SUSPEND state;
        // - modifies only the protocol parameters and timers as indicated by
        //   upper layers.
        msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] RLC_LOCAL_SUSPEND_STATE\n", rlc->module_id, rlc->rb_id, mac_xface->frame);
        break;

      default:
        msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] TX UNKNOWN PROTOCOL STATE %02X hex\n", rlc->module_id, rlc->rb_id, mac_xface->frame, rlc->protocol_state);
  }
}

//-----------------------------------------------------------------------------
struct mac_status_resp
rlc_um_mac_status_indication (void *rlcP, u16_t tbs_sizeP, struct mac_status_ind tx_statusP)
{
//-----------------------------------------------------------------------------
  struct mac_status_resp status_resp;

  if (rlcP) {

#ifdef RLC_UM_TEST_TRAFFIC
    if ((mac_xface->frame % 200) == 0) {
        rlc_um_test_send_sdu(rlcP, RLC_UM_TEST_SDU_TYPE_TCPIP);
    }
    if ((mac_xface->frame % 40) == 0) {
        rlc_um_test_send_sdu(rlcP, RLC_UM_TEST_SDU_TYPE_VOIP);
    }
    if ((mac_xface->frame % 4) == 0) {
        rlc_um_test_send_sdu(rlcP, RLC_UM_TEST_SDU_TYPE_SMALL);
    }
#endif

  ((rlc_um_entity_t *) rlcP)->nb_bytes_requested_by_mac = tbs_sizeP;

  status_resp.buffer_occupancy_in_bytes = rlc_um_get_buffer_occupancy ((rlc_um_entity_t *) rlcP);
  if (status_resp.buffer_occupancy_in_bytes > 0) {
    status_resp.buffer_occupancy_in_bytes += ((rlc_um_entity_t *) rlcP)->header_min_length_in_bytes;
  }
//msg("[RLC_UM][MOD %d][RB %d][FRAME %05d] MAC_STATUS_INDICATION BO = %d\n", ((rlc_um_entity_t *) rlcP)->module_id, ((rlc_um_entity_t *) rlcP)->rb_id, status_resp.buffer_occupancy_in_bytes);

  status_resp.rlc_info.rlc_protocol_state = ((rlc_um_entity_t *) rlcP)->protocol_state;
#ifdef DEBUG_RLC_UM_TX_STATUS
  if (((rlc_um_entity_t *) rlcP)->rb_id > 0) {
    msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] MAC_STATUS_INDICATION (DATA) %d bytes -> %d bytes\n", ((rlc_um_entity_t *) rlcP)->module_id, ((rlc_um_entity_t *) rlcP)->rb_id, mac_xface->frame, tbs_sizeP, status_resp.buffer_occupancy_in_bytes);
    if ((tx_statusP.tx_status == MAC_TX_STATUS_SUCCESSFUL) && (tx_statusP.no_pdu)) {
      msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] MAC_STATUS_INDICATION  TX STATUS   SUCCESSFUL %d PDUs\n",((rlc_um_entity_t *) rlcP)->module_id, ((rlc_um_entity_t *) rlcP)->rb_id, mac_xface->frame, tx_statusP.no_pdu);
    }
    if ((tx_statusP.tx_status == MAC_TX_STATUS_UNSUCCESSFUL) && (tx_statusP.no_pdu)) {
      msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] MAC_STATUS_INDICATION  TX STATUS UNSUCCESSFUL %d PDUs\n",((rlc_um_entity_t *) rlcP)->module_id, ((rlc_um_entity_t *) rlcP)->rb_id, mac_xface->frame, tx_statusP.no_pdu);
    }
  }
#endif
 }
 else
   msg("[RLC] RLCp not defined!!!\n");
  return status_resp;
}

//-----------------------------------------------------------------------------
struct mac_data_req
rlc_um_mac_data_request (void *rlcP)
{
//-----------------------------------------------------------------------------
  struct mac_data_req data_req;

  rlc_um_get_pdus (rlcP);

  list_init (&data_req.data, NULL);
  list_add_list (&((rlc_um_entity_t *) rlcP)->pdus_to_mac_layer, &data_req.data);
#ifdef DEBUG_RLC_STATS
  ((rlc_um_entity_t *) rlcP)->tx_pdus += data_req.data.nb_elements;
#endif

#ifdef DEBUG_RLC_UM_MAC_DATA_REQUEST
    msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] MAC_DATA_REQUEST %d TBs\n", ((rlc_um_entity_t *) rlcP)->module_id, ((rlc_um_entity_t *) rlcP)->rb_id, mac_xface->frame, data_req.data.nb_elements);
#endif
  data_req.buffer_occupancy_in_bytes = rlc_um_get_buffer_occupancy ((rlc_um_entity_t *) rlcP);
  if (data_req.buffer_occupancy_in_bytes > 0) {
    data_req.buffer_occupancy_in_bytes += ((rlc_um_entity_t *) rlcP)->header_min_length_in_bytes;
  }
  data_req.rlc_info.rlc_protocol_state = ((rlc_um_entity_t *) rlcP)->protocol_state;
  return data_req;
}

//-----------------------------------------------------------------------------
void
rlc_um_mac_data_indication (void *rlcP, struct mac_data_ind data_indP)
{
//-----------------------------------------------------------------------------
  rlc_um_rx (rlcP, data_indP);
}

//-----------------------------------------------------------------------------
void
rlc_um_data_req (void *rlcP, mem_block_t *sduP)
{
//-----------------------------------------------------------------------------
  rlc_um_entity_t *rlc = (rlc_um_entity_t *) rlcP;

#ifndef USER_MODE
  unsigned long int rlc_um_time_us;
  int min, sec, usec;
#endif

#ifdef DEBUG_RLC_UM_DATA_REQUEST
    msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] RLC_UM_DATA_REQ size %d Bytes, BO %d , NB SDU %d current_sdu_index=%d next_sdu_index=%d\n",
     rlc->module_id,
     rlc->rb_id,
     mac_xface->frame,
	 ((struct rlc_um_data_req *) (sduP->data))->data_size,
	 rlc->buffer_occupancy,
	 rlc->nb_sdu,
      rlc->current_sdu_index,
	 rlc->next_sdu_index);
  /*#ifndef USER_MODE
  rlc_um_time_us = (unsigned long int)(rt_get_time_ns ()/(RTIME)1000);
  sec = (rlc_um_time_us/ 1000000);
  min = (sec / 60) % 60;
  sec = sec % 60;
  usec =  rlc_um_time_us % 1000000;
  msg ("[RLC_UM_LITE][RB  %d] at time %2d:%2d.%6d\n", rlc->rb_id, min, sec , usec);
#endif*/
#endif
  if (rlc->input_sdus[rlc->next_sdu_index] == NULL) {
    rlc->input_sdus[rlc->next_sdu_index] = sduP;
    // IMPORTANT : do not change order of affectations
    ((struct rlc_um_tx_sdu_management *) (sduP->data))->sdu_size = ((struct rlc_um_data_req *) (sduP->data))->data_size;
    rlc->buffer_occupancy += ((struct rlc_um_tx_sdu_management *) (sduP->data))->sdu_size;
    rlc->nb_sdu += 1;
    ((struct rlc_um_tx_sdu_management *) (sduP->data))->first_byte = &sduP->data[sizeof (struct rlc_um_data_req_alloc)];
    ((struct rlc_um_tx_sdu_management *) (sduP->data))->sdu_remaining_size = ((struct rlc_um_tx_sdu_management *)
                                                                              (sduP->data))->sdu_size;
    ((struct rlc_um_tx_sdu_management *) (sduP->data))->sdu_segmented_size = 0;
    // LG ((struct rlc_um_tx_sdu_management *) (sduP->data))->sdu_creation_time = *rlc->frame_tick_milliseconds;
    // LG ??? WHO WROTE THAT LINE ?((struct rlc_um_tx_sdu_management *) (sduP->data))->sdu_creation_time = 0;
    rlc->next_sdu_index = (rlc->next_sdu_index + 1) % rlc->size_input_sdus_buffer;
  } else {
    msg("[RLC_UM][MOD %d][RB %d][FRAME %05d] RLC-UM_DATA_REQ input buffer full SDU garbaged\n",rlc->module_id, rlc->rb_id, mac_xface->frame);
    free_mem_block (sduP);
  }
}
