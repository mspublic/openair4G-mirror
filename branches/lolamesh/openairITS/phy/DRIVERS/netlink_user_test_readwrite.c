/* stand-alone netlink test file (used for first netlink tests */

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "netlink.h"

int mynetlink_init()
{
    int fd;
    struct sockaddr_nl s_nladdr;
    
    fd = socket(AF_NETLINK ,SOCK_RAW , NETLINK_EXMIMO );
    
    memset(&s_nladdr, 0 ,sizeof(s_nladdr));
    s_nladdr.nl_family= AF_NETLINK ;
    s_nladdr.nl_pad=0;
    s_nladdr.nl_pid = pthread_self() << 16 | getpid();
    s_nladdr.nl_groups = MYNETLINK_GROUP;

    bind(fd, (struct sockaddr*)&s_nladdr, sizeof(s_nladdr));
    
    return fd;
}

int mynetlink_send(int fd, char *payload, int payloadlen, int payloadtype)
{
    struct sockaddr_nl d_nladdr;
    struct msghdr msg ;
    struct nlmsghdr *nlh=NULL ; // sizeof (struct nlmsghdr) = 24 bytes
    struct iovec iov;
    
    /* destination address */
    memset(&d_nladdr, 0 ,sizeof(d_nladdr));
    d_nladdr.nl_family = AF_NETLINK ;
    d_nladdr.nl_pad = 0;
    d_nladdr.nl_pid = 0; /* destined to kernel */
    d_nladdr.nl_groups = MYNETLINK_GROUP;

    /** Fill in packet for TX */
    /* Fill the netlink message header */
    nlh = (struct nlmsghdr *) malloc( NLMSG_SPACE(payloadlen) + NLMSG_HDRLEN );
    
    if (nlh == NULL)
    {
        printf ("Error: netlink_send(): Couldn't alloc memory for nlh\n");
        return -1;
    }
    
    memset(nlh , 0 , NLMSG_SPACE(payloadlen) + NLMSG_HDRLEN);
    memcpy(NLMSG_DATA(nlh), payload, payloadlen );
    
    nlh->nlmsg_len = NLMSG_SPACE( payloadlen );
    nlh->nlmsg_pid = pthread_self() << 16 | getpid();
    nlh->nlmsg_flags = NLM_F_REQUEST;
    nlh->nlmsg_type = payloadtype; // can put control value here, must be >= NLMSG_MIN_TYPE=0x10

    /*iov structure */
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    /* msg */
    memset(&msg,0,sizeof(msg));
    msg.msg_name = (void *) &d_nladdr;
    msg.msg_namelen=sizeof(d_nladdr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    
    sendmsg(fd, &msg, 0);
    
    free(nlh);
}

int mynetlink_recv(int fd, char *dst)
{
    struct sockaddr_nl d_nladdr;
    struct msghdr msg ;
    struct nlmsghdr *nlh=NULL ; // sizeof (struct nlmsghdr) = 24 bytes
    struct iovec iov;
    int payloadlen;
    
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

    recvmsg(fd, &msg, 0 /*0=Block, MSG_DONTWAIT=nonblocking*/);
    
    payloadlen = nlh->nlmsg_len - NLMSG_HDRLEN;
    if (payloadlen >= 0)
        strncpy( dst, (char *)NLMSG_DATA(nlh), payloadlen);
    else
        printf ("Error: netlink_recv(): negative payloadlen: %i\n", payloadlen);

    
    free(nlh);
    
    return payloadlen;
}

int main()
{
	char str[] = "Mr. Kernel, Are you ready  (len=36)?";
    char str2[255];
	int i;
	
    int fd = mynetlink_init();

    mynetlink_send(fd, str, strlen( str )+1, 21);
    mynetlink_send(fd, str, strlen( str ), 22);
    
    memset(str2, 0, 255);
    i = mynetlink_recv(fd, str2);
    printf ("main(): Received Msg(len=%i): %s\n", i, str2 );
    
    i = mynetlink_recv(fd, str2);
    printf ("main(): Received Msg(len=%i): %s\n", i, str2 );    

    close(fd);
    return (EXIT_SUCCESS);
}