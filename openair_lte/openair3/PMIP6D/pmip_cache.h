/*****************************************************************
 * C header: pmip_cache
 * Description: 
 * Author: 
 *   Christian Bonnet
 *   Huu-Nghia Nguyen
 *   Hussain & Daniel
 * Copyright: Eurecom Institute, (C) 2008
 ******************************************************************/

#ifndef __pmip_cache_h
#define __pmip_cache_h
#include "tqueue.h"
#include "hash.h"
#include "pmip_types.h"

struct pmip_entry {
	struct in6_addr mn_prefix;	/* Network Address Prefix for MN */
	struct in6_addr our_addr;	/* Address to which we got BU */
	struct in6_addr mn_iid;	/* MN IID */
	struct in6_addr mn_addr;	/* Full MN Address */
	struct in6_addr mn_serv_mag_addr;  /* Serving MAG Address */
	struct in6_addr mn_serv_lma_addr;
	struct in6_addr mn_link_local_addr;	/* Link Local Address  for MN */
	struct timespec add_time;       /* When was the binding added or modified */
	struct timespec lifetime;      	/* lifetime sent in this BU, in seconds */
	uint16_t seqno_in;			/* sequence number for response messages */
	uint16_t seqno_out;			/* sequence number for created messages */
	uint16_t PBU_flags;			/* PBU flags */
	uint8_t PBA_flags;			/* PBA flags */
	int type;			/* Entry type */
	int unreach;			/* ICMP dest unreach count */
	int tunnel;			/* Tunnel interface index */
	int link;			/* Home link interface index */
	
/* PBU/PBRR message for retransmissions */	
	struct iovec mh_vec[7];
	int iovlen;
	
/* info_block status flags */	
	uint8_t status;
	ip6ts timestamp;
	uint32_t msg_event;

/* Following fields are for internal use only */
	struct timespec br_lastsent;	/* BR ratelimit */
	int br_count;			/* BR ratelimit */
	int n_rets_counter;		/* Counter for N retransmissions before deleting the entry */
	pthread_rwlock_t lock;		/* Protects the entry */ 
	struct tq_elem tqe;		/* Timer queue entry for expire */

	void (*cleanup)(struct pmip_entry *bce); /* Clean up bce data */
};

//Dedicated to PMIP cache
#define BCE_NO_ENTRY (-1)
#define BCE_PMIP 5
#define BCE_TEMP 6 
#define BCE_HINT 7
#define BCE_CN 8

int 				pmip_cache_init(void);					/**Initializes the Proxy Binding Cache */
struct pmip_entry *	pmip_cache_alloc(int type);			/**Allocated memory for PMIP Entry */
struct pmip_entry * pmip_cache_add(struct pmip_entry *bce); 	
struct pmip_entry *	pmip_cache_get(const struct in6_addr *our_addr,const struct in6_addr *peer_addr); /** returns the entry */
int 				pmip_cache_start(struct pmip_entry *bce); 			/** start the PMIP cache entry */
void 				pmipcache_release_entry(struct pmip_entry *bce);
int 				pmip_cache_exists(const struct in6_addr *our_addr, const struct in6_addr *peer_addr);
void 				pmipcache_free(struct pmip_entry *bce);
void 				pmip_cache_delete(const struct in6_addr *our_addr,const struct in6_addr *peer_addr);

#endif //__pmip_cache_h
