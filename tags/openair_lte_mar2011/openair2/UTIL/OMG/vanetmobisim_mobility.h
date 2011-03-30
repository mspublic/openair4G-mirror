/*
 * vanetmobisim_mobility.h
 *
 *  Created on: Jan 27, 2011
 *      Author: jerome haerri
 */

#ifndef VANETMOBISIM_MOBILITY_H_
#define VANETMOBISIM_MOBILITY_H_

#include "common.h"
#include "helper.h"

/*typedef struct pair_struct *Pair;

struct pair_struct {
	float time;
	Node *node;
};*/

/* ======================================================================
    Function Prototypes
   ====================================================================== */


void update_node_position(Node*);

void update_node_position();

void update(void);

Node** get_nodes(void);

Node* get_node(int);

void usage(char*);


/* ======================================================================
    Global variable
   ====================================================================== */
static Pair **job_list;

static Node_Vector *nodes;

static int m_time;


#endif /* VANETMOBISIM_MOBILITY_H_ */
