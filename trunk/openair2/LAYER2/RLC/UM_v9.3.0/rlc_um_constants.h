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
/*! \file rlc_um_constants.h
* \brief This file defines constant values used inside RLC UM.
* \author GAUTHIER Lionel
* \date 2010-2011
* \version
* \company Eurecom
* \email: lionel.gauthier@eurecom.fr
* \note
* \bug
* \warning
*/
#    ifndef __RLC_UM_CONSTANTS_H__
#        define __RLC_UM_CONSTANTS_H__

#        define RLC_UM_SN_10_BITS_MODULO                    1024
#        define RLC_UM_SN_10_BITS_MASK                      0x03FF
#        define RLC_UM_WINDOW_SIZE_SN_10_BITS               512
#        define RLC_UM_SN_5_BITS_MODULO                     32
#        define RLC_UM_SN_5_BITS_MASK                       0x1F
#        define RLC_UM_WINDOW_SIZE_SN_5_BITS                16
         // This constant is used by the receiving UM RLC entity to define SNs of those
         // UMD PDUs that can be received without causing an advancement of the
         // receiving window. UM_Window_Size = 16 when a 5 bit SN is configured,
         // UM_Window_Size = 512 when a 10 bit SN is configured and UM_Window_Size = 0
         // when the receiving UM RLC entity is configured for MCCH or MTCH.
         // li field (values shifted 1 bit left)
#        define RLC_UM_SEGMENT_NB_MAX_LI_PER_PDU            24
//----------------------------------------------------------
// Events defined for state model of the acknowledged mode entity
#        define RLC_UM_RECEIVE_CRLC_CONFIG_REQ_ENTER_NULL_STATE_EVENT                 0x00
#        define RLC_UM_RECEIVE_CRLC_CONFIG_REQ_ENTER_DATA_TRANSFER_READY_STATE_EVENT  0x01
#        define RLC_UM_RECEIVE_CRLC_SUSPEND_REQ_EVENT                                 0x10
#        define RLC_UM_TRANSMIT_CRLC_SUSPEND_CNF_EVENT                                0x11
#        define RLC_UM_RECEIVE_CRLC_RESUME_REQ_EVENT                                  0x12
#    endif
