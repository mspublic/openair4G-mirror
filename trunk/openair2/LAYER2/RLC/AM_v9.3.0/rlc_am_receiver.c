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
#define RLC_AM_RECEIVER_C
//#include "rtos_header.h"
#include "platform_types.h"
//-----------------------------------------------------------------------------
#include "assertions.h"
#include "rlc.h"
#include "rlc_am.h"
#include "list.h"
#include "LAYER2/MAC/extern.h"
#include "UTIL/LOG/log.h"

#define TRACE_RLC_AM_RX
//#define DEBUG_RLC_AM_DISPLAY_TB_DATA
//#define RLC_AM_GENERATE_ERRORS
//-----------------------------------------------------------------------------
signed int rlc_am_get_data_pdu_infos(frame_t frameP, rlc_am_pdu_sn_10_t* header_pP, int16_t total_sizeP, rlc_am_pdu_info_t* pdu_info_pP)
//-----------------------------------------------------------------------------
{
    memset(pdu_info_pP, 0, sizeof (rlc_am_pdu_info_t));

    int16_t          sum_li = 0;
    pdu_info_pP->d_c = header_pP->b1 >> 7;
    pdu_info_pP->num_li = 0;


    if (pdu_info_pP->d_c) {
        pdu_info_pP->rf  = (header_pP->b1 >> 6) & 0x01;
        pdu_info_pP->p   = (header_pP->b1 >> 5) & 0x01;
        pdu_info_pP->fi  = (header_pP->b1 >> 3) & 0x03;
        pdu_info_pP->e   = (header_pP->b1 >> 2) & 0x01;
        pdu_info_pP->sn  = header_pP->b2 +  (((uint16_t)(header_pP->b1 & 0x03)) << 8);

        pdu_info_pP->header_size  = 2;
        if (pdu_info_pP->rf) {
            pdu_info_pP->lsf = (header_pP->data[0] >> 7) & 0x01;
            pdu_info_pP->so  = header_pP->data[1] +  (((uint16_t)(header_pP->data[0] & 0x7F)) << 8);
            pdu_info_pP->payload = &header_pP->data[2];
            pdu_info_pP->header_size  += 2;
        } else {
            pdu_info_pP->payload = &header_pP->data[0];
        }

        if (pdu_info_pP->e) {
            rlc_am_e_li_t      *e_li;
            unsigned int li_length_in_bytes  = 1;
            unsigned int li_to_read          = 1;

            if (pdu_info_pP->rf) {
                e_li = (rlc_am_e_li_t*)(&header_pP->data[2]);
            } else {
                e_li = (rlc_am_e_li_t*)(header_pP->data);
            }
            while (li_to_read)  {
                li_length_in_bytes = li_length_in_bytes ^ 3;
                if (li_length_in_bytes  == 2) {
                    pdu_info_pP->li_list[pdu_info_pP->num_li] = ((uint16_t)(e_li->b1 << 4)) & 0x07F0;
                    pdu_info_pP->li_list[pdu_info_pP->num_li] |= (((uint8_t)(e_li->b2 >> 4)) & 0x000F);
                    li_to_read = e_li->b1 & 0x80;
                    pdu_info_pP->header_size  += 2;
                } else {
                    pdu_info_pP->li_list[pdu_info_pP->num_li] = ((uint16_t)(e_li->b2 << 8)) & 0x0700;
                    pdu_info_pP->li_list[pdu_info_pP->num_li] |=  e_li->b3;
                    li_to_read = e_li->b2 & 0x08;
                    e_li++;
                    pdu_info_pP->header_size  += 1;
                }
                sum_li += pdu_info_pP->li_list[pdu_info_pP->num_li];
                pdu_info_pP->num_li = pdu_info_pP->num_li + 1;
                if (pdu_info_pP->num_li > RLC_AM_MAX_SDU_IN_PDU) {
                    LOG_E(RLC, "[FRAME %5u][RLC_AM][MOD XX][RB XX][GET PDU INFO]  SN %04d TOO MANY LIs ",
                          frameP,
                          pdu_info_pP->sn);
                    return -2;
                }
            }
            if (li_length_in_bytes  == 2) {
                pdu_info_pP->payload = &e_li->b3;
            } else {
                pdu_info_pP->payload = &e_li->b1;
            }
        }
        pdu_info_pP->payload_size = total_sizeP - pdu_info_pP->header_size;
        if (pdu_info_pP->payload_size > sum_li) {
            pdu_info_pP->hidden_size = pdu_info_pP->payload_size - sum_li;
        }
        return 0;
    } else {
        LOG_W(RLC, "[FRAME %5u][RLC_AM][MOD XX][RB XX][GET DATA PDU INFO]  SN %04d ERROR CONTROL PDU ",
              frameP,
              pdu_info_pP->sn);
        return -1;
    }
}
//-----------------------------------------------------------------------------
void rlc_am_display_data_pdu_infos(rlc_am_entity_t *rlc_pP, frame_t frameP, rlc_am_pdu_info_t* pdu_info_pP)
//-----------------------------------------------------------------------------
{
    int num_li;

    if (pdu_info_pP->d_c) {
        if (pdu_info_pP->rf) {
            LOG_D(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][DISPLAY DATA PDU] RX DATA PDU SN %04d FI %1d SO %05d LSF %01d POLL %1d ",
                  frameP,
                  (rlc_pP->is_enb) ? "eNB" : "UE",
                  rlc_pP->enb_module_id,
                  rlc_pP->ue_module_id,
                  rlc_pP->rb_id,
                  pdu_info_pP->sn,
                  pdu_info_pP->fi,
                  pdu_info_pP->so,
                  pdu_info_pP->lsf, pdu_info_pP->p);
        } else {
            LOG_D(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][DISPLAY DATA PDU] RX DATA PDU SN %04d FI %1d POLL %1d ",
                  frameP,
                  (rlc_pP->is_enb) ? "eNB" : "UE",
                  rlc_pP->enb_module_id,
                  rlc_pP->ue_module_id,
                  rlc_pP->rb_id,
                  pdu_info_pP->sn,
                  pdu_info_pP->fi,
                  pdu_info_pP->p);
        }
        for (num_li = 0; num_li < pdu_info_pP->num_li; num_li++) {
            LOG_D(RLC, "LI %05d ",  pdu_info_pP->li_list[num_li]);
        }
        if (pdu_info_pP->hidden_size > 0) {
            LOG_D(RLC, "hidden size %05d ",  pdu_info_pP->hidden_size);
        }
        LOG_D(RLC, "\n");
    } else {
        LOG_E(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][DISPLAY DATA PDU] ERROR RX CONTROL PDU\n",
              frameP,
              (rlc_pP->is_enb) ? "eNB" : "UE",
              rlc_pP->enb_module_id,
              rlc_pP->ue_module_id,
              rlc_pP->rb_id);
    }
}
// assumed the sn of the tb_p is equal to VR(MS)
//-----------------------------------------------------------------------------
void rlc_am_rx_update_vr_ms(rlc_am_entity_t *rlc_pP, frame_t frameP, mem_block_t* tb_pP)
//-----------------------------------------------------------------------------
{
    //rlc_am_pdu_info_t* pdu_info_p        = &((rlc_am_rx_pdu_management_t*)(tb_pP->data))->pdu_info;
    rlc_am_pdu_info_t* pdu_info_cursor_p = NULL;
    mem_block_t*       cursor_p;

    cursor_p = tb_pP;
    if (cursor_p) {
        do {
            pdu_info_cursor_p = &((rlc_am_rx_pdu_management_t*)(cursor_p->data))->pdu_info;
            if (((rlc_am_rx_pdu_management_t*)(cursor_p->data))->all_segments_received == 0) {
#ifdef TRACE_RLC_AM_RX
               LOG_D(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][UPDATE VR(MS)] UPDATED VR(MS) %04d -> %04d\n",
                     frameP,
                     (rlc_pP->is_enb) ? "eNB" : "UE",
                     rlc_pP->enb_module_id,
                     rlc_pP->ue_module_id,
                     rlc_pP->rb_id,
                     rlc_pP->vr_ms, pdu_info_cursor_p->sn);
#endif
                rlc_pP->vr_ms = pdu_info_cursor_p->sn;
                return;
            }
            cursor_p = cursor_p->next;
        } while (cursor_p != NULL);
#ifdef TRACE_RLC_AM_RX
        LOG_D(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][UPDATE VR(MS)] UPDATED VR(MS) %04d -> %04d\n",
              frameP,
              (rlc_pP->is_enb) ? "eNB" : "UE",
              rlc_pP->enb_module_id,
              rlc_pP->ue_module_id,
              rlc_pP->rb_id,
              rlc_pP->vr_ms,
              (pdu_info_cursor_p->sn + 1)  & RLC_AM_SN_MASK);
