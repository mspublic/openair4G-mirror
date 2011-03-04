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
#include "../include/OCG.h"
#include "../include/OCG_parse_XML.h"
#include "../include/log.h"
/*----------------------------------------------------------------------------*/

OAI_Emulation_ oai_emulation_;

void start_document(void *user_data) {
	//printf("Start parsing ............ \n");
}

void end_document(void *user_data) {
	//printf("End parsing .\n");
}

void start_element(void *user_data, const xmlChar *name, const xmlChar **attrs) { // called once at the beginning of each element 
	if (!xmlStrcmp(name, "OPENAIRINTERFACE")) {
		oai_emulation = 1;
	} else if (!xmlStrcmp(name, "ENVICONFIG")) {
		envi_config = 1;
	} else if (!xmlStrcmp(name, "AREA")) {
		area = 1;
	} else if (!xmlStrcmp(name, "X")) {
		x = 1;
	} else if (!xmlStrcmp(name, "Y")) {
		y = 1;
	} else if (!xmlStrcmp(name, "GEOGRAPHY")) {
		geography = 1;
	} else if (!xmlStrcmp(name, "HOME")) {
		home = 1;
	} else if (!xmlStrcmp(name, "URBAN")) {
		urban = 1;
	} else if (!xmlStrcmp(name, "RURAL")) {
		rural = 1;
	} else if (!xmlStrcmp(name, "TOPOGRAPHY")) {
		topography = 1;
	} else if (!xmlStrcmp(name, "FLAT")) {
		flat = 1;
	} else if (!xmlStrcmp(name, "OBSTRUCTED")) {
		obstructed = 1;
	} else if (!xmlStrcmp(name, "HILLY")) {
		hilly = 1;
	} else if (!xmlStrcmp(name, "FADING")) {
		fading = 1;
	} else if (!xmlStrcmp(name, "FREESPACEPROPAGATION")) {
		free_space_propagation = 1;
	} else if (!xmlStrcmp(name, "PATHLOSSEXPONENT")) {
		pathloss_exponent = 1;
	} else if (!xmlStrcmp(name, "PATHLOSS0")) {
		pathloss_0 = 1;
	} else if (!xmlStrcmp(name, "SMALLSCALE")) {
		small_scale = 1;
	} else if (!xmlStrcmp(name, "RAYLEIGH")) {
		rayleigh = 1;
	} else if (!xmlStrcmp(name, "DELAYSPREAD")) {
		delay_spread = 1;
	} else if (!xmlStrcmp(name, "RICIAN")) {
		rician = 1;
	} else if (!xmlStrcmp(name, "WALLPENETRATIONLOSS")) {
		wall_penetration_loss = 1;
	} else if (!xmlStrcmp(name, "NOISEPOWER")) {
		noise_power = 1;
	} else if (!xmlStrcmp(name, "TOPOCONFIG")) {
		topo_config = 1;
	} else if (!xmlStrcmp(name, "TYPE")) {
		net_type = 1;
	} else if (!xmlStrcmp(name, "HOMOGENEOUS")) {
		homogeneous = 1;
	} else if (!xmlStrcmp(name, "HETEROGENEOUS")) {
		heterogeneous = 1;
	} else if (!xmlStrcmp(name, "CELLTYPE")) {
		cell_type = 1;
	} else if (!xmlStrcmp(name, "MACROCELL")) {
		macrocell = 1;
	} else if (!xmlStrcmp(name, "MICROCELL")) {
		microcell = 1;
	} else if (!xmlStrcmp(name, "PICOCELL")) {
		picocell = 1;
	} else if (!xmlStrcmp(name, "FEMTOCELL")) {
		femtocell = 1;
	} else if (!xmlStrcmp(name, "RELAY")) {
		relay = 1;
	} else if (!xmlStrcmp(name, "NUMBEROFRELAYS")) {
		number_of_relays = 1;
	} else if (!xmlStrcmp(name, "ENBTOPOLOGY")) {
		eNB_topology = 1;
	} else if (!xmlStrcmp(name, "GRID")) {
		grid = 1;
	} else if (!xmlStrcmp(name, "HEXAGONAL")) {
		hexagonal = 1;
	} else if (!xmlStrcmp(name, "NUMBEROFCELLS")) {
		number_of_cells = 1;
	} else if (!xmlStrcmp(name, "RANDOM")) {
		totally_random = 1;
	} else if (!xmlStrcmp(name, "INTERENBDISTANCE")) {
		inter_eNB_distance = 1;
	} else if (!xmlStrcmp(name, "UEDISTRIBUTION")) {
		UE_distribution = 1;
	} else if (!xmlStrcmp(name, "CONCENTRATED")) {
		concentrated = 1;
	} else if (!xmlStrcmp(name, "GRIDMAP")) {
		grid_map = 1;
	} else if (!xmlStrcmp(name, "INTERBLOCKDISTANCE")) {
		inter_block_distance = 1;
	} else if (!xmlStrcmp(name, "SYSTEMBANDWIDTH")) {
		system_bandwidth = 1;
	} else if (!xmlStrcmp(name, "UEFREQUENCY")) {
		UE_frequency = 1;
	} else if (!xmlStrcmp(name, "MOBILITY")) {
		mobility = 1;
	} else if (!xmlStrcmp(name, "MOBILITYTYPE")) {
		mobility_type = 1;
	} else if (!xmlStrcmp(name, "FIXED")) {
		fixed = 1;
	} else if (!xmlStrcmp(name, "RANDOMWAYPOINT")) {
		random_waypoint = 1;
	} else if (!xmlStrcmp(name, "RANDOMWALK")) {
		random_walk = 1;
	} else if (!xmlStrcmp(name, "GRIDWALK")) {
		grid_walk = 1;
	} else if (!xmlStrcmp(name, "MOVINGDYNAMICS")) {
		moving_dynamics = 1;
	} else if (!xmlStrcmp(name, "MINSPEED")) {
		min_speed = 1;
	} else if (!xmlStrcmp(name, "MAXSPEED")) {
		max_speed = 1;
	} else if (!xmlStrcmp(name, "MINPAUSETIME")) {
		min_pause_time = 1;
	} else if (!xmlStrcmp(name, "MAXPAUSETIME")) {
		max_pause_time = 1;
	} else if (!xmlStrcmp(name, "APPCONFIG")) {
		app_config = 1;
	} else if (!xmlStrcmp(name, "APPTYPE")) {
		app_type = 1;
	} else if (!xmlStrcmp(name, "CBR")) {
		cbr = 1;
	} else if (!xmlStrcmp(name, "GAMING")) {
		gaming = 1;
	} else if (!xmlStrcmp(name, "M2M")) {
		m2m = 1;
	} else if (!xmlStrcmp(name, "TRAFFIC")) {
		traffic = 1;
	} else if (!xmlStrcmp(name, "TRANSPORTPROTOCOL")) {
		transport_protocol = 1;
	} else if (!xmlStrcmp(name, "UDP")) {
		udp = 1;
	} else if (!xmlStrcmp(name, "TCP")) {
		tcp = 1;
	} else if (!xmlStrcmp(name, "PACKETSIZE")) {
		packet_size = 1;
	} else if (!xmlStrcmp(name, "FIXEDVALUE")) {
		fixed_value = 1;
	} else if (!xmlStrcmp(name, "UNIFORM")) {
		uniform = 1;
	} else if (!xmlStrcmp(name, "MINVALUE")) {
		min_value = 1;
	} else if (!xmlStrcmp(name, "MAXVALUE")) {
		max_value = 1;
	} else if (!xmlStrcmp(name, "INTERARRIVALTIME")) {
		inter_arrival_time = 1;
	} else if (!xmlStrcmp(name, "POISSON")) {
		poisson = 1;
	} else if (!xmlStrcmp(name, "EXPECTEDINTERARRIVALTIME")) {
		expected_inter_arrival_time = 1;
	} else if (!xmlStrcmp(name, "EMUCONFIG")) {
		emu_config = 1;
	} else if (!xmlStrcmp(name, "EMUTIME")) {
		emu_time = 1;
	} else if (!xmlStrcmp(name, "PERFORMANCE")) {
		performance = 1;
	} else if (!xmlStrcmp(name, "METRIC")) {
		metric = 1;
	} else if (!xmlStrcmp(name, "THROUGHPUT")) {
		throughput = 1;
	} else if (!xmlStrcmp(name, "LATENCY")) {
		latency = 1;
	} else if (!xmlStrcmp(name, "SIGNALLINGOVERHEAD")) {
		signalling_overhead = 1;
	} else if (!xmlStrcmp(name, "LAYER")) {
		layer = 1;
	} else if (!xmlStrcmp(name, "MAC")) {
		mac = 1;
	} else if (!xmlStrcmp(name, "RLC")) {
		rlc = 1;
	} else if (!xmlStrcmp(name, "PDCP")) {
		pdcp = 1;
	} else if (!xmlStrcmp(name, "LOGEMUL")) {
		log_emu = 1;
	} else if (!xmlStrcmp(name, "DEBUG")) {
		debug = 1;
	} else if (!xmlStrcmp(name, "INFO")) {
		info = 1;
	} else if (!xmlStrcmp(name, "WARNING")) {
		warning = 1;
	} else if (!xmlStrcmp(name, "ERROR")) {
		error = 1;
	} else if (!xmlStrcmp(name, "PACKETTRACE")) {
		packet_trace = 1;
	} else if (!xmlStrcmp(name, "PROFILE")) {
		profile = 1;
	} else {
		LOG_W(OCG, "One element could not be parsed : unknown element name '%s'\n", name);
	}
}

