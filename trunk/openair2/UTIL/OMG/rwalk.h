/*
 * rwalk.h
 *
 *  Created on: Feb 3, 2011
 *      Author: jerome haerri
 */

#ifndef RWALK_H_
#define RWALK_H_

#include "common.h"
#include "OMG_constants.h"
#include "OMG.h"




Pair move_rwalk_node(NodePtr node, m_global_param param_list, double cur_time);

void update_rwalk_nodes(double cur_time);

#endif /* RWALK_H_ */
