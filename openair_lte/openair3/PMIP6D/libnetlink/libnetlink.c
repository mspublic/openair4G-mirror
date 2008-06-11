/*
 * libnetlink.c	RTnetlink service routines.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <net/if_arp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/uio.h>

#include "libnetlink.h"

void rtnl_close(struct rtnl_handle *rth)
{
	close(rth->fd);
}

int rtnl_wilddump_request(struct rtnl_handle *rth, int family, int type)
{
	struct {
		struct nlmsghdr nlh;
		struct rtgenmsg g;
	} req;
	struct sockaddr_nl nladdr;

	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;

	req.nlh.nlmsg_len = sizeof(req);
	req.nlh.nlmsg_type = type;
	req.nlh.nlmsg_flags = NLM_F_ROOT|NLM_F_MATCH|NLM_F_REQUEST;
	req.nlh.nlmsg_pid = 0;
	req.nlh.nlmsg_seq = rth->dump = ++rth->seq;
	req.g.rtgen_family = family;

	return sendto(rth->fd, (void*)&req, sizeof(req), 0, (struct sockaddr*)&nladdr, sizeof(nladdr));
}

int rtnl_dump_filter(struct rtnl_handle *rth,
		     int (*filter)(struct sockaddr_nl *, struct nlmsghdr *n, void *),
		     void *arg1,
		     int (*junk)(struct sockaddr_nl *,struct nlmsghdr *n, void *),
		     void *arg2)
{
	char	buf[8192];
	struct sockaddr_nl nladdr;
	struct iovec iov;

	iov.iov_base = buf;
	iov.iov_len = sizeof(buf);

	while (1) {
		int status;
		struct nlmsghdr *h;

		struct msghdr msg;

		msg.msg_name = (void*)&nladdr;
		msg.msg_namelen = sizeof(nladdr);
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;
		msg.msg_control = NULL;
		msg.msg_controllen = 0;
		msg.msg_flags = 0;

		status = recvmsg(rth->fd, &msg, 0);

		if (status < 0) {
			continue;
		}
		if (status == 0) {
			return -1;
		}
		if (msg.msg_namelen != sizeof(nladdr)) {
			return -1;
		}

		h = (struct nlmsghdr*)buf;
		while (NLMSG_OK(h, status)) {
			int err;

			if (nladdr.nl_pid != 0 ||
			    h->nlmsg_pid != rth->local.nl_pid ||
			    h->nlmsg_seq != rth->dump) {
				if (junk) {
					err = junk(&nladdr, h, arg2);
					if (err < 0)
						return err;
				}
				goto skip_it;
			}

			if (h->nlmsg_type == NLMSG_DONE)
				return 0;
			if (h->nlmsg_type == NLMSG_ERROR) {
				struct nlmsgerr *err = (struct nlmsgerr*)NLMSG_DATA(h);
				if (h->nlmsg_len < NLMSG_LENGTH(sizeof(struct nlmsgerr))) {
				} else {
					return err->error;
				}
				return -1;
			}
			err = filter(&nladdr, h, arg1);
			if (err < 0)
				return err;

skip_it:
			h = NLMSG_NEXT(h, status);
		}
		if (msg.msg_flags & MSG_TRUNC) {
			continue;
		}
		if (status) {
			return -1;
		}
	}
}

int rtnl_talk(struct rtnl_handle *rtnl, struct nlmsghdr *n, pid_t peer,
	      unsigned groups, struct nlmsghdr *answer,
	      int (*junk)(struct sockaddr_nl *,struct nlmsghdr *n, void *),
	      void *jarg)
{
	int status;
	unsigned seq;
	struct nlmsghdr *h;
	struct sockaddr_nl nladdr;
	struct iovec iov;
	char   buf[8192];
	struct msghdr msg;

	iov.iov_base = (void*)n;
	iov.iov_len = n->nlmsg_len;

	msg.msg_name = (void*)&nladdr;
	msg.msg_namelen = sizeof(nladdr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;

	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;
	nladdr.nl_pid = peer;
	nladdr.nl_groups = groups;

	n->nlmsg_seq = seq = ++rtnl->seq;
	if (answer == NULL)
		n->nlmsg_flags |= NLM_F_ACK;

	status = sendmsg(rtnl->fd, &msg, 0);

	if (status < 0) {
		return -1;
	}

	iov.iov_base = buf;

	while (1) {
		iov.iov_len = sizeof(buf);
		status = recvmsg(rtnl->fd, &msg, 0);

		if (status < 0) {
			continue;
		}
		if (status == 0) {
			return -1;
		}
		if (msg.msg_namelen != sizeof(nladdr)) {
			return -1;
		}
		for (h = (struct nlmsghdr*)buf; status >= sizeof(*h); ) {
			int err;
			int len = h->nlmsg_len;
			int l = len - sizeof(*h);

			if (l<0 || len>status) {
				return -1;
			}

			if (nladdr.nl_pid != peer ||
			    h->nlmsg_pid != rtnl->local.nl_pid ||
			    h->nlmsg_seq != seq) {
				if (junk) {
					err = junk(&nladdr, h, jarg);
					if (err < 0)
						return err;
				}
				continue;
			}

			if (h->nlmsg_type == NLMSG_ERROR) {
				struct nlmsgerr *err = (struct nlmsgerr*)NLMSG_DATA(h);
				if (l < sizeof(struct nlmsgerr)) {
				} else {
					if (!err->error) {
						if (answer)
							memcpy(answer, h, h->nlmsg_len);
						return 0;
					}
				}
				return err->error;
			}
			if (answer) {
				memcpy(answer, h, h->nlmsg_len);
				return 0;
			}

			status -= NLMSG_ALIGN(len);
			h = (struct nlmsghdr*)((char*)h + NLMSG_ALIGN(len));
		}
		if (msg.msg_flags & MSG_TRUNC) {
			continue;
		}
		if (status) {
			return -1;
		}
	}
}

int addattr32(struct nlmsghdr *n, int maxlen, int type, __u32 data)
{
	int len = RTA_LENGTH(4);
	struct rtattr *rta;
	if (NLMSG_ALIGN(n->nlmsg_len) + len > maxlen)
		return -1;
	rta = (struct rtattr*)(((char*)n) + NLMSG_ALIGN(n->nlmsg_len));
	rta->rta_type = type;
	rta->rta_len = len;
	memcpy(RTA_DATA(rta), &data, 4);
	n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + len;
	return 0;
}

int addattr_l(struct nlmsghdr *n, int maxlen, int type, const void *data, int alen)
{
	int len = RTA_LENGTH(alen);
	struct rtattr *rta;

	if (NLMSG_ALIGN(n->nlmsg_len) + len > maxlen)
		return -1;
	rta = (struct rtattr*)(((char*)n) + NLMSG_ALIGN(n->nlmsg_len));
	rta->rta_type = type;
	rta->rta_len = len;
	memcpy(RTA_DATA(rta), data, alen);
	n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + len;
	return 0;
}

int rta_addattr32(struct rtattr *rta, int maxlen, int type, __u32 data)
{
	int len = RTA_LENGTH(4);
	struct rtattr *subrta;

	if (RTA_ALIGN(rta->rta_len) + len > maxlen)
		return -1;
	subrta = (struct rtattr*)(((char*)rta) + RTA_ALIGN(rta->rta_len));
	subrta->rta_type = type;
	subrta->rta_len = len;
	memcpy(RTA_DATA(subrta), &data, 4);
	rta->rta_len = NLMSG_ALIGN(rta->rta_len) + len;
	return 0;
}

int rta_addattr_l(struct rtattr *rta, int maxlen, int type, void *data, int alen)
{
	struct rtattr *subrta;
	int len = RTA_LENGTH(alen);

	if (RTA_ALIGN(rta->rta_len) + len > maxlen)
		return -1;
	subrta = (struct rtattr*)(((char*)rta) + RTA_ALIGN(rta->rta_len));
	subrta->rta_type = type;
	subrta->rta_len = len;
	memcpy(RTA_DATA(subrta), data, alen);
	rta->rta_len = NLMSG_ALIGN(rta->rta_len) + len;
	return 0;
}


int parse_rtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len)
{
	while (RTA_OK(rta, len)) {
		if (rta->rta_type <= max)
			tb[rta->rta_type] = rta;
		rta = RTA_NEXT(rta,len);
	}
	return 0;
}
