#ifndef __RLC_AM_INIT_H__
#    define __RLC_AM_INIT_H__

#    include "UTIL/MEM/mem_block.h"
//-----------------------------------------------------------------------------
#        ifdef RLC_AM_INIT_C
#            define private_rlc_am_init(x)    x
#            define protected_rlc_am_init(x)  x
#            define public_rlc_am_init(x)     x
#        else
#            ifdef RLC_AM_MODULE
#                define private_rlc_am_init(x)
#                define protected_rlc_am_init(x)  extern x
#                define public_rlc_am_init(x)     extern x
#            else
#                define private_rlc_am_init(x)
#                define protected_rlc_am_init(x)
#                define public_rlc_am_init(x)     extern x
#            endif
#        endif
//-----------------------------------------------------------------------------
#include "platform_types.h"
#include "platform_constants.h"
#include "PHY/defs.h"

//-----------------------------------------------------------------------------
public_rlc_am_init( void rlc_am_init   (rlc_am_entity_t* rlcP);)
public_rlc_am_init( void rlc_am_cleanup(rlc_am_entity_t* rlcP);)
public_rlc_am_init( void rlc_am_configure(rlc_am_entity_t *rlcP,
                                          u16_t max_retx_thresholdP,
                                          u16_t poll_pduP,
                                          u16_t poll_byteP,
                                          u32_t t_poll_retransmitP,
                                          u32_t t_reorderingP,
                                          u32_t t_status_prohibitP);)
public_rlc_am_init( void rlc_am_set_debug_infos(rlc_am_entity_t *rlcP, module_id_t module_idP, rb_id_t rb_idP, rb_type_t rb_typeP);)
#endif
