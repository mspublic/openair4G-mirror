
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>


int buffer_tx(const char *fname,void *data,int length,int dec) {

  FILE *fp;
  int i;

  fp = fopen(fname,"w+");

   if (fp== NULL) {
     printf("[OPENAIR][FILE OUTPUT] Cannot open file_buffer_tx %s\n",fname);
     return(-1);
   }

          for (i=0;i<length<<1;i+=2*dec) {
    	fprintf(fp,"%g ",((double *)data)[i]);
    	fprintf(fp,"%g ",((double *)data)[i+1]);
          }

  fclose(fp);
  return(0);
}


int buffer_rx(char *fname,double *data,int length,int dec)
{
	  FILE *fp;
	  int i;

	    fp = fopen(fname,"r");

	     if (fp== NULL) {
	       printf("[OPENAIR][FILE OUTPUT] Cannot open file_buffer_rx %s\n",fname);
	       return(-1);
	     }

	      for (i=0;i<length<<1;i+=2*dec) {
	     fscanf(fp,"%lg ",&((double *)data)[i]);
	     fscanf(fp,"%lg ",&((double *)data)[i+1]);

	      }
	      fclose(fp);
	      return 0;

}

void buffer_tx_mmap(char *fname,double *data,int length,int dec)
{
    int i;
    int fd;
    int result;
    double *map;
    unsigned int FILESIZE;

    FILESIZE=length*(sizeof(double));
       fd = open(fname, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
       if (fd == -1) {
   	perror("Error opening file for writing");
   	exit(EXIT_FAILURE);
       }

       result = lseek(fd, FILESIZE-1, SEEK_SET);
       if (result == -1) {
   	close(fd);
   	perror("Error calling lseek() to 'stretch' the file");
   	exit(EXIT_FAILURE);
       }

       result = write(fd, "", 1);
       if (result != 1) {
   	close(fd);
   	perror("Error writing last byte of the file");
   	exit(EXIT_FAILURE);
       }

       map = mmap(0, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
       if (map == MAP_FAILED) {
   	close(fd);
   	perror("Error mmapping the file");
   	exit(EXIT_FAILURE);
       }

       for (i = 0; i <=length; ++i) {
   	map[i] = data[i];
       }

       if (munmap(map, FILESIZE) == -1) {
   	perror("Error un-mmapping the file");
       }


       close(fd);
}

void buffer_rx_mmap(char *fname,double *data,int length,int dec){
    int i;
    int fd;
    int result;
    double *map;
    unsigned int FILESIZE;

    FILESIZE=length*(sizeof(double));

    fd = open(fname, O_RDONLY);
       if (fd == -1) {
   	perror("Error opening file for reading");
   	exit(EXIT_FAILURE);
       }

       map = mmap(0, FILESIZE, PROT_READ, MAP_SHARED, fd, 0);
       if (map == MAP_FAILED) {
   	close(fd);
   	perror("Error mmapping the file");
   	exit(EXIT_FAILURE);
       }



       for (i = 0; i <=length; ++i) {
    	   data[i] = map[i];
        }


       if (munmap(map, FILESIZE) == -1) {
   	perror("Error un-mmapping the file");
       }
       close(fd);
}

