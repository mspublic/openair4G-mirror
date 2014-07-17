/*******************************************************************************
    OpenAirInterface
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is
   included in this distribution in the file called "COPYING". If not,
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@eurecom.fr

  Address      : Eurecom, Compus SophiaTech 450, route des chappes, 06451 Biot, France.

 *******************************************************************************/
#define RLC_AM_MODULE
#define RLC_AM_INIT_C
#ifdef USER_MODE
#include <string.h>
#endif
//-----------------------------------------------------------------------------
#include "rlc_am.h"
#include "LAYER2/MAC/extern.h"
#include "UTIL/LOG/log.h"
//-----------------------------------------------------------------------------
void rlc_am_init(rlc_am_entity_t *rlc_pP, frame_t frameP)
//-----------------------------------------------------------------------------
{
    if (rlc_pP->initialized == TRUE) {
        LOG_D(RLC, "[FRAME %5u][RLC_AM][MOD XX][RB XX][INIT] INITIALIZATION ALREADY DONE, DOING NOTHING\n", frameP);
    } else {
        LOG_D(RLC, "[FRAME %5u][RLC_AM][MOD XX][RB XX][INIT] INITIALIZATION: STATE VARIABLES, BUFFERS, LISTS\n", frameP);
        memset(rlc_pP, 0, sizeof(rlc_am_entity_t));

        list2_init(&rlc_pP->receiver_buffer,      "RX BUFFER");
        list_init(&rlc_pP->pdus_to_mac_layer,     "PDUS TO MAC");
        list_init(&rlc_pP->control_pdu_list,      "CONTROL PDU LIST");
        list_init(&rlc_pP->segmentation_pdu_list, "SEGMENTATION PDU LIST");
        //LOG_D(RLC,"RLC_AM_SDU_CONTROL_BUFFER_SIZE %d sizeof(rlc_am_tx_sdu_management_t) %d \n",  RLC_AM_SDU_CONTROL_BUFFER_SIZE, sizeof(rlc_am_tx_sdu_management_t));

        rlc_pP->input_sdus               = calloc(1, RLC_AM_SDU_CONTROL_BUFFER_SIZE*sizeof(rlc_am_tx_sdu_management_t));
        rlc_pP->pdu_retrans_buffer       = calloc(1, (uint16_t)((unsigned int)RLC_AM_PDU_RETRANSMISSION_BUFFER_SIZE*(unsigned int)sizeof(rlc_am_tx_data_pdu_management_t)));
        LOG_D(RLC, "[FRAME %5u][RLC_AM][MOD XX][RB XX][INIT] input_sdus[] = %p  element size=%d\n", frameP, rlc_pP->input_sdus,sizeof(rlc_am_tx_sdu_management_t));
        LOG_D(RLC, "[FRAME %5u][RLC_AM][MOD XX][RB XX][INIT] pdu_retrans_buffer[] = %p element size=%d\n", frameP, rlc_pP->pdu_retrans_buffer,sizeof(rlc_am_tx_data_pdu_management_t));

        // TX state variables
        //rlc_pP->vt_a    = 0;
        rlc_pP->vt_ms   = rlc_pP->vt_a + RLC_AM_WINDOW_SIZE;
        //rlc_pP->vt_s    = 0;
        //rlc_pP->poll_sn = 0;
        // TX counters
        //rlc_pP->c_pdu_without_poll  = 0;
        //rlc_pP->c_byte_without_poll = 0;
        // RX state variables
        //rlc_pP->vr_r    = 0;
        rlc_pP->vr_mr   = rlc_pP->vr_r + RLC_AM_WINDOW_SIZE;
        //rlc_pP->vr_x    = 0;
        //rlc_pP->vr_ms   = 0;
        //rlc_pP->vr_h    = 0;

        rlc_pP->last_frame_status_indication = 123456; // any value > 1
        rlc_pP->first_retrans_pdu_sn         = -1;

        rlc_pP->initialized                  = TRUE;
    }
}
//-----------------------------------------------------------------------------
void rlc_am_reestablish(rlc_am_entity_t *rlc_pP, frame_t frameP)
//-----------------------------------------------------------------------------
{
    /*
     * RLC re-establishment is performed upon request by RRC, and the function
     * is applicable for AM, UM and TM RLC entities.
     * When RRC indicates that an RLC entity should be re-established, the RLC entity shall:
     * - if it is an AM RLC entity:
     *    - when possible, reassemble RLC SDUs from any byte segments of AMD PDUs with SN < VR(MR) in the
     *       receiving side, remove RLC headers when doing so and deliver all reassembled RLC SDUs to upper layer in
     *        ascending order of the RLC SN, if not delivered before;
     *    - discard the remaining AMD PDUs and byte segments of AMD PDUs in the receiving side;
     *    - discard all RLC SDUs and AMD PDUs in the transmitting side;
     *    - discard all RLC control PDUs.
     *    - stop and reset all timers;
     *    - reset all state variables to their initial values.
     */
    LOG_D(RLC, "[FRAME %5u][RLC_AM][MOD XX][RB XX][REESTABLISH] RE-INIT STATE VARIABLES, BUFFERS, LISTS\n", frameP);

#warning TODO when possible reassemble RLC SDUs from any byte segments of AMD PDUs with SN inf VR(MR)
    list2_free(&rlc_pP->receiver_buffer);

    list_free(&rlc_pP->pdus_to_mac_layer);
    list_free(&rlc_pP->control_pdu_list);
    list_free(&rlc_pP->segmentation_pdu_list);


    // TX state variables
    rlc_pP->vt_a    = 0;
    rlc_pP->vt_ms   = rlc_pP->vt_a + RLC_AM_WINDOW_SIZE;
    rlc_pP->vt_s    = 0;
    rlc_pP->poll_sn = 0;

    // TX counters
    rlc_pP->c_pdu_without_poll  = 0;
    rlc_pP->c_byte_without_poll = 0;

    // RX state variables
    rlc_pP->vr_r    = 0;
    rlc_pP->vr_mr   = rlc_pP->vr_r + RLC_AM_WINDOW_SIZE;
    rlc_pP->vr_x    = 0;
    rlc_pP->vr_ms   = 0;
    rlc_pP->vr_h    = 0;

    rlc_pP->last_frame_status_indication = 123456; // any value > 1
    rlc_pP->first_retrans_pdu_sn         = -1;

    rlc_pP->initialized                  = TRUE;

}

