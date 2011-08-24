/*! \file pmip_msgs.h
* \brief 
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/

/** @defgroup MESSAGES
 * @ingroup PMIP6D
 *  PMIP Messages (MSGs) 
 *  @{
 */

#ifndef __PMIP_MSGS_H__
#    define __PMIP_MSGS_H__
//-----------------------------------------------------------------------------
#    ifdef PMIP_MSGS_C
#        define private_pmip_msgs(x) x
#        define protected_pmip_msgs(x) x
#        define public_pmip_msgs(x) x
#    else
#        ifdef PMIP
#            define private_pmip_msgs(x)
#            define protected_pmip_msgs(x) extern x
#            define public_pmip_msgs(x) extern x
#        else
#            define private_pmip_msgs(x)
#            define protected_pmip_msgs(x)
#            define public_pmip_msgs(x) extern x
#        endif
#    endif
//-----------------------------------------------------------------------------
#    include "conf.h"
#    include <netinet/icmp6.h>
#    include <errno.h>
#    include <netinet/ip6mh.h>
#    include "mh.h"
#    include "util.h"
#    include "debug.h"
#    include "ndisc.h"
#    include "pmip_consts.h"
#    include "pmip_types.h"
#    include "pmip_cache.h"
#    include "pmip_mag_proc.h"
//hip
#    include <sys/ioctl.h>
#    include <sys/socket.h>
#    include <sys/time.h>
#    include <sys/uio.h>
#    include <sys/stat.h>
#    include <fcntl.h>
#    include <netinet/in.h>
#    include <netinet/ip6.h>
#    include <arpa/inet.h>
#    include <sys/sysctl.h>
#    include <net/if.h>
//hip includes end
//-----------------------------------------------------------------------------
private_pmip_msgs(struct in6_addr get_node_id(struct in6_addr *mn_addr));
private_pmip_msgs(struct in6_addr get_node_prefix(struct in6_addr *mn_addr));
private_pmip_msgs(int mh_create_opt_home_net_prefix(struct iovec *iov, struct in6_addr
                            *Home_Network_Prefix));
private_pmip_msgs(int mh_create_opt_mn_identifier(struct iovec *iov, int flags, ip6mnid_t * MN_ID));
private_pmip_msgs(int mh_create_opt_time_stamp(struct iovec *iov, ip6ts_t * Timestamp));
private_pmip_msgs(int mh_create_opt_link_local_add(struct iovec *iov, struct in6_addr
                           *LinkLocal));
private_pmip_msgs(int mh_create_opt_dst_mn_addr(struct iovec *iov, struct in6_addr *dst_mn_addr));
private_pmip_msgs(int mh_create_opt_serv_mag_addr(struct iovec *iov, struct in6_addr
                          *Serv_MAG_addr));
private_pmip_msgs(int mh_create_opt_serv_lma_addr(struct iovec *iov, struct in6_addr
                          *serv_lma_addr));
private_pmip_msgs(int mh_create_opt_src_mn_addr(struct iovec *iov, struct in6_addr *src_mn_addr));
private_pmip_msgs(int mh_create_opt_src_mag_addr(struct iovec *iov, struct in6_addr
                         *src_mag_addr));
protected_pmip_msgs(int mh_pbu_parse(msg_info_t * info, struct ip6_mh_binding_update *pbu, ssize_t len, const struct in6_addr_bundle *in_addrs, int iif));
protected_pmip_msgs(int mh_pba_parse(msg_info_t * info, struct ip6_mh_binding_ack *pba, ssize_t len, const struct in6_addr_bundle *in_addrs, int iif));
protected_pmip_msgs(int mh_pbreq_parse(msg_info_t * info, ip6_mh_proxy_binding_request_t * pbr, ssize_t len, const struct in6_addr_bundle *in_addrs, int iif));
protected_pmip_msgs(int mh_pbres_parse(msg_info_t * info, ip6_mh_proxy_binding_response_t * pbre, ssize_t len, const struct in6_addr_bundle *in_addrs, int iif));
protected_pmip_msgs(int icmp_rs_parse(msg_info_t * info, struct nd_router_solicit *rs, const struct in6_addr *saddr, const struct in6_addr *daddr, int iif, int hoplimit));
protected_pmip_msgs(int icmp_ns_parse(msg_info_t * info, struct nd_neighbor_solicit *ns, const struct in6_addr *saddr, const struct in6_addr *daddr, int iif, int hoplimit));
protected_pmip_msgs(int icmp_na_parse(msg_info_t * info, struct nd_neighbor_advert *na, const struct in6_addr *saddr, const struct in6_addr *daddr, int iif, int hoplimit));
protected_pmip_msgs(int pmip_mh_send(const struct in6_addr_bundle *addrs, const struct iovec *mh_vec, int iovlen, int oif));
protected_pmip_msgs(int mh_send_pbu(const struct in6_addr_bundle *addrs, pmip_entry_t * bce, struct timespec *lifetime, int oif));
protected_pmip_msgs(int mh_send_pba(const struct in6_addr_bundle *addrs, pmip_entry_t * bce, struct timespec *lifetime, int oif));
protected_pmip_msgs(int
            mh_send_pbreq(struct in6_addr_bundle *addrs,
                  struct in6_addr *dst_mn_iid,
                  struct in6_addr *dst_mn_addr, struct in6_addr *src_mag_addr, struct in6_addr *src_mn_addr, uint16_t seqno, uint16_t action, int link, pmip_entry_t * bce));
protected_pmip_msgs(int
            mh_send_pbres(struct in6_addr_bundle *addrs,
                  struct in6_addr *src_mn_addr,
                  struct in6_addr *dst_mn_addr,
                  struct in6_addr *dst_mn_iid, struct in6_addr *serv_mag_addr, struct in6_addr *serv_lma_addr, uint16_t seqno, uint16_t status, int link));
private_pmip_msgs(void mn_send_rs());
#endif
