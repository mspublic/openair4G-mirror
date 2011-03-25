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
#include "OCG.h"
#include "OCG_call_emu.h"
//#include "../include/log.h"
/*----------------------------------------------------------------------------*/

OAI_Emulation oai_emulation;

int call_emu(char dst_dir[DIR_LENGTH_MAX]) {


	////////// print the configuration 
	FILE *file;
	char dst_file[DIR_LENGTH_MAX] = "";
	strcat(dst_file, dst_dir);
	strcat(dst_file, "emulation_result.txt");
	file = fopen(dst_file,"w");
	system("../../../openair1/SIMULATION/LTE_PHY_L2/physim --help");
/*
	fprintf(file, "\nConfiguration by the user is\n\n");
	fprintf(file, "- envi_config:area:   x   y = %lf  %lf\n", oai_emulation.envi_config.area.x_, oai_emulation.envi_config.area.y_);
	
	if (oai_emulation.emu_config.performance.layer.mac == 1) {
		set_comp_log(MAC, LOG_INFO, LOG_DEF_ONLINE);
	}

	fprintf(file, "             :geography:   %s \n", oai_emulation.envi_config.geography.selected_option_);
	fprintf(file, "             :topography:   %s \n", oai_emulation.envi_config.topography.selected_option_);
	fprintf(file, "             :fading:free_space_propagation:   pathloss_exponent   pathloss_0 = %lf   %lf   \n", oai_emulation.envi_config.fading.free_space_propagation.pathloss_exponent_, oai_emulation.envi_config.fading.free_space_propagation.pathloss_0_);
	fprintf(file, "             :fading:   the selected small_scale model is ");
	if (!strcmp(oai_emulation.envi_config.fading.small_scale.selected_option_, "rayleigh")) {
		fprintf(file, "Rayleigh ");
		fprintf(file, "with delay spread = %lf\n", oai_emulation.envi_config.fading.small_scale.rayleigh.delay_spread_);
	} else if (!strcmp(oai_emulation.envi_config.fading.small_scale.selected_option_, "rician")) {
		fprintf(file, "Rician ");
		fprintf(file, "with delay spread = %lf\n", oai_emulation.envi_config.fading.small_scale.rician.delay_spread_);
	}
	fprintf(file, "             :wall_penetration_loss = %d\n", oai_emulation.envi_config.wall_penetration_loss_);
	fprintf(file, "             :noise_power = %lf   \n\n", oai_emulation.envi_config.noise_power_);
	fprintf(file, "- net_config :net_type: %s\n", oai_emulation.topo_config.net_type.selected_option_);
	fprintf(file, "             :cell_type:   %s\n", oai_emulation.topo_config.cell_type.selected_option_);
	fprintf(file, "             :number of relays = %d\n", oai_emulation.topo_config.relay.number_of_relays_);
	fprintf(file, "             :eNB_topology is ");
	if (!strcmp(oai_emulation.topo_config.eNB_topology.selected_option_, "grid")) {
		fprintf(file, "Grid with %d * %d \n", oai_emulation.topo_config.eNB_topology.grid.x_, oai_emulation.topo_config.eNB_topology.grid.y_);
	} else if (!strcmp(oai_emulation.topo_config.eNB_topology.selected_option_, "hexagonal")) {
		fprintf(file, "Hexagonal with %d cells\n", oai_emulation.topo_config.eNB_topology.hexagonal.number_of_cells_);
	} else if (!strcmp(oai_emulation.topo_config.eNB_topology.selected_option_, "random")) {
		fprintf(file, "Random\n");
	}
	fprintf(file, "             :inter_eNB_distance = %lf\n", oai_emulation.topo_config.inter_eNB_distance_);
	fprintf(file, "             :UE_distribution:   %s\n", oai_emulation.topo_config.UE_distribution.selected_option_);
	fprintf(file, "             :system_bandwidth = %lf\n", oai_emulation.topo_config.system_bandwidth_);
	fprintf(file, "             :UE_frequency = %lf\n", oai_emulation.topo_config.UE_frequency_);
	fprintf(file, "             :mobility:mobility_type   %s \n", oai_emulation.topo_config.mobility.mobility_type.selected_option_);
	fprintf(file, "             :mobility:moving_dynamics   min_speed   max_speed   min_pause_time   max_pause_time = %lf %lf %lf %lf\n\n", oai_emulation.topo_config.mobility.moving_dynamics.min_speed_, oai_emulation.topo_config.mobility.moving_dynamics.max_speed_, oai_emulation.topo_config.mobility.moving_dynamics.min_pause_time_, oai_emulation.topo_config.mobility.moving_dynamics.max_pause_time_);
	fprintf(file, "- term_config:app_type:   %s \n", oai_emulation.app_config.app_type.selected_option_);
	fprintf(file, "             :traffic:transport_protocol:   %s \n", oai_emulation.app_config.traffic.transport_protocol.selected_option_);
	fprintf(file, "             :traffic:   the packet size is ");
	if (!strcmp(oai_emulation.app_config.traffic.packet_size.selected_option_, "fixed")) {
		fprintf(file, "Fixed with value = %lf\n", oai_emulation.app_config.traffic.packet_size.fixed.fixed_value_);
	} else if (!strcmp(oai_emulation.app_config.traffic.packet_size.selected_option_, "uniform")) {
		fprintf(file, "Uniform with min = %lf max = %lf \n", oai_emulation.app_config.traffic.packet_size.uniform.min_value_, oai_emulation.app_config.traffic.packet_size.uniform.max_value_);
	}
	fprintf(file, "             :traffic:   the inter_arrival_time is ");
	if (!strcmp(oai_emulation.app_config.traffic.packet_size.selected_option_, "fixed")) {
		fprintf(file, "Fixed with value = %lf\n\n", oai_emulation.app_config.traffic.inter_arrival_time.fixed.fixed_value_);
	} else if (!strcmp(oai_emulation.app_config.traffic.packet_size.selected_option_, "uniform")) {
		fprintf(file, "Uniform with min = %lf max = %lf\n", oai_emulation.app_config.traffic.inter_arrival_time.uniform.min_value_, oai_emulation.app_config.traffic.inter_arrival_time.uniform.max_value_);
	} else if (!strcmp(oai_emulation.app_config.traffic.packet_size.selected_option_, "poisson")) {
		fprintf(file, "Poisson with lambda = %lf\n", oai_emulation.app_config.traffic.inter_arrival_time.poisson.expected_inter_arrival_time_);
	}
	fprintf(file, "- emu_config :emu_time = %lf\n", oai_emulation.emu_config.emu_time_);
	fprintf(file, "             :performance:metric:   throughput   latency   signalling_overhead = %d %d %d \n", oai_emulation.emu_config.performance.metric.throughput_, oai_emulation.emu_config.performance.metric.latency_, oai_emulation.emu_config.performance.metric.signalling_overhead_);
	fprintf(file, "             :performance:layer:    mac   rlc   pdcp = %d %d %d \n", oai_emulation.emu_config.performance.layer.mac_, oai_emulation.emu_config.performance.layer.rlc_, oai_emulation.emu_config.performance.layer.pdcp_);
	fprintf(file, "             :performance:log_emu:	debug   info   warning   error = %d %d %d %d\n", oai_emulation.emu_config.performance.log_emu.debug_, oai_emulation.emu_config.performance.log_emu.info_, oai_emulation.emu_config.performance.log_emu.warning_, oai_emulation.emu_config.performance.log_emu.error_);
	fprintf(file, "             :performance:packet_trace:   mac = %d\n\n", oai_emulation.emu_config.performance.packet_trace.mac_);

	fprintf(file, "- profile = %s\n\n", oai_emulation.profile_);
*/	
	fclose(file);

	LOG_I(OCG, "Emulation finished\n");
	return MODULE_OK;
}
