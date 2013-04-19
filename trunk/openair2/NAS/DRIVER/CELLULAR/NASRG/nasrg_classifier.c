/***************************************************************************
                          nasrg_classifier.c  -  description
 ***************************************************************************
  Eurecom OpenAirInterface 2
  Copyright(c) 1999 - 2013 Eurecom

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
  Address      : Eurecom, 450 route des Chappes, 06410 Biot Sophia Antipolis, France
*******************************************************************************/
/*! \file nasrg_classifier.c
* \brief Flow classification functions for OpenAirInterface CELLULAR version - RG
* \author  michelle.wetterwald, navid.nikaein, raymond.knopp, Lionel Gauthier
* \company Eurecom
* \email: michelle.wetterwald@eurecom.fr, raymond.knopp@eurecom.fr, navid.nikaein@eurecom.fr,  lionel.gauthier@eurecom.fr
*/
/*******************************************************************************/
#include "nasrg_variables.h"
#include "nasrg_proto.h"

#include <net/ip6_fib.h>
#include <net/route.h>

#define IN_CLASSA(a)            ((((long int) (a)) & 0x80000000) == 0)
#define IN_CLASSB(a)            ((((long int) (a)) & 0xc0000000) == 0x80000000)
#define IN_CLASSC(a)            ((((long int) (a)) & 0xe0000000) == 0xc0000000)
#define IN_CLASSD(a)            ((((long int) (a)) & 0xf0000000) == 0xe0000000)

/* Address to accept any incoming messages. */
#define INADDR_ANY              ((unsigned long int) 0x00000000)

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
void nasrg_create_mask_ipv6_addr(struct in6_addr *masked_addrP, int prefix_len){
//---------------------------------------------------------------------------
  int   u6_addr8_index;
  int   u6_addr1_index;
  int   index;

  masked_addrP->s6_addr32[0] = 0xFFFFFFFF;
  masked_addrP->s6_addr32[1] = 0xFFFFFFFF;
  masked_addrP->s6_addr32[2] = 0xFFFFFFFF;
  masked_addrP->s6_addr32[3] = 0xFFFFFFFF;

  u6_addr8_index = prefix_len >> 3;
  u6_addr1_index = prefix_len & 0x07;

  for (index = u6_addr8_index + 1; index < 16; index++) {
      masked_addrP->s6_addr[index] = 0;
  }
  if (u6_addr1_index > 0) {
    masked_addrP->s6_addr[u6_addr8_index] = 0xFF << (8-u6_addr1_index);
  }
  #ifdef NAS_DEBUG_CLASS
  //printk("nasrg_create_mask_ipv6_addr: MASK = %X:%X:%X:%X:%X:%X:%X:%X\n",NIP6ADDR(masked_addrP));
  #endif
}
//---------------------------------------------------------------------------
void nasrg_create_mask_ipv4_addr(struct in_addr *masked_addrP, int prefix_len){
//---------------------------------------------------------------------------
  if (prefix_len > 32) {
      prefix_len = 32;
  }
  masked_addrP->s_addr = 0xFFFFFFFF << (32 - prefix_len);
  #ifdef NAS_DEBUG_CLASS
  //printk("nasrg_create_mask_ipv4_addr: MASK = %d.%d.%d.%d\n",NIPADDR(masked_addrP));
  #endif
  return;
}

//---------------------------------------------------------------------------
// Add a new classifier rule (send direction)
struct classifier_entity *nasrg_CLASS_add_sclassifier(struct cx_entity *cx, u8 dscp, u16 classref){
//---------------------------------------------------------------------------
  struct classifier_entity *gc;

  #ifdef NAS_DEBUG_CLASS
  printk("nasrg_CLASS_add_sclassifier: begin for dscp %d, classref %d\n", dscp,classref);
  #endif
  if (cx==NULL){
     printk("nasrg_CLASS_add_sclassifier - input parameter cx is NULL \n");
    return NULL;
  }
//***
  for (gc=cx->sclassifier[dscp]; gc!=NULL; gc=gc->next){
    if (gc->classref==classref){
      #ifdef NAS_DEBUG_CLASS
      printk("nasrg_CLASS_add_sclassifier: classifier already exist for dscp %d, classref %d\n",dscp,classref);
      #endif
      return gc;
    }
  }
  gc=(struct classifier_entity *)kmalloc(sizeof(struct classifier_entity), GFP_KERNEL);
  if (gc==NULL)
    return NULL;
  memset(gc, 0, sizeof(struct classifier_entity));
  gc->next=cx->sclassifier[dscp];
  gc->classref=classref;
  cx->sclassifier[dscp]=gc;
  ++cx->nsclassifier;
  ++gpriv->next_sclassref; //increment send classref index - MW 15/01/07
  #ifdef NAS_DEBUG_CLASS
  printk("nasrg_CLASS_add_sclassifier: classifier created for dscp %d, classref %d\n",dscp,classref);
  #endif
  return gc;
}

//---------------------------------------------------------------------------
// Add a new classifier rule (receive direction)
struct classifier_entity *nasrg_CLASS_add_rclassifier(u8 dscp, u16 classref){
//---------------------------------------------------------------------------
  struct classifier_entity *gc;

