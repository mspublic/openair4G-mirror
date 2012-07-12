/******************************************************************************
 *
 * Copyright(c) EURECOM / Thales Communications & Security
 *
 * Portions of this file are derived from the Atheros ath5k project.
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
 * Thales Communications & Security <philippe.agostini@thalesgroup.com>
 * EURECOM <marouane.amamou@eurecom.fr>
 *
 *****************************************************************************/

#include "ieee80211p-device.h"

/******************************************************************************
 * 
 * Device related routines 
 *
 *****************************************************************************/

/*********** 
 * TX path *
 ***********/

static void ieee80211p_tasklet_tx(unsigned long data) {
	
	/* Driver's private data */	
	struct ieee80211p_device_priv *priv = (void *)data;

	/* Configuration and hardware information for an 802.11 PHY */
	struct ieee80211_hw *hw = priv->hw;
	
	/* Loop variable */
	int i;

	/* Return value */
	int ret = 0;
	
	/* Element of a queue */	
	struct ieee80211p_skbqueue *skbqueue;
	struct ieee80211p_skbqueue *next;

	/* Lock */	
	spin_lock(&priv->txq_lock);

	/* Check if there's still pending queues */	
	if (priv->tx_pending == TRUE) {

		/* For each queues */		
		for(i=0;i<IEEE80211P_NUM_TXQ;i++) {

			/* TODO : check if priv->txqs[i].queue.list is empty ? */

			/* TODO : check if skbqueue->skb == NULL ? */

			list_for_each_entry_safe(skbqueue,next,&priv->txqs[i].queue.list,list) {
				/* TODO : Send the skb to the hw */

				/* If the hw is still busy */
				if (ret == -1) {
					/* Free the skb */
					dev_kfree_skb_any(skbqueue->skb);
					printk(KERN_ERR "ieee80211p_tx_queue: frame dropped\n");					
				} 
				/* The transmission went fine */				
				else {
					ieee80211_tx_status(hw,skbqueue->skb);
				}

				/* Del the element from the queue*/
				list_del(&skbqueue->list);
			} /* end list_for_each_entry_safe */
		} /* end for(i=0;i<IEEE80211P_NUM_TXQ;i++) */
	} /* end if (priv->tx_pending == TRUE) */

	/* There's more pending skb */	
	priv->tx_pending = FALSE;

	/* Lock */	
	spin_unlock(&priv->txq_lock);

} /* ieee80211p_tasklet_tx */

/*********** 
 * RX path *
 ***********/

void remove_padding(struct sk_buff *skb){
	
	int padpos ;
	int padsize;

	/*padding position computation*/
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	__le16 frame_control = hdr->frame_control;
	padpos = 24;

	if (ieee80211_has_a4(frame_control))
		padpos += ETH_ALEN;

	if (ieee80211_is_data_qos(frame_control))
		padpos += IEEE80211_QOS_CTL_LEN;
	/*end padpos computation*/

	padsize  = padpos & 3; //padding size computation
	
	if (padsize && skb->len >= padpos + padsize) {
		memmove(skb->data + padsize, skb->data, padpos);
		skb_pull(skb, padsize);
		
	}
	
} /* ieee80211p_remove_padding */

u16 find_rate_idx(struct ieee80211p_device_priv *priv,
							enum ieee80211_band band, u16 bitrate) {

	/* Data rate index  */
	int rate_idx = -1;

	/* Loop variable */
	int i;

	struct wiphy *wiphy = priv->hw->wiphy;

	/* We look for the idx of the RX bitrate in the bitrates of the band */
	for (i=0;i<wiphy->bands[band]->n_bitrates;i++) {
		if (wiphy->bands[band]-> bitrates[i].bitrate == bitrate) {
			rate_idx = i;
		}
	}

	return rate_idx;

} /* ieee80211p_find_rate_idx */

