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
/*! \file rlc_um_control_primitives.h
* \brief This file defines the prototypes of the functions dealing with the control primitives and initialization.
* \author GAUTHIER Lionel
* \date 2010-2011
* \version
* \note
* \bug
* \warning
*/
/** @defgroup _rlc_um_init_impl_ RLC UM Init Reference Implementation
* @ingroup _rlc_um_impl_
* @{
*/
#    ifndef __RLC_UM_CONTROL_PRIMITIVES_H__
#        define __RLC_UM_CONTROL_PRIMITIVES_H__
//-----------------------------------------------------------------------------
#        include "rlc_um_entity.h"
#        include "mem_block.h"
//#        include "rrm_config_structs.h"
#        include "rlc_um_structs.h"
#        include "rlc_um_constants.h"
//#        include "rlc.h"
#        include "platform_types.h"
//-----------------------------------------------------------------------------
#        ifdef RLC_UM_CONTROL_PRIMITIVES_C
#            define private_rlc_um_control_primitives(x)    x
#            define protected_rlc_um_control_primitives(x)  x
#            define public_rlc_um_control_primitives(x)     x
#        else
#            ifdef RLC_UM_MODULE
#                define private_rlc_um_control_primitives(x)
#                define protected_rlc_um_control_primitives(x)  extern x
#                define public_rlc_um_control_primitives(x)     extern x
#            else
#                define private_rlc_um_control_primitives(x)
#                define protected_rlc_um_control_primitives(x)
#                define public_rlc_um_control_primitives(x)     extern x
#            endif
#        endif

typedef volatile struct {
    u32_t             is_uplink_downlink;
    u32_t             timer_reordering;
    u32_t             sn_field_length; // 5 or 10
    u32_t             is_mXch; // boolean, true if configured for MTCH or MCCH
} rlc_um_info_t;


/*! \fn void config_req_rlc_um (rlc_um_entity_t *rlcP, u32_t frame, module_id_t module_idP, rlc_um_info_t * config_umP, u8_t rb_idP, rb_type_t rb_typeP)
* \brief    Allocate memory for RLC UM instance, reset protocol variables, and set protocol parameters. After this configuration the RLC UM protocol instance will be in RLC_DATA_TRANSFER_READY_STATE state.
* \param[in]  rlcP                      RLC UM protocol instance pointer.
* \param[in]  frame                     Frame index.
* \param[in]  module_idP                Virtualized module identifier.
* \param[in]  config_umP                Configuration parameters for RLC UM instance.
* \param[in]  rb_idP                    Radio bearer identifier.
* \param[in]  rb_typeP                  Radio bearer type (Signalling or Data).
*/
public_rlc_um_control_primitives(   void config_req_rlc_um (rlc_um_entity_t *rlcP, u32_t frame, module_id_t module_idP, rlc_um_info_t * config_umP, u8_t rb_idP, rb_type_t rb_typeP);)

/*! \fn void rlc_um_init (rlc_um_entity_t *rlcP)
* \brief    Initialize a RLC UM protocol instance, initialize all variables, lists, allocate buffers for making this instance ready to be configured with protocol configuration parameters. After this initialization the RLC UM protocol instance will be in RLC_NULL_STATE state.
* \param[in]  rlcP                      RLC UM protocol instance pointer.
*/
protected_rlc_um_control_primitives(void rlc_um_init (rlc_um_entity_t *rlcP);)

/*! \fn void rlc_um_reset_state_variables (rlc_um_entity_t *rlcP)
* \brief    Reset protocol variables and state variables to initial values.
* \param[in]  rlcP                      RLC UM protocol instance pointer.
*/
protected_rlc_um_control_primitives(void rlc_um_reset_state_variables (rlc_um_entity_t *rlcP);)

/*! \fn void rlc_um_cleanup(rlc_um_entity_t *rlcP)
* \brief    Free all allocated memory (lists and buffers) previously allocated by this RLC UM instance.
* \param[in]  rlcP                      RLC UM protocol instance pointer.
*/
public_rlc_um_control_primitives(   void rlc_um_cleanup(rlc_um_entity_t *rlcP);)

/*! \fn void rlc_um_configure(rlc_um_entity_t *rlcP, u32_t frame, u32_t timer_reorderingP, u32_t sn_field_lengthP, u32_t is_mXchP)
* \brief    Configure RLC UM protocol parameters.
* \param[in]  rlcP                      RLC UM protocol instance pointer.
* \param[in]  frame                     Frame index.
* \param[in]  timer_reorderingP         t-Reordering timer initialization value, units in frame.
* \param[in]  sn_field_lengthP          Length of the sequence number, 5 or 10 bits.
* \param[in]  is_mXchP                  Is the radio bearer for MCCH, MTCH.
*/
protected_rlc_um_control_primitives(void rlc_um_configure(rlc_um_entity_t *rlcP, u32_t frame, u32_t timer_reorderingP, u32_t sn_field_lengthP, u32_t is_mXchP);)

/*! \fn void rlc_um_set_debug_infos(rlc_um_entity_t *rlcP, u32_t frame, module_id_t module_idP, rb_id_t rb_idP, rb_type_t rb_typeP)
* \brief    Set debug informations for a RLC UM protocol instance, these informations are only for trace purpose.
* \param[in]  rlcP                      RLC UM protocol instance pointer.
* \param[in]  frame                     Frame index.
* \param[in]  module_idP                Virtualized module identifier.
* \param[in]  rb_idP                    Radio bearer identifier.
* \param[in]  rb_typeP                  Radio bearer type (Signalling or Data).
*/
protected_rlc_um_control_primitives(void rlc_um_set_debug_infos(rlc_um_entity_t *rlcP, u32_t frame, module_id_t module_idP, rb_id_t rb_idP, rb_type_t rb_typeP);)
/** @} */
#    endif
