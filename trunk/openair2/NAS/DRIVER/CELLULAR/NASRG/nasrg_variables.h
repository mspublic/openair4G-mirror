/***************************************************************************
                          nasrg_variables.h  -  description
                             -------------------
    copyright            : (C) 2002 by Eurecom
    email                : michelle.wetterwald@eurecom.fr
                           yan.moret@eurecom.fr
 ***************************************************************************

 ***************************************************************************/

#ifndef _NASRGD_VAR_H
#define _NASRGD_VAR_H

#include <linux/if_arp.h>
#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/ipv6.h>
#include <linux/ip.h>
#include <linux/sysctl.h>
#include <linux/timer.h>
#include <linux/unistd.h>
//#include <asm/unistd.h>
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

#include "nasrg_constant.h"
#include "nasrg_sap.h"

struct cx_entity;

struct rb_entity{
//  u16 default_rab;
  u32   cnxid;
	nasRadioBearerId_t rab_id;  //ue_rbId
	nasRadioBearerId_t rg_rbId;
	nasRadioBearerId_t mbms_rbId;
	nasSapId_t sapi;
	nasQoSTrafficClass_t qos;
	nasQoSTrafficClass_t RadioQosClass;
  nasIPdscp_t dscp;  //this is DL dscp
  nasIPdscp_t dscp_ul;
	u8 state;
  u8 result;
	u8 retry;
	u32 countimer;
//for MBMS
  u16 serviceId;
  u16 sessionId;
  u16 duration;
  u8  mcast_address[16];
//
	struct rb_entity *next;
};

struct classifier_entity{
	u32 classref;               // classifier identity
	u8 version;                 // IP version 4 or 6
	union{
		struct in6_addr ipv6;
		u32 ipv4;
	} saddr;                    // IP source address
	u8 splen;                   // IP prefix size
	union{
		struct in6_addr ipv6;
		u32 ipv4;
	} daddr;                    // IP destination address
	u8 dplen;                   // IP prefix size
	u8 protocol; 	              // layer 4 protocol type (tcp, udp, ...)
	u16 sport; 	                // source port
	u16 dport; 	                // destination port
	struct rb_entity *rb;
	nasRadioBearerId_t rab_id;  // RAB identification
	void (*fct)(struct sk_buff *skb, struct cx_entity *cx, struct classifier_entity *gc);
	struct classifier_entity *next;
};


struct cx_entity{
	int sap[GRAAL_SAPI_CX_MAX];
	u8 state; 			              // state of the connection
	nasLocalConnectionRef_t lcr;	// Local connection reference
	nasCellID_t cellid;		        // cell identification
	u32 countimer;			          // timeout's counter
	u8 retry;			                // number of retransmissions
	struct classifier_entity *sclassifier[GRAAL_DSCP_MAX]; // send classifiers table
	u16 nsclassifier;
	u32 iid6[2]; 			            // IPv6  interface identification
	u8 iid4; 			                // IPv4 interface identification
	struct rb_entity *rb;         // RB entities for RABs
	u16 num_rb;
  // MW - 17/5/05
  int  ue_id;
  int rrc_state;
  // MBMS
  int requested_joined_services[NASRG_MBMS_SVCES_MAX];
  int requested_left_services[NASRG_MBMS_SVCES_MAX];
  int joined_services[NASRG_MBMS_SVCES_MAX];
};

//struct mbms_rb_entity{
//  u32   cnxid;
//	nasRadioBearerId_t mbms_rbId;
//	nasSapId_t sapi;
//	nasQoSTrafficClass_t qos;
//	nasQoSTrafficClass_t RadioQosClass;
//	u8 state;
//  u8 result;
//	u8 retry;
//	u32 countimer;
//
//  u16 serviceId;
//  u16 sessionId;
//  u16 duration;
//};
//

struct graal_priv
{
	int irq;
	struct timer_list timer;
	spinlock_t lock;
	struct net_device_stats stats;
	u8 retry_limit;
	u32 timer_establishment;
	u32 timer_release;
	struct cx_entity cx[GRAAL_CX_MAX];
	struct classifier_entity *rclassifier[GRAAL_DSCP_MAX]; // receive classifier
	int nrclassifier;
  u32 next_sclassref;
	int sap[GRAAL_SAPI_MAX];
	u8 xbuffer[NAS_MAX_LENGTH]; // transmission buffer
	u8 rbuffer[NAS_MAX_LENGTH]; // reception buffer
  // MW - 17/5/05
  int broadcast_counter;
  int SIB18_counter;
  // MBMS
  struct rb_entity mbms_rb[NASRG_MBMS_SVCES_MAX];
	struct classifier_entity *mbmsclassifier[NASRG_MBMS_SVCES_MAX]; // mbms classifier
	int nmbmsclassifier;
  u32 next_mbmsclassref;
};

struct ipversion {
#if defined(__LITTLE_ENDIAN_BITFIELD)
         u8    reserved:4,
                 version:4;
#else
         u8    version:4,
                 reserved:4;
#endif
};

extern struct graal_priv *gpriv;
extern struct net_device *gdev;
extern int bytes_wrote;
extern int bytes_read;

extern u8 GRAAL_RG_IMEI[14];
extern u8 GRAAL_NULL_IMEI[14];

//global variables shared with RRC
extern int *pt_nas_rg_irq;
extern u16 *pt_rg_own_cell_id;

#endif
