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

/*! \file trace.c
* \brief The trace-based mobility model for OMG/OAI (mobility is statically imported from a file)
* \author  S. Uppoor
* \date 2011
* \version 0.1
* \company INRIA
* \email: sandesh.uppoor@inria.fr
* \note
* \warning
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "trace.h"
#include "omg.h"
#include "mobility_parser.h"

extern node_info* head_node_info;
extern hash_table_t* table;

int start_trace_generator(omg_global_param omg_param_list) {

  NodePtr node = NULL;
  // MobilityPtr mobility = NULL;

  //read the mobility file here
  table=read_mobility_file(omg_param_list.mobility_file); // JHNOTE: in order to debug, please give and change name here...
  LOG_W(OMG, "mobility file %s put in the table %p\n",omg_param_list.mobility_file, table );
  
  //sort_veh_movement(table);
  
  if (omg_param_list.nodes <= 0){
    LOG_W(OMG, "Number of nodes has not been set\n");
    return(-1);
  }
  /*
  // check and match number of nodes in mobility file provided
  if (omg_param_list.nodes != get_num_nodes()){
      LOG_W(OMG, "Make sure all nodes have mobility description in mobility file\n");
      return(-1);
  }*/

  if (omg_param_list.nodes_type == eNB) {
	LOG_I(OMG, "Node type has been set to eNB\n");
  } else if (omg_param_list.nodes_type == UE) {
	LOG_I(OMG, "Node type has been set to UE\n");
  }
  LOG_I(OMG, "Number of TRACE nodes has been set to %d\n", omg_param_list.nodes);
   

  int count = 0;
  node_info * head_node = head_node_info;
  while (head_node->next!=NULL){
    if (count<omg_param_list.nodes){
      LOG_D(OMG,"TRACE head node %p for node id %d\n", head_node,count);
      create_trace_node(node,head_node);
      count++;
      head_node=head_node->next;
    }else
      break;
    
  }
  if (count!=omg_param_list.nodes)
    create_trace_node(node,head_node);
  
  return(0);
}

int create_trace_node(NodePtr node,node_info *head_node){
	
  double cur_time = 1;//.10;
	MobilityPtr mobility = NULL;
	node = (NodePtr) create_node();
	mobility = (MobilityPtr) create_mobility();
	node->mobile = 0;  // static for the moment
	node->ID = head_node->vid;
	node->type = omg_param_list.nodes_type; // UE eNB
	node->generator = omg_param_list.mobility_type;   // STATIC, RWP...
	node->mob = mobility;

	place_trace_node(node);	//initial positions

	Pair pair = malloc (sizeof(Pair));
	pair = sleep_trace_node(node,cur_time,0); //sleep , assumed that initial node position exist in mobility file with speed 0

	Job_Vector = add_job(pair, Job_Vector);
	Job_Vector_len ++;

	if (Job_Vector == NULL)
	   LOG_E(OMG, "Job Vector is NULL\n");
	return 0;
}

void place_trace_node(NodePtr node) {
	Exnode* next_loc=NULL;
	next_loc=get_next_position(table,node->ID);
        
	node->X_pos = next_loc->x;
	node->mob->X_from = node->X_pos;
	node->mob->X_to = node->X_pos;
	node->Y_pos = next_loc->y;
	node->mob->Y_from = node->Y_pos;
	node->mob->Y_to = node->Y_pos;

	node->mob->speed = next_loc->speed;
	node->mob->journey_time = 0.0;

	LOG_D(OMG,"--------INITIALIZE TRACE NODE-------- \n ");
  	LOG_I(OMG,"Initial position of node ID: %d type: %d (X = %.2f, Y = %.2f) speed = 0.0\n ", node->ID, node->type, node->X_pos, node->Y_pos);
	Node_Vector[TRACE] = (Node_list) add_entry(node, Node_Vector[TRACE]);
    	Node_Vector_len[TRACE]++;
	
}

// sleep just after creation and after mobility description is over 
Pair sleep_trace_node(NodePtr node, double cur_time,float sleep_duration){
        
	node->mobile = 0;
	node->mob->speed = 0.0;
	node->mob->X_from = node->mob->X_to;
	node->mob->Y_from = node->mob->Y_to;
	node->X_pos = node->mob->X_to;
	node->Y_pos = node->mob->Y_to;
	Pair pair = malloc(sizeof(Pair)) ;

	node->mob->sleep_duration = sleep_duration ;
	LOG_D(OMG, "node: %d \tsleep duration : %.2f\n",node->ID, node->mob->sleep_duration);

	node->mob->start_journey = cur_time;
	pair->a = node->mob->start_journey + node->mob->sleep_duration; //when to wake up
        
	LOG_D(OMG, "to wake up at time: cur_time + sleep_duration : %.2f\n", pair->a);
	pair->b = node;

	return pair;
}


