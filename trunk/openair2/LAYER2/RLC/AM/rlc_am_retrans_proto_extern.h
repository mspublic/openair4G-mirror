/***************************************************************************
                          rlc_am_retrans_proto_extern.h  -
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr

 ***************************************************************************/
#    ifndef __RLC_AM_RETRANS_PROTO_EXTERN_H__
#        define __RLC_AM_RETRANS_PROTO_EXTERN_H__
//-----------------------------------------------------------------------------
#        include "rlc_am_entity.h"
#        include "rlc_am_structs.h"
#        include "rlc_am_constants.h"
//-----------------------------------------------------------------------------
extern int16_t      retransmission_buffer_management_ack (struct rlc_am_entity *rlcP, uint8_t * sufiP, uint8_t byte_alignedP, int16_t * first_error_indicated_snP);
extern uint8_t      *retransmission_buffer_management_bitmap (struct rlc_am_entity *rlcP, uint8_t * bitmap_sufiP, uint8_t byte_alignedP, int16_t * first_error_indicated_snP);
extern uint8_t      *retransmission_buffer_management_list (struct rlc_am_entity *rlcP, uint8_t * bitmap_sufiP, uint8_t byte_alignedP, int16_t * first_error_indicated_snP);
extern void     free_retransmission_buffer (struct rlc_am_entity *rlcP, uint16_t indexP);
extern void     free_retransmission_buffer_no_confirmation (struct rlc_am_entity *rlcP, uint16_t indexP);
extern void     insert_into_retransmission_buffer (struct rlc_am_entity *rlcP, uint16_t indexP, mem_block_t * pduP);
extern uint8_t       retransmit_pdu (struct rlc_am_entity *rlcP, uint16_t snP);
extern void     rlc_am_get_not_acknowledged_pdu (struct rlc_am_entity *rlcP);
extern void     rlc_am_get_not_acknowledged_pdu_vt_s_minus_1 (struct rlc_am_entity *rlcP);
extern void     rlc_am_get_not_acknowledged_pdu_optimized (struct rlc_am_entity *rlcP);
#    endif