static void ieee80211p_tasklet_rx(unsigned long data) {
	
	/* Driver's private data */	
	struct ieee80211p_device_priv *priv = (void *)data;	

	/* RX status */	
	struct ieee80211_rx_status *rxs;
	struct ieee80211p_rx_status rs ;

	/* RX buffer = RX status + data */
	struct ieee80211p_rx_buf *rx_buf;

	/* skb */	
	char *skb_addr; //rx data addr
	struct sk_buff *skb;
	u32 skb_len = IEEE80211_MAX_FRAME_LEN;
	
	/* lock */	
	spin_lock(&priv->rxq_lock);

	/****************
	 * skb creation *
	 ****************/
							
	/* skb allocation */	
	skb = __dev_alloc_skb(skb_len,GFP_ATOMIC);
	if (!skb) {
		printk(KERN_ERR "ieee80211p_tasklet_rx: skb allocation failed\n");
		goto error;
	}		

	/* Check if the address of the RX buffer has been initialized */
	if (priv->rx_buf.buf == NULL) {
		printk(KERN_ERR "ieee80211_tasklet_rx: RX buffer not initialized\n");
		dev_kfree_skb_any(skb);
		goto error;
	}

	/* Get the RX buffer and RX status */
	rx_buf = &priv->rx_buf;
	skb_addr = rx_buf->buf;
	rs = rx_buf->status;

	/* Copy the RX data in the skb */	
	memcpy(skb_put(skb,rs.data_len),skb_addr,rs.data_len);

	//TODO check if remove_padding(skb); is needed
 
	/*********
	 * Stats *
	 *********/

	rxs = IEEE80211_SKB_RXCB(skb);

	rxs->freq = priv->cur_chan->center_freq;	//center frequency in MHz
	rxs->signal = rs.rssi;	//rssi provided by the board in dBm
	rxs->band = rs.band;
	rxs->flag = 0;
	rxs->rate_idx = find_rate_idx(priv,rxs->band,rs.rate);

	if (rxs->rate_idx == -1) {
		printk(KERN_ERR "ieee80211_tasklet_rx: unknown data rate\n");
		dev_kfree_skb_any(skb);
		goto error;
	}

	if (rs.flags & IEEE80211P_MMIC_ERROR) {
		rxs->flag |= RX_FLAG_MMIC_ERROR;
	}
	if (rs.flags & IEEE80211P_FAILED_FCS_CRC) {
		rxs->flag |= RX_FLAG_FAILED_FCS_CRC;
	}
	if (rs.flags & IEEE80211P_FAILED_PLCP_CRC) {
		rxs->flag |= RX_FLAG_FAILED_PLCP_CRC;
	}
	if (rs.flags & IEEE80211P_MACTIME_MPDU) {
		rxs->flag |= RX_FLAG_MACTIME_MPDU;
	}
	if (rs.flags & IEEE80211P_NO_SIGNAL_VAL) {
		rxs->flag |= RX_FLAG_NO_SIGNAL_VAL;
	}

	/* Give skb to the mac80211 driver */
	ieee80211_rx(priv->hw, skb);

error:
	/* unlock */
	spin_unlock(&priv->rxq_lock);

} /* ieee80211p_tasklet_rx */

/********
 * Init *
 ********/

static int reg_copy_regd(const struct ieee80211_regdomain **dst_regd,
			const struct ieee80211_regdomain *src_regd) {

	struct ieee80211_regdomain *regd;
	int size_of_regd = 0;
	int ret = 0;
	int i = 0;

	size_of_regd = sizeof(struct ieee80211_regdomain) + ((src_regd->n_reg_rules + 1)*(sizeof(struct ieee80211_reg_rule)));

	regd = kzalloc(size_of_regd, GFP_KERNEL);

	if (!regd) {
		ret = -1;
		goto error;
	}

	memcpy(regd,src_regd,sizeof(struct ieee80211_regdomain));

	for (i=0;i<src_regd->n_reg_rules;i++) {
		memcpy(&regd->reg_rules[i],&src_regd->reg_rules[i],
			sizeof(struct ieee80211_reg_rule));
	}

	*dst_regd = regd;

error:
	return ret;

} /* reg_copy_regd */

