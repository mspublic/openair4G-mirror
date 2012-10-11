/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2011 Eurecom

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

/*! \file otg_rx.c
* \brief function containing the OTG traffic generation functions 
* \author A. Hafsaoui
* \date 2011
* \version 0.1
* \company Eurecom
* \email: 
* \note
* \warning
*/

#include "otg_rx.h"
#include "otg_vars.h"
#include "../UTIL/MATH/oml.h"
#include <math.h>
#include "otg_form.h"

//#include "LAYER2/MAC/extern.h"

#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)<(y)?(x):(y))



// Check if the packet is well received or not and extract data
int otg_rx_pkt( int src, int dst, int ctime, char *buffer_tx, unsigned int size){
  
  int bytes_read=0;
  otg_hdr_info_t * otg_hdr_info_rx;
  otg_hdr_t * otg_hdr_rx;
  int is_size_ok=0;
  unsigned int seq_num_rx;
  unsigned int nb_loss_pkts;
  //packet_t *packet_rx=NULL;
  //float owd_mobile_core=0;
  //float owd_IP_backbone=0;
  //float owd_application=0;
char * hdr_payload=NULL;
int header_size;

  if (buffer_tx!=NULL) { 
    otg_hdr_info_rx = (otg_hdr_info_t *) (&buffer_tx[bytes_read]);
    bytes_read += sizeof (otg_hdr_info_t);



    if ((otg_hdr_info_rx->flag == 0xffff)||(otg_hdr_info_rx->flag == 0xbbbb)){ //data traffic
      /*is_size_ok= 0;
      if (( otg_hdr_info_rx->size ) == size ) {*/
      is_size_ok= 1;
	otg_hdr_rx = (otg_hdr_t *) (&buffer_tx[bytes_read]);
	LOG_I(OTG,"[SRC %d][DST %d] RX INFO pkt at time %d: flag 0x %x, seq number %d, tx time %d, size (hdr %d, pdcp %d) \n", src, dst,ctime, otg_hdr_info_rx->flag, otg_hdr_rx->seq_num, otg_hdr_rx->time, otg_hdr_info_rx->size, size);
	 bytes_read += sizeof (otg_hdr_t);

	      /*if (( otg_hdr_info_rx->size ) != size ) {
		header_size=bytes_read;
		hdr_payload=(char*)malloc((size-bytes_read)*sizeof(char)); //size-bytes_read
		//char hdr_payload[size-bytes_read];
 		memcpy(hdr_payload, &buffer_tx[bytes_read], size-bytes_read); 
		hdr_payload[size-bytes_read-1]=0;
		printf("[SRC %d][DST %d] FALSE::RX pkt at (time %d, ctime %d) flag 0x %x, seq number %d, 
                size (hdr %d, pdcp %d, other %d) hdr %d: %s: %c \n", src, dst,otg_hdr_rx->time, ctime, otg_hdr_info_rx->flag, 
                otg_hdr_rx->seq_num, otg_hdr_info_rx->size,   size, header_size+strlen(hdr_payload),header_size,hdr_payload, buffer_tx[size-2]);
		printf("PACKET_RX hdr %d , %d, strlen %d  %s end %c \n", bytes_read, size-bytes_read,strlen(hdr_payload), hdr_payload, buffer_tx[size-2]);
		free(hdr_payload);
	      }*/

        if (otg_hdr_info_rx->flag == 0xffff){
          seq_num_rx=otg_info->seq_num_rx[src][dst];
          nb_loss_pkts=otg_info->nb_loss_pkts[src][dst];
        }
	else{
          seq_num_rx=otg_info->seq_num_rx_background[src][dst];
          nb_loss_pkts=otg_info->nb_loss_pkts_background[src][dst];
        }


	LOG_D(OTG,"[%d][%d] AGGREGATION LEVEL (RX) %d \n", src, dst, otg_hdr_rx->aggregation_level);

	/* Loss and out of sequence data management */
	if ((otg_hdr_rx->seq_num)==seq_num_rx+1) {
	  LOG_D(OTG,"check_packet :: (i=%d,j=%d) packet seq_num TX=%d, seq_num RX=%d \n",src,dst, otg_hdr_rx->seq_num, seq_num_rx+1);
	  seq_num_rx+=1;
	}
	else if ((otg_hdr_rx->seq_num)>seq_num_rx+1){ // out of sequence packet:  previous packet lost 
	  LOG_D(OTG,"check_packet :: (i=%d,j=%d) :: out of sequence :: packet seq_num TX=%d > seq_num RX=%d \n",src,dst, otg_hdr_rx->seq_num, seq_num_rx+1);
	  nb_loss_pkts+=((otg_hdr_rx->seq_num)-(seq_num_rx+1));
	  seq_num_rx=otg_hdr_rx->seq_num;
	} 
	else if ((otg_hdr_rx->seq_num)< seq_num_rx+1){ //the received packet arrived late 
	  nb_loss_pkts-=1;
	  LOG_D(OTG,"check_packet :: (i=%d,j=%d) :: out of sequence :: packet seq_num TX=%d < seq_num RX=%d \n",src,dst, otg_hdr_rx->seq_num, seq_num_rx+1);
	}
        /* End Loss and out of sequence data management */

	if (otg_info->owd_const[src][dst]==0)
	  owd_const_gen(src,dst,1);



	if (otg_hdr_rx->time<=ctime){
	  otg_info->radio_access_delay[src][dst]=(float)(ctime- otg_hdr_rx->time);
         }

	
 	otg_info->rx_pkt_owd[src][dst]=otg_info->owd_const[src][dst]+ otg_info->radio_access_delay[src][dst];
	

	LOG_I(OTG,"INFO LATENCY :: [SRC %d][DST %d] radio access %f (tx time %d, ctime %d), OWD:%f (ms):\n", src, dst, otg_info->radio_access_delay[src][dst], otg_hdr_rx->time, ctime , otg_info->rx_pkt_owd[src][dst]);

        /*if (g_otg->latency_metric==1)
          add_log_metric(src, dst, ctime, otg_info->radio_access_delay[src][dst], OTG_LATENCY); // TO FIX !!!! segmentation fault */
	

	if (otg_info->rx_owd_max[src][dst]==0){
	  otg_info->rx_owd_max[src][dst]=otg_info->rx_pkt_owd[src][dst];
	  otg_info->rx_owd_min[src][dst]=otg_info->rx_pkt_owd[src][dst];
	}
	else {
	  otg_info->rx_owd_max[src][dst]=MAX(otg_info->rx_owd_max[src][dst],otg_info->rx_pkt_owd[src][dst] );
	  otg_info->rx_owd_min[src][dst]=MIN(otg_info->rx_owd_min[src][dst],otg_info->rx_pkt_owd[src][dst] );
	}
	LOG_I(OTG,"RX INFO :: RTT MIN(one way) ms: %d, RTT MAX(one way) ms: %d \n", otg_info->rx_owd_min[src][dst], otg_info->rx_owd_max[src][dst]);
	
	// xforms part	
	if (g_otg->curve==1){ 
          if (g_otg->owd_radio_access==0)
            add_tab_metric(src, dst, otg_info->rx_pkt_owd[src][dst], otg_hdr_info_rx->size*8/otg_info->rx_pkt_owd[src][dst],  ctime);
          else
            add_tab_metric(src, dst,  otg_info->radio_access_delay[src][dst], otg_hdr_info_rx->size*8/otg_info->rx_pkt_owd[src][dst],  ctime);    
        }

//printf("payload_size %d, header_size %d \n", otg_hdr_rx->pkts_size, otg_hdr_rx->hdr_type);
  LOG_I(OTG,"PACKET SIZE RX [SRC %d][DST %d]: Flag (0x%x), time(%d), Seq num (%d), Total size (%d)\n", src, dst, otg_hdr_info_rx->flag, ctime, otg_hdr_rx->seq_num, size);
 /*LOG_I(OTG,"details::RX [SRC %d][DST %d]: Flag (0x%x), time(%d), Seq num (%d), Total size (%d), header(%d), payload (%d) \n",  src, dst, otg_hdr_info_rx->flag, ctime, otg_hdr_rx->seq_num, size, strlen(packet_rx->header), strlen(packet_rx->payload));*/


 	if (otg_hdr_info_rx->flag == 0xffff){
 	  otg_info->rx_num_pkt[src][dst]+=1;
	  otg_info->rx_num_bytes[src][dst]+=otg_hdr_info_rx->size;
          otg_info->seq_num_rx[src][dst]=seq_num_rx;
          otg_info->nb_loss_pkts[src][dst]=nb_loss_pkts;		
        }
	else{
	  otg_info->rx_num_pkt_background[src][dst]+=1;
	  otg_info->rx_num_bytes_background[src][dst]+=otg_hdr_info_rx->size;
	  otg_info->seq_num_rx_background[src][dst]=seq_num_rx;
          otg_info->nb_loss_pkts_background[src][dst]=nb_loss_pkts;
	}

      if (is_size_ok == 0) {
	otg_hdr_rx = (otg_hdr_t *) (&buffer_tx[bytes_read]);
	LOG_I(OTG,"[SRC %d][DST %d] RX pkt: seq number %d size mis-matche (hdr %d, pdcp %d) \n", src, dst, otg_hdr_rx->seq_num, otg_hdr_info_rx->size, size);
      }
      return(0);
    } else{
      LOG_I(OTG,"RX: Not an OTG pkt, forward to upper layer (flag %x, size %d, pdcp_size %d) FIX ME \n", otg_hdr_info_rx->flag, otg_hdr_info_rx->size, size);	
      return(0); //????? have to be fixed on the real case to one 
    }
   
  }

  return(0);
}



