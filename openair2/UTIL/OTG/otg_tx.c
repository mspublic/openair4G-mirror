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
int type_header=0;
otg_hdr_t *otg_hdr_p;
char *background_data=NULL;

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
 default :
   LOG_E(OTG, "IDT distribution unknown \n");
 }
 
 LOG_D(OTG,"IDT :: Inter Departure Time Distribution= %d , val= %d\n", g_otg->idt_dist[src][dst][state],idt);
 return idt;
}


// otg_params.size Distribution Function to distribute the packet otg_params.size using the required distribution


int size_dist(int src, int dst, int state) {

  int size_data=0;
  LOG_D(OTG,"Size Distribution idx= %d \n", g_otg->size_dist[src][dst][state]);
  switch  (g_otg->size_dist[src][dst][state]) {
  case UNIFORM : 
    size_data = ceil(uniform_dist(g_otg->size_min[src][dst][state], g_otg->size_max[src][dst][state]));
    break;
  case GAUSSIAN :
    size_data = ceil(gaussian_dist((g_otg->size_max[src][dst][state] + g_otg->size_min[src][dst][state])/2 , g_otg->size_std_dev[src][dst][state]));
    break;
  case EXPONENTIAL : 
    size_data= ceil(exponential_dist(g_otg->size_lambda[src][dst][state])); //SIZE_COEF * 
    break;
  case POISSON :
    size_data =ceil(poisson_dist(g_otg->size_lambda[src][dst][state]));
    break;
  case FIXED :
    size_data=ceil(g_otg->size_min[src][dst][state]);
    break;
  case WEIBULL :
    size_data =ceil(weibull_dist(g_otg->size_scale[src][dst][state],g_otg->size_shape[src][dst][state] ));
    break;
  case PARETO :
    size_data =ceil(pareto_dist(g_otg->size_scale[src][dst][state],g_otg->size_shape[src][dst][state] ));
    break;
  case GAMMA :
    size_data =ceil(gamma_dist(g_otg->size_scale[src][dst][state],g_otg->size_shape[src][dst][state] ));
    break;
  case CAUCHY :
    size_data =ceil(cauchy_dist(g_otg->size_scale[src][dst][state],g_otg->size_shape[src][dst][state] ));
    break;
  default:
    LOG_E(OTG, "PKT Size Distribution unknown \n");
  }
  //Case when size overfill min and max values	
  size_data=adjust_size(size_data);
  LOG_D(OTG,"Packet :: Size=%d  Distribution= %d \n", size_data, g_otg->size_dist[src][dst][state]);
  
  return size_data;
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
  LOG_I(OTG," random_string :: Generated string= %s\n", data);
  return data;
}




