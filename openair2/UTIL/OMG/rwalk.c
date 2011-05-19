#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "rwalk.h"
#include "omg.h"

//const double min_azimuth = 0.0;
//const double max_azimuth = 360.0;

int start_rwalk_generator(omg_global_param omg_param_list) { 
 
  int n_id=0;
  //omg_omg_param_list.seed= time(NULL); 
  srand(omg_param_list.seed);
  Node_Vector_Rwalk = NULL;
  Job_Vector_Rwalk = NULL;
  Job_Vector_Rwalk_len = 0; 
  double cur_time = 0.5; 
  NodePtr node = NULL;
  MobilityPtr mobility = NULL;
  
  if (omg_param_list.nodes <= 0){
    LOG_W(OMG,"Number of nodes has not been set\n");
    return(-1);
  }
  set_time(cur_time);
  
  
  for (n_id = 0; n_id< omg_param_list.nodes; n_id++) {
    
    node = (NodePtr) create_node();
    mobility = (MobilityPtr) create_mobility();
    
    node->ID = n_id;
    node->type = omg_param_list.nodes_type; 
    node->generator = omg_param_list.mobility_type; 
    node->mob = mobility;
    
    place_rwalk_node(node);	//initial positions
    
    Pair pair = malloc (sizeof(Pair));
    pair = sleep_rwalk_node(node, cur_time); //sleep
    
    Job_Vector_Rwalk = add(pair, Job_Vector_Rwalk,  node->generator);
    
    if (Job_Vector_Rwalk == NULL)
      LOG_D(OMG,"Job Vector is NULL\n");
    // else
    // LOG_T(OMG,"\nJob_Vector_Rwalk->pair->b->ID %d\n", Job_Vector_Rwalk->pair->b->ID);
  }

  LOG_I(OMG, "*****DISPLAY NODE LIST********\n"); 
  display_node_list(Node_Vector_Rwalk, 2);
  LOG_T(OMG, "********DISPLAY JOB LIST********\n"); 
  display_job_list(Job_Vector_Rwalk);
  Job_Vector_Rwalk = quick_sort (Job_Vector_Rwalk);
  LOG_T(OMG, "**********DISPLAY JOB LIST AFTER SORTING**********\n"); 
  display_job_list(Job_Vector_Rwalk);
  return(0);
}
void place_rwalk_node(NodePtr node) {

    	
	node->X_pos = (double) ((int) (randomGen(omg_param_list.min_X, omg_param_list.max_X)*1000))/ 1000;
	node->mob->X_from = node->X_pos;
	node->mob->X_to = node->X_pos;
	node->Y_pos = (double) ((int) (randomGen(omg_param_list.min_Y,omg_param_list.max_Y)*1000))/ 1000;
	node->mob->Y_from = node->Y_pos;
	node->mob->Y_to = node->Y_pos;

	node->mob->speed = 0.0;
	node->mob->journey_time = 0.0;

	LOG_D(OMG, "\n ********INITIALIZE NODE******** ");
    	LOG_D(OMG, "\nID: %d\nX = %.3f\nY = %.3f\nspeed = 0.0", node->ID, node->X_pos, node->Y_pos);   
	Node_Vector_Rwalk = (Node_list) add_entry(node, Node_Vector_Rwalk);
}

