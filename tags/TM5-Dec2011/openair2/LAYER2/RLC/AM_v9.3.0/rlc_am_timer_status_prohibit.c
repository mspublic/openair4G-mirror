/*******************************************************************************

Eurecom OpenAirInterface 2
Copyright(c) 1999 - 2010 Eurecom

This program is free software; you can redistribute it and/or modify it
under the terms and conditions of the GNU General Public License,
version 2, as published by the Free Software Foundation.

This program is distributed in the hope it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

The full GNU General Public License is included in this distribution in
the file called "COPYING".

Contact Information
Openair Admin: openair_admin@eurecom.fr
Openair Tech : openair_tech@eurecom.fr
Forums       : http://forums.eurecom.fsr/openairinterface
Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/
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