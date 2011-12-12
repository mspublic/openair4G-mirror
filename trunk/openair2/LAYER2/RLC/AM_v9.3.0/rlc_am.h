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
/*! \file rlc_am.h
* \brief This file, and only this file must be included by code that interact with RLC AM layer.
* \author GAUTHIER Lionel
* \date 2010-2011
* \version
* \company Eurecom
* \email: lionel.gauthier@eurecom.fr
* \note
* \bug
* \warning
*/
/** @defgroup _rlc_am_impl_ RLC AM Layer Reference Implementation
* @ingroup _rlc_impl_
* @{
*/

#    ifndef __RLC_AM_H__
#        define __RLC_AM_H__
#        ifdef RLC_AM_C
#            define private_rlc_am(x)
#            define protected_rlc_am(x)
#            define public_rlc_am(x)
#        else
#            ifdef RLC_AM_MODULE
#                define private_rlc_am(x)
#                define protected_rlc_am(x)  extern x
#                define public_rlc_am(x)     extern x
#            else
#                define private_rlc_am(x)
#                define protected_rlc_am(x)
#                define public_rlc_am(x)     extern x
#            endif
#        endif
#        include "platform_types.h"
#        include "rlc_def.h"
#        include "rlc_def_lte.h"
#        include "rlc_am_constants.h"
#        include "rlc_am_structs.h"
#        include "rlc_am_entity.h"
#        include "rlc_am_windows.h"
#        include "mem_block.h"
#        include "rlc_am_in_sdu.h"
#        include "rlc_am_segment.h"
#        include "rlc_am_segments_holes.h"
#        include "rlc_am_timer_poll_retransmit.h"
#        include "rlc_am_timer_reordering.h"
#        include "rlc_am_timer_status_prohibit.h"
#        include "rlc_am_retransmit.h"
#        include "rlc_am_receiver.h"
#        include "rlc_am_status_report.h"
#        include "rlc_am_rx_list.h"
#        include "rlc_am_reassembly.h"
#        include "rlc_am_init.h"
//#        include "rlc_am_test.h"

#ifdef USER_MODE
//#        include "rlc_am_very_simple_test.h"
#endif

/*! \struct  rlc_am_info_t
* \brief Structure containing RLC AM configuration parameters.
*/
typedef volatile struct {
    u16_t max_retx_threshold;  /*!< \brief Maximum number of retransmissions for one RLC AM PDU. */
    u16_t poll_pdu;            /*!< \brief Generate a status each poll_pdu pdu sent. */
    u16_t poll_byte;           /*!< \brief Generate a status each time poll_byte bytes have been sent. */
    u32_t t_poll_retransmit;   /*!< \brief t-PollRetransmit timer initial value. */
    u32_t t_reordering;        /*!< \brief t-Reordering timer initial value. */
    u32_t t_status_prohibit;   /*!< \brief t-StatusProhibit timer initial value. */
} rlc_am_info_t;


/*! \fn void     rlc_am_release (rlc_am_entity_t *rlcP)
* \brief    Empty function, TO DO.
* \param[in]  rlcP                      RLC AM protocol instance pointer.
*/
public_rlc_am(void     rlc_am_release (rlc_am_entity_t *rlcP);)


/*! \fn void config_req_rlc_am (rlc_am_entity_t *rlcP, module_id_t module_idP, rlc_am_info_t * config_amP, u8_t rb_idP, rb_type_t rb_typeP)
* \brief    Request the maximum number of bytes that can be served by RLC instance to MAC and fix the amount of bytes requested by MAC for next RLC transmission. After this configuration the RLC AM protocol instance will be in RLC_DATA_TRANSFER_READY_STATE state.
* \param[in]  rlcP                      RLC AM protocol instance pointer.
* \param[in]  module_idP                Virtualized module identifier.
* \param[in]  config_amP                Configuration parameters for RLC UM instance.
* \param[in]  rb_idP                    Radio bearer identifier.
* \param[in]  rb_typeP                  Radio bearer type (Signalling or Data).
*/
public_rlc_am(void     config_req_rlc_am (rlc_am_entity_t *rlcP, module_id_t module_idP, rlc_am_info_t * config_amP, u8_t rb_idP, rb_type_t rb_typeP);)

