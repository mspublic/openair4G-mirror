#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "ProtocolDiscriminator.h"
#include "SecurityHeaderType.h"
#include "MessageType.h"
#include "EpsMobileIdentity.h"
#include "TrackingAreaIdentityList.h"

#ifndef GUTI_REALLOCATION_COMMAND_H_
#define GUTI_REALLOCATION_COMMAND_H_

/* Minimum length macro. Formed by minimum length of each mandatory field */
#define GUTI_REALLOCATION_COMMAND_MINIMUM_LENGTH ( \
    EPS_MOBILE_IDENTITY_MINIMUM_LENGTH )

/* Maximum length macro. Formed by maximum length of each field */
#define GUTI_REALLOCATION_COMMAND_MAXIMUM_LENGTH ( \
    EPS_MOBILE_IDENTITY_MAXIMUM_LENGTH + \
    TRACKING_AREA_IDENTITY_LIST_MAXIMUM_LENGTH )

/* If an optional value is present and should be encoded, the corresponding
 * Bit mask should be set to 1.
 */
# define GUTI_REALLOCATION_COMMAND_TAI_LIST_PRESENT (1<<0)

typedef enum guti_reallocation_command_iei_tag {
    GUTI_REALLOCATION_COMMAND_TAI_LIST_IEI  = 0x54, /* 0x54 = 84 */
} guti_reallocation_command_iei;

/*
 * Message name: GUTI reallocation command
 * Description: This message is sent by the network to the UE to reallocate a GUTI and optionally to provide a new TAI list. See table 8.2.16.1.
 * Significance: dual
 * Direction: network to UE
 */

typedef struct guti_reallocation_command_msg_tag {
    /* Mandatory fields */
    ProtocolDiscriminator                  protocoldiscriminator:4;
    SecurityHeaderType                     securityheadertype:4;
    MessageType                            messagetype;
    EpsMobileIdentity                      guti;
    /* Optional fields */
    uint32_t                               presencemask;
    TrackingAreaIdentityList               tailist;
} guti_reallocation_command_msg;

int decode_guti_reallocation_command(guti_reallocation_command_msg *gutireallocationcommand, uint8_t *buffer, uint32_t len);

int encode_guti_reallocation_command(guti_reallocation_command_msg *gutireallocationcommand, uint8_t *buffer, uint32_t len);

#endif /* ! defined(GUTI_REALLOCATION_COMMAND_H_) */

