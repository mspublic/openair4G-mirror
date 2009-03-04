/*****************************************************************
 * C header: pmip_msgc.c
 * Description: creates new options and sends and parses PBU/PBA.
 * Author: 
 *   Christian Bonnet
 *   Huu-Nghia Nguyen
 *   Hussain & Daniel
 * Copyright: Eurecom Institute, (C) 2008
 ******************************************************************/

#include "conf.h" 
#include <netinet/icmp6.h>
#include <errno.h>
#include <netinet/ip6mh.h>
#include "mh.h"
#include "util.h"
#include "debug.h"

#include "pmip_consts.h"
#include "pmip_types.h"
#include "pmip_cache.h"

struct sock {
	pthread_mutex_t send_mutex;
	int fd;
};
extern struct sock mh_sock;
/* We can use these safely, since they are only read and never change */
static const uint8_t _pad1[1] = { 0x00 };
static const uint8_t _pad2[2] = { 0x01, 0x00 };
static const uint8_t _pad3[3] = { 0x01, 0x01, 0x00 };
static const uint8_t _pad4[4] = { 0x01, 0x02, 0x00, 0x00 };
static const uint8_t _pad5[5] = { 0x01, 0x03, 0x00, 0x00, 0x00 };
static const uint8_t _pad6[6] = { 0x01, 0x04, 0x00, 0x00, 0x00, 0x00 };
static const uint8_t _pad7[7] = { 0x01, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00 };

static int create_opt_pad(struct iovec *iov, int pad)
{
	if (pad == 2)
		iov->iov_base = (void *)_pad2;
	else if (pad == 4)
		iov->iov_base = (void *)_pad4;
	else if (pad == 6)
		iov->iov_base = (void *)_pad6;
	/* Odd pads do not occur with current spec, so test them last */
	else if (pad == 1)
		iov->iov_base = (void *)_pad1;
	else if (pad == 3)
		iov->iov_base = (void *)_pad3;
	else if (pad == 5)
		iov->iov_base = (void *)_pad5;
	else if (pad == 7)
		iov->iov_base = (void *)_pad7;

	iov->iov_len = pad;

	return 0;
}

static inline int optpad(int xn, int y, int offset)
{
	return ((y - offset) & (xn - 1));
}

static int mh_try_pad(const struct iovec *in, struct iovec *out, int count)
{
	size_t len = 0;
	int m, n = 1, pad = 0;
	struct ip6_mh_opt *opt;

	out[0].iov_len = in[0].iov_len;
	out[0].iov_base = in[0].iov_base;
	len += in[0].iov_len;

	for (m = 1; m < count; m++) {
		opt = (struct ip6_mh_opt *)in[m].iov_base;
		switch (opt->ip6mhopt_type) {
		case IP6_MHOPT_BREFRESH:
			pad = optpad(2, 0, len); /* 2n */
			break;
		case IP6_MHOPT_ALTCOA:
			pad = optpad(8, 6, len); /* 8n+6 */
			break;
		case IP6_MHOPT_NONCEID:
			pad = optpad(2, 0, len); /* 2n */
			break;
		case IP6_MHOPT_BAUTH:
			pad = optpad(8, 2, len); /* 8n+2 */
			break;
		}
		if (pad > 0) {
			create_opt_pad(&out[n++], pad);
			len += pad;
		}
		len += in[m].iov_len;
		out[n].iov_len = in[m].iov_len;
		out[n].iov_base = in[m].iov_base;
		n++;
	}
	if (count == 1) {
		pad = optpad(8, 0, len);
		create_opt_pad(&out[n++], pad);
	}

	return n;
}

static size_t mh_length(struct iovec *vec, int count)
{
	size_t len = 0;
	int i;

	for (i = 0; i < count; i++) {
		len += vec[i].iov_len;
	}
	return len;
}

/**** DO NOT TOUCH THE ABOVE CODE ***************************/

struct in6_addr get_node_id (struct in6_addr * mn_addr)
{
	struct in6_addr result;
	result = in6addr_any;
	memcpy(&result.in6_u.u6_addr32[2], &mn_addr->in6_u.u6_addr32[2], sizeof(ip6mnid));
	return result;
}

struct in6_addr get_node_prefix (struct in6_addr * mn_addr)
{
	struct in6_addr result;
	result = in6addr_any;
	memcpy(&result.in6_u.u6_addr32[0], &mn_addr->in6_u.u6_addr32[0], PLEN/8);
	return result;
}

