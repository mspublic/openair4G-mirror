/*******************************************************************************

  Eurecom OpenAirInterface 2
  Copyright(c) 1999 - 2010 Eurecom

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

/*____________________________OPT/def.h___________________________
Authors: Ben Romdhanne Bilel, Navid NIKAIEN
Company: EURECOM
Emails:
*This file include all defined structure used in this module
*/
#ifndef opt_struc
#include "opt.h"
#endif
/***************************************************/
/**
 * Typedef
 * */
#define opt_struc 1;
#define Analyzer_max=250;
typedef enum pdu_type
{
MAC_PDU,
MAC_RA_PDU,
MAC_UL_PDU,
MAC_DL_PDU,
RRC_PDU
}pdu_type;
typedef enum opt_output_type
{
socket_output,
dump_output,
double_output
}opt_output_type;
/********************************************/

#ifndef ip
char* ip="127.0.0.2";
#endif
#ifndef port
int port= 1234;
#endif
FILE* fp;
int file_index=0;
opt_output_type output_type=socket_output; 
char *path;
int mode=1;
unsigned short RIV2nb_rb_LUT25_OPT[512];


extern unsigned int current_dlsch_cqi;
