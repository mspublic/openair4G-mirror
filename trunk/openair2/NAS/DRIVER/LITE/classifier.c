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
  Address      : Eurecom, 2229, route des crÃªtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/

/*! \file classifier.c
* \brief Classify IP packets
* \author  yan moret(no longer valid),  Michelle Wetterwald, Raymond knopp, Navid Nikaein, Lionel GAUTHIER
* \company Eurecom
* \email:  michelle.wetterwald@eurecom.fr, knopp@eurecom.fr, navid.nikaein@eurecom.fr, lionel.gauthier@eurecom.fr
*/


#include "local.h"
#include "proto_extern.h"
#include <net/ip6_fib.h>
#include <net/route.h>
#ifdef MPLS
#include <net/mpls.h>
#endif


#define IN_CLASSA(a)            ((((long int) (a)) & 0x80000000) == 0)
#define IN_CLASSB(a)            ((((long int) (a)) & 0xc0000000) == 0x80000000)
#define IN_CLASSC(a)            ((((long int) (a)) & 0xe0000000) == 0xc0000000)
#define IN_CLASSD(a)            ((((long int) (a)) & 0xf0000000) == 0xe0000000)
#define IN_MULTICAST(a)         IN_CLASSD(a)
#define IN_EXPERIMENTAL(a)      ((((long int) (a)) & 0xf0000000) == 0xf0000000)
#define IN_BADCLASS(a)          IN_EXPERIMENTAL((a))
/* Address to accept any incoming messages. */
#define INADDR_ANY              ((unsigned long int) 0x00000000)
/* Address to send to all hosts. */
#define INADDR_BROADCAST        ((unsigned long int) 0xffffffff)
/* Address indicating an error return. */
#define INADDR_NONE             ((unsigned long int) 0xffffffff)


#define NIPADDR(addr) \
        (uint8_t)(addr & 0x000000FF), \
        (uint8_t)((addr & 0x0000FF00) >> 8), \
        (uint8_t)((addr & 0x00FF0000) >> 16), \
        (uint8_t)((addr & 0xFF000000) >> 24)

#define NIP6ADDR(addr) \
        ntohs((addr)->s6_addr16[0]), \
        ntohs((addr)->s6_addr16[1]), \
        ntohs((addr)->s6_addr16[2]), \
        ntohs((addr)->s6_addr16[3]), \
        ntohs((addr)->s6_addr16[4]), \
        ntohs((addr)->s6_addr16[5]), \
        ntohs((addr)->s6_addr16[6]), \
        ntohs((addr)->s6_addr16[7])


#define IN6_IS_ADDR_UNSPECIFIED(a) \
        (((__const uint32_t *) (a))[0] == 0                                   \
         && ((__const uint32_t *) (a))[1] == 0                                \
         && ((__const uint32_t *) (a))[2] == 0                                \
         && ((__const uint32_t *) (a))[3] == 0)

#define IN6_IS_ADDR_LOOPBACK(a) \
        (((__const uint32_t *) (a))[0] == 0                                   \
         && ((__const uint32_t *) (a))[1] == 0                                \
         && ((__const uint32_t *) (a))[2] == 0                                \
         && ((__const uint32_t *) (a))[3] == htonl (1))

#define IN6_IS_ADDR_MULTICAST(a) (((__const uint8_t *) (a))[0] == 0xff)

#define IN6_IS_ADDR_LINKLOCAL(a) \
        ((((__const uint32_t *) (a))[0] & htonl (0xffc00000))                 \
         == htonl (0xfe800000))

#define IN6_IS_ADDR_SITELOCAL(a) \
        ((((__const uint32_t *) (a))[0] & htonl (0xffc00000))                 \
         == htonl (0xfec00000))

#define IN6_IS_ADDR_V4MAPPED(a) \
        ((((__const uint32_t *) (a))[0] == 0)                                 \
         && (((__const uint32_t *) (a))[1] == 0)                              \
         && (((__const uint32_t *) (a))[2] == htonl (0xffff)))

#define IN6_IS_ADDR_V4COMPAT(a) \
        ((((__const uint32_t *) (a))[0] == 0)                                 \
         && (((__const uint32_t *) (a))[1] == 0)                              \
         && (((__const uint32_t *) (a))[2] == 0)                              \
         && (ntohl (((__const uint32_t *) (a))[3]) > 1))

#define IN6_ARE_ADDR_EQUAL(a,b) \
        ((((__const uint32_t *) (a))[0] == ((__const uint32_t *) (b))[0])     \
         && (((__const uint32_t *) (a))[1] == ((__const uint32_t *) (b))[1])  \
         && (((__const uint32_t *) (a))[2] == ((__const uint32_t *) (b))[2])  \
         && (((__const uint32_t *) (a))[3] == ((__const uint32_t *) (b))[3]))

#define IN6_IS_ADDR_MC_NODELOCAL(a) \
        (IN6_IS_ADDR_MULTICAST(a)                                             \
         && ((((__const uint8_t *) (a))[1] & 0xf) == 0x1))

#define IN6_IS_ADDR_MC_LINKLOCAL(a) \
        (IN6_IS_ADDR_MULTICAST(a)                                             \
         && ((((__const uint8_t *) (a))[1] & 0xf) == 0x2))

