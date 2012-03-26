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

/*! \file client_traci_OMG.c
* \brief The OMG TraCI to send/receive commands from/to  SUMO via socket interfaces.
* \author  S. Uppoor, J. Harri
* \date 2012
* \version 0.1
* \company INRIA, Eurecom
* \email: sandesh.uppor@inria.fr, haerri@eurecom.fr
* \note
* \warning
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#include "client_traci_OMG.h"
#include "TraCIConstants.h"

int handshake(char *hoststr,int portno){
    
   check_endianness(); // check endianness
   int i;

   for(i = 0; i< 5; i++) {
        if ( connection_(hoststr,portno) <0 ) {
            printf("connection error...trying again in 3 seconds\n");
            //LOG_E(OMG, " Could not connect to TraCIServer - sleeping before trying again...\n");
            sleep(3);
        }
        else {
  	printf(" SUMO now connected to OMG on host address\n");
              //LOG_N(OMG, " SUMO now connected to OMG on host address %c and port %n .\n", hoststr, portno);
 	      return 0;  
	}
   }
   printf(" SUMO unreachable...giving up...\n"); 
   //LOG_E(OMG, " SUMO unreachable...giving up...\n");
    return -1;
}

void init(int max_sim_time) {
  
  //LOG_N(OMG, " Initializing TraCI...\n");
  printf(" Initializing TraCI...\n");
  char *objID = " ";
 // size_t size = strlen(objID);

  int noSubscribedVars = 2;
  writeUnsignedByte(0);
  writeInt(1 + 4 + 1 + 4 + 4 + 4 + (int) strlen(objID) + 1 + noSubscribedVars);
  writeUnsignedByte(CMD_SUBSCRIBE_SIM_VARIABLE); // command id
  writeInt(0); // begin time
  writeInt(max_sim_time*1000); // end time
  writeString(objID); // object id
  writeUnsignedByte(noSubscribedVars); // variable number
  writeUnsignedByte(VAR_DEPARTED_VEHICLES_IDS);
  writeUnsignedByte(VAR_ARRIVED_VEHICLES_IDS);

  // send request message
  sendExact(storageLength(storageStart));
  extractCommandStatus(receiveExact(), CMD_SUBSCRIBE_SIM_VARIABLE, description);
  if (departed == NULL) 
    departed = (String_list)malloc(sizeof(String_list)); // departed MUST point to HEAD

  if (arrived == NULL) 
    arrived = (String_list)malloc(sizeof(String_list));  // arrived MUST point to HEAD

  processSubscriptions();

  reset();
}

void processSubscriptions() {
   int noSubscriptions = readInt();
   printf(" number subscription: %d....\n",noSubscriptions);
   String_list tmp_departed = departed;
   String_list tmp_arrived = arrived;
   int s;
   for (s = 0; s<noSubscriptions; ++s) {
      printf(" processing Subscriptions....\n");
      int respStart = readInt();
      //int extLength = readUnsignedByte();
      int respLength = readUnsignedByte();
      if (respLength == 0)
         respLength = readInt();

      //int respLength = readUnsignedByte();
      int cmdId = readUnsignedByte();
      printf(" subscription ID: %d....\n",cmdId);
      if (cmdId<0xe0||cmdId>0xef) {  // only responses to subscription to supported types (vehicles, TLC, polygones...) are accepted
         //LOG_W(OMG, " Invalide Subscription response: %d\n",cmdId);
         printf(" Invalide Subscription response: %d\n",cmdId);
         return;
      }
      char *objID = readString();
      int varNo = readUnsignedByte();
      printf(" number of variables: %d....\n",varNo);
      int i;
      for (i=0; i<varNo; ++i) {
          int varID = readUnsignedByte();
           printf(" variable ID is: %d....\n",varID);
          bool ok = readUnsignedByte()==RTYPE_OK;
          int valueDataType = readUnsignedByte();
          if (ok&&cmdId==CMD_SUBSCRIBE_SIM_VARIABLE+0x10&&varID==VAR_DEPARTED_VEHICLES_IDS) {
               printf(" cars departed....\n");
               tmp_departed = readStringList(tmp_departed);
               continue;
           }
           if (ok&&cmdId==CMD_SUBSCRIBE_SIM_VARIABLE+0x10&&varID==VAR_ARRIVED_VEHICLES_IDS) {
               printf(" cars arrived....\n");
               tmp_arrived = readStringList(tmp_arrived);
               continue;
           }
       }
    }
}

int extractCommandStatus(storage *s, unsigned char commandId, char * description)
{
	 int success=0;

         if(s == NULL) {
           //LOG_E(OMG, " client_traci_OMG::extractCommandStatus():  Tracker is NULL \n");
           printf(" client_traci_OMG::extractCommandStatus():  Tracker is NULL \n");
           return success=0;
	}

	// validate the the message response from SUMO
	//int storageLength_ = storageLength(s);

	// tracker currently points to the begining of the recieved data in the linked list        
        tracker = s;
        storage *freeTracker = tracker;   // save it for calling free

	int commandLength = readUnsignedByte();
        
	// CommandID needs to fit
        int rcvdCommandId = readUnsignedByte();
	printf("received command %d",rcvdCommandId);
	//if (rcvdCommandId = ((int)readUnsignedByte() != (int)commandId))
        if (rcvdCommandId != (int)commandId)
	{
                printf("%d",rcvdCommandId);
                //LOG_E(OMG, " Server answered to command\n");
		printf(" Server answered to command\n");
	}
	printf(" Server answered to command with command %d , was expecting command %d \n", rcvdCommandId, commandId);
	// Get result and description
	unsigned char result = readUnsignedByte();
	if (result != RTYPE_OK)
	{       
                printf(" Server returned error\n");
                //error(" Server returned error ");
		return success=0;
		
	}
	
	if (result == RTYPE_OK) {
		 printf(" Server returned success\n");
                //printf ("Success");
		success=1;
        }

       	description = readString();
	// print description if needed 

	
        //free actual message content
	//depends on the message which is handled

     /*   if (commandId != CMD_GET_VEHICLE_VARIABLE)
			freeStorage(freeTracker);*/

	return success;
}


