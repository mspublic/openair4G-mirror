/*****************************************************************************
            Eurecom OpenAirInterface 3
            Copyright(c) 2012 Eurecom

Source      emm_as.c

Version     0.1

Date        2012/10/16

Product     NAS stack

Subsystem   EPS Mobility Management

Author      Frederic Maurel

Description Defines the EMMAS Service Access Point that provides
        services to the EPS Mobility Management for NAS message
        transfer to/from the Access Stratum sublayer.

*****************************************************************************/

#include "emm_as.h"
#include "emm_recv.h"
#include "emm_send.h"
#include "commonDef.h"
#include "nas_log.h"

#include "TLVDecoder.h"
#include "as_message.h"
#include "nas_message.h"

#include "emm_cause.h"
#include "LowerLayer.h"

#include <string.h> // memset
#include <stdlib.h> // malloc, free

#if (defined(EPC_BUILD) && defined(NAS_MME)) || (defined(UE_BUILD) && defined(NAS_UE))
# include "nas_itti_messaging.h"
#endif

/****************************************************************************/
/****************  E X T E R N A L    D E F I N I T I O N S  ****************/
/****************************************************************************/

#ifdef NAS_UE
extern int emm_proc_plmn_selection_end(int found, tac_t tac, ci_t ci, AcT_t rat);
#endif

extern int emm_proc_status(unsigned int ueid, int emm_cause);

/****************************************************************************/
/*******************  L O C A L    D E F I N I T I O N S  *******************/
/****************************************************************************/

/*
 * String representation of EMMAS-SAP primitives
 */
static const char *_emm_as_primitive_str[] = {
    "EMMAS_SECURITY_REQ",
    "EMMAS_SECURITY_IND",
    "EMMAS_SECURITY_RES",
    "EMMAS_SECURITY_REJ",
    "EMMAS_ESTABLISH_REQ",
    "EMMAS_ESTABLISH_CNF",
    "EMMAS_ESTABLISH_REJ",
    "EMMAS_RELEASE_REQ",
    "EMMAS_RELEASE_IND",
    "EMMAS_DATA_REQ",
    "EMMAS_DATA_IND",
    "EMMAS_PAGE_IND",
    "EMMAS_STATUS_IND",
    "EMMAS_CELL_INFO_REQ",
    "EMMAS_CELL_INFO_RES",
    "EMMAS_CELL_INFO_IND",
};

/*
 * Functions executed to process EMM procedures upon receiving
 * data from the network
 */
static int _emm_as_recv(unsigned int ueid, const char *msg, int len,
                        int *emm_cause);

#ifdef NAS_UE
static int _emm_as_establish_cnf(const emm_as_establish_t *msg, int *emm_cause);
static int _emm_as_establish_rej(void);
static int _emm_as_release_ind(const emm_as_release_t *msg);
static int _emm_as_page_ind(const emm_as_page_t *msg);
#endif

#ifdef NAS_MME
static int _emm_as_establish_req(const emm_as_establish_t *msg, int *emm_cause);
#endif

static int _emm_as_cell_info_res(const emm_as_cell_info_t *msg);
static int _emm_as_cell_info_ind(const emm_as_cell_info_t *msg);

static int _emm_as_data_ind(const emm_as_data_t *msg, int *emm_cause);

/*
 * Functions executed to send data to the network when requested
 * within EMM procedure processing
 */
static EMM_msg *_emm_as_set_header(nas_message_t *msg,
                                   const emm_as_security_data_t *security);
static int _emm_as_encode(as_nas_info_t *info, nas_message_t *msg, int length);
static int _emm_as_encrypt(as_nas_info_t *info,
                           const nas_message_security_header_t *header, const char *buffer, int length);
static int _emm_as_send(const emm_as_t *msg);

#ifdef NAS_UE
static int _emm_as_security_res(const emm_as_security_t *,
                                ul_info_transfer_req_t *);
static int _emm_as_establish_req(const emm_as_establish_t *,
                                 nas_establish_req_t *);
#endif

#ifdef NAS_MME
static int _emm_as_security_req(const emm_as_security_t *,
                                dl_info_transfer_req_t *);
static int _emm_as_security_rej(const emm_as_security_t *,
                                dl_info_transfer_req_t *);
static int _emm_as_establish_cnf(const emm_as_establish_t *,
                                 nas_establish_rsp_t *);
static int _emm_as_establish_rej(const emm_as_establish_t *,
                                 nas_establish_rsp_t *);
static int _emm_as_page_ind(const emm_as_page_t *, paging_req_t *);
#endif

static int _emm_as_cell_info_req(const emm_as_cell_info_t *, cell_info_req_t *);

static int _emm_as_data_req(const emm_as_data_t *, ul_info_transfer_req_t *);
static int _emm_as_status_ind(const emm_as_status_t *, ul_info_transfer_req_t *);
static int _emm_as_release_req(const emm_as_release_t *, nas_release_req_t *);

/****************************************************************************/
/******************  E X P O R T E D    F U N C T I O N S  ******************/
/****************************************************************************/

/****************************************************************************
 **                                                                        **
 ** Name:    emm_as_initialize()                                       **
 **                                                                        **
 ** Description: Initializes the EMMAS Service Access Point                **
 **                                                                        **
 ** Inputs:  None                                                      **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    None                                       **
 **      Others:    NONE                                       **
 **                                                                        **
 ***************************************************************************/
void emm_as_initialize(void)
{
    LOG_FUNC_IN;

    /* TODO: Initialize the EMMAS-SAP */

    LOG_FUNC_OUT;
}

/****************************************************************************
 **                                                                        **
 ** Name:    emm_as_send()                                             **
 **                                                                        **
 ** Description: Processes the EMMAS Service Access Point primitive.       **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_as_send(const emm_as_t *msg)
{
    LOG_FUNC_IN;

    int rc;
    int emm_cause = EMM_CAUSE_SUCCESS;
    emm_as_primitive_t primitive = msg->primitive;

    UInt32_t ueid = 0;

    LOG_TRACE(INFO, "EMMAS-SAP - Received primitive %s (%d)",
              _emm_as_primitive_str[primitive - _EMMAS_START - 1], primitive);

    switch (primitive) {
        case _EMMAS_DATA_IND:
            rc = _emm_as_data_ind(&msg->u.data, &emm_cause);
            ueid = msg->u.data.ueid;
            break;

#ifdef NAS_UE
        case _EMMAS_ESTABLISH_CNF:
            rc = _emm_as_establish_cnf(&msg->u.establish, &emm_cause);
            break;

        case _EMMAS_ESTABLISH_REJ:
            rc = _emm_as_establish_rej();
            break;

        case _EMMAS_RELEASE_IND:
            rc = _emm_as_release_ind(&msg->u.release);
            break;

        case _EMMAS_PAGE_IND:
            rc = _emm_as_page_ind(&msg->u.page);
            break;
#endif
#ifdef NAS_MME
        case _EMMAS_ESTABLISH_REQ:
            rc = _emm_as_establish_req(&msg->u.establish, &emm_cause);
            ueid = msg->u.establish.ueid;
            break;
#endif
        case _EMMAS_CELL_INFO_RES:
            rc = _emm_as_cell_info_res(&msg->u.cell_info);
            break;

        case _EMMAS_CELL_INFO_IND:
            rc = _emm_as_cell_info_ind(&msg->u.cell_info);
            break;

        default:
            /* Other primitives are forwarded to the Access Stratum */
            rc = _emm_as_send(msg);
            if (rc != RETURNok) {
                LOG_TRACE(ERROR, "EMMAS-SAP - "
                          "Failed to process primitive %s (%d)",
                          _emm_as_primitive_str[primitive - _EMMAS_START - 1],
                          primitive);
                LOG_FUNC_RETURN (RETURNerror);
            }
            break;
    }

    /* Handle decoding errors */
    if (emm_cause != EMM_CAUSE_SUCCESS) {
        /* Ignore received message that is too short to contain a complete
         * message type information element */
        if (rc == TLV_DECODE_BUFFER_TOO_SHORT) {
            LOG_FUNC_RETURN (RETURNok);
        }
        /* Ignore received message that contains not supported protocol
         * discriminator */
        else if (rc == TLV_DECODE_PROTOCOL_NOT_SUPPORTED) {
            LOG_FUNC_RETURN (RETURNok);
        } else if (rc == TLV_DECODE_WRONG_MESSAGE_TYPE) {
            emm_cause = EMM_CAUSE_MESSAGE_TYPE_NOT_IMPLEMENTED;
        }
        /* EMM message processing failed */
        LOG_TRACE(WARNING, "EMMAS-SAP - Received EMM message is not valid "
                  "(cause=%d)", emm_cause);
        /* Return an EMM status message */
        rc = emm_proc_status(ueid, emm_cause);
    }

    if (rc != RETURNok) {
        LOG_TRACE(ERROR, "EMMAS-SAP - Failed to process primitive %s (%d)",
                  _emm_as_primitive_str[primitive - _EMMAS_START - 1],
                  primitive);
    }

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************/
/*********************  L O C A L    F U N C T I O N S  *********************/
/****************************************************************************/

