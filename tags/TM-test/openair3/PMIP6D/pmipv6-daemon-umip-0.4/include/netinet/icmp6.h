#ifndef _NETINET__ICMP6_H
#define _NETINET__ICMP6_H 1

#include_next <netinet/icmp6.h>

#include <config.h>
/*
 *   ICMP message types and definitions for Mobile IPv6 are defined in
 *   <netinet/icmp6.h>
 */
#define MIP_HA_DISCOVERY_REQUEST	144
#define MIP_HA_DISCOVERY_REPLY		145
#define MIP_PREFIX_SOLICIT		146
#define MIP_PREFIX_ADVERT		147

/*
 *   The following data structures can be used for the ICMP message
 *   types discussed in section 6.5 through 6.8 in the base Mobile 
 *   IPv6 [2] specification.
 */
#ifndef HAVE_STRUCT_MIP_DHAAD_REQ
struct mip_dhaad_req {		/* Dynamic HA Address Discovery */
	struct icmp6_hdr mip_dhreq_hdr;
};

#define mip_dhreq_type			mip_dhreq_hdr.icmp6_type
#define mip_dhreq_code			mip_dhreq_hdr.icmp6_code
#define mip_dhreq_cksum			mip_dhreq_hdr.icmp6_cksum
#define mip_dhreq_id			mip_dhreq_hdr.icmp6_data16[0]
#define mip_dhreq_reserved		mip_dhreq_hdr.icmp6_data16[1]
#endif

#ifndef HAVE_STRUCT_MIP_DHAAD_REP
struct mip_dhaad_rep {		/* HA Address Discovery Reply */
	struct icmp6_hdr mip_dhrep_hdr;
	/* Followed by Home Agent IPv6 addresses */
};

#define mip_dhrep_type			mip_dhrep_hdr.icmp6_type
#define mip_dhrep_code			mip_dhrep_hdr.icmp6_code
#define mip_dhrep_cksum			mip_dhrep_hdr.icmp6_cksum
#define mip_dhrep_id			mip_dhrep_hdr.icmp6_data16[0]
#define mip_dhrep_reserved		mip_dhrep_hdr.icmp6_data16[1]
#endif

#ifndef HAVE_STRUCT_MIP_PREFIX_SOLICIT
struct mip_prefix_solicit {	/* Mobile Prefix Solicitation */
	struct icmp6_hdr mip_ps_hdr;
};

#define mip_ps_type		mip_ps_hdr.icmp6_type
#define mip_ps_code		mip_ps_hdr.icmp6_code
#define mip_ps_cksum		mip_ps_hdr.icmp6_cksum
#define mip_ps_id		mip_ps_hdr.icmp6_data16[0]
#define mip_ps_reserved		mip_ps_hdr.icmp6_data16[1]
#endif

#ifndef HAVE_STRUCT_MIP_PREFIX_ADVERT
struct mip_prefix_advert {	/* Mobile Prefix Adverisements */
	struct icmp6_hdr mip_pa_hdr;
	/* Followed by one or more PI options */
};

#define mip_pa_type		mip_pa_hdr.icmp6_type
#define mip_pa_code		mip_pa_hdr.icmp6_code
#define mip_pa_cksum		mip_pa_hdr.icmp6_cksum
#define mip_pa_id		mip_pa_hdr.icmp6_data16[0]
#define mip_pa_flags_reserved	mip_pa_hdr.icmp6_data16[1]

#if BYTE_ORDER == BIG_ENDIAN
#define MIP_PA_FLAG_MANAGED	0x8000
#define MIP_PA_FLAG_OTHER	0x4000
#else				/* BYTE_ORDER == LITTLE_ENDIAN */
#define MIP_PA_FLAG_MANAGED	0x0080
#define MIP_PA_FLAG_OTHER	0x0040
#endif

#endif

#define nd_opt_ai_type		nd_opt_adv_interval_type
#define nd_opt_ai_len		nd_opt_adv_interval_len
#define nd_opt_ai_reserved	nd_opt_adv_interval_reserved
#define nd_opt_ai_interval	nd_opt_adv_interval_ival

#define ND_OPT_ADV_INTERVAL	7	/* Adv Interval Option  */
#define ND_OPT_HA_INFORMATION	8	/* HA Information option */

#ifndef HAVE_STRUCT_ND_OPT_HOMEAGENT_INFO
struct nd_opt_homeagent_info {	/* Home Agent information */
	uint8_t		nd_opt_hai_type;
	uint8_t		nd_opt_hai_len;
	uint16_t	nd_opt_hai_reserved;
	uint16_t	nd_opt_hai_preference;
	uint16_t	nd_opt_hai_lifetime;
};
#endif

#endif	/* netinet/icmp6.h */
