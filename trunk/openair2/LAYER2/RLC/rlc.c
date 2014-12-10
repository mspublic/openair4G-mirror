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

  Address      : Eurecom, Campus SophiaTech, 450 Route des Chappes, CS 50193 - 06904 Biot Sophia Antipolis cedex, FRANCE

 *******************************************************************************/
/*
                                rlc.c
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier at eurecom dot fr
*/
#define RLC_C
#include "rlc.h"
#include "mem_block.h"
#include "../MAC/extern.h"
#include "UTIL/LOG/log.h"
#include "UTIL/OCG/OCG_vars.h"
#include "UTIL/LOG/vcd_signal_dumper.h"

#include "assertions.h"

extern boolean_t pdcp_data_ind(
    const module_id_t enb_mod_idP,
    const module_id_t ue_mod_idP,
    const frame_t frameP,
    const eNB_flag_t enb_flagP,
    const srb_flag_t srb_flagP,
    const MBMS_flag_t MBMS_flagP,
    const rb_id_t rb_idP,
    const sdu_size_t sdu_buffer_sizeP,
    mem_block_t* const sdu_buffer_pP);

#define DEBUG_RLC_PDCP_INTERFACE 1
//#define DEBUG_RLC_PAYLOAD 1
#define DEBUG_RLC_DATA_REQ 1

