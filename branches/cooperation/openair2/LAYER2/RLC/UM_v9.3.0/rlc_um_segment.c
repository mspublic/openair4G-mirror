#define RLC_UM_MODULE
#define RLC_UM_SEGMENT_C
#ifdef USER_MODE
#include <assert.h>
#endif
#include "rtos_header.h"
#include "platform_types.h"
#include "list.h"
#include "rlc_um.h"
#include "rlc_primitives.h"
#include "MAC_INTERFACE/extern.h"

#define RLC_UM_SEGMENT
//-----------------------------------------------------------------------------
void
rlc_um_segment_10 (struct rlc_um_entity *rlcP)
{
//-----------------------------------------------------------------------------
    list_t              pdus;
    signed int          pdu_remaining_size;
    signed int          test_pdu_remaining_size;

    int                 nb_bytes_to_transmit = rlcP->nb_bytes_requested_by_mac;
    rlc_um_pdu_sn_10_t *pdu;
    struct mac_tb_req  *pdu_tb_req;
    mem_block_t        *pdu_mem;
    char               *data;
    char               *data_sdu;
    rlc_um_e_li_t      *e_li;
    struct rlc_um_tx_sdu_management *sdu_mngt;
    unsigned int       li_length_in_bytes;
    unsigned int       test_li_length_in_bytes;
    unsigned int       test_remaining_size_to_substract;
    unsigned int       test_remaining_num_li_to_substract;
    unsigned int       continue_fill_pdu_with_sdu;
    unsigned int       num_fill_sdu;
    unsigned int       test_num_li;
    unsigned int       fill_num_li;
    unsigned int       sdu_buffer_index;
    unsigned int       data_pdu_size;

    unsigned int       fi_first_byte_pdu_is_first_byte_sdu;
    unsigned int       fi_last_byte_pdu_is_last_byte_sdu;
    unsigned int       fi;
    unsigned int       max_li_overhead;

    msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] SEGMENT\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame);
    list_init (&pdus, NULL);    // param string identifying the list is NULL
    pdu_mem = NULL;

    while ((rlcP->input_sdus[rlcP->current_sdu_index]) && (nb_bytes_to_transmit > 0)) {
        msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] SEGMENT nb_bytes_to_transmit %d BO %d\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, nb_bytes_to_transmit, rlcP->buffer_occupancy);
        // pdu management
        if (!pdu_mem) {
            if (rlcP->nb_sdu <= 1) {
                max_li_overhead = 0;
            } else {
                max_li_overhead = (((rlcP->nb_sdu - 1) * 3) / 2) + ((rlcP->nb_sdu - 1) % 2);
            }
            if  (nb_bytes_to_transmit >= (rlcP->buffer_occupancy + rlcP->header_min_length_in_bytes + max_li_overhead)) {
                data_pdu_size = rlcP->buffer_occupancy + rlcP->header_min_length_in_bytes + max_li_overhead;
#ifdef RLC_UM_SEGMENT
                msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] SEGMENT alloc PDU size %d bytes to contain not all bytes requested by MAC but all BO of RLC@1\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, data_pdu_size);
#endif
            } else {
                data_pdu_size = nb_bytes_to_transmit;
#ifdef RLC_UM_SEGMENT
                msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] SEGMENT alloc PDU size %d bytes to contain all bytes requested by MAC@1\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, data_pdu_size);
#endif
            }
            if (!(pdu_mem = get_free_mem_block (data_pdu_size + sizeof(struct mac_tb_req)))) {
                msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] SEGMENT ERROR COULD NOT GET NEW PDU, EXIT\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame);
                return;
            }
#ifdef RLC_UM_SEGMENT
            msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] SEGMENT get new PDU %d bytes\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, data_pdu_size);
