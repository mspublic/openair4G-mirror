/***************************************************************************
                          rlc_am.c  -
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr
 ***************************************************************************/
#define RLC_AM_MODULE
#define RLC_AM_C
//-----------------------------------------------------------------------------
#include "rtos_header.h"
#include "platform_types.h"
#include "platform_constants.h"
//-----------------------------------------------------------------------------
#include "rlc_am.h"
#include "rlc_am_segment.h"
#include "rlc_am_timer_poll_retransmit.h"
#include "mac_primitives.h"
#include "rlc_primitives.h"
#include "list.h"
#include "LAYER2/MAC/extern.h"
#define TRACE_RLC_AM_DATA_REQUEST
//#define TRACE_RLC_AM_TX_STATUS
#define TRACE_RLC_AM_TX
#define TRACE_RLC_AM_RX
#define TRACE_RLC_AM_BO
//-----------------------------------------------------------------------------
u32_t
rlc_am_get_buffer_occupancy_in_bytes (rlc_am_entity_t *rlcP)
{
//-----------------------------------------------------------------------------
  u32_t max_li_overhead;
  u32_t header_overhead;

  // priority of control trafic
  if (rlcP->status_requested) {
      if (rlcP->t_status_prohibit.running == 0) {
#ifdef TRACE_RLC_AM_BO
          msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d] BO : CONTROL PDU %d bytes \n", mac_xface->frame, rlcP->module_id, rlcP->rb_id, ((15  +  rlcP->num_nack_sn*(10+1)  +  rlcP->num_nack_so*(15+15+1) + 7) >> 3));
#endif
          return ((15  +  rlcP->num_nack_sn*(10+1)  +  rlcP->num_nack_so*(15+15+1) + 7) >> 3);
      }
  }

  // data traffic
  if (rlcP->nb_sdu_no_segmented <= 1) {
      max_li_overhead = 0;
  } else {
      max_li_overhead = (((rlcP->nb_sdu_no_segmented - 1) * 3) / 2) + ((rlcP->nb_sdu_no_segmented - 1) % 2);
  }
  if (rlcP->sdu_buffer_occupancy == 0) {
      header_overhead = 0;
  } else {
      header_overhead = 2;
  }


#ifdef TRACE_RLC_AM_BO
  msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d] BO : STATUS  BUFFER %d bytes \n", mac_xface->frame, rlcP->module_id, rlcP->rb_id, rlcP->status_buffer_occupancy);
  msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d] BO : RETRANS BUFFER %d bytes \n", mac_xface->frame, rlcP->module_id,rlcP->rb_id, rlcP->retransmission_buffer_occupancy);
  msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d] BO : SDU     BUFFER %d bytes + li_overhead %d bytes header_overhead %d bytes (nb sdu not segmented %d)\n", mac_xface->frame, rlcP->module_id,rlcP->rb_id, rlcP->sdu_buffer_occupancy, max_li_overhead, header_overhead, rlcP->nb_sdu_no_segmented);
#endif
  return rlcP->status_buffer_occupancy + rlcP->retransmission_buffer_occupancy + rlcP->sdu_buffer_occupancy + max_li_overhead + header_overhead;
}
//-----------------------------------------------------------------------------
void rlc_am_release (rlc_am_entity_t *rlcP)
//-----------------------------------------------------------------------------
{

}
//-----------------------------------------------------------------------------
void config_req_rlc_am (rlc_am_entity_t *rlcP, module_id_t module_idP, rlc_am_info_t * config_amP, u8_t rb_idP, rb_type_t rb_typeP)
{
//-----------------------------------------------------------------------------
    rlc_am_init(rlcP);
    rlc_am_set_debug_infos(rlcP, module_idP, rb_idP, rb_typeP);
    rlc_am_configure(rlcP,
                      config_amP->max_retx_threshold,
                      config_amP->poll_pdu,
                      config_amP->poll_byte,
                      config_amP->t_poll_retransmit,
                      config_amP->t_reordering,
                      config_amP->t_status_prohibit);

}
//-----------------------------------------------------------------------------
void rlc_am_stat_req     (rlc_am_entity_t *rlcP,
                              unsigned int* tx_pdcp_sdu,
                              unsigned int* tx_pdcp_sdu_discarded,
                              unsigned int* tx_retransmit_pdu_unblock,
                              unsigned int* tx_retransmit_pdu_by_status,
                              unsigned int* tx_retransmit_pdu,
                              unsigned int* tx_data_pdu,
                              unsigned int* tx_control_pdu,
                              unsigned int* rx_sdu,
                              unsigned int* rx_error_pdu,
                              unsigned int* rx_data_pdu,
                              unsigned int* rx_data_pdu_out_of_window,
                              unsigned int* rx_control_pdu)
