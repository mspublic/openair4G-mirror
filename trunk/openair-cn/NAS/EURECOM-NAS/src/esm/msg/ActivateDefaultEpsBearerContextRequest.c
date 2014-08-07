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
#include <string.h>
#include <stdint.h>


#include "TLVEncoder.h"
#include "TLVDecoder.h"
#include "ActivateDefaultEpsBearerContextRequest.h"

int decode_activate_default_eps_bearer_context_request(activate_default_eps_bearer_context_request_msg *activate_default_eps_bearer_context_request, uint8_t *buffer, uint32_t len)
{
    uint32_t decoded = 0;
    int decoded_result = 0;

    // Check if we got a NULL pointer and if buffer length is >= minimum length expected for the message.
    CHECK_PDU_POINTER_AND_LENGTH_DECODER(buffer, ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_MINIMUM_LENGTH, len);

    /* Decoding mandatory fields */
    if ((decoded_result = decode_eps_quality_of_service(&activate_default_eps_bearer_context_request->epsqos, 0, buffer + decoded, len - decoded)) < 0)
        return decoded_result;
    else
        decoded += decoded_result;

    if ((decoded_result = decode_access_point_name(&activate_default_eps_bearer_context_request->accesspointname, 0, buffer + decoded, len - decoded)) < 0)
        return decoded_result;
    else
        decoded += decoded_result;

    if ((decoded_result = decode_pdn_address(&activate_default_eps_bearer_context_request->pdnaddress, 0, buffer + decoded, len - decoded)) < 0)
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
            case ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_TRANSACTION_IDENTIFIER_IEI:
                if ((decoded_result =
                    decode_transaction_identifier(&activate_default_eps_bearer_context_request->transactionidentifier,
                    ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_TRANSACTION_IDENTIFIER_IEI,
                    buffer + decoded, len - decoded)) <= 0)
                    return decoded_result;
                decoded += decoded_result;
                /* Set corresponding mask to 1 in presencemask */
                activate_default_eps_bearer_context_request->presencemask |= ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_TRANSACTION_IDENTIFIER_PRESENT;
                break;
            case ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_NEGOTIATED_QOS_IEI:
                if ((decoded_result =
                    decode_quality_of_service(&activate_default_eps_bearer_context_request->negotiatedqos,
                    ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_NEGOTIATED_QOS_IEI,
                    buffer + decoded, len - decoded)) <= 0)
                    return decoded_result;
                decoded += decoded_result;
                /* Set corresponding mask to 1 in presencemask */
                activate_default_eps_bearer_context_request->presencemask |= ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_NEGOTIATED_QOS_PRESENT;
                break;
            case ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_NEGOTIATED_LLC_SAPI_IEI:
                if ((decoded_result =
                    decode_llc_service_access_point_identifier(&activate_default_eps_bearer_context_request->negotiatedllcsapi,
                    ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_NEGOTIATED_LLC_SAPI_IEI,
                    buffer + decoded, len - decoded)) <= 0)
                    return decoded_result;
                decoded += decoded_result;
                /* Set corresponding mask to 1 in presencemask */
                activate_default_eps_bearer_context_request->presencemask |= ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_NEGOTIATED_LLC_SAPI_PRESENT;
                break;
            case ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_RADIO_PRIORITY_IEI:
                if ((decoded_result =
                    decode_radio_priority(&activate_default_eps_bearer_context_request->radiopriority,
                    ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_RADIO_PRIORITY_IEI,
                    buffer + decoded, len - decoded)) <= 0)
                    return decoded_result;
                decoded += decoded_result;
                /* Set corresponding mask to 1 in presencemask */
                activate_default_eps_bearer_context_request->presencemask |= ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_RADIO_PRIORITY_PRESENT;
                break;
            case ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_PACKET_FLOW_IDENTIFIER_IEI:
                if ((decoded_result =
                    decode_packet_flow_identifier(&activate_default_eps_bearer_context_request->packetflowidentifier,
                    ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_PACKET_FLOW_IDENTIFIER_IEI,
                    buffer + decoded, len - decoded)) <= 0)
                    return decoded_result;
                decoded += decoded_result;
                /* Set corresponding mask to 1 in presencemask */
                activate_default_eps_bearer_context_request->presencemask |= ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_PACKET_FLOW_IDENTIFIER_PRESENT;
                break;
            case ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_APNAMBR_IEI:
                if ((decoded_result =
                    decode_apn_aggregate_maximum_bit_rate(&activate_default_eps_bearer_context_request->apnambr,
                    ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_APNAMBR_IEI,
                    buffer + decoded, len - decoded)) <= 0)
                    return decoded_result;
                decoded += decoded_result;
                /* Set corresponding mask to 1 in presencemask */
                activate_default_eps_bearer_context_request->presencemask |= ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_APNAMBR_PRESENT;
                break;
            case ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_ESM_CAUSE_IEI:
                if ((decoded_result =
                    decode_esm_cause(&activate_default_eps_bearer_context_request->esmcause,
                    ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_ESM_CAUSE_IEI,
                    buffer + decoded, len - decoded)) <= 0)
                    return decoded_result;
                decoded += decoded_result;
                /* Set corresponding mask to 1 in presencemask */
                activate_default_eps_bearer_context_request->presencemask |= ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_ESM_CAUSE_PRESENT;
                break;
            case ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_PROTOCOL_CONFIGURATION_OPTIONS_IEI:
                if ((decoded_result =
                    decode_protocol_configuration_options(&activate_default_eps_bearer_context_request->protocolconfigurationoptions,
                    ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_PROTOCOL_CONFIGURATION_OPTIONS_IEI,
                    buffer + decoded, len - decoded)) <= 0)
                    return decoded_result;
                decoded += decoded_result;
                /* Set corresponding mask to 1 in presencemask */
                activate_default_eps_bearer_context_request->presencemask |= ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_PROTOCOL_CONFIGURATION_OPTIONS_PRESENT;
                break;
            default:
                errorCodeDecoder = TLV_DECODE_UNEXPECTED_IEI;
                return TLV_DECODE_UNEXPECTED_IEI;
        }
    }
    return decoded;
}

