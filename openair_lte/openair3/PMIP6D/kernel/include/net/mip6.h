/*
 * Copyright (C)2003 Helsinki University of Technology
 * Copyright (C)2003 USAGI/WIDE Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
 * Authors:
 *	Noriaki TAKAMIYA @USAGI
 *	Masahide NAKAMURA @USAGI
 *	YOSHIFUJI Hideaki @USAGI
 */
#ifndef _MIP6_H
#define _MIP6_H

#include <linux/skbuff.h>
#include <linux/in6.h>
#include <net/sock.h>

#ifdef CONFIG_IPV6_MIP6_DEBUG
#define MIP6_DEBUG 3
#else
#define MIP6_DEBUG 2
#endif

#if MIP6_DEBUG >= 3
#define MIP6_DBG(x...) do { printk(KERN_DEBUG x); } while (0)
#else
#define MIP6_DBG(x...) do { ; } while (0)
#endif

extern int mip6_init(void);
extern void mip6_fini(void);
extern int mip6_mh_filter(struct sock *sk, struct sk_buff *skb);
extern int mip6_destopt_place_find(struct sk_buff *skb, u8 **nexthdr);
extern int mip6_rthdr_place_find(struct sk_buff *skb, u8 **nexthdr);

/* XXX: Home Address Option in Destination Option Header */
struct destopt_hao
{
	__u8			type;
	__u8			length;
	struct in6_addr		addr;	/* Home Address */
} __attribute__ ((__packed__));


/*
 * Mobility Header
 */

struct ip6_mh {
	__u8	ip6mh_proto;	/* NO_NXTHDR by default */
	__u8	ip6mh_hdrlen;	/* Header Len in unit of 8 Octets
				   excluding the first 8 Octets */
	__u8	ip6mh_type;	/* Type of Mobility Header */
	__u8	ip6mh_reserved;	/* Reserved */
	__u16	ip6mh_cksum;	/* Mobility Header Checksum */
	/* Followed by type specific messages */
} __attribute__ ((__packed__));

struct ip6_mh_binding_request {
	struct ip6_mh	ip6mhbr_hdr;
	__u16	ip6mhbr_reserved;
	/* Followed by optional Mobility Options */
} __attribute__ ((__packed__));

struct ip6_mh_home_test_init {
	struct ip6_mh	ip6mhhti_hdr;
	__u16	ip6mhhti_reserved;
	__u32	ip6mhhti_cookie[2];	/* 64 bit Cookie by MN */
	/* Followed by optional Mobility Options */
} __attribute__ ((__packed__));

struct ip6_mh_careof_test_init {
	struct ip6_mh	ip6mhcti_hdr;
	__u16	ip6mhcti_reserved;
	__u32	ip6mhcti_cookie[2];	/* 64 bit Cookie by MN */
	/* Followed by optional Mobility Options */
} __attribute__ ((__packed__));

struct ip6_mh_home_test {
	struct ip6_mh	ip6mhht_hdr;
	__u16	ip6mhht_nonce_index;
	__u32	ip6mhht_cookie[2];	/* Cookie from HOTI msg */
	__u32	ip6mhht_keygen[2];	/* 64 Bit Key by CN */
	/* Followed by optional Mobility Options */
} __attribute__ ((__packed__));

struct ip6_mh_careof_test {
	struct ip6_mh	ip6mhct_hdr;
	__u16	ip6mhct_nonce_index;
	__u32	ip6mhct_cookie[2];	/* Cookie from COTI message */
	__u32	ip6mhct_keygen[2];	/* 64bit key by CN */
	/* Followed by optional Mobility Options */
} __attribute__ ((__packed__));

struct ip6_mh_binding_update {
	struct ip6_mh	ip6mhbu_hdr;
	__u16	ip6mhbu_seqno;		/* Sequence Number */
	__u16	ip6mhbu_flags;
	__u16	ip6mhbu_lifetime;	/* Time in unit of 4 sec */
	/* Followed by optional Mobility Options */
} __attribute__ ((__packed__));

