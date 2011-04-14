/*! \file pmip_extern.h
* \brief Describe all external functions and variables
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/

/** @defgroup EXTERNAL_STRUCTURES
 * @ingroup PMIP6D
 *  PMIP External functions and Variables 
 *  @{
 */

#ifndef __pmip_extern_h
#    define __pmip_extern_h
#    include "pmip_types.h"
#    include "pmip_consts.h"
#    include "pmip_cache.h"
#    include <netinet/in.h>
#    include <netinet/ip6mh.h>
#    include "icmp6.h"
void pmip_timer_bce_expired_handler(struct tq_elem *tqe);
/** pointer to task upon entry expiry! */
struct in6_addr get_node_id(struct in6_addr *mn_addr);
struct in6_addr get_node_prefix(struct in6_addr *mn_addr);
struct in6_addr *get_mn_addr(pmip_entry_t * bce);
extern struct icmp6_handler pmip_mag_ns_handler;
extern struct mh_handler pmip_mag_pbu_handler;
extern struct mh_handler pmip_mag_pba_handler;
extern struct mh_handler pmip_lma_pbu_handler;
extern struct icmp6_handler pmip_mag_recv_na_handler;
extern struct mh_handler pmip_pbreq_handler;
extern struct mh_handler pmip_pbres_handler;
extern struct icmp6_handler pmip_mag_rs_handler;    //hip
void mn_send_rs();      //hip
#endif
