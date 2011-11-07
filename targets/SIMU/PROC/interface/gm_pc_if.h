
#ifndef __GM_PC_IF_H__
#    define __GM_PC_IF_H__

#    include "definitions.h"
#    include "message_modules.h"
#    include "openair_io_sync.h"


#ifdef __cplusplus
extern "C" {
#endif

extern void gmInitMsgHandler (Soc_t* thisP);
extern obj_ref_t gmGetObjRef (Soc_t* thisP);
extern void gmHandleMessage(const obj_ref_t* to_refP, const obj_ref_t* from_refP, const msg_t2 msgP);
extern void gmMsgNotHandled(Soc_t* this, const obj_ref_t* to_refP, const obj_ref_t* from_refP, const msg_t2 msgP);
extern void Exec_Msg_Rcv(Soc_t* this, const obj_ref_t* to_refP, const obj_ref_t* from_refP, const msg_t2 msgP);
extern void EResponse_Rcv(Soc_t* this, const obj_ref_t* to_refP, const obj_ref_t* from_refP, const msg_t2 msgP);

  
#ifdef __cplusplus
    }
#endif

#endif
