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
#define RLC_AM_REASSEMBLY_C
#include "platform_types.h"
//-----------------------------------------------------------------------------
#include "assertions.h"
#include "rlc.h"
#include "rlc_am.h"
#include "list.h"
#include "LAYER2/MAC/extern.h"
#include "UTIL/LOG/log.h"

//#define TRACE_RLC_AM_RX_DECODE
//-----------------------------------------------------------------------------
inline void rlc_am_clear_rx_sdu (rlc_am_entity_t *rlc_pP)
//-----------------------------------------------------------------------------
{
  rlc_pP->output_sdu_size_to_write = 0;
}
//-----------------------------------------------------------------------------
void rlc_am_reassembly (u8_t * src_pP, s32_t lengthP, rlc_am_entity_t *rlc_pP, frame_t frameP)
//-----------------------------------------------------------------------------
{
  LOG_D(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][REASSEMBLY PAYLOAD] reassembly()  %d bytes\n",
      frameP,
      (rlc_pP->is_enb) ? "eNB" : "UE",
      rlc_pP->enb_module_id,
      rlc_pP->ue_module_id,
      rlc_pP->rb_id,
      lengthP);

  if (rlc_pP->output_sdu_in_construction == NULL) {
      rlc_pP->output_sdu_in_construction = get_free_mem_block (RLC_SDU_MAX_SIZE);
      rlc_pP->output_sdu_size_to_write = 0;
      assert(rlc_pP->output_sdu_in_construction != NULL);
  }
  if (rlc_pP->output_sdu_in_construction != NULL) {

      // check if no overflow in size
      if ((rlc_pP->output_sdu_size_to_write + lengthP) <= RLC_SDU_MAX_SIZE) {
          memcpy (&rlc_pP->output_sdu_in_construction->data[rlc_pP->output_sdu_size_to_write], src_pP, lengthP);

          rlc_pP->output_sdu_size_to_write += lengthP;
      } else {
          LOG_E(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][REASSEMBLY PAYLOAD] ERROR  SDU SIZE OVERFLOW SDU GARBAGED\n",
              frameP,
              (rlc_pP->is_enb) ? "eNB" : "UE",
                  rlc_pP->enb_module_id,
                  rlc_pP->ue_module_id,
                  rlc_pP->rb_id);
#if defined(STOP_ON_IP_TRAFFIC_OVERLOAD)
      AssertFatal(0, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u] RLC_AM_DATA_IND, SDU SIZE OVERFLOW SDU GARBAGED\n",
          frameP,
          (rlc_pP->is_enb) ? "eNB" : "UE",
          rlc_pP->enb_module_id,
          rlc_pP->ue_module_id,
          rlc_pP->rb_id);
#endif
          // erase  SDU
          rlc_pP->output_sdu_size_to_write = 0;
      }
  } else {
      LOG_E(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][REASSEMBLY PAYLOAD] ERROR  OUTPUT SDU IS NULL\n",
          frameP,
          (rlc_pP->is_enb) ? "eNB" : "UE",
           rlc_pP->enb_module_id,
           rlc_pP->ue_module_id,
           rlc_pP->rb_id);
#if defined(STOP_ON_IP_TRAFFIC_OVERLOAD)
      AssertFatal(0, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u] RLC_AM_DATA_IND, SDU DROPPED, OUT OF MEMORY\n",
          frameP,
          (rlc_pP->is_enb) ? "eNB" : "UE",
          rlc_pP->enb_module_id,
          rlc_pP->ue_module_id,
          rlc_pP->rb_id);
