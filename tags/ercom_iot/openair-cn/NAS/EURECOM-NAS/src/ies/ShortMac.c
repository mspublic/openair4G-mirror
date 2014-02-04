#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#include "TLVEncoder.h"
#include "TLVDecoder.h"
#include "ShortMac.h"

int decode_short_mac(ShortMac *shortmac, uint8_t iei, uint8_t *buffer, uint32_t len)
{
    int decoded = 0;
    if (iei > 0)
    {
        CHECK_IEI_DECODER(iei, *buffer);
        decoded++;
    }
    //IES_DECODE_U16(*shortmac, *(buffer + decoded));
    IES_DECODE_U16(buffer, decoded, *shortmac);
#if defined (NAS_DEBUG)
    dump_short_mac_xml(shortmac, iei);
#endif
    return decoded;
}

int encode_short_mac(ShortMac *shortmac, uint8_t iei, uint8_t *buffer, uint32_t len)
{
    uint32_t encoded = 0;
    /* Checking IEI and pointer */
    CHECK_PDU_POINTER_AND_LENGTH_ENCODER(buffer, SHORT_MAC_MINIMUM_LENGTH, len);
#if defined (NAS_DEBUG)
    dump_short_mac_xml(shortmac, iei);
#endif
    if (iei > 0)
    {
        *buffer = iei;
        encoded++;
    }
    IES_ENCODE_U16(buffer, encoded, *shortmac);
    return encoded;
}

void dump_short_mac_xml(ShortMac *shortmac, uint8_t iei)
{
    printf("<Short Mac>\n");
    if (iei > 0)
        /* Don't display IEI if = 0 */
        printf("    <IEI>0x%X</IEI>\n", iei);
    printf("    <Short MAC value>%u</Short MAC value>\n", *shortmac);
    printf("</Short Mac>\n");
}

