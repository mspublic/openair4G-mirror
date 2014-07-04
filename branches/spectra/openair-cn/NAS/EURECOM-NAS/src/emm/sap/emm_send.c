/*****************************************************************************
            Eurecom OpenAirInterface 3
            Copyright(c) 2012 Eurecom

Source      emm_send.c

Version     0.1

Date        2013/01/30

Product     NAS stack

Subsystem   EPS Mobility Management

Author      Frederic Maurel

Description Defines functions executed at the EMMAS Service Access
        Point to send EPS Mobility Management messages to the
        Access Stratum sublayer.

*****************************************************************************/

#include "emm_send.h"
#include "commonDef.h"
#include "nas_log.h"

#include "emm_msgDef.h"
#include "emm_proc.h"

#include <string.h> // strlen

/****************************************************************************/
/****************  E X T E R N A L    D E F I N I T I O N S  ****************/
/****************************************************************************/

/****************************************************************************/
/*******************  L O C A L    D E F I N I T I O N S  *******************/
/****************************************************************************/

/****************************************************************************/
/******************  E X P O R T E D    F U N C T I O N S  ******************/
/****************************************************************************/

/*
 * --------------------------------------------------------------------------
 * Functions executed by both the UE and the MME to send EMM messages
 * --------------------------------------------------------------------------
 */
/****************************************************************************
 **                                                                        **
 ** Name:    emm_send_status()                                         **
 **                                                                        **
 ** Description: Builds EMM status message                                 **
 **                                                                        **
 **      The EMM status message is sent by the UE or the network   **
 **      at any time to report certain error conditions.           **
 **                                                                        **
 ** Inputs:  emm_cause: EMM cause code                             **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     emm_msg:   The EMM message to be sent                 **
 **      Return:    The size of the EMM message                **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_send_status(const emm_as_status_t *msg, emm_status_msg *emm_msg)
{
    LOG_FUNC_IN;

    int size = EMM_HEADER_MAXIMUM_LENGTH;

    LOG_TRACE(WARNING, "EMMAS-SAP - Send EMM Status message (cause=%d)",
              msg->emm_cause);

    /* Mandatory - Message type */
    emm_msg->messagetype = EMM_STATUS;

    /* Mandatory - EMM cause */
    size += EMM_CAUSE_MAXIMUM_LENGTH;
    emm_msg->emmcause = msg->emm_cause;

    LOG_FUNC_RETURN (size);
}

/****************************************************************************
 **                                                                        **
 ** Name:    emm_send_detach_accept()                                  **
 **                                                                        **
 ** Description: Builds Detach Accept message                              **
 **                                                                        **
 **      The Detach Accept message is sent by the UE or the net-   **
 **      work to indicate that the detach procedure has been com-  **
 **      pleted.                                                   **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     emm_msg:   The EMM message to be sent                 **
 **      Return:    The size of the EMM message                **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_send_detach_accept(const emm_as_data_t *msg,
                           detach_accept_msg *emm_msg)
{
    LOG_FUNC_IN;

    int size = EMM_HEADER_MAXIMUM_LENGTH;

    LOG_TRACE(INFO, "EMMAS-SAP - Send Detach Accept message");

    /* Mandatory - Message type */
    emm_msg->messagetype = DETACH_ACCEPT;

    LOG_FUNC_RETURN (size);
}

/*
 * --------------------------------------------------------------------------
 * Functions executed by the UE to send EMM messages to the network
 * --------------------------------------------------------------------------
 */
