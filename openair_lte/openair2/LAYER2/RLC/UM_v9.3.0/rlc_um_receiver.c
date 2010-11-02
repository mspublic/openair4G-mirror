#define RLC_UM_MODULE
#define RLC_UM_RECEIVER_C
#include "rtos_header.h"
#include "platform_types.h"
//-----------------------------------------------------------------------------
#include "rlc.h"
#include "rlc_um.h"
#include "rlc_um_structs.h"
#include "rlc_primitives.h"
#include "mac_primitives.h"
#include "list.h"
#define DEBUG_RLC_UM_DISPLAY_TB_DATA
//-----------------------------------------------------------------------------
void
rlc_um_receive (struct rlc_um_entity *rlcP, struct mac_data_ind data_indP)
{
//-----------------------------------------------------------------------------

    mem_block_t        *tb;
    u8_t               *first_byte;
    u8_t                tb_size_in_bytes;

    while ((tb = list_remove_head (&data_indP.data))) {
#ifdef DEBUG_RLC_STATS
        rlcP->rx_pdus += 1;
#endif

#ifdef DEBUG_RLC_UM_DISPLAY_TB_DATA
        msg ("[RLC_UM][MOD %d][RB %d] DUMP RX PDU:", rlcP->module_id, rlcP->rb_id);
        for (tb_size_in_bytes = 0; tb_size_in_bytes < data_indP.tb_size; tb_size_in_bytes++) {
            msg ("%02X.", ((struct mac_tb_ind *) (tb->data))->data_ptr[tb_size_in_bytes]);
        }
        msg ("\n");
#endif
        if (!(((struct mac_tb_ind *) (tb->data))->error_indication)) {
            first_byte = ((struct mac_tb_ind *) (tb->data))->data_ptr;
            tb_size_in_bytes = ((struct mac_tb_ind *) (tb->data))->size;

            rlc_um_receive_process_dar (rlcP, tb, first_byte, tb_size_in_bytes);
            msg ("[RLC_UM][MOD %d][RB %d] VR(UR)=%03d VR(UX)=%03d VR(UH)=%03d\n", rlcP->module_id, rlcP->rb_id, rlcP->vr_ur, rlcP->vr_ux, rlcP->vr_uh);
        } else {
#ifdef DEBUG_RLC_STATS
            rlcP->rx_pdus_in_error += 1;
#endif
#ifdef DEBUG_RLC_UM_RX
            msg ("[RLC_UM][MOD %d][RB %d] RX PDU  WITH ERROR INDICATED BY LOWER LAYERS -> GARBAGE\n", rlcP->module_id, rlcP->rb_id);
#endif
        }
        free_mem_block (tb);
    }                           // end while
}
