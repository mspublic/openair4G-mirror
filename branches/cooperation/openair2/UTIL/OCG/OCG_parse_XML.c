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

/*! \file OCG_parse_XML.c
* \brief Parse the content of the XML configuration file
* \author Lusheng Wang
* \date 2011
* \version 0.1
* \company Eurecom
* \email: lusheng.wang@eurecom.fr
* \note
* \warning
*/

/*--- INCLUDES ---------------------------------------------------------------*/
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <stdio.h>
#include <string.h>
#include "OCG.h"
#include "OCG_extern.h"
#include "OCG_parse_XML.h"
/*----------------------------------------------------------------------------*/

static int oai_emulation_;	/*!< \brief indicating that the parsing position is now within OAI_Emulation_*/
static int envi_config_;		/*!< \brief indicating that the parsing position is now within Envi_Config_*/
static int area_;				/*!< \brief indicating that the parsing position is now within Area_*/
static int x_;
static int y_;
static int geography_;			/*!< \brief indicating that the parsing position is now within Geography_*/
static int home_;
static int urban_;
static int rural_;
static int topography_;		/*!< \brief indicating that the parsing position is now within Topography_*/
static int flat_;
static int obstructed_;
static int hilly_;
static int fading_;				/*!< \brief indicating that the parsing position is now within Fading_*/
static int free_space_propagation_;	/*!< \brief indicating that the parsing position is now within Free_Space_Propagation_*/
static int pathloss_exponent_;
static int pathloss_0_;
static int small_scale_;		/*!< \brief indicating that the parsing position is now within Small_Scale_*/
static int rayleigh_;
static int delay_spread_;
static int rician_;
static int wall_penetration_loss_;
static int noise_power_;
static int topo_config_;		/*!< \brief indicating that the parsing position is now within Topo_Config_*/
static int net_type_;
static int homogeneous_;
static int heterogeneous_;
static int cell_type_;			/*!< \brief indicating that the parsing position is now within Cell_Type_*/
static int macrocell_;
static int microcell_;
static int picocell_;
static int femtocell_;
static int relay_;
static int number_of_relays_;
static int eNB_topology_;		/*!< \brief indicating that the parsing position is now within eNB_Topology_*/
static int grid_;
static int hexagonal_;
static int number_of_cells_;
static int number_of_eNB_;
static int number_of_UE_;
static int totally_random_; 	/*!< \brief to take the place of 'random' which is reserved for C*/ 
static int inter_eNB_distance_;
static int UE_distribution_;	/*!< \brief indicating that the parsing position is now within UE_Distribution_*/
static int concentrated_;
static int grid_map_;
static int inter_block_distance_;
static int system_bandwidth_;
static int UE_frequency_;
static int mobility_;			/*!< \brief indicating that the parsing position is now within Mobility_*/
static int mobility_type_;	/*!< \brief indicating that the parsing position is now within Mobility_Type_*/
static int fixed_;
static int random_waypoint_;
static int random_walk_;
static int grid_walk_;
static int moving_dynamics_;	/*!< \brief indicating that the parsing position is now within Moving_Dynamics_*/
static int min_speed_;
static int max_speed_;
static int min_pause_time_;
static int max_pause_time_;
static int app_config_;		/*!< \brief indicating that the parsing position is now within App_Config_*/
static int app_type_;			/*!< \brief indicating that the parsing position is now within App_Type_*/
static int cbr_;
static int gaming_;
static int m2m_;
static int traffic_;			/*!< \brief indicating that the parsing position is now within Traffic_*/
static int transport_protocol_;	/*!< \brief indicating that the parsing position is now within Transport_Protocol_*/
static int udp_;
static int tcp_;
static int packet_size_;		/*!< \brief indicating that the parsing position is now within Packet_Size_*/
static int fixed_value_;
static int uniform_;
static int min_value_;
static int max_value_;
static int inter_arrival_time_;	/*!< \brief indicating that the parsing position is now within Inter_Arrival_Time_*/
static int poisson_;
static int expected_inter_arrival_time_;
static int emu_config_;		/*!< \brief indicating that the parsing position is now within Emu_Config_*/
static int emu_time_;
static int performance_;		/*!< \brief indicating that the parsing position is now within Performance_*/
static int metric_;
static int throughput_;
static int latency_;
static int signalling_overhead_;
static int layer_;				/*!< \brief indicating that the parsing position is now within Layer_*/
static int mac_;
static int rlc_;
static int pdcp_;
static int log_emu_;
static int debug_;
static int info_;
static int warning_;
static int error_;
static int packet_trace_;
static int profile_;