#endif
            pdu_remaining_size = data_pdu_size - 2;
            pdu        = (rlc_um_pdu_sn_10_t*) (&pdu_mem->data[sizeof(struct mac_tb_req)]);
            pdu_tb_req = (struct mac_tb_req*) (pdu_mem->data);

            memset (pdu_mem->data, 0, sizeof (rlc_um_pdu_sn_10_t)+sizeof(struct mac_tb_req));
            li_length_in_bytes = 1;
        }
        //----------------------------------------
        // compute how many SDUS can fill the PDU
        //----------------------------------------
        continue_fill_pdu_with_sdu = 1;
        num_fill_sdu               = 0;
        test_num_li                = 0;
        sdu_buffer_index           = rlcP->current_sdu_index;
        test_pdu_remaining_size    = pdu_remaining_size;
        test_li_length_in_bytes    = 1;
        test_remaining_size_to_substract   = 0;
        test_remaining_num_li_to_substract = 0;


        while ((rlcP->input_sdus[sdu_buffer_index]) && (continue_fill_pdu_with_sdu > 0)) {
            sdu_mngt = ((struct rlc_um_tx_sdu_management *) (rlcP->input_sdus[sdu_buffer_index]->data));

            if (sdu_mngt->sdu_remaining_size > test_pdu_remaining_size) {
                // no LI
                continue_fill_pdu_with_sdu = 0;
                num_fill_sdu += 1;
                test_pdu_remaining_size = 0;
                test_remaining_size_to_substract = 0;
                test_remaining_num_li_to_substract = 0;
            } else if (sdu_mngt->sdu_remaining_size == test_pdu_remaining_size) {
                // fi will indicate end of PDU is end of SDU, no need for LI
                continue_fill_pdu_with_sdu = 0;
                num_fill_sdu += 1;
                test_pdu_remaining_size = 0;
                test_remaining_size_to_substract = 0;
                test_remaining_num_li_to_substract = 0;
            } else if ((sdu_mngt->sdu_remaining_size + (test_li_length_in_bytes ^ 3)) == test_pdu_remaining_size ) {
                // no LI
                continue_fill_pdu_with_sdu = 0;
                num_fill_sdu += 1;
                test_pdu_remaining_size = 0;
                test_remaining_size_to_substract = 0;
                test_remaining_num_li_to_substract = 0;
                pdu_remaining_size = pdu_remaining_size - (test_li_length_in_bytes ^ 3);
            } else if ((sdu_mngt->sdu_remaining_size + (test_li_length_in_bytes ^ 3)) < test_pdu_remaining_size ) {
                test_num_li += 1;
                num_fill_sdu += 1;
                test_pdu_remaining_size = test_pdu_remaining_size - (sdu_mngt->sdu_remaining_size + (test_li_length_in_bytes ^ 3));
                test_remaining_size_to_substract = test_li_length_in_bytes ^ 3;
                test_remaining_num_li_to_substract = 1;
                test_li_length_in_bytes = test_li_length_in_bytes ^ 3;
            } else {
                msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] SEGMENT sdu_mngt->sdu_remaining_size=%d test_pdu_remaining_size=%d test_li_length_in_bytes=%d\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, sdu_mngt->sdu_remaining_size, test_pdu_remaining_size, test_li_length_in_bytes ^ 3);
                // reduce the size of the PDU
                continue_fill_pdu_with_sdu = 0;
                num_fill_sdu += 1;
                test_pdu_remaining_size = 0;
                test_remaining_size_to_substract = 0;
                test_remaining_num_li_to_substract = 0;
                pdu_remaining_size = pdu_remaining_size - 1;
            }
            sdu_buffer_index = (sdu_buffer_index + 1) % rlcP->size_input_sdus_buffer;
        }
        if (test_remaining_num_li_to_substract > 0) {
            // there is a LI that is not necessary
            test_num_li = test_num_li - 1;
            pdu_remaining_size = pdu_remaining_size - test_remaining_size_to_substract;
        }
        //----------------------------------------
        // Do the real filling of the pdu
        //----------------------------------------
        msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] data shift %d Bytes num_li %d\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, ((test_num_li*3) +1) >> 1, test_num_li);

        data = ((char*)(&pdu->data[((test_num_li*3) +1) >> 1]));
        e_li = (rlc_um_e_li_t*)(pdu->data);
        continue_fill_pdu_with_sdu          = 1;
        li_length_in_bytes                  = 1;
        fill_num_li                         = 0;
        fi_first_byte_pdu_is_first_byte_sdu = 0;
        fi_last_byte_pdu_is_last_byte_sdu   = 0;

        if (
            ((struct rlc_um_tx_sdu_management *) (rlcP->input_sdus[rlcP->current_sdu_index]->data))->sdu_remaining_size ==
            ((struct rlc_um_tx_sdu_management *) (rlcP->input_sdus[rlcP->current_sdu_index]->data))->sdu_size) {
            fi_first_byte_pdu_is_first_byte_sdu = 1;
        }
        while ((rlcP->input_sdus[rlcP->current_sdu_index]) && (continue_fill_pdu_with_sdu > 0)) {
            sdu_mngt = ((struct rlc_um_tx_sdu_management *) (rlcP->input_sdus[rlcP->current_sdu_index]->data));
#ifdef RLC_UM_SEGMENT
            if (sdu_mngt->sdu_segmented_size == 0) {
                msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] SEGMENT GET NEW SDU %p AVAILABLE SIZE %d Bytes\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, sdu_mngt, sdu_mngt->sdu_remaining_size);
            } else {
                msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] SEGMENT GET AGAIN SDU %p REMAINING AVAILABLE SIZE %d Bytes / %d Bytes \n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, sdu_mngt, sdu_mngt->sdu_remaining_size, sdu_mngt->sdu_size);
            }