void owd_const_gen(int src, int dst, unsigned int flag){
  otg_info->owd_const[src][dst]=owd_const_capillary()+owd_const_mobile_core()+owd_const_IP_backbone();
}



float owd_const_capillary(){
  float capillary_domain_latency=0;
  capillary_domain_latency=uniform_dist(MIN_APPLICATION_PROCESSING_GATEWAY_DELAY, MAX_APPLICATION_PROCESSING_GATEWAY_DELAY) + uniform_dist(MIN_FORMATING_TRANSFERRING_DELAY, MAX_FORMATING_TRANSFERRING_DELAY) + uniform_dist(MIN_ACCESS_DELAY, MAX_ACCESS_DELAY) + TERMINAL_ACCESS_DELAY;
  return capillary_domain_latency;
}


float owd_const_mobile_core(){
  float mobile_core_domain_latency=0;
  mobile_core_domain_latency= uniform_dist(MIN_U_PLANE_CORE_IP_ACCESS_DELAY, MAX_U_PLANE_CORE_IP_ACCESS_DELAY) +  uniform_dist(MIN_FW_PROXY_DELAY,MAX_FW_PROXY_DELAY);
return mobile_core_domain_latency;

}

float owd_const_IP_backbone(){
  float IP_backbone_domain_latency=0;
  IP_backbone_domain_latency= uniform_dist(MIN_NETWORK_ACCESS_DELAY,MAX_NETWORK_ACCESS_DELAY);;
  return IP_backbone_domain_latency;
}


float owd_const_application(){
  float application_latency=0;
  application_latency= uniform_dist(MIN_APPLICATION_ACESS_DELAY, MAX_APPLICATION_ACESS_DELAY);
  return application_latency;
}




