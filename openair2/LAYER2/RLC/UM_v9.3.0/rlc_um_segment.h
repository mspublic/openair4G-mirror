#    ifndef __RLC_UM_SEGMENT_PROTO_EXTERN_H__
#        define __RLC_UM_SEGMENT_PROTO_EXTERN_H__
//-----------------------------------------------------------------------------
#        include "rlc_um_entity.h"
#        include "rlc_um_structs.h"
#        include "rlc_um_constants.h"
#        include "list.h"
//-----------------------------------------------------------------------------
#        ifdef RLC_UM_SEGMENT_C
#            define private_rlc_um_segment(x)    x
#            define protected_rlc_um_segment(x)  x
#            define public_rlc_um_segment(x)     x
#        else
#            ifdef RLC_UM_MODULE
#                define private_rlc_um_segment(x)
#                define protected_rlc_um_segment(x)  extern x
#                define public_rlc_um_segment(x)     extern x
#            else
#                define private_rlc_um_segment(x)
#                define protected_rlc_um_segment(x)
#                define public_rlc_um_segment(x)     extern x
#            endif
#        endif
#
protected_rlc_um_segment(void rlc_um_segment_10 (struct rlc_um_entity *rlcP));
#    endif
