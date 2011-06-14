#ifndef __RLC_AM_TIMER_STATUS_PROHIBIT_H__
#    define __RLC_AM_TIMER_STATUS_PROHIBIT_H__
#    ifdef RLC_AM_TIMER_STATUS_PROHIBIT_C
#        define private_rlc_am_timer_status_prohibit(x)    x
#        define protected_rlc_am_timer_status_prohibit(x)  x
#        define public_rlc_am_timer_status_prohibit(x)     x
#    else
#        ifdef RLC_AM_MODULE
#            define private_rlc_am_timer_status_prohibit(x)
#            define protected_rlc_am_timer_status_prohibit(x)  extern x
#            define public_rlc_am_timer_status_prohibit(x)     extern x
#        else
#            define private_rlc_am_timer_status_prohibit(x)
#            define protected_rlc_am_timer_status_prohibit(x)
#            define public_rlc_am_timer_status_prohibit(x)     extern x
#        endif
#    endif

protected_rlc_am_timer_status_prohibit(void rlc_am_check_timer_poll_retransmit(rlc_am_entity_t *rlcP));
protected_rlc_am_timer_status_prohibit(void rlc_am_stop_and_reset_timer_poll_retransmit(rlc_am_entity_t *rlcP);)
protected_rlc_am_timer_status_prohibit(void rlc_am_start_timer_status_prohibit(rlc_am_entity_t *rlcP);)
protected_rlc_am_timer_status_prohibit(void rlc_am_init_timer_status_prohibit(rlc_am_entity_t *rlcP, u32_t time_outP);)
protected_rlc_am_timer_status_prohibit(void rlc_am_check_timer_status_prohibit(rlc_am_entity_t *rlcP);)
#endif
