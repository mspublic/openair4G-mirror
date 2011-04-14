/***************************************************************************
                          nasmt_device.c  -  description
                             -------------------
    copyright            : (C) 2002 by Eurecom
    email                : yan.moret@eurecom.fr
                           michelle.wetterwald@eurecom.fr
 ***************************************************************************
  Device Driver features for MT
 ***************************************************************************/

//#include "rt_compat.h"

#ifdef RTAI
#include "rtos_header.h"
#define RTAI_IRQ 30 //try to get this irq with RTAI

#else /* RTLINUX */
#include <rtl.h>
#endif

#include "nasmt_variables.h"
#include "nasmt_proto.h"
//
#include <linux/module.h>
#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <asm/segment.h>
#include <asm/page.h>
#include <asm/delay.h>
#include <asm/unistd.h>

struct net_device *gdev;
struct graal_priv *gpriv;
int bytes_wrote;
int bytes_read;
u8 GRAAL_NULL_IMEI[14]={0x00, 0x00, 0x00, 0x00, 0x00 ,0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ,0x00, 0x00, 0x00};

//---------------------------------------------------------------------------
//int graal_interrupt(int irq, void *dev_id, struct pt_regs *regs){
void nasmt_interrupt(void){
//---------------------------------------------------------------------------
	u8 cxi;
#ifdef GRAAL_DEBUG_INTERRUPT
	printk("nasmt_interrupt - begin\n");
#endif
	spin_lock(&gpriv->lock);
	cxi=0;
	nasmt_COMMON_QOS_receive(gpriv->cx+cxi);
	nasmt_ASCTL_GC_receive();
	nasmt_ASCTL_DC_receive(gpriv->cx+cxi);
	spin_unlock(&gpriv->lock);
#ifdef GRAAL_DEBUG_INTERRUPT
	printk("nasmt_interrupt: end\n");
#endif
//  return 0;
}

//---------------------------------------------------------------------------
// Called by ifconfig when the device is activated by ifconfig
int nasmt_open(struct net_device *dev){
//---------------------------------------------------------------------------
	printk("nasmt_open: begin\n");
//	MOD_INC_USE_COUNT;

  // Address has already been set at init

	if (((struct graal_priv *)(dev->priv))->irq==-EBUSY)
	{
		printk("nasmt_open: irq failure\n");
		return -EBUSY;
	}
	netif_start_queue(dev);
//
	init_timer(&gpriv->timer);
	(gpriv->timer).expires=jiffies+GRAAL_TIMER_TICK;
	(gpriv->timer).data=0L;
	(gpriv->timer).function=nasmt_ASCTL_timer;
	add_timer(&gpriv->timer);
//
	printk("nasmt_open: name = %s, end\n", dev->name);
	return 0;
}

//---------------------------------------------------------------------------
// Called by ifconfig when the device is desactivated by ifconfig
int nasmt_stop(struct net_device *dev){
//---------------------------------------------------------------------------
	struct graal_priv *priv = netdev_priv(dev);

	printk("nasmt_stop: begin\n");
	del_timer(&priv->timer);
	netif_stop_queue(dev);
//	MOD_DEC_USE_COUNT;
	printk("nasmt_stop: name = %s, end\n", dev->name);
	return 0;
}

//---------------------------------------------------------------------------
void nasmt_teardown(struct net_device *dev){
//---------------------------------------------------------------------------
	int cxi;
	struct graal_priv *priv = netdev_priv(dev);

	printk("nasmt_teardown: begin\n");
//	priv=(struct graal_priv *)(gdev.priv);
	if (priv->irq!=-EBUSY)
	{
		*pt_nas_ue_irq=-1;
#ifdef RTAI
// V1
//    rt_free_linux_irq(priv->irq, NULL);
// END V1
		rt_free_srq(priv->irq);
#else /* RTLinux */
		rtl_free_soft_irq(priv->irq);
#endif
// Start IRQ linux
//    free_irq(priv->irq, NULL);
// End IRQ linux

	}
//	for (sapi=0; sapi<GRAAL_SAPI_MAX; ++sapi)
//		close(priv->sap[sapi]);
	nasmt_CLASS_flush_rclassifier();
	cxi=0;
		nasmt_COMMON_flush_rb(gpriv->cx+cxi);
		nasmt_CLASS_flush_sclassifier(gpriv->cx+cxi);
//		for (sapi=0; sapi<GRAAL_SAPI_CX_MAX; ++sapi)
//			close(priv->cx[cxi].sap[sapi]);
	printk("nasmt_teardown: end\n");
}

