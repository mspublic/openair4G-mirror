/*
 * rwalk.c
 *
 *  Created on: Feb 3, 2011
 *      Author: jerome haerri
 */

#include "rwp.h"
#include "job.h"

void start_RWALK_Generator(Node_Vector *nodes, m_global_param param_list) {
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
		node->generator = RWALK;

		nodes->next = create_entry(node);
		Mobility *mobility = create_mobility();
		node->mob = mobility;
		node->mob->cartesian = 0;
		initialize(node);
		sleep_node(node);
	}

	/*
	 * sorting the joblist upon creation and after each modification
	 */
	qsort(jobs, job_len, sizeof(Jobs), pair_cmp);
}

void initialize_rwalk_node(Node *node) {
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
void move_rwalk_node(Node *node) {

	float X_next = random(min_X,max_X);
	node->mob->X_angle = X_next;
	float Y_next = random(min_Y,max_Y);
	node->mob->Y_angle = Y_next;

	float speed_next = random(min_speed, max_speed);
	node->mob->speed = speed_next;

	float journeyTime_next = random(min_journey, max_journey);
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
void sleep_rwalk_node(Node *node) {
	update_rwalk_node_position(node);

	node->refresh_time = now();
	node->mobile = 0;

	node->mob->X_from=node->X_pos;
	node->mob->Y_from=node->Y_pos;
	node->mob->speed = 0.0;
	node->mob->journey_time = 0.0;

	float sleep_next = random(min_sleep, max_sleep);
	node->mob->sleep = sleep_next;

	Pair *pair;
	pair->a = &(now());
	pair->b = node;

	push_job(pair);
}

void update_rwalk_node_position(Node *node) {

	float len = sqrt(pow(node->mobility->X_from - node->mobility->X_angle,2)+pow(node->mobility->Y_from - node->mobility->Y_angle,2));
	float dx = abs(node->mobility->X_from - node->mobility->X_angle) / len;
	float dy = abs(node->mobility->Y_from - node->mobility->Y_from) / len;

	float X_now = node->X_pos + dx * (node->mob.speed * (now() - node->refresh_time));
	float X_now = node->Y_pos + dy * (node->mob.speed * (now() - node->refresh_time));

	// need to implement an out-of-area check as well as a rebound function to stay in the area
	node->X_pos = X_now;
	node->Y_pos = Y_now;
	node->refresh_time = now();
}