#define IN6_IS_ADDR_MC_SITELOCAL(a) \
        (IN6_IS_ADDR_MULTICAST(a)                                             \
         && ((((__const uint8_t *) (a))[1] & 0xf) == 0x5))

#define IN6_IS_ADDR_MC_ORGLOCAL(a) \
        (IN6_IS_ADDR_MULTICAST(a)                                             \
         && ((((__const uint8_t *) (a))[1] & 0xf) == 0x8))

#define IN6_IS_ADDR_MC_GLOBAL(a) \
        (IN6_IS_ADDR_MULTICAST(a)                                             \
         && ((((__const uint8_t *) (a))[1] & 0xf) == 0xe))

#define IN6_ARE_ADDR_MASKED_EQUAL(a,b,m) \
           (((((__const uint32_t *) (a))[0] & (((__const uint32_t *) (m))[0])) == (((__const uint32_t *) (b))[0] & (((__const uint32_t *) (m))[0])))  \
         && ((((__const uint32_t *) (a))[1] & (((__const uint32_t *) (m))[1])) == (((__const uint32_t *) (b))[1] & (((__const uint32_t *) (m))[1])))  \
         && ((((__const uint32_t *) (a))[2] & (((__const uint32_t *) (m))[2])) == (((__const uint32_t *) (b))[2] & (((__const uint32_t *) (m))[2])))  \
         && ((((__const uint32_t *) (a))[3] & (((__const uint32_t *) (m))[3])) == (((__const uint32_t *) (b))[3] & (((__const uint32_t *) (m))[3]))))

#define IN_ARE_ADDR_MASKED_EQUAL(a,b,m) \
           (((((__const uint8_t *) (a))[0] & (((__const uint8_t *) (m))[0])) == (((__const uint8_t *) (b))[0] & (((__const uint8_t *) (m))[0])))  \
         && ((((__const uint8_t *) (a))[1] & (((__const uint8_t *) (m))[1])) == (((__const uint8_t *) (b))[1] & (((__const uint8_t *) (m))[1])))  \
         && ((((__const uint8_t *) (a))[2] & (((__const uint8_t *) (m))[2])) == (((__const uint8_t *) (b))[2] & (((__const uint8_t *) (m))[2])))  \
         && ((((__const uint8_t *) (a))[3] & (((__const uint8_t *) (m))[3])) == (((__const uint8_t *) (b))[3] & (((__const uint8_t *) (m))[3]))))


//---------------------------------------------------------------------------
// Find the IP traffic type (UNICAST, MULTICAST, BROADCAST)
traffic_type_t oai_nw_drv_find_traffic_type(struct sk_buff  *skb) {
  //---------------------------------------------------------------------------
  traffic_type_t            traffic_type = OAI_NW_DRV_IPVX_ADDR_TYPE_UNKNOWN;

  if (skb!=NULL) {
    switch (ntohs(skb->protocol))  {
    case ETH_P_IPV6:
      traffic_type = OAI_NW_DRV_IPV6_ADDR_TYPE_UNKNOWN;
      #ifdef OAI_DRV_DEBUG_CLASS
      printk("SOURCE ADDR %X:%X:%X:%X:%X:%X:%X:%X",NIP6ADDR(&(ipv6_hdr(skb)->saddr)));
      printk("    DEST   ADDR %X:%X:%X:%X:%X:%X:%X:%X\n",NIP6ADDR(&(ipv6_hdr(skb)->daddr)));
      #endif
      if (IN6_IS_ADDR_MULTICAST(&ipv6_hdr(skb)->daddr.in6_u.u6_addr32[0])) {
          traffic_type = OAI_NW_DRV_IPV6_ADDR_TYPE_MULTICAST;

      } else {
          traffic_type = OAI_NW_DRV_IPV6_ADDR_TYPE_UNICAST;
      }
      
      break;
      
      
    case ETH_P_IP:
      traffic_type = OAI_NW_DRV_IPV4_ADDR_TYPE_UNKNOWN;
#ifdef KERNEL_VERSION_GREATER_THAN_2622
      //print_TOOL_pk_ipv4((struct iphdr *)skb->network_header);
      if (IN_MULTICAST(htonl(ip_hdr(skb)->daddr))) {
          traffic_type = OAI_NW_DRV_IPV4_ADDR_TYPE_MULTICAST;
      } else {
          traffic_type = OAI_NW_DRV_IPV4_ADDR_TYPE_UNICAST;
      }
      // TO DO BROADCAST
      
#else
      //print_TOOL_pk_ipv4(skb->nh.iph);
      if (IN_MULTICAST(htonl(ip_hdr(skb)->daddr))) {
          traffic_type = OAI_NW_DRV_IPV4_ADDR_TYPE_MULTICAST;
      } else {
          traffic_type = OAI_NW_DRV_IPV4_ADDR_TYPE_UNICAST;
      }
      // TO DO BROADCAST
#endif
      break;
      
      case ETH_P_ARP:
          traffic_type = OAI_NW_DRV_IPV4_ADDR_TYPE_BROADCAST;
	  break;
      
    default:;
    }
  }
  return traffic_type;
}
