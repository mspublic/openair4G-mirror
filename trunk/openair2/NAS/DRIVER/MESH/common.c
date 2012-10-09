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

/***************************************************************************
/*! \file nas_common.c
* \brief implementation of emultor tx and rx 
* \author Navid Nikaein and Raymomd Knopp
* \date 2011
* \version 1.0 
* \company Eurecom
* \email: navid.nikaein@eurecom.fr
*/ 

//#include "nas_common.h"
#include "local.h"
#include "proto_extern.h"
//#include "LAYER2/PDCP/pdcp.h"
#ifndef NAS_NETLINK
#include "rtai_fifos.h"
#endif

//#define NAS_DEBUG_RECEIVE 1
//#define NAS_DEBUG_SEND 1
//#define NAS_DEBUG_CLASS 1
//#define NAS_ADDRESS_FIX 1

#include <linux/inetdevice.h>

#include <net/tcp.h>
#include <net/udp.h>


void nas_COMMON_receive(u16 dlen, 
			void *pdcp_sdu,
			int inst,
			struct classifier_entity *rclass,
			nasRadioBearerId_t rb_id) {

  //---------------------------------------------------------------------------
  struct sk_buff *skb;
  struct ipversion *ipv;
  struct nas_priv *gpriv=netdev_priv(nasdev[inst]);
  u32 odaddr,osaddr;

  int i;

  unsigned char protocol;

  unsigned char *addr,*daddr,*saddr,*ifaddr,sn;

  struct udphdr *uh;
  struct tcphdr *th;
  u16 *cksum,check;

  struct iphdr *network_header; 

#ifdef NAS_DEBUG_RECEIVE

  printk("NAS_COMMON_RECEIVE: begin RB %d Inst %d Length %d bytes\n",rb_id,inst,dlen);
#endif
  skb = dev_alloc_skb( dlen + 2 );
  if(!skb)
    {
      printk("NAS_COMMON_RECEIVE: low on memory\n");
      ++gpriv->stats.rx_dropped;
      return;
    }
  skb_reserve(skb,2);
  memcpy(skb_put(skb, dlen), pdcp_sdu,dlen);

  skb->dev = nasdev[inst];
  
#ifdef KERNEL_VERSION_GREATER_THAN_2622
  skb->mac_header = skb->data;
#else
  skb->mac.raw = skb->data;
#endif
//printk("[NAC_COMMIN_RECEIVE]: Packet Type %d (%d,%d)",skb->pkt_type,PACKET_HOST,PACKET_BROADCAST);
  skb->pkt_type = PACKET_HOST;

  if (rclass->version != NAS_MPLS_VERSION_CODE) {  // This is an IP packet

    skb->ip_summed = CHECKSUM_NONE;


    ipv = (struct ipversion *)skb->data;
    switch (ipv->version)
      {
      case 6:
#ifdef NAS_DEBUG_RECEIVE
	printk("NAS_COMMON_RECEIVE: receive IPv6 message\n");
#endif


#ifdef KERNEL_VERSION_GREATER_THAN_2622
	skb->network_header = skb->data;
#else
	skb->nh.ipv6h = (struct ipv6hdr *)skb->data;
#endif

	skb->protocol = htons(ETH_P_IPV6);
//	printk("Writing packet with protocol %x\n",ntohs(skb->protocol));

	break;
      case 4:
#ifdef NAS_ADDRESS_FIX
	// Make the third byte of both the source and destination equal to the fourth of the destination



	daddr = (unsigned char *)&((struct iphdr *)skb->data)->daddr;
	odaddr = ((struct iphdr *)skb->data)->daddr;
	//		sn = addr[3];
	saddr = (unsigned char *)&((struct iphdr *)skb->data)->saddr;
	osaddr = ((struct iphdr *)skb->data)->saddr;
		  
	if (daddr[0] == saddr[0]) {// same network 
	  daddr[2] = daddr[3]; // set third byte of destination to that of local machine so that local IP stack accepts the packet
	  saddr[2] = daddr[3]; // set third byte of source to that of local machine so that local IP stack accepts the packet
	}
	else { // get the 3rd byte from device address in net_device structure
	  ifaddr = (unsigned char *)(&(((struct in_device *)((nasdev[inst])->ip_ptr))->ifa_list->ifa_local));
	  if (saddr[0] == ifaddr[0]) { // source is in same network as local machine
	    daddr[0] += saddr[3];        // fix address of remote destination to undo change at source
	    saddr[2] =  ifaddr[2];       // set third byte to that of local machine so that local IP stack accepts the packet
	  }
	  else {                         // source is remote machine from outside network
	    saddr[0] -= daddr[3];        // fix address of remote source to be understood by destination
	    daddr[2] =  daddr[3];        // fix 3rd byte of local address to be understood by IP stack of 
	    // destination 
	  }
	}
		  
#endif //NAS_ADDRESS_FIX
#ifdef NAS_DEBUG_RECEIVE
	//		printk("NAS_TOOL_RECEIVE: receive IPv4 message\n");
	addr = (unsigned char *)&((struct iphdr *)skb->data)->saddr;
	if (addr) {
	  //		  addr[2]^=0x01;
	  printk("[NAS][COMMON][RECEIVE] Source %d.%d.%d.%d\n",addr[0],addr[1],addr[2],addr[3]);
	}
	addr = (unsigned char *)&((struct iphdr *)skb->data)->daddr;
	if (addr){
	  //		  addr[2]^=0x01;
	  printk("[NAS][COMMON][RECEIVE] Dest %d.%d.%d.%d\n",addr[0],addr[1],addr[2],addr[3]);
	}
	printk("[NAS][COMMON][RECEIVE] protocol  %d\n",((struct iphdr *)skb->data)->protocol);
		  
#endif
		  
#ifdef KERNEL_VERSION_GREATER_THAN_2622
	skb->network_header = skb->data;
	network_header = (struct iphdr *)skb_network_header(skb);
	protocol = network_header->protocol;

#else
	skb->nh.iph = (struct iphdr *)skb->data;
	protocol=skb->nh.iph->protocol;
#endif 

#ifdef NAS_DEBUG_RECEIVE
	switch (protocol) {
	case IPPROTO_IP:
	  printk("[NAS][COMMON][RECEIVE] Received Raw IPv4 packet\n");
	  break;
	case IPPROTO_IPV6:
	  printk("[NAS][COMMON][RECEIVE] Received Raw IPv6 packet\n");
	  break;
	case IPPROTO_ICMP:
	  printk("[NAS][COMMON][RECEIVE] Received Raw ICMP packet\n");
	  break;
	case IPPROTO_TCP:
	  printk("[NAS][COMMON][RECEIVE] Received TCP packet\n");
	  break;
	case IPPROTO_UDP:
	  printk("[NAS][COMMON][RECEIVE] Received UDP packet\n");
	  break;
	default:
	  break;
	}

#endif
	
#ifdef NAS_ADDRESS_FIX

#ifdef NAS_DEBUG_RECEIVE
  printk("NAS_COMMON_RECEIVE: dumping the packet before the csum recalculation\n",skb->len);
  for (i=0;i<skb->len;i++)
    printk("%2x ",((unsigned char *)(skb->data))[i]); 
  printk("\n");
#endif //NAS_DEBUG_RECEIVE


#ifdef KERNEL_VERSION_GREATER_THAN_2622
	network_header->check = 0;
	network_header->check = ip_fast_csum((unsigned char *) network_header,
				 network_header->ihl);
#ifdef NAS_DEBUG_RECEIVE
	printk("[NAS][COMMON][RECEIVE] IP Fast Checksum %x \n", network_header->check);
#endif

#else
	skb->nh.iph->check = 0;
	skb->nh.iph->check = ip_fast_csum((unsigned char *)skb->data,
					  skb->nh.iph->ihl);

#ifdef NAS_DEBUG_RECEIVE
	printk("[NAS][COMMON][RECEIVE] IP Fast Checksum %x \n", skb->nh.iph->check);
#endif 

#endif 
	//	  if (!(skb->nh.iph->frag_off & htons(IP_OFFSET))) {
		    

		    
		    
	switch(protocol)
	  {
		
	  case IPPROTO_TCP:

#ifdef KERNEL_VERSION_GREATER_THAN_2622
	    cksum  = (u16*)&(((struct tcphdr*)((network_header + (network_header->ihl<<2))))->check);
	    //check  = csum_tcpudp_magic(((struct iphdr *)network_header)->saddr, ((struct iphdr *)network_header)->daddr, tcp_hdrlen(skb), IPPROTO_TCP, ~(*cksum));	    
#else
	    cksum  = (u16*)&(((struct tcphdr*)((skb->data + (skb->nh.iph->ihl<<2))))->check);
	    //check  = csum_tcpudp_magic(((struct iphdr *)skb->data)->saddr, ((struct iphdr *)skb->data)->daddr,tcp_hdrlen(skb), IPPROTO_TCP, ~(*cksum));
#endif

#ifdef NAS_DEBUG_RECEIVE
	    printk("[NAS][COMMON] Inst %d TCP packet calculated CS %x, CS = %x (before), SA (%x)%x, DA (%x)%x\n", 
		   inst, 
		   network_header->check,
		   *cksum,
		   osaddr,
		   ((struct iphdr *)skb->data)->saddr,
		   odaddr,
		   ((struct iphdr *)skb->data)->daddr);
#endif 
	    check  = csum_tcpudp_magic(((struct iphdr *)skb->data)->saddr, ((struct iphdr *)skb->data)->daddr,0,0, ~(*cksum));
	    *cksum = csum_tcpudp_magic(~osaddr, ~odaddr, 0, 0, ~check);
	    
#ifdef NAS_DEBUG_RECEIVE
	    printk("[NAS][COMMON] Inst %d TCP packet NEW CS %x\n",
		   inst,
		   *cksum);
#endif
	break;
	
	  case IPPROTO_UDP:

#ifdef KERNEL_VERSION_GREATER_THAN_2622
	    cksum  = (u16*)&(((struct udphdr*)((network_header + (network_header->ihl<<2))))->check);
	    // check = csum_tcpudp_magic(((struct iphdr *)network_header)->saddr, ((struct iphdr *)network_header)->daddr, udp_hdr(skb)->len, IPPROTO_UDP, ~(*cksum));
#else		
	    cksum  = (u16*)&(((struct udphdr*)((skb->data + (skb->nh.iph->ihl<<2))))->check);
	    //check = csum_tcpudp_magic(((struct iphdr *)skb->data)->saddr, ((struct iphdr *)skb->data)->daddr, udp_hdr(skb)->len, IPPROTO_UDP, ~(*cksum));
#endif 
#ifdef NAS_DEBUG_RECEIVE
	    printk("[NAS][COMMON] Inst %d UDP packet CS = %x (before), SA (%x)%x, DA (%x)%x\n", 
		   inst, 
		   *cksum,
		   osaddr,
		   ((struct iphdr *)skb->data)->saddr,
		   odaddr,
		   ((struct iphdr *)skb->data)->daddr);
#endif	
	    check = csum_tcpudp_magic(((struct iphdr *)skb->data)->saddr, ((struct iphdr *)skb->data)->daddr, 0,0, ~(*cksum));
	    *cksum= csum_tcpudp_magic(~osaddr, ~odaddr,0,0, ~check);
	    //*cksum= csum_tcpudp_magic(~osaddr, ~odaddr,udp_hdr(skb)->len, IPPROTO_UDP, ~check);

#ifdef NAS_DEBUG_RECEIVE
	    printk("[NAS][COMMON] Inst %d UDP packet NEW CS %x\n",
		   inst,
		   *cksum);
#endif	    	
	    //		if ((check = *cksum) != 0) {
	    // src, dst, len, proto, sum 

			  
	    //		}
		
	    break;
		      
	  default:
	    break;
	  }
	//	  }

	//#endif  // KERNEL VERSION > 22
#endif //NAS_ADDRESS_FIX

	skb->protocol = htons(ETH_P_IP);
//	printk("[NAS][COMMON] Writing packet with protocol %x\n",ntohs(skb->protocol));
	break;
      default:
	printk("NAS_COMMON_RECEIVE: begin RB %d Inst %d Length %d bytes\n",rb_id,inst,dlen);
		  
	printk("[NAS][COMMON] Inst %d: receive unknown message (version=%d)\n",inst,ipv->version);
		
      }
  }
  else {  // This is an MPLS packet

#ifdef NAS_DEBUG_RECEIVE
    printk("NAS_COMMON_RECEIVE: Received an MPLS packet on RB %d\n",rb_id);
#endif
    skb->protocol = htons(ETH_P_MPLS_UC);

  }
  ++gpriv->stats.rx_packets;
  gpriv->stats.rx_bytes += dlen;
#ifdef NAS_DEBUG_RECEIVE
  printk("NAS_COMMON_RECEIVE: sending packet of size %d to kernel\n",skb->len);
  for (i=0;i<skb->len;i++)
    printk("%2x ",((unsigned char *)(skb->data))[i]); 
  printk("\n");
#endif //NAS_DEBUG_RECEIVE
  netif_rx(skb);
#ifdef NAS_DEBUG_RECEIVE
  printk("NAS_COMMON_RECEIVE: end\n");
#endif
}

