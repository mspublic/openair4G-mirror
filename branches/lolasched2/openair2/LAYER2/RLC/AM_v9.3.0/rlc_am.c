/*******************************************************************************

Eurecom OpenAirInterface 2
Copyright(c) 1999 - 2010 Eurecom

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
#define RLC_AM_MODULE
#define RLC_AM_C
//-----------------------------------------------------------------------------
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
#include "UTIL/LOG/log.h"
#include "UL-AM-RLC.h"
#include "DL-AM-RLC.h"
//#define TRACE_RLC_AM_DATA_REQUEST
//#define TRACE_RLC_AM_TX_STATUS
//#define TRACE_RLC_AM_TX
//#define TRACE_RLC_AM_RX
//#define TRACE_RLC_AM_BO
//-----------------------------------------------------------------------------
u32_t
rlc_am_get_buffer_occupancy_in_bytes (rlc_am_entity_t *rlcP,u32 frame)
{
//-----------------------------------------------------------------------------
  u32_t max_li_overhead;
  u32_t header_overhead;

  // priority of control trafic
  if (rlcP->status_requested) {
      if (rlcP->t_status_prohibit.running == 0) {
#ifdef TRACE_RLC_AM_BO
          if (((15  +  rlcP->num_nack_sn*(10+1)  +  rlcP->num_nack_so*(15+15+1) + 7) >> 3) > 0) {
              LOG_D(RLC, "[FRAME %05d][RLC_AM][MOD %02d][RB %02d] BO : CONTROL PDU %d bytes \n", frame, rlcP->module_id, rlcP->rb_id, ((15  +  rlcP->num_nack_sn*(10+1)  +  rlcP->num_nack_so*(15+15+1) + 7) >> 3));
          }
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
  if ((rlcP->status_buffer_occupancy + rlcP->retransmission_buffer_occupancy + rlcP->sdu_buffer_occupancy + max_li_overhead + header_overhead) > 0) {
    LOG_D(RLC, "[FRAME %05d][RLC_AM][MOD %02d][RB %02d] BO : STATUS  BUFFER %d bytes \n", frame, rlcP->module_id, rlcP->rb_id, rlcP->status_buffer_occupancy);
    LOG_D(RLC, "[FRAME %05d][RLC_AM][MOD %02d][RB %02d] BO : RETRANS BUFFER %d bytes \n", frame, rlcP->module_id,rlcP->rb_id, rlcP->retransmission_buffer_occupancy);
    LOG_D(RLC, "[FRAME %05d][RLC_AM][MOD %02d][RB %02d] BO : SDU     BUFFER %d bytes + li_overhead %d bytes header_overhead %d bytes (nb sdu not segmented %d)\n", frame, rlcP->module_id,rlcP->rb_id, rlcP->sdu_buffer_occupancy, max_li_overhead, header_overhead, rlcP->nb_sdu_no_segmented);
  }
#endif
  return rlcP->status_buffer_occupancy + rlcP->retransmission_buffer_occupancy + rlcP->sdu_buffer_occupancy + max_li_overhead + header_overhead;
}
//-----------------------------------------------------------------------------
void rlc_am_release (rlc_am_entity_t *rlcP)
//-----------------------------------------------------------------------------
{

}
//-----------------------------------------------------------------------------
void config_req_rlc_am (rlc_am_entity_t *rlcP, u32_t frame, u8_t eNB_flagP, module_id_t module_idP, rlc_am_info_t * config_amP, rb_id_t rb_idP, rb_type_t rb_typeP)
{
//-----------------------------------------------------------------------------
  LOG_D(RLC, "[MSC_MSG][FRAME %05d][RRC_%s][MOD %02d][][--- CONFIG_REQ (max_retx_threshold=%d poll_pdu=%d poll_byte=%d t_poll_retransmit=%d t_reord=%d t_status_prohibit=%d) --->][RLC_AM][MOD %02d][RB %02d]\n",
                                                                                                       frame,
                                                                                                       ( Mac_rlc_xface->Is_cluster_head[module_idP] == 1) ? "eNB":"UE",
                                                                                                       module_idP,
                                                                                                       config_amP->max_retx_threshold,
                                                                                                       config_amP->poll_pdu,
                                                                                                       config_amP->poll_byte,
                                                                                                       config_amP->t_poll_retransmit,
                                                                                                       config_amP->t_reordering,
                                                                                                       config_amP->t_status_prohibit,
                                                                                                       module_idP,
                                                                                                       rb_idP);

  rlc_am_init(rlcP,frame);
  rlc_am_set_debug_infos(rlcP, frame, eNB_flagP, module_idP, rb_idP, rb_typeP);
  rlc_am_configure(rlcP,frame,
           config_amP->max_retx_threshold,
           config_amP->poll_pdu,
           config_amP->poll_byte,
           config_amP->t_poll_retransmit,
           config_amP->t_reordering,
           config_amP->t_status_prohibit);

}
u32_t pollPDU_tab[PollPDU_pInfinity+1]={4,8,16,32,64,128,256,1024};  // What is PollPDU_pInfinity??? 1024 for now
u32_t maxRetxThreshold_tab[UL_AM_RLC__maxRetxThreshold_t32+1]={1,2,3,4,6,8,16,32};
u32_t pollByte_tab[PollByte_spare1]={25,50,75,100,125,250,375,500,750,1000,1250,1500,2000,3000,10000};  // What is PollByte_kBinfinity??? 10000 for now
u32_t PollRetransmit_tab[T_PollRetransmit_spare9]={5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100,105,110,115,120,125,130,135,140,145,150,155,160,165,170,175,180,185,190,195,200,205,210,215,220,225,230,235,240,245,250,300,350,400,450,500};
u32_t am_t_Reordering_tab[T_Reordering_spare1]={0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100,110,120,130,140,150,160,170,180,190,200};
u32_t t_StatusProhibit_tab[T_StatusProhibit_spare8]={0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100,105,110,115,120,125,130,135,140,145,150,155,160,165,170,175,180,185,190,195,200,205,210,215,220,225,230,235,240,245,250,300,350,400,450,500};

//-----------------------------------------------------------------------------
void config_req_rlc_am_asn1 (rlc_am_entity_t *rlcP, u32_t frame, u8_t eNB_flagP, module_id_t module_idP, struct RLC_Config__am * config_amP, rb_id_t rb_idP, rb_type_t rb_typeP)
{
//-----------------------------------------------------------------------------
  if (	(config_amP->ul_AM_RLC.maxRetxThreshold <= UL_AM_RLC__maxRetxThreshold_t32) &&
	(config_amP->ul_AM_RLC.pollPDU<=PollPDU_pInfinity) &&
	(config_amP->ul_AM_RLC.pollByte<PollByte_spare1) &&
	(config_amP->ul_AM_RLC.t_PollRetransmit<T_PollRetransmit_spare9) &&
	(config_amP->dl_AM_RLC.t_Reordering<T_Reordering_spare1) &&
	(config_amP->dl_AM_RLC.t_StatusProhibit<T_StatusProhibit_spare8) ){

    LOG_D(RLC, "[MSC_MSG][FRAME %05d][RRC_%s][MOD %02d][][--- CONFIG_REQ (max_retx_threshold=%d poll_pdu=%d poll_byte=%d t_poll_retransmit=%d t_reord=%d t_status_prohibit=%d) --->][RLC_AM][MOD %02d][RB %02d]\n",
	  frame,
	  ( Mac_rlc_xface->Is_cluster_head[module_idP] == 1) ? "eNB":"UE",
	  module_idP,
	  maxRetxThreshold_tab[config_amP->ul_AM_RLC.maxRetxThreshold],
	  pollPDU_tab[config_amP->ul_AM_RLC.pollPDU],
	  pollByte_tab[config_amP->ul_AM_RLC.pollByte],
	  PollRetransmit_tab[config_amP->ul_AM_RLC.t_PollRetransmit],
	  am_t_Reordering_tab[config_amP->dl_AM_RLC.t_Reordering],
	  t_StatusProhibit_tab[config_amP->dl_AM_RLC.t_StatusProhibit],
	  module_idP,
	  rb_idP);
    
    rlc_am_init(rlcP,frame);
    rlc_am_set_debug_infos(rlcP, frame, eNB_flagP, module_idP, rb_idP, rb_typeP);
    rlc_am_configure(rlcP,frame,
		     maxRetxThreshold_tab[config_amP->ul_AM_RLC.maxRetxThreshold],
		     pollPDU_tab[config_amP->ul_AM_RLC.pollPDU],
		     pollByte_tab[config_amP->ul_AM_RLC.pollByte],
		     PollRetransmit_tab[config_amP->ul_AM_RLC.t_PollRetransmit],
		     am_t_Reordering_tab[config_amP->dl_AM_RLC.t_Reordering],
		     t_StatusProhibit_tab[config_amP->dl_AM_RLC.t_StatusProhibit]);
    
  }
  else {
    LOG_D(RLC, "[MSC_MSG][FRAME %05d][RRC_%s][MOD %02d][][--- ILLEGAL CONFIG_REQ (max_retx_threshold=%d poll_pdu=%d poll_byte=%d t_poll_retransmit=%d t_reord=%d t_status_prohibit=%d) --->][RLC_AM][MOD %02d][RB %02d], RLC-AM NOT CONFIGURED\n",
	  frame,
	  ( Mac_rlc_xface->Is_cluster_head[module_idP] == 1) ? "eNB":"UE",
	  module_idP,
	  config_amP->ul_AM_RLC.maxRetxThreshold,
	  config_amP->ul_AM_RLC.pollPDU,
	  config_amP->ul_AM_RLC.pollByte,
	  config_amP->ul_AM_RLC.t_PollRetransmit,
	  config_amP->dl_AM_RLC.t_Reordering,
	  config_amP->dl_AM_RLC.t_StatusProhibit,
	  module_idP,
	  rb_idP);
  }
}

  //-----------------------------------------------------------------------------
void rlc_am_stat_req     (rlc_am_entity_t *rlcP,
                              unsigned int* stat_tx_pdcp_sdu,
                              unsigned int* stat_tx_pdcp_bytes,
                              unsigned int* stat_tx_pdcp_sdu_discarded,
                              unsigned int* stat_tx_pdcp_bytes_discarded,
                              unsigned int* stat_tx_data_pdu,
                              unsigned int* stat_tx_data_bytes,
                              unsigned int* stat_tx_retransmit_pdu_by_status,
                              unsigned int* stat_tx_retransmit_bytes_by_status,
                              unsigned int* stat_tx_retransmit_pdu,
                              unsigned int* stat_tx_retransmit_bytes,
                              unsigned int* stat_tx_control_pdu,
                              unsigned int* stat_tx_control_bytes,
                              unsigned int* stat_rx_pdcp_sdu,
                              unsigned int* stat_rx_pdcp_bytes,
                              unsigned int* stat_rx_data_pdus_duplicate,
                              unsigned int* stat_rx_data_bytes_duplicate,
                              unsigned int* stat_rx_data_pdu,
                              unsigned int* stat_rx_data_bytes,
                              unsigned int* stat_rx_data_pdu_dropped,
                              unsigned int* stat_rx_data_bytes_dropped,
                              unsigned int* stat_rx_data_pdu_out_of_window,
                              unsigned int* stat_rx_data_bytes_out_of_window,
                              unsigned int* stat_rx_control_pdu,
                              unsigned int* stat_rx_control_bytes,
                              unsigned int* stat_timer_reordering_timed_out,
                              unsigned int* stat_timer_poll_retransmit_timed_out,
                              unsigned int* stat_timer_status_prohibit_timed_out)
//-----------------------------------------------------------------------------
{
    *stat_tx_pdcp_sdu                     = rlcP->stat_tx_pdcp_sdu;
    *stat_tx_pdcp_bytes                   = rlcP->stat_tx_pdcp_bytes;
    *stat_tx_pdcp_sdu_discarded           = rlcP->stat_tx_pdcp_sdu_discarded;
    *stat_tx_pdcp_bytes_discarded         = rlcP->stat_tx_pdcp_bytes_discarded;
    *stat_tx_data_pdu                     = rlcP->stat_tx_data_pdu;
    *stat_tx_data_bytes                   = rlcP->stat_tx_data_bytes;
    *stat_tx_retransmit_pdu_by_status     = rlcP->stat_tx_retransmit_pdu_by_status;
    *stat_tx_retransmit_bytes_by_status   = rlcP->stat_tx_retransmit_bytes_by_status;
    *stat_tx_retransmit_pdu               = rlcP->stat_tx_retransmit_pdu;
    *stat_tx_retransmit_bytes             = rlcP->stat_tx_retransmit_bytes;
    *stat_tx_control_pdu                  = rlcP->stat_tx_control_pdu;
    *stat_tx_control_bytes                = rlcP->stat_tx_control_bytes;
    *stat_rx_pdcp_sdu                     = rlcP->stat_rx_pdcp_sdu;
    *stat_rx_pdcp_bytes                   = rlcP->stat_rx_pdcp_bytes;
    *stat_rx_data_pdus_duplicate          = rlcP->stat_rx_data_pdus_duplicate;
    *stat_rx_data_bytes_duplicate         = rlcP->stat_rx_data_bytes_duplicate;
    *stat_rx_data_pdu                     = rlcP->stat_rx_data_pdu;
    *stat_rx_data_bytes                   = rlcP->stat_rx_data_bytes;
    *stat_rx_data_pdu_dropped             = rlcP->stat_rx_data_pdu_dropped;
    *stat_rx_data_bytes_dropped           = rlcP->stat_rx_data_bytes_dropped;
    *stat_rx_data_pdu_out_of_window       = rlcP->stat_rx_data_pdu_out_of_window;
    *stat_rx_data_bytes_out_of_window     = rlcP->stat_rx_data_bytes_out_of_window;
    *stat_rx_control_pdu                  = rlcP->stat_rx_control_pdu;
    *stat_rx_control_bytes                = rlcP->stat_rx_control_bytes;
    *stat_timer_reordering_timed_out      = rlcP->stat_timer_reordering_timed_out;
    *stat_timer_poll_retransmit_timed_out = rlcP->stat_timer_poll_retransmit_timed_out;
    *stat_timer_status_prohibit_timed_out = rlcP->stat_timer_status_prohibit_timed_out;

}
//-----------------------------------------------------------------------------
void
rlc_am_get_pdus (rlc_am_entity_t *rlcP,u32_t frame)
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
                    rlc_am_send_status_pdu(rlcP,frame);
                    mem_block_t* pdu = list_remove_head(&rlcP->control_pdu_list);
                    if (pdu) {
                        list_add_tail_eurecom (pdu, &rlcP->pdus_to_mac_layer);
                        rlcP->status_requested = 0;
                        rlc_am_start_timer_status_prohibit(rlcP,frame);
                        return;
                    }
                }
                  else {
                      LOG_D(RLC, "[FRAME %05d][RLC_AM][MOD %02d][RB %02d] DELAYED SENT STATUS PDU BECAUSE T-STATUS-PROHIBIT RUNNING (TIME-OUT FRAME %05d)\n",frame,  rlcP->module_id, rlcP->rb_id, rlcP->t_status_prohibit.frame_time_out);
                }
            }
            /*while ((rlcP->nb_bytes_requested_by_mac > 0) && (stay_on_this_list)) {
                mem_block_t* pdu = list_get_head(&rlcP->control_pdu_list);
                if (pdu != NULL {
                    if ( ((rlc_am_tx_control_pdu_management_t*)(pdu->data))->size <= rlcP->nb_bytes_requested_by_mac) {
                        pdu = list_remove_head(&rlcP->control_pdu_list);
#ifdef TRACE_RLC_AM_TX
                        msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d] SEND CONTROL PDU\n", ((rlc_am_entity_t *) rlcP)->module_id,((rlc_am_entity_t *) rlcP)->rb_id, frame);
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
                        mem_block_t* copy = rlc_am_retransmit_get_copy(rlcP, frame,rlcP->first_retrans_pdu_sn);
                        LOG_D(RLC, "[FRAME %05d][RLC_AM][MOD %02d][RB %02d] RE-SEND DATA PDU SN %04d   %d BYTES\n",frame,  rlcP->module_id,rlcP->rb_id, rlcP->first_retrans_pdu_sn, tx_data_pdu_management->header_and_payload_size);
                        rlcP->stat_tx_data_pdu                   += 1;
                        rlcP->stat_tx_retransmit_pdu             += 1;
                        rlcP->stat_tx_retransmit_pdu_by_status   += 1;
                        rlcP->stat_tx_data_bytes                 += tx_data_pdu_management->header_and_payload_size;
                        rlcP->stat_tx_retransmit_bytes           += tx_data_pdu_management->header_and_payload_size;
                        rlcP->stat_tx_retransmit_bytes_by_status += tx_data_pdu_management->header_and_payload_size;

                        list_add_tail_eurecom (copy, &rlcP->pdus_to_mac_layer);
                        rlcP->nb_bytes_requested_by_mac = rlcP->nb_bytes_requested_by_mac - tx_data_pdu_management->header_and_payload_size;

                        tx_data_pdu_management->retx_count += 1;
                        return;
                    } else if ((tx_data_pdu_management->retx_count >= 0) && (rlcP->nb_bytes_requested_by_mac >= RLC_AM_MIN_SEGMENT_SIZE_REQUEST)) {
                        LOG_D(RLC, "[FRAME %05d][RLC_AM][MOD %02d][RB %02d] SEND SEGMENT OF DATA PDU SN %04d MAC BYTES %d SIZE %d RTX COUNT %d  nack_so_start %d nack_so_stop %04X(hex)\n", frame, rlcP->module_id,rlcP->rb_id,
                        rlcP->first_retrans_pdu_sn,
                        rlcP->nb_bytes_requested_by_mac,
                        tx_data_pdu_management->header_and_payload_size,
                        tx_data_pdu_management->retx_count,
                        tx_data_pdu_management->nack_so_start,
                        tx_data_pdu_management->nack_so_stop);

                        mem_block_t* copy = rlc_am_retransmit_get_subsegment(rlcP, frame, rlcP->first_retrans_pdu_sn, &rlcP->nb_bytes_requested_by_mac);
                        LOG_D(RLC, "[FRAME %05d][RLC_AM][MOD %02d][RB %02d] SEND SEGMENT OF DATA PDU SN %04d (NEW SO %05d)\n", frame, rlcP->module_id,rlcP->rb_id, rlcP->first_retrans_pdu_sn, tx_data_pdu_management->nack_so_start);
                        rlcP->stat_tx_data_pdu                   += 1;
                        rlcP->stat_tx_retransmit_pdu             += 1;
                        rlcP->stat_tx_retransmit_pdu_by_status   += 1;
                        rlcP->stat_tx_data_bytes                 += (((struct mac_tb_req*)(copy->data))->tb_size_in_bits >> 3);
                        rlcP->stat_tx_retransmit_bytes           += (((struct mac_tb_req*)(copy->data))->tb_size_in_bits >> 3);
                        rlcP->stat_tx_retransmit_bytes_by_status += (((struct mac_tb_req*)(copy->data))->tb_size_in_bits >> 3);
                        list_add_tail_eurecom (copy, &rlcP->pdus_to_mac_layer);
                    } else {
                        break;
                    }
                    // update first_retrans_pdu_sn
                    while ((rlcP->first_retrans_pdu_sn != rlcP->vt_s) &&
                           (!(rlcP->pdu_retrans_buffer[rlcP->first_retrans_pdu_sn].flags.retransmit))) {
                        rlcP->first_retrans_pdu_sn = (rlcP->first_retrans_pdu_sn+1) & RLC_AM_SN_MASK;
                        LOG_D(RLC, "[FRAME %05d][RLC_AM][MOD %02d][RB %02d] UPDATED first_retrans_pdu_sn SN %04d\n", frame, rlcP->module_id, rlcP->rb_id, rlcP->first_retrans_pdu_sn);
                    };

                    display_flag = 1;
                    if (rlcP->first_retrans_pdu_sn == rlcP->vt_s) {
                        // no more pdu to be retransmited
                        rlcP->first_retrans_pdu_sn = -1;
                        display_flag = 0;
                        LOG_D(RLC, "[FRAME %05d][RLC_AM][MOD %02d][RB %02d] CLEAR first_retrans_pdu_sn\n",frame, rlcP->module_id, rlcP->rb_id);
                    }
                    if (display_flag > 0) {
                        LOG_D(RLC, "[FRAME %05d][RLC_AM][MOD %02d][RB %02d] UPDATED first_retrans_pdu_sn %04d\n", frame, rlcP->module_id, rlcP->rb_id, rlcP->first_retrans_pdu_sn);
                    }
                    return;

/* ONLY ONE TB PER TTI
                    if ((tx_data_pdu_management->retx_count >= 0) && (rlcP->nb_bytes_requested_by_mac < RLC_AM_MIN_SEGMENT_SIZE_REQUEST)) {
#ifdef TRACE_RLC_AM_TX
                      msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d] BREAK LOOP ON RETRANSMISSION BECAUSE ONLY %d BYTES ALLOWED TO TRANSMIT BY MAC\n",frame,  ((rlc_am_entity_t *) rlcP)->module_id,((rlc_am_entity_t *) rlcP)->rb_id, rlcP->nb_bytes_requested_by_mac);
#endif
                      break;
                    }*/
                }
            }
            if ((rlcP->nb_bytes_requested_by_mac > 2) && (rlcP->vt_s != rlcP->vt_ms)) {
                rlc_am_segment_10(rlcP,frame);
                list_add_list (&rlcP->segmentation_pdu_list, &rlcP->pdus_to_mac_layer);
                if (rlcP->pdus_to_mac_layer.head != NULL) {
                    rlcP->stat_tx_data_pdu                   += 1;
                    rlcP->stat_tx_data_bytes                 += (((struct mac_tb_req*)(rlcP->pdus_to_mac_layer.head->data))->tb_size_in_bits >> 3);
                    return;
                }
            }
            if ((rlcP->pdus_to_mac_layer.head == NULL) && (rlc_am_is_timer_poll_retransmit_timed_out(rlcP)) && (rlcP->nb_bytes_requested_by_mac > 2)) {
                rlc_am_retransmit_any_pdu(rlcP,frame);
                return;
            } else {
                LOG_D(RLC, "[FRAME %05d][RLC_AM][MOD %02d][RB %02d] COULD NOT RETRANSMIT ANY PDU BECAUSE ",frame,  rlcP->module_id, rlcP->rb_id);
                if (rlcP->pdus_to_mac_layer.head != NULL) {
                    LOG_D(RLC, "THERE ARE SOME PDUS READY TO TRANSMIT ");
                }
                if (!(rlc_am_is_timer_poll_retransmit_timed_out(rlcP))) {
                    LOG_D(RLC, "TIMER POLL DID NOT TIMED OUT (RUNNING = %d NUM PDUS TO RETRANS = %d  NUM BYTES TO RETRANS = %d) ", rlcP->t_poll_retransmit.running, rlcP->retrans_num_pdus, rlcP->retrans_num_bytes_to_retransmit);
                }
                if (rlcP->nb_bytes_requested_by_mac <= 2) {
                    LOG_D(RLC, "NUM BYTES REQUESTED BY MAC = %d", rlcP->nb_bytes_requested_by_mac);
                }
                LOG_D(RLC, "\n");
            }
            break;

        default:
            LOG_E(RLC, "[FRAME %05d][RLC_AM][MOD %02d][RB %02d] MAC_DATA_REQ UNKNOWN PROTOCOL STATE 0x%02X\n", frame, rlcP->module_id, rlcP->rb_id, rlcP->protocol_state);
    }
}
//-----------------------------------------------------------------------------
void
rlc_am_rx (void *argP, u32_t frame, u8_t eNB_flag, struct mac_data_ind data_indP)
{
//-----------------------------------------------------------------------------

  rlc_am_entity_t *rlc = (rlc_am_entity_t *) argP;

  switch (rlc->protocol_state) {

      case RLC_NULL_STATE:
       LOG_N(RLC, "[RLC_AM %p] ERROR MAC_DATA_IND IN RLC_NULL_STATE\n", argP);
        list_free (&data_indP.data);
        break;

      case RLC_DATA_TRANSFER_READY_STATE:
        rlc_am_receive_routing (rlc, frame, eNB_flag, data_indP);
        break;

      default:
        LOG_E(RLC, "[RLC_AM %p] TX UNKNOWN PROTOCOL STATE 0x%02X\n", rlc, rlc->protocol_state);
  }
}

