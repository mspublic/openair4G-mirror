#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "ProtocolDiscriminator.h"
#include "SecurityHeaderType.h"
#include "MessageType.h"
#include "IdentityType2.h"

#ifndef IDENTITY_REQUEST_H_
#define IDENTITY_REQUEST_H_

/* Minimum length macro. Formed by minimum length of each mandatory field */
#define IDENTITY_REQUEST_MINIMUM_LENGTH ( \
    IDENTITY_TYPE_2_MINIMUM_LENGTH )

/* Maximum length macro. Formed by maximum length of each field */
#define IDENTITY_REQUEST_MAXIMUM_LENGTH ( \
    IDENTITY_TYPE_2_MAXIMUM_LENGTH )


/*
 * Message name: Identity request
 * Description: This message is sent by the network to the UE to request the UE to provide the specified identity. See table 8.2.18.1.
 * Significance: dual
 * Direction: network to UE
 */

typedef struct identity_request_msg_tag {
    /* Mandatory fields */
    ProtocolDiscriminator          protocoldiscriminator:4;
    SecurityHeaderType             securityheadertype:4;
    MessageType                    messagetype;
    IdentityType2                  identitytype;
} identity_request_msg;

int decode_identity_request(identity_request_msg *identityrequest, uint8_t *buffer, uint32_t len);

int encode_identity_request(identity_request_msg *identityrequest, uint8_t *buffer, uint32_t len);

#endif /* ! defined(IDENTITY_REQUEST_H_) */

