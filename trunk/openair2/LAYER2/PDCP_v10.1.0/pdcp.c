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

/*! \file pdcp.c
 * \brief pdcp interface with RLC
 * \author  Lionel GAUTHIER and Navid Nikaein
 * \date 2009-2012
 * \version 1.0
 */

#define PDCP_C
#ifndef USER_MODE
#include <rtai_fifos.h>
#endif
#include "assertions.h"
#include "pdcp.h"
#include "pdcp_util.h"
#include "pdcp_sequence_manager.h"
#include "LAYER2/RLC/rlc.h"
#include "LAYER2/MAC/extern.h"
#include "RRC/L2_INTERFACE/openair_rrc_L2_interface.h"
#include "pdcp_primitives.h"
#include "OCG.h"
#include "OCG_extern.h"
#include "UTIL/LOG/log.h"
#include <inttypes.h>
#include "platform_constants.h"
#include "UTIL/LOG/vcd_signal_dumper.h"

#if defined(ENABLE_SECURITY)
# include "UTIL/OSA/osa_defs.h"
#endif

#if defined(ENABLE_ITTI)
# include "intertask_interface.h"
#endif

#ifndef OAI_EMU
extern int otg_enabled;
#endif

//extern char *packet_gen(int src, int dst, int ctime, int *pkt_size);
extern int otg_rx_pkt( int src, int dst, int ctime, char *buffer_tx, unsigned int size);

//-----------------------------------------------------------------------------
/*
 * If PDCP_UNIT_TEST is set here then data flow between PDCP and RLC is broken
 * and PDCP has no longer anything to do with RLC. In this case, after it's handed
 * an SDU it appends PDCP header and returns (by filling in incoming pointer parameters)
 * this mem_block_t to be dissected for testing purposes. For further details see test
 * code at targets/TEST/PDCP/test_pdcp.c:test_pdcp_data_req()
 */
boolean_t pdcp_data_req(
        const module_id_t    enb_mod_idP,
        const module_id_t    ue_mod_idP,
        const frame_t        frameP,
        const eNB_flag_t     enb_flagP,
        const srb_flag_t     srb_flagP,
        const rb_id_t        rb_idP,
        const mui_t          muiP,
        const confirm_t      confirmP,
        const sdu_size_t     sdu_buffer_sizeP,
        unsigned char *const sdu_buffer_pP,
        const pdcp_transmission_mode_t modeP)
{
  //-----------------------------------------------------------------------------
  pdcp_t            *pdcp_p          = NULL;
  uint8_t            i               = 0;
  uint8_t            pdcp_header_len = 0;
  uint8_t            pdcp_tailer_len = 0;
  uint16_t           pdcp_pdu_size   = 0;
  uint16_t           current_sn      = 0;
  mem_block_t       *pdcp_pdu_p      = NULL;
  rlc_op_status_t    rlc_status;
  boolean_t          ret             = TRUE;

  AssertError (enb_mod_idP < NUMBER_OF_eNB_MAX, return FALSE, "eNB id is too high (%u/%d) %u %u!\n", enb_mod_idP, NUMBER_OF_eNB_MAX, ue_mod_idP, rb_idP);
  AssertError (ue_mod_idP < NUMBER_OF_UE_MAX, return FALSE, "UE id is too high (%u/%d) %u %u!\n", ue_mod_idP, NUMBER_OF_UE_MAX, enb_mod_idP, rb_idP);
  if (modeP == PDCP_TRANSMISSION_MODE_TRANSPARENT) {
      AssertError (rb_idP < NB_RB_MBMS_MAX, return FALSE, "RB id is too high (%u/%d) %u %u!\n", rb_idP, NB_RB_MBMS_MAX, ue_mod_idP, enb_mod_idP);
  } else {
      if (srb_flagP) {
          AssertError (rb_idP < 2, return FALSE, "RB id is too high (%u/%d) %u %u!\n", rb_idP, 2, ue_mod_idP, enb_mod_idP);
      } else {
          AssertError (rb_idP < maxDRB, return FALSE, "RB id is too high (%u/%d) %u %u!\n", rb_idP, maxDRB, ue_mod_idP, enb_mod_idP);
      }
  }

  if (enb_flagP == ENB_FLAG_NO) {
      if (srb_flagP) {
          pdcp_p = &pdcp_array_srb_ue[ue_mod_idP][rb_idP-1];
  } else {
          pdcp_p = &pdcp_array_drb_ue[ue_mod_idP][rb_idP-1];
      }
  } else {
      if (srb_flagP) {
          pdcp_p = &pdcp_array_srb_eNB[enb_mod_idP][ue_mod_idP][rb_idP-1];
      } else {
          pdcp_p = &pdcp_array_drb_eNB[enb_mod_idP][ue_mod_idP][rb_idP-1];
      }
  }

  if ((pdcp_p->instanciated_instance == FALSE) && (modeP != PDCP_TRANSMISSION_MODE_TRANSPARENT)) {
      if (enb_flagP == ENB_FLAG_NO) {
          LOG_W(PDCP, "[UE %d] Instance is not configured for eNB %d, rb_id %d Ignoring SDU...\n",
              ue_mod_idP, enb_mod_idP, rb_idP);
      } else {
          LOG_W(PDCP, "[eNB %d] Instance is not configured for UE %d, rb_id %d Ignoring SDU...\n",
              enb_mod_idP, ue_mod_idP, rb_idP);
      }
      return FALSE;
  }
  if (sdu_buffer_sizeP == 0) {
      LOG_W(PDCP, "Handed SDU is of size 0! Ignoring...\n");
      return FALSE;
  }
  /*
   * XXX MAX_IP_PACKET_SIZE is 4096, shouldn't this be MAX SDU size, which is 8188 bytes?
   */

  if (sdu_buffer_sizeP > MAX_IP_PACKET_SIZE) {
      LOG_E(PDCP, "Requested SDU size (%d) is bigger than that can be handled by PDCP (%u)!\n",
          sdu_buffer_sizeP, MAX_IP_PACKET_SIZE);
      // XXX What does following call do?
      mac_xface->macphy_exit("PDCP sdu buffer size > MAX_IP_PACKET_SIZE");
  }

  if (enb_flagP == ENB_FLAG_NO)
    start_meas(&eNB_pdcp_stats[enb_mod_idP].data_req);
  else
    start_meas(&UE_pdcp_stats[ue_mod_idP].data_req);
 
  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PDCP_DATA_REQ,VCD_FUNCTION_IN);
   
  // PDCP transparent mode for MBMS traffic

  if (modeP == PDCP_TRANSMISSION_MODE_TRANSPARENT) {
      LOG_D(PDCP, " [TM] Asking for a new mem_block of size %d\n",sdu_buffer_sizeP);
      pdcp_pdu_p = get_free_mem_block(sdu_buffer_sizeP);
      if (pdcp_pdu_p != NULL) {
          memcpy(&pdcp_pdu_p->data[0], sdu_buffer_pP, sdu_buffer_sizeP);
          rlc_util_print_hex_octets(PDCP,
                                    (unsigned char*)&pdcp_pdu_p->data[0],
                                    sdu_buffer_sizeP);

          rlc_status = rlc_data_req(enb_mod_idP, ue_mod_idP, frameP, enb_flagP, srb_flagP, MBMS_FLAG_YES, rb_idP, muiP, confirmP, sdu_buffer_sizeP, pdcp_pdu_p);
      } else {
        rlc_status = RLC_OP_STATUS_OUT_OF_RESSOURCES;
	LOG_W(PDCP,"[FRAME %5u][%s][PDCP][MOD %u/%u][RB %u] PDCP_DATA_REQ SDU DROPPED, OUT OF MEMORY \n",
	      frameP,
	      (enb_flagP) ? "eNB" : "UE",
	      enb_mod_idP,
	      ue_mod_idP,
	      rb_idP);
#if defined(STOP_ON_IP_TRAFFIC_OVERLOAD)
        AssertFatal(0, "[FRAME %5u][%s][PDCP][MOD %u/%u][RB %u] PDCP_DATA_REQ SDU DROPPED, OUT OF MEMORY \n",
            frameP,
            (enb_flagP) ? "eNB" : "UE",
            enb_mod_idP,
            ue_mod_idP,
            rb_idP);
#endif
      }
  } else {
      // calculate the pdcp header and trailer size
      if (srb_flagP) {
          pdcp_header_len = PDCP_CONTROL_PLANE_DATA_PDU_SN_SIZE;
          pdcp_tailer_len = PDCP_CONTROL_PLANE_DATA_PDU_MAC_I_SIZE;
      } else {
          pdcp_header_len = PDCP_USER_PLANE_DATA_PDU_LONG_SN_HEADER_SIZE;
          pdcp_tailer_len = 0;
      }
      pdcp_pdu_size = sdu_buffer_sizeP + pdcp_header_len + pdcp_tailer_len;

      LOG_D(PDCP, "Data request notification for PDCP entity %s enb id %u ue_id %u and radio bearer ID %d pdu size %d (header%d, trailer%d)\n",
          (enb_flagP) ? "eNB" : "UE",
              enb_mod_idP,
              ue_mod_idP,
              rb_idP,
              pdcp_pdu_size,
              pdcp_header_len,
              pdcp_tailer_len);

      /*
       * Allocate a new block for the new PDU (i.e. PDU header and SDU payload)
       */
      LOG_D(PDCP, "Asking for a new mem_block of size %d\n", pdcp_pdu_size);
      pdcp_pdu_p = get_free_mem_block(pdcp_pdu_size);

      if (pdcp_pdu_p != NULL) {
          /*
           * Create a Data PDU with header and append data
           *
           * Place User Plane PDCP Data PDU header first
           */

          if (srb_flagP) { // this Control plane PDCP Data PDU
              pdcp_control_plane_data_pdu_header pdu_header;
              pdu_header.sn = pdcp_get_next_tx_seq_number(pdcp_p);
              current_sn = pdu_header.sn;
              memset(&pdu_header.mac_i[0],0,PDCP_CONTROL_PLANE_DATA_PDU_MAC_I_SIZE);
              memset(&pdcp_pdu_p->data[sdu_buffer_sizeP + pdcp_header_len],0,PDCP_CONTROL_PLANE_DATA_PDU_MAC_I_SIZE);
              if (pdcp_serialize_control_plane_data_pdu_with_SRB_sn_buffer((unsigned char*)pdcp_pdu_p->data, &pdu_header) == FALSE) {
                  LOG_E(PDCP, "Cannot fill PDU buffer with relevant header fields!\n");
                  if (enb_flagP == ENB_FLAG_NO)
                    stop_meas(&eNB_pdcp_stats[enb_mod_idP].data_req);
                  else
                    stop_meas(&UE_pdcp_stats[ue_mod_idP].data_req);
		  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PDCP_DATA_REQ,VCD_FUNCTION_OUT);
                  return FALSE;
              }
          } else {
              pdcp_user_plane_data_pdu_header_with_long_sn pdu_header;
              pdu_header.dc = (modeP == PDCP_TRANSMISSION_MODE_DATA) ? PDCP_DATA_PDU_BIT_SET :  PDCP_CONTROL_PDU_BIT_SET;
              pdu_header.sn = pdcp_get_next_tx_seq_number(pdcp_p);
              current_sn = pdu_header.sn ;
              if (pdcp_serialize_user_plane_data_pdu_with_long_sn_buffer((unsigned char*)pdcp_pdu_p->data, &pdu_header) == FALSE) {
                  LOG_E(PDCP, "Cannot fill PDU buffer with relevant header fields!\n");
                  if (enb_flagP == ENB_FLAG_NO)
                    stop_meas(&eNB_pdcp_stats[enb_mod_idP].data_req);
                  else
                    stop_meas(&UE_pdcp_stats[ue_mod_idP].data_req);
		  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PDCP_DATA_REQ,VCD_FUNCTION_OUT);
                  return FALSE;
              }
          }
          /*
           * Validate incoming sequence number, there might be a problem with PDCP initialization
           */
          if (current_sn > pdcp_calculate_max_seq_num_for_given_size(pdcp_p->seq_num_size)) {
              LOG_E(PDCP, "Generated sequence number (%lu) is greater than a sequence number could ever be!\n", current_sn);
              LOG_E(PDCP, "There must be a problem with PDCP initialization, ignoring this PDU...\n");

              free_mem_block(pdcp_pdu_p);
              if (enb_flagP == ENB_FLAG_NO)
                stop_meas(&eNB_pdcp_stats[enb_mod_idP].data_req);
              else
                stop_meas(&UE_pdcp_stats[ue_mod_idP].data_req);
	      vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PDCP_DATA_REQ,VCD_FUNCTION_OUT);
              return FALSE;
          }

          LOG_D(PDCP, "Sequence number %d is assigned to current PDU\n", current_sn);

          /* Then append data... */
          memcpy(&pdcp_pdu_p->data[pdcp_header_len], sdu_buffer_pP, sdu_buffer_sizeP);

          //For control plane data that are not integrity protected,
          // the MAC-I field is still present and should be padded with padding bits set to 0.
          // NOTE: user-plane data are never integrity protected
          for (i=0;i<pdcp_tailer_len;i++)
            pdcp_pdu_p->data[pdcp_header_len + sdu_buffer_sizeP + i] = 0x00;// pdu_header.mac_i[i];

