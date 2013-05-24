/***************************************************************************
                          nasrg_classifier.c  -  description
                             -------------------
    copyright            : (C) 2003 by Eurecom
    email                : yan.moret@eurecom.fr
                           michelle.wetterwald@eurecom.fr
 ***************************************************************************

 ***************************************************************************/

#include "nasrg_variables.h"
#include "nasrg_proto.h"

#include <net/ip6_fib.h>
#include <net/route.h>

//---------------------------------------------------------------------------
// Add a new classifier rule (send direction)
struct classifier_entity *nasrg_CLASS_add_sclassifier(struct cx_entity *cx, u8 dscp, u16 classref){
//---------------------------------------------------------------------------
	struct classifier_entity *gc;

  #ifdef GRAAL_DEBUG_CLASS
  printk("nasrg_CLASS_add_sclassifier: begin for dscp %d, classref %d\n", dscp,classref);
  #endif
  if (cx==NULL){
 	  printk("nasrg_CLASS_add_sclassifier - input parameter cx is NULL \n");
    return NULL;
  }
//***
	for (gc=cx->sclassifier[dscp]; gc!=NULL; gc=gc->next){
		if (gc->classref==classref){
      #ifdef GRAAL_DEBUG_CLASS
		  printk("nasrg_CLASS_add_sclassifier: classifier already exist for dscp %d, classref %d\n",dscp,classref);
      #endif
			return gc;
    }
	}
	gc=(struct classifier_entity *)kmalloc(sizeof(struct classifier_entity), GFP_KERNEL);
	if (gc==NULL)
		return NULL;
	gc->next=cx->sclassifier[dscp];
	gc->classref=classref;
	cx->sclassifier[dscp]=gc;
	++cx->nsclassifier;
  ++gpriv->next_sclassref; //increment send classref index - MW 15/01/07
  #ifdef GRAAL_DEBUG_CLASS
  printk("nasrg_CLASS_add_sclassifier: classifier created for dscp %d, classref %d\n",dscp,classref);
  #endif
	return gc;
}

//---------------------------------------------------------------------------
// Add a new classifier rule (receive direction)
struct classifier_entity *nasrg_CLASS_add_rclassifier(u8 dscp, u16 classref){
//---------------------------------------------------------------------------
	struct classifier_entity *gc;

