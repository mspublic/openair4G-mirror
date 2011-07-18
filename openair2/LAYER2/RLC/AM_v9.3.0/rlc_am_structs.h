/***************************************************************************
                          rlc_am_structs.h  -
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr
 ***************************************************************************/
#    ifndef __RLC_AM_STRUCTS_H__
#        define __RLC_AM_STRUCTS_H__

#        include "platform_types.h"
#        include "platform_constants.h"
#        include "list.h"
#        include "mem_block.h"
#        include "rlc_am_constants.h"
//#ifdef USER_MODE
#        include "mac_rlc_primitives.h"
//#endif //USER_MODE
#        include "mac_primitives.h"
#        include "rlc_primitives.h"
//-----------------------------------------------------------------------------
// SDU MANAGEMENT
//-----------------------------------------------------------------------------
typedef struct sdu_management_flags {
  u8_t discarded:1;
  u8_t segmented:1;
  u8_t segmentation_in_progress:1;
  u8_t no_new_sdu_segmented_in_last_pdu:1;
  u8_t transmitted_successfully:1;
  u8_t dummy:3;
} sdu_management_flags_t;

typedef struct pdu_management_flags {
  u8_t ack:1;              // used when pdu is in retransmission buffer;
  u8_t retransmit:1;
  u8_t dummy:6;
} pdu_management_flags_t;

typedef struct rlc_am_tx_sdu_management {
  mem_block_t            *mem_block;
  u8_t                   *first_byte;   // payload
  s32_t                   sdu_creation_time;
  u32_t                   mui;          // mui may be used only by radio access bearers
  u16_t                   sdu_remaining_size;
  u16_t                   sdu_segmented_size;
  u16_t                   sdu_size;
  s16_t                   pdus_index[RLC_AM_MAX_SDU_FRAGMENTS];       // sn of pdus (is an array)
  u16_t                   last_pdu_sn;
  u8_t                    nb_pdus;      // number of pdus were this sdu was segmented
  u8_t                    nb_pdus_internal_use; // count the number of pdus transmitted to lower layers (used in mux procedure)
  u8_t                    nb_pdus_ack;  // counter used for confirm and discard MaxDAT
  u8_t                    nb_pdus_time; // counter used for timer based discard

  s8_t                    li_index_for_discard; // indicates the li index in the last pdu of the sdu, marking the end of the sdu
  sdu_management_flags_t  flags;
} rlc_am_tx_sdu_management_t;



typedef struct rlc_am_tx_data_pdu_management {
  mem_block_t     *mem_block;         //pointer on pdu queued in retransmission_buffer_to_send in order
  // not to insert the same pdu to retransmit twice, also to avoid the transmission of the pdu if acknowledged
  // but previously was queued for retransmission in retransmission_buffer_to_send but not sent because of
  // limited number of pdu delivered by TTI.
  u8_t             *first_byte;   // pointer on the pdu including header, LIs;
  u8_t             *payload;      // pointer on the pdu payload
  s16_t             sdus_index[RLC_AM_MAX_SDU_IN_PDU]; // index of sdu having segments in this pdu (index in rlc_am_entity.input_sdus[])
  u32_t             last_nack_time;
  u16_t             hole_so_start  [RLC_AM_MAX_HOLES_REPORT_PER_PDU];
  u16_t             hole_so_stop   [RLC_AM_MAX_HOLES_REPORT_PER_PDU];
  u8_t              num_holes;

  s16_t             header_and_payload_size;
  s16_t             payload_size;
  s16_t             sn;
  s16_t             nack_so_start; // must be set to 0 if global nack
  s16_t             nack_so_stop;  // must be set to data_size if global nack
  s16_t             last_segment_offset;


  s8_t              nb_sdus;       // number of sdu having segments in this pdu
  s8_t              retx_count;
  // This counter counts the number of retransmissions of an AMD PDU (see subclause 5.2.1). There is one RETX_COUNT
  // counter per PDU that needs to be retransmitted.
  // there is one VT(DAT) for each PDU and it is incremented each time the PDU is transmitted;

  pdu_management_flags_t          flags;


} rlc_am_tx_data_pdu_management_t;