/*
 * --------------------------------------------------------------------------
 * Functions executed to process EMM procedures upon receiving data from the
 * network
 * --------------------------------------------------------------------------
 */

/****************************************************************************
 **                                                                        **
 ** Name:    _emm_as_recv()                                            **
 **                                                                        **
 ** Description: Decodes and processes the EPS Mobility Management message **
 **      received from the Access Stratum                          **
 **                                                                        **
 ** Inputs:  ueid:      UE lower layer identifier                  **
 **      msg:       The EMM message to process                 **
 **      len:       The length of the EMM message              **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     emm_cause: EMM cause code                             **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_as_recv(unsigned int ueid, const char *msg, int len,
                        int *emm_cause)
{
    LOG_FUNC_IN;

    int decoder_rc;
    int rc = RETURNerror;

    LOG_TRACE(INFO, "EMMAS-SAP - Received EMM message (length=%d)", len);

    nas_message_t nas_msg;
    memset(&nas_msg, 0 , sizeof(nas_message_t));

    /* Decode the received message */
    decoder_rc = nas_message_decode(msg, &nas_msg, len);

    if (decoder_rc < 0) {
        LOG_TRACE(WARNING, "EMMAS-SAP - Failed to decode NAS message "
                  "(err=%d)", decoder_rc);
        *emm_cause = EMM_CAUSE_PROTOCOL_ERROR;
        LOG_FUNC_RETURN (decoder_rc);
    }

    /* Process NAS message */
    EMM_msg *emm_msg = &nas_msg.plain.emm;
    switch (emm_msg->header.message_type) {
        case EMM_STATUS:
            rc = emm_recv_status(ueid, &emm_msg->emm_status, emm_cause);
            break;

#ifdef NAS_UE
        case IDENTITY_REQUEST:
            rc = emm_recv_identity_request(&emm_msg->identity_request,
                                           emm_cause);
            break;

        case AUTHENTICATION_REQUEST:
            rc = emm_recv_authentication_request(
                     &emm_msg->authentication_request,
                     emm_cause);
            break;

        case AUTHENTICATION_REJECT:
            rc = emm_recv_authentication_reject(
                     &emm_msg->authentication_reject,
                     emm_cause);
            break;

        case SECURITY_MODE_COMMAND:
            rc = emm_recv_security_mode_command(
                     &emm_msg->security_mode_command,
                     emm_cause);
            break;

        case DETACH_ACCEPT:
            rc = emm_recv_detach_accept(&emm_msg->detach_accept, emm_cause);
            break;
#endif

#ifdef NAS_UE
        case TRACKING_AREA_UPDATE_ACCEPT:
        case TRACKING_AREA_UPDATE_REJECT:
        case SERVICE_REJECT:
        case GUTI_REALLOCATION_COMMAND:
        case EMM_INFORMATION:
        case DOWNLINK_NAS_TRANSPORT:
        case CS_SERVICE_NOTIFICATION:
            /* TODO */
            break;
#endif
#ifdef NAS_MME
        case IDENTITY_RESPONSE:
            rc = emm_recv_identity_response(ueid,
                                            &emm_msg->identity_response,
                                            emm_cause);
            break;

        case AUTHENTICATION_RESPONSE:
            rc = emm_recv_authentication_response(ueid,
                                                  &emm_msg->authentication_response,
                                                  emm_cause);
            break;

        case AUTHENTICATION_FAILURE:
            rc = emm_recv_authentication_failure(ueid,
                                                 &emm_msg->authentication_failure,
                                                 emm_cause);
            break;

        case SECURITY_MODE_COMPLETE:
            rc = emm_recv_security_mode_complete(ueid,
                                                 &emm_msg->security_mode_complete,
                                                 emm_cause);
            break;

        case SECURITY_MODE_REJECT:
            rc = emm_recv_security_mode_reject(ueid,
                                               &emm_msg->security_mode_reject,
                                               emm_cause);
            break;

        case ATTACH_COMPLETE:
            rc = emm_recv_attach_complete(ueid, &emm_msg->attach_complete,
                                          emm_cause);
            break;

        case TRACKING_AREA_UPDATE_COMPLETE:
        case GUTI_REALLOCATION_COMPLETE:
        case UPLINK_NAS_TRANSPORT:
            /* TODO */
            break;

        case DETACH_REQUEST:
            rc = emm_recv_detach_request(ueid, &emm_msg->detach_request,
                                         emm_cause);
            break;
#endif

        default:
            LOG_TRACE(WARNING, "EMMAS-SAP - EMM message 0x%x is not valid",
                      emm_msg->header.message_type);
            *emm_cause = EMM_CAUSE_MESSAGE_TYPE_NOT_COMPATIBLE;
            break;
    }

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:    _emm_as_data_ind()                                        **
 **                                                                        **
 ** Description: Processes the EMMAS-SAP data transfer indication          **
 **      primitive                                                 **
 **                                                                        **
 ** EMMAS-SAP - AS->EMM: DATA_IND - Data transfer procedure                **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     emm_cause: EMM cause code                             **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_as_data_ind(const emm_as_data_t *msg, int *emm_cause)
{
    LOG_FUNC_IN;

    int rc = RETURNerror;

    LOG_TRACE(INFO, "EMMAS-SAP - Received AS data transfer indication "
              "(ueid=%u, delivered=%s, length=%d)", msg->ueid,
              (msg->delivered)? "TRUE" : "FALSE", msg->NASmsg.length);

    if (msg->delivered) {
        if (msg->NASmsg.length > 0) {
            /* Process the received NAS message */
            char *plain_msg = (char *)malloc(msg->NASmsg.length);
            if (plain_msg) {
                nas_message_security_header_t header;
                /* Decrypt the received security protected message */
                int bytes = nas_message_decrypt((char *)(msg->NASmsg.value),
                                                plain_msg, &header,
                                                msg->NASmsg.length);
                if (bytes < 0) {
                    /* Failed to decrypt the message */
                    *emm_cause = EMM_CAUSE_PROTOCOL_ERROR;
                    LOG_FUNC_RETURN (bytes);
                } else if (header.protocol_discriminator ==
                           EPS_MOBILITY_MANAGEMENT_MESSAGE) {
                    /* Process EMM data */
                    rc = _emm_as_recv(msg->ueid, plain_msg, bytes, emm_cause);
                } else if (header.protocol_discriminator ==
                           EPS_SESSION_MANAGEMENT_MESSAGE) {
                    const OctetString data = {bytes, (uint8_t *)plain_msg};
                    /* Foward ESM data to EPS session management */
                    rc = lowerlayer_data_ind(msg->ueid, &data);
                }
                free(plain_msg);
            }
        } else {
            /* Process successfull lower layer transfer indication */
            rc = lowerlayer_success(msg->ueid);
        }
    } else {
        /* Process lower layer transmission failure of NAS message */
        rc = lowerlayer_failure(msg->ueid);
    }

    LOG_FUNC_RETURN (rc);
}

