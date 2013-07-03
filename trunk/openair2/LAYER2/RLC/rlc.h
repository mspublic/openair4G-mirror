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
/*! \file rlc.h
* \brief This file, and only this file must be included by external code that interact with RLC layer.
* \author GAUTHIER Lionel
* \date 2010-2011
* \version
* \note
* \bug
* \warning
*/
/** @defgroup _rlc_impl_ RLC Layer Reference Implementation
* @ingroup _ref_implementation_
* @{
*/
#ifndef __RLC_H__
#    define __RLC_H__

#    include "platform_types.h"
#    include "platform_constants.h"
#    include "rlc_am.h"
#    include "rlc_um.h"
#    include "rlc_tm.h"
#    include "rlc_am_structs.h"
#    include "rlc_tm_structs.h"
#    include "rlc_um_structs.h"
#    include "asn1_constants.h"
#    include "UTIL/LOG/log.h"
#    include "mem_block.h"
#    include "PHY/defs.h"
#    include "RLC-Config.h"
#    include "DRB-ToAddMod.h"
#    include "DRB-ToAddModList.h"
#    include "SRB-ToAddMod.h"
#    include "SRB-ToAddModList.h"
#ifdef Rel10
#include "PMCH-InfoList-r9.h"
#endif

//-----------------------------------------------------------------------------
#    ifdef RLC_MAC_C
#        define private_rlc_mac(x) x
#        define public_rlc_mac(x) x
#    else
#        define private_rlc_mac(x)
#        define public_rlc_mac(x) extern x
#    endif

#    ifdef RLC_MPLS_C
#        define private_rlc_mpls(x) x
#        define public_rlc_mpls(x) x
#    else
#        define private_rlc_mpls(x)
#        define public_rlc_mpls(x) extern x
#    endif

#    ifdef RLC_RRC_C
#        define private_rlc_rrc(x) x
#        define public_rlc_rrc(x) x
#    else
#        define private_rlc_rrc(x)
#        define public_rlc_rrc(x) extern x
#    endif

#    ifdef RLC_C
#        define private_rlc(x) x
#        define protected_rlc(x) x
#        define public_rlc(x) x
#    else
#        define private_rlc(x)
#        if defined(RLC_MAC_C) || defined(RLC_MPLS_C) || defined(RLC_RRC_C) || defined(RLC_AM_C) || defined(RLC_TM_C) || defined(RLC_UM_C) || defined (PDCP_C)
#            define protected_rlc(x) extern x
#        else
#            define protected_rlc(x)
#        endif
#        define public_rlc(x) extern x
#    endif

//-----------------------------------------------------------------------------
#define  RLC_OP_STATUS_OK                1
#define  RLC_OP_STATUS_BAD_PARAMETER     22
#define  RLC_OP_STATUS_INTERNAL_ERROR    2
#define  RLC_OP_STATUS_OUT_OF_RESSOURCES 3

#define  RLC_SDU_CONFIRM_YES   1
#define  RLC_SDU_CONFIRM_NO    0

#define  RLC_MUI_UNDEFINED     0


/*! \struct  rlc_info_t
* \brief Structure containing RLC protocol configuration parameters.
*/
typedef volatile struct {
  rlc_mode_t             rlc_mode;
  union {
      rlc_am_info_t              rlc_am_info; /*!< \sa rlc_am.h. */
      rlc_tm_info_t              rlc_tm_info; /*!< \sa rlc_tm.h. */
      rlc_um_info_t              rlc_um_info; /*!< \sa rlc_um.h. */
  }rlc;
} rlc_info_t;

/*! \struct  mac_rlc_status_resp_t
* \brief Primitive exchanged between RLC and MAC informing about the buffer occupancy of the RLC protocol instance.
*/
typedef  struct {
    u32_t                        bytes_in_buffer; /*!< \brief Bytes buffered in RLC protocol instance. */
    u32_t                        pdus_in_buffer;  /*!< \brief Number of PDUs buffered in RLC protocol instance (OBSOLETE). */
} mac_rlc_status_resp_t;


