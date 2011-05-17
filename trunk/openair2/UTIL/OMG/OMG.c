#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include <time.h>
#include "global_parameters.h"
#include "OMG_constants.h"
#include "OMG.h"
#include "static.h"
#include "log.h"
#include "common.h"
#include "rwp.h"
#include "rwalk.h"


void usage(void){
	fprintf(stderr, "\n\t-X: assign maximum width of the simulation area (X_max)"\
		"\n\t-x: assign minimum width of the simulation area (X_min)"\
		"\n\t-Y: assign maximum height of the simulation area (Y_max)"\
		"\n\t-y: assign minimum height of the simulation area (Y_min)"\
		"\n\t-N: assign number of nodes"\
		"\n\t-B: assign maximum duration of sleep (max_break)"
		"\n\t-b: assign minimum duration of sleep (min_break)"\
		"\n\t-J: assign maximum duration of journey (max_journey_time)"\
		"\n\t-j: assign minimum duration of journey (min_journey_time)"\
		"\n\t-S: assign maximum speed "\
		"\n\t-s: assign minimum speed"\
		"\n\t-g: choose generator (SATIC: 0x00, RWP: 0x01 or RWALK 0x02)\n"\
	);	
	exit(0);
}
int get_options(int argc, char *argv[]){
	char tag;
	while ((tag = getopt(argc, argv, "j:J:g:B:b:S:s:Y:y:X:x:N:h:")) != EOF) {


		switch (tag) {

		case 'N':
			param_list.nodes = atoi(optarg);
			LOG_D("Number of nodes : %d \n",param_list.nodes);
			break;

		case 'b':
			param_list.min_sleep = atof(optarg);
			LOG_D("min sleep : %.3f \n",param_list.min_sleep);
			break;

		case 'B':
			param_list.max_sleep = atof(optarg);
			LOG_D("max_sleep : %.3f \n",param_list.max_sleep);
			break;

		case 's':
			param_list.min_speed = atof(optarg);
			LOG_D("min_speed : %.3f \n",param_list.min_speed);
			break;

		case 'S':
			param_list.max_speed = atof(optarg);
			LOG_D("max_speed : %.3f \n",param_list.max_speed);
			break;

		case 'X':
			param_list.max_X = atof(optarg);
			LOG_D("X_max : %.3f \n",param_list.max_X);
			break;
		case 'x':
			param_list.min_X = atof(optarg);
			LOG_D("X_min : %.3f \n",param_list.min_X);
			break;

		case 'Y':
			param_list.max_Y = atof(optarg);
			LOG_D("Y_max : %.3f \n",param_list.max_Y);
			break;

		case 'y':
			param_list.min_Y = atof(optarg);
			LOG_D("Y_min : %.3f \n",param_list.min_Y);
			break;

		case 'J':
			param_list.max_journey_time = atof(optarg);
			LOG_D("Journey_time_max : %.3f \n",param_list.max_journey_time);
			break;


		case 'j':
			param_list.min_journey_time = atof(optarg);
			LOG_D("Journey_time_min : %.3f \n",param_list.min_journey_time);
			break;

		
		case 'g':
			param_list.mobility_type = atoi(optarg);
			LOG_D("Mobility type is %d \n",param_list.mobility_type);
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

#ifdef TEST_OMG 
int main(int argc, char *argv[]) {
#else
int OMG_main(x, X, y, Y, s, S, b, B, N, j, J, g, OAI_time) {
#endif

	int i;
	double cur_time=0.0;
	srand(time(NULL));
	double ms=0.0;
	Node_list Current_positions = NULL;
	NodePtr my_node = NULL;
 	int my_ID = 1;

#ifdef TEST_OMG
	get_options(argc, argv);
#else
	param_list.min_X = x;
	param_list.min_X = X;
	param_list.min_Y = y;
	param_list.max_Y = Y;
	param_list.min_speed = s;
	param_list.max_speed = S;
	param_list.min_sleep = b;
	param_list.max_sleep = B;
	param_list.nodes = N;
	param_list.min_journey_time = j;
	param_list.max_journey_time = J;
	param_list.mobility_type = g;
#endif

	if(param_list.max_X == 0.0 || param_list.max_Y == 0.0 || param_list.nodes == 0 ) {
		usage();
		exit(1);
	}

#ifdef TEST_OMG
	set_time(0.001);
#else
	set_time(OAI_time);
#endif
	LOG_D("\ncurrent time =  %.3f", get_time());
	cur_time = get_time();

    	initialize_nodes(cur_time); // initial positions + sleep
	if (param_list.mobility_type != STATIC){
   		for (ms = 0.002; ms<1.0; ms+= 0.001){
			set_time(ms);
			LOG_D("\ncurrent time =  %.3f", get_time());
			cur_time = get_time();
			update_nodes(cur_time);
	    	}
	} else {
		LOG_D("\n\n Nodes are static ");
	    }
     	
	Current_positions = get_current_positions();
	LOG_D("\n********DISPLAY Current_positions LIST********");
	display_node_list(Current_positions,1);
	if (Current_positions == NULL){LOG_D("\n  Current_positions == NULL");  }

	my_node = (NodePtr) get_node_position( my_ID);
	LOG_D("\n********At %.3f, Node number %d is in position: (%.3f, %.3f) with mobility_type: %d",cur_time,my_ID,my_node->X_pos,my_node->Y_pos,my_node->mobile);
	
	return 0;
}

void initialize_nodes(double cur_time) {
	int number_node = param_list.nodes;
    	int i=0;

        Node_Vector = NULL;
	Job_Vector = NULL;
	Job_Vector_len = 0; 

	NodePtr node = NULL;
	MobilityPtr mobility = NULL;

	
   	for (i = 0; i<number_node; i++) {
		
        	node = (NodePtr) create_node();
		mobility = (MobilityPtr) create_mobility();

		node->ID = i;
		node->type = 0; // car
		node->generator = param_list.mobility_type; 
		node->mob = mobility;

		initialize_node(node);	//initial positions
		
		Pair pair = malloc (sizeof(Pair));
		pair = sleep_node(node, param_list,  cur_time); //sleep
		
		Job_Vector = add(pair, Job_Vector);
		//LOG_D("\nJob_Vector->pair->b->ID %d\n", Job_Vector->pair->b->ID);
		if (Job_Vector == NULL){
			LOG_D("\nJob Vector is nulllllllllllllllllllllllllllllllllllll\n");
		}
		
		
	}
		LOG_D("\n  ********DISPLAY NODE LIST********"); 
		display_node_list(Node_Vector, 2);
		LOG_D("\n\n ********DISPLAY JOB LIST******** "); 
		display_job_list(Job_Vector);
		Job_Vector = quick_sort (Job_Vector);
		LOG_D("\n\n **********DISPLAY JOB LIST AFTER SORTING**********"); 
		display_job_list(Job_Vector);
}


void initialize_node(NodePtr node) {

    	
	node->X_pos = (double) ((int) (randomGen(param_list.min_X, param_list.max_X)*1000))/ 1000;
	node->mob->X_from = node->X_pos;
	node->mob->X_to = node->X_pos;
	node->Y_pos = (double) ((int) (randomGen(param_list.min_Y,param_list.max_Y)*1000))/ 1000;
	node->mob->Y_from = node->Y_pos;
	node->mob->Y_to = node->Y_pos;

	node->mob->speed = 0.0;
	node->mob->journey_time = 0.0;

	LOG_D("\n ********INITIALIZE NODE******** ");
    	LOG_D("\nID: %d\nX = %.3f\nY = %.3f\nspeed = 0.0", node->ID, node->X_pos, node->Y_pos);   
	Node_Vector = (Node_list) add_entry(node, Node_Vector);
}

Pair sleep_node(NodePtr node, m_global_param param_list, double cur_time){
	node->mobile = 0;
	node->mob->speed = 0.0;
	node->mob->X_from = node->mob->X_to;
	node->mob->Y_from = node->mob->Y_to;
	node->X_pos = node->mob->X_to;
	node->Y_pos = node->mob->Y_to;
	Pair pair = malloc(sizeof(Pair)) ;

	node->mob->sleep_duration = (double) ((int) (randomGen(param_list.min_sleep, param_list.max_sleep)*1000))/ 1000;
	LOG_D("\nnode: %d \nsleep duration : %.3f",node->ID, node->mob->sleep_duration);

	node->mob->start_journey = cur_time;
	pair->a = node->mob->start_journey + node->mob->sleep_duration; //when to wake up
	LOG_D("\nto wake up at time: cur_time + sleep_duration : %.3f", pair->a);
	pair->b = node;

	return pair;
}


Pair move_node(NodePtr node, m_global_param param_list, double cur_time) {
	switch (param_list.mobility_type) {
	    case RWP:
	    	move_rwp_node(node, param_list, cur_time);
		break;

	    
	    case RWALK:
	         move_rwalk_node(node, param_list, cur_time);
	         break;

	    case STATIC:
	    default:
	    	LOG_N("Static or Unsupported generator %c \n", param_list.mobility_type);
	}
}

void update_nodes(double cur_time){
	switch (param_list.mobility_type) {
	    case RWP:
	    	update_rwp_nodes(cur_time);
			LOG_D("\n  **********DISPLAY NODE LIST**********"); 
			display_node_list(Node_Vector,2);
			LOG_D("\n\n **********DISPLAY JOB LIST********** "); 
			display_job_list(Job_Vector);
		break;

	    
	    case RWALK:
	         update_rwalk_nodes(cur_time);
			LOG_D("\n  **********DISPLAY NODE LIST**********"); 
			display_node_list(Node_Vector,2);
			LOG_D("\n\n **********DISPLAY JOB LIST********** "); 
			display_job_list(Job_Vector);
	    break;

	    case UNDEF:
	    default:
	    	LOG_N("Unsupported generator %c \n", param_list.mobility_type);
	}
}


Node_list get_current_positions(){
	Node_list Current_positions = NULL;
	Node_list tmp = Node_Vector;
	
	if (tmp == NULL){LOG_D("\n !!!!!!!!!!!!!!!!!!!!!!!!!!Node_Vector == NULL"); }
   	while (tmp != NULL){
		Current_positions = (Node_list) add_entry(tmp->node, Current_positions);
		tmp = tmp->next;	
    }
	return Current_positions;
}

NodePtr get_node_position(int nID){
	Node_list tmp = Node_Vector;
	if (tmp == NULL){LOG_D("\n !!!!!!!!!!!!!!!!!!!!!!!!!!Node_Vector == NULL"); }
   	while (tmp != NULL){
		if (tmp->node->ID == nID){
			return tmp->node;
		}
		tmp = tmp->next;
    }

}

// openair emu will set this valut as a function of frame number
void set_time(double time) {
	m_time = time;
}

double get_time() {
	return m_time;
}


//firt method
/*void update_nodes(double cur_time) {
	LOG_D("\n\n**********UPDATE**********");
	//Job_list tmp = Job_Vector;
	int l = 0;
	
//	if (param_list.mobility_type = STATIC) {
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
				pair = sleep_node(my_node, param_list, Job_Vector, cur_time);
				Job_Vector->pair = pair;
			}
			else if (my_node->mobile ==0) {
				LOG_D("\n node slept enough...let's move again " );
				// node slept enough...let's move again
				my_node->mobile = 1;
				Pair pair = malloc(sizeof(Pair));
				pair = move_node(my_node, param_list, Job_Vector, cur_time);
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