#ifdef NAS_UE
/****************************************************************************
 **                                                                        **
 ** Name:    _emm_as_establish_cnf()                                   **
 **                                                                        **
 ** Description: Processes the EMMAS-SAP connection establish confirmation **
 **      primitive                                                 **
 **                                                                        **
 ** EMMAS-SAP - AS->EMM: ESTABLISH_CNF - NAS signalling connection         **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     emm_cause: EMM cause code                             **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_as_establish_cnf(const emm_as_establish_t *msg,
                                 int *emm_cause)
{
    LOG_FUNC_IN;

    int decoder_rc;
    int rc = RETURNerror;

    LOG_TRACE(INFO, "EMMAS-SAP - Received AS connection establish confirm");

    if (msg->NASmsg.length > 0) {
        /* The NAS signalling connection is established */
        (void) lowerlayer_establish();
    } else {
        /* The initial NAS message has been successfully delivered to
         * lower layers */
        rc = lowerlayer_success(0);
        LOG_FUNC_RETURN (rc);
    }

    nas_message_t nas_msg;
    memset(&nas_msg, 0 , sizeof(nas_message_t));

    /* Decode initial NAS message */
    decoder_rc = nas_message_decode((char *)(msg->NASmsg.value), &nas_msg,
                                    msg->NASmsg.length);
    if (decoder_rc < 0) {
        LOG_TRACE(WARNING, "EMMAS-SAP - Failed to decode initial NAS message"
                  "(err=%d)", decoder_rc);
        *emm_cause = EMM_CAUSE_PROTOCOL_ERROR;
        LOG_FUNC_RETURN (decoder_rc);
    }

    /* Process initial NAS message */
    EMM_msg *emm_msg = &nas_msg.plain.emm;
    switch (emm_msg->header.message_type) {
        case ATTACH_ACCEPT:
            rc = emm_recv_attach_accept(&emm_msg->attach_accept, emm_cause);
            break;

        case ATTACH_REJECT:
            rc = emm_recv_attach_reject(&emm_msg->attach_reject, emm_cause);
            break;

        case DETACH_ACCEPT:
            break;

        case TRACKING_AREA_UPDATE_ACCEPT:
            break;

        default:
            LOG_TRACE(WARNING, "EMMAS-SAP - Initial NAS message 0x%x is "
                      "not valid", emm_msg->header.message_type);
            *emm_cause = EMM_CAUSE_MESSAGE_TYPE_NOT_COMPATIBLE;
            break;
    }

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:    _emm_as_establish_rej()                                   **
 **                                                                        **
 ** Description: Processes the EMMAS-SAP connection establish reject       **
 **      primitive                                                 **
 **                                                                        **
 ** EMMAS-SAP - AS->EMM: ESTABLISH_REJ - NAS signalling connection         **
 **                                                                        **
 ** Inputs:  None                                                      **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_as_establish_rej(void)
{
    LOG_FUNC_IN;

    int rc;

    LOG_TRACE(INFO, "EMMAS-SAP - Received AS initial NAS message transmission "
              "failure");

    /* Process lower layer transmission failure of initial NAS message */
    rc = lowerlayer_failure(0);

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:    _emm_as_release_ind()                                     **
 **                                                                        **
 ** Description: Processes the EMMAS-SAP connection release indication     **
 **      primitive                                                 **
 **                                                                        **
 ** EMMAS-SAP - AS->EMM: RELEASE_IND - NAS signalling release procedure    **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_as_release_ind(const emm_as_release_t *msg)
{
    LOG_FUNC_IN;

    int rc = RETURNok;

    LOG_TRACE(INFO, "EMMAS-SAP - Received AS connection release indication "
              "(cause=%d)", msg->cause);

    /* Process NAS signalling connection release indication */
    rc = lowerlayer_release(msg->cause);

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:    _emm_as_page_ind()                                        **
 **                                                                        **
 ** Description: Processes the EMMAS-SAP paging data indication primitive  **
 **                                                                        **
 ** EMMAS-SAP - AS->EMM: PAGE_IND - Paging data procedure                  **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_as_page_ind(const emm_as_page_t *msg)
{
    LOG_FUNC_IN;

    int rc = RETURNok;

    LOG_TRACE(INFO, "EMMAS-SAP - Received AS paging data indication");

    /* TODO */

    LOG_FUNC_RETURN (rc);
}

#endif

#ifdef NAS_MME
/****************************************************************************
 **                                                                        **
 ** Name:    _emm_as_establish_req()                                   **
 **                                                                        **
 ** Description: Processes the EMMAS-SAP connection establish request      **
 **      primitive                                                 **
 **                                                                        **
 ** EMMAS-SAP - AS->EMM: ESTABLISH_REQ - NAS signalling connection         **
 **     The AS notifies the NAS that establishment of the signal-  **
 **     ling connection has been requested to tranfer initial NAS  **
 **     message from the UE.                                       **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     emm_cause: EMM cause code                             **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_as_establish_req(const emm_as_establish_t *msg, int *emm_cause)
{
    LOG_FUNC_IN;

    int decoder_rc;
    int rc = RETURNerror;

    LOG_TRACE(INFO, "EMMAS-SAP - Received AS connection establish request");

    nas_message_t nas_msg;
    memset(&nas_msg, 0 , sizeof(nas_message_t));

    /* Decode initial NAS message */
    decoder_rc = nas_message_decode((char *)(msg->NASmsg.value), &nas_msg,
                                    msg->NASmsg.length);

    if (decoder_rc < TLV_DECODE_FATAL_ERROR) {
        *emm_cause = EMM_CAUSE_PROTOCOL_ERROR;
        LOG_FUNC_RETURN (decoder_rc);
    } else if (decoder_rc == TLV_DECODE_UNEXPECTED_IEI) {
        *emm_cause = EMM_CAUSE_IE_NOT_IMPLEMENTED;
    } else if (decoder_rc < 0) {
        *emm_cause = EMM_CAUSE_PROTOCOL_ERROR;
    }

    /* Process initial NAS message */
    EMM_msg *emm_msg = &nas_msg.plain.emm;
    switch (emm_msg->header.message_type) {
        case ATTACH_REQUEST:
            rc = emm_recv_attach_request(msg->ueid, &emm_msg->attach_request,
                                         emm_cause);
            break;

        case DETACH_REQUEST:
            rc = RETURNok;  /* TODO */
            break;

        case TRACKING_AREA_UPDATE_REQUEST:
            rc = RETURNok;  /* TODO */
            break;

        case SERVICE_REQUEST:
            rc = RETURNok;  /* TODO */
            break;

        case EXTENDED_SERVICE_REQUEST:
            rc = RETURNok;  /* TODO */
            break;

        default:
            LOG_TRACE(WARNING, "EMMAS-SAP - Initial NAS message 0x%x is "
                      "not valid", emm_msg->header.message_type);
            *emm_cause = EMM_CAUSE_MESSAGE_TYPE_NOT_COMPATIBLE;
            break;
    }

    LOG_FUNC_RETURN (rc);
}
#endif

/****************************************************************************
 **                                                                        **
 ** Name:    _emm_as_cell_info_res()                                   **
 **                                                                        **
 ** Description: Processes the EMMAS-SAP cell information response         **
 **      primitive                                                 **
 **                                                                        **
 ** EMMAS-SAP - AS->EMM: CELL_INFO_RES - PLMN and cell selection procedure **
 **     The NAS received a response to cell selection request pre- **
 **     viously sent to the Access-Startum. If a suitable cell is  **
 **     found to serve the selected PLMN with associated Radio Ac- **
 **     cess Technologies, this cell is selected to camp on.       **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_as_cell_info_res(const emm_as_cell_info_t *msg)
{
    LOG_FUNC_IN;

    int rc = RETURNok;

    LOG_TRACE(INFO, "EMMAS-SAP - Received AS cell information response");

#ifdef NAS_UE
    int AcT = NET_ACCESS_EUTRAN;
    if (msg->found == TRUE) {
        /* Get the first supported access technology */
        while (AcT != NET_ACCESS_UNAVAILABLE) {
            if (msg->rat & (1 << AcT)) {
                break;
            }
            AcT -= 1;
        }
    }
    /* Notify EMM that a cell has been found */
    rc = emm_proc_plmn_selection_end(msg->found, msg->tac, msg->cellID, AcT);
