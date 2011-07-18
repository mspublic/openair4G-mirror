/* $Id: inet6_rth_gettype.c,v 1.1.1.1 2008/04/23 13:21:08 nguyenhn Exp $ */

/* This is a substitute for a missing inet6_rth_getaddr(). */

#include <stdint.h>

int inet6_rth_gettype(void *bp)
{
	uint8_t *rthp = (uint8_t *)bp;

	return rthp[2];
}
