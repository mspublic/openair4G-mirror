/*! \file pmip_cache.c
* \brief PMIP binding cache functions 
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/
#define PMIP
#define PMIP_CACHE_C
#include "pmip_cache.h"
#include "pmip_config.h"
#include    <config.h>
#include    <includes.h>
#include    <pathnames.h>

static struct hash pmip_hash;
static int pmip_cache_count = 0;

/*!
*
* \param type
* \return number of entries in the cache.
*/
int get_pmip_cache_count(int type)
{
    if (type == BCE_PMIP || type == BCE_TEMP)
    return pmip_cache_count;
    return 0;
}

/*!
*  Dump in a stream the content of the binding cache entry
* \param bce the binding cache entry
* \param os output stream
*/
void dump_pbce(void *bce, void *os)
{
    pmip_entry_t *e = (pmip_entry_t *) bce;
    FILE *out = (FILE *) os;
    fprintf(out, " == Proxy Binding Cache entry ");
    switch (e->type) {
    case BCE_PMIP:
    fprintf(out, "(BCE_PMIP)\n");
    break;
    case BCE_TEMP:
    fprintf(out, "(BCE_TEMP)\n");
    break;
    default:
    fprintf(out, "(Unknown)\n");
    }
    fprintf(out, " MN IID:                 %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&e->mn_suffix));
    fprintf(out, " MN HW Address:          %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&e->mn_hw_address));
    fprintf(out, " MN Serving MAG Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&e->mn_serv_mag_addr));
    fprintf(out, " MN Serving LMA Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&e->mn_serv_lma_addr));
    fprintf(out, " lifetime %ld\n ", e->lifetime.tv_sec);
    fprintf(out, " seqno    %d\n", e->seqno_out);
    fflush(out);
}


/*!
*  initialize the pmip cache
* \return status of execution
*/
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

/*!
*  Set router advertisement parameters
*/
void init_iface_ra()
{
    router_ad_iface.AdvSendAdvert = DFLT_AdvSendAdv;
    router_ad_iface.MaxRtrAdvInterval = DFLT_MaxRtrAdvInterval;
    router_ad_iface.MinRtrAdvInterval = 1;  //changed from -1
    router_ad_iface.AdvIntervalOpt = DFLT_AdvIntervalOpt;
    router_ad_iface.AdvCurHopLimit = DFLT_AdvCurHopLimit;
    router_ad_iface.AdvHomeAgentFlag = DFLT_AdvHomeAgentFlag;
    router_ad_iface.AdvHomeAgentInfo = DFLT_AdvHomeAgentInfo;
    router_ad_iface.HomeAgentPreference = DFLT_HomeAgentPreference;
    router_ad_iface.HomeAgentLifetime = 10000;  //changed from -1
    router_ad_iface.AdvReachableTime = DFLT_AdvReachableTime;
    router_ad_iface.AdvRetransTimer = DFLT_AdvRetransTimer;
    router_ad_iface.AdvDefaultLifetime = 6000;
    router_ad_iface.AdvManagedFlag = 0;
    router_ad_iface.AdvOtherConfigFlag = 0;
// default values for Prefix.
    router_ad_iface.Adv_Prefix.AdvOnLinkFlag = DFLT_AdvOnLinkFlag;
    router_ad_iface.Adv_Prefix.AdvAutonomousFlag = DFLT_AdvAutonomousFlag;
    router_ad_iface.Adv_Prefix.AdvRouterAddr = DFLT_AdvRouterAddr;
    router_ad_iface.Adv_Prefix.PrefixLen = 64;
    router_ad_iface.Adv_Prefix.AdvValidLifetime = DFLT_AdvValidLifetime;
    router_ad_iface.Adv_Prefix.AdvPreferredLifetime = DFLT_AdvPreferredLifetime;
}

/*!
*  Allocates a new binding cache entry. Returns allocated space for an entry or NULL if none available.
* \param type type of entry
* \return the allocated binding cache entry
*/
pmip_entry_t *pmip_cache_alloc(int type)
{
    pmip_entry_t *tmp;
    tmp = malloc(sizeof(pmip_entry_t));
    if (tmp == NULL) {
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

/*!
*  insert a bindind cache entry in the cache
* \param bce binding cache entry
* \return 0 if success
*/
static int __pmipcache_insert(pmip_entry_t * bce)
{
    int ret;
    ret = hash_add(&pmip_hash, bce, &bce->our_addr, &bce->mn_hw_address);
    if (ret)
      return ret;
    pmip_cache_count++;
    dbg("PMIP cache entry is inserted for: %x:%x:%x:%x:%x:%x:%x:%x <-> %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&bce->our_addr), NIP6ADDR(&bce->mn_hw_address));
    return 0;
}

/*!
*  start a timer on a binding cache entry
*
* \param bce binding cache entry
* \return always 0
*/
int pmip_cache_start(pmip_entry_t * bce)
{
    dbg("PMIP cache start is initialized.. \n");
    struct timespec expires;
    clock_gettime(CLOCK_REALTIME, &bce->add_time);
    tsadd(bce->add_time, bce->lifetime, expires);
    add_task_abs(&expires, &bce->tqe, (void *) pmip_timer_bce_expired_handler);
    return 0;
}

/*!
*  add an entry in the binding cache
* \param bce binding cache entry
* \return the binding cache entry if success, else NULL
*/
pmip_entry_t *pmip_cache_add(pmip_entry_t * bce)
{
    dbg("inside pmip_cache_add\n");
    int ret = 1;
    assert(bce);
    dbg("after assertion\n");
    bce->unreach = 0;
    pthread_rwlock_wrlock(&pmip_lock);
    if ((ret = __pmipcache_insert(bce)) != 0) {
    pthread_rwlock_unlock(&pmip_lock);
    dbg("WARNING: PMIP ENTRY NOT INSERTED..\n");
    return NULL;
    }
    dbg("Making Entry\n");
    dbg("PMIP cache entry for: %x:%x:%x:%x:%x:%x:%x:%x with type %d is added\n", NIP6ADDR(&bce->mn_hw_address), bce->type);
    bce->n_rets_counter = conf.Max_Rets;
    dbg("Retransmissions counter intialized: %d\n", bce->n_rets_counter);
    if (bce->type == BCE_PMIP) {
    pmip_cache_start(bce);
    }
    pthread_rwlock_unlock(&pmip_lock);
    return bce;
}

/*!
*  get a binding cache entry
* \param our_addr
* \param peer_addr
* \return the corresponding cache entry
*/
pmip_entry_t *pmip_cache_get(const struct in6_addr * our_addr, const struct in6_addr * peer_addr)
{
    pmip_entry_t *bce;
    assert(peer_addr && our_addr);
    pthread_rwlock_rdlock(&pmip_lock);
    bce = hash_get(&pmip_hash, our_addr, peer_addr);
    if (bce) {
      pthread_rwlock_wrlock(&bce->lock);
      dbg("PMIP cache entry is found for: %x:%x:%x:%x:%x:%x:%x:%x with type %d\n", NIP6ADDR(&bce->mn_hw_address), (bce->type));
    } else {
      pthread_rwlock_unlock(&pmip_lock);
      dbg("PMIP cache entry is NOT found for %x:%x:%x:%x:%x:%x:%x:%x <-> %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(our_addr), NIP6ADDR(peer_addr));
    }
    return bce;
}

/*!
*  Unlocks a binding cache entry
* \param bce binding cache entry
*/
void pmipcache_release_entry(pmip_entry_t * bce)
{
    assert(bce);
    pthread_rwlock_unlock(&bce->lock);
    pthread_rwlock_unlock(&pmip_lock);
    dbg("PMIP cache entry is released\n");
}

/*!
*  check the existence of a cache entry for a Tuple of addresses
* \param our_addr
* \param peer_addr
* \return the type of the binding cache entry if found, -1 if entry not found
*/
int pmip_cache_exists(const struct in6_addr *our_addr, const struct in6_addr *peer_addr)
{
    pmip_entry_t *bce;
    int type;
    bce = pmip_cache_get(our_addr, peer_addr);
    if (bce == NULL)
    return -1;
    dbg("PMIP cache entry does exist with type: %d\n", (bce->type));
    type = bce->type;
    pmipcache_release_entry(bce);
    return type;
}

/*!
*  Release the allocated memory back of a binding cache entry to unused pool.
* \param bce Binding cache entry to free
*/
void pmipcache_free(pmip_entry_t * bce)
{
/* This function should really return allocated space to free
* pool. */
    pthread_rwlock_destroy(&bce->lock);
    free(bce);
    dbg("PMIP cache entry is free\n");
}

/*!
*  remove from the binding cache and free an entry
* \param bce Binding cache entry to delete
*/
void pmip_bce_delete(pmip_entry_t * bce)
{
    pthread_rwlock_wrlock(&bce->lock);
    del_task(&bce->tqe);
    if (bce->cleanup)
    bce->cleanup(bce);
    pmip_cache_count--;
    hash_delete(&pmip_hash, &bce->our_addr, &bce->mn_hw_address);
    pthread_rwlock_unlock(&bce->lock);
    pmipcache_free(bce);
    dbg("PMIP cache entry is deleted!\n");
}

/*!
*  Deletes a proxy bul entry
* \param our_addr
* \param peer_addr
*/
void pmip_cache_delete(const struct in6_addr *our_addr, const struct in6_addr *peer_addr)
{
    pmip_entry_t *bce;
    pthread_rwlock_wrlock(&pmip_lock);
    bce = hash_get(&pmip_hash, our_addr, peer_addr);
    if (bce)
    pmip_bce_delete(bce);
    pthread_rwlock_unlock(&pmip_lock);
}

/*!
*  Apply function to every BC entry
* Iterates through proxy binding cache, calling @func for each entry.
* Extra data may be passed to @func in @arg.  @func takes a bcentry
* as its first argument and @arg as second argument.
* \param func function to apply
* \param arg  extra data for func
* \return the iterate status
*/
int pmip_cache_iterate(int (*func) (void *, void *), void *arg)
{
    int err;
    pthread_rwlock_rdlock(&pmip_lock);
    err = hash_iterate(&pmip_hash, func, arg);
    pthread_rwlock_unlock(&pmip_lock);
    return err;
}
