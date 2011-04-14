/* file: 3gpplte.c
   purpose: Encoding routines for implementing Turbo-coded (DLSCH) transport channels from 36-212, V8.6 2009-03
   author: raymond.knopp@eurecom.fr
   date: 10.2009 
*/
#include "defs.h"
#include "lte_interleaver_inline.h"


//#define DEBUG_TURBO_ENCODER 1

unsigned short threegpplte_interleaver_output;
unsigned int threegpplte_interleaver_tmp;

inline void threegpplte_interleaver_reset() {
  threegpplte_interleaver_output = 0;
  threegpplte_interleaver_tmp    = 0;
}

inline unsigned short threegpplte_interleaver(unsigned short f1,
					      unsigned short f2,
					      unsigned short K) {

  threegpplte_interleaver_tmp = (threegpplte_interleaver_tmp+(f2<<1));

  threegpplte_interleaver_output = (threegpplte_interleaver_output + threegpplte_interleaver_tmp + f1 - f2)%K;

#ifdef DEBUG_TURBO_ENCODER

  //  printf("Interleaver output %d\n",threegpplte_interleaver_output);
#endif
  return(threegpplte_interleaver_output);
}

inline unsigned char threegpplte_rsc(unsigned char input,unsigned char *state) {

  unsigned char output;

  output = (input ^ (*state>>2) ^ (*state>>1))&1;
  *state = (((input<<2)^(*state>>1))^((*state>>1)<<2)^((*state)<<2))&7;
  return(output);

}

inline void threegpplte_rsc_termination(unsigned char *x,unsigned char *z,unsigned char *state) {


  *z     = ((*state>>2) ^ (*state))   &1;
  *x     = ((*state)    ^ (*state>>1))   &1;
  *state = (*state)>>1;


}

void threegpplte_turbo_encoder(unsigned char *input,
			       unsigned short input_length_bytes,
			       unsigned char *output,
			       unsigned char F,
			       unsigned short interleaver_f1,
			       unsigned short interleaver_f2) {
  
  int i,k=0;
  unsigned char *x;
  unsigned char b,z,zprime,xprime;
  unsigned char state0=0,state1=0;
  unsigned short input_length_bits = input_length_bytes<<3, pi=0,pi_pos,pi_bitpos;


  x = output;
  threegpplte_interleaver_reset();
  pi = 0;
  
  for (i=0;i<input_length_bytes;i++) {
    
#ifdef DEBUG_TURBO_ENCODER
    printf("\n****input %x : %x\n",i,input[i]);
#endif //DEBUG_TURBO_ENCODER

    for (b=0;b<8;b++) {
      *x = (input[i]&(1<<(7-b)))>>(7-b);

#ifdef DEBUG_TURBO_ENCODER
      printf("bit %d: %d\n",b,*x);
#endif //DEBUG_TURBO_ENCODER

#ifdef DEBUG_TURBO_ENCODER
      printf("state0: %d\n",state0);
#endif //DEBUG_TURBO_ENCODER
      z               = threegpplte_rsc(*x,&state0) ;

#ifdef DEBUG_TURBO_ENCODER
      printf("(x,z): (%d,%d),state0 %d\n",*x,z,state0);
#endif //DEBUG_TURBO_ENCODER

      // Filler bits get punctured
      if (k<F) {
	*x = LTE_NULL;
	z  = LTE_NULL;
      }

      pi_pos          = pi>>3; 
      pi_bitpos       = pi&7;
      xprime          = (input[pi_pos]&(1<<(7-pi_bitpos)))>>(7-pi_bitpos);
      zprime          = threegpplte_rsc(xprime,&state1);
#ifdef DEBUG_TURBO_ENCODER 
      printf("pi %d, pi_pos %d, pi_bitpos %d, x %d, z %d, xprime %d, zprime %d, state0 %d state1 %d\n",pi,pi_pos,pi_bitpos,*x,z,xprime,zprime,state0,state1);
#endif //DEBUG_TURBO_ENCODER
      x[1]            = z;
      x[2]            = zprime;
      

      x+=3;

      pi              = threegpplte_interleaver(interleaver_f1,interleaver_f2,input_length_bits);
      k++;
    }
  }
  // Trellis termination
  threegpplte_rsc_termination(&x[0],&x[1],&state0);
#ifdef DEBUG_TURBO_ENCODER 
  printf("term: x0 %d, x1 %d, state0 %d\n",x[0],x[1],state0);
#endif //DEBUG_TURBO_ENCODER

  threegpplte_rsc_termination(&x[2],&x[3],&state0);
#ifdef DEBUG_TURBO_ENCODER 
  printf("term: x0 %d, x1 %d, state0 %d\n",x[2],x[3],state0);
#endif //DEBUG_TURBO_ENCODER

  threegpplte_rsc_termination(&x[4],&x[5],&state0);
#ifdef DEBUG_TURBO_ENCODER 
  printf("term: x0 %d, x1 %d, state0 %d\n",x[4],x[5],state0);
#endif //DEBUG_TURBO_ENCODER

  threegpplte_rsc_termination(&x[6],&x[7],&state1);

#ifdef DEBUG_TURBO_ENCODER 
  printf("term: x0 %d, x1 %d, state1 %d\n",x[6],x[7],state1);
#endif //DEBUG_TURBO_ENCODER
  threegpplte_rsc_termination(&x[8],&x[9],&state1);
#ifdef DEBUG_TURBO_ENCODER 
  printf("term: x0 %d, x1 %d, state1 %d\n",x[8],x[9],state1);
#endif //DEBUG_TURBO_ENCODER
  threegpplte_rsc_termination(&x[10],&x[11],&state1);

#ifdef DEBUG_TURBO_ENCODER 
  printf("term: x0 %d, x1 %d, state1 %d\n",x[10],x[11],state1);
#endif //DEBUG_TURBO_ENCODER

}

inline short threegpp_interleaver_parameters(unsigned short bytes_per_codeword) {
  if (bytes_per_codeword<=64)
    return (bytes_per_codeword-5);
  else if (bytes_per_codeword <=128)
    return (59 + ((bytes_per_codeword-64)>>1));
  else if (bytes_per_codeword <= 256)
    return (91 + ((bytes_per_codeword-128)>>2));
  else if (bytes_per_codeword <= 768)
    return (123 + ((bytes_per_codeword-256)>>3));
  else {
#ifdef DEBUG_TURBO_ENCODER
    printf("Illegal codeword size !!!\n");
#endif 
    return(-1);
  }
}


#ifdef MAIN

#define INPUT_LENGTH 5
#define F1 3
#define F2 10

int main(int argc,char **argv) {

  unsigned char input[INPUT_LENGTH],state,state2;
  unsigned char output[12+(3*(INPUT_LENGTH<<3))],x,z;
  int i;
  unsigned char out;

  for (state=0;state<8;state++) {
    for (i=0;i<2;i++) {
      state2=state;
      out = threegpplte_rsc(i,&state2);
      printf("State (%d->%d) : (%d,%d)\n",state,state2,i,out);      
    }
  }
  printf("\n");

  for (state=0;state<8;state++) {
    
    state2=state;
    threegpplte_rsc_termination(&x,&z,&state2);
    printf("Termination: (%d->%d) : (%d,%d)\n",state,state2,x,z);   
  }

  for (i=0;i<5;i++) {
    input[i] = i*219;
    printf("Input %d : %x\n",i,input[i]);
  }

  threegpplte_turbo_encoder(&input[0],
			    5,
			    &output[0],
			    F1,
			    F2);
  return(0);
}

#endif // MAIN
