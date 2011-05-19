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
	omg_config();
	ocg_config(emulation_scen, n_frames);
	opt_config();
	otg_config();


}

void olg_config(char * g_log_level) {



 //initialize the log generator 
  logInit(map_str_to_int(log_level_names, g_log_level));
  LOG_T(LOG,"global log level is set to %s \n",g_log_level );

}

void omg_config(){

	init_omg_global_params();
	// setup params for openair mobility generator
	//common params
	omg_param_list.min_X = 0;
	omg_param_list.max_X = 100;
	omg_param_list.min_Y = 0;
	omg_param_list.max_Y = 100;
	omg_param_list.min_speed = 0.1;
	omg_param_list.max_speed = 20.0;
	omg_param_list.min_journey_time = 0.1;
	omg_param_list.max_journey_time = 10.0;
	omg_param_list.min_azimuth = 0; 
	omg_param_list.max_azimuth = 360; 
	omg_param_list.min_sleep = 0.1;
	omg_param_list.max_sleep = 5.0;
	
	// enb 
	omg_param_list.mobility_type = STATIC; // set eNB to be static
	omg_param_list.nodes_type = eNB;//enb
	omg_param_list.nodes = emu_info.nb_enb_local;	
 	omg_param_list.seed = emu_info.nb_enb_local; // specific seed for enb and ue to avoid node overlapping
	init_mobility_generator(omg_param_list);
	
	//ue 
	if (emu_info.omg_enabled == 1)
	  omg_param_list.mobility_type = emu_info.omg_model;
	else
	  omg_param_list.mobility_type = STATIC; 
	
	omg_param_list.nodes = emu_info.nb_ue_local;
	omg_param_list.nodes_type = UE;
	omg_param_list.seed = emu_info.nb_ue_local+1;// specific seed for enb and ue to avoid node overlapping
	
	init_mobility_generator(omg_param_list);

}

void ocg_config(OAI_Emulation * emulation_scen, u16 * n_frames){

#ifdef OCG_FLAG
  if (emu_info.ocg_enabled == 1){ // activate OCG: xml-based scenario parser 
    emulation_scen= OCG_main(emu_info.local_server);// eurecom or portable
    // here is to check if OCG is successful, otherwise, we might not run the emulation
    if (emulation_scen->useful_info.OCG_OK != 1) { 
      LOG_E(OCG, "Error found by OCG; emulation not launched. Please find more information in the OCG_report.xml. \nRemind: please check the name of the XML configuration file and its content if you use a self-specified file.\n");
      exit(EXIT_FAILURE);
      }

      set_envi(emulation_scen);
      set_topo(emulation_scen, &emu_info);
      set_app(emulation_scen);
      set_emu(emulation_scen, n_frames);
      LOG_T(OCG," ue local %d enb local %d frame %d\n",   emu_info.nb_ue_local,   emu_info.nb_enb_local, n_frames );

     /* : TODO
      LOG_I(OCG, "OPT output file directory = %s\n", emulation_scen->useful_info.output_path);
      Init_OPT(2, emulation_scen->useful_info.output_path, NULL, 0);*/

   }
#endif

}

void opt_config(){

}

void otg_config(){

}

void set_envi(OAI_Emulation * emulation_scen) {
	/*LOG_T(OCG,"the area is x %f y %f option %s\n",
	emulation_scen->envi_config.area.x,
	emulation_scen->envi_config.area.y, 
	emulation_scen->topo_config.eNB_topology.selected_option);*/
	LOG_I(OCG, "environment is set\n");
}

