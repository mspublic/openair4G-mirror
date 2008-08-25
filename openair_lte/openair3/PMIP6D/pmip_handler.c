/*****************************************************************
 * C Implementation: pmip_handler.c
 * Description: 
 * Author: 
 *   Christian Bonnet
 *   Huu-Nghia Nguyen
 *   Hussain & Daniel
 * Copyright: Eurecom Institute,(C) 2008
 ******************************************************************/
#include "pmip_consts.h"
#include "pmip_types.h"
#include "icmp6.h"
#include "mh.h"
#include <netinet/in.h>
#include <netinet/ip6mh.h>
#include "debug.h"
#include "bcache.h"
#include "pmip_extern.h"
#include "conf.h"
#include "util.h"
#include "pmip_cache.h"
#include "rtnl.h"
#include "tunnelctl.h"
#include "prefix.h"


uint16_t seqno_pbreq = 0;

//compare two addresses and returns 0 if the same, else -1.
int COMPARE(struct in6_addr *addr1,struct in6_addr *addr2)
{
	dbg("Compare: %x:%x:%x:%x:%x:%x:%x:%x  to: %x:%x:%x:%x:%x:%x:%x:%x \n", NIP6ADDR(addr1), NIP6ADDR(addr2));
	if(addr1->in6_u.u6_addr32[0] == addr2->in6_u.u6_addr32[0] && addr1->in6_u.u6_addr32[1] == addr2->in6_u.u6_addr32[1] && addr1->in6_u.u6_addr32[2] == addr2->in6_u.u6_addr32[2] && addr1->in6_u.u6_addr32[3] == addr2->in6_u.u6_addr32[3])
	return 0;
	else
	return -1;
}


