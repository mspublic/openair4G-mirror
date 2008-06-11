/*****************************************************************
 * C Implementation: pmip6d.c
 * Description: 
 * Author: 
 *   Christian Bonnet
 *   Huu-Nghia Nguyen
 *   Hussain & Daniel
 * Copyright: Eurecom Institute,(C) 2008
 ******************************************************************/
#include "pmip_extern.h"
#include "bcache.h"
#include <netinet/in.h>
#include "debug.h"
#include "conf.h"
#include "pmip_types.h"
#include "hash.h"
#include "util.h"
#include "pmip_consts.h"
#include "icmp6.h"
#include "mh.h"
#include "cn.h"
#include <netinet/ip6mh.h>
#include <pthread.h>
#include <errno.h>
#include "xfrm.h"
#include "tunnelctl.h"
#include "keygen.h"
#include "vt.h"
#include "pmip_cache.h"

/**
 *  Some defined Macros for ID & ADDRESS conversions.
 **/

//ADDR2ID converts an address into an ID.
struct in6_addr *ADDR2ID(struct in6_addr *addr,int plen)
{
	struct in6_addr id=in6addr_any;
	if(plen ==64)
	{
		memcpy(&id.in6_u.u6_addr32[2], addr->in6_u.u6_addr32[2],sizeof(__identifier));
	}
	return &id;
}



//NUD_ADDR converts an ID into a Link Local Address!
struct in6_addr *link_local_addr(struct in6_addr *id)
{
	struct in6_addr ADDR =in6addr_any;

	ADDR.s6_addr32[0] = htonl(0xfe800000);

	//copy the MN_ID.
	memcpy(&ADDR.in6_u.u6_addr32[2],&id->in6_u.u6_addr32[2],sizeof(__identifier));

	return &ADDR;
}


/**
 *  PMIP binding cache functions.
 **/


#define PMIPCACHE_BUCKETS 32
static struct hash pmip_hash;

static int pmip_cache_count = 0;

pthread_rwlock_t pmip_lock; /* Protects proxy binding cache */

/** 
 * get_pmip_cache_count - returns number of home and cache entries
 * @type: PMIP or TEMP
 **/
int get_pmip_cache_count(int type)
{
	if (type == BCE_PMIP || type == BCE_TEMP)
		return pmip_cache_count;
	return 0;
}

void dump_pbce(void *bce, void *os)
{
	struct pmip_entry *e = (struct pmip_entry *)bce;
	FILE *out = (FILE *)os;

	fprintf(out, " == Proxy Binding Cache entry ");

	switch(e->type) {
	case BCE_PMIP:
		fprintf(out, "(BCE_PMIP)\n");
		break;
	case BCE_TEMP:
		fprintf(out, "(BCE_TEMP)\n");
		break;
	
	default:
		fprintf(out, "(Unknown)\n");
	}
	fprintf(out, " Peer_addr:    %x:%x:%x:%x:%x:%x:%x:%x\n",
		NIP6ADDR(&e->peer_addr));
	fprintf(out, " Serv_MAG_addr:%x:%x:%x:%x:%x:%x:%x:%x\n", 
	     NIP6ADDR(&e->Serv_MAG_addr));
	fprintf(out, " LMA_addr:     %x:%x:%x:%x:%x:%x:%x:%x\n",
		NIP6ADDR(&e->LMA_addr));
	fprintf(out, " lifetime %ld\n ", e->lifetime.tv_sec);
	fprintf(out, " seqno %d\n", e->seqno);

	fflush(out);
}

//initializes the PMIP cache.
int pmip_cache_init(void)
{
	int ret;

	if (pthread_rwlock_init(&pmip_lock, NULL))
		return -1;

	pthread_rwlock_wrlock(&pmip_lock);
	ret = hash_init(&pmip_hash, DOUBLE_ADDR, PMIPCACHE_BUCKETS);
	pthread_rwlock_unlock(&pmip_lock);

	#ifdef ENABLE_VT
	if (ret < 0)
		return ret;
	 ret = vt_pbc_init();
	#endif

	return ret;
}

/**
 * pmipcache_alloc - allocate binding cache entry
 * @type: type of entry
 *
 * Allocates a new binding cache entry. Returns allocated space for an entry or NULL if none
 * available.
 **/

struct pmip_entry *pmip_cache_alloc(int type)
{
	struct pmip_entry *tmp;
	
	tmp = malloc(sizeof(struct pmip_entry));

	if (tmp == NULL){
		dbg("NO memory allocated for PMIP entry..\n");
		return NULL;
}
	if (pthread_rwlock_init(&tmp->lock, NULL)) {
		free(tmp);
		return NULL;
	}
	memset(tmp, 0, sizeof(*tmp));
	INIT_LIST_HEAD(&tmp->tqe.list);
	dbg("PMIP cache entry is allocated..\n");
	return tmp;
}



static int __pmipcache_insert(struct pmip_entry *bce)
{
	int ret;
	ret = hash_add(&pmip_hash, bce, &bce->our_addr, &bce->peer_addr);
	if (ret)
		return ret;

	pmip_cache_count++;
	dbg("PMIP cache entry is inserted..\n");
	return 0;
}

