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
/*! \file rlc_um_structs.h
* \brief This file defines structures used inside the RLC UM.
* \author GAUTHIER Lionel
* \date 2010-2011
* \version
* \company Eurecom
* \email: lionel.gauthier@eurecom.fr
* \note
* \bug
* \warning
*/
#    ifndef __RLC_UM_STRUCTS_H__
#        define __RLC_UM_STRUCTS_H__

#        include "platform_types.h"
#        include "list.h"
#        include "rlc_am_constants.h"
#        include "mac_primitives.h"
#        include "rlc_primitives.h"
//#ifdef USER_MODE
#        include "mac_rlc_primitives.h"
//#endif //USER_MODE
//-----------------------
typedef struct rlc_um_tx_sdu_management {
  u8_t             *first_byte;
  s32_t             sdu_creation_time;
  u16_t             sdu_remaining_size;
  u16_t             sdu_test_remaining_size;
  u16_t             sdu_segmented_size;
  u16_t             sdu_size;
}rlc_um_tx_sdu_management_t;
//-----------------------
struct rlc_um_tx_data_pdu_management {
  union {
    struct mac_tb_req tb_req;
    struct mac_tx_tb_management tb_mngt;
#        ifdef BYPASS_L1
    struct mac_tb_ind dummy2;
    struct mac_rx_tb_management dummy3;
    struct rlc_indication dummy4;
#        endif
  } dummy;
};
//-----------------------
typedef struct rlc_um_pdu_sn_5 {
/*  u8_t fi:2;
  u8_t e:1;
  u8_t sn:5;*/
  u8_t     b1;
  u8_t     data[3];
} __attribute__((__packed__)) rlc_um_pdu_sn_5_t ;
//-----------------------
typedef struct rlc_um_pdu_sn_10 {
  u8_t  b1;
  u8_t  b2;
  u8_t  data[2];
}__attribute__((__packed__)) rlc_um_pdu_sn_10_t ;

typedef struct rlc_um_e_li {
  u8_t  b1;
  u8_t  b2;
  u8_t  b3;
}rlc_um_e_li_t;
//-----------------------
struct rlc_um_data_req_alloc {  // alloc enought bytes for sdu mngt also
  union {
    struct rlc_um_data_req dummy1;
    struct rlc_um_tx_sdu_management dummy2;
  } dummy;
};

typedef struct rlc_um_pdu_info {
    u16_t  free_bits:3;
    u16_t  fi:2;
    u16_t  e:1;
    u16_t  sn:10;
    u16_t  num_li;
    s16_t  li_list[RLC_UM_SEGMENT_NB_MAX_LI_PER_PDU];
    s16_t  hidden_size;
    u8_t*  payload;
    s16_t  payload_size;
    s16_t  header_size;
} rlc_um_pdu_info_t ;
#    endif
