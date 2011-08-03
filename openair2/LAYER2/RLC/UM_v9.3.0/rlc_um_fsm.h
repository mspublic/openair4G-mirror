#    ifndef __RLC_UM_FSM_PROTO_EXTERN_H__
#        define __RLC_UM_FSM_PROTO_EXTERN_H__
//-----------------------------------------------------------------------------
#        ifdef RLC_UM_FSM_C
#            define private_rlc_um_fsm(x)    x
#            define protected_rlc_um_fsm(x)  x
#            define public_rlc_um_fsm(x)     x
#        else
#            ifdef RLC_UM_MODULE
#                define private_rlc_um_fsm(x)
#                define protected_rlc_um_fsm(x)  extern x
#                define public_rlc_um_fsm(x)     extern x
#            else
#                define private_rlc_um_fsm(x)
#                define protected_rlc_um_fsm(x)
#                define public_rlc_um_fsm(x)     extern x
#            endif
#        endif
#        include "platform_types.h"
#        include "rlc_um_entity.h"
//-----------------------------------------------------------------------------
protected_rlc_um_fsm(int      rlc_um_fsm_notify_event (struct rlc_um_entity *rlcP, u8_t eventP));
#    endif
