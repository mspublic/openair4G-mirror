/*******************************************************************************

Eurecom OpenAirInterface 2
Copyright(c) 1999 - 2010 Eurecom

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
/*! \file rlc_am_init.h
* \brief This file defines the prototypes of the functions initializing a RLC AM protocol instance.
* \author GAUTHIER Lionel
* \date 2010-2011
* \version
* \company Eurecom
* \email: lionel.gauthier@eurecom.fr
* \note
* \bug
* \warning
*/
#ifndef __RLC_AM_INIT_H__
#    define __RLC_AM_INIT_H__

#    include "UTIL/MEM/mem_block.h"
//-----------------------------------------------------------------------------
#        ifdef RLC_AM_INIT_C
#            define private_rlc_am_init(x)    x
#            define protected_rlc_am_init(x)  x
#            define public_rlc_am_init(x)     x
#        else
#            ifdef RLC_AM_MODULE
#                define private_rlc_am_init(x)
#                define protected_rlc_am_init(x)  extern x
#                define public_rlc_am_init(x)     extern x
#            else
#                define private_rlc_am_init(x)
#                define protected_rlc_am_init(x)
#                define public_rlc_am_init(x)     extern x
#            endif
#        endif
//-----------------------------------------------------------------------------
#include "platform_types.h"
#include "platform_constants.h"
#include "PHY/defs.h"

//-----------------------------------------------------------------------------
public_rlc_am_init( void rlc_am_init   (rlc_am_entity_t* rlcP);)
public_rlc_am_init( void rlc_am_cleanup(rlc_am_entity_t* rlcP);)
public_rlc_am_init( void rlc_am_configure(rlc_am_entity_t *rlcP,
                                          u16_t max_retx_thresholdP,
                                          u16_t poll_pduP,
                                          u16_t poll_byteP,
                                          u32_t t_poll_retransmitP,
                                          u32_t t_reorderingP,
                                          u32_t t_status_prohibitP);)
public_rlc_am_init( void rlc_am_set_debug_infos(rlc_am_entity_t *rlcP, module_id_t module_idP, rb_id_t rb_idP, rb_type_t rb_typeP);)
#endif
