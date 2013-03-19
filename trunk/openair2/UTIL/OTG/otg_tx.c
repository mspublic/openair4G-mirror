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

extern unsigned char NB_eNB_INST;
extern unsigned char NB_UE_INST;

// char string for payload gen
const char HEADER_STRING[] = "048717272261061594909177115977673656394812939088509638561159848103044447631759621785741859753883189837913087874397230808338408049820398430984093477437879374893649639479575023646394693665969273071047";
const char PAYLOAD_STRING[] = "UVk5miARQfZGDFKf1wS0dt57kHigd0fXNrUZCjIhpyOS4pZWMHOP1GPdgXmlPtarLUjd3Rmkg05bhUZWtDDmdhrl5EzMZz6DkhIg0Uq7NlaU8ZGrt9EzgVLdr9SiBOLLXiTN3aMInMrlDYFYZ8n5WYbfZTnpz13lbMY4OBE4eWfIMLvBLLyzzzEqjUGILBVMfKGVccPi0VSCyg28RqAiR3z1P6zryk4FWFp0G78AUT1hZWhGcGOTDcKj9bCzny592m1Dj123KWczIm5KVLupO7AP83flqamimfLz6GtHrz5ZN2BAEVQjUhYSc35s5jDhofIlL2U4qPT3Ilsd7amTjaCl5zE0L89ZeIcPCWKSEuNdH5gG8sojuSvph1hU0gG4QOLhCk15IE8eCeMCz2LTL68U0hEQqeM6UmgmA9j7Eid7oPzQHbzj8A30HzGXGhWpt4CT3MSwWVvcCWSbYjkYGgOhHj5csTsONWyGAh5l3qquf8v3jGRSRu0nGXqYILCkw1SX9Na46qodrN6BnPl49djH2AuAaYKAStoR9oL7I1aZG6rVLFPMIZiAqF1tuDVcX9VWnyTVpTMXR6GtBp5bgfDyKuT4ZE9MDUASikGA5hoMfX5Gf2Ml7eLGBtEqZF4rouczHI0DRfgX4ev967n6dYFFkaXbFTvWdykN5bfMinzcrWeqVrmZhTvtUkvq3Rc9enM9qTNz6cDo0HHM0VD8EYtpaPH3yG2CYGDgogHlkaCcHaOyViyq8RH8wf4WQWoHuTNG1kWdkpgTrWic5Gv5p24O9YAPMOn6A1IsdvwpOF85qj8nPvj4nfIo385HOjGfadzfBXueruaKEa0lvbhLgS1bQWKv5fE7k2cMPzQ8USIpUyBhBGUHsLKaykvsr1qDTueAUWAGH8VqyozZZkyhWahjmFEEwU6hhcK1Z9wv9jOAAeqopQvbQFm4aQzzBwGIAhBqhZMiarIBwYPOFdPmK1hKHIa94GGtQbMZ0n83IGt6w8K3dqfOhmpQWqSRZscFwPuo4uhC0ByoC9hFpnizCBfoRZ7Gj9cGOzVLT2eMtD0XC8rUnDiR3p7Ke4ho6lWMLHmtCr7VWYIpY19dtiWoyU0FQ7VMlHoWeBhIUfuG54WVVX0h5Mvvvo0cnLQzh4knysVhAfQw9EhXq94mLrI9GydUaTihOvydQikfq2CrvLeNK81msBlYYoK0aT7OewTUI51YYufj7kYGkPVDf7t5n3VnMV3ShMERKwFyTNHQZyo9ccFibYdoT1FyMAfVeMDO89bUMKAD7RFaT9kUZpaIiH5W7dIbPcPPdSBSr1krKPtuQEeHVgf4OcQLfpOWtsEya4ftXpNw76RPCXmp4rjKt1mCh0pBiTYkQ5GDnj2khLZMzb1uua6R1ika8ACglrs1n0vDbnNjZEVpIMK4OGLFOXIOn9UBserI4Pa63PhUl49TGLNjiqQWdnAsolTKrcjnSklN1swcmyVU8B5gTO4Y3vhkG2U2";
const char FIXED_STRING[]="ABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCDABCD";
//may be put in vars
//packet_t *packet=NULL;
int type_header=0;
unsigned int state=OFF_STATE; // default traffic state 
unsigned int application=0;


// Time Distribution function to distribute the inter-departure time using the required distribution


int time_dist(int src, int dst,int application, int state) {

int idt=0;

 switch (g_otg->idt_dist[src][dst][application][state]) {
 case  UNIFORM:
   idt =  ceil((uniform_dist(g_otg->idt_min[src][dst][application][state], g_otg->idt_max[src][dst][application][state])));
   break;
 case GAUSSIAN:
   idt =  ceil((gaussian_dist((g_otg->idt_max[src][dst][application][state] + g_otg->idt_min[src][dst][application][state])/2 , g_otg->idt_std_dev[src][dst][application][state])));
   break;
 case EXPONENTIAL :
   idt=  ceil((exponential_dist(g_otg->idt_lambda[src][dst][application][state])));
   break;
 case  POISSON:
   idt =  ceil((poisson_dist(g_otg->idt_lambda[src][dst][application][state])));
   break;
 case FIXED :
   idt = ceil((g_otg->idt_min[src][dst][application][state])); //IDT_TH *
   break;
 case WEIBULL :
   idt =ceil(weibull_dist(g_otg->idt_scale[src][dst][application][state],g_otg->idt_shape[src][dst][application][state] ));
   break;
 case PARETO :
   idt =ceil(pareto_dist(g_otg->idt_scale[src][dst][application][state],g_otg->idt_shape[src][dst][application][state] ));
   break;
 case GAMMA :
   idt =ceil(gamma_dist(g_otg->idt_scale[src][dst][application][state],g_otg->idt_shape[src][dst][application][state] ));
   break;
 case CAUCHY :
   idt =ceil(cauchy_dist(g_otg->idt_scale[src][dst][application][state],g_otg->idt_shape[src][dst][application][state] ));
   break;
 case LOG_NORMAL :
   idt =ceil((lognormal_dist((g_otg->idt_max[src][dst][application][state] + g_otg->idt_min[src][dst][application][state])/2 , g_otg->idt_std_dev[src][dst][application][state])));
   break;
 default :
   idt =0;
   LOG_W(OTG, "IDT distribution unknown, set to 0 \n");
 }
 
 LOG_D(OTG,"IDT :: Inter Departure Time Distribution= %d , val= %d\n", g_otg->idt_dist[src][dst][application][state],idt);
 return idt;
}


// otg_params.size Distribution Function to distribute the packet otg_params.size using the required distribution


