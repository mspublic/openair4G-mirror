#    ifndef __RLC_UM_DAR_PROTO_EXTERN_H__
#        define __RLC_UM_DAR_PROTO_EXTERN_H__
//-----------------------------------------------------------------------------
#        include "rlc_um_entity.h"
#        include "rlc_um_structs.h"
#        include "rlc_um_constants.h"
#        include "list.h"
//-----------------------------------------------------------------------------
#        ifdef RLC_UM_DAR_C
#            define private_rlc_um_dar(x)    x
#            define protected_rlc_um_dar(x)  x
#            define public_rlc_um_dar(x)     x
#        else
#            ifdef RLC_UM_MODULE
#                define private_rlc_um_dar(x)
#                define protected_rlc_um_dar(x)  extern x
#                define public_rlc_um_dar(x)     extern x
#            else
#                define private_rlc_um_dar(x)
#                define protected_rlc_um_dar(x)
#                define public_rlc_um_dar(x)     extern x
#            endif
#        endif
private_rlc_um_dar(  int rlc_um_read_length_indicators(unsigned char**dataP, rlc_um_e_li_t* e_liP, unsigned int* li_arrayP, unsigned int *num_liP, unsigned int *data_sizeP));
private_rlc_um_dar(  void rlc_um_try_reassembly      (rlc_um_entity_t *rlcP, signed int snP));
private_rlc_um_dar(  void rlc_um_check_timer_dar_time_out(rlc_um_entity_t *rlcP));
private_rlc_um_dar(  mem_block_t *rlc_um_remove_pdu_from_dar_buffer(rlc_um_entity_t *rlcP, u16_t snP));
private_rlc_um_dar(  inline mem_block_t* rlc_um_get_pdu_from_dar_buffer(rlc_um_entity_t *rlcP, u16_t snP));
protected_rlc_um_dar(inline signed int rlc_um_in_window(rlc_um_entity_t *rlcP, signed int lower_boundP, signed int snP, signed int higher_boundP));
protected_rlc_um_dar(void rlc_um_receive_process_dar (rlc_um_entity_t *rlcP, mem_block_t *pdu_memP,rlc_um_pdu_sn_10_t *pduP, u16_t tb_sizeP));
#    endif