#ifdef NAS_UE
/****************************************************************************
 **                                                                        **
 ** Name:    emm_send_attach_request()                                 **
 **                                                                        **
 ** Description: Builds Attach Request message                             **
 **                                                                        **
 **      The Attach Request message is sent by the UE to the net-  **
 **      work in order to perform an attach procedure.             **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     emm_msg:   The EMM message to be sent                 **
 **      Return:    The size of the EMM message                **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_send_attach_request(const emm_as_establish_t *msg,
                            attach_request_msg *emm_msg)
{
    LOG_FUNC_IN;

    int size = EMM_HEADER_MAXIMUM_LENGTH;

    LOG_TRACE(INFO, "EMMAS-SAP - Send Attach Request message");

    /* Mandatory - Message type */
    emm_msg->messagetype = ATTACH_REQUEST;

    /* Mandatory - EPS attach type */
    size += EPS_ATTACH_TYPE_MAXIMUM_LENGTH;
    if (msg->type == EMM_ATTACH_TYPE_EPS) {
        emm_msg->epsattachtype = EPS_ATTACH_TYPE_EPS;
    } else if (msg->type == EMM_ATTACH_TYPE_IMSI) {
        emm_msg->epsattachtype = EPS_ATTACH_TYPE_IMSI;
    } else if (msg->type == EMM_ATTACH_TYPE_EMERGENCY) {
        emm_msg->epsattachtype = EPS_ATTACH_TYPE_EMERGENCY;
    } else if (msg->type == EMM_ATTACH_TYPE_RESERVED) {
        emm_msg->epsattachtype = EPS_ATTACH_TYPE_RESERVED;
    } else {
        /* All other values shall be interpreted as "EPS attach" */
        emm_msg->epsattachtype = EPS_ATTACH_TYPE_EPS;
    }

    /* Mandatory - NAS key set identifier */
    size += NAS_KEY_SET_IDENTIFIER_MAXIMUM_LENGTH;
    if (msg->ksi != EMM_AS_NO_KEY_AVAILABLE) {
        emm_msg->naskeysetidentifier.naskeysetidentifier = msg->ksi;
    } else {
        emm_msg->naskeysetidentifier.naskeysetidentifier =
            NAS_KEY_SET_IDENTIFIER_NOT_AVAILABLE;
    }

    /* Mandatory - EPS mobile identity */
    size += EPS_MOBILE_IDENTITY_MAXIMUM_LENGTH;
    if (msg->UEid.guti) {
        LOG_TRACE(INFO, "EMMAS-SAP - Send Attach Request message with GUTI");
        /* Set GUTI mobile identity */
        GutiEpsMobileIdentity_t *guti = &emm_msg->oldgutiorimsi.guti;
        guti->typeofidentity = EPS_MOBILE_IDENTITY_GUTI;
        guti->oddeven = EPS_MOBILE_IDENTITY_EVEN;
        guti->mmegroupid = msg->UEid.guti->gummei.MMEgid;
        guti->mmecode = msg->UEid.guti->gummei.MMEcode;
        guti->mtmsi = msg->UEid.guti->m_tmsi;
        guti->mccdigit1 = msg->UEid.guti->gummei.plmn.MCCdigit1;
        guti->mccdigit2 = msg->UEid.guti->gummei.plmn.MCCdigit2;
        guti->mccdigit3 = msg->UEid.guti->gummei.plmn.MCCdigit3;
        guti->mncdigit1 = msg->UEid.guti->gummei.plmn.MNCdigit1;
        guti->mncdigit2 = msg->UEid.guti->gummei.plmn.MNCdigit2;
        guti->mncdigit3 = msg->UEid.guti->gummei.plmn.MNCdigit3;
    } else if (msg->UEid.imsi) {
        LOG_TRACE(INFO, "EMMAS-SAP - Send Attach Request message with IMSI");
        /* Set IMSI mobile identity */
        ImsiEpsMobileIdentity_t *imsi = &emm_msg->oldgutiorimsi.imsi;
        imsi->typeofidentity = EPS_MOBILE_IDENTITY_IMSI;
        imsi->oddeven = msg->UEid.imsi->u.num.parity;
        imsi->digit1 = msg->UEid.imsi->u.num.digit1;
        imsi->digit2 = msg->UEid.imsi->u.num.digit2;
        imsi->digit3 = msg->UEid.imsi->u.num.digit3;
        imsi->digit4 = msg->UEid.imsi->u.num.digit4;
        imsi->digit5 = msg->UEid.imsi->u.num.digit5;
        imsi->digit6 = msg->UEid.imsi->u.num.digit6;
        imsi->digit7 = msg->UEid.imsi->u.num.digit7;
        imsi->digit8 = msg->UEid.imsi->u.num.digit8;
        imsi->digit9 = msg->UEid.imsi->u.num.digit9;
        imsi->digit10 = msg->UEid.imsi->u.num.digit10;
        imsi->digit11 = msg->UEid.imsi->u.num.digit11;
        imsi->digit12 = msg->UEid.imsi->u.num.digit12;
        imsi->digit13 = msg->UEid.imsi->u.num.digit13;
        imsi->digit14 = msg->UEid.imsi->u.num.digit14;
        imsi->digit15 = msg->UEid.imsi->u.num.digit15;
    } else if (msg->UEid.imei) {
        LOG_TRACE(INFO, "EMMAS-SAP - Send Attach Request message with IMEI");
        /* Set IMEI mobile identity */
        ImeiEpsMobileIdentity_t *imei = &emm_msg->oldgutiorimsi.imei;
        imei->typeofidentity = EPS_MOBILE_IDENTITY_IMEI;
        imei->oddeven = msg->UEid.imei->u.num.parity;
        imei->digit1 = msg->UEid.imei->u.num.digit1;
        imei->digit2 = msg->UEid.imei->u.num.digit2;
        imei->digit3 = msg->UEid.imei->u.num.digit3;
        imei->digit4 = msg->UEid.imei->u.num.digit4;
        imei->digit5 = msg->UEid.imei->u.num.digit5;
        imei->digit6 = msg->UEid.imei->u.num.digit6;
        imei->digit7 = msg->UEid.imei->u.num.digit7;
        imei->digit8 = msg->UEid.imei->u.num.digit8;
        imei->digit9 = msg->UEid.imei->u.num.digit9;
        imei->digit10 = msg->UEid.imei->u.num.digit10;
        imei->digit11 = msg->UEid.imei->u.num.digit11;
        imei->digit12 = msg->UEid.imei->u.num.digit12;
        imei->digit13 = msg->UEid.imei->u.num.digit13;
        imei->digit14 = msg->UEid.imei->u.num.digit14;
        imei->digit15 = msg->UEid.imei->u.num.digit15;
    }

    /* Mandatory - UE network capability */
    size += UE_NETWORK_CAPABILITY_MAXIMUM_LENGTH;
    emm_msg->uenetworkcapability.eea = (0x80 >> msg->encryption);
    emm_msg->uenetworkcapability.eia = (0x80 >> msg->integrity);
    emm_msg->uenetworkcapability.uea = 0;
    emm_msg->uenetworkcapability.ucs2 = 0;
    emm_msg->uenetworkcapability.uia = 0;
    emm_msg->uenetworkcapability.csfb = 0;
    emm_msg->uenetworkcapability.lpp = 0;
    emm_msg->uenetworkcapability.lcs = 0;
    emm_msg->uenetworkcapability.srvcc = 0;
    emm_msg->uenetworkcapability.nf = 0;

    /* Mandatory - ESM message container */
    size += ESM_MESSAGE_CONTAINER_MINIMUM_LENGTH + msg->NASmsg.length;
    emm_msg->esmmessagecontainer.esmmessagecontainercontents = msg->NASmsg;

    /* Optional - Last visited registered TAI */
    if (msg->UEid.tai) {
        size += TRACKING_AREA_IDENTITY_MAXIMUM_LENGTH;
        emm_msg->presencemask |= ATTACH_REQUEST_LAST_VISITED_REGISTERED_TAI_PRESENT;
        emm_msg->lastvisitedregisteredtai.mccdigit2 =
            msg->UEid.tai->plmn.MCCdigit2;
        emm_msg->lastvisitedregisteredtai.mccdigit1 =
            msg->UEid.tai->plmn.MCCdigit1;
        emm_msg->lastvisitedregisteredtai.mncdigit3 =
            msg->UEid.tai->plmn.MNCdigit3;
        emm_msg->lastvisitedregisteredtai.mccdigit3 =
            msg->UEid.tai->plmn.MCCdigit3;
        emm_msg->lastvisitedregisteredtai.mncdigit2 =
            msg->UEid.tai->plmn.MNCdigit2;
        emm_msg->lastvisitedregisteredtai.mncdigit1 =
            msg->UEid.tai->plmn.MNCdigit1;
        emm_msg->lastvisitedregisteredtai.tac = msg->UEid.tai->tac;
    }

    /* Optional - Old GUTI type */
    if (msg->UEid.guti) {
        size += GUTI_TYPE_MAXIMUM_LENGTH;
        emm_msg->presencemask |= ATTACH_REQUEST_OLD_GUTI_TYPE_PRESENT;
        emm_msg->oldgutitype = GUTI_NATIVE;
    }

    LOG_FUNC_RETURN (size);
}

