/*****************************************************************
 * C Implementation: pmip_fsm
 * Description: 
 * Author: 
 *   Christian Bonnet
 *   Huu-Nghia Nguyen
 *   Hussain & Daniel
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

#ifdef FSM_DEBUG
	#define dbg(...) dbgprint(__FUNCTION__, __VA_ARGS__)
#else
	#define dbg(...)
#endif

extern pthread_rwlock_t pmip_lock; /* Protects proxy binding cache */
extern uint16_t seqno_pbreq;
void pmip_timer_retrans_pbu_handler(struct tq_elem *tqe);

//We apply some trick here to advoid create/delete of tunnel too frepquently.
//===========================================================
int pmip_tunnel_add(struct in6_addr *local, struct in6_addr *remote, int link)
//===========================================================
{
	if (conf.tunneling_enabled)
	{
		dbg("Creating IP-in-IP tunnel...\n");
		int tunnel = tunnel_add(local, remote, link, 0, 0);
		return tunnel;
	}
	else 
	{
		dbg("IP-in-IP tunneling is disabled, no tunnel is created\n");
		return 0;
	}
}

//===========================================================
int pmip_tunnel_del(int ifindex)
//===========================================================
{
	int res = 0;
	if (conf.tunneling_enabled)
	{
		dbg("Decrease reference number of tunnel %d\n", ifindex);
		if (ifindex > 0)
		{
			int usercount = tunnel_getusers(ifindex);
			if (usercount > 1 || conf.dtun_enabled) res = tunnel_del(ifindex, 0, 0);
			else if (usercount == 1) 
			{ 
				//TODO: Put the tunnel  in to the pool
				//TODO: Set timer to delete the tunnel after a long stalled period
			}
		}
		else res = -1;
	}
	else dbg("IP-in-IP tunneling is disabled, no tunnel is deleted\n");
	return res;
}


/** Finite State Machine; return 0 for success and -1 if error */

