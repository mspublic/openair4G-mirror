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

/******************************************************************************
 *
 * Module information
 *
 ******************************************************************************/

#define DRV_DESCRIPTION	"EURECOM / THALES COMMUNICATIONS & SECURITY IEEE 802.11p driver"
#define DRV_VERSION "V0.1"
#define DRV_AUTHOR "EURECOM / THALES COMMUNICATIONS & SECURITY"

MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
MODULE_DESCRIPTION(DRV_DESCRIPTION);
MODULE_AUTHOR(DRV_AUTHOR);

/*****************************************************************************
 * 
 * Callbacks from mac80211p to the driver
 *
 ****************************************************************************/

/*void ieee80211p_tx_queue(struct ieee80211_hw *hw, struct sk_buff *skb,
					struct ieee80211p_txq) {

}*/

static void ieee80211p_tx(struct ieee80211_hw *hw, struct sk_buff *skb) {
	
	/* Get driver's private data */
	struct ieee80211p_device_priv *priv = hw->priv;

	/* Get the number of the TX queue */
	int qnum = skb_get_queue_mapping(skb);

	if (qnum >= IEEE80211P_NUM_TXQ) {
		printk(KERN_ERR "ieee80211p_tx: wrong queue number hw\n");
		dev_kfree_skb_any(skb);
		return;
	}

	//ieee80211p_tx_queue(hw,skb,&priv->txqs[qnum]);
}


static int ieee80211p_start(struct ieee80211_hw *hw) {

	/* Nothing to be done here */	
	
	return 0;
}


static void ieee80211p_stop(struct ieee80211_hw *hw) {

	/* Nothing to be done here */

}


static int ieee80211p_add_interface(struct ieee80211_hw *hw, struct ieee80211_vif *vif) {
	return 0;
}


static void ieee80211p_remove_interface(struct ieee80211_hw *hw, struct ieee80211_vif *vif) {
}


static int ieee80211p_config(struct ieee80211_hw *hw, u32 changed) {
	return 0;
}


static void ieee80211p_configure_filter(struct ieee80211_hw *hw, unsigned int changed_flags, unsigned int *new_flags, u64 multicast) {
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

/*****************************************************************************
 *
 * Initialization and release functions
 *
 ****************************************************************************/

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
}

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
}

/*****************************************************************************
 *
 * Driver and module entry point
 *
 *****************************************************************************/

/* Driver's private data */	
static struct ieee80211p_device_priv priv;

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
}

/* Called at rmmod */
static void __exit ieee80211p_exit(void)
{
	ieee80211p_driver_stop(&priv);	

	printk(KERN_DEBUG "ieee80211p_init: ieee80211p driver removed\n");
}

module_exit(ieee80211p_exit);
module_init(ieee80211p_init);