/****************************************************************************
 **                                                                        **
 ** Name:    emm_send_attach_complete()                                **
 **                                                                        **
 ** Description: Builds Attach Complete message                            **
 **                                                                        **
 **      The Attach Complete message is sent by the UE to the net- **
 **      work in response to an Attach Accept message.             **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     emm_msg:   The EMM message to be sent                 **
 **      Return:    The size of the EMM message                **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_send_attach_complete(const emm_as_data_t *msg,
                             attach_complete_msg *emm_msg)
{
    LOG_FUNC_IN;

    int size = EMM_HEADER_MAXIMUM_LENGTH;

    LOG_TRACE(INFO, "EMMAS-SAP - Send Attach Complete message");

    /* Mandatory - Message type */
    emm_msg->messagetype = ATTACH_COMPLETE;

    /* Mandatory - ESM message container */
    size += ESM_MESSAGE_CONTAINER_MINIMUM_LENGTH + msg->NASmsg.length;
    emm_msg->esmmessagecontainer.esmmessagecontainercontents = msg->NASmsg;

    LOG_FUNC_RETURN (size);
}

/****************************************************************************
 **                                                                        **
 ** Name:    emm_send_initial_detach_request()                         **
 **                                                                        **
 ** Description: Builds Detach Request message                             **
 **                                                                        **
 **      The Detach Request message is sent by the UE to request   **
 **      the release of an EMM context.                            **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     emm_msg:   The EMM message to be sent                 **
 **      Return:    The size of the EMM message                **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_send_initial_detach_request(const emm_as_establish_t *msg,
                                    detach_request_msg *emm_msg)
{
    LOG_FUNC_IN;

    int size = EMM_HEADER_MAXIMUM_LENGTH;

    LOG_TRACE(INFO, "EMMAS-SAP - Send Detach Request message");

    /* Mandatory - Message type */
    emm_msg->messagetype = DETACH_REQUEST;

    /* Mandatory - Detach type */
    size += DETACH_TYPE_MAXIMUM_LENGTH;
    if (msg->switch_off) {
        emm_msg->detachtype.switchoff = DETACH_TYPE_SWITCH_OFF;
    } else {
        emm_msg->detachtype.switchoff = DETACH_TYPE_NORMAL_DETACH;
    }
    if (msg->type == EMM_DETACH_TYPE_EPS) {
        emm_msg->detachtype.typeofdetach = DETACH_TYPE_EPS;
    } else if (msg->type == EMM_DETACH_TYPE_IMSI) {
        emm_msg->detachtype.typeofdetach = DETACH_TYPE_IMSI;
    } else if (msg->type == EMM_DETACH_TYPE_EPS_IMSI) {
        emm_msg->detachtype.typeofdetach = DETACH_TYPE_EPS_IMSI;
    } else if (msg->type == EMM_DETACH_TYPE_RESERVED) {
        emm_msg->detachtype.typeofdetach = DETACH_TYPE_RESERVED_1;
    } else {
        /* All other values are interpreted as "combined EPS/IMSI detach" */
        emm_msg->detachtype.typeofdetach = DETACH_TYPE_EPS_IMSI;
    }

    /* Mandatory - NAS key set identifier */
    size += NAS_KEY_SET_IDENTIFIER_MAXIMUM_LENGTH;
    if (msg->ksi != EMM_AS_NO_KEY_AVAILABLE) {
        emm_msg->naskeysetidentifier.naskeysetidentifier = msg->ksi;
    } else {
        emm_msg->naskeysetidentifier.naskeysetidentifier =
            NAS_KEY_SET_IDENTIFIER_NOT_AVAILABLE;
    }

    /* Mandatory - EPS mobile identity */
    size += EPS_MOBILE_IDENTITY_MAXIMUM_LENGTH;
    if (msg->UEid.guti) {
        /* Set GUTI mobile identity */
        GutiEpsMobileIdentity_t *guti = &emm_msg->gutiorimsi.guti;
        guti->typeofidentity = EPS_MOBILE_IDENTITY_GUTI;
        guti->oddeven = EPS_MOBILE_IDENTITY_EVEN;
        guti->mmegroupid = msg->UEid.guti->gummei.MMEgid;
        guti->mmecode = msg->UEid.guti->gummei.MMEcode;
        guti->mtmsi = msg->UEid.guti->m_tmsi;
        guti->mccdigit1 = msg->UEid.guti->gummei.plmn.MCCdigit1;
        guti->mccdigit2 = msg->UEid.guti->gummei.plmn.MCCdigit2;
        guti->mccdigit3 = msg->UEid.guti->gummei.plmn.MCCdigit3;
        guti->mncdigit1 = msg->UEid.guti->gummei.plmn.MNCdigit1;
        guti->mncdigit2 = msg->UEid.guti->gummei.plmn.MNCdigit2;
        guti->mncdigit3 = msg->UEid.guti->gummei.plmn.MNCdigit3;
    } else if (msg->UEid.imsi) {
        /* Set IMSI mobile identity */
        ImsiEpsMobileIdentity_t *imsi = &emm_msg->gutiorimsi.imsi;
        imsi->typeofidentity = EPS_MOBILE_IDENTITY_IMSI;
        imsi->oddeven = msg->UEid.imsi->u.num.parity;
        imsi->digit1 = msg->UEid.imsi->u.num.digit1;
        imsi->digit2 = msg->UEid.imsi->u.num.digit2;
        imsi->digit3 = msg->UEid.imsi->u.num.digit3;
        imsi->digit4 = msg->UEid.imsi->u.num.digit4;
        imsi->digit5 = msg->UEid.imsi->u.num.digit5;
        imsi->digit6 = msg->UEid.imsi->u.num.digit6;
        imsi->digit7 = msg->UEid.imsi->u.num.digit7;
        imsi->digit8 = msg->UEid.imsi->u.num.digit8;
        imsi->digit9 = msg->UEid.imsi->u.num.digit9;
        imsi->digit10 = msg->UEid.imsi->u.num.digit10;
        imsi->digit11 = msg->UEid.imsi->u.num.digit11;
        imsi->digit12 = msg->UEid.imsi->u.num.digit12;
        imsi->digit13 = msg->UEid.imsi->u.num.digit13;
        imsi->digit14 = msg->UEid.imsi->u.num.digit14;
        imsi->digit15 = msg->UEid.imsi->u.num.digit15;
    } else if (msg->UEid.imei) {
        /* Set IMEI mobile identity */
        ImeiEpsMobileIdentity_t *imei = &emm_msg->gutiorimsi.imei;
        imei->typeofidentity = EPS_MOBILE_IDENTITY_IMEI;
        imei->oddeven = msg->UEid.imei->u.num.parity;
        imei->digit1 = msg->UEid.imei->u.num.digit1;
        imei->digit2 = msg->UEid.imei->u.num.digit2;
        imei->digit3 = msg->UEid.imei->u.num.digit3;
        imei->digit4 = msg->UEid.imei->u.num.digit4;
        imei->digit5 = msg->UEid.imei->u.num.digit5;
        imei->digit6 = msg->UEid.imei->u.num.digit6;
        imei->digit7 = msg->UEid.imei->u.num.digit7;
        imei->digit8 = msg->UEid.imei->u.num.digit8;
        imei->digit9 = msg->UEid.imei->u.num.digit9;
        imei->digit10 = msg->UEid.imei->u.num.digit10;
        imei->digit11 = msg->UEid.imei->u.num.digit11;
        imei->digit12 = msg->UEid.imei->u.num.digit12;
        imei->digit13 = msg->UEid.imei->u.num.digit13;
        imei->digit14 = msg->UEid.imei->u.num.digit14;
        imei->digit15 = msg->UEid.imei->u.num.digit15;
    }

    LOG_FUNC_RETURN (size);
}

