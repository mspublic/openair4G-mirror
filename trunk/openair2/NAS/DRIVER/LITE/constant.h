/*******************************************************************************

  Eurecom OpenAirInterface 2
  Copyright(c) 1999 - 2010 Eurecom

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fsr/openairinterface
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/


#ifndef _OAI_NW_DRV_CST
#define _OAI_NW_DRV_CST

#define MAX_MEASURE_NB 5
#define OAI_NW_DRV_MAX_LENGTH 180

//Debug flags
//#define OAI_NW_DRV_DEBUG_DC
#define OAI_NW_DRV_DEBUG_SEND
#define OAI_NW_DRV_DEBUG_RECEIVE
//#define OAI_NW_DRV_DEBUG_CLASS
//#define OAI_NW_DRV_DEBUG_GC
//#define OAI_NW_DRV_DEBUG_DC_MEASURE
//#define OAI_NW_DRV_DEBUG_TIMER
//#define OAI_NW_DRV_DEBUG_DEVICE
//#define OAI_NW_DRV_DEBUG_INTERRUPT
//#define OAI_NW_DRV_DEBUG_TOOL

// Other flags
#define DEMO_3GSM

// General Constants
#define OAI_NW_DRV_MTU                    1500
#define OAI_NW_DRV_TX_QUEUE_LEN           100
#define OAI_NW_DRV_ADDR_LEN               8
#define OAI_NW_DRV_INET6_ADDRSTRLEN       46
#define OAI_NW_DRV_INET_ADDRSTRLEN        16

#define OAI_NW_DRV_RESET_RX_FLAGS         0

#define OAI_NW_DRV_CX_MAX                 32  //Identical to RRC constant
//#define OAI_NW_DRV_CX_MULTICAST_ALLNODE 2

#define OAI_NW_DRV_RETRY_LIMIT_DEFAULT    5

#define OAI_NW_DRV_MESSAGE_MAXLEN         5004

#define OAI_NW_DRV_SIG_SRB3               3
#define OAI_NW_DRV_SIG_SRB4               3 // not used yet

//peer-to-peer messages between NAS entities
#define OAI_NW_DRV_CMD_OPEN_RB            1

//#define OAI_NW_DRV_IID1_CONTROL 0x0
//#define OAI_NW_DRV_IID2_CONTROL __constant_htonl(0xffffffff)

//#define OAI_NW_DRV_STATE_IDLE                        0
//#define OAI_NW_DRV_STATE_CONNECTED                   1
//#define OAI_NW_DRV_STATE_ESTABLISHMENT_REQUEST       2
//#define OAI_NW_DRV_STATE_ESTABLISHMENT_FAILURE       3
//#define OAI_NW_DRV_STATE_RELEASE_FAILURE             4
#define OAI_NW_DRV_CX_RELEASE_UNDEF_CAUSE 1

// MT+RG NAS States
#define OAI_NW_DRV_IDLE                  0x01
// Connection
#define OAI_NW_DRV_CX_FACH               0x06
#define OAI_NW_DRV_CX_DCH                0x0A
#define OAI_NW_DRV_CX_RECEIVED           0x10
#define OAI_NW_DRV_CX_CONNECTING         0x04
#define OAI_NW_DRV_CX_RELEASING          0x08
#define OAI_NW_DRV_CX_CONNECTING_FAILURE 0x14
#define OAI_NW_DRV_CX_RELEASING_FAILURE  0x18
// Radio Bearers
#define OAI_NW_DRV_RB_ESTABLISHING       0x24
#define OAI_NW_DRV_RB_RELEASING          0x28
#define OAI_NW_DRV_RB_DCH                0x2A


#define OAI_NW_DRV_TIMER_ESTABLISHMENT_DEFAULT 12
#define OAI_NW_DRV_TIMER_RELEASE_DEFAULT       2
#define OAI_NW_DRV_TIMER_IDLE                  UINT_MAX
#define OAI_NW_DRV_TIMER_TICK                  HZ

#define OAI_NW_DRV_PDCPH_SIZE                  sizeof(struct pdcp_data_req_header_s)
#define OAI_NW_DRV_IPV4_SIZE                   20
#define OAI_NW_DRV_IPV6_SIZE                   40

#define OAI_NW_DRV_DIRECTION_SEND              0
#define OAI_NW_DRV_DIRECTION_RECEIVE           1
#define OAI_NW_DRV_DIRECTION_FORWARD           2

// function number
#define OAI_NW_DRV_FCT_DEL_SEND                1
#define OAI_NW_DRV_FCT_QOS_SEND                2
#define OAI_NW_DRV_FCT_DC_SEND                 3
#define OAI_NW_DRV_FCT_CTL_SEND                4

// type of IOCTL command
#define OAI_NW_DRV_IOCTL_RRM 0x89F0

// Error cause
#define OAI_NW_DRV_ERROR_ALREADYEXIST          1
#define OAI_NW_DRV_ERROR_NOMEMORY              3
#define OAI_NW_DRV_ERROR_NOTMT                 9
#define OAI_NW_DRV_ERROR_NOTRG                 10
#define OAI_NW_DRV_ERROR_NOTIDLE               11
#define OAI_NW_DRV_ERROR_NOTCONNECTED          12
#define OAI_NW_DRV_ERROR_NORB                  14
#define OAI_NW_DRV_ERROR_NOTCORRECTVALUE       32
#define OAI_NW_DRV_ERROR_NOTCORRECTLCR         33
#define OAI_NW_DRV_ERROR_NOTCORRECTDIR         34
#define OAI_NW_DRV_ERROR_NOTCORRECTDSCP        35
#define OAI_NW_DRV_ERROR_NOTCORRECTVERSION     36
#define OAI_NW_DRV_ERROR_NOTCORRECTRABI        37


/**********************************************************/
/* Constants related with IP protocols                    */
/**********************************************************/

