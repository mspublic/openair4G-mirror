#define RLC_UM_MODULE
#define RLC_UM_DAR_C
#include "rtos_header.h"
#include "platform_types.h"
//-----------------------------------------------------------------------------
#include "rlc.h"
#include "rlc_um.h"
#include "rlc_primitives.h"
#include "mac_primitives.h"
#include "list.h"
#include "MAC_INTERFACE/extern.h"

#define DEBUG_RLC_UM_RX_DECODE
//#define DEBUG_RLC_UM_RX
//-----------------------------------------------------------------------------
int rlc_um_read_length_indicators(unsigned char**dataP, rlc_um_e_li_t* e_liP, unsigned int* li_arrayP, unsigned int *num_liP, unsigned int *data_sizeP) {
//-----------------------------------------------------------------------------
    int continue_loop = 1;
    *num_liP = 0;
    unsigned int e1;
    unsigned int li1;
    unsigned int e2;
    unsigned int li2;

    while ((continue_loop)) {
        //msg("[RLC_UM] e_liP->b1 = %02X\n", e_liP->b1);
        //msg("[RLC_UM] e_liP->b2 = %02X\n", e_liP->b2);
        e1 = ((unsigned int)e_liP->b1 & 0x00000080) >> 7;
        li1 = (((unsigned int)e_liP->b1 & 0x0000007F) << 4) + (((unsigned int)e_liP->b2 & 0x000000F0) >> 4);
        li_arrayP[*num_liP] = li1;
        *data_sizeP = *data_sizeP - li1 - 2;
        *num_liP = *num_liP +1;
        if ((e1)) {
            e2 = ((unsigned int)e_liP->b2 & 0x00000008) >> 3;
            li2 = (((unsigned int)e_liP->b2 & 0x00000007) << 8) + ((unsigned int)e_liP->b3 & 0x000000FF);
            li_arrayP[*num_liP] = li2;
            *data_sizeP = *data_sizeP - li2 - 1;
            *num_liP = *num_liP +1;
            if (e2 == 0) {
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
    *dataP = *dataP + (((*num_liP*3) +1) >> 1);
    return 0;
}
//-----------------------------------------------------------------------------
void rlc_um_try_reassembly(rlc_um_entity_t *rlcP, signed int snP) {
//-----------------------------------------------------------------------------
    mem_block_t        *pdu_mem;
    struct mac_tb_ind  *tb_ind;
    rlc_um_e_li_t      *e_li;
    unsigned char      *data;
    int                 e;
    int                 fi;
    unsigned int        size;
    signed int          sn;
    unsigned int        continue_reassembly;
    unsigned int        num_li;
    unsigned int        li_array[RLC_UM_SEGMENT_NB_MAX_LI_PER_PDU];
    int i;
    int reassembly_start_index;

    if (snP < 0) snP = snP + rlcP->sn_modulo;

#ifdef DEBUG_RLC_UM_RX
    msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] TRY REASSEMBLY FROM PDU SN=%03d+1  TO  PDU SN=%03d\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, rlcP->last_reassemblied_sn, snP);
#endif

    continue_reassembly = 1;
    sn = (rlcP->last_reassemblied_sn + 1) % rlcP->sn_modulo;

    while (continue_reassembly) {
        if ((pdu_mem = rlcP->dar_buffer[sn])) {
            rlcP->last_reassemblied_sn = sn;
#ifdef DEBUG_RLC_UM_RX
            msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] TRY REASSEMBLY PDU SN=%03d\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, sn);
#endif
            tb_ind = (struct mac_tb_ind *)(pdu_mem->data);
            if (rlcP->sn_length == 10) {
                e  = (((rlc_um_pdu_sn_10_t*)(tb_ind->data_ptr))->b1 & 0x04) >> 2;
                fi = (((rlc_um_pdu_sn_10_t*)(tb_ind->data_ptr))->b1 & 0x18) >> 3;
                e_li = (rlc_um_e_li_t*)((rlc_um_pdu_sn_10_t*)(tb_ind->data_ptr))->data;
                size   = tb_ind->size - 2;
                data = &tb_ind->data_ptr[2];
            } else {
                e  = (((rlc_um_pdu_sn_5_t*)(tb_ind->data_ptr))->b1 & 0x20) >> 5;
                fi = (((rlc_um_pdu_sn_5_t*)(tb_ind->data_ptr))->b1 & 0xC0) >> 6;
                e_li = (rlc_um_e_li_t*)((rlc_um_pdu_sn_5_t*)(tb_ind->data_ptr))->data;
                size   = tb_ind->size - 1;
                data = &tb_ind->data_ptr[1];
            }
            if (e == RLC_E_FIXED_PART_DATA_FIELD_FOLLOW) {
                switch (fi) {
                    case RLC_FI_1ST_BYTE_DATA_IS_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_LAST_BYTE_SDU:
#ifdef DEBUG_RLC_UM_RX_DECODE
                        msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] TRY REASSEMBLY PDU NO E_LI FI=11 (00)\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame);
#endif
                        // one complete SDU
                        rlc_um_send_sdu(rlcP); // may be not necessary
                        rlc_um_reassembly (data, size, rlcP);
                        rlc_um_send_sdu(rlcP); // may be not necessary
                        rlcP->reassembly_missing_sn_detected = 0;
                        break;
                    case RLC_FI_1ST_BYTE_DATA_IS_1ST_BYTE_PDU_LAST_BYTE_DATA_IS_NOT_LAST_BYTE_SDU:
#ifdef DEBUG_RLC_UM_RX_DECODE
                        msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] TRY REASSEMBLY PDU NO E_LI FI=10 (01)\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame);