/****************************************************************************
 **                                                                        **
 ** Name:    emm_send_detach_request()                                 **
 **                                                                        **
 ** Description: Builds Detach Request message                             **
 **                                                                        **
 **      The Detach Request message is sent by the UE to request   **
 **      the release of an EMM context.                            **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     emm_msg:   The EMM message to be sent                 **
 **      Return:    The size of the EMM message                **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_send_detach_request(const emm_as_data_t *msg,
                            detach_request_msg *emm_msg)
{
    LOG_FUNC_IN;

    int size = EMM_HEADER_MAXIMUM_LENGTH;

    LOG_TRACE(INFO, "EMMAS-SAP - Send Detach Request message");

    /* Mandatory - Message type */
    emm_msg->messagetype = DETACH_REQUEST;

    /* Mandatory - Detach type */
    size += DETACH_TYPE_MAXIMUM_LENGTH;
    if (msg->switch_off) {
        emm_msg->detachtype.switchoff = DETACH_TYPE_SWITCH_OFF;
    } else {
        emm_msg->detachtype.switchoff = DETACH_TYPE_NORMAL_DETACH;
    }
    if (msg->type == EMM_DETACH_TYPE_EPS) {
        emm_msg->detachtype.typeofdetach = DETACH_TYPE_EPS;
    } else if (msg->type == EMM_DETACH_TYPE_IMSI) {
        emm_msg->detachtype.typeofdetach = DETACH_TYPE_IMSI;
    } else if (msg->type == EMM_DETACH_TYPE_EPS_IMSI) {
        emm_msg->detachtype.typeofdetach = DETACH_TYPE_EPS_IMSI;
    } else if (msg->type == EMM_DETACH_TYPE_RESERVED) {
        emm_msg->detachtype.typeofdetach = DETACH_TYPE_RESERVED_1;
    } else {
        /* All other values are interpreted as "combined EPS/IMSI detach" */
        emm_msg->detachtype.typeofdetach = DETACH_TYPE_EPS_IMSI;
    }

    /* Mandatory - NAS key set identifier */
    size += NAS_KEY_SET_IDENTIFIER_MAXIMUM_LENGTH;
    if (msg->sctx.ksi != EMM_AS_NO_KEY_AVAILABLE) {
        emm_msg->naskeysetidentifier.naskeysetidentifier = msg->sctx.ksi;
    } else {
        emm_msg->naskeysetidentifier.naskeysetidentifier =
            NAS_KEY_SET_IDENTIFIER_NOT_AVAILABLE;
    }

    /* Mandatory - EPS mobile identity */
    size += EPS_MOBILE_IDENTITY_MAXIMUM_LENGTH;
    if (msg->guti) {
        /* Set GUTI mobile identity */
        GutiEpsMobileIdentity_t *guti = &emm_msg->gutiorimsi.guti;
        guti->typeofidentity = EPS_MOBILE_IDENTITY_GUTI;
        guti->oddeven = EPS_MOBILE_IDENTITY_EVEN;
        guti->mmegroupid = msg->guti->gummei.MMEgid;
        guti->mmecode = msg->guti->gummei.MMEcode;
        guti->mtmsi = msg->guti->m_tmsi;
        guti->mccdigit1 = msg->guti->gummei.plmn.MCCdigit1;
        guti->mccdigit2 = msg->guti->gummei.plmn.MCCdigit2;
        guti->mccdigit3 = msg->guti->gummei.plmn.MCCdigit3;
        guti->mncdigit1 = msg->guti->gummei.plmn.MNCdigit1;
        guti->mncdigit2 = msg->guti->gummei.plmn.MNCdigit2;
        guti->mncdigit3 = msg->guti->gummei.plmn.MNCdigit3;
    }

    LOG_FUNC_RETURN (size);
}

/****************************************************************************
 **                                                                        **
 ** Name:    emm_send_initial_tau_request()                            **
 **                                                                        **
 ** Description: Builds Tracking Area Update Request message               **
 **                                                                        **
 **      The Tracking Area Update Request message is sent by the   **
 **      UE to the network to update the registration of the ac-   **
 **      tual tracking area of a UE in the network and to periodi- **
 **      cally notify the availability of the UE to the network.   **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     emm_msg:   The EMM message to be sent                 **
 **      Return:    The size of the EMM message                **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_send_initial_tau_request(const emm_as_establish_t *msg,
                                 tracking_area_update_request_msg *emm_msg)
{
    LOG_FUNC_IN;

    int size = EMM_HEADER_MAXIMUM_LENGTH;

    LOG_TRACE(INFO, "EMMAS-SAP - Send Tracking Area Update Request message");

    /* Mandatory - Message type */
    emm_msg->messagetype = TRACKING_AREA_UPDATE_REQUEST;

    LOG_FUNC_RETURN (size);
}

