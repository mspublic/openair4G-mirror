/*******************************************************************************
    OpenAirInterface
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is
   included in this distribution in the file called "COPYING". If not,
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@eurecom.fr

  Address      : Eurecom, Compus SophiaTech 450, route des chappes, 06451 Biot, France.

 *******************************************************************************/
/***************************************************************************
                          rlc_am_entity.h  -
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr

 ***************************************************************************/
#    ifndef __RLC_AM_ENTITY_H__
#        define __RLC_AM_ENTITY_H__
//-----------------------------------------------------------------------------
#        include "mem_block.h"
#        include "rlc_am_structs.h"
#        include "rlc_def.h"
#        include "platform_types.h"
#        include "platform_constants.h"
//-----------------------------------------------------------------------------
struct rlc_am_entity {
  module_id_t    module_id;
  // for stats and trace purpose :
  uint16_t             data_plane;   // act as a boolean

  uint16_t              rb_id;
  //-----------------------------
  // polling info
  //-----------------------------
  uint16_t             poll_pdu_trigger;
  uint16_t             poll_sdu_trigger;
  uint16_t             timer_poll_trigger;
  uint16_t             timer_poll_prohibit_trigger;
  uint8_t              last_transmission_pdu_poll_trigger;
  uint8_t              last_retransmission_pdu_poll_trigger;
  uint8_t              poll_window_trigger;
  //-----------------------------
  // timers
  //-----------------------------
  list2_t         rlc_am_timer_list;
  //uint16_t timer_poll;
  //uint16_t timer_poll_prohibit;

  uint16_t             timer_poll_periodic;
  //uint16_t timer_status_prohibit;
  uint16_t             timer_status_periodic;
  
  signed int     timer_status_prohibit;
  signed int     running_timer_status_prohibit;
  
  
  mem_block_t      *timer_rst;
  uint16_t             time_out_events;
  mem_block_t      *timer_mrw;    // if NULL : no timer is running
  //uint8_t timer_mrw_is_running;

  uint8_t              max_mrw;
  //uint16_t timer_poll_init;
  //uint16_t timer_poll_prohibit_init;
  //uint16_t timer_epc_init;
  uint16_t             timer_discard_init;
  uint16_t             timer_poll_periodic_init;
  //uint16_t timer_status_prohibit_init;
  //uint16_t timer_status_periodic_init;
  uint16_t             timer_rst_init;
  uint16_t             timer_mrw_init;

  uint32_t             transmitted_pdu_types;
  int                last_tx_status_frame;

  uint32_t            *frame_tick_milliseconds;      // pointer on this tick variable handled by RRC : READ ONLY

  uint8_t              missing_pdu_indicator;
  //-----------------------------
  // tranmission
  //-----------------------------

  //struct cnt_dbl_lk_list_up  segmentation_buffer;  // output of segmentation/concatenation function
  list2_t         sdu_conf_segmented;   // contain sdu_headers:sdu processed by the segmentation unit. (we keep them for confirm)
  list2_t         sdu_discard_segmented;        // contain sdu_headers:sdu processed by the segmentation unit. (we keep them for discard)
  list2_t         sdu_discarded;        // contain sdu_headers

  list2_t         discard_procedures;   // contain procedures (struct rlc_am_discard_procedure)

  //struct cnt_dbl_lk_list_up  transmission_buffer; // output of mux module

  //uint16_t                        data_pdu_size;
  //uint16_t                        control_pdu_size;



  //struct cnt_list_up        dcch_pdus_to_mac_layer;
  //struct cnt_list_up        dtch_pdus_to_mac_layer;

  //struct list_up            dcch_pdus_from_mac_layer;
  //struct list_up            dtch_pdus_from_mac_layer;

  // implementation specific: our transmiter buffer is an array whose size must be a power of 2
#        define RLC_AM_DISCARD_REASSEMBLY_AT_LI_INDEX_0  0x00
  uint8_t              discard_reassembly_after_li;  // when received mrw sufi
  uint16_t             discard_reassembly_start_sn;
  //-----------------------------
  // management of received PDU for piggybacked status PDU and status PDU
  //-----------------------------
  struct sufi_to_insert_in_status status_in_construction[NB_MAX_SUFI];
  struct sufi_ack ack;
  uint8_t              sufi_to_insert_index;
  //-----------------------------
  // Reset
  //-----------------------------
  uint8_t              send_status_pdu_requested;
  uint8_t              reset_sequence_number;
  uint8_t              last_received_rsn;
  uint8_t              max_rst;
  //-----------------------------
  // Mapping info
  //-----------------------------
  uint8_t              dcch_logical_channel_identity;
  uint8_t              dtch_logical_channel_identity;
  uint8_t              nb_logical_channels_per_rlc;
  //-----------------------------
  // buffer occupancy measurements sent to MAC
  //-----------------------------
  // note occupancy of other buffers is deducted from nb elements in lists
  uint32_t             buffer_occupancy_retransmission_buffer;       // nb of pdus