//---------------------------------------------------------------------------
int nasmt_set_config(struct net_device *dev, struct ifmap *map){
//---------------------------------------------------------------------------
	printk("nasmt_set_config: begin\n");
	if (dev->flags & IFF_UP)
		return -EBUSY;
	if (map->base_addr != dev->base_addr)
	{
		printk(KERN_WARNING "nasmt_set_config: Can't change I/O address\n");
		return -EOPNOTSUPP;
	}
	if (map->irq != dev->irq)
		dev->irq = map->irq;
	printk("nasmt_set_config: end\n");
	return 0;
}

//---------------------------------------------------------------------------
//
int nasmt_hard_start_xmit(struct sk_buff *skb, struct net_device *dev){
//---------------------------------------------------------------------------
// Start debug information
#ifdef GRAAL_DEBUG_DEVICE
	printk("nasmt_hard_start_xmit: begin\n");
#endif
  if (!skb){
 	  printk("nasmt_hard_start_xmit - input parameter skb is NULL \n");
    return -1;
  }
// End debug information
	netif_stop_queue(dev);
	dev->trans_start = jiffies;
#ifdef GRAAL_DEBUG_SEND_DETAIL
	printk("nasmt_hard_start_xmit: step 1\n");
#endif
	nasmt_CLASS_send(skb);
#ifdef GRAAL_DEBUG_SEND_DETAIL
	printk("nasmt_hard_start_xmit: step 2\n");
#endif
  dev_kfree_skb(skb);
#ifdef GRAAL_DEBUG_SEND_DETAIL
	printk("nasmt_hard_start_xmit: step 3\n");
#endif
	netif_wake_queue(dev);
#ifdef GRAAL_DEBUG_DEVICE
	printk("nasmt_hard_start_xmit: end\n");
#endif
	return 0;
}

//---------------------------------------------------------------------------
struct net_device_stats *nasmt_get_stats(struct net_device *dev){
//---------------------------------------------------------------------------
//	return &((struct graal_priv *)dev->priv)->stats;
  struct graal_priv *npriv = netdev_priv(dev);
	return &npriv->stats;
}

//---------------------------------------------------------------------------
int nasmt_change_mtu(struct net_device *dev, int mtu){
//---------------------------------------------------------------------------
	printk("nasmt_change_mtu: begin\n");
	if ((mtu<1280) || (mtu>1500))
		return -EINVAL;
	dev->mtu = mtu;
	return 0;
}

//---------------------------------------------------------------------------
void nasmt_tx_timeout(struct net_device *dev){
//---------------------------------------------------------------------------
	/* Transmitter timeout, serious problems. */
	printk("nasmt_tx_timeout: begin\n");
	((struct graal_priv *)(dev->priv))->stats.tx_errors++;
	dev->trans_start = jiffies;
	netif_wake_queue(dev);
	printk("nasmt_tx_timeout: transmit timed out %s\n",dev->name);
}