//===========================================================
int mag_fsm(struct msg_info* info)
//===========================================================
{
	int result = 0;
	struct pmip_entry *bce;	
	int type = pmip_cache_exists(&conf.our_addr,&info->mn_iid);

	switch (type)
	{
		//--------------------------------------
		case BCE_NO_ENTRY:
			if (info->msg_event == hasNS)
			{
				if (info->is_dad)
				{
					dbg("New MN is found, start Movement Detection ...\n");
					bce = pmip_cache_alloc(BCE_HINT);					
					result = mag_pmip_md(info, bce);
					pmip_cache_add(bce);
				}
				//Either ARP or Unreachability detection!!					
				else if (!ipv6_pfx_cmp(&info->ns_target, &conf.Home_Network_Prefix, PLEN))
				{
					dbg("Possible Trigger for RO  %x:%x:%x:%x:%x:%x:%x:%x- %x:%x:%x:%x:%x:%x:%x:%x ...\n", NIP6ADDR(&info->src), NIP6ADDR(&info->ns_target));			
					result = mag_pmip_detect_ro(info);
				}					
			}

			else if (info->msg_event == hasPBRES)
			{
				dbg("On-going Route Optimization\n");
				mag_ro_fsm(type, info);
			}

			break;

		//--------------------------------------
		case BCE_HINT:
			//If at Hint, we receive some NS for DAD --> override the last entry
			if (info->msg_event == hasNS && info->is_dad)
			{
				dbg("Existing MN is found at BCE_HINT state, continue Movement Detection\n");
				bce = pmip_cache_get(&conf.our_addr,&info->mn_iid);
				result = mag_pmip_md(info, bce);
				pmipcache_release_entry(bce);
			}					

			//If this is an NA & addressing to MR & the entry is temporary created, this
			//is the answer for the hint of network_based movment detection	
 			else if (info->msg_event == hasNA && IN6_ARE_ADDR_EQUAL(&conf.mag_addr_ingress, &info->dst))
			{
				dbg("Network-based Movement Detection - New attachment!\n");
				bce = pmip_cache_get(&conf.our_addr,&info->mn_iid);
				del_task(&bce->tqe); //Stop Network-based movement detection
				mag_start_registration(bce);
				bce->type = BCE_TEMP;
				pmipcache_release_entry(bce);
			}
			break;

		//--------------------------------------
		case BCE_TEMP:
			if (info->msg_event == hasPBA)
			{
				bce = pmip_cache_get(&conf.our_addr,&info->mn_iid);
				if (info->seqno == bce->seqno_out)
				{
					dbg("Finish Location Registration\n");
					//Modify the entry with additional info.
					del_task(&bce->tqe); //Delete timer retransmission PBU (if any)
					bce->PBA_flags = info->PBA_flags;
					bce->lifetime = info->lifetime;
					mag_end_registration(bce, info->iif);			 					
				}
				else dbg("Seq# of PBA is Not equal to Seq# of sent PBU!\n");
				pmipcache_release_entry(bce);
			}
			else if (info->msg_event == hasPBU && info->lifetime.tv_sec ==0 && info->lifetime.tv_nsec==0) //Location Deregistration 
			{
				dbg("Start Location De-registration on Demand\n");
				bce = pmip_cache_get(&conf.our_addr,&info->mn_iid); 
				mag_dereg(bce, 0);		
			}

			break;

		//--------------------------------------
		case BCE_PMIP:
			if (info->msg_event == hasNA) 
			{
				//Reset couter, Delete task for entry deletion  & Add a new task for NS expiry.
				bce = pmip_cache_get(&conf.our_addr,&info->mn_iid);				
				bce->n_rets_counter = conf.Max_Rets; //Reset the Retransmissions Counter.
				dbg("Reset the Reachability Counter = %d for %x:%x:%x:%x:%x:%x:%x:%x\n", bce->n_rets_counter, NIP6ADDR(&info->mn_iid));				
				del_task(&bce->tqe);
				pmip_cache_start(bce);				
				pmipcache_release_entry(bce);
			}
			else if (info->msg_event == hasPBU && info->lifetime.tv_sec ==0 && info->lifetime.tv_nsec==0) //Location Deregistration
			{
				dbg("Start Location De-registration on Demand\n");
				bce = pmip_cache_get(&conf.our_addr,&info->mn_iid); 
				mag_dereg(bce, 0);		
			}
			else if (info->msg_event == hasPBREQ && info->PBREQ_action == PBREQ_LOCATE) //Heart Beat Messages only available for registered MN 
			{
				dbg("Hearbeat on Demand - I'm ALIVE\n");
				struct in6_addr_bundle addrs;
				addrs.src = &conf.our_addr;
				addrs.dst = &info->src;				
				mh_send_pbres(&addrs, NULL, &info->mn_addr, &info->mn_iid, NULL, NULL, info->seqno, PBRES_OK, info->iif); 
			}
			else if (info->msg_event == hasPBREQ && info->PBREQ_action == PBREQ_RO_INIT) 
				mag_ro_fsm(type, info);
			else if (info->msg_event == hasPBRES)
			  mag_ro_fsm(type, info);
			  
			break;
		default:
			dbg("No action for this event (%d) at current state (%d) !\n", info->msg_event, type);
	}
	return result;
	
}


