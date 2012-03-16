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


#include "otg_tx.h" 





// Time Distribution function to distribute the inter-departure time using the required distribution


int time_dist(int src, int dst, int state) {

int idt=0;

	if (g_otg->idt_dist[src][dst][state] == UNIFORM)
		idt =  ceil((uniform_dist(g_otg->idt_min[src][dst][state], g_otg->idt_max[src][dst][state])));
	else if (g_otg->idt_dist[src][dst][state] == GAUSSIAN)
		idt =  ceil((gaussian_dist((g_otg->idt_max[src][dst][state] + g_otg->idt_min[src][dst][state])/2 , g_otg->idt_std_dev[src][dst][state])));
	else if (g_otg->idt_dist[src][dst][state] == EXPONENTIAL)
		idt=  ceil((exponential_dist(g_otg->idt_lambda[src][dst][state])));
	else if (g_otg->idt_dist[src][dst][state] == POISSON)
		idt =  ceil((poisson_dist(g_otg->idt_lambda[src][dst][state])));
	else if (g_otg->idt_dist[src][dst][state] == FIXED)
		idt = ceil((g_otg->idt_min[src][dst][state])); //IDT_TH *
	else if (g_otg->idt_dist[src][dst][state] == WEIBULL)
		idt =ceil(weibull_dist(g_otg->idt_scale[src][dst][state],g_otg->idt_shape[src][dst][state] ));
	else if (g_otg->idt_dist[src][dst][state] == PARETO)
		idt =ceil(pareto_dist(g_otg->idt_scale[src][dst][state],g_otg->idt_shape[src][dst][state] ));
	else if (g_otg->idt_dist[src][dst][state] == GAMMA)
		idt =ceil(gamma_dist(g_otg->idt_scale[src][dst][state],g_otg->idt_shape[src][dst][state] ));
	else if (g_otg->idt_dist[src][dst][state] == CAUCHY)
		idt =ceil(cauchy_dist(g_otg->idt_scale[src][dst][state],g_otg->idt_shape[src][dst][state] ));
	LOG_I(OTG,"IDT :: Inter Departure Time Distribution= %d , val= %d\n", g_otg->idt_dist[src][dst][state],idt);
return idt;
}


// otg_params.size Distribution Function to distribute the packet otg_params.size using the required distribution


int size_dist(int src, int dst, int state) {
int size;
	

LOG_I(OTG,"Size Distribution idx= %d \n", g_otg->size_dist[src][dst][state]);

	if (g_otg->size_dist[src][dst][state] == UNIFORM)
		size = ceil(uniform_dist(g_otg->size_min[src][dst][state], g_otg->size_max[src][dst][state]));
	else if (g_otg->size_dist[src][dst][state] == GAUSSIAN)
		size = ceil(gaussian_dist((g_otg->size_max[src][dst][state] + g_otg->size_min[src][dst][state])/2 , g_otg->size_std_dev[src][dst][state]));
	else if (g_otg->size_dist[src][dst][state] == EXPONENTIAL)
		size= ceil(exponential_dist(g_otg->size_lambda[src][dst][state])); //SIZE_COEF * 
	else if (g_otg->size_dist[src][dst][state] == POISSON)
		size =ceil(poisson_dist(g_otg->size_lambda[src][dst][state]));
	else if (g_otg->size_dist[src][dst][state] == FIXED)
		size=ceil(g_otg->size_min[src][dst][state]);
	else if (g_otg->size_dist[src][dst][state] == WEIBULL)
		size =ceil(weibull_dist(g_otg->size_scale[src][dst][state],g_otg->size_shape[src][dst][state] ));
	else if (g_otg->size_dist[src][dst][state] == PARETO)
		size =ceil(pareto_dist(g_otg->size_scale[src][dst][state],g_otg->size_shape[src][dst][state] ));
	else if (g_otg->size_dist[src][dst][state] == GAMMA)
		size =ceil(gamma_dist(g_otg->size_scale[src][dst][state],g_otg->size_shape[src][dst][state] ));
	else if (g_otg->size_dist[src][dst][state] == CAUCHY)
		size =ceil(cauchy_dist(g_otg->size_scale[src][dst][state],g_otg->size_shape[src][dst][state] ));

	//Case when size overfill min and max values	
	size=adjust_size(size);
	LOG_I(OTG,"Packet :: Size=%d  Distribution= %d \n", size, g_otg->size_dist[src][dst][state]);

	return size;


}