void start_document(void *user_data) {
	//printf("Start parsing ............ \n");
}

void end_document(void *user_data) {
	//printf("End parsing .\n");
}

void start_element(void *user_data, const xmlChar *name, const xmlChar **attrs) { // called once at the beginning of each element 
	if (!xmlStrcmp(name, "OPENAIRINTERFACE")) {
		oai_emulation_ = 1;
	} else if (!xmlStrcmp(name, "ENVICONFIG")) {
		envi_config_ = 1;
	} else if (!xmlStrcmp(name, "AREA")) {
		area_ = 1;
	} else if (!xmlStrcmp(name, "X")) {
		x_ = 1;
	} else if (!xmlStrcmp(name, "Y")) {
		y_ = 1;
	} else if (!xmlStrcmp(name, "GEOGRAPHY")) {
		geography_ = 1;
	} else if (!xmlStrcmp(name, "HOME")) {
		home_ = 1;
	} else if (!xmlStrcmp(name, "URBAN")) {
		urban_ = 1;
	} else if (!xmlStrcmp(name, "RURAL")) {
		rural_ = 1;
	} else if (!xmlStrcmp(name, "TOPOGRAPHY")) {
		topography_ = 1;
	} else if (!xmlStrcmp(name, "FLAT")) {
		flat_ = 1;
	} else if (!xmlStrcmp(name, "OBSTRUCTED")) {
		obstructed_ = 1;
	} else if (!xmlStrcmp(name, "HILLY")) {
		hilly_ = 1;
	} else if (!xmlStrcmp(name, "FADING")) {
		fading_ = 1;
	} else if (!xmlStrcmp(name, "FREESPACEPROPAGATION")) {
		free_space_propagation_ = 1;
	} else if (!xmlStrcmp(name, "PATHLOSSEXPONENT")) {
		pathloss_exponent_ = 1;
	} else if (!xmlStrcmp(name, "PATHLOSS0")) {
		pathloss_0_ = 1;
	} else if (!xmlStrcmp(name, "SMALLSCALE")) {
		small_scale_ = 1;
	} else if (!xmlStrcmp(name, "RAYLEIGH")) {
		rayleigh_ = 1;
	} else if (!xmlStrcmp(name, "DELAYSPREAD")) {
		delay_spread_ = 1;
	} else if (!xmlStrcmp(name, "RICIAN")) {
		rician_ = 1;
	} else if (!xmlStrcmp(name, "WALLPENETRATIONLOSS")) {
		wall_penetration_loss_ = 1;
	} else if (!xmlStrcmp(name, "NOISEPOWER")) {
		noise_power_ = 1;
	} else if (!xmlStrcmp(name, "TOPOCONFIG")) {
		topo_config_ = 1;
	} else if (!xmlStrcmp(name, "TYPE")) {
		net_type_ = 1;
	} else if (!xmlStrcmp(name, "HOMOGENEOUS")) {
		homogeneous_ = 1;
	} else if (!xmlStrcmp(name, "HETEROGENEOUS")) {
		heterogeneous_ = 1;
	} else if (!xmlStrcmp(name, "CELLTYPE")) {
		cell_type_ = 1;
	} else if (!xmlStrcmp(name, "MACROCELL")) {
		macrocell_ = 1;
	} else if (!xmlStrcmp(name, "MICROCELL")) {
		microcell_ = 1;
	} else if (!xmlStrcmp(name, "PICOCELL")) {
		picocell_ = 1;
	} else if (!xmlStrcmp(name, "FEMTOCELL")) {
		femtocell_ = 1;
	} else if (!xmlStrcmp(name, "RELAY")) {
		relay_ = 1;
	} else if (!xmlStrcmp(name, "NUMBEROFRELAYS")) {
		number_of_relays_ = 1;
	} else if (!xmlStrcmp(name, "ENBTOPOLOGY")) {
		eNB_topology_ = 1;
	} else if (!xmlStrcmp(name, "GRID")) {
		grid_ = 1;
	} else if (!xmlStrcmp(name, "HEXAGONAL")) {
		hexagonal_ = 1;
	} else if (!xmlStrcmp(name, "NBCELLS")) {
		number_of_cells_ = 1;
	} else if (!xmlStrcmp(name, "NUMBEROFENB")) {
		number_of_eNB_ = 1;
	} else if (!xmlStrcmp(name, "NBUES")) {
		number_of_UE_ = 1;
	} else if (!xmlStrcmp(name, "RANDOM")) {
		totally_random_ = 1;
	} else if (!xmlStrcmp(name, "INTERENBDISTANCE")) {
		inter_eNB_distance_ = 1;
	} else if (!xmlStrcmp(name, "UEDISTRIBUTION")) {
		UE_distribution_ = 1;
	} else if (!xmlStrcmp(name, "CONCENTRATED")) {
		concentrated_ = 1;
	} else if (!xmlStrcmp(name, "GRIDMAP")) {
		grid_map_ = 1;
	} else if (!xmlStrcmp(name, "INTERBLOCKDISTANCE")) {
		inter_block_distance_ = 1;
	} else if (!xmlStrcmp(name, "SYSTEMBANDWIDTH")) {
		system_bandwidth_ = 1;
	} else if (!xmlStrcmp(name, "UEFREQUENCY")) {
		UE_frequency_ = 1;
	} else if (!xmlStrcmp(name, "MOBILITY")) {
		mobility_ = 1;
	} else if (!xmlStrcmp(name, "MOBILITYTYPE")) {
		mobility_type_ = 1;
	} else if (!xmlStrcmp(name, "FIXED")) {
		fixed_ = 1;
	} else if (!xmlStrcmp(name, "RANDOMWAYPOINT")) {
		random_waypoint_ = 1;
	} else if (!xmlStrcmp(name, "RANDOMWALK")) {
		random_walk_ = 1;
	} else if (!xmlStrcmp(name, "GRIDWALK")) {
		grid_walk_ = 1;
	} else if (!xmlStrcmp(name, "MOVINGDYNAMICS")) {
		moving_dynamics_ = 1;
	} else if (!xmlStrcmp(name, "MINSPEED")) {
		min_speed_ = 1;
	} else if (!xmlStrcmp(name, "MAXSPEED")) {
		max_speed_ = 1;
	} else if (!xmlStrcmp(name, "MINPAUSETIME")) {
		min_pause_time_ = 1;
	} else if (!xmlStrcmp(name, "MAXPAUSETIME")) {
		max_pause_time_ = 1;
	} else if (!xmlStrcmp(name, "APPCONFIG")) {
		app_config_ = 1;
	} else if (!xmlStrcmp(name, "APPTYPE")) {
		app_type_ = 1;
	} else if (!xmlStrcmp(name, "CBR")) {
		cbr_ = 1;
	} else if (!xmlStrcmp(name, "GAMING")) {
		gaming_ = 1;
	} else if (!xmlStrcmp(name, "M2M")) {
		m2m_ = 1;
	} else if (!xmlStrcmp(name, "TRAFFIC")) {
		traffic_ = 1;
	} else if (!xmlStrcmp(name, "TRANSPORTPROTOCOL")) {
		transport_protocol_ = 1;
	} else if (!xmlStrcmp(name, "UDP")) {
		udp_ = 1;
	} else if (!xmlStrcmp(name, "TCP")) {
		tcp_ = 1;
	} else if (!xmlStrcmp(name, "PACKETSIZE")) {
		packet_size_ = 1;
	} else if (!xmlStrcmp(name, "FIXEDVALUE")) {
		fixed_value_ = 1;
	} else if (!xmlStrcmp(name, "UNIFORM")) {
		uniform_ = 1;
	} else if (!xmlStrcmp(name, "MINVALUE")) {
		min_value_ = 1;
	} else if (!xmlStrcmp(name, "MAXVALUE")) {
		max_value_ = 1;
	} else if (!xmlStrcmp(name, "INTERARRIVALTIME")) {
		inter_arrival_time_ = 1;
	} else if (!xmlStrcmp(name, "POISSON")) {
		poisson_ = 1;
	} else if (!xmlStrcmp(name, "EXPECTEDINTERARRIVALTIME")) {
		expected_inter_arrival_time_ = 1;
	} else if (!xmlStrcmp(name, "EMUCONFIG")) {
		emu_config_ = 1;
	} else if (!xmlStrcmp(name, "EMUTIME")) {
		emu_time_ = 1;
	} else if (!xmlStrcmp(name, "PERFORMANCE")) {
		performance_ = 1;
	} else if (!xmlStrcmp(name, "METRIC")) {
		metric_ = 1;
	} else if (!xmlStrcmp(name, "THROUGHPUT")) {
		throughput_ = 1;
	} else if (!xmlStrcmp(name, "LATENCY")) {
		latency_ = 1;
	} else if (!xmlStrcmp(name, "SIGNALLINGOVERHEAD")) {
		signalling_overhead_ = 1;
	} else if (!xmlStrcmp(name, "LAYER")) {
		layer_ = 1;
	} else if (!xmlStrcmp(name, "MAC")) {
		mac_ = 1;
	} else if (!xmlStrcmp(name, "RLC")) {
		rlc_ = 1;
	} else if (!xmlStrcmp(name, "PDCP")) {
		pdcp_ = 1;
	} else if (!xmlStrcmp(name, "LOGEMUL")) {
		log_emu_ = 1;
	} else if (!xmlStrcmp(name, "DEBUG")) {
		debug_ = 1;
	} else if (!xmlStrcmp(name, "INFO")) {
		info_ = 1;
	} else if (!xmlStrcmp(name, "WARNING")) {
		warning_ = 1;
	} else if (!xmlStrcmp(name, "ERROR")) {
		error_ = 1;
	} else if (!xmlStrcmp(name, "PACKETTRACE")) {
		packet_trace_ = 1;
	} else if (!xmlStrcmp(name, "PROFILE")) {
		profile_ = 1;
	} else {
		LOG_W(OCG, "One element could not be parsed : unknown element name '%s'\n", name);
	}
}

