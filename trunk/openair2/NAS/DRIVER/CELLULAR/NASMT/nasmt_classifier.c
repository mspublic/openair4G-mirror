/***************************************************************************
                          nasmt_classifier.c  -  description
                             -------------------
    copyright            : (C) 2003 by Eurecom
    email                : yan.moret@eurecom.fr
                           michelle.wetterwald@eurecom.fr
 ***************************************************************************

 ***************************************************************************/

#include "nasmt_variables.h"
#include "nasmt_proto.h"

#include <net/ip6_fib.h>
#include <net/route.h>

//---------------------------------------------------------------------------
// Add a new classifier rule (send direction)
struct classifier_entity *nasmt_CLASS_add_sclassifier(struct cx_entity *cx, u8 dscp, u16 classref){
//---------------------------------------------------------------------------
	struct classifier_entity *gc;

  #ifdef GRAAL_DEBUG_CLASS
  printk("nasmt_CLASS_add_sclassifier: begin for dscp %d, classref %d\n", dscp,classref);
  #endif
  if (cx==NULL){
 	  printk("nasmt_CLASS_add_sclassifier - input parameter cx is NULL \n");
    return NULL;
  }
//***
	for (gc=cx->sclassifier[dscp]; gc!=NULL; gc=gc->next){
		if (gc->classref==classref){
      #ifdef GRAAL_DEBUG_CLASS
		  printk("nasmt_CLASS_add_sclassifier: classifier already exist for dscp %d, classref %d\n",dscp,classref);
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
  printk("nasmt_CLASS_add_sclassifier: classifier created for dscp %d, classref %d\n",dscp,classref);
  #endif
	return gc;
}

//---------------------------------------------------------------------------
// Add a new classifier rule (receive direction)
struct classifier_entity *nasmt_CLASS_add_rclassifier(u8 dscp, u16 classref){
//---------------------------------------------------------------------------
	struct classifier_entity *gc;

  #ifdef GRAAL_DEBUG_CLASS
  printk("nasmt_CLASS_add_rclassifier: begin\n");
  #endif
//***
	for (gc=gpriv->rclassifier[dscp]; gc!=NULL; gc=gc->next)
	{
		if (gc->classref==classref){
      #ifdef GRAAL_DEBUG_CLASS
		  printk("nasmt_CLASS_add_rclassifier: classifier already exist for dscp %d, classref %d\n",dscp,classref);
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
  printk("nasmt_CLASS_add_rclassifier: classifier created for dscp %d, classref %d\n",dscp,classref);
  #endif
	return gc;
}

//---------------------------------------------------------------------------
void nasmt_CLASS_flush_sclassifier(struct cx_entity *cx){
//---------------------------------------------------------------------------
	u8 dscpi;
	struct classifier_entity *gc;

  #ifdef GRAAL_DEBUG_CLASS
  printk("nasmt_CLASS_flush_sclassifier: begin\n");
  #endif
  if (cx==NULL){
 	  printk("nasmt_CLASS_flush_sclassifier - input parameter cx is NULL \n");
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
  printk("nasmt_CLASS_flush_sclassifier: end\n");
  #endif
}

//---------------------------------------------------------------------------
void nasmt_CLASS_flush_rclassifier(){
//---------------------------------------------------------------------------
	u8 dscpi;
	struct classifier_entity *gc;

  #ifdef GRAAL_DEBUG_CLASS
  printk("nasmt_CLASS_flush_rclassifier: begin\n");
  #endif
//***
	for (dscpi=0; dscpi<GRAAL_DSCP_MAX; ++dscpi)
	{
		for (gc=gpriv->rclassifier[dscpi]; gc!=NULL; gc=gpriv->rclassifier[dscpi])
		{
			gpriv->rclassifier[dscpi]=gc->next;
			kfree(gc);
		}
	}
	gpriv->nrclassifier=0;
  #ifdef GRAAL_DEBUG_CLASS
  printk("nasmt_CLASS_flush_rclassifier: end\n");
  #endif
}

//---------------------------------------------------------------------------
// Delete a classifier rule (send direction)
void nasmt_CLASS_del_sclassifier(struct cx_entity *cx, u8 dscp, u16 classref){
//---------------------------------------------------------------------------
	struct classifier_entity *p,*np;

  #ifdef GRAAL_DEBUG_CLASS
  printk("nasmt_CLASS_del_sclassifier: begin\n");
  #endif
  if (cx==NULL){
 	  printk("nasmt_CLASS_del_sclassifier - input parameter cx is NULL \n");
    return;
  }
//***
	p=cx->sclassifier[dscp];
	if (p==NULL)
		return;
	if (p->classref==classref)
	{
		cx->sclassifier[dscp]=p->next;
		kfree(p);
		--cx->nsclassifier;
		return;
	}
	for (np=p->next; np!=NULL; p=np)
	{
		if (np->classref==classref)
		{
			p->next=np->next;
			kfree(np);
			--cx->nsclassifier;
			return;
		}
	}
  #ifdef GRAAL_DEBUG_CLASS
  printk("nasmt_CLASS_del_sclassifier: end\n");
  #endif
}

//---------------------------------------------------------------------------
// Delete a classifier rule (receive direction)
void nasmt_CLASS_del_rclassifier(u8 dscp, u16 classref){
//---------------------------------------------------------------------------
	struct classifier_entity *p,*np;

  #ifdef GRAAL_DEBUG_CLASS
  printk("nasmt_CLASS_del_rclassifier: begin\n");
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
		if (np->classref==classref)
		{
			p->next=np->next;
			kfree(np);
			--gpriv->nrclassifier;
			return;
		}
	}
  #ifdef GRAAL_DEBUG_CLASS
  printk("nasmt_CLASS_del_rclassifier: end\n");
  #endif
}

//---------------------------------------------------------------------------
// Search the entity with the IPv6 address 'addr'
struct cx_entity *nasmt_CLASS_cx6(struct sk_buff *skb){
//---------------------------------------------------------------------------
	u8 cxi;
  #ifdef GRAAL_DEBUG_CLASS
  printk("nasmt_CLASS_cx6: begin\n");
  #endif
	cxi=0;
	return gpriv->cx+cxi;
}

//---------------------------------------------------------------------------
// Search the entity with the IPv4 address 'addr'
struct cx_entity *nasmt_CLASS_cx4(struct sk_buff *skb){
//---------------------------------------------------------------------------
	u8 cxi;
  #ifdef GRAAL_DEBUG_CLASS
  printk("nasmt_CLASS_cx4: begin\n");
  #endif
	cxi=0;
	return gpriv->cx+cxi;
}

//---------------------------------------------------------------------------
// Search the sending function for IP Packet
void nasmt_CLASS_send(struct sk_buff *skb){
//---------------------------------------------------------------------------
	struct classifier_entity *p, *sp;
	u8 *protocolh;
	u8 protocol, dscp;
	u16 classref;
	struct cx_entity *cx;
	char sfct[10], sprotocol[10];
//	u16 dport, sport;

  #ifdef GRAAL_DEBUG_CLASS
  printk("nasmt_CLASS_send: begin - [before switch on IP protocol version] \n");
  #endif
  if (skb==NULL){
 	  printk("nasmt_CLASS_send - input parameter skb is NULL \n");
    return;
  }
//***
  // Get mobile connexion entity, protocol and dscp from IP packet
	switch (ntohs(skb->protocol)){
    case ETH_P_IPV6:
//   	cx=graal_CLASS_cx6(skb);
      #ifdef GRAAL_DEBUG_CLASS
      printk("nasmt_CLASS_send : skb->protocol : IPv6 \n");
      #endif
      cx = gpriv->cx;
    	protocolh=nasmt_TOOL_get_protocol6(skb->nh.ipv6h, &protocol);
    	dscp=nasmt_TOOL_get_dscp6(skb->nh.ipv6h);
    	break;
    case ETH_P_IP:
//   	cx=graal_CLASS_cx4(skb);
      #ifdef GRAAL_DEBUG_CLASS
      printk("nasmt_CLASS_send : skb->protocol : IPv4 \n");
      #endif
      cx = gpriv->cx;
    	protocolh=nasmt_TOOL_get_protocol4(skb->nh.iph, &protocol);
    	dscp=nasmt_TOOL_get_dscp4(skb->nh.iph);
    	break;
    default:
    	printk("nasmt_CLASS_send: Unknown IP version protocol\n");
    	return;
	}
	if (cx==NULL){
		printk("nasmt_CLASS_send: No corresponding connection, so the message is dropped\n");
		return;
	}
  //#define GRAAL_DEBUG_CLASS_PACKET
  #ifdef GRAAL_DEBUG_CLASS_PACKET
  printk("nasmt_CLASS_send : packet header: \n");
  if ((skb->data) != NULL)
     nasmt_TOOL_print_buffer(skb->data,skb->len);
  else
     printk("nasmt_CLASS_send : packet header has a NULL pointer \n");
  #endif
//***
	classref=0;
	sp=NULL;
//***
  // First, look if there is a classifier entity for IP packet DSCP
  #ifdef GRAAL_DEBUG_SEND_DETAIL
  printk("nasmt_CLASS_send: before for loop on IP packet DSCP \n");
  #endif
	for (p=cx->sclassifier[dscp]; p!=NULL; p=p->next){
    if (p->classref>=classref){
      sp=p;
      classref=sp->classref;
      #ifdef GRAAL_DEBUG_SEND_DETAIL
      printk("nasmt_CLASS_send: classifier found for dscp %u \n", dscp);
      #endif
    }
	}
//***
  // if no classifier, check if a default DSCP classifier exists
	if (sp==NULL){
  #ifdef GRAAL_DEBUG_SEND_DETAIL
  printk("nasmt_CLASS_send: before for loop on DSCP_DEFAULT \n");
  #endif
    for (p=cx->sclassifier[GRAAL_DSCP_DEFAULT]; p!=NULL; p=p->next){
      if (p->classref>=classref){
        sp=p;
        classref=sp->classref;
        #ifdef GRAAL_DEBUG_SEND_DETAIL
        printk("nasmt_CLASS_send: classifier found for dscp DEFAULT\n");
        #endif
      }
    }
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
  printk("nasmt_CLASS_send: before: (sp!=NULL) - Print and execute result \n");
#endif
	if (sp!=NULL){
#ifdef GRAAL_DEBUG_CLASS
    // classifier entity found. Print its parameters
		if (sp->fct==nasmt_COMMON_QOS_send)
			strcpy(sfct, "data xfer");
		if (sp->fct==nasmt_CTL_send)
			strcpy(sfct, "iocontrol");
		if (sp->fct==nasmt_COMMON_del_send)
			strcpy(sfct, "delete");
		if (sp->fct==nasmt_ASCTL_DC_send_sig_data_request)
			strcpy(sfct, "DC-SAP");
		printk("nasmt_CLASS_send: (dscp %u, %s) received, (classref %u, fct %s, rab_id %u) classifier rule\n",
						dscp, sprotocol, sp->classref, sfct, sp->rab_id);
#endif
    //forward packet to the correct entity
	  if (sp->fct!=NULL){
		  sp->fct(skb, cx, sp);
    }else{
      printk("\n\nnasmt_CLASS_send: ERROR : CLASSIFIER FUNCTION IS NULL\n\n");
    }
	}else{
    // classifier entity not found. drop the packet
		printk("nasmt_CLASS_send: no corresponding item in the classifier list, so the message is dropped\n");
		printk("nasmt_CLASS_send: packet parameters: dscp %u, %s\n", dscp, sprotocol);
		nasmt_COMMON_del_send(skb, cx, NULL);
	}
#ifdef GRAAL_DEBUG_CLASS
  printk("nasmt_CLASS_send: end\n");
#endif

}