int adjust_size(int size){

	if (size<PAYLOAD_MIN){
		LOG_W(OTG,"Packet Size=%d out of range, size=%d \n", size, PAYLOAD_MIN);
		size=PAYLOAD_MIN;
	}
	else if  (size>PAYLOAD_MAX){
		LOG_W(OTG,"Packet Size=%d out of range, size=%d \n", size, PAYLOAD_MAX);
		size=PAYLOAD_MAX;
	}
return(size);
}


void init_predef_otg() {
int i;
int j;


LOG_I(OTG,"OTG_CONFIG num_node %d\n",  g_otg->num_nodes);

	for (i=0; i<g_otg->num_nodes; i++){
		for (j=0; j<g_otg->num_nodes; j++){


LOG_I(OTG,"OTG_CONFIG node (src=%d,dst=%d)\n",  i,j);


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
				LOG_I(OTG,"OTG_CONFIG CBR, src = %d, dst = %d, dist type for size = %d\n", i, j, g_otg->size_dist[i][j][0]);
				g_otg->size_min[i][j][0] =  50;
				g_otg->size_min[i][j][1] =  50;
				g_otg->size_max[i][j][0] =  50;
				g_otg->size_max[i][j][1] =  50;
				g_otg->size_std_dev[i][j][0] = 0;
				g_otg->size_std_dev[i][j][1] = 0;
				g_otg->size_lambda[i][j][0] = 0;
				g_otg->size_lambda[i][j][1] = 0;
				#ifdef STANDALONE
					g_otg->dst_port[i] = 0;
					g_otg->duration[i] = 1000;

				#endif 

			} else if (g_otg->application_type[i][j] == AUTO_PILOT) { 

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
				g_otg->size_min[i][j][0] =  800;
				g_otg->size_min[i][j][1] =  0;
				g_otg->size_max[i][j][0] =  800; 
				g_otg->size_max[i][j][1] =  0;
				g_otg->size_std_dev[i][j][0] = 0;
				g_otg->size_std_dev[i][j][1] = 0;
				g_otg->size_lambda[i][j][0] = 0;
				g_otg->size_lambda[i][j][1] = 1/800;
				#ifdef STANDALONE
					g_otg->dst_port[i] = 0;
					g_otg->duration[i] = 1000;
				#endif 

			} else if (g_otg->application_type[i][j] == BICYCLE_RACE) { 

				g_otg->trans_proto[i] = 0;
				g_otg->ip_v[i] = 0;
				g_otg->idt_dist[i][j][0] = UNIFORM;
				g_otg->idt_dist[i][j][1] = EXPONENTIAL;
				LOG_I(OTG,"OTG_CONFIG M2M_BR, src = %d, dst = %d, dist IDT = %d\n", i, j, g_otg->idt_dist[i][j][0]);
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
				g_otg->size_min[i][j][0] =  800;
				g_otg->size_min[i][j][1] =  0;
				g_otg->size_max[i][j][0] =  800;
				g_otg->size_max[i][j][1] =  0;
				g_otg->size_std_dev[i][j][0] = 0;
				g_otg->size_std_dev[i][j][1] = 0;
				g_otg->size_lambda[i][j][0] = 0;
				g_otg->size_lambda[i][j][1] = 1/800;
				#ifdef STANDALONE
					g_otg->dst_port[i] = 0;
					g_otg->duration[i] = 1000;
				#endif 

			} else if (g_otg->application_type[i][j] == OPENARENA) { 

				g_otg->trans_proto[i] = 0;
				g_otg->ip_v[i] = 0;
				g_otg->idt_dist[i][j][0] = UNIFORM;
				g_otg->idt_dist[i][j][1] = UNIFORM;
				LOG_I(OTG,"OTG_CONFIG GAMING_OA, src = %d, dst = %d, dist IDT = %d\n", i, j, g_otg->idt_dist[i][j][0]);
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
				g_otg->size_min[i][j][0] =  5;
				g_otg->size_min[i][j][1] =  5;
				g_otg->size_max[i][j][0] =  43;
				g_otg->size_max[i][j][1] =  43;
				g_otg->size_std_dev[i][j][0] = 5;
				g_otg->size_std_dev[i][j][1] = 5;
				g_otg->size_lambda[i][j][0] = 0;
				g_otg->size_lambda[i][j][1] = 0;
				#ifdef STANDALONE
					g_otg->dst_port[i] = 0;
					g_otg->duration[i] = 1000;
				#endif 

			} else if (g_otg->application_type[i][j] == TEAM_FORTRESS) { 

				g_otg->trans_proto[i] = 0;
				g_otg->ip_v[i] = 0;
				g_otg->idt_dist[i][j][0] = UNIFORM;
				g_otg->idt_dist[i][j][1] = UNIFORM;
				LOG_I(OTG,"OTG_CONFIG GAMING_TF, src = %d, dst = %d, dist IDT = %d\n", i, j, g_otg->idt_dist[i][j][0]);
				g_otg->idt_min[i][j][0] =  31;
				g_otg->idt_min[i][j][1] =  31;
				g_otg->idt_max[i][j][0] =  42;
				g_otg->idt_max[i][j][1] =  42;
				g_otg->idt_std_dev[i][j][0] = 0;
				g_otg->idt_std_dev[i][j][1] = 0;
				g_otg->idt_lambda[i][j][0] = 0;
				g_otg->idt_lambda[i][j][1] = 0;
				g_otg->size_dist[i][j][0] = GAUSSIAN;
				g_otg->size_dist[i][j][1] = GAUSSIAN;
				g_otg->size_min[i][j][0] =  5;
				g_otg->size_min[i][j][1] =  5;
				g_otg->size_max[i][j][0] =  43;
				g_otg->size_max[i][j][1] =  43;
				g_otg->size_std_dev[i][j][0] = 5;
				g_otg->size_std_dev[i][j][1] = 5;
				g_otg->size_lambda[i][j][0] = 0;
				g_otg->size_lambda[i][j][1] = 0;
				#ifdef STANDALONE
					g_otg->dst_port[i] = 0;
					g_otg->duration[i] = 1000;
				#endif 

			}
		}
	}
}