//---------------------------------------------------------------------------
// Initialisation of the network device
void nasmt_init(struct net_device *dev){
//---------------------------------------------------------------------------
	u8 cxi, dscpi;
//	int write_flag = O_WRONLY | O_NONBLOCK | O_NDELAY;
//	int read_flag = O_RDONLY | O_NONBLOCK | O_NDELAY;
//  int ret;

	printk("nasmt_init: begin\n");
//	SET_MODULE_OWNER(dev);
//	dev->priv = kmalloc(sizeof(struct graal_priv),GFP_KERNEL);
//	if (dev->priv == NULL)
//	{
//		printk("GRAAL_INIT: no enough memory for private parameters\n");
//		return -ENOMEM;
//	}

	memset(dev->priv, 0, sizeof(struct graal_priv));
//	gpriv=(struct graal_priv *)(dev->priv);
	gpriv=netdev_priv(dev);
//

	dev->open = nasmt_open;
	dev->stop = nasmt_stop;
	dev->set_config = nasmt_set_config;
	dev->hard_start_xmit = nasmt_hard_start_xmit;
	dev->do_ioctl = nasmt_CTL_ioctl;
	dev->get_stats = nasmt_get_stats;
	dev->rebuild_header = NULL;
	dev->hard_header = NULL;
	dev->change_mtu = nasmt_change_mtu;
	dev->hard_header_cache = NULL;
	dev->header_cache_update = NULL;
	dev->tx_timeout = nasmt_tx_timeout;
//
	dev->type = ARPHRD_EURUMTS;
	dev->features = NETIF_F_NO_CSUM;
	dev->hard_header_len = 0;
	dev->addr_len = GRAAL_ADDR_LEN;
	dev->flags = IFF_MULTICAST|IFF_NOARP;
	dev->tx_queue_len = GRAAL_TX_QUEUE_LEN;
	dev->mtu = GRAAL_MTU;
//
// Initialize private structure
	gpriv->sap[GRAAL_GC_SAPI] = RRC_DEVICE_GC;
	gpriv->sap[GRAAL_NT_SAPI] = RRC_DEVICE_NT;
	gpriv->cx[0].sap[GRAAL_DC_INPUT_SAPI] = RRC_DEVICE_DC_INPUT0;
	gpriv->cx[0].sap[GRAAL_DC_OUTPUT_SAPI] = RRC_DEVICE_DC_OUTPUT0;
//	gpriv->sap[GRAAL_CO_INPUT_SAPI] = QOS_DEVICE_CONVERSATIONAL_INPUT;
//	gpriv->sap[GRAAL_ST_INPUT_SAPI] = QOS_DEVICE_STREAMING_INPUT;
//	gpriv->sap[GRAAL_IN_INPUT_SAPI] = QOS_DEVICE_INTERACTIVE_INPUT;
//	gpriv->sap[GRAAL_BA_INPUT_SAPI] = QOS_DEVICE_BACKGROUND_INPUT;
	// LG FORCE USING FIFO CONVERSATIONAL
	gpriv->sap[GRAAL_CO_INPUT_SAPI] = QOS_DEVICE_CONVERSATIONAL_INPUT;
	gpriv->sap[GRAAL_ST_INPUT_SAPI] = QOS_DEVICE_CONVERSATIONAL_INPUT;
	gpriv->sap[GRAAL_IN_INPUT_SAPI] = QOS_DEVICE_CONVERSATIONAL_INPUT;
	gpriv->sap[GRAAL_BA_INPUT_SAPI] = QOS_DEVICE_CONVERSATIONAL_INPUT;
//	gpriv->sap[GRAAL_CO_OUTPUT_SAPI] = QOS_DEVICE_CONVERSATIONAL_OUTPUT;
//	gpriv->sap[GRAAL_ST_OUTPUT_SAPI] = QOS_DEVICE_STREAMING_OUTPUT;
//	gpriv->sap[GRAAL_IN_OUTPUT_SAPI] = QOS_DEVICE_INTERACTIVE_OUTPUT;
//	gpriv->sap[GRAAL_BA_OUTPUT_SAPI] = QOS_DEVICE_BACKGROUND_OUTPUT;
	gpriv->sap[GRAAL_CO_OUTPUT_SAPI] = QOS_DEVICE_CONVERSATIONAL_OUTPUT;
	gpriv->sap[GRAAL_ST_OUTPUT_SAPI] = QOS_DEVICE_CONVERSATIONAL_OUTPUT;
	gpriv->sap[GRAAL_IN_OUTPUT_SAPI] = QOS_DEVICE_CONVERSATIONAL_OUTPUT;
	gpriv->sap[GRAAL_BA_OUTPUT_SAPI] = QOS_DEVICE_CONVERSATIONAL_OUTPUT;

	gpriv->retry_limit=GRAAL_RETRY_LIMIT_DEFAULT;
	gpriv->timer_establishment=GRAAL_TIMER_ESTABLISHMENT_DEFAULT;
	gpriv->timer_release=GRAAL_TIMER_RELEASE_DEFAULT;
	for (dscpi=0; dscpi<65; ++dscpi)
		gpriv->rclassifier[dscpi]=NULL;
	gpriv->nrclassifier=0;
//
	cxi=0;
#ifdef GRAAL_DEBUG_DEVICE
		printk("nasmt_init: init classifiers, state and timer for MT %u\n", cxi);
#endif
		gpriv->cx[cxi].state=GRAAL_IDLE;
		gpriv->cx[cxi].countimer=GRAAL_TIMER_IDLE;
		gpriv->cx[cxi].retry=0;
		gpriv->cx[cxi].lcr=cxi;
		gpriv->cx[cxi].rb=NULL;
		gpriv->cx[cxi].num_rb=0;
		// initialisation of the classifier
		for (dscpi=0; dscpi<65; ++dscpi)
			gpriv->cx[cxi].sclassifier[dscpi]=NULL;
		gpriv->cx[cxi].nsclassifier=0;
		// initialisation of the IP address
		nasmt_TOOL_imei2iid(GRAAL_NULL_IMEI, (u8 *)gpriv->cx[cxi].iid6);
		gpriv->cx[cxi].iid4=0;
//
	spin_lock_init(&gpriv->lock);

	nasmt_TOOL_imei2iid(nas_IMEI, dev->dev_addr);// IMEI to device address (for stateless autoconfiguration address)
	nasmt_TOOL_imei2iid(nas_IMEI, (u8 *)gpriv->cx[0].iid6);
	printk("nasmt_init: init IMEI to IID\n");
	nasmt_ASCTL_init();

	printk("nasmt_init: end\n");
	return;
}

