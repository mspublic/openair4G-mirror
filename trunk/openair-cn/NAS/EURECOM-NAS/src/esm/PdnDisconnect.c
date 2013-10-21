/*****************************************************************************
			Eurecom OpenAirInterface 3
			Copyright(c) 2012 Eurecom

Source		PdnDisconnect.c

Version		0.1

Date		2013/05/15

Product		NAS stack

Subsystem	EPS Session Management

Author		Frederic Maurel

Description	Defines the PDN disconnect ESM procedure executed by the
		Non-Access Stratum.

		The PDN disconnect procedure is used by the UE to request
		disconnection from one PDN.

		All EPS bearer contexts established towards this PDN, inclu-
		ding the default EPS bearer context, are released.

*****************************************************************************/

#include "esm_proc.h"
#include "commonDef.h"
#include "nas_log.h"

#include "esmData.h"
#include "esm_cause.h"
#include "esm_pt.h"

#ifdef NAS_UE
#include "esm_sap.h"
#endif

#include "emm_sap.h"

/****************************************************************************/
/****************  E X T E R N A L    D E F I N I T I O N S  ****************/
/****************************************************************************/

#ifdef NAS_MME
extern int _pdn_connectivity_delete(unsigned int ueid, int pid);
#endif

/****************************************************************************/
/*******************  L O C A L    D E F I N I T I O N S  *******************/
/****************************************************************************/

/*
 * --------------------------------------------------------------------------
 *	Internal data handled by the PDN disconnect procedure in the UE
 * --------------------------------------------------------------------------
 */
#ifdef NAS_UE
/*
 * PDN disconnection handlers
 */
static int _pdn_disconnect_get_default_ebi(int pti);

/*
 * Timer handlers
 */
static void* _pdn_disconnect_t3492_handler(void*);

/* Maximum value of the PDN disconnect request retransmission counter */
#define ESM_PDN_DISCONNECT_COUNTER_MAX 5

#endif // NAS_UE

/*
 * --------------------------------------------------------------------------
 *	Internal data handled by the PDN disconnect procedure in the MME
 * --------------------------------------------------------------------------
 */
#ifdef NAS_MME
/*
 * PDN disconnection handlers
 */
static int _pdn_disconnect_get_pid(unsigned int ueid, int pti);

#endif // NAS_MME

/****************************************************************************/
/******************  E X P O R T E D    F U N C T I O N S  ******************/
/****************************************************************************/

/*
 * --------------------------------------------------------------------------
 * 		  PDN disconnect procedure executed by the UE
 * --------------------------------------------------------------------------
 */
