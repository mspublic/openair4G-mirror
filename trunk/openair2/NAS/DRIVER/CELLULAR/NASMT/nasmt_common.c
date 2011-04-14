/***************************************************************************
                          nasmt_common.c  -  description
                             -------------------
    copyright            : (C) 2002 by Eurecom
    email                : yan.moret@eurecom.fr
                           michelle.wetterwald@eurecom.fr
 ***************************************************************************

 ***************************************************************************/

#include "nasmt_variables.h"
#include "nasmt_proto.h"
//---------------------------------------------------------------------------
// Receive data from FIFO (QOS or DC)
void nasmt_COMMON_receive(u16 hlen, u16 dlen, int sap){
//---------------------------------------------------------------------------
	struct sk_buff *skb;
	struct ipversion *ipv;
#ifdef GRAAL_DEBUG_RECEIVE
	printk("nasmt_COMMON_receive: begin\n");
#endif
	skb = dev_alloc_skb( dlen + 2 );
	if(!skb)
	{
		printk("nasmt_COMMON_receive: low on memory\n");
		++gpriv->stats.rx_dropped;
		return;
	}
	skb_reserve(skb,2);
	bytes_read += rtf_get(sap, skb_put(skb, dlen), dlen);
	if (bytes_read != hlen+dlen)
	{
		printk("nasmt_COMMON_receive: problem while reading DC sap\n");
		kfree(skb->data);
		dev_kfree_skb(skb);
		return;
	}
	skb->dev = gdev;
	skb->mac.raw = skb->data;
	skb->pkt_type = PACKET_HOST;
	skb->ip_summed = CHECKSUM_UNNECESSARY;
	ipv = (struct ipversion *)skb->data;
	switch (ipv->version)
	{
	case 6:
#ifdef GRAAL_DEBUG_RECEIVE_BASIC
		printk("nasmt_COMMON_receive: receive IPv6 message\n");
#endif
		skb->nh.ipv6h = (struct ipv6hdr *)skb->data;
		skb->protocol = htons(ETH_P_IPV6);
		break;
	case 4:
#ifdef GRAAL_DEBUG_RECEIVE_BASIC
		printk("nasmt_COMMON_receive: receive IPv4 message\n");
#endif
		skb->nh.iph = (struct iphdr *)skb->data;
		skb->protocol = htons(ETH_P_IP);
		break;
	default:
		printk("nasmt_COMMON_receive: receive unknown message\n");
	}
	++gpriv->stats.rx_packets;
	gpriv->stats.rx_bytes += bytes_read;
	netif_rx(skb);
#ifdef GRAAL_DEBUG_RECEIVE
		printk("nasmt_COMMON_receive: end\n");
#endif
}

//---------------------------------------------------------------------------
// Delete the data
void nasmt_COMMON_del_send(struct sk_buff *skb, struct cx_entity *cx, struct classifier_entity *sp){
//---------------------------------------------------------------------------
	++gpriv->stats.tx_dropped;
}

//---------------------------------------------------------------------------
// Request the transfer of data (QoS SAP)
void nasmt_COMMON_QOS_send(struct sk_buff *skb, struct cx_entity *cx, struct classifier_entity *gc){
//---------------------------------------------------------------------------
	struct pdcp_data_req     pdcph;
// Start debug information
#ifdef GRAAL_DEBUG_SEND
	printk("nasmt_COMMON_QOS_send - begin \n");
#endif
//	if (cx->state!=GRAAL_STATE_CONNECTED) // <--- A REVOIR
//	{
//		gpriv->stats.tx_dropped ++;
//		printk("GRAAL_QOS_SEND: No connected, so message are dropped \n");
//		return;
//	}
  if (skb==NULL){
 	  printk("nasmt_COMMON_QOS_send - input parameter skb is NULL \n");
    return;
  }
  if (gc==NULL){
 	  printk("nasmt_COMMON_QOS_send - input parameter gc is NULL \n");
    return;
  }
  if (cx==NULL){
 	  printk("nasmt_COMMON_QOS_send - input parameter cx is NULL \n");
    return;
  }
// End debug information
	if (gc->rb==NULL){
		gc->rb = nasmt_COMMON_search_rb(cx, gc->rab_id);
		if (gc->rb==NULL){
			++gpriv->stats.tx_dropped;
			printk("nasmt_COMMON_QOS_send: No corresponding Radio Bearer, so message are dropped, rab_id=%u \n", gc->rab_id);
			return;
		}
	}
#ifdef GRAAL_DEBUG_SEND
	printk("nasmt_COMMON_QOS_send #1 :");
	printk("lcr %u, rab_id %u, rab_id %u\n", cx->lcr, (gc->rb)->rab_id, gc->rab_id);
  nasmt_TOOL_print_classifier(gc);
#endif
	pdcph.data_size  = skb->len;
	pdcph.rb_id      = (gc->rb)->rab_id+(32*cx->lcr);
	bytes_wrote = rtf_put(gpriv->sap[(gc->rb)->sapi], &pdcph, NAS_PDCPH_SIZE);
	if (bytes_wrote != NAS_PDCPH_SIZE){
		printk("nasmt_COMMON_QOS_send: problem while writing PDCP's header\n");
		printk("rb_id %d, SAP index %d, Wrote %d, Header Size %d \n", pdcph.rb_id , (gc->rb)->sapi, bytes_wrote, NAS_PDCPH_SIZE);
		return;
	}
	bytes_wrote += rtf_put(gpriv->sap[(gc->rb)->sapi], skb->data, skb->len);
	if (bytes_wrote != skb->len+NAS_PDCPH_SIZE){
		printk("nasmt_COMMON_QOS_send: problem while writing PDCP's data\n"); // congestion
		return;
	}
#ifdef GRAAL_DEBUG_SEND
	printk("nasmt_COMMON_QOS_send - %d bytes wrote to rb_id %d, sap %d \n", bytes_wrote, pdcph.rb_id,gpriv->sap[(gc->rb)->sapi]);
#endif
	gpriv->stats.tx_bytes   += skb->len;
	gpriv->stats.tx_packets ++;
#ifdef GRAAL_DEBUG_SEND
	printk("nasmt_COMMON_QOS_send - end \n");
#endif
}

