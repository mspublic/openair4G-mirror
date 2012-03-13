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
#include "otg_defs.h"
#include "otg_config.h"


/*! \fn int check_packet(int src, int dst, int ctime);
* \brief check if the packet is well received and do measurements: one way delay, throughput,etc. 
* \param[in] the source, the destination, time of the emulation
* \param[out] return 1 is the packet is well received, and drop it, else -1
* \note 
* @ingroup  _otg
*/
int check_packet(int src, int dst, int ctime);


#endif