#if defined(ENABLE_SECURITY)
          if ((pdcp_p->security_activated != 0) &&
              (((pdcp_p->cipheringAlgorithm) != 0) ||
              ((pdcp_p->integrityProtAlgorithm) != 0))) {

              if (enb_flagP == ENB_FLAG_NO)
                start_meas(&eNB_pdcp_stats[enb_mod_idP].apply_security);
              else
                start_meas(&UE_pdcp_stats[ue_mod_idP].apply_security);

              pdcp_apply_security(pdcp_p,
                      srb_flagP,
                      rb_idP % maxDRB,
                      pdcp_header_len,
                      current_sn,
                      pdcp_pdu_p->data,
                      sdu_buffer_sizeP);

              if (enb_flagP == ENB_FLAG_NO)
                stop_meas(&eNB_pdcp_stats[enb_mod_idP].apply_security);
              else
                stop_meas(&UE_pdcp_stats[ue_mod_idP].apply_security);
          }

#endif

          /* Print octets of outgoing data in hexadecimal form */
          LOG_D(PDCP, "Following content with size %d will be sent over RLC (PDCP PDU header is the first two bytes)\n",
              pdcp_pdu_size);
          //util_print_hex_octets(PDCP, (unsigned char*)pdcp_pdu_p->data, pdcp_pdu_size);
          //util_flush_hex_octets(PDCP, (unsigned char*)pdcp_pdu->data, pdcp_pdu_size);
      } else {
          LOG_E(PDCP, "Cannot create a mem_block for a PDU!\n");
          if (enb_flagP == ENB_FLAG_NO)
              stop_meas(&eNB_pdcp_stats[enb_mod_idP].data_req);
          else
              stop_meas(&UE_pdcp_stats[ue_mod_idP].data_req);
#if defined(STOP_ON_IP_TRAFFIC_OVERLOAD)
        AssertFatal(0, "[FRAME %5u][%s][PDCP][MOD %u/%u][RB %u] PDCP_DATA_REQ SDU DROPPED, OUT OF MEMORY \n",
            frameP,
            (enb_flagP) ? "eNB" : "UE",
            enb_mod_idP,
            ue_mod_idP,
            rb_idP);
#endif
	vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PDCP_DATA_REQ,VCD_FUNCTION_OUT);
	return FALSE;
      }
      /*
       * Ask sublayer to transmit data and check return value
       * to see if RLC succeeded
       */
#ifdef PDCP_MSG_PRINT
      int i=0;
      LOG_F(PDCP,"[MSG] PDCP DL %s PDU on rb_id %d\n", (srb_flagP)? "CONTROL" : "DATA", rb_idP);
      for (i = 0; i < pdcp_pdu_size; i++)
	LOG_F(PDCP,"%02x ", ((uint8_t*)pdcp_pdu_p->data)[i]);
      LOG_F(PDCP,"\n");
#endif 
      rlc_status = rlc_data_req(enb_mod_idP, ue_mod_idP, frameP, enb_flagP, srb_flagP, MBMS_FLAG_NO, rb_idP, muiP, confirmP, pdcp_pdu_size, pdcp_pdu_p);

  }
  switch (rlc_status) {
  case RLC_OP_STATUS_OK:
    LOG_D(PDCP, "Data sending request over RLC succeeded!\n");
    ret=TRUE;
    break;

  case RLC_OP_STATUS_BAD_PARAMETER:
    LOG_W(PDCP, "Data sending request over RLC failed with 'Bad Parameter' reason!\n");
    ret= FALSE;
    break;
  case RLC_OP_STATUS_INTERNAL_ERROR:
    LOG_W(PDCP, "Data sending request over RLC failed with 'Internal Error' reason!\n");
    ret= FALSE;
    break;

  case RLC_OP_STATUS_OUT_OF_RESSOURCES:
    LOG_W(PDCP, "Data sending request over RLC failed with 'Out of Resources' reason!\n");
    ret= FALSE;
    break;

  default:
    LOG_W(PDCP, "RLC returned an unknown status code after PDCP placed the order to send some data (Status Code:%d)\n", rlc_status);
    ret= FALSE;
    break;
  }
  if (enb_flagP == ENB_FLAG_NO)
    stop_meas(&eNB_pdcp_stats[enb_mod_idP].data_req);
  else
    stop_meas(&UE_pdcp_stats[ue_mod_idP].data_req);
  /*
   * Control arrives here only if rlc_data_req() returns RLC_OP_STATUS_OK
   * so we return TRUE afterwards
   */
  /*
   if (rb_id>=DTCH) {
    if (enb_flagP == 1) {
      Pdcp_stats_tx[module_id][(rb_id & RAB_OFFSET2 )>> RAB_SHIFT2][(rb_id & RAB_OFFSET)-DTCH]++;
      Pdcp_stats_tx_bytes[module_id][(rb_id & RAB_OFFSET2 )>> RAB_SHIFT2][(rb_id & RAB_OFFSET)-DTCH] += sdu_buffer_size;
    } else {
      Pdcp_stats_tx[module_id][(rb_id & RAB_OFFSET2 )>> RAB_SHIFT2][(rb_id & RAB_OFFSET)-DTCH]++;
      Pdcp_stats_tx_bytes[module_id][(rb_id & RAB_OFFSET2 )>> RAB_SHIFT2][(rb_id & RAB_OFFSET)-DTCH] += sdu_buffer_size;
    }
    }*/
  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PDCP_DATA_REQ,VCD_FUNCTION_OUT);
  return ret;

}