#if defined(DEBUG_RLC_PAYLOAD)
//-----------------------------------------------------------------------------
void rlc_util_print_hex_octets(comp_name_t componentP, unsigned char* dataP, const signed long sizeP)
//-----------------------------------------------------------------------------
{
  unsigned long octet_index = 0;

  if (dataP == NULL) {
    return;
  }




  LOG_T(componentP, "+-----+-------------------------------------------------+\n");
  LOG_T(componentP, "|     |  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f |\n");
  LOG_T(componentP, "+-----+-------------------------------------------------+\n");
  for (octet_index = 0; octet_index < sizeP; octet_index++) {
    if ((octet_index % 16) == 0){
      if (octet_index != 0) {
        LOG_T(componentP, " |\n");
      }
      LOG_T(componentP, " %04d |", octet_index);
    }
    /*
     * Print every single octet in hexadecimal form
     */
    LOG_T(componentP, " %02x", dataP[octet_index]);
    /*
     * Align newline and pipes according to the octets in groups of 2
     */
  }

  /*
   * Append enough spaces and put final pipe
   */
  unsigned char index;
  for (index = octet_index; index < 16; ++index)
    LOG_T(componentP, "   ");
  LOG_T(componentP, " |\n");
}
#endif
//-----------------------------------------------------------------------------
rlc_op_status_t rlc_stat_req     (
                  const module_id_t   enb_module_idP,
                  const module_id_t   ue_module_idP,
                  const frame_t       frameP,
                  const eNB_flag_t    enb_flagP,
                  const srb_flag_t    srb_flagP,
                  const rb_id_t       rb_idP,
                  unsigned int* stat_tx_pdcp_sdu,
                  unsigned int* stat_tx_pdcp_bytes,
                  unsigned int* stat_tx_pdcp_sdu_discarded,
                  unsigned int* stat_tx_pdcp_bytes_discarded,
                  unsigned int* stat_tx_data_pdu,
                  unsigned int* stat_tx_data_bytes,
                  unsigned int* stat_tx_retransmit_pdu_by_status,
                  unsigned int* stat_tx_retransmit_bytes_by_status,
                  unsigned int* stat_tx_retransmit_pdu,
                  unsigned int* stat_tx_retransmit_bytes,
                  unsigned int* stat_tx_control_pdu,
                  unsigned int* stat_tx_control_bytes,
                  unsigned int* stat_rx_pdcp_sdu,
                  unsigned int* stat_rx_pdcp_bytes,
                  unsigned int* stat_rx_data_pdus_duplicate,
                  unsigned int* stat_rx_data_bytes_duplicate,
                  unsigned int* stat_rx_data_pdu,
                  unsigned int* stat_rx_data_bytes,
                  unsigned int* stat_rx_data_pdu_dropped,
                  unsigned int* stat_rx_data_bytes_dropped,
                  unsigned int* stat_rx_data_pdu_out_of_window,
                  unsigned int* stat_rx_data_bytes_out_of_window,
                  unsigned int* stat_rx_control_pdu,
                  unsigned int* stat_rx_control_bytes,
                  unsigned int* stat_timer_reordering_timed_out,
                  unsigned int* stat_timer_poll_retransmit_timed_out,
                  unsigned int* stat_timer_status_prohibit_timed_out) {
//-----------------------------------------------------------------------------
    rlc_mode_t             rlc_mode        = RLC_MODE_NONE;
    rlc_union_t           *rlc_union_p     = NULL;
    hash_key_t             key             = HASHTABLE_QUESTIONABLE_KEY_VALUE;
    hashtable_rc_t         h_rc;

#ifdef OAI_EMU
    if (enb_flagP) {
        AssertFatal ((enb_module_idP >= oai_emulation.info.first_enb_local) && (oai_emulation.info.nb_enb_local > 0),
            "eNB module id is too low (%u/%d)!\n",
            enb_module_idP,
            oai_emulation.info.first_enb_local);
        AssertFatal ((enb_module_idP < (oai_emulation.info.first_enb_local + oai_emulation.info.nb_enb_local)) && (oai_emulation.info.nb_enb_local > 0),
            "eNB module id is too high (%u/%d)!\n",
            enb_module_idP,
            oai_emulation.info.first_enb_local + oai_emulation.info.nb_enb_local);
        AssertFatal (ue_module_idP  < NB_UE_INST,
            "UE module id is too high (%u/%d)!\n",
            ue_module_idP,
            oai_emulation.info.first_ue_local + oai_emulation.info.nb_ue_local);
    } else {
        AssertFatal (ue_module_idP  < (oai_emulation.info.first_ue_local + oai_emulation.info.nb_ue_local),
            "UE module id is too high (%u/%d)!\n",
            ue_module_idP,
            oai_emulation.info.first_ue_local + oai_emulation.info.nb_ue_local);
        AssertFatal (ue_module_idP  >= oai_emulation.info.first_ue_local,
            "UE module id is too low (%u/%d)!\n",
            ue_module_idP,
            oai_emulation.info.first_ue_local);
    }
#endif
    AssertFatal (rb_idP < NB_RB_MAX, "RB id is too high (%u/%d)!\n", rb_idP, NB_RB_MAX);
    key = RLC_COLL_KEY_VALUE(enb_module_idP, ue_module_idP, enb_flagP, rb_idP, srb_flagP);
    h_rc = hashtable_get(rlc_coll_p, key, (void**)&rlc_union_p);
    if (h_rc == HASH_TABLE_OK) {
        rlc_mode = rlc_union_p->mode;
    }
    switch (rlc_mode) {
        case RLC_MODE_NONE:
            *stat_tx_pdcp_sdu                     = 0;
            *stat_tx_pdcp_bytes                   = 0;
            *stat_tx_pdcp_sdu_discarded           = 0;
            *stat_tx_pdcp_bytes_discarded         = 0;
            *stat_tx_data_pdu                     = 0;
            *stat_tx_data_bytes                   = 0;
            *stat_tx_retransmit_pdu_by_status     = 0;
            *stat_tx_retransmit_bytes_by_status   = 0;
            *stat_tx_retransmit_pdu               = 0;
            *stat_tx_retransmit_bytes             = 0;
            *stat_tx_control_pdu                  = 0;
            *stat_tx_control_bytes                = 0;
            *stat_rx_pdcp_sdu                     = 0;
            *stat_rx_pdcp_bytes                   = 0;
            *stat_rx_data_pdus_duplicate          = 0;
            *stat_rx_data_bytes_duplicate         = 0;
            *stat_rx_data_pdu                     = 0;
            *stat_rx_data_bytes                   = 0;
            *stat_rx_data_pdu_dropped             = 0;
            *stat_rx_data_bytes_dropped           = 0;
            *stat_rx_data_pdu_out_of_window       = 0;
            *stat_rx_data_bytes_out_of_window     = 0;
            *stat_rx_control_pdu                  = 0;
            *stat_rx_control_bytes                = 0;
            *stat_timer_reordering_timed_out      = 0;
            *stat_timer_poll_retransmit_timed_out = 0;
            *stat_timer_status_prohibit_timed_out = 0;
            return RLC_OP_STATUS_BAD_PARAMETER;
            break;

        case RLC_MODE_AM:
            rlc_am_stat_req(&rlc_union_p->rlc.am,
                            stat_tx_pdcp_sdu,
                            stat_tx_pdcp_bytes,
                            stat_tx_pdcp_sdu_discarded,
                            stat_tx_pdcp_bytes_discarded,
                            stat_tx_data_pdu,
                            stat_tx_data_bytes,
                            stat_tx_retransmit_pdu_by_status,
                            stat_tx_retransmit_bytes_by_status,
                            stat_tx_retransmit_pdu,
                            stat_tx_retransmit_bytes,
                            stat_tx_control_pdu,
                            stat_tx_control_bytes,
                            stat_rx_pdcp_sdu,
                            stat_rx_pdcp_bytes,
                            stat_rx_data_pdus_duplicate,
                            stat_rx_data_bytes_duplicate,
                            stat_rx_data_pdu,
                            stat_rx_data_bytes,
                            stat_rx_data_pdu_dropped,
                            stat_rx_data_bytes_dropped,
                            stat_rx_data_pdu_out_of_window,
                            stat_rx_data_bytes_out_of_window,
                            stat_rx_control_pdu,
                            stat_rx_control_bytes,
                            stat_timer_reordering_timed_out,
                            stat_timer_poll_retransmit_timed_out,
                            stat_timer_status_prohibit_timed_out);
                            return RLC_OP_STATUS_OK;
           break;

       case RLC_MODE_UM:
           *stat_tx_retransmit_pdu_by_status     = 0;
           *stat_tx_retransmit_bytes_by_status   = 0;
           *stat_tx_retransmit_pdu               = 0;
           *stat_tx_retransmit_bytes             = 0;
           *stat_tx_control_pdu                  = 0;
           *stat_tx_control_bytes                = 0;
           *stat_rx_data_pdu_dropped             = 0;
           *stat_rx_data_bytes_dropped           = 0;
           *stat_rx_data_pdu_out_of_window       = 0;
           *stat_rx_data_bytes_out_of_window     = 0;
           *stat_timer_poll_retransmit_timed_out = 0;
           *stat_timer_status_prohibit_timed_out = 0;
           rlc_um_stat_req (&rlc_union_p->rlc.um,
                              stat_tx_pdcp_sdu,
                              stat_tx_pdcp_bytes,
                              stat_tx_pdcp_sdu_discarded,
                              stat_tx_pdcp_bytes_discarded,
                              stat_tx_data_pdu,
                              stat_tx_data_bytes,
                              stat_rx_pdcp_sdu,
                              stat_rx_pdcp_bytes,
                              stat_rx_data_pdus_duplicate,
                              stat_rx_data_bytes_duplicate,
                              stat_rx_data_pdu,
                              stat_rx_data_bytes,
                              stat_rx_data_pdu_dropped,
                              stat_rx_data_bytes_dropped,
                              stat_rx_data_pdu_out_of_window,
                              stat_rx_data_bytes_out_of_window,
                              stat_timer_reordering_timed_out);
           return RLC_OP_STATUS_OK;
           break;

       case RLC_MODE_TM:
           *stat_tx_pdcp_sdu                     = 0;
           *stat_tx_pdcp_bytes                   = 0;
           *stat_tx_pdcp_sdu_discarded           = 0;
           *stat_tx_pdcp_bytes_discarded         = 0;
           *stat_tx_data_pdu                     = 0;
           *stat_tx_data_bytes                   = 0;
           *stat_tx_retransmit_pdu_by_status     = 0;
           *stat_tx_retransmit_bytes_by_status   = 0;
           *stat_tx_retransmit_pdu               = 0;
           *stat_tx_retransmit_bytes             = 0;
           *stat_tx_control_pdu                  = 0;
           *stat_tx_control_bytes                = 0;
           *stat_rx_pdcp_sdu                     = 0;
           *stat_rx_pdcp_bytes                   = 0;
           *stat_rx_data_pdus_duplicate          = 0;
           *stat_rx_data_bytes_duplicate         = 0;
           *stat_rx_data_pdu                     = 0;
           *stat_rx_data_bytes                   = 0;
           *stat_rx_data_pdu_dropped             = 0;
           *stat_rx_data_bytes_dropped           = 0;
           *stat_rx_data_pdu_out_of_window       = 0;
           *stat_rx_data_bytes_out_of_window     = 0;
           *stat_rx_control_pdu                  = 0;
           *stat_rx_control_bytes                = 0;
           *stat_timer_reordering_timed_out      = 0;
           *stat_timer_poll_retransmit_timed_out = 0;
           *stat_timer_status_prohibit_timed_out = 0;
           return RLC_OP_STATUS_BAD_PARAMETER;
           break;

       default:
           *stat_tx_pdcp_sdu                     = 0;
           *stat_tx_pdcp_bytes                   = 0;
           *stat_tx_pdcp_sdu_discarded           = 0;
           *stat_tx_pdcp_bytes_discarded         = 0;
           *stat_tx_data_pdu                     = 0;
           *stat_tx_data_bytes                   = 0;
           *stat_tx_retransmit_pdu_by_status     = 0;
           *stat_tx_retransmit_bytes_by_status   = 0;
           *stat_tx_retransmit_pdu               = 0;
           *stat_tx_retransmit_bytes             = 0;
           *stat_tx_control_pdu                  = 0;
           *stat_tx_control_bytes                = 0;
           *stat_rx_pdcp_sdu                     = 0;
           *stat_rx_pdcp_bytes                   = 0;
           *stat_rx_data_pdus_duplicate          = 0;
           *stat_rx_data_bytes_duplicate         = 0;
           *stat_rx_data_pdu                     = 0;
           *stat_rx_data_bytes                   = 0;
           *stat_rx_data_pdu_dropped             = 0;
           *stat_rx_data_bytes_dropped           = 0;
           *stat_rx_data_pdu_out_of_window       = 0;
           *stat_rx_data_bytes_out_of_window     = 0;
           *stat_rx_control_pdu                  = 0;
           *stat_rx_control_bytes                = 0;

           *stat_timer_poll_retransmit_timed_out = 0;
           *stat_timer_status_prohibit_timed_out = 0;
           return RLC_OP_STATUS_BAD_PARAMETER;
    }
}

