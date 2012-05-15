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

/*! \file otg_kpi.c main used funstions to compute KPIs
* \brief define KPIs to compute for performance analysis 
* \author A. Hafsaoui
* \date 2012
* \version 0.1
* \company Eurecom
* \email: openair_tech@eurecom.fr
* \note
* \warning

*/


#include"otg_kpi.h"
#include"otg_externs.h"


void tx_throughput(int src, int dst){
  
  otg_info->tx_throughput[src][dst]=((double)otg_info->tx_num_bytes[src][dst] / get_ctime()); // unit Kbytes/sec, if ctime in ms
  LOG_I(OTG,"KPI :: TX src=%d, dst=%d, ctime=%d, tx throughput=%.3lf(Kbytes/sec) \n", src, dst, get_ctime(), otg_info->tx_throughput[src][dst]);
}



void rx_goodput(int src, int dst){

  otg_info->rx_goodput[src][dst]=((double)otg_info->rx_num_bytes[src][dst] /get_ctime()); // unit bytes/sec, if ctime in ms 
  LOG_I(OTG,"KPI :: RX src=%d, dst=%d, ctime=%d , bytes=%d, tx throughput=%.3lf(Kbytes/sec) \n", src, dst, get_ctime(), otg_info->rx_num_bytes[src][dst],otg_info->rx_goodput[src][dst]);
}



void rx_loss_rate_pkts(int src, int dst){
  
  //double loss_rate;
  if (otg_info->rx_num_pkt[src][dst]<otg_info->tx_num_pkt[src][dst])
    otg_info->rx_loss_rate[src][dst]= 1 - ((double)otg_info->rx_num_pkt[src][dst]/otg_info->tx_num_pkt[src][dst]);
  else
    otg_info->rx_loss_rate[src][dst]=0;
  
  LOG_I(OTG, "loss rate (src=%d, dst=%d):: = %lf(pkts) \n",src, dst, otg_info->rx_loss_rate[src][dst]);
  
}


void rx_loss_rate_bytes(int src, int dst){

  //double loss_rate;
  if (otg_info->rx_num_pkt[src][dst]<otg_info->tx_num_pkt[src][dst])
    otg_info->rx_loss_rate[src][dst]= 1 - ((double)otg_info->rx_num_bytes[src][dst]/otg_info->tx_num_bytes[src][dst]);
  else
    otg_info->rx_loss_rate[src][dst]=0;
  LOG_I(OTG, "loss rate (src=%d, dst=%d):: = %lf(bytes) \n",src, dst, otg_info->rx_loss_rate[src][dst]);

}





