/***************************************************************************
                          nasrg_tool.c  -  description
                             -------------------
    copyright            : (C) 2002 by Eurecom
    email                : yan.moret@eurecom.fr
                           michelle.wetterwald@eurecom.fr
 ***************************************************************************

 ***************************************************************************/
#include "nasrg_variables.h"
#include "nasrg_proto.h"

//---------------------------------------------------------------------------
//
void nasrg_TOOL_fct(struct classifier_entity *gc, u8 fct){
//---------------------------------------------------------------------------
// Start debug information
#ifdef GRAAL_DEBUG_TOOL
	printk("nasrg_TOOL_fct - begin \n");
#endif
  if (gc==NULL){
 	  printk("nasrg_TOOL_fct - input parameter gc is NULL \n");
    return;
  }
// End debug information
	switch(fct){
    	case GRAAL_FCT_QOS_SEND:
    		gc->fct=nasrg_COMMON_QOS_send;
    		break;
    	case GRAAL_FCT_CTL_SEND:
    		gc->fct=nasrg_CTL_send;
    		break;
    	case GRAAL_FCT_DC_SEND:
    		gc->fct=nasrg_ASCTL_DC_send_sig_data_request;
    		break;
    	case GRAAL_FCT_DEL_SEND:
    		gc->fct=nasrg_COMMON_del_send;
    		break;
    	default:
    		gc->fct=nasrg_COMMON_del_send;
	}
}

//---------------------------------------------------------------------------
u8 nasrg_TOOL_invfct(struct classifier_entity *gc){
//---------------------------------------------------------------------------
// Start debug information
#ifdef GRAAL_DEBUG_TOOL
	printk("nasrg_TOOL_invfct - begin \n");
#endif
  if (gc==NULL){
 	  printk("nasrg_TOOL_invfct - input parameter gc is NULL \n");
    return 0;
  }
// End debug information
	if (gc->fct==nasrg_COMMON_QOS_send)
		return GRAAL_FCT_QOS_SEND;
	if (gc->fct==nasrg_CTL_send)
		return GRAAL_FCT_CTL_SEND;
	if (gc->fct==nasrg_COMMON_del_send)
		return GRAAL_FCT_DEL_SEND;
	if (gc->fct==nasrg_ASCTL_DC_send_sig_data_request)
		return GRAAL_FCT_DC_SEND;
	return 0;
}

//---------------------------------------------------------------------------
u8 nasrg_TOOL_get_dscp6(struct ipv6hdr *iph){
//---------------------------------------------------------------------------
// Start debug information
#ifdef GRAAL_DEBUG_TOOL
	printk("nasrg_TOOL_get_dscp6 - begin \n");
#endif
  if (iph==NULL){
 	  printk("nasrg_TOOL_get_dscp6 - input parameter iph is NULL \n");
    return 0;
  }
// End debug information
// return (ntohl(((*(__u32 *)iph)&GRAAL_TRAFFICCLASS_MASK)))>>22; //Yan
	return (ntohl(((*(__u32 *)iph)&GRAAL_TRAFFICCLASS_MASK)))>>20;
}

//---------------------------------------------------------------------------
u8 nasrg_TOOL_get_dscp4(struct iphdr *iph){
//---------------------------------------------------------------------------
// Start debug information
#ifdef GRAAL_DEBUG_TOOL
	printk("nasrg_TOOL_get_dscp4 - begin \n");
#endif
  if (iph==NULL){
 	  printk("nasrg_TOOL_get_dscp4 - input parameter iph is NULL \n");
    return 0;
  }
// End debug information
	return ((iph->tos)>>5)<<3;
}

