#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


#include "TLVEncoder.h"
#include "TLVDecoder.h"
#include "ActivateDedicatedEpsBearerContextReject.h"

int decode_activate_dedicated_eps_bearer_context_reject(activate_dedicated_eps_bearer_context_reject_msg *activate_dedicated_eps_bearer_context_reject, uint8_t *buffer, uint32_t len)
{
    uint32_t decoded = 0;
    int decoded_result = 0;

    // Check if we got a NULL pointer and if buffer length is >= minimum length expected for the message.
    CHECK_PDU_POINTER_AND_LENGTH_DECODER(buffer, ACTIVATE_DEDICATED_EPS_BEARER_CONTEXT_REJECT_MINIMUM_LENGTH, len);

    /* Decoding mandatory fields */
    if ((decoded_result = decode_esm_cause(&activate_dedicated_eps_bearer_context_reject->esmcause, 0, buffer + decoded, len - decoded)) < 0)
        return decoded_result;
    else
        decoded += decoded_result;

    /* Decoding optional fields */
    while(len - decoded > 0)
    {
        uint8_t ieiDecoded = *(buffer + decoded);
        /* Type | value iei are below 0x80 so just return the first 4 bits */
        if (ieiDecoded >= 0x80)
            ieiDecoded = ieiDecoded & 0xf0;
        switch(ieiDecoded)
        {
            case ACTIVATE_DEDICATED_EPS_BEARER_CONTEXT_REJECT_PROTOCOL_CONFIGURATION_OPTIONS_IEI:
                if ((decoded_result =
                    decode_protocol_configuration_options(&activate_dedicated_eps_bearer_context_reject->protocolconfigurationoptions,
                    ACTIVATE_DEDICATED_EPS_BEARER_CONTEXT_REJECT_PROTOCOL_CONFIGURATION_OPTIONS_IEI,
                    buffer + decoded, len - decoded)) <= 0)
                    return decoded_result;
                decoded += decoded_result;
                /* Set corresponding mask to 1 in presencemask */
                activate_dedicated_eps_bearer_context_reject->presencemask |= ACTIVATE_DEDICATED_EPS_BEARER_CONTEXT_REJECT_PROTOCOL_CONFIGURATION_OPTIONS_PRESENT;
                break;
            default:
                errorCodeDecoder = TLV_DECODE_UNEXPECTED_IEI;
                return TLV_DECODE_UNEXPECTED_IEI;
        }
    }
    return decoded;
}

int encode_activate_dedicated_eps_bearer_context_reject(activate_dedicated_eps_bearer_context_reject_msg *activate_dedicated_eps_bearer_context_reject, uint8_t *buffer, uint32_t len)
{
    int encoded = 0;
    int encode_result = 0;

    /* Checking IEI and pointer */
    CHECK_PDU_POINTER_AND_LENGTH_ENCODER(buffer, ACTIVATE_DEDICATED_EPS_BEARER_CONTEXT_REJECT_MINIMUM_LENGTH, len);

    if ((encode_result =
         encode_esm_cause(&activate_dedicated_eps_bearer_context_reject->esmcause,
         0, buffer + encoded, len - encoded)) < 0)        //Return in case of error
        return encode_result;
    else
        encoded += encode_result;

    if ((activate_dedicated_eps_bearer_context_reject->presencemask & ACTIVATE_DEDICATED_EPS_BEARER_CONTEXT_REJECT_PROTOCOL_CONFIGURATION_OPTIONS_PRESENT)
        == ACTIVATE_DEDICATED_EPS_BEARER_CONTEXT_REJECT_PROTOCOL_CONFIGURATION_OPTIONS_PRESENT)
    {
        if ((encode_result =
             encode_protocol_configuration_options(&activate_dedicated_eps_bearer_context_reject->protocolconfigurationoptions,
             ACTIVATE_DEDICATED_EPS_BEARER_CONTEXT_REJECT_PROTOCOL_CONFIGURATION_OPTIONS_IEI,
             buffer + encoded, len - encoded)) < 0)
            // Return in case of error
            return encode_result;
        else
            encoded += encode_result;
    }

    return encoded;
}