//---------------------------------------------------------------------------
// Delete the data
void nas_COMMON_del_send(struct sk_buff *skb, struct cx_entity *cx, struct classifier_entity *sp,int inst){
  struct nas_priv *priv=netdev_priv(nasdev[inst]);

  //---------------------------------------------------------------------------
  ++priv->stats.tx_dropped;
}

//---------------------------------------------------------------------------
// Request the transfer of data (QoS SAP)

void nas_COMMON_QOS_send(struct sk_buff *skb, struct cx_entity *cx, struct classifier_entity *gc,int inst){
  //---------------------------------------------------------------------------
  struct pdcp_data_req_header_t     pdcph;
  struct nas_priv *priv=netdev_priv(nasdev[inst]);
#ifdef LOOPBACK_TEST
  int i;
#endif
  unsigned int bytes_wrote;
  unsigned char j; 
  // Start debug information
#ifdef NAS_DEBUG_SEND
  printk("NAS_COMMON_QOS_SEND - inst %d begin \n",inst);
#endif
  //	if (cx->state!=NAS_STATE_CONNECTED) // <--- A REVOIR
  //	{
  //		priv->stats.tx_dropped ++;
  //		printk("NAS_QOS_SEND: No connected, so message are dropped \n");
  //		return;
  //	}
  if (skb==NULL){
#ifdef NAS_DEBUG_SEND
    printk("NAS_COMMON_QOS_SEND - input parameter skb is NULL \n");
#endif
    return;
  }
  if (gc==NULL){
#ifdef NAS_DEBUG_SEND
    printk("NAS_COMMON_QOS_SEND - input parameter gc is NULL \n");
#endif
    return;
  }
  if (cx==NULL){
#ifdef NAS_DEBUG_SEND
    printk("NAS_COMMON_QOS_SEND - input parameter cx is NULL \n");
#endif
    return;
  }
  // End debug information
  if (gc->rb==NULL)
    {
      gc->rb=nas_COMMON_search_rb(cx, gc->rab_id);
      if (gc->rb==NULL)
	{
	  ++priv->stats.tx_dropped;
	  printk("NAS_COMMON_QOS_SEND: No corresponding Radio Bearer, so message are dropped, rab_id=%u \n", gc->rab_id);
	  return;
	}
    }
#ifdef NAS_DEBUG_SEND
  printk("NAS_COMMON_QOS_SEND #1 :");
  printk("lcr %u, rab_id %u, rab_id %u, skb_len %d\n", cx->lcr, (gc->rb)->rab_id, gc->rab_id,skb->len);
  nas_print_classifier(gc);
#endif
  pdcph.data_size  = skb->len;
  pdcph.rb_id      = (gc->rb)->rab_id;
  pdcph.inst       = inst;


#ifdef NAS_NETLINK
  bytes_wrote = nas_netlink_send((char *)&pdcph,NAS_PDCPH_SIZE);
  printk("[NAS] Wrote %d bytes (header for %d byte skb) to PDCP via netlink\n",
  	       bytes_wrote,skb->len);
#else
  bytes_wrote = rtf_put(NAS2PDCP_FIFO, &pdcph, NAS_PDCPH_SIZE);
  printk("[NAS] Wrote %d bytes (header for %d byte skb) to PDCP fifo\n",
  	       bytes_wrote,skb->len);
#endif //NAS_NETLINK
  
  if (bytes_wrote != NAS_PDCPH_SIZE)
    {
      printk("NAS_COMMON_QOS_SEND: problem while writing PDCP's header (bytes wrote = %d to fifo %d)\n",bytes_wrote,NAS2PDCP_FIFO);
      printk("rb_id %d, Wrote %d, Header Size %d \n", pdcph.rb_id , bytes_wrote, NAS_PDCPH_SIZE);
#ifndef NAS_NETLINK
      rtf_reset(NAS2PDCP_FIFO);
#endif //NAS_NETLINK
      return;
    }
#ifdef  NAS_NETLINK
  bytes_wrote += nas_netlink_send((char *)skb->data,skb->len);
#else
  bytes_wrote += rtf_put(NAS2PDCP_FIFO, skb->data, skb->len);
#endif //NAS_NETLINK

  if (bytes_wrote != skb->len+NAS_PDCPH_SIZE)    
    {
      printk("NAS_COMMON_QOS_SEND: Inst %d, RB_ID %d: problem while writing PDCP's data, bytes_wrote = %d, Data_len %d, PDCPH_SIZE %d\n",
	     inst,
	     pdcph.rb_id,
	     bytes_wrote,
	     skb->len,
	     NAS_PDCPH_SIZE); // congestion
#ifndef NAS_NETLINK
      rtf_reset(NAS2PDCP_FIFO);
#endif //NAS_NETLINK
      return;
    }
#ifdef NAS_DEBUG_SEND
  printk("NAS_SEND: Sending packet of size %d to PDCP \n",skb->len);
 
 for (j=0;j<skb->len;j++)
    printk("%2x ",((unsigned char *)(skb->data))[j]); 
  printk("\n");
#endif

  priv->stats.tx_bytes   += skb->len;
  priv->stats.tx_packets ++;
#ifdef NAS_DEBUG_SEND
  printk("NAS_COMMON_QOS_SEND - end \n");
#endif
}

