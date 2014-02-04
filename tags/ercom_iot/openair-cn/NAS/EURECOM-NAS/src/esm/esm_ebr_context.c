/*****************************************************************************
            Eurecom OpenAirInterface 3
            Copyright(c) 2012 Eurecom

Source      esm_ebr_context.h

Version     0.1

Date        2013/05/28

Product     NAS stack

Subsystem   EPS Session Management

Author      Frederic Maurel

Description Defines functions used to handle EPS bearer contexts.

*****************************************************************************/
#include <stdlib.h> // malloc, free
#include <string.h> // memset

#include "commonDef.h"
#include "nas_log.h"

#include "emmData.h"
#include "esm_ebr.h"

#include "esm_ebr_context.h"

#include "emm_sap.h"

#if defined(ENABLE_ITTI)
# include "assertions.h"
#endif

/****************************************************************************/
/****************  E X T E R N A L    D E F I N I T I O N S  ****************/
/****************************************************************************/

/****************************************************************************/
/*******************  L O C A L    D E F I N I T I O N S  *******************/
/****************************************************************************/

#ifdef NAS_UE
static int _esm_ebr_context_check_identifiers(const network_tft_t *,
        const network_tft_t *);
static int _esm_ebr_context_check_precedence(const network_tft_t *,
        const network_tft_t *);
#endif

/****************************************************************************/
/******************  E X P O R T E D    F U N C T I O N S  ******************/
/****************************************************************************/

/****************************************************************************
 **                                                                        **
 ** Name:    esm_ebr_context_create()                                  **
 **                                                                        **
 ** Description: Creates a new EPS bearer context to the PDN with the spe- **
 **      cified PDN connection identifier                          **
 **                                                                        **
 ** Inputs:  ueid:      UE identifier                              **
 **      pid:       PDN connection identifier                  **
 **      ebi:       EPS bearer identity                        **
 **      is_default:    TRUE if the new bearer is a default EPS    **
 **             bearer context                             **
 **      esm_qos:   EPS bearer level QoS parameters            **
 **      tft:       Traffic flow template parameters           **
 **      Others:    _esm_data                                  **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    The EPS bearer identity of the default EPS **
 **             bearer associated to the new EPS bearer    **
 **             context if successfully created;           **
 **             UNASSIGN EPS bearer value otherwise.       **
 **      Others:    _esm_data                                  **
 **                                                                        **
 ***************************************************************************/
