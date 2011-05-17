#ifndef RWP_H_
#define RWP_H_

#include "common.h"
#include "global_parameters.h"
#include "OMG_constants.h"
#include "OMG.h"


Pair move_rwp_node(NodePtr node, m_global_param param_list, double cur_time) ;

void update_rwp_nodes(double cur_time);

#endif /* RWP_H_ */