/****************************************************************************
 **                                                                        **
 ** Name:    emm_send_initial_sr_request()                             **
 **                                                                        **
 ** Description: Build the Service Request message                         **
 **                                                                        **
 **      The Service Request message is sent by the UE to the      **
 **      network to request the establishment of a NAS signalling  **
 **      connection and of the radio and S1 bearers.               **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     emm_msg:   The EMM message to be sent                 **
 **      Return:    The size of the EMM message                **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_send_initial_sr_request(const emm_as_establish_t *msg,
                                service_request_msg *emm_msg)
{
    LOG_FUNC_IN;

    int size = EMM_HEADER_MAXIMUM_LENGTH;

    LOG_TRACE(INFO, "EMMAS-SAP - Send Service Request message");

    LOG_FUNC_RETURN (size);
}

/****************************************************************************
 **                                                                        **
 ** Name:    emm_send_initial_extsr_request()                          **
 **                                                                        **
 ** Description: Build the Extended Service Request message                **
 **                                                                        **
 **      The Extended Service Request message is sent by the UE to **
 **      the network to request the establishment of a NAS signal- **
 **      ling connection and of the radio and S1 bearers for pa-   **
 **      cket services, if the UE needs to provide additional in-  **
 **      formation that cannot be provided via a SERVICE REQUEST   **
 **      message.
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     emm_msg:   The EMM message to be sent                 **
 **      Return:    The size of the EMM message                **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_send_initial_extsr_request(const emm_as_establish_t *msg,
                                   extended_service_request_msg *emm_msg)
{
    LOG_FUNC_IN;

    int size = EMM_HEADER_MAXIMUM_LENGTH;

    LOG_TRACE(INFO, "EMMAS-SAP - Send Extended Service Request message");

    /* Mandatory - Message type */
    emm_msg->messagetype = EXTENDED_SERVICE_REQUEST;

    LOG_FUNC_RETURN (size);
}

/****************************************************************************
 **                                                                        **
 ** Name:    emm_send_identity_response()                              **
 **                                                                        **
 ** Description: Builds Identity Response message                          **
 **                                                                        **
 **      The Identity Response message is sent by the UE to the    **
 **      network in response to an Identity Request message and    **
 **      provides the requested identity.                          **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     emm_msg:   The EMM message to be sent                 **
 **      Return:    The size of the EMM message                **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_send_identity_response(const emm_as_security_t *msg,
                               identity_response_msg *emm_msg)
{
    LOG_FUNC_IN;

    int size = EMM_HEADER_MAXIMUM_LENGTH;

    LOG_TRACE(INFO, "EMMAS-SAP - Send Identity Response message");

    /* Mandatory - Message type */
    emm_msg->messagetype = IDENTITY_RESPONSE;

    /* Mandatory - Mobile identity */
    size += MOBILE_IDENTITY_MAXIMUM_LENGTH;
    if (msg->identType == EMM_IDENT_TYPE_IMSI) {
        if (msg->imsi) {
            ImsiMobileIdentity_t *imsi = &emm_msg->mobileidentity.imsi;
            imsi->typeofidentity = MOBILE_IDENTITY_IMSI;
            imsi->oddeven = msg->imsi->u.num.parity;
            imsi->digit1 = msg->imsi->u.num.digit1;
            imsi->digit2 = msg->imsi->u.num.digit2;
            imsi->digit3 = msg->imsi->u.num.digit3;
            imsi->digit4 = msg->imsi->u.num.digit4;
            imsi->digit5 = msg->imsi->u.num.digit5;
            imsi->digit6 = msg->imsi->u.num.digit6;
            imsi->digit7 = msg->imsi->u.num.digit7;
            imsi->digit8 = msg->imsi->u.num.digit8;
            imsi->digit9 = msg->imsi->u.num.digit9;
            imsi->digit10 = msg->imsi->u.num.digit10;
            imsi->digit11 = msg->imsi->u.num.digit11;
            imsi->digit12 = msg->imsi->u.num.digit12;
            imsi->digit13 = msg->imsi->u.num.digit13;
            imsi->digit14 = msg->imsi->u.num.digit14;
            imsi->digit15 = msg->imsi->u.num.digit15;
        }
    } else if (msg->identType == EMM_IDENT_TYPE_TMSI) {
        if (msg->tmsi) {
            const char *p_tmsi = (char *)(&msg->tmsi);
            TmsiMobileIdentity_t *tmsi = &emm_msg->mobileidentity.tmsi;
            tmsi->typeofidentity = MOBILE_IDENTITY_TMSI;
            tmsi->oddeven = MOBILE_IDENTITY_EVEN;
            tmsi->digit1 = 0xf;
            tmsi->digit2 = (*p_tmsi & 0xf);
            tmsi->digit3 = ((*p_tmsi >> 4) & 0xf);
            p_tmsi++;
            tmsi->digit4 = (*p_tmsi & 0xf);
            tmsi->digit5 = ((*p_tmsi >> 4) & 0xf);
            p_tmsi++;
            tmsi->digit6 = (*p_tmsi & 0xf);
            tmsi->digit7 = ((*p_tmsi >> 4) & 0xf);
            p_tmsi++;
            tmsi->digit8 = (*p_tmsi & 0xf);
            tmsi->digit9 = ((*p_tmsi >> 4) & 0xf);
        }
    } else if (msg->identType == EMM_IDENT_TYPE_IMEI) {
        if (msg->imei) {
            ImeiMobileIdentity_t *imei = &emm_msg->mobileidentity.imei;
            imei->typeofidentity = MOBILE_IDENTITY_IMEI;
            imei->oddeven = msg->imei->u.num.parity;
            imei->digit1 = msg->imei->u.num.digit1;
            imei->digit2 = msg->imei->u.num.digit2;
            imei->digit3 = msg->imei->u.num.digit3;
            imei->digit4 = msg->imei->u.num.digit4;
            imei->digit5 = msg->imei->u.num.digit5;
            imei->digit6 = msg->imei->u.num.digit6;
            imei->digit7 = msg->imei->u.num.digit7;
            imei->digit8 = msg->imei->u.num.digit8;
            imei->digit9 = msg->imei->u.num.digit9;
            imei->digit10 = msg->imei->u.num.digit10;
            imei->digit11 = msg->imei->u.num.digit11;
            imei->digit12 = msg->imei->u.num.digit12;
            imei->digit13 = msg->imei->u.num.digit13;
            imei->digit14 = msg->imei->u.num.digit14;
            imei->digit15 = msg->imei->u.num.digit15;
        }
    } else if (msg->identType == EMM_IDENT_TYPE_NOT_AVAILABLE) {
        NoMobileIdentity_t *no_id = &emm_msg->mobileidentity.no_id;
        no_id->typeofidentity = MOBILE_IDENTITY_NOT_AVAILABLE;
    }

    LOG_FUNC_RETURN (size);
}

