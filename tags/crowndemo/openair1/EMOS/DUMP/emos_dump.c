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

/*! \file emod_dump.c
* \brief reads data from a fifo and dumps it to disk
* \author F. Kaltenberger, Leonardo Cardoso
* \date 2012
* \version 0.2
* \company Eurecom
* \email: florian.kaltenberger@eurecom.fr
* \note
* \warning
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <time.h>


#include "SCHED/phy_procedures_emos.h"
#include "emos_dump.h"

int end=0;

static void endme(int dummy)
{
  printf("SIGINT received\n");
  end=1;
}

/*
#define TIMESTAMP_SIZE 8
#define PDU_ERRORS_SIZE 4
#define RX_RSSI_SIZE 2
#define COMPLEX16_SIZE 4
#define NO_OF_OFDM_CARRIERS 256
#define CHANNEL_BUFFER_SIZE (NB_ANTENNAS*COMPLEX16_SIZE*NO_OF_OFDM_CARRIERS+TIMESTAMP_SIZE+PDU_ERRORS_SIZE+RX_RSSI_SIZE) //in bytes
*/
#define NO_ESTIMATES_DISK 100 //No. of estimates that are aquired before dumped to disk

int main (int argc, char **argv)
{
  char c, eNB_flag=0;
  char *fifo2file_buffer, *fifo2file_ptr;

  int fifo, counter=0, bytes;

  FILE  *dumpfile_id;
  char  dumpfile_name[1024];
  time_t starttime_tmp;
  struct tm starttime;

  int channel_buffer_size;

  while ((c = getopt (argc, argv, "he")) != -1) {
    switch (c) {
    case 'e':
      eNB_flag=1;
      break;
    case 'h':
      printf("EMOS FIFO to file dump.\n");
      printf("Usage: %s -h(elp) -e(NB) (default=ue)\n",argv[0]);
      exit(0);
    default:
      break;
    }
  }

  if (eNB_flag==1)
    channel_buffer_size = sizeof(fifo_dump_emos_eNB);
  else
    channel_buffer_size = sizeof(fifo_dump_emos_UE);

  // allocate memory for NO_FRAMES_DISK channes estimations
  fifo2file_buffer = malloc(NO_ESTIMATES_DISK*channel_buffer_size);
  fifo2file_ptr = fifo2file_buffer;

  if (fifo2file_buffer == NULL)
    {
      printf("Cound not allocate memory for fifo2file_buffer\n");
      exit(EXIT_FAILURE);
    }

  if ((fifo = open(CHANSOUNDER_FIFO_DEV, O_RDONLY)) < 0)
    {
      fprintf(stderr, "Error opening the fifo\n");
      exit(EXIT_FAILURE);
    }


  time(&starttime_tmp);
  localtime_r(&starttime_tmp,&starttime);
  snprintf(dumpfile_name,1024,"/tmp/data_%d%0.2d%0.2d_%0.2d%0.2d%0.2d.EMOS",1900+starttime.tm_year, starttime.tm_mon+1, starttime.tm_mday, starttime.tm_hour, starttime.tm_min, starttime.tm_sec);

  dumpfile_id = fopen(dumpfile_name,"w");
  if (dumpfile_id == NULL)
    {
      fprintf(stderr, "Error opening dumpfile\n");
      exit(EXIT_FAILURE);
    }

  signal(SIGINT, endme);


  while (!end)
    {
      bytes = rtf_read_all_at_once(fifo, fifo2file_ptr, channel_buffer_size);
      if (eNB_flag==1)
	printf("eNB: count %d, frame %d, read: %d bytes from the fifo\n",counter, ((fifo_dump_emos_eNB*)fifo2file_ptr)->frame_tx,bytes);
      else
	printf("UE: count %d, frame %d, read: %d bytes from the fifo\n",counter, ((fifo_dump_emos_UE*)fifo2file_ptr)->frame_rx,bytes);

      fifo2file_ptr += channel_buffer_size;
      counter ++;

      if (counter == NO_ESTIMATES_DISK)
        {
          //reset stuff
          fifo2file_ptr = fifo2file_buffer;
          counter = 0;

          //flush buffer to disk
          printf("flushing buffer to disk\n");

          if (fwrite(fifo2file_buffer, sizeof(char), NO_ESTIMATES_DISK*channel_buffer_size, dumpfile_id) != NO_ESTIMATES_DISK*channel_buffer_size)
            {
              fprintf(stderr, "Error writing to dumpfile\n");
              exit(EXIT_FAILURE);
            }
        }
    }

  free(fifo2file_buffer);
  fclose(dumpfile_id);

  return 0;

}