#endif
            data_sdu = &((rlcP->input_sdus[rlcP->current_sdu_index])->data[sizeof (struct rlc_um_tx_sdu_management) + sdu_mngt->sdu_segmented_size]);

            if (sdu_mngt->sdu_remaining_size > pdu_remaining_size) {
#ifdef RLC_UM_SEGMENT
                msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] SEGMENT Filling all remaining PDU with %d bytes\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, pdu_remaining_size);
#endif
                msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] SEGMENT pdu_mem %p pdu %p pdu->data %p data %p data_sdu %p pdu_remaining_size %d\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, pdu_mem, pdu, pdu->data, data, data_sdu,pdu_remaining_size);

                memcpy(data, data_sdu, pdu_remaining_size);
                sdu_mngt->sdu_remaining_size = sdu_mngt->sdu_remaining_size - pdu_remaining_size;
                sdu_mngt->sdu_segmented_size = sdu_mngt->sdu_segmented_size + pdu_remaining_size;
                fi_last_byte_pdu_is_last_byte_sdu = 0;
                // no LI
                rlcP->buffer_occupancy -= pdu_remaining_size;
                continue_fill_pdu_with_sdu = 0;
                pdu_remaining_size = 0;
            } else if (sdu_mngt->sdu_remaining_size == pdu_remaining_size) {
#ifdef RLC_UM_SEGMENT
                msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] SEGMENT Exactly Filling remaining PDU with %d remaining bytes of SDU\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, pdu_remaining_size);
#endif
                memcpy(data, data_sdu, pdu_remaining_size);

                // free SDU
                rlcP->buffer_occupancy -= sdu_mngt->sdu_remaining_size;
                free_mem_block (rlcP->input_sdus[rlcP->current_sdu_index]);
                rlcP->input_sdus[rlcP->current_sdu_index] = NULL;
                rlcP->nb_sdu -= 1;
                rlcP->current_sdu_index = (rlcP->current_sdu_index + 1) % rlcP->size_input_sdus_buffer;

                fi_last_byte_pdu_is_last_byte_sdu = 1;
                // fi will indicate end of PDU is end of SDU, no need for LI
                continue_fill_pdu_with_sdu = 0;
                pdu_remaining_size = 0;
            } else if ((sdu_mngt->sdu_remaining_size + (li_length_in_bytes ^ 3)) < pdu_remaining_size ) {
#ifdef RLC_UM_SEGMENT
                msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] SEGMENT Filling  PDU with %d all remaining bytes of SDU\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, sdu_mngt->sdu_remaining_size);
#endif
                memcpy(data, data_sdu, sdu_mngt->sdu_remaining_size);
                data = &data[sdu_mngt->sdu_remaining_size];
                li_length_in_bytes = li_length_in_bytes ^ 3;
                fill_num_li += 1;
                if (li_length_in_bytes  == 2) {
                    if (fill_num_li == test_num_li) {
                        //e_li->e1  = 0;
                        e_li->b1 = 0;
                    } else {
                        //e_li->e1  = 1;
                        e_li->b1 =  0x80;
                    }
                    //e_li->li1 = sdu_mngt->sdu_remaining_size;
                    e_li->b1 = e_li->b1 | (sdu_mngt->sdu_remaining_size >> 4);
                    e_li->b2 = sdu_mngt->sdu_remaining_size << 4;
#ifdef RLC_UM_SEGMENT
                    msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] SEGMENT set e_li->b1=%02X set e_li->b2=%02X fill_num_li=%d test_num_li=%d\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, e_li->b1, e_li->b2, fill_num_li, test_num_li);