//-----------------------------------------------------------------------------
{
    *tx_pdcp_sdu                 = rlcP->stat_tx_pdcp_sdu;
    *tx_pdcp_sdu_discarded       = rlcP->stat_tx_pdcp_sdu_discarded;
    *tx_retransmit_pdu_unblock   = rlcP->stat_tx_retransmit_pdu_unblock;
    *tx_retransmit_pdu_by_status = rlcP->stat_tx_retransmit_pdu_by_status;
    *tx_retransmit_pdu           = rlcP->stat_tx_retransmit_pdu;
    *tx_data_pdu                 = rlcP->stat_tx_data_pdu;
    *tx_control_pdu              = rlcP->stat_tx_control_pdu;
    *rx_sdu                      = rlcP->stat_rx_sdu;
    *rx_error_pdu                = rlcP->stat_rx_error_pdu;
    *rx_data_pdu                 = rlcP->stat_rx_data_pdu;
    *rx_data_pdu_out_of_window   = rlcP->stat_rx_data_pdu_out_of_window;
    *rx_control_pdu              = rlcP->stat_rx_control_pdu;
}
//-----------------------------------------------------------------------------
void
rlc_am_get_pdus (rlc_am_entity_t *rlcP)
{
//-----------------------------------------------------------------------------
  int display_flag = 0;
  // 5.1.3.1 Transmit operations
  // 5.1.3.1.1
  // General
  // The transmitting side of an AM RLC entity shall prioritize transmission of RLC control PDUs over RLC data PDUs.
  // The transmitting side of an AM RLC entity shall prioritize retransmission of RLC data PDUs over transmission of new
  // AMD PDUs.


    switch (rlcP->protocol_state) {

        case RLC_NULL_STATE:
            break;

        case RLC_DATA_TRANSFER_READY_STATE:
            // TRY TO SEND CONTROL PDU FIRST
            if ((rlcP->nb_bytes_requested_by_mac > 2) && (rlcP->status_requested)) {
                // When STATUS reporting has been triggered, the receiving side of an AM RLC entity shall:
                // - if t-StatusProhibit is not running:
                //     - at the first transmission opportunity indicated by lower layer, construct a STATUS PDU and deliver it to lower layer;
                // - else:
                //     - at the first transmission opportunity indicated by lower layer after t-StatusProhibit expires, construct a single
                //       STATUS PDU even if status reporting was triggered several times while t-StatusProhibit was running and
                //       deliver it to lower layer;
                //
                // When a STATUS PDU has been delivered to lower layer, the receiving side of an AM RLC entity shall:
                //     - start t-StatusProhibit.
                if (rlcP->t_status_prohibit.running == 0) {
                    rlc_am_send_status_pdu(rlcP);
                    mem_block_t* pdu = list_remove_head(&rlcP->control_pdu_list);
                    if (pdu) {
                        list_add_tail_eurecom (pdu, &rlcP->pdus_to_mac_layer);
                        rlcP->status_requested = 0;
                        rlc_am_start_timer_status_prohibit(rlcP);
                        return;
                    }
                }
#ifdef TRACE_RLC_AM_TX
                  else {
                        msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d] DELAYED SENT STATUS PDU BECAUSE T-STATUS-PROHIBIT RUNNING (TIME-OUT FRAME %05d)\n",mac_xface->frame,  rlcP->module_id, rlcP->rb_id, rlcP->t_status_prohibit.frame_time_out);
                }
#endif

            }
            /*while ((rlcP->nb_bytes_requested_by_mac > 0) && (stay_on_this_list)) {
                mem_block_t* pdu = list_get_head(&rlcP->control_pdu_list);
                if (pdu != NULL) {
                    if ( ((rlc_am_tx_control_pdu_management_t*)(pdu->data))->size <= rlcP->nb_bytes_requested_by_mac) {
                        pdu = list_remove_head(&rlcP->control_pdu_list);
#ifdef TRACE_RLC_AM_TX
                        msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d] SEND CONTROL PDU\n", ((rlc_am_entity_t *) rlcP)->module_id,((rlc_am_entity_t *) rlcP)->rb_id, mac_xface->frame);
#endif
                        list_add_tail_eurecom (pdu, &rlcP->pdus_to_mac_layer);
                        rlcP->nb_bytes_requested_by_mac = rlcP->nb_bytes_requested_by_mac - ((rlc_am_tx_control_pdu_management_t*)(pdu->data))->size;
                    } else {
                      stay_on_this_list = 0;
                    }
                } else {
                    stay_on_this_list = 0;
                }
            }*/
            // THEN TRY TO SEND RETRANS PDU
            if (rlcP->first_retrans_pdu_sn >= 0) {
                rlc_am_tx_data_pdu_management_t* tx_data_pdu_management;
                // tx min 3 bytes because of the size of the RLC header
                while ((rlcP->nb_bytes_requested_by_mac > 2) &&
                       (rlcP->first_retrans_pdu_sn  >= 0) &&
                       (rlcP->first_retrans_pdu_sn != rlcP->vt_s)) {

                    tx_data_pdu_management = &rlcP->pdu_retrans_buffer[rlcP->first_retrans_pdu_sn];

                    if ((tx_data_pdu_management->header_and_payload_size <= rlcP->nb_bytes_requested_by_mac) && (tx_data_pdu_management->retx_count >= 0) && (tx_data_pdu_management->nack_so_start == 0) && (tx_data_pdu_management->nack_so_stop == 0x7FFF)) {
                        mem_block_t* copy = rlc_am_retransmit_get_copy(rlcP, rlcP->first_retrans_pdu_sn);
#ifdef TRACE_RLC_AM_TX
                        msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d] RE-SEND DATA PDU SN %04d   %d BYTES\n",mac_xface->frame,  rlcP->module_id,rlcP->rb_id, rlcP->first_retrans_pdu_sn, tx_data_pdu_management->header_and_payload_size);
#endif
                        list_add_tail_eurecom (copy, &rlcP->pdus_to_mac_layer);
                        rlcP->nb_bytes_requested_by_mac = rlcP->nb_bytes_requested_by_mac - tx_data_pdu_management->header_and_payload_size;

                        tx_data_pdu_management->retx_count += 1;
                        return;
                    } else if ((tx_data_pdu_management->retx_count >= 0) && (rlcP->nb_bytes_requested_by_mac >= RLC_AM_MIN_SEGMENT_SIZE_REQUEST)) {
                        msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d] SEND SEGMENT OF DATA PDU SN %04d MAC BYTES %d SIZE %d RTX COUNT %d  nack_so_start %d nack_so_stop %04X(hex)\n", mac_xface->frame, rlcP->module_id,rlcP->rb_id,
                        rlcP->first_retrans_pdu_sn,
                        rlcP->nb_bytes_requested_by_mac,
                        tx_data_pdu_management->header_and_payload_size,
                        tx_data_pdu_management->retx_count,
                        tx_data_pdu_management->nack_so_start,
                        tx_data_pdu_management->nack_so_stop);

                        mem_block_t* copy = rlc_am_retransmit_get_subsegment(rlcP, rlcP->first_retrans_pdu_sn, &rlcP->nb_bytes_requested_by_mac);
#ifdef TRACE_RLC_AM_TX
                        msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d] SEND SEGMENT OF DATA PDU SN %04d (NEW SO %05d)\n", mac_xface->frame, rlcP->module_id,rlcP->rb_id, rlcP->first_retrans_pdu_sn, tx_data_pdu_management->nack_so_start);