void end_element(void *user_data, const xmlChar *name) { // called once at the end of each element 
	if (!xmlStrcmp(name, "OPENAIRINTERFACE")) {
		oai_emulation = 0;
	} else if (!xmlStrcmp(name, "ENVICONFIG")) {
		envi_config = 0;
	} else if (!xmlStrcmp(name, "AREA")) {
		area = 0;
	} else if (!xmlStrcmp(name, "X")) {
		x = 0;
	} else if (!xmlStrcmp(name, "Y")) {
		y = 0;
	} else if (!xmlStrcmp(name, "GEOGRAPHY")) {
		geography = 0;
	} else if (!xmlStrcmp(name, "HOME")) {
		home = 0;
	} else if (!xmlStrcmp(name, "URBAN")) {
		urban = 0;
	} else if (!xmlStrcmp(name, "RURAL")) {
		rural = 0;
	} else if (!xmlStrcmp(name, "TOPOGRAPHY")) {
		topography = 0;
	} else if (!xmlStrcmp(name, "FLAT")) {
		flat = 0;
	} else if (!xmlStrcmp(name, "OBSTRUCTED")) {
		obstructed = 0;
	} else if (!xmlStrcmp(name, "HILLY")) {
		hilly = 0;
	} else if (!xmlStrcmp(name, "FADING")) {
		fading = 0;
	} else if (!xmlStrcmp(name, "FREESPACEPROPAGATION")) {
		free_space_propagation = 0;
	} else if (!xmlStrcmp(name, "PATHLOSSEXPONENT")) {
		pathloss_exponent = 0;
	} else if (!xmlStrcmp(name, "PATHLOSS0")) {
		pathloss_0 = 0;
	} else if (!xmlStrcmp(name, "SMALLSCALE")) {
		small_scale = 0;
	} else if (!xmlStrcmp(name, "RAYLEIGH")) {
		rayleigh = 0;
	} else if (!xmlStrcmp(name, "DELAYSPREAD")) {
		delay_spread = 0;
	} else if (!xmlStrcmp(name, "RICIAN")) {
		rician = 0;
	} else if (!xmlStrcmp(name, "WALLPENETRATIONLOSS")) {
		wall_penetration_loss = 0;
	} else if (!xmlStrcmp(name, "NOISEPOWER")) {
		noise_power = 0;
	} else if (!xmlStrcmp(name, "TOPOCONFIG")) {
		topo_config = 0;
	} else if (!xmlStrcmp(name, "TYPE")) {
		net_type = 0;
	} else if (!xmlStrcmp(name, "HOMOGENEOUS")) {
		homogeneous = 0;
	} else if (!xmlStrcmp(name, "HETEROGENEOUS")) {
		heterogeneous = 0;
	} else if (!xmlStrcmp(name, "CELLTYPE")) {
		cell_type = 0;
	} else if (!xmlStrcmp(name, "MACROCELL")) {
		macrocell = 0;
	} else if (!xmlStrcmp(name, "MICROCELL")) {
		microcell = 0;
	} else if (!xmlStrcmp(name, "PICOCELL")) {
		picocell = 0;
	} else if (!xmlStrcmp(name, "FEMTOCELL")) {
		femtocell = 0;
	} else if (!xmlStrcmp(name, "RELAY")) {
		relay = 0;
	} else if (!xmlStrcmp(name, "NUMBEROFRELAYS")) {
		number_of_relays = 0;
	} else if (!xmlStrcmp(name, "ENBTOPOLOGY")) {
		eNB_topology = 0;
	} else if (!xmlStrcmp(name, "GRID")) {
		grid = 0;
	} else if (!xmlStrcmp(name, "HEXAGONAL")) {
		hexagonal = 0;
	} else if (!xmlStrcmp(name, "NUMBEROFCELLS")) {
		number_of_cells = 0;
	} else if (!xmlStrcmp(name, "RANDOM")) {
		totally_random = 0;
	} else if (!xmlStrcmp(name, "INTERENBDISTANCE")) {
		inter_eNB_distance = 0;
	} else if (!xmlStrcmp(name, "UEDISTRIBUTION")) {
		UE_distribution = 0;
	} else if (!xmlStrcmp(name, "CONCENTRATED")) {
		concentrated = 0;
	} else if (!xmlStrcmp(name, "GRIDMAP")) {
		grid_map = 0;
	} else if (!xmlStrcmp(name, "INTERBLOCKDISTANCE")) {
		inter_block_distance = 0;
	} else if (!xmlStrcmp(name, "SYSTEMBANDWIDTH")) {
		system_bandwidth = 0;
	} else if (!xmlStrcmp(name, "UEFREQUENCY")) {
		UE_frequency = 0;
	} else if (!xmlStrcmp(name, "MOBILITY")) {
		mobility = 0;
	} else if (!xmlStrcmp(name, "MOBILITYTYPE")) {
		mobility_type = 0;
	} else if (!xmlStrcmp(name, "FIXED")) {
		fixed = 0;
	} else if (!xmlStrcmp(name, "RANDOMWAYPOINT")) {
		random_waypoint = 0;
	} else if (!xmlStrcmp(name, "RANDOMWALK")) {
		random_walk = 0;
	} else if (!xmlStrcmp(name, "GRIDWALK")) {
		grid_walk = 0;
	} else if (!xmlStrcmp(name, "MOVINGDYNAMICS")) {
		moving_dynamics = 0;
	} else if (!xmlStrcmp(name, "MINSPEED")) {
		min_speed = 0;
	} else if (!xmlStrcmp(name, "MAXSPEED")) {
		max_speed = 0;
	} else if (!xmlStrcmp(name, "MINPAUSETIME")) {
		min_pause_time = 0;
	} else if (!xmlStrcmp(name, "MAXPAUSETIME")) {
		max_pause_time = 0;
	} else if (!xmlStrcmp(name, "APPCONFIG")) {
		app_config = 0;
	} else if (!xmlStrcmp(name, "APPTYPE")) {
		app_type = 0;
	} else if (!xmlStrcmp(name, "CBR")) {
		cbr = 0;
	} else if (!xmlStrcmp(name, "GAMING")) {
		gaming = 0;
	} else if (!xmlStrcmp(name, "M2M")) {
		m2m = 0;
	} else if (!xmlStrcmp(name, "TRAFFIC")) {
		traffic = 0;
	} else if (!xmlStrcmp(name, "TRANSPORTPROTOCOL")) {
		transport_protocol = 0;
	} else if (!xmlStrcmp(name, "UDP")) {
		udp = 0;
	} else if (!xmlStrcmp(name, "TCP")) {
		tcp = 0;
	} else if (!xmlStrcmp(name, "PACKETSIZE")) {
		packet_size = 0;
	} else if (!xmlStrcmp(name, "FIXEDVALUE")) {
		fixed_value = 0;
	} else if (!xmlStrcmp(name, "UNIFORM")) {
		uniform = 0;
	} else if (!xmlStrcmp(name, "MINVALUE")) {
		min_value = 0;
	} else if (!xmlStrcmp(name, "MAXVALUE")) {
		max_value = 0;
	} else if (!xmlStrcmp(name, "INTERARRIVALTIME")) {
		inter_arrival_time = 0;
	} else if (!xmlStrcmp(name, "POISSON")) {
		poisson = 0;
	} else if (!xmlStrcmp(name, "EXPECTEDINTERARRIVALTIME")) {
		expected_inter_arrival_time = 0;
	} else if (!xmlStrcmp(name, "EMUCONFIG")) {
		emu_config = 0;
	} else if (!xmlStrcmp(name, "EMUTIME")) {
		emu_time = 0;
	} else if (!xmlStrcmp(name, "PERFORMANCE")) {
		performance = 0;
	} else if (!xmlStrcmp(name, "METRIC")) {
		metric = 0;
	} else if (!xmlStrcmp(name, "THROUGHPUT")) {
		throughput = 0;
	} else if (!xmlStrcmp(name, "LATENCY")) {
		latency = 0;
	} else if (!xmlStrcmp(name, "SIGNALLINGOVERHEAD")) {
		signalling_overhead = 0;
	} else if (!xmlStrcmp(name, "LAYER")) {
		layer = 0;
	} else if (!xmlStrcmp(name, "MAC")) {
		mac = 0;
	} else if (!xmlStrcmp(name, "RLC")) {
		rlc = 0;
	} else if (!xmlStrcmp(name, "PDCP")) {
		pdcp = 0;
	} else if (!xmlStrcmp(name, "LOGEMUL")) {
		log_emu = 0;
	} else if (!xmlStrcmp(name, "DEBUG")) {
		debug = 0;
	} else if (!xmlStrcmp(name, "INFO")) {
		info = 0;
	} else if (!xmlStrcmp(name, "WARNING")) {
		warning = 0;
	} else if (!xmlStrcmp(name, "ERROR")) {
		error = 0;
	} else if (!xmlStrcmp(name, "PACKETTRACE")) {
		packet_trace = 0;
	} else if (!xmlStrcmp(name, "PROFILE")) {
		profile = 0;
	}
}

