#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "rwalk.h"#include "log.h"
#include "global_parameters.h"
#include "common.h"
#include "OMG.h"
#include "job.h"
#include "rwalk.h" 

const double min_azimuth = 0.0;
const double max_azimuth = 360.0;

 
Pair move_rwalk_node(NodePtr node, m_global_param param_list,double cur_time) {
	Pair pair = malloc(sizeof(Pair));
	LOG_D("\n\nMOVE RWALK NODE\n ID: %d",node->ID);
	
	node->mob->X_from = node->X_pos;
	node->mob->Y_from = node->Y_pos;

	double speed_next = randomGen(param_list.min_speed, param_list.max_speed);
	node->mob->speed = speed_next;
    	LOG_D("\nspeed_next: %f", speed_next); //m/s

	double azimuth_next = randomGen(min_azimuth, max_azimuth);
	node->mob->azimuth = azimuth_next;
    	LOG_D("\nazimuth_next: %f", node->mob->azimuth); 

	double journeyTime_next = randomGen(param_list.min_journey_time, param_list.max_journey_time);
	node->mob->journey_time = journeyTime_next;
    	LOG_D("\njourney_time_next: %f", node->mob->journey_time); 

	double distance = node->mob->speed * node->mob->journey_time;
	LOG_D("\ndistance = speed * journey_time: %f", distance); 

	double dX = distance * cos(node->mob->azimuth*M_PI/180);
	LOG_D("\ndX = distance * cos(azimuth): %f", dX); 
	double dY = distance * sin(node->mob->azimuth*M_PI/180);
	LOG_D("\ndY = distance * sin(azimuth): %f", dY); 
	
	LOG_D("\nfrom: (%.3f, %.3f)", node->X_pos, node->Y_pos);
	double X_next = (double) ((int)((node->X_pos + dX) *1000))/ 1000;
	double Y_next = (double) ((int)((node->X_pos + dY) *1000))/ 1000;
	LOG_D("\ntheoritical_destination: (%f, %f)", X_next, Y_next);

	/*if (X_next<param_list.min_X){ 
		node->mob->X_to = param_list.min_X;  
	}
	else if (X_next>param_list.max_X){
		node->mob->X_to =  param_list.max_X;
	}
	else {
		node->mob->X_to = X_next;
	}

	if (Y_next<param_list.min_Y){ 
		node->mob->Y_to = param_list.min_Y;  
	}
	else if (Y_next>param_list.max_Y){
		node->mob->Y_to =  param_list.max_Y;
	}
	else {
		node->mob->Y_to = Y_next;
	}*/
	if (X_next < param_list.min_X){ 
		node->mob->X_to = X_next + param_list.max_X;  
	}
	else if (X_next > param_list.max_X){
		node->mob->X_to =  X_next - param_list.max_X;
	}
	else {
		node->mob->X_to = X_next;
	}

	if (Y_next < param_list.min_Y){ 
		node->mob->Y_to = Y_next + param_list.max_Y;  
	}
	else if (Y_next > param_list.max_Y){
		node->mob->Y_to =  Y_next - param_list.max_Y;
	}
	else {
		node->mob->Y_to = Y_next;
	}
	
	LOG_D("\ndestination: (%.3f, %.3f)", node->mob->X_to, node->mob->Y_to);

	node->mob->start_journey = cur_time;
   	LOG_D("\nstart_journey %.3f", node->mob->start_journey );
	
	pair->a = (double) ((int) ( (node->mob->start_journey + journeyTime_next) *1000))/ 1000 ;
	LOG_D("\npair->a= start journey + journeyTime_next next %lf\n", pair->a);

	pair->b = node;
	return pair;
	
}



