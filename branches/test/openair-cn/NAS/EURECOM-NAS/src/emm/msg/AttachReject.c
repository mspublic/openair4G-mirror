#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


#include "TLVEncoder.h"
#include "TLVDecoder.h"
#include "AttachReject.h"

int decode_attach_reject(attach_reject_msg *attach_reject, uint8_t *buffer, uint32_t len)
{
    uint32_t decoded = 0;
    int decoded_result = 0;

    // Check if we got a NULL pointer and if buffer length is >= minimum length expected for the message.
    CHECK_PDU_POINTER_AND_LENGTH_DECODER(buffer, ATTACH_REJECT_MINIMUM_LENGTH, len);

    /* Decoding mandatory fields */
    if ((decoded_result = decode_emm_cause(&attach_reject->emmcause, 0, buffer + decoded, len - decoded)) < 0)
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
            case ATTACH_REJECT_ESM_MESSAGE_CONTAINER_IEI:
                if ((decoded_result =
                    decode_esm_message_container(&attach_reject->esmmessagecontainer,
                    ATTACH_REJECT_ESM_MESSAGE_CONTAINER_IEI, buffer + decoded,
                    len - decoded)) <= 0)
                    return decoded_result;
                decoded += decoded_result;
                /* Set corresponding mask to 1 in presencemask */
                attach_reject->presencemask |= ATTACH_REJECT_ESM_MESSAGE_CONTAINER_PRESENT;
                break;
            default:
                errorCodeDecoder = TLV_DECODE_UNEXPECTED_IEI;
                return TLV_DECODE_UNEXPECTED_IEI;
        }
    }
    return decoded;
}

int encode_attach_reject(attach_reject_msg *attach_reject, uint8_t *buffer, uint32_t len)
{
    int encoded = 0;
    int encode_result = 0;

    /* Checking IEI and pointer */
    CHECK_PDU_POINTER_AND_LENGTH_ENCODER(buffer, ATTACH_REJECT_MINIMUM_LENGTH, len);

    if ((encode_result = encode_emm_cause(&attach_reject->emmcause, 0, buffer +
         encoded, len - encoded)) < 0)        //Return in case of error
        return encode_result;
    else
        encoded += encode_result;

    if ((attach_reject->presencemask & ATTACH_REJECT_ESM_MESSAGE_CONTAINER_PRESENT)
        == ATTACH_REJECT_ESM_MESSAGE_CONTAINER_PRESENT)
    {
        if ((encode_result =
             encode_esm_message_container(&attach_reject->esmmessagecontainer,
             ATTACH_REJECT_ESM_MESSAGE_CONTAINER_IEI, buffer + encoded, len -
             encoded)) < 0)
            // Return in case of error
            return encode_result;
        else
            encoded += encode_result;
    }

    return encoded;
}

