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
	#include "../../../openair1/PHY/impl_defs_top.h" /* \brief To define the NUMBER_OF_eNB_MAX and NUMBER_OF_UE_MAX */
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
	CBR,
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
* \enum ALPHABET
*
*\brief ALPHABET Alphabet type to generate random string 
*
*/
typedef enum {  
	NUM_LETTER=0,
	NUM, 
}ALPHABET;



/**
* \enum HEADER_TYPE
*
* \brief HEADER_TYPE alows to identify the transport protocol and IP version    
*
*/

typedef enum { 
TCP_IPV4=0,
UDP_IPV4,
TCP_IPV6,
UDP_IPV6,
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
	int num_state; /*!\brief Number of states */
	int state_dist[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE]; /*!\brief States distribution */ 
	int state_prob[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE]; /*!\brief State probablity: prob to move from one state to the other one */
	
	// num stream for each src
	// int stream [NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; // this requires multi thread for parallel stream for a givcen src	
	// emu info
	int duration[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; /*!\brief Duration of traffic generation or use the emuulation time instead */
	int seed; /*!\brief The seed used to generate the random positions*/


	int  dst_port[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; /*!\brief Destination port number, for the socket mode*/
	char *dst_ip[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; /*!\brief Destination IP address, for the socket mode*/
}otg_t; 


/**
* \struct otg_hdr_t
*
* \brief otg_hdr_t corresponds to the added OTG control header to check the received packet at the server side
*
*
*/
typedef struct{
	int flow_id:16; 	/*!< \brief It identify the flow ID (we can have source and destination with several flows)  */
	int time:16; 		/*!< \brief simulation time at the tx, this is ctime */
	int seq_num:16; 	/*!< \brief Sequence Number, counter of data packets between tx and rx */  
	int hdr_type:16; 	/*!< \brief Header type: tcp/udp vs ipv4/ipv6 */
}__attribute__((__packed__)) otg_hdr_t;



/**
* \struct packet_t
*
* \brief packet_t corresponds to the global structure of the generated packet
*
*
*/

typedef struct{
	otg_hdr_t*  otg_hdr; 	/*!< \brief OTG header  */
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
	int ctime; 						/*!< \brief Simulation time in ms*/							
	int ptime[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][MAX_NUM_TRAFFIC_STATE]; /*!< \brief time of last sent data (time in ms)*/		
	int seq_num[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];		/*!< \brief the simulation time from the simulator, in ms  */	
	int idt[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];			/*!< \brief  Inter Departure Time in ms*/
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

/*!< \brief KPI part: calculate the KPIs, total */ 
	float tx_throughput[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; 	/*!< \brief  Tx throughput: (size of transmitted data)/ctime*/ 
	float rx_goodput[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; 		/*!< \brief  Rx goodput: (size of received data)/ctime*/
	float rx_loss_rate[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; 	/*!< \brief  Rx Loss Rate: ratio, unit: bytes*/
	//int rx_latency[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX][NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX]; 		/*!< \brief  Rx Latency */
	
}otg_info_t;


char buffer_tx[MAX_BUFF_TX]; /*!< \brief define the buffer for the data to transmit */

#endif
