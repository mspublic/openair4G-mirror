/*****************************************************************************
            Eurecom OpenAirInterface 3
            Copyright(c) 2012 Eurecom

Source      esm_send.h

Version     0.1

Date        2013/02/11

Product     NAS stack

Subsystem   EPS Session Management

Author      Frederic Maurel

Description Defines functions executed at the ESM Service Access
        Point to send EPS Session Management messages to the
        EPS Mobility Management sublayer.

*****************************************************************************/
#ifndef __ESM_SEND_H__
#define __ESM_SEND_H__

#include "EsmStatus.h"

#ifdef NAS_MME
#include "PdnConnectivityReject.h"
#include "PdnDisconnectReject.h"
#include "BearerResourceAllocationReject.h"
#include "BearerResourceModificationReject.h"

#include "ActivateDefaultEpsBearerContextRequest.h"
#include "ActivateDedicatedEpsBearerContextRequest.h"
#include "ModifyEpsBearerContextRequest.h"
#include "DeactivateEpsBearerContextRequest.h"

#include "EsmInformationRequest.h"
#endif

#ifdef NAS_UE
#include "PdnConnectivityRequest.h"
#include "PdnDisconnectRequest.h"
#include "BearerResourceAllocationRequest.h"
#include "BearerResourceModificationRequest.h"

#include "ActivateDefaultEpsBearerContextAccept.h"
#include "ActivateDefaultEpsBearerContextReject.h"
#include "ActivateDedicatedEpsBearerContextAccept.h"
#include "ActivateDedicatedEpsBearerContextReject.h"
#include "ModifyEpsBearerContextAccept.h"
#include "ModifyEpsBearerContextReject.h"
#include "DeactivateEpsBearerContextAccept.h"

#include "EsmInformationResponse.h"
#endif

/****************************************************************************/
/*********************  G L O B A L    C O N S T A N T S  *******************/
/****************************************************************************/

/****************************************************************************/
/************************  G L O B A L    T Y P E S  ************************/
/****************************************************************************/

/****************************************************************************/
/********************  G L O B A L    V A R I A B L E S  ********************/
/****************************************************************************/

/****************************************************************************/
/******************  E X P O R T E D    F U N C T I O N S  ******************/
/****************************************************************************/

/*
 * --------------------------------------------------------------------------
 * Functions executed by both the UE and the MME to send ESM messages
 * --------------------------------------------------------------------------
 */
int esm_send_status(int pti, int ebi, esm_status_msg *msg, int esm_cause);

/*
 * --------------------------------------------------------------------------
 * Functions executed by the UE to send ESM message to the network
 * --------------------------------------------------------------------------
 */
#ifdef NAS_UE
/*
 * Transaction related messages
 * ----------------------------
 */
int esm_send_pdn_connectivity_request(int pti, int is_emergency, int pdn_type,
                                      const char *apn, pdn_connectivity_request_msg *msg);
int esm_send_pdn_disconnect_request(int pti, int ebi,
                                    pdn_disconnect_request_msg *msg);

/*
 * Messages related to EPS bearer contexts
 * ---------------------------------------
 */
int esm_send_activate_default_eps_bearer_context_accept(int ebi,
        activate_default_eps_bearer_context_accept_msg *msg);
int esm_send_activate_default_eps_bearer_context_reject(int ebi,
        activate_default_eps_bearer_context_reject_msg *msg, int esm_cause);

int esm_send_activate_dedicated_eps_bearer_context_accept(int ebi,
        activate_dedicated_eps_bearer_context_accept_msg *msg);
int esm_send_activate_dedicated_eps_bearer_context_reject(int ebi,
        activate_dedicated_eps_bearer_context_reject_msg *msg, int esm_cause);

int esm_send_deactivate_eps_bearer_context_accept(int ebi,
        deactivate_eps_bearer_context_accept_msg *msg);

#endif

/*
 * --------------------------------------------------------------------------
 * Functions executed by the MME to send ESM message to the UE
 * --------------------------------------------------------------------------
 */
#ifdef NAS_MME
/*
 * Transaction related messages
 * ----------------------------
 */
int esm_send_pdn_connectivity_reject(int pti, pdn_connectivity_reject_msg *msg,
                                     int esm_cause);

int esm_send_pdn_disconnect_reject(int pti, pdn_disconnect_reject_msg *msg,
                                   int esm_cause);

/*
 * Messages related to EPS bearer contexts
 * ---------------------------------------
 */
int esm_send_activate_default_eps_bearer_context_request(int pti, int ebi,
        activate_default_eps_bearer_context_request_msg *msg, const OctetString *apn,
        int pdn_type, const OctetString *pdn_addr, const EpsQualityOfService *qos,
        int esm_cause);

int esm_send_activate_dedicated_eps_bearer_context_request(int pti, int ebi,
        activate_dedicated_eps_bearer_context_request_msg *msg, int linked_ebi,
        const EpsQualityOfService *qos, PacketFilters *pkfs, int n_pkfs);

int esm_send_deactivate_eps_bearer_context_request(int pti, int ebi,
        deactivate_eps_bearer_context_request_msg *msg, int esm_cause);

#endif

#endif /* __ESM_SEND_H__*/
