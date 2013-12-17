/*****************************************************************************
            Eurecom OpenAirInterface 3
            Copyright(c) 2012 Eurecom

Source      Identification.c

Version     0.1

Date        2013/04/09

Product     NAS stack

Subsystem   EPS Mobility Management

Author      Frederic Maurel

Description Defines the identification EMM procedure executed by the
        Non-Access Stratum.

        The identification procedure is used by the network to request
        a particular UE to provide specific identification parameters
        (IMSI, IMEI).

*****************************************************************************/

#include "emm_proc.h"
#include "nas_log.h"
#include "nas_timer.h"

#include "emmData.h"

#include "emm_sap.h"

#include <stdlib.h> // malloc, free
#include <string.h> // memcpy

/****************************************************************************/
/****************  E X T E R N A L    D E F I N I T I O N S  ****************/
/****************************************************************************/

/****************************************************************************/
/*******************  L O C A L    D E F I N I T I O N S  *******************/
/****************************************************************************/

/* String representation of the requested identity type */
static const char *_emm_identity_type_str[] = {
    "NOT AVAILABLE", "IMSI", "IMEI", "IMEISV", "TMSI"
};

/*
 * --------------------------------------------------------------------------
 *  Internal data handled by the identification procedure in the UE
 * --------------------------------------------------------------------------
 */
#ifdef NAS_UE
#endif  // NAS_UE

/*
 * --------------------------------------------------------------------------
 *  Internal data handled by the identification procedure in the MME
 * --------------------------------------------------------------------------
 */
#ifdef NAS_MME
/*
 * Timer handlers
 */
static void *_identification_t3470_handler(void *);

/*
 * Function executed whenever the ongoing EMM procedure that initiated
 * the identification procedure is aborted or the maximum value of the
 * retransmission timer counter is exceed
 */
static int _identification_abort(void *);

/*
 * Internal data used for identification procedure
 */
typedef struct {
    unsigned int ueid;          /* UE identifier        */
#define IDENTIFICATION_COUNTER_MAX  5
    unsigned int retransmission_count;  /* Retransmission counter   */
    emm_proc_identity_type_t type;  /* Type of UE identity      */
    int notify_failure;         /* Indicates whether the identification
                     * procedure failure shall be notified
                     * to the ongoing EMM procedure */
} identification_data_t;

static int _identification_request(identification_data_t *data);
#endif // NAS_MME

/****************************************************************************/
/******************  E X P O R T E D    F U N C T I O N S  ******************/
/****************************************************************************/

/*
 * --------------------------------------------------------------------------
 *      Identification procedure executed by the UE
 * --------------------------------------------------------------------------
 */
#ifdef NAS_UE
/****************************************************************************
 **                                                                        **
 ** Name:    emm_proc_identification_request()                         **
 **                                                                        **
 ** Description: Performs the MME requested identification procedure.      **
 **                                                                        **
 **              3GPP TS 24.301, section 5.4.4.3                           **
 **      Upon receiving the IDENTITY REQUEST message, the UE shall **
 **      send  an  IDENTITY  RESPONSE  message to the network. The **
 **      IDENTITY RESPONSE message shall contain the identifica-   **
 **      tion parameters as requested by the network.              **
 **                                                                        **
 ** Inputs:  type:      Type of the requested identity             **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int emm_proc_identification_request(emm_proc_identity_type_t type)
{
    LOG_FUNC_IN;

    int rc;
    emm_sap_t emm_sap;

    LOG_TRACE(INFO, "EMM-PROC  - Identification requested type = %s (%d)",
              _emm_identity_type_str[type], type);

    /* Setup EMM procedure handler to be executed upon receiving
     * lower layer notification */
    rc = emm_proc_lowerlayer_initialize(NULL, NULL, NULL, NULL);
    if (rc != RETURNok) {
        LOG_TRACE(WARNING,
                  "EMM-PROC  - Failed to initialize EMM procedure handler");
        LOG_FUNC_RETURN (RETURNerror);
    }

    emm_sap.u.emm_as.u.security.identType = EMM_IDENT_TYPE_NOT_AVAILABLE;

    switch (type) {
        case EMM_IDENT_TYPE_IMSI:
            /* International Mobile Subscriber Identity is requested */
            if (_emm_data.imsi) {
                emm_sap.u.emm_as.u.security.identType = EMM_IDENT_TYPE_IMSI;
                emm_sap.u.emm_as.u.security.imsi = _emm_data.imsi;
            }
            break;

        case EMM_IDENT_TYPE_IMEI:
            /* International Mobile Equipment Identity is requested */
            if (_emm_data.imei) {
                emm_sap.u.emm_as.u.security.identType = EMM_IDENT_TYPE_IMEI;
                emm_sap.u.emm_as.u.security.imei = _emm_data.imei;
            }
            break;

        case EMM_IDENT_TYPE_TMSI:
            /* Temporary Mobile Subscriber Identity is requested */
            if (_emm_data.guti) {
                emm_sap.u.emm_as.u.security.identType = EMM_IDENT_TYPE_TMSI;
                emm_sap.u.emm_as.u.security.tmsi = _emm_data.guti->m_tmsi;
            }
            break;

        default:
            /* Other identities are not available */
            break;
    }

    /*
     * Notify EMM-AS SAP that Identity Response message has to be sent
     * to the MME
     */
    emm_sap.primitive = EMMAS_SECURITY_RES;
    emm_sap.u.emm_as.u.security.guti = _emm_data.guti;
    emm_sap.u.emm_as.u.security.ueid = 0;
    emm_sap.u.emm_as.u.security.msgType = EMM_AS_MSG_TYPE_IDENT;
    /* Setup EPS NAS security data */
    emm_as_set_security_data(&emm_sap.u.emm_as.u.security.sctx,
                             _emm_data.security, FALSE, TRUE);
    rc = emm_sap_send(&emm_sap);

    LOG_FUNC_RETURN (rc);
}
#endif // NAS_UE

