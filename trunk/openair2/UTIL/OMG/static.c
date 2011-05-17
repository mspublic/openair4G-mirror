#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "global_parameters.h"
#include "common.h"
#include "static.h"
#include "log.h"
#include "OMG_constants.h"
#include "OMG.h"




void start_Static_Generator() {
	int number_node = param_list.nodes;
	int i;

	NodePtr node;
	MobilityPtr mobility;
	
	printf("\nSTATIC MODEL\n");
	
        srand(time(NULL));
	for (i = 0; i<number_node; i++) {

		node = (NodePtr) create_node();
		mobility = (MobilityPtr) create_mobility();

		node->ID = i;
		node->type = 1;//car
		node->generator = STATIC;

		node->mob = mobility;
		node->mob->speed = 0.0;

		initialize_node(node);
	}

}