/*! \fn void     rlc_am_stat_req     (rlc_am_entity_t *rlcP, unsigned int* tx_pdcp_sdu,unsigned int* tx_pdcp_sdu_discarded,unsigned int* tx_retransmit_pdu_unblock,unsigned int* tx_retransmit_pdu_by_status,unsigned int* tx_retransmit_pdu,unsigned int* tx_data_pdu,unsigned int* tx_control_pdu,unsigned int* rx_sdu,unsigned int* rx_error_pdu,unsigned int* rx_data_pdu,unsigned int* rx_data_pdu_out_of_window,unsigned int* rx_control_pdu)
* \brief    Request TX and RX statistics of a RLC UM protocol instance.
* \param[in]  rlcP                      RLC UM protocol instance pointer.
* \param[out] tx_pdcp_sdu               Number of transmitted SDUs coming from upper layers.
* \param[out] tx_pdcp_sdu_discarded     Number of discarded SDUs coming from upper layers.
* \param[out] tx_retransmit_pdu_unblock
* \param[out] tx_retransmit_pdu_by_status  Number of re-transmitted data PDUs due to status reception.
* \param[out] tx_retransmit_pdu         Number of re-transmitted data PDUs to lower layers.
* \param[out] tx_data_pdu               Number of transmitted data PDUs to lower layers.
* \param[out] tx_control_pdu            Number of transmitted control PDUs to lower layers.
* \param[out] rx_sdu                    Number of reassemblied SDUs, sent to upper layers.
* \param[out] rx_error_pdu              Number of received PDUs from lower layers, marked as containing an error.
* \param[out] rx_data_pdu               Number of received PDUs from lower layers.
* \param[out] rx_data_pdu_out_of_window Number of data PDUs received out of the receive window.
* \param[out] rx_control_pdu            Number of control PDUs received.
*/
public_rlc_am(void     rlc_am_stat_req     (rlc_am_entity_t *rlcP,
                              unsigned int* tx_pdcp_sdu,
                              unsigned int* tx_pdcp_sdu_discarded,
                              unsigned int* tx_retransmit_pdu_unblock,
                              unsigned int* tx_retransmit_pdu_by_status,
                              unsigned int* tx_retransmit_pdu,
                              unsigned int* tx_data_pdu,
                              unsigned int* tx_control_pdu,
                              unsigned int* rx_sdu,
                              unsigned int* rx_error_pdu,
                              unsigned int* rx_data_pdu,
                              unsigned int* rx_data_pdu_out_of_window,
                              unsigned int* rx_control_pdu);)

/*! \fn void     rlc_am_get_pdus (void *rlcP)
* \brief    Request the segmentation of SDUs based on status previously sent by MAC.
* \param[in]  rlcP                      RLC AM protocol instance pointer.
*/
private_rlc_am(   void     rlc_am_get_pdus (void *argP);)

/*! \fn void rlc_am_rx (void *rlcP, struct mac_data_ind data_indication)
* \brief    Process the received PDUs from lower layer.
* \param[in]  rlcP                      RLC AM protocol instance pointer.
* \param[in]  data_indication           PDUs from MAC.
*/
protected_rlc_am( void     rlc_am_rx (void *, struct mac_data_ind);)

/*! \fn struct mac_status_resp rlc_am_mac_status_indication (void *rlcP, u16_t tbs_sizeP, struct mac_status_ind tx_statusP)
* \brief    Request the maximum number of bytes that can be served by RLC instance to MAC and fix the amount of bytes requested by MAC for next RLC transmission.
* \param[in]  rlcP                      RLC AM protocol instance pointer.
* \param[in]  tbs_sizeP                 Number of bytes requested by MAC for next transmission.
* \param[in]  tx_statusP                Transmission status given by MAC on previous MAC transmission of the PDU.
* \return     The maximum number of bytes that can be served by RLC instance to MAC.
*/
public_rlc_am(    struct mac_status_resp rlc_am_mac_status_indication (void *rlcP, u16_t tbs_sizeP, struct mac_status_ind tx_statusP);)

/*! \fn struct mac_data_req rlc_am_mac_data_request (void *rlcP)
* \brief    Gives PDUs to lower layer MAC.
* \param[in]  rlcP                      RLC AM protocol instance pointer.
* \return     A PDU of the previously requested number of bytes, and the updated maximum number of bytes that can be served by RLC instance to MAC for next RLC transmission.
*/
public_rlc_am(    struct mac_data_req rlc_am_mac_data_request (void *rlcP);)

/*! \fn void     rlc_am_mac_data_indication (void *rlcP, struct mac_data_ind data_indP)
* \brief    Receive PDUs from lower layer MAC.
* \param[in]  rlcP             RLC UM protocol instance pointer.
* \param[in]  data_indP        PDUs from MAC.
*/
public_rlc_am(    void     rlc_am_mac_data_indication (void *rlcP, struct mac_data_ind data_indP);)

/*! \fn void     rlc_am_data_req (void *rlcP, mem_block_t *sduP)
* \brief    Interface with higher layers, buffer higher layer SDUS for transmission.
* \param[in]  rlcP             RLC AM protocol instance pointer.
* \param[in]  sduP             SDU. (A struct rlc_am_data_req is mapped on sduP->data.)
*/
public_rlc_am(    void     rlc_am_data_req (void *rlcP, mem_block_t *sduP);)
/** @} */
#    endif
