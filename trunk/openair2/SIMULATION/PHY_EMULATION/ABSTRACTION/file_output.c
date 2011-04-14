#ifdef USER_MODE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Write output file from signal data
// fname  = output file name
// vname  = output vector name (for MATLAB/OCTAVE)
// data   = point to data 
// length = length of data vector to output
// dec    = decimation level
// format = data format (0 = real 16-bit, 1 = complex 16-bit,2 real 32-bit, 3 complex 32-bit,4 = real 8-bit)

int write_output(const char *fname,const char *vname,void *data,int length,int dec,char format) {

  FILE *fp;
  int i;

  printf("Writing %d elements of type %d to %s\n",length,format,fname);
  fp = fopen(fname,"w+");

  if (fp== NULL) {
    printf("[OPENAIR][FILE OUTPUT] Cannot open file %s\n",fname);
    return(-1);
  }

  fprintf(fp,"%s = [",vname);
  
  switch (format) 
    {
    case 0:   // real 16-bit
      
      for (i=0;i<length;i+=dec) {
	fprintf(fp,"%d\n",((short *)data)[i]);
      }
      break;
      
    case 1:  // complex 16-bit
      
      for (i=0;i<length<<1;i+=(2*dec)) {
	fprintf(fp,"%d + j*(%d)\n",((short *)data)[i],((short *)data)[i+1]);
	
	//	  printf("%d + j*(%d)\n",((short *)data)[i],((short *)data)[i+1]);
      }
      break;
      
    case 2:  // real 32-bit
      for (i=0;i<length;i+=dec) {
	fprintf(fp,"%d\n",((int *)data)[i]);
      }
      break;
      
    case 3: // complex 32-bit
      for (i=0;i<length<<1;i+=(2*dec)) {
	fprintf(fp,"%d + j*(%d)\n",((int *)data)[i],((int *)data)[i+1]);
      }
      break;

case 4: // real 8-bit
      for (i=0;i<length;i+=dec) {
	   fprintf(fp,"%d\n",((char *)data)[i]);
      }
      break;
      
    }



  fprintf(fp,"];\n");
  fclose(fp);
}

#endif USER_MODE
