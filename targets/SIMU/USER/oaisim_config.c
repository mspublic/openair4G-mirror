#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "oaisim_config.h"
#include "OCG.h"
#include "OCG_extern.h"
#include "UTIL/OMG/omg.h"


mapping log_level_names[] =
{
    {"emerg", LOG_EMERG},
    {"alert", LOG_ALERT},
    {"crit", LOG_CRIT},
    {"err", LOG_ERR},
    {"warn", LOG_WARNING},
    {"notice", LOG_NOTICE},
    {"info", LOG_INFO},
    {"debug", LOG_DEBUG},
    {"trace", LOG_TRACE},
    {NULL, -1}
};

mapping omg_model_names[] =
{
    {"STATIC", STATIC},
    {"RWP", RWP},
    {"RWALK", RWALK},
    {"TRACE", TRACE},
    {"MAX_NUM_MOB_TYPES", MAX_NUM_MOB_TYPES},
    {NULL, -1}
};


void init_oai_emulation() {

	oai_emulation.environment_system_config.fading.large_scale.selected_option = "free_space";
	oai_emulation.environment_system_config.fading.free_space_model_parameters.pathloss_exponent = 2.0;
	oai_emulation.environment_system_config.fading.free_space_model_parameters.pathloss_0_dB = -50;
	oai_emulation.environment_system_config.fading.small_scale.selected_option = "Rayleigh8";
	oai_emulation.environment_system_config.fading.ricean_8tap.rice_factor_dB = 0;
	oai_emulation.environment_system_config.fading.shadowing.decorrelation_distance_m = 0;
	oai_emulation.environment_system_config.fading.shadowing.variance_dB = 0;
	oai_emulation.environment_system_config.fading.shadowing.inter_site_correlation = 0;
	oai_emulation.environment_system_config.antenna.eNB_antenna.number_of_sectors = 3;
	oai_emulation.environment_system_config.antenna.eNB_antenna.beam_width_dB = 1.13;
	oai_emulation.environment_system_config.antenna.eNB_antenna.alpha_rad[1] = 0;
	oai_emulation.environment_system_config.antenna.eNB_antenna.alpha_rad[2] = 0;
	oai_emulation.environment_system_config.antenna.eNB_antenna.alpha_rad[3] = 0;
	oai_emulation.environment_system_config.antenna.eNB_antenna.antenna_gain_dBi = 16;
	oai_emulation.environment_system_config.antenna.eNB_antenna.tx_power_dBm = 40;
	oai_emulation.environment_system_config.antenna.eNB_antenna.rx_noise_level_dB = 5;
	oai_emulation.environment_system_config.antenna.eNB_antenna.antenna_orientation_degree[1] = 0;
	oai_emulation.environment_system_config.antenna.eNB_antenna.antenna_orientation_degree[2] = 0;
	oai_emulation.environment_system_config.antenna.eNB_antenna.antenna_orientation_degree[3] = 0;
	oai_emulation.environment_system_config.antenna.UE_antenna.antenna_gain_dBi = 0;
	oai_emulation.environment_system_config.antenna.UE_antenna.tx_power_dBm = 20;
	oai_emulation.environment_system_config.antenna.UE_antenna.rx_noise_level_dB = 1;
	oai_emulation.environment_system_config.wall_penetration_loss_dB = 5;
	oai_emulation.environment_system_config.system_bandwidth_MB = 7.68;
	oai_emulation.environment_system_config.system_frequency_GHz = 1.9;


	oai_emulation.topology_config.area.x_km = 100;
	oai_emulation.topology_config.area.y_km = 100;
	oai_emulation.topology_config.network_type.selected_option = "homogeneous";
	oai_emulation.topology_config.cell_type.selected_option = "macrocell";
	oai_emulation.topology_config.relay.number_of_relays = 0;
	oai_emulation.topology_config.mobility.UE_mobility.UE_mobility_type.selected_option = "STATIC";
	oai_emulation.topology_config.mobility.UE_mobility.grid_walk.grid_map.horizontal_grid = 1;
	oai_emulation.topology_config.mobility.UE_mobility.grid_walk.grid_map.vertical_grid = 1;
	oai_emulation.topology_config.mobility.UE_mobility.grid_walk.grid_trip_type.selected_option = "random_destination";
	oai_emulation.topology_config.mobility.UE_mobility.UE_initial_distribution.selected_option = "random";
	oai_emulation.topology_config.mobility.UE_mobility.random_UE_distribution.number_of_nodes = 1;
	oai_emulation.topology_config.mobility.UE_mobility.concentrated_UE_distribution.number_of_nodes = 1;
	oai_emulation.topology_config.mobility.UE_mobility.grid_UE_distribution.random_grid.number_of_nodes = 1;
	oai_emulation.topology_config.mobility.UE_mobility.grid_UE_distribution.border_grid.number_of_nodes = 1;
	oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.min_speed_mps = 0.1;
	oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.max_speed_mps = 20.0;
	oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.min_sleep_ms = 0.1;
	oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.max_sleep_ms = 5.0;
	oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.min_journey_time_ms = 0.1;
	oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.max_journey_time_ms = 10.0;
	oai_emulation.topology_config.mobility.eNB_mobility.eNB_mobility_type.selected_option = "STATIC";
	oai_emulation.topology_config.mobility.eNB_mobility.eNB_initial_distribution.selected_option = "random";
	oai_emulation.topology_config.mobility.eNB_mobility.random_eNB_distribution.number_of_cells = 1;
	oai_emulation.topology_config.mobility.eNB_mobility.hexagonal_eNB_distribution.number_of_cells = 1;
	oai_emulation.topology_config.mobility.eNB_mobility.hexagonal_eNB_distribution.inter_eNB_distance_km = 1;
	oai_emulation.topology_config.mobility.eNB_mobility.grid_eNB_distribution.number_of_grid_x = 1;
	oai_emulation.topology_config.mobility.eNB_mobility.grid_eNB_distribution.number_of_grid_y = 1;

	oai_emulation.application_config.application_type.selected_option = "cbr";
	oai_emulation.application_config.traffic.transport_protocol.selected_option = "udp";
	oai_emulation.application_config.traffic.packet_size.selected_option = "fixed";
	oai_emulation.application_config.traffic.fixed_packet_size.fixed_value_byte = 10;
	oai_emulation.application_config.traffic.uniform_packet_size.min_value_byte = 5;
	oai_emulation.application_config.traffic.uniform_packet_size.max_value_byte = 15;
	oai_emulation.application_config.traffic.inter_arrival_time.selected_option = "fixed";
	oai_emulation.application_config.traffic.fixed_inter_arrival_time.fixed_value_ms = 10;
	oai_emulation.application_config.traffic.uniform_inter_arrival_time.min_value_ms = 5;
	oai_emulation.application_config.traffic.uniform_inter_arrival_time.max_value_ms = 15;
	oai_emulation.application_config.traffic.poisson_inter_arrival_time.expected_inter_arrival_time_ms = 10;
					
	oai_emulation.emulation_config.emulation_time_ms = 0;
	oai_emulation.emulation_config.performance.metrics.throughput = 0;
	oai_emulation.emulation_config.performance.metrics.latency = 0;
	oai_emulation.emulation_config.performance.metrics.signalling_overhead = 0;
	oai_emulation.emulation_config.performance.layer.mac = 0;
	oai_emulation.emulation_config.performance.layer.rlc = 0;
	oai_emulation.emulation_config.performance.layer.pdcp = 0;
	oai_emulation.emulation_config.performance.log_emu.debug = 0;
	oai_emulation.emulation_config.performance.log_emu.info = 0;
	oai_emulation.emulation_config.performance.log_emu.warning = 0;
	oai_emulation.emulation_config.performance.log_emu.error = 0;
	oai_emulation.emulation_config.performance.packet_trace.mac = 0;
	oai_emulation.emulation_config.seed.selected_option = "oai_seed";
	oai_emulation.emulation_config.user_seed.seed_value = 1;

	oai_emulation.info.ocg_ok = 0;


   // phy related params :
    oai_emulation.info.n_frames=0xffff; // number of frames simulated by default
    oai_emulation.info.n_frames_flag=0; // if set, then let the emulation goes to infinity

//status 
  oai_emulation.info.is_primary_master=0;
  oai_emulation.info.master_list=0;
  oai_emulation.info.nb_ue_remote=0;
  oai_emulation.info.nb_enb_remote=0;
  oai_emulation.info.first_ue_local=0;
  oai_emulation.info.first_enb_local=0;
  oai_emulation.info.master_id=0;
  oai_emulation.info.nb_ue_local= 1;//default 1 UE 
  oai_emulation.info.nb_enb_local= 1;//default 1 eNB
  oai_emulation.info.ethernet_flag=0;
  oai_emulation.info.ocm_enabled=1;// flag c
  oai_emulation.info.ocg_enabled=0;// flag c
  oai_emulation.info.opt_enabled=0; // P flag
  oai_emulation.info.omg_model_enb=STATIC; //default to static mobility model
  oai_emulation.info.omg_model_ue=STATIC; //default to static mobility model
  oai_emulation.info.omg_model_ue_current=STATIC; //default to static mobility model
  oai_emulation.info.otg_enabled=0;// T flag
  oai_emulation.info.frame = 0; // frame counter of emulation 
  oai_emulation.info.time = 0; // time of emulation 
  oai_emulation.info.seed = time(NULL); // time-based random seed , , included in ocg report

   oai_emulation.info.nb_master =0;
   oai_emulation.info.ethernet_id=0;
   oai_emulation.info.multicast_group=0;
   strcpy(&oai_emulation.info.global_log_level, "trace");
	
    
    oai_emulation.info.frame_type=1;
    oai_emulation.info.tdd_config=3;
    oai_emulation.info.extended_prefix_flag=0;
    oai_emulation.info.N_RB_DL=25;
     oai_emulation.info.transmission_mode=2;

    oai_emulation.profile = "EURECOM";
	
}


