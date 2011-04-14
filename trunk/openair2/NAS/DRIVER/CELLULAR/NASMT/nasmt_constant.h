/***************************************************************************
                          nasmt_constant.h  -  description
                             -------------------
    copyright            : (C) 2002 by Eurecom
    email                : michelle.wetterwald@eurecom.fr
                           yan.moret@eurecom.fr
 ***************************************************************************
  Defines all constants for NAS MT - In particular, constants for the default RAB
 ***************************************************************************/

#ifndef _NASMTD_CST
#define _NASMTD_CST

//Debug flags
#define GRAAL_DEBUG_DC
//#define GRAAL_DEBUG_DC_DETAIL    // detail of DC-SAP operation
//#define GRAAL_DEBUG_SEND
//#define GRAAL_DEBUG_SEND_DETAIL  // detail of packet transmission
//#define GRAAL_DEBUG_RECEIVE
#define GRAAL_DEBUG_RECEIVE_BASIC
#define GRAAL_DEBUG_CLASS
//#define GRAAL_DEBUG_GC
//#define GRAAL_DEBUG_DC_MEASURE
//#define GRAAL_DEBUG_TIMER
#define GRAAL_DEBUG_DEVICE
//#define GRAAL_DEBUG_INTERRUPT
//#define GRAAL_DEBUG_TOOL

// Other flags
#define DEMO_3GSM

// Parameters for the default RAB started after attachment (needs DEMO_3GSM defined)
#define NASMT_DEFAULTRAB_CLASSREF   1  //MW-01/01/07-
#define NASMT_DEFAULTRAB_DSCP       0  //MW-01/01/07-
#define NASMT_DEFAULTRAB_IPVERSION  GRAAL_VERSION_DEFAULT  //MW-01/01/07-

// General Constants
#define GRAAL_MTU 1500
#define GRAAL_TX_QUEUE_LEN 100
#define GRAAL_ADDR_LEN 8
#define GRAAL_INET6_ADDRSTRLEN 46
#define GRAAL_INET_ADDRSTRLEN 16

#define GRAAL_CX_MAX 1
//#define GRAAL_CX_MULTICAST_ALLNODE 2
#define NASMT_MBMS_SVCES_MAX 4 // Identical to RRC constant

#define GRAAL_RETRY_LIMIT_DEFAULT 5

#define GRAAL_MESSAGE_MAXLEN 1600

#define GRAAL_SIG_SRB3 3
#define GRAAL_SIG_SRB4 3 // not used yet

//peer-to-peer messages between GRAAL entities
#define GRAAL_CMD_OPEN_RB 1

//#define GRAAL_IID1_CONTROL 0x0
//#define GRAAL_IID2_CONTROL __constant_htonl(0xffffffff)

//#define GRAAL_STATE_IDLE 			0
//#define GRAAL_STATE_CONNECTED			1
//#define GRAAL_STATE_ESTABLISHMENT_REQUEST	2
//#define GRAAL_STATE_ESTABLISHMENT_FAILURE	3
//#define GRAAL_STATE_RELEASE_FAILURE		4
#define GRAAL_CX_RELEASE_UNDEF_CAUSE 1

// MT+RG GRAAL States
#define GRAAL_IDLE                  0x01
// Connection
#define GRAAL_CX_FACH               0x06
#define GRAAL_CX_DCH                0x0A
#define GRAAL_CX_RECEIVED           0x10
#define GRAAL_CX_CONNECTING         0x04
#define GRAAL_CX_RELEASING          0x08
#define GRAAL_CX_CONNECTING_FAILURE 0x14
#define GRAAL_CX_RELEASING_FAILURE  0x18
// Radio Bearers
#define GRAAL_RB_ESTABLISHING       0x24
#define GRAAL_RB_RELEASING          0x28
#define GRAAL_RB_DCH					      0x2A


#define GRAAL_TIMER_ESTABLISHMENT_DEFAULT 12
#define GRAAL_TIMER_RELEASE_DEFAULT 2
#define GRAAL_TIMER_IDLE UINT_MAX
#define GRAAL_TIMER_TICK HZ

#define NAS_PDCPH_SIZE sizeof(struct pdcp_data_req)
#define GRAAL_IPV4_SIZE 20
#define GRAAL_IPV6_SIZE 40

#define GRAAL_DIRECTION_SEND	0
#define GRAAL_DIRECTION_RECEIVE	1

// function number
#define GRAAL_FCT_DEL_SEND	1
#define GRAAL_FCT_QOS_SEND	2
#define GRAAL_FCT_DC_SEND	3
#define GRAAL_FCT_CTL_SEND	4

// type of IOCTL command
#define NASMT_IOCTL_RAL 0x89F0

// Error cause
#define GRAAL_ERROR_ALREADYEXIST	1
#define GRAAL_ERROR_NOMEMORY		3
#define GRAAL_ERROR_NOTMT 		9
#define GRAAL_ERROR_NOTRG 		10
#define GRAAL_ERROR_NOTIDLE 		11
#define GRAAL_ERROR_NOTCONNECTED  	12
#define GRAAL_ERROR_NORB		14
#define GRAAL_ERROR_NOTCORRECTVALUE	32
#define GRAAL_ERROR_NOTCORRECTLCR	33
#define GRAAL_ERROR_NOTCORRECTDIR	34
#define GRAAL_ERROR_NOTCORRECTDSCP	35
#define GRAAL_ERROR_NOTCORRECTVERSION	36
#define GRAAL_ERROR_NOTCORRECTRABI	37


/**********************************************************/
/* Constants related with IP protocols                    */
/**********************************************************/

//#define GRAAL_PORT_CONTROL __constant_htons(0xc45)
//#define GRAAL_PORT_AUTHENTICATION __constant_htons(1811)

//#define GRAAL_TRAFFICCLASS_MASK __constant_htonl(0x0fc00000) //Yan
#define GRAAL_TRAFFICCLASS_MASK __constant_htonl(0x0ff00000)

// Network control codepoint 111000 + IP version 6
#define GRAAL_FLOWINFO_NCONTROL __constant_htonl(0x6e000000)
// network control codepoint 111000
#define GRAAL_DSCP_NCONTROL 56   //0x38
// default codepoint 1000000
#define GRAAL_DSCP_DEFAULT 64
#define GRAAL_DSCP_MAX 65

#define GRAAL_PROTOCOL_DEFAULT 0
#define GRAAL_PROTOCOL_TCP IPPROTO_TCP
#define GRAAL_PROTOCOL_UDP IPPROTO_UDP
#define GRAAL_PROTOCOL_ICMP4 IPPROTO_ICMP
#define GRAAL_PROTOCOL_ICMP6 IPPROTO_ICMPV6

#define GRAAL_PORT_DEFAULT	__constant_htons(65535)
#define GRAAL_PORT_HTTP 	__constant_htons(80)

#define GRAAL_VERSION_DEFAULT 	0
#define GRAAL_VERSION_4 	4
#define GRAAL_VERSION_6 	0 //?MW




#endif



