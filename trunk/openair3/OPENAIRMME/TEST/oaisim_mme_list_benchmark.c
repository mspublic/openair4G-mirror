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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>

#include "sctp_primitives_client.h"

#include "s1ap_common.h"
#include "s1ap_eNB.h"
#include "s1ap_mme.h"
#include "s1ap_ies_defs.h"

#include "s1ap_eNB_encoder.h"
#include "s1ap_eNB_decoder.h"

#define NB_OF_ENB 10
#define NB_OF_UES 100

static int connected_eNB = 0;
static char ip_addr[] = "127.0.0.1";
static uint8_t id[] = { 0x03, 0x56, 0xf0, 0xd8 };
static char identity[] = { 0x02, 0x08, 0x34 };
static char tac[] = { 0x00, 0x01 };
static char infoNAS[] = { 0x07, 0x42, 0x01, 0xE0, 0x06, 0x00, 0x00, 0xF1, 0x10, 0x00, 0x01, 0x00, 0x2C,
0x52, 0x01, 0xC1, 0x01, 0x09, 0x10, 0x03, 0x77, 0x77, 0x77, 0x07, 0x61, 0x6E, 0x72, 0x69, 0x74,
0x73, 0x75, 0x03, 0x63, 0x6F, 0x6D, 0x05, 0x01, 0x0A, 0x01, 0x20, 0x37, 0x27, 0x0E, 0x80, 0x80,
0x21, 0x0A, 0x03, 0x00, 0x00, 0x0A, 0x81, 0x06, 0x0A, 0x00, 0x00, 0x01, 0x50, 0x0B, 0xF6,
0x00, 0xF1, 0x10, 0x80, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01 };
uint32_t nb_eNB = NB_OF_ENB;
uint32_t nb_ue  = NB_OF_UES;

void s1ap_test_generate_s1_setup_request(uint32_t eNB_id, uint8_t **buffer, uint32_t *length);
int s1ap_test_generate_initial_ue_message(uint32_t eNB_UE_S1AP_ID,
                                          uint8_t **buffer,
                                          uint32_t *length);
int recv_callback(uint32_t assocId,
                  uint32_t stream,
                  uint8_t *buffer,
                  uint32_t length);
int sctp_connected(void *args,
                   uint32_t assocId,
                   uint32_t instreams,
                   uint32_t outstreams);

void s1ap_test_generate_s1_setup_request(uint32_t eNB_id, uint8_t **buffer, uint32_t *length) {
    S1SetupRequestIEs_t s1SetupRequest;
    SupportedTAs_Item_t ta;
    PLMNidentity_t plmnIdentity;
    uint8_t *id_p = (uint8_t*)(&eNB_id + 1);

    memset(&s1SetupRequest, 0, sizeof(S1SetupRequestIEs_t));
    s1SetupRequest.global_ENB_ID.eNB_ID.present = ENB_ID_PR_macroENB_ID;
    s1SetupRequest.global_ENB_ID.eNB_ID.choice.macroENB_ID.buf = id_p;
    s1SetupRequest.global_ENB_ID.eNB_ID.choice.macroENB_ID.size = 3;
    s1SetupRequest.global_ENB_ID.eNB_ID.choice.macroENB_ID.bits_unused = 4;
    OCTET_STRING_fromBuf(&s1SetupRequest.global_ENB_ID.pLMNidentity, identity, 3);

    s1SetupRequest.presenceMask |= S1SETUPREQUESTIES_ENBNAME_PRESENT;
    OCTET_STRING_fromBuf(&s1SetupRequest.eNBname, "ENB 1 eurecom", strlen("ENB 1 eurecom"));

    memset(&ta, 0, sizeof(SupportedTAs_Item_t));
    memset(&plmnIdentity, 0, sizeof(PLMNidentity_t));
    OCTET_STRING_fromBuf(&ta.tAC, tac, 2);
    OCTET_STRING_fromBuf(&plmnIdentity, identity, 3);
    ASN_SEQUENCE_ADD(&ta.broadcastPLMNs, &plmnIdentity);
    ASN_SEQUENCE_ADD(&s1SetupRequest.supportedTAs, &ta);

    s1SetupRequest.defaultPagingDRX = PagingDRX_v64;

    s1ap_eNB_encode_s1_setup_request(&s1SetupRequest, buffer, length);
}