/****************************************************************************
 **                                                                        **
 ** Name:    emm_send_authentication_response()                        **
 **                                                                        **
 ** Description: Builds Authentication Response message                    **
 **                                                                        **
 **      The Authentication Response message is sent by the UE to  **
 **      the network to deliver a calculated authentication res-   **
 **      ponse to the network.                                     **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     emm_msg:   The EMM message to be sent                 **
 **      Return:    The size of the EMM message                **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_send_authentication_response(const emm_as_security_t *msg,
                                     authentication_response_msg *emm_msg)
{
    int size = EMM_HEADER_MAXIMUM_LENGTH;

    LOG_TRACE(INFO, "EMMAS-SAP - Send Authentication Response message");

    /* Mandatory - Message type */
    emm_msg->messagetype = AUTHENTICATION_RESPONSE;

    /* Mandatory - Authentication response parameter */
    size += AUTHENTICATION_RESPONSE_PARAMETER_MAXIMUM_LENGTH;
    emm_msg->authenticationresponseparameter.res = *msg->res;

    LOG_FUNC_RETURN (size);
}

/****************************************************************************
 **                                                                        **
 ** Name:    emm_send_authentication_failure()                         **
 **                                                                        **
 ** Description: Builds Authentication Failure message                     **
 **                                                                        **
 **      The Authentication Failure message is sent by the UE to   **
 **      the network to indicate that authentication of the net-   **
 **      work has failed                                           **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     emm_msg:   The EMM message to be sent                 **
 **      Return:    The size of the EMM message                **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_send_authentication_failure(const emm_as_security_t *msg,
                                    authentication_failure_msg *emm_msg)
{
    int size = EMM_HEADER_MAXIMUM_LENGTH;

    LOG_TRACE(INFO, "EMMAS-SAP - Send Authentication Failure message (cause=%d)",
              msg->emm_cause);

    /* Mandatory - Message type */
    emm_msg->messagetype = AUTHENTICATION_FAILURE;

    /* Mandatory - EMM cause */
    size += EMM_CAUSE_MAXIMUM_LENGTH;
    emm_msg->emmcause = msg->emm_cause;

    /* Optional - Authentication response parameter */
    if (msg->auts && msg->auts->length > 0) {
        size += AUTHENTICATION_RESPONSE_PARAMETER_MAXIMUM_LENGTH;
        emm_msg->presencemask |=
            AUTHENTICATION_FAILURE_AUTHENTICATION_FAILURE_PARAMETER_PRESENT;
        emm_msg->authenticationfailureparameter.auts = *msg->auts;
    }

    LOG_FUNC_RETURN (size);
}

/****************************************************************************
 **                                                                        **
 ** Name:    emm_send_security_mode_complete()                         **
 **                                                                        **
 ** Description: Builds Security Mode Complete message                     **
 **                                                                        **
 **      The Security Mode Complete message is sent by the UE to   **
 **      the network in response to a Security Mode Command mes-   **
 **      sage.                                                     **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     emm_msg:   The EMM message to be sent                 **
 **      Return:    The size of the EMM message                **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_send_security_mode_complete(const emm_as_security_t *msg,
                                    security_mode_complete_msg *emm_msg)
{
    LOG_FUNC_IN;

    int size = EMM_HEADER_MAXIMUM_LENGTH;

    LOG_TRACE(INFO, "EMMAS-SAP - Send Security Mode Complete message");

    /* Mandatory - Message type */
    emm_msg->messagetype = SECURITY_MODE_COMPLETE;

    LOG_FUNC_RETURN (size);
}

/****************************************************************************
 **                                                                        **
 ** Name:    emm_send_security_mode_reject()                           **
 **                                                                        **
 ** Description: Builds Security Mode Reject message                       **
 **                                                                        **
 **      The Security Mode Reject message is sent by the UE to the **
 **      network to indicate that the corresponding security mode  **
 **      command has been rejected.                                **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     emm_msg:   The EMM message to be sent                 **
 **      Return:    The size of the EMM message                **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_send_security_mode_reject(const emm_as_security_t *msg,
                                  security_mode_reject_msg *emm_msg)
{
    LOG_FUNC_IN;

    int size = EMM_HEADER_MAXIMUM_LENGTH;

    LOG_TRACE(INFO, "EMMAS-SAP - Send Security Mode Reject message (cause=%d)",
              msg->emm_cause);

    /* Mandatory - Message type */
    emm_msg->messagetype = SECURITY_MODE_REJECT;

    /* Mandatory - EMM cause */
    size += EMM_CAUSE_MAXIMUM_LENGTH;
    emm_msg->emmcause = msg->emm_cause;

    LOG_FUNC_RETURN (size);
}

#endif // NAS_UE

/*
 * --------------------------------------------------------------------------
 * Functions executed by the MME to send EMM messages to the UE
 * --------------------------------------------------------------------------
 */
