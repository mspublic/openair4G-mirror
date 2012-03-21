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

/*! \file otg_rx.c
* \brief function containing the OTG traffic generation functions 
* \author A. Hafsaoui
* \date 2011
* \version 0.1
* \company Eurecom
* \email: 
* \note
* \warning
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "otg_defs.h"
//#include "../../../openair1/PHY/CODING/defs.h" // for CRC computing 
#include "otg_vars.h"

int check_packet(char *packet){
	
	int status_ok=0;

//No need to check the CRC
/*
	char *pch;
	char *pch2;
	char *tmp_pkts;
	char *crc_string;
	char *crc_rx_string;
	unsigned int crc_rx=0;
	
	int pos;


// CRC RX computation 

	pch=strstr(packet, END_OTG_HEADER);
	tmp_pkts=(char*)malloc((strlen(pch))*sizeof(char*));
	sprintf(tmp_pkts,"%s",(pch+strlen(END_OTG_HEADER)));
	//printf ("STRING OBTAINED=%s\n",tmp_pkts);

	crc_rx= crc_gen(tmp_pkts, CRC_FLAG);
	printf("CRC RX=%u\n", crc_rx);

	crc_rx_string=(char*)malloc(20*sizeof(char*));
	sprintf(crc_rx_string,"%u",crc_rx);

// CRC CX regeneration

	pch2=strstr(packet, OTG_CTRL_FLAG);

	//printf ("found at %d\n",pch2-packet+1);
	crc_string=(char*)malloc((pch2-packet)*sizeof(char*));
	strncpy(crc_string, packet, pch2-packet);

	printf("CRC TX=%s\n",crc_string); //(int)atol(crc_string)
	printf("CRC size %d \n", strlen(crc_string));

	// COMPARE CRC
 
	// overflow condition occuring with crc values higher than 999999999. atoi function returns 2147483647

	 if ((strcmp(crc_string,crc_rx_string)==0)){ //
		printf("CRC check OK!\n");
		 status_ok=1;
	}
	else 
		printf("CRC error!\n");
	

	if (NULL != tmp_pkts){
		tmp_pkts=NULL;  					
		free(tmp_pkts);
	}

	if (NULL != pch){
		pch=NULL;  					
		free(pch);
	}
	if (NULL != pch2){
		pch2=NULL;  					
		free(pch2);
	}
	if (NULL != crc_rx_string){
		crc_rx_string=NULL;  					
		free(crc_rx_string);
	}

*/
	// Free the received packet

	if (NULL != packet){

	printf ("Received packet=%s\n",packet);
	status_ok=1;
		packet=NULL;  					
		free(packet);
	}

	printf("RX Free packet\n");

	return(status_ok);

}




