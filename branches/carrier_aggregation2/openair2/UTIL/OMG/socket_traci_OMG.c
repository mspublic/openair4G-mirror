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

/*! \file socket_traci_OMG.c
* \brief The socket interface of TraCI to connect OAI to SUMO. A 'C' reimplementation of the TraCI version of simITS (F. Hrizi, fatma.hrizi@eurecom.fr)
* \author  S. Uppoor
* \date 2012
* \version 0.1
* \company INRIA
* \email: sandesh.uppor@inria.fr
* \note
* \warning
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "socket_traci_OMG.h"
#include "storage_traci_OMG.h"


int connection_(char *hoststr,int portno){
        host = gethostbyname(hoststr);
	
	
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
             #ifdef STANDALONE 
   		 printf(" Socket Error\n");
            #else
                 LOG_D(OMG, " Socket Error\n");
 	     #endif	
        }
         
	server_addr.sin_family = AF_INET;     
        server_addr.sin_port = htons(portno);   
        server_addr.sin_addr = *((struct in_addr *)host->h_addr);
        bzero(&(server_addr.sin_zero),8); 

        if (connect(sock, (struct sockaddr *)&server_addr,
                    sizeof(struct sockaddr)) < 0) 
        {
              #ifdef STANDALONE 
   		  printf("Connection Failed\n"); 
              #else
                 LOG_E(OMG, " Connection Error\n");
 	     #endif
              
            return -1;
            
        }  
	       
        return 1;
}


void sendExact(int cmdLength){
	
        msgLength = cmdLength + 4;
        writeInt(msgLength);

        rearange();
        unsigned char *buf = (unsigned char *)malloc(sizeof(unsigned char) * (msgLength));
        storage *cur_ptr = storageStart;
        size_t i = 0;
        size_t numbytes = msgLength;
        while (cur_ptr->next != NULL){
	buf[i]= cur_ptr->item;
        cur_ptr = cur_ptr->next;
        i++;
        }
	buf[i]= cur_ptr->item;

        while (numbytes > 0){
                int n = send(sock, buf, (int)numbytes, 0);//<----- need to check
		if (numbytes == n)
			break;
		
                if (n<0) {
                    #ifdef STANDALONE 
   		       printf(" ERROR writing to socket\n");
                    #else
                        LOG_E(OMG, " ERROR writing to socket\n");
 	            #endif	
                        
                }
		
                numbytes -= n;
                buf +=n;
        } 
        freeStorage(storageStart);
}



storage * receiveExact(){

        unsigned char* bufLength = (unsigned char *)malloc(sizeof(unsigned char) * (4));
	int bytesRead = 0;
	int readThisTime = 0;
	
        //Get the length of the entire message by reading the 1st field	
	while (bytesRead<4)
	{
	        readThisTime = recv( sock, (char*)(bufLength + bytesRead), 4-bytesRead, 0 );

		if( readThisTime <= 0 ) {
                        #ifdef STANDALONE 
   		       printf(" tcpip::Socket::receive() @ recv\n");
                    #else
                        LOG_E(OMG, " tcpip::Socket::receive() @ recv\n");
 	            #endif
		}

		bytesRead += readThisTime;
        }
        
        // create storage to access the content
        tracker = writePacket(bufLength, 4);
        
        // store pointer to free the space later
        storage *freeTracker = tracker;   
        int s= readInt();
	int NN = s - 4;
        printf("debug \n");
        printf("value of s is %d \n",s);
        printf("end debug \n");
        //Free space after use
        //freeStorage(freeTracker); // JHNOte: will be done by calling reset() in storage_traci_omg

        int mySize = 0;
        mySize = sizeof(unsigned char) * (NN);
        printf("debug \n");
        printf("value of mySize is %d \n",mySize);
        printf("end debug \n");
	// receive actual message content 
	//unsigned char* buf = (unsigned char *)malloc(sizeof(unsigned char) * (NN));
	unsigned char* buf = (unsigned char *)malloc(mySize);
	
        bytesRead = 0;
	readThisTime = 0;
	
	while (bytesRead<NN)
	{
		readThisTime = recv( sock, (char*)(buf + bytesRead), NN-bytesRead, 0 );
		
                if( readThisTime <= 0 ) {
                    #ifdef STANDALONE 
   		       printf(" tcpip::Socket::receive() @ recv\n");
                    #else
                       LOG_E(OMG, " tcpip::Socket::receive() @ recv\n");
 	            #endif
                }

		bytesRead += readThisTime;
	}
	
	return writePacket(buf, NN);
        
}


void close_connection(){
        #ifdef STANDALONE 
   		  printf("closing the socket \n");
        #else
                 LOG_E(OMG, "closing the socket \n");
        #endif
        close(sock);
}

