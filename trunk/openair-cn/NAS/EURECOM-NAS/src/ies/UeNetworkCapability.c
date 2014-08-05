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
#include "UeNetworkCapability.h"

int decode_ue_network_capability(UeNetworkCapability *uenetworkcapability, uint8_t iei, uint8_t *buffer, uint32_t len)
{
    int decoded = 0;
    uint8_t ielen = 0;
    if (iei > 0)
    {
        CHECK_IEI_DECODER(iei, *buffer);
        decoded++;
    }
    DECODE_U8(buffer + decoded, ielen, decoded);

    LOG_TRACE(INFO, "decode_ue_network_capability len = %d",ielen);
    CHECK_LENGTH_DECODER(len - decoded, ielen);
    uenetworkcapability->eea = *(buffer + decoded);
    decoded++;
    uenetworkcapability->eia = *(buffer + decoded);
    decoded++;

    /* Parts below not mandatory and may not be present */
    if (ielen > 2) {
        uenetworkcapability->uea = *(buffer + decoded);
        decoded++;

        if (ielen > 3) {
            uenetworkcapability->ucs2 = (*(buffer + decoded) >> 7) & 0x1;
            uenetworkcapability->uia = *(buffer + decoded) & 0x7f;
            decoded++;

            if (ielen > 4) {
                uenetworkcapability->spare = (*(buffer + decoded) >> 5) & 0x7;
                uenetworkcapability->csfb  = (*(buffer + decoded) >> 4) & 0x1;
                uenetworkcapability->lpp   = (*(buffer + decoded) >> 3) & 0x1;
                uenetworkcapability->lcs   = (*(buffer + decoded) >> 2) & 0x1;
                uenetworkcapability->srvcc = (*(buffer + decoded) >> 1) & 0x1;
                uenetworkcapability->nf    = *(buffer + decoded) & 0x1;
                decoded++;
            }
        }
    }
    LOG_TRACE(INFO, "uenetworkcapability decoded=%u\n", decoded);

    if ((ielen + 2) != decoded) {
        decoded = ielen + 1 + (iei > 0 ? 1 : 0) /* Size of header for this IE */;
        LOG_TRACE(INFO, "uenetworkcapability then decoded=%u\n", decoded);
    }

#if defined (NAS_DEBUG)
    dump_ue_network_capability_xml(uenetworkcapability, iei);
#endif
    return decoded;
}
int encode_ue_network_capability(UeNetworkCapability *uenetworkcapability, uint8_t iei, uint8_t *buffer, uint32_t len)
{
    uint8_t *lenPtr;
    uint32_t encoded = 0;
    /* Checking IEI and pointer */
    CHECK_PDU_POINTER_AND_LENGTH_ENCODER(buffer, UE_NETWORK_CAPABILITY_MINIMUM_LENGTH, len);
#if defined (NAS_DEBUG)
    dump_ue_network_capability_xml(uenetworkcapability, iei);
#endif
    if (iei > 0)
    {
        *buffer = iei;
        encoded++;
    }
    lenPtr  = (buffer + encoded);
    encoded ++;
    *(buffer + encoded) = uenetworkcapability->eea;
    encoded++;
    *(buffer + encoded) = uenetworkcapability->eia;
    encoded++;
    *(buffer + encoded) = uenetworkcapability->uea;
    encoded++;
    *(buffer + encoded) = 0x00 | ((uenetworkcapability->ucs2 & 0x1) << 7) |
    (uenetworkcapability->uia & 0x7f);
    encoded++;

    *(buffer + encoded) = 0x00 |
	//((uenetworkcapability->spare & 0x7) << 5) | // spare coded as zero
	((uenetworkcapability->csfb  & 0x1) << 4) |
	((uenetworkcapability->lpp   & 0x1) << 3) |
	((uenetworkcapability->lcs   & 0x1) << 2) |
	((uenetworkcapability->srvcc & 0x1) << 1) |
	(uenetworkcapability->nf     & 0x1);
    encoded++;

    *lenPtr = encoded - 1 - ((iei > 0) ? 1 : 0);
    return encoded;
}

void dump_ue_network_capability_xml(UeNetworkCapability *uenetworkcapability, uint8_t iei)
{
    printf("<Ue Network Capability>\n");
    if (iei > 0)
        /* Don't display IEI if = 0 */
        printf("    <IEI>0x%X</IEI>\n", iei);
    printf("    <EEA>%02x</EEA>\n", uenetworkcapability->eea);
    printf("    <EIA>%02x</EIA>\n", uenetworkcapability->eia);
    printf("    <UEA>%02x</UEA>\n", uenetworkcapability->uea);
    printf("    <UCS2>%u</UCS2>\n", uenetworkcapability->ucs2);
    printf("    <UIA>%u</UIA>\n", uenetworkcapability->uia);
    printf("    <SPARE>%u</SPARE>\n", uenetworkcapability->spare);
    printf("    <CSFB>%u</CSFB>\n", uenetworkcapability->csfb);
    printf("    <LPP>%u</LPP>\n", uenetworkcapability->lpp);
    printf("    <LCS>%u</LCS>\n", uenetworkcapability->lcs);
    printf("    <SR VCC>%u</SR VCC>\n", uenetworkcapability->srvcc);
    printf("    <NF>%u<NF/>\n", uenetworkcapability->nf);
    printf("</Ue Network Capability>\n");
}

