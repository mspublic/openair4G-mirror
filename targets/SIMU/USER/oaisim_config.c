#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "oaisim_config.h"

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


void oaisim_config(OAI_Emulation * emulation_scen, u16 * n_frames, char * g_log_level) {
 
	olg_config(g_log_level);

#ifdef OCG_FLAG
  if (emu_info.ocg_enabled == 1){ // activate OCG: xml-based scenario parser 
    emulation_scen = OCG_main(emu_info.local_server);// eurecom or portable
  }
#endif

	ocg_config(emulation_scen, n_frames);
	omg_config(emulation_scen);
	opt_config();
	otg_config();


}

void olg_config(char * g_log_level) {

 //initialize the log generator 
  logInit(map_str_to_int(log_level_names, g_log_level));


	set_comp_log(OCG,  LOG_INFO, 1);
	
	set_comp_log(OMG,  LOG_INFO, 1);
	
  LOG_T(LOG,"global log level is set to %s \n",g_log_level );

}

void omg_config(OAI_Emulation * emulation_scen){

	init_omg_global_params();
	// setup params for openair mobility generator
	//common params
	omg_param_list.min_X = 0;
	omg_param_list.max_X = emulation_scen->topology_config.area.x;
	omg_param_list.min_Y = 0;
	omg_param_list.max_Y = emulation_scen->topology_config.area.y;
	// init values
	omg_param_list.min_speed = 1.0;
	omg_param_list.max_speed = 50.0;
	omg_param_list.min_journey_time = 1.0;
	omg_param_list.max_journey_time = 10.0;
	omg_param_list.min_azimuth = 0; // ???
	omg_param_list.max_azimuth = 360; // ???
	omg_param_list.min_sleep = 0.1;
	omg_param_list.max_sleep = 8.0;
	
	// init OMG for eNB
	if (!strcmp(emulation_scen->topology_config.mobility.eNB_mobility.eNB_mobility_type.selected_option, "fixed")) {
		emu_info.omg_model_enb = STATIC;
	} else if (!strcmp(emulation_scen->topology_config.mobility.eNB_mobility.eNB_mobility_type.selected_option, "mobile")) {
		emu_info.omg_model_enb = RWP; // set eNB to be random waypoint
	}
	omg_param_list.mobility_type = emu_info.omg_model_enb; 

	omg_param_list.nodes_type = eNB;  //eNB
	omg_param_list.nodes = emu_info.nb_enb_local;
 	omg_param_list.seed = emu_info.nb_enb_local; // specific seed for enb and ue to avoid node overlapping

	// at this moment, we use the above moving dynamics for mobile eNB

	init_mobility_generator(omg_param_list);


	
	// init OMG for UE
	// input of OMG: STATIC: 0, RWP: 1 or RWALK 2
	if (!strcmp(emulation_scen->topology_config.mobility.UE_mobility.UE_mobility_type.selected_option, "fixed")) {
		emu_info.omg_model_ue = STATIC;
	} else if (!strcmp(emulation_scen->topology_config.mobility.UE_mobility.UE_mobility_type.selected_option, "random_waypoint")) {
		emu_info.omg_model_ue = RWP;
	} else if (!strcmp(emulation_scen->topology_config.mobility.UE_mobility.UE_mobility_type.selected_option, "random_walk")) {
		emu_info.omg_model_ue = RWALK;
	} else {
		emu_info.omg_model_ue = STATIC;
	}
	omg_param_list.mobility_type = emu_info.omg_model_ue;

	omg_param_list.nodes_type = UE;//UE
	omg_param_list.nodes = emu_info.nb_ue_local;
	omg_param_list.seed = emu_info.nb_ue_local; // specific seed for enb and ue to avoid node overlapping
	omg_param_list.seed = emu_info.nb_ue_local+1;// specific seed for enb and ue to avoid node overlapping

	omg_param_list.min_speed = (emulation_scen->topology_config.mobility.UE_mobility.UE_moving_dynamics.min_speed == 0) ? 0.1 : emulation_scen->topology_config.mobility.UE_mobility.UE_moving_dynamics.min_speed;
	omg_param_list.max_speed = (emulation_scen->topology_config.mobility.UE_mobility.UE_moving_dynamics.max_speed == 0) ? 0.1 : emulation_scen->topology_config.mobility.UE_mobility.UE_moving_dynamics.max_speed;

	omg_param_list.min_journey_time = (emulation_scen->topology_config.mobility.UE_mobility.UE_moving_dynamics.min_journey_time == 0) ? 0.1 : emulation_scen->topology_config.mobility.UE_mobility.UE_moving_dynamics.min_journey_time;
	omg_param_list.max_journey_time = (emulation_scen->topology_config.mobility.UE_mobility.UE_moving_dynamics.max_journey_time == 0) ? 0.1 : emulation_scen->topology_config.mobility.UE_mobility.UE_moving_dynamics.max_journey_time;

	omg_param_list.min_azimuth = 0.1; // wait for advanced OSD
	omg_param_list.max_azimuth = 360;

	omg_param_list.min_sleep = (emulation_scen->topology_config.mobility.UE_mobility.UE_moving_dynamics.min_pause_time == 0) ? 0.1 : emulation_scen->topology_config.mobility.UE_mobility.UE_moving_dynamics.min_pause_time;
	omg_param_list.max_sleep = (emulation_scen->topology_config.mobility.UE_mobility.UE_moving_dynamics.max_pause_time == 0) ? 0.1 : emulation_scen->topology_config.mobility.UE_mobility.UE_moving_dynamics.max_pause_time;

	init_mobility_generator(omg_param_list);

}

void ocg_config(OAI_Emulation * emulation_scen, u16 * n_frames){

#ifdef OCG_FLAG
    // here is to check if OCG is successful, otherwise, we might not run the emulation
    if (emulation_scen->info.ocg_ok != 1) { 
      LOG_E(OCG, "Error found by OCG; emulation not launched. Please find more information in the OCG_report.xml. \nRemind: please check the name of the XML configuration file and its content if you use a self-specified file.\n");
      exit(EXIT_FAILURE);
      }

      set_envi(emulation_scen);
      set_topo(emulation_scen);
      set_app(emulation_scen);
      set_emu(emulation_scen, n_frames);
      LOG_T(OCG," ue local %d enb local %d frame %d\n",   emu_info.nb_ue_local,   emu_info.nb_enb_local, n_frames );

     /* : TODO
      LOG_I(OCG, "OPT output file directory = %s\n", emulation_scen->info.output_path);
      Init_OPT(2, emulation_scen->info.output_path, NULL, 0);*/

#endif

}

void opt_config(){

}

void otg_config(){

}

void set_envi(OAI_Emulation * emulation_scen) {

	LOG_I(OCG, "environment is set\n");
}

void set_topo(OAI_Emulation * emulation_scen) {
	emu_info.nb_ue_local  = emulation_scen->info.number_of_UE; // configure the number of UE
	emu_info.nb_enb_local = emulation_scen->info.number_of_eNB; // configure the number of eNB
 
	LOG_I(OCG, "topology is set\n");
}

void set_app(OAI_Emulation * emulation_scen) {

	LOG_I(OCG, "application is set\n");
}

void set_emu(OAI_Emulation * emulation_scen, u16 * n_frames) {
	*n_frames  =  (int) emulation_scen->emulation_config.emulation_time / 10; // configure the number of frame

	LOG_I(OCG, "emulation is set\n");
}