#endif
        rlc_pP->vr_ms = (pdu_info_cursor_p->sn + 1)  & RLC_AM_SN_MASK;
    }
}
// assumed the sn of the tb_p is equal to VR(R)
//-----------------------------------------------------------------------------
void rlc_am_rx_update_vr_r(rlc_am_entity_t *rlc_pP,frame_t frameP,mem_block_t* tb_pP)
//-----------------------------------------------------------------------------
{
    rlc_am_pdu_info_t* pdu_info_cursor_p = NULL;
    mem_block_t*       cursor_p;

    cursor_p = tb_pP;
    if (cursor_p) {
        do {
            pdu_info_cursor_p = &((rlc_am_rx_pdu_management_t*)(cursor_p->data))->pdu_info;
            if ((((rlc_am_rx_pdu_management_t*)(cursor_p->data))->all_segments_received == 0) ||
               (rlc_pP->vr_r != pdu_info_cursor_p->sn)) {
                return;
            }
#ifdef TRACE_RLC_AM_RX
            LOG_D(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][UPDATE VR(R)] UPDATED VR(R) %04d -> %04d\n",
                  frameP,
                  (rlc_pP->is_enb) ? "eNB" : "UE",
                  rlc_pP->enb_module_id,
                  rlc_pP->ue_module_id,
                  rlc_pP->rb_id,
                  rlc_pP->vr_r,
                  (pdu_info_cursor_p->sn + 1) & RLC_AM_SN_MASK);
#endif
            if (((rlc_am_rx_pdu_management_t*)(cursor_p->data))->pdu_info.rf == 1) {
                if (((rlc_am_rx_pdu_management_t*)(cursor_p->data))->pdu_info.lsf == 1) {
                    rlc_pP->vr_r = (rlc_pP->vr_r + 1) & RLC_AM_SN_MASK;
                }
            } else  {
                rlc_pP->vr_r = (rlc_pP->vr_r + 1) & RLC_AM_SN_MASK;
            }
            cursor_p = cursor_p->next;
        } while (cursor_p != NULL);
        //rlc_pP->vr_r = (pdu_info_cursor_p->sn + 1) & RLC_AM_SN_MASK;
    }
}
//-----------------------------------------------------------------------------
void
rlc_am_receive_routing (rlc_am_entity_t *rlc_pP, frame_t frameP, eNB_flag_t eNB_flagP, struct mac_data_ind data_indP)
//-----------------------------------------------------------------------------
{
    mem_block_t        *tb_p             = NULL;
    uint8_t               *first_byte_p     = NULL;
    int16_t               tb_size_in_bytes;

    while ((tb_p = list_remove_head (&data_indP.data))) {
        first_byte_p = ((struct mac_tb_ind *) (tb_p->data))->data_ptr;
        tb_size_in_bytes = ((struct mac_tb_ind *) (tb_p->data))->size;

        if (tb_size_in_bytes > 0) {
            if ((*first_byte_p & 0x80) == 0x80) {
                rlc_pP->stat_rx_data_bytes += tb_size_in_bytes;
                rlc_pP->stat_rx_data_pdu   += 1;
                rlc_am_receive_process_data_pdu (rlc_pP, frameP, eNB_flagP, tb_p, first_byte_p, tb_size_in_bytes);
            } else {
                rlc_pP->stat_rx_control_bytes += tb_size_in_bytes;
                rlc_pP->stat_rx_control_pdu += 1;
                rlc_am_receive_process_control_pdu (rlc_pP, frameP, tb_p, &first_byte_p, &tb_size_in_bytes);
                // Test if remaining bytes not processed (up to know, highest probability is bug in MAC)
                AssertFatal( tb_size_in_bytes == 0,
                                        "Remaining %d bytes following a control PDU",
                                        tb_size_in_bytes);
            }
            LOG_D(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][RX ROUTING] VR(R)=%03d VR(MR)=%03d\n",
                  frameP,
                  (rlc_pP->is_enb) ? "eNB" : "UE",
                  rlc_pP->enb_module_id,
                  rlc_pP->ue_module_id,
                  rlc_pP->rb_id,
                  rlc_pP->vr_r,
                  rlc_pP->vr_mr);
        }
    } // end while
}
//-----------------------------------------------------------------------------
void rlc_am_receive_process_data_pdu (rlc_am_entity_t *rlc_pP, frame_t frameP, eNB_flag_t eNB_flagP, mem_block_t* tb_pP, uint8_t* first_byte_pP, uint16_t tb_size_in_bytesP)
//-----------------------------------------------------------------------------
{
  // 5.1.3.2 Receive operations
  // 5.1.3.2.1 General
  // The receiving side of an AM RLC entity shall maintain a receiving window according to state variables VR(R) and
  // VR(MR) as follows:
  //     - a SN falls within the receiving window if VR(R) <= SN < VR(MR);
  //     - a SN falls outside of the receiving window otherwise.
  //
  // When receiving a RLC data PDU from lower layer, the receiving side of an AM RLC entity shall:
  // - either discard the received RLC data PDU or place it in the reception buffer (see sub clause 5.1.3.2.2);
  // - if the received RLC data PDU was placed in the reception buffer:
  //     - update state variables, reassemble and deliver RLC SDUs to upper layer and start/stop t-Reordering as
  //       needed (see sub clause 5.1.3.2.3).
  // When t-Reordering expires, the receiving side of an AM RLC entity shall:
  //     - update state variables and start t-Reordering as needed (see sub clause 5.1.3.2.4).


  // 5.1.3.2.2 Actions when a RLC data PDU is received from lower layer
  // When a RLC data PDU is received from lower layer, where the RLC data PDU contains byte segment numbers y to z of
  // an AMD PDU with SN = x, the receiving side of an AM RLC entity shall:
  //     - if x falls outside of the receiving window; or
  //     - if byte segment numbers y to z of the AMD PDU with SN = x have been received before:
  //         - discard the received RLC data PDU;
  //     - else:
  //         - place the received RLC data PDU in the reception buffer;
  //         - if some byte segments of the AMD PDU contained in the RLC data PDU have been received before:
  //             - discard the duplicate byte segments.
  rlc_am_pdu_info_t*  pdu_info_p         = &((rlc_am_rx_pdu_management_t*)(tb_pP->data))->pdu_info;
  rlc_am_pdu_sn_10_t* rlc_am_pdu_sn_10_p = (rlc_am_pdu_sn_10_t*)first_byte_pP;

  if (rlc_am_get_data_pdu_infos(frameP,rlc_am_pdu_sn_10_p, tb_size_in_bytesP, pdu_info_p) >= 0) {

      ((rlc_am_rx_pdu_management_t*)(tb_pP->data))->all_segments_received = 0;
      if (rlc_am_in_rx_window(rlc_pP, pdu_info_p->sn)) {

          if (pdu_info_p->p) {
              LOG_D(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][PROCESS RX PDU]  POLL BIT SET, STATUS REQUESTED:\n",
                  frameP,
                  (rlc_pP->is_enb) ? "eNB" : "UE",
                  rlc_pP->enb_module_id,
                  rlc_pP->ue_module_id,
                  rlc_pP->rb_id);
              rlc_pP->status_requested = 1;
          }
          LOG_D(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][PROCESS RX PDU] VR(R) %04d VR(H) %04d VR(MR) %04d VR(MS) %04d VR(X) %04d\n",
                frameP,
                (rlc_pP->is_enb) ? "eNB" : "UE",
                rlc_pP->enb_module_id,
                rlc_pP->ue_module_id,
                rlc_pP->rb_id,
                rlc_pP->vr_r,
                rlc_pP->vr_h,
                rlc_pP->vr_mr,
                rlc_pP->vr_ms,
                rlc_pP->vr_x);

          if (rlc_am_rx_list_insert_pdu(rlc_pP, frameP,tb_pP) < 0) {
              rlc_pP->stat_rx_data_pdu_dropped     += 1;
              rlc_pP->stat_rx_data_bytes_dropped   += tb_size_in_bytesP;
              free_mem_block (tb_pP);
              LOG_D(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][PROCESS RX PDU]  PDU DISCARDED, STATUS REQUESTED:\n",
                  frameP,
                  (rlc_pP->is_enb) ? "eNB" : "UE",
                  rlc_pP->enb_module_id,
                  rlc_pP->ue_module_id,
                  rlc_pP->rb_id);
              rlc_pP->status_requested = 1;
#if defined(RLC_STOP_ON_LOST_PDU)
              AssertFatal( 0 == 1,
                    "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u] LOST PDU DETECTED\n",
                    frameP,
                    (rlc_pP->is_enb) ? "eNB" : "UE",
                    rlc_pP->enb_module_id,
                    rlc_pP->ue_module_id,
                    rlc_pP->rb_id);
#endif
          } else {
            // 5.1.3.2.3
            // Actions when a RLC data PDU is placed in the reception buffer
            //
            // When a RLC data PDU with SN = x is placed in the reception buffer, the receiving side of an AM RLC entity shall:
            //     - if x >= VR(H)
            //         - update VR(H) to x+ 1;
            //
            //     - if all byte segments of the AMD PDU with SN = VR(MS) are received:
            //         - update VR(MS) to the SN of the first AMD PDU with SN > current VR(MS) for which not all byte segments
            //           have been received;
            //
            //     - if x = VR(R):
            //         - if all byte segments of the AMD PDU with SN = VR(R) are received:
            //             - update VR(R) to the SN of the first AMD PDU with SN > current VR(R) for which not all byte segments
            //               have been received;
            //             - update VR(MR) to the updated VR(R) + AM_Window_Size;
            //
            //         - reassemble RLC SDUs from any byte segments of AMD PDUs with SN that falls outside of the receiving
            //           window and in-sequence byte segments of the AMD PDU with SN = VR(R), remove RLC headers when
            //           doing so and deliver the reassembled RLC SDUs to upper layer in sequence if not delivered before;
            //
            //     - if t-Reordering is running:
            //         - if VR(X) = VR(R); or
            //         - if VR(X) falls outside of the receiving window and VR(X) is not equal to VR(MR):
            //             - stop and reset t-Reordering;
            //
            //     - if t-Reordering is not running (includes the case t-Reordering is stopped due to actions above):
            //         - if VR (H) > VR(R):
            //             - start t-Reordering;
            //             - set VR(X) to VR(H).


#ifdef TRACE_RLC_AM_RX
              LOG_D(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][PROCESS RX PDU]  RX LIST AFTER INSERTION:\n",
                    frameP,
                    (rlc_pP->is_enb) ? "eNB" : "UE",
                    rlc_pP->enb_module_id,
                    rlc_pP->ue_module_id,
                    rlc_pP->rb_id);
              rlc_am_rx_list_display(rlc_pP, "rlc_am_receive_process_data_pdu AFTER INSERTION ");
