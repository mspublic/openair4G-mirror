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
#include "EsmCause.h"

int decode_esm_cause(EsmCause *esmcause, uint8_t iei, uint8_t *buffer, uint32_t len)
{
    int decoded = 0;
    if (iei > 0)
    {
        CHECK_IEI_DECODER(iei, *buffer);
        decoded++;
    }
    *esmcause = *(buffer + decoded);
    decoded++;
#if defined (NAS_DEBUG)
    dump_esm_cause_xml(esmcause, iei);
#endif
    return decoded;
}

int encode_esm_cause(EsmCause *esmcause, uint8_t iei, uint8_t *buffer, uint32_t len)
{
    uint32_t encoded = 0;
    /* Checking IEI and pointer */
    CHECK_PDU_POINTER_AND_LENGTH_ENCODER(buffer, ESM_CAUSE_MINIMUM_LENGTH, len);
#if defined (NAS_DEBUG)
    dump_esm_cause_xml(esmcause, iei);
#endif
    if (iei > 0)
    {
        *buffer = iei;
        encoded++;
    }
    *(buffer + encoded) = *esmcause;
    encoded++;
    return encoded;
}

void dump_esm_cause_xml(EsmCause *esmcause, uint8_t iei)
{
    printf("<Esm Cause>\n");
    if (iei > 0)
        /* Don't display IEI if = 0 */
        printf("    <IEI>0x%X</IEI>\n", iei);
    printf("    <Cause value>%u</Cause value>\n", *esmcause);
    printf("</Esm Cause>\n");
}

