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
* \author A. Hafsaoui and Navid Nikaein
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

#ifdef ENABLE_DB_STATS
#include <mysql.h>
#include <m_ctype.h>
#include <sql_common.h>
#endif 

extern unsigned char NB_eNB_INST;
extern unsigned char NB_UE_INST;

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
  unsigned int lost_packet=0;
  //int header_size;

  if (buffer_tx!=NULL) { 
    otg_hdr_info_rx = (otg_hdr_info_t *) (&buffer_tx[bytes_read]);
    bytes_read += sizeof (otg_hdr_info_t);

    
    if (((otg_hdr_info_rx->flag == 0xffff)||(otg_hdr_info_rx->flag == 0xbbbb) || (otg_hdr_info_rx->flag == 0x1000)) && 
	(otg_hdr_info_rx->size ==size )){ //data traffic
      
      LOG_I(OTG,"MAX_RX_INFO %d %d \n",NB_eNB_INST,  NB_UE_INST);
      
      /*is_size_ok= 0;
	if (( otg_hdr_info_rx->size ) == size ) {*/
      is_size_ok= 1;
      otg_hdr_rx = (otg_hdr_t *) (&buffer_tx[bytes_read]);
      LOG_I(OTG,"[SRC %d][DST %d] [FLOW_idx %d][APP TYPE %d] RX INFO pkt at time %d: flag 0x %x, seq number %d, tx time %d, size (hdr %d, pdcp %d) \n", src, dst,otg_hdr_rx->flow_id,  otg_hdr_rx->traffic_type, ctime, otg_hdr_info_rx->flag, otg_hdr_rx->seq_num, otg_hdr_rx->time, otg_hdr_info_rx->size, size);
      bytes_read += sizeof (otg_hdr_t);
      
      if(otg_hdr_rx->traffic_type > MAX_NUM_APPLICATION){
	LOG_W(OTG,"RX packet: application type out of range %d for the pair of (src %d, dst %d) \n", 
	      otg_hdr_rx->traffic_type, src, dst);
	otg_hdr_rx->traffic_type=0;
    }
      
      if (otg_hdr_info_rx->flag == 0xffff){
	seq_num_rx=otg_info->seq_num_rx[src][dst][otg_hdr_rx->traffic_type];
	if (src<NB_eNB_INST)
	  nb_loss_pkts=otg_info->nb_loss_pkts_dl[src][dst][otg_hdr_rx->traffic_type];
	else
	  nb_loss_pkts=otg_info->nb_loss_pkts_ul[src][dst][otg_hdr_rx->traffic_type];
      }
      else if (otg_hdr_info_rx->flag == 0x1000){
	seq_num_rx = otg_multicast_info->rx_sn[src][dst][otg_hdr_rx->traffic_type];
	nb_loss_pkts = otg_multicast_info->loss_pkts_dl[src][dst][otg_hdr_rx->traffic_type];
	//	otg_multicast_info->ran_owd[src][dst][otg_hdr_rx->traffic_type] = ctime- otg_hdr_rx->time;
	//	rx_check_loss(src, dst, otg_hdr_info_rx->flag, otg_hdr_rx->seq_num, &seq_num_rx, &nb_loss_pkts);
	//	otg_multicast_info->loss_rate[src][dst][otg_hdr_rx->traffic_type]=nb_loss_pkts;
	//otg_multicast_info->rx_sn[src][dst][otg_hdr_rx->traffic_type]=seq_num_rx;
	//	LOG_I(OTG,"received a multicast packet with size %d sn %d ran owd %d loss rate %d\n",
	//	      otg_hdr_info_rx->size, seq_num_rx, ctime- otg_hdr_rx->time, nb_loss_pkts);
	//return 0;
	
      }
      else{
	seq_num_rx=otg_info->seq_num_rx_background[src][dst];
	if (src<NB_eNB_INST)
	  nb_loss_pkts=otg_info->nb_loss_pkts_background_dl[src][dst];
	else
	  nb_loss_pkts=otg_info->nb_loss_pkts_background_ul[src][dst];
      }

	
      LOG_D(OTG,"[%d][%d] AGGREGATION LEVEL (RX) %d \n", src, dst, otg_hdr_rx->aggregation_level);
      otg_info->aggregation_level[src][dst]=otg_hdr_rx->aggregation_level;
      
      /* Loss and out of sequence data management */
      lost_packet= rx_check_loss(src, dst, otg_hdr_info_rx->flag, otg_hdr_rx->seq_num, &seq_num_rx, &nb_loss_pkts);
      
      
      if (otg_info->owd_const[src][dst][otg_hdr_rx->flow_id]==0)
	owd_const_gen(src,dst,otg_hdr_rx->flow_id, otg_hdr_rx->traffic_type);
      
     
	/******/
/*
float owd_const_capillary_v=owd_const_capillary()/2;
float owd_const_mobile_core_v=owd_const_mobile_core()/2;
float owd_const_IP_backbone_v=owd_const_IP_backbone()/2;
float owd_const_application_v=owd_const_application()/2;

  FILE *file;
  file = fopen("/tmp/log_latency_m2m.txt", "a");
 fprintf(file," %d %d [%d] [%d]  %.2f %.2f %.2f %.2f %.2f %.2f\n", otg_hdr_rx->time, otg_hdr_info_rx->size, src, dst, owd_const_capillary_v,otg_info->radio_access_delay[src][dst],owd_const_mobile_core_v, owd_const_IP_backbone_v, owd_const_application_v, owd_const_capillary_v + otg_info->radio_access_delay[src][dst]+owd_const_mobile_core_v + owd_const_IP_backbone_v+ owd_const_application_v);
    fclose(file);
*/
/******/
	//  }
      
      if (otg_hdr_rx->time<=ctime){
	otg_info->radio_access_delay[src][dst]=ctime- otg_hdr_rx->time;
	otg_multicast_info->radio_access_delay[src][dst]=ctime- otg_hdr_rx->time;
      } else 
	LOG_N(OTG,"received packet has tx time %d greater than the current time %d\n",otg_hdr_rx->time,ctime );
      
      otg_info->rx_pkt_owd[src][dst]=otg_info->owd_const[src][dst][otg_hdr_rx->flow_id]+ otg_info->radio_access_delay[src][dst];
      otg_multicast_info->rx_pkt_owd[src][dst]=otg_multicast_info->radio_access_delay[src][dst];
      // compute the jitter by ignoring the packet loss
      if (lost_packet == 0){
	otg_info->rx_pkt_owd_history[src][dst][1] = otg_info->rx_pkt_owd_history[src][dst][0]; // the previous owd
	otg_info->rx_pkt_owd_history[src][dst][0] = otg_info->rx_pkt_owd[src][dst]; // the current owd
	if (otg_info->rx_pkt_owd_history[src][dst][1] == 0) // first packet
	  otg_info->rx_pkt_jitter[src][dst]=0;
	else // for the consecutive packets
	  otg_info->rx_pkt_jitter[src][dst]= abs(otg_info->rx_pkt_owd_history[src][dst][0] - otg_info->rx_pkt_owd_history[src][dst][1]);
  
	LOG_D(OTG,"The packet jitter for the pair (src %d, dst %d)) at %d is %lf (current %lf, previous %lf) \n",
	      src, dst, ctime, otg_info->rx_pkt_jitter[src][dst],
	      otg_info->rx_pkt_owd_history[src][dst][0], otg_info->rx_pkt_owd_history[src][dst][1]);
      }

      if (otg_hdr_info_rx->flag == 0x1000){
	LOG_I(OTG,"[SRC%d -> DST %d] Received a multicast packet at time %d with size %d, seq num %d, ran owd %d number loss packet %d\n",
	      dst, src, ctime,otg_hdr_info_rx->size, otg_hdr_rx->seq_num, ctime - otg_hdr_rx->time, nb_loss_pkts);
	
	LOG_I(OTG,"INFO LATENCY :: [SRC %d][DST %d] radio access %.2f (tx time %d, ctime %d), OWD:%.2f (ms):\n", 
	      src, dst, otg_multicast_info->radio_access_delay[src][dst], otg_hdr_rx->time, ctime , otg_multicast_info->rx_pkt_owd[src][dst]);

	if (otg_multicast_info->rx_owd_max[src][dst][otg_hdr_rx->traffic_type]==0){
	  otg_multicast_info->rx_owd_max[src][dst][otg_hdr_rx->traffic_type]=otg_multicast_info->rx_pkt_owd[src][dst];
	  otg_multicast_info->rx_owd_min[src][dst][otg_hdr_rx->traffic_type]=otg_multicast_info->rx_pkt_owd[src][dst];
	}	
	else {
	  otg_multicast_info->rx_owd_max[src][dst][otg_hdr_rx->traffic_type]=MAX(otg_multicast_info->rx_owd_max[src][dst][otg_hdr_rx->traffic_type],otg_multicast_info->rx_pkt_owd[src][dst] );
	  otg_multicast_info->rx_owd_min[src][dst][otg_hdr_rx->traffic_type]=MIN(otg_multicast_info->rx_owd_min[src][dst][otg_hdr_rx->traffic_type],otg_multicast_info->rx_pkt_owd[src][dst] );
	}
	if (g_otg->curve==1){ 
	  if (g_otg->owd_radio_access==0)
	    add_tab_metric(src, dst, otg_multicast_info->rx_pkt_owd[src][dst],  ((otg_hdr_info_rx->size*1000*8)/(otg_multicast_info->rx_pkt_owd[src][dst]*1024 )),  otg_hdr_rx->time);
	  else
	    add_tab_metric(src, dst, otg_multicast_info->radio_access_delay[src][dst],  ((otg_hdr_info_rx->size*1000*8)/(otg_multicast_info->rx_pkt_owd[src][dst]*1024 )),  otg_hdr_rx->time);    
	    }
	otg_multicast_info->rx_total_bytes_dl+=otg_hdr_info_rx->size;
      }
      else {
	
	LOG_I(OTG,"INFO LATENCY :: [SRC %d][DST %d] radio access %.2f (tx time %d, ctime %d), OWD:%.2f (ms):\n", 
	      src, dst, otg_info->radio_access_delay[src][dst], otg_hdr_rx->time, ctime , otg_info->rx_pkt_owd[src][dst]);

	if (otg_hdr_info_rx->flag == 0xffff){
	
	  if (otg_info->rx_owd_max[src][dst][otg_hdr_rx->traffic_type]==0){
	    otg_info->rx_owd_max[src][dst][otg_hdr_rx->traffic_type]=otg_info->rx_pkt_owd[src][dst];
	    otg_info->rx_owd_min[src][dst][otg_hdr_rx->traffic_type]=otg_info->rx_pkt_owd[src][dst];
	  }		
	  else {
	    otg_info->rx_owd_max[src][dst][otg_hdr_rx->traffic_type]=MAX(otg_info->rx_owd_max[src][dst][otg_hdr_rx->traffic_type],otg_info->rx_pkt_owd[src][dst] );
	    otg_info->rx_owd_min[src][dst][otg_hdr_rx->traffic_type]=MIN(otg_info->rx_owd_min[src][dst][otg_hdr_rx->traffic_type],otg_info->rx_pkt_owd[src][dst] );
	  }
	  if (otg_info->rx_jitter_max[src][dst][otg_hdr_rx->traffic_type]==0){
	    otg_info->rx_jitter_max[src][dst][otg_hdr_rx->traffic_type]=otg_info->rx_pkt_jitter[src][dst];
	    otg_info->rx_jitter_min[src][dst][otg_hdr_rx->traffic_type]=otg_info->rx_pkt_jitter[src][dst];
	  }		
	  else if (lost_packet==0){
	    otg_info->rx_jitter_max[src][dst][otg_hdr_rx->traffic_type]=MAX(otg_info->rx_jitter_max[src][dst][otg_hdr_rx->traffic_type],otg_info->rx_pkt_jitter[src][dst] );
	    otg_info->rx_jitter_min[src][dst][otg_hdr_rx->traffic_type]=MIN(otg_info->rx_jitter_min[src][dst][otg_hdr_rx->traffic_type],otg_info->rx_pkt_jitter[src][dst] );
	    // avg jitter
	    otg_info->rx_jitter_avg[src][dst][otg_hdr_rx->traffic_type] +=  otg_info->rx_pkt_jitter[src][dst]; 
	    otg_info->rx_jitter_sample[src][dst][otg_hdr_rx->traffic_type] +=1;
	  }
	  	  
	}
	 
	if (g_otg->curve==1){ 
	  if (g_otg->owd_radio_access==0)
	    add_tab_metric(src, dst, otg_info->rx_pkt_owd[src][dst],  ((otg_hdr_info_rx->size*1000*8)/(otg_info->rx_pkt_owd[src][dst]*1024 )),  otg_hdr_rx->time);
	  else
	    add_tab_metric(src, dst, otg_info->radio_access_delay[src][dst],  ((otg_hdr_info_rx->size*1000*8)/(otg_info->rx_pkt_owd[src][dst]*1024 )),  otg_hdr_rx->time);    
	}
	
	if (src<NB_eNB_INST)
	  otg_info->rx_total_bytes_dl+=otg_hdr_info_rx->size;
	else
	  otg_info->rx_total_bytes_ul+=otg_hdr_info_rx->size;
	
      }
      
      //LOG_I(OTG,"RX INFO :: RTT MIN(one way) ms: %.2f, RTT MAX(one way) ms: %.2f \n", otg_info->rx_owd_min[src][dst], otg_info->rx_owd_max[src][dst]);
      
      /* xforms part: add metrics  */	
    
      //printf("payload_size %d, header_size %d \n", otg_hdr_rx->pkts_size, otg_hdr_rx->hdr_type);
      LOG_I(OTG,"[RX] OTG packet, PACKET SIZE [SRC %d][DST %d]: Flag (0x%x), time(%d), Seq num (%d), Total size (%d)\n", src, dst, otg_hdr_info_rx->flag, ctime, otg_hdr_rx->seq_num, size);
      /*LOG_I(OTG,"details::RX [SRC %d][DST %d]: Flag (0x%x), time(%d), Seq num (%d), Total size (%d), header(%d), payload (%d) \n",  src, dst, otg_hdr_info_rx->flag, ctime, otg_hdr_rx->seq_num, size, strlen(packet_rx->header), strlen(packet_rx->payload));*/
      
      
      if (otg_hdr_info_rx->flag == 0xffff){
	otg_info->rx_num_pkt[src][dst][otg_hdr_rx->traffic_type]+=1;
	otg_info->rx_num_bytes[src][dst][otg_hdr_rx->traffic_type]+=otg_hdr_info_rx->size;
	otg_info->seq_num_rx[src][dst][otg_hdr_rx->traffic_type]=seq_num_rx;
	
	if (src<NB_eNB_INST)
	  otg_info->nb_loss_pkts_dl[src][dst][otg_hdr_rx->traffic_type]=nb_loss_pkts;
	else
	  otg_info->nb_loss_pkts_ul[src][dst][otg_hdr_rx->traffic_type]=nb_loss_pkts;		
	
	/*Plots of latency and goodput are only plotted for the data traffic*/
	/*measurements are done for the data and background traffic */
	
	if (g_otg->latency_metric) {
	  if (g_otg->owd_radio_access==0)
	    add_log_metric(src, dst, otg_hdr_rx->time, otg_info->rx_pkt_owd[src][dst], OTG_LATENCY); 
	  else {
	    add_log_metric(src, dst, otg_hdr_rx->time, otg_info->radio_access_delay[src][dst], OTG_LATENCY); 
	    add_log_metric(src, dst, otg_hdr_rx->time, otg_info->rx_pkt_jitter[src][dst], OTG_JITTER); 
	  }
	}
	
	if (g_otg->throughput_metric)
	  add_log_metric(src, dst, otg_hdr_rx->time, ((otg_hdr_info_rx->size*1000*8)/(otg_info->rx_pkt_owd[src][dst]*1024 )), OTG_GP); /* compute the throughput in Kbit/s  */
      
      } 
      else if (otg_hdr_info_rx->flag == 0x1000){
	otg_multicast_info->rx_num_pkt[src][dst][otg_hdr_rx->traffic_type]+=1;
	otg_multicast_info->rx_num_bytes[src][dst][otg_hdr_rx->traffic_type]+=otg_hdr_info_rx->size;
	//	LOG_D(OTG,"DUY: otg_multicast_info->rx_num_bytes[%d][%d][%d] is %d \nn",src,dst,otg_hdr_rx->traffic_type,otg_multicast_info->rx_num_bytes[src][dst][otg_hdr_rx->traffic_type]);
	otg_multicast_info->rx_sn[src][dst][otg_hdr_rx->traffic_type]=seq_num_rx;
	otg_multicast_info->loss_pkts_dl[src][dst][otg_hdr_rx->traffic_type]=nb_loss_pkts;

	/*Plots of latency and goodput are only plotted for the data traffic*/
	/*measurements are done for the data and background traffic */
	
	if (g_otg->latency_metric) {
	  if (g_otg->owd_radio_access==0)
	    add_log_metric(src, dst, otg_hdr_rx->time, otg_multicast_info->rx_pkt_owd[src][dst], OTG_LATENCY); 
	  else
	    add_log_metric(src, dst, otg_hdr_rx->time, otg_multicast_info->radio_access_delay[src][dst], OTG_LATENCY); 
	}
	if (g_otg->throughput_metric)
	  add_log_metric(src, dst, otg_hdr_rx->time, ((otg_hdr_info_rx->size*1000*8)/(otg_multicast_info->rx_pkt_owd[src][dst]*1024 )), OTG_GP); /* compute the throughput in Kbit/s  */
      }  
      else{
	otg_info->rx_num_pkt_background[src][dst]+=1;
	otg_info->rx_num_bytes_background[src][dst]+=otg_hdr_info_rx->size;
	otg_info->seq_num_rx_background[src][dst]=seq_num_rx;
	if (src<NB_eNB_INST)
	  otg_info->nb_loss_pkts_background_dl[src][dst]=nb_loss_pkts;
	else
	  otg_info->nb_loss_pkts_background_ul[src][dst]=nb_loss_pkts;
	
	if (g_otg->latency_metric) {
	  if (g_otg->owd_radio_access==0)
	    add_log_metric(src, dst, otg_hdr_rx->time, otg_info->rx_pkt_owd[src][dst], OTG_LATENCY_BG); 
	  else
	    add_log_metric(src, dst, otg_hdr_rx->time, otg_info->radio_access_delay[src][dst], OTG_LATENCY_BG); 
	}
	if (g_otg->throughput_metric)
	  add_log_metric(src, dst, otg_hdr_rx->time, ((otg_hdr_info_rx->size*1000*8)/(otg_info->rx_pkt_owd[src][dst]*1024 )), OTG_GP_BG);
      }
      
      if (is_size_ok == 0) {
	otg_hdr_rx = (otg_hdr_t *) (&buffer_tx[bytes_read]);
	LOG_W(OTG,"[SRC %d][DST %d] RX pkt: seq number %d size mis-matche (hdr %d, pdcp %d) \n", src, dst, otg_hdr_rx->seq_num, otg_hdr_info_rx->size, size);
	otg_info->nb_loss_pkts_otg[src][dst]++;
      }
      return(0);
    } else{
      LOG_W(OTG,"RX: Not an OTG pkt, forward to upper layer (flag %x, size %d, pdcp_size %d) FIX ME \n", otg_hdr_info_rx->flag, otg_hdr_info_rx->size, size);	
      return(0); //to be fixed on the real case to one 
    }
    
  }

  return(0);
}




