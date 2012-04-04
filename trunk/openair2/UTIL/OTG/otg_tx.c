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
#include "otg_vars.h"

//may be put in vars
packet_t *packet=NULL;

// Time Distribution function to distribute the inter-departure time using the required distribution


int time_dist(int src, int dst, int state) {

int idt=0;

 switch (g_otg->idt_dist[src][dst][state]) {
 case  UNIFORM:
   idt =  ceil((uniform_dist(g_otg->idt_min[src][dst][state], g_otg->idt_max[src][dst][state])));
   break;
 case GAUSSIAN:
   idt =  ceil((gaussian_dist((g_otg->idt_max[src][dst][state] + g_otg->idt_min[src][dst][state])/2 , g_otg->idt_std_dev[src][dst][state])));
   break;
 case EXPONENTIAL :
   idt=  ceil((exponential_dist(g_otg->idt_lambda[src][dst][state])));
   break;
 case  POISSON:
   idt =  ceil((poisson_dist(g_otg->idt_lambda[src][dst][state])));
   break;
 case FIXED :
   idt = ceil((g_otg->idt_min[src][dst][state])); //IDT_TH *
   break;
 case WEIBULL :
   idt =ceil(weibull_dist(g_otg->idt_scale[src][dst][state],g_otg->idt_shape[src][dst][state] ));
   break;
 case PARETO :
   idt =ceil(pareto_dist(g_otg->idt_scale[src][dst][state],g_otg->idt_shape[src][dst][state] ));
   break;
 case GAMMA :
   idt =ceil(gamma_dist(g_otg->idt_scale[src][dst][state],g_otg->idt_shape[src][dst][state] ));
   break;
 case CAUCHY :
   idt =ceil(cauchy_dist(g_otg->idt_scale[src][dst][state],g_otg->idt_shape[src][dst][state] ));
   break;
 deafult :
   LOG_E(OTG, "IDT distribution unknown \n");
 }
 
 LOG_I(OTG,"IDT :: Inter Departure Time Distribution= %d , val= %d\n", g_otg->idt_dist[src][dst][state],idt);
 return idt;
}


// otg_params.size Distribution Function to distribute the packet otg_params.size using the required distribution


int size_dist(int src, int dst, int state) {
int size;
	

LOG_I(OTG,"Size Distribution idx= %d \n", g_otg->size_dist[src][dst][state]);

 switch  (g_otg->size_dist[src][dst][state]) {
 case UNIFORM : 
   size = ceil(uniform_dist(g_otg->size_min[src][dst][state], g_otg->size_max[src][dst][state]));
   break;
 case GAUSSIAN :
   size = ceil(gaussian_dist((g_otg->size_max[src][dst][state] + g_otg->size_min[src][dst][state])/2 , g_otg->size_std_dev[src][dst][state]));
   break;
 case EXPONENTIAL : 
   size= ceil(exponential_dist(g_otg->size_lambda[src][dst][state])); //SIZE_COEF * 
   break;
 case POISSON :
   size =ceil(poisson_dist(g_otg->size_lambda[src][dst][state]));
   break;
 case FIXED :
   size=ceil(g_otg->size_min[src][dst][state]);
   break;
 case WEIBULL :
   size =ceil(weibull_dist(g_otg->size_scale[src][dst][state],g_otg->size_shape[src][dst][state] ));
   break;
 case PARETO :
   size =ceil(pareto_dist(g_otg->size_scale[src][dst][state],g_otg->size_shape[src][dst][state] ));
   break;
 case GAMMA :
   size =ceil(gamma_dist(g_otg->size_scale[src][dst][state],g_otg->size_shape[src][dst][state] ));
   break;
 case CAUCHY :
   size =ceil(cauchy_dist(g_otg->size_scale[src][dst][state],g_otg->size_shape[src][dst][state] ));
   break;
 deafult:
   LOG_E(OTG, "PKT Size Distribution unknown \n");
 }
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

 for (i=0; i<g_otg->num_nodes; i++){ // src 
   for (j=0; j<g_otg->num_nodes; j++){ // dst
     
     LOG_I(OTG,"OTG_CONFIG node (src=%d,dst=%d)\n",  i,j);
     
     switch  (g_otg->application_type[i][j]) {
     case  CBR : 
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
       break;
     case AUTO_PILOT : 
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
       break;
     case BICYCLE_RACE :  
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
       break;
     case OPENARENA : 
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
       break;  
     case TEAM_FORTRESS : 
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
       break;
     deafult :
       LOG_E(OTG, "Unknown traffic type\n");
     }
   }
 }
}

