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

/*! \file OCG_parse_XML.h
* \brief Variables indicating the element which is currently parsed
* \author Lusheng Wang
* \date 2011
* \version 0.1
* \company Eurecom
* \email: lusheng.wang@eurecom.fr
* \note
* \warning
*/

#ifndef __OCG_PARSE_XML_H__

#define __OCG_PARSE_XML_H__

#ifdef __cplusplus
extern "C" {
#endif
/** @defgroup _parsing_position_indicator Parsing Position Indicator
 *  @ingroup _parse_XML
 *  @brief Indicate the position where the program is current parsing in the XML file
 * @{*/ 
int oai_emulation;	/*!< \brief indicating that the parsing position is now within OAI_Emulation_*/
int envi_config;		/*!< \brief indicating that the parsing position is now within Envi_Config_*/
int area;				/*!< \brief indicating that the parsing position is now within Area_*/
int x;
int y;
int geography;			/*!< \brief indicating that the parsing position is now within Geography_*/
int home;
int urban;
int rural;
int topography;		/*!< \brief indicating that the parsing position is now within Topography_*/
int flat;
int obstructed;
int hilly;
int fading;				/*!< \brief indicating that the parsing position is now within Fading_*/
int free_space_propagation;	/*!< \brief indicating that the parsing position is now within Free_Space_Propagation_*/
int pathloss_exponent;
int pathloss_0;
int small_scale;		/*!< \brief indicating that the parsing position is now within Small_Scale_*/
int rayleigh;
int delay_spread;
int rician;
int wall_penetration_loss;
int noise_power;
int topo_config;		/*!< \brief indicating that the parsing position is now within Topo_Config_*/
int net_type;
int homogeneous;
int heterogeneous;
int cell_type;			/*!< \brief indicating that the parsing position is now within Cell_Type_*/
int macrocell;
int microcell;
int picocell;
int femtocell;
int relay;
int number_of_relays;
int eNB_topology;		/*!< \brief indicating that the parsing position is now within eNB_Topology_*/
int grid;
int hexagonal;
int number_of_cells;
int totally_random; 	/*!< \brief to take the place of 'random' which is reserved for C*/ 
int inter_eNB_distance;
int UE_distribution;	/*!< \brief indicating that the parsing position is now within UE_Distribution_*/
int concentrated;
int grid_map;
int inter_block_distance;
int system_bandwidth;
int UE_frequency;
int mobility;			/*!< \brief indicating that the parsing position is now within Mobility_*/
int mobility_type;	/*!< \brief indicating that the parsing position is now within Mobility_Type_*/
int fixed;
int random_waypoint;
int random_walk;
int grid_walk;
int moving_dynamics;	/*!< \brief indicating that the parsing position is now within Moving_Dynamics_*/
int min_speed;
int max_speed;
int min_pause_time;
int max_pause_time;
int app_config;		/*!< \brief indicating that the parsing position is now within App_Config_*/
int app_type;			/*!< \brief indicating that the parsing position is now within App_Type_*/
int cbr;
int gaming;
int m2m;
int traffic;			/*!< \brief indicating that the parsing position is now within Traffic_*/
int transport_protocol;	/*!< \brief indicating that the parsing position is now within Transport_Protocol_*/
int udp;
int tcp;
int packet_size;		/*!< \brief indicating that the parsing position is now within Packet_Size_*/
int fixed_value;
int uniform;
int min_value;
int max_value;
int inter_arrival_time;	/*!< \brief indicating that the parsing position is now within Inter_Arrival_Time_*/
int poisson;
int expected_inter_arrival_time;
int emu_config;		/*!< \brief indicating that the parsing position is now within Emu_Config_*/
int emu_time;
int performance;		/*!< \brief indicating that the parsing position is now within Performance_*/
int metric;
int throughput;
int latency;
int signalling_overhead;
int layer;				/*!< \brief indicating that the parsing position is now within Layer_*/
int mac;
int rlc;
int pdcp;
int log_emu;
int debug;
int info;
int warning;
int error;
int packet_trace;
int profile;
/* @}*/

/** @defgroup _parse_XML Parse XML
 *  @ingroup _fn
 *  @brief Parse the XML configuration file
 * @{*/ 
	int parse_XML(char src_file[FILENAME_LENGTH_MAX + DIR_LENGTH_MAX]);
/* @}*/

#ifdef __cplusplus
}
#endif

#endif
