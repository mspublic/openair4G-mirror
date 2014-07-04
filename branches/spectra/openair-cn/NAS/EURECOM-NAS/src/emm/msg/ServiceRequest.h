#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "ProtocolDiscriminator.h"
#include "SecurityHeaderType.h"
#include "KsiAndSequenceNumber.h"
#include "ShortMac.h"

#ifndef SERVICE_REQUEST_H_
#define SERVICE_REQUEST_H_

/* Minimum length macro. Formed by minimum length of each mandatory field */
#define SERVICE_REQUEST_MINIMUM_LENGTH ( \
    KSI_AND_SEQUENCE_NUMBER_MINIMUM_LENGTH + \
    SHORT_MAC_MINIMUM_LENGTH )

/* Maximum length macro. Formed by maximum length of each field */
#define SERVICE_REQUEST_MAXIMUM_LENGTH ( \
    KSI_AND_SEQUENCE_NUMBER_MAXIMUM_LENGTH + \
    SHORT_MAC_MAXIMUM_LENGTH )


/*
 * Message name: Service request
 * Description: This message is sent by the UE to the network to request the establishment of a NAS signalling connection and of the radio and S1 bearers. Its structure does not follow the structure of a standard layer 3 message. See table 8.2.25.1.
 * Significance: dual
 * Direction: UE to network
 */

typedef struct service_request_msg_tag {
    /* Mandatory fields */
    ProtocolDiscriminator     protocoldiscriminator:4;
    SecurityHeaderType        securityheadertype:4;
    KsiAndSequenceNumber      ksiandsequencenumber;
    ShortMac                  messageauthenticationcode;
} service_request_msg;

int decode_service_request(service_request_msg *servicerequest, uint8_t *buffer, uint32_t len);

int encode_service_request(service_request_msg *servicerequest, uint8_t *buffer, uint32_t len);

#endif /* ! defined(SERVICE_REQUEST_H_) */

