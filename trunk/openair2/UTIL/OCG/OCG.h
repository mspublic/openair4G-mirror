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
* \author Lusheng Wang
* \date 2011
* \version 0.1
* \company Eurecom
* \email: lusheng.wang@eurecom.fr
* \note
* \warning
*/

#ifndef __OCG_H__
#define __OCG_H__

#ifdef __cplusplus
extern "C" {
#endif
/** @defgroup _OCG OpenAir Config Generation (OCG)
 * @{*/ 
/* @}*/ 

/** @defgroup _useful_macro Macro Definition
 *  @ingroup _OCG
 *  @brief the following macros will be used in the code of OCG
 * @{*/ 
#define MODULE_NOT_PROCESSED -9999 /*!< \brief the module state indicator is set to -9999 before the module being processed */
#define MODULE_ERROR -1 /*!< \brief the module state indicator is set to -1 for error */
#define MODULE_OK 0 /*!< \brief the module state indicator is set to 0 when successfully processed */
#define GET_HELP 1 /*!< \brief the module state indicator is set to 1 for get_opt_OK when the user types -h option */
#define NO_FILE -2 /*!< \brief the module state indicator is set to -2 for detect_file_OK when no file is detected */
#define WEB_XML_FOLDER "/nfs/webxml/" /*!< \brief the web portal generates XML files into this folder */
#define LOCAL_XML_FOLDER "local_XML/" /*!< \brief this folder contains some XML files for demo, users could also put their own XML files into this folder for a direct emulation without using the web portal*/
#define TEMP_OUTPUT_DIR "temp_output/" /*!< \brief temporary output files will be generated in this folder when folders for an emulation could not be created due to errors */
#define OUTPUT_DIR "/nfs/emu_results/" /*!< \brief this folder contains all the output files when folders for an emulation could be successfully created */
#define FILENAME_LENGTH_MAX 64 /*!< \brief the maximum length of a filename */
#define DIR_LENGTH_MAX 64 /*!< \brief the maximum length of the path name */
#define MOBI_XML_FOLDER "mobi_XML/" /*!< \brief the folder that mobigen generate XML files in */
#define DIR_TO_MOBIGEN "XML_to_mobigen/" /*!< \brief the folder that mobigen detects XML file from OCG */
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
 *     h [ label = " call OAI emulator"];
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
 *    g->h [ label = "OK" ];
 *    g->i [ label = "error" ];
 *    h->i;
 *    i->b2;
 *	label = "OCG Flow Chart"
 *		
 * }
 * \enddot
 */
 
enum {
	STATE_START_OCG, /*!< \brief initiate OCG */
	STATE_GET_OPT, /*!< \brief get options of OCG command */
	STATE_DETECT_FILE, /*!< \brief detect the configuration file in folder USER_XML_FOLDER */
	STATE_INI_EMU, /*!< \brief initiate an emulation after finding a configuration file*/
	STATE_PARSE_FILENAME, /*!< \brief parse the filename into user_name and file_date */
	STATE_CREATE_DIR, /*!< \brief create directory for current emulation */
	STATE_PARSE_XML, /*!< \brief parse the configuration file */
	STATE_SAVE_XML, /*!< \brief save the configuration file to the created directory */
	STATE_CALL_EMU, /*!< \brief call the emulator */
	STATE_GENERATE_REPORT, /*!< \brief generate some information of OCG */
	STATE_END/*!< \brief lead to an end of the OCG process */
};
/* @}*/

// the OSD_basic : 
		typedef struct {
			double x;
			double y;
		}Area;

		typedef struct {
			char *selected_option;
			int home;
			int urban;
			int rural;
		}Geography;

		typedef struct {
			char *selected_option;
			int flat;
			int obstructed;
			int hilly;
		}Topography;

				typedef struct {
					double pathloss_exponent;
					double pathloss_0;
				}Free_Space_Propagation;

						typedef struct {
							double delay_spread;
						}Rayleigh;
					
						typedef struct {
							double delay_spread;
						}Rician;
					
				typedef struct {
					char *selected_option;
					Rayleigh rayleigh;
					Rician rician;
				}Small_Scale;

		typedef struct {
			Free_Space_Propagation free_space_propagation;
			Small_Scale small_scale;
		}Fading;

/** @defgroup _envi_config Environment Configuration
 *  @ingroup _OSD_basic
 *  @brief Including simulation area, geography, topography, fading information, etc
 * @{*/ 