int esm_ebr_context_create(
#ifdef NAS_MME
    emm_data_context_t *ctx,
#endif
    int pid, int ebi, int is_default,
    const network_qos_t *qos, const network_tft_t *tft)
{
    int bid;
    esm_data_context_t *esm_ctx;
    esm_pdn_t *pdn = NULL;

    unsigned int ueid = 0;

    LOG_FUNC_IN;

#ifdef NAS_UE
    esm_ctx = &_esm_data;
#endif
#ifdef NAS_MME
# if defined(EPC_BUILD)
    esm_ctx = &ctx->esm_data_ctx;
# else
    if (ueid < ESM_DATA_NB_UE_MAX) {
        ctx = _esm_data.ctx[ueid];
    } else {
        LOG_FUNC_RETURN (ESM_EBI_UNASSIGNED);
    }
# endif
#endif

    bid = ESM_DATA_EPS_BEARER_MAX;

    LOG_TRACE(INFO, "ESM-PROC  - Create new %s EPS bearer context (ebi=%d) "
              "for PDN connection (pid=%d)",
              (is_default)? "default" : "dedicated", ebi, pid);

    if (pid < ESM_DATA_PDN_MAX) {
        if (pid != esm_ctx->pdn[pid].pid) {
            LOG_TRACE(ERROR, "ESM-PROC  - PDN connection identifier %d is "
                      "not valid", pid);
        } else if (esm_ctx->pdn[pid].data == NULL) {
            LOG_TRACE(ERROR, "ESM-PROC  - PDN connection %d has not been "
                      "allocated", pid);
        }
        /* Check the total number of active EPS bearers */
        else if (esm_ctx->n_ebrs > ESM_DATA_EPS_BEARER_TOTAL) {
            LOG_TRACE(WARNING, "ESM-PROC  - The total number of active EPS"
                      "bearers is exeeded");
        } else {
            /* Get the PDN connection entry */
            pdn = esm_ctx->pdn[pid].data;
            if (is_default) {
                /* Default EPS bearer entry is defined at index 0 */
                bid = 0;
                if (pdn->bearer[bid] != NULL) {
                    LOG_TRACE(ERROR, "ESM-PROC  - A default EPS bearer context "
                              "is already allocated");
                    LOG_FUNC_RETURN (ESM_EBI_UNASSIGNED);
                }
            } else {
                /* Search for an available EPS bearer context entry */
                for (bid = 1; bid < ESM_DATA_EPS_BEARER_MAX; bid++) {
                    if (pdn->bearer[bid] != NULL) {
                        continue;
                    }
                    break;
                }
            }
        }
    }

    if (bid < ESM_DATA_EPS_BEARER_MAX) {
        /* Create new EPS bearer context */
        esm_bearer_t *ebr = (esm_bearer_t *)malloc(sizeof(esm_bearer_t));
        if (ebr != NULL) {
            memset(ebr, 0 , sizeof(esm_bearer_t));
            /* Increment the total number of active EPS bearers */
            esm_ctx->n_ebrs += 1;
            /* Increment the number of EPS bearer for this PDN connection */
            pdn->n_bearers += 1;
            /* Setup the EPS bearer data */
            pdn->bearer[bid] = ebr;
            ebr->bid = bid;
            ebr->ebi = ebi;
            if (qos != NULL) {
                /* EPS bearer level QoS parameters */
                ebr->qos = *qos;
            }
            if ( (tft != NULL) && (tft->n_pkfs < NET_PACKET_FILTER_MAX) ) {
                int i;
                /* Traffic flow template parameters */
                for (i = 0; i < tft->n_pkfs; i++) {
                    ebr->tft.pkf[i] =
                        (network_pkf_t *)malloc(sizeof(network_pkf_t));
                    if (ebr->tft.pkf[i] != NULL) {
                        *(ebr->tft.pkf[i]) = *(tft->pkf[i]);
                        ebr->tft.n_pkfs += 1;
                    }
                }
            }

            if (is_default) {
                /* Set the PDN connection activation indicator */
                esm_ctx->pdn[pid].is_active = TRUE;
                /* Update the emergency bearer services indicator */
                if (pdn->is_emergency) {
                    esm_ctx->emergency = TRUE;
                }
            }

            /* Return the EPS bearer identity of the default EPS bearer
             * associated to the new EPS bearer context */
            LOG_FUNC_RETURN (pdn->bearer[0]->ebi);
        }
        LOG_TRACE(WARNING, "ESM-PROC  - Failed to create new EPS bearer "
                  "context (ebi=%d)", ebi);
    }

    LOG_FUNC_RETURN (ESM_EBI_UNASSIGNED);
}

/****************************************************************************
 **                                                                        **
 ** Name:    esm_ebr_context_release()                                 **
 **                                                                        **
 ** Description: Releases EPS bearer context entry previously allocated    **
 **      to the EPS bearer with the specified EPS bearer identity  **
 **                                                                        **
 ** Inputs:  ueid:      UE identifier                              **
 **      ebi:       EPS bearer identity                        **
 **      Others:    _esm_data                                  **
 **                                                                        **
 ** Outputs:     pid:       Identifier of the PDN connection entry the **
 **             EPS bearer context belongs to              **
 **      bid:       Identifier of the released EPS bearer con- **
 **             text entry                                 **
 **      Return:    The EPS bearer identity associated to the  **
 **             EPS bearer context if successfully relea-  **
 **             sed; UNASSIGN EPS bearer value otherwise.  **
 **      Others:    _esm_data                                  **
 **                                                                        **
 ***************************************************************************/
