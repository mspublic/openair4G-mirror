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
/*! \file rlc_um_entity.h
* \brief This file defines the RLC UM variables stored in a struct called rlc_um_entity_t.
* \author GAUTHIER Lionel
* \date 2010-2011
* \version
* \note The rlc_um_entity_t structure store protocol variables, statistic variables, allocation variables, buffers and other miscellaneous variables.
* \bug
* \warning
*/
#    ifndef __RLC_UM_ENTITY_H__
#        define __RLC_UM_ENTITY_H__

#        include "platform_types.h"
#        include "platform_constants.h"
#        include "list.h"
#        include "rlc_primitives.h"
#        include "rlc_def.h"

/*! \struct  rlc_um_entity_t
* \brief Structure containing a RLC UM instance protocol variables, statistic variables, allocation variables, buffers and other miscellaneous variables.
*/

typedef struct rlc_um_entity {
  module_id_t       module_id;          /*!< \brief Virtualization index for this protocol instance, means handset or eNB index.*/
  u8_t              allocation;         /*!< \brief Boolean for rlc_am_entity_t struct allocation. */
  u8_t              is_uplink_downlink; /*!< \brief Is this instance is a transmitter, a receiver or both? */
  u8_t              protocol_state;     /*!< \brief Protocol state, can be RLC_NULL_STATE, RLC_DATA_TRANSFER_READY_STATE, RLC_LOCAL_SUSPEND_STATE. */
  u16_t             is_data_plane;      /*!< \brief To know if the RLC belongs to a data radio bearer or a signalling radio bearer, for statistics and trace purpose. */
  boolean_t         is_enb;             /*!< \brief To know if the RLC belongs to a eNB or UE. */
  //-----------------------------
  // PROTOCOL VARIABLES
  //-----------------------------
  u16_t              vt_us; /*!< \brief This state variable holds the value of the SN to be assigned for the next newly generated UMD PDU. It is initially set to 0, and is updated whenever the UM RLC entity delivers an UMD PDU with SN = VT(US). */
  u16_t              vr_ur; /*!< \brief UM receive state variable. This state variable holds the value of the SN of the earliest UMD PDU that is still considered for reordering. It is initially set to 0. */
  u16_t              vr_ux; /*!< \brief UM t-Reordering state variable. This state variable holds the value of the SN following the SN of the UMD PDU which triggered t-Reordering. */
  u16_t              vr_uh; /*!< \brief UM highest received state variable. This state variable holds the value of the SN following the SN of the UMD PDU with the highest SN among received UMD PDUs, and it serves as the higher edge of the reordering window. It is initially set to 0. */
  //-----------------------------
  // TIMERS
  //-----------------------------
  signed int        timer_reordering;           /*!< \brief Timer t-Reordering starting time frame, this timer is used by the receiving side of an AM RLC entity and receiving UM RLC entity in order to detect loss of RLC PDUs at lower layer. If t-Reordering is running, t-Reordering shall not be started additionally, i.e. only one t-Reordering per RLC entity is running at a given time. */
  signed int        timer_reordering_init;     /*!< \brief Timer t-Reordering initial configuration value. */
  signed int        timer_reordering_running;  /*!< \brief Boolean to know if timer t-Reordering is running. */
  //*****************************************************************************
  // CONFIGURATION PARAMETERS
  //*****************************************************************************
  u8_t              sn_length;                  /*!< \brief Length of sequence number in bits, can be 5 or 10. */
  u8_t              header_min_length_in_bytes; /*!< \brief Length of PDU header, can be 1 or 2 bytes. */
  signed int        sn_modulo;                  /*!< \brief Module of the sequence number of PDU, can be RLC_UM_SN_5_BITS_MODULO or RLC_UM_SN_10_BITS_MODULO. */
  signed int        um_window_size;
  //-----------------------------
  // tranmission
  //-----------------------------
  // sdu communication;
  mem_block_t     **input_sdus;                /*!< \brief Input SDU buffer (for SDUs coming from upper layers). Should be accessed as an array. */
  mem_block_t     * input_sdus_alloc;          /*!< \brief Allocated memory for the input SDU buffer (for SDUs coming from upper layers). */
  u16_t             size_input_sdus_buffer;    /*!< \brief Size of the input SDU buffer. */
  u16_t             nb_sdu;                    /*!< \brief Total number of SDUs in input_sdus[] */
  u16_t             next_sdu_index;            /*!< \brief Next SDU index for a new incomin SDU in input_sdus[]. */
  u16_t             current_sdu_index;         /*!< \brief Current SDU index in input_sdus array to be segmented. */
  u32_t             buffer_occupancy;          /*!< \brief Number of bytes contained in input_sdus buffer.*/
  u32_t             nb_bytes_requested_by_mac; /*!< \brief Number of bytes requested by lower layer for next transmission. */
  list_t            pdus_to_mac_layer;         /*!< \brief PDUs buffered for transmission to MAC layer. */
  //*****************************************************************************
  // RECEIVER
  //*****************************************************************************
  mem_block_t    *  output_sdu_in_construction;     /*!< \brief Memory area where a complete SDU is reassemblied before being send to upper layers. */
  s32_t             output_sdu_size_to_write;       /*!< \brief Size of the reassemblied SDU. */

  mem_block_t     **dar_buffer;                     /*!< \brief Array of rx PDUs. */
  mem_block_t      *dar_buffer_alloc;               /*!< \brief Allocated memory for the DAR buffer. */
  list_t            pdus_from_mac_layer;            /*!< \brief Not Used. */

  u16_t             rb_id;                          /*!< \brief Radio bearer identifier, for statistics and trace purpose. */
  u16_t             last_reassemblied_sn;           /*!< \brief Sequence number of the last reassemblied PDU. */
  u16_t             last_reassemblied_missing_sn;   /*!< \brief Sequence number of the last found missing PDU. */
  u16_t             reassembly_missing_sn_detected; /*!< \brief Act as a boolean, set if a hole in the sequence numbering of received PDUs has been found. */
  //-----------------------------
  // STATISTICS
  //-----------------------------
  u32_t             tx_sdus;                        /*!< \brief Not updated. */
  u32_t             rx_sdus;                        /*!< \brief Number of SDUs reassemblied and sent to upper layers. */
  u32_t             tx_pdus;                        /*!< \brief Number of PDUs sent to lower layers. */
  u32_t             rx_pdus;                        /*!< \brief Number of PDUs received from lower layers. */
  u32_t             rx_pdus_in_error;               /*!< \brief Number of PDUs received from lower layers marked as containing an error. */
  u8_t              first_pdu;                      /*!< \brief Act as a boolean, tells if the next PDU is the first PDU to be received. */

  unsigned int tx_pdcp_sdu;                         /*!< \brief For statistic report, number of transmitted SDUs coming from upper layers. */
  unsigned int tx_pdcp_sdu_discarded;               /*!< \brief For statistic report, number of discarded SDUs coming from upper layers. */
  unsigned int tx_data_pdu;                         /*!< \brief For statistic report, number of transmitted PDUs to lower layers. */
  unsigned int rx_sdu;                              /*!< \brief For statistic report, number of reassemblied SDUs, sent to upper layers. */
  unsigned int rx_error_pdu;                        /*!< \brief For statistic report, number of received PDUs from lower layers, marked as containing an error. */
  unsigned int rx_data_pdu;                         /*!< \brief For statistic report, number of received PDUs from lower layers. */
  unsigned int rx_data_pdu_out_of_window;           /*!< \brief Number of data PDUs received out of the receive window. */
}rlc_um_entity_t;
/** @} */
#    endif