#ifdef NAS_MME
/****************************************************************************
 **                                                                        **
 ** Name:    emm_send_attach_accept()                                  **
 **                                                                        **
 ** Description: Builds Attach Accept message                              **
 **                                                                        **
 **      The Attach Accept message is sent by the network to the   **
 **      UE to indicate that the corresponding attach request has  **
 **      been accepted.                                            **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     emm_msg:   The EMM message to be sent                 **
 **      Return:    The size of the EMM message                **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_send_attach_accept(const emm_as_establish_t *msg,
                           attach_accept_msg *emm_msg)
{
    LOG_FUNC_IN;

    int size = EMM_HEADER_MAXIMUM_LENGTH;

    LOG_TRACE(INFO, "EMMAS-SAP - Send Attach Accept message");
    LOG_TRACE(INFO, "EMMAS-SAP - size = EMM_HEADER_MAXIMUM_LENGTH(%d)",
            size);

    /* Mandatory - Message type */
    emm_msg->messagetype = ATTACH_ACCEPT;

    /* Mandatory - EPS attach result */
    size += EPS_ATTACH_RESULT_MAXIMUM_LENGTH;
    emm_msg->epsattachresult = EPS_ATTACH_RESULT_EPS;

    /* Mandatory - T3412 value */
    size += GPRS_TIMER_MAXIMUM_LENGTH;
    emm_msg->t3412value.unit = GPRS_TIMER_UNIT_0S;
    LOG_TRACE(INFO, "EMMAS-SAP - size += GPRS_TIMER_MAXIMUM_LENGTH(%d)  (%d)",
            GPRS_TIMER_MAXIMUM_LENGTH, size);

    /* Mandatory - Tracking area identity list */
    size += TRACKING_AREA_IDENTITY_LIST_MINIMUM_LENGTH;
    emm_msg->tailist.typeoflist =
        TRACKING_AREA_IDENTITY_LIST_ONE_PLMN_CONSECUTIVE_TACS;
    emm_msg->tailist.numberofelements = msg->n_tacs;
    emm_msg->tailist.mccdigit1 = msg->UEid.guti->gummei.plmn.MCCdigit1;
    emm_msg->tailist.mccdigit2 = msg->UEid.guti->gummei.plmn.MCCdigit2;
    emm_msg->tailist.mccdigit3 = msg->UEid.guti->gummei.plmn.MCCdigit3;
    emm_msg->tailist.mncdigit1 = msg->UEid.guti->gummei.plmn.MNCdigit1;
    emm_msg->tailist.mncdigit2 = msg->UEid.guti->gummei.plmn.MNCdigit2;
    emm_msg->tailist.mncdigit3 = msg->UEid.guti->gummei.plmn.MNCdigit3;
    emm_msg->tailist.tac = msg->tac;
    LOG_TRACE(INFO,
            "EMMAS-SAP - size += "\
            "TRACKING_AREA_IDENTITY_LIST_MINIMUM_LENGTH(%d)  (%d)",
            TRACKING_AREA_IDENTITY_LIST_MINIMUM_LENGTH,
            size);

    /* Mandatory - ESM message container */
    size += ESM_MESSAGE_CONTAINER_MINIMUM_LENGTH + msg->NASmsg.length;
    emm_msg->esmmessagecontainer.esmmessagecontainercontents = msg->NASmsg;
    LOG_TRACE(INFO,
            "EMMAS-SAP - size += "\
            "ESM_MESSAGE_CONTAINER_MINIMUM_LENGTH(%d)  (%d)",
            ESM_MESSAGE_CONTAINER_MINIMUM_LENGTH,
            size);

    /* Optional - GUTI */
    if (msg->new_guti) {
        size += EPS_MOBILE_IDENTITY_MAXIMUM_LENGTH;
        emm_msg->presencemask |= ATTACH_ACCEPT_GUTI_PRESENT;
        emm_msg->guti.guti.typeofidentity = EPS_MOBILE_IDENTITY_GUTI;
        emm_msg->guti.guti.oddeven = EPS_MOBILE_IDENTITY_EVEN;
        emm_msg->guti.guti.mmegroupid = msg->new_guti->gummei.MMEgid;
        emm_msg->guti.guti.mmecode = msg->new_guti->gummei.MMEcode;
        emm_msg->guti.guti.mtmsi = msg->new_guti->m_tmsi;
        emm_msg->guti.guti.mccdigit1 = msg->new_guti->gummei.plmn.MCCdigit1;
        emm_msg->guti.guti.mccdigit2 = msg->new_guti->gummei.plmn.MCCdigit2;
        emm_msg->guti.guti.mccdigit3 = msg->new_guti->gummei.plmn.MCCdigit3;
        emm_msg->guti.guti.mncdigit1 = msg->new_guti->gummei.plmn.MNCdigit1;
        emm_msg->guti.guti.mncdigit2 = msg->new_guti->gummei.plmn.MNCdigit2;
        emm_msg->guti.guti.mncdigit3 = msg->new_guti->gummei.plmn.MNCdigit3;
        LOG_TRACE(INFO,
                "EMMAS-SAP - size += "\
                "EPS_MOBILE_IDENTITY_MAXIMUM_LENGTH(%d)  (%d)",
                EPS_MOBILE_IDENTITY_MAXIMUM_LENGTH,
                size);
    }
    LOG_FUNC_RETURN (size);
}

/****************************************************************************
 **                                                                        **
 ** Name:    emm_send_attach_reject()                                  **
 **                                                                        **
 ** Description: Builds Attach Reject message                              **
 **                                                                        **
 **      The Attach Reject message is sent by the network to the   **
 **      UE to indicate that the corresponding attach request has  **
 **      been rejected.                                            **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     emm_msg:   The EMM message to be sent                 **
 **      Return:    The size of the EMM message                **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_send_attach_reject(const emm_as_establish_t *msg,
                           attach_reject_msg *emm_msg)
{
    LOG_FUNC_IN;

    int size = EMM_HEADER_MAXIMUM_LENGTH;

    LOG_TRACE(INFO, "EMMAS-SAP - Send Attach Reject message (cause=%d)",
              msg->emm_cause);

    /* Mandatory - Message type */
    emm_msg->messagetype = ATTACH_REJECT;

    /* Mandatory - EMM cause */
    size += EMM_CAUSE_MAXIMUM_LENGTH;
    emm_msg->emmcause = msg->emm_cause;

    /* Optional - ESM message container */
    if (msg->NASmsg.length > 0) {
        size += ESM_MESSAGE_CONTAINER_MINIMUM_LENGTH + msg->NASmsg.length;
        emm_msg->presencemask |= ATTACH_REJECT_ESM_MESSAGE_CONTAINER_PRESENT;
        emm_msg->esmmessagecontainer.esmmessagecontainercontents = msg->NASmsg;
    }

    LOG_FUNC_RETURN (size);
}

/****************************************************************************
 **                                                                        **
 ** Name:        emm_send_tracking_area_update_reject()                    **
 **                                                                        **
 ** Description: Builds Tracking Area Update Reject message                **
 **                                                                        **
 **              The Tracking Area Update Reject message is sent by the    **
 **              network to the UE to indicate that the corresponding      **
 **              tracking area update has been rejected.                   **
 **                                                                        **
 ** Inputs:      msg:           The EMMAS-SAP primitive to process         **
 **              Others:        None                                       **
 **                                                                        **
 ** Outputs:     emm_msg:       The EMM message to be sent                 **
 **              Return:        The size of the EMM message                **
 **              Others:        None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_send_tracking_area_update_reject(const emm_as_establish_t *msg,
                                         tracking_area_update_reject_msg *emm_msg)
{
    LOG_FUNC_IN;

    int size = EMM_HEADER_MAXIMUM_LENGTH;

    LOG_TRACE(INFO, "EMMAS-SAP - Send Tracking Area Update Reject message (cause=%d)",
              msg->emm_cause);

    /* Mandatory - Message type */
    emm_msg->messagetype = TRACKING_AREA_UPDATE_REJECT;

    /* Mandatory - EMM cause */
    size += EMM_CAUSE_MAXIMUM_LENGTH;
    emm_msg->emmcause = msg->emm_cause;

    LOG_FUNC_RETURN (size);
}

