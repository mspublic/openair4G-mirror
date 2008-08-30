#ifndef _NET_IP6_ROUTE_H
#define _NET_IP6_ROUTE_H

#define IP6_RT_PRIO_FW		16
#define IP6_RT_PRIO_USER	1024
#define IP6_RT_PRIO_ADDRCONF	256
#define IP6_RT_PRIO_KERN	512
#define IP6_RT_FLOW_MASK	0x00ff

#ifdef __KERNEL__

#include <net/flow.h>
#include <net/ip6_fib.h>
#include <net/sock.h>
#include <linux/ip.h>
#include <linux/ipv6.h>

#define RT6_TABLE_UNSPEC RT_TABLE_UNSPEC
#define RT6_TABLE_MAIN RT_TABLE_MAIN

#ifdef CONFIG_IPV6_MULTIPLE_TABLES
#define RT6_TABLE_MIN 1
#define RT6_TABLE_LOCAL RT_TABLE_LOCAL
#define RT6_TABLE_MAX RT_TABLE_MAX
#else
#define RT6_TABLE_MIN RT6_TABLE_MAIN
#define RT6_TABLE_LOCAL RT6_TABLE_MAIN
#define RT6_TABLE_MAX RT6_TABLE_MAIN
#endif

#define RT6_F_STRICT		0x1
#define RT6_F_HAS_SADDR		0x2

struct rt6_table {
	unsigned char table_id;
	unsigned char dead;
	atomic_t refcnt;
	rwlock_t lock;
	struct fib6_node root;
	struct rt6_info *dflt_pointer;
	spinlock_t dflt_lock;
};

struct fib6_rule
{
	struct fib6_rule *r_next;
	atomic_t	r_clntref;
	u32		r_preference;
	struct rt6key   r_src;
	struct rt6key   r_dst;
	int		r_ifindex;
	u8		r_tclass;
	u8		r_flags;
	unsigned char	r_action;
	unsigned char	r_table;
	char		r_ifname[IFNAMSIZ];
	int		r_dead;
};

#ifdef CONFIG_IPV6_SUBTREES

#define FIB6_SUBTREE(fn) ((fn)->subtree)

extern struct fib6_node		*fib6_subtree_lookup(struct fib6_node *parent,
						     struct in6_addr *saddr);

struct inet6_ifaddr;

extern void			rt6_ifa_del(struct inet6_ifaddr *ifa);

#else

#define FIB6_SUBTREE(fn) NULL

static inline struct fib6_node 	*fib6_subtree_lookup(struct fib6_node *parent,
						     struct in6_addr *saddr)
{
	return parent;
}
#endif /* CONFIG_IPV6_SUBTREES */

extern struct rt6_info	ip6_null_entry;

extern int ip6_rt_gc_interval;

#ifdef CONFIG_IPV6_MULTIPLE_TABLES

extern struct rt6_info	ip6_prohibit_entry;
extern struct rt6_info	ip6_blk_hole_entry;

extern struct rt6_table *rt6_tables[RT6_TABLE_MAX + 1];

struct dst_entry *
rt6_rule_lookup(struct flowi *fl, int flags,
		struct rt6_info *( *tb_pol_lookup)(struct rt6_table *table,
						   struct flowi *fl,
						   int flags,
						   struct fib6_rule *rule));

extern int inet6_rtm_delrule(struct sk_buff *skb, struct nlmsghdr* nlh, void *arg);

extern int inet6_rtm_newrule(struct sk_buff *skb, struct nlmsghdr* nlh, void *arg);

extern int inet6_dump_rules(struct sk_buff *skb, struct netlink_callback *cb);

extern void __init fib6_rules_init(void);

extern void fib6_rules_exit(void);

#else

extern struct rt6_table	ip6_routing_table;

static inline struct dst_entry *
rt6_rule_lookup(struct flowi *fl, int flags,
		struct rt6_info *( *tb_pol_lookup)(struct rt6_table *table,
						   struct flowi *fl,
						   int flags,
						   struct fib6_rule *rule))
{
	return (struct dst_entry *) tb_pol_lookup(&ip6_routing_table,
						  fl, flags, NULL);
}
#endif /*  CONFIG_IPV6_MULTIPLE_TABLES */


extern struct rt6_info *	ip6_pol_route_input(struct rt6_table *table,
						    struct flowi *fl,
						    int flags,
						    struct fib6_rule *rule);