//---------------------------------------------------------------------------
void nasmt_COMMON_QOS_receive(struct cx_entity *cx){
//---------------------------------------------------------------------------
	u8 sapi;
	struct pdcp_data_ind     pdcph;
// Start debug information
#ifdef GRAAL_DEBUG_RECEIVE
	printk("nasmt_COMMON_QOS_receive - begin \n");
#endif
  if (!cx){
 	  printk("nasmt_COMMON_QOS_receive - input parameter cx is NULL \n");
    return;
  }
// End debug information
  // LG force the use of only 1 rt fifo
  sapi = GRAAL_CO_OUTPUT_SAPI;
	// LG for (sapi = GRAAL_CO_OUTPUT_SAPI; sapi <= GRAAL_BA_OUTPUT_SAPI; ++sapi)
	{
		bytes_read =  rtf_get(gpriv->sap[sapi], &pdcph, NAS_PDCPH_SIZE);
		while (bytes_read>0)
		{
			if (bytes_read != NAS_PDCPH_SIZE)
			{
				printk("nasmt_COMMON_QOS_receive: problem while reading PDCP header\n");
				return;
			}
			nasmt_COMMON_receive(NAS_PDCPH_SIZE, pdcph.data_size, gpriv->sap[sapi]);
			bytes_read =  rtf_get(gpriv->sap[sapi], &pdcph, NAS_PDCPH_SIZE);
		}
	}
#ifdef GRAAL_DEBUG_RECEIVE
	printk("nasmt_COMMON_QOS_receive - end \n");
#endif
}

//---------------------------------------------------------------------------
struct cx_entity *nasmt_COMMON_search_cx(nasLocalConnectionRef_t lcr){
//---------------------------------------------------------------------------
#ifdef GRAAL_DEBUG_CLASS
	printk("nasmt_COMMON_search_cx - lcr %d\n",lcr);
#endif
	if (lcr<GRAAL_CX_MAX)
		return gpriv->cx+lcr;
	else
		return NULL;
}

//---------------------------------------------------------------------------
// Search a Radio Bearer
struct rb_entity *nasmt_COMMON_search_rb(struct cx_entity *cx, nasRadioBearerId_t rab_id){
//---------------------------------------------------------------------------
	struct rb_entity *rb;
#ifdef GRAAL_DEBUG_CLASS
	printk("nasmt_COMMON_search_rb - rab_id %d\n", rab_id);
#endif
  if (!cx){
 	  printk("nasmt_COMMON_search_rb - input parameter cx is NULL \n");
    return NULL;
  }
	for (rb=cx->rb; rb!=NULL; rb=rb->next)
	{
		if (rb->rab_id==rab_id)
			return rb;
	}
	return NULL;
}

