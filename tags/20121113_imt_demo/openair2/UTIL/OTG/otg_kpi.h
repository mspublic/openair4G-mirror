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

/*! \file otg_kpi.h functions to compute OTG KPIs
* \brief desribe function for KPIs computation 
* \author A. Hafsaoui
* \date 2012
* \version 0.1
* \company Eurecom
* \email: openair_tech@eurecom.fr
* \note
* \warning

*/

#ifndef __OTG_KPI_H__
#	define __OTG_KPI_H__


#include <stdio.h>
#include <stdlib.h>
#include "otg.h"
#include "otg_externs.h" // not needed, you should compute kpi from the pkt header


unsigned int start_log_latency=0;
unsigned int start_log_GP=0;

/*! \fn void tx_throughput( int src, int dst)
* \brief compute the transmitter throughput in bytes per seconds
* \param[in] Source, destination
* \param[out]
* \note 
* @ingroup  _otg
*/
void tx_throughput( int src, int dst);

/*! \fn void rx_goodput( int src, int dst)
* \brief compute the receiver goodput in bytes per seconds
* \param[in] Source, destination
* \param[out] 
* \note 
* @ingroup  _otg
*/
void rx_goodput( int src, int dst);


/*void rx_loss_rate_pkts(int src, int dst)
* \brief compute the loss rate in bytes at the server bytes
* \param[in] Source, destination
* \param[out] 
* \note 
* @ingroup  _otg
*/
void rx_loss_rate_pkts(int src, int dst);

/*void rx_loss_rate_bytes(int src, int dst)
* \brief compute the loss rate in pkts at the server bytes
* \param[in] Source, destination
* \param[out] 
* \note 
* @ingroup  _otg
*/
void rx_loss_rate_bytes(int src, int dst);

/*void kpi_gen(void)
* \brief compute KPIs after the end of the simulation 
* \param[in] 
* \param[out] 
* \note 
* @ingroup  _otg
*/
void kpi_gen(void);

void add_log_metric(int src, int dst, int ctime, double metric, unsigned int labelc);

void  add_log_label(unsigned int label, unsigned int * start_log_metric);

#endif