int esm_ebr_context_release(
#ifdef NAS_MME
    emm_data_context_t *ctx,
#endif
    int ebi, int *pid, int *bid)
{
    int found = FALSE;
    esm_pdn_t *pdn = NULL;
    esm_data_context_t *esm_ctx;

    unsigned int ueid = 0;

    LOG_FUNC_IN;

#ifdef NAS_UE
    esm_ctx = &_esm_data;
#endif

#ifdef NAS_MME
# if defined(EPC_BUILD)
    esm_ctx = &ctx->esm_data_ctx;
# else
    if (ueid < ESM_DATA_NB_UE_MAX) {
        ctx = &_esm_data.ctx[ueid];
    } else {
        LOG_FUNC_RETURN (ESM_EBI_UNASSIGNED);
    }
# endif
#endif

    if (ebi != ESM_EBI_UNASSIGNED) {
        /*
         * The identity of the EPS bearer to released is given;
         * Release the EPS bearer context entry that match the specified EPS
         * bearer identity
         */

        /* Search for active PDN connection */
        for (*pid = 0; *pid < ESM_DATA_PDN_MAX; (*pid)++) {
            if ( !esm_ctx->pdn[*pid].is_active ) {
                continue;
            }
            /* An active PDN connection is found */
            if (esm_ctx->pdn[*pid].data != NULL) {
                pdn = esm_ctx->pdn[*pid].data;
                /* Search for the specified EPS bearer context entry */
                for (*bid = 0; *bid < pdn->n_bearers; (*bid)++) {
                    if (pdn->bearer[*bid] != NULL) {
                        if (pdn->bearer[*bid]->ebi != ebi) {
                            continue;
                        }
                        /* The EPS bearer context entry is found */
                        found = TRUE;
                        break;
                    }
                }
            }
            if (found) {
                break;
            }
        }
    } else {
        /*
         * The identity of the EPS bearer to released is not given;
         * Release the EPS bearer context entry allocated with the EPS
         * bearer context identifier (bid) to establish connectivity to
         * the PDN identified by the PDN connection identifier (pid).
         * Default EPS bearer to a given PDN is always identified by the
         * first EPS bearer context entry at index bid = 0
         */
        if (*pid < ESM_DATA_PDN_MAX) {
            if (*pid != esm_ctx->pdn[*pid].pid) {
                LOG_TRACE(ERROR, "ESM-PROC  - PDN connection identifier %d "
                          "is not valid", *pid);
            } else if (!esm_ctx->pdn[*pid].is_active) {
                LOG_TRACE(WARNING,"ESM-PROC  - PDN connection %d is not active",
                          *pid);
            } else if (esm_ctx->pdn[*pid].data == NULL) {
                LOG_TRACE(ERROR, "ESM-PROC  - PDN connection %d has not been "
                          "allocated", *pid);
            } else {
                pdn = esm_ctx->pdn[*pid].data;
                if (pdn->bearer[*bid] != NULL) {
                    ebi = pdn->bearer[*bid]->ebi;
                    found = TRUE;
                }
            }
        }
    }

    if (found) {
        int i, j;
        /*
         * Delete the specified EPS bearer context entry
         */
        if (pdn->bearer[*bid]->bid != *bid) {
            LOG_TRACE(ERROR, "ESM-PROC  - EPS bearer identifier %d is "
                      "not valid", *bid);
            LOG_FUNC_RETURN (ESM_EBI_UNASSIGNED);
        }

        LOG_TRACE(WARNING, "ESM-PROC  - Release EPS bearer context "
                  "(ebi=%d)", ebi);

        /* Delete the TFT */
        for (i = 0; i < pdn->bearer[*bid]->tft.n_pkfs; i++) {
            free(pdn->bearer[*bid]->tft.pkf[i]);
        }
        /* Release the specified EPS bearer data */
        free(pdn->bearer[*bid]);
        pdn->bearer[*bid] = NULL;
        /* Decrement the number of EPS bearer context allocated
         * to the PDN connection */
        pdn->n_bearers -= 1;
        /* Decrement the total number of active EPS bearers */
        esm_ctx->n_ebrs -= 1;

        if (*bid == 0) {
            /* 3GPP TS 24.301, section 6.4.4.3, 6.4.4.6
             * If the EPS bearer identity is that of the default bearer to a
             * PDN, the UE shall delete all EPS bearer contexts associated to
             * that PDN connection.
             */
            for (i = 1; pdn->n_bearers > 0; i++) {
                if (pdn->bearer[i]) {

                    LOG_TRACE(WARNING, "ESM-PROC  - Release EPS bearer context "
                              "(ebi=%d)", pdn->bearer[i]->ebi);

                    /* Delete the TFT */
                    for (j = 0; j < pdn->bearer[i]->tft.n_pkfs; j++) {
                        free(pdn->bearer[i]->tft.pkf[j]);
                    }
                    /* Set the EPS bearer context state to INACTIVE */
#ifdef NAS_UE
                    (void) esm_ebr_set_status(pdn->bearer[i]->ebi,
                                              ESM_EBR_INACTIVE, TRUE);
#endif
#ifdef NAS_MME
                    (void) esm_ebr_set_status(ctx, pdn->bearer[i]->ebi,
                                              ESM_EBR_INACTIVE, TRUE);
#endif
                    /* Release EPS bearer data */
#ifdef NAS_UE
                    (void) esm_ebr_release(pdn->bearer[i]->ebi);
#endif
#ifdef NAS_MME
                    (void) esm_ebr_release(ctx, pdn->bearer[i]->ebi);
#endif
                    // esm_ebr_release()
                    /* Release dedicated EPS bearer data */
                    free(pdn->bearer[i]);
                    pdn->bearer[i] = NULL;
                    /* Decrement the number of EPS bearer context allocated
                     * to the PDN connection */
                    pdn->n_bearers -= 1;
                    /* Decrement the total number of active EPS bearers */
                    esm_ctx->n_ebrs -= 1;
                }
            }
            /* Reset the PDN connection activation indicator */
            esm_ctx->pdn[*pid].is_active = FALSE;
            /* Update the emergency bearer services indicator */
            if (pdn->is_emergency) {
                esm_ctx->emergency = FALSE;
            }
        }

#ifdef NAS_UE
        /* 3GPP TS 24.301, section 6.4.4.6
         * If the UE locally deactivated all EPS bearer contexts, the UE
         * shall perform a local detach and enter state EMM-DEREGISTERED.
         */
        if (esm_ctx->n_ebrs == 0) {
            emm_sap_t emm_sap;
            emm_sap.primitive = EMMESM_ESTABLISH_CNF;
            emm_sap.u.emm_esm.u.establish.is_attached = FALSE;
            (void) emm_sap_send(&emm_sap);
        }
        /* 3GPP TS 24.301, section 6.4.4.3, 6.4.4.6
         * If due to the EPS bearer context deactivation only the PDN
         * connection for emergency bearer services remains established,
         * the UE shall consider itself attached for emergency bearer
         * services only.
         */
        else if (esm_ctx->emergency && (esm_ctx->n_ebrs == 1) ) {
            emm_sap_t emm_sap;
            emm_sap.primitive = EMMESM_ESTABLISH_CNF;
            emm_sap.u.emm_esm.u.establish.is_attached = TRUE;
            emm_sap.u.emm_esm.u.establish.is_emergency = TRUE;
            (void) emm_sap_send(&emm_sap);
        }
#endif // NAS_UE
#ifdef NAS_MME
        if (esm_ctx->n_ebrs == 0) {
            /* TODO: Release the PDN connection and marked the UE as inactive
             * in the network for EPS services (is_attached = FALSE) */
        }
#endif

        LOG_FUNC_RETURN (ebi);
    }

    LOG_FUNC_RETURN (ESM_EBI_UNASSIGNED);
}