void oaisim_config(char *g_log_level) {

  // init log gen first
  olg_config(g_log_level);
  // init ocg if enabled, otherwise take the params form the init_oai_emulation()
 //  and command line options given by the user
  if (oai_emulation.info.ocg_enabled == 1){ // activate OCG: xml-based scenario parser
    OCG_main(oai_emulation.info.local_server);// eurecom or portable
     if (oai_emulation.info.ocg_ok != 1) {
      LOG_E(OCG, "Error found by OCG; emulation not launched.\n");
      LOG_E(OCG, "Please find more information in the OCG_report.xml located at %s.\n", oai_emulation.info.output_path);
      exit (-1);
     }
   } 
    // init other comps
    ocg_config_env();// mobility gen
    ocg_config_topo(); // packet tracer using wireshark
    ocg_config_app(); // packet generator 
    ocg_config_emu(); // packet generator 


  
}

int olg_config(char * g_log_level) {

 //initialize the log generator 
  logInit(map_str_to_int(log_level_names, g_log_level));
  set_glog(LOG_DEBUG, LOG_MED); //g_glog
  set_comp_log(OCG,  LOG_INFO, LOG_LOW, 10);
  set_comp_log(OMG,  LOG_INFO, LOG_LOW, 10);
  set_comp_log(EMU,  LOG_INFO, LOG_LOW, 10);
  
  LOG_T(LOG,"global log level is set to %s \n",g_log_level );
  return 1; 
}