//===================================================================
//creates the Home Network Prefix.
int mh_create_opt_home_net_prefix(struct iovec *iov,struct in6_addr *Home_Network_Prefix)
//===================================================================
{
	struct ip6_mh_opt_home_net_prefix *opt;
	size_t optlen = sizeof(struct ip6_mh_opt_home_net_prefix);

	iov->iov_base = malloc(optlen);
	iov->iov_len = optlen;

	if (iov->iov_base == NULL)
		return -ENOMEM;

	opt = (struct ip6_mh_opt_home_net_prefix *)iov->iov_base;

	opt->ip6hnp_type = IP6_MHOPT_HOM_NET_PREX;
	opt->ip6hnp_len = 18;   //18 bytes
	opt->ip6hnp_reserved = 0;
	opt->ip6hnp_prefix_len = 128; //128 bits
	opt->ip6hnp_prefix = *Home_Network_Prefix;
	return 0;
}

//===================================================================
//creates the mobile interface identifier option
int mh_create_opt_mn_identifier (struct iovec *iov, int flags, ip6mnid * MN_ID)
//===================================================================
{
	struct ip6_mh_opt_mn_identifier *opt;
	size_t optlen = sizeof(struct ip6_mh_opt_mn_identifier);

	iov->iov_base = malloc(optlen);
	iov->iov_len = optlen;

	if (iov->iov_base == NULL)
		return -ENOMEM;

	opt = (struct ip6_mh_opt_mn_identifier *)iov->iov_base;

	opt->ip6mnid_type = IP6_MHOPT_MOB_IDENTIFIER;
	opt->ip6mnid_len = 10; //set to 10 bytes.
	opt->ip6mnid_flags = flags;
	opt->mn_identifier = *MN_ID; 	
	return 0;
}

//===================================================================
//creates the timestamp option
int mh_create_opt_time_stamp (struct iovec *iov, ip6ts * Timestamp )
//===================================================================
{
	struct ip6_mh_opt_time_stamp *opt;
	size_t optlen = sizeof(struct ip6_mh_opt_time_stamp );

	iov->iov_base = malloc(optlen);
	iov->iov_len = optlen;

	if (iov->iov_base == NULL)
		return -ENOMEM;

	opt = (struct ip6_mh_opt_time_stamp *)iov->iov_base;

	opt->ip6mots_type = IP6_MHOPT_TIME_STAMP;
	opt->ip6mots_len = 8; // set to 8 bytes.
	opt->time_stamp = *Timestamp;

	return 0;
}

//===================================================================
//creates the link local address option.
int mh_create_opt_link_local_add (struct iovec *iov, struct in6_addr *LinkLocal )
//===================================================================
{
	struct ip6_mh_link_local_add *opt;
	size_t optlen = sizeof(struct ip6_mh_link_local_add);

	iov->iov_base = malloc(optlen);
	iov->iov_len = optlen;

	if (iov->iov_base == NULL)
		return -ENOMEM;

	opt = (struct ip6_mh_link_local_add*)iov->iov_base;

	opt->ip6link_type = IP6_MHOPT_LINK_ADDR;
	opt->ip6link_len= 16; //set to 16 bytes
	opt->ip6link_addr = *LinkLocal;
	return 0;
}

//===================================================================
//creates the Source MN address option.
int mh_create_opt_dst_mn_addr(struct iovec *iov,struct in6_addr * dst_mn_addr)
//===================================================================
{
	struct ip6_mh_opt_dst_mn_addr *opt;
	size_t optlen = sizeof(struct ip6_mh_opt_dst_mn_addr);

	iov->iov_base = malloc(optlen);
	iov->iov_len = optlen;

	if (iov->iov_base == NULL)
		return -ENOMEM;

	opt = (struct ip6_mh_opt_dst_mn_addr *)iov->iov_base;
	opt->ip6dma_type = IP6_MHOPT_DST_MN_ADDR;
	opt->ip6dma_len = 16;   
	opt->dst_mn_addr = * dst_mn_addr;
	return 0;
}

//===================================================================
//creates the Serving MAG address option.
int mh_create_opt_serv_mag_addr(struct iovec *iov,struct in6_addr *Serv_MAG_addr)
//===================================================================
{
	struct ip6_mh_opt_serv_mag_addr *opt;
	size_t optlen = sizeof(struct ip6_mh_opt_serv_mag_addr);

	iov->iov_base = malloc(optlen);
	iov->iov_len = optlen;

	if (iov->iov_base == NULL)
		return -ENOMEM;

	opt = (struct ip6_mh_opt_serv_mag_addr *)iov->iov_base;

	opt->ip6sma_type = IP6_MHOPT_SERV_MAG_ADDR;
	opt->ip6sma_len = 16;   //16 bytes
	opt->serv_mag_addr = *Serv_MAG_addr;
	return 0;
}