  #ifdef NAS_DEBUG_CLASS
  printk("nasrg_CLASS_add_rclassifier: begin\n");
  #endif
//***
  for (gc=gpriv->rclassifier[dscp]; gc!=NULL; gc=gc->next)
  {
    if (gc->classref==classref){
      #ifdef NAS_DEBUG_CLASS
      printk("nasrg_CLASS_add_rclassifier: classifier already exist for dscp %d, classref %d\n",dscp,classref);
      #endif
      return gc;
    }
  }
  gc=(struct classifier_entity *)kmalloc(sizeof(struct classifier_entity), GFP_KERNEL);
  if (gc==NULL)
    return NULL;
  gc->next=gpriv->rclassifier[dscp];
  gc->classref=classref;
  gpriv->rclassifier[dscp]=gc;
  ++gpriv->nrclassifier;
  #ifdef NAS_DEBUG_CLASS
  printk("nasrg_CLASS_add_rclassifier: classifier created for dscp %d, classref %d\n",dscp,classref);
  #endif
  return gc;
}

//---------------------------------------------------------------------------
// Add a new classifier rule (mbms direction)
struct classifier_entity *nasrg_CLASS_add_mbmsclassifier(int mbms_ix, u16 classref){
//---------------------------------------------------------------------------
  struct classifier_entity *gc;

  #ifdef NAS_DEBUG_CLASS
  printk("nasrg_CLASS_add_mbmsclassifier: begin\n");
  #endif
//***
  for (gc=gpriv->mbmsclassifier[mbms_ix]; gc!=NULL; gc=gc->next)
  {
    if (gc->classref==classref){
      #ifdef NAS_DEBUG_CLASS
      printk("nasrg_CLASS_add_mbmsclassifier: classifier already exist for service %d, classref %d\n",mbms_ix,classref);
      #endif
      return gc;
    }
  }
  gc=(struct classifier_entity *)kmalloc(sizeof(struct classifier_entity), GFP_KERNEL);
  if (gc==NULL)
    return NULL;
  gc->next=gpriv->mbmsclassifier[mbms_ix];
  gc->classref=classref;
  gpriv->mbmsclassifier[mbms_ix]=gc;
  ++gpriv->nmbmsclassifier;
  #ifdef NAS_DEBUG_CLASS
  printk("nasrg_CLASS_add_mbmsclassifier: classifier created for service index %d, classref %d\n",mbms_ix,classref);
  #endif
  return gc;
}

//---------------------------------------------------------------------------
void nasrg_CLASS_flush_sclassifier(struct cx_entity *cx){
//---------------------------------------------------------------------------
  u8 dscpi;
  struct classifier_entity *gc;

  #ifdef NAS_DEBUG_CLASS
  printk("nasrg_CLASS_flush_sclassifier: begin\n");
  #endif
  if (cx==NULL){
     printk("nasrg_CLASS_flush_sclassifier - input parameter cx is NULL \n");
    return;
  }
//***
  for (dscpi=0; dscpi<NAS_DSCP_MAX; ++dscpi){
    for (gc=cx->sclassifier[dscpi]; gc!=NULL; gc=cx->sclassifier[dscpi]){
      cx->sclassifier[dscpi]=gc->next;
      kfree(gc);
    }
  }
  cx->nsclassifier=0;
  #ifdef NAS_DEBUG_CLASS
  printk("nasrg_CLASS_flush_sclassifier: end\n");
  #endif
}

//---------------------------------------------------------------------------
void nasrg_CLASS_flush_rclassifier(){
//---------------------------------------------------------------------------
  u8 dscpi;
  struct classifier_entity *gc;

  #ifdef NAS_DEBUG_CLASS
  printk("nasrg_CLASS_flush_rclassifier: begin\n");
  #endif
//***
  for (dscpi=0; dscpi<NAS_DSCP_MAX; ++dscpi){
    for (gc=gpriv->rclassifier[dscpi]; gc!=NULL; gc=gpriv->rclassifier[dscpi]){
      gpriv->rclassifier[dscpi]=gc->next;
      kfree(gc);
    }
  }
  gpriv->nrclassifier=0;
  #ifdef NAS_DEBUG_CLASS
  printk("nasrg_CLASS_flush_rclassifier: end\n");
  #endif
}

//---------------------------------------------------------------------------
void nasrg_CLASS_flush_mbmsclassifier(){
//---------------------------------------------------------------------------
  int mbmsi;
  struct classifier_entity *gc;

  #ifdef NAS_DEBUG_CLASS
  printk("nasrg_CLASS_flush_mbmsclassifier: begin\n");
  #endif
//***
  for (mbmsi=0; mbmsi<NASRG_MBMS_SVCES_MAX; ++mbmsi){
    for (gc=gpriv->mbmsclassifier[mbmsi]; gc!=NULL; gc=gpriv->mbmsclassifier[mbmsi]){
      gpriv->mbmsclassifier[mbmsi]=gc->next;
      kfree(gc);
    }
  }
  gpriv->nmbmsclassifier=0;
  #ifdef NAS_DEBUG_CLASS
  printk("nasrg_CLASS_flush_mbmsclassifier: end\n");
  #endif
}

//---------------------------------------------------------------------------
// Delete a classifier rule (send direction)
void nasrg_CLASS_del_sclassifier(struct cx_entity *cx, u8 dscp, u16 classref){
//---------------------------------------------------------------------------
  struct classifier_entity *p,*np;

  #ifdef NAS_DEBUG_CLASS
  printk("nasrg_CLASS_del_sclassifier: begin\n");
  #endif
  if (cx==NULL){
     printk("nasrg_CLASS_del_sclassifier - input parameter cx is NULL \n");
    return;
  }
//***
  p=cx->sclassifier[dscp];
  if (p==NULL)
    return;
  if (p->classref==classref){
    cx->sclassifier[dscp]=p->next;
    kfree(p);
    --cx->nsclassifier;
    return;
  }
  for (np=p->next; np!=NULL; p=np){
    if (np->classref==classref){
      p->next=np->next;
      kfree(np);
      --cx->nsclassifier;
      return;
    }
  }
  #ifdef NAS_DEBUG_CLASS
  printk("nasrg_CLASS_del_sclassifier: end\n");
  #endif
}

