#ifndef COMMON_H_
#define COMMON_H_

#include <stdlib.h>
#include "global_parameters.h"

typedef struct mobility_struct {
	double sleep_duration;
	double speed;
	double azimuth;
	double journey_time;
	double start_journey;
	double X_from;
	double Y_from;
	double X_to;
	double Y_to;
}mobility_structure;

typedef struct mobility_struct *MobilityPtr;

typedef struct node_struct {
	int ID;
	int type; // vehicle/ pedestrian, tram, train...
	int mobile;
	double X_pos;
	double Y_pos;
	mobility_structure *mob;
	int generator; // static  , RWP  , RWalk 
}node_structure;
typedef struct node_struct* NodePtr;


struct node_list_struct {
	NodePtr node;
	struct node_list_struct *next;
}node_list_structure;

typedef struct node_list_struct* Node_list;


typedef struct pair_struct {
	double a;   //time
	node_structure *b; //node
}pair_structure;
typedef struct pair_struct* Pair;



void display_node_list(Node_list Node_Vector, int r);

double randomGen(double a, double b);

NodePtr create_node(void);

MobilityPtr create_mobility(void);


#endif /* COMMON_H_ */
