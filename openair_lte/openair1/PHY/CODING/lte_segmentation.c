#include "PHY/defs.h"

//#define DEBUG_SEGMENTATION
void lte_segmentation(unsigned char *input_buffer,
		      unsigned char **output_buffers,
		      unsigned int B,
		      unsigned int *C,
		      unsigned int *Cplus,
		      unsigned int *Cminus,
		      unsigned int *Kplus,
		      unsigned int *Kminus,
		      unsigned int *F) {

  unsigned int L,Bprime,Bprime_by_C,r,Kr,k,s,crc;

  if (B<=6144) {
    L=0;
    *C=1;
    Bprime=B;
  }
  else {
    L=24;
    *C = B/(6144-L);
    if ((6144-L)*(*C) < B)
      *C=*C+1;
    Bprime = B+((*C)*L); 
#ifdef DEBUG_SEGMENTATION
    printf("Bprime %d\n",Bprime);
#endif
  }

  // Find K+
  Bprime_by_C  = Bprime/(*C);
#ifdef DEBUG_SEGMENTATION
  printf("Bprime_by_C %d\n",Bprime_by_C);
#endif
    //  Bprime = Bprime_by_C>>3;

  if (Bprime_by_C <= 40) {
    *Kplus = 40;
    *Kminus = 0;
  }
  else if (Bprime_by_C<=512) { // increase by 1 byte til here
    *Kplus = (Bprime_by_C>>3)<<3;
    *Kminus = Bprime_by_C-8;
  }
  else if (Bprime_by_C <=1024) {// increase by 2 bytes til here
    *Kplus = (Bprime_by_C>>4)<<4;
    if (*Kplus < Bprime_by_C)
      *Kplus = *Kplus + 16;
    *Kminus = (*Kplus - 16);
  }
  else if (Bprime_by_C <= 2048) { // increase by 4 bytes til here
    *Kplus = (Bprime_by_C>>5)<<5;
    if (*Kplus < Bprime_by_C)
      *Kplus = *Kplus + 32;
    *Kminus = (*Kplus - 32);
  }
  else if (Bprime_by_C <=6144 ) { // increase by 8 bytes til here

   *Kplus = (Bprime_by_C>>6)<<6;
#ifdef DEBUG_SEGMENTATION
    printf("Bprime_by_C_by_C %d , Kplus %d\n",Bprime_by_C,*Kplus);
#endif 
    if (*Kplus < Bprime_by_C)
      *Kplus = *Kplus + 64;
#ifdef DEBUG_SEGMENTATION
    printf("Bprime_by_C_by_C %d , Kplus2 %d\n",Bprime_by_C,*Kplus);
#endif
    *Kminus = (*Kplus - 64);
  }
  else {
    msg("lte_segmentation.c: Illegal codeword size !!!\n");
    return(-1);
  }  

  if (*C == 1) {
    *Cplus = *C;
    *Kminus = 0;
    *Cminus = 0;
  }
  else {

    *Cminus = ((*C)*(*Kplus) - (Bprime))/((*Kplus) - (*Kminus));
    *Cplus  = (*C) - (*Cminus);
  }



  *F = ((*Cplus)*(*Kplus) + (*Cminus)*(*Kminus) - (Bprime));
#ifdef DEBUG_SEGMENTATION
  printf("C %d, Cplus %d, Cminus %d, Kplus %d, Kminus %d, Bprime_bytes %d, Bprime %d, F %d\n",*C,*Cplus,*Cminus,*Kplus,*Kminus,Bprime>>3,Bprime,*F);
#endif
  if ((input_buffer) && (output_buffers)) {

    for (k=0;k<*F>>3;k++) {
      output_buffers[0][k] = 0;
    } 
    s=0;
    for (r=0;r<*C;r++) {

      if (r<*Cminus)
	Kr = *Kminus;
      else
	Kr = *Kplus;

      while (k<((Kr - L)>>3)) {
	output_buffers[r][k] = input_buffer[s];
	k++;
	s++;
      }

      if (*C > 1) { // add CRC
	crc = crc24b(output_buffers[r],Kr-24)>>8;
	*(unsigned int*)(&output_buffers[r][(Kr-24)>>3]) = crc;
#ifdef DEBUG_SEGMENTATION
	printf("Segment %d : CRC %x\n",r,crc);
#endif
      }
      k=0;
    }
  }
}


#ifdef MAIN
main() {

  unsigned int Kplus,Kminus,C,Cplus,Cminus,F,Bbytes;
  
  for (Bbytes=5;Bbytes<2*768;Bbytes++) {
    lte_segmentation(0,0,Bbytes<<3,&C,&Cplus,&Cminus,&Kplus,&Kminus,&F);
    printf("Bbytes %d : C %d, Cplus %d, Cminus %d, Kplus %d, Kminus %d, F %d\n",
	   Bbytes, C, Cplus, Cminus, Kplus, Kminus, F);
  }
}
#endif
