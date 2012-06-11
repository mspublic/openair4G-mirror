/*******************************************************************************
 *
 * Eurecom OpenAirInterface 3
 * Copyright(c) 2012 Eurecom
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * The full GNU General Public License is included in this distribution in
 * the file called "COPYING".
 *
 * Contact Information
 * Openair Admin: openair_admin@eurecom.fr
 * Openair Tech : openair_tech@eurecom.fr
 * Forums       : http://forums.eurecom.fsr/openairinterface
 * Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France
 *
 *******************************************************************************/
/*! \file MIH_C_F4_data_types_for_links_codec.h
 * \brief This file defines the prototypes of the functions for coding and decoding data types for links defined in Std 802.21-2008 Table F4.
 * \author BRIZZOLA Davide, GAUTHIER Lionel, MAUREL Frederic, WETTERWALD Michelle
 * \date 2012
 * \version
 * \note
 * \bug
 * \warning
 */
/** \defgroup MIH_C_F4_DATA_TYPES_FOR_LINKS_CODEC F4 Data types for links codec
 * \ingroup MIH_C_INTERFACE
 *
 *  @{
 */

#ifndef __MIH_C_F4_DATA_TYPES_FOR_LINKS_CODEC_H__
#    define __MIH_C_F4_DATA_TYPES_FOR_LINKS_CODEC_H__
//-----------------------------------------------------------------------------
#        ifdef MIH_C_F4_DATA_TYPES_FOR_LINKS_CODEC_C
#            define private_F4_codec(x)    x
#            define protected_F4_codec(x)  x
#            define public_F4_codec(x)     x
#        else
#            ifdef MIH_C_INTERFACE
#                define private_F4_codec(x)
#                define protected_F4_codec(x)  extern x
#                define public_F4_codec(x)     extern x
#            else
#                define private_F4_codec(x)
#                define protected_F4_codec(x)
#                define public_F4_codec(x)     extern x
#            endif
#        endif
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
//-----------------------------------------------------------------------------
#include "MIH_C.h"
//-----------------------------------------------------------------------------
public_F4_codec(inline void MIH_C_DEV_STATE_RSP_encode(Bit_Buffer_t* bbP, MIH_C_DEV_STATE_RSP_T *dataP);)
public_F4_codec(inline void MIH_C_DEV_STATE_RSP_decode(Bit_Buffer_t* bbP, MIH_C_DEV_STATE_RSP_T *dataP);)
public_F4_codec(unsigned int MIH_C_LINK_ID2String(MIH_C_LINK_ID_T *dataP, char* bufP);)
public_F4_codec(inline void MIH_C_LINK_ID_encode(Bit_Buffer_t* bbP, MIH_C_LINK_ID_T *dataP);)
public_F4_codec(inline void MIH_C_LINK_ID_decode(Bit_Buffer_t* bbP, MIH_C_LINK_ID_T *dataP);)
public_F4_codec(inline void MIH_C_LINK_ACTION_REQ_encode(Bit_Buffer_t* bbP, MIH_C_LINK_ACTION_REQ_T *dataP);)
public_F4_codec(inline void MIH_C_LINK_ACTION_REQ_decode(Bit_Buffer_t* bbP, MIH_C_LINK_ACTION_REQ_T *dataP);)
public_F4_codec(unsigned int MIH_C_SIG_STRENGTH2String(MIH_C_SIG_STRENGTH_T *dataP, char* bufP);)
public_F4_codec(inline void MIH_C_SIG_STRENGTH_encode(Bit_Buffer_t* bbP, MIH_C_SIG_STRENGTH_T *dataP);)
public_F4_codec(inline void MIH_C_SIG_STRENGTH_decode(Bit_Buffer_t* bbP, MIH_C_SIG_STRENGTH_T *dataP);)
public_F4_codec(unsigned int MIH_C_LINK_SCAN_RSP2String(MIH_C_LINK_SCAN_RSP_T *dataP, char* bufP);)
public_F4_codec(inline void MIH_C_LINK_SCAN_RSP_encode(Bit_Buffer_t* bbP, MIH_C_LINK_SCAN_RSP_T *dataP);)
public_F4_codec(inline void MIH_C_LINK_SCAN_RSP_decode(Bit_Buffer_t* bbP, MIH_C_LINK_SCAN_RSP_T *dataP);)
public_F4_codec(inline void MIH_C_LINK_ACTION_RSP_encode(Bit_Buffer_t* bbP, MIH_C_LINK_ACTION_RSP_T *dataP);)
public_F4_codec(inline void MIH_C_LINK_ACTION_RSP_decode(Bit_Buffer_t* bbP, MIH_C_LINK_ACTION_RSP_T *dataP);)
public_F4_codec(unsigned int MIH_C_THRESHOLD2String(MIH_C_THRESHOLD_T *dataP, char* bufP);)
public_F4_codec(inline void MIH_C_THRESHOLD_encode(Bit_Buffer_t* bbP, MIH_C_THRESHOLD_T *dataP);)
public_F4_codec(inline void MIH_C_THRESHOLD_decode(Bit_Buffer_t* bbP, MIH_C_THRESHOLD_T *dataP);)
public_F4_codec(unsigned int MIH_C_LINK_PARAM_TYPE2String( MIH_C_LINK_PARAM_TYPE_T *dataP, char* bufP);)
public_F4_codec(inline void MIH_C_LINK_PARAM_TYPE_encode(Bit_Buffer_t* bbP, MIH_C_LINK_PARAM_TYPE_T *dataP);)
public_F4_codec(inline void MIH_C_LINK_PARAM_TYPE_decode(Bit_Buffer_t* bbP, MIH_C_LINK_PARAM_TYPE_T *dataP);)
public_F4_codec(unsigned int MIH_C_LINK_CFG_PARAM2String(MIH_C_LINK_CFG_PARAM_T *dataP, char* bufP);)
public_F4_codec(inline void MIH_C_LINK_CFG_PARAM_encode(Bit_Buffer_t* bbP, MIH_C_LINK_CFG_PARAM_T *dataP);)
public_F4_codec(inline void MIH_C_LINK_CFG_PARAM_decode(Bit_Buffer_t* bbP, MIH_C_LINK_CFG_PARAM_T *dataP);)
public_F4_codec(unsigned int MIH_C_LINK_CFG_STATUS2String(MIH_C_LINK_CFG_STATUS_T *dataP, char* bufP);)
public_F4_codec(inline void MIH_C_LINK_CFG_STATUS_encode(Bit_Buffer_t* bbP, MIH_C_LINK_CFG_STATUS_T *dataP);)
public_F4_codec(inline void MIH_C_LINK_CFG_STATUS_decode(Bit_Buffer_t* bbP, MIH_C_LINK_CFG_STATUS_T *dataP);)
public_F4_codec(unsigned int MIH_C_LINK_DESC_RSP2String(MIH_C_LINK_DESC_RSP_T *dataP, char* bufP);)
public_F4_codec(inline void MIH_C_LINK_DESC_RSP_encode(Bit_Buffer_t* bbP, MIH_C_LINK_DESC_RSP_T *dataP);)
public_F4_codec(inline void MIH_C_LINK_DESC_RSP_decode(Bit_Buffer_t* bbP, MIH_C_LINK_DESC_RSP_T *dataP);)
public_F4_codec(unsigned int MIH_C_LINK_PARAM2String(MIH_C_LINK_PARAM_T *dataP, char* bufP);)
public_F4_codec(inline void MIH_C_LINK_PARAM_encode(Bit_Buffer_t* bbP, MIH_C_LINK_PARAM_T *dataP);)
public_F4_codec(inline void MIH_C_LINK_PARAM_decode(Bit_Buffer_t* bbP, MIH_C_LINK_PARAM_T *dataP);)
public_F4_codec(unsigned int MIH_C_LINK_PARAM_RPT2String(MIH_C_LINK_PARAM_RPT_T *dataP, char* bufP);)
public_F4_codec(inline void MIH_C_LINK_PARAM_RPT_encode(Bit_Buffer_t* bbP, MIH_C_LINK_PARAM_RPT_T *dataP);)
public_F4_codec(inline void MIH_C_LINK_PARAM_RPT_decode(Bit_Buffer_t* bbP, MIH_C_LINK_PARAM_RPT_T *dataP);)
public_F4_codec(inline void MIH_C_LINK_POA_LIST_encode(Bit_Buffer_t* bbP, MIH_C_LINK_POA_LIST_T *dataP);)
public_F4_codec(inline void MIH_C_LINK_POA_LIST_decode(Bit_Buffer_t* bbP, MIH_C_LINK_POA_LIST_T *dataP);)
public_F4_codec(unsigned int MIH_C_LINK_STATES_RSP2String(MIH_C_LINK_STATES_RSP_T *dataP, char* bufP);)
public_F4_codec(inline void MIH_C_LINK_STATES_RSP_encode(Bit_Buffer_t* bbP, MIH_C_LINK_STATES_RSP_T *dataP);)
public_F4_codec(inline void MIH_C_LINK_STATES_RSP_decode(Bit_Buffer_t* bbP, MIH_C_LINK_STATES_RSP_T *dataP);)
public_F4_codec(inline void MIH_C_LINK_STATUS_REQ_encode(Bit_Buffer_t* bbP, MIH_C_LINK_STATUS_REQ_T *dataP);)
public_F4_codec(inline void MIH_C_LINK_STATUS_REQ_decode(Bit_Buffer_t* bbP, MIH_C_LINK_STATUS_REQ_T *dataP);)
public_F4_codec(inline void MIH_C_LINK_STATUS_RSP_encode(Bit_Buffer_t* bbP, MIH_C_LINK_STATUS_RSP_T *dataP);)
public_F4_codec(inline void MIH_C_LINK_STATUS_RSP_decode(Bit_Buffer_t* bbP, MIH_C_LINK_STATUS_RSP_T *dataP);)
public_F4_codec(inline void MIH_C_LINK_TUPLE_ID_encode(Bit_Buffer_t* bbP, MIH_C_LINK_TUPLE_ID_T *dataP);)
public_F4_codec(inline void MIH_C_LINK_TUPLE_ID_decode(Bit_Buffer_t* bbP, MIH_C_LINK_TUPLE_ID_T *dataP);)
public_F4_codec(inline void MIH_C_LINK_DET_INFO_encode(Bit_Buffer_t* bbP, MIH_C_LINK_DET_INFO_T *dataP);)
public_F4_codec(inline void MIH_C_LINK_DET_INFO_decode(Bit_Buffer_t* bbP, MIH_C_LINK_DET_INFO_T *dataP);)
public_F4_codec(unsigned int MIH_C_TH_ACTION2String2(MIH_C_TH_ACTION_T *actionP, char* bufP);)
public_F4_codec(unsigned int MIH_C_THRESHOLD_XDIR2String2(MIH_C_THRESHOLD_XDIR_T *valP, char* bufP);)
public_F4_codec(unsigned int MIH_C_LINK_PARAM_GEN2String2(MIH_C_LINK_PARAM_GEN_T *valP, char* bufP);)

#endif
/** @}*/