int rx_check_loss(int src, int dst, unsigned int flag, int seq_num, unsigned int *seq_num_rx, unsigned int *nb_loss_pkts){

  /* Loss and out of sequence data management, we have 3 case : */
  /* (1) Receieved packet corresponds to the expected one, in terms of the sequence number*/
  int lost_packet=0;
  if (seq_num==*seq_num_rx) {
    LOG_D(OTG,"check_packet :: (src=%d,dst=%d, flag=0x%x) packet seq_num TX=%d, seq_num RX=%d \n",src,dst,flag, seq_num, *seq_num_rx);
    *seq_num_rx+=1;
  }
  /* (2) Receieved packet with a sequence number higher than the expected sequence number (there is a gap): packet loss */
  else if (seq_num>*seq_num_rx){ // out of sequence packet:  previous packet lost 
    LOG_D(OTG,"check_packet :: (src=%d,dst=%d, flag=0x%x) :: out of sequence :: packet seq_num TX=%d > seq_num RX=%d \n",src,dst,flag, seq_num, *seq_num_rx);
    *nb_loss_pkts+=seq_num-(*seq_num_rx);
    *seq_num_rx=seq_num+1;
    lost_packet=1;
  } 
  /* (3) Receieved packet with a sequence number less than the expected sequence number: recovery after loss/out of sequence  */
  else if (seq_num< *seq_num_rx){ //the received packet arrived late 
    *nb_loss_pkts-=1;
    LOG_D(OTG,"check_packet :: (src=%d,dst=%d, flag=0x%x) :: recovery after loss or out of sequence :: packet seq_num TX=%d < seq_num RX=%d \n",src,dst,flag, seq_num, *seq_num_rx);
  }
  else {
    LOG_D(OTG,"check_packet :: (src=%d,dst=%d, flag=0x%x) ::  packet seq_num TX=%d , seq_num RX=%d (ERROR)\n",src,dst,flag, seq_num, *seq_num_rx);
    lost_packet=1;
  }
  return lost_packet;
}




