/*****************************************************************************
			Eurecom OpenAirInterface 3
			Copyright(c) 2012 Eurecom

Source		Authentication.c

Version		0.1

Date		2013/03/04

Product		NAS stack

Subsystem	EPS Mobility Management

Author		Frederic Maurel

Description	Defines the authentication EMM procedure executed by the
		Non-Access Stratum.

		The purpose of the EPS authentication and key agreement (AKA)
		procedure is to provide mutual authentication between the user
		and the network and to agree on a key KASME. The procedure is
		always initiated and controlled by the network. However, the
		UE can reject the EPS authentication challenge sent by the
		network.

		A partial native EPS security context is established in the
		UE and the network when an EPS authentication is successfully
		performed. The computed key material KASME is used as the
		root for the EPS integrity protection and ciphering key
		hierarchy.

*****************************************************************************/

#include "emm_proc.h"
#include "nas_log.h"
#include "nas_timer.h"

#include "emmData.h"

#include "emm_sap.h"
#include "emm_cause.h"

#ifdef NAS_UE
#include "usim_api.h"
#endif

#include <stdlib.h>	// malloc, free
#include <string.h>	// memcpy, memcmp, memset

/****************************************************************************/
/****************  E X T E R N A L    D E F I N I T I O N S  ****************/
/****************************************************************************/

#ifdef NAS_UE
/*
 * Retransmission timer handlers
 */
extern void* _emm_attach_t3410_handler(void*);
extern void* _emm_service_t3417_handler(void*);
extern void* _emm_detach_t3421_handler(void*);
extern void* _emm_tau_t3430_handler(void*);
#endif // NAS_UE

/****************************************************************************/
/*******************  L O C A L    D E F I N I T I O N S  *******************/
/****************************************************************************/

/*
 * --------------------------------------------------------------------------
 * 	Internal data handled by the authentication procedure in the UE
 * --------------------------------------------------------------------------
 */
#ifdef NAS_UE
/*
 * Timer handlers
 */
static void* _authentication_t3416_handler(void*);
static void* _authentication_t3418_handler(void*);
static void* _authentication_t3420_handler(void*);

/*
 * Internal data used for authentication procedure
 */
static struct {
    uint8_t rand[AUTH_RAND_SIZE];	/* Random challenge number	*/
    uint8_t res[AUTH_RES_SIZE];		/* Authentication response	*/
    uint8_t ck[AUTH_CK_SIZE];		/* Ciphering key		*/
    uint8_t ik[AUTH_IK_SIZE];		/* Integrity key		*/
#define AUTHENTICATION_T3410	0x01
#define AUTHENTICATION_T3417	0x02
#define AUTHENTICATION_T3421	0x04
#define AUTHENTICATION_T3430	0x08
    unsigned char timers;		/* Timer restart bitmap		*/
#define AUTHENTICATION_COUNTER_MAX 3
    unsigned char mac_count:2;	/* MAC failure counter (#20)		*/
    unsigned char umts_count:2;	/* UMTS challenge failure counter (#26)	*/
    unsigned char sync_count:2;	/* Sync failure counter (#21)		*/
} _authentication_data;

/*
 * Abnormal case authentication procedure
 */
static int _authentication_abnormal_cases_cde(int emm_cause, const OctetString* auts);
static int _authentication_abnormal_case_f(void);

static int _authentication_stop_timers(void);
static int _authentication_start_timers(void);
static int _authentication_kasme(const OctetString* autn, const OctetString* ck, const OctetString* ik, const plmn_t* plmn, OctetString* kasme);
#endif // NAS_UE

/*
 * --------------------------------------------------------------------------
 * 	Internal data handled by the authentication procedure in the MME
 * --------------------------------------------------------------------------
 */
#ifdef NAS_MME
/*
 * Timer handlers
 */
static void* _authentication_t3460_handler(void*);

/*
 * Function executed whenever the ongoing EMM procedure that initiated
 * the authentication procedure is aborted or the maximum value of the
 * retransmission timer counter is exceed
 */
static int _authentication_abort(void*);

/*
 * Internal data used for authentication procedure
 */
typedef struct {
    unsigned int ueid;			/* UE identifier		*/
#define AUTHENTICATION_COUNTER_MAX	5
    unsigned int retransmission_count;	/* Retransmission counter	*/
    int ksi;				/* NAS key set identifier	*/
    OctetString rand;			/* Random challenge number	*/
    OctetString autn;			/* Authentication token		*/
    int notify_failure;			/* Indicates whether the authentication
					 * procedure failure shall be notified
					 * to the ongoing EMM procedure	*/
} authentication_data_t;

static int _authentication_request(authentication_data_t* data);
static int _authentication_reject(unsigned int ueid);
#endif // NAS_MME

/****************************************************************************/
/******************  E X P O R T E D    F U N C T I O N S  ******************/
/****************************************************************************/

/*
 * --------------------------------------------------------------------------
 * 		Authentication procedure executed by the UE
 * --------------------------------------------------------------------------
 */
#ifdef NAS_UE
/****************************************************************************
 **                                                                        **
 ** Name:	 emm_proc_authentication_request()                         **
 **                                                                        **
 ** Description: Performs the MME requested authentication procedure.      **
 **                                                                        **
 **              3GPP TS 24.301, section 5.4.2.3                           **
 **		 Upon receiving the AUTHENTICATION REQUEST message, the UE **
 **		 shall store the received RAND together with the RES re-   **
 **		 turned from the USIM in the volatile memory of the ME, to **
 **		 avoid a synchronisation failure. The UE shall process the **
 **		 authentication challenge data and respond with an AUTHEN- **
 **		 TICATION RESPONSE message to the network.                 **
 **                                                                        **
 ** Inputs:	 native_ksi:	TRUE if the security context is of type    **
 **				native (for KSIASME)                       **
 **		 ksi:		The NAS ket sey identifier                 **
 **		 rand:		Authentication parameter RAND              **
 **		 autn:		Authentication parameter AUTN              **
 **		 Others:	_emm_data, _authentication_data            **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	_emm_data, _authentication_data, T3416,    **
 **				T3418, T3420                               **
 **                                                                        **
 ***************************************************************************/
