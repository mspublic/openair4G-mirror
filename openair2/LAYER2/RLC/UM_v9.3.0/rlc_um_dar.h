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
/*! \file rlc_um_dar.h
* \brief This file defines the prototypes of the functions dealing with the reassembly buffer.
* \author GAUTHIER Lionel
* \date 2010-2011
* \version
* \company Eurecom
* \email: lionel.gauthier@eurecom.fr
* \note
* \bug
* \warning
*/
#    ifndef __RLC_UM_DAR_H__
#        define __RLC_UM_DAR_H__
//-----------------------------------------------------------------------------
#        include "rlc_um_entity.h"
#        include "rlc_um_structs.h"
#        include "rlc_um_constants.h"
#        include "list.h"
//-----------------------------------------------------------------------------
#        ifdef RLC_UM_DAR_C
#            define private_rlc_um_dar(x)    x
#            define protected_rlc_um_dar(x)  x
#            define public_rlc_um_dar(x)     x
#        else
#            ifdef RLC_UM_MODULE
#                define private_rlc_um_dar(x)
#                define protected_rlc_um_dar(x)  extern x
#                define public_rlc_um_dar(x)     extern x
#            else
#                define private_rlc_um_dar(x)
#                define protected_rlc_um_dar(x)
#                define public_rlc_um_dar(x)     extern x
#            endif
#        endif
private_rlc_um_dar(  signed int rlc_um_get_pdu_infos(rlc_um_pdu_sn_10_t* headerP, s16_t total_sizeP, rlc_um_pdu_info_t* pdu_infoP));
private_rlc_um_dar(  int rlc_um_read_length_indicators(unsigned char**dataP, rlc_um_e_li_t* e_liP, unsigned int* li_arrayP, unsigned int *num_liP, unsigned int *data_sizeP));
private_rlc_um_dar(  void rlc_um_try_reassembly      (rlc_um_entity_t *rlcP, signed int snP));
private_rlc_um_dar(  void rlc_um_check_timer_dar_time_out(rlc_um_entity_t *rlcP));
private_rlc_um_dar(  mem_block_t *rlc_um_remove_pdu_from_dar_buffer(rlc_um_entity_t *rlcP, u16_t snP));
private_rlc_um_dar(  inline mem_block_t* rlc_um_get_pdu_from_dar_buffer(rlc_um_entity_t *rlcP, u16_t snP));
protected_rlc_um_dar(inline signed int rlc_um_in_window(rlc_um_entity_t *rlcP, signed int lower_boundP, signed int snP, signed int higher_boundP));
protected_rlc_um_dar(void rlc_um_receive_process_dar (rlc_um_entity_t *rlcP, mem_block_t *pdu_memP,rlc_um_pdu_sn_10_t *pduP, u16_t tb_sizeP));
#    endif
