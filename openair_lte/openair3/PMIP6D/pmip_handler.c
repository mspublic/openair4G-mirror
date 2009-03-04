/*****************************************************************
 * C Implementation: pmip_handler.c
 * Description: 
 * Author: 
 *   Christian Bonnet
 *   Huu-Nghia Nguyen
 *   Hussain & Daniel
 * Copyright: Eurecom Institute,(C) 2008
 ******************************************************************/
#include "icmp6.h"
#include "mh.h"
#include <netinet/in.h>
#include <netinet/ip6mh.h>
#include "debug.h"
#include "conf.h"
#include "rtnl.h"
#include "prefix.h"

#include "pmip_consts.h"
#include "pmip_types.h"
#include "pmip_extern.h"
#include "pmip_cache.h"
#include "pmip_ro_cache.h"


uint16_t seqno_pbreq = 0;

//NUD_ADDR converts an ID into a Link Local Address!
struct in6_addr *link_local_addr(struct in6_addr *id)
{
	static struct in6_addr ADDR;
	ADDR =in6addr_any;

	ADDR.s6_addr32[0] = htonl(0xfe800000);

	//copy the MN_ID.
	memcpy(&ADDR.in6_u.u6_addr32[2],&id->in6_u.u6_addr32[2],sizeof(ip6mnid));

	return &ADDR;
}


//CONVERT_ID2ADDR converts an ID & a prefix into an Address.
struct in6_addr * CONVERT_ID2ADDR(struct in6_addr *result, struct in6_addr *prefix,struct in6_addr *id)
{
	*result = in6addr_any;
	memcpy(&result->in6_u.u6_addr32[0],&prefix->in6_u.u6_addr32[0],sizeof(ip6mnid));
	memcpy(&result->in6_u.u6_addr32[2],&id->in6_u.u6_addr32[2],sizeof(ip6mnid));
	//dbg("Global Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(result));
	return result;
}

struct in6_addr *get_mn_addr (struct pmip_entry * bce)
{
	CONVERT_ID2ADDR(&bce->mn_addr, &bce->mn_prefix, &bce->mn_iid);
	return &bce->mn_addr;
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
	
	//copy the least 24 bits from the MN_ID.
	memcpy(&ADDR2.in6_u.u6_addr8[13],&id->in6_u.u6_addr8[13],3*sizeof(ADDR2.in6_u.u6_addr8));
	//dbg("Solicited Multicast address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&ADDR2));
	return &ADDR2;	
}


/**
 * Handlers triggered by add_task_abs for entry expiry and deletion.
 **/

extern pthread_rwlock_t pmip_lock; /* Protects proxy binding cache */


/**
 * Retransmit PBU
 **/

void pmip_timer_retrans_pbu_handler(struct tq_elem *tqe)
{
	pthread_rwlock_wrlock(&pmip_lock);
	printf("-------------------------------------\n");
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
			pmip_cache_delete(&e->our_addr,&e->mn_iid);
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
			add_task_abs(&expires, &e->tqe, pmip_timer_retrans_pbu_handler);
			dbg("PBU Retransmissions timer is triggered again....\n");

			pthread_rwlock_unlock(&e->lock);
			}
	}
	pthread_rwlock_unlock(&pmip_lock);
	
}
/**
 * _expired - expire PMIP binding cache entry and rets for both PBREQ on LMA and NS on MAG
 **/

