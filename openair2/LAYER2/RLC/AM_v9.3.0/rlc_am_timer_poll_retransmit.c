#define RLC_AM_MODULE
#define RLC_AM_TIMER_POLL_RETRANSMIT_C
//-----------------------------------------------------------------------------
#include "rtos_header.h"
#include "platform_types.h"
#include "platform_constants.h"
//-----------------------------------------------------------------------------
#include "rlc_am.h"
#include "LAYER2/MAC/extern.h"
//-----------------------------------------------------------------------------
void rlc_am_check_timer_poll_retransmit(rlc_am_entity_t *rlcP)
//-----------------------------------------------------------------------------
{
  // 5.2.2.3 Expiry of t-PollRetransmit
  // Upon expiry of t-PollRetransmit, the transmitting side of an AM RLC entity shall:
  //     - if both the transmission buffer and the retransmission buffer are empty (excluding transmitted RLC data PDU
  //           awaiting for acknowledgements); or
  //     - if no new RLC data PDU can be transmitted (e.g. due to window stalling):
  //         - consider the AMD PDU with SN = VT(S) – 1 for retransmission; or
  //         - consider any AMD PDU which has not been positively acknowledged for retransmission;
  //     - include a poll in a RLC data PDU as described in section 5.2.2.1.

    if (rlcP->t_poll_retransmit.running) {
        if (rlcP->t_poll_retransmit.frame_time_out == mac_xface->frame) {
            rlcP->t_poll_retransmit.running   = 0;
            rlcP->t_poll_retransmit.timed_out = 1;
            msg("[FRAME %05d][RLC_AM][MOD %02d][RB %02d][T_POLL_RETRANSMIT] TIME-OUT\n", mac_xface->frame,
                        rlcP->module_id, rlcP->rb_id);


#warning         TO DO rlc_am_check_timer_poll_retransmit
            rlcP->t_poll_retransmit.frame_time_out = mac_xface->frame + rlcP->t_poll_retransmit.time_out;
        }
    }
}
//-----------------------------------------------------------------------------
int rlc_am_is_timer_poll_retransmit_timed_out(rlc_am_entity_t *rlcP)
//-----------------------------------------------------------------------------
{
    return rlcP->t_poll_retransmit.timed_out;
}
//-----------------------------------------------------------------------------
void rlc_am_stop_and_reset_timer_poll_retransmit(rlc_am_entity_t *rlcP)
//-----------------------------------------------------------------------------
{
   msg("[FRAME %05d][RLC_AM][MOD %02d][RB %02d][T_POLL_RETRANSMIT] STOPPED AND RESET\n", mac_xface->frame,
                        rlcP->module_id, rlcP->rb_id);
    rlcP->t_poll_retransmit.running         = 0;
    rlcP->t_poll_retransmit.frame_time_out  = 0;
    rlcP->t_poll_retransmit.timed_out       = 0;
}
//-----------------------------------------------------------------------------
void rlc_am_start_timer_poll_retransmit(rlc_am_entity_t *rlcP)
//-----------------------------------------------------------------------------
{
    rlcP->t_poll_retransmit.running         = 1;
    rlcP->t_poll_retransmit.frame_time_out  = mac_xface->frame + rlcP->t_poll_retransmit.time_out;
    rlcP->t_poll_retransmit.timed_out       = 0;
    msg("[FRAME %05d][RLC_AM][MOD %02d][RB %02d][T_POLL_RETRANSMIT] STARTED (TIME-OUT = FRAME %05d)\n", mac_xface->frame, rlcP->module_id, rlcP->rb_id, rlcP->t_poll_retransmit.frame_time_out);
}
//-----------------------------------------------------------------------------
void rlc_am_init_timer_poll_retransmit(rlc_am_entity_t *rlcP, u32_t time_outP)
//-----------------------------------------------------------------------------
{
    rlcP->t_poll_retransmit.running         = 0;
    rlcP->t_poll_retransmit.frame_time_out  = 0;
    rlcP->t_poll_retransmit.time_out        = time_outP;
    rlcP->t_poll_retransmit.timed_out       = 0;
}
