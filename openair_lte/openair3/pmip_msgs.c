/*****************************************************************
 * C header: pmip_msgc.c
 * Description: creates new options and sends and parses PBU/PBA.
 * Author: 
 *   Christian Bonnet
 *   Huu-Nghia Nguyen
 *   Hussain & Daniel
 * Copyright: Eurecom Institute, (C) 2008
 ******************************************************************/
#include "pmip_types.h"
#include <netinet/icmp6.h>
#include <errno.h>
#include <netinet/ip6mh.h>
#include "mh.h"
#include "util.h"
#include "debug.h"
#include "pmip_cache.h"
#include "pmip_consts.h"
#include "conf.h" 

//#include <net/mip6.h>


//--------------------------------------------------------------
//mh.c
//--------------------------------------------------------------
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
//**** DO NOT TOUCH THE ABOVE CODE ***************************
//*****************************************************************


//creates the Home Network Prefix.
int mh_create_opt_home_net_prefix(struct iovec *iov,struct in6_addr *Home_Network_Prefix)
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
	memcpy( &(opt->ip6hnp_prefix), Home_Network_Prefix, sizeof(struct in6_addr));
	return 0;
}

//creates the mobile interface identifier option

int mh_create_opt_mn_identifier (struct iovec *iov, int flags, __identifier * MN_ID)
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
	memcpy( &(opt->mn_identifier), MN_ID, sizeof(__identifier)); 	

	return 0;
}

//creates the timestamp option

int mh_create_opt_time_stamp (struct iovec *iov, __timestamp * Timestamp )
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
	memcpy( &(opt->time_stamp), Timestamp, sizeof(__timestamp));

	return 0;
}

//creates the link local address option.
int mh_create_opt_link_local_add (struct iovec *iov, struct in6_addr *LinkLocal )
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
	memcpy( &(opt->ip6link_addr), LinkLocal, sizeof(struct in6_addr));
	return 0;
}

//creates the Serving MAG address option.
int mh_create_opt_serv_mag_addr(struct iovec *iov,struct in6_addr *Serv_MAG_addr)
{
	struct ip6_mh_opt_serv_mag_addr *opt;
	size_t optlen = sizeof(struct ip6_mh_opt_serv_mag_addr);

	iov->iov_base = malloc(optlen);
	iov->iov_len = optlen;

	if (iov->iov_base == NULL)
		return -ENOMEM;

	opt = (struct ip6_mh_opt_serv_mag_addr *)iov->iov_base;

	opt->ip6sma_type = IP6_MHOPT_SERV_MAG_ADDR;
	opt->ip6sma_len = 18;   //18 bytes
	opt->ip6sma_reserved = 0;
	memcpy( &(opt->serv_mag_addr), Serv_MAG_addr, sizeof(struct in6_addr));
	return 0;
}

void mh_send_pbu(const struct in6_addr_bundle *addrs,struct pmip_entry *bce, int oif)
{
	int iovlen = 1;
	struct ip6_mh_binding_update *pbu;
	static struct iovec mh_vec[7];
	bzero(mh_vec, sizeof(mh_vec));
	pbu = mh_create(&mh_vec[0], IP6_MH_TYPE_BU);

	if (!pbu)
		return;
	pbu->ip6mhbu_seqno = htons(bce->seqno);
	pbu->ip6mhbu_flags = htons(bce->PBU_flags);
	pbu->ip6mhbu_lifetime = htons(bce->lifetime.tv_sec >> 2);
	dbg("PBU lifetime = %d (%d s)\n",pbu->ip6mhbu_lifetime, bce->lifetime.tv_sec);

	__identifier MN_ID;
	memcpy(&MN_ID, &bce->peer_addr.in6_u.u6_addr32[2], sizeof(__identifier));

/**
	* create the options in this order.
	* home network prefix.
	* mobile Identifier.
	* padN with len=0
	* timestamp.
	* padN with len =4.
	* link local address.
*/
	dbg("PBU options are created....\n");

	mh_create_opt_home_net_prefix(&mh_vec[iovlen++], &bce->peer_prefix);

	uint16_t p_flag = htons(IP6_MH_MNID);
	mh_create_opt_mn_identifier (&mh_vec[iovlen++], p_flag, &MN_ID);

	//create_opt_pad(&mh_vec[iovlen++], 2); //2 byte PadN option header + 0 bytes
	
	mh_create_opt_time_stamp (&mh_vec[iovlen++], &bce->Timestamp );

	//create_opt_pad(&mh_vec[iovlen++], 6); //2 byte PadN option header + 4 bytes

	mh_create_opt_link_local_add (&mh_vec[iovlen++],&bce->LinkLocal);


	//TODO calculate the length of the message.
	pbu->ip6mhbu_hdr.ip6mh_hdrlen = mh_length(mh_vec,iovlen);

	dbg("send PBU....\n");
	pmip_mh_send(addrs,mh_vec, iovlen, oif);
	
	dbg("copy PBU message into TEMP PMIP entry iovec....\n");
	//copy the PBU message into the mh_vector for the entry for future retransmissions.
	memcpy(bce->mh_vec,mh_vec, sizeof(mh_vec));
	bce->iovlen = iovlen;

	//free_iov_data(mh_vec, iovlen);
	dbg("PBU is sent....\n");
}




