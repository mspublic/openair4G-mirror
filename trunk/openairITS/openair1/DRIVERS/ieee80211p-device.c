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

#include "ieee80211p-device.h"

/*********************************************
 * 										
 * Regulatory domain structures and routines 
 *										
 *********************************************/

/* Regulatory domain */ 
static struct ieee80211_regdomain regd = {
	.n_reg_rules = 1,
	.alpha2 = "01",
	.dfs_region = 0,
	.reg_rules = {
		/* start freq / end freq / bandwidth / gain / eirp / flags */
		REG_RULE(0,5000,40,0,47,0),
	}
};

/* Regulatory domain copy */
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
}

/****************************************
 * 										
 * ieee80211p device related structures 
 *										
 ****************************************/

/* Supported channel */
static struct ieee80211_channel channels = {
	.band = IEEE80211_BAND_2GHZ,
	.center_freq = 2000,
	.hw_value = 0,
	.flags = 0,
	.max_antenna_gain = 0,
	.max_power = 20,
	.max_reg_power = 20,
	.beacon_found = FALSE,
	.orig_flags = 0,
	.orig_mag = 0,
	.orig_mpwr = 0,
};


/* Supported bitrate */
static struct ieee80211_rate bitrates = {
	.flags = 0,
	/* bitrate in units of 100 Kbps */
	.bitrate = 10,
	.hw_value = 0,
	.hw_value_short = 0,
};

/* Supported band (channel + bitrate) */
static struct ieee80211_supported_band bands = {
	.channels = &channels,
	.bitrates = &bitrates,
	.band = IEEE80211_BAND_2GHZ,
	.n_channels = 1,
	.n_bitrates = 1,
	.ht_cap.ht_supported = FALSE,
};

/**************************************
 * 
 * ieee80211p device related routines 
 *
 **************************************/

/* TX handler */
static void ieee80211p_tasklet_tx(unsigned long data) {
}

/* RX handler */

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

	padsize  = padpos & 3;	//padding size computation
	
	if (padsize && skb->len >= padpos + padsize) {
		memmove(skb->data + padsize, skb->data, padpos);
		skb_pull(skb, padsize);
		
	}

	
}/*end remove padding*/



static void ieee80211p_tasklet_rx(unsigned long data) {
struct oai_rx_status rs ;
	struct oai_rx_buf *bf;
	struct sk_buff *skb;
	void *skb_addr;  //????????????????
	struct ieee80211p_device_priv *priv = (void *)data;
	//struct ieee80211p_device_buf *bf;
	
	struct ieee80211_rx_status *rxs;	//rx status to update for the received frame
	
	u32 skb_len;

	//int ret;
	
	
	spin_lock(&priv->rxqlock);
	skb_len = IEEE80211_MAX_FRAME_LEN;		//max_80211_frame_size 2352 (reference 802.11e section 7.1.2)
							

	skb = __dev_alloc_skb(skb_len,GFP_ATOMIC);	//mask to make the operation atomic 
							//TODO check the exact length to allocate to the skb
	if (!skb) {
		printk(KERN_ERR "skbuff alloc of size %u failed\n",skb_len );
	
	}		// TODO check the size allocated-->if needed skb_reserve()


////////////////////////////////////////

	bf = priv->rx_buf_addr;
	rs = bf->rs;
	skb_addr = (void *)&bf->skb;

////////////////////////////////////////

	memcpy(skb_put(skb, skb_len),skb_addr ,skb_len);
	
	remove_padding(skb); //  if needed 
	rxs = IEEE80211_SKB_RXCB(skb);
/*
	rxs->flag = 0;
		rxs->flag |= RX_FLAG_MMIC_ERROR;			//report a MMIC error
*/

/*
//	rxs->mactime = extend_tsf(ah, rs->rs_tstamp);			//not needed for the ieee80211p since synchronization is not done through the timing informations given 									//by the frames

//	rxs->flag |= RX_FLAG_MACTIME_MPDU;				//never set if the mactime is not updated

	rxs->freq = channels.center_freq;				//center frequency in MHz
	rxs->band = channels.band;					//band this channel belongs to

	rxs->signal = rs->rs_rssi;					//rssi provided by the board in dB

//	rxs->antenna = rs->rs_antenna;					//antenna used. not really important since we are using only one antenna


	rxs->rate_idx = index_rate;					// TODO index of data rate into band's supported rates or MCS index if HT rates are use (RX_FLAG_HT)
//	rxs->flag |= RX_FLAG_DECRYPTED;					//add RX_FLAG_DECRYPTED if decryptuion is done correctly in the board. we don't suppor this function


//	rxs->flag |= RX_FLAG_SHORTPRE;					//Short preamble was used for this frame. Not mandatory in our case
*/
	
	ieee80211_rx(priv->hw, skb);	//entry point to the MAC80211


//unlock:
	spin_unlock(&priv->rxqlock);


}

//////PCIe interface function///////////
///////////send the address of the rx_buf to the board
/*
pcie_send_rx_buf_addr(dma_addr_t addr){

}
*/

/////Allocate memory space for the rx_buff used in in DMA transfert from the board//////////
	/////This should be done when starting the board//////
/*
static struct oai_rx_buf *ieee80211p_alloc_rx_buf(){
	priv->rx_buf_addr=(unsigned int*)bigphys_malloc(sizeof(struct oai_rx_buf));	//bigphys_malloc is used to allocate memory. It is defined in openair-rf
	pcie_send_rx_buf_addr(virt_to_phys(priv->rx_buf_addr));
}
*/