typedef struct rlc_am_tx_control_pdu_management {
  mem_block_t      *mem_block;         //pointer on pdu queued in retransmission_buffer_to_send in order
  u8_t             *first_byte;   // pointer on the pdu including header, LIs;
  u16_t             size;
} rlc_am_tx_control_pdu_management_t;


typedef struct rlc_am_pdu_sn_10 {
  u8_t  b1;
  u8_t  b2;
  u8_t  data[2];
}__attribute__((__packed__)) rlc_am_pdu_sn_10_t ;

typedef struct rlc_am_e_li {
  u8_t  b1;
  u8_t  b2;
  u8_t  b3;
}rlc_am_e_li_t;

typedef struct rlc_am_pdu_info {
  u32_t  d_c:1;
  u32_t  rf:1;
  u32_t  p:1;
  u32_t  fi:2;
  u32_t  e:1;
  u32_t  sn:10;
  u32_t  lsf:1;
  u32_t  so:15;
  u16_t  num_li;
  s16_t  li_list[RLC_AM_MAX_SDU_IN_PDU];
  s16_t  hidden_size;
  u8_t*  payload;
  s16_t  payload_size;
  s16_t  header_size;
} rlc_am_pdu_info_t ;

typedef struct nack_sn {
  u16_t nack_sn:10;
  u16_t e1:1;
  u16_t e2:1;
  u32_t so_start:15;
  u32_t so_end:15;

} nack_sn_t;

typedef struct rlc_am_control_pdu_info {
  u16_t      d_c:1;
  u16_t      cpt:3;
  u16_t      ack_sn:10;
  u16_t      e1:1;
  u16_t      dummy:2;
  u16_t      num_nack;
  nack_sn_t  nack_list[RLC_AM_MAX_NACK_IN_STATUS_PDU];
} rlc_am_control_pdu_info_t ;

typedef struct rlc_am_timer {
  u32_t  frame_time_out;
  u32_t  time_out;
  u32_t  running:1;
  u32_t  timed_out:1;
  u32_t  dummy:30;
} rlc_am_timer_t ;

//-----------------------------------------------------------------------------
// DATA PDU
//-----------------------------------------------------------------------------
typedef struct rlc_am_rx_pdu_management {
  rlc_am_pdu_info_t pdu_info;
  u8_t              all_segments_received;
} rlc_am_rx_pdu_management_t;



//-----------------------------------------------------------------------------
// HEADERS
//-----------------------------------------------------------------------------
/*struct rlc_am_pdu_header {
  u8_t              byte1;
  u8_t              byte2;
  u8_t              li_data_7[1];
};

struct rlc_am_reset_header {
  u8_t              byte1;
  u8_t              hfni[3];      // is coded on 20 most significant bits of 24 bits
};

struct rlc_am_status_header {
  u8_t              byte1;
  u8_t              suffi[1];     // next suffi(s)
};*/
//-----------------------------------------------------------------------------
//  interlayers optimizations
//-----------------------------------------------------------------------------
struct rlc_am_tx_data_pdu_allocation {
  union {
    struct rlc_am_tx_data_pdu_management rlc_am_tx_pdu_mngmnt;
    struct mac_tb_req tb_req;
    struct mac_tx_tb_management tb_mngt;
#        ifdef BYPASS_L1
    struct rlc_am_rx_pdu_management dummy;
    struct mac_tb_ind dummy2;
    struct mac_rx_tb_management dummy3;
    struct rlc_indication dummy4;
#        endif
  } dummy;
};

struct rlc_am_tx_control_pdu_allocation {
  union {
    struct mac_tb_req tb_req;
    struct mac_tx_tb_management tb_mngt;
    struct rlc_am_tx_control_pdu_management rlc_am_tx_pdu_mngmnt;
#        ifdef BYPASS_L1
    struct mac_tb_ind dummy2;
    struct mac_rx_tb_management dummy3;
    struct rlc_indication dummy4;
#        endif
  } dummy;
};

struct rlc_am_data_req_alloc {  // alloc enought bytes for sdu mngt also
  union {
    struct rlc_am_data_req dummy1;
    struct rlc_am_tx_sdu_management dummy2;
  } dummy;
};

#    endif