typedef struct {
	Area area;
	Geography geography;
	Topography topography;
	Fading fading;
	int wall_penetration_loss;
	double noise_power;
}Envi_Config;
/* @}*/

		typedef struct {
			char *selected_option;
			int homogeneous;
			int heterogeneous;
		}Net_Type;

			
		typedef struct {
			char *selected_option;
			int macrocell;
			int microcell;
			int picocell;
			int femtocell;
		}Cell_Type;
		
		typedef struct {
			int number_of_relays;
		}Relay; // may not exist in the XML if RELAY is not selected by the user 
			
				typedef struct {
					int x;
					int y;
				}Grid;
			
				typedef struct {
					int number_of_cells;
				}Hexagonal;
	
				typedef struct {
					int number_of_eNB;
				}Totally_Random;
			
		typedef struct {
			char *selected_option;
			Grid grid;
			Hexagonal hexagonal;
			Totally_Random totally_random;
		}eNB_Topology;

				typedef struct {
					double inter_block_distance;
				}Grid_Map;

		typedef struct {
			char *selected_option;
			Grid_Map grid_map;
		}UE_Distribution;

				typedef struct {
					char *selected_option;
					int fixed;
					int random_waypoint;
					int random_walk;
					int grid_walk;
				}Mobility_Type;
				
				typedef struct {
					double min_speed;
					double max_speed;
					double min_pause_time;
					double max_pause_time;
				}Moving_Dynamics;

		typedef struct {
			Mobility_Type mobility_type;
			Moving_Dynamics moving_dynamics;
		}Mobility;

/** @defgroup _topo_config Topology Configuration
 *  @ingroup _OSD_basic
 *  @brief Including cell type, eNB topology, UE distribution, mobility information, etc
 * @{*/ 
typedef struct {
	Net_Type net_type;
	Cell_Type cell_type;
	Relay relay;
	eNB_Topology eNB_topology;
	double inter_eNB_distance;
	UE_Distribution UE_distribution;
	int number_of_UE;
	double system_bandwidth;
	double UE_frequency;
	Mobility mobility;
}Topo_Config;
/* @}*/

		typedef struct {
			char *selected_option;
			int cbr;
			int gaming;
			int m2m;
		}App_Type;

				typedef struct {
					char *selected_option;
					int udp;
					int tcp;
				}Transport_Protocol;

						typedef struct {
							double fixed_value;
						}Fixed;
					
						typedef struct {
							double min_value;
							double max_value;
						}Uniform;
					
						typedef struct {
							double expected_inter_arrival_time;
						}Poisson;
					
				typedef struct {
					char *selected_option;
					Fixed fixed;
					Uniform uniform;
				}Packet_Size;	/*!< \brief Distribution of packet size  */

				typedef struct {
					char *selected_option;
					Fixed fixed;
					Uniform uniform;
					Poisson poisson;
				}Inter_Arrival_Time;	/*!< \brief Distribution of packet's inter-arrival time */

		typedef struct {
			Transport_Protocol transport_protocol;
			Packet_Size packet_size;
			Inter_Arrival_Time inter_arrival_time; 
		}Traffic;

/** @defgroup _app_config Application Configuration
 *  @ingroup _OSD_basic
 *  @brief Including application type and traffic information
 * @{*/ 
typedef struct {
	App_Type app_type;
	Traffic traffic;
}App_Config;
/* @}*/

				typedef struct {
					int throughput;
					int latency;
					int signalling_overhead;
				}Metric;

				typedef struct {
					int mac;
					int rlc;
					int pdcp;
				}Layer;

				typedef struct {
					int debug;
					int info;
					int warning;
					int error;
				}Log_Emu;
				
				typedef struct {
					int mac;
				}Packet_Trace;
				
		typedef struct {
			Metric metric;
			Layer layer;
			Log_Emu log_emu;
			Packet_Trace packet_trace;
		}Performance;
		
/** @defgroup _emu_config Emulation Configuration
 *  @ingroup _OSD_basic
 *  @brief Including emulation time and performance output
 * @{*/ 
typedef struct {
	double emu_time;
	Performance performance;
}Emu_Config;
/* @}*/

/** @defgroup  _OSD_basic Basic OpenAirInterface Scenario Descriptor
 *  @ingroup _OCG
 *  @brief OAI Emulation struct for OSD_basic
 * @{*/ 
typedef struct {
	Envi_Config envi_config;	/*!< \brief Evironment configuration */
	Topo_Config topo_config;	/*!< \brief Topology configuration */
	App_Config app_config;	/*!< \brief Applications configuration */
	Emu_Config emu_config;	/*!< \brief Emulation configuration */

	char *profile;
}OAI_Emulation;
/* @}*/

/** @defgroup _fn Functions in OCG
 *  @ingroup _OCG
 *  @brief describing all the functions used by OCG
 * @{*/ 
 /* @}*/
 
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

OAI_Emulation * OCG_main(void);

#include "UTIL/LOG/log_if.h"


#ifdef __cplusplus
}
#endif

#endif

