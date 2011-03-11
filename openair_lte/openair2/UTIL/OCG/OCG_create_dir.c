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

/*! \file OCG_create_dir.c
* \brief Create directory for current emulation
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
#include "OCG.h"
#include "OCG_extern.h"
#include "OCG_create_dir.h"
/*----------------------------------------------------------------------------*/

int create_dir(char user_name[FILENAME_LENGTH_MAX / 2], char file_date[FILENAME_LENGTH_MAX / 2]) {

	char directory[FILENAME_LENGTH_MAX + DIR_LENGTH_MAX] = "";
	strcat(directory, OUTPUT_DIR);
	strcat(directory, user_name);
	
	mkdir(directory, 0777);

	strcat(directory, "/");
	strcat(directory, file_date);

	mkdir(directory, 0777);

	LOG_I(OCG, "Directory for current emulation is created\n");
	return MODULE_OK;
	
}
