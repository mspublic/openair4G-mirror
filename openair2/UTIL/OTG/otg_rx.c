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


#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)<(y)?(x):(y))

// Check if the packet is well received or not and extract data
char *check_packet( int src, int dst, int ctime, char *buffer_tx){
	
	int status_ok=0;
	packet_t* packet_rx;

if (buffer_tx!=NULL) {

	packet_rx= malloc(sizeof(*packet_rx));

	packet_rx->flag= (char*)malloc(OTG_FLAG_SIZE);
	memcpy(packet_rx->flag,  buffer_tx, OTG_FLAG_SIZE);


	set_ctime(ctime);



	if (strcmp(packet_rx->flag,"OTG")==0){

		LOG_I(OTG,"check_packet :: FLAG OTG OK !!!! \n");

		packet_rx->flow_id= (int*)malloc(sizeof(int));
		memcpy(packet_rx->flow_id,  buffer_tx + OTG_FLAG_SIZE, sizeof(packet_rx->flow_id));

		packet_rx->time= (int*)malloc(sizeof(int));
		memcpy(packet_rx->time,   buffer_tx  + OTG_FLAG_SIZE+ sizeof(packet_rx->flow_id), sizeof(packet_rx->time));

		packet_rx->payload_size= (int*)malloc(sizeof(int));
		memcpy(packet_rx->payload_size,   buffer_tx + OTG_FLAG_SIZE + sizeof(packet_rx->flow_id) + sizeof(packet_rx->time), sizeof(packet_rx->payload_size));

		packet_rx->seq_num= (int*)malloc(sizeof(int));
		memcpy(packet_rx->seq_num,   buffer_tx + OTG_FLAG_SIZE + sizeof(packet_rx->flow_id) + sizeof(packet_rx->time)+sizeof(packet_rx->payload_size), sizeof(packet_rx->seq_num));

		packet_rx->header_size= (int*)malloc(sizeof(int));
		memcpy(packet_rx->header_size,   buffer_tx + OTG_FLAG_SIZE + sizeof(packet_rx->flow_id) + sizeof(packet_rx->time)+sizeof(packet_rx->payload_size)+sizeof(packet_rx->seq_num), sizeof(packet_rx->header_size));



		LOG_I(OTG,"Received OTG payload SEQ_NUM=%i, PAYLOAD SIZE=%i, HEADER SIZE=%i, TIME TX=%i , FLOW ID= %i, FLAG=%s \n", *packet_rx->seq_num, *packet_rx->payload_size, *packet_rx->header_size, *packet_rx->time, *packet_rx->flow_id, packet_rx->flag );


LOG_I(OTG,"check_packet :: protocol HEADER= (%d,%s)\n", *packet_rx->header_size, packet_rx->header);

		packet_rx->header= (char *)malloc(*packet_rx->header_size);
		memcpy(packet_rx->header,   buffer_tx + OTG_FLAG_SIZE  + sizeof(packet_rx->flow_id) + sizeof(packet_rx->time)+sizeof(packet_rx->payload_size)+sizeof(packet_rx->seq_num)+ sizeof(packet_rx->header_size), *packet_rx->header_size);

		LOG_I(OTG,"check_packet :: protocol HEADER= (%d,%s)\n", *packet_rx->header_size, packet_rx->header);


		packet_rx->payload= (char *)malloc(*packet_rx->payload_size+1);
		memcpy(packet_rx->payload,   buffer_tx + OTG_FLAG_SIZE + sizeof(packet_rx->flow_id) + sizeof(packet_rx->time)+sizeof(packet_rx->payload_size)+sizeof(packet_rx->seq_num)+ sizeof(packet_rx->header_size) + *packet_rx->header_size, *packet_rx->payload_size);

		LOG_I(OTG,"check_packet :: payload=(%d,%s)\n", *packet_rx->payload_size, packet_rx->payload);




			
		if ((*packet_rx->seq_num)==otg_info->seq_num_rx[src][dst]+1) {
			LOG_T(OTG,"check_packet :: (i=%d,j=%d) packet seq_num TX=%d, seq_num RX=%d \n",src,dst, *packet_rx->seq_num, otg_info->seq_num_rx[src][dst]+1);
			otg_info->seq_num_rx[src][dst]+=1;
			}
		else if ((*packet_rx->seq_num)> otg_info->seq_num_rx[src][dst]+1){ // out of sequence packet:  previous packet lost 
			LOG_T(OTG,"check_packet :: (i=%d,j=%d) :: out of sequence :: packet seq_num TX=%d > seq_num RX=%d \n",src,dst, *packet_rx->seq_num, otg_info->seq_num_rx[src][dst]+1);
			otg_info->nb_loss_pkts[src][dst]+=((*packet_rx->seq_num)-(otg_info->seq_num_rx[src][dst]+1));
			otg_info->seq_num_rx[src][dst]=*packet_rx->seq_num;
			


			} 
		else if ((*packet_rx->seq_num)< otg_info->seq_num_rx[src][dst]+1){ //the received packet arrived late 
			otg_info->nb_loss_pkts[src][dst]-=1;
			LOG_T(OTG,"check_packet :: (i=%d,j=%d) :: out of sequence :: packet seq_num TX=%d < seq_num RX=%d \n",src,dst, *packet_rx->seq_num, otg_info->seq_num_rx[src][dst]+1);
		}

		status_ok=1;

		LOG_I(OTG,"RX NFO ::  Header :%s \n", packet_rx->header);

		LOG_I(OTG,"RX INFO ::  Payload :%s \n", packet_rx->payload);
		LOG_I(OTG,"RX INFO ::  flow id :%d \n", *packet_rx->flow_id);
		LOG_I(OTG,"RX INFO :: header size type: %d \n", *packet_rx->header_size);
		LOG_I(OTG,"RX INFO :: payload size type: %d \n", *packet_rx->payload_size);
		LOG_I(OTG,"RX INFO :: time: %d \n", *packet_rx->time);
		LOG_I(OTG,"RX INFO :: Sequence NB: %d \n", *packet_rx->seq_num);

			// Compute STAT
		otg_info->rx_num_pkt[src][dst]+=1;
		

	LOG_I(OTG,"PACKET SIZE (RX):  time(%d), otg header(%d), header (%d), payload (%d), Total (%d) \n", ctime, OTG_FLAG_SIZE +  sizeof(packet_rx->flow_id) + sizeof(packet_rx->time)+ sizeof(packet_rx->payload_size) + sizeof(packet_rx->seq_num)+ sizeof(packet_rx->header_size), *packet_rx->header_size, *packet_rx->payload_size, OTG_FLAG_SIZE +  sizeof(packet_rx->flow_id) + sizeof(packet_rx->time)+ sizeof(packet_rx->payload_size) + sizeof(packet_rx->seq_num)+ sizeof(packet_rx->header_size)+ *packet_rx->header_size + *packet_rx->payload_size);

		otg_info->rx_num_bytes[src][dst]+= OTG_FLAG_SIZE + sizeof(packet_rx->flow_id) + sizeof(packet_rx->time)+ sizeof(packet_rx->payload_size) + sizeof(packet_rx->seq_num)+ sizeof(packet_rx->header_size) +  *packet_rx->header_size + *packet_rx->payload_size ;
		otg_info->rx_pkt_owd[src][dst]= get_ctime() - *packet_rx->time ;
		LOG_I(OTG,"RX INFO :: RTT (one way) ms: %d \n", otg_info->rx_pkt_owd[src][dst]);
	

		if (otg_info->rx_owd_max[src][dst]==0){
				otg_info->rx_owd_max[src][dst]=otg_info->rx_pkt_owd[src][dst];
				otg_info->rx_owd_min[src][dst]=otg_info->rx_pkt_owd[src][dst];
		}
		else {
				otg_info->rx_owd_max[src][dst]=MAX(otg_info->rx_owd_max[src][dst],otg_info->rx_pkt_owd[src][dst] );
				otg_info->rx_owd_min[src][dst]=MIN(otg_info->rx_owd_min[src][dst],otg_info->rx_pkt_owd[src][dst] );
		}



		LOG_I(OTG,"RX INFO :: RTT MIN(one way) ms: %d, RTT MAX(one way) ms: %d \n", otg_info->rx_owd_min[src][dst], otg_info->rx_owd_max[src][dst]);
		
			// end STAT



		return(NULL);
	}
	else
		LOG_W(OTG,"check_packet	:: ERROR: FORWARD DATA \n");


	

	if (packet_rx!=NULL){ 
		packet_rx=NULL;  					
		free(packet_rx);
	}
	

	return(buffer_tx);

}

else {
	LOG_W(OTG,"check_packet :: ERROR: NO_PACKETS RECEIVED\n");
	return("fffff"); //The case when no packet is received
	}

}




