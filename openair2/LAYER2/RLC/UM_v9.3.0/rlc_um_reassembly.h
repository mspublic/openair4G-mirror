#    ifndef __RLC_UM_REASSEMBLY_PROTO_EXTERN_H__
#        define __RLC_UM_REASSEMBLY_PROTO_EXTERN_H__
//-----------------------------------------------------------------------------
#        ifdef RLC_UM_REASSEMBLY_C
#            define private_rlc_um_reassembly(x)    x
#            define protected_rlc_um_reassembly(x)  x
#            define public_rlc_um_reassembly(x)     x
#        else
#            ifdef RLC_UM_MODULE
#                define private_rlc_um_reassembly(x)
#                define protected_rlc_um_reassembly(x)  extern x
#                define public_rlc_um_reassembly(x)     extern x
#            else
#                define private_rlc_um_reassembly(x)
#                define protected_rlc_um_reassembly(x)
#                define public_rlc_um_reassembly(x)     extern x
#            endif
#        endif
#        include "rlc_um_entity.h"
//-----------------------------------------------------------------------------
protected_rlc_um_reassembly(void     rlc_um_send_sdu_minus_1_byte (rlc_um_entity_t *rlcP));
protected_rlc_um_reassembly(void     rlc_um_clear_rx_sdu (rlc_um_entity_t *rlcP));
protected_rlc_um_reassembly(void     rlc_um_reassembly (u8_t * srcP, s32_t lengthP, rlc_um_entity_t *rlcP));
protected_rlc_um_reassembly(void     rlc_um_send_sdu (rlc_um_entity_t *rlcP));
#    endif