#endif
                        // one beginning segment of SDU in PDU
                        rlc_um_send_sdu(rlcP); // may be not necessary
                        rlc_um_reassembly (data, size, rlcP);
                        rlcP->reassembly_missing_sn_detected = 0;
                        break;
                    case RLC_FI_1ST_BYTE_DATA_IS_NOT_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_LAST_BYTE_SDU:
#ifdef DEBUG_RLC_UM_RX_DECODE
                        msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] TRY REASSEMBLY PDU NO E_LI FI=01 (10)\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame);
#endif
                        // one last segment of SDU
                        if (rlcP->reassembly_missing_sn_detected == 0) {
                            rlc_um_reassembly (data, size, rlcP);
                            rlc_um_send_sdu(rlcP);
                        } // else { clear sdu already done
                        rlcP->reassembly_missing_sn_detected = 0;
                        break;
                    case RLC_FI_1ST_BYTE_DATA_IS_NOT_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_NOT_LAST_BYTE_SDU:
#ifdef DEBUG_RLC_UM_RX_DECODE
                        msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] TRY REASSEMBLY PDU NO E_LI FI=00 (11)\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame);
#endif
                        if (rlcP->reassembly_missing_sn_detected == 0) {
                            // one whole segment of SDU in PDU
                            rlc_um_reassembly (data, size, rlcP);
                        } else {
                            rlcP->reassembly_missing_sn_detected = 1; // not necessary but for readability of the code
                        }

                        break;
#ifdef USER_MODE
		default:
		  assert(0 != 0);