////---------------------------------------------------------------------------
//int nasrg_TOOL_network6(struct in6_addr *addr, struct in6_addr *prefix, u8 plen){
////---------------------------------------------------------------------------
//// Start debug information
//#ifdef GRAAL_DEBUG_TOOL
//	printk("nasrg_TOOL_network6 - begin \n");
//#endif
//  if (!addr || !prefix){
// 	  printk("nasrg_TOOL_network6 - input parameter addr || prefix is NULL \n");
//    return 0;
//  }
//// End debug information
//	switch(plen/32){
//  	case 0:
//  		return (((addr->s6_addr32[0]>>(32-plen))<<(32-plen))==prefix->s6_addr[0]);
//  	case 1:
//  		return ((addr->s6_addr32[0]==prefix->s6_addr[0])&&
//  			(((addr->s6_addr32[1]>>(64-plen))<<(64-plen))==prefix->s6_addr[1]));
//  	case 2:
//  		return ((addr->s6_addr32[0]==prefix->s6_addr[0])&&
//  			(addr->s6_addr32[1]==prefix->s6_addr[1])&&
//  			(((addr->s6_addr32[2]>>(96-plen))<<(96-plen))==prefix->s6_addr[2]));
//  	case 3:
//  		return ((addr->s6_addr32[0]==prefix->s6_addr[0])&&
//  			(addr->s6_addr32[1]==prefix->s6_addr[1])&&
//  			(addr->s6_addr32[2]==prefix->s6_addr[2])&&
//  			(((addr->s6_addr32[3]>>(128-plen))<<(128-plen))==prefix->s6_addr[3]));
//  	default:
//  		return ((addr->s6_addr32[0]==prefix->s6_addr[0])&&
//  			(addr->s6_addr32[1]==prefix->s6_addr[1])&&
//  			(addr->s6_addr32[2]==prefix->s6_addr[2])&&
//  			(addr->s6_addr32[3]==prefix->s6_addr[3]));
//	}
//}

////---------------------------------------------------------------------------
//int nasrg_TOOL_network4(u32 *addr, u32 *prefix, u8 plen){
////---------------------------------------------------------------------------
//// Start debug information
//#ifdef GRAAL_DEBUG_TOOL
//	printk("nasrg_TOOL_network4 - begin \n");
//#endif
//  if (!addr || !prefix){
// 	  printk("nasrg_TOOL_network4 - input parameter addr || prefix is NULL \n");
//    return 0;
//  }
//// End debug information
//	if (plen>=32)
//		return (*addr==*prefix);
//	else
//		return (((*addr>>(32-plen))<<(32-plen))==*prefix);
//}

//---------------------------------------------------------------------------
u8 *nasrg_TOOL_get_protocol6(struct ipv6hdr *iph, u8 *protocol){
//---------------------------------------------------------------------------
	u16 size;
// Start debug information
#ifdef GRAAL_DEBUG_TOOL
	printk("nasrg_TOOL_get_protocol6 - begin \n");
#endif
  if (iph==NULL){
 	  printk("nasrg_TOOL_get_protocol6 - input parameter iph is NULL \n");
    return NULL;
  }
  if (protocol==NULL){
 	  printk("nasrg_TOOL_get_protocol6 - input parameter protocol is NULL \n");
    return NULL;
  }
// End debug information

	*protocol=iph->nexthdr;
	size=GRAAL_IPV6_SIZE;
	while (1){
		switch(*protocol){
		case IPPROTO_UDP:
		case IPPROTO_TCP:
		case IPPROTO_ICMPV6:
			return (u8 *)((u8 *)iph+size);
		case IPPROTO_HOPOPTS:
		case IPPROTO_ROUTING:
		case IPPROTO_DSTOPTS:
			*protocol=((u8 *)iph+size)[0];
			size+=((u8 *)iph+size)[1]*8+8;
			break;
		case IPPROTO_FRAGMENT:
			*protocol=((u8 *)iph+size)[0];
			size+=((u8 *)iph+size)[1]+8;
			break;
		case IPPROTO_NONE:
		case IPPROTO_AH:
		case IPPROTO_ESP:
		default:
			return NULL;
		}
	}
}