/*
 * --------------------------------------------------------------------------
 *      Identification procedure executed by the MME
 * --------------------------------------------------------------------------
 */
#ifdef NAS_MME
/****************************************************************************
 **                                                                        **
 ** Name:    emm_proc_identification()                                 **
 **                                                                        **
 ** Description: Initiates an identification procedure.                    **
 **                                                                        **
 **              3GPP TS 24.301, section 5.4.4.2                           **
 **      The network initiates the identification procedure by     **
 **      sending an IDENTITY REQUEST message to the UE and star-   **
 **      ting the timer T3470. The IDENTITY REQUEST message speci- **
 **      fies the requested identification parameters in the Iden- **
 **      tity type information element.                            **
 **                                                                        **
 ** Inputs:  ueid:      UE lower layer identifier                  **
 **      type:      Type of the requested identity             **
 **      success:   Callback function executed when the identi-**
 **             fication procedure successfully completes  **
 **      reject:    Callback function executed when the identi-**
 **             fication procedure fails or is rejected    **
 **      failure:   Callback function executed whener a lower  **
 **             layer failure occured before the identifi- **
 **             cation procedure comnpletes                **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    _emm_data                                  **
 **                                                                        **
 ***************************************************************************/
int emm_proc_identification(unsigned int                   ueid,
                            emm_data_context_t            *emm_ctx,
                            emm_proc_identity_type_t       type,
                            emm_common_success_callback_t  success,
                            emm_common_reject_callback_t   reject,
                            emm_common_failure_callback_t  failure)
{
    LOG_FUNC_IN;

    int rc = RETURNerror;

    LOG_TRACE(INFO, "EMM-PROC  - Initiate identification type = %s (%d), ctx = %p",
              _emm_identity_type_str[type], type, emm_ctx);

    /* Allocate parameters of the retransmission timer callback */
    identification_data_t *data =
        (identification_data_t *)malloc(sizeof(identification_data_t));

    if (data != NULL) {
        /* Setup ongoing EMM procedure callback functions */
        rc = emm_proc_common_initialize(ueid, success, reject, failure,
                                        _identification_abort, data);
        if (rc != RETURNok) {
            LOG_TRACE(WARNING, "Failed to initialize EMM callback functions");
            free(data);
            LOG_FUNC_RETURN (RETURNerror);
        }
        /* Set the UE identifier */
        data->ueid = ueid;
        /* Reset the retransmission counter */
        data->retransmission_count = 0;
        /* Set the type of the requested identity */
        data->type = type;
        /* Set the failure notification indicator */
        data->notify_failure = FALSE;
        /* Send identity request message to the UE */
        rc = _identification_request(data);
        if (rc != RETURNerror) {
            /*
             * Notify EMM that common procedure has been initiated
             */
            emm_sap_t emm_sap;
            emm_sap.primitive = EMMREG_COMMON_PROC_REQ;
            emm_sap.u.emm_reg.ueid = ueid;
            emm_sap.u.emm_reg.ctx  = emm_ctx;
            rc = emm_sap_send(&emm_sap);
        }
    }

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:    emm_proc_identification_complete()                        **
 **                                                                        **
 ** Description: Performs the identification completion procedure executed **
 **      by the network.                                           **
 **                                                                        **
 **              3GPP TS 24.301, section 5.4.4.4                           **
 **      Upon receiving the IDENTITY RESPONSE message, the MME     **
 **      shall stop timer T3470.                                   **
 **                                                                        **
 ** Inputs:  ueid:      UE lower layer identifier                  **
 **      imsi:      The IMSI received from the UE              **
 **      imei:      The IMEI received from the UE              **
 **      tmsi:      The TMSI received from the UE              **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    _emm_data, T3470                           **
 **                                                                        **
 ***************************************************************************/
int emm_proc_identification_complete(unsigned int ueid, const imsi_t *imsi,
                                     const imei_t *imei, UInt32_t *tmsi)
{
    int rc = RETURNerror;
    emm_sap_t emm_sap;

    emm_data_context_t *emm_ctx = NULL;

    LOG_FUNC_IN;

    LOG_TRACE(INFO, "EMM-PROC  - Identification complete (ueid=%u)", ueid);

    /* Stop timer T3470 */
    LOG_TRACE(INFO, "EMM-PROC  - Stop timer T3470 (%d)", T3470.id);
    T3470.id = nas_timer_stop(T3470.id);

    /* Release retransmission timer paramaters */
    identification_data_t *data =
        (identification_data_t *)(emm_proc_common_get_args(ueid));
    if (data) {
        free(data);
    }

    /* Get the UE context */
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
        if (imsi) {
            /* Update the IMSI */
            if (emm_ctx->imsi == NULL) {
                emm_ctx->imsi = (imsi_t *)malloc(sizeof(imsi_t));
            }
            if (emm_ctx->imsi) {
                memcpy(emm_ctx->imsi, imsi, sizeof(imsi_t));
            }
        } else if (imei) {
            /* Update the IMEI */
            if (emm_ctx->imei == NULL) {
                emm_ctx->imei = (imei_t *)malloc(sizeof(imei_t));
            }
            if (emm_ctx->imei) {
                memcpy(emm_ctx->imei, imei, sizeof(imei_t));
            }
        } else if (tmsi) {
            /* Update the GUTI */
            if (emm_ctx->guti == NULL) {
                emm_ctx->guti = (GUTI_t *)malloc(sizeof(GUTI_t));
            }
            if (emm_ctx->guti) {
                memcpy(&emm_ctx->guti->gummei,
                       &_emm_data.conf.gummei, sizeof(gummei_t));
                emm_ctx->guti->m_tmsi = *tmsi;
            }
        }
        /*
         * Notify EMM that the identification procedure successfully completed
         */
        emm_sap.primitive = EMMREG_COMMON_PROC_CNF;
        emm_sap.u.emm_reg.ueid = ueid;
        emm_sap.u.emm_reg.ctx  = emm_ctx;
        emm_sap.u.emm_reg.u.common.is_attached = emm_ctx->is_attached;
    } else {
        LOG_TRACE(ERROR, "EMM-PROC  - No EMM context exists");
        /*
         * Notify EMM that the identification procedure failed
         */
        emm_sap.primitive = EMMREG_COMMON_PROC_REJ;
        emm_sap.u.emm_reg.ueid = ueid;
        emm_sap.u.emm_reg.ctx  = emm_ctx;
    }

    rc = emm_sap_send(&emm_sap);

    LOG_FUNC_RETURN (rc);
}

