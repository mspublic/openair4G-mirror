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

/*! \file main.c
* \brief main program for OTG CX + RX  
* \author A. Hafsaoui
* \date 2011
* \version 0.1
* \company Eurecom
* \email: openair_tech@eurecom.fr
* \note
* \warning
*/


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "otg_tx.h"
#include "otg_tx_socket.h"
#include "otg_rx.h"
#include "otg_vars.h"
#include "otg_config.h"


#define STANDALONE 1 

int main (){

	int i, j, k, l, rx_otg=0,cmpt_pkts=0, size, test =1;
	double idt;
	char *packet;
	char *payload;
	char *ctl_otg_header;
	int mode_socket=0;  // to switch between socket and below IP mode

	init_all_otg();

	init_config_otg();

	init_seeds(g_otg->seed);

	

	do { // generate a tick in second 
	
		otg_info->emu_time+=1; // in ms  
	
		for (i=0; i<MAX_NUM_NODES; i++){

			otg_info->seq_num[i]=0;

	 		for (j=0; j<MAX_NUM_NODES; j++){
				for (k=0; k<MAX_NUM_TRAFFIC_STATE; k++){

				
					otg_info->seq_num[i]+=1;
					
				
					printf("SEQUENCE NUMBER PKTS=%d\n", otg_info->seq_num[i]);	

if (mode_socket==0){

	printf("===============CX==============\n");
	printf("\n");
	printf("===============START==============\n");	
	printf("\n");			
					
					packet=packet_gen(i, j, k);
					cmpt_pkts+=1;
	printf("\n");			
	printf("===============END================\n");	
		
	printf("\n");
	printf("\n");	


	printf("===============RX==============\n");
	printf("\n");
	printf("===============START==============\n");	
	printf("\n");

	rx_otg+=check_packet(packet);

	printf("\n");
	printf("===============END================\n");	
	printf("\n");
	printf("\n");
}
else{

printf("other mode = socket\n");
cmpt_pkts+=1;
socket_packet_send(i, j, k);

}


				}
			}
		}
	g_otg->duration[0]--;
	}
	while (g_otg->duration[0]!=0);





/*LOG_I(OTG, "this is a test %d \n", test);
LOG_W(OTG, "this is a test %d \n", test);
LOG_E(OTG, "this is a test %d \n", test);
LOG_D(OTG, "this is a test %d \n", test);
*/

free_addr_otg();

if (cmpt_pkts==rx_otg)
printf("--------Packet check OK --------\n");

printf("cmpt pkts =%d\n",cmpt_pkts);
printf("cmpt RX =%d\n",rx_otg);

return 0;

}