#ifdef NAS_UE
/****************************************************************************
 **                                                                        **
 ** Name:	 esm_proc_pdn_disconnect()                                 **
 **                                                                        **
 ** Description: Return the procedure transaction identity assigned to the **
 **		 PDN connection and the EPS bearer identity of the default **
 **		 bearer associated to the PDN context with specified iden- **
 **		 tifier                                                    **
 **                                                                        **
 ** Inputs:	 cid:		PDN context identifier                     **
 **		 Others:	_esm_data                                  **
 **                                                                        **
 ** Outputs:	 pti:		Procedure transaction identity assigned to **
 **				the PDN connection to be released          **
 **		 ebi:		EPS bearer identity of the default bearer  **
 **				associated to the specified PDN context    **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
int esm_proc_pdn_disconnect(int cid, unsigned int* pti, unsigned int* ebi)
{
    LOG_FUNC_IN;

    int rc = RETURNerror;
    int pid = cid - 1;

    if (pid < ESM_DATA_PDN_MAX) {
	if (pid != _esm_data.pdn[pid].pid) {
	    LOG_TRACE(WARNING, "ESM-PROC  - PDN connection identifier %d is "
		      "not valid", pid);
	}
	else if (_esm_data.pdn[pid].data == NULL) {
	    LOG_TRACE(ERROR, "ESM-PROC  - PDN connection %d has not been "
		      "allocated", pid);
	}
	else if (!_esm_data.pdn[pid].is_active) {
	    LOG_TRACE(WARNING, "ESM-PROC  - PDN connection is not active");
	}
	else {
	    /* Get the procedure transaction identity assigned to the PDN
	     * connection to be released */
	    *pti = _esm_data.pdn[pid].data->pti;
	    /* Get the EPS bearer identity of the default bearer associated
	     * with the PDN to disconnect from */
	    *ebi = _esm_data.pdn[pid].data->bearer[0]->ebi;
	    rc = RETURNok;
	}
    }

    LOG_FUNC_RETURN(rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 esm_proc_pdn_disconnect_request()                         **
 **                                                                        **
 ** Description: Initiates PDN disconnection procedure in order to request **
 **		 disconnection from a PDN.                                 **
 **                                                                        **
 **              3GPP TS 24.301, section 6.5.2.2                           **
 **		 The UE requests PDN disconnection from a PDN by sending a **
 **		 PDN DISCONNECT REQUEST message to the MME, starting timer **
 **		 T3492 and entering state PROCEDURE TRANSACTION PENDING.   **
 **                                                                        **
 ** Inputs:	 is_standalone:	Should be always TRUE                      **
 **		 pti:		Procedure transaction identity             **
 **		 msg:		Encoded PDN disconnect request message to  **
 **				be sent                                    **
 **		 sent_by_ue:	Not used                                   **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
int esm_proc_pdn_disconnect_request(int is_standalone, int pti,
				    OctetString* msg, int sent_by_ue)
{
    LOG_FUNC_IN;

    int rc = RETURNok;

    LOG_TRACE(INFO, "ESM-PROC  - Initiate PDN disconnection (pti=%d)", pti);

    if (is_standalone)
    {
	emm_sap_t emm_sap;
	emm_esm_data_t *emm_esm = &emm_sap.u.emm_esm.u.data;
	/*
	 * Notity EMM that ESM PDU has to be forwarded to lower layers
	 */
	emm_sap.primitive = EMMESM_UNITDATA_REQ;
	emm_sap.u.emm_esm.ueid = 0;
	emm_esm->msg.length = msg->length;
	emm_esm->msg.value = msg->value;
	rc = emm_sap_send(&emm_sap);

	if (rc != RETURNerror) {
	    /* Start T3482 retransmission timer */
	    rc = esm_pt_start_timer(pti, msg, T3492_DEFAULT_VALUE,
				    _pdn_disconnect_t3492_handler);
	}
    }

    if (rc != RETURNerror) {
	/* Set the procedure transaction state to PENDING */
	rc = esm_pt_set_status(pti, ESM_PT_PENDING);
	if (rc != RETURNok) {
	    /* The procedure transaction was already in PENDING state */
	    LOG_TRACE(WARNING, "ESM-PROC  - PTI %d was already PENDING", pti);
	}
    }
    LOG_FUNC_RETURN(rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 esm_proc_pdn_disconnect_accept()                          **
 **                                                                        **
 ** Description: Performs PDN disconnection procedure accepted by the net- **
 **		 work.                                                     **
 **                                                                        **
 **              3GPP TS 24.301, section 6.5.2.3                           **
 **		 The shall stop timer T3492 and enter the state PROCEDURE  **
 **		 TRANSACTION INACTIVE.                                     **
 **                                                                        **
 ** Inputs:	 pti:		Identifies the UE requested PDN disconnect **
 **				procedure accepted by the network          **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 esm_cause:	Cause code returned upon ESM procedure     **
 **				failure                                    **
 **		 Return:	The identifier of the PDN context to de-   **
 **				activate when successfully found;          **
 **				RETURNerror otherwise.                     **
 **		 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
int esm_proc_pdn_disconnect_accept(int pti, int* esm_cause)
{
    LOG_FUNC_IN;

    LOG_TRACE(INFO, "ESM-PROC  - PDN disconnection accepted by the network "
	      "(pti=%d)", pti);

    /* Stop T3492 timer if running */
    (void) esm_pt_stop_timer(pti);
    /* Set the procedure transaction state to INACTIVE */
    int rc = esm_pt_set_status(pti, ESM_PT_INACTIVE);
    if (rc != RETURNok) {
	/* The procedure transaction was already in INACTIVE state */
	LOG_TRACE(WARNING, "ESM-PROC  - PTI %d was already INACTIVE", pti);
	*esm_cause = ESM_CAUSE_MESSAGE_TYPE_NOT_COMPATIBLE;
    }
    else {
	/* Immediately release the transaction identity assigned to this
	 * procedure */
	rc = esm_pt_release(pti);
	if (rc != RETURNok) {
	    LOG_TRACE(WARNING, "ESM-PROC  - Failed to release PTI %d", pti);
	}
    }

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 esm_proc_pdn_disconnect_reject()                          **
 **                                                                        **
 ** Description: Performs PDN disconnection procedure not accepted by the  **
 **		 network.                                                  **
 **                                                                        **
 **              3GPP TS 24.301, section 6.5.2.4                           **
 **		 Upon receipt of the PDN DISCONNECT REJECT message, the UE **
 **		 shall stop timer T3492 and enter the state PROCEDURE      **
 **		 TRANSACTION INACTIVE and abort the PDN disconnection pro- **
 **		 cedure.                                                   **
 **                                                                        **
 ** Inputs:	 pti:		Identifies the UE requested PDN disconnec- **
 **				tion procedure rejected by the network     **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 esm_cause:	Cause code returned upon ESM procedure     **
 **				failure                                    **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
int esm_proc_pdn_disconnect_reject(int pti, int* esm_cause)
{
    LOG_FUNC_IN;

    int rc;

    LOG_TRACE(WARNING, "ESM-PROC  - PDN disconnection rejected by the network "
	      "(pti=%d), ESM cause = %d", pti, *esm_cause);

    /* Stop T3492 timer if running */
    (void) esm_pt_stop_timer(pti);
    /* Set the procedure transaction state to INACTIVE */
    rc = esm_pt_set_status(pti, ESM_PT_INACTIVE);
    if (rc != RETURNok) {
	/* The procedure transaction was already in INACTIVE state
	 * as the request may have already been rejected */
	LOG_TRACE(WARNING, "ESM-PROC  - PTI %d was already INACTIVE", pti);
	*esm_cause = ESM_CAUSE_MESSAGE_TYPE_NOT_COMPATIBLE;
    }
    else {
	/* Release the transaction identity assigned to this procedure */
	rc = esm_pt_release(pti);
	if (rc != RETURNok) {
	    LOG_TRACE(WARNING, "ESM-PROC  - Failed to release PTI %d", pti);
	    *esm_cause = ESM_CAUSE_REQUEST_REJECTED_UNSPECIFIED;
	}
	else if (*esm_cause != ESM_CAUSE_LAST_PDN_DISCONNECTION_NOT_ALLOWED)
	{
	    /* Get the identity of the default EPS bearer context allocated to
	     * the PDN connection entry assigned to this procedure transaction */
	    int ebi = _pdn_disconnect_get_default_ebi(pti);
	    if (ebi < 0) {
		LOG_TRACE(ERROR, "ESM-PROC  - No default EPS bearer found");
		*esm_cause = ESM_CAUSE_PROTOCOL_ERROR;
		LOG_FUNC_RETURN (RETURNerror);
	    }
	    /*
	     * Notify ESM that all EPS bearer contexts to this PDN have to be
	     * locally deactivated
	     */
	    esm_sap_t esm_sap;
	    esm_sap.primitive = ESM_EPS_BEARER_CONTEXT_DEACTIVATE_REQ;
	    esm_sap.is_standalone = TRUE;
	    esm_sap.recv = NULL;
	    esm_sap.send.length = 0;
	    esm_sap.data.eps_bearer_context_deactivate.ebi = ebi;
	    rc = esm_sap_send(&esm_sap);

	    if (rc != RETURNok) {
		*esm_cause = ESM_CAUSE_PROTOCOL_ERROR;
	    }
	}
    }

    LOG_FUNC_RETURN(rc);
}

#endif // NAS_UE


/*
 * --------------------------------------------------------------------------
 * 		  PDN disconnect procedure executed by the MME
 * --------------------------------------------------------------------------
 */
#ifdef NAS_MME
/****************************************************************************
 **                                                                        **
 ** Name:	 esm_proc_pdn_disconnect_request()                         **
 **                                                                        **
 ** Description: Performs PDN disconnect procedure requested by the UE.    **
 **                                                                        **
 **              3GPP TS 24.301, section 6.5.2.3                           **
 **		 Upon receipt of the PDN DISCONNECT REQUEST message, if it **
 **		 is accepted by the network, the MME shall initiate the    **
 **		 bearer context deactivation procedure.                    **
 **                                                                        **
 ** Inputs:	 ueid:		UE lower layer identifier                  **
 **		 pti:		Identifies the PDN disconnect procedure    **
 **				requested by the UE                        **
 **		 Others:	_esm_data                                  **
 **                                                                        **
 ** Outputs:	 esm_cause:	Cause code returned upon ESM procedure     **
 **				failure                                    **
 **		 Return:	The identifier of the PDN connection to be **
 **				released, if it exists;                    **
 **		 		RETURNerror otherwise.                     **
 **		 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
int esm_proc_pdn_disconnect_request(unsigned int ueid, int pti, int* esm_cause)
{
    LOG_FUNC_IN;

    int pid = RETURNerror;

    LOG_TRACE(INFO, "ESM-PROC  - PDN disconnect requested by the UE "
	      "(ueid=%d, pti=%d)", ueid, pti);

    if (ueid < ESM_DATA_NB_UE_MAX)
    {
	/* Get UE's ESM context */
	esm_data_context_t* ctx = _esm_data.ctx[ueid];
	if (_esm_data.ctx[ueid] == NULL) {
	    LOG_TRACE(ERROR, "ESM-PROC  - No ESM context exists");
	    *esm_cause = ESM_CAUSE_PROTOCOL_ERROR;
	}
	else if (ctx->n_pdns > 1) {
	    /* Get the identifier of the PDN connection entry assigned to the
	     * procedure transaction identity */
	    pid = _pdn_disconnect_get_pid(ueid, pti);
	    if (pid < 0) {
		LOG_TRACE(ERROR, "ESM-PROC  - No PDN connection found (pti=%d)",
			  pti);
		*esm_cause = ESM_CAUSE_PROTOCOL_ERROR;
		LOG_FUNC_RETURN (RETURNerror);
	    }	    
	}
	else {
	    /* Attempt to disconnect from the last PDN disconnection
	     * is not allowed */
	    *esm_cause = ESM_CAUSE_LAST_PDN_DISCONNECTION_NOT_ALLOWED;
	}
    }

    LOG_FUNC_RETURN(pid);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 esm_proc_pdn_disconnect_accept()                          **
 **                                                                        **
 ** Description: Performs PDN disconnect procedure accepted by the UE.     **
 **                                                                        **
 **              3GPP TS 24.301, section 6.5.2.3                           **
 **		 On reception of DEACTIVATE EPS BEARER CONTEXT ACCEPT mes- **
 **		 sage from the UE, the MME releases all the resources re-  **
 **		 served for the PDN in the network.                        **
 **                                                                        **
 ** Inputs:	 ueid:		UE lower layer identifier                  **
 **		 pid:		Identifier of the PDN connection to be     **
 **				released                                   **
 **		 Others:	_esm_data                                  **
 **                                                                        **
 ** Outputs:	 esm_cause:	Cause code returned upon ESM procedure     **
 **				failure                                    **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
int esm_proc_pdn_disconnect_accept(unsigned int ueid, int pid, int* esm_cause)
{
    LOG_FUNC_IN;

    LOG_TRACE(INFO, "ESM-PROC  - PDN disconnect accepted by the UE "
	      "(ueid=%d, pid=%d)", ueid, pid);

    /* Release the connectivity with the requested PDN */
    int rc = mme_api_unsubscribe(NULL);
    if (rc != RETURNerror) {
	/* Delete the PDN connection entry */
	int pti = _pdn_connectivity_delete(ueid, pid);
	if (pti != ESM_PT_UNASSIGNED) {
	    LOG_FUNC_RETURN (RETURNok);
	}
    }
    *esm_cause = ESM_CAUSE_PROTOCOL_ERROR;
    LOG_FUNC_RETURN (RETURNerror);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 esm_proc_pdn_disconnect_reject()                          **
 **                                                                        **
 ** Description: Performs PDN disconnect procedure not accepted by the     **
 **		 network.                                                  **
 **                                                                        **
 **              3GPP TS 24.301, section 6.5.2.4                           **
 **		 Upon receipt of the PDN DISCONNECT REQUEST message, if it **
 **		 is not accepted by the network, the MME shall send a PDN  **
 **		 DISCONNECT REJECT message to the UE.                      **
 **                                                                        **
 ** Inputs:	 is_standalone:	Not used - Always TRUE                     **
 **		 ueid:		UE lower layer identifier                  **
 **		 ebi:		Not used                                   **
 **		 msg:		Encoded PDN disconnect reject message to   **
 **				be sent                                    **
 **		 ue_triggered:	Not used                                   **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
int esm_proc_pdn_disconnect_reject(int is_standalone, unsigned int ueid,
				   int ebi, OctetString* msg, int ue_triggered)
{
    LOG_FUNC_IN;

    int rc;
    emm_sap_t emm_sap;

    LOG_TRACE(WARNING, "ESM-PROC  - PDN disconnect not accepted by the network "
	      "(ueid=%d)", ueid);

    /*
     * Notity EMM that ESM PDU has to be forwarded to lower layers
     */
    emm_sap.primitive = EMMESM_UNITDATA_REQ;
    emm_sap.u.emm_esm.ueid = ueid;
    emm_sap.u.emm_esm.u.data.msg.length = msg->length;
    emm_sap.u.emm_esm.u.data.msg.value = msg->value;
    rc = emm_sap_send(&emm_sap);

    LOG_FUNC_RETURN(rc);
}
#endif // NAS_MME

/****************************************************************************/
/*********************  L O C A L    F U N C T I O N S  *********************/
/****************************************************************************/

/*
 * --------------------------------------------------------------------------
 *				Timer handlers
 * --------------------------------------------------------------------------
 */

#ifdef NAS_UE
/****************************************************************************
 **                                                                        **
 ** Name:	 _pdn_disconnect_t3492_handler()                           **
 **                                                                        **
 ** Description: T3492 timeout handler                                     **
 **                                                                        **
 **              3GPP TS 24.301, section 6.5.2.5, case a                   **
 **		 On the first expiry of the timer T3492, the UE shall re-  **
 **		 send the PDN DISCONNECT REQUEST and shall reset and re-   **
 **		 start timer T3492. This retransmission is repeated four   **
 **		 times, i.e. on the fifth expiry of timer T3492, the UE    **
 **		 shall abort the procedure, deactivate all EPS bearer con- **
 **		 texts for this PDN connection locally, release the PTI    **
 **		 allocated for this invocation and enter the state PROCE-  **
 **		 DURE TRANSACTION INACTIVE.                                **
 **                                                                        **
 ** Inputs:	 args:		handler parameters                         **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	None                                       **
 **		 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
static void* _pdn_disconnect_t3492_handler(void* args)
{
    LOG_FUNC_IN;

    int rc;

    /* Get retransmission timer parameters data */
    esm_pt_timer_data_t* data = (esm_pt_timer_data_t*)(args);

    /* Increment the retransmission counter */
    data->count += 1;

    LOG_TRACE(WARNING, "ESM-PROC  - T3492 timer expired (pti=%d), "
	      "retransmission counter = %d", data->pti, data->count);

    if (data->count < ESM_PDN_DISCONNECT_COUNTER_MAX) {
	emm_sap_t emm_sap;
	emm_esm_data_t *emm_esm = &emm_sap.u.emm_esm.u.data;
	/*
	 * Notify EMM that the PDN connectivity request message
	 * has to be sent again
	 */
	emm_sap.primitive = EMMESM_UNITDATA_REQ;
	emm_sap.u.emm_esm.ueid = 0;
	emm_esm->msg.length = data->msg.length;
	emm_esm->msg.value = data->msg.value;
	rc = emm_sap_send(&emm_sap);

	if (rc != RETURNerror) {
	    /* Restart the timer T3492 */
	    rc = esm_pt_start_timer(data->pti, &data->msg, T3492_DEFAULT_VALUE,
				    _pdn_disconnect_t3492_handler);
	}
    }
    else {
	/* Set the procedure transaction state to INACTIVE */
	rc = esm_pt_set_status(data->pti, ESM_PT_INACTIVE);
	if (rc != RETURNok) {
	    /* The procedure transaction was already in INACTIVE state */
	    LOG_TRACE(WARNING, "ESM-PROC  - PTI %d was already INACTIVE",
		      data->pti);
	}
	else {
	    /* Release the transaction identity assigned to this procedure */
	    rc = esm_pt_release(data->pti);
	    if (rc != RETURNok) {
		LOG_TRACE(WARNING, "ESM-PROC  - Failed to release PTI %d",
			  data->pti);
	    }
	    else {
		/* Get the identity of the default EPS bearer context
		 * allocated to the PDN connection entry assigned to
		 * this procedure transaction */
		int ebi = _pdn_disconnect_get_default_ebi(data->pti);
		if (ebi < 0) {
		    LOG_TRACE(ERROR, "ESM-PROC  - No default EPS bearer found");
		    LOG_FUNC_RETURN (NULL);
		}
		/*
		 * Notify ESM that all EPS bearer contexts to this PDN have
		 * to be locally deactivated
		 */
		esm_sap_t esm_sap;
		esm_sap.primitive = ESM_EPS_BEARER_CONTEXT_DEACTIVATE_REQ;
		esm_sap.is_standalone = TRUE;
		esm_sap.recv = NULL;
		esm_sap.send.length = 0;
		esm_sap.data.eps_bearer_context_deactivate.ebi = ebi;
		rc = esm_sap_send(&esm_sap);
	    }
	}
    }

    LOG_FUNC_RETURN(NULL);
}
#endif // NAS_UE

/*
 *---------------------------------------------------------------------------
 *			    PDN disconnection handlers
 *---------------------------------------------------------------------------
 */

#ifdef NAS_UE
/****************************************************************************
 **                                                                        **
 ** Name:	 _pdn_disconnect_get_default_ebi()                         **
 **                                                                        **
 ** Description: Returns the EPS bearer identity of the default EPS bearer **
 **		 context allocated to the PDN connection to which the gi-  **
 **		 ven procedure transaction identity has been assigned      **
 **                                                                        **
 ** Inputs:	 pti:		The procedure transaction identity         **
 **		 Others:	_esm_data                                  **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	The EPS bearer identity of the default EPS **
 **				bearer context, if it exists; -1 otherwise **
 **		 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
static int _pdn_disconnect_get_default_ebi(int pti)
{
    int ebi = -1;

    for (int i = 0; i < ESM_DATA_PDN_MAX; i++) {
	if ( (_esm_data.pdn[i].pid != -1) && _esm_data.pdn[i].data ) {
	    if (_esm_data.pdn[i].data->pti != pti) continue;
	    /* PDN entry found */
	    if (_esm_data.pdn[i].data->bearer[0] != NULL) {
		/* Get the EPS bearer identity of the default EPS bearer
		 * context associated to the PDN connection */
		ebi = _esm_data.pdn[i].data->bearer[0]->ebi;
	    }
	    break;
	}
    }

    return (ebi);
}
#endif // NAS_UE

