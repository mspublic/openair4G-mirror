#    ifndef __RLC_UM_CONTROL_PRIMITIVES_H__
#        define __RLC_UM_CONTROL_PRIMITIVES_H__
//-----------------------------------------------------------------------------
#        include "rlc_um_entity.h"
#        include "mem_block.h"
#        include "rrm_config_structs.h"
#        include "rlc_um_structs.h"
#        include "rlc_um_constants.h"
#        include "rlc.h"
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

public_rlc_um_control_primitives(void config_req_rlc_um (rlc_um_entity_t *rlcP, module_id_t module_idP, rlc_um_info_t * config_umP, u8_t rb_idP, rb_type_t rb_typeP);)
public_rlc_um_control_primitives(void send_rlc_um_control_primitive (rlc_um_entity_t *rlcP, module_id_t module_idP, mem_block_t *cprimitiveP);)
protected_rlc_um_control_primitives(void init_rlc_um (rlc_um_entity_t *rlcP);)
protected_rlc_um_control_primitives(void rlc_um_reset_state_variables (rlc_um_entity_t *rlcP);)
protected_rlc_um_control_primitives(void rlc_um_free_all_resources (rlc_um_entity_t *rlcP);)
protected_rlc_um_control_primitives(void rlc_um_set_configured_parameters (rlc_um_entity_t *rlcP, mem_block_t *cprimitiveP);)
#    endif