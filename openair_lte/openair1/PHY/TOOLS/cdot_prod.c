#include "defs.h"
#include <xmmintrin.h>
#include <pmmintrin.h>
#include <tmmintrin.h>

// returns the complex dot product of x and y 

#ifdef MAIN
void print_ints(char *s,__m128i *x);
void print_shorts(char *s,__m128i *x);
void print_bytes(char *s,__m128i *x);
#endif

int dot_product(short *x,
		short *y,
		unsigned int N, //must be a multiple of 8
		unsigned char output_shift) {

  __m128i *x128,*y128,mmtmp0,mmtmp1,mmtmp2,mmtmp3,mmtmp4,mmtmp5,mmtmp6,mmcumul;
  __m64 mmtmp7;
  __m128i minus_i = _mm_set_epi16(-1,1,-1,1,-1,1,-1,1);
  unsigned int n;
  int result;

  x128 = (__m128i*) x;
  y128 = (__m128i*) y;

  mmcumul = _mm_setzero_si128();

  for (n=0;n<(N>>3);n++) {
    // unroll 0
    //print_shorts("x",&x128[0]);
    //print_shorts("y",&y128[0]);

    // this computes Re(z) = Re(x)*Re(y)-Im(x)*Im(y)
    mmtmp0 = _mm_sign_epi16(y128[0],minus_i);
    mmtmp1 = _mm_madd_epi16(x128[0],mmtmp0);
    //print_ints("re",&mmtmp1);
    // mmtmp1 contains real part of 4 consecutive outputs (32-bit)
    
    // this computes Im(z) = Re(x)*Im(y) + Re(y)*Im(x)
    mmtmp2 = _mm_shufflelo_epi16(y128[0],_MM_SHUFFLE(2,3,0,1));
    mmtmp2 = _mm_shufflehi_epi16(mmtmp2,_MM_SHUFFLE(2,3,0,1));
    mmtmp3 = _mm_madd_epi16(x128[0],mmtmp2);
    //print_ints("im",&mmtmp3);
    // mmtmp1 contains imag part of 4 consecutive outputs (32-bit)
    
    // this returns Re0+R1 Re2+Re3 Im0+Im1 Im2+Im3
    mmtmp4 = _mm_hadd_epi32(mmtmp1,mmtmp3);
    //print_ints("add1",&mmtmp4);
    
    
    // unroll 1
    // this computes Re(z) = Re(x)*Re(y)-Im(x)*Im(y)
    mmtmp0 = _mm_sign_epi16(y128[1],minus_i);
    mmtmp1 = _mm_madd_epi16(x128[1],mmtmp0);
    //	print_ints("re",&mmtmp0);
    // mmtmp1 contains real part of 4 consecutive outputs (32-bit)
    
    // this computes Im(z) = Re(x)*Im(y) + Re(y)*Im(x)
    mmtmp2 = _mm_shufflelo_epi16(y128[1],_MM_SHUFFLE(2,3,0,1));
    mmtmp2 = _mm_shufflehi_epi16(mmtmp2,_MM_SHUFFLE(2,3,0,1));
    //	print_ints("im",&mmtmp1);
    mmtmp3 = _mm_madd_epi16(x128[1],mmtmp2);
    // mmtmp1 contains imag part of 4 consecutive outputs (32-bit)
    
    // this returns Re4+R5 Re6+Re7 Im4+Im5 Im6+Im7
    mmtmp5 = _mm_hadd_epi32(mmtmp1,mmtmp3);
    //print_ints("add1",&mmtmp5);
    

    // this returns Re0+R1+Re2+Re3 Im0+Im1+Im2+Im3 Re4+R5+Re6+Re7 Im4+Im5+Im6+Im7
    mmtmp6 = _mm_hadd_epi32(mmtmp4,mmtmp5);
    
    // accumulate results
    mmcumul = _mm_add_epi32(mmcumul,mmtmp6);
    //print_ints("mcumul",&mmcumul);

    x128+=2;
    y128+=2;
}


  // do the last add
  mmcumul = _mm_shuffle_epi32(mmcumul,_MM_SHUFFLE(3,1,2,0));
  mmcumul = _mm_hadd_epi32(mmcumul,mmcumul);  
  // extract the lower half
  mmtmp7 = _mm_movepi64_pi64(mmcumul);
  // shift back to 16 bits precission
  mmtmp7 = _mm_srai_pi32(mmtmp7, output_shift);
  // pack the result
  mmtmp7 = _mm_packs_pi32(mmtmp7,mmtmp7);
  // convert back to integer
  result = _mm_cvtsi64_si32(mmtmp7);

  return(result);
}



#ifdef MAIN
void print_bytes(char *s,__m128i *x) {

  char *tempb = (char *)x;

  printf("%s  : %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",s,
         tempb[0],tempb[1],tempb[2],tempb[3],tempb[4],tempb[5],tempb[6],tempb[7],
         tempb[8],tempb[9],tempb[10],tempb[11],tempb[12],tempb[13],tempb[14],tempb[15]
         );

}

void print_shorts(char *s,__m128i *x) {

  short *tempb = (short *)x;

  printf("%s  : %d,%d,%d,%d,%d,%d,%d,%d\n",s,
         tempb[0],tempb[1],tempb[2],tempb[3],tempb[4],tempb[5],tempb[6],tempb[7]
         );

}

void print_ints(char *s,__m128i *x) {

  int *tempb = (int *)x;

  printf("%s  : %d,%d,%d,%d\n",s,
         tempb[0],tempb[1],tempb[2],tempb[3]
         );

}

void main(void) {

  int result;

  short x[16*2] __attribute__((aligned(16))) = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};  
  short y[16*2] __attribute__((aligned(16))) = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

  result = dot_product(x,y,8*2,0);

  printf("result = %d, %d\n", ((short*) &result)[0],  ((short*) &result)[1] );
}
#endif
