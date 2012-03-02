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
/*! \file rlc_am_receiver.h
* \brief This file defines the prototypes of the functions dealing with the first stage of the receiving process.
* \author GAUTHIER Lionel
* \date 2010-2011
* \version
* \company Eurecom
* \email: lionel.gauthier@eurecom.fr
* \note
* \bug
* \warning
*/
#    ifndef __RLC_AM_RECEIVER_H__
#        define __RLC_AM_RECEIVER_H__
#        ifdef RLC_AM_RECEIVER_C
#            define private_rlc_am_receiver(x)    x
#            define protected_rlc_am_receiver(x)  x
#            define public_rlc_am_receiver(x)     x
#        else
#            ifdef RLC_AM_MODULE
#                define private_rlc_am_receiver(x)
#                define protected_rlc_am_receiver(x)  extern x
#                define public_rlc_am_receiver(x)     extern x
#            else
#                define private_rlc_am_receiver(x)
#                define protected_rlc_am_receiver(x)
#                define public_rlc_am_receiver(x)     extern x
#            endif
#        endif
protected_rlc_am_receiver( signed int rlc_am_get_data_pdu_infos(rlc_am_pdu_sn_10_t* headerP, s16_t sizeP, rlc_am_pdu_info_t* pdu_infoP));
protected_rlc_am_receiver( void rlc_am_display_data_pdu_infos(rlc_am_entity_t *rlcP, rlc_am_pdu_info_t* pdu_infoP);)
protected_rlc_am_receiver( void rlc_am_rx_update_vr_ms(rlc_am_entity_t *rlcP,mem_block_t* tbP);)
protected_rlc_am_receiver( void rlc_am_rx_update_vr_r (rlc_am_entity_t *rlcP,mem_block_t* tbP);)
protected_rlc_am_receiver( void rlc_am_receive_routing (rlc_am_entity_t *rlcP, struct mac_data_ind data_indP));
private_rlc_am_receiver( void rlc_am_receive_process_data_pdu (rlc_am_entity_t *rlcP, mem_block_t* tbP, u8_t* first_byteP, u16_t tb_size_in_bytesP));
#    endif