boolean_t pdcp_data_ind(
        const module_id_t  enb_mod_idP,
        const module_id_t  ue_mod_idP,
        const frame_t      frameP,
        const eNB_flag_t   enb_flagP,
        const srb_flag_t   srb_flagP,
        const MBMS_flag_t  MBMS_flagP,
        const rb_id_t      rb_idP,
        const sdu_size_t   sdu_buffer_sizeP,
        mem_block_t* const sdu_buffer_pP)
{
  //-----------------------------------------------------------------------------
  pdcp_t      *pdcp_p          = NULL;
  list_t      *sdu_list_p      = NULL;
  mem_block_t *new_sdu_p       = NULL;
  uint8_t      pdcp_header_len = 0;
  uint8_t      pdcp_tailer_len = 0;
  pdcp_sn_t    sequence_number = 0;
  uint8_t      payload_offset  = 0;
  rb_id_t      rb_id           = rb_idP;
  boolean_t    packet_forwarded = FALSE;

 

#ifdef OAI_EMU
  if (enb_flagP) {
      AssertFatal ((enb_mod_idP >= oai_emulation.info.first_enb_local) && (oai_emulation.info.nb_enb_local > 0),
          "eNB module id is too low (%u/%d)!\n",
          enb_mod_idP,
          oai_emulation.info.first_enb_local);
      AssertFatal ((enb_mod_idP < (oai_emulation.info.first_enb_local + oai_emulation.info.nb_enb_local)) && (oai_emulation.info.nb_enb_local > 0),
          "eNB module id is too high (%u/%d)!\n",
          enb_mod_idP,
          oai_emulation.info.first_enb_local + oai_emulation.info.nb_enb_local);
      AssertFatal (ue_mod_idP  < NB_UE_INST,
          "UE module id is too high (%u/%d)!\n",
          ue_mod_idP,
          oai_emulation.info.first_ue_local + oai_emulation.info.nb_ue_local);
  } else {
      AssertFatal (ue_mod_idP  < (oai_emulation.info.first_ue_local + oai_emulation.info.nb_ue_local),
          "UE module id is too high (%u/%d)!\n",
          ue_mod_idP,
          oai_emulation.info.first_ue_local + oai_emulation.info.nb_ue_local);
      AssertFatal (ue_mod_idP  >= oai_emulation.info.first_ue_local,
          "UE module id is too low (%u/%d)!\n",
          ue_mod_idP,
          oai_emulation.info.first_ue_local);
  }
#endif
#ifdef PDCP_MSG_PRINT
      int i=0;
      LOG_F(PDCP,"[MSG] PDCP UL %s PDU on rb_id %d\n", (srb_flagP)? "CONTROL" : "DATA", rb_idP);
      for (i = 0; i < sdu_buffer_sizeP; i++)
	LOG_F(PDCP,"%02x ", ((uint8_t*)sdu_buffer_pP->data)[i]);
      LOG_F(PDCP,"\n");
#endif 

  if (MBMS_flagP) {
      AssertError (rb_idP < NB_RB_MBMS_MAX, return FALSE, "RB id is too high (%u/%d) %u %u!\n", rb_idP, NB_RB_MBMS_MAX, ue_mod_idP, enb_mod_idP);
      if (enb_flagP == ENB_FLAG_NO) {
          LOG_I(PDCP, "e-MBMS Data indication notification for PDCP entity from eNB %u to UE %u "
                "and radio bearer ID %d rlc sdu size %d enb_flagP %d\n",
                enb_mod_idP, ue_mod_idP, rb_idP, sdu_buffer_sizeP, enb_flagP);
      } else {
          LOG_I(PDCP, "Data indication notification for PDCP entity from UE %u to eNB %u "
          "and radio bearer ID %d rlc sdu size %d enb_flagP %d eNB_id %d\n",
          ue_mod_idP, enb_mod_idP , rb_idP, sdu_buffer_sizeP, enb_flagP, enb_mod_idP);
      }
  } else {
      rb_id = rb_idP % maxDRB;
      AssertError (rb_id < maxDRB, return FALSE, "RB id is too high (%u/%d) %u %u!\n", rb_id, maxDRB, ue_mod_idP, enb_mod_idP);
      AssertError (rb_id > 0, return FALSE, "RB id is too low (%u/%d) %u %u!\n", rb_id, maxDRB, ue_mod_idP, enb_mod_idP);

      if (enb_flagP == ENB_FLAG_NO) {
          if (srb_flagP) {
              pdcp_p = &pdcp_array_srb_ue[ue_mod_idP][rb_id-1];
              LOG_D(PDCP, "Data indication notification for PDCP entity from eNB %u to UE %u "
                    "and signalling radio bearer ID %d rlc sdu size %d enb_flagP %d\n",
                    enb_mod_idP, ue_mod_idP, rb_id, sdu_buffer_sizeP, enb_flagP);
          } else {
              pdcp_p = &pdcp_array_drb_ue[ue_mod_idP][rb_id-1];
              LOG_D(PDCP, "Data indication notification for PDCP entity from eNB %u to UE %u "
                    "and data radio bearer ID %d rlc sdu size %d enb_flagP %d\n",
                    enb_mod_idP, ue_mod_idP, rb_id, sdu_buffer_sizeP, enb_flagP);
          }

      } else {
          if (srb_flagP) {
              pdcp_p = &pdcp_array_srb_eNB[enb_mod_idP][ue_mod_idP][rb_id-1];
              LOG_D(PDCP, "Data indication notification for PDCP entity from UE %u to eNB %u "
                  "and signalling radio bearer ID %d rlc sdu size %d enb_flagP %d eNB_id %d\n",
                  ue_mod_idP, enb_mod_idP , rb_id, sdu_buffer_sizeP, enb_flagP, enb_mod_idP);
          } else {
              pdcp_p = &pdcp_array_drb_eNB[enb_mod_idP][ue_mod_idP][rb_id-1];
              LOG_D(PDCP, "Data indication notification for PDCP entity from UE %u to eNB %u "
                  "and data radio bearer ID %d rlc sdu size %d enb_flagP %d eNB_id %d\n",
                  ue_mod_idP, enb_mod_idP , rb_id, sdu_buffer_sizeP, enb_flagP, enb_mod_idP);
          }

      }
  }
  
  sdu_list_p = &pdcp_sdu_list;

  if (sdu_buffer_sizeP == 0) {
      LOG_W(PDCP, "SDU buffer size is zero! Ignoring this chunk!\n");
      return FALSE;
  }
  
  if (enb_flagP)
    start_meas(&eNB_pdcp_stats[enb_mod_idP].data_ind);
  else
    start_meas(&UE_pdcp_stats[ue_mod_idP].data_ind);  
  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PDCP_DATA_IND,VCD_FUNCTION_IN);

  /*
   * Parse the PDU placed at the beginning of SDU to check
   * if incoming SN is in line with RX window
   */
  
  if (MBMS_flagP == 0 ) {
    if (srb_flagP) { //SRB1/2
      pdcp_header_len = PDCP_CONTROL_PLANE_DATA_PDU_SN_SIZE;
      pdcp_tailer_len = PDCP_CONTROL_PLANE_DATA_PDU_MAC_I_SIZE;
      sequence_number =   pdcp_get_sequence_number_of_pdu_with_SRB_sn((unsigned char*)sdu_buffer_pP->data);
    } else { // DRB
      pdcp_tailer_len = 0;
      if (pdcp_p->seq_num_size == PDCP_SN_7BIT){
	pdcp_header_len = PDCP_USER_PLANE_DATA_PDU_SHORT_SN_HEADER_SIZE;
	sequence_number =     pdcp_get_sequence_number_of_pdu_with_short_sn((unsigned char*)sdu_buffer_pP->data);
      }else if (pdcp_p->seq_num_size == PDCP_SN_12BIT){
	pdcp_header_len = PDCP_USER_PLANE_DATA_PDU_LONG_SN_HEADER_SIZE;
	sequence_number =     pdcp_get_sequence_number_of_pdu_with_long_sn((unsigned char*)sdu_buffer_pP->data);
      } else {
	//sequence_number = 4095;
	LOG_E(PDCP,"wrong sequence number  (%d) for this pdcp entity \n", pdcp_p->seq_num_size);
      }
      //uint8_t dc = pdcp_get_dc_filed((unsigned char*)sdu_buffer_pP->data);
    }
    
    /*
     * Check if incoming SDU is long enough to carry a PDU header
     */
    if (sdu_buffer_sizeP < pdcp_header_len + pdcp_tailer_len ) {
          LOG_W(PDCP, "Incoming (from RLC) SDU is short of size (size:%d)! Ignoring...\n", sdu_buffer_sizeP);
          free_mem_block(sdu_buffer_pP);
          if (enb_flagP)
            stop_meas(&eNB_pdcp_stats[enb_mod_idP].data_ind);
          else
            stop_meas(&UE_pdcp_stats[ue_mod_idP].data_ind);
	  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PDCP_DATA_IND,VCD_FUNCTION_OUT);
          return FALSE;
      }
  
    if (pdcp_is_rx_seq_number_valid(sequence_number, pdcp_p, srb_flagP) == TRUE) {
      LOG_D(PDCP, "Incoming PDU has a sequence number (%d) in accordance with RX window\n", sequence_number);
      /* if (dc == PDCP_DATA_PDU )
	 LOG_D(PDCP, "Passing piggybacked SDU to NAS driver...\n");
	 else
	 LOG_D(PDCP, "Passing piggybacked SDU to RRC ...\n");*/
    } else {
      LOG_W(PDCP, "Incoming PDU has an unexpected sequence number (%d), RX window snychronisation have probably been lost!\n", sequence_number);
      /*
       * XXX Till we implement in-sequence delivery and duplicate discarding
       * mechanism all out-of-order packets will be delivered to RRC/IP
       */
#if 0
      LOG_D(PDCP, "Ignoring PDU...\n");
      free_mem_block(sdu_buffer);
      return FALSE;
#else
      LOG_W(PDCP, "Delivering out-of-order SDU to upper layer...\n");
#endif
    }
      // SRB1/2: control-plane data
    if (srb_flagP){
#if defined(ENABLE_SECURITY)
      if (pdcp_p->security_activated == 1) {
          if (enb_flagP == ENB_FLAG_NO)
              start_meas(&eNB_pdcp_stats[enb_mod_idP].validate_security);
          else
              start_meas(&UE_pdcp_stats[ue_mod_idP].validate_security);

          pdcp_validate_security(pdcp_p,
                  srb_flagP,
                  rb_idP,
                  pdcp_header_len,
                  sequence_number,
                  sdu_buffer_pP->data,
                  sdu_buffer_sizeP - pdcp_tailer_len);
          if (enb_flagP == ENB_FLAG_NO)
              stop_meas(&eNB_pdcp_stats[enb_mod_idP].validate_security);
          else
              stop_meas(&UE_pdcp_stats[ue_mod_idP].validate_security);
      }
#endif
      //rrc_lite_data_ind(module_id, //Modified MW - L2 Interface
      pdcp_rrc_data_ind(enb_mod_idP,
              ue_mod_idP,
              frameP,
              enb_flagP,
              rb_id,
              sdu_buffer_sizeP - pdcp_header_len - pdcp_tailer_len,
              (uint8_t*)&sdu_buffer_pP->data[pdcp_header_len]);
      free_mem_block(sdu_buffer_pP);
      // free_mem_block(new_sdu);
      if (enb_flagP)
          stop_meas(&eNB_pdcp_stats[enb_mod_idP].data_ind);
      else
          stop_meas(&UE_pdcp_stats[ue_mod_idP].data_ind);
      vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PDCP_DATA_IND,VCD_FUNCTION_OUT);
      return TRUE;
    }
  /*
   * DRBs 
   */
    payload_offset=pdcp_header_len;// PDCP_USER_PLANE_DATA_PDU_LONG_SN_HEADER_SIZE;