#endif
                }
            } else {
                if (rlc_um_read_length_indicators(&data, e_li, li_array, &num_li, &size ) >= 0) {
                    switch (fi) {
                        case RLC_FI_1ST_BYTE_DATA_IS_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_LAST_BYTE_SDU:
#ifdef DEBUG_RLC_UM_RX_DECODE
                            msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] TRY REASSEMBLY PDU FI=11 (00) Li=", rlcP->module_id, rlcP->rb_id, mac_xface->frame);
                            for (i=0; i < num_li; i++) {
                                msg("%d ",li_array[i]);
                            }
                            msg(" remaining size %d\n",size);
#endif
                            // N complete SDUs
                            rlc_um_send_sdu(rlcP);

                            for (i = 0; i < num_li; i++) {
                                rlc_um_reassembly (data, li_array[i], rlcP);
                                rlc_um_send_sdu(rlcP);
                                data = &data[li_array[i]];
                            }
                            if (size > 0) { // normally should always be > 0 but just for help debug
                                // data is already ok, done by last loop above
                                rlc_um_reassembly (data, size, rlcP);
                                rlc_um_send_sdu(rlcP);
                            }
                            rlcP->reassembly_missing_sn_detected = 0;
                            break;
                        case RLC_FI_1ST_BYTE_DATA_IS_1ST_BYTE_PDU_LAST_BYTE_DATA_IS_NOT_LAST_BYTE_SDU:
#ifdef DEBUG_RLC_UM_RX_DECODE
                            msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] TRY REASSEMBLY PDU FI=10 (01) Li=", rlcP->module_id, rlcP->rb_id, mac_xface->frame);
                            for (i=0; i < num_li; i++) {
                                msg("%d ",li_array[i]);
                            }
                            msg(" remaining size %d\n",size);
#endif
                            // N complete SDUs + one segment of SDU in PDU
                            rlc_um_send_sdu(rlcP);
                            for (i = 0; i < num_li; i++) {
                                rlc_um_reassembly (data, li_array[i], rlcP);
                                rlc_um_send_sdu(rlcP);
                                data = &data[li_array[i]];
                            }
                            if (size > 0) { // normally should always be > 0 but just for help debug
                                // data is already ok, done by last loop above
                                rlc_um_reassembly (data, size, rlcP);
                            }
                            rlcP->reassembly_missing_sn_detected = 0;
                            break;
                        case RLC_FI_1ST_BYTE_DATA_IS_NOT_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_LAST_BYTE_SDU:
#ifdef DEBUG_RLC_UM_RX_DECODE
                            msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] TRY REASSEMBLY PDU FI=01 (10) Li=", rlcP->module_id, rlcP->rb_id, mac_xface->frame);
                            for (i=0; i < num_li; i++) {
                                msg("%d ",li_array[i]);
                            }
                            msg(" remaining size %d\n",size);
#endif
                            if (rlcP->reassembly_missing_sn_detected) {
                                reassembly_start_index = 1;
                                data = &data[li_array[0]];
                            } else {
                                reassembly_start_index = 0;
                            }

                            // one last segment of SDU + N complete SDUs in PDU
                            for (i = reassembly_start_index; i < num_li; i++) {
                                rlc_um_reassembly (data, li_array[i], rlcP);
                                rlc_um_send_sdu(rlcP);
                                data = &data[li_array[i]];
                            }
                            if (size > 0) { // normally should always be > 0 but just for help debug
                                // data is already ok, done by last loop above
                                rlc_um_reassembly (data, size, rlcP);
                                rlc_um_send_sdu(rlcP);
                            }
                            rlcP->reassembly_missing_sn_detected = 0;
                            break;
                        case RLC_FI_1ST_BYTE_DATA_IS_NOT_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_NOT_LAST_BYTE_SDU:
#ifdef DEBUG_RLC_UM_RX_DECODE
                            msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] TRY REASSEMBLY PDU FI=00 (11) Li=", rlcP->module_id, rlcP->rb_id, mac_xface->frame);
                            for (i=0; i < num_li; i++) {
                                msg("%d ",li_array[i]);
                            }
                            msg(" remaining size %d\n",size);
#endif
                            if (rlcP->reassembly_missing_sn_detected) {
#ifdef DEBUG_RLC_UM_RX_DECODE
                                msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] DISCARD FIRST LI %d", rlcP->module_id, rlcP->rb_id, mac_xface->frame, li_array[0]);
#endif
                                reassembly_start_index = 1;
                                data = &data[li_array[0]];
                            } else {
                                reassembly_start_index = 0;
                            }

                            for (i = reassembly_start_index; i < num_li; i++) {
                                rlc_um_reassembly (data, li_array[i], rlcP);
                                rlc_um_send_sdu(rlcP);
                                data = &data[li_array[i]];
                            }
                            if (size > 0) { // normally should always be > 0 but just for help debug
                                // data is already ok, done by last loop above
                                rlc_um_reassembly (data, size, rlcP);
                            } else {
#ifdef USER_MODE
			      assert (5!=5);
#endif
                            }
                            rlcP->reassembly_missing_sn_detected = 0;
                            break;
