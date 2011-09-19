#    ifndef __RLC_UM_ENTITY_H__
#        define __RLC_UM_ENTITY_H__

#        include "platform_types.h"
#        include "platform_constants.h"
#        include "list.h"
#        include "rlc_primitives.h"
#        include "rlc_def.h"
typedef struct rlc_um_entity {
  module_id_t    module_id;

  u8_t              allocation;
  u8_t              protocol_state;
  // for stats and trace purpose :
  u16_t             is_data_plane;   // act as a boolean
  //-----------------------------
  // PROTOCOL VARIABLES
  //-----------------------------
  u16_t              vt_us;
  // This state variable holds the value of the SN to be assigned for the next
  // newly generated UMD PDU. It is initially set to 0, and is updated whenever
  // the UM RLC entity delivers an UMD PDU with SN = VT(US).


  u16_t              vr_ur;        // UM receive state variable
  // This state variable holds the value of the SN of the earliest UMD PDU that
  // is still considered for reordering.
  // It is initially set to 0.


  u16_t              vr_ux;// UM t-Reordering state variable
  // This state variable holds the value of the SN following the SN of the UMD
  // PDU which triggered t-Reordering.

  u16_t              vr_uh;//UM highest received state variable
  // This state variable holds the value of the SN following the SN of the UMD
  // PDU with the highest SN among received UMD PDUs, and it serves as the
  // higher edge of the reordering window.
  // It is initially set to 0.


  //-----------------------------
  // TIMERS
  //-----------------------------
  signed int        timer_reordering;
  signed int        timer_reordering_init;
  signed int        timer_reordering_running;
  // This timer is used by the receiving side of an AM RLC entity and receiving
  // UM RLC entity in order to detect loss of RLC PDUs at lower layer (see sub
  // clauses 5.1.2.2 and 5.1.3.2). If t-Reordering is running, t-Reordering
  // shall not be started additionally, i.e. only one t-Reordering per RLC
  // entity is running at a given time.


  //*****************************************************************************
  // CONFIGURATION PARAMETERS
  //*****************************************************************************
  u8_t              sn_length;
  u8_t              header_min_length_in_bytes;
  signed int        sn_modulo;
  signed int        um_window_size;


  //u32_t            *frame_tick_milliseconds;      // pointer on this tick variable handled by RRC : READ ONLY
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

  //u16_t             nb_pdu_requested_by_mac;
  u32_t             nb_bytes_requested_by_mac;

  list_t          pdus_to_mac_layer;
  //-----------------------------
  // C-SAP
  //-----------------------------
  list_t          c_sap;
  //-----------------------------
  // Mapping info
  //-----------------------------
  u8_t              logical_channel_identity;


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

  mem_block_t    **dar_buffer; // array of rx pdus
  mem_block_t     *dar_buffer_alloc;     // allocation of the array

  //struct rlc_um_data_ind   output_rlc_primitive;// for writing header in rt_fifo

  list_t          pdus_from_mac_layer;

  //u8_t              last_reassemblied_sn:7;
  u16_t              rb_id;
  u16_t              last_reassemblied_sn;
  u16_t              last_reassemblied_missing_sn;
  u16_t              reassembly_missing_sn_detected;

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
}rlc_um_entity_t;
#    endif