#endif

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:    _emm_as_cell_info_ind()                                   **
 **                                                                        **
 ** Description: Processes the EMMAS-SAP cell information indication       **
 **      primitive                                                 **
 **                                                                        **
 ** EMMAS-SAP - AS->EMM: CELL_INFO_IND - PLMN and cell selection procedure **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_as_cell_info_ind(const emm_as_cell_info_t *msg)
{
    LOG_FUNC_IN;

    int rc = RETURNok;

    LOG_TRACE(INFO, "EMMAS-SAP - Received AS cell information indication");

    /* TODO */

    LOG_FUNC_RETURN (rc);
}

/*
 * --------------------------------------------------------------------------
 * Functions executed to send data to the network when requested within EMM
 * procedure processing
 * --------------------------------------------------------------------------
 */

/****************************************************************************
 **                                                                        **
 ** Name:    _emm_as_set_header()                                      **
 **                                                                        **
 ** Description: Setup the security header of the given NAS message        **
 **                                                                        **
 ** Inputs:  security:  The NAS security data to use               **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     msg:       The NAS message                            **
 **      Return:    Pointer to the plain NAS message to be se- **
 **             curity protected if setting of the securi- **
 **             ty header succeed;                         **
 **             NULL pointer otherwise                     **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static EMM_msg *_emm_as_set_header(nas_message_t *msg,
                                   const emm_as_security_data_t *security)
{
    LOG_FUNC_IN;

    msg->header.protocol_discriminator = EPS_MOBILITY_MANAGEMENT_MESSAGE;

    if ( security && (security->ksi != EMM_AS_NO_KEY_AVAILABLE) ) {
        /* A valid EPS security context exists */
        if (security->is_new) {
            /* New EPS security context is taken into use */
            if (security->k_int) {
                if (security->k_enc) {
                    /* NAS integrity and cyphering keys are available */
                    msg->header.security_header_type =
                        SECURITY_HEADER_TYPE_INTEGRITY_PROTECTED_CYPHERED_NEW;
                } else {
                    /* NAS integrity key only is available */
                    msg->header.security_header_type =
                        SECURITY_HEADER_TYPE_INTEGRITY_PROTECTED_NEW;
                }
                LOG_FUNC_RETURN (&msg->protected.plain.emm);
            }
        } else if (security->k_int) {
            if (security->k_enc) {
                /* NAS integrity and cyphering keys are available */
                msg->header.security_header_type =
                    SECURITY_HEADER_TYPE_INTEGRITY_PROTECTED_CYPHERED;
            } else {
                /* NAS integrity key only is available */
                msg->header.security_header_type =
                    SECURITY_HEADER_TYPE_INTEGRITY_PROTECTED;
            }
            LOG_FUNC_RETURN (&msg->protected.plain.emm);
        }
    } else {
        /* No valid EPS security context exists */
        msg->header.security_header_type = SECURITY_HEADER_TYPE_NOT_PROTECTED;
        LOG_FUNC_RETURN (&msg->plain.emm);
    }

    /* A valid EPS security context exists but NAS integrity key
     * is not available */
    LOG_FUNC_RETURN (NULL);
}

/****************************************************************************
 **                                                                        **
 ** Name:    _emm_as_encode()                                          **
 **                                                                        **
 ** Description: Encodes NAS message into NAS information container        **
 **                                                                        **
 ** Inputs:  msg:       The NAS message to encode                  **
 **      length:    The maximum length of the NAS message      **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     info:      The NAS information container              **
 **      msg:       The NAS message to encode                  **
 **      Return:    The number of bytes successfully encoded   **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_as_encode(as_nas_info_t *info, nas_message_t *msg, int length)
{
    LOG_FUNC_IN;

    int bytes = 0;

    if (msg->header.security_header_type != SECURITY_HEADER_TYPE_NOT_PROTECTED) {
        emm_msg_header_t *header = &msg->protected.plain.emm.header;
        /* Expand size of protected NAS message */
        length += NAS_MESSAGE_SECURITY_HEADER_SIZE;
        /* Set header of plain NAS message */
        header->protocol_discriminator = EPS_MOBILITY_MANAGEMENT_MESSAGE;
        header->security_header_type = SECURITY_HEADER_TYPE_NOT_PROTECTED;
    }
    /* Allocate memory to the NAS information container */
    info->data = (Byte_t *)malloc(length * sizeof(Byte_t));
    if (info->data != NULL) {
        /* Encode the NAS message */
        bytes = nas_message_encode((char *)(info->data), msg, length);
        if (bytes > 0) {
            info->length = bytes;
        } else {
            free(info->data);
            info->length = 0;
            info->data = NULL;
        }
    }

    LOG_FUNC_RETURN (bytes);
}

