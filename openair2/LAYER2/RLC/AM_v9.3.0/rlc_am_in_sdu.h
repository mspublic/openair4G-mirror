#    ifndef __RLC_AM_IN_SDU_PROTO_EXTERN_H__
#        define __RLC_AM_IN_SDU_PROTO_EXTERN_H__
//-----------------------------------------------------------------------------
#        ifdef RLC_AM_IN_SDU_C
#            define private_rlc_am_in_sdu(x)    x
#            define protected_rlc_am_in_sdu(x)  x
#            define public_rlc_am_in_sdu(x)     x
#        else
#            ifdef RLC_AM_MODULE
#                define private_rlc_am_in_sdu(x)
#                define protected_rlc_am_in_sdu(x)  extern x
#                define public_rlc_am_in_sdu(x)     extern x
#            else
#                define private_rlc_am_in_sdu(x)
#                define protected_rlc_am_in_sdu(x)
#                define public_rlc_am_in_sdu(x)     extern x
#            endif
#        endif
protected_rlc_am_in_sdu(void rlc_am_free_in_sdu      (rlc_am_entity_t *rlcP, unsigned int index_in_bufferP);)
protected_rlc_am_in_sdu(void rlc_am_free_in_sdu_data (rlc_am_entity_t *rlcP, unsigned int index_in_bufferP);)
protected_rlc_am_in_sdu(signed int rlc_am_in_sdu_is_empty(rlc_am_entity_t *rlcP);)

#    endif
