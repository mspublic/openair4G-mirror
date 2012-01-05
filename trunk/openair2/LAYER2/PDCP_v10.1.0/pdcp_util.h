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

/*! \file pdcp_util.h
* \brief PDCP Util Methods
* \author Baris Demiray
* \date 2011
*/

#ifndef PDCP_UTIL_H
#define PDCP_UTIL_H

#include "UTIL/LOG/log.h"

/*
 * Prints incoming byte stream in hexadecimal and readable form
 *
 * @param component Utilised as with macros defined in UTIL/LOG/log.h
 * @param data unsigned char* pointer for data buffer
 * @param size Number of octets in data buffer
 * @return none
 */
void util_print_hex_octets(comp_name_t component, unsigned char* data, unsigned long size)
{
  if (data == NULL) {
    LOG_W(component, "Incoming buffer is NULL! Ignoring...\n");
    return;
  }

  unsigned long octet_index = 0;
  unsigned int group_index = 0;

  // XXX Utilisation of LOG_D here is temporary, LOG_T will be used afterwards so 
  // don't be scared of garbage output caused by PDCP, it'll be fixed soon :)
  LOG_D(component, "     |  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f |\n");
  LOG_D(component, "-----+-------------------------------------------------|\n");
  LOG_D(component, " 000 |");
  for (octet_index = 0; octet_index < size; ++octet_index) {
    /*
     * Print every single octet in hexadecimal form
     */
    LOG_D(component, " %02x", data[octet_index]);
    /*
     * Align newline and pipes according to the octets in groups of 2
     */
    if (octet_index != 0 && (octet_index+1) % 16 == 0)
      LOG_D(component, " |\n %03d |", octet_index);
  }

  /*
   * Append enough spaces and put final pipe
   */
  unsigned char index;
  for (index = octet_index; index < 16; ++index)
    LOG_D(component, "   ");
  LOG_D(component, " |\n");
}

#endif // PDCP_UTIL_H

