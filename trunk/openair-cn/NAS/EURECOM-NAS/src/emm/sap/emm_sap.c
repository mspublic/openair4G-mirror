/*****************************************************************************
            Eurecom OpenAirInterface 3
            Copyright(c) 2012 Eurecom

Source      emm_sap.c

Version     0.1

Date        2012/10/01

Product     NAS stack

Subsystem   EPS Mobility Management

Author      Frederic Maurel

Description Defines the EMM Service Access Points at which the EPS
        Mobility Management sublayer provides procedures for the
        control of security and mobility when the User Equipment
        is using the Evolved UTRA Network.

*****************************************************************************/

#include "emm_sap.h"
#include "commonDef.h"
#include "nas_log.h"

#include "emm_reg.h"
#include "emm_esm.h"
#include "emm_as.h"

/****************************************************************************/
/****************  E X T E R N A L    D E F I N I T I O N S  ****************/
/****************************************************************************/

/****************************************************************************/
/*******************  L O C A L    D E F I N I T I O N S  *******************/
/****************************************************************************/

/****************************************************************************/
/******************  E X P O R T E D    F U N C T I O N S  ******************/
/****************************************************************************/

/****************************************************************************
 **                                                                        **
 ** Name:    emm_sap_initialize()                                      **
 **                                                                        **
 ** Description: Initializes the EMM Service Access Points                 **
 **                                                                        **
 ** Inputs:  None                                                      **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    None                                       **
 **      Others:    NONE                                       **
 **                                                                        **
 ***************************************************************************/
void emm_sap_initialize(void)
{
    LOG_FUNC_IN;

    emm_reg_initialize();
    emm_esm_initialize();
    emm_as_initialize();

    LOG_FUNC_OUT;
}

/****************************************************************************
 **                                                                        **
 ** Name:    emm_sap_send()                                            **
 **                                                                        **
 ** Description: Processes the EMM Service Access Point primitive          **
 **                                                                        **
 ** Inputs:  msg:       The EMM-SAP primitive to process           **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     msg:       The EMM-SAP primitive to process           **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_sap_send(emm_sap_t *msg)
{
    LOG_FUNC_IN;

    int rc = RETURNerror;

    emm_primitive_t primitive = msg->primitive;

    /* Check the EMM-SAP primitive */
    if ( (primitive > EMMREG_PRIMITIVE_MIN) &&
            (primitive < EMMREG_PRIMITIVE_MAX) ) {
        /* Forward to the EMMREG-SAP */
        msg->u.emm_reg.primitive = primitive;
        rc = emm_reg_send(&msg->u.emm_reg);
    } else if ( (primitive > EMMESM_PRIMITIVE_MIN) &&
                (primitive < EMMESM_PRIMITIVE_MAX) ) {
        /* Forward to the EMMESM-SAP */
        msg->u.emm_esm.primitive = primitive;
        rc = emm_esm_send(&msg->u.emm_esm);
    } else if ( (primitive > EMMAS_PRIMITIVE_MIN) &&
                (primitive < EMMAS_PRIMITIVE_MAX) ) {
        /* Forward to the EMMAS-SAP */
        msg->u.emm_as.primitive = primitive;
        rc = emm_as_send(&msg->u.emm_as);
    } else {
        LOG_TRACE(WARNING, "EMM-SAP -   Out of range primitive (%d)", primitive);
    }

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************/
/*********************  L O C A L    F U N C T I O N S  *********************/
/****************************************************************************/

