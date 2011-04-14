/***************************************************************************
                          rlc_tm_segment_proto_extern.h  -
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr

 ***************************************************************************/
#    ifndef __RLC_TM_SEGMENT_PROTO_EXTERN_H__
#        define __RLC_TM_SEGMENT_PROTO_EXTERN_H__
//-----------------------------------------------------------------------------
#        include "rlc_tm_entity.h"
//-----------------------------------------------------------------------------
extern void     rlc_tm_segment (struct rlc_tm_entity *rlcP);
extern void     rlc_tm_no_segment (struct rlc_tm_entity *rlcP);
#    endif