//===================================================================
//creates the Serving LMA address option.
int mh_create_opt_serv_lma_addr(struct iovec *iov,struct in6_addr * serv_lma_addr)
//===================================================================
{
	struct ip6_mh_opt_serv_lma_addr *opt;
	size_t optlen = sizeof(struct ip6_mh_opt_serv_lma_addr);

	iov->iov_base = malloc(optlen);
	iov->iov_len = optlen;

	if (iov->iov_base == NULL)
		return -ENOMEM;

	opt = (struct ip6_mh_opt_serv_lma_addr *)iov->iov_base;

	opt->ip6sla_type = IP6_MHOPT_SERV_LMA_ADDR;
	opt->ip6sla_len = 16;   //16 bytes
	opt->serv_lma_addr = *serv_lma_addr;
	return 0;
}

//===================================================================
//creates the Source MN address option.
int mh_create_opt_src_mn_addr(struct iovec *iov,struct in6_addr * src_mn_addr)
//===================================================================
{
	struct ip6_mh_opt_src_mn_addr *opt;
	size_t optlen = sizeof(struct ip6_mh_opt_src_mn_addr);

	iov->iov_base = malloc(optlen);
	iov->iov_len = optlen;

	if (iov->iov_base == NULL)
		return -ENOMEM;

	opt = (struct ip6_mh_opt_src_mn_addr *)iov->iov_base;
	opt->ip6sma_type = IP6_MHOPT_SRC_MN_ADDR;
	opt->ip6sma_len = 16;   //16 bytes
	opt->src_mn_addr = * src_mn_addr;
	return 0;
}

//===================================================================
//creates the Source MAG address option.
int mh_create_opt_src_mag_addr(struct iovec *iov,struct in6_addr * src_mag_addr)
//===================================================================
{
	struct ip6_mh_opt_src_mag_addr *opt;
	size_t optlen = sizeof(struct ip6_mh_opt_src_mag_addr);

	iov->iov_base = malloc(optlen);
	iov->iov_len = optlen;

	if (iov->iov_base == NULL)
		return -ENOMEM;

	opt = (struct ip6_mh_opt_src_mag_addr *)iov->iov_base;
	opt->ip6sma_type = IP6_MHOPT_SRC_MAG_ADDR;
	opt->ip6sma_len = 16;   //16 bytes
	opt->src_mag_addr = * src_mag_addr;
	return 0;
}


