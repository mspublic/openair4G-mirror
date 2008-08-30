#ifndef __NET_GENERIC_NETLINK_H
#define __NET_GENERIC_NETLINK_H

#include <linux/genetlink.h>
#include <net/netlink.h>

/**
 * struct genl_family - generic netlink family
 * @id: protocol family idenfitier
 * @hdrsize: length of user specific header in bytes
 * @name: name of family
 * @version: protocol version
 * @maxattr: maximum number of attributes supported
 * @attrbuf: buffer to store parsed attributes
 * @ops_list: list of all assigned operations
 * @family_list: family list
 */
struct genl_family
{
	unsigned int		id;
	unsigned int		hdrsize;
	char			name[GENL_NAMSIZ];
	unsigned int		version;
	unsigned int		maxattr;
	struct nlattr **	attrbuf;	/* private */
	struct list_head	ops_list;	/* private */
	struct list_head	family_list;	/* private */
};

#define GENL_ADMIN_PERM		0x01

/**
 * struct genl_info - receiving information
 * @snd_seq: sending sequence number
 * @snd_pid: netlink pid of sender
 * @nlhdr: netlink message header
 * @genlhdr: generic netlink message header
 * @userhdr: user specific header
 * @attrs: netlink attributes
 */
struct genl_info
{
	u32			snd_seq;
	u32			snd_pid;
	struct nlmsghdr *	nlhdr;
	struct genlmsghdr *	genlhdr;
	void *			userhdr;
	struct nlattr **	attrs;
};

/**
 * struct genl_ops - generic netlink operations
 * @cmd: command identifier
 * @flags: flags
 * @policy: attribute validation policy
 * @doit: standard command callback
 * @dumpit: callback for dumpers
 * @ops_list: operations list
 */
struct genl_ops
{
	u8			cmd;
	unsigned int		flags;
	struct nla_policy	*policy;
	int		       (*doit)(struct sk_buff *skb,
				       struct genl_info *info);
	int		       (*dumpit)(struct sk_buff *skb,
					 struct netlink_callback *cb);
	struct list_head	ops_list;
};

extern int genl_register_family(struct genl_family *family);
extern int genl_unregister_family(struct genl_family *family);
extern int genl_register_ops(struct genl_family *, struct genl_ops *ops);
extern int genl_unregister_ops(struct genl_family *, struct genl_ops *ops);

extern struct sock *genl_sock;

/**
 * genlmsg_put - Add generic netlink header to netlink message
 * @skb: socket buffer holding the message
 * @pid: netlink pid the message is addressed to
 * @seq: sequence number (usually the one of the sender)
 * @type: netlink message type
 * @hdrlen: length of the user specific header
 * @flags netlink message flags
 * @cmd: generic netlink command
 * @version: version
 *
 * Returns pointer to user specific header
 */
static inline void *genlmsg_put(struct sk_buff *skb, u32 pid, u32 seq,
				int type, int hdrlen, int flags,
				u8 cmd, u8 version)
{
	struct nlmsghdr *nlh;
	struct genlmsghdr *hdr;

	nlh = nlmsg_put(skb, pid, seq, type, GENL_HDRLEN + hdrlen, flags);
	if (nlh == NULL)
		return NULL;

	hdr = nlmsg_data(nlh);
	hdr->cmd = cmd;
	hdr->version = version;
	hdr->reserved = 0;

	return (char *) hdr + GENL_HDRLEN;
}

/**
 * genlmsg_end - Finalize a generic netlink message
 * @skb: socket buffer the message is stored in
 * @hdr: user specific header
 */
static inline int genlmsg_end(struct sk_buff *skb, void *hdr)
{
	return nlmsg_end(skb, hdr - GENL_HDRLEN - NLMSG_HDRLEN);
}

/**
 * genlmsg_cancel - Cancel construction of a generic netlink message
 * @skb: socket buffer the message is stored in
 * @hdr: generic netlink message header
 */
static inline int genlmsg_cancel(struct sk_buff *skb, void *hdr)
{
	return nlmsg_cancel(skb, hdr - GENL_HDRLEN - NLMSG_HDRLEN);
}

/**
 * genlmsg_multicast - multicast a netlink message
 * @skb: netlink message as socket buffer
 * @pid: own netlink pid to avoid sending to yourself
 * @group: multicast group id
 */
static inline int genlmsg_multicast(struct sk_buff *skb, u32 pid,
				    unsigned int group)
{
	return nlmsg_multicast(genl_sock, skb, pid, group);
}

/**
 * genlmsg_unicast - unicast a netlink message
 * @skb: netlink message as socket buffer
 * @pid: netlink pid of the destination socket
 */
static inline int genlmsg_unicast(struct sk_buff *skb, u32 pid)
{
	return nlmsg_unicast(genl_sock, skb, pid);
}

#endif	/* __NET_GENERIC_NETLINK_H */
