/*
 * rwalk.h
 *
 *  Created on: Feb 3, 2011
 *      Author: jerome haerri
 */

#ifndef RWALK_H_
#define RWALK_H_

#include "omg.h"

int start_rwalk_generator(omg_global_param omg_param_list);

void place_rwalk_node(NodePtr node);

Pair sleep_rwalk_node(NodePtr node, double cur_time);

Pair move_rwalk_node(NodePtr node, double cur_time);

void update_rwalk_nodes(double cur_time);

void get_rwalk_positions_updated(double cur_time);

#endif /* RWALK_H_ */