int size_dist(int src, int dst, int application, int state) {

  int size_data=0;

	if (state==PU_STATE)
		size_data=g_otg->pu_size_pkts[src][dst][application];

	else if (state==ED_STATE)
		size_data=g_otg->ed_size_pkts[src][dst][application];
else{
  		LOG_D(OTG,"Size Distribution idx= %d \n", g_otg->size_dist[src][dst][application][state]);
  		switch  (g_otg->size_dist[src][dst][application][state]) {
  			case UNIFORM : 
    			size_data = ceil(uniform_dist(g_otg->size_min[src][dst][application][state], g_otg->size_max[src][dst][application][state]));
    			break;
  			case GAUSSIAN :
    			size_data = ceil(gaussian_dist((g_otg->size_max[src][dst][application][state] + g_otg->size_min[src][dst][application][state])/2 , g_otg->size_std_dev[src][dst][application][state]));
    			break;
  			case EXPONENTIAL :
    			size_data= ceil(exponential_dist(g_otg->size_lambda[src][dst][application][state])); //SIZE_COEF * 
    			break;
  			case POISSON :
    			size_data =ceil(poisson_dist(g_otg->size_lambda[src][dst][application][state]));
    			break;
  			case FIXED :
    			size_data=ceil(g_otg->size_min[src][dst][application][state]);
    			break;
  			case WEIBULL :
    			size_data =ceil(weibull_dist(g_otg->size_scale[src][dst][application][state],g_otg->size_shape[src][dst][application][state] ));
    			break;
  			case PARETO :
    			size_data =ceil(pareto_dist(g_otg->size_scale[src][dst][application][state],g_otg->size_shape[src][dst][application][state] ));
    			break;
  			case GAMMA :
    			size_data =ceil(gamma_dist(g_otg->size_scale[src][dst][application][state],g_otg->size_shape[src][dst][application][state] ));
    			break;
  			case CAUCHY :
    			size_data =ceil(cauchy_dist(g_otg->size_scale[src][dst][application][state],g_otg->size_shape[src][dst][application][state] ));
    			break;
 				case LOG_NORMAL :
   				size_data =ceil((lognormal_dist((g_otg->size_max[src][dst][application][state] + g_otg->size_min[src][dst][application][state])/2 , g_otg->size_std_dev[src][dst][application][state])));
   				break;
  			default:
    			LOG_E(OTG, "PKT Size Distribution unknown \n");
  }

}
  //Case when size overfill min and max values	
  size_data=adjust_size(size_data);
  LOG_D(OTG,"[src %d] [dst %d] [application %d] [state %d]Packet :: Size=%d  Distribution= %d \n", src, dst, application, state, size_data, g_otg->size_dist[src][dst][application][state]);
  
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





unsigned char *packet_gen(int src, int dst, int ctime, int * pkt_size){ // when pdcp, ctime = frame cnt

	//unsigned char *packet=NULL;
	unsigned int size=0;
	unsigned int buffer_size =0;
  unsigned char flow;
  unsigned int flow_id_background=1;
  unsigned int seq_num;
  unsigned int flag;
  char *payload=NULL;
  char *header=NULL;

 LOG_I(OTG,"MAX_TX_INFO %d %d \n",NB_eNB_INST,  NB_UE_INST);

	set_ctime(ctime);
	*pkt_size=0;

	init_packet_gen(src, dst);
	size=check_data_transmit(src,dst,ctime);

	if ((size>0) || (otg_info->traffic_type_background[src][dst]==1)) { 

	/* No aggregation for background traffic   */
	if (otg_info->traffic_type_background[src][dst]==0){
  	header = random_string(otg_info->header_size[src][dst], g_otg->packet_gen_type, HEADER_ALPHABET);
  	payload = random_string(size, g_otg->packet_gen_type, PAYLOAD_ALPHABET);
  	flag=0xffff;
  	flow=otg_info->flow_id[src][dst];
  	seq_num=otg_info->seq_num[src][dst];
  	otg_info->header_type[src][dst]=type_header;
  	otg_info->seq_num[src][dst]+=1;
  	otg_info->tx_num_bytes[src][dst]+=  otg_hdr_size(src,dst) + strlen(header) + strlen(payload) ; 
  	otg_info->tx_num_pkt[src][dst]+=1;
  	if (size!=strlen(payload))
  	  LOG_E(OTG,"[%d][%d] [0x %x] The expected packet size does not match the payload size : size %d, strlen %d, seq_num %d packet: |%s|%s| \n", src, dst, flag, 	size, strlen(payload), seq_num, header, payload);
  	else 
  	  LOG_T(OTG,"[%d][%d] [0x %x] [m2m Aggre %d] [Flow %d]TX INFO pkt at time %d Size= [payload %d] [Total %d] with seq num %d, state=%d : |%s|%s| \n", src, dst, flag, otg_info->m2m_aggregation[src][dst],otg_info->flow_id[src][dst], ctime,  size, strlen(header)+strlen(payload), seq_num,state, header, payload);
  
	 } 	
	else {
		if ((g_otg->aggregation_level[src][dst][application]*otg_info->size_background[src][dst])<=PAYLOAD_MAX)
			otg_info->size_background[src][dst]=g_otg->aggregation_level[src][dst][application]*otg_info->size_background[src][dst];
		else{
			otg_info->size_background[src][dst]=PAYLOAD_MAX;
    	LOG_E(OTG,"[BACKGROUND] Aggregated packet larger than PAYLOAD_MAX, payload is limited to PAYLOAD_MAX \n");
	}
 	 header =random_string(header_size_gen_background(src,dst),  g_otg->packet_gen_type, HEADER_ALPHABET);
 	 payload = random_string(otg_info->size_background[src][dst],  g_otg->packet_gen_type, PAYLOAD_ALPHABET);
	  flag=0xbbbb;
 	 flow=flow_id_background;
 	 seq_num=otg_info->seq_num_background[src][dst];
  	otg_info->tx_num_bytes_background[src][dst]+= otg_hdr_size(src,dst) + strlen(header) + strlen(payload) ; 
  	otg_info->tx_num_pkt_background[src][dst]+=1;
  	otg_info->seq_num_background[src][dst]+=1;
  	if (otg_info->size_background[src][dst]!=strlen(payload))
   	 LOG_E(OTG,"[%d][%d] [0x %x] The expected packet size does not match the payload size : size %d, strlen %d, seq num %d, packet |%s|%s| \n", src, dst, flag, otg_info->size_background[src][dst], strlen(payload), seq_num, header, payload);
 		else
    	LOG_T(OTG,"[%d][%d][0x %x] TX INFO pkt at time %d size is %d with seq num %d, state=%d : |%s|%s| \n", src, dst, flag,ctime, otg_info->size_background[src][dst], seq_num, state, header, payload);
	}
 
 	buffer_size = otg_hdr_size(src,dst) + strlen(header) + strlen(payload);
	*pkt_size = buffer_size;
	if (src<NB_eNB_INST)
		otg_info->tx_total_bytes_dl+=buffer_size;
	else
		otg_info->tx_total_bytes_ul+=buffer_size;

 	return serialize_buffer(header, payload, buffer_size, otg_info->traffic_type[src][dst], flag, flow, ctime, seq_num, otg_info->header_type_app[src][dst][flow], state, otg_info->m2m_aggregation[src][dst]);
	}

	else 
		return NULL;


}



int otg_hdr_size(int src, int dst){
	if (otg_info->hdr_size[src][dst]==0)
		otg_info->hdr_size[src][dst]=sizeof(otg_hdr_info_t) + sizeof(otg_hdr_t); 
	return otg_info->hdr_size[src][dst];
}

void init_packet_gen(int src, int dst){
	otg_info->m2m_aggregation[src][dst]=0;
	otg_info->flow_id[src][dst]=0;
	otg_info->traffic_type[src][dst]=0;
	otg_info->traffic_type_background[src][dst]=0;
}



int check_data_transmit(int src,int dst, int ctime){

	unsigned int size=0;


	for (application=0; application<g_otg->application_idx[src][dst]; application++){  
	otg_info->gen_pkts=0;

	//LOG_D(OTG,"FLOW_INFO [src %d][dst %d] [IDX %d] [APPLICATION TYPE %d] MAX %d [M2M %d ]\n", src, dst, application , g_otg->application_type[src][dst][application],g_otg->application_idx[src][dst], g_otg->m2m[src][dst][application]);

  // do not generate packet for this pair of src, dst : no app type and/or no idt are defined	
  if ((g_otg->application_type[src][dst][application]==0)&&(g_otg->idt_dist[src][dst][application][PE_STATE]==0)){  
    //LOG_D(OTG,"Do not generate packet for this pair of src=%d, dst =%d\n", src, dst); 
    size+=0;	 
  }

	else if ((g_otg->application_type[src][dst][application] >0) || (g_otg->idt_dist[src][dst][application][PE_STATE] > 0)) {
		state = get_application_state(src, dst, application, ctime);

		#ifdef STANDALONE
  	//pre-config for the standalone
  	if (ctime<otg_info->ptime[src][dst][application]) //it happends when the emulation was finished
    	otg_info->ptime[src][dst][application]=ctime;
  	if (ctime==0)
    	otg_info->idt[src][dst][application]=0; //for the standalone mode: the emulation is run several times, we need to initialise the idt to 0 when ctime=0
  	//end pre-config
		#endif 
		//LOG_D(OTG,"MY_STATE %d \n", state);
 
		if (state!=OFF_STATE) {

  		if (((state==PU_STATE)||(state==ED_STATE))|| (otg_info->idt[src][dst][application]==0) || (( (ctime-otg_info->ptime[src][dst][application]) >= otg_info->idt[src][dst][application] ) )) {
  			LOG_D(OTG,"Time To Transmit::OK (Source= %d, Destination= %d, Application %d, State= %d) , (IDT= %d ,ctime= %d, ptime= %d) \n", 
				src, dst ,application, state, otg_info->idt[src][dst][application], ctime, otg_info->ptime[src][dst][application]); 
    		otg_info->ptime[src][dst][application]=ctime;	

				if (state==PE_STATE)  //compute the IDT only for PE STATE
				otg_info->idt[src][dst][application]=time_dist(src, dst, application,state);
				otg_info->gen_pkts=1;
				header_size_gen(src,dst, application); 
				//for(i=1;i<=g_otg->aggregation_level[src][dst][application];i++)
				if   (g_otg->m2m[src][dst][application]==M2M){
			  size+=size_dist(src, dst, application,state);
					if (otg_info->header_size_app[src][dst][application] > otg_info->header_size[src][dst]) /*adapt the header to the application (increment the header if the the new header size is 			largest that the already computed)*/
						otg_info->header_size[src][dst]+=otg_info->header_size_app[src][dst][application]; 
						otg_info->m2m_aggregation[src][dst]++;
						otg_info->flow_id[src][dst]=application;
						otg_info->traffic_type[src][dst]=M2M;
				}
				else{ 
					/* For the case of non M2M traffic: when more than one flows transmit data in the same time
				 --> the second flow transmit  (because of non data aggragation)  */
					size=size_dist(src, dst, application,state); 
					otg_info->header_size[src][dst]=otg_info->header_size_app[src][dst][application]; 
					otg_info->flow_id[src][dst]=application;
					otg_info->traffic_type[src][dst]=g_otg->application_type[src][dst][application];
				} 


				/* if the aggregated size is less than PAYLOAD_MAX the traffic is aggregated, otherwise size=PAYLOAD_MAX */
				if (size>=(PAYLOAD_MAX-(sizeof(otg_hdr_info_t) + sizeof(otg_hdr_t) + otg_info->header_size[src][dst]))) {
					size=PAYLOAD_MAX- (sizeof(otg_hdr_info_t) + sizeof(otg_hdr_t) + otg_info->header_size[src][dst]);
    			LOG_E(OTG,"Aggregated packet larger than PAYLOAD_MAX, payload is limited to PAYLOAD_MAX \n");
				}

  		}  //check if there is background traffic to generate
  		else if ((otg_info->gen_pkts==0) && (g_otg->background[src][dst][application]==1)&&(background_gen(src, dst, ctime)!=0)){ // the gen_pkts condition could be relaxed here
    		otg_info->traffic_type_background[src][dst]=1;
    		LOG_D(OTG,"[BACKGROUND=%d] Time To Transmit [SRC %d][DST %d] \n", otg_info->traffic_type_background[src][dst], src, dst);
  		}
  		
		}	

	}
}

return size;
}


unsigned int get_application_state(int src, int dst, int application, int ctime){

	switch (g_otg->application_type[src][dst][application]){
	case VOIP_G711:
	case VOIP_G729:
		voip_traffic(src, dst, application, ctime);
		return otg_info->voip_state[src][dst][application];
    break;
  default:
		state_management(src,dst,application, ctime);
		return otg_info->state[src][dst][application];
		break;	
	}
}

void header_size_gen(int src, int dst, int application){

unsigned int size_header=0;
unsigned int type_header=0;

	if (otg_info->header_size_app[src][dst][application]==0)
	{
		if (g_otg->ip_v[src][dst][application]==1) { 
			size_header+=HDR_IP_v4_MIN;
			otg_info->header_type_app[src][dst][application]+=0;
		}
		else if  (g_otg->ip_v[src][dst][application]==2){	
			size_header+=HDR_IP_v6;
			otg_info->header_type_app[src][dst][application]+=2;
			
		}
		
		if (g_otg->trans_proto[src][dst][application]==1){	
 			size_header+= HDR_UDP ;
			otg_info->header_type_app[src][dst][application]+=1;
		}	
		else if (g_otg->trans_proto[src][dst][application]==2){
			size_header+= HDR_TCP;
			otg_info->header_type_app[src][dst][application]+=2;
		}
	
		if ((g_otg->application_type[src][dst][application]==VOIP_G711)||(g_otg->application_type[src][dst][application]==VOIP_G729))
 			size_header+=RTP_HEADER;
		LOG_D(OTG,"Header size is %d [IP V=%d][PROTO=%d]\n",  size_header, g_otg->ip_v[src][dst][application],g_otg->trans_proto[src][dst][application]);
		otg_info->header_size_app[src][dst][application]=size_header;
	}		
}


// Generate a random string[size]
char *random_string(int size, ALPHABET_GEN mode, ALPHABET_TYPE data_type) {
  char *data=NULL;
  int i=0,start=0;
  switch (mode){
  case REPEAT_STRING:
    start = uniform_dist (0, abs(strlen(FIXED_STRING)- size - 1));
    return str_sub (FIXED_STRING, start, start + size -1);
    break;
  case SUBSTRACT_STRING:
    //data=strndup(data_string + (strlen(data_string) - size), strlen(data_string));
    //data=strndup(data_string + (strlen(data_string) - size), size);	
    if (data_type == HEADER_ALPHABET){
      start = uniform_dist (0, abs(strlen(HEADER_STRING)- size - 1));
      return str_sub (HEADER_STRING, start, start + size -1);
    }else if (data_type == PAYLOAD_ALPHABET) {
      start = uniform_dist (0, abs(strlen(PAYLOAD_STRING)- size - 1));
      return str_sub (PAYLOAD_STRING, start, start+size - 1 );
    }else 
      LOG_E(OTG, "unsupported alphabet data type \n");
    break;
  case RANDOM_POSITION:
    data=(char*)malloc((size+1)*sizeof(char));
    for(i=0;i<=size;i++){
      if (data_type == HEADER_ALPHABET)
	data[i]=HEADER_STRING [(int)uniform_dist(0, 128)];
      else if (data_type == PAYLOAD_ALPHABET) 
	data[i]=PAYLOAD_STRING [(int)uniform_dist(0, 1500)];
      else
	LOG_E(OTG,"unsupported alphabet data type \n");
	//pos = rand()%(strlen(data_string));               
      //data[i]=data_string[pos];
      //data[i]= taus(OTG) % (126 - 33 + 1) + 33;
    }
    data[size] = '\0';  
    break; 
 case RANDOM_STRING:
   data=(char*)malloc((size+1)*sizeof(char));
   for(i=0;i<=size;i++){
     if (data_type == HEADER_ALPHABET)
       data[i]= (char) uniform_dist(48,57); // take only the numbers 
     else if (data_type == PAYLOAD_ALPHABET) 
       data[i]= (char) uniform_dist(48,126); // take all the ascii chars 
     else
       LOG_E(OTG,"unsupported alphabet data type \n");
   }
   data[size] = '\0';  
   break;
  default:
   LOG_E(OTG,"not supported string generation");
   break;
  }
   return data;
}

unsigned int header_size(int hdr_size){

  int size = hdr_size;
  if (hdr_size>(sizeof(otg_hdr_info_t) + sizeof(otg_hdr_t)))
    size-=(sizeof(otg_hdr_info_t) + sizeof(otg_hdr_t));
  else
    LOG_W(OTG,"OTG Header not included inside packet header (OTG header:%d, Header%d)\n", hdr_size, sizeof(otg_hdr_info_t) + sizeof(otg_hdr_t) );
 
return(size);

}


unsigned char * serialize_buffer(char* header, char* payload, unsigned int buffer_size, int traffic_type, int flag, int flow_id, int ctime, int seq_num, int hdr_type, int state, unsigned int aggregation_level){
 
  unsigned char *tx_buffer=NULL;
  otg_hdr_info_t *otg_hdr_info_p=NULL;
  otg_hdr_t *otg_hdr_p=NULL;
  unsigned int  byte_tx_count=0; 
  
  if (header == NULL || payload == NULL)
    return NULL;
  // allocate memory for the tx_buffer
  tx_buffer= (char*)malloc(buffer_size);
  // add otg control information first for decoding and computing the statistics 
  otg_hdr_info_p = (otg_hdr_info_t *) (&tx_buffer[byte_tx_count]);
  otg_hdr_info_p->size = buffer_size;
  otg_hdr_info_p->flag = flag; // (background_ok==0)? 0xffff : 0xbbbb;
  byte_tx_count = sizeof(otg_hdr_info_t);
  otg_hdr_p = (otg_hdr_t *) (&tx_buffer[byte_tx_count]);
  otg_hdr_p->flow_id =flow_id;
  otg_hdr_p->time =ctime;
  otg_hdr_p->seq_num =seq_num;
  otg_hdr_p->hdr_type=hdr_type;
  otg_hdr_p->state = state;
  otg_hdr_p->aggregation_level=aggregation_level;
	otg_hdr_p->traffic_type=traffic_type;
  byte_tx_count += sizeof(otg_hdr_t);
  
  // copy the header first 
  memcpy(&tx_buffer[byte_tx_count], header, strlen(header));
  byte_tx_count += strlen(header);
  free(header); 
  // now append the payload 
  memcpy(&tx_buffer[byte_tx_count], payload, strlen(payload));
  byte_tx_count +=strlen(payload);
  free(payload);
  
  return tx_buffer;
}


void init_predef_traffic(unsigned char nb_ue_local, unsigned char nb_enb_local) {
int i;
int j;
int k;

//LOG_I(OTG,"OTG_CONFIG num_node %d\n",  g_otg->num_nodes);
//LOG_I(OTG,"MAX_NODES [MAX UE= %d] [MAX eNB= %d] \n",nb_ue_local,  nb_enb_local);

 for (i=0; i<g_otg->num_nodes; i++){ // src 
   for (j=0; j<g_otg->num_nodes; j++){ // dst
     for (k=0; k<g_otg->application_idx[i][j]; k++){  

     switch  (g_otg->application_type[i][j][k]) {
     case  SCBR : 
       g_otg->trans_proto[i][j][k] = 1;
       g_otg->ip_v[i][j][k] = 1;
       g_otg->idt_dist[i][j][k][PE_STATE] = FIXED;
       g_otg->idt_min[i][j][k][PE_STATE] =  100; 
       g_otg->idt_max[i][j][k][PE_STATE] =  10;
       g_otg->size_dist[i][j][k][PE_STATE] = FIXED;
       g_otg->size_min[i][j][k][PE_STATE] =  50;
       g_otg->size_max[i][j][k][PE_STATE] =  50;

       LOG_I(OTG,"OTG_CONFIG SCBR, src = %d, dst = %d, traffic id %d, dist type for size = %d\n", i, j, k, g_otg->size_dist[i][j][k][PE_STATE]);
				
#ifdef STANDALONE
       g_otg->dst_port[i][j] = 302;
       g_otg->duration[i][j] = 1000;
#endif 
       break;
     case MCBR :
       g_otg->trans_proto[i][j][k] = 1;
       g_otg->ip_v[i][j][k] = 1;
       g_otg->idt_dist[i][j][k][PE_STATE] = FIXED;
       g_otg->idt_min[i][j][k][PE_STATE] =  200;
       g_otg->idt_max[i][j][k][PE_STATE] =  10;
       g_otg->size_dist[i][j][k][PE_STATE] = FIXED;
       g_otg->size_min[i][j][k][PE_STATE] =  512;
       g_otg->size_max[i][j][k][PE_STATE] =  512;

       LOG_I(OTG,"OTG_CONFIG MCBR, src = %d, dst = %d,  traffic id %d, dist type for size = %d\n", i, j,k , g_otg->size_dist[i][j][k][PE_STATE]);
#ifdef STANDALONE
       g_otg->dst_port[i][j] = 302;
       g_otg->duration[i][j] = 1000;
#endif 
       break;
     case BCBR :
       g_otg->trans_proto[i][j][k] = 1;
       g_otg->ip_v[i][j][k] = 1;
       g_otg->idt_dist[i][j][k][PE_STATE] = FIXED;// main param in this mode
       g_otg->idt_min[i][j][k][PE_STATE] =  100; // main param in this mode
       g_otg->idt_max[i][j][k][PE_STATE] =  10;
       g_otg->size_dist[i][j][k][PE_STATE] = FIXED; // main param in this mode
       g_otg->size_min[i][j][k][PE_STATE] =  1024;// main param in this mode
       g_otg->size_max[i][j][k][PE_STATE] =  1024;

       LOG_I(OTG,"OTG_CONFIG BCBR, src = %d, dst = %d, dist type for size = %d\n", i, j, g_otg->size_dist[i][j][k][PE_STATE]);
#ifdef STANDALONE
       g_otg->dst_port[i][j] = 302;
       g_otg->duration[i][j] = 1000;
#endif  
       break;
     case AUTO_PILOT : 
       g_otg->trans_proto[i][j][k] = 2;
       g_otg->ip_v[i][j][k] = 1;
       g_otg->idt_dist[i][j][k][PE_STATE] = UNIFORM;
       LOG_I(OTG,"OTG_CONFIG M2M_AP, src = %d, dst = %d, dist IDT = %d\n", i, j, g_otg->idt_dist[i][j][k][PE_STATE]);
       g_otg->idt_min[i][j][k][PE_STATE] =  100;
       g_otg->idt_max[i][j][k][PE_STATE] =  500;
       g_otg->idt_std_dev[i][j][k][PE_STATE] = 0;
       g_otg->idt_lambda[i][j][k][PE_STATE] = 0;
       g_otg->size_dist[i][j][k][PE_STATE] = FIXED;
       g_otg->size_min[i][j][k][PE_STATE] =  800;
       g_otg->size_max[i][j][k][PE_STATE] =  800; 
       g_otg->size_std_dev[i][j][k][PE_STATE] = 0;
       g_otg->size_lambda[i][j][k][PE_STATE] = 0;

#ifdef STANDALONE
       g_otg->dst_port[i][j] = 302;
       g_otg->duration[i][j] = 1000;
#endif 
       break;
     case BICYCLE_RACE :  
       g_otg->trans_proto[i][j][k] = 2;
       g_otg->ip_v[i][j][k] = 1;
       g_otg->idt_dist[i][j][k][PE_STATE] = UNIFORM;
       LOG_I(OTG,"OTG_CONFIG M2M_BR, src = %d, dst = %d, dist IDT = %d\n", i, j, g_otg->idt_dist[i][j][k][PE_STATE]);
       g_otg->idt_min[i][j][k][PE_STATE] =  100;
       g_otg->idt_max[i][j][k][PE_STATE] =  500;
       g_otg->idt_std_dev[i][j][k][PE_STATE] = 0;
       g_otg->idt_lambda[i][j][k][PE_STATE] = 0;
       g_otg->size_dist[i][j][k][PE_STATE] = FIXED;
       g_otg->size_min[i][j][k][PE_STATE] =  800;
       g_otg->size_max[i][j][k][PE_STATE] =  800;
       g_otg->size_std_dev[i][j][k][PE_STATE] = 0;
       g_otg->size_lambda[i][j][k][PE_STATE] = 0;

#ifdef STANDALONE
       g_otg->dst_port[i][j] = 302;
       g_otg->duration[i][j] = 1000;
#endif 
       break;
     case OPENARENA : 
       g_otg->trans_proto[i][j][k] = 2;
       g_otg->ip_v[i][j][k] = 1;
       g_otg->idt_dist[i][j][k][PE_STATE] = UNIFORM;
       LOG_I(OTG,"OTG_CONFIG GAMING_OA, src = %d, dst = %d, dist IDT = %d\n", i, j, g_otg->idt_dist[i][j][k][PE_STATE]);
       g_otg->idt_min[i][j][k][PE_STATE] =  69;
       g_otg->idt_max[i][j][k][PE_STATE] =  103;
       g_otg->idt_std_dev[i][j][k][PE_STATE] = 0;
       g_otg->idt_lambda[i][j][k][PE_STATE] = 0;
       g_otg->size_dist[i][j][k][PE_STATE] = GAUSSIAN;
       g_otg->size_min[i][j][k][PE_STATE] =  5;
       g_otg->size_max[i][j][k][PE_STATE] =  43;
       g_otg->size_std_dev[i][j][k][PE_STATE] = 5;
       g_otg->size_lambda[i][j][k][PE_STATE] = 0;

#ifdef STANDALONE
       g_otg->dst_port[i][j] = 302;
       g_otg->duration[i][j] = 1000;
#endif 
       break;  
     case TEAM_FORTRESS : 
       g_otg->trans_proto[i][j][k] = 2;
       g_otg->ip_v[i][j][k] = 1;
       g_otg->idt_dist[i][j][k][PE_STATE] = UNIFORM;
       LOG_I(OTG,"OTG_CONFIG GAMING_TF, src = %d, dst = %d, dist IDT = %d\n", i, j, g_otg->idt_dist[i][j][k][PE_STATE]);
       g_otg->idt_min[i][j][k][PE_STATE] =  31;
       g_otg->idt_max[i][j][k][PE_STATE] =  42;
       g_otg->idt_std_dev[i][j][k][PE_STATE] = 0;
       g_otg->idt_lambda[i][j][k][PE_STATE] = 0;
       g_otg->size_dist[i][j][k][PE_STATE] = GAUSSIAN;
       g_otg->size_min[i][j][k][PE_STATE] =  5;
       g_otg->size_max[i][j][k][PE_STATE] =  43;
       g_otg->size_std_dev[i][j][k][PE_STATE] = 5;
       g_otg->size_lambda[i][j][k][PE_STATE] = 0;

#ifdef STANDALONE
       g_otg->dst_port[i][j] = 302;
       g_otg->duration[i][j] = 1000;
#endif 
       break;
     case NO_PREDEFINED_TRAFFIC : 
       LOG_I(OTG, "[SRC %d][DST %d] No predefined Traffic \n", i, j);
       g_otg->trans_proto[i][j][k] = 0;
       g_otg->ip_v[i][j][k] = 0;
       g_otg->idt_dist[i][j][k][PE_STATE] = 0;
       g_otg->idt_min[i][j][k][PE_STATE] =  0; 
       g_otg->idt_max[i][j][k][PE_STATE] =  0;
       g_otg->size_dist[i][j][k][PE_STATE] = FIXED;
       g_otg->size_min[i][j][k][PE_STATE] =  0;
       g_otg->size_max[i][j][k][PE_STATE] = 0;
#ifdef STANDALONE
       g_otg->dst_port[i][j] = 302;
       g_otg->duration[i][j] = 1000;
#endif 
       break;
   case M2M_TRAFFIC : 
       LOG_I(OTG," M2M_TRAFFIC, src = %d, dst = %d \n", i, j, g_otg->application_type[i][j][k]);
       g_otg->trans_proto[i][j][k] = 2;
       g_otg->ip_v[i][j][k] = 1;
			 g_otg->pu_size_pkts[i][j][k]=50;
			 g_otg->ed_size_pkts[i][j][k]=60;

       g_otg->idt_dist[i][j][k][PE_STATE] = UNIFORM;
       g_otg->idt_min[i][j][k][PE_STATE] =  1000;
       g_otg->idt_max[i][j][k][PE_STATE] =  15;
       g_otg->size_dist[i][j][k][PE_STATE] = UNIFORM;
       g_otg->size_min[i][j][k][PE_STATE] =  200;
       g_otg->size_max[i][j][k][PE_STATE] =  800;

       g_otg->prob_off_pu[i][j][k]=0.2;
       g_otg->prob_off_ed[i][j][k]=0.3;
       g_otg->prob_off_pe[i][j][k]=0.4;
       g_otg->prob_pu_ed[i][j][k]=0.1;
       g_otg->prob_pu_pe[i][j][k]=0.6;
       g_otg->prob_ed_pu[i][j][k]=0.3;
       g_otg->prob_ed_pe[i][j][k]=0.5;
       
       g_otg->holding_time_off_pu[i][j][k]=10;
       g_otg->holding_time_off_ed[i][j][k]=12; 
       g_otg->holding_time_off_pe[i][j][k]=13; 
       g_otg->holding_time_pe_off[i][j][k]=30;
			 g_otg->m2m[i][j][k]=1;
#ifdef STANDALONE
       g_otg->dst_port[i][j] = 302;
       g_otg->duration[i][j] = 1000;
#endif 
case AUTO_PILOT_L : 
       LOG_I(OTG,"AUTO PILOT LOW SPEEDS, src = %d, dst = %d, application type = %d\n", i, j, g_otg->application_type[i][j][k]);
			 /* DL SCENARIO*/
       g_otg->trans_proto[i][j][k] = 2;
       g_otg->ip_v[i][j][k] = 1;
			 g_otg->pu_size_pkts[i][j][k]=1000;
			 g_otg->ed_size_pkts[i][j][k]=1000;
       g_otg->idt_dist[i][j][k][PE_STATE] = FIXED;
       g_otg->idt_min[i][j][k][PE_STATE] =  100;
       g_otg->idt_max[i][j][k][PE_STATE] =  100;
       g_otg->size_dist[i][j][k][PE_STATE] = FIXED;
       g_otg->size_min[i][j][k][PE_STATE] =  1000;
       g_otg->size_max[i][j][k][PE_STATE] =  1000;
       g_otg->prob_off_pu[i][j][k]=0.2;
       g_otg->prob_off_ed[i][j][k]=0.3;
       g_otg->prob_off_pe[i][j][k]=0;
       g_otg->prob_pu_ed[i][j][k]=0.5;
       g_otg->prob_pu_pe[i][j][k]=0;
       g_otg->prob_ed_pu[i][j][k]=0;
       g_otg->prob_ed_pe[i][j][k]=1;
       g_otg->holding_time_off_pu[i][j][k]=100;
       g_otg->holding_time_off_ed[i][j][k]=10; 
       g_otg->holding_time_pe_off[i][j][k]=1000;
			 g_otg->m2m[i][j][k]=1;
#ifdef STANDALONE
       g_otg->dst_port[i][j]= 303;
       g_otg->duration[i][j] = 1000;
#endif 
			 /* UL SCENARIO*/
       g_otg->trans_proto[j][i][k] = 2;
       g_otg->ip_v[j][i][k] = 1;
			 g_otg->pu_size_pkts[j][i][k]=1000;
			 g_otg->ed_size_pkts[j][i][k]=1000;
       g_otg->idt_dist[j][i][k][PE_STATE] = FIXED;
       g_otg->idt_min[j][i][k][PE_STATE] =  100;
       g_otg->idt_max[j][i][k][PE_STATE] =  100;
       g_otg->size_dist[j][i][k][PE_STATE] = UNIFORM;
       g_otg->size_min[j][i][k][PE_STATE] =  64;
       g_otg->size_max[j][i][k][PE_STATE] =  1000;
       g_otg->prob_off_pu[j][i][k]=0.2;
       g_otg->prob_off_ed[j][i][k]=0.3;
       g_otg->prob_off_pe[j][i][k]=0;
       g_otg->prob_pu_ed[j][i][k]=0.5;
       g_otg->prob_pu_pe[j][i][k]=0;
       g_otg->prob_ed_pu[j][i][k]=0;
       g_otg->prob_ed_pe[j][i][k]=1;
       g_otg->holding_time_off_pu[j][i][k]=100;
       g_otg->holding_time_off_ed[j][i][k]=10;  
       g_otg->holding_time_pe_off[j][i][k]=1000;
			 g_otg->m2m[j][i][k]=1;
#ifdef STANDALONE
       g_otg->dst_port[j][i] = 302;[k]
       g_otg->duration[j][i] = 1000;
#endif 
 			break; 
case AUTO_PILOT_M : 
       LOG_I(OTG,"AUTO PILOT MEDIUM SPEEDS, src = %d, dst = %d, application type = %d\n", i, j, g_otg->application_type[i][j][k]);
			 /* DL SCENARIO*/
       g_otg->trans_proto[i][j][k] = 2;
       g_otg->ip_v[i][j][k] = 1;
			 g_otg->pu_size_pkts[i][j][k]=1000;
			 g_otg->ed_size_pkts[i][j][k]=1000;
       g_otg->idt_dist[i][j][k][PE_STATE] = FIXED;
       g_otg->idt_min[i][j][k][PE_STATE] =  100;
       g_otg->idt_max[i][j][k][PE_STATE] =  100;
       g_otg->size_dist[i][j][k][PE_STATE] = FIXED;
       g_otg->size_min[i][j][k][PE_STATE] =  1000;
       g_otg->size_max[i][j][k][PE_STATE] =  1000;
       g_otg->prob_off_pu[i][j][k]=0.2;
       g_otg->prob_off_ed[i][j][k]=0.3;
       g_otg->prob_off_pe[i][j][k]=0;
       g_otg->prob_pu_ed[i][j][k]=0.5;
       g_otg->prob_pu_pe[i][j][k]=0;
       g_otg->prob_ed_pu[i][j][k]=0;
       g_otg->prob_ed_pe[i][j][k]=1;
       g_otg->holding_time_off_pu[i][j][k]=40;
       g_otg->holding_time_off_ed[i][j][k]=10; 
       g_otg->holding_time_pe_off[i][j][k]=1000;
			 g_otg->m2m[i][j][k]=1;
#ifdef STANDALONE
       g_otg->dst_port[i][j] = 303;
       g_otg->duration[i][j] = 1000;
#endif 
			 /* UL SCENARIO*/
       g_otg->trans_proto[j][i][k] = 2;
       g_otg->ip_v[j][i][k] = 1;
			 g_otg->pu_size_pkts[j][i][k]=1000;
			 g_otg->ed_size_pkts[j][i][k]=1000;
       g_otg->idt_dist[j][i][k][PE_STATE] = FIXED;
       g_otg->idt_min[j][i][k][PE_STATE] =  100;
       g_otg->idt_max[j][i][k][PE_STATE] =  100;
       g_otg->size_dist[j][i][k][PE_STATE] = UNIFORM;
       g_otg->size_min[j][i][k][PE_STATE] =  64;
       g_otg->size_max[j][i][k][PE_STATE] =  1000;
       g_otg->prob_off_pu[j][i][k]=0.2;
       g_otg->prob_off_ed[j][i][k]=0.3;
       g_otg->prob_off_pe[j][i][k]=0;
       g_otg->prob_pu_ed[j][i][k]=0.5;
       g_otg->prob_pu_pe[j][i][k]=0;
       g_otg->prob_ed_pu[j][i][k]=0;
       g_otg->prob_ed_pe[j][i][k]=1;
       g_otg->holding_time_off_pu[j][i][k]=40;
       g_otg->holding_time_off_ed[j][i][k]=10;  
       g_otg->holding_time_pe_off[j][i][k]=1000;
			 g_otg->m2m[j][i][k]=1;
#ifdef STANDALONE
       g_otg->dst_port[j][i] = 302;
       g_otg->duration[j][i] = 1000;
#endif 
 			break; 
case AUTO_PILOT_H : 
       LOG_I(OTG,"AUTO PILOT HIGH SPEEDS, src = %d, dst = %d, application type = %d\n", i, j, g_otg->application_type[i][j][k]);
			 /* DL SCENARIO*/
       g_otg->trans_proto[i][j][k] = 2;
       g_otg->ip_v[i][j][k] = 1;
			 g_otg->pu_size_pkts[i][j][k]=1000;
			 g_otg->ed_size_pkts[i][j][k]=1000;
       g_otg->idt_dist[i][j][k][PE_STATE] = UNIFORM;
       g_otg->idt_min[i][j][k][PE_STATE] =  20;
       g_otg->idt_max[i][j][k][PE_STATE] =  25;
       g_otg->size_dist[i][j][k][PE_STATE] = FIXED;
       g_otg->size_min[i][j][k][PE_STATE] =  1000;
       g_otg->size_max[i][j][k][PE_STATE] =  1000;
       g_otg->prob_off_pu[i][j][k]=0.2;
       g_otg->prob_off_ed[i][j][k]=0.3;
       g_otg->prob_off_pe[i][j][k]=0;
       g_otg->prob_pu_ed[i][j][k]=0.5;
       g_otg->prob_pu_pe[i][j][k]=0;
       g_otg->prob_ed_pu[i][j][k]=0;
       g_otg->prob_ed_pe[i][j][k]=1;
       g_otg->holding_time_off_pu[i][j][k]=20;
       g_otg->holding_time_off_ed[i][j][k]=10; 
       g_otg->holding_time_pe_off[i][j][k]=1000;
			 g_otg->m2m[i][j][k]=1;
#ifdef STANDALONE
       g_otg->dst_port[i][j] = 303;
       g_otg->duration[i][j] = 1000;
#endif 
			 /* UL SCENARIO*/
       g_otg->trans_proto[j][i][k] = 2;
       g_otg->ip_v[j][i][k] = 1;
			 g_otg->pu_size_pkts[j][i][k] =1000;
			 g_otg->ed_size_pkts[j][i][k] =1000;
       g_otg->idt_dist[j][i][k][PE_STATE] = UNIFORM;
       g_otg->idt_min[j][i][k][PE_STATE] =  20;
       g_otg->idt_max[j][i][k][PE_STATE] =  25;
       g_otg->size_dist[j][i][k][PE_STATE] = UNIFORM;
       g_otg->size_min[j][i][k][PE_STATE] =  64;
       g_otg->size_max[j][i][k][PE_STATE] =  1000;
       g_otg->prob_off_pu[j][i][k]=0.2;
       g_otg->prob_off_ed[j][i][k]=0.3;
       g_otg->prob_off_pe[j][i][k]=0;
       g_otg->prob_pu_ed[j][i][k]=0.5;
       g_otg->prob_pu_pe[j][i][k]=0;
       g_otg->prob_ed_pu[j][i][k]=0;
       g_otg->prob_ed_pe[j][i][k]=1;
       g_otg->holding_time_off_pu[j][i][k]=20;
       g_otg->holding_time_off_ed[j][i][k]=10;  
       g_otg->holding_time_pe_off[j][i][k]=1000;
			 g_otg->m2m[i][j][k]=1;
#ifdef STANDALONE
       g_otg->dst_port[j][i] = 302;
       g_otg->duration[j][i] = 1000;
#endif 
 			break; 
case AUTO_PILOT_E : 
       LOG_I(OTG,"AUTO PILOT EMERGENCY, src = %d, dst = %d, application type = %d\n", i, j, g_otg->application_type[i][j][k]);
			 /* DL SCENARIO*/
       g_otg->trans_proto[i][j][k] = 2;
       g_otg->ip_v[i][j][k] = 1;
			 g_otg->pu_size_pkts[i][j][k]=1000;
			 g_otg->ed_size_pkts[i][j][k]=1000;
       g_otg->idt_dist[i][j][k][PE_STATE] = UNIFORM;
       g_otg->idt_min[i][j][k][PE_STATE] =  20;
       g_otg->idt_max[i][j][k][PE_STATE] =  25;
       g_otg->size_dist[i][j][k][PE_STATE] = FIXED;
       g_otg->size_min[i][j][k][PE_STATE] =  1000;
       g_otg->size_max[i][j][k][PE_STATE] =  1000;
       g_otg->prob_off_pu[i][j][k]=0.2;
       g_otg->prob_off_ed[i][j][k]=0.3;
       g_otg->prob_off_pe[i][j][k]=0;
       g_otg->prob_pu_ed[i][j][k]=0.5;
       g_otg->prob_pu_pe[i][j][k]=0;
       g_otg->prob_ed_pu[i][j][k]=0;
       g_otg->prob_ed_pe[i][j][k]=1;
       g_otg->holding_time_off_pu[i][j][k]=10;
       g_otg->holding_time_off_ed[i][j][k]=10; 
       g_otg->holding_time_pe_off[i][j][k]=1000;
			 g_otg->m2m[i][j][k]=1;
#ifdef STANDALONE
       g_otg->dst_port[i][j] = 303;
       g_otg->duration[i][j] = 1000;
#endif 
			 /* UL SCENARIO*/
       g_otg->trans_proto[j][i][k] = 2;
       g_otg->ip_v[j][i][k] = 1;
			 g_otg->pu_size_pkts[j][i][k]=1000;
			 g_otg->ed_size_pkts[j][i][k]=1000;
       g_otg->idt_dist[j][i][k][PE_STATE] = UNIFORM;
       g_otg->idt_min[j][i][k][PE_STATE] =  20;
       g_otg->idt_max[j][i][k][PE_STATE] =  25;
       g_otg->size_dist[j][i][k][PE_STATE] = UNIFORM;
       g_otg->size_min[j][i][k][PE_STATE] =  64;
       g_otg->size_max[j][i][k][PE_STATE] =  1000;
       g_otg->prob_off_pu[j][i][k]=0.2;
       g_otg->prob_off_ed[j][i][k]=0.3;
       g_otg->prob_off_pe[j][i][k]=0;
       g_otg->prob_pu_ed[j][i][k]=0.5;
       g_otg->prob_pu_pe[j][i][k]=0;
       g_otg->prob_ed_pu[j][i][k]=0;
       g_otg->prob_ed_pe[j][i][k]=1;
       g_otg->holding_time_off_pu[j][i][k]=10;
       g_otg->holding_time_off_ed[j][i][k]=10;  
       g_otg->holding_time_pe_off[j][i][k]=1000;
			 g_otg->m2m[i][j][k]=1;
#ifdef STANDALONE
       g_otg->dst_port[j][i] = 302;
       g_otg->duration[j][i] = 1000;
#endif 
 			break; 
case VIRTUAL_GAME_L : 
       LOG_I(OTG,"VIRTUAL GAME LOW SPEEDS, src = %d, dst = %d, application type = %d\n", i, j, g_otg->application_type[i][j][k]);
			 /* DL SCENARIO*/
       g_otg->trans_proto[i][j][k] = 2;
       g_otg->ip_v[i][j][k] = 1;
			 g_otg->pu_size_pkts[i][j][k]=1000;
       g_otg->prob_off_pu[i][j][k]=1;
       g_otg->holding_time_off_pu[i][j][k]=1000;
			 g_otg->m2m[i][j][k]=1;
#ifdef STANDALONE
       g_otg->dst_port[i][j][k] = 302;
       g_otg->duration[i][j][k] = 1000;
#endif 
			 /* UL SCENARIO*/
       g_otg->trans_proto[j][i][k] = 2;
       g_otg->ip_v[j][i][k] = 1;
			 g_otg->pu_size_pkts[j][i][k]=1000;
       g_otg->prob_off_pu[j][i][k]=1;
       g_otg->holding_time_off_pu[j][i][k]=500;
			 g_otg->m2m[j][i][k]=1;
#ifdef STANDALONE
       g_otg->dst_port[j][i] = 303;
       g_otg->duration[j][i] = 1000;
#endif 
 			break; 
case VIRTUAL_GAME_M : 
       LOG_I(OTG,"VIRTUAL GAME MEDIUM SPEEDS, src = %d, dst = %d, application type = %d\n", i, j, g_otg->application_type[i][j][k]);
			 /* DL SCENARIO*/
       g_otg->trans_proto[i][j][k] = 2;
       g_otg->ip_v[i][j][k] = 1;
			 g_otg->pu_size_pkts[i][j][k]=1000;
       g_otg->prob_off_pu[i][j][k]=1;
       g_otg->holding_time_off_pu[i][j][k]=1000;
			 g_otg->m2m[i][j][k]=1;
#ifdef STANDALONE
       g_otg->dst_port[i][j] = 302;
       g_otg->duration[i][j] = 1000;
#endif 
			 /* UL SCENARIO*/
       g_otg->trans_proto[j][i][k] = 2;
       g_otg->ip_v[j][i][k] = 1;
			 g_otg->pu_size_pkts[j][i][k]=1000;
       g_otg->prob_off_pu[j][i][k]=1;
       g_otg->holding_time_off_pu[j][i][k]=150;
			 g_otg->m2m[j][i][k]=1;
#ifdef STANDALONE
       g_otg->dst_port[j][i] = 303;
       g_otg->duration[j][i] = 1000;
#endif 
 			break; 
case VIRTUAL_GAME_H : 
       LOG_I(OTG,"VIRTUAL GAME HIGH SPEEDS, src = %d, dst = %d, application type = %d\n", i, j, g_otg->application_type[i][j][k]);
			 /* DL SCENARIO*/
       g_otg->trans_proto[i][j][k] = 2;
       g_otg->ip_v[i][j][k] = 1;
			 g_otg->pu_size_pkts[i][j][k]=1000;
       g_otg->prob_off_pu[i][j][k]=1;
       g_otg->holding_time_off_pu[i][j][k]=1000;
			 g_otg->m2m[i][j][k]=1;
#ifdef STANDALONE
       g_otg->dst_port[i][j] = 302;
       g_otg->duration[i][j] = 1000;
#endif 
			 /* UL SCENARIO*/
       g_otg->trans_proto[j][i][k] = 2;
       g_otg->ip_v[j][i][k] = 1;
			 g_otg->pu_size_pkts[j][i][k]=1000;
       g_otg->prob_off_pu[j][i][k]=1;
       g_otg->holding_time_off_pu[j][i][k]=100;
			 g_otg->m2m[j][i][k]=1;
#ifdef STANDALONE
       g_otg->dst_port[j][i] = 303;
       g_otg->duration[j][i] = 1000;
#endif 
 			break; 
case VIRTUAL_GAME_F : 
       LOG_I(OTG,"VIRTUAL GAME FINISH, src = %d, dst = %d, application type = %d\n", i, j, g_otg->application_type[i][j][k]);
			 /* DL SCENARIO*/
       g_otg->trans_proto[i][j][k] = 2;
       g_otg->ip_v[i][j][k] = 1;
			 g_otg->pu_size_pkts[i][j][k]=1000;
       g_otg->prob_off_pu[i][j][k]=1;
       g_otg->holding_time_off_pu[i][j][k]=1000;
			 g_otg->m2m[i][j][k]=1;
#ifdef STANDALONE
       g_otg->dst_port[i][j] = 302;
       g_otg->duration[i][j] = 1000;
#endif 
			 /* UL SCENARIO*/
       g_otg->trans_proto[j][i][k] = 2;
       g_otg->ip_v[j][i][k]= 1;
			 g_otg->pu_size_pkts[j][i][k]=1000;
       g_otg->prob_off_pu[j][i][k]=1;
       g_otg->holding_time_off_pu[j][i][k]=70;
			 g_otg->m2m[j][i][k]=1;
#ifdef STANDALONE
       g_otg->dst_port[j][i] = 303;
       g_otg->duration[j][i] = 1000;
#endif 
 			break; 
case ALARM_HUMIDITY : 
       LOG_I(OTG,"ALARM HUMIDITY, src = %d, dst = %d, application type = %d\n", i, j, g_otg->application_type[i][j][k]);
			 /* DU SCENARIO*/
       g_otg->trans_proto[i][j][k] = 2;
       g_otg->ip_v[i][j][k] = 1;
			 g_otg->pu_size_pkts[i][j][k]=1000;
			 g_otg->ed_size_pkts[i][j][k]=2000;
       g_otg->prob_off_pu[i][j][k]=0.5;
       g_otg->prob_off_ed[i][j][k]=0.5;
       g_otg->prob_pu_ed[i][j][k]=0.5;
       g_otg->holding_time_off_pu[i][j][k]=1680000;		/* 28 minutes*/
       g_otg->holding_time_off_ed[i][j][k]=32400000;  	/* 9 hours*/
			 g_otg->m2m[i][j][k]=1;
#ifdef STANDALONE
       g_otg->dst_port[i][j] = 303;
       g_otg->duration[i][j] = 1000;
#endif 
 			break; 
case ALARM_SMOKE : 
       LOG_I(OTG,"ALARM SMOKE, src = %d, dst = %d, application type = %d\n", i, j, g_otg->application_type[i][j][k]);
			 /* UL SCENARIO*/
       g_otg->trans_proto[i][j][k] = 2;
       g_otg->ip_v[i][j][k] = 1;
			 g_otg->pu_size_pkts[i][j][k]=1000;
			 g_otg->ed_size_pkts[i][j][k]=2000;
       g_otg->prob_off_pu[i][j][k]=0.5;
       g_otg->prob_off_ed[i][j][k]=0.5;
       g_otg->prob_pu_ed[i][j][k]=0.5;
       g_otg->holding_time_off_pu[i][j][k]=60000;		/* 1 minute*/
       g_otg->holding_time_off_ed[i][j][k]=43200000;  	/* 12 hours*/
			 g_otg->m2m[i][j][k]=1;
#ifdef STANDALONE
       g_otg->dst_port[i][j] = 303;
       g_otg->duration[i][j] = 1000;
#endif 
 			break; 
case ALARM_TEMPERATURE : 
       LOG_I(OTG,"ALARM TEMPERATURE, src = %d, dst = %d, application type = %d\n", i, j, g_otg->application_type[i][j][k]);
			 /* UL SCENARIO*/
       g_otg->trans_proto[i][j][k] = 2;
       g_otg->ip_v[i][j][k] = 1;
			 g_otg->pu_size_pkts[i][j][k]=1000;
			 g_otg->ed_size_pkts[i][j][k]=4000;
       g_otg->prob_off_pu[i][j][k]=0.5;
       g_otg->prob_off_ed[i][j][k]=0.5;
       g_otg->prob_pu_ed[i][j][k]=0.5;
       g_otg->holding_time_off_pu[i][j][k]=1680000;		/* 28 minute*/
       g_otg->holding_time_off_ed[i][j][k]=18000000;  	/* 5 hours*/
			 g_otg->m2m[i][j][k]=1;
#ifdef STANDALONE
       g_otg->dst_port[i][j]= 303;
       g_otg->duration[i][j] = 1000;
#endif 
   case OPENARENA_UL : 
       g_otg->trans_proto[i][j][k] = 2;
       g_otg->ip_v[i][j][k] = 1;
       g_otg->idt_dist[i][j][k][PE_STATE] = FIXED;
       LOG_I(OTG,"OTG_CONFIG GAMING_OA_UL, src = %d, dst = %d, dist IDT = %d\n", i, j, g_otg->idt_dist[i][j][k][PE_STATE]);
       g_otg->idt_min[i][j][k][PE_STATE] =  11;
       g_otg->idt_max[i][j][k][PE_STATE] =  11;
       g_otg->size_dist[i][j][k][PE_STATE] = FIXED;
       g_otg->size_min[i][j][k][PE_STATE] =  42;
       g_otg->size_max[i][j][k][PE_STATE] =  42;
			 g_otg->m2m[i][j][k]=1;
#ifdef STANDALONE
       g_otg->dst_port[i][j] = 302;
       g_otg->duration[i][j] = 1000;
#endif 

   case OPENARENA_DL : 
       g_otg->trans_proto[i][j][k] = 2;
       g_otg->ip_v[i][j][k] = 1;
       g_otg->idt_dist[i][j][k][PE_STATE] = FIXED;
       LOG_I(OTG,"OTG_CONFIG GAMING_OA_UL, src = %d, dst = %d, dist IDT = %d\n", i, j, g_otg->idt_dist[i][j][k][PE_STATE]);
       g_otg->idt_min[i][j][k][PE_STATE] =  40;
       g_otg->idt_max[i][j][k][PE_STATE] =  40;
       g_otg->size_dist[i][j][k][PE_STATE] = FIXED;
       g_otg->size_min[i][j][k][PE_STATE] =  140;
       g_otg->size_max[i][j][k][PE_STATE] =  140;
			 g_otg->m2m[i][j][k]=1;

#ifdef STANDALONE
       g_otg->dst_port[i][j] = 302;
       g_otg->duration[i][j] = 1000;
#endif
 			break; 
     case VOIP_G711 :  /*http://www.computerweekly.com/feature/VoIP-bandwidth-fundamentals */
											/* Voice bit rate= 64 Kbps |	Sample time= 20 msec |	Voice payload=160 bytes */
       g_otg->trans_proto[i][j][k] = 1;
       g_otg->ip_v[i][j][k] = 1;
       g_otg->idt_dist[i][j][k][SIMPLE_TALK] = FIXED;
       LOG_I(OTG,"OTG_CONFIG VOIP G711, src = %d, dst = %d, dist IDT = %d\n", i, j, g_otg->idt_dist[i][j][k][SIMPLE_TALK]);
       g_otg->idt_min[i][j][k][SIMPLE_TALK] =  20;
       g_otg->idt_max[i][j][k][SIMPLE_TALK] =  20;
       g_otg->size_dist[i][j][k][SIMPLE_TALK] = FIXED;
       g_otg->size_min[i][j][k][SIMPLE_TALK] =  160;
       g_otg->size_max[i][j][k][SIMPLE_TALK] =  160;

#ifdef STANDALONE
       g_otg->dst_port[i][j] = 302;
       g_otg->duration[i][j] = 1000;
#endif 
			break;
     case VOIP_G729 :  /*http://www.computerweekly.com/feature/VoIP-bandwidth-fundamentals */  
											/* Voice bit rate= 8 Kbps |	Sample time= 30 msec |	Voice payload=30 bytes */
       g_otg->trans_proto[i][j][k] = 1;
       g_otg->ip_v[i][j][k] = 1;
       g_otg->idt_dist[i][j][k][SIMPLE_TALK] = FIXED;
       LOG_I(OTG,"OTG_CONFIG  VOIP G729, src = %d, dst = %d, dist IDT = %d\n", i, j, g_otg->idt_dist[i][j][k][SIMPLE_TALK]);
       g_otg->idt_min[i][j][k][SIMPLE_TALK] =  30;
       g_otg->idt_max[i][j][k][SIMPLE_TALK] =  30;
       g_otg->size_dist[i][j][k][SIMPLE_TALK] = FIXED;
       g_otg->size_min[i][j][k][SIMPLE_TALK] =  30;
       g_otg->size_max[i][j][k][SIMPLE_TALK] =  30;

#ifdef STANDALONE
       g_otg->dst_port[i][j] = 302;
       g_otg->duration[i][j] = 1000;
#endif 
			break;
     case IQSIM_MANGO : 
       g_otg->trans_proto[i][j][k] = TCP;
       g_otg->ip_v[i][j][k] = IPV4;
       g_otg->idt_dist[i][j][k][PE_STATE] = UNIFORM;
       g_otg->idt_min[i][j][k][PE_STATE] =  5000;
       g_otg->idt_max[i][j][k][PE_STATE] = 10000;
       g_otg->size_dist[i][j][k][PE_STATE] = FIXED;
			 if (i<nb_enb_local){ 
       LOG_I(OTG,"IQSIM_MANGO [DL], src = %d, dst = %d, dist IDT = %d\n", i, j, g_otg->idt_dist[i][j][k][PE_STATE]);
       g_otg->size_min[i][j][k][PE_STATE] =  141;
       g_otg->size_max[i][j][k][PE_STATE] =  141; 
			}
			else {
			LOG_I(OTG,"IQSIM_MANGO [UL], src = %d, dst = %d, dist IDT = %d\n", i, j, g_otg->idt_dist[i][j][k][PE_STATE]);
       g_otg->size_min[i][j][k][PE_STATE] =  144;
       g_otg->size_max[i][j][k][PE_STATE] =  144; 
			}

#ifdef STANDALONE
       g_otg->dst_port[i][j] = 302;
       g_otg->duration[i][j] = 1000;
#endif 

			break;
     case IQSIM_NEWSTEO : 
       g_otg->trans_proto[i][j][k] = TCP;
       g_otg->ip_v[i][j][k] = IPV4;
       g_otg->idt_dist[i][j][k][PE_STATE] = UNIFORM;
       g_otg->idt_min[i][j][k][PE_STATE] =  5000;
       g_otg->idt_max[i][j][k][PE_STATE] = 10000;
       g_otg->size_dist[i][j][k][PE_STATE] = FIXED;
			 if (i<nb_enb_local){ 
       LOG_I(OTG,"IQSIM_NEWSTEO [DL], src = %d, dst = %d, dist IDT = %d\n", i, j, g_otg->idt_dist[i][j][k][PE_STATE]);
       g_otg->size_min[i][j][k][PE_STATE] =  467;
       g_otg->size_max[i][j][k][PE_STATE] =  467; 
			}
			else {
			LOG_I(OTG,"IQSIM_NEWSTEO [UL], src = %d, dst = %d, dist IDT = %d\n", i, j, g_otg->idt_dist[i][j][k][PE_STATE]);
       g_otg->size_min[i][j][k][PE_STATE] =  251;
       g_otg->size_max[i][j][k][PE_STATE] =  251; 
			}

#ifdef STANDALONE
       g_otg->dst_port[i][j] = 302;
       g_otg->duration[i][j] = 1000;
#endif 

			break;
     default:
       LOG_D(OTG, "[SRC %d][DST %d] Unknown traffic type\n", i, j);

     }
	 }
   }
 }
}


int background_gen(int src, int dst, int ctime){ 

	/*check if it is time to transmit the background traffic
	- we have different distributions for packet size and idt for the UL and DL */
  if ((((ctime-otg_info->ptime_background) >=  otg_info->idt_background[src][dst])) ||  
      (otg_info->idt_background[src][dst]==0)){
    LOG_D(OTG,"[SRC %d][DST %d] 	:: OK (idt=%d, ctime=%d,ptime=%d ) !!\n", src, dst, otg_info->idt_background[src][dst], ctime, otg_info->ptime_background);
    /* Distinguish between the UL and DL case*/
    if (src<NB_eNB_INST) // DL case
      otg_info->size_background[src][dst]=ceil(lognormal_dist(5.46,0.85)); /*lognormal distribution for DL background packet*/
    else //UL case
      otg_info->size_background[src][dst]=ceil(lognormal_dist(3.03,0.5)); /*lognormal distribution for DL background packet*/
    
    // adjust the packet size if needed 
    if (otg_info->size_background[src][dst]>1400)
      otg_info->size_background[src][dst]=1400;
    if (otg_info->size_background[src][dst]<=0)
    	otg_info->size_background[src][dst]=10;

   
/* Compute the corresponding IDT*/

	/* Eq. (7) from "Users in Cells: a Data Traffic Analysis (Markus Laner, Philipp Svoboda, Stefan Schwarz, and Markus Rupp)" 
	Measured traffic consists of a mixture of many different types (e.g., web, video streaming, file download), gives an intuition for the encountered heavy-tails
	of. Most sessions are short with low data-volume, consisting of small downloads (e.g., e-mail, TCP-acknowledges), whereas some few sessions last very long and 			require high throughput (e.g., VoIP, video streaming, file-download). */

    otg_info->idt_background[src][dst]=ceil(((otg_info->size_background[src][dst])*8000)/pow(10, lognormal_dist(1.3525, 0.1954)));
		/*if(otg_info->idt_background[src][dst]==0)
			otg_info->idt_background[src][dst]=10;*/
    otg_info->ptime_background=ctime;	

 LOG_D(OTG,"[BACKGROUND] TRAFFIC:: (src=%d, dst=%d) pkts size=%d idt=%d  \n", src, dst, otg_info->size_background[src][dst],otg_info->idt_background[src][dst]);

    return 1;
  }
  else {
    //LOG_D(OTG,"[SRC %d][DST %d] [BACKGROUND] TRAFFIC:: not the time to transmit= (idt=%d, ctime=%d,ptime=%d ) size= %d \n", src, dst, otg_info->idt_background[src][dst], 	ctime, otg_info->ptime_background, otg_info->size_background[src][dst]);
    return 0;
  }
  
}



int header_size_gen_background(int src, int dst){
 int size_header=0;

	if (otg_info->header_size_background[src][dst]==0){

	if(g_otg->trans_proto_background[src][dst]==0)
	  g_otg->trans_proto_background[src][dst]= rand() % (TCP_IPV6 - UDP_IPV4 + 1) + UDP_IPV4;
 	switch (g_otg->trans_proto_background[src][dst]) {
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
		break;
	}
	otg_info->header_size_background[src][dst]=size_header;
}
  LOG_D(OTG," [SRC %d][DST %d]  BACKGROUND TRAFFIC:: size header %d \n", src, dst, otg_info->header_size_background[src][dst]);
	return otg_info->header_size_background[src][dst];
}



void state_management(int src, int dst, int application, int ctime) {


if ((g_otg->holding_time_off_pu[src][dst][application]==0) && (g_otg->holding_time_off_ed[src][dst][application]==0) && (g_otg->holding_time_off_pe[src][dst][application]==0))  
	otg_info->state[src][dst][application]=PE_STATE;
else{
   if (otg_info->state_transition_prob[src][dst][application]==0){
    otg_info->state_transition_prob[src][dst][application]=uniform_dist(0,1);
    otg_info->state[src][dst][application]=OFF_STATE;
    LOG_I(OTG,"[%d][%d][Appli id %d] STATE:: OFF INIT \n", src, dst, application);
    otg_info->start_holding_time_off[src][dst][application]=0/*ctime*/;
  }
//LOG_D(OTG,"[%d][[%d] HOLDING_TIMES OFF_PE: %d, OFF_PU: %d, OFF_ED %d, PE_OFF: %d \n", src, dst, g_otg->holding_time_off_pe[src][dst], g_otg->holding_time_off_pu[src][dst],g_otg->holding_time_off_ed[src][dst], g_otg->holding_time_pe_off[src][dst] );  

 switch (otg_info->state[src][dst][application]){

 case OFF_STATE:
	
if (ctime>otg_info->start_holding_time_off[src][dst][application]){
 otg_info->c_holding_time_off[src][dst][application]= ctime - otg_info->start_holding_time_off[src][dst][application];
    LOG_I(OTG,"[%d][%d][Appli id %d][Agg Level=%d] STATE:: OFF Holding Time %d (%d, %d)\n", src, dst,application , g_otg->aggregation_level[src][dst][application], otg_info->c_holding_time_off[src][dst][application], ctime, otg_info->start_holding_time_off[src][dst][application]);
    }

  if ( ((otg_info->state_transition_prob[src][dst][application]>= 1-(g_otg->prob_off_pu[src][dst][application]+g_otg->prob_off_ed[src][dst][application]+g_otg->prob_off_pe[src][dst][application])) && (otg_info->state_transition_prob[src][dst][application]<1-(g_otg->prob_off_ed[src][dst][application]+g_otg->prob_off_pe[src][dst][application])))  && (otg_info->c_holding_time_off[src][dst][application]>=g_otg->holding_time_off_pu[src][dst][application])){  
		 otg_info->state[src][dst][application]=PU_STATE;
     LOG_I(OTG,"[%d][%d][Appli id %d][Agg Level=%d] NEW STATE:: OFF-->PU  %d  (ctime %d, start %d )\n", src, dst,application, g_otg->aggregation_level[src][dst][application], otg_info->c_holding_time_off[src][dst][application], ctime, otg_info->start_holding_time_off[src][dst][application]);
     otg_info->state_transition_prob[src][dst][application]=uniform_dist(0,1);

  }
  else if ( ((otg_info->state_transition_prob[src][dst][application]>= 1-(g_otg->prob_off_ed[src][dst][application]+g_otg->prob_off_pe[src][dst][application])) && (otg_info->state_transition_prob[src][dst][application]< 1-g_otg->prob_off_pe[src][dst][application])) && (otg_info->c_holding_time_off[src][dst][application]>=g_otg->holding_time_off_ed[src][dst][application])){
     otg_info->state[src][dst][application]=ED_STATE;
     LOG_I(OTG,"[%d][%d][Appli id %d][Agg Level=%d] NEW STATE:: OFF-->ED \n", src, dst,application, g_otg->aggregation_level[src][dst][application]);
     otg_info->state_transition_prob[src][dst][application]=uniform_dist(0,1);
  }

  else if (((otg_info->state_transition_prob[src][dst][application]>=1-g_otg->prob_off_pe[src][dst][application]) && (otg_info->state_transition_prob[src][dst][application]<=1)) && (otg_info->c_holding_time_off[src][dst]>=g_otg->holding_time_off_pe[src][dst])){
     otg_info->state[src][dst][application]=PE_STATE;
     LOG_I(OTG,"[%d][%d][Appli id %d][Agg Level=%d] NEW STATE:: OFF-->PE \n", src, dst,application, g_otg->aggregation_level[src][dst]);
     otg_info->state_transition_prob[src][dst][application]=uniform_dist(0,1);

}
  else{
     otg_info->c_holding_time_off[src][dst][application]= ctime - otg_info->start_holding_time_off[src][dst][application];
     otg_info->state_transition_prob[src][dst][application]=uniform_dist(0,1);
     LOG_I(OTG,"[%d][%d][Appli id %d][Agg Level=%d] STATE:: OFF\n", src, dst,application, g_otg->aggregation_level[src][dst][application]);
  }
  break;

 case PU_STATE:
     if  (otg_info->state_transition_prob[src][dst][application]<=1-(g_otg->prob_pu_ed[src][dst][application]+g_otg->prob_pu_pe[src][dst][application])){
       //otg_info->state[src][dst]=OFF_STATE;
       otg_info->state[src][dst][application]=OFF_STATE;
       otg_info->start_holding_time_off[src][dst][application]=ctime; 
			 otg_info->c_holding_time_off[src][dst][application]=0;
       LOG_I(OTG,"[%d][%d][Appli id %d][Agg Level=%d] NEW STATE:: PU-->OFF \n", src, dst,application, g_otg->aggregation_level[src][dst][application]);
       otg_info->state_transition_prob[src][dst][application]=uniform_dist(0,1);
     }	
     else if  ((otg_info->state_transition_prob[src][dst][application]<=1-g_otg->prob_pu_pe[src][dst][application]) && (otg_info->state_transition_prob[src][dst][application]>1-(g_otg->prob_pu_ed[src][dst][application]+g_otg->prob_pu_pe[src][dst][application]))){
       //otg_info->state[src][dst]=ON_STATE;
       otg_info->state[src][dst][application]=ED_STATE;
	     LOG_I(OTG,"[%d][%d][Appli id %d][Agg Level=%d] NEW STATE:: PU-->ED \n", src, dst,application, g_otg->aggregation_level[src][dst][application]);
       otg_info->state_transition_prob[src][dst][application]=uniform_dist(0,1);
     }
    else /*if (otg_info->state_transition_prob[src][dst]>=g_otg->prob_pu_ed)*/{
       //otg_info->state[src][dst]=ON_STATE;
       otg_info->state[src][dst][application]=PE_STATE;
       otg_info->start_holding_time_pe_off[src][dst][application]=ctime;
       LOG_I(OTG,"[%d][%d][Appli id %d][Agg Level=%d] NEW STATE:: PU-->PE \n", src, dst,application, g_otg->aggregation_level[src][dst][application]);
       otg_info->state_transition_prob[src][dst][application]=uniform_dist(0,1);
     }
  break;
 case ED_STATE:
     if  (otg_info->state_transition_prob[src][dst][application]<1-(g_otg->prob_ed_pu[src][dst][application] + g_otg->prob_ed_pe[src][dst][application])){
       //otg_info->state[src][dst]=OFF_STATE;
       otg_info->state[src][dst][application]=OFF_STATE;
       otg_info->start_holding_time_off[src][dst][application]=ctime; 
			 otg_info->c_holding_time_off[src][dst][application]=0;
       LOG_I(OTG,"[%d][%d][Appli id %d][Agg Level=%d] NEW STATE:: ED-->OFF \n", src, dst,application, g_otg->aggregation_level[src][dst][application]);
       otg_info->state_transition_prob[src][dst][application]=uniform_dist(0,1);
     }
     else if  ((otg_info->state_transition_prob[src][dst][application]>=1-(g_otg->prob_ed_pu[src][dst][application] + g_otg->prob_ed_pe[src][dst][application] )) && (otg_info->state_transition_prob[src][dst][application]<1-g_otg->prob_ed_pe[src][dst][application]))  {
       //otg_info->state[src][dst]=ON_STATE;
       otg_info->state[src][dst][application]=PE_STATE;
       LOG_I(OTG,"[%d][%d][Appli id %d][Agg Level=%d] NEW STATE:: ED-->PU \n", src, dst,application, g_otg->aggregation_level[src][dst][application]);
       otg_info->state_transition_prob[src][dst][application]=uniform_dist(0,1);
     }
     else /*if ((otg_info->state_transition_prob[src][dst]>=1-g_otg->prob_ed_pe)&&(otg_info->state_transition_prob[src][dst]<=1)) */{
       //otg_info->state[src][dst]=ON_STATE;
       otg_info->state[src][dst][application]=PE_STATE;
       otg_info->start_holding_time_pe_off[src][dst][application]=ctime;
       LOG_I(OTG,"[%d][%d][Appli id %d][Agg Level=%d] NEW STATE:: ED-->PE \n", src, dst,application, g_otg->aggregation_level[src][dst][application]);
       otg_info->state_transition_prob[src][dst][application]=uniform_dist(0,1);
     }
  break;

 case PE_STATE:
     if (g_otg->holding_time_pe_off[src][dst][application]<=otg_info->c_holding_time_pe_off[src][dst][application]){
       //otg_info->state[src][dst]=OFF_STATE;
       otg_info->state[src][dst][application]=OFF_STATE;
       LOG_I(OTG,"[%d][%d][Appli id %d][Agg Level=%d] NEW STATE:: PE->OFF\n", src, dst,application, g_otg->aggregation_level[src][dst][application]);  
       otg_info->c_holding_time_pe_off[src][dst][application]=0;
       otg_info->state_transition_prob[src][dst][application]=uniform_dist(0,1);
       otg_info->start_holding_time_off[src][dst][application]=ctime; 
			 otg_info->c_holding_time_off[src][dst][application]=0;
     }
     else /* if (g_otg->holding_time_pe_off>otg_info->c_holding_time_pe_off[src][dst])*/{
		 	if (ctime>otg_info->start_holding_time_pe_off[src][dst][application])
          otg_info->c_holding_time_pe_off[src][dst][application]=ctime-otg_info->start_holding_time_pe_off[src][dst][application];
      		LOG_I(OTG,"[%d][%d][Appli id %d] STATE:: PE \n", src, dst,application);
     	}
  break;
  default:
      LOG_W(OTG,"Unknown state(%d) \n", otg_info->state[src][dst][application]);
      otg_info->state[src][dst][application]= OFF_STATE; // switch to default state
      
      break;
}
}
}




void voip_traffic(int src, int dst, int application, int ctime) {

/*****************************************************************************************************
RECOMMANDATION UIT-T P.59 (VOIX CONVERSATIONNELLE ARTIFICIELLE)
Tst = − 0, 854 ln (1 − x1 ) //Tst (duration of single word)
Tdt = − 0, 226 ln (1 − x2 ) //Tdt (duration of double-talk)
Tms = − 0, 456 ln (1 − x3 ) //Tms (length of mutual silence)

0 < x1, x2, x3 < 1: Random variables with uniform distribution
*****************************************************************************************************/

	if (otg_info->silence_time[src][dst][application]==0){
  	//otg_info->voip_transition_prob[src][dst][application]=uniform_dist(0,1);
    otg_info->voip_state[src][dst][application]=SILENCE;
    LOG_I(OTG,"[%d][%d][Appli id %d] STATE:: SILENCE INIT \n", src, dst, application);
    otg_info->start_voip_silence[src][dst][application]=ctime /*ctime*/;
		otg_info->c_holding_time_talk[src][dst][application]=0;
		otg_info->c_holding_time_silence[src][dst][application]=0;
		otg_info->silence_time[src][dst][application]=ceil((-0.854*log(1-(uniform_dist(0,1))))*1000) + ceil((-0.226*log(1-(uniform_dist(0,1))))*1000);
		otg_info->simple_talk_time[src][dst][application]=ceil((-0.356*log(1-(uniform_dist(0,1))))*1000) ;

  }


	switch (otg_info->voip_state[src][dst][application]){

	case SILENCE:

		if (ctime>otg_info->start_voip_silence[src][dst][application]){
			if (ctime>otg_info->start_voip_silence[src][dst][application])
 				otg_info->c_holding_time_silence[src][dst][application]= ctime - otg_info->start_voip_silence[src][dst][application];
    	LOG_D(OTG,"[%d][%d][Appli id %d] VOIP STATE:: SILENCE %d (ctime=%d, start=%d)\n", src, dst,application, 
			otg_info->c_holding_time_silence[src][dst][application], ctime, otg_info->start_voip_silence[src][dst][application]);
    }

		if (otg_info->c_holding_time_silence[src][dst][application]>=otg_info->silence_time[src][dst][application]){  
			otg_info->voip_state[src][dst][application]=SIMPLE_TALK;
    	LOG_I(OTG,"[%d][%d][Appli id %d] NEW VOIP STATE :: SILENCE-->TALK  %d  (ctime=%d, start=%d )\n", src, dst,application , 
			otg_info->c_holding_time_silence[src][dst][application], ctime, otg_info->start_voip_silence[src][dst][application]);
			otg_info->start_voip_talk[src][dst][application]=ctime;
			otg_info->c_holding_time_talk[src][dst][application]=0;
			otg_info->simple_talk_time[src][dst][application]=ceil((-0.854*log(1-(uniform_dist(0,1))))*1000);
  	}else{
				if (ctime>otg_info->start_voip_silence[src][dst][application])
 					otg_info->c_holding_time_silence[src][dst][application]= ctime - otg_info->start_voip_silence[src][dst][application];
    	LOG_I(OTG,"[%d][%d][Appli id %d] STATE:: SILENCE [timer:%d] \n", src, dst,application, otg_info->c_holding_time_silence[src][dst][application]);
  	}
  break;
	case SIMPLE_TALK:
		if (otg_info->c_holding_time_talk[src][dst][application]>=otg_info->simple_talk_time[src][dst][application]){  
			otg_info->voip_state[src][dst][application]=SILENCE;
    	LOG_I(OTG,"[%d][%d][Appli id %d] NEW VOIP STATE:: TALK-->SILENCE  %d  (ctime=%d, start=%d )\n", src, dst,application ,
		  otg_info->c_holding_time_talk[src][dst][application], ctime, otg_info->start_voip_talk[src][dst][application]);
			otg_info->start_voip_silence[src][dst][application]=ctime;
			otg_info->c_holding_time_silence[src][dst][application]=0;
			otg_info->silence_time[src][dst][application]=ceil((-0.456*log(1-(uniform_dist(0,1))))*1000)+ceil((-0.226*log(1-(uniform_dist(0,1))))*1000);
  	}else{
				if (ctime>otg_info->start_voip_talk[src][dst][application])
 					otg_info->c_holding_time_talk[src][dst][application]= ctime - otg_info->start_voip_talk[src][dst][application];
     		LOG_I(OTG,"[%d][%d][Appli id %d] VOIP STATE:: TALK [timer:%d]\n", src, dst,application, otg_info->c_holding_time_talk[src][dst][application]);
				LOG_I(OTG, "test_talk [ctime %d] [start talk %d] [%d] \n",ctime, otg_info->start_voip_talk[src][dst][application], otg_info->c_holding_time_talk[src][dst][application] );
  		}
	break;
	default:
      LOG_W(OTG,"Unknown VOIP state(%d) \n", otg_info->voip_state[src][dst][application]);
      otg_info->voip_state[src][dst][application]= SILENCE; // switch to default state
      
      break;
}


}

