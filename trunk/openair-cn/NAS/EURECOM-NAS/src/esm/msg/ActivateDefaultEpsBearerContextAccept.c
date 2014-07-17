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
#include "ActivateDefaultEpsBearerContextAccept.h"

int decode_activate_default_eps_bearer_context_accept(activate_default_eps_bearer_context_accept_msg *activate_default_eps_bearer_context_accept, uint8_t *buffer, uint32_t len)
{
    uint32_t decoded = 0;
    int decoded_result = 0;

    // Check if we got a NULL pointer and if buffer length is >= minimum length expected for the message.
    CHECK_PDU_POINTER_AND_LENGTH_DECODER(buffer, ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_ACCEPT_MINIMUM_LENGTH, len);

    /* Decoding mandatory fields */
    /* Decoding optional fields */
    while(len - decoded > 0)
    {
        uint8_t ieiDecoded = *(buffer + decoded);
        /* Type | value iei are below 0x80 so just return the first 4 bits */
        if (ieiDecoded >= 0x80)
            ieiDecoded = ieiDecoded & 0xf0;
        switch(ieiDecoded)
        {
            case ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_ACCEPT_PROTOCOL_CONFIGURATION_OPTIONS_IEI:
                if ((decoded_result =
                    decode_protocol_configuration_options(&activate_default_eps_bearer_context_accept->protocolconfigurationoptions,
                    ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_ACCEPT_PROTOCOL_CONFIGURATION_OPTIONS_IEI,
                    buffer + decoded, len - decoded)) <= 0)
                    return decoded_result;
                decoded += decoded_result;
                /* Set corresponding mask to 1 in presencemask */
                activate_default_eps_bearer_context_accept->presencemask |= ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_ACCEPT_PROTOCOL_CONFIGURATION_OPTIONS_PRESENT;
                break;
            default:
                errorCodeDecoder = TLV_DECODE_UNEXPECTED_IEI;
                return TLV_DECODE_UNEXPECTED_IEI;
        }
    }
    return decoded;
}

int encode_activate_default_eps_bearer_context_accept(activate_default_eps_bearer_context_accept_msg *activate_default_eps_bearer_context_accept, uint8_t *buffer, uint32_t len)
{
    int encoded = 0;
    int encode_result = 0;

    /* Checking IEI and pointer */
    CHECK_PDU_POINTER_AND_LENGTH_ENCODER(buffer, ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_ACCEPT_MINIMUM_LENGTH, len);

    if ((activate_default_eps_bearer_context_accept->presencemask & ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_ACCEPT_PROTOCOL_CONFIGURATION_OPTIONS_PRESENT)
        == ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_ACCEPT_PROTOCOL_CONFIGURATION_OPTIONS_PRESENT)
    {
        if ((encode_result =
             encode_protocol_configuration_options(&activate_default_eps_bearer_context_accept->protocolconfigurationoptions,
             ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_ACCEPT_PROTOCOL_CONFIGURATION_OPTIONS_IEI,
             buffer + encoded, len - encoded)) < 0)
            // Return in case of error
            return encode_result;
        else
            encoded += encode_result;
    }

    return encoded;
}

