#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "assertions.h"
#include "intertask_interface.h"

#include "NwGtpv2c.h"
#include "NwGtpv2cIe.h"
#include "NwGtpv2cMsg.h"
#include "NwGtpv2cMsgParser.h"

#include "s11_common.h"
#include "s11_mme_session_manager.h"
#include "s11_ie_formatter.h"

int s11_mme_create_session_request(NwGtpv2cStackHandleT    *stack_p,
                                   SgwCreateSessionRequest *create_session_p)
{
    NwGtpv2cUlpApiT ulp_req;

    NwRcT   rc;
    uint8_t restart_counter = 0;

    DevAssert(stack_p          != NULL);
    DevAssert(create_session_p != NULL);

    memset(&ulp_req, 0, sizeof(NwGtpv2cUlpApiT));

    ulp_req.apiType = NW_GTPV2C_ULP_API_INITIAL_REQ;

    /* Prepare a new Create Session Request msg */
    rc = nwGtpv2cMsgNew(*stack_p,
                        NW_TRUE,
                        NW_GTP_CREATE_SESSION_REQ,
                        create_session_p->teid,
                        0,
                        &(ulp_req.hMsg));

    ulp_req.apiInfo.initialReqInfo.peerIp    = create_session_p->peer_ip;
    ulp_req.apiInfo.initialReqInfo.teidLocal = create_session_p->sender_fteid_for_cp.teid;

    /* Add recovery if contacting the peer for the first time */
    rc = nwGtpv2cMsgAddIe((ulp_req.hMsg), NW_GTPV2C_IE_RECOVERY, 1, 0,
                          (NwU8T*)&restart_counter);
    DevAssert(NW_OK == rc);

    /* Putting the information Elements */
    s11_imsi_ie_set(&(ulp_req.hMsg), &create_session_p->imsi);
    s11_rat_type_ie_set(&(ulp_req.hMsg), &create_session_p->rat_type);
    s11_pdn_type_ie_set(&(ulp_req.hMsg), &create_session_p->pdn_type);

    /* Sender F-TEID for Control Plane (MME S11) */
    rc = nwGtpv2cMsgAddIeFteid((ulp_req.hMsg), NW_GTPV2C_IE_INSTANCE_ZERO,
                               S11_MME_GTP_C,
                               create_session_p->sender_fteid_for_cp.teid,
                               create_session_p->sender_fteid_for_cp.ipv4 ?
                               create_session_p->sender_fteid_for_cp.ipv4_address : 0,
                               create_session_p->sender_fteid_for_cp.ipv6 ?
                               create_session_p->sender_fteid_for_cp.ipv6_address : NULL);
    /* The P-GW TEID should be present on the S11 interface.
     * In case of an initial attach it should be set to 0...
     */
    rc = nwGtpv2cMsgAddIeFteid((ulp_req.hMsg), NW_GTPV2C_IE_INSTANCE_ONE,
                               S5_S8_PGW_GTP_C,
                               create_session_p->pgw_address_for_cp.teid,
                               create_session_p->pgw_address_for_cp.ipv4 ?
                               create_session_p->pgw_address_for_cp.ipv4_address : 0,
                               create_session_p->pgw_address_for_cp.ipv6 ?
                               create_session_p->pgw_address_for_cp.ipv6_address : NULL);

    s11_apn_ie_set(&(ulp_req.hMsg), create_session_p->apn);

    s11_serving_network_ie_set(&(ulp_req.hMsg), &create_session_p->serving_network);

    s11_bearer_context_to_create_ie_set(
        &(ulp_req.hMsg), &create_session_p->bearer_to_create);

    rc = nwGtpv2cProcessUlpReq(*stack_p, &ulp_req);
    DevAssert(NW_OK == rc);

    return 0;
}

