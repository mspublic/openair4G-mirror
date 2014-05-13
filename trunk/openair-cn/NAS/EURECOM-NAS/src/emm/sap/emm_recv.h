/*****************************************************************************
            Eurecom OpenAirInterface 3
            Copyright(c) 2012 Eurecom

Source      emm_recv.h

Version     0.1

Date        2013/01/30

Product     NAS stack

Subsystem   EPS Mobility Management

Author      Frederic Maurel

Description Defines functions executed at the EMMAS Service Access
        Point upon receiving EPS Mobility Management messages
        from the Access Stratum sublayer.

*****************************************************************************/
#ifndef __EMM_RECV_H__
#define __EMM_RECV_H__

#include "EmmStatus.h"

#include "DetachRequest.h"
#include "DetachAccept.h"

#ifdef NAS_UE
#include "AttachAccept.h"
#include "AttachReject.h"
#include "TrackingAreaUpdateAccept.h"
#include "TrackingAreaUpdateReject.h"
#include "ServiceReject.h"
#include "GutiReallocationCommand.h"
#include "AuthenticationRequest.h"
#include "AuthenticationReject.h"
#include "IdentityRequest.h"
#include "NASSecurityModeCommand.h"
#include "EmmInformation.h"
#include "DownlinkNasTransport.h"
#include "CsServiceNotification.h"
#endif

#ifdef NAS_MME
#include "AttachRequest.h"
#include "AttachComplete.h"
#include "TrackingAreaUpdateRequest.h"
#include "TrackingAreaUpdateComplete.h"
#include "ServiceRequest.h"
#include "ExtendedServiceRequest.h"
#include "GutiReallocationComplete.h"
#include "AuthenticationResponse.h"
#include "AuthenticationFailure.h"
#include "IdentityResponse.h"
#include "SecurityModeComplete.h"
#include "SecurityModeReject.h"
#include "UplinkNasTransport.h"
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
 * Functions executed by both the UE and the MME upon receiving EMM messages
 * --------------------------------------------------------------------------
 */
int emm_recv_status(unsigned int ueid, emm_status_msg *msg, int *emm_cause);

/*
 * --------------------------------------------------------------------------
 * Functions executed by the UE upon receiving EMM message from the network
 * --------------------------------------------------------------------------
 */
#ifdef NAS_UE
int emm_recv_attach_accept(attach_accept_msg *msg, int *emm_cause);
int emm_recv_attach_reject(attach_reject_msg *msg, int *emm_cause);

int emm_recv_detach_accept(detach_accept_msg *msg, int *emm_cause);

int emm_recv_identity_request(identity_request_msg *msg, int *emm_cause);
int emm_recv_authentication_request(authentication_request_msg *msg,
                                    int *emm_cause);
int emm_recv_authentication_reject(authentication_reject_msg *msg,
                                   int *emm_cause);
int emm_recv_security_mode_command(security_mode_command_msg *msg,
                                   int *emm_cause);
#endif

/*
 * --------------------------------------------------------------------------
 * Functions executed by the MME upon receiving EMM message from the UE
 * --------------------------------------------------------------------------
 */
#ifdef NAS_MME
int emm_recv_attach_request(unsigned int ueid, const attach_request_msg *msg,
                            int *emm_cause);
int emm_recv_attach_complete(unsigned int ueid, const attach_complete_msg *msg,
                             int *emm_cause);

int emm_recv_detach_request(unsigned int ueid, const detach_request_msg *msg,
                            int *emm_cause);

int emm_recv_tracking_area_update_request(unsigned int  ueid,
                                          const tracking_area_update_request_msg *msg,
                                          int *emm_cause);

int emm_recv_identity_response(unsigned int ueid, identity_response_msg *msg,
                               int *emm_cause);
int emm_recv_authentication_response(unsigned int ueid,
                                     authentication_response_msg *msg, int *emm_cause);
int emm_recv_authentication_failure(unsigned int ueid,
                                    authentication_failure_msg *msg, int *emm_cause);
int emm_recv_security_mode_complete(unsigned int ueid,
                                    security_mode_complete_msg *msg, int *emm_cause);
int emm_recv_security_mode_reject(unsigned int ueid,
                                  security_mode_reject_msg *msg, int *emm_cause);
#endif

#endif /* __EMM_RECV_H__*/