/*! \struct  mac_rlc_max_rx_header_size_t
* \brief Usefull only for debug scenario where we connect 2 RLC protocol instances without the help of the MAC .
*/
typedef struct {
  union {
    struct rlc_am_rx_pdu_management dummy1;
    struct rlc_tm_rx_pdu_management dummy2;
    //struct rlc_um_rx_pdu_management dummy3;
    struct mac_tb_ind dummy4;
    struct mac_rx_tb_management dummy5;
  } dummy;
} mac_rlc_max_rx_header_size_t;

//-----------------------------------------------------------------------------
//   PRIVATE INTERNALS OF RLC
//-----------------------------------------------------------------------------
#define  RLC_MAX_NUM_INSTANCES_RLC_AM  ((maxDRB * MAX_MOBILES_PER_RG)/2)
#ifdef Rel10
#define  RLC_MAX_NUM_INSTANCES_RLC_UM  ((maxDRB * MAX_MOBILES_PER_RG)/2 + maxSessionPerPMCH * maxServiceCount)
#else
#define  RLC_MAX_NUM_INSTANCES_RLC_UM  ((maxDRB * MAX_MOBILES_PER_RG)/2)
#endif
#define  RLC_MAX_NUM_INSTANCES_RLC_TM  (MAX_MOBILES_PER_RG)

#ifdef Rel10
#define  RLC_MAX_RB  ((maxDRB + 3) * MAX_MOBILES_PER_RG + maxSessionPerPMCH * maxServiceCount)
#else
#define  RLC_MAX_RB  ((maxDRB + 3)* MAX_MOBILES_PER_RG)
#endif

#ifdef Rel10
#define  RLC_MAX_LC  ((max_val_DRB_Identity+1) * MAX_MOBILES_PER_RG + maxSessionPerPMCH * maxServiceCount)
#else
#define  RLC_MAX_LC  ((max_val_DRB_Identity+1)* MAX_MOBILES_PER_RG)
#endif

protected_rlc(void            (*rlc_rrc_data_ind)  (module_id_t , u32_t, u8_t, rb_id_t , sdu_size_t , char* );)
protected_rlc(void            (*rlc_rrc_data_conf) (module_id_t , rb_id_t , mui_t, rlc_tx_status_t );)

/*! \struct  rlc_pointer_t
* \brief Structure helping finding the right RLC protocol instance in a rlc_t structure.
*/
typedef struct rlc_pointer_t {
    rlc_mode_t rlc_type;    /*!< \brief Is RLC protocol instance is AM, UM or TM. */
    int        rlc_index;   /*!< \brief Index of RLC protocol instance in rlc_t/m_rlc_Xm_array[]. */
} rlc_pointer_t;

/*! \struct  rlc_t
* \brief Structure to be instanciated to allocate memory for RLC protocol instances.
*/
typedef struct rlc_t {
    //int                  m_mbms_rlc_pointer[maxSessionPerPMCH][maxServiceCount];                              /*!< \brief Link between (service id, session id) and (implicit) RLC UM protocol instance. */
    signed long int      m_lcid2rbid[RLC_MAX_LC];              /*!< \brief Pairing logical channel identifier with radio bearer identifer. */
    rlc_pointer_t        m_rlc_pointer[RLC_MAX_RB];                        /*!< \brief Link between radio bearer ID and RLC protocol instance. */
    rlc_am_entity_t      m_rlc_am_array[RLC_MAX_NUM_INSTANCES_RLC_AM];     /*!< \brief RLC AM protocol instances. */
    rlc_um_entity_t      m_rlc_um_array[RLC_MAX_NUM_INSTANCES_RLC_UM];     /*!< \brief RLC UM protocol instances. */
    rlc_tm_entity_t      m_rlc_tm_array[RLC_MAX_NUM_INSTANCES_RLC_TM];     /*!< \brief RLC TM protocol instances. */
    char                 m_mscgen_trace[260];
    unsigned char        m_mscgen_trace_length;
}rlc_t;

// RK-LG was protected, public for debug
/*! \var rlc_t rlc[MAX_MODULES]
\brief Global var for RLC layer, allocate memory for RLC protocol instances.
*/
public_rlc(rlc_t rlc[MAX_MODULES];)

