/*
                                rlc_mpls.c
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr
*/

#define RLC_MPLS_C
#include "rlc.h"


//-----------------------------------------------------------------------------
rlc_op_status_t mpls_rlc_data_req     (module_id_t module_idP, rb_id_t rb_idP, sdu_size_t sdu_sizeP, mem_block_t* sduP) {
//-----------------------------------------------------------------------------
  return rlc_data_req(module_idP, rb_idP, RLC_MUI_UNDEFINED, RLC_SDU_CONFIRM_NO, sdu_sizeP, sduP);
}

