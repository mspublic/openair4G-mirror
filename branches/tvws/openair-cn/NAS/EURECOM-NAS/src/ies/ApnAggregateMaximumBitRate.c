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
    printf("</Apn Aggregate Maximum Bit Rate>\n");
}

