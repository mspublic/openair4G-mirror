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

/*! \file OCG.h
* \brief Data structure for OCG of OpenAir emulator
* \author Lusheng Wang and Navid Nikaein
* \date 2011
* \version 1.0
* \company Eurecom
* \email: openair_tech@eurecom.fr
* \note
* \warning
*/

//#include "SIMULATION/TOOLS/defs.h"


#ifndef __OCG_H__
#define __OCG_H__

#include "PHY/impl_defs_top.h"
#ifdef __cplusplus
extern "C"
{
#endif
/** @defgroup _OCG OpenAir Config Generation (OCG)
 * @{*/
/* @}*/

/** @defgroup _useful_macro Macro Definition
 *  @ingroup _OCG
 *  @brief the following macros will be used in the code of OCG
 * @{*/
#define MODULE_NOT_PROCESSED -9999	/*!< \brief the module state indicator is set to -9999 before the module being processed */
#define MODULE_ERROR -1		/*!< \brief the module state indicator is set to -1 for error */
#define MODULE_OK 0		/*!< \brief the module state indicator is set to 0 when successfully processed */
#define GET_HELP 1		/*!< \brief the module state indicator is set to 1 for get_opt_OK when the user types -h option */
#define NO_FILE -2		/*!< \brief the module state indicator is set to -2 for detect_file_OK when no file is detected */
#define WEB_XML_FOLDER "/nfs/webxml/"	/*!< \brief the web portal generates XML files into this folder */
#define LOCAL_XML_FOLDER "local_XML/"	/*!< \brief this folder contains some XML files for demo, users could also put their own XML files into this folder for a direct emulation without using the web portal */
#define TEMP_OUTPUT_DIR "temp_output/"	/*!< \brief temporary output files will be generated in this folder when folders for an emulation could not be created due to errors */
#define OUTPUT_DIR "/nfs/emu_results/"	/*!< \brief this folder contains all the output files when folders for an emulation could be successfully created */
#define FILENAME_LENGTH_MAX 128	/*!< \brief the maximum length of a filename */
#define DIR_LENGTH_MAX 128	/*!< \brief the maximum length of the path name */
#define MOBI_XML_FOLDER "mobi_XML/"	/*!< \brief the folder that mobigen generate XML files in */
#define DIR_TO_MOBIGEN "XML_to_mobigen/"	/*!< \brief the folder that mobigen detects XML file from OCG */

#define NUMBER_OF_MASTER_MAX   20

/* @}*/



/** @defgroup _enum_fsm OCG Finite State Machine (FSM)
 *  @ingroup _OCG
 *  @brief See the flow chart for details
 * @{*/

/** @defgroup _fsm_flow FSM Flow Chart
 *  @ingroup _enum_fsm
 *  @brief This flow chart shows how the FSM works
 
There are the following steps the OCG module should contain :
- start OCG
- get option
- detect file
- initiate an emulation
- parse filename
- create directory
- parse XML
- save XML
- call OAI emulator
- generate report
- end OCG

The following diagram is based on graphviz (http://www.graphviz.org/), you need to install the package to view the diagram.  

 * \dot
 * digraph ocg_flow_chart  {
 *     node [shape=rect, fontname=Helvetica, fontsize=14,style=filled,fillcolor=lightgrey];
 *     a [ label = " start OCG"];
 *     b1 [ label = " get option"];
 *     b2 [ label = " detect file"];
 *     c [ label = " initiate an emulation"];
 *     d [ label = " parse filename"];
 *     e [ label = " create directory"];
 *     f [ label = " parse XML"];
 *     g [ label = " save XML"];
 *     i [ label = " generate report"];
 *     j [ label = " end OCG"];
 *		a->b1;
 *    b1->b2 [ label = "OCG" ];
 *		b1->c [ label = "OCG -f filename" ];
 *    b1->j [ label = "OCG -h or command wrong" ];
 *    b2->c [ label = "file detected" ];
 *    b2->b2 [ label = "check every sec" ];
 *		c->d;
 *		d->e [ label = "OK" ];
 *    d->i [ label = "error" ];
 *		e->f [ label = "OK" ];
 *    e->i [ label = "error" ];
 *		f->g [ label = "OK" ];
 *    f->i [ label = "error" ];
 *    g->i [ label = "OK" ];
 *    g->i [ label = "error" ];
 *    i->b2;
 *	label = "OCG Flow Chart"
 *		
 * }
 * \enddot
 */

  enum
  {
    STATE_START_OCG,		/*!< \brief initiate OCG */
    STATE_GET_OPT,		/*!< \brief get options of OCG command */
    STATE_DETECT_FILE,		/*!< \brief detect the configuration file in folder USER_XML_FOLDER */
    STATE_INI_EMU,		/*!< \brief initiate an emulation after finding a configuration file */
    STATE_PARSE_FILENAME,	/*!< \brief parse the filename into user_name and file_date */
    STATE_CREATE_DIR,		/*!< \brief create directory for current emulation */
    STATE_PARSE_XML,		/*!< \brief parse the configuration file */
    STATE_SAVE_XML,		/*!< \brief save the configuration file to the created directory */
    STATE_CALL_EMU,		/*!< \brief call the emulator */
    STATE_GENERATE_REPORT,	/*!< \brief generate some information of OCG */
    STATE_END			/*!< \brief lead to an end of the OCG process */
  };
/* @}*/

// the OSD_basic : 



  typedef struct
  {
    char *selected_option;
    int free_space;
    int urban;
    int rural;
  } Large_Scale;

  ////// options of generic
  typedef struct
  {
    double pathloss_exponent;	// default 2
    double pathloss_0_dB;	// default -50 dB
  } Free_Space_Model_Parameters;

  typedef struct
  {
    char *selected_option;
    int SCM_A;			//3GPP 36.384 Spatial channel model A
    int SCM_B;
    int SCM_C;
    int SCM_D;
    int rayleigh_8tap;		// updated to advanced
    int ricean_8tap;
    int EPA;
    int EVA;
    int ETU;
  } Small_Scale;

  ////// options of Ricean_8Tap
  typedef struct
  {
    int rice_factor_dB;		// default 10 dB    
  } Ricean_8Tap;

  typedef struct
  {
    double decorrelation_distance_m;	// (m), default 100m
    double variance_dB;		// (dB), default 10dB
    double inter_site_correlation;	// (0...1), default 1, not yet implemented
  } Shadowing;

  typedef struct
  {
    Large_Scale large_scale;
    Small_Scale small_scale;
    Shadowing shadowing;

    Free_Space_Model_Parameters free_space_model_parameters;
    Ricean_8Tap ricean_8tap;
  } Fading;

  typedef struct
  {
    int number_of_sectors;	/// Number of sectors (1-3), for UE always 1
    double beam_width_dB;	/// Antenna 3dB beam width (in radians) (set to 2*M_PI for onmidirectional antennas), for UE always 2*M_PI
    double alpha_rad[3];	/// Antenna orientation for each sector (for non-omnidirectional antennas) in radians wrt north
    double antenna_gain_dBi;	/// Antenna gain (dBi) (same for Tx and Rx)
    double tx_power_dBm;	/// Tx power (dBm)
    double rx_noise_level_dB;	/// Rx noise level (dB)
    double antenna_orientation_degree[3];	// 3 is the number of sectors for OpenAir // we need think about how to parse it from the XML ??????
  } eNB_Antenna;

  typedef struct
  {
    double antenna_gain_dBi;	/// Antenna gain (dBi) (same for Tx and Rx)
    double tx_power_dBm;	/// Tx power (dBm)
    double rx_noise_level_dB;	/// Rx noise level (dB)
  } UE_Antenna;

  typedef struct
  {
    eNB_Antenna eNB_antenna;
    UE_Antenna UE_antenna;
  } Antenna;

/** @defgroup _envi_config Environment Configuration
 *  @ingroup _OSD_basic
 *  @brief Including simulation area, geography, topography, fading information, etc
 * @{*/
  typedef struct
  {
    Fading fading;
    double wall_penetration_loss_dB;
    double system_bandwidth_MB;
    double system_frequency_GHz;
    Antenna antenna;
  } Environment_System_Config;
/* @}*/


  typedef struct
  {
    double x_m;
    double y_m;
    //double zz; // for test
  } Area;

  typedef struct
  {
    char *selected_option;
    int homogeneous;
    int heterogeneous;
  } Network_Type;

  typedef struct
  {
    char *selected_option;
    int macrocell;
    int microcell;
    int picocell;
    int femtocell;
  } Cell_Type;

  typedef struct
  {
    int number_of_relays;
  } Relay;			// may not exist in the XML if RELAY is not selected by the user 

  typedef struct
  {
    char *selected_option;
    int fixed;			// static
    int random_waypoint;
    int random_walk;
    int grid_walk;
    int trace;
    int sumo;
  } UE_Mobility_Type;

  ////// options of UE_Mobility_Type
  typedef struct
  {
    int horizontal_grid;
    int vertical_grid;
  } Grid_Map;

  typedef struct
  {
    char *selected_option;
    int random_destination;	// TRIP_RANDOM
    int random_turn;		// TRIP_NONE
  } Grid_Trip_Type;

  typedef struct
  {
    Grid_Map grid_map;
    Grid_Trip_Type grid_trip_type;	// JHNote: modificaiton
  } Grid_Walk;
  //////

  typedef struct
  {
    char *selected_option;
    int random;
    int concentrated;
    int grid;
  } UE_Initial_Distribution;

  ////// options of UE_Initial_Distribution
  typedef struct
  {
    int number_of_nodes;
  } Random_UE_Distribution;

  typedef struct
  {
    int number_of_nodes;
  } Concentrated_UE_Distribution;

  typedef struct
  {
    int number_of_nodes;
  } Random_Grid;

  typedef struct
  {
    int number_of_nodes;
  } Border_Grid;

  typedef struct
  {
    char *selected_option;
    Random_Grid random_grid;	// random choice of vertex in Grid_Graph mobility
    Border_Grid border_grid;	// random choice of vertex located on the border in Grid_Graph mobility                
  } Grid_UE_Distribution;
  //////

  typedef struct
  {
    double min_speed_mps;	// m/s
    double max_speed_mps;
    double min_sleep_ms;
    double max_sleep_ms;
    double min_journey_time_ms;
    double max_journey_time_ms;
  } UE_Moving_Dynamics;		// now, we use uniform distribution for these dynamics
  typedef struct {
    char *trace_mobility_file;
  }Trace_Configuration;
  typedef struct
  {
    char *command;
    char *file;
    int start;
    int end;
    int step;
    char *hip;
    int hport;
  } SUMO_Configuration;
  typedef struct
  {
    UE_Mobility_Type UE_mobility_type;
    // ! Note: Grid_Walk is a UE_Mobility_Type, we put it here for the sake of simplicity of the XML file
    Grid_Walk grid_walk;

    UE_Initial_Distribution UE_initial_distribution;
    // ! Note: the following three options are for UE_Initial_Distribution, we put them here for the sake of simplicity of the XML file
    Random_UE_Distribution random_UE_distribution;
    Concentrated_UE_Distribution concentrated_UE_distribution;
    Grid_UE_Distribution grid_UE_distribution;

    UE_Moving_Dynamics UE_moving_dynamics;
    Trace_Configuration trace_config;
    SUMO_Configuration sumo_config;
  } UE_Mobility;

  typedef struct
  {
    char *selected_option;
    int fixed;
    int mobile;			// at this moment, it is OMG who decides this mobility type, so there is even no config of moving dynamics for eNB
  } eNB_Mobility_Type;

  typedef struct
  {
    char *selected_option;
    int random;
    int hexagonal;
    int grid;
  } eNB_Initial_Distribution;

  ////// options of eNB_Initial_Distribution
  typedef struct
  {
    int number_of_cells;
  } Random_eNB_Distribution;

  typedef struct
  {
    int number_of_cells;
    double inter_eNB_distance_km;
  } Hexagonal_eNB_Distribution;

  typedef struct
  {
    int number_of_grid_x;
    int number_of_grid_y;
  } Grid_eNB_Distribution;	// inter_eNB_distance = (x/num_x) and (y/num_y) for horizontal and vertical dimensions
  //////

  typedef struct
  {
    eNB_Mobility_Type eNB_mobility_type;

    eNB_Initial_Distribution eNB_initial_distribution;
    // ! Note: the following three options are for eNB_Initial_Distribution, we put them here for the sake of simplicity of the XML file
    Random_eNB_Distribution random_eNB_distribution;
    Hexagonal_eNB_Distribution hexagonal_eNB_distribution;
    Grid_eNB_Distribution grid_eNB_distribution;
  } eNB_Mobility;

  typedef struct
  {
    UE_Mobility UE_mobility;
    eNB_Mobility eNB_mobility;
  } Mobility;

/** @defgroup _topo_config Topology Configuration
 *  @ingroup _OSD_basic
 *  @brief Including cell type, eNB topology, UE distribution, mobility information, etc
 * @{*/
  typedef struct
  {
    Area area;
    Network_Type network_type;
    Cell_Type cell_type;
    Relay relay;
    Mobility mobility;
    int omv;
  } Topology_Config;
/* @}*/


  typedef struct
  {
    char *application_type[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
    char *source_id[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
    char *destination_id[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
    char *background[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
  } Predefined_Traffic;

  typedef struct
  {
	int pu;
	double prob_off_pu;
	double prob_pu_ed;
	double holding_time_off_pu;
	int pe;
	double prob_off_ed;
	double prob_ed_pe;
	double holding_time_off_ed;
	int ed;
	double holding_time_off_pe;
    
    char *application_type[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
    char *source_id[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
    char *destination_id[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
    char *traffic[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
    char *transport_protocol[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
    char *ip_version[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
    char *idt_dist[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
    int idt_min_ms[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
    int idt_max_ms[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
    double idt_standard_deviation[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
    double idt_lambda[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
    double idt_scale[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
    double idt_shape[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
    char *size_dist[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
    int size_min_byte[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
    int size_max_byte[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
    double size_standard_deviation[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
    double size_lambda[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
    double size_scale[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
    double size_shape[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
    int stream[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
    int destination_port[NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX];
  } Customized_Traffic;

/** @defgroup _app_config Application Configuration
 *  @ingroup _OSD_basic
 *  @brief Including application type and traffic information
 * @{*/
  typedef struct
  {
    char *packet_gen_type;
    Predefined_Traffic predefined_traffic;
    Customized_Traffic customized_traffic;
  } Application_Config;
/* @}*/

  typedef struct
  {
    char *throughput;
    char *latency;
    char *loss_rate;
    int signalling_overhead;
    char *owd_radio_access;
  } Performance_Metrics;

  typedef struct
  {
    int phy;
    int mac;
    int rlc;
    int rrc;
    int pdcp;
    int omg;
    int emu;
    int otg;
  } Layer;

  typedef struct
  {
    char * level;
    char *verbosity;
    int interval;
  } Log_Emu;

  typedef struct
  {
    int enabled;
    unsigned int start_time;
    unsigned int end_time;
  } Packet_Trace;

  typedef struct
  {
    int value;
  } Seed;

  
/** @defgroup _emu_config Emulation Configuration
 *  @ingroup _OSD_basic
 *  @brief Including emulation time and performance output
 * @{*/
  typedef struct
  {
    double emulation_time_ms;
    Performance_Metrics performance_metrics;
    Layer layer;
    Log_Emu log_emu;
    Packet_Trace packet_trace;
    Seed seed;
    char * curve;
    // CLI tags are in Info 
    //char g_log_level[20];
  } Emulation_Config;
/* @}*/

/** @defgroup  _OSD_basic Basic OpenAirInterface Scenario Descriptor
 *  @ingroup _OCG
 *  @brief OAI Emulation struct for OSD_basic
 * @{*/


  typedef struct
  {
    unsigned char nb_ue;
    unsigned char first_ue;
    unsigned char nb_enb;
    unsigned char first_enb;
  } master_info_t;

  typedef struct
  {
    char *output_path;		/*!< \brief The path where we generate all the emulation results */
    int ocg_ok;

    // distributed emulation params 
    master_info_t master[NUMBER_OF_MASTER_MAX];
    unsigned char nb_ue_local;
    unsigned char nb_ue_remote;
    unsigned char nb_enb_local;
    unsigned char nb_enb_remote;
    unsigned char first_enb_local;
    unsigned char first_ue_local;
    unsigned short master_id;
    unsigned char nb_master;
    unsigned int master_list;
    unsigned int is_primary_master;
    unsigned int ethernet_id;
    char local_server[128];	// for the oaisim -c option : 0 = EURECOM web portal; -1 = local; 1 - N or filename = running a specific XML configuration file 
    unsigned char ethernet_flag;
    unsigned char multicast_group;
    // status
    unsigned char ocg_enabled;
    unsigned char ocm_enabled;
    unsigned char opt_enabled;
    unsigned char opt_mode; 
    unsigned char otg_enabled; 
    unsigned char omv_enabled;
    unsigned char vcd_enabled;
    unsigned char otg_traffic;
    unsigned char otg_bg_traffic_enabled;
    unsigned char omg_model_enb;
    unsigned char omg_model_ue;
    unsigned char omg_model_ue_current;	// when mixed mbility is used 
    // control eNB/UE instance through CLI
    unsigned char cli_enabled;
    unsigned char cli_num_enb;
    unsigned char cli_start_enb[NUMBER_OF_eNB_MAX];
    unsigned char cli_num_ue;
    unsigned char cli_start_ue[NUMBER_OF_UE_MAX];
    unsigned char oai_ifup[NUMBER_OF_eNB_MAX+NUMBER_OF_UE_MAX];
    // emu related 
    unsigned int seed;
    unsigned int frame;
    double time_s;
    double time_ms;
    unsigned int g_log_level;
    unsigned int g_log_verbosity;
    // phy related params
    unsigned int n_frames;
    unsigned int n_frames_flag;	// if set, then let the emulation goes to infinity
    unsigned char frame_type;
    unsigned char tdd_config;
    unsigned char tdd_config_S;
    unsigned char extended_prefix_flag;
    unsigned char N_RB_DL;
    unsigned char transmission_mode;

    int max_predefined_traffic_config_index;
    int max_customized_traffic_config_index;

  } Info;
/* @}*/

/** @defgroup  _OSD_basic Basic OpenAirInterface Scenario Descriptor
 *  @ingroup _OCG
 *  @brief OAI Emulation struct for OSD_basic
 * @{*/
  typedef struct
  {
    Environment_System_Config environment_system_config;	/*!< \brief Evironment configuration */
    Topology_Config topology_config;	/*!< \brief Topology configuration */
    Application_Config application_config;	/*!< \brief Applications configuration */
    Emulation_Config emulation_config;	/*!< \brief Emulation configuration */
    Info info;			/*!< \brief Some important information which should be able to be reached by OAISIM */
    char *profile;
  } OAI_Emulation;
/* @}*/


/** @defgroup _fn Functions in OCG
 *  @ingroup _OCG
 *  @brief describing all the functions used by OCG
 * @{*/
  /* @} */

/** @defgroup _log_gen LOG GEN Commands
 *  @ingroup _OCG
 *  @brief using the following macro instead of "printf"
 * @{*/

//#define LOG_A printf("OCG: "); printf /*!< \brief alert */
//#define LOG_C printf("OCG: "); printf /*!< \brief critical */
//#define LOG_W printf("OCG: "); printf /*!< \brief warning */
//#define LOG_N printf("OCG: "); printf /*!< \brief notice */
//#define LOG_E printf("OCG: "); printf /*!< \brief error */
//#define LOG_I printf("OCG: "); printf /*!< \brief info */
//#define LOG_D printf("OCG: "); printf /*!< \brief debug */
//#define LOG_T printf("OCG: "); printf /*!< \brief trace */
/* @}*/

  int OCG_main (char is_local_server[FILENAME_LENGTH_MAX]);

  void init_oai_emulation (void);
  //#include "UTIL/LOG/log.h"

#ifdef __cplusplus
}
#endif

#endif
