/***************************************************************************
                          nasrg_device.c  -  description
                             -------------------
    copyright            : (C) 2002 by Eurecom
    email                : yan.moret@eurecom.fr
                           michelle.wetterwald@eurecom.fr
 ***************************************************************************
  Device Driver features for RG
 ***************************************************************************/

//#include "rt_compat.h"

#ifdef RTAI
#include "rtos_header.h"
//#include <rtai.h>
#define RTAI_IRQ 200 //try to get this irq with RTAI

#else /* RTLINUX */
#include <rtl.h>
#endif

#include "nasrg_variables.h"
#include "nasrg_proto.h"
#include <linux/module.h>
#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <asm/segment.h>
#include <asm/page.h>
#include <asm/delay.h>
#include <asm/unistd.h>

/* Global variables */
struct net_device *gdev;
struct graal_priv *gpriv;
int bytes_wrote;
int bytes_read;
u8 GRAAL_RG_IMEI[14]={0x00, 0x00, 0x00, 0x00, 0x00 ,0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ,0x00, 0x00, 0x01};
u8 GRAAL_NULL_IMEI[14]={0x00, 0x00, 0x00, 0x00, 0x00 ,0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ,0x00, 0x00, 0x00};

//---------------------------------------------------------------------------
//int graal_interrupt(int irq, void *dev_id, struct pt_regs *regs){
void nasrg_interrupt(void){
//---------------------------------------------------------------------------
	u8 cxi;
#ifdef GRAAL_DEBUG_INTERRUPT
	printk("nasrg_interrupt - begin\n");
#endif
	spin_lock(&gpriv->lock);
	for (cxi=0;cxi<GRAAL_CX_MAX;++cxi)
		nasrg_COMMON_QOS_receive(gpriv->cx+cxi);
	for (cxi=0;cxi<GRAAL_CX_MAX;++cxi)
		nasrg_ASCTL_DC_receive(gpriv->cx+cxi);
	spin_unlock(&gpriv->lock);
#ifdef GRAAL_DEBUG_INTERRUPT
	printk("nasrg_interrupt: end\n");
#endif
//  return 0;
}

//---------------------------------------------------------------------------
// Called by ifconfig when the device is activated by ifconfig
int nasrg_open(struct net_device *dev){
//---------------------------------------------------------------------------
	printk("nasrg_open: begin\n");

//	MOD_INC_USE_COUNT;

  // Address has already been set at init

	if (((struct graal_priv *)(dev->priv))->irq==-EBUSY)
	{
		printk("nasrg_open: irq failure\n");
		return -EBUSY;
	}
	netif_start_queue(dev);
//
  init_timer(&gpriv->timer);
	(gpriv->timer).expires=jiffies+GRAAL_TIMER_TICK;
	(gpriv->timer).data=0L;
	(gpriv->timer).function=nasrg_ASCTL_timer;
	add_timer(&gpriv->timer);
//
	printk("nasrg_open: name = %s, end\n", dev->name);
	return 0;
}

//---------------------------------------------------------------------------
// Called by ifconfig when the device is desactivated by ifconfig
int nasrg_stop(struct net_device *dev){
//---------------------------------------------------------------------------
	struct graal_priv *priv = netdev_priv(dev);
	printk("nasrg_stop: begin\n");
	del_timer(&priv->timer);
	netif_stop_queue(dev);
//	MOD_DEC_USE_COUNT;
	printk("nasrg_stop: name = %s, end\n", dev->name);
	return 0;
}

