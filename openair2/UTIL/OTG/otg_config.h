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

/*! \file otg_config.h main used structures
* \brief otg structure 
* \author A. Hafsaoui
* \date 2011
* \version 0.1
* \company Eurecom
* \email: openair_tech@eurecom.fr
* \note
* \warning

*/
#include "traffic_config.h"

#define  STANDALONE 1

#define MAXSIZE 1024 
#define MAXIDT 100000 

// header definition param - unit= bytes
#define HDR_IP_v4_MIN 20
#define HDR_IP_v4_MAX 60
#define HDR_IP_v6 60

#define HDR_TCP 20
#define HDR_UDP 8

#define HDR_OTG_SIZE 15
#define HDR_OTG_FLAG_SIZE 3
#define HDR_OTG_TIME_SIZE 4
#define HDR_OTG_SEQ_SIZE 5
#define HDR_OTG_CRC_SIZE 8
#define OTG_CTRL_FLAG "OTG"
#define END_OTG_HEADER "fff"


#define CRC_FLAG CRC_16

#define	ALPHABET_NUM_LETTER "abcdefghijklmnopqrstuvwyzABCDEFGHIGKLMNOPQRSTUVWXYZ0123456789"
#define	ALPHABET_NUM "0123456789"