#ifdef USER_MODE
                        default:
			  assert(1 != 1);
#endif
			  rlcP->reassembly_missing_sn_detected = 1;
                    }
                }
            }
            free_mem_block(rlcP->dar_buffer[sn]);
            rlcP->dar_buffer[sn] = NULL;
        } else {
            rlcP->last_reassemblied_missing_sn = sn;
            rlcP->reassembly_missing_sn_detected = 1;
            rlc_um_clear_rx_sdu(rlcP);
        }
        sn = (sn + 1) % rlcP->sn_modulo;
        if ((sn == rlcP->vr_uh) || (sn == snP)){
            continue_reassembly = 0;
        }
    }
#ifdef DEBUG_RLC_UM_RX
    msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] TRIED REASSEMBLY VR(UR)=%03d VR(UX)=%03d VR(UH)=%03d\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, rlcP->vr_ur, rlcP->vr_ux,rlcP->vr_uh);
#endif
}
//-----------------------------------------------------------------------------
void rlc_um_check_timer_dar_time_out(rlc_um_entity_t *rlcP) {
//-----------------------------------------------------------------------------
    signed int in_window;
    u16_t     saved_sn;
    if ((rlcP->timer_reordering_running)) {
        if ((rlcP->timer_reordering  + rlcP->timer_reordering_init)   == mac_xface->frame) {
            // 5.1.2.2.4   Actions when t-Reordering expires
            //  When t-Reordering expires, the receiving UM RLC entity shall:
            //  -update VR(UR) to the SN of the first UMD PDU with SN >= VR(UX) that has not been received;
            //  -reassemble RLC SDUs from any UMD PDUs with SN < updated VR(UR), remove RLC headers when doing so and deliver the reassembled RLC SDUs to upper layer in ascending order of the RLC SN if not delivered before;
            //  -if VR(UH) > VR(UR):
            //      -start t-Reordering;
            //      -set VR(UX) to VR(UH).
#ifdef DEBUG_RLC_UM_RX
            msg ("\n\n[RLC_UM][MOD %d][RB %d][FRAME %05d]*****************************************************\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame);
            msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d]*    T I M E  -  O U T                              *\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame);
            msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d]*****************************************************\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame);
            msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] TIMER t-Reordering expiration\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame);
            msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] timer_reordering=%d frame=%d\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, rlcP->timer_reordering, mac_xface->frame);
            msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] set VR(UR)=%03d to", rlcP->module_id, rlcP->rb_id, mac_xface->frame, rlcP->vr_ur);
#endif
            saved_sn = rlcP->vr_ur;
            rlcP->vr_ur = rlcP->vr_ux;
            while (rlc_um_get_pdu_from_dar_buffer(rlcP, rlcP->vr_ur)) {
                rlcP->vr_ur = (rlcP->vr_ur+1)%rlcP->sn_modulo;
#ifdef DEBUG_RLC_UM_RX
            msg (".");
#endif
            }
#ifdef DEBUG_RLC_UM_RX
            msg (" %d\n", rlcP->vr_ur);
#endif

            rlc_um_try_reassembly(rlcP, rlcP->vr_ur);

            in_window = rlc_um_in_window(rlcP, rlcP->vr_ur,  rlcP->vr_uh,  rlcP->vr_uh);
            if (in_window == 2) {
                rlcP->timer_reordering_running = 1;
                rlcP->timer_reordering         = rlcP->timer_reordering_init;
                rlcP->vr_ux = rlcP->vr_uh;
#ifdef DEBUG_RLC_UM_RX
                msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] restarting t-Reordering set VR(UX) to %d (VR(UH)>VR(UR))\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, rlcP->vr_ux);
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