  //**************************************************************
  // new members
  //**************************************************************
  uint8_t              allocation;
  uint8_t              location;     // UTRAN/UE
  uint8_t              protocol_state;
  //-----------------------------
  // protocol variables
  //-----------------------------
  uint16_t             first_li_in_next_pdu; // indicates :
  // value = 000000000000000 that the previous PDU was exactly
  // with the last segment of an RLC SDU and there is no LI that
  // indicates the end of the SDU in the previous RLC PDU.
  // value = 111111111111011 The last segment of an RLC SDU was one octet
  // short of exactly filling the previous RLC PDU and there is no LI that
  // indicates the end of the SDU in the previous RLC PDU. The remaining one
  // octet in the previous RLC PDU is ignored.
  // value = 111111111111110 AMD PDU: The rest of the RLC PDU includes a
  // piggybacked STATUS PDU.
  // value = 111111111111111 The rest of the RLC PDU is padding. The padding
  // length can be zero.

  uint16_t             vt_s;         // send state variable
  // The sequence number of the next PDU to be transmitted for the
  // first time(i.e. excluding retransmission). It is updated after
  // transmission of a PDU, which includes not earlier transmitted
  // PDUs and after transmission of a MRW SUFI which includes SN_MRW
  // length > VT(S).
  uint16_t             vt_a;         // Acknowledged state variable
  // The sequence number of the next in sequence PDU expected to be
  // acknowledged, which forms the lower edge of the window of acceptable
  // acknowledgments. VT(A) is updated based on a receipt of a STATUS
  // PDU including an ACK and/or MRW_ACK super field.
  uint16_t             max_dat;      // This state variable counts the number of times a PDU has been transmited
  // There is one vt(dat) for each PDU and it is incremented each time the pdu
  // is transmited. The initial value of this variable is 0;
  uint16_t             vt_ms;        // Maximun send state variable
  // The sequence number of the first PDU not allowed by the receiver
  // [the receiver will allow up to VT(MS)-1], VT(MS) = VT(A)+VT(WS).
  // This value represents the upper edge of the transmit window. The
  // transmitter shall not transmit a PDU with SN>=VT(MS). VT(MS) is
  // updated when either VT(A) or VT(WS) is updated. The PDU with SN
  // VT(S)-1 can be transmitted also when VT(S) >= VT(MS).
  uint16_t             vt_pdu;       // not used
  uint16_t             vt_sdu;       // this state variable is used when the poll every Poll_SDU SDU
  // function is used. It is incremented with 1 each SDU is
  // transmitted. When  it reaches Poll_SDU a new poll is transmitted
  // the state variable is set to 0. The poll bit should be set in the
  // PDU that contains the last segment of the SDU.
  uint16_t             vt_rst;       // Reset state variable
  // it is used to count the number of times a RESET PDU is transmitted.
  // vt(rst) is incremented with 1 each time a RESET PDU is transmitted. VT(rst) is
  // reset only upon the reception of a RESET ACK PDU, i.e. VT(RST) is not reset when
  // an RLC RESET occurs which was initiated from the peer RLC entity. The initial value
  // of this variable is 0.
  uint16_t             vt_mrw;       // MRW command send state variable
  // It is used to count the number of times a MRW command is transmitted.
  // VT(MRW) is reset when the discard procedure is terminated.
  uint16_t             vt_ws;        // transmitter window size state variable
  // The size that should be used for the transmitter window. VT(WS)
  //is set equal to the WSN field when the transmitter receives a
  // STATUS PDU including a WindowSize super-field.
  uint16_t             vr_r;         // Receive state variable
  // The sequence number of the next in sequence PDU expected to be
  // received. It is equal to SNmax+1 upon receipt of the next in
  // sequence PDU, where SNmax is the sequence number of the highest
  // received in sequence PDU. The initial value of this variable is 0
  uint16_t             vr_h;         // Highest expected state variable
  // The sequence number of the highest expected PDU. this state
  // variable is set equal to SN+1 only when a new PDU is received
  // with VR(MR)>SN>=VR(H). The initial value of this variable is 0.
  uint16_t             vr_mr;        // Maximum acceptable Receive state variable
  // The sequence number of the first PDU not allowed by the receiver.
  // It shall be set equal to SN+1 upon reception of a PDU. The
  // initial value of this variable is 0.
  int16_t             vr_ep;        // Estimated PDU counter state variable
  // The number of PDUs that should be received yet as a consequence
  // of the transmission of the latest status report. In acknowledged
  // mode, this state variable is updated at the end of each TTI. It
  // is decremented by the number of PDUs that should have been
  // received during the TTI. If VR(EP) is equal to 0, then check if
  // all PDUs requested for retransmission in the latest status report
  // have been received.