//-----------------------------------------------------------------------------
struct mac_status_resp
rlc_am_mac_status_indication (void *rlcP, u32 frame, u16 tb_sizeP, struct mac_status_ind tx_statusP)
{
//-----------------------------------------------------------------------------
  struct mac_status_resp  status_resp;
  u16_t  sdu_size = 0;
  u16_t  sdu_remaining_size = 0;
  s32_t diff_time=0;
  rlc_am_entity_t *rlc = (rlc_am_entity_t *) rlcP;

  status_resp.buffer_occupancy_in_bytes        = 0;
  status_resp.buffer_occupancy_in_pdus         = 0;
  status_resp.head_sdu_remaining_size_to_send  = 0;
  status_resp.head_sdu_creation_time           = 0;
  status_resp.head_sdu_is_segmented            = 0;
  status_resp.rlc_info.rlc_protocol_state = rlc->protocol_state;

  if (rlc->last_frame_status_indication != frame) {
    rlc_am_check_timer_poll_retransmit(rlc,frame);
    rlc_am_check_timer_reordering(rlc,frame);
    rlc_am_check_timer_status_prohibit(rlc,frame);
  }
  rlc->last_frame_status_indication = frame;

  rlc->nb_bytes_requested_by_mac = tb_sizeP;

  status_resp.buffer_occupancy_in_bytes = rlc_am_get_buffer_occupancy_in_bytes(rlc,frame);
  
  if ((rlc->input_sdus[rlc->current_sdu_index].mem_block != NULL) && (status_resp.buffer_occupancy_in_bytes)) {
          
	  //status_resp.buffer_occupancy_in_bytes += ((rlc_am_entity_t *) rlc)->tx_header_min_length_in_bytes;
	  status_resp.buffer_occupancy_in_pdus = rlc->nb_sdu;
	  diff_time =   frame - ((rlc_am_tx_sdu_management_t *) (rlc->input_sdus[rlc->current_sdu_index].mem_block->data))->sdu_creation_time;
	  
	  status_resp.head_sdu_creation_time = (diff_time > 0 ) ? (u32_t) diff_time :  (u32_t)(0xffffffff - diff_time + frame) ;
	  
	  sdu_size            = ((rlc_am_tx_sdu_management_t *) (rlc->input_sdus[rlc->current_sdu_index].mem_block->data))->sdu_size;
	  sdu_remaining_size  = ((rlc_am_tx_sdu_management_t *) (rlc->input_sdus[rlc->current_sdu_index].mem_block->data))->sdu_remaining_size;
	  
	  status_resp.head_sdu_remaining_size_to_send = sdu_remaining_size;
	  if (sdu_size == sdu_remaining_size)  {
           status_resp.head_sdu_is_segmented = 0; 
	  }
	  else {
	   status_resp.head_sdu_is_segmented = 1; 
	  }
	
  } else {
  }
  
  
#ifdef TRACE_RLC_AM_TX_STATUS
  if (tb_sizeP > 0) {
      LOG_D(RLC, "[FRAME %05d][RLC_AM][MOD %02d][RB %02d] MAC_STATUS_INDICATION (DATA) %d bytes -> %d bytes\n", frame, rlc->module_id, rlc->rb_id, tb_sizeP, status_resp.buffer_occupancy_in_bytes);
      /*if ((tx_statusP.tx_status == MAC_TX_STATUS_SUCCESSFUL) && (tx_statusP.no_pdu)) {
          msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d] MAC_STATUS_INDICATION  TX STATUS   SUCCESSFUL %d PDUs\n",rlc->module_id,
rlc->rb_id, frame, tx_statusP.no_pdu);
      }
      if ((tx_statusP.tx_status == MAC_TX_STATUS_UNSUCCESSFUL) && (tx_statusP.no_pdu)) {
          msg ("[FRAME %05d][RLC_AM][MOD %02d][RB %02d] MAC_STATUS_INDICATION  TX STATUS UNSUCCESSFUL %d PDUs\n",rlc->module_id, rlc->rb_id,
frame, tx_statusP.no_pdu);
      }*/
  }
#endif
  return status_resp;
}
//-----------------------------------------------------------------------------
struct mac_data_req
rlc_am_mac_data_request (void *rlcP,u32 frame)
{
//-----------------------------------------------------------------------------
  struct mac_data_req data_req;
  rlc_am_entity_t *l_rlc = (rlc_am_entity_t *) rlcP;
  unsigned int nb_bytes_requested_by_mac = ((rlc_am_entity_t *) rlcP)->nb_bytes_requested_by_mac;

  rlc_am_get_pdus (rlcP,frame);

  list_init (&data_req.data, NULL);
  list_add_list (&l_rlc->pdus_to_mac_layer, &data_req.data);
  //((rlc_am_entity_t *) rlcP)->tx_pdus += data_req.data.nb_elements;
  if ((nb_bytes_requested_by_mac + data_req.data.nb_elements) > 0) {
      LOG_D(RLC, "[FRAME %05d][RLC_AM][MOD %02d][RB %02d] MAC_DATA_REQUEST %05d BYTES REQUESTED -> %d TBs\n", frame, l_rlc->module_id,l_rlc->rb_id, nb_bytes_requested_by_mac, data_req.data.nb_elements);
  }
  data_req.buffer_occupancy_in_bytes   = rlc_am_get_buffer_occupancy_in_bytes(l_rlc,frame);
  data_req.rlc_info.rlc_protocol_state = l_rlc->protocol_state;

  if (data_req.data.nb_elements > 0) {
      LOG_D(RLC, "[RLC_AM][MOD %d][RB %d][FRAME %05d] MAC_DATA_REQUEST %d TBs\n", l_rlc->module_id, l_rlc->rb_id, frame, data_req.data.nb_elements);
      mem_block_t *tb;
      rlc[l_rlc->module_id].m_mscgen_trace_length = sprintf(rlc[l_rlc->module_id].m_mscgen_trace, "[MSC_MSG][FRAME %05d][RLC_AM][MOD %02d][RB %02d][--- MAC_DATA_REQ/ %d TB(s) ",
              frame,
              l_rlc->module_id,
              l_rlc->rb_id,
              data_req.data.nb_elements);

      tb = data_req.data.head;

      while (tb != NULL) {

          if ((((struct mac_tb_req *) (tb->data))->data_ptr[0] & RLC_DC_MASK) == RLC_DC_DATA_PDU ) {
              rlc[l_rlc->module_id].m_mscgen_trace_length += sprintf(&rlc[l_rlc->module_id].m_mscgen_trace[rlc[l_rlc->module_id].m_mscgen_trace_length], " SN %d %d Bytes ",
                                                                 (((struct mac_tb_req *) (tb->data))->data_ptr[1]) +  (((u16_t)((((struct mac_tb_req *) (tb->data))->data_ptr[0]) & 0x03)) << 8),
                                                                 ((struct mac_tb_req *) (tb->data))->tb_size_in_bits>>3);
          } else {
              if ((((struct mac_tb_req *) (tb->data))->data_ptr[1] & 0x02) == 0 ) {
                  rlc[l_rlc->module_id].m_mscgen_trace_length += sprintf(&rlc[l_rlc->module_id].m_mscgen_trace[rlc[l_rlc->module_id].m_mscgen_trace_length], " STATUS ACK SN %d ... %d Bytes ",
                                                                 (((struct mac_tb_req *) (tb->data))->data_ptr[1] >> 2) +  (((u16_t)((((struct mac_tb_req *) (tb->data))->data_ptr[0]) & 0x0F)) << 8),
                                                                 ((struct mac_tb_req *) (tb->data))->tb_size_in_bits>>3);
              } else {
                  rlc[l_rlc->module_id].m_mscgen_trace_length += sprintf(&rlc[l_rlc->module_id].m_mscgen_trace[rlc[l_rlc->module_id].m_mscgen_trace_length], " STATUS ACK SN %d %d Bytes ",
                                                                 (((struct mac_tb_req *) (tb->data))->data_ptr[1] >> 2) +  (((u16_t)((((struct mac_tb_req *) (tb->data))->data_ptr[0]) & 0x0F)) << 8),
                                                                 ((struct mac_tb_req *) (tb->data))->tb_size_in_bits>>3);
              }
          }
          tb = tb->next;
      }
      rlc[l_rlc->module_id].m_mscgen_trace_length += sprintf(&rlc[l_rlc->module_id].m_mscgen_trace[rlc[l_rlc->module_id].m_mscgen_trace_length], "BO=%d --->][MAC_%s][MOD %02d][]\n",
            data_req.buffer_occupancy_in_bytes,
            (l_rlc->is_enb) ? "eNB":"UE",
            l_rlc->module_id);
      rlc[l_rlc->module_id].m_mscgen_trace[rlc[l_rlc->module_id].m_mscgen_trace_length] = 0;
      LOG_D(RLC, "%s", rlc[l_rlc->module_id].m_mscgen_trace);
  }

  return data_req;
}
//-----------------------------------------------------------------------------
void
rlc_am_mac_data_indication (void *rlcP, u32_t frame, u8 eNB_flag, struct mac_data_ind data_indP)
{
//-----------------------------------------------------------------------------
    rlc_am_entity_t           *l_rlc = (rlc_am_entity_t *) rlcP;
    rlc_am_pdu_info_t         pdu_info;
    rlc_am_control_pdu_info_t control_pdu_info;
    mem_block_t               *tb;
    int                       num_li;
    int                       num_nack;

    if (data_indP.data.nb_elements > 0) {
        LOG_D(RLC, "[RLC_AM][MOD %d][RB %d][FRAME %05d] MAC_DATA_IND %d TBs\n", l_rlc->module_id, l_rlc->rb_id, frame, data_indP.data.nb_elements);
        rlc[l_rlc->module_id].m_mscgen_trace_length = sprintf(rlc[l_rlc->module_id].m_mscgen_trace, "[MSC_MSG][FRAME %05d][MAC_%s][MOD %02d][][--- MAC_DATA_IND/ %d TB(s) ",
              frame,
              (l_rlc->is_enb) ? "eNB":"UE",
              l_rlc->module_id,
              data_indP.data.nb_elements);

        tb = data_indP.data.head;
        while (tb != NULL) {
            if ((((struct mac_tb_ind *) (tb->data))->data_ptr[0] & RLC_DC_MASK) == RLC_DC_DATA_PDU ) {
                rlc_am_get_data_pdu_infos(frame,(rlc_am_pdu_sn_10_t*) ((struct mac_tb_ind *) (tb->data))->data_ptr, (s16_t) ((struct mac_tb_ind *) (tb->data))->size, &pdu_info);
                rlc[l_rlc->module_id].m_mscgen_trace_length += sprintf(&rlc[l_rlc->module_id].m_mscgen_trace[rlc[l_rlc->module_id].m_mscgen_trace_length],
                                                                       "\\nSN %d %c%c%c%c%c",
                                                                       (((struct mac_tb_ind *) (tb->data))->data_ptr[1]) +  (((u16_t)((((struct mac_tb_ind *) (tb->data))->data_ptr[0]) & 0x03)) << 8),
                                                                       (((struct mac_tb_ind *) (tb->data))->data_ptr[0] & 0x40) ?  'R':'_',
                                                                       (((struct mac_tb_ind *) (tb->data))->data_ptr[0] & 0x20) ?  'P':'_',
                                                                       (((struct mac_tb_ind *) (tb->data))->data_ptr[0] & 0x10) ?  '}':'{',
                                                                       (((struct mac_tb_ind *) (tb->data))->data_ptr[0] & 0x08) ?  '{':'}',
                                                                       (((struct mac_tb_ind *) (tb->data))->data_ptr[0] & 0x04) ?  'E':'_');
                num_li = 0;
                if (pdu_info.num_li > 0) {
                    rlc[l_rlc->module_id].m_mscgen_trace_length += sprintf(&rlc[l_rlc->module_id].m_mscgen_trace[rlc[l_rlc->module_id].m_mscgen_trace_length], "/LI(");
                    while (num_li != (pdu_info.num_li - 1)) {
                        rlc[l_rlc->module_id].m_mscgen_trace_length += sprintf(&rlc[l_rlc->module_id].m_mscgen_trace[rlc[l_rlc->module_id].m_mscgen_trace_length], "%d,",
                                                                        pdu_info.li_list[num_li]);
                        num_li += 1;
                    }
                    rlc[l_rlc->module_id].m_mscgen_trace_length += sprintf(&rlc[l_rlc->module_id].m_mscgen_trace[rlc[l_rlc->module_id].m_mscgen_trace_length], "%d)", pdu_info.li_list[num_li]);
                }

                rlc[l_rlc->module_id].m_mscgen_trace_length += sprintf(&rlc[l_rlc->module_id].m_mscgen_trace[rlc[l_rlc->module_id].m_mscgen_trace_length],
                                                                       "/%d Bytes ",
                                                                       ((struct mac_tb_ind *) (tb->data))->size);
            } else {
                rlc_am_get_control_pdu_infos((rlc_am_pdu_sn_10_t*) ((struct mac_tb_ind *) (tb->data))->data_ptr,
                                             (s16_t) ((struct mac_tb_ind *) (tb->data))->size,
                                             &control_pdu_info);

                rlc[l_rlc->module_id].m_mscgen_trace_length += sprintf(&rlc[l_rlc->module_id].m_mscgen_trace[rlc[l_rlc->module_id].m_mscgen_trace_length],
                                                                       "\\nSTATUS ACK SN %d",
                                                                       control_pdu_info.ack_sn);
                num_nack = 0;
                while  (num_nack != control_pdu_info.num_nack) {
                    if (control_pdu_info.nack_list[num_nack].e2) {
                        rlc[l_rlc->module_id].m_mscgen_trace_length += sprintf(&rlc[l_rlc->module_id].m_mscgen_trace[rlc[l_rlc->module_id].m_mscgen_trace_length],
                                                                               "\\nNACK SN %d {%d:%d}",
                                                                               control_pdu_info.nack_list[num_nack].nack_sn,
                                                                               control_pdu_info.nack_list[num_nack].so_start,
                                                                               control_pdu_info.nack_list[num_nack].so_end);
                    } else {
                        rlc[l_rlc->module_id].m_mscgen_trace_length += sprintf(&rlc[l_rlc->module_id].m_mscgen_trace[rlc[l_rlc->module_id].m_mscgen_trace_length],
                                                                               "\\nNACK SN %d",
                                                                               control_pdu_info.nack_list[num_nack].nack_sn);
                    }
                    num_nack += 1;
                }
            }
            tb = tb->next;
        }
        rlc[l_rlc->module_id].m_mscgen_trace_length += sprintf(&rlc[l_rlc->module_id].m_mscgen_trace[rlc[l_rlc->module_id].m_mscgen_trace_length], " --->][RLC_AM][MOD %02d][RB %02d]\n",
            l_rlc->module_id,
            l_rlc->rb_id);

        rlc[l_rlc->module_id].m_mscgen_trace[rlc[l_rlc->module_id].m_mscgen_trace_length] = 0;
        LOG_D(RLC, "%s", rlc[l_rlc->module_id].m_mscgen_trace);
    }
    rlc_am_rx (rlcP, frame, eNB_flag, data_indP);
}

