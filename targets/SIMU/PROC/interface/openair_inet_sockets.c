
#define OPENAIR_INET_SOCKETS_C
#define OPENAIR_IO
#include "openair_inet_sockets.h"
#include "openair_io_sync.h"
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>   
#include "gm_pc_if.h"

void
openairInetRecvFrom(int sock_fd, obj_ref_t* toP)
{

  //struct sockaddr_in6 from_addr;
  socklen_t fromlen;
  ssize_t packet_size;
  time_t now = time(NULL);
  
  obj_ref_t from;
  
  // for addr comp
  char addr_str[INET_ADDRSTRLEN]; // defined in netinet/in.h
  memset(addr_str, 0, INET_ADDRSTRLEN);
  fromlen = sizeof(struct sockaddr_in);
  packet_size = recvfrom(sock_fd,
			 io_sync_recvbuf,
			 IO_SYNC_RECV_BUFFER_SIZE,
			 0,//NULL,0);
			 (struct sockaddr *)&from.obj_ref.sock_inet_ref.sockaddr,
			 &fromlen);
  
  from.object_class    = -1;
  from.ref_type        = 4;
  from.callback        = openairInetSendTo;
  from.mem_ref.pointer = NULL;
  from.obj_ref.sock_inet_ref.sock_fd        = sock_fd;
  
  
  if (packet_size <= 0) {
    if (packet_size < 0 && errno != EWOULDBLOCK) {
      printf("error recvfrom: %s", strerror(errno));
    }
  }
  else{
    if (((inet_ntop(AF_INET,(char*) &from.obj_ref.sock_inet_ref.sockaddr.sin_addr, addr_str,INET_ADDRSTRLEN)) == 0)) {
      printf("error occured during address translation");
    }
    else{
    //  printf("recvfrom %s packet port %d size %d bytes\n",  addr_str, from.obj_ref.sock_inet_ref.sockaddr.sin_port, packet_size);
      msg_t2 message = openairDeserializeMsg(io_sync_recvbuf, packet_size);
      toP->callback(toP, &from, message);
      
    }
    
  }
}


void
openairInetSendTo(const struct obj_ref *toP, const struct obj_ref *fromP, msg_t2 msgP)
{
  int sent_rc = 0;
  time_t now;
  char addr_str[INET_ADDRSTRLEN]; // defined in netinet/in.h
  memcpy(io_sync_txbuf, &msgP.head, sizeof (msg_head_t));
  memcpy(&io_sync_txbuf[sizeof (msg_head_t)], msgP.data, msgP.head.size);
  
  inet_ntop(AF_INET, &toP->obj_ref.sock_inet_ref.sockaddr.sin_addr, addr_str, INET_ADDRSTRLEN);
  
  sent_rc = sendto(toP->obj_ref.sock_inet_ref.sock_fd,
		   (char*)io_sync_txbuf,
		   msgP.head.size + sizeof (msg_head_t),
		   0, //MSG_DONTWAIT,
		   (struct sockaddr *)&(toP->obj_ref.sock_inet_ref.sockaddr),
		   sizeof(struct sockaddr_in));
  
  if (sent_rc <= 0) {
    if (sent_rc < 0 && errno != EWOULDBLOCK) {
      printf("sendto %d\n", sent_rc);
      //printf(0, LOG_CLASS, LOG_ERR, "sendto: %s with fd %d %d bytes", strerror(errno),toP->obj_ref.sock_inet_ref.sock_fd, msgP.head.size + sizeof (msg_head_t));
    }
  }
  else
    if ((inet_ntop(AF_INET,(char*) &toP->obj_ref.sock_inet_ref.sockaddr.sin_addr, addr_str,INET_ADDRSTRLEN)) == 0) {
      printf("error occured during address translation");
    }

}

int
openairInetCreateSocket(const int typeP, const int protocolP, const char* addrP, const int portP)
{
  int desc;
  int one;
  
  struct sockaddr_in sin;
  char addr_str[INET_ADDRSTRLEN]; // defined in netinet/in.h
  
  if ((desc = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    printf("socket: %s", strerror(errno));
    return -1;
  } else {
  //  printf("create socket: fd %d PF_INET type %d protocol %d\n", desc, typeP, protocolP);
  }
  
  // set the sock option to allow sendinf of broadcast datagrams: fixme later
  if (setsockopt(desc, SOL_SOCKET, SO_BROADCAST, (void*)&one, sizeof(one)) < 0) {
    perror("setsockopt SO_BROADCAST failed");
    close(desc);
  }
  if (setsockopt(desc, SOL_SOCKET, SO_REUSEADDR, (void*)&one, sizeof(one)) < 0) {
    perror("setsockopt SO_REUSEADDR failed");
    close(desc);
  }
  
  memset(&sin, 0, sizeof(struct sockaddr_in));
  // address of binding

#ifdef SIN_LEN
  sin.sin_len = sizeof(sin);
#endif
  sin.sin_family   = AF_INET;
  inet_pton(AF_INET, addrP, &sin);
  sin.sin_port     = htons(portP);
  // binding the socket
  if (bind (desc, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
    printf("bind: %s fd %d to @ %s", strerror(errno), desc, addrP);
    return -1;
    
  } else {
  //  printf("bind: socket fd %d to @ %s\n", desc,addrP);
    
  }
  return desc;
}
