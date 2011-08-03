#    ifndef __RLC_AM_SEGMENT_PROTO_EXTERN_H__
#        define __RLC_AM_SEGMENT_PROTO_EXTERN_H__
//-----------------------------------------------------------------------------
#        ifdef RLC_AM_SEGMENT_C
#            define private_rlc_am_segment(x)    x
#            define protected_rlc_am_segment(x)  x
#            define public_rlc_am_segment(x)     x
#        else
#            ifdef RLC_AM_MODULE
#                define private_rlc_am_segment(x)
#                define protected_rlc_am_segment(x)  extern x
#                define public_rlc_am_segment(x)     extern x
#            else
#                define private_rlc_am_segment(x)
#                define protected_rlc_am_segment(x)
#                define public_rlc_am_segment(x)     extern x
#            endif
#        endif

protected_rlc_am_segment(void rlc_am_pdu_polling (rlc_am_entity_t *rlcP, rlc_am_pdu_sn_10_t *pduP, s16_t payload_sizeP);)
protected_rlc_am_segment(void rlc_am_segment_10 (rlc_am_entity_t *rlcP);)
#    endif
