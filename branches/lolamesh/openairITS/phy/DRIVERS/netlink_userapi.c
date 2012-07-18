/** netlink_userapi.c

    API calls for user space application to receive EXMIMO messages through netlink

   Author: Matthias Ihmig <matthias.ihmig@mytum.de>, 2011
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "netlink.h"

int netlink_init()
{
    int fd, ret;
    struct sockaddr_nl s_nladdr;
    
    if (MAX_NL_PACKET % 4 != 0)
	printf("ERROR: MAX_NL_PACKET MUST BE MULTIPLE OF 4!\n");
    
    fd = socket(AF_NETLINK ,SOCK_RAW , NETLINK_EXMIMO );
    
    if (fd < 0)
	printf("ERROR: socket() failed, returned %i, errno=%i (%s) \n", fd, errno, strerror(errno));
    
    memset(&s_nladdr, 0 ,sizeof(s_nladdr));
    s_nladdr.nl_family= AF_NETLINK ;
    s_nladdr.nl_pad=0;
    s_nladdr.nl_pid = pthread_self() << 16 | getpid();
    s_nladdr.nl_groups = MYNETLINK_GROUP;

    ret = bind(fd, (struct sockaddr*)&s_nladdr, sizeof(s_nladdr));
    
    if (ret < 0)
	printf("ERROR: bind() failed, returned %i, errno=%i (%s)\n", ret, errno, strerror(errno));
    
    return (ret<0? -1 : fd);
}

int netlink_send(int fd, unsigned int payloadcommand, unsigned int payloadlen, char *payload)
{
    struct sockaddr_nl d_nladdr;
    struct msghdr msg ;
    struct nlmsghdr *nlh=NULL ; // sizeof (struct nlmsghdr) = 24 bytes
    struct iovec iov;
	unsigned int *ptr;
	int ret;
    
    /* destination address */
    memset(&d_nladdr, 0 ,sizeof(d_nladdr));
    d_nladdr.nl_family = AF_NETLINK ;
    d_nladdr.nl_pad = 0;
    d_nladdr.nl_pid = 0; /* destined to kernel */
    d_nladdr.nl_groups = MYNETLINK_GROUP;

    /** Fill in packet for TX */
    /* Fill the netlink message header */
    nlh = (struct nlmsghdr *) malloc( NLMSG_SPACE(payloadlen + 8/*cmd+len*/) + NLMSG_HDRLEN );
    
    if (nlh == NULL)
    {
        printf ("Error: netlink_send(): Couldn't alloc memory for netlinkheader nlh\n");
        return -1;
    }
    
    memset(nlh , 0 , NLMSG_SPACE(payloadlen + 8/*cmd+len*/) + NLMSG_HDRLEN );
    ptr = (unsigned int *)NLMSG_DATA(nlh);
    ptr[0] = payloadcommand;
    ptr[1] = payloadlen;
    if (payloadlen != 0)
        memcpy( &ptr[2] /*after cmd+len*/, payload, payloadlen );

    payloadlen += 8/*+cmd+len*/;

    nlh->nlmsg_len = NLMSG_SPACE( payloadlen );
    nlh->nlmsg_pid = pthread_self() << 16 | getpid();
    nlh->nlmsg_flags = NLM_F_REQUEST;
    nlh->nlmsg_type = 21; // payloadtype; // can put control value here, must be >= NLMSG_MIN_TYPE=0x10

    /*iov structure */
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    /* msg */
    memset(&msg,0,sizeof(msg));
    msg.msg_name = (void *) &d_nladdr;
    msg.msg_namelen=sizeof(d_nladdr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    
    ret = sendmsg(fd, &msg, 0);
    if (ret < 0)
	printf("ERROR: sendmsg(): ret=%i, errno=%i (%s)\n", ret, errno, strerror(errno));
    
    free(nlh);
    return ret; 
}

int netlink_recv(int fd, netlink_packet_t *rxpkt, char isblocking)
{
    struct sockaddr_nl d_nladdr;
    struct msghdr msg ;
    struct nlmsghdr *nlh=NULL ; // sizeof (struct nlmsghdr) = 24 bytes
    struct iovec iov;
    int packetpayloadlen, ret;

    if (rxpkt == NULL || rxpkt->data == NULL) {
        printf ("Error: netlink_recv(): rxpkt or rxpkt->data points to NULL!\n");
        return -1;
    }
    
    /** Receive a packet from kernel */
    /* Fill the netlink message header */
    nlh = (struct nlmsghdr *) malloc( MAX_NL_PACKET  );
    if (nlh == NULL)
    {
        printf ("Error: netlink_recv(): Couldn't alloc memory for nlh\n");
        return -1;
    }
    
    memset(nlh , 0 , MAX_NL_PACKET);
    
    d_nladdr.nl_family = AF_NETLINK ;
    d_nladdr.nl_groups = MYNETLINK_GROUP;

    //nlh->nlmsg_len = NLMSG_SPACE( strlen(NLMSG_DATA(nlh)) );

    /*iov structure */
    iov.iov_base = (void *)nlh;
    iov.iov_len = NLMSG_SPACE( MAX_NL_PAYLOAD );

    /* msg */
    memset(&msg,0,sizeof(msg));
    msg.msg_name = (void *) &d_nladdr ;
    msg.msg_namelen=sizeof(d_nladdr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    
    /** Receive a packet from kernel */
    iov.iov_base = (void *)nlh;
    iov.iov_len = MAX_NL_PAYLOAD; //MAX_NL_MSG_LEN;

    ret = recvmsg(fd, &msg, (isblocking ? 0 : MSG_DONTWAIT) /*0=Block, MSG_DONTWAIT=nonblocking*/);
    
    if (ret < 0) {
	printf("Error in netlink_recv()->recvmsg(): ret=%i, errno=%i (%s)\n", ret, errno, strerror(errno));
	packetpayloadlen = -1;
    } else {
        packetpayloadlen = nlh->nlmsg_len - NLMSG_HDRLEN;
        if (packetpayloadlen >= 0)  {
		unsigned int *ptr = (unsigned int *)NLMSG_DATA(nlh);
		rxpkt->command  = ptr[0];
		rxpkt->len      = ptr[1];
		memcpy( rxpkt->data, &ptr[2], packetpayloadlen /*rxpkt->len*/ );
	} else  {
		rxpkt->command  = 0;
		rxpkt->len      = 0;
		printf ("Error: netlink_recv(): negative packetpayloadlen: %i\n", packetpayloadlen);
	}
    }

    free(nlh);
    
    return packetpayloadlen;
}

void netlink_close(fd)
{
	close(fd);
}