int ieee80211p_device_init(struct ieee80211p_device_priv *priv) {
	
	/* Configuration and hardware information for an 802.11 PHY */	
	struct ieee80211_hw *hw = priv->hw;
	struct wiphy *wiphy = hw->wiphy; 	

	/* Loop variable */	
	int i = 0;

	/* Return value */
	int ret = 0;
	
	/* Test MAC address */	
	char mac_address[ETH_ALEN] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};

	/******************************
	 * Initializing hardware data *
	 ******************************/
	
	/* Received frames include FCS field */
	/* Received signal power is given in dBm */	
	hw->flags = IEEE80211_HW_RX_INCLUDES_FCS | IEEE80211_HW_SIGNAL_DBM;

	/* Headroom to reserve in each transmit skb */	
	hw->extra_tx_headroom = 0;

	//hw->channel_change_time = 0;
	//hw->max_signal = 100;
	//hw->max_listen_interval = 5;

	/* Number of available hardware queues */
	hw->queues = IEEE80211P_NUM_TXQ;

	//hw->rate_control_algorithm = NULL;
	//hw->vif_data_size = sizeof(struct ieee80211p_vif_priv);
	//hw->sta_data_size = sizeof(struct ieee80211p_sta_priv);
	//hw->max_rates = 1;
	//hw->max_report_rates = 1;
	//hw->max_rate_tries = 1;
	//hw->napi_weight = 0;
	//hw->max_rx_aggregation = 0;
	//hw->max_tx_aggregation = 0;

	/***************************
	 * Initializing wiphy data *
	 ***************************/

	//wiphy->reg_notifier = &ieee80211p_reg_notifier;

	/* Regulatory domain */
	/* Mandatory parameter to be defined */
	ret = reg_copy_regd(&wiphy->regd,&regd);

	if (ret == -1) {
		goto error;	
	}

	//wiphy->signal_type = CFG80211p_SIGNAL_TYPE_NONE;
	//wiphy->cipher_suites = NULL;
	//wiphy->n_cipher_suites = 0;
	//wiphy->retry_short = 1;
	//wiphy->retry_long = 1;
	//wiphy->frag_threshold = -1;
	//wiphy->rts_threshold = -1;
	//wiphy_net_set(&hw->wiphy,NULL);
	
	/* Set MAC address to hw->wiphy->perm_addr */
	SET_IEEE80211_PERM_ADDR(hw,&mac_address[0]);

	//wiphy->addr_mask
	//wiphy->n_addresses = 0;
	//wiphy->addresses = NULL;
	//wiphy->registered
	//wiphy->debugfsdir
	//SET_IEEE80211_DEV(hw,NULL);
	//wiphy->wext = NULL;

	wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION);

	//wiphy->iface_combinations = NULL;
	//wiphy->n_iface_combinations = 0;
	//wiphy->software_iftypes = 0;
	//wiphy->flags |= WIPHY_FLAG_IBSS_RSN;
	//wiphy->flags = 0;
	//wiphy->features = 0;
	//wiphy->bss_priv_size = sizeof(struct ieee80211p_bss);
	//wiphy->max_scan_ssids = 0;
	//wiphy->max_sched_scan_ssids = 0;
	//wiphy->max_match_sets = 0;
	//wiphy->max_scan_ie_len = 0;
	//wiphy->max_sched_scan_ie = 0;
	//wiphy->coverage_class = 0;
	//wiphy->fw_version
	//wiphy->hw_version = 0;
	//wiphy->max_num_pmkids = 0;
	//wiphy->privid
	
	/* Describes the frequency bands a wiphy is able to operate in */
	/* Mandatory parameter to be defined */
	for (i = 0;i<IEEE80211_NUM_BANDS;i++) {
		wiphy->bands[i] = NULL;
	}
	wiphy->bands[IEEE80211_BAND_2GHZ] = &bands;

	//wiphy->mgmt_stypes = NULL;
	//wiphy->available_antennas_tx = 0x3;
	//wiphy->available_antennas_rx = 0x3;
	//wiphy->probe_resp_offload = 0;
	//wiphy->max_remain_on_channel_duration = 0;
	//wiphy->wowlan.flags = WIPHY_WOWLAN_ANY;
	//wiphy->wowlan.n_patterns = 0;
	//wiphy->wowlan.pattern_max_len = 0;
	//wiphy->wowlan.pattern_min_len = 0;	
	//wiphy->ap_sme_capa = 0;
	//wiphy->ht_capa_mod_mask = NULL;

	/***********************************
	 * Initilizing driver private data *
	 ***********************************/

	/* TX queues setup */		
	for (i=0;i<IEEE80211P_NUM_TXQ;i++) {
		INIT_LIST_HEAD(&priv->txqs[i].txq);	
		spin_lock_init(&priv->txqs[i].txqlock);
		priv->txqs[i].txq_len = 0;
		priv->txqs[i].txq_max = IEEE80211P_TXQ_LEN_MAX;
	}
	tasklet_init(&priv->txtq,ieee80211p_tasklet_tx,(unsigned long)priv);

	/* RX queues setup */
	//INIT_LIST_HEAD(&priv->rxq);
	spin_lock_init(&priv->rxqlock);
	tasklet_init(&priv->rxtq,ieee80211p_tasklet_rx,(unsigned long)priv);

error:
	return ret;
}

void ieee80211p_device_exit(struct ieee80211p_device_priv *priv) {

	/*********************************
	 * Freeing driver's private data *
	 *********************************/

	tasklet_kill(&priv->txtq);
	tasklet_kill(&priv->rxtq);

}