void update_rwalk_nodes(double cur_time) {// need to implement an out-of-area check as well as a rebound function to stay in the area

	LOG_D("\n\n**********UPDATE**********");
	int l = 0;
	double X_now=0.0;
	double Y_now=0.0;
	
	while (l < Job_Vector_len){
		LOG_D("\n		l == %d \n", l); 
		//LOG_D("\n		length == %d \n", Job_Vector_len); 
		Pair my_pair = Job_Vector->pair;
		if((my_pair !=NULL) && ( (double)my_pair->a >= cur_time - eps) && ( (double)my_pair->a <= cur_time + eps)) { 
			LOG_D("\n %.3f == %.3f ",my_pair->a, cur_time );
 	
			NodePtr my_node= (NodePtr)my_pair->b;
			if(my_node->mobile == 1) {
 				LOG_D("\n stop node and let it sleep " );
				// stop node and let it sleep
				my_node->mobile = 0;
				Pair pair = malloc(sizeof(Pair));
				pair = sleep_node(my_node, param_list, cur_time);
				Job_Vector->pair = pair;
			}
			else if (my_node->mobile ==0) {
				LOG_D("\n node slept enough...let's move again " );
				// node slept enough...let's move again
				my_node->mobile = 1;
				Pair pair = malloc(sizeof(Pair));
				pair = move_node(my_node, param_list, cur_time);
				Job_Vector->pair = pair;
			}
			else
				{LOG_E("update_generator: unsupported node state - mobile : %d \n", my_node->mobile);}
			//sorting the new entries
			LOG_D("\n  **********DISPLAY NODE LIST**********"); 
			display_node_list(Node_Vector,2);
			LOG_D("\n\n **********DISPLAY JOB LIST********** "); 
			display_job_list(Job_Vector);
			Job_Vector = quick_sort (Job_Vector);
			LOG_D("\n\n **********DISPLAY JOB LIST AFTER SORTING**********"); 
			display_job_list(Job_Vector);
			l++;
			
		}
		else if ( (my_pair !=NULL) && (cur_time < my_pair->a ) ){
		    LOG_D("\n\n %.3f < %.3f ",cur_time, my_pair->a);
		    l = Job_Vector_len;
		    Job_list tmp = Job_Vector;
		    while (tmp != NULL){
			if (tmp->pair->b->mobile == 0){ //node is sleeping
			    LOG_D("\nnode number %d is sleeping at location: (%.3f, %.3f)", tmp->pair->b->ID, tmp->pair->b->X_pos, tmp->pair->b->Y_pos);
			    LOG_D("\nnothing to do");
			} 
else if (tmp->pair->b->mobile == 1){ //node is moving
			    LOG_D("\n\nNode_number %d", tmp->pair->b->ID);
			    LOG_D("\ndestination not yet reached\nX_from %.3f\nY_from %.3f\nX_to %.3f\nY_to %.3f\nspeed %.3f\nX_pos %.3f\nY_pos %.3f", tmp->pair->b->mob->X_from, tmp->pair->b->mob->Y_from, tmp->pair->b->mob->X_to, tmp->pair->b->mob->Y_to,tmp->pair->b->mob->speed, tmp->pair->b->X_pos, tmp->pair->b->Y_pos);
			    double len = sqrtf(pow(tmp->pair->b->mob->X_from - tmp->pair->b->mob->X_to,2)+pow(tmp->pair->b->mob->Y_from - tmp->pair->b->mob->Y_to,2));
			    double dx = fabs(tmp->pair->b->mob->X_from - tmp->pair->b->mob->X_to) / len;
				  
			    double dy = fabs(tmp->pair->b->mob->Y_from - tmp->pair->b->mob->Y_to) / len;
			    LOG_D("\nlen %f\ndx %f\ndy %f", len, dx, dy);
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

			    LOG_D("\nX_now %f\nY_now %f", X_now, Y_now);
			    if (X_now < param_list.min_X){ 
				X_now = X_now + param_list.max_X;  
				}
				else if (X_now > param_list.max_X){
				X_now =  X_now - param_list.max_X;
				}
				else {
				X_now = X_now;
				}

			    if (Y_now < param_list.min_Y){ 
				Y_now = Y_now + param_list.max_Y;  
				}
			    else if (Y_now > param_list.max_Y){
				Y_now =  Y_now - param_list.max_Y;
				}
			    else {
				Y_now = Y_now;
				}
			    tmp->pair->b->X_pos = (double) ((int) (X_now*1000))/ 1000;
			    tmp->pair->b->Y_pos = (double) ((int) (Y_now*1000))/ 1000;
 				   //tmp->pair->b->mob->X_from = tmp->pair->b->X_pos;
				   //tmp->pair->b->mob->Y_from = tmp->pair->b->Y_pos;
				   //tmp->pair->b->mob->start_journey = cur_time;
			    LOG_D("\nupdated_position of node number %d is :(%.3f, %.3f)", tmp->pair->b->ID, tmp->pair->b->X_pos, tmp->pair->b->Y_pos);
			    //LOG_D("\nupdated position of node %d is : (%.3f, %.3f)\n", tmp->pair->b->ID ,tmp->pair->b->X_pos, tmp->pair->b->Y_pos);
			  }
			else { //node is moving
			    LOG_D("\nnode is moving");
			}
		
		tmp = tmp->next;
		    }
		}
		else {
			LOG_D("\nERROR current time > first job_time" );
			return;
		}	
}	
}