#ifndef NAS_NETLINK
//---------------------------------------------------------------------------
void nas_COMMON_QOS_receive(){
  //---------------------------------------------------------------------------
  u8 sapi;
  struct pdcp_data_ind_header_t     pdcph;
  unsigned char data_buffer[2048];
  struct classifier_entity *rclass;
  struct nas_priv *priv;
  int bytes_read;

  // Start debug information
#ifdef NAS_DEBUG_RECEIVE
  printk("NAS_COMMON_QOS_RECEIVE - begin \n");
#endif
  
  // End debug information
  
  bytes_read =  rtf_get(PDCP2NAS_FIFO,&pdcph, NAS_PDCPH_SIZE);
  
  while (bytes_read>0) {
    if (bytes_read != NAS_PDCPH_SIZE)
      {
	printk("NAS_COMMON_QOS_RECEIVE: problem while reading PDCP header\n");
	return;
      }
    
    priv=netdev_priv(nasdev[pdcph.inst]);
    rclass = nas_COMMON_search_class_for_rb(pdcph.rb_id,priv);
    
    bytes_read+= rtf_get(PDCP2NAS_FIFO, 
			 data_buffer,
			 pdcph.data_size);
    
#ifdef NAS_DEBUG_RECEIVE
    printk("NAS_COMMON_QOS_RECEIVE - Got header for RB %d, Inst %d \n",
	   pdcph.rb_id,
	   pdcph.inst);
#endif
    
    if (rclass) {
#ifdef NAS_DEBUG_RECEIVE
      printk("[NAS][COMMON] Found corresponding connection in classifier for RAB\n");
#endif //NAS_DEBUG_RECEIVE
      
      nas_COMMON_receive(pdcph.data_size, 
			 (void *)data_buffer,
			 pdcph.inst,
			 rclass,
			 pdcph.rb_id);
    }
    
    bytes_read =  rtf_get(PDCP2NAS_FIFO, &pdcph, NAS_PDCPH_SIZE);
  }
  
  
  
#ifdef NAS_DEBUG_RECEIVE
  printk("NAS_COMMON_QOS_RECEIVE - end \n");
#endif
}

