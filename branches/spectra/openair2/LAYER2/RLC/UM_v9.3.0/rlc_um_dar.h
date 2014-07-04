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
/*! \file rlc_um_dar.h
* \brief This file defines the prototypes of the functions dealing with the reassembly buffer.
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
/*! \fn signed int rlc_um_get_pdu_infos(frame_t frameP,rlc_um_pdu_sn_10_t* header_pP, int16_t total_sizeP, rlc_um_pdu_info_t* pdu_info_pP, uint8_t sn_lengthP)
* \brief    Extract PDU informations (header fields, data size, etc) from the serialized PDU.
* \param[in]  frameP             Frame index.
* \param[in]  header_pP          RLC UM header PDU pointer.
* \param[in]  total_sizeP        Size of RLC UM PDU.
* \param[in]  pdu_info_pP        Structure containing extracted informations from PDU.
* \param[in]  sn_lengthP         Sequence number length in bits in PDU header (5 or 10).
* \return     0 if no error was encountered during the parsing of the PDU, else -1;
*/
protected_rlc_um_dar(  signed int rlc_um_get_pdu_infos(
    const frame_t               frameP,
    rlc_um_pdu_sn_10_t  * const header_pP,
    const sdu_size_t            total_sizeP,
    rlc_um_pdu_info_t   * const pdu_info_pP,
    const uint8_t               sn_lengthP));

/*! \fn int rlc_um_read_length_indicators(unsigned char**data_ppP, rlc_um_e_li_t* e_li_pP, unsigned int* li_array_pP, unsigned int *num_li_pP, sdu_size_t *data_size_pP)
* \brief    Reset protocol variables and state variables to initial values.
* \param[in,out]  data_ppP       Pointer on data payload.
* \param[in]      e_li_pP        Pointer on first LI + e bit in PDU.
* \param[in,out]  li_array_pP    Array containing read LI.
* \param[in,out]  num_li_pP      Pointer on number of LI read.
* \param[in,out]  data_size_pP   Pointer on data size.
* \return     0 if no error was encountered during the parsing of the PDU, else -1;
*/
private_rlc_um_dar(  int rlc_um_read_length_indicators(unsigned char**data_ppP, rlc_um_e_li_t* e_li_pP, unsigned int* li_array_pP, unsigned int *num_li_pP, sdu_size_t *data_size_pP));

/*! \fn void rlc_um_try_reassembly      (rlc_um_entity_t *rlc_pP, frame_t frameP, eNB_flag_t eNB_flagP, rlc_sn_t start_snP, rlc_sn_t end_snP)
* \brief    Try reassembly PDUs from DAR buffer, starting at sequence number snP.
* \param[in]  rlc_pP      RLC UM protocol instance pointer.
* \param[in]  frameP      Frame index.
* \param[in]  eNB_flagP   Flag to indicate eNB (1) or UE (0).
* \param[in]  start_snP   First PDU to be reassemblied if possible.
* \param[in]  end_snP     Last excluded highest sequence number of PDU to be reassemblied.
*/
private_rlc_um_dar(  void rlc_um_try_reassembly      (rlc_um_entity_t *rlc_pP, frame_t frameP, eNB_flag_t eNB_flagP, rlc_sn_t start_snP, rlc_sn_t end_snP));

/*! \fn void rlc_um_check_timer_reordering(rlc_um_entity_t *rlc_pP,frame_t frameP)
* \brief      Check if timer reordering has timed-out, if so it is stopped and has the status "timed-out".
* \param[in]  rlc_pP            RLC UM protocol instance pointer.
* \param[in]  frameP            Frame index
*/
protected_rlc_um_dar(void rlc_um_check_timer_reordering(rlc_um_entity_t  *rlc_pP,frame_t frameP));

/*! \fn void rlc_um_stop_and_reset_timer_reordering(rlc_um_entity_t *rlc_pP,frame_t frameP)
* \brief      Stop and reset the timer reordering.
* \param[in]  rlc_pP            RLC UM protocol instance pointer.
* \param[in]  frameP            Frame index.
*/
protected_rlc_um_dar(void rlc_um_stop_and_reset_timer_reordering(rlc_um_entity_t *rlc_pP,frame_t frameP);)

/*! \fn void rlc_um_start_timer_reordering(rlc_um_entity_t *rlc_pP,frame_t frameP)
* \brief      Re-arm (based on RLC UM config parameter) and start timer reordering.
* \param[in]  rlc_pP            RLC UM protocol instance pointer.
* \param[in]  frameP            Frame index.
*/
protected_rlc_um_dar(void rlc_um_start_timer_reordering(rlc_um_entity_t *rlc_pP,frame_t frameP);)

/*! \fn void rlc_um_init_timer_reordering(rlc_um_entity_t *rlc_pP, uint32_t time_outP)
* \brief      Initialize the timer reordering with RLC UM time-out config parameter.
* \param[in]  rlc_pP            RLC UM protocol instance pointer.
* \param[in]  time_outP         Time-out in frameP units.
*/
protected_rlc_um_dar(void rlc_um_init_timer_reordering(rlc_um_entity_t *rlc_pP, uint32_t time_outP);)

