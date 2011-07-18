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

/*! \file OCG_detect_file.c
* \brief Detect if a new XML is generated from the web portal
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
#include <dirent.h>
#include "OCG.h"
#include "OCG_extern.h"
#include "OCG_detect_file.h"
/*----------------------------------------------------------------------------*/
	
//int detect_file(int argc, char *argv[], char src_dir[DIR_LENGTH_MAX]) {
int detect_file(char src_dir[DIR_LENGTH_MAX]) {
	DIR *dir = NULL; 
	struct dirent *file = NULL;
	
	if((dir = opendir(src_dir)) == NULL) {
		LOG_E(OCG, "directory %s not found\n", src_dir);
		return MODULE_ERROR;
	}

	while((file = readdir(dir)) != NULL) {
		if(strcmp(file->d_name, ".") && strcmp(file->d_name, "..")) {
		  if(file->d_type == DT_DIR) { // does this mean there are multiple files ? 
		    //				detect_file(argc, argv, strncat(src_dir, file->d_name, FILENAME_LENGTH_MAX + DIR_LENGTH_MAX));
			  detect_file(strncat(src_dir, file->d_name, FILENAME_LENGTH_MAX + DIR_LENGTH_MAX));
			} else {
				if (strlen(file->d_name) <= FILENAME_LENGTH_MAX) {
					strcpy(filename, file->d_name);
					LOG_I(OCG, "Configuration file \"%s\" is detected\n", filename);
					return MODULE_OK;
				} else {
					LOG_E(OCG, "File name too long: char filename[] should be less than 64 characters\n");
					return MODULE_ERROR;
				}
			}
		}
	}

	closedir(dir);
	return NO_FILE;
}