void mh_send_pba(const struct in6_addr_bundle *addrs,struct pmip_entry *bce, int oif)
{
	int iovlen = 1;
	struct ip6_mh_binding_ack *pba;
	static struct iovec mh_vec[7];

	bzero(mh_vec, sizeof(mh_vec));

	dbg("status %d\n", bce->status);

	pba = mh_create(&mh_vec[0], IP6_MH_TYPE_BACK);
	if (!pba)
		return;

	pba->ip6mhba_status = bce->status;
	pba->ip6mhba_flags = htons(bce->PBA_flags);//check since it is only one byte!!
	pba->ip6mhba_seqno = htons(bce->seqno);
	pba->ip6mhba_lifetime = htons(bce->lifetime.tv_sec >> 2);

	__identifier MN_ID;
	memcpy(&MN_ID, &bce->peer_addr.in6_u.u6_addr32[2], sizeof(__identifier));

/**
	* create the options in this order.
	* home network prefix.
	* mobile Identifier.
	* padN with len=0
	* timestamp.
	* padN with len =4.
	* link local address.
*/
	dbg("PBA options are created....\n");
	mh_create_opt_home_net_prefix(&mh_vec[iovlen++],&bce->peer_prefix);

	uint16_t p_flag = htons(IP6_MH_MNID);
	mh_create_opt_mn_identifier (&mh_vec[iovlen++], p_flag, &MN_ID);

	//create_opt_pad(&mh_vec[iovlen++], 2); //2 byte PadN option header + 0 bytes
	
	mh_create_opt_time_stamp (&mh_vec[iovlen++], &bce->Timestamp );

	//create_opt_pad(&mh_vec[iovlen++], 6); //2 byte PadN option header + 4 bytes

	mh_create_opt_link_local_add (&mh_vec[iovlen++],&bce->LinkLocal ); 

	//TODO calculate the length of the message.
	pba->ip6mhba_hdr.ip6mh_hdrlen = mh_length(mh_vec,iovlen);

	pmip_mh_send(addrs, mh_vec, iovlen, oif); 
	free_iov_data(mh_vec, iovlen);
	dbg("PBA is sent....\n");
}

struct pmip_entry *mh_pbu_parse(struct ip6_mh_binding_update *pbu, ssize_t len,const struct in6_addr_bundle *in_addrs,int iif)
{
	static struct mh_options mh_opts; 
	bzero(&mh_opts, sizeof(mh_opts));
	struct ip6_mh_opt_home_net_prefix * home_net_prefix;
	struct ip6_mh_opt_mn_identifier * mh_id_opt;
	struct ip6_mh_opt_time_stamp * time_stamp_opt;
	struct ip6_mh_link_local_add * link_local;
	//struct in6_addr peer_addr = in6addr_any;

	dbg("Parse PBU message....\n");

	if (len < sizeof(struct ip6_mh_binding_update) ||
	    mh_opt_parse(&pbu->ip6mhbu_hdr, len, 
			 sizeof(struct ip6_mh_binding_update), &mh_opts) < 0)
		return -1;

