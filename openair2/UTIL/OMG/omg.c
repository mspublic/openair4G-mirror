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
* \brief Main function containing theOMG interface
* \author J. Harri, M. Marhashi, and N. Nikaein
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

//#define STANDALONE

void init_omg_global_params(void){

  Job_Vector_Rwp_len=0;
  Job_Vector_Rwalk_len=0;

  Node_Vector_Static_eNB = NULL;
  Node_Vector_Static_UE = NULL;
  Node_Vector_Rwalk = NULL;
  Node_Vector_Rwp= NULL ;

  Job_Vector_Rwp = NULL ;
  Job_Vector_Rwalk = NULL ;
  
}

void init_mobility_generator(omg_global_param omg_param_list) {
  switch (omg_param_list.mobility_type) {
  case RWP: 
    start_rwp_generator(omg_param_list);
    break;
  
  case STATIC: 
    start_static_generator(omg_param_list);
    break;
  
  case RWALK: 
    start_rwalk_generator(omg_param_list);
    break;
  
  case UNDEF:
  default:
    printf("Unsupported generator %c \n", omg_param_list.mobility_type);
  }
}

void update_nodes(int mobility_type, double cur_time){
  set_time(cur_time);
  switch (mobility_type) {
  case RWP:
    update_rwp_nodes(cur_time);
    /*LOG_D("\n  **********DISPLAY NODE LIST**********"); 
      display_node_list(Node_Vector,2);
      LOG_D("\n\n **********DISPLAY JOB LIST********** "); 
      display_job_list(Job_Vector);*/
    break;
    
    
  case RWALK:
    update_rwalk_nodes(cur_time);
    /*LOG_D("\n  **********DISPLAY NODE LIST**********"); 
      display_node_list(Node_Vector,2);
      LOG_D("\n\n **********DISPLAY JOB LIST********** "); 
      display_job_list(Job_Vector);*/
    break;
    
  case UNDEF:
  default:
    LOG_N(OMG, "Static or Unsupported generator %c \n", omg_param_list.mobility_type);
  }
}

Node_list get_current_positions(int mobility_type, int nodes_type, double cur_time){
  Node_list Node_Vector = NULL;
  switch (mobility_type) {
  case RWP:
    get_rwp_positions_updated(cur_time);
    //LOG_D("\n  **********DISPLAY NODE LIST**********"); 
    //display_node_list(Node_Vector,2);
    //LOG_D("\n\n **********DISPLAY JOB LIST********** "); 
    //display_job_list(Job_Vector);
    Node_Vector = Node_Vector_Rwp;
    break;
    
  case STATIC:
    Node_Vector = ((nodes_type == 0) ? Node_Vector_Static_eNB : Node_Vector_Static_UE);
    break;
  case RWALK:
    get_rwalk_positions_updated(cur_time);
    //	LOG_D("\n  **********DISPLAY NODE LIST**********"); 
    //	display_node_list(Node_Vector,2);
    //	LOG_D("\n\n **********DISPLAY JOB LIST********** "); 
    //	display_job_list(Job_Vector);
    Node_Vector = Node_Vector_Rwalk;
    break;
    
  case UNDEF:
  default:
    Node_Vector = NULL;
    LOG_N(OMG, "\n Static or Unsupported generator %c \n", omg_param_list.mobility_type);
  }
  return Node_Vector;
}

// get the position for a specific node 
NodePtr get_node_position(int mobility_type, int nodes_type, int nID){
  Node_list Node_Vector = NULL;
  switch (mobility_type) {
  case RWP:
    Node_Vector = Node_Vector_Rwp;
    break;
  case STATIC:
    Node_Vector = ((nodes_type == 0) ? Node_Vector_Static_eNB : Node_Vector_Static_UE);
    break;
  case RWALK:
    Node_Vector = Node_Vector_Rwalk;
    break;
  case UNDEF:
  default:
    Node_Vector = NULL;
    LOG_N(OMG, "\n Static or Unsupported generator %c \n", omg_param_list.mobility_type);
  }
  if (Node_Vector == NULL){
    LOG_D(OMG, "\n Node_Vector == NULL"); 
  }
  while (Node_Vector != NULL){
    if (Node_Vector->node->ID == nID){
      return Node_Vector->node;
    }
    Node_Vector = Node_Vector->next;
  }
  
}

// openair emu will set this valut as a function of frame number
void set_time(double time) {
	m_time = time;
}