//-----------------------------------------------------------------------------
void
rlc_am_data_req (void *rlcP, u32_t frame, mem_block_t * sduP)
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


    memset(&rlc->input_sdus[rlc->next_sdu_index], 0, sizeof(rlc_am_tx_sdu_management_t));
    rlc->input_sdus[rlc->next_sdu_index].mem_block = sduP;

    mui         = ((struct rlc_am_data_req *) (sduP->data))->mui;
    data_offset = ((struct rlc_am_data_req *) (sduP->data))->data_offset;
    data_size   = ((struct rlc_am_data_req *) (sduP->data))->data_size;
    conf        = ((struct rlc_am_data_req *) (sduP->data))->conf;


    rlc->stat_tx_pdcp_sdu   += 1;
    rlc->stat_tx_pdcp_bytes += data_size;

    rlc->input_sdus[rlc->next_sdu_index].mui      = mui;
    rlc->input_sdus[rlc->next_sdu_index].sdu_size = data_size;
    //rlc->input_sdus[rlc->next_sdu_index].confirm  = conf;

    rlc->sdu_buffer_occupancy += data_size;
    rlc->nb_sdu += 1;
    rlc->nb_sdu_no_segmented += 1;

    rlc->input_sdus[rlc->next_sdu_index].first_byte = (u8_t*)(&sduP->data[data_offset]);
    rlc->input_sdus[rlc->next_sdu_index].sdu_remaining_size = rlc->input_sdus[rlc->next_sdu_index].sdu_size;
    rlc->input_sdus[rlc->next_sdu_index].sdu_segmented_size = 0;
    rlc->input_sdus[rlc->next_sdu_index].sdu_creation_time = frame;
    rlc->input_sdus[rlc->next_sdu_index].nb_pdus = 0;
    rlc->input_sdus[rlc->next_sdu_index].nb_pdus_ack = 0;
    //rlc->input_sdus[rlc->next_sdu_index].nb_pdus_time = 0;
    //rlc->input_sdus[rlc->next_sdu_index].nb_pdus_internal_use = 0;
    rlc->input_sdus[rlc->next_sdu_index].flags.discarded = 0;
    rlc->input_sdus[rlc->next_sdu_index].flags.segmented = 0;
    rlc->input_sdus[rlc->next_sdu_index].flags.segmentation_in_progress = 0;
    rlc->input_sdus[rlc->next_sdu_index].flags.no_new_sdu_segmented_in_last_pdu = 0;
    //rlc->input_sdus[rlc->next_sdu_index].li_index_for_discard = -1;
    rlc->next_sdu_index = (rlc->next_sdu_index + 1) % RLC_AM_SDU_CONTROL_BUFFER_SIZE;
    LOG_I(RLC, "[FRAME %05d][RLC_AM][MOD %02d][RB %02d] RLC_AM_DATA_REQ size %d Bytes,  NB SDU %d current_sdu_index=%d next_sdu_index=%d conf %d mui %d\n", frame, rlc->module_id, rlc->rb_id, data_size, rlc->nb_sdu, rlc->current_sdu_index, rlc->next_sdu_index, conf, mui);
  } else {
    LOG_W(RLC, "[FRAME %05d][RLC_AM][MOD %02d][RB %02d] RLC_AM_DATA_REQ BUFFER FULL, NB SDU %d current_sdu_index=%d next_sdu_index=%d size_input_sdus_buffer=%d\n", frame, rlc->module_id, rlc->rb_id, rlc->nb_sdu, rlc->current_sdu_index, rlc->next_sdu_index, RLC_AM_SDU_CONTROL_BUFFER_SIZE);
    LOG_W(RLC, "                                        input_sdus[].mem_block=%p next input_sdus[].flags.segmented=%d\n", rlc->input_sdus[rlc->next_sdu_index].mem_block, rlc->input_sdus[rlc->next_sdu_index].flags.segmented);
    rlc->stat_tx_pdcp_sdu_discarded   += 1;
    rlc->stat_tx_pdcp_bytes_discarded += ((struct rlc_am_data_req *) (sduP->data))->data_size;
    free_mem_block (sduP);
  }
}