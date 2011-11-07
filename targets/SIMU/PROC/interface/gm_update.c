

#define COMPONENT_GM
#define COMPONENT_GM_CORE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <sys/time.h>

#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>

#include "gm_update.h"
#include "gm_pc_if.h"
#include "gm_init_if.h"
#include "definitions.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "openair_inet_sockets.h"
#include "openair_io_sync.h"
#include "message_transport.h"

#include <net/if.h>
#include <linux/rtnetlink.h>


void CreatSocket(obj_ref_t *peer,int fd,const char* addrP, int portP)
{
  const int one = 1;
  
  peer->object_class       = 1;
  peer->ref_type           = 4;
  peer->callback           = openairInetSendTo; 
  peer->mem_ref.pointer    = NULL;
  
  if (( peer->obj_ref.sock_inet_ref.sock_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    printf("socket() failed");
  
  if (setsockopt(peer->obj_ref.sock_inet_ref.sock_fd, SOL_SOCKET, SO_BROADCAST, (void*)&one, sizeof(one)) < 0) {
    perror("setsockopt SO_BROADCAST failed");
    close(peer->obj_ref.sock_inet_ref.sock_fd);
  }

  peer->obj_ref.sock_inet_ref.sockaddr.sin_port         = htons(portP);
  peer->obj_ref.sock_inet_ref.sockaddr.sin_family       = AF_INET;
  peer->obj_ref.sock_inet_ref.sockaddr.sin_addr.s_addr  = inet_addr(addrP);
  //printf("Create Socket, taget IP is:%s.\n",addrP);

}








