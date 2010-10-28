#define RLC_UM_MODULE
#define RLC_UM_SEGMENT_C
#include "rtos_header.h"
#include "platform_types.h"
#include "list.h"
#include "rlc_um.h"
#include "rlc_primitives.h"

#define RLC_UM_SEGMENT
//-----------------------------------------------------------------------------
void
rlc_um_segment_10 (struct rlc_um_entity *rlcP)
{
//-----------------------------------------------------------------------------
    list_t              pdus;
    signed int          pdu_remaining_size;
    signed int          test_pdu_remaining_size;

    //int                 nb_pdu_to_transmit = rlcP->nb_pdu_requested_by_mac;
    int                 nb_bytes_to_transmit = rlcP->nb_bytes_requested_by_mac;
    rlc_um_pdu_sn_10_t *pdu;
    struct mac_tb_req  *pdu_tb_req;
    mem_block_t        *pdu_mem;
    char               *data;
    char               *data_sdu;
    rlc_um_e_li_t      *e_li;
    struct rlc_um_tx_sdu_management *sdu_mngt;
    unsigned int       li_length_in_bytes;
    unsigned int       continue_fill_pdu_with_sdu;
    unsigned int       num_fill_sdu;
    unsigned int       test_num_li;
    unsigned int       fill_num_li;
    unsigned int       sdu_buffer_index;
    unsigned int       data_pdu_size;

    unsigned int       fi_first_byte_pdu_is_first_byte_sdu;
    unsigned int       fi_last_byte_pdu_is_last_byte_sdu;
    unsigned int       fi;

    msg ("[RLC_UM][MOD %d][RB %d] SEGMENT\n", rlcP->module_id, rlcP->rb_id);
    list_init (&pdus, NULL);    // param string identifying the list is NULL
    pdu_mem = NULL;

    while ((rlcP->input_sdus[rlcP->current_sdu_index]) && (nb_bytes_to_transmit > 0)) {
        msg ("[RLC_UM][MOD %d][RB %d] SEGMENT nb_bytes_to_transmit %d BO %d\n", rlcP->module_id, rlcP->rb_id, nb_bytes_to_transmit, rlcP->buffer_occupancy);
        // pdu management
        if (!pdu_mem) {
            if (rlcP->data_pdu_size > 0) {
                // PDU fixed size
                data_pdu_size = rlcP->data_pdu_size;
#ifdef RLC_UM_SEGMENT
                msg ("[RLC_UM][MOD %d][RB %d] SEGMENT fixed PDU size %d bytes\n", rlcP->module_id, rlcP->rb_id, data_pdu_size);
#endif
            } else {
                if ((rlcP->input_sdus[rlcP->current_sdu_index])) {
                    sdu_mngt = ((struct rlc_um_tx_sdu_management *) (rlcP->input_sdus[rlcP->current_sdu_index]->data));
                    // try to make one PDU contain exactly one SDU
                    if (sdu_mngt->sdu_remaining_size == sdu_mngt->sdu_size) {
                        if  (nb_bytes_to_transmit >= (sdu_mngt->sdu_size + rlcP->header_min_length_in_bytes)) {
                            data_pdu_size = sdu_mngt->sdu_size + rlcP->header_min_length_in_bytes;
#ifdef RLC_UM_SEGMENT
                        msg ("[RLC_UM][MOD %d][RB %d] SEGMENT alloc PDU size %d bytes to contain only 1 complete SDU\n", rlcP->module_id, rlcP->rb_id, data_pdu_size);
#endif
                        } else {
                            data_pdu_size = nb_bytes_to_transmit;
#ifdef RLC_UM_SEGMENT
                            msg ("[RLC_UM][MOD %d][RB %d] SEGMENT alloc PDU size %d bytes to contain all bytes requested by MAC\n", rlcP->module_id, rlcP->rb_id, data_pdu_size);
#endif
                        }
                    } else {
                        if  (nb_bytes_to_transmit >= (rlcP->buffer_occupancy + rlcP->header_min_length_in_bytes)) {
                            data_pdu_size = rlcP->buffer_occupancy + rlcP->header_min_length_in_bytes;
#ifdef RLC_UM_SEGMENT
                            msg ("[RLC_UM][MOD %d][RB %d] SEGMENT alloc PDU size %d bytes to contain not all bytes requested by MAC but all BO of RLC@0\n", rlcP->module_id, rlcP->rb_id, data_pdu_size);
#endif
                        } else {
                            data_pdu_size = nb_bytes_to_transmit;
#ifdef RLC_UM_SEGMENT
                            msg ("[RLC_UM][MOD %d][RB %d] SEGMENT alloc PDU size %d bytes to contain all bytes requested by MAC@0\n", rlcP->module_id, rlcP->rb_id, data_pdu_size);
#endif
                        }
                    }
                } else {
                    if  (nb_bytes_to_transmit >= (rlcP->buffer_occupancy + rlcP->header_min_length_in_bytes)) {
                        data_pdu_size = rlcP->buffer_occupancy + rlcP->header_min_length_in_bytes;
#ifdef RLC_UM_SEGMENT
                        msg ("[RLC_UM][MOD %d][RB %d] SEGMENT alloc PDU size %d bytes to contain not all bytes requested by MAC but all BO of RLC@1\n", rlcP->module_id, rlcP->rb_id, data_pdu_size);
#endif
                    } else {
                        data_pdu_size = nb_bytes_to_transmit;
#ifdef RLC_UM_SEGMENT
                        msg ("[RLC_UM][MOD %d][RB %d] SEGMENT alloc PDU size %d bytes to contain all bytes requested by MAC@1\n", rlcP->module_id, rlcP->rb_id, data_pdu_size);
#endif
                    }
                }
            }
            if (!(pdu_mem = get_free_mem_block (data_pdu_size + sizeof(struct mac_tb_req)))) {
                msg ("[RLC_UM][MOD %d][RB %d] SEGMENT ERROR COULD NOT GET NEW PDU, EXIT\n", rlcP->module_id, rlcP->rb_id);
                return;
            }
#ifdef RLC_UM_SEGMENT
            msg ("[RLC_UM][MOD %d][RB %d] SEGMENT get new PDU %d bytes\n", rlcP->module_id, rlcP->rb_id, data_pdu_size);
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

        while ((rlcP->input_sdus[sdu_buffer_index]) && (continue_fill_pdu_with_sdu > 0)) {
            sdu_mngt = ((struct rlc_um_tx_sdu_management *) (rlcP->input_sdus[sdu_buffer_index]->data));

            if (sdu_mngt->sdu_remaining_size > test_pdu_remaining_size) {
                // no LI
                continue_fill_pdu_with_sdu = 0;
                num_fill_sdu += 1;
                test_pdu_remaining_size = 0;
            } else if (sdu_mngt->sdu_remaining_size == test_pdu_remaining_size) {
                // fi will indicate end of PDU is end of SDU, no need for LI
                continue_fill_pdu_with_sdu = 0;
                num_fill_sdu += 1;
                test_pdu_remaining_size = 0;
            } else if (sdu_mngt->sdu_remaining_size <= (test_pdu_remaining_size + (li_length_in_bytes ^ 3))) {
                test_num_li += 1;
                num_fill_sdu += 1;
                test_pdu_remaining_size = test_pdu_remaining_size - (sdu_mngt->sdu_remaining_size + (li_length_in_bytes ^ 3));
            } else {
                // reduce the size of the PDU
                continue_fill_pdu_with_sdu = 0;
                test_pdu_remaining_size = test_pdu_remaining_size - sdu_mngt->sdu_remaining_size;
                pdu_remaining_size = pdu_remaining_size - test_pdu_remaining_size;
            }
            sdu_buffer_index = (sdu_buffer_index + 1) % rlcP->size_input_sdus_buffer;

        }
        //----------------------------------------
        // Do the real filling of the pdu
        //----------------------------------------
        msg ("[RLC_UM][MOD %d][RB %d] data shift %d Bytes num_li %d\n", rlcP->module_id, rlcP->rb_id, test_num_li + ((test_num_li+1) >> 1), test_num_li);

        data = ((char*)(&pdu->data[test_num_li + ((test_num_li+1) >> 1)]));
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
                msg ("[RLC_UM][MOD %d][RB %d] SEGMENT GET NEW SDU %p AVAILABLE SIZE %d Bytes\n", rlcP->module_id, rlcP->rb_id, sdu_mngt, sdu_mngt->sdu_remaining_size);
            } else {
                msg ("[RLC_UM][MOD %d][RB %d] SEGMENT GET AGAIN SDU %p REMAINING AVAILABLE SIZE %d Bytes / %d Bytes \n", rlcP->module_id, rlcP->rb_id, sdu_mngt, sdu_mngt->sdu_remaining_size, sdu_mngt->sdu_size);
            }
#endif
            data_sdu = &((rlcP->input_sdus[rlcP->current_sdu_index])->data[sizeof (struct rlc_um_tx_sdu_management) + sdu_mngt->sdu_segmented_size]);

            if (sdu_mngt->sdu_remaining_size > pdu_remaining_size) {
#ifdef RLC_UM_SEGMENT
                msg ("[RLC_UM][MOD %d][RB %d] SEGMENT Filling all remaining PDU with %d bytes\n", rlcP->module_id, rlcP->rb_id, pdu_remaining_size);
#endif
                msg ("[RLC_UM][MOD %d][RB %d] SEGMENT pdu_mem %p pdu %p pdu->data %p data %p data_sdu %p pdu_remaining_size %d\n", rlcP->module_id, rlcP->rb_id, pdu_mem, pdu, pdu->data, data, data_sdu,pdu_remaining_size);

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
                msg ("[RLC_UM][MOD %d][RB %d] SEGMENT Exactly Filling remaining PDU with %d remaining bytes of SDU\n", rlcP->module_id, rlcP->rb_id, pdu_remaining_size);
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
            } else if (sdu_mngt->sdu_remaining_size <= (pdu_remaining_size + (li_length_in_bytes ^ 3))) {
#ifdef RLC_UM_SEGMENT
                msg ("[RLC_UM][MOD %d][RB %d] SEGMENT Filling  PDU with %d all remaining bytes of SDU\n", rlcP->module_id, rlcP->rb_id, sdu_mngt->sdu_remaining_size);
#endif
                memcpy(data, data_sdu, sdu_mngt->sdu_remaining_size);
                data = &data[sdu_mngt->sdu_remaining_size];
                li_length_in_bytes = li_length_in_bytes ^ 3;
                fill_num_li += 1;
                if (li_length_in_bytes  == 2) {
                    if (fill_num_li == test_num_li) {
                        e_li->e1  = 0;
                    } else {
                        e_li->e1  = 1;
                    }
                    e_li->li1 = sdu_mngt->sdu_remaining_size;
                } else {
                    if (fill_num_li == test_num_li) {
                        e_li->e2  = 0;
                    } else {
                        e_li->e2  = 1;
                    }
                    e_li->li2 = sdu_mngt->sdu_remaining_size;
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
                msg ("[RLC_UM][MOD %d][RB %d] SEGMENT Filling  PDU with %d all remaining bytes of SDU and reduce TB size by %d bytes\n", rlcP->module_id, rlcP->rb_id, sdu_mngt->sdu_remaining_size, pdu_remaining_size - sdu_mngt->sdu_remaining_size);
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
        pdu->fi = fi;

        // set fist e bit
        if (fill_num_li > 0) {
            pdu->e = 1;
        }

        pdu_tb_req->data_ptr        = (unsigned char*)pdu;
        pdu_tb_req->tb_size_in_bits = (data_pdu_size - pdu_remaining_size) << 3;
        list_add_tail_eurecom (pdu_mem, &rlcP->pdus_to_mac_layer);
        pdu = NULL;
        pdu_mem = NULL;

        nb_bytes_to_transmit = nb_bytes_to_transmit - data_pdu_size;
    }
}