#endif
  }
}
//-----------------------------------------------------------------------------
void rlc_am_send_sdu (rlc_am_entity_t *rlc_pP,frame_t frameP, eNB_flag_t eNB_flagP)
//-----------------------------------------------------------------------------
{
  if ((rlc_pP->output_sdu_in_construction)) {
      LOG_D(RLC, "\n\n\n[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][SEND_SDU] %d bytes sdu %p\n",
          frameP,
          (rlc_pP->is_enb) ? "eNB" : "UE",
              rlc_pP->enb_module_id,
              rlc_pP->ue_module_id,
              rlc_pP->rb_id,
              rlc_pP->output_sdu_size_to_write,
              rlc_pP->output_sdu_in_construction);
      if (rlc_pP->output_sdu_size_to_write > 0) {
          rlc_pP->stat_rx_pdcp_sdu   += 1;
          rlc_pP->stat_rx_pdcp_bytes += rlc_pP->output_sdu_size_to_write;
#ifdef TEST_RLC_AM
          rlc_am_v9_3_0_test_data_ind (rlc_pP->module_id,
              rlc_pP->rb_id,
              rlc_pP->output_sdu_size_to_write,
              rlc_pP->output_sdu_in_construction);
#else
          rlc_data_ind (rlc_pP->enb_module_id, rlc_pP->ue_module_id, frameP, eNB_flagP, RLC_MBMS_NO, rlc_pP->rb_id, rlc_pP->output_sdu_size_to_write, rlc_pP->output_sdu_in_construction, rlc_pP->is_data_plane);
#endif
          rlc_pP->output_sdu_in_construction = NULL;
      } else {
          LOG_E(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][SEND_SDU] ERROR SIZE <= 0 ... DO NOTHING, SET SDU SIZE TO 0\n",
              frameP,
              (rlc_pP->is_enb) ? "eNB" : "UE",
                  rlc_pP->enb_module_id,
                  rlc_pP->ue_module_id,
                  rlc_pP->rb_id);
          //msg("[RLC_AM][MOD %d] Freeing mem_block ...\n", rlc_pP->module_id);
          //free_mem_block (rlc_pP->output_sdu_in_construction);
          assert(3==4);
      }
      rlc_pP->output_sdu_size_to_write = 0;
  }
}
//-----------------------------------------------------------------------------
void rlc_am_reassemble_pdu(rlc_am_entity_t* rlc_pP, frame_t frameP, eNB_flag_t eNB_flagP, mem_block_t* tb_pP) {
  //-----------------------------------------------------------------------------
  int i,j;

  rlc_am_pdu_info_t* pdu_info        = &((rlc_am_rx_pdu_management_t*)(tb_pP->data))->pdu_info;
  LOG_D(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][REASSEMBLY PDU] TRY REASSEMBLY PDU SN=%03d\n",
      frameP,
      (rlc_pP->is_enb) ? "eNB" : "UE",
          rlc_pP->enb_module_id,
          rlc_pP->ue_module_id,
          rlc_pP->rb_id,
          pdu_info->sn);
#ifdef TRACE_RLC_AM_RX_DECODE
  rlc_am_display_data_pdu_infos(rlc_pP, frameP, pdu_info);
