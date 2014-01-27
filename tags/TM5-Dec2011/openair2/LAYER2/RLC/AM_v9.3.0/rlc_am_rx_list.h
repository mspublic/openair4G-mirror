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
/*! \file rlc_am_rx_list.h
* \brief This file defines the prototypes of the functions dealing with a RX list data structure supporting re-segmentation.
* \author GAUTHIER Lionel
* \date 2010-2011
* \version
* \company Eurecom
* \email: lionel.gauthier@eurecom.fr
* \note
* \bug
* \warning
*/
#ifndef __RLC_AM_RX_LIST_H__
#    define __RLC_AM_RX_LIST_H__

#    include "UTIL/MEM/mem_block.h"
//-----------------------------------------------------------------------------
#        ifdef RLC_AM_RX_LIST_C
#            define private_rlc_am_rx_list(x)    x
#            define protected_rlc_am_rx_list(x)  x
#            define public_rlc_am_rx_list(x)     x
#        else
#            ifdef RLC_AM_MODULE
#                define private_rlc_am_rx_list(x)
#                define protected_rlc_am_rx_list(x)  extern x
#                define public_rlc_am_rx_list(x)     extern x
#            else
#                define private_rlc_am_rx_list(x)
#                define protected_rlc_am_rx_list(x)
#                define public_rlc_am_rx_list(x)     extern x
#            endif
#        endif
//-----------------------------------------------------------------------------
#include "platform_types.h"
#include "platform_constants.h"
#include "PHY/defs.h"

//-----------------------------------------------------------------------------
protected_rlc_am_rx_list( signed int rlc_am_rx_list_insert_pdu(rlc_am_entity_t* rlcP, mem_block_t* tbP);)
protected_rlc_am_rx_list( void rlc_am_rx_check_all_byte_segments(rlc_am_entity_t* rlcP, mem_block_t* tbP);)
protected_rlc_am_rx_list( void rlc_am_rx_mark_all_segments_received(rlc_am_entity_t* rlcP, mem_block_t* fisrt_segment_tbP);)
protected_rlc_am_rx_list( void rlc_am_rx_list_reassemble_rlc_sdus(rlc_am_entity_t* rlcP);)

public_rlc_am_rx_list( mem_block_t* list2_insert_before_element (mem_block_t * element_to_insertP, mem_block_t * elementP, list2_t * listP);)
public_rlc_am_rx_list( mem_block_t* list2_insert_after_element (mem_block_t * element_to_insertP, mem_block_t * elementP, list2_t * listP);)
protected_rlc_am_rx_list( void rlc_am_rx_list_display (rlc_am_entity_t* rlcP, char* messageP);)
#endif