//---------------------------------------------------------------------------
// Delete a classifier rule (receive direction)
void nasrg_CLASS_del_rclassifier(u8 dscp, u16 classref){
//---------------------------------------------------------------------------
  struct classifier_entity *p,*np;

  #ifdef NAS_DEBUG_CLASS
  printk("nasrg_CLASS_del_rclassifier: begin\n");
  #endif
//***
  p=gpriv->rclassifier[dscp];
  if (p==NULL)
    return;
  if (p->classref==classref){
    gpriv->rclassifier[dscp]=p->next;
    kfree(p);
    --gpriv->nrclassifier;
    return;
  }
  for (np=p->next; np!=NULL; p=np){
    if (np->classref==classref){
      p->next=np->next;
      kfree(np);
      --gpriv->nrclassifier;
      return;
    }
  }
  #ifdef NAS_DEBUG_CLASS
  printk("nasrg_CLASS_del_rclassifier: end\n");
  #endif
}

//---------------------------------------------------------------------------
// Delete a classifier rule (mbms direction)
void nasrg_CLASS_del_mbmsclassifier(int mbms_ix, u16 classref){
//---------------------------------------------------------------------------
  struct classifier_entity *p,*np;

  #ifdef NAS_DEBUG_CLASS
  printk("nasrg_CLASS_del_mbmsclassifier: begin\n");
  #endif
//***
  p=gpriv->mbmsclassifier[mbms_ix];
  if (p==NULL)
    return;
  if (p->classref==classref){
    gpriv->mbmsclassifier[mbms_ix]=p->next;
    kfree(p);
    --gpriv->nmbmsclassifier;
    return;
  }
  for (np=p->next; np!=NULL; p=np){
    if (np->classref==classref){
      p->next=np->next;
      kfree(np);
      --gpriv->nmbmsclassifier;
      return;
    }
  }
  #ifdef NAS_DEBUG_CLASS
  printk("nasrg_CLASS_del_mbmsclassifier: end\n");
  #endif
}

/*  ORIGINAL VERSION
//---------------------------------------------------------------------------
// Search the entity with the IPv4 address 'addr'
struct cx_entity *nasrg_CLASS_cx4(struct sk_buff *skb){
//---------------------------------------------------------------------------
  u8 cxi;
  #ifdef NAS_DEBUG_CLASS
  printk("nasrg_CLASS_cx4: begin\n");
  #endif
  cxi=0;
  return gpriv->cx+cxi;
//#endif
}*/

//---------------------------------------------------------------------------
// Search the entity with the IPv4 address 'addr'
struct cx_entity *nasrg_CLASS_cx4(struct sk_buff *skb, unsigned char dscp, int *paddr_type, unsigned char *cx_index) {
  //---------------------------------------------------------------------------
  unsigned char cxi;
  u32 daddr;
  struct cx_entity *cx=NULL;
  struct classifier_entity *pclassifier=NULL;
  struct in_addr masked_addr;

  #ifdef NAS_DEBUG_CLASS
  printk("nasrg_CLASS_cx4: begin\n");
  #endif
  if (skb!=NULL) {
    daddr = ((struct iphdr*)(skb_network_header(skb)))->daddr;
    if (daddr != INADDR_ANY) {
      #ifdef NAS_DEBUG_CLASS
      printk("SOURCE ADDR %d.%d.%d.%d",NIPADDR(ip_hdr(skb)->saddr));
      printk(" DEST ADDR %d.%d.%d.%d\n",NIPADDR(ip_hdr(skb)->daddr));
      #endif
      if (ipv4_is_multicast(ip_hdr(skb)->daddr)) {
        // TO BE CHECKED
        *paddr_type = NAS_IPV4_ADDR_MC_SIGNALLING;
      } else {
        if (ipv4_is_lbcast(ip_hdr(skb)->daddr)) {
        // TO BE CHECKED
        *paddr_type = NAS_IPV4_ADDR_BROADCAST;
        } else {
          if (IN_CLASSA(ip_hdr(skb)->daddr) || IN_CLASSB(ip_hdr(skb)->daddr) || IN_CLASSC(ip_hdr(skb)->daddr)) {
            *paddr_type = NAS_IPV4_ADDR_UNICAST;
            for (cxi=*cx_index; cxi<NAS_CX_MAX; ++cxi) {
              (*cx_index)++;
              pclassifier = gpriv->cx[cxi].sclassifier[dscp];
              while (pclassifier!=NULL) {
                // verify that this is an IPv4 classifier
                if ((pclassifier->version == NAS_VERSION_4)  || (pclassifier->version == NAS_VERSION_DEFAULT)) {
                  nasrg_create_mask_ipv4_addr(&masked_addr, pclassifier->dplen);
                  if (IN_ARE_ADDR_MASKED_EQUAL(&ip_hdr(skb)->daddr, &(pclassifier->daddr.ipv4), &masked_addr)) {
                    #ifdef NAS_DEBUG_CLASS
                    printk("nasrg_CLASS_cx4: IP MASK MATCHED: found cx %d: %d.%d.%d.%d/%d\n",cxi, NIPADDR(pclassifier->daddr.ipv4), pclassifier->dplen);
                    #endif
                  return &gpriv->cx[cxi];
                  }
                }
                // goto to next classification rule for the connection
                pclassifier = pclassifier->next;
              }
            }
          } else {
            *paddr_type = NAS_IPV4_ADDR_UNKNOWN;
          }
        }
      }
    }
  }
  return cx;
}

