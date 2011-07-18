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
#include <string.h>
#include "OCG.h"
#include "OCG_vars.h"
#include "OCG_detect_file.h"
#include "OCG_parse_filename.h"
#include "OCG_create_dir.h"
#include "OCG_parse_XML.h"
#include "OCG_save_XML.h"
#include "OCG_generate_report.h"
//#include "../include/log.h"
/*----------------------------------------------------------------------------*/
//#define TEST_OCG



#ifdef TEST_OCG 
int main(int argc, char *argv[]) {
#else
  OAI_Emulation * OCG_main(void){
#endif  
	int state = STATE_START_OCG;

    while(state != STATE_END) {
	
		switch(state) {
			
			case STATE_START_OCG :
				LOG_I(OCG, "OCG starts ...\n\n");
				get_opt_OK = MODULE_NOT_PROCESSED; // -9999 is the initial value, representing the module not processed 
				detect_file_OK = MODULE_NOT_PROCESSED;

				// to write the pid into a file 'OCG.pid' so that the web portal part could check if OCG is running
				pid_t pid;
				pid = getpid();
				FILE *OCG_pid;
				OCG_pid = fopen("OCG.pid", "w");
				fprintf(OCG_pid, "%d\n", pid);
				fclose(OCG_pid);
#ifdef TEST_OCG
				state = STATE_GET_OPT;

			case STATE_GET_OPT :
				get_opt_OK = get_opt(argc, argv);
				if (get_opt_OK == MODULE_OK) {
					strcpy(src_file, LOCAL_XML_FOLDER);
					local_or_web = 1;
					state = STATE_INI_EMU;
				} else if (get_opt_OK == GET_HELP) state = STATE_END;
				else state = STATE_DETECT_FILE;
				break;
#else
				state = STATE_DETECT_FILE;
				get_opt_OK == MODULE_OK;
#endif

			case STATE_DETECT_FILE :
				strcpy(src_file, WEB_XML_FOLDER);
				//detect_file_OK = detect_file(argc, argv, src_file);
				detect_file_OK = detect_file(src_file);
				if (detect_file_OK == MODULE_OK) {
					local_or_web = 2;
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
				LOG_I(OCG, "An emulation for this file is initiated\n", filename);
				state = STATE_PARSE_FILENAME;
				break;

			case STATE_PARSE_FILENAME :
				strcat(src_file, filename);
				if ((parse_filename_OK = parse_filename(filename)) == MODULE_OK) state = STATE_CREATE_DIR;
				else {
					if (local_or_web == 2) remove(src_file);
					state = STATE_GENERATE_REPORT;
				}
				break;
				
			case STATE_CREATE_DIR :
				if ((create_dir_OK = create_dir(user_name, file_date)) == MODULE_OK) {
					state = STATE_PARSE_XML;
					strcpy(dst_dir, OUTPUT_DIR);
					strcat(dst_dir, user_name);
					strcat(dst_dir, "/");
					strcat(dst_dir, file_date);
					strcat(dst_dir, "/");
				} else state = STATE_GENERATE_REPORT;
				break;
				
			case STATE_PARSE_XML :
				if ((parse_XML_OK = parse_XML(src_file)) == MODULE_OK) state = STATE_SAVE_XML;
				else {
					if (local_or_web == 2) remove(src_file);
					state = STATE_GENERATE_REPORT;
				}
				break;
				
			case STATE_SAVE_XML :
			  if ((save_XML_OK = save_XML(local_or_web, src_file, dst_dir, filename)) == MODULE_OK) 
			    state = STATE_CALL_EMU;
			  else state = STATE_GENERATE_REPORT;
			  break;
			  
		        case STATE_CALL_EMU :
#ifdef TEST_OCG				

		  call_emu_OK = call_emu(dst_dir);
				config_mobi_OK = config_mobi("RWMEmulator.xml", filename); // generate the XML for Mobigen
#endif
				state = STATE_GENERATE_REPORT;
				break;
				
			case STATE_GENERATE_REPORT :
				if (create_dir_OK != MODULE_OK) generate_report(TEMP_OUTPUT_DIR, "OCG_report.xml");
				else generate_report_OK = generate_report(dst_dir, "OCG_report.xml");
				if (local_or_web == 1) state = STATE_END;
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
