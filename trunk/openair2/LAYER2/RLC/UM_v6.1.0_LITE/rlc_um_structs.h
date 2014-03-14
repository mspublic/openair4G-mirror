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

/******************************************************************************
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr
*******************************************************************************/
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
struct rlc_um_tx_sdu_management {
  uint8_t             *first_byte;
  int32_t             sdu_creation_time;
  uint16_t             sdu_remaining_size;
  uint16_t             sdu_segmented_size;
  uint16_t             sdu_size;
  uint8_t              use_special_li;
};
//-----------------------
struct rlc_um_tx_pdu_management {
  uint8_t             *first_byte;   // point to first byte of RLC header
  uint8_t             *payload;
  uint16_t             data_size;    // size of payload (not included byte1, LIs, and padding)
  uint8_t              dummy[MAC_HEADER_MAX_SIZE];   // optimization for MAC layer : reserve bytes for MAC header
};
//-----------------------
struct rlc_um_rx_pdu_management {
  uint8_t             *first_byte;
  uint8_t              sn;
};
//-----------------------
struct rlc_um_tx_data_pdu_management {
  union {
    struct rlc_um_tx_pdu_management tx_pdu_mngmnt;
    struct mac_tb_req tb_req;
    struct mac_tx_tb_management tb_mngt;
#        ifdef BYPASS_L1
    struct rlc_um_rx_pdu_management dummy1;
    struct mac_tb_ind dummy2;
    struct mac_rx_tb_management dummy3;
    struct rlc_indication dummy4;
#        endif
  } dummy;
};
//-----------------------
struct rlc_um_rx_data_pdu_struct {
  uint8_t              byte1;
  uint8_t              li_data_7[1];
};
//-----------------------
struct rlc_um_tx_data_pdu_struct {
  uint8_t              byte1;
  uint8_t              li_data_7[1];
};
//-----------------------
struct rlc_um_data_req_alloc {  // alloc enought bytes for sdu mngt also
  union {
    struct rlc_um_data_req dummy1;
    struct rlc_um_tx_sdu_management dummy2;
  } dummy;
};
#    endif
