/*****************************************************************************
			Eurecom OpenAirInterface 3
			Copyright(c) 2012 Eurecom

Source		Attach.c

Version		0.1

Date		2012/12/04

Product		NAS stack

Subsystem	EPS Mobility Management

Author		Frederic Maurel

Description	Defines the attach related EMM procedure executed by the
		Non-Access Stratum.

		To get internet connectivity from the network, the network
		have to know about the UE. When the UE is switched on, it
		has to initiate the attach procedure to get initial access
		to the network and register its presence to the Evolved
		Packet Core (EPC) network in order to receive EPS services.

		As a result of a successful attach procedure, a context is
		created for the UE in the MME, and a default bearer is esta-
		blished between the UE and the PDN-GW. The UE gets the home
		agent IPv4 and IPv6 addresses and full connectivity to the
		IP network.

		The network may also initiate the activation of additional
		dedicated bearers for the support of a specific service.

*****************************************************************************/

#include "emm_proc.h"
#include "networkDef.h"
#include "nas_log.h"
#include "nas_timer.h"

#include "emmData.h"

#include "emm_sap.h"
#include "esm_sap.h"
#include "emm_cause.h"

#ifdef NAS_MME
#include "mme_api.h"
#endif

#include <string.h>	// memcmp, memcpy
#include <stdlib.h>	// malloc, free

/****************************************************************************/
/****************  E X T E R N A L    D E F I N I T I O N S  ****************/
/****************************************************************************/

/****************************************************************************/
/*******************  L O C A L    D E F I N I T I O N S  *******************/
/****************************************************************************/

/* String representation of the EPS attach type */
static const char* _emm_attach_type_str[] = {
    "EPS", "IMSI", "EMERGENCY", "RESERVED"
};

/*
 * --------------------------------------------------------------------------
 *	    Internal data handled by the attach procedure in the UE
 * --------------------------------------------------------------------------
 */
#ifdef NAS_UE
/*
 * Timer handlers
 */
void* _emm_attach_t3410_handler(void*);
static void* _emm_attach_t3411_handler(void*);
static void* _emm_attach_t3402_handler(void*);

/*
 * Abnormal case attach procedure
 */
static void _emm_attach_abnormal_cases_bcd(emm_sap_t*);

/*
 * Internal data used for attach procedure
 */
static struct {
#define EMM_ATTACH_COUNTER_MAX	5
    unsigned int attempt_count;	/* Counter used to limit the number of
				 * subsequently rejected attach attempts */
} _emm_attach_data = {0};
#endif // NAS_UE

/*
 * --------------------------------------------------------------------------
 *	    Internal data handled by the attach procedure in the MME
 * --------------------------------------------------------------------------
 */
#ifdef NAS_MME
/*
 * Timer handlers
 */
static void* _emm_attach_t3450_handler(void*);

/*
 * Functions that may initiate EMM common procedures
 */
static int _emm_attach_identify(void*);
static int _emm_attach_security(void*);
static int _emm_attach(void*);

/*
 * Abnormal case attach procedures
 */
static int _emm_attach_release(void*);
static int _emm_attach_reject(void*);
static int _emm_attach_abort(void*);

static int _emm_attach_have_changed(const emm_data_context_t* ctx,
			emm_proc_attach_type_t type, int ksi,
			GUTI_t* guti, imsi_t* imsi, imei_t* imei,
			int eea, int eia);
static int _emm_attach_update(emm_data_context_t* ctx, unsigned int ueid,
			emm_proc_attach_type_t type, int ksi,
			GUTI_t* guti, imsi_t* imsi, imei_t* imei,
			int eea, int eia, const OctetString* esm_msg);

/*
 * Internal data used for attach procedure
 */
typedef struct {
    unsigned int ueid;			/* UE identifier		*/
#define ATTACH_COUNTER_MAX	5
    unsigned int retransmission_count;	/* Retransmission counter	*/
    OctetString esm_msg;		/* ESM message to be sent within
					 * the Attach Accept message	*/
} attach_data_t;

static int _emm_attach_accept(emm_data_context_t* emm_ctx, attach_data_t* data);

#endif // NAS_MME

/****************************************************************************/
/******************  E X P O R T E D    F U N C T I O N S  ******************/
/****************************************************************************/

/*
 * --------------------------------------------------------------------------
 * 			Attach procedure executed by the UE
 * --------------------------------------------------------------------------
 */
#ifdef NAS_UE
/****************************************************************************
 **                                                                        **
 ** Name:	 emm_proc_attach()                                         **
 **                                                                        **
 ** Description: Initiate EPS attach procedure to register a UE in PS mode **
 **		 of operation for EPS services only, or register a UE for  **
 **		 emergency bearer services.                                **
 **                                                                        **
 **              3GPP TS 24.301, section 5.5.1.2.2                         **
 **		 In state EMM-DEREGISTERED, the UE initiates the attach    **
 **		 procedure by sending an ATTACH REQUEST message to the MME,**
 **		 starting timer T3410 and entering state EMM-REGISTERED-   **
 **		 INITIATED.                                                **
 **                                                                        **
 ** Inputs:	 type:		Type of the requested attach               **
 **		 Others:	_emm_data                                  **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	T3402, T3410, T3411                        **
 **                                                                        **
 ***************************************************************************/
