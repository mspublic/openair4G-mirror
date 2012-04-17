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

#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)<(y)?(x):(y))

// Check if the packet is well received or not and extract data
int otg_rx_pkt( int src, int dst, int ctime, char *buffer_tx, unsigned int size){
  
  int bytes_read=0;
  otg_hdr_info_t * otg_hdr_info_rx;
  otg_hdr_t * otg_hdr_rx;
  // char * message; 
  //int hdr_size;
  
  if (buffer_tx!=NULL) { // 1st check : buffer_tx
    
    otg_hdr_info_rx = (otg_hdr_info_t *) (&buffer_tx[bytes_read]);
    bytes_read += sizeof (otg_hdr_info_t);
    
    //LOG_D(OTG,"[SRC %d][DST %d] RX pkt with size (hdr %d, pdcp %d)\n", src, dst, otg_hdr_info_rx->size, size);
   
    if ((otg_hdr_info_rx->flag == 0xffff) /*&& ((otg_hdr_info_rx->size ) == size)*/){ //2nd check : OTG FLAG
      
      otg_hdr_rx = (otg_hdr_t *) (&buffer_tx[bytes_read]);
      LOG_I(OTG,"[SRC %d][DST %d] RX pkt: seq number %d size (hdr %d, pdcp %d) \n", src, dst, otg_hdr_rx->seq_num, otg_hdr_info_rx->size, size);
       /*
      LOG_I(OTG,"HDR OTG: SIZE= HEADER + PAYLOAD %d\n", otg_hdr_rx->pkts_size);
      LOG_I(OTG,"HDR OTG: FLOW ID %d\n", otg_hdr_rx->flow_id);
      LOG_I(OTG,"HDR OTG: TX TIME %d\n", otg_hdr_rx->time);
      LOG_I(OTG,"HDR OTG: SEQ NUM %d\n", otg_hdr_rx->seq_num);
      LOG_I(OTG,"HDR OTG: HEADER TYPE %d\n", otg_hdr_rx->hdr_type);
      */      
      bytes_read += sizeof (otg_hdr_t);
      //      message= (char *) (&buffer_tx[bytes_read]);
      
      set_ctime(ctime);
      
      //hdr_size=sizeof(otg_hdr_info_t) + sizeof(otg_hdr_t); //add if for the flag and  
      
      if ((otg_hdr_rx->seq_num)==otg_info->seq_num_rx[src][dst]+1) {
	LOG_D(OTG,"check_packet :: (i=%d,j=%d) packet seq_num TX=%d, seq_num RX=%d \n",src,dst, otg_hdr_rx->seq_num, otg_info->seq_num_rx[src][dst]+1);
	otg_info->seq_num_rx[src][dst]+=1;
      }
      else if ((otg_hdr_rx->seq_num)> otg_info->seq_num_rx[src][dst]+1){ // out of sequence packet:  previous packet lost 
	LOG_D(OTG,"check_packet :: (i=%d,j=%d) :: out of sequence :: packet seq_num TX=%d > seq_num RX=%d \n",src,dst, otg_hdr_rx->seq_num, otg_info->seq_num_rx[src][dst]+1);
	otg_info->nb_loss_pkts[src][dst]+=((otg_hdr_rx->seq_num)-(otg_info->seq_num_rx[src][dst]+1));
	otg_info->seq_num_rx[src][dst]=otg_hdr_rx->seq_num;
      } 
      else if ((otg_hdr_rx->seq_num)< otg_info->seq_num_rx[src][dst]+1){ //the received packet arrived late 
	otg_info->nb_loss_pkts[src][dst]-=1;
	LOG_D(OTG,"check_packet :: (i=%d,j=%d) :: out of sequence :: packet seq_num TX=%d < seq_num RX=%d \n",src,dst, otg_hdr_rx->seq_num, otg_info->seq_num_rx[src][dst]+1);
      }
      // Compute STAT
      otg_info->rx_num_pkt[src][dst]+=1;
      //LOG_I(OTG,"PACKET SIZE (RX):  time(%d), otg header(%d), header + payload (%d), Total (%d)\n", ctime, hdr_size , strlen(message), otg_hdr_info_rx->size);
      
      otg_info->rx_num_bytes[src][dst]+=   otg_hdr_info_rx->size;
      otg_info->rx_pkt_owd[src][dst]= get_ctime() - otg_hdr_rx->time ;
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
      
      
      //Free pointers
      /*			if (buffer_tx!=NULL){   					
				free(buffer_tx);
				LOG_I(OTG,"RX :: Free buffer_tx\n");
				}*/
      
      return(0);
    }// end 3rd check: received data
    else{
      LOG_I(OTG," Not an OTG pkt, forward to upper layer \n");	
      return(1);
    }
    
  }
  return(0);
}