//===========================================================
int lma_fsm(struct msg_info* info)
//===========================================================
{
	int result = 0;
	struct pmip_entry *bce;	
	int type = pmip_cache_exists(&conf.our_addr,&info->mn_iid);

	switch (type)
	{
		//--------------------------------------
		case BCE_NO_ENTRY:
			dbg("No PMIP entry found for %x:%x:%x:%x:%x:%x:%x:%x ... \n", NIP6ADDR(&info->mn_iid));
			if(info->msg_event == hasPBU && (info->lifetime.tv_sec > 0 || info->lifetime.tv_nsec >0))
			{
				//Create New Proxy Binding Entry storing information
				dbg("PBU for a new MN ... Location Registration\n");
				bce = pmip_cache_alloc(BCE_PMIP);
				if (bce != NULL)	
				{								
					lma_update_binding_entry(bce, info); //Save information into bce
					lma_reg(bce);
					get_mn_addr(bce);
					info->mn_addr = bce->mn_addr;
					lma_update_ro_cache(bce, info);
					pmip_cache_add(bce);
				}

				//Advertise PBRes to All-LMA@ to start Location Deregistration in old LMA or maintain inter-cluster communication				
				if (!IN6_ARE_ADDR_EQUAL(&conf.all_lma_addr, &in6addr_loopback))
				{
					dbg("Advertise PBRes to ALL-LMA@ ...\n");						
					struct in6_addr_bundle addrs;			
					addrs.src = &conf.our_addr;
					addrs.dst = &conf.all_lma_addr; //TODO All_LMA@ need to be defined.
					mh_send_pbres(&addrs, NULL, get_mn_addr(bce), &bce->mn_iid, &bce->mn_serv_mag_addr, &conf.our_addr, 0, PBRES_INTER_CLUSTER_MOBILITY, 0);
				}
			}

			else if (info->msg_event == hasPBREQ)
			{				
				//Save the communication session in Destination Cache if neccessary
				//CONVERT_ID2ADDR(&info->mn_addr, &info->mn_prefix, &info->mn_iid);		
				struct pmip_ro_entry* de = pmip_ro_cache_get(&info->src_mn_addr, &info->mn_addr); //TODO our_addr must be replaced by addresses->src
				if (de == NULL)
				{ 					
					de = pmip_ro_cache_alloc();
					de->src_addr = info->src_mn_addr;
					de->dst_addr = info->mn_addr;
					if (pmip_ro_cache_add(de) != NULL)
					{						
						pthread_rwlock_rdlock(&pmip_ro_cache_lock);
						pthread_rwlock_wrlock(&de->lock);
						//TODO Start dcache timer					
					}
				}
			
				struct peer_list * pl;
				pl = pmip_ro_peer_list_get(&info->mn_addr);
				assert(pl);
				int i;
				for (i = 0; i<pl->count; i++)
				{
					dbg("%x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&pl->peers[i]));
				}
				pmip_ro_peer_list_release(pl);


				dbg("Update RO cache entry\n");
				de->sender_addr = info->src;
				de->src_serv_mag_addr = info->src_mag_addr;				
				de->dst_iid = info->mn_iid;				
				de->iif = info->iif;
				de->action = PBREQ_LOCATE; //To be explicit.													
				
				//TODO Check lifetime of RO cache entry. If it is invalide then  Send PBREQ --> All-LMA 
				dbg("Destination RO entry invalid for %x:%x:%x:%x:%x:%x:%x:%x ... Send PBREQ to All-LMA@\n", NIP6ADDR(&de->dst_addr));
				if (IN6_ARE_ADDR_EQUAL(&conf.all_lma_addr, &info->src) || IN6_ARE_ADDR_EQUAL(&conf.all_lma_addr, &info->dst) || IN6_ARE_ADDR_EQUAL(&conf.all_lma_addr, &in6addr_loopback))
					dbg("All-LMA@ cause a loop ... Stop PBREQ chain!\n"); //Never do a loop!
				else {						
					struct in6_addr_bundle addrs;			
					addrs.src = &conf.our_addr;
					addrs.dst = &conf.all_lma_addr; //TODO All_LMA@ need to be defined.
					struct in6_addr * ptr_src_mn_addr = NULL;
					struct in6_addr * ptr_src_mag_addr = NULL;
					if (!IN6_ARE_ADDR_EQUAL(&de->src_serv_mag_addr, &in6addr_any)) ptr_src_mag_addr = &de->src_serv_mag_addr;
					if (!IN6_ARE_ADDR_EQUAL(&de->src_addr, &in6addr_any)) ptr_src_mn_addr = &de->src_addr;							
					mh_send_pbreq(&addrs, &de->dst_iid, &de->dst_addr, ptr_src_mag_addr, ptr_src_mn_addr, seqno_pbreq, PBREQ_LOCATE, 0, NULL); //Let the system find the outgoing interface
					seqno_pbreq++;
				}
				if (de != NULL) pmip_ro_cache_release_entry(de);                 
			}

			else if (info->msg_event == hasPBRES)
			{
				dbg("No PMIP entry found for %x:%x:%x:%x:%x:%x:%x:%x... \n",  NIP6ADDR(&info->mn_addr));

				if (info->PBRES_status == PBRES_INTER_CLUSTER_MOBILITY)
				{
					lma_update_ro_cache(NULL, info);
					break;
				}
				struct pmip_ro_entry *de = pmip_ro_cache_get(&info->src_mn_addr, &info->mn_addr);		
				if (de && info->PBRES_status == PBRES_OK) //TODO Check seqno & status
				{
					//Update other fields in the destination cache entry.		
					de->dst_serv_mag_addr = info->mn_serv_mag_addr;
					de->dst_serv_lma_addr = info->mn_serv_lma_addr;

					//create a tunnel between LMA and LMA.
					if (de->tunnel <= 0 && conf.tunneling_enabled)
					{
						de->tunnel = pmip_tunnel_add(&conf.our_addr, &de->dst_serv_lma_addr, info->iif);
						dbg("Add new route for %x:%x:%x:%x:%x:%x:%x:%x in table %d\n", NIP6ADDR(&de->dst_addr), RT6_TABLE_MIP6);
						route_add(de->tunnel, RT6_TABLE_MIP6, RTPROT_MIP,0, IP6_RT_PRIO_MIP6_FWD, &in6addr_any, 0, &de->dst_addr, 128, NULL);			
					}

					dbg("Create PBRES message to answer MAG's PBREQ...\n");
					struct in6_addr_bundle addrs;
					addrs.src = &conf.our_addr;
					addrs.dst = &de->sender_addr;
					mh_send_pbres(&addrs, &de->src_addr, &de->dst_addr, &de->dst_iid, &de->dst_serv_mag_addr, &de->dst_serv_lma_addr, de->seqno, PBRES_OK, de->iif);	
					pmip_ro_cache_release_entry(de);
				}
				
			}

			break;

		//--------------------------------------
		case BCE_PMIP:
			if(info->msg_event == hasPBU && (info->lifetime.tv_sec > 0 || info->lifetime.tv_nsec >0))
			{
				// life time is not zero and there is an existing entry ==> check Serv_MAG, if not same ==> modify it.
				dbg("PBU for an existing MN ... possible Location Registration\n");
				bce = pmip_cache_get(&conf.our_addr,&info->mn_iid);
				if( !IN6_ARE_ADDR_EQUAL(&info->src, &bce->mn_serv_mag_addr) ) 
				{
 					lma_dereg(bce, 1); //Inform the old MAG to deregister MN 									
					lma_update_binding_entry(bce, info);
					lma_reg(bce);
				}	
				pmipcache_release_entry(bce);
			}

			else if (info->msg_event == hasPBU && info->lifetime.tv_sec == 0 && info->lifetime.tv_nsec==0) 
			{
				dbg("PBU with Lifetime = 0... start Location De-registration\n");
				bce = pmip_cache_get(&conf.our_addr,&info->mn_iid);					
				if(IN6_ARE_ADDR_EQUAL(&info->src, &bce->mn_serv_mag_addr)) //Received PBU from old MAG!!!
				{
					lma_dereg(bce, 0);
					pmipcache_release_entry(bce);
					pmip_bce_delete(bce);
				} else pmipcache_release_entry(bce);
			}

			else if (info->msg_event == hasPBREQ)
			{ 
				dbg("PMIP Cache hit, create PBRES\n");
				struct in6_addr_bundle addrs;
				addrs.dst = &info->src;
				addrs.src = &conf.our_addr;
			
				//create a PB response.
				bce = pmip_cache_get(&conf.our_addr,&info->mn_iid);
				mh_send_pbres(&addrs, &info->src_mn_addr, get_mn_addr(bce), &bce->mn_iid, &bce->mn_serv_mag_addr, &conf.our_addr, info->seqno, PBRES_OK, info->iif);

				//Is Inter-cluster? Check If we have created a tunnel before? If not --> create inter-cluster bidirectional tunnel
				if (conf.tunneling_enabled)
				{
					pmip_tunnel_add(&conf.our_addr, &info->src, info->iif);
 					//if (!IN6_ARE_ADDR_EQUAL(&info->src_mag_addr, &in6addr_any)) pmip_tunnel_add(&conf.our_addr, &info->src_mag_addr, info->iif);
				}

				pmipcache_release_entry(bce);
			}

			else if (info->msg_event == hasPBRES) 
			{
				bce = pmip_cache_get(&conf.our_addr,&info->mn_iid);	
 				if (info->PBRES_status == PBRES_INTER_CLUSTER_MOBILITY) 
 				{
 					dbg("PBRes with Inter Cluster flag ... start Location De-registration\n");
					lma_update_ro_cache(NULL, info);
 					lma_dereg(bce, 1);
 					pmipcache_release_entry(bce);
 					pmip_bce_delete(bce);
 					break;
 				} 
 				else 
				if(info->seqno == bce->seqno_out) //Heart Beat Messages only available for registered MN
				{					
					dbg("Timer for Expiry is intialized!\n");
					del_task(&bce->tqe); //Delete the Task (if ANY)
					bce->n_rets_counter = conf.Max_Rets; //Reset the Retransmissions counter.					
					pmip_cache_start(bce); //Add task for entry expiry.					
				}
				else dbg("Seq# of PBRES is Not equal to Seq# of PBREQ ... Ignore!\n");
				pmipcache_release_entry(bce);
			}
			break;

		default:
			dbg("No action for this event (%d) at current state (%d) !\n", info->msg_event, type);

	}
	return result;	
}
	



//===========================================================
int mag_ro_fsm(int type, struct msg_info* info)
//===========================================================
{
	int result = 0;
	struct pmip_entry *bce;	
	switch (type)
	{
		//--------------------------------------
		case BCE_NO_ENTRY:
			if (info->msg_event == hasPBRES)
			{		
				struct pmip_ro_entry *de = pmip_ro_cache_get(&info->src_mn_addr, &info->mn_addr);
				if (de)
				{
					if (de->action == PBREQ_LOCATE)
					{							
					        dbg("Create NA as proxy arp for %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&de->dst_addr));
					        uint32_t na_flags = NDP_NA_SOLICITED | NDP_NA_OVERRIDE;
					        ndisc_send_na(de->iif, &conf.mag_addr_ingress, &de->src_addr, &de->dst_addr,na_flags);	

						dbg("confirm PBREQ_LOCATE for an existing RO entry, update & start Route Optimization\n");	  
						de->dst_serv_lma_addr = info->mn_serv_lma_addr;
						de->dst_serv_mag_addr = info->mn_serv_mag_addr;

						if (conf.ro_enabled)
						{
						  struct in6_addr_bundle address;
						  address.src = &conf.our_addr;
						  address.dst = &de->dst_serv_mag_addr;
						  mh_send_pbreq(&address, &de->dst_addr, &de->dst_addr, &conf.our_addr, &de->src_addr, seqno_pbreq, PBREQ_RO_INIT, 0, NULL); //send PBREQ to peer MAG to setup tunnel.						 
						}
					}
					pmip_ro_cache_release_entry(de);
				} //if de
			} //if hasPBRES
			break;

		//--------------------------------------
		case BCE_PMIP:
			if (info->msg_event == hasPBREQ) 
			{
				dbg("Action %d == %d?\n", info->PBREQ_action, PBREQ_LOCATE); 
				if (info->PBREQ_action == PBREQ_LOCATE)
			    {
					dbg("PBREQ_LOCATE - Hearbeat on Demand - Return: I'm ALIVE\n");
					struct in6_addr_bundle addrs;
					addrs.src = &conf.our_addr;
					addrs.dst = &info->src;				
					mh_send_pbres(&addrs, NULL, &info->mn_addr, &info->mn_iid, NULL, NULL, info->seqno, PBRES_OK, info->iif); 
			    }

				else if (info->PBREQ_action == PBREQ_RO_INIT)
				{
						//Create/Update RO entry 
						//Note that the connection is seen in a reverse way at this MAG
						struct pmip_ro_entry *de = pmip_ro_cache_get(&info->mn_addr, &info->src_mn_addr);
	
						if (de == NULL)
						{ 	
							dbg("PBREQ_RO_INIT for new RO entry, create RO entry & do Route Optimization\n");				
							de = pmip_ro_cache_alloc();
							de->src_addr = info->mn_addr;
							de->dst_addr = info->src_mn_addr;
							de->dst_iid = de->dst_addr; //TODO
							de->sender_addr = info->src;
							if (pmip_ro_cache_add(de) != NULL)
							{
								pthread_rwlock_rdlock(&pmip_ro_cache_lock);
								pthread_rwlock_wrlock(&de->lock);
								//TODO Start dcache timer
							}			
						}
						else dbg("PBREQ_RO_INIT for existing RO entry, update & do Route Optimization\n");
	
						if (de)
						{	
							de->src_serv_mag_addr = conf.our_addr;			
	// 						de->iif = info->iif;	  
	// 						de->dst_serv_lma_addr = info->mn_serv_lma_addr;
							de->dst_serv_mag_addr = info->src_mag_addr;
							de->action = info->PBREQ_action;
									
							if (conf.ro_enabled)
							{							
								//create a PBRES
								struct in6_addr_bundle addrs;
								addrs.src = &conf.our_addr;
								addrs.dst = &de->dst_serv_mag_addr;
								mh_send_pbres(&addrs, &de->src_addr, &de->dst_addr, &de->dst_iid, NULL, NULL, info->seqno, PBRES_OK, info->iif);					 
	
								//create a tunnel between MAG and serving MAG
								if (conf.tunneling_enabled)
								{
									if (de->tunnel <= 0) de->tunnel = pmip_tunnel_add(&conf.our_addr,&de->dst_serv_mag_addr, info->iif);
									dbg("Add new route for %x:%x:%x:%x:%x:%x:%x:%x in table %d\n", NIP6ADDR(&de->dst_addr), RT6_TABLE_PMIP);
									route_add(de->tunnel, RT6_TABLE_PMIP, RTPROT_MIP,0, IP6_RT_PRIO_MIP6_FWD,&in6addr_any, 0, &de->dst_addr, 128, NULL);
								} //if tunneling_enabled
							} //if ro_enabled
							pmip_ro_cache_release_entry(de);
						} //if de
			    } //if PBREQ_RO_INIT
			}


			if (info->msg_event == hasPBRES)
			{
				//Note that the connection is seen from in the reverse way of the peer			
				struct pmip_ro_entry *de = pmip_ro_cache_get(&info->mn_addr, &info->src_mn_addr);
				//if (de && de->seqno != info->seqno)
				// dbg("Seq# of PBRES is Not equal to Seq# of PBREQ ... Ignore!\n");
				if (de) // && de->action == PBREQ_RO_INIT && info->PBRES_status == PBRES_OK) 
				{
					//This is the confirmation for RO
					//create a tunnel between MAG and serving MAG
					if (conf.ro_enabled && conf.tunneling_enabled)
					{
						if (de->tunnel <= 0) de->tunnel = pmip_tunnel_add(&conf.our_addr,&de->dst_serv_mag_addr, info->iif);			  
						dbg("Add new route for %x:%x:%x:%x:%x:%x:%x:%x in table %d\n", NIP6ADDR(&de->dst_addr), RT6_TABLE_PMIP);
						route_add(de->tunnel, RT6_TABLE_PMIP, RTPROT_MIP,0, IP6_RT_PRIO_MIP6_FWD,&in6addr_any, 0, &de->dst_addr, 128, NULL);						   	
					}
				}					
				pmip_ro_cache_release_entry(de);
			}
			break;
		default:
			dbg("No action for this event (%d) at current state (%d) !\n", info->msg_event, type);
	}
	return result;
	
}


//===========================================================
int lma_ro_fsm(struct msg_info* info)
//===========================================================
{
}


