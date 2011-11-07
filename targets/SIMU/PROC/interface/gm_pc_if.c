#define COMPONENT_GM
#define COMPONENT_GM_PC_IF

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "gm_update.h"
#include "../interface.h"
#include "definitions.h"
#include "gm_pc_if.h"

void gmInitMsgHandler (Soc_t* this)
{

  int  i;
  for (i = 0; i < NB_MSG; i++) {
    switch (i) {
    case Exec_Msg:
           this->m_msg_handler[i] = Exec_Msg_Rcv;
           break;
    case Exec_Msg_Response:
           this->m_msg_handler[i] = EResponse_Rcv;
           break;
    default:
      this->m_msg_handler[i] = gmMsgNotHandled;
    }
  }
}


obj_ref_t gmGetObjRef (Soc_t* this)
{
    return this->m_ref;
}

void gmHandleMessage  (const obj_ref_t* to_refP, const obj_ref_t* from_refP, const msg_t2 msgP)
{
	Soc_t* this = (Soc_t*)(to_refP->mem_ref.pointer);
    assert(this!=NULL);
    //	openair_log(0, TC_CLASS, LOG_INFO, "tcHandleMessage()");
    (*this->m_msg_handler[msgP.head.msg_type]) ((void*)this, to_refP, from_refP, msgP);
    
}
void Exec_Msg_Rcv (Soc_t* this, const obj_ref_t* to_refP, const obj_ref_t* from_refP, const msg_t2 msgP)
{
	  msg_t2              msg;
	  Exec_Msg_t msg_CO;

	  memset(&msg_CO, 0, sizeof(Exec_Msg_t));
	  memcpy(&msg_CO, msgP.data, sizeof(msg_CO));

	  s_t* st2=(s_t*)(Instance[1].gm->mem_ref.pointer);

		st2->frame=msg_CO.frame;
		st2->slot=msg_CO.slot;

}

void EResponse_Rcv (Soc_t* this, const obj_ref_t* to_refP, const obj_ref_t* from_refP, const msg_t2 msgP)
{
	  msg_t2              msg;
	  Exec_Msg_Response_t msg_CO;

	  memset(&msg_CO, 0, sizeof(Exec_Msg_Response_t));
	  memcpy(&msg_CO, msgP.data, sizeof(msg_CO));

}


void gmMsgNotHandled (Soc_t* this, const obj_ref_t* to_refP, const struct obj_ref* from_refP, const msg_t2 sduP)
{
	printf("Message Not handled\n");
  
}
