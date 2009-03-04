/*****************************************************************
 * C header: pmip_types.h
 * Description: Describe all types for pmip
 * Author: 
 *   Christian Bonnet
 *   Huu-Nghia Nguyen
 *   Hussain & Daniel
 * Copyright: Eurecom Institute, (C) 2008
 ******************************************************************/

#ifndef __pmip_types_h
#define __pmip_types_h

#include <netinet/in.h>
#include <netinet/ip6mh.h>
#include "mh.h"
#include <linux/types.h>
#include "pmip_consts.h"
#include <inttypes.h>

/*
 * Mobility Option TLV data structure
 *New options defined for Proxy BU & BA
 */
struct ip6_mh_opt_home_net_prefix {     /*Home netowork prefix option*/
	__u8	ip6hnp_type;
	__u8	ip6hnp_len;
	__u8 	ip6hnp_reserved;
	__u8	ip6hnp_prefix_len;      /*prefix length for home network*/
	struct in6_addr ip6hnp_prefix;  /*home address prefix*/
} __attribute__ ((__packed__));


typedef struct {
	__u32 first;
	__u32 second;
} ip6mnid;

typedef struct {
	__u32 first;
	__u32 second;
} ip6ts;



struct ip6_mh_opt_mn_identifier {     /*Mobile node Interface Identifier option*/
	__u8	ip6mnid_type;
	__u8	ip6mnid_len;
	__u16	ip6mnid_flags;
	ip6mnid mn_identifier;      /*mobile interface identifier*/
} __attribute__ ((__packed__));

struct ip6_mh_opt_time_stamp {	   /*Timestamp option*/
	__u8 ip6mots_type;
	__u8 ip6mots_len;
	ip6ts time_stamp;
} __attribute__ ((__packed__));

struct ip6_mh_link_local_add {
	__u8	ip6link_type;
	__u8	ip6link_len;
	struct in6_addr ip6link_addr;   /* Link Local Address*/
} __attribute__ ((__packed__));


// ******** Extended options for cluster based architecture & Route optimiztion ***********

struct ip6_mh_opt_dst_mn_addr {
	__u8	ip6dma_type;
	__u8	ip6dma_len;
	struct in6_addr dst_mn_addr;   /* Destination MN Address */
} __attribute__ ((__packed__));

struct ip6_mh_opt_serv_mag_addr {
	__u8	ip6sma_type;
	__u8	ip6sma_len;
	struct in6_addr serv_mag_addr;   /* Serving MAG Address of the destination */
} __attribute__ ((__packed__));

struct ip6_mh_opt_serv_lma_addr {
	__u8	ip6sla_type;
	__u8	ip6sla_len;
	struct in6_addr serv_lma_addr;   /* Serving LMA Address of the destination */
} __attribute__ ((__packed__));

struct ip6_mh_opt_src_mn_addr {
	__u8	ip6sma_type;
	__u8	ip6sma_len;
	struct in6_addr src_mn_addr;   /* Source MN Address */
} __attribute__ ((__packed__));

struct ip6_mh_opt_src_mag_addr {
	__u8	ip6sma_type;
	__u8	ip6sma_len;	
	struct in6_addr src_mag_addr;   /* Source MAG Address */
} __attribute__ ((__packed__));

// ****************************************************************************************

// Define the Proxy Binding Request message structure

struct ip6_mh_proxy_binding_request {
	struct ip6_mh	ip6mhpbreq_hdr;
	__u16	ip6mhpbreq_seqno;        /* Sequence Number */
	__u16	ip6mhpbreq_action;
	/* Followed by Mobility Options */
} __attribute__ ((packed));


struct ip6_mh_proxy_binding_response {
	struct ip6_mh	ip6mhpbres_hdr;
	__u16	ip6mhpbres_seqno;		/* Sequence Number */
	__u16	ip6mhpbres_status;
	/* Followed by Mobility Options */
} __attribute__ ((packed));

struct msg_info {
	struct in6_addr 		src;
	struct in6_addr 		dst;
	struct in6_addr_bundle	addrs;
	int 					iif;
	uint32_t 				msg_event;

	struct in6_addr mn_iid;	/* MN IID */
	struct in6_addr mn_addr;	/* Full MN Address */
	struct in6_addr mn_prefix;	/* Network Address Prefix for MN */
	struct in6_addr mn_serv_mag_addr;  /* Serving MAG Address */
	struct in6_addr mn_serv_lma_addr;
	struct in6_addr mn_link_local_addr;	/* Link Local Address  for MN */
	struct timespec addtime;       /* When was the binding added or modified */
	struct timespec lifetime;      	/* lifetime sent in this BU, in seconds */
	uint16_t 		seqno;			/* sequence number of the message */
	uint16_t 		PBU_flags;			/* PBU flags */
	uint8_t 		PBA_flags;			/* PBA flags */
	ip6ts 			timestamp;
	uint16_t		PBREQ_action;
	uint16_t 		PBRES_status;

	//Route optimization or flow control
	struct in6_addr src_mag_addr;  /* Source MAG Address */
	struct in6_addr src_mn_addr;  /* Source MN Address */

	struct in6_addr na_target;
	struct in6_addr ns_target;
	int		is_dad; // is NS used for DAD process?	
	int		hoplimit;	
} msg_info;





#endif
