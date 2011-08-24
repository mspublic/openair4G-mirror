/*! \file pmip_cache.h
* \brief 
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/

/** @defgroup CACHE
 * @ingroup PMIP6D
 *  PMIP CACHE
 *  @{
 */

#ifndef __PMIP_CACHE_H__
#    define __PMIP_CACHE_H__
//-----------------------------------------------------------------------------
#    ifdef PMIP_CACHE_C
#        define private_pmip_cache(x) x
#        define protected_pmip_cache(x) x
#        define public_pmip_cache(x) x
#    else
#        ifdef PMIP
#            define private_pmip_cache(x)
#            define protected_pmip_cache(x) extern x
#            define public_pmip_cache(x) extern x
#        else
#            define private_pmip_cache(x)
#            define protected_pmip_cache(x)
#            define public_pmip_cache(x) extern x
#        endif
#    endif
//-----------------------------------------------------------------------------
#    include "tqueue.h"
#    include "hash.h"
#    include "pmip_types.h"
#    include "debug.h"
#    include "conf.h"
#    include "vt.h"
#    include "pmip_consts.h"
#    include "pmip_types.h"
#    include "arpa/inet.h"
#    include "netinet/in.h"
#    define PMIP_CACHE_BUCKETS 128
#    ifdef PMIP_CACHE_DEBUG
#        define dbg(...) dbgprint(__FUNCTION__, __VA_ARGS__)
#    else
#        define dbg(...)
#    endif
#    define MNCOUNT 64
int terminateMN;
#    ifndef ND_OPT_PI_FLAG_RADDR
#        define ND_OPT_PI_FLAG_RADDR        0x20
#    endif
#    define DFLT_AdvValidLifetime           86400
/* seconds */
#    define DFLT_AdvOnLinkFlag      1
#    define DFLT_AdvPreferredLifetime      14400    /* seconds */
#    define DFLT_AdvAutonomousFlag      1
#    ifndef ND_OPT_HAI_FLAG_SUPPORT_MR
#        if BYTE_ORDER== BIG_ENDIAN
#            define ND_OPT_HAI_FLAG_SUPPORT_MR  0x8000
#        else
#            define ND_OPT_HAI_FLAG_SUPPORT_MR  0x0080
#        endif
#    endif
#    define DFLT_AdvSendAdv         1
#    define DFLT_MaxRtrAdvInterval      1.5
#    define DFLT_MinRtrAdvInterval            1 //(iface) (0.33 * (iface)->MaxRtrAdvInterval)
#    define DFLT_AdvCurHopLimit     64  /* as per RFC 1700 or the
                       next incarnation of it :) */
#    define DFLT_AdvReachableTime       0
#    define DFLT_AdvRetransTimer        0
#    define DFLT_HomeAgentPreference    20
#    define DFLT_AdvHomeAgentFlag       1
#    define DFLT_AdvIntervalOpt     1
#    define DFLT_AdvHomeAgentInfo       1
#    define DFLT_AdvRouterAddr      1
#    define MSG_SIZE 4096   //hip

typedef struct AdvPrefix_t {
    struct in6_addr Prefix;
    uint8_t PrefixLen;
    int AdvOnLinkFlag;
    int AdvAutonomousFlag;
    uint32_t AdvValidLifetime;
    uint32_t AdvPreferredLifetime;
/* Mobile IPv6 extensions */
    int AdvRouterAddr;
} adv_prefix_t;


//hip
typedef struct HomeAgentInfo_t {
    uint8_t type;
    uint8_t length;
    uint16_t flags_reserved;
    uint16_t preference;
    uint16_t lifetime;
} home_agent_info_t;


typedef struct ra_iface_t {
    int AdvSendAdvert;
    double MaxRtrAdvInterval;
    double MinRtrAdvInterval;
    uint32_t AdvReachableTime;
    uint32_t AdvRetransTimer;
    int32_t AdvDefaultLifetime;
    int AdvMobRtrSupportFlag;
    uint8_t AdvCurHopLimit;
/* Mobile IPv6 extensions */
    int AdvIntervalOpt;
    int AdvHomeAgentInfo;
    int AdvHomeAgentFlag;
    uint16_t HomeAgentPreference;
    int32_t HomeAgentLifetime;  /* XXX: really uint16_t but we need to use -1 */
    int AdvManagedFlag;
    int AdvOtherConfigFlag;
    adv_prefix_t Adv_Prefix;
} router_ad_iface_t;