// Generate a random string[size]
char *random_string(int size, ALPHABET data_type, char *data_string) {
	char *data=NULL;
	int i, pos;


	data=(char*)malloc(size*sizeof(char*));
#ifdef STRING_STATIC	
	data=strndup(data_string + (strlen(data_string) - size), strlen(data_string));	
	LOG_D(OTG," random_string :: STATIC\n");
#else
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
#endif 
	LOG_I(OTG," random_string :: Generated string= %s\n", data);
	return data;
}




int packet_gen(int src, int dst, int state, int ctime){ // when pdcp, ctime = frame cnt

	//double idt;
	int size;
	char *header=NULL;
	otg_hdr_t * otg_hdr=NULL;
	packet_t *packet=NULL;
	HEADER_TYPE header_type;
	

	set_ctime(ctime);	
	LOG_I(OTG,"num_nodes_tx:: %d , seed:: %d \n", g_otg->num_nodes, g_otg->seed);

	LOG_I(OTG,"NODE_INFO (Source= %d, Destination= %d,State= %d) ctime %d \n", src, dst, state, otg_info->ctime);


	LOG_I(OTG,"INFO_SIM (src=%d, dst=%d, state=%d) application=%d, idt dist =%d, pkts dist= %d\n", src, dst, state, g_otg->application_type[src][dst], g_otg->idt_dist[src][dst][state], g_otg->size_dist[src][dst][state]);

	LOG_I(OTG,"Transmission info: idt=%d, simulation time=%d \n", otg_info->idt[src][dst], ctime); 
	// do not generate packet for this pair of src, dst : no app type and/or idt are defined	
	if ((g_otg->application_type[src][dst] == 0) && (g_otg->idt_dist[src][dst][0] == 0)){
        	LOG_I(OTG,"Do not generate packet for this pair of src=%d, dst =%d: no app type and/or idt are defined\n", src, dst); 
		return 0;	 
	}

//pre-config for the standalone
	if (ctime<otg_info->ptime[src][dst][state]) //it happends when the emulation was finished
		otg_info->ptime[src][dst][state]=ctime;
	if (ctime==0)
	otg_info->idt[src][dst]=0; //for the standalone mode: the emulation is run several times, we need to initialise the idt to 0 when ctime=0
//end pre-config

	if ((otg_info->idt[src][dst]==(ctime-otg_info->ptime[src][dst][state])) || (otg_info->idt[src][dst]==0)) {
			
		   LOG_I(OTG,"Time To Transmit (Source= %d, Destination= %d,State= %d) , (IDT= %d ,simu time= %d, previous packet time= %d) \n", src, dst, state ,otg_info->idt[src][dst], ctime, otg_info->ptime[src][dst][state]); 
		   otg_info->ptime[src][dst][state]=ctime;	
		   otg_info->idt[src][dst]=time_dist(src, dst, state); // update the idt for the next otg_tx
		}
		else {
		   LOG_I(OTG,"It is not the time to transmit (ctime= %d, previous time=%d, packet idt=%d),  node( %d,%d) \n", ctime,otg_info->ptime[src][dst][state], otg_info->idt[src][dst], src, dst);  
		   return 0; // do not generate the packet, and keep the idt
			}

	

	size=size_dist(src, dst, state);	
	LOG_I(OTG,"Generate Packet for (Source= %d, Destination= %d,State= %d) , pkt size dist= %d, simu time= %d ,packet size=%d \n",
	src, dst, state, g_otg->size_dist[src][dst][state], otg_info->ctime, size);


 	packet= malloc(sizeof(*packet));


	LOG_I(OTG,"==============STEP 1: OTG PAYLOAD OK============== \n");		
	LOG_I(OTG,"Payload size=%d\n",size);	  
	packet->payload=payload_pkts(size);
	LOG_I(OTG,"packet_gen :: payload= (%d, %s) \n", size, packet->payload);
	
	
	printf("==============STEP 2: OTG protocol HEADER OK============== \n");	
	packet->header=header_gen(g_otg->ip_v[src], g_otg->trans_proto[src]);
	LOG_I(OTG,"packet_gen :: protocol HEADER= (%d, %s) \n", strlen(packet->header),packet->header);
	
	if ((g_otg->ip_v[src]==TCP) && (g_otg->trans_proto[src]==IPV4))
		header_type=TCP_IPV4;
	else if ((g_otg->ip_v[src]==UDP) && (g_otg->trans_proto[src]==IPV4))
	header_type=UDP_IPV4;
	else if ((g_otg->ip_v[src]==TCP) && (g_otg->trans_proto[src]==IPV6))
	header_type=TCP_IPV6;
	else if ((g_otg->ip_v[src]==UDP) && (g_otg->trans_proto[src]==IPV6))
	header_type=UDP_IPV6;
 
	otg_info->header_type[src][dst]=header_type;

	otg_info->seq_num[src][dst]+=1;
	
	LOG_I(OTG,"==============STEP 3: OTG control HEADER OK========== \n");
	otg_hdr=otg_header_gen(otg_info->ctime,  otg_info->seq_num[src][dst], header_type);

    	packet->otg_hdr= (otg_hdr_t*)otg_hdr;




	LOG_I(OTG,"==============STEP 4: PACKET OK============= \n");	

	LOG_I(OTG,"PACKET SIZE (TX): time(%d)otg header(%d), header (%d), payload (%d), Total (%d) \n", ctime, sizeof(otg_hdr_t), strlen(packet->header), strlen(packet->payload),( sizeof(otg_hdr_t) + strlen(packet->header) + strlen(packet->payload)));


	otg_info->tx_num_bytes[src][dst]+= sizeof(otg_hdr_t) + strlen(packet->header) + strlen(packet->payload) ; 
	otg_info->tx_num_pkt[src][dst]+=1;

// Serialization
	memcpy(&buffer_tx, packet,  sizeof(otg_hdr_t) + strlen(packet->header) + strlen(packet->payload));


	if (NULL != otg_hdr){
		otg_hdr=NULL;
		free(otg_hdr);
	}


	if (NULL != packet){
			packet=NULL;  					
			free(packet);
			LOG_I(OTG,"RX Free packet\n");
	}

	//return packet;
	return 1;
}



