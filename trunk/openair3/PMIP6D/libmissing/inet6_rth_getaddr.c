/* $Id: inet6_rth_getaddr.c,v 1.1.1.1 2008/04/23 13:21:08 nguyenhn Exp $ */

/* This is a substitute for a missing inet6_rth_getaddr(). */

#include <netinet/in.h>

struct in6_addr *inet6_rth_getaddr(const void *bp, int index)
{
	uint8_t *rthp = (uint8_t *)bp;
	struct in6_addr *addr = NULL;

	if (rthp[1] & 1) return NULL;
	if (index < 0 || index > rthp[3]) return NULL;

	addr = (struct in6_addr *)
		(rthp + 8 + index * sizeof(struct in6_addr));

	return addr;
}