Pair sleep_rwalk_node(NodePtr node, double cur_time){
	node->mobile = 0;
	node->mob->speed = 0.0;
	node->mob->X_from = node->mob->X_to;
	node->mob->Y_from = node->mob->Y_to;
	node->X_pos = node->mob->X_to;
	node->Y_pos = node->mob->Y_to;
	Pair pair = malloc(sizeof(Pair)) ;

	node->mob->sleep_duration = (double) ((int) (randomGen(omg_param_list.min_sleep, omg_param_list.max_sleep)*1000))/ 1000;
	LOG_D(OMG, "\nnode: %d \nsleep duration : %.3f",node->ID, node->mob->sleep_duration);

	node->mob->start_journey = cur_time;
	pair->a = node->mob->start_journey + node->mob->sleep_duration; //when to wake up
	LOG_D(OMG, "\nto wake up at time: cur_time + sleep_duration : %.3f", pair->a);
	pair->b = node;

	return pair;
}

 
Pair move_rwalk_node(NodePtr node, double cur_time) {
	Pair pair = malloc(sizeof(Pair));
	LOG_D(OMG,"\n\nMOVE RWALK NODE\n ID: %d",node->ID);
	
	node->mob->X_from = node->X_pos;
	node->mob->Y_from = node->Y_pos;

	double speed_next = randomGen(omg_param_list.min_speed, omg_param_list.max_speed);
	node->mob->speed = speed_next;
    	LOG_D(OMG,"\nspeed_next: %f", speed_next); //m/s

	double azimuth_next = randomGen(omg_param_list.min_azimuth, omg_param_list.max_azimuth);
	node->mob->azimuth = azimuth_next;
    	LOG_D(OMG,"\nazimuth_next: %f", node->mob->azimuth); 

	double journeyTime_next = randomGen(omg_param_list.min_journey_time, omg_param_list.max_journey_time);
	node->mob->journey_time = journeyTime_next;
    	LOG_D(OMG,"\njourney_time_next: %f", node->mob->journey_time); 

	double distance = node->mob->speed * node->mob->journey_time;
	LOG_D(OMG,"\ndistance = speed * journey_time: %f", distance); 

	double dX = distance * cos(node->mob->azimuth*M_PI/180);
	LOG_D(OMG,"\ndX = distance * cos(azimuth): %f", dX); 
	double dY = distance * sin(node->mob->azimuth*M_PI/180);
	LOG_D(OMG,"\ndY = distance * sin(azimuth): %f", dY); 
	
	LOG_D(OMG,"\nfrom: (%.3f, %.3f)", node->X_pos, node->Y_pos);
	double X_next = (double) ((int)((node->X_pos + dX) *1000))/ 1000;
	double Y_next = (double) ((int)((node->X_pos + dY) *1000))/ 1000;
	LOG_D(OMG,"\ntheoritical_destination: (%f, %f)", X_next, Y_next);

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
	if (X_next < omg_param_list.min_X){ 
		node->mob->X_to = X_next + omg_param_list.max_X;  
	}
	else if (X_next > omg_param_list.max_X){
		node->mob->X_to =  X_next - omg_param_list.max_X;
	}
	else {
		node->mob->X_to = X_next;
	}

	if (Y_next < omg_param_list.min_Y){ 
		node->mob->Y_to = Y_next + omg_param_list.max_Y;  
	}
	else if (Y_next > omg_param_list.max_Y){
		node->mob->Y_to =  Y_next - omg_param_list.max_Y;
	}
	else {
		node->mob->Y_to = Y_next;
	}
	
	LOG_D(OMG,"\ndestination: (%.3f, %.3f)", node->mob->X_to, node->mob->Y_to);

	node->mob->start_journey = cur_time;
   	LOG_D(OMG,"\nstart_journey %.3f", node->mob->start_journey );
	
	pair->a = (double) ((int) ( (node->mob->start_journey + journeyTime_next) *1000))/ 1000 ;
	LOG_D(OMG,"\npair->a= start journey + journeyTime_next next %lf\n", pair->a);

	pair->b = node;
	return pair;
	
}



void update_rwalk_nodes(double cur_time) {// need to implement an out-of-area check as well as a rebound function to stay in the area

	LOG_D(OMG,"\n\n**********UPDATE**********");
	int l = 0;
	double X_now=0.0;
	double Y_now=0.0;
	
	while (l < Job_Vector_Rwalk_len){
		LOG_D(OMG,"\n		l == %d \n", l); 
		//LOG_D(OMG,"\n		length == %d \n", Job_Vector_Rwalk_len); 
		Pair my_pair = Job_Vector_Rwalk->pair;
		if((my_pair !=NULL) && ( (double)my_pair->a >= cur_time - eps) && ( (double)my_pair->a <= cur_time + eps)) { 
			LOG_D(OMG,"\n %.3f == %.3f ",my_pair->a, cur_time );
 	
			NodePtr my_node= (NodePtr)my_pair->b;
			if(my_node->mobile == 1) {
 				LOG_D(OMG,"\n stop node and let it sleep " );
				// stop node and let it sleep
				my_node->mobile = 0;
				Pair pair = malloc(sizeof(Pair));
				pair = sleep_rwalk_node(my_node, cur_time);
				Job_Vector_Rwalk->pair = pair;
			}
			else if (my_node->mobile ==0) {
				LOG_D(OMG,"\n node slept enough...let's move again " );
				// node slept enough...let's move again
				my_node->mobile = 1;
				Pair pair = malloc(sizeof(Pair));
				pair = move_rwalk_node(my_node, cur_time);
				Job_Vector_Rwalk->pair = pair;
			}
			else
			  {LOG_E(OMG, "update_generator: unsupported node state - mobile : %d \n", my_node->mobile);}
			//sorting the new entries
			LOG_D(OMG,"\n  **********DISPLAY NODE LIST**********"); 
			display_node_list(Node_Vector_Rwalk,2);
			LOG_D(OMG,"\n\n **********DISPLAY JOB LIST********** "); 
			display_job_list(Job_Vector_Rwalk);
			Job_Vector_Rwalk = quick_sort (Job_Vector_Rwalk);
			LOG_D(OMG,"\n\n **********DISPLAY JOB LIST AFTER SORTING**********"); 
			display_job_list(Job_Vector_Rwalk);
			l++;
			
		}
		else if ( (my_pair !=NULL) && (cur_time < my_pair->a ) ){
		    LOG_D(OMG,"\n\n %.3f < %.3f ",cur_time, my_pair->a);
		    l = Job_Vector_Rwalk_len;
		    LOG_D(OMG,"\nNothing to do");
		}
		else {
			LOG_D(OMG,"\nERROR current time > first job_time" );
		}	
}	
}