int s11_mme_handle_create_session_response(NwGtpv2cStackHandleT *stack_p,
                                           NwGtpv2cUlpApiT      *pUlpApi)
{
    NwRcT  rc = NW_OK;
    NwU8T  offendingIeType, offendingIeInstance;
    NwU16T offendingIeLength;

    SgwCreateSessionResponse *create_session_resp_p;
    MessageDef               *message_p;

    NwGtpv2cMsgParserT *pMsgParser;

    DevAssert(stack_p != NULL);

    message_p = itti_alloc_new_message(TASK_S11, SGW_CREATE_SESSION_RESPONSE);

    create_session_resp_p = &message_p->ittiMsg.sgwCreateSessionResponse;

    /* Create a new message parser */
    rc = nwGtpv2cMsgParserNew(*stack_p, NW_GTP_CREATE_SESSION_RSP,
                              s11_ie_indication_generic,
                              NULL, &pMsgParser);
    DevAssert(NW_OK == rc);

    /* Cause IE */
    rc = nwGtpv2cMsgParserAddIe(pMsgParser,
                                NW_GTPV2C_IE_CAUSE,
                                NW_GTPV2C_IE_INSTANCE_ZERO,
                                NW_GTPV2C_IE_PRESENCE_MANDATORY,
                                s11_cause_ie_get,
                                &create_session_resp_p->cause);
    DevAssert(NW_OK == rc);

    /* Sender FTEID for CP IE */
    rc = nwGtpv2cMsgParserAddIe(pMsgParser, NW_GTPV2C_IE_FTEID,
                                NW_GTPV2C_IE_INSTANCE_ZERO,
                                NW_GTPV2C_IE_PRESENCE_CONDITIONAL,
                                s11_fteid_ie_get,
                                &create_session_resp_p->s11_sgw_teid);
    DevAssert(NW_OK == rc);

    /* Sender FTEID for PGW S5/S8 IE */
    rc = nwGtpv2cMsgParserAddIe(pMsgParser, NW_GTPV2C_IE_FTEID,
                                NW_GTPV2C_IE_INSTANCE_ONE,
                                NW_GTPV2C_IE_PRESENCE_CONDITIONAL,
                                s11_fteid_ie_get,
                                &create_session_resp_p->s5_s8_pgw_teid);
    DevAssert(NW_OK == rc);

    /* PAA IE */
    rc = nwGtpv2cMsgParserAddIe(pMsgParser, NW_GTPV2C_IE_PAA,
                                NW_GTPV2C_IE_INSTANCE_ZERO,
                                NW_GTPV2C_IE_PRESENCE_CONDITIONAL,
                                s11_paa_ie_get,
                                &create_session_resp_p->paa);
    DevAssert(NW_OK == rc);

    /* Bearer Contexts Created IE */
    rc = nwGtpv2cMsgParserAddIe(pMsgParser, NW_GTPV2C_IE_BEARER_CONTEXT,
                                NW_GTPV2C_IE_INSTANCE_ZERO,
                                NW_GTPV2C_IE_PRESENCE_CONDITIONAL,
                                s11_bearer_context_created_ie_get,
                                &create_session_resp_p->bearer_context_created);
    DevAssert(NW_OK == rc);

    /* Run the parser */
    rc = nwGtpv2cMsgParserRun(pMsgParser,
                              (pUlpApi->hMsg),
                              &offendingIeType,
                              &offendingIeInstance,
                              &offendingIeLength);
    if (rc != NW_OK)
    {
        /* TODO: handle this case */
        itti_free(ITTI_MSG_ORIGIN_ID(message_p), message_p);
        message_p = NULL;

        rc = nwGtpv2cMsgParserDelete(*stack_p, pMsgParser);
        DevAssert(NW_OK == rc);

        rc = nwGtpv2cMsgDelete(*stack_p, (pUlpApi->hMsg));
        DevAssert(NW_OK == rc);

        return -1;
    }

    rc = nwGtpv2cMsgParserDelete(*stack_p, pMsgParser);
    DevAssert(NW_OK == rc);

    rc = nwGtpv2cMsgDelete(*stack_p, (pUlpApi->hMsg));
    DevAssert(NW_OK == rc);

    return itti_send_msg_to_task(TASK_MME_APP, INSTANCE_DEFAULT, message_p);
}