//---------------------------------------------------------------------------
u8 *nasrg_TOOL_get_protocol4(struct iphdr *iph, u8 *protocol){
//---------------------------------------------------------------------------
// Start debug information
#ifdef GRAAL_DEBUG_TOOL
	printk("nasrg_TOOL_get_protocol4 - begin \n");
#endif
  if (iph==NULL){
 	  printk("nasrg_TOOL_get_protocol4 - input parameter iph is NULL \n");
    return NULL;
  }
  if (protocol==NULL){
 	  printk("nasrg_TOOL_get_protocol4 - input parameter protocol is NULL \n");
    return NULL;
  }
// End debug information
	*protocol=iph->protocol;
	switch(*protocol){
	case IPPROTO_UDP:
	case IPPROTO_TCP:
	case IPPROTO_ICMP:
		return (u8 *)((u8 *)iph+iph->tot_len);
	default:
		return NULL;
	}
}

//---------------------------------------------------------------------------
// Convert the IMEI to iid
void nasrg_TOOL_imei2iid(u8 *imei, u8 *iid){
//---------------------------------------------------------------------------
// Start debug information
#ifdef GRAAL_DEBUG_TOOL
	printk("nasrg_TOOL_imei2iid - begin \n");
#endif
  if (imei==NULL){
 	  printk("nasrg_TOOL_imei2iid - input parameter imei is NULL \n");
    return;
  }
  if (iid==NULL){
 	  printk("nasrg_TOOL_imei2iid - input parameter iid is NULL \n");
    return;
  }
// End debug information
	memset(iid, 0, GRAAL_ADDR_LEN);
	iid[0] = 0x03;
	iid[1] = 16*imei[0]+imei[1];
	iid[2] = 16*imei[2]+imei[3];
	iid[3] = 16*imei[4]+imei[5];
	iid[4] = 16*imei[6]+imei[7];
	iid[5] = 16*imei[8]+imei[9];
	iid[6] = 16*imei[10]+imei[11];
	iid[7] = 16*imei[12]+imei[13];
}

//---------------------------------------------------------------------------
// Convert the RG IMEI to iid
void nasrg_TOOL_RGimei2iid(u8 *imei, u8 *iid){
//---------------------------------------------------------------------------
// Start debug information
#ifdef GRAAL_DEBUG_TOOL
	printk("nasrg_TOOL_RGimei2iid - begin \n");
#endif
  if (!imei || !iid){
 	  printk("nasrg_TOOL_RGimei2iid - input parameter imei or iid is NULL \n");
    return;
  }

// End debug information
	memset(iid, 0, GRAAL_ADDR_LEN);
	iid[0] = 0x00;   // to be compatible between link local and global
	iid[1] = 16*imei[0]+imei[1];
	iid[2] = 16*imei[2]+imei[3];
	iid[3] = 16*imei[4]+imei[5];
	iid[4] = 16*imei[6]+imei[7];
	iid[5] = 16*imei[8]+imei[9];
	iid[6] = 16*imei[10]+imei[11];
	iid[7] = 16*imei[12]+imei[13];
}

////---------------------------------------------------------------------------
//char *nasrg_TOOL_get_udpmsg(struct udphdr *udph){
////---------------------------------------------------------------------------
//// Start debug information
//#ifdef GRAAL_DEBUG_TOOL
//	printk("nasrg_TOOL_get_udpmsg - begin \n");
//#endif
//  if (udph==NULL){
// 	  printk("nasrg_TOOL_get_udpmsg - input parameter udph is NULL \n");
//    return NULL;
//  }
//// End debug information
//	return ((char *)udph+sizeof(struct udphdr));
//}

////---------------------------------------------------------------------------
//// Compute the UDP checksum (the data size must be odd)
//u16 nasrg_TOOL_udpcksum(struct in6_addr *saddr, struct in6_addr *daddr, u8 proto, u32 udplen, void *data){
////---------------------------------------------------------------------------
//	u32 i;
//  u16 *data16;
//	u32 csum=0;
//
//// Start debug information
//#ifdef GRAAL_DEBUG_TOOL
//	printk("nasrg_TOOL_udpcksum - begin \n");
//#endif
//  if (!saddr || !daddr || !data ){
// 	  printk("nasrg_TOOL_udpcksum - input parameter saddr || daddr || data is NULL \n");
//    return 0;
//  }
//// End debug information
//
//	data16=data;
//	for (i=0; i<8; ++i){
//		csum+=ntohs(saddr->s6_addr16[i]);
//		if (csum>0xffff)
//			csum-=0xffff;
//	}
//	for (i=0; i<8; ++i){
//		csum+=ntohs(daddr->s6_addr16[i]);
//		if (csum>0xffff)
//			csum-=0xffff;
//	}
//	csum+=(udplen>>16); // udplen checksum
//	if (csum>0xffff)
//		csum -= 0xffff;
//	csum+=udplen & 0xffff;
//	if (csum>0xffff)
//		csum -= 0xffff;
//	csum+=proto; // protocol checksum
//	if (csum>0xffff)
//		csum-=0xffff;
//	for (i = 0; 2*i < udplen; i++){
//		csum+=ntohs(data16[i]);
//		if (csum>0xffff)
//			csum-=0xffff;
//	}
//	return htons((u16)(~csum)&0xffff);
//}

