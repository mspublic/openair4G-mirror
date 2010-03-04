/***************************************************************************
                          rlc_tm_fsm.c  -
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr
 ***************************************************************************/
#include "rtos_header.h"
#include "platform_types.h"
//-----------------------------------------------------------------------------
#include "rlc_tm_entity.h"
#include "rlc_tm_constants.h"
#include "rlc_def.h"
//-----------------------------------------------------------------------------
#ifdef DEBUG_RLC_TM_FSM
#    define   PRINT_RLC_TM_FSM msg
#else
#    define   PRINT_RLC_TM_FSM  //
#endif
//-----------------------------------------------------------------------------
int
rlc_tm_fsm_notify_event (struct rlc_tm_entity *rlcP, u8_t eventP)
{
//-----------------------------------------------------------------------------

  switch (rlcP->protocol_state) {
        //-------------------------------
        // RLC_NULL_STATE
        //-------------------------------
      case RLC_NULL_STATE:
        switch (eventP) {
            case RLC_TM_RECEIVE_CRLC_CONFIG_REQ_ENTER_DATA_TRANSFER_READY_STATE_EVENT:
              PRINT_RLC_TM_FSM ("[RLC_TM %p][FSM] RLC_NULL_STATE -> RLC_DATA_TRANSFER_READY_STATE\n", rlcP);
              rlcP->protocol_state = RLC_DATA_TRANSFER_READY_STATE;
              return 1;
              break;

            default:
              msg ("[RLC_TM %p][FSM] WARNING PROTOCOL ERROR - EVENT %02X hex NOT EXPECTED FROM NULL_STATE\n", rlcP, eventP);
              return 0;
        }
        break;
        //-------------------------------
        // RLC_DATA_TRANSFER_READY_STATE
        //-------------------------------
      case RLC_DATA_TRANSFER_READY_STATE:
        switch (eventP) {
            case RLC_TM_RECEIVE_CRLC_CONFIG_REQ_ENTER_NULL_STATE_EVENT:
              PRINT_RLC_TM_FSM ("[RLC_TM %p][FSM] RLC_DATA_TRANSFER_READY_STATE -> RLC_NULL_STATE\n", rlcP);
              rlcP->protocol_state = RLC_NULL_STATE;
              return 1;
              break;
        }
        break;

      default:
        msg ("[RLC_TM %p][FSM] ERROR UNKNOWN STATE %d\n", rlcP, rlcP->protocol_state);
        return 0;
  }
  return 0;
}
