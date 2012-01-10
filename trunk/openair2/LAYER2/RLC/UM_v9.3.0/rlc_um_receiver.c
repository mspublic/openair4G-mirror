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
#define RLC_UM_MODULE
#define RLC_UM_RECEIVER_C
#include "platform_types.h"
//-----------------------------------------------------------------------------
#include "rlc.h"
#include "rlc_um.h"
#include "rlc_um_structs.h"
#include "rlc_primitives.h"
#include "mac_primitives.h"
#include "list.h"
#include "MAC_INTERFACE/extern.h"
#include "UTIL/LOG/log.h"

#define DEBUG_RLC_UM_DISPLAY_TB_DATA
//#define RLC_UM_GENERATE_ERRORS
//-----------------------------------------------------------------------------
void
rlc_um_receive (struct rlc_um_entity *rlcP, u32_t frame, struct mac_data_ind data_indP)
{
//-----------------------------------------------------------------------------

    mem_block_t        *tb;
    u8_t               *first_byte;
    u16_t               tb_size_in_bytes;

    while ((tb = list_remove_head (&data_indP.data))) {
#ifdef DEBUG_RLC_STATS
        rlcP->rx_pdus += 1;
#endif

#ifdef DEBUG_RLC_UM_DISPLAY_TB_DATA
        msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] DUMP RX PDU(%d bytes):", rlcP->module_id, rlcP->rb_id, frame, ((struct mac_tb_ind *) (tb->data))->size);
        for (tb_size_in_bytes = 0; tb_size_in_bytes < ((struct mac_tb_ind *) (tb->data))->size; tb_size_in_bytes++) {
            msg ("%02X.", ((struct mac_tb_ind *) (tb->data))->data_ptr[tb_size_in_bytes]);
        }
        msg ("\n");
#endif

#ifdef RLC_UM_GENERATE_ERRORS
            if (random() % 10 == 4) {
                ((struct mac_tb_ind *) (tb->data))->error_indication = 1;
                msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d]  RX PDU GENERATE ERROR", rlcP->module_id, rlcP->rb_id, frame);
            }
#endif

        if (!(((struct mac_tb_ind *) (tb->data))->error_indication)) {
            first_byte = ((struct mac_tb_ind *) (tb->data))->data_ptr;
            tb_size_in_bytes = ((struct mac_tb_ind *) (tb->data))->size;
            if (tb_size_in_bytes > 0) {
	      rlc_um_receive_process_dar (rlcP, frame, tb, (rlc_um_pdu_sn_10_t *)first_byte, tb_size_in_bytes);
	      msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] VR(UR)=%03d VR(UX)=%03d VR(UH)=%03d\n", rlcP->module_id, rlcP->rb_id, frame, rlcP->vr_ur, rlcP->vr_ux, rlcP->vr_uh);
            }
        } else {
            rlcP->rx_pdus_in_error += 1;
#ifdef DEBUG_RLC_UM_RX
            msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] RX PDU WITH ERROR INDICATED BY LOWER LAYERS -> GARBAGE\n", rlcP->module_id, rlcP->rb_id, frame);
#endif
        }
        free_mem_block (tb);
    }                           // end while
}
