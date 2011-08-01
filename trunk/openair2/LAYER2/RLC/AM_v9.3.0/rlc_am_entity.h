/***************************************************************************
                          rlc_am_entity.h  -
                             -------------------

 ***************************************************************************/
#    ifndef __RLC_AM_ENTITY_H__
#        define __RLC_AM_ENTITY_H__
//-----------------------------------------------------------------------------
#        include "platform_types.h"
#        include "platform_constants.h"
#        include "list.h"
#        include "rlc_primitives.h"
#        include "rlc_def_lte.h"
#        include "rlc_am_structs.h"
#        include "rlc_am_constants.h"
//-----------------------------------------------------------------------------
// This constant is used by both the transmitting side and the receiving side of each AM RLC entity to calculate VT(MS)
// from VT(A), and VR(MR) from VR(R). AM_Window_Size = 512.

typedef struct rlc_am_entity {
  module_id_t     module_id;                      // OK
  // for stats and trace purpose :
  u16_t           rb_id;               // OK
  boolean_t       is_data_plane;

  signed int      sdu_buffer_occupancy;    // OK
  signed int      retransmission_buffer_occupancy;  // OK
  signed int      status_buffer_occupancy;  // OK

  //---------------------------------------------------------------------
  // TX BUFFERS
  //---------------------------------------------------------------------
  // sdu communication;
  mem_block_t*                 input_sdus_alloc;
  rlc_am_tx_sdu_management_t   *input_sdus;//[RLC_AM_SDU_CONTROL_BUFFER_SIZE];
  signed int      nb_sdu;               // total number of valid rlc_am_tx_sdu_management_t in input_sdus[]
  signed int      nb_sdu_no_segmented;  // include SDUs partially segmented
  signed int      next_sdu_index;       // next location of incoming sdu
  signed int      current_sdu_index;


  mem_block_t*                    pdu_retrans_buffer_alloc;
  rlc_am_tx_data_pdu_management_t *pdu_retrans_buffer;//[RLC_AM_PDU_RETRANSMISSION_BUFFER_SIZE];
  signed int      retrans_num_pdus;
  signed int      retrans_num_bytes; // num bytes in the retransmission buffer
  signed int      retrans_num_bytes_to_retransmit; // num bytes of the retransmission buffer to retransmit
  unsigned int    num_nack_so;
  unsigned int    num_nack_sn;

  //---------------------------------------------------------------------
  // RX BUFFERS
  //---------------------------------------------------------------------
  list2_t         receiver_buffer;
  mem_block_t     *output_sdu_in_construction;
  s32_t           output_sdu_size_to_write;     // for writing in sdu


  //---------------------------------------------------------------------
  // PROTOCOL VARIABLES
  //---------------------------------------------------------------------
  u8_t            protocol_state;
  //-----------------------------
  // TX STATE VARIABLES
  //-----------------------------
  u16_t           vt_a;         // Acknowledgement state variable
  // This state variable holds the value of the SN of the next AMD PDU for which a positive acknowledgment is to be
  // received in-sequence, and it serves as the lower edge of the transmitting window. It is initially set to 0, and is updated
  // whenever the AM RLC entity receives a positive acknowledgment for an AMD PDU with SN = VT(A).

  u16_t           vt_ms;         // Maximum send state variable
  // This state variable equals VT(A) + AM_Window_Size, and it serves as the higher edge of the transmitting window.

  u16_t           vt_s;         // Send state variable
  // This state variable holds the value of the SN to be assigned for the next newly generated AMD PDU. It is initially set to
  // 0, and is updated whenever the AM RLC entity delivers an AMD PDU with SN = VT(S).

  u16_t           poll_sn;         // Poll send state variable
  // This state variable holds the value of VT(S)-1 upon the most recent transmission of a RLC data PDU with the poll bit
  // set to “1”. It is initially set to 0.


  //-----------------------------
  // RX STATE VARIABLES
  //-----------------------------
  u16_t           vr_r;         // Receive state variable
  // This state variable holds the value of the SN following the last in-sequence completely received AMD PDU, and it
  // serves as the lower edge of the receiving window. It is initially set to 0, and is updated whenever the AM RLC entity
  // receives an AMD PDU with SN = VR(R).

  u16_t           vr_mr;         //  Maximum acceptable receive state variable
  // This state variable equals VR(R) + AM_Window_Size, and it holds the value of the SN of the first AMD PDU that is
  // beyond the receiving window and serves as the higher edge of the receiving window.


  u16_t           vr_x;          // t-Reordering state variable
  // This state variable holds the value of the SN following the SN of the RLC data PDU which triggered t-Reordering..

  u16_t           vr_ms;         // Maximum STATUS transmit state variable
  // This state variable holds the highest possible value of the SN which can be indicated by “ACK_SN” when a STATUS
  // PDU needs to be constructed. It is initially set to 0.

  u16_t           vr_h;         // Highest received state variable
  // This state variable holds the value of the SN following the SN of the RLC data PDU with the highest SN among
  // received RLC data PDUs. It is initially set to 0.


  //-----------------------------
  // TIMERS CONFIGURED BY RRC
  //-----------------------------
  rlc_am_timer_t  t_poll_retransmit;
  // This timer is used by the transmitting side of an AM RLC entity in order to retransmit a poll (see sub clause 5.2.2).

  rlc_am_timer_t  t_reordering;
  // This timer is used by the receiving side of an AM RLC entity and receiving UM RLC entity in order to detect loss of
  // RLC PDUs at lower layer (see sub clauses 5.1.2.2 and 5.1.3.2). If t-Reordering is running, t-Reordering shall not be
  // started additionally, i.e. only one t-Reordering per RLC entity is running at a given time.

  rlc_am_timer_t  t_status_prohibit;
  // This timer is used by the receiving side of an AM RLC entity in order to prohibit transmission of a STATUS PDU (see
  // sub clause 5.2.3).

  //-----------------------------
  // COUNTERS
  //-----------------------------
  unsigned int    c_pdu_without_poll;
  // This counter is initially set to 0. It counts the number of AMD PDUs sent since the most recent poll bit was transmitted.

  unsigned int    c_byte_without_poll;
  // This counter is initially set to 0. It counts the number of data bytes sent since the most recent poll bit was transmitted.


  //-----------------------------
  // PARAMETERS CONFIGURED BY RRC
  //-----------------------------
  u16_t           max_retx_threshold;
  // This parameter is used by the transmitting side of each AM RLC entity to limit the number of retransmissions of an
  // AMD PDU (see subclause 5.2.1).

  u16_t           poll_pdu;
  // This parameter is used by the transmitting side of each AM RLC entity to trigger a poll for every pollPDU PDUs (see
  // subclause 5.2.2).

  u16_t           poll_byte;
  // This parameter is used by the transmitting side of each AM RLC entity to trigger a poll for every pollByte bytes (see
  // subclause 5.2.2).

  //---------------------------------------------------------------------
  // STATISTICS
  //---------------------------------------------------------------------
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
  unsigned int stat_rx_data_pdu_duplicate;
  unsigned int stat_rx_data_pdu_out_of_window;
  unsigned int stat_rx_control_pdu;

  //---------------------------------------------------------------------
  // OUTPUTS
  //---------------------------------------------------------------------
  u16_t             nb_bytes_requested_by_mac;
  list_t            pdus_to_mac_layer;
  list_t            control_pdu_list;
  s16_t             first_retrans_pdu_sn;
  list_t            segmentation_pdu_list;



  u32_t             status_requested;
  u32_t             last_frame_status_indication;




  //u32_t            *frame_tick_milliseconds;      // pointer on this tick variable handled by RRC : READ ONLY

  //-----------------------------
  // buffer occupancy measurements sent to MAC
  //-----------------------------
  // note occupancy of other buffers is deducted from nb elements in lists
  u32_t             buffer_occupancy_retransmission_buffer;       // nb of pdus

  //**************************************************************
  // new members
  //**************************************************************
  u8_t              allocation;
  u8_t              location;     // UTRAN/UE

} rlc_am_entity_t;
#    endif