#endif
              if (rlc_am_sn_gte_vr_h(rlc_pP, pdu_info_p->sn) > 0) {
                  rlc_pP->vr_h = (pdu_info_p->sn + 1) & RLC_AM_SN_MASK;
              }
              rlc_am_rx_check_all_byte_segments(rlc_pP, frameP, tb_pP);
              if ((pdu_info_p->sn == rlc_pP->vr_ms) && (((rlc_am_rx_pdu_management_t*)(tb_pP->data))->all_segments_received)) {
                  rlc_am_rx_update_vr_ms(rlc_pP, frameP, tb_pP);
              }
              if (pdu_info_p->sn == rlc_pP->vr_r) {
                 if (((rlc_am_rx_pdu_management_t*)(tb_pP->data))->all_segments_received) {
                      rlc_am_rx_update_vr_r(rlc_pP, frameP, tb_pP);
                      rlc_pP->vr_mr = (rlc_pP->vr_r + RLC_AM_WINDOW_SIZE) & RLC_AM_SN_MASK;
                  }
                  rlc_am_rx_list_reassemble_rlc_sdus(rlc_pP,frameP,eNB_flagP);
              }

              if (rlc_pP->t_reordering.running) {
                  if ((rlc_pP->vr_x == rlc_pP->vr_r) || ((rlc_am_in_rx_window(rlc_pP, pdu_info_p->sn) == 0) && (rlc_pP->vr_x != rlc_pP->vr_mr))) {
                      rlc_am_stop_and_reset_timer_reordering(rlc_pP,frameP);
                  }
              }
              if (!(rlc_pP->t_reordering.running)) {
                  if (rlc_pP->vr_h != rlc_pP->vr_r) { // - if VR (H) > VR(R) translated to - if VR (H) != VR(R)
                      rlc_am_start_timer_reordering(rlc_pP,frameP);
                      rlc_pP->vr_x = rlc_pP->vr_h;
                  }
              }
          }
          LOG_D(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][PROCESS RX PDU] VR(R) %04d VR(H) %04d  VR(MS) %04d  VR(MR) %04d\n",
                frameP,
                (rlc_pP->is_enb) ? "eNB" : "UE",
                rlc_pP->enb_module_id,
                rlc_pP->ue_module_id,
                rlc_pP->rb_id,
                rlc_pP->vr_r,
                rlc_pP->vr_h,
                rlc_pP->vr_ms,
                rlc_pP->vr_mr);
      } else {
          rlc_pP->stat_rx_data_pdu_out_of_window     += 1;
          rlc_pP->stat_rx_data_bytes_out_of_window   += tb_size_in_bytesP;
          free_mem_block (tb_pP);
          LOG_D(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][PROCESS RX PDU]  PDU OUT OF RX WINDOW, DISCARDED, STATUS REQUESTED:\n",
                frameP,
                (rlc_pP->is_enb) ? "eNB" : "UE",
                rlc_pP->enb_module_id,
                rlc_pP->ue_module_id,
                rlc_pP->rb_id);
          rlc_pP->status_requested = 1;
      }
  } else {
      free_mem_block (tb_pP);
  }
}
