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

/*! \file OMG.c
* \brief Main function containing the OMG interface
* \author  M. Mahersi,  J. Harri, N. Nikaein,
* \date 2011
* \version 0.1
* \company Eurecom
* \email: 
* \note
* \warning
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "omg.h"

#define frand() ((double) rand() / (RAND_MAX+1))
#ifndef STANDALONE
mapping nodes_type[] =
{
    {"eNB", eNB},
    {"UE", UE},
    {NULL, -1}
};

mapping nodes_status[] =
{
    {"sleeping", 0},
    {"moving", 1},
    {NULL, -1}
};

mapping mob_type[] =
{
    {"STATIC", STATIC},
    {"RWP", RWP},
	 {"RWALK", RWALK},
	 {"TRACE", TRACE},
    {NULL, -1}
};
#endif
NodePtr create_node(void) {
	NodePtr ptr;
	ptr = malloc(sizeof(node_struct));
	return ptr;
}

double randomGen(double a, double b){

    return ( rand()/(double)RAND_MAX ) * (b-a) + a;
}


MobilityPtr create_mobility(void) {
	MobilityPtr ptr;
	ptr = malloc(sizeof(mobility_struct));
	return ptr;
}


Node_list add_entry(NodePtr node, Node_list Node_Vector){
    Node_list entry = malloc(sizeof(node_list_struct));
    entry->node = node;
    entry->next = NULL;
    if (Node_Vector == NULL) {
    // LOG_D(OMG, "\nempty Node_list");
	//LOG_D(OMG, "\nadded elmt ID %d\n", entry->node->ID);
        return entry;
    }
    else {
        Node_list tmp = Node_Vector;
        while (tmp->next != NULL){
            tmp = tmp->next;
        }
        tmp->next = entry;
        //LOG_D(OMG, "\nnon empty Node_list");
	     //LOG_D(OMG, "added elmt ID %d\n", entry->node->ID);
	
        return Node_Vector;
    }
}


// display list of nodes
void display_node_list(Node_list Node_Vector){
    Node_list tmp = Node_Vector;
    if (tmp == NULL){
      LOG_D(OMG, "Empty Node_list\n");
    }	
    while (tmp != NULL){
  	 LOG_I(OMG," [%s] [%s] Node of ID %d is %s. Now, it is at location (%.3f, %.3f)\n", 
		map_int_to_str(mob_type, tmp->node->generator),
		map_int_to_str(nodes_type, tmp->node->type),  
		tmp->node->ID,
		map_int_to_str(nodes_status, tmp->node->mobile), 
		tmp->node->X_pos,
		tmp->node->Y_pos );

    //LOG_I(OMG, "node number %d\tstatus(fix/mobile) %d\tX_pos %.2f\tY_pos %.2f\tnode_type(eNB, UE)%d\t", tmp->node->ID,tmp->node->mobile, tmp->node->X_pos,tmp->node->Y_pos, tmp->node->type);
      //LOG_D(OMG, "mob->X_from %.3f\tmob->Y_from %.3f\tmob->X_to %.3f\tmob->Y_to %.3f\t", tmp->node->mob->X_from,tmp->node->mob->Y_from, tmp->node->mob->X_to, tmp->node->mob->Y_to );
      tmp = tmp->next;
    }
}

void display_node_position(int ID, int generator, int type, int mobile, double X, double Y){
  LOG_I(OMG," [%s] [%s] Node of ID %d is %s. Now, it is at location (%.2f, %.2f) \n", 
		map_int_to_str(mob_type, generator),
		map_int_to_str(nodes_type, type),  
		ID,
		map_int_to_str(nodes_status, mobile),
		X,
		Y
	);
}

Node_list filter(Node_list Vector, int node_type){
  Node_list tmp1, tmp2;
  tmp1 = Vector;
  tmp2 = NULL;
  while (tmp1 != NULL){
    if (tmp1->node->type == node_type){
      tmp2 = add_entry(tmp1->node, tmp2);
    }
    tmp1 = tmp1->next;
  }
  return tmp2;
}


Node_list remove_node(Node_list list, int nID, int node_type){
  
  int found;
  Node_list  current, previous;
  //int cond=0;
	//int i=0;
    if (list == NULL){  found = 1;} //false
    else{                             //start search
      current = list;
      while ((current != NULL) && (current->node->ID != nID) || (current->node->type != node_type )){
        previous = current;        //  previous hobbles after
        current = current->next;
      }
      //holds: current = NULL or  type != node_type or.., but not both
      if (current ==NULL) { 
        found= 1  ;
	LOG_E(OMG," Element to remove is not found\n "); 
        return NULL;
      }              //value not found
      else{
        found = 0; // true                value found
        if (current == list) {
          list = current->next  ;
          LOG_D(OMG,"Element to remove is found at beginning\n");
        }    
          
        else {
          previous->next = current->next;

        }
        
      }
    return list;
    }
  }