int encode_activate_default_eps_bearer_context_request(activate_default_eps_bearer_context_request_msg *activate_default_eps_bearer_context_request, uint8_t *buffer, uint32_t len)
{
    int encoded = 0;
    int encode_result = 0;

    /* Checking IEI and pointer */
    CHECK_PDU_POINTER_AND_LENGTH_ENCODER(buffer, ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_MINIMUM_LENGTH, len);

    if ((encode_result =
         encode_eps_quality_of_service(&activate_default_eps_bearer_context_request->epsqos,
         0, buffer + encoded, len - encoded)) < 0) {       //Return in case of error
        LOG_TRACE(ERROR, "ESM  ENCODE epsqos");
        return encode_result;
    } else
        encoded += encode_result;

    if ((encode_result =
         encode_access_point_name(&activate_default_eps_bearer_context_request->accesspointname,
         0, buffer + encoded, len - encoded)) < 0) {       //Return in case of error
        LOG_TRACE(ERROR, "ESM  ENCODE accesspointname");
        return encode_result;
    } else
        encoded += encode_result;

    if ((encode_result =
         encode_pdn_address(&activate_default_eps_bearer_context_request->pdnaddress,
         0, buffer + encoded, len - encoded)) < 0) {       //Return in case of error
        LOG_TRACE(ERROR, "ESM  ENCODE pdnaddress");
        return encode_result;
    } else
        encoded += encode_result;

    if ((activate_default_eps_bearer_context_request->presencemask & ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_TRANSACTION_IDENTIFIER_PRESENT)
        == ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_TRANSACTION_IDENTIFIER_PRESENT)
    {
        if ((encode_result =
             encode_transaction_identifier(&activate_default_eps_bearer_context_request->transactionidentifier,
             ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_TRANSACTION_IDENTIFIER_IEI,
             buffer + encoded, len - encoded)) < 0) {
            LOG_TRACE(ERROR, "ESM  ENCODE transactionidentifier");
            // Return in case of error
            return encode_result;
        } else
            encoded += encode_result;
    }

    if ((activate_default_eps_bearer_context_request->presencemask & ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_NEGOTIATED_QOS_PRESENT)
        == ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_NEGOTIATED_QOS_PRESENT)
    {
        if ((encode_result =
             encode_quality_of_service(&activate_default_eps_bearer_context_request->negotiatedqos,
             ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_NEGOTIATED_QOS_IEI,
             buffer + encoded, len - encoded)) < 0) {
            LOG_TRACE(ERROR, "ESM  ENCODE negotiatedqos");
            // Return in case of error
            return encode_result;
        } else
            encoded += encode_result;
    }

    if ((activate_default_eps_bearer_context_request->presencemask & ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_NEGOTIATED_LLC_SAPI_PRESENT)
        == ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_NEGOTIATED_LLC_SAPI_PRESENT)
    {
        if ((encode_result =
             encode_llc_service_access_point_identifier(&activate_default_eps_bearer_context_request->negotiatedllcsapi,
             ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_NEGOTIATED_LLC_SAPI_IEI,
             buffer + encoded, len - encoded)) < 0) {
            LOG_TRACE(ERROR, "ESM  ENCODE negotiatedllcsapi");
            // Return in case of error
            return encode_result;
        } else
            encoded += encode_result;
    }

    if ((activate_default_eps_bearer_context_request->presencemask & ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_RADIO_PRIORITY_PRESENT)
        == ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_RADIO_PRIORITY_PRESENT)
    {
        if ((encode_result =
             encode_radio_priority(&activate_default_eps_bearer_context_request->radiopriority,
             ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_RADIO_PRIORITY_IEI,
             buffer + encoded, len - encoded)) < 0) {
            LOG_TRACE(ERROR, "ESM  ENCODE radiopriority");
            // Return in case of error
            return encode_result;
        } else
            encoded += encode_result;
    }

    if ((activate_default_eps_bearer_context_request->presencemask & ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_PACKET_FLOW_IDENTIFIER_PRESENT)
        == ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_PACKET_FLOW_IDENTIFIER_PRESENT)
    {
        if ((encode_result =
             encode_packet_flow_identifier(&activate_default_eps_bearer_context_request->packetflowidentifier,
             ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_PACKET_FLOW_IDENTIFIER_IEI,
             buffer + encoded, len - encoded)) < 0) {
            LOG_TRACE(ERROR, "ESM  ENCODE packetflowidentifier");
            // Return in case of error
            return encode_result;
        } else
            encoded += encode_result;
    }

    if ((activate_default_eps_bearer_context_request->presencemask & ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_APNAMBR_PRESENT)
        == ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_APNAMBR_PRESENT)
    {
        if ((encode_result =
             encode_apn_aggregate_maximum_bit_rate(&activate_default_eps_bearer_context_request->apnambr,
             ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_APNAMBR_IEI, buffer +
             encoded, len - encoded)) < 0) {
            LOG_TRACE(ERROR, "ESM  ENCODE apnambr");
            // Return in case of error
            return encode_result;
        } else
            encoded += encode_result;
    }

    if ((activate_default_eps_bearer_context_request->presencemask & ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_ESM_CAUSE_PRESENT)
        == ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_ESM_CAUSE_PRESENT)
    {
        if ((encode_result =
             encode_esm_cause(&activate_default_eps_bearer_context_request->esmcause,
             ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_ESM_CAUSE_IEI, buffer
             + encoded, len - encoded)) < 0) {
            LOG_TRACE(ERROR, "ESM  ENCODE esmcause");
            // Return in case of error
            return encode_result;
        } else
            encoded += encode_result;
    }

#if 1 /* LW: force Protocol Configuration Options to be included in the ESM message */
    {
#define CONFIGURATION_PROTOCOL_PPP          0

#define PROTOCOL_ID_IPCP                    0x8021
#define PROTOCOL_ID_DNS_SERVER_IPV4_ADDRESS 0x000D

        /* Force this item to be present */
        activate_default_eps_bearer_context_request->presencemask |= ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_PROTOCOL_CONFIGURATION_OPTIONS_PRESENT;
        /* Fill this item with data from PFT trace */
        activate_default_eps_bearer_context_request->protocolconfigurationoptions.configurationprotol = CONFIGURATION_PROTOCOL_PPP;
        activate_default_eps_bearer_context_request->protocolconfigurationoptions.protocolid = PROTOCOL_ID_IPCP;
        activate_default_eps_bearer_context_request->protocolconfigurationoptions.lengthofprotocolid = 16; /* Size of PROTOCOL_ID_IPCP */
        activate_default_eps_bearer_context_request->protocolconfigurationoptions.protocolidcontents.value = (uint8_t *)
                /* PROTOCOL_ID_IPCP data */
                "\x03\x00\x00\x10\x81\x06\xC0\xA8\x0C\x64\x83\x06\xC0\xA8\x6A\x0C";
                /* Additional parameters PROTOCOL_ID_DNS_SERVER_IPV4_ADDRESS data */
                //"\x00\x0d\x04\x52\x61\x00\x78"
                /* Additional parameters PROTOCOL_ID_DNS_SERVER_IPV4_ADDRESS data */
                //"\x00\x0d\x04\x52\x61\x01\x78";
        //activate_default_eps_bearer_context_request->protocolconfigurationoptions.protocolidcontents.length = 16 + 7 + 7;
        activate_default_eps_bearer_context_request->protocolconfigurationoptions.protocolidcontents.length = 16;
    }
#endif

    if ((activate_default_eps_bearer_context_request->presencemask & ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_PROTOCOL_CONFIGURATION_OPTIONS_PRESENT)
        == ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_PROTOCOL_CONFIGURATION_OPTIONS_PRESENT)
    {
        if ((encode_result =
             encode_protocol_configuration_options(&activate_default_eps_bearer_context_request->protocolconfigurationoptions,
             ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_PROTOCOL_CONFIGURATION_OPTIONS_IEI,
             buffer + encoded, len - encoded)) < 0) {
            LOG_TRACE(ERROR, "ESM  ENCODE protocolconfigurationoptions");
            // Return in case of error
            return encode_result;
        } else
            encoded += encode_result;
    }
    LOG_TRACE(INFO, "ESM  ENCODED activate_default_eps_bearer_context_request");
    return encoded;
}