////---------------------------------------------------------------------------
//void nasrg_TOOL_pk_udp(struct udphdr *udph){
////---------------------------------------------------------------------------
//// Start debug information
//#ifdef GRAAL_DEBUG_TOOL
//	printk("nasrg_TOOL_pk_udp - begin \n");
//#endif
//  if (udph==NULL){
// 	  printk("nasrg_TOOL_pk_udp - input parameter udph is NULL \n");
//    return;
//  }
//// End debug information
//	if (udph!=NULL)
//	{
//		printk("UDP:\t source = %u, dest = %u, len = %u, check = %x\n", ntohs(udph->source), ntohs(udph->dest), ntohs(udph->len), udph->check);
//	}
//}
//
////---------------------------------------------------------------------------
//void nasrg_TOOL_pk_tcp(struct tcphdr *tcph){
////---------------------------------------------------------------------------
//// Start debug information
//#ifdef GRAAL_DEBUG_TOOL
//	printk("nasrg_TOOL_pk_tcp - begin \n");
//#endif
//  if (tcph==NULL){
// 	  printk("nasrg_TOOL_pk_tcp - input parameter tcph is NULL \n");
//    return;
//  }
//// End debug information
//	if (tcph!=NULL)
//	{
//		printk("TCP:\t source = %u, dest = %u\n", tcph->source, tcph->dest);
//	}
//}
//
////---------------------------------------------------------------------------
//void nasrg_TOOL_pk_icmp6(struct icmp6hdr *icmph){
////---------------------------------------------------------------------------
//// Start debug information
//#ifdef GRAAL_DEBUG_TOOL
//	printk("nasrg_TOOL_pk_icmp6 - begin \n");
//#endif
//  if (icmph==NULL){
// 	  printk("nasrg_TOOL_pk_icmp6 - input parameter icmph is NULL \n");
//    return;
//  }
//// End debug information
//
//	if (icmph!=NULL){
//		printk("ICMPv6:\t type= %d, code = %d\n", icmph->icmp6_type, icmph->icmp6_code);
//		switch(icmph->icmp6_type){
//      case ICMPV6_DEST_UNREACH:printk("Destination unreachable\n");break;
//      case ICMPV6_PKT_TOOBIG:printk("Packet too big\n");break;
//      case ICMPV6_TIME_EXCEED:printk("Time exceeded\n");break;
//      case ICMPV6_PARAMPROB:printk("Parameter problem\n");break;
//      case ICMPV6_ECHO_REQUEST:printk("Echo request\n");break;
//      case ICMPV6_ECHO_REPLY:printk("Echo reply\n");break;
//      case ICMPV6_MGM_QUERY:printk("Multicast listener query\n");break;
//      case ICMPV6_MGM_REPORT:printk("Multicast listener report\n");break;
//      case ICMPV6_MGM_REDUCTION:printk("Multicast listener done\n");break;
//      case NDISC_ROUTER_SOLICITATION:printk("Router solicitation\n");break;
//      case NDISC_ROUTER_ADVERTISEMENT:printk("Router advertisment\n");break;
//      case NDISC_NEIGHBOUR_SOLICITATION:printk("Neighbour solicitation\n");break;
//      case NDISC_NEIGHBOUR_ADVERTISEMENT:printk("Neighbour advertisment\n");break;
//      case NDISC_REDIRECT:printk("redirect message\n");break;
//		}
//	}
//}
//
////---------------------------------------------------------------------------
//void nasrg_TOOL_pk_ipv6(struct ipv6hdr *iph){
////---------------------------------------------------------------------------
//// Start debug information
//#ifdef GRAAL_DEBUG_TOOL
//	printk("nasrg_TOOL_pk_ipv6 - begin \n");
//#endif
//  if (iph==NULL){
// 	  printk("nasrg_TOOL_pk_ipv6 - input parameter iph is NULL \n");
//    return;
//  }
//// End debug information
//
//	if (iph!=NULL){
////		char addr[GRAAL_INET6_ADDRSTRLEN];
//		printk("IP:\t version = %u, priority = %u, payload_len = %u\n", iph->version, iph->priority, ntohs(iph->payload_len));
//		printk("\t fl0 = %u, fl1 = %u, fl2 = %u\n",iph->flow_lbl[0],iph->flow_lbl[1],iph->flow_lbl[2]);
//		printk("\t next header = %u, hop_limit = %u\n", iph->nexthdr, iph->hop_limit);
////		inet_ntop(AF_INET6, (void *)(&iph->saddr), addr, GRAAL_INET6_ADDRSTRLEN);
////		printk("\t saddr = %s",addr);
////		inet_ntop(AF_INET6, (void *)(&iph->daddr), addr, GRAAL_INET6_ADDRSTRLEN);
////		printk(", daddr = %s\n",addr);
//		switch(iph->nexthdr){
//      case IPPROTO_UDP:
//      	nasrg_TOOL_pk_udp((struct udphdr *)((char *)iph+sizeof(struct ipv6hdr)));
//      	break;
//      case IPPROTO_TCP:
//      	nasrg_TOOL_pk_tcp((struct tcphdr *)((char *)iph+sizeof(struct ipv6hdr)));
//      	break;
//      case IPPROTO_ICMPV6:
//      	nasrg_TOOL_pk_icmp6((struct icmp6hdr *)((char *)iph+sizeof(struct ipv6hdr)));
//      	break;
//      case IPPROTO_IPV6:
//      	nasrg_TOOL_pk_ipv6((struct ipv6hdr *)((char *)iph+sizeof(struct ipv6hdr)));
//      	break;
//      default:
//      	printk("nasrg_TOOL_pk_ipv6 : Unknown upper layer\n");
//		}
//	}
//}
//
////---------------------------------------------------------------------------
//void nasrg_TOOL_pk_ipv4(struct iphdr *iph){
////---------------------------------------------------------------------------
//// Start debug information
//#ifdef GRAAL_DEBUG_TOOL
//	printk("nasrg_TOOL_pk_ipv4 - begin \n");
//#endif
//  if (iph==NULL){
// 	  printk("nasrg_TOOL_pk_ipv4 - input parameter iph is NULL \n");
//    return;
//  }
//// End debug information
//
//	if (iph!=NULL){
////		char addr[GRAAL_INET_ADDRSTRLEN];
//		printk("IP:\t version = %u, IP length = %u\n", iph->version, iph->ihl);
////		inet_ntop(AF_INET, (void *)(&iph->saddr), addr, GRAAL_INET_ADDRSTRLEN);
////		printk("\t saddr = %s", addr);
////		inet_ntop(AF_INET, (void *)(&iph->saddr), addr, GRAAL_INET_ADDRSTRLEN);
////		printk("\t saddr = %s", addr);
//	}
//}
//
////---------------------------------------------------------------------------
//// To be removed - Currently, never called
//void nasrg_TOOL_pk_all(struct sk_buff *skb){
////---------------------------------------------------------------------------
//// Start debug information
//#ifdef GRAAL_DEBUG_TOOL
//	printk("nasrg_TOOL_pk_all - begin \n");
//#endif
//  if (skb==NULL){
// 	  printk("nasrg_TOOL_pk_all - input parameter skb is NULL \n");
//    return;
//  }
//// End debug information
//
//	printk("Skb:\t %u, len = %u\n", (unsigned int)skb, skb->len);
//	printk("Skb:\t available buf space = %u, cur used space = %u \n", skb->end-skb->head, skb->tail-skb->data);
//	switch (ntohs(skb->protocol)){
//    case ETH_P_IPV6:
//    	nasrg_TOOL_pk_ipv6((skb->nh).ipv6h);
//    	break;
//    case ETH_P_IP:
//    	nasrg_TOOL_pk_ipv4((skb->nh).iph);
//    	break;
//	}
//}