//PMIP cache start
int pmip_cache_start(struct pmip_entry *bce)
{
	
	if(is_mag()){

		dbg("PMIP cache start triggered.. \n"); 
		struct timespec expires;
		clock_gettime(CLOCK_REALTIME, &bce->add_time);
		tsadd(bce->add_time, bce->lifetime, expires);
		add_task_abs(&expires, &bce->tqe, _EXPIRED);
		dbg("Expiry Timer for PMIP cache entry is triggered.. \n"); 
		return 0;
	}
	if(is_lma()){

		dbg("PMIP cache start triggered.. \n"); 
		struct timespec expires, lifetime;
		clock_gettime(CLOCK_REALTIME, &bce->add_time);
		//Add additional time for bce entry expriy at CH side.(To Reduce signalling!)
		tsadd(bce->lifetime, conf.CH_bce_expire, lifetime);
		tsadd(bce->add_time, lifetime, expires);
		add_task_abs(&expires, &bce->tqe, _EXPIRED);
		dbg("Expiry Timer for PMIP cache entry is triggered.. \n"); 
		return 0;
	}
}

struct pmip_entry * pmip_cache_add(struct pmip_entry *bce) /** return the added new entry */
{
	int ret = 0;

	dbg("Add cache entry for: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&bce->peer_addr));
	dbg("PMIP cache entry type: %d\n",bce->type);

	assert(bce);
	bce->unreach = 0;
	pthread_rwlock_wrlock(&pmip_lock);
	if ((ret = __pmipcache_insert(bce)) != 0) {
		pthread_rwlock_unlock(&pmip_lock);
		dbg("PMIP ENTRY NOT INSERTED..\n");
		return ret;
	}
	
	bce->n_rets_counter = conf.Max_Rets;
	dbg("Retransmissions counter intialized: %d\n",bce->n_rets_counter);
	if(bce->type == BCE_PMIP){
		pmip_cache_start(bce);
	}
	
	pthread_rwlock_unlock(&pmip_lock);

	return bce;		
	
}




struct pmip_entry *pmip_cache_get(const struct in6_addr *our_addr,const struct in6_addr *peer_addr)
{
	struct pmip_entry *bce;

	assert(peer_addr && our_addr);

	pthread_rwlock_rdlock(&pmip_lock);

	bce = hash_get(&pmip_hash, our_addr, peer_addr);

	if (bce){ 
		pthread_rwlock_wrlock(&bce->lock);
		dbg("PMIP cache entry is found for: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&bce->peer_addr));
		dbg("PMIP cache entry type: %d\n",(bce->type));
	}
	else{
		pthread_rwlock_unlock(&pmip_lock);
		dbg("NO PMIP cache entry found...\n");
		
	}

	return bce;	

}

/**
 * pmip cache_release_entry - unlocks a binding cache entry 
 **/
void pmipcache_release_entry(struct pmip_entry *bce)
{
	assert(bce);
	pthread_rwlock_unlock(&bce->lock);
	pthread_rwlock_unlock(&pmip_lock);
	dbg("PMIP cache entry is released...\n");
}

int pmip_cache_exists(const struct in6_addr *our_addr, const struct in6_addr *peer_addr)
{
        struct pmip_entry *bce;
        int type;

        bce = pmip_cache_get(our_addr, peer_addr);

        if (bce == NULL)
                return -1;
	
	dbg("PMIP cache does exist with type: %d\n",(bce->type));
        type = bce->type;
        pmipcache_release_entry(bce);

        return type;
}


/**
 * pmip cache_free - release allocated memory
 * @bce: BC entry to free
 *
 * Release allocated memory back to unused pool.
 **/
void pmipcache_free(struct pmip_entry *bce)
{
	/* This function should really return allocated space to free
	 * pool. */
	pthread_rwlock_destroy(&bce->lock);
	free(bce);
	dbg("Free PMIP cache entry...\n");
}

void pmip_bce_delete(struct pmip_entry *bce)
{
	pthread_rwlock_wrlock(&bce->lock);
	del_task(&bce->tqe);
	
	if (bce->cleanup)
		bce->cleanup(bce);

	pmip_cache_count--;
	hash_delete(&pmip_hash, &bce->our_addr, &bce->peer_addr);
	pthread_rwlock_unlock(&bce->lock);
	pmipcache_free(bce);
	dbg("PMIP cache entry is Deleted.. Try again!!!\n");
}

/**
 * pmip cache_delete - deletes a proxy bul entry
 **/
void pmip_cache_delete(const struct in6_addr *our_addr,const struct in6_addr *peer_addr)
{
	struct pmip_entry *bce;
	pthread_rwlock_wrlock(&pmip_lock);
		bce = hash_get(&pmip_hash, our_addr, peer_addr);
		if (bce)
		pmip_bce_delete(bce);
			
	pthread_rwlock_unlock(&pmip_lock);
}

/**
 * pmip_cache_iterate - apply function to every BC entry
 * @func: function to apply
 * @arg: extra data for @func
 *
 * Iterates through proxy binding cache, calling @func for each entry.
 * Extra data may be passed to @func in @arg.  @func takes a bcentry
 * as its first argument and @arg as second argument.
 **/
int pmip_cache_iterate(int (* func)(void *, void *), void *arg)
{
	int err;
	pthread_rwlock_rdlock(&pmip_lock); 
	err = hash_iterate(&pmip_hash, func, arg);
	pthread_rwlock_unlock(&pmip_lock);
	return err;
}

