/***************************************************************************
                          nasmt_proto.h  -  description
                             -------------------
    copyright            : (C) 2002 by Eurecom
    email                : michelle.wetterwald@eurecom.fr
                           yan.moret@eurecom.fr
 ***************************************************************************

 ***************************************************************************/

#ifndef _NASMTD_PROTO_H
#define _NASMTD_PROTO_H

#include <linux/if_arp.h>
#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/ipv6.h>
#include <linux/ip.h>
#include <linux/sysctl.h>
#include <linux/timer.h>
#include <asm/param.h>
//#include <sys/sysctl.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/icmp.h>
#include <linux/icmpv6.h>
#include <linux/in.h>
#include <net/ndisc.h>

#include "rrc_nas_primitives.h"
#include "protocol_vars_extern.h"
#include "as_sap.h"
#include "rrc_qos.h"
#include "rrc_sap.h"

// nasmt_common.c
void nasmt_COMMON_receive(u16 hlen, u16 dlength, int sap);
void nasmt_COMMON_QOS_send(struct sk_buff *skb, struct cx_entity *cx, struct classifier_entity *gc);
void nasmt_COMMON_del_send(struct sk_buff *skb, struct cx_entity *cx, struct classifier_entity *gc);
void nasmt_COMMON_QOS_receive(struct cx_entity *cx);
struct rb_entity *nasmt_COMMON_add_rb(struct cx_entity *cx, nasRadioBearerId_t rabi, nasQoSTrafficClass_t qos);
struct rb_entity *nasmt_COMMON_search_rb(struct cx_entity *cx, nasRadioBearerId_t rabi);
struct cx_entity *nasmt_COMMON_search_cx(nasLocalConnectionRef_t lcr);
void nasmt_COMMON_del_rb(struct cx_entity *cx, nasRadioBearerId_t rab_id, nasIPdscp_t dscp);
void nasmt_COMMON_flush_rb(struct cx_entity *cx);


//nasmt_ascontrol.c
void nasmt_ASCTL_init(void);
void nasmt_ASCTL_timer(unsigned long data);
int  nasmt_ASCTL_DC_receive(struct cx_entity *cx);
int  nasmt_ASCTL_GC_receive(void);
int  nasmt_ASCTL_DC_send_cx_establish_request(struct cx_entity *cx);
int  nasmt_ASCTL_DC_send_cx_release_request(struct cx_entity *cx);
void nasmt_ASCTL_DC_send_sig_data_request(struct sk_buff *skb, struct cx_entity *cx, struct classifier_entity *gc);

// graal_iocontrol.c
void nasmt_CTL_send(struct sk_buff *skb, struct cx_entity *cx, struct classifier_entity *gc);
//int graal_CTL_receive_authentication(struct ipv6hdr *iph, struct cx-entity *cx, u8 sapi);
int nasmt_CTL_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);

// graal_classifier.c
void nasmt_CLASS_send(struct sk_buff *skb);
struct classifier_entity *nasmt_CLASS_add_sclassifier(struct cx_entity *cx, u8 dscp, u16 classref);
struct classifier_entity *nasmt_CLASS_add_rclassifier(u8 dscp, u16 classref);
void nasmt_CLASS_del_sclassifier(struct cx_entity *cx, u8 dscp, u16 classref);
void nasmt_CLASS_del_rclassifier(u8 dscp, u16 classref);
void nasmt_CLASS_flush_sclassifier(struct cx_entity *cx);
void nasmt_CLASS_flush_rclassifier(void);

// graal_tool.c
u8 nasmt_TOOL_invfct(struct classifier_entity *gc);
void nasmt_TOOL_fct(struct classifier_entity *gc, u8 fct);
void nasmt_TOOL_imei2iid(u8 *imei, u8 *iid);
u8 nasmt_TOOL_get_dscp6(struct ipv6hdr *iph);
u8 nasmt_TOOL_get_dscp4(struct iphdr *iph);
u8 *nasmt_TOOL_get_protocol6(struct ipv6hdr *iph, u8 *protocol);
u8 *nasmt_TOOL_get_protocol4(struct iphdr *iph, u8 *protocol);
char *nasmt_TOOL_get_udpmsg(struct udphdr *udph);
u16 nasmt_TOOL_udpcksum(struct in6_addr *saddr, struct in6_addr *daddr, u8 proto, u32 udplen, void *data);
int nasmt_TOOL_network6(struct in6_addr *addr, struct in6_addr *prefix, u8 plen);
int nasmt_TOOL_network4(u32 *addr, u32 *prefix, u8 plen);

void nasmt_TOOL_pk_all(struct sk_buff *skb);
void nasmt_TOOL_pk_ipv6(struct ipv6hdr *iph);
void nasmt_TOOL_print_state(u8 state);
void nasmt_TOOL_print_buffer(unsigned char * buffer,int length);
void nasmt_TOOL_print_rb_entity(struct rb_entity *rb);
void nasmt_TOOL_print_classifier(struct classifier_entity *gc);

#endif