/*! \fn void rlc_um_check_timer_dar_time_out(rlc_um_entity_t *rlc_pP,frame_t frameP,eNB_flag_t eNB_flagP)
* \brief    Check if t-Reordering expires and take the appropriate actions as described in 3GPP specifications.
* \param[in]  rlc_pP       RLC UM protocol instance pointer.
* \param[in]  frameP       Frame index.
* \param[in]  eNB_flagP    Flag to indicate eNB(1) or UE (1)
*/
protected_rlc_um_dar(  void rlc_um_check_timer_dar_time_out(rlc_um_entity_t *rlc_pP,frame_t frameP,eNB_flag_t eNB_flagP));

/*! \fn mem_block_t *rlc_um_remove_pdu_from_dar_buffer(rlc_um_entity_t *rlc_pP, uint16_t snP)
* \brief    Remove the PDU with sequence number snP from the DAR buffer and return it.
* \param[in]  rlc_pP        RLC UM protocol instance pointer.
* \param[in]  snP           Sequence number.
* \return     The PDU stored in the DAR buffer having sequence number snP, else return NULL.
*/
private_rlc_um_dar(  mem_block_t *rlc_um_remove_pdu_from_dar_buffer(rlc_um_entity_t *rlc_pP, uint16_t snP));

/*! \fn mem_block_t *rlc_um_remove_pdu_from_dar_buffer(rlc_um_entity_t *rlc_pP, uint16_t snP)
* \brief    Get the PDU with sequence number snP from the DAR buffer.
* \param[in]  rlc_pP        RLC UM protocol instance pointer.
* \param[in]  snP           Sequence number.
* \return     The PDU stored in the DAR buffer having sequence number snP, else return NULL.
*/
protected_rlc_um_dar(  inline mem_block_t* rlc_um_get_pdu_from_dar_buffer(rlc_um_entity_t *rlc_pP, uint16_t snP));

/*! \fn signed int rlc_um_in_window(rlc_um_entity_t *rlc_pP, frame_t frameP, rlc_sn_t lower_boundP, rlc_sn_t snP, rlc_sn_t higher_boundP)
* \brief    Compute if the sequence number of a PDU is in a window .
* \param[in]  rlc_pP         RLC UM protocol instance pointer.
* \param[in]  frameP         Frame index.
* \param[in]  lower_boundP   Lower bound of a window.
* \param[in]  snP            Sequence number of a theorical PDU.
* \param[in]  higher_boundP  Higher bound of a window.
* \return     -2 if lower_boundP  > sn, -1 if higher_boundP < sn, 0 if lower_boundP  < sn < higher_boundP, 1 if lower_boundP  == sn, 2 if higher_boundP == sn, 3 if higher_boundP == sn == lower_boundP.
*/
protected_rlc_um_dar(inline signed int rlc_um_in_window(rlc_um_entity_t *rlc_pP, frame_t frameP, rlc_sn_t lower_boundP, rlc_sn_t snP, rlc_sn_t higher_boundP));

/*! \fn signed int rlc_um_in_reordering_window(rlc_um_entity_t *rlc_pP, frame_t frameP, rlc_sn_t snP)
* \brief    Compute if the sequence number of a PDU is in a window .
* \param[in]  rlc_pP         RLC UM protocol instance pointer.
* \param[in]  frameP         Frame index.
* \param[in]  snP            Sequence number of a theorical PDU.
* \return     0 if snP is in reordering window, else -1.
*/
protected_rlc_um_dar(inline signed int rlc_um_in_reordering_window(rlc_um_entity_t *rlc_pP, frame_t frameP, rlc_sn_t snP));

/*! \fn void rlc_um_receive_process_dar (rlc_um_entity_t *rlc_pP, frame_t frameP, eNB_flag_t eNB_flagP, mem_block_t *pdu_mem_pP,rlc_um_pdu_sn_10_t *pdu_pP, sdu_size_t tb_sizeP)
* \brief    Apply the DAR process for a PDU: put it in DAR buffer and try to reassembly or discard it.
* \param[in]  rlc_pP     RLC UM protocol instance pointer.
* \param[in]  frameP     Frame index.
* \param[in]  eNB_flagP  Flag to indicated eNB (1) or UE (0).
* \param[in]  pdu_mem_pP mem_block_t wrapper for a UM PDU .
* \param[in]  pdu_pP     Pointer on the header of the UM PDU.
* \param[in]  tb_sizeP   Size of the UM PDU.
*/
protected_rlc_um_dar(void rlc_um_receive_process_dar (rlc_um_entity_t *rlc_pP, frame_t frameP, eNB_flag_t eNB_flagP, mem_block_t *pdu_mem_pP,rlc_um_pdu_sn_10_t *pdu_pP, sdu_size_t tb_sizeP));
/** @} */
#    endif
