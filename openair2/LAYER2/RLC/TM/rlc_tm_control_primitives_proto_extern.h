/***************************************************************************
                          rlc_tm_control_primitives_proto_extern.h  -
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr
 ***************************************************************************/
#    ifndef __RLC_TM_CONTROL_PRIMITIVES_H__
#        define __RLC_TM_CONTROL_PRIMITIVES_H__
//-----------------------------------------------------------------------------
#        include "rlc_tm_entity.h"
#        include "mem_block.h"
#        include "rrm_config_structs.h"
//-----------------------------------------------------------------------------
extern void     config_req_rlc_tm (struct rlc_tm_entity *rlcP, module_id_t module_idP,rlc_tm_info_t * config_tmP, rb_id_t rb_idP, rb_type_t rb_typeP);
extern void     send_rlc_tm_control_primitive (struct rlc_tm_entity *rlcP, module_id_t module_idP, mem_block_t *cprimitiveP);
extern void     init_rlc_tm (struct rlc_tm_entity *rlcP);
extern void     rlc_tm_reset_state_variables (struct rlc_tm_entity *rlcP);
extern void     rlc_tm_free_all_resources (struct rlc_tm_entity *rlcP);
extern void     rlc_tm_set_configured_parameters (struct rlc_tm_entity *rlcP, mem_block_t *cprimitiveP);
#    endif
