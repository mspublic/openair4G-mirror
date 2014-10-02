/*******************************************************************************
    OpenAirInterface
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is
   included in this distribution in the file called "COPYING". If not,
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@eurecom.fr

  Address      : Eurecom, Compus SophiaTech 450, route des chappes, 06451 Biot, France.

 *******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "ProtocolDiscriminator.h"
#include "SecurityHeaderType.h"
#include "MessageType.h"
#include "EmmCause.h"
#include "GprsTimer.h"

#ifndef SERVICE_REJECT_H_
#define SERVICE_REJECT_H_

/* Minimum length macro. Formed by minimum length of each mandatory field */
#define SERVICE_REJECT_MINIMUM_LENGTH ( \
    EMM_CAUSE_MINIMUM_LENGTH )

/* Maximum length macro. Formed by maximum length of each field */
#define SERVICE_REJECT_MAXIMUM_LENGTH ( \
    EMM_CAUSE_MAXIMUM_LENGTH + \
    GPRS_TIMER_MAXIMUM_LENGTH )


/*
 * Message name: Service reject
 * Description: This message is sent by the network to the UE in order to reject the service request procedure. See table 8.2.24.1.
 * Significance: dual
 * Direction: network to UE
 */

typedef struct service_reject_msg_tag {
    /* Mandatory fields */
    ProtocolDiscriminator        protocoldiscriminator:4;
    SecurityHeaderType           securityheadertype:4;
    MessageType                  messagetype;
    EmmCause                     emmcause;
    /* Optional fields */
    uint32_t                     presencemask;
    GprsTimer                    t3442value;
} service_reject_msg;

int decode_service_reject(service_reject_msg *servicereject, uint8_t *buffer, uint32_t len);

int encode_service_reject(service_reject_msg *servicereject, uint8_t *buffer, uint32_t len);

#endif /* ! defined(SERVICE_REJECT_H_) */

