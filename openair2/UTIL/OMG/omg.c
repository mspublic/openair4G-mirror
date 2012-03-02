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
* \author  M. Mahersi, N. Nikaein, J. Harri
* \date 2011
* \version 0.1
* \company Eurecom
* \email: 
* \note
* \warning
*/


#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include <time.h>
#include "omg.h"
#include "static.h"
#include "rwp.h"
#include "rwalk.h"
#include "trace.h"

//#define STANDALONE

void init_omg_global_params(void){ //if we want to re initialize all
  int i = 0;
  Job_Vector_len=0;
  Job_Vector = NULL;
  for (i=0; i<MAX_NUM_MOB_TYPES; i++){
    Node_Vector[i] = NULL;
	 Node_Vector_len[i] = 0;
	 //Initial_Node_Vector_len[i] = 0;
  }
}

void init_mobility_generator(omg_global_param omg_param_list) {
  switch (omg_param_list.mobility_type) {

  case STATIC: 
    start_static_generator(omg_param_list);
    break;
  
  case RWP: 
    start_rwp_generator(omg_param_list);
    LOG_D(OMG," --------DISPLAY JOB LIST-------- \n"); 
    display_job_list(Job_Vector);
    Job_Vector = quick_sort (Job_Vector);
    LOG_D(OMG,"--------DISPLAY JOB LIST AFTER SORTING--------\n"); 
    display_job_list(Job_Vector);
    break;

  case TRACE:
      start_trace_generator(omg_param_list);
      LOG_D(OMG," --------DISPLAY JOB LIST-------- \n");
      display_job_list(Job_Vector);
      Job_Vector = quick_sort (Job_Vector);
      LOG_D(OMG,"--------DISPLAY JOB LIST AFTER SORTING--------\n");
      display_job_list(Job_Vector);
      break;    
    
  case RWALK: 
    start_rwalk_generator(omg_param_list);
    LOG_D(OMG," --------DISPLAY JOB LIST-------- \n"); 
    display_job_list(Job_Vector);
    Job_Vector = quick_sort (Job_Vector);
    LOG_D(OMG,"--------DISPLAY JOB LIST AFTER SORTING--------\n"); 
    display_job_list(Job_Vector);
    
    break;
  
  default:
    LOG_N(OMG, "Unsupported generator %d \n", omg_param_list.mobility_type);
  }
}

void update_nodes(double cur_time){
LOG_D(OMG, "UPDATE NODES" );
int i = 0;
for (i=(STATIC+1); i<MAX_NUM_MOB_TYPES; i++){ 
      if (Node_Vector[i] != NULL){
        LOG_D(OMG, " Mob model to update is: %d \n ", i); 
        update_node_vector(i, cur_time);
      }
    }
  
}


void update_node_vector(int mobility_type, double cur_time){
  //set_time(cur_time);
  switch (mobility_type) {
  case RWP:
    update_rwp_nodes(cur_time);
    break;
  case TRACE:
      update_trace_nodes(cur_time);
      break;     
  case RWALK:
    update_rwalk_nodes(cur_time);
    break;
    
  default:
    LOG_N(OMG, "Static or Unsupported generator %d \n", omg_param_list.mobility_type);
  }
}

Node_list get_current_positions(int mobility_type, int node_type, double cur_time){
  Node_list Vector = NULL;
  if (Node_Vector[mobility_type] != NULL){
    switch (mobility_type) {
    case RWP:
      get_rwp_positions_updated(cur_time);
      Vector = Node_Vector[RWP];
      break;
    case TRACE:
      get_trace_positions_updated(cur_time);
      Vector = Node_Vector[TRACE];
      break;   
    case STATIC:
      LOG_D(OMG,"get_static_positions\n");
      Vector = (Node_list)Node_Vector[STATIC];
    break;
    case RWALK:
      get_rwalk_positions_updated(cur_time);
      Vector = Node_Vector[RWALK];
    break;
    
    default:
      Vector = NULL;
      LOG_N(OMG, " Unsupported generator %c \n", omg_param_list.mobility_type);
    }
    if (node_type != ALL){
      Vector = filter(Vector, node_type);
    }
    return Vector;
  }
  else {
  LOG_T( OMG, " No node of mobility type %d is available  \n",  mobility_type);
  return NULL;
  }
}


