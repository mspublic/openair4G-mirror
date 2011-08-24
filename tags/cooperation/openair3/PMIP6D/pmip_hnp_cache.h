/*! \file pmip_hnp_cache.h
* \brief 
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/

/** @defgroup HNP_CACHE
 * @ingroup PMIP6D
 *  PMIP HNP Cache 
 *  @{
 */

#ifndef __PMIP_HNP_CACHE_H__
#    define __PMIP_HNP_CACHE_H__
//-----------------------------------------------------------------------------
#    ifdef PMIP_HNP_CACHE_C
#        define private_pmip_hnp_cache(x) x
#        define protected_pmip_hnp_cache(x) x
#        define public_pmip_hnp_cache(x) x
#    else
#        ifdef PMIP
#            define private_pmip_hnp_cache(x)
#            define protected_pmip_hnp_cache(x) extern x
#            define public_pmip_hnp_cache(x) extern x
#        else
#            define private_pmip_hnp_cache(x)
#            define protected_pmip_hnp_cache(x)
#            define public_pmip_hnp_cache(x) extern x
#        endif
#    endif
//-----------------------------------------------------------------------------
#    include "pmip_types.h"
#    include "debug.h"
#    include "conf.h"
#    include "vt.h"
#    include "pmip_consts.h"
#    include "pmip_types.h"
#    ifdef PMIP_CACHE_DEBUG
#        define dbg(...) dbgprint(__FUNCTION__, __VA_ARGS__)
#    else
#        define dbg(...)
#    endif
#    define MNCOUNT 64
typedef struct mnid_hnp_t {
    struct in6_addr mn_prefix;  /* Network Address Prefix for MN */
    struct in6_addr mn_iid; /* MN IID */
} mnid_hnp_t;
//hip
#    include "pmip_extern.h"
//-GLOBAL VARIABLES----------------------------------------------------------------------------
//-PROTOTYPES----------------------------------------------------------------------------
protected_pmip_hnp_cache(struct in6_addr eth_address2hw_address(struct in6_addr macaddr));
protected_pmip_hnp_cache(struct in6_addr hw_address2eth_address(struct in6_addr iid));
protected_pmip_hnp_cache(void pmip_insert_into_hnp_cache(struct in6_addr mn_iid, struct in6_addr addr));
private_pmip_hnp_cache(int get_pmip_hnp_cache_count(int type));
protected_pmip_hnp_cache(void pmip_lma_mn_to_hnp_cache_init(void));
protected_pmip_hnp_cache(struct in6_addr lma_mnid_hnp_map(struct in6_addr mnid, int *aaa_result));
protected_pmip_hnp_cache(int pmip_mn_to_hnp_cache_init(void));
protected_pmip_hnp_cache(struct in6_addr mnid_hnp_map(struct in6_addr mnid, int *aaa_result));
#endif
