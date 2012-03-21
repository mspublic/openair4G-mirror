/*
 * trace.h
 *
 *  Created on: Aug 11, 2011
 *      Author: suppoor
 */
#ifndef TRACE_H_
#define TRACE_H_
//#include "defs.h"
#include "omg.h"
#include "hashtable.h"
#include "mobility_parser.h"


/**
 * \fn void start_trace_generator(omg_global_param omg_param_list)
 * \brief Start the TRACE model by setting the initial positions of each node then letting it sleep for a random duration and adding this job to the Job_Vector
 * \param omg_param_list a structure that contains the main parameters needed to establish the random positions distribution
 */
int start_trace_generator(omg_global_param omg_param_list) ;
hash_table_t* table;
/**
 * \fn int create_trace_node(node_info *head_node)
 * \brief Called by the function start_trace_generator(), it assigns position ((X,Y) coordinates)  for a node from mobility file and add it to the Node_Vector_Rwp
 * \param node a pointer of type NodePtr that represents the node to which the random position is assigned
 */
int create_trace_node(NodePtr node,node_info *head_node) ;
/**
 * \fn void place_rwp_node(NodePtr node)
 * \brief Called by the function start_rwp_generator(), it generates a random position ((X,Y) coordinates)  for a node and add it to the Node_Vector_Rwp
 * \param node a pointer of type NodePtr that represents the node to which the random position is assigned
 */
void place_trace_node(NodePtr node) ;

/**
 * \fn Pair sleep_rwp_node(NodePtr node, double cur_time)
 * \brief Called by the function start_rwp_generator(omg_global_param omg_param_list) and update_rwp_nodes(double cur_time), it allows the node to sleep for a random duration starting
 * 	from the current time
 * \param node a pointer of type NodePtr that represents the node to which the random sleep duration is assigned
 * \param cur_time a variable of type double that represents the current time
 * \return A Pair structure containing the node and the time when it is reaching the destination
 */
Pair sleep_trace_node(NodePtr node, double cur_time,float sleep_duration) ;

/**
 * \fn Pair move_rwp_node(NodePtr node, double cur_time)
 * \brief Called by the function update_rwp_nodes(double cur_time), it computes the next destination of a node ((X,Y) coordinates and the arrival
 *	 time at the destination)
 * \param node a variable of type NodePtr that represents the node that has to move
 * \param cur_time a variable of type double that represents the current time
 * \return A Pair structure containing the node structure and the arrival time at the destination
 */
Pair move_trace_node(NodePtr node, double cur_time) ;

/**
 * \fn void update_rwp_nodes(double cur_time)
 * \brief Update the positions of the nodes. After comparing the current time to the first job_time, it is decided wether to start
 * \	a new job or to keep the current job
 * \param cur_time a variable of type double that represents the current time
 */
void update_trace_nodes(double cur_time) ;

/**
 * \fn void get_rwp_positions_updated(double cur_time)
 * \brief Compute the positions of the nodes at a given time in case they are moving (intermediate positions). Otherwise, generate a message saying that
 * 	the nodes are still sleeping
 * \param cur_time a variable of type double that represents the current time
 */
void get_trace_positions_updated(double cur_time);


#endif /* TRACE_H_ */
