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

/*! \file otg_tx.c
* \brief function containing the OTG TX traffic generation functions 
* \author N. Nikaein and A. Hafsaoui
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
#include "otg_vars.h"
#include "../MATH/oml.h"
//#include "../../../openair1/PHY/CODING/defs.h" // for CRC copmputing 
//#include "net_data.h"
#include "otg_config.h"
//#include "UTIL/LOG/log.h"

#define STANDALONE 1 


// Defining initial and default values of variables

void init_all_otg()  {

	//set otg params to 0
 	g_otg = calloc(1, sizeof(otg_t));
 	memset(g_otg, 0, sizeof(otg_t));		

	//set otg infos to 0
 	otg_info = calloc(1, sizeof(otg_info));
 	memset(otg_info, 0, sizeof(otg_info));

	//set otg header to 0
 	otg_hdr = calloc(1, sizeof(otg_hdr));
 	memset(otg_hdr, 0, sizeof(otg_hdr));

}



// Time Distribution function to distribute the inter-departure time using the required distribution


double time_dist(int src, int dst, int state) {

double idt;
 
	#ifdef STANDALONE

	if (g_otg->idt_dist[src][dst][state] == UNIFORM)
	idt =  ceil(MAXIDT * uniform_dist(g_otg->idt_min[src][dst][state], g_otg->idt_max[src][dst][state]));
	else if (g_otg->idt_dist[src][dst][state] == GAUSSIAN)
	idt =  ceil(MAXIDT * gaussian_dist((g_otg->idt_max[src][dst][state] - g_otg->idt_min[src][dst][state])/2 , g_otg->idt_std_dev[src][dst][state]));
	else if (g_otg->idt_dist[src][dst][state] == EXPONENTIAL)
	idt=  ceil(MAXIDT * exponential_dist(g_otg->idt_lambda[src][dst][state]));
	else if (g_otg->idt_dist[src][dst][state] == POISSON)
	idt =  ceil(MAXIDT * poisson_dist(g_otg->idt_lambda[src][dst][state]));
	else if (g_otg->idt_dist[src][dst][state] == FIXED)
	idt = (int) ceil(g_otg->idt_min[src][dst][state]);
	#else
	idt = (get_emu_time() % (1000 / idt)) ? 1.0 : 0.0; // need to be revised  
	#endif
	printf ("Inter Departure Time Distribution= %lf \n", idt);
return idt;
}


// otg_params.size Distribution Function to distribute the packet otg_params.size using the required distribution


int size_dist(int src, int dst, int state) {
int size;
	
	if (g_otg->size_dist[src][dst][state] == UNIFORM)
	size = (int) ceil(uniform_dist(g_otg->size_min[src][dst][state], g_otg->size_max[src][dst][state]));
	else if (g_otg->size_dist[src][dst][state] == GAUSSIAN)
	size = (int) ceil(gaussian_dist((g_otg->size_max[src][dst][state] - g_otg->size_min[src][dst][state])/2 , g_otg->size_std_dev[src][dst][state]));
	else if (g_otg->size_dist[src][dst][state] == EXPONENTIAL)
	size= (int) ceil(exponential_dist(g_otg->size_lambda[src][dst][state]));
	else if (g_otg->size_dist[src][dst][state] == POISSON)
	size = (int) ceil(poisson_dist(g_otg->size_lambda[src][dst][state]));
	else if (g_otg->size_dist[src][dst][state] == FIXED)
	size = (int) ceil(g_otg->size_min[src][dst][state]);
	printf ("Packet Size Distribution= %d \n", size);


	return size;


}

void init_predef_otg() {
int i;
int j;

	for (i=0; i<g_otg->num_nodes; i++){
		for (j=0; j<g_otg->num_nodes; j++){
			if (g_otg->application_type[i][j] == CBR) {
				g_otg->trans_proto[i] = 0;
				g_otg->ip_v[i] = 0;
				g_otg->idt_dist[i][j][0] = FIXED;
				g_otg->idt_dist[i][j][1] = FIXED;

				g_otg->idt_min[i][j][0] =  10;
				g_otg->idt_min[i][j][1] =  10;


				g_otg->idt_max[i][j][0] =  10;
				g_otg->idt_max[i][j][1] =  10;

				g_otg->idt_std_dev[i][j][0] = 0;
				g_otg->idt_std_dev[i][j][1] = 0;

				g_otg->idt_lambda[i][j][0] = 0;
				g_otg->idt_lambda[i][j][1] = 0;

				g_otg->size_dist[i][j][0] = FIXED;
				g_otg->size_dist[i][j][1] = FIXED;
	printf("OTG_CONFIG CBR, src = %d, dst = %d, dist type for size = %d\n", i, j, g_otg->size_dist[i][j][0]);
				g_otg->size_min[i][j][0] =  50;
				g_otg->size_min[i][j][1] =  50;


				g_otg->size_max[i][j][0] =  50;
				g_otg->size_max[i][j][1] =  50;

				g_otg->size_std_dev[i][j][0] = 0;
				g_otg->size_std_dev[i][j][1] = 0;

				g_otg->size_lambda[i][j][0] = 0;
				g_otg->size_lambda[i][j][1] = 0;

				g_otg->dst_port[j] = 0;

			} else if (g_otg->application_type[i][j] == M2M_AP) { 

				g_otg->trans_proto[i] = 0;
				g_otg->ip_v[i] = 0;

				g_otg->idt_dist[i][j][0] = UNIFORM;
				g_otg->idt_dist[i][j][1] = EXPONENTIAL;
	printf("OTG_CONFIG M2M_AP, src = %d, dst = %d, dist IDT = %d\n", i, j, g_otg->idt_dist[i][j][0]);
				g_otg->idt_min[i][j][0] =  100;
				g_otg->idt_min[i][j][1] =  0;


				g_otg->idt_max[i][j][0] =  500;
				g_otg->idt_max[i][j][1] =  0;

				g_otg->idt_std_dev[i][j][0] = 0;
				g_otg->idt_std_dev[i][j][1] = 0;

				g_otg->idt_lambda[i][j][0] = 0;
				g_otg->idt_lambda[i][j][1] = 5; //pkt/s

				g_otg->size_dist[i][j][0] = FIXED;
				g_otg->size_dist[i][j][1] = EXPONENTIAL;

				g_otg->size_min[i][j][0] =  8000;
				g_otg->size_min[i][j][1] =  0;


				g_otg->size_max[i][j][0] =  8000;
				g_otg->size_max[i][j][1] =  0;

				g_otg->size_std_dev[i][j][0] = 0;
				g_otg->size_std_dev[i][j][1] = 0;

				g_otg->size_lambda[i][j][0] = 0;
				g_otg->size_lambda[i][j][1] = 1/8000;

				g_otg->dst_port[j] = 0;

			} else if (g_otg->application_type[i][j] == M2M_BR) { 

				g_otg->trans_proto[i] = 0;
				g_otg->ip_v[i] = 0;

				g_otg->idt_dist[i][j][0] = UNIFORM;
				g_otg->idt_dist[i][j][1] = EXPONENTIAL;
	printf("OTG_CONFIG M2M_BR, src = %d, dst = %d, dist IDT = %d\n", i, j, g_otg->idt_dist[i][j][0]);
				g_otg->idt_min[i][j][0] =  100;
				g_otg->idt_min[i][j][1] =  0;


				g_otg->idt_max[i][j][0] =  500;
				g_otg->idt_max[i][j][1] =  0;

				g_otg->idt_std_dev[i][j][0] = 0;
				g_otg->idt_std_dev[i][j][1] = 0;

				g_otg->idt_lambda[i][j][0] = 0;
				g_otg->idt_lambda[i][j][1] = 10; //pkt/s

				g_otg->size_dist[i][j][0] = FIXED;
				g_otg->size_dist[i][j][1] = EXPONENTIAL;

				g_otg->size_min[i][j][0] =  8000;
				g_otg->size_min[i][j][1] =  0;


				g_otg->size_max[i][j][0] =  8000;
				g_otg->size_max[i][j][1] =  0;

				g_otg->size_std_dev[i][j][0] = 0;
				g_otg->size_std_dev[i][j][1] = 0;

				g_otg->size_lambda[i][j][0] = 0;
				g_otg->size_lambda[i][j][1] = 1/8000;

				g_otg->dst_port[j] = 0;

			} else if (g_otg->application_type[i][j] == GAMING_OA) { 

				g_otg->trans_proto[i] = 0;
				g_otg->ip_v[i] = 0;

				g_otg->idt_dist[i][j][0] = UNIFORM;
				g_otg->idt_dist[i][j][1] = UNIFORM;
	printf("OTG_CONFIG GAMING_OA, src = %d, dst = %d, dist IDT = %d\n", i, j, g_otg->idt_dist[i][j][0]);
				g_otg->idt_min[i][j][0] =  69;
				g_otg->idt_min[i][j][1] =  69;


				g_otg->idt_max[i][j][0] =  103;
				g_otg->idt_max[i][j][1] =  103;

				g_otg->idt_std_dev[i][j][0] = 0;
				g_otg->idt_std_dev[i][j][1] = 0;

				g_otg->idt_lambda[i][j][0] = 0;
				g_otg->idt_lambda[i][j][1] = 0; //pkt/s

				g_otg->size_dist[i][j][0] = GAUSSIAN;
				g_otg->size_dist[i][j][1] = GAUSSIAN;

				g_otg->size_min[i][j][0] =  4.6;
				g_otg->size_min[i][j][1] =  4.6;


				g_otg->size_max[i][j][0] =  42.2;
				g_otg->size_max[i][j][1] =  42.2;

				g_otg->size_std_dev[i][j][0] = 0;
				g_otg->size_std_dev[i][j][1] = 0;

				g_otg->size_lambda[i][j][0] = 0;
				g_otg->size_lambda[i][j][1] = 0;

				g_otg->dst_port[j] = 0;
			} else if (g_otg->application_type[i][j] == GAMING_TF) { 

				g_otg->trans_proto[i] = 0;
				g_otg->ip_v[i] = 0;

				g_otg->idt_dist[i][j][0] = UNIFORM;
				g_otg->idt_dist[i][j][1] = UNIFORM;
	printf("OTG_CONFIG GAMING_TF, src = %d, dst = %d, dist IDT = %d\n", i, j, g_otg->idt_dist[i][j][0]);
				g_otg->idt_min[i][j][0] =  31;
				g_otg->idt_min[i][j][1] =  31;


				g_otg->idt_max[i][j][0] =  42;
				g_otg->idt_max[i][j][1] =  42;

				g_otg->idt_std_dev[i][j][0] = 0;
				g_otg->idt_std_dev[i][j][1] = 0;

				g_otg->idt_lambda[i][j][0] = 0;
				g_otg->idt_lambda[i][j][1] = 0; //pkt/s

				g_otg->size_dist[i][j][0] = GAUSSIAN;
				g_otg->size_dist[i][j][1] = GAUSSIAN;

				g_otg->size_min[i][j][0] =  4.6;
				g_otg->size_min[i][j][1] =  4.6;


				g_otg->size_max[i][j][0] =  42.2;
				g_otg->size_max[i][j][1] =  42.2;

				g_otg->size_std_dev[i][j][0] = 0;
				g_otg->size_std_dev[i][j][1] = 0;

				g_otg->size_lambda[i][j][0] = 0;
				g_otg->size_lambda[i][j][1] = 0;

				g_otg->dst_port[j] = 0;
			}
		}
	}
}

// init OTG with config parameters
/*
void init_config_otg() {

int i, j, k;

	for (i=0; i<g_otg->num_nodes; i++){

		//g_otg->trans_proto[i]=TCP;
		
		g_otg->duration[i]=5; //100
		g_otg->dst_port[i]=DST_PORT;
		//g_otg->dst_ip[i]=(char*)malloc(100*sizeof(char*));
		g_otg->dst_ip[i]=DST_IP;

		for (j=0; j<g_otg->num_nodes; j++){

			g_otg->application_type[i][j]=1;	
			for (k=0; k<MAX_NUM_TRAFFIC_STATE; k++){

				g_otg->idt_dist[i][j][k]=IDT_DIST;
				g_otg->idt_min[i][j][k]=IDT_MIN; // unit second
				g_otg->idt_max[i][j][k]=IDT_MAX;
				g_otg->idt_std_dev[i][j][k]=IDT_STD_DEV;
				g_otg->idt_lambda[i][j][k]=IDT_LAMBDA;

				g_otg->size_dist[i][j][k]=PKTS_SIZE_DIST; //unit packet per second 
				g_otg->size_min[i][j][k]=PKTS_SIZE_MIN;
				g_otg->size_max[i][j][k]=PKTS_SIZE_MAX;
				g_otg->size_std_dev[i][j][k]=PKTS_SIZE_STD_DEV;
				g_otg->size_lambda[i][j][k]=PKTS_SIZE_LAMBDA;



				
				
				
			}
		}
	}
}

*/
// Generate a random string[size]
char *random_string(int size, ALPHABET data_type) {

	char *data;
	int i, pos;

	data=(char*)malloc(size*sizeof(char*));

	if (data_type==1)
	{
		for(i=0;i<size;i++){
    			pos = rand()%(strlen(ALPHABET_NUM));		
			data[i]=ALPHABET_NUM[pos];
		}
	}

	else
	{
		for(i=0;i<size;i++){
    			pos = rand()%(strlen(ALPHABET_NUM_LETTER));		
			data[i]=ALPHABET_NUM_LETTER[pos];
		}
	}
 
	//printf ("Generated string= %s\n", data);
	return data;
}



