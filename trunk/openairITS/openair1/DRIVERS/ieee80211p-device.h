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

#ifndef __ieee80211p_device_h__
#define __ieee80211p_device_h__

#include <linux/spinlock_types.h>
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

struct ieee80211p_txq {
	struct list_head txq; //Transmit buffer
	spinlock_t txqlock; //Lock
	int txq_len; //Number of queued buffers
	int txq_max; //Max allowed num of queue buffers
};/* struct ieee80211p_txq */

/*RX related struct*/
struct oai_rx_status{
	u16     rs_datalen;	//frame data lenght
	u16     rs_tstamp;	//time stamp used to compute the MAC time
	u8      rs_status;	//errors status (CRC, PHY, fifo, decryption, MIC errors) used to check if continue frame processing or not
	u8      rs_phyerr;
	s8      rs_rssi;
	u8      rs_keyix;	//key index for the decryption check
	u8      rs_rate;
	u8      rs_antenna;
	u8      rs_more;	//jumbo frame errors
};

struct oai_rx_buf{
	
	struct oai_rx_status 	rs;
	uint8_t  		skb[IEEE80211_MAX_FRAME_LEN];

};/*end RX related struct*/

struct ieee80211p_device_priv {
	
	/* Configuration and hardware information for an 802.11 PHY */	
	struct ieee80211_hw *hw;	
		
	struct oai_rx_buf *rx_buf_addr;		//the rx buffer DMA mapped. should contain the physical address the DMA space allocated  
	
	/* TX */
	struct ieee80211p_txq txqs[IEEE80211P_NUM_TXQ];	
	struct tasklet_struct txtq;
	bool tx_pending; //TX tasklet pending
	
	/* RX */
	//struct list_head rxq; //Receive buffer
	spinlock_t rxqlock;	
	struct tasklet_struct rxtq;
	bool rx_pending; //RX tasklet pending

};/* ieee80211p_device_priv */

/* ieee80211p device related routines */

int ieee80211p_device_init(struct ieee80211p_device_priv *priv);
void ieee80211p_device_exit(struct ieee80211p_device_priv *priv);

#endif /* __ieee80211_device_h__ */
