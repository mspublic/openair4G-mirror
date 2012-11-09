/*******************************************************************************

  Eurecom OpenAirInterface
  Copyright(c) 1999 - 2011 Eurecom

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fsr/openairinterface
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/
/*
 * \file rwp.h
 * \brief Functions used for the Random WayPoint Model
 * \date 22 May 2011
 * 
 */

#ifndef RWP_H_
#define RWP_H_

#include "omg.h"


/**
 * \fn void start_rwp_generator(omg_global_param omg_param_list)
 * \brief Start the RWP model by setting the initial positions of each node then letting it sleep for a random duration and adding this job to the Job_Vector
 * \param omg_param_list a structure that contains the main parameters needed to establish the random positions distribution
 */
int start_rwp_generator(omg_global_param omg_param_list) ;

/**
 * \fn void place_rwp_node(NodePtr node)
 * \brief Called by the function start_rwp_generator(), it generates a random position ((X,Y) coordinates)  for a node and add it to the Node_Vector_Rwp 
 * \param node a pointer of type NodePtr that represents the node to which the random position is assigned
 */
void place_rwp_node(NodePtr node);

/**
 * \fn Pair sleep_rwp_node(NodePtr node, double cur_time)
 * \brief Called by the function start_rwp_generator(omg_global_param omg_param_list) and update_rwp_nodes(double cur_time), it allows the node to sleep for a random duration starting
 * 	from the current time
 * \param node a pointer of type NodePtr that represents the node to which the random sleep duration is assigned
 * \param cur_time a variable of type double that represents the current time
 * \return A Pair structure containing the node and the time when it is reaching the destination
 */
Pair sleep_rwp_node(NodePtr node, double cur_time) ;

/**
 * \fn Pair move_rwp_node(NodePtr node, double cur_time)
 * \brief Called by the function update_rwp_nodes(double cur_time), it computes the next destination of a node ((X,Y) coordinates and the arrival
 *	 time at the destination)
 * \param node a variable of type NodePtr that represents the node that has to move
 * \param cur_time a variable of type double that represents the current time
 * \return A Pair structure containing the node structure and the arrival time at the destination
 */
Pair move_rwp_node(NodePtr node, double cur_time) ;

/**
 * \fn void update_rwp_nodes(double cur_time)
 * \brief Update the positions of the nodes. After comparing the current time to the first job_time, it is decided wether to start
 * \	a new job or to keep the current job 
 * \param cur_time a variable of type double that represents the current time
 */
void update_rwp_nodes(double cur_time) ;

/**
 * \fn void get_rwp_positions_updated(double cur_time)
 * \brief Compute the positions of the nodes at a given time in case they are moving (intermediate positions). Otherwise, generate a message saying that 
 * 	the nodes are still sleeping
 * \param cur_time a variable of type double that represents the current time
 */
void get_rwp_positions_updated(double cur_time);


#endif /* RWP_H_ */
