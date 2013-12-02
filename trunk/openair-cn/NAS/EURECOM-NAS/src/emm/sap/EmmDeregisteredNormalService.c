/*****************************************************************************
            Eurecom OpenAirInterface 3
            Copyright(c) 2012 Eurecom

Source      EmmDeregisteredNormalService.c

Version     0.1

Date        2012/10/03

Product     NAS stack

Subsystem   EPS Mobility Management

Author      Frederic Maurel

Description Implements the EPS Mobility Management procedures executed
        when the EMM-SAP is in EMM-DEREGISTERED.NORMAL-SERVICE state.

        In EMM-DEREGISTERED.NORMAL-SERVICE state, the EPS update
        status is EU1 or EU2, in the meantime a suitable cell has
        been found and the PLMN or tracking area is not in the
        forbidden list.

*****************************************************************************/

#ifdef NAS_UE

#include "emm_fsm.h"
#include "commonDef.h"
#include "networkDef.h"
#include "nas_log.h"

#include "emm_proc.h"

#include <assert.h>

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
 ** Name:    EmmDeregisteredNormalService()                            **
 **                                                                        **
 ** Description: Handles the behaviour of the UE while the EMM-SAP is in   **
 **      EMM-DEREGISTERED.NORMAL-SERVICE state.                    **
 **                                                                        **
 **              3GPP TS 24.301, section 5.2.2.3.1                         **
 **      The UE shall initiate an attach or combined attach proce- **
 **      dure.                                                     **
 **                                                                        **
 ** Inputs:  evt:       The received EMM-SAP event                 **
 **      Others:    emm_fsm_status                             **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    emm_fsm_status                             **
 **                                                                        **
 ***************************************************************************/
int EmmDeregisteredNormalService(const emm_reg_t *evt)
{
    LOG_FUNC_IN;

    int rc = RETURNerror;

    assert(emm_fsm_get_status() == EMM_DEREGISTERED_NORMAL_SERVICE);

    switch (evt->primitive) {
        case _EMMREG_REGISTER_REQ:
            /*
             * The user manually re-selected a PLMN to register to
             */
            rc = emm_fsm_set_status(EMM_DEREGISTERED_PLMN_SEARCH);
            if (rc != RETURNerror) {
                /* Process the network registration request */
                rc = emm_fsm_process(evt);
            }
            break;

        case _EMMREG_ATTACH_INIT:
            /*
             * Initiate the attach procedure for EPS services
             */
            rc = emm_proc_attach(EMM_ATTACH_TYPE_EPS);
            break;

        case _EMMREG_ATTACH_REQ:
            /*
             * An EPS network attach has been requested (Attach Request
             * message successfully delivered to the network);
             * enter state EMM-REGISTERED-INITIATED
             */
            rc = emm_fsm_set_status(EMM_REGISTERED_INITIATED);
            break;

        case _EMMREG_LOWERLAYER_SUCCESS:
            /*
             * Initial NAS message has been successfully delivered
             * to the network
             */
            rc = emm_proc_lowerlayer_success();
            break;

        case _EMMREG_LOWERLAYER_FAILURE:
            /*
             * Initial NAS message failed to be delivered to the network
             */
            rc = emm_proc_lowerlayer_failure(TRUE);
            break;

        case _EMMREG_LOWERLAYER_RELEASE:
            /*
             * NAS signalling connection has been released
             */
            rc = emm_proc_lowerlayer_release();
            break;

        case _EMMREG_ATTACH_CNF:
            /*
             * Attach procedure successful and default EPS bearer
             * context activated;
             * enter state EMM-REGISTERED.
             */
            rc = emm_fsm_set_status(EMM_REGISTERED);
            break;

        default:
            LOG_TRACE(ERROR, "EMM-FSM   - Primitive is not valid (%d)",
                      evt->primitive);
            break;
    }

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************/
/*********************  L O C A L    F U N C T I O N S  *********************/
/****************************************************************************/

#endif
