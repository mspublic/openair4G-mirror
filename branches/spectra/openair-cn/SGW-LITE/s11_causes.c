/*******************************************************************************
Eurecom OpenAirInterface core network
Copyright(c) 1999 - 2014 Eurecom

This program is free software; you can redistribute it and/or modify it
under the terms and conditions of the GNU General Public License,
version 2, as published by the Free Software Foundation.

This program is distributed in the hope it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

The full GNU General Public License is included in this distribution in
the file called "COPYING".

Contact Information
Openair Admin: openair_admin@eurecom.fr
Openair Tech : openair_tech@eurecom.fr
Forums       : http://forums.eurecom.fsr/openairinterface
Address      : EURECOM,
               Campus SophiaTech,
               450 Route des Chappes,
               CS 50193
               06904 Biot Sophia Antipolis cedex,
               FRANCE
*******************************************************************************/
/*! \file s11_causes.c
* \brief
* \author Sebastien ROUX, Lionel Gauthier
* \company Eurecom
* \email: lionel.gauthier@eurecom.fr
*/
#define SGW_LITE
#define S11_CAUSES_C

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "s11_causes.h"
#include "common_types.h"
#include "sgw_lite_ie_defs.h"

static const SGWCauseMapping_t causes[] = {
    { LOCAL_DETACH,                "Local detach",                              0, 0, 0, 0 },
    { COMPLETE_DETACH,             "Complete detach",                           0, 0, 0, 0 },
    { RAT_CHANGE_3GPP_TO_NON_3GPP, "From 3GPP to non 3GPP RAT change",          0, 0, 0, 0 },
    { ISR_DEACTIVATION,            "ISR deactivation",                          0, 0, 0, 0 },
    { ERROR_IND_FROM_RNC_ENB_SGSN, "Error ind received from RNC/eNB/SGSN",      0, 0, 0, 0 },
    { IMSI_DETACH_ONLY,            "IMSI detach only",                          0, 0, 0, 0 },
    { REQUEST_ACCEPTED,            "Request accepted",                          1, 1, 1, 1 },
    { REQUEST_ACCEPTED_PARTIALLY,  "Request accepted partially",                1, 1, 1, 0 },
    { NEW_PDN_TYPE_NW_PREF,        "New PDN type network preference",           1, 0, 0, 0 },
    { NEW_PDN_TYPE_SAB_ONLY,       "New PDN type single address bearer only",   1, 0, 0, 0 },
    { CONTEXT_NOT_FOUND,           "Context not found",                         0, 1, 1, 1 },
    { INVALID_MESSAGE_FORMAT,      "Invalid message format",                    1, 1, 1, 1 },
    { INVALID_LENGTH,              "Invalid length",                            1, 1, 1, 0 },
    { SERVICE_NOT_SUPPORTED,       "Service not supported",                     0, 0, 1, 0 },
    { SYSTEM_FAILURE,              "System failure",                            1, 1, 1, 0 },
    { NO_RESOURCES_AVAILABLE,      "No resources available",                    1, 0, 0, 0 },
    { MISSING_OR_UNKNOWN_APN,      "Missing or unknown APN",                    1, 0, 0, 0 },
    { GRE_KEY_NOT_FOUND,           "GRE KEY not found",                         1, 0, 0, 0 },
    { DENIED_IN_RAT,               "Denied in RAT",                             1, 1, 0, 0 },
    { UE_NOT_RESPONDING,           "UE not responding",                         0, 1, 0, 0 },
    { SERVICE_DENIED,              "Service Denied",                            0, 0, 0, 0 },
    { UNABLE_TO_PAGE_UE,           "Unable to page UE",                         0, 1, 0, 0 },
    { NO_MEMORY_AVAILABLE,         "No memory available",                       1, 1, 1, 0 },
    { REQUEST_REJECTED,            "Request rejected",                          1, 1, 1, 0 },
    { INVALID_PEER,                "Invalid peer",                              0, 0, 0, 1 },
    { TEMP_REJECT_HO_IN_PROGRESS,  "Temporarily rejected due to HO in progress",0, 0, 0, 0 },
    { M_PDN_APN_NOT_ALLOWED,       "Multiple PDN for a given APN not allowed",  1, 0, 0, 0 },
    { 0,                           NULL,                                        0, 0, 0, 0 },
};

static int compare_cause_id(const void *m1, const void *m2)
{
    struct SGWCauseMapping_e *scm1 = (struct SGWCauseMapping_e*)m1;
    struct SGWCauseMapping_e *scm2 = (struct SGWCauseMapping_e*)m2;

    return (scm1->value - scm2->value);
}

char *sgw_cause_2_string(uint8_t cause_value)
{
    SGWCauseMapping_t *res, key;
    key.value = cause_value;
    res = bsearch(&key, causes, sizeof(causes), sizeof(SGWCauseMapping_t), compare_cause_id);
    if (res == NULL) {
        return "Unknown cause";
    } else {
        return res->name;
    }
}
