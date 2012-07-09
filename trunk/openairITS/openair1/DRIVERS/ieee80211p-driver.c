/******************************************************************************
 *
 * Copyright(c) EURECOM / Thales Communications & Security
 *
 * Portions of this file are derived from the ath5k project.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * Thales Communications & Seucrity <philippe.agostini@thalesgroup.com>
 *
 *****************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/skbuff.h>

#include "ieee80211p-device.h"
#include "ieee80211p-driver.h"

/******************************************************************************
 *
 * Module information
 *
 *****************************************************************************/

#define DRV_DESCRIPTION	"EURECOM / THALES COMMUNICATIONS & SECURITY IEEE 802.11p driver"
#define DRV_VERSION "V0.1"
#define DRV_AUTHOR "EURECOM / THALES COMMUNICATIONS & SECURITY"

MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
MODULE_DESCRIPTION(DRV_DESCRIPTION);
MODULE_AUTHOR(DRV_AUTHOR);

/******************************************************************************
 *
 * Global variables
 *
 *****************************************************************************/

/* Driver's private data */	
static struct ieee80211p_device_priv priv;

/******************************************************************************
 * 
 * Callbacks from mac80211p to the driver
 *
 *****************************************************************************/

int ieee80211p_tx_buf_setup(struct ieee80211p_skbqueue *buf, struct sk_buff *skb) {
	
	/* Return value */	
	int ret = 0;	

	/* Element allocation */
	buf = kzalloc(sizeof(struct ieee80211p_skbqueue),GFP_KERNEL);

	if (!buf) {
		ret = -1;
		goto error;
	}

	/* skb copy */
	buf->skb = skb_clone(skb,GFP_KERNEL);

	if (!buf->skb) {
		ret = -1;
		goto error;
	}

error:
	return ret;

} /* ieee80211p_tx_buf_setup */

void ieee80211p_tx_queue(struct ieee80211_hw *hw, struct sk_buff *skb,
					struct ieee80211p_txq *txq) {	

	/* Return value */
	int ret = 0;	

	/* New buffered frame */
	struct ieee80211p_skbqueue *buf;

	/* Get driver's private data */
	struct ieee80211p_device_priv *priv = hw->priv;

	/* TODO : If needed add padding after the header */

	/* TODO : Send the skb to the hw */
	
	/* If the hw is busy, put the skb in queue */
	if (ret == -1) {
		/* Check if there's space in the queue */
		/* If there's not, stop mac80211 queues and drop the frame */	
		if (txq->queue_len >= txq->queue_max) {
			ieee80211_stop_queue(hw, txq->queue_num);
			dev_kfree_skb_any(skb);
			printk(KERN_ERR "ieee80211p_tx_queue: frame dropped\n");
			return;
		}

		/* Lock */	
		spin_lock(&priv->txqlock);
		
		/* Create a new ieee80211p_skbqueue element */
		ret = ieee80211p_tx_buf_setup(buf,skb);

		if(ret == -1) {
			dev_kfree_skb_any(skb);
			printk(KERN_ERR "ieee80211p_tx_queue: alloc failed\n");
			return;
		}

		/* Add ieee80211p_skbqueue element to the queue */		
		list_add_tail(&buf->list,&txq->queue.list);
		txq->queue_len++;

		/* Schedule the TX tasklet for delayed TX */
		tasklet_schedule(&priv->txtq);
		priv->tx_pending = TRUE;

		/* Unlock */ 
		spin_unlock(&priv->txqlock);
	} 
	
	/* Else the transmission went fine */	
	else {
	
		ieee80211_tx_status(hw,skb);
	}
	
} /* ieee80211p_tx_queue */

static void ieee80211p_tx(struct ieee80211_hw *hw, struct sk_buff *skb) {
	
	/* Get driver's private data */
	struct ieee80211p_device_priv *priv = hw->priv;

	/* Get the number of the TX queue */
	int qnum = skb_get_queue_mapping(skb);

	if (qnum >= IEEE80211P_NUM_TXQ) {
		printk(KERN_ERR "ieee80211p_tx: wrong queue number\n");
		dev_kfree_skb_any(skb);
		return;
	}

	ieee80211p_tx_queue(hw,skb,&priv->txqs[qnum]);
} /* ieee80211p_tx */


static int ieee80211p_start(struct ieee80211_hw *hw) {

	/* Nothing to be done here */	
	
	return 0;
} /* ieee80211p_start */


static void ieee80211p_stop(struct ieee80211_hw *hw) {

	/* Nothing to be done here */
} /* ieee80211p_stop */


static int ieee80211p_add_interface(struct ieee80211_hw *hw, struct ieee80211_vif *vif) {
	
	struct ieee80211p_device_priv *priv = hw->priv;
	struct ieee80211p_vif_priv *vif_priv = (void *)vif->drv_priv;	
	int ret = 0;	

	spin_lock(&priv->lock);

	/* IEEE 80211.p only supports STA interfaces */	
	if (vif->type == NL80211_IFTYPE_STATION || vif->type == NL80211_IFTYPE_ADHOC) {
		vif_priv->opmode = vif->type;
	} else {
		ret = -EOPNOTSUPP;
		goto end;
	}

	/* Keep track of the number of vifs */	
	priv->nvifs++;		

end:
	spin_unlock(&priv->lock);
	return ret;
} /* ieee80211p_add_interface */