int s1ap_test_generate_initial_ue_message(uint32_t eNB_UE_S1AP_ID,
                                         uint8_t **buffer,
                                         uint32_t *length) {
    InitialUEMessageIEs_t  initialUEmessageIEs;
    InitialUEMessageIEs_t *initialUEmessageIEs_p = &initialUEmessageIEs;

    memset(initialUEmessageIEs_p, 0, sizeof(InitialUEMessageIEs_t));

    initialUEmessageIEs.eNB_UE_S1AP_ID = eNB_UE_S1AP_ID & 0x00ffffff;
    initialUEmessageIEs.nas_pdu.buf = (uint8_t *)infoNAS;
    initialUEmessageIEs.nas_pdu.size = sizeof(infoNAS);

    initialUEmessageIEs.tai.tAC.buf = (uint8_t *)tac;
    initialUEmessageIEs.tai.tAC.size = 2;
    initialUEmessageIEs.tai.pLMNidentity.buf = (uint8_t *)identity;
    initialUEmessageIEs.tai.pLMNidentity.size = 3;
    initialUEmessageIEs.eutran_cgi.pLMNidentity.buf = (uint8_t *)identity;
    initialUEmessageIEs.eutran_cgi.pLMNidentity.size = 3;
    initialUEmessageIEs.eutran_cgi.cell_ID.buf = (uint8_t *)id;
    initialUEmessageIEs.eutran_cgi.cell_ID.size = 4;
    initialUEmessageIEs.eutran_cgi.cell_ID.bits_unused = 4;

    initialUEmessageIEs.rrC_Establishment_Cause = RRC_Establishment_Cause_mo_Data;

    return s1ap_eNB_encode_initial_ue_message(initialUEmessageIEs_p, buffer, length);
}

int recv_callback(uint32_t  assocId,
                  uint32_t  stream,
                  uint8_t  *buffer,
                  uint32_t  length) {
    s1ap_message message;
    uint8_t *buffer2;
    uint32_t len;
    int j;

    if (s1ap_eNB_decode_pdu(&message, buffer, length) < 0) {
        fprintf(stderr, "s1ap_eNB_decode_pdu returned status < 0\n");
        free(buffer);
        return -1;
    }

    if (message.procedureCode == ProcedureCode_id_S1Setup
        && message.direction == S1AP_PDU_PR_successfulOutcome) {
        for (j = 0; j < nb_ue; j++) {
            s1ap_test_generate_initial_ue_message(j, &buffer2, &len);
            if (sctp_send_msg(assocId, j % 64 + 1, buffer2, len) < 0) {
                fprintf(stderr, "sctp_send_msg returned status < 0\nSomething bad happened on SCTP layer\n");
                free(buffer2);
                break;
            }
            free(buffer2);
        }
    } else if (message.procedureCode == ProcedureCode_id_InitialContextSetup
               && message.direction == S1AP_PDU_PR_initiatingMessage) {
        fprintf(stdout, "Received InitialContextSetup request\n");
    } else {
        fprintf(stderr, "Received unexpected message %d %d\n", message.procedureCode, message.direction);
        free(buffer);
        return -1;
    }

    free(buffer);

    return 0;
}

int sctp_connected(void     *args,
                   uint32_t  assocId,
                   uint32_t  instreams,
                   uint32_t  outstreams) {
    uint8_t *buffer;
    uint32_t len;

    fprintf(stdout, "New association %d\n", assocId);

    s1ap_test_generate_s1_setup_request(assocId, &buffer, &len);

    if (sctp_send_msg(assocId, 0, buffer, len) < 0)
    {
        free(buffer);
        fprintf(stderr, "sctp_send_msg returned status < 0. Something bad happened on SCTP layer\n");
        exit(0);
    }
    free(buffer);

    connected_eNB++;
    return 0;
}

int main(int argc, char *argv[])
{
    asn_enc_rval_t retVal;

    int i;

    SupportedTAs_Item_t ta;
    PLMNidentity_t plmnIdentity;

    asn_debug = 0;
    asn1_xer_print = 0;

    if (argc > 1) {
        nb_eNB = atoi(argv[1]);
        if (argc > 2)
            nb_ue = atoi(argv[2]);
    }

    for (i = 0; i < nb_eNB; i++) {
        sctp_connect_to_remote_host(ip_addr, 36412, NULL, sctp_connected, recv_callback);
    }
    while (1) {
        sleep(1);
    }


//     generateUplinkNASTransport(&buffer, &len);
//     sctp_send_msg(assoc[0], 0, buffer, len);
//     s1ap_mme_decode_pdu(buffer, len);

    sctp_terminate();

    return(0);
}