#endif // NAS_MME

/****************************************************************************/
/*********************  L O C A L    F U N C T I O N S  *********************/
/****************************************************************************/

#ifdef NAS_MME
/*
 * --------------------------------------------------------------------------
 *              Timer handlers
 * --------------------------------------------------------------------------
 */

/****************************************************************************
 **                                                                        **
 ** Name:    _identification_t3470_handler()                           **
 **                                                                        **
 ** Description: T3470 timeout handler                                     **
 **      Upon T3470 timer expiration, the identification request   **
 **      message is retransmitted and the timer restarted. When    **
 **      retransmission counter is exceed, the MME shall abort the **
 **      identification procedure and any ongoing EMM procedure.   **
 **                                                                        **
 **              3GPP TS 24.301, section 5.4.4.6, case b                   **
 **                                                                        **
 ** Inputs:  args:      handler parameters                         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    None                                       **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static void *_identification_t3470_handler(void *args)
{
    LOG_FUNC_IN;

    int rc;

    identification_data_t *data = (identification_data_t *)(args);

    /* Increment the retransmission counter */
    data->retransmission_count += 1;

    LOG_TRACE(WARNING, "EMM-PROC  - T3470 timer expired, retransmission "
              "counter = %d", data->retransmission_count);

    if (data->retransmission_count < IDENTIFICATION_COUNTER_MAX) {
        /* Send identity request message to the UE */
        rc = _identification_request(data);
    } else {
        /* Set the failure notification indicator */
        data->notify_failure = TRUE;
        /* Abort the identification procedure */
        rc = _identification_abort(data);
    }

    LOG_FUNC_RETURN (NULL);
}