// get the position for a specific node 
NodePtr get_node_position(int node_type, int nID){
 int found = 0;  //not found
 int i=0;
 while ((i<MAX_NUM_MOB_TYPES) && (found == 0)){ 
     	if (Node_Vector[i] != NULL){
        Node_list tmp = Node_Vector[i];
        while ((tmp != NULL) && (found == 0)){
  	     	   if ((tmp->node->ID == nID) && (tmp->node->type == node_type)) {
             LOG_T(OMG, "found a node vector %d node id %d with type %d\n",i, tmp->node->ID, tmp->node->type);	
			    found = 1;   //found
				 display_node_position(tmp->node->ID, tmp->node->generator, tmp->node->type ,tmp->node->mobile, tmp->node->X_pos, tmp->node->Y_pos );
				 return tmp->node;
          }
			  tmp = tmp->next;
       }
     }
     i++;
}
if (found == 0 ){LOG_N(OMG, "Node does not exist\n");}
    return NULL;
}

/*NodePtr get_node_position(int mobility_type, int node_type, int nID){
  if (Node_Vector[mobility_type] != NULL){
    Node_list Vector = NULL;
    int done =1;
    switch (mobility_type) {
    case RWP:
      Vector = Node_Vector[RWP];
      break;
    case STATIC:
      Vector = Node_Vector[STATIC];
      break;
    case RWALK:
      Vector = Node_Vector[RWALK];
      break;
    default:
      Vector = NULL;
      LOG_E(OMG, "Static or Unsupported generator %c \n", omg_param_list.mobility_type);
    }
    if (Vector == NULL){
      LOG_D(OMG, "\n Vector == NULL"); 
    }
    while (Vector != NULL){
      if (Vector->node->ID == nID && Vector->node->type == node_type){
        done =0;
		 
		  display_node_position(Vector->node->ID, Vector->node->generator, Vector->node->type ,Vector->node->mobile, Vector->node->X_pos, Vector->node->Y_pos );
  	 //   LOG_I(OMG, "Node number %d is in position: (%.2f, %.2f) with mobility_type: %d\n",Vector->node->ID,Vector->node->X_pos,Vector->node->Y_pos,Vector->node->mobile);
        return Vector->node;
      }
      Vector = Vector->next;
    }
    if (done!= 0 ){LOG_N(OMG, "Node does not exist\n");}
    return NULL;
  }
  else {
    LOG_N( OMG, "No node of mobility type %d is available  \n",  mobility_type);
  return NULL;
  }
}*/



