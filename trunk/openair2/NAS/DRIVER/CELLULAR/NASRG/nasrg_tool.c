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
#ifdef NAS_DEBUG_TOOL
 printk("nasrg_TOOL_fct - begin \n");
#endif
  if (gc==NULL){
    printk("nasrg_TOOL_fct - input parameter gc is NULL \n");
    return;
  }
// End debug information
 switch(fct){
     case NAS_FCT_QOS_SEND:
      gc->fct=nasrg_COMMON_QOS_send;
      break;
     case NAS_FCT_CTL_SEND:
      gc->fct=nasrg_CTL_send;
      break;
     case NAS_FCT_DC_SEND:
      gc->fct=nasrg_ASCTL_DC_send_sig_data_request;
      break;
     case NAS_FCT_DEL_SEND:
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
#ifdef NAS_DEBUG_TOOL
 printk("nasrg_TOOL_invfct - begin \n");
#endif
  if (gc==NULL){
    printk("nasrg_TOOL_invfct - input parameter gc is NULL \n");
    return 0;
  }
// End debug information
 if (gc->fct==nasrg_COMMON_QOS_send)
  return NAS_FCT_QOS_SEND;
 if (gc->fct==nasrg_CTL_send)
  return NAS_FCT_CTL_SEND;
 if (gc->fct==nasrg_COMMON_del_send)
  return NAS_FCT_DEL_SEND;
 if (gc->fct==nasrg_ASCTL_DC_send_sig_data_request)
  return NAS_FCT_DC_SEND;
 return 0;
}

//---------------------------------------------------------------------------
u8 nasrg_TOOL_get_dscp6(struct ipv6hdr *iph){
//---------------------------------------------------------------------------
// Start debug information
#ifdef NAS_DEBUG_TOOL
 printk("nasrg_TOOL_get_dscp6 - begin \n");
#endif
  if (iph==NULL){
    printk("nasrg_TOOL_get_dscp6 - input parameter iph is NULL \n");
    return 0;
  }
// End debug information
// return (ntohl(((*(__u32 *)iph)&NAS_TRAFFICCLASS_MASK)))>>22; //Yan
 return (ntohl(((*(__u32 *)iph)&NAS_TRAFFICCLASS_MASK)))>>20;
}

//---------------------------------------------------------------------------
u8 nasrg_TOOL_get_dscp4(struct iphdr *iph){
//---------------------------------------------------------------------------
// Start debug information
#ifdef NAS_DEBUG_TOOL
 printk("nasrg_TOOL_get_dscp4 - begin \n");
#endif
  if (iph==NULL){
    printk("nasrg_TOOL_get_dscp4 - input parameter iph is NULL \n");
    return 0;
  }
// End debug information
 return ((iph->tos)>>5)<<3;
}

//---------------------------------------------------------------------------
u8 *nasrg_TOOL_get_protocol6(struct ipv6hdr *iph, u8 *protocol){
//---------------------------------------------------------------------------
 u16 size;
// Start debug information
#ifdef NAS_DEBUG_TOOL
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
 size=NAS_IPV6_SIZE;
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
#ifdef NAS_DEBUG_TOOL
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
#ifdef NAS_DEBUG_TOOL
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
  memset(iid, 0, NAS_ADDR_LEN);
  iid[0] = 0x03;
#ifdef NAS_DRIVER_TYPE_ETHERNET
  iid[0] = 0x02;
#endif
  iid[1] = 16*imei[0]+imei[1];
  iid[2] = 16*imei[2]+imei[3];
  iid[3] = 16*imei[4]+imei[5];
  iid[4] = 16*imei[6]+imei[7];
#ifdef NAS_DRIVER_TYPE_ETHERNET
  iid[3] = 0xff;
  iid[4] = 0xfe;
#endif
  iid[5] = 16*imei[8]+imei[9];
  iid[6] = 16*imei[10]+imei[11];
  iid[7] = 16*imei[12]+imei[13];
}