#if defined(ENABLE_SECURITY)
    if (pdcp_p->security_activated == 1) {
      if (enb_flagP == ENB_FLAG_NO)
          start_meas(&eNB_pdcp_stats[enb_mod_idP].validate_security);
      else
          start_meas(&UE_pdcp_stats[ue_mod_idP].validate_security);
      
      pdcp_validate_security(pdcp_p, 
              srb_flagP,
              rb_idP,
              pdcp_header_len,
              sequence_number,
              sdu_buffer_pP->data,
              sdu_buffer_sizeP - pdcp_tailer_len);
      if (enb_flagP == ENB_FLAG_NO)
          stop_meas(&eNB_pdcp_stats[enb_mod_idP].validate_security);
      else
          stop_meas(&UE_pdcp_stats[ue_mod_idP].validate_security);
      
    }
    
#endif
  } else {
      payload_offset=0;
  }
  
#if defined(USER_MODE) && defined(OAI_EMU)
  if (oai_emulation.info.otg_enabled == 1) {
      module_id_t src_id, dst_id;
      int    ctime;
   
      if (pdcp_p->rlc_mode == RLC_MODE_AM ) {
          pdcp_p->last_submitted_pdcp_rx_sn = sequence_number;
      }
   
      rlc_util_print_hex_octets(PDCP,
                                (unsigned char*)&sdu_buffer_pP->data[payload_offset],
                                sdu_buffer_sizeP - payload_offset);

      src_id = (enb_flagP == ENB_FLAG_NO) ?  enb_mod_idP : ue_mod_idP +   NB_eNB_INST;
      dst_id = (enb_flagP == ENB_FLAG_NO) ? ue_mod_idP +  NB_eNB_INST: enb_mod_idP;
      ctime = oai_emulation.info.time_ms; // avg current simulation time in ms : we may get the exact time through OCG?
      LOG_D(PDCP, "Check received buffer : enb_flag %d  rab id %d (src %d, dst %d)\n",
          enb_flagP, rb_id, src_id, dst_id);

      if (otg_rx_pkt(src_id, dst_id,ctime,&sdu_buffer_pP->data[payload_offset],
          sdu_buffer_sizeP - payload_offset ) == 0 ) {
            free_mem_block(sdu_buffer_pP);
            if (enb_flagP)
                stop_meas(&eNB_pdcp_stats[enb_mod_idP].data_ind);
            else
                stop_meas(&UE_pdcp_stats[ue_mod_idP].data_ind);
            vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PDCP_DATA_IND,VCD_FUNCTION_OUT);
            return TRUE;
      }
  }
#else
  if (otg_enabled==1) {
      LOG_D(OTG,"Discarding received packed\n");
      free_mem_block(sdu_buffer_pP);
      if (enb_flagP)
          stop_meas(&eNB_pdcp_stats[enb_mod_idP].data_ind);
      else
          stop_meas(&UE_pdcp_stats[ue_mod_idP].data_ind);
      vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PDCP_DATA_IND,VCD_FUNCTION_OUT);
      return TRUE;
  }
#endif


  // XXX Decompression would be done at this point

  /*
   * After checking incoming sequence number PDCP header
   * has to be stripped off so here we copy SDU buffer starting
   * from its second byte (skipping 0th and 1st octets, i.e.
   * PDCP header)
   */
#if defined(LINK_PDCP_TO_GTPV1U)
  if (enb_flagP) {
      LOG_I(PDCP,"Sending to GTPV1U %d bytes\n", sdu_buffer_sizeP - payload_offset);

      gtpv1u_new_data_req(
              enb_mod_idP, //gtpv1u_data_t *gtpv1u_data_p,
              ue_mod_idP,//rb_id/maxDRB, TO DO UE ID
              ((pdcp_data_ind_header_t *) new_sdu_p->data)->rb_id,
              &sdu_buffer_pP->data[payload_offset],
              sdu_buffer_sizeP - payload_offset);
      packet_forwarded = TRUE;
  }
#else
  packet_forwarded = FALSE;
#endif
  if (FALSE == packet_forwarded) {
      new_sdu_p = get_free_mem_block(sdu_buffer_sizeP - payload_offset + sizeof (pdcp_data_ind_header_t));

      if (new_sdu_p) {
          if (pdcp_p->rlc_mode == RLC_MODE_AM ) {
              pdcp_p->last_submitted_pdcp_rx_sn = sequence_number;
          }
          /*
           * Prepend PDCP indication header which is going to be removed at pdcp_fifo_flush_sdus()
           */
          memset(new_sdu_p->data, 0, sizeof (pdcp_data_ind_header_t));
          ((pdcp_data_ind_header_t *) new_sdu_p->data)->data_size = sdu_buffer_sizeP - payload_offset;

          // Here there is no virtualization possible
          // set ((pdcp_data_ind_header_t *) new_sdu_p->data)->inst for IP layer here
          if (enb_flagP == ENB_FLAG_NO) {
              ((pdcp_data_ind_header_t *) new_sdu_p->data)->rb_id = rb_id;
#if defined(OAI_EMU)
              ((pdcp_data_ind_header_t *) new_sdu_p->data)->inst  = ue_mod_idP + oai_emulation.info.nb_enb_local - oai_emulation.info.first_ue_local;
#endif
          } else {
              ((pdcp_data_ind_header_t *) new_sdu_p->data)->rb_id = rb_id + (ue_mod_idP * maxDRB);
#if defined(OAI_EMU)
              ((pdcp_data_ind_header_t *) new_sdu_p->data)->inst  = enb_mod_idP - oai_emulation.info.first_enb_local;
#endif
          }

          memcpy(&new_sdu_p->data[sizeof (pdcp_data_ind_header_t)], \
                  &sdu_buffer_pP->data[payload_offset], \
                  sdu_buffer_sizeP - payload_offset);
          list_add_tail_eurecom (new_sdu_p, sdu_list_p);

          /* Print octets of incoming data in hexadecimal form */
          LOG_D(PDCP, "Following content has been received from RLC (%d,%d)(PDCP header has already been removed):\n",
              sdu_buffer_sizeP  - payload_offset + sizeof(pdcp_data_ind_header_t),
              sdu_buffer_sizeP  - payload_offset);
          //util_print_hex_octets(PDCP, &new_sdu_p->data[sizeof (pdcp_data_ind_header_t)], sdu_buffer_sizeP - payload_offset);
          //util_flush_hex_octets(PDCP, &new_sdu_p->data[sizeof (pdcp_data_ind_header_t)], sdu_buffer_sizeP - payload_offset);

          /*
           * Update PDCP statistics
           * XXX Following two actions are identical, is there a merge error?
           */

          /*if (enb_flagP == 1) {
              Pdcp_stats_rx[module_id][(rb_idP & RAB_OFFSET2) >> RAB_SHIFT2][(rb_idP & RAB_OFFSET) - DTCH]++;
              Pdcp_stats_rx_bytes[module_id][(rb_idP & RAB_OFFSET2) >> RAB_SHIFT2][(rb_idP & RAB_OFFSET) - DTCH] += sdu_buffer_sizeP;
            } else {
              Pdcp_stats_rx[module_id][(rb_idP & RAB_OFFSET2) >> RAB_SHIFT2][(rb_idP & RAB_OFFSET) - DTCH]++;
              Pdcp_stats_rx_bytes[module_id][(rb_idP & RAB_OFFSET2) >> RAB_SHIFT2][(rb_idP & RAB_OFFSET) - DTCH] += sdu_buffer_sizeP;
            }*/
      }
  }
#if defined(STOP_ON_IP_TRAFFIC_OVERLOAD)
else {
  AssertFatal(0, "[FRAME %5u][%s][PDCP][MOD %u/%u][RB %u] PDCP_DATA_IND SDU DROPPED, OUT OF MEMORY \n",
        frameP,
        (enb_flagP) ? "eNB" : "UE",
        enb_mod_idP,
        ue_mod_idP,
        rb_id);
}
#endif
#endif

  free_mem_block(sdu_buffer_pP);
  if (enb_flagP)
    stop_meas(&eNB_pdcp_stats[enb_mod_idP].data_ind);
  else
    stop_meas(&UE_pdcp_stats[ue_mod_idP].data_ind);
  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PDCP_DATA_IND,VCD_FUNCTION_OUT);
  return TRUE;
}

//-----------------------------------------------------------------------------
void pdcp_run (
        const frame_t frameP,
        const eNB_flag_t  enb_flagP,
        const module_id_t ue_mod_idP,
        const module_id_t enb_mod_idP) {
  //-----------------------------------------------------------------------------
#if defined(ENABLE_ITTI)
  MessageDef   *msg_p;
  const char   *msg_name;
  instance_t    instance;
  int           result;
#endif
  if (enb_flagP)
    start_meas(&eNB_pdcp_stats[enb_mod_idP].pdcp_run);
  else
    start_meas(&UE_pdcp_stats[ue_mod_idP].pdcp_run);

  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PDCP_RUN, VCD_FUNCTION_IN);

