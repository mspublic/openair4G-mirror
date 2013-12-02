/*****************************************************************************
            Eurecom OpenAirInterface 3
            Copyright(c) 2012 Eurecom

Source      DefaultEpsBearerContextActivation.c

Version     0.1

Date        2013/01/28

Product     NAS stack

Subsystem   EPS Session Management

Author      Frederic Maurel

Description Defines the default EPS bearer context activation ESM
        procedure executed by the Non-Access Stratum.

        The purpose of the default bearer context activation procedure
        is to establish a default EPS bearer context between the UE
        and the EPC.

        The procedure is initiated by the network as a response to
        the PDN CONNECTIVITY REQUEST message received from the UE.
        It can be part of the attach procedure.

*****************************************************************************/

#include "esm_proc.h"
#include "commonDef.h"
#include "nas_log.h"

#include "esm_cause.h"
#include "esm_ebr.h"
#include "esm_ebr_context.h"

#include "emm_sap.h"

/****************************************************************************/
/****************  E X T E R N A L    D E F I N I T I O N S  ****************/
/****************************************************************************/

/****************************************************************************/
/*******************  L O C A L    D E F I N I T I O N S  *******************/
/****************************************************************************/

/*
 * --------------------------------------------------------------------------
 * Internal data handled by the default EPS bearer context activation
 * procedure in the UE
 * --------------------------------------------------------------------------
 */
#ifdef NAS_UE
static struct {
    int ebi;    /* EPS bearer identity of the default EPS bearer associated
         * to the PDN connection to be activated */
} _default_eps_bearer_context_data = {ESM_EBI_UNASSIGNED};

#endif // NAS_UE

/*
 * --------------------------------------------------------------------------
 * Internal data handled by the default EPS bearer context activation
 * procedure in the MME
 * --------------------------------------------------------------------------
 */
#ifdef NAS_MME
/*
 * Timer handlers
 */
static void *_default_eps_bearer_activate_t3485_handler(void *);

/* Maximum value of the activate default EPS bearer context request
 * retransmission counter */
#define DEFAULT_EPS_BEARER_ACTIVATE_COUNTER_MAX 5

static int _default_eps_bearer_activate(unsigned int ueid, int ebi,
                                        const OctetString *msg);
#endif // NAS_MME

/****************************************************************************/
/******************  E X P O R T E D    F U N C T I O N S  ******************/
/****************************************************************************/

/*
 * --------------------------------------------------------------------------
 *    Default EPS bearer context activation procedure executed by the MME
 * --------------------------------------------------------------------------
 */
#ifdef NAS_MME
/****************************************************************************
 **                                                                        **
 ** Name:    esm_proc_default_eps_bearer_context()                     **
 **                                                                        **
 ** Description: Allocates resources required for activation of a default  **
 **      EPS bearer context.                                       **
 **                                                                        **
 ** Inputs:  ueid:      UE local identifier                        **
 **          pid:       PDN connection identifier                  **
 **      qos:       EPS bearer level QoS parameters            **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     ebi:       EPS bearer identity assigned to the de-    **
 **             fault EPS bearer context                   **
 **      esm_cause: Cause code returned upon ESM procedure     **
 **             failure                                    **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int esm_proc_default_eps_bearer_context(unsigned int ueid, int pid,
                                        unsigned int *ebi,
                                        const esm_proc_qos_t *qos,
                                        int *esm_cause)
{
    LOG_FUNC_IN;

    LOG_TRACE(INFO, "ESM-PROC  - Default EPS bearer context activation "
              "(ueid=%u, pid=%d)", ueid, pid);

    /* Assign new EPS bearer context */
    *ebi = esm_ebr_assign(ueid, ESM_EBI_UNASSIGNED);

    if (*ebi != ESM_EBI_UNASSIGNED) {
        /* Create default EPS bearer context */
        *ebi = esm_ebr_context_create(ueid, pid, *ebi, TRUE, qos, NULL);
        if (*ebi == ESM_EBI_UNASSIGNED) {
            /* No resource available */
            LOG_TRACE(WARNING, "ESM-PROC  - Failed to create new default EPS "
                      "bearer context (ebi=%d)", *ebi);
            *esm_cause = ESM_CAUSE_INSUFFICIENT_RESOURCES;
            LOG_FUNC_RETURN (RETURNerror);
        }
        LOG_FUNC_RETURN (RETURNok);
    }

    LOG_TRACE(WARNING, "ESM-PROC  - Failed to assign new EPS bearer context");
    *esm_cause = ESM_CAUSE_INSUFFICIENT_RESOURCES;
    LOG_FUNC_RETURN (RETURNerror);
}

