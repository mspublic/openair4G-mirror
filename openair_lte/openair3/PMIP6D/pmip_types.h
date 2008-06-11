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
} __identifier;

typedef struct {
	__u32 first;
	__u32 second;
} __timestamp;



struct ip6_mh_opt_mn_identifier {     /*Mobile node Interface Identifier option*/
	__u8	ip6mnid_type;
	__u8	ip6mnid_len;
	__u16	ip6mnid_flags;
	__identifier mn_identifier;      /*mobile interface identifier*/
} __attribute__ ((__packed__));

struct ip6_mh_opt_time_stamp {	   /*Timestamp option*/
	__u8 ip6mots_type;
	__u8 ip6mots_len;
	__timestamp time_stamp;
} __attribute__ ((__packed__));

struct ip6_mh_link_local_add {
	__u8	ip6link_type;
	__u8	ip6link_len;
	struct in6_addr ip6link_addr;   /* Link Local Address*/
} __attribute__ ((__packed__));


struct ip6_mh_opt_serv_mag_addr {
	__u8	ip6sma_type;
	__u8	ip6sma_len;
	__u16	ip6sma_reserved;
	struct in6_addr serv_mag_addr;   /* Serving MAG Address*/
} __attribute__ ((__packed__));


// Define the Proxy Binding Request message structure

struct ip6_mh_proxy_binding_request {
	struct ip6_mh	ip6mhpbrr_hdr;
	__u16	ip6mhpbrr_seqno;        /* Sequence Number */
	/* Followed by optional Mobility Options */
} __attribute__ ((packed));


struct ip6_mh_proxy_binding_response {
	struct ip6_mh	ip6mhpbre_hdr;
	__u16	ip6mhpbre_seqno;		/* Sequence Number */
	/* Followed by optional Mobility Options */
} __attribute__ ((packed));



#endif