#ifdef NAS_MME
/****************************************************************************
 **                                                                        **
 ** Name:	 _pdn_disconnect_get_pid()                                 **
 **                                                                        **
 ** Description: Returns the identifier of the PDN connection to which the **
 **		 given procedure transaction identity has been assigned    **
 **		 to establish connectivity to the specified UE             **
 **                                                                        **
 ** Inputs:	 ueid:		UE local identifier                        **
 **		 pti:		The procedure transaction identity         **
 **		 Others:	_esm_data                                  **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	The identifier of the PDN connection if    **
 **				found in the list; -1 otherwise.           **
 **		 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
static int _pdn_disconnect_get_pid(unsigned int ueid, int pti)
{
    int i = ESM_DATA_PDN_MAX;

    if ( (ueid < ESM_DATA_NB_UE_MAX) && (_esm_data.ctx[ueid] != NULL) ) {
	for (i = 0; i < ESM_DATA_PDN_MAX; i++) {
	    if ( (_esm_data.ctx[ueid]->pdn[i].pid != -1) && 
		 (_esm_data.ctx[ueid]->pdn[i].data != NULL) ) {
		if (_esm_data.ctx[ueid]->pdn[i].data->pti != pti) continue;
		/* PDN entry found */
		break;
	    }
	}
    }

    /* Return the identifier of the PDN connection */
    return (_esm_data.ctx[ueid]->pdn[i].pid);
}
#endif // NAS_MME