//-----------------------------------------------------------------------------
rlc_op_status_t rlc_data_req     (const module_id_t  enb_module_idP,
                                  const module_id_t  ue_module_idP,
                                  const frame_t      frameP,
                                  const eNB_flag_t   enb_flagP,
                                  const srb_flag_t   srb_flagP,
                                  const MBMS_flag_t  MBMS_flagP,
                                  const rb_id_t      rb_idP,
                                  const mui_t        muiP,
                                  confirm_t    confirmP,
                                  sdu_size_t   sdu_sizeP,
                                  mem_block_t *sdu_pP) {
//-----------------------------------------------------------------------------
  mem_block_t           *new_sdu_p    = NULL;
  rlc_mode_t             rlc_mode     = RLC_MODE_NONE;
  rlc_union_t           *rlc_union_p = NULL;
  hash_key_t             key         = HASHTABLE_QUESTIONABLE_KEY_VALUE;
  hashtable_rc_t         h_rc;

#ifdef Rel10
  rlc_mbms_id_t         *mbms_id_p  = NULL;
  logical_chan_id_t      log_ch_id  = 0;
#endif
#ifdef DEBUG_RLC_DATA_REQ
  LOG_D(RLC,"rlc_data_req: %s enb id  %u  ue id %u, rb_id %u (MAX %d), muip %d, confirmP %d, sud_sizeP %d, sdu_pP %p\n",
        (enb_flagP) ? "eNB" : "UE",
        enb_module_idP,
        ue_module_idP,
        rb_idP,
        NB_RAB_MAX,
        muiP,
        confirmP,
        sdu_sizeP,
        sdu_pP);
#endif
#ifdef Rel10
#else
  AssertFatal(MBMS_flagP == 0, "MBMS_flagP %u", MBMS_flagP);
#endif
#ifdef OAI_EMU
  if (enb_flagP) {
      AssertFatal ((enb_module_idP >= oai_emulation.info.first_enb_local) && (oai_emulation.info.nb_enb_local > 0),
          "eNB module id is too low (%u/%d)!\n",
          enb_module_idP,
          oai_emulation.info.first_enb_local);
      AssertFatal ((enb_module_idP < (oai_emulation.info.first_enb_local + oai_emulation.info.nb_enb_local)) && (oai_emulation.info.nb_enb_local > 0),
          "eNB module id is too high (%u/%d)!\n",
          enb_module_idP,
          oai_emulation.info.first_enb_local + oai_emulation.info.nb_enb_local);
      AssertFatal (ue_module_idP  < NB_UE_INST,
          "UE module id is too high (%u/%d)!\n",
          ue_module_idP,
          oai_emulation.info.first_ue_local + oai_emulation.info.nb_ue_local);
  } else {
      AssertFatal (ue_module_idP  < (oai_emulation.info.first_ue_local + oai_emulation.info.nb_ue_local),
          "UE module id is too high (%u/%d)!\n",
          ue_module_idP,
          oai_emulation.info.first_ue_local + oai_emulation.info.nb_ue_local);
      AssertFatal (ue_module_idP  >= oai_emulation.info.first_ue_local,
          "UE module id is too low (%u/%d)!\n",
          ue_module_idP,
          oai_emulation.info.first_ue_local);
  }
#endif
  if (MBMS_flagP) {
      AssertFatal (rb_idP < NB_RB_MBMS_MAX, "RB id is too high (%u/%d)!\n", rb_idP, NB_RB_MBMS_MAX);
  } else {
      AssertFatal (rb_idP < NB_RB_MAX, "RB id is too high (%u/%d)!\n", rb_idP, NB_RB_MAX);
  }
  DevAssert(sdu_pP != NULL);
  DevCheck(sdu_sizeP > 0, sdu_sizeP, 0, 0);

#ifndef Rel10
  DevCheck(MBMS_flagP == 0, MBMS_flagP, 0, 0);
#endif

  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_RLC_DATA_REQ,VCD_FUNCTION_IN);