/*
 * --------------------------------------------------------------------------
 *              MME specific local functions
 * --------------------------------------------------------------------------
 */

/****************************************************************************
 **                                                                        **
 ** Name:    _identification_request()                                 **
 **                                                                        **
 ** Description: Sends IDENTITY REQUEST message and start timer T3470.     **
 **                                                                        **
 ** Inputs:  args:      handler parameters                         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    None                                       **
 **      Others:    T3470                                      **
 **                                                                        **
 ***************************************************************************/
int _identification_request(identification_data_t *data)
{
    emm_sap_t emm_sap;
    int rc;

    struct emm_data_context_s *emm_ctx = NULL;

    LOG_FUNC_IN;

    /*
     * Notify EMM-AS SAP that Identity Request message has to be sent
     * to the UE
     */
    emm_sap.primitive = EMMAS_SECURITY_REQ;
    emm_sap.u.emm_as.u.security.guti = NULL;
    emm_sap.u.emm_as.u.security.ueid = data->ueid;
    emm_sap.u.emm_as.u.security.msgType = EMM_AS_MSG_TYPE_IDENT;
    emm_sap.u.emm_as.u.security.identType = data->type;

#if defined(EPC_BUILD)
    if (data->ueid > 0) {
        emm_ctx = emm_data_context_get(&_emm_data, data->ueid);
    }
#else
    if (data->ueid < EMM_DATA_NB_UE_MAX) {
        emm_ctx = _emm_data.ctx[data->ueid];
    }
#endif
    /* Setup EPS NAS security data */
    emm_as_set_security_data(&emm_sap.u.emm_as.u.security.sctx,
                             emm_ctx->security, FALSE, TRUE);
    rc = emm_sap_send(&emm_sap);

    if (rc != RETURNerror) {
        if (T3470.id != NAS_TIMER_INACTIVE_ID) {
            /* Re-start T3470 timer */
            T3470.id = nas_timer_restart(T3470.id);
        } else {
            /* Start T3470 timer */
            T3470.id = nas_timer_start(T3470.sec, _identification_t3470_handler,
                                       data);
        }
        LOG_TRACE(INFO,"EMM-PROC  - Timer T3470 (%d) expires in %ld seconds",
                  T3470.id, T3470.sec);
    }

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:    _identification_abort()                                   **
 **                                                                        **
 ** Description: Aborts the identification procedure currently in progress **
 **                                                                        **
 ** Inputs:  args:      Identification data to be released         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    None                                       **
 **      Others:    T3470                                      **
 **                                                                        **
 ***************************************************************************/
static int _identification_abort(void *args)
{
    LOG_FUNC_IN;

    int rc = RETURNerror;

    identification_data_t *data = (identification_data_t *)(args);

    if (data) {
        unsigned int ueid = data->ueid;
        int notify_failure = data->notify_failure;

        LOG_TRACE(WARNING, "EMM-PROC  - Abort identification procedure "
                  "(ueid=%u)", ueid);

        /* Stop timer T3470 */
        if (T3470.id != NAS_TIMER_INACTIVE_ID) {
            LOG_TRACE(INFO, "EMM-PROC  - Stop timer T3470 (%d)", T3470.id);
            T3470.id = nas_timer_stop(T3470.id);
        }
        /* Release retransmission timer paramaters */
        free(data);

        /*
         * Notify EMM that the identification procedure failed
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

    LOG_FUNC_RETURN(rc);
}

#endif // NAS_MME

