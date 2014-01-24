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
#include "UTIL/OTG/otg_tx.h"
#include "UTIL/OTG/otg.h"
#include "UTIL/OTG/otg_vars.h"


mapping log_level_names[] =
{
    {"emerg", LOG_EMERG},
    {"alert", LOG_ALERT},
    {"crit", LOG_CRIT},
    {"error", LOG_ERR},
    {"warn", LOG_WARNING},
    {"notice", LOG_NOTICE},
    {"info", LOG_INFO},
    {"debug", LOG_DEBUG},
    {"trace", LOG_TRACE},
    {NULL, -1}
};
mapping log_verbosity_names[] =
{
    {"low", 0x4},
    {"medium", 0x14},
    {"high", 0x34}, 
    {"full", 0x174},
    {NULL, -1}
};
mapping omg_model_names[] =
{
    {"STATIC", STATIC},
    {"RWP", RWP},
    {"RWALK", RWALK},
    {"TRACE", TRACE},
    {"SUMO", SUMO},
    {"MAX_NUM_MOB_TYPES", MAX_NUM_MOB_TYPES},
    {NULL, -1}
};

mapping otg_app_type_names[] =
{
    {"no_predefined_traffic", 0},
    {"scbr", 1},
    {"mcbr", 2},
    {"bcbr", 3},
    {"m2m_AP", 4},
    {"m2m_BR", 5},
    {"gaming_OA", 6},
    {"gaming_TF", 7},
    {"full_buffer", 8},
    {NULL, -1}
};

mapping otg_transport_protocol_names[] =
{
    {"no_proto", 0},
    {"udp", 1},
    {"tcp", 2},
    {NULL, -1}
};

mapping otg_ip_version_names[] =
{
    {"no_ip", 0},
    {"ipv4", 1},
    {"ipv6", 2},
    {NULL, -1}
};

mapping otg_distribution_names[] =
{
    {"no_customized_traffic", 0},
    {"uniform", 1},
    {"gaussian", 2},
    {"exponential", 3},
    {"poisson", 4},
    {"fixed", 5}, 
    {"weibull", 6},
    {"pareto", 7},
    {"gamma", 8},
    {"cauchy",9}, 
    {NULL, -1}
};