void set_new_mob_type(int nID, int node_type, int new_mob, double cur_time){
  double first_Job_time;
  int err = 999;
  int i = STATIC;
  int old_mob = err;
  int found = 0;  //not found
  while ((i<MAX_NUM_MOB_TYPES) && (found == 0)){ 
      if (Node_Vector[i] != NULL){
        Node_list tmp = Node_Vector[i];
        while ((tmp != NULL) && (found == 0)){
  	       if ((tmp->node->ID == nID) && (tmp->node->type == node_type)) {
             found = 1;   //found
  				 old_mob = i;
      		 LOG_D( OMG,"old mob %d\n", old_mob );  //LOG_N
				 if (old_mob == new_mob){
   				 LOG_D(OMG, "Nothing to change (%d == %d)\n",old_mob, new_mob);
   			    return;
  				 }
				 else {
               LOG_D(OMG,"Node_Vector[%d] != NULL\n", old_mob);
   				//int done = 1;
               //Node_list Vector = Node_Vector[old_mob];
               //NodePtr nd = NULL;
					NodePtr nd =  tmp->node;
		 			if (new_mob == STATIC) { //OKEY            // remove from Job_Vector
          			LOG_D(OMG, "new_mob == STATIC\n");
	       			nd->mobile = 0;
	       			nd->mob->speed = 0.0;

	       			nd->mob->X_from = nd->X_pos;
	       			nd->mob->Y_from = nd->Y_pos;
			 			nd->mob->X_to = nd->X_pos;
	       			nd->mob->Y_to = nd->Y_pos;
	       			//nd->X_pos = nd->mob->X_to;
	       			//nd->Y_pos = nd->mob->Y_to;
		   			 LOG_D(OMG, "Before remove");
		   			 display_job_list( Job_Vector);

			 			if (Job_Vector != NULL){
		      		  Job_Vector = remove_job(Job_Vector, nID, node_type);  //nd->ID
		    			  //LOG_D(OMG, "After remove\n");
	        			  Job_Vector_len--;
            		  LOG_D(OMG, "After remove\n");
		      		  display_job_list( Job_Vector);
          		  }
			 		  else {
			 		    LOG_E(OMG, "ERROR, Job_Vector == NULL while there are mobile nodes");
          		  }		
		    
	           }
	    
	    		Node_Vector[old_mob] = remove_node(Node_Vector[old_mob], nID, node_type);
		 		Node_Vector_len[old_mob]--;
	    		nd->ID = nID; //Node_Vector_len[new_mob]; 
				LOG_D(OMG, "Node_Vector_len[new_mob]  %d\n", Node_Vector_len[new_mob]);
				LOG_D(OMG, "nd->ID  %d\n", nd->ID);
      		nd->generator = new_mob;    // ---> it is changed in Job_Vector if new_mob is RWP or RWALK

	    		Node_Vector[new_mob] = add_entry(nd, Node_Vector[new_mob]);
		 		Node_Vector_len[new_mob]++;

				if (old_mob == STATIC){		//OKEY  	  // add to Job_Vector , start by sleeping
		  			LOG_D(OMG, "old_mob == STATIC\n");
		 
         		if(Job_Vector == NULL){
			 		  LOG_D(OMG, "Job_Vector == NULL\n");
			  		  first_Job_time = cur_time; 
		  			}
              else {
					  LOG_D(OMG, "Job_Vector != NULL\n");
			        first_Job_time = Job_Vector->pair->a;
		        }

        		  LOG_D(OMG, "-------------------------------------------------------------	first_Job_time %f\n", first_Job_time);
 		        Pair pair = malloc(sizeof(Pair)) ;

		        nd->mob->sleep_duration = (double) ((int) (randomGen(omg_param_list.min_sleep, omg_param_list.max_sleep)*100))/ 100;
		        LOG_D(OMG, "node: %d \tsleep duration : %.2f\n",nd->ID, nd->mob->sleep_duration);
	 	        pair->a = first_Job_time + 1 + nd->mob->sleep_duration; //when to wake up    ????
	           LOG_D(OMG, "to wake up at time: cur_time + sleep_duration : %.2f\n", pair->a);

		        pair->b = nd;
              Job_Vector = add_job(pair, Job_Vector);
			     Job_Vector_len++;
		     }
		     LOG_D(OMG," --------DISPLAY JOB LIST-------- \n"); 
			  display_job_list(Job_Vector);
			  Job_Vector = quick_sort (Job_Vector);
			  LOG_D(OMG,"--------DISPLAY JOB LIST AFTER SORTING--------\n"); 
		 	  display_job_list(Job_Vector);

				
   		  LOG_D(OMG, "--------display Node_Vector[new_mob]--------\n");
    	     display_node_list(Node_Vector[new_mob]);
    	     LOG_D(OMG, "--------display Node_Vector[old_mob]--------\n");
    	     display_node_list(Node_Vector[old_mob]);
           return;
          }
			 }
        tmp = tmp->next;
       } 
     }
   i++;
  }
}

 /*if (old_mob == err){
      LOG_N( OMG,"Node (ID= %d,type= %d) does not exist\n", nID, node_type );  //LOG_N
 }
 else{
  if (old_mob == new_mob){
    LOG_N(OMG, "Nothing to change (%d == %d)\n",old_mob, new_mob);
    return;
  }
  //LOG_D(OMG,"old_mob %d\n",old_mob);
  //if (Node_Vector[old_mob] != NULL){
  
    LOG_D(OMG,"Node_Vector[%d] != NULL\n", old_mob);
    int done = 1;
    Node_list Vector = Node_Vector[old_mob];
    NodePtr nd = NULL;
    while (Vector != NULL){
      if ((Vector->node->ID == nID) && (Vector->node->type == node_type)){
        //LOG_D(OMG, "Vector->node->ID == nID && Vector->node->type == node_type\n");
       done =0;  //found
		 nd =  Vector->node;
		 if (new_mob == STATIC) { //OKEY            // remove from Job_Vector
          LOG_D(OMG, "new_mob == STATIC\n");
	       nd->mobile = 0;
	       nd->mob->speed = 0.0;
	       nd->mob->X_from = nd->X_pos;
	       nd->mob->Y_from = nd->Y_pos;
			 nd->mob->X_to = nd->X_pos;
	       nd->mob->Y_to = nd->Y_pos;
	       //nd->X_pos = nd->mob->X_to;
	       //nd->Y_pos = nd->mob->Y_to;
		    LOG_D(OMG, "Before remove");
		    display_job_list( Job_Vector);

			 if (Job_Vector != NULL){
		      Job_Vector = remove_job(Job_Vector, nID, node_type);  //nd->ID
		    //LOG_D(OMG, "After remove\n");
	         Job_Vector_len--;
            LOG_D(OMG, "After remove\n");
		      display_job_list( Job_Vector);
          }
			 else {
			  LOG_E(OMG, "ERROR, Job_Vector == NULL while there are mobile nodes");
          }
		    
	    }
	    
	    Node_Vector[old_mob] = remove_node(Node_Vector[old_mob], nID, node_type);
		 Node_Vector_len[old_mob]--;
	    nd->ID = nID; //Node_Vector_len[new_mob]; 
		 LOG_D(OMG, "Node_Vector_len[new_mob]  %d\n", Node_Vector_len[new_mob]);
		 LOG_D(OMG, "nd->ID  %d\n", nd->ID);
       nd->generator = new_mob;    // ---> it is changed in Job_Vector if new_mob is RWP or RWALK

	    Node_Vector[new_mob] = add_entry(nd, Node_Vector[new_mob]);
		 Node_Vector_len[new_mob]++;

       

		if (old_mob == STATIC){		//OKEY  	  // add to Job_Vector , start by sleeping
		  LOG_D(OMG, "old_mob == STATIC\n");
		 
         if(Job_Vector == NULL){
			  LOG_D(OMG, "Job_Vector == NULL\n");
			  first_Job_time = cur_time; 
		  }
          else {
			LOG_D(OMG, "Job_Vector != NULL\n");
			 first_Job_time = Job_Vector->pair->a;
		  }

        LOG_D(OMG, "-------------------------------------------------------------	first_Job_time %f\n", first_Job_time);
 		  Pair pair = malloc(sizeof(Pair)) ;

		  nd->mob->sleep_duration = (double) ((int) (randomGen(omg_param_list.min_sleep, omg_param_list.max_sleep)*100))/ 100;
		  LOG_D(OMG, "node: %d \tsleep duration : %.2f\n",nd->ID, nd->mob->sleep_duration);
	 	  pair->a = first_Job_time + 1 + nd->mob->sleep_duration; //when to wake up    ????
	      LOG_D(OMG, "to wake up at time: cur_time + sleep_duration : %.2f\n", pair->a);

		  pair->b = nd;
          Job_Vector = add_job(pair, Job_Vector);
			 Job_Vector_len++;
		}
		LOG_D(OMG," --------DISPLAY JOB LIST-------- \n"); 
		display_job_list(Job_Vector);
		Job_Vector = quick_sort (Job_Vector);
		LOG_D(OMG,"--------DISPLAY JOB LIST AFTER SORTING--------\n"); 
		display_job_list(Job_Vector);

				
   	LOG_I(OMG, "--------display Node_Vector[new_mob]--------\n");
    	display_node_list(Node_Vector[new_mob]);
    	LOG_I(OMG, "--------display Node_Vector[old_mob]--------\n");
    	display_node_list(Node_Vector[old_mob]);

	  return;
      }
      Vector = Vector->next;
    }
    if (done != 0 ){
      LOG_E( OMG,"Node (ID= %d,type= %d, generator= %d) does not exist\n", nID, node_type, old_mob );  //LOG_N
    }
  
  //else {
   // LOG_E( OMG,"No node of mobility model %d is available  \n",  old_mob); //LOG_N
 // }
}
}
}*/

