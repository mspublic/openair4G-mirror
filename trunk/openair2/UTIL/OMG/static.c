#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "omg.h"

void start_static_generator(omg_global_param omg_param_list) {
  
  int n_id=0;
  double cur_time = 0.5; 
  NodePtr node = NULL;
  MobilityPtr mobility = NULL;
  
  if (omg_param_list.nodes <= 0){
    LOG_W(OMG,"Number of nodes has not been set\n");
    return(-1);
  }
  LOG_I(OMG, "\nStart STATIC Mobility MODEL\n");

  srand(omg_param_list.seed); 
  for (n_id = 0; n_id< omg_param_list.nodes; n_id++) {
    
    node = (NodePtr) create_node();
    mobility = (MobilityPtr) create_mobility();
    
    node->ID = n_id;
    node->type = 0; 
    node->generator = omg_param_list.mobility_type; 
    node->mob = mobility;
    
    place_static_node(node);	//initial positions
    
  }
  return(0);
}
  
void place_static_node(NodePtr node) {

    	
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
	Node_Vector_Static = (Node_list) add_entry(node, Node_Vector_Static);
 
}