void init_oai_emulation() {

  int i;
  
	oai_emulation.environment_system_config.fading.large_scale.selected_option = "free_space";
	oai_emulation.environment_system_config.fading.free_space_model_parameters.pathloss_exponent = 2.0;
	oai_emulation.environment_system_config.fading.free_space_model_parameters.pathloss_0_dB = -50;
	oai_emulation.environment_system_config.fading.small_scale.selected_option = "AWGN";
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


	oai_emulation.topology_config.area.x_m = 500;
	oai_emulation.topology_config.area.y_m = 500;
	oai_emulation.topology_config.network_type.selected_option = "homogeneous";
	oai_emulation.topology_config.cell_type.selected_option = "macrocell";
	oai_emulation.topology_config.relay.number_of_relays = 0;
	oai_emulation.topology_config.mobility.UE_mobility.UE_mobility_type.selected_option = "RWP";
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
	oai_emulation.topology_config.mobility.UE_mobility.trace_config.trace_mobility_file = (char*) malloc(256);
	sprintf(oai_emulation.topology_config.mobility.UE_mobility.trace_config.trace_mobility_file,"%s/UTIL/OMG/mobility.txt",getenv("OPENAIR2_DIR"));
	oai_emulation.topology_config.mobility.UE_mobility.sumo_config.command = (char*) malloc(20);
	sprintf(oai_emulation.topology_config.mobility.UE_mobility.sumo_config.command,"sumo");
	oai_emulation.topology_config.mobility.UE_mobility.sumo_config.file = (char*) malloc(256);
	sprintf(oai_emulation.topology_config.mobility.UE_mobility.sumo_config.file,"%s/UTIL/OMG/SUMO/SCENARIOS/scen.sumo.cfg",getenv("OPENAIR2_DIR"));
	oai_emulation.topology_config.mobility.UE_mobility.sumo_config.start=0;
	oai_emulation.topology_config.mobility.UE_mobility.sumo_config.end=0;
	oai_emulation.topology_config.mobility.UE_mobility.sumo_config.step=1; //  1000ms
	oai_emulation.topology_config.mobility.UE_mobility.sumo_config.hip = (char*) malloc(40);
	sprintf(oai_emulation.topology_config.mobility.UE_mobility.sumo_config.hip,"127.0.1.1");
	oai_emulation.topology_config.mobility.UE_mobility.sumo_config.hport = 8883;
	
	for (i = 0; i < NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX; i++) {
		oai_emulation.application_config.predefined_traffic.source_id[i] = "1:10"; 
		oai_emulation.application_config.predefined_traffic.application_type[i] = "no_predefined_traffic";
		oai_emulation.application_config.predefined_traffic.destination_id[i] = 0;
	
		oai_emulation.application_config.customized_traffic.source_id[i] = "1";
		oai_emulation.application_config.customized_traffic.destination_id[i] = "2";
		oai_emulation.application_config.customized_traffic.transport_protocol[i] = "udp";
		oai_emulation.application_config.customized_traffic.ip_version[i] = "ipv4";
		oai_emulation.application_config.customized_traffic.idt_dist[i] = "uniform";
		oai_emulation.application_config.customized_traffic.idt_min_ms[i] = 100;
		oai_emulation.application_config.customized_traffic.idt_max_ms[i] = 1000;
		oai_emulation.application_config.customized_traffic.idt_standard_deviation[i] = 1;
		oai_emulation.application_config.customized_traffic.idt_lambda[i] = 1;
		oai_emulation.application_config.customized_traffic.size_dist[i] = "uniform";
		oai_emulation.application_config.customized_traffic.size_min_byte[i] = 200;
		oai_emulation.application_config.customized_traffic.size_max_byte[i] = 500;
		oai_emulation.application_config.customized_traffic.size_standard_deviation[i] = 1;
		oai_emulation.application_config.customized_traffic.size_lambda[i] = 1;
		oai_emulation.application_config.customized_traffic.stream[i] = 1;
		oai_emulation.application_config.customized_traffic.destination_port[i] = 8080;
	}


	oai_emulation.emulation_config.emulation_time_ms = 0;
	oai_emulation.emulation_config.performance_metrics.throughput = 0;
	oai_emulation.emulation_config.performance_metrics.latency = 0;
	oai_emulation.emulation_config.performance_metrics.loss_rate = 0;
	oai_emulation.emulation_config.layer.phy = 0;
	oai_emulation.emulation_config.layer.mac = 0;
	oai_emulation.emulation_config.layer.rlc = 0;
	oai_emulation.emulation_config.layer.pdcp = 0;
	oai_emulation.emulation_config.layer.rrc = 0;
	oai_emulation.emulation_config.layer.omg = 0;
	oai_emulation.emulation_config.layer.otg = 0;
	oai_emulation.emulation_config.layer.emu = 1;

	oai_emulation.emulation_config.log_emu.level = "debug";
	oai_emulation.emulation_config.log_emu.verbosity = "low";
	oai_emulation.emulation_config.log_emu.interval = 1;
	oai_emulation.emulation_config.packet_trace.enabled = 0;
	oai_emulation.emulation_config.seed.value = 0; // 0 means randomly generated by OAI

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
  oai_emulation.info.ocm_enabled=1;// flag ?
  oai_emulation.info.ocg_enabled=0;// flag c
  oai_emulation.info.otg_enabled=0;// flag T
  oai_emulation.info.opt_enabled=0; // P flag
  oai_emulation.info.cli_enabled=0;// I flag
  oai_emulation.info.omv_enabled =0; // v flag
  oai_emulation.info.omg_model_enb=STATIC; //default to static mobility model
  oai_emulation.info.omg_model_ue=STATIC; //default to static mobility model
  oai_emulation.info.omg_model_ue_current=STATIC; //default to static mobility model
  oai_emulation.info.otg_enabled=0;// T flag
  oai_emulation.info.frame = 0; // frame counter of emulation 
  oai_emulation.info.time_s = 0; // time of emulation  
  oai_emulation.info.time_ms = 0; // time of emulation 
  oai_emulation.info.seed = time(NULL); // time-based random seed , , included in ocg report

  oai_emulation.info.cli_num_enb= NUMBER_OF_eNB_MAX;
  oai_emulation.info.cli_num_ue= NUMBER_OF_UE_MAX;
 
  for (i=0; i < oai_emulation.info.cli_num_enb; i++)
    oai_emulation.info.cli_start_enb[i]=1;    
  for (i=0; i < oai_emulation.info.cli_num_ue; i++)
    oai_emulation.info.cli_start_ue[i]=1;
  
   oai_emulation.info.nb_master =0;
   oai_emulation.info.ethernet_id=0;
   oai_emulation.info.multicast_group=0;
   oai_emulation.info.g_log_level= LOG_TRACE;
   oai_emulation.info.g_log_verbosity= LOG_LOW;
    
    oai_emulation.info.frame_type=1;
    oai_emulation.info.tdd_config=3;
    oai_emulation.info.tdd_config_S=0;
    oai_emulation.info.extended_prefix_flag=0;
    oai_emulation.info.N_RB_DL=25;
    oai_emulation.info.transmission_mode=1;

    oai_emulation.profile = "EURECOM";
	
}


void oaisim_config() {

  // init log gen first
 //initialize the log generator 
  logInit();

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
  olg_config();
  ocg_config_emu(); 
  ocg_config_env();// mobility gen
  ocg_config_topo(); // packet tracer using wireshark
 	// if T is set or ocg enabled 
  if (oai_emulation.info.otg_enabled ) {
    ocg_config_app(); // packet generator 
}
     
}

