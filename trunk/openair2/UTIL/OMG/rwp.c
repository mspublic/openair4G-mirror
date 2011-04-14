/*
 * rwp.c
 *
 *  Created on: Jan 28, 2011
 *      Author: jerome haerri
 */

#include "rwp.h"
#include "job.h"

void start_RWP_Generator(Node_Vector *nodes, m_global_param param_list) {
	int number_node = param_list->nodes;
	/*
	 * TODO fill a structure with basic configuration parameters
	 */

	// if job_list has not been initialized
	// job_list = [number_node];
	// else
	// resize with number_node

	init_job();

	for (int i = 0; i<number_node; i++) {

		Node *node = create_node();
		node->ID = i;
		node->type = 0; // car
		node->generator = RWP;

		nodes->next = create_entry(node);
		Mobility *mobility = create_mobility();
		node->mob = mobility;
		node->mob->cartesian = 1;
		initialize(node);
		sleep_node(node);
	}

	/*
	 * sorting the joblist upon creation and after each modification
	 */
	qsort(jobs, job_len, sizeof(Jobs), pair_cmp);
}

void initialize_rwp_node(Node *node) {
	node->X_pos = random(min_X,max_X);
	node->mob->X_from = node->X_pos;
	node->Y_pos = random(min_Y,max_Y);
	node->mob->Y_from = Y_pos;
	node->refresh_time = now();

	node->mob->speed = 0;
	node->journey_time = 0;
	node->speed = 0;

}


/*
 * TODO find a structure to keep the basic configuration parameters
 */
void move_rwp_node(Node *node) {

	float X_next = random(min_X,max_X);
	node->mob->X_to = X_next;
	float Y_next = random(min_Y,max_Y);
	node->mob->Y_to = Y_next;

	float speed_next = random(min_speed, max_speed);
	node->mob->speed = speed_next;

	float distance = sqrt(pow(X_from - X_next, 2) + pow(Y_from - Y_next, 2));
	float journeyTime_next = distance/speed_next;
	node->mob->journeyTime = journeyTime_next;
	node->mobile = 1;
	node->refresh_time = now();
	node->mob->start_journey = now();

	Pair *pair;
	pair->a = &(now()+journeyTime );
	pair->b = node;

	push_job(pair);
}

/*
 * TODO find a structure to keep the basic configuration parameters
 */
void sleep_rwp_node(Node *node) {
	node->X_pos = node->mob->X_to;
	node->Y_pos = node->mob->Y_to;
	node->refresh_time = now();
	node->mobile = 0;

	node->mob->X_from=mob->X_to;
	node->mob->Y_from=mob->Y_to;
	node->mob->speed = 0.0;

	float sleep_next = random(min_sleep, max_sleep);
	node->mob->sleep = sleep_next;

	Pair *pair;
	pair->a = &(now());
	pair->b = node;

	push_job(pair);
}

void update_rwp_node_position(Node *node) {

	float len = sqrt(pow(X_from - X_to,2)+pow(Y_from - Y_to,2));
	float dx = abs(X_from - X_to) / len;
	float dy = abs(Y_from - Y_to) / len;

	float X_now = node->X_pos + dx * (node->mob.speed * (now() - node->refresh_time));
	float X_now = node->Y_pos + dy * (node->mob.speed * (now() - node->refresh_time));

	node->X_pos = X_now;
	node->Y_pos = Y_now;
	node->refresh_time = now();
}