/****************************************************************************
 **                                                                        **
 ** Name:    _emm_as_encrypt()                                         **
 **                                                                        **
 ** Description: Encryts NAS message into NAS information container        **
 **                                                                        **
 ** Inputs:  header:    The Security header in used                **
 **      msg:       The NAS message to encrypt                 **
 **      length:    The maximum length of the NAS message      **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     info:      The NAS information container              **
 **      Return:    The number of bytes successfully encrypted **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_as_encrypt(as_nas_info_t *info,
                           const nas_message_security_header_t *header,
                           const char *msg, int length)
{
    LOG_FUNC_IN;

    int bytes = 0;

    if (header->security_header_type != SECURITY_HEADER_TYPE_NOT_PROTECTED) {
        /* Expand size of protected NAS message */
        length += NAS_MESSAGE_SECURITY_HEADER_SIZE;
    }
    /* Allocate memory to the NAS information container */
    info->data = (Byte_t *)malloc(length * sizeof(Byte_t));
    if (info->data != NULL) {
        /* Encrypt the NAS information message */
        bytes = nas_message_encrypt(msg, (char *)(info->data), header, length);
        if (bytes > 0) {
            info->length = bytes;
        } else {
            free(info->data);
            info->length = 0;
            info->data = NULL;
        }
    }

    LOG_FUNC_RETURN (bytes);
}

/****************************************************************************
 **                                                                        **
 ** Name:    _emm_as_send()                                            **
 **                                                                        **
 ** Description: Builds NAS message according to the given EMMAS Service   **
 **      Access Point primitive and sends it to the Access Stratum **
 **      sublayer                                                  **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to be sent         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_as_send(const emm_as_t *msg)
{
    LOG_FUNC_IN;

    as_message_t as_msg;
    memset(&as_msg, 0 , sizeof(as_message_t));

    switch (msg->primitive) {
        case _EMMAS_DATA_REQ:
            as_msg.msgID = _emm_as_data_req(&msg->u.data,
                                            &as_msg.msg.ul_info_transfer_req);
            break;

        case _EMMAS_STATUS_IND:
            as_msg.msgID = _emm_as_status_ind(&msg->u.status,
                                              &as_msg.msg.ul_info_transfer_req);
            break;

        case _EMMAS_RELEASE_REQ:
            as_msg.msgID = _emm_as_release_req(&msg->u.release,
                                               &as_msg.msg.nas_release_req);
            break;

#ifdef NAS_UE
        case _EMMAS_SECURITY_RES:
            as_msg.msgID = _emm_as_security_res(&msg->u.security,
                                                &as_msg.msg.ul_info_transfer_req);
            break;

        case _EMMAS_ESTABLISH_REQ:
            as_msg.msgID = _emm_as_establish_req(&msg->u.establish,
                                                 &as_msg.msg.nas_establish_req);
            break;

#endif
#ifdef NAS_MME
        case _EMMAS_SECURITY_REQ:
            as_msg.msgID = _emm_as_security_req(&msg->u.security,
                                                &as_msg.msg.dl_info_transfer_req);
            break;

        case _EMMAS_SECURITY_REJ:
            as_msg.msgID = _emm_as_security_rej(&msg->u.security,
                                                &as_msg.msg.dl_info_transfer_req);
            break;

        case _EMMAS_ESTABLISH_CNF:
            as_msg.msgID = _emm_as_establish_cnf(&msg->u.establish,
                                                 &as_msg.msg.nas_establish_rsp);
            break;

        case _EMMAS_ESTABLISH_REJ:
            as_msg.msgID = _emm_as_establish_rej(&msg->u.establish,
                                                 &as_msg.msg.nas_establish_rsp);
            break;

        case _EMMAS_PAGE_IND:
            as_msg.msgID = _emm_as_page_ind(&msg->u.page,
                                            &as_msg.msg.paging_req);
            break;

#endif
        case _EMMAS_CELL_INFO_REQ:
            as_msg.msgID = _emm_as_cell_info_req(&msg->u.cell_info,
                                                 &as_msg.msg.cell_info_req);
            /*
             * TODO: NAS may provide a list of equivalent PLMNs, if available,
             * that AS shall use for cell selection and cell reselection.
             */
            break;

        default:
            as_msg.msgID = 0;
            break;
    }

    /* Send the message to the Access Stratum or S1AP in case of MME */
    if (as_msg.msgID > 0) {
#if defined(EPC_BUILD) && defined(NAS_MME)
        LOG_TRACE(DEBUG, "EMMAS-SAP - "
                  "Sending msg with id 0x%x, primitive %s (%d) to S1AP layer for transmission",
                  as_msg.msgID,
                  _emm_as_primitive_str[msg->primitive - _EMMAS_START - 1],
                  msg->primitive);

        switch (as_msg.msgID) {
            case AS_DL_INFO_TRANSFER_REQ: {
                nas_itti_dl_data_req(as_msg.msg.dl_info_transfer_req.UEid,
                                     as_msg.msg.dl_info_transfer_req.nasMsg.data,
                                     as_msg.msg.dl_info_transfer_req.nasMsg.length);
                LOG_FUNC_RETURN (RETURNok);
            } break;

            case AS_NAS_ESTABLISH_CNF: {
                if (as_msg.msg.nas_establish_rsp.errCode != AS_SUCCESS) {
                    nas_itti_dl_data_req(as_msg.msg.nas_establish_rsp.UEid,
                                         as_msg.msg.nas_establish_rsp.nasMsg.data,
                                         as_msg.msg.nas_establish_rsp.nasMsg.length);
                    LOG_FUNC_RETURN (RETURNok);
                } else {
                    /* Handle success case */
                    nas_itti_establish_cnf(as_msg.msg.nas_establish_rsp.UEid,
                                           as_msg.msg.nas_establish_rsp.errCode,
                                           as_msg.msg.nas_establish_rsp.nasMsg.data,
                                           as_msg.msg.nas_establish_rsp.nasMsg.length);
                    LOG_FUNC_RETURN (RETURNok);
                }
            } break;

            default:
                break;
        }
#else
# if defined(UE_BUILD) && defined(NAS_UE)
        LOG_TRACE(DEBUG, "EMMAS-SAP - "
                  "Sending msg with id 0x%x, primitive %s (%d) to RRC layer for transmission",
                  as_msg.msgID,
                  _emm_as_primitive_str[msg->primitive - _EMMAS_START - 1],
                  msg->primitive);

        switch (as_msg.msgID) {
            case AS_CELL_INFO_REQ: {
                nas_itti_cell_info_req(as_msg.msg.cell_info_req.plmnID,
                                       as_msg.msg.cell_info_req.rat);
                LOG_FUNC_RETURN (RETURNok);
            } break;

            case AS_NAS_ESTABLISH_REQ: {
                nas_itti_nas_establish_req(as_msg.msg.nas_establish_req.cause,
                                           as_msg.msg.nas_establish_req.type,
                                           as_msg.msg.nas_establish_req.s_tmsi,
                                           as_msg.msg.nas_establish_req.plmnID,
                                           as_msg.msg.nas_establish_req.initialNasMsg.data,
                                           as_msg.msg.nas_establish_req.initialNasMsg.length);
                LOG_FUNC_RETURN (RETURNok);
            } break;

            case AS_UL_INFO_TRANSFER_REQ: {
                nas_itti_ul_data_req(as_msg.msg.ul_info_transfer_req.UEid,
                                     as_msg.msg.ul_info_transfer_req.nasMsg.data,
                                     as_msg.msg.ul_info_transfer_req.nasMsg.length);
                LOG_FUNC_RETURN (RETURNok);
            } break;

            case AS_RAB_ESTABLISH_RSP: {
                nas_itti_rab_establish_rsp(as_msg.msg.rab_establish_rsp.s_tmsi,
                                           as_msg.msg.rab_establish_rsp.rabID,
                                           as_msg.msg.rab_establish_rsp.errCode);
                LOG_FUNC_RETURN (RETURNok);
            } break;

            default:
              break;
        }
# else
        int bytes = as_message_send(&as_msg);
        if (bytes > 0) {
            LOG_FUNC_RETURN (RETURNok);
        }
# endif
#endif
    }
    LOG_FUNC_RETURN (RETURNerror);
}

