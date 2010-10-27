#    ifndef __RLC_UM_VERY_SIMPLE_TEST_H__
#        define __RLC_UM_VERY_SIMPLE_TEST_H__
//-----------------------------------------------------------------------------
#        include "rlc_um_entity.h"
#        include "mem_block.h"
#        include "rrm_config_structs.h"
#        include "rlc_um_structs.h"
#        include "rlc_um_constants.h"
//-----------------------------------------------------------------------------
#        ifdef RLC_UM_VERY_SIMPLE_TEST_C
#            define private_rlc_um_very_simple_test(x)    x
#            define protected_rlc_um_very_simple_test(x)  x
#            define public_rlc_um_very_simple_test(x)     x
#        else
#            ifdef RLC_UM_MODULE
#                define private_rlc_um_very_simple_test(x)
#                define protected_rlc_um_very_simple_test(x)  extern x
#                define public_rlc_um_very_simple_test(x)     extern x
#            else
#                define private_rlc_um_very_simple_test(x)
#                define protected_rlc_um_very_simple_test(x)
#                define public_rlc_um_very_simple_test(x)     extern x
#            endif
#        endif
#define RLC_UM_TEST_SDU_TYPE_TCPIP 0
#define RLC_UM_TEST_SDU_TYPE_VOIP  1
#define RLC_UM_TEST_SDU_TYPE_SMALL 2

public_rlc_um_very_simple_test(void rlc_um_test_send_sdu (rlc_um_entity_t* rlcP,  unsigned int sdu_typeP));
#    endif