void commandSimulationStep(double time)
{	// progress the simulation in SUMO
	// reset is used to initalize the global parameters
	reset();
	// Send command
	//writeUnsignedByte(0x06);
        writeUnsignedByte(1 + 1 + 4); // command length
	writeUnsignedByte(CMD_SIMSTEP2); // look up TraCIConstants.h
        writeInt((int)(time*1000)); // TraCI accepts time in milli seconds
	sendExact(storageLength(storageStart));

        extractCommandStatus(receiveExact(), CMD_SIMSTEP2, description);
	
         if (departed == NULL) 
   		departed = (String_list)malloc(sizeof(String_list));  // departed MUST point to HEAD

  	if (arrived == NULL) 
    		arrived = (String_list)malloc(sizeof(String_list));  // departed MUST point to HEAD
        
	printf("updated sumo and gotten new cars \n");
        processSubscriptions();

}  

void commandClose()
{	reset();
	// command length
    	writeUnsignedByte(0x02);
    	// command id
    	writeUnsignedByte(CMD_CLOSE);
	  	
	// send request message
        sendExact(storageLength(storageStart));
        extractCommandStatus(receiveExact(), CMD_CLOSE, description);

}


void commandGetVehicleVariable(char *vehID, int varID)// malloc for vehID and varID depends on speed or position
{	
	reset();
    	int domID = CMD_GET_VEHICLE_VARIABLE;//0xa4 specific for get vehicle variable command
	
   	// command length
    	writeUnsignedByte(1 + 1 + 1 + 4 + (int)strlen(vehID));
    	// command id
    	writeUnsignedByte(CMD_GET_VEHICLE_VARIABLE);
    	// variable id
    	writeUnsignedByte(varID);
    	// object id
    	writeString(vehID);


    	// send request message
    	sendExact(storageLength(storageStart));
    	// receive answer message
        //receiveExact();
    	if (extractCommandStatus(receiveExact(), CMD_GET_VEHICLE_VARIABLE, description)){//<---RESPONSE_GET_VEHICLE_VARIABLE
	
    	// validate result state
        if(tracker == NULL) {
            // LOG_E(OMG, " client_traci_OMG::commandGetVehicleVariable():  Tracker is NULL \n");
            printf(" client_traci_OMG::commandGetVehicleVariable():  Tracker is NULL \n");
            /*vehicle->x = -1.0;
	    vehicle->y = -1.0;
	    vehicle->speed = -1.0;*/
            return;
	}

	//int res = readUnsignedByte();
        int length = readUnsignedByte();
        if(length ==0)
	  length = readInt();
       	int cmdId =readUnsignedByte();
        
        printf(" CMD is %d\n",cmdId);

        if (cmdId != (CMD_GET_VEHICLE_VARIABLE+0x10)) {
		//LOG_E(OMG, " Wrong response recieved\n");
                 printf(" Wrong response recieved\n");
            	return;
        }
        int VariableID = readUnsignedByte();
	char* rs = readString();

        int valueDataType = readUnsignedByte();
        if (valueDataType == POSITION_2D) {
	  printf("expecting POSITION INFO\n");
        }
        else if (valueDataType == TYPE_DOUBLE) {
          printf("expecting DOUBLE INFO\n");
        }
        else {
          printf("expecting OTHER INFO\n");
        }
        	//printf( " Double value: %f",doublev);
        //readAndReportTypeDependent(inMsg, valueDataType);
    
	/*if (valueDataType == TYPE_DOUBLE) {
        	double doublev = readDouble();
        	//printf( " Double value: %f",doublev);
    	} else if (valueDataType == POSITION_2D) {
        	vehicle->x = (double) readFloat();
        	vehicle->y = (double) readFloat();
		//float xv =readFloat();
		//float yv =readFloat();
       	 	//printf( " position value: %f %f\n",xv,yv);
    	} else if (valueDataType == TYPE_FLOAT) {
        	vehicle->speed = (double)readFloat();
		//float floatv=readFloat();
        	//printf(" float value: %f\n ",floatv);
	}
      	  else {
           // LOG_W(OMG, " No Matching Data Type Value\n"); 
            printf( " No Matching Data Type Value\n"); 
          }
	}    
	else 
		{	vehicle = NULL;
			return; }*/
    }
}