#endif
                        list_add_tail_eurecom (copy, &rlcP->pdus_to_mac_layer);
                    } else {
                        break;
                    }
                    // update first_retrans_pdu_sn
                    while ((rlcP->first_retrans_pdu_sn != rlcP->vt_s) &&
                           (!(rlcP->pdu_retrans_buffer[rlcP->first_retrans_pdu_sn].flags.retransmit))) {
                        rlcP->first_retrans_pdu_sn = (rlcP->first_retrans_pdu_sn+1) & RLC_AM_SN_MASK;
#ifdef TRACE_RLC_AM_TX
                        msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d] UPDATED first_retrans_pdu_sn SN %04d\n", mac_xface->frame, rlcP->module_id, rlcP->rb_id, rlcP->first_retrans_pdu_sn);
#endif
                    };

                    display_flag = 1;
                    if (rlcP->first_retrans_pdu_sn == rlcP->vt_s) {
                        // no more pdu to be retransmited
                        rlcP->first_retrans_pdu_sn = -1;
                        display_flag = 0;
#ifdef TRACE_RLC_AM_TX
                        msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d] CLEAR first_retrans_pdu_sn\n",mac_xface->frame, rlcP->module_id, rlcP->rb_id);
#endif
                    }
#ifdef TRACE_RLC_AM_TX
                    if (display_flag > 0) {
                        msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d] UPDATED first_retrans_pdu_sn %04d\n", mac_xface->frame, rlcP->module_id, rlcP->rb_id, rlcP->first_retrans_pdu_sn);
                    }
