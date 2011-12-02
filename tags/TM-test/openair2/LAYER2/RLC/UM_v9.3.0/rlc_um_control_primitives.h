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

public_rlc_um_control_primitives(   void config_req_rlc_um (rlc_um_entity_t *rlcP, module_id_t module_idP, rlc_um_info_t * config_umP, u8_t rb_idP, rb_type_t rb_typeP);)
protected_rlc_um_control_primitives(void rlc_um_init (rlc_um_entity_t *rlcP);)
protected_rlc_um_control_primitives(void rlc_um_reset_state_variables (rlc_um_entity_t *rlcP);)
public_rlc_um_control_primitives(   void rlc_um_cleanup(rlc_um_entity_t *rlcP);)
protected_rlc_um_control_primitives(void rlc_um_configure(rlc_um_entity_t *rlcP, u32_t timer_reorderingP, u32_t sn_field_lengthP, u32_t is_mXchP);)
protected_rlc_um_control_primitives(void rlc_um_set_debug_infos(rlc_um_entity_t *rlcP, module_id_t module_idP, rb_id_t rb_idP, rb_type_t rb_typeP);)

#    endif
