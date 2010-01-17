/*
* @ingroup _PHY_CODING_BLOCKS_
* @{
*/

#include "defs.h"


unsigned short glte[] = { 0133, 0171, 0165 }; // {A,B}
unsigned short glte_rev[] = { 0155, 0117, 0127 }; // {A,B}   
unsigned char  ccodelte_table[128];      // for transmitter
unsigned char  ccodelte_table_rev[128];  // for receiver


/*************************************************************************

  Encodes for an arbitrary convolutional code of rate 1/2
  with a constraint length of 7 bits.
  The inputs are bit packed in octets (from MSB to LSB).
  Trellis termination is included here
*************************************************************************/



void
ccodelte_encode (unsigned int numbits, 
		 unsigned char *crc,
		 unsigned char *inPtr, 
		 unsigned char *outPtr) {
  unsigned int             state;

  unsigned char              c, out, shiftbit =0, first_bit;
  unsigned short next_last_byte;

  //  printf("In ccodelte_encode (%d,%p,%p,%d)\n",numbytes,inPtr,outPtr,puncturing);

#ifdef DEBUG_CCODE
  unsigned int  dummy=0;
#endif //DEBUG_CCODE

  /* The input bit is shifted in position 8 of the state.
     Shiftbit will take values between 1 and 8 */
  state = 0;

  // Perform Tail-biting
  next_last_byte = numbits>>3;
  first_bit      = (numbits-6)&7; 
  c = inPtr[next_last_byte-1];

#ifdef DEBUG_CCODE
  printf("next_last_byte %x (numbits %d, %d)\n",c,numbits,next_last_byte);
#endif

  for (shiftbit = first_bit; shiftbit<8; shiftbit++) {
    state >>= 1;
    if ((c&(1<<shiftbit)) != 0)
      state |= 64;

#ifdef DEBUG_CCODE
    printf("shiftbit %d, %d: %d -> %d \n",shiftbit,state>>6,dummy,state);
    dummy+=3;
#endif
  }
  if ((numbits&7)>0) {
    c = inPtr[next_last_byte];
    printf("last_byte %x\n",c);
    for (shiftbit = 0 ; shiftbit < (numbits&7) ; shiftbit++) {
      state >>= 1;
      if ((c&(1<<shiftbit)) != 0)
	state |= 64;
#ifdef DEBUG_CCODE
    printf("shiftbit %d, %d: %d -> %d \n",shiftbit,state>>6,dummy,state);
    dummy+=3;
#endif
    }
  }

  state = state & 0x3f;

#ifdef DEBUG_CCODE
  printf("Initial state %d\n",state);
  dummy = 0;
#endif //DEBUG_CCODE
  /* Do not increment inPtr until we read the next octet */
  while (numbits > 0) {
    c = *inPtr++;
#ifdef DEBUG_CCODE
    printf("** %x **\n",c);
#endif //DEBUG_CCODE


    for (shiftbit = 0; (shiftbit<8) && (numbits>0);shiftbit++,numbits--) {
      state >>= 1;
      if ((c&(1<<shiftbit)) != 0){
	state |= 64;
      }

      out = ccodelte_table[state];

      *outPtr++ = out  & 1;
      *outPtr++ = (out>>1)&1;
      *outPtr++ = (out>>2)&1;

#ifdef DEBUG_CCODE
      printf("numbits %d, input %d, outbit %d: %d -> %d (%d)\n",numbits,state>>6,dummy,state,out,ccodelte_table[state]);
      dummy+=3;
#endif //DEBUG_CCODE      

    }

  }


}



/*************************************************************************
  
  Functions to initialize the code tables

*************************************************************************/


/* Basic code table initialization for constraint length 7 */
/* Input in MSB, followed by state in 6 LSBs */

void ccodelte_init(void)
{
  unsigned int  i, j, k, sum;

  for (i = 0; i < 192; i++) {
    ccodelte_table[i] = 0;
    /* Compute 3 output bits */
    for (j = 0; j < 3; j++) {
      sum = 0;
      for (k = 0; k < 7; k++)
        if ((i & glte[j]) & (1 << k))
          sum++;
      /* Write the sum modulo 2 in bit j */
      ccodelte_table[i] |= (sum & 1) << j;
    }
  }
}

/* Input in LSB, followed by state in 6 MSBs */
void ccodelte_init_inv(void)
{
  unsigned int  i, j, k, sum;

  for (i = 0; i < 128; i++) {
    ccodelte_table_rev[i] = 0;
    /* Compute R output bits */
    for (j = 0; j < 3; j++) {
      sum = 0;
      for (k = 0; k < 7; k++)
        if ((i & glte_rev[j]) & (1 << k))
          sum++;
      /* Write the sum modulo 2 in bit j */
      ccodelte_table_rev[i] |= (sum & 1) << j;
    }
  }
}



/*****************************************************************/
/**
  Test program 

******************************************************************/

#ifdef CCODE_MAIN
#include <stdio.h>

main()
{
  unsigned char test[] = "Thebigredfox";
  unsigned char output[512], *inPtr, *outPtr;
  unsigned int i;

  test[0] = 128;
  test[1] = 0;


  ccodelte_init();
  
  inPtr = test;
  outPtr = output;

  ccodelte_encode(21, inPtr, outPtr);
  for (i = 0; i < 21*3; i++) printf("%x ", output[i]);

  printf("\n");
}
#endif

/** @}*/
