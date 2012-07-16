/** netlink_kernelapi.c

    Communication with userspace through netlink sockets, API

    Used in Kernel device driver to interface with netlink routines through skbuff

    Author: Matthias Ihmig <matthias.ihmig@mytum.de>, 2011
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <net/net_namespace.h>
#include <linux/kthread.h>

#include "netlink.h"
#include "extern.h"

static struct sock *nl_sk = NULL;
static int pid_user = 0;

void (*nl_rx_handler)(char *, int) = NULL;

// this example is from the internet. if it doesn't work, see for different examnple: http://tomoyo.sourceforge.jp/cgi-bin/lxr/source/drivers/scsi/scsi_netlink.c#L79
static void nl_data_ready (struct sk_buff *skb)
{
    struct nlmsghdr *nlh = NULL;
    unsigned int rlen, len, msgtype;
    unsigned int *p;
    if (skb == NULL)
    {
        printk("%s: ERROR: skb is NULL\n", __func__);
        return;
    }
    nlh = (struct nlmsghdr *)skb->data;

    rlen = NLMSG_ALIGN(nlh->nlmsg_len);
    if (rlen > skb->len)
        rlen = skb->len;

    pid_user = nlh->nlmsg_pid;
    len = nlmsg_len(nlh);
    msgtype = nlh->nlmsg_type;
    p = (unsigned int *) NLMSG_DATA(nlh);
    printk("[EXMIMO]: nl_data_ready(): received netlink message (pid=%i, len=%i, type=%i): %08X %08X %08X %08X\n",pid_user, len, msgtype, p[0], p[1], p[2], p[3]);

    (*nl_rx_handler) (NLMSG_DATA(nlh), len);

    skb_pull( skb, rlen );
}

void netlink_send(char *msg, unsigned int len)
{
    struct nlmsghdr *nlh = NULL;
    struct sk_buff *skb_nltx = NULL;
    unsigned int *ptr = (unsigned int*)msg;

    int err;
    
    if (pid_user == 0)
        printk("[EXMIMO]: netlink_send(): ERROR: pid_user not set (0)! Userspace must send a packet first.\n");

    skb_nltx = alloc_skb( NLMSG_SPACE( len ), in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
    if (skb_nltx == NULL)
    {
        printk ("[EXMIMO]: ERROR: netlink_send(): alloc_skb (%i bytes) failed\n", MAX_NL_PAYLOAD);
        return;
    }
  
    skb_put( skb_nltx, NLMSG_SPACE( len ) );

    nlh = (struct nlmsghdr *) skb_nltx->data;
    nlh->nlmsg_len = NLMSG_SPACE( len );
    nlh->nlmsg_pid = pid_user; // 0:from kernel
    nlh->nlmsg_flags = 0;

    memcpy( NLMSG_DATA( nlh ), msg, len);

    NETLINK_CB(skb_nltx).pid = 0;     // from kernel
    NETLINK_CB(skb_nltx).dst_group = MYNETLINK_GROUP;

    err = netlink_unicast(nl_sk, skb_nltx, pid_user, /*dst_group*/ MYNETLINK_GROUP);
    //err = netlink_broadcast(nl_sk, skb, /*pid*/0, /*dst_group*/ MYNETLINK_GROUP, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);

    if (err <= 0)
	printk("[EXMIMO]: ERROR in netlink_send()->netlink_unicast(): %i\n", err);
    printk("[EXMIMO]: netlink_send(): len=%i, ret= %i from netlink_unicast (msg: %08X %08X %08X %08X) \n", len, err, ptr[0], ptr[1], ptr[2], ptr[3]);

    //dev_kfree_skb( skb_nltx ); // it seems it is freed by netlink_unicast (at least for now), see http://stackoverflow.com/questions/5884711/problem-with-netlink-socket-kernel-freeze
}

int netlink_init( void (*netlink_rx_handler)(char *, int) )
{
    nl_rx_handler = netlink_rx_handler;

    nl_sk = netlink_kernel_create(&init_net,NETLINK_EXMIMO,0, nl_data_ready,NULL, THIS_MODULE);
    if (nl_sk == NULL)
    {
        printk ("[EXMIMO]: ERROR: netlink_init(): netlink_kernel_create failed\n");
        return (-1);
    }

/*    skb_nltx = alloc_skb( NLMSG_SPACE( MAX_NL_PAYLOAD ), in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
    if (skb_nltx == NULL)
    {
        printk ("[EXMIMO]: netlink_init(): alloc_skb (%i bytes) failed\n", MAX_NL_PAYLOAD);
        sock_release(nl_sk->sk_socket);
        return (-1);
    }*/
    return 0;
}

void netlink_release(void)
{
    //dev_kfree_skb( skb_nltx );
    sock_release(nl_sk->sk_socket);
}



 