#ifdef Rel10
  if (MBMS_flagP == TRUE) {
      if (enb_flagP) {
            log_ch_id = rlc_mbms_enb_get_lcid_by_rb_id(enb_module_idP,rb_idP);
            mbms_id_p = &rlc_mbms_lcid2service_session_id_eNB[enb_module_idP][log_ch_id];
      } else {
            log_ch_id = rlc_mbms_ue_get_lcid_by_rb_id(ue_module_idP,rb_idP);
            mbms_id_p = &rlc_mbms_lcid2service_session_id_ue[ue_module_idP][log_ch_id];
      }
      key = RLC_COLL_KEY_MBMS_VALUE(enb_module_idP, ue_module_idP, enb_flagP, mbms_id_p->service_id, mbms_id_p->session_id);
  } else
#endif
  {
      key = RLC_COLL_KEY_VALUE(enb_module_idP, ue_module_idP, enb_flagP, rb_idP, srb_flagP);
  }

  h_rc = hashtable_get(rlc_coll_p, key, (void**)&rlc_union_p);
  if (h_rc == HASH_TABLE_OK) {
      rlc_mode = rlc_union_p->mode;
  } else {
      rlc_mode = RLC_MODE_NONE;
      AssertFatal (0 , "RLC not configured key %ju\n", key);
  }

  if (MBMS_flagP == 0) {
      LOG_D(RLC, "[FRAME %5u][%s][RLC][INST %u/%u][RB %u] Display of rlc_data_req:\n",
          frameP,
          (enb_flagP) ? "eNB" : "UE",
          enb_module_idP,
          ue_module_idP,
          rb_idP);
#if defined(DEBUG_RLC_PAYLOAD)
      rlc_util_print_hex_octets(RLC, (unsigned char*)sdu_pP->data, sdu_sizeP);
#endif

#ifdef DEBUG_RLC_DATA_REQ
      LOG_D(RLC,"RLC_TYPE : %d ",rlc_mode);
#endif
      switch (rlc_mode) {
          case RLC_MODE_NONE:
              free_mem_block(sdu_pP);
              LOG_E(RLC, "Received RLC_MODE_NONE as rlc_type for %s eNB id  %u, ue id %u, rb_id %u\n",
                    (enb_flagP) ? "eNB" : "UE",
                    enb_module_idP,
                    ue_module_idP,
                    rb_idP);
	      vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_RLC_DATA_REQ,VCD_FUNCTION_OUT);
              return RLC_OP_STATUS_BAD_PARAMETER;

          case RLC_MODE_AM:
#ifdef DEBUG_RLC_DATA_REQ
              msg("RLC_MODE_AM\n");
#endif
              new_sdu_p = get_free_mem_block (sdu_sizeP + sizeof (struct rlc_am_data_req_alloc));

              if (new_sdu_p != NULL) {
                  // PROCESS OF COMPRESSION HERE:
                  memset (new_sdu_p->data, 0, sizeof (struct rlc_am_data_req_alloc));
                  memcpy (&new_sdu_p->data[sizeof (struct rlc_am_data_req_alloc)], &sdu_pP->data[0], sdu_sizeP);

                  ((struct rlc_am_data_req *) (new_sdu_p->data))->data_size = sdu_sizeP;
                  ((struct rlc_am_data_req *) (new_sdu_p->data))->conf = confirmP;
                  ((struct rlc_am_data_req *) (new_sdu_p->data))->mui  = muiP;
                  ((struct rlc_am_data_req *) (new_sdu_p->data))->data_offset = sizeof (struct rlc_am_data_req_alloc);
                  free_mem_block(sdu_pP);
                  LOG_D(RLC, "%s\n",RLC_FG_BRIGHT_COLOR_RED);

                  LOG_D(RLC, "[FRAME %5u][%s][%s][INST %u/%u][%s %u][--- RLC_AM_DATA_REQ/%d Bytes --->][RLC_AM][INST %u/%u][%s %u]\n",
                          frameP,
                          (enb_flagP) ? "eNB" : "UE",
                          (srb_flagP) ? "RRC" : "PDCP",
                          enb_module_idP,
                          ue_module_idP,
                          (srb_flagP) ? "SRB" : "DRB",
                          rb_idP,
                          sdu_sizeP,
                          enb_module_idP,
                          ue_module_idP,
                          (srb_flagP) ? "SRB" : "DRB",
                          rb_idP);
                  LOG_D(RLC, "%s\n",RLC_FG_COLOR_DEFAULT);
                  rlc_am_data_req(&rlc_union_p->rlc.am, frameP, new_sdu_p);
		  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_RLC_DATA_REQ,VCD_FUNCTION_OUT);
                  return RLC_OP_STATUS_OK;
              } else {
		vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_RLC_DATA_REQ,VCD_FUNCTION_OUT);
                  return RLC_OP_STATUS_INTERNAL_ERROR;
              }
              break;

          case RLC_MODE_UM:
            new_sdu_p = get_free_mem_block (sdu_sizeP + sizeof (struct rlc_um_data_req_alloc));

              if (new_sdu_p != NULL) {
                  // PROCESS OF COMPRESSION HERE:
                  memset (new_sdu_p->data, 0, sizeof (struct rlc_um_data_req_alloc));
                  memcpy (&new_sdu_p->data[sizeof (struct rlc_um_data_req_alloc)], &sdu_pP->data[0], sdu_sizeP);

                  ((struct rlc_um_data_req *) (new_sdu_p->data))->data_size = sdu_sizeP;
                  ((struct rlc_um_data_req *) (new_sdu_p->data))->data_offset = sizeof (struct rlc_um_data_req_alloc);
                  free_mem_block(sdu_pP);

                  //LOG_D(RLC, "%s\n",RLC_FG_BRIGHT_COLOR_RED);
                  LOG_D(RLC, "[FRAME %5u][%s][%s][INST %u/%u][%s %u][--- RLC_UM_DATA_REQ/%d Bytes --->][RLC_UM][INST %u/%u][%s %u]\n",
                          frameP,
                          (enb_flagP) ? "eNB" : "UE",
                          (srb_flagP) ? "RRC" : "PDCP",
                          enb_module_idP,
                          ue_module_idP,
                          (srb_flagP) ? "SRB" : "DRB",
                          rb_idP,
                          sdu_sizeP,
                          enb_module_idP,
                          ue_module_idP,
                          (srb_flagP) ? "SRB" : "DRB",
                          rb_idP);
                  //LOG_D(RLC, "%s\n",RLC_FG_COLOR_DEFAULT);
                  rlc_um_data_req(&rlc_union_p->rlc.um, frameP, new_sdu_p);

                  //free_mem_block(new_sdu);
		  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_RLC_DATA_REQ,VCD_FUNCTION_OUT);
                  return RLC_OP_STATUS_OK;
              } else {
		vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_RLC_DATA_REQ,VCD_FUNCTION_OUT);
		return RLC_OP_STATUS_INTERNAL_ERROR;
              }
              break;

          case RLC_MODE_TM:
            new_sdu_p = get_free_mem_block (sdu_sizeP + sizeof (struct rlc_tm_data_req_alloc));

              if (new_sdu_p != NULL) {
                  // PROCESS OF COMPRESSION HERE:
                  memset (new_sdu_p->data, 0, sizeof (struct rlc_tm_data_req_alloc));
                  memcpy (&new_sdu_p->data[sizeof (struct rlc_tm_data_req_alloc)], &sdu_pP->data[0], sdu_sizeP);

                  ((struct rlc_tm_data_req *) (new_sdu_p->data))->data_size = sdu_sizeP;
                  ((struct rlc_tm_data_req *) (new_sdu_p->data))->data_offset = sizeof (struct rlc_tm_data_req_alloc);
                  free_mem_block(sdu_pP);
                  LOG_D(RLC, "%s\n",RLC_FG_BRIGHT_COLOR_RED);
                  LOG_D(RLC, "[FRAME %5u][%s][%s][INST %u/%u][%s %u][--- RLC_TM_DATA_REQ/%d Bytes --->][RLC_TM][INST %u/%u][%s %u]\n",
                                 frameP,
                                 (enb_flagP) ? "eNB" : "UE",
                                 (srb_flagP) ? "RRC" : "PDCP",
                                 enb_module_idP,
                                 ue_module_idP,
                                 (srb_flagP) ? "SRB" : "DRB",
                                 rb_idP,
                                 sdu_sizeP,
                                 enb_module_idP,
                                 ue_module_idP,
                                 (srb_flagP) ? "SRB" : "DRB",
                                 rb_idP);
                  LOG_D(RLC, "%s\n",RLC_FG_COLOR_DEFAULT);
                  rlc_tm_data_req(&rlc_union_p->rlc.tm, new_sdu_p);
		  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_RLC_DATA_REQ,VCD_FUNCTION_OUT);
                  return RLC_OP_STATUS_OK;
              } else {
                  //handle_event(ERROR,"FILE %s FONCTION rlc_data_req() LINE %s : out of memory\n", __FILE__, __LINE__);
		vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_RLC_DATA_REQ,VCD_FUNCTION_OUT);
		return RLC_OP_STATUS_INTERNAL_ERROR;
              }
              break;

          default:
              free_mem_block(sdu_pP);
	      vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_RLC_DATA_REQ,VCD_FUNCTION_OUT);
              return RLC_OP_STATUS_INTERNAL_ERROR;

      }

