#ifndef __DEFS_H__
#define  __DEFS_H__

#include <stdlib.h>

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
}mobility_struct;

typedef struct mobility_struct *MobilityPtr;

typedef struct node_struct {
	int ID;
	int type; // vehicle/ pedestrian, tram, train...
	int mobile;
	double X_pos;
	double Y_pos;
	mobility_struct *mob;
	int generator; // static  , RWP  , RWalk 
}node_struct;

typedef struct node_struct* NodePtr;

struct node_list_struct {
	node_struct *node;
	struct node_list_struct *next;
}node_list_struct;

typedef struct node_list_struct* Node_list;

typedef struct pair_struct {
	double a;   //time
	node_struct *b; //node
}pair_struct;

typedef struct pair_struct* Pair;

struct job_list_struct {
	Pair pair;
	struct job_list_struct *next;
}job_list_struct;
typedef struct job_list_struct* Job_list;


typedef struct omg_global_param{
	int nodes;
	double min_X;
	double max_X;
	double min_Y;
	double max_Y;
	double min_speed; // must NOT be 0.0 to avoid instability
	double max_speed;
	double min_journey_time; // must NOT be 0.0 to avoid instability
	double max_journey_time; 
	double min_azimuth; // RWALK direction
	double max_azimuth; 
	double min_sleep; // must NOT be 0.0 to avoid instability
	double max_sleep;
	int mobility_type;
  int seed; 
  int run; // to be checked later  
}omg_global_param;


// func prototype
void display_node_list(Node_list Node_Vector, int r);

double randomGen(double a, double b);

NodePtr create_node(void);

MobilityPtr create_mobility(void);

Job_list add(Pair job, Job_list Job_Vector, int mobility_type);

void display_job_list(Job_list Job_Vector);
Job_list job_list_sort (Job_list list, Job_list end);
Job_list quick_sort (Job_list list);

#endif /*  __DEFS_H__ */