char *packet_gen(int src, int dst, int ctime, int * pkt_size){ // when pdcp, ctime = frame cnt


  //double idt;
  int size;
  int flow_id=1;
  //  char *header=NULL;
  //  otg_hdr_t * otg_hdr=NULL;
  char *buffer_tx=NULL;
  //HEADER_TYPE header_type;
  otg_hdr_info_t *otg_hdr_info_p;
  unsigned int  byte_tx_count=0; 
  int state=ON_STATE; // default traffic state 
  int state_transition_prob=0;
  int hdr_size;
  int buffer_size = 0;
  *pkt_size = 0;
  set_ctime(ctime);
 //int background_size=0;

	
  //LOG_I(OTG,"num_nodes_tx:: %d , seed:: %d \n", g_otg->num_nodes, g_otg->seed);
    //LOG_I(OTG,"NODE_INFO (Source= %d, Destination= %d,State= %d) ctime %d \n", src, dst, state, otg_info->ctime);
    
  //LOG_I(OTG,"Transmission info: idt=%d, simulation time=%d \n", otg_info->idt[src][dst], ctime); 
  // do not generate packet for this pair of src, dst : no app type and/or no idt are defined	
  if ((g_otg->application_type[src][dst] == 0) && (g_otg->idt_dist[src][dst][state] == 0)){ //???? to fix 
    LOG_D(OTG,"Do not generate packet for this pair of src=%d, dst =%d: no app type and/or idt are defined\n", src, dst); 
    return NULL;	 
  }
#ifdef STANDALONE
  //pre-config for the standalone
  if (ctime<otg_info->ptime[src][dst]) //it happends when the emulation was finished
    otg_info->ptime[src][dst]=ctime;
  if (ctime==0)
    otg_info->idt[src][dst]=0; //for the standalone mode: the emulation is run several times, we need to initialise the idt to 0 when ctime=0
  //end pre-config
#endif 
  
  if (g_otg->num_state [src] > ON_STATE ){  // statefull traffic case, determine if we should do a state transition   
    switch (otg_info->state[src]) {
      state_transition_prob = uniform_dist(0,1);
    case ON_STATE:
      if ( state_transition_prob < g_otg->state_prob[src][ON_STATE] )
	otg_info->state[src]= ON_STATE;
      else //if (state_transition_prob < g_otg->state_prob[src][ON_STATE]  + g_otg->state_prob[src][OFF_STATE] )
	otg_info->state[src]= OFF_STATE; 
      // else
      //   otg_info->state[src]= ACTIVE_STATE; 
      break;
    case OFF_STATE:
      if ( state_transition_prob < g_otg->state_prob[src][ON_STATE] )
	otg_info->state[src]= ON_STATE;
      else //if (state_transition_prob < g_otg->state_prob[src][ON_STATE]  + g_otg->state_prob[src][OFF_STATE] )
	otg_info->state[src]= OFF_STATE; 
      // else
      //   otg_info->state[src]= ACTIVE_STATE; 
      break;
    case ACTIVE_STATE:
    default:
      otg_info->state[src]= ON_STATE; // switch to default state
      LOG_W(OTG,"Unknown state\n");
      break;
    }
  }
    state =  otg_info->state[src];

printf("INFO_SIM (src=%d, dst=%d) application=%d, idt dist =%d, pkts dist= %d\n", src, dst, g_otg->application_type[src][dst], g_otg->idt_dist[src][dst][state], g_otg->size_dist[src][dst][state]);

//  LOG_D(OTG,"INFO_SIM (src=%d, dst=%d) application=%d, idt dist =%d, pkts dist= %d\n", src, dst, g_otg->application_type[src][dst], g_otg->idt_dist[src][dst][state], g_otg->size_dist[src][dst][state]);
  
  if ((otg_info->idt[src][dst]==0) || (( (ctime-otg_info->ptime[src][dst]) > otg_info->idt[src][dst] ) )) {
  printf("Time To Transmit (Source= %d, Destination= %d,State= %d) , (IDT= %d ,ctime= %d, ptime= %d) \n", src, dst, state ,otg_info->idt[src][dst], ctime, otg_info->ptime[src][dst]); 
  LOG_D(OTG,"Time To Transmit (Source= %d, Destination= %d,State= %d) , (IDT= %d ,ctime= %d, ptime= %d) \n", src, dst, state ,otg_info->idt[src][dst], ctime, otg_info->ptime[src][dst]); 
    otg_info->ptime[src][dst]=ctime;	
    otg_info->idt[src][dst]=time_dist(src, dst, state); // update the idt for the next otg_tx
  }
  else {
  //check if there is background traffic to generate
    background_gen(src, dst, ctime);
    if(background_data!=NULL){
	//background_size=otg_info->size_background[src][dst] + sizeof(otg_hdr_info_t);
		
	packet= malloc(sizeof(*packet));
	packet->header=header_gen(header_size_gen(src)); //???? to modify to random
        packet->payload=background_data;

//Serialization :: BACKGROUND
	buffer_tx= (char*)malloc(strlen(packet->header) + strlen(packet->payload) + sizeof(otg_hdr_info_t)+ sizeof(otg_hdr_t));

	otg_hdr_info_p = (otg_hdr_info_t *) (&buffer_tx[byte_tx_count]);
    	otg_hdr_info_p->size = strlen(packet->header) + strlen(packet->payload) + sizeof(otg_hdr_info_t)+ sizeof(otg_hdr_t);
	otg_hdr_info_p->flag = 0xbbbb;
	byte_tx_count +=sizeof(otg_hdr_info_t);
	otg_hdr_p = (otg_hdr_t *) (&buffer_tx[byte_tx_count]);
  	otg_header_gen(flow_id, ctime, otg_info->seq_num_background[src][dst],type_header, state, strlen(packet->header) + strlen(packet->payload));
	byte_tx_count += sizeof(otg_hdr_t);
  	memcpy(&buffer_tx[byte_tx_count], packet->header, strlen(packet->header));
  	byte_tx_count += strlen(packet->header);	
  	memcpy(&buffer_tx[byte_tx_count], packet->payload, strlen(packet->payload));
       *pkt_size = sizeof(otg_hdr_info_t) + sizeof(otg_hdr_t) + strlen(packet->header) + strlen(packet->payload);

       otg_info->tx_num_bytes_background[src][dst]+=  *pkt_size ; 
       otg_info->tx_num_pkt_background[src][dst]+=1;
       otg_info->seq_num_background[src][dst]+=1;

       LOG_I(OTG,"PACKET SIZE: BACKGROUND TX [SRC %d][DST %d] :: time(%d), Seq num (%d), Total size (%d)\n", src, dst, ctime, otg_info->seq_num_background[src][dst], *pkt_size);

	return buffer_tx;

    }
    else
    return NULL; // do not generate the packet, and keep the idt

  }
  
  size=size_dist(src, dst, state); 
	printf("Generate Packet for (Source= %d, Destination= %d,State= %d) , pkt size dist= %d, simu time= %d ,packet size=%d \n", src, dst, state, g_otg->size_dist[src][dst][state], otg_info->ctime, size);
  // LOG_I(OTG,"Generate Packet for (Source= %d, Destination= %d,State= %d) , pkt size dist= %d, simu time= %d ,packet size=%d \n", src, dst, state, g_otg->size_dist[src][dst][state], otg_info->ctime, size);
  packet= malloc(sizeof(*packet));
  //LOG_I(OTG,"Payload size=%d\n",size);	  
  packet->payload=payload_pkts(size);
  //LOG_I(OTG,"packet_gen :: payload= (%d, %s) \n", size, packet->payload);
  //LOG_D(OTG,"==============STEP 1: OTG PAYLOAD OK============== \n");		
  packet->header=header_gen(header_size_gen(src));


  //LOG_I(OTG,"packet_gen :: protocol HEADER= (%d, %s) \n", strlen(packet->header),packet->header);
  //LOG_D(OTG,"==============STEP 2: OTG protocol HEADER OK============== \n");
  
  hdr_size=sizeof(otg_hdr_info_t) + sizeof(otg_hdr_t);

  otg_info->tx_num_bytes[src][dst]+=  hdr_size + strlen(packet->header) + strlen(packet->payload) ; 
  otg_info->tx_num_pkt[src][dst]+=1;
	
  
  // Serialization
  buffer_size = hdr_size + strlen(packet->header) + strlen(packet->payload);
  buffer_tx= (char*)malloc(buffer_size);
  otg_hdr_info_p = (otg_hdr_info_t *) (&buffer_tx[byte_tx_count]);
  otg_hdr_info_p->size = buffer_size;
  otg_hdr_info_p->flag = 0xffff;
  byte_tx_count = sizeof(otg_hdr_info_t);
  otg_hdr_p = (otg_hdr_t *) (&buffer_tx[byte_tx_count]);
  otg_header_gen(flow_id, ctime, otg_info->seq_num[src][dst],type_header, state, strlen(packet->header) + strlen(packet->payload));
  byte_tx_count += sizeof(otg_hdr_t);
  //LOG_D(OTG,"==============STEP 3: OTG control HEADER OK========== \n");
  
  memcpy(&buffer_tx[byte_tx_count], packet->header, strlen(packet->header));
  byte_tx_count += strlen(packet->header);	
  memcpy(&buffer_tx[byte_tx_count], packet->payload, strlen(packet->payload));
  //LOG_D(OTG,"==============STEP 4: PACKET OK============= \n");
  
  LOG_I(OTG,"PACKET SIZE TX [SRC %d][DST %d]:  time(%d), Seq num (%d), Total size (%d)\n", src, dst, ctime, otg_info->seq_num[src][dst], buffer_size);


  //add stats
  otg_info->header_type[src][dst]=type_header;
  otg_info->seq_num[src][dst]+=1;
  //end stats
  
  /* 
  if (NULL != packet)
    //free(packet);  									
    if (NULL != packet->payload)
      // free(packet->payload);
      if (NULL != packet->header)	
	//free(packet->header);
	*/  
  *pkt_size = buffer_size;
  
  return buffer_tx;

}


