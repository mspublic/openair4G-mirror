/*****************************************************************
 * C Implementation: pmip_lma_proc
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

//===========================================================
int lma_setup_route(struct in6_addr * pmip6_addr, int tunnel)
//===========================================================
{
	int res = 0;
	if (conf.tunneling_enabled)
	{
		dbg("Forward: Add new route for for %x:%x:%x:%x:%x:%x:%x:%x in table %d\n", NIP6ADDR(pmip6_addr), RT6_TABLE_MIP6);
		res = route_add(tunnel, RT6_TABLE_MIP6, RTPROT_MIP,0, IP6_RT_PRIO_MIP6_FWD, &in6addr_any, 0, pmip6_addr, 128, NULL);
	}
	return res;
}
//===========================================================
int lma_remove_route(struct in6_addr * pmip6_addr, int tunnel)
//=========================================================== 
{
	int res = 0;
	if (conf.tunneling_enabled)
	{
		//Delete existing rule for the deleted MN
		dbg("Forward: Delete old route for: %x:%x:%x:%x:%x:%x:%x:%x from table %d\n", NIP6ADDR(pmip6_addr), RT6_TABLE_MIP6);	
		res = route_del(tunnel, RT6_TABLE_MIP6, IP6_RT_PRIO_MIP6_FWD,&in6addr_any, 0, pmip6_addr, 128,NULL);
	}
	return res;
}

//===========================================================
int lma_reg(struct pmip_entry * bce)
//===========================================================
{
	//create a tunnel between MAG and LMA && add a route for peer address.	
	bce->tunnel = pmip_tunnel_add(&conf.our_addr,&bce->mn_serv_mag_addr, bce->link);
	lma_setup_route(get_mn_addr(bce), bce->tunnel);
	bce->status = 0; 			//PBU was Accepted!

	//Add task for entry expiry.
	pmip_cache_start(bce);

	//Send a pba to ack new serving mag.
	dbg("Create PBA to new Serving MAG...\n");
	struct in6_addr_bundle addrs;
	addrs.src= &conf.our_addr;
	addrs.dst= &bce->mn_serv_mag_addr;
	mh_send_pba(&addrs, bce, &conf.PBA_LifeTime, 0);	

	return 0;
}


//===========================================================
int lma_dereg(struct pmip_entry * bce, int propagate)
//===========================================================
{
	//Delete the Task
	del_task(&bce->tqe);

	//delete old route to old tunnel.
	lma_remove_route(get_mn_addr(bce), bce->tunnel);

	//decrement users of old tunnel.
	pmip_tunnel_del(bce->tunnel);

	if (propagate)
	{
		dbg("Sends a PBU with lifetime = 0 to Old MAG (%x:%x:%x:%x:%x:%x:%x:%x)\n", NIP6ADDR(&bce->mn_serv_mag_addr));
		struct in6_addr_bundle addrs;
		struct timespec lifetime = {0,0};
		addrs.src= &conf.lma_addr;
		addrs.dst= &bce->mn_serv_mag_addr;
		mh_send_pbu(&addrs, bce,&lifetime, 0);
	}

	return 0;
}

//===========================================================
int lma_update_binding_entry(struct pmip_entry * bce, struct msg_info * info)
//===========================================================
{
	//bce->Serv_MAG_addr = info->Serv_MAG_addr;
	dbg("Store Binding Entry\n");
	bce->our_addr = conf.our_addr;
	bce->mn_iid = info->mn_iid;
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


//bce = NULL --> inter cluster different LMAs
//bce --> intra cluster, same LMA
//===========================================================
int lma_update_ro_cache(struct pmip_entry * bce, struct msg_info * info)
//===========================================================
{
  struct peer_list * pl;
  pl = pmip_ro_peer_list_get(&info->mn_addr);
  if (pl)
	{
		int i;
		for (i = pl->count-1; i>=0; i--)
		{
			struct pmip_ro_entry * de;
			de = pmip_ro_cache_get(&pl->peers[i], &info->mn_addr); 
			if (de)
			{
	  			dbg("TODO: UPDATE");
	  			//Invalidate the corresponding routing entry
          		if (conf.tunneling_enabled)
	  			{
					dbg("Delete route for ro {%x:%x:%x:%x:%x:%x:%x:%x - %x:%x:%x:%x:%x:%x:%x:%x} in table %d\n", NIP6ADDR(&de->src_addr), NIP6ADDR(&de->dst_addr), RT6_TABLE_MIP6);
					route_del(de->tunnel, RT6_TABLE_MIP6, IP6_RT_PRIO_MIP6_FWD, &in6addr_any, 0, &de->dst_addr, 128, NULL);			
					pmip_tunnel_del(de->tunnel);

					if (!bce) //Only create RO entry for inter cluster communication
					{
						//Update fields in the ro cache entry.		
						de->dst_serv_mag_addr = info->mn_serv_mag_addr;
						de->dst_serv_lma_addr = info->mn_serv_lma_addr;
	
						//create a tunnel between LMA and LMA.
						if (conf.tunneling_enabled)
						{
							de->tunnel = pmip_tunnel_add(&conf.our_addr, &de->dst_serv_lma_addr, info->iif);
							dbg("Add new route for %x:%x:%x:%x:%x:%x:%x:%x in table %d\n", NIP6ADDR(&de->dst_addr), RT6_TABLE_MIP6);
							route_add(de->tunnel, RT6_TABLE_MIP6, RTPROT_MIP,0, IP6_RT_PRIO_MIP6_FWD, &in6addr_any, 0, &de->dst_addr, 128, NULL);			
						}
					}
	  			}

				//Invalidate the RO cache entry
				pmip_ro_cache_release_entry(de);
				//if (bce)
				//{
				//	pmip_ro_delete(de);
		        //}
			} //if de
		} // for
		pmip_ro_peer_list_release(pl);
    }
}
// //===========================================================
// int lma_update_ro_entry(struct pmip_ro_entry * roe, struct msg_info * info)
// //===========================================================
// {
// 	//Update other field in the destination cache entry.
// 	dbg("Store RO Entry\n");
// 	roe->sender_addr = info->src;
// 	roe->src_prefix = get_node_prefix(&info->src) ;	/* Network Address Prefix for Src MN */
// 	roe->src_addr = info->src;	/* Src MN addr */
// 
// 	roe->dst_prefix = get_node_prefix(&info->dst) ;	/* Network Address Prefix for Src MN */
// 	roe->dst_addr = info->;	/* Src MN addr */
// 	roe->dst_serv_lma_addr = info->mn_serv_lma_addr;
// 	roe->dst_serv_mag_addr = info->mn_serv_mag_addr;
// 	//roe->add_time;       /* When was the entry added or modified */
// 	//roe->lifetime;      	/* lifetime for this route optimization entry, in seconds */
// 	//roe->seqno;			/* outstanding sequence number for the last PBREQ message */
// 	return 0;
// }
