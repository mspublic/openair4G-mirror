

#ifndef __OPENAIR_INET_SOCKETS_H__
#define __OPENAIR_INET_SOCKETS_H__

#include "object_reference.h"
#include <netinet/in.h>


extern void openairInetRecvFrom(int sock_fd, obj_ref_t* fromP);
extern void openairInetSendTo(const struct obj_ref *, const struct obj_ref *, msg_t2);
extern int  openairInetCreateSocket(const int, const int, const char*, const int);

#endif
