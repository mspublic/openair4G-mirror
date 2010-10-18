/*
                               rlc_um_entity.h
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr


 ***************************************************************************/
#    ifndef __RLC_UM_ENTITY_H__
#        define __RLC_UM_ENTITY_H__

#        include "platform_types.h"
#        include "platform_constants.h"
#        include "list.h"
#        include "rlc_primitives.h"
#        include "rlc_def.h"
struct rlc_um_entity {
  module_id_t    module_id;

  u8_t              allocation;
  u8_t              protocol_state;
  // for stats and trace purpose :
  u16_t             data_plane;   // act as a boolean
  //-----------------------------
  // PROTOCOL VARIABLES
  //-----------------------------
  u8_t              vt_us;
  // This state variable contains the "Sequence Number" of the next UMD PDU to
  // be transmitted. It shall be incremented by 1 each time a UMD PDU is
  // transmitted.
  // The initial value of this variable is 0.


  u8_t              vr_us;        // Receiver Send Sequence state variable
  // This state variable is applicable only when "out of sequence SDU delivery"
  // is not configured. This state variable contains the "Sequence Number"
  // following that of the last UMD PDU received by the reception buffer
  // (see Fig. 4.3 and 4.3a). When a UMD PDU with "Sequence Number" equal to x
  // is received by the reception buffer, the state variable shall set equal to
  // x + 1.
  // The initial value of this variable is 0.


  u8_t              vr_uoh;// UM out of sequence SDU delivery highest received state variable
  // This state variable contains the "Sequence Number" of the highest numbered
  // UMD PDU that has been received.
  // The initial value of this variable is set according to subclause 11.2.3.2.

  u8_t              vr_udr;//UM duplicate avoidance and reordering send state variable.
  // This state variable contains the "Sequence Number" of the next UMD PDU that
  // is expected to be received in sequence. Its value is set according to
  // subclause 9.7.10.
  // The initial value of this variable is set according to subclause 9.7.10.


  u8_t              vr_udh;//UM duplicate avoidance and reordering highest received state variable.
  // This state variable contains the "Sequence Number" of the highest numbered
  // UMD PDU that has been received by the duplicate avoidance and reordering
  // function.
  // The initial value of this variable is set according to 9.7.10.

  u8_t              vr_udt;//UM duplicate avoidance and reordering timer state variable.
  // This state variable contains the sequence number of the UMD PDU associated
  // with Timer_DAR when the timer is running. Its value is set according to
  // subclause 9.7.10.


  u8_t              vr_um;//Maximum acceptable Receive state variable.
  // This state variable contains the "Sequence Number" of the first UMD PDU
  // that shall be rejected by the Receiver,
  // VR(UM) = VR(US) + Configured_Rx_Window_Size. This state variable is only
  // applicable when out-of-sequence reception is configured by higher layers.

  //-----------------------------
  // TIMERS
  //-----------------------------
  signed int        timer_discard;
  signed int        timer_discard_init;
  // This timer shall be used when timer-based SDU discard is configured by
  // upper layers. The value of the timer is signalled by upper layers. In the
  // transmitter, a new timer is started upon reception of an SDU from upper
  // layer.
  // In UM/TM, if a timer expires before the corresponding SDU is submitted to
  // lower layer, "SDU discard without explicit signalling" specified in
  // subclauses 11.2.4.3 and 11.1.4.2 shall be initiated. ...


  signed int        timer_osd;
  signed int        timer_osd_init;
  // This timer is used with UM out of sequence SDU delivery. It is used to
  // trigger the deleting of stored PDUs.
  // The timer is started and stopped according to subclause 11.2.3.2.


  signed int        timer_dar;
  signed int        timer_dar_init;
  // This timer is used with the UM duplicate avoidance and reordering function.
  // It is used to trigger the transfer of PDUs to the next in sequence UM RLC
  // receiver function.
  // The timer is started and stopped according to subclause 9.7.10.

  //*****************************************************************************
  // CONFIGURATION PARAMETERS
  //*****************************************************************************
  u8_t              is_uplink;
  u8_t              alternative_e_bit_interpretation;
  u8_t              dl_li_size;
  u16_t             largest_ul_pdu_size;
  u8_t              out_of_sequence_delivery;
  u8_t              duplicate_avoidance_and_reordering;
  u8_t              sn_delivery;
  u16_t             sn_value_for_delivery;
  u8_t              fixed_pdu_size;
  u8_t              flexible_pdu_size;
  u16_t             osd_window_size;
  u16_t             configured_rx_window_size;

  //-----------------------------
  u8_t              previous_pdu_used_15_bits_li;
  u8_t              previous_pdu_used_7_bits_li;
  u8_t              li_one_byte_short_to_add_in_next_pdu;
  u8_t              li_exactly_filled_to_add_in_next_pdu;
  //-----------------------------
  // discard info
  //-----------------------------
  u8_t              sdu_discard_mode;
  u8_t              sdu_discard_without_explicit_signalling;

  u32_t            *frame_tick_milliseconds;      // pointer on this tick variable handled by RRC : READ ONLY
  //-----------------------------
  // tranmission
  //-----------------------------
  // sdu communication;
  mem_block_t     **input_sdus;   // should be accessed as an array
  mem_block_t     *input_sdus_alloc;     // allocation of the array
  u16_t             size_input_sdus_buffer;
  u16_t             nb_sdu;

  u16_t             next_sdu_index;       // next location of incoming sdu
  u16_t             current_sdu_index;

  u32_t             buffer_occupancy;
  u16_t             data_pdu_size;
  u16_t             data_pdu_size_in_bits;
  u16_t             nb_pdu_requested_by_mac;


  list_t          pdus_to_mac_layer;
  //-----------------------------
  // C-SAP
  //-----------------------------
  list_t          c_sap;
  //-----------------------------
  // Mapping info
  //-----------------------------
  u8_t              logical_channel_identity;


  u16_t             first_li_in_next_pdu; // indicates :
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

  //*****************************************************************************
  // RECEIVER
  //*****************************************************************************
  //-----------------------------
  // receiver
  //-----------------------------
  // the current output sdu is the first in the list
  list_t          output_sdu_list;
  mem_block_t    *output_sdu_in_construction;
  s32_t             output_sdu_size_to_write;     // for writing in sdu

  //struct rlc_um_data_ind   output_rlc_primitive;// for writing header in rt_fifo

  list_t          pdus_from_mac_layer;

  u8_t              last_reassemblied_sn:7;
  u16_t              rb_id;


  //-----------------------------
  // STATISTICS
  //-----------------------------

  u32_t             tx_sdus;
  u32_t             rx_sdus;
  u32_t             tx_pdus;
  u32_t             rx_pdus;
  u32_t             rx_pdus_in_error;
  u8_t              first_pdu;

  unsigned int tx_pdcp_sdu;
  unsigned int tx_pdcp_sdu_discarded;
  unsigned int tx_data_pdu;
  unsigned int rx_sdu;
  unsigned int rx_error_pdu;
  unsigned int rx_data_pdu;
  unsigned int rx_data_pdu_out_of_window;
};
#    endif