//---------------------------------------------------------------------------
void nasrg_TOOL_print_state(u8 state){
//---------------------------------------------------------------------------
	switch(state){
  	case  GRAAL_IDLE:printk("GRAAL_IDLE\n");return;
    case  GRAAL_CX_FACH:printk("GRAAL_CX_FACH\n");return;
    case  GRAAL_CX_DCH:printk("GRAAL_CX_DCH\n");return;
    case  GRAAL_CX_RECEIVED:printk("GRAAL_CX_RECEIVED\n");return;
    case  GRAAL_CX_CONNECTING:printk("GRAAL_CX_CONNECTING\n");return;
    case  GRAAL_CX_RELEASING:printk("GRAAL_CX_RELEASING\n");return;
    case  GRAAL_CX_CONNECTING_FAILURE:printk("GRAAL_CX_CONNECTING_FAILURE\n");return;
    case  GRAAL_CX_RELEASING_FAILURE:printk("GRAAL_CX_RELEASING_FAILURE\n");return;
    case  GRAAL_RB_ESTABLISHING:printk("GRAAL_RB_ESTABLISHING\n");return;
    case  GRAAL_RB_RELEASING:printk("GRAAL_RB_RELEASING\n");return;
    case  GRAAL_RB_ESTABLISHED:printk("GRAAL_RB_ESTABLISHED\n");return;

  	default: printk(" Unknown state\n");
	}
}

