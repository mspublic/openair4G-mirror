/*! \mainpage Useful Numerical Functions
\section scope_numerical Scope
The present document specifies several numerical functions for testing of digital communication systems.

-# Generation of Uniform Random Bits
-# Generation of Quantized Gaussian Random Variables
-# Generation of QAM-modulated random sequences (with provision for OFDM signal formats)

@defgroup numerical

@defgroup _taus 
@ingroup numerical Tausworthe Uniform Random Variable Generator.*/


#include <time.h>

unsigned int s0, s1, s2, b;

//----------------------------------------------
//
/*!\brief Tausworthe Uniform Random Generator.  This is based on the hardware implementation described in 
  Lee et al, "A Hardware Gaussian Noise Generator Usign the Box-Muller Method and its Error Analysis," IEEE Trans. on Computers, 2006.
*/
//

inline unsigned int taus() {

  b = (((s0 << 13) ^ s0) >> 19);
  s0 = (((s0 & 0xFFFFFFFE) << 12)^  b);
  b = (((s1 << 2) ^ s1) >> 25);
  s1 = (((s1 & 0xFFFFFFF8) << 4)^  b);
  b = (((s2 << 3) ^ s2) >> 11);
  s2 = (((s2 & 0xFFFFFFF0) << 17)^  b);
  return s0 ^ s1 ^ s2;
}

void set_taus_seed() {


  s0 = (unsigned int)time(NULL);// 0x1e23d852;
  s1 = (unsigned int)time(NULL); //0x81f38a1c;
  s2 = (unsigned int)time(NULL); //0xfe1a133e;

}

#ifdef MAIN

main() {

  unsigned int i,rand;

  set_taus_seed();

  for (i=0;i<10;i++){

    rand = taus();
    printf("%u\n",rand);

  }
}
#endif MAIN

  
  
