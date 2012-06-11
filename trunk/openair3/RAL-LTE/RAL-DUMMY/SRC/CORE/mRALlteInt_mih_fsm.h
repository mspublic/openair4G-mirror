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
/*! \file mRALlteInt_mih_fsm.h
 * \brief
 * \author BRIZZOLA Davide, GAUTHIER Lionel, MAUREL Frederic, WETTERWALD Michelle
 * \date 2012
 * \version
 * \note
 * \bug
 * \warning
 */

#ifndef __MRALLTEINT_MIH_FSM_H__
#    define __MRALLTEINT_MIH_FSM_H__
//-----------------------------------------------------------------------------
#        ifdef MRALLTEINT_MIH_FSM_C
#            define private_mRALlteInt_mih_fsm(x)    x
#            define protected_mRALlteInt_mih_fsm(x)  x
#            define public_mRALlteInt_mih_fsm(x)     x
#        else
#            ifdef MRAL_MODULE
#                define private_mRALlteInt_mih_fsm(x)
#                define protected_mRALlteInt_mih_fsm(x)  extern x
#                define public_mRALlteInt_mih_fsm(x)     extern x
#            else
#                define private_mRALlteInt_mih_fsm(x)
#                define protected_mRALlteInt_mih_fsm(x)
#                define public_mRALlteInt_mih_fsm(x)     extern x
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
#include "mRALlteInt_thresholds.h"
#include "mRALlteInt_parameters.h"
#include "mRALlteInt_action.h"
#include "mRALlteInt_mih_msg.h"
//-----------------------------------------------------------------------------
typedef enum  {
                  MIH_FSM_STATE_INIT = 0,
                  MIH_FSM_STATE_REGISTER_SENT,
                  MIH_FSM_STATE_DISCOVERED,
                  MIH_FSM_STATE_SEND_LINK_DETECTED_INDICATION,
                  MIH_FSM_STATE_SEND_LINK_UP_INDICATION,
                  MIH_FSM_STATE_SEND_LINK_PARAMETERS_REPORT_INDICATION,
                  MIH_FSM_STATE_SEND_LINK_GOING_DOWN_INDICATION,
                  MIH_FSM_STATE_SEND_LINK_DOWN_INDICATION
              } MIH_FSM_STATE_ENUM_T;
//-----------------------------------------------------------------------------
protected_mRALlteInt_mih_fsm(MIH_FSM_STATE_ENUM_T         g_mih_fsm_state;)
//-----------------------------------------------------------------------------
public_mRALlteInt_mih_fsm(void mRALlte_mih_fsm(MIH_C_Message_Wrapper_t* mP, int statusP);)

#endif
