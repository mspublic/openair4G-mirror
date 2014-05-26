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
/*! \file rlc_tm_init.h
* \brief This file defines the prototypes of the functions dealing with the control primitives and initialization.
* \author GAUTHIER Lionel
* \date 2010-2011
* \version
* \note
* \bug
* \warning
*/
/** @defgroup _rlc_tm_init_impl_ RLC TM Init Reference Implementation
* @ingroup _rlc_tm_impl_
* @{
*/
#    ifndef __RLC_TM_INIT_H__
#        define __RLC_TM_INIT_H__
//-----------------------------------------------------------------------------
#        include "rlc_tm_entity.h"
#        include "mem_block.h"
//#        include "rrm_config_structs.h"
#        include "rlc_tm_structs.h"
//#        include "rlc.h"
#        include "platform_types.h"
//-----------------------------------------------------------------------------
#        ifdef RLC_TM_INIT_C
#            define private_rlc_tm_init(x)    x
#            define protected_rlc_tm_init(x)  x
#            define public_rlc_tm_init(x)     x
#        else
#            ifdef RLC_TM_MODULE
#                define private_rlc_tm_init(x)
#                define protected_rlc_tm_init(x)  extern x
#                define public_rlc_tm_init(x)     extern x
#            else
#                define private_rlc_tm_init(x)
#                define protected_rlc_tm_init(x)
#                define public_rlc_tm_init(x)     extern x
#            endif
#        endif

typedef volatile struct rlc_tm_info_s{
    uint8_t             is_uplink_downlink;
 } rlc_tm_info_t;


/*! \fn void config_req_rlc_tm (void config_req_rlc_tm (
    const module_id_t enb_module_idP,
    const module_id_t ue_module_idP,
    const frame_t     frameP,
    const eNB_flag_t  eNB_flagP,
    const srb_flag_t  srb_flagP,
    const rlc_tm_info_t * const config_tmP,
    const rb_id_t     rb_idP,
    const srb_flag_t  srb_flagP)
* \brief    Allocate memory for RLC TM instance, reset protocol variables, and set protocol parameters.
* \param[in]  frameP                     Frame index.
* \param[in]  eNB_flag                  Flag to indicate eNB (1) or UE (0)
* \param[in]  srb_flagP                 Flag to indicate SRB (1) or DRB (0)
* \param[in]  enb_module_idP            eNB Virtualized module identifier.
* \param[in]  ue_module_idP             UE Virtualized module identifier.
* \param[in]  config_tmP                Configuration parameters for RLC TM instance.
* \param[in]  rb_idP                    Radio bearer identifier.
* \param[in]  srb_flagP                 Flag to indicate signalling radio bearer (1) or data radio bearer (0).
*/
public_rlc_tm_init(   void config_req_rlc_tm (
    const module_id_t enb_module_idP,
    const module_id_t ue_module_idP,
    const frame_t     frameP,
    const eNB_flag_t  eNB_flagP,
    const srb_flag_t  srb_flagP,
    const rlc_tm_info_t * const config_tmP,
    const rb_id_t     rb_idP);)

/*! \fn void rlc_tm_init (rlc_tm_entity_t *rlcP)
* \brief    Initialize a RLC TM protocol instance, initialize all variables, lists, allocate buffers for making this instance ready to be configured with protocol configuration parameters. After this initialization the RLC TM protocol instance will be in RLC_NULL_STATE state.
* \param[in]  rlcP                      RLC TM protocol instance pointer.
*/
protected_rlc_tm_init(void rlc_tm_init (rlc_tm_entity_t * const rlcP);)

/*! \fn void rlc_tm_reset_state_variables (rlc_tm_entity_t *rlcP)
* \brief    Reset protocol variables and state variables to initial values.
* \param[in]  rlcP                      RLC TM protocol instance pointer.
*/
protected_rlc_tm_init(void rlc_tm_reset_state_variables (rlc_tm_entity_t * const rlcP);)

/*! \fn void rlc_tm_cleanup(rlc_tm_entity_t *rlcP)
* \brief    Free all allocated memory (lists and buffers) previously allocated by this RLC TM instance.
* \param[in]  rlcP                      RLC TM protocol instance pointer.
*/
public_rlc_tm_init(   void rlc_tm_cleanup(rlc_tm_entity_t * const rlcP);)

/*! \fn void rlc_tm_configure(rlc_tm_entity_t * const rlcP, const boolean_t is_uplink_downlinkP)
* \brief    Configure RLC TM protocol parameters.
* \param[in]  rlcP                      RLC TM protocol instance pointer.
* \param[in]  is_uplink_downlinkP       Is this instance is TRANSMITTER_ONLY,
RECEIVER_ONLY, or TRANSMITTER_AND_RECEIVER.
*/
protected_rlc_tm_init(void rlc_tm_configure(rlc_tm_entity_t * const rlcP, const boolean_t is_uplink_downlinkP);)

/*! \fn void rlc_tm_set_debug_infos(rlc_tm_entity_t *rlcP, const frame_t frameP, uint8_t eNB_flagP,  const module_id_t enb_module_idP, const module_id_t ue_module_idP, const rb_id_t rb_idP, const srb_flag_t srb_flagP)
* \brief    Set debug informations for a RLC TM protocol instance, these informations are only for trace purpose.
* \param[in]  rlcP                      RLC TM protocol instance pointer.
* \param[in]  enb_module_idP            eNB Virtualized module identifier.
* \param[in]  ue_module_idP             UE Virtualized module identifier.
* \param[in]  frameP                     Frame index.
* \param[in]  eNB_flag                  Flag to indicate eNB (1) or UE (0)
* \param[in]  srb_flagP                 Flag to indicate signalling radio bearer (1) or data radio bearer (0).
* \param[in]  rb_idP                    Radio bearer identifier.
*/
protected_rlc_tm_init(void rlc_tm_set_debug_infos(
    rlc_tm_entity_t * const rlcP,
    const module_id_t enb_module_idP,
    const module_id_t ue_module_idP,
    const frame_t frameP,
    const eNB_flag_t eNB_flagP,
    const srb_flag_t srb_flagP,
    const rb_id_t rb_idP);)
/** @} */
#    endif