int ocg_config_env() {
// int func related to channel desc from oaisim.c could be moved here
return 1;
}
int ocg_config_topo() {

	// omg
	init_omg_global_params();

	// setup params for openair mobility generator
	//common params

	omg_param_list.min_X = 0;
	omg_param_list.max_X = oai_emulation.topology_config.area.x_km;
	omg_param_list.min_Y = 0;
	omg_param_list.max_Y = oai_emulation.topology_config.area.y_km;
	// init values
	omg_param_list.min_speed = 0.1;
	omg_param_list.max_speed = 20.0;
	omg_param_list.min_journey_time = 0.1;
	omg_param_list.max_journey_time = 10.0;
	omg_param_list.min_azimuth = 0; // ???
	omg_param_list.max_azimuth = 360; // ???
	omg_param_list.min_sleep = 0.1;
	omg_param_list.max_sleep = 8.0;
	

	// init OMG for eNB
	if (!strcmp(oai_emulation.topology_config.mobility.eNB_mobility.eNB_mobility_type.selected_option, "STATIC")) {
		oai_emulation.info.omg_model_enb = STATIC;
	} else if (!strcmp(oai_emulation.topology_config.mobility.eNB_mobility.eNB_mobility_type.selected_option, "RWP")) {
		oai_emulation.info.omg_model_enb = RWP; // set eNB to be random waypoint
	}
	omg_param_list.mobility_type = oai_emulation.info.omg_model_enb; 
		
	omg_param_list.nodes_type = eNB;  //eNB
	omg_param_list.nodes = oai_emulation.info.nb_enb_local;
 	omg_param_list.seed = oai_emulation.info.seed; // specific seed for enb and ue to avoid node overlapping
	
	omg_param_list.mobility_file = (char*) malloc(256);
	sprintf(omg_param_list.mobility_file,"%s/UTIL/OMG/mobility.txt",getenv("OPENAIR2_DIR")); // default trace-driven mobility file

	// at this moment, we use the above moving dynamics for mobile eNB
	if (omg_param_list.nodes >0 ) 
	  init_mobility_generator(omg_param_list);
	
	// init OMG for UE
	// input of OMG: STATIC: 0, RWP: 1, RWALK 2, or TRACE 3
	
	if ((oai_emulation.info.omg_model_ue = map_str_to_int(omg_model_names, oai_emulation.topology_config.mobility.UE_mobility.UE_mobility_type.selected_option))== -1)
	  oai_emulation.info.omg_model_ue = STATIC; 
	omg_param_list.mobility_type    = oai_emulation.info.omg_model_ue; 

		
	omg_param_list.mobility_type = oai_emulation.info.omg_model_ue;

	omg_param_list.nodes_type = UE;//UE
	omg_param_list.nodes = oai_emulation.info.nb_ue_local;
	omg_param_list.seed = oai_emulation.info.seed; // specific seed for enb and ue to avoid node overlapping

	omg_param_list.min_speed = (oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.min_speed_mps == 0) ? 0.1 : oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.min_speed_mps;
	omg_param_list.max_speed = (oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.max_speed_mps == 0) ? 0.1 : oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.max_speed_mps;

	omg_param_list.min_journey_time = (oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.min_journey_time_ms == 0) ? 0.1 : oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.min_journey_time_ms;
	omg_param_list.max_journey_time = (oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.max_journey_time_ms == 0) ? 0.1 : oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.max_journey_time_ms;

	omg_param_list.min_azimuth = 0.1; // wait for advanced OSD
	omg_param_list.max_azimuth = 360;

	omg_param_list.min_sleep = (oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.min_sleep_ms == 0) ? 0.1 : oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.min_sleep_ms;
	omg_param_list.max_sleep = (oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.max_sleep_ms == 0) ? 0.1 : oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.max_sleep_ms;
	
	omg_param_list.mobility_file = (char*) malloc(256);
	sprintf(omg_param_list.mobility_file,"%s/UTIL/OMG/mobility.txt",getenv("OPENAIR2_DIR")); // default trace-driven mobility file

	if (omg_param_list.nodes >0 ) 
	  init_mobility_generator(omg_param_list);
	


return 1;
}


int ocg_config_app(){
return 1;
}


int ocg_config_emu(){


    if (oai_emulation.emulation_config.emulation_time_ms != 0) {
	oai_emulation.info.n_frames  =  (int) oai_emulation.emulation_config.emulation_time_ms / 10; // configure the number of frame
	oai_emulation.info.n_frames_flag = 1;
	 LOG_I(OCG, "number of frames in emulation is set to %d\n", oai_emulation.info.n_frames);
    } else
	 LOG_I(OCG, "number of frames in emulation is set to infinity\n");

    if (!strcmp(oai_emulation.emulation_config.seed.selected_option, "user_seed")) {
	oai_emulation.info.seed = oai_emulation.emulation_config.user_seed.seed_value;
     } // otherwise, keep the default value 

      /* : TODO
       LOG_I(OCG, "OPT output file directory = %s\n", oai_emulation.info.output_path);
      Init_OPT(0,"outfile.dump","127.0.0.1",1234); // Init_OPT(2, oai_emulation.info.output_path, NULL, 0);*/
	

  return 1;  

}


