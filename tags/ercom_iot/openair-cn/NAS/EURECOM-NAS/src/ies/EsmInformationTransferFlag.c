#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#include "TLVEncoder.h"
#include "TLVDecoder.h"
#include "EsmInformationTransferFlag.h"

int decode_esm_information_transfer_flag(EsmInformationTransferFlag *esminformationtransferflag, uint8_t iei, uint8_t *buffer, uint32_t len)
{
    int decoded = 0;
    CHECK_PDU_POINTER_AND_LENGTH_DECODER(buffer, ESM_INFORMATION_TRANSFER_FLAG_MINIMUM_LENGTH, len);
    if (iei > 0)
    {
        CHECK_IEI_DECODER((*buffer & 0xf0), iei);
    }
    *esminformationtransferflag = *buffer & 0x1;
    decoded++;
#if defined (NAS_DEBUG)
    dump_esm_information_transfer_flag_xml(esminformationtransferflag, iei);
#endif
    return decoded;
}

int decode_u8_esm_information_transfer_flag(EsmInformationTransferFlag *esminformationtransferflag, uint8_t iei, uint8_t value, uint32_t len)
{
    int decoded = 0;
    uint8_t *buffer = &value;
    *esminformationtransferflag = *buffer & 0x1;
    decoded++;
#if defined (NAS_DEBUG)
    dump_esm_information_transfer_flag_xml(esminformationtransferflag, iei);
#endif
    return decoded;
}

int encode_esm_information_transfer_flag(EsmInformationTransferFlag *esminformationtransferflag, uint8_t iei, uint8_t *buffer, uint32_t len)
{
    uint8_t encoded = 0;
    /* Checking length and pointer */
    CHECK_PDU_POINTER_AND_LENGTH_ENCODER(buffer, ESM_INFORMATION_TRANSFER_FLAG_MINIMUM_LENGTH, len);
#if defined (NAS_DEBUG)
    dump_esm_information_transfer_flag_xml(esminformationtransferflag, iei);
#endif
    *(buffer + encoded) = 0x00 | (iei & 0xf0) |
    (*esminformationtransferflag & 0x1);
    encoded++;
    return encoded;
}

uint8_t encode_u8_esm_information_transfer_flag(EsmInformationTransferFlag *esminformationtransferflag)
{
    uint8_t bufferReturn;
    uint8_t *buffer = &bufferReturn;
    uint8_t encoded = 0;
    uint8_t iei = 0;
    dump_esm_information_transfer_flag_xml(esminformationtransferflag, 0);
    *(buffer + encoded) = 0x00 | (iei & 0xf0) |
    (*esminformationtransferflag & 0x1);
    encoded++;

    return bufferReturn;
}

void dump_esm_information_transfer_flag_xml(EsmInformationTransferFlag *esminformationtransferflag, uint8_t iei)
{
    printf("<Esm Information Transfer Flag>\n");
    if (iei > 0)
        /* Don't display IEI if = 0 */
        printf("    <IEI>0x%X</IEI>\n", iei);
    printf("    <EIT value>%u</EIT value>\n", *esminformationtransferflag);
    printf("</Esm Information Transfer Flag>\n");
}

