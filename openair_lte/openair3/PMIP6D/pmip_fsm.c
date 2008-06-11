/*****************************************************************
 * C Implementation: pmip_fsm
 * Description: 
 * Author: 
 *   Christian Bonnet
 *   Huu-Nghia Nguyen
 *   Hussain & Daniel
 * Copyright: Eurecom Institute, (C) 2008
 ******************************************************************/
#include "pmip_extern.h"
#include "mh.h"
#include "debug.h"
#include "conf.h"
#include "pmip_cache.h"
#include "pmip_consts.h"
#include "rtnl.h"
#include "tunnelctl.h"
#include <pthread.h>

extern pthread_rwlock_t pmip_lock; /* Protects proxy binding cache */

/**
	Finite State Machine; return 0 for success and -1 if no entry exists, pointer to NULL.
*/

int pmip_fsm(struct in6_addr_bundle *addresses, struct pmip_entry *info, int iif)
{
	
	if(!info)
	{
		dbg("No Existing Entry is found!\n");
		return -1; 
	}
	
	
	if(is_mag())
	{
		uint32_t status = info->FLAGS & hasPBA;
		if(status == hasPBA)
		{
			dbg("hasPBA: %d\n",hasPBA);
			dbg("FLAGS: %d\n",info->FLAGS);
			dbg("Proxy Binding Update Lifetime: %d\n",info->lifetime.tv_sec);
			
			//Change the BCE type.
			info->type = BCE_PMIP;
			dbg("New PMIP cache entry type: %d\n",info->type);
			//Reset the Retransmissions counter.
			info->n_rets_counter = conf.Max_Rets;
			//Add task for entry expiry.
			pmip_cache_start(info);
			dbg("Timer for Expiry is intialized!\n");
			
			//create a tunnel between MAG and LMA.
			info->tunnel = tunnel_add(&conf.our_addr,&conf.lma_addr,iif,0,0);

			int usercount = tunnel_getusers(info->tunnel);
			dbg("# of binding entries %d \n", usercount);
			if (usercount == 1) {
				dbg("Add routing entry for uplink traffic");
				route_add(info->tunnel, RT6_TABLE_PMIP, RTPROT_MIP,0, IP6_RT_PRIO_MIP6_FWD,&in6addr_any, 0,&in6addr_any, 0,NULL);
			}

			//TODO add a rule for MN
			//rule_add(NULL, RT6_TABLE_PMIP,IP6_RULE_PRIO_PMIP6_FWD, RTN_UNICAST,ID2ADDR(&info->peer_prefix,&info->peer_addr), 128, &in6addr_any, 0);
			//add a route CH (any src) ==> MN
			//route_add(info->link, RT6_TABLE_MIP6, RTPROT_MIP,0, IP6_RT_PRIO_MIP6_FWD,&in6addr_any, 0,ID2ADDR(&info->peer_prefix,&info->peer_addr), 128,NULL);
			mag_setup_route(ID2ADDR(&info->peer_prefix,&info->peer_addr), info->link);

			mag_reg_update_proxy_ndisc(info);
		
			pmipcache_release_entry(info);

		}

		status = 0;
		status = info->FLAGS & hasPBU;
		if(status == hasPBU)
		{
			dbg("Checking the Cache for existing entry....\n");					
			int exist = pmip_cache_exists(&conf.our_addr,&info->peer_addr);			
			if(info->lifetime.tv_sec ==0 && info->lifetime.tv_nsec==0 && exist==BCE_PMIP || exist==BCE_TEMP)
			{		
				struct pmip_entry *bce;
				bce = pmip_cache_get(&conf.our_addr,&info->peer_addr); 
				mag_dereg(bce, 0);
			}	
		}


		status = 0;
		status = info->FLAGS & hasPBREQ;
		if(status == hasPBREQ)
		{
			dbg("Create PBRES message...\n");

			struct in6_addr_bundle addrs;
			addrs.src = addresses->dst;
			addrs.dst = addresses->src;
			
			//create a PB response.
			mh_send_pbres(&addrs,&info->peer_addr,&info->LMA_addr,&info->Serv_MAG_addr,&info->peer_prefix,info->seqno,iif);
			pmipcache_release_entry(info);
		}

		status = 0;
		status = info->FLAGS & hasPBRES;
		if(status == hasPBRES)
		{
			//TODO
			// creat a tunnel for the CN under another MR!!
			dbg("Route Optimization!!\n");
			
		}

	return 0;
	}

	
	if(is_lma())
	{
		
		uint32_t status =info->FLAGS & hasPBU;
		if(status == hasPBU )
		{
			dbg("Checking the Cache for existing entry....\n");
						
			int exist = pmip_cache_exists(&conf.our_addr,&info->peer_addr);
			
			if(info->lifetime.tv_sec ==0 && info->lifetime.tv_nsec==0) 
				{
					if (exist==BCE_PMIP || exist==BCE_TEMP) 
					// if returns a type equal to 5 or 6 ==> means there is a match.
					{
					dbg("MN entry exists!\n");
					dbg("Received PBU message with Lifetime = 0...\n");
					
					struct pmip_entry *bce;
					bce = pmip_cache_get(&conf.our_addr,&info->peer_addr);
					
					if(COMPARE(addresses->src, &bce->Serv_MAG_addr) == 0)
					{
						dbg("received PBU from Old Serving MAG!!!\n");
						dbg("Delete old route for: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(ID2ADDR(&bce->peer_prefix,&bce->peer_addr)));

						//delete old route.
						lma_remove_route(ID2ADDR(&bce->peer_prefix,&bce->peer_addr), bce->tunnel); 						

						//decrement users of old tunnel.
						tunnel_del(bce->tunnel,0,0);

						dbg("Delete PBU entry....\n");
						pmipcache_release_entry(bce);
						pmip_bce_delete(bce);
						dbg("PBU entry deleted....\n");
						return 0;	
					}
					pmipcache_release_entry(bce);
					}
					else 
					{
						dbg("lifetime = 0 with NO existing entry!!!\n");
						return 0;					
					}
					
				}			
			else if (exist!=BCE_PMIP && exist!=BCE_TEMP)  //NO EXISTING ENTRY FOUND ==> CREATE A NEW ONE
				{			
					dbg("New MN is found....\n");
					//set the type of entry to PMIP.
					info->type = BCE_PMIP;
					
					dbg("info->our_addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&info->our_addr));
					dbg("info->Serv_MAG : %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&info->Serv_MAG_addr));

					struct pmip_entry *bce;
					bce = pmip_cache_alloc(BCE_PMIP);
					if (bce != NULL)	
					{
						*bce = *info;
						pmip_cache_add(bce);
					}
	
					//create the address bundle for PBA.
					dbg("Create PBA...\n");
					struct in6_addr_bundle addrs;
					struct in6_addr src, dst;
					src = info->our_addr;
					dst = info->Serv_MAG_addr;
					addrs.src= &src;
					addrs.dst= &dst;
					dbg("addrs.src Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(addrs.src));
					dbg("addrs.dst Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(addrs.dst));
										
					//PBU was Accepted!
					bce->status = 0;
					//send PBA.
					mh_send_pba(&addrs,info, 0);
					
					dbg("Number of lock ref %d \n", pmip_lock);

					//create a tunnel between MAG and LMA.
					bce->tunnel = tunnel_add(&conf.our_addr,&info->Serv_MAG_addr,iif,0,0);

					//add a route for peer address (incoming packets)
					//route_add(bce->tunnel, RT6_TABLE_MIP6, RTPROT_MIP,0, IP6_RT_PRIO_MIP6_FWD,&in6addr_any, 0,ID2ADDR(&bce->peer_prefix,&bce->peer_addr), 128,NULL);
					lma_setup_route(ID2ADDR(&bce->peer_prefix,&bce->peer_addr), bce->tunnel);

					#ifdef TEST_PMIP
					dbg("Sending the message pba through netbuf\n");
					extern uint8_t netbuf[3000];
					extern uint32_t netlen;		
					pmip_mag_pba_handler.recv((struct ip6_mh *) netbuf, netlen, &addrs, 0);
					#endif	
				
				}
			else {
				// life time is not zero and there is an existing entry ==> check Serv_MAG, if not same ==> modify it.
				dbg("PBU for an existing entry ==> old MN\n");
				struct pmip_entry *bce;
				bce = pmip_cache_get(&conf.our_addr,&info->peer_addr);
				
				if(COMPARE(&info->Serv_MAG_addr, &bce->Serv_MAG_addr) == -1)
				{
					// MN moved to new  serving MAG.
					//Delete the Task
					del_task(&bce->tqe);

					lma_dereg_old_mag(bce);

					dbg("Delete old route for: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(ID2ADDR(&bce->peer_prefix,&bce->peer_addr)));

					//delete old route.
					lma_remove_route(ID2ADDR(&bce->peer_prefix,&bce->peer_addr), bce->tunnel);					

					//decrement users of old tunnel.
					tunnel_del(bce->tunnel,0,0);

					dbg("Old MN with new Serving MAG...\n");
					memcpy(&bce->Serv_MAG_addr,&info->Serv_MAG_addr,sizeof(struct in6_addr));
					bce->lifetime = info->lifetime;
					bce->n_rets_counter = conf.Max_Rets;
					//bce->link = iif;
					//Add task for entry expiry.
					pmip_cache_start(bce);

					//send a pba to new serving mag.
					dbg("Create PBA to new Serving MAG...\n");
					struct in6_addr_bundle addrs;
					addrs.src= addresses->dst;
					addrs.dst= addresses->src;
					mh_send_pba(&addrs,bce, 0);


					//create a tunnel between MAG and LMA.
					bce->tunnel = tunnel_add(&conf.our_addr,&info->Serv_MAG_addr,iif,0,0);

					//add a route for peer address.
					lma_setup_route(ID2ADDR(&info->peer_prefix,&info->peer_addr), bce->tunnel);
				}
				//esle if the same Serv MAG ==> no change to the entry.		
				pmipcache_release_entry(bce);
			}

			return 0;
			
		}
		status = 0;
		status = info->FLAGS & hasPBREQ;
		if(status == hasPBREQ)
		{
			dbg("Create PBRE message...\n");

			struct in6_addr_bundle addrs;
			addrs.src = addresses->dst;
			addrs.dst = addresses->src;
			
			//create a PB response.
			mh_send_pbres(&addrs,&info->peer_addr,&info->LMA_addr,&info->Serv_MAG_addr,&info->peer_prefix,info->seqno,iif);
			pmipcache_release_entry(info);
		}

		status = 0;
		status = info->FLAGS & hasPBRES;
		if(status == hasPBRES)
		{
			//Delete the Task (if ANY)
			del_task(&info->tqe);
			//Reset the Retransmissions counter.
			info->n_rets_counter = conf.Max_Rets;
			//Add task for entry expiry.
			pmip_cache_start(info);
			dbg("Timer for Expiry is intialized!\n");
		}
	return 0;
	}
}	
//===========================================================
int mag_setup_route(struct in6_addr * pmip6_addr, int downlink)
//===========================================================
{
	int res = -1;
	//add a rule for MN for uplink traffic from MN must querry the TABLE for PMIP --> tunneled
	dbg("Uplink: Add new rule for tunneling src=%x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(pmip6_addr)); 
	res = rule_add(NULL, RT6_TABLE_PMIP, IP6_RULE_PRIO_PMIP6_FWD, RTN_UNICAST, pmip6_addr, 128, &in6addr_any, 0);
	//add a route for downlink traffic through CH (any src) ==> MN
	dbg("Downlink: Add new route for %x:%x:%x:%x:%x:%x:%x:%x in table %d\n", NIP6ADDR(pmip6_addr), RT6_TABLE_MIP6);
	res != route_add(downlink, RT6_TABLE_MIP6, RTPROT_MIP,0, IP6_RT_PRIO_MIP6_FWD,&in6addr_any, 0, pmip6_addr, 128,NULL);
	return res;
}
//===========================================================
int mag_remove_route(struct in6_addr * pmip6_addr, int downlink)
//=========================================================== 
{
	int res = -1;
	//Delete existing rule for the deleted MN
	dbg("Uplink: Delete old rule for tunneling src=%x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(pmip6_addr));
	res = rule_del(NULL, RT6_TABLE_PMIP, IP6_RULE_PRIO_PMIP6_FWD, RTN_UNICAST, pmip6_addr, 128, &in6addr_any, 0);
	//Delete existing route for the deleted MN
	dbg("Downlink: Delete old routes for: %x:%x:%x:%x:%x:%x:%x:%x from table %d\n", NIP6ADDR(pmip6_addr), RT6_TABLE_MIP6);	
	res != route_del(downlink, RT6_TABLE_MIP6, IP6_RT_PRIO_MIP6_FWD,&in6addr_any, 0, pmip6_addr, 128,NULL);
}

//===========================================================
int lma_setup_route(struct in6_addr * pmip6_addr, int tunnel)
//===========================================================
{
	int res = -1;
	dbg("Forward: Add new route for for %x:%x:%x:%x:%x:%x:%x:%x in table %d\n", NIP6ADDR(pmip6_addr), RT6_TABLE_MIP6);
	res = route_add(tunnel, RT6_TABLE_MIP6, RTPROT_MIP,0, IP6_RT_PRIO_MIP6_FWD, &in6addr_any, 0, pmip6_addr, 128, NULL);
	return res;
}
//===========================================================
int lma_remove_route(struct in6_addr * pmip6_addr, int tunnel)
//=========================================================== 
{
	int res = -1;
	//Delete existing rule for the deleted MN
	dbg("Forward: Delete old route for: %x:%x:%x:%x:%x:%x:%x:%x from table %d\n", NIP6ADDR(pmip6_addr), RT6_TABLE_MIP6);	
	res != route_del(tunnel, RT6_TABLE_MIP6, IP6_RT_PRIO_MIP6_FWD,&in6addr_any, 0, pmip6_addr, 128,NULL);
	return res;
}


//===========================================================
int mag_dereg(struct pmip_entry * bce, int propagate)
//===========================================================
{
	if (propagate)
	{
		struct in6_addr_bundle addrs;
		addrs.src= &conf.our_addr;
		addrs.dst= &conf.lma_addr;			
		struct timespec Lifetime = {0,0};
		bce->lifetime=Lifetime;				
		dbg("Create PBU for CH to delete the PMIP entry too....\n");
		mh_send_pbu(&addrs, bce, 0);
	}

	//Delete existing route & rule for the deleted MN
	mag_remove_route(ID2ADDR(&bce->peer_prefix,&bce->peer_addr), bce->link);
	int usercount = tunnel_getusers(bce->tunnel);
	dbg("# of binding entries %d \n", usercount);
	if (usercount == 1)
		route_del(bce->tunnel, RT6_TABLE_PMIP, IP6_RT_PRIO_MIP6_FWD,&in6addr_any, 0,&in6addr_any, 0,NULL);

	//decrement users of old tunnel.
	tunnel_del(bce->tunnel,0,0);

	//Proxy Arp for away from link MN
	mag_dereg_update_proxy_ndisc(bce);

	//Delete PBU cache entry
	dbg("Delete PBU entry....\n");
	pmipcache_release_entry(bce);
	pmip_bce_delete(bce);	
	return 0;	
}


//===========================================================
int mag_dereg_update_proxy_ndisc(struct pmip_entry * bce)
//===========================================================
{
	dbg("Do proxy ARP for the away-from-link MN (%x:%x:%x:%x:%x:%x:%x:%x)\n", NIP6ADDR(ID2ADDR(&bce->peer_prefix,&bce->peer_addr)));	
	proxy_nd_start(bce->link, ID2ADDR(&bce->peer_prefix,&bce->peer_addr), &conf.mag_addr_ingress, 0);
}

//===========================================================
int mag_reg_update_proxy_ndisc(struct pmip_entry * bce)
//===========================================================
{		
	//Delete Proxy ARP for this MN 
	dbg("Stop proxy ARP for the returning-link MN (%x:%x:%x:%x:%x:%x:%x:%x)\n", NIP6ADDR(ID2ADDR(&bce->peer_prefix,&bce->peer_addr)));	
	proxy_nd_stop(bce->link, ID2ADDR(&bce->peer_prefix,&bce->peer_addr), 0);
	
	//Find out all on-going communication.
	//Do the Proxy Arp for the peer addresses
	dbg("Do proxy ARP for the away-from-link peer MN(s)\n");
	dbg("HARD CODED!\n");
	struct in6_addr peer_list[2];
	int i;
	inet_pton(AF_INET6, "::fcfd:ff:fe00:500", &peer_list[0]);
	inet_pton(AF_INET6, "::fcfd:ff:fe00:600", &peer_list[1]);

	struct in6_addr cn_addr;
	inet_pton(AF_INET6, "2000::1", &cn_addr);
        proxy_nd_start(bce->link, &cn_addr, &conf.mag_addr_ingress, 0);
	for (i=0; i<2; i++)
	{
		if (COMPARE(&bce->peer_addr, &peer_list[i]) != 0)
		{ 
			int exist = pmip_cache_exists(&conf.our_addr, &peer_list[i]);
			if (exist != BCE_PMIP ) 
			{
				struct in6_addr pmip6_addr;
				pmip6_addr = * ID2ADDR(&bce->peer_prefix, &peer_list[i]);
				
				dbg("Do proxy ARP for the away-from-link node (%x:%x:%x:%x:%x:%x:%x:%x)\n", NIP6ADDR(&pmip6_addr));	
				proxy_nd_start(bce->link, &pmip6_addr, &conf.mag_addr_ingress, 0);			
			}
		}
	}
}

//===========================================================
int lma_dereg_old_mag(struct pmip_entry * bce)
//===========================================================
{
	dbg("Sends a PBU with lifetime = 0 to Old MAG (%x:%x:%x:%x:%x:%x:%x:%x)\n", NIP6ADDR(&bce->Serv_MAG_addr));
	struct in6_addr_bundle addrs;
	struct timespec lifetime = {0,0};
	addrs.src= &conf.lma_addr;
 	addrs.dst= &bce->Serv_MAG_addr;
	bce->lifetime=lifetime;			
	mh_send_pbu(&addrs, bce, 0);
}