//---------------------------------------------------------------------------
void nasrg_teardown(struct net_device *dev){
//---------------------------------------------------------------------------
	int cxi;
	struct graal_priv *priv = netdev_priv(dev);

	printk("nasrg_teardown: begin\n");
//	priv=(struct graal_priv *)(gdev.priv);
	if (priv->irq!=-EBUSY)
	{
		*pt_nas_rg_irq=-1;
#ifdef RTAI
//    rt_free_linux_irq(priv->irq, NULL);
		rt_free_srq(priv->irq);
#else /* RTLinux */
		rtl_free_soft_irq(priv->irq);
#endif
	}
//	for (sapi=0; sapi<GRAAL_SAPI_MAX; ++sapi)
//		close(priv->sap[sapi]);
	nasrg_CLASS_flush_rclassifier();
  nasrg_CLASS_flush_mbmsclassifier();
	for (cxi=0;cxi<GRAAL_CX_MAX;++cxi)
	{
		nasrg_COMMON_flush_rb(gpriv->cx+cxi);
		nasrg_CLASS_flush_sclassifier(gpriv->cx+cxi);
//		for (sapi=0; sapi<GRAAL_SAPI_CX_MAX; ++sapi)
//			close(priv->cx[cxi].sap[sapi]);
	}
	printk("nasrg_teardown: end\n");
}

//---------------------------------------------------------------------------
int nasrg_set_config(struct net_device *dev, struct ifmap *map){
//---------------------------------------------------------------------------
	printk("nasrg_set_config: begin\n");
	if (dev->flags & IFF_UP)
		return -EBUSY;
	if (map->base_addr != dev->base_addr)
	{
		printk(KERN_WARNING "nasrg_set_config: Can't change I/O address\n");
		return -EOPNOTSUPP;
	}
	if (map->irq != dev->irq)
		dev->irq = map->irq;
	return 0;
}

//---------------------------------------------------------------------------
int nasrg_hard_start_xmit(struct sk_buff *skb, struct net_device *dev){
//---------------------------------------------------------------------------
// Start debug information
#ifdef GRAAL_DEBUG_DEVICE
	printk("nasrg_hard_start_xmit: begin\n");
#endif
  if (!skb){
 	  printk("nasrg_hard_start_xmit - input parameter skb is NULL \n");
    return -1;
  }
// End debug information
	netif_stop_queue(dev);
	dev->trans_start = jiffies;
#ifdef GRAAL_DEBUG_SEND_DETAIL
	printk("nasrg_hard_start_xmit: step 1\n");
#endif
	nasrg_CLASS_send(skb);
#ifdef GRAAL_DEBUG_SEND_DETAIL
	printk("nasrg_hard_start_xmit: step 2\n");
#endif
//  if (!skb){
// 	  printk("GRAAL_HARD_START_XMIT - parameter skb is NULL \n");
//    return -1;
//  }else
  dev_kfree_skb(skb);
#ifdef GRAAL_DEBUG_SEND_DETAIL
	printk("nasrg_hard_start_xmit: step 3\n");
#endif
	netif_wake_queue(dev);
#ifdef GRAAL_DEBUG_DEVICE
	printk("nasrg_hard_start_xmit: end\n");
#endif
	return 0;
}

//---------------------------------------------------------------------------
struct net_device_stats *nasrg_get_stats(struct net_device *dev){
//---------------------------------------------------------------------------
  struct graal_priv *npriv = netdev_priv(dev);
	return &npriv->stats;
}

//---------------------------------------------------------------------------
int nasrg_change_mtu(struct net_device *dev, int mtu){
//---------------------------------------------------------------------------
	printk("nasrg_change_mtu: begin\n");
	if ((mtu<1280) || (mtu>1500))
		return -EINVAL;
	dev->mtu = mtu;
	return 0;
}

//---------------------------------------------------------------------------
void nasrg_tx_timeout(struct net_device *dev){
//---------------------------------------------------------------------------
	/* Transmitter timeout, serious problems. */
	printk("nasrg_tx_timeout: begin\n");
	((struct graal_priv *)(dev->priv))->stats.tx_errors++;
	dev->trans_start = jiffies;
	netif_wake_queue(dev);
	printk("nasrg_tx_timeout: transmit timed out %s\n",dev->name);
}