Pair move_trace_node(NodePtr node, double cur_time) {
        
	Exnode* next_loc=NULL;
	next_loc=get_next_position(table,node->ID);

	/* Trace defined time varies with OAI time :(
        if ((next_loc->time - cur_time) >= eps){ // this is for broken mobility description where node has to begin trip at some later time from same current position
		reset_visit_status(table,next_loc->time,node->ID);
		float sleep_duration=(next_loc->time-cur_time-eps);
		return sleep_trace_node(node,cur_time,sleep_duration); // correct this send the duration of sleep
	}*/
        
        Job_list tmp1 = Job_Vector;
        if (next_loc==NULL) {
          printf("OMG sleeping\n");     
	  return sleep_trace_node(node,cur_time,9999);                
                
                }//Option 1 : no job, I am done sleeping

        while((next_loc->x==tmp1->pair->b->mob->X_to && next_loc->y==tmp1->pair->b->mob->Y_to) || next_loc->speed==0)
              next_loc=get_next_position(table,node->ID);  
        

	Pair pair = malloc(sizeof(Pair));
	LOG_D(OMG, "MOVE TRACE NODE\n");
	LOG_D(OMG, "node: %d\n",node->ID );
	node->mob->X_from = node->X_pos;
	node->mob->Y_from = node->Y_pos;
	LOG_I(OMG, "Current Position: (%.2f, %.2f)\n", node->mob->X_from, node->mob->Y_from);
	
	node->mobile = 0;
	double X_next;
	double Y_next;
	if (next_loc!=NULL) X_next = next_loc->x;// veh finished travelling
	else X_next = node->X_pos; //no description ..stay at the last loc
	node->mob->X_to = X_next;
	if (next_loc!=NULL) Y_next = next_loc->y;
	else Y_next = node->Y_pos; //no description ..stay at the last loc
	node->mob->Y_to = Y_next;
	LOG_I(OMG, "destination: (%.2f, %.2f)\n", node->mob->X_to, node->mob->Y_to);
	double speed_next;
	if (next_loc!=NULL) speed_next = next_loc->speed;
	else speed_next = 0.0; //no mobility description available
	node->mob->speed = speed_next;
    	LOG_D(OMG, "speed_next %.2f\n", speed_next); //m/s

	double distance = (double) ((int)(sqrtf(pow(node->mob->X_from - X_next, 2) + pow(node->mob->Y_from - Y_next, 2))*100))/ 100;
	LOG_D(OMG, "distance %.2f\n", distance); //m
        
        double journeyTime_next =  (double) ((int)(distance/speed_next*100))/ 100;   //duration to get to dest ;
        
	////node->mob->journey_time = journeyTime_next;
	node->mobile = 1;
    	LOG_D(OMG, "mob->journey_time_next %.2f\n",journeyTime_next );

	node->mob->start_journey = cur_time;
   	LOG_D(OMG, "start_journey %.2f\n", node->mob->start_journey );
        
	pair->a = node->mob->start_journey + journeyTime_next;      //when to reach the destination
	LOG_D(OMG, "when to reach the destination: pair->a= start journey + journey_time next =%.2f\n", pair->a);

	pair->b = node;
	return pair;
	
}

void update_trace_nodes(double cur_time) {
   
   LOG_D(OMG, "--------UPDATE--------\n");
   Job_list tmp = Job_Vector;
   int done = 0; //
   
   while ((tmp != NULL) && (done == 0)){
     //  if 	(tmp->pair == NULL){LOG_E(OMG, "UPDATE TRACE : tmp->pair ==NULL\n" );}
     //  if 	(tmp->pair != NULL){LOG_E(OMG, "UPDATE TRACE : tmp->pair !=NULL\n" );}
     LOG_D(OMG, "cur_time %f\n", cur_time );
     LOG_D(OMG, "tmp->pair->a  %f\n", tmp->pair->a  );

     if((tmp->pair !=NULL) && ( (double)tmp->pair->a >= cur_time - eps) && ( (double)tmp->pair->a <= cur_time + eps) ) {
       if (tmp->pair->b->generator == TRACE){
	 LOG_D(OMG, " (first_job_time) %.2f == %.2f (cur_time) \n ",tmp->pair->a, cur_time );
	 LOG_D(OMG, " UPDATE TRACE \n ");
	 NodePtr my_node = (NodePtr)tmp->pair->b;

	 if (my_node->mobile == 1 || my_node->mobile == 0) { //only move no stop , put to sleep in move_trace_node() only for disconnected mobility
	   LOG_D(OMG, " node %d ready to move to next destination \n",  my_node->ID);
	   my_node->mobile = 1;
	   Pair pair = malloc(sizeof(Pair));
	   pair = move_trace_node(my_node, cur_time);
           
	   tmp->pair = pair;
	   tmp = tmp->next;
	 }
	 else{
	   LOG_E(OMG, "update_generator: unsupported node state - mobile : %d \n", my_node->mobile);
	   exit(-1);
	 }
       }
       else {
	 LOG_D(OMG, " (first_job_time) %.2f == %.2f(cur_time) but (generator=%d) != (TRACE=%d)\n ",tmp->pair->a, cur_time, tmp->pair->b->generator, TRACE );
	 tmp = tmp->next;
       }
     }
     else if ( (tmp->pair != NULL) && (cur_time < tmp->pair->a ) ){  //&& (tmp->pair->b->generator == RWP)
       LOG_D(OMG, "%.2f < %.2f \n",cur_time, tmp->pair->a);
       LOG_D(OMG, "Nothing to do\n");
       done = 1;  //quit the loop
     }
     else {
       LOG_E(OMG, "%.2f > %.2f\n", cur_time,tmp->pair->a   );   //LOG_D(OMG, " (generator=%d) != (RWP=%d) \n", tmp->pair->b->generator,  RWP );
       done = 1;  //quit the loop
       exit(-1);
     }
 }
   //sorting the new entries
   LOG_D(OMG, "--------DISPLAY JOB LIST--------\n"); //LOG_T
   display_job_list(Job_Vector);
   Job_Vector = quick_sort (Job_Vector);///////////
   LOG_D(OMG, "--------DISPLAY JOB LIST AFTER SORTING--------\n");
   display_job_list(Job_Vector);
}