static void ieee80211p_remove_interface(struct ieee80211_hw *hw, struct ieee80211_vif *vif) {

	struct ieee80211p_device_priv *priv = hw->priv;

	spin_lock(&priv->lock);
	
	/* Keep track of the number of vifs */
	priv->nvifs--;

	spin_unlock(&priv->lock);

} /* ieee80211p_remove_interface */


static int ieee80211p_config(struct ieee80211_hw *hw, u32 changed) {
	
	struct ieee80211p_device_priv *priv = hw->priv;
	struct ieee80211_conf *conf = &hw->conf;

	spin_lock(&priv->lock);

	if (changed & IEEE80211_CONF_CHANGE_CHANNEL) {
		if (conf->channel != NULL) {
			priv->curchan = conf->channel;
		}
	}

	if (changed & IEEE80211_CONF_CHANGE_POWER) {
		priv->power_level = conf->power_level;
	}
	
	spin_unlock(&priv->lock);
	
	return 0;
} /* ieee80211p_config */


static void ieee80211p_configure_filter(struct ieee80211_hw *hw, unsigned int changed_flags,
									 unsigned int *new_flags, u64 multicast) {

#define SUPPORTED_FIF_FLAGS \
	(FIF_PROMISC_IN_BSS | FIF_ALLMULTI | FIF_FCSFAIL | \
	FIF_PLCPFAIL | FIF_CONTROL | FIF_OTHER_BSS | \
	FIF_BCN_PRBRESP_PROMISC)	
	
	*new_flags &= SUPPORTED_FIF_FLAGS;	
}

/* Only the mandatory callbacks from ieee80211p_ops are implemented */
const struct ieee80211_ops ieee80211p_driver_ops = {		
	.tx = ieee80211p_tx,
	.start = ieee80211p_start,
	.stop = ieee80211p_stop,
	.add_interface = ieee80211p_add_interface,
	.remove_interface = ieee80211p_remove_interface,
	.config = ieee80211p_config,
	.configure_filter = ieee80211p_configure_filter,
};

/******************************************************************************
 *
 * Driver's exported functions
 *
 *****************************************************************************/

char *ieee80211p_driver_get_rx_addr(void) {
	return priv.rx_buf.buf;
}

EXPORT_SYMBOL(ieee80211p_driver_get_rx_addr);

/******************************************************************************
 *
 * Driver's initialization and release functions
 *
 *****************************************************************************/

static int ieee80211p_driver_start(struct ieee80211p_device_priv *priv) {

	/* Return value */	
	int ret = 0;

	/* Configuration and hardware information for an 802.11 PHY */
	struct ieee80211_hw *hw = NULL;
	
	/*******************************************
	 * Allocating hw (mac80211 main structure) * 
	 * and priv (driver private data)          *
	 *******************************************/

	hw = ieee80211_alloc_hw(sizeof(struct ieee80211p_device_priv),&ieee80211p_driver_ops);

	if (hw == NULL) {
		ret = -1;
		printk(KERN_ERR "ieee80211p_driver_start: can't alloc ieee80211 hw\n");
		goto error;
	}	

	priv->hw = hw;

	/*************************************************
	 * Initializing hardware and driver private data *
	 *************************************************/

	ret = ieee80211p_device_init(priv);

	if (ret == -1) {
		printk(KERN_ERR "ieee80211p_driver_start: can't init device hw\n");		
		goto error;
	}

	/******************
	 * Registering hw *
	 *****************/

	ret = ieee80211_register_hw(hw);

	if (ret) {
		printk(KERN_ERR "ieee80211p_driver_start: can't register ieee80211 hw\n");		
		goto error;
	}

	return 0;
	
error:
	return ret;
} /* ieee80211p_driver_start */

static void ieee80211p_driver_stop(struct ieee80211p_device_priv *priv) {
	
	/********************
	 * Freeing hardware *
	 ********************/

	struct ieee80211_hw *hw = priv->hw;
	
	ieee80211_unregister_hw(hw);
	ieee80211_free_hw(hw);

	/*********************************
 	 * Freeing driver's private data *
	 *********************************/

	ieee80211p_device_exit(priv);
} /* ieee80211p_driver_stop */

/******************************************************************************
 *
 * Driver entry point
 *
 *****************************************************************************/

/* Called at insmod */
static int __init ieee80211p_init(void)
{
	int ret = 0;	

	printk(KERN_DEBUG "ieee80211p_init: ieee80211p driver inserted\n");

	ret = ieee80211p_driver_start(&priv);

	if (ret == -1) {
		printk(KERN_ERR "ieee80211p_init: can't start ieee80211p driver\n");
	}

	return 0;
} /* ieee80211p_init */

/* Called at rmmod */
static void __exit ieee80211p_exit(void)
{
	ieee80211p_driver_stop(&priv);	

	printk(KERN_DEBUG "ieee80211p_init: ieee80211p driver removed\n");
} /* ieee80211p_exit */


module_exit(ieee80211p_exit);
module_init(ieee80211p_init);
