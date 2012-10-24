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
* \note
* \bug
* \warning
*/
#    ifndef __RLC_UM_STRUCTS_H__
#        define __RLC_UM_STRUCTS_H__

#        include "platform_types.h"
#        include "list.h"
#        include "rlc_um_constants.h"
#        include "mac_primitives.h"
#        include "rlc_primitives.h"
//#ifdef USER_MODE
#        include "mac_rlc_primitives.h"
//#endif //USER_MODE
//-----------------------
/**
* @addtogroup _rlc_um_impl_
* @{
*/
/*! \struct  rlc_um_tx_sdu_management_t
* \brief Structure containing SDU variables related to its segmentation and transmission.
*/
typedef struct rlc_um_tx_sdu_management {
    u8_t             *first_byte;                 /*!< \brief Pointer on SDU payload. */
    s32_t             sdu_creation_time;          /*!< \brief Time stamped with mac_xface->frame. */
    u16_t             sdu_remaining_size;         /*!< \brief Remaining size in bytes to be filled in a PDU. */
  u16_t             sdu_test_remaining_size;
  u16_t             sdu_segmented_size;           /*!< \brief Bytes already segmented in a/several PDU(s). */
  u16_t             sdu_size;                     /*!< \brief SDU size in bytes. */
}rlc_um_tx_sdu_management_t;
/** @} */

/**
* @addtogroup _rlc_um_segment_impl_
* @{
*/
/*! \struct  rlc_um_pdu_sn_5_t
* \brief Structure helping coding and decoding the first byte of a UMD PDU.
*/
typedef struct rlc_um_pdu_sn_5 {
/*  u8_t fi:2;
  u8_t e:1;
  u8_t sn:5;*/
  u8_t     b1;      /*!< \brief 1st byte. */
  u8_t     data[3]; /*!< \brief Following bytes. */
} __attribute__((__packed__)) rlc_um_pdu_sn_5_t ;

/*! \struct  rlc_um_pdu_sn_10_t
* \brief Structure helping coding and decoding the first 2 bytes of a UMD PDU.
*/
typedef struct rlc_um_pdu_sn_10 {
    u8_t  b1;      /*!< \brief 1st byte. */
    u8_t  b2;      /*!< \brief 2nd byte. */
    u8_t  data[2]; /*!< \brief Following bytes. */
}__attribute__((__packed__)) rlc_um_pdu_sn_10_t ;

/*! \struct  rlc_am_e_li_t
* \brief Structure helping coding and decoding LI and e bits in UMD PDUs.
*/
typedef struct rlc_um_e_li {
    u8_t  b1; /*!< \brief 1st byte. */
    u8_t  b2; /*!< \brief 2nd byte. */
    u8_t  b3; /*!< \brief 3rd byte. */
}rlc_um_e_li_t;
/** @} */
/**
* @addtogroup _rlc_um_segment_impl_
* @{
*/
/*! \struct  rlc_um_pdu_info_t
* \brief Structure for storing decoded informations from the header of a UMD PDU.
*/
typedef struct rlc_um_pdu_info {
    u16_t  free_bits:3; /*!< \brief unused bits in bitfield. */
    u16_t  fi:2;        /*!< \brief Framing Info field. */
    u16_t  e:1;         /*!< \brief Extension bit field. */
    u16_t  sn:10;       /*!< \brief Sequence Number field. */
    u16_t  num_li;      /*!< \brief Number of Length Indicators. */
    s16_t  li_list[RLC_UM_SEGMENT_NB_MAX_LI_PER_PDU]; /*!< \brief List of Length Indicators. */
    s16_t  hidden_size; /*!< \brief Part of payload size in bytes that is not included in the sum of LI fields. */;
    u8_t*  payload;     /*!< \brief Pointer on PDU payload. */
    s16_t  payload_size;/*!< \brief Size of payload in bytes. */
    s16_t  header_size; /*!< \brief Size of header in bytes (including SO field and LI fields). */
} rlc_um_pdu_info_t ;
/** @} */


struct rlc_um_data_req_alloc {  // alloc enought bytes for sdu mngt also
  union {
    struct rlc_um_data_req dummy1;
    struct rlc_um_tx_sdu_management dummy2;
  } dummy;
};

#    endif