/****************************************************************************
 **                                                                        **
 ** Name:    emm_send_identity_request()                               **
 **                                                                        **
 ** Description: Builds Identity Request message                           **
 **                                                                        **
 **      The Identity Request message is sent by the network to    **
 **      the UE to request the UE to provide the specified identi- **
 **      ty.                                                       **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     emm_msg:   The EMM message to be sent                 **
 **      Return:    The size of the EMM message                **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_send_identity_request(const emm_as_security_t *msg,
                              identity_request_msg *emm_msg)
{
    LOG_FUNC_IN;

    int size = EMM_HEADER_MAXIMUM_LENGTH;

    LOG_TRACE(INFO, "EMMAS-SAP - Send Identity Request message");

    /* Mandatory - Message type */
    emm_msg->messagetype = IDENTITY_REQUEST;

    /* Mandatory - Identity type 2 */
    size += IDENTITY_TYPE_2_MAXIMUM_LENGTH;
    if (msg->identType == EMM_IDENT_TYPE_IMSI) {
        emm_msg->identitytype = IDENTITY_TYPE_2_IMSI;
    } else if (msg->identType == EMM_IDENT_TYPE_TMSI) {
        emm_msg->identitytype = IDENTITY_TYPE_2_TMSI;
    } else if (msg->identType == EMM_IDENT_TYPE_IMEI) {
        emm_msg->identitytype = IDENTITY_TYPE_2_IMEI;
    } else {
        /* All other values are interpreted as "IMSI" */
        emm_msg->identitytype = IDENTITY_TYPE_2_IMSI;
    }

    LOG_FUNC_RETURN (size);
}

/****************************************************************************
 **                                                                        **
 ** Name:    emm_send_authentication_request()                         **
 **                                                                        **
 ** Description: Builds Authentication Request message                     **
 **                                                                        **
 **      The Authentication Request message is sent by the network **
 **      to the UE to initiate authentication of the UE identity.  **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     emm_msg:   The EMM message to be sent                 **
 **      Return:    The size of the EMM message                **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_send_authentication_request(const emm_as_security_t *msg,
                                    authentication_request_msg *emm_msg)
{
    LOG_FUNC_IN;

    int size = EMM_HEADER_MAXIMUM_LENGTH;

    LOG_TRACE(INFO, "EMMAS-SAP - Send Authentication Request message");

    /* Mandatory - Message type */
    emm_msg->messagetype = AUTHENTICATION_REQUEST;

    /* Mandatory - NAS key set identifier */
    size += NAS_KEY_SET_IDENTIFIER_MAXIMUM_LENGTH;
    emm_msg->naskeysetidentifierasme.tsc = NAS_KEY_SET_IDENTIFIER_NATIVE;
    emm_msg->naskeysetidentifierasme.naskeysetidentifier = msg->ksi;

    /* Mandatory - Authentication parameter RAND */
    size += AUTHENTICATION_PARAMETER_RAND_MAXIMUM_LENGTH;
    emm_msg->authenticationparameterrand.rand = *msg->rand;

    /* Mandatory - Authentication parameter AUTN */
    size += AUTHENTICATION_PARAMETER_AUTN_MAXIMUM_LENGTH;
    emm_msg->authenticationparameterautn.autn = *msg->autn;

    LOG_FUNC_RETURN (size);
}

/****************************************************************************
 **                                                                        **
 ** Name:    emm_send_authentication_reject()                          **
 **                                                                        **
 ** Description: Builds Authentication Reject message                      **
 **                                                                        **
 **      The Authentication Reject message is sent by the network  **
 **      to the UE to indicate that the authentication procedure   **
 **      has failed and that the UE shall abort all activities.    **
 **                                                                        **
 ** Inputs:  None                                                      **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     emm_msg:   The EMM message to be sent                 **
 **      Return:    The size of the EMM message                **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_send_authentication_reject(authentication_reject_msg *emm_msg)
{
    LOG_FUNC_IN;

    int size = EMM_HEADER_MAXIMUM_LENGTH;

    LOG_TRACE(INFO, "EMMAS-SAP - Send Authentication Reject message");

    /* Mandatory - Message type */
    emm_msg->messagetype = AUTHENTICATION_REJECT;

    LOG_FUNC_RETURN (size);
}

/****************************************************************************
 **                                                                        **
 ** Name:    emm_send_security_mode_command()                          **
 **                                                                        **
 ** Description: Builds Security Mode Command message                      **
 **                                                                        **
 **      The Security Mode Command message is sent by the network  **
 **      to the UE to establish NAS signalling security.           **
 **                                                                        **
 ** Inputs:  msg:       The EMMAS-SAP primitive to process         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     emm_msg:   The EMM message to be sent                 **
 **      Return:    The size of the EMM message                **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_send_security_mode_command(const emm_as_security_t *msg,
                                   security_mode_command_msg *emm_msg)
{
    LOG_FUNC_IN;

    int size = EMM_HEADER_MAXIMUM_LENGTH;

    LOG_TRACE(INFO, "EMMAS-SAP - Send Security Mode Command message");

    /* Mandatory - Message type */
    emm_msg->messagetype = SECURITY_MODE_COMMAND;

    /* Selected NAS security algorithms */
    size += NAS_SECURITY_ALGORITHMS_MAXIMUM_LENGTH;
    emm_msg->selectednassecurityalgorithms.typeofcipheringalgorithm =
        msg->selected_eea;
    emm_msg->selectednassecurityalgorithms.typeofintegrityalgorithm =
        msg->selected_eia;

    /* NAS key set identifier */
    size += NAS_KEY_SET_IDENTIFIER_MAXIMUM_LENGTH;
    emm_msg->naskeysetidentifier.tsc = NAS_KEY_SET_IDENTIFIER_NATIVE;
    emm_msg->naskeysetidentifier.naskeysetidentifier = msg->ksi;

    /* Replayed UE security capabilities */
    size += UE_SECURITY_CAPABILITY_MAXIMUM_LENGTH;
    emm_msg->replayeduesecuritycapabilities.eea = msg->eea;
    emm_msg->replayeduesecuritycapabilities.eia = msg->eia;
    emm_msg->replayeduesecuritycapabilities.non_eps_security_present = 0;
    emm_msg->replayeduesecuritycapabilities.uea = 0x00;
    emm_msg->replayeduesecuritycapabilities.uia = 0x00;
    emm_msg->replayeduesecuritycapabilities.gea = 0x00;

    LOG_FUNC_RETURN (size);
}

#endif // NAS_MME

/****************************************************************************/
/*********************  L O C A L    F U N C T I O N S  *********************/
/****************************************************************************/
