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
/*! \file rlc_am_windows.h
* \brief This file defines the prototypes of the functions testing window, based on SN modulo and rx and tx protocol state variables.
* \author GAUTHIER Lionel
* \date 2010-2011
* \version
* \company Eurecom
* \email: lionel.gauthier@eurecom.fr
* \note
* \bug
* \warning
*/
#    ifndef __RLC_AM_WINDOWS_H__
#        define __RLC_AM_WINDOWS_H__
//-----------------------------------------------------------------------------
#        ifdef RLC_AM_WINDOWS_C
#            define private_rlc_am_windows(x)    x
#            define protected_rlc_am_windows(x)  x
#            define public_rlc_am_windows(x)     x
#        else
#            ifdef RLC_AM_MODULE
#                define private_rlc_am_windows(x)
#                define protected_rlc_am_windows(x)  extern x
#                define public_rlc_am_windows(x)     extern x
#            else
#                define private_rlc_am_windows(x)
#                define protected_rlc_am_windows(x)
#                define public_rlc_am_windows(x)     extern x
#            endif
#        endif

protected_rlc_am_windows(signed int rlc_am_in_tx_window(rlc_am_entity_t* rlcP, u16_t snP);)
protected_rlc_am_windows(signed int rlc_am_in_rx_window(rlc_am_entity_t* rlcP, u16_t snP);)
protected_rlc_am_windows(signed int rlc_am_sn_gte_vr_h (rlc_am_entity_t* rlcP, u16_t snP);)
protected_rlc_am_windows(signed int rlc_am_sn_gte_vr_x (rlc_am_entity_t* rlcP, u16_t snP);)
protected_rlc_am_windows(signed int rlc_am_sn_gt_vr_ms(rlc_am_entity_t* rlcP, u16_t snP);)
protected_rlc_am_windows(signed int rlc_am_tx_sn1_gt_sn2(rlc_am_entity_t* rlcP, u16_t sn1P, u16_t sn2P);)
protected_rlc_am_windows(signed int rlc_am_rx_sn1_gt_sn2(rlc_am_entity_t* rlcP, u16_t sn1P, u16_t sn2P);)

#    endif