double get_time() {
	return m_time;
}

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
	while ((tag = getopt(argc, argv, "j:J:g:B:b:S:s:Y:y:X:x:N:h:e:")) != EOF) {


		switch (tag) {

		case 'N':
			omg_param_list.nodes = atoi(optarg);
			printf("Number of nodes : %d \n",omg_param_list.nodes);
			break;

		case 'b':
			omg_param_list.min_sleep = atof(optarg);
			printf("min sleep : %.3f \n",omg_param_list.min_sleep);
			break;

		case 'B':
			omg_param_list.max_sleep = atof(optarg);
			printf("max_sleep : %.3f \n",omg_param_list.max_sleep);
			break;

		case 's':
			omg_param_list.min_speed = atof(optarg);
			printf("min_speed : %.3f \n",omg_param_list.min_speed);
			break;

		case 'S':
			omg_param_list.max_speed = atof(optarg);
			printf("max_speed : %.3f \n",omg_param_list.max_speed);
			break;

		case 'X':
			omg_param_list.max_X = atof(optarg);
			printf("X_max : %.3f \n",omg_param_list.max_X);
			break;
		case 'x':
			omg_param_list.min_X = atof(optarg);
			printf("X_min : %.3f \n",omg_param_list.min_X);
			break;

		case 'Y':
			omg_param_list.max_Y = atof(optarg);
			printf("Y_max : %.3f \n",omg_param_list.max_Y);
			break;

		case 'y':
			omg_param_list.min_Y = atof(optarg);
			printf("Y_min : %.3f \n",omg_param_list.min_Y);
			break;

		case 'J':
			omg_param_list.max_journey_time = atof(optarg);
			printf("Journey_time_max : %.3f \n",omg_param_list.max_journey_time);
			break;


		case 'j':
			omg_param_list.min_journey_time = atof(optarg);
			printf("Journey_time_min : %.3f \n",omg_param_list.min_journey_time);
			break;

		
		case 'g':
			omg_param_list.mobility_type = atoi(optarg);
			printf("Mobility type is %d \n",omg_param_list.mobility_type);
			break;

		case 'e':
			omg_param_list.seed = atoi(optarg);
			printf("Seed is %d \n",omg_param_list.seed );
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
 	int my_ID = 1;

	get_options(argc, argv);
	if(omg_param_list.max_X == 0.0 || omg_param_list.max_Y == 0.0 || omg_param_list.nodes == 0 ) {
		usage();
		exit(1);
	}

	set_time(1.0);
	LOG_D("\ncurrent time =  %.3f", get_time()); 
	cur_time = get_time();

    	initialize_nodes(cur_time); // initial positions + sleep 
	if (omg_param_list.mobility_type != STATIC){
   		for (ms = 2.0; ms<=10.0; ms++){
		  	LOG_D("\n ms  %.3f ", ms);
			cur_time = ms;
			LOG_D("\ncurrent time =  %.3f", ms); 
			//	cur_time = get_time();
			update_nodes(cur_time);
	    	}
	}
	else {
		LOG_D("\n\n Nodes are static "); 
	    }
     	
	Current_positions = get_current_positions(cur_time);
	LOG_D("\n********DISPLAY Current_positions LIST********"); 
	display_node_list(Current_positions,1);
	if (Current_positions == NULL){LOG_D("\n  Current_positions == NULL");  }

	my_node = (NodePtr) get_node_position( my_ID);
	LOG_D("\n********At %.3f, Node number %d is in position: (%.3f, %.3f) with mobility_type: %d",cur_time,my_ID,my_node->X_pos,my_node->Y_pos,my_node->mobile); 
	
	return 0;
}

#endif 




//firt method
/*void update_nodes(double cur_time) {
	LOG_D("\n\n**********UPDATE**********");
	//Job_list tmp = Job_Vector;
	int l = 0;
	
//	if (omg_param_list.mobility_type = STATIC) {
//		LOG_D("\n\n Nodes are static "); 
	//}fabs(val1-val2)/(fabs(val1)+fabs(val2))<eps
	 
	 //if( (my_pair !=NULL) && ( fabs(my_pair->a  - cur_time)/(fabs(my_pair->a)+fabs(cur_time))<eps  ) ){ 
	//if (Equality(my_pair->a , cur_time, eps)) {
	//if (nearlyEqual(my_pair->a, cur_time, eps)){
	//if (fabs(my_pair->a - cur_time)<eps * max(my_pair, cur_time)){
	while (l < Job_Vector_len){
		LOG_D("\n		l == %d \n", l); 
		//LOG_D("\n		length == %d \n", Job_Vector_len); 
		Pair my_pair = Job_Vector->pair;
		if((my_pair !=NULL) && ( (double)my_pair->a >= cur_time - eps) && ( (double)my_pair->a <= cur_time + eps)) { 
			LOG_D("\n %.3f == %.3f ",my_pair->a, cur_time );
 			//LOG_D("\n OKiiiiiiiiiiiiiiiiiiiiiii " );
			NodePtr my_node= (NodePtr)my_pair->b;
			if(my_node->mobile == 1) {
 				LOG_D("\n stop node and let it sleep " );
				// stop node and let it sleep
				my_node->mobile = 0;
				Pair pair = malloc(sizeof(Pair));
				pair = sleep_node(my_node, omg_param_list, Job_Vector, cur_time);
				Job_Vector->pair = pair;
			}
			else if (my_node->mobile ==0) {
				LOG_D("\n node slept enough...let's move again " );
				// node slept enough...let's move again
				my_node->mobile = 1;
				Pair pair = malloc(sizeof(Pair));
				pair = move_node(my_node, omg_param_list, Job_Vector, cur_time);
				Job_Vector->pair = pair;
			}
			else
				{LOG_E("update_generator: unsupported node state - mobile : %d \n", my_node->mobile);}
		//sorting the new entries
		LOG_D("\n  **********DISPLAY NODE LIST**********"); 
		display_node_list(Node_Vector);
		LOG_D("\n\n **********DISPLAY JOB LIST********** "); 
		display_job_list(Job_Vector);
		Job_Vector = quick_sort (Job_Vector);
		LOG_D("\n\n **********DISPLAY JOB LIST AFTER SORTING**********"); 
		display_job_list(Job_Vector);
			
		}
		else{
			LOG_D("\n\n %.3f != %.3f ",my_pair->a, cur_time );
			LOG_D("\n Nothing to do \n"); 
			//return;
		}
		l++;	
}
	
}*/