#else
void nas_COMMON_QOS_receive(struct nlmsghdr *nlh)
{

  struct pdcp_data_ind_header_t     *pdcph = (struct pdcp_data_ind_header_t *)NLMSG_DATA(nlh);
  struct classifier_entity *rclass;
  struct nas_priv *priv;

  priv = netdev_priv(nasdev[pdcph->inst]);


#ifdef NAS_DEBUG_RECEIVE
  printk("[NAS][COMMON][NETLINK] QOS receive from PDCP, size %d, rab %d, inst %d\n",
	 pdcph->data_size,pdcph->rb_id,pdcph->inst);
#endif //NAS_DEBUG_RECEIVE

  rclass = nas_COMMON_search_class_for_rb(pdcph->rb_id,priv);

  if (rclass) {
#ifdef NAS_DEBUG_RECEIVE
    printk("[NAS][COMMON][NETLINK] Found corresponding connection in classifier for RAB\n");
#endif //NAS_DEBUG_RECEIVE
 
    nas_COMMON_receive(pdcph->data_size, 
		       (unsigned char *)NLMSG_DATA(nlh) + NAS_PDCPH_SIZE,
		       pdcph->inst,
		       rclass,
		       pdcph->rb_id);
  } 

}
#endif //NAS_NETLINK