int header_size_gen(int src){

int size_header=0;
type_header=0;

	if (g_otg->ip_v[src]==1) { 
		size_header+=HDR_IP_v4_MIN;
		type_header+=0;
	}
	else if  (g_otg->ip_v[src]==2){	
		size_header+=HDR_IP_v6;
		type_header+=2;
		
	}
	
	if (g_otg->trans_proto[src]==1){	
 		size_header+= HDR_UDP ;
		type_header+=1;
	}	
	else if (g_otg->trans_proto[src]==2){
		size_header+= HDR_TCP;
		type_header+=2;
	}

printf(" src %d, version_HDR %d \n", src, size_header);
//LOG_I(OTG,"version_ %d, %d \n", type_header, size_header);

return size_header;

}


char *header_gen(int hdr_size){


  char *hdr=NULL;
  
  if (hdr_size>(sizeof(otg_hdr_info_t) + sizeof(otg_hdr_t)))
    hdr_size-=(sizeof(otg_hdr_info_t) + sizeof(otg_hdr_t));
  else
    LOG_W(OTG,"OTG Header not included inside packet header (OTG header:%d, Header%d)\n", hdr_size, sizeof(otg_hdr_info_t) + sizeof(otg_hdr_t) );
  
  hdr=(char*)malloc(hdr_size*sizeof(char));
  // if (generate_static_string)
  hdr=random_string(hdr_size,STATIC_STRING, HEADER_STRING);
  //hdr=random_string(hdr_size,RANDOM_STRING, HEADER_STRING);
  

return(hdr);

}


