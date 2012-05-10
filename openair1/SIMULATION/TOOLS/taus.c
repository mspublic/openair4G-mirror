
#ifndef RTAI_ENABLED
#include <time.h>
#else
#include <asm/io.h>
#include <asm/rtai.h>
#include <rtai.h>
#include <rtai_sched.h>
#define time(x) (unsigned int)(rt_get_time_ns())
#endif

unsigned int s0, s1, s2, b;

//----------------------------------------------
//
/*!
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

void set_taus_seed(unsigned int seed_init) {
  
  if (seed_init == 0) {
    s0 = (unsigned int)time(NULL); 
    s1 = (unsigned int)time(NULL); 
    s2 = (unsigned int)time(NULL); 
  }
  else {
    s0 = (unsigned int)0x1e23d852;
    s1 = (unsigned int)0x81f38a1c;
    s2 = (unsigned int)0xfe1a133e;
  }
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
#endif //MAIN

  
  