extern struct rt6_info *	ip6_pol_route_output(struct rt6_table *table,
						     struct flowi *fl,
						     int flags,
						     struct fib6_rule *rule);

extern struct rt6_info *	ip6_pol_route_lookup(struct rt6_table *table,
						     struct flowi *fl,
						     int flags,
						     struct fib6_rule *rule);

static inline int rt6_need_strict(struct in6_addr *daddr)
{
	return (ipv6_addr_type(daddr) &
		(IPV6_ADDR_MULTICAST|IPV6_ADDR_LINKLOCAL));
}

static inline void ip6_route_input(struct sk_buff *skb)
{
	struct ipv6hdr *iph = skb->nh.ipv6h;
	int flags = rt6_need_strict(&iph->daddr) ?
		RT6_F_STRICT|RT6_F_HAS_SADDR : RT6_F_HAS_SADDR;

	struct flowi fl = {
		.iif = skb->dev->ifindex,
		.nl_u =
		{ .ip6_u =
		  { .daddr = iph->daddr,
		    .saddr = iph->saddr,
		    .flowlabel = (* (__u32 *) iph)&IPV6_FLOWINFO_MASK, } },
		.proto = iph->nexthdr,
	};
	skb->dst = rt6_rule_lookup(&fl, flags, ip6_pol_route_input);
}

static inline struct dst_entry *ip6_route_output(struct sock *sk,
						 struct flowi *fl)
{
	int flags = rt6_need_strict(&fl->fl6_dst) ? RT6_F_STRICT : 0;

	/* In some cases (RS, DAD probe) the unspecified address may be used
	   as a source address. Such packets can only be sent from userspace
	   if the user passes the whole IPv6 header to the kernel. */

	if (!ipv6_addr_any(&fl->fl6_src) || (sk && inet_sk(sk)->hdrincl))
		flags |= RT6_F_HAS_SADDR;

	return rt6_rule_lookup(fl, flags, ip6_pol_route_output);
}

static inline struct rt6_info *rt6_lookup(struct in6_addr *daddr,
					  struct in6_addr *saddr,
					  int oif, int strict)
{
	struct flowi fl = {
		.oif = oif,
		.nl_u =
		{ .ip6_u =
		  { .daddr = *daddr,
		    .saddr = saddr ? *saddr : in6addr_any, } },
	};
	struct dst_entry *dst;
	int flags = strict ? RT6_F_STRICT :  0;

	if (saddr)
		flags |= RT6_F_HAS_SADDR;

	dst = rt6_rule_lookup(&fl, flags, ip6_pol_route_lookup);
	if (dst->error == 0)
		return (struct rt6_info *) dst;
	dst_release(dst);
	return NULL;
}

extern int			ip6_route_me_harder(struct sk_buff *skb);

extern void			ip6_route_init(void);
extern void			ip6_route_cleanup(void);

extern int			ipv6_route_ioctl(unsigned int cmd, void __user *arg);

extern int			ip6_route_add(struct in6_rtmsg *rtmsg,
					      struct nlmsghdr *,
					      void *rtattr,
					      struct netlink_skb_parms *req,
					      unsigned char table_id);
extern int			ip6_ins_rt(struct rt6_info *,
					   struct nlmsghdr *,
					   void *rtattr,
					   struct netlink_skb_parms *req);
extern int			ip6_del_rt(struct rt6_info *,
					   struct nlmsghdr *,
					   void *rtattr,
					   struct netlink_skb_parms *req);

extern int			ip6_rt_addr_add(struct in6_addr *addr,
						struct net_device *dev,
						int anycast);

extern int			ip6_rt_addr_del(struct in6_addr *addr,
						struct net_device *dev);

extern void			rt6_sndmsg(int type, struct in6_addr *dst,
					   struct in6_addr *src,
					   struct in6_addr *gw,
					   struct net_device *dev,
					   int dstlen, int srclen,
					   int metric, __u32 flags);

extern struct dst_entry *ndisc_dst_alloc(struct net_device *dev,
					 struct neighbour *neigh,
					 struct in6_addr *addr,
					 int (*output)(struct sk_buff *));
extern int ndisc_dst_gc(int *more);
extern void fib6_force_start_gc(void);