char *payload_pkts(int payload_size){
	
	char *payload=NULL;	
	payload=(char*)malloc(payload_size*sizeof(char));
	payload=random_string(payload_size, STATIC_STRING, PAYLOAD_STRING);
	//payload=random_string(payload_size, RANDOM_STRING, PAYLOAD_STRING);
	return (payload);

}


void otg_header_gen(int flow_id, int ctime, int seq_num, int hdr_type, int state, int size){


  otg_hdr_p->flow_id =flow_id;
  otg_hdr_p->time =ctime;
  otg_hdr_p->seq_num =seq_num;
  otg_hdr_p->hdr_type=hdr_type;
  otg_hdr_p->state = state;
  otg_hdr_p->pkts_size = size;

  LOG_D(OTG, " otg_hdr: HDR TYPE %i FLOW ID %i TIME %i  NUM SEQUENCE %i SIZE (PAYLOAD + HEADER) %i \n",
	otg_hdr_p->hdr_type, otg_hdr_p->flow_id, otg_hdr_p->time, otg_hdr_p->seq_num,otg_hdr_p->pkts_size);
  
}


void init_predef_traffic() {
int i;
int j;

LOG_I(OTG,"OTG_CONFIG num_node %d\n",  g_otg->num_nodes);


 for (i=0; i<g_otg->num_nodes; i++){ // src 
   for (j=0; j<g_otg->num_nodes; j++){ // dst
  
printf("OTG_CONFIG_, src = %d, dst = %d, application type= %d\n", i, j,  g_otg->application_type[i][j]);
  
     LOG_D(OTG,"OTG_CONFIG node (src=%d,dst=%d)\n",  i,j);
     
     switch  (g_otg->application_type[i][j]) {
     case  SCBR : 
       g_otg->trans_proto[i] = 1;
       g_otg->ip_v[i] = 1;
       g_otg->idt_dist[i][j][0] = FIXED;
       g_otg->idt_dist[i][j][1] = FIXED;
       g_otg->idt_min[i][j][0] =  10;
       g_otg->idt_min[i][j][1] =  10;
       g_otg->idt_max[i][j][0] =  10;
       g_otg->idt_max[i][j][1] =  10;
       g_otg->size_dist[i][j][0] = FIXED;
       g_otg->size_dist[i][j][1] = FIXED;
       g_otg->size_min[i][j][0] =  50;
       g_otg->size_min[i][j][1] =  50;
       g_otg->size_max[i][j][0] =  50;
       g_otg->size_max[i][j][1] =  50;
       LOG_I(OTG,"OTG_CONFIG SCBR, src = %d, dst = %d, dist type for size = %d\n", i, j, g_otg->size_dist[i][j][0]);
#ifdef STANDALONE
       g_otg->dst_port[i] = 0;
       g_otg->duration[i] = 1000;
#endif 
       break;
     case MCBR :
       g_otg->trans_proto[i] = 1;
       g_otg->ip_v[i] = 1;
       g_otg->idt_dist[i][j][0] = FIXED;
       g_otg->idt_dist[i][j][1] = FIXED;
       g_otg->idt_min[i][j][0] =  10;
       g_otg->idt_min[i][j][1] =  10;
       g_otg->idt_max[i][j][0] =  10;
       g_otg->idt_max[i][j][1] =  10;
       g_otg->size_dist[i][j][0] = FIXED;
       g_otg->size_dist[i][j][1] = FIXED;
       g_otg->size_min[i][j][0] =  512;
       g_otg->size_min[i][j][1] =  512;
       g_otg->size_max[i][j][0] =  512;
       g_otg->size_max[i][j][1] =  512;
       LOG_I(OTG,"OTG_CONFIG MCBR, src = %d, dst = %d, dist type for size = %d\n", i, j, g_otg->size_dist[i][j][0]);
#ifdef STANDALONE
       g_otg->dst_port[i] = 0;
       g_otg->duration[i] = 1000;
#endif 
       break;
     case BCBR :
       g_otg->trans_proto[i] = 1;
       g_otg->ip_v[i] = 1;
       g_otg->idt_dist[i][j][0] = FIXED;
       g_otg->idt_dist[i][j][1] = FIXED;
       g_otg->idt_min[i][j][0] =  10;
       g_otg->idt_min[i][j][1] =  10;
       g_otg->idt_max[i][j][0] =  10;
       g_otg->idt_max[i][j][1] =  10;
       g_otg->size_dist[i][j][0] = FIXED;
       g_otg->size_dist[i][j][1] = FIXED;
       g_otg->size_min[i][j][0] =  1024;
       g_otg->size_min[i][j][1] =  1024;
       g_otg->size_max[i][j][0] =  1024;
       g_otg->size_max[i][j][1] =  1024;
       LOG_I(OTG,"OTG_CONFIG BCBR, src = %d, dst = %d, dist type for size = %d\n", i, j, g_otg->size_dist[i][j][0]);
#ifdef STANDALONE
       g_otg->dst_port[i] = 0;
       g_otg->duration[i] = 1000;
#endif  
       break;
     case AUTO_PILOT : 
       g_otg->trans_proto[i] = 2;
       g_otg->ip_v[i] = 1;
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
       g_otg->trans_proto[i] = 2;
       g_otg->ip_v[i] = 1;
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
       g_otg->trans_proto[i] = 2;
       g_otg->ip_v[i] = 1;
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
       g_otg->trans_proto[i] = 2;
       g_otg->ip_v[i] = 1;
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
     default :
       LOG_E(OTG, "Unknown traffic type\n");
     }
   }
 }
}


