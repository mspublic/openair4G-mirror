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

  Address      : Eurecom, Compus SophiaTech 450, route des chappes, 06451 Biot, France.

 *******************************************************************************/
/*****************************************************************************

Source      EmmCommonProcedureInitiated.c

Version     0.1

Date        2012/10/03

Product     NAS stack

Subsystem   EPS Mobility Management

Author      Frederic Maurel

Description Implements the EPS Mobility Management procedures executed
        when the EMM-SAP is in EMM-COMMON-PROCEDURE-INITIATED state.

        In EMM-COMMON-PROCEDURE-INITIATED state, the MME has started
        a common EMM procedure and is waiting for a response from the
        UE.

*****************************************************************************/

#ifdef NAS_MME

#include "emm_fsm.h"
#include "commonDef.h"
#include "nas_log.h"

#include "EmmCommon.h"

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
 ** Name:    EmmCommonProcedureInitiated()                             **
 **                                                                        **
 ** Description: Handles the behaviour of the MME while the EMM-SAP is in  **
 **      EMM_COMMON_PROCEDURE_INITIATED state.                     **
 **                                                                        **
 **              3GPP TS 24.301, section 5.1.3.4.2                         **
 **                                                                        **
 ** Inputs:  evt:       The received EMM-SAP event                 **
 **      Others:    emm_fsm_status                             **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    emm_fsm_status                             **
 **                                                                        **
 ***************************************************************************/
int EmmCommonProcedureInitiated(const emm_reg_t *evt)
{
    int rc = RETURNerror;

    LOG_FUNC_IN;

    assert(emm_fsm_get_status(evt->ueid,
                              evt->ctx) == EMM_COMMON_PROCEDURE_INITIATED);

    switch (evt->primitive) {
        case _EMMREG_PROC_ABORT:
            /*
             * The EMM procedure that initiated EMM common procedure aborted
             */
            rc = emm_proc_common_abort(evt->ueid);
            break;

        case _EMMREG_COMMON_PROC_CNF:
            /*
             * An EMM common procedure successfully completed;
             */
            if (evt->u.common.is_attached) {
                rc = emm_fsm_set_status(evt->ueid, evt->ctx, EMM_REGISTERED);
            } else {
                rc = emm_fsm_set_status(evt->ueid, evt->ctx, EMM_DEREGISTERED);
            }
            if (rc != RETURNerror) {
                rc = emm_proc_common_success(evt->ueid);
            }
            break;

        case _EMMREG_COMMON_PROC_REJ:
            /*
             * An EMM common procedure failed;
             * enter state EMM-DEREGISTERED.
             */
            rc = emm_fsm_set_status(evt->ueid, evt->ctx, EMM_DEREGISTERED);
            if (rc != RETURNerror) {
                rc = emm_proc_common_reject(evt->ueid);
            }
            break;

        case _EMMREG_ATTACH_CNF:
            /*
             * Attach procedure successful and default EPS bearer
             * context activated;
             * enter state EMM-REGISTERED.
             */
            rc = emm_fsm_set_status(evt->ueid, evt->ctx, EMM_REGISTERED);
            break;

        case _EMMREG_ATTACH_REJ:
            /*
             * Attach procedure failed;
             * enter state EMM-DEREGISTERED.
             */
            rc = emm_fsm_set_status(evt->ueid, evt->ctx, EMM_DEREGISTERED);
            break;

        case _EMMREG_LOWERLAYER_SUCCESS:
            /*
             * Data successfully delivered to the network
             */
            rc = RETURNok;
            break;

        case _EMMREG_LOWERLAYER_FAILURE:
            /*
             * Transmission failure occurred before the EMM common
             * procedure being completed
             */
            rc = emm_proc_common_failure(evt->ueid);
            if (rc != RETURNerror) {
                rc = emm_fsm_set_status(evt->ueid, evt->ctx, EMM_DEREGISTERED);
            }
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