void owd_const_gen(int src, int dst, int flow_id, unsigned int flag){
  otg_info->owd_const[src][dst][flow_id]=(owd_const_mobile_core()+owd_const_IP_backbone()+owd_const_application())/2;

  if ((flag==M2M)||(flag==M2M_TRAFFIC)||(flag==AUTO_PILOT_L)||(flag==AUTO_PILOT_M)||(flag==AUTO_PILOT_H)||(flag==VIRTUAL_GAME_L)||(flag==VIRTUAL_GAME_M)|| (flag==VIRTUAL_GAME_H)||(flag==VIRTUAL_GAME_F)||(flag==ALARM_HUMIDITY)||(flag==ALARM_SMOKE)||(flag==ALARM_TEMPERATURE)||(flag==OPENARENA)||(flag==IQSIM_MANGO)||(flag==IQSIM_NEWSTEO)) {
    otg_info->owd_const[src][dst][flow_id]+=(owd_const_capillary()/2);
    LOG_D(OTG,"(RX) [src %d] [dst %d] [ID %d] TRAFFIC_TYPE IS M2M [Add Capillary const]\n", src, dst, flow_id);
  }
  else 
    LOG_D(OTG,"(RX) [src %d] [dst %d] [ID %d] TRAFFIC_TYPE WITHOUT M2M [Capillary const]\n", src, dst, flow_id);
}



