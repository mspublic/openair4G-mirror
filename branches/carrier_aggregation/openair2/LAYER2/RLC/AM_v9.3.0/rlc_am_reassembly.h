#ifndef __RLC_AM_REASSEMBLY_H__
#    define __RLC_AM_REASSEMBLY_H__
#    ifdef RLC_AM_REASSEMBLY_C
#        define private_rlc_am_reassembly(x)    x
#        define protected_rlc_am_reassembly(x)  x
#        define public_rlc_am_reassembly(x)     x
#    else
#        ifdef RLC_AM_MODULE
#            define private_rlc_am_reassembly(x)
#            define protected_rlc_am_reassembly(x)  extern x
#            define public_rlc_am_reassembly(x)     extern x
#        else
#            define private_rlc_am_reassembly(x)
#            define protected_rlc_am_reassembly(x)
#            define public_rlc_am_reassembly(x)     extern x
#        endif
#    endif
private_rlc_am_reassembly(   void rlc_am_clear_rx_sdu (rlc_am_entity_t *rlcP);)
private_rlc_am_reassembly(   void rlc_am_reassembly   (u8_t * srcP, s32_t lengthP, rlc_am_entity_t *rlcP);)
private_rlc_am_reassembly(   void rlc_am_send_sdu     (rlc_am_entity_t *rlcP);)
protected_rlc_am_reassembly( void rlc_am_reassemble_pdu(rlc_am_entity_t* rlcP, mem_block_t* tbP);)
#endif

