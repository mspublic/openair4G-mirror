/* $Id: inet6_rth_space.c,v 1.1.1.1 2008/04/23 13:21:08 nguyenhn Exp $ */

/* This is a substitute for a missing inet6_rth_space(). */

#include <netinet/in.h>

#ifndef IPV6_RTHDR_TYPE_2
#define IPV6_RTHDR_TYPE_2 2
#endif

socklen_t inet6_rth_space(int type, int segments)
{
	if (type == IPV6_RTHDR_TYPE_0) {
		if (segments > 128)
			return 0;
		return 8 + segments * sizeof(struct in6_addr);
	} else if (type == IPV6_RTHDR_TYPE_2) {
		if (segments != 1)
			return 0;
		return 8 + sizeof(struct in6_addr);
	}
	return 0;
}