/*! \fn tbs_size_t mac_rlc_serialize_tb (char* bufferP, list_t transport_blocksP)
* \brief  Serialize a list of transport blocks coming from RLC in order to be processed by MAC.
* \param[in]  bufferP                 Memory area where the transport blocks have to be serialized.
* \param[in]  transport_blocksP       List of transport blocks.
* \return     The amount of bytes that have been written due to serialization.
*/
private_rlc_mac(tbs_size_t            mac_rlc_serialize_tb   (char*, list_t);)

/*! \fn struct mac_data_ind mac_rlc_deserialize_tb (char* bufferP, tb_size_t tb_sizeP, num_tb_t num_tbP, crc_t *crcsP)
* \brief  Serialize a list of transport blocks coming from RLC in order to be processed by MAC.
* \param[in]  bufferP       Memory area where the transport blocks are serialized.
* \param[in]  tb_sizeP      Size of transport blocks.
* \param[in]  num_tbP       Number of transport blocks.
* \param[in]  crcsP         Array of CRC for each transport block.
* \return     A mac_data_ind structure containing a list of transport blocks.
*/
private_rlc_mac(struct mac_data_ind   mac_rlc_deserialize_tb (char*, tb_size_t, num_tb_t, crc_t *);)


//-----------------------------------------------------------------------------
//   PUBLIC INTERFACE WITH RRC
//-----------------------------------------------------------------------------
#ifdef Rel10
/*! \fn rlc_op_status_t rrc_rlc_config_asn1_req (module_id_t module_idP, u32_t frame, u8_t eNB_flagP,  u8_t UE_index, SRB_ToAddMod_t* srb2addmod, DRB_ToAddModList_t* drb2add_listP, DRB_ToReleaseList_t*  drb2release_listP, MBMS_SessionInfoList_r9_t *SessionInfo_listP)
* \brief  Function for RRC to configure a Radio Bearer.
* \param[in]  module_idP         Virtualized module identifier.
* \param[in]  frame              Frame index.
* \param[in]  eNB_flag           Flag to indicate eNB (1) or UE (0)
* \param[in]  UE_index           Index of UE in eNB RRC
* \param[in]  srb2add_listP      SRB configuration list to be created.
* \param[in]  drb2add_listP      DRB configuration list to be created.
* \param[in]  drb2release_listP  DRB configuration list to be released.
* \param[in]  pmch_info_listP    eMBMS pmch info list to be created.
* \return     A status about the processing, OK or error code.
*/
public_rlc_rrc( rlc_op_status_t rrc_rlc_config_asn1_req (module_id_t, u32_t, u8_t, u8_t UE_index, SRB_ToAddModList_t*, DRB_ToAddModList_t*, DRB_ToReleaseList_t*, PMCH_InfoList_r9_t *pmch_info_listP);)
#else
/*! \fn rlc_op_status_t rrc_rlc_config_asn1_req (module_id_t module_idP, u32_t frame, u8_t eNB_flagP,  u8_t UE_index, SRB_ToAddModList_t* srb2add_listP, DRB_ToAddModList_t* drb2add_listP, DRB_ToReleaseList_t*  drb2release_listP, MBMS_SessionInfoList_r9_t *SessionInfo_listP)
* \brief  Function for RRC to configure a Radio Bearer.
* \param[in]  module_idP         Virtualized module identifier.
* \param[in]  frame              Frame index.
* \param[in]  eNB_flag           Flag to indicate eNB (1) or UE (0)
* \param[in]  UE_index           Index of UE in eNB RRC
* \param[in]  srb2add_listP      SRB configuration list to be created.
* \param[in]  drb2add_listP      DRB configuration list to be created.
* \param[in]  drb2release_listP  DRB configuration list to be released.
* \return     A status about the processing, OK or error code.
*/
public_rlc_rrc( rlc_op_status_t rrc_rlc_config_asn1_req (module_id_t, u32_t, u8_t, u8_t UE_index, SRB_ToAddModList_t*, DRB_ToAddModList_t*, DRB_ToReleaseList_t*);)
#endif

