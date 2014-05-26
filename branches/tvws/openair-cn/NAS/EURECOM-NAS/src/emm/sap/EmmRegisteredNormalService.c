/*****************************************************************************
            Eurecom OpenAirInterface 3
            Copyright(c) 2012 Eurecom

Source      EmmRegisteredNormalService.c

Version     0.1

Date        2012/10/03

Product     NAS stack

Subsystem   EPS Mobility Management

Author      Frederic Maurel

Description Implements the EPS Mobility Management procedures executed
        when the EMM-SAP is in EMM-REGISTERED.NORMAL-SERVICE state.

        The EMM-REGISTERED.NORMAL-SERVICE state is the primary
        substate choosen when the UE enters the state EMM-REGISTERED.

*****************************************************************************/

#ifdef NAS_UE

#include "emm_fsm.h"
#include "commonDef.h"
#include "nas_log.h"

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
 ** Name:    EmmRegisteredNormalService()                              **
 **                                                                        **
 ** Description: Handles the behaviour of the UE while the EMM-SAP is in   **
 **      EMM-REGISTERED.NORMAL-SERVICE state.                      **
 **                                                                        **
 ** Inputs:  evt:       The received EMM-SAP event                 **
 **      Others:    emm_fsm_status                             **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    emm_fsm_status                             **
 **                                                                        **
 ***************************************************************************/
int EmmRegisteredNormalService(const emm_reg_t *evt)
{
    LOG_FUNC_IN;

    assert(emm_fsm_get_status() == EMM_REGISTERED_NORMAL_SERVICE);

    /* TODO */

    LOG_FUNC_RETURN (RETURNok);
}

/****************************************************************************/
/*********************  L O C A L    F U N C T I O N S  *********************/
/****************************************************************************/

#endif