#if defined(ENABLE_ITTI)
  do {
      // Checks if a message has been sent to PDCP sub-task
      itti_poll_msg (enb_flagP ? TASK_PDCP_ENB : TASK_PDCP_UE, &msg_p);

      if (msg_p != NULL) {
          msg_name = ITTI_MSG_NAME (msg_p);
          instance = ITTI_MSG_INSTANCE (msg_p);

          switch (ITTI_MSG_ID(msg_p)) {
          case RRC_DCCH_DATA_REQ:
            LOG_D(PDCP, "Received %s from %s: instance %d, frame %d, enb_flagP %d, rb_id %d, muiP %d, confirmP %d, mode %d\n",
                msg_name, ITTI_MSG_ORIGIN_NAME(msg_p), instance,
                RRC_DCCH_DATA_REQ (msg_p).frame, RRC_DCCH_DATA_REQ (msg_p).enb_flag, RRC_DCCH_DATA_REQ (msg_p).rb_id,
                RRC_DCCH_DATA_REQ (msg_p).muip, RRC_DCCH_DATA_REQ (msg_p).confirmp, RRC_DCCH_DATA_REQ (msg_p).mode);

            result = pdcp_data_req (RRC_DCCH_DATA_REQ (msg_p).eNB_index,
                RRC_DCCH_DATA_REQ (msg_p).ue_index,
                RRC_DCCH_DATA_REQ (msg_p).frame,
                RRC_DCCH_DATA_REQ (msg_p).enb_flag,
                SRB_FLAG_YES,
                RRC_DCCH_DATA_REQ (msg_p).rb_id,
                RRC_DCCH_DATA_REQ (msg_p).muip,
                RRC_DCCH_DATA_REQ (msg_p).confirmp,
                RRC_DCCH_DATA_REQ (msg_p).sdu_size,
                RRC_DCCH_DATA_REQ (msg_p).sdu_p,
                RRC_DCCH_DATA_REQ (msg_p).mode);
            AssertFatal (result == TRUE, "PDCP data request failed!\n");

            // Message buffer has been processed, free it now.
            result = itti_free (ITTI_MSG_ORIGIN_ID(msg_p), RRC_DCCH_DATA_REQ (msg_p).sdu_p);
            AssertFatal (result == EXIT_SUCCESS, "Failed to free memory (%d)!\n", result);
            break;

          default:
            LOG_E(PDCP, "Received unexpected message %s\n", msg_name);
            break;
          }

          result = itti_free (ITTI_MSG_ORIGIN_ID(msg_p), msg_p);
          AssertFatal (result == EXIT_SUCCESS, "Failed to free memory (%d)!\n", result);
      }
  } while(msg_p != NULL);

# if 0
  {
      MessageDef *msg_resp_p;

      msg_resp_p = itti_alloc_new_message(TASK_PDCP_ENB, MESSAGE_TEST);

      itti_send_msg_to_task(TASK_RRC_ENB, 1, msg_resp_p);
  }
  {
    MessageDef *msg_resp_p;

    msg_resp_p = itti_alloc_new_message(TASK_PDCP_ENB, MESSAGE_TEST);

    itti_send_msg_to_task(TASK_ENB_APP, 2, msg_resp_p);
  }
  {
    MessageDef *msg_resp_p;

    msg_resp_p = itti_alloc_new_message(TASK_PDCP_ENB, MESSAGE_TEST);

    itti_send_msg_to_task(TASK_MAC_ENB, 3, msg_resp_p);
  }
# endif
#endif

#if defined(USER_MODE) && defined(OAI_EMU)
  pdcp_t            *pdcp_p          = NULL;
  int               drb_id=1 ;
  int               ue_id=0;
  int               read_otg=1;
    // add check for other rb_ids later
  if (enb_flagP == ENB_FLAG_NO) {
    if (pdcp_array_drb_ue[ue_mod_idP][drb_id-1].instanciated_instance  != TRUE )
      read_otg=0;
  } else {
    for (ue_id=0; ue_id < NB_UE_INST; ue_id++)
      if (pdcp_array_drb_eNB[enb_mod_idP][ue_id][drb_id-1].instanciated_instance  != TRUE ){
	read_otg =0;
	break;
      }
  }
  if (read_otg == 1 )
    pdcp_fifo_read_input_sdus_from_otg(frameP, enb_flagP, ue_mod_idP, enb_mod_idP);
#endif

  // IP/NAS -> PDCP traffic : TX, read the pkt from the upper layer buffer
#if defined(LINK_PDCP_TO_GTPV1U)
  if (enb_flagP == ENB_FLAG_NO)
#endif
  {
      pdcp_fifo_read_input_sdus(frameP, enb_flagP, ue_mod_idP, enb_mod_idP);
  }
  // PDCP -> NAS/IP traffic: RX
  if (enb_flagP)
    start_meas(&eNB_pdcp_stats[enb_mod_idP].pdcp_ip);
  else
    start_meas(&UE_pdcp_stats[ue_mod_idP].pdcp_ip);

  pdcp_fifo_flush_sdus(frameP, enb_flagP, enb_mod_idP, ue_mod_idP);

  if (enb_flagP)
    stop_meas(&eNB_pdcp_stats[enb_mod_idP].pdcp_ip);
  else
    stop_meas(&UE_pdcp_stats[ue_mod_idP].pdcp_ip);

  if (enb_flagP)
    stop_meas(&eNB_pdcp_stats[enb_mod_idP].pdcp_run);
  else
    stop_meas(&UE_pdcp_stats[ue_mod_idP].pdcp_run);

  vcd_signal_dumper_dump_function_by_name(VCD_SIGNAL_DUMPER_FUNCTIONS_PDCP_RUN, VCD_FUNCTION_OUT);

}

