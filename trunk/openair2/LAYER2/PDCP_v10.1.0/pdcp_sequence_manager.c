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

/*! \file pdcp_sequence_manager.c
* \brief PDCP Sequence Numbering Methods
* \author Baris Demiray
* \date 201
*/

#include "pdcp.h"
#include <math.h>

/*
 * Initializes sequence numbering state
 * @param pdcp_entity The PDCP entity to be initialized
 * @return none
 */
void pdcp_init_seq_numbers(pdcp_t* pdcp_entity)
{
  if (pdcp_entity == NULL)
    return;

  /* Sequence number state variables */
  // TX and RX window
  pdcp_entity->next_pdcp_tx_sn = 0;
  pdcp_entity->next_pdcp_rx_sn = 0;
  // TX and RX Hyper Frame Numbers
  pdcp_entity->tx_hfn = 0;
  pdcp_entity->rx_hfn = 0;
  // SN of the last PDCP SDU delivered to upper layers
  pdcp_entity->last_submitted_pdcp_rx_sn = 0;
}

u16 pdcp_check_seq_num_validity(pdcp_t* pdcp_entity, u8 seq_num_size)
{
  if (pdcp_entity == NULL)
    return -1;

  // Check if the size of SN is valid (see 3GPP TS 36.323 v10.1.0 item 6.3.2)
  if (seq_num_size != 5 && seq_num_size != 7 && seq_num_size != 12)
    // How to log this here?
    return -2;
}

u16 pdcp_calculate_max_seq_num_for_given_size(u8 seq_num_size)
{
  return (u16) pow(2.0, seq_num_size) - 1;
}

u16 pdcp_get_next_tx_seq_number(pdcp_t* pdcp_entity, u8 seq_num_size)
{
  if (pdcp_check_seq_num_validity(pdcp_entity, seq_num_size) < 0)
    return -1;

  // Sequence number should be incremented after it is assigned for a PDU
  u16 pdcp_seq_num = pdcp_entity->next_pdcp_tx_sn;

  // Update sequence numbering state (see 5.1 PDCP Data Transfer Procedures)
  if (pdcp_entity->next_pdcp_tx_sn == pdcp_calculate_max_seq_num_for_given_size(seq_num_size)) {
    pdcp_entity->next_pdcp_tx_sn = 0;
    pdcp_entity->tx_hfn++;
  } else {
    pdcp_entity->next_pdcp_tx_sn++;
  }

  return pdcp_seq_num;
}

u16 pdcp_get_next_rx_seq_number(pdcp_t* pdcp_entity, u8 seq_num_size)
{
  if (pdcp_check_seq_num_validity(pdcp_entity, seq_num_size) < 0)
    return -1;

  // Sequence number should be incremented after it is assigned for a PDU
  u16 pdcp_seq_num = pdcp_entity->next_pdcp_rx_sn;

  // Update sequence numbering state (see 5.1 PDCP Data Transfer Procedures)
  if (pdcp_entity->next_pdcp_rx_sn == pdcp_calculate_max_seq_num_for_given_size(seq_num_size)) {
    pdcp_entity->next_pdcp_rx_sn = 0;
    pdcp_entity->rx_hfn++;
  } else {
    pdcp_entity->next_pdcp_rx_sn++;
  }

  return pdcp_seq_num;
}














