/*****************************************************************************
            Eurecom OpenAirInterface 3
            Copyright(c) 2012 Eurecom

Source      EmmDeregisteredNoCellAvailable.c

Version     0.1

Date        2012/10/03

Product     NAS stack

Subsystem   EPS Mobility Management

Author      Frederic Maurel

Description Implements the EPS Mobility Management procedures executed
        when the EMM-SAP is in EMM-DEREGISTERED.NO-CELL-AVAILABLE
        state.

        In EMM-DEREGISTERED.NO-CELL-AVAILABLE state, no E-UTRAN cell
        can be selected. A first intensive search failed when in
        substate EMM_DEREGISTERED.PLMN-SEARCH. Cells are searched for
        at a low rhythm. No EPS services are offered.

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
 ** Name:    EmmDeregisteredNoCellAvailable()                          **
 **                                                                        **
 ** Description: Handles the behaviour of the UE while the EMM-SAP is in   **
 **      EMM-DEREGISTERED.NO-CELL-AVAILABLE state.                 **
 **                                                                        **
 **              3GPP TS 24.301, section 5.2.2.3.7                         **
 **      The UE shall perform cell selection and choose an appro-  **
 **      priate substate when a cell is found.                     **
 **                                                                        **
 ** Inputs:  evt:       The received EMM-SAP event                 **
 **      Others:    emm_fsm_status                             **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    emm_fsm_status                             **
 **                                                                        **
 ***************************************************************************/
int EmmDeregisteredNoCellAvailable(const emm_reg_t *evt)
{
    LOG_FUNC_IN;

    int rc = RETURNerror;

    assert(emm_fsm_get_status() == EMM_DEREGISTERED_NO_CELL_AVAILABLE);

    switch (evt->primitive) {
            /* TODO: network re-selection is not allowed when in No Cell
             * Available substate. The AS should search for a suitable cell
             * and notify the NAS when such a cell is found (TS 24.008 section
             * 4.2.4.1.2) */
        case _EMMREG_REGISTER_REQ:
            /*
             * The user manually re-selected a PLMN to register to
             */
            rc = emm_fsm_set_status(EMM_DEREGISTERED_PLMN_SEARCH);

            if (rc != RETURNerror) {
                /*
                 * Notify EMM that the MT is currently searching an operator
                 * to register to
                 */
                rc = emm_proc_registration_notify(NET_REG_STATE_ON);
                if (rc != RETURNok) {
                    LOG_TRACE(WARNING, "EMM-FSM   - "
                              "Failed to notify registration update");
                }
                /*
                 * Perform network re-selection procedure
                 */
                rc = emm_proc_plmn_selection(evt->u.regist.index);
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
