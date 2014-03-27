/*******************************************************************************
Eurecom OpenAirInterface 2
Copyright(c) 1999 - 2014 Eurecom

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
Address      : EURECOM,
               Campus SophiaTech,
               450 Route des Chappes,
               CS 50193
               06904 Biot Sophia Antipolis cedex,
               FRANCE
*******************************************************************************/
#define RLC_AM_MODULE
#define RLC_AM_SEGMENT_C
//-----------------------------------------------------------------------------
#ifdef USER_MODE
#include <assert.h>
#endif
//-----------------------------------------------------------------------------
//#include "rtos_header.h"
#include "platform_types.h"
//-----------------------------------------------------------------------------
#include "list.h"
#include "rlc_am.h"
#include "LAYER2/MAC/extern.h"
#include "UTIL/LOG/log.h"

//-----------------------------------------------------------------------------
void rlc_am_pdu_polling (
        rlc_am_entity_t *const rlc_pP,
        const frame_t frameP,
        rlc_am_pdu_sn_10_t *const pdu_pP,
        const int16_t payload_sizeP)
//-----------------------------------------------------------------------------
{
    // 5.2.2 Polling
    // An AM RLC entity can poll its peer AM RLC entity in order to trigger STATUS reporting at the peer AM RLC entity.
    // 5.2.2.1 Transmission of a AMD PDU or AMD PDU segment
    // Upon assembly of a new AMD PDU, the transmitting side of an AM RLC entity shall:
    //     - increment PDU_WITHOUT_POLL by one;
    //     - increment BYTE_WITHOUT_POLL by every new byte of Data field element that it maps to the Data field of
    //       the RLC data PDU;
    //     - if PDU_WITHOUT_POLL >= pollPDU; or
    //     - if BYTE_WITHOUT_POLL >= pollByte;
    //         -include a poll in the RLC data PDU as described below.
    // Upon assembly of an AMD PDU or AMD PDU segment, the transmitting side of an AM RLC entity shall:
    //     - if both the transmission buffer and the retransmission buffer becomes empty (excluding transmitted RLC data
    //       PDU awaiting for acknowledgements) after the transmission of the RLC data PDU; or
    //     - if no new RLC data PDU can be transmitted after the transmission of the RLC data PDU (e.g. due to window
    //       stalling);
    //         - include a poll in the RLC data PDU as described below.
    // To include a poll in a RLC data PDU, the transmitting side of an AM RLC entity shall:
    //     - set the P field of the RLC data PDU to "1";
    //     - set PDU_WITHOUT_POLL to 0;
    //     - set BYTE_WITHOUT_POLL to 0;
    // After delivering a RLC data PDU including a poll to lower layer and after incrementing of VT(S) if necessary, the
    // transmitting side of an AM RLC entity shall:
    //     - set POLL_SN to VT(S) – 1;
    //     - if t-PollRetransmit is not running:
    //         - start t-PollRetransmit;
    //     - else:
    //         - restart t-PollRetransmit;
    rlc_pP->c_pdu_without_poll     += 1;
    rlc_pP->c_byte_without_poll    += payload_sizeP;

    if (
        (rlc_pP->c_pdu_without_poll >= rlc_pP->poll_pdu) ||
        (rlc_pP->c_byte_without_poll >= rlc_pP->poll_byte) ||
        ((rlc_pP->sdu_buffer_occupancy == 0) && (rlc_pP->retrans_num_bytes_to_retransmit == 0)) ||
        (rlc_pP->vt_s == rlc_pP->vt_ms)
        ) {

        if (rlc_pP->c_pdu_without_poll >= rlc_pP->poll_pdu) {
            LOG_T(RLC, "[FRAME %05d][%s][RLC_AM][MOD %u/%u][RB %u][POLL] SET POLL BECAUSE TX NUM PDU THRESHOLD %d  HAS BEEN REACHED\n",
                  frameP,
                  (rlc_pP->is_enb) ? "eNB" : "UE",
                  rlc_pP->enb_module_id,
                  rlc_pP->ue_module_id,
                  rlc_pP->rb_id,
                  rlc_pP->poll_pdu);
        }
        if (rlc_pP->c_pdu_without_poll >= rlc_pP->poll_pdu) {
            LOG_T(RLC, "[FRAME %05d][%s][RLC_AM][MOD %u/%u][RB %u][POLL] SET POLL BECAUSE TX NUM BYTES THRESHOLD %d  HAS BEEN REACHED\n",
                  frameP,
                  (rlc_pP->is_enb) ? "eNB" : "UE",
                  rlc_pP->enb_module_id,
                  rlc_pP->ue_module_id,
                  rlc_pP->rb_id,
                  rlc_pP->poll_byte);
        }
        if ((rlc_pP->sdu_buffer_occupancy == 0) && (rlc_pP->retrans_num_bytes_to_retransmit == 0)) {
            LOG_T(RLC, "[FRAME %05d][%s][RLC_AM][MOD %u/%u][RB %u][POLL] SET POLL BECAUSE TX BUFFERS ARE EMPTY\n",
                  frameP,
                  (rlc_pP->is_enb) ? "eNB" : "UE",
                  rlc_pP->enb_module_id,
                  rlc_pP->ue_module_id,
                  rlc_pP->rb_id);
        }
        if (rlc_pP->vt_s == rlc_pP->vt_ms) {
            LOG_T(RLC, "[FRAME %05d][%s][RLC_AM][MOD %u/%u][RB %u][POLL] SET POLL BECAUSE OF WINDOW STALLING\n",
                  frameP,
                  (rlc_pP->is_enb) ? "eNB" : "UE",
                  rlc_pP->enb_module_id,
                  rlc_pP->ue_module_id,
                  rlc_pP->rb_id);
        }
        pdu_pP->b1 = pdu_pP->b1 | 0x20;
        rlc_pP->c_pdu_without_poll     = 0;
        rlc_pP->c_byte_without_poll    = 0;

        rlc_pP->poll_sn = (rlc_pP->vt_s -1) & RLC_AM_SN_MASK;
        //optimisation if (!rlc_pP->t_poll_retransmit.running) {
        rlc_am_start_timer_poll_retransmit(rlc_pP,frameP);
        //optimisation } else {
        //optimisation     rlc_pP->t_poll_retransmit.frame_time_out = frameP + rlc_pP->t_poll_retransmit.time_out;
        //optimisation }
    } else {
        pdu_pP->b1 = pdu_pP->b1 & 0xDF;
    }
}
//-----------------------------------------------------------------------------
void rlc_am_segment_10 (
        rlc_am_entity_t *const rlc_pP,
        const frame_t frameP)
{
//-----------------------------------------------------------------------------
    list_t              pdus;
    sdu_size_t          pdu_remaining_size      = 0;
    sdu_size_t          test_pdu_remaining_size = 0;

    sdu_size_t                       nb_bytes_to_transmit = rlc_pP->nb_bytes_requested_by_mac;
    rlc_am_pdu_sn_10_t              *pdu_p        = NULL;
    struct mac_tb_req               *pdu_tb_req_p = NULL;
    mem_block_t                     *pdu_mem_p    = NULL;
    unsigned char                   *data         = NULL;
    unsigned char                   *data_sdu_p   = NULL;
    rlc_am_e_li_t                   *e_li_p       = NULL;
    rlc_am_tx_sdu_management_t      *sdu_mngt_p   = NULL;
    rlc_am_tx_data_pdu_management_t *pdu_mngt_p   = NULL;

    sdu_size_t         li_length_in_bytes         = 0;
    sdu_size_t         test_li_length_in_bytes    = 0;
    sdu_size_t         test_remaining_size_to_substract= 0;
    unsigned int       test_remaining_num_li_to_substract = 0;
    unsigned int       continue_fill_pdu_with_sdu         = 0;
    unsigned int       num_fill_sdu                       = 0;
    unsigned int       test_num_li                        = 0;
    unsigned int       fill_num_li                        = 0;
    unsigned int       sdu_buffer_index                   = 0;
    sdu_size_t         data_pdu_size                      = 0;

    unsigned int       fi_first_byte_pdu_is_first_byte_sdu = 0;
    unsigned int       fi_last_byte_pdu_is_last_byte_sdu   = 0;
    unsigned int       fi                                  = 0;
    signed int         max_li_overhead                     = 0;

    LOG_T(RLC, "[FRAME %05d][%s][RLC_AM][MOD %u/%u][RB %u][SEGMENT] rlc_pP->current_sdu_index %d rlc_pP->next_sdu_index %d rlc_pP->input_sdus[rlc_pP->current_sdu_index].mem_block %p sdu_buffer_occupancy %d\n",
          frameP,
          (rlc_pP->is_enb) ? "eNB" : "UE",
          rlc_pP->enb_module_id,
          rlc_pP->ue_module_id,
          rlc_pP->rb_id,
          rlc_pP->current_sdu_index,
          rlc_pP->next_sdu_index,
          rlc_pP->input_sdus[rlc_pP->current_sdu_index].mem_block,
          rlc_pP->sdu_buffer_occupancy);
    if (rlc_pP->sdu_buffer_occupancy <= 0) {
        return;
    }

    //msg ("[FRAME %05d][%s][RLC_AM][MOD %u/%u][RB %u][SEGMENT]\n", rlc_pP->module_id, rlc_pP->rb_id, frameP);
    list_init (&pdus, NULL);    // param string identifying the list is NULL
    pdu_mem_p = NULL;


    while ((rlc_pP->input_sdus[rlc_pP->current_sdu_index].mem_block) && (nb_bytes_to_transmit > 0) ) {
        LOG_T(RLC, "[FRAME %05d][%s][RLC_AM][MOD %u/%u][RB %u][SEGMENT] nb_bytes_to_transmit %d BO %d\n",
              frameP,
              (rlc_pP->is_enb) ? "eNB" : "UE",
              rlc_pP->enb_module_id,
              rlc_pP->ue_module_id,
              rlc_pP->rb_id,
              nb_bytes_to_transmit,
              rlc_pP->sdu_buffer_occupancy);
        // pdu_p management
        if (!pdu_mem_p) {
            if (rlc_pP->nb_sdu_no_segmented <= 1) {
                max_li_overhead = 0;
            } else {
                max_li_overhead = (((rlc_pP->nb_sdu_no_segmented - 1) * 3) / 2) + ((rlc_pP->nb_sdu_no_segmented - 1) % 2);
            }
            LOG_T(RLC, "[FRAME %05d][%s][RLC_AM][MOD %u/%u][RB %u][SEGMENT] max_li_overhead %d\n",
                  frameP,
                  (rlc_pP->is_enb) ? "eNB" : "UE",
                  rlc_pP->enb_module_id,
                  rlc_pP->ue_module_id,
                  rlc_pP->rb_id,
                  max_li_overhead);
            if  (nb_bytes_to_transmit >= (rlc_pP->sdu_buffer_occupancy + RLC_AM_HEADER_MIN_SIZE + max_li_overhead)) {
                data_pdu_size = rlc_pP->sdu_buffer_occupancy + RLC_AM_HEADER_MIN_SIZE + max_li_overhead;
                LOG_T(RLC, "[FRAME %05d][%s][RLC_AM][MOD %u/%u][RB %u][SEGMENT] alloc PDU size %d bytes to contain not all bytes requested by MAC but all BO of RLC@1\n",
                      frameP,
                      (rlc_pP->is_enb) ? "eNB" : "UE",
                      rlc_pP->enb_module_id,
                      rlc_pP->ue_module_id,
                      rlc_pP->rb_id,
                      data_pdu_size);
            } else {
                data_pdu_size = nb_bytes_to_transmit;
               LOG_T(RLC, "[FRAME %05d][%s][RLC_AM][MOD %u/%u][RB %u][SEGMENT] alloc PDU size %d bytes to contain all bytes requested by MAC@1\n",
                     frameP,
                     (rlc_pP->is_enb) ? "eNB" : "UE",
                     rlc_pP->enb_module_id,
                     rlc_pP->ue_module_id,
                     rlc_pP->rb_id,
                     data_pdu_size);
            }
            if (!(pdu_mem_p = get_free_mem_block (data_pdu_size + sizeof(struct mac_tb_req)))) {
                LOG_C(RLC, "[FRAME %05d][%s][RLC_AM][MOD %u/%u][RB %u][SEGMENT] ERROR COULD NOT GET NEW PDU, EXIT\n",
                frameP,
                (rlc_pP->is_enb) ? "eNB" : "UE",
                rlc_pP->enb_module_id,
                rlc_pP->ue_module_id,
                rlc_pP->rb_id);
                return;
            }
            LOG_T(RLC, "[FRAME %05d][%s][RLC_AM][MOD %u/%u][RB %u][SEGMENT] get new PDU %d bytes\n",
                  frameP,
                  (rlc_pP->is_enb) ? "eNB" : "UE",
                  rlc_pP->enb_module_id,
                  rlc_pP->ue_module_id,
                  rlc_pP->rb_id,
                  data_pdu_size);
            pdu_remaining_size = data_pdu_size - RLC_AM_HEADER_MIN_SIZE;
            pdu_p        = (rlc_am_pdu_sn_10_t*) (&pdu_mem_p->data[sizeof(struct mac_tb_req)]);
            pdu_tb_req_p = (struct mac_tb_req*) (pdu_mem_p->data);
            pdu_mngt_p   = &rlc_pP->pdu_retrans_buffer[rlc_pP->vt_s % RLC_AM_PDU_RETRANSMISSION_BUFFER_SIZE];
            memset(pdu_mngt_p, 0, sizeof (rlc_am_tx_data_pdu_management_t));

            memset (pdu_mem_p->data, 0, sizeof (rlc_am_pdu_sn_10_t)+sizeof(struct mac_tb_req));
            li_length_in_bytes = 1;
        }
        //----------------------------------------
        // compute how many SDUS can fill the PDU
        //----------------------------------------
        continue_fill_pdu_with_sdu = 1;
        num_fill_sdu               = 0;
        test_num_li                = 0;
        sdu_buffer_index           = rlc_pP->current_sdu_index;
        test_pdu_remaining_size    = pdu_remaining_size;
        test_li_length_in_bytes    = 1;
        test_remaining_size_to_substract   = 0;
        test_remaining_num_li_to_substract = 0;


        while ((rlc_pP->input_sdus[sdu_buffer_index].mem_block) && (continue_fill_pdu_with_sdu > 0)) {
            sdu_mngt_p = &rlc_pP->input_sdus[sdu_buffer_index];

            if (sdu_mngt_p->sdu_remaining_size > test_pdu_remaining_size) {
                // no LI
                continue_fill_pdu_with_sdu = 0;
                num_fill_sdu += 1;
                test_pdu_remaining_size = 0;
                test_remaining_size_to_substract = 0;
                test_remaining_num_li_to_substract = 0;
            } else if (sdu_mngt_p->sdu_remaining_size == test_pdu_remaining_size) {
                // fi will indicate end of PDU is end of SDU, no need for LI
                continue_fill_pdu_with_sdu = 0;
                num_fill_sdu += 1;
                test_pdu_remaining_size = 0;
                test_remaining_size_to_substract = 0;
                test_remaining_num_li_to_substract = 0;
            } else if ((sdu_mngt_p->sdu_remaining_size + (test_li_length_in_bytes ^ 3)) == test_pdu_remaining_size ) {
                // no LI
                continue_fill_pdu_with_sdu = 0;
                num_fill_sdu += 1;
                test_pdu_remaining_size = 0;
                test_remaining_size_to_substract = 0;
                test_remaining_num_li_to_substract = 0;
                pdu_remaining_size = pdu_remaining_size - (test_li_length_in_bytes ^ 3);
            } else if ((sdu_mngt_p->sdu_remaining_size + (test_li_length_in_bytes ^ 3)) < test_pdu_remaining_size ) {
                if (pdu_mngt_p->nb_sdus >= (RLC_AM_MAX_SDU_IN_PDU-1)) {
                    continue_fill_pdu_with_sdu = 0;
                    //num_fill_sdu += 1;
                    test_pdu_remaining_size = 0;
                    test_remaining_size_to_substract = 0;
                    test_remaining_num_li_to_substract = 0;
                    pdu_remaining_size = pdu_remaining_size - 1;
                } else {
                    test_num_li += 1;
                    num_fill_sdu += 1;
                    test_pdu_remaining_size = test_pdu_remaining_size - (sdu_mngt_p->sdu_remaining_size + (test_li_length_in_bytes ^ 3));
                    test_remaining_size_to_substract = test_li_length_in_bytes ^ 3;
                    test_remaining_num_li_to_substract = 1;
                    test_li_length_in_bytes = test_li_length_in_bytes ^ 3;
                }
            } else {
                LOG_T(RLC, "[FRAME %05d][%s][RLC_AM][MOD %u/%u][RB %u][SEGMENT] sdu_mngt_p->sdu_remaining_size=%d test_pdu_remaining_size=%d test_li_length_in_bytes=%d\n",
                      frameP,
                      (rlc_pP->is_enb) ? "eNB" : "UE",
                      rlc_pP->enb_module_id,
                      rlc_pP->ue_module_id,
                      rlc_pP->rb_id,
                      sdu_mngt_p->sdu_remaining_size,
                      test_pdu_remaining_size,
                      test_li_length_in_bytes ^ 3);
                // reduce the size of the PDU
                continue_fill_pdu_with_sdu = 0;
                num_fill_sdu += 1;
                test_pdu_remaining_size = 0;
                test_remaining_size_to_substract = 0;
                test_remaining_num_li_to_substract = 0;
                pdu_remaining_size = pdu_remaining_size - 1;
            }
            pdu_mngt_p->sdus_index[pdu_mngt_p->nb_sdus++] = sdu_buffer_index;
            sdu_mngt_p->pdus_index[sdu_mngt_p->nb_pdus++] = rlc_pP->vt_s % RLC_AM_PDU_RETRANSMISSION_BUFFER_SIZE;
            assert(sdu_mngt_p->nb_pdus < RLC_AM_MAX_SDU_FRAGMENTS);
            sdu_buffer_index = (sdu_buffer_index + 1) % RLC_AM_SDU_CONTROL_BUFFER_SIZE;
        }
        if (test_remaining_num_li_to_substract > 0) {
            // there is a LI that is not necessary
            test_num_li = test_num_li - 1;
            pdu_remaining_size = pdu_remaining_size - test_remaining_size_to_substract;
        }
        //----------------------------------------
        // Do the real filling of the pdu_p
        //----------------------------------------
        LOG_T(RLC, "[FRAME %05d][%s][RLC_AM][MOD %u/%u][RB %u] data shift %d Bytes num_li %d\n",
              frameP,
              (rlc_pP->is_enb) ? "eNB" : "UE",
              rlc_pP->enb_module_id,
              rlc_pP->ue_module_id,
              rlc_pP->rb_id,
              ((test_num_li*3) +1) >> 1,
              test_num_li);
        data = ((unsigned char*)(&pdu_p->data[((test_num_li*3) +1) >> 1]));
        pdu_mngt_p->payload = data;
        e_li_p = (rlc_am_e_li_t*)(pdu_p->data);
        continue_fill_pdu_with_sdu          = 1;
        li_length_in_bytes                  = 1;
        fill_num_li                         = 0;
        fi_first_byte_pdu_is_first_byte_sdu = 0;
        fi_last_byte_pdu_is_last_byte_sdu   = 0;

        if (rlc_pP->input_sdus[rlc_pP->current_sdu_index].sdu_remaining_size ==
            rlc_pP->input_sdus[rlc_pP->current_sdu_index].sdu_size) {
            fi_first_byte_pdu_is_first_byte_sdu = 1;
        }
        while ((rlc_pP->input_sdus[rlc_pP->current_sdu_index].mem_block) && (continue_fill_pdu_with_sdu > 0)) {
            sdu_mngt_p = &rlc_pP->input_sdus[rlc_pP->current_sdu_index];
            if (sdu_mngt_p->sdu_segmented_size == 0) {
                LOG_T(RLC, "[FRAME %05d][%s][RLC_AM][MOD %u/%u][RB %u][SEGMENT] GET NEW SDU %p AVAILABLE SIZE %d Bytes\n",
                      frameP,
                      (rlc_pP->is_enb) ? "eNB" : "UE",
                      rlc_pP->enb_module_id,
                      rlc_pP->ue_module_id,
                      rlc_pP->rb_id,
                      sdu_mngt_p,
                      sdu_mngt_p->sdu_remaining_size);
            } else {
                LOG_T(RLC, "[FRAME %05d][%s][RLC_AM][MOD %u/%u][RB %u][SEGMENT] GET AGAIN SDU %p REMAINING AVAILABLE SIZE %d Bytes / %d Bytes LENGTH \n",
                      frameP,
                      (rlc_pP->is_enb) ? "eNB" : "UE",
                      rlc_pP->enb_module_id,
                      rlc_pP->ue_module_id,
                      rlc_pP->rb_id,
                      sdu_mngt_p,
                      sdu_mngt_p->sdu_remaining_size,
                      sdu_mngt_p->sdu_size);
            }
            data_sdu_p = &sdu_mngt_p->first_byte[sdu_mngt_p->sdu_segmented_size];

            if (sdu_mngt_p->sdu_remaining_size > pdu_remaining_size) {
                LOG_T(RLC, "[FRAME %05d][%s][RLC_AM][MOD %u/%u][RB %u][SEGMENT] Filling all remaining PDU with %d bytes\n",
                      frameP,
                      (rlc_pP->is_enb) ? "eNB" : "UE",
                      rlc_pP->enb_module_id,
                      rlc_pP->ue_module_id,
                      rlc_pP->rb_id,
                      pdu_remaining_size);
                //msg ("[FRAME %05d][%s][RLC_AM][MOD %u/%u][RB %u][SEGMENT] pdu_mem_p %p pdu_p %p pdu_p->data %p data %p data_sdu_p %p pdu_remaining_size %d\n", rlc_pP->module_id, rlc_pP->rb_id, frameP, pdu_mem_p, pdu_p, pdu_p->data, data, data_sdu_p,pdu_remaining_size);

                memcpy(data, data_sdu_p, pdu_remaining_size);
                pdu_mngt_p->payload_size += pdu_remaining_size;
                sdu_mngt_p->sdu_remaining_size = sdu_mngt_p->sdu_remaining_size - pdu_remaining_size;
                sdu_mngt_p->sdu_segmented_size = sdu_mngt_p->sdu_segmented_size + pdu_remaining_size;
                fi_last_byte_pdu_is_last_byte_sdu = 0;
                // no LI
                rlc_pP->sdu_buffer_occupancy -= pdu_remaining_size;
                continue_fill_pdu_with_sdu = 0;
                pdu_remaining_size = 0;
                LOG_T(RLC, "[FRAME %05d][%s][RLC_AM][MOD %u/%u][RB %u][SEGMENT] sdu_remaining_size %d bytes sdu_segmented_size %d bytes\n",
                      frameP,
                      (rlc_pP->is_enb) ? "eNB" : "UE",
                      rlc_pP->enb_module_id,
                      rlc_pP->ue_module_id,
                      rlc_pP->rb_id,
                      sdu_mngt_p->sdu_remaining_size,
                      sdu_mngt_p->sdu_segmented_size);
            } else if (sdu_mngt_p->sdu_remaining_size == pdu_remaining_size) {
                LOG_T(RLC, "[FRAME %05d][%s][RLC_AM][MOD %u/%u][RB %u][SEGMENT] Exactly Filling remaining PDU with %d remaining bytes of SDU\n",
                      frameP,
                      (rlc_pP->is_enb) ? "eNB" : "UE",
                      rlc_pP->enb_module_id,
                      rlc_pP->ue_module_id,
                      rlc_pP->rb_id,
                      pdu_remaining_size);
                memcpy(data, data_sdu_p, pdu_remaining_size);
                pdu_mngt_p->payload_size += pdu_remaining_size;

                // free SDU
                rlc_pP->sdu_buffer_occupancy -= sdu_mngt_p->sdu_remaining_size;
                rlc_am_free_in_sdu_data(rlc_pP, rlc_pP->current_sdu_index);
                //free_mem_block (rlc_pP->input_sdus[rlc_pP->current_sdu_index]);
                //rlc_pP->input_sdus[rlc_pP->current_sdu_index] = NULL;
                //rlc_pP->nb_sdu -= 1;
                rlc_pP->current_sdu_index = (rlc_pP->current_sdu_index + 1) % RLC_AM_SDU_CONTROL_BUFFER_SIZE;

                fi_last_byte_pdu_is_last_byte_sdu = 1;
                // fi will indicate end of PDU is end of SDU, no need for LI
                continue_fill_pdu_with_sdu = 0;
                pdu_remaining_size = 0;
            } else if ((sdu_mngt_p->sdu_remaining_size + (li_length_in_bytes ^ 3)) < pdu_remaining_size ) {
                if (fill_num_li == (RLC_AM_MAX_SDU_IN_PDU - 1)) {
                    LOG_T(RLC, "[FRAME %05d][%s][RLC_AM][MOD %u/%u][RB %u][SEGMENT] [SIZE %d] REACHING RLC_AM_MAX_SDU_IN_PDU LIs -> STOP SEGMENTATION FOR THIS PDU SDU\n",
                          frameP,
                          (rlc_pP->is_enb) ? "eNB" : "UE",
                          rlc_pP->enb_module_id,
                          rlc_pP->ue_module_id,
                          rlc_pP->rb_id,
                          sdu_mngt_p->sdu_remaining_size);
                    memcpy(data, data_sdu_p, sdu_mngt_p->sdu_remaining_size);
                    pdu_mngt_p->payload_size += sdu_mngt_p->sdu_remaining_size;
                    pdu_remaining_size = 0; //Forced to 0 pdu_remaining_size - sdu_mngt_p->sdu_remaining_size;
                    // free SDU
                    rlc_pP->sdu_buffer_occupancy -= sdu_mngt_p->sdu_remaining_size;
                    rlc_am_free_in_sdu_data(rlc_pP, rlc_pP->current_sdu_index);
                    //rlc_pP->input_sdus[rlc_pP->current_sdu_index] = NULL;
                    //rlc_pP->nb_sdu -= 1;
                    rlc_pP->current_sdu_index = (rlc_pP->current_sdu_index + 1) % RLC_AM_SDU_CONTROL_BUFFER_SIZE;

                    // reduce the size of the PDU
                    continue_fill_pdu_with_sdu = 0;
                    fi_last_byte_pdu_is_last_byte_sdu = 1;
                } else {
                    LOG_T(RLC, "[FRAME %05d][%s][RLC_AM][MOD %u/%u][RB %u][SEGMENT] Filling  PDU with %d all remaining bytes of SDU\n",
                          frameP,
                          (rlc_pP->is_enb) ? "eNB" : "UE",
                          rlc_pP->enb_module_id,
                          rlc_pP->ue_module_id,
                          rlc_pP->rb_id,
                          sdu_mngt_p->sdu_remaining_size);
                    memcpy(data, data_sdu_p, sdu_mngt_p->sdu_remaining_size);
                    pdu_mngt_p->payload_size += sdu_mngt_p->sdu_remaining_size;
                    data = &data[sdu_mngt_p->sdu_remaining_size];
                    li_length_in_bytes = li_length_in_bytes ^ 3;
                    fill_num_li += 1;
                    if (li_length_in_bytes  == 2) {
                        if (fill_num_li == test_num_li) {
                            //e_li_p->e1  = 0;
                            e_li_p->b1 = 0;
                        } else {
                            //e_li_p->e1  = 1;
                            e_li_p->b1 =  0x80;
                        }
                        //e_li_p->li1 = sdu_mngt_p->sdu_remaining_size;
                        e_li_p->b1 = e_li_p->b1 | (sdu_mngt_p->sdu_remaining_size >> 4);
                        e_li_p->b2 = sdu_mngt_p->sdu_remaining_size << 4;
                        LOG_T(RLC, "[FRAME %05d][%s][RLC_AM][MOD %u/%u][RB %u][SEGMENT] set e_li_p->b1=0x%02X set e_li_p->b2=0x%02X fill_num_li=%d test_num_li=%d\n",
                              frameP,
                              (rlc_pP->is_enb) ? "eNB" : "UE",
                              rlc_pP->enb_module_id,
                              rlc_pP->ue_module_id,
                              rlc_pP->rb_id,
                              e_li_p->b1,
                              e_li_p->b2,
                              fill_num_li,
                              test_num_li);
                    } else {
                        if (fill_num_li != test_num_li) {
                            //e_li_p->e2  = 1;
                            e_li_p->b2  = e_li_p->b2 | 0x08;
                        }
                        //e_li_p->li2 = sdu_mngt_p->sdu_remaining_size;
                        e_li_p->b2 = e_li_p->b2 | (sdu_mngt_p->sdu_remaining_size >> 8);
                        e_li_p->b3 = sdu_mngt_p->sdu_remaining_size & 0xFF;
                        LOG_T(RLC, "[FRAME %05d][%s][RLC_AM][MOD %u/%u][RB %u][SEGMENT] set e_li_p->b2=0x%02X set e_li_p->b3=0x%02X fill_num_li=%d test_num_li=%d\n",
                              frameP,
                              (rlc_pP->is_enb) ? "eNB" : "UE",
                              rlc_pP->enb_module_id,
                              rlc_pP->ue_module_id,
                              rlc_pP->rb_id,
                              e_li_p->b2,
                              e_li_p->b3,
                              fill_num_li,
                              test_num_li);
                        e_li_p++;
                    }

                    pdu_remaining_size = pdu_remaining_size - (sdu_mngt_p->sdu_remaining_size + li_length_in_bytes);
                    // free SDU
                    rlc_pP->sdu_buffer_occupancy  -= sdu_mngt_p->sdu_remaining_size;
                    sdu_mngt_p->sdu_remaining_size = 0;

                    rlc_am_free_in_sdu_data(rlc_pP, rlc_pP->current_sdu_index);
                    //free_mem_block (rlc_pP->input_sdus[rlc_pP->current_sdu_index]);
                    //rlc_pP->input_sdus[rlc_pP->current_sdu_index] = NULL;
                    //rlc_pP->nb_sdu -= 1;
                    rlc_pP->current_sdu_index = (rlc_pP->current_sdu_index + 1) % RLC_AM_SDU_CONTROL_BUFFER_SIZE;
                }
            } else {
                LOG_T(RLC, "[FRAME %05d][%s][RLC_AM][MOD %u/%u][RB %u][SEGMENT] Filling  PDU with %d all remaining bytes of SDU and reduce TB size by %d bytes\n",
                      frameP,
                      (rlc_pP->is_enb) ? "eNB" : "UE",
                      rlc_pP->enb_module_id,
                      rlc_pP->ue_module_id,
                      rlc_pP->rb_id,
                      sdu_mngt_p->sdu_remaining_size,
                      pdu_remaining_size - sdu_mngt_p->sdu_remaining_size);
#ifdef USER_MODE
                assert(1!=1);
#endif
                memcpy(data, data_sdu_p, sdu_mngt_p->sdu_remaining_size);
                pdu_mngt_p->payload_size += sdu_mngt_p->sdu_remaining_size;
                pdu_remaining_size = pdu_remaining_size - sdu_mngt_p->sdu_remaining_size;
                // free SDU
                rlc_pP->sdu_buffer_occupancy -= sdu_mngt_p->sdu_remaining_size;
                rlc_am_free_in_sdu_data(rlc_pP, rlc_pP->current_sdu_index);
                //rlc_pP->input_sdus[rlc_pP->current_sdu_index] = NULL;
                //rlc_pP->nb_sdu -= 1;
                rlc_pP->current_sdu_index = (rlc_pP->current_sdu_index + 1) % RLC_AM_SDU_CONTROL_BUFFER_SIZE;

                // reduce the size of the PDU
                continue_fill_pdu_with_sdu = 0;
                fi_last_byte_pdu_is_last_byte_sdu = 1;
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
        pdu_p->b1 = pdu_p->b1 | (fi << 3);

        // set fist e bit
        if (fill_num_li > 0) {
            pdu_p->b1 = pdu_p->b1 | 0x04;
        }
        LOG_T(RLC, "[FRAME %05d][%s][RLC_AM][MOD %u/%u][RB %u][SEGMENT] SEND PDU SN %04d  SIZE %d BYTES PAYLOAD SIZE %d BYTES\n",
             frameP,
             (rlc_pP->is_enb) ? "eNB" : "UE",
             rlc_pP->enb_module_id,
             rlc_pP->ue_module_id,
             rlc_pP->rb_id,
             rlc_pP->vt_s,
             data_pdu_size - pdu_remaining_size,
             pdu_mngt_p->payload_size);

        rlc_pP->stat_tx_data_pdu   += 1;
        rlc_pP->stat_tx_data_bytes += (data_pdu_size - pdu_remaining_size);

        //pdu_p->sn = rlc_pP->vt_s;
        pdu_p->b1 = pdu_p->b1 | 0x80; // DATA/CONTROL field is DATA PDU
        pdu_p->b1 = pdu_p->b1 | (rlc_pP->vt_s >> 8);
        pdu_p->b2 = rlc_pP->vt_s & 0xFF;
        rlc_pP->vt_s = (rlc_pP->vt_s+1) & RLC_AM_SN_MASK;

        pdu_tb_req_p->data_ptr        = (unsigned char*)pdu_p;
        pdu_tb_req_p->tb_size         = data_pdu_size - pdu_remaining_size;

        assert(pdu_tb_req_p->tb_size < 3000);
        rlc_am_pdu_polling(rlc_pP, frameP,pdu_p, pdu_mngt_p->payload_size);

        //list_add_tail_eurecom (pdu_mem_p, &rlc_pP->segmentation_pdu_list);
        pdu_mngt_p->mem_block  = pdu_mem_p;
        pdu_mngt_p->first_byte = (unsigned char*)pdu_p;
        pdu_mngt_p->header_and_payload_size  = data_pdu_size - pdu_remaining_size;
        pdu_mngt_p->retx_count = -1;

        rlc_pP->retrans_num_pdus  += 1;
        rlc_pP->retrans_num_bytes += pdu_mngt_p->header_and_payload_size;

        pdu_p = NULL;
        pdu_mem_p = NULL;

        //nb_bytes_to_transmit = nb_bytes_to_transmit - data_pdu_size;
        nb_bytes_to_transmit = 0; // 1 PDU only

        mem_block_t* copy = rlc_am_retransmit_get_copy (rlc_pP, frameP,(rlc_pP->vt_s-1) & RLC_AM_SN_MASK);
        list_add_tail_eurecom (copy, &rlc_pP->segmentation_pdu_list);

    }
}
