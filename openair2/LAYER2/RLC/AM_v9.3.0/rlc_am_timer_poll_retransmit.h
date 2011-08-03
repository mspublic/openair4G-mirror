#ifndef __RLC_AM_TIMER_POLL_RETRANSMIT_H__
#    define __RLC_AM_TIMER_POLL_RETRANSMIT_H__
#    ifdef RLC_AM_TIMER_POLL_RETRANSMIT_C
#        define private_rlc_am_timer_poll_retransmit(x)    x
#        define protected_rlc_am_timer_poll_retransmit(x)  x
#        define public_rlc_am_timer_poll_retransmit(x)     x
#    else
#        ifdef RLC_AM_MODULE
#            define private_rlc_am_timer_poll_retransmit(x)
#            define protected_rlc_am_timer_poll_retransmit(x)  extern x
#            define public_rlc_am_timer_poll_retransmit(x)     extern x
#        else
#            define private_rlc_am_timer_poll_retransmit(x)
#            define protected_rlc_am_timer_poll_retransmit(x)
#            define public_rlc_am_timer_poll_retransmit(x)     extern x
#        endif
#    endif

protected_rlc_am_timer_poll_retransmit(void rlc_am_check_timer_poll_retransmit(rlc_am_entity_t *rlcP));
protected_rlc_am_timer_poll_retransmit(int  rlc_am_is_timer_poll_retransmit_timed_out(rlc_am_entity_t *rlcP);)
protected_rlc_am_timer_poll_retransmit(void rlc_am_stop_and_reset_timer_poll_retransmit(rlc_am_entity_t *rlcP);)
protected_rlc_am_timer_poll_retransmit(void rlc_am_start_timer_poll_retransmit(rlc_am_entity_t *rlcP);)
protected_rlc_am_timer_poll_retransmit(void rlc_am_init_timer_poll_retransmit(rlc_am_entity_t *rlcP, u32_t time_outP);)

#endif
