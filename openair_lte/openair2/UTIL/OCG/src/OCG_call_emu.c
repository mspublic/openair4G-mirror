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

/*! \file OCG_call_emu.c
* \brief Call the emulator
* \author Lusheng Wang
* \date 2011
* \version 0.1
* \company Eurecom
* \email: lusheng.wang@eurecom.fr
* \note
* \warning
*/

/*--- INCLUDES ---------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "../include/OCG.h"
#include "../include/OCG_call_emu.h"
#include "../include/log.h"
/*----------------------------------------------------------------------------*/

OAI_Emulation_ oai_emulation_;

int call_emu(char dst_dir[DIR_LENGTH_MAX]) {


	////////// print the configuration 
	FILE *file;
	char dst_file[DIR_LENGTH_MAX] = "";
	strcat(dst_file, dst_dir);
	strcat(dst_file, "emulation_result.txt");
	file = fopen(dst_file,"w");
/*
	fprintf(file, "\nConfiguration by the user is\n\n");
	fprintf(file, "- envi_config:area:   x   y = %lf  %lf\n", oai_emulation_.envi_config_.area_.x_, oai_emulation_.envi_config_.area_.y_);
	
	if (oai_emulation_.emu_config_.performance_.layer_.mac_ == 1) {
		set_comp_log(MAC, LOG_INFO, LOG_DEF_ONLINE);
	}

	fprintf(file, "             :geography:   %s \n", oai_emulation_.envi_config_.geography_.selected_option_);
	fprintf(file, "             :topography:   %s \n", oai_emulation_.envi_config_.topography_.selected_option_);
	fprintf(file, "             :fading:free_space_propagation:   pathloss_exponent   pathloss_0 = %lf   %lf   \n", oai_emulation_.envi_config_.fading_.free_space_propagation_.pathloss_exponent_, oai_emulation_.envi_config_.fading_.free_space_propagation_.pathloss_0_);
	fprintf(file, "             :fading:   the selected small_scale model is ");
	if (!strcmp(oai_emulation_.envi_config_.fading_.small_scale_.selected_option_, "rayleigh")) {
		fprintf(file, "Rayleigh ");
		fprintf(file, "with delay spread = %lf\n", oai_emulation_.envi_config_.fading_.small_scale_.rayleigh_.delay_spread_);
	} else if (!strcmp(oai_emulation_.envi_config_.fading_.small_scale_.selected_option_, "rician")) {
		fprintf(file, "Rician ");
		fprintf(file, "with delay spread = %lf\n", oai_emulation_.envi_config_.fading_.small_scale_.rician_.delay_spread_);
	}
	fprintf(file, "             :wall_penetration_loss = %d\n", oai_emulation_.envi_config_.wall_penetration_loss_);
	fprintf(file, "             :noise_power = %lf   \n\n", oai_emulation_.envi_config_.noise_power_);
	fprintf(file, "- net_config :net_type: %s\n", oai_emulation_.topo_config_.net_type_.selected_option_);
	fprintf(file, "             :cell_type:   %s\n", oai_emulation_.topo_config_.cell_type_.selected_option_);
	fprintf(file, "             :number of relays = %d\n", oai_emulation_.topo_config_.relay_.number_of_relays_);
	fprintf(file, "             :eNB_topology is ");
	if (!strcmp(oai_emulation_.topo_config_.eNB_topology_.selected_option_, "grid")) {
		fprintf(file, "Grid with %d * %d \n", oai_emulation_.topo_config_.eNB_topology_.grid_.x_, oai_emulation_.topo_config_.eNB_topology_.grid_.y_);
	} else if (!strcmp(oai_emulation_.topo_config_.eNB_topology_.selected_option_, "hexagonal")) {
		fprintf(file, "Hexagonal with %d cells\n", oai_emulation_.topo_config_.eNB_topology_.hexagonal_.number_of_cells_);
	} else if (!strcmp(oai_emulation_.topo_config_.eNB_topology_.selected_option_, "random")) {
		fprintf(file, "Random\n");
	}
	fprintf(file, "             :inter_eNB_distance = %lf\n", oai_emulation_.topo_config_.inter_eNB_distance_);
	fprintf(file, "             :UE_distribution:   %s\n", oai_emulation_.topo_config_.UE_distribution_.selected_option_);
	fprintf(file, "             :system_bandwidth = %lf\n", oai_emulation_.topo_config_.system_bandwidth_);
	fprintf(file, "             :UE_frequency = %lf\n", oai_emulation_.topo_config_.UE_frequency_);
	fprintf(file, "             :mobility:mobility_type   %s \n", oai_emulation_.topo_config_.mobility_.mobility_type_.selected_option_);
	fprintf(file, "             :mobility:moving_dynamics   min_speed   max_speed   min_pause_time   max_pause_time = %lf %lf %lf %lf\n\n", oai_emulation_.topo_config_.mobility_.moving_dynamics_.min_speed_, oai_emulation_.topo_config_.mobility_.moving_dynamics_.max_speed_, oai_emulation_.topo_config_.mobility_.moving_dynamics_.min_pause_time_, oai_emulation_.topo_config_.mobility_.moving_dynamics_.max_pause_time_);
	fprintf(file, "- term_config:app_type:   %s \n", oai_emulation_.app_config_.app_type_.selected_option_);
	fprintf(file, "             :traffic:transport_protocol:   %s \n", oai_emulation_.app_config_.traffic_.transport_protocol_.selected_option_);
	fprintf(file, "             :traffic:   the packet size is ");
	if (!strcmp(oai_emulation_.app_config_.traffic_.packet_size_.selected_option_, "fixed")) {
		fprintf(file, "Fixed with value = %lf\n", oai_emulation_.app_config_.traffic_.packet_size_.fixed_.fixed_value_);
	} else if (!strcmp(oai_emulation_.app_config_.traffic_.packet_size_.selected_option_, "uniform")) {
		fprintf(file, "Uniform with min = %lf max = %lf \n", oai_emulation_.app_config_.traffic_.packet_size_.uniform_.min_value_, oai_emulation_.app_config_.traffic_.packet_size_.uniform_.max_value_);
	}
	fprintf(file, "             :traffic:   the inter_arrival_time is ");
	if (!strcmp(oai_emulation_.app_config_.traffic_.packet_size_.selected_option_, "fixed")) {
		fprintf(file, "Fixed with value = %lf\n\n", oai_emulation_.app_config_.traffic_.inter_arrival_time_.fixed_.fixed_value_);
	} else if (!strcmp(oai_emulation_.app_config_.traffic_.packet_size_.selected_option_, "uniform")) {
		fprintf(file, "Uniform with min = %lf max = %lf\n", oai_emulation_.app_config_.traffic_.inter_arrival_time_.uniform_.min_value_, oai_emulation_.app_config_.traffic_.inter_arrival_time_.uniform_.max_value_);
	} else if (!strcmp(oai_emulation_.app_config_.traffic_.packet_size_.selected_option_, "poisson")) {
		fprintf(file, "Poisson with lambda = %lf\n", oai_emulation_.app_config_.traffic_.inter_arrival_time_.poisson_.expected_inter_arrival_time_);
	}
	fprintf(file, "- emu_config :emu_time = %lf\n", oai_emulation_.emu_config_.emu_time_);
	fprintf(file, "             :performance:metric:   throughput   latency   signalling_overhead = %d %d %d \n", oai_emulation_.emu_config_.performance_.metric_.throughput_, oai_emulation_.emu_config_.performance_.metric_.latency_, oai_emulation_.emu_config_.performance_.metric_.signalling_overhead_);
	fprintf(file, "             :performance:layer:    mac   rlc   pdcp = %d %d %d \n", oai_emulation_.emu_config_.performance_.layer_.mac_, oai_emulation_.emu_config_.performance_.layer_.rlc_, oai_emulation_.emu_config_.performance_.layer_.pdcp_);
	fprintf(file, "             :performance:log_emu:	debug   info   warning   error = %d %d %d %d\n", oai_emulation_.emu_config_.performance_.log_emu_.debug_, oai_emulation_.emu_config_.performance_.log_emu_.info_, oai_emulation_.emu_config_.performance_.log_emu_.warning_, oai_emulation_.emu_config_.performance_.log_emu_.error_);
	fprintf(file, "             :performance:packet_trace:   mac = %d\n\n", oai_emulation_.emu_config_.performance_.packet_trace_.mac_);

	fprintf(file, "- profile = %s\n\n", oai_emulation_.profile_);
*/	
	fclose(file);

	LOG_I(OCG, "Emulation finished\n");
	return MODULE_OK;
}