#ifdef NAS_UE
/****************************************************************************
 **                                                                        **
 ** Name:    esm_ebr_context_get_pid()                                 **
 **                                                                        **
 ** Description: Returns the identifier of the PDN connection entry the    **
 **      default EPS bearer context with the specified EPS bearer  **
 **      identity belongs to                                       **
 **                                                                        **
 ** Inputs:  ebi:       The EPS bearer identity of the default EPS **
 **             bearer context                             **
 **      Others:    _esm_data                                  **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    The identifier of the PDN connection entry **
 **             associated to the specified default EPS    **
 **             bearer context if it exists; -1 otherwise. **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int esm_ebr_context_get_pid(int ebi)
{
    LOG_FUNC_IN;

    int pid;

    for (pid = 0; pid < ESM_DATA_PDN_MAX; pid++) {
        if (_esm_data.pdn[pid].data == NULL) {
            continue;
        }
        if (_esm_data.pdn[pid].data->bearer[0] == NULL) {
            continue;
        }
        if (_esm_data.pdn[pid].data->bearer[0]->ebi == ebi) {
            break;
        }
    }

    if (pid < ESM_DATA_PDN_MAX) {
        LOG_FUNC_RETURN (pid);
    }
    LOG_FUNC_RETURN (-1);
}

/****************************************************************************
 **                                                                        **
 ** Name:    esm_ebr_context_check_tft()                               **
 **                                                                        **
 ** Description: Checks syntactical errors in packet filters associated to **
 **      the EPS bearer context with the specified EPS bearer      **
 **      identity for the PDN connection entry with the given      **
 **      identifier                                                **
 **                                                                        **
 ** Inputs:  pid:       Identifier of the PDN connection entry the **
 **             EPS bearer context belongs to              **
 **      ebi:       The EPS bearer identity of the EPS bearer  **
 **             context with associated packet filter list **
 **      tft:       The traffic flow template (set of packet   **
 **             filters) to be checked                     **
 **      operation: Traffic flow template operation            **
 **      Others:    _esm_data                                  **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    RETURNok, RETURNerror                      **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
int esm_ebr_context_check_tft(int pid, int ebi,
                              const network_tft_t *tft,
                              esm_ebr_context_tft_t operation)
{
    LOG_FUNC_IN;

    int rc = RETURNerror;
    int i;

    if (pid < ESM_DATA_PDN_MAX) {
        if (pid != _esm_data.pdn[pid].pid) {
            LOG_TRACE(ERROR, "ESM-PROC  - PDN connection identifier %d "
                      "is not valid", pid);
        } else if (_esm_data.pdn[pid].data == NULL) {
            LOG_TRACE(ERROR, "ESM-PROC  - PDN connection %d has not been "
                      "allocated", pid);
        } else if (operation == ESM_EBR_CONTEXT_TFT_CREATE) {
            esm_pdn_t *pdn = _esm_data.pdn[pid].data;
            /* For each EPS bearer context associated to the PDN connection */
            for (i = 0; i < pdn->n_bearers; i++) {
                if (pdn->bearer[i]) {
                    if (pdn->bearer[i]->ebi == ebi) {
                        /* Check the packet filter identifiers */
                        rc = _esm_ebr_context_check_identifiers(tft,
                                                                &pdn->bearer[i]->tft);
                        if (rc != RETURNok) {
                            break;
                        }
                    }
                    /* Check the packet filter precedence values */
                    rc = _esm_ebr_context_check_precedence(tft,
                                                           &pdn->bearer[i]->tft);
                    if (rc != RETURNok) {
                        break;
                    }
                }
            }
        }
    }

    LOG_FUNC_RETURN (rc);
}
#endif // NAS_UE

