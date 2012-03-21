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
	printf ("Packet Size Distribution= %d \n", size);
	return size;


}


// init OTG with config parameters
void init_config_otg() {

int i, j, k;

	for (i=0; i<MAX_NUM_NODES; i++){

		//g_otg->trans_proto[i]=TCP;
		
		g_otg->duration[i]=5; //100



		for (j=0; j<MAX_NUM_NODES; j++){
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


				g_otg->dst_port[i][j][k]=DST_PORT;
				g_otg->dst_ip[i][j][k]=(char*)malloc(100*sizeof(char*));
				g_otg->dst_ip[i][j][k]=DST_IP;
				
				
				
			}
		}
	}
}


// Generate a random string[size]
char *random_string(int size, ALPHABET data_type) {

	char *data;
	int i, pos;

	data=(char*)malloc(size*sizeof(char*));

	if (data_type==NUM)
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
	char *payload;
	char *otg_header;
	char *header;
	char *tmp_packet;
	char *packet;
	//char *crc_header;



	idt=time_dist(src, dst, state);					
	size=size_dist(src, dst, state);
	payload=payload_pkts(size-HDR_OTG_SIZE); //include OTG header in the payload  
	printf("==============STEP 1: PAYLOAD OK============= \n");

	//otg_header=otg_header_gen("OTG", otg_info->emu_time,  otg_info->seq_num[src], payload);



	

	header=header_gen(g_otg->ip_v[src], g_otg->trans_proto[src]);
	
	printf("==============STEP 2: HEADER OK============== \n");

	printf("HEADER= %s \n", header);

	packet=(char*)malloc((strlen(payload) + strlen(header))*sizeof(char*));
	
	otg_header=otg_header_gen(OTG_CTRL_FLAG, otg_info->emu_time,  otg_info->seq_num[src]);

	printf("==============STEP 3: OTG HEADER OK========== \n");
	
	printf("OTG HEADER= %s \n", otg_header);


	tmp_packet=(char*)malloc((strlen(payload) + strlen(header))*sizeof(char*));
	sprintf(tmp_packet, "%s%s", header, payload);
	
	//printf ("Generated tmp Packet= %s\n", tmp_packet);

	packet=(char*)malloc((100 + strlen(otg_header) + strlen(payload) + strlen(header))*sizeof(char*));
	
	// No need to compute CRC 	
	/*otg_hdr->crc16=crc_gen(tmp_packet, CRC_FLAG);	
	printf("==============STEP 4: CRC OK================ \n");
	sprintf(packet, "%u%s%s",crc_gen(tmp_packet, CRC_FLAG), otg_header ,tmp_packet);
	*/

	sprintf(packet,"%s%s", otg_header ,tmp_packet);
	printf ("TMP PACKET SIZE: %d \n", strlen(tmp_packet));

		if (NULL != tmp_packet){
			tmp_packet=NULL;
			free(tmp_packet);
		}


	printf("==============STEP 5: PACKET OK============= \n");	

	printf ("------Packet Gen------\n");
	printf ("PACKET SIZE: %d \n", strlen(packet));
	printf ("HEADER SIZE: %d \n", strlen(header));
	printf ("PAYLOAD SIZE: %d \n", strlen(payload));
	printf ("OTG CTL SIZE: %d \n", strlen(otg_header));
	

	
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


char *header_gen(int ip_v, int trans_proto){

	int packet_size, hdr_size=0;
	char *hdr;
	char *packet;

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
	//strcpy(otg_hdr->flag,flag); 
	//otg_hdr->time=time; 
	//otg_hdr->seq_num=seq_num;

	//printf("GENERATED FLAG=%d\n",  otg_hdr->flag);
	printf("GENERATED FLAG=%s\n", OTG_CTRL_FLAG);		
// TIME 
	time_s=(char*)malloc(HDR_OTG_TIME_SIZE*sizeof(char*));
	tmp=(char*)malloc(HDR_OTG_TIME_SIZE*sizeof(char*));		
	sprintf(time_s, "%d", time);
		for(i=0; i<HDR_OTG_TIME_SIZE-strlen(time_s); i++){		
			strncat(tmp,"0",1);	
		}
	sprintf(time_s,"%s%d",tmp,time);
	printf("GENERATED INT TIME=%d\n",time);
	printf("GENERATED CHAR TIME=%s\n",time_s);
		if (NULL != tmp){
			tmp=NULL;
			free(tmp);
		}
// SEQ NB
	seq_num_s=(char*)malloc(HDR_OTG_SEQ_SIZE*sizeof(char*));
	tmp=(char*)malloc(HDR_OTG_SEQ_SIZE*sizeof(char*));		
	sprintf(seq_num_s, "%d", seq_num);
		for(i=0; i<HDR_OTG_SEQ_SIZE-strlen(seq_num_s); i++){		
			strncat(tmp,"0",1);	
		}
	sprintf(seq_num_s,"%s%d",tmp,seq_num);
	printf("GENERATED INT SEQ NB=%d\n",seq_num);
	printf("GENERATED CHAR SEQ NB=%s\n",seq_num_s);
		if (NULL != tmp){
			tmp=NULL;
			free(tmp);
		}
	//printf("GENERATED SEQ NB=%d\n", seq_num);
	sprintf(ctrl_head, "%s%s%s%s", OTG_CTRL_FLAG, time_s, seq_num_s,END_OTG_HEADER);

	printf("OTG CONTROL HEADER=%s\n",ctrl_head);

		if (NULL != time_s){
			time_s=NULL;
			free(time_s);
		}

		if (NULL != seq_num_s){
			seq_num_s=NULL;
			free(seq_num_s);
		}
	return(ctrl_head);
}



void free_addr_otg(){
	int i, j, k; 
		for (i=0; i<MAX_NUM_NODES; i++){
			for (j=0; j<MAX_NUM_NODES; j++){
				for (k=0; k<MAX_NUM_TRAFFIC_STATE; k++){

					if (NULL != g_otg->dst_ip[i][j][k]){
						g_otg->dst_ip[i][j][k]=NULL;
						free(g_otg->dst_ip[i][j][k]);
					}
				}
			}
		}
}



#ifdef STANDALONE

double get_emu_time(void){

return otg_info->emu_time;
}
#endif


