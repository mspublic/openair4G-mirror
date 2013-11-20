/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2012 Eurecom

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
  Forums       : http://forums.eurecom.fr/openairinterface
  Address      : EURECOM, Campus SophiaTech, 450 Route des Chappes
                 06410 Biot FRANCE

*******************************************************************************/

/*! \file s1ap_eNB_decoder.c
 * \brief s1ap pdu decode procedures for eNB
 * \author Sebastien ROUX <sebastien.roux@eurecom.fr>
 * \date 2013
 * \version 0.1
 */

#include <stdio.h>

#include "assertions.h"

#include "intertask_interface.h"

#include "s1ap_common.h"
#include "s1ap_ies_defs.h"
#include "s1ap_eNB_decoder.h"

static int s1ap_eNB_decode_initiating_message(s1ap_message *message,
    S1ap_InitiatingMessage_t *initiating_p)
{
    int ret = -1;
    MessageDef *message_p;
    char       *message_string = NULL;
    size_t      message_string_size;

    DevAssert(initiating_p != NULL);

    message_string = calloc(10000, sizeof(char));

    s1ap_string_total_size = 0;

    message->procedureCode = initiating_p->procedureCode;
    message->criticality   = initiating_p->criticality;

    switch(initiating_p->procedureCode)
    {
        case S1ap_ProcedureCode_id_downlinkNASTransport:
            ret = s1ap_decode_s1ap_downlinknastransporties(
                &message->msg.s1ap_DownlinkNASTransportIEs, &initiating_p->value);
            s1ap_xer_print_s1ap_downlinknastransport(s1ap_xer__print2sp, message_string, message);
            break;

        case S1ap_ProcedureCode_id_InitialContextSetup:
            ret = s1ap_decode_s1ap_initialcontextsetuprequesties(
                &message->msg.s1ap_InitialContextSetupRequestIEs, &initiating_p->value);
            s1ap_xer_print_s1ap_initialcontextsetuprequest(s1ap_xer__print2sp, message_string, message);
            break;

        default:
            S1AP_ERROR("Unknown procedure ID (%d) for initiating message\n",
                       (int)initiating_p->procedureCode);
            return -1;
    }

    message_string_size = strlen(message_string);

    message_p = itti_alloc_new_message_sized(TASK_S1AP, GENERIC_LOG, message_string_size);
    memcpy(&message_p->msg.generic_log, message_string, message_string_size);

    itti_send_msg_to_task(TASK_UNKNOWN, INSTANCE_DEFAULT, message_p);

    free(message_string);

    return ret;
}

static int s1ap_eNB_decode_successful_outcome(s1ap_message *message,
    S1ap_SuccessfulOutcome_t *successfullOutcome_p)
{
    int ret = -1;
    MessageDef *message_p;
    char       *message_string = NULL;
    size_t      message_string_size;

    DevAssert(successfullOutcome_p != NULL);

    message_string = malloc(sizeof(char) * 10000);

    s1ap_string_total_size = 0;

    message->procedureCode = successfullOutcome_p->procedureCode;
    message->criticality   = successfullOutcome_p->criticality;

    switch(successfullOutcome_p->procedureCode)
    {
        case S1ap_ProcedureCode_id_S1Setup:
            ret = s1ap_decode_s1ap_s1setupresponseies(
                &message->msg.s1ap_S1SetupResponseIEs, &successfullOutcome_p->value);
            s1ap_xer_print_s1ap_s1setupresponse(s1ap_xer__print2sp, message_string, message);
            break;

        default:
            S1AP_ERROR("Unknown procedure ID (%d) for successfull outcome message\n",
                       (int)successfullOutcome_p->procedureCode);
            return -1;
    }

    message_string_size = strlen(message_string);

    message_p = itti_alloc_new_message_sized(TASK_S1AP, GENERIC_LOG, message_string_size);
    memcpy(&message_p->msg.generic_log, message_string, message_string_size);

    itti_send_msg_to_task(TASK_UNKNOWN, INSTANCE_DEFAULT, message_p);

    free(message_string);

    return ret;
}

static int s1ap_eNB_decode_unsuccessful_outcome(s1ap_message *message,
    S1ap_UnsuccessfulOutcome_t *unSuccessfullOutcome_p)
{
    int ret = -1;
    DevAssert(unSuccessfullOutcome_p != NULL);

    message->procedureCode = unSuccessfullOutcome_p->procedureCode;
    message->criticality   = unSuccessfullOutcome_p->criticality;

    switch(unSuccessfullOutcome_p->procedureCode) {
        case S1ap_ProcedureCode_id_S1Setup:
            return s1ap_decode_s1ap_s1setupfailureies(
                &message->msg.s1ap_S1SetupFailureIEs, &unSuccessfullOutcome_p->value);

        default:
            S1AP_ERROR("Unknown procedure ID (%d) for unsuccessfull outcome message\n",
                       (int)unSuccessfullOutcome_p->procedureCode);
            break;
    }
    return ret;
}

int s1ap_eNB_decode_pdu(s1ap_message *message, const uint8_t * const buffer,
                        const uint32_t length)
{
    S1AP_PDU_t  pdu;
    S1AP_PDU_t *pdu_p = &pdu;
    asn_dec_rval_t dec_ret;

    DevAssert(buffer != NULL);

    memset((void *)pdu_p, 0, sizeof(S1AP_PDU_t));

    dec_ret = aper_decode(NULL,
                          &asn_DEF_S1AP_PDU,
                          (void **)&pdu_p,
                          buffer,
                          length,
                          0,
                          0);

    if (dec_ret.code != RC_OK) {
        S1AP_ERROR("Failed to decode pdu\n");
        return -1;
    }

    message->direction = pdu_p->present;

    switch(pdu_p->present) {
        case S1AP_PDU_PR_initiatingMessage:
            return s1ap_eNB_decode_initiating_message(message,
                    &pdu_p->choice.initiatingMessage);
        case S1AP_PDU_PR_successfulOutcome:
            return s1ap_eNB_decode_successful_outcome(message,
                    &pdu_p->choice.successfulOutcome);
        case S1AP_PDU_PR_unsuccessfulOutcome:
            return s1ap_eNB_decode_unsuccessful_outcome(message,
                    &pdu_p->choice.unsuccessfulOutcome);
        default:
            S1AP_DEBUG("Unknown presence (%d) or not implemented\n", (int)pdu_p->present);
            break;
    }
    return -1;
}
