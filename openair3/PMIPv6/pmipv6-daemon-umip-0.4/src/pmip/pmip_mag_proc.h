/*! \file pmip_mag_proc.h
* \brief
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/

/** \defgroup MAG_Processing MAG_Processing
 * \ingroup PMIP6D
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
#    define SYSCTL_IP6_FORWARDING CTL_NET, NET_IPV6, NET_IPV6_CONF, NET_PROTO_CONF_ALL, NET_IPV6_FORWARDING
#    define PROC_SYS_IP6_FORWARDING "/proc/sys/net/ipv6/conf/all/forwarding"
#    define PROC_SYS_IP6_LINKMTU "/proc/sys/net/ipv6/conf/%s/mtu"
#    define PROC_SYS_IP6_CURHLIM "/proc/sys/net/ipv6/conf/%s/hop_limit"
#    define PROC_SYS_IP6_BASEREACHTIME_MS "/proc/sys/net/ipv6/neigh/%s/base_reachable_time_ms"
#    define PROC_SYS_IP6_BASEREACHTIME "/proc/sys/net/ipv6/neigh/%s/base_reachable_time"
#    define PROC_SYS_IP6_RETRANSTIMER_MS "/proc/sys/net/ipv6/neigh/%s/retrans_time_ms"
#    define PROC_SYS_IP6_RETRANSTIMER "/proc/sys/net/ipv6/neigh/%s/retrans_time"

#    ifndef IPV6_ADDR_LINKLOCAL
#        define IPV6_ADDR_LINKLOCAL 0x0020U
#    endif
//-----------------------------------------------------------------------------
#	include <netinet/ip6.h>
#	include "pmip_cache.h"
#	include "pmip_msgs.h"
//-PROTOTYPES----------------------------------------------------------------------------
/*! \fn int mag_setup_route(struct in6_addr *pmip6_addr, int downlink)
* \brief	Set a route on a MAG for reaching a mobile node.
* \param[in]  pmip6_addr Mobile node destination address
* \param[in]  downlink Outgoing downlink interface id
* \return   Returns zero on success, negative otherwise.
*/
private_pmip_mag_proc(int mag_setup_route(struct in6_addr *pmip6_addr, int downlink);)
/*! \fn int mag_remove_route(struct in6_addr *, int )
* \brief  Remove a route on a MAG
* \param[in]  pmip6_addr Mobile node destination address
* \param[in]  downlink Outgoing downlink interface id
* \return   Returns zero on success, negative otherwise.
*/
protected_pmip_mag_proc(int mag_remove_route(struct in6_addr *pmip6_addr, int downlink);)
/*! \fn int mag_dereg(pmip_entry_t * bce, int propagate)
* \brief  Deregister a binding cache entry and its associated network configuration.
* \param[in]  bce       A binding cache entry
* \param[in]  propagate Propagate deregistration to LMA
* \return   Returns zero on success, negative otherwise.
*/
protected_pmip_mag_proc(int mag_dereg(pmip_entry_t * bce, int propagate);)
/*! \fn int mag_start_registration(pmip_entry_t *)
* \brief  Start the Location Registration Procedure for a mobile node by sending a PBU to the LMA.
* \param[in]  bce A binding cache entry
* \return   Always Zero.
*/
protected_pmip_mag_proc(int mag_start_registration(pmip_entry_t * bce);)
/*! \fn int mag_end_registration(pmip_entry_t * bce, int )
* \brief   End the Location Registration Procedure
* \param[in]  bce A binding cache entry
* \param[in]  iif Interface id used to communicate with the LMA
* \return   Always Zero.
* \note     Creates a tunnel between MAG and LMA, set a route for uplink traffic towards LMA if the route does not exist, send a router advertisement to the mobile node, and finally set the route to reach the mobile node.
*/
protected_pmip_mag_proc(int mag_end_registration(pmip_entry_t * bce, int iif);)
/*! \fn int mag_force_update_registration(pmip_entry_t * bce, int )
* \brief   Start the Location Registration Procedure for a mobile node by sending a PBU to the LMA.
* \param[in]  bce A binding cache entry
* \param[in]  iif Interface id used to communicate with the LMA
* \return   Always Zero.
* \note     Sometimes appear the case where the access point did not detect the departure of the mobile node,
*           so when the mobile come back again, we have to register again to the LMA, in order to let LMA build
*           a tunnel between LMA and this MAG.
*/
protected_pmip_mag_proc(int mag_force_update_registration(pmip_entry_t * bce, int iif);)
/*! \fn int mag_kickoff_ra(pmip_entry_t * bce)
* \brief  Start sendind router advertisements to a mobile node.
* \param[in]  bce A binding cache entry
* \return   Returns zero on success, negative otherwise.
*/
protected_pmip_mag_proc(int mag_kickoff_ra(pmip_entry_t * bce);)
/*! \fn int check_ip6_forwarding(void)
* \brief  Check if IPv6 forwarding is set in the kernel
* \return   Zero if forwarding is set, else -1.
*/
private_pmip_mag_proc(int check_ip6_forwarding(void);)
/*! \fn int mag_get_ingress_info(int *, char *)
* \brief Retrieve the link local address of the MAG interface accessible to mobile nodes.
* \param[out]  if_index         The interface identifier
* \param[out]  dev_name_mn_link The name of the interface
* \return   1 if success, else -1.
* \note The informations are retrieved by comparing /proc/net/if_inet6 with the configuration variable MagAddressIngress.
*/
protected_pmip_mag_proc(int mag_get_ingress_info(int *if_index, char *dev_name_mn_link);)
/*! \fn int setup_linklocal_addr(struct in6_addr *)
* \brief Retrieve the link local address of the MAG interface accessible to mobile nodes.
* \param[out]  src The link local address.
* \return    1 if success, else -1.
*/
protected_pmip_mag_proc(int setup_linklocal_addr(struct in6_addr *src);)
/*! \fn int mag_update_binding_entry(pmip_entry_t * bce, msg_info_t * info)
* \brief  Update a binding cache entry with received message informations.
* \param[in]  bce A binding cache entry
* \param[in]  info Informations contained in the message received
* \return   Always Zero.
*/
private_pmip_mag_proc(int mag_update_binding_entry(pmip_entry_t * bce, msg_info_t * info);)
/*! \fn int mag_pmip_md(msg_info_t * info, pmip_entry_t * bce)
* \brief   Start movement detection for a mobile node.
* \param[in]  info Informations contained in the message received
* \param[in]  bce A binding cache entry
* \return   Always Zero.
* \note   Initiate registration (send PBU) to the LMA.
*/
protected_pmip_mag_proc(int mag_pmip_md(msg_info_t * info, pmip_entry_t * bce);)
#endif
/** @}*/
