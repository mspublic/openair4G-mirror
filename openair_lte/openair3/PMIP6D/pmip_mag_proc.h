/*! \file pmip_mag_proc.h
* \brief 
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/

/** @defgroup MAG_Processing
 * @ingroup PMIP6D
 *  PMIP PROCessing for MAG 
 *  @{
 */

#ifndef __PMIP_MAG_PROC_H__
#    define __PMIP_MAG_PROC_H__
//-----------------------------------------------------------------------------
#    ifdef PMIP_MAG_PROC_C
#        define private_pmip_mag_proc(x) x
#        define protected_pmip_mag_proc(x) x
#        define public_pmip_mag_proc(x) x
#    else
#        ifdef PMIP
#            define private_pmip_mag_proc(x)
#            define protected_pmip_mag_proc(x) extern x
#            define public_pmip_mag_proc(x) extern x
#        else
#            define private_pmip_mag_proc(x)
#            define protected_pmip_mag_proc(x)
#            define public_pmip_mag_proc(x) extern x
#        endif
#    endif
//-----------------------------------------------------------------------------
#    include "prefix.h"
#    include "mh.h"
#    include "debug.h"
#    include "conf.h"
#    include "pmip_cache.h"
#    include "pmip_tunnel.h"
#    include "pmip_msgs.h"
#    include "pmip_consts.h"
#    include "pmip_handler.h"
#    include "rtnl.h"
#    include "tunnelctl.h"
#    include <pthread.h>
#    include "pmip_extern.h"
#    include "unistd.h"
#    define SYSCTL_IP6_FORWARDING CTL_NET, NET_IPV6, NET_IPV6_CONF, NET_PROTO_CONF_ALL, NET_IPV6_FORWARDING
#    define PROC_SYS_IP6_FORWARDING "/proc/sys/net/ipv6/conf/all/forwarding"
#    define PROC_SYS_IP6_LINKMTU "/proc/sys/net/ipv6/conf/%s/mtu"
#    define PROC_SYS_IP6_CURHLIM "/proc/sys/net/ipv6/conf/%s/hop_limit"
#    define PROC_SYS_IP6_BASEREACHTIME_MS "/proc/sys/net/ipv6/neigh/%s/base_reachable_time_ms"
#    define PROC_SYS_IP6_BASEREACHTIME "/proc/sys/net/ipv6/neigh/%s/base_reachable_time"
#    define PROC_SYS_IP6_RETRANSTIMER_MS "/proc/sys/net/ipv6/neigh/%s/retrans_time_ms"
#    define PROC_SYS_IP6_RETRANSTIMER "/proc/sys/net/ipv6/neigh/%s/retrans_time"
//hip
#    include <sys/ioctl.h>
#    include <sys/socket.h>
#    include <sys/time.h>
#    include <sys/uio.h>
#    include <sys/stat.h>
#    include <fcntl.h>
#    include <netinet/in.h>
#    include <netinet/ip6.h>
#    include <netinet/icmp6.h>
#    include <arpa/inet.h>
#    include <sys/sysctl.h>
#    include <net/if.h>
//hip includes end
//hip
#    ifndef IPV6_ADDR_LINKLOCAL
#        define IPV6_ADDR_LINKLOCAL 0x0020U
#    endif
//-----------------------------------------------------------------------------
private_pmip_mag_proc(int mag_setup_route(struct in6_addr *pmip6_addr, int downlink));
protected_pmip_mag_proc(int mag_remove_route(struct in6_addr *pmip6_addr, int downlink));
protected_pmip_mag_proc(int mag_dereg(pmip_entry_t * bce, int propagate));
protected_pmip_mag_proc(int mag_start_registration(pmip_entry_t * bce));
protected_pmip_mag_proc(int mag_end_registration(pmip_entry_t * bce, int iif));
protected_pmip_mag_proc(int mag_kickoff_ra(pmip_entry_t * bce));
private_pmip_mag_proc(int check_ip6_forwarding(void));
protected_pmip_mag_proc(int mag_get_ingress_info(int *if_index, char *dev_name_mn_link));
protected_pmip_mag_proc(int setup_linklocal_addr(struct in6_addr *src));
private_pmip_mag_proc(int mag_update_binding_entry(pmip_entry_t * bce, msg_info_t * info));
protected_pmip_mag_proc(int mag_pmip_md(msg_info_t * info, pmip_entry_t * bce));
#endif