#endif

  if (pdu_info->e == RLC_E_FIXED_PART_DATA_FIELD_FOLLOW) {
      switch (pdu_info->fi) {
      case RLC_FI_1ST_BYTE_DATA_IS_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_LAST_BYTE_SDU:
        LOG_D(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][REASSEMBLY PDU] TRY REASSEMBLY PDU NO E_LI FI=11 (00)\n",
            frameP,
            (rlc_pP->is_enb) ? "eNB" : "UE",
                rlc_pP->enb_module_id,
                rlc_pP->ue_module_id,
                rlc_pP->rb_id);
        // one complete SDU
        rlc_am_send_sdu(rlc_pP,frameP,eNB_flagP); // may be not necessary
        rlc_am_reassembly (pdu_info->payload, pdu_info->payload_size, rlc_pP,frameP);
        rlc_am_send_sdu(rlc_pP,frameP,eNB_flagP); // may be not necessary
        //rlc_pP->reassembly_missing_sn_detected = 0;
        break;
      case RLC_FI_1ST_BYTE_DATA_IS_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_NOT_LAST_BYTE_SDU:
        LOG_D(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][REASSEMBLY PDU] TRY REASSEMBLY PDU NO E_LI FI=10 (01)\n",
            frameP,
            (rlc_pP->is_enb) ? "eNB" : "UE",
                rlc_pP->enb_module_id,
                rlc_pP->ue_module_id,
                rlc_pP->rb_id);
        // one beginning segment of SDU in PDU
        rlc_am_send_sdu(rlc_pP,frameP,eNB_flagP); // may be not necessary
        rlc_am_reassembly (pdu_info->payload, pdu_info->payload_size, rlc_pP,frameP);
        //rlc_pP->reassembly_missing_sn_detected = 0;
        break;
      case RLC_FI_1ST_BYTE_DATA_IS_NOT_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_LAST_BYTE_SDU:
        LOG_D(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][REASSEMBLY PDU] TRY REASSEMBLY PDU NO E_LI FI=01 (10)\n",
            frameP,
            (rlc_pP->is_enb) ? "eNB" : "UE",
                rlc_pP->enb_module_id,
                rlc_pP->ue_module_id,
                rlc_pP->rb_id);
        // one last segment of SDU
        //if (rlc_pP->reassembly_missing_sn_detected == 0) {
        rlc_am_reassembly (pdu_info->payload, pdu_info->payload_size, rlc_pP,frameP);
        rlc_am_send_sdu(rlc_pP,frameP,eNB_flagP);
        //} // else { clear sdu already done
        //rlc_pP->reassembly_missing_sn_detected = 0;
        break;
      case RLC_FI_1ST_BYTE_DATA_IS_NOT_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_NOT_LAST_BYTE_SDU:
        LOG_D(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][REASSEMBLY PDU] TRY REASSEMBLY PDU NO E_LI FI=00 (11)\n",
            frameP,
            (rlc_pP->is_enb) ? "eNB" : "UE",
                rlc_pP->enb_module_id,
                rlc_pP->ue_module_id,
                rlc_pP->rb_id);
        //if (rlc_pP->reassembly_missing_sn_detected == 0) {
        // one whole segment of SDU in PDU
        rlc_am_reassembly (pdu_info->payload, pdu_info->payload_size, rlc_pP,frameP);
        //} else {
        //    rlc_pP->reassembly_missing_sn_detected = 1; // not necessary but for readability of the code
        //}

        break;
#ifdef USER_MODE
      default:
        assert(0 != 0);