/****************************************************************************
 **                                                                        **
 ** Name:    _emm_as_data_req()                                        **
 **                                                                        **
 ** Description: Processes the EMMAS-SAP data transfer request             **
 **      primitive                                                 **
 **                                                                        **
 ** EMMAS-SAP - EMM->AS: DATA_REQ - Data transfer procedure                **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     as_msg:    The message to send to the AS              **
 **      Return:    The identifier of the AS message           **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_as_data_req(const emm_as_data_t *msg,
                            ul_info_transfer_req_t *as_msg)
{
    LOG_FUNC_IN;

    int size = 0;
    int is_encoded = FALSE;

    LOG_TRACE(INFO, "EMMAS-SAP - Send AS data transfer request");

    nas_message_t nas_msg;
    memset(&nas_msg, 0 , sizeof(nas_message_t));

    /* Setup the AS message */
    if (msg->guti) {
        as_msg->s_tmsi.MMEcode = msg->guti->gummei.MMEcode;
        as_msg->s_tmsi.m_tmsi = msg->guti->m_tmsi;
    } else {
        as_msg->UEid = msg->ueid;
    }

    /* Setup the NAS security header */
    EMM_msg *emm_msg = _emm_as_set_header(&nas_msg, &msg->sctx);

    /* Setup the NAS information message */
    if (emm_msg != NULL) switch (msg->NASinfo) {
#ifdef NAS_UE
            case EMM_AS_NAS_DATA_ATTACH:
                size = emm_send_attach_complete(msg, &emm_msg->attach_complete);
                break;

            case EMM_AS_NAS_DATA_DETACH:
                size = emm_send_detach_request(msg, &emm_msg->detach_request);
                break;
#endif

#ifdef NAS_MME
            case EMM_AS_NAS_DATA_DETACH:
                size = emm_send_detach_accept(msg, &emm_msg->detach_accept);
                break;
#endif

            default:
                /* Send other NAS messages as already encoded ESM messages */
                size = msg->NASmsg.length;
                is_encoded = TRUE;
                break;
        }

    if (size > 0) {
        int bytes;
        if (!is_encoded) {
            /* Encode the NAS information message */
            bytes = _emm_as_encode(&as_msg->nasMsg, &nas_msg, size);
        } else {
            /* Encrypt the NAS information message */
            bytes = _emm_as_encrypt(&as_msg->nasMsg, &nas_msg.header,
                                    (char *)(msg->NASmsg.value), size);
        }
        if (bytes > 0) {
#ifdef NAS_UE
            LOG_FUNC_RETURN (AS_UL_INFO_TRANSFER_REQ);
#endif
#ifdef NAS_MME
            LOG_FUNC_RETURN (AS_DL_INFO_TRANSFER_REQ);
#endif
        }
    }
    LOG_FUNC_RETURN (0);
}

/****************************************************************************
 **                                                                        **
 ** Name:    _emm_as_status_ind()                                      **
 **                                                                        **
 ** Description: Processes the EMMAS-SAP status indication primitive       **
 **                                                                        **
 ** EMMAS-SAP - EMM->AS: STATUS_IND - EMM status report procedure          **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     as_msg:    The message to send to the AS              **
 **      Return:    The identifier of the AS message           **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_as_status_ind(const emm_as_status_t *msg,
                              ul_info_transfer_req_t *as_msg)
{
    LOG_FUNC_IN;

    int size = 0;

    LOG_TRACE(INFO, "EMMAS-SAP - Send AS status indication (cause=%d)",
              msg->emm_cause);

    nas_message_t nas_msg;
    memset(&nas_msg, 0 , sizeof(nas_message_t));

    /* Setup the AS message */
    if (msg->guti) {
        as_msg->s_tmsi.MMEcode = msg->guti->gummei.MMEcode;
        as_msg->s_tmsi.m_tmsi = msg->guti->m_tmsi;
    } else {
        as_msg->UEid = msg->ueid;
    }

    /* Setup the NAS security header */
    EMM_msg *emm_msg = _emm_as_set_header(&nas_msg, &msg->sctx);

    /* Setup the NAS information message */
    if (emm_msg != NULL) {
        size = emm_send_status(msg, &emm_msg->emm_status);
    }

    if (size > 0) {
        /* Encode the NAS information message */
        int bytes = _emm_as_encode(&as_msg->nasMsg, &nas_msg, size);
        if (bytes > 0) {
#ifdef NAS_UE
            LOG_FUNC_RETURN (AS_UL_INFO_TRANSFER_REQ);
#endif
#ifdef NAS_MME
            LOG_FUNC_RETURN (AS_DL_INFO_TRANSFER_REQ);
#endif
        }
    }
    LOG_FUNC_RETURN (0);
}

/****************************************************************************
 **                                                                        **
 ** Name:    _emm_as_release_req()                                     **
 **                                                                        **
 ** Description: Processes the EMMAS-SAP connection release request        **
 **      primitive                                                 **
 **                                                                        **
 ** EMMAS-SAP - EMM->AS: RELEASE_REQ - NAS signalling release procedure    **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     as_msg:    The message to send to the AS              **
 **      Return:    The identifier of the AS message           **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_as_release_req(const emm_as_release_t *msg,
                               nas_release_req_t *as_msg)
{
    LOG_FUNC_IN;

    LOG_TRACE(INFO, "EMMAS-SAP - Send AS release request");

    /* Setup the AS message */
    if (msg->guti) {
        as_msg->s_tmsi.MMEcode = msg->guti->gummei.MMEcode;
        as_msg->s_tmsi.m_tmsi = msg->guti->m_tmsi;
    } else {
        as_msg->UEid = msg->ueid;
    }

    if (msg->cause == EMM_AS_CAUSE_AUTHENTICATION) {
        as_msg->cause = AS_AUTHENTICATION_FAILURE;
    } else if (msg->cause == EMM_AS_CAUSE_DETACH) {
        as_msg->cause = AS_DETACH;
    }

    LOG_FUNC_RETURN (AS_NAS_RELEASE_REQ);
}

