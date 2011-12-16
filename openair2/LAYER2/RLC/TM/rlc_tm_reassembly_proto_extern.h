/***************************************************************************
                          rlc_tm_reassembly_proto_extern.h  -
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr
 ***************************************************************************/
#    ifndef __RLC_TM_REASSEMBLY_PROTO_EXTERN_H__
#        define __RLC_TM_REASSEMBLY_PROTO_EXTERN_H__
//-----------------------------------------------------------------------------
#        include "rlc_tm_entity.h"
//-----------------------------------------------------------------------------
extern void     rlc_tm_send_sdu_no_segment (struct rlc_tm_entity *rlcP, u8_t error_indicationP, u8 * srcP, u16_t length_in_bitsP);
extern void     rlc_tm_send_sdu_segment (struct rlc_tm_entity *rlcP, u8_t error_indicationP);
#    endif
