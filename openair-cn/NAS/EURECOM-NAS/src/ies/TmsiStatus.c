#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#include "TLVEncoder.h"
#include "TLVDecoder.h"
#include "TmsiStatus.h"

int decode_tmsi_status(TmsiStatus *tmsistatus, uint8_t iei, uint8_t *buffer, uint32_t len)
{
    int decoded = 0;
    CHECK_PDU_POINTER_AND_LENGTH_DECODER(buffer, TMSI_STATUS_MINIMUM_LENGTH, len);
    if (iei > 0)
    {
        CHECK_IEI_DECODER((*buffer & 0xf0), iei);
    }
    *tmsistatus = *buffer & 0x1;
    decoded++;
#if defined (NAS_DEBUG)
    dump_tmsi_status_xml(tmsistatus, iei);
#endif
    return decoded;
}

int decode_u8_tmsi_status(TmsiStatus *tmsistatus, uint8_t iei, uint8_t value, uint32_t len)
{
    int decoded = 0;
    uint8_t *buffer = &value;
    *tmsistatus = *buffer & 0x1;
    decoded++;
#if defined (NAS_DEBUG)
    dump_tmsi_status_xml(tmsistatus, iei);
#endif
    return decoded;
}

int encode_tmsi_status(TmsiStatus *tmsistatus, uint8_t iei, uint8_t *buffer, uint32_t len)
{
    uint8_t encoded = 0;
    /* Checking length and pointer */
    CHECK_PDU_POINTER_AND_LENGTH_ENCODER(buffer, TMSI_STATUS_MINIMUM_LENGTH, len);
#if defined (NAS_DEBUG)
    dump_tmsi_status_xml(tmsistatus, iei);
#endif
    *(buffer + encoded) = 0x00 | (iei & 0xf0) |
    (*tmsistatus & 0x1);
    encoded++;
    return encoded;
}

uint8_t encode_u8_tmsi_status(TmsiStatus *tmsistatus)
{
    uint8_t bufferReturn;
    uint8_t *buffer = &bufferReturn;
    uint8_t encoded = 0;
    uint8_t iei = 0;
    dump_tmsi_status_xml(tmsistatus, 0);
    *(buffer + encoded) = 0x00 | (iei & 0xf0) |
    (*tmsistatus & 0x1);
    encoded++;

    return bufferReturn;
}

void dump_tmsi_status_xml(TmsiStatus *tmsistatus, uint8_t iei)
{
    printf("<Tmsi Status>\n");
    if (iei > 0)
        /* Don't display IEI if = 0 */
        printf("    <IEI>0x%X</IEI>\n", iei);
    printf("    <TMSI flag>%u</TMSI flag>\n", *tmsistatus);
    printf("</Tmsi Status>\n");
}