  #ifdef GRAAL_DEBUG_CLASS
  printk("nasrg_CLASS_add_rclassifier: begin\n");
  #endif
//***
	for (gc=gpriv->rclassifier[dscp]; gc!=NULL; gc=gc->next)
	{
		if (gc->classref==classref){
      #ifdef GRAAL_DEBUG_CLASS
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
  #ifdef GRAAL_DEBUG_CLASS
  printk("nasrg_CLASS_add_rclassifier: classifier created for dscp %d, classref %d\n",dscp,classref);
  #endif
	return gc;
}

//---------------------------------------------------------------------------
// Add a new classifier rule (mbms direction)
struct classifier_entity *nasrg_CLASS_add_mbmsclassifier(int mbms_ix, u16 classref){
//---------------------------------------------------------------------------
	struct classifier_entity *gc;

  #ifdef GRAAL_DEBUG_CLASS
  printk("nasrg_CLASS_add_mbmsclassifier: begin\n");
  #endif
//***
	for (gc=gpriv->mbmsclassifier[mbms_ix]; gc!=NULL; gc=gc->next)
	{
		if (gc->classref==classref){
      #ifdef GRAAL_DEBUG_CLASS
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
  #ifdef GRAAL_DEBUG_CLASS
  printk("nasrg_CLASS_add_mbmsclassifier: classifier created for service index %d, classref %d\n",mbms_ix,classref);
  #endif
	return gc;
}

//---------------------------------------------------------------------------
void nasrg_CLASS_flush_sclassifier(struct cx_entity *cx){
//---------------------------------------------------------------------------
	u8 dscpi;
	struct classifier_entity *gc;

  #ifdef GRAAL_DEBUG_CLASS
  printk("nasrg_CLASS_flush_sclassifier: begin\n");
  #endif
  if (cx==NULL){
 	  printk("nasrg_CLASS_flush_sclassifier - input parameter cx is NULL \n");
    return;
  }
//***
	for (dscpi=0; dscpi<GRAAL_DSCP_MAX; ++dscpi){
		for (gc=cx->sclassifier[dscpi]; gc!=NULL; gc=cx->sclassifier[dscpi]){
			cx->sclassifier[dscpi]=gc->next;
			kfree(gc);
		}
	}
	cx->nsclassifier=0;
  #ifdef GRAAL_DEBUG_CLASS
  printk("nasrg_CLASS_flush_sclassifier: end\n");
  #endif
}

//---------------------------------------------------------------------------
void nasrg_CLASS_flush_rclassifier(){
//---------------------------------------------------------------------------
	u8 dscpi;
	struct classifier_entity *gc;

  #ifdef GRAAL_DEBUG_CLASS
  printk("nasrg_CLASS_flush_rclassifier: begin\n");
  #endif
//***
	for (dscpi=0; dscpi<GRAAL_DSCP_MAX; ++dscpi){
		for (gc=gpriv->rclassifier[dscpi]; gc!=NULL; gc=gpriv->rclassifier[dscpi]){
			gpriv->rclassifier[dscpi]=gc->next;
			kfree(gc);
		}
	}
	gpriv->nrclassifier=0;
  #ifdef GRAAL_DEBUG_CLASS
  printk("nasrg_CLASS_flush_rclassifier: end\n");
  #endif
}

//---------------------------------------------------------------------------
void nasrg_CLASS_flush_mbmsclassifier(){
//---------------------------------------------------------------------------
	int mbmsi;
	struct classifier_entity *gc;

  #ifdef GRAAL_DEBUG_CLASS
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
  #ifdef GRAAL_DEBUG_CLASS
  printk("nasrg_CLASS_flush_mbmsclassifier: end\n");
  #endif
}

//---------------------------------------------------------------------------
// Delete a classifier rule (send direction)
void nasrg_CLASS_del_sclassifier(struct cx_entity *cx, u8 dscp, u16 classref){
//---------------------------------------------------------------------------
	struct classifier_entity *p,*np;

  #ifdef GRAAL_DEBUG_CLASS
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
  #ifdef GRAAL_DEBUG_CLASS
  printk("nasrg_CLASS_del_sclassifier: end\n");
  #endif
}

//---------------------------------------------------------------------------
// Delete a classifier rule (receive direction)
void nasrg_CLASS_del_rclassifier(u8 dscp, u16 classref){
//---------------------------------------------------------------------------
	struct classifier_entity *p,*np;

  #ifdef GRAAL_DEBUG_CLASS
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
  #ifdef GRAAL_DEBUG_CLASS
  printk("nasrg_CLASS_del_rclassifier: end\n");
  #endif
}

//---------------------------------------------------------------------------
// Delete a classifier rule (mbms direction)
void nasrg_CLASS_del_mbmsclassifier(int mbms_ix, u16 classref){
//---------------------------------------------------------------------------
	struct classifier_entity *p,*np;

  #ifdef GRAAL_DEBUG_CLASS
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
  #ifdef GRAAL_DEBUG_CLASS
  printk("nasrg_CLASS_del_mbmsclassifier: end\n");
  #endif
}

//---------------------------------------------------------------------------
// Search the entity with the IPv4 address 'addr'
struct cx_entity *nasrg_CLASS_cx4(struct sk_buff *skb){
//---------------------------------------------------------------------------
	u8 cxi;
/*#ifdef NODE_RG     A RETABLIR QUAND IL Y AURA UN PROTO DE CONFIG D'ADRESSE
	if (skb->dst!=NULL)
	{
		for (cxi=0; cxi<GRAAL_CX_MAX; ++cxi)
		{
			if (gpriv->cx[cxi].iid4==((((struct rtable *)(skb->dst))->rt_gateway)>>24))
				return gpriv->cx+cxi;
		}
	}
	return NULL;
#else*/
  #ifdef GRAAL_DEBUG_CLASS
  printk("nasrg_CLASS_cx4: begin\n");
  #endif
	cxi=0;
	return gpriv->cx+cxi;
//#endif
}

////---------------------------------------------------------------------------
//// Search the entity with the IPv6 address 'addr' //Yan version
//struct cx_entity *nasrg_CLASS_cx6(struct sk_buff *skb){
////---------------------------------------------------------------------------
//	u8 cxi;
////	if (skb->dst!=NULL)
////	{
////		for (cxi=0; cxi<GRAAL_CX_MAX; ++cxi)
////		{
////			if (((gpriv->cx[cxi]).iid6[1]==((struct rt6_info *)(skb->dst))->rt6i_gateway.s6_addr32[3])&&
////				((gpriv->cx[cxi]).iid6[0]==((struct rt6_info *)(skb->dst))->rt6i_gateway.s6_addr32[2]))
////				return gpriv->cx+cxi;
////		}
////	}
////	return NULL;
//// [mw] should check IMEI + multicast addresses as in addrconf.c[ipv6_addr_type]
//  #ifdef GRAAL_DEBUG_CLASS
//  printk("nasrg_CLASS_cx6: begin\n");
//  #endif
//	cxi=0;
//	return gpriv->cx+cxi;
//}
//  addr_type=nasrg_CLASS_cx6(skb, cx, &mbms_ix);

//---------------------------------------------------------------------------
// Search the entity corresponding to destination address in IPv6 header
struct cx_entity *nasrg_CLASS_cx6(struct sk_buff *skb, int* paddr_type, int* pmbms_ix){
//---------------------------------------------------------------------------
	struct cx_entity * cx=NULL;
  u8 cxi;
  u32 mc_addr_hdr, uni_ifid1, uni_ifid2;
  //int addr_type = NASRG_ADDR_UNKNOWN;

  #ifdef GRAAL_DEBUG_CLASS
  printk("nasrg_CLASS_cx6: begin\n");
  #endif
  if (!skb){
 	  printk("nasrg_CLASS_cx6 - input parameter skb is NULL \n");
    return cx;
  }
	*paddr_type = NASRG_ADDR_UNKNOWN;
  mc_addr_hdr = ntohl(skb->nh.ipv6h->daddr.in6_u.u6_addr32[0]);
  // First check if multicast [1st octet is FF]
	if ((mc_addr_hdr & 0xFF000000) == 0xFF000000) {
     // packet type according to the scope of the multicast packet
     // we don't consider RPT bits in second octet [maybe done later if needed]
 		 switch(mc_addr_hdr & 0x000F0000) {
			 case (0x00020000):
				 *paddr_type = NASRG_ADDR_MC_SIGNALLING;
         #ifdef GRAAL_DEBUG_CLASS
         printk("nasrg_CLASS_cx6: multicast packet - signalling \n");
         #endif
				 break;
			 case (0x000E0000):
				 *paddr_type = NASRG_ADDR_MC_MBMS;
         *pmbms_ix = 0;
         cx=gpriv->cx;  // MBMS associate to Mobile 0
         #ifdef GRAAL_DEBUG_CLASS
         printk("nasrg_CLASS_cx6: multicast packet - MBMS data \n");
         #endif
				 break;
       default:
				 *paddr_type = NASRG_ADDR_UNKNOWN;
         *pmbms_ix = NASRG_MBMS_SVCES_MAX;
     }
  // This is not multicast, so we should be able to identify the MT
  }else{
     #ifdef GRAAL_DEBUG_CLASS
     printk("nasrg_CLASS_cx6: unicast packet\n");
     #endif
     *paddr_type = NASRG_ADDR_UNICAST;
     uni_ifid1 = ntohl(skb->nh.ipv6h->daddr.in6_u.u6_addr32[2]);
     uni_ifid2 = ntohl(skb->nh.ipv6h->daddr.in6_u.u6_addr32[3]);
     for (cxi=0; cxi<GRAAL_CX_MAX; cxi++){
       cx=gpriv->cx+cxi;
       #ifdef GRAAL_DEBUG_SEND_DETAIL
       printk("nasrg_CLASS_cx6: Compared addresses \n");
       printk("                Daddr[2] %ul, Daddr[3] %ul\n",
                 skb->nh.ipv6h->daddr.in6_u.u6_addr32[2],skb->nh.ipv6h->daddr.in6_u.u6_addr32[3]);
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
}


//---------------------------------------------------------------------------
// Search the sending function for IP Packet
void nasrg_CLASS_send(struct sk_buff *skb){
//---------------------------------------------------------------------------
	struct classifier_entity *p, *sp;
	u8 *protocolh;
	u8 cxi;
	u8 protocol, dscp;
	u16 classref;
  int addr_type, mbms_ix;
	struct cx_entity *cx = NULL;
	char sfct[10], sprotocol[10];
//	u16 dport, sport;

  #ifdef GRAAL_DEBUG_CLASS
  printk("nasrg_CLASS_send: begin - [before switch on IP protocol version] \n");
  #endif
  if (skb==NULL){
 	  printk("nasrg_CLASS_send - input parameter skb is NULL \n");
    return;
  }
//***
  // Get IP@, mobile connection entity, protocol and dscp from IP packet
	switch (ntohs(skb->protocol)){
  	case ETH_P_IPV6:
//  		cx=nasrg_CLASS_cx6(skb);
  		cx=nasrg_CLASS_cx6(skb, &addr_type, &mbms_ix);
  		protocolh=nasrg_TOOL_get_protocol6(skb->nh.ipv6h, &protocol);
  		dscp=nasrg_TOOL_get_dscp6(skb->nh.ipv6h);
  		break;
  	case ETH_P_IP:
  		cx=nasrg_CLASS_cx4(skb);
  		protocolh=nasrg_TOOL_get_protocol4(skb->nh.iph, &protocol);
  		dscp=nasrg_TOOL_get_dscp4(skb->nh.iph);
      addr_type=NASRG_ADDR_UNICAST;
  		break;
  	default:
  		printk("nasrg_CLASS_send: Unknown IP version protocol\n");
  		return;
	}
  //#define GRAAL_DEBUG_CLASS_PACKET
  #ifdef GRAAL_DEBUG_CLASS_PACKET
  printk("nasrg_CLASS_send : packet header: \n");
  if ((skb->data) != NULL)
     nasrg_TOOL_print_buffer(skb->data,skb->len);
  else
     printk("nasrg_CLASS_send : packet header has a NULL pointer \n");
  #endif

//***
	classref=0;
	sp=NULL;
//***
  switch (addr_type){
     case NASRG_ADDR_UNKNOWN:
       printk("nasrg_CLASS_send: No corresponding connection, so the message is dropped\n");
		   return;
     case NASRG_ADDR_MC_MBMS:
       sp = gpriv->mbmsclassifier[mbms_ix];
       if (sp!= NULL){
         classref=sp->classref;
         #ifdef GRAAL_DEBUG_SEND_DETAIL
         printk("nasrg_CLASS_send: classifier found for multicast service %d \n", mbms_ix);
         #endif
       }else{
         printk("nasrg_CLASS_send: No corresponding multicast bearer, so the message is dropped\n");
         return;
       }
       break;
     case NASRG_ADDR_UNICAST:
       // look if there is a classifier entity for IP packet DSCP
       #ifdef GRAAL_DEBUG_SEND_DETAIL
       printk("nasrg_CLASS_send: NASRG_ADDR_UNICAST, before for loop on IP packet DSCP \n");
       #endif
       for (p=cx->sclassifier[dscp]; p!=NULL; p=p->next){
         if (p->classref>=classref){
           sp=p;
           classref=sp->classref;
           #ifdef GRAAL_DEBUG_SEND_DETAIL
           printk("nasrg_CLASS_send: classifier found for dscp %u \n", dscp);
           #endif
         }
       }
       //***
       // if no classifier, check if a default DSCP classifier exists
       if (sp==NULL){
       #ifdef GRAAL_DEBUG_SEND_DETAIL
       printk("nasrg_CLASS_send: NASRG_ADDR_UNICAST, before for loop on DSCP_DEFAULT \n");
       #endif
         for (p=cx->sclassifier[GRAAL_DSCP_DEFAULT]; p!=NULL; p=p->next){
           if (p->classref>=classref){
             sp=p;
             classref=sp->classref;
             #ifdef GRAAL_DEBUG_SEND_DETAIL
             printk("nasrg_CLASS_send: NASRG_ADDR_UNICAST, classifier found for dscp DEFAULT\n");
             #endif
           }
         }
       }
       break;
     case NASRG_ADDR_MC_SIGNALLING:
       #ifdef GRAAL_DEBUG_SEND_DETAIL
       printk("nasrg_CLASS_send: NASRG_ADDR_MC_SIGNALLING, loop on existing connections \n");
       #endif
       for (cxi=0; cxi<GRAAL_CX_MAX; cxi++){
         cx=gpriv->cx+cxi;
         if ((p=cx->sclassifier[GRAAL_DSCP_DEFAULT])!=NULL){
           sp=p;
           classref=sp->classref;
           #ifdef GRAAL_DEBUG_SEND_DETAIL
           printk("nasrg_CLASS_send: NASRG_ADDR_MC_SIGNALLING, classifier found with dscp DEFAULT\n");
           #endif
           break;
         }
       }
       break;
	}
//***
  switch(protocol){
    case GRAAL_PROTOCOL_UDP:strcpy(sprotocol, "udp");break;
    case GRAAL_PROTOCOL_TCP:strcpy(sprotocol, "tcp");break;
    case GRAAL_PROTOCOL_ICMP4:strcpy(sprotocol, "icmp4");break;
    case GRAAL_PROTOCOL_ICMP6:strcpy(sprotocol, "icmp6");break;
    default:strcpy(sprotocol, "other L4");break;
  }

  #ifdef GRAAL_DEBUG_SEND_DETAIL
  printk("nasrg_CLASS_send: before : (sp!=NULL) -  Print and execute result \n");
  #endif
	if (sp!=NULL){
#ifdef GRAAL_DEBUG_CLASS
    // classifier entity found. Print its parameters
		if (sp->fct==nasrg_COMMON_QOS_send)
			strcpy(sfct, "data xfer");
		if (sp->fct==nasrg_CTL_send)
			strcpy(sfct, "iocontrol");
		if (sp->fct==nasrg_COMMON_del_send)
			strcpy(sfct, "delete");
		if (sp->fct==nasrg_ASCTL_DC_send_sig_data_request)
			strcpy(sfct, "DC-SAP");
		printk("nasrg_CLASS_send: (dscp %u, %s) received, (classref %u, fct %s, rab_id %u) classifier rule\n",
               dscp, sprotocol, sp->classref, sfct, sp->rab_id);
#endif
    //forward packet to the correct entity
    if (addr_type!= NASRG_ADDR_MC_SIGNALLING){
	    if (sp->fct!=NULL)
		    sp->fct(skb, cx, sp);
      else
        printk("\n\nnasrg_CLASS_send: ERROR : CLASSIFIER FUNCTION IS NULL\n\n");
    }else{
      for (cxi=0; cxi<GRAAL_CX_MAX; cxi++){
        cx=gpriv->cx+cxi;
        if ((p=cx->sclassifier[GRAAL_DSCP_DEFAULT])!=NULL){
          sp=p;
          classref=sp->classref;
          if (sp->fct!=NULL)
            sp->fct(skb, cx, sp);
        }
      }
    }
	}else{
    // classifier entity not found. drop the packet
		printk("nasrg_CLASS_send: no corresponding item in the classifier list, so the message is dropped\n");
		printk("nasrg_CLASS_send: packet parameters: dscp %u, %s\n", dscp, sprotocol);
		nasrg_COMMON_del_send(skb, cx, NULL);
	}
  #ifdef GRAAL_DEBUG_CLASS
  printk("nasrg_CLASS_send: end\n");
  #endif
}
