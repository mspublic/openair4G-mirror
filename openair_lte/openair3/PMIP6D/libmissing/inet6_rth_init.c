/* $Id: inet6_rth_init.c,v 1.1.1.1 2008/04/23 13:21:08 nguyenhn Exp $ */

/* This is a substitute for a missing inet6_rth_init(). */

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip6.h>

#ifndef IPV6_RTHDR_TYPE_2
#define IPV6_RTHDR_TYPE_2 2
#endif

void *inet6_rth_init(void *bp, socklen_t bplen, int type,
		     int segments)
{
	struct ip6_rthdr *rth = (struct ip6_rthdr *)bp;
	uint8_t type_len = 0;

	if (type == IPV6_RTHDR_TYPE_0) {
		type_len = 8;
		*(uint32_t *)(rth+1) = 0;
	} else if (type == IPV6_RTHDR_TYPE_2) {
		type_len = 8;
		*(uint32_t *)(rth+1) = 0;
	} else
		return NULL;

	if (bplen < type_len + segments * sizeof(struct in6_addr))
		return NULL;

	rth->ip6r_nxt = 0;
	rth->ip6r_len = segments << 1;
	rth->ip6r_type = type;
	rth->ip6r_segleft = 0;
	
	return bp;
}