int olg_config() {
  int comp;
  oai_emulation.info.g_log_level = map_str_to_int(log_level_names, oai_emulation.emulation_config.log_emu.level);
  oai_emulation.info.g_log_verbosity = map_str_to_int(log_verbosity_names, oai_emulation.emulation_config.log_emu.verbosity);
  LOG_N(EMU,"global log level is set to (%s,%d) with vebosity (%s, 0x%x) and frequency %d\n", 
	oai_emulation.emulation_config.log_emu.level, 
	oai_emulation.info.g_log_level,
	oai_emulation.emulation_config.log_emu.verbosity,
	oai_emulation.info.g_log_verbosity,
	oai_emulation.emulation_config.log_emu.interval );
  set_glog(oai_emulation.info.g_log_level, oai_emulation.info.g_log_verbosity ); //g_glog
  // component, log level, log interval
  for (comp = PHY; comp < MAX_LOG_COMPONENTS ; comp++)
    set_comp_log(comp,  oai_emulation.info.g_log_level, oai_emulation.info.g_log_verbosity, oai_emulation.emulation_config.log_emu.interval);
  // if perf eval then reset the otg log level
  /*
  set_log(OCG,  LOG_DEBUG, 1);  
  set_log(EMU,  LOG_INFO,  20);
  set_log(MAC,  LOG_DEBUG, 1);  
  set_log(RLC,  LOG_TRACE, 1);  
  set_log(PHY,  LOG_DEBUG, 1);  
  set_log(PDCP, LOG_TRACE, 1);  
  set_log(RRC,  LOG_DEBUG, 1);  
  set_log(OCM,  LOG_INFO, 20);  
  set_log(OTG,  LOG_INFO, 1);  
  */
  // set_comp_log(MAC, LOG_TRACE, LOG_FULL,1);
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
	omg_param_list.max_X = oai_emulation.topology_config.area.x_m;
	omg_param_list.min_Y = 0;
	omg_param_list.max_Y = oai_emulation.topology_config.area.y_m;
	// init values
	omg_param_list.min_speed = 0.1;
	omg_param_list.max_speed = 20.0;
	omg_param_list.min_journey_time = 0.1;
	omg_param_list.max_journey_time = 10.0;
	omg_param_list.min_azimuth = 0; // ???
	omg_param_list.max_azimuth = 360; // ???
	omg_param_list.min_sleep = 0.1;
	omg_param_list.max_sleep = 8.0;
	

	// init OMG for eNBs
	if (!strcmp(oai_emulation.topology_config.mobility.eNB_mobility.eNB_mobility_type.selected_option, "RWP")) {
	  oai_emulation.info.omg_model_enb = RWP; // set eNB to be random waypoint
	} else { // if (!strcmp(oai_emulation.topology_config.mobility.eNB_mobility.eNB_mobility_type.selected_option, "STATIC")) 
	  oai_emulation.info.omg_model_enb = STATIC;
	}
	omg_param_list.mobility_type = oai_emulation.info.omg_model_enb; 
		
	omg_param_list.nodes_type = eNB;  //eNB
	omg_param_list.nodes = oai_emulation.info.nb_enb_local;
 	omg_param_list.seed = oai_emulation.info.seed; // specific seed for enb and ue to avoid node overlapping
	
	// at this moment, we use the above moving dynamics for mobile eNB
	if (omg_param_list.nodes >0 ) 
	  init_mobility_generator(omg_param_list);

	// init OMG for UE
	// input of OMG: STATIC: 0, RWP: 1, RWALK 2, or TRACE 3, or SUMO
	
	if ((oai_emulation.info.omg_model_ue = map_str_to_int(omg_model_names, oai_emulation.topology_config.mobility.UE_mobility.UE_mobility_type.selected_option))== -1)
	  oai_emulation.info.omg_model_ue = STATIC; 
	omg_param_list.mobility_type    = oai_emulation.info.omg_model_ue; 
	omg_param_list.nodes_type = UE;//UE
	omg_param_list.nodes = oai_emulation.info.nb_ue_local;
	omg_param_list.seed = oai_emulation.info.seed + oai_emulation.info.nb_ue_local; //fixme: specific seed for enb and ue to avoid node overlapping

	omg_param_list.min_speed = (oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.min_speed_mps == 0) ? 0.1 : oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.min_speed_mps;
	omg_param_list.max_speed = (oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.max_speed_mps == 0) ? 0.1 : oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.max_speed_mps;

	omg_param_list.min_journey_time = (oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.min_journey_time_ms == 0) ? 0.1 : oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.min_journey_time_ms;
	omg_param_list.max_journey_time = (oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.max_journey_time_ms == 0) ? 0.1 : oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.max_journey_time_ms;

	omg_param_list.min_azimuth = 0.1; // wait for advanced OSD
	omg_param_list.max_azimuth = 360;

	omg_param_list.min_sleep = (oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.min_sleep_ms == 0) ? 0.1 : oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.min_sleep_ms;
	omg_param_list.max_sleep = (oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.max_sleep_ms == 0) ? 0.1 : oai_emulation.topology_config.mobility.UE_mobility.UE_moving_dynamics.max_sleep_ms;
	
	if (oai_emulation.info.omg_model_ue == TRACE) {
	  omg_param_list.mobility_file = (char*) malloc(256); // default trace-driven mobility file
	  sprintf(omg_param_list.mobility_file,"%s",oai_emulation.topology_config.mobility.UE_mobility.trace_config.trace_mobility_file); 
	} else if (oai_emulation.info.omg_model_ue == SUMO){
	  omg_param_list.sumo_command = (char*) malloc(20);
	  sprintf(omg_param_list.sumo_command, "%s", oai_emulation.topology_config.mobility.UE_mobility.sumo_config.command);  
	  omg_param_list.sumo_config = (char*) malloc(256);
	  sprintf(omg_param_list.sumo_config, "%s", oai_emulation.topology_config.mobility.UE_mobility.sumo_config.file);  
	  omg_param_list.sumo_start = oai_emulation.topology_config.mobility.UE_mobility.sumo_config.start;

	  if (oai_emulation.topology_config.mobility.UE_mobility.sumo_config.end > 0 )
	    omg_param_list.sumo_end = oai_emulation.topology_config.mobility.UE_mobility.sumo_config.end;
	  else 
	    omg_param_list.sumo_end = (oai_emulation.info.n_frames_flag == 1 ) ?  oai_emulation.info.n_frames : 1024 ; // fixme: the else case is infinity 
	  
	  omg_param_list.sumo_step = oai_emulation.topology_config.mobility.UE_mobility.sumo_config.step=1; //  1000ms
	  omg_param_list.sumo_host = (char*) malloc(40);
	  sprintf(omg_param_list.sumo_host,"%s",oai_emulation.topology_config.mobility.UE_mobility.sumo_config.hip);
	  omg_param_list.sumo_port = oai_emulation.topology_config.mobility.UE_mobility.sumo_config.hport ;
	  LOG_D(OMG, "opt (%s,%d) cmd (%s,%s) config_file (%s,%s) hip (%s,%s) \n", 
		oai_emulation.topology_config.mobility.UE_mobility.UE_mobility_type.selected_option,oai_emulation.info.omg_model_ue,
		omg_param_list.sumo_command, oai_emulation.topology_config.mobility.UE_mobility.sumo_config.command,
		omg_param_list.sumo_config, oai_emulation.topology_config.mobility.UE_mobility.sumo_config.file,
		omg_param_list.sumo_host, oai_emulation.topology_config.mobility.UE_mobility.sumo_config.hip);
	}
	if (omg_param_list.nodes >0 ) 
	    init_mobility_generator(omg_param_list);
	
	if (oai_emulation.topology_config.omv == 1 ) 
	  oai_emulation.info.omv_enabled =  1;
	  
	return 1;
}



