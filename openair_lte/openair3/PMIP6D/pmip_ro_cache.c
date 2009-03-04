/*****************************************************************
 * C Implementation: pmip_dcache
 * Description: 
 * Author: 
 *   Christian Bonnet
 *   Huu-Nghia Nguyen
 * Copyright: Eurecom Institute, (C) 2008
 ******************************************************************/

#include <netinet/in.h>
#include <netinet/ip6mh.h>
#include <pthread.h>
#include "debug.h"
#include "pmip_ro_cache.h"


#define PMIP_RO_CACHE_BUCKETS 16
static struct hash pmip_ro_hash;
pthread_rwlock_t pmip_ro_cache_lock; /* Protects RO cache */

#ifdef PEER_LIST
#define CONN_PER_DEST 5
static struct hash pmip_peer_list_hash;
#endif


//===================================================================
void dump_roe(void * re, void *os)
//===================================================================
{
	struct pmip_ro_entry *e = (struct pmip_ro_entry *) re;
	FILE *out = (FILE *)os;

	fprintf(out, " == Route Optimiztion Cache entry ");
	fprintf(out, " src_addr:    %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&e->src_addr));
	fprintf(out, " dst_addr:    %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&e->dst_addr));
	fprintf(out, " dst_serv_mag_addr:%x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&e->dst_serv_mag_addr));
	fprintf(out, " dst_serv_lma_addr:     %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&e->dst_serv_lma_addr));
	fprintf(out, " lifetime %ld\n ", e->lifetime.tv_sec);
	fprintf(out, " seqno %d\n", e->seqno);
	fflush(out);
}

//initializes the PMIP RO cache.
//===================================================================
int pmip_ro_cache_init(void)
//===================================================================
{
	int ret;

	if (pthread_rwlock_init(&pmip_ro_cache_lock, NULL)) return -1;

	pthread_rwlock_wrlock(&pmip_ro_cache_lock);
	bzero(&pmip_ro_hash, sizeof(pmip_ro_hash));
	ret = hash_init(&pmip_ro_hash, DOUBLE_ADDR, PMIP_RO_CACHE_BUCKETS);
    if (ret < 0) return ret;

	#ifdef PEER_LIST
	bzero(&pmip_peer_list_hash, sizeof(pmip_peer_list_hash));
    ret = hash_init(&pmip_peer_list_hash, SINGLE_ADDR, PMIP_RO_CACHE_BUCKETS) ;
	#endif

	pthread_rwlock_unlock(&pmip_ro_cache_lock);

	#ifdef ENABLE_VT
	if (ret < 0) return ret;
	//ret = vt_pdc_init();
	#endif
	return ret;
}

/**
 * Allocates a new RO cache entry. Returns allocated space for an entry or NULL if none
 * available.
 **/
//===================================================================
struct pmip_ro_entry *pmip_ro_cache_alloc()
//===================================================================
{
	struct pmip_ro_entry *tmp;
	
	tmp = malloc(sizeof(struct pmip_ro_entry));

	if (tmp == NULL){
		dbg("WARNING: No memory allocated for PMIP RO entry..\n");
		return NULL;
}
	if (pthread_rwlock_init(&tmp->lock, NULL)) {
		free(tmp);
		return NULL;
	}
	memset(tmp, 0, sizeof(*tmp));
	INIT_LIST_HEAD(&tmp->tqe.list);
	dbg("PMIP RO cache entry is allocated..\n");
	return tmp;
}

//===================================================================
static int __pmip_ro_cache_insert(struct pmip_ro_entry * de)
//===================================================================
{
	int ret;
	ret = hash_add(&pmip_ro_hash, de, &de->src_addr, &de->dst_addr);
	if (ret)
		return ret;
	dbg("PMIP RO cache entry is inserted..\n");
	return 0;
}

//===================================================================
int pmip_ro_cache_start(struct pmip_ro_entry * de)
//===================================================================
{
	dbg("PMIP RO cache start triggered.. \n"); 
	struct timespec expires;
	clock_gettime(CLOCK_REALTIME, &de->add_time);
	tsadd(de->add_time, de->lifetime, expires);
	//TODO add_task_abs(&expires, &de->tqe,(void *)_EXPIRED);
	dbg("Expiry Timer for PMIP RO cache entry is triggered.. \n"); 
	return 0;
}

//===================================================================
struct pmip_ro_entry * pmip_ro_cache_add(struct pmip_ro_entry *de) 
//===================================================================
{
	int ret = 0;
	dbg("Add RO cache entry for {src-dst} = {%x:%x:%x:%x:%x:%x:%x:%x-%x:%x:%x:%x:%x:%x:%x:%x}\n", 
		NIP6ADDR(&de->src_addr), NIP6ADDR(&de->dst_addr));
	assert(de);
	pthread_rwlock_wrlock(&pmip_ro_cache_lock);
	if ((ret = __pmip_ro_cache_insert(de)) != 0) {
		pthread_rwlock_unlock(&pmip_ro_cache_lock);
		dbg("WARNING: PMIP RO cache entry is not inserted!\n");
		return NULL;
	}

	#ifdef PEER_LIST
        struct peer_list * pl = NULL;
		struct in6_addr dummy;
		dummy = in6addr_any;
        pl = hash_get(&pmip_peer_list_hash, NULL, &de->dst_addr);
        if (pl)
        {
			if (pl->count == pl->max)
            {
				dbg("reallocate peer_list\n");              
				pl->peers = realloc((char *) pl->peers, pl->max * sizeof(struct in6_addr));
				pl->max += CONN_PER_DEST;
            }
            pl->peers[pl->count++] = de->src_addr;
            dbg("append src addr %x:%x:%x:%x:%x:%x:%x:%x to peer list, count = %d\n",  NIP6ADDR(&de->src_addr), pl->count);
        }
        else
        {
            dbg("allocate peer_list\n");
			pl = malloc(sizeof(struct peer_list));		  		          
			if (pl)
            {
				bzero(pl, sizeof(struct peer_list));
				pl->max = CONN_PER_DEST;
				pl->peers = malloc(pl->max * sizeof(struct in6_addr));			 
				pl->count = 0;
				if (pl->peers)
				{
					pl->peers[pl->count++] = de->src_addr;					
					dbg("append src addr %x:%x:%x:%x:%x:%x:%x:%x to peer list, count = %d\n",  NIP6ADDR(&de->src_addr), pl->count);
					hash_add(&pmip_peer_list_hash, pl, NULL, &de->dst_addr);
				}
				else dbg("WARNING: Unable to allocate peer list of %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&de->dst_addr));
				assert(pl->peers);				
            }
			else dbg("WARNING: Unable to allocate peer list of %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&de->dst_addr));
			assert(pl);
        }
	#endif
	
	//TODO pmip_ro_cache_start(de);
	pthread_rwlock_unlock(&pmip_ro_cache_lock);
	return de;		
}

//===================================================================
struct pmip_ro_entry * pmip_ro_cache_get(const struct in6_addr* src_addr, const struct in6_addr* dst_addr)
//===================================================================
{
	struct pmip_ro_entry *de;
	assert(dst_addr && src_addr);

	pthread_rwlock_rdlock(&pmip_ro_cache_lock);
	de = hash_get(&pmip_ro_hash, src_addr, dst_addr);
	if (de){ 
		pthread_rwlock_wrlock(&de->lock);
		dbg("PMIP RO cache entry is found for {src-dst} = {%x:%x:%x:%x:%x:%x:%x:%x-%x:%x:%x:%x:%x:%x:%x:%x}\n", 
		NIP6ADDR(&de->src_addr), NIP6ADDR(&de->dst_addr));
	}
	else{
		pthread_rwlock_unlock(&pmip_ro_cache_lock);
		dbg("No PMIP RO cache entry found...\n");	
	}
	return de;	

}

//===================================================================
void pmip_ro_cache_release_entry(struct pmip_ro_entry *de)
//===================================================================
{
	assert(de);
	pthread_rwlock_unlock(&de->lock);
	pthread_rwlock_unlock(&pmip_ro_cache_lock);
	dbg("PMIP cache entry is released...\n");
}

//===================================================================
int pmip_ro_cache_exists(const struct in6_addr* src_addr, const struct in6_addr* dst_addr)
//===================================================================
{
        struct pmip_ro_entry *de;
        de = pmip_ro_cache_get(src_addr, dst_addr);
        if (de == NULL) return -1;
        pmip_ro_cache_release_entry(de);
        return 1;
}

//===================================================================
void pmip_ro_cache_free(struct pmip_ro_entry *de)
//===================================================================
{
	pthread_rwlock_destroy(&de->lock);
	free(de);
	dbg("PMIP RO cache entry is deallocated!\n");
}

//===================================================================
void pmip_ro_delete(struct pmip_ro_entry *de)
//===================================================================
{
	pthread_rwlock_wrlock(&de->lock);
	del_task(&de->tqe);

	#ifdef PEER_LIST
        struct peer_list * pl;
		struct in6_addr dummy;
		dummy = in6addr_any;
        pl = hash_get(&pmip_peer_list_hash, NULL, &de->dst_addr);
        if (pl)
        {
          int i;
          for (i = 0; i<pl->count; i++)
            {
              if (IN6_ARE_ADDR_EQUAL(&de->src_addr, &pl->peers[i]))
                {
					pl->peers[i] = pl->peers[--(pl->count)];
					dbg("deleted  %x:%x:%x:%x:%x:%x:%x:%x from peer list of %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&de->src_addr), NIP6ADDR(&de->dst_addr));   
					break;
                }
            }
          if (pl->count == 0)
            {
              hash_delete(&pmip_peer_list_hash, NULL, &de->dst_addr);
              free(pl->peers);
              free(pl);
            }
        }
	#endif
	
	if (de->cleanup) de->cleanup(de);
	hash_delete(&pmip_ro_hash, &de->src_addr, &de->dst_addr);
	pthread_rwlock_unlock(&de->lock);
	pmip_ro_cache_free(de);
	dbg("PMIP RO cache entry is deleted!\n");
}

//===================================================================
void pmip_ro_cache_delete(const struct in6_addr* src_addr, const struct in6_addr* dst_addr)
//===================================================================
{
	struct pmip_ro_entry *de;
	pthread_rwlock_wrlock(&pmip_ro_cache_lock);
		de = hash_get(&pmip_ro_hash, src_addr, dst_addr);
		if (de) pmip_ro_delete(de);		
	pthread_rwlock_unlock(&pmip_ro_cache_lock);
}


//===================================================================
struct peer_list * pmip_ro_peer_list_get(const struct in6_addr* dst_addr)
//===================================================================
{
  struct peer_list * pl = NULL;
  #ifdef PEER_LIST  
  struct in6_addr dummy; 
  dummy = in6addr_any;
  pl = hash_get(&pmip_peer_list_hash, NULL, dst_addr);
  if (pl)
  {
    //pthread_rwlock_rdlock(&pmip_ro_cache_lock);
    dbg("PMIP peer list found for %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(dst_addr));
  }
  else dbg("PMIP peer list not found for %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(dst_addr));
  #endif 
  return pl;
}

//===================================================================
void pmip_ro_peer_list_release(struct peer_list * pl)
//===================================================================
{
  #ifdef PEER_LIST
  assert(pl);
  //pthread_rwlock_unlock(&pmip_ro_cache_lock);
  dbg("PMIP peer list is released...\n");
  #endif
}

/**
 * pmip_ro_cache_iterate - apply function to every dc entry
 * @func: function to apply
 * @arg: extra data for @func
 *
 * Iterates through proxy RO cache, calling @func for each entry.
 * Extra data may be passed to @func in @arg.  @func takes a ro entry
 * as its first argument and @arg as second argument.
 **/
//===================================================================
int pmip_ro_cache_iterate(int (* func)(void *, void *), void *arg)
//===================================================================
{
	int err;
	pthread_rwlock_rdlock(&pmip_ro_cache_lock); 
	err = hash_iterate(&pmip_ro_hash, func, arg);
	pthread_rwlock_unlock(&pmip_ro_cache_lock);
	return err;
}

#ifdef UNIT_TEST

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>


int main()
{
	struct in6_addr src, dst;
	struct pmip_ro_entry* de1, *de2;

	assert(pmip_ro_cache_init() >= 0);

	de1 = pmip_ro_cache_alloc();
	assert(inet_pton(AF_INET6, "2001::1", &src) > 0);
	assert(inet_pton(AF_INET6, "2001::3", &dst) > 0);
	de1->src_addr = src;
	de1->dst_addr = dst;
	assert(pmip_ro_cache_add(de1)); 
	assert(de1=pmip_ro_cache_get(&src, &dst));
 	pmip_ro_cache_release_entry(de1);

	de2 = pmip_ro_cache_alloc();
	assert(inet_pton(AF_INET6, "2001::2", &src) > 0);
	assert(inet_pton(AF_INET6, "2001::3", &dst) > 0);
	de2->src_addr = src;
	de2->dst_addr = dst;
	assert(pmip_ro_cache_add(de2)); 
	assert(de2=pmip_ro_cache_get(&src, &dst)); 
	pmip_ro_cache_release_entry(de2);

	struct peer_list * pl;
  	pl = pmip_ro_peer_list_get(&dst);
	assert(pl);
	assert(pl->count == 2);
	int i;
	for (i = 0; i<pl->count; i++)
	{
		dbg("%x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&pl->peers[i]));
	}
	pmip_ro_peer_list_release(pl);

	pmip_ro_delete(de1);
  	pl = pmip_ro_peer_list_get(&dst);
	assert(pl);
	assert(pl->count == 1);
	for (i = 0; i<pl->count; i++)
	{
		dbg("%x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&pl->peers[i]));
	}
	pmip_ro_peer_list_release(pl);

	pmip_ro_delete(de2);
  	pl = pmip_ro_peer_list_get(&dst);
	assert(pl == NULL);
}
#endif