int commandGetMaxSUMONodesVariable()
{	
	reset();

        int max_car = 0;

   	// command length
    	writeUnsignedByte(1 + 1 + 1 + 1 + 4 + 1 + 1 + 4);
        // command id
        writeUnsignedByte(CMD_SCENARIO);
        // flag
	writeUnsignedByte(0x00); // GET command for the generic environment-related values
    	// command id
    	//writeUnsignedByte(CMD_SCENARIO);
        // domain id
	writeUnsignedByte(0x01); // vehicle
	
         writeInt(0); // first vehicular domain
    	
        // variable id
    	writeUnsignedByte(DOMVAR_MAXCOUNT); // get maximum number of vehicles

	writeUnsignedByte(TYPE_INTEGER); // get maximum number of vehicles
	writeInt(max_car); // get maximum number of vehicles
    
    	// send request message
    	sendExact(storageLength(storageStart));

    	// receive answer message
    	if (extractCommandStatus(receiveExact(), CMD_SCENARIO, description)){//<---RESPONSE_GET_VEHICLE_VARIABLE
	
    	  // validate result state
          
          if(tracker == NULL) {
            LOG_E(OMG, " client_traci_OMG::commandGetMaxSUMONodesVariable():  Tracker is NULL \n");
            return -1;
	  }
	  int res = readUnsignedByte();
	  int Length = readUnsignedByte(); // to check with Int
       	  int cmdId =readUnsignedByte();
          if (cmdId != (CMD_SCENARIO)) {
		LOG_E(OMG, " Wrong response recieved \n");
            	return;
          }

 	  int flag = readUnsignedByte(); 
 	  int dom = readUnsignedByte(); // domain
          int domID = readInt(); // domain ID
          int VariableID = readUnsignedByte();

          int valueDataType = readUnsignedByte();
    
	  if (valueDataType == TYPE_INTEGER) {
        	max_car = readInt();
		LOG_N(OMG, " max Number SUMO nodes is: %f \n", max_car);
    	  } 
      	  else LOG_W(OMG, " No Matching Data Type Value \n"); 
	}   

    return max_car;
}




void GetSpeed(NodePtr node, char * sumo_id)
{
    commandGetVehicleVariable(sumo_id, VAR_SPEED);
    double speed_double = readDouble();
    node->mob->speed = speed_double;
}

void GetPosition(NodePtr node, char * sumo_id)
{    
    commandGetVehicleVariable(sumo_id, VAR_POSITION);
    double x_double = readDouble();
    double y_double = readDouble();
    printf("Node %d has position X: %f and Y: %f \n",node->ID, x_double, y_double);

    node->X_pos = x_double;
    node->Y_pos = y_double;
}

/*vehicleVar* get_pos_speed(int i)
{
int r,q;
char q1,r1,*n1,*n2,*n3,*n4;

if (i<10){
	n1 = (char *)malloc(sizeof(char) * (2));
	n2=n1;	
	*n1++ = (char)(((int)'0')+i);
	*n1++ = '\0';
	vehicleVar *temp_ = (vehicleVar *)malloc(sizeof(vehicleVar));
	vehicle =temp_;
	commandGetVehicleVariable(n2,0x42);
	commandGetVehicleVariable(n2,0x40);

}
else if(i>=10 && i<100) {  
		
	n3 = (char *)malloc(sizeof(char) * (3));
	n4=n3;
	q = i/10;
	q1 = (char)(((int)'0')+q);
	*(n3++) = q1;
	r = i%10;
	r1 =(char)(((int)'0')+r);
	*(n3++) = r1;
	*(n3++) ='\0';
	vehicleVar *temp_ = (vehicleVar *)malloc(sizeof(vehicleVar));
	vehicle =temp_;
	commandGetVehicleVariable(n4,0x42);
	commandGetVehicleVariable(n4,0x40);

	}
else{
	//LOG_N(OMG, " Help me in get_pos_speed \n");
        printf(" Help me in get_pos_speed \n");
	}

return vehicle;

}*/

/*int main()
	
{       
    vehicleVar *trial;
    //printf("INIT done...\n");
    handshake("localhost",8883);
    //printf("connection done...\n");
    commandSimulationStep(1000);
    //commandSimulationStep(6);
    //commandSimulationStep(10);
    //commandSimulationStep(15);
    //commandGetVehicleVariable("flow0_0",0x40);
    trial = get_pos_speed(0);
    if (trial !=NULL){
    	printf("vehicle x : %f\n",trial->x);
    	printf("vehicle y : %f\n",trial->y);
    	printf("vehicle speed : %f\n",trial->speed);}
    else
	LOG_W(OMG, " ***** Vehicle info not available ***** \n");

    commandClose();
    //commandGetVehicleVariable(vehID);
    //while(1){}
    //commandSimulationStep();
    //close_connection();
    return 0;
} */
