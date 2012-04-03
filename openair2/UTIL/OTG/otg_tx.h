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

/*! \file otg_tx.h
* \brief Data structure and functions for OTG 
* \author N. Nikaein and A. Hafsaoui
* \date 2011
* \version 1.0
* \company Eurecom
* \email: openair_tech@eurecom.fr
* \note
* \warning
*/

#ifndef __OTG_TX_H__
#	define __OTG_TX_H__



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>


#include "otg.h"

#include "../MATH/oml.h"



/*! \fn int time_dist(int src, int dst, int state)
* \brief compute Inter Departure Time, in ms
* \param[in] Source, destination, state
* \param[out] Inter Departure Time 
* \note 
* @ingroup  _otg
*/
int time_dist(int src, int dst, int state);

/*! \fn int size_dist(int src, int dst, int state)
* \brief compute the payload size, in bytes 
* \param[in] Source, node_dst, state
* \param[out] size of the payload, in bytes
* \note 
* @ingroup  _otg
*/
int size_dist(int src, int dst, int state);

/*! \fn char *random_string(int size, ALPHABET data_type, char *data_string)
* \brief return a random string[size]
* \param[in] size  of the string to generate, data : numeric or letters + numeric, a static string used to generate the output  random string 
* \param[out] string of a random char
* \note 
* @ingroup  _otg
*/
char *random_string(int size, ALPHABET data_type, char *data_string);

/*! \fn int packet_gen(int src, int dst, int state, int ctime)
* \brief return int= 1 if the packet is generated: OTG header + header + payload, else 0
* \param[in] source, 
* \param[out] packet_t: the generated packet: otg_header + header + payload
* \note 
* @ingroup  _otg
*/
char *packet_gen(int src, int dst, int state, int ctime);


/*! \fn char *header_gen(int  hdr_size);
* \brief generate IP (v4/v6) + transport header(TCP/UDP) 
* \param[in] int : size 
* \param[out] the payload corresponding to ip version and transport protocol
* \note 
* @ingroup  _otg
*/
char *header_gen(int hdr_size);

/*! \fn char *payload_pkts(int payload_size);
* \brief generate the payload
* \param[in] int : payload size  
* \param[out] char * payload
* \note 
* @ingroup  _otg
*/
char *payload_pkts(int payload_size);


/*! \fn void otg_header_gen(int time, int seq_num, HEADER_TYPE header_type,int payload_size);
* \brief generate OTG header 
* \param[in]  simulation time, header_size (to know the transport/ip version in the RX) and, packet sequence number and the payload_size  
* \param[out] otg header
* \note 
* @ingroup  _otg
*/
void otg_header_gen(int flow_id, int time, int seq_num, int payload_size);


/*! \fn int adjust_size(int size);
* \brief adjuste the generated packet size when size<min or size>max   
* \param[in]  size  
* \param[out] modified size in case
* \note 
* @ingroup  _otg
*/
int adjust_size(int size);


/*! \fn int header_size_genint src();
* \brief return the header size corresponding to ip version and transport protocol  
* \param[in]  the sender (src)
* \param[out] size of packet header 
* \note 
* @ingroup  _otg
*/
int header_size_gen(int src);
#endif