//ID2ADDR converts an ID & a prefix into an Address.
struct in6_addr *ID2ADDR(struct in6_addr *prefix,struct in6_addr *id)
{
	static struct in6_addr ADDR1;
	ADDR1=in6addr_any;
	memcpy(&ADDR1.in6_u.u6_addr32[0],&prefix->in6_u.u6_addr32[0],sizeof(__identifier));
	memcpy(&ADDR1.in6_u.u6_addr32[2],&id->in6_u.u6_addr32[2],sizeof(__identifier));
	dbg("Global_ADDR: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&ADDR1));
	return &ADDR1;
}

//NUD_ADDR converts an ID into a Multicast Address for NS Unreachability!
struct in6_addr *solicited_mcast(struct in6_addr *id)
{
	static struct in6_addr ADDR2;
	ADDR2=in6addr_any;
	ADDR2.in6_u.u6_addr32[0] = htonl(0xff020000);
	ADDR2.in6_u.u6_addr32[1] = htonl(0x00000000);
	ADDR2.in6_u.u6_addr32[2] = htonl(0x00000001);
	ADDR2.in6_u.u6_addr8[12] = 0xff;
	dbg("Solicited Multicast address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&ADDR2));
	//copy the least 24 bits from the MN_ID.
	memcpy(&ADDR2.in6_u.u6_addr8[13],&id->in6_u.u6_addr8[13],3*sizeof(ADDR2.in6_u.u6_addr8));
	dbg("Solicited Multicast address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&ADDR2));
	return &ADDR2;	
}


/**
 * Handlers triggered by add_task_abs for entry expiry and deletion.
 **/

extern pthread_rwlock_t pmip_lock; /* Protects proxy binding cache */


/**
 * Retransmit PBU
 **/

void _RET_PBU(struct tq_elem *tqe)
{
	pthread_rwlock_wrlock(&pmip_lock);
	dbg(" activated\n");
	if (!task_interrupted()) {
		struct pmip_entry *e = tq_data(tqe, struct pmip_entry, tqe);
		pthread_rwlock_wrlock(&e->lock);

	dbg("Retransmissions counter : %d\n",e->n_rets_counter);
	if(e->n_rets_counter ==0)
		{
			pthread_rwlock_unlock(&e->lock);
			free_iov_data((struct iovec *)&e->mh_vec,e->iovlen);
	
			dbg("No PBA received from CH....\n");
			dbg("Abort Trasmitting the PBU....\n");
			pmip_cache_delete(&e->our_addr,&e->peer_addr);
			return;
		}
	else
		{
			//Decrement the N trasnmissions counter.
			e->n_rets_counter--;
			struct in6_addr_bundle addrs;
			addrs.src= &conf.our_addr;
 			addrs.dst= &conf.lma_addr;
					
			//sends a PBU
			dbg("send PBU again....\n");
			pmip_mh_send(&addrs, e->mh_vec, e->iovlen, e->link);

			
			//add a new task for PBU retransmission.
			struct timespec expires;
			clock_gettime(CLOCK_REALTIME, &e->add_time);
			tsadd(e->add_time, conf.N_RetsTime, expires);
			add_task_abs(&expires, &e->tqe, _RET_PBU);
			dbg("PBU Retransmissions timer is triggered again....\n");

			pthread_rwlock_unlock(&e->lock);
			}
	}
	pthread_rwlock_unlock(&pmip_lock);
	
}
/**
 * _expired - expire PMIP binding cache entry and rets for both PBREQ on LMA and NS on MAG
 **/

void _EXPIRED(struct tq_elem *tqe)
{	
	pthread_rwlock_wrlock(&pmip_lock);
	dbg(" activated\n");
	if (!task_interrupted()) {
		struct pmip_entry *e = tq_data(tqe, struct pmip_entry, tqe);
		pthread_rwlock_wrlock(&e->lock);


		dbg("Retransmissions counter : %d\n",e->n_rets_counter);

		
		if(e->n_rets_counter ==0)
		{
			if(is_mag())
			{
				struct in6_addr_bundle addrs;
				addrs.src= &conf.our_addr;
				addrs.dst= &conf.lma_addr;
				
				struct timespec Lifetime = {0,0};
				
				dbg("Create PBU for CH to delete the PMIP entry too....\n");
				
				++e->seqno_out; 				

				mh_send_pbu(&addrs,e,&Lifetime,0);

				//Delete existing route & rule for the deleted MN
				mag_remove_route(ID2ADDR(&e->peer_prefix,&e->peer_addr), e->link);

				int usercount = tunnel_getusers(e->tunnel);
				dbg("# of binding entries %d \n", usercount);
				if (usercount == 1) {
					route_del(e->tunnel, RT6_TABLE_PMIP, IP6_RT_PRIO_MIP6_FWD,&in6addr_any, 0,&in6addr_any, 0,NULL);
				}
				//decrement users of old tunnel.
				pmip_tunnel_del(e->tunnel);
			}
			//Delete existing route for the deleted MN
			if(is_lma())
			{
				//dbg("Delete old route for: %x:%x:%x:%x:%x:%x:%x:%x\n", 
				lma_remove_route(ID2ADDR(&e->peer_prefix,&e->peer_addr), e->tunnel); 

				//decrement users of old tunnel.
				pmip_tunnel_del(e->tunnel);
			}

			//Delete entry for MN.

			pthread_rwlock_unlock(&e->lock);

			free_iov_data((struct iovec *)&e->mh_vec,e->iovlen);
			pmip_bce_delete(e);
			dbg("Number of lock ref %d \n", pmip_lock);
			pthread_rwlock_unlock(&pmip_lock);
			
			return;
		}
		

		if(is_mag())
		{
			dbg("Send NS for Neighbour Reachability for:%x:%x:%x:%x:%x:%x:%x:%x iif=%d\n", NIP6ADDR(&e->peer_addr), e->link);
			
			//Create NS for Reachability test!			
			ndisc_send_ns(e->link, &conf.mag_addr_ingress,solicited_mcast(&e->peer_addr),ID2ADDR(&e->peer_prefix,&e->peer_addr));
		}

		if(is_lma())
		{
			dbg("Send PBREQ to Serving MAG of:%x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&e->peer_addr));
			//send Proxy Binding refresh request to MAG.
			struct in6_addr_bundle addrs;
			bzero(&addrs,sizeof(struct in6_addr_bundle));
			addrs.src = &e->our_addr;
			addrs.dst = &e->Serv_MAG_addr;
			if ((e->n_rets_counter) == (conf.Max_Rets)){
			++e->seqno_out; 				
			mh_send_pbreq(&addrs,&e->peer_addr, &e->peer_prefix, e->seqno_out, e->link, e);
			}
			else{
			pmip_mh_send(&addrs, e->mh_vec, e->iovlen, e->link);
			}
		}
			struct timespec expires;
			clock_gettime(CLOCK_REALTIME, &e->add_time);
			tsadd(e->add_time, conf.N_RetsTime, expires);
			// Add a new task for deletion of entry if No Na is received.
			add_task_abs(&expires, &e->tqe, _EXPIRED);
			dbg("Start the Timer for Retransmission/Deletion ....\n");

			//Decrements the Retransmissions counter.
			e->n_rets_counter--;

			pthread_rwlock_unlock(&e->lock);
	}
	pthread_rwlock_unlock(&pmip_lock);
}


/**
 * Handlers defined for MH and ICMP messages.
 **/

//check destination address is multicast
static inline int ipv6_addr_is_solicited_mcast(const struct in6_addr *addr)
{
  return (addr->s6_addr32[0] == htonl(0xff020000) &&
	  addr->s6_addr32[1] == htonl(0x00000000) &&
	  addr->s6_addr32[2] == htonl(0x00000001) &&
	  addr->s6_addr [12] == 0xff);
}


static inline int ipv6_addr_is_multicast(const struct in6_addr *addr)
{
	return (addr->s6_addr32[0] & htonl(0xFF000000)) == htonl(0xFF000000);
}

static inline int ipv6_addr_is_linklocal(const struct in6_addr *addr)
{
	return IN6_IS_ADDR_LINKLOCAL(addr);
}


static void pmip_mag_recv_ns(const struct icmp6_hdr *ih, ssize_t len, 
	const struct in6_addr *saddr, const struct in6_addr *daddr, int iif, int hoplimit)
{

	//check that Source is not MAG address.	
	if(COMPARE(saddr,&conf.mag_addr_ingress) ==0 || COMPARE(saddr,&conf.mag_addr_egress) ==0)
	{
		dbg("Outgoing NS message ..\n");
		return;
	}
	

	// define the MN interface identifier.
	struct in6_addr id = in6addr_any;
	
	dbg("Received NS Src Addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(saddr));
	dbg("Received NS Dst addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(daddr));


	struct nd_neighbor_solicit *msg = (struct nd_neighbor_solicit *) ih;

	if(ih->icmp6_type == 135) {

		dbg("icmp_type is: %d\n",ih->icmp6_type);
	}
	
	//CHECK target is not link local address.
	if (ipv6_addr_is_linklocal(&msg->nd_ns_target)) {  
		dbg("ICMPv6 NS: Link Local target addres..\n");
		return;
	}
	
	//CHECK target is not multicast.
	if (ipv6_addr_is_multicast(&msg->nd_ns_target)) {  
		dbg("ICMPv6 NS: multicast target address..\n");
		return;
	}

	
 	if (len - sizeof(struct nd_neighbor_solicit) > 0) //Either ARP or Unreachability detection!!
	{
		dbg("NS with options received for Target: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&msg->nd_ns_target));
		dbg("Check Cache for Existing entry for Target..\n");

		memcpy(&id.in6_u.u6_addr32[2], &msg->nd_ns_target.in6_u.u6_addr32[2], sizeof(__identifier));
		
		int exist = pmip_cache_exists(&conf.our_addr,&id);

		//Source checking
		//struct in6_addr source_id = in6addr_any;
		//memcpy(&source_id.in6_u.u6_addr32[2], &daddr->in6_u.u6_addr32[2], sizeof(__identifier));	
		//int source_exist = pmip_cache_exists(&conf.our_addr,&source_id);

			if(exist == BCE_PMIP || exist == BCE_TEMP) // if returns a type equal to 5 means there is a match.
				{
					dbg("Target entry exists!\n");
					
				}
 			else //TODO Check if the target is a PMIPv6 address & the source is registered as PMIPv6 address as well.
				{		
					dbg("No cache entry exists for Target!\n");
					struct in6_addr mag_id = in6addr_any;
					memcpy(&mag_id.in6_u.u6_addr32[2], &conf.mag_addr_egress.in6_u.u6_addr32[2], sizeof(__identifier));
					//check that target is not MAG address interface.
					if(COMPARE(&msg->nd_ns_target,&conf.mag_addr_ingress) ==0 || COMPARE(&msg->nd_ns_target,&conf.mag_addr_egress) ==0 ||
					COMPARE(&mag_id,&id) ==0 )
					{
					dbg("NS Target is one of MAG's addresses!\n");
					}
					
					else
					{
					struct in6_addr_bundle address;
					address.src = &conf.our_addr;
					address.dst = &conf.lma_addr; 
					struct in6_addr prefix = in6addr_any;
					memcpy(&prefix.in6_u.u6_addr32[0], &msg->nd_ns_target.in6_u.u6_addr32[2], sizeof(__identifier));

					//send PBREQ to CH.
					mh_send_pbreq(&address,&id, &prefix,seqno_pbreq, 0, 0);
					}
					seqno_pbreq++;

					//send an NA as ARP reply.
					//dbg("Create NA as a reply for NS with option ....\n");
					//uint32_t na_flags = NDP_NA_ROUTER | NDP_NA_SOLICITED | NDP_NA_OVERRIDE;
					uint32_t na_flags = NDP_NA_OVERRIDE;
					ndisc_send_na(iif, &conf.mag_addr_ingress, saddr, &msg->nd_ns_target,na_flags);
					//dbg("Do proxy ARP for the CN (%x:%x:%x:%x:%x:%x:%x:%x)\n", NIP6ADDR(&msg->nd_ns_target));	
					//proxy_nd_start(iif, &msg->nd_ns_target, &conf.mag_addr_ingress, 0);

				}

  		return;
  	}

	dbg("NS is verified for DAD with no options....\n");
	dbg("processing NEIGHBOR SOLICIT\n");
 	dbg("DAD  received for Target: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&msg->nd_ns_target));
	memcpy(&id.in6_u.u6_addr32[2], &msg->nd_ns_target.in6_u.u6_addr32[2], sizeof(__identifier)); //find MN identifier from new address in target field.
	
	dbg("Checking the Cache for existing entry....\n");

	int exist = pmip_cache_exists(&conf.our_addr,&id);
	

	if(exist ==BCE_PMIP ) // if returns a type equal to 5 means there is a match.
	{
		dbg("MN entry exists!\n");
		dbg("Old MN!\n");
 		return;	
	}
 	else 
	{	
		struct pmip_entry *bce;
		if (exist == BCE_TEMP || exist == BCE_HINT)
		{ 
			dbg("New MN is found in Hint/Temp state, Update the temporary entry\n");
			bce = pmip_cache_get(&conf.our_addr,&id);

		}
		else 
		{
			dbg("New MN is found, Create new temporary entry\n");
			bce = pmip_cache_alloc(BCE_HINT);
			if (bce != NULL) bzero(bce, sizeof(struct pmip_entry));
		}

		//Store information in to proxy binding cache entry
		if (bce != NULL)	
		{
			bce->our_addr = conf.our_addr;
			dbg("copy our address....\n");
			memcpy(&bce->peer_addr,&id,sizeof(struct in6_addr)); 
			memcpy(&bce->peer_prefix,&conf.Home_Network_Prefix,sizeof(struct in6_addr));
			dbg("peer_prefix: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&bce->peer_prefix));
			memcpy(&bce->Serv_MAG_addr,&conf.our_addr,sizeof(struct in6_addr));
			memcpy(&bce->LMA_addr,&conf.lma_addr,sizeof(struct in6_addr));
						
			bce->seqno_out = 0;
			uint16_t flags = IP6_MH_BU_ACK | IP6_MH_PBU;
			bce->PBU_flags = flags;
			bce->link = iif;

			//TODO
			__timestamp Timestamp; 
			bzero(&Timestamp, sizeof(Timestamp));
			bce->Timestamp= Timestamp;

			struct in6_addr ID;
			memcpy(&ID,&bce->peer_addr,sizeof(struct in6_addr));
			struct in6_addr *link_local = link_local_addr((struct in6_addr *)&ID);

			//struct in6_addr address=
			memcpy(&bce->LinkLocal,link_local,sizeof(struct in6_addr));   // link local address of MN
		}

		//Added by Nghia
		//If we don't have any info & the prefix is not the same as the PMIP6 daemon prefix
		//we consider this as a hint for the network_based movment detection
		//Otherwise, we send PBU to the LMA
		#define PLEN 64
		if (exist != BCE_TEMP && exist != BCE_HINT && ipv6_pfx_cmp(&msg->nd_ns_target, &conf.Home_Network_Prefix, PLEN))
		//TODO Add some more conditions
		{
			dbg("Start Network-based Movement Detection for:%x:%x:%x:%x:%x:%x:%x:%x iif=%d\n", NIP6ADDR(&bce->peer_addr), bce->link);

			//Add a Temporary Hint Entry to the PMIP cache.
			bce->type = BCE_HINT;			
			pmip_cache_add(bce);

			//Create NS for Network-based Movment Detection!		
			struct in6_addr mn_addr = *ID2ADDR(&conf.Home_Network_Prefix, &bce->peer_addr);
			//			ndisc_send_ns(iif, &conf.mag_addr_ingress,solicited_mcast(&bce->peer_addr),ID2ADDR(&conf.Home_Network_Prefix, &bce->peer_addr));
			ndisc_send_ns(iif, &conf.mag_addr_ingress, solicited_mcast(&mn_addr), &mn_addr);
			
			//Activate Timer for NS to delete the temporary entry 
			//if there is no answer
			bce->n_rets_counter = 2;
			struct timespec expires;
			clock_gettime(CLOCK_REALTIME, &bce->add_time);
			tsadd(bce->add_time, bce->lifetime, expires);
			add_task_abs(&expires, &bce->tqe, _EXPIRED);
		}
		else if ( exist != BCE_TEMP && !ipv6_pfx_cmp(&msg->nd_ns_target, &conf.Home_Network_Prefix, PLEN))
		{			
			//NS from same pmip6 prefix, in any case (Hint/Not exist) --> upgrade to BCE_TEMP and send PBU
		        bce->type = BCE_TEMP;			
		dbg("Create PBU....\n");
		struct in6_addr_bundle addrs;
		addrs.src= &conf.our_addr;
		addrs.dst= &conf.lma_addr;
		
		dbg("addrs.src Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(addrs.src));
		dbg("addrs.dst Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(addrs.dst));


		dbg("Add a temporary entry for the New MN....\n");
		//Add a Temporary entry to the PMIP cache.
		if (exist != BCE_HINT) pmip_cache_add(bce); //Add this brand new entry to cache
		else del_task(&bce->tqe); //Stop Network based movement detection, in case
	
		//Send First PBU.
		mh_send_pbu(&addrs,bce,&conf.PBU_LifeTime,0);

		//add a new task for PBU retransmission.
		struct timespec expires;
		clock_gettime(CLOCK_REALTIME, &bce->add_time);
		tsadd(bce->add_time, conf.N_RetsTime, expires);
		add_task_abs(&expires, &bce->tqe, _RET_PBU);
		dbg("PBU Retransmissions timer is triggered....\n");



#ifdef TEST_PMIP
		extern uint8_t netbuf[3000];
		extern uint32_t netlen;

	        dbg("PBU Handler is triggered....\n");		
		pmip_lma_pbu_handler.recv((struct ip6_mh *) netbuf, netlen, &addrs, 0);
#endif	
		}
		//We need to release the entry if this entry is taken from the cache.
		if (exist == BCE_TEMP || exist == BCE_HINT) pmipcache_release_entry(bce);
	}

	//return;
}


static void pmip_mag_recv_pba(const struct ip6_mh *mh, ssize_t len, const struct in6_addr_bundle *in_addrs, int iif)
{
	dbg("Proxy Binding Acknowledgement Received\n");
	dbg("Received PBA Src Addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(in_addrs->src));
	dbg("Received PBA Dst addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(in_addrs->dst));
	//define the values for calling the parsing function
	//call the parsing function
	struct ip6_mh_binding_ack *pba = (const struct ip6_mh_binding_ack *) mh;
	
	//call the fsm function.
	pmip_fsm(in_addrs,mh_pba_parse(pba, len,in_addrs,iif), iif);
	

}

static void pmip_mag_recv_pbu(const struct ip6_mh *mh, ssize_t len, const struct in6_addr_bundle *in_addrs, int iif)
{
	dbg("Proxy Binding Update Received\n");
	dbg("Received PBU Src Addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(in_addrs->src));
	dbg("Received PBU Dst addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(in_addrs->dst));
	//define the values for the parsing function
	//call the parsing function
	struct ip6_mh_binding_update *pbu = (struct ip6_mh_binding_update *) mh;
	
	//call the fsm function.
	pmip_fsm(in_addrs,mh_pbu_parse(pbu, len,in_addrs,iif),iif);
}

static void pmip_lma_recv_pbu(const struct ip6_mh *mh, ssize_t len, const struct in6_addr_bundle *in_addrs, int iif)
{
	dbg("Proxy Binding Update Received\n");
	dbg("Received PBU Src Addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(in_addrs->src));
	dbg("Received PBU Dst addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(in_addrs->dst));
	//define the values for the parsing function
	//call the parsing function
	struct ip6_mh_binding_update *pbu = (struct ip6_mh_binding_update *) mh;
	
	//call the fsm function.
	pmip_fsm(in_addrs,mh_pbu_parse(pbu, len,in_addrs,iif),iif);
	
}



static void pmip_recv_na(const struct icmp6_hdr *ih, ssize_t len, 
	const struct in6_addr *saddr, const struct in6_addr *daddr, int iif, int hoplimit)
{

	// define the MN identifier
	struct in6_addr id = in6addr_any;
	
	dbg("Received NA Src Addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(saddr));
	dbg("Received NA Dst addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(daddr));


	struct nd_neighbor_advert *msg = (struct nd_neighbor_advert *) ih;

	if(ih->icmp6_type == 136) {

		dbg("icmp_type is: %d\n",ih->icmp6_type);
	}
	
	//TODO CHECK target is not link local address.
	if (ipv6_addr_is_linklocal(&msg->nd_na_target)) {  
		dbg("ICMPv6 NA: Link Local target addres..\n");
		return;
	}
	
	//CHECK target is not multicast.
	if (ipv6_addr_is_multicast(&msg->nd_na_target)) {  
		dbg("ICMPv6 NA: multicast target address..\n");
		return;
	}
 	if (len - sizeof(struct nd_neighbor_advert) > 0)
	{
		dbg("Reply received for Solicited node: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&msg->nd_na_target));
		dbg("Check Cache for Existing entry for Target..\n");

		memcpy(&id.in6_u.u6_addr32[2], &msg->nd_na_target.in6_u.u6_addr32[2], sizeof(__identifier));

		int exist = pmip_cache_exists(&conf.our_addr,&id);

		//Added by Nghia
		//If this is an NA & addressing to MR & the entry is temporary created, this
		//is the answer for the hint of network_based movment detection	
		if (exist == BCE_HINT && !COMPARE(&conf.mag_addr_ingress,daddr))
		{
			dbg("Network-based movement detection - New attachment!\n");
			struct pmip_entry *bce;
			bce = pmip_cache_get(&conf.our_addr,&id);

			dbg("Create PBU....\n");
			struct in6_addr_bundle addrs;
			addrs.src= &conf.mag_addr_egress;
			addrs.dst= &conf.lma_addr;
			
			dbg("addrs.src Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(addrs.src));
			dbg("addrs.dst Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(addrs.dst));

			//DELETE task for entry deletion & stop Network-based movement detection
			del_task(&bce->tqe);
			
			//Send First PBU.
			mh_send_pbu(&addrs,bce,&conf.PBU_LifeTime,0);
	
			//add a new task for PBU retransmission.
			struct timespec expires;
			clock_gettime(CLOCK_REALTIME, &bce->add_time);
			tsadd(bce->add_time, conf.N_RetsTime, expires);
			add_task_abs(&expires, &bce->tqe, _RET_PBU);
			dbg("PBU Retransmissions timer is triggered....\n");	

        		pmipcache_release_entry(bce);
		}
		else if (exist == BCE_PMIP) // if returns a type equal to 5 means there is a match.
			//if(exist == BCE_TEMP || exist == BCE_PMIP) // if returns a type equal to 5 means there is a match.
				{
					dbg("Target entry exists!\n");
					struct pmip_entry *bce;
					bce = pmip_cache_get(&conf.our_addr,&id);

					//Reset the Retransmissions Counter.
					bce->n_rets_counter = conf.Max_Rets;
					dbg("Reset the Retransmissions counter: %d\n",bce->n_rets_counter);
					//DELETE task for entry deletion.
					del_task(&bce->tqe);
					//add a new task for NS expiry.
					struct timespec expires;
					clock_gettime(CLOCK_REALTIME, &bce->add_time);
					tsadd(bce->add_time, bce->lifetime, expires);
					add_task_abs(&expires, &bce->tqe, _EXPIRED);
        				pmipcache_release_entry(bce);
					
				}
 			else 
				{		
					dbg("No cache entry exists for Target!\n");
					return;
				}
  	}
	return;
}

static void pmip_recv_pbreq(const struct ip6_mh *mh, ssize_t len, const struct in6_addr_bundle *in_addrs, int iif)
{
	
	dbg("Proxy Binding Refresh Request Received\n");
	dbg("Received PBREQ Src Addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(in_addrs->src));
	dbg("Received PBREQ Dst addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(in_addrs->dst));
	//define the values for the parsing function
	//call the parsing function
	struct ip6_mh_proxy_binding_request *pbr = (struct ip6_mh_proxy_binding_request *) mh;
	
	//call the fsm function.
	pmip_fsm(in_addrs,mh_pbreq_parse(pbr, len,in_addrs,iif),iif);
	
}


static void pmip_recv_pbres(const struct ip6_mh *mh, ssize_t len, const struct in6_addr_bundle *in_addrs, int iif)
{
	dbg("Proxy Binding Response Received\n");
	dbg("Received PBRES Src Addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(in_addrs->src));
	dbg("Received PBRES Dst addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(in_addrs->dst));
	//define the values for the parsing function
	//call the parsing function
	struct ip6_mh_proxy_binding_response *pbre = (struct ip6_mh_proxy_binding_response *) mh;
	
	//call the fsm function.
	pmip_fsm(in_addrs,mh_pbres_parse(pbre, len,in_addrs,iif),iif);

}

struct icmp6_handler pmip_mag_ns_handler = {
        .recv = pmip_mag_recv_ns
};

struct mh_handler pmip_mag_pbu_handler = {
	.recv = pmip_mag_recv_pbu
};

struct mh_handler pmip_mag_pba_handler = {
	.recv = pmip_mag_recv_pba
};

struct mh_handler pmip_lma_pbu_handler = {
	.recv = pmip_lma_recv_pbu
};

struct icmp6_handler pmip_recv_na_handler = {
        .recv = pmip_recv_na
};

struct mh_handler pmip_pbreq_handler = {
	.recv = pmip_recv_pbreq
};

struct mh_handler pmip_pbres_handler = {
	.recv = pmip_recv_pbres
};
