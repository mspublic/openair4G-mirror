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
#include "pmip_cache.h"
#include <netinet/in.h>
#include <netinet/ip6mh.h>
#include "icmp6.h"

//--------------------------------------------------------------
//pmip_handler.c
//--------------------------------------------------------------
void pmip_timer_bce_expired_handler(struct tq_elem *tqe);	/** pointer to task upon entry expiry! */
struct in6_addr get_node_id (struct in6_addr * mn_addr);
struct in6_addr get_node_prefix(struct in6_addr * mn_addr);
struct in6_addr *get_mn_addr(struct pmip_entry * bce);

extern struct icmp6_handler pmip_mag_ns_handler;
extern struct mh_handler pmip_mag_pbu_handler;
extern struct mh_handler pmip_mag_pba_handler;
extern struct mh_handler pmip_lma_pbu_handler;
extern struct icmp6_handler pmip_mag_recv_na_handler;
extern struct mh_handler pmip_pbreq_handler;
extern struct mh_handler pmip_pbres_handler;

//--------------------------------------------------------------
//pmip_fsm.c
//--------------------------------------------------------------
//int pmip_fsm(struct in6_addr_bundle *addrs,struct pmip_entry * info, int iif);

//--------------------------------------------------------------
//pmip_mag_proc.c
//--------------------------------------------------------------
int mag_setup_route(struct in6_addr * pmip6_addr, int downlink);
int mag_remove_route(struct in6_addr * pmip6_addr, int downlink);
int mag_dereg(struct pmip_entry * bce, int propagate);
int mag_dereg_update_proxy_ndisc(struct pmip_entry * bce);
int mag_reg_update_proxy_ndisc(struct pmip_entry * bce);
int mag_pmip_detect_ro(struct msg_info* info);
int mag_start_registration(struct pmip_entry* bce);
int mag_end_registration(struct pmip_entry* bce, int iif);
int mag_update_binding_entry(struct pmip_entry * bce, struct msg_info * info);
int mag_pmip_md(struct msg_info* info, struct pmip_entry *bce);

#endif 
