/*******************************************************************************
    OpenAirInterface 
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is 
   included in this distribution in the file called "COPYING". If not, 
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@eurecom.fr
  
  Address      : Eurecom, Campus SophiaTech, 450 Route des Chappes, CS 50193 - 06904 Biot Sophia Antipolis cedex, FRANCE

 *******************************************************************************/
#include "defs.h"
#include <xmmintrin.h>
#ifdef __SSE3__
#include <pmmintrin.h>
#include <tmmintrin.h>
#else
static short zero[8]={0,0,0,0,0,0,0,0};
#define _mm_abs_epi16(xmmx) xmmx=_mm_xor_si128((xmmx),_mm_cmpgt_epi16(*(__m128i *)&zero[0],(xmmx)))
#define _mm_sign_epi16(xmmx,xmmy) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(*(__m128i *)&zero[0],(xmmy)))
#define _mm_hadd_epi32(xmmx,xmmy) _mm_unpacklo_epi64(_mm_add_epi32(_mm_shuffle_epi32((xmmx),_MM_SHUFFLE(0,2,0,2)),_mm_shuffle_epi32((xmmx),_MM_SHUFFLE(1,3,1,3))),_mm_add_epi32(_mm_shuffle_epi32((xmmy),_MM_SHUFFLE(0,2,0,2)),_mm_shuffle_epi32((xmmy),_MM_SHUFFLE(1,3,1,3))))
#endif

// returns the complex dot product of x and y 

#ifdef MAIN
void print_ints(char *s,__m128i *x);
void print_shorts(char *s,__m128i *x);
void print_bytes(char *s,__m128i *x);
#endif

int32_t dot_product(int16_t *x,
		int16_t *y,
		uint32_t N, //must be a multiple of 8
		uint8_t output_shift) {

  __m128i *x128,*y128,mmtmp1,mmtmp2,mmtmp3,mmcumul,mmcumul_re,mmcumul_im;
  __m64 mmtmp7;
  __m128i minus_i = _mm_set_epi16(-1,1,-1,1,-1,1,-1,1);
  uint32_t n;
  int32_t result;

  x128 = (__m128i*) x;
  y128 = (__m128i*) y;

  mmcumul_re = _mm_setzero_si128();
  mmcumul_im = _mm_setzero_si128();

  for (n=0;n<(N>>2);n++) {

    //printf("n=%d, x128=%p, y128=%p\n",n,x128,y128);
    //    print_shorts("x",&x128[0]);
    //    print_shorts("y",&y128[0]);

    // this computes Re(z) = Re(x)*Re(y) + Im(x)*Im(y)
    mmtmp1 = _mm_madd_epi16(x128[0],y128[0]);
    //    print_ints("re",&mmtmp1);
	// mmtmp1 contains real part of 4 consecutive outputs (32-bit)

    // shift and accumulate results
    mmtmp1 = _mm_srai_epi32(mmtmp1,output_shift);
    mmcumul_re = _mm_add_epi32(mmcumul_re,mmtmp1);
    //    print_ints("re",&mmcumul_re);

    
    // this computes Im(z) = Re(x)*Im(y) - Re(y)*Im(x)
    mmtmp2 = _mm_shufflelo_epi16(y128[0],_MM_SHUFFLE(2,3,0,1));
    //    print_shorts("y",&mmtmp2);
    mmtmp2 = _mm_shufflehi_epi16(mmtmp2,_MM_SHUFFLE(2,3,0,1));
    //    print_shorts("y",&mmtmp2);
    mmtmp2 = _mm_sign_epi16(mmtmp2,minus_i);
    //        print_shorts("y",&mmtmp2);

    mmtmp3 = _mm_madd_epi16(x128[0],mmtmp2);
    //        print_ints("im",&mmtmp3);
    // mmtmp3 contains imag part of 4 consecutive outputs (32-bit)

    // shift and accumulate results
    mmtmp3 = _mm_srai_epi32(mmtmp3,output_shift);
    mmcumul_im = _mm_add_epi32(mmcumul_im,mmtmp3);
    //    print_ints("im",&mmcumul_im);

    x128++;
    y128++;
  }

  // this gives Re Re Im Im
  mmcumul = _mm_hadd_epi32(mmcumul_re,mmcumul_im);
  //  print_ints("cumul1",&mmcumul);

  // this gives Re Im Re Im  
  mmcumul = _mm_hadd_epi32(mmcumul,mmcumul);  

  //  print_ints("cumul2",&mmcumul);


  //mmcumul = _mm_srai_epi32(mmcumul,output_shift);
  // extract the lower half
  mmtmp7 = _mm_movepi64_pi64(mmcumul);
  //  print_ints("mmtmp7",&mmtmp7);
  // pack the result
  mmtmp7 = _mm_packs_pi32(mmtmp7,mmtmp7);
  //  print_shorts("mmtmp7",&mmtmp7);
  // convert back to integer
  result = _mm_cvtsi64_si32(mmtmp7);

  _mm_empty();
  _m_empty();


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

  int16_t *tempb = (int16_t *)x;

  printf("%s  : %d,%d,%d,%d,%d,%d,%d,%d\n",s,
         tempb[0],tempb[1],tempb[2],tempb[3],tempb[4],tempb[5],tempb[6],tempb[7]
         );

}

void print_ints(char *s,__m128i *x) {

  int32_t *tempb = (int32_t *)x;

  printf("%s  : %d,%d,%d,%d\n",s,
         tempb[0],tempb[1],tempb[2],tempb[3]
         );

}

void main(void) {

  int32_t result;

  int16_t x[16*2] __attribute__((aligned(16))) = {1<<0,1<<1,1<<2,1<<3,1<<4,1<<5,1<<6,1<<7,1<<8,1<<9,1<<10,1<<11,1<<12,1<<13,1<<12,1<<13,1<<0,1<<1,1<<2,1<<3,1<<4,1<<5,1<<6,1<<7,1<<8,1<<9,1<<10,1<<11,1<<12,1<<13,1<<12,1<<13};  
  int16_t y[16*2] __attribute__((aligned(16))) = {1<<0,1<<1,1<<2,1<<3,1<<4,1<<5,1<<6,1<<7,1<<8,1<<9,1<<10,1<<11,1<<12,1<<13,1<<12,1<<13,1<<0,1<<1,1<<2,1<<3,1<<4,1<<5,1<<6,1<<7,1<<8,1<<9,1<<10,1<<11,1<<12,1<<13,1<<12,1<<13};  
  //  int16_t y[16*2] __attribute__((aligned(16))) = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

  result = dot_product(x,y,8*2,15);

  printf("result = %d, %d\n", ((int16_t*) &result)[0],  ((int16_t*) &result)[1] );
}
#endif
