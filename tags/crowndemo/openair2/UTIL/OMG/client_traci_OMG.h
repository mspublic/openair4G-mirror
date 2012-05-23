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

/*! \file client_traci_OMG.h
* \brief The OMG TraCI to send/receive commands from/to  SUMO via socket interfaces.
* \author  S. Uppoor, J. Harri
* \date 2012
* \version 0.1
* \company INRIA, Eurecom
* \email: sandesh.uppor@inria.fr, haerri@eurecom.fr
* \note
* \warning
*/

#ifndef TRACICLIENT_OMG_H
#define TRACICLIENT_OMG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "omg.h" 

//#include "TraCIConstants.h"
#include "storage_traci_OMG.h"
#include "socket_traci_OMG.h"

#define MAX_ATTEMPTS 5;  // number of connection attemps to SUMO

int targetTime;

char *description;

String_list departed;  // string list of all vehicles appearing in SUMO at the current time step
String_list arrived;  // string list of all vehicles leaving SUMO at the current time step

/**
 * Global parameters defined in storage_traci_OMG.h
 */
extern storage *head;
extern storage *storageStart;
extern storage *tracker;
extern descLen;

/**
 * \fn handshake(char *,int)
 * \brief Talks to SUMO by establishing connection
 * \param Accepts host name and port number
 */
int handshake(char *,int);

/**
 * \fn extractCommandStatus(storage *, unsigned char , char *)
 * \brief Validates the RESPONSE recieved by SUMO 
 * \param Accepts storage pointer where data is read and stored, command ID for which response is recieved, description 
 */
int extractCommandStatus(storage *, unsigned char , char *);

/**
 * \fn commandSimulationStep(double)
 * \brief Advances the SUMO Simulation
 * \param Accepts number of steps or seconds SUMO should Simulate 
 */
void commandSimulationStep(double);

/**
 * \fn commandClose();
 * \brief Send termination command to SUMO
 */
void commandClose();

/**
 * \fn commandGetVehicleVariable(char *vehID, int varID)
 * \brief Gets the position position and speed info  
 * \param Accepts vehicle ID and variable ID, here variable ID is the comaand type
 */
void commandGetVehicleVariable(char *vehID, int varID);


/**
 * \fn get_num_sumo_nodes()
 * \brief Return the total number of nodes to be simulated in SUMO
 */
int commandGetMaxSUMONodesVariable();

/**
 * \fn init(int max_sim_time)
 * \brief Initialize SUMO and set the periodic subscriptions
 */
void init(int max_sim_time);

/**
 * \fn void processSubscriptions(String_list departed, String_list arrived)
 * \brief process the subscription values sent by SUMO at each SUMO time step
 */
void processSubscriptions(void);

/**
 * \fn void GetSpeed(NodePtr node, char * sumo_id);
 * \brief ask SUMO to return the speed for the indicated vehicle ID
 * \param NodePtr node the pointer to the OAISim node
 * \param char *sumo_id the SUMO ID of the target node
 */
void GetSpeed(NodePtr node, char * sumo_id);

/**
 * \fn void Position(NodePtr node, char * sumo_id);
 * \brief ask SUMO to return the position (X,Y) for the indicated vehicle ID
 * \param NodePtr node the pointer to the OAISim node
 * \param char *sumo_id the SUMO ID of the target node
 */
void GetPosition(NodePtr node, char * sumo_id);


#endif 

