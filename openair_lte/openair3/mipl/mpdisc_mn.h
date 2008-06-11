/* $Id: mpdisc_mn.h,v 1.1.1.1 2008/04/23 13:21:04 nguyenhn Exp $ */

#ifndef __MPDISC_MN_H__
#define __MPDISC_MN_H__ 1

#include <netinet/icmp6.h>

#ifdef ENABLE_VT
int mpd_poll_mps(const struct in6_addr *hoa,
		 const struct in6_addr *ha,
		 struct timespec *delay,
		 struct timespec *lastsent);
#endif

int mpd_schedule_first_mps(const struct in6_addr *hoa,
			   const struct in6_addr *ha,
			   const struct timespec *preferred_time);

static inline int mpd_trigger_mps(const struct in6_addr *hoa, 
				  const struct in6_addr *ha)
{
	return mpd_schedule_first_mps(hoa, ha, &INITIAL_SOLICIT_TIMER_TS);
}

void mpd_cancel_mps(const struct in6_addr *hoa, 
		    const struct in6_addr *ha);

int mpd_mn_init(void);
void mpd_mn_cleanup(void);

#endif
