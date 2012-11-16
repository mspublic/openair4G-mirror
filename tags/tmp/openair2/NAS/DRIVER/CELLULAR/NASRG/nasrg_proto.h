/***************************************************************************
                          nasrg_proto.h  -  description
                             -------------------
    copyright            : (C) 2002 by Eurecom
    email                : michelle.wetterwald@eurecom.fr
                           yan.moret@eurecom.fr
 ***************************************************************************

 ***************************************************************************/

#ifndef _NASRGD_PROTO_H
#define _NASRGD_PROTO_H

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

// nasrg_common.c
void nasrg_COMMON_receive(u16 hlen, u16 dlength, int sap);
void nasrg_COMMON_QOS_send(struct sk_buff *skb, struct cx_entity *cx, struct classifier_entity *gc);
void nasrg_COMMON_del_send(struct sk_buff *skb, struct cx_entity *cx, struct classifier_entity *gc);
void nasrg_COMMON_QOS_receive(struct cx_entity *cx);
struct rb_entity *nasrg_COMMON_add_rb(struct cx_entity *cx, nasRadioBearerId_t rabi, nasQoSTrafficClass_t qos);
struct rb_entity *nasrg_COMMON_search_rb(struct cx_entity *cx, nasRadioBearerId_t rabi);
struct cx_entity *nasrg_COMMON_search_cx(nasLocalConnectionRef_t lcr);
void nasrg_COMMON_del_rb(struct cx_entity *cx, nasRadioBearerId_t rab_id, nasIPdscp_t dscp);
void nasrg_COMMON_flush_rb(struct cx_entity *cx);

//nasrg_ascontrol.c
void nasrg_ASCTL_init(void);
void nasrg_ASCTL_timer(unsigned long data);
void nasrg_ASCTL_DC_send_sig_data_request(struct sk_buff *skb, struct cx_entity *cx, struct classifier_entity *gc);
int nasrg_ASCTL_DC_receive(struct cx_entity *cx);
int nasrg_ASCTL_DC_send_cx_establish_confirm(struct cx_entity *cx, u8 response);
int nasrg_ASCTL_DC_send_rb_establish_request(struct cx_entity *cx, struct rb_entity *rb);
int nasrg_ASCTL_DC_send_rb_release_request(struct cx_entity *cx, struct rb_entity *rb);
int nasrg_ASCTL_GC_send_mbms_bearer_establish_req(int mbms_ix );
int nasrg_ASCTL_GC_send_mbms_bearer_release_req(int mbms_ix);
int nasrg_ASCTL_DC_send_mbms_ue_notify_req(struct cx_entity *cx);

// nasrg_iocontrol.c
void nasrg_CTL_send(struct sk_buff *skb, struct cx_entity *cx, struct classifier_entity *gc);
int nasrg_CTL_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);

// nasrg_classifier.c
void nasrg_CLASS_send(struct sk_buff *skb);
struct classifier_entity *nasrg_CLASS_add_sclassifier(struct cx_entity *cx, u8 dscp, u16 classref);
struct classifier_entity *nasrg_CLASS_add_rclassifier(u8 dscp, u16 classref);
struct classifier_entity *nasrg_CLASS_add_mbmsclassifier(int mbms_ix, u16 classref);
void nasrg_CLASS_del_sclassifier(struct cx_entity *cx, u8 dscp, u16 classref);
void nasrg_CLASS_del_rclassifier(u8 dscp, u16 classref);
void nasrg_CLASS_del_mbmsclassifier(int mbms_ix, u16 classref);
void nasrg_CLASS_flush_sclassifier(struct cx_entity *cx);
void nasrg_CLASS_flush_rclassifier(void);
void nasrg_CLASS_flush_mbmsclassifier(void);

// nasrg_tool.c
u8 nasrg_TOOL_invfct(struct classifier_entity *gc);
void nasrg_TOOL_fct(struct classifier_entity *gc, u8 fct);
void nasrg_TOOL_imei2iid(u8 *imei, u8 *iid);
void nasrg_TOOL_RGimei2iid(u8 *imei, u8 *iid);
u8 nasrg_TOOL_get_dscp6(struct ipv6hdr *iph);
u8 nasrg_TOOL_get_dscp4(struct iphdr *iph);
u8 *nasrg_TOOL_get_protocol6(struct ipv6hdr *iph, u8 *protocol);
u8 *nasrg_TOOL_get_protocol4(struct iphdr *iph, u8 *protocol);
char *nasrg_TOOL_get_udpmsg(struct udphdr *udph);
u16 nasrg_TOOL_udpcksum(struct in6_addr *saddr, struct in6_addr *daddr, u8 proto, u32 udplen, void *data);
int nasrg_TOOL_network6(struct in6_addr *addr, struct in6_addr *prefix, u8 plen);
int nasrg_TOOL_network4(u32 *addr, u32 *prefix, u8 plen);

void nasrg_TOOL_pk_all(struct sk_buff *skb);
void nasrg_TOOL_pk_ipv6(struct ipv6hdr *iph);
void nasrg_TOOL_print_state(u8 state);
void nasrg_TOOL_print_buffer(unsigned char * buffer,int length);
void nasrg_TOOL_print_rb_entity(struct rb_entity *rb);
void nasrg_TOOL_print_classifier(struct classifier_entity *gc);

#endif
