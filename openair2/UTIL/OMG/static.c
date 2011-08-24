/*
 * static.c
 *
 *  Created on: Jan 28, 2011
 *      Author: haerri
 */

#include "static.h"

void start_Static_Generator(m_global_param param_list) {
	int number_node = param_list->nodes;

	for (int i = 0; i<number_node; i++) {

		Node *node = create_node();
		Mobility *mobility = create_mobility();

		node->type = 0; // car
		node->generator = STATIC;

		node->mob = mobility;
		node->mob->cartesian = 1;
		initialize(node);
		node->mob->speed = 0.0;
	}

}

void initialize_node(Node *node) {
	node->X_pos = random(min_X,max_X);
	node->mob->X_from = node->X_pos;
	node->Y_pos = random(min_Y,max_Y);
	node->mob->Y_from = Y_pos;
	node->refresh_time = now();

	node->mob->speed = 0;
	node->journey_time = 0;
	node->speed = 0;

}