//-----------------------------------------------------------------------------
// Print the content of a buffer in hexadecimal
void nasrg_TOOL_print_buffer(unsigned char * buffer,int length) {
//-----------------------------------------------------------------------------
   int i;

// Start debug information
#ifdef GRAAL_DEBUG_TOOL
	printk("nasrg_TOOL_print_buffer - begin \n");
#endif
  if (buffer==NULL){
 	  printk("\n nasrg_TOOL_print_buffer - input parameter buffer is NULL \n");
    return;
  }
// End debug information
   printk("\nBuffer content: ");
	 for (i=0;i<length;i++)
		 printk("-%hx-",buffer[i]);
 	 printk(",\t length %d\n", length);
}

//-----------------------------------------------------------------------------
void nasrg_TOOL_print_rb_entity(struct rb_entity *rb){
//-----------------------------------------------------------------------------
// Start debug information
#ifdef GRAAL_DEBUG_TOOL
	printk("nasrg_TOOL_print_rb_entity - begin \n");
#endif
  if (rb==NULL){
 	  printk("\n nasrg_TOOL_print_rb_entity - input parameter rb is NULL \n");
    return;
  }
// End debug information
   printk("\nrb_entity content: rab_id %d, sapi %d, qos %d, dscp %d, \n", rb->rab_id, rb->sapi, rb->qos, rb->dscp);
   printk("state %d, retry %d, countimer %d\n",rb->state, rb->retry, rb->countimer);
};

//-----------------------------------------------------------------------------
void nasrg_TOOL_print_classifier(struct classifier_entity *gc){
//-----------------------------------------------------------------------------
// Start debug information
#ifdef GRAAL_DEBUG_TOOL
	printk("nasrg_TOOL_print_classifier - begin \n");
#endif
  if (gc==NULL){
 	  printk("\n nasrg_TOOL_print_classifier - input parameter gc is NULL \n");
    return;
  }
// End debug information
   printk("\nClassifier content: classref %d, version %d, splen %d, dplen %d,\n", gc->classref, gc->version, gc->splen, gc->dplen);
   printk("protocol %d, sport %d, dport %d, rab_id %d\n", gc->protocol, gc->sport, gc->dport, gc->rab_id);
   nasrg_TOOL_print_rb_entity(gc->rb);
};