void end_element(void *user_data, const xmlChar *name) { // called once at the end of each element 
	if (!xmlStrcmp(name, "OPENAIRINTERFACE")) {
		oai_emulation_ = 0;
	} else if (!xmlStrcmp(name, "ENVICONFIG")) {
		envi_config_ = 0;
	} else if (!xmlStrcmp(name, "AREA")) {
		area_ = 0;
	} else if (!xmlStrcmp(name, "X")) {
		x_ = 0;
	} else if (!xmlStrcmp(name, "Y")) {
		y_ = 0;
	} else if (!xmlStrcmp(name, "GEOGRAPHY")) {
		geography_ = 0;
	} else if (!xmlStrcmp(name, "HOME")) {
		home_ = 0;
	} else if (!xmlStrcmp(name, "URBAN")) {
		urban_ = 0;
	} else if (!xmlStrcmp(name, "RURAL")) {
		rural_ = 0;
	} else if (!xmlStrcmp(name, "TOPOGRAPHY")) {
		topography_ = 0;
	} else if (!xmlStrcmp(name, "FLAT")) {
		flat_ = 0;
	} else if (!xmlStrcmp(name, "OBSTRUCTED")) {
		obstructed_ = 0;
	} else if (!xmlStrcmp(name, "HILLY")) {
		hilly_ = 0;
	} else if (!xmlStrcmp(name, "FADING")) {
		fading_ = 0;
	} else if (!xmlStrcmp(name, "FREESPACEPROPAGATION")) {
		free_space_propagation_ = 0;
	} else if (!xmlStrcmp(name, "PATHLOSSEXPONENT")) {
		pathloss_exponent_ = 0;
	} else if (!xmlStrcmp(name, "PATHLOSS0")) {
		pathloss_0_ = 0;
	} else if (!xmlStrcmp(name, "SMALLSCALE")) {
		small_scale_ = 0;
	} else if (!xmlStrcmp(name, "RAYLEIGH")) {
		rayleigh_ = 0;
	} else if (!xmlStrcmp(name, "DELAYSPREAD")) {
		delay_spread_ = 0;
	} else if (!xmlStrcmp(name, "RICIAN")) {
		rician_ = 0;
	} else if (!xmlStrcmp(name, "WALLPENETRATIONLOSS")) {
		wall_penetration_loss_ = 0;
	} else if (!xmlStrcmp(name, "NOISEPOWER")) {
		noise_power_ = 0;
	} else if (!xmlStrcmp(name, "TOPOCONFIG")) {
		topo_config_ = 0;
	} else if (!xmlStrcmp(name, "TYPE")) {
		net_type_ = 0;
	} else if (!xmlStrcmp(name, "HOMOGENEOUS")) {
		homogeneous_ = 0;
	} else if (!xmlStrcmp(name, "HETEROGENEOUS")) {
		heterogeneous_ = 0;
	} else if (!xmlStrcmp(name, "CELLTYPE")) {
		cell_type_ = 0;
	} else if (!xmlStrcmp(name, "MACROCELL")) {
		macrocell_ = 0;
	} else if (!xmlStrcmp(name, "MICROCELL")) {
		microcell_ = 0;
	} else if (!xmlStrcmp(name, "PICOCELL")) {
		picocell_ = 0;
	} else if (!xmlStrcmp(name, "FEMTOCELL")) {
		femtocell_ = 0;
	} else if (!xmlStrcmp(name, "RELAY")) {
		relay_ = 0;
	} else if (!xmlStrcmp(name, "NUMBEROFRELAYS")) {
		number_of_relays_ = 0;
	} else if (!xmlStrcmp(name, "ENBTOPOLOGY")) {
		eNB_topology_ = 0;
	} else if (!xmlStrcmp(name, "GRID")) {
		grid_ = 0;
	} else if (!xmlStrcmp(name, "HEXAGONAL")) {
		hexagonal_ = 0;
	} else if (!xmlStrcmp(name, "NBCELLS")) {
		number_of_cells_ = 0;
	} else if (!xmlStrcmp(name, "NUMBEROFENB")) {
		number_of_eNB_ = 0;
	} else if (!xmlStrcmp(name, "NBUES")) {
		number_of_UE_ = 0;
	} else if (!xmlStrcmp(name, "RANDOM")) {
		totally_random_ = 0;
	} else if (!xmlStrcmp(name, "INTERENBDISTANCE")) {
		inter_eNB_distance_ = 0;
	} else if (!xmlStrcmp(name, "UEDISTRIBUTION")) {
		UE_distribution_ = 0;
	} else if (!xmlStrcmp(name, "CONCENTRATED")) {
		concentrated_ = 0;
	} else if (!xmlStrcmp(name, "GRIDMAP")) {
		grid_map_ = 0;
	} else if (!xmlStrcmp(name, "INTERBLOCKDISTANCE")) {
		inter_block_distance_ = 0;
	} else if (!xmlStrcmp(name, "SYSTEMBANDWIDTH")) {
		system_bandwidth_ = 0;
	} else if (!xmlStrcmp(name, "UEFREQUENCY")) {
		UE_frequency_ = 0;
	} else if (!xmlStrcmp(name, "MOBILITY")) {
		mobility_ = 0;
	} else if (!xmlStrcmp(name, "MOBILITYTYPE")) {
		mobility_type_ = 0;
	} else if (!xmlStrcmp(name, "FIXED")) {
		fixed_ = 0;
	} else if (!xmlStrcmp(name, "RANDOMWAYPOINT")) {
		random_waypoint_ = 0;
	} else if (!xmlStrcmp(name, "RANDOMWALK")) {
		random_walk_ = 0;
	} else if (!xmlStrcmp(name, "GRIDWALK")) {
		grid_walk_ = 0;
	} else if (!xmlStrcmp(name, "MOVINGDYNAMICS")) {
		moving_dynamics_ = 0;
	} else if (!xmlStrcmp(name, "MINSPEED")) {
		min_speed_ = 0;
	} else if (!xmlStrcmp(name, "MAXSPEED")) {
		max_speed_ = 0;
	} else if (!xmlStrcmp(name, "MINPAUSETIME")) {
		min_pause_time_ = 0;
	} else if (!xmlStrcmp(name, "MAXPAUSETIME")) {
		max_pause_time_ = 0;
	} else if (!xmlStrcmp(name, "APPCONFIG")) {
		app_config_ = 0;
	} else if (!xmlStrcmp(name, "APPTYPE")) {
		app_type_ = 0;
	} else if (!xmlStrcmp(name, "CBR")) {
		cbr_ = 0;
	} else if (!xmlStrcmp(name, "GAMING")) {
		gaming_ = 0;
	} else if (!xmlStrcmp(name, "M2M")) {
		m2m_ = 0;
	} else if (!xmlStrcmp(name, "TRAFFIC")) {
		traffic_ = 0;
	} else if (!xmlStrcmp(name, "TRANSPORTPROTOCOL")) {
		transport_protocol_ = 0;
	} else if (!xmlStrcmp(name, "UDP")) {
		udp_ = 0;
	} else if (!xmlStrcmp(name, "TCP")) {
		tcp_ = 0;
	} else if (!xmlStrcmp(name, "PACKETSIZE")) {
		packet_size_ = 0;
	} else if (!xmlStrcmp(name, "FIXEDVALUE")) {
		fixed_value_ = 0;
	} else if (!xmlStrcmp(name, "UNIFORM")) {
		uniform_ = 0;
	} else if (!xmlStrcmp(name, "MINVALUE")) {
		min_value_ = 0;
	} else if (!xmlStrcmp(name, "MAXVALUE")) {
		max_value_ = 0;
	} else if (!xmlStrcmp(name, "INTERARRIVALTIME")) {
		inter_arrival_time_ = 0;
	} else if (!xmlStrcmp(name, "POISSON")) {
		poisson_ = 0;
	} else if (!xmlStrcmp(name, "EXPECTEDINTERARRIVALTIME")) {
		expected_inter_arrival_time_ = 0;
	} else if (!xmlStrcmp(name, "EMUCONFIG")) {
		emu_config_ = 0;
	} else if (!xmlStrcmp(name, "EMUTIME")) {
		emu_time_ = 0;
	} else if (!xmlStrcmp(name, "PERFORMANCE")) {
		performance_ = 0;
	} else if (!xmlStrcmp(name, "METRIC")) {
		metric_ = 0;
	} else if (!xmlStrcmp(name, "THROUGHPUT")) {
		throughput_ = 0;
	} else if (!xmlStrcmp(name, "LATENCY")) {
		latency_ = 0;
	} else if (!xmlStrcmp(name, "SIGNALLINGOVERHEAD")) {
		signalling_overhead_ = 0;
	} else if (!xmlStrcmp(name, "LAYER")) {
		layer_ = 0;
	} else if (!xmlStrcmp(name, "MAC")) {
		mac_ = 0;
	} else if (!xmlStrcmp(name, "RLC")) {
		rlc_ = 0;
	} else if (!xmlStrcmp(name, "PDCP")) {
		pdcp_ = 0;
	} else if (!xmlStrcmp(name, "LOGEMUL")) {
		log_emu_ = 0;
	} else if (!xmlStrcmp(name, "DEBUG")) {
		debug_ = 0;
	} else if (!xmlStrcmp(name, "INFO")) {
		info_ = 0;
	} else if (!xmlStrcmp(name, "WARNING")) {
		warning_ = 0;
	} else if (!xmlStrcmp(name, "ERROR")) {
		error_ = 0;
	} else if (!xmlStrcmp(name, "PACKETTRACE")) {
		packet_trace_ = 0;
	} else if (!xmlStrcmp(name, "PROFILE")) {
		profile_ = 0;
	}
}