	static struct pmip_entry msg;
	bzero(&msg,sizeof(msg));

	dbg("Received PBU message for a MN...\n");

	memcpy(&msg.Serv_MAG_addr,in_addrs->src,sizeof(struct in6_addr));
	dbg("SERVING MAG ADDR: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&msg.Serv_MAG_addr));
	memcpy(&msg.our_addr,in_addrs->dst,sizeof(struct in6_addr));
	dbg("OUR Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&msg.our_addr));
	memcpy(&msg.LMA_addr,in_addrs->dst,sizeof(struct in6_addr));

	msg.PBU_flags = ntohs(pbu->ip6mhbu_flags);

	msg.lifetime.tv_sec =  (ntohs(pbu->ip6mhbu_lifetime) << 2);

	dbg("Proxy Binding Update Lifetime: %d (%d s)\n",pbu->ip6mhbu_lifetime, msg.lifetime.tv_sec);

	msg.seqno = ntohs(pbu->ip6mhbu_seqno);
	
	msg.FLAGS = hasPBU;
	dbg("FSM Flags: %d\n",msg.FLAGS);
		
	home_net_prefix = mh_opt(&pbu->ip6mhbu_hdr, &mh_opts, IP6_MHOPT_HOM_NET_PREX);
	if (home_net_prefix)
	{
		//copy
		msg.peer_prefix = home_net_prefix->ip6hnp_prefix;
	} 

	mh_id_opt = mh_opt(&pbu->ip6mhbu_hdr, &mh_opts, IP6_MHOPT_MOB_IDENTIFIER);
	if (mh_id_opt)
	{
		//copy
		memcpy(&msg.peer_addr.in6_u.u6_addr32[2],&mh_id_opt->mn_identifier,sizeof(__identifier));
		dbg("Peer Address: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&msg.peer_addr));

	}

	time_stamp_opt = mh_opt(&pbu->ip6mhbu_hdr, &mh_opts, IP6_MHOPT_TIME_STAMP);
	if (time_stamp_opt)
	{
		//copy
		msg.Timestamp = time_stamp_opt->time_stamp;
	}


	link_local = mh_opt(&pbu->ip6mhbu_hdr, &mh_opts, IP6_MHOPT_LINK_ADDR);
	if (link_local)
	{
		//copy
		msg.LinkLocal = link_local->ip6link_addr;
	}


	return &msg ;
}




struct pmip_entry * mh_pba_parse(struct ip6_mh_binding_ack *pba, ssize_t len,const struct in6_addr_bundle *in_addrs, int iif)
{
	
	
	static struct mh_options mh_opts; 
	bzero(&mh_opts, sizeof(mh_opts));

	struct ip6_mh_opt_home_net_prefix * home_net_prefix;
	struct ip6_mh_opt_mn_identifier * mh_id_opt;
	struct ip6_mh_opt_time_stamp * time_stamp_opt;
	struct ip6_mh_link_local_add * link_local;
	
	
	struct in6_addr peer_prefix= in6addr_any, peer_addr = in6addr_any ,LinkLocal = in6addr_any;
	__identifier Timestamp;
	
	
	dbg("Parse PBA message....\n");

	if (len < sizeof(struct ip6_mh_binding_ack) ||
	    mh_opt_parse(&pba->ip6mhba_hdr, len, 
			 sizeof(struct ip6_mh_binding_ack), &mh_opts) < 0)
				return -1;

	dbg("Received PBU message for a MN...\n");

	home_net_prefix = mh_opt(&pba->ip6mhba_hdr, &mh_opts, IP6_MHOPT_HOM_NET_PREX);
	if (home_net_prefix)
	{
		//copy
		memcpy(&peer_prefix,&home_net_prefix->ip6hnp_prefix,sizeof(struct in6_addr));
	} 


	mh_id_opt = mh_opt(&pba->ip6mhba_hdr, &mh_opts, IP6_MHOPT_MOB_IDENTIFIER);
	if (mh_id_opt)
	{
		//copy
		memcpy(&peer_addr.in6_u.u6_addr32[2],&mh_id_opt->mn_identifier,sizeof(__identifier));
	}

	time_stamp_opt = mh_opt(&pba->ip6mhba_hdr, &mh_opts, IP6_MHOPT_TIME_STAMP);
	if (time_stamp_opt)
	{
		//copy
		memcpy(&Timestamp,&time_stamp_opt->time_stamp,sizeof(__identifier));
	}


	link_local = mh_opt(&pba->ip6mhba_hdr, &mh_opts, IP6_MHOPT_LINK_ADDR);
	if (link_local)
	{
		//copy
		memcpy(&LinkLocal,&link_local->ip6link_addr,sizeof(struct in6_addr));
	}

	dbg("Received PBA for: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&peer_addr));
	dbg("Received PBA to : %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&conf.our_addr));

	struct pmip_entry *msg;
	msg = pmip_cache_get(&conf.our_addr,&peer_addr);
	if(msg==NULL)
		dbg("No Existing Entry is found!\n");
	else
	{
		//Delete timer (if any).
		del_task(&msg->tqe);
		//Modify the entry with additional info.
		dbg("An Existing Entry is found with type:%d\n",(msg->type));
		msg->PBA_flags = ntohs(pba->ip6mhba_flags);
		
		msg->lifetime.tv_sec  = ntohs(pba->ip6mhba_lifetime) << 2;
		dbg("Proxy Binding Ack Lifetime: %d (%d s)\n",pba->ip6mhba_lifetime, msg->lifetime.tv_sec);
		msg->FLAGS = hasPBA;
		dbg("FSM Flags: %d\n",msg->FLAGS);
	}
	return msg;
	

}

#ifdef TEST_PMIP
uint8_t netbuf[3000];
uint32_t netlen;
int pmip_mh_send(const struct in6_addr_bundle *addrs, const struct iovec *mh_vec, int iovlen, int oif)
{
	int m;
	bzero(netbuf, sizeof(netbuf));
	netlen = 0;

	for (m = 0; m < iovlen; m++) {
		memcpy(&netbuf[netlen], mh_vec[m].iov_base, mh_vec[m].iov_len);
		netlen += mh_vec[m].iov_len;
	}
	return 0;
}
#else

int pmip_mh_send(const struct in6_addr_bundle *addrs, const struct iovec *mh_vec, int iovlen, int oif)

{

	struct ip6_mh_opt_auth_data lbad;

	struct sockaddr_in6 daddr;

	

	struct iovec iov[2*(IP6_MHOPT_MAX+1)];

	struct msghdr msg;

	struct cmsghdr *cmsg;

	struct cmsghdr controlmsg;

	int cmsglen;

	struct in6_pktinfo pinfo;

	int ret = 0, on = 1;

	struct ip6_mh *mh;

	int iov_count;

	socklen_t rthlen = 0;


	iov_count = mh_try_pad(mh_vec, iov, iovlen);


	mh = (struct ip6_mh *)iov[0].iov_base;

	mh->ip6mh_hdrlen = (mh_length(iov, iov_count) >> 3) - 1;


	dbg("sending MH type %d\n"

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

	setsockopt(mh_sock.fd, IPPROTO_IPV6, IPV6_PKTINFO,

		   &on, sizeof(int));

	ret = sendmsg(mh_sock.fd, &msg, 0);

	if (ret < 0)

		dbg("sendmsg: %s\n", strerror(errno));



	pthread_mutex_unlock(&mh_sock.send_mutex);



	free(msg.msg_control);

	dbg("MH is sent....\n");

	return ret;

}


/**
	Creates the Proxy Binding Request with its options
*/

void mh_send_pbreq(struct in6_addr_bundle *addrs,struct in6_addr *peer_addr,struct in6_addr *peer_prefix,uint16_t seqno, int link, struct pmip_entry *bce)
{
	int iovlen = 1;
	struct ip6_mh_proxy_binding_request *pbr;
	static struct iovec mh_vec[3];	
	bzero(mh_vec, sizeof(mh_vec));
	pbr = mh_create(&mh_vec[0], IP6_MH_TYPE_PBREQ);
	if (!pbr)
		return;
	//TODO verify (whether or not) the sequence number is the same as the last PBU received!
	pbr->ip6mhpbrr_seqno = htons(seqno);
	__identifier MN_ID;
	memcpy(&MN_ID, &peer_addr->in6_u.u6_addr32[2], sizeof(__identifier));
/**
	* create the options in this order:
	* home network prefix.
	* mobile Identifier.
	*
*/
	dbg("PBREQ options are created....\n");

	mh_create_opt_home_net_prefix(&mh_vec[iovlen++], peer_prefix);

	
	uint16_t p_flag = htons(IP6_MH_MNID);
	mh_create_opt_mn_identifier (&mh_vec[iovlen++], p_flag, &MN_ID);
	
		
	//calculate the length of the message.
	pbr->ip6mhpbrr_hdr.ip6mh_hdrlen = mh_length(&mh_vec,iovlen);

	dbg("send PBREQ....\n");
	pmip_mh_send(addrs, &mh_vec, iovlen, link);
	if(is_lma())
	{
		dbg("copy PBREQ message into PMIP entry iovec....\n");
		//copy the PBR message into the mh_vector for the entry for future retransmissions.
		memcpy(bce->mh_vec,mh_vec, sizeof(mh_vec));
		bce->iovlen = iovlen;
	}
	else
	free_iov_data(mh_vec, iovlen);
}

/**
	Parses the PBREQ messages.
*/

struct pmip_entry *mh_pbreq_parse(struct ip6_mh_proxy_binding_request *pbr, ssize_t len,const struct in6_addr_bundle *in_addrs,int iif)
{
	
	static struct mh_options mh_opts; 
	bzero(&mh_opts, sizeof(mh_opts));
		
	struct ip6_mh_opt_home_net_prefix  *home_net_prefix;
	struct ip6_mh_opt_mn_identifier  *mh_id_opt;


	dbg("Parse PBREQ message....\n");
	
	if (len < sizeof(struct ip6_mh_proxy_binding_request) ||
	    mh_opt_parse(&pbr->ip6mhpbrr_hdr, len, 
			 sizeof(struct ip6_mh_proxy_binding_request), &mh_opts) < 0)
		return -1;

	
	struct in6_addr peer_addr= in6addr_any, peer_prefix = in6addr_any;
		 
	home_net_prefix = mh_opt(&pbr->ip6mhpbrr_hdr, &mh_opts, IP6_MHOPT_HOM_NET_PREX);
	if (home_net_prefix)
	{
		//copy
		memcpy(&peer_prefix ,&home_net_prefix->ip6hnp_prefix, sizeof(struct in6_addr));
	} 


	mh_id_opt = mh_opt(&pbr->ip6mhpbrr_hdr, &mh_opts, IP6_MHOPT_MOB_IDENTIFIER);
	if (mh_id_opt)
	{
		//copy
		memcpy(&peer_addr.in6_u.u6_addr32[2],&mh_id_opt->mn_identifier,sizeof(__identifier));
		dbg("Peer_addr: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&peer_addr));
	}

	struct pmip_entry *msg;
	msg = pmip_cache_get(&conf.our_addr,&peer_addr);
	if(msg==NULL)
		dbg("No Existing Entry is found!\n");
	else
	{
		dbg("An Existing Entry is found!\n");
		msg->seqno = ntohs(pbr->ip6mhpbrr_seqno);
		msg->FLAGS = hasPBREQ;
	}
	return msg;
}

/**
	Creates the Proxy Binding response with its options
*/


void mh_send_pbres(struct in6_addr_bundle *addrs,struct in6_addr *peer_addr,struct in6_addr *LMA_addr,struct in6_addr *Serv_MAG_addr, struct in6_addr *peer_prefix,uint16_t seqno, int link)
{
	int iovlen = 1;
	struct ip6_mh_proxy_binding_response *pbre;
	static struct iovec mh_vect[4];
	bzero(mh_vect, sizeof(mh_vect));
	pbre = mh_create(&mh_vect[0], IP6_MH_TYPE_PBRES);
	if (!pbre)
		return;
	pbre->ip6mhpbre_seqno = htons(seqno);
	__identifier MN_ID;
	memcpy(&MN_ID, &peer_addr->in6_u.u6_addr32[2], sizeof(__identifier));

/**
	* create the options in this order.
	* home network prefix.
	* mobile Identifier.
	* Serving MAG Address.
*/
	dbg("PBRES options are created....\n");

	mh_create_opt_home_net_prefix(&mh_vect[iovlen++], peer_prefix);

	uint16_t p_flag = htons(IP6_MH_MNID);
	mh_create_opt_mn_identifier (&mh_vect[iovlen++], p_flag, &MN_ID);

	mh_create_opt_serv_mag_addr(&mh_vect[iovlen++],Serv_MAG_addr);


	//calculate the length of the message.
	pbre->ip6mhpbre_hdr.ip6mh_hdrlen = mh_length(&mh_vect,iovlen);

	dbg("send PBRES....\n");
	pmip_mh_send(addrs, &mh_vect, iovlen, link);
	
	free_iov_data(mh_vect, iovlen);
	
}

/**
	Parses the PBRES messages.
*/

struct pmip_entry *mh_pbres_parse(struct ip6_mh_proxy_binding_response *pbre, ssize_t len,const struct in6_addr_bundle *in_addrs,int iif)
{
	
	static struct mh_options mh_opts; 
	bzero(&mh_opts, sizeof(mh_opts));
		
	struct ip6_mh_opt_home_net_prefix * home_net_prefix;
	struct ip6_mh_opt_mn_identifier * mh_id_opt;
	struct ip6_mh_opt_serv_mag_addr * serv_mag_addr_opt;

	dbg("Parse PBRES message....\n");
	
	if (len < sizeof(struct ip6_mh_proxy_binding_response) ||
	    mh_opt_parse(&pbre->ip6mhpbre_hdr, len, 
			 sizeof(struct ip6_mh_proxy_binding_response), &mh_opts) < 0)
		return -1;

	
	struct in6_addr peer_addr= in6addr_any, peer_prefix= in6addr_any, serv_mag = in6addr_any;
		 
	home_net_prefix = mh_opt(&pbre->ip6mhpbre_hdr, &mh_opts, IP6_MHOPT_HOM_NET_PREX);
	if (home_net_prefix)
	{
		//copy
		memcpy(&peer_prefix ,&home_net_prefix->ip6hnp_prefix, sizeof(struct in6_addr));
	} 


	mh_id_opt = mh_opt(&pbre->ip6mhpbre_hdr, &mh_opts, IP6_MHOPT_MOB_IDENTIFIER);
	if (mh_id_opt)
	{
		//copy
		memcpy(&peer_addr.in6_u.u6_addr32[2],&mh_id_opt->mn_identifier,sizeof(__identifier));
		dbg("Peer_addrs for: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&peer_addr));
	}

	serv_mag_addr_opt = mh_opt(&pbre->ip6mhpbre_hdr, &mh_opts, IP6_MHOPT_SERV_MAG_ADDR);
	if (serv_mag_addr_opt)
	{
		//copy
		memcpy(&serv_mag ,&serv_mag_addr_opt->serv_mag_addr, sizeof(struct in6_addr));
		dbg("Serv_mag_addrs for: %x:%x:%x:%x:%x:%x:%x:%x\n", NIP6ADDR(&serv_mag));
	} 

	if(is_mag()){
		struct pmip_entry msg;
		memcpy(&msg.Serv_MAG_addr,&serv_mag , sizeof(struct in6_addr));
		memcpy(&msg.peer_addr,&peer_addr,sizeof(struct in6_addr));
		msg.FLAGS = hasPBRES;
		return &msg;
	}

	if(is_lma()){
		struct pmip_entry *msg;
		msg = pmip_cache_get(&conf.our_addr,&peer_addr);
		
		if(msg==NULL)
			dbg("No Existing Entry is found!\n");
		else
		{
			dbg("An Existing Entry is found!\n");
			msg->FLAGS = hasPBRES;
			pmipcache_release_entry(msg);
		}
		return msg;
	}
	
}
#endif


