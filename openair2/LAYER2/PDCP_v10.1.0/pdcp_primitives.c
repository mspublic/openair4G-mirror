/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2011 Eurecom

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fsr/openairinterface
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/

/*! \file pdcp_primitives.c
* \brief PDCP PDU buffer dissector code
* \author Baris Demiray
* \date 2011
* \version 0.1
*/
#include "platform_types.h"
#include "platform_constants.h"
#include "pdcp_primitives.h"

/*
 * Parses sequence number out of buffer of User Plane PDCP Data PDU with 
 * long PDCP SN (12-bit)
 *
 * @param pdu_buffer PDCP PDU buffer
 * @return 12-bit sequence number
 */
u16 pdcp_get_sequence_number_of_pdu_with_long_sn(unsigned char* pdu_buffer)
{
  u16 sequence_number = 0x00;

  if (pdu_buffer == NULL)
    return 0;

  /*
   * First octet carries the first 4 bits of SN (see 6.2.3)
   */
  sequence_number = (u8)pdu_buffer[0] & 0x0F; // Reset D/C field
  sequence_number <<= 8;
  /*
   * Second octet carries the second part (8-bit) of SN (see 6.2.3)
   */
  sequence_number |= (u8)pdu_buffer[1] & 0xFF;

  return sequence_number;
}

/*
 * Parses sequence number out of buffer of User Plane PDCP Data PDU with 
 * short PDCP SN (7-bit)
 *
 * @param pdu_buffer PDCP PDU buffer
 * @return 7-bit sequence number
 */
u8 pdcp_get_sequence_number_of_pdu_with_short_sn(unsigned char* pdu_buffer)
{
  if (pdu_buffer == NULL)
    return 0;

  /*
   * First octet carries all 7 bits of SN (see 6.2.4)
   */
  return (u8)pdu_buffer[0] & 0x7F; // Reset D/C field
}

