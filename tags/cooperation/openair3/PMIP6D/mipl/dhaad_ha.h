/* $Id: dhaad_ha.h,v 1.1.1.1 2008/04/23 13:21:04 nguyenhn Exp $ */

#ifndef __DHAAD_HA_H__
#define __DHAAD_HA_H__ 1

#define MAX_HOME_AGENTS 77

struct icmp6_hdr;

/* Home Agent functions */

struct ha_interface;
struct nd_opt_prefix_info;

#ifdef ENABLE_VT
void dhaad_halist_iterate(struct ha_interface *iface,
			  int (* func)(int, void *, void *), void *arg);
#endif

void dhaad_insert_halist(struct ha_interface *i, 
			 uint16_t key, uint16_t life_sec,
			 struct nd_opt_prefix_info *pinfo,
			 const struct in6_addr *lladdr);

int dhaad_ha_init(void);
void dhaad_ha_cleanup(void);

#endif