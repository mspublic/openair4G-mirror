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
/*! \file rlc_am_segment_holes.h
* \brief This file defines the prototypes of the functions dealing with the reading/writting of informations from/in RLC AM control PDUs, and the processing of received control PDUs.
* \author GAUTHIER Lionel
* \date 2010-2011
* \version
* \company Eurecom
* \email: lionel.gauthier@eurecom.fr
* \note
* \bug
* \warning
*/
#ifndef __RLC_AM_STATUS_REPORT_H__
#    define __RLC_AM_STATUS_REPORT_H__

#    include "UTIL/MEM/mem_block.h"
//-----------------------------------------------------------------------------
#        ifdef RLC_AM_STATUS_REPORT_C
#            define private_rlc_am_status_report(x)    x
#            define protected_rlc_am_status_report(x)  x
#            define public_rlc_am_status_report(x)     x
#        else
#            ifdef RLC_AM_MODULE
#                define private_rlc_am_status_report(x)
#                define protected_rlc_am_status_report(x)  extern x
#                define public_rlc_am_status_report(x)     extern x
#            else
#                define private_rlc_am_status_report(x)
#                define protected_rlc_am_status_report(x)
#                define public_rlc_am_status_report(x)     extern x
#            endif
#        endif
//-----------------------------------------------------------------------------
#include "platform_types.h"
#include "platform_constants.h"
#include "PHY/defs.h"

//-----------------------------------------------------------------------------
protected_rlc_am_status_report( u16_t      rlc_am_read_bit_field             (u8_t** dataP, unsigned int* bit_posP, signed int
bits_to_readP);)
protected_rlc_am_status_report(void        rlc_am_write8_bit_field(u8_t** dataP, unsigned int* bit_posP, signed int
bits_to_writeP, u8_t valueP);)
protected_rlc_am_status_report(void        rlc_am_write16_bit_field(u8_t** dataP, unsigned int* bit_posP, signed int
bits_to_writeP, u16_t valueP);)

protected_rlc_am_status_report( signed int rlc_am_get_control_pdu_infos      (rlc_am_pdu_sn_10_t* headerP, s16_t total_sizeP,
rlc_am_control_pdu_info_t* pdu_infoP);)
protected_rlc_am_status_report( void       rlc_am_display_control_pdu_infos(rlc_am_control_pdu_info_t* pdu_infoP);)
protected_rlc_am_status_report( void       rlc_am_receive_process_control_pdu(rlc_am_entity_t* rlcP, mem_block_t*  tbP, u8_t*
first_byte, u16_t tb_size_in_bytes);)
protected_rlc_am_status_report(int         rlc_am_write_status_pdu(rlc_am_pdu_sn_10_t* rlc_am_pdu_sn_10P,
rlc_am_control_pdu_info_t* pdu_infoP);)
protected_rlc_am_status_report(void        rlc_am_send_status_pdu(rlc_am_entity_t* rlcP);)

#endif
