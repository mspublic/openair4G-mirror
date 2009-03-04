/*****************************************************************
 * C header: pmip_dcache
 * Description: 
 * Author: 
 *   Christian Bonnet
 *   Huu-Nghia Nguyen
 * Copyright: Eurecom Institute, (C) 2008
 ******************************************************************/

#ifndef __pmip_dcache_h
#define __pmip_dcache_h

#include "tqueue.h"
#include "hash.h"

struct peer_list 
{
  int count;
  int max;
  struct in6_addr * peers;
};

struct pmip_ro_entry {

	struct in6_addr sender_addr;	/* Address of the PBREQ's or NS's Sender which trigger the creation of this entry */

	struct in6_addr src_prefix;	/* Network Address Prefix for Src MN */
	struct in6_addr src_addr;	/* Src MN addr */
	struct in6_addr src_serv_mag_addr;  /* Serving MAG Address of CN */
	struct in6_addr src_serv_lma_addr;  /* Serving LMA Address of CN */

	struct in6_addr dst_iid;	/* CN addr */
	struct in6_addr dst_prefix;	/* Network Address Prefix for CN */
	struct in6_addr dst_addr;	/* CN addr */
	struct in6_addr dst_serv_mag_addr;  /* Serving MAG Address of CN */
	struct in6_addr dst_serv_lma_addr;	/* Serving LMA Address of CN */

	struct timespec add_time;       /* When was the entry added or modified */
	struct timespec lifetime;      	/* lifetime for this route optimization entry, in seconds */
	uint16_t seqno;			/* outstanding sequence number for the last PBREQ message */

	int tunnel;			/* Tunnel interface index */
	int iif;			/* Interface index of message which create this RO entry */
    int action;                     /* Current outstanding action */
  
	pthread_rwlock_t lock;		/* Protects the entry */ 
	struct tq_elem tqe;		/* Timer queue entry for expire */
	void (*cleanup)(struct pmip_ro_entry* de); /* Clean up destination data */
};

/* Initialization, allocate & free */
int 	pmip_ro_cache_init(void);		
struct 	pmip_ro_entry* pmip_ro_cache_alloc();			/**Allocated memory for dstination entry */
void 	pmip_ro_cache_free(struct pmip_ro_entry* de);

/* add, get, delete, release operation */
struct pmip_ro_entry* pmip_ro_cache_add(struct pmip_ro_entry* de);
void 	pmip_ro_cache_delete(const struct in6_addr* src_addr, const struct in6_addr* dst_addr); 	
struct 	pmip_ro_entry* pmip_ro_cache_get(const struct in6_addr* src_addr, const struct in6_addr* dst_addr); /** returns the dstination entry */
int 	pmip_ro_cache_exists(const struct in6_addr* src_addr, const struct in6_addr* dst_addr);
void 	pmip_ro_cache_release_entry(struct pmip_ro_entry* de);

/* Start timer */
int 	pmip_ro_cache_start(struct pmip_ro_entry* de); 			/** start the dst cache entry */

extern 	pthread_rwlock_t pmip_ro_cache_lock; /* Protects dstination cache */

#endif //__pmip_dcache_h