boolean_t rrc_pdcp_config_asn1_req (
        const module_id_t               enb_mod_idP,
        const module_id_t               ue_mod_idP,
        const frame_t              frameP,
        const eNB_flag_t           enb_flagP,
        SRB_ToAddModList_t  *const srb2add_list_pP,
        DRB_ToAddModList_t  *const drb2add_list_pP,
        DRB_ToReleaseList_t *const drb2release_list_pP,
        const uint8_t                   security_modeP,
        uint8_t                  *const kRRCenc_pP,
        uint8_t                  *const kRRCint_pP,
        uint8_t                  *const kUPenc_pP
#ifdef Rel10
,PMCH_InfoList_r9_t*  const pmch_InfoList_r9_pP
#endif
)
{
  long int        lc_id          = 0;
  DRB_Identity_t  srb_id         = 0;
  long int        mch_id         = 0;
  rlc_mode_t      rlc_type       = RLC_MODE_NONE;
  DRB_Identity_t  drb_id         = 0;
  DRB_Identity_t *pdrb_id_p      = NULL;
  uint8_t         drb_sn         = 12;
  uint8_t         srb_sn         = 5; // fixed sn for SRBs
  uint8_t         drb_report     = 0;
  long int        cnt            = 0;
  uint16_t        header_compression_profile = 0;
  config_action_t action                     = CONFIG_ACTION_ADD;
  SRB_ToAddMod_t *srb_toaddmod_p = NULL;
  DRB_ToAddMod_t *drb_toaddmod_p = NULL;
  pdcp_t         *pdcp_p         = NULL;

#ifdef Rel10
  int i,j;
  MBMS_SessionInfoList_r9_t *mbms_SessionInfoList_r9_p = NULL;
  MBMS_SessionInfo_r9_t     *MBMS_SessionInfo_p        = NULL;
#endif

  if (enb_flagP == ENB_FLAG_NO) {
      LOG_D(PDCP, "[UE %u] CONFIG REQ ASN1 for eNB %u\n", ue_mod_idP, enb_mod_idP);
  } else {
      LOG_D(PDCP, "[eNB %u] CONFIG REQ ASN1 for UE %u\n", enb_mod_idP, ue_mod_idP);
  }
  // srb2add_list does not define pdcp config, we use rlc info to setup the pdcp dcch0 and dcch1 channels

  if (srb2add_list_pP != NULL) {
      for (cnt=0;cnt<srb2add_list_pP->list.count;cnt++) {
          srb_id = srb2add_list_pP->list.array[cnt]->srb_Identity;
          srb_toaddmod_p = srb2add_list_pP->list.array[cnt];
          rlc_type = RLC_MODE_AM;
          lc_id = srb_id + 2;

          if (enb_flagP == ENB_FLAG_NO) {
              pdcp_p = &pdcp_array_srb_ue[ue_mod_idP][srb_id-1];
          } else {
              pdcp_p = &pdcp_array_srb_eNB[enb_mod_idP][ue_mod_idP][srb_id-1];
          }


          if (pdcp_p->instanciated_instance == TRUE) {
              action = CONFIG_ACTION_MODIFY;
          } else {
              action = CONFIG_ACTION_ADD;
          }

          if (srb_toaddmod_p->rlc_Config) {
              switch (srb_toaddmod_p->rlc_Config->present) {
              case SRB_ToAddMod__rlc_Config_PR_NOTHING:
                break;
              case SRB_ToAddMod__rlc_Config_PR_explicitValue:
                switch (srb_toaddmod_p->rlc_Config->choice.explicitValue.present) {
                case RLC_Config_PR_NOTHING:
                  break;
                default:
                  pdcp_config_req_asn1 (pdcp_p,
                      enb_mod_idP,
                      ue_mod_idP,
                      frameP,
                      enb_flagP, // not really required
                      SRB_FLAG_YES,
                      rlc_type,
                      action,
                      lc_id,
                      mch_id,
                      srb_id,
                      srb_sn,
                      0, // drb_report
                      0, // header compression
                      security_modeP,
                      kRRCenc_pP,
                      kRRCint_pP,
                      kUPenc_pP);
                  break;
                }
                break;
                case SRB_ToAddMod__rlc_Config_PR_defaultValue:
                  // already the default values
                  break;
                default:
                  DevParam(srb_toaddmod_p->rlc_Config->present, ue_mod_idP, enb_mod_idP);
                  break;
              }
          }
      }
  }
  // reset the action

  if (drb2add_list_pP != NULL) {
      for (cnt=0;cnt<drb2add_list_pP->list.count;cnt++) {

          drb_toaddmod_p = drb2add_list_pP->list.array[cnt];

          drb_id = drb_toaddmod_p->drb_Identity;// + drb_id_offset;

          lc_id = drb_id + 2;


          DevCheck4(drb_id < maxDRB, drb_id, maxDRB, ue_mod_idP, enb_mod_idP);

          if (enb_flagP == ENB_FLAG_NO) {
              pdcp_p = &pdcp_array_drb_ue[ue_mod_idP][drb_id-1];
          } else {
              pdcp_p = &pdcp_array_drb_eNB[enb_mod_idP][ue_mod_idP][drb_id-1];
          }

          if (pdcp_p->instanciated_instance == TRUE)
            action = CONFIG_ACTION_MODIFY;
          else
            action = CONFIG_ACTION_ADD;

          if (drb_toaddmod_p->pdcp_Config){
              if (drb_toaddmod_p->pdcp_Config->discardTimer) {
                  // set the value of the timer
              }
              if (drb_toaddmod_p->pdcp_Config->rlc_AM) {
                  drb_report = drb_toaddmod_p->pdcp_Config->rlc_AM->statusReportRequired;
		  drb_sn = PDCP_Config__rlc_UM__pdcp_SN_Size_len12bits; // default SN size 
                  rlc_type = RLC_MODE_AM;
              }
              if (drb_toaddmod_p->pdcp_Config->rlc_UM){
                  drb_sn = drb_toaddmod_p->pdcp_Config->rlc_UM->pdcp_SN_Size;
                  rlc_type =RLC_MODE_UM;
              }
              switch (drb_toaddmod_p->pdcp_Config->headerCompression.present) {
              case PDCP_Config__headerCompression_PR_NOTHING:
              case PDCP_Config__headerCompression_PR_notUsed:
                header_compression_profile=0x0;
                break;
              case PDCP_Config__headerCompression_PR_rohc:
                // parse the struc and get the rohc profile
                if(drb_toaddmod_p->pdcp_Config->headerCompression.choice.rohc.profiles.profile0x0001)
                  header_compression_profile=0x0001;
                else if(drb_toaddmod_p->pdcp_Config->headerCompression.choice.rohc.profiles.profile0x0002)
                  header_compression_profile=0x0002;
                else if(drb_toaddmod_p->pdcp_Config->headerCompression.choice.rohc.profiles.profile0x0003)
                  header_compression_profile=0x0003;
                else if(drb_toaddmod_p->pdcp_Config->headerCompression.choice.rohc.profiles.profile0x0004)
                  header_compression_profile=0x0004;
                else if(drb_toaddmod_p->pdcp_Config->headerCompression.choice.rohc.profiles.profile0x0006)
                  header_compression_profile=0x0006;
                else if(drb_toaddmod_p->pdcp_Config->headerCompression.choice.rohc.profiles.profile0x0101)
                  header_compression_profile=0x0101;
                else if(drb_toaddmod_p->pdcp_Config->headerCompression.choice.rohc.profiles.profile0x0102)
                  header_compression_profile=0x0102;
                else if(drb_toaddmod_p->pdcp_Config->headerCompression.choice.rohc.profiles.profile0x0103)
                  header_compression_profile=0x0103;
                else if(drb_toaddmod_p->pdcp_Config->headerCompression.choice.rohc.profiles.profile0x0104)
                  header_compression_profile=0x0104;
                else {
                    header_compression_profile=0x0;
                    LOG_W(PDCP,"unknown header compresion profile\n");
                }
                // set the applicable profile
                break;
              default:
                LOG_W(PDCP,"[MOD_id %u/%u][RB %u] unknown drb_toaddmod->PDCP_Config->headerCompression->present \n",
                    enb_mod_idP, ue_mod_idP, drb_id);
                break;
              }
              pdcp_config_req_asn1 (pdcp_p,
                  enb_mod_idP,
                  ue_mod_idP,
                  frameP,
                  enb_flagP, // not really required
                  SRB_FLAG_NO,
                  rlc_type,
                  action,
                  lc_id,
                  mch_id,
                  drb_id,
                  drb_sn,
                  drb_report,
                  header_compression_profile,
                  security_modeP,
                  kRRCenc_pP,
                  kRRCint_pP,
                  kUPenc_pP);
          }
      }
  }

  if (drb2release_list_pP != NULL) {
      for (cnt=0;cnt<drb2release_list_pP->list.count;cnt++) {
          pdrb_id_p = drb2release_list_pP->list.array[cnt];
          drb_id =  *pdrb_id_p;
          lc_id = drb_id + 2;
          if (enb_flagP == ENB_FLAG_NO) {
              pdcp_p = &pdcp_array_drb_ue[ue_mod_idP][drb_id-1];
          } else {
              pdcp_p = &pdcp_array_drb_eNB[enb_mod_idP][ue_mod_idP][drb_id-1];
          }
          action = CONFIG_ACTION_REMOVE;
          pdcp_config_req_asn1 (pdcp_p,
              enb_mod_idP,
              ue_mod_idP,
              frameP,
              enb_flagP, // not really required
              SRB_FLAG_NO,
              rlc_type,
              action,
              lc_id,
              mch_id,
              drb_id,
              0,
              0,
              0,
              security_modeP,
              kRRCenc_pP,
              kRRCint_pP,
              kUPenc_pP);
      }
  }

#ifdef Rel10
  if (pmch_InfoList_r9_pP != NULL) {
      for (i=0;i<pmch_InfoList_r9_pP->list.count;i++) {
          mbms_SessionInfoList_r9_p = &(pmch_InfoList_r9_pP->list.array[i]->mbms_SessionInfoList_r9);
          for (j=0;j<mbms_SessionInfoList_r9_p->list.count;j++) {
              MBMS_SessionInfo_p = mbms_SessionInfoList_r9_p->list.array[j];
              lc_id = MBMS_SessionInfo_p->sessionId_r9->buf[0];
              mch_id = MBMS_SessionInfo_p->tmgi_r9.serviceId_r9.buf[2]; //serviceId is 3-octet string

              // can set the mch_id = i
              if (enb_flagP) {
                  drb_id =  (mch_id * maxSessionPerPMCH ) + lc_id ;//+ (maxDRB + 3)*MAX_MOBILES_PER_ENB; // 1
                  if (pdcp_mbms_array_eNB[enb_mod_idP][mch_id][lc_id].instanciated_instance == TRUE) {
                      action = CONFIG_ACTION_MBMS_MODIFY;
                  }else {
                      action = CONFIG_ACTION_MBMS_ADD;
                  }
              } else {
                  drb_id =  (mch_id * maxSessionPerPMCH ) + lc_id; // + (maxDRB + 3); // 15
                  if (pdcp_mbms_array_ue[ue_mod_idP][mch_id][lc_id].instanciated_instance == TRUE) {
                      action = CONFIG_ACTION_MBMS_MODIFY;
                  } else {
                      action = CONFIG_ACTION_MBMS_ADD;
                  }
              }

              pdcp_config_req_asn1 (
                  NULL,  // unused for MBMS
                  enb_mod_idP,
                  ue_mod_idP,
                  frameP,
                  enb_flagP,
                  SRB_FLAG_NO,
                  RLC_MODE_NONE,
                  action,
                  lc_id,
                  mch_id,
                  drb_id,
                  0,   // unused for MBMS
                  0,   // unused for MBMS
                  0,   // unused for MBMS
                  0,   // unused for MBMS
                  NULL,  // unused for MBMS
                  NULL,  // unused for MBMS
                  NULL); // unused for MBMS
          }
      }
  }
#endif

  return 1;

}

