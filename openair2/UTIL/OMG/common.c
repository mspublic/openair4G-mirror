#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "log.h"

#include "global_parameters.h"
#include "OMG.h"

#define frand() ((double) rand() / (RAND_MAX+1))

NodePtr create_node(void) {
	NodePtr ptr;
	ptr = malloc(sizeof(node_structure));
	return ptr;
}
inline double randomGen(double a, double b){

    return ( rand()/(double)RAND_MAX ) * (b-a) + a;
}


MobilityPtr create_mobility(void) {
	MobilityPtr ptr;
	ptr = malloc(sizeof(mobility_structure));
	return ptr;
}


Node_list add_entry(NodePtr node, Node_list Node_Vector){
    Node_list entry = malloc(sizeof(Node_list));
    entry->node = node;
    entry->next = NULL;
    if (Node_Vector == NULL) {
        LOG_D("\nempty Node_list");
	LOG_D("\nadded elmt ID %d\n", entry->node->ID);
        return entry;
    }
    else {
        Node_list tmp = Node_Vector;
        while (tmp->next != NULL){
            tmp = tmp->next;
        }
        tmp->next = entry;
        LOG_D("\nnon empty Node_list");
	LOG_D("\nadded elmt ID %d\n", entry->node->ID);
	
        return Node_Vector;
    }
}


// display list of nodes
void display_node_list(Node_list Node_Vector, int r){

    Node_list tmp = Node_Vector;
	//int i=0;
    while (tmp != NULL){
	if (r == 1 ){
        	LOG_D("\nnode number %d\nmobility_type %d\nX_pos %.3f\nY_pos %.3f\n", tmp->node->ID,tmp->node->mobile, tmp->node->X_pos,tmp->node->Y_pos );
	}
	else {
        LOG_D("\nnode_number %d\nmobility_type %d\nX_pos %.3f\nY_pos %.3f\nmob->X_from %.3f\nmob->Y_from %.3f\nmob->X_to %.3f\nmob->Y_to %.3f\n", tmp->node->ID,tmp->node->mobile, tmp->node->X_pos,tmp->node->Y_pos, tmp->node->mob->X_from,tmp->node->mob->Y_from, tmp->node->mob->X_to, tmp->node->mob->Y_to );
	}
        tmp = tmp->next;
	//i++;
    }
}