//---------------------------------------------------------------------------
struct cx_entity *nas_COMMON_search_cx(nasLocalConnectionRef_t lcr,struct nas_priv *priv){
  //---------------------------------------------------------------------------
#ifdef NAS_DEBUG_CLASS
  printk("NAS_COMMON_SEARCH_CX - lcr %d\n",lcr);
#endif
  if (lcr<NAS_CX_MAX)
    return priv->cx+lcr;
  else
    return NULL;
}

//---------------------------------------------------------------------------
// Search a Radio Bearer
struct rb_entity *nas_COMMON_search_rb(struct cx_entity *cx, nasRadioBearerId_t rab_id){
  //---------------------------------------------------------------------------
  struct rb_entity *rb;
#ifdef NAS_DEBUG_CLASS
  printk("NAS_COMMON_SEARCH_RB - rab_id %d\n", rab_id);
#endif
  for (rb=cx->rb; rb!=NULL; rb=rb->next)
    {
      if (rb->rab_id==rab_id)
	return rb;
    }
  return NULL;
}

//
// Search for a classifier with corresponding radio bearer

struct classifier_entity *nas_COMMON_search_class_for_rb(nasRadioBearerId_t rab_id,struct nas_priv *priv) {

  struct rb_entity *rb;
  int dscp;
  struct classifier_entity *rclass;

#ifdef NAS_DEBUG_CLASS
  printk("[NAS][COMMON] NAS_COMMON_SEARCH_CLASS_FOR_RB - rab_id %d\n", rab_id);
#endif
  for (dscp=0;dscp<NAS_DSCP_MAX;dscp++) {

    //	    printk("[NAS][COMMON] priv->rclassifier[%d] = %p\n",dscp,priv->rclassifier[dscp]);
    for (rclass=priv->rclassifier[dscp]; rclass!=NULL; rclass=rclass->next) {
#ifdef NAS_DEBUG_CLASS
      printk("[NAS][COMMON] NAS_COMMON_SEARCH_CLASS_FOR_RB - dscp %d, rb %d\n", dscp,rclass->rab_id);
#endif
      if (rclass->rab_id==rab_id)
	return rclass;
    }
  }
  return NULL;
   
}