//---------------------------------------------------------------------------
static int __init nasmt_init_module(void){
//---------------------------------------------------------------------------
	int err, ret=9;
  struct graal_priv *priv;

//	gdev.init = graal_init;
//	memcpy(gdev.name, "graal0", 7);
	printk("nasmt_init_module: begin \n");

// Initialize parameters shared with RRC
	if (pt_nas_ue_irq==NULL){
		printk("nasmt_init_module: shared irq parameter not initialised\n");
    err =  -EBUSY;
		printk("nasmt_init_module: returning %d \n\n", err);
		return err;
	}
	printk("nasmt_init_module: pt_nas_ue_irq valid \n");

  gdev = alloc_netdev(sizeof(struct graal_priv),"graal0", nasmt_init);

////
  priv = netdev_priv(gdev);

#ifdef RTAI //with RTAI you have to indicate which irq# you want
//V1
//	ret = rt_request_linux_irq(RTAI_IRQ, graal_interrupt, "graalirq", graal_interrupt);
////	ret = rt_request_linux_irq(RTAI_IRQ, graal_interrupt, "graalirq", gdev);
//
//	if (ret < 0) {
//		priv->irq = -EBUSY;
//		printk("\nGRAAL_INIT: RTAI tried to get irq %d, this failed.\n", RTAI_IRQ);
//	} else {
//		priv->irq = RTAI_IRQ;
//	}
// END V1
	priv->irq=rt_request_srq(0, nasmt_interrupt, NULL);

#else /* RTLinux */
	priv->irq=rtl_get_soft_irq(nasmt_interrupt, "graalirq");
#endif

	if (priv->irq == -EBUSY || priv->irq == -EINVAL){
		printk("\n nasmt_init_module: No interrupt resource available\n");
    if (gdev){
      	free_netdev(gdev);
        printk("nasmt_init_module: free_netdev ..\n");
    }
		return -EBUSY;
	}
	else
		printk("nasmt_init_module: Interrupt %d, ret = %d \n", priv->irq , ret);
  //rt_startup_irq(RTAI_IRQ);

  //rt_enable_irq(RTAI_IRQ);

	if (pt_nas_ue_irq==NULL){
		printk("nasmt_init_module: shared irq parameter has been reset\n");
  }else{
	*pt_nas_ue_irq=priv->irq;
  }
//
  err= register_netdev(gdev);
	if (err){
		printk("nasmt_init_module: error %i registering device %s\n", err, gdev->name);
	}else{
	  printk("nasmt_init_module: registering device %s, ifindex = %d\n\n",gdev->name, gdev->ifindex);
  }
	return err;

}

//---------------------------------------------------------------------------
static void __exit nasmt_cleanup_module(void){
//---------------------------------------------------------------------------
	printk("nasmt_cleanup_module: begin\n");
	unregister_netdev(gdev);
  nasmt_teardown(gdev);
	free_netdev(gdev);
	printk("nasmt_cleanup_module: end\n");
}

//---------------------------------------------------------------------------
module_init (nasmt_init_module);
module_exit (nasmt_cleanup_module);
//---------------------------------------------------------------------------

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("UMTS (TD-CDMA) Driver for Mobile Terminal, playing as Non Access Stratum");
MODULE_AUTHOR("Michelle Wetterwald <michelle.wetterwald@eurecom.fr>");
