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

#include"otg_rx.h"

// Check if the packet is well received or not and extract data
int check_packet( int src, int dst, int ctime){
	
	int status_ok=0;
	packet_t* packet_rx = (packet_t*)buffer_tx;
	set_ctime(ctime);



//  Free the received packet if it is well received

	if (NULL != packet_rx){

		
		if (packet_rx->otg_hdr->seq_num==otg_info->rx_num_pkt[src][dst]+1){
			LOG_T(OTG,"check_packet :: (i=%d,j=%d) packet seq_num TX=%d, num_pkts RX=%d \n",src,dst, packet_rx->otg_hdr->seq_num, otg_info->rx_num_pkt[src][dst]+1);
			status_ok=1;
		if (packet_rx->header!=NULL)
			LOG_I(OTG,"RX NFO ::  Header :%s \n", packet_rx->header);
		if (packet_rx->payload!=NULL)
			LOG_I(OTG,"RX INFO ::  Payload :%s \n", packet_rx->payload);
		LOG_I(OTG,"RX INFO ::  flow id :%d \n", packet_rx->otg_hdr->flow_id);
		LOG_I(OTG,"RX INFO :: header type: %d \n", packet_rx->otg_hdr->hdr_type);
		LOG_I(OTG,"RX INFO :: time: %d \n", packet_rx->otg_hdr->time);
		LOG_I(OTG,"RX INFO :: Sequence NB: %d \n", packet_rx->otg_hdr->seq_num);

			// Compute STAT
		otg_info->rx_num_pkt[src][dst]+=1;
		LOG_I(OTG,"PACKET SIZE (RX):  time(%d), otg header(%d), header (%d), payload (%d), Total (%d) \n", ctime, sizeof(otg_hdr_t), strlen(packet_rx->header), strlen(packet_rx->payload),( sizeof(otg_hdr_t) + strlen(packet_rx->header) + strlen(packet_rx->payload)));
		otg_info->rx_num_bytes[src][dst]+= sizeof(otg_hdr_t) +  strlen(packet_rx->header) + strlen(packet_rx->payload) ;

//printf("time_diff current=%d, previous=%d \n", get_ctime() , packet_rx->otg_hdr->time);
		otg_info->rx_pkt_owd[src][dst]= get_ctime() - packet_rx->otg_hdr->time ;
	
	
		LOG_I(OTG,"RX INFO :: RTT (one way) ms: %d \n", otg_info->rx_pkt_owd[src][dst]);
	
		if ((otg_info->rx_pkt_owd[src][dst] > otg_info->rx_owd_max[src][dst]) || (otg_info->rx_owd_min[src][dst]==0))
				otg_info->rx_owd_max[src][dst]=otg_info->rx_pkt_owd[src][dst];

		if  ((otg_info->rx_pkt_owd[src][dst] < otg_info->rx_owd_min[src][dst]) || (otg_info->rx_owd_min[src][dst]==0)) 
				otg_info->rx_owd_min[src][dst]=otg_info->rx_pkt_owd[src][dst];

		LOG_I(OTG,"RX INFO :: RTT MIN(one way) ms: %d, RTT MAX(one way) ms: %d \n", otg_info->rx_owd_min[src][dst], otg_info->rx_owd_max[src][dst]);
			
		//free the packet 
		packet_rx=NULL;  					
		free(packet_rx);
		LOG_I(OTG,"RX Free packet\n");
		}

		else
			LOG_W(OTG,"check_packet :: (i=%d,j=%d) ERROR: packet seq_num TX=%d, num_pkts RX=%d \n",src, dst, packet_rx->otg_hdr->seq_num, otg_info->rx_num_pkt[src][dst]+1);

	}
	else 
	LOG_W(OTG,"check_packet :: ERROR: NO_PACKETS RECEIVED\n");

	

	return(status_ok);

}