//===================================================================
/** Parse PBU messages */
int mh_pbu_parse(struct msg_info * info, struct ip6_mh_binding_update *pbu, ssize_t len,const struct in6_addr_bundle *in_addrs,int iif)
//===================================================================
{
	static struct mh_options mh_opts; bzero(&mh_opts, sizeof(mh_opts));
	struct ip6_mh_opt_home_net_prefix * home_net_prefix;
	struct ip6_mh_opt_mn_identifier * mh_id_opt;
	struct ip6_mh_opt_time_stamp * time_stamp_opt;
	struct ip6_mh_link_local_add * link_local;
	//struct in6_addr peer_addr = in6addr_any;

	info->src = * in_addrs->src;
	info->dst = * in_addrs->dst;
	info->iif = iif;
	info->addrs.src = &info->src;
	info->addrs.dst = &info->dst;

	if (len < sizeof(struct ip6_mh_binding_update) ||
	    mh_opt_parse(&pbu->ip6mhbu_hdr, len, 
			 sizeof(struct ip6_mh_binding_update), &mh_opts) < 0)
		return 0;
	
	info->PBU_flags = ntohs(pbu->ip6mhbu_flags);
	info->lifetime.tv_sec =  (ntohs(pbu->ip6mhbu_lifetime) << 2);
	info->seqno = ntohs(pbu->ip6mhbu_seqno);
	dbg("Serving MAG Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&info->src));
	dbg("Our Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&info->dst));
	dbg("PBU Lifetime: %d (%d s)\n",pbu->ip6mhbu_lifetime, info->lifetime.tv_sec);
	dbg("PBU Sequence No: %d\n", info->seqno);
	
	home_net_prefix = mh_opt(&pbu->ip6mhbu_hdr, &mh_opts, IP6_MHOPT_HOM_NET_PREX);
	if (home_net_prefix)
	{
		//copy
		info->mn_prefix = home_net_prefix->ip6hnp_prefix;
		dbg("MN Home Network Prefix: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&info->mn_prefix));
	} 

	mh_id_opt = mh_opt(&pbu->ip6mhbu_hdr, &mh_opts, IP6_MHOPT_MOB_IDENTIFIER);
	if (mh_id_opt)
	{
		//copy
		info->mn_iid = in6addr_any;
		memcpy(&info->mn_iid.in6_u.u6_addr32[2],&mh_id_opt->mn_identifier,sizeof(ip6mnid));
		dbg("MN IID: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&info->mn_iid));
	}

	time_stamp_opt = mh_opt(&pbu->ip6mhbu_hdr, &mh_opts, IP6_MHOPT_TIME_STAMP);
	if (time_stamp_opt)
	{
		//copy
		info->timestamp = time_stamp_opt->time_stamp;
	}


	link_local = mh_opt(&pbu->ip6mhbu_hdr, &mh_opts, IP6_MHOPT_LINK_ADDR);
	if (link_local)
	{
		//copy
		info->mn_link_local_addr = link_local->ip6link_addr;
		dbg("MN Link Local Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&info->mn_link_local_addr));
	}

	info->msg_event = hasPBU;
	dbg("FSM Message Event: %d\n",info->msg_event);
	return 0;
}

//===================================================================
/** Parse PBA messages */
int mh_pba_parse(struct msg_info * info, struct ip6_mh_binding_ack *pba, ssize_t len,const struct in6_addr_bundle *in_addrs, int iif)
//===================================================================
{
	static struct mh_options mh_opts; bzero(&mh_opts, sizeof(mh_opts));
	struct ip6_mh_opt_home_net_prefix * home_net_prefix;
	struct ip6_mh_opt_mn_identifier * mh_id_opt;
	struct ip6_mh_opt_time_stamp * time_stamp_opt;
	struct ip6_mh_link_local_add * link_local;

	info->src = * in_addrs->src;
	info->dst = * in_addrs->dst;
	info->iif = iif;
	info->addrs.src = &info->src;
	info->addrs.dst = &info->dst;
	
	if (len < sizeof(struct ip6_mh_binding_ack) ||
	    mh_opt_parse(&pba->ip6mhba_hdr, len, 
			 sizeof(struct ip6_mh_binding_ack), &mh_opts) < 0)
				return 0;

	home_net_prefix = mh_opt(&pba->ip6mhba_hdr, &mh_opts, IP6_MHOPT_HOM_NET_PREX);
	if (home_net_prefix)
	{
		info->mn_prefix = home_net_prefix->ip6hnp_prefix;
		dbg("MN Home Network Prefix: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&info->mn_prefix));
	} 


	mh_id_opt = mh_opt(&pba->ip6mhba_hdr, &mh_opts, IP6_MHOPT_MOB_IDENTIFIER);
	if (mh_id_opt)
	{
		//copy
		info->mn_iid = in6addr_any;
		memcpy(&info->mn_iid.in6_u.u6_addr32[2],&mh_id_opt->mn_identifier,sizeof(ip6mnid));
		dbg("MN IID: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&info->mn_iid));
	}

	time_stamp_opt = mh_opt(&pba->ip6mhba_hdr, &mh_opts, IP6_MHOPT_TIME_STAMP);
	if (time_stamp_opt)
	{
		//copy
		memcpy(&info->timestamp,&time_stamp_opt->time_stamp,sizeof(ip6mnid));
	}


	link_local = mh_opt(&pba->ip6mhba_hdr, &mh_opts, IP6_MHOPT_LINK_ADDR);
	if (link_local)
	{
		//copy
		info->mn_link_local_addr = link_local->ip6link_addr;
		dbg("MN Link Local Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&info->mn_link_local_addr));
	}

	info->seqno = ntohs(pba->ip6mhba_seqno);
	info->PBA_flags = ntohs(pba->ip6mhba_flags);
	info->lifetime.tv_sec  = ntohs(pba->ip6mhba_lifetime) << 2;
	dbg("Proxy Binding Ack Lifetime: %d (%d s)\n",pba->ip6mhba_lifetime, info->lifetime.tv_sec);

	info->msg_event = hasPBA;
	dbg("FSM Message Event: %d\n",info->msg_event);
	return 0;	
}

//===================================================================
/** Parse PBREQ messages */
int mh_pbreq_parse(struct msg_info * info, struct ip6_mh_proxy_binding_request *pbr, ssize_t len,const struct in6_addr_bundle *in_addrs,int iif)
//===================================================================
{	
	struct mh_options mh_opts; 
	struct ip6_mh_opt_dst_mn_addr  *dst_mn_addr_opt;
	struct ip6_mh_opt_mn_identifier  *mh_id_opt;
	struct ip6_mh_opt_src_mag_addr  *mh_src_mag_opt;
	struct ip6_mh_opt_src_mn_addr  *mh_src_mn_opt;
	bzero(&mh_opts, sizeof(mh_opts));

	info->src = * in_addrs->src;
	info->dst = * in_addrs->dst;
	info->iif = iif;
	info->addrs.src = &info->src;
	info->addrs.dst = &info->dst;

	if (len < sizeof(struct ip6_mh_proxy_binding_request) ||
	    mh_opt_parse(&pbr->ip6mhpbreq_hdr, len, 
			 sizeof(struct ip6_mh_proxy_binding_request), &mh_opts) < 0)
		return 0;

		 
	dst_mn_addr_opt = mh_opt(&pbr->ip6mhpbreq_hdr, &mh_opts, IP6_MHOPT_DST_MN_ADDR);
	if (dst_mn_addr_opt)
	{
		info->mn_addr = dst_mn_addr_opt->dst_mn_addr;
		dbg("Dst MN Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&info->mn_addr));
	}

	mh_id_opt = mh_opt(&pbr->ip6mhpbreq_hdr, &mh_opts, IP6_MHOPT_MOB_IDENTIFIER);
	if (mh_id_opt)
	{
		//copy
		info->mn_iid = in6addr_any;
		memcpy(&info->mn_iid.in6_u.u6_addr32[2],&mh_id_opt->mn_identifier,sizeof(ip6mnid));
		dbg("Dst MNIID: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&info->mn_iid));
	}

	mh_src_mag_opt = mh_opt(&pbr->ip6mhpbreq_hdr, &mh_opts, IP6_MHOPT_SRC_MAG_ADDR);
	if (mh_src_mag_opt)
	{
		//copy
		info->src_mag_addr = mh_src_mag_opt->src_mag_addr;
		dbg("Src Mag Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&info->src_mag_addr));
	}
	
	mh_src_mn_opt = mh_opt(&pbr->ip6mhpbreq_hdr, &mh_opts, IP6_MHOPT_SRC_MN_ADDR);
	if (mh_src_mn_opt)
	{
		//copy
		info->src_mn_addr = mh_src_mn_opt->src_mn_addr;
		dbg("Src MN Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&info->src_mn_addr));
	}

	info->seqno = ntohs(pbr->ip6mhpbreq_seqno);
	info->PBREQ_action = ntohs(pbr->ip6mhpbreq_action);
	dbg("Sequence# = %d\n",info->seqno);
	dbg("Action = %d\n",info->PBREQ_action);
	info->msg_event = hasPBREQ;
	return 0;
}

//===================================================================
/** Parse PBRES messages */
int mh_pbres_parse(struct msg_info * info, struct ip6_mh_proxy_binding_response *pbre, ssize_t len,const struct in6_addr_bundle *in_addrs,int iif)
//===================================================================
{
	struct mh_options mh_opts;
	bzero(&mh_opts, sizeof(mh_opts));
	struct ip6_mh_opt_src_mn_addr  *mh_src_mn_opt;		
	struct ip6_mh_opt_dst_mn_addr * dst_mn_addr_opt;
	struct ip6_mh_opt_mn_identifier * mh_id_opt;
	struct ip6_mh_opt_serv_mag_addr * serv_mag_addr_opt;
	struct ip6_mh_opt_serv_lma_addr * serv_lma_addr_opt;

	if (len < sizeof(struct ip6_mh_proxy_binding_response) ||
	    mh_opt_parse(&pbre->ip6mhpbres_hdr, len, 
			 sizeof(struct ip6_mh_proxy_binding_response), &mh_opts) < 0)
		return 0;	
 
	mh_src_mn_opt = mh_opt(&pbre->ip6mhpbres_hdr, &mh_opts, IP6_MHOPT_SRC_MN_ADDR);
	if (mh_src_mn_opt)
	{
		//copy
		info->src_mn_addr = mh_src_mn_opt->src_mn_addr;
		dbg("Src MN Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&info->src_mn_addr));
	}

	dst_mn_addr_opt = mh_opt(&pbre->ip6mhpbres_hdr, &mh_opts, IP6_MHOPT_DST_MN_ADDR);
	if (dst_mn_addr_opt)
	{
		//copy
		info->mn_addr = dst_mn_addr_opt->dst_mn_addr;
		dbg("Dst MN Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&info->mn_addr));
	} 

	mh_id_opt = mh_opt(&pbre->ip6mhpbres_hdr, &mh_opts, IP6_MHOPT_MOB_IDENTIFIER);
	if (mh_id_opt)
	{
		//copy	
		info->mn_iid = in6addr_any;
		memcpy(&info->mn_iid.in6_u.u6_addr32[2],&mh_id_opt->mn_identifier,sizeof(ip6mnid));
		dbg("MN IID: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&info->mn_iid));
	}

	serv_mag_addr_opt = mh_opt(&pbre->ip6mhpbres_hdr, &mh_opts, IP6_MHOPT_SERV_MAG_ADDR);
	if (serv_mag_addr_opt)
	{
		info->mn_serv_mag_addr = serv_mag_addr_opt->serv_mag_addr;
		dbg("MN Serving MAG Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&info->mn_serv_mag_addr));
	}

	serv_lma_addr_opt = mh_opt(&pbre->ip6mhpbres_hdr, &mh_opts, IP6_MHOPT_SERV_LMA_ADDR);
	if (serv_lma_addr_opt)
	{
		info->mn_serv_lma_addr = serv_lma_addr_opt->serv_lma_addr;
		dbg("MN Serving LMA Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&info->mn_serv_lma_addr));
	}

	info->seqno = ntohs(pbre->ip6mhpbres_seqno);
	info->PBRES_status = ntohs(pbre->ip6mhpbres_status);
	dbg("Sequence# = %d\n",info->seqno);
	info->msg_event = hasPBRES;
	return 0;
}

/** Parse ICMPv6 NS messages */
int icmp_ns_parse(struct msg_info * info, struct nd_neighbor_solicit * ns, const struct in6_addr *saddr, const struct in6_addr *daddr, int iif, int hoplimit)
//===================================================================
{
	bzero(info, sizeof(struct msg_info));		
	info->ns_target = ns->nd_ns_target;
	info->hoplimit = hoplimit;
	info->msg_event = hasNS;

	info->src = *saddr;
	info->dst = *daddr;
	info->iif = iif;	
	info->addrs.src = &info->src;
	info->addrs.dst = &info->dst;

	//Calculated fields
	info->mn_iid = get_node_id(&info->ns_target);
	info->mn_addr = info->ns_target;
	info->mn_prefix = get_node_prefix(&info->ns_target);
	dbg("NS Target: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&ns->nd_ns_target));
	return 0;
}


/** Parse ICMPv6 NA messages */
int icmp_na_parse(struct msg_info * info, struct  nd_neighbor_advert * na, const struct in6_addr *saddr, const struct in6_addr *daddr, int iif, int hoplimit)
//===================================================================
{
	bzero(info, sizeof(struct msg_info));		
	info->na_target = na->nd_na_target;
	info->hoplimit = hoplimit;
	info->msg_event = hasNA;

	info->src = *saddr;
	info->dst = *daddr;
	info->iif = iif;
	info->addrs.src = &info->src;
	info->addrs.dst = &info->dst;
	
	//Calculated fields
	info->mn_iid = get_node_id(&info->na_target);
	info->mn_addr = info->na_target;
	info->mn_prefix = get_node_prefix(&info->na_target);
	dbg("NA Target: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&na->nd_na_target));
	return 0;
}


//===================================================================
int pmip_mh_send(const struct in6_addr_bundle *addrs, const struct iovec *mh_vec, int iovlen, int oif)
//===================================================================
{

//	struct ip6_mh_opt_auth_data lbad;
	struct sockaddr_in6 daddr;
	struct iovec iov[2*(IP6_MHOPT_MAX+1)];
	struct msghdr msg;
	struct cmsghdr *cmsg;

//	struct cmsghdr controlmsg;
	int cmsglen;
	struct in6_pktinfo pinfo;
	int ret = 0, on = 1;
	struct ip6_mh *mh;
	int iov_count;

//	socklen_t rthlen = 0;
	iov_count = mh_try_pad(mh_vec, iov, iovlen);
	mh = (struct ip6_mh *)iov[0].iov_base;
	mh->ip6mh_hdrlen = (mh_length(iov, iov_count) >> 3) - 1;
	dbg("Sending MH type %d\n"
	     "from %x:%x:%x:%x:%x:%x:%x:%x\n"
	     "to %x:%x:%x:%x:%x:%x:%x:%x\n",
	     mh->ip6mh_type, NIP6ADDR(addrs->src), NIP6ADDR(addrs->dst));
	memset(&daddr, 0, sizeof(struct sockaddr_in6));
	daddr.sin6_family = AF_INET6;
	daddr.sin6_addr = *addrs->dst;
	daddr.sin6_port = htons(IPPROTO_MH);
	memset(&pinfo, 0, sizeof(pinfo));
	pinfo.ipi6_addr = *addrs->src;
	pinfo.ipi6_ifindex = oif;
	cmsglen = CMSG_SPACE(sizeof(pinfo));
	cmsg = malloc(cmsglen);
	if (cmsg == NULL) {
		dbg("malloc failed\n");
		return -ENOMEM;
	}
	memset(cmsg, 0, cmsglen);
	memset(&msg, 0, sizeof(msg));
	msg.msg_control = cmsg;
	msg.msg_controllen = cmsglen;
	msg.msg_iov = iov;
	msg.msg_iovlen = iov_count;
	msg.msg_name = (void *)&daddr;
	msg.msg_namelen = sizeof(daddr);
	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_len = CMSG_LEN(sizeof(pinfo));
	cmsg->cmsg_level = IPPROTO_IPV6;
	cmsg->cmsg_type = IPV6_PKTINFO;
	memcpy(CMSG_DATA(cmsg), &pinfo, sizeof(pinfo));

	pthread_mutex_lock(&mh_sock.send_mutex);
	setsockopt(mh_sock.fd, IPPROTO_IPV6, IPV6_PKTINFO, &on, sizeof(int));
	ret = sendmsg(mh_sock.fd, &msg, 0);

	if (ret < 0)
		dbg("sendmsg: %s\n", strerror(errno));
	pthread_mutex_unlock(&mh_sock.send_mutex);

	free(msg.msg_control);
	dbg("MH is sent....\n");
	return ret;
}

//===================================================================
/** Send PBU message */
int mh_send_pbu(const struct in6_addr_bundle *addrs,struct pmip_entry *bce,  struct timespec *lifetime, int oif)
//===================================================================
{
	int iovlen = 1;
	struct ip6_mh_binding_update *pbu;
	static struct iovec mh_vec[7];
	bzero(mh_vec, sizeof(mh_vec));
	pbu = mh_create(&mh_vec[0], IP6_MH_TYPE_BU);

	if (!pbu)
	{
		dbg("mh_create() failed\n");
		return -ENOMEM;
	}

	pbu->ip6mhbu_seqno = htons(bce->seqno_out);
	pbu->ip6mhbu_flags = htons(bce->PBU_flags);
	pbu->ip6mhbu_lifetime = htons(lifetime->tv_sec >> 2);
	dbg("Create PBU with lifetime = %d (%d s)\n",pbu->ip6mhbu_lifetime, conf.PBU_LifeTime);

	ip6mnid mn_id;
	memcpy(&mn_id, &bce->mn_iid.in6_u.u6_addr32[2], sizeof(ip6mnid));

/**
	* create the options in this order.
	* home network prefix.
	* mobile Identifier.
	* padN with len=0
	* timestamp.
	* padN with len =4.
	* link local address.
*/
	dbg("Create PBU options...\n");

	mh_create_opt_home_net_prefix(&mh_vec[iovlen++], &bce->mn_prefix);
	uint16_t p_flag = htons(IP6_MH_MNID);
	mh_create_opt_mn_identifier (&mh_vec[iovlen++], p_flag, &mn_id);
	//create_opt_pad(&mh_vec[iovlen++], 2); //2 byte PadN option header + 0 bytes
	mh_create_opt_time_stamp (&mh_vec[iovlen++], &bce->timestamp );
	//create_opt_pad(&mh_vec[iovlen++], 6); //2 byte PadN option header + 4 bytes
	mh_create_opt_link_local_add (&mh_vec[iovlen++],&bce->mn_link_local_addr);

	//calculate the length of the message.
	pbu->ip6mhbu_hdr.ip6mh_hdrlen = mh_length(mh_vec,iovlen);

	dbg("Send PBU....\n");
	pmip_mh_send(addrs,mh_vec, iovlen, oif);
	
	dbg("Copy PBU message into TEMP PMIP entry iovec....\n");
	//copy the PBU message into the mh_vector for the entry for future retransmissions.
	memcpy(bce->mh_vec,mh_vec, sizeof(mh_vec));
	bce->iovlen = iovlen;

	//free_iov_data(mh_vec, iovlen); --> Don't free, keep for retransmission of PBU
	return 0;
}

//===================================================================}
/** Send PBA message */
int mh_send_pba(const struct in6_addr_bundle *addrs,struct pmip_entry *bce,  struct timespec *lifetime,  int oif)
//===================================================================
{
	int iovlen = 1;
	struct ip6_mh_binding_ack *pba;
	static struct iovec mh_vec[7];
	bzero(mh_vec, sizeof(mh_vec));

	pba = mh_create(&mh_vec[0], IP6_MH_TYPE_BACK);
	if (!pba) 
	{
		dbg("mh_create() failed\n");
		return -ENOMEM;
	}

	pba->ip6mhba_status = bce->status;
	pba->ip6mhba_flags = htons(bce->PBA_flags);//check since it is only one byte!!
	pba->ip6mhba_seqno = htons(bce->seqno_in);
	pba->ip6mhba_lifetime = htons(lifetime->tv_sec >> 2);

/**
	* create the options in this order.
	* home network prefix.
	* mobile Identifier.
	* padN with len=0
	* timestamp.
	* padN with len =4.
	* link local address.
*/
	dbg("Create PBA options....\n");
	mh_create_opt_home_net_prefix(&mh_vec[iovlen++],&bce->mn_prefix);
	uint16_t p_flag = htons(IP6_MH_MNID); ip6mnid mn_id; memcpy(&mn_id, &bce->mn_iid.in6_u.u6_addr32[2], sizeof(ip6mnid));
	mh_create_opt_mn_identifier (&mh_vec[iovlen++], p_flag, &mn_id);
	//create_opt_pad(&mh_vec[iovlen++], 2); //2 byte PadN option header + 0 bytes
	mh_create_opt_time_stamp (&mh_vec[iovlen++], &bce->timestamp );
	//create_opt_pad(&mh_vec[iovlen++], 6); //2 byte PadN option header + 4 bytes
	mh_create_opt_link_local_add (&mh_vec[iovlen++],&bce->mn_link_local_addr ); 

	//calculate the length of the message.
	pba->ip6mhba_hdr.ip6mh_hdrlen = mh_length(mh_vec,iovlen);

	dbg("Send PBA...\n");
	pmip_mh_send(addrs, mh_vec, iovlen, oif); 
	free_iov_data(mh_vec, iovlen);
	return 0;
}

//===================================================================
/** Send PBREQ message */
int mh_send_pbreq(
	struct in6_addr_bundle *addrs,
	struct in6_addr *dst_mn_iid,
	struct in6_addr *dst_mn_addr, 
	struct in6_addr *src_mag_addr, 
	struct in6_addr *src_mn_addr, 
	uint16_t seqno, 
	uint16_t action, 
	int link,
	struct pmip_entry * bce)
//===================================================================
{
	int iovlen = 1;
	struct ip6_mh_proxy_binding_request *pbreq;
	static struct iovec mh_vec[10];	
	bzero(mh_vec, sizeof(mh_vec));

	pbreq = mh_create(&mh_vec[0], IP6_MH_TYPE_PBREQ);
	if (!pbreq) 
	{
		dbg("mh_create() failed\n");
		return -ENOMEM;
	}
	
	pbreq->ip6mhpbreq_seqno = htons(seqno);
	pbreq->ip6mhpbreq_action = htons(action);
	ip6mnid MN_ID;
	memcpy(&MN_ID, &dst_mn_iid->in6_u.u6_addr32[2], sizeof(ip6mnid));
	
	/*
	* create the options in this order:
	* home network prefix.
	* mobile Identifier.
	*
	*/
	dbg("Create PBREQ options...\n");
	uint16_t p_flag = htons(IP6_MH_MNID);
	mh_create_opt_mn_identifier (&mh_vec[iovlen++], p_flag, &MN_ID);

	//Optional options	 
	if (dst_mn_addr)  mh_create_opt_dst_mn_addr(&mh_vec[iovlen++], dst_mn_addr);
	if (src_mag_addr) mh_create_opt_src_mag_addr (&mh_vec[iovlen++], src_mag_addr);
	if (src_mn_addr) mh_create_opt_src_mn_addr (&mh_vec[iovlen++], src_mn_addr);
		
	//calculate the length of the message.
	pbreq->ip6mhpbreq_hdr.ip6mh_hdrlen = mh_length((struct iovec *) &mh_vec,iovlen);

	dbg("send PBREQ....\n");
	pmip_mh_send(addrs, (struct iovec *) &mh_vec, iovlen, link);
	if(bce)
	{
		dbg("copy PBREQ message into PMIP entry iovec....\n");
		//copy the PBR message into the mh_vector for the entry for future retransmissions.
		memcpy(bce->mh_vec,mh_vec, sizeof(mh_vec));
		bce->iovlen = iovlen;
	}
	else free_iov_data(mh_vec, iovlen);
	return 0;
}

//===================================================================
/* Send PBRES messages */
int mh_send_pbres(struct in6_addr_bundle *addrs, struct in6_addr *src_mn_addr, struct in6_addr *dst_mn_addr, struct in6_addr *dst_mn_iid, struct in6_addr * serv_mag_addr, struct in6_addr *serv_lma_addr, uint16_t seqno, uint16_t status, int link)
//===================================================================
{
	int iovlen = 1;
	struct ip6_mh_proxy_binding_response *pbres;
	static struct iovec mh_vect[10];
	bzero(mh_vect, sizeof(mh_vect));
	pbres = mh_create(&mh_vect[0], IP6_MH_TYPE_PBRES);
	if (!pbres)
	{
		dbg("mh_create() failed\n");
		return -ENOMEM;
	}
	pbres->ip6mhpbres_seqno = htons(seqno);
	pbres->ip6mhpbres_status = htons(status);

	/*
	* create the options in this order.
	* home network prefix.
	* mobile Identifier.
	* Serving MAG Address.
	*/
	dbg("Create PBRES options...\n");
	uint16_t p_flag = htons(IP6_MH_MNID);
	ip6mnid MN_ID;
	memcpy(&MN_ID, &dst_mn_iid->in6_u.u6_addr32[2], sizeof(ip6mnid));	
	mh_create_opt_mn_identifier (&mh_vect[iovlen++], p_flag, &MN_ID);

	//Optional options
	if (src_mn_addr) mh_create_opt_src_mn_addr (&mh_vect[iovlen++], src_mn_addr);
	if (dst_mn_addr)  mh_create_opt_dst_mn_addr(&mh_vect[iovlen++], dst_mn_addr);
	if (serv_mag_addr) mh_create_opt_serv_mag_addr(&mh_vect[iovlen++],serv_mag_addr);
	if (serv_lma_addr) mh_create_opt_serv_lma_addr(&mh_vect[iovlen++],serv_lma_addr);
 

	//calculate the length of the message.
	pbres->ip6mhpbres_hdr.ip6mh_hdrlen = mh_length((struct iovec *) &mh_vect,iovlen);

	dbg("Send PBRES....\n");
	pmip_mh_send(addrs, (struct iovec *) &mh_vect, iovlen, link);
	free_iov_data(mh_vect, iovlen);
	return 0;	
}

