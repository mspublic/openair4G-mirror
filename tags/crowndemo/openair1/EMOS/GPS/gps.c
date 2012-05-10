#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <gps.h>
#include <forms.h>

struct gps_data_t *gps_data = NULL;
struct gps_fix_t dummy_gps_data;
FILE *dumpfile_id = NULL; 


void gps_data_callback(int gps_fd, void* data)
{
  //char tmptxt[1024];
  //printf("GPS timer called\n");
  if (gps_data)
    {
      if (gps_poll(gps_data) != 0)
	{
	  //sprintf(tmptxt,"Error polling data from GPS, gps_data = %x", gps_data);
	  printf("Error polling data from GPS\n");
	}
      //else
      //printf("GPS poll called\n");
    }
  //fl_set_timer(ob, 0.05);
  
  //write GPS
  if (gps_data)
    {
      if (fwrite(&(gps_data->fix), sizeof(char), sizeof(struct gps_fix_t), dumpfile_id) != sizeof(struct gps_fix_t))
	{
	  printf("Error writing to dumpfile, stopping recording\n");
	}
    }
  else
    {
      printf("WARNING: No GPS data available, storing dummy packet\n");
      if (fwrite(&(dummy_gps_data), sizeof(char), sizeof(struct gps_fix_t), dumpfile_id) != sizeof(struct gps_fix_t))
	{
	  printf("Error writing to dumpfile, stopping recording\n");
	}
    } 
}

void stop_gps(int sig) {

  // stop gps
  if (gps_data) 
    fl_remove_io_callback(gps_data->gps_fd, FL_READ , &gps_data_callback);

  // close the GPS 
  if (gps_data) 
    gps_close(gps_data);

  fclose(dumpfile_id);
  dumpfile_id = NULL;

  exit(0);
}

int main(int argc, char *argv[]) {

  // open GPS
  gps_data = gps_open("127.0.0.1","2947");
  if (gps_data == NULL) 
    {
      printf("Could not open GPS\n");
      exit(-1);
    }
  else if (gps_query(gps_data, "w+x\n") != 0)
    {
      //sprintf(tmptxt,"Error sending command to GPS, gps_data = %x", gps_data);
      printf("Error sending command to GPS\n");
      exit(-1);
    }

  dumpfile_id = fopen("gps_trace","w");
  if (dumpfile_id == NULL)
    {
      printf("Error opening dumpfile");
      exit(-1);
    }

  signal(SIGINT, stop_gps); 

  if (gps_data)  
    fl_add_io_callback(gps_data->gps_fd, FL_READ, &gps_data_callback, NULL);

  while (1);

  exit(0);
}