//---------------------------------------------------------------------------
struct rb_entity *nasmt_COMMON_add_rb(struct cx_entity *cx, nasRadioBearerId_t rab_id, nasQoSTrafficClass_t qos){
//---------------------------------------------------------------------------
	struct rb_entity *rb;
#ifdef GRAAL_DEBUG_CLASS
 	  printk("nasmt_COMMON_add_rb - begin for rab_id %d , qos %d\n", rab_id, qos );
#endif
  if (cx==NULL){
 	  printk("nasmt_COMMON_add_rb - input parameter cx is NULL \n");
    return NULL;
  }
	rb=nasmt_COMMON_search_rb(cx, rab_id);
	if (rb==NULL)
	{
		rb=(struct rb_entity *)kmalloc(sizeof(struct rb_entity), GFP_KERNEL);
		if (rb!=NULL)
		{
			rb->retry=0;
			rb->countimer=GRAAL_TIMER_IDLE;
			rb->rab_id=rab_id;
//			rb->rab_id=rab_id+(32*cx->lcr);
      #ifdef GRAAL_DEBUG_DC
      printk("nasmt_COMMON_add_rb: rb rab_id=%u, rab_id=%u, mt_id=%u\n",rb->rab_id,rab_id, cx->lcr);
      #endif
			rb->qos=qos;
      rb->sapi=GRAAL_CO_INPUT_SAPI;
			// LG force the use of only one rt-fifo rb->sapi=GRAAL_BA_INPUT_SAPI;
			rb->state=GRAAL_IDLE;
			rb->next=cx->rb;
			cx->rb=rb;
			++cx->num_rb;
		}
		else
			printk("nasmt_COMMON_add_rb: no memory\n");
	}
#ifdef GRAAL_DEBUG_CLASS
 	  printk("nasmt_COMMON_add_rb - end \n" );
#endif
	return rb;
}

//---------------------------------------------------------------------------
// free the memory that has previously been allocated to rb and remove from linked list
void nasmt_COMMON_del_rb(struct cx_entity *cx, nasRadioBearerId_t rab_id, nasIPdscp_t dscp){
//---------------------------------------------------------------------------
  struct rb_entity *rb, *curr_rb, *prev_rb;
  struct classifier_entity *p;
  u16 classref=0;

// Start debug information
#ifdef GRAAL_DEBUG_CLASS
  printk("nasmt_COMMON_del_rb - begin\n");
#endif
  if (cx==NULL){
    printk("nasmt_COMMON_del_rb - input parameter cx is NULL \n");
    return;
  }
// End debug information

// Clear the associated classifier
  for (p=cx->sclassifier[dscp]; p!=NULL; p=p->next){
    if (p->classref>=classref){
      classref=p->classref;
      #ifdef GRAAL_DEBUG_CLASS
      printk("nasmt_COMMON_del_rb: classifier found for dscp %u \n", dscp);
      #endif
    }
  }
  nasmt_CLASS_del_sclassifier(cx, dscp, classref);

// Now, delete the RB
  curr_rb = NULL;
  prev_rb = NULL;
  for (rb=cx->rb; rb!=NULL; rb=rb->next){
    if (rb->rab_id == rab_id){
      curr_rb = rb;
      if (prev_rb!=NULL){
        prev_rb->next = rb->next;
      }else{
        cx->rb=rb->next;
      }
      break;
    }else{
      prev_rb = rb;
    }
  }
  if (curr_rb!= NULL){
    printk("nasmt_COMMON_del_rb: del rab_id %u\n", rb->rab_id);
    kfree(rb);
    (cx->num_rb)--;
  }else{
    printk("\n\n--nasmt_COMMON_del_rb: ERROR, invalid rab_id %u\n", rb->rab_id);
  }
#ifdef GRAAL_DEBUG_CLASS
  printk("nasmt_COMMON_del_rb - end\n");
#endif
}

//---------------------------------------------------------------------------
void nasmt_COMMON_flush_rb(struct cx_entity *cx){
//---------------------------------------------------------------------------
	struct rb_entity *rb;
	struct classifier_entity *gc;
	u8 dscp;
// Start debug information
#ifdef GRAAL_DEBUG_CLASS
	printk("nasmt_COMMON_flush_rb - begin\n");
#endif
  if (cx==NULL){
 	  printk("nasmt_COMMON_flush_rb - input parameter cx is NULL \n");
    return;
  }
// End debug information
	for (rb=cx->rb; rb!=NULL; rb=cx->rb){
		printk("nasmt_COMMON_flush_rb: del rab_id %u\n", rb->rab_id);
		cx->rb=rb->next;
		kfree(rb);
	}
	cx->num_rb=0;
	cx->rb=NULL;
	for(dscp=0; dscp<GRAAL_DSCP_MAX; ++dscp){
		for (gc=cx->sclassifier[dscp]; gc!=NULL; gc=gc->next)
			gc->rb=NULL;
	}
#ifdef GRAAL_DEBUG_CLASS
	printk("nasmt_COMMON_flush_rb - end\n");
#endif
}
