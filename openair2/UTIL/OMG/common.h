/*
 * common.h
 *
 *  Created on: Jan 28, 2011
 *      Author: jerome haerri
 */

#ifndef COMMON_H_
#define COMMON_H_

typedef struct mobility_struct *Mobility;

struct mobility_struct {
	int cartesian;
	float sleep;
	float speed;
	float journey_time;
	float start_journey;
	float X_from;
	float Y_from;
	float X_to;
	float Y_to;
	float X_angle;
	float Y_angle;
};

typedef struct node_struct *Node;

struct node_struct {
	int ID;
	int type; // vehicle/ pedestrian, tram, train...
	int mobile;
	float X_pos;
	float Y_pos;
	float refresh_time;
	Mobility *mob;
	int generator; // static, RWP, RWalk

	//Technology *tech;
};

typedef struct node_list *Node_Vector;

struct node_list {
	int ID;
	Node *node;
	struct node_list *next;
};

inline Node* create_node(void) {
	return (Node*)malloc(sizeof(Node));
}

inline Mobility* create_mobility(void) {
	return (Mobility*)malloc(sizeof(Mobility));
}

inline Node_Vector* create_entry(Node *node) {
	Node_Vector *entry = malloc(sizeof(Node_Vector));
	entry->ID = node->ID;
	entry->node = node;
	entry->next = NULL;
	return entry;
}


#endif /* COMMON_H_ */
