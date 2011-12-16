#define RLC_AM_MODULE
#define RLC_AM_TIMER_STATUS_PROHIBIT_C
//-----------------------------------------------------------------------------
#include "rtos_header.h"
#include "platform_types.h"
#include "platform_constants.h"
//-----------------------------------------------------------------------------
#include "rlc_am.h"
# include "LAYER2/MAC/extern.h"
//-----------------------------------------------------------------------------
void rlc_am_check_timer_status_prohibit(rlc_am_entity_t *rlcP)
//-----------------------------------------------------------------------------
{
    if (rlcP->t_status_prohibit.running) {
        if (rlcP->t_status_prohibit.frame_time_out == mac_xface->frame) {
            rlcP->t_status_prohibit.running   = 0;
            rlcP->t_status_prohibit.timed_out = 1;

            msg("[FRAME %05d][RLC_AM][MOD %02d][RB %02d][T-STATUS-PROHIBIT] TIME-OUT\n", mac_xface->frame,
                        rlcP->module_id, rlcP->rb_id);
#warning         TO DO rlc_am_check_timer_status_prohibit
            rlcP->t_status_prohibit.frame_time_out = mac_xface->frame + rlcP->t_status_prohibit.time_out;
        }
    }
}
//-----------------------------------------------------------------------------
void rlc_am_stop_and_reset_timer_status_prohibit(rlc_am_entity_t *rlcP)
//-----------------------------------------------------------------------------
{
    msg("[FRAME %05d][RLC_AM][MOD %02d][RB %02d][T-STATUS-PROHIBIT] STOPPED AND RESET\n", mac_xface->frame,
                        rlcP->module_id, rlcP->rb_id);
    rlcP->t_status_prohibit.running        = 0;
    rlcP->t_status_prohibit.frame_time_out = 0;
    rlcP->t_status_prohibit.timed_out      = 0;
}
//-----------------------------------------------------------------------------
void rlc_am_start_timer_status_prohibit(rlc_am_entity_t *rlcP)
//-----------------------------------------------------------------------------
{
    rlcP->t_status_prohibit.running        = 1;
    rlcP->t_status_prohibit.frame_time_out = rlcP->t_status_prohibit.time_out + mac_xface->frame;
    rlcP->t_status_prohibit.timed_out = 0;
    msg("[FRAME %05d][RLC_AM][MOD %02d][RB %02d][T-STATUS-PROHIBIT] STARTED (TIME-OUT = FRAME %05d)\n", mac_xface->frame, rlcP->module_id, rlcP->rb_id, rlcP->t_status_prohibit.frame_time_out);
    msg("TIME-OUT = FRAME %05d\n",  rlcP->t_status_prohibit.frame_time_out);
}
//-----------------------------------------------------------------------------
void rlc_am_init_timer_status_prohibit(rlc_am_entity_t *rlcP, u32_t time_outP)
//-----------------------------------------------------------------------------
{
    rlcP->t_status_prohibit.running        = 0;
    rlcP->t_status_prohibit.frame_time_out = 0;
    rlcP->t_status_prohibit.time_out       = time_outP;
    rlcP->t_status_prohibit.timed_out      = 0;
}