/*! \fn rlc_op_status_t rrc_rlc_remove_rlc   (module_id_t module_idP, u32_t frame, rb_id_t rb_idP)
* \brief  Remove a RLC protocol instance from a radio bearer.
* \param[in]  module_idP       Virtualized module identifier.
* \param[in]  frame            Frame index.
* \param[in]  rb_idP           Radio bearer identifier.
* \return     A status about the processing, OK or error code.
*/
private_rlc_rrc(rlc_op_status_t rrc_rlc_remove_rlc   (module_id_t, u32_t, rb_id_t);)

/*! \fn rlc_op_status_t rrc_rlc_add_rlc   (module_id_t module_idP, u32_t frameP, rb_id_t rb_idP, chan_id_t chan_idP, rlc_mode_t rlc_modeP)
* \brief  Add a RLC protocol instance to a radio bearer.
* \param[in]  module_idP       Virtualized module identifier.
* \param[in]  frameP           Frame index.
* \param[in]  rb_idP           Radio bearer identifier.
* \param[in]  chan_idP         Logical channel identifier.
* \param[in]  rlc_modeP        Mode of RLC (AM, UM, TM).
* \return     A status about the processing, OK or error code.
*/
private_rlc_rrc(rlc_op_status_t rrc_rlc_add_rlc      (module_id_t, u32_t, rb_id_t, chan_id_t, rlc_mode_t);)

/*! \fn rlc_op_status_t rrc_rlc_config_req (module_id_t module_idP, u32_t frame, u8_t eNB_flagP, config_action_t actionP, rb_id_t rb_idP, rb_type_t rb_typeP, rlc_info_t rlc_infoP)
* \brief  Function for RRC to configure a Radio Bearer.
* \param[in]  module_idP       Virtualized module identifier.
* \param[in]  frame            Frame index.
* \param[in]  eNB_flag         Flag to indicate eNB (1) or UE (0)
* \param[in]  actionP          Action for this radio bearer (add, modify, remove).
* \param[in]  rb_idP           Radio bearer identifier.
* \param[in]  rb_typeP         Type of radio bearer (signalling, data).
* \param[in]  rlc_infoP        RLC configuration parameters issued from Radio Resource Manager.
* \return     A status about the processing, OK or error code.
*/
public_rlc_rrc( rlc_op_status_t rrc_rlc_config_req   (module_id_t, u32_t, u8_t , config_action_t, rb_id_t, rb_type_t, rlc_info_t );)

/*! \fn rlc_op_status_t rrc_rlc_data_req     (module_id_t module_idP, u32_t frame, u8_t eNB_flagP, rb_id_t rb_idP, mui_t muiP, confirm_t confirmP, sdu_size_t sdu_sizeP, char* sduP)
* \brief  Function for RRC to send a SDU through a Signalling Radio Bearer.
* \param[in]  module_idP       Virtualized module identifier.
* \param[in]  frame            Frame index
* \param[in]  eNB_flag         Flag to indicate eNB (1) or UE (0)
* \param[in]  MBMS_flag        Flag to indicate whether this is an MBMS service (1) or not (0)
* \param[in]  rb_idP           Radio bearer identifier.
* \param[in]  muiP             Message Unit identifier.
* \param[in]  confirmP         Boolean, is confirmation requested.
* \param[in]  sdu_sizeP        Size of SDU in bytes.
* \param[in]  sduP             SDU.
* \return     A status about the processing, OK or error code.
*/
public_rlc_rrc( rlc_op_status_t rrc_rlc_data_req     (module_id_t, u32_t, u8_t, u8_t, rb_id_t, mui_t, confirm_t, sdu_size_t, char *);)

/*! \fn void  rrc_rlc_register_rrc ( void (*rrc_data_indP)  (module_id_t module_idP, u32_t frame, u8_t eNB_flag, rb_id_t rb_idP, sdu_size_t sdu_sizeP, char* sduP), void (*rrc_data_confP) (module_id_t module_idP, rb_id_t rb_idP, mui_t muiP, rlc_tx_status_t statusP)
* \brief  This function is called by RRC to register its DATA-INDICATE and DATA-CONFIRM handlers to RLC layer.
* \param[in]  rrc_data_indP       Pointer on RRC data indicate function.
* \param[in]  rrc_data_confP      Pointer on RRC data confirm callback function.
*/
public_rlc_rrc( void   rrc_rlc_register_rrc ( void (*rrc_data_indP)  (module_id_t , u32_t, u8_t, rb_id_t , sdu_size_t , char*),
                void (*rrc_data_conf) (module_id_t , rb_id_t , mui_t, rlc_tx_status_t) );)

