/*****************************************************************
 * C Implementation: pmip6d.c
 * Description: PMIP binding cache functions.
 * Author: 
 *   Christian Bonnet
 *   Huu-Nghia Nguyen
 *   Hussain & Daniel
 * Copyright: Eurecom Institute,(C) 2008
 ******************************************************************/

#include "debug.h"
#include "conf.h"
#include "vt.h"

#include "pmip_consts.h"
#include "pmip_types.h"
#include "pmip_extern.h"
#include "pmip_cache.h"

#define PMIP_CACHE_BUCKETS 32

#ifdef PMIP_CACHE_DEBUG
	#define dbg(...) dbgprint(__FUNCTION__, __VA_ARGS__)
#else
	#define dbg(...)
#endif

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
	fprintf(out, " MN IID:    %x:%x:%x:%x:%x:%x:%x:%x\n",
		NIP6ADDR(&e->mn_iid));
	fprintf(out, " MN Serving MAG Address:%x:%x:%x:%x:%x:%x:%x:%x\n", 
	     NIP6ADDR(&e->mn_serv_mag_addr));
	fprintf(out, " MN Serving LMA Address:     %x:%x:%x:%x:%x:%x:%x:%x\n",
		NIP6ADDR(&e->mn_serv_lma_addr));
	fprintf(out, " lifetime %ld\n ", e->lifetime.tv_sec);
	fprintf(out, " seqno %d\n", e->seqno_out);

	fflush(out);
}

//initializes the PMIP cache.
int pmip_cache_init(void)
{
	int ret;

	if (pthread_rwlock_init(&pmip_lock, NULL))
		return -1;

	pthread_rwlock_wrlock(&pmip_lock);
	ret = hash_init(&pmip_hash, DOUBLE_ADDR, PMIP_CACHE_BUCKETS);
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
		dbg("NO memory allocated for PMIP cache entry..\n");
		return NULL;
}
	if (pthread_rwlock_init(&tmp->lock, NULL)) {
		free(tmp);
		return NULL;
	}
	memset(tmp, 0, sizeof(*tmp));
	INIT_LIST_HEAD(&tmp->tqe.list);
	tmp->type = type;
	dbg("PMIP cache entry is allocated..\n");
	return tmp;
}


static int __pmipcache_insert(struct pmip_entry *bce)
{
	int ret;
	ret = hash_add(&pmip_hash, bce, &bce->our_addr, &bce->mn_iid);
	if (ret)
		return ret;

	pmip_cache_count++;
	dbg("PMIP cache entry is inserted..\n");
	return 0;
}

//PMIP cache start
int pmip_cache_start(struct pmip_entry *bce)
{
	dbg("PMIP cache start is initialized.. \n"); 
	struct timespec expires;
	clock_gettime(CLOCK_REALTIME, &bce->add_time);
	tsadd(bce->add_time, bce->lifetime, expires);
	add_task_abs(&expires, &bce->tqe,(void *)pmip_timer_bce_expired_handler);
	return 0;
}

struct pmip_entry * pmip_cache_add(struct pmip_entry *bce) 
{
	int ret = 1;
	assert(bce);
	bce->unreach = 0;
	pthread_rwlock_wrlock(&pmip_lock);
	if ((ret = __pmipcache_insert(bce)) != 0) {
		pthread_rwlock_unlock(&pmip_lock);
		dbg("WARNING: PMIP ENTRY NOT INSERTED..\n");
		return ret;
	}

	dbg("PMIP cache entry for: %x:%x:%x:%x:%x:%x:%x:%x with type %d is added\n", NIP6ADDR(&bce->mn_iid), bce->type);	
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
		dbg("PMIP cache entry is found for: %x:%x:%x:%x:%x:%x:%x:%x with type %d\n", NIP6ADDR(&bce->mn_iid), (bce->type));
	}
	else{
		pthread_rwlock_unlock(&pmip_lock);
		dbg("PMIP cache entry is not found...\n");
		
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
	dbg("PMIP cache entry is released\n");
}

int pmip_cache_exists(const struct in6_addr *our_addr, const struct in6_addr *peer_addr)
{
        struct pmip_entry *bce;
        int type;

        bce = pmip_cache_get(our_addr, peer_addr);

        if (bce == NULL)
                return -1;
	
	dbg("PMIP cache entry does exist with type: %d\n",(bce->type));
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
	dbg("PMIP cache entry is free\n");
}

void pmip_bce_delete(struct pmip_entry *bce)
{
	pthread_rwlock_wrlock(&bce->lock);
	del_task(&bce->tqe);
	
	if (bce->cleanup)
		bce->cleanup(bce);

	pmip_cache_count--;
	hash_delete(&pmip_hash, &bce->our_addr, &bce->mn_iid);
	pthread_rwlock_unlock(&bce->lock);
	pmipcache_free(bce);
	dbg("PMIP cache entry is deleted!\n");
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