#endif
      }
  } else {
      switch (pdu_info->fi) {
      case RLC_FI_1ST_BYTE_DATA_IS_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_LAST_BYTE_SDU:
        LOG_D(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][REASSEMBLY PDU] TRY REASSEMBLY PDU FI=11 (00) Li=",
            frameP,
            (rlc_pP->is_enb) ? "eNB" : "UE",
                rlc_pP->enb_module_id,
                rlc_pP->ue_module_id,
                rlc_pP->rb_id);
        for (i=0; i < pdu_info->num_li; i++) {
            LOG_D(RLC, "%d ",pdu_info->li_list[i]);
        }
        LOG_D(RLC, "\n");
        //msg(" remaining size %d\n",size);
        // N complete SDUs
        rlc_am_send_sdu(rlc_pP,frameP,eNB_flagP);
        j = 0;
        for (i = 0; i < pdu_info->num_li; i++) {
            rlc_am_reassembly (&pdu_info->payload[j], pdu_info->li_list[i], rlc_pP,frameP);
            rlc_am_send_sdu(rlc_pP,frameP,eNB_flagP);
            j = j + pdu_info->li_list[i];
        }
        if (pdu_info->hidden_size > 0) { // normally should always be > 0 but just for help debug
            // data is already ok, done by last loop above
            rlc_am_reassembly (&pdu_info->payload[j], pdu_info->hidden_size, rlc_pP,frameP);
            rlc_am_send_sdu(rlc_pP,frameP,eNB_flagP);
        }
        //rlc_pP->reassembly_missing_sn_detected = 0;
        break;
      case RLC_FI_1ST_BYTE_DATA_IS_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_NOT_LAST_BYTE_SDU:
        LOG_D(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][REASSEMBLY PDU] TRY REASSEMBLY PDU FI=10 (01) Li=",
            frameP,
            (rlc_pP->is_enb) ? "eNB" : "UE",
                rlc_pP->enb_module_id,
                rlc_pP->ue_module_id,
                rlc_pP->rb_id);
        for (i=0; i < pdu_info->num_li; i++) {
            LOG_D(RLC, "%d ",pdu_info->li_list[i]);
        }
        LOG_D(RLC, "\n");
        //msg(" remaining size %d\n",size);
        // N complete SDUs + one segment of SDU in PDU
        rlc_am_send_sdu(rlc_pP,frameP,eNB_flagP);
        j = 0;
        for (i = 0; i < pdu_info->num_li; i++) {
            rlc_am_reassembly (&pdu_info->payload[j], pdu_info->li_list[i], rlc_pP,frameP);
            rlc_am_send_sdu(rlc_pP,frameP,eNB_flagP);
            j = j + pdu_info->li_list[i];
        }
        if (pdu_info->hidden_size > 0) { // normally should always be > 0 but just for help debug
            // data is already ok, done by last loop above
            rlc_am_reassembly (&pdu_info->payload[j], pdu_info->hidden_size, rlc_pP, frameP);
        }
        //rlc_pP->reassembly_missing_sn_detected = 0;
        break;
      case RLC_FI_1ST_BYTE_DATA_IS_NOT_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_LAST_BYTE_SDU:
        LOG_D(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][REASSEMBLY PDU] TRY REASSEMBLY PDU FI=01 (10) Li=",
            frameP,
            (rlc_pP->is_enb) ? "eNB" : "UE",
                rlc_pP->enb_module_id,
                rlc_pP->ue_module_id,
                rlc_pP->rb_id);
        for (i=0; i < pdu_info->num_li; i++) {
            LOG_D(RLC, "%d ",pdu_info->li_list[i]);
        }
        LOG_D(RLC, "\n");
        //msg(" remaining size %d\n",size);
        // one last segment of SDU + N complete SDUs in PDU
        j = 0;
        for (i = 0; i < pdu_info->num_li; i++) {
            rlc_am_reassembly (&pdu_info->payload[j], pdu_info->li_list[i], rlc_pP,frameP);
            rlc_am_send_sdu(rlc_pP,frameP,eNB_flagP);
            j = j + pdu_info->li_list[i];
        }
        if (pdu_info->hidden_size > 0) { // normally should always be > 0 but just for help debug
            // data is already ok, done by last loop above
            rlc_am_reassembly (&pdu_info->payload[j], pdu_info->hidden_size, rlc_pP,frameP);
            rlc_am_send_sdu(rlc_pP,frameP,eNB_flagP);
        }
        //rlc_pP->reassembly_missing_sn_detected = 0;
        break;
      case RLC_FI_1ST_BYTE_DATA_IS_NOT_1ST_BYTE_SDU_LAST_BYTE_DATA_IS_NOT_LAST_BYTE_SDU:
        LOG_D(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][REASSEMBLY PDU] TRY REASSEMBLY PDU FI=00 (11) Li=",
            frameP,
            (rlc_pP->is_enb) ? "eNB" : "UE",
                rlc_pP->enb_module_id,
                rlc_pP->ue_module_id,
                rlc_pP->rb_id);
        for (i=0; i < pdu_info->num_li; i++) {
            LOG_D(RLC, "%d ",pdu_info->li_list[i]);
        }
        LOG_D(RLC, "\n");
        //msg(" remaining size %d\n",size);
        j = 0;
        for (i = 0; i < pdu_info->num_li; i++) {
            rlc_am_reassembly (&pdu_info->payload[j], pdu_info->li_list[i], rlc_pP,frameP);
            rlc_am_send_sdu(rlc_pP,frameP,eNB_flagP);
            j = j + pdu_info->li_list[i];
        }
        if (pdu_info->hidden_size > 0) { // normally should always be > 0 but just for help debug
            // data is already ok, done by last loop above
            rlc_am_reassembly (&pdu_info->payload[j], pdu_info->hidden_size, rlc_pP,frameP);
        } else {
#ifdef USER_MODE
            //assert (5!=5);
#endif
        }
        //rlc_pP->reassembly_missing_sn_detected = 0;
        break;
#ifdef USER_MODE
      default:
        assert(1 != 1);
#endif
      }
  }
  free_mem_block(tb_pP);
}
