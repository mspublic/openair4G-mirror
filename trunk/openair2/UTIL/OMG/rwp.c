#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "rwp.h"#include "log.h"
#include "global_parameters.h"
#include "common.h"
#include "OMG.h"
#include "job.h"



Pair move_rwp_node(NodePtr node, m_global_param param_list, double cur_time) {
	
	Pair pair = malloc(sizeof(Pair));
	LOG_D("\n\nMOVE RWP NODE");

	LOG_D("\nnode: %d",node->ID );
	node->mob->X_from = node->X_pos;
	node->mob->Y_from = node->Y_pos;
	LOG_D("\nCurrent Position: (%.3f, %.3f)", node->mob->X_from, node->mob->Y_from);


	double X_next = (double) ((int)(randomGen(param_list.min_X,param_list.max_X)*1000))/ 1000;
	node->mob->X_to = X_next;
	double Y_next = (double) ((int)(randomGen(param_list.min_Y,param_list.max_Y)*1000))/ 1000;
	node->mob->Y_to = Y_next;
	LOG_D("\ndestination: (%.3f, %.3f)", node->mob->X_to, node->mob->Y_to);

	double speed_next = (double) ((int)(randomGen(param_list.min_speed, param_list.max_speed)*1000))/ 1000;
	node->mob->speed = speed_next;
    	LOG_D("\nspeed_next %.3f", speed_next); //m/s
	double distance = (double) ((int)(sqrtf(pow(node->mob->X_from - X_next, 2) + pow(node->mob->Y_from - Y_next, 2))*1000))/ 1000;
	LOG_D("\ndistance %f", distance); //m

	double journeyTime_next =  (double) ((int)(distance/speed_next*1000))/ 1000;   //duration to get to dest
	////node->mob->journey_time = journeyTime_next;
	node->mobile = 1;
    	LOG_D("\n mob->journey_time_next %f",journeyTime_next );
	
	node->mob->start_journey = cur_time;
   	LOG_D("\nstart_journey %.3f", node->mob->start_journey );
	pair->a = node->mob->start_journey + journeyTime_next;      //when to reach the destination
	LOG_D("\nwhen to reach the destination: pair->a= start journey + journey_time next =%.3f\n", pair->a);

	pair->b = node;
	return pair;
	}
	


//second method
void update_rwp_nodes(double cur_time) {
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
 			//LOG_D("\n OKiiiiiiiiiiiiiiiiiiiiiii " );
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
				LOG_D("\n node %d slept enough...let's move again ",  my_node->ID);
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
			    LOG_D("\ndestination not yet reached\nfrom (%.3f\n, %.3f)\nto (%.3f\n, %.3f)\nspeed %.3f\nX_pos %.3f\nY_pos %.3f", tmp->pair->b->mob->X_from, tmp->pair->b->mob->Y_from, tmp->pair->b->mob->X_to, tmp->pair->b->mob->Y_to,tmp->pair->b->mob->speed, tmp->pair->b->X_pos, tmp->pair->b->Y_pos);
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
			    tmp->pair->b->X_pos = (double) ((int) (X_now*1000))/ 1000;
			    tmp->pair->b->Y_pos = (double) ((int) (Y_now*1000))/ 1000;
 				   //tmp->pair->b->mob->X_from = tmp->pair->b->X_pos;
				   //tmp->pair->b->mob->Y_from = tmp->pair->b->Y_pos;
				   //tmp->pair->b->mob->start_journey = cur_time;
			    LOG_D("\nupdated_position of node number %d is :(%.3f, %.3f)", tmp->pair->b->ID, tmp->pair->b->X_pos, tmp->pair->b->Y_pos);
			    //LOG_D("\nupdated position of node %d is : (%.3f, %.3f)\n", tmp->pair->b->ID ,tmp->pair->b->X_pos, tmp->pair->b->Y_pos);
			  }
				
			else
			{
			   LOG_E("update_generator: unsupported node state - mobile : %d \n", tmp->pair->b->mobile);
			}
		    tmp = tmp->next;
		    }
		}
		else {
			LOG_D("\nERROR current time > first job_time" );
		}	
}	
}










/*void update_rwp_node_position(NodePtr node, Job_list Job_Vector, double update_time) {
	int nID = node->ID;
	double reach_dest_time = fetch_reach_dest_time(Job_Vector, nID);
	double wake_up_time = fetch_wake_up_time(Job_Vector, nID);

	LOG_D("\nupdate_time: %lf \nreach_dest_time: %lf \nwake_up_time: %lf", update_time, reach_dest_time, wake_up_time );

	if (update_time < reach_dest_time ){  // node is still moving

		LOG_D("\ndestination not yet reached\nX_from %f\nY_from %f\nX_to %f\nY_to %f\nspeed %f", node->mob->X_from, node->mob->Y_from, node->mob->X_to, node->mob->Y_to,node->mob->speed);

		double len = sqrtf(pow(node->mob->X_from - node->mob->X_to,2)+pow(node->mob->Y_from - node->mob->Y_to,2));
		double dx = fabs(node->mob->X_from - node->mob->X_to) / len;
		double dy = fabs(node->mob->Y_from - node->mob->Y_to) / len;
		//LOG_D("\nlen %f\ndx %f\ndy %f", len, dx, dy);
		//LOG_D("\ndx * len + xpos  = xTo %f", dx*len+node->X_pos);
		
		//LOG_D("\n\nnode->mob->speed * (get_time() - node->refresh_time) %f",node->mob->speed * (get_time() - node->refresh_time));

		double X_now=0.0;
		double Y_now=0.0;
		if (node->mob->X_from < node->mob->X_to ){
			 X_now = node->X_pos + dx * (node->mob->speed * (update_time - node->mob->start_journey));
		}
		else{
			 X_now = node->X_pos - dx * (node->mob->speed * (update_time - node->mob->start_journey));
		}
		if (node->mob->Y_from < node->mob->Y_to ){
			 Y_now = node->Y_pos + dy * (node->mob->speed * (update_time - node->mob->start_journey));
		}
		else{
			 Y_now = node->Y_pos - dy * (node->mob->speed * (update_time - node->mob->start_journey));
		}

		//LOG_D("\nX_now %f\nY_now %f", X_now, Y_now);
		node->X_pos = X_now;
		node->Y_pos = Y_now;
  		LOG_D("\nupdated position of node %d is : (%f, %f)\n", nID ,node->X_pos, node->Y_pos);
	}
	else if ( update_time > reach_dest_time && update_time < wake_up_time ){ //node has reached dest and is sleeping 
		node->X_pos = node->mob->X_to;
		node->Y_pos = node->mob->Y_to;
		node->mob->X_from = node->mob->X_to;
		node->mob->Y_from = node->mob->Y_to;
		node->mob->speed = 0.0;
		LOG_D("\nnode has already reached the target position and is sleeping at location: (%f, %f)\n", node->X_pos, node->Y_pos);

	}
	else { // node's been wocken up and started a new journey, update again 
		LOG_D("\nnode's been wocken up and started a new journey, update again" );
		Job_Vector = NULL;

		move_rwp_node(node, param_list, triplet, wake_up_time);
		sleep_node(node, param_list, Job_Vector);

		update_node_position(NodePtr node, Job_list Job_Vector, double update_time);

	}*/