/*  ORIGINAL VERSION
//---------------------------------------------------------------------------
// Search the entity corresponding to destination address in IPv6 header
struct cx_entity *nasrg_CLASS_cx6(struct sk_buff *skb, int* paddr_type, int* pmbms_ix){
//---------------------------------------------------------------------------
  struct cx_entity * cx=NULL;
  u8 cxi;
  u32 mc_addr_hdr, uni_ifid1, uni_ifid2;
  //int addr_type = NASRG_ADDR_UNKNOWN;

  #ifdef NAS_DEBUG_CLASS
  printk("nasrg_CLASS_cx6: begin\n");
  #endif
  if (!skb){
     printk("nasrg_CLASS_cx6 - input parameter skb is NULL \n");
    return cx;
  }
  *paddr_type = NASRG_ADDR_UNKNOWN;
  //mc_addr_hdr = ntohl(skb->nh.ipv6h->daddr.in6_u.u6_addr32[0]);
  mc_addr_hdr = ntohl(ipv6_hdr(skb)->daddr.in6_u.u6_addr32[0]);
  // First check if multicast [1st octet is FF]
  if ((mc_addr_hdr & 0xFF000000) == 0xFF000000) {
     // packet type according to the scope of the multicast packet
     // we don't consider RPT bits in second octet [maybe done later if needed]
      switch(mc_addr_hdr & 0x000F0000) {
       case (0x00020000):
         *paddr_type = NASRG_ADDR_MC_SIGNALLING;
         #ifdef NAS_DEBUG_CLASS
         printk("nasrg_CLASS_cx6: multicast packet - signalling \n");
         #endif
         break;
       case (0x000E0000):
         *paddr_type = NASRG_ADDR_MC_MBMS;
         *pmbms_ix = 0;
         cx=gpriv->cx;  // MBMS associate to Mobile 0
         #ifdef NAS_DEBUG_CLASS
         printk("nasrg_CLASS_cx6: multicast packet - MBMS data \n");
         #endif
         break;
       default:
         *paddr_type = NASRG_ADDR_UNKNOWN;
         *pmbms_ix = NASRG_MBMS_SVCES_MAX;
     }
  // This is not multicast, so we should be able to identify the MT
  }else{
     #ifdef NAS_DEBUG_CLASS
     printk("nasrg_CLASS_cx6: unicast packet\n");
     #endif
     *paddr_type = NASRG_ADDR_UNICAST;
     uni_ifid1 = ntohl(ipv6_hdr(skb)->daddr.in6_u.u6_addr32[2]);
     uni_ifid2 = ntohl(ipv6_hdr(skb)->daddr.in6_u.u6_addr32[3]);
     for (cxi=0; cxi<NAS_CX_MAX; cxi++){
       cx=gpriv->cx+cxi;
       #ifdef NAS_DEBUG_SEND_DETAIL
       printk("nasrg_CLASS_cx6: Compared addresses \n");
       printk("                Daddr[2] %ul, Daddr[3] %ul\n",
                 ipv6_hdr(skb)->daddr.in6_u.u6_addr32[2],ipv6_hdr(skb)->daddr.in6_u.u6_addr32[3]);
       printk("          ntohl Daddr[2] %ul, Daddr[3] %ul\n",uni_ifid1,uni_ifid2);
       printk("                IIF[0]   %ul, IIF[1]   %ul\n",cx->iid6[0],cx->iid6[1]);
       printk("          htonl IIF[0]   %ul, IIF[1]   %ul\n",htonl(cx->iid6[0]),htonl(cx->iid6[1]));
       #endif
       if (((cx->iid6[0] == uni_ifid1)&&(cx->iid6[1] == uni_ifid2))
              || ((htonl(cx->iid6[0]) == uni_ifid1)&&(htonl(cx->iid6[1]) == uni_ifid2))){
         return cx;
       }
     }
  }
  return cx;
}*/

//---------------------------------------------------------------------------
// Search the entity with the IPv6 address 'addr'
// Navid: the ipv6 classifier is not fully tested
struct cx_entity *nasrg_CLASS_cx6(struct sk_buff *skb, unsigned char dscp, int *paddr_type, unsigned char *cx_index, int* pmbms_ix) {
  //---------------------------------------------------------------------------
  u8 cxi;
  struct cx_entity *cx = NULL;
  struct classifier_entity *sclassifier= NULL;
  u32 mc_addr_hdr;
  struct in6_addr masked_addr;