/****************************************************************************/
/*********************  L O C A L    F U N C T I O N S  *********************/
/****************************************************************************/

#ifdef NAS_UE
/****************************************************************************
 **                                                                        **
 ** Name:    _esm_ebr_context_check_identifiers()                      **
 **                                                                        **
 ** Description: Compares traffic flow templates to check whether two or   **
 **      more packet filters have identical packet filter identi-  **
 **      fiers                                                     **
 **                                                                        **
 ** Inputs:  tft1:      The first set of packet filters            **
 **      tft2:      The second set of packet filters           **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    RETURNerror if at least one packet filter  **
 **             has same identifier in both traffic flow   **
 **             templates; RETURNok otherwise.             **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static int _esm_ebr_context_check_identifiers(const network_tft_t *tft1,
        const network_tft_t *tft2)
{
    int i;
    int j;

    if ( (tft1 == NULL) || (tft2 == NULL) ) {
        return (RETURNok);
    }
    for (i = 0; i < tft1->n_pkfs; i++) {
        for (j = 0; j < tft2->n_pkfs; j++) {
            /* Packet filters should have been allocated */
            if (tft1->pkf[i]->id == tft2->pkf[i]->id) {
                /* 3GPP TS 24.301, section 6.4.2.5, abnormal cases d.1
                 * Packet filters have same identifier */
                return (RETURNerror);
            }
        }
    }
    return (RETURNok);
}

/****************************************************************************
 **                                                                        **
 ** Name:    _esm_ebr_context_check_precedence()                       **
 **                                                                        **
 ** Description: Compares traffic flow templates to check whether two or   **
 **      more packet filters have identical precedence values      **
 **                                                                        **
 ** Inputs:  tft1:      The first set of packet filters            **
 **      tft2:      The second set of packet filters           **
 **      Others:    None                                       **
 **                                                                        **
 ** Outputs:     None                                                      **
 **      Return:    RETURNerror if at least one packet filter  **
 **             has same precedence value in both traffic  **
 **             flow templates; RETURNerror otherwise.     **
 **      Others:    None                                       **
 **                                                                        **
 ***************************************************************************/
static int _esm_ebr_context_check_precedence(const network_tft_t *tft1,
        const network_tft_t *tft2)
{
    int i;
    int j;

    if ( (tft1 == NULL) || (tft2 == NULL) ) {
        return (RETURNok);
    }
    for (i = 0; i < tft1->n_pkfs; i++) {
        for (j = 0; j < tft2->n_pkfs; j++) {
            /* Packet filters should have been allocated */
            if (tft1->pkf[i]->precedence == tft2->pkf[i]->precedence) {
                /* 3GPP TS 24.301, section 6.4.2.5, abnormal cases d.2
                 * Packet filters have same precedence value */
                /* TODO: Actually if the old packet filters do not belong
                 * to the default EPS bearer context, the UE shall not
                 * diagnose an error (see 6.4.2.5, abnormal cases d.2) */
                return (RETURNerror);
            }
        }
    }
    return (RETURNok);
}
#endif // NAS_UE
