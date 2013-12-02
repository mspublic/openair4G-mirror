/*****************************************************************************
            Eurecom OpenAirInterface 3
            Copyright(c) 2012 Eurecom

Source      emm_esm.c

Version     0.1

Date        2012/10/16

Product     NAS stack

Subsystem   EPS Mobility Management

Author      Frederic Maurel

Description Defines the EMMESM Service Access Point that provides
        interlayer services to the EPS Session Management sublayer
        for service registration and activate/deactivate PDN
        connections.

*****************************************************************************/

#include "emm_esm.h"
#include "commonDef.h"
#include "nas_log.h"

#include "LowerLayer.h"

#ifdef NAS_UE
#include "emm_proc.h"
#endif

/****************************************************************************/
/****************  E X T E R N A L    D E F I N I T I O N S  ****************/
/****************************************************************************/

/****************************************************************************/
/*******************  L O C A L    D E F I N I T I O N S  *******************/
/****************************************************************************/

/*
 * String representation of EMMESM-SAP primitives
 */
static const char *_emm_esm_primitive_str[] = {
#ifdef NAS_UE
    "EMMESM_ESTABLISH_REQ",
    "EMMESM_ESTABLISH_CNF",
    "EMMESM_ESTABLISH_REJ",
#endif
    "EMMESM_RELEASE_IND",
    "EMMESM_UNITDATA_REQ",
    "EMMESM_UNITDATA_IND",
};

/****************************************************************************/
/******************  E X P O R T E D    F U N C T I O N S  ******************/
/****************************************************************************/

/****************************************************************************
 **                                                                        **
 ** Name:    emm_esm_initialize()                                      **
 **                                                                        **
 ** Description: Initializes the EMMESM Service Access Point               **
 **                                                                        **
 ** Inputs:  None                                                      **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    None                                       **
 **      Others:    NONE                                       **
 **                                                                        **
 ***************************************************************************/
void emm_esm_initialize(void)
{
    LOG_FUNC_IN;

    /* TODO: Initialize the EMMESM-SAP */

    LOG_FUNC_OUT;
}

/****************************************************************************
 **                                                                        **
 ** Name:    emm_esm_send()                                            **
 **                                                                        **
 ** Description: Processes the EMMESM Service Access Point primitive       **
 **                                                                        **
 ** Inputs:  msg:       The EMMESM-SAP primitive to process        **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_esm_send(const emm_esm_t *msg)
{
    LOG_FUNC_IN;

    int rc = RETURNerror;
    emm_esm_primitive_t primitive = msg->primitive;

    LOG_TRACE(INFO, "EMMESM-SAP - Received primitive %s (%d)",
              _emm_esm_primitive_str[primitive - _EMMESM_START - 1], primitive);

    switch (primitive) {
#ifdef NAS_UE
        case _EMMESM_ESTABLISH_REQ:
            /* ESM requests EMM to initiate an attach procedure before
             * requesting subsequent connectivity to additional PDNs */
            rc = emm_proc_attach_restart();
            break;

        case _EMMESM_ESTABLISH_CNF:
            /* ESM notifies EMM that PDN connectivity procedure successfully
             * processed */
            if (msg->u.establish.is_attached) {
                if (msg->u.establish.is_emergency) {
                    /* Consider the UE attached for emergency bearer services
                     * only */
                    rc = emm_proc_attach_set_emergency();
                }
            } else {
                /* Consider the UE locally detached from the network */
                rc = emm_proc_attach_set_detach();
            }
            break;

        case _EMMESM_ESTABLISH_REJ:
            /* ESM notifies EMM that PDN connectivity procedure failed */
            break;
#endif
        case _EMMESM_UNITDATA_REQ:
            /* ESM requests EMM to transfer ESM data unit to lower layer */
            rc = lowerlayer_data_req(msg->ueid, &msg->u.data.msg);
            break;

        default:
            break;

    }

    if (rc != RETURNok) {
        LOG_TRACE(WARNING, "EMMESM-SAP - Failed to process primitive %s (%d)",
                  _emm_esm_primitive_str[primitive - _EMMESM_START - 1],
                  primitive);
    }

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************/
/*********************  L O C A L    F U N C T I O N S  *********************/
/****************************************************************************/