int ieee80211p_device_init(struct ieee80211p_device_priv *priv) {	

	/* Configuration and hardware information for an 802.11 PHY */	
	struct ieee80211_hw *hw = priv->hw;
	struct wiphy *wiphy = hw->wiphy; 	

	/* Return value */
	int ret = 0;
	
	/* Loop variable */
	int i = 0;

	/* Test MAC address */	
	char mac_address[ETH_ALEN] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};

	/******************************
	 * Initializing hardware data *
	 ******************************/
	
	/* Received signal power is given in dBm */	
	hw->flags = IEEE80211_HW_SIGNAL_DBM | IEEE80211_HW_DOT11OCB_SUPPORTED;

	/* Headroom to reserve in each transmit skb */	
	hw->extra_tx_headroom = 0;

	/* Number of available hardware queues */
	hw->queues = IEEE80211P_NUM_TXQ;

	/* Virtual interface's private data size */
	hw->vif_data_size = sizeof(struct ieee80211p_vif_priv);

	/***************************
	 * Initializing wiphy data *
	 ***************************/
	
	/* We have our own regulatory domain */
	//ret = reg_copy_regd(&wiphy->regd,&regd);

	if (ret == -1) {
		printk(KERN_ERR "ieee80211p_device_init: reg domain copy failed\n");
		goto error;
	}
	
	/* Set MAC address to hw->wiphy->perm_addr */
	SET_IEEE80211_PERM_ADDR(hw,&mac_address[0]);

	/* Set interface mode */
	wiphy->interface_modes = BIT(NL80211_IFTYPE_ADHOC);
	
	/* Describes the frequency bands a wiphy is able to operate in */
	wiphy->bands[IEEE80211_BAND_2GHZ] = &bands;

	/* Enable ieee 80211.p mode */
	wiphy->dot11OCBActivated = 1;

	/***********************************
	 * Initilizing driver private data *
	 ***********************************/
	
	/* Lock */
	spin_lock_init(&priv->lock);

	/* TX queues setup */
	spin_lock_init(&priv->txq_lock);

	for (i=0;i<IEEE80211P_NUM_TXQ;i++) {
		INIT_LIST_HEAD(&priv->txqs[i].queue.list);	
		priv->txqs[i].queue_len = 0;
		priv->txqs[i].queue_max = IEEE80211P_TXQ_LEN_MAX;
	}

	tasklet_init(&priv->tx_tq,ieee80211p_tasklet_tx,(unsigned long)priv);

	/* RX queues setup */
	spin_lock_init(&priv->rxq_lock);
	tasklet_init(&priv->rx_tq,ieee80211p_tasklet_rx,(unsigned long)priv);

	/* RX buffer init */
	priv->rx_buf.buf = kzalloc(IEEE80211_MAX_FRAME_LEN,GFP_KERNEL);

	/* Virtual interfaces init */
	priv->nvifs = 0;

	/* Current channel init */
	/* The default current channel is the 1st one in the band */
	priv->cur_chan = &bands.channels[0];

	/* Power level init */
	/* Default value =  max power level in the default curent channel */
	priv->cur_power = priv->cur_chan->max_power;

	/* Data rate init */
	/* Default value = first bitrate of the band */
	priv->cur_datarate = bands.bitrates[0].bitrate;

error:
	return ret;

} /* ieee80211p_device_init */

/********
 * Exit *
 ********/

void ieee80211p_device_exit(struct ieee80211p_device_priv *priv) {

	/*********************************
	 * Freeing driver's private data *
	 *********************************/

	tasklet_kill(&priv->tx_tq);
	tasklet_kill(&priv->rx_tq);

	kfree(priv->rx_buf.buf);

} /* ieee80211_device_exit */