int emm_proc_attach(emm_proc_attach_type_t type)
{
    LOG_FUNC_IN;

    emm_sap_t emm_sap;
    emm_as_establish_t *emm_as = &emm_sap.u.emm_as.u.establish;
    esm_sap_t esm_sap;
    int rc;

    LOG_TRACE(INFO, "EMM-PROC  - Initiate EPS attach type = %s (%d)",
	      _emm_attach_type_str[type], type);

    /* Update the emergency bearer service indicator */
    if (type == EMM_ATTACH_TYPE_EMERGENCY) {
	_emm_data.is_emergency = TRUE;
    }

    /* Setup initial NAS information message to transfer */
    emm_as->NASinfo = EMM_AS_NAS_INFO_ATTACH;
    /* Set the attach type */
    emm_as->type = type;
    /* Set the RRC connection establishment cause */
    if (_emm_data.is_emergency) {
	emm_as->RRCcause = NET_ESTABLISH_CAUSE_EMERGENCY;
	emm_as->RRCtype = NET_ESTABLISH_TYPE_EMERGENCY_CALLS;
    } else {
	emm_as->RRCcause = NET_ESTABLISH_CAUSE_MO_SIGNAL;
	emm_as->RRCtype = NET_ESTABLISH_TYPE_ORIGINATING_SIGNAL;
    }
    /* Set the PLMN identifier of the selected PLMN */
    emm_as->plmnID = &_emm_data.splmn;
    /*
     * Process the EPS mobile identity
     */
    emm_as->UEid.guti = NULL;
    emm_as->UEid.tai  = NULL;
    emm_as->UEid.imsi = NULL;
    emm_as->UEid.imei = NULL;
    /* Check whether the UE is configured for "AttachWithIMSI" */
    if (_emm_data.AttachWithImsi) {
	/* Check whether the selected PLMN is neither the registered PLMN
	 * nor in the list of equivalent PLMNs */
	if ( (!_emm_data.is_rplmn) && (!_emm_data.is_eplmn) ) {
	    /* Include the IMSI */
	    emm_as->UEid.imsi = _emm_data.imsi;
	}
    }
    else if (_emm_data.guti) {
	/* Include a valid GUTI and the last visited registered TAI */
	emm_as->UEid.guti = _emm_data.guti;
	emm_as->UEid.tai = _emm_data.tai;
    }
    else if (!_emm_data.is_emergency) {
	/* Include the IMSI if no valid GUTI is available */
	emm_as->UEid.imsi = _emm_data.imsi;
    }
    else {
	/* The UE is attaching for emergency bearer services and
	 * does not hold a valid GUTI */
	if (_emm_data.imsi) {
	    /* Include the IMSI if valid (USIM is present) */
	    emm_as->UEid.imsi = _emm_data.imsi;
	} else {
	    /* Include the IMEI if the IMSI is not valid */
	    emm_as->UEid.imei = _emm_data.imei;
	}
    }
    /* Setup EPS NAS security data */
    emm_as_set_security_data(&emm_as->sctx, _emm_data.security, FALSE, FALSE);
    emm_as->ksi = EMM_AS_NO_KEY_AVAILABLE;
    if (_emm_data.security) {
	if (_emm_data.security->type != EMM_KSI_NOT_AVAILABLE) {
	    emm_as->ksi = _emm_data.security->eksi;
	}
	emm_as->encryption = _emm_data.security->capability.encryption;
	emm_as->integrity = _emm_data.security->capability.integrity;
    }
    /*
     * Notify ESM that initiation of a PDN connectivity procedure
     * is requested to setup a default EPS bearer
     */
    esm_sap.primitive = ESM_PDN_CONNECTIVITY_REQ;
    esm_sap.is_standalone = FALSE;
    esm_sap.data.pdn_connect.is_defined = TRUE;
    esm_sap.data.pdn_connect.cid = 1;
    /* TODO: PDN type should be set according to the IP capability of the UE */
    esm_sap.data.pdn_connect.pdn_type = NET_PDN_TYPE_IPV4V6;
    esm_sap.data.pdn_connect.apn = NULL;
    esm_sap.data.pdn_connect.is_emergency = _emm_data.is_emergency;
    rc = esm_sap_send(&esm_sap);

    if (rc != RETURNerror) {
	/* Setup EMM procedure handler to be executed upon receiving
	 * lower layer notification */
	rc = emm_proc_lowerlayer_initialize(emm_proc_attach_request,
					    emm_proc_attach_failure,
					    emm_proc_attach_release, NULL);
	if (rc != RETURNok) {
	    LOG_TRACE(WARNING, "Failed to initialize EMM procedure handler");
	    LOG_FUNC_RETURN (RETURNerror);
	}

	/* Start T3410 timer */
	T3410.id = nas_timer_start(T3410.sec, _emm_attach_t3410_handler, NULL);
	LOG_TRACE(INFO,"EMM-PROC  - Timer T3410 (%d) expires in %ld seconds",
		  T3410.id, T3410.sec);
	/* Stop T3402 and T3411 timers if running */
	T3402.id = nas_timer_stop(T3402.id);
	T3411.id = nas_timer_stop(T3411.id);

	/*
	 * Notify EMM-AS SAP that a RRC connection establishment procedure
	 * is requested from the Access-Stratum to send initial NAS message
	 * attach request to the network
	 */
	emm_sap.primitive = EMMAS_ESTABLISH_REQ;
	/* Get the PDN connectivity request to transfer within the ESM
	 * container of the initial attach request message */
	emm_sap.u.emm_as.u.establish.NASmsg = esm_sap.send;
	rc = emm_sap_send(&emm_sap);
    }

    LOG_FUNC_RETURN(rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 emm_proc_attach_request()                                 **
 **                                                                        **
 ** Description: Performs the attach procedure upon receipt of indication  **
 **		 from lower layers that Attach Request message has been    **
 **		 successfully delivered to the network.                    **
 **                                                                        **
 ** Inputs:	 args:		Not used                                   **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_proc_attach_request(void* args)
{
    LOG_FUNC_IN;

    emm_sap_t emm_sap;
    int rc;

    /*
     * Notify EMM that Attach Request has been sent to the network
     */
    emm_sap.primitive = EMMREG_ATTACH_REQ;
    rc = emm_sap_send(&emm_sap);

    LOG_FUNC_RETURN(rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 emm_proc_attach_accept()                                  **
 **                                                                        **
 ** Description: Performs the attach procedure accepted by the network.    **
 **                                                                        **
 **              3GPP TS 24.301, section 5.5.1.2.4                         **
 **		 Upon receiving the ATTACH ACCEPT message, the UE shall    **
 **		 stop timer T3410 and send an ATTACH COMPLETE message to   **
 **		 the MME.                                                  **
 **                                                                        **
 ** Inputs:	 t3412:		Value of the T3412 timer in seconds        **
 **		 t3402:		Value of the T3402 timer in seconds        **
 **		 t3423:		Value of the T3423 timer in seconds        **
 **		 n_tais:	Number of tracking area identities contai- **
 **				ned in the TAI list                        **
 **		 tai:		The TAI list that identifies the tracking  **
 **				areas the UE is registered to              **
 **		 guti:		New UE's temporary identity assigned by    **
 **				the MME (GUTI reallocation)                **
 **		 n_eplmns:	Number of equivalent PLMNs                 **
 **		 eplmns:	List of equivalent PLMNs                   **
 **		 esm_msg:	Activate default EPS bearer context re-    **
 **				quest ESM message                          **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	_emm_data, T3412, T3402, T3423             **
 **                                                                        **
 ***************************************************************************/
int emm_proc_attach_accept(long t3412, long t3402, long t3423,
			   int n_tais, tai_t* tai, GUTI_t* guti,
			   int n_eplmns, plmn_t* eplmn,
			   const OctetString* esm_msg)
{
    LOG_FUNC_IN;

    emm_sap_t emm_sap;
    esm_sap_t esm_sap;
    int rc;

    LOG_TRACE(INFO, "EMM-PROC  - EPS attach accepted by the network");

    /* Stop timer T3410 */
    LOG_TRACE(INFO, "EMM-PROC  - Stop timer T3410 (%d)", T3410.id);
    T3410.id = nas_timer_stop(T3410.id);

    /* Delete old TAI list and store the received TAI list */
    _emm_data.ltai.n_tais = n_tais;
    for (int i = 0; (i < n_tais) && (i < EMM_DATA_TAI_MAX); i++) {
	_emm_data.ltai.tai[i] = tai[i];
    }

    /* Update periodic tracking area update timer value */
    T3412.sec = t3412;
    /* Update attach failure timer value */
    if ( !(t3402 < 0) ) {
	T3402.sec = t3402;
    }
    /* Update E-UTRAN deactivate ISR timer value */
    if ( !(t3423 < 0) ) {
	T3423.sec = t3423;
    }

    /* Delete old GUTI and store the new assigned GUTI if provided */
    if (guti) {
	*_emm_data.guti = *guti;
    }

    /* Update the stored list of equivalent PLMNs */
    _emm_data.nvdata.eplmn.n_plmns = 0;
    if (n_eplmns > 0) {
	for (int i = 0; (i < n_eplmns) && (i < EMM_DATA_EPLMN_MAX); i++) {
	    int is_forbidden = FALSE;
	    if (!_emm_data.is_emergency) {
		/* If the attach procedure is not for emergency bearer
		 * services, the UE shall remove from the list any PLMN
		 * code that is already in the list of forbidden PLMNs */
		for (int j = 0; j < _emm_data.fplmn.n_plmns; j++) {
		    if (PLMNS_ARE_EQUAL(eplmn[i], _emm_data.fplmn.plmn[j])) {
			is_forbidden = TRUE;
			break;
		    }
		}
	    }
	    if ( !is_forbidden ) {
		_emm_data.nvdata.eplmn.plmn[_emm_data.nvdata.eplmn.n_plmns++] =
		    eplmn[i];
	    }
	}
	/* Add the PLMN code of the registered PLMN that sent the list */
	if (_emm_data.nvdata.eplmn.n_plmns < EMM_DATA_EPLMN_MAX) {
	    _emm_data.nvdata.eplmn.plmn[_emm_data.nvdata.eplmn.n_plmns++] =
		_emm_data.splmn;
	}
    }

    /*
     * Notify ESM that a default EPS bearer has to be activated
     */
    esm_sap.primitive = ESM_DEFAULT_EPS_BEARER_CONTEXT_ACTIVATE_REQ;
    esm_sap.is_standalone = FALSE;
    esm_sap.recv = esm_msg;
    rc = esm_sap_send(&esm_sap);

    if ( (rc != RETURNerror) && (esm_sap.err == ESM_SAP_SUCCESS) )
    {
	/* Setup EMM procedure handler to be executed upon receiving
	 * lower layer notification */
	rc = emm_proc_lowerlayer_initialize(emm_proc_attach_complete,
					    emm_proc_attach_failure,
					    NULL, NULL);
	if (rc != RETURNok) {
	    LOG_TRACE(WARNING,
		      "EMM-PROC  - Failed to initialize EMM procedure handler");
	    LOG_FUNC_RETURN (RETURNerror);
	}
	/*
	 * Notify EMM-AS SAP that Attach Complete message together with
	 * an Activate Default EPS Bearer Context Accept message has to
	 * be sent to the network
	 */
	emm_sap.primitive = EMMAS_DATA_REQ;
	emm_sap.u.emm_as.u.data.guti = _emm_data.guti;
	emm_sap.u.emm_as.u.data.ueid = 0;
	/* Setup EPS NAS security data */
	emm_as_set_security_data(&emm_sap.u.emm_as.u.data.sctx,
				 _emm_data.security, FALSE, TRUE);
	/* Get the activate default EPS bearer context accept message
	 * to be transfered within the ESM container of the attach
	 * complete message */
	emm_sap.u.emm_as.u.data.NASinfo = EMM_AS_NAS_DATA_ATTACH;
	emm_sap.u.emm_as.u.data.NASmsg = esm_sap.send;
	rc = emm_sap_send(&emm_sap);
    }
    else if (esm_sap.err != ESM_SAP_DISCARDED) {
	/* 3GPP TS 24.301, section 5.5.1.2.6, case j
	 * If the ACTIVATE DEFAULT BEARER CONTEXT REQUEST message combined
	 * with the ATTACH ACCEPT is not accepted by the UE due to failure
	 * in the UE ESM sublayer, then the UE shall initiate the detach
	 * procedure by sending a DETACH REQUEST message to the network.
	 */
	emm_sap.primitive = EMMREG_DETACH_INIT;
	rc = emm_sap_send(&emm_sap);
    }
    else {
	/*
	 * ESM procedure failed and, received message has been discarded or
	 * Status message has been returned; ignore ESM procedure failure
	 */
	rc = RETURNok;
    }

    LOG_FUNC_RETURN(rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 emm_proc_attach_reject()                                  **
 **                                                                        **
 ** Description: Performs the attach procedure rejected by the network.    **
 **                                                                        **
 **              3GPP TS 24.301, section 5.5.1.2.5                         **
 **		 Upon receiving the ATTACH REJECT message, the UE shall    **
 **		 stop timer T3410 and take actions depending on the EMM    **
 **		 cause value received.                                     **
 **                                                                        **
 ** Inputs:	 emm_cause:	EMM cause indicating why the network re-   **
 **				jected the attach request                  **
 **		 esm_msg:	PDN connectivity reject ESM message        **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	_emm_data, _emm_attach_data, T3410         **
 **                                                                        **
 ***************************************************************************/
int emm_proc_attach_reject(int emm_cause, const OctetString* esm_msg)
{
    LOG_FUNC_IN;

    emm_sap_t emm_sap;
    int rc;

    LOG_TRACE(WARNING, "EMM-PROC  - EPS attach rejected by the network, "
	      "EMM cause = %d", emm_cause);

    /* Stop timer T3410 */
    LOG_TRACE(INFO, "EMM-PROC  - Stop timer T3410 (%d)", T3410.id);
    T3410.id = nas_timer_stop(T3410.id);

    /* Update the EPS update status, the GUTI, the visited registered TAI and
     * the eKSI */
    switch (emm_cause)
    {
	case EMM_CAUSE_ILLEGAL_UE:
	case EMM_CAUSE_ILLEGAL_ME:
	case EMM_CAUSE_EPS_NOT_ALLOWED:
	case EMM_CAUSE_BOTH_NOT_ALLOWED:
	case EMM_CAUSE_PLMN_NOT_ALLOWED:
	case EMM_CAUSE_NOT_AUTHORIZED_IN_PLMN:
	case EMM_CAUSE_EPS_NOT_ALLOWED_IN_PLMN:
	case EMM_CAUSE_TA_NOT_ALLOWED:
	case EMM_CAUSE_ROAMING_NOT_ALLOWED:
	case EMM_CAUSE_NO_SUITABLE_CELLS:
	    /* Set the EPS update status to EU3 ROAMING NOT ALLOWED */
	    _emm_data.status = EU3_ROAMING_NOT_ALLOWED;
	    /* Delete the GUTI */
	    _emm_data.guti = NULL;
	    /* Delete the last visited registered TAI */
	    _emm_data.tai = NULL;
	    /* Delete the eKSI */
	    if (_emm_data.security) {
		_emm_data.security->type = EMM_KSI_NOT_AVAILABLE;
	    }
	    break;

	default :
	    break;
    }

    /* Update list of equivalents PLMNs and attach attempt counter */
    switch (emm_cause)
    {
	case EMM_CAUSE_ILLEGAL_UE:
	case EMM_CAUSE_ILLEGAL_ME:
	case EMM_CAUSE_EPS_NOT_ALLOWED:
	case EMM_CAUSE_BOTH_NOT_ALLOWED:
	    /* Consider the USIM as invalid for EPS services */
	    _emm_data.usim_is_valid = FALSE;
	    /* Delete the list of equivalent PLMNs */
	    _emm_data.nvdata.eplmn.n_plmns = 0;
	    break;

	case EMM_CAUSE_PLMN_NOT_ALLOWED:
	case EMM_CAUSE_NOT_AUTHORIZED_IN_PLMN:
	case EMM_CAUSE_ROAMING_NOT_ALLOWED:
	    /* Delete the list of equivalent PLMNs */
	    _emm_data.nvdata.eplmn.n_plmns = 0;
	    /* Reset the attach attempt counter */
	    _emm_attach_data.attempt_count = 0;
	    break;

	case EMM_CAUSE_TA_NOT_ALLOWED:
	case EMM_CAUSE_EPS_NOT_ALLOWED_IN_PLMN:
	case EMM_CAUSE_NO_SUITABLE_CELLS:
	    /* Reset the attach attempt counter */
	    _emm_attach_data.attempt_count = 0;
	    break;

	case EMM_CAUSE_ESM_FAILURE:
	    /* 3GPP TS 24.301, section 5.5.1.2.6, case d */
	    if (_emm_data.NAS_SignallingPriority != 1) {
		/* The UE is not configured for NAS signalling low priority;
		 * set the attach attempt counter to 5 */
		_emm_attach_data.attempt_count = EMM_ATTACH_COUNTER_MAX;
	    }
	    break;

	case EMM_CAUSE_SEMANTICALLY_INCORRECT:
	case EMM_CAUSE_INVALID_MANDATORY_INFO:
	case EMM_CAUSE_MESSAGE_TYPE_NOT_IMPLEMENTED:
	case EMM_CAUSE_IE_NOT_IMPLEMENTED:
	case EMM_CAUSE_PROTOCOL_ERROR:
	    /* 3GPP TS 24.301, section 5.5.1.2.6, case d
	     * Set the attach attempt counter to 5 */
	    _emm_attach_data.attempt_count = EMM_ATTACH_COUNTER_MAX;
	    break;

	default :
	    break;
    }

    /* Update "forbidden lists" */
    switch (emm_cause)
    {
	case EMM_CAUSE_PLMN_NOT_ALLOWED:
	case EMM_CAUSE_NOT_AUTHORIZED_IN_PLMN:
	    /* Store the PLMN identity in the "forbidden PLMN list" */
	    _emm_data.fplmn.plmn[_emm_data.fplmn.n_plmns++] = _emm_data.splmn;
	    break;

	case EMM_CAUSE_TA_NOT_ALLOWED:
	    /* Store the current TAI in the list of "forbidden tracking
	     * areas for regional provision of service" */
	    _emm_data.ftai.tai[_emm_data.ftai.n_tais++] = *_emm_data.tai;
	    break;

	case EMM_CAUSE_ROAMING_NOT_ALLOWED:
	    /* Store the current TAI in the list of "forbidden tracking
	     * areas for roaming" */
	    _emm_data.ftai_roaming.tai[_emm_data.ftai_roaming.n_tais++] = *_emm_data.tai;
	    break;

	case EMM_CAUSE_EPS_NOT_ALLOWED_IN_PLMN:
	    /* Store the PLMN identity in the "forbidden PLMNs for GPRS
	     * service" list */
	    _emm_data.fplmn_gprs.plmn[_emm_data.fplmn_gprs.n_plmns++] = _emm_data.splmn;
	    break;

	default :
	    break;
    }

    /* Update state of EMM sublayer */
    switch (emm_cause)
    {
	case EMM_CAUSE_ILLEGAL_UE:
	case EMM_CAUSE_ILLEGAL_ME:
	case EMM_CAUSE_EPS_NOT_ALLOWED:
	case EMM_CAUSE_BOTH_NOT_ALLOWED:
	    /*
	     * Notify EMM that EPS attach is rejected
	     */
	    emm_sap.primitive = EMMREG_ATTACH_REJ;
	    break;

	case EMM_CAUSE_PLMN_NOT_ALLOWED:
	case EMM_CAUSE_NOT_AUTHORIZED_IN_PLMN:
	case EMM_CAUSE_EPS_NOT_ALLOWED_IN_PLMN:
	    /*
	     * Notify EMM that the UE has to perform a PLMN selection because
	     * it is not allowed to operate in the currently selected PLMN
	     */
	    emm_sap.primitive = EMMREG_REGISTER_REQ;
	    break;

	case EMM_CAUSE_TA_NOT_ALLOWED:
	case EMM_CAUSE_ROAMING_NOT_ALLOWED:
	case EMM_CAUSE_NO_SUITABLE_CELLS:
	    /*
	     * Notify EMM that the UE failed to register to the network for
	     * EPS services because it is not allowed to operate in the
	     * requested tracking area
	     */
	    emm_sap.primitive = EMMREG_REGISTER_REJ;
	    break;

	case EMM_CAUSE_IMEI_NOT_ACCEPTED:
	    if (_emm_data.is_emergency) {
		/*
		 * Notify EMM that the UE failed to register to the network
		 * for emergency bearer services because "IMEI not accepted"
		 */
		emm_sap.primitive = EMMREG_NO_IMSI;
		break;
	    }
	    /* break is volontary missing */

	default :
	    /* Other values are considered as abnormal cases
	     * 3GPP TS 24.301, section 5.5.1.2.6, case d */
	    _emm_attach_abnormal_cases_bcd(&emm_sap);
	    break;
    }

    rc = emm_sap_send(&emm_sap);

    /*
     * Notify ESM that the network rejected connectivity to the PDN
     */
    if (esm_msg != NULL) {
	esm_sap_t esm_sap;
	esm_sap.primitive = ESM_PDN_CONNECTIVITY_REJ;
	esm_sap.is_standalone = FALSE;
	esm_sap.recv = esm_msg;
	rc = esm_sap_send(&esm_sap);
    }

    LOG_FUNC_RETURN(rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 emm_proc_attach_complete()                                **
 **                                                                        **
 ** Description: Terminates the attach procedure when Attach Complete mes- **
 **		 sage has been successfully delivered to the MME.          **
 **                                                                        **
 **              3GPP TS 24.301, section 5.5.1.2.4                         **
 **		 Upon successfully sending the ATTACH COMPLETE message,    **
 **		 the UE shall reset the attach attempt counter and tra-    **
 **		 cking area updating attempt counter, enter state EMM-     **
 **		 REGISTERED and set the EPS update status to EU1-UPDATED.  **
 **                                                                        **
 ** Inputs:	 args:		Not used                                   **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	_emm_data, _emm_attach_data                **
 **                                                                        **
 ***************************************************************************/
int emm_proc_attach_complete(void* args)
{
    LOG_FUNC_IN;

    emm_sap_t emm_sap;
    esm_sap_t esm_sap;
    int rc;

    LOG_TRACE(INFO, "EMM-PROC  - EPS attach complete");

    /* Reset EMM procedure handler */
    (void) emm_proc_lowerlayer_initialize(NULL, NULL, NULL, NULL);

    /* Reset the attach attempt counter */
    _emm_attach_data.attempt_count = 0;
    /* TODO: Reset the tracking area updating attempt counter */

    /* Set the EPS update status to EU1 UPDATED */
    _emm_data.status = EU1_UPDATED;
    _emm_data.is_attached = TRUE;

    /*
     * Notify EMM that network attach complete message has been delivered
     * to the network
     */
    emm_sap.primitive = EMMREG_ATTACH_CNF;
    rc = emm_sap_send(&emm_sap);

    if (rc != RETURNerror) {
	/*
	 * Notify ESM that the Activate Default EPS Bearer Context Accept
	 * message has been delivered to the network within the Attach
	 * Complete message
	 */
	esm_sap.primitive = ESM_DEFAULT_EPS_BEARER_CONTEXT_ACTIVATE_CNF;
	esm_sap.is_standalone = FALSE;
	rc = esm_sap_send(&esm_sap);
    }

    LOG_FUNC_RETURN(rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 emm_proc_attach_failure()                                 **
 **                                                                        **
 ** Description: Performs the attach procedure abnormal case upon receipt  **
 **		 of transmission failure of Attach Request message or At-  **
 **		 tach Complete message.                                    **
 **                                                                        **
 **              3GPP TS 24.301, section 5.5.1.2.6, cases h and i          **
 **		 The  UE  shall restart the attach  procedure when timer   **
 **		 T3411 expires.                                            **
 **                                                                        **
 ** Inputs:	 is_initial:	TRUE if the NAS message that failed to be  **
 **				transfered is an initial NAS message (ESM  **
 **				message embedded within an Attach Request  **
 **				message)                                   **
 ** 		 args:		Not used                                   **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	T3410, T3411                               **
 **                                                                        **
 ***************************************************************************/
int emm_proc_attach_failure(int is_initial, void* args)
{
    LOG_FUNC_IN;

    int rc = RETURNok;
    esm_sap_t esm_sap;

    LOG_TRACE(WARNING, "EMM-PROC  - EPS attach failure");

    /* Reset EMM procedure handler */
    (void) emm_proc_lowerlayer_initialize(NULL, NULL, NULL, NULL);

    /* Stop timer T3410 if still running */
    if (T3410.id != NAS_TIMER_INACTIVE_ID) {
	LOG_TRACE(INFO, "EMM-PROC  - Stop timer T3410 (%d)", T3410.id);
	T3410.id = nas_timer_stop(T3410.id);
    }

    if (is_initial) {
	/*
	 * Notify ESM that the PDN CONNECTIVITY REQUEST message contained
	 * in the ESM message container IE of the ATTACH REQUEST has failed
	 * to be transmitted
	 */
	esm_sap.primitive = ESM_PDN_CONNECTIVITY_REJ;
	esm_sap.is_standalone = FALSE;
	esm_sap.recv = NULL;
    }
    else {
	/*
	 * Notify ESM that ACTIVATE DEFAULT EPS BEARER CONTEXT REQUEST message
	 * contained in the ESM message container IE of the ATTACH COMPLETE
	 * has failed to be transmitted
	 */
	esm_sap.primitive = ESM_DEFAULT_EPS_BEARER_CONTEXT_ACTIVATE_REJ;
	esm_sap.is_standalone = FALSE;
	esm_sap.recv = NULL;
    }
    rc = esm_sap_send(&esm_sap);

    if (rc != RETURNerror) {
	/* Start T3411 timer */
	T3411.id = nas_timer_start(T3411.sec, _emm_attach_t3411_handler, NULL);
	LOG_TRACE(INFO, "EMM-PROC  - Timer T3411 (%d) expires in %ld seconds",
		  T3411.id, T3411.sec);
    }

    LOG_FUNC_RETURN(rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 emm_proc_attach_release()                                 **
 **                                                                        **
 ** Description: Performs the attach procedure abnormal case upon receipt  **
 **		 of NAS signalling connection release indication.          **
 **                                                                        **
 **              3GPP TS 24.301, section 5.5.1.2.6, case b                 **
 **		 The attach procedure shall be aborted and the UE shall    **
 **		 execute abnormal case attach procedure.                   **
 **                                                                        **
 ** Inputs:	 args:		Not used                                   **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_proc_attach_release(void* args)
{
    LOG_FUNC_IN;

    emm_sap_t emm_sap;
    int rc;

    LOG_TRACE(WARNING, "EMM-PROC  - NAS signalling connection released");

    /* Execute abnormal case attach procedure */
    _emm_attach_abnormal_cases_bcd(&emm_sap);

    rc = emm_sap_send(&emm_sap);

    LOG_FUNC_RETURN(rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 emm_proc_attach_restart()                                 **
 **                                                                        **
 ** Description: Restarts the attach procedure                             **
 **                                                                        **
 ** Inputs:	 None                                                      **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_proc_attach_restart(void)
{
    LOG_FUNC_IN;

    emm_sap_t emm_sap;
    int rc;

    LOG_TRACE(INFO, "EMM-PROC  - Restart EPS attach procedure");

    /*
     * Notify EMM that the attach procedure has to be restarted
     */
    emm_sap.primitive = EMMREG_ATTACH_INIT;
    emm_sap.u.emm_reg.u.attach.is_emergency = _emm_data.is_emergency;
    rc = emm_sap_send(&emm_sap);

    LOG_FUNC_RETURN(rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 emm_proc_attach_set_emergency()                           **
 **                                                                        **
 ** Description: Set the emergency bearer services indicator               **
 **                                                                        **
 ** Inputs:	 None                                                      **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	_emm_data                                  **
 **                                                                        **
 ***************************************************************************/
int emm_proc_attach_set_emergency(void)
{
    LOG_FUNC_IN;

    LOG_TRACE(WARNING, "EMM-PROC  - UE is now attached to the network for "
	      "emergency bearer services only");

    _emm_data.is_emergency = TRUE;

    LOG_FUNC_RETURN(RETURNok);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 emm_proc_attach_set_detach()                              **
 **                                                                        **
 ** Description: Reset the network attachment indicator and enter state    **
 **		 EMM-DEREGISTERED
 **                                                                        **
 ** Inputs:	 None                                                      **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	_emm_data                                  **
 **                                                                        **
 ***************************************************************************/
int emm_proc_attach_set_detach(void)
{
    LOG_FUNC_IN;

    int rc;

    LOG_TRACE(WARNING,
	      "EMM-PROC  - UE is now locally detached from the network");

    /* Reset the network attachment indicator */
    _emm_data.is_attached = FALSE;
    /*
     * Notify that the UE is locally detached from the network
     */
    emm_sap_t emm_sap;
    emm_sap.primitive = EMMREG_DETACH_CNF;
    rc = emm_sap_send(&emm_sap);

    LOG_FUNC_RETURN(rc);
}

#endif

/*
 * --------------------------------------------------------------------------
 * 			Attach procedure executed by the MME
 * --------------------------------------------------------------------------
 */
#ifdef NAS_MME
/****************************************************************************
 **                                                                        **
 ** Name:	 emm_proc_attach_request()                                 **
 **                                                                        **
 ** Description: Performs the UE requested attach procedure                **
 **                                                                        **
 **              3GPP TS 24.301, section 5.5.1.2.3                         **
 **		 The network may initiate EMM common procedures, e.g. the  **
 **		 identification, authentication and security mode control  **
 **		 procedures during the attach procedure, depending on the  **
 **		 information received in the ATTACH REQUEST message (e.g.  **
 **		 IMSI, GUTI and KSI).                                      **
 **                                                                        **
 ** Inputs:	 ueid:		UE lower layer identifier                  **
 **		 type:		Type of the requested attach               **
 **		 native_ksi:	TRUE if the security context is of type    **
 **				native (for KSIASME)                       **
 **		 ksi:		The NAS ket sey identifier                 **
 **		 native_guti:	TRUE if the provided GUTI is native GUTI   **
 **		 guti:		The GUTI if provided by the UE             **
 **		 imsi:		The IMSI if provided by the UE             **
 **		 imei:		The IMEI if provided by the UE             **
 **		 tai:		Identifies the last visited tracking area  **
 **				the UE is registered to                    **
 **		 eea:		Supported EPS encryption algorithms        **
 **		 eia:		Supported EPS integrity algorithms         **
 **		 esm_msg:	PDN connectivity request ESM message       **
 **		 Others:	_emm_data                                  **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	_emm_data                                  **
 **                                                                        **
 ***************************************************************************/
int emm_proc_attach_request(unsigned int ueid, emm_proc_attach_type_t type,
			    int native_ksi, int ksi, int native_guti,
			    GUTI_t* guti, imsi_t* imsi, imei_t* imei,
			    tai_t* tai, int eea, int eia,
			    const OctetString* esm_msg)
{
    LOG_FUNC_IN;

    int rc;
    emm_data_context_t ue_ctx;

    LOG_TRACE(INFO, "EMM-PROC  - EPS attach type = %s (%d) requested (ueid=0x%08x)",
              _emm_attach_type_str[type], type, ueid);

    /* Initialize the temporary UE context */
    memset(&ue_ctx, 0 , sizeof(emm_data_context_t));
    ue_ctx.is_dynamic = FALSE;
    ue_ctx.ueid = ueid;

#if !defined(EPC_BUILD)
    /* UE identifier sanity check */
    if (ueid >= EMM_DATA_NB_UE_MAX) {
        ue_ctx.emm_cause = EMM_CAUSE_ILLEGAL_UE;
        /* Do not accept UE with invalid identifier */
        rc = _emm_attach_reject(&ue_ctx);
        LOG_FUNC_RETURN(rc);
    }
#endif

    /* 3GPP TS 24.301, section 5.5.1.1
     * MME not configured to support attach for emergency bearer services
     * shall reject any request to attach with an attach type set to "EPS
     * emergency attach".
     */
    if ( !(_emm_data.conf.features & MME_API_EMERGENCY_ATTACH) &&
         (type == EMM_ATTACH_TYPE_EMERGENCY) )
    {
        ue_ctx.emm_cause = EMM_CAUSE_IMEI_NOT_ACCEPTED;
        /* Do not accept the UE to attach for emergency services */
        rc = _emm_attach_reject(&ue_ctx);
        LOG_FUNC_RETURN(rc);
    }

    /* Get the UE's EMM context if it exists */
    emm_data_context_t** emm_ctx = NULL;

#if defined(EPC_BUILD)
    emm_data_context_t* temp = NULL;

    temp    = emm_data_context_get(&_emm_data, ueid);
    emm_ctx = &temp;
#else
    emm_ctx = &_emm_data.ctx[ueid];
#endif

    if (*emm_ctx != NULL) {
	/* An EMM context already exists for the UE in the network */
	if (_emm_attach_have_changed(*emm_ctx, type, ksi, guti, imsi, imei,
				     eea, eia))
	{
	    /*
	     * 3GPP TS 24.301, section 5.5.1.2.7, abnormal case e
	     * The attach parameters have changed from the one received within
	     * the previous Attach Request message;
	     * the previously initiated attach procedure shall be aborted and
	     * the new attach procedure shall be executed;
	     */
	    LOG_TRACE(WARNING, "EMM-PROC  - Attach parameters have changed");
	    /*
	     * Notify EMM that the attach procedure is aborted
	     */
	    emm_sap_t emm_sap;
	    emm_sap.primitive = EMMREG_PROC_ABORT;
	    emm_sap.u.emm_reg.ueid = ueid;
            emm_sap.u.emm_reg.ctx  = *emm_ctx;

	    rc = emm_sap_send(&emm_sap);

	    if (rc != RETURNerror) {
		/* Process new attach procedure */
		LOG_TRACE(WARNING, "EMM-PROC  - Initiate new attach procedure");
		rc = emm_proc_attach_request(ueid, type, native_ksi, ksi,
					     native_guti, guti, imsi, imei,
					     tai, eea, eia, esm_msg);
	    }
	    LOG_FUNC_RETURN(rc);
	}
	else {
	    /* Continue with the previous attach procedure */
	    LOG_TRACE(WARNING, "EMM-PROC  - Received duplicated Attach Request");
	    LOG_FUNC_RETURN(RETURNok);
	}
    }
    else {
	/* Create UE's EMM context */
	*emm_ctx = (emm_data_context_t*)malloc(sizeof(emm_data_context_t));
	if (emm_ctx == NULL) {
	    LOG_TRACE(WARNING, "EMM-PROC  - Failed to create EMM context");
	    ue_ctx.emm_cause = EMM_CAUSE_ILLEGAL_UE;
	    /* Do not accept the UE to attach to the network */
	    rc = _emm_attach_reject(&ue_ctx);
	    LOG_FUNC_RETURN(rc);
	}
	(*emm_ctx)->is_dynamic = TRUE;
	(*emm_ctx)->guti = NULL;
	(*emm_ctx)->old_guti = NULL;
	(*emm_ctx)->imsi = NULL;
	(*emm_ctx)->imei = NULL;
	(*emm_ctx)->security = NULL;
	(*emm_ctx)->esm_msg.length = 0;
	(*emm_ctx)->esm_msg.value = NULL;
	(*emm_ctx)->emm_cause = EMM_CAUSE_SUCCESS;

        (*emm_ctx)->_emm_fsm_status = EMM_DEREGISTERED;

#if defined(EPC_BUILD)
        emm_data_context_add(&_emm_data, *(emm_ctx));
#endif
    }

    /* Update the EMM context with the current attach procedure parameters */
    rc = _emm_attach_update(*emm_ctx, ueid, type, ksi, guti, imsi, imei,
			    eea, eia, esm_msg);
    if (rc != RETURNok) {
	LOG_TRACE(WARNING, "EMM-PROC  - Failed to update EMM context");
	/* Do not accept the UE to attach to the network */
	(*emm_ctx)->ueid = ueid;
	(*emm_ctx)->emm_cause = EMM_CAUSE_ILLEGAL_UE;
	rc = _emm_attach_reject(*emm_ctx);
    }
    else {
	/*
	 * Performs UE identification
	 */
	rc = _emm_attach_identify(*emm_ctx);
    }

    LOG_FUNC_RETURN(rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 emm_proc_attach_reject()                                  **
 **                                                                        **
 ** Description: Performs the protocol error abnormal case                 **
 **                                                                        **
 **              3GPP TS 24.301, section 5.5.1.2.7, case b                 **
 **		 If the ATTACH REQUEST message is received with a protocol **
 **		 error, the network shall return an ATTACH REJECT message. **
 **                                                                        **
 ** Inputs:	 ueid:		UE lower layer identifier                  **
 ** 		 emm_cause:	EMM cause code to be reported              **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	_emm_data                                  **
 **                                                                        **
 ***************************************************************************/
int emm_proc_attach_reject(unsigned int ueid, int emm_cause)
{
    LOG_FUNC_IN;

    int rc;

    /* Create temporary UE context */
    emm_data_context_t ue_ctx;
    memset(&ue_ctx, 0 , sizeof(emm_data_context_t));
    ue_ctx.is_dynamic = FALSE;
    ue_ctx.ueid = ueid;

    /* Update the EMM cause code */
#if defined(EPC_BUILD)
    if (ueid == 0)
#else
    if (ueid < EMM_DATA_NB_UE_MAX)
#endif
    {
        ue_ctx.emm_cause = emm_cause;
    } else {
        ue_ctx.emm_cause = EMM_CAUSE_ILLEGAL_UE;
    }

    /* Do not accept attach request with protocol error */
    rc = _emm_attach_reject(&ue_ctx);

    LOG_FUNC_RETURN(rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 emm_proc_attach_complete()                                **
 **                                                                        **
 ** Description: Terminates the attach procedure upon receiving Attach     **
 **		 Complete message from the UE.                             **
 **                                                                        **
 **              3GPP TS 24.301, section 5.5.1.2.4                         **
 **		 Upon receiving an ATTACH COMPLETE message, the MME shall  **
 **		 stop timer T3450, enter state EMM-REGISTERED and consider **
 **		 the GUTI sent in the ATTACH ACCEPT message as valid.      **
 **                                                                        **
 ** Inputs:	 ueid:		UE lower layer identifier                  **
 **		 esm_msg:	Activate default EPS bearer context accept **
 **				ESM message                                **
 **		 Others:	_emm_data                                  **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	_emm_data, T3450                           **
 **                                                                        **
 ***************************************************************************/
int emm_proc_attach_complete(unsigned int ueid, const OctetString* esm_msg)
{
    LOG_FUNC_IN;

    int rc = RETURNerror;
    emm_sap_t emm_sap;
    esm_sap_t esm_sap;

    LOG_TRACE(INFO, "EMM-PROC  - EPS attach complete (ueid=%u)", ueid);

    /* Stop timer T3450 */
    LOG_TRACE(INFO, "EMM-PROC  - Stop timer T3450 (%d)", T3450.id);
    T3450.id = nas_timer_stop(T3450.id);

    /* Release retransmission timer parameters */
    attach_data_t* data = (attach_data_t*)(emm_proc_common_get_args(ueid));
    if (data) {
	if (data->esm_msg.length > 0) {
	    free(data->esm_msg.value);
	}
	free(data);
    }

    /* Get the UE context */
    emm_data_context_t* emm_ctx = NULL;

#if defined(EPC_BUILD)
    if (ueid > 0) {
        emm_ctx = emm_data_context_get(&_emm_data, ueid);
    }
#else
    if (ueid < EMM_DATA_NB_UE_MAX) {
	emm_ctx = _emm_data.ctx[ueid];
    }
#endif

    if (emm_ctx) {
	/* Delete the old GUTI and consider the GUTI sent in the Attach
	 * Accept message as valid */
	emm_ctx->guti_is_new = FALSE;
	emm_ctx->old_guti = NULL;
	/*
	 * Forward the Activate Default EPS Bearer Context Accept message
	 * to the EPS session management sublayer
	 */
	esm_sap.primitive = ESM_DEFAULT_EPS_BEARER_CONTEXT_ACTIVATE_CNF;
	esm_sap.is_standalone = FALSE;
	esm_sap.ueid = ueid;
	esm_sap.recv = esm_msg;
	rc = esm_sap_send(&esm_sap);
    }
    else {
	LOG_TRACE(ERROR, "EMM-PROC  - No EMM context exists");
    }

    if ( (rc != RETURNerror) && (esm_sap.err == ESM_SAP_SUCCESS) ) {
	/* Set the network attachment indicator */
	emm_ctx->is_attached = TRUE;
	/*
	 * Notify EMM that attach procedure has successfully completed
	 */
	emm_sap.primitive = EMMREG_ATTACH_CNF;
	emm_sap.u.emm_reg.ueid = ueid;
	rc = emm_sap_send(&emm_sap);
    }
    else if (esm_sap.err != ESM_SAP_DISCARDED) {
	/*
	 * Notify EMM that attach procedure failed
	 */
	emm_sap.primitive = EMMREG_ATTACH_REJ;
	emm_sap.u.emm_reg.ueid = ueid;
	rc = emm_sap_send(&emm_sap);
    }
    else {
	/*
	 * ESM procedure failed and, received message has been discarded or
	 * Status message has been returned; ignore ESM procedure failure
	 */
	rc = RETURNok;
    }

    LOG_FUNC_RETURN(rc);
}

#endif	// NAS_MME

/****************************************************************************/
/*********************  L O C A L    F U N C T I O N S  *********************/
/****************************************************************************/

#ifdef NAS_UE
/*
 * --------------------------------------------------------------------------
 *				Timer handlers
 * --------------------------------------------------------------------------
 */

/****************************************************************************
 **                                                                        **
 ** Name:	 _emm_attach_t3410_handler()                               **
 **                                                                        **
 ** Description: T3410 timeout handler                                     **
 **                                                                        **
 **              3GPP TS 24.301, section 5.5.1.2.6, case c                 **
 **		 Upon T3410 timer expiration, the attach procedure shall   **
 **		 be aborted and the UE shall execute abnormal case attach  **
 **		 procedure.                                                **
 **		 The NAS signalling connection shall be released locally.  **
 **                                                                        **
 ** Inputs:	 args:		handler parameters                         **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	None                                       **
 **		 Others:	T3410                                      **
 **                                                                        **
 ***************************************************************************/
void* _emm_attach_t3410_handler(void* args)
{
    LOG_FUNC_IN;

    emm_sap_t emm_sap;
    int rc;

    LOG_TRACE(WARNING, "EMM-PROC  - T3410 timer expired");

    /* Stop T3410 timer */
    T3410.id = nas_timer_stop(T3410.id);
    /* Execute abnormal case attach procedure */
    _emm_attach_abnormal_cases_bcd(&emm_sap);

    rc = emm_sap_send(&emm_sap);
    if (rc != RETURNerror) {
	/* Locally release the NAS signalling connection */
	_emm_data.ecm_status = ECM_IDLE;
    }

    LOG_FUNC_RETURN(NULL);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 _emm_attach_t3411_handler()                               **
 **                                                                        **
 ** Description: T3411 timeout handler                                     **
 **                                                                        **
 **              3GPP TS 24.301, section 5.5.1.2.6                         **
 **		 Upon T3411 timer expiration, the attach procedure shall   **
 **		 be restarted, if still required by ESM sublayer.          **
 **                                                                        **
 ** Inputs:	 args:		handler parameters                         **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	None                                       **
 **		 Others:	T3411                                      **
 **                                                                        **
 ***************************************************************************/
static void* _emm_attach_t3411_handler(void* args)
{
    LOG_FUNC_IN;

    emm_sap_t emm_sap;

    LOG_TRACE(WARNING, "EMM-PROC  - T3411 timer expired");

    /* Stop T3411 timer */
    T3411.id = nas_timer_stop(T3411.id);
    /*
     * Notify EMM that timer T3411 expired and attach procedure has to be
     * restarted
     */
    emm_sap.primitive = EMMREG_ATTACH_INIT;
    emm_sap.u.emm_reg.u.attach.is_emergency = _emm_data.is_emergency;

    (void) emm_sap_send(&emm_sap);

    LOG_FUNC_RETURN(NULL);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 _emm_attach_t3402_handler()                               **
 **                                                                        **
 ** Description: T3402 timeout handler                                     **
 **                                                                        **
 **		 Upon T3402 timer expiration:                              **
 **              3GPP TS 24.301, section 5.5.1.1                           **
 **		 the attach attempt counter shall be reset when the UE is  **
 **		 in substate DEREGISTERED.ATTEMPTING-TO-ATTACH;            **
 **              3GPP TS 24.301, section 5.2.2.3.3                         **
 **		 the UE shall initiate an attach or combined attach proce- **
 **		 dure in substate DEREGISTERED.ATTEMPTING-TO-ATTACH;       **
 **                                                                        **
 ** Inputs:	 args:		handler parameters                         **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	None                                       **
 **		 Others:	_emm_attach_data, T3402                    **
 **                                                                        **
 ***************************************************************************/
static void* _emm_attach_t3402_handler(void* args)
{
    LOG_FUNC_IN;

    emm_sap_t emm_sap;

    LOG_TRACE(WARNING, "EMM-PROC  - T3402 timer expired");

    /* Stop T3402 timer */
    T3402.id = nas_timer_stop(T3402.id);
    /* Reset the attach attempt counter */
    _emm_attach_data.attempt_count = 0;
    /*
     * Notify EMM that timer T3402 expired and attach procedure has to be
     * restarted
     */
    emm_sap.primitive = EMMREG_ATTACH_INIT;
    emm_sap.u.emm_reg.u.attach.is_emergency = _emm_data.is_emergency;

    (void) emm_sap_send(&emm_sap);

    LOG_FUNC_RETURN(NULL);
}

/*
 * --------------------------------------------------------------------------
 *			    Abnormal cases in the UE
 * --------------------------------------------------------------------------
 */

/****************************************************************************
 **                                                                        **
 ** Name:	 _emm_attach_abnormal_cases_bcd()                          **
 **                                                                        **
 ** Description: Performs the abnormal case attach procedure.              **
 **                                                                        **
 **		 3GPP TS 24.301, section 5.5.1.2.6, cases b, c and d       **
 **		 The Timer T3410 shall be stopped if still running, the    **
 **		 attach attempt counter shall be incremented and the UE    **
 **		 shall proceed depending on whether the attach attempt     **
 **		 counter reached its maximum value or not.                 **
 **                                                                        **
 ** Inputs:	 None                                                      **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 emm_sap:	EMM service access point                   **
 **		 Return:	None                                       **
 **		 Others:	_emm_data, _emm_attach_data, T3402, T3410, **
 **				T3411                                      **
 **                                                                        **
 ***************************************************************************/
static void _emm_attach_abnormal_cases_bcd(emm_sap_t* emm_sap)
{
    LOG_FUNC_IN;

    LOG_TRACE(WARNING, "EMM-PROC  - Abnormal case, attach counter = %d",
	      _emm_attach_data.attempt_count);

    /* Stop timer T3410 */
    if (T3410.id != NAS_TIMER_INACTIVE_ID) {
	LOG_TRACE(INFO, "EMM-PROC  - Stop timer T3410 (%d)", T3410.id);
	T3410.id = nas_timer_stop(T3410.id);
    }

    if (_emm_attach_data.attempt_count < EMM_ATTACH_COUNTER_MAX) {
	/* Increment the attach attempt counter */
	_emm_attach_data.attempt_count += 1;
	/* Start T3411 timer */
	T3411.id = nas_timer_start(T3411.sec, _emm_attach_t3411_handler, NULL);
	LOG_TRACE(INFO, "EMM-PROC  - Timer T3411 (%d) expires in %ld seconds",
		  T3411.id, T3411.sec);
	/*
	 * Notify EMM that the attempt to attach for EPS services failed and
	 * the attach attempt counter didn't reach its maximum value; network
	 * attach procedure shall be restarted when timer T3411 expires.
	 */
	emm_sap->primitive = EMMREG_ATTACH_FAILED;
    }
    else {
	/* Delete the GUTI */
	_emm_data.guti = NULL;
	/* Delete the TAI list */
	_emm_data.ltai.n_tais = 0;
	/* Delete the last visited registered TAI */
	_emm_data.tai = NULL;
	/* Delete the list of equivalent PLMNs */
	_emm_data.nvdata.eplmn.n_plmns = 0;
	/* Delete the eKSI */
	if (_emm_data.security) {
	    _emm_data.security->type = EMM_KSI_NOT_AVAILABLE;
	}
	/* Set the EPS update status to EU2 NOT UPDATED */
	_emm_data.status = EU2_NOT_UPDATED;

	/* Start T3402 timer */
	T3402.id = nas_timer_start(T3402.sec, _emm_attach_t3402_handler, NULL);
	LOG_TRACE(INFO, "EMM-PROC  - Timer T3402 (%d) expires in %ld seconds",
		  T3402.id, T3402.sec);
	/*
	 * Notify EMM that the attempt to attach for EPS services failed and
	 * the attach attempt counter reached its maximum value.
	 */
	emm_sap->primitive = EMMREG_ATTACH_EXCEEDED;
    }

    LOG_FUNC_OUT;
}
#endif // NAS_UE

#ifdef NAS_MME
/*
 * --------------------------------------------------------------------------
 *				Timer handlers
 * --------------------------------------------------------------------------
 */

/****************************************************************************
 **                                                                        **
 ** Name:	 _emm_attach_t3450_handler()                               **
 **                                                                        **
 ** Description: T3450 timeout handler                                     **
 **                                                                        **
 **              3GPP TS 24.301, section 5.5.1.2.7, case c                 **
 **		 On the first expiry of the timer T3450, the network shall **
 **		 retransmit the ATTACH ACCEPT message and shall reset and  **
 **		 restart timer T3450. This retransmission is repeated four **
 **		 times, i.e. on the fifth expiry of timer T3450, the at-   **
 **		 tach procedure shall be aborted and the MME enters state  **
 **		 EMM-DEREGISTERED.                                         **
 **                                                                        **
 ** Inputs:	 args:		handler parameters                         **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	None                                       **
 **		 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
static void* _emm_attach_t3450_handler(void* args)
{
    LOG_FUNC_IN;

    int rc;

    attach_data_t* data = (attach_data_t*)(args);

    /* Increment the retransmission counter */
    data->retransmission_count += 1;

    LOG_TRACE(WARNING, "EMM-PROC  - T3450 timer expired, retransmission "
	      "counter = %d", data->retransmission_count);

    /* Get the UE's EMM context */
    emm_data_context_t* emm_ctx = NULL;

#if defined(EPC_BUILD)
    emm_ctx = emm_data_context_get(&_emm_data, data->ueid);
#else
    emm_ctx = _emm_data.ctx[data->ueid];
#endif

    if (data->retransmission_count < ATTACH_COUNTER_MAX) {
	/* Send attach accept message to the UE */
	rc = _emm_attach_accept(emm_ctx, data);
    }
    else {
	/* Abort the attach procedure */
	rc = _emm_attach_abort(data);
    }

    LOG_FUNC_RETURN(NULL);
}

/*
 * --------------------------------------------------------------------------
 *			    Abnormal cases in the MME
 * --------------------------------------------------------------------------
 */

/****************************************************************************
 **                                                                        **
 ** Name:	 _emm_attach_release()                                     **
 **                                                                        **
 ** Description: Releases the UE context data.                             **
 **                                                                        **
 ** Inputs:	 args:		Data to be released                        **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	None                                       **
 **		 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_attach_release(void* args)
{
    LOG_FUNC_IN;

    int rc = RETURNerror;

    emm_data_context_t* emm_ctx = (emm_data_context_t*)(args);

    if (emm_ctx)
    {
	LOG_TRACE(WARNING, "EMM-PROC  - Release UE context data (ueid=%u)",
		  emm_ctx->ueid);

	unsigned int ueid = emm_ctx->ueid;

	if (emm_ctx->guti) free(emm_ctx->guti);
	if (emm_ctx->imsi) free(emm_ctx->imsi);
	if (emm_ctx->imei) free(emm_ctx->imei);
	if (emm_ctx->esm_msg.length > 0) free(emm_ctx->esm_msg.value);
	/* Release NAS security context */
	if (emm_ctx->security) {
	    emm_security_context_t* security = emm_ctx->security;
	    if (security->kasme.value) free(security->kasme.value);
	    if (security->knas_enc.value) free(security->knas_enc.value);
	    if (security->knas_int.value) free(security->knas_int.value);
	    free(emm_ctx->security);
	}
	/* Release the EMM context */
#if defined(EPC_BUILD)
        emm_data_context_remove(&_emm_data, emm_ctx);
#else
	free(_emm_data.ctx[ueid]);
	_emm_data.ctx[ueid] = NULL;
#endif
	/*
	 * Notify EMM that the attach procedure is aborted
	 */
	emm_sap_t emm_sap;
	emm_sap.primitive = EMMREG_PROC_ABORT;
	emm_sap.u.emm_reg.ueid = ueid;
	rc = emm_sap_send(&emm_sap);
    }

    LOG_FUNC_RETURN(rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 _emm_attach_reject()                                      **
 **                                                                        **
 ** Description: Performs the attach procedure not accepted by the network.**
 **                                                                        **
 **              3GPP TS 24.301, section 5.5.1.2.5                         **
 **		 If the attach request cannot be accepted by the network,  **
 **		 the MME shall send an ATTACH REJECT message to the UE in- **
 **		 including an appropriate EMM cause value.                 **
 **                                                                        **
 ** Inputs:	 args:		UE context data                            **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_attach_reject(void* args)
{
    LOG_FUNC_IN;

    int rc = RETURNerror;

    emm_data_context_t* emm_ctx = (emm_data_context_t*)(args);

    if (emm_ctx)
    {
	emm_sap_t emm_sap;
	LOG_TRACE(WARNING, "EMM-PROC  - EMM attach procedure not accepted "
		  "by the network (ueid=%08x, cause=%d)",
		  emm_ctx->ueid, emm_ctx->emm_cause);
	/*
	 * Notify EMM-AS SAP that Attach Reject message has to be sent
	 * onto the network
	 */
	emm_sap.primitive = EMMAS_ESTABLISH_REJ;
	emm_sap.u.emm_as.u.establish.ueid = emm_ctx->ueid;
	emm_sap.u.emm_as.u.establish.UEid.guti = NULL;
	if (emm_ctx->emm_cause == EMM_CAUSE_SUCCESS) {
	    emm_ctx->emm_cause = EMM_CAUSE_ILLEGAL_UE;
	}
	emm_sap.u.emm_as.u.establish.emm_cause = emm_ctx->emm_cause;
	emm_sap.u.emm_as.u.establish.NASinfo = EMM_AS_NAS_INFO_ATTACH;
	if (emm_ctx->emm_cause != EMM_CAUSE_ESM_FAILURE) {
	    emm_sap.u.emm_as.u.establish.NASmsg.length = 0;
	    emm_sap.u.emm_as.u.establish.NASmsg.value = NULL;
	} else if (emm_ctx->esm_msg.length > 0) {
	    emm_sap.u.emm_as.u.establish.NASmsg = emm_ctx->esm_msg;
	} else {
	    LOG_TRACE(ERROR, "EMM-PROC  - ESM message is missing");
	    LOG_FUNC_RETURN(RETURNerror);
	}
	/* Setup EPS NAS security data */
	emm_as_set_security_data(&emm_sap.u.emm_as.u.establish.sctx,
				 emm_ctx->security, FALSE, TRUE);
	rc = emm_sap_send(&emm_sap);

	/* Release the UE context, even if the network failed to send the
	 * ATTACH REJECT message */
	if (emm_ctx->is_dynamic) {
	    rc = _emm_attach_release(emm_ctx);
	}
    }

    LOG_FUNC_RETURN(rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 _emm_attach_abort()                                       **
 **                                                                        **
 ** Description: Aborts the attach procedure                               **
 **                                                                        **
 ** Inputs:	 args:		Attach procedure data to be released       **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	T3450                                      **
 **                                                                        **
 ***************************************************************************/
static int _emm_attach_abort(void* args)
{
    LOG_FUNC_IN;

    int rc = RETURNerror;

    attach_data_t* data = (attach_data_t*)(args);

    if (data)
    {
	unsigned int ueid = data->ueid;

	LOG_TRACE(WARNING, "EMM-PROC  - Abort the attach procedure (ueid=%u)",
		  ueid);

	/* Stop timer T3450 */
	if (T3450.id != NAS_TIMER_INACTIVE_ID) {
	    LOG_TRACE(INFO, "EMM-PROC  - Stop timer T3450 (%d)", T3450.id);
	    T3450.id = nas_timer_stop(T3450.id);
	}
	/* Release retransmission timer parameters */
	if (data->esm_msg.length > 0) {
	    free(data->esm_msg.value);
	}
	free(data);

	/*
	 * Notify ESM that the network locally refused PDN connectivity
	 * to the UE
	 */
	esm_sap_t esm_sap;
	esm_sap.primitive = ESM_PDN_CONNECTIVITY_REJ;
	esm_sap.ueid = ueid;
	esm_sap.recv = NULL;
	rc = esm_sap_send(&esm_sap);
	if (rc != RETURNerror) {
	    /*
	     * Notify EMM that EPS attach procedure failed
	     */
	    emm_sap_t emm_sap;
	    emm_sap.primitive = EMMREG_ATTACH_REJ;
	    emm_sap.u.emm_reg.ueid = ueid;
	    rc = emm_sap_send(&emm_sap);
	    if (rc != RETURNerror) {
                struct emm_data_context_s *ctx = NULL;

#if defined(EPC_BUILD)
                ctx = emm_data_context_get(&_emm_data, ueid);
#else
                ctx = _emm_data.ctx[ueid];
#endif

		/* Release the UE context */
                rc = _emm_attach_release(ctx);
	    }
	}
    }

    LOG_FUNC_RETURN (rc);
}

/*
 * --------------------------------------------------------------------------
 *		Functions that may initiate EMM common procedures
 * --------------------------------------------------------------------------
 */

/****************************************************************************
 **                                                                        **
 ** Name:	 _emm_attach_identify()                                    **
 **                                                                        **
 ** Description: Performs UE's identification. May initiates identifica-   **
 **		 tion, authentication and security mode control EMM common **
 **		 procedures.                                               **
 **                                                                        **
 ** Inputs:	 args:		Identification argument parameters         **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	_emm_data                                  **
 **                                                                        **
 ***************************************************************************/
static int _emm_attach_identify(void* args)
{
    LOG_FUNC_IN;

    int rc = RETURNerror;
    emm_data_context_t* emm_ctx = (emm_data_context_t*)(args);
    int guti_reallocation = FALSE;

    LOG_TRACE(INFO, "EMM-PROC  - Identify incoming UE (ueid=0x%08x) using %s",
	      emm_ctx->ueid, (emm_ctx->imsi)? "IMSI" : (emm_ctx->guti)? "GUTI" :
	      (emm_ctx->imei)? "IMEI" : "none");

    /*
     * UE's identification
     * -------------------
     */
    if (emm_ctx->imsi) {
	/* The UE identifies itself using an IMSI */
	rc = mme_api_identify_imsi(emm_ctx->imsi, &emm_ctx->vector);
	if (rc != RETURNok) {
	    LOG_TRACE(WARNING, "EMM-PROC  - "
		      "Failed to identify the UE using provided IMSI");
	    emm_ctx->emm_cause = EMM_CAUSE_ILLEGAL_UE;
	}
	guti_reallocation = TRUE;
    }
    else if (emm_ctx->guti) {
	/* The UE identifies itself using a GUTI */
	rc = mme_api_identify_guti(emm_ctx->guti, &emm_ctx->vector);
	if (rc != RETURNok) {
	    LOG_TRACE(WARNING, "EMM-PROC  - Failed to identify the UE using "
		      "provided GUTI (tmsi=%u)", emm_ctx->guti->m_tmsi);
	    /* 3GPP TS 24.401, Figure 5.3.2.1-1, point 4
	     * The UE was attempting to attach to the network using a GUTI
	     * that is not known by the network; the MME shall initiate an
	     * identification procedure to retrieve the IMSI from the UE.
	     */
	    rc = emm_proc_identification(emm_ctx->ueid,
                                         emm_ctx,
                                         EMM_IDENT_TYPE_IMSI,
					 _emm_attach_identify,
					 _emm_attach_release,
					 _emm_attach_release);
	    if (rc != RETURNok) {
		/* Failed to initiate the identification procedure */
		LOG_TRACE(WARNING, "EMM-PROC  - "
			  "Failed to initiate identification procedure");
		emm_ctx->emm_cause = EMM_CAUSE_ILLEGAL_UE;
		/* Do not accept the UE to attach to the network */
		rc = _emm_attach_reject(emm_ctx);
	    }
	    /* Relevant callback will be executed when identification
	     * procedure completes */
	    LOG_FUNC_RETURN(rc);
	}
    }
    else if ( (emm_ctx->imei) && (emm_ctx->is_emergency) ) {
	/* The UE is attempting to attach to the network for emergency
	 * services using an IMEI */
	rc = mme_api_identify_imei(emm_ctx->imei, &emm_ctx->vector);
	if (rc != RETURNok) {
	    LOG_TRACE(WARNING, "EMM-PROC  - "
		      "Failed to identify the UE using provided IMEI");
	    emm_ctx->emm_cause = EMM_CAUSE_IMEI_NOT_ACCEPTED;
	}
    }
    else {
	LOG_TRACE(WARNING, "EMM-PROC  - UE's identity is not available");
	emm_ctx->emm_cause = EMM_CAUSE_ILLEGAL_UE;
    }

    /*
     * GUTI reallocation
     * -----------------
     */
    if ( (rc != RETURNerror) && guti_reallocation ) {
	/* Release the old GUTI */
	if (emm_ctx->old_guti) {
	    free(emm_ctx->old_guti);
	}
	/* Save the GUTI previously used by the UE to identify itself */
	emm_ctx->old_guti = emm_ctx->guti;
	/* Allocate a new GUTI */
	emm_ctx->guti = (GUTI_t*)malloc(sizeof(GUTI_t));
	/* Request the MME to assign a GUTI to the UE */
	rc = mme_api_new_guti(emm_ctx->imsi, emm_ctx->guti,
			      &emm_ctx->tac, &emm_ctx->n_tacs);
	if (rc != RETURNok) {
	    LOG_TRACE(WARNING, "EMM-PROC  - Failed to assign new GUTI");
	    emm_ctx->emm_cause = EMM_CAUSE_ILLEGAL_UE;
	}
	else {
	    LOG_TRACE(WARNING, "EMM-PROC  - New GUTI assigned to the UE "
		      "(tmsi=%u)", emm_ctx->guti->m_tmsi);
	    /* Update the GUTI indicator as new */
	    emm_ctx->guti_is_new = TRUE;
	}
    }

    /*
     * UE's authentication
     * -------------------
     */
    if (rc != RETURNerror) {
	if (emm_ctx->security) {
	    /* A security context exists for the UE in the network;
	     * proceed with the attach procedure.
	     */
	    rc = _emm_attach(emm_ctx);
	}
	else if ( (emm_ctx->is_emergency) &&
		  (_emm_data.conf.features & MME_API_UNAUTHENTICATED_IMSI) ) {
	    /* 3GPP TS 24.301, section 5.5.1.2.3
	     * 3GPP TS 24.401, Figure 5.3.2.1-1, point 5a
	     * MME configured to support Emergency Attach for unauthenticated
	     * IMSIs may choose to skip the authentication procedure even if
	     * no EPS security context is available and proceed directly to the
	     * execution of the security mode control procedure.
	     */
	    rc = _emm_attach_security(emm_ctx);
	}
	else {
	    /* 3GPP TS 24.401, Figure 5.3.2.1-1, point 5a
	     * No EMM context exists for the UE in the network; authentication
	     * and NAS security setup to activate integrity protection and NAS
	     * ciphering are mandatory.
	     */
	    auth_vector_t* auth = &emm_ctx->vector;
	    const OctetString loc_rand = {AUTH_RAND_SIZE, (uint8_t*)auth->rand};
	    const OctetString autn = {AUTH_AUTN_SIZE, (uint8_t*)auth->autn};
	    rc = emm_proc_authentication(emm_ctx->ueid, 0, // TODO: eksi != 0
                                         &loc_rand, &autn,
					 _emm_attach_security,
					 _emm_attach_release,
					 _emm_attach_release);
	    if (rc != RETURNok) {
		/* Failed to initiate the authentication procedure */
		LOG_TRACE(WARNING, "EMM-PROC  - "
			  "Failed to initiate authentication procedure");
		emm_ctx->emm_cause = EMM_CAUSE_ILLEGAL_UE;
	    }
	}
   }

    if (rc != RETURNok) {
	/* Do not accept the UE to attach to the network */
	rc = _emm_attach_reject(emm_ctx);
    }
    LOG_FUNC_RETURN (rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 _emm_attach_security()                                    **
 **                                                                        **
 ** Description: Initiates security mode control EMM common procedure.     **
 **                                                                        **
 ** Inputs:	 args:		security argument parameters               **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	_emm_data                                  **
 **                                                                        **
 ***************************************************************************/
static int _emm_attach_security(void* args)
{
    LOG_FUNC_IN;

    int rc;
    emm_data_context_t* emm_ctx = (emm_data_context_t*)(args);

    LOG_TRACE(INFO, "EMM-PROC  - Setup NAS security (ueid=%u)", emm_ctx->ueid);

    /* Create new NAS security context */
    if (emm_ctx->security == NULL) {
	emm_ctx->security =
	    (emm_security_context_t*)malloc(sizeof(emm_security_context_t));
    }
    if (emm_ctx->security) {
	memset(emm_ctx->security, 0, sizeof(emm_security_context_t));
	emm_ctx->security->type = EMM_KSI_NOT_AVAILABLE;
    }
    else {
	LOG_TRACE(WARNING, "EMM-PROC  - Failed to create security context");
	emm_ctx->emm_cause = EMM_CAUSE_ILLEGAL_UE;
	/* Do not accept the UE to attach to the network */
	rc = _emm_attach_reject(emm_ctx);
	LOG_FUNC_RETURN(rc);
    }

    /* Initialize the security mode control procedure */
    rc = emm_proc_security_mode_control(emm_ctx->ueid, 0, // TODO: eksi != 0
					emm_ctx->eea, emm_ctx->eia,
					_emm_attach, _emm_attach_release,
					_emm_attach_release);
    if (rc != RETURNok) {
	/* Failed to initiate the security mode control procedure */
	LOG_TRACE(WARNING, "EMM-PROC  - "
		  "Failed to initiate security mode control procedure");
	emm_ctx->emm_cause = EMM_CAUSE_ILLEGAL_UE;
	/* Do not accept the UE to attach to the network */
	rc = _emm_attach_reject(emm_ctx);
    }

    LOG_FUNC_RETURN (rc);
}

/*
 * --------------------------------------------------------------------------
 *			    MME specific local functions
 * --------------------------------------------------------------------------
 */

/****************************************************************************
 **                                                                        **
 ** Name:	 _emm_attach()                                             **
 **                                                                        **
 ** Description: Performs the attach signalling procedure while a context  **
 **		 exists for the incoming UE in the network.                **
 **                                                                        **
 **              3GPP TS 24.301, section 5.5.1.2.4                         **
 **		 Upon receiving the ATTACH REQUEST message, the MME shall  **
 **		 send an ATTACH ACCEPT message to the UE and start timer   **
 **		 T3450.                                                    **
 **                                                                        **
 ** Inputs:	 args:		attach argument parameters                 **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	_emm_data                                  **
 **                                                                        **
 ***************************************************************************/
static int _emm_attach(void* args)
{
    LOG_FUNC_IN;

    esm_sap_t esm_sap;
    int rc;

    emm_data_context_t* emm_ctx = (emm_data_context_t*)(args);

    LOG_TRACE(INFO, "EMM-PROC  - Attach UE (ueid=%u)", emm_ctx->ueid);

    /* 3GPP TS 24.401, Figure 5.3.2.1-1, point 5a
     * At this point, all NAS messages shall be protected by the NAS security
     * functions (integrity and ciphering) indicated by the MME unless the UE
     * is emergency attached and not successfully authenticated.
     */

    /*
     * Notify ESM that PDN connectivity is requested
     */
    esm_sap.primitive = ESM_PDN_CONNECTIVITY_REQ;
    esm_sap.is_standalone = FALSE;
    esm_sap.ueid = emm_ctx->ueid;
    esm_sap.recv = &emm_ctx->esm_msg;
    rc = esm_sap_send(&esm_sap);

    if ( (rc != RETURNerror) && (esm_sap.err == ESM_SAP_SUCCESS) ) {
	/*
	 * The attach request is accepted by the network
	 */

	/* Delete the stored UE radio capability information, if any */
	/* Store the UE network capability */
	/* Assign the TAI list the UE is registered to */

	/* Allocate parameters of the retransmission timer callback */
	attach_data_t* data = (attach_data_t*)malloc(sizeof(attach_data_t));

	if (data != NULL) {
	    /* Setup ongoing EMM procedure callback functions */
	    rc = emm_proc_common_initialize(emm_ctx->ueid, NULL, NULL, NULL,
					    _emm_attach_abort, data);
	    if (rc != RETURNok) {
		LOG_TRACE(WARNING,
			  "Failed to initialize EMM callback functions");
		free(data);
		LOG_FUNC_RETURN (RETURNerror);
	    }
	    /* Set the UE identifier */
	    data->ueid = emm_ctx->ueid;
	    /* Reset the retransmission counter */
	    data->retransmission_count = 0;
	    /* Setup the ESM message container */
	    data->esm_msg.value = (uint8_t*)malloc(esm_sap.send.length);
	    if (data->esm_msg.value) {
		data->esm_msg.length = esm_sap.send.length;
		memcpy(data->esm_msg.value, esm_sap.send.value,
		       esm_sap.send.length);
	    } else {
		data->esm_msg.length = 0;
	    }
	    /* Send attach accept message to the UE */
	    rc = _emm_attach_accept(emm_ctx, data);
	    if (rc != RETURNerror) {
		if (emm_ctx->guti_is_new && emm_ctx->old_guti) {
		    /* Implicit GUTI reallocation;
		     * Notify EMM that common procedure has been initiated
		     */
		    emm_sap_t emm_sap;
		    emm_sap.primitive = EMMREG_COMMON_PROC_REQ;
		    emm_sap.u.emm_reg.ueid = data->ueid;
		    rc = emm_sap_send(&emm_sap);
		}
	    }
	}
    }
    else if (esm_sap.err != ESM_SAP_DISCARDED) {
	/*
	 * The attach procedure failed due to an ESM procedure failure
	 */
	emm_ctx->emm_cause = EMM_CAUSE_ESM_FAILURE;
	/* Setup the ESM message container to include PDN Connectivity Reject
	 * message within the Attach Reject message */
	if (emm_ctx->esm_msg.length > 0) {
	    free(emm_ctx->esm_msg.value);
	}
	emm_ctx->esm_msg.value = (uint8_t*)malloc(esm_sap.send.length);
	if (emm_ctx->esm_msg.value) {
	    emm_ctx->esm_msg.length = esm_sap.send.length;
	    memcpy(emm_ctx->esm_msg.value, esm_sap.send.value,
		   esm_sap.send.length);
	    /* Send Attach Reject message */
	    rc = _emm_attach_reject(emm_ctx);
	} else {
	    emm_ctx->esm_msg.length = 0;
	}
    }
    else {
	/*
	 * ESM procedure failed and, received message has been discarded or
	 * Status message has been returned; ignore ESM procedure failure
	 */
	rc = RETURNok;
    }

    if (rc != RETURNok) {
	/* The attach procedure failed */
	LOG_TRACE(WARNING, "EMM-PROC  - Failed to respond to Attach Request");
	emm_ctx->emm_cause = EMM_CAUSE_PROTOCOL_ERROR;
	/* Do not accept the UE to attach to the network */
	rc = _emm_attach_reject(emm_ctx);
    }

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 _emm_attach_accept()                                      **
 **                                                                        **
 ** Description: Sends ATTACH ACCEPT message and start timer T3450         **
 **                                                                        **
 ** Inputs:	 data:		Attach accept retransmission data          **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	T3450                                      **
 **                                                                        **
 ***************************************************************************/
static int _emm_attach_accept(emm_data_context_t* emm_ctx, attach_data_t* data)
{
    LOG_FUNC_IN;

    emm_sap_t emm_sap;
    int rc;

    /*
     * Notify EMM-AS SAP that Attach Accept message together with an Activate
     * Default EPS Bearer Context Request message has to be sent to the UE
     */
    emm_sap.primitive = EMMAS_ESTABLISH_CNF;
    emm_sap.u.emm_as.u.establish.ueid = emm_ctx->ueid;
    if (emm_ctx->guti_is_new && emm_ctx->old_guti) {
	/* Implicit GUTI reallocation;
	 * include the new assigned GUTI in the Attach Accept message  */
	emm_sap.u.emm_as.u.establish.UEid.guti = emm_ctx->old_guti;
	emm_sap.u.emm_as.u.establish.new_guti = emm_ctx->guti;
    } else {
	emm_sap.u.emm_as.u.establish.UEid.guti = emm_ctx->guti;
	emm_sap.u.emm_as.u.establish.new_guti = NULL;
    }
    emm_sap.u.emm_as.u.establish.n_tacs = emm_ctx->n_tacs;
    emm_sap.u.emm_as.u.establish.tac = emm_ctx->tac;
    emm_sap.u.emm_as.u.establish.NASinfo = EMM_AS_NAS_INFO_ATTACH;
    /* Setup EPS NAS security data */
    emm_as_set_security_data(&emm_sap.u.emm_as.u.establish.sctx,
			     emm_ctx->security, FALSE, TRUE);
    /* Get the activate default EPS bearer context request message to
     * transfer within the ESM container of the attach accept message */
    emm_sap.u.emm_as.u.establish.NASmsg = data->esm_msg;
    rc = emm_sap_send(&emm_sap);

    if (rc != RETURNerror) {
	if (T3450.id != NAS_TIMER_INACTIVE_ID) {
	    /* Re-start T3450 timer */
	    T3450.id = nas_timer_restart(T3450.id);
	} else {
	    /* Start T3450 timer */
	    T3450.id = nas_timer_start(T3450.sec, _emm_attach_t3450_handler, data);
	}
	LOG_TRACE(INFO,"EMM-PROC  - Timer T3450 (%d) expires in %ld seconds",
		  T3450.id, T3450.sec);
    }

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 _emm_attach_have_changed()                                **
 **                                                                        **
 ** Description: Check whether the given attach parameters differs from    **
 **		 those previously stored when the attach procedure has     **
 **		 been initiated.                                           **
 **                                                                        **
 ** Inputs:	 ctx:		EMM context of the UE in the network       **
 **		 type:		Type of the requested attach               **
 **		 ksi:		Security ket sey identifier                **
 **		 guti:		The GUTI provided by the UE                **
 **		 imsi:		The IMSI provided by the UE                **
 **		 imei:		The IMEI provided by the UE                **
 **		 eea:		Supported EPS encryption algorithms        **
 **		 eia:		Supported EPS integrity algorithms         **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	TRUE if at least one of the parameters     **
 **				differs; FALSE otherwise.                  **
 **		 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_attach_have_changed(const emm_data_context_t* ctx,
				    emm_proc_attach_type_t type, int ksi,
				    GUTI_t* guti, imsi_t* imsi, imei_t* imei,
				    int eea, int eia)
{
    /* Emergency bearer services indicator */
    if ( (type == EMM_ATTACH_TYPE_EMERGENCY) != ctx->is_emergency)
	return (TRUE);
    /* Security key set identifier */
    if (ksi != ctx->ksi) return (TRUE);
    /* Supported EPS encryption algorithms */
    if (eea != ctx->eea) return (TRUE);
    /* Supported EPS integrity algorithms */
    if (eia != ctx->eia) return (TRUE);
    /* The GUTI if provided by the UE */
    if ( (guti) && (ctx->guti == NULL) ) return (TRUE);
    if ( (guti == NULL) && (ctx->guti) ) return (TRUE);
    if ( (guti) && (ctx->guti) ) {
	if (guti->m_tmsi != ctx->guti->m_tmsi) return (TRUE);
	if ( memcmp(&guti->gummei, &ctx->guti->gummei, sizeof(gummei_t)) != 0 )
	    return (TRUE);
    }
    /* The IMSI if provided by the UE */
    if ( (imsi) && (ctx->imsi == NULL) ) return (TRUE);
    if ( (imsi == NULL) && (ctx->imsi) ) return (TRUE);
    if ( (imsi) && (ctx->imsi) ) {
	if ( memcmp(imsi, ctx->imsi, sizeof(imsi_t)) != 0 ) return (TRUE);
    }
    /* The IMEI if provided by the UE */
    if ( (imei) && (ctx->imei == NULL) ) return (TRUE);
    if ( (imei == NULL) && (ctx->imei) ) return (TRUE);
    if ( (imei) && (ctx->imei) ) {
	if ( memcmp(imei, ctx->imei, sizeof(imei_t)) != 0 ) return (TRUE);
    }
    return (FALSE);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 _emm_attach_update()                                      **
 **                                                                        **
 ** Description: Update the EMM context with the given attach procedure    **
 **		 parameters.                                               **
 **                                                                        **
 ** Inputs:	 ueid:		UE lower layer identifier                  **
 **		 type:		Type of the requested attach               **
 **		 ksi:		Security ket sey identifier                **
 **		 guti:		The GUTI provided by the UE                **
 **		 imsi:		The IMSI provided by the UE                **
 **		 imei:		The IMEI provided by the UE                **
 **		 eea:		Supported EPS encryption algorithms        **
 **		 eia:		Supported EPS integrity algorithms         **
 **		 esm_msg:	ESM message contained with the attach re-  **
 **				quest                                      **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 ctx:		EMM context of the UE in the network       **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
static int _emm_attach_update(emm_data_context_t* ctx, unsigned int ueid,
			      emm_proc_attach_type_t type, int ksi,
			      GUTI_t* guti, imsi_t* imsi, imei_t* imei,
			      int eea, int eia, const OctetString* esm_msg)
{
    /* UE identifier */
    ctx->ueid = ueid;
    /* Emergency bearer services indicator */
    ctx->is_emergency = (type == EMM_ATTACH_TYPE_EMERGENCY);
    /* Security key set identifier */
    ctx->ksi = ksi;
    /* Supported EPS encryption algorithms */
    ctx->eea = eea;
    /* Supported EPS integrity algorithms */
    ctx->eia = eia;
    /* The GUTI if provided by the UE */
    if (guti) {
	if (ctx->guti == NULL)
	    ctx->guti = (GUTI_t*)malloc(sizeof(GUTI_t));
	if (ctx->guti != NULL)
	    memcpy(ctx->guti, guti, sizeof(GUTI_t));
	else return (RETURNerror);
    }
    /* The IMSI if provided by the UE */
    if (imsi) {
	if (ctx->imsi == NULL)
	    ctx->imsi = (imsi_t*)malloc(sizeof(imsi_t));
	if (ctx->imsi != NULL)
	    memcpy(ctx->imsi, imsi, sizeof(imsi_t));
	else return (RETURNerror);
    }
    /* The IMEI if provided by the UE */
    if (imei) {
	if (ctx->imei == NULL)
	    ctx->imei = (imei_t*)malloc(sizeof(imei_t));
	if (ctx->imei != NULL)
	    memcpy(ctx->imei, imei, sizeof(imei_t));
	else return (RETURNerror);
    }
    /* The ESM message contained within the attach request */
    if (esm_msg->length > 0) {
	if (ctx->esm_msg.length == 0) {
	    ctx->esm_msg.value = (uint8_t*)malloc(esm_msg->length);
	}
	if (ctx->esm_msg.value != NULL) {
	    strncpy((char*)ctx->esm_msg.value,
		    (char*)esm_msg->value, esm_msg->length);
	} else return (RETURNerror);
    }
    ctx->esm_msg.length = esm_msg->length;
    /* Attachment indicator */
    ctx->is_attached = FALSE;

    return (RETURNok);
}

#endif // NAS_MME