boolean_t pdcp_config_req_asn1 (pdcp_t   *pdcp_pP,
    module_id_t     enb_mod_idP,
    module_id_t     ue_mod_idP,
    frame_t         frameP,
    eNB_flag_t      enb_flagP,
    srb_flag_t      srb_flagP,
    rlc_mode_t      rlc_modeP,
    config_action_t actionP,
    uint16_t        lc_idP,
    uint16_t        mch_idP,
    rb_id_t         rb_idP,
    uint8_t         rb_snP,
    uint8_t         rb_reportP,
    uint16_t        header_compression_profileP,
    uint8_t         security_modeP,
    uint8_t         *kRRCenc_pP,
    uint8_t         *kRRCint_pP,
    uint8_t         *kUPenc_pP)
{

  switch (actionP) {
  case CONFIG_ACTION_ADD:
    DevAssert(pdcp_pP != NULL);
    pdcp_pP->instanciated_instance      = TRUE;
    pdcp_pP->is_ue                      = (enb_flagP == ENB_FLAG_NO) ? TRUE : FALSE;
    pdcp_pP->is_srb                     = (srb_flagP == SRB_FLAG_YES) ? TRUE : FALSE;
    pdcp_pP->lcid                       = lc_idP;
    pdcp_pP->rb_id                      = rb_idP;
    pdcp_pP->header_compression_profile = header_compression_profileP;
    pdcp_pP->status_report              = rb_reportP;

    if (rb_snP == PDCP_Config__rlc_UM__pdcp_SN_Size_len12bits) {
        pdcp_pP->seq_num_size = PDCP_SN_12BIT;
    } else if (rb_snP == PDCP_Config__rlc_UM__pdcp_SN_Size_len7bits) {
        pdcp_pP->seq_num_size = PDCP_SN_7BIT;
    } else {
        pdcp_pP->seq_num_size = PDCP_SN_5BIT;
    }


    pdcp_pP->rlc_mode                         = rlc_modeP;
    pdcp_pP->next_pdcp_tx_sn                  = 0;
    pdcp_pP->next_pdcp_rx_sn                  = 0;
    pdcp_pP->next_pdcp_rx_sn_before_integrity = 0;
    pdcp_pP->tx_hfn                           = 0;
    pdcp_pP->rx_hfn                           = 0;
    pdcp_pP->last_submitted_pdcp_rx_sn        = 4095;
    pdcp_pP->first_missing_pdu                = -1;
    pdcp_pP->rx_hfn_offset                    = 0;
    
    if (enb_flagP == ENB_FLAG_NO) {
        LOG_I(PDCP, "[UE %d] Config request : Action ADD for eNB %d: Frame %d LCID %d (rb id %d) "
            "configured with SN size %d bits and RLC %s\n",
            ue_mod_idP, enb_mod_idP, frameP, lc_idP, rb_idP, pdcp_pP->seq_num_size,
            (rlc_modeP == RLC_MODE_AM ) ? "AM" : (rlc_modeP == RLC_MODE_TM) ? "TM" : "UM");
    } else {
        LOG_D(PDCP, "[eNB %d] Config request : Action ADD for UE %d: Frame %d LCID %d (rb id %d) "
            "configured with SN size %d bits and RLC %s\n",
            enb_mod_idP, ue_mod_idP, frameP, lc_idP, rb_idP, pdcp_pP->seq_num_size,
            (rlc_modeP == RLC_MODE_AM) ? "AM" : (rlc_modeP == RLC_MODE_TM) ? "TM" : "UM");
    }

    /* Setup security */
    if (security_modeP != 0xff) {
        pdcp_config_set_security(pdcp_pP, enb_mod_idP, ue_mod_idP, frameP, enb_flagP, rb_idP, lc_idP, security_modeP, kRRCenc_pP, kRRCint_pP, kUPenc_pP);
    }

    LOG_D(PDCP, "[FRAME %5u][%s][PDCP][MOD %u/%u][RB %u]\n", frameP, (enb_flagP == ENB_FLAG_NO) ? "UE" : "eNB",  enb_mod_idP, ue_mod_idP, rb_idP);
    break;

  case CONFIG_ACTION_MODIFY:
    DevAssert(pdcp_pP != NULL);
    pdcp_pP->header_compression_profile=header_compression_profileP;
    pdcp_pP->status_report = rb_reportP;
    pdcp_pP->rlc_mode = rlc_modeP;

    /* Setup security */
    if (security_modeP != 0xff) {
        pdcp_config_set_security(pdcp_pP, enb_mod_idP, ue_mod_idP, frameP, enb_flagP, rb_idP, lc_idP, security_modeP, kRRCenc_pP, kRRCint_pP, kUPenc_pP);
    }

    if (rb_snP == PDCP_Config__rlc_UM__pdcp_SN_Size_len7bits) {
        pdcp_pP->seq_num_size = 7;
    } else if (rb_snP == PDCP_Config__rlc_UM__pdcp_SN_Size_len12bits) {
        pdcp_pP->seq_num_size = 12;
    } else {
        pdcp_pP->seq_num_size=5;
    }

    if (enb_flagP == ENB_FLAG_NO) {
        LOG_I(PDCP,"[UE %d] Config request : Action MODIFY for eNB %d: Frame %d LCID %d "
            "RB id %d configured with SN size %d and RLC %s \n",
            ue_mod_idP, enb_mod_idP, frameP, lc_idP, rb_idP, rb_snP,
            (rlc_modeP == RLC_MODE_AM) ? "AM" : (rlc_modeP == RLC_MODE_TM) ? "TM" : "UM");
    } else {
        LOG_D(PDCP,"[eNB %d] Config request : Action MODIFY for UE %d: Frame %d LCID %d "
            "RB id %d configured with SN size %d and RLC %s \n",
            enb_mod_idP, ue_mod_idP, frameP, lc_idP, rb_idP, rb_snP,
            (rlc_modeP == RLC_MODE_AM) ? "AM" : (rlc_modeP == RLC_MODE_TM) ? "TM" : "UM");
    }
    break;
  case CONFIG_ACTION_REMOVE:
    DevAssert(pdcp_pP != NULL);
    pdcp_pP->instanciated_instance = FALSE;
    pdcp_pP->lcid = 0;
    pdcp_pP->header_compression_profile = 0x0;
    pdcp_pP->cipheringAlgorithm = 0xff;
    pdcp_pP->integrityProtAlgorithm = 0xff;
    pdcp_pP->status_report = 0;
    pdcp_pP->rlc_mode = RLC_MODE_NONE;
    pdcp_pP->next_pdcp_tx_sn = 0;
    pdcp_pP->next_pdcp_rx_sn = 0;
    pdcp_pP->tx_hfn = 0;
    pdcp_pP->rx_hfn = 0;
    pdcp_pP->last_submitted_pdcp_rx_sn = 4095;
    pdcp_pP->seq_num_size = 0;
    pdcp_pP->first_missing_pdu = -1;
    pdcp_pP->security_activated = 0;

    if (enb_flagP == ENB_FLAG_NO) {
        LOG_I(PDCP, "[UE %d] Config request : CONFIG_ACTION_REMOVE for eNB %d: Frame %d LCID %d RBID %d configured\n",
            ue_mod_idP, enb_mod_idP, frameP, lc_idP, rb_idP);
    } else {
        LOG_D(PDCP, "[eNB %d] Config request : CONFIG_ACTION_REMOVE for UE %d: Frame %d LCID %d RBID %d configured\n",
            enb_mod_idP, ue_mod_idP, frameP, lc_idP, rb_idP);
    }
    /* Security keys */
    if (pdcp_pP->kUPenc != NULL) {
        free(pdcp_pP->kUPenc);
    }
    if (pdcp_pP->kRRCint != NULL) {
        free(pdcp_pP->kRRCint);
    }
    if (pdcp_pP->kRRCenc != NULL) {
        free(pdcp_pP->kRRCenc);
    }
    break;
#if defined(Rel10)
  case CONFIG_ACTION_MBMS_ADD:
  case CONFIG_ACTION_MBMS_MODIFY:
    if (enb_flagP == ENB_FLAG_NO) {
        LOG_I(PDCP,"[UE %d] Config request for eNB %d: %s: Frame %d service_id/mch index %d, session_id/lcid %d, rbid %d configured\n",
            ue_mod_idP, enb_mod_idP, actionP == CONFIG_ACTION_MBMS_ADD ? "CONFIG_ACTION_MBMS_ADD" : "CONFIG_ACTION_MBMS_MODIFY", frameP, mch_idP, lc_idP, rb_idP);
    } else {
        LOG_D(PDCP,"[eNB %d] Config request for UE %d: %s: Frame %d service_id/mch index %d, session_id/lcid %d, rbid %d configured\n",
            enb_mod_idP, ue_mod_idP, actionP == CONFIG_ACTION_MBMS_ADD ? "CONFIG_ACTION_MBMS_ADD" : "CONFIG_ACTION_MBMS_MODIFY", frameP, mch_idP, lc_idP, rb_idP);
    }
    if (enb_flagP == 1) {
        pdcp_mbms_array_eNB[enb_mod_idP][mch_idP][lc_idP].instanciated_instance = TRUE ;
        pdcp_mbms_array_eNB[enb_mod_idP][mch_idP][lc_idP].rb_id = rb_idP;
    } else {
        pdcp_mbms_array_ue[ue_mod_idP][mch_idP][lc_idP].instanciated_instance = TRUE ;
        pdcp_mbms_array_ue[ue_mod_idP][mch_idP][lc_idP].rb_id = rb_idP;
    }
    break;
#endif
  case CONFIG_ACTION_SET_SECURITY_MODE:
    pdcp_config_set_security(pdcp_pP, enb_mod_idP, ue_mod_idP, frameP, enb_flagP, rb_idP, lc_idP, security_modeP, kRRCenc_pP, kRRCint_pP, kUPenc_pP);
    break;
  default:
    DevParam(actionP, enb_mod_idP, ue_mod_idP);
    break;
  }
  return 0;
}

void pdcp_config_set_security(pdcp_t    *pdcp_pP,
    module_id_t     enb_mod_idP,
    module_id_t     ue_mod_idP,
    frame_t    frameP,
    eNB_flag_t enb_flagP,
    rb_id_t    rb_idP,
    uint16_t        lc_idP,
    uint8_t         security_modeP,
    uint8_t        *kRRCenc,
    uint8_t        *kRRCint,
    uint8_t        *kUPenc)
{
  DevAssert(pdcp_pP != NULL);

  if ((security_modeP >= 0) && (security_modeP <= 0x77)) {
      pdcp_pP->cipheringAlgorithm     = security_modeP & 0x0f;
      pdcp_pP->integrityProtAlgorithm = (security_modeP>>4) & 0xf;

      if (enb_flagP == ENB_FLAG_NO) {
          LOG_D(PDCP,"[UE %d][RB %02d] Set security mode : CONFIG_ACTION_SET_SECURITY_MODE: "
              "Frame %d  cipheringAlgorithm %d integrityProtAlgorithm %d\n",
              ue_mod_idP,
              rb_idP,
              frameP,
              pdcp_pP->cipheringAlgorithm,
              pdcp_pP->integrityProtAlgorithm);
      } else {
          LOG_D(PDCP,"[eNB %d][UE %d][RB %02d] Set security mode : CONFIG_ACTION_SET_SECURITY_MODE: "
              "Frame %d  cipheringAlgorithm %d integrityProtAlgorithm %d\n",
              enb_mod_idP,
              ue_mod_idP,
              rb_idP,
              frameP,
              pdcp_pP->cipheringAlgorithm,
              pdcp_pP->integrityProtAlgorithm);
      }
      pdcp_pP->kRRCenc = kRRCenc;
      pdcp_pP->kRRCint = kRRCint;
      pdcp_pP->kUPenc  = kUPenc;

      /* Activate security */
      pdcp_pP->security_activated = 1;
  } else {
      LOG_E(PDCP,"[%s %d] bad security mode %d", security_modeP);
  }
}