/*// openair emu will set this valut as a function of frame number
void set_time(double time) {
	m_time = time;
}

double get_time() {
	return m_time;
}*/

#ifdef STANDALONE
void usage(void){
	fprintf(stderr, "\n\t-X: assign maximum width of the simulation area (X_max)"\
		"\n\t-x: assign minimum width of the simulation area (X_min)"\
		"\n\t-Y: assign maximum height of the simulation area (Y_max)"\
		"\n\t-y: assign minimum height of the simulation area (Y_min)"\
		"\n\t-N: assign number of nodes"\
		"\n\t-B: assign maximum duration of sleep/pause time (max_break)"
		"\n\t-b: assign minimum duration of sleep/pause time (min_break)"\
		"\n\t-J: assign maximum duration of journey (max_journey_time)"\
		"\n\t-j: assign minimum duration of journey (min_journey_time)"\
		"\n\t-S: assign maximum speed "\
		"\n\t-s: assign minimum speed"\
		"\n\t-g: choose generator (SATIC: 0x00, RWP: 0x01 or RWALK 0x02)\n"\
		"\n\t-e: choose seed \n"\
	);	
	exit(0);
}
int get_options(int argc, char *argv[]){
	char tag;
	while ((tag = getopt(argc, argv, "j:J:g:B:b:S:s:Y:y:X:x:N:h:e:t:")) != EOF) {


		switch (tag) {

		case 'N':
			omg_param_list.nodes = atoi(optarg);
			LOG_D(OMG, "Number of nodes : %d \n",omg_param_list.nodes);
			break;

		case 't':
			omg_param_list.nodes_type = atoi(optarg);
			LOG_D(OMG, "Type of nodes : %d \n",omg_param_list.nodes_type);
			break;
		case 'b':
			omg_param_list.min_sleep = atof(optarg);
			LOG_D(OMG, "min sleep : %.2f \n",omg_param_list.min_sleep);
			break;

		case 'B':
			omg_param_list.max_sleep = atof(optarg);
			LOG_D(OMG, "max_sleep : %.2f \n",omg_param_list.max_sleep);
			break;

		case 's':
			omg_param_list.min_speed = atof(optarg);
			LOG_D(OMG, "min_speed : %.2f \n",omg_param_list.min_speed);
			break;

		case 'S':
			omg_param_list.max_speed = atof(optarg);
			LOG_D(OMG, "max_speed : %.2f \n",omg_param_list.max_speed);
			break;

		case 'X':
			omg_param_list.max_X = atof(optarg);
			LOG_D(OMG, "X_max : %.2f \n",omg_param_list.max_X);
			break;
		case 'x':
			omg_param_list.min_X = atof(optarg);
			LOG_D(OMG, "X_min : %.2f \n",omg_param_list.min_X);
			break;

		case 'Y':
			omg_param_list.max_Y = atof(optarg);
			LOG_D(OMG, "Y_max : %.2f \n",omg_param_list.max_Y);
			break;

		case 'y':
			omg_param_list.min_Y = atof(optarg);
			LOG_D(OMG, "Y_min : %.2f \n",omg_param_list.min_Y);
			break;

		case 'J':
			omg_param_list.max_journey_time = atof(optarg);
			LOG_D(OMG, "Journey_time_max : %.2f \n",omg_param_list.max_journey_time);
			break;


		case 'j':
			omg_param_list.min_journey_time = atof(optarg);
			LOG_D(OMG, "Journey_time_min : %.2f \n",omg_param_list.min_journey_time);
			break;

		
		case 'g':
			omg_param_list.mobility_type = atoi(optarg);
			LOG_D(OMG, "Mobility type is %d \n",omg_param_list.mobility_type);
			break;

		case 'e':
			omg_param_list.seed = atoi(optarg);
			LOG_D(OMG, "Seed is %d \n",omg_param_list.seed );
			break;
		case 'h':
			usage();
			break;

		default:
			usage();
			exit(1);
		}
	}
	return 0;
}