//-----------------------------------------------------------------------------
//   PUBLIC INTERFACE WITH MAC
//-----------------------------------------------------------------------------
/*! \fn tbs_size_t mac_rlc_data_req     (module_id_t module_idP, u32_t frame, u8_t MBMS_flagP, chan_id_t rb_idP, char* bufferP)
* \brief    Interface with MAC layer, map data request to the RLC corresponding to the radio bearer.
* \param [in]     module_idP       Virtualized module identifier.
* \param [in]     frame            Frame index
* \param [in]     MBMS_flagP       Flag to indicate whether this is the MBMS service (1) or not (0)
* \param [in]     rb_idP           Radio bearer identifier.
* \param [in,out] bufferP          Memory area to fill with the bytes requested by MAC.
* \return     A status about the processing, OK or error code.
*/
public_rlc_mac(tbs_size_t            mac_rlc_data_req     (module_id_t, u32_t, u8_t, chan_id_t, char*);)

/*! \fn void mac_rlc_data_ind     (module_id_t module_idP, u32_t frame, u8_t eNB_flagP, u8_t MBMS_flagP, chan_id_t rb_idP, u32_t frame, char* bufferP, tb_size_t tb_sizeP, num_tb_t num_tbP, crc_t *crcs)
* \brief    Interface with MAC layer, deserialize the transport blocks sent by MAC, then map data indication to the RLC instance corresponding to the radio bearer identifier.
* \param[in]  module_idP       Virtualized module identifier.
* \param[in]  frame            Frame index
* \param[in]  eNB_flagP        Flag to indicate eNB (1) or UE (0)
* \param[in]  MBMS_flagP       Flag to indicate whether this is the MBMS service (1) or not (0)
* \param[in]  rb_idP           Radio bearer identifier.
* \param[in]  frame            Frame index.
* \param[in]  bufferP          Memory area containing the transport blocks sent by MAC.
* \param[in]  tb_sizeP         Size of a transport block in bits.
* \param[in]  num_tbP          Number of transport blocks.
* \param[in]  crcs             Array of CRC decoding.
*/
public_rlc_mac(void                  mac_rlc_data_ind     (module_id_t, u32_t, u8_t, u8_t, chan_id_t, char*, tb_size_t, num_tb_t, crc_t* );)

/*! \fn mac_rlc_status_resp_t mac_rlc_status_ind     (module_id_t module_idP, u32_t frame, u8_t eNB_flag, u8_t MBMS_flagP, chan_id_t rb_idP, tb_size_t tb_sizeP)
* \brief    Interface with MAC layer, request and set the number of bytes scheduled for transmission by the RLC instance corresponding to the radio bearer identifier.
* \param[in]  module_idP       Virtualized module identifier.
* \param[in]  frame            Frame index.
* \param[in]  eNB_flag         Flag to indicate eNB operation (1 true, 0 false)
* \param[in]  MBMS_flagP       Flag to indicate whether this is the MBMS service (1) or not (0)
* \param[in]  rb_idP           Radio bearer identifier.
* \param[in]  tb_sizeP         Size of a transport block set in bytes.
* \return     The maximum number of bytes that the RLC instance can send in the next transmission sequence.
*/
public_rlc_mac(mac_rlc_status_resp_t mac_rlc_status_ind   (module_id_t, u32_t, u8_t, u8_t, chan_id_t, tb_size_t );)

//-----------------------------------------------------------------------------
//   PUBLIC RLC CONSTANTS
//-----------------------------------------------------------------------------
/** RLC null type identifier. */
#define  RLC_NONE  0
/** RLC AM type identifier. */
#define  RLC_AM    1
/** RLC UM type identifier. */
#define  RLC_UM    2
/** RLC TM type identifier. */
#define  RLC_TM    4

