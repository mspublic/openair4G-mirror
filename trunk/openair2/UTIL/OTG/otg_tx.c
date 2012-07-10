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
//packet_t *packet=NULL;
int type_header=0;
otg_hdr_t *otg_hdr_p;


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
 case LOG_NORMAL :
   idt =ceil((lognormal_dist((g_otg->idt_max[src][dst][state] + g_otg->idt_min[src][dst][state])/2 , g_otg->idt_std_dev[src][dst][state])));
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
 case LOG_NORMAL :
   size_data =ceil((lognormal_dist((g_otg->size_max[src][dst][state] + g_otg->size_min[src][dst][state])/2 , g_otg->size_std_dev[src][dst][state])));
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





char *packet_gen(int src, int dst, int ctime, int * pkt_size){ // when pdcp, ctime = frame cnt


  int size;
  unsigned int flow_id=1;
  char *buffer_tx=NULL;
  otg_hdr_info_t *otg_hdr_info_p;
  unsigned int  byte_tx_count=0; 
  unsigned char state=ON_STATE; // default traffic state 
  int state_transition_prob=0;
  int hdr_size;
  unsigned int buffer_size = 0;
  *pkt_size = 0;
  set_ctime(ctime);
  unsigned int gen_pkts=0;
  unsigned int background_ok=0; 
  unsigned char flow;
  unsigned int seq_num;
  unsigned int flow_id_background=1;
  unsigned int flag;
  char *payload=NULL;
  char *header=NULL;
	

  //LOG_I(OTG,"num_nodes_tx:: %d , seed:: %d \n", g_otg->num_nodes, g_otg->seed);
  //if (g_otg->idt_dist[src][dst][0]>0)
  //LOG_D(OTG,"NODE_INFO (Source= %d, Destination= %d,idt= %d) ctime %d \n", src, dst, g_otg->idt_dist[src][dst][0], otg_info->ctime);
    

  // do not generate packet for this pair of src, dst : no app type and/or no idt are defined	
  if ((g_otg->application_type[src][dst]==0) && (g_otg->background[src][dst]==0)){ //???? to fix 
    LOG_D(OTG,"Do not generate packet for this pair of src=%d, dst =%d: no app type and/or idt are defined\n", src, dst); 
    return NULL;	 
  }

else if ((g_otg->application_type[src][dst] >0) || (g_otg->idt_dist[src][dst][state] > 0)) {

#ifdef STANDALONE
  //pre-config for the standalone
  if (ctime<otg_info->ptime[src][dst]) //it happends when the emulation was finished
    otg_info->ptime[src][dst]=ctime;
  if (ctime==0)
    otg_info->idt[src][dst]=0; //for the standalone mode: the emulation is run several times, we need to initialise the idt to 0 when ctime=0
  //end pre-config
#endif 
  
  if (g_otg->num_state[src] > ON_STATE ){  // statefull traffic case, determine if we should do a state transition   
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
//  LOG_D(OTG,"INFO_SIM (src=%d, dst=%d) application=%d, idt dist =%d, pkts dist= %d\n", src, dst, g_otg->application_type[src][dst], g_otg->idt_dist[src][dst][state], g_otg->size_dist[src][dst][state]);
  
  if ((otg_info->idt[src][dst]==0) || (( (ctime-otg_info->ptime[src][dst]) >= otg_info->idt[src][dst] ) )) {
  LOG_D(OTG,"Time To Transmit::OK (Source= %d, Destination= %d,State= %d) , (IDT= %d ,ctime= %d, ptime= %d) \n", src, dst, state ,otg_info->idt[src][dst], ctime, otg_info->ptime[src][dst]); 
    otg_info->ptime[src][dst]=ctime;	
    otg_info->idt[src][dst]=time_dist(src, dst, state); // update the idt for the next otg_tx
    gen_pkts=1;
  }  //check if there is background traffic to generate
  else if  ((background_gen(src, dst, ctime) != 0) && (gen_pkts==0) && (g_otg->background[src][dst]==1)) {
    background_ok=1;
    LOG_I(OTG,"BACKGROUND :: Time To Transmit [SRC %d][DST %d] \n", src, dst);
  }
  else{  
    // LOG_D(OTG,"Time To Transmit::NO (Source= %d, Destination= %d,State= %d) , (IDT= %d ,ctime= %d, ptime= %d) \n", src, dst, state ,otg_info->idt[src][dst], ctime, otg_info->ptime[src][dst]);
   return NULL; // do not generate the packet, and keep the idt
  }
  size=size_dist(src, dst, state);
  hdr_size=sizeof(otg_hdr_info_t) + sizeof(otg_hdr_t);
}

else if (((g_otg->application_type[src][dst]==0)||(g_otg->idt_dist[src][dst][state]==0))&&(g_otg->background[src][dst]==1)){ //The case when we configure only background between src and dst, without data traffic
	if (background_gen(src, dst, ctime)!=0){
	    background_ok=1;
	    LOG_I(OTG,"(only) BACKGROUND :: Time To Transmit [SRC %d][DST %d] \n", src, dst);
	}
	else 
	return NULL;
}


if (background_ok==0){
  header=header_gen(header_size_gen(src));
  payload=payload_pkts(size) ;
  flag=0xffff;
  flow=flow_id;
  seq_num=otg_info->seq_num[src][dst];
  otg_info->header_type[src][dst]=type_header;
  otg_info->seq_num[src][dst]+=1;
  otg_info->tx_num_bytes[src][dst]+=  hdr_size + strlen(header) + strlen(payload) ; 
  otg_info->tx_num_pkt[src][dst]+=1;
if (size!=strlen(payload))
  LOG_I(OTG,"data to send : size %d, strlen %d, %s \n", size, strlen(payload), payload);
  }
else{
  header=header_gen(header_size_gen_background(src)); 
  payload=payload_pkts(otg_info->size_background[src][dst]);
  flag=0xbbbb;
  flow=flow_id_background;
  seq_num=otg_info->seq_num_background[src][dst];
  otg_info->tx_num_bytes_background[src][dst]+=  hdr_size + strlen(header) + strlen(payload) ; 
  otg_info->tx_num_pkt_background[src][dst]+=1;
  otg_info->seq_num_background[src][dst]+=1;
if (otg_info->size_background[src][dst]!=strlen(payload))
  LOG_I(OTG,"data to send : size %d, strlen %d, %s \n", otg_info->size_background[src][dst], strlen(payload), payload);
}


// Serialization
 buffer_size = hdr_size + strlen(header) + strlen(payload);
  buffer_tx= (char*)malloc(buffer_size);
  otg_hdr_info_p = (otg_hdr_info_t *) (&buffer_tx[byte_tx_count]);
  otg_hdr_info_p->size = buffer_size;
  otg_hdr_info_p->flag = flag; // (background_ok==0)? 0xffff : 0xbbbb;
  byte_tx_count = sizeof(otg_hdr_info_t);
  otg_hdr_p = (otg_hdr_t *) (&buffer_tx[byte_tx_count]);
  otg_header_gen(flow, ctime, seq_num,1, state); //???? Fix HDR Type 
  byte_tx_count += sizeof(otg_hdr_t);
  memcpy(&buffer_tx[byte_tx_count], header, strlen(header));
  byte_tx_count += strlen(header);
  memcpy(&buffer_tx[byte_tx_count], payload, strlen(payload));
  byte_tx_count +=strlen(payload);
  LOG_I(OTG,"PACKET SIZE TX [SRC %d][DST %d]: Flag (0%x), time(%d), Seq num (%d), Total size (%d)\n", src, dst, flag, ctime, seq_num, byte_tx_count);
 


if (buffer_tx[byte_tx_count-1]=='F'){ 
 LOG_I(OTG, "BAD_PACKET TX %c ctime %d \n", buffer_tx[byte_tx_count-1], ctime);}
else 
 LOG_I(OTG, "BAD_PACKET TX ERROR %c ctime %d \n", buffer_tx[byte_tx_count-1], ctime);
 
  *pkt_size = buffer_size;
  if (payload!=NULL)
    free(payload);
  if (header!=NULL)
    free(header); 

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
LOG_I(OTG,"version_ %d, %d \n", type_header, size_header);

return size_header;

}


// Generate a random string[size]
char *random_string(int size, ALPHABET data_type, char *data_string) {

  char *data=NULL;
  int i, pos;
  data=(char*)malloc(size*sizeof(char));
  switch (data_type){
  case STATIC_STRING:
    //data=strndup(data_string + (strlen(data_string) - size), strlen(data_string));
      data=strndup(data_string + (strlen(data_string) - size), size);	
    break;
  case RANDOM_STRING:
    for(i=0;i<size;i++){
      pos = rand()%(strlen(data_string));		
      data[i]=data_string[pos];
    }
    break;
  } 

  return data;
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


void otg_header_gen(int flow_id, int ctime, int seq_num, int hdr_type, int state){


  otg_hdr_p->flow_id =flow_id;
  otg_hdr_p->time =ctime;
  otg_hdr_p->seq_num =seq_num;
  otg_hdr_p->hdr_type=hdr_type;
  otg_hdr_p->state = state;
  //otg_hdr_p->pkts_size = size;

  LOG_D(OTG, " otg_hdr: HDR TYPE %i FLOW ID %i TIME %i  NUM SEQUENCE %i SIZE (PAYLOAD) %i \n",
	otg_hdr_p->hdr_type, otg_hdr_p->flow_id, otg_hdr_p->time, otg_hdr_p->seq_num,otg_hdr_p->pkts_size);
  
}


void init_predef_traffic() {
int i;
int j;

//LOG_I(OTG,"OTG_CONFIG num_node %d\n",  g_otg->num_nodes);


 for (i=0; i<g_otg->num_nodes; i++){ // src 
   for (j=0; j<g_otg->num_nodes; j++){ // dst
  
//LOG_I("OTG_CONFIG_, src = %d, dst = %d, application type= %d\n", i, j,  g_otg->application_type[i][j]);
  
     //LOG_D(OTG,"OTG_CONFIG node (src=%d,dst=%d)\n",  i,j);
     
     switch  (g_otg->application_type[i][j]) {
     case  SCBR : 
       g_otg->trans_proto[i] = 1;
       g_otg->ip_v[i] = 1;
       g_otg->idt_dist[i][j][0] = FIXED;
       g_otg->idt_dist[i][j][1] = FIXED;
       g_otg->idt_min[i][j][0] =  100; 
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
       g_otg->idt_min[i][j][0] =  100;
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
       g_otg->idt_min[i][j][0] =  100;
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
       LOG_I(OTG,"OTG_CONFIG M2M_AP, src = %d, dst = %d, dist IDT = %d\n", i, j, g_otg->idt_dist[i][j][0]);
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
     case NO_PREDEFINED_TRAFFIC : 
       LOG_I(OTG, "[SRC %d][DST %d] No predefined Traffic \n", i, j);
       g_otg->trans_proto[i] = 0;
       g_otg->ip_v[i] = 0;
       g_otg->idt_dist[i][j][0] = 0;
       g_otg->idt_dist[i][j][1] = 0;
       g_otg->idt_min[i][j][0] =  0; 
       g_otg->idt_min[i][j][1] =  0;
       g_otg->idt_max[i][j][0] =  0;
       g_otg->idt_max[i][j][1] =  0;
       g_otg->size_dist[i][j][0] = FIXED;
       g_otg->size_dist[i][j][1] = FIXED;
       g_otg->size_min[i][j][0] =  0;
       g_otg->size_min[i][j][1] =  0;
       g_otg->size_max[i][j][0] = 0;
       g_otg->size_max[i][j][1] =  0;
       break;
     default:
       LOG_E(OTG, "[SRC %d][DST %d] Unknown traffic type\n", i, j);
     }
   }
 }
}


int background_gen(int src, int dst, int ctime){ 
/*if (otg_info->idt_background[src][dst]==0){
  //size=ceil(lognormal_dist(5.46,0.85));
   otg_info->size_background[src][dst]=ceil(lognormal_dist(5.46,0.85));
   if (otg_info->size_background[src][dst]>1500)
       otg_info->size_background[src][dst]=1500;
   otg_info->idt_background[src][dst]=ceil(((otg_info->size_background[src][dst])*8000)/pow(10, lognormal_dist(1.3525, 0.1954)));  
}*/
  if ((((ctime-otg_info->ptime_background) >=  otg_info->idt_background[src][dst])) ||  (otg_info->idt_background[src][dst]==0)){
     LOG_I(OTG,"[SRC %d][DST %d] BACKGROUND TRAFFIC:: OK (idt=%d, ctime=%d,ptime=%d ) !!\n", src, dst, otg_info->idt_background[src][dst], ctime, otg_info->ptime_background);

	otg_info->size_background[src][dst]=ceil(lognormal_dist(5.46,0.85));
   	if (otg_info->size_background[src][dst]>1500)
       		otg_info->size_background[src][dst]=1500;
   LOG_I(OTG,"BACKGROUND TRAFFIC:: (src=%d, dst=%d) pkts size=%d idt=%d  \n", src, dst, otg_info->size_background[src][dst],otg_info->idt_background[src][dst]);
   otg_info->idt_background[src][dst]=ceil(((otg_info->size_background[src][dst])*8000)/pow(10, lognormal_dist(1.3525, 0.1954)));
     otg_info->ptime_background=ctime;	
     return 1;
  }
   else {
     // LOG_D(OTG,"[SRC %d][DST %d] BACKGROUND TRAFFIC:: not the time to transmit= (idt=%d, ctime=%d,ptime=%d ) size= %d \n", src, dst, otg_info->idt_background[src][dst], ctime, otg_info->ptime_background, otg_info->size_background[src][dst]);
     return 0;
   }

}



int header_size_gen_background(src){
 int size_header=0;
  if(g_otg->trans_proto_background[src]==0)
  g_otg->trans_proto_background[src]= rand() % (TCP_IPV6 - UDP_IPV4 + 1) + UDP_IPV4;

 switch (g_otg->trans_proto_background[src]) {
 case  UDP_IPV4:
   size_header=HDR_IP_v4_MIN + HDR_UDP;
 break;
 case  UDP_IPV6:
   size_header=HDR_IP_v6 + HDR_UDP;
 break;
 case  TCP_IPV4:
   size_header=HDR_IP_v4_MIN + HDR_TCP;
 break;
 case  TCP_IPV6:
   size_header=HDR_IP_v6 + HDR_TCP;
 break;
 default :
   size_header=HDR_IP_v6 + HDR_TCP;
}

  LOG_I(OTG," [SRC %d]  BACKGROUND TRAFFIC:: size header%d \n", src, size_header);

return size_header;
}