#ifdef Rel10
  } else { /* MBMS_flag != 0 */
	  //  LOG_I(RLC,"DUY rlc_data_req: mbms_rb_id in RLC instant is: %d\n", mbms_rb_id);
          if (sdu_pP != NULL) {
              if (sdu_sizeP > 0) {
                  LOG_I(RLC,"received a packet with size %d for MBMS \n", sdu_sizeP);
                  new_sdu_p = get_free_mem_block (sdu_sizeP + sizeof (struct rlc_um_data_req_alloc));
                  if (new_sdu_p != NULL) {
                      // PROCESS OF COMPRESSION HERE:
                      memset (new_sdu_p->data, 0, sizeof (struct rlc_um_data_req_alloc));
                      memcpy (&new_sdu_p->data[sizeof (struct rlc_um_data_req_alloc)], &sdu_pP->data[0], sdu_sizeP);
                      ((struct rlc_um_data_req *) (new_sdu_p->data))->data_size = sdu_sizeP;
                      ((struct rlc_um_data_req *) (new_sdu_p->data))->data_offset = sizeof (struct rlc_um_data_req_alloc);
                      free_mem_block(sdu_pP);
                      LOG_D(RLC, "%s\n",RLC_FG_BRIGHT_COLOR_RED);
                          LOG_D(RLC, "[FRAME %5u][%s][%s][INST %u/%u][RB %u][--- RLC_UM_DATA_REQ/%d Bytes (MBMS) --->][RLC_UM][INST %u/%u][RB %u]\n",
                            frameP,
                            (enb_flagP) ? "eNB" : "UE",
                            (srb_flagP) ? "RRC" : "PDCP",
                            enb_module_idP,
                            ue_module_idP,
                            rb_idP,
                            sdu_sizeP,
                            enb_module_idP,
                            ue_module_idP,
                            rb_idP);
                      LOG_D(RLC, "%s\n",RLC_FG_COLOR_DEFAULT);
                      rlc_um_data_req(&rlc_union_p->rlc.um, frameP, new_sdu_p);

                      //free_mem_block(new_sdu);
		      vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_RLC_DATA_REQ,VCD_FUNCTION_OUT);
                      return RLC_OP_STATUS_OK;
                  } else {
		    vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_RLC_DATA_REQ,VCD_FUNCTION_OUT);
                      return RLC_OP_STATUS_BAD_PARAMETER;
                  }
              } else {
		vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_RLC_DATA_REQ,VCD_FUNCTION_OUT);
                  return RLC_OP_STATUS_BAD_PARAMETER;
              }
          } else {
	    vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_RLC_DATA_REQ,VCD_FUNCTION_OUT);
              return RLC_OP_STATUS_BAD_PARAMETER;
          }
  }
