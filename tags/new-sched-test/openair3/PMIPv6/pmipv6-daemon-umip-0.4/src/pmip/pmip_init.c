/*! \file pmip6d.c
* \brief The main PMIP6D file
* \author OpenAir3 Group
* \date 12th of October 2010
* \version 1.0
* \company Eurecom
* \project OpenAirInterface
* \email: openair3@eurecom.fr
*/
#define PMIP
#define PMIP_INIT_C
//---------------------------------------------------------------------------------------------------------------------
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <unistd.h>
//---------------------------------------------------------------------------------------------------------------------
#include "pmip_cache.h"
#include "pmip_fsm.h"
#include "pmip_handler.h"
#include "pmip_hnp_cache.h"
#include "pmip_init.h"
#include "pmip_lma_proc.h"
#include "pmip_mag_proc.h"
#include "pmip_msgs.h"
#include "pmip_pcap.h"
#include "pmip_tunnel.h"
#include "pmip_types.h"
//---------------------------------------------------------------------------------------------------------------------
#include "rtnl.h"
#include "tunnelctl.h"
#ifdef ENABLE_VT
#    include "vt.h"
#endif
#include "debug.h"
#include "conf.h"

#define IPV6_ALL_SOLICITED_MCAST_ADDR 68
//---------------------------------------------------------------------------------------------------------------------
extern struct sock icmp6_sock;
//---------------------------------------------------------------------------------------------------------------------
void init_mag_icmp_sock(void)
//---------------------------------------------------------------------------------------------------------------------
{
	if (0) {
		int on = 1;
		dbg("Set SOLRAW, IPV6_ALL_SOLICTED_MCAST_ADDR = %d\n", IPV6_ALL_SOLICITED_MCAST_ADDR);
		if (setsockopt(icmp6_sock.fd, SOL_RAW, IPV6_ALL_SOLICITED_MCAST_ADDR, &on, sizeof(on)) < 0) {
			perror("allow all solicited mcast address\n");
		}
	}
}
//---------------------------------------------------------------------------------------------------------------------
static int pmip_cache_delete_each(void *data, void *arg)
//---------------------------------------------------------------------------------------------------------------------
{
    pmip_entry_t *bce = (pmip_entry_t *) data;
    if (is_mag()) {
        //Delete existing route & rule for the deleted MN
        mag_remove_route(&bce->mn_addr, bce->link);
        int usercount = tunnel_getusers(bce->tunnel);
        dbg("# of binding entries %d \n", usercount);
        if (usercount == 1) {
            route_del(bce->tunnel, RT6_TABLE_PMIP, IP6_RT_PRIO_MIP6_FWD, &in6addr_any, 0, &in6addr_any, 0, NULL);
        }
        //decrement users of old tunnel.
        pmip_tunnel_del(bce->tunnel);
    }
    //Delete existing route for the deleted MN
    if (is_lma()) {
        lma_remove_route(&bce->mn_addr, bce->tunnel);
        //decrement users of old tunnel.
        pmip_tunnel_del(bce->tunnel);
    }
    //Delete the Entry.
    free_iov_data((struct iovec *) &bce->mh_vec, bce->iovlen);
    pmip_bce_delete(bce);
    return 0;
}
//---------------------------------------------------------------------------------------------------------------------
void pmip_cleanup(void)
//---------------------------------------------------------------------------------------------------------------------
{
    //Release the pmip cache ==> deletes the routes and rules and "default route on PMIP" and tunnels created.
    dbg("Release all occupied resources...\n");
    //delete the default rule.
    dbg("Remove default rule...\n");
    rule_del(NULL, RT6_TABLE_MIP6, IP6_RULE_PRIO_MIP6_FWD, RTN_UNICAST, &in6addr_any, 0, &in6addr_any, 0, 0);

    dbg("Release pmip_cache...\n");
    pmip_cache_iterate(pmip_cache_delete_each, NULL);

//#undef HAVE_PCAP_BREAKLOOP
#define HAVE_PCAP_BREAKLOOP
#ifdef HAVE_PCAP_BREAKLOOP
    /*
    * We have "pcap_breakloop()"; use it, so that we do as little
    * as possible in the signal handler (it's probably not safe
    * to do anything with standard I/O streams in a signal handler -
    * the ANSI C standard doesn't say it is).
    */
    if (is_mag()) {
        pcap_breakloop(pcap_descr);
    }
#endif
}