char *packet_gen(int src, int dst, int state){

	double idt;
	int size;
	char *payload=NULL;
	char *otg_header=NULL;
	char *header=NULL;
	char *packet_payload=NULL;
	char *packet=NULL;

	//if ((g_otg->application_type[src][dst] == 0) && (g_otg->idt_dist[src][dst][0] == 0))
	if (g_otg->idt_dist[src][dst][0] == 0)
		return 0;	

	printf("OTG :: Source =%d\n",src);
	printf("OTG :: Destination =%d\n",dst);
	printf("OTG :: Application=%d\n",g_otg->application_type[src][dst]);
	printf("OTG :: lambda IDT = %d\n", g_otg->idt_lambda[src][dst][1]);
	printf("OTG :: lambda PKTS = %d\n", g_otg->size_lambda[src][dst][1]);
	printf("OTG :: transport protocol = %d\n", g_otg->trans_proto[src]);
	printf("OTG :: IP version = %d\n",  g_otg->ip_v[src]);

	idt=time_dist(src, dst, state);	
	printf("OTG :: IDT = %lf\n",idt);			
	size=size_dist(src, dst, state);
	printf("OTG :: Payload size=%d\n",size);
	
	
	  
	payload=payload_pkts(size);
	printf("==============STEP 1: OTG PAYLOAD OK============= \n");
	printf("OTG payload= (%d, %s) \n", size, payload);
	
	
	printf("==============STEP 2: OTG protocol HEADER OK============== \n");	
	header=header_gen(g_otg->ip_v[src], g_otg->trans_proto[src]);
	printf("OTG protocol HEADER= (%d, %s) \n", strlen(header),header);
	otg_info->emu_time= 1000; // to modify 
	otg_info->seq_num[src]= 77; // to modify

	printf("==============STEP 3: OTG control HEADER OK========== \n");
	otg_header=otg_header_gen(OTG_CTRL_FLAG, otg_info->emu_time,  otg_info->seq_num[src]);
	printf("OTG control HEADER= (%d, %s) \n", strlen(otg_header), otg_header);

	
	packet=(char*)malloc((strlen(payload) + strlen(header))*sizeof(char*));
	memset(packet, 0, strlen(packet));
	snprintf(packet, strlen(otg_header)+1, "%s", otg_header);

	if (strlen(header) > strlen(otg_header)) {
		strncat(packet, header, strlen(header) - strlen(otg_header) );}

	strncat(packet, payload, strlen(payload));
	printf("OTG packet= (%d, %s) \n", strlen(payload) + strlen(header), packet);


		if (NULL != packet_payload){
			packet_payload=NULL;
			free(packet_payload);
		}


	printf("==============STEP 5: PACKET OK============= \n");	

	if (NULL != header){
		header=NULL;
		free(header);
	}
	if (NULL != otg_header){
		otg_header=NULL;
		free(otg_header);
	}
	if (NULL != payload){
		payload=NULL;
		free(payload);
	}
	
	printf ("Generated Packet= %s\n", packet);
	
	return packet;

}

