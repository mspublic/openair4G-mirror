#define RLC_UM_MODULE
#define RLC_UM_DAR_C
#include "rtos_header.h"
#include "platform_types.h"
//-----------------------------------------------------------------------------
#include "rlc.h"
#include "rlc_um_entity.h"
#include "rlc_um_constants.h"
#include "rlc_um_structs.h"
#include "rlc_primitives.h"
#include "rlc_def.h"
#include "rlc_def_lte.h"
#include "mac_primitives.h"
#include "list.h"
#include "rlc_um_reassembly_proto_extern.h"
#include "rlc_um_dar_proto_extern.h"

#define DEBUG_RLC_UM_RX_DECODE
//-----------------------------------------------------------------------------
int rlc_um_read_length_indicators(unsigned char**dataP, rlc_um_e_li_t* e_liP, unsigned int* li_arrayP, unsigned int *num_liP, unsigned int *data_sizeP) {
//-----------------------------------------------------------------------------
    int continue_loop = 1;
    *num_liP = 0;

    while ((continue_loop)) {
        li_arrayP[*num_liP] = e_liP->li1;
        *data_sizeP = *data_sizeP - e_liP->li1;
        *dataP = &*dataP[e_liP->li1];
        *num_liP = *num_liP +1;
        if ((e_liP->e1)) {
            li_arrayP[*num_liP] = e_liP->li2;
            *data_sizeP = *data_sizeP - e_liP->li2;
            *dataP = &*dataP[e_liP->li2];
            *num_liP = *num_liP +1;
            if (e_liP->e2 == 0) {
                continue_loop = 0;
            } else {
                e_liP++;
            }
        } else {
            continue_loop = 0;
        }
        if (*num_liP >= RLC_UM_SEGMENT_NB_MAX_LI_PER_PDU) {
            return -1;
        }
    }
    return 0;
}
//-----------------------------------------------------------------------------
void rlc_um_try_reassembly(rlc_um_entity_t *rlcP, u16_t snP) {
//-----------------------------------------------------------------------------
    mem_block_t        *pdu_mem;
    struct mac_tb_ind  *tb_ind;
    rlc_um_e_li_t      *e_li;
    unsigned char      *data;
    int                 e;
    int                 fi;
    unsigned int        size;
    u16_t sn;
    u16_t sn_tmp = snP;
    unsigned int        num_li;
    unsigned int        li_array[RLC_UM_SEGMENT_NB_MAX_LI_PER_PDU];
    int i;

    assert(rlcP->dar_buffer[snP] != NULL);
    // go backward in the buffer till there are continuous PDUs
    sn_tmp = snP;
    while ((rlcP->dar_buffer[sn]) && (sn != rlcP->vr_uh)) {
        sn = sn_tmp;
        sn_tmp = (sn_tmp - 1) & ((1 << rlcP->sn_length) - 1);
    }

    while ((pdu_mem = rlcP->dar_buffer[sn])) {
        tb_ind = (struct mac_tb_ind *)(pdu_mem->data);
        if (rlcP->sn_length == 10) {
            e = ((rlc_um_pdu_sn_10_t*)(tb_ind->data_ptr))->e;
            fi = ((rlc_um_pdu_sn_10_t*)(tb_ind->data_ptr))->fi;
            e_li = (rlc_um_e_li_t*)((rlc_um_pdu_sn_10_t*)(tb_ind->data_ptr))->data;
            size   = tb_ind->size - 2;
            data = &tb_ind->data_ptr[2];
        } else {
            e = ((rlc_um_pdu_sn_5_t*)(tb_ind->data_ptr))->e;
            fi = ((rlc_um_pdu_sn_5_t*)(tb_ind->data_ptr))->fi;
            e_li = (rlc_um_e_li_t*)((rlc_um_pdu_sn_5_t*)(tb_ind->data_ptr))->data;
            size   = tb_ind->size - 1;
            data = &tb_ind->data_ptr[1];
        }
        if (e == RLC_E_FIXED_PART_DATA_FIELD_FOLLOW) {
            switch (fi) {
                case RLC_FI_1ST_BYTE_DATA_IS_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_LAST_BYTE_SDU:
#ifdef DEBUG_RLC_UM_RX_DECODE
            msg ("[RLC_UM][MOD %d][RB %d] RX PDU NO E_LI FI=11 (00)\n", rlcP->module_id, rlcP->rb_id);
#endif
                    // one complete SDU
                    rlc_um_send_sdu(rlcP); // may be not necessary
                    rlc_um_reassembly (data, size, rlcP);
                    rlc_um_send_sdu(rlcP); // may be not necessary
                    break;
                case RLC_FI_1ST_BYTE_DATA_IS_1ST_BYTE_PDU_LAST_BYTE_DATA_IS_NOT_LAST_BYTE_SDU:
#ifdef DEBUG_RLC_UM_RX_DECODE
            msg ("[RLC_UM][MOD %d][RB %d] RX PDU NO E_LI FI=10 (01)\n", rlcP->module_id, rlcP->rb_id);
#endif
                    // one beginning segment of SDU in PDU
                    rlc_um_send_sdu(rlcP); // may be not necessary
                    rlc_um_reassembly (data, size, rlcP);
                    break;
                case RLC_FI_1ST_BYTE_DATA_IS_NOT_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_LAST_BYTE_SDU:
#ifdef DEBUG_RLC_UM_RX_DECODE
            msg ("[RLC_UM][MOD %d][RB %d] RX PDU NO E_LI FI=01 (10)\n", rlcP->module_id, rlcP->rb_id);
#endif
                    // one last segment of SDU
                    rlc_um_reassembly (data, size, rlcP);
                    rlc_um_send_sdu(rlcP);
                    break;
                case RLC_FI_1ST_BYTE_DATA_IS_NOT_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_NOT_LAST_BYTE_SDU:
#ifdef DEBUG_RLC_UM_RX_DECODE
            msg ("[RLC_UM][MOD %d][RB %d] RX PDU NO E_LI FI=00 (11)\n", rlcP->module_id, rlcP->rb_id);
#endif
                    // one whole segment of SDU in PDU
                    rlc_um_reassembly (data, size, rlcP);
                    break;
                default:;
            }
        } else {
            if (rlc_um_read_length_indicators(&data, e_li, li_array, &num_li, &size ) >= 0) {
                switch (fi) {
                    case RLC_FI_1ST_BYTE_DATA_IS_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_LAST_BYTE_SDU:
#ifdef DEBUG_RLC_UM_RX_DECODE
            msg ("[RLC_UM][MOD %d][RB %d] RX PDU FI=11 (00) Li=", rlcP->module_id, rlcP->rb_id);
            for (i=0; i < num_li; i++) {
                msg("%d ",li_array[i]);
            }
            msg(" remaining size %d\n",size);
#endif
                        // N complete SDUs
                        rlc_um_send_sdu(rlcP);
                        for (i = 0; i < num_li; num_li++) {
                            rlc_um_reassembly (data, li_array[i], rlcP);
                            rlc_um_send_sdu(rlcP);
                            data = &data[li_array[i]];
                        }
                        if (size > 0) { // normally should always be > 0 but just for help debug
                            // data is already ok, done by last loop above
                            rlc_um_reassembly (data, size, rlcP);
                            rlc_um_send_sdu(rlcP);
                        }
                        break;
                    case RLC_FI_1ST_BYTE_DATA_IS_1ST_BYTE_PDU_LAST_BYTE_DATA_IS_NOT_LAST_BYTE_SDU:
#ifdef DEBUG_RLC_UM_RX_DECODE
            msg ("[RLC_UM][MOD %d][RB %d] RX PDU FI=10 (01) Li=", rlcP->module_id, rlcP->rb_id);
            for (i=0; i < num_li; i++) {
                msg("%d ",li_array[i]);
            }
            msg(" remaining size %d\n",size);
#endif
                        // N complete SDUs + one segment of SDU in PDU
                        rlc_um_send_sdu(rlcP);
                        for (i = 0; i < num_li; num_li++) {
                            rlc_um_reassembly (data, li_array[i], rlcP);
                            rlc_um_send_sdu(rlcP);
                            data = &data[li_array[i]];
                        }
                        if (size > 0) { // normally should always be > 0 but just for help debug
                            // data is already ok, done by last loop above
                            rlc_um_reassembly (data, size, rlcP);
                        }
                        break;
                    case RLC_FI_1ST_BYTE_DATA_IS_NOT_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_LAST_BYTE_SDU:
#ifdef DEBUG_RLC_UM_RX_DECODE
            msg ("[RLC_UM][MOD %d][RB %d] RX PDU FI=01 (10) Li=", rlcP->module_id, rlcP->rb_id);
            for (i=0; i < num_li; i++) {
                msg("%d ",li_array[i]);
            }
            msg(" remaining size %d\n",size);
#endif
                        // one last segment of SDU + N complete SDUs in PDU
                        for (i = 0; i < num_li; num_li++) {
                            rlc_um_reassembly (data, li_array[i], rlcP);
                            rlc_um_send_sdu(rlcP);
                            data = &data[li_array[i]];
                        }
                        if (size > 0) { // normally should always be > 0 but just for help debug
                            // data is already ok, done by last loop above
                            rlc_um_reassembly (data, size, rlcP);
                            rlc_um_send_sdu(rlcP);
                        }
                        break;
                    case RLC_FI_1ST_BYTE_DATA_IS_NOT_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_NOT_LAST_BYTE_SDU:
#ifdef DEBUG_RLC_UM_RX_DECODE
            msg ("[RLC_UM][MOD %d][RB %d] RX PDU FI=00 (11) Li=", rlcP->module_id, rlcP->rb_id);
            for (i=0; i < num_li; i++) {
                msg("%d ",li_array[i]);
            }
            msg(" remaining size %d\n",size);
#endif
                        for (i = 0; i < num_li; num_li++) {
                            rlc_um_reassembly (data, li_array[i], rlcP);
                            rlc_um_send_sdu(rlcP);
                            data = &data[li_array[i]];
                        }
                        if (size > 0) { // normally should always be > 0 but just for help debug
                            // data is already ok, done by last loop above
                            rlc_um_reassembly (data, size, rlcP);
                        }
                        break;
                    default:;
                }
            }
        }
    }

}
//-----------------------------------------------------------------------------
void rlc_um_check_timer_dar_time_out(rlc_um_entity_t *rlcP) {
//-----------------------------------------------------------------------------
    signed int in_window;
    u16_t     saved_sn;
    if ((rlcP->timer_reordering_running)) {
        if ((rlcP->timer_reordering  + rlcP->timer_reordering_init)   == *rlcP->frame_tick_milliseconds) {
            // 5.1.2.2.4   Actions when t-Reordering expires
            //  When t-Reordering expires, the receiving UM RLC entity shall:
            //  -update VR(UR) to the SN of the first UMD PDU with SN >= VR(UX) that has not been received;
            //  -reassemble RLC SDUs from any UMD PDUs with SN < updated VR(UR), remove RLC headers when doing so and deliver the reassembled RLC SDUs to upper layer in ascending order of the RLC SN if not delivered before;
            //  -if VR(UH) > VR(UR):
            //      -start t-Reordering;
            //      -set VR(UX) to VR(UH).
#ifdef DEBUG_RLC_UM_RX
            msg ("[RLC_UM][MOD %d][RB %d] TIMER t-Reordering expiration\n", rlcP->module_id, rlcP->rb_id);
            msg ("[RLC_UM][MOD %d][RB %d] set VR(UR) to", rlcP->module_id, rlcP->rb_id);
#endif
            saved_sn = rlcP->vr_ur;
            rlcP->vr_ur = rlcP->vr_ux;
            while (rlc_um_get_pdu_from_dar_buffer(rlcP, rlcP->vr_ur)) {
                rlcP->vr_ur = (rlcP->vr_ur+1)%(1 << rlcP->sn_length);
#ifdef DEBUG_RLC_UM_RX
            msg (".");
#endif
            }
#ifdef DEBUG_RLC_UM_RX
            msg (" %d\n", rlcP->vr_ur);
#endif

            rlc_um_try_reassembly(rlcP, saved_sn);

            in_window = rlc_um_in_window(rlcP, rlcP->vr_ur,  rlcP->vr_uh,  rlcP->vr_uh);
            if (in_window == 2) {
                rlcP->timer_reordering_running = 1;
                rlcP->timer_reordering         = rlcP->timer_reordering_init;
                rlcP->vr_ux = rlcP->vr_uh;
#ifdef DEBUG_RLC_UM_RX
                msg ("[RLC_UM][MOD %d][RB %d] restarting t-Reordering set VR(UX) to %d (VR(UH)>VR(UR))\n", rlcP->module_id, rlcP->rb_id, rlcP->vr_ux);
#endif
            }
        }
    }
}
//-----------------------------------------------------------------------------
inline mem_block_t *
rlc_um_remove_pdu_from_dar_buffer(rlc_um_entity_t *rlcP, u16_t snP)
{
//-----------------------------------------------------------------------------
    mem_block_t * pdu     = rlcP->dar_buffer[snP];
    rlcP->dar_buffer[snP] = NULL;
    return pdu;
}
//-----------------------------------------------------------------------------
inline mem_block_t *
rlc_um_get_pdu_from_dar_buffer(rlc_um_entity_t *rlcP, u16_t snP)
{
//-----------------------------------------------------------------------------

    return rlcP->dar_buffer[snP];
}
//-----------------------------------------------------------------------------
inline void
rlc_um_store_pdu_in_dar_buffer(rlc_um_entity_t *rlcP, mem_block_t *pduP, u16_t snP)
{
//-----------------------------------------------------------------------------

    rlcP->dar_buffer[snP] = pduP;
}
//-----------------------------------------------------------------------------
inline signed int rlc_um_in_window(rlc_um_entity_t *rlcP, signed int lower_boundP, signed int snP, signed int higher_boundP) {
//-----------------------------------------------------------------------------
    int modulo = 1 << rlcP->sn_length;
    signed int modulus = (signed int)rlcP->vr_uh - (signed int)(modulo >> 1);
    lower_boundP  = (lower_boundP  - modulus) % modulo;
    higher_boundP = (higher_boundP - modulus) % modulo;
    snP           = (snP           - modulus) % modulo;

    if ( lower_boundP > snP) {
        return -2;
    }
    if ( higher_boundP < snP) {
        return -1;
    }
    if ( lower_boundP == snP) {
        if ( higher_boundP == snP) {
            return 3;
        }
        return 1;
    }
    if ( higher_boundP == snP) {
        return 2;
    }
    return 0;

}
//-----------------------------------------------------------------------------
inline signed int rlc_um_in_reordering_window(rlc_um_entity_t *rlcP, signed int snP) {
//-----------------------------------------------------------------------------
    int modulo = 1 << rlcP->sn_length;
    signed int modulus = (signed int)rlcP->vr_uh - (signed int)(modulo >> 1);
    snP           = (snP - modulus) % modulo;

    if ( 0 <= snP) {
        if (snP < (signed int)(modulo >> 1)) {
            return 0;
        }
    }
    return -1;
}
//-----------------------------------------------------------------------------
void
rlc_um_receive_process_dar (rlc_um_entity_t *rlcP, mem_block_t *pdu_memP,rlc_um_pdu_sn_10_t *pduP, u16_t tb_sizeP)
{
//-----------------------------------------------------------------------------
    // 36.322v9.3.0 section 5.1.2.2.1:
    // The receiving UM RLC entity shall maintain a reordering window according to state variable VR(UH) as follows:
    //      -a SN falls within the reordering window if (VR(UH) – UM_Window_Size) <= SN < VR(UH);
    //      -a SN falls outside of the reordering window otherwise.
    // When receiving an UMD PDU from lower layer, the receiving UM RLC entity shall:
    //      -either discard the received UMD PDU or place it in the reception buffer (see sub clause 5.1.2.2.2);
    //      -if the received UMD PDU was placed in the reception buffer:
    //          -update state variables, reassemble and deliver RLC SDUs to upper layer and start/stop t-Reordering as needed (see sub clause 5.1.2.2.3);
    // When t-Reordering expires, the receiving UM RLC entity shall:
    // -   update state variables, reassemble and deliver RLC SDUs to upper layer and start t-Reordering as needed (see sub clause 5.1.2.2.4).



    // When an UMD PDU with SN = x is received from lower layer, the receiving UM RLC entity shall:
    // -if VR(UR) < x < VR(UH) and the UMD PDU with SN = x has been received before; or
    // -if (VR(UH) – UM_Window_Size) <= x < VR(UR):
    //      -discard the received UMD PDU;
    // -else:
    //      -place the received UMD PDU in the reception buffer.

    unsigned int sn_tmp;
    signed int sn = pduP->sn;
    signed int in_window = rlc_um_in_window(rlcP, rlcP->vr_uh - UM_WINDOW_SIZE_SN_10_BITS, sn, rlcP->vr_ur);

    if ((in_window == 1) || (in_window == 0)){
#ifdef DEBUG_RLC_UM_RX
        msg ("[RLC_UM][MOD %d][RB %d] RX PDU  VR(UH) – UM_Window_Size) <= SN %d < VR(UR) -> GARBAGE\n", rlcP->module_id, rlcP->rb_id, sn);
#endif
        //discard the PDU
        free_mem_block(pdu_memP);
        pdu_memP = NULL;
        return;
    }
    if ((rlc_um_get_pdu_from_dar_buffer(rlcP, sn))) {
        in_window = rlc_um_in_window(rlcP, rlcP->vr_ur, sn, rlcP->vr_uh);
        if (in_window == 0){
#ifdef DEBUG_RLC_UM_RX
        msg ("[RLC_UM][MOD %d][RB %d] RX PDU  VR(UR) < SN %d < VR(UH) and RECEIVED BEFORE-> GARBAGE\n", rlcP->module_id, rlcP->rb_id, sn);
#endif
            //discard the PDU
            free_mem_block(pdu_memP);
            pdu_memP = NULL;
            return;
        }
    }
    rlc_um_store_pdu_in_dar_buffer(rlcP, pdu_memP, sn);


    // -if x falls outside of the reordering window:
    //      -update VR(UH) to x + 1;
    //      -reassemble RLC SDUs from any UMD PDUs with SN that falls outside of
    //       the reordering window, remove RLC headers when doing so and deliver
    //       the reassembled RLC SDUs to upper layer in ascending order of the
    //       RLC SN if not delivered before;
    //
    //      -if VR(UR) falls outside of the reordering window:
    //          -set VR(UR) to (VR(UH) – UM_Window_Size);
    if (rlc_um_in_reordering_window(rlcP, sn) < 0) {
#ifdef DEBUG_RLC_UM_RX
        msg ("[RLC_UM][MOD %d][RB %d] RX PDU  SN %d OUTSIDE REORDERING WINDOW\n", rlcP->module_id, rlcP->rb_id, sn);
#endif
        rlcP->vr_uh = (sn + 1) % (1 << rlcP->sn_length);

        rlc_um_try_reassembly(rlcP, sn);

        if (rlc_um_in_reordering_window(rlcP, rlcP->vr_ur) < 0) {
            in_window = rlcP->vr_uh - (1 << (rlcP->sn_length-1));
            if (in_window < 0) {
                in_window = in_window + (1 << rlcP->sn_length);
            }
#ifdef DEBUG_RLC_UM_RX
            msg ("[RLC_UM][MOD %d][RB %d] VR(UR) %d OUTSIDE REORDERING WINDOW SET TO VR(UH) – UM_Window_Size = %d\n", rlcP->module_id, rlcP->rb_id, rlcP->vr_ur, in_window);
#endif
            rlcP->vr_uh = in_window;
        }
    }
    // -if the reception buffer contains an UMD PDU with SN = VR(UR):
    //      -update VR(UR) to the SN of the first UMD PDU with SN > current
    //          VR(UR) that has not been received;
    //      -reassemble RLC SDUs from any UMD PDUs with SN < updated VR(UR),
    //          remove RLC headers when doing so and deliver the reassembled RLC
    //          SDUs to upper layer in ascending order of the RLC SN if not
    //          delivered before;
    if (rlc_um_get_pdu_from_dar_buffer(rlcP, rlcP->vr_ur)) {
        sn_tmp = rlcP->vr_ur;
        do {
            rlcP->vr_ur = (rlcP->vr_ur+1)%(1 << rlcP->sn_length);
        } while (rlc_um_get_pdu_from_dar_buffer(rlcP, rlcP->vr_ur));
        rlc_um_try_reassembly(rlcP, sn_tmp);
    }

    // -if t-Reordering is running:
    //      -if VR(UX) <= VR(UR); or
    //      -if VR(UX) falls outside of the reordering window and VR(UX) is not
    //          equal to VR(UH)::
    //          -stop and reset t-Reordering;
    if (rlcP->timer_reordering_running) {
        if (rlcP->vr_uh != rlcP->vr_ux) {
            in_window = rlc_um_in_reordering_window(rlcP, rlcP->vr_ux);
            if (in_window < 0) {
                rlcP->timer_reordering_running = 0;
                rlcP->timer_reordering         = 0;
            }
        }
    }
    if (rlcP->timer_reordering_running) {
        in_window = rlc_um_in_window(rlcP, rlcP->vr_ux,  rlcP->vr_ur,  rlcP->vr_ur);
        if (in_window >= 0) {
            rlcP->timer_reordering_running = 0;
            rlcP->timer_reordering         = 0;
        }
    }
    // -if t-Reordering is not running (includes the case when t-Reordering is
    //      stopped due to actions above):
    //      -if VR(UH) > VR(UR):
    //          -start t-Reordering;
    //          -set VR(UX) to VR(UH).
    if (rlcP->timer_reordering_running == 0) {
        in_window = rlc_um_in_window(rlcP, rlcP->vr_ur,  rlcP->vr_uh,  rlcP->vr_uh);
        if (in_window == 2) {
            rlcP->timer_reordering_running = 1;
            rlcP->timer_reordering         = rlcP->timer_reordering_init;
            rlcP->vr_ux = rlcP->vr_uh;
        }
    }
    rlc_um_check_timer_dar_time_out(rlcP);
}