#define  RLC_MBMS_NO   0
#define  RLC_MBMS_YES  1

#define RLC_CHANNEL_ID_DUMMY -1
//-----------------------------------------------------------------------------
//   RLC methods
//-----------------------------------------------------------------------------
/*
 * Prints incoming byte stream in hexadecimal and readable form
 *
 * @param componentP Component identifier, see macros defined in UTIL/LOG/log.h
 * @param dataP      Pointer to data buffer to be displayed
 * @param sizeP      Number of octets in data buffer
 */
public_rlc(void rlc_util_print_hex_octets(comp_name_t componentP, unsigned char* dataP, unsigned long sizeP);)



/*! \fn rlc_op_status_t rlc_data_req(module_id_t module_idP, u32_t frame, u8_t eNB_flagP, u8_t MBMS_flagP, rb_id_t rb_idP, mui_t muiP, confirm_t confirmP, sdu_size_t sdu_sizeP, mem_block_t *sduP)
* \brief    Interface with higher layers, map request to the RLC corresponding to the radio bearer.
* \param[in]  module_idP       Virtualized module identifier.
* \param[in]  frame            Frame index.
* \param[in]  eNB_flagP        Flag to indicate eNB (1) or UE (0)
* \param[in]  MBMS_flagP       Flag to indicate whether this is the MBMS service (1) or not (0)
* \param[in]  rb_idP           Radio bearer identifier.
* \param[in]  muiP             Message Unit identifier.
* \param[in]  confirmP         Boolean, is confirmation requested.
* \param[in]  sdu_sizeP        Size of SDU in bytes.
* \param[in]  sduP             SDU.
* \return     A status about the processing, OK or error code.
*/
public_rlc(rlc_op_status_t rlc_data_req     (module_id_t, u32_t, u8_t, u8_t,rb_id_t, mui_t, confirm_t, sdu_size_t, mem_block_t*);)

/*! \fn void rlc_data_ind (module_id_t module_idP, u32_t frame, u8_t eNB_flag, u8_t MBMS_flagP, rb_id_t rb_idP, sdu_size_t sdu_sizeP, mem_block_t* sduP, boolean_t is_data_planeP)
* \brief    Interface with higher layers, route SDUs coming from RLC protocol instances to upper layer instance.
* \param[in]  module_idP       Virtualized module identifier.
* \param[in]  frame            Frame index
* \param[in]  eNB_flagP        Flag to indicate eNB (1) or UE (0)
* \param[in]  MBMS_flagP       Flag to indicate whether this is the MBMS service (1) or not (0)
* \param[in]  rb_idP           Radio bearer identifier.
* \param[in]  sdu_sizeP        Size of SDU in bytes.
* \param[in]  sduP             SDU.
* \param[in]  is_data_planeP   Boolean, is data radio bearer or not.
*/
public_rlc(void            rlc_data_ind     (module_id_t, u32_t frame, u8_t eNB_flag, u8_t MBMS_flagP, rb_id_t, sdu_size_t, mem_block_t*, boolean_t);)


/*! \fn void rlc_data_conf     (module_id_t module_idP, u32_t frameP, u8_t eNB_flagP, rb_id_t rb_idP, mui_t muiP, rlc_tx_status_t statusP, boolean_t is_data_planeP)
* \brief    Interface with higher layers, confirm to upper layer the transmission status for a SDU stamped with a MUI, scheduled for transmission.
* \param[in]  module_idP       Virtualized module identifier.
* \param[in]  frameP           Frame index
* \param[in]  eNB_flagP        Flag to indicate eNB (1) or UE (0)
* \param[in]  rb_idP           Radio bearer identifier.
* \param[in]  muiP             Message Unit identifier.
* \param[in]  statusP          Status of the transmission (RLC_SDU_CONFIRM_YES, RLC_SDU_CONFIRM_NO).
* \param[in]  is_data_planeP   Boolean, is data radio bearer or not.
*/
public_rlc(void            rlc_data_conf    (module_id_t, u32_t, u8_t , rb_id_t, mui_t, rlc_tx_status_t, boolean_t );)