#endif
                    return;

/* ONLY ONE TB PER TTI
                    if ((tx_data_pdu_management->retx_count >= 0) && (rlcP->nb_bytes_requested_by_mac < RLC_AM_MIN_SEGMENT_SIZE_REQUEST)) {
#ifdef TRACE_RLC_AM_TX
                      msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d] BREAK LOOP ON RETRANSMISSION BECAUSE ONLY %d BYTES ALLOWED TO TRANSMIT BY MAC\n",mac_xface->frame,  ((rlc_am_entity_t *) rlcP)->module_id,((rlc_am_entity_t *) rlcP)->rb_id, rlcP->nb_bytes_requested_by_mac);
#endif
                      break;
                    }*/
                }
            }
            if ((rlcP->nb_bytes_requested_by_mac > 2) && (rlcP->vt_s != rlcP->vt_ms)) {
                rlc_am_segment_10 (rlcP);
                list_add_list (&rlcP->segmentation_pdu_list, &rlcP->pdus_to_mac_layer);
                if (rlcP->pdus_to_mac_layer.head != NULL) {
                    return;
                }
            }
            if ((rlcP->pdus_to_mac_layer.head == NULL) && (rlc_am_is_timer_poll_retransmit_timed_out(rlcP)) && (rlcP->nb_bytes_requested_by_mac > 2)) {
                rlc_am_retransmit_any_pdu(rlcP);
                return;
            }
#ifdef TRACE_RLC_AM_TX
            else {
                msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d] COULD NOT RETRANSMIT ANY PDU BECAUSE ",mac_xface->frame,  rlcP->module_id, rlcP->rb_id);
                if (rlcP->pdus_to_mac_layer.head != NULL) {
                    msg ("THERE ARE SOME PDUS READY TO TRANSMIT ");
                }
                if (!(rlc_am_is_timer_poll_retransmit_timed_out(rlcP))) {
                    msg ("TIMER POLL DID NOT TIMED OUT (RUNNING = %d NUM PDUS TO RETRANS = %d  NUM BYTES TO RETRANS = %d) ", rlcP->t_poll_retransmit.running, rlcP->retrans_num_pdus, rlcP->retrans_num_bytes_to_retransmit);
                }
                if (rlcP->nb_bytes_requested_by_mac <= 2) {
                    msg ("NUM BYTES REQUESTED BY MAC = %d", rlcP->nb_bytes_requested_by_mac);
                }
                msg ("\n");
            }
#endif

            break;

        default:
            msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d] MAC_DATA_REQ UNKNOWN PROTOCOL STATE 0x%02X\n", mac_xface->frame, rlcP->module_id, rlcP->rb_id, rlcP->protocol_state);
    }
}
//-----------------------------------------------------------------------------
void
rlc_am_rx (void *argP, struct mac_data_ind data_indP)
{
//-----------------------------------------------------------------------------

  rlc_am_entity_t *rlc = (rlc_am_entity_t *) argP;

  switch (rlc->protocol_state) {

      case RLC_NULL_STATE:
        msg ("[RLC_AM %p] ERROR MAC_DATA_IND IN RLC_NULL_STATE\n", argP);
        list_free (&data_indP.data);
        break;

      case RLC_DATA_TRANSFER_READY_STATE:
        rlc_am_receive_routing (rlc, data_indP);
        break;

      default:
        msg ("[RLC_AM %p] TX UNKNOWN PROTOCOL STATE 0x%02X\n", rlc, rlc->protocol_state);
  }
}