  #ifdef NAS_DEBUG_CLASS
  printk("nasrg_CLASS_cx6: begin\n");
  #endif
  if (skb) {
    #ifdef NAS_DEBUG_CLASS
    printk("nasrg_CLASS_cx6: SOURCE ADDR %X:%X:%X:%X:%X:%X:%X:%X",NIP6ADDR(&(ipv6_hdr(skb)->saddr)));
    printk(" DEST ADDR %X:%X:%X:%X:%X:%X:%X:%X\n",NIP6ADDR(&(ipv6_hdr(skb)->daddr)));
    #endif
    mc_addr_hdr = ntohl(ipv6_hdr(skb)->daddr.in6_u.u6_addr32[0]);
    // First check if multicast [1st octet is FF]
    if ((mc_addr_hdr & 0xFF000000) == 0xFF000000) {
      // packet type according to the scope of the multicast packet
      // we don't consider RPT bits in second octet [maybe done later if needed]
      switch(mc_addr_hdr & 0x000F0000) {
        case (0x00020000):
          *paddr_type = NAS_IPV6_ADDR_MC_SIGNALLING;
          #ifdef NAS_DEBUG_CLASS
          printk("nasrg_CLASS_cx6: multicast packet - signalling \n");
          #endif
          break;
        case (0x000E0000):
          *paddr_type = NAS_IPV6_ADDR_MC_MBMS;
          *pmbms_ix = 0;
          cx=gpriv->cx;  // MBMS associate to Mobile 0
          #ifdef NAS_DEBUG_CLASS
          printk("nasrg_CLASS_cx6: multicast packet - MBMS data \n");
          #endif
          break;
        default:
          printk("nasrg_CLASS_cx6: default multicast\n");
          *paddr_type = NAS_IPV6_ADDR_UNKNOWN;
          *pmbms_ix = NASRG_MBMS_SVCES_MAX;
      }
    } else {
      // This is not multicast, so we should be able to identify the MT
      *paddr_type = NAS_IPV6_ADDR_UNICAST;
      for (cxi=*cx_index; cxi<NAS_CX_MAX; cxi++) {
        //cxi = 0;
        (*cx_index)++;
        sclassifier = gpriv->cx[cxi].sclassifier[dscp];
  
        while (sclassifier!=NULL) {
          // verify that this is an IPv6 classifier
          if ((sclassifier->version == NAS_VERSION_6) || (sclassifier->version == NAS_VERSION_DEFAULT)) {
            /*LGif (IN6_IS_ADDR_UNSPECIFIED(&(sclassifier->daddr.ipv6))) {
              printk("nas_CLASS_cx6: addr is null \n");
              sclassifier = sclassifier->next;
              continue;
              }*/
            #ifdef NAS_DEBUG_CLASS
            printk("cx %d : DSCP %d %X:%X:%X:%X:%X:%X:%X:%X\n",cxi, dscp, NIP6ADDR(&(sclassifier->daddr.ipv6)));
            #endif //NAS_DEBUG_CLASS
            //if ((dst = (unsigned int*)&(((struct rt6_info *)skbdst)->rt6i_gateway)) == 0){
            // LG: STRANGE
            if (IN6_IS_ADDR_UNSPECIFIED(&ipv6_hdr(skb)->daddr)) {
              printk("nasrg_CLASS_cx6: dst addr is null \n");
              sclassifier = sclassifier->next;
              continue;
            }
            nasrg_create_mask_ipv6_addr(&masked_addr, sclassifier->dplen);
            // Modified MW to check only the iid6
            masked_addr.s6_addr32[0] = 0x00000000;
            masked_addr.s6_addr32[1] = 0x00000000;

            if (IN6_ARE_ADDR_MASKED_EQUAL(&ipv6_hdr(skb)->daddr, &(sclassifier->daddr.ipv6), &masked_addr)) {
              #ifdef NAS_DEBUG_CLASS
              printk("nasrg_CLASS_cx6: found cx %d: %X:%X:%X:%X:%X:%X:%X:%X\n",cxi, NIP6ADDR(&(sclassifier->daddr.ipv6)));
              #endif //NAS_DEBUG_CLASS
              return &gpriv->cx[cxi];
            }
          }
          // Go to next classifier entry for connection
          sclassifier = sclassifier->next;
        }
      }
    }
  }
  //printk("nas_CLASS_cx6 NOT FOUND: %X:%X:%X:%X:%X:%X:%X:%X\n",NIP6ADDR(&ipv6_hdr(skb)->daddr));
  return cx;
}