//---------------------------------------------------------------------------
// Initialisation of the network device
void nasrg_init(struct net_device *dev){
//---------------------------------------------------------------------------
	u8 cxi, dscpi;
//	int write_flag = O_WRONLY | O_NONBLOCK | O_NDELAY;
//	int read_flag = O_RDONLY | O_NONBLOCK | O_NDELAY;
// int ret;

	printk("nasrg_init: begin\n");
//	SET_MODULE_OWNER(dev);
//	dev->priv = kmalloc(sizeof(struct graal_priv),GFP_KERNEL);
//	if (dev->priv == NULL)
//	{
//		printk("GRAAL_INIT: no enough memory for private parameters\n");
//		return -ENOMEM;
//	}

	memset(dev->priv, 0, sizeof(struct graal_priv));
	gpriv=netdev_priv(dev);
//
	dev->open = nasrg_open;
	dev->stop = nasrg_stop;
	dev->set_config = nasrg_set_config;
	dev->hard_start_xmit = nasrg_hard_start_xmit;
	dev->do_ioctl = nasrg_CTL_ioctl;
	dev->get_stats = nasrg_get_stats;
	dev->rebuild_header = NULL;
	dev->hard_header = NULL;
	dev->change_mtu = nasrg_change_mtu;
	dev->hard_header_cache = NULL;
	dev->header_cache_update = NULL;
	dev->tx_timeout = nasrg_tx_timeout;
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
	gpriv->cx[1].sap[GRAAL_DC_INPUT_SAPI] = RRC_DEVICE_DC_INPUT1;
	gpriv->cx[1].sap[GRAAL_DC_OUTPUT_SAPI] = RRC_DEVICE_DC_OUTPUT1;
//	gpriv->sap[GRAAL_CO_INPUT_SAPI] = QOS_DEVICE_CONVERSATIONAL_INPUT;
//	gpriv->sap[GRAAL_ST_INPUT_SAPI] = QOS_DEVICE_STREAMING_INPUT;
//	gpriv->sap[GRAAL_IN_INPUT_SAPI] = QOS_DEVICE_INTERACTIVE_INPUT;
//	gpriv->sap[GRAAL_BA_INPUT_SAPI] = QOS_DEVICE_BACKGROUND_INPUT;
	gpriv->sap[GRAAL_CO_INPUT_SAPI] = QOS_DEVICE_CONVERSATIONAL_INPUT;
	gpriv->sap[GRAAL_ST_INPUT_SAPI] = QOS_DEVICE_STREAMING_INPUT;
	gpriv->sap[GRAAL_IN_INPUT_SAPI] = QOS_DEVICE_INTERACTIVE_INPUT;
	gpriv->sap[GRAAL_BA_INPUT_SAPI] = QOS_DEVICE_BACKGROUND_INPUT;
//	gpriv->sap[GRAAL_CO_OUTPUT_SAPI] = QOS_DEVICE_CONVERSATIONAL_OUTPUT;
//	gpriv->sap[GRAAL_ST_OUTPUT_SAPI] = QOS_DEVICE_STREAMING_OUTPUT;
//	gpriv->sap[GRAAL_IN_OUTPUT_SAPI] = QOS_DEVICE_INTERACTIVE_OUTPUT;
//	gpriv->sap[GRAAL_BA_OUTPUT_SAPI] = QOS_DEVICE_BACKGROUND_OUTPUT;
	gpriv->sap[GRAAL_CO_OUTPUT_SAPI] = QOS_DEVICE_CONVERSATIONAL_OUTPUT;
	gpriv->sap[GRAAL_ST_OUTPUT_SAPI] = QOS_DEVICE_STREAMING_OUTPUT;
	gpriv->sap[GRAAL_IN_OUTPUT_SAPI] = QOS_DEVICE_INTERACTIVE_OUTPUT;
	gpriv->sap[GRAAL_BA_OUTPUT_SAPI] = QOS_DEVICE_BACKGROUND_OUTPUT;
//
	gpriv->retry_limit=GRAAL_RETRY_LIMIT_DEFAULT;
	gpriv->timer_establishment=GRAAL_TIMER_ESTABLISHMENT_DEFAULT;
	gpriv->timer_release=GRAAL_TIMER_RELEASE_DEFAULT;
	for (dscpi=0; dscpi<GRAAL_DSCP_MAX; ++dscpi)
		gpriv->rclassifier[dscpi]=NULL;
	gpriv->nrclassifier=0;
//
	for(cxi=0; cxi<GRAAL_CX_MAX;++cxi)
	{
#ifdef GRAAL_DEBUG_DEVICE
		printk("nasrg_init: init classifiers, state and timer for MTs %u\n", cxi);
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
		nasrg_TOOL_imei2iid(GRAAL_NULL_IMEI, (u8 *)gpriv->cx[cxi].iid6);
		gpriv->cx[cxi].iid4=0;
	}
	spin_lock_init(&gpriv->lock);

	nasrg_TOOL_RGimei2iid(GRAAL_RG_IMEI, dev->dev_addr);// IMEI to device address (for stateless autoconfiguration address)
	printk("nasrg_init: init IMEI to IID\n");
	nasrg_ASCTL_init();

	printk("nasrg_init: end\n");
	return ;
}

