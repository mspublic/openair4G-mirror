/* Do not edit.  This file is automatically created during make. */
/* $Id: pmgr.h.in 1.1 05/02/21 14:45:42+02:00 anttit@tcs.hut.fi $ */

#ifndef __PMGR_H__
#define __PMGR_H__ 1

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <limits.h>
#include "policy.h"

struct pmgr_cb {
	int (*best_iface) (const struct in6_addr * hoa,
			   const struct in6_addr * ha, int pref_iface);
	int (*best_coa) (const struct in6_addr * hoa,
			 const struct in6_addr * ha, int iif,
			 const struct in6_addr * pref_coa,
			 struct in6_addr * coa);
	int (*max_binding_life) (const struct in6_addr * remote_hoa,
				 const struct in6_addr * remote_coa,
				 const struct in6_addr * local_addr,
				 const struct ip6_mh_binding_update * bu,
				 ssize_t len,
				 const struct timespec * suggested,
				 struct timespec * lifetime);
	int (*discard_binding) (const struct in6_addr * remote_hoa,
				const struct in6_addr * remote_coa,
				const struct in6_addr * local_addr,
				const struct ip6_mh_binding_update * bu,
				ssize_t len);
	int (*use_bradv) (const struct in6_addr * remote_hoa,
			  const struct in6_addr * remote_coa,
			  const struct in6_addr * local_addr,
			  const struct timespec * lft,
			  struct timespec * refresh);
	int (*use_keymgm) (const struct in6_addr * remote_addr,
			   const struct in6_addr * local_addr);
	int (*accept_ra) (int iif,
			  const struct in6_addr * saddr,
			  const struct in6_addr * daddr,
			  const struct nd_router_advert * ra);
	int (*best_ro_coa) (const struct in6_addr * hoa,
			    const struct in6_addr * cn,
			    struct in6_addr * coa);
	int (*accept_inet6_iface) (int iif);
	char so_path[_POSIX_PATH_MAX + 1];
	void *handle;
	struct pmgr_cb *old;
};

int pmgr_init(char *libpath, struct pmgr_cb *lb);

int pmgr_close(struct pmgr_cb *lb);

#endif
