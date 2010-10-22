/*
                                rlc_um.c
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr
 ***************************************************************************/
#define RLC_UM_MODULE
#define RLC_UM_C
#include "rtos_header.h"
#include "platform_types.h"

#include "list.h"
#include "rlc_um_entity.h"
#include "rlc_um_proto_extern.h"
#include "rlc_um_structs.h"
#include "rlc_def.h"
#include "rlc_primitives.h"
#include "mac_primitives.h"
#include "rlc_um_segment_proto_extern.h"
#include "rlc_um_receiver_proto_extern.h"
#include "LAYER2/MAC/extern.h"


#define DEBUG_RLC_UM_DATA_REQUEST
#define DEBUG_RLC_UM_MAC_DATA_REQUEST
#define DEBUG_RLC_UM_MAC_DATA_INDICATION
#define DEBUG_RLC_UM_DISCARD_SDU
//-----------------------------------------------------------------------------
void
rlc_um_stat_req     (struct rlc_um_entity *rlcP,
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
rlc_um_get_buffer_occupancy (struct rlc_um_entity *rlcP)
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
  struct rlc_um_entity *rlc = (struct rlc_um_entity *) argP;

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
        msg ("[RLC_UM][MOD %d][RB %d] MAC_DATA_REQ UNKNOWN PROTOCOL STATE %02X hex\n", rlc->module_id, rlc->rb_id, rlc->protocol_state);
  }
}

//-----------------------------------------------------------------------------
void
rlc_um_rx (void *argP, struct mac_data_ind data_indP)
{
//-----------------------------------------------------------------------------
  struct rlc_um_entity *rlc = (struct rlc_um_entity *) argP;

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
        msg ("[RLC_UM][MOD %d][RB %d] RLC_LOCAL_SUSPEND_STATE\n", rlc->module_id, rlc->rb_id);
        break;

      default:
        msg ("[RLC_UM][MOD %d][RB %d] TX UNKNOWN PROTOCOL STATE %02X hex\n", rlc->module_id, rlc->rb_id, rlc->protocol_state);
  }
}

