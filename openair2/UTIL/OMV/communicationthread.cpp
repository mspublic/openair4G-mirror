/*******************************************************************************

  Eurecom OpenAirInterface 2
  Copyright(c) 1999 - 2010 Eurecom

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


/*! \file communicationthread.cpp
* \brief this thread is to process the communication between the simulator and the visualisor
* \author M. Mosli
* \date 2012
* \version 0.1 
* \company Eurecom
* \email: mosli@eurecom.fr
*/ 

#include "communicationthread.h"

typedef struct {
    char *name;	/*!< \brief string name of item */
    int value;	/*!< \brief integer value of mapping */
} mapping;

mapping omg_model_names[] = {
  {"STATIC", 0},
  {"RWP", 1}, 
  {"RWALK", 2},
  {"TRACE", 3}, 
  {"SUMO", 4}, 
  {NULL, -1}
};

/* map an int to a string. Takes a mapping array and a value */
char *map_int_to_str(mapping *map, int val) {
    while (1) {
        if (map->name == NULL) {
            return NULL;
        }
        if (map->value == val) {
            return map->name;
        }
        map++;
    }
}
// mobtype = map_int_to_str(omg_model_names,omv_data.geo[i].mobility_type)

extern int pfd[2];
extern struct Geo geo[NUMBER_OF_eNB_MAX+NUMBER_OF_UE_MAX];
extern int nb_frames;
extern int node_number;
extern int nb_enb;
extern int nb_ue;

CommunicationThread::CommunicationThread(MyWindow* window){
    this->window = window;
    QObject::connect(this, SIGNAL(newData(QString)), window, SLOT(writeToConsole(QString)));
    QObject::connect(this, SIGNAL(newPosition()), window->getGL(), SLOT(drawNewPosition()));
    QObject::connect(this, SIGNAL(endOfTheSimulation()), window, SLOT(endOfTheSimulation()));
}

void CommunicationThread::run()
{
    int nread;
    int end = 0;
    int counter = nb_frames;
    Data_Flow_Unit data;

    while (counter > 0 && !end){
        switch( nread = read(pfd[0], &data, sizeof(Data_Flow_Unit))) {
        case -1 :
	    if (counter < nb_frames)
		end = 1;
            perror( "read" );
            break;
        case 0 :
	    if (counter < nb_frames)
		end = 1;
            perror( "EOF" );
            break;
        default :
            QString information, information_for_a_node;
	    geo[nb_enb].mobility_type = data.geo[nb_enb].mobility_type;
	    information.sprintf("Frame: %d\nUE nodes mobility type: %s\n",nb_frames - counter, 
			map_int_to_str(omg_model_names, geo[nb_enb].mobility_type));   
         
	    end = data.end;
            
            for (int i = 0; i<nb_enb; i++){

                geo[i].Neighbors = data.geo[i].Neighbors;
                geo[i].x = data.geo[i].x;
		geo[i].y = data.geo[i].y;
		geo[i].node_type = data.geo[i].node_type;

		information_for_a_node.sprintf("    eNb %d: Position (%d,%d)\n" , i, data.geo[i].x, data.geo[i].y);
		information += information_for_a_node;		

                for(int j = 0; j<data.geo[i].Neighbors; j++)
                    geo[i].Neighbor[j] = data.geo[i].Neighbor[j];
            }

	    for (int i = nb_enb; i<node_number; i++){

                geo[i].Neighbors = data.geo[i].Neighbors;
                geo[i].x = data.geo[i].x;
		geo[i].y = data.geo[i].y;
		geo[i].node_type = data.geo[i].node_type;

		information_for_a_node.sprintf("    UE %d: Position (%d,%d)\n" , i, data.geo[i].x, data.geo[i].y);
		information += information_for_a_node;		

                for(int j = 0; j<data.geo[i].Neighbors; j++)
                    geo[i].Neighbor[j] = data.geo[i].Neighbor[j];
            }

            emit newData(information);
            emit newPosition();
            break;
        }
        counter--;
    }

    sleep(10);
    emit endOfTheSimulation();

    

/*
    switch(nread = read(pfd[0], &final, sizeof(Final_Data))){
        case -1 :
            perror( "read" );
            break;
        case 0 :
            perror( "EOF" );
            break;
        default :
            QString final_data;
            final_data.sprintf("\n*** Final results ***\n\n* Node number: %d\n* Loss rate: %f\n* All forwarded packets: %d\n* Used memory [device, host]: [%d, %d]\n* Elapsed time (average): %f\n* Elapsed time (min): %f\n* Elapsed time (max): %f\n",final.node_number, final.loss, final.forwarded,final.device_memory, final.host_memory,final.average_time, final.min_time, final.max_time);
            emit newData(final_data);
            emit endOfTheSimulation();
            break;
    }
*/
}