typedef struct pmip_entry_t {
    struct in6_addr mn_prefix;  /* Network Address Prefix for MN */
    struct in6_addr our_addr;   /* Address to which we got BU */
    struct in6_addr mn_suffix;     /* MN IID */
    struct in6_addr mn_hw_address; /* MAC ADDR */
    struct in6_addr mn_addr;    /* Full MN Address */
    struct in6_addr mn_serv_mag_addr;   /* Serving MAG Address */
    struct in6_addr mn_serv_lma_addr;
    struct in6_addr mn_link_local_addr; /* Link Local Address  for MN */
    struct timespec add_time;   /* When was the binding added or modified */
    struct timespec lifetime;   /* lifetime sent in this BU, in seconds */
    uint16_t seqno_in;      /* sequence number for response messages */
    uint16_t seqno_out;     /* sequence number for created messages */
    uint16_t PBU_flags;     /* PBU flags */
    uint8_t PBA_flags;      /* PBA flags */
    int type;           /* Entry type */
    int unreach;        /* ICMP dest unreach count */
    int tunnel;         /* Tunnel interface index */
    int link;           /* Home link interface index */
/* PBU/PBRR message for retransmissions */
    struct iovec mh_vec[7];
    int iovlen;
/* info_block status flags */
    uint8_t status;
    ip6ts_t timestamp;
    uint32_t msg_event;
/* Following fields are for internal use only */
    struct timespec br_lastsent;    /* BR ratelimit */
    int br_count;       /* BR ratelimit */
    int n_rets_counter;     /* Counter for N retransmissions before deleting the entry */
    pthread_rwlock_t lock;  /* Protects the entry */
    struct tq_elem tqe;     /* Timer queue entry for expire */
    void (*cleanup) (struct pmip_entry_t * bce);    /* Clean up bce data */
} pmip_entry_t;


#    include "pmip_extern.h"
//Dedicated to PMIP cache
#    define BCE_NO_ENTRY (-1)
#    define BCE_PMIP 5
#    define BCE_TEMP 6
#    define BCE_HINT 7
#    define BCE_CN 8


//-GLOBAL VARIABLES----------------------------------------------------------------------------
protected_pmip_cache(pthread_rwlock_t pmip_lock);
protected_pmip_cache(router_ad_iface_t router_ad_iface);
//-PROTOTYPES----------------------------------------------------------------------------
private_pmip_cache(int get_pmip_cache_count(int type));
private_pmip_cache(void dump_pbce(void *bce, void *os));
protected_pmip_cache(int pmip_cache_init(void));
protected_pmip_cache(void init_iface_ra());
protected_pmip_cache(pmip_entry_t * pmip_cache_alloc(int type));
protected_pmip_cache(int pmip_cache_start(pmip_entry_t * bce));
protected_pmip_cache(pmip_entry_t * pmip_cache_add(pmip_entry_t * bce));
protected_pmip_cache(pmip_entry_t * pmip_cache_get(const struct in6_addr *our_addr, const struct in6_addr *peer_addr));
protected_pmip_cache(void pmipcache_release_entry(pmip_entry_t * bce));
protected_pmip_cache(int pmip_cache_exists(const struct in6_addr *our_addr, const struct in6_addr *peer_addr));
private_pmip_cache(void pmipcache_free(pmip_entry_t * bce));
protected_pmip_cache(void pmip_bce_delete(pmip_entry_t * bce));
protected_pmip_cache(void pmip_cache_delete(const struct in6_addr *our_addr, const struct in6_addr *peer_addr));
public_pmip_cache(int pmip_cache_iterate(int (*func) (void *, void *), void *arg));
#endif