void get_rwalk_positions_updated(double cur_time){
	int l = 0;
	double X_now=0.0;
	double Y_now=0.0;
	LOG_D(OMG,"\n\n**********GET POSITIONS**********"); 
		Pair my_pair = Job_Vector_Rwalk->pair;
		if ( (my_pair !=NULL) && (cur_time <= my_pair->a ) ){
		    LOG_D(OMG,"\n\n %.3f <= %.3f ",cur_time, my_pair->a);
		    Job_list tmp = Job_Vector_Rwalk;
		    while (tmp != NULL){
			if (tmp->pair->b->mobile == 0){ //node is sleeping
			    LOG_D(OMG,"\nnode number %d is sleeping at location: (%.3f, %.3f)", tmp->pair->b->ID, tmp->pair->b->X_pos, tmp->pair->b->Y_pos);
			    LOG_D(OMG,"\nnothing to do");
			} 
			else if (tmp->pair->b->mobile == 1){ //node is moving
			     LOG_D(OMG,"\n\nNode_number %d", tmp->pair->b->ID);
			    LOG_D(OMG,"\ndestination not yet reached\nX_from %.3f\nY_from %.3f\nX_to %.3f\nY_to %.3f\nspeed %.3f\nX_pos %.3f\nY_pos %.3f", tmp->pair->b->mob->X_from, tmp->pair->b->mob->Y_from, tmp->pair->b->mob->X_to, tmp->pair->b->mob->Y_to,tmp->pair->b->mob->speed, tmp->pair->b->X_pos, tmp->pair->b->Y_pos);
			    double len = sqrtf(pow(tmp->pair->b->mob->X_from - tmp->pair->b->mob->X_to,2)+pow(tmp->pair->b->mob->Y_from - tmp->pair->b->mob->Y_to,2));
			    double dx = fabs(tmp->pair->b->mob->X_from - tmp->pair->b->mob->X_to) / len;
				  
			    double dy = fabs(tmp->pair->b->mob->Y_from - tmp->pair->b->mob->Y_to) / len;
			    LOG_D(OMG,"\nlen %f\ndx %f\ndy %f", len, dx, dy);
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

			    LOG_D(OMG,"\nX_now %f\nY_now %f", X_now, Y_now);
			    if (X_now < omg_param_list.min_X){ 
				X_now = X_now + omg_param_list.max_X;  
				}
				else if (X_now > omg_param_list.max_X){
				X_now =  X_now - omg_param_list.max_X;
				}
				else {
				X_now = X_now;
				}

			    if (Y_now < omg_param_list.min_Y){ 
				Y_now = Y_now + omg_param_list.max_Y;  
				}
			    else if (Y_now > omg_param_list.max_Y){
				Y_now =  Y_now - omg_param_list.max_Y;
				}
			    else {
				Y_now = Y_now;
				}
			    tmp->pair->b->X_pos = (double) ((int) (X_now*1000))/ 1000;
			    tmp->pair->b->Y_pos = (double) ((int) (Y_now*1000))/ 1000;
 				   //tmp->pair->b->mob->X_from = tmp->pair->b->X_pos;
				   //tmp->pair->b->mob->Y_from = tmp->pair->b->Y_pos;
				   //tmp->pair->b->mob->start_journey = cur_time;
			    LOG_D(OMG,"\nupdated_position of node number %d is :(%.3f, %.3f)", tmp->pair->b->ID, tmp->pair->b->X_pos, tmp->pair->b->Y_pos);
			}
				
			else
			{
			  LOG_E(OMG,"update_generator: unsupported node state - mobile : %d \n", tmp->pair->b->mobile);
			   return; 
			}
		    tmp = tmp->next;
		    }
		}
		else {
			LOG_D(OMG,"\nERROR current time > first job_time" );
			
		}	
}








		