void set_topo(OAI_Emulation * emulation_scen, emu_info_t * emu_info) {
	emu_info->nb_ue_local  = emulation_scen->topo_config.number_of_UE; // configure the number of UE
	emu_info->nb_enb_local = emulation_scen->topo_config.number_of_eNB; // configure the number of eNB

	omg_param_list.min_X = 0;
	omg_param_list.max_X = emulation_scen->envi_config.area.x;
	omg_param_list.min_Y = 0;
	omg_param_list.max_Y = emulation_scen->envi_config.area.y;

	// init OMG for eNB
	omg_param_list.nodes = emulation_scen->topo_config.number_of_eNB;
	omg_param_list.min_speed = 0;
	omg_param_list.max_speed = 0;
	omg_param_list.min_journey_time = 0;
	omg_param_list.max_journey_time = 0;
	omg_param_list.min_azimuth = 0; // ???
	omg_param_list.max_azimuth = 360; // ???
	omg_param_list.min_sleep = 0;
	omg_param_list.max_sleep = 0;
	omg_param_list.mobility_type = 0; // set eNB to be static
	// omg_param_list.seed = ;

	init_mobility_generator(omg_param_list);


	// init OMG for UE
	omg_param_list.nodes = emulation_scen->topo_config.number_of_UE;  
	omg_param_list.min_speed = (emulation_scen->topo_config.mobility.moving_dynamics.min_speed == 0) ? 0.1 : emulation_scen->topo_config.mobility.moving_dynamics.min_speed;
	omg_param_list.max_speed = (emulation_scen->topo_config.mobility.moving_dynamics.max_speed == 0) ? 0.1 : emulation_scen->topo_config.mobility.moving_dynamics.max_speed;
	omg_param_list.min_journey_time = 0.1; // TODO to be added into OSD and OCG
	omg_param_list.max_journey_time = 10;
	omg_param_list.min_azimuth = 0.1;
	omg_param_list.max_azimuth = 360;
	omg_param_list.min_sleep = (emulation_scen->topo_config.mobility.moving_dynamics.min_pause_time == 0) ? 0.1 : emulation_scen->topo_config.mobility.moving_dynamics.min_pause_time;
	omg_param_list.max_sleep = (emulation_scen->topo_config.mobility.moving_dynamics.max_pause_time == 0) ? 0.1 : emulation_scen->topo_config.mobility.moving_dynamics.max_pause_time;
	

	//input of OMG: STATIC: 0, RWP: 1 or RWALK 2
	if (!strcmp(emulation_scen->topo_config.mobility.mobility_type.selected_option, "fixed")) {
		omg_param_list.mobility_type = 0;
	} else if (!strcmp(emulation_scen->topo_config.mobility.mobility_type.selected_option, "random_waypoint")) {
		omg_param_list.mobility_type = 1;
	} else if (!strcmp(emulation_scen->topo_config.mobility.mobility_type.selected_option, "random_walk")) {
		omg_param_list.mobility_type = 2;
	//} else if (!strcmp(emulation_scen->topo_config.mobility.mobility_type.selected_option, "grid_walk")) {
	//	omg_param_list.mobility_type = 3;
	} else {
		omg_param_list.mobility_type = 0;
	}
	// omg_param_list.seed = ;

	init_mobility_generator(omg_param_list);

	emu_info->omg_enabled = 1; // TODO omg always enabled ???
/*
// inputs for OMG : TODO: seed


	if (!strcmp(emulation_scen->topo_config.mobility.random_seed.selected_option, "oaiseed")) {
	} else if (!strcmp(emulation_scen->topo_config.mobility.random_seed.selected_option, "userseed")) {
		emulation_scen->topo_config.mobility.random_seed.user_seed.seed_value;
	}
	
*/
	LOG_I(OCG, "topology is set\n");
}

void set_app(OAI_Emulation * emulation_scen) {

	LOG_I(OCG, "application is set\n");
}

void set_emu(OAI_Emulation * emulation_scen, u16 * n_frames) {
	*n_frames  =  (int) emulation_scen->emu_config.emu_time / 10; // configure the number of frame

	set_comp_log(OCG,  LOG_ERR, 1);
	set_comp_log(OCG,  LOG_INFO, 1);
	set_comp_log(OCG,  LOG_TRACE, 1);

	LOG_I(OCG, "emulation is set\n");
}