void kpi_gen() {
  int i, j;
  
  int tx_total_bytes=0;
  int tx_total_pkts=0;
  int rx_total_bytes=0;
  int rx_total_pkts=0;
  
  int min_owd=0;
  int max_owd=0;
  

#ifdef STANDALONE	
  FILE *file;
  file = fopen("log_OTG.txt", "w"); 
#endif
  for (i=0; i<(NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX); i++){
    for (j=0; j<(NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX); j++){
      
      tx_throughput(i, j);
      rx_goodput(i, j);
      rx_loss_rate_pkts(i, j);
      
      LOG_I(OTG,"KPI: (src=%d, dst=%d) NB packet TX= %d,  NB packet RX= %d\n ",i, j,  otg_info->tx_num_pkt[i][j],  otg_info->rx_num_pkt[i][j]);
      
      
      if (otg_info->tx_throughput[i][j]>0) {
	
	if (otg_info->tx_throughput[i][j]==otg_info->rx_goodput[i][j]){
	  LOG_I(OTG,"KPI:  (src=%d, dst=%d), RTT MIN (one way)ms= %d, RTT MAX (one way)ms= %d, TX throughput = %lf(Kbytes/sec), RX goodput= %lf (Kbytes/sec), loss rate(percentage)= %d\n",i, j, otg_info->rx_owd_min[i][j], otg_info->rx_owd_max[i][j],otg_info->tx_throughput[i][j],otg_info->rx_goodput[i][j],0);
	}
	
	else if (otg_info->tx_throughput[i][j]>otg_info->rx_goodput[i][j]){
	  LOG_I(OTG,"KPI: (LOSS):: (src=%d, dst=%d), RTT MIN (one way)ms= %d, RTT MAX (one way)ms= %d, TX throughput = %.3lf(Kbytes/sec), RX goodput= %lf (Kbytes/sec), loss rate(percentage)= %lf pkts\n ",i, j, otg_info->rx_owd_min[i][j], otg_info->rx_owd_max[i][j],otg_info->tx_throughput[i][j],otg_info->rx_goodput[i][j], (otg_info->rx_loss_rate[i][j]*100)); }
	
	tx_total_bytes+=otg_info->tx_num_bytes[i][j];
	tx_total_pkts+=otg_info->tx_num_pkt[i][j];
	
	rx_total_bytes+=otg_info->rx_num_bytes[i][j];
	rx_total_pkts+=otg_info->rx_num_pkt[i][j];
	
	if ((min_owd>otg_info->rx_owd_min[i][j]) || (min_owd==0))
	  min_owd=otg_info->rx_owd_min[i][j];
	if ((max_owd<otg_info->rx_owd_max[i][j]) || (max_owd==0))
	  max_owd=otg_info->rx_owd_max[i][j];
	
	LOG_I(OTG,"KPI: (src=%d, dst=%d) NB packet TX= %d,  NB packet RX= %d\n ",i, j,  otg_info->tx_num_pkt[i][j],  otg_info->rx_num_pkt[i][j]);
	
#ifdef STANDALONE
	
	fprintf(file,"----------------------------------------------------------\n");	
	fprintf(file,"Total Time= %d \n", otg_info->ctime);
	fprintf(file,"From eNB= %d \n", i);
	fprintf(file,"To UE= %d \n", j);
	fprintf(file,"Total packets(TX)= %d \n", otg_info->tx_num_pkt[i][j]);
	fprintf(file,"Total bytes(TX)= %d \n", otg_info->tx_num_bytes[i][j]);
	fprintf(file,"RTT MIN (one way)ms= %d \n", otg_info->rx_owd_min[i][j]);
	fprintf(file,"RTT MAX (one way)ms= %d \n", otg_info->rx_owd_max[i][j]);
	fprintf(file,"TX throughput = %lf(Kbytes/sec) \n", otg_info->tx_throughput[i][j]);
	fprintf(file,"RX goodput= %lf (Kbytes/sec) \n", otg_info->rx_goodput[i][j]);
	if (otg_info->rx_loss_rate[i][j]>0){
	  fprintf(file,"Loss rate(percentage)= %lf pkts \n", (otg_info->rx_loss_rate[i][j]*100));
	  fprintf(file,"NB Lost  packets=%d \n", otg_info->nb_loss_pkts[i][j]);
	}
#else
	LOG_F(OTG,"----------------------------------------------------------\n");
	LOG_F(OTG,"Total Time= %d \n", otg_info->ctime);
	LOG_F(OTG,"From eNB= %d \n", i);
	LOG_F(OTG,"To UE= %d \n", j);
	LOG_F(OTG,"Total packets(TX)= %d \n", otg_info->tx_num_pkt[i][j]);
	LOG_F(OTG,"Total bytes(TX)= %d \n", otg_info->tx_num_bytes[i][j]);
	LOG_F(OTG,"RTT MIN (one way)ms= %d \n", otg_info->rx_owd_min[i][j]);
	LOG_F(OTG,"RTT MAX (one way)ms= %d \n", otg_info->rx_owd_max[i][j]);
	LOG_F(OTG,"TX throughput = %lf(Kbytes/sec) \n", otg_info->tx_throughput[i][j]);
	LOG_F(OTG,"RX goodput= %lf (Kbytes/sec) \n", otg_info->rx_goodput[i][j]);
	if (otg_info->rx_loss_rate[i][j]>0){
	  LOG_F(OTG,"Loss rate(percentage)= %lf pkts \n", (otg_info->rx_loss_rate[i][j]*100));
	  LOG_F(OTG,"NB Lost  packets= %d \n", otg_info->nb_loss_pkts[i][j]);
	}
#endif 
      }
    }
  }
  	
#ifdef STANDALONE
  fprintf (file,"**************** TOTAL RESULTS ******************\n");
  fprintf(file,"Total Time= %d \n", otg_info->ctime);
  fprintf(file,"Total packets(TX)= %d \n", tx_total_pkts);
  fprintf(file,"Total bytes(TX)= %d \n", tx_total_bytes);
  fprintf(file,"Total packets(RX)= %d \n", rx_total_pkts);
  fprintf(file,"Total bytes(RX)= %d \n", rx_total_bytes);
  fprintf(file,"RTT MIN (one way)ms= %d \n", min_owd);
  fprintf(file,"RTT MAX (one way)ms= %d \n", max_owd);
  fprintf(file,"TX throughput = %lf(Kbytes/sec) \n", (double)tx_total_bytes/otg_info->ctime);
  fprintf(file,"RX throughput = %lf(Kbytes/sec) \n", (double)rx_total_bytes/otg_info->ctime);
  fclose(file);
#else
  LOG_F(OTG,"**************** TOTAL RESULTS ******************\n");
  LOG_F(OTG,"Total Time= %d \n", otg_info->ctime);
  LOG_F(OTG,"Total packets(TX)= %d \n", tx_total_pkts);
  LOG_F(OTG,"Total packets(RX)= %d \n", rx_total_pkts);
  LOG_F(OTG,"Total bytes(TX)= %d \n", tx_total_bytes);
  LOG_F(OTG,"Total bytes(RX)= %d \n", rx_total_bytes);
  LOG_F(OTG,"RTT MIN (one way)ms= %d \n", min_owd);
  LOG_F(OTG,"RTT MAX (one way)ms= %d \n", max_owd);
  LOG_F(OTG,"TX throughput = %lf(Kbytes/sec) \n", (double)tx_total_bytes/otg_info->ctime);
  LOG_F(OTG,"RX throughput = %lf(Kbytes/sec) \n", (double)rx_total_bytes/otg_info->ctime);
#endif
}