//-----------------------------------------------------------------------------
struct mac_status_resp
rlc_am_mac_status_indication (void *rlcP, u16 tb_sizeP, struct mac_status_ind tx_statusP)
{
//-----------------------------------------------------------------------------
  struct mac_status_resp  status_resp;
  rlc_am_entity_t *rlc = (rlc_am_entity_t *) rlcP;

  if (rlc->last_frame_status_indication != mac_xface->frame) {
      rlc_am_check_timer_poll_retransmit(rlc);
      rlc_am_check_timer_reordering(rlc);
      rlc_am_check_timer_status_prohibit(rlc);
  }
  rlc->last_frame_status_indication = mac_xface->frame;

  rlc->nb_bytes_requested_by_mac = tb_sizeP;

  status_resp.buffer_occupancy_in_bytes = rlc_am_get_buffer_occupancy_in_bytes(rlc);
#ifdef TRACE_RLC_AM_TX_STATUS
  if (tb_sizeP > 0) {
      msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d] MAC_STATUS_INDICATION (DATA) %d bytes -> %d bytes\n", mac_xface->frame, rlc->module_id, rlc->rb_id, tb_sizeP, status_resp.buffer_occupancy_in_bytes);
      /*if ((tx_statusP.tx_status == MAC_TX_STATUS_SUCCESSFUL) && (tx_statusP.no_pdu)) {
          msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d] MAC_STATUS_INDICATION  TX STATUS   SUCCESSFUL %d PDUs\n",rlc->module_id,
rlc->rb_id, mac_xface->frame, tx_statusP.no_pdu);
      }
      if ((tx_statusP.tx_status == MAC_TX_STATUS_UNSUCCESSFUL) && (tx_statusP.no_pdu)) {
          msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d] MAC_STATUS_INDICATION  TX STATUS UNSUCCESSFUL %d PDUs\n",rlc->module_id, rlc->rb_id,
mac_xface->frame, tx_statusP.no_pdu);
      }*/
  }
#endif
  return status_resp;
}
//-----------------------------------------------------------------------------
struct mac_data_req
rlc_am_mac_data_request (void *rlcP)
{
//-----------------------------------------------------------------------------
  struct mac_data_req data_req;
#ifdef TRACE_RLC_AM_TX
  unsigned int nb_bytes_requested_by_mac = ((rlc_am_entity_t *) rlcP)->nb_bytes_requested_by_mac;
#endif

  rlc_am_get_pdus (rlcP);

  list_init (&data_req.data, NULL);
  list_add_list (&((rlc_am_entity_t *) rlcP)->pdus_to_mac_layer, &data_req.data);
#ifdef DEBUG_RLC_STATS
  ((rlc_am_entity_t *) rlcP)->tx_pdus += data_req.data.nb_elements;
#endif

#ifdef TRACE_RLC_AM_TX
  msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d] MAC_DATA_REQUEST %05d BYTES REQUESTED -> %d TBs\n", mac_xface->frame, ((rlc_am_entity_t *) rlcP)->module_id,((rlc_am_entity_t *) rlcP)->rb_id, nb_bytes_requested_by_mac, data_req.data.nb_elements);
#endif
  data_req.buffer_occupancy_in_bytes   = rlc_am_get_buffer_occupancy_in_bytes((rlc_am_entity_t *)rlcP);
  data_req.rlc_info.rlc_protocol_state = ((rlc_am_entity_t *) rlcP)->protocol_state;

  return data_req;
}
//-----------------------------------------------------------------------------
void
rlc_am_mac_data_indication (void *rlcP, struct mac_data_ind data_indP)
{
//-----------------------------------------------------------------------------
  rlc_am_rx (rlcP, data_indP);
}

