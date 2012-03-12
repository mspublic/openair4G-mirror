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

/*! \file otg.c
* \brief common function for otc tx and rx 
* \author N. Nikaein and A. Hafsaoui
* \date 2011
* \version 0.1
* \company Eurecom
* \email: openair_tech@eurecom.fr
* \note
* \warning
*/


#include "otg.h"



// Defining initial and default values of OTG structures 

void init_all_otg()  {

	//set otg params to 0
 	g_otg = calloc(1, sizeof(otg_t));
	if (g_otg == NULL)
   	/* Memory could not be allocated */
   		printf("Couldn't allocate memory\n");
 	memset(g_otg, 0, sizeof(otg_t));		

	//set otg infos to 0
 	otg_info = calloc(1, sizeof(otg_info_t));
	if (otg_info == NULL)
   	/* Memory could not be allocated */
   		printf("Couldn't allocate memory\n");
 	memset(otg_info, 0, sizeof(otg_info_t));


	//set otg header to 0
 	otg_hdr = calloc(1, sizeof(otg_hdr_t));
	if (otg_hdr == NULL)
   	/* Memory could not be allocated */
   		printf("Couldn't allocate memory\n");
 	memset(otg_hdr, 0, sizeof(otg_hdr_t));

}

// set the simulation time
void set_ctime(int ctime){
	otg_info->ctime=ctime;
	printf("ctime :: val =%d \n",otg_info->ctime);
}



// get the simulation time 
int get_ctime(void){
	return otg_info->ctime;
}


void free_addr_otg(){
int i, j, k; 
for (i=0; i<g_otg->num_nodes; i++){
	if (NULL != g_otg->dst_ip[i]){
		g_otg->dst_ip[i]=NULL;
		free(g_otg->dst_ip[i]);
		}
	}

	if (NULL !=g_otg){
		g_otg=NULL;
		free(g_otg);
	}


	if (NULL !=otg_info){
		otg_info=NULL;
		free(otg_info);
	}


	if (NULL !=otg_hdr){
		otg_hdr=NULL;
		free(otg_hdr);
	}



}