char *header_gen(int ip_v, int trans_proto){

	int hdr_size=0;
	char *hdr=NULL;

	if (ip_v==0) { 
		hdr_size=HDR_IP_v4_MIN;
	}
	else if  (ip_v==1){	
		hdr_size=HDR_IP_v6;
	}
	
	if (trans_proto==0){	
 		hdr_size=hdr_size + HDR_UDP ;
	}	
	else if (trans_proto==1){
		hdr_size=hdr_size + HDR_TCP;
	}

		if (hdr_size> sizeof(otg_hdr_t))
			hdr_size-=sizeof(otg_hdr_t);
		else if (hdr_size<= sizeof(otg_hdr_t))
			LOG_E(OTG,"header_gen :: ERROR: header size (%d) < OTG header size (%d)\n", hdr_size, sizeof(otg_hdr_t));
		 

	hdr=(char*)malloc(hdr_size*sizeof(char*));
	hdr=random_string(hdr_size,NUM, HEADER_STRING);

return(hdr);

}


char *payload_pkts(int payload_size){
	
	char *payload=NULL;	
	payload=(char*)malloc(payload_size*sizeof(char*));
	payload=random_string(payload_size,NUM_LETTER, PAYLOAD_STRING);
	return (payload);

}



otg_hdr_t *otg_header_gen(int time, int seq_num, HEADER_TYPE header_type){


	otg_hdr->flow_id=1; // we manage only one flow	
	otg_hdr->time=time; 		
	otg_hdr->seq_num=seq_num; 	  
	otg_hdr->hdr_type=header_type; 
	

return otg_hdr; 

}






