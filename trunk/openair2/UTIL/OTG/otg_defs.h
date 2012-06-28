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

#ifndef __OTG_DEFS_H__
#	define __OTG_DEFS_H__


#if STANDALONE==1
#	define NUMBER_OF_eNB_MAX 3
#	define NUMBER_OF_UE_MAX 3
#else
	#include "PHY/impl_defs_top.h" /* \brief To define the NUMBER_OF_eNB_MAX and NUMBER_OF_UE_MAX */
#endif	



#include "otg_config.h"

/**
* \enum Application
* \brief OTG applications type 
* 
*
* Application describes the class of traffic supported and can be pre-configured bu otg_tx
*/
typedef enum {
	NO_PREDEFINED_TRAFFIC =0,
	SCBR,
	MCBR,
	BCBR,
	AUTO_PILOT,
	BICYCLE_RACE,
	OPENARENA,
	TEAM_FORTRESS,
	FULL_BUFFER,
}Application;


/**
* \enum dist_type
* \brief OTG implemented distribution for packet and inter departure time computation
* 
*
* dist_type presents the used distribition to generate inter departure time and packet size
*/

typedef enum {  
	NO_TRAFFIC=0,
	UNIFORM, 
	GAUSSIAN,
	EXPONENTIAL,
	POISSON,
	FIXED,
	WEIBULL,
	PARETO,
	GAMMA,
	CAUCHY,
        LOG_NORMAL,
}dist_type;


/**
* \enum trans_proto
*
* \brief trans_proto enmerates used transport protocol
*
*
*/

typedef enum { 
	NO_PROTO=0,
	UDP=1,
	TCP,
}trans_proto;


/**
* \enum ip_v
*
*\brief ip_v presents the used IP version to generate the packet 
*
*/
typedef enum { 	
	NO_IP=0,
	IPV4=1,
	IPV6, 
}ip_v;

/**
* \enum ip_v
*
*\brief ip_v presents the used IP version to generate the packet 
*
*/
typedef enum { 	
  //	MIN_NUM_STATE,
	ON_STATE=0,
	OFF_STATE,
	ACTIVE_STATE,
	MAX_NUM_STATE,
}TRAFFIC_STATE;

/**
* \enum ALPHABET
*
*\brief ALPHABET Alphabet type to generate random string 
*
*/
typedef enum {  
	STATIC_STRING=0,
	RANDOM_STRING, 
}ALPHABET;



/**
* \enum HEADER_TYPE
*
* \brief HEADER_TYPE alows to identify the transport protocol and IP version    
*
*/

typedef enum { 
NO_HEADER=0,
UDP_IPV4,
TCP_IPV4,
UDP_IPV6,
TCP_IPV6,
}HEADER_TYPE; 


