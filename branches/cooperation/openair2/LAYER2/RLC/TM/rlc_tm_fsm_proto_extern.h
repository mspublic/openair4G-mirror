/***************************************************************************
                          rlc_tm_fsm_proto_extern.h  -
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr
 ***************************************************************************/
#    ifndef __rlc_tm_FSM_PROTO_EXTERN_H__
#        define __rlc_tm_FSM_PROTO_EXTERN_H__
//-----------------------------------------------------------------------------
#        include "platform_types.h"
#        include "rlc_tm_entity.h"
//-----------------------------------------------------------------------------
extern int      rlc_tm_fsm_notify_event (struct rlc_tm_entity *rlcP, u8_t eventP);
#    endif
