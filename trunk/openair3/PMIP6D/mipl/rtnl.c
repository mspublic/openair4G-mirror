/*
 * $Id: rtnl.c,v 1.1.1.1 2008/04/23 13:21:04 nguyenhn Exp $
 *
 * This file is part of the MIPL Mobile IPv6 for Linux.
 * 
 * Authors:
 *  Ville Nuorvala <vnuorval@tcs.hut.fi>,
 *  Antti Tuominen <anttit@tcs.hut.fi>
 *
 * Copyright 2003-2005 Go-Core Project
 * Copyright 2003-2006 Helsinki University of Technology
 *
 * MIPL Mobile IPv6 for Linux is free software; you can redistribute
 * it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; version 2 of
 * the License.
 *
 * MIPL Mobile IPv6 for Linux is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MIPL Mobile IPv6 for Linux; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307 USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <time.h>
#include <syslog.h>
#include <netinet/in.h>
#include <netinet/icmp6.h>
#include <libnetlink.h>
#include <sys/uio.h>
#include <sys/socket.h>

#include "debug.h"
#include "rtnl.h"

#define RT_DEBUG_LEVEL 0

#if RT_DEBUG_LEVEL >= 1
#define RTDBG dbg
#else 
#define RTDBG(...) 
#endif /* RTDBG */

int rtnl_ext_open(struct rtnl_handle *rth, int proto, unsigned subscriptions)
{
	socklen_t addr_len;

	memset(rth, 0, sizeof(rth));

	rth->fd = socket(AF_NETLINK, SOCK_RAW, proto);
	if (rth->fd < 0) {
		syslog(LOG_ERR,
		       "Unable to open netlink socket! "
		       "Do you have root permissions?");
		return -1;
	}

	memset(&rth->local, 0, sizeof(rth->local));
	rth->local.nl_family = AF_NETLINK;
	rth->local.nl_groups = subscriptions;

	if (bind(rth->fd, (struct sockaddr*)&rth->local, sizeof(rth->local)) < 0) {
		return -1;
	}
	addr_len = sizeof(rth->local);
	if (getsockname(rth->fd, (struct sockaddr*)&rth->local, &addr_len) < 0) {
		return -1;
	}
	if (addr_len != sizeof(rth->local)) {
		return -1;
	}
	if (rth->local.nl_family != AF_NETLINK) {
		return -1;
	}
	rth->seq = time(NULL);
	return 0;
}