float owd_const_capillary(){
  /*return (uniform_dist(MIN_APPLICATION_PROCESSING_GATEWAY_DELAY, MAX_APPLICATION_PROCESSING_GATEWAY_DELAY) + 
	   uniform_dist(MIN_FORMATING_TRANSFERRING_DELAY, MAX_FORMATING_TRANSFERRING_DELAY) + 
	   uniform_dist(MIN_ACCESS_DELAY, MAX_ACCESS_DELAY) + 
	   TERMINAL_ACCESS_DELAY);*/
	return ((double)MIN_APPLICATION_PROCESSING_GATEWAY_DELAY+ (double)MAX_APPLICATION_PROCESSING_GATEWAY_DELAY + (double)MIN_FORMATING_TRANSFERRING_DELAY+ (double)MAX_FORMATING_TRANSFERRING_DELAY+ (double)MIN_ACCESS_DELAY+(double)MAX_ACCESS_DELAY) /2 + (double)TERMINAL_ACCESS_DELAY;
}


float owd_const_mobile_core(){
  /*double delay;
  // this is a delay model for a loaded GGSN according to 
	//"M. Laner, P. Svoboda and M. Rupp, Latency Analysis of 3G Network Components, EW'12, Poznan, Poland, 2012", table 2, page 6.
  if(uniform_rng ()<0.3){
	delay=uniform_dist (0.4,1.2);
  }else{
	// in this case, according to the fit in the paper,
	//     the delay is generalized pareto: GP(k=0.75,s=0.55,t=1.2)
	//     using inverse cdf method we have CDF(x)=1-(k(x-t)/s+1)^(-1/k), 
	//	 x=CDF^(-1)(u)=t+s/k*((1-u)^(-k)-1) , hence when u~uniform, than x~GP(k,s,t)  

	double k,s,t,u;
	k=0.75;
	s=0.55;
	t=1.2;
	u=uniform_rng();
	delay= t + s/k*(pow(1-u,-k)-1);
  }
  return delay; */
  /*return ( uniform_dist(MIN_U_PLANE_CORE_IP_ACCESS_DELAY, MAX_U_PLANE_CORE_IP_ACCESS_DELAY) + uniform_dist(MIN_FW_PROXY_DELAY,MAX_FW_PROXY_DELAY)); */
  return ((double)MIN_U_PLANE_CORE_IP_ACCESS_DELAY+ (double)MAX_U_PLANE_CORE_IP_ACCESS_DELAY + (double)MIN_FW_PROXY_DELAY + (double)MAX_FW_PROXY_DELAY)/2;
}

float owd_const_IP_backbone(){
  /*return uniform_dist(MIN_NETWORK_ACCESS_DELAY,MAX_NETWORK_ACCESS_DELAY);*/
	return ((double)MIN_NETWORK_ACCESS_DELAY+(double)MAX_NETWORK_ACCESS_DELAY)/2;

}

float owd_const_application(){
  /*return uniform_dist(MIN_APPLICATION_ACESS_DELAY, MAX_APPLICATION_ACESS_DELAY);*/
	return ((double)MIN_APPLICATION_ACESS_DELAY+(double)MAX_APPLICATION_ACESS_DELAY)/2;
}




