#define RLC_AM_MODULE
#define RLC_AM_TIMER_POLL_REORDERING_C
//-----------------------------------------------------------------------------
#include "rtos_header.h"
#include "platform_types.h"
#include "platform_constants.h"
//-----------------------------------------------------------------------------
#include "rlc_am.h"
# include "LAYER2/MAC/extern.h"
//-----------------------------------------------------------------------------
void rlc_am_check_timer_reordering(rlc_am_entity_t *rlcP)
//-----------------------------------------------------------------------------
{
    return ; // for debug


    if (rlcP->t_reordering.running) {
        if (rlcP->t_reordering.frame_time_out == mac_xface->frame) {
            // 5.1.3.2.4 Actions when t-Reordering expires
            // When t-Reordering expires, the receiving side of an AM RLC entity shall:
            //     - update VR(MS) to the SN of the first AMD PDU with SN >= VR(X) for which not all byte segments have been
            //       received;
            //     - if VR(H) > VR(MS):
            //         - start t-Reordering;
            //         - set VR(X) to VR(H).


            rlcP->t_reordering.running   = 0;
            rlcP->t_reordering.timed_out = 1;

            rlc_am_pdu_info_t* pdu_info;
            mem_block_t*       cursor;
            cursor    =  rlcP->receiver_buffer.head;

            if (cursor) {
                do {
                    pdu_info =  &((rlc_am_rx_pdu_management_t*)(cursor->data))->pdu_info;

                    // NOT VERY SURE ABOUT THAT, THINK ABOUT IT
                    rlcP->vr_ms = (pdu_info->sn + 1) & RLC_AM_SN_MASK;

                    if (rlc_am_sn_gte_vr_x(rlcP, pdu_info->sn)) {
                        if (((rlc_am_rx_pdu_management_t*)(cursor->data))->all_segments_received == 0) {
                            rlcP->vr_ms = pdu_info->sn;
                            break;
                        }
                    }
                    cursor = cursor->next;
                } while (cursor != NULL);
                msg("[FRAME %05d][RLC_AM][MOD %02d][RB %02d][T-REORDERING] TIME-OUT UPDATED VR(MS) %04d\n", mac_xface->frame, rlcP->module_id, rlcP->rb_id, rlcP->vr_ms);
            }

            if (rlc_am_sn_gt_vr_ms(rlcP, rlcP->vr_h)) {
                rlcP->vr_x = rlcP->vr_h;
                rlcP->t_reordering.frame_time_out = mac_xface->frame + rlcP->t_reordering.time_out;
                msg("[FRAME %05d][RLC_AM][MOD %02d][RB %02d][T-REORDERING] TIME-OUT, RESTARTED T-REORDERING, UPDATED VR(X) to VR(R) %04d\n", mac_xface->frame, rlcP->module_id, rlcP->rb_id, rlcP->vr_x);
            }

            rlcP->status_requested = 1;
        }
    }
}
//-----------------------------------------------------------------------------
void rlc_am_stop_and_reset_timer_reordering(rlc_am_entity_t *rlcP)
//-----------------------------------------------------------------------------
{
    msg("[FRAME %05d][RLC_AM][MOD %02d][RB %02d][T-REORDERING] STOPPED AND RESET\n", mac_xface->frame,
                        rlcP->module_id, rlcP->rb_id);
    rlcP->t_reordering.running         = 0;
    rlcP->t_reordering.frame_time_out  = 0;
    rlcP->t_reordering.timed_out       = 0;
}
//-----------------------------------------------------------------------------
void rlc_am_start_timer_reordering(rlc_am_entity_t *rlcP)
//-----------------------------------------------------------------------------
{
    rlcP->t_reordering.running         = 1;
    rlcP->t_reordering.frame_time_out  = mac_xface->frame + rlcP->t_reordering.time_out;
    rlcP->t_reordering.timed_out       = 0;
    msg("[FRAME %05d][RLC_AM][MOD %02d][RB %02d][T-REORDERING] STARTED (TIME-OUT = FRAME %05d)\n",
            mac_xface->frame, rlcP->module_id, rlcP->rb_id, rlcP->t_reordering.frame_time_out);
}
//-----------------------------------------------------------------------------
void rlc_am_init_timer_reordering(rlc_am_entity_t *rlcP, u32_t time_outP)
//-----------------------------------------------------------------------------
{
    rlcP->t_reordering.running         = 0;
    rlcP->t_reordering.frame_time_out  = 0;
    rlcP->t_reordering.time_out        = time_outP;
    rlcP->t_reordering.timed_out       = 0;
}
