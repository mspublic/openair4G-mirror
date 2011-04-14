/* $Id: cn.h,v 1.1.1.1 2008/04/23 13:21:04 nguyenhn Exp $ */

#ifndef __CN_H__
#define __CN_H__ 1

/* How long before binding expiry do we send a BRR */
#define CN_BRR_BEFORE_EXPIRY 2
extern const struct timespec cn_brr_before_expiry_ts;
#define CN_BRR_BEFORE_EXPIRY_TS cn_brr_before_expiry_ts

struct ip6_mh;
struct in6_addr_bundle;

extern void cn_recv_bu(const struct ip6_mh *mh, ssize_t len,
		       const struct in6_addr_bundle *in, int iif);

extern void cn_init(void);
extern void cn_cleanup(void);

#endif
