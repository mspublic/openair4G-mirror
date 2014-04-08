#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


#include "TLVEncoder.h"
#include "TLVDecoder.h"
#include "PdnConnectivityRequest.h"

int decode_pdn_connectivity_request(pdn_connectivity_request_msg *pdn_connectivity_request, uint8_t *buffer, uint32_t len)
{
    uint32_t decoded = 0;
    int decoded_result = 0;

    // Check if we got a NULL pointer and if buffer length is >= minimum length expected for the message.
    CHECK_PDU_POINTER_AND_LENGTH_DECODER(buffer, PDN_CONNECTIVITY_REQUEST_MINIMUM_LENGTH, len);

    /* Decoding mandatory fields */
    if ((decoded_result = decode_u8_request_type(&pdn_connectivity_request->pdntype, 0, *(buffer + decoded) >> 4, len - decoded)) < 0)
        return decoded_result;

    if ((decoded_result = decode_u8_pdn_type(&pdn_connectivity_request->requesttype, 0, *(buffer + decoded) & 0x0f, len - decoded)) < 0)
        return decoded_result;

    decoded++;
    /* Decoding optional fields */
    while(len - decoded > 0)
    {
        uint8_t ieiDecoded = *(buffer + decoded);
        /* Type | value iei are below 0x80 so just return the first 4 bits */
        if (ieiDecoded >= 0x80)
            ieiDecoded = ieiDecoded & 0xf0;
        switch(ieiDecoded)
        {
            case PDN_CONNECTIVITY_REQUEST_ESM_INFORMATION_TRANSFER_FLAG_IEI:
                if ((decoded_result =
                    decode_esm_information_transfer_flag(&pdn_connectivity_request->esminformationtransferflag,
                    PDN_CONNECTIVITY_REQUEST_ESM_INFORMATION_TRANSFER_FLAG_IEI,
                    buffer + decoded, len - decoded)) <= 0)
                    return decoded_result;
                decoded += decoded_result;
                /* Set corresponding mask to 1 in presencemask */
                pdn_connectivity_request->presencemask |= PDN_CONNECTIVITY_REQUEST_ESM_INFORMATION_TRANSFER_FLAG_PRESENT;
                break;
            case PDN_CONNECTIVITY_REQUEST_ACCESS_POINT_NAME_IEI:
                if ((decoded_result =
                    decode_access_point_name(&pdn_connectivity_request->accesspointname,
                    PDN_CONNECTIVITY_REQUEST_ACCESS_POINT_NAME_IEI, buffer +
                    decoded, len - decoded)) <= 0)
                    return decoded_result;
                decoded += decoded_result;
                /* Set corresponding mask to 1 in presencemask */
                pdn_connectivity_request->presencemask |= PDN_CONNECTIVITY_REQUEST_ACCESS_POINT_NAME_PRESENT;
                break;
            case PDN_CONNECTIVITY_REQUEST_PROTOCOL_CONFIGURATION_OPTIONS_IEI:
                if ((decoded_result =
                    decode_protocol_configuration_options(&pdn_connectivity_request->protocolconfigurationoptions,
                    PDN_CONNECTIVITY_REQUEST_PROTOCOL_CONFIGURATION_OPTIONS_IEI,
                    buffer + decoded, len - decoded)) <= 0)
                    return decoded_result;
                decoded += decoded_result;
                /* Set corresponding mask to 1 in presencemask */
                pdn_connectivity_request->presencemask |= PDN_CONNECTIVITY_REQUEST_PROTOCOL_CONFIGURATION_OPTIONS_PRESENT;
                break;
            default:
                errorCodeDecoder = TLV_DECODE_UNEXPECTED_IEI;
                return TLV_DECODE_UNEXPECTED_IEI;
        }
    }
    return decoded;
}

int encode_pdn_connectivity_request(pdn_connectivity_request_msg *pdn_connectivity_request, uint8_t *buffer, uint32_t len)
{
    int encoded = 0;
    int encode_result = 0;

    /* Checking IEI and pointer */
    CHECK_PDU_POINTER_AND_LENGTH_ENCODER(buffer, PDN_CONNECTIVITY_REQUEST_MINIMUM_LENGTH, len);

    *(buffer + encoded) = ((encode_u8_pdn_type(&pdn_connectivity_request->pdntype) & 0x0f) << 4) | (encode_u8_request_type(&pdn_connectivity_request->requesttype) & 0x0f);
    encoded++;

    if ((pdn_connectivity_request->presencemask & PDN_CONNECTIVITY_REQUEST_ESM_INFORMATION_TRANSFER_FLAG_PRESENT)
        == PDN_CONNECTIVITY_REQUEST_ESM_INFORMATION_TRANSFER_FLAG_PRESENT)
    {
        if ((encode_result =
             encode_esm_information_transfer_flag(&pdn_connectivity_request->esminformationtransferflag,
             PDN_CONNECTIVITY_REQUEST_ESM_INFORMATION_TRANSFER_FLAG_IEI, buffer
             + encoded, len - encoded)) < 0)
            // Return in case of error
            return encode_result;
        else
            encoded += encode_result;
    }

    if ((pdn_connectivity_request->presencemask & PDN_CONNECTIVITY_REQUEST_ACCESS_POINT_NAME_PRESENT)
        == PDN_CONNECTIVITY_REQUEST_ACCESS_POINT_NAME_PRESENT)
    {
        if ((encode_result =
             encode_access_point_name(&pdn_connectivity_request->accesspointname,
             PDN_CONNECTIVITY_REQUEST_ACCESS_POINT_NAME_IEI, buffer + encoded,
             len - encoded)) < 0)
            // Return in case of error
            return encode_result;
        else
            encoded += encode_result;
    }

    if ((pdn_connectivity_request->presencemask & PDN_CONNECTIVITY_REQUEST_PROTOCOL_CONFIGURATION_OPTIONS_PRESENT)
        == PDN_CONNECTIVITY_REQUEST_PROTOCOL_CONFIGURATION_OPTIONS_PRESENT)
    {
        if ((encode_result =
             encode_protocol_configuration_options(&pdn_connectivity_request->protocolconfigurationoptions,
             PDN_CONNECTIVITY_REQUEST_PROTOCOL_CONFIGURATION_OPTIONS_IEI,
             buffer + encoded, len - encoded)) < 0)
            // Return in case of error
            return encode_result;
        else
            encoded += encode_result;
    }

    return encoded;
}