extern struct rt6_info *addrconf_dst_alloc(struct inet6_dev *idev,
					   const struct in6_addr *addr,
					   int anycast);

/*
 *	support functions for ND
 *
 */
extern struct rt6_info *	rt6_get_dflt_router(struct in6_addr *addr,
						    struct net_device *dev);
extern struct rt6_info *	rt6_add_dflt_router(struct in6_addr *gwaddr,
						    struct net_device *dev);

extern void			rt6_purge_dflt_routers(int ifindex);

extern void			rt6_reset_dflt_pointer(struct rt6_info *rt);

extern void			rt6_redirect(struct in6_addr *dest,
					     struct in6_addr *saddr,
					     struct in6_addr *gateway,
					     struct neighbour *neigh,
					     u8 *lladdr,
					     int on_link);

extern void			rt6_pmtu_discovery(struct in6_addr *daddr,
						   struct in6_addr *saddr,
						   struct net_device *dev,
						   u32 pmtu);

struct nlmsghdr;
struct netlink_callback;
extern int inet6_dump_fib(struct sk_buff *skb, struct netlink_callback *cb);
extern int inet6_rtm_newroute(struct sk_buff *skb, struct nlmsghdr* nlh, void *arg);
extern int inet6_rtm_delroute(struct sk_buff *skb, struct nlmsghdr* nlh, void *arg);
extern int inet6_rtm_getroute(struct sk_buff *skb, struct nlmsghdr* nlh, void *arg);

extern void rt6_ifdown(struct net_device *dev);
extern void rt6_mtu_change(struct net_device *dev, unsigned mtu);

/*
 *	Store a destination cache entry in a socket
 */
static inline void ip6_dst_store(struct sock *sk, struct dst_entry *dst,
				 struct in6_addr *daddr,
				 struct in6_addr *saddr)
{
	struct ipv6_pinfo *np = inet6_sk(sk);
	struct rt6_info *rt = (struct rt6_info *) dst;

	write_lock(&sk->sk_dst_lock);
	__sk_dst_set(sk, dst);
	np->daddr_cache = daddr;
#if CONFIG_IPV6_SUBTREES
	np->saddr_cache = saddr;
#endif
	np->dst_cookie = rt->rt6i_node ? rt->rt6i_node->fn_sernum : 0;
	write_unlock(&sk->sk_dst_lock);
}

static inline int ipv6_unicast_destination(struct sk_buff *skb)
{
	struct rt6_info *rt = (struct rt6_info *) skb->dst;

	return rt->rt6i_flags & RTF_LOCAL;
}

#ifdef CONFIG_IPV6_MULTIPLE_TABLES

extern rwlock_t rt6_table_lock;

static inline struct rt6_table *rt6_hold_table(struct rt6_table *table)
{
	BUG_TRAP(table != NULL);
	atomic_inc(&table->refcnt);
	return table;
}

extern struct rt6_table *rt6_get_table(unsigned char id);
extern void rt6_put_table(struct rt6_table *table);
extern struct rt6_table *__rt6_new_table(unsigned char id);
extern struct rt6_table *rt6_new_table(unsigned char id);

/**
 * To do: allow the user to define which routing table will be used for
 * storing routes generated from router advertisements.
 **/

static inline unsigned char rt6_dev_dflt_table_id(int ifindex)
{
	return RT6_TABLE_MAIN;
}

/**
 * To do: allow the user to define which routing table will be used for
 * storing default multicast routes.
 **/

static inline unsigned char rt6_dev_dflt_mcast_table_id(int ifindex)
{
	return RT6_TABLE_MAIN;
}

#else

static inline struct rt6_table *rt6_hold_table(struct rt6_table *table)
{
	return table;
}

static inline struct rt6_table *rt6_get_table(unsigned char id)
{
	return &ip6_routing_table;
}

#define rt6_put_table(x) do {} while(0)

static inline struct rt6_table *rt6_new_table(unsigned char id)
{
	return rt6_get_table(id);
}

static inline unsigned char rt6_dev_dflt_table_id(int ifindex)
{
	return RT6_TABLE_MAIN;
}

static inline unsigned char rt6_dev_dflt_mcast_table_id(int ifindex)
{
	return RT6_TABLE_MAIN;
}

#endif /*CONFIG_IPV6_MULTIPLE_TABLES */

#endif
#endif