//---------------------------------------------------------------------------
struct rb_entity *nas_COMMON_add_rb(struct cx_entity *cx, nasRadioBearerId_t rab_id, nasQoSTrafficClass_t qos){
  //--------------------------------------------------------------------------
  struct rb_entity *rb;
#ifdef NAS_DEBUG_CLASS
  printk("NAS_COMMON_ADD_RB - begin for rab_id %d , qos %d\n", rab_id, qos );
#endif
  if (cx==NULL){
#ifdef NAS_DEBUG_CLASS
    printk("NAS_COMMON_ADD_RB - input parameter cx is NULL \n");
#endif
    return NULL;
  }
  rb=nas_COMMON_search_rb(cx, rab_id);
  if (rb==NULL)
    {
      rb=(struct rb_entity *)kmalloc(sizeof(struct rb_entity), GFP_KERNEL);
      if (rb!=NULL)
	{
	  rb->retry=0;
	  rb->countimer=NAS_TIMER_IDLE;
	  rb->rab_id=rab_id;
	  //			rb->rab_id=rab_id+(32*cx->lcr);
#ifdef NAS_DEBUG_DC
	  printk("NAS_COMMON_ADD_RB: rab_id=%u, mt_id=%u\n",rb->rab_id, cx->lcr);
#endif
	  rb->qos=qos;
	  rb->sapi=NAS_RAB_INPUT_SAPI;
	  rb->state=NAS_IDLE;
	  rb->next=cx->rb;
	  cx->rb=rb;
	  ++cx->num_rb;
	}
      else
	printk("NAS_ADD_CTL_RB: no memory\n");
    }
#ifdef NAS_DEBUG_CLASS
  printk("NAS_COMMON_ADD_RB - end \n" );
#endif
  return rb;
}

//---------------------------------------------------------------------------
void nas_COMMON_flush_rb(struct cx_entity *cx){
  //---------------------------------------------------------------------------
  struct rb_entity *rb;
  struct classifier_entity *gc;
  u8 dscp;
  // End debug information
#ifdef NAS_DEBUG_CLASS
  printk("NAS_COMMON_FLUSH_RB - begin\n");
#endif
  if (cx==NULL){
#ifdef NAS_DEBUG_CLASS
    printk("NAS_COMMON_FLUSH_RB - input parameter cx is NULL \n");
#endif
    return;
  }
  // End debug information
  for (rb=cx->rb; rb!=NULL; rb=cx->rb){
    printk("NAS_COMMON_FLUSH_RB: del rab_id %u\n", rb->rab_id);
    cx->rb=rb->next;
    kfree(rb);
  }
  cx->num_rb=0;
  cx->rb=NULL;
  for(dscp=0; dscp<NAS_DSCP_MAX; ++dscp)
    {
      for (gc=cx->sclassifier[dscp]; gc!=NULL; gc=gc->next)
	gc->rb=NULL;
    }
#ifdef NAS_DEBUG_CLASS
  printk("NAS_COMMON_FLUSH_RB - end\n");
#endif
}