#ifdef NAS_UE
/****************************************************************************
 **                                                                        **
 ** Name:    _emm_as_security_res()                                    **
 **                                                                        **
 ** Description: Processes the EMMAS-SAP security response primitive       **
 **                                                                        **
 ** EMMAS-SAP - EMM->AS: SECURITY_RES - Security mode control procedure    **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     as_msg:    The message to send to the AS              **
 **      Return:    The identifier of the AS message           **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_as_security_res(const emm_as_security_t *msg,
                                ul_info_transfer_req_t *as_msg)
{
    LOG_FUNC_IN;

    int size = 0;

    LOG_TRACE(INFO, "EMMAS-SAP - Send AS security response");

    nas_message_t nas_msg;
    memset(&nas_msg, 0 , sizeof(nas_message_t));

    /* Setup the AS message */
    if (msg->guti) {
        as_msg->s_tmsi.MMEcode = msg->guti->gummei.MMEcode;
        as_msg->s_tmsi.m_tmsi = msg->guti->m_tmsi;
    }

    /* Setup the NAS security header */
    EMM_msg *emm_msg = _emm_as_set_header(&nas_msg, &msg->sctx);

    /* Setup the NAS security message */
    if (emm_msg != NULL) switch (msg->msgType) {
            case EMM_AS_MSG_TYPE_IDENT:
                size = emm_send_identity_response(msg, &emm_msg->identity_response);
                break;

            case EMM_AS_MSG_TYPE_AUTH:
                if (msg->emm_cause != EMM_CAUSE_SUCCESS) {
                    size = emm_send_authentication_failure(msg,
                                                           &emm_msg->authentication_failure);
                } else {
                    size = emm_send_authentication_response(msg,
                                                            &emm_msg->authentication_response);
                }
                break;

            case EMM_AS_MSG_TYPE_SMC:
                if (msg->emm_cause != EMM_CAUSE_SUCCESS) {
                    size = emm_send_security_mode_reject(msg,
                                                         &emm_msg->security_mode_reject);
                } else {
                    size = emm_send_security_mode_complete(msg,
                                                           &emm_msg->security_mode_complete);
                }
                break;

            default:
                LOG_TRACE(WARNING, "EMMAS-SAP - Type of NAS security "
                          "message 0x%.2x is not valid", msg->msgType);
        }

    if (size > 0) {
        /* Encode the NAS security message */
        int bytes = _emm_as_encode(&as_msg->nasMsg, &nas_msg, size);
        if (bytes > 0) {
            LOG_FUNC_RETURN (AS_UL_INFO_TRANSFER_REQ);
        }
    }
    LOG_FUNC_RETURN (0);
}

/****************************************************************************
 **                                                                        **
 ** Name:    _emm_as_establish_req()                                   **
 **                                                                        **
 ** Description: Processes the EMMAS-SAP connection establish request      **
 **      primitive                                                 **
 **                                                                        **
 ** EMMAS-SAP - EMM->AS: ESTABLISH_REQ - NAS signalling connection         **
 **     The NAS requests the AS to establish signalling connection **
 **     to tranfer initial NAS message to the network.             **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     as_msg:    The message to send to the AS              **
 **      Return:    The identifier of the AS message           **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_as_establish_req(const emm_as_establish_t *msg,
                                 nas_establish_req_t *as_msg)
{
    LOG_FUNC_IN;

    int size = 0;

    LOG_TRACE(INFO, "EMMAS-SAP - Send AS connection establish request");

    nas_message_t nas_msg;
    memset(&nas_msg, 0 , sizeof(nas_message_t));

    /* Setup the AS message */
    as_msg->cause = msg->RRCcause;
    as_msg->type = msg->RRCtype;
    as_msg->plmnID = *msg->plmnID;
    /* Derive the S-TMSI from the GUTI, if valid */
    if (msg->UEid.guti) {
        as_msg->s_tmsi.MMEcode = msg->UEid.guti->gummei.MMEcode;
        as_msg->s_tmsi.m_tmsi = msg->UEid.guti->m_tmsi;
    }

    /* Setup the NAS security header */
    EMM_msg *emm_msg = _emm_as_set_header(&nas_msg, &msg->sctx);

    /* Setup the initial NAS information message */
    if (emm_msg != NULL) switch (msg->NASinfo) {
            case EMM_AS_NAS_INFO_ATTACH:
                size = emm_send_attach_request(msg, &emm_msg->attach_request);
                break;

            case EMM_AS_NAS_INFO_DETACH:
                size = emm_send_initial_detach_request(msg,
                                                       &emm_msg->detach_request);
                break;

            case EMM_AS_NAS_INFO_TAU:
                size = emm_send_initial_tau_request(msg,
                                                    &emm_msg->tracking_area_update_request);
                break;

            case EMM_AS_NAS_INFO_SR:
                size = emm_send_initial_sr_request(msg, &emm_msg->service_request);
                break;

            case EMM_AS_NAS_INFO_EXTSR:
                size = emm_send_initial_extsr_request(msg,
                                                      &emm_msg->extended_service_request);
                break;

            default:
                LOG_TRACE(WARNING, "EMMAS-SAP - Type of initial NAS "
                          "message 0x%.2x is not valid", msg->NASinfo);
                break;
        }

    if (size > 0) {
        /* Encode the initial NAS information message */
        int bytes = _emm_as_encode(&as_msg->initialNasMsg, &nas_msg, size);
        if (bytes > 0) {
            LOG_FUNC_RETURN (AS_NAS_ESTABLISH_REQ);
        }
    }
    LOG_FUNC_RETURN (0);
}
#endif // NAS_UE

#ifdef NAS_MME
/****************************************************************************
 **                                                                        **
 ** Name:    _emm_as_security_req()                                    **
 **                                                                        **
 ** Description: Processes the EMMAS-SAP security request primitive        **
 **                                                                        **
 ** EMMAS-SAP - EMM->AS: SECURITY_REQ - Security mode control procedure    **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     as_msg:    The message to send to the AS              **
 **      Return:    The identifier of the AS message           **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_as_security_req(const emm_as_security_t *msg,
                                dl_info_transfer_req_t *as_msg)
{
    LOG_FUNC_IN;

    int size = 0;

    LOG_TRACE(INFO, "EMMAS-SAP - Send AS security request");

    nas_message_t nas_msg;
    memset(&nas_msg, 0 , sizeof(nas_message_t));

    /* Setup the AS message */
    if (msg->guti) {
        as_msg->s_tmsi.MMEcode = msg->guti->gummei.MMEcode;
        as_msg->s_tmsi.m_tmsi = msg->guti->m_tmsi;
    } else {
        as_msg->UEid = msg->ueid;
    }

    /* Setup the NAS security header */
    EMM_msg *emm_msg = _emm_as_set_header(&nas_msg, &msg->sctx);

    /* Setup the NAS security message */
    if (emm_msg != NULL) switch (msg->msgType) {
            case EMM_AS_MSG_TYPE_IDENT:
                size = emm_send_identity_request(msg, &emm_msg->identity_request);
                break;

            case EMM_AS_MSG_TYPE_AUTH:
                size = emm_send_authentication_request(msg,
                                                       &emm_msg->authentication_request);
                break;

            case EMM_AS_MSG_TYPE_SMC:
                size = emm_send_security_mode_command(msg,
                                                      &emm_msg->security_mode_command);
                break;

            default:
                LOG_TRACE(WARNING, "EMMAS-SAP - Type of NAS security "
                          "message 0x%.2x is not valid", msg->msgType);
        }

    if (size > 0) {
        /* Encode the NAS security message */
        int bytes = _emm_as_encode(&as_msg->nasMsg, &nas_msg, size);
        if (bytes > 0) {
            LOG_FUNC_RETURN (AS_DL_INFO_TRANSFER_REQ);
        }
    }
    LOG_FUNC_RETURN (0);
}