//---------------------------------------------------------------------------
// Convert the RG IMEI to iid
void nasrg_TOOL_RGimei2iid(u8 *imei, u8 *iid){
//---------------------------------------------------------------------------
// Start debug information
#ifdef NAS_DEBUG_TOOL
 printk("nasrg_TOOL_RGimei2iid - begin \n");
#endif
  if (!imei || !iid){
    printk("nasrg_TOOL_RGimei2iid - input parameter imei or iid is NULL \n");
    return;
  }

// End debug information
 memset(iid, 0, NAS_ADDR_LEN);
 iid[0] = 0x00;   // to be compatible between link local and global
 iid[1] = 16*imei[0]+imei[1];
 iid[2] = 16*imei[2]+imei[3];
 iid[3] = 16*imei[4]+imei[5];
 iid[4] = 16*imei[6]+imei[7];
 iid[5] = 16*imei[8]+imei[9];
 iid[6] = 16*imei[10]+imei[11];
 iid[7] = 16*imei[12]+imei[13];
}

//---------------------------------------------------------------------------
void nasrg_TOOL_pk_icmp6(struct icmp6hdr *icmph){
//---------------------------------------------------------------------------
// Start debug information
#ifdef GRAAL_DEBUG_TOOL
  printk("nasrg_TOOL_pk_icmp6 - begin \n");
#endif
  if (!icmph){
     printk("nasrg_TOOL_pk_icmp6 - input parameter icmph is NULL \n");
    return;
  }
// End debug information
  printk("ICMPv6:\t type= %d, code = %d\n", icmph->icmp6_type, icmph->icmp6_code);
  switch(icmph->icmp6_type){
    case ICMPV6_DEST_UNREACH:printk("Destination unreachable\n");break;
    case ICMPV6_PKT_TOOBIG:printk("Packet too big\n");break;
    case ICMPV6_TIME_EXCEED:printk("Time exceeded\n");break;
    case ICMPV6_PARAMPROB:printk("Parameter problem\n");break;
    case ICMPV6_ECHO_REQUEST:printk("Echo request\n");break;
    case ICMPV6_ECHO_REPLY:printk("Echo reply\n");break;
    case ICMPV6_MGM_QUERY:printk("Multicast listener query\n");break;
    case ICMPV6_MGM_REPORT:printk("Multicast listener report\n");break;
    case ICMPV6_MGM_REDUCTION:printk("Multicast listener done\n");break;
    case NDISC_ROUTER_SOLICITATION:printk("Router solicitation\n");break;
    case NDISC_ROUTER_ADVERTISEMENT:printk("Router advertisment\n");break;
    case NDISC_NEIGHBOUR_SOLICITATION:printk("Neighbour solicitation\n");break;
    case NDISC_NEIGHBOUR_ADVERTISEMENT:printk("Neighbour advertisment\n");break;
    case NDISC_REDIRECT:printk("redirect message\n");break;
  }
}


//---------------------------------------------------------------------------
void nasrg_TOOL_print_state(u8 state){
//---------------------------------------------------------------------------
  switch(state){
    case  NAS_IDLE:printk("NAS_IDLE\n");return;
    case  NAS_CX_FACH:printk("NAS_CX_FACH\n");return;
    case  NAS_CX_DCH:printk("NAS_CX_DCH\n");return;
    case  NAS_CX_RECEIVED:printk("NAS_CX_RECEIVED\n");return;
    case  NAS_CX_CONNECTING:printk("NAS_CX_CONNECTING\n");return;
    case  NAS_CX_RELEASING:printk("NAS_CX_RELEASING\n");return;
    case  NAS_CX_CONNECTING_FAILURE:printk("NAS_CX_CONNECTING_FAILURE\n");return;
    case  NAS_CX_RELEASING_FAILURE:printk("NAS_CX_RELEASING_FAILURE\n");return;
    case  NAS_RB_ESTABLISHING:printk("NAS_RB_ESTABLISHING\n");return;
    case  NAS_RB_RELEASING:printk("NAS_RB_RELEASING\n");return;
    case  NAS_RB_ESTABLISHED:printk("NAS_RB_ESTABLISHED\n");return;
    default: printk(" Unknown state\n");
  }
}

//-----------------------------------------------------------------------------
// Print the content of a buffer in hexadecimal
void nasrg_TOOL_print_buffer(unsigned char * buffer,int length) {
//-----------------------------------------------------------------------------
   int i;

// Start debug information
#ifdef NAS_DEBUG_TOOL
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
#ifdef NAS_DEBUG_TOOL
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
#ifdef NAS_DEBUG_TOOL
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


