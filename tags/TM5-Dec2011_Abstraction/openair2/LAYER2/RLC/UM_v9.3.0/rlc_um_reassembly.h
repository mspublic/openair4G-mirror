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
/*! \file rlc_um_reassembly.h
* \brief This file defines the prototypes of the functions dealing with the reassembly of segments.
* \author GAUTHIER Lionel
* \date 2010-2011
* \version
* \company Eurecom
* \email: lionel.gauthier@eurecom.fr
* \note
* \bug
* \warning
*/
#    ifndef __RLC_UM_REASSEMBLY_PROTO_EXTERN_H__
#        define __RLC_UM_REASSEMBLY_PROTO_EXTERN_H__
//-----------------------------------------------------------------------------
#        ifdef RLC_UM_REASSEMBLY_C
#            define private_rlc_um_reassembly(x)    x
#            define protected_rlc_um_reassembly(x)  x
#            define public_rlc_um_reassembly(x)     x
#        else
#            ifdef RLC_UM_MODULE
#                define private_rlc_um_reassembly(x)
#                define protected_rlc_um_reassembly(x)  extern x
#                define public_rlc_um_reassembly(x)     extern x
#            else
#                define private_rlc_um_reassembly(x)
#                define protected_rlc_um_reassembly(x)
#                define public_rlc_um_reassembly(x)     extern x
#            endif
#        endif
#        include "rlc_um_entity.h"
//-----------------------------------------------------------------------------
protected_rlc_um_reassembly(void     rlc_um_send_sdu_minus_1_byte (rlc_um_entity_t *rlcP));
protected_rlc_um_reassembly(void     rlc_um_clear_rx_sdu (rlc_um_entity_t *rlcP));
protected_rlc_um_reassembly(void     rlc_um_reassembly (u8_t * srcP, s32_t lengthP, rlc_um_entity_t *rlcP));
protected_rlc_um_reassembly(void     rlc_um_send_sdu (rlc_um_entity_t *rlcP));
#    endif