void get_trace_positions_updated(double cur_time){

  double X_now=0.0;
  double Y_now=0.0;
  LOG_D(OMG, "--------GET TRACE POSITIONS--------\n");

  Pair my_pair = Job_Vector->pair;
  if ( (my_pair !=NULL) && (cur_time <= my_pair->a )){
    LOG_D(OMG, "%.2f <= %.2f\n ",cur_time, my_pair->a);
    Job_list tmp = Job_Vector;
    while (tmp != NULL){
      if (tmp->pair->b->generator == TRACE){
        if (tmp->pair->b->mobile == 0){ //node is sleeping
          LOG_T(OMG, "node number %d is sleeping at location: (%.2f, %.2f)\n", tmp->pair->b->ID, tmp->pair->b->X_pos, tmp->pair->b->Y_pos);
          LOG_T(OMG, "nothing to do\n");
        }
        else if (tmp->pair->b->mobile == 1){ //node is moving
          LOG_T(OMG, "Node_number %d\n", tmp->pair->b->ID);
          LOG_T(OMG, "destination not yet reached\tfrom (%.2f, %.2f)\tto (%.2f, %.2f)\tspeed %.2f\t(X_pos %.2f\tY_pos %.2f)\n", tmp->pair->b->mob->X_from, tmp->pair->b->mob->Y_from,tmp->pair->b->mob->X_to, tmp->pair->b->mob->Y_to,tmp->pair->b->mob->speed, tmp->pair->b->X_pos, tmp->pair->b->Y_pos);

          double len = sqrtf(pow(tmp->pair->b->mob->X_from - tmp->pair->b->mob->X_to,2)+pow(tmp->pair->b->mob->Y_from - tmp->pair->b->mob->Y_to,2));
          double dx = fabs(tmp->pair->b->mob->X_from - tmp->pair->b->mob->X_to) / len;

          double dy = fabs(tmp->pair->b->mob->Y_from - tmp->pair->b->mob->Y_to) / len;
          LOG_D(OMG, "len %f\tdx %f\tdy %f\n", len, dx, dy);
          if (tmp->pair->b->mob->X_from < tmp->pair->b->mob->X_to ){
            X_now = tmp->pair->b->mob->X_from + (dx * (tmp->pair->b->mob->speed * (cur_time - tmp->pair->b->mob->start_journey) ) );
          }
          else{
            X_now = tmp->pair->b->mob->X_from - (dx * (tmp->pair->b->mob->speed * (cur_time - tmp->pair->b->mob->start_journey)));
          }

          if (tmp->pair->b->mob->Y_from < tmp->pair->b->mob->Y_to ){
            Y_now = tmp->pair->b->mob->Y_from + (dy * (tmp->pair->b->mob->speed * (cur_time - tmp->pair->b->mob->start_journey)));
          }
          else{
            Y_now = tmp->pair->b->mob->Y_from - (dy * (tmp->pair->b->mob->speed * (cur_time - tmp->pair->b->mob->start_journey)));
          }

          LOG_D(OMG, "X_now %f\tY_now %f\n", X_now, Y_now);
          tmp->pair->b->X_pos = (double) ((int) (X_now*100))/ 100;
          tmp->pair->b->Y_pos = (double) ((int) (Y_now*100))/ 100;
          //tmp->pair->b->mob->X_from = tmp->pair->b->X_pos;
          //tmp->pair->b->mob->Y_from = tmp->pair->b->Y_pos;
	  //tmp->pair->b->mob->start_journey = cur_time;
          LOG_I(OMG, "Updated_position of node number %d is :(%.2f, %.2f)\n", tmp->pair->b->ID, tmp->pair->b->X_pos, tmp->pair->b->Y_pos);
        }
        else{
          LOG_E(OMG, "Update_generator: unsupported node state - mobile : %d \n", tmp->pair->b->mobile);
          return;
	}
      }
      tmp = tmp->next;
    }
 }
 else {
   LOG_E(OMG, "ERROR (current time) %f > %f (first job_time) \n", cur_time ,my_pair->a );
 }
}
