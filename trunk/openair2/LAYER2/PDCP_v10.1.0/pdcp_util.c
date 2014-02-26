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

/*! \file pdcp_util.c
* \brief PDCP Util Methods
* \author Baris Demiray
* \date 2012
*/

#include <assert.h>

#include "UTIL/LOG/log.h"
#include "pdcp_util.h"

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

  LOG_T(component, "     |  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f |\n");
  LOG_T(component, "-----+-------------------------------------------------|\n");
  LOG_T(component, " 000 |");
  for (octet_index = 0; octet_index < size; ++octet_index) {
    /*
     * Print every single octet in hexadecimal form
     */
    LOG_T(component, "%02x.", data[octet_index]);
    /*
     * Align newline and pipes according to the octets in groups of 2
     */
    if (octet_index != 0 && (octet_index + 1) % 16 == 0) {
      LOG_T(component, " |\n");
      LOG_T(component, " %03d |", octet_index);
      }
  }

  /*
   * Append enough spaces and put final pipe
   */
  unsigned char index;
  for (index = octet_index; index < 16; ++index)
    LOG_T(component, "   ");
  LOG_T(component, " \n");
}

void util_flush_hex_octets(comp_name_t component, unsigned char* data, unsigned long size)
{
  if (data == NULL) {
    LOG_W(component, "Incoming buffer is NULL! Ignoring...\n");
    return;
  }
  printf("[PDCP]");

  unsigned long octet_index = 0;
  for (octet_index = 0; octet_index < size; ++octet_index) {
      //LOG_T(component, "%02x.", data[octet_index]);
      printf("%02x.", data[octet_index]);
  }
  //LOG_T(component, " \n");
  printf(" \n");
}

/*
 * Prints binary representation of given octet prepending 
 * passed log message
 *
 * @param Octet as an unsigned character
 * @return None
 */
void util_print_binary_representation(unsigned char* message, u8_t octet)
{
  unsigned char index = 0;
  unsigned char mask = 0x80;

  LOG_T(PDCP, "%s", message);

  for (index = 0; index < 8; ++index) {
    if (octet & mask) {
      LOG_T(PDCP, "1");
    } else {
      LOG_T(PDCP, "0");
    }

    mask /= 2;
  }
  LOG_T(PDCP, "\n");
}

/*
 * Sets the bit of given octet at `index'
 *
 * @param octet 8-bit data
 * @param index Index of bit to be set
 * @return TRUE on success, FALSE otherwise
 */
boolean_t util_mark_nth_bit_of_octet(u8_t* octet, u8_t index)
{
  u8_t mask = 0x80;

  assert(octet != NULL);

  /*
   * Prepare mask
   */
  mask >>= index;

  /*
   * Set relevant bit
   */
  *octet |= mask;

  return TRUE;
}

