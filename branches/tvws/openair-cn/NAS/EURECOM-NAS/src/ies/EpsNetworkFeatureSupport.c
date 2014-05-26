#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#include "TLVEncoder.h"
#include "TLVDecoder.h"
#include "EpsNetworkFeatureSupport.h"

int decode_eps_network_feature_support(EpsNetworkFeatureSupport *epsnetworkfeaturesupport, uint8_t iei, uint8_t *buffer, uint32_t len)
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
    *epsnetworkfeaturesupport = *buffer & 0x1;
    decoded++;
#if defined (NAS_DEBUG)
    dump_eps_network_feature_support_xml(epsnetworkfeaturesupport, iei);
#endif
    return decoded;
}
int encode_eps_network_feature_support(EpsNetworkFeatureSupport *epsnetworkfeaturesupport, uint8_t iei, uint8_t *buffer, uint32_t len)
{
    uint8_t *lenPtr;
    uint32_t encoded = 0;
    /* Checking IEI and pointer */
    CHECK_PDU_POINTER_AND_LENGTH_ENCODER(buffer, EPS_NETWORK_FEATURE_SUPPORT_MINIMUM_LENGTH, len);
#if defined (NAS_DEBUG)
    dump_eps_network_feature_support_xml(epsnetworkfeaturesupport, iei);
#endif
    if (iei > 0)
    {
        *buffer = iei;
        encoded++;
    }
    lenPtr  = (buffer + encoded);
    encoded ++;
    *(buffer + encoded) = 0x00 |
    (*epsnetworkfeaturesupport & 0x1);
    encoded++;
    *lenPtr = encoded - 1 - ((iei > 0) ? 1 : 0);
    return encoded;
}

void dump_eps_network_feature_support_xml(EpsNetworkFeatureSupport *epsnetworkfeaturesupport, uint8_t iei)
{
    printf("<Eps Network Feature Support>\n");
    if (iei > 0)
        /* Don't display IEI if = 0 */
        printf("    <IEI>0x%X</IEI>\n", iei);
    printf("    <IMS VoPS>%u</IMS VoPS>\n", *epsnetworkfeaturesupport);
    printf("</Eps Network Feature Support>\n");
}

