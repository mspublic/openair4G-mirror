/*****************************************************************
 * C Implementation: pmip_mag_proc
 * Description: 
 * Author: 
 *   Christian Bonnet
 *   Huu-Nghia Nguyen
 * Copyright: Eurecom Institute, (C) 2008
 ******************************************************************/

#include "prefix.h"
#include "mh.h"
#include "debug.h"
#include "conf.h"
#include "pmip_cache.h"
#include "pmip_consts.h"
#include "rtnl.h"
#include "tunnelctl.h"
#include <pthread.h>
#include "pmip_ro_cache.h"
#include "pmip_extern.h"

void pmip_timer_retrans_pbu_handler(struct tq_elem *tqe);
extern uint16_t seqno_pbreq;

//===========================================================
int mag_setup_route(struct in6_addr * pmip6_addr, int downlink)
//===========================================================
{
	int res = 0;
	if (conf.tunneling_enabled)
	{
		//add a rule for MN for uplink traffic from MN must querry the TABLE for PMIP --> tunneled
		dbg("Uplink: Add new rule for tunneling src=%x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(pmip6_addr)); 
		res = rule_add(NULL, RT6_TABLE_PMIP, IP6_RULE_PRIO_PMIP6_FWD, RTN_UNICAST, pmip6_addr, 128, &in6addr_any, 0);
		//add a route for downlink traffic through CH (any src) ==> MN
		dbg("Downlink: Add new route for %x:%x:%x:%x:%x:%x:%x:%x in table %d\n", NIP6ADDR(pmip6_addr), RT6_TABLE_MIP6);
		res |= route_add(downlink, RT6_TABLE_MIP6, RTPROT_MIP,0, IP6_RT_PRIO_MIP6_FWD,&in6addr_any, 0, pmip6_addr, 128,NULL);
	}
	return res;
}
//===========================================================
int mag_remove_route(struct in6_addr * pmip6_addr, int downlink)
//=========================================================== 
{
	int res = 0;
	if (conf.tunneling_enabled)
	{
		//Delete existing rule for the deleted MN
		dbg("Uplink: Delete old rule for tunneling src=%x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(pmip6_addr));
		res = rule_del(NULL, RT6_TABLE_PMIP, IP6_RULE_PRIO_PMIP6_FWD, RTN_UNICAST, pmip6_addr, 128, &in6addr_any, 0);
		//Delete existing route for the deleted MN
		dbg("Downlink: Delete old routes for: %x:%x:%x:%x:%x:%x:%x:%x from table %d\n", NIP6ADDR(pmip6_addr), RT6_TABLE_MIP6);	
		res |= route_del(downlink, RT6_TABLE_MIP6, IP6_RT_PRIO_MIP6_FWD,&in6addr_any, 0, pmip6_addr, 128,NULL);
	}
	return res;
}

//===========================================================
int mag_dereg(struct pmip_entry * bce, int propagate)
//===========================================================
{
	//Delete existing route & rule for the deleted MN
	int res = 0;
	res = mag_remove_route(get_mn_addr(bce), bce->link);
	int usercount = tunnel_getusers(bce->tunnel);
	dbg("# of binding entries %d \n", usercount);
	if (usercount == 1)
		route_del(bce->tunnel, RT6_TABLE_PMIP, IP6_RT_PRIO_MIP6_FWD,&in6addr_any, 0,&in6addr_any, 0,NULL);

	//decrement users of old tunnel.
	pmip_tunnel_del(bce->tunnel);
	
	if (propagate)
	{
		struct in6_addr_bundle addrs;
		addrs.src= &conf.our_addr;
		addrs.dst= &conf.lma_addr;			
		struct timespec Lifetime = {0,0};
		dbg("Create PBU for CH to delete the PMIP entry too....\n");
		mh_send_pbu(&addrs, bce,&Lifetime, 0);
	}	

	//Proxy Arp for away from link MN
	res |= mag_dereg_update_proxy_ndisc(bce);

	//Delete PBU cache entry
	dbg("Delete PBU entry....\n");
	pmipcache_release_entry(bce);
	pmip_bce_delete(bce);	
	return res;	
}

//===========================================================
int mag_dereg_update_proxy_ndisc(struct pmip_entry * bce)
//===========================================================
{
	if (conf.pndisc_enabled)
	{
		dbg("Do proxy ARP for the away-from-link MN (%x:%x:%x:%x:%x:%x:%x:%x)\n", NIP6ADDR(get_mn_addr(bce)));	
		proxy_nd_start(bce->link, get_mn_addr(bce), &conf.mag_addr_ingress, 0);
	}
	return 0;
}

//===========================================================
int mag_reg_update_proxy_ndisc(struct pmip_entry * bce)
//===========================================================
{
	if (conf.pndisc_enabled)
	{			
		//Delete Proxy ARP for this MN 
		dbg("Stop proxy ARP for the returning-link MN (%x:%x:%x:%x:%x:%x:%x:%x)\n", NIP6ADDR(get_mn_addr(bce)));	
		proxy_nd_stop(bce->link, get_mn_addr(bce), 0);
		
		dbg("Do proxy ARP for the every MR(s)\n");
		dbg("HARD CODED!\n");
		int mr_count = 2;
		int i;        
		struct in6_addr mr_list[2];
		struct in6_addr mr_ll[2];
		inet_pton(AF_INET6, "fe80::fcfd:ff:fe00:300", &mr_ll[0]);
		inet_pton(AF_INET6, "fe80::fcfd:ff:fe00:400", &mr_ll[1]);
		inet_pton(AF_INET6, "2001:1::1", &mr_list[0]);
		inet_pton(AF_INET6, "2001:1::2", &mr_list[1]);
		for (i=0; i< mr_count; i++)
		{
			if ( !IN6_ARE_ADDR_EQUAL(&conf.mag_addr_ingress, &mr_list[i]))
			{
				dbg("Proxy ARP for home link router!\n");
				proxy_nd_stop(bce->link, &mr_ll[i], 0);
				proxy_nd_start(bce->link, &mr_ll[i], &conf.mag_addr_ingress, 0);
			}
		}
	
	#if 0
		//Find out all on-going communication.
		//Do the Proxy Arp for the peer addresses
		dbg("Do proxy ARP for the away-from-link peer MN(s)\n");
		dbg("HARD CODED!\n");
		struct in6_addr peer_list[2];
		//int i;
		inet_pton(AF_INET6, "::fcfd:ff:fe00:500", &peer_list[0]);
		inet_pton(AF_INET6, "::fcfd:ff:fe00:600", &peer_list[1]);
	
		struct in6_addr cn_addr;
		inet_pton(AF_INET6, "2000::1", &cn_addr);
		proxy_nd_start(bce->link, &cn_addr, &conf.mag_addr_ingress, 0);
		for (i=0; i<2; i++)
		{
			if ( !IN6_ARE_ADDR_EQUAL(&bce->mn_iid, &peer_list[i]) )
			{ 
				int exist = pmip_cache_exists(&conf.our_addr, &peer_list[i]);
				if (exist != BCE_PMIP ) 
				{
					struct in6_addr pmip6_addr;
					pmip6_addr = * ID2ADDR(&bce->mn_prefix, &peer_list[i]);
					
					dbg("Do proxy ARP for the away-from-link node (%x:%x:%x:%x:%x:%x:%x:%x)\n", NIP6ADDR(&pmip6_addr));	
					proxy_nd_start(bce->link, &pmip6_addr, &conf.mag_addr_ingress, 0);			
				}
			}
		}
	#endif
	}
	return 0;
}

//===========================================================
int mag_pmip_detect_ro(struct msg_info* info)
//===========================================================
{
		struct pmip_ro_entry* de = NULL;
        de = pmip_ro_cache_get(&info->src, &info->ns_target); //TODO our_addr must be replaced by saddr
		if (de == NULL)
		{ 		
			de = pmip_ro_cache_alloc();
			dbg("test 4");
			if (de == NULL) return -1;

			de->src_addr = info->src;
			de->dst_addr = info->ns_target;
			if (pmip_ro_cache_add(de) != NULL)
			{
				pthread_rwlock_rdlock(&pmip_ro_cache_lock);
				pthread_rwlock_wrlock(&de->lock);
				//TODO Start dcache timer
			}

			de->sender_addr = info->src;
			de->src_serv_mag_addr = conf.our_addr;
			de->dst_iid = info->mn_iid;				
			de->iif = info->iif;
			de->action = PBREQ_LOCATE; //To be explicit.
		}
		else
		{
		         //send an NA as ARP reply.
		         dbg("Create NA as proxy arp for %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&de->dst_addr));
		         uint32_t na_flags = NDP_NA_SOLICITED | NDP_NA_OVERRIDE;
		         ndisc_send_na(info->iif, &conf.mag_addr_ingress, &de->src_addr, &de->dst_addr,na_flags);	
		}

		//TODO Check lifetime of RO cache entry. If it is invalide then  Send PBREQ --> CH & start sending NA after a Timeout			
		dbg("No RO entry found for %x:%x:%x:%x:%x:%x:%x:%x ... Send PBREQ to serving CH\n", NIP6ADDR(&de->dst_addr)); 					
		struct in6_addr_bundle address;
		address.src = &conf.our_addr;
		address.dst = &conf.lma_addr; 
		mh_send_pbreq(&address, &de->dst_iid, &de->dst_addr, &de->src_serv_mag_addr, &de->src_addr, seqno_pbreq, PBREQ_LOCATE, 0, NULL); //send PBREQ to CH.					
		seqno_pbreq++;

		//send an NA as ARP reply.
		//dbg("Create NA as proxy arp for %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&de->dst_addr));
		//uint32_t na_flags = NDP_NA_SOLICITED | NDP_NA_OVERRIDE;
		//ndisc_send_na(info->iif, &conf.mag_addr_ingress, &de->src_addr, &de->dst_addr,na_flags);	

		if (de != NULL) pmip_ro_cache_release_entry(de);
  		return 0;
}

//===========================================================
//Start the Location Registration Procedure 
int mag_start_registration(struct pmip_entry* bce)
//===========================================================
{
	//Create PBU and send to the LMA
	struct in6_addr_bundle addrs;
	addrs.src= &conf.mag_addr_egress;
	addrs.dst= &conf.lma_addr;	
	mh_send_pbu(&addrs,bce,&conf.PBU_LifeTime,0);
	
	//add a new task for PBU retransmission.
	struct timespec expires;
	clock_gettime(CLOCK_REALTIME, &bce->add_time);
	tsadd(bce->add_time, conf.N_RetsTime, expires);
	add_task_abs(&expires, &bce->tqe, pmip_timer_retrans_pbu_handler);
	dbg("PBU Retransmissions Timer is registered....\n");
	return 0;
}

//===========================================================
int mag_end_registration(struct pmip_entry* bce, int iif)
//===========================================================
{
	//Change the BCE type.
	bce->type = BCE_PMIP;
	dbg("New PMIP cache entry type: %d\n",bce->type);
	//Reset the Retransmissions counter.
	bce->n_rets_counter = conf.Max_Rets;

	//Add task for entry expiry.
	dbg("Timer for Expiry is intialized: %d(s)!\n", bce->lifetime.tv_sec);
	pmip_cache_start(bce);
			
	//create a tunnel between MAG and LMA.
	bce->tunnel = pmip_tunnel_add(&conf.our_addr, &conf.lma_addr, iif);	
	int usercount = tunnel_getusers(bce->tunnel);
	dbg("# of binding entries %d \n", usercount);
	if (usercount == 1) 
	{
		dbg("Add routing entry for uplink traffic");
		route_add(bce->tunnel, RT6_TABLE_PMIP, RTPROT_MIP,0, IP6_RT_PRIO_MIP6_FWD,&in6addr_any, 0,&in6addr_any, 0,NULL);
	}			
	mag_setup_route(get_mn_addr(bce), bce->link);
	mag_reg_update_proxy_ndisc(bce);
	return 0;
}

int mag_update_binding_entry(struct pmip_entry * bce, struct msg_info * info)
{
	dbg("Store binding entry\n");
	bce->our_addr = conf.our_addr;
	bce->mn_iid = info->mn_iid;
	bce->mn_prefix = conf.Home_Network_Prefix;
	bce->mn_addr = info->mn_addr;
	bce->mn_link_local_addr = info->mn_link_local_addr;
	bce->mn_serv_mag_addr = info->src;
	bce->lifetime = info->lifetime;
	bce->n_rets_counter = conf.Max_Rets;
	bce->seqno_in = info->seqno;
	bce->link = info->iif;
	return 0;
}

//===========================================================
int mag_pmip_md(struct msg_info* info, struct pmip_entry *bce)
//===========================================================
{
	if (bce != NULL)	
	{
		bce->our_addr = conf.our_addr;
		bce->mn_iid = info->mn_iid; 
		bce->mn_prefix = conf.Home_Network_Prefix;
		bce->mn_serv_mag_addr = conf.our_addr;
		bce->mn_serv_lma_addr = conf.lma_addr;					
		bce->seqno_out = 0;
		bce->PBU_flags = IP6_MH_BU_ACK | IP6_MH_PBU;
		bce->link = info->iif;
		struct in6_addr *link_local = link_local_addr(&info->mn_iid);
		bce->mn_link_local_addr = *link_local;   // link local address of MN
	}

	//If we don't have any info & the prefix is not the same as the PMIP6 daemon prefix
	//we consider this as a hint for the network_based movment detection
	//Otherwise, we send PBU to the LMA
	if (ipv6_pfx_cmp(&info->ns_target, &conf.Home_Network_Prefix, PLEN))
	{
		dbg("Start Network-based Movement Detection for:%x:%x:%x:%x:%x:%x:%x:%x iif=%d\n", NIP6ADDR(&bce->mn_iid), bce->link);

		//Add a Temporary Hint Entry to the PMIP cache.
		bce->type = BCE_HINT;			

		//Create NS for Network-based Movment Detection!		
		struct in6_addr mn_addr;
		CONVERT_ID2ADDR(&mn_addr, &conf.Home_Network_Prefix, &bce->mn_iid);
		ndisc_send_ns(info->iif, &conf.mag_addr_ingress, solicited_mcast(&mn_addr), &mn_addr);
		
		//Activate Timer for NS to delete the temporary entry if there is no answer
		bce->n_rets_counter = 2;
		pmip_cache_start(bce);
	}
	else if (!ipv6_pfx_cmp(&info->ns_target, &conf.Home_Network_Prefix, PLEN))
	{			
		//NS from same pmip6 prefix, in any case (Hint/Not exist) --> upgrade to BCE_TEMP and send PBU
		dbg("New attachment detected! start Location Registration procedure...\n");
		bce->type = BCE_TEMP;			
		mag_start_registration(bce);
	}
	return 0;
}