void characters(void *user_data, const xmlChar *ch, int len) { // called once when there is content in each element 
	if (oai_emulation_) {
		if (envi_config_) {
			if (area_) {
				if (x_) {
					oai_emulation.envi_config.area.x = atof(ch);
				} else if (y_) {
					oai_emulation.envi_config.area.y = atof(ch);
				}
			} else if (geography_) {
				oai_emulation.envi_config.geography.selected_option = strndup(ch, len);
			} else if (topography_) {
				oai_emulation.envi_config.topography.selected_option = strndup(ch, len);
			} else if (fading_) {
				if (free_space_propagation_) {
					if (pathloss_exponent_) {
							oai_emulation.envi_config.fading.free_space_propagation.pathloss_exponent = atof(ch);
					} else if (pathloss_0_) {
							oai_emulation.envi_config.fading.free_space_propagation.pathloss_0 = atof(ch);
					}
				} else if (small_scale_) {
					if (rayleigh_) {
						oai_emulation.envi_config.fading.small_scale.selected_option = "rayleigh";
						if (delay_spread_) {
							oai_emulation.envi_config.fading.small_scale.rayleigh.delay_spread = atof(ch);
						}
					} else if (rician_) {
						oai_emulation.envi_config.fading.small_scale.selected_option = "rician";
						if (delay_spread_) {
							oai_emulation.envi_config.fading.small_scale.rician.delay_spread = atof(ch);
						}
					}
				}
			} else if (wall_penetration_loss_) {
				oai_emulation.envi_config.wall_penetration_loss = atof(ch);
			} else if (noise_power_) {
				oai_emulation.envi_config.noise_power = atof(ch);
			}
		} else if (topo_config_) {
			if (net_type_) {
				oai_emulation.topo_config.net_type.selected_option = strndup(ch, len);
			} else if (cell_type_) {
				oai_emulation.topo_config.cell_type.selected_option = strndup(ch, len);
			} else if (relay_) {
				if (number_of_relays_) {
					oai_emulation.topo_config.relay.number_of_relays = atoi(ch);
				}
			} else if (eNB_topology_) {
				if (grid_) {
					oai_emulation.topo_config.eNB_topology.selected_option = "grid";
					if (x_) {
						oai_emulation.topo_config.eNB_topology.grid.x = atoi(ch);
					} else if (y_) {
						oai_emulation.topo_config.eNB_topology.grid.y = atoi(ch);
					}
				} else if (hexagonal_) {
					oai_emulation.topo_config.eNB_topology.selected_option = "hexagonal";
					if (number_of_cells_) {
						oai_emulation.topo_config.eNB_topology.hexagonal.number_of_cells = atoi(ch);
					}
				} else if (totally_random_) {
					oai_emulation.topo_config.eNB_topology.selected_option = "random";
					if (number_of_eNB_) {
						oai_emulation.topo_config.eNB_topology.totally_random.number_of_eNB = atoi(ch);
					}
				}
			} else if (inter_eNB_distance_) {
				oai_emulation.topo_config.inter_eNB_distance = atof(ch);
			} else if (UE_distribution_) {
				if (totally_random_) {
					oai_emulation.topo_config.UE_distribution.selected_option = "random";
				} else if (concentrated_) {
					oai_emulation.topo_config.UE_distribution.selected_option = "concentrated";
				} else if (grid_map_) {
					oai_emulation.topo_config.UE_distribution.selected_option = "grid_map";
					if (inter_block_distance_) {
						oai_emulation.topo_config.UE_distribution.grid_map.inter_block_distance = atof(ch);
					}
				}
			} else if (number_of_UE_) {
				oai_emulation.topo_config.number_of_UE = atoi(ch);
			} else if (system_bandwidth_) {
				oai_emulation.topo_config.system_bandwidth = atof(ch);
			} else if (UE_frequency_) {
				oai_emulation.topo_config.UE_frequency = atof(ch);
			} else if (mobility_) {
				if (mobility_type_) {
					oai_emulation.topo_config.mobility.mobility_type.selected_option = strndup(ch, len);
				} else if (moving_dynamics_) {
					if (min_speed_) {
						oai_emulation.topo_config.mobility.moving_dynamics.min_speed = atof(ch);
					} else if (max_speed_) {
						oai_emulation.topo_config.mobility.moving_dynamics.max_speed = atof(ch);
					} else if (min_pause_time_) {
						oai_emulation.topo_config.mobility.moving_dynamics.min_pause_time = atof(ch);
					} else if (max_pause_time_) {
						oai_emulation.topo_config.mobility.moving_dynamics.max_pause_time = atof(ch);
					}
				}
			}
		} else if (app_config_) {
			if (app_type_) {
				oai_emulation.app_config.app_type.selected_option = strndup(ch, len);
			} else if (traffic_) {
				if (transport_protocol_) {
					oai_emulation.app_config.traffic.transport_protocol.selected_option = strndup(ch, len);
				} else if (packet_size_) {
					if (fixed_) {
						oai_emulation.app_config.traffic.packet_size.selected_option = "fixed";
						if (fixed_value_) {
							oai_emulation.app_config.traffic.packet_size.fixed.fixed_value = atof(ch);
						}
					} else if (uniform_) {
						oai_emulation.app_config.traffic.packet_size.selected_option = "uniform";
						if (min_value_) {
							oai_emulation.app_config.traffic.packet_size.uniform.min_value = atof(ch);
						} else if (max_value_) {
							oai_emulation.app_config.traffic.packet_size.uniform.max_value = atof(ch);
						}
					}
				} else if (inter_arrival_time_) {
					if (fixed_) {
						oai_emulation.app_config.traffic.inter_arrival_time.selected_option = "fixed";
						if (fixed_value_) {
							oai_emulation.app_config.traffic.inter_arrival_time.fixed.fixed_value = atof(ch);
						}
					} else if (uniform_) {
						oai_emulation.app_config.traffic.inter_arrival_time.selected_option = "uniform";
						if (min_value_) {
							oai_emulation.app_config.traffic.inter_arrival_time.uniform.min_value = atof(ch);
						} else if (max_value_) {
							oai_emulation.app_config.traffic.inter_arrival_time.uniform.max_value = atof(ch);
						}
					} else if (poisson_) {
						oai_emulation.app_config.traffic.inter_arrival_time.selected_option = "poisson";
						if (expected_inter_arrival_time_) {
							oai_emulation.app_config.traffic.inter_arrival_time.poisson.expected_inter_arrival_time = atof(ch);
						}
					}
				}
			}
		} else if (emu_config_) {
			if (emu_time_) {
				oai_emulation.emu_config.emu_time = atof(ch);
			} else if (performance_) {
				if (metric_) {
					if (throughput_) {
						oai_emulation.emu_config.performance.metric.throughput = atoi(ch);
					} else if (latency_) {
						oai_emulation.emu_config.performance.metric.latency = atoi(ch);
					} else if (signalling_overhead_) {
						oai_emulation.emu_config.performance.metric.signalling_overhead = atoi(ch);
					}
				} else if (layer_) {
					if (mac_) {
						oai_emulation.emu_config.performance.layer.mac = atoi(ch);
					} else if (rlc_) {
						oai_emulation.emu_config.performance.layer.rlc = atoi(ch);
					} else if (pdcp_) {
						oai_emulation.emu_config.performance.layer.pdcp = atoi(ch);
					}
				} else if (log_emu_) {
					if (debug_) {
						oai_emulation.emu_config.performance.log_emu.debug = atoi(ch);
					} else if (info_) {
						oai_emulation.emu_config.performance.log_emu.info = atoi(ch);
					} else if (warning_) {
						oai_emulation.emu_config.performance.log_emu.warning = atoi(ch);
					} else if (error_) {
						oai_emulation.emu_config.performance.log_emu.error = atoi(ch);
					}
				} else if (packet_trace_) {
					if (mac_) {
						oai_emulation.emu_config.performance.packet_trace.mac = atoi(ch);
					}
				}
			}
		} else if (profile_) {
			oai_emulation.profile = strndup(ch, len);
		}
	}
}

int parse_XML(char src_file[FILENAME_LENGTH_MAX + DIR_LENGTH_MAX]){
	// config the parser, refer to 'http://www.saxproject.org/apidoc/org/xml/sax/ContentHandler.html'
	xmlSAXHandler sax_handler = { 0 }; // a Simple API for XML (SAX) handler 
	sax_handler.startDocument = start_document;
	sax_handler.endDocument = end_document;
	sax_handler.startElement = start_element;
	sax_handler.endElement = end_element;
	sax_handler.characters = characters;

	// Parsing the XML file
	if (xmlSAXUserParseFile(&sax_handler, NULL, src_file) != 0) {
		LOG_E(OCG, "An error occurs while parsing the configuration file!\n");
		return MODULE_ERROR;
	}
	LOG_I(OCG, "The configuration file is parsed\n");
	return MODULE_OK;
}
