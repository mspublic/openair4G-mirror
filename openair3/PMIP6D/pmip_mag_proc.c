/*! \file pmip_mag_proc.c
* \brief 
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/
#define PMIP
#define PMIP_MAG_PROC_C
#include "pmip_mag_proc.h"
void pmip_timer_retrans_pbu_handler(struct tq_elem *tqe);
extern uint16_t seqno_pbreq;
/*!
*  set a route
* \param pmip6_addr
* \param downlink
* \return status of setup
*/
int mag_setup_route(struct in6_addr *pmip6_addr, int downlink)
{
    int res = 0;
    if (conf.tunneling_enabled) {
//add a rule for MN for uplink traffic from MN must query the TABLE for PMIP --> tunneled
    dbg("Uplink: Add new rule for tunneling src=%x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(pmip6_addr));
    res = rule_add(NULL, RT6_TABLE_PMIP, IP6_RULE_PRIO_PMIP6_FWD, RTN_UNICAST, pmip6_addr, 128, &in6addr_any, 0);
//add a route for downlink traffic through LMA (any src) ==> MN
    dbg("Downlink: Add new route for %x:%x:%x:%x:%x:%x:%x:%x in table %d\n", NIP6ADDR(pmip6_addr), RT6_TABLE_MIP6);
    res |= route_add(downlink, RT6_TABLE_MIP6, RTPROT_MIP, 0, IP6_RT_PRIO_MIP6_FWD, &in6addr_any, 0, pmip6_addr, 128, NULL);
    dbg("Downlink route addition success");
    }
    return res;
}

/*!
*  remove a route
* \param pmip6_addr
* \param downlink
* \return status of remove
*/
int mag_remove_route(struct in6_addr *pmip6_addr, int downlink)
{
    int res = 0;
    if (conf.tunneling_enabled) {
//Delete existing rule for the deleted MN
    dbg("Uplink: Delete old rule for tunneling src=%x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(pmip6_addr));
    res = rule_del(NULL, RT6_TABLE_PMIP, IP6_RULE_PRIO_PMIP6_FWD, RTN_UNICAST, pmip6_addr, 128, &in6addr_any, 0);
//Delete existing route for the deleted MN
    dbg("Downlink: Delete old routes for: %x:%x:%x:%x:%x:%x:%x:%x from table %d\n", NIP6ADDR(pmip6_addr), RT6_TABLE_MIP6);
    res |= route_del(downlink, RT6_TABLE_MIP6, IP6_RT_PRIO_MIP6_FWD, &in6addr_any, 0, pmip6_addr, 128, NULL);
    }
    return res;
}

/*!
*  Deregister a binding cache entry and its associated network configuration
* \param bce
* \param propagate
* \return status of deregister
*/
int mag_dereg(pmip_entry_t * bce, int propagate)
{
//Delete existing route & rule for the deleted MN
    int res = 0;
	bce->type = BCE_NO_ENTRY;
    res = mag_remove_route(get_mn_addr(bce), bce->link);
    int usercount = tunnel_getusers(bce->tunnel);
    dbg("# of binding entries %d \n", usercount);
    if (usercount == 1)
    route_del(bce->tunnel, RT6_TABLE_PMIP, IP6_RT_PRIO_MIP6_FWD, &in6addr_any, 0, &in6addr_any, 0, NULL);
//decrement users of old tunnel.
    pmip_tunnel_del(bce->tunnel);
    if (propagate) {
    dbg("Propagate the deregistration... \n");
    struct in6_addr_bundle addrs;
    addrs.src = &conf.our_addr;
    addrs.dst = &conf.lma_addr;
    struct timespec Lifetime = { 0, 0 };
    dbg("Create PBU for LMA for deregistration....\n");
    mh_send_pbu(&addrs, bce, &Lifetime, 0);
    }
//Delete PBU cache entry
    dbg("Delete PBU entry....\n");
    pmipcache_release_entry(bce);
    pmip_bce_delete(bce);
    return res;
}

/*!
*  Start the Location Registration Procedure
* \param bce
* \return always 0
*/
int mag_start_registration(pmip_entry_t * bce)
{
//Create PBU and send to the LMA
    struct in6_addr_bundle addrs;
    addrs.src = &conf.mag_addr_egress;
    addrs.dst = &conf.lma_addr;
    mh_send_pbu(&addrs, bce, &conf.PBU_LifeTime, 0);
//add a new task for PBU retransmission.
    struct timespec expires;
    clock_gettime(CLOCK_REALTIME, &bce->add_time);
    tsadd(bce->add_time, conf.N_RetsTime, expires);
    add_task_abs(&expires, &bce->tqe, pmip_timer_retrans_pbu_handler);
    dbg("PBU Retransmissions Timer is registered....\n");
    return 0;
}

/*!
*  End the Location Registration Procedure
* \param bce
* \param iif
* \return always 0
*/
int mag_end_registration(pmip_entry_t * bce, int iif)
{
//Change the BCE type.
    bce->type = BCE_PMIP;
    dbg("New PMIP cache entry type: %d\n", bce->type);
//Reset the Retransmissions counter.
    bce->n_rets_counter = conf.Max_Rets;
//Add task for entry expiry.
    dbg("Timer for Expiry is initialized: %d(s)!\n", bce->lifetime.tv_sec);
    pmip_cache_start(bce);
//create a tunnel between MAG and LMA.
    bce->tunnel = pmip_tunnel_add(&conf.our_addr, &conf.lma_addr, iif);
    int usercount = tunnel_getusers(bce->tunnel);
    dbg("# of binding entries %d \n", usercount);
    if (usercount == 1) {
    dbg("Add routing entry for uplink traffic");
    route_add(bce->tunnel, RT6_TABLE_PMIP, RTPROT_MIP, 0, IP6_RT_PRIO_MIP6_FWD, &in6addr_any, 0, &in6addr_any, 0, NULL);
    }
    mag_kickoff_ra(bce);
    dbg("Adding route for : %x:%x:%x:%x:%x:%x:%x:%x \n", NIP6ADDR(get_mn_addr(bce)));
    mag_setup_route(get_mn_addr(bce), bce->link);
    dbg("Out of setup route\n");
    return 0;
}

/*!
*  Start sendind router advertisements
* \param bce
* \return status of the sent of RA
*/
int mag_kickoff_ra(pmip_entry_t * bce)
{
    struct in6_addr *src;
    src = malloc(sizeof(struct in6_addr));
    memset(src, 0, sizeof(struct in6_addr));
    struct iovec iov;
    struct nd_router_advert *radvert;
    adv_prefix_t prefix;
    unsigned char buff[MSG_SIZE];
    size_t len = 0;
    memset(&buff, 0, sizeof(buff));
    radvert = (struct nd_router_advert *) buff;
    radvert->nd_ra_type = ND_ROUTER_ADVERT;
    radvert->nd_ra_code = 0;
    radvert->nd_ra_cksum = 0;
    radvert->nd_ra_curhoplimit = router_ad_iface.AdvCurHopLimit;
    radvert->nd_ra_flags_reserved = (router_ad_iface.AdvManagedFlag) ? ND_RA_FLAG_MANAGED : 0;
    radvert->nd_ra_flags_reserved |= (router_ad_iface.AdvOtherConfigFlag) ? ND_RA_FLAG_OTHER : 0;
/* Mobile IPv6 ext */
    radvert->nd_ra_flags_reserved |= (router_ad_iface.AdvHomeAgentFlag) ? ND_RA_FLAG_HOME_AGENT : 0;
/* if forwarding is disabled, send zero router lifetime */
    radvert->nd_ra_router_lifetime = !check_ip6_forwarding()? htons(router_ad_iface.AdvDefaultLifetime) : 0;
    radvert->nd_ra_reachable = htonl(router_ad_iface.AdvReachableTime); //ask giuliana
    radvert->nd_ra_retransmit = htonl(router_ad_iface.AdvRetransTimer); // ask giuliana
    len = sizeof(struct nd_router_advert);
    prefix = router_ad_iface.Adv_Prefix;
/*
*  add prefix options
*/
    struct nd_opt_prefix_info *pinfo;
    pinfo = (struct nd_opt_prefix_info *) (buff + len);
    pinfo->nd_opt_pi_type = ND_OPT_PREFIX_INFORMATION;
    pinfo->nd_opt_pi_len = 4;
    pinfo->nd_opt_pi_prefix_len = prefix.PrefixLen;
    pinfo->nd_opt_pi_flags_reserved = (prefix.AdvOnLinkFlag) ? ND_OPT_PI_FLAG_ONLINK : 0;
    pinfo->nd_opt_pi_flags_reserved |= (prefix.AdvAutonomousFlag) ? ND_OPT_PI_FLAG_AUTO : 0;
/* Mobile IPv6 ext */
    pinfo->nd_opt_pi_flags_reserved |= (prefix.AdvRouterAddr) ? ND_OPT_PI_FLAG_RADDR : 0;
    pinfo->nd_opt_pi_valid_time = htonl(prefix.AdvValidLifetime);
    pinfo->nd_opt_pi_preferred_time = htonl(prefix.AdvPreferredLifetime);
    pinfo->nd_opt_pi_reserved2 = 0;
    memcpy(&pinfo->nd_opt_pi_prefix, &bce->mn_prefix, sizeof(struct in6_addr));
    len += sizeof(*pinfo);
//mobile ip extension
    if (router_ad_iface.AdvHomeAgentInfo
    && (router_ad_iface.AdvMobRtrSupportFlag || router_ad_iface.HomeAgentPreference != 0 || router_ad_iface.HomeAgentLifetime != router_ad_iface.AdvDefaultLifetime)) {
    home_agent_info_t ha_info;
    ha_info.type = ND_OPT_HOME_AGENT_INFO;
    ha_info.length = 1;
    ha_info.flags_reserved = (router_ad_iface.AdvMobRtrSupportFlag) ? ND_OPT_HAI_FLAG_SUPPORT_MR : 0;
    ha_info.preference = htons(router_ad_iface.HomeAgentPreference);
    ha_info.lifetime = htons(router_ad_iface.HomeAgentLifetime);
    memcpy(buff + len, &ha_info, sizeof(ha_info));
    len += sizeof(ha_info);
    }
    iov.iov_len = len;
    iov.iov_base = (caddr_t) buff;
    int err;
    err = icmp6_send(bce->link, 255, src, &bce->mn_link_local_addr, &iov, 1);
    if (err < 0) {
    dbg("Error: couldn't send a RA message ...");
    } else
    dbg("RA LL ADDRESS sent\n");
    return err;
}

/*!
*  Check if IPv6 forwarding is set in the kernel
* \return 0 if forwarding is set
*/
int check_ip6_forwarding(void)
{
    int forw_sysctl[] = { SYSCTL_IP6_FORWARDING };
    int value;
    size_t size = sizeof(value);
    FILE *fp = NULL;
#ifdef __linux__
    fp = fopen(PROC_SYS_IP6_FORWARDING, "r");
    if (fp) {
    fscanf(fp, "%d", &value);
    fclose(fp);
    } else
    dbg("Correct IPv6 forwarding procfs entry not found, " "perhaps the procfs is disabled, " "or the kernel interface has changed?");
#endif              /* __linux__ */
    if (!fp && sysctl(forw_sysctl, sizeof(forw_sysctl) / sizeof(forw_sysctl[0]), &value, &size, NULL, 0) < 0) {
    dbg("Correct IPv6 forwarding sysctl branch not found, " "perhaps the kernel interface has changed?");
    return (0);     /* this is of advisory value only */
    }
    if (value != 1) {
    dbg("IPv6 forwarding setting is: %u, should be 1", value);
    return (-1);
    }
    return (0);
}

/*!
*  Retrieve the link local address of the MAG interface accessible to MNs
* \param src
* \return 1 if success else -1
*/
int mag_get_ingress_info(int *if_index, char *dev_name_mn_link)
{
    FILE *fp;
    char str_addr[40];
    unsigned int plen, scope, dad_status, if_idx;
    struct in6_addr addr;
    unsigned int ap;
    int i;

    char devname[32];
    if ((fp = fopen("/proc/net/if_inet6", "r")) == NULL) {
    dbg("you don't have root previleges, please logon as root, can't open %s:", "/proc/net/if_inet6");
    return -1;
    }
// first find the device name
    while (fscanf(fp, "%32s %x %02x %02x %02x %15s\n", str_addr, &if_idx, &plen, &scope, &dad_status, devname) != EOF) {
    for (i = 0; i < 16; i++) {
        sscanf(str_addr + i * 2, "%02x", &ap);
        addr.s6_addr[i] = (unsigned char) ap;
    }
    if (memcmp(&conf.mag_addr_ingress, &addr, sizeof(struct in6_addr)) == 0) {
        strcpy(dev_name_mn_link, devname);
        *if_index = if_idx;
        dbg("The interface name of the device that is used for communicate with MNs is %s\n", dev_name_mn_link);
        fclose(fp);
        return 1;
    }
    }
    fclose(fp);
    dbg("No interface name of the device that is used for communicate with MNs found");
    return -1;
}

/*!
*  Retrieve the link local address of the MAG interface accessible to MNs
* \param src
* \return 1 if success else -1
*/
int setup_linklocal_addr(struct in6_addr *src)
{
    FILE *fp;
    char str_addr[40];
    unsigned int plen, scope, dad_status, if_idx;
//char devname[IFNAMSIZ];
    struct in6_addr addr;
    unsigned int ap;
    int i;
    int flagy = 0;
    char devname[32];
    char dev_name_mn_link[32];
    if ((fp = fopen("/proc/net/if_inet6", "r")) == NULL) {
    dbg("you don't have root previleges, please logon as root, can't open %s:", "/proc/net/if_inet6");
    return -1;
    }
// first find the device name
    while (fscanf(fp, "%32s %x %02x %02x %02x %15s\n", str_addr, &if_idx, &plen, &scope, &dad_status, devname) != EOF) {
    for (i = 0; i < 16; i++) {
        sscanf(str_addr + i * 2, "%02x", &ap);
        addr.s6_addr[i] = (unsigned char) ap;
    }
    if (memcmp(&conf.mag_addr_ingress, &addr, sizeof(struct in6_addr))
        == 0) {
        strcpy(dev_name_mn_link, devname);
        flagy = 1;
        dbg("The interface name of the device that is used for communicate with MNs is %s\n", dev_name_mn_link);
        break;
    }
    }
    fclose(fp);
    if ((fp = fopen("/proc/net/if_inet6", "r")) == NULL) {
    dbg("can't open %s:", "/proc/net/if_inet6");
    return -1;
    }
    while (fscanf(fp, "%32s %x %02x %02x %02x %15s\n", str_addr, &if_idx, &plen, &scope, &dad_status, devname) != EOF) {
    if (scope == IPV6_ADDR_LINKLOCAL && strcmp(devname, dev_name_mn_link) == 0) //we have to store the interface name from which we get the router solicitation
    {
        dbg("entered the if to get %s iface ll address ", dev_name_mn_link);
        flagy = 1;
        for (i = 0; i < 16; i++) {
        sscanf(str_addr + i * 2, "%02x", &ap);
        addr.s6_addr[i] = (unsigned char) ap;
        }
        dbg("PMIP cache entry is found for: %x:%x:%x:%x:%x:%x:%x:%x \n", NIP6ADDR(&addr));
        *src = addr;
    }
    }
    if (flagy == 0) {
    dbg("no link local address configured ");
    fclose(fp);
    return -1;
    } else {
    fclose(fp);
    return 1;
    }
}

/*!
*  Update a bce with informations
* \param bce
* \param info
* \return always 0
*/
int mag_update_binding_entry(pmip_entry_t * bce, msg_info_t * info)
{
    dbg("Store binding entry\n");
    bce->our_addr = conf.our_addr;
    bce->mn_suffix = info->mn_iid;
    bce->mn_hw_address = eth_address2hw_address(info->mn_iid);
    bce->mn_prefix = info->mn_prefix;
    bce->mn_addr = info->mn_addr;
    bce->mn_link_local_addr = info->mn_link_local_addr;
    bce->mn_serv_mag_addr = info->src;
    bce->lifetime = info->lifetime;
    bce->n_rets_counter = conf.Max_Rets;
    bce->seqno_in = info->seqno;
    bce->link = info->iif;
    return 0;
}

/*!
*  Start movement detection for a MN
* \param info
* \param bce
* \param flag
* \return always 0
*/
int mag_pmip_md(msg_info_t * info, pmip_entry_t * bce)
{
    if (bce != NULL) {
    bce->our_addr = conf.our_addr;
    bce->mn_suffix = info->mn_iid;
    bce->mn_prefix = info->mn_prefix;   //hip
    dbg("Making BCE entry in MAG with HN prefix  %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&bce->mn_prefix));
    bce->mn_serv_mag_addr = conf.our_addr;
    bce->mn_serv_lma_addr = conf.lma_addr;
    bce->seqno_out = 0;
    bce->PBU_flags = IP6_MH_BU_ACK | IP6_MH_PBU;
    bce->link = info->iif;
    struct in6_addr *link_local = link_local_addr(&bce->mn_suffix);
    bce->mn_link_local_addr = *link_local;  // link local address of MN
    dbg("New attachment detected! Start Location Registration procedure...\n");
    bce->type = BCE_TEMP;
    mag_start_registration(bce);
    return 0;
	}
}