int emm_proc_authentication_request(int native_ksi, int ksi,
				    const OctetString* rand,
				    const OctetString* autn)
{
    LOG_FUNC_IN;

    int rc = RETURNerror;

    LOG_TRACE(INFO, "EMM-PROC  - Authentication requested ksi = %d", ksi);

    /* 3GPP TS 24.301, section 5.4.2.1
     * The UE shall proceed with an EPS authentication challenge only if a
     * USIM is present
     */
    if (!_emm_data.usim_is_valid) {
	LOG_TRACE(WARNING, "EMM-PROC  - USIM is not present or not valid");
	LOG_FUNC_RETURN (RETURNerror);
    }

    /* Stop timer T3418, if running */
    if (T3418.id != NAS_TIMER_INACTIVE_ID) {
	LOG_TRACE(INFO, "EMM-PROC  - Stop timer T3418 (%d)", T3418.id);
	T3418.id = nas_timer_stop(T3418.id);
    }
    /* Stop timer T3420, if running */
    if (T3420.id != NAS_TIMER_INACTIVE_ID) {
	LOG_TRACE(INFO, "EMM-PROC  - Stop timer T3420 (%d)", T3420.id);
	T3420.id = nas_timer_stop(T3420.id);
    }

    /* Setup security keys */
    OctetString ck = {AUTH_CK_SIZE, _authentication_data.ck};
    OctetString ik = {AUTH_IK_SIZE, _authentication_data.ik};
    OctetString res = {AUTH_RES_SIZE, _authentication_data.res};

    if (memcmp(_authentication_data.rand, rand->value, AUTH_CK_SIZE) != 0) {
	/*
	 * There is no valid stored RAND in the ME or the stored RAND is
	 * different from the new received value in the AUTHENTICATION
	 * REQUEST message
	 */
	OctetString auts;
	auts.length = 0;
	auts.value = (uint8_t*)malloc(AUTH_AUTS_SIZE);
	if (auts.value == NULL) {
	    LOG_TRACE(WARNING, "EMM-PROC  - Failed to allocate AUTS parameter");
	    LOG_FUNC_RETURN (RETURNerror);
	}

	/* 3GPP TS 33.401, section 6.1.1
	 * Get the "separation bit" of the AMF field of AUTN */
	int sbit = AUTH_AMF_SEPARATION_BIT(autn->value[AUTH_AMF_INDEX]);
	if (sbit != 0) {
	    /*
	     * Perform EPS authentication challenge to check the authenticity
	     * of the core network by means of the received AUTN parameter and
	     * request the USIM to compute RES, CK and IK for given RAND
	     */
	    rc = usim_api_authenticate(rand, autn, &auts, &res, &ck, &ik);
	}
	if (rc != RETURNok) {
	    /*
	     * Network authentication not accepted by the UE
	     */
	    LOG_TRACE(WARNING, "EMM-PROC  - Network authentication failed (%s)",
		      (auts.length > 0) ? "SQN failure" :
		      (sbit == 0) ? "Non-EPS authentication unacceptable" :
		      "MAC code failure");
	    /* Delete any previously stored RAND and RES and stop timer T3416 */
	    (void) emm_proc_authentication_delete();
	    /* Proceed authentication abnormal cases procedure */
	    if (auts.length > 0) {
		/* 3GPP TS 24.301, section 5.4.2.6, case e
		 * SQN failure */
		rc = _authentication_abnormal_cases_cde(
				EMM_CAUSE_SYNCH_FAILURE, &auts);
	    } else if (sbit == 0) {
		/* 3GPP TS 24.301, section 5.4.2.6, case d
		 * Non-EPS authentication unacceptable */
		rc = _authentication_abnormal_cases_cde(
				EMM_CAUSE_NON_EPS_AUTH_UNACCEPTABLE, NULL);
	    } else {
		/* 3GPP TS 24.301, section 5.4.2.6, case c
		 * MAC code failure */
		rc = _authentication_abnormal_cases_cde(
				EMM_CAUSE_MAC_FAILURE, NULL);
	    }
	    /* Free the AUTS parameter */
	    free(auts.value);
	    LOG_FUNC_RETURN (rc);
	}

	/* Free the AUTS parameter */
	free(auts.value);
	/* Store the new RAND in the volatile memory */
	if (rand->length <= AUTH_RAND_SIZE) {
	    memcpy(_authentication_data.rand, rand->value, rand->length);
	}
	/* Start, or reset and restart timer T3416 */
	if (T3416.id != NAS_TIMER_INACTIVE_ID) {
	    LOG_TRACE(INFO, "EMM-PROC  - Stop timer T3416 (%d)", T3416.id);
	    T3416.id = nas_timer_stop(T3416.id);
	}
	T3416.id = nas_timer_start(T3416.sec, _authentication_t3416_handler, NULL);
	LOG_TRACE(INFO, "EMM-PROC  - Timer T3416 (%d) expires in  %ld seconds",
		  T3416.id, T3416.sec);
    }

    /*
     * The stored RAND value is equal to the new received value in the
     * AUTHENTICATION REQUEST message, or the UE has successfully checked
     * the authenticity of the core network
     */
    /* Start any retransmission timers */
    rc = _authentication_start_timers();
    if (rc != RETURNok) {
	LOG_TRACE(WARNING, "EMM-PROC  - Failed to start retransmission timers");
	LOG_FUNC_RETURN (RETURNerror);
    }

    /* Setup EMM procedure handler to be executed upon receiving
     * lower layer notification */
    rc = emm_proc_lowerlayer_initialize(NULL, NULL, NULL, NULL);
    if (rc != RETURNok) {
	LOG_TRACE(WARNING,
		  "EMM-PROC  - Failed to initialize EMM procedure handler");
	LOG_FUNC_RETURN (RETURNerror);
    }

    /*
     * Notify EMM-AS SAP that Authentication Response message has to be sent
     * to the network
     */
    emm_sap_t emm_sap;
    emm_sap.primitive = EMMAS_SECURITY_RES;
    emm_sap.u.emm_as.u.security.guti = _emm_data.guti;
    emm_sap.u.emm_as.u.security.ueid = 0;
    emm_sap.u.emm_as.u.security.msgType = EMM_AS_MSG_TYPE_AUTH;
    emm_sap.u.emm_as.u.security.emm_cause = EMM_CAUSE_SUCCESS;
    emm_sap.u.emm_as.u.security.res = &res;
    /* Setup EPS NAS security data */
    emm_as_set_security_data(&emm_sap.u.emm_as.u.security.sctx,
			     _emm_data.security, FALSE, TRUE);
    rc = emm_sap_send(&emm_sap);

    if (rc != RETURNerror) {
	/* Reset the authentication failure counters */
	_authentication_data.mac_count = 0;
	_authentication_data.umts_count = 0;
	_authentication_data.sync_count = 0;
	/* Create non-current EPS security context */
	if (_emm_data.non_current == NULL) {
	    _emm_data.non_current =
		(emm_security_context_t*)malloc(sizeof(emm_security_context_t));
	}
	if (_emm_data.non_current) {
	    memset(_emm_data.non_current, 0, sizeof(emm_security_context_t));
	    /* Set the security context type */
	    if (native_ksi) {
		_emm_data.non_current->type = EMM_KSI_NATIVE;
	    } else {
		_emm_data.non_current->type = EMM_KSI_MAPPED;
	    }
	    /* Set the EPS key set identifier */
	    _emm_data.non_current->eksi = ksi;
	    /* Derive the Kasme from the authentication challenge using
	     * the PLMN identity of the selected PLMN */
	    _emm_data.non_current->kasme.length = AUTH_KASME_SIZE;
	    _emm_data.non_current->kasme.value =
		(uint8_t*)malloc(sizeof(AUTH_KASME_SIZE));
	    _authentication_kasme(autn, &ck, &ik, &_emm_data.splmn,
	    			  &_emm_data.non_current->kasme);
	    /* NAS integrity and cyphering keys are not yet available */
	}
    }

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 emm_proc_authentication_reject()                          **
 **                                                                        **
 ** Description: Performs the authentication procedure not accepted by the **
 **		 network.                                                  **
 **                                                                        **
 **              3GPP TS 24.301, section 5.4.2.5                           **
 **		 Upon receiving an AUTHENTICATION REJECT message, the UE   **
 **		 shall abort any EMM signalling procedure, stop any of the **
 **		 timers T3410, T3417 or T3430 (if running) and enter state **
 **		 EMM-DEREGISTERED.                                         **
 **                                                                        **
 ** Inputs:	 None                                                      **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	_emm_data, _authentication_data, T3410,    **
 **				T3417, T3430                               **
 **                                                                        **
 ***************************************************************************/
int emm_proc_authentication_reject(void)
{
    LOG_FUNC_IN;

    emm_sap_t emm_sap;
    int rc;

    LOG_TRACE(WARNING, "EMM-PROC  - Authentication not accepted by the network");

    /* Delete any previously stored RAND and RES and stop timer T3416 */
    (void) emm_proc_authentication_delete();

    /* Set the EPS update status to EU3 ROAMING NOT ALLOWED */
    _emm_data.status = EU3_ROAMING_NOT_ALLOWED;
    /* Delete the stored GUTI */
    _emm_data.guti = NULL;
    /* Delete the TAI list */
    _emm_data.ltai.n_tais = 0;
    /* Delete the last visited registered TAI */
    _emm_data.tai = NULL;
    /* Delete the eKSI */
    if (_emm_data.security) {
	_emm_data.security->type = EMM_KSI_NOT_AVAILABLE;
    }
    /* Consider the USIM invalid */
    _emm_data.usim_is_valid = FALSE;

    /* Stop timer T3410 */
    if (T3410.id != NAS_TIMER_INACTIVE_ID) {
	LOG_TRACE(INFO, "EMM-PROC  - Stop timer T3410 (%d)", T3410.id);
	T3410.id = nas_timer_stop(T3410.id);
    }
    /* Stop timer T3417 */
    if (T3417.id != NAS_TIMER_INACTIVE_ID) {
	LOG_TRACE(INFO, "EMM-PROC  - Stop timer T3417 (%d)", T3417.id);
	T3417.id = nas_timer_stop(T3417.id);
    }
    /* Stop timer T3430 */
    if (T3430.id != NAS_TIMER_INACTIVE_ID) {
	LOG_TRACE(INFO, "EMM-PROC  - Stop timer T3430 (%d)", T3430.id);
	T3430.id = nas_timer_stop(T3430.id);
    }

    /* Abort any EMM signalling procedure (prevent the retransmission timers to
     * be restarted) */
    _authentication_data.timers = 0x00;

    /*
     * Notify EMM that authentication is not accepted by the network
     */
    emm_sap.primitive = EMMREG_AUTH_REJ;
    rc = emm_sap_send(&emm_sap);

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 emm_proc_authentication_delete()                          **
 **                                                                        **
 ** Description: Deletes the RAND and RES values stored into the volatile  **
 **		 memory of the Mobile Equipment and stop timer T3416, if   **
 **		 running, upon receipt of a SECURITY MODE COMMAND, SERVICE **
 **		 REJECT, TRACKING AREA UPDATE REJECT, TRACKING AREA UPDATE **
 **		 ACCEPT or AUTHENTICATION REJECT message; upon expiry of   **
 **		 timer  T3416; or if the UE  enters  the  EMM  state  EMM- **
 **		 DEREGISTERED or EMM-NULL.                                 **
 **                                                                        **
 **              3GPP TS 24.301, section 5.4.2.3                           **
 **                                                                        **
 ** Inputs:	 None                                                      **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	_authentication_data, T3416                **
 **                                                                        **
 ***************************************************************************/
int emm_proc_authentication_delete(void)
{
    LOG_FUNC_IN;

    LOG_TRACE(INFO, "EMM-PROC  - Delete authentication data RAND and RES");

    /* Stop timer T3416, if running */
    if (T3416.id != NAS_TIMER_INACTIVE_ID) {
	LOG_TRACE(INFO, "EMM-PROC  - Stop timer T3416 (%d)", T3416.id);
	T3416.id = nas_timer_stop(T3416.id);
    }
    /* Delete any previously stored RAND and RES */
    memset(_authentication_data.rand, 0, AUTH_RAND_SIZE);
    memset(_authentication_data.res, 0, AUTH_RES_SIZE);

    LOG_FUNC_RETURN (RETURNok);
}
#endif // NAS_UE

/*
 * --------------------------------------------------------------------------
 * 		Authentication procedure executed by the MME
 * --------------------------------------------------------------------------
 */
#ifdef NAS_MME
/****************************************************************************
 **                                                                        **
 ** Name:	 emm_proc_authentication()                                 **
 **                                                                        **
 ** Description: Initiates authentication procedure to establish partial   **
 **		 native EPS security context in the UE and the MME.        **
 **                                                                        **
 **              3GPP TS 24.301, section 5.4.2.2                           **
 **		 The network initiates the authentication procedure by     **
 **		 sending an AUTHENTICATION REQUEST message to the UE and   **
 **		 starting the timer T3460. The AUTHENTICATION REQUEST mes- **
 **		 sage contains the parameters necessary to calculate the   **
 **		 authentication response.                                  **
 **                                                                        **
 ** Inputs:	 ueid:		UE lower layer identifier                  **
 **		 ksi:		NAS key set identifier                     **
 **		 rand:		Random challenge number                    **
 **		 autn:		Authentication token                       **
 **		 success:	Callback function executed when the authen-**
 **				tication procedure successfully completes  **
 **		 reject:	Callback function executed when the authen-**
 **				tication procedure fails or is rejected    **
 **		 failure:	Callback function executed whener a lower  **
 **				layer failure occured before the authenti- **
 **				cation procedure comnpletes                **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_proc_authentication(unsigned int ueid, int ksi,
			    const OctetString* _rand, const OctetString* autn,
			    emm_common_success_callback_t success,
			    emm_common_reject_callback_t reject,
			    emm_common_failure_callback_t failure)

{
    LOG_FUNC_IN;

    int rc = RETURNerror;

    LOG_TRACE(INFO, "EMM-PROC  - Initiate authentication KSI = %d", ksi);

    /* Allocate parameters of the retransmission timer callback */
    authentication_data_t* data =
	(authentication_data_t*)malloc(sizeof(authentication_data_t));

    if (data != NULL) {
	/* Setup ongoing EMM procedure callback functions */
	rc = emm_proc_common_initialize(ueid, success, reject, failure,
					_authentication_abort, data);
	if (rc != RETURNok) {
	    LOG_TRACE(WARNING, "Failed to initialize EMM callback functions");
	    LOG_FUNC_RETURN (RETURNerror);
	}

	/* Set the UE identifier */
	data->ueid = ueid;
	/* Reset the retransmission counter */
	data->retransmission_count = 0;
	/* Set the key set identifier */
	data->ksi = ksi;
	/* Set the authentication random challenge number */
        if (_rand->length > 0) {
            data->rand.value = (uint8_t*)malloc(_rand->length);
	    data->rand.length = 0;
	    if (data->rand.value) {
                memcpy(data->rand.value, _rand->value, _rand->length);
                data->rand.length = _rand->length;
	    }
	}
	/* Set the authentication token */
	if (autn->length > 0) {
	    data->autn.value = (uint8_t*)malloc(autn->length);
	    data->autn.length = 0;
	    if (data->autn.value) {
		memcpy(data->autn.value, autn->value, autn->length);
		data->autn.length = autn->length;
	    }
	}
	/* Set the failure notification indicator */
	data->notify_failure = FALSE;
	/* Send authentication request message to the UE */
	rc = _authentication_request(data);
	if (rc != RETURNerror) {
	    /*
	     * Notify EMM that common procedure has been initiated
	     */
	    emm_sap_t emm_sap;
	    emm_sap.primitive = EMMREG_COMMON_PROC_REQ;
	    emm_sap.u.emm_reg.ueid = ueid;
	    rc = emm_sap_send(&emm_sap);
	}
    }

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 emm_proc_authentication_complete()                        **
 **                                                                        **
 ** Description: Performs the authentication completion procedure executed **
 **		 by the network.                                           **
 **                                                                        **
 **              3GPP TS 24.301, section 5.4.2.4                           **
 **		 Upon receiving the AUTHENTICATION RESPONSE message, the   **
 **		 MME shall stop timer T3460 and check the correctness of   **
 **		 the RES parameter.                                        **
 **                                                                        **
 ** Inputs:	 ueid:		UE lower layer identifier                  **
 **		 emm_cause:	Authentication failure EMM cause code      **
 **		 res:		Authentication response parameter          **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	_emm_data, T3460                           **
 **                                                                        **
 ***************************************************************************/
int emm_proc_authentication_complete(unsigned int ueid, int emm_cause,
				     const OctetString* res)
{
    LOG_FUNC_IN;

    int rc;
    emm_sap_t emm_sap;

    LOG_TRACE(INFO, "EMM-PROC  - Authentication complete (ueid=%u, cause=%d)",
	      ueid, emm_cause);

    /* Stop timer T3460 */
    LOG_TRACE(INFO, "EMM-PROC  - Stop timer T3460 (%d)", T3460.id);
    T3460.id = nas_timer_stop(T3460.id);

    /* Release retransmission timer paramaters */
    authentication_data_t* data =
	(authentication_data_t*)(emm_proc_common_get_args(ueid));
    if (data) {
	if (data->rand.length > 0) {
	    free(data->rand.value);
	}
	if (data->autn.length > 0) {
	    free(data->autn.value);
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

    if (emm_cause == EMM_CAUSE_SUCCESS) {
	/* Check the received RES parameter */
	if ( (emm_ctx == NULL) ||
	     (memcmp(res->value, &emm_ctx->vector.xres, res->length) != 0) ) {
	    /* RES does not match the XRES parameter */
	    LOG_TRACE(WARNING, "EMM-PROC  - Failed to authentify the UE");
	    emm_cause = EMM_CAUSE_ILLEGAL_UE;
	}
    }

    if (emm_cause != EMM_CAUSE_SUCCESS) {
	/* The MME received an authentication failure message or the RES
	 * contained in the Authentication Response message received from
	 * the UE does not match the XRES parameter computed by the network */
	(void) _authentication_reject(ueid);
	/*
	 * Notify EMM that the authentication procedure failed
	 */
	emm_sap.primitive = EMMREG_COMMON_PROC_REJ;
	emm_sap.u.emm_reg.ueid = ueid;
    }
    else {
	/*
	 * Notify EMM that the authentication procedure successfully completed
	 */
	emm_sap.primitive = EMMREG_COMMON_PROC_CNF;
	emm_sap.u.emm_reg.ueid = ueid;
	emm_sap.u.emm_reg.u.common.is_attached = emm_ctx->is_attached;
    }

    rc = emm_sap_send(&emm_sap);

    LOG_FUNC_RETURN (rc);
}
#endif // NAS_MME

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
 ** Name:	 _authentication_t3416_handler()                           **
 **                                                                        **
 ** Description: T3416 timeout handler                                     **
 **		 Upon T3416 timer expiration, the RAND and RES values sto- **
 **		 red in the ME shall be deleted.                           **
 **                                                                        **
 **              3GPP TS 24.301, section 5.4.2.3                           **
 **                                                                        **
 ** Inputs:	 args:		handler parameters                         **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	None                                       **
 **		 Others:	T3416                                      **
 **                                                                        **
 ***************************************************************************/
static void* _authentication_t3416_handler(void* args)
{
    LOG_FUNC_IN;

    LOG_TRACE(WARNING, "EMM-PROC  - T3416 timer expired");

    /* Stop timer T3416 */
    T3416.id = nas_timer_stop(T3416.id);
    /* Delete previouly stored RAND and RES authentication data */
    (void) emm_proc_authentication_delete();

    LOG_FUNC_RETURN (NULL);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 _authentication_t3418_handler()                           **
 **                                                                        **
 ** Description: T3418 timeout handler                                     **
 **		 Upon T3418 timer expiration, the UE shall deem that the   **
 **		 source of the  authentication  challenge is not  genuine  **
 **		 (authentication not accepted by the UE).                  **
 **                                                                        **
 **              3GPP TS 24.301, section 5.4.2.7, case c                   **
 **                                                                        **
 ** Inputs:	 args:		handler parameters                         **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	None                                       **
 **		 Others:	_authentication_data, T3418                **
 **                                                                        **
 ***************************************************************************/
static void* _authentication_t3418_handler(void* args)
{
    LOG_FUNC_IN;

    int rc;

    LOG_TRACE(WARNING, "EMM-PROC  - T3418 timer expired");

    /* Stop timer T3418 */
    T3418.id = nas_timer_stop(T3418.id);
    /* Reset the MAC failure and UMTS challenge failure counters */
    _authentication_data.mac_count = 0;
    _authentication_data.umts_count = 0;
    /* 3GPP TS 24.301, section 5.4.2.7, case f */
    rc = _authentication_abnormal_case_f();
    if (rc != RETURNok) {
	LOG_TRACE(WARNING, "EMM-PROC  - Failed to proceed abnormal case f");
    }

    LOG_FUNC_RETURN (NULL);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 _authentication_t3420_handler()                           **
 **                                                                        **
 ** Description: T3420 timeout handler                                     **
 **		 Upon T3420 timer expiration, the UE shall deem that the   **
 **		 network has failed the authentication check.              **
 **                                                                        **
 **              3GPP TS 24.301, section 5.4.2.7, case e                   **
 **                                                                        **
 ** Inputs:	 args:		handler parameters                         **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	None                                       **
 **		 Others:	_authentication_data, T3420                **
 **                                                                        **
 ***************************************************************************/
static void* _authentication_t3420_handler(void* args)
{
    LOG_FUNC_IN;

    int rc;

    LOG_TRACE(WARNING, "EMM-PROC  - T3420 timer expired");

    /* Stop timer T3420 */
    T3420.id = nas_timer_stop(T3420.id);
    /* Reset the sync failure counter */
    _authentication_data.sync_count = 0;
    /* 3GPP TS 24.301, section 5.4.2.7, case f */
    rc = _authentication_abnormal_case_f();
    if (rc != RETURNok) {
	LOG_TRACE(WARNING, "EMM-PROC  - Failed to proceed abnormal case f");
    }

    LOG_FUNC_RETURN (NULL);
}

/*
 * --------------------------------------------------------------------------
 *			    Abnormal cases in the UE
 * --------------------------------------------------------------------------
 */

/****************************************************************************
 **                                                                        **
 ** Name:	 _authentication_abnormal_cases_cde()                      **
 **                                                                        **
 ** Description: Performs the abnormal case authentication procedure.      **
 **                                                                        **
 **		 3GPP TS 24.301, section 5.4.2.7, cases c, d and e         **
 **                                                                        **
 ** Inputs:	 emm_cause:	EMM cause code                             **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	_authentication_data, T3418, T3420         **
 **                                                                        **
 ***************************************************************************/
static int _authentication_abnormal_cases_cde(int emm_cause,
					      const OctetString* auts)
{
    LOG_FUNC_IN;

    int rc;

    LOG_TRACE(WARNING, "EMM-PROC  - "
	      "Abnormal case, authentication counters c/d/e = %d/%d/%d",
	      _authentication_data.mac_count, _authentication_data.umts_count,
	      _authentication_data.sync_count);

    /*
     * Notify EMM-AS SAP that Authentication Failure message has to be sent
     * to the network
     */
    emm_sap_t emm_sap;
    emm_sap.primitive = EMMAS_SECURITY_RES;
    emm_sap.u.emm_as.u.security.guti = _emm_data.guti;
    emm_sap.u.emm_as.u.security.ueid = 0;
    emm_sap.u.emm_as.u.security.msgType = EMM_AS_MSG_TYPE_AUTH;
    emm_sap.u.emm_as.u.security.emm_cause = emm_cause;
    emm_sap.u.emm_as.u.security.auts = auts;
    /* Setup EPS NAS security data */
    emm_as_set_security_data(&emm_sap.u.emm_as.u.security.sctx,
			     _emm_data.security, FALSE, TRUE);
    rc = emm_sap_send(&emm_sap);

    if (rc != RETURNerror) {
	/*
	 * Update the authentication failure counters
	 */
	switch (emm_cause)
	{
	    case EMM_CAUSE_MAC_FAILURE:
		/* 3GPP TS 24.301, section 5.4.2.6, case c
		 * Update the MAC failure counter */
		_authentication_data.mac_count += 1;
		/* Start timer T3418 */
		T3418.id = nas_timer_start(T3418.sec,
				       _authentication_t3418_handler, NULL);
		LOG_TRACE(INFO,"EMM-PROC  - Timer T3418 (%d) expires in "
			  "%ld seconds", T3418.id, T3418.sec);
		break;

	    case EMM_CAUSE_NON_EPS_AUTH_UNACCEPTABLE:
		/* 3GPP TS 24.301, section 5.4.2.6, case d
		 * Update the UMTS challenge failure counter */
		_authentication_data.umts_count += 1;
		/* Start timer T3418 */
		T3418.id = nas_timer_start(T3418.sec,
				       _authentication_t3418_handler, NULL);
		LOG_TRACE(INFO,"EMM-PROC  - Timer T3418 (%d) expires in "
			  "%ld seconds", T3418.id, T3418.sec);
		break;

	    case EMM_CAUSE_SYNCH_FAILURE:
		/* 3GPP TS 24.301, section 5.4.2.6, case e
		 * Update the synch failure counter */
		_authentication_data.sync_count += 1;
		/* Start timer T3420 */
		T3420.id = nas_timer_start(T3420.sec,
				       _authentication_t3420_handler, NULL);
		LOG_TRACE(INFO,"EMM-PROC  - Timer T3420 (%d) expires in "
			  "%ld seconds", T3420.id, T3420.sec);
		break;

	    default:
		LOG_TRACE(WARNING, "EMM cause code is not valid (%d)",
			  emm_cause);
		LOG_FUNC_RETURN (RETURNerror);
	}
	/*
	 * Stop any retransmission timers that are running
	 */
	rc = _authentication_stop_timers();
	if (rc != RETURNok) {
	    LOG_TRACE(WARNING, "EMM-PROC  - "
		      "Failed to stop retransmission timers");
	    LOG_FUNC_RETURN (RETURNerror);
	}
	/*
	 * Check whether the network has failed the authentication check
	 */
	int failure_counter = 0;
	if (emm_cause == EMM_CAUSE_MAC_FAILURE) {
	    failure_counter = _authentication_data.mac_count
		+ _authentication_data.sync_count;
	} else if (emm_cause == EMM_CAUSE_SYNCH_FAILURE) {
	    failure_counter = _authentication_data.mac_count
		+ _authentication_data.umts_count
		+ _authentication_data.sync_count;
	}
	if (failure_counter >= AUTHENTICATION_COUNTER_MAX) {
	    /* 3GPP TS 24.301, section 5.4.2.6, case f */
	    rc = _authentication_abnormal_case_f();
	    if (rc != RETURNok) {
		LOG_TRACE(WARNING, "EMM-PROC  - "
			  "Failed to proceed abnormal case f");
	    }
	}
    }

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 _authentication_abnormal_case_f()                         **
 **                                                                        **
 ** Description: Performs the abnormal case authentication procedure.      **
 **                                                                        **
 **		 3GPP TS 24.301, section 5.4.2.7, case f                   **
 **                                                                        **
 ** Inputs:	 None                                                      **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
static int _authentication_abnormal_case_f(void)
{
    LOG_FUNC_IN;

    int rc;

    LOG_TRACE(WARNING, "EMM-PROC  - Authentication abnormal case f");

    /*
     * Request RRC to locally release the RRC connection and treat
     * the active cell as barred
     */
    emm_sap_t emm_sap;
    emm_sap.primitive = EMMAS_RELEASE_REQ;
    emm_sap.u.emm_as.u.release.guti = _emm_data.guti;
    emm_sap.u.emm_as.u.release.cause = EMM_AS_CAUSE_AUTHENTICATION;
    rc = emm_sap_send(&emm_sap);

    if (rc != RETURNerror) {
	/* Start any retransmission timers (e.g. T3410, T3417, T3421 or
	 * T3430), if they were running and stopped when the UE received
	 * the first AUTHENTICATION REQUEST message containing an invalid
	 * MAC or SQN */
	rc = _authentication_start_timers();
    }

    LOG_FUNC_RETURN (rc);
}

/*
 * --------------------------------------------------------------------------
 *			    UE specific local functions
 * --------------------------------------------------------------------------
 */

/****************************************************************************
 **                                                                        **
 ** Name:	 _authentication_stop_timers()                             **
 **                                                                        **
 ** Description: Stops any retransmission timers (e.g. T3410, T3417, T3421 **
 **		 or T3430) that are running                                **
 **                                                                        **
 ** Inputs:	 None                                                      **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	_authentication_data, T3410, T3417, T3421, **
 **				T3430                                      **
 **                                                                        **
 ***************************************************************************/
static int _authentication_stop_timers(void)
{
    LOG_FUNC_IN;

    /* Stop attach timer */
    if (T3410.id != NAS_TIMER_INACTIVE_ID) {
	LOG_TRACE(INFO, "EMM-PROC  - Stop timer T3410 (%d)", T3410.id);
	T3410.id = nas_timer_stop(T3410.id);
	_authentication_data.timers |= AUTHENTICATION_T3410;
    }
    /* Stop service request timer */
    if (T3417.id != NAS_TIMER_INACTIVE_ID) {
	LOG_TRACE(INFO, "EMM-PROC  - Stop timer T3417 (%d)", T3417.id);
	T3417.id = nas_timer_stop(T3417.id);
	_authentication_data.timers |= AUTHENTICATION_T3417;
    }
    /* Stop detach timer */
    if (T3421.id != NAS_TIMER_INACTIVE_ID) {
	LOG_TRACE(INFO, "EMM-PROC  - Stop timer T3421 (%d)", T3421.id);
	T3421.id = nas_timer_stop(T3421.id);
	_authentication_data.timers |= AUTHENTICATION_T3421;
    }
    /* Stop tracking area update timer */
    if (T3430.id != NAS_TIMER_INACTIVE_ID) {
	LOG_TRACE(INFO, "EMM-PROC  - Stop timer T3430 (%d)", T3430.id);
	T3430.id = nas_timer_stop(T3430.id);
	_authentication_data.timers |= AUTHENTICATION_T3430;
    }

    LOG_FUNC_RETURN (RETURNok);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 _authentication_start_timers()                            **
 **                                                                        **
 ** Description: Starts any retransmission timers (e.g. T3410, T3417,      **
 **		 T3421 or T3430), if they were running and stopped when    **
 **		 the UE received the first AUTHENTICATION REQUEST message  **
 **		 containing an invalid MAC or SQN                          **
 **                                                                        **
 **		 3GPP TS 24.301, section 5.4.2.7, case f                   **
 **                                                                        **
 ** Inputs:	 None                                                      **
 **		 Others:	_authentication_data                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	T3410, T3417, T3421, T3430                 **
 **                                                                        **
 ***************************************************************************/
static int _authentication_start_timers(void)
{
    LOG_FUNC_IN;

    if (_authentication_data.timers & AUTHENTICATION_T3410) {
	/* Start attach timer */
	T3410.id = nas_timer_start(T3410.sec, _emm_attach_t3410_handler, NULL);
	LOG_TRACE(INFO,"EMM-PROC  - Timer T3410 (%d) expires in "
		  "%ld seconds", T3410.id, T3410.sec);
    }
    if (_authentication_data.timers & AUTHENTICATION_T3417) {
	/* Start service request timer */
	T3417.id = nas_timer_start(T3417.sec, _emm_service_t3417_handler, NULL);
	LOG_TRACE(INFO,"EMM-PROC  - Timer T3417 (%d) expires in "
		  "%ld seconds", T3417.id, T3417.sec);
    }
    if (_authentication_data.timers & AUTHENTICATION_T3421) {
	/* Start detach timer */
	T3421.id = nas_timer_start(T3421.sec, _emm_detach_t3421_handler, NULL);
	LOG_TRACE(INFO,"EMM-PROC  - Timer T3421 (%d) expires in "
		  "%ld seconds", T3421.id, T3421.sec);
    }
    if (_authentication_data.timers & AUTHENTICATION_T3430) {
	/* Start tracking area update timer */
	T3430.id = nas_timer_start(T3430.sec, _emm_tau_t3430_handler, NULL);
	LOG_TRACE(INFO,"EMM-PROC  - Timer T3430 (%d) expires in "
		  "%ld seconds", T3430.id, T3430.sec);
    }

    LOG_FUNC_RETURN (RETURNok);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 _authentication_kasme()                                   **
 **                                                                        **
 ** Description: Computes the Key Access Security Management Entity Kasme  **
 **		 from the provided authentication challenge data.          **
 **                                                                        **
 **              3GPP TS 33.401, Annex A.2                                 **
 **                                                                        **
 ** Inputs:	 autn:		Authentication token                       **
 **		 ck:		Cipherig key                               **
 **		 ik:		Integrity key                              **
 **		 plmn:		Identifier of the currently selected PLMN  **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 kasme:		Key Access Security Management Entity      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
static int _authentication_kasme(const OctetString* autn,
				 const OctetString* ck, const OctetString* ik,
				 const plmn_t* plmn, OctetString* kasme)
{
    LOG_FUNC_IN;

    /* Compute the derivation key KEY = CK || IK */
    UInt8_t key[ck->length + ik->length];
    memcpy(key, ck->value, ck->length);
    memcpy(key + ck->length, ik->value, ik->length);

    /* Compute the KDF input parameter
     * S = FC(0x10) || SNid || 0x00 0x03 || SQN ⊕ AK || 0x00 0x06
     */
    UInt8_t input[kasme->length];
    UInt16_t length;
    int offset = 0;
    int size_of_length = sizeof(length);
    input[offset] = 0x10; offset += 1;
    length = AUTH_SNID_SIZE;
    memcpy(input + offset, plmn, length); offset += length;
    memcpy(input + offset, &length, size_of_length); offset += size_of_length;
    length = AUTH_SQN_SIZE;
    memcpy(input + offset, autn, length); offset += length;
    memcpy(input + offset, &length, size_of_length);

    /* TODO !!! Compute the Kasme key */
    // todo_hmac_256(key, input, kasme->value);

    LOG_FUNC_RETURN (RETURNok);
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
 ** Name:	 _authentication_t3460_handler()                           **
 **                                                                        **
 ** Description: T3460 timeout handler                                     **
 **		 Upon T3460 timer expiration, the authentication request   **
 **		 message is retransmitted and the timer restarted. When    **
 **		 retransmission counter is exceed, the MME shall abort the **
 **		 authentication procedure and any ongoing EMM specific     **
 **		 procedure and release the NAS signalling connection.      **
 **                                                                        **
 **              3GPP TS 24.301, section 5.4.2.7, case b                   **
 **                                                                        **
 ** Inputs:	 args:		handler parameters                         **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	None                                       **
 **		 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
static void* _authentication_t3460_handler(void* args)
{
    LOG_FUNC_IN;

    int rc;

    authentication_data_t* data = (authentication_data_t*)(args);

    /* Increment the retransmission counter */
    data->retransmission_count += 1;

    LOG_TRACE(WARNING, "EMM-PROC  - T3460 timer expired, retransmission "
	      "counter = %d", data->retransmission_count);

    if (data->retransmission_count < AUTHENTICATION_COUNTER_MAX) {
	/* Send authentication request message to the UE */
	rc = _authentication_request(data);
    }
    else {
	unsigned int ueid = data->ueid;
	/* Set the failure notification indicator */
	data->notify_failure = TRUE;
	/* Abort the authentication procedure */
	rc = _authentication_abort(data);
	/* Release the NAS signalling connection */
	if (rc != RETURNerror) {
	    emm_sap_t emm_sap;
	    emm_sap.primitive = EMMAS_RELEASE_REQ;
	    emm_sap.u.emm_as.u.release.guti = NULL;
	    emm_sap.u.emm_as.u.release.ueid = ueid;
	    emm_sap.u.emm_as.u.release.cause = EMM_AS_CAUSE_AUTHENTICATION;
	    rc = emm_sap_send(&emm_sap);
	}
    }

    LOG_FUNC_RETURN (NULL);
}

/*
 * --------------------------------------------------------------------------
 *			    MME specific local functions
 * --------------------------------------------------------------------------
 */

/****************************************************************************
 **                                                                        **
 ** Name:	 _authentication_request()                                 **
 **                                                                        **
 ** Description: Sends AUTHENTICATION REQUEST message and start timer T3460**
 **                                                                        **
 ** Inputs:	 args:		handler parameters                         **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	T3460                                      **
 **                                                                        **
 ***************************************************************************/
int _authentication_request(authentication_data_t* data)
{
    LOG_FUNC_IN;

    emm_sap_t emm_sap;
    int rc;
    struct emm_data_context_s *emm_ctx;

    /*
     * Notify EMM-AS SAP that Authentication Request message has to be sent
     * to the UE
     */
    emm_sap.primitive = EMMAS_SECURITY_REQ;
    emm_sap.u.emm_as.u.security.guti = NULL;
    emm_sap.u.emm_as.u.security.ueid = data->ueid;
    emm_sap.u.emm_as.u.security.msgType = EMM_AS_MSG_TYPE_AUTH;
    emm_sap.u.emm_as.u.security.ksi = data->ksi;
    emm_sap.u.emm_as.u.security.rand = &data->rand;
    emm_sap.u.emm_as.u.security.autn = &data->autn;

    /* TODO: check for pointer validity */
#if defined(EPC_BUILD)
    emm_ctx = emm_data_context_get(&_emm_data, data->ueid);
#else
    emm_ctx = _emm_data.ctx[data->ueid];
#endif

    /* Setup EPS NAS security data */
    emm_as_set_security_data(&emm_sap.u.emm_as.u.security.sctx,
                             emm_ctx->security, FALSE, TRUE);
    rc = emm_sap_send(&emm_sap);

    if (rc != RETURNerror) {
	if (T3460.id != NAS_TIMER_INACTIVE_ID) {
	    /* Re-start T3460 timer */
	    T3460.id = nas_timer_restart(T3460.id);
	} else {
	    /* Start T3460 timer */
	    T3460.id = nas_timer_start(T3460.sec, _authentication_t3460_handler,
				   data);
	}
	LOG_TRACE(INFO,"EMM-PROC  - Timer T3460 (%d) expires in %ld seconds",
		  T3460.id, T3460.sec);
    }

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 _authentication_reject()                                  **
 **                                                                        **
 ** Description: Sends AUTHENTICATION REJECT message                       **
 **                                                                        **
 ** Inputs:	 ueid:		UE lower layer identifier                  **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	None                                       **
 **                                                                        **
 ***************************************************************************/
static int _authentication_reject(unsigned int ueid)
{
    LOG_FUNC_IN;

    emm_sap_t emm_sap;
    int rc;
    struct emm_data_context_s *emm_ctx;

    /*
     * Notify EMM-AS SAP that Authentication Reject message has to be sent
     * to the UE
     */
    emm_sap.primitive = EMMAS_SECURITY_REJ;
    emm_sap.u.emm_as.u.security.guti = NULL;
    emm_sap.u.emm_as.u.security.ueid = ueid;
    emm_sap.u.emm_as.u.security.msgType = EMM_AS_MSG_TYPE_AUTH;

#if defined(EPC_BUILD)
    emm_ctx = emm_data_context_get(&_emm_data, ueid);
#else
    emm_ctx = _emm_data.ctx[ueid];
#endif
    /* Setup EPS NAS security data */
    emm_as_set_security_data(&emm_sap.u.emm_as.u.security.sctx,
                             emm_ctx->security, FALSE, TRUE);
    rc = emm_sap_send(&emm_sap);

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:	 _authentication_abort()                                   **
 **                                                                        **
 ** Description: Aborts the authentication procedure currently in progress **
 **                                                                        **
 ** Inputs:	 args:		Authentication data to be released         **
 **		 Others:	None                                       **
 **                                                                        **
 ** Outputs:	 None                                                      **
 **		 Return:	RETURNok, RETURNerror                      **
 **		 Others:	T3460                                      **
 **                                                                        **
 ***************************************************************************/
static int _authentication_abort(void* args)
{
    LOG_FUNC_IN;

    int rc = RETURNerror;

    authentication_data_t* data = (authentication_data_t*)(args);

    if (data)
    {
	unsigned int ueid = data->ueid;
	int notify_failure = data->notify_failure;

	LOG_TRACE(WARNING, "EMM-PROC  - Abort authentication procedure "
		  "(ueid=%u)", ueid);

	/* Stop timer T3460 */
	if (T3460.id != NAS_TIMER_INACTIVE_ID) {
	    LOG_TRACE(INFO, "EMM-PROC  - Stop timer T3460 (%d)", T3460.id);
	    T3460.id = nas_timer_stop(T3460.id);
	}
	/* Release retransmission timer paramaters */
	if (data->rand.length > 0) {
	    free(data->rand.value);
	}
	if (data->autn.length > 0) {
	    free(data->autn.value);
	}
	free(data);

	/*
	 * Notify EMM that the authentication procedure failed
	 */
	if (notify_failure) {
	    emm_sap_t emm_sap;
	    emm_sap.primitive = EMMREG_COMMON_PROC_REJ;
	    emm_sap.u.emm_reg.ueid = ueid;
	    rc = emm_sap_send(&emm_sap);
	} else {
	    rc = RETURNok;
	}
    }

    LOG_FUNC_RETURN (rc);
}

#endif // NAS_MME

