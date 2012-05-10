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

/*! \file otg.h
* \brief Data structure and functions for OTG 
* \author N. Nikaein and A. Hafsaoui
* \date 2011
* \version 1.0
* \company Eurecom
* \email: openair_tech@eurecom.fr
* \note
* \warning
*/


#ifndef __OTG_H__
#	define __OTG_H__


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "otg_defs.h"


#if STANDALONE==1
	#define LOG_G(c, x...) printf(x)
	#define LOG_A(c, x...) printf(x)
	#define LOG_C(c, x...) printf(x)
	#define LOG_E(c, x...) printf(x)
	#define LOG_W(c, x...) printf(x)
	#define LOG_N(c, x...) printf(x)
	#define LOG_I(c, x...) printf(x)
	#define LOG_D(c, x...) printf(x)
	#define LOG_F(c, x...) printf(x)  
	#define LOG_T(c, x...) printf(x)
typedef enum {MIN_NUM_COMPS=0, PHY, OMG, OCM, OTG, MAX_NUM_COMPS} comp_t;
#include "otg_vars.h"
#else
	#include "../UTIL/LOG/log.h"
	#include "otg_externs.h"
#endif 




/*! \fn void init_all_otg(void)
* \brief set initial values de 0 
* \param[in] void
* \param[out] void
* \note 
* @ingroup  _otg
*/
void init_all_otg(void);

/*! \fn void set_ctime(int ctime)
* \brief set the simulation time to the ctime 
* \param[in] simulation time , void
* \param[out] void
* \note 
* @ingroup  _otg
*/
void set_ctime(int ctime);


/*! \fn void get_ctime(void)
* \brief get the simulation time 
* \param[in] int
* \param[out] void
* \note 
* @ingroup  _otg
*/
int get_ctime(void);



/*! \fn void free_addr_otg(void);
* \brief free the sting of IP address for the socket mode
* \param[in] 
* \param[out]
* \note 
* @ingroup  _otg
*/
void free_addr_otg(void);



#endif
