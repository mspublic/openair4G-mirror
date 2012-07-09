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
 * EURECOM EURECOM <marouane.amamou@eurecom.fr>
 *
 *****************************************************************************/

#ifndef __ieee80211p_device_h__
#define __ieee80211p_device_h__

#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/skbuff.h>
#include <linux/gfp.h>
#include <net/mac80211.h>
#include <net/cfg80211.h>

#define ETH_ALEN 6
#define IEEE80211P_NUM_TXQ 1
#define IEEE80211P_TXQ_LEN_MAX 100
#define FALSE 0
#define TRUE 1
#define RX_STATUS_LEN 11

/******************************************************************************
 *
 * Device structures 
 *
 *****************************************************************************/

/****************************** 										
 * Channels / Birates / Bands *
 ******************************/

/* Supported channel */
static struct ieee80211_channel channels = {
	.band = IEEE80211_BAND_2GHZ,
	.center_freq = 2437,
	.hw_value = 0,
	.flags = 0,
	.max_antenna_gain = 3,
	.max_power = 20,
	.max_reg_power = 20,
	.beacon_found = FALSE,
	.orig_flags = 0,
	.orig_mag = 0,
	.orig_mpwr = 0,
};/* struct ieee80211_channel */


/* Supported bitrate */
static struct ieee80211_rate bitrates = {
	.flags = 0,
	/* bitrate in units of 100 Kbps */
	.bitrate = 10,
	.hw_value = 0,
	.hw_value_short = 0,
};/* struct ieee80211_rate */

/* Supported band (channel + bitrate) */
static struct ieee80211_supported_band bands = {
	.channels = &channels,
	.bitrates = &bitrates,
	.band = IEEE80211_BAND_2GHZ,
	.n_channels = 1,
	.n_bitrates = 1,
	.ht_cap.ht_supported = FALSE,
};/* struct ieee80211_supported_band */

/*********** 
 * TX path *
 ***********/

struct ieee80211p_skbqueue {
	struct list_head list;	
	struct sk_buff *skb;
};/* struct ieee80211p_skbqueue */

struct ieee80211p_txq {
	struct ieee80211p_skbqueue queue; //Doubly linked list of struct sk_buff	
	int queue_num; //Number of the queue
	int queue_len; //Number of queued buffers
	int queue_max; //Max allowed num of queue buffers
};/* struct ieee80211p_txq */

/*********** 
 * RX path *
 ***********/

struct ieee80211p_rx_status {
	u16     rs_datalen;	//frame data lenght
	u16     rs_tstamp;	//time stamp used to compute the MAC time
	u8      rs_status;	//errors status (CRC, PHY, fifo, decryption, MIC errors) used to check if continue frame processing or not
	u8      rs_phyerr;
	s8      rs_rssi;
	u8      rs_keyix;	//key index for the decryption check
	u8      rs_rate;
	u8      rs_antenna;
	u8      rs_more;	//jumbo frame errors
}; /* struct ieee80211p_rx_status */

struct ieee80211p_rx_buf {
	struct ieee80211p_rx_status rs;
	char *buf;
}; /* struct ieee80211p_rx_buf */

/*********************
 * Regulatory domain *									
 *********************/

/* Regulatory domain */ 
static struct ieee80211_regdomain regd = {
	.n_reg_rules = 1,
	.alpha2 = "99",
	.dfs_region = 0,
	.reg_rules = {
		/* start freq / end freq / bandwidth / gain / eirp / flags */
		REG_RULE(0,5000,40,0,47,0),
	}
};

/******************************************************************************
 *
 * Device's private data 
 *
 *****************************************************************************/

struct ieee80211p_vif_priv {
	enum nl80211_iftype opmode;
};

struct ieee80211p_device_priv {
	
	/* Configuration and hardware information for an 802.11 PHY */	
	struct ieee80211_hw *hw;	
		
	/* The RX buffer DMA mapped */
	/* Shall contain the physical address the DMA space allocated */
	struct ieee80211p_rx_buf rx_buf; 
	
	/* TX */
	spinlock_t txqlock; //Lock
	struct ieee80211p_txq txqs[IEEE80211P_NUM_TXQ];	
	struct tasklet_struct txtq;
	bool tx_pending; //TX tasklet pending
	
	/* RX */
	spinlock_t rxqlock;	
	struct tasklet_struct rxtq;
	bool rx_pending; //RX tasklet pending

	/* Lock */	
	spinlock_t lock;

	/* Virtual interfaces */
	int nvifs;

	/* Current channel in use */
	struct ieee80211_channel *curchan;

	/* Requested power level in dBm */
	int power_level;

};/* ieee80211p_device_priv */

/******************************************************************************
 * 
 * Device related routines
 *
 *****************************************************************************/

int ieee80211p_device_init(struct ieee80211p_device_priv *priv);
void ieee80211p_device_exit(struct ieee80211p_device_priv *priv);

#endif /* __ieee80211_device_h__ */
