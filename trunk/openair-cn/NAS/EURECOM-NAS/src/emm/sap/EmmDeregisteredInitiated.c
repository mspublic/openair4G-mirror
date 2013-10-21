/*****************************************************************************
			Eurecom OpenAirInterface 3
			Copyright(c) 2012 Eurecom

Source		EmmDeregisteredInitiated.c

Version		0.1

Date		2012/10/03

Product		NAS stack

Subsystem	EPS Mobility Management

Author		Frederic Maurel

Description	Implements the EPS Mobility Management procedures executed
		when the EMM-SAP is in EMM-DEREGISTERED-INITIATED state.

		In EMM-DEREGISTERED-INITIATED state, the UE has requested
		release of the EMM context by starting the detach or combined
		detach procedure and is waiting for a response from the MME.
		The MME has started a detach procedure and is waiting for a
		response from the UE.

*****************************************************************************/

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
 ** Name:	 EmmDeregisteredInitiated()                                **
 **                                                                        **
 ** Description: Handles the behaviour of the UE and the MME while the     **
 **		 EMM-SAP is in EMM-DEREGISTERED-INITIATED state.           **
 **                                                                        **
 ** Inputs:	 evt:		The received EMM-SAP event                 **
 **		 Others:	emm_fsm_status                             **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	emm_fsm_status                             **
 **                                                                        **
 ***************************************************************************/
int EmmDeregisteredInitiated(const emm_reg_t* evt)
{
    LOG_FUNC_IN;

    int rc = RETURNerror;

#ifdef NAS_UE
    assert(emm_fsm_get_status() == EMM_DEREGISTERED_INITIATED);
#endif
#ifdef NAS_MME
    assert(emm_fsm_get_status(evt->ueid, evt->ctx) == EMM_DEREGISTERED_INITIATED);
#endif

    switch (evt->primitive)
    {
#ifdef NAS_UE
	case _EMMREG_DETACH_CNF:
	    /*
	     * The UE explicitly detached from the network (all EPS
	     * bearer contexts have been deactivated as UE initiated
	     * detach procedure successfully completed)
	     */
	    rc = emm_fsm_set_status(EMM_DEREGISTERED);
	    break;

	case _EMMREG_DETACH_FAILED:
	    /*
	     * The detach procedure failed
	     */
	    if (evt->u.detach.type == EMM_DETACH_TYPE_IMSI) {
		rc = emm_fsm_set_status(EMM_REGISTERED_NORMAL_SERVICE);
	    } else {
		rc = emm_fsm_set_status(EMM_DEREGISTERED);
	    }
	    break;

	case _EMMREG_LOWERLAYER_SUCCESS:
	    /*
	     * Ignore Detach Request message successful retransmission
	     */
	    rc = RETURNok;
	    break;

	case _EMMREG_LOWERLAYER_FAILURE:
	case _EMMREG_LOWERLAYER_RELEASE:
	    /*
	     * Lower layer failure or release of the NAS signalling connection
	     * before the Detach Accept is received
	     */
	    rc = emm_proc_lowerlayer_release();
	    break;
#endif

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