// Generate a random string[size]
char *random_string(int size, ALPHABET data_type, char *data_string) {
  char *data=NULL;
  int i, pos;
  
  
  data=(char*)malloc(size*sizeof(char*));
  switch (data_type){
  case STATIC_STRING:
    data=strndup(data_string + (strlen(data_string) - size), strlen(data_string));	
    break;
  case RANDOM_STRING:
    for(i=0;i<size;i++){
      pos = rand()%(strlen(data_string));		
      data[i]=data_string[pos];
    }
    break;
  }
#endif 
  LOG_I(OTG," random_string :: Generated string= %s\n", data);
  return data;
}




char *packet_gen(int src, int dst, int state, int ctime){ // when pdcp, ctime = frame cnt

  //double idt;
  int size;
  int flow_id=1;
  char *header=NULL;
  otg_hdr_t * otg_hdr=NULL;
  char *buffer_tx=NULL;
  HEADER_TYPE header_type;
  otg_hdr_t *otg_hdr_p;
  otg_hdr_info_t *otg_hdr_info_p;
  unsigned int  byte_tx_count=0; 
  int header_size;
  int payload_size;
  
  set_ctime(ctime);	
  //LOG_I(OTG,"num_nodes_tx:: %d , seed:: %d \n", g_otg->num_nodes, g_otg->seed);
    //LOG_I(OTG,"NODE_INFO (Source= %d, Destination= %d,State= %d) ctime %d \n", src, dst, state, otg_info->ctime);
  LOG_D(OTG,"INFO_SIM (src=%d, dst=%d, state=%d) application=%d, idt dist =%d, pkts dist= %d\n", src, dst, state, g_otg->application_type[src][dst], g_otg->idt_dist[src][dst][state], g_otg->size_dist[src][dst][state]);
  
  //LOG_I(OTG,"Transmission info: idt=%d, simulation time=%d \n", otg_info->idt[src][dst], ctime); 
  // do not generate packet for this pair of src, dst : no app type and/or no idt are defined	
  if ((g_otg->application_type[src][dst] == 0) && (g_otg->idt_dist[src][dst][0] == 0)){ //???? to fix 
    LOG_I(OTG,"Do not generate packet for this pair of src=%d, dst =%d: no app type and/or idt are defined\n", src, dst); 
    return NULL;	 
  }
  
  //pre-config for the standalone
  if (ctime<otg_info->ptime[src][dst][state]) //it happends when the emulation was finished
    otg_info->ptime[src][dst][state]=ctime;
#ifdef STANDALONE
  if (ctime==0)
    otg_info->idt[src][dst]=0; //for the standalone mode: the emulation is run several times, we need to initialise the idt to 0 when ctime=0
  //end pre-config
#enif 
  if ((otg_info->idt[src][dst]==(ctime-otg_info->ptime[src][dst][state])) || (otg_info->idt[src][dst]==0)) {
    
    LOG_I(OTG,"Time To Transmit (Source= %d, Destination= %d,State= %d) , (IDT= %d ,simu time= %d, previous packet time= %d) \n", src, dst, state ,otg_info->idt[src][dst], ctime, otg_info->ptime[src][dst][state]); 
    otg_info->ptime[src][dst][state]=ctime;	
    otg_info->idt[src][dst]=time_dist(src, dst, state); // update the idt for the next otg_tx
  }
  else {
    LOG_I(OTG,"Wait for idt to expires (ctime= %d, previous time=%d, packet idt=%d),  node( %d,%d) \n", ctime,otg_info->ptime[src][dst][state], otg_info->idt[src][dst], src, dst);  
    return NULL; // do not generate the packet, and keep the idt
  }
  
  size=size_dist(src, dst, state);	
  header_size=header_size_gen(src);
  LOG_I(OTG,"Generate Packet for (Source= %d, Destination= %d,State= %d) , pkt size dist= %d, simu time= %d ,packet size=%d hdr size %d \n",
	src, dst, state, g_otg->size_dist[src][dst][state], otg_info->ctime, size, header_size);
  
  packet= malloc(sizeof(packet_t));
  LOG_D(OTG,"==============STEP 1: OTG PAYLOAD OK============== \n");		
		  
  packet->payload=payload_pkts(size);
  LOG_I(OTG,"packet_gen :: payload= (%d, %s) \n", size, packet->payload);
  
  LOG_D(OTG,"==============STEP 2: OTG protocol HEADER OK============== \n");	
  packet->header=header_gen(header_size);
  LOG_D(OTG,"packet_gen :: protocol HEADER= (%d, %s) \n", strlen(packet->header),packet->header);

  //payload_size=strlen(packet->header);
  otg_info->header_type[src][dst]=header_type; // fix me ????
  otg_info->seq_num[src][dst]+=1;

  // LOG_D(OTG,"==============STEP 3: OTG control HEADER OK========== \n");
  //otg_header_gen(flow_id, otg_info->ctime,  otg_info->seq_num[src][dst], size);
  //LOG_D(OTG,"==============STEP 4: PACKET OK============= \n");	

  //LOG_I(OTG,"PACKET SIZE (TX): time(%d)otg header(%d), header (%d), payload (%d), Total (%d) \n", ctime, otg_header_size, strlen(packet->header), strlen(packet->payload),( otg_header_size + strlen(packet->header) + strlen(packet->payload)));

  otg_info->tx_num_bytes[src][dst]+= sizeof(otg_hdr_info_t) + sizeof(otg_hdr_t) + strlen(packet->header) + strlen(packet->payload) ; 
  otg_info->tx_num_pkt[src][dst]+=1;

  // Serialization
  buffer_tx= (char*)malloc(sizeof(otg_hdr_info_t) + sizeof(otg_hdr_t) + strlen(packet->header) + strlen(packet->payload));
  otg_hdr_info_p = (otg_hdr_info_t *) buffer_tx[byte_tx_count];
  
  otg_hdr_info_p->size= sizeof(otg_hdr_info_t) + sizeof(otg_hdr_t) + strlen(packet->header) + strlen(packet->payload);
  otg_hdr_info_p->flag=11;
  byte_tx_count = sizeof(otg_hdr_info_t);
  otg_hdr_p = (otg_hdr_t *) buffer_tx[byte_tx_count];
  otg_hdr_p->flow_id =flow_id;
  otg_hdr_p->time =otg_info->ctime;
  otg_hdr_p->seq_num =otg_info->seq_num[src][dst];
  
  byte_tx_count += sizeof(otg_hdr_t);
  memcpy(buffer_tx[byte_tx_count], packet->header, strlen(packet->header));
  byte_tx_count += strlen(packet->header);	
  memcpy(buffer_tx[byte_tx_count], packet->payload, strlen(packet->payload));
  
  if (NULL != otg_hdr_info_p){
    free(otg_hdr_info_p);
    otg_hdr_info_p=NULL;  					
    LOG_D(OTG,"Free OTG otg_hdr_info\n");
  }
  if (NULL != otg_hdr_p){
    free(otg_hdr_p);
    otg_hdr=NULL;  								
    LOG_D(OTG,"Free OTG header\n");
  }
  if (NULL != packet){
    free(packet);
    packet=NULL;  					
    LOG_D(OTG,"Free packet\n");
  }
  return buffer_tx;
}