int ocg_config_app(){

	char colon[] = ":";
	char comma[] = ",";
	char tmp_source_id[128];
	char *check_format1;
	char *check_format2;
	char *source_id_start;
	char *source_id_end;
	char *per_source_id;
	int source_id_index;
	int destination_id_index;
	int i,j,k;

	int predefined_traffic_config_index;
	int customized_traffic_config_index;

	init_all_otg();

	g_otg->num_nodes = oai_emulation.info.nb_enb_local + oai_emulation.info.nb_ue_local;

	for (i=0; i<g_otg->num_nodes; i++){
		g_otg->duration[i]=oai_emulation.emulation_config.emulation_time_ms;
		g_otg->dst_port[i]=oai_emulation.application_config.customized_traffic.destination_port[i];
		g_otg->dst_ip[i]=oai_emulation.application_config.customized_traffic.destination_id[i];
		g_otg->ip_v[i]=map_str_to_int(otg_ip_version_names, oai_emulation.application_config.customized_traffic.ip_version[i]);
		g_otg->trans_proto[i]=map_str_to_int(otg_transport_protocol_names, oai_emulation.application_config.customized_traffic.transport_protocol[i]);

		for (j=0; j<g_otg->num_nodes; j++){

			g_otg->application_type[i][j]=map_str_to_int(otg_app_type_names, "no_predefined_traffic");
			for (k=0; k<MAX_NUM_TRAFFIC_STATE; k++){

				g_otg->idt_dist[i][j][k]=map_str_to_int(otg_distribution_names, "no_customized_traffic");
				g_otg->idt_min[i][j][k]=oai_emulation.application_config.customized_traffic.idt_min_ms[i];
				g_otg->idt_max[i][j][k]=oai_emulation.application_config.customized_traffic.idt_max_ms[i];
				g_otg->idt_std_dev[i][j][k]=oai_emulation.application_config.customized_traffic.idt_standard_deviation[i];
				g_otg->idt_lambda[i][j][k]=oai_emulation.application_config.customized_traffic.idt_lambda[i];
				g_otg->idt_scale[i][j][k]=oai_emulation.application_config.customized_traffic.idt_scale[i];
				g_otg->idt_shape[i][j][k]=oai_emulation.application_config.customized_traffic.idt_shape[i];

				g_otg->size_dist[i][j][k]=map_str_to_int(otg_distribution_names, "no_customized_traffic");
				g_otg->size_min[i][j][k]=oai_emulation.application_config.customized_traffic.size_min_byte[i];
				g_otg->size_max[i][j][k]=oai_emulation.application_config.customized_traffic.size_max_byte[i];
				g_otg->size_std_dev[i][j][k]=oai_emulation.application_config.customized_traffic.size_standard_deviation[i];
				g_otg->size_lambda[i][j][k]=oai_emulation.application_config.customized_traffic.size_lambda[i];
				g_otg->size_scale[i][j][k]=oai_emulation.application_config.customized_traffic.size_scale[i];
				g_otg->size_shape[i][j][k]=oai_emulation.application_config.customized_traffic.size_shape[i];
			}
		}
	}

	init_seeds(g_otg->seed); // initialize all the nodes, then configure the nodes the user specifically did in the XML in the following

	LOG_I(OTG,"oai_emulation.info.max_predefined_traffic_config_index = %d\n", oai_emulation.info.max_predefined_traffic_config_index);


	if (oai_emulation.info.ocg_ok) {
	  ///// for the predefined traffic
	  for (	predefined_traffic_config_index = 1; 
		predefined_traffic_config_index <= oai_emulation.info.max_predefined_traffic_config_index; 
		predefined_traffic_config_index++) {
	    
	    LOG_I(OTG,"OCG_config_OTG: predefined no. %d\n", predefined_traffic_config_index);

	    strcpy(tmp_source_id, oai_emulation.application_config.predefined_traffic.source_id[predefined_traffic_config_index]);
	    check_format1 = strstr(tmp_source_id, colon);
	    check_format2 = strstr(tmp_source_id, comma);
	    
	    if (check_format1 != NULL) { // format 1:10
	      source_id_start = strtok(tmp_source_id, colon);
	      source_id_end = strtok(NULL, colon);
	      
	      source_id_start = atoi(source_id_start);
	      source_id_end = atoi(source_id_end);
	      
	      destination_id_index = atoi(oai_emulation.application_config.predefined_traffic.destination_id[predefined_traffic_config_index]);
	      
	      for (source_id_index = source_id_start; source_id_index <= source_id_end; source_id_index++) {
		g_otg->application_type[source_id_index][destination_id_index] = map_str_to_int(otg_app_type_names, oai_emulation.application_config.predefined_traffic.application_type[predefined_traffic_config_index]);

LOG_I(OTG,"predef:: OCG_config_OTG: FORMAT (%d:%d) source = %d, dest = %d, Application = %d\n", source_id_start, source_id_end, source_id_index, destination_id_index, g_otg->application_type[source_id_index][destination_id_index]);
		

	      }
	    } else if (check_format2 != NULL) { // format 1,2,3,5
	      per_source_id = strtok(tmp_source_id, comma);
	      destination_id_index = atoi(oai_emulation.application_config.predefined_traffic.destination_id[predefined_traffic_config_index]);
	      while (per_source_id != NULL) {
		source_id_index = atoi(per_source_id);
		g_otg->application_type[source_id_index][destination_id_index] = map_str_to_int(otg_app_type_names, oai_emulation.application_config.predefined_traffic.application_type[predefined_traffic_config_index]);
		per_source_id = strtok(NULL, comma);
	      }
	    } else { // single node configuration
	      source_id_index = atoi(oai_emulation.application_config.predefined_traffic.source_id[predefined_traffic_config_index]);
	      destination_id_index = atoi(oai_emulation.application_config.predefined_traffic.destination_id[predefined_traffic_config_index]);
	      g_otg->application_type[source_id_index][destination_id_index] = map_str_to_int(otg_app_type_names, oai_emulation.application_config.predefined_traffic.application_type[predefined_traffic_config_index]);
	      
	    }
	    init_predef_traffic();
	  }
	  	  
	  
///////// for the customized traffic
	  for (	customized_traffic_config_index = 1; 
		customized_traffic_config_index <= oai_emulation.info.max_customized_traffic_config_index; 
		customized_traffic_config_index++) {
	    
	    LOG_I(OTG,"OCG_config_OTG: customized no. %d\n", customized_traffic_config_index);
	    strcpy(tmp_source_id, oai_emulation.application_config.customized_traffic.source_id[customized_traffic_config_index]);
	    
	    check_format1 = strstr(tmp_source_id, colon);
	    check_format2 = strstr(tmp_source_id, comma);
	    
	    
	    if (check_format1 != NULL) { // format 1:10
	      source_id_start = strtok(tmp_source_id, colon);
	      source_id_end = strtok(NULL, colon);
	      
	      source_id_start = atoi(source_id_start);
	      source_id_end = atoi(source_id_end);
	      destination_id_index = atoi(oai_emulation.application_config.customized_traffic.destination_id[customized_traffic_config_index]);
	      
	      for (source_id_index = source_id_start; source_id_index <= source_id_end; source_id_index++) {
		
		g_otg->trans_proto[source_id_index] = map_str_to_int(otg_transport_protocol_names, oai_emulation.application_config.customized_traffic.transport_protocol[customized_traffic_config_index]);
		g_otg->ip_v[source_id_index] = map_str_to_int(otg_ip_version_names, oai_emulation.application_config.customized_traffic.ip_version[customized_traffic_config_index]);
		
		g_otg->idt_dist[source_id_index][destination_id_index][0] = map_str_to_int(otg_distribution_names, oai_emulation.application_config.customized_traffic.idt_dist[customized_traffic_config_index]);
		g_otg->idt_dist[source_id_index][destination_id_index][1] = map_str_to_int(otg_distribution_names, oai_emulation.application_config.customized_traffic.idt_dist[customized_traffic_config_index]);
		
		g_otg->idt_min[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.idt_min_ms[customized_traffic_config_index];
		g_otg->idt_min[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.idt_min_ms[customized_traffic_config_index];
		
		g_otg->idt_max[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.idt_max_ms[customized_traffic_config_index];
		g_otg->idt_max[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.idt_max_ms[customized_traffic_config_index];
		
		g_otg->idt_std_dev[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.idt_standard_deviation[customized_traffic_config_index];
		g_otg->idt_std_dev[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.idt_standard_deviation[customized_traffic_config_index];
		
		g_otg->idt_lambda[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.idt_lambda[customized_traffic_config_index];
		g_otg->idt_lambda[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.idt_lambda[customized_traffic_config_index];
		
		g_otg->idt_scale[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.idt_scale[customized_traffic_config_index];
		g_otg->idt_scale[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.idt_scale[customized_traffic_config_index];
		
		g_otg->idt_shape[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.idt_shape[customized_traffic_config_index];
		g_otg->idt_shape[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.idt_shape[customized_traffic_config_index];
		
		g_otg->size_dist[source_id_index][destination_id_index][0] = map_str_to_int(otg_distribution_names, oai_emulation.application_config.customized_traffic.size_dist[customized_traffic_config_index]);
		g_otg->size_dist[source_id_index][destination_id_index][1] = map_str_to_int(otg_distribution_names, oai_emulation.application_config.customized_traffic.size_dist[customized_traffic_config_index]);
		
		g_otg->size_min[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.size_min_byte[customized_traffic_config_index];
		g_otg->size_min[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.size_min_byte[customized_traffic_config_index];
		
		g_otg->size_max[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.size_max_byte[customized_traffic_config_index];
		g_otg->size_max[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.size_max_byte[customized_traffic_config_index];
		
		g_otg->size_std_dev[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.size_standard_deviation[customized_traffic_config_index];
		g_otg->size_std_dev[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.size_standard_deviation[customized_traffic_config_index];

		g_otg->size_lambda[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.size_lambda[customized_traffic_config_index];
		g_otg->size_lambda[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.size_lambda[customized_traffic_config_index];

		g_otg->size_scale[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.size_scale[customized_traffic_config_index];
		g_otg->size_scale[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.size_scale[customized_traffic_config_index];
		
		g_otg->size_shape[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.size_shape[customized_traffic_config_index];
		g_otg->size_shape[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.size_shape[customized_traffic_config_index];
		
		g_otg->dst_port[destination_id_index] = oai_emulation.application_config.customized_traffic.destination_port[customized_traffic_config_index];

LOG_I(OTG,"custom:: OCG_config_OTG: FORMAT (%d:%d) source = %d, dest = %d, dist type for size = %d\n", source_id_start, source_id_end, source_id_index, destination_id_index, g_otg->size_dist[source_id_index][destination_id_index][0]);
		
	      }
	      
	    } else if (check_format2 != NULL) { // format 1,2,3,5
	      per_source_id = strtok(tmp_source_id, comma);
	      destination_id_index = atoi(oai_emulation.application_config.customized_traffic.destination_id[customized_traffic_config_index]);
	      
	      while (per_source_id != NULL) {
		source_id_index = atoi(per_source_id);

		g_otg->trans_proto[source_id_index] = map_str_to_int(otg_transport_protocol_names, oai_emulation.application_config.customized_traffic.transport_protocol[customized_traffic_config_index]);
		g_otg->ip_v[source_id_index] = map_str_to_int(otg_ip_version_names, oai_emulation.application_config.customized_traffic.ip_version[customized_traffic_config_index]);
		
		g_otg->idt_dist[source_id_index][destination_id_index][0] = map_str_to_int(otg_distribution_names, oai_emulation.application_config.customized_traffic.idt_dist[customized_traffic_config_index]);
		g_otg->idt_dist[source_id_index][destination_id_index][1] = map_str_to_int(otg_distribution_names, oai_emulation.application_config.customized_traffic.idt_dist[customized_traffic_config_index]);

		g_otg->idt_min[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.idt_min_ms[customized_traffic_config_index];
		g_otg->idt_min[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.idt_min_ms[customized_traffic_config_index];

		g_otg->idt_max[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.idt_max_ms[customized_traffic_config_index];
		g_otg->idt_max[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.idt_max_ms[customized_traffic_config_index];
		
		g_otg->idt_std_dev[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.idt_standard_deviation[customized_traffic_config_index];
		g_otg->idt_std_dev[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.idt_standard_deviation[customized_traffic_config_index];
		
		g_otg->idt_lambda[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.idt_lambda[customized_traffic_config_index];
		g_otg->idt_lambda[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.idt_lambda[customized_traffic_config_index];
		
		g_otg->idt_scale[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.idt_scale[customized_traffic_config_index];
		g_otg->idt_scale[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.idt_scale[customized_traffic_config_index];

		g_otg->idt_shape[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.idt_shape[customized_traffic_config_index];
		g_otg->idt_shape[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.idt_shape[customized_traffic_config_index];

		g_otg->size_dist[source_id_index][destination_id_index][0] = map_str_to_int(otg_distribution_names, oai_emulation.application_config.customized_traffic.size_dist[customized_traffic_config_index]);
		g_otg->size_dist[source_id_index][destination_id_index][1] = map_str_to_int(otg_distribution_names, oai_emulation.application_config.customized_traffic.size_dist[customized_traffic_config_index]);

		g_otg->size_min[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.size_min_byte[customized_traffic_config_index];
		g_otg->size_min[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.size_min_byte[customized_traffic_config_index];

		
		g_otg->size_max[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.size_max_byte[customized_traffic_config_index];
		g_otg->size_max[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.size_max_byte[customized_traffic_config_index];
		
		g_otg->size_std_dev[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.size_standard_deviation[customized_traffic_config_index];
		g_otg->size_std_dev[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.size_standard_deviation[customized_traffic_config_index];
		
		g_otg->size_lambda[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.size_lambda[customized_traffic_config_index];
		g_otg->size_lambda[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.size_lambda[customized_traffic_config_index];
		
		g_otg->size_scale[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.size_scale[customized_traffic_config_index];
		g_otg->size_scale[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.size_scale[customized_traffic_config_index];
		
		g_otg->size_shape[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.size_shape[customized_traffic_config_index];
		g_otg->size_shape[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.size_shape[customized_traffic_config_index];
		
		g_otg->dst_port[destination_id_index] = oai_emulation.application_config.customized_traffic.destination_port[customized_traffic_config_index];
		
		per_source_id = strtok(NULL, comma);
		}
	      
	    } else { // single node configuration
	      source_id_index = atoi(oai_emulation.application_config.customized_traffic.source_id[customized_traffic_config_index]);
	      destination_id_index = atoi(oai_emulation.application_config.customized_traffic.destination_id[customized_traffic_config_index]);
	      
	      g_otg->trans_proto[source_id_index] = map_str_to_int(otg_transport_protocol_names, oai_emulation.application_config.customized_traffic.transport_protocol[customized_traffic_config_index]);
	      g_otg->ip_v[source_id_index] = map_str_to_int(otg_ip_version_names, oai_emulation.application_config.customized_traffic.ip_version[customized_traffic_config_index]);
	      g_otg->idt_dist[source_id_index][destination_id_index][0] = map_str_to_int(otg_distribution_names, oai_emulation.application_config.customized_traffic.idt_dist[customized_traffic_config_index]);
	      g_otg->idt_dist[source_id_index][destination_id_index][1] = map_str_to_int(otg_distribution_names, oai_emulation.application_config.customized_traffic.idt_dist[customized_traffic_config_index]);
	      
	      g_otg->idt_min[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.idt_min_ms[customized_traffic_config_index];
	      g_otg->idt_min[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.idt_min_ms[customized_traffic_config_index];
	      
	      g_otg->idt_max[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.idt_max_ms[customized_traffic_config_index];
	      g_otg->idt_max[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.idt_max_ms[customized_traffic_config_index];

	      g_otg->idt_std_dev[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.idt_standard_deviation[customized_traffic_config_index];
	      g_otg->idt_std_dev[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.idt_standard_deviation[customized_traffic_config_index];
	      
	      g_otg->idt_lambda[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.idt_lambda[customized_traffic_config_index];
	      g_otg->idt_lambda[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.idt_lambda[customized_traffic_config_index];
	      
	      g_otg->idt_scale[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.idt_scale[customized_traffic_config_index];
	      g_otg->idt_scale[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.idt_scale[customized_traffic_config_index];
	      
	      g_otg->idt_shape[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.idt_shape[customized_traffic_config_index];
	      g_otg->idt_shape[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.idt_shape[customized_traffic_config_index];
	      
	      g_otg->size_dist[source_id_index][destination_id_index][0] = map_str_to_int(otg_distribution_names, oai_emulation.application_config.customized_traffic.size_dist[customized_traffic_config_index]);
	      g_otg->size_dist[source_id_index][destination_id_index][1] = map_str_to_int(otg_distribution_names, oai_emulation.application_config.customized_traffic.size_dist[customized_traffic_config_index]);
	      LOG_I(OTG,"OCG_config_OTG: source = %d, dest = %d, dist type for size = %d\n", source_id_index, destination_id_index, g_otg->size_dist[source_id_index][destination_id_index][0]);
	      g_otg->size_min[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.size_min_byte[customized_traffic_config_index];
	      g_otg->size_min[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.size_min_byte[customized_traffic_config_index];
	      
	      
	      g_otg->size_max[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.size_max_byte[customized_traffic_config_index];
	      g_otg->size_max[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.size_max_byte[customized_traffic_config_index];
	      
	      g_otg->size_std_dev[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.size_standard_deviation[customized_traffic_config_index];
	      g_otg->size_std_dev[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.size_standard_deviation[customized_traffic_config_index];
	      
	      g_otg->size_lambda[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.size_lambda[customized_traffic_config_index];
	      g_otg->size_lambda[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.size_lambda[customized_traffic_config_index];

	      
	      g_otg->size_scale[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.size_scale[customized_traffic_config_index];
	      g_otg->size_scale[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.size_scale[customized_traffic_config_index];
	      
	      g_otg->size_shape[source_id_index][destination_id_index][0] =  oai_emulation.application_config.customized_traffic.size_shape[customized_traffic_config_index];
	      g_otg->size_shape[source_id_index][destination_id_index][1] =  oai_emulation.application_config.customized_traffic.size_shape[customized_traffic_config_index];
	      
	      g_otg->dst_port[destination_id_index] = oai_emulation.application_config.customized_traffic.destination_port[customized_traffic_config_index];
	      
	    }
	    
	  }
	} else { // OCG not used, but -T option is used, so config here
	  for (i=0; i<g_otg->num_nodes; i++){
	    for (j=0; j<g_otg->num_nodes; j++){ 
	      g_otg->application_type[i][j] = oai_emulation.info.otg_traffic;
	      init_predef_traffic();
	    }
	  }
	}
	
	printf("OCG_config_OTG done! \n");
	return 1;
}




int ocg_config_emu(){

  if (oai_emulation.emulation_config.emulation_time_ms != 0) {
    oai_emulation.info.n_frames  =  (int) oai_emulation.emulation_config.emulation_time_ms / 10; // configure the number of frame
    oai_emulation.info.n_frames_flag = 1;
    LOG_I(OCG, "number of frames in emulation is set to %d\n", oai_emulation.info.n_frames);
  } else
    LOG_I(OCG, "number of frames in emulation is set to infinity\n");
  
  oai_emulation.info.seed = (oai_emulation.emulation_config.seed.value == 0) ? oai_emulation.info.seed : oai_emulation.emulation_config.seed.value;
  
  if (oai_emulation.info.cli_enabled == 1){
    if (cli_server_init(cli_server_recv) < 0) {
      LOG_E(EMU,"cli server init failed \n");
      exit(-1);
    }
    LOG_I(EMU, "OAI CLI is enabled\n");
    LOG_I(EMU,"eNB start state is %d, UE start state %d\n", 
	  oai_emulation.info.cli_start_enb[0],
	  oai_emulation.info.cli_start_ue[0]);
  }
  
  //bin/LOG_I(OCG, "OPT output file directory = %s\n", oai_emulation.info.output_path);
  oai_emulation.info.opt_enabled = ( oai_emulation.emulation_config.packet_trace.enabled == 0) ? oai_emulation.info.opt_enabled :  oai_emulation.emulation_config.packet_trace.enabled;
  if (oai_emulation.info.opt_enabled == 1 ){
      LOG_I(OPT, " initiated  ");
      Init_OPT(0,"outfile.dump","127.0.0.1",1234); // Init_OPT(2, oai_emulation.info.output_path, NULL, 0);*/
    }
  
  return 1;  

}
