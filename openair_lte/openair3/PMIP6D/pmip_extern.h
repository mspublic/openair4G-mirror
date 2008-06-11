/*****************************************************************
 * C header: pmip_extern.h
 * Description: Describe all external functions and variables
 * Author: 
 *   Christian Bonnet
 *   Huu-Nghia Nguyen
 *   Hussain & Daniel
 * Copyright: Eurecom Institute, (C) 2008
 ******************************************************************/

#ifndef __pmip_extern_h
#define __pmip_extern_h

#include "pmip_types.h"
#include "pmip_consts.h"
#include <netinet/in.h>
#include <netinet/ip6mh.h>
#include "icmp6.h"

//--------------------------------------------------------------
//pmip_msgs.c
//--------------------------------------------------------------
//void mh_send_pbu(const struct in6_addr_bundle *addrs,struct pmip_entry *bce, int oif);
//void mh_send_pba(const struct in6_addr_bundle *addrs,struct pmip_entry *bce, int oif);
//struct pmip_entry *mh_pbu_parse(struct ip6_mh_binding_update *pbu, ssize_t len,const struct in6_addr_bundle *in_addrs,int iif);
//struct pmip_entry * mh_pba_parse(struct ip6_mh_binding_ack *pba, ssize_t len,const struct in6_addr_bundle *in_addrs, int iif);
//int pmip_mh_send(const struct in6_addr_bundle *addrs, const struct iovec *mh_vec, int iovlen, int oif);
//void mh_send_pbreq(struct pmip_entry *bce);
//void mh_send_pbres(struct pmip_entry *bce);
//struct pmip_entry *mh_pbreq_parse(struct ip6_mh_proxy_binding_request *pbr, ssize_t len,const struct in6_addr_bundle *in_addrs,int iif);
//void mh_send_pbres(struct pmip_entry *bce);
//struct pmip_entry *mh_pbres_parse(struct ip6_mh_proxy_binding_response *pbre, ssize_t len,const struct in6_addr_bundle *in_addrs,int iif);



//MH_New options.
int mh_create_opt_home_net_prefix(struct iovec *iov,struct in6_addr *Home_Network_Prefix);
int mh_create_opt_mn_identifier (struct iovec *iov, int flags, __identifier * MN_ID);
int mh_create_opt_time_stamp (struct iovec *iov, __timestamp * Timestamp );
int mh_create_opt_link_local_add (struct iovec *iov, struct in6_addr *LinkLocal );
int mh_create_opt_serv_mag_addr(struct iovec *iov,struct in6_addr *Serv_MAG_addr);

//--------------------------------------------------------------
//pmip_handler.c
//--------------------------------------------------------------
void _EXPIRED(struct tq_elem *tqe);	/** pointer to task upon entry expiry! */
int COMPARE(struct in6_addr *addr1,struct in6_addr *addr2);

extern struct icmp6_handler pmip_mag_ns_handler;
extern struct mh_handler pmip_mag_pbu_handler;
extern struct mh_handler pmip_mag_pba_handler;
extern struct mh_handler pmip_lma_pbu_handler;
extern struct icmp6_handler pmip_recv_na_handler;
extern struct mh_handler pmip_pbreq_handler;
extern struct mh_handler pmip_pbres_handler;

//--------------------------------------------------------------
//pmip_fsm.c
//--------------------------------------------------------------
//int pmip_fsm(struct in6_addr_bundle *addrs,struct pmip_entry * info, int iif);

#endif 