//-----------------------------------------------------------------------------
void rlc_am_cleanup(rlc_am_entity_t *rlc_pP)
//-----------------------------------------------------------------------------
{
    LOG_I(RLC, "[FRAME ?????][%s][RLC_AM][MOD %u/%u][RB %u][CLEANUP]\n",
          (rlc_pP->is_enb) ? "eNB" : "UE",
          rlc_pP->enb_module_id,
          rlc_pP->ue_module_id,
          rlc_pP->rb_id);

    list2_free(&rlc_pP->receiver_buffer);
    list_free(&rlc_pP->pdus_to_mac_layer);
    list_free(&rlc_pP->control_pdu_list);
    list_free(&rlc_pP->segmentation_pdu_list);


    if (rlc_pP->output_sdu_in_construction != NULL) {
        free_mem_block(rlc_pP->output_sdu_in_construction);
        rlc_pP->output_sdu_in_construction = NULL;
    }
    unsigned int i;
    if (rlc_pP->input_sdus != NULL) {
        for (i=0; i < RLC_AM_SDU_CONTROL_BUFFER_SIZE; i++) {
            if (rlc_pP->input_sdus[i].mem_block != NULL) {
                free_mem_block(rlc_pP->input_sdus[i].mem_block);
                rlc_pP->input_sdus[i].mem_block = NULL;
            }
        }
        free(rlc_pP->input_sdus);
        rlc_pP->input_sdus       = NULL;
    }
    if (rlc_pP->pdu_retrans_buffer != NULL) {
        for (i=0; i < RLC_AM_PDU_RETRANSMISSION_BUFFER_SIZE; i++) {
            if (rlc_pP->pdu_retrans_buffer[i].mem_block != NULL) {
                free_mem_block(rlc_pP->pdu_retrans_buffer[i].mem_block);
                rlc_pP->pdu_retrans_buffer[i].mem_block = NULL;
            }
        }
        free(rlc_pP->pdu_retrans_buffer);
        rlc_pP->pdu_retrans_buffer       = NULL;
    }
    memset(rlc_pP, 0, sizeof(rlc_am_entity_t));
}
//-----------------------------------------------------------------------------
void rlc_am_configure(rlc_am_entity_t *rlc_pP,
		      frame_t          frameP,
                      uint16_t            max_retx_thresholdP,
                      uint16_t            poll_pduP,
                      uint16_t            poll_byteP,
                      uint32_t            t_poll_retransmitP,
                      uint32_t            t_reorderingP,
                      uint32_t            t_status_prohibitP)
