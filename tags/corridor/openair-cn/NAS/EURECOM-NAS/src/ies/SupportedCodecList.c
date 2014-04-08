#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#include "TLVEncoder.h"
#include "TLVDecoder.h"
#include "SupportedCodecList.h"

int decode_supported_codec_list(SupportedCodecList *supportedcodeclist, uint8_t iei, uint8_t *buffer, uint32_t len)
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
    supportedcodeclist->systemidentification = *(buffer + decoded);
    decoded++;
    supportedcodeclist->lengthofbitmap = *(buffer + decoded);
    decoded++;
    //IES_DECODE_U16(supportedcodeclist->codecbitmap, *(buffer + decoded));
    IES_DECODE_U16(buffer, decoded, supportedcodeclist->codecbitmap);
#if defined (NAS_DEBUG)
    dump_supported_codec_list_xml(supportedcodeclist, iei);
#endif
    return decoded;
}
int encode_supported_codec_list(SupportedCodecList *supportedcodeclist, uint8_t iei, uint8_t *buffer, uint32_t len)
{
    uint8_t *lenPtr;
    uint32_t encoded = 0;
    /* Checking IEI and pointer */
    CHECK_PDU_POINTER_AND_LENGTH_ENCODER(buffer, SUPPORTED_CODEC_LIST_MINIMUM_LENGTH, len);
#if defined (NAS_DEBUG)
    dump_supported_codec_list_xml(supportedcodeclist, iei);
#endif
    if (iei > 0)
    {
        *buffer = iei;
        encoded++;
    }
    lenPtr  = (buffer + encoded);
    encoded ++;
    *(buffer + encoded) = supportedcodeclist->systemidentification;
    encoded++;
    *(buffer + encoded) = supportedcodeclist->lengthofbitmap;
    encoded++;
    IES_ENCODE_U16(buffer, encoded, supportedcodeclist->codecbitmap);
    *lenPtr = encoded - 1 - ((iei > 0) ? 1 : 0);
    return encoded;
}

void dump_supported_codec_list_xml(SupportedCodecList *supportedcodeclist, uint8_t iei)
{
    printf("<Supported Codec List>\n");
    if (iei > 0)
        /* Don't display IEI if = 0 */
        printf("    <IEI>0x%X</IEI>\n", iei);
    printf("    <System identification>%u</System identification>\n", supportedcodeclist->systemidentification);
    printf("    <Length of bitmap>%u</Length of bitmap>\n", supportedcodeclist->lengthofbitmap);
    printf("    <Codec bitmap>%u</Codec bitmap>\n", supportedcodeclist->codecbitmap);
    printf("</Supported Codec List>\n");
}

