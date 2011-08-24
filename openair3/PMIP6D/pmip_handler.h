/*! \file pmip_handler.h
* \brief 
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/

/** @defgroup HANDLER 
 * @ingroup PMIP6D
 *  PMIP Handler
 *  @{
 */

#ifndef __PMIP_HANDLER_H__
#    define __PMIP_HANDLER_H__
//-----------------------------------------------------------------------------
#    ifdef PMIP_HANDLER_C
#        define private_pmip_handler(x) x
#        define protected_pmip_handler(x) x
#        define public_pmip_handler(x) x
#    else
#        ifdef PMIP
#            define private_pmip_handler(x)
#            define protected_pmip_handler(x) extern x
#            define public_pmip_handler(x) extern x
#        else
#            define private_pmip_handler(x)
#            define protected_pmip_handler(x)
#            define public_pmip_handler(x) extern x
#        endif
#    endif
//-----------------------------------------------------------------------------
#    include "icmp6.h"
#    include "mh.h"
#    include <netinet/in.h>
#    include <netinet/ip6mh.h>
#    include "debug.h"
#    include "conf.h"
#    include "rtnl.h"
#    include "prefix.h"
#    include "ndisc.h"
#    include "pmip_consts.h"
#    include "pmip_types.h"
#    include "pmip_extern.h"
#    include "pmip_cache.h"
#    include "pmip_hnp_cache.h"
#    include "pmip_msgs.h"
#    include "pmip_lma_proc.h"
#    include "pmip_mag_proc.h"
#    include "pmip_fsm.h"
//-----------------------------------------------------------------------------
protected_pmip_handler(struct in6_addr
               *link_local_addr(struct in6_addr *id));
protected_pmip_handler(struct in6_addr
               *CONVERT_ID2ADDR(struct in6_addr *result, struct in6_addr *prefix, struct in6_addr *id));
private_pmip_handler(struct in6_addr *get_mn_addr(pmip_entry_t * bce));
protected_pmip_handler(struct in6_addr
               *solicited_mcast(struct in6_addr *id));
private_pmip_handler(void pmip_timer_retrans_pbu_handler(struct tq_elem *tqe));
private_pmip_handler(void pmip_timer_bce_expired_handler(struct tq_elem *tqe));
#endif
