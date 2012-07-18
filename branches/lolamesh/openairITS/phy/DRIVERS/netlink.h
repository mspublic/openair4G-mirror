/** netlink.h

    Header file for netlink interface.
	Used for both kernel driver and userspace application

   Author: Matthias Ihmig <matthias.ihmig@mytum.de>, 2011
 */

#ifndef __NETLINK_H
#define __NETLINK_H

#include "../pcie_defs.h"

// 17 = used for NETLINK_DM Events
#define NETLINK_EXMIMO  17
#define MYNETLINK_GROUP 1

#define MAX_NL_PAYLOAD    ( PCIE_MAXPAYLOADSIZE +32)
#define MAX_NL_PACKET     ( NLMSG_SPACE(MAX_NL_PAYLOAD) + NLMSG_HDRLEN)

#define NETLINK_BLOCKING 1
#define NETLINK_NONBLOCKING 0

typedef struct {
	unsigned int command;
	unsigned int len;
	char *data;
} netlink_packet_t;


#ifdef __KERNEL__

void netlink_send( char *msg, unsigned int len);
int  netlink_init( void (*netlink_rx_handler)(char *, int) );
void netlink_release(void);

#else

int  netlink_init();	          // returns socket filedescriptor fd
int  netlink_send(int fd, unsigned int payloadcommand, unsigned int payloadlen, char *payload);          // send message through netlink interface, returns no. of bytes sent or -1 on error
int  netlink_recv(int fd, netlink_packet_t *rxpkt, char isblocking);  // receives netlink message (blocking or non-blocking), returns payload length
void netlink_close(int fd); 

#endif

#endif //__NETLINK_H
