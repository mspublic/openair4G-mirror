/*******************************************************************************
    OpenAirInterface
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is
   included in this distribution in the file called "COPYING". If not,
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@eurecom.fr

  Address      : Eurecom, Compus SophiaTech 450, route des chappes, 06451 Biot, France.

 *******************************************************************************/
/*! \file rlc_um_reassembly.h
* \brief This file defines the prototypes of the functions dealing with the reassembly of segments.
* \author GAUTHIER Lionel
* \date 2010-2011
* \version
* \note
* \bug
* \warning
*/
/** @defgroup _rlc_um_receiver_impl_ RLC UM Receiver Implementation
* @ingroup _rlc_um_impl_
* @{
*/
#    ifndef __RLC_UM_REASSEMBLY_PROTO_EXTERN_H__
#        define __RLC_UM_REASSEMBLY_PROTO_EXTERN_H__
//-----------------------------------------------------------------------------
#        ifdef RLC_UM_REASSEMBLY_C
#            define private_rlc_um_reassembly(x)    x
#            define protected_rlc_um_reassembly(x)  x
#            define public_rlc_um_reassembly(x)     x
#        else
#            ifdef RLC_UM_MODULE
#                define private_rlc_um_reassembly(x)
#                define protected_rlc_um_reassembly(x)  extern x
#                define public_rlc_um_reassembly(x)     extern x
#            else
#                define private_rlc_um_reassembly(x)
#                define protected_rlc_um_reassembly(x)
#                define public_rlc_um_reassembly(x)     extern x
#            endif
#        endif
#        include "rlc_um_entity.h"
//-----------------------------------------------------------------------------
/*! \fn void rlc_um_clear_rx_sdu (rlc_um_entity_t *rlcP)
* \brief    Erase the SDU in construction.
* \param[in]  rlcP        RLC UM protocol instance pointer.
*/
protected_rlc_um_reassembly(void rlc_um_clear_rx_sdu (rlc_um_entity_t *rlcP));

/*! \fn void rlc_um_reassembly (uint8_t * srcP, int32_t lengthP, rlc_um_entity_t *rlcP, frame_t frame)
* \brief    Reassembly lengthP bytes to the end of the SDU in construction.
* \param[in]  srcP        Pointer on data to be reassemblied.
* \param[in]  lengthP     Length to reassembly.
* \param[in]  rlcP        RLC UM protocol instance pointer.
* \param[in]  frame       Frame index.
*/
protected_rlc_um_reassembly(void     rlc_um_reassembly (uint8_t * srcP, int32_t lengthP, rlc_um_entity_t *rlcP, frame_t frame));

/*! \fn void rlc_um_send_sdu (rlc_um_entity_t *rlcP,frame_t frame,eNB_flag_t eNB_flagP)
* \brief    Send SDU if any reassemblied to upper layer.
* \param[in]  rlcP        RLC UM protocol instance pointer.
* \param[in]  frameP      Frame index.
* \param[in]  eNB_flagP   Boolean to know if rlcP is in eNB (1 or true), or in UE (0 or false).
*/
protected_rlc_um_reassembly(void     rlc_um_send_sdu (rlc_um_entity_t *rlcP,frame_t frameP,eNB_flag_t eNB_flagP));
/** @} */
#    endif