/*! \fn rlc_op_status_t rlc_stat_req     (module_id_t module_idP,
                              u32_t         frame,
                              rb_id_t       rb_idP,
                        unsigned int* stat_tx_pdcp_sdu,
                        unsigned int* stat_tx_pdcp_bytes,
                        unsigned int* stat_tx_pdcp_sdu_discarded,
                        unsigned int* stat_tx_pdcp_bytes_discarded,
                        unsigned int* stat_tx_data_pdu,
                        unsigned int* stat_tx_data_bytes,
                        unsigned int* stat_tx_retransmit_pdu_by_status,
                        unsigned int* stat_tx_retransmit_bytes_by_status,
                        unsigned int* stat_tx_retransmit_pdu,
                        unsigned int* stat_tx_retransmit_bytes,
                        unsigned int* stat_tx_control_pdu,
                        unsigned int* stat_tx_control_bytes,
                        unsigned int* stat_rx_pdcp_sdu,
                        unsigned int* stat_rx_pdcp_bytes,
                        unsigned int* stat_rx_data_pdus_duplicate,
                        unsigned int* stat_rx_data_bytes_duplicate,
                        unsigned int* stat_rx_data_pdu,
                        unsigned int* stat_rx_data_bytes,
                        unsigned int* stat_rx_data_pdu_dropped,
                        unsigned int* stat_rx_data_bytes_dropped,
                        unsigned int* stat_rx_data_pdu_out_of_window,
                        unsigned int* stat_rx_data_bytes_out_of_window,
                        unsigned int* stat_rx_control_pdu,
                        unsigned int* stat_rx_control_bytes,
                        unsigned int* stat_timer_reordering_timed_out,
                        unsigned int* stat_timer_poll_retransmit_timed_out,
                        unsigned int* stat_timer_status_prohibit_timed_out)

* \brief    Request RLC statistics of a particular radio bearer.
* \param[in]  module_idP                   .
* \param[in]  frame
* \param[in]  rb_idP                       .
* \param[out] stat_tx_pdcp_sdu                     Number of SDUs coming from upper layers.
* \param[out] stat_tx_pdcp_bytes                   Number of bytes coming from upper layers.
* \param[out] stat_tx_pdcp_sdu_discarded           Number of discarded SDUs coming from upper layers.
* \param[out] stat_tx_pdcp_bytes_discarded         Number of discarded bytes coming from upper layers.
* \param[out] stat_tx_data_pdu                     Number of transmitted data PDUs to lower layers.
* \param[out] stat_tx_data_bytes                   Number of transmitted data bytes to lower layers.
* \param[out] stat_tx_retransmit_pdu_by_status     Number of re-transmitted data PDUs due to status reception.
* \param[out] stat_tx_retransmit_bytes_by_status   Number of re-transmitted data bytes due to status reception.
* \param[out] stat_tx_retransmit_pdu               Number of re-transmitted data PDUs to lower layers.
* \param[out] stat_tx_retransmit_bytes             Number of re-transmitted data bytes to lower layers.
* \param[out] stat_tx_control_pdu                  Number of transmitted control PDUs to lower layers.
* \param[out] stat_tx_control_bytes                Number of transmitted control bytes to lower layers.
* \param[out] stat_rx_pdcp_sdu                     Number of SDUs delivered to upper layers.
* \param[out] stat_rx_pdcp_bytes                   Number of bytes delivered to upper layers.
* \param[out] stat_rx_data_pdus_duplicate          Number of duplicate PDUs received.
* \param[out] stat_rx_data_bytes_duplicate         Number of duplicate bytes received.
* \param[out] stat_rx_data_pdu                     Number of received PDUs from lower layers.
* \param[out] stat_rx_data_bytes                   Number of received bytes from lower layers.
* \param[out] stat_rx_data_pdu_dropped             Number of received PDUs from lower layers, then dropped.
* \param[out] stat_rx_data_bytes_dropped           Number of received bytes from lower layers, then dropped.
* \param[out] stat_rx_data_pdu_out_of_window       Number of data PDUs received out of the receive window.
* \param[out] stat_rx_data_bytes_out_of_window     Number of data bytes received out of the receive window.
* \param[out] stat_rx_control_pdu                  Number of control PDUs received.
* \param[out] stat_rx_control_bytes                Number of control bytes received.
* \param[out] stat_timer_reordering_timed_out      Number of times the timer "reordering" has timed-out.
* \param[out] stat_timer_poll_retransmit_timed_out Number of times the timer "poll_retransmit" has timed-out.
* \param[out] stat_timer_status_prohibit_timed_out Number of times the timer "status_prohibit" has timed-out.
*/

