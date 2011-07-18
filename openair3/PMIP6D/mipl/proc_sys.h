/* $Id: proc_sys.h,v 1.1.1.1 2008/04/23 13:21:04 nguyenhn Exp $ */

#ifndef __PROC_SYS_H__
#define __PROC_SYS_H__ 1

#define PROC_SYS_IP6_AUTOCONF "/proc/sys/net/ipv6/conf/%s/autoconf"
#define PROC_SYS_IP6_ACCEPT_RA "/proc/sys/net/ipv6/conf/%s/accept_ra"
#define PROC_SYS_IP6_RTR_SOLICITS "/proc/sys/net/ipv6/conf/%s/router_solicitations"
#define PROC_SYS_IP6_RTR_SOLICIT_INTERVAL "/proc/sys/net/ipv6/conf/%s/router_solicitation_interval"
#define PROC_SYS_IP6_LINKMTU "/proc/sys/net/ipv6/conf/%s/mtu"
#define PROC_SYS_IP6_CURHLIM "/proc/sys/net/ipv6/conf/%s/hop_limit"
#define PROC_SYS_IP6_APP_SOLICIT "/proc/sys/net/ipv6/neigh/%s/app_solicit"
#define PROC_SYS_IP6_BASEREACHTIME_MS "/proc/sys/net/ipv6/neigh/%s/base_reachable_time_ms"
#define PROC_SYS_IP6_RETRANSTIMER_MS "/proc/sys/net/ipv6/neigh/%s/retrans_time_ms"

int set_iface_proc_entry(const char *tmpl, const char *if_name, int val);

int get_iface_proc_entry(const char *tmpl, const char *if_name, int *val);

#endif
