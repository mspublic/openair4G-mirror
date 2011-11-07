
#ifndef __DEFINITIONS_H__
#    define __DEFINITIONS_H__
#include <netinet/in.h>
#include <sys/un.h>
#    include "message_modules.h"
#    include "message_transport.h"

#include "openair_io_sync.h"

#ifdef __cplusplus
extern "C" {
#endif


  typedef struct {
    obj_ref_t  m_ref;
    obj_ref_t  m_cmm;
    void (* m_msg_handler[NB_MSG]) (void* thisP, const obj_ref_t* from_refP, const obj_ref_t* to_refP, const msg_t2 sduP);
    openair_io_sync_t  m_io_sync;
    int              m_inet6_if1;
    int              m_inet_if2;
    int              m_inet_if3;
    int port; // which port , each instance should listen to
  } Soc_t;


  typedef struct{

    obj_ref_t *ptr;

  }OBJ_POINTER;

  OBJ_POINTER obj_inst[1];


#ifdef __cplusplus
}
#endif

#endif


 