int rtnl_ext_listen(struct rtnl_handle *rtnl, 
		    int (*handler)(struct sockaddr_nl *,
				   struct nlmsghdr *n,
				   void *),
		    void *jarg)
{
	int status;
	struct nlmsghdr *h;
	struct sockaddr_nl nladdr;
	struct iovec iov;
	char buf[1024];
	struct msghdr msg;

	msg.msg_name = (void*)&nladdr;
	msg.msg_namelen = sizeof(nladdr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;

	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;
	nladdr.nl_pid = 0;
	nladdr.nl_groups = 0;


	iov.iov_base = buf;

	while (1) {
		iov.iov_len = sizeof(buf);
		status = recvmsg(rtnl->fd, &msg, 0);

		if (status < 0) {
			if (errno == EBADF)
				return -1;
			continue;
		}
		if (status == 0)
			return 0;

		if (msg.msg_namelen != sizeof(nladdr))
			continue;

		for (h = (struct nlmsghdr*)buf; status >= sizeof(*h); ) {
			int err;
			int len = h->nlmsg_len;
			int l = len - sizeof(*h);

			if (l < 0 || len > status)
				break;

			err = handler(&nladdr, h, jarg);
			if (err < 0)
				break;

			status -= NLMSG_ALIGN(len);
			h = (struct nlmsghdr*)((char*)h + NLMSG_ALIGN(len));
		}
	}
}

int rtnl_do(int proto, struct nlmsghdr *sn, struct nlmsghdr *rn)
{
	struct rtnl_handle rth;
	int err;
	if (rtnl_ext_open(&rth, proto, 0) < 0) {
		dbg("huh?\n");
		return -1;
	}
	err = rtnl_talk(&rth, sn, 0, 0, rn, NULL, NULL);
	rtnl_close(&rth);
	return err;
}

int addr_do(const struct in6_addr *addr, int plen, int ifindex, void *arg,
	    int (*do_callback)(struct ifaddrmsg *ifa, 
			       struct rtattr *rta_tb[], void *arg))
{
	uint8_t sbuf[256];
	uint8_t rbuf[256];
	struct nlmsghdr *sn, *rn;
	struct ifaddrmsg *ifa;
	int err;
	struct rtattr *rta_tb[IFA_MAX+1];

	memset(sbuf, 0, sizeof(sbuf));
	sn = (struct nlmsghdr *)sbuf;
	sn->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
	sn->nlmsg_flags = NLM_F_REQUEST;
	sn->nlmsg_type = RTM_GETADDR;

	ifa = NLMSG_DATA(sn);
	ifa->ifa_family = AF_INET6;
	ifa->ifa_prefixlen = plen;
	ifa->ifa_scope = RT_SCOPE_UNIVERSE;
	ifa->ifa_index = ifindex;

	addattr_l(sn, sizeof(sbuf), IFA_LOCAL, addr, sizeof(*addr));

	memset(rbuf, 0, sizeof(rbuf));
	rn = (struct nlmsghdr *)rbuf;
	err = rtnl_route_do(sn, rn);
	if (err < 0) {
		rn = sn;
		ifa = NLMSG_DATA(rn);
	} else {
		ifa = NLMSG_DATA(rn);
		
		if (rn->nlmsg_type != RTM_NEWADDR || 
		    rn->nlmsg_len < NLMSG_LENGTH(sizeof(*ifa)) ||
		    ifa->ifa_family != AF_INET6) {
			return -EINVAL;
		}
	}
	memset(rta_tb, 0, sizeof(rta_tb));
	parse_rtattr(rta_tb, IFA_MAX, IFA_RTA(ifa), 
		     rn->nlmsg_len - NLMSG_LENGTH(sizeof(*ifa)));

	if (!rta_tb[IFA_ADDRESS])
		rta_tb[IFA_ADDRESS] = rta_tb[IFA_LOCAL];

	if (!rta_tb[IFA_ADDRESS] ||
	    !IN6_ARE_ADDR_EQUAL(RTA_DATA(rta_tb[IFA_ADDRESS]), addr)) {
		return -EINVAL;
	}
	if (do_callback)
		err = do_callback(ifa, rta_tb, arg);

	return err;

}

static int addr_mod(int cmd, uint16_t nlmsg_flags,
		    const struct in6_addr *addr, uint8_t plen, 
		    uint8_t flags, uint8_t scope, int ifindex, 
		    uint32_t prefered, uint32_t valid)
				      
{
	uint8_t buf[256];
	struct nlmsghdr *n;
	struct ifaddrmsg *ifa;

	memset(buf, 0, sizeof(buf));
	n = (struct nlmsghdr *)buf;
	n->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
	n->nlmsg_flags = NLM_F_REQUEST | nlmsg_flags;
	n->nlmsg_type = cmd;

	ifa = NLMSG_DATA(n);
	ifa->ifa_family = AF_INET6;
	ifa->ifa_prefixlen = plen;
	ifa->ifa_flags = flags;
	ifa->ifa_scope = scope;
	ifa->ifa_index = ifindex;

	addattr_l(n, sizeof(buf), IFA_LOCAL, addr, sizeof(*addr));

	if (prefered || valid) {
		struct ifa_cacheinfo ci;
		ci.ifa_prefered = prefered;
		ci.ifa_valid = valid;
		ci.cstamp = 0;
		ci.tstamp = 0;
		addattr_l(n, sizeof(buf), IFA_CACHEINFO, &ci, sizeof(ci));
	}
	return rtnl_route_do(n, NULL);
}

int addr_add(const struct in6_addr *addr, uint8_t plen, 
	     uint8_t flags, uint8_t scope, int ifindex, 
	     uint32_t prefered, uint32_t valid)
{
	return addr_mod(RTM_NEWADDR, NLM_F_CREATE|NLM_F_REPLACE,
			addr, plen, flags, scope, ifindex, prefered, valid);
}


int addr_del(const struct in6_addr *addr, uint8_t plen, int ifindex)
{
	return addr_mod(RTM_DELADDR, 0, addr, plen, 0, 0, ifindex, 0, 0);
}

int prefix_add(int ifindex, const struct nd_opt_prefix_info *pinfo)
{
	uint8_t buf[128];
	struct nlmsghdr *n;
	struct prefixmsg *pfxm;
	struct prefix_cacheinfo ci;

	memset(buf, 0, sizeof(buf));
	n = (struct nlmsghdr *)buf;
	n->nlmsg_len = NLMSG_LENGTH(sizeof(struct prefixmsg));
	n->nlmsg_flags = NLM_F_REQUEST|NLM_F_CREATE|NLM_F_REPLACE;
	n->nlmsg_type = RTM_NEWPREFIX;

	pfxm = NLMSG_DATA(n);
	pfxm->prefix_family = AF_INET6;
	pfxm->prefix_ifindex = ifindex;
	pfxm->prefix_type = pinfo->nd_opt_pi_type;	    
	pfxm->prefix_len = pinfo->nd_opt_pi_prefix_len;
	pfxm->prefix_flags = pinfo->nd_opt_pi_flags_reserved;

	addattr_l(n, sizeof(buf), PREFIX_ADDRESS, &pinfo->nd_opt_pi_prefix,
		  sizeof(struct in6_addr));
	memset(&ci, 0, sizeof(ci));
	/* pinfo lifetimes stored locally in host byte order */
	ci.valid_time = htonl(pinfo->nd_opt_pi_valid_time);
	ci.preferred_time = htonl(pinfo->nd_opt_pi_preferred_time);
	addattr_l(n, sizeof(buf), PREFIX_CACHEINFO, &ci, sizeof(ci));

	return rtnl_route_do(n, NULL);
}

static int route_mod(int cmd, int oif, uint8_t table, uint8_t proto,
		     unsigned flags, uint32_t priority,
		     const struct in6_addr *src, int src_plen,
		     const struct in6_addr *dst, int dst_plen,
		     const struct in6_addr *gateway)
{
	uint8_t buf[512];
	struct nlmsghdr *n;
	struct rtmsg *rtm;

	if (cmd == RTM_NEWROUTE && oif == 0)
		return -1;

	memset(buf, 0, sizeof(buf));
	n = (struct nlmsghdr *)buf;
	
	n->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
	n->nlmsg_flags = NLM_F_REQUEST;
	if (cmd == RTM_NEWROUTE) {
		n->nlmsg_flags |= NLM_F_CREATE|NLM_F_EXCL;
	}
	n->nlmsg_type = cmd;

	rtm = NLMSG_DATA(n);
	rtm->rtm_family = AF_INET6;
	rtm->rtm_dst_len = dst_plen;
	rtm->rtm_src_len = src_plen;
	rtm->rtm_table = table;
	rtm->rtm_protocol = proto;
	rtm->rtm_scope = RT_SCOPE_UNIVERSE;
	rtm->rtm_type = RTN_UNICAST;
	rtm->rtm_flags = flags;

	addattr_l(n, sizeof(buf), RTA_DST, dst, sizeof(*dst));
	if (src)
		addattr_l(n, sizeof(buf), RTA_SRC, src, sizeof(*src));
	addattr32(n, sizeof(buf), RTA_OIF, oif);
	if (gateway)
		addattr_l(n, sizeof(buf), 
			  RTA_GATEWAY, gateway, sizeof(*gateway));
	if (priority)
		addattr32(n, sizeof(buf), RTA_PRIORITY, priority);
	return rtnl_route_do(n, NULL);
}


/**
 * route_add - add route to kernel routing table
 * @oif: outgoing interface
 * @table: routing table number
 * @metric: route preference
 * @src: source prefix
 * @src_plen: source prefix length
 * @dst: destination prefix
 * @dst_plen: destination prefix length
 * @gateway: possible gateway
 *
 * Adds a new route through interface @oif, with source
 * @src/@src_plen, to destinations specified by @dst/@dst_plen.  Route
 * will be added to routing table number @table.  Returns zero on
 * success, negative otherwise.
 **/
int route_add(int oif, uint8_t table, uint8_t proto,
	      unsigned flags, uint32_t metric, 
	      const struct in6_addr *src, int src_plen,
	      const struct in6_addr *dst, int dst_plen, 
	      const struct in6_addr *gateway)
{
	return route_mod(RTM_NEWROUTE, oif, table, proto, flags,
			 metric, src, src_plen, dst, dst_plen, gateway);
}

/**
 * route_del - delete route from kernel routing table
 * @oif: outgoing interface
 * @table: routing table number
 * @metric: route preference
 * @src: source prefix
 * @src_plen: source prefix length
 * @dst: destination prefix
 * @dst_plen: destination prefix length
 * @gateway: possible gateway
 *
 * Deletes an entry with @src/@src_plen as source and @dst/@dst_plen
 * as destination, through interface @oif, from the routing table
 * number @table.
 **/
int route_del(int oif, uint8_t table, uint32_t metric, 
	      const struct in6_addr *src, int src_plen,
	      const struct in6_addr *dst, int dst_plen,
	      const struct in6_addr *gateway)
{
	return route_mod(RTM_DELROUTE, oif, table, RTPROT_UNSPEC, 
			 0, metric, src, src_plen, dst, dst_plen, gateway);
}

static int rule_mod(const char *iface, int cmd, uint8_t table, 
		    uint32_t priority, uint8_t action,
		    const struct in6_addr *src, int src_plen,
		    const struct in6_addr *dst, int dst_plen)
{
	uint8_t buf[512];
	struct nlmsghdr *n;
	struct rtmsg *rtm;

	memset(buf, 0, sizeof(buf));
	n = (struct nlmsghdr *)buf;
	
	n->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
	n->nlmsg_flags = NLM_F_REQUEST;
	if (cmd == RTM_NEWRULE) {
		n->nlmsg_flags |= NLM_F_CREATE;
	}
	n->nlmsg_type = cmd;

	rtm = NLMSG_DATA(n);
	rtm->rtm_family = AF_INET6;
	rtm->rtm_dst_len = dst_plen;
	rtm->rtm_src_len = src_plen;
	rtm->rtm_table = table;
	rtm->rtm_scope = RT_SCOPE_UNIVERSE;
	rtm->rtm_type = action;

	addattr_l(n, sizeof(buf), RTA_DST, dst, sizeof(*dst));
	if (src)
		addattr_l(n, sizeof(buf), RTA_SRC, src, sizeof(*src));
	if (priority)
		addattr32(n, sizeof(buf), RTA_PRIORITY, priority);
	if (iface)
		addattr_l(n, sizeof(buf), RTA_IIF, iface, strlen(iface) + 1);

	return rtnl_route_do(n, NULL);
}

/**
 * rule_add - add rule for routes
 * @src: source prefix
 * @src_plen: source prefix length
 * @dst: destination prefix
 * @dst_plen: destination prefix length
 *
 * Add routing rule for routes with @src/@src_plen source and
 * @dst/@dst_plen destination.  Returns table number on success,
 * negative otherwise.
 **/
int rule_add(const char *iface, uint8_t table,
	     uint32_t priority, uint8_t action,
	     const struct in6_addr *src, int src_plen,
	     const struct in6_addr *dst, int dst_plen)
{
	return rule_mod(iface, RTM_NEWRULE, table, 
			priority, action,
			src, src_plen, dst, dst_plen);
}

/**
 * rule_del - delete rule for routes
 * @src: source prefix
 * @src_plen: source prefix length
 * @dst: destination prefix
 * @dst_plen: destination prefix length
 *
 * Deletes routing rule for routes with @src/@src_plen source and
 * @dst/@dst_plen destination.  Returns zero on success, negative
 * otherwise.
 **/
int rule_del(const char *iface, uint8_t table,
	     uint32_t priority, uint8_t action,
	     const struct in6_addr *src, int src_plen,
	     const struct in6_addr *dst, int dst_plen)
{
	return rule_mod(iface, RTM_DELRULE, table, 
			priority, action,
			src, src_plen, dst, dst_plen);
}

int rtnl_iterate(int proto, int type,
	int (*func)(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg),
	void *extarg)
{
	struct rtnl_handle rth;

	if (rtnl_ext_open(&rth, proto, 0) < 0)
		return -1;

	if (rtnl_wilddump_request(&rth, AF_INET6, type) < 0) {
		rtnl_close(&rth);
		return -1;
	}

	if (rtnl_dump_filter(&rth, func, extarg, NULL, NULL) < 0) {
		rtnl_close(&rth);
		return -1;
	}

	rtnl_close(&rth);

	return 0;
}