#endif
                } else {
                    if (fill_num_li != test_num_li) {
                        //e_li->e2  = 1;
                        e_li->b2  = e_li->b2 | 0x08;
                    }
                    //e_li->li2 = sdu_mngt->sdu_remaining_size;
                    e_li->b2 = e_li->b2 | (sdu_mngt->sdu_remaining_size >> 8);
                    e_li->b3 = sdu_mngt->sdu_remaining_size & 0xFF;
#ifdef RLC_UM_SEGMENT
                msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] SEGMENT set e_li->b2=%02X set e_li->b3=%02X fill_num_li=%d test_num_li=%d\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, e_li->b2, e_li->b3, fill_num_li, test_num_li);
#endif
                    e_li++;
                }

                // free SDU
                rlcP->buffer_occupancy -= sdu_mngt->sdu_remaining_size;
                free_mem_block (rlcP->input_sdus[rlcP->current_sdu_index]);
                rlcP->input_sdus[rlcP->current_sdu_index] = NULL;
                rlcP->nb_sdu -= 1;
                rlcP->current_sdu_index = (rlcP->current_sdu_index + 1) % rlcP->size_input_sdus_buffer;

                pdu_remaining_size = pdu_remaining_size - (sdu_mngt->sdu_remaining_size + li_length_in_bytes);
            } else {
#ifdef RLC_UM_SEGMENT
                msg ("[RLC_UM][MOD %d][RB %d][FRAME %05d] SEGMENT Filling  PDU with %d all remaining bytes of SDU and reduce TB size by %d bytes\n", rlcP->module_id, rlcP->rb_id, mac_xface->frame, sdu_mngt->sdu_remaining_size, pdu_remaining_size - sdu_mngt->sdu_remaining_size);
#endif
#ifdef USER_MODE
                assert(1!=1);
#endif
                memcpy(data, data_sdu, sdu_mngt->sdu_remaining_size);
                // free SDU
                rlcP->buffer_occupancy -= sdu_mngt->sdu_remaining_size;
                free_mem_block (rlcP->input_sdus[rlcP->current_sdu_index]);
                rlcP->input_sdus[rlcP->current_sdu_index] = NULL;
                rlcP->nb_sdu -= 1;
                rlcP->current_sdu_index = (rlcP->current_sdu_index + 1) % rlcP->size_input_sdus_buffer;

                // reduce the size of the PDU
                continue_fill_pdu_with_sdu = 0;
                fi_last_byte_pdu_is_last_byte_sdu = 1;
                pdu_remaining_size = pdu_remaining_size - sdu_mngt->sdu_remaining_size;
            }
        }

        // set framing info
        if (fi_first_byte_pdu_is_first_byte_sdu) {
            fi = 0;
        } else {
            fi = 2;
        }
        if (!fi_last_byte_pdu_is_last_byte_sdu) {
            fi = fi + 1;
        }
        //pdu->b1 = pdu->b1 | (fi << 3);
        pdu->b1 = (fi << 3);

        // set fist e bit
        if (fill_num_li > 0) {
            pdu->b1 = pdu->b1 | 0x04;
        }
        //pdu->sn = rlcP->vt_us;
        pdu->b1 = pdu->b1 | ((rlcP->vt_us >> 8) & 0x03);
        pdu->b2 = rlcP->vt_us & 0xFF;
        rlcP->vt_us = rlcP->vt_us+1;

        pdu_tb_req->data_ptr        = (unsigned char*)pdu;
        pdu_tb_req->tb_size_in_bits = (data_pdu_size - pdu_remaining_size) << 3;
        list_add_tail_eurecom (pdu_mem, &rlcP->pdus_to_mac_layer);
        pdu = NULL;
        pdu_mem = NULL;

        //nb_bytes_to_transmit = nb_bytes_to_transmit - data_pdu_size;
        nb_bytes_to_transmit = 0; // 1 PDU only
    }
}
