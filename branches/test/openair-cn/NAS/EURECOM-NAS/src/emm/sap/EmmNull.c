/*****************************************************************************
			Eurecom OpenAirInterface 3
			Copyright(c) 2012 Eurecom

Source		EmmNull.c

Version		0.1

Date		2012/10/03

Product		NAS stack

Subsystem	EPS Mobility Management

Author		Frederic Maurel

Description	Implements the EPS Mobility Management procedures executed
		when the EMM-SAP is in EMM-NULL state.

		In EMM-NULL state, the EPS capability is disabled in the UE.
		No EPS mobility management function shall be performed.

*****************************************************************************/

#ifdef NAS_UE

#include "emm_fsm.h"
#include "commonDef.h"
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
 ** Name:	 EmmNull()                                                 **
 **                                                                        **
 ** Description: Handles the behaviour of the UE while the EMM-SAP is in   **
 **		 EMM-NULL state.                                           **
 **                                                                        **
 ** Inputs:	 evt:		The received EMM-SAP event                 **
 **		 Others:	emm_fsm_status                             **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	emm_fsm_status                             **
 **                                                                        **
 ***************************************************************************/
int EmmNull(const emm_reg_t* evt)
{
    LOG_FUNC_IN;

    int rc;

    assert(emm_fsm_get_status() == EMM_NULL);

    /* Delete the authentication data RAND and RES */
    rc = emm_proc_authentication_delete();
    if (rc != RETURNok) {
	LOG_FUNC_RETURN (rc);
    }

    switch (evt->primitive)
    {
	case _EMMREG_S1_ENABLED:
	    /*
	     * The EPS capability has been enabled in the UE:
	     * Move to the DEREGISTERED state;
	     */
	    rc = emm_fsm_set_status(EMM_DEREGISTERED);

	    /*
	     * And initialize the EMM procedure call manager in order to
	     * establish an EMM context and make the UE reachable by an MME.
	     */
	    if (rc != RETURNerror) {
		rc = emm_proc_initialize();
	    }
	    break;

	default:
	    rc = RETURNerror;
	    LOG_TRACE(ERROR, "EMM-FSM   - Primitive is not valid (%d)",
		      evt->primitive);
	    LOG_TRACE(WARNING, "EMM-FSM   - Set phone functionnality to "
		      "enable EPS capability (+cfun=1)");
	    break;
    }

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************/
/*********************  L O C A L    F U N C T I O N S  *********************/
/****************************************************************************/

#endif