int main(int argc, char *argv[]) {
	int i;
	double cur_time=0.0;
	//srand(time(NULL));
	double ms=0.0;
	Node_list Current_positions = NULL;
	NodePtr my_node = NULL;
 	int my_ID = 0;
        double emu_info_time;

	omg_param_list.min_X = 0;
	omg_param_list.max_X = 100;
	omg_param_list.min_Y = 0;
	omg_param_list.max_Y = 100;
	omg_param_list.min_speed = 0.1;
	omg_param_list.max_speed = 20.0;
	omg_param_list.min_journey_time = 0.1;
	omg_param_list.max_journey_time = 10.0;
	omg_param_list.min_azimuth = 0; 
	omg_param_list.max_azimuth = 360; 
	omg_param_list.min_sleep = 0.1;
	omg_param_list.max_sleep = 5.0;
	
	get_options(argc, argv);
	if(omg_param_list.max_X == 0.0 || omg_param_list.max_Y == 0.0  ) {
		usage();
		exit(1);
	}

       // init_omg_global_params(); //initialization de Node_Vector et Job_Vector

        //omg_param_list.mobility_type = STATIC;
	//omg_param_list.nodes_type = eNB;
	//init_mobility_generator(omg_param_list); // initial positions + sleep  /// need to indicate time of initialization
		//LOG_I(OMG, "*****DISPLAY NODE LIST********\n"); 
		//display_node_list(Node_Vector[0]);
		/*LOG_T(OMG, "********DISPLAY JOB LIST********\n"); 
		display_job_list(Job_Vector);
		Job_Vector = quick_sort (Job_Vector);
		LOG_T(OMG, "**********DISPLAY JOB LIST AFTER SORTING**********\n"); 
		display_job_list(Job_Vector);
*/
	// omg_param_list.mobility_type = RWP;  
	// omg_param_list.nodes_type = UE;
	// init_mobility_generator(omg_param_list); // initial positions + sleep  /// need to indicate time of initialization


	//omg_param_list.mobility_type = RWALK;  	
	//omg_param_list.nodes_type = UE;
	//init_mobility_generator(omg_param_list); // initial positions + sleep  /// need to indicate time of initialization


	//omg_param_list.mobility_type = STATIC;
	//omg_param_list.nodes_type = UE;
	//init_mobility_generator(omg_param_list); // initial positions + sleep  /// need to indicate time of initialization
  	
		//LOG_I(OMG, "*****DISPLAY NODE LIST********\n"); 
		//display_node_list(Node_Vector[0]);
		/*LOG_T(OMG, "********DISPLAY JOB LIST********\n"); 
		display_job_list(Job_Vector);
		Job_Vector = quick_sort (Job_Vector);
		LOG_T(OMG, "**********DISPLAY JOB LIST AFTER SORTING**********\n"); 
		display_job_list(Job_Vector);*/
	 
 	/*init_omg_global_params();        //re-initialization de Node_Vector et Job_Vector
  LOG_I(OMG, "*****DISPLAY NODE LIST********\n"); 
  display_node_list(Node_Vector[0]);
  LOG_T(OMG, "********DISPLAY JOB LIST********\n"); 
  display_job_list(Job_Vector);
  Job_Vector = quick_sort (Job_Vector);
  LOG_T(OMG, "**********DISPLAY JOB LIST AFTER SORTING**********\n"); 
  display_job_list(Job_Vector);*/


	/////////////// to call by OCG
 	
	 // for (emu_info_time = 1.0 ; emu_info_time <= 10.0; emu_info_time+=1.0/100){
				  
	//double emu_info.time += 1.0/100; // emu time in ms
	  // for (i=(STATIC+1); i<MAX_NUM_MOB_TYPES; i++){ //
	  //     if (Node_Vector[i] != NULL){
	//	  LOG_D(OMG, " mob model  %d \n ", i); 
	  //        update_nodes(i ,emu_info_time);
	       }
	      //else {LOG_D( "nodes are STATIC\n"); }
	    }		
	  }
	
	/*LOG_D(OMG, " **********DISPLAY JOB LIST**********\n "); 
        display_job_list(Job_Vector);
	emu_info_time = 10.0;*/

	//Current_positions = get_current_positions(RWP, emu_info_time); // type: enb, ue, all 
	//LOG_D(OMG, " **********Current_positions**********\n "); 
	//display_node_list(Current_positions);
	//if (Current_positions == NULL)
	//   {LOG_D(OMG, " Cur_pos == NULL\n");}
	//my_node = (NodePtr) get_node_position( RWP, UE, my_ID);
	//LOG_D(OMG, "********At %.2f, Node number %d is in position: (%.2f, %.2f) with mobility_type: %d\n",emu_info_time, my_ID,my_node->X_pos,my_node->Y_pos,my_node->mobile);





	return 0;
}

#endif 








