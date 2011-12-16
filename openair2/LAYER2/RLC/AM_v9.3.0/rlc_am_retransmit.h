#    ifndef __RLC_AM_RETRANSMIT_H__
#        define __RLC_AM_RETRANSMIT_H__
//-----------------------------------------------------------------------------
#        ifdef RLC_AM_RETRANSMIT_C
#            define private_rlc_am_retransmit(x)    x
#            define protected_rlc_am_retransmit(x)  x
#            define public_rlc_am_retransmit(x)     x
#        else
#            ifdef RLC_AM_MODULE
#                define private_rlc_am_retransmit(x)
#                define protected_rlc_am_retransmit(x)  extern x
#                define public_rlc_am_retransmit(x)     extern x
#            else
#                define private_rlc_am_retransmit(x)
#                define protected_rlc_am_retransmit(x)
#                define public_rlc_am_retransmit(x)     extern x
#            endif
#        endif
protected_rlc_am_retransmit(void         rlc_am_nack_pdu (rlc_am_entity_t *rlcP, u16_t snP, u16_t so_startP, u16_t so_endP);)
protected_rlc_am_retransmit(void         rlc_am_ack_pdu (rlc_am_entity_t *rlcP, u16_t snP);)
protected_rlc_am_retransmit(mem_block_t* rlc_am_retransmit_get_copy (rlc_am_entity_t *rlcP, u16_t snP));
protected_rlc_am_retransmit(mem_block_t* rlc_am_retransmit_get_subsegment (rlc_am_entity_t *rlcP, u16_t snP, u16_t *sizeP));
protected_rlc_am_retransmit(void rlc_am_retransmit_any_pdu(rlc_am_entity_t* rlcP);)
protected_rlc_am_retransmit(void rlc_am_tx_buffer_display (rlc_am_entity_t* rlcP, char* messageP);)

#    endif
