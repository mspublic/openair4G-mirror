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

/******************************************************************************
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr
*******************************************************************************/
#ifndef USER_MODE
#    define __NO_VERSION__

#    ifdef RTAI
#        include <rtai.h>
#    else
      /* RTLINUX */
#        include <rtl.h>
#    endif

#    include <asm/page.h>

#else
#    include <stdlib.h>
#    include <stdio.h>
#    include <fcntl.h>
#    include <signal.h>
#    include <sys/types.h>
#    include <sys/stat.h>
#endif
//-----------------------------------------------------------------------------
#include "print.h"
#include "platform.h"
#include "rlc_um_entity.h"
#include "rlc_um_constants.h"
#include "rlc_def.h"
#include "debug_l2.h"
//-----------------------------------------------------------------------------
#ifdef DEBUG_RLC_UM_FSM
#    define   PRINT_RLC_UM_FSM msg
#else
#    define   PRINT_RLC_UM_FSM  //
#endif
//-----------------------------------------------------------------------------
int
rlc_um_fsm_notify_event (struct rlc_um_entity *rlcP, uint8_t eventP)
{
//-----------------------------------------------------------------------------

  switch (rlcP->protocol_state) {
        //-------------------------------
        // RLC_NULL_STATE
        //-------------------------------
      case RLC_NULL_STATE:
        switch (eventP) {
            case RLC_UM_RECEIVE_CRLC_CONFIG_REQ_ENTER_DATA_TRANSFER_READY_STATE_EVENT:
              PRINT_RLC_UM_FSM ("[RLC_UM %p][FSM] RLC_NULL_STATE -> RLC_DATA_TRANSFER_READY_STATE\n", rlcP);
              rlcP->protocol_state = RLC_DATA_TRANSFER_READY_STATE;
              return 1;
              break;

            default:
              msg ("[RLC_UM %p][FSM] WARNING PROTOCOL ERROR - EVENT %02X hex NOT EXPECTED FROM NULL_STATE\n", rlcP, eventP);
              return 0;
        }
        break;
        //-------------------------------
        // RLC_DATA_TRANSFER_READY_STATE
        //-------------------------------
      case RLC_DATA_TRANSFER_READY_STATE:
        switch (eventP) {
            case RLC_UM_RECEIVE_CRLC_CONFIG_REQ_ENTER_NULL_STATE_EVENT:
              PRINT_RLC_UM_FSM ("[RLC_UM %p][FSM] RLC_DATA_TRANSFER_READY_STATE -> RLC_NULL_STATE\n", rlcP);
              rlcP->protocol_state = RLC_NULL_STATE;
              return 1;
              break;

            case RLC_UM_RECEIVE_CRLC_SUSPEND_REQ_EVENT:
            case RLC_UM_TRANSMIT_CRLC_SUSPEND_CNF_EVENT:
              PRINT_RLC_UM_FSM ("[RLC_UM %p][FSM] RLC_DATA_TRANSFER_READY_STATE -> RLC_LOCAL_SUSPEND_STATE\n", rlcP);
              rlcP->protocol_state = RLC_LOCAL_SUSPEND_STATE;
              return 1;
              break;

            default:
              msg ("[RLC_UM %p][FSM] WARNING PROTOCOL ERROR - EVENT %02X hex NOT EXPECTED FROM DATA_TRANSFER_READY_STATE\n", rlcP, eventP);
              return 0;
        }
        break;
        //-------------------------------
        // RLC_LOCAL_SUSPEND_STATE
        //-------------------------------
      case RLC_LOCAL_SUSPEND_STATE:
        switch (eventP) {
            case RLC_UM_RECEIVE_CRLC_CONFIG_REQ_ENTER_NULL_STATE_EVENT:
              PRINT_RLC_UM_FSM ("[RLC_UM %p][FSM] RLC_LOCAL_SUSPEND_STATE -> RLC_NULL_STATE\n", rlcP);
              rlcP->protocol_state = RLC_NULL_STATE;
              return 1;
              break;

            case RLC_UM_RECEIVE_CRLC_RESUME_REQ_EVENT:
              PRINT_RLC_UM_FSM ("[RLC_UM %p][FSM] RLC_LOCAL_SUSPEND_STATE -> RLC_DATA_TRANSFER_READY_STATE\n", rlcP);
              rlcP->protocol_state = RLC_DATA_TRANSFER_READY_STATE;
              return 1;
              break;

            default:
              msg ("[RLC_UM %p][FSM] WARNING PROTOCOL ERROR - EVENT %02X hex NOT EXPECTED FROM RLC_LOCAL_SUSPEND_STATE\n", rlcP, eventP);
              return 0;
        }
        break;

      default:
        msg ("[RLC_UM %p][FSM] ERROR UNKNOWN STATE %d\n", rlcP, rlcP->protocol_state);
        return 0;
  }
}