#ifdef DEBUG_RLC_UM_RX
    msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] STORE PDU IN DAR BUFFER  SN=%03d  VR(UR)=%03d VR(UX)=%03d VR(UH)=%03d\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, snP, rlcP->vr_ur, rlcP->vr_ux, rlcP->vr_uh);
#endif
    rlcP->dar_buffer[snP] = pduP;
}
//-----------------------------------------------------------------------------
// returns -2 if lower_bound  > sn
// returns -1 if higher_bound < sn
// returns  0 if lower_bound  < sn < higher_bound
// returns  1 if lower_bound  == sn
// returns  2 if higher_bound == sn
// returns  3 if higher_bound == sn == lower_bound
inline signed int rlc_um_in_window(rlc_um_entity_t *rlcP, signed int lower_boundP, signed int snP, signed int higher_boundP) {
//-----------------------------------------------------------------------------

    signed int modulus = (signed int)rlcP->vr_uh - rlcP->um_window_size;
#ifdef DEBUG_RLC_UM_RX
    signed int     lower_bound  = lower_boundP;
    signed int     higher_bound = higher_boundP;
    signed int     sn           = snP;
#endif
    lower_boundP  = (lower_boundP  - modulus) % rlcP->sn_modulo;
    higher_boundP = (higher_boundP - modulus) % rlcP->sn_modulo;
    snP           = (snP           - modulus) % rlcP->sn_modulo;

    if ( lower_boundP > snP) {
#ifdef DEBUG_RLC_UM_RX
        msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] %d not in WINDOW[%03d:%03d] (SN<LOWER BOUND)\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, sn, lower_bound, higher_bound);
#endif
        return -2;
    }
    if ( higher_boundP < snP) {
#ifdef DEBUG_RLC_UM_RX
        msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] %d not in WINDOW[%03d:%03d] (SN>HIGHER BOUND) <=> %d not in WINDOW[%03d:%03d]\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, sn, lower_bound, higher_bound, snP, lower_boundP, higher_boundP);
#endif
        return -1;
    }
    if ( lower_boundP == snP) {
        if ( higher_boundP == snP) {
#ifdef DEBUG_RLC_UM_RX
        msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] %d  in WINDOW[%03d:%03d] (SN=HIGHER BOUND=LOWER BOUND)\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, sn, lower_bound, higher_bound);
#endif
            return 3;
        }
#ifdef DEBUG_RLC_UM_RX
        msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] %d  in WINDOW[%03d:%03d] (SN=LOWER BOUND)\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, sn, lower_bound, higher_bound);
#endif
        return 1;
    }
    if ( higher_boundP == snP) {
#ifdef DEBUG_RLC_UM_RX
        msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] %d  in WINDOW[%03d:%03d] (SN=HIGHER BOUND)\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, sn, lower_bound, higher_bound);
