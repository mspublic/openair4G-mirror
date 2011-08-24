/*
 * rwp.h
 *
 *  Created on: Jan 28, 2011
 *      Author: jerome haerri
 */

#ifndef RWP_H_
#define RWP_H_

#include "common.h"

void move_rwp_node(Node*);
void sleep_rwp_node(Node*);
void initialize_rwp_node(Node*);


void start_RWP_Generator(Node_Vector*, m_global_param param_list);

#endif /* RWP_H_ */