int header_size_gen(int src){

int size_header=0;

	if (g_otg->ip_v[src]==0) { 
		size_header+=HDR_IP_v4_MIN;
	}
	else if  (g_otg->ip_v[src]==1){	
		size_header+=HDR_IP_v6;
	}
	
	if (g_otg->trans_proto[src]==0){	
 		size_header+= HDR_UDP ;
	}	
	else if (g_otg->trans_proto[src]==1){
		size_header+= HDR_TCP;
	}

return size_header;

}


char *header_gen(int hdr_size){


	char *hdr=NULL;


	hdr=(char*)malloc(hdr_size*sizeof(char));
	// if (generate_static_string)
	hdr=random_string(hdr_size,STATIC_STRING, HEADER_STRING);
	
return(hdr);

}


char *payload_pkts(int payload_size){
	
	char *payload=NULL;	
	payload=(char*)malloc(payload_size*sizeof(char));
	payload=random_string(payload_size, STATIC_STRING, PAYLOAD_STRING);
	return (payload);

}


void otg_header_gen(int flow_id, int time, int seq_num, int payload_size){

/*
packet->flag=OTG_FLAG;
packet->flow_id=&flow_id; // we manage only one flow	
packet->time=&time;
packet->payload_size=&payload_size;
packet->seq_num=&seq_num; 


printf( "HEADER_ TX: FLAG %s\n", packet->flag);
printf( "HEADER_ TX: FLOW ID %i\n", *packet->flow_id);
printf( "HEADER_ TX: TIME %i\n", *packet->time);
printf( "HEADER_ TX: NUM SEQUENCE %i\n", *packet->seq_num);
//printf( "HEADER_ TX: HEADER SIZE %i\n", *packet->header_size);
printf( "HEADER_ TX: PAYLOAD SIZE %i\n", *packet->payload_size);

*/

}