//-----------------------------------------------------------------------------
{
  if (rlc_pP->configured == TRUE) {
      LOG_I(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][RECONFIGURE] max_retx_threshold %d poll_pdu %d poll_byte %d t_poll_retransmit %d t_reordering %d t_status_prohibit %d\n",
          frameP,
          (rlc_pP->is_enb) ? "eNB" : "UE",
          rlc_pP->enb_module_id,
          rlc_pP->ue_module_id,
          rlc_pP->rb_id,
          max_retx_thresholdP,
          poll_pduP,
          poll_byteP,
          t_poll_retransmitP,
          t_reorderingP,
          t_status_prohibitP);

      rlc_pP->max_retx_threshold = max_retx_thresholdP;
      rlc_pP->poll_pdu           = poll_pduP;
      rlc_pP->poll_byte          = poll_byteP;
      rlc_pP->protocol_state     = RLC_DATA_TRANSFER_READY_STATE;

      rlc_pP->t_poll_retransmit.time_out   = t_poll_retransmitP;
      rlc_pP->t_reordering.time_out        = t_reorderingP;
      rlc_pP->t_status_prohibit.time_out   = t_status_prohibitP;
  } else {
      LOG_I(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][CONFIGURE] max_retx_threshold %d poll_pdu %d poll_byte %d t_poll_retransmit %d t_reordering %d t_status_prohibit %d\n",
          frameP,
          (rlc_pP->is_enb) ? "eNB" : "UE",
          rlc_pP->enb_module_id,
          rlc_pP->ue_module_id,
          rlc_pP->rb_id,
          max_retx_thresholdP,
          poll_pduP,
          poll_byteP,
          t_poll_retransmitP,
          t_reorderingP,
          t_status_prohibitP);

      rlc_pP->max_retx_threshold = max_retx_thresholdP;
      rlc_pP->poll_pdu           = poll_pduP;
      rlc_pP->poll_byte          = poll_byteP;
      rlc_pP->protocol_state     = RLC_DATA_TRANSFER_READY_STATE;


      rlc_am_init_timer_poll_retransmit(rlc_pP, t_poll_retransmitP);
      rlc_am_init_timer_reordering     (rlc_pP, t_reorderingP);
      rlc_am_init_timer_status_prohibit(rlc_pP, t_status_prohibitP);

      rlc_pP->configured = TRUE;
  }

}
//-----------------------------------------------------------------------------
void rlc_am_set_debug_infos(rlc_am_entity_t *rlc_pP,
                            frame_t          frameP,
                            eNB_flag_t       eNB_flagP,
                            srb_flag_t       srb_flagP,
                            module_id_t      enb_module_idP,
                            module_id_t      ue_module_idP,
                            rb_id_t          rb_idP)
//-----------------------------------------------------------------------------
{
    LOG_D(RLC, "[FRAME %5u][%s][RLC_AM][MOD %u/%u][RB %u][SET DEBUG INFOS] module_id %d rb_id %d is SRB %d\n",
          frameP,
          (rlc_pP->is_enb) ? "eNB" : "UE",
          rlc_pP->enb_module_id,
          rlc_pP->ue_module_id,
          rb_idP,
          enb_module_idP,
          ue_module_idP,
          rb_idP,
          (srb_flagP) ? "TRUE" : "FALSE");

    rlc_pP->enb_module_id = enb_module_idP;
    rlc_pP->ue_module_id  = ue_module_idP;
    rlc_pP->rb_id         = rb_idP;
    if (srb_flagP) {
      rlc_pP->is_data_plane = 0;
    } else {
      rlc_pP->is_data_plane = 1;
    }
    rlc_pP->is_enb = eNB_flagP;
}