//---------------------------------------------------------------------------
// Search the sending function for IP Packet
void nasrg_CLASS_send(struct sk_buff *skb){
//---------------------------------------------------------------------------
  struct classifier_entity  *pclassifier, *sp;
  u8 *protocolh = NULL;
  u8 version;
  u8 protocol, dscp;
  u16 classref;
  struct cx_entity *cx;
  unsigned int i;
  #ifdef NAS_DEBUG_CLASS
  char sfct[10], sprotocol[10];
  #endif
  struct net_device *dev = gdev;
  unsigned char cx_index,no_connection;
  int addr_type;
  int mbms_ix;
  struct in6_addr masked6_addr;
  struct in_addr  masked_addr;
  // RARP vars
  struct arphdr  *rarp;
  unsigned char  *rarp_ptr;
   /* s for "source", t for "target" */
  __be32 sip, tip;
  unsigned char *sha, *tha;

  #ifdef NAS_DEBUG_CLASS
  printk("nasrg_CLASS_send: begin -\n");
  #endif
  if (skb==NULL){
     printk("nasrg_CLASS_send - input parameter skb is NULL \n");
    return;
  }
//***
  #ifdef NAS_DEBUG_SEND
  printk("nasrg_CLASS_send - Received IP packet to transmit, length %d", skb->len);
  if ((skb->data) != NULL){
    if (skb->len<150)
     nasrg_TOOL_print_buffer(skb->data,skb->len);
    else
     nasrg_TOOL_print_buffer(skb->data,150);
  }
  #endif
//***
  // find all connections related to socket
  cx_index   = 0;
  no_connection = 1;
  cx = NULL;
  #ifdef NAS_DEBUG_CLASS
  printk("nasrg_CLASS_send: [before switch on IP protocol version] \n");
  #endif


  // Get mobile connexion entity, protocol and dscp from IP packet
  switch (ntohs(skb->protocol)) {
    case ETH_P_IPV6:
      #ifdef NAS_DEBUG_CLASS
      printk("nasrg_CLASS_send : skb->protocol : IPv6 \n");
      #endif
      version = NAS_VERSION_6;
      addr_type = NAS_IPV6_ADDR_UNKNOWN;
      protocolh = nasrg_TOOL_get_protocol6(ipv6_hdr(skb), &protocol);
      dscp      = nasrg_TOOL_get_dscp6 (ipv6_hdr(skb));
      cx        = nasrg_CLASS_cx6 (skb, dscp, &addr_type, &cx_index, &mbms_ix);
      #ifdef NAS_DEBUG_CLASS
      printk("nasrg_CLASS_send - ETH_P_IPV6 skb %p dscp %d gpriv %p cx_index %p \n",skb, dscp, gpriv, &cx_index);
      #endif
      // find in default DSCP a valid classification
      if (cx == NULL) {
        switch (addr_type) {
          case NAS_IPV6_ADDR_MC_SIGNALLING:
          case NAS_IPV6_ADDR_UNICAST:
            #ifdef NAS_DEBUG_CLASS
            printk("nasrg_CLASS_send - case NAS_IPV6_ADDR_MC_SIGNALLING | NAS_IPV6_ADDR_UNICAST\n");
            #endif //NAS_DEBUG_CLASS
            for (i=0; i<NAS_CX_MAX; i++){
              pclassifier=(&gpriv->cx[i])->sclassifier[NAS_DSCP_DEFAULT];
              while (pclassifier!=NULL) {
                if ((pclassifier->version == NAS_VERSION_6) || (pclassifier->version == NAS_VERSION_DEFAULT)) {
                  // ok found default classifier for this packet
                  nasrg_create_mask_ipv6_addr(&masked6_addr, pclassifier->dplen);
                  // Modified MW to let everything go (pb with signalling)
                  masked6_addr.s6_addr32[0] = 0x00000000;
                  masked6_addr.s6_addr32[1] = 0x00000000;
                  #ifdef NAS_DEBUG_CLASS
                  printk("nasrg_CLASS_send - cx %d : DSCP NAS_DSCP_DEFAULT %X:%X:%X:%X:%X:%X:%X:%X\n",i, NIP6ADDR(&(pclassifier->daddr.ipv6)));
                  #endif //NAS_DEBUG_CLASS

                  if (IN6_ARE_ADDR_MASKED_EQUAL(&pclassifier->daddr.ipv6, &ipv6_hdr(skb)->daddr, &masked6_addr)) {
                    // then force dscp
                    cx = &gpriv->cx[i];
                    #ifdef NAS_DEBUG_CLASS
                    printk("nasrg_CLASS_send - ETH_P_IPV6 FOUND NAS_DSCP_DEFAULT with IN6_ARE_ADDR_MASKED_EQUAL(%d bits)\n",pclassifier->dplen);
                    #endif
                    dscp = NAS_DSCP_DEFAULT;
                    break;
                  } else {
                    if(IN6_IS_ADDR_UNSPECIFIED(&pclassifier->daddr.ipv6)) {
                      cx = &gpriv->cx[i];
                      #ifdef NAS_DEBUG_CLASS
                      printk("nasrg_CLASS_send - ETH_P_IPV6 FOUND NAS_DSCP_DEFAULT with IN6_IS_ADDR_UNSPECIFIED\n");
                      #endif
                      dscp = NAS_DSCP_DEFAULT;
                      break;
                    }
                  }
                }
                pclassifier = pclassifier->next;
              }
            }
            break;
          // MBMS is broken!!!! To be updated (these values will be over-ridden afterwards
          case NAS_IPV6_ADDR_MC_MBMS:
            #ifdef NAS_DEBUG_CLASS
            printk("nasrg_CLASS_send - case NAS_IPV6_ADDR_MC_MBMS\n");
            #endif //NAS_DEBUG_CLASS
            pclassifier = gpriv->mbmsclassifier[mbms_ix];
            printk("nasrg_CLASS_send: MBMS is broken!!!!\n\n\n");
            sp = gpriv->mbmsclassifier[mbms_ix];
            if (sp!= NULL){
              classref=sp->classref;
              #ifdef NAS_DEBUG_SEND_DETAIL
              printk("nasrg_CLASS_send: classifier found for multicast service %d \n", mbms_ix);
              #endif
            }else{
              printk("nasrg_CLASS_send: No corresponding multicast bearer, so the message is dropped\n");
              return;
            }
            break;
          // should have found a valid classification rule
          case NAS_IPV6_ADDR_UNKNOWN:
          default:
            printk("nasrg_CLASS_send: No corresponding address type\n");
        }
      }
      break;

    case ETH_P_ARP:
      #ifdef NAS_DEBUG_CLASS
      printk("nasrg_CLASS_send : skb->protocol : ARP \n");
      #endif
      version = NAS_VERSION_4;
      addr_type = NAS_IPV4_ADDR_BROADCAST;
      dscp = 0;
      cx = NULL;
      // Basic sanity checks can be done without the lock
      rarp = (struct arphdr *)skb_network_header(skb);
      if (rarp) {
         if (rarp->ar_hln != dev->addr_len || dev->type != ntohs(rarp->ar_hrd)) {
            printk("nasrg_CLASS_send: ARP PACKET WRONG ADDR LEN or WRONG ARP HEADER TYPE\n");
                    break;
         }
      } else {
         printk("nasrg_CLASS_send: ARP HEADER POINTER IS NULL\n");
         break;
      }
      // If it's not Ethernet, delete it.
      if (rarp->ar_pro != htons(ETH_P_IP)) {
         printk("nasrg_CLASS_send: ARP PACKET PROTOCOL IS NOT ETHERNET\n");
         break;
      }
      rarp_ptr = (unsigned char *) (rarp + 1);
      sha = rarp_ptr;
      rarp_ptr += dev->addr_len;
      memcpy(&sip, rarp_ptr, 4);
      rarp_ptr += 4;
      tha = rarp_ptr;
      rarp_ptr += dev->addr_len;
      memcpy(&tip, rarp_ptr, 4);
      #ifdef NAS_DEBUG_CLASS
      printk("nasrg_CLASS_send: ARP DEST IP transport IP = %d.%d.%d.%d\n",NIPADDR(tip));
      #endif
      for (i=0; i<NAS_CX_MAX; i++){
        pclassifier=(&gpriv->cx[i])->sclassifier[NAS_DSCP_DEFAULT];
        while (pclassifier!=NULL) {
          if ((pclassifier->version == NAS_VERSION_4) || (pclassifier->version == NAS_VERSION_DEFAULT)) {
          // ok found default classifier for this packet
          nasrg_create_mask_ipv4_addr(&masked_addr, pclassifier->dplen);
          #ifdef NAS_DEBUG_CLASS
          printk("nasrg_CLASS_send: MASK = %d.%d.%d.%d\n",NIPADDR(masked_addr.s_addr));
          #endif
          //
            if (IN_ARE_ADDR_MASKED_EQUAL(&pclassifier->daddr.ipv4, &tip, &masked_addr.s_addr)) {
              // then force dscp
              cx = &gpriv->cx[i];
              #ifdef NAS_DEBUG_CLASS
              printk("nasrg_CLASS_send: ETH_P_ARP FOUND NAS_DSCP_DEFAULT with IN_ARE_ADDR_MASKED_EQUAL(%d bits)\n", pclassifier->dplen);
              #endif
              dscp = NAS_DSCP_DEFAULT;
              break;
            } else {
              if (INADDR_ANY == pclassifier->daddr.ipv4) {
                cx = &gpriv->cx[i];
                #ifdef NAS_DEBUG_CLASS
                printk("nasrg_CLASS_send: ETH_P_ARP FOUND NAS_DSCP_DEFAULT with INADDR_ANY\n");
                #endif
                dscp = NAS_DSCP_DEFAULT;
                break;
              }
            }
          }
          pclassifier = pclassifier->next;
        }
      }
      break;

    case ETH_P_IP:
      #ifdef NAS_DEBUG_CLASS
      printk("nasrg_CLASS_send : skb->protocol : IPv4 \n");
      #endif
      version   = NAS_VERSION_4;
      addr_type = NAS_IPV4_ADDR_UNKNOWN;
      dscp      = nasrg_TOOL_get_dscp4((struct iphdr *)(skb_network_header(skb)));
      cx        = nasrg_CLASS_cx4(skb, dscp, &addr_type, &cx_index);
      protocolh = nasrg_TOOL_get_protocol4((struct iphdr *)(skb_network_header(skb)), &protocol);
      // find in default DSCP a valid classification
      if (cx == NULL) {
        switch (addr_type) {
          case NAS_IPV4_ADDR_MC_SIGNALLING:
          case NAS_IPV4_ADDR_UNICAST:
          case NAS_IPV4_ADDR_BROADCAST:
            for (i=0; i<NAS_CX_MAX; i++){
              pclassifier=(&gpriv->cx[i])->sclassifier[NAS_DSCP_DEFAULT];
              while (pclassifier != NULL) {
                if ((pclassifier->version == NAS_VERSION_4) || (pclassifier->version == NAS_VERSION_DEFAULT)) {
                  // ok found default classifier for this packet
                  nasrg_create_mask_ipv4_addr(&masked_addr, pclassifier->dplen);
                  #ifdef NAS_DEBUG_CLASS
                  printk("nasrg_CLASS_send : MASK = %d.%d.%d.%d\n", NIPADDR(masked_addr.s_addr));
                  #endif
                  if (IN_ARE_ADDR_MASKED_EQUAL(&pclassifier->daddr.ipv4, &ip_hdr(skb)->daddr, &masked_addr.s_addr)) {
                    // then force dscp
                    cx = &gpriv->cx[i];
                    #ifdef NAS_DEBUG_CLASS
                    printk("nasrg_CLASS_send : ETH_P_IP FOUND NAS_DSCP_DEFAULT with IN_ARE_ADDR_MASKED_EQUAL(%d bits)\n",pclassifier->dplen);
                    #endif
                    dscp = NAS_DSCP_DEFAULT;
                    break;
                  } else {
                    if(INADDR_ANY == pclassifier->daddr.ipv4) {
                      cx = &gpriv->cx[i];
                      #ifdef NAS_DEBUG_CLASS
                      printk("nasrg_CLASS_send : ETH_P_IP FOUND NAS_DSCP_DEFAULT with INADDR_ANY\n");
                      #endif
                      dscp = NAS_DSCP_DEFAULT;
                      break;
                    }
                  }
                }
                pclassifier = pclassifier->next;
              }
            }
            break;
            // should have found a valid classification rule
          case NAS_IPV4_ADDR_UNKNOWN:
          default:
            printk("nasrg_CLASS_send: No corresponding address type\n");
        }
      }
      #ifdef NAS_DEBUG_CLASS
      if (cx)
        printk("nasrg_CLASS_send: ETH_P_IP Received IPv4 packet (%02X), dscp = %d, cx = %d\n",ntohs(skb->protocol),dscp,cx->lcr);
      else
        printk("nasrg_CLASS_send: ETH_P_IP Received IPv4 packet (%02X), dscp = %d, No valid connection\n",ntohs(skb->protocol),dscp);
      #endif
     break;

    default:
      printk("nasrg_CLASS_send: Unknown IP version protocol\n");
      version = 0;
      return;
  }
  #ifdef NAS_DEBUG_SEND_DETAIL
  printk("nasrg_CLASS_send: [before if (cx != NULL)]\n");
  #endif

  //Next lines bypass classifiers to test the netlink socket
  //#define DEBUG_NETLINKRG_TEST
  #ifdef DEBUG_NETLINKRG_TEST
  nasrg_COMMON_QOS_send_test_netlink(skb);
  return;
  #endif

  // If a valid connection for the DSCP/EXP with destination address
  // is found scan all protocol-based classification rules
  if (cx != NULL) {
    classref = 0;
    sp       = NULL;
    if (addr_type==NAS_IPV6_ADDR_MC_MBMS){
      sp = gpriv->mbmsclassifier[mbms_ix];
      if (sp!= NULL){
        classref=sp->classref;
        #ifdef NAS_DEBUG_SEND_DETAIL
        printk("nasrg_CLASS_send: classifier found for multicast service %d \n", mbms_ix);
        #endif
      }else{
        printk("nasrg_CLASS_send: No corresponding multicast bearer, so the message is dropped\n");
        return;
      }
    }else{
     #ifdef NAS_DEBUG_CLASS
      printk("nasrg_CLASS_send: DSCP %d version %d: looking for classifier entry\n",dscp, version);
      #endif
      for (pclassifier=cx->sclassifier[dscp]; pclassifier!=NULL; pclassifier=pclassifier->next) {
        #ifdef NAS_DEBUG_CLASS
        printk("nasrg_CLASS_send: DSCP %d p->classref=%d,p->protocol=%d,p->version=%d\n",dscp,pclassifier->classref,pclassifier->protocol,pclassifier->version);
        #endif
        // normal rule checks that network protocol version matches
        if ((pclassifier->version == version)  || (pclassifier->version == NAS_VERSION_DEFAULT)){
            //printk("nasrg_CLASS_send: IP version are equals\n");
            sp=pclassifier;
            classref=sp->classref;
            #ifdef NAS_DEBUG_SEND_DETAIL
            printk("nasrg_CLASS_send: classifier found for dscp %u \n", dscp);
            #endif
            break;
        }
      }
    }

    if (sp!=NULL) {
      #ifdef NAS_DEBUG_CLASS
      //char sfct[10], sprotocol[10];
      // classifier entity found. Print its parameters
      if (sp->fct==nasrg_COMMON_QOS_send)
        strcpy(sfct, "data xfer");
      if (sp->fct==nasrg_CTL_send)
        strcpy(sfct, "iocontrol");
      if (sp->fct==nasrg_COMMON_del_send)
        strcpy(sfct, "delete");
      if (sp->fct==nasrg_ASCTL_DC_send_sig_data_request)
        strcpy(sfct, "DC-SAP");

      switch(protocol) {
        case NAS_PROTOCOL_UDP:strcpy(sprotocol, "udp");printk("udp packet\n");break;
        case NAS_PROTOCOL_TCP:strcpy(sprotocol, "tcp");printk("tcp packet\n");break;
        case NAS_PROTOCOL_ICMP4:strcpy(sprotocol, "icmp4");printk("icmp4 packet\n");break;
        case NAS_PROTOCOL_ICMP6:strcpy(sprotocol, "icmp6"); nasrg_TOOL_pk_icmp6((struct icmp6hdr*)protocolh);break;
        default:strcpy(sprotocol, "other L4");break;
      }
      printk("nasrg_CLASS_send: (dscp %u, %s) received, (classref %u, fct %s, drb_id %u) classifier rule\n",
            dscp, sprotocol, sp->classref, sfct, sp->rab_id);
      #endif

      //forward packet to the correct entity
      if (sp->fct!=NULL){
        sp->fct(skb, cx, sp);
      }else{
        printk("\n\nnasrg_CLASS_send: ERROR : CLASSIFIER FUNCTION IS NULL\n\n");
      }
      no_connection = 0;
    // end : if classifier entry match found
    } else {
      printk("nasrg_CLASS_send: no corresponding item in the classifier list, so the message is dropped\n");
      printk("nasrg_CLASS_send: packet parameters: dscp %u, %s\n", dscp, sprotocol);
      nasrg_COMMON_del_send(skb, cx, NULL);  // Note MW: LG has commented this line. Why?
    }
  }   // if connection found

  #ifdef NAS_DEBUG_CLASS
  if (no_connection == 1) {
    printk("nasrg_CLASS_send: no corresponding connection, so the message is dropped\n");
  }
  printk("nasrg_CLASS_send: end\n");
  #endif
}