void rrc_pdcp_config_req (module_id_t enb_mod_idP, module_id_t ue_mod_idP, frame_t frameP, eNB_flag_t enb_flagP, srb_flag_t srb_flagP, uint32_t actionP, rb_id_t rb_idP, uint8_t security_modeP)
{
  pdcp_t *pdcp_p = NULL;

  if (enb_flagP == ENB_FLAG_NO) {
      if (srb_flagP) {
          pdcp_p = &pdcp_array_srb_ue[ue_mod_idP][rb_idP-1];
      } else {
          pdcp_p = &pdcp_array_drb_ue[ue_mod_idP][rb_idP-1];
      }
  } else {
      if (srb_flagP) {
          pdcp_p = &pdcp_array_srb_eNB[enb_mod_idP][ue_mod_idP][rb_idP-1];
      } else {
          pdcp_p = &pdcp_array_drb_eNB[enb_mod_idP][ue_mod_idP][rb_idP-1];
      }
  }

  /*
   * Initialize sequence number state variables of relevant PDCP entity
   */
  switch (actionP) {
  case CONFIG_ACTION_ADD:
    pdcp_p->instanciated_instance = TRUE;
    pdcp_p->is_srb = srb_flagP;
    pdcp_p->rb_id  = rb_idP;
    if (enb_flagP == ENB_FLAG_NO) {
        pdcp_p->is_ue = TRUE;
    } else {
        pdcp_p->is_ue = FALSE;
    }

    pdcp_p->next_pdcp_tx_sn = 0;
    pdcp_p->next_pdcp_rx_sn = 0;
    pdcp_p->tx_hfn = 0;
    pdcp_p->rx_hfn = 0;
    /* SN of the last PDCP SDU delivered to upper layers */
    pdcp_p->last_submitted_pdcp_rx_sn = 4095;

    if (rb_idP < DTCH) { // SRB
        pdcp_p->seq_num_size = 5;
    } else { // DRB
        pdcp_p->seq_num_size = 12;
    }
    pdcp_p->first_missing_pdu = -1;
    LOG_D(PDCP,"[%s %d] Config request : Action ADD: Frame %d radio bearer id %d configured\n",
        (enb_flagP) ? "eNB" : "UE", (enb_flagP) ? enb_mod_idP : ue_mod_idP, frameP, rb_idP);
    break;
  case CONFIG_ACTION_MODIFY:
    break;
  case CONFIG_ACTION_REMOVE:
    pdcp_p->instanciated_instance = FALSE;
    pdcp_p->next_pdcp_tx_sn = 0;
    pdcp_p->next_pdcp_rx_sn = 0;
    pdcp_p->tx_hfn = 0;
    pdcp_p->rx_hfn = 0;
    pdcp_p->last_submitted_pdcp_rx_sn = 4095;
    pdcp_p->seq_num_size = 0;
    pdcp_p->first_missing_pdu = -1;
    pdcp_p->security_activated = 0;
    LOG_D(PDCP,"[%s %d] Config request : CONFIG_ACTION_REMOVE: Frame %d radio bearer id %d configured\n",
        (enb_flagP) ? "eNB" : "UE",  (enb_flagP) ? enb_mod_idP : ue_mod_idP, frameP, rb_idP);

    break;
  case CONFIG_ACTION_SET_SECURITY_MODE:
    if ((security_modeP >= 0) && (security_modeP <= 0x77)) {
        pdcp_p->cipheringAlgorithm= security_modeP & 0x0f;
        pdcp_p->integrityProtAlgorithm = (security_modeP>>4) & 0xf;
        LOG_D(PDCP,"[%s %d]Set security mode : CONFIG_ACTION_SET_SECURITY_MODE: Frame %d  cipheringAlgorithm %d integrityProtAlgorithm %d\n",
            (enb_flagP) ? "eNB" : "UE",  (enb_flagP) ? enb_mod_idP : ue_mod_idP, frameP,
                pdcp_p->cipheringAlgorithm,
                pdcp_p->integrityProtAlgorithm );
    } else {
        LOG_D(PDCP,"[%s %d] bad security mode %d", security_modeP);
    }
    break;
  default:
    DevParam(actionP, ue_mod_idP, enb_mod_idP);
    break;
  }
}

// TODO PDCP module initialization code might be removed
int pdcp_module_init (void) {
  //-----------------------------------------------------------------------------
#ifdef NAS_FIFO
  int ret;

  ret=rtf_create(PDCP2NAS_FIFO,32768);

  if (ret < 0) {
      LOG_E(PDCP, "Cannot create PDCP2NAS fifo %d (ERROR %d)\n", PDCP2NAS_FIFO, ret);

      return -1;
  } else {
      LOG_D(PDCP, "Created PDCP2NAS fifo %d\n", PDCP2NAS_FIFO);
      rtf_reset(PDCP2NAS_FIFO);
  }

  ret=rtf_create(NAS2PDCP_FIFO,32768);

  if (ret < 0) {
      LOG_E(PDCP, "Cannot create NAS2PDCP fifo %d (ERROR %d)\n", NAS2PDCP_FIFO, ret);

      return -1;
  } else {
      LOG_D(PDCP, "Created NAS2PDCP fifo %d\n", NAS2PDCP_FIFO);
      rtf_reset(NAS2PDCP_FIFO);
  }

  pdcp_2_nas_irq = 0;
  pdcp_input_sdu_remaining_size_to_read=0;
  pdcp_input_sdu_size_read=0;
#endif

  return 0;

}

//-----------------------------------------------------------------------------
void pdcp_module_cleanup (void)
//-----------------------------------------------------------------------------
{
#ifdef NAS_FIFO
  rtf_destroy(NAS2PDCP_FIFO);
  rtf_destroy(PDCP2NAS_FIFO);
#endif
}

//-----------------------------------------------------------------------------
void pdcp_layer_init(void)
{
  //-----------------------------------------------------------------------------
  module_id_t       instance;
  module_id_t       instance2;
  rb_id_t           rb_id;
#if defined(Rel10)
  mbms_session_id_t session_id;
  mbms_service_id_t service_id;
#endif
  /*
   * Initialize SDU list
   */
  list_init(&pdcp_sdu_list, NULL);

  for (instance = 0; instance < NUMBER_OF_UE_MAX; instance++) {
      for (rb_id = 0; rb_id < maxDRB; rb_id++) {
          memset(&pdcp_array_drb_ue[instance][rb_id], 0, sizeof(pdcp_t));
      }
      for (rb_id = 0; rb_id < 2; rb_id++) {
          memset(&pdcp_array_srb_ue[instance][rb_id], 0, sizeof(pdcp_t));
      }
#if defined(Rel10)
      for (service_id = 0; service_id < maxServiceCount; service_id++) {
          for (session_id = 0; session_id < maxSessionPerPMCH; session_id++) {
              memset(&pdcp_mbms_array_ue[instance][service_id][session_id], 0, sizeof(pdcp_mbms_t));
          }
      }
#endif
  }
  for (instance = 0; instance < NUMBER_OF_eNB_MAX; instance++) {
      for (instance2 = 0; instance2 < NUMBER_OF_UE_MAX; instance2++) {
          for (rb_id = 0; rb_id < maxDRB; rb_id++) {
              memset(&pdcp_array_drb_eNB[instance][instance2][rb_id], 0, sizeof(pdcp_t));
          }
          for (rb_id = 0; rb_id < 2; rb_id++) {
              memset(&pdcp_array_srb_eNB[instance][instance2][rb_id], 0, sizeof(pdcp_t));
          }
      }
#if defined(Rel10)
      for (service_id = 0; service_id < maxServiceCount; service_id++) {
          for (session_id = 0; session_id < maxSessionPerPMCH; session_id++) {
              memset(&pdcp_mbms_array_eNB[instance][service_id][session_id], 0, sizeof(pdcp_mbms_t));
          }
      }
#endif
  }

  LOG_I(PDCP, "PDCP layer has been initialized\n");

  pdcp_output_sdu_bytes_to_write=0;
  pdcp_output_header_bytes_to_write=0;
  pdcp_input_sdu_remaining_size_to_read=0;

  memset(Pdcp_stats_tx, 0, sizeof(Pdcp_stats_tx));
  memset(Pdcp_stats_tx_bytes, 0, sizeof(Pdcp_stats_tx_bytes));
  memset(Pdcp_stats_tx_bytes_last, 0, sizeof(Pdcp_stats_tx_bytes_last));
  memset(Pdcp_stats_tx_rate, 0, sizeof(Pdcp_stats_tx_rate));

  memset(Pdcp_stats_rx, 0, sizeof(Pdcp_stats_rx));
  memset(Pdcp_stats_rx_bytes, 0, sizeof(Pdcp_stats_rx_bytes));
  memset(Pdcp_stats_rx_bytes_last, 0, sizeof(Pdcp_stats_rx_bytes_last));
  memset(Pdcp_stats_rx_rate, 0, sizeof(Pdcp_stats_rx_rate));
}

//-----------------------------------------------------------------------------
void pdcp_layer_cleanup (void)
//-----------------------------------------------------------------------------
{
  list_free (&pdcp_sdu_list);
}

#ifdef NAS_FIFO
EXPORT_SYMBOL(pdcp_2_nas_irq);
#endif //NAS_FIFO