/**
* \struct otg_t
*
*\brief otg_t  define the traffic generator global parameters, it include a matrix of nodes (source, destination and state) parameters
*
*/
typedef struct {
	int application_type[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];  /*!\brief It identify the application of the simulated traffic, could be cbr, m2m, gaming,etc*/ 
 /*!\header info */
	int trans_proto[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; 	/*!\brief Transport Protocol*/
	int ip_v[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; 	/*!\brief Ip version */
	//int header_compression; 				/*!\brief Specify if header compression is used or not */
	int num_nodes; 						/*!\brief Number of used nodes in the simulation */
	int background[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; /*!\brief enable or disable background traffic  */

	// src id , dst id, and state  						// think to the case of several streams per node !!!!!
	int idt_dist[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE];	/*!\brief Inter Departure Time distribution */	
	int idt_min[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE]; 	/*!\brief Min Inter Departure Time, for uniform distrib  */
	int idt_max[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE]; 	/*!\brief idt, Max Inter Departure Time, for uniform distrib  */

	double idt_std_dev[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE]; 	/*!\brief idt, Standard Deviation, for guassian distrib */
	double idt_lambda[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE];     /*!\brief idt, lambda, for exponential/poisson  distrib */
	double idt_scale[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE];  	/*!\brief scale :parameter for Pareto, Gamma, Weibull and Cauchy distribution*/
	double idt_shape[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE]; 	/*!\brief shape :parameter for Pareto, Gamma, Weibull and Cauchy distribution*/

	int size_dist[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE];	/*!\brief Paylolad size distribution */	
	int size_min[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE];	/*!\brief Min Payload size, for uniform distrib  */
	int size_max[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE]; 	/*!\brief payload, Max Inter Departure Time, for uniform distrib  */
	double size_std_dev[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE]; 	/*!\brief payload, Standard Deviation, for guassian distrib */
	double size_lambda[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE];     /*!\brief payload, lambda, for exponential/poisson  distrib */

	double size_scale[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE];  	/*!\brief scale :parameter for Pareto, Gamma, Weibull and Cauchy distribution */
	double size_shape[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE]; 	/*!\brief shape :parameter for Pareto, Gamma, Weibull and Cauchy distribution*/

	// info for state-based traffic gen
	int num_state [NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; /*!\brief Number of states for source node*/
  //	int state_dist[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE]; /*!\brief States distribution */ 
	double state_prob[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE]; /*!\brief State probablity: prob to move from one state to the other one */
	
	// num stream for each src
	// int stream [NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; // this requires multi thread for parallel stream for a givcen src	
	// emu info
	int duration[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; /*!\brief Duration of traffic generation or use the emuulation time instead */
	int seed; /*!\brief The seed used to generate the random positions*/


	int  dst_port[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; /*!\brief Destination port number, for the socket mode*/
	char *dst_ip[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; /*!\brief Destination IP address, for the socket mode*/

	int trans_proto_background[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; /*!\brief define the transport protocol and IP version for background traffic*/



// add m2m specific config params by Lusheng below
// to be improved by Aymen

	double prob_off_pu;
	double prob_pu_ed;
	double holding_time_off_pu;

	double prob_off_ed;
	double prob_ed_pe;
	double holding_time_off_ed;
	
	double holding_time_off_pe;


unsigned int throughput_metric;
unsigned int latency_metric; 
unsigned int loss_metric;
unsigned int graphic;
unsigned int owd_radio_access;
}otg_t; 


typedef struct{

	unsigned char flow_id; 	/*!< \brief It identify the flow ID (we can have source and destination with several flows)  */
	unsigned int time; 		/*!< \brief simulation time at the tx, this is ctime */
	unsigned int seq_num; 	/*!< \brief Sequence Number, counter of data packets between tx and rx */  
	unsigned char state; 	/*!< \brief state of node : on, off, or active */
	unsigned char hdr_type; 	/*!< \brief Header type: tcp/udp vs ipv4/ipv6 */
	unsigned short pkts_size;		/*!< \brief the size of payload + header */
	//int payload_size;	/*!< \brief the size of the payload to transmit */
	//int header_size; 	/*!< \brief Header type: tcp/udp vs ipv4/ipv6 */
}__attribute__((__packed__)) otg_hdr_t;

typedef struct{
	unsigned int flag;
	unsigned int size;
}__attribute__((__packed__)) otg_hdr_info_t;
 

/**
* \struct packet_t
*
* \brief packet_t corresponds to the global structure of the generated packet
*
*
*/


typedef struct{
	char* header ; 		/*!< \brief  Header */
	char* payload; 		/*!< \brief  Payload*/  
}__attribute__((__packed__)) packet_t;



/**
* \struct packet_t
*
* \brief The OTG information and KPI of the simulation structure
*
*/

typedef struct{
/*!< \brief  info part: */ 
	int ctime; 												/*!< \brief Simulation time in ms*/
	int ptime_background;										/*!< \brief time of last sent background data (time in ms)*/		
	int ptime[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; 			/*!< \brief time of last sent data (time in ms)*/		
	int seq_num[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];		/*!< \brief the sequence number of the sender  */
	int seq_num_background[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];     /*!< \brief the sequence number for background traffic of the sender  */
	int seq_num_rx[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];		/*!< \brief the sequence number of the receiver */
        int seq_num_rx_background[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];	/*!< \brief the sequence number for background traffic of the receiver */


	int idt_background[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];         /*!< \brief  Inter Departure Time for background traffic in ms*/
	int size_background[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];        /*!< \brief  payload size for background traffic*/
	int idt[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];			/*!< \brief  Inter Departure Time in ms*/
	int state[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];			                /*!< \brief  current state of src node */
	int header_type[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];		/*!< \brief  Define the type of header: Transport layer + IP version*/
			
/*!< \brief Statics part: vars updated at each iteration of otg_tx */
	int tx_num_pkt[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];		/*!< \brief  Number of data packet in the tx*/
	int tx_num_bytes[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; 		/*!< \brief  Number of bytes in the tx*/ // get the size and calculate the avg throughput
	// vars updated at each iteration of otg_rx	
	int rx_num_pkt[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; 		/*!< \brief  Number of data packet in the rx */
	int rx_num_bytes[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; 		/*!< \brief  Number of bytes in the rx */
	int rx_pkt_owd[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; 		/*!< \brief  One way delay: rx_ctime - tx_ctime */  
	int rx_owd_min[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];  		/*!< \brief  One way delay min*/
	int rx_owd_max[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; 		/*!< \brief  One way delay max*/
	int nb_loss_pkts[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];		/*!< \brief  Number of data packets losses*/
	int owd_const[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];		/*!< \brief  One way delay constant*/
/*!< \brief KPI part: calculate the KPIs, total */ 
	float tx_throughput[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; 	/*!< \brief  Tx throughput: (size of transmitted data)/ctime*/ 
	float rx_goodput[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; 		/*!< \brief  Rx goodput: (size of received data)/ctime*/
	float rx_loss_rate[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; 	/*!< \brief  Rx Loss Rate: ratio, unit: bytes*/
	//int rx_latency[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; 		/*!< \brief  Rx Latency */
/*!< \brief Background traffic part: SATS + KPIs */
        int tx_num_pkt_background[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; 	/*!< \brief  Number of background data packet in the rx */
	int tx_num_bytes_background[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; 	/*!< \brief  Number of background bytes in the rx */
        int rx_num_pkt_background[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; 	/*!< \brief  Number of background data packet in the tx */
	int rx_num_bytes_background[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; 	/*!< \brief  Number of background bytes in the tx */

//	int rx_pkt_owd_background[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; 		/*!< \brief  One way delay: rx_ctime - tx_ctime */  
//	int rx_owd_min_background[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];  	/*!< \brief  One way delay min*/
//	int rx_owd_max_background[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; 		/*!< \brief  One way delay max*/


        int nb_loss_pkts_background[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
        float tx_throughput_background[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; 	/*!< \brief  Tx throughput: (size of transmitted data)/ctime*/ 
	float rx_goodput_background[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; 	/*!< \brief  Rx goodput: (size of received data)/ctime*/
	float rx_loss_rate_background[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; 	/*!< \brief  Rx Loss Rate: ratio, unit: bytes*/

float radio_access_delay[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; 

	

}otg_info_t;


//char buffer_tx[MAX_BUFF_TX]; /*!< \brief define the buffer for the data to transmit */


typedef struct{
//	char *src_ip; 		/*!< \brief   */
	int src:16; 		/*!< \brief  */
	int dst:16; 		/*!< \brief  */  
	int  trans_proto:16; 	/*!< \brief  */
	int  ip_v:16; 		/*!< \brief  */
//	int payload_size:16;
}__attribute__((__packed__)) control_hdr_t;




typedef struct{
	control_hdr_t* control_hdr; 	/*!< \brief   */
	char* payload_rest; 			/*!< \brief  */  
}__attribute__((__packed__)) payload_t;




#define MAX_SAMPLES 100

typedef struct{
	int init_forms;
	int is_data_plot_ul;
	int is_data_plot_dl;
	float data_owd_ul[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_SAMPLES];
	float data_throughput_ul[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_SAMPLES];
	float data_ctime_ul[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_SAMPLES];
	int idx_ul[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
	float data_owd_dl[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_SAMPLES];
	float data_throughput_dl[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_SAMPLES];
	float data_ctime_dl[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_SAMPLES];
	int idx_dl[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
}otg_forms_info_t;

#endif
