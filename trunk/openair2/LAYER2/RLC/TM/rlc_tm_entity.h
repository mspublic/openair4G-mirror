/***************************************************************************
                          rlc_tm_entity.h  -
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr


 ***************************************************************************/
#    ifndef __RLC_TM_ENTITY_H__
#        define __RLC_TM_ENTITY_H__
//-----------------------------------------------------------------------------
#        include "platform_types.h"
#        include "platform_constants.h"
#        include "rlc_tm_structs.h"
#        include "rlc_def.h"
//-----------------------------------------------------------------------------
struct rlc_tm_entity {

  module_id_t     module_id;
  u8_t              allocation;
  u8_t              protocol_state;
  // for stats and trace purpose :
  u16_t             data_plane;   // act as a boolean
  u16_t              rb_id;
  //-----------------------------
  // discard info
  //-----------------------------
  u8_t              sdu_discard_mode;
  //-----------------------------
  // time
  //-----------------------------
  u16_t             timer_discard_init;
  u32_t            *frame_tick_milliseconds;      // pointer on this tick variable handled by RRC : READ ONLY
  s32_t             last_tti;
  //-----------------------------
  // tranmission
  //-----------------------------
  // sdu communication;
  mem_block_t   **input_sdus;   // should be accessed as an array
  mem_block_t    *input_sdus_alloc;     // allocation of the array
  u16_t             size_input_sdus_buffer;
  u16_t             nb_sdu;
  void            (*segmentation) (struct rlc_tm_entity * rlcP);

  u16_t             next_sdu_index;       // next location of incoming sdu
  u16_t             current_sdu_index;

  list_t          pdus_to_mac_layer;

  u16_t             rlc_pdu_size;
  u16_t             nb_pdu_requested_by_mac;
  u8_t              segmentation_indication;
  u8_t              delivery_of_erroneous_sdu;
  u32_t             buffer_occupancy;
  //-----------------------------
  // receiver
  //-----------------------------
  unsigned int    output_sdu_size_to_write;     // for writing in sdu
  mem_block_t    *output_sdu_in_construction;
  void            (*rx) (void *argP, struct mac_data_ind data_indP);
  u8_t              last_bit_position_reassemblied;


  list_t          pdus_from_mac_layer;

  //-----------------------------
  // C-SAP
  //-----------------------------
  list_t          c_sap;
  //-----------------------------
  // Mapping info
  //-----------------------------
  u8_t              logical_channel_identity;
};
#    endif
