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

/*! \file id_manager.c
* \brief Main function containing the OMG ID Manager
* \author  J. Harri
* \date 2012
* \version 0.1
* \company Eurecom
* \email: 
* \note
* \warning
*/

#include "id_manager.h"
#include <stdio.h>
#include <string.h>

MapPtr create_map(void) {
	MapPtr ptr;
	ptr = malloc(sizeof(map_struct));
	return ptr;
}

IDManagerPtr create_IDManager(void) {
	IDManagerPtr ptr;
	ptr = malloc(sizeof(id_manager_struct));
        ptr->map_oai2sumo = NULL;
        ptr->map_sumo2oai = NULL;
	return ptr;
}

Map_list add_map_entry(MapPtr map, Map_list Map_Vector){
    Map_list entry = malloc(sizeof(map_list_struct));
    entry->map = map;
    entry->next = NULL;
    if (Map_Vector == NULL) {
         //printf("Map-vector is NULL, assigning a new entry\n");
        return entry;
    }
    else {
        Map_list tmp = Map_Vector;
        while (tmp->next != NULL){
           //printf("getting the tail...\n");
            tmp = tmp->next;
        }
        //printf("got it...adding entry...\n");
        tmp->next = entry;
	
        return Map_Vector;
    }
}

Map_list remove_entry(MapPtr map, Map_list Map_Vector){
    Map_list entry = malloc(sizeof(map_list_struct));
    entry->map = map;
    entry->next = NULL;
    if (Map_Vector == NULL) {
        return entry;
    }
    else {
        Map_list tmp = Map_Vector;
        while (tmp->next != NULL){
            tmp = tmp->next;
        }
        tmp->next = entry;
	
        return Map_Vector;
    }
}

char* get_SumoID_by_OAI(int oai_id, IDManagerPtr ID_manager) {
  if(ID_manager->map_oai2sumo == NULL)
    return NULL;

  else {
    return get_sumo_entry(oai_id, ID_manager->map_oai2sumo);
  } 
}

int get_oaiID_by_SUMO(char *sumo_id, IDManagerPtr ID_manager) {
  if(ID_manager->map_sumo2oai == NULL) {
     //printf("ID_Manager: get_oaiID_by_SUMO: uninitialized map\n");
     return -1;
}

  else {
      //printf("ID_Manager: get_oaiID_by_SUMO: OAI_entry is: %d \n", get_oai_entry(sumo_id, ID_manager->map_sumo2oai));
     return get_oai_entry(sumo_id, ID_manager->map_sumo2oai);
  } 
}

void remove_oaiID_by_SUMO(char *sumo_id, IDManagerPtr ID_manager) {
  if(ID_manager->map_sumo2oai == NULL) {
     printf("ID_Manager: remove_oaiID_by_SUMO: uninitialized map\n");
    return;
  }
  else {
      ID_manager->map_sumo2oai =  remove_oai_entry(sumo_id, ID_manager->map_sumo2oai);
      if(ID_manager->map_oai2sumo !=NULL) {
        ID_manager->map_oai2sumo = remove_oai_entry(sumo_id, ID_manager->map_oai2sumo); // need to remove in the other list as well
      }
      
      //return ID_manager->map_sumo2oai; 
       
  } 
}

char* get_sumo_entry(int oai_id, Map_list Map_Vector) {
    Map_list tmp = Map_Vector;
    
    if(Map_Vector == NULL) {
       printf("bug here..should not be NULL");
    }
     
    if(Map_Vector->map == NULL) {
       printf("bug here..map should have been initialized");
    } 
    
    if (tmp->map->oai_id == oai_id) {
       //printf("got it...at the head and value is %s \n",tmp->map->sumo_id);
       return tmp->map->sumo_id;
    }
    else {
      //printf("here...\n");
      while (tmp->next != NULL){
            tmp = tmp->next;
            if (tmp->map->oai_id == oai_id) {
      		//printf("got it...in main value is %s \n",tmp->map->sumo_id);
                return tmp->map->sumo_id; 
	    }
      }
    }
    return NULL;
}

int get_oai_entry(char *sumo_id, Map_list Map_Vector) {
    Map_list tmp = Map_Vector;
    if (strcmp(tmp->map->sumo_id, sumo_id) == 0) {
       //printf("found it %s \n",tmp->map->sumo_id);
       return tmp->map->oai_id;
    }
    else {
      while (tmp->next != NULL){
            tmp = tmp->next;
             if (strcmp(tmp->map->sumo_id, sumo_id) == 0) {
                return tmp->map->oai_id;
	     }
      }
    }
    return -1;
}

Map_list remove_oai_entry(char *sumo_id, Map_list Map_Vector) {
    Map_list tmp = Map_Vector;
    Map_list entry;
    //printf("removing entry %s \n",sumo_id);	
    if (strcmp(tmp->map->sumo_id, sumo_id) == 0) {
       //printf("1: found it %s \n",tmp->map->sumo_id);
       int id = tmp->map->oai_id;
    //   free(tmp);
       if(tmp->next == NULL)
         return NULL;

        else {
          return tmp->next;
        }
       //if(strcmp(tmp->map->sumo_id, "0") == 0) {
    	  //printf("OAI ID is %d \n",id);	
          //exit(-1);
       //}
       //return id;
    }
    else {
      //printf("removing entry %s \n",sumo_id);	
      while (tmp->next != NULL){
             if (strcmp(tmp->next->map->sumo_id, sumo_id) == 0) {
                //printf("2: found it %s \n",tmp->next->map->sumo_id);
                int id = tmp->next->map->oai_id;
                entry =  tmp->next;   // save the entry to remove 
                tmp->next = tmp->next->next; // jump over the entry to be removed
              //  free(entry); // freeing the entry
               //if(strcmp(tmp->next->map->sumo_id, "0") == 0)
    		//  exit(-1);
                
               //return id;
             }
             tmp = tmp->next;
      }
    }
    return Map_Vector;
}