//---------------------------------------------------------------------------------------------------------------------
int pmip_common_init(void)
//---------------------------------------------------------------------------------------------------------------------
{
    /**
    * Probe for the local address
	**/
    int probe_fd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (probe_fd < 0) {
        perror("socket");
        exit(2);
    }
    unsigned int alen;
    struct sockaddr_in6 host;
    struct sockaddr_in6 firsthop;

    memset(&firsthop, 0, sizeof(firsthop));
    firsthop.sin6_port = htons(1025);
    firsthop.sin6_family = AF_INET6;
    if (connect(probe_fd, (struct sockaddr *) &firsthop, sizeof(firsthop)) == -1) {
        perror("connect");
        return -1;;
    }
    alen = sizeof(host);
    if (getsockname(probe_fd, (struct sockaddr *) &host, &alen) == -1) {
        perror("probe getsockname");
        return -1;;
    }
    close(probe_fd);


    /**
    * Initializes PMIP cache.
    **/
    if (pmip_cache_init() < 0) {
        dbg("PMIP Binding Cache initialization failed! \n");
        return -1;
    } else {
        dbg("PMIP Binding Cache is initialized!\n");
    }
    /**
    * Adds a default rule for RT6_TABLE_MIP6.
    */
    dbg("Add default rule for RT6_TABLE_MIP6\n");
    if (rule_add(NULL, RT6_TABLE_MIP6, IP6_RULE_PRIO_MIP6_FWD, RTN_UNICAST, &in6addr_any, 0, &in6addr_any, 0, 0) < 0) {
        dbg("Add default rule for RT6_TABLE_MIP6 failed, insufficient privilege/kernel options missing!\n");
        return -1;
    }
	return 0;
}
//---------------------------------------------------------------------------------------------------------------------
int pmip_mag_init(void)
//---------------------------------------------------------------------------------------------------------------------
{
	pmip_common_init();
	conf.OurAddress = conf.MagAddressEgress;
	conf.HomeNetworkPrefix = get_node_prefix(&conf.MagAddressIngress); //copy Home network prefix.
	dbg("Running as MAG entity\n");
	dbg("Entity Egress Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&conf.OurAddress));
	dbg("Entity Ingress Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&conf.MagAddressIngress));
	dbg("Home Network Prefix Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&conf.HomeNetworkPrefix));
	if (mag_init_fsm() < 0) {
		dbg("Initialization of FSM failed...exit\n");
		exit(-1);
	}

	init_pbu_sequence_number();

	init_iface_ra();
	init_mag_icmp_sock();
	dbg("Initializing the NA handler\n");
	// to capture NA message
	icmp6_handler_reg(ND_NEIGHBOR_ADVERT, &pmip_mag_recv_na_handler);
	dbg("Initializing the RS handler\n");
	// to capture RS message
	icmp6_handler_reg(ND_ROUTER_SOLICIT, &pmip_mag_rs_handler);
	dbg("Initializing the PBA handler\n");
	//To capture PBA message.
	mh_handler_reg(IP6_MH_TYPE_BACK, &pmip_mag_pba_handler);

	/**
	* Deletes the default route for MN prefix so routing is per unicast MN address!
	**/
	//route_del((int) NULL, RT6_TABLE_MAIN, IP6_RT_PRIO_ADDRCONF, &in6addr_any, 0, &conf.HomeNetworkPrefix, 64, NULL);
	dbg("Initializing the HNP cache\n");
	if (pmip_mn_to_hnp_cache_init() < 0) {
		exit (-1);
	}

    char devname[32];
	int iif;
	dbg("Getting ingress informations\n");
	mag_get_ingress_info(&iif, devname);
	dbg("Starting capturing AP messages for incoming MNs detection\n");
	pmip_pcap_loop(devname, iif);
	return 0;
}
//---------------------------------------------------------------------------------------------------------------------
int pmip_lma_init(void)
//---------------------------------------------------------------------------------------------------------------------
{
	pmip_common_init();
	pmip_lma_mn_to_hnp_cache_init();
	conf.OurAddress = conf.LmaAddress;
	dbg("Entity Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&conf.OurAddress));
	dbg("Initializing the PBU handler\n");
	//To capture PBU message.
	mh_handler_reg(IP6_MH_TYPE_BU, &pmip_lma_pbu_handler);
	return 0;
}
