/*******************************************************************************
Eurecom OpenAirInterface 2
Copyright(c) 1999 - 2014 Eurecom

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
Address      : EURECOM,
               Campus SophiaTech,
               450 Route des Chappes,
               CS 50193
               06904 Biot Sophia Antipolis cedex,
               FRANCE
*******************************************************************************/
/***************************************************************************
                          rlc_am_proto_extern.h  -
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr

 ***************************************************************************/
#    ifndef __RLC_AM_PROTO_EXTERN_H__
#        define __RLC_AM_PROTO_EXTERN_H__
//-----------------------------------------------------------------------------
#        include "platform_types.h"
#        include "platform_constants.h"
#        include "list.h"
#        include "rlc_am_entity.h"
#        include "mac_primitives.h"
#        include "mem_block.h"
//-----------------------------------------------------------------------------
extern void     display_protocol_vars_rlc_am (struct rlc_am_entity *rlcP);
extern uint32_t      rlc_am_get_buffer_occupancy (struct rlc_am_entity *rlcP, uint8_t logical_channelsP);
extern void     init_rlc_am (struct rlc_am_entity *rlcP);
extern void    *rlc_am_tx (void *argP);
extern void     rlc_am_rx (void *argP, struct mac_data_ind data_indP);
extern void    *init_code_rlc_am (void *t);
extern void     send_rlc_am_control_primitive (struct rlc_am_entity *rlcP, module_id_t module_idP, mem_block_t * cprimitiveP);
extern void     rlc_am_send_mac_data_request (void *macP, uint8_t logical_channel_identityP, list_t * pduP);

extern struct mac_status_resp rlc_am_mac_status_indication (void *rlcP, uint16_t no_tbP, uint16_t tb_sizeP, struct mac_status_ind tx_statusP);
extern struct mac_status_resp rlc_am_mac_status_indication_on_first_channel (void *rlcP, uint16_t no_tbP, uint16_t tb_sizeP, struct mac_status_ind tx_statusP);
extern struct mac_status_resp rlc_am_mac_status_indication_on_second_channel (void *rlcP, uint16_t no_tbP, uint16_t tb_sizeP, struct mac_status_ind tx_statusP);
extern struct mac_data_req rlc_am_mac_data_request (void *rlcP);
extern struct mac_data_req rlc_am_mac_data_request_on_first_channel (void *rlcP);
extern struct mac_data_req rlc_am_mac_data_request_on_second_channel (void *rlcP);
extern void     rlc_am_mac_data_indication (void *rlcP, struct mac_data_ind data_indP);
extern void     rlc_am_data_req (void *rlcP, mem_block_t * sduP);
#    endif
