#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#include "TLVEncoder.h"
#include "TLVDecoder.h"
#include "AccessPointName.h"

int decode_access_point_name(AccessPointName *accesspointname, uint8_t iei, uint8_t *buffer, uint32_t len)
{
    int decoded = 0;
    uint8_t ielen = 0;
    int decode_result;
    if (iei > 0)
    {
        CHECK_IEI_DECODER(iei, *buffer);
        decoded++;
    }
    ielen = *(buffer + decoded);
    decoded++;
    CHECK_LENGTH_DECODER(len - decoded, ielen);
    if ((decode_result = decode_octet_string(&accesspointname->accesspointnamevalue, ielen, buffer + decoded, len - decoded)) < 0)
        return decode_result;
    else
        decoded += decode_result;
#if defined (NAS_DEBUG)
    dump_access_point_name_xml(accesspointname, iei);
#endif
    return decoded;
}
int encode_access_point_name(AccessPointName *accesspointname, uint8_t iei, uint8_t *buffer, uint32_t len)
{
    uint8_t *lenPtr;
    uint32_t encoded = 0;
    int encode_result;
    /* Checking IEI and pointer */
    CHECK_PDU_POINTER_AND_LENGTH_ENCODER(buffer, ACCESS_POINT_NAME_MINIMUM_LENGTH, len);
#if defined (NAS_DEBUG)
    dump_access_point_name_xml(accesspointname, iei);
#endif
    if (iei > 0)
    {
        *buffer = iei;
        encoded++;
    }
    lenPtr  = (buffer + encoded);
    encoded ++;
    if ((encode_result = encode_octet_string(&accesspointname->accesspointnamevalue, buffer + encoded, len - encoded)) < 0)
        return encode_result;
    else
        encoded += encode_result;
    *lenPtr = encoded - 1 - ((iei > 0) ? 1 : 0);
    return encoded;
}

void dump_access_point_name_xml(AccessPointName *accesspointname, uint8_t iei)
{
    printf("<Access Point Name>\n");
    if (iei > 0)
        /* Don't display IEI if = 0 */
        printf("    <IEI>0x%X</IEI>\n", iei);
    dump_octet_string_xml(&accesspointname->accesspointnamevalue);
    printf("</Access Point Name>\n");
}