#endif
        return 2;
    }
    return 0;

}
//-----------------------------------------------------------------------------
inline signed int rlc_um_in_reordering_window(rlc_um_entity_t *rlcP, signed int snP) {
//-----------------------------------------------------------------------------
    signed int modulus = (signed int)rlcP->vr_uh - rlcP->um_window_size;
#ifdef DEBUG_RLC_UM_RX
signed int sn = snP;
#endif
    snP           = (snP - modulus) % rlcP->sn_modulo;

    if ( 0 <= snP) {
        if (snP < rlcP->um_window_size) {
#ifdef DEBUG_RLC_UM_RX
            msg ("[RL^C_UM][MOD %d][RB %d][FRAME %05d] %d IN REORDERING WINDOW[%03d:%03d[ SN %d IN [%03d:%03d[ VR(UR)=%03d VR(UH)=%03d\n",
                 rlcP->module_id, rlcP->rb_id, mac_xface->frame, snP, 0, rlcP->um_window_size,
                                                                 sn, (signed int)rlcP->vr_uh - rlcP->um_window_size, rlcP->vr_uh,
                                                                 rlcP->vr_ur, rlcP->vr_uh);
#endif
            return 0;
        }
    }
#ifdef DEBUG_RLC_UM_RX
    if (modulus < 0) {
        msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] %d NOT IN REORDERING WINDOW[%03d:%03d[ SN %d NOT IN [%03d:%03d[ VR(UR)=%03d VR(UH)=%03d\n",
             rlcP->module_id, rlcP->rb_id, mac_xface->frame, snP, modulus + 1024, rlcP->um_window_size, sn, modulus + 1024 , rlcP->vr_uh, rlcP->vr_ur, rlcP->vr_uh);
    } else {
        msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] %d NOT IN REORDERING WINDOW[%03d:%03d[ SN %d NOT IN [%03d:%03d[ VR(UR)=%03d VR(UH)=%03d\n",
             rlcP->module_id, rlcP->rb_id, mac_xface->frame, snP, modulus, rlcP->um_window_size, sn, modulus , rlcP->vr_uh, rlcP->vr_ur, rlcP->vr_uh);
    }
#endif
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
    signed int sn = ((pduP->b1 & 0x00000003) << 8) + pduP->b2;
    signed int in_window = rlc_um_in_window(rlcP, rlcP->vr_uh - rlcP->um_window_size, sn, rlcP->vr_ur);

    if ((in_window == 1) || (in_window == 0)){
#ifdef DEBUG_RLC_UM_RX
        msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] RX PDU  VR(UH) – UM_Window_Size) <= SN %d < VR(UR) -> GARBAGE\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, sn);
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
        msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] RX PDU  VR(UR) < SN %d < VR(UH) and RECEIVED BEFORE-> GARBAGE\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, sn);
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
        msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] RX PDU  SN %d OUTSIDE REORDERING WINDOW VR(UH)=%d UM_Window_Size=%d\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, sn, rlcP->vr_uh, rlcP->um_window_size);
#endif
        rlcP->vr_uh = (sn + 1) % rlcP->sn_modulo;

        rlc_um_try_reassembly(rlcP, rlcP->vr_uh);

        if (rlc_um_in_reordering_window(rlcP, rlcP->vr_ur) < 0) {
            in_window = rlcP->vr_uh - rlcP->um_window_size;
            if (in_window < 0) {
                in_window = in_window + rlcP->sn_modulo;
            }
#ifdef DEBUG_RLC_UM_RX
            msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] VR(UR) %d OUTSIDE REORDERING WINDOW SET TO VR(UH) – UM_Window_Size = %d\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, rlcP->vr_ur, in_window);
#endif
            rlcP->vr_ur = in_window;
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
            rlcP->vr_ur = (rlcP->vr_ur+1) % rlcP->sn_modulo;
        } while (rlc_um_get_pdu_from_dar_buffer(rlcP, rlcP->vr_ur) && (rlcP->vr_ur != rlcP->vr_uh));
        rlc_um_try_reassembly(rlcP, rlcP->vr_ur);
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
#ifdef DEBUG_RLC_UM_RX
            msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] STOP and RESET t-Reordering because VR(UX) falls outside of the reordering window and VR(UX)=%d is not equal to VR(UH)=%d -or- VR(UX) <= VR(UR)\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame,rlcP->vr_ux,rlcP->vr_uh);
#endif
                rlcP->timer_reordering_running = 0;
                rlcP->timer_reordering         = 0;
            }
        }
    }
    if (rlcP->timer_reordering_running) {
        in_window = rlc_um_in_window(rlcP, rlcP->vr_ur,  rlcP->vr_ux,  rlcP->vr_uh);
        if ((in_window == -2) || (in_window == 1)) {
#ifdef DEBUG_RLC_UM_RX
            msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] STOP and RESET t-Reordering because VR(UX) falls outside of the reordering window and VR(UX)=%d is not equal to VR(UH)=%d\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame,rlcP->vr_ux,rlcP->vr_uh);
#endif
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
#ifdef DEBUG_RLC_UM_RX
            msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] RESTART t-Reordering set VR(UX) to VR(UH) =%d\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame,rlcP->vr_ux);
#endif
        }
    }
    rlc_um_check_timer_dar_time_out(rlcP);
}
