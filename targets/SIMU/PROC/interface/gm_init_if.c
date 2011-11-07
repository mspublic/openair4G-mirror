
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "openair_io_sync.h"
#include "openair_inet_sockets.h"
#include "../interface.h"
#include "definitions.h"
#include "gm_init_if.h"
#include "gm_pc_if.h"

void gmInit (Soc_t* this, unsigned int n)
{
  memset(this, 0, sizeof(Soc_t));
  gmInitMsgHandler(this);
  this->m_ref.object_class             = 5;
  this->m_ref.ref_type                 = 0;
  this->m_ref.callback                 = gmHandleMessage; 
  this->m_ref.mem_ref.pointer          = this;

  openairIoSyncInit(&this->m_io_sync);
  s_t* st2=(s_t*)(Instance[1].gm->mem_ref.pointer);
  this->m_inet_if2 = openairInetCreateSocket(SOCK_DGRAM,IPPROTO_UDP,"172.27.204.62",(st2->port));
  openairAddIoFileDesc(&this->m_io_sync, this->m_inet_if2, this->m_inet_if2, openairInetRecvFrom, this->m_ref);
 // openairIoSyncCreateThread(&this->m_io_sync);
}

obj_ref_t* gmNew (unsigned int n)
{
  Soc_t *soc = calloc(1, sizeof(Soc_t));
  gmInit(soc,n);
  obj_ref_t *this_ref = calloc(1, sizeof(obj_ref_t));
  this_ref->object_class             = 5;
  this_ref->ref_type                 = 0;
  this_ref->callback                 = gmHandleMessage;
  this_ref->mem_ref.pointer          = soc;
  return this_ref;
}

void IntInitAll(void)
{
    obj_inst[0].ptr=gmNew(0);
}


