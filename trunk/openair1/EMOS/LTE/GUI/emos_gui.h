/*******************************************************************************
    OpenAirInterface 
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is 
   included in this distribution in the file called "COPYING". If not, 
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@eurecom.fr
  
  Address      : Eurecom, Compus SophiaTech 450, route des chappes, 06451 Biot, France.

 *******************************************************************************/
#define TERM_MODE_SINGL 	0
#define TERM_MODE_MULTI 	1

#define TERM_ON 		1
#define TERM_OFF 		0

#define SCREEN_COLOR_ON		FL_CHARTREUSE	
#define SCREEN_COLOR_OFF	FL_RIGHT_BCOL
#define SCREEN_COLOR_BG		FL_BLACK
#define SCREEN_COLOR_HL		FL_RED
		
#define LED_COLOR_ON		FL_RED
#define LED_COLOR_OFF		FL_RIGHT_BCOL

#define REC_ON 			1
#define REC_OFF 		0

#define EMOS_NOT_READY		0
#define EMOS_READY		1

#define TIME_DOMAIN		0
#define FREQ_DOMAIN		1

#define N0			0
#define SNR			1

#define REC_FRAMES_MAX          (100*60*5)      //5min 
#define REC_FRAMES_PER_FILE     (100*(60*5-5))  //4min55sec
#define REC_FILE_IDX_MAX        100

#define CHANSOUNDER_FIFO_DEV "/dev/rtf3"

#define NO_ESTIMATES_DISK 100 //No. of estimates that are aquired before dumped to disk

//#define CHANNEL_BUFFER_SIZE sizeof(fifo_dump_emos_UE) //in bytes

#define SCREEN_MEMORY_SIZE 32

#define DISP_MAX_POWER 4194304 //maximum value for the display of the frequency response

//#define GPSD_BUFFER_SIZE 64

