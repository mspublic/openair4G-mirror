#ifndef __RLC_AM_TIMER_REORDERING_H__
#    define __RLC_AM_TIMER_REORDERING_H__
#    ifdef RLC_AM_TIMER_POLL_REORDERING_C
#        define private_rlc_am_timer_reordering(x)    x
#        define protected_rlc_am_timer_reordering(x)  x
#        define public_rlc_am_timer_reordering(x)     x
#    else
#        ifdef RLC_AM_MODULE
#            define private_rlc_am_timer_reordering(x)
#            define protected_rlc_am_timer_reordering(x)  extern x
#            define public_rlc_am_timer_reordering(x)     extern x
#        else
#            define private_rlc_am_timer_reordering(x)
#            define protected_rlc_am_timer_reordering(x)
#            define public_rlc_am_timer_reordering(x)     extern x
#        endif
#    endif

protected_rlc_am_timer_reordering(void rlc_am_check_timer_reordering(rlc_am_entity_t *rlcP));
protected_rlc_am_timer_reordering(void rlc_am_stop_and_reset_timer_reordering(rlc_am_entity_t *rlcP);)
protected_rlc_am_timer_reordering(void rlc_am_start_timer_reordering(rlc_am_entity_t *rlcP);)
protected_rlc_am_timer_reordering(void rlc_am_init_timer_reordering(rlc_am_entity_t *rlcP, u32_t time_outP);)

#endif