/*
unsigned int crc_gen(char *packet, CRC crc){

	unsigned int hdr_crc;

	crcTableInit();

	
	//hdr_crc=(unsigned int*)malloc(8*sizeof(unsigned int*));

	switch(crc)
	{
		case 0: 
			
			hdr_crc= crc8(packet, (sizeof(packet) - 1)*8);
			//printf ("CRC=CRC8\n");
			break;

		case 1:
			
			hdr_crc= crc16(packet, (sizeof(packet) - 1)*8);
			//printf ("CRC=CRC16\n");
			break;

		case 2:
			
			hdr_crc= crc24a(packet, (sizeof(packet) - 1)*8);
			//printf ("CRC=CRC24A\n");
			break;

		case 3:
			
			hdr_crc= crc24b(packet, (sizeof(packet) - 1)*8);
			//printf ("CRC=CRC24B\n");
			break;

	}
	//printf("GENERATED CRC=%x\n",  hdr_crc);

	return (hdr_crc);
}

*/


char *header_gen(int ip_v, int trans_proto){

	int hdr_size=0;
	char *hdr;

	if (ip_v==0) { 
		hdr_size=HDR_IP_v4_MIN;
	}
	else {	
		hdr_size=HDR_IP_v6;
	}
	
	if (trans_proto==0){	
 		hdr_size=hdr_size + HDR_UDP ;
	}	
	else {
		hdr_size=hdr_size + HDR_TCP;
	}

	hdr=(char*)malloc(hdr_size*sizeof(char*));
	hdr=random_string(hdr_size,NUM);


return(hdr);

}


char *payload_pkts(int payload_size){
	
	char *payload;	
	payload=(char*)malloc(payload_size*sizeof(char*));
	payload=random_string(payload_size,NUM_LETTER);
	return (payload);

}


char *otg_header_gen( char *flag, int time, int seq_num){

	char *ctrl_head;
	char* time_s;
	char *seq_num_s;
	char *tmp;

int i;
	ctrl_head=(char*)malloc(HDR_OTG_SIZE *sizeof(char*)); 
	snprintf(ctrl_head, HDR_OTG_SIZE, "%d%d", time, seq_num);
	return ctrl_head;

}



void free_addr_otg(){
	int i; 
		for (i=0; i<g_otg->num_nodes; i++){
			//for (j=0; j<g_otg->num_nodes; j++){
			//	for (k=0; k<MAX_NUM_TRAFFIC_STATE; k++){

					if (NULL != g_otg->dst_ip[i]){
						g_otg->dst_ip[i]=NULL;
						free(g_otg->dst_ip[i]);
					}
				//}
			//}
		}
}



#ifdef STANDALONE

double get_emu_time(void){

return otg_info->emu_time;
}
#endif