/****************************************************************************
 **                                                                        **
 ** Name:    _emm_as_security_rej()                                    **
 **                                                                        **
 ** Description: Processes the EMMAS-SAP security reject primitive         **
 **                                                                        **
 ** EMMAS-SAP - EMM->AS: SECURITY_REJ - Security mode control procedure    **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     as_msg:    The message to send to the AS              **
 **      Return:    The identifier of the AS message           **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_as_security_rej(const emm_as_security_t *msg,
                                dl_info_transfer_req_t *as_msg)
{
    LOG_FUNC_IN;

    int size = 0;

    LOG_TRACE(INFO, "EMMAS-SAP - Send AS security reject");

    nas_message_t nas_msg;
    memset(&nas_msg, 0 , sizeof(nas_message_t));

    /* Setup the AS message */
    if (msg->guti) {
        as_msg->s_tmsi.MMEcode = msg->guti->gummei.MMEcode;
        as_msg->s_tmsi.m_tmsi = msg->guti->m_tmsi;
    } else {
        as_msg->UEid = msg->ueid;
    }

    /* Setup the NAS security header */
    EMM_msg *emm_msg = _emm_as_set_header(&nas_msg, &msg->sctx);

    /* Setup the NAS security message */
    if (emm_msg != NULL) switch (msg->msgType) {
            case EMM_AS_MSG_TYPE_AUTH:
                size = emm_send_authentication_reject(
                           &emm_msg->authentication_reject);
                break;

            default:
                LOG_TRACE(WARNING, "EMMAS-SAP - Type of NAS security "
                          "message 0x%.2x is not valid", msg->msgType);
        }

    if (size > 0) {
        /* Encode the NAS security message */
        int bytes = _emm_as_encode(&as_msg->nasMsg, &nas_msg, size);
        if (bytes > 0) {
            LOG_FUNC_RETURN (AS_DL_INFO_TRANSFER_REQ);
        }
    }
    LOG_FUNC_RETURN (0);
}

/****************************************************************************
 **                                                                        **
 ** Name:    _emm_as_establish_cnf()                                   **
 **                                                                        **
 ** Description: Processes the EMMAS-SAP connection establish confirm      **
 **      primitive                                                 **
 **                                                                        **
 ** EMMAS-SAP - EMM->AS: ESTABLISH_CNF - NAS signalling connection         **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     as_msg:    The message to send to the AS              **
 **      Return:    The identifier of the AS message           **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_as_establish_cnf(const emm_as_establish_t *msg,
                                 nas_establish_rsp_t *as_msg)
{
    EMM_msg *emm_msg;
    int size = 0;

    LOG_FUNC_IN;

    LOG_TRACE(INFO, "EMMAS-SAP - Send AS connection establish confirmation");

    nas_message_t nas_msg;
    memset(&nas_msg, 0 , sizeof(nas_message_t));

    /* Setup the AS message */
    as_msg->UEid = msg->ueid;
    if (msg->UEid.guti == NULL) {
        LOG_TRACE(WARNING, "EMMAS-SAP - GUTI is NULL...");
        LOG_FUNC_RETURN (0);
    }
    as_msg->s_tmsi.MMEcode = msg->UEid.guti->gummei.MMEcode;
    as_msg->s_tmsi.m_tmsi = msg->UEid.guti->m_tmsi;

    /* Setup the NAS security header */
    emm_msg = _emm_as_set_header(&nas_msg, &msg->sctx);

    /* Setup the initial NAS information message */
    if (emm_msg != NULL) switch (msg->NASinfo) {
            case EMM_AS_NAS_INFO_ATTACH:
                size = emm_send_attach_accept(msg, &emm_msg->attach_accept);
                break;

            default:
                LOG_TRACE(WARNING, "EMMAS-SAP - Type of initial NAS "
                          "message 0x%.2x is not valid", msg->NASinfo);
                break;
        }

    if (size > 0) {
        /* Encode the initial NAS information message */
        int bytes = _emm_as_encode(&as_msg->nasMsg, &nas_msg, size);
        if (bytes > 0) {
            as_msg->errCode = AS_SUCCESS;
            LOG_FUNC_RETURN (AS_NAS_ESTABLISH_CNF);
        }
    }

    LOG_TRACE(WARNING, "EMMAS-SAP - Size <= 0");
    LOG_FUNC_RETURN (0);
}

/****************************************************************************
 **                                                                        **
 ** Name:    _emm_as_establish_rej()                                   **
 **                                                                        **
 ** Description: Processes the EMMAS-SAP connection establish reject       **
 **      primitive                                                 **
 **                                                                        **
 ** EMMAS-SAP - EMM->AS: ESTABLISH_REJ - NAS signalling connection         **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     as_msg:    The message to send to the AS              **
 **      Return:    The identifier of the AS message           **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_as_establish_rej(const emm_as_establish_t *msg,
                                 nas_establish_rsp_t *as_msg)
{
    EMM_msg *emm_msg;
    int size = 0;
    nas_message_t nas_msg;

    LOG_FUNC_IN;

    LOG_TRACE(INFO, "EMMAS-SAP - Send AS connection establish reject");

    memset(&nas_msg, 0 , sizeof(nas_message_t));

    /* Setup the AS message */
    if (msg->UEid.guti) {
        as_msg->s_tmsi.MMEcode = msg->UEid.guti->gummei.MMEcode;
        as_msg->s_tmsi.m_tmsi = msg->UEid.guti->m_tmsi;
    } else {
        as_msg->UEid = msg->ueid;
    }

    /* Setup the NAS security header */
    emm_msg = _emm_as_set_header(&nas_msg, &msg->sctx);

    /* Setup the NAS information message */
    if (emm_msg != NULL) switch (msg->NASinfo) {
            case EMM_AS_NAS_INFO_ATTACH:
                size = emm_send_attach_reject(msg, &emm_msg->attach_reject);
                break;

            default:
                LOG_TRACE(WARNING, "EMMAS-SAP - Type of initial NAS "
                          "message 0x%.2x is not valid", msg->NASinfo);
                break;
        }

    if (size > 0) {
        /* Encode the initial NAS information message */
        int bytes = _emm_as_encode(&as_msg->nasMsg, &nas_msg, size);
        if (bytes > 0) {
            as_msg->errCode = AS_TERMINATED_NAS;
            LOG_FUNC_RETURN (AS_NAS_ESTABLISH_RSP);
        }
    }
    LOG_FUNC_RETURN (0);
}

/****************************************************************************
 **                                                                        **
 ** Name:    _emm_as_page_ind()                                        **
 **                                                                        **
 ** Description: Processes the EMMAS-SAP paging data indication primitive  **
 **                                                                        **
 ** EMMAS-SAP - EMM->AS: PAGE_IND - Paging data procedure                  **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     as_msg:    The message to send to the AS              **
 **      Return:    The identifier of the AS message           **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_as_page_ind(const emm_as_page_t *msg, paging_req_t *as_msg)
{
    LOG_FUNC_IN;

    int bytes = 0;

    LOG_TRACE(INFO, "EMMAS-SAP - Send AS data paging indication");

    /* TODO */

    if (bytes > 0) {
        LOG_FUNC_RETURN (AS_PAGING_IND);
    }
    LOG_FUNC_RETURN (0);
}

#endif // NAS_MME

/****************************************************************************
 **                                                                        **
 ** Name:    _emm_as_cell_info_req()                                   **
 **                                                                        **
 ** Description: Processes the EMMAS-SAP cell information request          **
 **      primitive                                                 **
 **                                                                        **
 ** EMMAS-SAP - EMM->AS: CELL_INFO_REQ - PLMN and cell selection procedure **
 **     The NAS requests the AS to select a cell belonging to the  **
 **     selected PLMN with associated Radio Access Technologies.   **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     as_msg:    The message to send to the AS              **
 **      Return:    The identifier of the AS message           **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_as_cell_info_req(const emm_as_cell_info_t *msg,
                                 cell_info_req_t *as_msg)
{
    LOG_FUNC_IN;

    LOG_TRACE(INFO, "EMMAS-SAP - Send AS cell information request");

    as_msg->plmnID = msg->plmnIDs.plmn[0];
    as_msg->rat = msg->rat;

    LOG_FUNC_RETURN (AS_CELL_INFO_REQ);
}