//-----------------------------------------------------------------------------
struct mac_status_resp
rlc_um_mac_status_indication (void *rlcP, u16_t no_tbP, u16_t tb_sizeP, struct mac_status_ind tx_statusP)
{
//-----------------------------------------------------------------------------
  struct mac_status_resp status_resp;

  if (rlcP) {
  ((struct rlc_um_entity *) rlcP)->nb_pdu_requested_by_mac = no_tbP - ((struct rlc_um_entity *)
                                                                       rlcP)->pdus_to_mac_layer.nb_elements;
  //((struct rlc_um_entity *) rlcP)->data_pdu_size = (tb_sizeP + 7) >> 3;
  ((struct rlc_um_entity *) rlcP)->data_pdu_size = tb_sizeP >> 3;
  ((struct rlc_um_entity *) rlcP)->data_pdu_size_in_bits = tb_sizeP;

#ifdef NODE_RG
  //  msg("[RLC_UM_LITE][RB %d] MAC_STATUS_INDICATION data_pdu_size_in_bits = %d\n", ((struct rlc_um_entity *) rlcP)->rb_id, tb_sizeP);
#endif

  status_resp.buffer_occupancy_in_bytes = rlc_um_get_buffer_occupancy ((struct rlc_um_entity *) rlcP);

  // LG + HA : approximation of num of transport blocks 21/10/2008
  if (status_resp.buffer_occupancy_in_bytes == 0 ) {
    status_resp.buffer_occupancy_in_pdus = 0;
  } else  if ((status_resp.buffer_occupancy_in_bytes + 1)  <=  ((struct rlc_um_entity *) rlcP)->data_pdu_size) {
    status_resp.buffer_occupancy_in_pdus = 1;
  } else {
    status_resp.buffer_occupancy_in_pdus = status_resp.buffer_occupancy_in_bytes / (((struct rlc_um_entity *)
										     rlcP)->data_pdu_size - 1);
    if ( (    status_resp.buffer_occupancy_in_bytes % (((struct rlc_um_entity *)rlcP)->data_pdu_size - 1))  > 0 ) {
      status_resp.buffer_occupancy_in_pdus +=  1;
    }
  }
  status_resp.rlc_info.rlc_protocol_state = ((struct rlc_um_entity *) rlcP)->protocol_state;
#ifdef DEBUG_RLC_UM_TX_STATUS
  if (((struct rlc_um_entity *) rlcP)->rb_id > 0) {
    msg ("[RLC_UM_LITE][RB %d] MAC_STATUS_INDICATION (DATA) %d TBs -> %d TBs\n", ((struct rlc_um_entity *) rlcP)->rb_id, no_tbP, status_resp.buffer_occupancy_in_pdus);
    if ((tx_statusP.tx_status == MAC_TX_STATUS_SUCCESSFUL) && (tx_statusP.no_pdu)) {
      msg ("[RLC_UM_LITE][RB %d] MAC_STATUS_INDICATION  TX STATUS   SUCCESSFUL %d PDUs\n", ((struct rlc_um_entity *)
                                                                                       rlcP)->rb_id, tx_statusP.no_pdu);
    }
    if ((tx_statusP.tx_status == MAC_TX_STATUS_UNSUCCESSFUL) && (tx_statusP.no_pdu)) {
      msg ("[RLC_UM_LITE][RB %d] MAC_STATUS_INDICATION  TX STATUS UNSUCCESSFUL %d PDUs\n", ((struct rlc_um_entity *)
                                                                                       rlcP)->rb_id, tx_statusP.no_pdu);
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
  list_add_list (&((struct rlc_um_entity *) rlcP)->pdus_to_mac_layer, &data_req.data);
#ifdef DEBUG_RLC_STATS
  ((struct rlc_um_entity *) rlcP)->tx_pdus += data_req.data.nb_elements;
#endif

#ifdef DEBUG_RLC_UM_MAC_DATA_REQUEST
  if (((struct rlc_um_entity *) rlcP)->rb_id > 10) {
    msg ("[RLC_UM] TTI %d: MAC_DATA_REQUEST %d TBs on RB %d\n",
	 Mac_rlc_xface->frame,
	 data_req.data.nb_elements,
	 ((struct rlc_um_entity *) rlcP)->rb_id);
  }
#endif
  data_req.buffer_occupancy_in_bytes = rlc_um_get_buffer_occupancy ((struct rlc_um_entity *) rlcP);
  data_req.buffer_occupancy_in_pdus = data_req.buffer_occupancy_in_bytes / ((struct rlc_um_entity *)
                                                                            rlcP)->data_pdu_size;
  data_req.rlc_info.rlc_protocol_state = ((struct rlc_um_entity *) rlcP)->protocol_state;
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
  struct rlc_um_entity *rlc = (struct rlc_um_entity *) rlcP;
  u8_t              insert_sdu = 0;
#ifdef DEBUG_RLC_UM_DISCARD_SDU
  int             index;
#endif
#ifndef USER_MODE
  unsigned long int rlc_um_time_us;
  int min, sec, usec;
#endif

#ifdef DEBUG_RLC_UM_DATA_REQUEST
  if (rlc->rb_id > 10)
    msg ("[RLC_UM][MOD %d][RB %d] TTI %d RLC_UM_DATA_REQ size %d Bytes, BO %d , NB SDU %d current_sdu_index=%d next_sdu_index=%d\n",
     rlc->module_id,
     rlc->rb_id,
	 Mac_rlc_xface->frame,
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
    insert_sdu = 1;
  } else {

    // from 3GPP TS 25.322 V9.2.0 p66-67
    // Upon a request of unacknowledged mode data transfer from upper layer,
    // the Sender shall:
    // - if no SDU discard configuration has been made by upper layers:
    //     - only discard SDUs when the Transmission buffer is full
    // (see subclause 9.7.3);
    // - if "Timer based SDU Discard without explicit signalling" is configured:
    //     - start a timer Timer_Discard for each SDU received from upper layer
    //       (see subclause 9.7.3);
    // - schedule the RLC SDUs received from upper layer for transmission;
    // - if one or more RLC SDUs have been scheduled for transmission:
    //     - notify the lower layer of reception of data from upper layers;
    //     - perform the actions specified in subclause 11.2.2.2.

    if ((rlc->sdu_discard_mode & RLC_SDU_DISCARD_NOT_CONFIGURED)) {
      if ((rlc->input_sdus[rlc->current_sdu_index])) {
        // from 3GPP TS 25.322 V9.2.0 p70
        // SDU discard without explicit signalling:
        // ...
        // - for the first UMD PDU to be transmitted after the discard operation,
        //   the Sender shall:
        //     -increment VT(US) so that the "Sequence Number" field in this UMD
        //      PDU is incremented with two compared with the previous UMD PDU;
        //     -fill the first data octet in this UMD PDU with the first octet
        //      of an RLC SDU;
        //     -if the "Extension bit" does not indicate that the UMD PDU
        //      contains a complete SDU which is not segmented, concatenated or
        //      padded:
        //        -set the first "Length Indicator" in this UMD PDU to indicate
        //         that the previous RLC PDU was exactly filled with the last
        //         segment of an RLC SDU (to avoid that the Receiver
        //         unnecessarily discards an extra SDU).
        // In the case where the TFC selection exchange has been initiated by
        // sending the RLC Entity Info parameter to MAC, the UE may wait until
        // after it provides MAC with the requested set of UMD PDUs before
        // discarding the afore-mentioned SDU.
        // EURECOM NOTE: since there is a single thread of execution for RRC-RLC-PDCP
        // there is no TFC selection initiated.
#ifdef DEBUG_RLC_UM_DISCARD_SDU
        msg ("[RLC_UM][RB %d] SDU DISCARDED : BUFFER OVERFLOW, BO %d , NB SDU %d\n", rlc->rb_id, rlc->buffer_occupancy, rlc->nb_sdu);
#endif
        //rlc->vt_us = (rlc->vt_us + 1) & 0x7F; // already incremented by one after previous transmission
        //rlc->do_sdu_discard_without_explicit_signalling_procedure = 1;

        if (((struct rlc_um_tx_sdu_management *) (rlc->input_sdus[rlc->current_sdu_index]->data))->sdu_remaining_size !=
            ((struct rlc_um_tx_sdu_management *) (rlc->input_sdus[rlc->current_sdu_index]->data))->sdu_size) {
#ifdef DEBUG_RLC_UM_VT_US
          msg ("[RLC_UM][RB %d] Inc VT(US) in rlc_um_data_req()/discarding SDU\n", rlc->rb_id);
#endif
          // LGv9 rlc->li_one_byte_short_to_add_in_next_pdu = 0;
          // LGv9 rlc->li_exactly_filled_to_add_in_next_pdu = 1;
          rlc->buffer_occupancy -= ((struct rlc_um_tx_sdu_management *)
                                    (rlc->input_sdus[rlc->current_sdu_index]->data))->sdu_remaining_size;
        } else {
          rlc->buffer_occupancy -= ((struct rlc_um_tx_sdu_management *)
                                    (rlc->input_sdus[rlc->current_sdu_index]->data))->sdu_size;
        }
        rlc->nb_sdu -= 1;
        free_mem_block (rlc->input_sdus[rlc->current_sdu_index]);
        rlc->input_sdus[rlc->current_sdu_index] = NULL;
        insert_sdu = 1;
        rlc->current_sdu_index = (rlc->current_sdu_index + 1) % rlc->size_input_sdus_buffer;
#ifdef DEBUG_RLC_UM_DISCARD_SDU
        //msg ("[RLC_UM_LITE][RB %d] DISCARD RESULT:\n", rlc->rb_id);
        //msg ("[RLC_UM_LITE][RB %d] size input buffer=%d current_sdu_index=%d next_sdu_index=%d\n", rlc->rb_id, rlc->size_input_sdus_buffer, rlc->current_sdu_index, rlc->next_sdu_index);
        //for (index = 0; index < rlc->size_input_sdus_buffer; index++) {
        //msg ("[RLC_UM_LITE][RB %d] BUFFER[%d]=%p\n", rlc->rb_id, index, rlc->input_sdus[index]);
        //}
#endif
      } else {
#ifdef DEBUG_RLC_UM_DISCARD_SDU
        msg ("[RLC_UM][RB %d] DISCARD : BUFFER OVERFLOW ERROR : SHOULD FIND A SDU\n", rlc->rb_id);
        msg ("[RLC_UM][RB %d] size input buffer=%d current_sdu_index=%d next_sdu_index=%d\n", rlc->rb_id, rlc->size_input_sdus_buffer, rlc->current_sdu_index, rlc->next_sdu_index);
        for (index = 0; index < rlc->size_input_sdus_buffer; index++) {
          msg ("[RLC_UM][RB %d] BUFFER[%d]=%p\n", rlc->rb_id, index, rlc->input_sdus[index]);
        }
#endif
      }
    }
  }
  if ((insert_sdu)) {
    rlc->input_sdus[rlc->next_sdu_index] = sduP;
    // IMPORTANT : do not change order of affectations
    ((struct rlc_um_tx_sdu_management *) (sduP->data))->sdu_size = ((struct rlc_um_data_req *) (sduP->data))->data_size;
    rlc->buffer_occupancy += ((struct rlc_um_tx_sdu_management *) (sduP->data))->sdu_size;
    rlc->nb_sdu += 1;
    ((struct rlc_um_tx_sdu_management *) (sduP->data))->first_byte = &sduP->data[sizeof (struct rlc_um_data_req_alloc)];
    ((struct rlc_um_tx_sdu_management *) (sduP->data))->sdu_remaining_size = ((struct rlc_um_tx_sdu_management *)
                                                                              (sduP->data))->sdu_size;
    ((struct rlc_um_tx_sdu_management *) (sduP->data))->sdu_segmented_size = 0;
    ((struct rlc_um_tx_sdu_management *) (sduP->data))->sdu_creation_time = *rlc->frame_tick_milliseconds;
    // LG ??? WHO WROTE THAT LINE ?((struct rlc_um_tx_sdu_management *) (sduP->data))->sdu_creation_time = 0;
    rlc->next_sdu_index = (rlc->next_sdu_index + 1) % rlc->size_input_sdus_buffer;
  } else {
    //    msg("[RLC][UM] Freeing sduP (%p)\n",sduP);
    free_mem_block (sduP);
  }
}
