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


#include "TLVEncoder.h"
#include "TLVDecoder.h"
#include "ApnAggregateMaximumBitRate.h"

int decode_apn_aggregate_maximum_bit_rate(ApnAggregateMaximumBitRate *apnaggregatemaximumbitrate, uint8_t iei, uint8_t *buffer, uint32_t len)
{
    int decoded = 0;
    uint8_t ielen = 0;
    if (iei > 0)
    {
        CHECK_IEI_DECODER(iei, *buffer);
        decoded++;
    }
    ielen = *(buffer + decoded);
    decoded++;
    CHECK_LENGTH_DECODER(len - decoded, ielen);
    apnaggregatemaximumbitrate->apnambrfordownlink = *(buffer + decoded);
    decoded++;
    apnaggregatemaximumbitrate->apnambrforuplink = *(buffer + decoded);
    decoded++;
    if (ielen >= 4) {
        apnaggregatemaximumbitrate->apnambrfordownlink_extended = *(buffer + decoded);
        decoded++;
        apnaggregatemaximumbitrate->apnambrforuplink_extended = *(buffer + decoded);
        decoded++;
        if (ielen >= 6) {
            apnaggregatemaximumbitrate->apnambrfordownlink_extended2 = *(buffer + decoded);
            decoded++;
            apnaggregatemaximumbitrate->apnambrforuplink_extended2 = *(buffer + decoded);
            decoded++;
        }
    }
#if defined (NAS_DEBUG)
    dump_apn_aggregate_maximum_bit_rate_xml(apnaggregatemaximumbitrate, iei);
#endif
    return decoded;
}
int encode_apn_aggregate_maximum_bit_rate(ApnAggregateMaximumBitRate *apnaggregatemaximumbitrate, uint8_t iei, uint8_t *buffer, uint32_t len)
{
    uint8_t *lenPtr;
    uint32_t encoded = 0;
    /* Checking IEI and pointer */
    CHECK_PDU_POINTER_AND_LENGTH_ENCODER(buffer, APN_AGGREGATE_MAXIMUM_BIT_RATE_MINIMUM_LENGTH, len);
#if defined (NAS_DEBUG)
    dump_apn_aggregate_maximum_bit_rate_xml(apnaggregatemaximumbitrate, iei);
#endif
    if (iei > 0)
    {
        *buffer = iei;
        encoded++;
    }
    lenPtr  = (buffer + encoded);
    encoded ++;
    *(buffer + encoded) = apnaggregatemaximumbitrate->apnambrfordownlink;
    encoded++;
    *(buffer + encoded) = apnaggregatemaximumbitrate->apnambrforuplink;
    encoded++;
    if (apnaggregatemaximumbitrate->extensions & APN_AGGREGATE_MAXIMUM_BIT_RATE_MAXIMUM_EXTENSION_PRESENT) {
        *(buffer + encoded) = apnaggregatemaximumbitrate->apnambrfordownlink_extended;
        encoded++;
        *(buffer + encoded) = apnaggregatemaximumbitrate->apnambrforuplink_extended;
        encoded++;
        if (apnaggregatemaximumbitrate->extensions & APN_AGGREGATE_MAXIMUM_BIT_RATE_MAXIMUM_EXTENSION2_PRESENT) {
            *(buffer + encoded) = apnaggregatemaximumbitrate->apnambrfordownlink_extended2;
            encoded++;
            *(buffer + encoded) = apnaggregatemaximumbitrate->apnambrforuplink_extended2;
            encoded++;
        }
    }
    *lenPtr = encoded - 1 - ((iei > 0) ? 1 : 0);
    return encoded;
}

void dump_apn_aggregate_maximum_bit_rate_xml(ApnAggregateMaximumBitRate *apnaggregatemaximumbitrate, uint8_t iei)
{
    printf("<Apn Aggregate Maximum Bit Rate>\n");
    if (iei > 0)
        /* Don't display IEI if = 0 */
        printf("    <IEI>0x%X</IEI>\n", iei);
    printf("    <APN AMBR for downlink>%u</APN AMBR for downlink>\n", apnaggregatemaximumbitrate->apnambrfordownlink);
    printf("    <APN AMBR for uplink>%u</APN AMBR for uplink>\n", apnaggregatemaximumbitrate->apnambrforuplink);
    if (apnaggregatemaximumbitrate->extensions & APN_AGGREGATE_MAXIMUM_BIT_RATE_MAXIMUM_EXTENSION_PRESENT) {
        printf("    <APN AMBR extended for downlink>%u</APN AMBR for downlink>\n", apnaggregatemaximumbitrate->apnambrfordownlink_extended);
        printf("    <APN AMBR extended for uplink>%u</APN AMBR for uplink>\n", apnaggregatemaximumbitrate->apnambrforuplink_extended);
        if (apnaggregatemaximumbitrate->extensions & APN_AGGREGATE_MAXIMUM_BIT_RATE_MAXIMUM_EXTENSION2_PRESENT) {
            printf("    <APN AMBR extended2 for downlink>%u</APN AMBR for downlink>\n", apnaggregatemaximumbitrate->apnambrfordownlink_extended);
            printf("    <APN AMBR extended2 for uplink>%u</APN AMBR for uplink>\n", apnaggregatemaximumbitrate->apnambrforuplink_extended);
        }
    }
    printf("</Apn Aggregate Maximum Bit Rate>\n");
}