/****************************************************************************
 **                                                                        **
 ** Name:    esm_proc_default_eps_bearer_context_request()             **
 **                                                                        **
 ** Description: Initiates the default EPS bearer context activation pro-  **
 **      cedure                                                    **
 **                                                                        **
 **      3GPP TS 24.301, section 6.4.1.2                           **
 **      The MME initiates the default EPS bearer context activa-  **
 **      tion procedure by sending an ACTIVATE DEFAULT EPS BEARER  **
 **      CONTEXT REQUEST message, starting timer T3485 and ente-   **
 **      ring state BEARER CONTEXT ACTIVE PENDING.                 **
 **                                                                        **
 ** Inputs:  is_standalone: Indicate whether the default bearer is     **
 **             activated as part of the attach procedure  **
 **             or as the response to a stand-alone PDN    **
 **             CONNECTIVITY REQUEST message               **
 **      ueid:      UE lower layer identifier                  **
 **      ebi:       EPS bearer identity                        **
 **      msg:       Encoded ESM message to be sent             **
 **      ue_triggered:  TRUE if the EPS bearer context procedure   **
 **             was triggered by the UE                    **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int esm_proc_default_eps_bearer_context_request(int is_standalone,
        unsigned int ueid, int ebi,
        OctetString *msg, int ue_triggered)
{
    LOG_FUNC_IN;

    int rc = RETURNok;

    LOG_TRACE(INFO,"ESM-PROC  - Initiate default EPS bearer context activation "
              "(ueid=%d, ebi=%d)", ueid, ebi);

    if (is_standalone) {
        /* Send activate default EPS bearer context request message and
         * start timer T3485 */
        rc = _default_eps_bearer_activate(ueid, ebi, msg);
    }

    if (rc != RETURNerror) {
        /* Set the EPS bearer context state to ACTIVE PENDING */
        rc = esm_ebr_set_status(ueid, ebi, ESM_EBR_ACTIVE_PENDING, ue_triggered);
        if (rc != RETURNok) {
            /* The EPS bearer context was already in ACTIVE PENDING state */
            LOG_TRACE(WARNING, "ESM-PROC  - EBI %d was already ACTIVE PENDING",
                      ebi);
        }
    }

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:    esm_proc_default_eps_bearer_context_accept()              **
 **                                                                        **
 ** Description: Performs default EPS bearer context activation procedure  **
 **      accepted by the UE.                                       **
 **                                                                        **
 **      3GPP TS 24.301, section 6.4.1.3                           **
 **      Upon receipt of the ACTIVATE DEFAULT EPS BEARER CONTEXT   **
 **      ACCEPT message, the MME shall enter the state BEARER CON- **
 **      TEXT ACTIVE and stop the timer T3485, if it is running.   **
 **                                                                        **
 ** Inputs:  ueid:      UE local identifier                        **
 **      ebi:       EPS bearer identity                        **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     esm_cause: Cause code returned upon ESM procedure     **
 **             failure                                    **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int esm_proc_default_eps_bearer_context_accept(unsigned int ueid, int ebi,
        int *esm_cause)
{
    LOG_FUNC_IN;

    int rc;

    LOG_TRACE(INFO, "ESM-PROC  - Default EPS bearer context activation "
              "accepted by the UE (ueid=%u, ebi=%d)", ueid, ebi);

    /* Stop T3485 timer if running */
    rc = esm_ebr_stop_timer(ueid, ebi);
    if (rc != RETURNerror) {
        /* Set the EPS bearer context state to ACTIVE */
        rc = esm_ebr_set_status(ueid, ebi, ESM_EBR_ACTIVE, FALSE);
        if (rc != RETURNok) {
            /* The EPS bearer context was already in ACTIVE state */
            LOG_TRACE(WARNING, "ESM-PROC  - EBI %d was already ACTIVE", ebi);
            *esm_cause = ESM_CAUSE_PROTOCOL_ERROR;
        }
    }

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:    esm_proc_default_eps_bearer_context_reject()              **
 **                                                                        **
 ** Description: Performs default EPS bearer context activation procedure  **
 **      not accepted by the UE.                                   **
 **                                                                        **
 **      3GPP TS 24.301, section 6.4.1.4                           **
 **      Upon receipt of the ACTIVATE DEFAULT EPS BEARER CONTEXT   **
 **      REJECT message, the MME shall enter the state BEARER CON- **
 **      TEXT INACTIVE and stop the timer T3485, if it is running. **
 **                                                                        **
 ** Inputs:  ueid:      UE local identifier                        **
 **      ebi:       EPS bearer identity                        **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     esm_cause: Cause code returned upon ESM procedure     **
 **             failure                                    **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int esm_proc_default_eps_bearer_context_reject(unsigned int ueid, int ebi,
        int *esm_cause)
{
    LOG_FUNC_IN;

    int rc;

    LOG_TRACE(WARNING, "ESM-PROC  - Default EPS bearer context activation "
              "not accepted by the UE (ueid=%u, ebi=%d)", ueid, ebi);

    /* Stop T3485 timer if running */
    rc = esm_ebr_stop_timer(ueid, ebi);
    if (rc != RETURNerror) {
        int pid, bid;
        /* Release the default EPS bearer context and enter state INACTIVE */
        rc = esm_proc_eps_bearer_context_deactivate(ueid, TRUE, ebi,
                &pid, &bid, NULL);
        if (rc != RETURNok) {
            /* Failed to release the default EPS bearer context */
            *esm_cause = ESM_CAUSE_PROTOCOL_ERROR;
        }
    }

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:    esm_proc_default_eps_bearer_context_failure()             **
 **                                                                        **
 ** Description: Performs default EPS bearer context activation procedure  **
 **      upon receiving notification from the EPS Mobility Manage- **
 **      ment sublayer that EMM procedure that initiated EPS de-   **
 **      fault bearer context activation locally failed.           **
 **                                                                        **
 **      The MME releases the default EPS bearer context previous- **
 **      ly allocated when ACTIVATE DEFAULT EPS BEARER CONTEXT RE- **
 **      QUEST message was received.                               **
 **                                                                        **
 ** Inputs:  ueid:      UE local identifier                        **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    The identifier of the PDN connection the   **
 **             default EPS bearer context belongs to if   **
 **             successfully released;                     **
 **             RETURNerror  otherwise.                    **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int esm_proc_default_eps_bearer_context_failure(unsigned int ueid)
{
    LOG_FUNC_IN;

    int rc = RETURNerror;
    int pid;

    LOG_TRACE(WARNING, "ESM-PROC  - Default EPS bearer context activation "
              "failure (ueid=%u)", ueid);

    /* Get the EPS bearer identity of the EPS bearer context which is still
     * pending in the active pending state */
    int ebi = esm_ebr_get_pending_ebi(ueid, ESM_EBR_ACTIVE_PENDING);
    if (ebi != ESM_EBI_UNASSIGNED) {
        int bid;
        /* Release the default EPS bearer context and enter state INACTIVE */
        rc = esm_proc_eps_bearer_context_deactivate(ueid, TRUE, ebi,
                &pid, &bid, NULL);
    }

    if (rc != RETURNerror) {
        LOG_FUNC_RETURN (pid);
    }
    LOG_FUNC_RETURN (RETURNerror);
}

#endif // NAS_MME

/*
 * --------------------------------------------------------------------------
 *    Default EPS bearer context activation procedure executed by the UE
 * --------------------------------------------------------------------------
 */
#ifdef NAS_UE
/****************************************************************************
 **                                                                        **
 ** Name:    esm_proc_default_eps_bearer_context_request()             **
 **                                                                        **
 ** Description: Creates local default EPS bearer context upon receipt of  **
 **      the ACTIVATE DEFAULT EPS BEARER CONTEXT REQUEST message.  **
 **                                                                        **
 ** Inputs:  pid:       PDN connection identifier                  **
 **      ebi:       EPS bearer identity                        **
 **      qos:       EPS bearer level QoS parameters            **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     esm_cause: Cause code returned upon ESM procedure     **
 **             failure                                    **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    _default_eps_bearer_context_data           **
 **                                                                        **
 ***************************************************************************/
int esm_proc_default_eps_bearer_context_request(int pid, int ebi,
        const esm_proc_qos_t *qos,
        int *esm_cause)
{
    LOG_FUNC_IN;

    int rc = RETURNerror;

    LOG_TRACE(INFO, "ESM-PROC  - Default EPS bearer context activation "
              "requested by the network (ebi=%d)", ebi);

    /* Assign default EPS bearer context */
    int new_ebi = esm_ebr_assign(ebi, pid+1, TRUE);
    if (new_ebi == ESM_EBI_UNASSIGNED) {
        /* 3GPP TS 24.301, section 6.4.1.5, abnormal cases a and b
         * Default EPS bearer context activation request for an already
         * activated default or dedicated EPS bearer context
         */
        int old_pid, old_bid;
        /* Locally deactivate the existing EPS bearer context and proceed
         * with the requested default EPS bearer context activation */
        rc = esm_proc_eps_bearer_context_deactivate(TRUE, ebi,
                &old_pid, &old_bid);
        if (rc != RETURNok) {
            /* Failed to release EPS bearer context */
            *esm_cause = ESM_CAUSE_PROTOCOL_ERROR;
        } else {
            /* Assign new default EPS bearer context */
            ebi = esm_ebr_assign(ebi, pid+1, TRUE);
        }
    }

    if (ebi != ESM_EBI_UNASSIGNED) {
        /* Create new default EPS bearer context */
        ebi = esm_ebr_context_create(pid, ebi, TRUE, qos, NULL);
        if (ebi != ESM_EBI_UNASSIGNED) {
            /* Default EPS bearer contextx successfully created */
            _default_eps_bearer_context_data.ebi = ebi;
            rc = RETURNok;
        } else {
            /* No resource available */
            LOG_TRACE(WARNING, "ESM-PROC  - Failed to create new default EPS "
                      "bearer context");
            *esm_cause = ESM_CAUSE_INSUFFICIENT_RESOURCES;
        }
    }

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:    esm_proc_default_eps_bearer_context_accept()              **
 **                                                                        **
 ** Description: Performs default EPS bearer context activation procedure  **
 **      accepted by the UE.                                       **
 **                                                                        **
 **      3GPP TS 24.301, section 6.4.1.3                           **
 **      The UE accepts default EPS bearer context activation by   **
 **      sending ACTIVATE DEFAULT EPS BEARER CONTEXT ACCEPT mes-   **
 **      sage and entering the state BEARER CONTEXT ACTIVE.        **
 **      If the default bearer is activated as part of the attach  **
 **      procedure, the UE shall send the accept message together  **
 **      with ATTACH COMPLETE message.                             **
 **                                                                        **
 ** Inputs:  is_standalone: Indicates whether the activate default EPS **
 **             bearer context accept has to be sent stand-**
 **             alone or together within an attach comple- **
 **             te message                                 **
 **      ebi:       EPS bearer identity                        **
 **      msg:       Encoded ESM message to be sent             **
 **      ue_triggered:  TRUE if the EPS bearer context procedure   **
 **             was triggered by the UE (should be always  **
 **             TRUE)                                      **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int esm_proc_default_eps_bearer_context_accept(int is_standalone, int ebi,
        OctetString *msg, int ue_triggered)
{
    LOG_FUNC_IN;

    int rc = RETURNok;

    LOG_TRACE(INFO,"ESM-PROC  - Default EPS bearer context activation "
              "accepted by the UE (ebi=%d)", ebi);

    if (is_standalone) {
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
    }

    if (rc != RETURNerror) {
        /* Set the EPS bearer context state to ACTIVE */
        rc = esm_ebr_set_status(ebi, ESM_EBR_ACTIVE, ue_triggered);
        if (rc != RETURNok) {
            /* The EPS bearer context was already in ACTIVE state */
            LOG_TRACE(WARNING, "ESM-PROC  - EBI %d was already ACTIVE", ebi);
            /* Accept network retransmission of already accepted activate
             * default EPS bearer context request */
            LOG_FUNC_RETURN (RETURNok);
        }
    }

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:    esm_proc_default_eps_bearer_context_reject()              **
 **                                                                        **
 ** Description: Performs default EPS bearer context activation procedure  **
 **      not accepted by the UE.                                   **
 **                                                                        **
 **      3GPP TS 24.301, section 6.4.1.4                           **
 **      The UE rejects default EPS bearer context activation by   **
 **      sending ACTIVATE DEFAULT EPS BEARER CONTEXT REJECT mes-   **
 **      sage and entering the state BEARER CONTEXT INACTIVE.      **
 **      If the default EPS bearer context activation is part of   **
 **      the attach procedure, the ESM sublayer shall notify the   **
 **      EMM sublayer of an ESM failure.                           **
 **                                                                        **
 ** Inputs:  is_standalone: Indicates whether the activate default EPS **
 **             bearer context accept has to be sent stand-**
 **             alone or together within an attach comple- **
 **             te message                                 **
 **      ebi:       EPS bearer identity                        **
 **      msg:       Encoded ESM message to be sent             **
 **      ue_triggered:  Not used                                   **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int esm_proc_default_eps_bearer_context_reject(int is_standalone, int ebi,
        OctetString *msg, int ue_triggered)
{
    LOG_FUNC_IN;

    int rc = RETURNok;

    LOG_TRACE(WARNING, "ESM-PROC  - Default EPS bearer context activation "
              "not accepted by the UE (ebi=%d)", ebi);

    if ( !esm_ebr_is_not_in_use(ebi) ) {
        /* Release EPS bearer data currently in use */
        rc = esm_ebr_release(ebi);
    }

    if (rc != RETURNok) {
        LOG_TRACE(WARNING, "ESM-PROC  - Failed to release EPS bearer data");
    } else if (is_standalone) {
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
    } else {
        /* An error is returned to notify EMM that the default EPS bearer
         * activation procedure initiated as part of the initial attach
         * procedure has failed
         */
        rc = RETURNerror;
    }

    LOG_FUNC_RETURN (rc);
}

/****************************************************************************
 **                                                                        **
 ** Name:    esm_proc_default_eps_bearer_context_complete()            **
 **                                                                        **
 ** Description: Terminates the default EPS bearer context activation pro- **
 **      cedure upon receiving indication from the EPS Mobility    **
 **      Management sublayer that the ACTIVATE DEFAULT EPS BEARER  **
 **      CONTEXT ACCEPT message has been successfully delivered to **
 **      the MME.                                                  **
 **                                                                        **
 ** Inputs:  None                                                      **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    _default_eps_bearer_context_data           **
 **                                                                        **
 ***************************************************************************/
int esm_proc_default_eps_bearer_context_complete(void)
{
    LOG_FUNC_IN;

    LOG_TRACE(INFO,
              "ESM-PROC  - Default EPS bearer context activation complete");

    /* Reset default EPS bearer context internal data */
    _default_eps_bearer_context_data.ebi = ESM_EBI_UNASSIGNED;

    LOG_FUNC_RETURN (RETURNok);
}

/****************************************************************************
 **                                                                        **
 ** Name:    esm_proc_default_eps_bearer_context_failure()             **
 **                                                                        **
 ** Description: Performs default EPS bearer context activation procedure  **
 **      upon receiving transmission failure of ESM message indi-  **
 **      cation from the EPS Mobility Management sublayer          **
 **                                                                        **
 **      The UE releases the default EPS bearer context previously **
 **      allocated before the ACTIVATE DEFAULT EPS BEARER CONTEXT  **
 **      ACCEPT message was sent.                                  **
 **                                                                        **
 ** Inputs:  None                                                      **
 **      Others:    _default_eps_bearer_context_data           **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    _default_eps_bearer_context_data           **
 **                                                                        **
 ***************************************************************************/
int esm_proc_default_eps_bearer_context_failure(void)
{
    LOG_FUNC_IN;

    int ebi = _default_eps_bearer_context_data.ebi;
    int pid, bid;

    LOG_TRACE(WARNING,
              "ESM-PROC  - Default EPS bearer context activation failure");

    /* Release the default EPS bearer context and enter state INACTIVE */
    int rc = esm_proc_eps_bearer_context_deactivate(TRUE, ebi, &pid, &bid);
    if (rc != RETURNerror) {
        /* Reset default EPS bearer context internal data */
        _default_eps_bearer_context_data.ebi = ESM_EBI_UNASSIGNED;
    }

    LOG_FUNC_RETURN (rc);
}
#endif // NAS_UE

/****************************************************************************/
/*********************  L O C A L    F U N C T I O N S  *********************/
/****************************************************************************/

/*
 * --------------------------------------------------------------------------
 *              Timer handlers
 * --------------------------------------------------------------------------
 */
#ifdef NAS_MME
/****************************************************************************
 **                                                                        **
 ** Name:    _default_eps_bearer_activate_t3485_handler()              **
 **                                                                        **
 ** Description: T3485 timeout handler                                     **
 **                                                                        **
 **              3GPP TS 24.301, section 6.4.1.6, case a                   **
 **      On the first expiry of the timer T3485, the MME shall re- **
 **      send the ACTIVATE DEFAULT EPS BEARER CONTEXT REQUEST and  **
 **      shall reset and restart timer T3485. This retransmission  **
 **      is repeated four times, i.e. on the fifth expiry of timer **
 **      T3485, the MME shall release possibly allocated resources **
 **      for this activation and shall abort the procedure.        **
 **                                                                        **
 ** Inputs:  args:      handler parameters                         **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    None                                       **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static void *_default_eps_bearer_activate_t3485_handler(void *args)
{
    LOG_FUNC_IN;

    int rc;

    /* Get retransmission timer parameters data */
    esm_ebr_timer_data_t *data = (esm_ebr_timer_data_t *)(args);

    /* Increment the retransmission counter */
    data->count += 1;

    LOG_TRACE(WARNING, "ESM-PROC  - T3485 timer expired (ueid=%d, ebi=%d), "
              "retransmission counter = %d",
              data->ueid, data->ebi, data->count);

    if (data->count < DEFAULT_EPS_BEARER_ACTIVATE_COUNTER_MAX) {
        /* Re-send activate default EPS bearer context request message
         * to the UE */
        rc = _default_eps_bearer_activate(data->ueid, data->ebi, &data->msg);
    } else {
        /*
         * The maximum number of activate default EPS bearer context request
         * message retransmission has exceed
         */
        int pid, bid;
        /* Release the default EPS bearer context and enter state INACTIVE */
        rc = esm_proc_eps_bearer_context_deactivate(data->ueid, TRUE,
                data->ebi, &pid, &bid,
                NULL);
        if (rc != RETURNerror) {
            /* Stop timer T3485 */
            rc = esm_ebr_stop_timer(data->ueid, data->ebi);
        }
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
 ** Name:    _default_eps_bearer_activate()                            **
 **                                                                        **
 ** Description: Sends ACTIVATE DEFAULT EPS BEREAR CONTEXT REQUEST message **
 **      and starts timer T3485                                    **
 **                                                                        **
 ** Inputs:  ueid:      UE local identifier                        **
 **      ebi:       EPS bearer identity                        **
 **      msg:       Encoded ESM message to be sent             **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    T3485                                      **
 **                                                                        **
 ***************************************************************************/
static int _default_eps_bearer_activate(unsigned int ueid, int ebi,
                                        const OctetString *msg)
{
    LOG_FUNC_IN;

    emm_sap_t emm_sap;
    int rc;

    /*
     * Notify EMM that an activate default EPS bearer context request message
     * has to be sent to the UE
     */
    emm_esm_data_t *emm_esm = &emm_sap.u.emm_esm.u.data;
    emm_sap.primitive = EMMESM_UNITDATA_REQ;
    emm_sap.u.emm_esm.ueid = ueid;
    emm_esm->msg = *msg;
    rc = emm_sap_send(&emm_sap);

    if (rc != RETURNerror) {
        /* Start T3485 retransmission timer */
        rc = esm_ebr_start_timer(ueid, ebi, msg, T3485_DEFAULT_VALUE,
                                 _default_eps_bearer_activate_t3485_handler);
    }

    LOG_FUNC_RETURN (rc);
}
#endif // NAS_MME
