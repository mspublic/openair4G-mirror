/*
 * rwalk.h
 *
 *  Created on: Feb 3, 2011
 *      Author: jerome haerri
 */

#ifndef RWALK_H_
#define RWALK_H_

#include "common.h"

void move_rwalk_node(Node*);
void sleep_rwalk_node(Node*);
void initialize_rwalk_node(Node*);


void start_RWALK_Generator(Node_Vector*, m_global_param param_list);

#endif /* RWALK_H_ */