public_rlc(rlc_op_status_t rlc_stat_req     (module_id_t   module_idP,
        u32_t frame,
        rb_id_t        rb_idP,
        unsigned int* stat_tx_pdcp_sdu,
        unsigned int* stat_tx_pdcp_bytes,
        unsigned int* stat_tx_pdcp_sdu_discarded,
        unsigned int* stat_tx_pdcp_bytes_discarded,
        unsigned int* stat_tx_data_pdu,
        unsigned int* stat_tx_data_bytes,
        unsigned int* stat_tx_retransmit_pdu_by_status,
        unsigned int* stat_tx_retransmit_bytes_by_status,
        unsigned int* stat_tx_retransmit_pdu,
        unsigned int* stat_tx_retransmit_bytes,
        unsigned int* stat_tx_control_pdu,
        unsigned int* stat_tx_control_bytes,
        unsigned int* stat_rx_pdcp_sdu,
        unsigned int* stat_rx_pdcp_bytes,
        unsigned int* stat_rx_data_pdus_duplicate,
        unsigned int* stat_rx_data_bytes_duplicate,
        unsigned int* stat_rx_data_pdu,
        unsigned int* stat_rx_data_bytes,
        unsigned int* stat_rx_data_pdu_dropped,
        unsigned int* stat_rx_data_bytes_dropped,
        unsigned int* stat_rx_data_pdu_out_of_window,
        unsigned int* stat_rx_data_bytes_out_of_window,
        unsigned int* stat_rx_control_pdu,
        unsigned int* stat_rx_control_bytes,
        unsigned int* stat_timer_reordering_timed_out,
        unsigned int* stat_timer_poll_retransmit_timed_out,
        unsigned int* stat_timer_status_prohibit_timed_out);)

/*! \fn int rlc_module_init(void)
* \brief    RAZ the memory of the RLC layer, initialize the memory pool manager (mem_block_t structures mainly used in RLC module).
*/
public_rlc(int rlc_module_init(void);)

/** @} */
#ifndef USER_MODE
#define assert(x) ((x)?msg("rlc assertion fails\n"):0)
#endif


#define RLC_FG_COLOR_BLACK            "\e[0;30m"
#define RLC_FG_COLOR_RED              "\e[0;31m"
#define RLC_FG_COLOR_GREEN            "\e[0;32m"
#define RLC_FG_COLOR_ORANGE           "\e[0;33m"
#define RLC_FG_COLOR_BLUE             "\e[0;34m"
#define RLC_FG_COLOR_MAGENTA          "\e[0;35m"
#define RLC_FG_COLOR_CYAN             "\e[0;36m"
#define RLC_FG_COLOR_GRAY_BLACK       "\e[0;37m"
#define RLC_FG_COLOR_DEFAULT          "\e[0;39m"
#define RLC_FG_BRIGHT_COLOR_DARK_GRAY "\e[1;30m"
#define RLC_FG_BRIGHT_COLOR_RED       "\e[1;31m"
#define RLC_FG_BRIGHT_COLOR_GREEN     "\e[1;32m"
#define RLC_FG_BRIGHT_COLOR_YELLOW    "\e[1;33m"
#define RLC_FG_BRIGHT_COLOR_BLUE      "\e[1;34m"
#define RLC_FG_BRIGHT_COLOR_MAGENTA   "\e[1;35m"
#define RLC_FG_BRIGHT_COLOR_CYAN      "\e[1;36m"
#define RLC_FG_BRIGHT_COLOR_WHITE     "\e[1;37m"
#define RLC_FG_BRIGHT_COLOR_DEFAULT   "\e[0;39m"
#define RLC_REVERSE_VIDEO             "\e[7m"
#define RLC_NORMAL_VIDEO              "\e[27m"

#endif