void pmip_timer_bce_expired_handler(struct tq_elem *tqe)
{	
	pthread_rwlock_wrlock(&pmip_lock);
	printf("-------------------------------------\n");
	if (!task_interrupted()) {
		struct pmip_entry *e = tq_data(tqe, struct pmip_entry, tqe);
		pthread_rwlock_wrlock(&e->lock);


		dbg("Retransmissions counter : %d\n",e->n_rets_counter);	
		if(e->n_rets_counter ==0)
		{
 			free_iov_data((struct iovec *)&e->mh_vec,e->iovlen);
			if(is_mag())
			{
				++e->seqno_out;
				mag_dereg(e, 1);				 				
			}
			//Delete existing route for the deleted MN
			if(is_lma())
			{
				lma_dereg(e, 1);
				pmipcache_release_entry(e);
				pmip_bce_delete(e);
			}
			return;
		}
		

		if(is_mag())
		{
			dbg("Send NS for Neighbour Reachability for:%x:%x:%x:%x:%x:%x:%x:%x iif=%d\n", NIP6ADDR(&e->mn_iid), e->link);
			
			//Create NS for Reachability test!			
			ndisc_send_ns(e->link, &conf.mag_addr_ingress,solicited_mcast(&e->mn_iid),get_mn_addr(e));
		}

		if(is_lma())
		{
			dbg("Send PBREQ to Serving MAG of:%x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&e->mn_iid));
			//send Proxy Binding refresh request to MAG.
			struct in6_addr_bundle addrs;
			bzero(&addrs,sizeof(struct in6_addr_bundle));
			addrs.src = &e->our_addr;
			addrs.dst = &e->mn_serv_mag_addr;
			if ((e->n_rets_counter) == (conf.Max_Rets)){
				++e->seqno_out; 				
				mh_send_pbreq(&addrs,&e->mn_iid, NULL, NULL, NULL, e->seqno_out, PBREQ_LOCATE, e->link, e);
			}
			else
				pmip_mh_send(&addrs, e->mh_vec, e->iovlen, e->link);	
		}
			struct timespec expires;
			clock_gettime(CLOCK_REALTIME, &e->add_time);
			tsadd(e->add_time, conf.N_RetsTime, expires);
			// Add a new task for deletion of entry if No Na is received.
			add_task_abs(&expires, &e->tqe, pmip_timer_bce_expired_handler);
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
	// define the MN interface identifier.
	struct in6_addr id = in6addr_any;
	struct nd_neighbor_solicit *msg = (struct nd_neighbor_solicit *) ih;

	//Outgoing NS message ?
	//check that Source is not MAG address.	
	if(IN6_ARE_ADDR_EQUAL(saddr,&conf.mag_addr_ingress) || IN6_ARE_ADDR_EQUAL(saddr,&conf.mag_addr_egress))	
		return;

	//CHECK target is not link local address 
	if (ipv6_addr_is_linklocal(&msg->nd_ns_target)) {  
		//dbg("Link Local target addres...ignore\n");
		return;
	}
	
	//CHECK target is not multicast.
	if (ipv6_addr_is_multicast(&msg->nd_ns_target)) {  
		//dbg("multicast target address...ignore\n");
		return;
	}

	//check that target is not MAG address interface & 
	//the prefix is PMIPv6 prefix.
	if (IN6_ARE_ADDR_EQUAL(&msg->nd_ns_target,&conf.mag_addr_ingress) || IN6_ARE_ADDR_EQUAL(&msg->nd_ns_target,&conf.mag_addr_egress))
	{
		//dbg("NS Target is one of MAG's addresses... Ignore\n");
		return;
	}			

	printf("-------------------------------------\n");
	dbg("Neighbor Solicitation (NS) Received\n");
	dbg("Received NS Src Addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(saddr));
	dbg("Received NS Dst addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(daddr));

	struct msg_info ns_info; bzero(&ns_info, sizeof(ns_info));
	icmp_ns_parse(&ns_info, (struct nd_neighbor_solicit *) ih, saddr, daddr, iif, hoplimit);
	ns_info.is_dad = (len - sizeof(struct nd_neighbor_solicit) == 0);
	mag_fsm(&ns_info);
}


static void pmip_mag_recv_pba(const struct ip6_mh *mh, ssize_t len, const struct in6_addr_bundle *in_addrs, int iif)
{
	printf("=====================================\n");	
	dbg("Proxy Binding Acknowledgement (PBA) Received\n");
	dbg("Received PBA Src Addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(in_addrs->src));
	dbg("Received PBA Dst addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(in_addrs->dst));
	//define the values for calling the parsing function
	//call the parsing function
	struct ip6_mh_binding_ack *pba = (const struct ip6_mh_binding_ack *) mh;
	
	//call the fsm function.
	struct msg_info info;
	mh_pba_parse(&info, pba, len,in_addrs,iif);
	mag_fsm(&info);
}

static void pmip_mag_recv_pbu(const struct ip6_mh *mh, ssize_t len, const struct in6_addr_bundle *in_addrs, int iif)
{
	printf("=====================================\n");	
	dbg("Proxy Binding Update (PBU) Received\n");
	dbg("Received PBU Src Addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(in_addrs->src));
	dbg("Received PBU Dst addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(in_addrs->dst));
	//define the values for the parsing function
	//call the parsing function
	struct ip6_mh_binding_update *pbu = (struct ip6_mh_binding_update *) mh;
	
	//call the fsm function.
	struct msg_info info; bzero(&info, sizeof(info));
	mh_pbu_parse(&info, pbu, len,in_addrs,iif);
	mag_fsm(&info);
}

static void pmip_lma_recv_pbu(const struct ip6_mh *mh, ssize_t len, const struct in6_addr_bundle *in_addrs, int iif)
{
	printf("=====================================\n");
	dbg("Proxy Binding Update (PBU) Received\n");
	dbg("Received PBU Src Addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(in_addrs->src));
	dbg("Received PBU Dst addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(in_addrs->dst));
	//define the values for the parsing function
	//call the parsing function
	struct ip6_mh_binding_update *pbu = (struct ip6_mh_binding_update *) mh;
	
	//call the fsm function.
	struct msg_info info; bzero(&info, sizeof(info));
	mh_pbu_parse(&info, pbu, len,in_addrs,iif);
	lma_fsm(&info);
}



static void pmip_mag_recv_na(const struct icmp6_hdr *ih, ssize_t len, 
	const struct in6_addr *saddr, const struct in6_addr *daddr, int iif, int hoplimit)
{

	// define the MN identifier
	struct in6_addr id = in6addr_any;
	struct nd_neighbor_advert *msg = (struct nd_neighbor_advert *) ih;

	//Check target is not link local address.
	if (ipv6_addr_is_linklocal(&msg->nd_na_target)) {  
		//dbg("ICMPv6 NA: Link Local target addres..\n");
		return;
	}
	
	//Check target is not multicast.
	if (ipv6_addr_is_multicast(&msg->nd_na_target)) {  
		//dbg("ICMPv6 NA: multicast target address..\n");
		return;
	}

 	if (len - sizeof(struct nd_neighbor_advert) > 0)
	{
		//dbg("Reply received for Solicited node: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&msg->nd_na_target));
		printf("-------------------------------------\n");
		dbg("Neighbor Advertisement (NA) Received\n");
		dbg("Received NA Src Addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(saddr));
		dbg("Received NA Dst addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(daddr));		
		struct msg_info na_info; bzero(&na_info, sizeof(na_info));
		icmp_na_parse(&na_info, (struct nd_neighbor_advert *) ih, saddr, daddr, iif, hoplimit);
		mag_fsm(&na_info);
  	}
	return;
}

static void pmip_recv_pbreq(const struct ip6_mh *mh, ssize_t len, const struct in6_addr_bundle *in_addrs, int iif)
{
	printf("=====================================\n");
	dbg("Proxy Binding Request (PBREQ) Received\n");
	dbg("Received PBREQ Src Addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(in_addrs->src));
	dbg("Received PBREQ Dst addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(in_addrs->dst));

	//define the values for the parsing function
	//call the parsing function
	struct ip6_mh_proxy_binding_request *pbreq = (struct ip6_mh_proxy_binding_request *) mh;

	//call the fsm function.
	struct msg_info info; bzero(&info, sizeof(info));
	mh_pbreq_parse(&info, pbreq, len,in_addrs,iif);
	if (is_mag()) mag_fsm(&info);
	else if (is_lma()) lma_fsm(&info);	
}


static void pmip_recv_pbres(const struct ip6_mh *mh, ssize_t len, const struct in6_addr_bundle *in_addrs, int iif)
{
	printf("=====================================\n");
	dbg("Proxy Binding Response (PBRES) Received\n");
	dbg("Received PBRES Src Addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(in_addrs->src));
	dbg("Received PBRES Dst addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(in_addrs->dst));

	//define the values for the parsing function
	//call the parsing function
	struct ip6_mh_proxy_binding_response *pbres = (struct ip6_mh_proxy_binding_response *) mh;
	
	struct msg_info info; bzero(&info, sizeof(info));
	mh_pbres_parse(&info, pbres, len,in_addrs,iif);
	if (is_mag()) mag_fsm(&info);
	else if (is_lma()) lma_fsm(&info);	
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

struct icmp6_handler pmip_mag_recv_na_handler = {
        .recv = pmip_mag_recv_na
};

struct mh_handler pmip_pbreq_handler = {
	.recv = pmip_recv_pbreq
};

struct mh_handler pmip_pbres_handler = {
	.recv = pmip_recv_pbres
};
