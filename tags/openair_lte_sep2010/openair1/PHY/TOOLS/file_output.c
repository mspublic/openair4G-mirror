#ifdef USER_MODE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

    case 5: // complex 8-bit
      for (i=0;i<length<<1;i+=(2*dec)) {
	fprintf(fp,"%d + j*(%d)\n",((char *)data)[i],((char *)data)[i+1]);
      }
      break;

    case 6:  // real 64-bit
      for (i=0;i<length;i+=dec) {
	fprintf(fp,"%lld\n",((long long*)data)[i]);
      }
      break;

    case 7: // real double
      for (i=0;i<length;i+=dec) {
	fprintf(fp,"%g\n",((double *)data)[i]);
      }
      break;

    case 8: // complex double
      for (i=0;i<length<<1;i+=2*dec) {
	fprintf(fp,"%g + j*(%g)\n",((double *)data)[i], ((double *)data)[i+1]);
      }
      break;
       
    }



  fprintf(fp,"];\n");
  fclose(fp);
  return(0);
}

#endif // USER_MODE
