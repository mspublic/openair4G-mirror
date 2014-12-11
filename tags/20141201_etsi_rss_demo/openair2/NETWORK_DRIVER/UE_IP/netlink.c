/*******************************************************************************
    OpenAirInterface
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is
   included in this distribution in the file called "COPYING". If not,
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@eurecom.fr

  Address      : Eurecom, Campus SophiaTech, 450 Route des Chappes, CS 50193 - 06904 Biot Sophia Antipolis cedex, FRANCE

*******************************************************************************/
/*! \file netlink.c
* \brief establish a netlink
* \author Navid Nikaein, Lionel Gauthier,  Raymond knopp
* \company Eurecom
* \email: navid.nikaein@eurecom.fr, lionel.gauthier@eurecom.fr, knopp@eurecom.fr
*/

//#include <linux/config.h>
#include <linux/socket.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/version.h>

#include "local.h"
#include "proto_extern.h"


#define NAS_NETLINK_ID 31
#define NL_DEST_PID 1

/*******************************************************************************
Prototypes
*******************************************************************************/
static inline void nasmesh_lock(void);
static inline void nasmesh_unlock(void);
static void nas_nl_data_ready (struct sk_buff *skb);
int ue_ip_netlink_init(void);

static struct sock *nas_nl_sk = NULL;
static int exit_netlink_thread=0;

#ifdef LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
struct netlink_kernel_cfg cfg = {
    .input = nas_nl_data_ready,
};
#endif

static DEFINE_MUTEX(nasmesh_mutex);


static inline void nasmesh_lock(void)
{
	mutex_lock(&nasmesh_mutex);
}

static inline void nasmesh_unlock(void)
{
	mutex_unlock(&nasmesh_mutex);
}

// This can also be implemented using thread to get the data from PDCP without blocking.
static void nas_nl_data_ready (struct sk_buff *skb)
{
  // wake_up_interruptible(skb->sk->sk_sleep);
  //nasmesh_lock();
  //netlink_rcv_skb(skb, &my_rcv_msg);// my_rcv_msg is the call back func>
  //nasmesh_unlock();

  struct nlmsghdr *nlh = NULL;
  if (skb) {
#ifdef NETLINK_DEBUG
    printk("[UE_IP_DRV][NETLINK] Received socket from PDCP\n");
#endif //NETLINK_DEBUG
    nlh = (struct nlmsghdr *)skb->data;
    ue_ip_common_wireless2ip(nlh);
    //kfree_skb(skb); // not required,
  }
}


int ue_ip_netlink_init(void)
{

  printk("[UE_IP_DRV][NETLINK] Running init ...\n");

  nas_nl_sk = netlink_kernel_create(
          &init_net,
#ifdef LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
          NAS_NETLINK_ID,
          &cfg
#else
          NAS_NETLINK_ID,
          0,
          nas_nl_data_ready,
          &nasmesh_mutex, // NULL
          THIS_MODULE
#endif
  );


  if (nas_nl_sk == NULL) {

    printk("[UE_IP_DRV][NETLINK] netlink_kernel_create failed \n");
    return(-1);
  }



  return(0);

}


void ue_ip_netlink_release(void) {

  exit_netlink_thread=1;
  printk("[UE_IP_DRV][NETLINK] Releasing netlink socket\n");

  if(nas_nl_sk){
    netlink_kernel_release(nas_nl_sk); //or skb->sk

  }

 //  printk("[UE_IP_DRV][NETLINK] Removing netlink_rx_thread\n");
 //kthread_stop(netlink_rx_thread);

}




int ue_ip_netlink_send(unsigned char *data,unsigned int len) {


  struct sk_buff *nl_skb = alloc_skb(NLMSG_SPACE(len),GFP_ATOMIC);
  struct nlmsghdr *nlh = (struct nlmsghdr *)nl_skb->data;
  int status;


  //  printk("[UE_IP_DRV][NETLINK] Sending %d bytes (%d)\n",len,NLMSG_SPACE(len));
  skb_put(nl_skb, NLMSG_SPACE(len));
  memcpy(NLMSG_DATA(nlh),data,len);

  nlh->nlmsg_len = NLMSG_SPACE(len);


  nlh->nlmsg_pid = 0;      /* from kernel */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 0)
  NETLINK_CB(nl_skb).pid = 0;
#endif

#ifdef NETLINK_DEBUG
  printk("[UE_IP_DRV][NETLINK] In nas_netlink_send, nl_skb %p, nl_sk %x, nlh %p, nlh->nlmsg_len %d\n",nl_skb,nas_nl_sk,nlh,nlh->nlmsg_len);
#endif //DEBUG_NETLINK

  if (nas_nl_sk) {

    //  nasmesh_lock();
    status = netlink_unicast(nas_nl_sk, nl_skb, NL_DEST_PID, MSG_DONTWAIT);
    // mutex_unlock(&nasmesh_mutex);

    if (status < 0) {
	printk("[UE_IP_DRV][NETLINK] SEND status is %d\n",status);
	return(0);
    }
    else {
#ifdef NETLINK_DEBUG
      printk("[UE_IP_DRV][NETLINK] SEND status is %d\n",status);
#endif
      return len;
    }
  }
  else {
    printk("[UE_IP_DRV][SEND] socket is NULL\n");
    return(0);
  }
}
