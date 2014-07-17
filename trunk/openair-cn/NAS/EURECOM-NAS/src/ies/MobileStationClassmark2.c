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
#include "MobileStationClassmark2.h"

int decode_mobile_station_classmark_2(MobileStationClassmark2 *mobilestationclassmark2, uint8_t iei, uint8_t *buffer, uint32_t len)
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
    mobilestationclassmark2->revisionlevel = (*(buffer + decoded) >> 5) & 0x3;
    mobilestationclassmark2->esind = (*(buffer + decoded) >> 4) & 0x1;
    mobilestationclassmark2->a51 = (*(buffer + decoded) >> 3) & 0x1;
    mobilestationclassmark2->rfpowercapability = *(buffer + decoded) & 0x7;
    decoded++;
    mobilestationclassmark2->pscapability = (*(buffer + decoded) >> 6) & 0x1;
    mobilestationclassmark2->ssscreenindicator = (*(buffer + decoded) >> 4) & 0x3;
    mobilestationclassmark2->smcapability = (*(buffer + decoded) >> 3) & 0x1;
    mobilestationclassmark2->vbs = (*(buffer + decoded) >> 2) & 0x1;
    mobilestationclassmark2->vgcs = (*(buffer + decoded) >> 1) & 0x1;
    mobilestationclassmark2->fc = *(buffer + decoded) & 0x1;
    decoded++;
    mobilestationclassmark2->cm3 = (*(buffer + decoded) >> 7) & 0x1;
    mobilestationclassmark2->lcsvacap = (*(buffer + decoded) >> 5) & 0x1;
    mobilestationclassmark2->ucs2 = (*(buffer + decoded) >> 4) & 0x1;
    mobilestationclassmark2->solsa = (*(buffer + decoded) >> 3) & 0x1;
    mobilestationclassmark2->cmsp = (*(buffer + decoded) >> 2) & 0x1;
    mobilestationclassmark2->a53 = (*(buffer + decoded) >> 1) & 0x1;
    mobilestationclassmark2->a52 = *(buffer + decoded) & 0x1;
    decoded++;
#if defined (NAS_DEBUG)
    dump_mobile_station_classmark_2_xml(mobilestationclassmark2, iei);
#endif
    return decoded;
}
int encode_mobile_station_classmark_2(MobileStationClassmark2 *mobilestationclassmark2, uint8_t iei, uint8_t *buffer, uint32_t len)
{
    uint8_t *lenPtr;
    uint32_t encoded = 0;
    /* Checking IEI and pointer */
    CHECK_PDU_POINTER_AND_LENGTH_ENCODER(buffer, MOBILE_STATION_CLASSMARK_2_MINIMUM_LENGTH, len);
#if defined (NAS_DEBUG)
    dump_mobile_station_classmark_2_xml(mobilestationclassmark2, iei);
#endif
    if (iei > 0)
    {
        *buffer = iei;
        encoded++;
    }
    lenPtr  = (buffer + encoded);
    encoded ++;
    *(buffer + encoded) = 0x00 |
    ((mobilestationclassmark2->revisionlevel & 0x3) << 5) |
    ((mobilestationclassmark2->esind & 0x1) << 4) |
    ((mobilestationclassmark2->a51 & 0x1) << 3) |
    (mobilestationclassmark2->rfpowercapability & 0x7);
    encoded++;
    *(buffer + encoded) = 0x00 |
    ((mobilestationclassmark2->pscapability & 0x1) << 6) |
    ((mobilestationclassmark2->ssscreenindicator & 0x3) << 4) |
    ((mobilestationclassmark2->smcapability & 0x1) << 3) |
    ((mobilestationclassmark2->vbs & 0x1) << 2) |
    ((mobilestationclassmark2->vgcs & 0x1) << 1) |
    (mobilestationclassmark2->fc & 0x1);
    encoded++;
    *(buffer + encoded) = 0x00 | ((mobilestationclassmark2->cm3 & 0x1) << 7) |
    ((mobilestationclassmark2->lcsvacap & 0x1) << 5) |
    ((mobilestationclassmark2->ucs2 & 0x1) << 4) |
    ((mobilestationclassmark2->solsa & 0x1) << 3) |
    ((mobilestationclassmark2->cmsp & 0x1) << 2) |
    ((mobilestationclassmark2->a53 & 0x1) << 1) |
    (mobilestationclassmark2->a52 & 0x1);
    encoded++;
    *lenPtr = encoded - 1 - ((iei > 0) ? 1 : 0);
    return encoded;
}

void dump_mobile_station_classmark_2_xml(MobileStationClassmark2 *mobilestationclassmark2, uint8_t iei)
{
    printf("<Mobile Station Classmark 2>\n");
    if (iei > 0)
        /* Don't display IEI if = 0 */
        printf("    <IEI>0x%X</IEI>\n", iei);
    printf("    <Revision level>%u</Revision level>\n", mobilestationclassmark2->revisionlevel);
    printf("    <ES IND>%u</ES IND>\n", mobilestationclassmark2->esind);
    printf("    <A51>%u</A51>\n", mobilestationclassmark2->a51);
    printf("    <RF power capability>%u</RF power capability>\n", mobilestationclassmark2->rfpowercapability);
    printf("    <PS capability>%u</PS capability>\n", mobilestationclassmark2->pscapability);
    printf("    <SS Screen indicator>%u</SS Screen indicator>\n", mobilestationclassmark2->ssscreenindicator);
    printf("    <SM capability>%u</SM capability>\n", mobilestationclassmark2->smcapability);
    printf("    <VBS>%u</VBS>\n", mobilestationclassmark2->vbs);
    printf("    <VGCS>%u</VGCS>\n", mobilestationclassmark2->vgcs);
    printf("    <FC>%u</FC>\n", mobilestationclassmark2->fc);
    printf("    <CM3>%u</CM3>\n", mobilestationclassmark2->cm3);
    printf("    <LCSVA CAP>%u</LCSVA CAP>\n", mobilestationclassmark2->lcsvacap);
    printf("    <UCS2>%u</UCS2>\n", mobilestationclassmark2->ucs2);
    printf("    <SoLSA>%u</SoLSA>\n", mobilestationclassmark2->solsa);
    printf("    <CMSP>%u</CMSP>\n", mobilestationclassmark2->cmsp);
    printf("    <A53>%u</A53>\n", mobilestationclassmark2->a53);
    printf("    <A52>%u</A52>\n", mobilestationclassmark2->a52);
    printf("</Mobile Station Classmark 2>\n");
}