  //-----------------------------
  // C-SAP
  //-----------------------------
  list_t          c_sap;

  //-----------------------------
  // discard
  //-----------------------------
  uint8_t              sdu_discard_mode;
  uint8_t              send_mrw;
  //-----------------------------
  // tranmission
  //-----------------------------
  mem_block_t     **input_sdus;   // should be accessed as an array
  mem_block_t      *input_sdus_alloc;     // allocation of the array
  uint16_t             size_input_sdus_buffer;
  uint16_t             nb_sdu;

  uint16_t             next_sdu_index;       // next location of incoming sdu
  uint16_t             current_sdu_index;
  uint32_t             buffer_occupancy;
  // for segmentation procedures (optimization save space on stack)
  uint16_t             li[RLC_AM_SEGMENT_NB_MAX_LI_PER_PDU];


  uint16_t             configured_tx_window_size;
  // implementation specific: our transmiter buffer is an array whose size must be a power of 2
  uint16_t             recomputed_configured_tx_window_size;

  mem_block_t      *(*rlc_segment) (struct rlc_am_entity * rlcP);

  mem_block_t      *retransmission_buffer_alloc;
  mem_block_t     **retransmission_buffer;

  list2_t         retransmission_buffer_to_send;        // contains PDUs that must be immediatly retransmitted
  list_t          control;      // contains control pdus

  uint16_t             nb_pdu_requested_by_mac_on_ch1;
  uint16_t             nb_pdu_requested_by_mac_on_ch2;
  uint16_t             pdu_size;

  uint8_t              li_one_byte_short_to_add_in_next_pdu;
  uint8_t              li_exactly_filled_to_add_in_next_pdu;

  list_t          pdus_to_mac_layer_ch1;
  list_t          pdus_to_mac_layer_ch2;

  //-----------------------------
  // receiver
  //-----------------------------
  sdu_size_t       output_sdu_size_to_write;
  mem_block_t      *output_sdu_in_construction;

  mem_block_t      *receiver_buffer_alloc;
  mem_block_t     **receiver_buffer;

  uint16_t             last_reassemblied_sn;
  uint16_t             configured_rx_window_size;
  uint16_t             recomputed_configured_rx_window_size;

  list_t          pdus_from_mac_layer_ch1;
  list_t          pdus_from_mac_layer_ch2;

  //-----------------------------
  // status generation
  //-----------------------------
  mem_block_t      *holes_alloc;
  struct rlc_am_hole *holes;
  uint16_t             nb_missing_pdus;
  uint16_t             ack_sn;
  uint16_t             nb_holes;
#        ifdef BYPASS_L1
  unsigned int    pdu_counter;
#        endif


  unsigned int stat_tx_pdcp_sdu;
  unsigned int stat_tx_pdcp_sdu_discarded;
  unsigned int stat_tx_retransmit_pdu_unblock;
  unsigned int stat_tx_retransmit_pdu_by_status;
  unsigned int stat_tx_retransmit_pdu;  
  unsigned int stat_tx_data_pdu;  
  unsigned int stat_tx_control_pdu;  
  
  unsigned int stat_rx_sdu;  
  unsigned int stat_rx_error_pdu;  
  unsigned int stat_rx_data_pdu;  
  unsigned int stat_rx_data_pdu_out_of_window;  
  unsigned int stat_rx_control_pdu;  
};
#    endif
