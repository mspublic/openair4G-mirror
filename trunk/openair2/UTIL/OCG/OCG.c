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

/*! \file OCG.c
* \brief Main function containing the FSM of OCG
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
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "OCG.h"
#include "OCG_vars.h"
#include "OCG_detect_file.h"
#include "OCG_parse_filename.h"
#include "OCG_create_dir.h"
#include "OCG_parse_XML.h"
#include "OCG_save_XML.h"
#include "OCG_generate_report.h"
/*----------------------------------------------------------------------------*/
//#define TEST_OCG



#ifdef TEST_OCG 
int main(int argc, char *argv[]) {
#else
OAI_Emulation * OCG_main(char is_local_server[FILENAME_LENGTH_MAX]) { 
#endif  
	int state = STATE_START_OCG;
	char web_XML_folder[DIR_LENGTH_MAX] = "";
	char output_dir[DIR_LENGTH_MAX] = "";

	char *OPENAIR_TARGETS=getenv("OPENAIR_TARGETS");
	if (OPENAIR_TARGETS == NULL) {
		LOG_E(OCG, "please set the PATH for OPENAIR_TARGETS");
		exit(EXIT_FAILURE);
	}

	if (!strcmp(is_local_server, "0")) { // Eurecom web server
		strcpy(web_XML_folder, WEB_XML_FOLDER);
		strcat(output_dir, OUTPUT_DIR);
	} else { // local user
		strcpy(web_XML_folder, OPENAIR_TARGETS);
		strcpy(output_dir, OPENAIR_TARGETS);

		char *slash;
		slash = web_XML_folder + strlen(web_XML_folder) - 1;
		if (strcmp(slash, "/")) { // check if the path OPENAIR_TARGETS is ended with a '/'
			strcat(web_XML_folder, "/");
			strcat(output_dir, "/");
		}
		strcat(web_XML_folder, "SIMU/EXAMPLES/OSD/WEBXML/");
		strcat(output_dir, "SIMU/EXAMPLES/OSD/RESULTS/");
	}

	LOG_I(OCG, "Folder for detecting the XML configuration file is %s\n", web_XML_folder);
	LOG_I(OCG, "Folder for generating the results is %s\n", output_dir);

	while(state != STATE_END) {
	
		switch(state) {
			
			case STATE_START_OCG :
				LOG_I(OCG, "OCG starts ...\n\n");
				get_opt_OK = MODULE_NOT_PROCESSED; // -9999 is the initial value, representing the module not processed
				detect_file_OK = MODULE_NOT_PROCESSED;
				create_dir_OK = MODULE_NOT_PROCESSED;

				// to write the pid into a file 'OCG.pid' so that the web portal part could check if OCG is running
				/*
				pid_t pid;
				pid = getpid();
				FILE *OCG_pid;
				OCG_pid = fopen("OCG.pid", "w");
				fprintf(OCG_pid, "%d\n", pid);
				fclose(OCG_pid);
				*/
#ifdef TEST_OCG
				state = STATE_GET_OPT;

			case STATE_GET_OPT :
				get_opt_OK = get_opt(argc, argv);
				if (get_opt_OK == MODULE_OK) {
					strcpy(src_file, LOCAL_XML_FOLDER);
					copy_or_move = 1;
					state = STATE_INI_EMU;
				} else if (get_opt_OK == GET_HELP) state = STATE_END;
				else state = STATE_DETECT_FILE;
				break;
#else
				state = STATE_DETECT_FILE;
				get_opt_OK == MODULE_OK;
#endif

			case STATE_DETECT_FILE :
				strcpy(src_file, web_XML_folder);
				detect_file_OK = detect_file(src_file, is_local_server);
				if (detect_file_OK == MODULE_OK) {
					if ((!strcmp(is_local_server, "0")) || (!strcmp(is_local_server, "-1"))) copy_or_move = 2;
					state = STATE_INI_EMU;
				} else if (detect_file_OK == MODULE_ERROR) state = STATE_GENERATE_REPORT;
				else if (detect_file_OK == NO_FILE) {
					state = STATE_DETECT_FILE;
					sleep(1);
				}
				break;
				
			case STATE_INI_EMU : // before initiating an emu, an XML file should be found above 
				parse_filename_OK = MODULE_NOT_PROCESSED;
				create_dir_OK = MODULE_NOT_PROCESSED;
				parse_XML_OK = MODULE_NOT_PROCESSED;
				save_XML_OK = MODULE_NOT_PROCESSED;
				call_emu_OK = MODULE_NOT_PROCESSED;
				config_mobi_OK = MODULE_NOT_PROCESSED;
				generate_report_OK = MODULE_NOT_PROCESSED;
				LOG_I(OCG, "An emulation for file %s is initiated\n", filename);
				state = STATE_PARSE_FILENAME;
				break;

			case STATE_PARSE_FILENAME :
				strcat(src_file, filename);
				if ((parse_filename_OK = parse_filename(filename)) == MODULE_OK) state = STATE_CREATE_DIR;
				else {
					if (copy_or_move == 2) {
						remove(src_file);
						state = STATE_DETECT_FILE;
					} else state = STATE_GENERATE_REPORT;
				}
				break;
				
			case STATE_CREATE_DIR :
				if ((create_dir_OK = create_dir(output_dir, user_name, file_date)) == MODULE_OK) {
					state = STATE_PARSE_XML;
					strcpy(dst_dir, output_dir);
					strcat(dst_dir, user_name);
					strcat(dst_dir, "/");
					strcat(dst_dir, file_date);
					strcat(dst_dir, "/");
					oai_emulation.useful_info.output_path = &dst_dir; // information for other modules within OAI
				} else state = STATE_GENERATE_REPORT;
				break;
				
			case STATE_PARSE_XML :
				if ((parse_XML_OK = parse_XML(src_file)) == MODULE_OK) state = STATE_SAVE_XML;
				else {
					if (copy_or_move == 2) remove(src_file);
					state = STATE_GENERATE_REPORT;
				}
				break;
				
			case STATE_SAVE_XML :
			  if ((save_XML_OK = save_XML(copy_or_move, src_file, dst_dir, filename)) == MODULE_OK) 
				state = STATE_CALL_EMU;
			  else state = STATE_GENERATE_REPORT;
			  break;
			  
		        case STATE_CALL_EMU :

				if ((detect_file_OK == MODULE_OK) && (parse_filename_OK == MODULE_OK) && (create_dir_OK == MODULE_OK) && (parse_XML_OK == MODULE_OK) && (save_XML_OK == MODULE_OK)) { 
				// if the above tasks are all successful, we could tell the oaisim.c that everything is ready before running the emulation
					oai_emulation.useful_info.OCG_OK = 1;
				}
#ifdef TEST_OCG				
				call_emu_OK = call_emu(dst_dir);
				config_mobi_OK = config_mobi("RWMEmulator.xml", filename); // generate the XML for Mobigen
#endif
				state = STATE_GENERATE_REPORT;
				break;
				
			case STATE_GENERATE_REPORT :
				if (create_dir_OK != MODULE_OK) {
					// a temp folder is required when the output folder could not be correctly generated
					strcpy(dst_dir, OPENAIR_TARGETS);
					strcat(dst_dir, "SIMU/EXAMPLES/OSD/");
					strcat(dst_dir, TEMP_OUTPUT_DIR);
					struct stat st;
					if(stat(dst_dir, &st) != 0) { // if temp output directory does not exist, we create it here
						mkdir(dst_dir, S_IRWXU | S_IRWXG | S_IRWXO);
						LOG_I(OCG, "temp output directory %s is created", dst_dir);
					}
				} else {
					strcat(dst_dir, "SCENARIO/STATE/");
				}

				generate_report(dst_dir, "OCG_report.xml");
				if (copy_or_move == 1) state = STATE_END;
				else state = STATE_END;
				//else state = STATE_DETECT_FILE;
				break;
		}
	} // end while
	// Cleanup function for the XML library.
	xmlCleanupParser();
	// this is to debug memory for regression tests
	xmlMemoryDump();
	LOG_I(OCG, "... OCG ends\n\n");
#ifdef TEST_OCG
	return 0;
#else
	return &oai_emulation;
#endif
}
