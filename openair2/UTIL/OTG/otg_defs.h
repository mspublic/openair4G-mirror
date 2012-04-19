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

/*! \file otg_defs.h main used structures
* \brief otg structure 
* \author N. Nikaein and A. Hafsaoui
* \date 2011
* \version 0.1
* \company Eurecom
* \email: openair_tech@eurecom.fr
* \note
* \warning
*/


#define  STANDALONE 1
#include "otg_config.h"
#include "PHY/impl_defs_top.h"

typedef enum {
	NO_PREDEFINED_TRAFFIC =0,
	CBR,
	M2M_AP,
	M2M_BR,
	GAMING_OA,
	GAMING_TF,
	FULL_BUFFER,
}Application;

typedef enum { /* \brief distribution */
	NO_TRAFFIC=0,
	MIN_NUM_DIST,
	UNIFORM, 
	GAUSSIAN,
	EXPONENTIAL,
	POISSON,
	MAX_NUM_DIST,
	FIXED,
}dist_type;

typedef enum { /* \brief TRANSPORT PROTO */
	UDP=0,
	TCP,
}trans_proto;

typedef enum { /* \brief IP version */
	IPV4=0,
	IPV6, 
}ip_v;

typedef enum { /* \brief Alphabet type to generate random string */ 
	NUM_LETTER=0,
	NUM, 
}ALPHABET;


typedef enum { /* \brief Alphabet type to generate random string */ 
	CRC_8=0,
	CRC_16,
	CRC_24A,
 	CRC_24B,
}CRC;



/*! openair traffic generator global parameters*/
typedef struct {
	int application_type[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; // could be char *, cbr, M2M_sensor, M2M_xxx, FPS_, random, full_buffer, ...
	int num_nodes;
	// header info
	int trans_proto[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
	int ip_v[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
	//int header_compression; 
	
	//payload info
	
	// src id , dst id, and state  						// think to the case of several streams per node !!!!!
	int idt_dist[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE];	// idt_dist[0][0]:default/on, 1 : off, 2: active 	
	int idt_min[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE];// described as min pkt per second 	
	int idt_max[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE];

	int idt_std_dev[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE];
	int idt_lambda[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE];


	int size_dist[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE];		
	int size_min[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE];	
	int size_max[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE];
	int size_std_dev[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE];
	int size_lambda[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE];

	
	// info for state-based traffic gen
	int num_state;
	int state_dist[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE];
	int state_prob[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE];
	
	// num stream for each src
	// int stream [NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; // this requires multi thread for parallel stream for a givcen src	
	// emu info
	int duration[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; /*!< duration of traffic generation or use the emuulation time instead */
	
	int rng_func; 
	int seed; /*!< The seed used to generate the random positions*/

#ifdef STANDALONE
	int  dst_port[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];//[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE];
	char *dst_ip[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; //[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE];
		
#endif 

}otg_t; 

typedef struct{
	int flag:16; //4
	int time:16; //8
	int seq_num:16; //4
	unsigned int crc16; //:32 
}__attribute__((__packed__)) otg_hdr_t;


typedef struct{
 //info
	int emu_time; // like tick, in ms, otg will be called every 1ms 	
	int seq_num [NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
//statics
	int tx_num_pkt[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
	int tx_throughput[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; // get the size and calculate the avg throughput
	
	int rx_num_pkt[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
	int rx_loss_rate[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
	int rx_goodput[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
	int rx_latency[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
	
}otg_info_t;