//#define OAI_NW_DRV_PORT_CONTROL __constant_htons(0xc45)
//#define OAI_NW_DRV_PORT_AUTHENTICATION __constant_htons(1811)

#define OAI_NW_DRV_TRAFFICCLASS_MASK  __constant_htonl(0x0fc00000)

// Network control codepoint 111000 + IP version 6
#define OAI_NW_DRV_FLOWINFO_NCONTROL  __constant_htonl(0x6e000000)
// network control codepoint 111000
#define OAI_NW_DRV_DSCP_NCONTROL      56   //0x38
// default codepoint 1000000
#define OAI_NW_DRV_DSCP_DEFAULT       64
#define OAI_NW_DRV_DSCP_MAX           193

#define OAI_NW_DRV_PROTOCOL_DEFAULT   0
#define OAI_NW_DRV_PROTOCOL_TCP       IPPROTO_TCP
#define OAI_NW_DRV_PROTOCOL_UDP       IPPROTO_UDP
#define OAI_NW_DRV_PROTOCOL_ICMP4     IPPROTO_ICMP
#define OAI_NW_DRV_PROTOCOL_ICMP6     IPPROTO_ICMPV6
#warning "OAI_NW_DRV_PROTOCOL_ARP value 200 may collide with new defined values in kernel"
#define OAI_NW_DRV_PROTOCOL_ARP       200

#define OAI_NW_DRV_PORT_DEFAULT       __constant_htons(65535)
#define OAI_NW_DRV_PORT_HTTP          __constant_htons(80)

#define OAI_NW_DRV_IP_VERSION_ALL        0
#define OAI_NW_DRV_IP_VERSION_4          4
#define OAI_NW_DRV_IP_VERSION_6          6

#define OAI_NW_DRV_DEFAULT_IPV4_ADDR  0
#define OAI_NW_DRV_DEFAULT_IPV6_ADDR0 0
#define OAI_NW_DRV_DEFAULT_IPV6_ADDR1 0
#define OAI_NW_DRV_DEFAULT_IPV6_ADDR2 0
#define OAI_NW_DRV_DEFAULT_IPV6_ADDR3 0


#define OAI_NW_DRV_IPVX_ADDR_TYPE_UNKNOWN        0
#define OAI_NW_DRV_IPV6_ADDR_TYPE_UNICAST        1
#define OAI_NW_DRV_IPV6_ADDR_TYPE_MULTICAST      2
#define OAI_NW_DRV_IPV6_ADDR_TYPE_UNKNOWN        3

#define OAI_NW_DRV_IPV4_ADDR_TYPE_UNICAST        5
#define OAI_NW_DRV_IPV4_ADDR_TYPE_MULTICAST      6
#define OAI_NW_DRV_IPV4_ADDR_TYPE_BROADCAST      7
#define OAI_NW_DRV_IPV4_ADDR_TYPE_UNKNOWN        8


#define OAI_NW_DRV_MPLS_VERSION_CODE  99

#define OAI_NW_DRV_NB_INSTANCES_MAX       8


#endif