struct ip6_mh_binding_ack {
	struct ip6_mh	ip6mhba_hdr;
	__u8	ip6mhba_status;	/* Status code */
	__u8	ip6mhba_flags;
	__u16	ip6mhba_seqno;
	__u16	ip6mhba_lifetime;
	/* Followed by optional Mobility Options */
} __attribute__ ((__packed__));

struct ip6_mh_binding_error {
	struct ip6_mh	ip6mhbe_hdr;
	__u8	ip6mhbe_status;	/* Error Status */
	__u8	ip6mhbe_reserved;
	struct in6_addr	ip6mhbe_homeaddr;
	/* Followed by optional Mobility Options */
} __attribute__ ((__packed__));

/*
 * Mobility Option TLV data structure
 */
struct ip6_mh_opt {
	__u8	ip6mhopt_type;	/* Option Type */
	__u8	ip6mhopt_len;	/* Option Length */
	/* Followed by variable length Option Data in bytes */
} __attribute__ ((__packed__));

/*
 * Mobility Option Data Structures 
 */
struct ip6_mh_opt_refresh_advice {
	__u8	ip6mora_type;
	__u8	ip6mora_len;
	__u16	ip6mora_interval;	/* Refresh interval in 4 sec */
} __attribute__ ((__packed__));

struct ip6_mh_opt_altcoa {
	__u8	ip6moa_type;
	__u8	ip6moa_len;
	struct in6_addr	ip6moa_addr;		/* Alternate Care-of Address */
} __attribute__ ((__packed__));

struct ip6_mh_opt_nonce_index {
	__u8	ip6moni_type;
	__u8	ip6moni_len;
	__u16	ip6moni_home_nonce;
	__u16	ip6moni_coa_nonce;
} __attribute__ ((__packed__));

struct ip6_mh_opt_auth_data {
	__u8 ip6moad_type;
	__u8 ip6moad_len;
	__u8 ip6moad_data[12];	/* 96 bit Authenticator */
} __attribute__ ((__packed__));

struct ip6_mh_opt_mob_net_prefix {
	__u8 ip6mnp_type;
	__u8 ip6mnp_len;
	__u8 ip6mnp_reserved;
	__u8 ip6mnp_prefix_len;
	struct in6_addr ip6mnp_prefix;
} __attribute__ ((__packed__));

/*
 *     Mobility Header Message Types
 */
#define IP6_MH_TYPE_BRR		0	/* Binding Refresh Request */
#define IP6_MH_TYPE_HOTI	1	/* HOTI Message */
#define IP6_MH_TYPE_COTI	2	/* COTI Message */
#define IP6_MH_TYPE_HOT		3	/* HOT Message */
#define IP6_MH_TYPE_COT		4	/* COT Message */
#define IP6_MH_TYPE_BU		5	/* Binding Update */
#define IP6_MH_TYPE_BACK	6	/* Binding ACK */
#define IP6_MH_TYPE_BERROR	7	/* Binding Error */

//Added for PMIP in pmip_consts.h
//  #define IP6_MH_TYPE_PBREQ	8	/* Proxy Binding Request */
//  #define IP6_MH_TYPE_PBRES	9	/* Proxy Binding Response */
//  #define IP6_MH_TYPE_MAX		(IP6_MH_TYPE_PBRES)

/*
 *     Mobility Header Message Option Types
 */
#define IP6_MHOPT_PAD1		0x00	/* PAD1 */
#define IP6_MHOPT_PADN		0x01	/* PADN */
#define IP6_MHOPT_BREFRESH	0x02	/* Binding Refresh */
#define IP6_MHOPT_ALTCOA	0x03	/* Alternate COA */
#define IP6_MHOPT_NONCEID	0x04	/* Nonce Index */
#define IP6_MHOPT_BAUTH		0x05	/* Binding Auth Data */
#define IP6_MHOPT_MOB_NET_PRFX	0x06	/* Mobile Network Prefix */

#endif