void characters(void *user_data, const xmlChar *ch, int len) { // called once when there is content in each element 
	if (oai_emulation) {
		if (envi_config) {
			if (area) {
				if (x) {
					oai_emulation_.envi_config_.area_.x_ = atof(ch);
				} else if (y) {
					oai_emulation_.envi_config_.area_.y_ = atof(ch);
				}
			} else if (geography) {
				oai_emulation_.envi_config_.geography_.selected_option_ = strndup(ch, len);
			} else if (topography) {
				oai_emulation_.envi_config_.topography_.selected_option_ = strndup(ch, len);
			} else if (fading) {
				if (free_space_propagation) {
					if (pathloss_exponent) {
							oai_emulation_.envi_config_.fading_.free_space_propagation_.pathloss_exponent_ = atof(ch);
					} else if (pathloss_0) {
							oai_emulation_.envi_config_.fading_.free_space_propagation_.pathloss_0_ = atof(ch);
					}
				} else if (small_scale) {
					if (rayleigh) {
						oai_emulation_.envi_config_.fading_.small_scale_.selected_option_ = "rayleigh";
						if (delay_spread) {
							oai_emulation_.envi_config_.fading_.small_scale_.rayleigh_.delay_spread_ = atof(ch);
						}
					} else if (rician) {
						oai_emulation_.envi_config_.fading_.small_scale_.selected_option_ = "rician";
						if (delay_spread) {
							oai_emulation_.envi_config_.fading_.small_scale_.rician_.delay_spread_ = atof(ch);
						}
					}
				}
			} else if (wall_penetration_loss) {
				oai_emulation_.envi_config_.wall_penetration_loss_ = atof(ch);
			} else if (noise_power) {
				oai_emulation_.envi_config_.noise_power_ = atof(ch);
			}
		} else if (topo_config) {
			if (net_type) {
				oai_emulation_.topo_config_.net_type_.selected_option_ = strndup(ch, len);
			} else if (cell_type) {
				oai_emulation_.topo_config_.cell_type_.selected_option_ = strndup(ch, len);
			} else if (relay) {
				if (number_of_relays) {
					oai_emulation_.topo_config_.relay_.number_of_relays_ = atoi(ch);
				}
			} else if (eNB_topology) {
				if (grid) {
					oai_emulation_.topo_config_.eNB_topology_.selected_option_ = "grid";
					if (x) {
						oai_emulation_.topo_config_.eNB_topology_.grid_.x_ = atof(ch);
					} else if (y) {
						oai_emulation_.topo_config_.eNB_topology_.grid_.y_ = atof(ch);
					}
				} else if (hexagonal) {
					oai_emulation_.topo_config_.eNB_topology_.selected_option_ = "hexagonal";
					if (number_of_cells) {
						oai_emulation_.topo_config_.eNB_topology_.hexagonal_.number_of_cells_ = atoi(ch);
					}
				} else if (totally_random) {
					oai_emulation_.topo_config_.eNB_topology_.selected_option_ = "random";
				}
			} else if (inter_eNB_distance) {
				oai_emulation_.topo_config_.inter_eNB_distance_ = atof(ch);
			} else if (UE_distribution) {
				if (totally_random) {
					oai_emulation_.topo_config_.UE_distribution_.selected_option_ = "random";
				} else if (concentrated) {
					oai_emulation_.topo_config_.UE_distribution_.selected_option_ = "concentrated";
				} else if (grid_map) {
					oai_emulation_.topo_config_.UE_distribution_.selected_option_ = "grid_map";
					if (inter_block_distance) {
						oai_emulation_.topo_config_.UE_distribution_.grid_map_.inter_block_distance_ = atof(ch);
					}
				}
			} else if (system_bandwidth) {
				oai_emulation_.topo_config_.system_bandwidth_ = atof(ch);
			} else if (UE_frequency) {
				oai_emulation_.topo_config_.UE_frequency_ = atof(ch);
			} else if (mobility) {
				if (mobility_type) {
					oai_emulation_.topo_config_.mobility_.mobility_type_.selected_option_ = strndup(ch, len);
				} else if (moving_dynamics) {
					if (min_speed) {
						oai_emulation_.topo_config_.mobility_.moving_dynamics_.min_speed_ = atof(ch);
					} else if (max_speed) {
						oai_emulation_.topo_config_.mobility_.moving_dynamics_.max_speed_ = atof(ch);
					} else if (min_pause_time) {
						oai_emulation_.topo_config_.mobility_.moving_dynamics_.min_pause_time_ = atof(ch);
					} else if (max_pause_time) {
						oai_emulation_.topo_config_.mobility_.moving_dynamics_.max_pause_time_ = atof(ch);
					}
				}
			}
		} else if (app_config) {
			if (app_type) {
				oai_emulation_.app_config_.app_type_.selected_option_ = strndup(ch, len);
			} else if (traffic) {
				if (transport_protocol) {
					oai_emulation_.app_config_.traffic_.transport_protocol_.selected_option_ = strndup(ch, len);
				} else if (packet_size) {
					if (fixed) {
						oai_emulation_.app_config_.traffic_.packet_size_.selected_option_ = "fixed";
						if (fixed_value) {
							oai_emulation_.app_config_.traffic_.packet_size_.fixed_.fixed_value_ = atof(ch);
						}
					} else if (uniform) {
						oai_emulation_.app_config_.traffic_.packet_size_.selected_option_ = "uniform";
						if (min_value) {
							oai_emulation_.app_config_.traffic_.packet_size_.uniform_.min_value_ = atof(ch);
						} else if (max_value) {
							oai_emulation_.app_config_.traffic_.packet_size_.uniform_.max_value_ = atof(ch);
						}
					}
				} else if (inter_arrival_time) {
					if (fixed) {
						oai_emulation_.app_config_.traffic_.inter_arrival_time_.selected_option_ = "fixed";
						if (fixed_value) {
							oai_emulation_.app_config_.traffic_.inter_arrival_time_.fixed_.fixed_value_ = atof(ch);
						}
					} else if (uniform) {
						oai_emulation_.app_config_.traffic_.inter_arrival_time_.selected_option_ = "uniform";
						if (min_value) {
							oai_emulation_.app_config_.traffic_.inter_arrival_time_.uniform_.min_value_ = atof(ch);
						} else if (max_value) {
							oai_emulation_.app_config_.traffic_.inter_arrival_time_.uniform_.max_value_ = atof(ch);
						}
					} else if (poisson) {
						oai_emulation_.app_config_.traffic_.inter_arrival_time_.selected_option_ = "poisson";
						if (expected_inter_arrival_time) {
							oai_emulation_.app_config_.traffic_.inter_arrival_time_.poisson_.expected_inter_arrival_time_ = atof(ch);
						}
					}
				}
			}
		} else if (emu_config) {
			if (emu_time) {
				oai_emulation_.emu_config_.emu_time_ = atof(ch);
			} else if (performance) {
				if (metric) {
					if (throughput) {
						oai_emulation_.emu_config_.performance_.metric_.throughput_ = atoi(ch);
					} else if (latency) {
						oai_emulation_.emu_config_.performance_.metric_.latency_ = atoi(ch);
					} else if (signalling_overhead) {
						oai_emulation_.emu_config_.performance_.metric_.signalling_overhead_ = atoi(ch);
					}
				} else if (layer) {
					if (mac) {
						oai_emulation_.emu_config_.performance_.layer_.mac_ = atoi(ch);
					} else if (rlc) {
						oai_emulation_.emu_config_.performance_.layer_.rlc_ = atoi(ch);
					} else if (pdcp) {
						oai_emulation_.emu_config_.performance_.layer_.pdcp_ = atoi(ch);
					}
				} else if (log_emu) {
					if (debug) {
						oai_emulation_.emu_config_.performance_.log_emu_.debug_ = atoi(ch);
					} else if (info) {
						oai_emulation_.emu_config_.performance_.log_emu_.info_ = atoi(ch);
					} else if (warning) {
						oai_emulation_.emu_config_.performance_.log_emu_.warning_ = atoi(ch);
					} else if (error) {
						oai_emulation_.emu_config_.performance_.log_emu_.error_ = atoi(ch);
					}
				} else if (packet_trace) {
					if (mac) {
						oai_emulation_.emu_config_.performance_.packet_trace_.mac_ = atoi(ch);
					}
				}
			}
		} else if (profile) {
			oai_emulation_.profile_ = strndup(ch, len);
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