void background_gen(int src, int dst, int ctime)
{ 

  background_data=NULL;


  //*size=100;

if (otg_info->idt_background[src][dst]==0){
//*size=ceil(lognormal_dist(5.46,0.85));
otg_info->size_background[src][dst]=ceil(lognormal_dist(5.46,0.85));
   if (otg_info->size_background[src][dst]>1500)
     otg_info->size_background[src][dst]=1500;
 otg_info->idt_background[src][dst]=ceil(((otg_info->size_background[src][dst])*8000)/pow(10, lognormal_dist(1.3525, 0.1954)));

  //otg_info->idt_background[src][dst]=2;
}
  LOG_I(OTG,"BACKGROUND TRAFFIC:: (src=%d, dst=%d) pkts size=%d idt=%d  \n", src, dst, otg_info->size_background[src][dst],otg_info->idt_background[src][dst]);

// if (otg_info->idt_background[src][dst]==(ctime-otg_info->ptime_background)){
if (((ctime-otg_info->ptime_background) >  otg_info->idt_background[src][dst])  ){
   LOG_I(OTG,"BACKGROUND TRAFFIC:: OK !!\n");
   otg_info->ptime_background=ctime;
   background_data=(char*)malloc((otg_info->size_background[src][dst])*sizeof(char));
   background_data=random_string(otg_info->size_background[src][dst], STATIC_STRING, PAYLOAD_STRING);
   LOG_I(OTG,"BACKGROUND TRAFFIC:: random_string= %s\n", background_data);
  }
  else {
    LOG_I(OTG,"[SRC %d][DST %d] BACKGROUND TRAFFIC:: not the time to transmit= (idt=%d, ctime=%d,ptime=%d )\n", src, dst, otg_info->idt_background[src][dst], ctime, otg_info->ptime_background);
  background_data=NULL;
  }

}




