#ifndef RWP_H_
#define RWP_H_

#include "omg.h"
int start_rwp_generator(omg_global_param omg_param_list);

Pair move_rwp_node(NodePtr node, double cur_time) ;

void update_rwp_nodes(double cur_time);

void get_rwp_positions_updated(double cur_time);

void place_rwp_node(NodePtr node) ;

Pair sleep_rwp_node(NodePtr node, double cur_time);

#endif /* RWP_H_ */
