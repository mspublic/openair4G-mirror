/* $Id: dhaad_mn.h,v 1.1.1.1 2008/04/23 13:21:04 nguyenhn Exp $ */

#ifndef __DHAAD_MN_H__
#define __DHAAD_MN_H__ 1

#include "list.h"

struct ha_candidate {
	struct list_head list;
	struct in6_addr addr;
	int retry;
};

struct icmp6_hdr;

/* Mobile Node functions */
struct home_addr_info;

void dhaad_start(struct home_addr_info *hai);
void dhaad_stop(struct home_addr_info *hai);

int dhaad_next_candidate(struct home_addr_info *hai);

int dhaad_home_reg_failed(struct home_addr_info *hai);

void dhaad_mn_init(void);
void dhaad_mn_cleanup(void);

#endif
