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
#include "otg_vars.h"
 

// Defining initial and default values of OTG structures 

void init_all_otg()  {

	//set otg params to 0
 	g_otg = calloc(1, sizeof(otg_t));
	if (g_otg == NULL)
   	/* Memory could not be allocated */
   		LOG_E(OTG,"Couldn't allocate memory for otg_t\n");
 	memset(g_otg, 0, sizeof(otg_t));		


	//set otg infos to 0
 	otg_info = calloc(1, sizeof(otg_info_t));
	if (otg_info == NULL)
   	/* Memory could not be allocated */
   		LOG_E(OTG,"Couldn't allocate memory for otg_info_t\n");
 	memset(otg_info, 0, sizeof(otg_info_t));

        //set otg forms infos to 0
	otg_forms_info=calloc(1, sizeof(otg_forms_info_t));
	if (otg_forms_info == NULL)
   	/* Memory could not be allocated */
   		LOG_E(OTG,"Couldn't allocate memory for otg_forms_info_t\n");
 	memset(otg_forms_info, 0, sizeof(otg_forms_info_t));

	LOG_I(OTG,"init done: init_all_otg\n");

}

char *str_sub (const char *s, unsigned int start, unsigned int end) {
   
  char *new_s = NULL;
  int i;
  
  if (s != NULL && start < end)   {
    new_s = malloc (sizeof (*new_s) * (end - start + 2));
    if (new_s != NULL) {
      for (i = start; i <= end; i++) {
	new_s[i-start] = s[i];
      }
      new_s[i-start] = '\0';
    }
    else {
	LOG_E(OTG,"Insufficient memory \n");
	exit (-1);
      }
  }
  return new_s;
}

// set the simulation time
void set_ctime(int ctime){
	otg_info->ctime=ctime;
}



// get the simulation time 
int get_ctime(void){
	return otg_info->ctime;
}


void free_otg(){
  int i,j,k; 
  
  for (i=0; i<g_otg->num_nodes; i++){
	if (NULL != g_otg->dst_ip[i]){
	  free(g_otg->dst_ip[i]);
	}
  }

  if (NULL !=g_otg){
	free(g_otg);
  }

  printf("OTG DEBUG TARMA: free_otg() called \n");
  for(i=0; i<NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX; i++){
	for(j=0; j<NUMBER_OF_eNB_MAX + NUMBER_OF_UE_MAX; j++){
	  for(k=0; k<MAX_NUM_APPLICATION; k++){
		if(otg_info->tarma_stream[i][j][k]){
		  free(otg_info->tarma_stream[i][j][k]);
		  printf("OTG DEBUG TARMA: freed tarma_stream[%d][%d][%d]\n",i,j,k);
		}
		if(otg_info->tarma_video[i][j][k]){
		  free(otg_info->tarma_video[i][j][k]);
		  printf("OTG DEBUG TARMA: freed tarma_video[%d][%d][%d]\n",i,j,k);
		}
		if(otg_info->background_stream[i][j][k]){
		  free(otg_info->background_stream[i][j][k]);
		  printf("OTG DEBUG TARMA: freed background_stream[%d][%d][%d]\n",i,j,k);
		}
	  }
	}
  }

  if (NULL !=otg_info){
	free(otg_info);
  }



}

