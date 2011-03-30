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
			double x_;
			double y_;
		}Area_;

		typedef struct {
			char *selected_option_;
			int home_;
			int urban_;
			int rural_;
		}Geography_;

		typedef struct {
			char *selected_option_;
			int flat_;
			int obstructed_;
			int hilly_;
		}Topography_;

				typedef struct {
					double pathloss_exponent_;
					double pathloss_0_;
				}Free_Space_Propagation_;

						typedef struct {
							double delay_spread_;
						}Rayleigh_;
					
						typedef struct {
							double delay_spread_;
						}Rician_;
					
				typedef struct {
					char *selected_option_;
					Rayleigh_ rayleigh_;
					Rician_ rician_;
				}Small_Scale_;

		typedef struct {
			Free_Space_Propagation_ free_space_propagation_;
			Small_Scale_ small_scale_;
		}Fading_;

/** @defgroup _envi_config Environment Configuration
 *  @ingroup _OSD_basic
 *  @brief Including simulation area, geography, topography, fading information, etc
 * @{*/ 
typedef struct {
	Area_ area_;
	Geography_ geography_;
	Topography_ topography_;
	Fading_ fading_;
	int wall_penetration_loss_;
	double noise_power_;
}Envi_Config_;
/* @}*/

		typedef struct {
			char *selected_option_;
			int homogeneous_;
			int heterogeneous_;
		}Net_Type_;

			
		typedef struct {
			char *selected_option_;
			int macrocell_;
			int microcell_;
			int picocell_;
			int femtocell_;
		}Cell_Type_;
		
		typedef struct {
			int number_of_relays_;
		}Relay_; // may not exist in the XML if RELAY is not selected by the user 
			
				typedef struct {
					int x_;
					int y_;
				}Grid_;
			
				typedef struct {
					int number_of_cells_;
				}Hexagonal_;
	
				typedef struct {
					int number_of_eNB_;
				}Totally_Random_;
			
		typedef struct {
			char *selected_option_;
			Grid_ grid_;
			Hexagonal_ hexagonal_;
			Totally_Random_ totally_random_;
		}eNB_Topology_;

				typedef struct {
					int number_of_UE_;
				}Totally_Random_UE_;
			
				typedef struct {
					int number_of_UE_;
				}Concentrated_;

				typedef struct {
					int number_of_UE_;
					double inter_block_distance_;
				}Grid_Map_;

		typedef struct {
			char *selected_option_;
			Totally_Random_UE_ totally_random_;
			Concentrated_ concentrated_;
			Grid_Map_ grid_map_;
		}UE_Distribution_;

				typedef struct {
					char *selected_option_;
					int fixed_;
					int random_waypoint_;
					int random_walk_;
					int grid_walk_;
				}Mobility_Type_;
				
				typedef struct {
					double min_speed_;
					double max_speed_;
					double min_pause_time_;
					double max_pause_time_;
				}Moving_Dynamics_;

		typedef struct {
			Mobility_Type_ mobility_type_;
			Moving_Dynamics_ moving_dynamics_;
		}Mobility_;

/** @defgroup _topo_config Topology Configuration
 *  @ingroup _OSD_basic
 *  @brief Including cell type, eNB topology, UE distribution, mobility information, etc
 * @{*/ 
typedef struct {
	Net_Type_ net_type_;
	Cell_Type_ cell_type_;
	Relay_ relay_;
	eNB_Topology_ eNB_topology_;
	double inter_eNB_distance_;
	UE_Distribution_ UE_distribution_;
	double system_bandwidth_;
	double UE_frequency_;
	Mobility_ mobility_;
}Topo_Config_;
/* @}*/

		typedef struct {
			char *selected_option_;
			int cbr_;
			int gaming_;
			int m2m_;
		}App_Type_;

				typedef struct {
					char *selected_option_;
					int udp_;
					int tcp_;
				}Transport_Protocol_;

						typedef struct {
							double fixed_value_;
						}Fixed_;
					
						typedef struct {
							double min_value_;
							double max_value_;
						}Uniform_;
					
						typedef struct {
							double expected_inter_arrival_time_;
						}Poisson_;
					
				typedef struct {
					char *selected_option_;
					Fixed_ fixed_;
					Uniform_ uniform_;
				}Packet_Size_;	/*!< \brief Distribution of packet size  */

				typedef struct {
					char *selected_option_;
					Fixed_ fixed_;
					Uniform_ uniform_;
					Poisson_ poisson_;
				}Inter_Arrival_Time_;	/*!< \brief Distribution of packet's inter-arrival time */

		typedef struct {
			Transport_Protocol_ transport_protocol_;
			Packet_Size_ packet_size_;
			Inter_Arrival_Time_ inter_arrival_time_; 
		}Traffic_;

/** @defgroup _app_config Application Configuration
 *  @ingroup _OSD_basic
 *  @brief Including application type and traffic information
 * @{*/ 
typedef struct {
	App_Type_ app_type_;
	Traffic_ traffic_;
}App_Config_;
/* @}*/

				typedef struct {
					int throughput_;
					int latency_;
					int signalling_overhead_;
				}Metric_;

				typedef struct {
					int mac_;
					int rlc_;
					int pdcp_;
				}Layer_;

				typedef struct {
					int debug_;
					int info_;
					int warning_;
					int error_;
				}Log_Emu_;
				
				typedef struct {
					int mac_;
				}Packet_Trace_;
				
		typedef struct {
			Metric_ metric_;
			Layer_ layer_;
			Log_Emu_ log_emu_;
			Packet_Trace_ packet_trace_;
		}Performance_;
		
/** @defgroup _emu_config Emulation Configuration
 *  @ingroup _OSD_basic
 *  @brief Including emulation time and performance output
 * @{*/ 
typedef struct {
	double emu_time_;
	Performance_ performance_;
}Emu_Config_;
/* @}*/

/** @defgroup  _OSD_basic Basic OpenAirInterface Scenario Descriptor
 *  @ingroup _OCG
 *  @brief OAI Emulation struct for OSD_basic
 * @{*/ 
typedef struct {
	Envi_Config_ envi_config_;	/*!< \brief Evironment configuration */
	Topo_Config_ topo_config_;	/*!< \brief Topology configuration */
	App_Config_ app_config_;	/*!< \brief Applications configuration */
	Emu_Config_ emu_config_;	/*!< \brief Emulation configuration */

	char *profile_;
}OAI_Emulation_;
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

OAI_Emulation_ * OCG_main(void);


#include "UTIL/LOG/log_if.h"


#ifdef __cplusplus
}
#endif

#endif