//-----------------------------------------------------------------------------
void
rlc_am_data_req (void *rlcP, mem_block_t * sduP)
{
//-----------------------------------------------------------------------------
  rlc_am_entity_t *rlc = (rlc_am_entity_t *) rlcP;
  u32_t             mui;
  u16_t             data_offset;
  u16_t             data_size;
  u8_t              conf;


  if ((rlc->input_sdus[rlc->next_sdu_index].mem_block == NULL) &&
      (rlc->input_sdus[rlc->next_sdu_index].flags.segmented == 0) &&
      (((rlc->next_sdu_index + 1) % RLC_AM_SDU_CONTROL_BUFFER_SIZE) != rlc->current_sdu_index)) {

    rlc->stat_tx_pdcp_sdu += 1;

    memset(&rlc->input_sdus[rlc->next_sdu_index], 0, sizeof(rlc_am_tx_sdu_management_t));
    rlc->input_sdus[rlc->next_sdu_index].mem_block = sduP;

    mui         = ((struct rlc_am_data_req *) (sduP->data))->mui;
    data_offset = ((struct rlc_am_data_req *) (sduP->data))->data_offset;
    data_size   = ((struct rlc_am_data_req *) (sduP->data))->data_size;
    conf        = ((struct rlc_am_data_req *) (sduP->data))->conf;

    rlc->input_sdus[rlc->next_sdu_index].mui      = mui;
    rlc->input_sdus[rlc->next_sdu_index].sdu_size = data_size;
    //rlc->input_sdus[rlc->next_sdu_index].confirm  = conf;

    rlc->sdu_buffer_occupancy += data_size;
    rlc->nb_sdu += 1;
    rlc->nb_sdu_no_segmented += 1;

    rlc->input_sdus[rlc->next_sdu_index].first_byte = &sduP->data[data_offset];
    rlc->input_sdus[rlc->next_sdu_index].sdu_remaining_size = rlc->input_sdus[rlc->next_sdu_index].sdu_size;
    rlc->input_sdus[rlc->next_sdu_index].sdu_segmented_size = 0;
    rlc->input_sdus[rlc->next_sdu_index].sdu_creation_time = mac_xface->frame;
    rlc->input_sdus[rlc->next_sdu_index].nb_pdus = 0;
    rlc->input_sdus[rlc->next_sdu_index].nb_pdus_ack = 0;
    rlc->input_sdus[rlc->next_sdu_index].nb_pdus_time = 0;
    rlc->input_sdus[rlc->next_sdu_index].nb_pdus_internal_use = 0;
    rlc->input_sdus[rlc->next_sdu_index].flags.discarded = 0;
    rlc->input_sdus[rlc->next_sdu_index].flags.segmented = 0;
    rlc->input_sdus[rlc->next_sdu_index].flags.segmentation_in_progress = 0;
    rlc->input_sdus[rlc->next_sdu_index].flags.no_new_sdu_segmented_in_last_pdu = 0;
    rlc->input_sdus[rlc->next_sdu_index].li_index_for_discard = -1;
    rlc->next_sdu_index = (rlc->next_sdu_index + 1) % RLC_AM_SDU_CONTROL_BUFFER_SIZE;
#ifdef TRACE_RLC_AM_DATA_REQUEST
    msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d] RLC_AM_DATA_REQ size %d Bytes,  NB SDU %d current_sdu_index=%d next_sdu_index=%d conf %d mui %d\n", mac_xface->frame, rlc->module_id, rlc->rb_id, data_size, rlc->nb_sdu, rlc->current_sdu_index, rlc->next_sdu_index, conf, mui);
#endif
  } else {
#ifdef TRACE_RLC_AM_DATA_REQUEST
    msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d] RLC_AM_DATA_REQ BUFFER FULL, NB SDU %d current_sdu_index=%d next_sdu_index=%d size_input_sdus_buffer=%d\n", mac_xface->frame, rlc->module_id, rlc->rb_id, rlc->nb_sdu, rlc->current_sdu_index, rlc->next_sdu_index, RLC_AM_SDU_CONTROL_BUFFER_SIZE);
    msg ("                                        input_sdus[].mem_block=%p next input_sdus[].flags.segmented=%d\n", rlc->input_sdus[rlc->next_sdu_index].mem_block, rlc->input_sdus[rlc->next_sdu_index].flags.segmented);
#endif
    rlc->stat_tx_pdcp_sdu_discarded += 1;
    free_mem_block (sduP);
    assert(2==3);
  }
}
