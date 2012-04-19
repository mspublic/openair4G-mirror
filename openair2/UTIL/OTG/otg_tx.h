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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#include "otg_defs.h"
#include "otg_config.h"
#include "otg_rx.h"

//#ifndef STANDALONE
//#include "../UTIL/LOG/log.h"
//#else
//#include "../LOG/log.h"
//#endif





void init_all_otg();

/*! \fn void init_config_otg();
* \brief set initial values (test)
* \param[in] 
* \param[out] 
* \note 
* @ingroup  _otg
*/

void init_config_otg();

/*! \fn double time_dist(int src, int dst, int state);
* \brief compute idt
* \param[in] node_src, node_dst, state
* \param[out] idt
* \note 
* @ingroup  _otg
*/

double time_dist(int src, int dst, int state);

/*! \fn size_dist(int src, int dst, int state);
* \brief compute size 
* \param[in] node_src, node_dst, state
* \param[out] size
* \note 
* @ingroup  _otg
*/

int size_dist(int src, int dst, int state);

/*! \fn char *random_string(int size);
* \brief return a random string[size]
* \param[in] size 
* \param[out] string
* \note 
* @ingroup  _otg
*/

char *random_string(int size, ALPHABET data_type);

/*! \fn char *packet_gen(int src, int dst, int state);
* \brief return the generated packet
* \param[in] src + dst + state
* \param[out] string : packet
* \note 
* @ingroup  _otg
*/

char *packet_gen(int src, int dst, int state);


/*! \fn void free_addr_otg();
* \brief free src and dst address
* \param[in] 
* \param[out]
* \note 
* @ingroup  _otg
*/

void free_addr_otg();

/*! \fn char *header_gen(int ip_v, int trans_proto);
* \brief generate IP (v4/v6) + transport header(TCP/UDP) 
* \param[in] int : ip version + transp proto  
* \param[out] char * packet
* \note 
* @ingroup  _otg
*/

char *header_gen(int ip_v, int trans_proto);

/*! \fn char *payload_pkts(int payload_size);
* \brief generate payload
* \param[in] int : payload size  
* \param[out] char * payload
* \note 
* @ingroup  _otg
*/

char *payload_pkts(int payload_size);

/*! \fn double get_emu_time (void);
* \brief get emulation time
* \param[in]   
* \param[out] 
* \note 
* @ingroup  _otg
*/

double get_emu_time (void);

/*! \fn unsigned int crc_gen(char *packet,  CRC crc);
* \brief generate a CRC from a packet 
* \param[in]  chart *packet + CRC 
* \param[out] crc
* \note 
* @ingroup  _otg
*/
//unsigned int crc_gen(char *packet,  CRC crc);


/*! \fn char *otg_header_gen( char* flag, int time, int seq_num);
* \brief generate a OTG header 
* \param[in]  chart *flag , int simulation time, int packet sequence number  
* \param[out] otg header
* \note 
* @ingroup  _otg
*/
char *otg_header_gen( char* flag, int time, int seq_num);

