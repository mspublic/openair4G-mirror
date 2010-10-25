/* $Id: tunnelctl.h,v 1.1.1.1 2008/04/23 13:21:04 nguyenhn Exp $ */
#ifndef __TUNNELCTL_H__
#define __TUNNELCTL_H__ 1

int tunnel_add(struct in6_addr *local,
	       struct in6_addr *remote,
	       int link,
	       int (*ext_tunnel_ops)(int request,
				     int old_if,
				     int new_if,
				     void *data),
	       void *data);

int tunnel_mod(int ifindex,
	       struct in6_addr *local,
	       struct in6_addr *remote,
	       int link,
	       int (*ext_tunnel_ops)(int request,
				     int old_if,
				     int new_if,
				     void *data),
	       void *data);

int tunnel_del(int ifindex,
	       int (*ext_tunnel_ops)(int request,
				     int old_if,
				     int new_if,
				     void *data),
	       void *data);


int tunnelctl_init(void);

void tunnelctl_cleanup(void);

int tunnel_getusers(int tun_index);
#endif