//---------------------------------------------------------------------------
static int __init nasrg_init_module(void){
//---------------------------------------------------------------------------
	int err, ret=9;
  struct graal_priv *priv;

//	gdev.init = graal_init;
//	memcpy(gdev.name, "graal0", 7);
	printk("nasrg_init_module: begin \n");

// Initialize parameters shared with RRC
	if (pt_rg_own_cell_id==NULL){
		printk("nasrg_init_module: shared cell_id parameter not initialised\n");
    err =  -EBUSY;
		printk("nasrg_init_module: returning %d \n\n\n", err);
		return err;
	}
	printk("nasrg_init_module: pt_rg_own_cell_id valid \n");
	*pt_rg_own_cell_id = NASRG_OWN_CELLID;

	if (pt_nas_rg_irq==NULL){
		printk("nasrg_init_module: shared irq parameter not initialised\n");
    err =  -EBUSY;
		printk("nasrg_init_module: returning %d \n\n", err);
		return err;
	}
	printk("nasrg_init_module: pt_nas_rg_irq valid \n");

  gdev = alloc_netdev(sizeof(struct graal_priv),"graal0", nasrg_init);

	printk("nasrg_init_module: after alloc_netdev \n");

////
  priv = netdev_priv(gdev);

////
//
#ifdef RTAI //with RTAI you have to indicate which irq# you want
//	ret = rt_request_linux_irq(RTAI_IRQ, graal_interrupt, "graalirq", gdev);
//
//	if (ret < 0) {
//		priv->irq = -EBUSY;
//		printk("NASRG_init_module: RTAI tried to get irq %d, this failed.\n", RTAI_IRQ);
//	} else {
//		priv->irq = RTAI_IRQ;
//	}
	priv->irq=rt_request_srq(0, nasrg_interrupt, NULL);
#else /* RTLinux */
	priv->irq=rtl_get_soft_irq(nasrg_interrupt, "graalirq");
#endif

	if (priv->irq == -EBUSY){
		printk("nasrg_init_module: No interrupt resource available\n");
    if (gdev){
      	free_netdev(gdev);
        printk("nasrg_init_module: free_netdev ..\n");
    }
		return -EBUSY;
	}else
		printk("nasrg_init_module: Interrupt %d, ret = %d \n", priv->irq , ret);

	*pt_nas_rg_irq= priv->irq;
//
//////

  err= register_netdev(gdev);

	if (err){
		printk("nasrg_init_module: error %i registering device %s\n", err, gdev->name);
	}else{
	  printk("nasrg_init_module: registering device %s, ifindex = %d\n\n",gdev->name, gdev->ifindex);
  }
	return err;
}

//---------------------------------------------------------------------------
static void __exit nasrg_cleanup_module(void){
//---------------------------------------------------------------------------
	printk("nasrg_cleanup_module: begin\n");
	unregister_netdev(gdev);
  nasrg_teardown(gdev);
	free_netdev(gdev);
	printk("nasrg_cleanup_module: end\n");
}

//---------------------------------------------------------------------------
module_init (nasrg_init_module);
module_exit (nasrg_cleanup_module);
//---------------------------------------------------------------------------
