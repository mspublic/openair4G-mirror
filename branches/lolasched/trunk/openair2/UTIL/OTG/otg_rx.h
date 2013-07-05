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

/*! \file otg_rx.h
* \brief Data structure and functions for OTG receiver  
* \author A. Hafsaoui
* \date 2011
* \version 1.0
* \company Eurecom
* \email: openair_tech@eurecom.fr
* \note
* \warning
*/


#ifndef __OTG_RX_H__
#	define __OTG_RX_H__

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "otg.h"





/*! \fn char *check_packet(int src, int dst, int ctime);
* \brief check if the packet is well received and do measurements: one way delay, throughput,etc. 
* \param[in] the source 
* \param[in] the destination 
* \param[in] time of the emulation
* \param[out] return NULL is the packet is well received,  else the packet to forward
* \note 
* @ingroup  _otg
*/
int otg_rx_pkt_packet(int src, int dst, int ctime, char *packet, unsigned int size);


/*! \fn void owd_const_gen(int src,int dst);
*\brief compute the one way delay introduced in LTE/LTE-A network REF PAPER: "Latency for Real-Time Machine-to-Machine Communication in LTE-Based System Architecture"
*\param[in] the source 
*\param[in] the destination
*\param[out] void
*\note 
*@ingroup  _otg
*/
void owd_const_gen(int src,int dst, int flow_id, unsigned int flag);

/*! \fn float owd_const_capillary();
*\brief compute the one way delay introduced in LTE/LTE-A network REF PAPER: "Latency for Real-Time Machine-to-Machine Communication in LTE-Based System Architecture"
*\param[out] float: capillary delay constant
*\note 
*@ingroup  _otg
*/
float owd_const_capillary();

/*! \fn float owd_const_mobile_core();
*\brief compute the one way delay introduced in LTE/LTE-A network REF PAPER: "Latency for Real-Time Machine-to-Machine Communication in LTE-Based System Architecture"
*\param[out] float: mobile core delay constant
*\note 
*@ingroup  _otg
*/
float owd_const_mobile_core();

/*! \fn float owd_const_IP_backbone();
*\brief compute the one way delay introduced in LTE/LTE-A network REF PAPER: "Latency for Real-Time Machine-to-Machine Communication in LTE-Based System Architecture"
*\param[out] float: IP backbone delay constant
*\note 
*@ingroup  _otg
*/
float owd_const_IP_backbone();

/*! \fn float owd_const_applicatione();
*\brief compute the one way delay introduced in LTE/LTE-A network REF PAPER: "Latency for Real-Time Machine-to-Machine Communication in LTE-Based System Architecture"
*\param[out] float: application delay constant
*\note 
*@ingroup  _otg
*/
float owd_const_application();


/*! \fn void rx_check_loss(int src, int dst, unsigned int flag, int seq_num, unsigned int *seq_num_rx, unsigned int *nb_loss_pkts);
*\brief check the number of loss packet/out of sequence
*\param[in] src
*\param[in] dst
*\param[in] flag: background or data
*\param[in] seq_num: packet sequence number
*\param[in] seq_num_rx:RX sequence number
*\param[in] nb_loss_pkts: number of lost packet 
*\param[out] lost_packet: (0) no lost packets, (1) lost packets
*\note 
*@ingroup  _otg
*/
int rx_check_loss(int src, int dst, unsigned int flag, int seq_num, unsigned int *seq_num_rx, unsigned int *nb_loss_pkts);

#endif