#else
  } else {/* MBMS_flag != 0 */
    free_mem_block(sdu_pP);
    LOG_E(RLC, "MBMS_flag != 0 while Rel10 is not defined...\n");
    //handle_event(ERROR,"FILE %s FONCTION rlc_data_req() LINE %s : parameter module_id out of bounds :%d\n", __FILE__, __LINE__, module_idP);
    vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_RLC_DATA_REQ,VCD_FUNCTION_OUT);
    return RLC_OP_STATUS_BAD_PARAMETER;
  }
#endif
}

//-----------------------------------------------------------------------------
void rlc_data_ind     (
    const module_id_t enb_module_idP,
    const module_id_t ue_module_idP,
    const frame_t     frameP,
    const eNB_flag_t  enb_flagP,
    const srb_flag_t  srb_flagP,
    const MBMS_flag_t MBMS_flagP,
    const rb_id_t     rb_idP,
    const sdu_size_t  sdu_sizeP,
    mem_block_t      *sdu_pP) {
//-----------------------------------------------------------------------------


#if defined(DEBUG_RLC_PAYLOAD)
  LOG_D(RLC, "[FRAME %5u][%s][RLC][INST %u/%u][%s %u] Display of rlc_data_ind: size %u\n",
        frameP,
        (enb_flagP) ? "eNB" : "UE",
        enb_module_idP,
        ue_module_idP,
        (srb_flagP) ? "SRB" : "DRB",
        rb_idP,
        sdu_sizeP);

  rlc_util_print_hex_octets(RLC, (unsigned char*)sdu_pP->data, sdu_sizeP);
#endif
  LOG_D(RLC, "[FRAME %5u][%s][RLC][INST %u/%u][%s %u][--- RLC_DATA_IND/%d Bytes --->][PDCP][INST %u/%u][%s %u]\n",
      frameP,
      (enb_flagP) ? "eNB" : "UE",
      enb_module_idP,
      ue_module_idP,
      (srb_flagP) ? "SRB" : "DRB",
      rb_idP,
      sdu_sizeP,
      enb_module_idP,
      ue_module_idP,
      (srb_flagP) ? "SRB" : "DRB",
      rb_idP);

  pdcp_data_ind (
      enb_module_idP,
      ue_module_idP,
      frameP,
      enb_flagP,
      srb_flagP,
      MBMS_flagP,
      rb_idP,
      sdu_sizeP,
      sdu_pP);
}
//-----------------------------------------------------------------------------
void rlc_data_conf     (const module_id_t     enb_module_idP,
                        const module_id_t     ue_module_idP,
                        const frame_t         frameP,
                        const eNB_flag_t      enb_flagP,
                        const srb_flag_t      srb_flagP,
                        const rb_id_t         rb_idP,
                        const mui_t           muiP,
                        const rlc_tx_status_t statusP) {
//-----------------------------------------------------------------------------

    if (srb_flagP) {
        if (rlc_rrc_data_conf != NULL) {
            LOG_D(RLC, "%s\n",RLC_FG_BRIGHT_COLOR_RED);
            LOG_D(RLC, "[FRAME %5u][%s][RLC_AM][INST %u/%u][%s %u][--- RLC_DATA_CONF /MUI %d --->][%s][INST %u/%u][][RLC_DATA_CONF/ MUI %d]\n",
                frameP,
                (enb_flagP) ? "eNB" : "UE",
                enb_module_idP,
                ue_module_idP,
                (srb_flagP) ? "SRB" : "DRB",
                rb_idP,
                muiP,
                (srb_flagP) ? "RRC" : "PDCP",
                enb_module_idP,
                ue_module_idP,
                muiP);

            LOG_D(RLC, "%s\n",RLC_FG_COLOR_DEFAULT);
            rlc_rrc_data_conf (enb_module_idP , ue_module_idP, enb_flagP, rb_idP , muiP, statusP);
        }
    }
}
//-----------------------------------------------------------------------------
int
rlc_module_init (void)
{
//-----------------------------------------------------------------------------
   int          k;
   module_id_t  module_id1;

   LOG_D(RLC, "MODULE INIT\n");
   rlc_rrc_data_ind  = NULL;
   rlc_rrc_data_conf = NULL;

   rlc_coll_p = hashtable_create ((maxDRB + 2) * 16, NULL, rb_free_rlc_union);
   AssertFatal(rlc_coll_p != NULL, "UNRECOVERABLE error, RLC hashtable_create failed");

   for (module_id1=0; module_id1 < NUMBER_OF_UE_MAX; module_id1++) {
#if defined(Rel10)
      for (k=0; k < RLC_MAX_MBMS_LC; k++) {
          rlc_mbms_lcid2service_session_id_ue[module_id1][k].service_id = 0;
          rlc_mbms_lcid2service_session_id_ue[module_id1][k].session_id = 0;
      }
      for (k=0; k < NB_RB_MBMS_MAX; k++) {
          rlc_mbms_rbid2lcid_eNB[module_id1][k] = RLC_LC_UNALLOCATED;
      }
#endif
   }

   for (module_id1=0; module_id1 < NUMBER_OF_eNB_MAX; module_id1++) {
#if defined(Rel10)
      for (k=0; k < RLC_MAX_MBMS_LC; k++) {
          rlc_mbms_lcid2service_session_id_eNB[module_id1][k].service_id = 0;
          rlc_mbms_lcid2service_session_id_eNB[module_id1][k].session_id = 0;
      }
      for (k=0; k < NB_RB_MBMS_MAX; k++) {
          rlc_mbms_rbid2lcid_ue[module_id1][k] = RLC_LC_UNALLOCATED;
      }
#endif
   }

   pool_buffer_init();

   return(0);
}
//-----------------------------------------------------------------------------
void
rlc_module_cleanup (void)
//-----------------------------------------------------------------------------
{
    hashtable_destroy(rlc_coll_p);
}
//-----------------------------------------------------------------------------
void
rlc_layer_init (void)
{
//-----------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------
void
rlc_layer_cleanup (void)
//-----------------------------------------------------------------------------
{
}

