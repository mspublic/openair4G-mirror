#ifdef USER_MODE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include <stdint.h>

#ifndef MR_MAIN
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "defs.h"
#else
#include "PHY/TOOLS/twiddle512.h"
#include "PHY/TOOLS/twiddle2048.h"
#include "PHY/TOOLS/twiddle4096.h"
#include "PHY/TOOLS/twiddle8192.h"
#include "time_meas.h"

int rev2048[2048],rev512[512],rev4096[4096],rev8192[8192];
#define debug_msg
#define ONE_OVER_SQRT2_Q15 23170

#endif


#include "emmintrin.h"
#include "xmmintrin.h"


static int16_t conjugatedft[8] __attribute__((aligned(16))) = {-1,1,-1,1,-1,1,-1,1} ;


#ifndef __SSE3__
__m128i zerodft;
#define _mm_abs_epi16(xmmx) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zerodft,(xmmx)))
#define _mm_sign_epi16(xmmx,xmmy) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zerodft,(xmmy)))
#else
#include <pmmintrin.h>
#include <tmmintrin.h>
#endif

static short reflip[8]  __attribute__((aligned(16))) = {1,-1,1,-1,1,-1,1,-1};

static inline void cmac(__m128i a,__m128i b, __m128i *re32, __m128i *im32) __attribute__((always_inline));
static inline void cmac(__m128i a,__m128i b, __m128i *re32, __m128i *im32) {

  __m128i cmac_tmp,cmac_tmp_re32,cmac_tmp_im32;

  cmac_tmp    = _mm_sign_epi16(b,*(__m128i*)reflip);
  cmac_tmp_re32  = _mm_madd_epi16(a,cmac_tmp);


  //  cmac_tmp    = _mm_shufflelo_epi16(b,_MM_SHUFFLE(2,3,0,1));
  //  cmac_tmp    = _mm_shufflehi_epi16(cmac_tmp,_MM_SHUFFLE(2,3,0,1));
  cmac_tmp = _mm_shuffle_epi8(b,_mm_set_epi8(13,12,15,14,9,8,11,10,5,4,7,6,1,0,3,2));
  cmac_tmp_im32  = _mm_madd_epi16(cmac_tmp,a);

  *re32 = _mm_add_epi32(*re32,cmac_tmp_re32);
  *im32 = _mm_add_epi32(*im32,cmac_tmp_im32);
}




static inline void cmult(__m128i a,__m128i b, __m128i *re32, __m128i *im32) __attribute__((always_inline));

static inline void cmult(__m128i a,__m128i b, __m128i *re32, __m128i *im32) {

  register __m128i mmtmpb;

  mmtmpb    = _mm_sign_epi16(b,*(__m128i*)reflip);
  *re32     = _mm_madd_epi16(a,mmtmpb);
  //  mmtmpb    = _mm_shufflelo_epi16(b,_MM_SHUFFLE(2,3,0,1));
  //  mmtmpb    = _mm_shufflehi_epi16(mmtmpb,_MM_SHUFFLE(2,3,0,1));
  mmtmpb        = _mm_shuffle_epi8(b,_mm_set_epi8(13,12,15,14,9,8,11,10,5,4,7,6,1,0,3,2));
  *im32  = _mm_madd_epi16(a,mmtmpb);

}

static inline void cmultc(__m128i a,__m128i b, __m128i *re32, __m128i *im32) __attribute__((always_inline));

static inline void cmultc(__m128i a,__m128i b, __m128i *re32, __m128i *im32) {

  register __m128i mmtmpb;

  *re32     = _mm_madd_epi16(a,b);
  mmtmpb    = _mm_sign_epi16(b,*(__m128i*)reflip);
  //  mmtmpb    = _mm_shufflelo_epi16(mmtmpb,_MM_SHUFFLE(2,3,0,1));
  //  mmtmpb    = _mm_shufflehi_epi16(mmtmpb,_MM_SHUFFLE(2,3,0,1));
  mmtmpb    = _mm_shuffle_epi8(mmtmpb,_mm_set_epi8(13,12,15,14,9,8,11,10,5,4,7,6,1,0,3,2));
  *im32  = _mm_madd_epi16(a,mmtmpb);

}


static inline __m128i cpack(__m128i xre,__m128i xim) __attribute__((always_inline));

static inline __m128i cpack(__m128i xre,__m128i xim) {

  register __m128i cpack_tmp1,cpack_tmp2;

  cpack_tmp1 = _mm_unpacklo_epi32(xre,xim);
  cpack_tmp2 = _mm_unpackhi_epi32(xre,xim);
  return(_mm_packs_epi32(_mm_srai_epi32(cpack_tmp1,15),_mm_srai_epi32(cpack_tmp2,15)));

}


static inline void packed_cmult(__m128i a,__m128i b, __m128i *c) __attribute__((always_inline));

static inline void packed_cmult(__m128i a,__m128i b, __m128i *c) {

  __m128i cre,cim;
  cmult(a,b,&cre,&cim);
  *c = cpack(cre,cim);

}


static inline void packed_cmultc(__m128i a,__m128i b, __m128i *c) __attribute__((always_inline));

static inline void packed_cmultc(__m128i a,__m128i b, __m128i *c) {

  __m128i cre,cim;

  cmultc(a,b,&cre,&cim);
  *c = cpack(cre,cim);

}

static inline __m128i packed_cmult2(__m128i a,__m128i b,__m128i b2) __attribute__((always_inline));

static inline __m128i packed_cmult2(__m128i a,__m128i b,__m128i b2) {
  
  
  register __m128i cre,cim;

  cre       = _mm_madd_epi16(a,b);
  cim       = _mm_madd_epi16(a,b2);
  /*
  mmtmpb    = _mm_sign_epi16(b,*(__m128i*)reflip);
  cre       = _mm_madd_epi16(a,mmtmpb);
  mmtmpb    = _mm_shufflelo_epi16(b,_MM_SHUFFLE(2,3,0,1));
  mmtmpb    = _mm_shufflehi_epi16(mmtmpb,_MM_SHUFFLE(2,3,0,1));
  cim       = _mm_madd_epi16(a,mmtmpb);
  */
  /*
  __m128i cre,cim;
  cmult(a,b,&cre,&cim);
  */

  return(cpack(cre,cim));

}

/*
static inline __m128i packed_cmultc2(__m128i a,__m128i b,__m128i b2) __attribute__((always_inline));

static inline __m128i packed_cmultc2(__m128i a,__m128i b,__m128i b2) {

  __m128i cre,cim;
  
  cmultc(a,b,&cre,&cim);
  return(cpack(cre,cim));

}
*/

static int16_t W0s[8]__attribute__((aligned(16))) = {32767,0,32767,0,32767,0,32767,0};

static int16_t W13s[8]__attribute__((aligned(16))) = {-16384,-28378,-16384,-28378,-16384,-28378,-16384,-28378};
static int16_t W23s[8]__attribute__((aligned(16))) = {-16384,28378,-16384,28378,-16384,28378,-16384,28378};

static int16_t W15s[8]__attribute__((aligned(16))) = {10126,-31163,10126,-31163,10126,-31163,10126,-31163};
static int16_t W25s[8]__attribute__((aligned(16))) = {-26509,-19260,-26509,-19260,-26509,-19260,-26509,-19260};
static int16_t W35s[8]__attribute__((aligned(16))) = {-26510,19260,-26510,19260,-26510,19260,-26510,19260};
static int16_t W45s[8]__attribute__((aligned(16))) = {10126,31163,10126,31163,10126,31163,10126,31163};

__m128i *W0 = (__m128i *)W0s;
__m128i *W13 = (__m128i *)W13s;
__m128i *W23 = (__m128i *)W23s;
__m128i *W15 = (__m128i *)W15s;
__m128i *W25 = (__m128i *)W25s;
__m128i *W35 = (__m128i *)W35s;
__m128i *W45 = (__m128i *)W45s;

static int16_t dft_norm_table[16] = {9459,  //12
				     6689,//24
				     5461,//36
				     4729,//48
				     4230,//60
				     23170,//72 
				     3344,//96
				     3153,//108
				     2991,//120
				     18918,//sqrt(3),//144
				     18918,//sqrt(3),//180
				     16384,//2, //192
				     18918,//sqrt(3), // 216
				     16384,//2, //240
				     18918,//sqrt(3), // 288
				     14654}; //sqrt(5) //300


static inline void bfly2(__m128i *x0, __m128i *x1,__m128i *y0, __m128i *y1,__m128i *tw)__attribute__((always_inline));

static inline void bfly2(__m128i *x0, __m128i *x1,__m128i *y0, __m128i *y1,__m128i *tw) {

  __m128i x0r_2,x0i_2,x1r_2,x1i_2,dy0r,dy1r,dy0i,dy1i;
  __m128i bfly2_tmp1,bfly2_tmp2;

  cmult(*(x0),*(W0),&x0r_2,&x0i_2);
  cmult(*(x1),*(tw),&x1r_2,&x1i_2);

  dy0r = _mm_srai_epi32(_mm_add_epi32(x0r_2,x1r_2),15);
  dy1r = _mm_srai_epi32(_mm_sub_epi32(x0r_2,x1r_2),15);
  dy0i = _mm_srai_epi32(_mm_add_epi32(x0i_2,x1i_2),15);
  //  printf("y0i %d\n",((int16_t *)y0i)[0]);
  dy1i = _mm_srai_epi32(_mm_sub_epi32(x0i_2,x1i_2),15);
  
  bfly2_tmp1 = _mm_unpacklo_epi32(dy0r,dy0i);
  bfly2_tmp2 = _mm_unpackhi_epi32(dy0r,dy0i);
  *y0 = _mm_packs_epi32(bfly2_tmp1,bfly2_tmp2);

  bfly2_tmp1 = _mm_unpacklo_epi32(dy1r,dy1i);
  bfly2_tmp2 = _mm_unpackhi_epi32(dy1r,dy1i);
  *y1 = _mm_packs_epi32(bfly2_tmp1,bfly2_tmp2);
}

static inline void bfly2_tw1(__m128i *x0, __m128i *x1, __m128i *y0, __m128i *y1)__attribute__((always_inline));

static inline void bfly2_tw1(__m128i *x0, __m128i *x1, __m128i *y0, __m128i *y1) {

  *y0  = _mm_adds_epi16(*x0,*x1);
  *y1  = _mm_subs_epi16(*x0,*x1);

}

static inline void bfly2_16(__m128i *x0, __m128i *x1, __m128i *y0, __m128i *y1, __m128i *tw, __m128i *twb)__attribute__((always_inline));

static inline void bfly2_16(__m128i *x0, __m128i *x1, __m128i *y0, __m128i *y1, __m128i *tw, __m128i *twb) {

  register __m128i x1t;

  x1t = packed_cmult2(*(x1),*(tw),*(twb));
  
  *y0  = _mm_adds_epi16(*x0,x1t);
  *y1  = _mm_subs_epi16(*x0,x1t);

}


static inline void ibfly2(__m128i *x0, __m128i *x1,__m128i *y0, __m128i *y1,__m128i *tw)__attribute__((always_inline));

static inline void ibfly2(__m128i *x0, __m128i *x1,__m128i *y0, __m128i *y1,__m128i *tw){

  __m128i x0r_2,x0i_2,x1r_2,x1i_2,dy0r,dy1r,dy0i,dy1i;
  __m128i bfly2_tmp1,bfly2_tmp2;

  cmultc(*(x0),*(W0),&x0r_2,&x0i_2);
  cmultc(*(x1),*(tw),&x1r_2,&x1i_2);

  dy0r = _mm_srai_epi32(_mm_add_epi32(x0r_2,x1r_2),15);
  dy1r = _mm_srai_epi32(_mm_sub_epi32(x0r_2,x1r_2),15);
  dy0i = _mm_srai_epi32(_mm_add_epi32(x0i_2,x1i_2),15);
  //  printf("y0i %d\n",((int16_t *)y0i)[0]);
  dy1i = _mm_srai_epi32(_mm_sub_epi32(x0i_2,x1i_2),15);
  
  bfly2_tmp1 = _mm_unpacklo_epi32(dy0r,dy0i);
  bfly2_tmp2 = _mm_unpackhi_epi32(dy0r,dy0i);
  *y0 = _mm_packs_epi32(bfly2_tmp1,bfly2_tmp2);

  bfly2_tmp1 = _mm_unpacklo_epi32(dy1r,dy1i);
  bfly2_tmp2 = _mm_unpackhi_epi32(dy1r,dy1i);
  *y1 = _mm_packs_epi32(bfly2_tmp1,bfly2_tmp2);
}



// This is the radix-3 butterfly (fft)
static inline void bfly3(__m128i *x0,__m128i *x1,__m128i *x2,
			 __m128i *y0,__m128i *y1,__m128i *y2,
			 __m128i *tw1,__m128i *tw2) __attribute__((always_inline));

static inline void bfly3(__m128i *x0,__m128i *x1,__m128i *x2,
			 __m128i *y0,__m128i *y1,__m128i *y2,
			 __m128i *tw1,__m128i *tw2)  { 

  __m128i tmpre,tmpim,x1_2,x2_2;

  packed_cmult(*(x1),*(tw1),&x1_2); 
  packed_cmult(*(x2),*(tw2),&x2_2); 
  *(y0)  = _mm_adds_epi16(*(x0),_mm_adds_epi16(x1_2,x2_2)); 
  cmult(x1_2,*(W13),&tmpre,&tmpim); 
  cmac(x2_2,*(W23),&tmpre,&tmpim);  
  *(y1) = cpack(tmpre,tmpim); 
  *(y1) = _mm_adds_epi16(*(x0),*(y1));
  cmult(x1_2,*(W23),&tmpre,&tmpim); 
  cmac(x2_2,*(W13),&tmpre,&tmpim);  
  *(y2) = cpack(tmpre,tmpim); 
  *(y2) = _mm_adds_epi16(*(x0),*(y2));
}

static inline void bfly3_tw1(__m128i *x0,__m128i *x1,__m128i *x2,
			     __m128i *y0,__m128i *y1,__m128i *y2) __attribute__((always_inline));

static inline void bfly3_tw1(__m128i *x0,__m128i *x1,__m128i *x2,
			     __m128i *y0,__m128i *y1,__m128i *y2) {

  __m128i tmpre,tmpim;

  *(y0) = _mm_adds_epi16(*(x0),_mm_adds_epi16(*(x1),*(x2)));	
  cmult(*(x1),*(W13),&tmpre,&tmpim); 
  cmac(*(x2),*(W23),&tmpre,&tmpim);  
  *(y1) = cpack(tmpre,tmpim); 
  *(y1) = _mm_adds_epi16(*(x0),*(y1));
  cmult(*(x1),*(W23),&tmpre,&tmpim); 
  cmac(*(x2),*(W13),&tmpre,&tmpim);  
  *(y2) = cpack(tmpre,tmpim); 
  *(y2) = _mm_adds_epi16(*(x0),*(y2));
}


static inline void bfly4(__m128i *x0,__m128i *x1,__m128i *x2,__m128i *x3,
			 __m128i *y0,__m128i *y1,__m128i *y2,__m128i *y3,
			 __m128i *tw1,__m128i *tw2,__m128i *tw3)__attribute__((always_inline));

static inline void bfly4(__m128i *x0,__m128i *x1,__m128i *x2,__m128i *x3,
			 __m128i *y0,__m128i *y1,__m128i *y2,__m128i *y3,
			 __m128i *tw1,__m128i *tw2,__m128i *tw3) {   

  __m128i x1r_2,x1i_2,x2r_2,x2i_2,x3r_2,x3i_2,dy0r,dy0i,dy1r,dy1i,dy2r,dy2i,dy3r,dy3i;

  //  cmult(*(x0),*(W0),&x0r_2,&x0i_2);		
  cmult(*(x1),*(tw1),&x1r_2,&x1i_2);
  cmult(*(x2),*(tw2),&x2r_2,&x2i_2);
  cmult(*(x3),*(tw3),&x3r_2,&x3i_2);
  //  dy0r = _mm_add_epi32(x0r_2,_mm_add_epi32(x1r_2,_mm_add_epi32(x2r_2,x3r_2)));
  //  dy0i = _mm_add_epi32(x0i_2,_mm_add_epi32(x1i_2,_mm_add_epi32(x2i_2,x3i_2)));
  //  *(y0)  = cpack(dy0r,dy0i);
  dy0r = _mm_add_epi32(x1r_2,_mm_add_epi32(x2r_2,x3r_2));
  dy0i = _mm_add_epi32(x1i_2,_mm_add_epi32(x2i_2,x3i_2));
  *(y0)  = _mm_add_epi16(*(x0),cpack(dy0r,dy0i));
  //  dy1r = _mm_add_epi32(x0r_2,_mm_sub_epi32(x1i_2,_mm_add_epi32(x2r_2,x3i_2)));
  //  dy1i = _mm_sub_epi32(x0i_2,_mm_add_epi32(x1r_2,_mm_sub_epi32(x2i_2,x3r_2)));
  //  *(y1)  = cpack(dy1r,dy1i);
  dy1r = _mm_sub_epi32(x1i_2,_mm_add_epi32(x2r_2,x3i_2));
  dy1i = _mm_sub_epi32(_mm_sub_epi32(x3r_2,x2i_2),x1r_2);
  *(y1)  = _mm_add_epi16(*(x0),cpack(dy1r,dy1i));
  //  dy2r = _mm_sub_epi32(x0r_2,_mm_sub_epi32(x1r_2,_mm_sub_epi32(x2r_2,x3r_2)));
  //  dy2i = _mm_sub_epi32(x0i_2,_mm_sub_epi32(x1i_2,_mm_sub_epi32(x2i_2,x3i_2)));
  //  *(y2)  = cpack(dy2r,dy2i);
  dy2r = _mm_sub_epi32(_mm_sub_epi32(x2r_2,x3r_2),x1r_2);
  dy2i = _mm_sub_epi32(_mm_sub_epi32(x2i_2,x3i_2),x1i_2);
  *(y2)  = _mm_add_epi16(*(x0),cpack(dy2r,dy2i));
  //  dy3r = _mm_sub_epi32(x0r_2,_mm_add_epi32(x1i_2,_mm_sub_epi32(x2r_2,x3i_2)));
  //  dy3i = _mm_add_epi32(x0i_2,_mm_sub_epi32(x1r_2,_mm_add_epi32(x2i_2,x3r_2)));
  //  *(y3) = cpack(dy3r,dy3i);
  dy3r = _mm_sub_epi32(_mm_sub_epi32(x3i_2,x2r_2),x1i_2);
  dy3i = _mm_sub_epi32(x1r_2,_mm_add_epi32(x2i_2,x3r_2));
  *(y3) = _mm_add_epi16(*(x0),cpack(dy3r,dy3i));
}

static inline void ibfly4(__m128i *x0,__m128i *x1,__m128i *x2,__m128i *x3,
			  __m128i *y0,__m128i *y1,__m128i *y2,__m128i *y3,
			  __m128i *tw1,__m128i *tw2,__m128i *tw3)__attribute__((always_inline));
   
static inline void ibfly4(__m128i *x0,__m128i *x1,__m128i *x2,__m128i *x3,
			  __m128i *y0,__m128i *y1,__m128i *y2,__m128i *y3,
			  __m128i *tw1,__m128i *tw2,__m128i *tw3) {   

  __m128i x1r_2,x1i_2,x2r_2,x2i_2,x3r_2,x3i_2,dy0r,dy0i,dy1r,dy1i,dy2r,dy2i,dy3r,dy3i;

  //  cmultc(*(x0),*(W0),&x0r_2,&x0i_2);		
  cmultc(*(x1),*(tw1),&x1r_2,&x1i_2);
  cmultc(*(x2),*(tw2),&x2r_2,&x2i_2);
  cmultc(*(x3),*(tw3),&x3r_2,&x3i_2);
  /*
  dy0r = _mm_add_epi32(x0r_2,_mm_add_epi32(x1r_2,_mm_add_epi32(x2r_2,x3r_2)));
  dy0i = _mm_add_epi32(x0i_2,_mm_add_epi32(x1i_2,_mm_add_epi32(x2i_2,x3i_2)));
  *(y0)  = cpack(dy0r,dy0i);
  dy3r = _mm_add_epi32(x0r_2,_mm_sub_epi32(x1i_2,_mm_add_epi32(x2r_2,x3i_2)));
  dy3i = _mm_sub_epi32(x0i_2,_mm_add_epi32(x1r_2,_mm_sub_epi32(x2i_2,x3r_2)));
  *(y3)  = cpack(dy3r,dy3i);
  dy2r = _mm_sub_epi32(x0r_2,_mm_sub_epi32(x1r_2,_mm_sub_epi32(x2r_2,x3r_2)));
  dy2i = _mm_sub_epi32(x0i_2,_mm_sub_epi32(x1i_2,_mm_sub_epi32(x2i_2,x3i_2)));
  *(y2)  = cpack(dy2r,dy2i);
  dy1r = _mm_sub_epi32(x0r_2,_mm_add_epi32(x1i_2,_mm_sub_epi32(x2r_2,x3i_2)));
  dy1i = _mm_add_epi32(x0i_2,_mm_sub_epi32(x1r_2,_mm_add_epi32(x2i_2,x3r_2)));
  *(y1) = cpack(dy1r,dy1i);
  */
  dy0r = _mm_add_epi32(x1r_2,_mm_add_epi32(x2r_2,x3r_2));
  dy0i = _mm_add_epi32(x1i_2,_mm_add_epi32(x2i_2,x3i_2));
  *(y0)  = _mm_add_epi16(*(x0),cpack(dy0r,dy0i));
  dy3r = _mm_sub_epi32(x1i_2,_mm_add_epi32(x2r_2,x3i_2));
  dy3i = _mm_sub_epi32(_mm_sub_epi32(x3r_2,x2i_2),x1r_2);
  *(y3)  = _mm_add_epi16(*(x0),cpack(dy3r,dy3i));
  dy2r = _mm_sub_epi32(_mm_sub_epi32(x2r_2,x3r_2),x1r_2);
  dy2i = _mm_sub_epi32(_mm_sub_epi32(x2i_2,x3i_2),x1i_2);
  *(y2)  = _mm_add_epi16(*(x0),cpack(dy2r,dy2i));
  dy1r = _mm_sub_epi32(_mm_sub_epi32(x3i_2,x2r_2),x1i_2);
  dy1i = _mm_sub_epi32(x1r_2,_mm_add_epi32(x2i_2,x3r_2));
  *(y1) = _mm_add_epi16(*(x0),cpack(dy1r,dy1i));
}


static inline void bfly4_tw1(__m128i *x0,__m128i *x1,__m128i *x2,__m128i *x3,
			     __m128i *y0,__m128i *y1,__m128i *y2,__m128i *y3)__attribute__((always_inline));

static inline void bfly4_tw1(__m128i *x0,__m128i *x1,__m128i *x2,__m128i *x3,
			     __m128i *y0,__m128i *y1,__m128i *y2,__m128i *y3) { 

  register __m128i x1_flip,x3_flip;

  *(y0) = _mm_adds_epi16(*(x0),_mm_adds_epi16(*(x1),_mm_adds_epi16(*(x2),*(x3)))); 

  x1_flip = _mm_sign_epi16(*(x1),*(__m128i*)conjugatedft);
  //  x1_flip = _mm_shufflelo_epi16(x1_flip,_MM_SHUFFLE(2,3,0,1));
  //  x1_flip = _mm_shufflehi_epi16(x1_flip,_MM_SHUFFLE(2,3,0,1));
  x1_flip = _mm_shuffle_epi8(x1_flip,_mm_set_epi8(13,12,15,14,9,8,11,10,5,4,7,6,1,0,3,2));
  x3_flip = _mm_sign_epi16(*(x3),*(__m128i*)conjugatedft);
  //  x3_flip = _mm_shufflelo_epi16(x3_flip,_MM_SHUFFLE(2,3,0,1));
  //  x3_flip = _mm_shufflehi_epi16(x3_flip,_MM_SHUFFLE(2,3,0,1));
  x3_flip = _mm_shuffle_epi8(x3_flip,_mm_set_epi8(13,12,15,14,9,8,11,10,5,4,7,6,1,0,3,2));
  *(y1)   = _mm_adds_epi16(*(x0),_mm_subs_epi16(x1_flip,_mm_adds_epi16(*(x2),x3_flip)));
  *(y2)   = _mm_subs_epi16(*(x0),_mm_subs_epi16(*(x1),_mm_subs_epi16(*(x2),*(x3))));
  *(y3)   = _mm_subs_epi16(*(x0),_mm_adds_epi16(x1_flip,_mm_subs_epi16(*(x2),x3_flip)));
}

static inline void ibfly4_tw1(__m128i *x0,__m128i *x1,__m128i *x2,__m128i *x3,
			      __m128i *y0,__m128i *y1,__m128i *y2,__m128i *y3)__attribute__((always_inline)); 

static inline void ibfly4_tw1(__m128i *x0,__m128i *x1,__m128i *x2,__m128i *x3,
			      __m128i *y0,__m128i *y1,__m128i *y2,__m128i *y3) { 
  
  register __m128i x1_flip,x3_flip;

  *(y0) = _mm_adds_epi16(*(x0),_mm_adds_epi16(*(x1),_mm_adds_epi16(*(x2),*(x3)))); 

  x1_flip = _mm_sign_epi16(*(x1),*(__m128i*)conjugatedft);
  //  x1_flip = _mm_shufflelo_epi16(x1_flip,_MM_SHUFFLE(2,3,0,1));
  //  x1_flip = _mm_shufflehi_epi16(x1_flip,_MM_SHUFFLE(2,3,0,1));
  x1_flip = _mm_shuffle_epi8(x1_flip,_mm_set_epi8(13,12,15,14,9,8,11,10,5,4,7,6,1,0,3,2));
  x3_flip = _mm_sign_epi16(*(x3),*(__m128i*)conjugatedft);
  //  x3_flip = _mm_shufflelo_epi16(x3_flip,_MM_SHUFFLE(2,3,0,1));
  //  x3_flip = _mm_shufflehi_epi16(x3_flip,_MM_SHUFFLE(2,3,0,1));
  x3_flip = _mm_shuffle_epi8(x3_flip,_mm_set_epi8(13,12,15,14,9,8,11,10,5,4,7,6,1,0,3,2));
  *(y1)   = _mm_subs_epi16(*(x0),_mm_adds_epi16(x1_flip,_mm_subs_epi16(*(x2),x3_flip)));
  *(y2)   = _mm_subs_epi16(*(x0),_mm_subs_epi16(*(x1),_mm_subs_epi16(*(x2),*(x3))));
  *(y3)   = _mm_adds_epi16(*(x0),_mm_subs_epi16(x1_flip,_mm_adds_epi16(*(x2),x3_flip)));
}

static inline void bfly4_16(__m128i *x0,__m128i *x1,__m128i *x2,__m128i *x3,
			    __m128i *y0,__m128i *y1,__m128i *y2,__m128i *y3,
			    __m128i *tw1,__m128i *tw2,__m128i *tw3,
			    __m128i *tw1b,__m128i *tw2b,__m128i *tw3b)__attribute__((always_inline));

static inline void bfly4_16(__m128i *x0,__m128i *x1,__m128i *x2,__m128i *x3,
			    __m128i *y0,__m128i *y1,__m128i *y2,__m128i *y3,
			    __m128i *tw1,__m128i *tw2,__m128i *tw3,
			    __m128i *tw1b,__m128i *tw2b,__m128i *tw3b) {   

  register __m128i x1t,x2t,x3t,x02t,x13t;
  register __m128i x1_flip,x3_flip;

  x1t = packed_cmult2(*(x1),*(tw1),*(tw1b));
  x2t = packed_cmult2(*(x2),*(tw2),*(tw2b));
  x3t = packed_cmult2(*(x3),*(tw3),*(tw3b));


  //  bfly4_tw1(x0,&x1t,&x2t,&x3t,y0,y1,y2,y3);
  x02t  = _mm_adds_epi16(*(x0),x2t);
  x13t  = _mm_adds_epi16(x1t,x3t);
  /*
  *(y0) = _mm_adds_epi16(*(x0),_mm_adds_epi16(x1t,_mm_adds_epi16(x2t,x3t))); 
  *(y2)   = _mm_subs_epi16(*(x0),_mm_subs_epi16(x1t,_mm_subs_epi16(x2t,x3t)));
  */
  *(y0)   = _mm_adds_epi16(x02t,x13t);
  *(y2)   = _mm_subs_epi16(x02t,x13t);

  x1_flip = _mm_sign_epi16(x1t,*(__m128i*)conjugatedft);
  //  x1_flip = _mm_shufflelo_epi16(x1_flip,_MM_SHUFFLE(2,3,0,1));
  //  x1_flip = _mm_shufflehi_epi16(x1_flip,_MM_SHUFFLE(2,3,0,1));
  x1_flip = _mm_shuffle_epi8(x1_flip,_mm_set_epi8(13,12,15,14,9,8,11,10,5,4,7,6,1,0,3,2));
  x3_flip = _mm_sign_epi16(x3t,*(__m128i*)conjugatedft);
  //  x3_flip = _mm_shufflelo_epi16(x3_flip,_MM_SHUFFLE(2,3,0,1));
  //  x3_flip = _mm_shufflehi_epi16(x3_flip,_MM_SHUFFLE(2,3,0,1));
  x3_flip = _mm_shuffle_epi8(x3_flip,_mm_set_epi8(13,12,15,14,9,8,11,10,5,4,7,6,1,0,3,2));
  x02t  = _mm_subs_epi16(*(x0),x2t);
  x13t  = _mm_subs_epi16(x1_flip,x3_flip);
  /*
  *(y1)   = _mm_adds_epi16(*(x0),_mm_subs_epi16(x1_flip,_mm_adds_epi16(x2t,x3_flip)));  // x0 + x1f - x2 - x3f
  *(y3)   = _mm_subs_epi16(*(x0),_mm_adds_epi16(x1_flip,_mm_subs_epi16(x2t,x3_flip)));  // x0 - x1f - x2 + x3f
  */
  *(y1)   = _mm_adds_epi16(x02t,x13t);  // x0 + x1f - x2 - x3f
  *(y3)   = _mm_subs_epi16(x02t,x13t);  // x0 - x1f - x2 + x3f

}

static inline void ibfly4_16(__m128i *x0,__m128i *x1,__m128i *x2,__m128i *x3,
			     __m128i *y0,__m128i *y1,__m128i *y2,__m128i *y3,
			     __m128i *tw1,__m128i *tw2,__m128i *tw3,
			     __m128i *tw1b,__m128i *tw2b,__m128i *tw3b)__attribute__((always_inline));

static inline void ibfly4_16(__m128i *x0,__m128i *x1,__m128i *x2,__m128i *x3,
			     __m128i *y0,__m128i *y1,__m128i *y2,__m128i *y3,
			     __m128i *tw1,__m128i *tw2,__m128i *tw3,
			     __m128i *tw1b,__m128i *tw2b,__m128i *tw3b) {   
  
  register __m128i x1t,x2t,x3t,x02t,x13t;
  register __m128i x1_flip,x3_flip;

  x1t = packed_cmult2(*(x1),*(tw1),*(tw1b));
  x2t = packed_cmult2(*(x2),*(tw2),*(tw2b));
  x3t = packed_cmult2(*(x3),*(tw3),*(tw3b));


  //  bfly4_tw1(x0,&x1t,&x2t,&x3t,y0,y1,y2,y3);
  x02t  = _mm_adds_epi16(*(x0),x2t);
  x13t  = _mm_adds_epi16(x1t,x3t);
  /*
  *(y0) = _mm_adds_epi16(*(x0),_mm_adds_epi16(x1t,_mm_adds_epi16(x2t,x3t))); 
  *(y2)   = _mm_subs_epi16(*(x0),_mm_subs_epi16(x1t,_mm_subs_epi16(x2t,x3t)));
  */
  *(y0)   = _mm_adds_epi16(x02t,x13t);
  *(y2)   = _mm_subs_epi16(x02t,x13t);

  x1_flip = _mm_sign_epi16(x1t,*(__m128i*)conjugatedft);
  //  x1_flip = _mm_shufflelo_epi16(x1_flip,_MM_SHUFFLE(2,3,0,1));
  //  x1_flip = _mm_shufflehi_epi16(x1_flip,_MM_SHUFFLE(2,3,0,1));
  x1_flip = _mm_shuffle_epi8(x1_flip,_mm_set_epi8(13,12,15,14,9,8,11,10,5,4,7,6,1,0,3,2));
  x3_flip = _mm_sign_epi16(x3t,*(__m128i*)conjugatedft);
  //  x3_flip = _mm_shufflelo_epi16(x3_flip,_MM_SHUFFLE(2,3,0,1));
  //  x3_flip = _mm_shufflehi_epi16(x3_flip,_MM_SHUFFLE(2,3,0,1));
  x3_flip = _mm_shuffle_epi8(x3_flip,_mm_set_epi8(13,12,15,14,9,8,11,10,5,4,7,6,1,0,3,2));
  x02t  = _mm_subs_epi16(*(x0),x2t);
  x13t  = _mm_subs_epi16(x1_flip,x3_flip);
  /*
  *(y1)   = _mm_adds_epi16(*(x0),_mm_subs_epi16(x1_flip,_mm_adds_epi16(x2t,x3_flip)));  // x0 + x1f - x2 - x3f
  *(y3)   = _mm_subs_epi16(*(x0),_mm_adds_epi16(x1_flip,_mm_subs_epi16(x2t,x3_flip)));  // x0 - x1f - x2 + x3f
  */
  *(y3)   = _mm_adds_epi16(x02t,x13t);  // x0 + x1f - x2 - x3f
  *(y1)   = _mm_subs_epi16(x02t,x13t);  // x0 - x1f - x2 + x3f

}

static inline void bfly5(__m128i *x0, __m128i *x1, __m128i *x2, __m128i *x3,__m128i *x4,
			 __m128i *y0, __m128i *y1, __m128i *y2, __m128i *y3,__m128i *y4,
			 __m128i *tw1,__m128i *tw2,__m128i *tw3,__m128i *tw4)__attribute__((always_inline));

static inline void bfly5(__m128i *x0, __m128i *x1, __m128i *x2, __m128i *x3,__m128i *x4,
			 __m128i *y0, __m128i *y1, __m128i *y2, __m128i *y3,__m128i *y4,
			 __m128i *tw1,__m128i *tw2,__m128i *tw3,__m128i *tw4) {



  __m128i x1_2,x2_2,x3_2,x4_2,tmpre,tmpim;

  packed_cmult(*(x1),*(tw1),&x1_2);
  packed_cmult(*(x2),*(tw2),&x2_2);
  packed_cmult(*(x3),*(tw3),&x3_2);
  packed_cmult(*(x4),*(tw4),&x4_2);

  *(y0)  = _mm_adds_epi16(*(x0),_mm_adds_epi16(x1_2,_mm_adds_epi16(x2_2,_mm_adds_epi16(x3_2,x4_2))));
  cmult(x1_2,*(W15),&tmpre,&tmpim); 
  cmac(x2_2,*(W25),&tmpre,&tmpim);  
  cmac(x3_2,*(W35),&tmpre,&tmpim);  
  cmac(x4_2,*(W45),&tmpre,&tmpim);  
  *(y1) = cpack(tmpre,tmpim); 
  *(y1) = _mm_adds_epi16(*(x0),*(y1));

  cmult(x1_2,*(W25),&tmpre,&tmpim); 
  cmac(x2_2,*(W45),&tmpre,&tmpim);  
  cmac(x3_2,*(W15),&tmpre,&tmpim);  
  cmac(x4_2,*(W35),&tmpre,&tmpim);  
  *(y2) = cpack(tmpre,tmpim); 
  *(y2) = _mm_adds_epi16(*(x0),*(y2));

  cmult(x1_2,*(W35),&tmpre,&tmpim); 
  cmac(x2_2,*(W15),&tmpre,&tmpim);  
  cmac(x3_2,*(W45),&tmpre,&tmpim);  
  cmac(x4_2,*(W25),&tmpre,&tmpim);  
  *(y3) = cpack(tmpre,tmpim); 
  *(y3) = _mm_adds_epi16(*(x0),*(y3));

  cmult(x1_2,*(W45),&tmpre,&tmpim); 
  cmac(x2_2,*(W35),&tmpre,&tmpim);  
  cmac(x3_2,*(W25),&tmpre,&tmpim);  
  cmac(x4_2,*(W15),&tmpre,&tmpim);  
  *(y4) = cpack(tmpre,tmpim); 
  *(y4) = _mm_adds_epi16(*(x0),*(y4));


}  



static inline void bfly5_tw1(__m128i *x0, __m128i *x1, __m128i *x2, __m128i *x3,__m128i *x4,
			     __m128i *y0, __m128i *y1, __m128i *y2, __m128i *y3,__m128i *y4) __attribute__((always_inline));

static inline void bfly5_tw1(__m128i *x0, __m128i *x1, __m128i *x2, __m128i *x3,__m128i *x4,
			     __m128i *y0, __m128i *y1, __m128i *y2, __m128i *y3,__m128i *y4) {

  __m128i tmpre,tmpim;

  *(y0) = _mm_adds_epi16(*(x0),_mm_adds_epi16(*(x1),_mm_adds_epi16(*(x2),_mm_adds_epi16(*(x3),*(x4))))); 
  cmult(*(x1),*(W15),&tmpre,&tmpim);   
  cmac(*(x2),*(W25),&tmpre,&tmpim);  
  cmac(*(x3),*(W35),&tmpre,&tmpim);  
  cmac(*(x4),*(W45),&tmpre,&tmpim);  
  *(y1) = cpack(tmpre,tmpim); 
  *(y1) = _mm_adds_epi16(*(x0),*(y1));
  cmult(*(x1),*(W25),&tmpre,&tmpim); 
  cmac(*(x2),*(W45),&tmpre,&tmpim);  
  cmac(*(x3),*(W15),&tmpre,&tmpim);  
  cmac(*(x4),*(W35),&tmpre,&tmpim);  
  *(y2) = cpack(tmpre,tmpim); 
  *(y2) = _mm_adds_epi16(*(x0),*(y2));
  cmult(*(x1),*(W35),&tmpre,&tmpim); 
  cmac(*(x2),*(W15),&tmpre,&tmpim);  
  cmac(*(x3),*(W45),&tmpre,&tmpim);  
  cmac(*(x4),*(W25),&tmpre,&tmpim);  
  *(y3) = cpack(tmpre,tmpim); 
  *(y3) = _mm_adds_epi16(*(x0),*(y3));
  cmult(*(x1),*(W45),&tmpre,&tmpim); 
  cmac(*(x2),*(W35),&tmpre,&tmpim);  
  cmac(*(x3),*(W25),&tmpre,&tmpim);  
  cmac(*(x4),*(W15),&tmpre,&tmpim);  
  *(y4) = cpack(tmpre,tmpim); 
  *(y4) = _mm_adds_epi16(*(x0),*(y4));
}

// performs 4x4 transpose of input x (complex interleaved) using 128bit SIMD intrinsics
// i.e. x = [x0r x0i x1r x1i ... x15r x15i], y = [x0r x0i x4r x4i x8r x8i x12r x12i x1r x1i x5r x5i x9r x9i x13r x13i x2r x2i ... x15r x15i]

static inline void transpose16(__m128i *x,__m128i *y) __attribute__((always_inline));
static inline void transpose16(__m128i *x,__m128i *y) {
  register __m128i ytmp0,ytmp1,ytmp2,ytmp3;

  ytmp0 = _mm_unpacklo_epi32(x[0],x[1]);
  ytmp1 = _mm_unpackhi_epi32(x[0],x[1]);
  ytmp2 = _mm_unpacklo_epi32(x[2],x[3]);
  ytmp3 = _mm_unpackhi_epi32(x[2],x[3]);
  y[0]    = _mm_unpacklo_epi64(ytmp0,ytmp2);
  y[1]    = _mm_unpackhi_epi64(ytmp0,ytmp2);
  y[2]    = _mm_unpacklo_epi64(ytmp1,ytmp3);
  y[3]    = _mm_unpackhi_epi64(ytmp1,ytmp3);
}

// same as above but output is offset by off
static inline void transpose16_ooff(__m128i *x,__m128i *y,int off) __attribute__((always_inline));

static inline void transpose16_ooff(__m128i *x,__m128i *y,int off) {
  register __m128i ytmp0,ytmp1,ytmp2,ytmp3;
  __m128i *y2=y;

  ytmp0 = _mm_unpacklo_epi32(x[0],x[1]);
  ytmp1 = _mm_unpackhi_epi32(x[0],x[1]);
  ytmp2 = _mm_unpacklo_epi32(x[2],x[3]);
  ytmp3 = _mm_unpackhi_epi32(x[2],x[3]);
  *y2     = _mm_unpacklo_epi64(ytmp0,ytmp2);y2+=off;
  *y2     = _mm_unpackhi_epi64(ytmp0,ytmp2);y2+=off;
  *y2     = _mm_unpacklo_epi64(ytmp1,ytmp3);y2+=off;
  *y2     = _mm_unpackhi_epi64(ytmp1,ytmp3);
}

static inline void transpose4_ooff(__m64 *x,__m64 *y,int off)__attribute__((always_inline));
static inline void transpose4_ooff(__m64 *x,__m64 *y,int off) {
  y[0]   = _mm_unpacklo_pi32(x[0],x[1]);
  y[off] = _mm_unpackhi_pi32(x[0],x[1]);
}

// 16-point optimized DFT kernel

int16_t tw16[24] __attribute__((aligned(16))) = { 32767,0,30272,-12540,23169 ,-23170,12539 ,-30273,
						  32767,0,23169,-23170,0     ,-32767,-23170,-23170,
						  32767,0,12539,-30273,-23170,-23170,-30273,12539};

int16_t tw16a[24] __attribute__((aligned(16))) = {32767,0,30272,12540,23169 ,23170,12539 ,30273,
						  32767,0,23169,23170,0     ,32767,-23170,23170,
						  32767,0,12539,30273,-23170,23170,-30273,-12539};

int16_t tw16b[24] __attribute__((aligned(16))) = { 0,32767,-12540,30272,-23170,23169 ,-30273,12539,
						   0,32767,-23170,23169,-32767,0     ,-23170,-23170,
						   0,32767,-30273,12539,-23170,-23170,12539 ,-30273};

int16_t tw16c[24] __attribute__((aligned(16))) = { 0,32767,12540,30272,23170,23169 ,30273 ,12539,
						   0,32767,23170,23169,32767,0     ,23170 ,-23170,
						   0,32767,30273,12539,23170,-23170,-12539,-30273};

static inline void dft16(int16_t *x,int16_t *y) __attribute__((always_inline));

static inline void dft16(int16_t *x,int16_t *y) {

  __m128i *tw16a_128=(__m128i *)tw16a,*tw16b_128=(__m128i *)tw16b,*x128=(__m128i *)x,*y128=(__m128i *)y;
  
  /*
  bfly4_tw1(x128,x128+1,x128+2,x128+3,
	    y128,y128+1,y128+2,y128+3);

  transpose16(y128,ytmp);

  bfly4_16(ytmp,ytmp+1,ytmp+2,ytmp+3,
	   y128,y128+1,y128+2,y128+3,
	   tw16_128,tw16_128+1,tw16_128+2);
  */

  register __m128i x1_flip,x3_flip,x02t,x13t;
  register __m128i ytmp0,ytmp1,ytmp2,ytmp3,xtmp0,xtmp1,xtmp2,xtmp3;

  // First stage : 4 Radix-4 butterflies without input twiddles

  x02t    = _mm_adds_epi16(x128[0],x128[2]);
  x13t    = _mm_adds_epi16(x128[1],x128[3]);
  xtmp0   = _mm_adds_epi16(x02t,x13t);
  xtmp2   = _mm_subs_epi16(x02t,x13t);

  /*
  xtmp0   = _mm_adds_epi16(x128[0],_mm_adds_epi16(x128[1],_mm_adds_epi16(x128[2],x128[3]))); 
  xtmp2   = _mm_subs_epi16(x128[0],_mm_subs_epi16(x128[1],_mm_subs_epi16(x128[2],x128[3])));
  */
  x1_flip = _mm_sign_epi16(x128[1],*(__m128i*)conjugatedft);
  //  x1_flip = _mm_shufflelo_epi16(x1_flip,_MM_SHUFFLE(2,3,0,1));
  //  x1_flip = _mm_shufflehi_epi16(x1_flip,_MM_SHUFFLE(2,3,0,1));
  x1_flip = _mm_shuffle_epi8(x1_flip,_mm_set_epi8(13,12,15,14,9,8,11,10,5,4,7,6,1,0,3,2));
  x3_flip = _mm_sign_epi16(x128[3],*(__m128i*)conjugatedft);
  //  x3_flip = _mm_shufflelo_epi16(x3_flip,_MM_SHUFFLE(2,3,0,1));
  //  x3_flip = _mm_shufflehi_epi16(x3_flip,_MM_SHUFFLE(2,3,0,1));
  x3_flip = _mm_shuffle_epi8(x3_flip,_mm_set_epi8(13,12,15,14,9,8,11,10,5,4,7,6,1,0,3,2));
  x02t    = _mm_subs_epi16(x128[0],x128[2]);
  x13t    = _mm_subs_epi16(x1_flip,x3_flip);
  xtmp1   = _mm_adds_epi16(x02t,x13t);  // x0 + x1f - x2 - x3f
  xtmp3   = _mm_subs_epi16(x02t,x13t);  // x0 - x1f - x2 + x3f
  /*
  xtmp1   = _mm_adds_epi16(x128[0],_mm_subs_epi16(x1_flip,_mm_adds_epi16(x128[2],x3_flip)));
  xtmp3   = _mm_subs_epi16(x128[0],_mm_adds_epi16(x1_flip,_mm_subs_epi16(x128[2],x3_flip)));
  */

  ytmp0   = _mm_unpacklo_epi32(xtmp0,xtmp1);
  ytmp1   = _mm_unpackhi_epi32(xtmp0,xtmp1);
  ytmp2   = _mm_unpacklo_epi32(xtmp2,xtmp3);
  ytmp3   = _mm_unpackhi_epi32(xtmp2,xtmp3);
  xtmp0   = _mm_unpacklo_epi64(ytmp0,ytmp2);
  xtmp1   = _mm_unpackhi_epi64(ytmp0,ytmp2);
  xtmp2   = _mm_unpacklo_epi64(ytmp1,ytmp3);
  xtmp3   = _mm_unpackhi_epi64(ytmp1,ytmp3);

  // Second stage : 4 Radix-4 butterflies with input twiddles
  xtmp1 = packed_cmult2(xtmp1,tw16a_128[0],tw16b_128[0]);
  xtmp2 = packed_cmult2(xtmp2,tw16a_128[1],tw16b_128[1]);
  xtmp3 = packed_cmult2(xtmp3,tw16a_128[2],tw16b_128[2]);

  x02t    = _mm_adds_epi16(xtmp0,xtmp2);
  x13t    = _mm_adds_epi16(xtmp1,xtmp3);
  y128[0] = _mm_adds_epi16(x02t,x13t);
  y128[2] = _mm_subs_epi16(x02t,x13t);

  /*
  y128[0] = _mm_adds_epi16(xtmp0,_mm_adds_epi16(xtmp1,_mm_adds_epi16(xtmp2,xtmp3))); 
  y128[2] = _mm_subs_epi16(xtmp0,_mm_subs_epi16(xtmp1,_mm_subs_epi16(xtmp2,xtmp3)));
  */

  x1_flip = _mm_sign_epi16(xtmp1,*(__m128i*)conjugatedft);
  //  x1_flip = _mm_shufflelo_epi16(x1_flip,_MM_SHUFFLE(2,3,0,1));
  //  x1_flip = _mm_shufflehi_epi16(x1_flip,_MM_SHUFFLE(2,3,0,1));
  x1_flip = _mm_shuffle_epi8(x1_flip,_mm_set_epi8(13,12,15,14,9,8,11,10,5,4,7,6,1,0,3,2));
  x3_flip = _mm_sign_epi16(xtmp3,*(__m128i*)conjugatedft);
  //  x3_flip = _mm_shufflelo_epi16(x3_flip,_MM_SHUFFLE(2,3,0,1));
  //  x3_flip = _mm_shufflehi_epi16(x3_flip,_MM_SHUFFLE(2,3,0,1));
  x3_flip = _mm_shuffle_epi8(x3_flip,_mm_set_epi8(13,12,15,14,9,8,11,10,5,4,7,6,1,0,3,2));
  x02t    = _mm_subs_epi16(xtmp0,xtmp2);
  x13t    = _mm_subs_epi16(x1_flip,x3_flip);
  y128[1] = _mm_adds_epi16(x02t,x13t);  // x0 + x1f - x2 - x3f
  y128[3] = _mm_subs_epi16(x02t,x13t);  // x0 - x1f - x2 + x3f
  /*
  y128[1] = _mm_adds_epi16(xtmp0,_mm_subs_epi16(x1_flip,_mm_adds_epi16(xtmp2,x3_flip)));
  y128[3] = _mm_subs_epi16(xtmp0,_mm_adds_epi16(x1_flip,_mm_subs_epi16(xtmp2,x3_flip)));
  */
}

static inline void idft16(int16_t *x,int16_t *y) __attribute__((always_inline));

static inline void idft16(int16_t *x,int16_t *y) {

  __m128i *tw16a_128=(__m128i *)tw16,*tw16b_128=(__m128i *)tw16c,*x128=(__m128i *)x,*y128=(__m128i *)y;
  
  /*
  bfly4_tw1(x128,x128+1,x128+2,x128+3,
	    y128,y128+1,y128+2,y128+3);

  transpose16(y128,ytmp);

  bfly4_16(ytmp,ytmp+1,ytmp+2,ytmp+3,
	   y128,y128+1,y128+2,y128+3,
	   tw16_128,tw16_128+1,tw16_128+2);
  */

  register __m128i x1_flip,x3_flip,x02t,x13t;
  register __m128i ytmp0,ytmp1,ytmp2,ytmp3,xtmp0,xtmp1,xtmp2,xtmp3;

  // First stage : 4 Radix-4 butterflies without input twiddles

  x02t    = _mm_adds_epi16(x128[0],x128[2]);
  x13t    = _mm_adds_epi16(x128[1],x128[3]);
  xtmp0   = _mm_adds_epi16(x02t,x13t);
  xtmp2   = _mm_subs_epi16(x02t,x13t);

  /*
  xtmp0   = _mm_adds_epi16(x128[0],_mm_adds_epi16(x128[1],_mm_adds_epi16(x128[2],x128[3]))); 
  xtmp2   = _mm_subs_epi16(x128[0],_mm_subs_epi16(x128[1],_mm_subs_epi16(x128[2],x128[3])));
  */
  x1_flip = _mm_sign_epi16(x128[1],*(__m128i*)conjugatedft);
  //  x1_flip = _mm_shufflelo_epi16(x1_flip,_MM_SHUFFLE(2,3,0,1));
  //  x1_flip = _mm_shufflehi_epi16(x1_flip,_MM_SHUFFLE(2,3,0,1));
  x1_flip = _mm_shuffle_epi8(x1_flip,_mm_set_epi8(13,12,15,14,9,8,11,10,5,4,7,6,1,0,3,2));
  x3_flip = _mm_sign_epi16(x128[3],*(__m128i*)conjugatedft);
  //  x3_flip = _mm_shufflelo_epi16(x3_flip,_MM_SHUFFLE(2,3,0,1));
  //  x3_flip = _mm_shufflehi_epi16(x3_flip,_MM_SHUFFLE(2,3,0,1));
  x3_flip = _mm_shuffle_epi8(x3_flip,_mm_set_epi8(13,12,15,14,9,8,11,10,5,4,7,6,1,0,3,2));
  x02t    = _mm_subs_epi16(x128[0],x128[2]);
  x13t    = _mm_subs_epi16(x1_flip,x3_flip);
  xtmp3   = _mm_adds_epi16(x02t,x13t);  // x0 + x1f - x2 - x3f
  xtmp1   = _mm_subs_epi16(x02t,x13t);  // x0 - x1f - x2 + x3f
  /*
  xtmp1   = _mm_adds_epi16(x128[0],_mm_subs_epi16(x1_flip,_mm_adds_epi16(x128[2],x3_flip)));
  xtmp3   = _mm_subs_epi16(x128[0],_mm_adds_epi16(x1_flip,_mm_subs_epi16(x128[2],x3_flip)));
  */

  ytmp0   = _mm_unpacklo_epi32(xtmp0,xtmp1);
  ytmp1   = _mm_unpackhi_epi32(xtmp0,xtmp1);
  ytmp2   = _mm_unpacklo_epi32(xtmp2,xtmp3);
  ytmp3   = _mm_unpackhi_epi32(xtmp2,xtmp3);
  xtmp0   = _mm_unpacklo_epi64(ytmp0,ytmp2);
  xtmp1   = _mm_unpackhi_epi64(ytmp0,ytmp2);
  xtmp2   = _mm_unpacklo_epi64(ytmp1,ytmp3);
  xtmp3   = _mm_unpackhi_epi64(ytmp1,ytmp3);

  // Second stage : 4 Radix-4 butterflies with input twiddles
  xtmp1 = packed_cmult2(xtmp1,tw16a_128[0],tw16b_128[0]);
  xtmp2 = packed_cmult2(xtmp2,tw16a_128[1],tw16b_128[1]);
  xtmp3 = packed_cmult2(xtmp3,tw16a_128[2],tw16b_128[2]);

  x02t    = _mm_adds_epi16(xtmp0,xtmp2);
  x13t    = _mm_adds_epi16(xtmp1,xtmp3);
  y128[0] = _mm_adds_epi16(x02t,x13t);
  y128[2] = _mm_subs_epi16(x02t,x13t);

  /*
  y128[0] = _mm_adds_epi16(xtmp0,_mm_adds_epi16(xtmp1,_mm_adds_epi16(xtmp2,xtmp3))); 
  y128[2] = _mm_subs_epi16(xtmp0,_mm_subs_epi16(xtmp1,_mm_subs_epi16(xtmp2,xtmp3)));
  */

  x1_flip = _mm_sign_epi16(xtmp1,*(__m128i*)conjugatedft);
  //  x1_flip = _mm_shufflelo_epi16(x1_flip,_MM_SHUFFLE(2,3,0,1));
  //  x1_flip = _mm_shufflehi_epi16(x1_flip,_MM_SHUFFLE(2,3,0,1));
  x1_flip = _mm_shuffle_epi8(x1_flip,_mm_set_epi8(13,12,15,14,9,8,11,10,5,4,7,6,1,0,3,2));
  x3_flip = _mm_sign_epi16(xtmp3,*(__m128i*)conjugatedft);
  //  x3_flip = _mm_shufflelo_epi16(x3_flip,_MM_SHUFFLE(2,3,0,1));
  //  x3_flip = _mm_shufflehi_epi16(x3_flip,_MM_SHUFFLE(2,3,0,1));
  x3_flip = _mm_shuffle_epi8(x3_flip,_mm_set_epi8(13,12,15,14,9,8,11,10,5,4,7,6,1,0,3,2));
  x02t    = _mm_subs_epi16(xtmp0,xtmp2);
  x13t    = _mm_subs_epi16(x1_flip,x3_flip);
  y128[3] = _mm_adds_epi16(x02t,x13t);  // x0 + x1f - x2 - x3f
  y128[1] = _mm_subs_epi16(x02t,x13t);  // x0 - x1f - x2 + x3f
  /*
  y128[1] = _mm_adds_epi16(xtmp0,_mm_subs_epi16(x1_flip,_mm_adds_epi16(xtmp2,x3_flip)));
  y128[3] = _mm_subs_epi16(xtmp0,_mm_adds_epi16(x1_flip,_mm_subs_epi16(xtmp2,x3_flip)));
  */
}

/*
static inline void idft16(int16_t *x,int16_t *y)__attribute__((always_inline)); 

static inline void idft16(int16_t *x,int16_t *y) {

  __m128i ytmp[4],*tw16_128=(__m128i *)tw16,*x128=(__m128i *)x,*y128=(__m128i *)y;


  ibfly4_tw1(x128,x128+1,x128+2,x128+3,
	    y128,y128+1,y128+2,y128+3);

  transpose16(y128,ytmp);

  ibfly4(ytmp,ytmp+1,ytmp+2,ytmp+3,
	 y128,y128+1,y128+2,y128+3,
	 tw16_128,tw16_128+1,tw16_128+2);

}
*/



// 64-point optimized DFT kernel

int16_t tw64[96] __attribute__((aligned(16))) = { 32767,0,32609,-3212,32137,-6393,31356,-9512,30272,-12540,28897,-15447,27244,-18205,25329,-20788,23169,-23170,20787,-25330,18204,-27245,15446,-28898,12539,-30273,9511,-31357,6392,-32138,3211,-32610,
						  32767,0,32137,-6393,30272,-12540,27244,-18205,23169,-23170,18204,-27245,12539,-30273,6392,-32138,0,-32767,-6393,-32138,-12540,-30273,-18205,-27245,-23170,-23170,-27245,-18205,-30273,-12540,-32138,-6393,
						  32767,0,31356,-9512,27244,-18205,20787,-25330,12539,-30273,3211,-32610,-6393,-32138,-15447,-28898,-23170,-23170,-28898,-15447,-32138,-6393,-32610,3211,-30273,12539,-25330,20787,-18205,27244,-9512,31356};

int16_t tw64a[96] __attribute__((aligned(16))) = { 32767,0,32609,3212,32137,6393,31356,9512,30272,12540,28897,15447,27244,18205,25329,20788,23169,23170,20787,25330,18204,27245,15446,28898,12539,30273,9511,31357,6392,32138,3211,32610,
						   32767,0,32137,6393,30272,12540,27244,18205,23169,23170,18204,27245,12539,30273,6392,32138,0,32767,-6393,32138,-12540,30273,-18205,27245,-23170,23170,-27245,18205,-30273,12540,-32138,6393,
						   32767,0,31356,9512,27244,18205,20787,25330,12539,30273,3211,32610,-6393,32138,-15447,28898,-23170,23170,-28898,15447,-32138,6393,-32610,-3211,-30273,-12539,-25330,-20787,-18205,-27244,-9512,-31356};

int16_t tw64b[96] __attribute__((aligned(16))) = { 0,32767,-3212,32609,-6393,32137,-9512,31356,-12540,30272,-15447,28897,-18205,27244,-20788,25329,-23170,23169,-25330,20787,-27245,18204,-28898,15446,-30273,12539,-31357,9511,-32138,6392,-32610,3211,
						   0,32767,-6393,32137,-12540,30272,-18205,27244,-23170,23169,-27245,18204,-30273,12539,-32138,6392,-32767,0,-32138,-6393,-30273,-12540,-27245,-18205,-23170,-23170,-18205,-27245,-12540,-30273,-6393,-32138,
						   0,32767,-9512,31356,-18205,27244,-25330,20787,-30273,12539,-32610,3211,-32138,-6393,-28898,-15447,-23170,-23170,-15447,-28898,-6393,-32138,3211,-32610,12539,-30273,20787,-25330,27244,-18205,31356,-9512};

int16_t tw64c[96] __attribute__((aligned(16))) = { 0,32767,3212,32609,6393,32137,9512,31356,12540,30272,15447,28897,18205,27244,20788,25329,23170,23169,25330,20787,27245,18204,28898,15446,30273,12539,31357,9511,32138,6392,32610,3211,
						   0,32767,6393,32137,12540,30272,18205,27244,23170,23169,27245,18204,30273,12539,32138,6392,32767,0,32138,-6393,30273,-12540,27245,-18205,23170,-23170,18205,-27245,12540,-30273,6393,-32138,
						   0,32767,9512,31356,18205,27244,25330,20787,30273,12539,32610,3211,32138,-6393,28898,-15447,23170,-23170,15447,-28898,6393,-32138,-3211,-32610,-12539,-30273,-20787,-25330,-27244,-18205,-31356,-9512};



void dft64(int16_t *x,int16_t *y,int scale) {

  __m128i xtmp[16],ytmp[16],*tw64a_128=(__m128i *)tw64a,*tw64b_128=(__m128i *)tw64b,*x128=(__m128i *)x,*y128=(__m128i *)y;

  
#ifdef D64STATS
  time_stats_t ts_t,ts_d,ts_b;

  reset_meas(&ts_t);
  reset_meas(&ts_d);
  reset_meas(&ts_b);
  start_meas(&ts_t);
#endif
  

  transpose16_ooff(x128,xtmp,4);
  transpose16_ooff(x128+4,xtmp+1,4);
  transpose16_ooff(x128+8,xtmp+2,4);
  transpose16_ooff(x128+12,xtmp+3,4);

  
#ifdef D64STATS
  stop_meas(&ts_t);
  start_meas(&ts_d);
#endif
  

  dft16((int16_t*)(xtmp),(int16_t*)ytmp);
  dft16((int16_t*)(xtmp+4),(int16_t*)(ytmp+4));
  dft16((int16_t*)(xtmp+8),(int16_t*)(ytmp+8));
  dft16((int16_t*)(xtmp+12),(int16_t*)(ytmp+12));

    
#ifdef D64STATS
  stop_meas(&ts_d);
  start_meas(&ts_b);
#endif
  

  bfly4_16(ytmp,ytmp+4,ytmp+8,ytmp+12,
	   y128,y128+4,y128+8,y128+12,
	   tw64a_128,tw64a_128+4,tw64a_128+8,
	   tw64b_128,tw64b_128+4,tw64b_128+8);

  bfly4_16(ytmp+1,ytmp+5,ytmp+9,ytmp+13,
	   y128+1,y128+5,y128+9,y128+13,
	   tw64a_128+1,tw64a_128+5,tw64a_128+9,
	   tw64b_128+1,tw64b_128+5,tw64b_128+9);

  bfly4_16(ytmp+2,ytmp+6,ytmp+10,ytmp+14,
	   y128+2,y128+6,y128+10,y128+14,
	   tw64a_128+2,tw64a_128+6,tw64a_128+10,
	   tw64b_128+2,tw64b_128+6,tw64b_128+10);

  bfly4_16(ytmp+3,ytmp+7,ytmp+11,ytmp+15,
	   y128+3,y128+7,y128+11,y128+15,
	   tw64a_128+3,tw64a_128+7,tw64a_128+11,
	   tw64b_128+3,tw64b_128+7,tw64b_128+11);
   
#ifdef D64STATS
  stop_meas(&ts_b);
  printf("t: %llu cycles, d: %llu cycles, b: %llu cycles\n",ts_t.diff,ts_d.diff,ts_b.diff);
#endif
  

  if (scale>0) {

    y128[0]  = _mm_srai_epi16(y128[0],3);
    y128[1]  = _mm_srai_epi16(y128[1],3);
    y128[2]  = _mm_srai_epi16(y128[2],3);
    y128[3]  = _mm_srai_epi16(y128[3],3);
    y128[4]  = _mm_srai_epi16(y128[4],3);
    y128[5]  = _mm_srai_epi16(y128[5],3);
    y128[6]  = _mm_srai_epi16(y128[6],3);
    y128[7]  = _mm_srai_epi16(y128[7],3);
    y128[8]  = _mm_srai_epi16(y128[8],3);
    y128[9]  = _mm_srai_epi16(y128[9],3);
    y128[10] = _mm_srai_epi16(y128[10],3);
    y128[11] = _mm_srai_epi16(y128[11],3);
    y128[12] = _mm_srai_epi16(y128[12],3);
    y128[13] = _mm_srai_epi16(y128[13],3);
    y128[14] = _mm_srai_epi16(y128[14],3);
    y128[15] = _mm_srai_epi16(y128[15],3);

  }
  _mm_empty();
  _m_empty();

}

void idft64(int16_t *x,int16_t *y,int scale) {

  __m128i xtmp[16],ytmp[16],*tw64a_128=(__m128i *)tw64,*tw64b_128=(__m128i *)tw64c,*x128=(__m128i *)x,*y128=(__m128i *)y;

  
#ifdef D64STATS
  time_stats_t ts_t,ts_d,ts_b;

  reset_meas(&ts_t);
  reset_meas(&ts_d);
  reset_meas(&ts_b);
  start_meas(&ts_t);
#endif
  

  transpose16_ooff(x128,xtmp,4);
  transpose16_ooff(x128+4,xtmp+1,4);
  transpose16_ooff(x128+8,xtmp+2,4);
  transpose16_ooff(x128+12,xtmp+3,4);

  
#ifdef D64STATS
  stop_meas(&ts_t);
  start_meas(&ts_d);
#endif
  

  idft16((int16_t*)(xtmp),(int16_t*)ytmp);
  idft16((int16_t*)(xtmp+4),(int16_t*)(ytmp+4));
  idft16((int16_t*)(xtmp+8),(int16_t*)(ytmp+8));
  idft16((int16_t*)(xtmp+12),(int16_t*)(ytmp+12));

    
#ifdef D64STATS
  stop_meas(&ts_d);
  start_meas(&ts_b);
#endif
  

  ibfly4_16(ytmp,ytmp+4,ytmp+8,ytmp+12,
	    y128,y128+4,y128+8,y128+12,
	    tw64a_128,tw64a_128+4,tw64a_128+8,
	    tw64b_128,tw64b_128+4,tw64b_128+8);

  ibfly4_16(ytmp+1,ytmp+5,ytmp+9,ytmp+13,
	    y128+1,y128+5,y128+9,y128+13,
	    tw64a_128+1,tw64a_128+5,tw64a_128+9,
	    tw64b_128+1,tw64b_128+5,tw64b_128+9);
  
  ibfly4_16(ytmp+2,ytmp+6,ytmp+10,ytmp+14,
	    y128+2,y128+6,y128+10,y128+14,
	    tw64a_128+2,tw64a_128+6,tw64a_128+10,
	    tw64b_128+2,tw64b_128+6,tw64b_128+10);
  
  ibfly4_16(ytmp+3,ytmp+7,ytmp+11,ytmp+15,
	    y128+3,y128+7,y128+11,y128+15,
	    tw64a_128+3,tw64a_128+7,tw64a_128+11,
	    tw64b_128+3,tw64b_128+7,tw64b_128+11);
   
#ifdef D64STATS
  stop_meas(&ts_b);
  printf("t: %llu cycles, d: %llu cycles, b: %llu cycles\n",ts_t.diff,ts_d.diff,ts_b.diff);
#endif
  

  if (scale>0) {

    y128[0]  = _mm_srai_epi16(y128[0],3);
    y128[1]  = _mm_srai_epi16(y128[1],3);
    y128[2]  = _mm_srai_epi16(y128[2],3);
    y128[3]  = _mm_srai_epi16(y128[3],3);
    y128[4]  = _mm_srai_epi16(y128[4],3);
    y128[5]  = _mm_srai_epi16(y128[5],3);
    y128[6]  = _mm_srai_epi16(y128[6],3);
    y128[7]  = _mm_srai_epi16(y128[7],3);
    y128[8]  = _mm_srai_epi16(y128[8],3);
    y128[9]  = _mm_srai_epi16(y128[9],3);
    y128[10] = _mm_srai_epi16(y128[10],3);
    y128[11] = _mm_srai_epi16(y128[11],3);
    y128[12] = _mm_srai_epi16(y128[12],3);
    y128[13] = _mm_srai_epi16(y128[13],3);
    y128[14] = _mm_srai_epi16(y128[14],3);
    y128[15] = _mm_srai_epi16(y128[15],3);

  }
  _mm_empty();
  _m_empty();

}


/*
void idft64(int16_t *x,int16_t *y,int scale) {

  __m128i xtmp[16],ytmp[16],*tw64_128=(__m128i *)tw64,*x128=(__m128i *)x,*y128=(__m128i *)y;

  transpose16_ooff(x128,xtmp,4);
  transpose16_ooff(x128+4,xtmp+1,4);
  transpose16_ooff(x128+8,xtmp+2,4);
  transpose16_ooff(x128+12,xtmp+3,4);
  
  idft16((int16_t*)(xtmp),(int16_t*)ytmp);
  idft16((int16_t*)(xtmp+4),(int16_t*)(ytmp+4));
  idft16((int16_t*)(xtmp+8),(int16_t*)(ytmp+8));
  idft16((int16_t*)(xtmp+12),(int16_t*)(ytmp+12));
  

  ibfly4(ytmp,ytmp+4,ytmp+8,ytmp+12,
	 y128,y128+4,y128+8,y128+12,
	 tw64_128,tw64_128+4,tw64_128+8);

  ibfly4(ytmp+1,ytmp+5,ytmp+9,ytmp+13,
	 y128+1,y128+5,y128+9,y128+13,
	 tw64_128+1,tw64_128+5,tw64_128+9);

  ibfly4(ytmp+2,ytmp+6,ytmp+10,ytmp+14,
	 y128+2,y128+6,y128+10,y128+14,
	 tw64_128+2,tw64_128+6,tw64_128+10);

  ibfly4(ytmp+3,ytmp+7,ytmp+11,ytmp+15,
	 y128+3,y128+7,y128+11,y128+15,
	 tw64_128+3,tw64_128+7,tw64_128+11);

  if (scale>0) {

    y128[0]  = _mm_srai_epi16(y128[0],3);
    y128[1]  = _mm_srai_epi16(y128[1],3);
    y128[2]  = _mm_srai_epi16(y128[2],3);
    y128[3]  = _mm_srai_epi16(y128[3],3);
    y128[4]  = _mm_srai_epi16(y128[4],3);
    y128[5]  = _mm_srai_epi16(y128[5],3);
    y128[6]  = _mm_srai_epi16(y128[6],3);
    y128[7]  = _mm_srai_epi16(y128[7],3);
    y128[8]  = _mm_srai_epi16(y128[8],3);
    y128[9]  = _mm_srai_epi16(y128[9],3);
    y128[10] = _mm_srai_epi16(y128[10],3);
    y128[11] = _mm_srai_epi16(y128[11],3);
    y128[12] = _mm_srai_epi16(y128[12],3);
    y128[13] = _mm_srai_epi16(y128[13],3);
    y128[14] = _mm_srai_epi16(y128[14],3);
    y128[15] = _mm_srai_epi16(y128[15],3);

  }
  _mm_empty();
  _m_empty();

}
*/

int16_t tw128[128] __attribute__((aligned(16))) = {  32767,0,32727,-1608,32609,-3212,32412,-4808,32137,-6393,31785,-7962,31356,-9512,30851,-11039,30272,-12540,29621,-14010,28897,-15447,28105,-16846,27244,-18205,26318,-19520,25329,-20788,24278,-22005,23169,-23170,22004,-24279,20787,-25330,19519,-26319,18204,-27245,16845,-28106,15446,-28898,14009,-29622,12539,-30273,11038,-30852,9511,-31357,7961,-31786,6392,-32138,4807,-32413,3211,-32610,1607,-32728,0,-32767,-1608,-32728,-3212,-32610,-4808,-32413,-6393,-32138,-7962,-31786,-9512,-31357,-11039,-30852,-12540,-30273,-14010,-29622,-15447,-28898,-16846,-28106,-18205,-27245,-19520,-26319,-20788,-25330,-22005,-24279,-23170,-23170,-24279,-22005,-25330,-20788,-26319,-19520,-27245,-18205,-28106,-16846,-28898,-15447,-29622,-14010,-30273,-12540,-30852,-11039,-31357,-9512,-31786,-7962,-32138,-6393,-32413,-4808,-32610,-3212,-32728,-1608};

int16_t tw128a[128] __attribute__((aligned(16))) = { 32767,0,32727,1608,32609,3212,32412,4808,32137,6393,31785,7962,31356,9512,30851,11039,30272,12540,29621,14010,28897,15447,28105,16846,27244,18205,26318,19520,25329,20788,24278,22005,23169,23170,22004,24279,20787,25330,19519,26319,18204,27245,16845,28106,15446,28898,14009,29622,12539,30273,11038,30852,9511,31357,7961,31786,6392,32138,4807,32413,3211,32610,1607,32728,0,32767,-1608,32728,-3212,32610,-4808,32413,-6393,32138,-7962,31786,-9512,31357,-11039,30852,-12540,30273,-14010,29622,-15447,28898,-16846,28106,-18205,27245,-19520,26319,-20788,25330,-22005,24279,-23170,23170,-24279,22005,-25330,20788,-26319,19520,-27245,18205,-28106,16846,-28898,15447,-29622,14010,-30273,12540,-30852,11039,-31357,9512,-31786,7962,-32138,6393,-32413,4808,-32610,3212,-32728,1608};

int16_t tw128b[128] __attribute__((aligned(16))) = {0,32767,-1608,32727,-3212,32609,-4808,32412,-6393,32137,-7962,31785,-9512,31356,-11039,30851,-12540,30272,-14010,29621,-15447,28897,-16846,28105,-18205,27244,-19520,26318,-20788,25329,-22005,24278,-23170,23169,-24279,22004,-25330,20787,-26319,19519,-27245,18204,-28106,16845,-28898,15446,-29622,14009,-30273,12539,-30852,11038,-31357,9511,-31786,7961,-32138,6392,-32413,4807,-32610,3211,-32728,1607,-32767,0,-32728,-1608,-32610,-3212,-32413,-4808,-32138,-6393,-31786,-7962,-31357,-9512,-30852,-11039,-30273,-12540,-29622,-14010,-28898,-15447,-28106,-16846,-27245,-18205,-26319,-19520,-25330,-20788,-24279,-22005,-23170,-23170,-22005,-24279,-20788,-25330,-19520,-26319,-18205,-27245,-16846,-28106,-15447,-28898,-14010,-29622,-12540,-30273,-11039,-30852,-9512,-31357,-7962,-31786,-6393,-32138,-4808,-32413,-3212,-32610,-1608,-32728};

int16_t tw128c[128] __attribute__((aligned(16))) = {0,32767,1608,32727,3212,32609,4808,32412,6393,32137,7962,31785,9512,31356,11039,30851,12540,30272,14010,29621,15447,28897,16846,28105,18205,27244,19520,26318,20788,25329,22005,24278,23170,23169,24279,22004,25330,20787,26319,19519,27245,18204,28106,16845,28898,15446,29622,14009,30273,12539,30852,11038,31357,9511,31786,7961,32138,6392,32413,4807,32610,3211,32728,1607,32767,0,32728,-1608,32610,-3212,32413,-4808,32138,-6393,31786,-7962,31357,-9512,30852,-11039,30273,-12540,29622,-14010,28898,-15447,28106,-16846,27245,-18205,26319,-19520,25330,-20788,24279,-22005,23170,-23170,22005,-24279,20788,-25330,19520,-26319,18205,-27245,16846,-28106,15447,-28898,14010,-29622,12540,-30273,11039,-30852,9512,-31357,7962,-31786,6393,-32138,4808,-32413,3212,-32610,1608,-32728};

void dft128(int16_t *x,int16_t *y,int scale) {

  __m64 xtmp[64],*x64 = (__m64 *)x;
  __m128i ytmp[32],*tw128a_128p=(__m128i *)tw128a,*tw128b_128p=(__m128i *)tw128b,*y128=(__m128i *)y,*y128p=(__m128i *)y;
  __m128i *ytmpp = &ytmp[0];
  int i;
  __m128i ONE_OVER_SQRT2_Q15_128 = _mm_set_epi16(ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15);

  transpose4_ooff(x64  ,xtmp,32);
  transpose4_ooff(x64+2,xtmp+1,32);
  transpose4_ooff(x64+4,xtmp+2,32);
  transpose4_ooff(x64+6,xtmp+3,32);
  transpose4_ooff(x64+8,xtmp+4,32);
  transpose4_ooff(x64+10,xtmp+5,32);
  transpose4_ooff(x64+12,xtmp+6,32);
  transpose4_ooff(x64+14,xtmp+7,32);
  transpose4_ooff(x64+16,xtmp+8,32);
  transpose4_ooff(x64+18,xtmp+9,32);
  transpose4_ooff(x64+20,xtmp+10,32);
  transpose4_ooff(x64+22,xtmp+11,32);
  transpose4_ooff(x64+24,xtmp+12,32);
  transpose4_ooff(x64+26,xtmp+13,32);
  transpose4_ooff(x64+28,xtmp+14,32);
  transpose4_ooff(x64+30,xtmp+15,32);
  transpose4_ooff(x64+32,xtmp+16,32);
  transpose4_ooff(x64+34,xtmp+17,32);
  transpose4_ooff(x64+36,xtmp+18,32);
  transpose4_ooff(x64+38,xtmp+19,32);
  transpose4_ooff(x64+40,xtmp+20,32);
  transpose4_ooff(x64+42,xtmp+21,32);
  transpose4_ooff(x64+44,xtmp+22,32);
  transpose4_ooff(x64+46,xtmp+23,32);
  transpose4_ooff(x64+48,xtmp+24,32);
  transpose4_ooff(x64+50,xtmp+25,32);
  transpose4_ooff(x64+52,xtmp+26,32);
  transpose4_ooff(x64+54,xtmp+27,32);
  transpose4_ooff(x64+56,xtmp+28,32);
  transpose4_ooff(x64+58,xtmp+29,32);
  transpose4_ooff(x64+60,xtmp+30,32);
  transpose4_ooff(x64+62,xtmp+31,32);

  dft64((int16_t*)(xtmp),(int16_t*)ytmp,1);
  dft64((int16_t*)(xtmp+32),(int16_t*)(ytmp+16),1);


  for (i=0;i<16;i++) {
    bfly2_16(ytmpp,ytmpp+16,
	     y128p,y128p+16,
	     tw128a_128p,
	     tw128b_128p);
    tw128a_128p++;
    tw128b_128p++;
    y128p++;
    ytmpp++;    
  }

  if (scale>0) {

    y128[0]  = _mm_mulhi_epi16(y128[0],ONE_OVER_SQRT2_Q15_128);y128[0] = _mm_slli_epi16(y128[0],1);
    y128[1]  = _mm_mulhi_epi16(y128[1],ONE_OVER_SQRT2_Q15_128);y128[1] = _mm_slli_epi16(y128[1],1);
    y128[2]  = _mm_mulhi_epi16(y128[2],ONE_OVER_SQRT2_Q15_128);y128[2] = _mm_slli_epi16(y128[2],1);
    y128[3]  = _mm_mulhi_epi16(y128[3],ONE_OVER_SQRT2_Q15_128);y128[3] = _mm_slli_epi16(y128[3],1);
    y128[4]  = _mm_mulhi_epi16(y128[4],ONE_OVER_SQRT2_Q15_128);y128[4] = _mm_slli_epi16(y128[4],1);
    y128[5]  = _mm_mulhi_epi16(y128[5],ONE_OVER_SQRT2_Q15_128);y128[5] = _mm_slli_epi16(y128[5],1);
    y128[6]  = _mm_mulhi_epi16(y128[6],ONE_OVER_SQRT2_Q15_128);y128[6] = _mm_slli_epi16(y128[6],1);
    y128[7]  = _mm_mulhi_epi16(y128[7],ONE_OVER_SQRT2_Q15_128);y128[7] = _mm_slli_epi16(y128[7],1);
    y128[8]  = _mm_mulhi_epi16(y128[8],ONE_OVER_SQRT2_Q15_128);y128[8] = _mm_slli_epi16(y128[8],1);
    y128[9]  = _mm_mulhi_epi16(y128[9],ONE_OVER_SQRT2_Q15_128);y128[9] = _mm_slli_epi16(y128[9],1);
    y128[10] = _mm_mulhi_epi16(y128[10],ONE_OVER_SQRT2_Q15_128);y128[10] = _mm_slli_epi16(y128[10],1);
    y128[11] = _mm_mulhi_epi16(y128[11],ONE_OVER_SQRT2_Q15_128);y128[11] = _mm_slli_epi16(y128[11],1);
    y128[12] = _mm_mulhi_epi16(y128[12],ONE_OVER_SQRT2_Q15_128);y128[12] = _mm_slli_epi16(y128[12],1);
    y128[13] = _mm_mulhi_epi16(y128[13],ONE_OVER_SQRT2_Q15_128);y128[13] = _mm_slli_epi16(y128[13],1);
    y128[14] = _mm_mulhi_epi16(y128[14],ONE_OVER_SQRT2_Q15_128);y128[14] = _mm_slli_epi16(y128[14],1);
    y128[15] = _mm_mulhi_epi16(y128[15],ONE_OVER_SQRT2_Q15_128);y128[15] = _mm_slli_epi16(y128[15],1);

    y128[16]  = _mm_mulhi_epi16(y128[16],ONE_OVER_SQRT2_Q15_128);y128[16] = _mm_slli_epi16(y128[16],1);
    y128[17]  = _mm_mulhi_epi16(y128[17],ONE_OVER_SQRT2_Q15_128);y128[17] = _mm_slli_epi16(y128[17],1);
    y128[18]  = _mm_mulhi_epi16(y128[18],ONE_OVER_SQRT2_Q15_128);y128[18] = _mm_slli_epi16(y128[18],1);
    y128[19]  = _mm_mulhi_epi16(y128[19],ONE_OVER_SQRT2_Q15_128);y128[19] = _mm_slli_epi16(y128[19],1);
    y128[20]  = _mm_mulhi_epi16(y128[20],ONE_OVER_SQRT2_Q15_128);y128[20] = _mm_slli_epi16(y128[20],1);
    y128[21]  = _mm_mulhi_epi16(y128[21],ONE_OVER_SQRT2_Q15_128);y128[21] = _mm_slli_epi16(y128[21],1);
    y128[22]  = _mm_mulhi_epi16(y128[22],ONE_OVER_SQRT2_Q15_128);y128[22] = _mm_slli_epi16(y128[22],1);
    y128[23]  = _mm_mulhi_epi16(y128[23],ONE_OVER_SQRT2_Q15_128);y128[23] = _mm_slli_epi16(y128[23],1);
    y128[24]  = _mm_mulhi_epi16(y128[24],ONE_OVER_SQRT2_Q15_128);y128[24] = _mm_slli_epi16(y128[24],1);
    y128[25]  = _mm_mulhi_epi16(y128[25],ONE_OVER_SQRT2_Q15_128);y128[25] = _mm_slli_epi16(y128[25],1);
    y128[26] = _mm_mulhi_epi16(y128[26],ONE_OVER_SQRT2_Q15_128);y128[26] = _mm_slli_epi16(y128[26],1);
    y128[27] = _mm_mulhi_epi16(y128[27],ONE_OVER_SQRT2_Q15_128);y128[27] = _mm_slli_epi16(y128[27],1);
    y128[28] = _mm_mulhi_epi16(y128[28],ONE_OVER_SQRT2_Q15_128);y128[28] = _mm_slli_epi16(y128[28],1);
    y128[29] = _mm_mulhi_epi16(y128[29],ONE_OVER_SQRT2_Q15_128);y128[29] = _mm_slli_epi16(y128[29],1);
    y128[30] = _mm_mulhi_epi16(y128[30],ONE_OVER_SQRT2_Q15_128);y128[30] = _mm_slli_epi16(y128[30],1);
    y128[31] = _mm_mulhi_epi16(y128[31],ONE_OVER_SQRT2_Q15_128);y128[31] = _mm_slli_epi16(y128[31],1);

  }
  _mm_empty();
  _m_empty();

}

void idft128(int16_t *x,int16_t *y,int scale) {

  __m64 xtmp[64],*x64 = (__m64 *)x;
  __m128i ytmp[32],*tw128_128p=(__m128i *)tw128,*y128=(__m128i *)y,*y128p=(__m128i *)y;
  __m128i *ytmpp = &ytmp[0];
  int i;
  __m128i ONE_OVER_SQRT2_Q15_128 = _mm_set_epi16(ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15);

  transpose4_ooff(x64  ,xtmp,32);
  transpose4_ooff(x64+2,xtmp+1,32);
  transpose4_ooff(x64+4,xtmp+2,32);
  transpose4_ooff(x64+6,xtmp+3,32);
  transpose4_ooff(x64+8,xtmp+4,32);
  transpose4_ooff(x64+10,xtmp+5,32);
  transpose4_ooff(x64+12,xtmp+6,32);
  transpose4_ooff(x64+14,xtmp+7,32);
  transpose4_ooff(x64+16,xtmp+8,32);
  transpose4_ooff(x64+18,xtmp+9,32);
  transpose4_ooff(x64+20,xtmp+10,32);
  transpose4_ooff(x64+22,xtmp+11,32);
  transpose4_ooff(x64+24,xtmp+12,32);
  transpose4_ooff(x64+26,xtmp+13,32);
  transpose4_ooff(x64+28,xtmp+14,32);
  transpose4_ooff(x64+30,xtmp+15,32);
  transpose4_ooff(x64+32,xtmp+16,32);
  transpose4_ooff(x64+34,xtmp+17,32);
  transpose4_ooff(x64+36,xtmp+18,32);
  transpose4_ooff(x64+38,xtmp+19,32);
  transpose4_ooff(x64+40,xtmp+20,32);
  transpose4_ooff(x64+42,xtmp+21,32);
  transpose4_ooff(x64+44,xtmp+22,32);
  transpose4_ooff(x64+46,xtmp+23,32);
  transpose4_ooff(x64+48,xtmp+24,32);
  transpose4_ooff(x64+50,xtmp+25,32);
  transpose4_ooff(x64+52,xtmp+26,32);
  transpose4_ooff(x64+54,xtmp+27,32);
  transpose4_ooff(x64+56,xtmp+28,32);
  transpose4_ooff(x64+58,xtmp+29,32);
  transpose4_ooff(x64+60,xtmp+30,32);
  transpose4_ooff(x64+62,xtmp+31,32);

  idft64((int16_t*)(xtmp),(int16_t*)ytmp,1);
  idft64((int16_t*)(xtmp+32),(int16_t*)(ytmp+16),1);


  for (i=0;i<16;i++) {
    ibfly2(ytmpp,ytmpp+16,
	   y128p,y128p+16,
	   tw128_128p);
    tw128_128p++;
    y128p++;
    ytmpp++;    
  }

  if (scale>0) {

    y128[0]  = _mm_mulhi_epi16(y128[0],ONE_OVER_SQRT2_Q15_128);y128[0] = _mm_slli_epi16(y128[0],1);
    y128[1]  = _mm_mulhi_epi16(y128[1],ONE_OVER_SQRT2_Q15_128);y128[1] = _mm_slli_epi16(y128[1],1);
    y128[2]  = _mm_mulhi_epi16(y128[2],ONE_OVER_SQRT2_Q15_128);y128[2] = _mm_slli_epi16(y128[2],1);
    y128[3]  = _mm_mulhi_epi16(y128[3],ONE_OVER_SQRT2_Q15_128);y128[3] = _mm_slli_epi16(y128[3],1);
    y128[4]  = _mm_mulhi_epi16(y128[4],ONE_OVER_SQRT2_Q15_128);y128[4] = _mm_slli_epi16(y128[4],1);
    y128[5]  = _mm_mulhi_epi16(y128[5],ONE_OVER_SQRT2_Q15_128);y128[5] = _mm_slli_epi16(y128[5],1);
    y128[6]  = _mm_mulhi_epi16(y128[6],ONE_OVER_SQRT2_Q15_128);y128[6] = _mm_slli_epi16(y128[6],1);
    y128[7]  = _mm_mulhi_epi16(y128[7],ONE_OVER_SQRT2_Q15_128);y128[7] = _mm_slli_epi16(y128[7],1);
    y128[8]  = _mm_mulhi_epi16(y128[8],ONE_OVER_SQRT2_Q15_128);y128[8] = _mm_slli_epi16(y128[8],1);
    y128[9]  = _mm_mulhi_epi16(y128[9],ONE_OVER_SQRT2_Q15_128);y128[9] = _mm_slli_epi16(y128[9],1);
    y128[10] = _mm_mulhi_epi16(y128[10],ONE_OVER_SQRT2_Q15_128);y128[10] = _mm_slli_epi16(y128[10],1);
    y128[11] = _mm_mulhi_epi16(y128[11],ONE_OVER_SQRT2_Q15_128);y128[11] = _mm_slli_epi16(y128[11],1);
    y128[12] = _mm_mulhi_epi16(y128[12],ONE_OVER_SQRT2_Q15_128);y128[12] = _mm_slli_epi16(y128[12],1);
    y128[13] = _mm_mulhi_epi16(y128[13],ONE_OVER_SQRT2_Q15_128);y128[13] = _mm_slli_epi16(y128[13],1);
    y128[14] = _mm_mulhi_epi16(y128[14],ONE_OVER_SQRT2_Q15_128);y128[14] = _mm_slli_epi16(y128[14],1);
    y128[15] = _mm_mulhi_epi16(y128[15],ONE_OVER_SQRT2_Q15_128);y128[15] = _mm_slli_epi16(y128[15],1);

    y128[16]  = _mm_mulhi_epi16(y128[16],ONE_OVER_SQRT2_Q15_128);y128[16] = _mm_slli_epi16(y128[16],1);
    y128[17]  = _mm_mulhi_epi16(y128[17],ONE_OVER_SQRT2_Q15_128);y128[17] = _mm_slli_epi16(y128[17],1);
    y128[18]  = _mm_mulhi_epi16(y128[18],ONE_OVER_SQRT2_Q15_128);y128[18] = _mm_slli_epi16(y128[18],1);
    y128[19]  = _mm_mulhi_epi16(y128[19],ONE_OVER_SQRT2_Q15_128);y128[19] = _mm_slli_epi16(y128[19],1);
    y128[20]  = _mm_mulhi_epi16(y128[20],ONE_OVER_SQRT2_Q15_128);y128[20] = _mm_slli_epi16(y128[20],1);
    y128[21]  = _mm_mulhi_epi16(y128[21],ONE_OVER_SQRT2_Q15_128);y128[21] = _mm_slli_epi16(y128[21],1);
    y128[22]  = _mm_mulhi_epi16(y128[22],ONE_OVER_SQRT2_Q15_128);y128[22] = _mm_slli_epi16(y128[22],1);
    y128[23]  = _mm_mulhi_epi16(y128[23],ONE_OVER_SQRT2_Q15_128);y128[23] = _mm_slli_epi16(y128[23],1);
    y128[24]  = _mm_mulhi_epi16(y128[24],ONE_OVER_SQRT2_Q15_128);y128[24] = _mm_slli_epi16(y128[24],1);
    y128[25]  = _mm_mulhi_epi16(y128[25],ONE_OVER_SQRT2_Q15_128);y128[25] = _mm_slli_epi16(y128[25],1);
    y128[26] = _mm_mulhi_epi16(y128[26],ONE_OVER_SQRT2_Q15_128);y128[26] = _mm_slli_epi16(y128[26],1);
    y128[27] = _mm_mulhi_epi16(y128[27],ONE_OVER_SQRT2_Q15_128);y128[27] = _mm_slli_epi16(y128[27],1);
    y128[28] = _mm_mulhi_epi16(y128[28],ONE_OVER_SQRT2_Q15_128);y128[28] = _mm_slli_epi16(y128[28],1);
    y128[29] = _mm_mulhi_epi16(y128[29],ONE_OVER_SQRT2_Q15_128);y128[29] = _mm_slli_epi16(y128[29],1);
    y128[30] = _mm_mulhi_epi16(y128[30],ONE_OVER_SQRT2_Q15_128);y128[30] = _mm_slli_epi16(y128[30],1);
    y128[31] = _mm_mulhi_epi16(y128[31],ONE_OVER_SQRT2_Q15_128);y128[31] = _mm_slli_epi16(y128[31],1);

  }
  _mm_empty();
  _m_empty();

}
  

int16_t tw256[384] __attribute__((aligned(16))) = {  32767,0,32757,-805,32727,-1608,32678,-2411,32609,-3212,32520,-4012,32412,-4808,32284,-5602,32137,-6393,31970,-7180,31785,-7962,31580,-8740,31356,-9512,31113,-10279,30851,-11039,30571,-11793,30272,-12540,29955,-13279,29621,-14010,29268,-14733,28897,-15447,28510,-16151,28105,-16846,27683,-17531,27244,-18205,26789,-18868,26318,-19520,25831,-20160,25329,-20788,24811,-21403,24278,-22005,23731,-22595,23169,-23170,22594,-23732,22004,-24279,21402,-24812,20787,-25330,20159,-25832,19519,-26319,18867,-26790,18204,-27245,17530,-27684,16845,-28106,16150,-28511,15446,-28898,14732,-29269,14009,-29622,13278,-29956,12539,-30273,11792,-30572,11038,-30852,10278,-31114,9511,-31357,8739,-31581,7961,-31786,7179,-31971,6392,-32138,5601,-32285,4807,-32413,4011,-32521,3211,-32610,2410,-32679,1607,-32728,804,-32758,
						     32767,0,32727,-1608,32609,-3212,32412,-4808,32137,-6393,31785,-7962,31356,-9512,30851,-11039,30272,-12540,29621,-14010,28897,-15447,28105,-16846,27244,-18205,26318,-19520,25329,-20788,24278,-22005,23169,-23170,22004,-24279,20787,-25330,19519,-26319,18204,-27245,16845,-28106,15446,-28898,14009,-29622,12539,-30273,11038,-30852,9511,-31357,7961,-31786,6392,-32138,4807,-32413,3211,-32610,1607,-32728,0,-32767,-1608,-32728,-3212,-32610,-4808,-32413,-6393,-32138,-7962,-31786,-9512,-31357,-11039,-30852,-12540,-30273,-14010,-29622,-15447,-28898,-16846,-28106,-18205,-27245,-19520,-26319,-20788,-25330,-22005,-24279,-23170,-23170,-24279,-22005,-25330,-20788,-26319,-19520,-27245,-18205,-28106,-16846,-28898,-15447,-29622,-14010,-30273,-12540,-30852,-11039,-31357,-9512,-31786,-7962,-32138,-6393,-32413,-4808,-32610,-3212,-32728,-1608,
						     32767,0,32678,-2411,32412,-4808,31970,-7180,31356,-9512,30571,-11793,29621,-14010,28510,-16151,27244,-18205,25831,-20160,24278,-22005,22594,-23732,20787,-25330,18867,-26790,16845,-28106,14732,-29269,12539,-30273,10278,-31114,7961,-31786,5601,-32285,3211,-32610,804,-32758,-1608,-32728,-4012,-32521,-6393,-32138,-8740,-31581,-11039,-30852,-13279,-29956,-15447,-28898,-17531,-27684,-19520,-26319,-21403,-24812,-23170,-23170,-24812,-21403,-26319,-19520,-27684,-17531,-28898,-15447,-29956,-13279,-30852,-11039,-31581,-8740,-32138,-6393,-32521,-4012,-32728,-1608,-32758,804,-32610,3211,-32285,5601,-31786,7961,-31114,10278,-30273,12539,-29269,14732,-28106,16845,-26790,18867,-25330,20787,-23732,22594,-22005,24278,-20160,25831,-18205,27244,-16151,28510,-14010,29621,-11793,30571,-9512,31356,-7180,31970,-4808,32412,-2411,32678};

int16_t tw256a[384] __attribute__((aligned(16))) = { 32767,0,32757,804,32727,1607,32678,2410,32609,3211,32520,4011,32412,4807,32284,5601,32137,6392,31970,7179,31785,7961,31580,8739,31356,9511,31113,10278,30851,11038,30571,11792,30272,12539,29955,13278,29621,14009,29268,14732,28897,15446,28510,16150,28105,16845,27683,17530,27244,18204,26789,18867,26318,19519,25831,20159,25329,20787,24811,21402,24278,22004,23731,22594,23169,23169,22594,23731,22004,24278,21402,24811,20787,25329,20159,25831,19519,26318,18867,26789,18204,27244,17530,27683,16845,28105,16150,28510,15446,28897,14732,29268,14009,29621,13278,29955,12539,30272,11792,30571,11038,30851,10278,31113,9511,31356,8739,31580,7961,31785,7179,31970,6392,32137,5601,32284,4807,32412,4011,32520,3211,32609,2410,32678,1607,32727,804,32757,
						     32767,0,32727,1607,32609,3211,32412,4807,32137,6392,31785,7961,31356,9511,30851,11038,30272,12539,29621,14009,28897,15446,28105,16845,27244,18204,26318,19519,25329,20787,24278,22004,23169,23169,22004,24278,20787,25329,19519,26318,18204,27244,16845,28105,15446,28897,14009,29621,12539,30272,11038,30851,9511,31356,7961,31785,6392,32137,4807,32412,3211,32609,1607,32727,0,32767,-1608,32727,-3212,32609,-4808,32412,-6393,32137,-7962,31785,-9512,31356,-11039,30851,-12540,30272,-14010,29621,-15447,28897,-16846,28105,-18205,27244,-19520,26318,-20788,25329,-22005,24278,-23170,23169,-24279,22004,-25330,20787,-26319,19519,-27245,18204,-28106,16845,-28898,15446,-29622,14009,-30273,12539,-30852,11038,-31357,9511,-31786,7961,-32138,6392,-32413,4807,-32610,3211,-32728,1607,
						     32767,0,32678,2410,32412,4807,31970,7179,31356,9511,30571,11792,29621,14009,28510,16150,27244,18204,25831,20159,24278,22004,22594,23731,20787,25329,18867,26789,16845,28105,14732,29268,12539,30272,10278,31113,7961,31785,5601,32284,3211,32609,804,32757,-1608,32727,-4012,32520,-6393,32137,-8740,31580,-11039,30851,-13279,29955,-15447,28897,-17531,27683,-19520,26318,-21403,24811,-23170,23169,-24812,21402,-26319,19519,-27684,17530,-28898,15446,-29956,13278,-30852,11038,-31581,8739,-32138,6392,-32521,4011,-32728,1607,-32758,-805,-32610,-3212,-32285,-5602,-31786,-7962,-31114,-10279,-30273,-12540,-29269,-14733,-28106,-16846,-26790,-18868,-25330,-20788,-23732,-22595,-22005,-24279,-20160,-25832,-18205,-27245,-16151,-28511,-14010,-29622,-11793,-30572,-9512,-31357,-7180,-31971,-4808,-32413,-2411,-32679};

int16_t tw256b[384] __attribute__((aligned(16))) = {0,32767,-805,32757,-1608,32727,-2411,32678,-3212,32609,-4012,32520,-4808,32412,-5602,32284,-6393,32137,-7180,31970,-7962,31785,-8740,31580,-9512,31356,-10279,31113,-11039,30851,-11793,30571,-12540,30272,-13279,29955,-14010,29621,-14733,29268,-15447,28897,-16151,28510,-16846,28105,-17531,27683,-18205,27244,-18868,26789,-19520,26318,-20160,25831,-20788,25329,-21403,24811,-22005,24278,-22595,23731,-23170,23169,-23732,22594,-24279,22004,-24812,21402,-25330,20787,-25832,20159,-26319,19519,-26790,18867,-27245,18204,-27684,17530,-28106,16845,-28511,16150,-28898,15446,-29269,14732,-29622,14009,-29956,13278,-30273,12539,-30572,11792,-30852,11038,-31114,10278,-31357,9511,-31581,8739,-31786,7961,-31971,7179,-32138,6392,-32285,5601,-32413,4807,-32521,4011,-32610,3211,-32679,2410,-32728,1607,-32758,804,
						    0,32767,-1608,32727,-3212,32609,-4808,32412,-6393,32137,-7962,31785,-9512,31356,-11039,30851,-12540,30272,-14010,29621,-15447,28897,-16846,28105,-18205,27244,-19520,26318,-20788,25329,-22005,24278,-23170,23169,-24279,22004,-25330,20787,-26319,19519,-27245,18204,-28106,16845,-28898,15446,-29622,14009,-30273,12539,-30852,11038,-31357,9511,-31786,7961,-32138,6392,-32413,4807,-32610,3211,-32728,1607,-32767,0,-32728,-1608,-32610,-3212,-32413,-4808,-32138,-6393,-31786,-7962,-31357,-9512,-30852,-11039,-30273,-12540,-29622,-14010,-28898,-15447,-28106,-16846,-27245,-18205,-26319,-19520,-25330,-20788,-24279,-22005,-23170,-23170,-22005,-24279,-20788,-25330,-19520,-26319,-18205,-27245,-16846,-28106,-15447,-28898,-14010,-29622,-12540,-30273,-11039,-30852,-9512,-31357,-7962,-31786,-6393,-32138,-4808,-32413,-3212,-32610,-1608,-32728,
						    0,32767,-2411,32678,-4808,32412,-7180,31970,-9512,31356,-11793,30571,-14010,29621,-16151,28510,-18205,27244,-20160,25831,-22005,24278,-23732,22594,-25330,20787,-26790,18867,-28106,16845,-29269,14732,-30273,12539,-31114,10278,-31786,7961,-32285,5601,-32610,3211,-32758,804,-32728,-1608,-32521,-4012,-32138,-6393,-31581,-8740,-30852,-11039,-29956,-13279,-28898,-15447,-27684,-17531,-26319,-19520,-24812,-21403,-23170,-23170,-21403,-24812,-19520,-26319,-17531,-27684,-15447,-28898,-13279,-29956,-11039,-30852,-8740,-31581,-6393,-32138,-4012,-32521,-1608,-32728,804,-32758,3211,-32610,5601,-32285,7961,-31786,10278,-31114,12539,-30273,14732,-29269,16845,-28106,18867,-26790,20787,-25330,22594,-23732,24278,-22005,25831,-20160,27244,-18205,28510,-16151,29621,-14010,30571,-11793,31356,-9512,31970,-7180,32412,-4808,32678,-2411};

void dft256(int16_t *x,int16_t *y,int scale) {

  __m128i xtmp[64],ytmp[64],*tw256a_128p=(__m128i *)tw256a,*tw256b_128p=(__m128i *)tw256b,*x128=(__m128i *)x,*y128=(__m128i *)y,*y128p=(__m128i *)y;
  __m128i *ytmpp = &ytmp[0];
  int i;

#ifdef D256STATS
  time_stats_t ts_t,ts_d,ts_b;

  reset_meas(&ts_t);
  reset_meas(&ts_d);
  reset_meas(&ts_b);
  start_meas(&ts_t);
#endif  
  /*
  for (i=0,j=0;i<64;i+=4,j++) {
    transpose16_ooff(x128+i,xtmp+j,16);
  }
  */
    transpose16_ooff(x128+0,xtmp+0,16);
    transpose16_ooff(x128+4,xtmp+1,16);
    transpose16_ooff(x128+8,xtmp+2,16);
    transpose16_ooff(x128+12,xtmp+3,16);
    transpose16_ooff(x128+16,xtmp+4,16);
    transpose16_ooff(x128+20,xtmp+5,16);
    transpose16_ooff(x128+24,xtmp+6,16);
    transpose16_ooff(x128+28,xtmp+7,16);
    transpose16_ooff(x128+32,xtmp+8,16);
    transpose16_ooff(x128+36,xtmp+9,16);
    transpose16_ooff(x128+40,xtmp+10,16);
    transpose16_ooff(x128+44,xtmp+11,16);
    transpose16_ooff(x128+48,xtmp+12,16);
    transpose16_ooff(x128+52,xtmp+13,16);
    transpose16_ooff(x128+56,xtmp+14,16);
    transpose16_ooff(x128+60,xtmp+15,16);

#ifdef D256STATS
  stop_meas(&ts_t);
  start_meas(&ts_d);
#endif

  dft64((int16_t*)(xtmp),(int16_t*)(ytmp),1);
  dft64((int16_t*)(xtmp+16),(int16_t*)(ytmp+16),1);
  dft64((int16_t*)(xtmp+32),(int16_t*)(ytmp+32),1);
  dft64((int16_t*)(xtmp+48),(int16_t*)(ytmp+48),1);

#ifdef D256STATS
  stop_meas(&ts_d);
  start_meas(&ts_b);
#endif

  for (i=0;i<16;i+=4) {
    bfly4_16(ytmpp,ytmpp+16,ytmpp+32,ytmpp+48,
	     y128p,y128p+16,y128p+32,y128p+48,
	     tw256a_128p,tw256a_128p+16,tw256a_128p+32,
	     tw256b_128p,tw256b_128p+16,tw256b_128p+32);
    bfly4_16(ytmpp+1,ytmpp+17,ytmpp+33,ytmpp+49,
	     y128p+1,y128p+17,y128p+33,y128p+49,
	     tw256a_128p+1,tw256a_128p+17,tw256a_128p+33,
	     tw256b_128p+1,tw256b_128p+17,tw256b_128p+33);
    bfly4_16(ytmpp+2,ytmpp+18,ytmpp+34,ytmpp+50,
	     y128p+2,y128p+18,y128p+34,y128p+50,
	     tw256a_128p+2,tw256a_128p+18,tw256a_128p+34,
	     tw256b_128p+2,tw256b_128p+18,tw256b_128p+34);
    bfly4_16(ytmpp+3,ytmpp+19,ytmpp+35,ytmpp+51,
	     y128p+3,y128p+19,y128p+35,y128p+51,
	     tw256a_128p+3,tw256a_128p+19,tw256a_128p+35,
	     tw256b_128p+3,tw256b_128p+19,tw256b_128p+35);
    tw256a_128p+=4;
    tw256b_128p+=4;
    y128p+=4;
    ytmpp+=4;
  }

#ifdef D256STATS
  stop_meas(&ts_b);
  printf("t: %llu cycles, d: %llu cycles, b: %llu cycles\n",ts_t.diff,ts_d.diff,ts_b.diff);
#endif
    
  if (scale>0) {

    for (i=0;i<4;i++) {
      y128[0]  = _mm_srai_epi16(y128[0],1);
      y128[1]  = _mm_srai_epi16(y128[1],1);
      y128[2]  = _mm_srai_epi16(y128[2],1);
      y128[3]  = _mm_srai_epi16(y128[3],1);
      y128[4]  = _mm_srai_epi16(y128[4],1);
      y128[5]  = _mm_srai_epi16(y128[5],1);
      y128[6]  = _mm_srai_epi16(y128[6],1);
      y128[7]  = _mm_srai_epi16(y128[7],1);
      y128[8]  = _mm_srai_epi16(y128[8],1);
      y128[9]  = _mm_srai_epi16(y128[9],1);
      y128[10] = _mm_srai_epi16(y128[10],1);
      y128[11] = _mm_srai_epi16(y128[11],1);
      y128[12] = _mm_srai_epi16(y128[12],1);
      y128[13] = _mm_srai_epi16(y128[13],1);
      y128[14] = _mm_srai_epi16(y128[14],1);
      y128[15] = _mm_srai_epi16(y128[15],1);

      y128+=16;
    }

  }
  _mm_empty();
  _m_empty();

}



void idft256(int16_t *x,int16_t *y,int scale) {

  __m128i xtmp[64],ytmp[64],*tw256_128p=(__m128i *)tw256,*x128=(__m128i *)x,*y128=(__m128i *)y,*y128p=(__m128i *)y;
  __m128i *ytmpp = &ytmp[0];
  int i,j;
  
  for (i=0,j=0;i<64;i+=4,j++) {
    transpose16_ooff(x128+i,xtmp+j,16);
  }
  

  idft64((int16_t*)(xtmp),(int16_t*)(ytmp),1);
  idft64((int16_t*)(xtmp+16),(int16_t*)(ytmp+16),1);
  idft64((int16_t*)(xtmp+32),(int16_t*)(ytmp+32),1);
  idft64((int16_t*)(xtmp+48),(int16_t*)(ytmp+48),1);

  for (i=0;i<16;i++) {
    ibfly4(ytmpp,ytmpp+16,ytmpp+32,ytmpp+48,
	  y128p,y128p+16,y128p+32,y128p+48,
	  tw256_128p,tw256_128p+16,tw256_128p+32);
    tw256_128p++;
    y128p++;
    ytmpp++;
  }
    
  if (scale>0) {

    for (i=0;i<4;i++) {
      y128[0]  = _mm_srai_epi16(y128[0],1);
      y128[1]  = _mm_srai_epi16(y128[1],1);
      y128[2]  = _mm_srai_epi16(y128[2],1);
      y128[3]  = _mm_srai_epi16(y128[3],1);
      y128[4]  = _mm_srai_epi16(y128[4],1);
      y128[5]  = _mm_srai_epi16(y128[5],1);
      y128[6]  = _mm_srai_epi16(y128[6],1);
      y128[7]  = _mm_srai_epi16(y128[7],1);
      y128[8]  = _mm_srai_epi16(y128[8],1);
      y128[9]  = _mm_srai_epi16(y128[9],1);
      y128[10] = _mm_srai_epi16(y128[10],1);
      y128[11] = _mm_srai_epi16(y128[11],1);
      y128[12] = _mm_srai_epi16(y128[12],1);
      y128[13] = _mm_srai_epi16(y128[13],1);
      y128[14] = _mm_srai_epi16(y128[14],1);
      y128[15] = _mm_srai_epi16(y128[15],1);

      y128+=16;
    }

  }
  _mm_empty();
  _m_empty();

} 

int16_t tw512[512] __attribute__((aligned(16))) = { 
32767,0,32764,-403,32757,-805,32744,-1207,32727,-1608,32705,-2010,32678,-2411,32646,-2812,32609,-3212,32567,-3612,32520,-4012,32468,-4410,32412,-4808,32350,-5206,32284,-5602,32213,-5998,32137,-6393,32056,-6787,31970,-7180,31880,-7572,31785,-7962,31684,-8352,31580,-8740,31470,-9127,31356,-9512,31236,-9896,31113,-10279,30984,-10660,30851,-11039,30713,-11417,30571,-11793,30424,-12167,30272,-12540,30116,-12910,29955,-13279,29790,-13646,29621,-14010,29446,-14373,29268,-14733,29085,-15091,28897,-15447,28706,-15800,28510,-16151,28309,-16500,28105,-16846,27896,-17190,27683,-17531,27466,-17869,27244,-18205,27019,-18538,26789,-18868,26556,-19195,26318,-19520,26077,-19841,25831,-20160,25582,-20475,25329,-20788,25072,-21097,24811,-21403,24546,-21706,24278,-22005,24006,-22302,23731,-22595,23452,-22884,23169,-23170,22883,-23453,22594,-23732,22301,-24007,22004,-24279,21705,-24547,21402,-24812,21096,-25073,20787,-25330,20474,-25583,20159,-25832,19840,-26078,19519,-26319,19194,-26557,18867,-26790,18537,-27020,18204,-27245,17868,-27467,17530,-27684,17189,-27897,16845,-28106,16499,-28310,16150,-28511,15799,-28707,15446,-28898,15090,-29086,14732,-29269,14372,-29447,14009,-29622,13645,-29791,13278,-29956,12909,-30117,12539,-30273,12166,-30425,11792,-30572,11416,-30714,11038,-30852,10659,-30985,10278,-31114,9895,-31237,9511,-31357,9126,-31471,8739,-31581,8351,-31685,7961,-31786,7571,-31881,7179,-31971,6786,-32057,6392,-32138,5997,-32214,5601,-32285,5205,-32351,4807,-32413,4409,-32469,4011,-32521,3611,-32568,3211,-32610,2811,-32647,2410,-32679,2009,-32706,1607,-32728,1206,-32745,804,-32758,402,-32765,0,-32767,-403,-32765,-805,-32758,-1207,-32745,-1608,-32728,-2010,-32706,-2411,-32679,-2812,-32647,-3212,-32610,-3612,-32568,-4012,-32521,-4410,-32469,-4808,-32413,-5206,-32351,-5602,-32285,-5998,-32214,-6393,-32138,-6787,-32057,-7180,-31971,-7572,-31881,-7962,-31786,-8352,-31685,-8740,-31581,-9127,-31471,-9512,-31357,-9896,-31237,-10279,-31114,-10660,-30985,-11039,-30852,-11417,-30714,-11793,-30572,-12167,-30425,-12540,-30273,-12910,-30117,-13279,-29956,-13646,-29791,-14010,-29622,-14373,-29447,-14733,-29269,-15091,-29086,-15447,-28898,-15800,-28707,-16151,-28511,-16500,-28310,-16846,-28106,-17190,-27897,-17531,-27684,-17869,-27467,-18205,-27245,-18538,-27020,-18868,-26790,-19195,-26557,-19520,-26319,-19841,-26078,-20160,-25832,-20475,-25583,-20788,-25330,-21097,-25073,-21403,-24812,-21706,-24547,-22005,-24279,-22302,-24007,-22595,-23732,-22884,-23453,-23170,-23170,-23453,-22884,-23732,-22595,-24007,-22302,-24279,-22005,-24547,-21706,-24812,-21403,-25073,-21097,-25330,-20788,-25583,-20475,-25832,-20160,-26078,-19841,-26319,-19520,-26557,-19195,-26790,-18868,-27020,-18538,-27245,-18205,-27467,-17869,-27684,-17531,-27897,-17190,-28106,-16846,-28310,-16500,-28511,-16151,-28707,-15800,-28898,-15447,-29086,-15091,-29269,-14733,-29447,-14373,-29622,-14010,-29791,-13646,-29956,-13279,-30117,-12910,-30273,-12540,-30425,-12167,-30572,-11793,-30714,-11417,-30852,-11039,-30985,-10660,-31114,-10279,-31237,-9896,-31357,-9512,-31471,-9127,-31581,-8740,-31685,-8352,-31786,-7962,-31881,-7572,-31971,-7180,-32057,-6787,-32138,-6393,-32214,-5998,-32285,-5602,-32351,-5206,-32413,-4808,-32469,-4410,-32521,-4012,-32568,-3612,-32610,-3212,-32647,-2812,-32679,-2411,-32706,-2010,-32728,-1608,-32745,-1207,-32758,-805,-32765,-403
};

int16_t tw512a[512] __attribute__((aligned(16))) = { 
  32767,0,32764,403,32757,805,32744,1207,32727,1608,32705,2010,32678,2411,32646,2812,32609,3212,32567,3612,32520,4012,32468,4410,32412,4808,32350,5206,32284,5602,32213,5998,32137,6393,32056,6787,31970,7180,31880,7572,31785,7962,31684,8352,31580,8740,31470,9127,31356,9512,31236,9896,31113,10279,30984,10660,30851,11039,30713,11417,30571,11793,30424,12167,30272,12540,30116,12910,29955,13279,29790,13646,29621,14010,29446,14373,29268,14733,29085,15091,28897,15447,28706,15800,28510,16151,28309,16500,28105,16846,27896,17190,27683,17531,27466,17869,27244,18205,27019,18538,26789,18868,26556,19195,26318,19520,26077,19841,25831,20160,25582,20475,25329,20788,25072,21097,24811,21403,24546,21706,24278,22005,24006,22302,23731,22595,23452,22884,23169,23170,22883,23453,22594,23732,22301,24007,22004,24279,21705,24547,21402,24812,21096,25073,20787,25330,20474,25583,20159,25832,19840,26078,19519,26319,19194,26557,18867,26790,18537,27020,18204,27245,17868,27467,17530,27684,17189,27897,16845,28106,16499,28310,16150,28511,15799,28707,15446,28898,15090,29086,14732,29269,14372,29447,14009,29622,13645,29791,13278,29956,12909,30117,12539,30273,12166,30425,11792,30572,11416,30714,11038,30852,10659,30985,10278,31114,9895,31237,9511,31357,9126,31471,8739,31581,8351,31685,7961,31786,7571,31881,7179,31971,6786,32057,6392,32138,5997,32214,5601,32285,5205,32351,4807,32413,4409,32469,4011,32521,3611,32568,3211,32610,2811,32647,2410,32679,2009,32706,1607,32728,1206,32745,804,32758,402,32765,0,32767,-403,32765,-805,32758,-1207,32745,-1608,32728,-2010,32706,-2411,32679,-2812,32647,-3212,32610,-3612,32568,-4012,32521,-4410,32469,-4808,32413,-5206,32351,-5602,32285,-5998,32214,-6393,32138,-6787,32057,-7180,31971,-7572,31881,-7962,31786,-8352,31685,-8740,31581,-9127,31471,-9512,31357,-9896,31237,-10279,31114,-10660,30985,-11039,30852,-11417,30714,-11793,30572,-12167,30425,-12540,30273,-12910,30117,-13279,29956,-13646,29791,-14010,29622,-14373,29447,-14733,29269,-15091,29086,-15447,28898,-15800,28707,-16151,28511,-16500,28310,-16846,28106,-17190,27897,-17531,27684,-17869,27467,-18205,27245,-18538,27020,-18868,26790,-19195,26557,-19520,26319,-19841,26078,-20160,25832,-20475,25583,-20788,25330,-21097,25073,-21403,24812,-21706,24547,-22005,24279,-22302,24007,-22595,23732,-22884,23453,-23170,23170,-23453,22884,-23732,22595,-24007,22302,-24279,22005,-24547,21706,-24812,21403,-25073,21097,-25330,20788,-25583,20475,-25832,20160,-26078,19841,-26319,19520,-26557,19195,-26790,18868,-27020,18538,-27245,18205,-27467,17869,-27684,17531,-27897,17190,-28106,16846,-28310,16500,-28511,16151,-28707,15800,-28898,15447,-29086,15091,-29269,14733,-29447,14373,-29622,14010,-29791,13646,-29956,13279,-30117,12910,-30273,12540,-30425,12167,-30572,11793,-30714,11417,-30852,11039,-30985,10660,-31114,10279,-31237,9896,-31357,9512,-31471,9127,-31581,8740,-31685,8352,-31786,7962,-31881,7572,-31971,7180,-32057,6787,-32138,6393,-32214,5998,-32285,5602,-32351,5206,-32413,4808,-32469,4410,-32521,4012,-32568,3612,-32610,3212,-32647,2812,-32679,2411,-32706,2010,-32728,1608,-32745,1207,-32758,805,-32765,403};



int16_t tw512b[512] __attribute__((aligned(16))) = {
  0,32767,-403,32764,-805,32757,-1207,32744,-1608,32727,-2010,32705,-2411,32678,-2812,32646,-3212,32609,-3612,32567,-4012,32520,-4410,32468,-4808,32412,-5206,32350,-5602,32284,-5998,32213,-6393,32137,-6787,32056,-7180,31970,-7572,31880,-7962,31785,-8352,31684,-8740,31580,-9127,31470,-9512,31356,-9896,31236,-10279,31113,-10660,30984,-11039,30851,-11417,30713,-11793,30571,-12167,30424,-12540,30272,-12910,30116,-13279,29955,-13646,29790,-14010,29621,-14373,29446,-14733,29268,-15091,29085,-15447,28897,-15800,28706,-16151,28510,-16500,28309,-16846,28105,-17190,27896,-17531,27683,-17869,27466,-18205,27244,-18538,27019,-18868,26789,-19195,26556,-19520,26318,-19841,26077,-20160,25831,-20475,25582,-20788,25329,-21097,25072,-21403,24811,-21706,24546,-22005,24278,-22302,24006,-22595,23731,-22884,23452,-23170,23169,-23453,22883,-23732,22594,-24007,22301,-24279,22004,-24547,21705,-24812,21402,-25073,21096,-25330,20787,-25583,20474,-25832,20159,-26078,19840,-26319,19519,-26557,19194,-26790,18867,-27020,18537,-27245,18204,-27467,17868,-27684,17530,-27897,17189,-28106,16845,-28310,16499,-28511,16150,-28707,15799,-28898,15446,-29086,15090,-29269,14732,-29447,14372,-29622,14009,-29791,13645,-29956,13278,-30117,12909,-30273,12539,-30425,12166,-30572,11792,-30714,11416,-30852,11038,-30985,10659,-31114,10278,-31237,9895,-31357,9511,-31471,9126,-31581,8739,-31685,8351,-31786,7961,-31881,7571,-31971,7179,-32057,6786,-32138,6392,-32214,5997,-32285,5601,-32351,5205,-32413,4807,-32469,4409,-32521,4011,-32568,3611,-32610,3211,-32647,2811,-32679,2410,-32706,2009,-32728,1607,-32745,1206,-32758,804,-32765,402,-32767,0,-32765,-403,-32758,-805,-32745,-1207,-32728,-1608,-32706,-2010,-32679,-2411,-32647,-2812,-32610,-3212,-32568,-3612,-32521,-4012,-32469,-4410,-32413,-4808,-32351,-5206,-32285,-5602,-32214,-5998,-32138,-6393,-32057,-6787,-31971,-7180,-31881,-7572,-31786,-7962,-31685,-8352,-31581,-8740,-31471,-9127,-31357,-9512,-31237,-9896,-31114,-10279,-30985,-10660,-30852,-11039,-30714,-11417,-30572,-11793,-30425,-12167,-30273,-12540,-30117,-12910,-29956,-13279,-29791,-13646,-29622,-14010,-29447,-14373,-29269,-14733,-29086,-15091,-28898,-15447,-28707,-15800,-28511,-16151,-28310,-16500,-28106,-16846,-27897,-17190,-27684,-17531,-27467,-17869,-27245,-18205,-27020,-18538,-26790,-18868,-26557,-19195,-26319,-19520,-26078,-19841,-25832,-20160,-25583,-20475,-25330,-20788,-25073,-21097,-24812,-21403,-24547,-21706,-24279,-22005,-24007,-22302,-23732,-22595,-23453,-22884,-23170,-23170,-22884,-23453,-22595,-23732,-22302,-24007,-22005,-24279,-21706,-24547,-21403,-24812,-21097,-25073,-20788,-25330,-20475,-25583,-20160,-25832,-19841,-26078,-19520,-26319,-19195,-26557,-18868,-26790,-18538,-27020,-18205,-27245,-17869,-27467,-17531,-27684,-17190,-27897,-16846,-28106,-16500,-28310,-16151,-28511,-15800,-28707,-15447,-28898,-15091,-29086,-14733,-29269,-14373,-29447,-14010,-29622,-13646,-29791,-13279,-29956,-12910,-30117,-12540,-30273,-12167,-30425,-11793,-30572,-11417,-30714,-11039,-30852,-10660,-30985,-10279,-31114,-9896,-31237,-9512,-31357,-9127,-31471,-8740,-31581,-8352,-31685,-7962,-31786,-7572,-31881,-7180,-31971,-6787,-32057,-6393,-32138,-5998,-32214,-5602,-32285,-5206,-32351,-4808,-32413,-4410,-32469,-4012,-32521,-3612,-32568,-3212,-32610,-2812,-32647,-2411,-32679,-2010,-32706,-1608,-32728,-1207,-32745,-805,-32758,-403,-32765};

int16_t tw512c[512] __attribute__((aligned(16))) = {
  0,32767,403,32764,805,32757,1207,32744,1608,32727,2010,32705,2411,32678,2812,32646,3212,32609,3612,32567,4012,32520,4410,32468,4808,32412,5206,32350,5602,32284,5998,32213,6393,32137,6787,32056,7180,31970,7572,31880,7962,31785,8352,31684,8740,31580,9127,31470,9512,31356,9896,31236,10279,31113,10660,30984,11039,30851,11417,30713,11793,30571,12167,30424,12540,30272,12910,30116,13279,29955,13646,29790,14010,29621,14373,29446,14733,29268,15091,29085,15447,28897,15800,28706,16151,28510,16500,28309,16846,28105,17190,27896,17531,27683,17869,27466,18205,27244,18538,27019,18868,26789,19195,26556,19520,26318,19841,26077,20160,25831,20475,25582,20788,25329,21097,25072,21403,24811,21706,24546,22005,24278,22302,24006,22595,23731,22884,23452,23170,23169,23453,22883,23732,22594,24007,22301,24279,22004,24547,21705,24812,21402,25073,21096,25330,20787,25583,20474,25832,20159,26078,19840,26319,19519,26557,19194,26790,18867,27020,18537,27245,18204,27467,17868,27684,17530,27897,17189,28106,16845,28310,16499,28511,16150,28707,15799,28898,15446,29086,15090,29269,14732,29447,14372,29622,14009,29791,13645,29956,13278,30117,12909,30273,12539,30425,12166,30572,11792,30714,11416,30852,11038,30985,10659,31114,10278,31237,9895,31357,9511,31471,9126,31581,8739,31685,8351,31786,7961,31881,7571,31971,7179,32057,6786,32138,6392,32214,5997,32285,5601,32351,5205,32413,4807,32469,4409,32521,4011,32568,3611,32610,3211,32647,2811,32679,2410,32706,2009,32728,1607,32745,1206,32758,804,32765,402,32767,0,32765,-403,32758,-805,32745,-1207,32728,-1608,32706,-2010,32679,-2411,32647,-2812,32610,-3212,32568,-3612,32521,-4012,32469,-4410,32413,-4808,32351,-5206,32285,-5602,32214,-5998,32138,-6393,32057,-6787,31971,-7180,31881,-7572,31786,-7962,31685,-8352,31581,-8740,31471,-9127,31357,-9512,31237,-9896,31114,-10279,30985,-10660,30852,-11039,30714,-11417,30572,-11793,30425,-12167,30273,-12540,30117,-12910,29956,-13279,29791,-13646,29622,-14010,29447,-14373,29269,-14733,29086,-15091,28898,-15447,28707,-15800,28511,-16151,28310,-16500,28106,-16846,27897,-17190,27684,-17531,27467,-17869,27245,-18205,27020,-18538,26790,-18868,26557,-19195,26319,-19520,26078,-19841,25832,-20160,25583,-20475,25330,-20788,25073,-21097,24812,-21403,24547,-21706,24279,-22005,24007,-22302,23732,-22595,23453,-22884,23170,-23170,22884,-23453,22595,-23732,22302,-24007,22005,-24279,21706,-24547,21403,-24812,21097,-25073,20788,-25330,20475,-25583,20160,-25832,19841,-26078,19520,-26319,19195,-26557,18868,-26790,18538,-27020,18205,-27245,17869,-27467,17531,-27684,17190,-27897,16846,-28106,16500,-28310,16151,-28511,15800,-28707,15447,-28898,15091,-29086,14733,-29269,14373,-29447,14010,-29622,13646,-29791,13279,-29956,12910,-30117,12540,-30273,12167,-30425,11793,-30572,11417,-30714,11039,-30852,10660,-30985,10279,-31114,9896,-31237,9512,-31357,9127,-31471,8740,-31581,8352,-31685,7962,-31786,7572,-31881,7180,-31971,6787,-32057,6393,-32138,5998,-32214,5602,-32285,5206,-32351,4808,-32413,4410,-32469,4012,-32521,3612,-32568,3212,-32610,2812,-32647,2411,-32679,2010,-32706,1608,-32728,1207,-32745,805,-32758,403,-32765};


void dft512(int16_t *x,int16_t *y,int scale) {

  __m64 xtmp[256],*xtmpp,*x64 = (__m64 *)x;
  __m128i ytmp[128],*tw512a_128p=(__m128i *)tw512a,*tw512b_128p=(__m128i *)tw512b,*y128=(__m128i *)y,*y128p=(__m128i *)y;
  __m128i *ytmpp = &ytmp[0];
  int i;
  __m128i ONE_OVER_SQRT2_Q15_128 = _mm_set_epi16(ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15);

  xtmpp = xtmp;

  for (i=0;i<4;i++) {
    transpose4_ooff(x64  ,xtmpp,128);
    transpose4_ooff(x64+2,xtmpp+1,128);
    transpose4_ooff(x64+4,xtmpp+2,128);
    transpose4_ooff(x64+6,xtmpp+3,128);
    transpose4_ooff(x64+8,xtmpp+4,128);
    transpose4_ooff(x64+10,xtmpp+5,128);
    transpose4_ooff(x64+12,xtmpp+6,128);
    transpose4_ooff(x64+14,xtmpp+7,128);
    transpose4_ooff(x64+16,xtmpp+8,128);
    transpose4_ooff(x64+18,xtmpp+9,128);
    transpose4_ooff(x64+20,xtmpp+10,128);
    transpose4_ooff(x64+22,xtmpp+11,128);
    transpose4_ooff(x64+24,xtmpp+12,128);
    transpose4_ooff(x64+26,xtmpp+13,128);
    transpose4_ooff(x64+28,xtmpp+14,128);
    transpose4_ooff(x64+30,xtmpp+15,128);
    transpose4_ooff(x64+32,xtmpp+16,128);
    transpose4_ooff(x64+34,xtmpp+17,128);
    transpose4_ooff(x64+36,xtmpp+18,128);
    transpose4_ooff(x64+38,xtmpp+19,128);
    transpose4_ooff(x64+40,xtmpp+20,128);
    transpose4_ooff(x64+42,xtmpp+21,128);
    transpose4_ooff(x64+44,xtmpp+22,128);
    transpose4_ooff(x64+46,xtmpp+23,128);
    transpose4_ooff(x64+48,xtmpp+24,128);
    transpose4_ooff(x64+50,xtmpp+25,128);
    transpose4_ooff(x64+52,xtmpp+26,128);
    transpose4_ooff(x64+54,xtmpp+27,128);
    transpose4_ooff(x64+56,xtmpp+28,128);
    transpose4_ooff(x64+58,xtmpp+29,128);
    transpose4_ooff(x64+60,xtmpp+30,128);
    transpose4_ooff(x64+62,xtmpp+31,128);
    x64+=64;
    xtmpp+=32;
  }

  dft256((int16_t*)(xtmp),(int16_t*)ytmp,1);
  dft256((int16_t*)(xtmp+128),(int16_t*)(ytmp+64),1);


  for (i=0;i<64;i+=8) {
    bfly2_16(ytmpp,ytmpp+64,
	     y128p,y128p+64,
	     tw512a_128p,
	     tw512b_128p);
    bfly2_16(ytmpp+1,ytmpp+65,
	     y128p+1,y128p+65,
	     tw512a_128p+1,
	     tw512b_128p+1);
    bfly2_16(ytmpp+2,ytmpp+66,
	     y128p+2,y128p+66,
	     tw512a_128p+2,
	     tw512b_128p+2);
    bfly2_16(ytmpp+3,ytmpp+67,
	     y128p+3,y128p+67,
	     tw512a_128p+3,
	     tw512b_128p+3);
    bfly2_16(ytmpp+4,ytmpp+68,
	     y128p+4,y128p+68,
	     tw512a_128p+4,
	     tw512b_128p+4);
    bfly2_16(ytmpp+5,ytmpp+69,
	     y128p+5,y128p+69,
	     tw512a_128p+5,
	     tw512b_128p+5);
    bfly2_16(ytmpp+6,ytmpp+70,
	     y128p+6,y128p+70,
	     tw512a_128p+6,
	     tw512b_128p+6);
    bfly2_16(ytmpp+7,ytmpp+71,
	     y128p+7,y128p+71,
	     tw512a_128p+7,
	     tw512b_128p+7);
    tw512a_128p+=8;
    tw512b_128p+=8;
    y128p+=8;
    ytmpp+=8;    
  }

  if (scale>0) {
    y128p = y128;
    for (i=0;i<8;i++) {
      y128p[0]  = _mm_mulhi_epi16(y128p[0],ONE_OVER_SQRT2_Q15_128);y128p[0] = _mm_slli_epi16(y128p[0],1);
      y128p[1]  = _mm_mulhi_epi16(y128p[1],ONE_OVER_SQRT2_Q15_128);y128p[1] = _mm_slli_epi16(y128p[1],1);
      y128p[2]  = _mm_mulhi_epi16(y128p[2],ONE_OVER_SQRT2_Q15_128);y128p[2] = _mm_slli_epi16(y128p[2],1);
      y128p[3]  = _mm_mulhi_epi16(y128p[3],ONE_OVER_SQRT2_Q15_128);y128p[3] = _mm_slli_epi16(y128p[3],1);
      y128p[4]  = _mm_mulhi_epi16(y128p[4],ONE_OVER_SQRT2_Q15_128);y128p[4] = _mm_slli_epi16(y128p[4],1);
      y128p[5]  = _mm_mulhi_epi16(y128p[5],ONE_OVER_SQRT2_Q15_128);y128p[5] = _mm_slli_epi16(y128p[5],1);
      y128p[6]  = _mm_mulhi_epi16(y128p[6],ONE_OVER_SQRT2_Q15_128);y128p[6] = _mm_slli_epi16(y128p[6],1);
      y128p[7]  = _mm_mulhi_epi16(y128p[7],ONE_OVER_SQRT2_Q15_128);y128p[7] = _mm_slli_epi16(y128p[7],1);
      y128p[8]  = _mm_mulhi_epi16(y128p[8],ONE_OVER_SQRT2_Q15_128);y128p[8] = _mm_slli_epi16(y128p[8],1);
      y128p[9]  = _mm_mulhi_epi16(y128p[9],ONE_OVER_SQRT2_Q15_128);y128p[9] = _mm_slli_epi16(y128p[9],1);
      y128p[10] = _mm_mulhi_epi16(y128p[10],ONE_OVER_SQRT2_Q15_128);y128p[10] = _mm_slli_epi16(y128p[10],1);
      y128p[11] = _mm_mulhi_epi16(y128p[11],ONE_OVER_SQRT2_Q15_128);y128p[11] = _mm_slli_epi16(y128p[11],1);
      y128p[12] = _mm_mulhi_epi16(y128p[12],ONE_OVER_SQRT2_Q15_128);y128p[12] = _mm_slli_epi16(y128p[12],1);
      y128p[13] = _mm_mulhi_epi16(y128p[13],ONE_OVER_SQRT2_Q15_128);y128p[13] = _mm_slli_epi16(y128p[13],1);
      y128p[14] = _mm_mulhi_epi16(y128p[14],ONE_OVER_SQRT2_Q15_128);y128p[14] = _mm_slli_epi16(y128p[14],1);
      y128p[15] = _mm_mulhi_epi16(y128p[15],ONE_OVER_SQRT2_Q15_128);y128p[15] = _mm_slli_epi16(y128p[15],1);
      y128p+=16;
    }
  }
  _mm_empty();
  _m_empty();

}

void idft512(int16_t *x,int16_t *y,int scale) {

  __m64 xtmp[256],*xtmpp,*x64 = (__m64 *)x;
  __m128i ytmp[128],*tw512_128p=(__m128i *)tw512,*y128=(__m128i *)y,*y128p=(__m128i *)y;
  __m128i *ytmpp = &ytmp[0];
  int i;
  __m128i ONE_OVER_SQRT2_Q15_128 = _mm_set_epi16(ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15);

  xtmpp = xtmp;

  for (i=0;i<4;i++) {
    transpose4_ooff(x64  ,xtmpp,128);
    transpose4_ooff(x64+2,xtmpp+1,128);
    transpose4_ooff(x64+4,xtmpp+2,128);
    transpose4_ooff(x64+6,xtmpp+3,128);
    transpose4_ooff(x64+8,xtmpp+4,128);
    transpose4_ooff(x64+10,xtmpp+5,128);
    transpose4_ooff(x64+12,xtmpp+6,128);
    transpose4_ooff(x64+14,xtmpp+7,128);
    transpose4_ooff(x64+16,xtmpp+8,128);
    transpose4_ooff(x64+18,xtmpp+9,128);
    transpose4_ooff(x64+20,xtmpp+10,128);
    transpose4_ooff(x64+22,xtmpp+11,128);
    transpose4_ooff(x64+24,xtmpp+12,128);
    transpose4_ooff(x64+26,xtmpp+13,128);
    transpose4_ooff(x64+28,xtmpp+14,128);
    transpose4_ooff(x64+30,xtmpp+15,128);
    transpose4_ooff(x64+32,xtmpp+16,128);
    transpose4_ooff(x64+34,xtmpp+17,128);
    transpose4_ooff(x64+36,xtmpp+18,128);
    transpose4_ooff(x64+38,xtmpp+19,128);
    transpose4_ooff(x64+40,xtmpp+20,128);
    transpose4_ooff(x64+42,xtmpp+21,128);
    transpose4_ooff(x64+44,xtmpp+22,128);
    transpose4_ooff(x64+46,xtmpp+23,128);
    transpose4_ooff(x64+48,xtmpp+24,128);
    transpose4_ooff(x64+50,xtmpp+25,128);
    transpose4_ooff(x64+52,xtmpp+26,128);
    transpose4_ooff(x64+54,xtmpp+27,128);
    transpose4_ooff(x64+56,xtmpp+28,128);
    transpose4_ooff(x64+58,xtmpp+29,128);
    transpose4_ooff(x64+60,xtmpp+30,128);
    transpose4_ooff(x64+62,xtmpp+31,128);
    x64+=64;
    xtmpp+=32;
  }

  idft256((int16_t*)(xtmp),(int16_t*)ytmp,1);
  idft256((int16_t*)(xtmp+128),(int16_t*)(ytmp+64),1);


  for (i=0;i<64;i++) {
    ibfly2(ytmpp,ytmpp+64,
	  y128p,y128p+64,
	  tw512_128p);
    tw512_128p++;
    y128p++;
    ytmpp++;    
  }

  if (scale>0) {
    y128p = y128;
    for (i=0;i<8;i++) {
      y128p[0]  = _mm_mulhi_epi16(y128p[0],ONE_OVER_SQRT2_Q15_128);y128p[0] = _mm_slli_epi16(y128p[0],1);
      y128p[1]  = _mm_mulhi_epi16(y128p[1],ONE_OVER_SQRT2_Q15_128);y128p[1] = _mm_slli_epi16(y128p[1],1);
      y128p[2]  = _mm_mulhi_epi16(y128p[2],ONE_OVER_SQRT2_Q15_128);y128p[2] = _mm_slli_epi16(y128p[2],1);
      y128p[3]  = _mm_mulhi_epi16(y128p[3],ONE_OVER_SQRT2_Q15_128);y128p[3] = _mm_slli_epi16(y128p[3],1);
      y128p[4]  = _mm_mulhi_epi16(y128p[4],ONE_OVER_SQRT2_Q15_128);y128p[4] = _mm_slli_epi16(y128p[4],1);
      y128p[5]  = _mm_mulhi_epi16(y128p[5],ONE_OVER_SQRT2_Q15_128);y128p[5] = _mm_slli_epi16(y128p[5],1);
      y128p[6]  = _mm_mulhi_epi16(y128p[6],ONE_OVER_SQRT2_Q15_128);y128p[6] = _mm_slli_epi16(y128p[6],1);
      y128p[7]  = _mm_mulhi_epi16(y128p[7],ONE_OVER_SQRT2_Q15_128);y128p[7] = _mm_slli_epi16(y128p[7],1);
      y128p[8]  = _mm_mulhi_epi16(y128p[8],ONE_OVER_SQRT2_Q15_128);y128p[8] = _mm_slli_epi16(y128p[8],1);
      y128p[9]  = _mm_mulhi_epi16(y128p[9],ONE_OVER_SQRT2_Q15_128);y128p[9] = _mm_slli_epi16(y128p[9],1);
      y128p[10] = _mm_mulhi_epi16(y128p[10],ONE_OVER_SQRT2_Q15_128);y128p[10] = _mm_slli_epi16(y128p[10],1);
      y128p[11] = _mm_mulhi_epi16(y128p[11],ONE_OVER_SQRT2_Q15_128);y128p[11] = _mm_slli_epi16(y128p[11],1);
      y128p[12] = _mm_mulhi_epi16(y128p[12],ONE_OVER_SQRT2_Q15_128);y128p[12] = _mm_slli_epi16(y128p[12],1);
      y128p[13] = _mm_mulhi_epi16(y128p[13],ONE_OVER_SQRT2_Q15_128);y128p[13] = _mm_slli_epi16(y128p[13],1);
      y128p[14] = _mm_mulhi_epi16(y128p[14],ONE_OVER_SQRT2_Q15_128);y128p[14] = _mm_slli_epi16(y128p[14],1);
      y128p[15] = _mm_mulhi_epi16(y128p[15],ONE_OVER_SQRT2_Q15_128);y128p[15] = _mm_slli_epi16(y128p[15],1);
      y128p+=16;
    }
  }
  _mm_empty();
  _m_empty();

}

int16_t tw1024[1536] __attribute__((aligned(16))) = {  32767,0,32766,-202,32764,-403,32761,-604,32757,-805,32751,-1006,32744,-1207,32736,-1407,32727,-1608,32717,-1809,32705,-2010,32692,-2210,32678,-2411,32662,-2611,32646,-2812,32628,-3012,32609,-3212,32588,-3412,32567,-3612,32544,-3812,32520,-4012,32495,-4211,32468,-4410,32441,-4609,32412,-4808,32382,-5007,32350,-5206,32318,-5404,32284,-5602,32249,-5800,32213,-5998,32176,-6196,32137,-6393,32097,-6590,32056,-6787,32014,-6983,31970,-7180,31926,-7376,31880,-7572,31833,-7767,31785,-7962,31735,-8157,31684,-8352,31633,-8546,31580,-8740,31525,-8933,31470,-9127,31413,-9320,31356,-9512,31297,-9704,31236,-9896,31175,-10088,31113,-10279,31049,-10470,30984,-10660,30918,-10850,30851,-11039,30783,-11228,30713,-11417,30643,-11605,30571,-11793,30498,-11981,30424,-12167,30349,-12354,30272,-12540,30195,-12725,30116,-12910,30036,-13095,29955,-13279,29873,-13463,29790,-13646,29706,-13828,29621,-14010,29534,-14192,29446,-14373,29358,-14553,29268,-14733,29177,-14912,29085,-15091,28992,-15269,28897,-15447,28802,-15624,28706,-15800,28608,-15976,28510,-16151,28410,-16326,28309,-16500,28208,-16673,28105,-16846,28001,-17018,27896,-17190,27790,-17361,27683,-17531,27575,-17700,27466,-17869,27355,-18037,27244,-18205,27132,-18372,27019,-18538,26905,-18703,26789,-18868,26673,-19032,26556,-19195,26437,-19358,26318,-19520,26198,-19681,26077,-19841,25954,-20001,25831,-20160,25707,-20318,25582,-20475,25456,-20632,25329,-20788,25201,-20943,25072,-21097,24942,-21250,24811,-21403,24679,-21555,24546,-21706,24413,-21856,24278,-22005,24143,-22154,24006,-22302,23869,-22449,23731,-22595,23592,-22740,23452,-22884,23311,-23028,23169,-23170,23027,-23312,22883,-23453,22739,-23593,22594,-23732,22448,-23870,22301,-24007,22153,-24144,22004,-24279,21855,-24414,21705,-24547,21554,-24680,21402,-24812,21249,-24943,21096,-25073,20942,-25202,20787,-25330,20631,-25457,20474,-25583,20317,-25708,20159,-25832,20000,-25955,19840,-26078,19680,-26199,19519,-26319,19357,-26438,19194,-26557,19031,-26674,18867,-26790,18702,-26906,18537,-27020,18371,-27133,18204,-27245,18036,-27356,17868,-27467,17699,-27576,17530,-27684,17360,-27791,17189,-27897,17017,-28002,16845,-28106,16672,-28209,16499,-28310,16325,-28411,16150,-28511,15975,-28609,15799,-28707,15623,-28803,15446,-28898,15268,-28993,15090,-29086,14911,-29178,14732,-29269,14552,-29359,14372,-29447,14191,-29535,14009,-29622,13827,-29707,13645,-29791,13462,-29874,13278,-29956,13094,-30037,12909,-30117,12724,-30196,12539,-30273,12353,-30350,12166,-30425,11980,-30499,11792,-30572,11604,-30644,11416,-30714,11227,-30784,11038,-30852,10849,-30919,10659,-30985,10469,-31050,10278,-31114,10087,-31176,9895,-31237,9703,-31298,9511,-31357,9319,-31414,9126,-31471,8932,-31526,8739,-31581,8545,-31634,8351,-31685,8156,-31736,7961,-31786,7766,-31834,7571,-31881,7375,-31927,7179,-31971,6982,-32015,6786,-32057,6589,-32098,6392,-32138,6195,-32177,5997,-32214,5799,-32250,5601,-32285,5403,-32319,5205,-32351,5006,-32383,4807,-32413,4608,-32442,4409,-32469,4210,-32496,4011,-32521,3811,-32545,3611,-32568,3411,-32589,3211,-32610,3011,-32629,2811,-32647,2610,-32663,2410,-32679,2209,-32693,2009,-32706,1808,-32718,1607,-32728,1406,-32737,1206,-32745,1005,-32752,804,-32758,603,-32762,402,-32765,201,-32767,
			 32767,0,32764,-403,32757,-805,32744,-1207,32727,-1608,32705,-2010,32678,-2411,32646,-2812,32609,-3212,32567,-3612,32520,-4012,32468,-4410,32412,-4808,32350,-5206,32284,-5602,32213,-5998,32137,-6393,32056,-6787,31970,-7180,31880,-7572,31785,-7962,31684,-8352,31580,-8740,31470,-9127,31356,-9512,31236,-9896,31113,-10279,30984,-10660,30851,-11039,30713,-11417,30571,-11793,30424,-12167,30272,-12540,30116,-12910,29955,-13279,29790,-13646,29621,-14010,29446,-14373,29268,-14733,29085,-15091,28897,-15447,28706,-15800,28510,-16151,28309,-16500,28105,-16846,27896,-17190,27683,-17531,27466,-17869,27244,-18205,27019,-18538,26789,-18868,26556,-19195,26318,-19520,26077,-19841,25831,-20160,25582,-20475,25329,-20788,25072,-21097,24811,-21403,24546,-21706,24278,-22005,24006,-22302,23731,-22595,23452,-22884,23169,-23170,22883,-23453,22594,-23732,22301,-24007,22004,-24279,21705,-24547,21402,-24812,21096,-25073,20787,-25330,20474,-25583,20159,-25832,19840,-26078,19519,-26319,19194,-26557,18867,-26790,18537,-27020,18204,-27245,17868,-27467,17530,-27684,17189,-27897,16845,-28106,16499,-28310,16150,-28511,15799,-28707,15446,-28898,15090,-29086,14732,-29269,14372,-29447,14009,-29622,13645,-29791,13278,-29956,12909,-30117,12539,-30273,12166,-30425,11792,-30572,11416,-30714,11038,-30852,10659,-30985,10278,-31114,9895,-31237,9511,-31357,9126,-31471,8739,-31581,8351,-31685,7961,-31786,7571,-31881,7179,-31971,6786,-32057,6392,-32138,5997,-32214,5601,-32285,5205,-32351,4807,-32413,4409,-32469,4011,-32521,3611,-32568,3211,-32610,2811,-32647,2410,-32679,2009,-32706,1607,-32728,1206,-32745,804,-32758,402,-32765,0,-32767,-403,-32765,-805,-32758,-1207,-32745,-1608,-32728,-2010,-32706,-2411,-32679,-2812,-32647,-3212,-32610,-3612,-32568,-4012,-32521,-4410,-32469,-4808,-32413,-5206,-32351,-5602,-32285,-5998,-32214,-6393,-32138,-6787,-32057,-7180,-31971,-7572,-31881,-7962,-31786,-8352,-31685,-8740,-31581,-9127,-31471,-9512,-31357,-9896,-31237,-10279,-31114,-10660,-30985,-11039,-30852,-11417,-30714,-11793,-30572,-12167,-30425,-12540,-30273,-12910,-30117,-13279,-29956,-13646,-29791,-14010,-29622,-14373,-29447,-14733,-29269,-15091,-29086,-15447,-28898,-15800,-28707,-16151,-28511,-16500,-28310,-16846,-28106,-17190,-27897,-17531,-27684,-17869,-27467,-18205,-27245,-18538,-27020,-18868,-26790,-19195,-26557,-19520,-26319,-19841,-26078,-20160,-25832,-20475,-25583,-20788,-25330,-21097,-25073,-21403,-24812,-21706,-24547,-22005,-24279,-22302,-24007,-22595,-23732,-22884,-23453,-23170,-23170,-23453,-22884,-23732,-22595,-24007,-22302,-24279,-22005,-24547,-21706,-24812,-21403,-25073,-21097,-25330,-20788,-25583,-20475,-25832,-20160,-26078,-19841,-26319,-19520,-26557,-19195,-26790,-18868,-27020,-18538,-27245,-18205,-27467,-17869,-27684,-17531,-27897,-17190,-28106,-16846,-28310,-16500,-28511,-16151,-28707,-15800,-28898,-15447,-29086,-15091,-29269,-14733,-29447,-14373,-29622,-14010,-29791,-13646,-29956,-13279,-30117,-12910,-30273,-12540,-30425,-12167,-30572,-11793,-30714,-11417,-30852,-11039,-30985,-10660,-31114,-10279,-31237,-9896,-31357,-9512,-31471,-9127,-31581,-8740,-31685,-8352,-31786,-7962,-31881,-7572,-31971,-7180,-32057,-6787,-32138,-6393,-32214,-5998,-32285,-5602,-32351,-5206,-32413,-4808,-32469,-4410,-32521,-4012,-32568,-3612,-32610,-3212,-32647,-2812,-32679,-2411,-32706,-2010,-32728,-1608,-32745,-1207,-32758,-805,-32765,-403,
			 32767,0,32761,-604,32744,-1207,32717,-1809,32678,-2411,32628,-3012,32567,-3612,32495,-4211,32412,-4808,32318,-5404,32213,-5998,32097,-6590,31970,-7180,31833,-7767,31684,-8352,31525,-8933,31356,-9512,31175,-10088,30984,-10660,30783,-11228,30571,-11793,30349,-12354,30116,-12910,29873,-13463,29621,-14010,29358,-14553,29085,-15091,28802,-15624,28510,-16151,28208,-16673,27896,-17190,27575,-17700,27244,-18205,26905,-18703,26556,-19195,26198,-19681,25831,-20160,25456,-20632,25072,-21097,24679,-21555,24278,-22005,23869,-22449,23452,-22884,23027,-23312,22594,-23732,22153,-24144,21705,-24547,21249,-24943,20787,-25330,20317,-25708,19840,-26078,19357,-26438,18867,-26790,18371,-27133,17868,-27467,17360,-27791,16845,-28106,16325,-28411,15799,-28707,15268,-28993,14732,-29269,14191,-29535,13645,-29791,13094,-30037,12539,-30273,11980,-30499,11416,-30714,10849,-30919,10278,-31114,9703,-31298,9126,-31471,8545,-31634,7961,-31786,7375,-31927,6786,-32057,6195,-32177,5601,-32285,5006,-32383,4409,-32469,3811,-32545,3211,-32610,2610,-32663,2009,-32706,1406,-32737,804,-32758,201,-32767,-403,-32765,-1006,-32752,-1608,-32728,-2210,-32693,-2812,-32647,-3412,-32589,-4012,-32521,-4609,-32442,-5206,-32351,-5800,-32250,-6393,-32138,-6983,-32015,-7572,-31881,-8157,-31736,-8740,-31581,-9320,-31414,-9896,-31237,-10470,-31050,-11039,-30852,-11605,-30644,-12167,-30425,-12725,-30196,-13279,-29956,-13828,-29707,-14373,-29447,-14912,-29178,-15447,-28898,-15976,-28609,-16500,-28310,-17018,-28002,-17531,-27684,-18037,-27356,-18538,-27020,-19032,-26674,-19520,-26319,-20001,-25955,-20475,-25583,-20943,-25202,-21403,-24812,-21856,-24414,-22302,-24007,-22740,-23593,-23170,-23170,-23593,-22740,-24007,-22302,-24414,-21856,-24812,-21403,-25202,-20943,-25583,-20475,-25955,-20001,-26319,-19520,-26674,-19032,-27020,-18538,-27356,-18037,-27684,-17531,-28002,-17018,-28310,-16500,-28609,-15976,-28898,-15447,-29178,-14912,-29447,-14373,-29707,-13828,-29956,-13279,-30196,-12725,-30425,-12167,-30644,-11605,-30852,-11039,-31050,-10470,-31237,-9896,-31414,-9320,-31581,-8740,-31736,-8157,-31881,-7572,-32015,-6983,-32138,-6393,-32250,-5800,-32351,-5206,-32442,-4609,-32521,-4012,-32589,-3412,-32647,-2812,-32693,-2210,-32728,-1608,-32752,-1006,-32765,-403,-32767,201,-32758,804,-32737,1406,-32706,2009,-32663,2610,-32610,3211,-32545,3811,-32469,4409,-32383,5006,-32285,5601,-32177,6195,-32057,6786,-31927,7375,-31786,7961,-31634,8545,-31471,9126,-31298,9703,-31114,10278,-30919,10849,-30714,11416,-30499,11980,-30273,12539,-30037,13094,-29791,13645,-29535,14191,-29269,14732,-28993,15268,-28707,15799,-28411,16325,-28106,16845,-27791,17360,-27467,17868,-27133,18371,-26790,18867,-26438,19357,-26078,19840,-25708,20317,-25330,20787,-24943,21249,-24547,21705,-24144,22153,-23732,22594,-23312,23027,-22884,23452,-22449,23869,-22005,24278,-21555,24679,-21097,25072,-20632,25456,-20160,25831,-19681,26198,-19195,26556,-18703,26905,-18205,27244,-17700,27575,-17190,27896,-16673,28208,-16151,28510,-15624,28802,-15091,29085,-14553,29358,-14010,29621,-13463,29873,-12910,30116,-12354,30349,-11793,30571,-11228,30783,-10660,30984,-10088,31175,-9512,31356,-8933,31525,-8352,31684,-7767,31833,-7180,31970,-6590,32097,-5998,32213,-5404,32318,-4808,32412,-4211,32495,-3612,32567,-3012,32628,-2411,32678,-1809,32717,-1207,32744,-604,32761};

void dft1024(int16_t *x,int16_t *y,int scale) {

  __m128i xtmp[256],ytmp[256],*tw1024_128p=(__m128i *)tw1024,*x128=(__m128i *)x,*y128=(__m128i *)y,*y128p=(__m128i *)y;
  __m128i *ytmpp = &ytmp[0];
  int i,j;
  
  for (i=0,j=0;i<256;i+=4,j++) {
    transpose16_ooff(x128+i,xtmp+j,64);
  }
  

  dft256((int16_t*)(xtmp),(int16_t*)(ytmp),1);
  dft256((int16_t*)(xtmp+64),(int16_t*)(ytmp+64),1);
  dft256((int16_t*)(xtmp+128),(int16_t*)(ytmp+128),1);
  dft256((int16_t*)(xtmp+192),(int16_t*)(ytmp+192),1);

  for (i=0;i<64;i++) {
    bfly4(ytmpp,ytmpp+64,ytmpp+128,ytmpp+192,
	  y128p,y128p+64,y128p+128,y128p+192,
	  tw1024_128p,tw1024_128p+64,tw1024_128p+128);
    tw1024_128p++;
    y128p++;
    ytmpp++;
  }
    
  if (scale>0) {

    for (i=0;i<16;i++) {
      y128[0]  = _mm_srai_epi16(y128[0],1);
      y128[1]  = _mm_srai_epi16(y128[1],1);
      y128[2]  = _mm_srai_epi16(y128[2],1);
      y128[3]  = _mm_srai_epi16(y128[3],1);
      y128[4]  = _mm_srai_epi16(y128[4],1);
      y128[5]  = _mm_srai_epi16(y128[5],1);
      y128[6]  = _mm_srai_epi16(y128[6],1);
      y128[7]  = _mm_srai_epi16(y128[7],1);
      y128[8]  = _mm_srai_epi16(y128[8],1);
      y128[9]  = _mm_srai_epi16(y128[9],1);
      y128[10] = _mm_srai_epi16(y128[10],1);
      y128[11] = _mm_srai_epi16(y128[11],1);
      y128[12] = _mm_srai_epi16(y128[12],1);
      y128[13] = _mm_srai_epi16(y128[13],1);
      y128[14] = _mm_srai_epi16(y128[14],1);
      y128[15] = _mm_srai_epi16(y128[15],1);

      y128+=16;
    }

  }
  _mm_empty();
  _m_empty();

}

void idft1024(int16_t *x,int16_t *y,int scale) {

  __m128i xtmp[256],ytmp[256],*tw1024_128p=(__m128i *)tw1024,*x128=(__m128i *)x,*y128=(__m128i *)y,*y128p=(__m128i *)y;
  __m128i *ytmpp = &ytmp[0];
  int i,j;
  
  for (i=0,j=0;i<256;i+=4,j++) {
    transpose16_ooff(x128+i,xtmp+j,64);
  }
  

  idft256((int16_t*)(xtmp),(int16_t*)(ytmp),1);
  idft256((int16_t*)(xtmp+64),(int16_t*)(ytmp+64),1);
  idft256((int16_t*)(xtmp+128),(int16_t*)(ytmp+128),1);
  idft256((int16_t*)(xtmp+192),(int16_t*)(ytmp+192),1);

  for (i=0;i<64;i++) {
    ibfly4(ytmpp,ytmpp+64,ytmpp+128,ytmpp+192,
	   y128p,y128p+64,y128p+128,y128p+192,
	   tw1024_128p,tw1024_128p+64,tw1024_128p+128);
    tw1024_128p++;
    y128p++;
    ytmpp++;
  }
    
  if (scale>0) {

    for (i=0;i<16;i++) {
      y128[0]  = _mm_srai_epi16(y128[0],1);
      y128[1]  = _mm_srai_epi16(y128[1],1);
      y128[2]  = _mm_srai_epi16(y128[2],1);
      y128[3]  = _mm_srai_epi16(y128[3],1);
      y128[4]  = _mm_srai_epi16(y128[4],1);
      y128[5]  = _mm_srai_epi16(y128[5],1);
      y128[6]  = _mm_srai_epi16(y128[6],1);
      y128[7]  = _mm_srai_epi16(y128[7],1);
      y128[8]  = _mm_srai_epi16(y128[8],1);
      y128[9]  = _mm_srai_epi16(y128[9],1);
      y128[10] = _mm_srai_epi16(y128[10],1);
      y128[11] = _mm_srai_epi16(y128[11],1);
      y128[12] = _mm_srai_epi16(y128[12],1);
      y128[13] = _mm_srai_epi16(y128[13],1);
      y128[14] = _mm_srai_epi16(y128[14],1);
      y128[15] = _mm_srai_epi16(y128[15],1);

      y128+=16;
    }

  }
  _mm_empty();
  _m_empty();

}

int16_t tw2048[2048] __attribute__((aligned(16))) = {32767,0,32766,-101,32766,-202,32765,-302,32764,-403,32763,-503,32761,-604,32759,-704,32757,-805,32754,-905,32751,-1006,32748,-1106,32744,-1207,32740,-1307,32736,-1407,32732,-1508,32727,-1608,32722,-1709,32717,-1809,32711,-1909,32705,-2010,32699,-2110,32692,-2210,32685,-2311,32678,-2411,32670,-2511,32662,-2611,32654,-2712,32646,-2812,32637,-2912,32628,-3012,32618,-3112,32609,-3212,32599,-3312,32588,-3412,32578,-3512,32567,-3612,32556,-3712,32544,-3812,32532,-3912,32520,-4012,32508,-4111,32495,-4211,32482,-4311,32468,-4410,32455,-4510,32441,-4609,32426,-4709,32412,-4808,32397,-4908,32382,-5007,32366,-5107,32350,-5206,32334,-5305,32318,-5404,32301,-5503,32284,-5602,32267,-5701,32249,-5800,32231,-5899,32213,-5998,32194,-6097,32176,-6196,32156,-6294,32137,-6393,32117,-6492,32097,-6590,32077,-6689,32056,-6787,32035,-6885,32014,-6983,31992,-7082,31970,-7180,31948,-7278,31926,-7376,31903,-7474,31880,-7572,31856,-7669,31833,-7767,31809,-7865,31785,-7962,31760,-8060,31735,-8157,31710,-8254,31684,-8352,31659,-8449,31633,-8546,31606,-8643,31580,-8740,31553,-8837,31525,-8933,31498,-9030,31470,-9127,31442,-9223,31413,-9320,31385,-9416,31356,-9512,31326,-9608,31297,-9704,31267,-9800,31236,-9896,31206,-9992,31175,-10088,31144,-10183,31113,-10279,31081,-10374,31049,-10470,31017,-10565,30984,-10660,30951,-10755,30918,-10850,30885,-10945,30851,-11039,30817,-11134,30783,-11228,30748,-11323,30713,-11417,30678,-11511,30643,-11605,30607,-11699,30571,-11793,30535,-11887,30498,-11981,30461,-12074,30424,-12167,30386,-12261,30349,-12354,30311,-12447,30272,-12540,30234,-12633,30195,-12725,30156,-12818,30116,-12910,30076,-13003,30036,-13095,29996,-13187,29955,-13279,29915,-13371,29873,-13463,29832,-13554,29790,-13646,29748,-13737,29706,-13828,29663,-13919,29621,-14010,29577,-14101,29534,-14192,29490,-14282,29446,-14373,29402,-14463,29358,-14553,29313,-14643,29268,-14733,29222,-14823,29177,-14912,29131,-15002,29085,-15091,29038,-15180,28992,-15269,28945,-15358,28897,-15447,28850,-15535,28802,-15624,28754,-15712,28706,-15800,28657,-15888,28608,-15976,28559,-16064,28510,-16151,28460,-16239,28410,-16326,28360,-16413,28309,-16500,28259,-16587,28208,-16673,28156,-16760,28105,-16846,28053,-16932,28001,-17018,27948,-17104,27896,-17190,27843,-17275,27790,-17361,27736,-17446,27683,-17531,27629,-17616,27575,-17700,27520,-17785,27466,-17869,27411,-17953,27355,-18037,27300,-18121,27244,-18205,27188,-18288,27132,-18372,27076,-18455,27019,-18538,26962,-18621,26905,-18703,26847,-18786,26789,-18868,26731,-18950,26673,-19032,26615,-19114,26556,-19195,26497,-19277,26437,-19358,26378,-19439,26318,-19520,26258,-19600,26198,-19681,26137,-19761,26077,-19841,26016,-19921,25954,-20001,25893,-20080,25831,-20160,25769,-20239,25707,-20318,25645,-20397,25582,-20475,25519,-20554,25456,-20632,25392,-20710,25329,-20788,25265,-20865,25201,-20943,25136,-21020,25072,-21097,25007,-21174,24942,-21250,24877,-21327,24811,-21403,24745,-21479,24679,-21555,24613,-21630,24546,-21706,24480,-21781,24413,-21856,24346,-21931,24278,-22005,24211,-22080,24143,-22154,24075,-22228,24006,-22302,23938,-22375,23869,-22449,23800,-22522,23731,-22595,23661,-22667,23592,-22740,23522,-22812,23452,-22884,23382,-22956,23311,-23028,23240,-23099,23169,-23170,23098,-23241,23027,-23312,22955,-23383,22883,-23453,22811,-23523,22739,-23593,22666,-23662,22594,-23732,22521,-23801,22448,-23870,22374,-23939,22301,-24007,22227,-24076,22153,-24144,22079,-24212,22004,-24279,21930,-24347,21855,-24414,21780,-24481,21705,-24547,21629,-24614,21554,-24680,21478,-24746,21402,-24812,21326,-24878,21249,-24943,21173,-25008,21096,-25073,21019,-25137,20942,-25202,20864,-25266,20787,-25330,20709,-25393,20631,-25457,20553,-25520,20474,-25583,20396,-25646,20317,-25708,20238,-25770,20159,-25832,20079,-25894,20000,-25955,19920,-26017,19840,-26078,19760,-26138,19680,-26199,19599,-26259,19519,-26319,19438,-26379,19357,-26438,19276,-26498,19194,-26557,19113,-26616,19031,-26674,18949,-26732,18867,-26790,18785,-26848,18702,-26906,18620,-26963,18537,-27020,18454,-27077,18371,-27133,18287,-27189,18204,-27245,18120,-27301,18036,-27356,17952,-27412,17868,-27467,17784,-27521,17699,-27576,17615,-27630,17530,-27684,17445,-27737,17360,-27791,17274,-27844,17189,-27897,17103,-27949,17017,-28002,16931,-28054,16845,-28106,16759,-28157,16672,-28209,16586,-28260,16499,-28310,16412,-28361,16325,-28411,16238,-28461,16150,-28511,16063,-28560,15975,-28609,15887,-28658,15799,-28707,15711,-28755,15623,-28803,15534,-28851,15446,-28898,15357,-28946,15268,-28993,15179,-29039,15090,-29086,15001,-29132,14911,-29178,14822,-29223,14732,-29269,14642,-29314,14552,-29359,14462,-29403,14372,-29447,14281,-29491,14191,-29535,14100,-29578,14009,-29622,13918,-29664,13827,-29707,13736,-29749,13645,-29791,13553,-29833,13462,-29874,13370,-29916,13278,-29956,13186,-29997,13094,-30037,13002,-30077,12909,-30117,12817,-30157,12724,-30196,12632,-30235,12539,-30273,12446,-30312,12353,-30350,12260,-30387,12166,-30425,12073,-30462,11980,-30499,11886,-30536,11792,-30572,11698,-30608,11604,-30644,11510,-30679,11416,-30714,11322,-30749,11227,-30784,11133,-30818,11038,-30852,10944,-30886,10849,-30919,10754,-30952,10659,-30985,10564,-31018,10469,-31050,10373,-31082,10278,-31114,10182,-31145,10087,-31176,9991,-31207,9895,-31237,9799,-31268,9703,-31298,9607,-31327,9511,-31357,9415,-31386,9319,-31414,9222,-31443,9126,-31471,9029,-31499,8932,-31526,8836,-31554,8739,-31581,8642,-31607,8545,-31634,8448,-31660,8351,-31685,8253,-31711,8156,-31736,8059,-31761,7961,-31786,7864,-31810,7766,-31834,7668,-31857,7571,-31881,7473,-31904,7375,-31927,7277,-31949,7179,-31971,7081,-31993,6982,-32015,6884,-32036,6786,-32057,6688,-32078,6589,-32098,6491,-32118,6392,-32138,6293,-32157,6195,-32177,6096,-32195,5997,-32214,5898,-32232,5799,-32250,5700,-32268,5601,-32285,5502,-32302,5403,-32319,5304,-32335,5205,-32351,5106,-32367,5006,-32383,4907,-32398,4807,-32413,4708,-32427,4608,-32442,4509,-32456,4409,-32469,4310,-32483,4210,-32496,4110,-32509,4011,-32521,3911,-32533,3811,-32545,3711,-32557,3611,-32568,3511,-32579,3411,-32589,3311,-32600,3211,-32610,3111,-32619,3011,-32629,2911,-32638,2811,-32647,2711,-32655,2610,-32663,2510,-32671,2410,-32679,2310,-32686,2209,-32693,2109,-32700,2009,-32706,1908,-32712,1808,-32718,1708,-32723,1607,-32728,1507,-32733,1406,-32737,1306,-32741,1206,-32745,1105,-32749,1005,-32752,904,-32755,804,-32758,703,-32760,603,-32762,502,-32764,402,-32765,301,-32766,201,-32767,100,-32767,0,-32767,-101,-32767,-202,-32767,-302,-32766,-403,-32765,-503,-32764,-604,-32762,-704,-32760,-805,-32758,-905,-32755,-1006,-32752,-1106,-32749,-1207,-32745,-1307,-32741,-1407,-32737,-1508,-32733,-1608,-32728,-1709,-32723,-1809,-32718,-1909,-32712,-2010,-32706,-2110,-32700,-2210,-32693,-2311,-32686,-2411,-32679,-2511,-32671,-2611,-32663,-2712,-32655,-2812,-32647,-2912,-32638,-3012,-32629,-3112,-32619,-3212,-32610,-3312,-32600,-3412,-32589,-3512,-32579,-3612,-32568,-3712,-32557,-3812,-32545,-3912,-32533,-4012,-32521,-4111,-32509,-4211,-32496,-4311,-32483,-4410,-32469,-4510,-32456,-4609,-32442,-4709,-32427,-4808,-32413,-4908,-32398,-5007,-32383,-5107,-32367,-5206,-32351,-5305,-32335,-5404,-32319,-5503,-32302,-5602,-32285,-5701,-32268,-5800,-32250,-5899,-32232,-5998,-32214,-6097,-32195,-6196,-32177,-6294,-32157,-6393,-32138,-6492,-32118,-6590,-32098,-6689,-32078,-6787,-32057,-6885,-32036,-6983,-32015,-7082,-31993,-7180,-31971,-7278,-31949,-7376,-31927,-7474,-31904,-7572,-31881,-7669,-31857,-7767,-31834,-7865,-31810,-7962,-31786,-8060,-31761,-8157,-31736,-8254,-31711,-8352,-31685,-8449,-31660,-8546,-31634,-8643,-31607,-8740,-31581,-8837,-31554,-8933,-31526,-9030,-31499,-9127,-31471,-9223,-31443,-9320,-31414,-9416,-31386,-9512,-31357,-9608,-31327,-9704,-31298,-9800,-31268,-9896,-31237,-9992,-31207,-10088,-31176,-10183,-31145,-10279,-31114,-10374,-31082,-10470,-31050,-10565,-31018,-10660,-30985,-10755,-30952,-10850,-30919,-10945,-30886,-11039,-30852,-11134,-30818,-11228,-30784,-11323,-30749,-11417,-30714,-11511,-30679,-11605,-30644,-11699,-30608,-11793,-30572,-11887,-30536,-11981,-30499,-12074,-30462,-12167,-30425,-12261,-30387,-12354,-30350,-12447,-30312,-12540,-30273,-12633,-30235,-12725,-30196,-12818,-30157,-12910,-30117,-13003,-30077,-13095,-30037,-13187,-29997,-13279,-29956,-13371,-29916,-13463,-29874,-13554,-29833,-13646,-29791,-13737,-29749,-13828,-29707,-13919,-29664,-14010,-29622,-14101,-29578,-14192,-29535,-14282,-29491,-14373,-29447,-14463,-29403,-14553,-29359,-14643,-29314,-14733,-29269,-14823,-29223,-14912,-29178,-15002,-29132,-15091,-29086,-15180,-29039,-15269,-28993,-15358,-28946,-15447,-28898,-15535,-28851,-15624,-28803,-15712,-28755,-15800,-28707,-15888,-28658,-15976,-28609,-16064,-28560,-16151,-28511,-16239,-28461,-16326,-28411,-16413,-28361,-16500,-28310,-16587,-28260,-16673,-28209,-16760,-28157,-16846,-28106,-16932,-28054,-17018,-28002,-17104,-27949,-17190,-27897,-17275,-27844,-17361,-27791,-17446,-27737,-17531,-27684,-17616,-27630,-17700,-27576,-17785,-27521,-17869,-27467,-17953,-27412,-18037,-27356,-18121,-27301,-18205,-27245,-18288,-27189,-18372,-27133,-18455,-27077,-18538,-27020,-18621,-26963,-18703,-26906,-18786,-26848,-18868,-26790,-18950,-26732,-19032,-26674,-19114,-26616,-19195,-26557,-19277,-26498,-19358,-26438,-19439,-26379,-19520,-26319,-19600,-26259,-19681,-26199,-19761,-26138,-19841,-26078,-19921,-26017,-20001,-25955,-20080,-25894,-20160,-25832,-20239,-25770,-20318,-25708,-20397,-25646,-20475,-25583,-20554,-25520,-20632,-25457,-20710,-25393,-20788,-25330,-20865,-25266,-20943,-25202,-21020,-25137,-21097,-25073,-21174,-25008,-21250,-24943,-21327,-24878,-21403,-24812,-21479,-24746,-21555,-24680,-21630,-24614,-21706,-24547,-21781,-24481,-21856,-24414,-21931,-24347,-22005,-24279,-22080,-24212,-22154,-24144,-22228,-24076,-22302,-24007,-22375,-23939,-22449,-23870,-22522,-23801,-22595,-23732,-22667,-23662,-22740,-23593,-22812,-23523,-22884,-23453,-22956,-23383,-23028,-23312,-23099,-23241,-23170,-23170,-23241,-23099,-23312,-23028,-23383,-22956,-23453,-22884,-23523,-22812,-23593,-22740,-23662,-22667,-23732,-22595,-23801,-22522,-23870,-22449,-23939,-22375,-24007,-22302,-24076,-22228,-24144,-22154,-24212,-22080,-24279,-22005,-24347,-21931,-24414,-21856,-24481,-21781,-24547,-21706,-24614,-21630,-24680,-21555,-24746,-21479,-24812,-21403,-24878,-21327,-24943,-21250,-25008,-21174,-25073,-21097,-25137,-21020,-25202,-20943,-25266,-20865,-25330,-20788,-25393,-20710,-25457,-20632,-25520,-20554,-25583,-20475,-25646,-20397,-25708,-20318,-25770,-20239,-25832,-20160,-25894,-20080,-25955,-20001,-26017,-19921,-26078,-19841,-26138,-19761,-26199,-19681,-26259,-19600,-26319,-19520,-26379,-19439,-26438,-19358,-26498,-19277,-26557,-19195,-26616,-19114,-26674,-19032,-26732,-18950,-26790,-18868,-26848,-18786,-26906,-18703,-26963,-18621,-27020,-18538,-27077,-18455,-27133,-18372,-27189,-18288,-27245,-18205,-27301,-18121,-27356,-18037,-27412,-17953,-27467,-17869,-27521,-17785,-27576,-17700,-27630,-17616,-27684,-17531,-27737,-17446,-27791,-17361,-27844,-17275,-27897,-17190,-27949,-17104,-28002,-17018,-28054,-16932,-28106,-16846,-28157,-16760,-28209,-16673,-28260,-16587,-28310,-16500,-28361,-16413,-28411,-16326,-28461,-16239,-28511,-16151,-28560,-16064,-28609,-15976,-28658,-15888,-28707,-15800,-28755,-15712,-28803,-15624,-28851,-15535,-28898,-15447,-28946,-15358,-28993,-15269,-29039,-15180,-29086,-15091,-29132,-15002,-29178,-14912,-29223,-14823,-29269,-14733,-29314,-14643,-29359,-14553,-29403,-14463,-29447,-14373,-29491,-14282,-29535,-14192,-29578,-14101,-29622,-14010,-29664,-13919,-29707,-13828,-29749,-13737,-29791,-13646,-29833,-13554,-29874,-13463,-29916,-13371,-29956,-13279,-29997,-13187,-30037,-13095,-30077,-13003,-30117,-12910,-30157,-12818,-30196,-12725,-30235,-12633,-30273,-12540,-30312,-12447,-30350,-12354,-30387,-12261,-30425,-12167,-30462,-12074,-30499,-11981,-30536,-11887,-30572,-11793,-30608,-11699,-30644,-11605,-30679,-11511,-30714,-11417,-30749,-11323,-30784,-11228,-30818,-11134,-30852,-11039,-30886,-10945,-30919,-10850,-30952,-10755,-30985,-10660,-31018,-10565,-31050,-10470,-31082,-10374,-31114,-10279,-31145,-10183,-31176,-10088,-31207,-9992,-31237,-9896,-31268,-9800,-31298,-9704,-31327,-9608,-31357,-9512,-31386,-9416,-31414,-9320,-31443,-9223,-31471,-9127,-31499,-9030,-31526,-8933,-31554,-8837,-31581,-8740,-31607,-8643,-31634,-8546,-31660,-8449,-31685,-8352,-31711,-8254,-31736,-8157,-31761,-8060,-31786,-7962,-31810,-7865,-31834,-7767,-31857,-7669,-31881,-7572,-31904,-7474,-31927,-7376,-31949,-7278,-31971,-7180,-31993,-7082,-32015,-6983,-32036,-6885,-32057,-6787,-32078,-6689,-32098,-6590,-32118,-6492,-32138,-6393,-32157,-6294,-32177,-6196,-32195,-6097,-32214,-5998,-32232,-5899,-32250,-5800,-32268,-5701,-32285,-5602,-32302,-5503,-32319,-5404,-32335,-5305,-32351,-5206,-32367,-5107,-32383,-5007,-32398,-4908,-32413,-4808,-32427,-4709,-32442,-4609,-32456,-4510,-32469,-4410,-32483,-4311,-32496,-4211,-32509,-4111,-32521,-4012,-32533,-3912,-32545,-3812,-32557,-3712,-32568,-3612,-32579,-3512,-32589,-3412,-32600,-3312,-32610,-3212,-32619,-3112,-32629,-3012,-32638,-2912,-32647,-2812,-32655,-2712,-32663,-2611,-32671,-2511,-32679,-2411,-32686,-2311,-32693,-2210,-32700,-2110,-32706,-2010,-32712,-1909,-32718,-1809,-32723,-1709,-32728,-1608,-32733,-1508,-32737,-1407,-32741,-1307,-32745,-1207,-32749,-1106,-32752,-1006,-32755,-905,-32758,-805,-32760,-704,-32762,-604,-32764,-503,-32765,-403,-32766,-302,-32767,-202,-32767,-101};


void dft2048(int16_t *x,int16_t *y,int scale) {

  __m64 xtmp[2048],*xtmpp,*x64 = (__m64 *)x;
  __m128i ytmp[512],*tw2048_128p=(__m128i *)tw2048,*y128=(__m128i *)y,*y128p=(__m128i *)y;
  __m128i *ytmpp = &ytmp[0];
  int i;
  __m128i ONE_OVER_SQRT2_Q15_128 = _mm_set_epi16(ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15);

  xtmpp = xtmp;

  for (i=0;i<16;i++) {
    transpose4_ooff(x64  ,xtmpp,512);
    transpose4_ooff(x64+2,xtmpp+1,512);
    transpose4_ooff(x64+4,xtmpp+2,512);
    transpose4_ooff(x64+6,xtmpp+3,512);
    transpose4_ooff(x64+8,xtmpp+4,512);
    transpose4_ooff(x64+10,xtmpp+5,512);
    transpose4_ooff(x64+12,xtmpp+6,512);
    transpose4_ooff(x64+14,xtmpp+7,512);
    transpose4_ooff(x64+16,xtmpp+8,512);
    transpose4_ooff(x64+18,xtmpp+9,512);
    transpose4_ooff(x64+20,xtmpp+10,512);
    transpose4_ooff(x64+22,xtmpp+11,512);
    transpose4_ooff(x64+24,xtmpp+12,512);
    transpose4_ooff(x64+26,xtmpp+13,512);
    transpose4_ooff(x64+28,xtmpp+14,512);
    transpose4_ooff(x64+30,xtmpp+15,512);
    transpose4_ooff(x64+32,xtmpp+16,512);
    transpose4_ooff(x64+34,xtmpp+17,512);
    transpose4_ooff(x64+36,xtmpp+18,512);
    transpose4_ooff(x64+38,xtmpp+19,512);
    transpose4_ooff(x64+40,xtmpp+20,512);
    transpose4_ooff(x64+42,xtmpp+21,512);
    transpose4_ooff(x64+44,xtmpp+22,512);
    transpose4_ooff(x64+46,xtmpp+23,512);
    transpose4_ooff(x64+48,xtmpp+24,512);
    transpose4_ooff(x64+50,xtmpp+25,512);
    transpose4_ooff(x64+52,xtmpp+26,512);
    transpose4_ooff(x64+54,xtmpp+27,512);
    transpose4_ooff(x64+56,xtmpp+28,512);
    transpose4_ooff(x64+58,xtmpp+29,512);
    transpose4_ooff(x64+60,xtmpp+30,512);
    transpose4_ooff(x64+62,xtmpp+31,512);
    x64+=64;
    xtmpp+=32;
  }

  dft1024((int16_t*)(xtmp),(int16_t*)ytmp,1);
  dft1024((int16_t*)(xtmp+512),(int16_t*)(ytmp+256),1);


  for (i=0;i<256;i++) {
    bfly2(ytmpp,ytmpp+256,
	  y128p,y128p+256,
	  tw2048_128p);
    tw2048_128p++;
    y128p++;
    ytmpp++;    
  }

  if (scale>0) {
    y128p = y128;
    for (i=0;i<32;i++) {
      y128p[0]  = _mm_mulhi_epi16(y128p[0],ONE_OVER_SQRT2_Q15_128);y128p[0] = _mm_slli_epi16(y128p[0],1);
      y128p[1]  = _mm_mulhi_epi16(y128p[1],ONE_OVER_SQRT2_Q15_128);y128p[1] = _mm_slli_epi16(y128p[1],1);
      y128p[2]  = _mm_mulhi_epi16(y128p[2],ONE_OVER_SQRT2_Q15_128);y128p[2] = _mm_slli_epi16(y128p[2],1);
      y128p[3]  = _mm_mulhi_epi16(y128p[3],ONE_OVER_SQRT2_Q15_128);y128p[3] = _mm_slli_epi16(y128p[3],1);
      y128p[4]  = _mm_mulhi_epi16(y128p[4],ONE_OVER_SQRT2_Q15_128);y128p[4] = _mm_slli_epi16(y128p[4],1);
      y128p[5]  = _mm_mulhi_epi16(y128p[5],ONE_OVER_SQRT2_Q15_128);y128p[5] = _mm_slli_epi16(y128p[5],1);
      y128p[6]  = _mm_mulhi_epi16(y128p[6],ONE_OVER_SQRT2_Q15_128);y128p[6] = _mm_slli_epi16(y128p[6],1);
      y128p[7]  = _mm_mulhi_epi16(y128p[7],ONE_OVER_SQRT2_Q15_128);y128p[7] = _mm_slli_epi16(y128p[7],1);
      y128p[8]  = _mm_mulhi_epi16(y128p[8],ONE_OVER_SQRT2_Q15_128);y128p[8] = _mm_slli_epi16(y128p[8],1);
      y128p[9]  = _mm_mulhi_epi16(y128p[9],ONE_OVER_SQRT2_Q15_128);y128p[9] = _mm_slli_epi16(y128p[9],1);
      y128p[10] = _mm_mulhi_epi16(y128p[10],ONE_OVER_SQRT2_Q15_128);y128p[10] = _mm_slli_epi16(y128p[10],1);
      y128p[11] = _mm_mulhi_epi16(y128p[11],ONE_OVER_SQRT2_Q15_128);y128p[11] = _mm_slli_epi16(y128p[11],1);
      y128p[12] = _mm_mulhi_epi16(y128p[12],ONE_OVER_SQRT2_Q15_128);y128p[12] = _mm_slli_epi16(y128p[12],1);
      y128p[13] = _mm_mulhi_epi16(y128p[13],ONE_OVER_SQRT2_Q15_128);y128p[13] = _mm_slli_epi16(y128p[13],1);
      y128p[14] = _mm_mulhi_epi16(y128p[14],ONE_OVER_SQRT2_Q15_128);y128p[14] = _mm_slli_epi16(y128p[14],1);
      y128p[15] = _mm_mulhi_epi16(y128p[15],ONE_OVER_SQRT2_Q15_128);y128p[15] = _mm_slli_epi16(y128p[15],1);
      y128p+=16;
    }
  }
  _mm_empty();
  _m_empty();

}

void idft2048(int16_t *x,int16_t *y,int scale) {

  __m64 xtmp[2048],*xtmpp,*x64 = (__m64 *)x;
  __m128i ytmp[512],*tw2048_128p=(__m128i *)tw2048,*y128=(__m128i *)y,*y128p=(__m128i *)y;
  __m128i *ytmpp = &ytmp[0];
  int i;
  __m128i ONE_OVER_SQRT2_Q15_128 = _mm_set_epi16(ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15,
						 ONE_OVER_SQRT2_Q15);

  xtmpp = xtmp;

  for (i=0;i<16;i++) {
    transpose4_ooff(x64  ,xtmpp,512);
    transpose4_ooff(x64+2,xtmpp+1,512);
    transpose4_ooff(x64+4,xtmpp+2,512);
    transpose4_ooff(x64+6,xtmpp+3,512);
    transpose4_ooff(x64+8,xtmpp+4,512);
    transpose4_ooff(x64+10,xtmpp+5,512);
    transpose4_ooff(x64+12,xtmpp+6,512);
    transpose4_ooff(x64+14,xtmpp+7,512);
    transpose4_ooff(x64+16,xtmpp+8,512);
    transpose4_ooff(x64+18,xtmpp+9,512);
    transpose4_ooff(x64+20,xtmpp+10,512);
    transpose4_ooff(x64+22,xtmpp+11,512);
    transpose4_ooff(x64+24,xtmpp+12,512);
    transpose4_ooff(x64+26,xtmpp+13,512);
    transpose4_ooff(x64+28,xtmpp+14,512);
    transpose4_ooff(x64+30,xtmpp+15,512);
    transpose4_ooff(x64+32,xtmpp+16,512);
    transpose4_ooff(x64+34,xtmpp+17,512);
    transpose4_ooff(x64+36,xtmpp+18,512);
    transpose4_ooff(x64+38,xtmpp+19,512);
    transpose4_ooff(x64+40,xtmpp+20,512);
    transpose4_ooff(x64+42,xtmpp+21,512);
    transpose4_ooff(x64+44,xtmpp+22,512);
    transpose4_ooff(x64+46,xtmpp+23,512);
    transpose4_ooff(x64+48,xtmpp+24,512);
    transpose4_ooff(x64+50,xtmpp+25,512);
    transpose4_ooff(x64+52,xtmpp+26,512);
    transpose4_ooff(x64+54,xtmpp+27,512);
    transpose4_ooff(x64+56,xtmpp+28,512);
    transpose4_ooff(x64+58,xtmpp+29,512);
    transpose4_ooff(x64+60,xtmpp+30,512);
    transpose4_ooff(x64+62,xtmpp+31,512);
    x64+=64;
    xtmpp+=32;
  }

  idft1024((int16_t*)(xtmp),(int16_t*)ytmp,1);
  idft1024((int16_t*)(xtmp+512),(int16_t*)(ytmp+256),1);


  for (i=0;i<256;i++) {
    ibfly2(ytmpp,ytmpp+256,
	   y128p,y128p+256,
	   tw2048_128p);
    tw2048_128p++;
    y128p++;
    ytmpp++;    
  }

  if (scale>0) {
    y128p = y128;
    for (i=0;i<32;i++) {
      y128p[0]  = _mm_mulhi_epi16(y128p[0],ONE_OVER_SQRT2_Q15_128);y128p[0] = _mm_slli_epi16(y128p[0],1);
      y128p[1]  = _mm_mulhi_epi16(y128p[1],ONE_OVER_SQRT2_Q15_128);y128p[1] = _mm_slli_epi16(y128p[1],1);
      y128p[2]  = _mm_mulhi_epi16(y128p[2],ONE_OVER_SQRT2_Q15_128);y128p[2] = _mm_slli_epi16(y128p[2],1);
      y128p[3]  = _mm_mulhi_epi16(y128p[3],ONE_OVER_SQRT2_Q15_128);y128p[3] = _mm_slli_epi16(y128p[3],1);
      y128p[4]  = _mm_mulhi_epi16(y128p[4],ONE_OVER_SQRT2_Q15_128);y128p[4] = _mm_slli_epi16(y128p[4],1);
      y128p[5]  = _mm_mulhi_epi16(y128p[5],ONE_OVER_SQRT2_Q15_128);y128p[5] = _mm_slli_epi16(y128p[5],1);
      y128p[6]  = _mm_mulhi_epi16(y128p[6],ONE_OVER_SQRT2_Q15_128);y128p[6] = _mm_slli_epi16(y128p[6],1);
      y128p[7]  = _mm_mulhi_epi16(y128p[7],ONE_OVER_SQRT2_Q15_128);y128p[7] = _mm_slli_epi16(y128p[7],1);
      y128p[8]  = _mm_mulhi_epi16(y128p[8],ONE_OVER_SQRT2_Q15_128);y128p[8] = _mm_slli_epi16(y128p[8],1);
      y128p[9]  = _mm_mulhi_epi16(y128p[9],ONE_OVER_SQRT2_Q15_128);y128p[9] = _mm_slli_epi16(y128p[9],1);
      y128p[10] = _mm_mulhi_epi16(y128p[10],ONE_OVER_SQRT2_Q15_128);y128p[10] = _mm_slli_epi16(y128p[10],1);
      y128p[11] = _mm_mulhi_epi16(y128p[11],ONE_OVER_SQRT2_Q15_128);y128p[11] = _mm_slli_epi16(y128p[11],1);
      y128p[12] = _mm_mulhi_epi16(y128p[12],ONE_OVER_SQRT2_Q15_128);y128p[12] = _mm_slli_epi16(y128p[12],1);
      y128p[13] = _mm_mulhi_epi16(y128p[13],ONE_OVER_SQRT2_Q15_128);y128p[13] = _mm_slli_epi16(y128p[13],1);
      y128p[14] = _mm_mulhi_epi16(y128p[14],ONE_OVER_SQRT2_Q15_128);y128p[14] = _mm_slli_epi16(y128p[14],1);
      y128p[15] = _mm_mulhi_epi16(y128p[15],ONE_OVER_SQRT2_Q15_128);y128p[15] = _mm_slli_epi16(y128p[15],1);
      y128p+=16;
    }
  }
  _mm_empty();
  _m_empty();

}

#include "twiddle1536.h"
// 512 x 3
void ifft1536(int16_t *input, int16_t *output) {
  int i,i2,j;
  uint32_t tmp[3][1024 ]__attribute__((aligned(16)));
  uint32_t tmpo[3][1024] __attribute__((aligned(16)));
  
  for (i=0,j=0;i<1024;i+=2) {
    ((int16_t*)tmp[0])[i]   = input[j++];
    ((int16_t*)tmp[0])[i+1]   = -input[j++];
    ((int16_t*)tmp[1])[i]   = input[j++];
    ((int16_t*)tmp[1])[i+1]   = -input[j++];
    ((int16_t*)tmp[2])[i]   = input[j++];
    ((int16_t*)tmp[2])[i+1]   = -input[j++];
  }
  fft((int16_t*)(tmp[0]),(int16_t*)(tmpo[0]),twiddle_fft512,rev512,9,4,0);
  fft((int16_t*)(tmp[1]),(int16_t*)(tmpo[1]),twiddle_fft512,rev512,9,4,0);
  fft((int16_t*)(tmp[2]),(int16_t*)(tmpo[2]),twiddle_fft512,rev512,9,4,0);
  for (i=1;i<512;i++) {
    tmpo[0][i] = tmpo[0][i<<1];
    tmpo[1][i] = tmpo[1][i<<1];
    tmpo[2][i] = tmpo[2][i<<1];
  }

  //  write_output("in.m","in",input,6144,1,1);
  //  write_output("out0.m","o0",tmpo[0],2048,1,1);
  //  write_output("out1.m","o1",tmpo[1],2048,1,1);
  //  write_output("out2.m","o2",tmpo[2],2048,1,1);
  
  for (i=0,i2=0;i<1024;i+=8,i2+=4)  {
    bfly3((__m128i*)(&tmpo[0][i2]),(__m128i*)(&tmpo[1][i2]),((__m128i*)&tmpo[2][i2]),
	  (__m128i*)(output+i),(__m128i*)(output+1024+i),(__m128i*)(output+2048+i),
	  (__m128i*)(twa1536+i),(__m128i*)(twb1536+i));
  }
  for (i=1;i<3072;i+=2)
    output[i] = -output[i];

  _mm_empty();
  _m_empty();

}

void fft1536(int16_t *input, int16_t *output) {
  int i,i2,j;
  uint32_t tmp[3][1024] __attribute__((aligned(16)));
  uint32_t tmpo[3][1024] __attribute__((aligned(16)));

  for (i=0,j=0;i<512;i++) {
    tmp[0][i] = ((uint32_t *)input)[j++];
    tmp[1][i] = ((uint32_t *)input)[j++];
    tmp[2][i] = ((uint32_t *)input)[j++];
  }
  fft((int16_t*)(tmp[0]),(int16_t*)(tmpo[0]),twiddle_fft512,rev512,9,4,0);
  fft((int16_t*)(tmp[1]),(int16_t*)(tmpo[1]),twiddle_fft512,rev512,9,4,0);
  fft((int16_t*)(tmp[2]),(int16_t*)(tmpo[2]),twiddle_fft512,rev512,9,4,0);

  for (i=1;i<512;i++) {
    tmpo[0][i] = tmpo[0][i<<1];
    tmpo[1][i] = tmpo[1][i<<1];
    tmpo[2][i] = tmpo[2][i<<1];
  }
  //  write_output("out0.m","o0",tmpo[0],2048,1,1);
  //  write_output("out1.m","o1",tmpo[1],2048,1,1);
  //  write_output("out2.m","o2",tmpo[2],2048,1,1);
  for (i=0,i2=0;i<1024;i+=8,i2+=4)  {
    bfly3((__m128i*)(&tmpo[0][i2]),(__m128i*)(&tmpo[1][i2]),(__m128i*)(&tmpo[2][i2]),
	  (__m128i*)(output+i),(__m128i*)(output+1024+i),(__m128i*)(output+2048+i),
	  (__m128i*)(twa1536+i),(__m128i*)(twb1536+i));
  }
  _mm_empty();
  _m_empty();

}

// 1024 x 3
void fft3072(int16_t *input, int16_t *output) {

}

void ifft3072(int16_t *input, int16_t *output) {

}

#include "twiddle6144.h"

void ifft6144(int16_t *input, int16_t *output) {
  int i,i2,j;
  uint32_t tmp[3][4096] __attribute__((aligned(16)));
  uint32_t tmpo[3][4096] __attribute__((aligned(16)));

  for (i=0,j=0;i<4096;i+=2) {
    ((int16_t*)tmp[0])[i]   = input[j++];
    ((int16_t*)tmp[0])[i+1]   = -input[j++];
    ((int16_t*)tmp[1])[i]   = input[j++];
    ((int16_t*)tmp[1])[i+1]   = -input[j++];
    ((int16_t*)tmp[2])[i]   = input[j++];
    ((int16_t*)tmp[2])[i+1]   = -input[j++];
  }
  fft((int16_t*)(tmp[0]),(int16_t*)(tmpo[0]),twiddle_fft2048,rev2048,11,5,0);
  fft((int16_t*)(tmp[1]),(int16_t*)(tmpo[1]),twiddle_fft2048,rev2048,11,5,0);
  fft((int16_t*)(tmp[2]),(int16_t*)(tmpo[2]),twiddle_fft2048,rev2048,11,5,0);
  for (i=1;i<2048;i++) {
    tmpo[0][i] = tmpo[0][i<<1];
    tmpo[1][i] = tmpo[1][i<<1];
    tmpo[2][i] = tmpo[2][i<<1];
  }

  //  write_output("in.m","in",input,6144,1,1);
  //  write_output("out0.m","o0",tmpo[0],2048,1,1);
  //  write_output("out1.m","o1",tmpo[1],2048,1,1);
  //  write_output("out2.m","o2",tmpo[2],2048,1,1);
  
  for (i=0,i2=0;i<4096;i+=8,i2+=4)  {
    bfly3((__m128i*)(&tmpo[0][i2]),(__m128i*)(&tmpo[1][i2]),((__m128i*)&tmpo[2][i2]),
	  (__m128i*)(output+i),(__m128i*)(output+4096+i),(__m128i*)(output+8192+i),
	  (__m128i*)(twa6144+i),(__m128i*)(twb6144+i));
  }
  for (i=1;i<12288;i+=2)
    output[i] = -output[i];

  //  write_output("out.m","out",output,6144,1,1);    
  _mm_empty();
  _m_empty();

}


void fft6144(int16_t *input, int16_t *output) {
  int i,i2,j;
  uint32_t tmp[3][4096] __attribute__((aligned(16)));
  uint32_t tmpo[3][4096] __attribute__((aligned(16)));

  for (i=0,j=0;i<2048;i++) {
    tmp[0][i] = ((uint32_t *)input)[j++];
    tmp[1][i] = ((uint32_t *)input)[j++];
    tmp[2][i] = ((uint32_t *)input)[j++];
  }
  fft((int16_t*)(tmp[0]),(int16_t*)(tmpo[0]),twiddle_fft2048,rev2048,11,5,0);
  fft((int16_t*)(tmp[1]),(int16_t*)(tmpo[1]),twiddle_fft2048,rev2048,11,5,0);
  fft((int16_t*)(tmp[2]),(int16_t*)(tmpo[2]),twiddle_fft2048,rev2048,11,5,0);

  for (i=1;i<2048;i++) {
    tmpo[0][i] = tmpo[0][i<<1];
    tmpo[1][i] = tmpo[1][i<<1];
    tmpo[2][i] = tmpo[2][i<<1];
  }
  //  write_output("out0.m","o0",tmpo[0],2048,1,1);
  //  write_output("out1.m","o1",tmpo[1],2048,1,1);
  //  write_output("out2.m","o2",tmpo[2],2048,1,1);
  for (i=0,i2=0;i<4096;i+=8,i2+=4)  {
    bfly3((__m128i*)(&tmpo[0][i2]),(__m128i*)(&tmpo[1][i2]),(__m128i*)(&tmpo[2][i2]),
	  (__m128i*)(output+i),(__m128i*)(output+4096+i),(__m128i*)(output+8192+i),
	  (__m128i*)(twa6144+i),(__m128i*)(twb6144+i));
  }
  _mm_empty();
  _m_empty();

}

#include "twiddle12288.h"

// 4096 x 3
void fft12288(int16_t *input, int16_t *output) {
  int i,i2,j;
  uint32_t tmp[3][8192] __attribute__((aligned(16)));
  uint32_t tmpo[3][8192] __attribute__((aligned(16)));

  for (i=0,j=0;i<4096;i++) {
    tmp[0][i] = ((uint32_t *)input)[j++];
    tmp[1][i] = ((uint32_t *)input)[j++];
    tmp[2][i] = ((uint32_t *)input)[j++];
  }
  fft((int16_t*)(tmp[0]),(int16_t*)(tmpo[0]),twiddle_fft4096,rev4096,12,6,0);
  fft((int16_t*)(tmp[1]),(int16_t*)(tmpo[1]),twiddle_fft4096,rev4096,12,6,0);
  fft((int16_t*)(tmp[2]),(int16_t*)(tmpo[2]),twiddle_fft4096,rev4096,12,6,0);

  for (i=1;i<4096;i++) {
    tmpo[0][i] = tmpo[0][i<<1];
    tmpo[1][i] = tmpo[1][i<<1];
    tmpo[2][i] = tmpo[2][i<<1];
  }
  //  write_output("out0.m","o0",tmpo[0],4096,1,1);
  //  write_output("out1.m","o1",tmpo[1],4096,1,1);
  //  write_output("out2.m","o2",tmpo[2],4096,1,1);
  for (i=0,i2=0;i<8192;i+=8,i2+=4)  {
    bfly3((__m128i*)(&tmpo[0][i2]),(__m128i*)(&tmpo[1][i2]),(__m128i*)(&tmpo[2][i2]),
	  (__m128i*)(output+i),(__m128i*)(output+8192+i),(__m128i*)(output+16384+i),
	  (__m128i*)(twa12288+i),(__m128i*)(twb12288+i));
  }
  _mm_empty();
  _m_empty();

}

void ifft12288(int16_t *input, int16_t *output) {
  int i,i2,j;
  uint32_t tmp[3][8192] __attribute__((aligned(16)));
  uint32_t tmpo[3][8192] __attribute__((aligned(16)));

  for (i=0,j=0;i<8192;i+=2) {
    ((int16_t*)tmp[0])[i]   = input[j++];
    ((int16_t*)tmp[0])[i+1]   = -input[j++];
    ((int16_t*)tmp[1])[i]   = input[j++];
    ((int16_t*)tmp[1])[i+1]   = -input[j++];
    ((int16_t*)tmp[2])[i]   = input[j++];
    ((int16_t*)tmp[2])[i+1]   = -input[j++];
  }
  fft((int16_t*)(tmp[0]),(int16_t*)(tmpo[0]),twiddle_fft4096,rev4096,12,6,0);
  fft((int16_t*)(tmp[1]),(int16_t*)(tmpo[1]),twiddle_fft4096,rev4096,12,6,0);
  fft((int16_t*)(tmp[2]),(int16_t*)(tmpo[2]),twiddle_fft4096,rev4096,12,6,0);
  for (i=1;i<4096;i++) {
    tmpo[0][i] = tmpo[0][i<<1];
    tmpo[1][i] = tmpo[1][i<<1];
    tmpo[2][i] = tmpo[2][i<<1];
  }

  //  write_output("in.m","in",input,6144,1,1);
  //  write_output("out0.m","o0",tmpo[0],4096,1,1);
  //  write_output("out1.m","o1",tmpo[1],4096,1,1);
  //  write_output("out2.m","o2",tmpo[2],4096,1,1);
  
  for (i=0,i2=0;i<8192;i+=8,i2+=4)  {
    bfly3((__m128i*)(&tmpo[0][i2]),(__m128i*)(&tmpo[1][i2]),((__m128i*)&tmpo[2][i2]),
	  (__m128i*)(output+i),(__m128i*)(output+8192+i),(__m128i*)(output+16384+i),
	  (__m128i*)(twa12288+i),(__m128i*)(twb12288+i));
  }
  for (i=1;i<24576;i+=2)
    output[i] = -output[i];
  _mm_empty();
  _m_empty();

  //  write_output("out.m","out",output,6144,1,1); 
}

// 6144 x 3
void fft18432(int16_t *input, int16_t *output) {

}

void ifft18432(int16_t *input, int16_t *output) {

}

#include "twiddle24576.h"
// 8192 x 3
void fft24576(int16_t *input, int16_t *output) {
  int i,i2,j;
  uint32_t tmp[3][16384] __attribute__((aligned(16)));
  uint32_t tmpo[3][16384] __attribute__((aligned(16)));

  for (i=0,j=0;i<8192;i++) {
    tmp[0][i] = ((uint32_t *)input)[j++];
    tmp[1][i] = ((uint32_t *)input)[j++];
    tmp[2][i] = ((uint32_t *)input)[j++];
  }
  fft((int16_t*)(tmp[0]),(int16_t*)(tmpo[0]),twiddle_fft8192,rev8192,13,6,0);
  fft((int16_t*)(tmp[1]),(int16_t*)(tmpo[1]),twiddle_fft8192,rev8192,13,6,0);
  fft((int16_t*)(tmp[2]),(int16_t*)(tmpo[2]),twiddle_fft8192,rev8192,13,6,0);

  for (i=1;i<8192;i++) {
    tmpo[0][i] = tmpo[0][i<<1];
    tmpo[1][i] = tmpo[1][i<<1];
    tmpo[2][i] = tmpo[2][i<<1];
  }
  //   write_output("out0.m","o0",tmpo[0],8192,1,1);
  //    write_output("out1.m","o1",tmpo[1],8192,1,1);
  //    write_output("out2.m","o2",tmpo[2],8192,1,1);
  for (i=0,i2=0;i<16384;i+=8,i2+=4)  {
    bfly3((__m128i*)(&tmpo[0][i2]),(__m128i*)(&tmpo[1][i2]),(__m128i*)(&tmpo[2][i2]),
	  (__m128i*)(output+i),(__m128i*)(output+16384+i),(__m128i*)(output+32768+i),
	  (__m128i*)(twa24576+i),(__m128i*)(twb24576+i));
  }
  _mm_empty();
  _m_empty();

  //  write_output("out.m","out",output,24576,1,1); 
}

void ifft24576(int16_t *input, int16_t *output) {
  int i,i2,j;
  uint32_t tmp[3][16384] __attribute__((aligned(16)));
  uint32_t tmpo[3][16384] __attribute__((aligned(16)));

  for (i=0,j=0;i<16384;i+=2) {
    ((int16_t*)tmp[0])[i]   = input[j++];
    ((int16_t*)tmp[0])[i+1]   = -input[j++];
    ((int16_t*)tmp[1])[i]   = input[j++];
    ((int16_t*)tmp[1])[i+1]   = -input[j++];
    ((int16_t*)tmp[2])[i]   = input[j++];
    ((int16_t*)tmp[2])[i+1]   = -input[j++];
  }
  fft((int16_t*)(tmp[0]),(int16_t*)(tmpo[0]),twiddle_fft8192,rev8192,13,6,0);
  fft((int16_t*)(tmp[1]),(int16_t*)(tmpo[1]),twiddle_fft8192,rev8192,13,6,0);
  fft((int16_t*)(tmp[2]),(int16_t*)(tmpo[2]),twiddle_fft8192,rev8192,13,6,0);
  for (i=1;i<8192;i++) {
    tmpo[0][i] = tmpo[0][i<<1];
    tmpo[1][i] = tmpo[1][i<<1];
    tmpo[2][i] = tmpo[2][i<<1];
  }
  /*  
    write_output("in.m","in",input,24576,1,1);
    write_output("out0.m","o0",tmpo[0],8192,1,1);
    write_output("out1.m","o1",tmpo[1],8192,1,1);
    write_output("out2.m","o2",tmpo[2],8192,1,1);
  */

  for (i=0,i2=0;i<16384;i+=8,i2+=4)  {
    bfly3((__m128i*)(&tmpo[0][i2]),(__m128i*)(&tmpo[1][i2]),((__m128i*)&tmpo[2][i2]),
	  (__m128i*)(output+i),(__m128i*)(output+16384+i),(__m128i*)(output+32768+i),
	  (__m128i*)(twa24576+i),(__m128i*)(twb24576+i));
  }
  for (i=1;i<(24576*2);i+=2)
    output[i] = -output[i];

  _mm_empty();
  _m_empty();

  //  write_output("out.m","out",output,24576,1,1); 
}

///  THIS SECTION IS FOR ALL PUSCH DFTS (i.e. radix 2^a * 3^b * 4^c * 5^d)
///  They use twiddles for 4-way parallel DFTS (i.e. 4 DFTS with interleaved input/output)

static int16_t W1_12s[8]__attribute__((aligned(16))) = {28377,-16383,28377,-16383,28377,-16383,28377,-16383};
static int16_t W2_12s[8]__attribute__((aligned(16))) = {16383,-28377,16383,-28377,16383,-28377,16383,-28377};
static int16_t W3_12s[8]__attribute__((aligned(16))) = {0,-32767,0,-32767,0,-32767,0,-32767};
static int16_t W4_12s[8]__attribute__((aligned(16))) = {-16383,-28377,-16383,-28377,-16383,-28377,-16383,-28377};
static int16_t W6_12s[8]__attribute__((aligned(16))) = {-32767,0,-32767,0,-32767,0,-32767,0};

__m128i *W1_12=(__m128i *)W1_12s;
__m128i *W2_12=(__m128i *)W2_12s;
__m128i *W3_12=(__m128i *)W3_12s;
__m128i *W4_12=(__m128i *)W4_12s;
__m128i *W6_12=(__m128i *)W6_12s;


static __m128i norm128;

static inline void dft12f(__m128i *x0,
			  __m128i *x1,
			  __m128i *x2,
			  __m128i *x3,
			  __m128i *x4,
			  __m128i *x5,
			  __m128i *x6,
			  __m128i *x7,
			  __m128i *x8,
			  __m128i *x9,
			  __m128i *x10,
			  __m128i *x11,
			  __m128i *y0,
			  __m128i *y1,
			  __m128i *y2,
			  __m128i *y3,
			  __m128i *y4,
			  __m128i *y5,
			  __m128i *y6,
			  __m128i *y7,
			  __m128i *y8,
			  __m128i *y9,
			  __m128i *y10,
			  __m128i *y11) __attribute__((always_inline));

static inline void dft12f(__m128i *x0,
			  __m128i *x1,
			  __m128i *x2,
			  __m128i *x3,
			  __m128i *x4,
			  __m128i *x5,
			  __m128i *x6,
			  __m128i *x7,
			  __m128i *x8,
			  __m128i *x9,
			  __m128i *x10,
			  __m128i *x11,
			  __m128i *y0,
			  __m128i *y1,
			  __m128i *y2,
			  __m128i *y3,
			  __m128i *y4,
			  __m128i *y5,
			  __m128i *y6,
			  __m128i *y7,
			  __m128i *y8,
			  __m128i *y9,
			  __m128i *y10,
			  __m128i *y11) {
   
  
  __m128i tmp_dft12[12];
  
  __m128i *tmp_dft12_ptr = &tmp_dft12[0];
  
  // msg("dft12\n");
  
  bfly4_tw1(x0, 
	    x3, 
	    x6, 
	    x9,
	    tmp_dft12_ptr,
	    tmp_dft12_ptr+3,
	    tmp_dft12_ptr+6,
	    tmp_dft12_ptr+9);
  
  
  bfly4_tw1(x1, 
	    x4, 
	    x7, 
	    x10,
	    tmp_dft12_ptr+1,
	    tmp_dft12_ptr+4,
	    tmp_dft12_ptr+7,
	    tmp_dft12_ptr+10);
  
  
  bfly4_tw1(x2, 
	    x5, 
	    x8, 
	    x11,
	    tmp_dft12_ptr+2,
	    tmp_dft12_ptr+5,
	    tmp_dft12_ptr+8,
	    tmp_dft12_ptr+11);
  
  //  k2=0;
  bfly3_tw1(tmp_dft12_ptr,
	    tmp_dft12_ptr+1,
	    tmp_dft12_ptr+2,
	    y0,
	    y4,
	    y8);
  
  
  
  //  k2=1;
  bfly3(tmp_dft12_ptr+3,
	tmp_dft12_ptr+4,
	tmp_dft12_ptr+5,
	y1,
	y5,
	y9,
 	W1_12,
        W2_12);
  
  
  
  //  k2=2;
  bfly3(tmp_dft12_ptr+6,
	tmp_dft12_ptr+7,
	tmp_dft12_ptr+8,
	y2,
	y6,
	y10,
	W2_12,
	W4_12);
  
  //  k2=3;
  bfly3(tmp_dft12_ptr+9,
	tmp_dft12_ptr+10,
	tmp_dft12_ptr+11,
	y3,
	y7,
	y11,	
	W3_12,
        W6_12);
  /*
  norm128 = _mm_set1_epi16(dft_norm_table[0]);

  *y0 = _mm_slli_epi16(_mm_mulhi_epi16(*y0,norm128),1);
  *y1 = _mm_slli_epi16(_mm_mulhi_epi16(*y1,norm128),1);
  *y2 = _mm_slli_epi16(_mm_mulhi_epi16(*y2,norm128),1);
  *y3 = _mm_slli_epi16(_mm_mulhi_epi16(*y3,norm128),1);
  *y4 = _mm_slli_epi16(_mm_mulhi_epi16(*y4,norm128),1);
  *y5 = _mm_slli_epi16(_mm_mulhi_epi16(*y5,norm128),1);
  *y6 = _mm_slli_epi16(_mm_mulhi_epi16(*y6,norm128),1);
  *y7 = _mm_slli_epi16(_mm_mulhi_epi16(*y7,norm128),1);
  *y8 = _mm_slli_epi16(_mm_mulhi_epi16(*y8,norm128),1);
  *y9 = _mm_slli_epi16(_mm_mulhi_epi16(*y9,norm128),1);
  *y10 = _mm_slli_epi16(_mm_mulhi_epi16(*y10,norm128),1);
  *y11 = _mm_slli_epi16(_mm_mulhi_epi16(*y11,norm128),1);
  */

}	    




void dft12(int16_t *x,int16_t *y) {

  __m128i *x128 = (__m128i *)x,*y128 = (__m128i *)y;
  dft12f(&x128[0],
	 &x128[1],
	 &x128[2],
	 &x128[3],
	 &x128[4],
	 &x128[5],
	 &x128[6],
	 &x128[7],
	 &x128[8],
	 &x128[9],
	 &x128[10],
	 &x128[11],
	 &y128[0],
	 &y128[1],
	 &y128[2],
	 &y128[3],
	 &y128[4],
	 &y128[5],
	 &y128[6],
	 &y128[7],
	 &y128[8],
	 &y128[9],
	 &y128[10],
	 &y128[11]);

  _mm_empty();
  _m_empty();

}

static int16_t tw24[88]__attribute__((aligned(16))) = {31650,-8480,31650,-8480,31650,-8480,31650,-8480,
						      28377,-16383,28377,-16383,28377,-16383,28377,-16383,
						      23169,-23169,23169,-23169,23169,-23169,23169,-23169,
						      16383,-28377,16383,-28377,16383,-28377,16383,-28377,
						      8480,-31650,8480,-31650,8480,-31650,8480,-31650,
						      0,-32767,0,-32767,0,-32767,0,-32767,
						      -8480,-31650,-8480,-31650,-8480,-31650,-8480,-31650,
						      -16383,-28377,-16383,-28377,-16383,-28377,-16383,-28377,
						      -23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
						      -28377,-16383,-28377,-16383,-28377,-16383,-28377,-16383,
						      -31650,-8480,-31650,-8480,-31650,-8480,-31650,-8480};

//static __m128i ytmp128array[300];
//static __m128i ytmp128array2[300];
//static __m128i ytmp128array3[300];
//static __m128i x2128array[300];

void dft24(int16_t *x,int16_t *y,unsigned char scale_flag) {
  
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *tw128=(__m128i *)&tw24[0];
  __m128i ytmp128[24];//=&ytmp128array[0];
  int i,j,k;

  //  msg("dft24\n");
  dft12f(x128,
	x128+2,
	x128+4,
	x128+6,
	x128+8,
	x128+10,
	x128+12,
	x128+14,
	x128+16,
	x128+18,
	x128+20,
	x128+22,
	ytmp128,
	ytmp128+2,
	ytmp128+4,
	ytmp128+6,
	ytmp128+8,
	ytmp128+10,
	ytmp128+12,
	ytmp128+14,
	ytmp128+16,
	ytmp128+18,
	ytmp128+20,
	ytmp128+22);
  //  msg("dft24b\n");

  dft12f(x128+1,
	x128+3,
	x128+5,
	x128+7,
	x128+9,
	x128+11,
	x128+13,
	x128+15,
	x128+17,
	x128+19,
	x128+21,
	x128+23,
	ytmp128+1,
	ytmp128+3,
	ytmp128+5,
	ytmp128+7,
	ytmp128+9,
	ytmp128+11,
	ytmp128+13,
	ytmp128+15,
	ytmp128+17,
	ytmp128+19,
	ytmp128+21,
	ytmp128+23);

  //  msg("dft24c\n");

  bfly2_tw1(ytmp128, 
	    ytmp128+1,
	    y128, 
	    y128+12);

  //  msg("dft24d\n");
  
  for (i=2,j=1,k=0;i<24;i+=2,j++,k++) {
    
    bfly2(ytmp128+i, 
	  ytmp128+i+1,
	  y128+j, 
	  y128+j+12,
	  tw128+k);
    //    msg("dft24e\n");
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(dft_norm_table[1]);
    
    for (i=0;i<24;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

}

static int16_t twa36[88]__attribute__((aligned(16))) = {32269,-5689,32269,-5689,32269,-5689,32269,-5689,
30790,-11206,30790,-11206,30790,-11206,30790,-11206,
28377,-16383,28377,-16383,28377,-16383,28377,-16383,
25100,-21062,25100,-21062,25100,-21062,25100,-21062,
21062,-25100,21062,-25100,21062,-25100,21062,-25100,
16383,-28377,16383,-28377,16383,-28377,16383,-28377,
11206,-30790,11206,-30790,11206,-30790,11206,-30790,
5689,-32269,5689,-32269,5689,-32269,5689,-32269,
0,-32767,0,-32767,0,-32767,0,-32767,
-5689,-32269,-5689,-32269,-5689,-32269,-5689,-32269,
-11206,-30790,-11206,-30790,-11206,-30790,-11206,-30790};

static int16_t twb36[88]__attribute__((aligned(16))) = {30790,-11206,30790,-11206,30790,-11206,30790,-11206,
25100,-21062,25100,-21062,25100,-21062,25100,-21062,
16383,-28377,16383,-28377,16383,-28377,16383,-28377,
5689,-32269,5689,-32269,5689,-32269,5689,-32269,
-5689,-32269,-5689,-32269,-5689,-32269,-5689,-32269,
-16383,-28377,-16383,-28377,-16383,-28377,-16383,-28377,
-25100,-21062,-25100,-21062,-25100,-21062,-25100,-21062,
-30790,-11206,-30790,-11206,-30790,-11206,-30790,-11206,
-32767,0,-32767,0,-32767,0,-32767,0,
-30790,11206,-30790,11206,-30790,11206,-30790,11206,
-25100,21062,-25100,21062,-25100,21062,-25100,21062};

void dft36(int16_t *x,int16_t *y,unsigned char scale_flag) {
  
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa36[0];
  __m128i *twb128=(__m128i *)&twb36[0];
  __m128i ytmp128[36];//&ytmp128array[0];


  int i,j,k;

  dft12f(x128,
	x128+3,
	x128+6,
	x128+9,
	x128+12,
	x128+15,
	x128+18,
	x128+21,
	x128+24,
	x128+27,
	x128+30,
	x128+33,
	ytmp128,
	ytmp128+3,
	ytmp128+6,
	ytmp128+9,
	ytmp128+12,
	ytmp128+15,
	ytmp128+18,
	ytmp128+21,
	ytmp128+24,
	ytmp128+27,
	ytmp128+30,
	ytmp128+33);

  dft12f(x128+1,
	x128+4,
	x128+7,
	x128+10,
	x128+13,
	x128+16,
	x128+19,
	x128+22,
	x128+25,
	x128+28,
	x128+31,
	x128+34,
	ytmp128+1,
	ytmp128+4,
	ytmp128+7,
	ytmp128+10,
	ytmp128+13,
	ytmp128+16,
	ytmp128+19,
	ytmp128+22,
	ytmp128+25,
	ytmp128+28,
	ytmp128+31,
	ytmp128+34);

  dft12f(x128+2,
	x128+5,
	x128+8,
	x128+11,
	x128+14,
	x128+17,
	x128+20,
	x128+23,
	x128+26,
	x128+29,
	x128+32,
	x128+35,
	ytmp128+2,
	ytmp128+5,
	ytmp128+8,
	ytmp128+11,
	ytmp128+14,
	ytmp128+17,
	ytmp128+20,
	ytmp128+23,
	ytmp128+26,
	ytmp128+29,
	ytmp128+32,
	ytmp128+35);


  bfly3_tw1(ytmp128, 
	    ytmp128+1,
	    ytmp128+2,
	    y128, 
	    y128+12,
	    y128+24);
  
  for (i=3,j=1,k=0;i<36;i+=3,j++,k++) {
    
    bfly3(ytmp128+i, 
	  ytmp128+i+1,
	  ytmp128+i+2,
	  y128+j, 
	  y128+j+12,
	  y128+j+24,
	  twa128+k,
	  twb128+k);
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(dft_norm_table[2]);
    
    for (i=0;i<36;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

}

static int16_t twa48[88]__attribute__((aligned(16))) = {32486,-4276,32486,-4276,32486,-4276,32486,-4276,
31650,-8480,31650,-8480,31650,-8480,31650,-8480,
30272,-12539,30272,-12539,30272,-12539,30272,-12539,
28377,-16383,28377,-16383,28377,-16383,28377,-16383,
25995,-19947,25995,-19947,25995,-19947,25995,-19947,
23169,-23169,23169,-23169,23169,-23169,23169,-23169,
19947,-25995,19947,-25995,19947,-25995,19947,-25995,
16383,-28377,16383,-28377,16383,-28377,16383,-28377,
12539,-30272,12539,-30272,12539,-30272,12539,-30272,
8480,-31650,8480,-31650,8480,-31650,8480,-31650,
4276,-32486,4276,-32486,4276,-32486,4276,-32486};

static int16_t twb48[88]__attribute__((aligned(16))) = {31650,-8480,31650,-8480,31650,-8480,31650,-8480,
28377,-16383,28377,-16383,28377,-16383,28377,-16383,
23169,-23169,23169,-23169,23169,-23169,23169,-23169,
16383,-28377,16383,-28377,16383,-28377,16383,-28377,
8480,-31650,8480,-31650,8480,-31650,8480,-31650,
0,-32767,0,-32767,0,-32767,0,-32767,
-8480,-31650,-8480,-31650,-8480,-31650,-8480,-31650,
-16383,-28377,-16383,-28377,-16383,-28377,-16383,-28377,
-23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
-28377,-16383,-28377,-16383,-28377,-16383,-28377,-16383,
-31650,-8480,-31650,-8480,-31650,-8480,-31650,-8480};

static int16_t twc48[88]__attribute__((aligned(16))) = {30272,-12539,30272,-12539,30272,-12539,30272,-12539,
23169,-23169,23169,-23169,23169,-23169,23169,-23169,
12539,-30272,12539,-30272,12539,-30272,12539,-30272,
0,-32767,0,-32767,0,-32767,0,-32767,
-12539,-30272,-12539,-30272,-12539,-30272,-12539,-30272,
-23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
-30272,-12539,-30272,-12539,-30272,-12539,-30272,-12539,
-32767,0,-32767,0,-32767,0,-32767,0,
-30272,12539,-30272,12539,-30272,12539,-30272,12539,
-23169,23169,-23169,23169,-23169,23169,-23169,23169,
-12539,30272,-12539,30272,-12539,30272,-12539,30272};

void dft48(int16_t *x, int16_t *y,unsigned char scale_flag) {
  
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa48[0];
  __m128i *twb128=(__m128i *)&twb48[0];
  __m128i *twc128=(__m128i *)&twc48[0];
  __m128i ytmp128[48];//=&ytmp128array[0];
  int i,j,k;


  dft12f(x128,
	x128+4,
	x128+8,
	x128+12,
	x128+16,
	x128+20,
	x128+24,
	x128+28,
	x128+32,
	x128+36,
	x128+40,
	x128+44,
	ytmp128,
	ytmp128+4,
	ytmp128+8,
	ytmp128+12,
	ytmp128+16,
	ytmp128+20,
	ytmp128+24,
	ytmp128+28,
	ytmp128+32,
	ytmp128+36,
	ytmp128+40,
	ytmp128+44);


  dft12f(x128+1,
	x128+5,
	x128+9,
	x128+13,
	x128+17,
	x128+21,
	x128+25,
	x128+29,
	x128+33,
	x128+37,
	x128+41,
	x128+45,
	ytmp128+1,
	ytmp128+5,
	ytmp128+9,
	ytmp128+13,
	ytmp128+17,
	ytmp128+21,
	ytmp128+25,
	ytmp128+29,
	ytmp128+33,
	ytmp128+37,
	ytmp128+41,
	ytmp128+45);


  dft12f(x128+2,
	x128+6,
	x128+10,
	x128+14,
	x128+18,
	x128+22,
	x128+26,
	x128+30,
	x128+34,
	x128+38,
	x128+42,
	x128+46,
	ytmp128+2,
	ytmp128+6,
	ytmp128+10,
	ytmp128+14,
	ytmp128+18,
	ytmp128+22,
	ytmp128+26,
	ytmp128+30,
	ytmp128+34,
	ytmp128+38,
	ytmp128+42,
	ytmp128+46);


  dft12f(x128+3,
	x128+7,
	x128+11,
	x128+15,
	x128+19,
	x128+23,
	x128+27,
	x128+31,
	x128+35,
	x128+39,
	x128+43,
	x128+47,
	ytmp128+3,
	ytmp128+7,
	ytmp128+11,
	ytmp128+15,
	ytmp128+19,
	ytmp128+23,
	ytmp128+27,
	ytmp128+31,
	ytmp128+35,
	ytmp128+39,
	ytmp128+43,
	ytmp128+47);



  bfly4_tw1(ytmp128, 
	    ytmp128+1,
	    ytmp128+2,
	    ytmp128+3,
	    y128, 
	    y128+12,
	    y128+24,
	    y128+36);


  
  for (i=4,j=1,k=0;i<48;i+=4,j++,k++) {
    
    bfly4(ytmp128+i, 
	  ytmp128+i+1,
	  ytmp128+i+2,
	  ytmp128+i+3,
	  y128+j, 
	  y128+j+12,
	  y128+j+24,
	  y128+j+36,
	  twa128+k,
	  twb128+k,
	  twc128+k);

  }

  if (scale_flag == 1) {
    norm128 = _mm_set1_epi16(dft_norm_table[3]);
    
    for (i=0;i<48;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

}

static int16_t twa60[88]__attribute__((aligned(16))) = {32587,-3425,32587,-3425,32587,-3425,32587,-3425,
32050,-6812,32050,-6812,32050,-6812,32050,-6812,
31163,-10125,31163,-10125,31163,-10125,31163,-10125,
29934,-13327,29934,-13327,29934,-13327,29934,-13327,
28377,-16383,28377,-16383,28377,-16383,28377,-16383,
26509,-19259,26509,-19259,26509,-19259,26509,-19259,
24350,-21925,24350,-21925,24350,-21925,24350,-21925,
21925,-24350,21925,-24350,21925,-24350,21925,-24350,
19259,-26509,19259,-26509,19259,-26509,19259,-26509,
16383,-28377,16383,-28377,16383,-28377,16383,-28377,
13327,-29934,13327,-29934,13327,-29934,13327,-29934};
static int16_t twb60[88]__attribute__((aligned(16))) = {32050,-6812,32050,-6812,32050,-6812,32050,-6812,
29934,-13327,29934,-13327,29934,-13327,29934,-13327,
26509,-19259,26509,-19259,26509,-19259,26509,-19259,
21925,-24350,21925,-24350,21925,-24350,21925,-24350,
16383,-28377,16383,-28377,16383,-28377,16383,-28377,
10125,-31163,10125,-31163,10125,-31163,10125,-31163,
3425,-32587,3425,-32587,3425,-32587,3425,-32587,
-3425,-32587,-3425,-32587,-3425,-32587,-3425,-32587,
-10125,-31163,-10125,-31163,-10125,-31163,-10125,-31163,
-16383,-28377,-16383,-28377,-16383,-28377,-16383,-28377,
-21925,-24350,-21925,-24350,-21925,-24350,-21925,-24350};
static int16_t twc60[88]__attribute__((aligned(16))) = {31163,-10125,31163,-10125,31163,-10125,31163,-10125,
26509,-19259,26509,-19259,26509,-19259,26509,-19259,
19259,-26509,19259,-26509,19259,-26509,19259,-26509,
10125,-31163,10125,-31163,10125,-31163,10125,-31163,
0,-32767,0,-32767,0,-32767,0,-32767,
-10125,-31163,-10125,-31163,-10125,-31163,-10125,-31163,
-19259,-26509,-19259,-26509,-19259,-26509,-19259,-26509,
-26509,-19259,-26509,-19259,-26509,-19259,-26509,-19259,
-31163,-10125,-31163,-10125,-31163,-10125,-31163,-10125,
-32767,0,-32767,0,-32767,0,-32767,0,
-31163,10125,-31163,10125,-31163,10125,-31163,10125};
static int16_t twd60[88]__attribute__((aligned(16))) = {29934,-13327,29934,-13327,29934,-13327,29934,-13327,
21925,-24350,21925,-24350,21925,-24350,21925,-24350,
10125,-31163,10125,-31163,10125,-31163,10125,-31163,
-3425,-32587,-3425,-32587,-3425,-32587,-3425,-32587,
-16383,-28377,-16383,-28377,-16383,-28377,-16383,-28377,
-26509,-19259,-26509,-19259,-26509,-19259,-26509,-19259,
-32050,-6812,-32050,-6812,-32050,-6812,-32050,-6812,
-32050,6812,-32050,6812,-32050,6812,-32050,6812,
-26509,19259,-26509,19259,-26509,19259,-26509,19259,
-16383,28377,-16383,28377,-16383,28377,-16383,28377,
-3425,32587,-3425,32587,-3425,32587,-3425,32587};

void dft60(int16_t *x,int16_t *y,unsigned char scale) {
  
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa60[0];
  __m128i *twb128=(__m128i *)&twb60[0];
  __m128i *twc128=(__m128i *)&twc60[0];
  __m128i *twd128=(__m128i *)&twd60[0];
  __m128i ytmp128[60];//=&ytmp128array[0];
  int i,j,k;

  dft12f(x128,
	x128+5,
	x128+10,
	x128+15,
	x128+20,
	x128+25,
	x128+30,
	x128+35,
	x128+40,
	x128+45,
	x128+50,
	x128+55,
	ytmp128,
	ytmp128+5,
	ytmp128+10,
	ytmp128+15,
	ytmp128+20,
	ytmp128+25,
	ytmp128+30,
	ytmp128+35,
	ytmp128+40,
	ytmp128+45,
	ytmp128+50,
	ytmp128+55);

  dft12f(x128+1,
	x128+6,
	x128+11,
	x128+16,
	x128+21,
	x128+26,
	x128+31,
	x128+36,
	x128+41,
	x128+46,
	x128+51,
	x128+56,
	ytmp128+1,
	ytmp128+6,
	ytmp128+11,
	ytmp128+16,
	ytmp128+21,
	ytmp128+26,
	ytmp128+31,
	ytmp128+36,
	ytmp128+41,
	ytmp128+46,
	ytmp128+51,
	ytmp128+56);

  dft12f(x128+2,
	x128+7,
	x128+12,
	x128+17,
	x128+22,
	x128+27,
	x128+32,
	x128+37,
	x128+42,
	x128+47,
	x128+52,
	x128+57,
	ytmp128+2,
	ytmp128+7,
	ytmp128+12,
	ytmp128+17,
	ytmp128+22,
	ytmp128+27,
	ytmp128+32,
	ytmp128+37,
	ytmp128+42,
	ytmp128+47,
	ytmp128+52,
	ytmp128+57);

  dft12f(x128+3,
	x128+8,
	x128+13,
	x128+18,
	x128+23,
	x128+28,
	x128+33,
	x128+38,
	x128+43,
	x128+48,
	x128+53,
	x128+58,
	ytmp128+3,
	ytmp128+8,
	ytmp128+13,
	ytmp128+18,
	ytmp128+23,
	ytmp128+28,
	ytmp128+33,
	ytmp128+38,
	ytmp128+43,
	ytmp128+48,
	ytmp128+53,
	ytmp128+58);

  dft12f(x128+4,
	x128+9,
	x128+14,
	x128+19,
	x128+24,
	x128+29,
	x128+34,
	x128+39,
	x128+44,
	x128+49,
	x128+54,
	x128+59,
	ytmp128+4,
	ytmp128+9,
	ytmp128+14,
	ytmp128+19,
	ytmp128+24,
	ytmp128+29,
	ytmp128+34,
	ytmp128+39,
	ytmp128+44,
	ytmp128+49,
	ytmp128+54,
	ytmp128+59);

  bfly5_tw1(ytmp128, 
	    ytmp128+1,
	    ytmp128+2,
	    ytmp128+3,
	    ytmp128+4,
	    y128, 
	    y128+12,
	    y128+24,
	    y128+36,
	    y128+48);
  
  for (i=5,j=1,k=0;i<60;i+=5,j++,k++) {
    
    bfly5(ytmp128+i, 
	  ytmp128+i+1,
	  ytmp128+i+2,
	  ytmp128+i+3,
	  ytmp128+i+4,
	  y128+j, 
	  y128+j+12,
	  y128+j+24,
	  y128+j+36,
	  y128+j+48,
	  twa128+k,
	  twb128+k,
	  twc128+k,
	  twd128+k);
  }

  if (scale == 1) {
    norm128 = _mm_set1_epi16(dft_norm_table[4]);
    
    for (i=0;i<60;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

}

static int16_t tw72[280]__attribute__((aligned(16))) = {32642,-2855,32642,-2855,32642,-2855,32642,-2855,
32269,-5689,32269,-5689,32269,-5689,32269,-5689,
31650,-8480,31650,-8480,31650,-8480,31650,-8480,
30790,-11206,30790,-11206,30790,-11206,30790,-11206,
29696,-13847,29696,-13847,29696,-13847,29696,-13847,
28377,-16383,28377,-16383,28377,-16383,28377,-16383,
26841,-18794,26841,-18794,26841,-18794,26841,-18794,
25100,-21062,25100,-21062,25100,-21062,25100,-21062,
23169,-23169,23169,-23169,23169,-23169,23169,-23169,
21062,-25100,21062,-25100,21062,-25100,21062,-25100,
18794,-26841,18794,-26841,18794,-26841,18794,-26841,
16383,-28377,16383,-28377,16383,-28377,16383,-28377,
13847,-29696,13847,-29696,13847,-29696,13847,-29696,
11206,-30790,11206,-30790,11206,-30790,11206,-30790,
8480,-31650,8480,-31650,8480,-31650,8480,-31650,
5689,-32269,5689,-32269,5689,-32269,5689,-32269,
2855,-32642,2855,-32642,2855,-32642,2855,-32642,
0,-32767,0,-32767,0,-32767,0,-32767,
-2855,-32642,-2855,-32642,-2855,-32642,-2855,-32642,
-5689,-32269,-5689,-32269,-5689,-32269,-5689,-32269,
-8480,-31650,-8480,-31650,-8480,-31650,-8480,-31650,
-11206,-30790,-11206,-30790,-11206,-30790,-11206,-30790,
-13847,-29696,-13847,-29696,-13847,-29696,-13847,-29696,
-16383,-28377,-16383,-28377,-16383,-28377,-16383,-28377,
-18794,-26841,-18794,-26841,-18794,-26841,-18794,-26841,
-21062,-25100,-21062,-25100,-21062,-25100,-21062,-25100,
-23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
-25100,-21062,-25100,-21062,-25100,-21062,-25100,-21062,
-26841,-18794,-26841,-18794,-26841,-18794,-26841,-18794,
-28377,-16383,-28377,-16383,-28377,-16383,-28377,-16383,
-29696,-13847,-29696,-13847,-29696,-13847,-29696,-13847,
-30790,-11206,-30790,-11206,-30790,-11206,-30790,-11206,
-31650,-8480,-31650,-8480,-31650,-8480,-31650,-8480,
-32269,-5689,-32269,-5689,-32269,-5689,-32269,-5689,
-32642,-2855,-32642,-2855,-32642,-2855,-32642,-2855,
};

void dft72(int16_t *x,int16_t *y,unsigned char scale_flag){

  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *tw128=(__m128i *)&tw72[0];
  __m128i x2128[72];// = (__m128i *)&x2128array[0];

  __m128i ytmp128[72];//=&ytmp128array2[0];

  for (i=0,j=0;i<36;i++,j+=2) {
    x2128[i]    = x128[j];    // even inputs
    x2128[i+36] = x128[j+1];  // odd inputs
  }

  dft36((int16_t *)x2128,(int16_t *)ytmp128,1);
  dft36((int16_t *)(x2128+36),(int16_t *)(ytmp128+36),1);

  bfly2_tw1(ytmp128,ytmp128+36,y128,y128+36);
  for (i=1,j=0;i<36;i++,j++) {
    bfly2(ytmp128+i,
	  ytmp128+36+i,
	  y128+i,
	  y128+36+i,
	  tw128+j);
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(dft_norm_table[5]);
    
    for (i=0;i<72;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

}

static int16_t tw96[376]__attribute__((aligned(16))) = {32696,-2143,32696,-2143,32696,-2143,32696,-2143,
32486,-4276,32486,-4276,32486,-4276,32486,-4276,
32137,-6392,32137,-6392,32137,-6392,32137,-6392,
31650,-8480,31650,-8480,31650,-8480,31650,-8480,
31028,-10532,31028,-10532,31028,-10532,31028,-10532,
30272,-12539,30272,-12539,30272,-12539,30272,-12539,
29387,-14492,29387,-14492,29387,-14492,29387,-14492,
28377,-16383,28377,-16383,28377,-16383,28377,-16383,
27244,-18204,27244,-18204,27244,-18204,27244,-18204,
25995,-19947,25995,-19947,25995,-19947,25995,-19947,
24635,-21604,24635,-21604,24635,-21604,24635,-21604,
23169,-23169,23169,-23169,23169,-23169,23169,-23169,
21604,-24635,21604,-24635,21604,-24635,21604,-24635,
19947,-25995,19947,-25995,19947,-25995,19947,-25995,
18204,-27244,18204,-27244,18204,-27244,18204,-27244,
16383,-28377,16383,-28377,16383,-28377,16383,-28377,
14492,-29387,14492,-29387,14492,-29387,14492,-29387,
12539,-30272,12539,-30272,12539,-30272,12539,-30272,
10532,-31028,10532,-31028,10532,-31028,10532,-31028,
8480,-31650,8480,-31650,8480,-31650,8480,-31650,
6392,-32137,6392,-32137,6392,-32137,6392,-32137,
4276,-32486,4276,-32486,4276,-32486,4276,-32486,
2143,-32696,2143,-32696,2143,-32696,2143,-32696,
0,-32767,0,-32767,0,-32767,0,-32767,
-2143,-32696,-2143,-32696,-2143,-32696,-2143,-32696,
-4276,-32486,-4276,-32486,-4276,-32486,-4276,-32486,
-6392,-32137,-6392,-32137,-6392,-32137,-6392,-32137,
-8480,-31650,-8480,-31650,-8480,-31650,-8480,-31650,
-10532,-31028,-10532,-31028,-10532,-31028,-10532,-31028,
-12539,-30272,-12539,-30272,-12539,-30272,-12539,-30272,
-14492,-29387,-14492,-29387,-14492,-29387,-14492,-29387,
-16383,-28377,-16383,-28377,-16383,-28377,-16383,-28377,
-18204,-27244,-18204,-27244,-18204,-27244,-18204,-27244,
-19947,-25995,-19947,-25995,-19947,-25995,-19947,-25995,
-21604,-24635,-21604,-24635,-21604,-24635,-21604,-24635,
-23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
-24635,-21604,-24635,-21604,-24635,-21604,-24635,-21604,
-25995,-19947,-25995,-19947,-25995,-19947,-25995,-19947,
-27244,-18204,-27244,-18204,-27244,-18204,-27244,-18204,
-28377,-16383,-28377,-16383,-28377,-16383,-28377,-16383,
-29387,-14492,-29387,-14492,-29387,-14492,-29387,-14492,
-30272,-12539,-30272,-12539,-30272,-12539,-30272,-12539,
-31028,-10532,-31028,-10532,-31028,-10532,-31028,-10532,
-31650,-8480,-31650,-8480,-31650,-8480,-31650,-8480,
-32137,-6392,-32137,-6392,-32137,-6392,-32137,-6392,
-32486,-4276,-32486,-4276,-32486,-4276,-32486,-4276,
-32696,-2143,-32696,-2143,-32696,-2143,-32696,-2143};

void dft96(int16_t *x,int16_t *y,unsigned char scale_flag){


  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *tw128=(__m128i *)&tw96[0];
  __m128i x2128[96];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[96];//=&ytmp128array2[0];


  for (i=0,j=0;i<48;i++,j+=2) {
    x2128[i]    = x128[j];
    x2128[i+48] = x128[j+1];
  }

  dft48((int16_t *)x2128,(int16_t *)ytmp128,0);
  dft48((int16_t *)(x2128+48),(int16_t *)(ytmp128+48),0);


  bfly2_tw1(ytmp128,ytmp128+48,y128,y128+48);
  for (i=1,j=0;i<48;i++,j++) {
    bfly2(ytmp128+i,
	  ytmp128+48+i,
	  y128+i,
	  y128+48+i,
	  tw128+j);
  }
  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(dft_norm_table[6]);
    
    for (i=0;i<96;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

}

static int16_t twa108[280]__attribute__((aligned(16))) = {32711,-1905,32711,-1905,32711,-1905,32711,-1905,
32545,-3804,32545,-3804,32545,-3804,32545,-3804,
32269,-5689,32269,-5689,32269,-5689,32269,-5689,
31883,-7556,31883,-7556,31883,-7556,31883,-7556,
31390,-9397,31390,-9397,31390,-9397,31390,-9397,
30790,-11206,30790,-11206,30790,-11206,30790,-11206,
30087,-12978,30087,-12978,30087,-12978,30087,-12978,
29281,-14705,29281,-14705,29281,-14705,29281,-14705,
28377,-16383,28377,-16383,28377,-16383,28377,-16383,
27376,-18005,27376,-18005,27376,-18005,27376,-18005,
26283,-19567,26283,-19567,26283,-19567,26283,-19567,
25100,-21062,25100,-21062,25100,-21062,25100,-21062,
23833,-22486,23833,-22486,23833,-22486,23833,-22486,
22486,-23833,22486,-23833,22486,-23833,22486,-23833,
21062,-25100,21062,-25100,21062,-25100,21062,-25100,
19567,-26283,19567,-26283,19567,-26283,19567,-26283,
18005,-27376,18005,-27376,18005,-27376,18005,-27376,
16383,-28377,16383,-28377,16383,-28377,16383,-28377,
14705,-29281,14705,-29281,14705,-29281,14705,-29281,
12978,-30087,12978,-30087,12978,-30087,12978,-30087,
11206,-30790,11206,-30790,11206,-30790,11206,-30790,
9397,-31390,9397,-31390,9397,-31390,9397,-31390,
7556,-31883,7556,-31883,7556,-31883,7556,-31883,
5689,-32269,5689,-32269,5689,-32269,5689,-32269,
3804,-32545,3804,-32545,3804,-32545,3804,-32545,
1905,-32711,1905,-32711,1905,-32711,1905,-32711,
0,-32767,0,-32767,0,-32767,0,-32767,
-1905,-32711,-1905,-32711,-1905,-32711,-1905,-32711,
-3804,-32545,-3804,-32545,-3804,-32545,-3804,-32545,
-5689,-32269,-5689,-32269,-5689,-32269,-5689,-32269,
-7556,-31883,-7556,-31883,-7556,-31883,-7556,-31883,
-9397,-31390,-9397,-31390,-9397,-31390,-9397,-31390,
-11206,-30790,-11206,-30790,-11206,-30790,-11206,-30790,
-12978,-30087,-12978,-30087,-12978,-30087,-12978,-30087,
-14705,-29281,-14705,-29281,-14705,-29281,-14705,-29281};

static int16_t twb108[280]__attribute__((aligned(16))) = {32545,-3804,32545,-3804,32545,-3804,32545,-3804,
31883,-7556,31883,-7556,31883,-7556,31883,-7556,
30790,-11206,30790,-11206,30790,-11206,30790,-11206,
29281,-14705,29281,-14705,29281,-14705,29281,-14705,
27376,-18005,27376,-18005,27376,-18005,27376,-18005,
25100,-21062,25100,-21062,25100,-21062,25100,-21062,
22486,-23833,22486,-23833,22486,-23833,22486,-23833,
19567,-26283,19567,-26283,19567,-26283,19567,-26283,
16383,-28377,16383,-28377,16383,-28377,16383,-28377,
12978,-30087,12978,-30087,12978,-30087,12978,-30087,
9397,-31390,9397,-31390,9397,-31390,9397,-31390,
5689,-32269,5689,-32269,5689,-32269,5689,-32269,
1905,-32711,1905,-32711,1905,-32711,1905,-32711,
-1905,-32711,-1905,-32711,-1905,-32711,-1905,-32711,
-5689,-32269,-5689,-32269,-5689,-32269,-5689,-32269,
-9397,-31390,-9397,-31390,-9397,-31390,-9397,-31390,
-12978,-30087,-12978,-30087,-12978,-30087,-12978,-30087,
-16383,-28377,-16383,-28377,-16383,-28377,-16383,-28377,
-19567,-26283,-19567,-26283,-19567,-26283,-19567,-26283,
-22486,-23833,-22486,-23833,-22486,-23833,-22486,-23833,
-25100,-21062,-25100,-21062,-25100,-21062,-25100,-21062,
-27376,-18005,-27376,-18005,-27376,-18005,-27376,-18005,
-29281,-14705,-29281,-14705,-29281,-14705,-29281,-14705,
-30790,-11206,-30790,-11206,-30790,-11206,-30790,-11206,
-31883,-7556,-31883,-7556,-31883,-7556,-31883,-7556,
-32545,-3804,-32545,-3804,-32545,-3804,-32545,-3804,
-32767,0,-32767,0,-32767,0,-32767,0,
-32545,3804,-32545,3804,-32545,3804,-32545,3804,
-31883,7556,-31883,7556,-31883,7556,-31883,7556,
-30790,11206,-30790,11206,-30790,11206,-30790,11206,
-29281,14705,-29281,14705,-29281,14705,-29281,14705,
-27376,18005,-27376,18005,-27376,18005,-27376,18005,
-25100,21062,-25100,21062,-25100,21062,-25100,21062,
-22486,23833,-22486,23833,-22486,23833,-22486,23833,
-19567,26283,-19567,26283,-19567,26283,-19567,26283};

void dft108(int16_t *x,int16_t *y,unsigned char scale_flag){


  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa108[0];
  __m128i *twb128=(__m128i *)&twb108[0];
  __m128i x2128[108];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[108];//=&ytmp128array2[0];


  for (i=0,j=0;i<36;i++,j+=3) {
    x2128[i]    = x128[j];
    x2128[i+36] = x128[j+1];
    x2128[i+72] = x128[j+2];
  }

  dft36((int16_t *)x2128,(int16_t *)ytmp128,0);
  dft36((int16_t *)(x2128+36),(int16_t *)(ytmp128+36),0);
  dft36((int16_t *)(x2128+72),(int16_t *)(ytmp128+72),0);

  bfly3_tw1(ytmp128,ytmp128+36,ytmp128+72,y128,y128+36,y128+72);
  for (i=1,j=0;i<36;i++,j++) {
    bfly3(ytmp128+i,
	  ytmp128+36+i,
	  ytmp128+72+i,
	  y128+i,
	  y128+36+i,
	  y128+72+i,
	  twa128+j,
	  twb128+j);

  }
  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(dft_norm_table[7]);
    
    for (i=0;i<108;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

}

static int16_t tw120[472]__attribute__((aligned(16))) = {32722,-1714,32722,-1714,32722,-1714,32722,-1714,
32587,-3425,32587,-3425,32587,-3425,32587,-3425,
32363,-5125,32363,-5125,32363,-5125,32363,-5125,
32050,-6812,32050,-6812,32050,-6812,32050,-6812,
31650,-8480,31650,-8480,31650,-8480,31650,-8480,
31163,-10125,31163,-10125,31163,-10125,31163,-10125,
30590,-11742,30590,-11742,30590,-11742,30590,-11742,
29934,-13327,29934,-13327,29934,-13327,29934,-13327,
29195,-14875,29195,-14875,29195,-14875,29195,-14875,
28377,-16383,28377,-16383,28377,-16383,28377,-16383,
27480,-17846,27480,-17846,27480,-17846,27480,-17846,
26509,-19259,26509,-19259,26509,-19259,26509,-19259,
25464,-20620,25464,-20620,25464,-20620,25464,-20620,
24350,-21925,24350,-21925,24350,-21925,24350,-21925,
23169,-23169,23169,-23169,23169,-23169,23169,-23169,
21925,-24350,21925,-24350,21925,-24350,21925,-24350,
20620,-25464,20620,-25464,20620,-25464,20620,-25464,
19259,-26509,19259,-26509,19259,-26509,19259,-26509,
17846,-27480,17846,-27480,17846,-27480,17846,-27480,
16383,-28377,16383,-28377,16383,-28377,16383,-28377,
14875,-29195,14875,-29195,14875,-29195,14875,-29195,
13327,-29934,13327,-29934,13327,-29934,13327,-29934,
11742,-30590,11742,-30590,11742,-30590,11742,-30590,
10125,-31163,10125,-31163,10125,-31163,10125,-31163,
8480,-31650,8480,-31650,8480,-31650,8480,-31650,
6812,-32050,6812,-32050,6812,-32050,6812,-32050,
5125,-32363,5125,-32363,5125,-32363,5125,-32363,
3425,-32587,3425,-32587,3425,-32587,3425,-32587,
1714,-32722,1714,-32722,1714,-32722,1714,-32722,
0,-32767,0,-32767,0,-32767,0,-32767,
-1714,-32722,-1714,-32722,-1714,-32722,-1714,-32722,
-3425,-32587,-3425,-32587,-3425,-32587,-3425,-32587,
-5125,-32363,-5125,-32363,-5125,-32363,-5125,-32363,
-6812,-32050,-6812,-32050,-6812,-32050,-6812,-32050,
-8480,-31650,-8480,-31650,-8480,-31650,-8480,-31650,
-10125,-31163,-10125,-31163,-10125,-31163,-10125,-31163,
-11742,-30590,-11742,-30590,-11742,-30590,-11742,-30590,
-13327,-29934,-13327,-29934,-13327,-29934,-13327,-29934,
-14875,-29195,-14875,-29195,-14875,-29195,-14875,-29195,
-16383,-28377,-16383,-28377,-16383,-28377,-16383,-28377,
-17846,-27480,-17846,-27480,-17846,-27480,-17846,-27480,
-19259,-26509,-19259,-26509,-19259,-26509,-19259,-26509,
-20620,-25464,-20620,-25464,-20620,-25464,-20620,-25464,
-21925,-24350,-21925,-24350,-21925,-24350,-21925,-24350,
-23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
-24350,-21925,-24350,-21925,-24350,-21925,-24350,-21925,
-25464,-20620,-25464,-20620,-25464,-20620,-25464,-20620,
-26509,-19259,-26509,-19259,-26509,-19259,-26509,-19259,
-27480,-17846,-27480,-17846,-27480,-17846,-27480,-17846,
-28377,-16383,-28377,-16383,-28377,-16383,-28377,-16383,
-29195,-14875,-29195,-14875,-29195,-14875,-29195,-14875,
-29934,-13327,-29934,-13327,-29934,-13327,-29934,-13327,
-30590,-11742,-30590,-11742,-30590,-11742,-30590,-11742,
-31163,-10125,-31163,-10125,-31163,-10125,-31163,-10125,
-31650,-8480,-31650,-8480,-31650,-8480,-31650,-8480,
-32050,-6812,-32050,-6812,-32050,-6812,-32050,-6812,
-32363,-5125,-32363,-5125,-32363,-5125,-32363,-5125,
-32587,-3425,-32587,-3425,-32587,-3425,-32587,-3425,
-32722,-1714,-32722,-1714,-32722,-1714,-32722,-1714};

void dft120(int16_t *x,int16_t *y, unsigned char scale_flag){


  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *tw128=(__m128i *)&tw120[0];
  __m128i x2128[120];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[120];//=&ytmp128array2[0];


  for (i=0,j=0;i<60;i++,j+=2) {
    x2128[i]    = x128[j];
    x2128[i+60] = x128[j+1];
  }

  dft60((int16_t *)x2128,(int16_t *)ytmp128,0);
  dft60((int16_t *)(x2128+60),(int16_t *)(ytmp128+60),0);


  bfly2_tw1(ytmp128,ytmp128+60,y128,y128+60);
  for (i=1,j=0;i<60;i++,j++) {
    bfly2(ytmp128+i,
	  ytmp128+60+i,
	  y128+i,
	  y128+60+i,
	  tw128+j);
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(dft_norm_table[8]);
    
    for (i=0;i<120;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

}

static int16_t twa144[376]__attribute__((aligned(16))) = {32735,-1429,32735,-1429,32735,-1429,32735,-1429,
32642,-2855,32642,-2855,32642,-2855,32642,-2855,
32486,-4276,32486,-4276,32486,-4276,32486,-4276,
32269,-5689,32269,-5689,32269,-5689,32269,-5689,
31990,-7092,31990,-7092,31990,-7092,31990,-7092,
31650,-8480,31650,-8480,31650,-8480,31650,-8480,
31250,-9853,31250,-9853,31250,-9853,31250,-9853,
30790,-11206,30790,-11206,30790,-11206,30790,-11206,
30272,-12539,30272,-12539,30272,-12539,30272,-12539,
29696,-13847,29696,-13847,29696,-13847,29696,-13847,
29064,-15130,29064,-15130,29064,-15130,29064,-15130,
28377,-16383,28377,-16383,28377,-16383,28377,-16383,
27635,-17605,27635,-17605,27635,-17605,27635,-17605,
26841,-18794,26841,-18794,26841,-18794,26841,-18794,
25995,-19947,25995,-19947,25995,-19947,25995,-19947,
25100,-21062,25100,-21062,25100,-21062,25100,-21062,
24158,-22137,24158,-22137,24158,-22137,24158,-22137,
23169,-23169,23169,-23169,23169,-23169,23169,-23169,
22137,-24158,22137,-24158,22137,-24158,22137,-24158,
21062,-25100,21062,-25100,21062,-25100,21062,-25100,
19947,-25995,19947,-25995,19947,-25995,19947,-25995,
18794,-26841,18794,-26841,18794,-26841,18794,-26841,
17605,-27635,17605,-27635,17605,-27635,17605,-27635,
16383,-28377,16383,-28377,16383,-28377,16383,-28377,
15130,-29064,15130,-29064,15130,-29064,15130,-29064,
13847,-29696,13847,-29696,13847,-29696,13847,-29696,
12539,-30272,12539,-30272,12539,-30272,12539,-30272,
11206,-30790,11206,-30790,11206,-30790,11206,-30790,
9853,-31250,9853,-31250,9853,-31250,9853,-31250,
8480,-31650,8480,-31650,8480,-31650,8480,-31650,
7092,-31990,7092,-31990,7092,-31990,7092,-31990,
5689,-32269,5689,-32269,5689,-32269,5689,-32269,
4276,-32486,4276,-32486,4276,-32486,4276,-32486,
2855,-32642,2855,-32642,2855,-32642,2855,-32642,
1429,-32735,1429,-32735,1429,-32735,1429,-32735,
0,-32767,0,-32767,0,-32767,0,-32767,
-1429,-32735,-1429,-32735,-1429,-32735,-1429,-32735,
-2855,-32642,-2855,-32642,-2855,-32642,-2855,-32642,
-4276,-32486,-4276,-32486,-4276,-32486,-4276,-32486,
-5689,-32269,-5689,-32269,-5689,-32269,-5689,-32269,
-7092,-31990,-7092,-31990,-7092,-31990,-7092,-31990,
-8480,-31650,-8480,-31650,-8480,-31650,-8480,-31650,
-9853,-31250,-9853,-31250,-9853,-31250,-9853,-31250,
-11206,-30790,-11206,-30790,-11206,-30790,-11206,-30790,
-12539,-30272,-12539,-30272,-12539,-30272,-12539,-30272,
-13847,-29696,-13847,-29696,-13847,-29696,-13847,-29696,
-15130,-29064,-15130,-29064,-15130,-29064,-15130,-29064};

static int16_t twb144[376]__attribute__((aligned(16))) = {32642,-2855,32642,-2855,32642,-2855,32642,-2855,
32269,-5689,32269,-5689,32269,-5689,32269,-5689,
31650,-8480,31650,-8480,31650,-8480,31650,-8480,
30790,-11206,30790,-11206,30790,-11206,30790,-11206,
29696,-13847,29696,-13847,29696,-13847,29696,-13847,
28377,-16383,28377,-16383,28377,-16383,28377,-16383,
26841,-18794,26841,-18794,26841,-18794,26841,-18794,
25100,-21062,25100,-21062,25100,-21062,25100,-21062,
23169,-23169,23169,-23169,23169,-23169,23169,-23169,
21062,-25100,21062,-25100,21062,-25100,21062,-25100,
18794,-26841,18794,-26841,18794,-26841,18794,-26841,
16383,-28377,16383,-28377,16383,-28377,16383,-28377,
13847,-29696,13847,-29696,13847,-29696,13847,-29696,
11206,-30790,11206,-30790,11206,-30790,11206,-30790,
8480,-31650,8480,-31650,8480,-31650,8480,-31650,
5689,-32269,5689,-32269,5689,-32269,5689,-32269,
2855,-32642,2855,-32642,2855,-32642,2855,-32642,
0,-32767,0,-32767,0,-32767,0,-32767,
-2855,-32642,-2855,-32642,-2855,-32642,-2855,-32642,
-5689,-32269,-5689,-32269,-5689,-32269,-5689,-32269,
-8480,-31650,-8480,-31650,-8480,-31650,-8480,-31650,
-11206,-30790,-11206,-30790,-11206,-30790,-11206,-30790,
-13847,-29696,-13847,-29696,-13847,-29696,-13847,-29696,
-16383,-28377,-16383,-28377,-16383,-28377,-16383,-28377,
-18794,-26841,-18794,-26841,-18794,-26841,-18794,-26841,
-21062,-25100,-21062,-25100,-21062,-25100,-21062,-25100,
-23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
-25100,-21062,-25100,-21062,-25100,-21062,-25100,-21062,
-26841,-18794,-26841,-18794,-26841,-18794,-26841,-18794,
-28377,-16383,-28377,-16383,-28377,-16383,-28377,-16383,
-29696,-13847,-29696,-13847,-29696,-13847,-29696,-13847,
-30790,-11206,-30790,-11206,-30790,-11206,-30790,-11206,
-31650,-8480,-31650,-8480,-31650,-8480,-31650,-8480,
-32269,-5689,-32269,-5689,-32269,-5689,-32269,-5689,
-32642,-2855,-32642,-2855,-32642,-2855,-32642,-2855,
-32767,0,-32767,0,-32767,0,-32767,0,
-32642,2855,-32642,2855,-32642,2855,-32642,2855,
-32269,5689,-32269,5689,-32269,5689,-32269,5689,
-31650,8480,-31650,8480,-31650,8480,-31650,8480,
-30790,11206,-30790,11206,-30790,11206,-30790,11206,
-29696,13847,-29696,13847,-29696,13847,-29696,13847,
-28377,16383,-28377,16383,-28377,16383,-28377,16383,
-26841,18794,-26841,18794,-26841,18794,-26841,18794,
-25100,21062,-25100,21062,-25100,21062,-25100,21062,
-23169,23169,-23169,23169,-23169,23169,-23169,23169,
-21062,25100,-21062,25100,-21062,25100,-21062,25100,
-18794,26841,-18794,26841,-18794,26841,-18794,26841};

void dft144(int16_t *x,int16_t *y,unsigned char scale_flag){

  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa144[0];
  __m128i *twb128=(__m128i *)&twb144[0];
  __m128i x2128[144];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[144];//=&ytmp128array2[0];



  for (i=0,j=0;i<48;i++,j+=3) {
    x2128[i]    = x128[j];
    x2128[i+48] = x128[j+1];
    x2128[i+96] = x128[j+2];
  }

  dft48((int16_t *)x2128,(int16_t *)ytmp128,1);
  dft48((int16_t *)(x2128+48),(int16_t *)(ytmp128+48),1);
  dft48((int16_t *)(x2128+96),(int16_t *)(ytmp128+96),1);

  bfly3_tw1(ytmp128,ytmp128+48,ytmp128+96,y128,y128+48,y128+96);
  for (i=1,j=0;i<48;i++,j++) {
    bfly3(ytmp128+i,
	  ytmp128+48+i,
	  ytmp128+96+i,
	  y128+i,
	  y128+48+i,
	  y128+96+i,
	  twa128+j,
	  twb128+j);
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(dft_norm_table[9]);
    
    for (i=0;i<144;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

}

static int16_t twa180[472]__attribute__((aligned(16))) = {32747,-1143,32747,-1143,32747,-1143,32747,-1143,
32687,-2285,32687,-2285,32687,-2285,32687,-2285,
32587,-3425,32587,-3425,32587,-3425,32587,-3425,
32448,-4560,32448,-4560,32448,-4560,32448,-4560,
32269,-5689,32269,-5689,32269,-5689,32269,-5689,
32050,-6812,32050,-6812,32050,-6812,32050,-6812,
31793,-7927,31793,-7927,31793,-7927,31793,-7927,
31497,-9031,31497,-9031,31497,-9031,31497,-9031,
31163,-10125,31163,-10125,31163,-10125,31163,-10125,
30790,-11206,30790,-11206,30790,-11206,30790,-11206,
30381,-12274,30381,-12274,30381,-12274,30381,-12274,
29934,-13327,29934,-13327,29934,-13327,29934,-13327,
29450,-14364,29450,-14364,29450,-14364,29450,-14364,
28931,-15383,28931,-15383,28931,-15383,28931,-15383,
28377,-16383,28377,-16383,28377,-16383,28377,-16383,
27787,-17363,27787,-17363,27787,-17363,27787,-17363,
27165,-18323,27165,-18323,27165,-18323,27165,-18323,
26509,-19259,26509,-19259,26509,-19259,26509,-19259,
25820,-20173,25820,-20173,25820,-20173,25820,-20173,
25100,-21062,25100,-21062,25100,-21062,25100,-21062,
24350,-21925,24350,-21925,24350,-21925,24350,-21925,
23570,-22761,23570,-22761,23570,-22761,23570,-22761,
22761,-23570,22761,-23570,22761,-23570,22761,-23570,
21925,-24350,21925,-24350,21925,-24350,21925,-24350,
21062,-25100,21062,-25100,21062,-25100,21062,-25100,
20173,-25820,20173,-25820,20173,-25820,20173,-25820,
19259,-26509,19259,-26509,19259,-26509,19259,-26509,
18323,-27165,18323,-27165,18323,-27165,18323,-27165,
17363,-27787,17363,-27787,17363,-27787,17363,-27787,
16383,-28377,16383,-28377,16383,-28377,16383,-28377,
15383,-28931,15383,-28931,15383,-28931,15383,-28931,
14364,-29450,14364,-29450,14364,-29450,14364,-29450,
13327,-29934,13327,-29934,13327,-29934,13327,-29934,
12274,-30381,12274,-30381,12274,-30381,12274,-30381,
11206,-30790,11206,-30790,11206,-30790,11206,-30790,
10125,-31163,10125,-31163,10125,-31163,10125,-31163,
9031,-31497,9031,-31497,9031,-31497,9031,-31497,
7927,-31793,7927,-31793,7927,-31793,7927,-31793,
6812,-32050,6812,-32050,6812,-32050,6812,-32050,
5689,-32269,5689,-32269,5689,-32269,5689,-32269,
4560,-32448,4560,-32448,4560,-32448,4560,-32448,
3425,-32587,3425,-32587,3425,-32587,3425,-32587,
2285,-32687,2285,-32687,2285,-32687,2285,-32687,
1143,-32747,1143,-32747,1143,-32747,1143,-32747,
0,-32767,0,-32767,0,-32767,0,-32767,
-1143,-32747,-1143,-32747,-1143,-32747,-1143,-32747,
-2285,-32687,-2285,-32687,-2285,-32687,-2285,-32687,
-3425,-32587,-3425,-32587,-3425,-32587,-3425,-32587,
-4560,-32448,-4560,-32448,-4560,-32448,-4560,-32448,
-5689,-32269,-5689,-32269,-5689,-32269,-5689,-32269,
-6812,-32050,-6812,-32050,-6812,-32050,-6812,-32050,
-7927,-31793,-7927,-31793,-7927,-31793,-7927,-31793,
-9031,-31497,-9031,-31497,-9031,-31497,-9031,-31497,
-10125,-31163,-10125,-31163,-10125,-31163,-10125,-31163,
-11206,-30790,-11206,-30790,-11206,-30790,-11206,-30790,
-12274,-30381,-12274,-30381,-12274,-30381,-12274,-30381,
-13327,-29934,-13327,-29934,-13327,-29934,-13327,-29934,
-14364,-29450,-14364,-29450,-14364,-29450,-14364,-29450,
-15383,-28931,-15383,-28931,-15383,-28931,-15383,-28931};

static int16_t twb180[472]__attribute__((aligned(16))) = {32687,-2285,32687,-2285,32687,-2285,32687,-2285,
32448,-4560,32448,-4560,32448,-4560,32448,-4560,
32050,-6812,32050,-6812,32050,-6812,32050,-6812,
31497,-9031,31497,-9031,31497,-9031,31497,-9031,
30790,-11206,30790,-11206,30790,-11206,30790,-11206,
29934,-13327,29934,-13327,29934,-13327,29934,-13327,
28931,-15383,28931,-15383,28931,-15383,28931,-15383,
27787,-17363,27787,-17363,27787,-17363,27787,-17363,
26509,-19259,26509,-19259,26509,-19259,26509,-19259,
25100,-21062,25100,-21062,25100,-21062,25100,-21062,
23570,-22761,23570,-22761,23570,-22761,23570,-22761,
21925,-24350,21925,-24350,21925,-24350,21925,-24350,
20173,-25820,20173,-25820,20173,-25820,20173,-25820,
18323,-27165,18323,-27165,18323,-27165,18323,-27165,
16383,-28377,16383,-28377,16383,-28377,16383,-28377,
14364,-29450,14364,-29450,14364,-29450,14364,-29450,
12274,-30381,12274,-30381,12274,-30381,12274,-30381,
10125,-31163,10125,-31163,10125,-31163,10125,-31163,
7927,-31793,7927,-31793,7927,-31793,7927,-31793,
5689,-32269,5689,-32269,5689,-32269,5689,-32269,
3425,-32587,3425,-32587,3425,-32587,3425,-32587,
1143,-32747,1143,-32747,1143,-32747,1143,-32747,
-1143,-32747,-1143,-32747,-1143,-32747,-1143,-32747,
-3425,-32587,-3425,-32587,-3425,-32587,-3425,-32587,
-5689,-32269,-5689,-32269,-5689,-32269,-5689,-32269,
-7927,-31793,-7927,-31793,-7927,-31793,-7927,-31793,
-10125,-31163,-10125,-31163,-10125,-31163,-10125,-31163,
-12274,-30381,-12274,-30381,-12274,-30381,-12274,-30381,
-14364,-29450,-14364,-29450,-14364,-29450,-14364,-29450,
-16383,-28377,-16383,-28377,-16383,-28377,-16383,-28377,
-18323,-27165,-18323,-27165,-18323,-27165,-18323,-27165,
-20173,-25820,-20173,-25820,-20173,-25820,-20173,-25820,
-21925,-24350,-21925,-24350,-21925,-24350,-21925,-24350,
-23570,-22761,-23570,-22761,-23570,-22761,-23570,-22761,
-25100,-21062,-25100,-21062,-25100,-21062,-25100,-21062,
-26509,-19259,-26509,-19259,-26509,-19259,-26509,-19259,
-27787,-17363,-27787,-17363,-27787,-17363,-27787,-17363,
-28931,-15383,-28931,-15383,-28931,-15383,-28931,-15383,
-29934,-13327,-29934,-13327,-29934,-13327,-29934,-13327,
-30790,-11206,-30790,-11206,-30790,-11206,-30790,-11206,
-31497,-9031,-31497,-9031,-31497,-9031,-31497,-9031,
-32050,-6812,-32050,-6812,-32050,-6812,-32050,-6812,
-32448,-4560,-32448,-4560,-32448,-4560,-32448,-4560,
-32687,-2285,-32687,-2285,-32687,-2285,-32687,-2285,
-32767,0,-32767,0,-32767,0,-32767,0,
-32687,2285,-32687,2285,-32687,2285,-32687,2285,
-32448,4560,-32448,4560,-32448,4560,-32448,4560,
-32050,6812,-32050,6812,-32050,6812,-32050,6812,
-31497,9031,-31497,9031,-31497,9031,-31497,9031,
-30790,11206,-30790,11206,-30790,11206,-30790,11206,
-29934,13327,-29934,13327,-29934,13327,-29934,13327,
-28931,15383,-28931,15383,-28931,15383,-28931,15383,
-27787,17363,-27787,17363,-27787,17363,-27787,17363,
-26509,19259,-26509,19259,-26509,19259,-26509,19259,
-25100,21062,-25100,21062,-25100,21062,-25100,21062,
-23570,22761,-23570,22761,-23570,22761,-23570,22761,
-21925,24350,-21925,24350,-21925,24350,-21925,24350,
-20173,25820,-20173,25820,-20173,25820,-20173,25820,
-18323,27165,-18323,27165,-18323,27165,-18323,27165};

void dft180(int16_t *x,int16_t *y,unsigned char scale_flag){

  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa180[0];
  __m128i *twb128=(__m128i *)&twb180[0];
  __m128i x2128[180];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[180];//=&ytmp128array2[0];



  for (i=0,j=0;i<60;i++,j+=3) {
    x2128[i]    = x128[j];
    x2128[i+60] = x128[j+1];
    x2128[i+120] = x128[j+2];
  }

  dft60((int16_t *)x2128,(int16_t *)ytmp128,1);
  dft60((int16_t *)(x2128+60),(int16_t *)(ytmp128+60),1);
  dft60((int16_t *)(x2128+120),(int16_t *)(ytmp128+120),1);

  bfly3_tw1(ytmp128,ytmp128+60,ytmp128+120,y128,y128+60,y128+120);
  for (i=1,j=0;i<60;i++,j++) {
    bfly3(ytmp128+i,
	  ytmp128+60+i,
	  ytmp128+120+i,
	  y128+i,
	  y128+60+i,
	  y128+120+i,
	  twa128+j,
	  twb128+j);
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(dft_norm_table[10]);
    
    for (i=0;i<180;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

}

static int16_t twa192[376]__attribute__((aligned(16))) = {32749,-1072,32749,-1072,32749,-1072,32749,-1072,
32696,-2143,32696,-2143,32696,-2143,32696,-2143,
32609,-3211,32609,-3211,32609,-3211,32609,-3211,
32486,-4276,32486,-4276,32486,-4276,32486,-4276,
32329,-5337,32329,-5337,32329,-5337,32329,-5337,
32137,-6392,32137,-6392,32137,-6392,32137,-6392,
31911,-7440,31911,-7440,31911,-7440,31911,-7440,
31650,-8480,31650,-8480,31650,-8480,31650,-8480,
31356,-9511,31356,-9511,31356,-9511,31356,-9511,
31028,-10532,31028,-10532,31028,-10532,31028,-10532,
30666,-11542,30666,-11542,30666,-11542,30666,-11542,
30272,-12539,30272,-12539,30272,-12539,30272,-12539,
29846,-13523,29846,-13523,29846,-13523,29846,-13523,
29387,-14492,29387,-14492,29387,-14492,29387,-14492,
28897,-15446,28897,-15446,28897,-15446,28897,-15446,
28377,-16383,28377,-16383,28377,-16383,28377,-16383,
27825,-17303,27825,-17303,27825,-17303,27825,-17303,
27244,-18204,27244,-18204,27244,-18204,27244,-18204,
26634,-19086,26634,-19086,26634,-19086,26634,-19086,
25995,-19947,25995,-19947,25995,-19947,25995,-19947,
25329,-20787,25329,-20787,25329,-20787,25329,-20787,
24635,-21604,24635,-21604,24635,-21604,24635,-21604,
23915,-22399,23915,-22399,23915,-22399,23915,-22399,
23169,-23169,23169,-23169,23169,-23169,23169,-23169,
22399,-23915,22399,-23915,22399,-23915,22399,-23915,
21604,-24635,21604,-24635,21604,-24635,21604,-24635,
20787,-25329,20787,-25329,20787,-25329,20787,-25329,
19947,-25995,19947,-25995,19947,-25995,19947,-25995,
19086,-26634,19086,-26634,19086,-26634,19086,-26634,
18204,-27244,18204,-27244,18204,-27244,18204,-27244,
17303,-27825,17303,-27825,17303,-27825,17303,-27825,
16383,-28377,16383,-28377,16383,-28377,16383,-28377,
15446,-28897,15446,-28897,15446,-28897,15446,-28897,
14492,-29387,14492,-29387,14492,-29387,14492,-29387,
13523,-29846,13523,-29846,13523,-29846,13523,-29846,
12539,-30272,12539,-30272,12539,-30272,12539,-30272,
11542,-30666,11542,-30666,11542,-30666,11542,-30666,
10532,-31028,10532,-31028,10532,-31028,10532,-31028,
9511,-31356,9511,-31356,9511,-31356,9511,-31356,
8480,-31650,8480,-31650,8480,-31650,8480,-31650,
7440,-31911,7440,-31911,7440,-31911,7440,-31911,
6392,-32137,6392,-32137,6392,-32137,6392,-32137,
5337,-32329,5337,-32329,5337,-32329,5337,-32329,
4276,-32486,4276,-32486,4276,-32486,4276,-32486,
3211,-32609,3211,-32609,3211,-32609,3211,-32609,
2143,-32696,2143,-32696,2143,-32696,2143,-32696,
1072,-32749,1072,-32749,1072,-32749,1072,-32749};

static int16_t twb192[376]__attribute__((aligned(16))) = {32696,-2143,32696,-2143,32696,-2143,32696,-2143,
32486,-4276,32486,-4276,32486,-4276,32486,-4276,
32137,-6392,32137,-6392,32137,-6392,32137,-6392,
31650,-8480,31650,-8480,31650,-8480,31650,-8480,
31028,-10532,31028,-10532,31028,-10532,31028,-10532,
30272,-12539,30272,-12539,30272,-12539,30272,-12539,
29387,-14492,29387,-14492,29387,-14492,29387,-14492,
28377,-16383,28377,-16383,28377,-16383,28377,-16383,
27244,-18204,27244,-18204,27244,-18204,27244,-18204,
25995,-19947,25995,-19947,25995,-19947,25995,-19947,
24635,-21604,24635,-21604,24635,-21604,24635,-21604,
23169,-23169,23169,-23169,23169,-23169,23169,-23169,
21604,-24635,21604,-24635,21604,-24635,21604,-24635,
19947,-25995,19947,-25995,19947,-25995,19947,-25995,
18204,-27244,18204,-27244,18204,-27244,18204,-27244,
16383,-28377,16383,-28377,16383,-28377,16383,-28377,
14492,-29387,14492,-29387,14492,-29387,14492,-29387,
12539,-30272,12539,-30272,12539,-30272,12539,-30272,
10532,-31028,10532,-31028,10532,-31028,10532,-31028,
8480,-31650,8480,-31650,8480,-31650,8480,-31650,
6392,-32137,6392,-32137,6392,-32137,6392,-32137,
4276,-32486,4276,-32486,4276,-32486,4276,-32486,
2143,-32696,2143,-32696,2143,-32696,2143,-32696,
0,-32767,0,-32767,0,-32767,0,-32767,
-2143,-32696,-2143,-32696,-2143,-32696,-2143,-32696,
-4276,-32486,-4276,-32486,-4276,-32486,-4276,-32486,
-6392,-32137,-6392,-32137,-6392,-32137,-6392,-32137,
-8480,-31650,-8480,-31650,-8480,-31650,-8480,-31650,
-10532,-31028,-10532,-31028,-10532,-31028,-10532,-31028,
-12539,-30272,-12539,-30272,-12539,-30272,-12539,-30272,
-14492,-29387,-14492,-29387,-14492,-29387,-14492,-29387,
-16383,-28377,-16383,-28377,-16383,-28377,-16383,-28377,
-18204,-27244,-18204,-27244,-18204,-27244,-18204,-27244,
-19947,-25995,-19947,-25995,-19947,-25995,-19947,-25995,
-21604,-24635,-21604,-24635,-21604,-24635,-21604,-24635,
-23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
-24635,-21604,-24635,-21604,-24635,-21604,-24635,-21604,
-25995,-19947,-25995,-19947,-25995,-19947,-25995,-19947,
-27244,-18204,-27244,-18204,-27244,-18204,-27244,-18204,
-28377,-16383,-28377,-16383,-28377,-16383,-28377,-16383,
-29387,-14492,-29387,-14492,-29387,-14492,-29387,-14492,
-30272,-12539,-30272,-12539,-30272,-12539,-30272,-12539,
-31028,-10532,-31028,-10532,-31028,-10532,-31028,-10532,
-31650,-8480,-31650,-8480,-31650,-8480,-31650,-8480,
-32137,-6392,-32137,-6392,-32137,-6392,-32137,-6392,
-32486,-4276,-32486,-4276,-32486,-4276,-32486,-4276,
-32696,-2143,-32696,-2143,-32696,-2143,-32696,-2143};

static int16_t twc192[376]__attribute__((aligned(16))) = {32609,-3211,32609,-3211,32609,-3211,32609,-3211,
32137,-6392,32137,-6392,32137,-6392,32137,-6392,
31356,-9511,31356,-9511,31356,-9511,31356,-9511,
30272,-12539,30272,-12539,30272,-12539,30272,-12539,
28897,-15446,28897,-15446,28897,-15446,28897,-15446,
27244,-18204,27244,-18204,27244,-18204,27244,-18204,
25329,-20787,25329,-20787,25329,-20787,25329,-20787,
23169,-23169,23169,-23169,23169,-23169,23169,-23169,
20787,-25329,20787,-25329,20787,-25329,20787,-25329,
18204,-27244,18204,-27244,18204,-27244,18204,-27244,
15446,-28897,15446,-28897,15446,-28897,15446,-28897,
12539,-30272,12539,-30272,12539,-30272,12539,-30272,
9511,-31356,9511,-31356,9511,-31356,9511,-31356,
6392,-32137,6392,-32137,6392,-32137,6392,-32137,
3211,-32609,3211,-32609,3211,-32609,3211,-32609,
0,-32767,0,-32767,0,-32767,0,-32767,
-3211,-32609,-3211,-32609,-3211,-32609,-3211,-32609,
-6392,-32137,-6392,-32137,-6392,-32137,-6392,-32137,
-9511,-31356,-9511,-31356,-9511,-31356,-9511,-31356,
-12539,-30272,-12539,-30272,-12539,-30272,-12539,-30272,
-15446,-28897,-15446,-28897,-15446,-28897,-15446,-28897,
-18204,-27244,-18204,-27244,-18204,-27244,-18204,-27244,
-20787,-25329,-20787,-25329,-20787,-25329,-20787,-25329,
-23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
-25329,-20787,-25329,-20787,-25329,-20787,-25329,-20787,
-27244,-18204,-27244,-18204,-27244,-18204,-27244,-18204,
-28897,-15446,-28897,-15446,-28897,-15446,-28897,-15446,
-30272,-12539,-30272,-12539,-30272,-12539,-30272,-12539,
-31356,-9511,-31356,-9511,-31356,-9511,-31356,-9511,
-32137,-6392,-32137,-6392,-32137,-6392,-32137,-6392,
-32609,-3211,-32609,-3211,-32609,-3211,-32609,-3211,
-32767,0,-32767,0,-32767,0,-32767,0,
-32609,3211,-32609,3211,-32609,3211,-32609,3211,
-32137,6392,-32137,6392,-32137,6392,-32137,6392,
-31356,9511,-31356,9511,-31356,9511,-31356,9511,
-30272,12539,-30272,12539,-30272,12539,-30272,12539,
-28897,15446,-28897,15446,-28897,15446,-28897,15446,
-27244,18204,-27244,18204,-27244,18204,-27244,18204,
-25329,20787,-25329,20787,-25329,20787,-25329,20787,
-23169,23169,-23169,23169,-23169,23169,-23169,23169,
-20787,25329,-20787,25329,-20787,25329,-20787,25329,
-18204,27244,-18204,27244,-18204,27244,-18204,27244,
-15446,28897,-15446,28897,-15446,28897,-15446,28897,
-12539,30272,-12539,30272,-12539,30272,-12539,30272,
-9511,31356,-9511,31356,-9511,31356,-9511,31356,
-6392,32137,-6392,32137,-6392,32137,-6392,32137,
-3211,32609,-3211,32609,-3211,32609,-3211,32609};

void dft192(int16_t *x,int16_t *y,unsigned char scale_flag){

  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa192[0];
  __m128i *twb128=(__m128i *)&twb192[0];
  __m128i *twc128=(__m128i *)&twc192[0];
  __m128i x2128[192];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[192];//=&ytmp128array2[0];



  for (i=0,j=0;i<48;i++,j+=4) {
    x2128[i]    = x128[j];
    x2128[i+48] = x128[j+1];
    x2128[i+96] = x128[j+2];
    x2128[i+144] = x128[j+3];
  }

  dft48((int16_t *)x2128,(int16_t *)ytmp128,1);
  dft48((int16_t *)(x2128+48),(int16_t *)(ytmp128+48),1);
  dft48((int16_t *)(x2128+96),(int16_t *)(ytmp128+96),1);
  dft48((int16_t *)(x2128+144),(int16_t *)(ytmp128+144),1);

  bfly4_tw1(ytmp128,ytmp128+48,ytmp128+96,ytmp128+144,y128,y128+48,y128+96,y128+144);
  for (i=1,j=0;i<48;i++,j++) {
    bfly4(ytmp128+i,
	  ytmp128+48+i,
	  ytmp128+96+i,
	  ytmp128+144+i,
	  y128+i,
	  y128+48+i,
	  y128+96+i,
	  y128+144+i,
	  twa128+j,
	  twb128+j,
	  twc128+j);
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(dft_norm_table[11]);
    
    for (i=0;i<192;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

}

static int16_t twa216[568]__attribute__((aligned(16))) = {32753,-953,32753,-953,32753,-953,32753,-953,
32711,-1905,32711,-1905,32711,-1905,32711,-1905,
32642,-2855,32642,-2855,32642,-2855,32642,-2855,
32545,-3804,32545,-3804,32545,-3804,32545,-3804,
32421,-4748,32421,-4748,32421,-4748,32421,-4748,
32269,-5689,32269,-5689,32269,-5689,32269,-5689,
32090,-6626,32090,-6626,32090,-6626,32090,-6626,
31883,-7556,31883,-7556,31883,-7556,31883,-7556,
31650,-8480,31650,-8480,31650,-8480,31650,-8480,
31390,-9397,31390,-9397,31390,-9397,31390,-9397,
31103,-10306,31103,-10306,31103,-10306,31103,-10306,
30790,-11206,30790,-11206,30790,-11206,30790,-11206,
30451,-12097,30451,-12097,30451,-12097,30451,-12097,
30087,-12978,30087,-12978,30087,-12978,30087,-12978,
29696,-13847,29696,-13847,29696,-13847,29696,-13847,
29281,-14705,29281,-14705,29281,-14705,29281,-14705,
28841,-15551,28841,-15551,28841,-15551,28841,-15551,
28377,-16383,28377,-16383,28377,-16383,28377,-16383,
27888,-17201,27888,-17201,27888,-17201,27888,-17201,
27376,-18005,27376,-18005,27376,-18005,27376,-18005,
26841,-18794,26841,-18794,26841,-18794,26841,-18794,
26283,-19567,26283,-19567,26283,-19567,26283,-19567,
25702,-20323,25702,-20323,25702,-20323,25702,-20323,
25100,-21062,25100,-21062,25100,-21062,25100,-21062,
24477,-21783,24477,-21783,24477,-21783,24477,-21783,
23833,-22486,23833,-22486,23833,-22486,23833,-22486,
23169,-23169,23169,-23169,23169,-23169,23169,-23169,
22486,-23833,22486,-23833,22486,-23833,22486,-23833,
21783,-24477,21783,-24477,21783,-24477,21783,-24477,
21062,-25100,21062,-25100,21062,-25100,21062,-25100,
20323,-25702,20323,-25702,20323,-25702,20323,-25702,
19567,-26283,19567,-26283,19567,-26283,19567,-26283,
18794,-26841,18794,-26841,18794,-26841,18794,-26841,
18005,-27376,18005,-27376,18005,-27376,18005,-27376,
17201,-27888,17201,-27888,17201,-27888,17201,-27888,
16383,-28377,16383,-28377,16383,-28377,16383,-28377,
15551,-28841,15551,-28841,15551,-28841,15551,-28841,
14705,-29281,14705,-29281,14705,-29281,14705,-29281,
13847,-29696,13847,-29696,13847,-29696,13847,-29696,
12978,-30087,12978,-30087,12978,-30087,12978,-30087,
12097,-30451,12097,-30451,12097,-30451,12097,-30451,
11206,-30790,11206,-30790,11206,-30790,11206,-30790,
10306,-31103,10306,-31103,10306,-31103,10306,-31103,
9397,-31390,9397,-31390,9397,-31390,9397,-31390,
8480,-31650,8480,-31650,8480,-31650,8480,-31650,
7556,-31883,7556,-31883,7556,-31883,7556,-31883,
6626,-32090,6626,-32090,6626,-32090,6626,-32090,
5689,-32269,5689,-32269,5689,-32269,5689,-32269,
4748,-32421,4748,-32421,4748,-32421,4748,-32421,
3804,-32545,3804,-32545,3804,-32545,3804,-32545,
2855,-32642,2855,-32642,2855,-32642,2855,-32642,
1905,-32711,1905,-32711,1905,-32711,1905,-32711,
953,-32753,953,-32753,953,-32753,953,-32753,
0,-32767,0,-32767,0,-32767,0,-32767,
-953,-32753,-953,-32753,-953,-32753,-953,-32753,
-1905,-32711,-1905,-32711,-1905,-32711,-1905,-32711,
-2855,-32642,-2855,-32642,-2855,-32642,-2855,-32642,
-3804,-32545,-3804,-32545,-3804,-32545,-3804,-32545,
-4748,-32421,-4748,-32421,-4748,-32421,-4748,-32421,
-5689,-32269,-5689,-32269,-5689,-32269,-5689,-32269,
-6626,-32090,-6626,-32090,-6626,-32090,-6626,-32090,
-7556,-31883,-7556,-31883,-7556,-31883,-7556,-31883,
-8480,-31650,-8480,-31650,-8480,-31650,-8480,-31650,
-9397,-31390,-9397,-31390,-9397,-31390,-9397,-31390,
-10306,-31103,-10306,-31103,-10306,-31103,-10306,-31103,
-11206,-30790,-11206,-30790,-11206,-30790,-11206,-30790,
-12097,-30451,-12097,-30451,-12097,-30451,-12097,-30451,
-12978,-30087,-12978,-30087,-12978,-30087,-12978,-30087,
-13847,-29696,-13847,-29696,-13847,-29696,-13847,-29696,
-14705,-29281,-14705,-29281,-14705,-29281,-14705,-29281,
-15551,-28841,-15551,-28841,-15551,-28841,-15551,-28841};

static int16_t twb216[568]__attribute__((aligned(16))) = {32711,-1905,32711,-1905,32711,-1905,32711,-1905,
32545,-3804,32545,-3804,32545,-3804,32545,-3804,
32269,-5689,32269,-5689,32269,-5689,32269,-5689,
31883,-7556,31883,-7556,31883,-7556,31883,-7556,
31390,-9397,31390,-9397,31390,-9397,31390,-9397,
30790,-11206,30790,-11206,30790,-11206,30790,-11206,
30087,-12978,30087,-12978,30087,-12978,30087,-12978,
29281,-14705,29281,-14705,29281,-14705,29281,-14705,
28377,-16383,28377,-16383,28377,-16383,28377,-16383,
27376,-18005,27376,-18005,27376,-18005,27376,-18005,
26283,-19567,26283,-19567,26283,-19567,26283,-19567,
25100,-21062,25100,-21062,25100,-21062,25100,-21062,
23833,-22486,23833,-22486,23833,-22486,23833,-22486,
22486,-23833,22486,-23833,22486,-23833,22486,-23833,
21062,-25100,21062,-25100,21062,-25100,21062,-25100,
19567,-26283,19567,-26283,19567,-26283,19567,-26283,
18005,-27376,18005,-27376,18005,-27376,18005,-27376,
16383,-28377,16383,-28377,16383,-28377,16383,-28377,
14705,-29281,14705,-29281,14705,-29281,14705,-29281,
12978,-30087,12978,-30087,12978,-30087,12978,-30087,
11206,-30790,11206,-30790,11206,-30790,11206,-30790,
9397,-31390,9397,-31390,9397,-31390,9397,-31390,
7556,-31883,7556,-31883,7556,-31883,7556,-31883,
5689,-32269,5689,-32269,5689,-32269,5689,-32269,
3804,-32545,3804,-32545,3804,-32545,3804,-32545,
1905,-32711,1905,-32711,1905,-32711,1905,-32711,
0,-32767,0,-32767,0,-32767,0,-32767,
-1905,-32711,-1905,-32711,-1905,-32711,-1905,-32711,
-3804,-32545,-3804,-32545,-3804,-32545,-3804,-32545,
-5689,-32269,-5689,-32269,-5689,-32269,-5689,-32269,
-7556,-31883,-7556,-31883,-7556,-31883,-7556,-31883,
-9397,-31390,-9397,-31390,-9397,-31390,-9397,-31390,
-11206,-30790,-11206,-30790,-11206,-30790,-11206,-30790,
-12978,-30087,-12978,-30087,-12978,-30087,-12978,-30087,
-14705,-29281,-14705,-29281,-14705,-29281,-14705,-29281,
-16383,-28377,-16383,-28377,-16383,-28377,-16383,-28377,
-18005,-27376,-18005,-27376,-18005,-27376,-18005,-27376,
-19567,-26283,-19567,-26283,-19567,-26283,-19567,-26283,
-21062,-25100,-21062,-25100,-21062,-25100,-21062,-25100,
-22486,-23833,-22486,-23833,-22486,-23833,-22486,-23833,
-23833,-22486,-23833,-22486,-23833,-22486,-23833,-22486,
-25100,-21062,-25100,-21062,-25100,-21062,-25100,-21062,
-26283,-19567,-26283,-19567,-26283,-19567,-26283,-19567,
-27376,-18005,-27376,-18005,-27376,-18005,-27376,-18005,
-28377,-16383,-28377,-16383,-28377,-16383,-28377,-16383,
-29281,-14705,-29281,-14705,-29281,-14705,-29281,-14705,
-30087,-12978,-30087,-12978,-30087,-12978,-30087,-12978,
-30790,-11206,-30790,-11206,-30790,-11206,-30790,-11206,
-31390,-9397,-31390,-9397,-31390,-9397,-31390,-9397,
-31883,-7556,-31883,-7556,-31883,-7556,-31883,-7556,
-32269,-5689,-32269,-5689,-32269,-5689,-32269,-5689,
-32545,-3804,-32545,-3804,-32545,-3804,-32545,-3804,
-32711,-1905,-32711,-1905,-32711,-1905,-32711,-1905,
-32767,0,-32767,0,-32767,0,-32767,0,
-32711,1905,-32711,1905,-32711,1905,-32711,1905,
-32545,3804,-32545,3804,-32545,3804,-32545,3804,
-32269,5689,-32269,5689,-32269,5689,-32269,5689,
-31883,7556,-31883,7556,-31883,7556,-31883,7556,
-31390,9397,-31390,9397,-31390,9397,-31390,9397,
-30790,11206,-30790,11206,-30790,11206,-30790,11206,
-30087,12978,-30087,12978,-30087,12978,-30087,12978,
-29281,14705,-29281,14705,-29281,14705,-29281,14705,
-28377,16383,-28377,16383,-28377,16383,-28377,16383,
-27376,18005,-27376,18005,-27376,18005,-27376,18005,
-26283,19567,-26283,19567,-26283,19567,-26283,19567,
-25100,21062,-25100,21062,-25100,21062,-25100,21062,
-23833,22486,-23833,22486,-23833,22486,-23833,22486,
-22486,23833,-22486,23833,-22486,23833,-22486,23833,
-21062,25100,-21062,25100,-21062,25100,-21062,25100,
-19567,26283,-19567,26283,-19567,26283,-19567,26283,
-18005,27376,-18005,27376,-18005,27376,-18005,27376};

void dft216(int16_t *x,int16_t *y,unsigned char scale_flag){

  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa216[0];
  __m128i *twb128=(__m128i *)&twb216[0];
  __m128i x2128[216];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[216];//=&ytmp128array3[0];



  for (i=0,j=0;i<72;i++,j+=3) {
    x2128[i]    = x128[j];
    x2128[i+72] = x128[j+1];
    x2128[i+144] = x128[j+2];
  }

  dft72((int16_t *)x2128,(int16_t *)ytmp128,1);
  dft72((int16_t *)(x2128+72),(int16_t *)(ytmp128+72),1);
  dft72((int16_t *)(x2128+144),(int16_t *)(ytmp128+144),1);

  bfly3_tw1(ytmp128,ytmp128+72,ytmp128+144,y128,y128+72,y128+144);
  for (i=1,j=0;i<72;i++,j++) {
    bfly3(ytmp128+i,
	  ytmp128+72+i,
	  ytmp128+144+i,
	  y128+i,
	  y128+72+i,
	  y128+144+i,
	  twa128+j,
	  twb128+j);
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(dft_norm_table[12]);
    
    for (i=0;i<216;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

}

static int16_t twa240[472]__attribute__((aligned(16))) = {32755,-857,32755,-857,32755,-857,32755,-857,
32722,-1714,32722,-1714,32722,-1714,32722,-1714,
32665,-2570,32665,-2570,32665,-2570,32665,-2570,
32587,-3425,32587,-3425,32587,-3425,32587,-3425,
32486,-4276,32486,-4276,32486,-4276,32486,-4276,
32363,-5125,32363,-5125,32363,-5125,32363,-5125,
32218,-5971,32218,-5971,32218,-5971,32218,-5971,
32050,-6812,32050,-6812,32050,-6812,32050,-6812,
31861,-7649,31861,-7649,31861,-7649,31861,-7649,
31650,-8480,31650,-8480,31650,-8480,31650,-8480,
31417,-9306,31417,-9306,31417,-9306,31417,-9306,
31163,-10125,31163,-10125,31163,-10125,31163,-10125,
30887,-10937,30887,-10937,30887,-10937,30887,-10937,
30590,-11742,30590,-11742,30590,-11742,30590,-11742,
30272,-12539,30272,-12539,30272,-12539,30272,-12539,
29934,-13327,29934,-13327,29934,-13327,29934,-13327,
29575,-14106,29575,-14106,29575,-14106,29575,-14106,
29195,-14875,29195,-14875,29195,-14875,29195,-14875,
28796,-15635,28796,-15635,28796,-15635,28796,-15635,
28377,-16383,28377,-16383,28377,-16383,28377,-16383,
27938,-17120,27938,-17120,27938,-17120,27938,-17120,
27480,-17846,27480,-17846,27480,-17846,27480,-17846,
27004,-18559,27004,-18559,27004,-18559,27004,-18559,
26509,-19259,26509,-19259,26509,-19259,26509,-19259,
25995,-19947,25995,-19947,25995,-19947,25995,-19947,
25464,-20620,25464,-20620,25464,-20620,25464,-20620,
24916,-21280,24916,-21280,24916,-21280,24916,-21280,
24350,-21925,24350,-21925,24350,-21925,24350,-21925,
23768,-22555,23768,-22555,23768,-22555,23768,-22555,
23169,-23169,23169,-23169,23169,-23169,23169,-23169,
22555,-23768,22555,-23768,22555,-23768,22555,-23768,
21925,-24350,21925,-24350,21925,-24350,21925,-24350,
21280,-24916,21280,-24916,21280,-24916,21280,-24916,
20620,-25464,20620,-25464,20620,-25464,20620,-25464,
19947,-25995,19947,-25995,19947,-25995,19947,-25995,
19259,-26509,19259,-26509,19259,-26509,19259,-26509,
18559,-27004,18559,-27004,18559,-27004,18559,-27004,
17846,-27480,17846,-27480,17846,-27480,17846,-27480,
17120,-27938,17120,-27938,17120,-27938,17120,-27938,
16383,-28377,16383,-28377,16383,-28377,16383,-28377,
15635,-28796,15635,-28796,15635,-28796,15635,-28796,
14875,-29195,14875,-29195,14875,-29195,14875,-29195,
14106,-29575,14106,-29575,14106,-29575,14106,-29575,
13327,-29934,13327,-29934,13327,-29934,13327,-29934,
12539,-30272,12539,-30272,12539,-30272,12539,-30272,
11742,-30590,11742,-30590,11742,-30590,11742,-30590,
10937,-30887,10937,-30887,10937,-30887,10937,-30887,
10125,-31163,10125,-31163,10125,-31163,10125,-31163,
9306,-31417,9306,-31417,9306,-31417,9306,-31417,
8480,-31650,8480,-31650,8480,-31650,8480,-31650,
7649,-31861,7649,-31861,7649,-31861,7649,-31861,
6812,-32050,6812,-32050,6812,-32050,6812,-32050,
5971,-32218,5971,-32218,5971,-32218,5971,-32218,
5125,-32363,5125,-32363,5125,-32363,5125,-32363,
4276,-32486,4276,-32486,4276,-32486,4276,-32486,
3425,-32587,3425,-32587,3425,-32587,3425,-32587,
2570,-32665,2570,-32665,2570,-32665,2570,-32665,
1714,-32722,1714,-32722,1714,-32722,1714,-32722,
857,-32755,857,-32755,857,-32755,857,-32755};

static int16_t twb240[472]__attribute__((aligned(16))) = {32722,-1714,32722,-1714,32722,-1714,32722,-1714,
32587,-3425,32587,-3425,32587,-3425,32587,-3425,
32363,-5125,32363,-5125,32363,-5125,32363,-5125,
32050,-6812,32050,-6812,32050,-6812,32050,-6812,
31650,-8480,31650,-8480,31650,-8480,31650,-8480,
31163,-10125,31163,-10125,31163,-10125,31163,-10125,
30590,-11742,30590,-11742,30590,-11742,30590,-11742,
29934,-13327,29934,-13327,29934,-13327,29934,-13327,
29195,-14875,29195,-14875,29195,-14875,29195,-14875,
28377,-16383,28377,-16383,28377,-16383,28377,-16383,
27480,-17846,27480,-17846,27480,-17846,27480,-17846,
26509,-19259,26509,-19259,26509,-19259,26509,-19259,
25464,-20620,25464,-20620,25464,-20620,25464,-20620,
24350,-21925,24350,-21925,24350,-21925,24350,-21925,
23169,-23169,23169,-23169,23169,-23169,23169,-23169,
21925,-24350,21925,-24350,21925,-24350,21925,-24350,
20620,-25464,20620,-25464,20620,-25464,20620,-25464,
19259,-26509,19259,-26509,19259,-26509,19259,-26509,
17846,-27480,17846,-27480,17846,-27480,17846,-27480,
16383,-28377,16383,-28377,16383,-28377,16383,-28377,
14875,-29195,14875,-29195,14875,-29195,14875,-29195,
13327,-29934,13327,-29934,13327,-29934,13327,-29934,
11742,-30590,11742,-30590,11742,-30590,11742,-30590,
10125,-31163,10125,-31163,10125,-31163,10125,-31163,
8480,-31650,8480,-31650,8480,-31650,8480,-31650,
6812,-32050,6812,-32050,6812,-32050,6812,-32050,
5125,-32363,5125,-32363,5125,-32363,5125,-32363,
3425,-32587,3425,-32587,3425,-32587,3425,-32587,
1714,-32722,1714,-32722,1714,-32722,1714,-32722,
0,-32767,0,-32767,0,-32767,0,-32767,
-1714,-32722,-1714,-32722,-1714,-32722,-1714,-32722,
-3425,-32587,-3425,-32587,-3425,-32587,-3425,-32587,
-5125,-32363,-5125,-32363,-5125,-32363,-5125,-32363,
-6812,-32050,-6812,-32050,-6812,-32050,-6812,-32050,
-8480,-31650,-8480,-31650,-8480,-31650,-8480,-31650,
-10125,-31163,-10125,-31163,-10125,-31163,-10125,-31163,
-11742,-30590,-11742,-30590,-11742,-30590,-11742,-30590,
-13327,-29934,-13327,-29934,-13327,-29934,-13327,-29934,
-14875,-29195,-14875,-29195,-14875,-29195,-14875,-29195,
-16383,-28377,-16383,-28377,-16383,-28377,-16383,-28377,
-17846,-27480,-17846,-27480,-17846,-27480,-17846,-27480,
-19259,-26509,-19259,-26509,-19259,-26509,-19259,-26509,
-20620,-25464,-20620,-25464,-20620,-25464,-20620,-25464,
-21925,-24350,-21925,-24350,-21925,-24350,-21925,-24350,
-23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
-24350,-21925,-24350,-21925,-24350,-21925,-24350,-21925,
-25464,-20620,-25464,-20620,-25464,-20620,-25464,-20620,
-26509,-19259,-26509,-19259,-26509,-19259,-26509,-19259,
-27480,-17846,-27480,-17846,-27480,-17846,-27480,-17846,
-28377,-16383,-28377,-16383,-28377,-16383,-28377,-16383,
-29195,-14875,-29195,-14875,-29195,-14875,-29195,-14875,
-29934,-13327,-29934,-13327,-29934,-13327,-29934,-13327,
-30590,-11742,-30590,-11742,-30590,-11742,-30590,-11742,
-31163,-10125,-31163,-10125,-31163,-10125,-31163,-10125,
-31650,-8480,-31650,-8480,-31650,-8480,-31650,-8480,
-32050,-6812,-32050,-6812,-32050,-6812,-32050,-6812,
-32363,-5125,-32363,-5125,-32363,-5125,-32363,-5125,
-32587,-3425,-32587,-3425,-32587,-3425,-32587,-3425,
-32722,-1714,-32722,-1714,-32722,-1714,-32722,-1714};

static int16_t twc240[472]__attribute__((aligned(16))) = {32665,-2570,32665,-2570,32665,-2570,32665,-2570,
32363,-5125,32363,-5125,32363,-5125,32363,-5125,
31861,-7649,31861,-7649,31861,-7649,31861,-7649,
31163,-10125,31163,-10125,31163,-10125,31163,-10125,
30272,-12539,30272,-12539,30272,-12539,30272,-12539,
29195,-14875,29195,-14875,29195,-14875,29195,-14875,
27938,-17120,27938,-17120,27938,-17120,27938,-17120,
26509,-19259,26509,-19259,26509,-19259,26509,-19259,
24916,-21280,24916,-21280,24916,-21280,24916,-21280,
23169,-23169,23169,-23169,23169,-23169,23169,-23169,
21280,-24916,21280,-24916,21280,-24916,21280,-24916,
19259,-26509,19259,-26509,19259,-26509,19259,-26509,
17120,-27938,17120,-27938,17120,-27938,17120,-27938,
14875,-29195,14875,-29195,14875,-29195,14875,-29195,
12539,-30272,12539,-30272,12539,-30272,12539,-30272,
10125,-31163,10125,-31163,10125,-31163,10125,-31163,
7649,-31861,7649,-31861,7649,-31861,7649,-31861,
5125,-32363,5125,-32363,5125,-32363,5125,-32363,
2570,-32665,2570,-32665,2570,-32665,2570,-32665,
0,-32767,0,-32767,0,-32767,0,-32767,
-2570,-32665,-2570,-32665,-2570,-32665,-2570,-32665,
-5125,-32363,-5125,-32363,-5125,-32363,-5125,-32363,
-7649,-31861,-7649,-31861,-7649,-31861,-7649,-31861,
-10125,-31163,-10125,-31163,-10125,-31163,-10125,-31163,
-12539,-30272,-12539,-30272,-12539,-30272,-12539,-30272,
-14875,-29195,-14875,-29195,-14875,-29195,-14875,-29195,
-17120,-27938,-17120,-27938,-17120,-27938,-17120,-27938,
-19259,-26509,-19259,-26509,-19259,-26509,-19259,-26509,
-21280,-24916,-21280,-24916,-21280,-24916,-21280,-24916,
-23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
-24916,-21280,-24916,-21280,-24916,-21280,-24916,-21280,
-26509,-19259,-26509,-19259,-26509,-19259,-26509,-19259,
-27938,-17120,-27938,-17120,-27938,-17120,-27938,-17120,
-29195,-14875,-29195,-14875,-29195,-14875,-29195,-14875,
-30272,-12539,-30272,-12539,-30272,-12539,-30272,-12539,
-31163,-10125,-31163,-10125,-31163,-10125,-31163,-10125,
-31861,-7649,-31861,-7649,-31861,-7649,-31861,-7649,
-32363,-5125,-32363,-5125,-32363,-5125,-32363,-5125,
-32665,-2570,-32665,-2570,-32665,-2570,-32665,-2570,
-32767,0,-32767,0,-32767,0,-32767,0,
-32665,2570,-32665,2570,-32665,2570,-32665,2570,
-32363,5125,-32363,5125,-32363,5125,-32363,5125,
-31861,7649,-31861,7649,-31861,7649,-31861,7649,
-31163,10125,-31163,10125,-31163,10125,-31163,10125,
-30272,12539,-30272,12539,-30272,12539,-30272,12539,
-29195,14875,-29195,14875,-29195,14875,-29195,14875,
-27938,17120,-27938,17120,-27938,17120,-27938,17120,
-26509,19259,-26509,19259,-26509,19259,-26509,19259,
-24916,21280,-24916,21280,-24916,21280,-24916,21280,
-23169,23169,-23169,23169,-23169,23169,-23169,23169,
-21280,24916,-21280,24916,-21280,24916,-21280,24916,
-19259,26509,-19259,26509,-19259,26509,-19259,26509,
-17120,27938,-17120,27938,-17120,27938,-17120,27938,
-14875,29195,-14875,29195,-14875,29195,-14875,29195,
-12539,30272,-12539,30272,-12539,30272,-12539,30272,
-10125,31163,-10125,31163,-10125,31163,-10125,31163,
-7649,31861,-7649,31861,-7649,31861,-7649,31861,
-5125,32363,-5125,32363,-5125,32363,-5125,32363,
-2570,32665,-2570,32665,-2570,32665,-2570,32665};

void dft240(int16_t *x,int16_t *y,unsigned char scale_flag){

  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa240[0];
  __m128i *twb128=(__m128i *)&twb240[0];
  __m128i *twc128=(__m128i *)&twc240[0];
  __m128i x2128[240];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[240];//=&ytmp128array2[0];



  for (i=0,j=0;i<60;i++,j+=4) {
    x2128[i]    = x128[j];
    x2128[i+60] = x128[j+1];
    x2128[i+120] = x128[j+2];
    x2128[i+180] = x128[j+3];
  }

  dft60((int16_t *)x2128,(int16_t *)ytmp128,1);
  dft60((int16_t *)(x2128+60),(int16_t *)(ytmp128+60),1);
  dft60((int16_t *)(x2128+120),(int16_t *)(ytmp128+120),1);
  dft60((int16_t *)(x2128+180),(int16_t *)(ytmp128+180),1);

  bfly4_tw1(ytmp128,ytmp128+60,ytmp128+120,ytmp128+180,y128,y128+60,y128+120,y128+180);
  for (i=1,j=0;i<60;i++,j++) {
    bfly4(ytmp128+i,
	  ytmp128+60+i,
	  ytmp128+120+i,
	  ytmp128+180+i,
	  y128+i,
	  y128+60+i,
	  y128+120+i,
	  y128+180+i,
	  twa128+j,
	  twb128+j,
	  twc128+j);
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(dft_norm_table[13]);
    
    for (i=0;i<240;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

}

/* Twiddles generated with
twa = floor(32767*exp(-sqrt(-1)*2*pi*(1:95)/288));
twb = floor(32767*exp(-sqrt(-1)*2*pi*(2:2:190)/288));
twa2 = zeros(1,191);
twb2 = zeros(1,191);
twa2(1:2:end) = real(twa);
twa2(2:2:end) = imag(twa);
twb2(1:2:end) = real(twb);
twb2(2:2:end) = imag(twb);


 */
static int16_t twa288[760]__attribute__((aligned(16))) = {32759,-714,32759,-714,32759,-714,32759,-714,
32735,-1429,32735,-1429,32735,-1429,32735,-1429,
32696,-2143,32696,-2143,32696,-2143,32696,-2143,
32642,-2855,32642,-2855,32642,-2855,32642,-2855,
32572,-3567,32572,-3567,32572,-3567,32572,-3567,
32486,-4276,32486,-4276,32486,-4276,32486,-4276,
32385,-4984,32385,-4984,32385,-4984,32385,-4984,
32269,-5689,32269,-5689,32269,-5689,32269,-5689,
32137,-6392,32137,-6392,32137,-6392,32137,-6392,
31990,-7092,31990,-7092,31990,-7092,31990,-7092,
31827,-7788,31827,-7788,31827,-7788,31827,-7788,
31650,-8480,31650,-8480,31650,-8480,31650,-8480,
31457,-9169,31457,-9169,31457,-9169,31457,-9169,
31250,-9853,31250,-9853,31250,-9853,31250,-9853,
31028,-10532,31028,-10532,31028,-10532,31028,-10532,
30790,-11206,30790,-11206,30790,-11206,30790,-11206,
30539,-11876,30539,-11876,30539,-11876,30539,-11876,
30272,-12539,30272,-12539,30272,-12539,30272,-12539,
29992,-13196,29992,-13196,29992,-13196,29992,-13196,
29696,-13847,29696,-13847,29696,-13847,29696,-13847,
29387,-14492,29387,-14492,29387,-14492,29387,-14492,
29064,-15130,29064,-15130,29064,-15130,29064,-15130,
28727,-15760,28727,-15760,28727,-15760,28727,-15760,
28377,-16383,28377,-16383,28377,-16383,28377,-16383,
28012,-16998,28012,-16998,28012,-16998,28012,-16998,
27635,-17605,27635,-17605,27635,-17605,27635,-17605,
27244,-18204,27244,-18204,27244,-18204,27244,-18204,
26841,-18794,26841,-18794,26841,-18794,26841,-18794,
26424,-19375,26424,-19375,26424,-19375,26424,-19375,
25995,-19947,25995,-19947,25995,-19947,25995,-19947,
25554,-20509,25554,-20509,25554,-20509,25554,-20509,
25100,-21062,25100,-21062,25100,-21062,25100,-21062,
24635,-21604,24635,-21604,24635,-21604,24635,-21604,
24158,-22137,24158,-22137,24158,-22137,24158,-22137,
23669,-22658,23669,-22658,23669,-22658,23669,-22658,
23169,-23169,23169,-23169,23169,-23169,23169,-23169,
22658,-23669,22658,-23669,22658,-23669,22658,-23669,
22137,-24158,22137,-24158,22137,-24158,22137,-24158,
21604,-24635,21604,-24635,21604,-24635,21604,-24635,
21062,-25100,21062,-25100,21062,-25100,21062,-25100,
20509,-25554,20509,-25554,20509,-25554,20509,-25554,
19947,-25995,19947,-25995,19947,-25995,19947,-25995,
19375,-26424,19375,-26424,19375,-26424,19375,-26424,
18794,-26841,18794,-26841,18794,-26841,18794,-26841,
18204,-27244,18204,-27244,18204,-27244,18204,-27244,
17605,-27635,17605,-27635,17605,-27635,17605,-27635,
16998,-28012,16998,-28012,16998,-28012,16998,-28012,
16383,-28377,16383,-28377,16383,-28377,16383,-28377,
15760,-28727,15760,-28727,15760,-28727,15760,-28727,
15130,-29064,15130,-29064,15130,-29064,15130,-29064,
14492,-29387,14492,-29387,14492,-29387,14492,-29387,
13847,-29696,13847,-29696,13847,-29696,13847,-29696,
13196,-29992,13196,-29992,13196,-29992,13196,-29992,
12539,-30272,12539,-30272,12539,-30272,12539,-30272,
11876,-30539,11876,-30539,11876,-30539,11876,-30539,
11206,-30790,11206,-30790,11206,-30790,11206,-30790,
10532,-31028,10532,-31028,10532,-31028,10532,-31028,
9853,-31250,9853,-31250,9853,-31250,9853,-31250,
9169,-31457,9169,-31457,9169,-31457,9169,-31457,
8480,-31650,8480,-31650,8480,-31650,8480,-31650,
7788,-31827,7788,-31827,7788,-31827,7788,-31827,
7092,-31990,7092,-31990,7092,-31990,7092,-31990,
6392,-32137,6392,-32137,6392,-32137,6392,-32137,
5689,-32269,5689,-32269,5689,-32269,5689,-32269,
4984,-32385,4984,-32385,4984,-32385,4984,-32385,
4276,-32486,4276,-32486,4276,-32486,4276,-32486,
3567,-32572,3567,-32572,3567,-32572,3567,-32572,
2855,-32642,2855,-32642,2855,-32642,2855,-32642,
2143,-32696,2143,-32696,2143,-32696,2143,-32696,
1429,-32735,1429,-32735,1429,-32735,1429,-32735,
714,-32759,714,-32759,714,-32759,714,-32759,
0,-32767,0,-32767,0,-32767,0,-32767,
-714,-32759,-714,-32759,-714,-32759,-714,-32759,
-1429,-32735,-1429,-32735,-1429,-32735,-1429,-32735,
-2143,-32696,-2143,-32696,-2143,-32696,-2143,-32696,
-2855,-32642,-2855,-32642,-2855,-32642,-2855,-32642,
-3567,-32572,-3567,-32572,-3567,-32572,-3567,-32572,
-4276,-32486,-4276,-32486,-4276,-32486,-4276,-32486,
-4984,-32385,-4984,-32385,-4984,-32385,-4984,-32385,
-5689,-32269,-5689,-32269,-5689,-32269,-5689,-32269,
-6392,-32137,-6392,-32137,-6392,-32137,-6392,-32137,
-7092,-31990,-7092,-31990,-7092,-31990,-7092,-31990,
-7788,-31827,-7788,-31827,-7788,-31827,-7788,-31827,
-8480,-31650,-8480,-31650,-8480,-31650,-8480,-31650,
-9169,-31457,-9169,-31457,-9169,-31457,-9169,-31457,
-9853,-31250,-9853,-31250,-9853,-31250,-9853,-31250,
-10532,-31028,-10532,-31028,-10532,-31028,-10532,-31028,
-11206,-30790,-11206,-30790,-11206,-30790,-11206,-30790,
-11876,-30539,-11876,-30539,-11876,-30539,-11876,-30539,
-12539,-30272,-12539,-30272,-12539,-30272,-12539,-30272,
-13196,-29992,-13196,-29992,-13196,-29992,-13196,-29992,
-13847,-29696,-13847,-29696,-13847,-29696,-13847,-29696,
-14492,-29387,-14492,-29387,-14492,-29387,-14492,-29387,
-15130,-29064,-15130,-29064,-15130,-29064,-15130,-29064,
-15760,-28727,-15760,-28727,-15760,-28727,-15760,-28727};

static int16_t twb288[760]__attribute__((aligned(16))) = {32735,-1429,32735,-1429,32735,-1429,32735,-1429,
32642,-2855,32642,-2855,32642,-2855,32642,-2855,
32486,-4276,32486,-4276,32486,-4276,32486,-4276,
32269,-5689,32269,-5689,32269,-5689,32269,-5689,
31990,-7092,31990,-7092,31990,-7092,31990,-7092,
31650,-8480,31650,-8480,31650,-8480,31650,-8480,
31250,-9853,31250,-9853,31250,-9853,31250,-9853,
30790,-11206,30790,-11206,30790,-11206,30790,-11206,
30272,-12539,30272,-12539,30272,-12539,30272,-12539,
29696,-13847,29696,-13847,29696,-13847,29696,-13847,
29064,-15130,29064,-15130,29064,-15130,29064,-15130,
28377,-16383,28377,-16383,28377,-16383,28377,-16383,
27635,-17605,27635,-17605,27635,-17605,27635,-17605,
26841,-18794,26841,-18794,26841,-18794,26841,-18794,
25995,-19947,25995,-19947,25995,-19947,25995,-19947,
25100,-21062,25100,-21062,25100,-21062,25100,-21062,
24158,-22137,24158,-22137,24158,-22137,24158,-22137,
23169,-23169,23169,-23169,23169,-23169,23169,-23169,
22137,-24158,22137,-24158,22137,-24158,22137,-24158,
21062,-25100,21062,-25100,21062,-25100,21062,-25100,
19947,-25995,19947,-25995,19947,-25995,19947,-25995,
18794,-26841,18794,-26841,18794,-26841,18794,-26841,
17605,-27635,17605,-27635,17605,-27635,17605,-27635,
16383,-28377,16383,-28377,16383,-28377,16383,-28377,
15130,-29064,15130,-29064,15130,-29064,15130,-29064,
13847,-29696,13847,-29696,13847,-29696,13847,-29696,
12539,-30272,12539,-30272,12539,-30272,12539,-30272,
11206,-30790,11206,-30790,11206,-30790,11206,-30790,
9853,-31250,9853,-31250,9853,-31250,9853,-31250,
8480,-31650,8480,-31650,8480,-31650,8480,-31650,
7092,-31990,7092,-31990,7092,-31990,7092,-31990,
5689,-32269,5689,-32269,5689,-32269,5689,-32269,
4276,-32486,4276,-32486,4276,-32486,4276,-32486,
2855,-32642,2855,-32642,2855,-32642,2855,-32642,
1429,-32735,1429,-32735,1429,-32735,1429,-32735,
0,-32767,0,-32767,0,-32767,0,-32767,
-1429,-32735,-1429,-32735,-1429,-32735,-1429,-32735,
-2855,-32642,-2855,-32642,-2855,-32642,-2855,-32642,
-4276,-32486,-4276,-32486,-4276,-32486,-4276,-32486,
-5689,-32269,-5689,-32269,-5689,-32269,-5689,-32269,
-7092,-31990,-7092,-31990,-7092,-31990,-7092,-31990,
-8480,-31650,-8480,-31650,-8480,-31650,-8480,-31650,
-9853,-31250,-9853,-31250,-9853,-31250,-9853,-31250,
-11206,-30790,-11206,-30790,-11206,-30790,-11206,-30790,
-12539,-30272,-12539,-30272,-12539,-30272,-12539,-30272,
-13847,-29696,-13847,-29696,-13847,-29696,-13847,-29696,
-15130,-29064,-15130,-29064,-15130,-29064,-15130,-29064,
-16383,-28377,-16383,-28377,-16383,-28377,-16383,-28377,
-17605,-27635,-17605,-27635,-17605,-27635,-17605,-27635,
-18794,-26841,-18794,-26841,-18794,-26841,-18794,-26841,
-19947,-25995,-19947,-25995,-19947,-25995,-19947,-25995,
-21062,-25100,-21062,-25100,-21062,-25100,-21062,-25100,
-22137,-24158,-22137,-24158,-22137,-24158,-22137,-24158,
-23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
-24158,-22137,-24158,-22137,-24158,-22137,-24158,-22137,
-25100,-21062,-25100,-21062,-25100,-21062,-25100,-21062,
-25995,-19947,-25995,-19947,-25995,-19947,-25995,-19947,
-26841,-18794,-26841,-18794,-26841,-18794,-26841,-18794,
-27635,-17605,-27635,-17605,-27635,-17605,-27635,-17605,
-28377,-16383,-28377,-16383,-28377,-16383,-28377,-16383,
-29064,-15130,-29064,-15130,-29064,-15130,-29064,-15130,
-29696,-13847,-29696,-13847,-29696,-13847,-29696,-13847,
-30272,-12539,-30272,-12539,-30272,-12539,-30272,-12539,
-30790,-11206,-30790,-11206,-30790,-11206,-30790,-11206,
-31250,-9853,-31250,-9853,-31250,-9853,-31250,-9853,
-31650,-8480,-31650,-8480,-31650,-8480,-31650,-8480,
-31990,-7092,-31990,-7092,-31990,-7092,-31990,-7092,
-32269,-5689,-32269,-5689,-32269,-5689,-32269,-5689,
-32486,-4276,-32486,-4276,-32486,-4276,-32486,-4276,
-32642,-2855,-32642,-2855,-32642,-2855,-32642,-2855,
-32735,-1429,-32735,-1429,-32735,-1429,-32735,-1429,
-32767,0,-32767,0,-32767,0,-32767,0,
-32735,1429,-32735,1429,-32735,1429,-32735,1429,
-32642,2855,-32642,2855,-32642,2855,-32642,2855,
-32486,4276,-32486,4276,-32486,4276,-32486,4276,
-32269,5689,-32269,5689,-32269,5689,-32269,5689,
-31990,7092,-31990,7092,-31990,7092,-31990,7092,
-31650,8480,-31650,8480,-31650,8480,-31650,8480,
-31250,9853,-31250,9853,-31250,9853,-31250,9853,
-30790,11206,-30790,11206,-30790,11206,-30790,11206,
-30272,12539,-30272,12539,-30272,12539,-30272,12539,
-29696,13847,-29696,13847,-29696,13847,-29696,13847,
-29064,15130,-29064,15130,-29064,15130,-29064,15130,
-28377,16383,-28377,16383,-28377,16383,-28377,16383,
-27635,17605,-27635,17605,-27635,17605,-27635,17605,
-26841,18794,-26841,18794,-26841,18794,-26841,18794,
-25995,19947,-25995,19947,-25995,19947,-25995,19947,
-25100,21062,-25100,21062,-25100,21062,-25100,21062,
-24158,22137,-24158,22137,-24158,22137,-24158,22137,
-23169,23169,-23169,23169,-23169,23169,-23169,23169,
-22137,24158,-22137,24158,-22137,24158,-22137,24158,
-21062,25100,-21062,25100,-21062,25100,-21062,25100,
-19947,25995,-19947,25995,-19947,25995,-19947,25995,
-18794,26841,-18794,26841,-18794,26841,-18794,26841,
-17605,27635,-17605,27635,-17605,27635,-17605,27635};

void dft288(int16_t *x,int16_t *y,unsigned char scale_flag){

  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa288[0];
  __m128i *twb128=(__m128i *)&twb288[0];
  __m128i x2128[288];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[288];//=&ytmp128array3[0];



  for (i=0,j=0;i<96;i++,j+=3) {
    x2128[i]    = x128[j];
    x2128[i+96] = x128[j+1];
    x2128[i+192] = x128[j+2];
  }

  dft96((int16_t *)x2128,(int16_t *)ytmp128,1);
  dft96((int16_t *)(x2128+96),(int16_t *)(ytmp128+96),1);
  dft96((int16_t *)(x2128+192),(int16_t *)(ytmp128+192),1);

  bfly3_tw1(ytmp128,ytmp128+96,ytmp128+192,y128,y128+96,y128+192);
  for (i=1,j=0;i<96;i++,j++) {
    bfly3(ytmp128+i,
	  ytmp128+96+i,
	  ytmp128+192+i,
	  y128+i,
	  y128+96+i,
	  y128+192+i,
	  twa128+j,
	  twb128+j);
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(dft_norm_table[14]);
    
    for (i=0;i<288;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

}

static int16_t twa300[472]__attribute__((aligned(16))) = {32759,-686,32759,-686,32759,-686,32759,-686,
32738,-1372,32738,-1372,32738,-1372,32738,-1372,
32702,-2057,32702,-2057,32702,-2057,32702,-2057,
32652,-2741,32652,-2741,32652,-2741,32652,-2741,
32587,-3425,32587,-3425,32587,-3425,32587,-3425,
32508,-4106,32508,-4106,32508,-4106,32508,-4106,
32415,-4786,32415,-4786,32415,-4786,32415,-4786,
32308,-5464,32308,-5464,32308,-5464,32308,-5464,
32186,-6139,32186,-6139,32186,-6139,32186,-6139,
32050,-6812,32050,-6812,32050,-6812,32050,-6812,
31901,-7482,31901,-7482,31901,-7482,31901,-7482,
31737,-8148,31737,-8148,31737,-8148,31737,-8148,
31559,-8811,31559,-8811,31559,-8811,31559,-8811,
31368,-9470,31368,-9470,31368,-9470,31368,-9470,
31163,-10125,31163,-10125,31163,-10125,31163,-10125,
30944,-10775,30944,-10775,30944,-10775,30944,-10775,
30711,-11421,30711,-11421,30711,-11421,30711,-11421,
30465,-12062,30465,-12062,30465,-12062,30465,-12062,
30206,-12697,30206,-12697,30206,-12697,30206,-12697,
29934,-13327,29934,-13327,29934,-13327,29934,-13327,
29648,-13951,29648,-13951,29648,-13951,29648,-13951,
29349,-14569,29349,-14569,29349,-14569,29349,-14569,
29038,-15180,29038,-15180,29038,-15180,29038,-15180,
28713,-15785,28713,-15785,28713,-15785,28713,-15785,
28377,-16383,28377,-16383,28377,-16383,28377,-16383,
28027,-16974,28027,-16974,28027,-16974,28027,-16974,
27666,-17557,27666,-17557,27666,-17557,27666,-17557,
27292,-18132,27292,-18132,27292,-18132,27292,-18132,
26906,-18700,26906,-18700,26906,-18700,26906,-18700,
26509,-19259,26509,-19259,26509,-19259,26509,-19259,
26099,-19810,26099,-19810,26099,-19810,26099,-19810,
25679,-20353,25679,-20353,25679,-20353,25679,-20353,
25247,-20886,25247,-20886,25247,-20886,25247,-20886,
24804,-21410,24804,-21410,24804,-21410,24804,-21410,
24350,-21925,24350,-21925,24350,-21925,24350,-21925,
23886,-22430,23886,-22430,23886,-22430,23886,-22430,
23411,-22925,23411,-22925,23411,-22925,23411,-22925,
22925,-23411,22925,-23411,22925,-23411,22925,-23411,
22430,-23886,22430,-23886,22430,-23886,22430,-23886,
21925,-24350,21925,-24350,21925,-24350,21925,-24350,
21410,-24804,21410,-24804,21410,-24804,21410,-24804,
20886,-25247,20886,-25247,20886,-25247,20886,-25247,
20353,-25679,20353,-25679,20353,-25679,20353,-25679,
19810,-26099,19810,-26099,19810,-26099,19810,-26099,
19259,-26509,19259,-26509,19259,-26509,19259,-26509,
18700,-26906,18700,-26906,18700,-26906,18700,-26906,
18132,-27292,18132,-27292,18132,-27292,18132,-27292,
17557,-27666,17557,-27666,17557,-27666,17557,-27666,
16974,-28027,16974,-28027,16974,-28027,16974,-28027,
16383,-28377,16383,-28377,16383,-28377,16383,-28377,
15785,-28713,15785,-28713,15785,-28713,15785,-28713,
15180,-29038,15180,-29038,15180,-29038,15180,-29038,
14569,-29349,14569,-29349,14569,-29349,14569,-29349,
13951,-29648,13951,-29648,13951,-29648,13951,-29648,
13327,-29934,13327,-29934,13327,-29934,13327,-29934,
12697,-30206,12697,-30206,12697,-30206,12697,-30206,
12062,-30465,12062,-30465,12062,-30465,12062,-30465,
11421,-30711,11421,-30711,11421,-30711,11421,-30711,
10775,-30944,10775,-30944,10775,-30944,10775,-30944};

static int16_t twb300[472]__attribute__((aligned(16))) = {32738,-1372,32738,-1372,32738,-1372,32738,-1372,
32652,-2741,32652,-2741,32652,-2741,32652,-2741,
32508,-4106,32508,-4106,32508,-4106,32508,-4106,
32308,-5464,32308,-5464,32308,-5464,32308,-5464,
32050,-6812,32050,-6812,32050,-6812,32050,-6812,
31737,-8148,31737,-8148,31737,-8148,31737,-8148,
31368,-9470,31368,-9470,31368,-9470,31368,-9470,
30944,-10775,30944,-10775,30944,-10775,30944,-10775,
30465,-12062,30465,-12062,30465,-12062,30465,-12062,
29934,-13327,29934,-13327,29934,-13327,29934,-13327,
29349,-14569,29349,-14569,29349,-14569,29349,-14569,
28713,-15785,28713,-15785,28713,-15785,28713,-15785,
28027,-16974,28027,-16974,28027,-16974,28027,-16974,
27292,-18132,27292,-18132,27292,-18132,27292,-18132,
26509,-19259,26509,-19259,26509,-19259,26509,-19259,
25679,-20353,25679,-20353,25679,-20353,25679,-20353,
24804,-21410,24804,-21410,24804,-21410,24804,-21410,
23886,-22430,23886,-22430,23886,-22430,23886,-22430,
22925,-23411,22925,-23411,22925,-23411,22925,-23411,
21925,-24350,21925,-24350,21925,-24350,21925,-24350,
20886,-25247,20886,-25247,20886,-25247,20886,-25247,
19810,-26099,19810,-26099,19810,-26099,19810,-26099,
18700,-26906,18700,-26906,18700,-26906,18700,-26906,
17557,-27666,17557,-27666,17557,-27666,17557,-27666,
16383,-28377,16383,-28377,16383,-28377,16383,-28377,
15180,-29038,15180,-29038,15180,-29038,15180,-29038,
13951,-29648,13951,-29648,13951,-29648,13951,-29648,
12697,-30206,12697,-30206,12697,-30206,12697,-30206,
11421,-30711,11421,-30711,11421,-30711,11421,-30711,
10125,-31163,10125,-31163,10125,-31163,10125,-31163,
8811,-31559,8811,-31559,8811,-31559,8811,-31559,
7482,-31901,7482,-31901,7482,-31901,7482,-31901,
6139,-32186,6139,-32186,6139,-32186,6139,-32186,
4786,-32415,4786,-32415,4786,-32415,4786,-32415,
3425,-32587,3425,-32587,3425,-32587,3425,-32587,
2057,-32702,2057,-32702,2057,-32702,2057,-32702,
686,-32759,686,-32759,686,-32759,686,-32759,
-686,-32759,-686,-32759,-686,-32759,-686,-32759,
-2057,-32702,-2057,-32702,-2057,-32702,-2057,-32702,
-3425,-32587,-3425,-32587,-3425,-32587,-3425,-32587,
-4786,-32415,-4786,-32415,-4786,-32415,-4786,-32415,
-6139,-32186,-6139,-32186,-6139,-32186,-6139,-32186,
-7482,-31901,-7482,-31901,-7482,-31901,-7482,-31901,
-8811,-31559,-8811,-31559,-8811,-31559,-8811,-31559,
-10125,-31163,-10125,-31163,-10125,-31163,-10125,-31163,
-11421,-30711,-11421,-30711,-11421,-30711,-11421,-30711,
-12697,-30206,-12697,-30206,-12697,-30206,-12697,-30206,
-13951,-29648,-13951,-29648,-13951,-29648,-13951,-29648,
-15180,-29038,-15180,-29038,-15180,-29038,-15180,-29038,
-16383,-28377,-16383,-28377,-16383,-28377,-16383,-28377,
-17557,-27666,-17557,-27666,-17557,-27666,-17557,-27666,
-18700,-26906,-18700,-26906,-18700,-26906,-18700,-26906,
-19810,-26099,-19810,-26099,-19810,-26099,-19810,-26099,
-20886,-25247,-20886,-25247,-20886,-25247,-20886,-25247,
-21925,-24350,-21925,-24350,-21925,-24350,-21925,-24350,
-22925,-23411,-22925,-23411,-22925,-23411,-22925,-23411,
-23886,-22430,-23886,-22430,-23886,-22430,-23886,-22430,
-24804,-21410,-24804,-21410,-24804,-21410,-24804,-21410,
-25679,-20353,-25679,-20353,-25679,-20353,-25679,-20353};

static int16_t twc300[472]__attribute__((aligned(16))) = {32702,-2057,32702,-2057,32702,-2057,32702,-2057,
32508,-4106,32508,-4106,32508,-4106,32508,-4106,
32186,-6139,32186,-6139,32186,-6139,32186,-6139,
31737,-8148,31737,-8148,31737,-8148,31737,-8148,
31163,-10125,31163,-10125,31163,-10125,31163,-10125,
30465,-12062,30465,-12062,30465,-12062,30465,-12062,
29648,-13951,29648,-13951,29648,-13951,29648,-13951,
28713,-15785,28713,-15785,28713,-15785,28713,-15785,
27666,-17557,27666,-17557,27666,-17557,27666,-17557,
26509,-19259,26509,-19259,26509,-19259,26509,-19259,
25247,-20886,25247,-20886,25247,-20886,25247,-20886,
23886,-22430,23886,-22430,23886,-22430,23886,-22430,
22430,-23886,22430,-23886,22430,-23886,22430,-23886,
20886,-25247,20886,-25247,20886,-25247,20886,-25247,
19259,-26509,19259,-26509,19259,-26509,19259,-26509,
17557,-27666,17557,-27666,17557,-27666,17557,-27666,
15785,-28713,15785,-28713,15785,-28713,15785,-28713,
13951,-29648,13951,-29648,13951,-29648,13951,-29648,
12062,-30465,12062,-30465,12062,-30465,12062,-30465,
10125,-31163,10125,-31163,10125,-31163,10125,-31163,
8148,-31737,8148,-31737,8148,-31737,8148,-31737,
6139,-32186,6139,-32186,6139,-32186,6139,-32186,
4106,-32508,4106,-32508,4106,-32508,4106,-32508,
2057,-32702,2057,-32702,2057,-32702,2057,-32702,
0,-32767,0,-32767,0,-32767,0,-32767,
-2057,-32702,-2057,-32702,-2057,-32702,-2057,-32702,
-4106,-32508,-4106,-32508,-4106,-32508,-4106,-32508,
-6139,-32186,-6139,-32186,-6139,-32186,-6139,-32186,
-8148,-31737,-8148,-31737,-8148,-31737,-8148,-31737,
-10125,-31163,-10125,-31163,-10125,-31163,-10125,-31163,
-12062,-30465,-12062,-30465,-12062,-30465,-12062,-30465,
-13951,-29648,-13951,-29648,-13951,-29648,-13951,-29648,
-15785,-28713,-15785,-28713,-15785,-28713,-15785,-28713,
-17557,-27666,-17557,-27666,-17557,-27666,-17557,-27666,
-19259,-26509,-19259,-26509,-19259,-26509,-19259,-26509,
-20886,-25247,-20886,-25247,-20886,-25247,-20886,-25247,
-22430,-23886,-22430,-23886,-22430,-23886,-22430,-23886,
-23886,-22430,-23886,-22430,-23886,-22430,-23886,-22430,
-25247,-20886,-25247,-20886,-25247,-20886,-25247,-20886,
-26509,-19259,-26509,-19259,-26509,-19259,-26509,-19259,
-27666,-17557,-27666,-17557,-27666,-17557,-27666,-17557,
-28713,-15785,-28713,-15785,-28713,-15785,-28713,-15785,
-29648,-13951,-29648,-13951,-29648,-13951,-29648,-13951,
-30465,-12062,-30465,-12062,-30465,-12062,-30465,-12062,
-31163,-10125,-31163,-10125,-31163,-10125,-31163,-10125,
-31737,-8148,-31737,-8148,-31737,-8148,-31737,-8148,
-32186,-6139,-32186,-6139,-32186,-6139,-32186,-6139,
-32508,-4106,-32508,-4106,-32508,-4106,-32508,-4106,
-32702,-2057,-32702,-2057,-32702,-2057,-32702,-2057,
-32767,0,-32767,0,-32767,0,-32767,0,
-32702,2057,-32702,2057,-32702,2057,-32702,2057,
-32508,4106,-32508,4106,-32508,4106,-32508,4106,
-32186,6139,-32186,6139,-32186,6139,-32186,6139,
-31737,8148,-31737,8148,-31737,8148,-31737,8148,
-31163,10125,-31163,10125,-31163,10125,-31163,10125,
-30465,12062,-30465,12062,-30465,12062,-30465,12062,
-29648,13951,-29648,13951,-29648,13951,-29648,13951,
-28713,15785,-28713,15785,-28713,15785,-28713,15785,
-27666,17557,-27666,17557,-27666,17557,-27666,17557};

static int16_t twd300[472]__attribute__((aligned(16))) = {32652,-2741,32652,-2741,32652,-2741,32652,-2741,
32308,-5464,32308,-5464,32308,-5464,32308,-5464,
31737,-8148,31737,-8148,31737,-8148,31737,-8148,
30944,-10775,30944,-10775,30944,-10775,30944,-10775,
29934,-13327,29934,-13327,29934,-13327,29934,-13327,
28713,-15785,28713,-15785,28713,-15785,28713,-15785,
27292,-18132,27292,-18132,27292,-18132,27292,-18132,
25679,-20353,25679,-20353,25679,-20353,25679,-20353,
23886,-22430,23886,-22430,23886,-22430,23886,-22430,
21925,-24350,21925,-24350,21925,-24350,21925,-24350,
19810,-26099,19810,-26099,19810,-26099,19810,-26099,
17557,-27666,17557,-27666,17557,-27666,17557,-27666,
15180,-29038,15180,-29038,15180,-29038,15180,-29038,
12697,-30206,12697,-30206,12697,-30206,12697,-30206,
10125,-31163,10125,-31163,10125,-31163,10125,-31163,
7482,-31901,7482,-31901,7482,-31901,7482,-31901,
4786,-32415,4786,-32415,4786,-32415,4786,-32415,
2057,-32702,2057,-32702,2057,-32702,2057,-32702,
-686,-32759,-686,-32759,-686,-32759,-686,-32759,
-3425,-32587,-3425,-32587,-3425,-32587,-3425,-32587,
-6139,-32186,-6139,-32186,-6139,-32186,-6139,-32186,
-8811,-31559,-8811,-31559,-8811,-31559,-8811,-31559,
-11421,-30711,-11421,-30711,-11421,-30711,-11421,-30711,
-13951,-29648,-13951,-29648,-13951,-29648,-13951,-29648,
-16383,-28377,-16383,-28377,-16383,-28377,-16383,-28377,
-18700,-26906,-18700,-26906,-18700,-26906,-18700,-26906,
-20886,-25247,-20886,-25247,-20886,-25247,-20886,-25247,
-22925,-23411,-22925,-23411,-22925,-23411,-22925,-23411,
-24804,-21410,-24804,-21410,-24804,-21410,-24804,-21410,
-26509,-19259,-26509,-19259,-26509,-19259,-26509,-19259,
-28027,-16974,-28027,-16974,-28027,-16974,-28027,-16974,
-29349,-14569,-29349,-14569,-29349,-14569,-29349,-14569,
-30465,-12062,-30465,-12062,-30465,-12062,-30465,-12062,
-31368,-9470,-31368,-9470,-31368,-9470,-31368,-9470,
-32050,-6812,-32050,-6812,-32050,-6812,-32050,-6812,
-32508,-4106,-32508,-4106,-32508,-4106,-32508,-4106,
-32738,-1372,-32738,-1372,-32738,-1372,-32738,-1372,
-32738,1372,-32738,1372,-32738,1372,-32738,1372,
-32508,4106,-32508,4106,-32508,4106,-32508,4106,
-32050,6812,-32050,6812,-32050,6812,-32050,6812,
-31368,9470,-31368,9470,-31368,9470,-31368,9470,
-30465,12062,-30465,12062,-30465,12062,-30465,12062,
-29349,14569,-29349,14569,-29349,14569,-29349,14569,
-28027,16974,-28027,16974,-28027,16974,-28027,16974,
-26509,19259,-26509,19259,-26509,19259,-26509,19259,
-24804,21410,-24804,21410,-24804,21410,-24804,21410,
-22925,23411,-22925,23411,-22925,23411,-22925,23411,
-20886,25247,-20886,25247,-20886,25247,-20886,25247,
-18700,26906,-18700,26906,-18700,26906,-18700,26906,
-16383,28377,-16383,28377,-16383,28377,-16383,28377,
-13951,29648,-13951,29648,-13951,29648,-13951,29648,
-11421,30711,-11421,30711,-11421,30711,-11421,30711,
-8811,31559,-8811,31559,-8811,31559,-8811,31559,
-6139,32186,-6139,32186,-6139,32186,-6139,32186,
-3425,32587,-3425,32587,-3425,32587,-3425,32587,
-686,32759,-686,32759,-686,32759,-686,32759,
2057,32702,2057,32702,2057,32702,2057,32702,
4786,32415,4786,32415,4786,32415,4786,32415,
7482,31901,7482,31901,7482,31901,7482,31901};

void dft300(int16_t *x,int16_t *y,unsigned char scale_flag){

  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa300[0];
  __m128i *twb128=(__m128i *)&twb300[0];
  __m128i *twc128=(__m128i *)&twc300[0];
  __m128i *twd128=(__m128i *)&twd300[0];
  __m128i x2128[300];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[300];//=&ytmp128array2[0];



  for (i=0,j=0;i<60;i++,j+=5) {
    x2128[i]    = x128[j];
    x2128[i+60] = x128[j+1];
    x2128[i+120] = x128[j+2];
    x2128[i+180] = x128[j+3];
    x2128[i+240] = x128[j+4];
  }

  dft60((int16_t *)x2128,(int16_t *)ytmp128,1);
  dft60((int16_t *)(x2128+60),(int16_t *)(ytmp128+60),1);
  dft60((int16_t *)(x2128+120),(int16_t *)(ytmp128+120),1);
  dft60((int16_t *)(x2128+180),(int16_t *)(ytmp128+180),1);
  dft60((int16_t *)(x2128+240),(int16_t *)(ytmp128+240),1);

  bfly5_tw1(ytmp128,ytmp128+60,ytmp128+120,ytmp128+180,ytmp128+240,y128,y128+60,y128+120,y128+180,y128+240);
  for (i=1,j=0;i<60;i++,j++) {
    bfly5(ytmp128+i,
	  ytmp128+60+i,
	  ytmp128+120+i,
	  ytmp128+180+i,
	  ytmp128+240+i,
	  y128+i,
	  y128+60+i,
	  y128+120+i,
	  y128+180+i,
	  y128+240+i,
	  twa128+j,
	  twb128+j,
	  twc128+j,
	  twd128+j);
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(dft_norm_table[15]);
    
    for (i=0;i<300;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

}

/* Twiddles generated with
twa = floor(32767*exp(-sqrt(-1)*2*pi*(1:107)/324));
twb = floor(32767*exp(-sqrt(-1)*2*pi*2*(1:107)/324));
twa2 = zeros(1,2*107);
twb2 = zeros(1,2*107);
twa2(1:2:end) = real(twa);
twa2(2:2:end) = imag(twa);
twb2(1:2:end) = real(twb);
twb2(2:2:end) = imag(twb);
fd=fopen("twiddle_tmp.txt","w");
fprintf(fd,"static int16_t twa324[107*2*4] = {");
for i=1:2:(2*106)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
fprintf(fd,"static int16_t twb324[107*2*4] = {");
for i=1:2:(2*106)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
fclose(fd);
 */
static int16_t twa324[107*2*4] = {32760,-636,32760,-636,32760,-636,32760,-636,
32742,-1271,32742,-1271,32742,-1271,32742,-1271,
32711,-1906,32711,-1906,32711,-1906,32711,-1906,
32668,-2540,32668,-2540,32668,-2540,32668,-2540,
32613,-3173,32613,-3173,32613,-3173,32613,-3173,
32545,-3805,32545,-3805,32545,-3805,32545,-3805,
32465,-4435,32465,-4435,32465,-4435,32465,-4435,
32373,-5064,32373,-5064,32373,-5064,32373,-5064,
32269,-5690,32269,-5690,32269,-5690,32269,-5690,
32152,-6315,32152,-6315,32152,-6315,32152,-6315,
32024,-6937,32024,-6937,32024,-6937,32024,-6937,
31883,-7557,31883,-7557,31883,-7557,31883,-7557,
31731,-8174,31731,-8174,31731,-8174,31731,-8174,
31566,-8788,31566,-8788,31566,-8788,31566,-8788,
31390,-9398,31390,-9398,31390,-9398,31390,-9398,
31202,-10005,31202,-10005,31202,-10005,31202,-10005,
31002,-10608,31002,-10608,31002,-10608,31002,-10608,
30790,-11207,30790,-11207,30790,-11207,30790,-11207,
30567,-11802,30567,-11802,30567,-11802,30567,-11802,
30333,-12393,30333,-12393,30333,-12393,30333,-12393,
30087,-12979,30087,-12979,30087,-12979,30087,-12979,
29829,-13560,29829,-13560,29829,-13560,29829,-13560,
29561,-14136,29561,-14136,29561,-14136,29561,-14136,
29281,-14706,29281,-14706,29281,-14706,29281,-14706,
28990,-15271,28990,-15271,28990,-15271,28990,-15271,
28689,-15831,28689,-15831,28689,-15831,28689,-15831,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
28054,-16931,28054,-16931,28054,-16931,28054,-16931,
27720,-17472,27720,-17472,27720,-17472,27720,-17472,
27376,-18006,27376,-18006,27376,-18006,27376,-18006,
27022,-18534,27022,-18534,27022,-18534,27022,-18534,
26657,-19054,26657,-19054,26657,-19054,26657,-19054,
26283,-19568,26283,-19568,26283,-19568,26283,-19568,
25898,-20074,25898,-20074,25898,-20074,25898,-20074,
25504,-20572,25504,-20572,25504,-20572,25504,-20572,
25100,-21063,25100,-21063,25100,-21063,25100,-21063,
24687,-21546,24687,-21546,24687,-21546,24687,-21546,
24265,-22020,24265,-22020,24265,-22020,24265,-22020,
23833,-22487,23833,-22487,23833,-22487,23833,-22487,
23393,-22945,23393,-22945,23393,-22945,23393,-22945,
22944,-23394,22944,-23394,22944,-23394,22944,-23394,
22486,-23834,22486,-23834,22486,-23834,22486,-23834,
22019,-24266,22019,-24266,22019,-24266,22019,-24266,
21545,-24688,21545,-24688,21545,-24688,21545,-24688,
21062,-25101,21062,-25101,21062,-25101,21062,-25101,
20571,-25505,20571,-25505,20571,-25505,20571,-25505,
20073,-25899,20073,-25899,20073,-25899,20073,-25899,
19567,-26284,19567,-26284,19567,-26284,19567,-26284,
19053,-26658,19053,-26658,19053,-26658,19053,-26658,
18533,-27023,18533,-27023,18533,-27023,18533,-27023,
18005,-27377,18005,-27377,18005,-27377,18005,-27377,
17471,-27721,17471,-27721,17471,-27721,17471,-27721,
16930,-28055,16930,-28055,16930,-28055,16930,-28055,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
15830,-28690,15830,-28690,15830,-28690,15830,-28690,
15270,-28991,15270,-28991,15270,-28991,15270,-28991,
14705,-29282,14705,-29282,14705,-29282,14705,-29282,
14135,-29562,14135,-29562,14135,-29562,14135,-29562,
13559,-29830,13559,-29830,13559,-29830,13559,-29830,
12978,-30088,12978,-30088,12978,-30088,12978,-30088,
12392,-30334,12392,-30334,12392,-30334,12392,-30334,
11801,-30568,11801,-30568,11801,-30568,11801,-30568,
11206,-30791,11206,-30791,11206,-30791,11206,-30791,
10607,-31003,10607,-31003,10607,-31003,10607,-31003,
10004,-31203,10004,-31203,10004,-31203,10004,-31203,
9397,-31391,9397,-31391,9397,-31391,9397,-31391,
8787,-31567,8787,-31567,8787,-31567,8787,-31567,
8173,-31732,8173,-31732,8173,-31732,8173,-31732,
7556,-31884,7556,-31884,7556,-31884,7556,-31884,
6936,-32025,6936,-32025,6936,-32025,6936,-32025,
6314,-32153,6314,-32153,6314,-32153,6314,-32153,
5689,-32270,5689,-32270,5689,-32270,5689,-32270,
5063,-32374,5063,-32374,5063,-32374,5063,-32374,
4434,-32466,4434,-32466,4434,-32466,4434,-32466,
3804,-32546,3804,-32546,3804,-32546,3804,-32546,
3172,-32614,3172,-32614,3172,-32614,3172,-32614,
2539,-32669,2539,-32669,2539,-32669,2539,-32669,
1905,-32712,1905,-32712,1905,-32712,1905,-32712,
1270,-32743,1270,-32743,1270,-32743,1270,-32743,
635,-32761,635,-32761,635,-32761,635,-32761,
0,-32767,0,-32767,0,-32767,0,-32767,
-636,-32761,-636,-32761,-636,-32761,-636,-32761,
-1271,-32743,-1271,-32743,-1271,-32743,-1271,-32743,
-1906,-32712,-1906,-32712,-1906,-32712,-1906,-32712,
-2540,-32669,-2540,-32669,-2540,-32669,-2540,-32669,
-3173,-32614,-3173,-32614,-3173,-32614,-3173,-32614,
-3805,-32546,-3805,-32546,-3805,-32546,-3805,-32546,
-4435,-32466,-4435,-32466,-4435,-32466,-4435,-32466,
-5064,-32374,-5064,-32374,-5064,-32374,-5064,-32374,
-5690,-32270,-5690,-32270,-5690,-32270,-5690,-32270,
-6315,-32153,-6315,-32153,-6315,-32153,-6315,-32153,
-6937,-32025,-6937,-32025,-6937,-32025,-6937,-32025,
-7557,-31884,-7557,-31884,-7557,-31884,-7557,-31884,
-8174,-31732,-8174,-31732,-8174,-31732,-8174,-31732,
-8788,-31567,-8788,-31567,-8788,-31567,-8788,-31567,
-9398,-31391,-9398,-31391,-9398,-31391,-9398,-31391,
-10005,-31203,-10005,-31203,-10005,-31203,-10005,-31203,
-10608,-31003,-10608,-31003,-10608,-31003,-10608,-31003,
-11207,-30791,-11207,-30791,-11207,-30791,-11207,-30791,
-11802,-30568,-11802,-30568,-11802,-30568,-11802,-30568,
-12393,-30334,-12393,-30334,-12393,-30334,-12393,-30334,
-12979,-30088,-12979,-30088,-12979,-30088,-12979,-30088,
-13560,-29830,-13560,-29830,-13560,-29830,-13560,-29830,
-14136,-29562,-14136,-29562,-14136,-29562,-14136,-29562,
-14706,-29282,-14706,-29282,-14706,-29282,-14706,-29282,
-15271,-28991,-15271,-28991,-15271,-28991,-15271,-28991,
-15831,-28690,-15831,-28690,-15831,-28690,-15831,-28690};
static int16_t twb324[107*2*4] = {32742,-1271,32742,-1271,32742,-1271,32742,-1271,
32668,-2540,32668,-2540,32668,-2540,32668,-2540,
32545,-3805,32545,-3805,32545,-3805,32545,-3805,
32373,-5064,32373,-5064,32373,-5064,32373,-5064,
32152,-6315,32152,-6315,32152,-6315,32152,-6315,
31883,-7557,31883,-7557,31883,-7557,31883,-7557,
31566,-8788,31566,-8788,31566,-8788,31566,-8788,
31202,-10005,31202,-10005,31202,-10005,31202,-10005,
30790,-11207,30790,-11207,30790,-11207,30790,-11207,
30333,-12393,30333,-12393,30333,-12393,30333,-12393,
29829,-13560,29829,-13560,29829,-13560,29829,-13560,
29281,-14706,29281,-14706,29281,-14706,29281,-14706,
28689,-15831,28689,-15831,28689,-15831,28689,-15831,
28054,-16931,28054,-16931,28054,-16931,28054,-16931,
27376,-18006,27376,-18006,27376,-18006,27376,-18006,
26657,-19054,26657,-19054,26657,-19054,26657,-19054,
25898,-20074,25898,-20074,25898,-20074,25898,-20074,
25100,-21063,25100,-21063,25100,-21063,25100,-21063,
24265,-22020,24265,-22020,24265,-22020,24265,-22020,
23393,-22945,23393,-22945,23393,-22945,23393,-22945,
22486,-23834,22486,-23834,22486,-23834,22486,-23834,
21545,-24688,21545,-24688,21545,-24688,21545,-24688,
20571,-25505,20571,-25505,20571,-25505,20571,-25505,
19567,-26284,19567,-26284,19567,-26284,19567,-26284,
18533,-27023,18533,-27023,18533,-27023,18533,-27023,
17471,-27721,17471,-27721,17471,-27721,17471,-27721,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
15270,-28991,15270,-28991,15270,-28991,15270,-28991,
14135,-29562,14135,-29562,14135,-29562,14135,-29562,
12978,-30088,12978,-30088,12978,-30088,12978,-30088,
11801,-30568,11801,-30568,11801,-30568,11801,-30568,
10607,-31003,10607,-31003,10607,-31003,10607,-31003,
9397,-31391,9397,-31391,9397,-31391,9397,-31391,
8173,-31732,8173,-31732,8173,-31732,8173,-31732,
6936,-32025,6936,-32025,6936,-32025,6936,-32025,
5689,-32270,5689,-32270,5689,-32270,5689,-32270,
4434,-32466,4434,-32466,4434,-32466,4434,-32466,
3172,-32614,3172,-32614,3172,-32614,3172,-32614,
1905,-32712,1905,-32712,1905,-32712,1905,-32712,
635,-32761,635,-32761,635,-32761,635,-32761,
-636,-32761,-636,-32761,-636,-32761,-636,-32761,
-1906,-32712,-1906,-32712,-1906,-32712,-1906,-32712,
-3173,-32614,-3173,-32614,-3173,-32614,-3173,-32614,
-4435,-32466,-4435,-32466,-4435,-32466,-4435,-32466,
-5690,-32270,-5690,-32270,-5690,-32270,-5690,-32270,
-6937,-32025,-6937,-32025,-6937,-32025,-6937,-32025,
-8174,-31732,-8174,-31732,-8174,-31732,-8174,-31732,
-9398,-31391,-9398,-31391,-9398,-31391,-9398,-31391,
-10608,-31003,-10608,-31003,-10608,-31003,-10608,-31003,
-11802,-30568,-11802,-30568,-11802,-30568,-11802,-30568,
-12979,-30088,-12979,-30088,-12979,-30088,-12979,-30088,
-14136,-29562,-14136,-29562,-14136,-29562,-14136,-29562,
-15271,-28991,-15271,-28991,-15271,-28991,-15271,-28991,
-16384,-28378,-16384,-28378,-16384,-28378,-16384,-28378,
-17472,-27721,-17472,-27721,-17472,-27721,-17472,-27721,
-18534,-27023,-18534,-27023,-18534,-27023,-18534,-27023,
-19568,-26284,-19568,-26284,-19568,-26284,-19568,-26284,
-20572,-25505,-20572,-25505,-20572,-25505,-20572,-25505,
-21546,-24688,-21546,-24688,-21546,-24688,-21546,-24688,
-22487,-23834,-22487,-23834,-22487,-23834,-22487,-23834,
-23394,-22945,-23394,-22945,-23394,-22945,-23394,-22945,
-24266,-22020,-24266,-22020,-24266,-22020,-24266,-22020,
-25101,-21063,-25101,-21063,-25101,-21063,-25101,-21063,
-25899,-20074,-25899,-20074,-25899,-20074,-25899,-20074,
-26658,-19054,-26658,-19054,-26658,-19054,-26658,-19054,
-27377,-18006,-27377,-18006,-27377,-18006,-27377,-18006,
-28055,-16931,-28055,-16931,-28055,-16931,-28055,-16931,
-28690,-15831,-28690,-15831,-28690,-15831,-28690,-15831,
-29282,-14706,-29282,-14706,-29282,-14706,-29282,-14706,
-29830,-13560,-29830,-13560,-29830,-13560,-29830,-13560,
-30334,-12393,-30334,-12393,-30334,-12393,-30334,-12393,
-30791,-11207,-30791,-11207,-30791,-11207,-30791,-11207,
-31203,-10005,-31203,-10005,-31203,-10005,-31203,-10005,
-31567,-8788,-31567,-8788,-31567,-8788,-31567,-8788,
-31884,-7557,-31884,-7557,-31884,-7557,-31884,-7557,
-32153,-6315,-32153,-6315,-32153,-6315,-32153,-6315,
-32374,-5064,-32374,-5064,-32374,-5064,-32374,-5064,
-32546,-3805,-32546,-3805,-32546,-3805,-32546,-3805,
-32669,-2540,-32669,-2540,-32669,-2540,-32669,-2540,
-32743,-1271,-32743,-1271,-32743,-1271,-32743,-1271,
-32767,-1,-32767,-1,-32767,-1,-32767,-1,
-32743,1270,-32743,1270,-32743,1270,-32743,1270,
-32669,2539,-32669,2539,-32669,2539,-32669,2539,
-32546,3804,-32546,3804,-32546,3804,-32546,3804,
-32374,5063,-32374,5063,-32374,5063,-32374,5063,
-32153,6314,-32153,6314,-32153,6314,-32153,6314,
-31884,7556,-31884,7556,-31884,7556,-31884,7556,
-31567,8787,-31567,8787,-31567,8787,-31567,8787,
-31203,10004,-31203,10004,-31203,10004,-31203,10004,
-30791,11206,-30791,11206,-30791,11206,-30791,11206,
-30334,12392,-30334,12392,-30334,12392,-30334,12392,
-29830,13559,-29830,13559,-29830,13559,-29830,13559,
-29282,14705,-29282,14705,-29282,14705,-29282,14705,
-28690,15830,-28690,15830,-28690,15830,-28690,15830,
-28055,16930,-28055,16930,-28055,16930,-28055,16930,
-27377,18005,-27377,18005,-27377,18005,-27377,18005,
-26658,19053,-26658,19053,-26658,19053,-26658,19053,
-25899,20073,-25899,20073,-25899,20073,-25899,20073,
-25101,21062,-25101,21062,-25101,21062,-25101,21062,
-24266,22019,-24266,22019,-24266,22019,-24266,22019,
-23394,22944,-23394,22944,-23394,22944,-23394,22944,
-22487,23833,-22487,23833,-22487,23833,-22487,23833,
-21546,24687,-21546,24687,-21546,24687,-21546,24687,
-20572,25504,-20572,25504,-20572,25504,-20572,25504,
-19568,26283,-19568,26283,-19568,26283,-19568,26283,
-18534,27022,-18534,27022,-18534,27022,-18534,27022,
-17472,27720,-17472,27720,-17472,27720,-17472,27720};

void dft324(int16_t *x,int16_t *y,unsigned char scale_flag){ // 108 x 3
  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa324[0];
  __m128i *twb128=(__m128i *)&twb324[0];
  __m128i x2128[324];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[324];//=&ytmp128array3[0];



  for (i=0,j=0;i<108;i++,j+=3) {
    x2128[i]    = x128[j];
    x2128[i+108] = x128[j+1];
    x2128[i+216] = x128[j+2];
  }

  dft108((int16_t *)x2128,(int16_t *)ytmp128,1);
  dft108((int16_t *)(x2128+108),(int16_t *)(ytmp128+108),1);
  dft108((int16_t *)(x2128+216),(int16_t *)(ytmp128+216),1);

  bfly3_tw1(ytmp128,ytmp128+108,ytmp128+216,y128,y128+108,y128+216);
  for (i=1,j=0;i<108;i++,j++) {
    bfly3(ytmp128+i,
	  ytmp128+108+i,
	  ytmp128+216+i,
	  y128+i,
	  y128+108+i,
	  y128+216+i,
	  twa128+j,
	  twb128+j);
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(dft_norm_table[14]);
    
    for (i=0;i<324;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

};

/* Twiddles generated with
twa = floor(32767*exp(-sqrt(-1)*2*pi*(1:119)/360));
twb = floor(32767*exp(-sqrt(-1)*2*pi*2*(1:119)/360));
twa2 = zeros(1,2*119);
twb2 = zeros(1,2*119);
twa2(1:2:end) = real(twa);
twa2(2:2:end) = imag(twa);
twb2(1:2:end) = real(twb);
twb2(2:2:end) = imag(twb);
fd=fopen("twiddle_tmp.txt","w");
fprintf(fd,"static int16_t twa360[119*2*4] = {");
for i=1:2:(2*118)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
fprintf(fd,"static int16_t twb360[119*2*4] = {");
for i=1:2:(2*118)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
fclose(fd);
 */
static int16_t twa360[119*2*4] = {32762,-572,32762,-572,32762,-572,32762,-572,
32747,-1144,32747,-1144,32747,-1144,32747,-1144,
32722,-1715,32722,-1715,32722,-1715,32722,-1715,
32687,-2286,32687,-2286,32687,-2286,32687,-2286,
32642,-2856,32642,-2856,32642,-2856,32642,-2856,
32587,-3426,32587,-3426,32587,-3426,32587,-3426,
32522,-3994,32522,-3994,32522,-3994,32522,-3994,
32448,-4561,32448,-4561,32448,-4561,32448,-4561,
32363,-5126,32363,-5126,32363,-5126,32363,-5126,
32269,-5690,32269,-5690,32269,-5690,32269,-5690,
32164,-6253,32164,-6253,32164,-6253,32164,-6253,
32050,-6813,32050,-6813,32050,-6813,32050,-6813,
31927,-7371,31927,-7371,31927,-7371,31927,-7371,
31793,-7928,31793,-7928,31793,-7928,31793,-7928,
31650,-8481,31650,-8481,31650,-8481,31650,-8481,
31497,-9032,31497,-9032,31497,-9032,31497,-9032,
31335,-9581,31335,-9581,31335,-9581,31335,-9581,
31163,-10126,31163,-10126,31163,-10126,31163,-10126,
30981,-10668,30981,-10668,30981,-10668,30981,-10668,
30790,-11207,30790,-11207,30790,-11207,30790,-11207,
30590,-11743,30590,-11743,30590,-11743,30590,-11743,
30381,-12275,30381,-12275,30381,-12275,30381,-12275,
30162,-12804,30162,-12804,30162,-12804,30162,-12804,
29934,-13328,29934,-13328,29934,-13328,29934,-13328,
29696,-13848,29696,-13848,29696,-13848,29696,-13848,
29450,-14365,29450,-14365,29450,-14365,29450,-14365,
29195,-14876,29195,-14876,29195,-14876,29195,-14876,
28931,-15384,28931,-15384,28931,-15384,28931,-15384,
28658,-15886,28658,-15886,28658,-15886,28658,-15886,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
28086,-16877,28086,-16877,28086,-16877,28086,-16877,
27787,-17364,27787,-17364,27787,-17364,27787,-17364,
27480,-17847,27480,-17847,27480,-17847,27480,-17847,
27165,-18324,27165,-18324,27165,-18324,27165,-18324,
26841,-18795,26841,-18795,26841,-18795,26841,-18795,
26509,-19260,26509,-19260,26509,-19260,26509,-19260,
26168,-19720,26168,-19720,26168,-19720,26168,-19720,
25820,-20174,25820,-20174,25820,-20174,25820,-20174,
25464,-20621,25464,-20621,25464,-20621,25464,-20621,
25100,-21063,25100,-21063,25100,-21063,25100,-21063,
24729,-21498,24729,-21498,24729,-21498,24729,-21498,
24350,-21926,24350,-21926,24350,-21926,24350,-21926,
23964,-22348,23964,-22348,23964,-22348,23964,-22348,
23570,-22762,23570,-22762,23570,-22762,23570,-22762,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
22761,-23571,22761,-23571,22761,-23571,22761,-23571,
22347,-23965,22347,-23965,22347,-23965,22347,-23965,
21925,-24351,21925,-24351,21925,-24351,21925,-24351,
21497,-24730,21497,-24730,21497,-24730,21497,-24730,
21062,-25101,21062,-25101,21062,-25101,21062,-25101,
20620,-25465,20620,-25465,20620,-25465,20620,-25465,
20173,-25821,20173,-25821,20173,-25821,20173,-25821,
19719,-26169,19719,-26169,19719,-26169,19719,-26169,
19259,-26510,19259,-26510,19259,-26510,19259,-26510,
18794,-26842,18794,-26842,18794,-26842,18794,-26842,
18323,-27166,18323,-27166,18323,-27166,18323,-27166,
17846,-27481,17846,-27481,17846,-27481,17846,-27481,
17363,-27788,17363,-27788,17363,-27788,17363,-27788,
16876,-28087,16876,-28087,16876,-28087,16876,-28087,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
15885,-28659,15885,-28659,15885,-28659,15885,-28659,
15383,-28932,15383,-28932,15383,-28932,15383,-28932,
14875,-29196,14875,-29196,14875,-29196,14875,-29196,
14364,-29451,14364,-29451,14364,-29451,14364,-29451,
13847,-29697,13847,-29697,13847,-29697,13847,-29697,
13327,-29935,13327,-29935,13327,-29935,13327,-29935,
12803,-30163,12803,-30163,12803,-30163,12803,-30163,
12274,-30382,12274,-30382,12274,-30382,12274,-30382,
11742,-30591,11742,-30591,11742,-30591,11742,-30591,
11206,-30791,11206,-30791,11206,-30791,11206,-30791,
10667,-30982,10667,-30982,10667,-30982,10667,-30982,
10125,-31164,10125,-31164,10125,-31164,10125,-31164,
9580,-31336,9580,-31336,9580,-31336,9580,-31336,
9031,-31498,9031,-31498,9031,-31498,9031,-31498,
8480,-31651,8480,-31651,8480,-31651,8480,-31651,
7927,-31794,7927,-31794,7927,-31794,7927,-31794,
7370,-31928,7370,-31928,7370,-31928,7370,-31928,
6812,-32051,6812,-32051,6812,-32051,6812,-32051,
6252,-32165,6252,-32165,6252,-32165,6252,-32165,
5689,-32270,5689,-32270,5689,-32270,5689,-32270,
5125,-32364,5125,-32364,5125,-32364,5125,-32364,
4560,-32449,4560,-32449,4560,-32449,4560,-32449,
3993,-32523,3993,-32523,3993,-32523,3993,-32523,
3425,-32588,3425,-32588,3425,-32588,3425,-32588,
2855,-32643,2855,-32643,2855,-32643,2855,-32643,
2285,-32688,2285,-32688,2285,-32688,2285,-32688,
1714,-32723,1714,-32723,1714,-32723,1714,-32723,
1143,-32748,1143,-32748,1143,-32748,1143,-32748,
571,-32763,571,-32763,571,-32763,571,-32763,
0,-32767,0,-32767,0,-32767,0,-32767,
-572,-32763,-572,-32763,-572,-32763,-572,-32763,
-1144,-32748,-1144,-32748,-1144,-32748,-1144,-32748,
-1715,-32723,-1715,-32723,-1715,-32723,-1715,-32723,
-2286,-32688,-2286,-32688,-2286,-32688,-2286,-32688,
-2856,-32643,-2856,-32643,-2856,-32643,-2856,-32643,
-3426,-32588,-3426,-32588,-3426,-32588,-3426,-32588,
-3994,-32523,-3994,-32523,-3994,-32523,-3994,-32523,
-4561,-32449,-4561,-32449,-4561,-32449,-4561,-32449,
-5126,-32364,-5126,-32364,-5126,-32364,-5126,-32364,
-5690,-32270,-5690,-32270,-5690,-32270,-5690,-32270,
-6253,-32165,-6253,-32165,-6253,-32165,-6253,-32165,
-6813,-32051,-6813,-32051,-6813,-32051,-6813,-32051,
-7371,-31928,-7371,-31928,-7371,-31928,-7371,-31928,
-7928,-31794,-7928,-31794,-7928,-31794,-7928,-31794,
-8481,-31651,-8481,-31651,-8481,-31651,-8481,-31651,
-9032,-31498,-9032,-31498,-9032,-31498,-9032,-31498,
-9581,-31336,-9581,-31336,-9581,-31336,-9581,-31336,
-10126,-31164,-10126,-31164,-10126,-31164,-10126,-31164,
-10668,-30982,-10668,-30982,-10668,-30982,-10668,-30982,
-11207,-30791,-11207,-30791,-11207,-30791,-11207,-30791,
-11743,-30591,-11743,-30591,-11743,-30591,-11743,-30591,
-12275,-30382,-12275,-30382,-12275,-30382,-12275,-30382,
-12804,-30163,-12804,-30163,-12804,-30163,-12804,-30163,
-13328,-29935,-13328,-29935,-13328,-29935,-13328,-29935,
-13848,-29697,-13848,-29697,-13848,-29697,-13848,-29697,
-14365,-29451,-14365,-29451,-14365,-29451,-14365,-29451,
-14876,-29196,-14876,-29196,-14876,-29196,-14876,-29196,
-15384,-28932,-15384,-28932,-15384,-28932,-15384,-28932,
-15886,-28659,-15886,-28659,-15886,-28659,-15886,-28659};
static int16_t twb360[119*2*4] = {32747,-1144,32747,-1144,32747,-1144,32747,-1144,
32687,-2286,32687,-2286,32687,-2286,32687,-2286,
32587,-3426,32587,-3426,32587,-3426,32587,-3426,
32448,-4561,32448,-4561,32448,-4561,32448,-4561,
32269,-5690,32269,-5690,32269,-5690,32269,-5690,
32050,-6813,32050,-6813,32050,-6813,32050,-6813,
31793,-7928,31793,-7928,31793,-7928,31793,-7928,
31497,-9032,31497,-9032,31497,-9032,31497,-9032,
31163,-10126,31163,-10126,31163,-10126,31163,-10126,
30790,-11207,30790,-11207,30790,-11207,30790,-11207,
30381,-12275,30381,-12275,30381,-12275,30381,-12275,
29934,-13328,29934,-13328,29934,-13328,29934,-13328,
29450,-14365,29450,-14365,29450,-14365,29450,-14365,
28931,-15384,28931,-15384,28931,-15384,28931,-15384,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
27787,-17364,27787,-17364,27787,-17364,27787,-17364,
27165,-18324,27165,-18324,27165,-18324,27165,-18324,
26509,-19260,26509,-19260,26509,-19260,26509,-19260,
25820,-20174,25820,-20174,25820,-20174,25820,-20174,
25100,-21063,25100,-21063,25100,-21063,25100,-21063,
24350,-21926,24350,-21926,24350,-21926,24350,-21926,
23570,-22762,23570,-22762,23570,-22762,23570,-22762,
22761,-23571,22761,-23571,22761,-23571,22761,-23571,
21925,-24351,21925,-24351,21925,-24351,21925,-24351,
21062,-25101,21062,-25101,21062,-25101,21062,-25101,
20173,-25821,20173,-25821,20173,-25821,20173,-25821,
19259,-26510,19259,-26510,19259,-26510,19259,-26510,
18323,-27166,18323,-27166,18323,-27166,18323,-27166,
17363,-27788,17363,-27788,17363,-27788,17363,-27788,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
15383,-28932,15383,-28932,15383,-28932,15383,-28932,
14364,-29451,14364,-29451,14364,-29451,14364,-29451,
13327,-29935,13327,-29935,13327,-29935,13327,-29935,
12274,-30382,12274,-30382,12274,-30382,12274,-30382,
11206,-30791,11206,-30791,11206,-30791,11206,-30791,
10125,-31164,10125,-31164,10125,-31164,10125,-31164,
9031,-31498,9031,-31498,9031,-31498,9031,-31498,
7927,-31794,7927,-31794,7927,-31794,7927,-31794,
6812,-32051,6812,-32051,6812,-32051,6812,-32051,
5689,-32270,5689,-32270,5689,-32270,5689,-32270,
4560,-32449,4560,-32449,4560,-32449,4560,-32449,
3425,-32588,3425,-32588,3425,-32588,3425,-32588,
2285,-32688,2285,-32688,2285,-32688,2285,-32688,
1143,-32748,1143,-32748,1143,-32748,1143,-32748,
0,-32767,0,-32767,0,-32767,0,-32767,
-1144,-32748,-1144,-32748,-1144,-32748,-1144,-32748,
-2286,-32688,-2286,-32688,-2286,-32688,-2286,-32688,
-3426,-32588,-3426,-32588,-3426,-32588,-3426,-32588,
-4561,-32449,-4561,-32449,-4561,-32449,-4561,-32449,
-5690,-32270,-5690,-32270,-5690,-32270,-5690,-32270,
-6813,-32051,-6813,-32051,-6813,-32051,-6813,-32051,
-7928,-31794,-7928,-31794,-7928,-31794,-7928,-31794,
-9032,-31498,-9032,-31498,-9032,-31498,-9032,-31498,
-10126,-31164,-10126,-31164,-10126,-31164,-10126,-31164,
-11207,-30791,-11207,-30791,-11207,-30791,-11207,-30791,
-12275,-30382,-12275,-30382,-12275,-30382,-12275,-30382,
-13328,-29935,-13328,-29935,-13328,-29935,-13328,-29935,
-14365,-29451,-14365,-29451,-14365,-29451,-14365,-29451,
-15384,-28932,-15384,-28932,-15384,-28932,-15384,-28932,
-16384,-28378,-16384,-28378,-16384,-28378,-16384,-28378,
-17364,-27788,-17364,-27788,-17364,-27788,-17364,-27788,
-18324,-27166,-18324,-27166,-18324,-27166,-18324,-27166,
-19260,-26510,-19260,-26510,-19260,-26510,-19260,-26510,
-20174,-25821,-20174,-25821,-20174,-25821,-20174,-25821,
-21063,-25101,-21063,-25101,-21063,-25101,-21063,-25101,
-21926,-24351,-21926,-24351,-21926,-24351,-21926,-24351,
-22762,-23571,-22762,-23571,-22762,-23571,-22762,-23571,
-23571,-22762,-23571,-22762,-23571,-22762,-23571,-22762,
-24351,-21926,-24351,-21926,-24351,-21926,-24351,-21926,
-25101,-21063,-25101,-21063,-25101,-21063,-25101,-21063,
-25821,-20174,-25821,-20174,-25821,-20174,-25821,-20174,
-26510,-19260,-26510,-19260,-26510,-19260,-26510,-19260,
-27166,-18324,-27166,-18324,-27166,-18324,-27166,-18324,
-27788,-17364,-27788,-17364,-27788,-17364,-27788,-17364,
-28378,-16384,-28378,-16384,-28378,-16384,-28378,-16384,
-28932,-15384,-28932,-15384,-28932,-15384,-28932,-15384,
-29451,-14365,-29451,-14365,-29451,-14365,-29451,-14365,
-29935,-13328,-29935,-13328,-29935,-13328,-29935,-13328,
-30382,-12275,-30382,-12275,-30382,-12275,-30382,-12275,
-30791,-11207,-30791,-11207,-30791,-11207,-30791,-11207,
-31164,-10126,-31164,-10126,-31164,-10126,-31164,-10126,
-31498,-9032,-31498,-9032,-31498,-9032,-31498,-9032,
-31794,-7928,-31794,-7928,-31794,-7928,-31794,-7928,
-32051,-6813,-32051,-6813,-32051,-6813,-32051,-6813,
-32270,-5690,-32270,-5690,-32270,-5690,-32270,-5690,
-32449,-4561,-32449,-4561,-32449,-4561,-32449,-4561,
-32588,-3426,-32588,-3426,-32588,-3426,-32588,-3426,
-32688,-2286,-32688,-2286,-32688,-2286,-32688,-2286,
-32748,-1144,-32748,-1144,-32748,-1144,-32748,-1144,
-32767,-1,-32767,-1,-32767,-1,-32767,-1,
-32748,1143,-32748,1143,-32748,1143,-32748,1143,
-32688,2285,-32688,2285,-32688,2285,-32688,2285,
-32588,3425,-32588,3425,-32588,3425,-32588,3425,
-32449,4560,-32449,4560,-32449,4560,-32449,4560,
-32270,5689,-32270,5689,-32270,5689,-32270,5689,
-32051,6812,-32051,6812,-32051,6812,-32051,6812,
-31794,7927,-31794,7927,-31794,7927,-31794,7927,
-31498,9031,-31498,9031,-31498,9031,-31498,9031,
-31164,10125,-31164,10125,-31164,10125,-31164,10125,
-30791,11206,-30791,11206,-30791,11206,-30791,11206,
-30382,12274,-30382,12274,-30382,12274,-30382,12274,
-29935,13327,-29935,13327,-29935,13327,-29935,13327,
-29451,14364,-29451,14364,-29451,14364,-29451,14364,
-28932,15383,-28932,15383,-28932,15383,-28932,15383,
-28378,16383,-28378,16383,-28378,16383,-28378,16383,
-27788,17363,-27788,17363,-27788,17363,-27788,17363,
-27166,18323,-27166,18323,-27166,18323,-27166,18323,
-26510,19259,-26510,19259,-26510,19259,-26510,19259,
-25821,20173,-25821,20173,-25821,20173,-25821,20173,
-25101,21062,-25101,21062,-25101,21062,-25101,21062,
-24351,21925,-24351,21925,-24351,21925,-24351,21925,
-23571,22761,-23571,22761,-23571,22761,-23571,22761,
-22762,23570,-22762,23570,-22762,23570,-22762,23570,
-21926,24350,-21926,24350,-21926,24350,-21926,24350,
-21063,25100,-21063,25100,-21063,25100,-21063,25100,
-20174,25820,-20174,25820,-20174,25820,-20174,25820,
-19260,26509,-19260,26509,-19260,26509,-19260,26509,
-18324,27165,-18324,27165,-18324,27165,-18324,27165,
-17364,27787,-17364,27787,-17364,27787,-17364,27787};


void dft360(int16_t *x,int16_t *y,unsigned char scale_flag){ // 120 x 3
  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa360[0];
  __m128i *twb128=(__m128i *)&twb360[0];
  __m128i x2128[360];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[360];//=&ytmp128array3[0];



  for (i=0,j=0;i<120;i++,j+=3) {
    x2128[i]    = x128[j];
    x2128[i+120] = x128[j+1];
    x2128[i+240] = x128[j+2];
  }

  dft120((int16_t *)x2128,(int16_t *)ytmp128,1);
  dft120((int16_t *)(x2128+120),(int16_t *)(ytmp128+120),1);
  dft120((int16_t *)(x2128+240),(int16_t *)(ytmp128+240),1);

  bfly3_tw1(ytmp128,ytmp128+120,ytmp128+240,y128,y128+120,y128+240);
  for (i=1,j=0;i<120;i++,j++) {
    bfly3(ytmp128+i,
	  ytmp128+120+i,
	  ytmp128+240+i,
	  y128+i,
	  y128+120+i,
	  y128+240+i,
	  twa128+j,
	  twb128+j);
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(dft_norm_table[14]);
    
    for (i=0;i<360;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

};

/* Twiddles generated with
twa = floor(32767*exp(-sqrt(-1)*2*pi*(1:95)/384));
twb = floor(32767*exp(-sqrt(-1)*2*pi*2*(1:95)/384));
twc = floor(32767*exp(-sqrt(-1)*2*pi*3*(1:95)/384));
twa2 = zeros(1,2*95);
twb2 = zeros(1,2*95);
twc2 = zeros(1,2*95);
twa2(1:2:end) = real(twa);
twa2(2:2:end) = imag(twa);
twb2(1:2:end) = real(twb);
twb2(2:2:end) = imag(twb);
twc2(1:2:end) = real(twc);
twc2(2:2:end) = imag(twc);
fd=fopen("twiddle_tmp.txt","w");
fprintf(fd,"static int16_t twa384[95*2*4] = {");
for i=1:2:(2*94)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
fprintf(fd,"static int16_t twb384[95*2*4] = {");
for i=1:2:(2*94)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
fprintf(fd,"static int16_t twc384[95*2*4] = {");
for i=1:2:(2*94)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twc2(i),twc2(i+1),twc2(i),twc2(i+1),twc2(i),twc2(i+1),twc2(i),twc2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twc2(i),twc2(i+1),twc2(i),twc2(i+1),twc2(i),twc2(i+1),twc2(i),twc2(i+1));
fclose(fd);
 */

static int16_t twa384[95*2*4] = {32762,-537,32762,-537,32762,-537,32762,-537,
32749,-1073,32749,-1073,32749,-1073,32749,-1073,
32727,-1608,32727,-1608,32727,-1608,32727,-1608,
32696,-2144,32696,-2144,32696,-2144,32696,-2144,
32657,-2678,32657,-2678,32657,-2678,32657,-2678,
32609,-3212,32609,-3212,32609,-3212,32609,-3212,
32552,-3745,32552,-3745,32552,-3745,32552,-3745,
32486,-4277,32486,-4277,32486,-4277,32486,-4277,
32412,-4808,32412,-4808,32412,-4808,32412,-4808,
32329,-5338,32329,-5338,32329,-5338,32329,-5338,
32237,-5866,32237,-5866,32237,-5866,32237,-5866,
32137,-6393,32137,-6393,32137,-6393,32137,-6393,
32028,-6918,32028,-6918,32028,-6918,32028,-6918,
31911,-7441,31911,-7441,31911,-7441,31911,-7441,
31785,-7962,31785,-7962,31785,-7962,31785,-7962,
31650,-8481,31650,-8481,31650,-8481,31650,-8481,
31507,-8998,31507,-8998,31507,-8998,31507,-8998,
31356,-9512,31356,-9512,31356,-9512,31356,-9512,
31196,-10024,31196,-10024,31196,-10024,31196,-10024,
31028,-10533,31028,-10533,31028,-10533,31028,-10533,
30851,-11039,30851,-11039,30851,-11039,30851,-11039,
30666,-11543,30666,-11543,30666,-11543,30666,-11543,
30473,-12043,30473,-12043,30473,-12043,30473,-12043,
30272,-12540,30272,-12540,30272,-12540,30272,-12540,
30063,-13034,30063,-13034,30063,-13034,30063,-13034,
29846,-13524,29846,-13524,29846,-13524,29846,-13524,
29621,-14010,29621,-14010,29621,-14010,29621,-14010,
29387,-14493,29387,-14493,29387,-14493,29387,-14493,
29146,-14972,29146,-14972,29146,-14972,29146,-14972,
28897,-15447,28897,-15447,28897,-15447,28897,-15447,
28641,-15918,28641,-15918,28641,-15918,28641,-15918,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
28105,-16846,28105,-16846,28105,-16846,28105,-16846,
27825,-17304,27825,-17304,27825,-17304,27825,-17304,
27538,-17757,27538,-17757,27538,-17757,27538,-17757,
27244,-18205,27244,-18205,27244,-18205,27244,-18205,
26943,-18648,26943,-18648,26943,-18648,26943,-18648,
26634,-19087,26634,-19087,26634,-19087,26634,-19087,
26318,-19520,26318,-19520,26318,-19520,26318,-19520,
25995,-19948,25995,-19948,25995,-19948,25995,-19948,
25665,-20370,25665,-20370,25665,-20370,25665,-20370,
25329,-20788,25329,-20788,25329,-20788,25329,-20788,
24985,-21199,24985,-21199,24985,-21199,24985,-21199,
24635,-21605,24635,-21605,24635,-21605,24635,-21605,
24278,-22005,24278,-22005,24278,-22005,24278,-22005,
23915,-22400,23915,-22400,23915,-22400,23915,-22400,
23545,-22788,23545,-22788,23545,-22788,23545,-22788,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
22787,-23546,22787,-23546,22787,-23546,22787,-23546,
22399,-23916,22399,-23916,22399,-23916,22399,-23916,
22004,-24279,22004,-24279,22004,-24279,22004,-24279,
21604,-24636,21604,-24636,21604,-24636,21604,-24636,
21198,-24986,21198,-24986,21198,-24986,21198,-24986,
20787,-25330,20787,-25330,20787,-25330,20787,-25330,
20369,-25666,20369,-25666,20369,-25666,20369,-25666,
19947,-25996,19947,-25996,19947,-25996,19947,-25996,
19519,-26319,19519,-26319,19519,-26319,19519,-26319,
19086,-26635,19086,-26635,19086,-26635,19086,-26635,
18647,-26944,18647,-26944,18647,-26944,18647,-26944,
18204,-27245,18204,-27245,18204,-27245,18204,-27245,
17756,-27539,17756,-27539,17756,-27539,17756,-27539,
17303,-27826,17303,-27826,17303,-27826,17303,-27826,
16845,-28106,16845,-28106,16845,-28106,16845,-28106,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
15917,-28642,15917,-28642,15917,-28642,15917,-28642,
15446,-28898,15446,-28898,15446,-28898,15446,-28898,
14971,-29147,14971,-29147,14971,-29147,14971,-29147,
14492,-29388,14492,-29388,14492,-29388,14492,-29388,
14009,-29622,14009,-29622,14009,-29622,14009,-29622,
13523,-29847,13523,-29847,13523,-29847,13523,-29847,
13033,-30064,13033,-30064,13033,-30064,13033,-30064,
12539,-30273,12539,-30273,12539,-30273,12539,-30273,
12042,-30474,12042,-30474,12042,-30474,12042,-30474,
11542,-30667,11542,-30667,11542,-30667,11542,-30667,
11038,-30852,11038,-30852,11038,-30852,11038,-30852,
10532,-31029,10532,-31029,10532,-31029,10532,-31029,
10023,-31197,10023,-31197,10023,-31197,10023,-31197,
9511,-31357,9511,-31357,9511,-31357,9511,-31357,
8997,-31508,8997,-31508,8997,-31508,8997,-31508,
8480,-31651,8480,-31651,8480,-31651,8480,-31651,
7961,-31786,7961,-31786,7961,-31786,7961,-31786,
7440,-31912,7440,-31912,7440,-31912,7440,-31912,
6917,-32029,6917,-32029,6917,-32029,6917,-32029,
6392,-32138,6392,-32138,6392,-32138,6392,-32138,
5865,-32238,5865,-32238,5865,-32238,5865,-32238,
5337,-32330,5337,-32330,5337,-32330,5337,-32330,
4807,-32413,4807,-32413,4807,-32413,4807,-32413,
4276,-32487,4276,-32487,4276,-32487,4276,-32487,
3744,-32553,3744,-32553,3744,-32553,3744,-32553,
3211,-32610,3211,-32610,3211,-32610,3211,-32610,
2677,-32658,2677,-32658,2677,-32658,2677,-32658,
2143,-32697,2143,-32697,2143,-32697,2143,-32697,
1607,-32728,1607,-32728,1607,-32728,1607,-32728,
1072,-32750,1072,-32750,1072,-32750,1072,-32750,
536,-32763,536,-32763,536,-32763,536,-32763};
static int16_t twb384[95*2*4] = {32749,-1073,32749,-1073,32749,-1073,32749,-1073,
32696,-2144,32696,-2144,32696,-2144,32696,-2144,
32609,-3212,32609,-3212,32609,-3212,32609,-3212,
32486,-4277,32486,-4277,32486,-4277,32486,-4277,
32329,-5338,32329,-5338,32329,-5338,32329,-5338,
32137,-6393,32137,-6393,32137,-6393,32137,-6393,
31911,-7441,31911,-7441,31911,-7441,31911,-7441,
31650,-8481,31650,-8481,31650,-8481,31650,-8481,
31356,-9512,31356,-9512,31356,-9512,31356,-9512,
31028,-10533,31028,-10533,31028,-10533,31028,-10533,
30666,-11543,30666,-11543,30666,-11543,30666,-11543,
30272,-12540,30272,-12540,30272,-12540,30272,-12540,
29846,-13524,29846,-13524,29846,-13524,29846,-13524,
29387,-14493,29387,-14493,29387,-14493,29387,-14493,
28897,-15447,28897,-15447,28897,-15447,28897,-15447,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
27825,-17304,27825,-17304,27825,-17304,27825,-17304,
27244,-18205,27244,-18205,27244,-18205,27244,-18205,
26634,-19087,26634,-19087,26634,-19087,26634,-19087,
25995,-19948,25995,-19948,25995,-19948,25995,-19948,
25329,-20788,25329,-20788,25329,-20788,25329,-20788,
24635,-21605,24635,-21605,24635,-21605,24635,-21605,
23915,-22400,23915,-22400,23915,-22400,23915,-22400,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
22399,-23916,22399,-23916,22399,-23916,22399,-23916,
21604,-24636,21604,-24636,21604,-24636,21604,-24636,
20787,-25330,20787,-25330,20787,-25330,20787,-25330,
19947,-25996,19947,-25996,19947,-25996,19947,-25996,
19086,-26635,19086,-26635,19086,-26635,19086,-26635,
18204,-27245,18204,-27245,18204,-27245,18204,-27245,
17303,-27826,17303,-27826,17303,-27826,17303,-27826,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
15446,-28898,15446,-28898,15446,-28898,15446,-28898,
14492,-29388,14492,-29388,14492,-29388,14492,-29388,
13523,-29847,13523,-29847,13523,-29847,13523,-29847,
12539,-30273,12539,-30273,12539,-30273,12539,-30273,
11542,-30667,11542,-30667,11542,-30667,11542,-30667,
10532,-31029,10532,-31029,10532,-31029,10532,-31029,
9511,-31357,9511,-31357,9511,-31357,9511,-31357,
8480,-31651,8480,-31651,8480,-31651,8480,-31651,
7440,-31912,7440,-31912,7440,-31912,7440,-31912,
6392,-32138,6392,-32138,6392,-32138,6392,-32138,
5337,-32330,5337,-32330,5337,-32330,5337,-32330,
4276,-32487,4276,-32487,4276,-32487,4276,-32487,
3211,-32610,3211,-32610,3211,-32610,3211,-32610,
2143,-32697,2143,-32697,2143,-32697,2143,-32697,
1072,-32750,1072,-32750,1072,-32750,1072,-32750,
0,-32767,0,-32767,0,-32767,0,-32767,
-1073,-32750,-1073,-32750,-1073,-32750,-1073,-32750,
-2144,-32697,-2144,-32697,-2144,-32697,-2144,-32697,
-3212,-32610,-3212,-32610,-3212,-32610,-3212,-32610,
-4277,-32487,-4277,-32487,-4277,-32487,-4277,-32487,
-5338,-32330,-5338,-32330,-5338,-32330,-5338,-32330,
-6393,-32138,-6393,-32138,-6393,-32138,-6393,-32138,
-7441,-31912,-7441,-31912,-7441,-31912,-7441,-31912,
-8481,-31651,-8481,-31651,-8481,-31651,-8481,-31651,
-9512,-31357,-9512,-31357,-9512,-31357,-9512,-31357,
-10533,-31029,-10533,-31029,-10533,-31029,-10533,-31029,
-11543,-30667,-11543,-30667,-11543,-30667,-11543,-30667,
-12540,-30273,-12540,-30273,-12540,-30273,-12540,-30273,
-13524,-29847,-13524,-29847,-13524,-29847,-13524,-29847,
-14493,-29388,-14493,-29388,-14493,-29388,-14493,-29388,
-15447,-28898,-15447,-28898,-15447,-28898,-15447,-28898,
-16384,-28378,-16384,-28378,-16384,-28378,-16384,-28378,
-17304,-27826,-17304,-27826,-17304,-27826,-17304,-27826,
-18205,-27245,-18205,-27245,-18205,-27245,-18205,-27245,
-19087,-26635,-19087,-26635,-19087,-26635,-19087,-26635,
-19948,-25996,-19948,-25996,-19948,-25996,-19948,-25996,
-20788,-25330,-20788,-25330,-20788,-25330,-20788,-25330,
-21605,-24636,-21605,-24636,-21605,-24636,-21605,-24636,
-22400,-23916,-22400,-23916,-22400,-23916,-22400,-23916,
-23170,-23170,-23170,-23170,-23170,-23170,-23170,-23170,
-23916,-22400,-23916,-22400,-23916,-22400,-23916,-22400,
-24636,-21605,-24636,-21605,-24636,-21605,-24636,-21605,
-25330,-20788,-25330,-20788,-25330,-20788,-25330,-20788,
-25996,-19948,-25996,-19948,-25996,-19948,-25996,-19948,
-26635,-19087,-26635,-19087,-26635,-19087,-26635,-19087,
-27245,-18205,-27245,-18205,-27245,-18205,-27245,-18205,
-27826,-17304,-27826,-17304,-27826,-17304,-27826,-17304,
-28378,-16384,-28378,-16384,-28378,-16384,-28378,-16384,
-28898,-15447,-28898,-15447,-28898,-15447,-28898,-15447,
-29388,-14493,-29388,-14493,-29388,-14493,-29388,-14493,
-29847,-13524,-29847,-13524,-29847,-13524,-29847,-13524,
-30273,-12540,-30273,-12540,-30273,-12540,-30273,-12540,
-30667,-11543,-30667,-11543,-30667,-11543,-30667,-11543,
-31029,-10533,-31029,-10533,-31029,-10533,-31029,-10533,
-31357,-9512,-31357,-9512,-31357,-9512,-31357,-9512,
-31651,-8481,-31651,-8481,-31651,-8481,-31651,-8481,
-31912,-7441,-31912,-7441,-31912,-7441,-31912,-7441,
-32138,-6393,-32138,-6393,-32138,-6393,-32138,-6393,
-32330,-5338,-32330,-5338,-32330,-5338,-32330,-5338,
-32487,-4277,-32487,-4277,-32487,-4277,-32487,-4277,
-32610,-3212,-32610,-3212,-32610,-3212,-32610,-3212,
-32697,-2144,-32697,-2144,-32697,-2144,-32697,-2144,
-32750,-1073,-32750,-1073,-32750,-1073,-32750,-1073};
static int16_t twc384[95*2*4] = {32727,-1608,32727,-1608,32727,-1608,32727,-1608,
32609,-3212,32609,-3212,32609,-3212,32609,-3212,
32412,-4808,32412,-4808,32412,-4808,32412,-4808,
32137,-6393,32137,-6393,32137,-6393,32137,-6393,
31785,-7962,31785,-7962,31785,-7962,31785,-7962,
31356,-9512,31356,-9512,31356,-9512,31356,-9512,
30851,-11039,30851,-11039,30851,-11039,30851,-11039,
30272,-12540,30272,-12540,30272,-12540,30272,-12540,
29621,-14010,29621,-14010,29621,-14010,29621,-14010,
28897,-15447,28897,-15447,28897,-15447,28897,-15447,
28105,-16846,28105,-16846,28105,-16846,28105,-16846,
27244,-18205,27244,-18205,27244,-18205,27244,-18205,
26318,-19520,26318,-19520,26318,-19520,26318,-19520,
25329,-20788,25329,-20788,25329,-20788,25329,-20788,
24278,-22005,24278,-22005,24278,-22005,24278,-22005,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
22004,-24279,22004,-24279,22004,-24279,22004,-24279,
20787,-25330,20787,-25330,20787,-25330,20787,-25330,
19519,-26319,19519,-26319,19519,-26319,19519,-26319,
18204,-27245,18204,-27245,18204,-27245,18204,-27245,
16845,-28106,16845,-28106,16845,-28106,16845,-28106,
15446,-28898,15446,-28898,15446,-28898,15446,-28898,
14009,-29622,14009,-29622,14009,-29622,14009,-29622,
12539,-30273,12539,-30273,12539,-30273,12539,-30273,
11038,-30852,11038,-30852,11038,-30852,11038,-30852,
9511,-31357,9511,-31357,9511,-31357,9511,-31357,
7961,-31786,7961,-31786,7961,-31786,7961,-31786,
6392,-32138,6392,-32138,6392,-32138,6392,-32138,
4807,-32413,4807,-32413,4807,-32413,4807,-32413,
3211,-32610,3211,-32610,3211,-32610,3211,-32610,
1607,-32728,1607,-32728,1607,-32728,1607,-32728,
0,-32767,0,-32767,0,-32767,0,-32767,
-1608,-32728,-1608,-32728,-1608,-32728,-1608,-32728,
-3212,-32610,-3212,-32610,-3212,-32610,-3212,-32610,
-4808,-32413,-4808,-32413,-4808,-32413,-4808,-32413,
-6393,-32138,-6393,-32138,-6393,-32138,-6393,-32138,
-7962,-31786,-7962,-31786,-7962,-31786,-7962,-31786,
-9512,-31357,-9512,-31357,-9512,-31357,-9512,-31357,
-11039,-30852,-11039,-30852,-11039,-30852,-11039,-30852,
-12540,-30273,-12540,-30273,-12540,-30273,-12540,-30273,
-14010,-29622,-14010,-29622,-14010,-29622,-14010,-29622,
-15447,-28898,-15447,-28898,-15447,-28898,-15447,-28898,
-16846,-28106,-16846,-28106,-16846,-28106,-16846,-28106,
-18205,-27245,-18205,-27245,-18205,-27245,-18205,-27245,
-19520,-26319,-19520,-26319,-19520,-26319,-19520,-26319,
-20788,-25330,-20788,-25330,-20788,-25330,-20788,-25330,
-22005,-24279,-22005,-24279,-22005,-24279,-22005,-24279,
-23170,-23170,-23170,-23170,-23170,-23170,-23170,-23170,
-24279,-22005,-24279,-22005,-24279,-22005,-24279,-22005,
-25330,-20788,-25330,-20788,-25330,-20788,-25330,-20788,
-26319,-19520,-26319,-19520,-26319,-19520,-26319,-19520,
-27245,-18205,-27245,-18205,-27245,-18205,-27245,-18205,
-28106,-16846,-28106,-16846,-28106,-16846,-28106,-16846,
-28898,-15447,-28898,-15447,-28898,-15447,-28898,-15447,
-29622,-14010,-29622,-14010,-29622,-14010,-29622,-14010,
-30273,-12540,-30273,-12540,-30273,-12540,-30273,-12540,
-30852,-11039,-30852,-11039,-30852,-11039,-30852,-11039,
-31357,-9512,-31357,-9512,-31357,-9512,-31357,-9512,
-31786,-7962,-31786,-7962,-31786,-7962,-31786,-7962,
-32138,-6393,-32138,-6393,-32138,-6393,-32138,-6393,
-32413,-4808,-32413,-4808,-32413,-4808,-32413,-4808,
-32610,-3212,-32610,-3212,-32610,-3212,-32610,-3212,
-32728,-1608,-32728,-1608,-32728,-1608,-32728,-1608,
-32767,-1,-32767,-1,-32767,-1,-32767,-1,
-32728,1607,-32728,1607,-32728,1607,-32728,1607,
-32610,3211,-32610,3211,-32610,3211,-32610,3211,
-32413,4807,-32413,4807,-32413,4807,-32413,4807,
-32138,6392,-32138,6392,-32138,6392,-32138,6392,
-31786,7961,-31786,7961,-31786,7961,-31786,7961,
-31357,9511,-31357,9511,-31357,9511,-31357,9511,
-30852,11038,-30852,11038,-30852,11038,-30852,11038,
-30273,12539,-30273,12539,-30273,12539,-30273,12539,
-29622,14009,-29622,14009,-29622,14009,-29622,14009,
-28898,15446,-28898,15446,-28898,15446,-28898,15446,
-28106,16845,-28106,16845,-28106,16845,-28106,16845,
-27245,18204,-27245,18204,-27245,18204,-27245,18204,
-26319,19519,-26319,19519,-26319,19519,-26319,19519,
-25330,20787,-25330,20787,-25330,20787,-25330,20787,
-24279,22004,-24279,22004,-24279,22004,-24279,22004,
-23170,23169,-23170,23169,-23170,23169,-23170,23169,
-22005,24278,-22005,24278,-22005,24278,-22005,24278,
-20788,25329,-20788,25329,-20788,25329,-20788,25329,
-19520,26318,-19520,26318,-19520,26318,-19520,26318,
-18205,27244,-18205,27244,-18205,27244,-18205,27244,
-16846,28105,-16846,28105,-16846,28105,-16846,28105,
-15447,28897,-15447,28897,-15447,28897,-15447,28897,
-14010,29621,-14010,29621,-14010,29621,-14010,29621,
-12540,30272,-12540,30272,-12540,30272,-12540,30272,
-11039,30851,-11039,30851,-11039,30851,-11039,30851,
-9512,31356,-9512,31356,-9512,31356,-9512,31356,
-7962,31785,-7962,31785,-7962,31785,-7962,31785,
-6393,32137,-6393,32137,-6393,32137,-6393,32137,
-4808,32412,-4808,32412,-4808,32412,-4808,32412,
-3212,32609,-3212,32609,-3212,32609,-3212,32609,
-1608,32727,-1608,32727,-1608,32727,-1608,32727};

void dft384(int16_t *x,int16_t *y,unsigned char scale_flag){ // 96 x 4
  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa384[0];
  __m128i *twb128=(__m128i *)&twb384[0];
  __m128i *twc128=(__m128i *)&twc384[0];
  __m128i x2128[384];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[384];//=&ytmp128array2[0];



  for (i=0,j=0;i<96;i++,j+=4) {
    x2128[i]    = x128[j];
    x2128[i+96] = x128[j+1];
    x2128[i+192] = x128[j+2];
    x2128[i+288] = x128[j+3];
  }

  dft96((int16_t *)x2128,(int16_t *)ytmp128,1);
  dft96((int16_t *)(x2128+96),(int16_t *)(ytmp128+96),1);
  dft96((int16_t *)(x2128+192),(int16_t *)(ytmp128+192),1);
  dft96((int16_t *)(x2128+288),(int16_t *)(ytmp128+288),1);

  bfly4_tw1(ytmp128,ytmp128+96,ytmp128+192,ytmp128+288,y128,y128+96,y128+192,y128+288);
  for (i=1,j=0;i<96;i++,j++) {
    bfly4(ytmp128+i,
	  ytmp128+96+i,
	  ytmp128+192+i,
	  ytmp128+288+i,
	  y128+i,
	  y128+96+i,
	  y128+192+i,
	  y128+288+i,
	  twa128+j,
	  twb128+j,
	  twc128+j);
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(16384);//dft_norm_table[13]);
    
    for (i=0;i<384;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

};

/* Twiddles generated with
twa = floor(32767*exp(-sqrt(-1)*2*pi*(1:107)/432));
twb = floor(32767*exp(-sqrt(-1)*2*pi*2*(1:107)/432));
twc = floor(32767*exp(-sqrt(-1)*2*pi*3*(1:107)/432));
twa2 = zeros(1,2*107);
twb2 = zeros(1,2*107);
twc2 = zeros(1,2*107);
twa2(1:2:end) = real(twa);
twa2(2:2:end) = imag(twa);
twb2(1:2:end) = real(twb);
twb2(2:2:end) = imag(twb);
twc2(1:2:end) = real(twc);
twc2(2:2:end) = imag(twc);
fd=fopen("twiddle_tmp.txt","w");
fprintf(fd,"static int16_t twa432[107*2*4] = {");
for i=1:2:(2*106)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
fprintf(fd,"static int16_t twb432[107*2*4] = {");
for i=1:2:(2*106)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
fprintf(fd,"static int16_t twc432[107*2*4] = {");
for i=1:2:(2*106)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twc2(i),twc2(i+1),twc2(i),twc2(i+1),twc2(i),twc2(i+1),twc2(i),twc2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twc2(i),twc2(i+1),twc2(i),twc2(i+1),twc2(i),twc2(i+1),twc2(i),twc2(i+1));
fclose(fd);
 */

static int16_t twa432[107*2*4] = {32763,-477,32763,-477,32763,-477,32763,-477,
32753,-954,32753,-954,32753,-954,32753,-954,
32735,-1430,32735,-1430,32735,-1430,32735,-1430,
32711,-1906,32711,-1906,32711,-1906,32711,-1906,
32680,-2381,32680,-2381,32680,-2381,32680,-2381,
32642,-2856,32642,-2856,32642,-2856,32642,-2856,
32597,-3331,32597,-3331,32597,-3331,32597,-3331,
32545,-3805,32545,-3805,32545,-3805,32545,-3805,
32486,-4277,32486,-4277,32486,-4277,32486,-4277,
32421,-4749,32421,-4749,32421,-4749,32421,-4749,
32348,-5221,32348,-5221,32348,-5221,32348,-5221,
32269,-5690,32269,-5690,32269,-5690,32269,-5690,
32183,-6159,32183,-6159,32183,-6159,32183,-6159,
32090,-6627,32090,-6627,32090,-6627,32090,-6627,
31990,-7093,31990,-7093,31990,-7093,31990,-7093,
31883,-7557,31883,-7557,31883,-7557,31883,-7557,
31770,-8020,31770,-8020,31770,-8020,31770,-8020,
31650,-8481,31650,-8481,31650,-8481,31650,-8481,
31523,-8941,31523,-8941,31523,-8941,31523,-8941,
31390,-9398,31390,-9398,31390,-9398,31390,-9398,
31250,-9854,31250,-9854,31250,-9854,31250,-9854,
31103,-10307,31103,-10307,31103,-10307,31103,-10307,
30950,-10758,30950,-10758,30950,-10758,30950,-10758,
30790,-11207,30790,-11207,30790,-11207,30790,-11207,
30624,-11654,30624,-11654,30624,-11654,30624,-11654,
30451,-12098,30451,-12098,30451,-12098,30451,-12098,
30272,-12540,30272,-12540,30272,-12540,30272,-12540,
30087,-12979,30087,-12979,30087,-12979,30087,-12979,
29895,-13415,29895,-13415,29895,-13415,29895,-13415,
29696,-13848,29696,-13848,29696,-13848,29696,-13848,
29492,-14279,29492,-14279,29492,-14279,29492,-14279,
29281,-14706,29281,-14706,29281,-14706,29281,-14706,
29064,-15131,29064,-15131,29064,-15131,29064,-15131,
28841,-15552,28841,-15552,28841,-15552,28841,-15552,
28612,-15970,28612,-15970,28612,-15970,28612,-15970,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
28135,-16795,28135,-16795,28135,-16795,28135,-16795,
27888,-17202,27888,-17202,27888,-17202,27888,-17202,
27635,-17606,27635,-17606,27635,-17606,27635,-17606,
27376,-18006,27376,-18006,27376,-18006,27376,-18006,
27111,-18403,27111,-18403,27111,-18403,27111,-18403,
26841,-18795,26841,-18795,26841,-18795,26841,-18795,
26564,-19183,26564,-19183,26564,-19183,26564,-19183,
26283,-19568,26283,-19568,26283,-19568,26283,-19568,
25995,-19948,25995,-19948,25995,-19948,25995,-19948,
25702,-20324,25702,-20324,25702,-20324,25702,-20324,
25404,-20695,25404,-20695,25404,-20695,25404,-20695,
25100,-21063,25100,-21063,25100,-21063,25100,-21063,
24791,-21426,24791,-21426,24791,-21426,24791,-21426,
24477,-21784,24477,-21784,24477,-21784,24477,-21784,
24158,-22138,24158,-22138,24158,-22138,24158,-22138,
23833,-22487,23833,-22487,23833,-22487,23833,-22487,
23504,-22831,23504,-22831,23504,-22831,23504,-22831,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
22830,-23505,22830,-23505,22830,-23505,22830,-23505,
22486,-23834,22486,-23834,22486,-23834,22486,-23834,
22137,-24159,22137,-24159,22137,-24159,22137,-24159,
21783,-24478,21783,-24478,21783,-24478,21783,-24478,
21425,-24792,21425,-24792,21425,-24792,21425,-24792,
21062,-25101,21062,-25101,21062,-25101,21062,-25101,
20694,-25405,20694,-25405,20694,-25405,20694,-25405,
20323,-25703,20323,-25703,20323,-25703,20323,-25703,
19947,-25996,19947,-25996,19947,-25996,19947,-25996,
19567,-26284,19567,-26284,19567,-26284,19567,-26284,
19182,-26565,19182,-26565,19182,-26565,19182,-26565,
18794,-26842,18794,-26842,18794,-26842,18794,-26842,
18402,-27112,18402,-27112,18402,-27112,18402,-27112,
18005,-27377,18005,-27377,18005,-27377,18005,-27377,
17605,-27636,17605,-27636,17605,-27636,17605,-27636,
17201,-27889,17201,-27889,17201,-27889,17201,-27889,
16794,-28136,16794,-28136,16794,-28136,16794,-28136,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
15969,-28613,15969,-28613,15969,-28613,15969,-28613,
15551,-28842,15551,-28842,15551,-28842,15551,-28842,
15130,-29065,15130,-29065,15130,-29065,15130,-29065,
14705,-29282,14705,-29282,14705,-29282,14705,-29282,
14278,-29493,14278,-29493,14278,-29493,14278,-29493,
13847,-29697,13847,-29697,13847,-29697,13847,-29697,
13414,-29896,13414,-29896,13414,-29896,13414,-29896,
12978,-30088,12978,-30088,12978,-30088,12978,-30088,
12539,-30273,12539,-30273,12539,-30273,12539,-30273,
12097,-30452,12097,-30452,12097,-30452,12097,-30452,
11653,-30625,11653,-30625,11653,-30625,11653,-30625,
11206,-30791,11206,-30791,11206,-30791,11206,-30791,
10757,-30951,10757,-30951,10757,-30951,10757,-30951,
10306,-31104,10306,-31104,10306,-31104,10306,-31104,
9853,-31251,9853,-31251,9853,-31251,9853,-31251,
9397,-31391,9397,-31391,9397,-31391,9397,-31391,
8940,-31524,8940,-31524,8940,-31524,8940,-31524,
8480,-31651,8480,-31651,8480,-31651,8480,-31651,
8019,-31771,8019,-31771,8019,-31771,8019,-31771,
7556,-31884,7556,-31884,7556,-31884,7556,-31884,
7092,-31991,7092,-31991,7092,-31991,7092,-31991,
6626,-32091,6626,-32091,6626,-32091,6626,-32091,
6158,-32184,6158,-32184,6158,-32184,6158,-32184,
5689,-32270,5689,-32270,5689,-32270,5689,-32270,
5220,-32349,5220,-32349,5220,-32349,5220,-32349,
4748,-32422,4748,-32422,4748,-32422,4748,-32422,
4276,-32487,4276,-32487,4276,-32487,4276,-32487,
3804,-32546,3804,-32546,3804,-32546,3804,-32546,
3330,-32598,3330,-32598,3330,-32598,3330,-32598,
2855,-32643,2855,-32643,2855,-32643,2855,-32643,
2380,-32681,2380,-32681,2380,-32681,2380,-32681,
1905,-32712,1905,-32712,1905,-32712,1905,-32712,
1429,-32736,1429,-32736,1429,-32736,1429,-32736,
953,-32754,953,-32754,953,-32754,953,-32754,
476,-32764,476,-32764,476,-32764,476,-32764};
static int16_t twb432[107*2*4] = {32753,-954,32753,-954,32753,-954,32753,-954,
32711,-1906,32711,-1906,32711,-1906,32711,-1906,
32642,-2856,32642,-2856,32642,-2856,32642,-2856,
32545,-3805,32545,-3805,32545,-3805,32545,-3805,
32421,-4749,32421,-4749,32421,-4749,32421,-4749,
32269,-5690,32269,-5690,32269,-5690,32269,-5690,
32090,-6627,32090,-6627,32090,-6627,32090,-6627,
31883,-7557,31883,-7557,31883,-7557,31883,-7557,
31650,-8481,31650,-8481,31650,-8481,31650,-8481,
31390,-9398,31390,-9398,31390,-9398,31390,-9398,
31103,-10307,31103,-10307,31103,-10307,31103,-10307,
30790,-11207,30790,-11207,30790,-11207,30790,-11207,
30451,-12098,30451,-12098,30451,-12098,30451,-12098,
30087,-12979,30087,-12979,30087,-12979,30087,-12979,
29696,-13848,29696,-13848,29696,-13848,29696,-13848,
29281,-14706,29281,-14706,29281,-14706,29281,-14706,
28841,-15552,28841,-15552,28841,-15552,28841,-15552,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
27888,-17202,27888,-17202,27888,-17202,27888,-17202,
27376,-18006,27376,-18006,27376,-18006,27376,-18006,
26841,-18795,26841,-18795,26841,-18795,26841,-18795,
26283,-19568,26283,-19568,26283,-19568,26283,-19568,
25702,-20324,25702,-20324,25702,-20324,25702,-20324,
25100,-21063,25100,-21063,25100,-21063,25100,-21063,
24477,-21784,24477,-21784,24477,-21784,24477,-21784,
23833,-22487,23833,-22487,23833,-22487,23833,-22487,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
22486,-23834,22486,-23834,22486,-23834,22486,-23834,
21783,-24478,21783,-24478,21783,-24478,21783,-24478,
21062,-25101,21062,-25101,21062,-25101,21062,-25101,
20323,-25703,20323,-25703,20323,-25703,20323,-25703,
19567,-26284,19567,-26284,19567,-26284,19567,-26284,
18794,-26842,18794,-26842,18794,-26842,18794,-26842,
18005,-27377,18005,-27377,18005,-27377,18005,-27377,
17201,-27889,17201,-27889,17201,-27889,17201,-27889,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
15551,-28842,15551,-28842,15551,-28842,15551,-28842,
14705,-29282,14705,-29282,14705,-29282,14705,-29282,
13847,-29697,13847,-29697,13847,-29697,13847,-29697,
12978,-30088,12978,-30088,12978,-30088,12978,-30088,
12097,-30452,12097,-30452,12097,-30452,12097,-30452,
11206,-30791,11206,-30791,11206,-30791,11206,-30791,
10306,-31104,10306,-31104,10306,-31104,10306,-31104,
9397,-31391,9397,-31391,9397,-31391,9397,-31391,
8480,-31651,8480,-31651,8480,-31651,8480,-31651,
7556,-31884,7556,-31884,7556,-31884,7556,-31884,
6626,-32091,6626,-32091,6626,-32091,6626,-32091,
5689,-32270,5689,-32270,5689,-32270,5689,-32270,
4748,-32422,4748,-32422,4748,-32422,4748,-32422,
3804,-32546,3804,-32546,3804,-32546,3804,-32546,
2855,-32643,2855,-32643,2855,-32643,2855,-32643,
1905,-32712,1905,-32712,1905,-32712,1905,-32712,
953,-32754,953,-32754,953,-32754,953,-32754,
0,-32767,0,-32767,0,-32767,0,-32767,
-954,-32754,-954,-32754,-954,-32754,-954,-32754,
-1906,-32712,-1906,-32712,-1906,-32712,-1906,-32712,
-2856,-32643,-2856,-32643,-2856,-32643,-2856,-32643,
-3805,-32546,-3805,-32546,-3805,-32546,-3805,-32546,
-4749,-32422,-4749,-32422,-4749,-32422,-4749,-32422,
-5690,-32270,-5690,-32270,-5690,-32270,-5690,-32270,
-6627,-32091,-6627,-32091,-6627,-32091,-6627,-32091,
-7557,-31884,-7557,-31884,-7557,-31884,-7557,-31884,
-8481,-31651,-8481,-31651,-8481,-31651,-8481,-31651,
-9398,-31391,-9398,-31391,-9398,-31391,-9398,-31391,
-10307,-31104,-10307,-31104,-10307,-31104,-10307,-31104,
-11207,-30791,-11207,-30791,-11207,-30791,-11207,-30791,
-12098,-30452,-12098,-30452,-12098,-30452,-12098,-30452,
-12979,-30088,-12979,-30088,-12979,-30088,-12979,-30088,
-13848,-29697,-13848,-29697,-13848,-29697,-13848,-29697,
-14706,-29282,-14706,-29282,-14706,-29282,-14706,-29282,
-15552,-28842,-15552,-28842,-15552,-28842,-15552,-28842,
-16384,-28378,-16384,-28378,-16384,-28378,-16384,-28378,
-17202,-27889,-17202,-27889,-17202,-27889,-17202,-27889,
-18006,-27377,-18006,-27377,-18006,-27377,-18006,-27377,
-18795,-26842,-18795,-26842,-18795,-26842,-18795,-26842,
-19568,-26284,-19568,-26284,-19568,-26284,-19568,-26284,
-20324,-25703,-20324,-25703,-20324,-25703,-20324,-25703,
-21063,-25101,-21063,-25101,-21063,-25101,-21063,-25101,
-21784,-24478,-21784,-24478,-21784,-24478,-21784,-24478,
-22487,-23834,-22487,-23834,-22487,-23834,-22487,-23834,
-23170,-23170,-23170,-23170,-23170,-23170,-23170,-23170,
-23834,-22487,-23834,-22487,-23834,-22487,-23834,-22487,
-24478,-21784,-24478,-21784,-24478,-21784,-24478,-21784,
-25101,-21063,-25101,-21063,-25101,-21063,-25101,-21063,
-25703,-20324,-25703,-20324,-25703,-20324,-25703,-20324,
-26284,-19568,-26284,-19568,-26284,-19568,-26284,-19568,
-26842,-18795,-26842,-18795,-26842,-18795,-26842,-18795,
-27377,-18006,-27377,-18006,-27377,-18006,-27377,-18006,
-27889,-17202,-27889,-17202,-27889,-17202,-27889,-17202,
-28378,-16384,-28378,-16384,-28378,-16384,-28378,-16384,
-28842,-15552,-28842,-15552,-28842,-15552,-28842,-15552,
-29282,-14706,-29282,-14706,-29282,-14706,-29282,-14706,
-29697,-13848,-29697,-13848,-29697,-13848,-29697,-13848,
-30088,-12979,-30088,-12979,-30088,-12979,-30088,-12979,
-30452,-12098,-30452,-12098,-30452,-12098,-30452,-12098,
-30791,-11207,-30791,-11207,-30791,-11207,-30791,-11207,
-31104,-10307,-31104,-10307,-31104,-10307,-31104,-10307,
-31391,-9398,-31391,-9398,-31391,-9398,-31391,-9398,
-31651,-8481,-31651,-8481,-31651,-8481,-31651,-8481,
-31884,-7557,-31884,-7557,-31884,-7557,-31884,-7557,
-32091,-6627,-32091,-6627,-32091,-6627,-32091,-6627,
-32270,-5690,-32270,-5690,-32270,-5690,-32270,-5690,
-32422,-4749,-32422,-4749,-32422,-4749,-32422,-4749,
-32546,-3805,-32546,-3805,-32546,-3805,-32546,-3805,
-32643,-2856,-32643,-2856,-32643,-2856,-32643,-2856,
-32712,-1906,-32712,-1906,-32712,-1906,-32712,-1906,
-32754,-954,-32754,-954,-32754,-954,-32754,-954};
static int16_t twc432[107*2*4] = {32735,-1430,32735,-1430,32735,-1430,32735,-1430,
32642,-2856,32642,-2856,32642,-2856,32642,-2856,
32486,-4277,32486,-4277,32486,-4277,32486,-4277,
32269,-5690,32269,-5690,32269,-5690,32269,-5690,
31990,-7093,31990,-7093,31990,-7093,31990,-7093,
31650,-8481,31650,-8481,31650,-8481,31650,-8481,
31250,-9854,31250,-9854,31250,-9854,31250,-9854,
30790,-11207,30790,-11207,30790,-11207,30790,-11207,
30272,-12540,30272,-12540,30272,-12540,30272,-12540,
29696,-13848,29696,-13848,29696,-13848,29696,-13848,
29064,-15131,29064,-15131,29064,-15131,29064,-15131,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
27635,-17606,27635,-17606,27635,-17606,27635,-17606,
26841,-18795,26841,-18795,26841,-18795,26841,-18795,
25995,-19948,25995,-19948,25995,-19948,25995,-19948,
25100,-21063,25100,-21063,25100,-21063,25100,-21063,
24158,-22138,24158,-22138,24158,-22138,24158,-22138,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
22137,-24159,22137,-24159,22137,-24159,22137,-24159,
21062,-25101,21062,-25101,21062,-25101,21062,-25101,
19947,-25996,19947,-25996,19947,-25996,19947,-25996,
18794,-26842,18794,-26842,18794,-26842,18794,-26842,
17605,-27636,17605,-27636,17605,-27636,17605,-27636,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
15130,-29065,15130,-29065,15130,-29065,15130,-29065,
13847,-29697,13847,-29697,13847,-29697,13847,-29697,
12539,-30273,12539,-30273,12539,-30273,12539,-30273,
11206,-30791,11206,-30791,11206,-30791,11206,-30791,
9853,-31251,9853,-31251,9853,-31251,9853,-31251,
8480,-31651,8480,-31651,8480,-31651,8480,-31651,
7092,-31991,7092,-31991,7092,-31991,7092,-31991,
5689,-32270,5689,-32270,5689,-32270,5689,-32270,
4276,-32487,4276,-32487,4276,-32487,4276,-32487,
2855,-32643,2855,-32643,2855,-32643,2855,-32643,
1429,-32736,1429,-32736,1429,-32736,1429,-32736,
0,-32767,0,-32767,0,-32767,0,-32767,
-1430,-32736,-1430,-32736,-1430,-32736,-1430,-32736,
-2856,-32643,-2856,-32643,-2856,-32643,-2856,-32643,
-4277,-32487,-4277,-32487,-4277,-32487,-4277,-32487,
-5690,-32270,-5690,-32270,-5690,-32270,-5690,-32270,
-7093,-31991,-7093,-31991,-7093,-31991,-7093,-31991,
-8481,-31651,-8481,-31651,-8481,-31651,-8481,-31651,
-9854,-31251,-9854,-31251,-9854,-31251,-9854,-31251,
-11207,-30791,-11207,-30791,-11207,-30791,-11207,-30791,
-12540,-30273,-12540,-30273,-12540,-30273,-12540,-30273,
-13848,-29697,-13848,-29697,-13848,-29697,-13848,-29697,
-15131,-29065,-15131,-29065,-15131,-29065,-15131,-29065,
-16384,-28378,-16384,-28378,-16384,-28378,-16384,-28378,
-17606,-27636,-17606,-27636,-17606,-27636,-17606,-27636,
-18795,-26842,-18795,-26842,-18795,-26842,-18795,-26842,
-19948,-25996,-19948,-25996,-19948,-25996,-19948,-25996,
-21063,-25101,-21063,-25101,-21063,-25101,-21063,-25101,
-22138,-24159,-22138,-24159,-22138,-24159,-22138,-24159,
-23170,-23170,-23170,-23170,-23170,-23170,-23170,-23170,
-24159,-22138,-24159,-22138,-24159,-22138,-24159,-22138,
-25101,-21063,-25101,-21063,-25101,-21063,-25101,-21063,
-25996,-19948,-25996,-19948,-25996,-19948,-25996,-19948,
-26842,-18795,-26842,-18795,-26842,-18795,-26842,-18795,
-27636,-17606,-27636,-17606,-27636,-17606,-27636,-17606,
-28378,-16384,-28378,-16384,-28378,-16384,-28378,-16384,
-29065,-15131,-29065,-15131,-29065,-15131,-29065,-15131,
-29697,-13848,-29697,-13848,-29697,-13848,-29697,-13848,
-30273,-12540,-30273,-12540,-30273,-12540,-30273,-12540,
-30791,-11207,-30791,-11207,-30791,-11207,-30791,-11207,
-31251,-9854,-31251,-9854,-31251,-9854,-31251,-9854,
-31651,-8481,-31651,-8481,-31651,-8481,-31651,-8481,
-31991,-7093,-31991,-7093,-31991,-7093,-31991,-7093,
-32270,-5690,-32270,-5690,-32270,-5690,-32270,-5690,
-32487,-4277,-32487,-4277,-32487,-4277,-32487,-4277,
-32643,-2856,-32643,-2856,-32643,-2856,-32643,-2856,
-32736,-1430,-32736,-1430,-32736,-1430,-32736,-1430,
-32767,-1,-32767,-1,-32767,-1,-32767,-1,
-32736,1429,-32736,1429,-32736,1429,-32736,1429,
-32643,2855,-32643,2855,-32643,2855,-32643,2855,
-32487,4276,-32487,4276,-32487,4276,-32487,4276,
-32270,5689,-32270,5689,-32270,5689,-32270,5689,
-31991,7092,-31991,7092,-31991,7092,-31991,7092,
-31651,8480,-31651,8480,-31651,8480,-31651,8480,
-31251,9853,-31251,9853,-31251,9853,-31251,9853,
-30791,11206,-30791,11206,-30791,11206,-30791,11206,
-30273,12539,-30273,12539,-30273,12539,-30273,12539,
-29697,13847,-29697,13847,-29697,13847,-29697,13847,
-29065,15130,-29065,15130,-29065,15130,-29065,15130,
-28378,16383,-28378,16383,-28378,16383,-28378,16383,
-27636,17605,-27636,17605,-27636,17605,-27636,17605,
-26842,18794,-26842,18794,-26842,18794,-26842,18794,
-25996,19947,-25996,19947,-25996,19947,-25996,19947,
-25101,21062,-25101,21062,-25101,21062,-25101,21062,
-24159,22137,-24159,22137,-24159,22137,-24159,22137,
-23170,23169,-23170,23169,-23170,23169,-23170,23169,
-22138,24158,-22138,24158,-22138,24158,-22138,24158,
-21063,25100,-21063,25100,-21063,25100,-21063,25100,
-19948,25995,-19948,25995,-19948,25995,-19948,25995,
-18795,26841,-18795,26841,-18795,26841,-18795,26841,
-17606,27635,-17606,27635,-17606,27635,-17606,27635,
-16384,28377,-16384,28377,-16384,28377,-16384,28377,
-15131,29064,-15131,29064,-15131,29064,-15131,29064,
-13848,29696,-13848,29696,-13848,29696,-13848,29696,
-12540,30272,-12540,30272,-12540,30272,-12540,30272,
-11207,30790,-11207,30790,-11207,30790,-11207,30790,
-9854,31250,-9854,31250,-9854,31250,-9854,31250,
-8481,31650,-8481,31650,-8481,31650,-8481,31650,
-7093,31990,-7093,31990,-7093,31990,-7093,31990,
-5690,32269,-5690,32269,-5690,32269,-5690,32269,
-4277,32486,-4277,32486,-4277,32486,-4277,32486,
-2856,32642,-2856,32642,-2856,32642,-2856,32642,
-1430,32735,-1430,32735,-1430,32735,-1430,32735};


void dft432(int16_t *x,int16_t *y,unsigned char scale_flag){ // 108 x 4
  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa432[0];
  __m128i *twb128=(__m128i *)&twb432[0];
  __m128i *twc128=(__m128i *)&twc432[0];
  __m128i x2128[432];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[432];//=&ytmp128array2[0];


  for (i=0,j=0;i<108;i++,j+=4) {
    x2128[i]    = x128[j];
    x2128[i+108] = x128[j+1];
    x2128[i+216] = x128[j+2];
    x2128[i+324] = x128[j+3];
  }

  dft108((int16_t *)x2128,(int16_t *)ytmp128,1);
  dft108((int16_t *)(x2128+108),(int16_t *)(ytmp128+108),1);
  dft108((int16_t *)(x2128+216),(int16_t *)(ytmp128+216),1);
  dft108((int16_t *)(x2128+324),(int16_t *)(ytmp128+324),1);

  bfly4_tw1(ytmp128,ytmp128+108,ytmp128+216,ytmp128+324,y128,y128+108,y128+216,y128+324);
  for (i=1,j=0;i<108;i++,j++) {
    bfly4(ytmp128+i,
	  ytmp128+108+i,
	  ytmp128+216+i,
	  ytmp128+324+i,
	  y128+i,
	  y128+108+i,
	  y128+216+i,
	  y128+324+i,
	  twa128+j,
	  twb128+j,
	  twc128+j);
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(16384);//dft_norm_table[13]);
    
    for (i=0;i<432;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

};
/*
Twiddles generated with 

twa = floor(32767*exp(-sqrt(-1)*2*pi*(1:119)/480));
twb = floor(32767*exp(-sqrt(-1)*2*pi*2*(1:119)/480));
twc = floor(32767*exp(-sqrt(-1)*2*pi*3*(1:119)/480));
twa2 = zeros(1,2*119);
twb2 = zeros(1,2*119);
twc2 = zeros(1,2*119);
twa2(1:2:end) = real(twa);
twa2(2:2:end) = imag(twa);
twb2(1:2:end) = real(twb);
twb2(2:2:end) = imag(twb);
twc2(1:2:end) = real(twc);
twc2(2:2:end) = imag(twc);
fd=fopen("twiddle_tmp.txt","w");
fprintf(fd,"static int16_t twa480[119*2*4] = {");
for i=1:2:(2*118)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
fprintf(fd,"static int16_t twb480[119*2*4] = {");
for i=1:2:(2*118)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
fprintf(fd,"static int16_t twc480[119*2*4] = {");
for i=1:2:(2*118)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twc2(i),twc2(i+1),twc2(i),twc2(i+1),twc2(i),twc2(i+1),twc2(i),twc2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twc2(i),twc2(i+1),twc2(i),twc2(i+1),twc2(i),twc2(i+1),twc2(i),twc2(i+1));
fclose(fd);
*/
static int16_t twa480[119*2*4] = {32764,-429,32764,-429,32764,-429,32764,-429,
32755,-858,32755,-858,32755,-858,32755,-858,
32741,-1287,32741,-1287,32741,-1287,32741,-1287,
32722,-1715,32722,-1715,32722,-1715,32722,-1715,
32696,-2144,32696,-2144,32696,-2144,32696,-2144,
32665,-2571,32665,-2571,32665,-2571,32665,-2571,
32629,-2999,32629,-2999,32629,-2999,32629,-2999,
32587,-3426,32587,-3426,32587,-3426,32587,-3426,
32539,-3852,32539,-3852,32539,-3852,32539,-3852,
32486,-4277,32486,-4277,32486,-4277,32486,-4277,
32427,-4702,32427,-4702,32427,-4702,32427,-4702,
32363,-5126,32363,-5126,32363,-5126,32363,-5126,
32293,-5550,32293,-5550,32293,-5550,32293,-5550,
32218,-5972,32218,-5972,32218,-5972,32218,-5972,
32137,-6393,32137,-6393,32137,-6393,32137,-6393,
32050,-6813,32050,-6813,32050,-6813,32050,-6813,
31959,-7232,31959,-7232,31959,-7232,31959,-7232,
31861,-7650,31861,-7650,31861,-7650,31861,-7650,
31758,-8066,31758,-8066,31758,-8066,31758,-8066,
31650,-8481,31650,-8481,31650,-8481,31650,-8481,
31536,-8895,31536,-8895,31536,-8895,31536,-8895,
31417,-9307,31417,-9307,31417,-9307,31417,-9307,
31293,-9717,31293,-9717,31293,-9717,31293,-9717,
31163,-10126,31163,-10126,31163,-10126,31163,-10126,
31028,-10533,31028,-10533,31028,-10533,31028,-10533,
30887,-10938,30887,-10938,30887,-10938,30887,-10938,
30741,-11342,30741,-11342,30741,-11342,30741,-11342,
30590,-11743,30590,-11743,30590,-11743,30590,-11743,
30434,-12143,30434,-12143,30434,-12143,30434,-12143,
30272,-12540,30272,-12540,30272,-12540,30272,-12540,
30106,-12935,30106,-12935,30106,-12935,30106,-12935,
29934,-13328,29934,-13328,29934,-13328,29934,-13328,
29757,-13719,29757,-13719,29757,-13719,29757,-13719,
29575,-14107,29575,-14107,29575,-14107,29575,-14107,
29387,-14493,29387,-14493,29387,-14493,29387,-14493,
29195,-14876,29195,-14876,29195,-14876,29195,-14876,
28998,-15257,28998,-15257,28998,-15257,28998,-15257,
28796,-15636,28796,-15636,28796,-15636,28796,-15636,
28589,-16011,28589,-16011,28589,-16011,28589,-16011,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
28160,-16754,28160,-16754,28160,-16754,28160,-16754,
27938,-17121,27938,-17121,27938,-17121,27938,-17121,
27711,-17485,27711,-17485,27711,-17485,27711,-17485,
27480,-17847,27480,-17847,27480,-17847,27480,-17847,
27244,-18205,27244,-18205,27244,-18205,27244,-18205,
27004,-18560,27004,-18560,27004,-18560,27004,-18560,
26758,-18912,26758,-18912,26758,-18912,26758,-18912,
26509,-19260,26509,-19260,26509,-19260,26509,-19260,
26254,-19606,26254,-19606,26254,-19606,26254,-19606,
25995,-19948,25995,-19948,25995,-19948,25995,-19948,
25732,-20286,25732,-20286,25732,-20286,25732,-20286,
25464,-20621,25464,-20621,25464,-20621,25464,-20621,
25192,-20953,25192,-20953,25192,-20953,25192,-20953,
24916,-21281,24916,-21281,24916,-21281,24916,-21281,
24635,-21605,24635,-21605,24635,-21605,24635,-21605,
24350,-21926,24350,-21926,24350,-21926,24350,-21926,
24061,-22243,24061,-22243,24061,-22243,24061,-22243,
23768,-22556,23768,-22556,23768,-22556,23768,-22556,
23471,-22865,23471,-22865,23471,-22865,23471,-22865,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
22864,-23472,22864,-23472,22864,-23472,22864,-23472,
22555,-23769,22555,-23769,22555,-23769,22555,-23769,
22242,-24062,22242,-24062,22242,-24062,22242,-24062,
21925,-24351,21925,-24351,21925,-24351,21925,-24351,
21604,-24636,21604,-24636,21604,-24636,21604,-24636,
21280,-24917,21280,-24917,21280,-24917,21280,-24917,
20952,-25193,20952,-25193,20952,-25193,20952,-25193,
20620,-25465,20620,-25465,20620,-25465,20620,-25465,
20285,-25733,20285,-25733,20285,-25733,20285,-25733,
19947,-25996,19947,-25996,19947,-25996,19947,-25996,
19605,-26255,19605,-26255,19605,-26255,19605,-26255,
19259,-26510,19259,-26510,19259,-26510,19259,-26510,
18911,-26759,18911,-26759,18911,-26759,18911,-26759,
18559,-27005,18559,-27005,18559,-27005,18559,-27005,
18204,-27245,18204,-27245,18204,-27245,18204,-27245,
17846,-27481,17846,-27481,17846,-27481,17846,-27481,
17484,-27712,17484,-27712,17484,-27712,17484,-27712,
17120,-27939,17120,-27939,17120,-27939,17120,-27939,
16753,-28161,16753,-28161,16753,-28161,16753,-28161,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
16010,-28590,16010,-28590,16010,-28590,16010,-28590,
15635,-28797,15635,-28797,15635,-28797,15635,-28797,
15256,-28999,15256,-28999,15256,-28999,15256,-28999,
14875,-29196,14875,-29196,14875,-29196,14875,-29196,
14492,-29388,14492,-29388,14492,-29388,14492,-29388,
14106,-29576,14106,-29576,14106,-29576,14106,-29576,
13718,-29758,13718,-29758,13718,-29758,13718,-29758,
13327,-29935,13327,-29935,13327,-29935,13327,-29935,
12934,-30107,12934,-30107,12934,-30107,12934,-30107,
12539,-30273,12539,-30273,12539,-30273,12539,-30273,
12142,-30435,12142,-30435,12142,-30435,12142,-30435,
11742,-30591,11742,-30591,11742,-30591,11742,-30591,
11341,-30742,11341,-30742,11341,-30742,11341,-30742,
10937,-30888,10937,-30888,10937,-30888,10937,-30888,
10532,-31029,10532,-31029,10532,-31029,10532,-31029,
10125,-31164,10125,-31164,10125,-31164,10125,-31164,
9716,-31294,9716,-31294,9716,-31294,9716,-31294,
9306,-31418,9306,-31418,9306,-31418,9306,-31418,
8894,-31537,8894,-31537,8894,-31537,8894,-31537,
8480,-31651,8480,-31651,8480,-31651,8480,-31651,
8065,-31759,8065,-31759,8065,-31759,8065,-31759,
7649,-31862,7649,-31862,7649,-31862,7649,-31862,
7231,-31960,7231,-31960,7231,-31960,7231,-31960,
6812,-32051,6812,-32051,6812,-32051,6812,-32051,
6392,-32138,6392,-32138,6392,-32138,6392,-32138,
5971,-32219,5971,-32219,5971,-32219,5971,-32219,
5549,-32294,5549,-32294,5549,-32294,5549,-32294,
5125,-32364,5125,-32364,5125,-32364,5125,-32364,
4701,-32428,4701,-32428,4701,-32428,4701,-32428,
4276,-32487,4276,-32487,4276,-32487,4276,-32487,
3851,-32540,3851,-32540,3851,-32540,3851,-32540,
3425,-32588,3425,-32588,3425,-32588,3425,-32588,
2998,-32630,2998,-32630,2998,-32630,2998,-32630,
2570,-32666,2570,-32666,2570,-32666,2570,-32666,
2143,-32697,2143,-32697,2143,-32697,2143,-32697,
1714,-32723,1714,-32723,1714,-32723,1714,-32723,
1286,-32742,1286,-32742,1286,-32742,1286,-32742,
857,-32756,857,-32756,857,-32756,857,-32756,
428,-32765,428,-32765,428,-32765,428,-32765};
static int16_t twb480[119*2*4] = {32755,-858,32755,-858,32755,-858,32755,-858,
32722,-1715,32722,-1715,32722,-1715,32722,-1715,
32665,-2571,32665,-2571,32665,-2571,32665,-2571,
32587,-3426,32587,-3426,32587,-3426,32587,-3426,
32486,-4277,32486,-4277,32486,-4277,32486,-4277,
32363,-5126,32363,-5126,32363,-5126,32363,-5126,
32218,-5972,32218,-5972,32218,-5972,32218,-5972,
32050,-6813,32050,-6813,32050,-6813,32050,-6813,
31861,-7650,31861,-7650,31861,-7650,31861,-7650,
31650,-8481,31650,-8481,31650,-8481,31650,-8481,
31417,-9307,31417,-9307,31417,-9307,31417,-9307,
31163,-10126,31163,-10126,31163,-10126,31163,-10126,
30887,-10938,30887,-10938,30887,-10938,30887,-10938,
30590,-11743,30590,-11743,30590,-11743,30590,-11743,
30272,-12540,30272,-12540,30272,-12540,30272,-12540,
29934,-13328,29934,-13328,29934,-13328,29934,-13328,
29575,-14107,29575,-14107,29575,-14107,29575,-14107,
29195,-14876,29195,-14876,29195,-14876,29195,-14876,
28796,-15636,28796,-15636,28796,-15636,28796,-15636,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
27938,-17121,27938,-17121,27938,-17121,27938,-17121,
27480,-17847,27480,-17847,27480,-17847,27480,-17847,
27004,-18560,27004,-18560,27004,-18560,27004,-18560,
26509,-19260,26509,-19260,26509,-19260,26509,-19260,
25995,-19948,25995,-19948,25995,-19948,25995,-19948,
25464,-20621,25464,-20621,25464,-20621,25464,-20621,
24916,-21281,24916,-21281,24916,-21281,24916,-21281,
24350,-21926,24350,-21926,24350,-21926,24350,-21926,
23768,-22556,23768,-22556,23768,-22556,23768,-22556,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
22555,-23769,22555,-23769,22555,-23769,22555,-23769,
21925,-24351,21925,-24351,21925,-24351,21925,-24351,
21280,-24917,21280,-24917,21280,-24917,21280,-24917,
20620,-25465,20620,-25465,20620,-25465,20620,-25465,
19947,-25996,19947,-25996,19947,-25996,19947,-25996,
19259,-26510,19259,-26510,19259,-26510,19259,-26510,
18559,-27005,18559,-27005,18559,-27005,18559,-27005,
17846,-27481,17846,-27481,17846,-27481,17846,-27481,
17120,-27939,17120,-27939,17120,-27939,17120,-27939,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
15635,-28797,15635,-28797,15635,-28797,15635,-28797,
14875,-29196,14875,-29196,14875,-29196,14875,-29196,
14106,-29576,14106,-29576,14106,-29576,14106,-29576,
13327,-29935,13327,-29935,13327,-29935,13327,-29935,
12539,-30273,12539,-30273,12539,-30273,12539,-30273,
11742,-30591,11742,-30591,11742,-30591,11742,-30591,
10937,-30888,10937,-30888,10937,-30888,10937,-30888,
10125,-31164,10125,-31164,10125,-31164,10125,-31164,
9306,-31418,9306,-31418,9306,-31418,9306,-31418,
8480,-31651,8480,-31651,8480,-31651,8480,-31651,
7649,-31862,7649,-31862,7649,-31862,7649,-31862,
6812,-32051,6812,-32051,6812,-32051,6812,-32051,
5971,-32219,5971,-32219,5971,-32219,5971,-32219,
5125,-32364,5125,-32364,5125,-32364,5125,-32364,
4276,-32487,4276,-32487,4276,-32487,4276,-32487,
3425,-32588,3425,-32588,3425,-32588,3425,-32588,
2570,-32666,2570,-32666,2570,-32666,2570,-32666,
1714,-32723,1714,-32723,1714,-32723,1714,-32723,
857,-32756,857,-32756,857,-32756,857,-32756,
0,-32767,0,-32767,0,-32767,0,-32767,
-858,-32756,-858,-32756,-858,-32756,-858,-32756,
-1715,-32723,-1715,-32723,-1715,-32723,-1715,-32723,
-2571,-32666,-2571,-32666,-2571,-32666,-2571,-32666,
-3426,-32588,-3426,-32588,-3426,-32588,-3426,-32588,
-4277,-32487,-4277,-32487,-4277,-32487,-4277,-32487,
-5126,-32364,-5126,-32364,-5126,-32364,-5126,-32364,
-5972,-32219,-5972,-32219,-5972,-32219,-5972,-32219,
-6813,-32051,-6813,-32051,-6813,-32051,-6813,-32051,
-7650,-31862,-7650,-31862,-7650,-31862,-7650,-31862,
-8481,-31651,-8481,-31651,-8481,-31651,-8481,-31651,
-9307,-31418,-9307,-31418,-9307,-31418,-9307,-31418,
-10126,-31164,-10126,-31164,-10126,-31164,-10126,-31164,
-10938,-30888,-10938,-30888,-10938,-30888,-10938,-30888,
-11743,-30591,-11743,-30591,-11743,-30591,-11743,-30591,
-12540,-30273,-12540,-30273,-12540,-30273,-12540,-30273,
-13328,-29935,-13328,-29935,-13328,-29935,-13328,-29935,
-14107,-29576,-14107,-29576,-14107,-29576,-14107,-29576,
-14876,-29196,-14876,-29196,-14876,-29196,-14876,-29196,
-15636,-28797,-15636,-28797,-15636,-28797,-15636,-28797,
-16384,-28378,-16384,-28378,-16384,-28378,-16384,-28378,
-17121,-27939,-17121,-27939,-17121,-27939,-17121,-27939,
-17847,-27481,-17847,-27481,-17847,-27481,-17847,-27481,
-18560,-27005,-18560,-27005,-18560,-27005,-18560,-27005,
-19260,-26510,-19260,-26510,-19260,-26510,-19260,-26510,
-19948,-25996,-19948,-25996,-19948,-25996,-19948,-25996,
-20621,-25465,-20621,-25465,-20621,-25465,-20621,-25465,
-21281,-24917,-21281,-24917,-21281,-24917,-21281,-24917,
-21926,-24351,-21926,-24351,-21926,-24351,-21926,-24351,
-22556,-23769,-22556,-23769,-22556,-23769,-22556,-23769,
-23170,-23170,-23170,-23170,-23170,-23170,-23170,-23170,
-23769,-22556,-23769,-22556,-23769,-22556,-23769,-22556,
-24351,-21926,-24351,-21926,-24351,-21926,-24351,-21926,
-24917,-21281,-24917,-21281,-24917,-21281,-24917,-21281,
-25465,-20621,-25465,-20621,-25465,-20621,-25465,-20621,
-25996,-19948,-25996,-19948,-25996,-19948,-25996,-19948,
-26510,-19260,-26510,-19260,-26510,-19260,-26510,-19260,
-27005,-18560,-27005,-18560,-27005,-18560,-27005,-18560,
-27481,-17847,-27481,-17847,-27481,-17847,-27481,-17847,
-27939,-17121,-27939,-17121,-27939,-17121,-27939,-17121,
-28378,-16384,-28378,-16384,-28378,-16384,-28378,-16384,
-28797,-15636,-28797,-15636,-28797,-15636,-28797,-15636,
-29196,-14876,-29196,-14876,-29196,-14876,-29196,-14876,
-29576,-14107,-29576,-14107,-29576,-14107,-29576,-14107,
-29935,-13328,-29935,-13328,-29935,-13328,-29935,-13328,
-30273,-12540,-30273,-12540,-30273,-12540,-30273,-12540,
-30591,-11743,-30591,-11743,-30591,-11743,-30591,-11743,
-30888,-10938,-30888,-10938,-30888,-10938,-30888,-10938,
-31164,-10126,-31164,-10126,-31164,-10126,-31164,-10126,
-31418,-9307,-31418,-9307,-31418,-9307,-31418,-9307,
-31651,-8481,-31651,-8481,-31651,-8481,-31651,-8481,
-31862,-7650,-31862,-7650,-31862,-7650,-31862,-7650,
-32051,-6813,-32051,-6813,-32051,-6813,-32051,-6813,
-32219,-5972,-32219,-5972,-32219,-5972,-32219,-5972,
-32364,-5126,-32364,-5126,-32364,-5126,-32364,-5126,
-32487,-4277,-32487,-4277,-32487,-4277,-32487,-4277,
-32588,-3426,-32588,-3426,-32588,-3426,-32588,-3426,
-32666,-2571,-32666,-2571,-32666,-2571,-32666,-2571,
-32723,-1715,-32723,-1715,-32723,-1715,-32723,-1715,
-32756,-858,-32756,-858,-32756,-858,-32756,-858};
static int16_t twc480[119*2*4] = {32741,-1287,32741,-1287,32741,-1287,32741,-1287,
32665,-2571,32665,-2571,32665,-2571,32665,-2571,
32539,-3852,32539,-3852,32539,-3852,32539,-3852,
32363,-5126,32363,-5126,32363,-5126,32363,-5126,
32137,-6393,32137,-6393,32137,-6393,32137,-6393,
31861,-7650,31861,-7650,31861,-7650,31861,-7650,
31536,-8895,31536,-8895,31536,-8895,31536,-8895,
31163,-10126,31163,-10126,31163,-10126,31163,-10126,
30741,-11342,30741,-11342,30741,-11342,30741,-11342,
30272,-12540,30272,-12540,30272,-12540,30272,-12540,
29757,-13719,29757,-13719,29757,-13719,29757,-13719,
29195,-14876,29195,-14876,29195,-14876,29195,-14876,
28589,-16011,28589,-16011,28589,-16011,28589,-16011,
27938,-17121,27938,-17121,27938,-17121,27938,-17121,
27244,-18205,27244,-18205,27244,-18205,27244,-18205,
26509,-19260,26509,-19260,26509,-19260,26509,-19260,
25732,-20286,25732,-20286,25732,-20286,25732,-20286,
24916,-21281,24916,-21281,24916,-21281,24916,-21281,
24061,-22243,24061,-22243,24061,-22243,24061,-22243,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
22242,-24062,22242,-24062,22242,-24062,22242,-24062,
21280,-24917,21280,-24917,21280,-24917,21280,-24917,
20285,-25733,20285,-25733,20285,-25733,20285,-25733,
19259,-26510,19259,-26510,19259,-26510,19259,-26510,
18204,-27245,18204,-27245,18204,-27245,18204,-27245,
17120,-27939,17120,-27939,17120,-27939,17120,-27939,
16010,-28590,16010,-28590,16010,-28590,16010,-28590,
14875,-29196,14875,-29196,14875,-29196,14875,-29196,
13718,-29758,13718,-29758,13718,-29758,13718,-29758,
12539,-30273,12539,-30273,12539,-30273,12539,-30273,
11341,-30742,11341,-30742,11341,-30742,11341,-30742,
10125,-31164,10125,-31164,10125,-31164,10125,-31164,
8894,-31537,8894,-31537,8894,-31537,8894,-31537,
7649,-31862,7649,-31862,7649,-31862,7649,-31862,
6392,-32138,6392,-32138,6392,-32138,6392,-32138,
5125,-32364,5125,-32364,5125,-32364,5125,-32364,
3851,-32540,3851,-32540,3851,-32540,3851,-32540,
2570,-32666,2570,-32666,2570,-32666,2570,-32666,
1286,-32742,1286,-32742,1286,-32742,1286,-32742,
0,-32767,0,-32767,0,-32767,0,-32767,
-1287,-32742,-1287,-32742,-1287,-32742,-1287,-32742,
-2571,-32666,-2571,-32666,-2571,-32666,-2571,-32666,
-3852,-32540,-3852,-32540,-3852,-32540,-3852,-32540,
-5126,-32364,-5126,-32364,-5126,-32364,-5126,-32364,
-6393,-32138,-6393,-32138,-6393,-32138,-6393,-32138,
-7650,-31862,-7650,-31862,-7650,-31862,-7650,-31862,
-8895,-31537,-8895,-31537,-8895,-31537,-8895,-31537,
-10126,-31164,-10126,-31164,-10126,-31164,-10126,-31164,
-11342,-30742,-11342,-30742,-11342,-30742,-11342,-30742,
-12540,-30273,-12540,-30273,-12540,-30273,-12540,-30273,
-13719,-29758,-13719,-29758,-13719,-29758,-13719,-29758,
-14876,-29196,-14876,-29196,-14876,-29196,-14876,-29196,
-16011,-28590,-16011,-28590,-16011,-28590,-16011,-28590,
-17121,-27939,-17121,-27939,-17121,-27939,-17121,-27939,
-18205,-27245,-18205,-27245,-18205,-27245,-18205,-27245,
-19260,-26510,-19260,-26510,-19260,-26510,-19260,-26510,
-20286,-25733,-20286,-25733,-20286,-25733,-20286,-25733,
-21281,-24917,-21281,-24917,-21281,-24917,-21281,-24917,
-22243,-24062,-22243,-24062,-22243,-24062,-22243,-24062,
-23170,-23170,-23170,-23170,-23170,-23170,-23170,-23170,
-24062,-22243,-24062,-22243,-24062,-22243,-24062,-22243,
-24917,-21281,-24917,-21281,-24917,-21281,-24917,-21281,
-25733,-20286,-25733,-20286,-25733,-20286,-25733,-20286,
-26510,-19260,-26510,-19260,-26510,-19260,-26510,-19260,
-27245,-18205,-27245,-18205,-27245,-18205,-27245,-18205,
-27939,-17121,-27939,-17121,-27939,-17121,-27939,-17121,
-28590,-16011,-28590,-16011,-28590,-16011,-28590,-16011,
-29196,-14876,-29196,-14876,-29196,-14876,-29196,-14876,
-29758,-13719,-29758,-13719,-29758,-13719,-29758,-13719,
-30273,-12540,-30273,-12540,-30273,-12540,-30273,-12540,
-30742,-11342,-30742,-11342,-30742,-11342,-30742,-11342,
-31164,-10126,-31164,-10126,-31164,-10126,-31164,-10126,
-31537,-8895,-31537,-8895,-31537,-8895,-31537,-8895,
-31862,-7650,-31862,-7650,-31862,-7650,-31862,-7650,
-32138,-6393,-32138,-6393,-32138,-6393,-32138,-6393,
-32364,-5126,-32364,-5126,-32364,-5126,-32364,-5126,
-32540,-3852,-32540,-3852,-32540,-3852,-32540,-3852,
-32666,-2571,-32666,-2571,-32666,-2571,-32666,-2571,
-32742,-1287,-32742,-1287,-32742,-1287,-32742,-1287,
-32767,-1,-32767,-1,-32767,-1,-32767,-1,
-32742,1286,-32742,1286,-32742,1286,-32742,1286,
-32666,2570,-32666,2570,-32666,2570,-32666,2570,
-32540,3851,-32540,3851,-32540,3851,-32540,3851,
-32364,5125,-32364,5125,-32364,5125,-32364,5125,
-32138,6392,-32138,6392,-32138,6392,-32138,6392,
-31862,7649,-31862,7649,-31862,7649,-31862,7649,
-31537,8894,-31537,8894,-31537,8894,-31537,8894,
-31164,10125,-31164,10125,-31164,10125,-31164,10125,
-30742,11341,-30742,11341,-30742,11341,-30742,11341,
-30273,12539,-30273,12539,-30273,12539,-30273,12539,
-29758,13718,-29758,13718,-29758,13718,-29758,13718,
-29196,14875,-29196,14875,-29196,14875,-29196,14875,
-28590,16010,-28590,16010,-28590,16010,-28590,16010,
-27939,17120,-27939,17120,-27939,17120,-27939,17120,
-27245,18204,-27245,18204,-27245,18204,-27245,18204,
-26510,19259,-26510,19259,-26510,19259,-26510,19259,
-25733,20285,-25733,20285,-25733,20285,-25733,20285,
-24917,21280,-24917,21280,-24917,21280,-24917,21280,
-24062,22242,-24062,22242,-24062,22242,-24062,22242,
-23170,23169,-23170,23169,-23170,23169,-23170,23169,
-22243,24061,-22243,24061,-22243,24061,-22243,24061,
-21281,24916,-21281,24916,-21281,24916,-21281,24916,
-20286,25732,-20286,25732,-20286,25732,-20286,25732,
-19260,26509,-19260,26509,-19260,26509,-19260,26509,
-18205,27244,-18205,27244,-18205,27244,-18205,27244,
-17121,27938,-17121,27938,-17121,27938,-17121,27938,
-16011,28589,-16011,28589,-16011,28589,-16011,28589,
-14876,29195,-14876,29195,-14876,29195,-14876,29195,
-13719,29757,-13719,29757,-13719,29757,-13719,29757,
-12540,30272,-12540,30272,-12540,30272,-12540,30272,
-11342,30741,-11342,30741,-11342,30741,-11342,30741,
-10126,31163,-10126,31163,-10126,31163,-10126,31163,
-8895,31536,-8895,31536,-8895,31536,-8895,31536,
-7650,31861,-7650,31861,-7650,31861,-7650,31861,
-6393,32137,-6393,32137,-6393,32137,-6393,32137,
-5126,32363,-5126,32363,-5126,32363,-5126,32363,
-3852,32539,-3852,32539,-3852,32539,-3852,32539,
-2571,32665,-2571,32665,-2571,32665,-2571,32665,
-1287,32741,-1287,32741,-1287,32741,-1287,32741};

void dft480(int16_t *x,int16_t *y,unsigned char scale_flag){ // 120 x 4
  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa480[0];
  __m128i *twb128=(__m128i *)&twb480[0];
  __m128i *twc128=(__m128i *)&twc480[0];
  __m128i x2128[480];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[480];//=&ytmp128array2[0];



  for (i=0,j=0;i<120;i++,j+=4) {
    x2128[i]    = x128[j];
    x2128[i+120] = x128[j+1];
    x2128[i+240] = x128[j+2];
    x2128[i+360] = x128[j+3];
  }

  dft120((int16_t *)x2128,(int16_t *)ytmp128,1);
  dft120((int16_t *)(x2128+120),(int16_t *)(ytmp128+120),1);
  dft120((int16_t *)(x2128+240),(int16_t *)(ytmp128+240),1);
  dft120((int16_t *)(x2128+360),(int16_t *)(ytmp128+360),1);

  bfly4_tw1(ytmp128,ytmp128+120,ytmp128+240,ytmp128+360,y128,y128+120,y128+240,y128+360);
  for (i=1,j=0;i<120;i++,j++) {
    bfly4(ytmp128+i,
	  ytmp128+120+i,
	  ytmp128+240+i,
	  ytmp128+360+i,
	  y128+i,
	  y128+120+i,
	  y128+240+i,
	  y128+360+i,
	  twa128+j,
	  twb128+j,
	  twc128+j);
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(16384);//dft_norm_table[13]);
    
    for (i=0;i<480;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

};

/*
Twiddles generated with

twa = floor(32767*exp(-sqrt(-1)*2*pi*(1:179)/540));
twb = floor(32767*exp(-sqrt(-1)*2*pi*2*(1:179)/540));
twa2 = zeros(1,2*179);
twb2 = zeros(1,2*179);
twa2(1:2:end) = real(twa);
twa2(2:2:end) = imag(twa);
twb2(1:2:end) = real(twb);
twb2(2:2:end) = imag(twb);
fd=fopen("twiddle_tmp.txt","w");
fprintf(fd,"static int16_t twa540[179*2*4] = {");
for i=1:2:(2*178)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
fprintf(fd,"static int16_t twb540[179*2*4] = {");
for i=1:2:(2*178)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
fclose(fd);
*/
static int16_t twa540[179*2*4] = {32764,-382,32764,-382,32764,-382,32764,-382,
32758,-763,32758,-763,32758,-763,32758,-763,
32747,-1144,32747,-1144,32747,-1144,32747,-1144,
32731,-1525,32731,-1525,32731,-1525,32731,-1525,
32711,-1906,32711,-1906,32711,-1906,32711,-1906,
32687,-2286,32687,-2286,32687,-2286,32687,-2286,
32658,-2666,32658,-2666,32658,-2666,32658,-2666,
32625,-3046,32625,-3046,32625,-3046,32625,-3046,
32587,-3426,32587,-3426,32587,-3426,32587,-3426,
32545,-3805,32545,-3805,32545,-3805,32545,-3805,
32498,-4183,32498,-4183,32498,-4183,32498,-4183,
32448,-4561,32448,-4561,32448,-4561,32448,-4561,
32392,-4938,32392,-4938,32392,-4938,32392,-4938,
32333,-5315,32333,-5315,32333,-5315,32333,-5315,
32269,-5690,32269,-5690,32269,-5690,32269,-5690,
32200,-6066,32200,-6066,32200,-6066,32200,-6066,
32128,-6440,32128,-6440,32128,-6440,32128,-6440,
32050,-6813,32050,-6813,32050,-6813,32050,-6813,
31969,-7186,31969,-7186,31969,-7186,31969,-7186,
31883,-7557,31883,-7557,31883,-7557,31883,-7557,
31793,-7928,31793,-7928,31793,-7928,31793,-7928,
31699,-8297,31699,-8297,31699,-8297,31699,-8297,
31600,-8665,31600,-8665,31600,-8665,31600,-8665,
31497,-9032,31497,-9032,31497,-9032,31497,-9032,
31390,-9398,31390,-9398,31390,-9398,31390,-9398,
31278,-9763,31278,-9763,31278,-9763,31278,-9763,
31163,-10126,31163,-10126,31163,-10126,31163,-10126,
31043,-10488,31043,-10488,31043,-10488,31043,-10488,
30919,-10848,30919,-10848,30919,-10848,30919,-10848,
30790,-11207,30790,-11207,30790,-11207,30790,-11207,
30658,-11565,30658,-11565,30658,-11565,30658,-11565,
30521,-11921,30521,-11921,30521,-11921,30521,-11921,
30381,-12275,30381,-12275,30381,-12275,30381,-12275,
30236,-12628,30236,-12628,30236,-12628,30236,-12628,
30087,-12979,30087,-12979,30087,-12979,30087,-12979,
29934,-13328,29934,-13328,29934,-13328,29934,-13328,
29777,-13675,29777,-13675,29777,-13675,29777,-13675,
29615,-14021,29615,-14021,29615,-14021,29615,-14021,
29450,-14365,29450,-14365,29450,-14365,29450,-14365,
29281,-14706,29281,-14706,29281,-14706,29281,-14706,
29108,-15046,29108,-15046,29108,-15046,29108,-15046,
28931,-15384,28931,-15384,28931,-15384,28931,-15384,
28750,-15719,28750,-15719,28750,-15719,28750,-15719,
28565,-16053,28565,-16053,28565,-16053,28565,-16053,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
28184,-16713,28184,-16713,28184,-16713,28184,-16713,
27988,-17040,27988,-17040,27988,-17040,27988,-17040,
27787,-17364,27787,-17364,27787,-17364,27787,-17364,
27584,-17687,27584,-17687,27584,-17687,27584,-17687,
27376,-18006,27376,-18006,27376,-18006,27376,-18006,
27165,-18324,27165,-18324,27165,-18324,27165,-18324,
26950,-18638,26950,-18638,26950,-18638,26950,-18638,
26731,-18951,26731,-18951,26731,-18951,26731,-18951,
26509,-19260,26509,-19260,26509,-19260,26509,-19260,
26283,-19568,26283,-19568,26283,-19568,26283,-19568,
26053,-19872,26053,-19872,26053,-19872,26053,-19872,
25820,-20174,25820,-20174,25820,-20174,25820,-20174,
25584,-20473,25584,-20473,25584,-20473,25584,-20473,
25344,-20769,25344,-20769,25344,-20769,25344,-20769,
25100,-21063,25100,-21063,25100,-21063,25100,-21063,
24854,-21353,24854,-21353,24854,-21353,24854,-21353,
24604,-21641,24604,-21641,24604,-21641,24604,-21641,
24350,-21926,24350,-21926,24350,-21926,24350,-21926,
24093,-22208,24093,-22208,24093,-22208,24093,-22208,
23833,-22487,23833,-22487,23833,-22487,23833,-22487,
23570,-22762,23570,-22762,23570,-22762,23570,-22762,
23304,-23035,23304,-23035,23304,-23035,23304,-23035,
23034,-23305,23034,-23305,23034,-23305,23034,-23305,
22761,-23571,22761,-23571,22761,-23571,22761,-23571,
22486,-23834,22486,-23834,22486,-23834,22486,-23834,
22207,-24094,22207,-24094,22207,-24094,22207,-24094,
21925,-24351,21925,-24351,21925,-24351,21925,-24351,
21640,-24605,21640,-24605,21640,-24605,21640,-24605,
21352,-24855,21352,-24855,21352,-24855,21352,-24855,
21062,-25101,21062,-25101,21062,-25101,21062,-25101,
20768,-25345,20768,-25345,20768,-25345,20768,-25345,
20472,-25585,20472,-25585,20472,-25585,20472,-25585,
20173,-25821,20173,-25821,20173,-25821,20173,-25821,
19871,-26054,19871,-26054,19871,-26054,19871,-26054,
19567,-26284,19567,-26284,19567,-26284,19567,-26284,
19259,-26510,19259,-26510,19259,-26510,19259,-26510,
18950,-26732,18950,-26732,18950,-26732,18950,-26732,
18637,-26951,18637,-26951,18637,-26951,18637,-26951,
18323,-27166,18323,-27166,18323,-27166,18323,-27166,
18005,-27377,18005,-27377,18005,-27377,18005,-27377,
17686,-27585,17686,-27585,17686,-27585,17686,-27585,
17363,-27788,17363,-27788,17363,-27788,17363,-27788,
17039,-27989,17039,-27989,17039,-27989,17039,-27989,
16712,-28185,16712,-28185,16712,-28185,16712,-28185,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
16052,-28566,16052,-28566,16052,-28566,16052,-28566,
15718,-28751,15718,-28751,15718,-28751,15718,-28751,
15383,-28932,15383,-28932,15383,-28932,15383,-28932,
15045,-29109,15045,-29109,15045,-29109,15045,-29109,
14705,-29282,14705,-29282,14705,-29282,14705,-29282,
14364,-29451,14364,-29451,14364,-29451,14364,-29451,
14020,-29616,14020,-29616,14020,-29616,14020,-29616,
13674,-29778,13674,-29778,13674,-29778,13674,-29778,
13327,-29935,13327,-29935,13327,-29935,13327,-29935,
12978,-30088,12978,-30088,12978,-30088,12978,-30088,
12627,-30237,12627,-30237,12627,-30237,12627,-30237,
12274,-30382,12274,-30382,12274,-30382,12274,-30382,
11920,-30522,11920,-30522,11920,-30522,11920,-30522,
11564,-30659,11564,-30659,11564,-30659,11564,-30659,
11206,-30791,11206,-30791,11206,-30791,11206,-30791,
10847,-30920,10847,-30920,10847,-30920,10847,-30920,
10487,-31044,10487,-31044,10487,-31044,10487,-31044,
10125,-31164,10125,-31164,10125,-31164,10125,-31164,
9762,-31279,9762,-31279,9762,-31279,9762,-31279,
9397,-31391,9397,-31391,9397,-31391,9397,-31391,
9031,-31498,9031,-31498,9031,-31498,9031,-31498,
8664,-31601,8664,-31601,8664,-31601,8664,-31601,
8296,-31700,8296,-31700,8296,-31700,8296,-31700,
7927,-31794,7927,-31794,7927,-31794,7927,-31794,
7556,-31884,7556,-31884,7556,-31884,7556,-31884,
7185,-31970,7185,-31970,7185,-31970,7185,-31970,
6812,-32051,6812,-32051,6812,-32051,6812,-32051,
6439,-32129,6439,-32129,6439,-32129,6439,-32129,
6065,-32201,6065,-32201,6065,-32201,6065,-32201,
5689,-32270,5689,-32270,5689,-32270,5689,-32270,
5314,-32334,5314,-32334,5314,-32334,5314,-32334,
4937,-32393,4937,-32393,4937,-32393,4937,-32393,
4560,-32449,4560,-32449,4560,-32449,4560,-32449,
4182,-32499,4182,-32499,4182,-32499,4182,-32499,
3804,-32546,3804,-32546,3804,-32546,3804,-32546,
3425,-32588,3425,-32588,3425,-32588,3425,-32588,
3045,-32626,3045,-32626,3045,-32626,3045,-32626,
2665,-32659,2665,-32659,2665,-32659,2665,-32659,
2285,-32688,2285,-32688,2285,-32688,2285,-32688,
1905,-32712,1905,-32712,1905,-32712,1905,-32712,
1524,-32732,1524,-32732,1524,-32732,1524,-32732,
1143,-32748,1143,-32748,1143,-32748,1143,-32748,
762,-32759,762,-32759,762,-32759,762,-32759,
381,-32765,381,-32765,381,-32765,381,-32765,
0,-32767,0,-32767,0,-32767,0,-32767,
-382,-32765,-382,-32765,-382,-32765,-382,-32765,
-763,-32759,-763,-32759,-763,-32759,-763,-32759,
-1144,-32748,-1144,-32748,-1144,-32748,-1144,-32748,
-1525,-32732,-1525,-32732,-1525,-32732,-1525,-32732,
-1906,-32712,-1906,-32712,-1906,-32712,-1906,-32712,
-2286,-32688,-2286,-32688,-2286,-32688,-2286,-32688,
-2666,-32659,-2666,-32659,-2666,-32659,-2666,-32659,
-3046,-32626,-3046,-32626,-3046,-32626,-3046,-32626,
-3426,-32588,-3426,-32588,-3426,-32588,-3426,-32588,
-3805,-32546,-3805,-32546,-3805,-32546,-3805,-32546,
-4183,-32499,-4183,-32499,-4183,-32499,-4183,-32499,
-4561,-32449,-4561,-32449,-4561,-32449,-4561,-32449,
-4938,-32393,-4938,-32393,-4938,-32393,-4938,-32393,
-5315,-32334,-5315,-32334,-5315,-32334,-5315,-32334,
-5690,-32270,-5690,-32270,-5690,-32270,-5690,-32270,
-6066,-32201,-6066,-32201,-6066,-32201,-6066,-32201,
-6440,-32129,-6440,-32129,-6440,-32129,-6440,-32129,
-6813,-32051,-6813,-32051,-6813,-32051,-6813,-32051,
-7186,-31970,-7186,-31970,-7186,-31970,-7186,-31970,
-7557,-31884,-7557,-31884,-7557,-31884,-7557,-31884,
-7928,-31794,-7928,-31794,-7928,-31794,-7928,-31794,
-8297,-31700,-8297,-31700,-8297,-31700,-8297,-31700,
-8665,-31601,-8665,-31601,-8665,-31601,-8665,-31601,
-9032,-31498,-9032,-31498,-9032,-31498,-9032,-31498,
-9398,-31391,-9398,-31391,-9398,-31391,-9398,-31391,
-9763,-31279,-9763,-31279,-9763,-31279,-9763,-31279,
-10126,-31164,-10126,-31164,-10126,-31164,-10126,-31164,
-10488,-31044,-10488,-31044,-10488,-31044,-10488,-31044,
-10848,-30920,-10848,-30920,-10848,-30920,-10848,-30920,
-11207,-30791,-11207,-30791,-11207,-30791,-11207,-30791,
-11565,-30659,-11565,-30659,-11565,-30659,-11565,-30659,
-11921,-30522,-11921,-30522,-11921,-30522,-11921,-30522,
-12275,-30382,-12275,-30382,-12275,-30382,-12275,-30382,
-12628,-30237,-12628,-30237,-12628,-30237,-12628,-30237,
-12979,-30088,-12979,-30088,-12979,-30088,-12979,-30088,
-13328,-29935,-13328,-29935,-13328,-29935,-13328,-29935,
-13675,-29778,-13675,-29778,-13675,-29778,-13675,-29778,
-14021,-29616,-14021,-29616,-14021,-29616,-14021,-29616,
-14365,-29451,-14365,-29451,-14365,-29451,-14365,-29451,
-14706,-29282,-14706,-29282,-14706,-29282,-14706,-29282,
-15046,-29109,-15046,-29109,-15046,-29109,-15046,-29109,
-15384,-28932,-15384,-28932,-15384,-28932,-15384,-28932,
-15719,-28751,-15719,-28751,-15719,-28751,-15719,-28751,
-16053,-28566,-16053,-28566,-16053,-28566,-16053,-28566};
static int16_t twb540[179*2*4] = {32758,-763,32758,-763,32758,-763,32758,-763,
32731,-1525,32731,-1525,32731,-1525,32731,-1525,
32687,-2286,32687,-2286,32687,-2286,32687,-2286,
32625,-3046,32625,-3046,32625,-3046,32625,-3046,
32545,-3805,32545,-3805,32545,-3805,32545,-3805,
32448,-4561,32448,-4561,32448,-4561,32448,-4561,
32333,-5315,32333,-5315,32333,-5315,32333,-5315,
32200,-6066,32200,-6066,32200,-6066,32200,-6066,
32050,-6813,32050,-6813,32050,-6813,32050,-6813,
31883,-7557,31883,-7557,31883,-7557,31883,-7557,
31699,-8297,31699,-8297,31699,-8297,31699,-8297,
31497,-9032,31497,-9032,31497,-9032,31497,-9032,
31278,-9763,31278,-9763,31278,-9763,31278,-9763,
31043,-10488,31043,-10488,31043,-10488,31043,-10488,
30790,-11207,30790,-11207,30790,-11207,30790,-11207,
30521,-11921,30521,-11921,30521,-11921,30521,-11921,
30236,-12628,30236,-12628,30236,-12628,30236,-12628,
29934,-13328,29934,-13328,29934,-13328,29934,-13328,
29615,-14021,29615,-14021,29615,-14021,29615,-14021,
29281,-14706,29281,-14706,29281,-14706,29281,-14706,
28931,-15384,28931,-15384,28931,-15384,28931,-15384,
28565,-16053,28565,-16053,28565,-16053,28565,-16053,
28184,-16713,28184,-16713,28184,-16713,28184,-16713,
27787,-17364,27787,-17364,27787,-17364,27787,-17364,
27376,-18006,27376,-18006,27376,-18006,27376,-18006,
26950,-18638,26950,-18638,26950,-18638,26950,-18638,
26509,-19260,26509,-19260,26509,-19260,26509,-19260,
26053,-19872,26053,-19872,26053,-19872,26053,-19872,
25584,-20473,25584,-20473,25584,-20473,25584,-20473,
25100,-21063,25100,-21063,25100,-21063,25100,-21063,
24604,-21641,24604,-21641,24604,-21641,24604,-21641,
24093,-22208,24093,-22208,24093,-22208,24093,-22208,
23570,-22762,23570,-22762,23570,-22762,23570,-22762,
23034,-23305,23034,-23305,23034,-23305,23034,-23305,
22486,-23834,22486,-23834,22486,-23834,22486,-23834,
21925,-24351,21925,-24351,21925,-24351,21925,-24351,
21352,-24855,21352,-24855,21352,-24855,21352,-24855,
20768,-25345,20768,-25345,20768,-25345,20768,-25345,
20173,-25821,20173,-25821,20173,-25821,20173,-25821,
19567,-26284,19567,-26284,19567,-26284,19567,-26284,
18950,-26732,18950,-26732,18950,-26732,18950,-26732,
18323,-27166,18323,-27166,18323,-27166,18323,-27166,
17686,-27585,17686,-27585,17686,-27585,17686,-27585,
17039,-27989,17039,-27989,17039,-27989,17039,-27989,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
15718,-28751,15718,-28751,15718,-28751,15718,-28751,
15045,-29109,15045,-29109,15045,-29109,15045,-29109,
14364,-29451,14364,-29451,14364,-29451,14364,-29451,
13674,-29778,13674,-29778,13674,-29778,13674,-29778,
12978,-30088,12978,-30088,12978,-30088,12978,-30088,
12274,-30382,12274,-30382,12274,-30382,12274,-30382,
11564,-30659,11564,-30659,11564,-30659,11564,-30659,
10847,-30920,10847,-30920,10847,-30920,10847,-30920,
10125,-31164,10125,-31164,10125,-31164,10125,-31164,
9397,-31391,9397,-31391,9397,-31391,9397,-31391,
8664,-31601,8664,-31601,8664,-31601,8664,-31601,
7927,-31794,7927,-31794,7927,-31794,7927,-31794,
7185,-31970,7185,-31970,7185,-31970,7185,-31970,
6439,-32129,6439,-32129,6439,-32129,6439,-32129,
5689,-32270,5689,-32270,5689,-32270,5689,-32270,
4937,-32393,4937,-32393,4937,-32393,4937,-32393,
4182,-32499,4182,-32499,4182,-32499,4182,-32499,
3425,-32588,3425,-32588,3425,-32588,3425,-32588,
2665,-32659,2665,-32659,2665,-32659,2665,-32659,
1905,-32712,1905,-32712,1905,-32712,1905,-32712,
1143,-32748,1143,-32748,1143,-32748,1143,-32748,
381,-32765,381,-32765,381,-32765,381,-32765,
-382,-32765,-382,-32765,-382,-32765,-382,-32765,
-1144,-32748,-1144,-32748,-1144,-32748,-1144,-32748,
-1906,-32712,-1906,-32712,-1906,-32712,-1906,-32712,
-2666,-32659,-2666,-32659,-2666,-32659,-2666,-32659,
-3426,-32588,-3426,-32588,-3426,-32588,-3426,-32588,
-4183,-32499,-4183,-32499,-4183,-32499,-4183,-32499,
-4938,-32393,-4938,-32393,-4938,-32393,-4938,-32393,
-5690,-32270,-5690,-32270,-5690,-32270,-5690,-32270,
-6440,-32129,-6440,-32129,-6440,-32129,-6440,-32129,
-7186,-31970,-7186,-31970,-7186,-31970,-7186,-31970,
-7928,-31794,-7928,-31794,-7928,-31794,-7928,-31794,
-8665,-31601,-8665,-31601,-8665,-31601,-8665,-31601,
-9398,-31391,-9398,-31391,-9398,-31391,-9398,-31391,
-10126,-31164,-10126,-31164,-10126,-31164,-10126,-31164,
-10848,-30920,-10848,-30920,-10848,-30920,-10848,-30920,
-11565,-30659,-11565,-30659,-11565,-30659,-11565,-30659,
-12275,-30382,-12275,-30382,-12275,-30382,-12275,-30382,
-12979,-30088,-12979,-30088,-12979,-30088,-12979,-30088,
-13675,-29778,-13675,-29778,-13675,-29778,-13675,-29778,
-14365,-29451,-14365,-29451,-14365,-29451,-14365,-29451,
-15046,-29109,-15046,-29109,-15046,-29109,-15046,-29109,
-15719,-28751,-15719,-28751,-15719,-28751,-15719,-28751,
-16384,-28378,-16384,-28378,-16384,-28378,-16384,-28378,
-17040,-27989,-17040,-27989,-17040,-27989,-17040,-27989,
-17687,-27585,-17687,-27585,-17687,-27585,-17687,-27585,
-18324,-27166,-18324,-27166,-18324,-27166,-18324,-27166,
-18951,-26732,-18951,-26732,-18951,-26732,-18951,-26732,
-19568,-26284,-19568,-26284,-19568,-26284,-19568,-26284,
-20174,-25821,-20174,-25821,-20174,-25821,-20174,-25821,
-20769,-25345,-20769,-25345,-20769,-25345,-20769,-25345,
-21353,-24855,-21353,-24855,-21353,-24855,-21353,-24855,
-21926,-24351,-21926,-24351,-21926,-24351,-21926,-24351,
-22487,-23834,-22487,-23834,-22487,-23834,-22487,-23834,
-23035,-23305,-23035,-23305,-23035,-23305,-23035,-23305,
-23571,-22762,-23571,-22762,-23571,-22762,-23571,-22762,
-24094,-22208,-24094,-22208,-24094,-22208,-24094,-22208,
-24605,-21641,-24605,-21641,-24605,-21641,-24605,-21641,
-25101,-21063,-25101,-21063,-25101,-21063,-25101,-21063,
-25585,-20473,-25585,-20473,-25585,-20473,-25585,-20473,
-26054,-19872,-26054,-19872,-26054,-19872,-26054,-19872,
-26510,-19260,-26510,-19260,-26510,-19260,-26510,-19260,
-26951,-18638,-26951,-18638,-26951,-18638,-26951,-18638,
-27377,-18006,-27377,-18006,-27377,-18006,-27377,-18006,
-27788,-17364,-27788,-17364,-27788,-17364,-27788,-17364,
-28185,-16713,-28185,-16713,-28185,-16713,-28185,-16713,
-28566,-16053,-28566,-16053,-28566,-16053,-28566,-16053,
-28932,-15384,-28932,-15384,-28932,-15384,-28932,-15384,
-29282,-14706,-29282,-14706,-29282,-14706,-29282,-14706,
-29616,-14021,-29616,-14021,-29616,-14021,-29616,-14021,
-29935,-13328,-29935,-13328,-29935,-13328,-29935,-13328,
-30237,-12628,-30237,-12628,-30237,-12628,-30237,-12628,
-30522,-11921,-30522,-11921,-30522,-11921,-30522,-11921,
-30791,-11207,-30791,-11207,-30791,-11207,-30791,-11207,
-31044,-10488,-31044,-10488,-31044,-10488,-31044,-10488,
-31279,-9763,-31279,-9763,-31279,-9763,-31279,-9763,
-31498,-9032,-31498,-9032,-31498,-9032,-31498,-9032,
-31700,-8297,-31700,-8297,-31700,-8297,-31700,-8297,
-31884,-7557,-31884,-7557,-31884,-7557,-31884,-7557,
-32051,-6813,-32051,-6813,-32051,-6813,-32051,-6813,
-32201,-6066,-32201,-6066,-32201,-6066,-32201,-6066,
-32334,-5315,-32334,-5315,-32334,-5315,-32334,-5315,
-32449,-4561,-32449,-4561,-32449,-4561,-32449,-4561,
-32546,-3805,-32546,-3805,-32546,-3805,-32546,-3805,
-32626,-3046,-32626,-3046,-32626,-3046,-32626,-3046,
-32688,-2286,-32688,-2286,-32688,-2286,-32688,-2286,
-32732,-1525,-32732,-1525,-32732,-1525,-32732,-1525,
-32759,-763,-32759,-763,-32759,-763,-32759,-763,
-32767,-1,-32767,-1,-32767,-1,-32767,-1,
-32759,762,-32759,762,-32759,762,-32759,762,
-32732,1524,-32732,1524,-32732,1524,-32732,1524,
-32688,2285,-32688,2285,-32688,2285,-32688,2285,
-32626,3045,-32626,3045,-32626,3045,-32626,3045,
-32546,3804,-32546,3804,-32546,3804,-32546,3804,
-32449,4560,-32449,4560,-32449,4560,-32449,4560,
-32334,5314,-32334,5314,-32334,5314,-32334,5314,
-32201,6065,-32201,6065,-32201,6065,-32201,6065,
-32051,6812,-32051,6812,-32051,6812,-32051,6812,
-31884,7556,-31884,7556,-31884,7556,-31884,7556,
-31700,8296,-31700,8296,-31700,8296,-31700,8296,
-31498,9031,-31498,9031,-31498,9031,-31498,9031,
-31279,9762,-31279,9762,-31279,9762,-31279,9762,
-31044,10487,-31044,10487,-31044,10487,-31044,10487,
-30791,11206,-30791,11206,-30791,11206,-30791,11206,
-30522,11920,-30522,11920,-30522,11920,-30522,11920,
-30237,12627,-30237,12627,-30237,12627,-30237,12627,
-29935,13327,-29935,13327,-29935,13327,-29935,13327,
-29616,14020,-29616,14020,-29616,14020,-29616,14020,
-29282,14705,-29282,14705,-29282,14705,-29282,14705,
-28932,15383,-28932,15383,-28932,15383,-28932,15383,
-28566,16052,-28566,16052,-28566,16052,-28566,16052,
-28185,16712,-28185,16712,-28185,16712,-28185,16712,
-27788,17363,-27788,17363,-27788,17363,-27788,17363,
-27377,18005,-27377,18005,-27377,18005,-27377,18005,
-26951,18637,-26951,18637,-26951,18637,-26951,18637,
-26510,19259,-26510,19259,-26510,19259,-26510,19259,
-26054,19871,-26054,19871,-26054,19871,-26054,19871,
-25585,20472,-25585,20472,-25585,20472,-25585,20472,
-25101,21062,-25101,21062,-25101,21062,-25101,21062,
-24605,21640,-24605,21640,-24605,21640,-24605,21640,
-24094,22207,-24094,22207,-24094,22207,-24094,22207,
-23571,22761,-23571,22761,-23571,22761,-23571,22761,
-23035,23304,-23035,23304,-23035,23304,-23035,23304,
-22487,23833,-22487,23833,-22487,23833,-22487,23833,
-21926,24350,-21926,24350,-21926,24350,-21926,24350,
-21353,24854,-21353,24854,-21353,24854,-21353,24854,
-20769,25344,-20769,25344,-20769,25344,-20769,25344,
-20174,25820,-20174,25820,-20174,25820,-20174,25820,
-19568,26283,-19568,26283,-19568,26283,-19568,26283,
-18951,26731,-18951,26731,-18951,26731,-18951,26731,
-18324,27165,-18324,27165,-18324,27165,-18324,27165,
-17687,27584,-17687,27584,-17687,27584,-17687,27584,
-17040,27988,-17040,27988,-17040,27988,-17040,27988};

void dft540(int16_t *x,int16_t *y,unsigned char scale_flag){ // 180 x 3
  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa540[0];
  __m128i *twb128=(__m128i *)&twb540[0];
  __m128i x2128[540];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[540];//=&ytmp128array3[0];



  for (i=0,j=0;i<180;i++,j+=3) {
    x2128[i]    = x128[j];
    x2128[i+180] = x128[j+1];
    x2128[i+360] = x128[j+2];
  }

  dft180((int16_t *)x2128,(int16_t *)ytmp128,1);
  dft180((int16_t *)(x2128+180),(int16_t *)(ytmp128+180),1);
  dft180((int16_t *)(x2128+360),(int16_t *)(ytmp128+360),1);

  bfly3_tw1(ytmp128,ytmp128+180,ytmp128+360,y128,y128+180,y128+360);
  for (i=1,j=0;i<180;i++,j++) {
    bfly3(ytmp128+i,
	  ytmp128+180+i,
	  ytmp128+360+i,
	  y128+i,
	  y128+180+i,
	  y128+360+i,
	  twa128+j,
	  twb128+j);
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(dft_norm_table[14]);
    
    for (i=0;i<540;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

};

/*
Twiddles generated with

twa = floor(32767*exp(-sqrt(-1)*2*pi*(1:191)/576));
twb = floor(32767*exp(-sqrt(-1)*2*pi*2*(1:191)/576));
twa2 = zeros(1,2*191);
twb2 = zeros(1,2*191);
twa2(1:2:end) = real(twa);
twa2(2:2:end) = imag(twa);
twb2(1:2:end) = real(twb);
twb2(2:2:end) = imag(twb);
fd=fopen("twiddle_tmp.txt","w");
fprintf(fd,"static int16_t twa576[191*2*4] = {");
for i=1:2:(2*190)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
fprintf(fd,"static int16_t twb576[191*2*4] = {");
for i=1:2:(2*190)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
fclose(fd);
*/ 
static int16_t twa576[191*2*4] = {32765,-358,32765,-358,32765,-358,32765,-358,
32759,-715,32759,-715,32759,-715,32759,-715,
32749,-1073,32749,-1073,32749,-1073,32749,-1073,
32735,-1430,32735,-1430,32735,-1430,32735,-1430,
32718,-1787,32718,-1787,32718,-1787,32718,-1787,
32696,-2144,32696,-2144,32696,-2144,32696,-2144,
32671,-2500,32671,-2500,32671,-2500,32671,-2500,
32642,-2856,32642,-2856,32642,-2856,32642,-2856,
32609,-3212,32609,-3212,32609,-3212,32609,-3212,
32572,-3568,32572,-3568,32572,-3568,32572,-3568,
32531,-3923,32531,-3923,32531,-3923,32531,-3923,
32486,-4277,32486,-4277,32486,-4277,32486,-4277,
32438,-4632,32438,-4632,32438,-4632,32438,-4632,
32385,-4985,32385,-4985,32385,-4985,32385,-4985,
32329,-5338,32329,-5338,32329,-5338,32329,-5338,
32269,-5690,32269,-5690,32269,-5690,32269,-5690,
32205,-6042,32205,-6042,32205,-6042,32205,-6042,
32137,-6393,32137,-6393,32137,-6393,32137,-6393,
32065,-6743,32065,-6743,32065,-6743,32065,-6743,
31990,-7093,31990,-7093,31990,-7093,31990,-7093,
31911,-7441,31911,-7441,31911,-7441,31911,-7441,
31827,-7789,31827,-7789,31827,-7789,31827,-7789,
31741,-8135,31741,-8135,31741,-8135,31741,-8135,
31650,-8481,31650,-8481,31650,-8481,31650,-8481,
31556,-8826,31556,-8826,31556,-8826,31556,-8826,
31457,-9170,31457,-9170,31457,-9170,31457,-9170,
31356,-9512,31356,-9512,31356,-9512,31356,-9512,
31250,-9854,31250,-9854,31250,-9854,31250,-9854,
31141,-10194,31141,-10194,31141,-10194,31141,-10194,
31028,-10533,31028,-10533,31028,-10533,31028,-10533,
30911,-10871,30911,-10871,30911,-10871,30911,-10871,
30790,-11207,30790,-11207,30790,-11207,30790,-11207,
30666,-11543,30666,-11543,30666,-11543,30666,-11543,
30539,-11877,30539,-11877,30539,-11877,30539,-11877,
30407,-12209,30407,-12209,30407,-12209,30407,-12209,
30272,-12540,30272,-12540,30272,-12540,30272,-12540,
30134,-12869,30134,-12869,30134,-12869,30134,-12869,
29992,-13197,29992,-13197,29992,-13197,29992,-13197,
29846,-13524,29846,-13524,29846,-13524,29846,-13524,
29696,-13848,29696,-13848,29696,-13848,29696,-13848,
29544,-14172,29544,-14172,29544,-14172,29544,-14172,
29387,-14493,29387,-14493,29387,-14493,29387,-14493,
29227,-14813,29227,-14813,29227,-14813,29227,-14813,
29064,-15131,29064,-15131,29064,-15131,29064,-15131,
28897,-15447,28897,-15447,28897,-15447,28897,-15447,
28727,-15761,28727,-15761,28727,-15761,28727,-15761,
28554,-16073,28554,-16073,28554,-16073,28554,-16073,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
28196,-16693,28196,-16693,28196,-16693,28196,-16693,
28012,-16999,28012,-16999,28012,-16999,28012,-16999,
27825,-17304,27825,-17304,27825,-17304,27825,-17304,
27635,-17606,27635,-17606,27635,-17606,27635,-17606,
27441,-17907,27441,-17907,27441,-17907,27441,-17907,
27244,-18205,27244,-18205,27244,-18205,27244,-18205,
27044,-18501,27044,-18501,27044,-18501,27044,-18501,
26841,-18795,26841,-18795,26841,-18795,26841,-18795,
26634,-19087,26634,-19087,26634,-19087,26634,-19087,
26424,-19376,26424,-19376,26424,-19376,26424,-19376,
26211,-19663,26211,-19663,26211,-19663,26211,-19663,
25995,-19948,25995,-19948,25995,-19948,25995,-19948,
25776,-20230,25776,-20230,25776,-20230,25776,-20230,
25554,-20510,25554,-20510,25554,-20510,25554,-20510,
25329,-20788,25329,-20788,25329,-20788,25329,-20788,
25100,-21063,25100,-21063,25100,-21063,25100,-21063,
24869,-21335,24869,-21335,24869,-21335,24869,-21335,
24635,-21605,24635,-21605,24635,-21605,24635,-21605,
24398,-21873,24398,-21873,24398,-21873,24398,-21873,
24158,-22138,24158,-22138,24158,-22138,24158,-22138,
23915,-22400,23915,-22400,23915,-22400,23915,-22400,
23669,-22659,23669,-22659,23669,-22659,23669,-22659,
23421,-22916,23421,-22916,23421,-22916,23421,-22916,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
22915,-23422,22915,-23422,22915,-23422,22915,-23422,
22658,-23670,22658,-23670,22658,-23670,22658,-23670,
22399,-23916,22399,-23916,22399,-23916,22399,-23916,
22137,-24159,22137,-24159,22137,-24159,22137,-24159,
21872,-24399,21872,-24399,21872,-24399,21872,-24399,
21604,-24636,21604,-24636,21604,-24636,21604,-24636,
21334,-24870,21334,-24870,21334,-24870,21334,-24870,
21062,-25101,21062,-25101,21062,-25101,21062,-25101,
20787,-25330,20787,-25330,20787,-25330,20787,-25330,
20509,-25555,20509,-25555,20509,-25555,20509,-25555,
20229,-25777,20229,-25777,20229,-25777,20229,-25777,
19947,-25996,19947,-25996,19947,-25996,19947,-25996,
19662,-26212,19662,-26212,19662,-26212,19662,-26212,
19375,-26425,19375,-26425,19375,-26425,19375,-26425,
19086,-26635,19086,-26635,19086,-26635,19086,-26635,
18794,-26842,18794,-26842,18794,-26842,18794,-26842,
18500,-27045,18500,-27045,18500,-27045,18500,-27045,
18204,-27245,18204,-27245,18204,-27245,18204,-27245,
17906,-27442,17906,-27442,17906,-27442,17906,-27442,
17605,-27636,17605,-27636,17605,-27636,17605,-27636,
17303,-27826,17303,-27826,17303,-27826,17303,-27826,
16998,-28013,16998,-28013,16998,-28013,16998,-28013,
16692,-28197,16692,-28197,16692,-28197,16692,-28197,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
16072,-28555,16072,-28555,16072,-28555,16072,-28555,
15760,-28728,15760,-28728,15760,-28728,15760,-28728,
15446,-28898,15446,-28898,15446,-28898,15446,-28898,
15130,-29065,15130,-29065,15130,-29065,15130,-29065,
14812,-29228,14812,-29228,14812,-29228,14812,-29228,
14492,-29388,14492,-29388,14492,-29388,14492,-29388,
14171,-29545,14171,-29545,14171,-29545,14171,-29545,
13847,-29697,13847,-29697,13847,-29697,13847,-29697,
13523,-29847,13523,-29847,13523,-29847,13523,-29847,
13196,-29993,13196,-29993,13196,-29993,13196,-29993,
12868,-30135,12868,-30135,12868,-30135,12868,-30135,
12539,-30273,12539,-30273,12539,-30273,12539,-30273,
12208,-30408,12208,-30408,12208,-30408,12208,-30408,
11876,-30540,11876,-30540,11876,-30540,11876,-30540,
11542,-30667,11542,-30667,11542,-30667,11542,-30667,
11206,-30791,11206,-30791,11206,-30791,11206,-30791,
10870,-30912,10870,-30912,10870,-30912,10870,-30912,
10532,-31029,10532,-31029,10532,-31029,10532,-31029,
10193,-31142,10193,-31142,10193,-31142,10193,-31142,
9853,-31251,9853,-31251,9853,-31251,9853,-31251,
9511,-31357,9511,-31357,9511,-31357,9511,-31357,
9169,-31458,9169,-31458,9169,-31458,9169,-31458,
8825,-31557,8825,-31557,8825,-31557,8825,-31557,
8480,-31651,8480,-31651,8480,-31651,8480,-31651,
8134,-31742,8134,-31742,8134,-31742,8134,-31742,
7788,-31828,7788,-31828,7788,-31828,7788,-31828,
7440,-31912,7440,-31912,7440,-31912,7440,-31912,
7092,-31991,7092,-31991,7092,-31991,7092,-31991,
6742,-32066,6742,-32066,6742,-32066,6742,-32066,
6392,-32138,6392,-32138,6392,-32138,6392,-32138,
6041,-32206,6041,-32206,6041,-32206,6041,-32206,
5689,-32270,5689,-32270,5689,-32270,5689,-32270,
5337,-32330,5337,-32330,5337,-32330,5337,-32330,
4984,-32386,4984,-32386,4984,-32386,4984,-32386,
4631,-32439,4631,-32439,4631,-32439,4631,-32439,
4276,-32487,4276,-32487,4276,-32487,4276,-32487,
3922,-32532,3922,-32532,3922,-32532,3922,-32532,
3567,-32573,3567,-32573,3567,-32573,3567,-32573,
3211,-32610,3211,-32610,3211,-32610,3211,-32610,
2855,-32643,2855,-32643,2855,-32643,2855,-32643,
2499,-32672,2499,-32672,2499,-32672,2499,-32672,
2143,-32697,2143,-32697,2143,-32697,2143,-32697,
1786,-32719,1786,-32719,1786,-32719,1786,-32719,
1429,-32736,1429,-32736,1429,-32736,1429,-32736,
1072,-32750,1072,-32750,1072,-32750,1072,-32750,
714,-32760,714,-32760,714,-32760,714,-32760,
357,-32766,357,-32766,357,-32766,357,-32766,
0,-32767,0,-32767,0,-32767,0,-32767,
-358,-32766,-358,-32766,-358,-32766,-358,-32766,
-715,-32760,-715,-32760,-715,-32760,-715,-32760,
-1073,-32750,-1073,-32750,-1073,-32750,-1073,-32750,
-1430,-32736,-1430,-32736,-1430,-32736,-1430,-32736,
-1787,-32719,-1787,-32719,-1787,-32719,-1787,-32719,
-2144,-32697,-2144,-32697,-2144,-32697,-2144,-32697,
-2500,-32672,-2500,-32672,-2500,-32672,-2500,-32672,
-2856,-32643,-2856,-32643,-2856,-32643,-2856,-32643,
-3212,-32610,-3212,-32610,-3212,-32610,-3212,-32610,
-3568,-32573,-3568,-32573,-3568,-32573,-3568,-32573,
-3923,-32532,-3923,-32532,-3923,-32532,-3923,-32532,
-4277,-32487,-4277,-32487,-4277,-32487,-4277,-32487,
-4632,-32439,-4632,-32439,-4632,-32439,-4632,-32439,
-4985,-32386,-4985,-32386,-4985,-32386,-4985,-32386,
-5338,-32330,-5338,-32330,-5338,-32330,-5338,-32330,
-5690,-32270,-5690,-32270,-5690,-32270,-5690,-32270,
-6042,-32206,-6042,-32206,-6042,-32206,-6042,-32206,
-6393,-32138,-6393,-32138,-6393,-32138,-6393,-32138,
-6743,-32066,-6743,-32066,-6743,-32066,-6743,-32066,
-7093,-31991,-7093,-31991,-7093,-31991,-7093,-31991,
-7441,-31912,-7441,-31912,-7441,-31912,-7441,-31912,
-7789,-31828,-7789,-31828,-7789,-31828,-7789,-31828,
-8135,-31742,-8135,-31742,-8135,-31742,-8135,-31742,
-8481,-31651,-8481,-31651,-8481,-31651,-8481,-31651,
-8826,-31557,-8826,-31557,-8826,-31557,-8826,-31557,
-9170,-31458,-9170,-31458,-9170,-31458,-9170,-31458,
-9512,-31357,-9512,-31357,-9512,-31357,-9512,-31357,
-9854,-31251,-9854,-31251,-9854,-31251,-9854,-31251,
-10194,-31142,-10194,-31142,-10194,-31142,-10194,-31142,
-10533,-31029,-10533,-31029,-10533,-31029,-10533,-31029,
-10871,-30912,-10871,-30912,-10871,-30912,-10871,-30912,
-11207,-30791,-11207,-30791,-11207,-30791,-11207,-30791,
-11543,-30667,-11543,-30667,-11543,-30667,-11543,-30667,
-11877,-30540,-11877,-30540,-11877,-30540,-11877,-30540,
-12209,-30408,-12209,-30408,-12209,-30408,-12209,-30408,
-12540,-30273,-12540,-30273,-12540,-30273,-12540,-30273,
-12869,-30135,-12869,-30135,-12869,-30135,-12869,-30135,
-13197,-29993,-13197,-29993,-13197,-29993,-13197,-29993,
-13524,-29847,-13524,-29847,-13524,-29847,-13524,-29847,
-13848,-29697,-13848,-29697,-13848,-29697,-13848,-29697,
-14172,-29545,-14172,-29545,-14172,-29545,-14172,-29545,
-14493,-29388,-14493,-29388,-14493,-29388,-14493,-29388,
-14813,-29228,-14813,-29228,-14813,-29228,-14813,-29228,
-15131,-29065,-15131,-29065,-15131,-29065,-15131,-29065,
-15447,-28898,-15447,-28898,-15447,-28898,-15447,-28898,
-15761,-28728,-15761,-28728,-15761,-28728,-15761,-28728,
-16073,-28555,-16073,-28555,-16073,-28555,-16073,-28555};
static int16_t twb576[191*2*4] = {32759,-715,32759,-715,32759,-715,32759,-715,
32735,-1430,32735,-1430,32735,-1430,32735,-1430,
32696,-2144,32696,-2144,32696,-2144,32696,-2144,
32642,-2856,32642,-2856,32642,-2856,32642,-2856,
32572,-3568,32572,-3568,32572,-3568,32572,-3568,
32486,-4277,32486,-4277,32486,-4277,32486,-4277,
32385,-4985,32385,-4985,32385,-4985,32385,-4985,
32269,-5690,32269,-5690,32269,-5690,32269,-5690,
32137,-6393,32137,-6393,32137,-6393,32137,-6393,
31990,-7093,31990,-7093,31990,-7093,31990,-7093,
31827,-7789,31827,-7789,31827,-7789,31827,-7789,
31650,-8481,31650,-8481,31650,-8481,31650,-8481,
31457,-9170,31457,-9170,31457,-9170,31457,-9170,
31250,-9854,31250,-9854,31250,-9854,31250,-9854,
31028,-10533,31028,-10533,31028,-10533,31028,-10533,
30790,-11207,30790,-11207,30790,-11207,30790,-11207,
30539,-11877,30539,-11877,30539,-11877,30539,-11877,
30272,-12540,30272,-12540,30272,-12540,30272,-12540,
29992,-13197,29992,-13197,29992,-13197,29992,-13197,
29696,-13848,29696,-13848,29696,-13848,29696,-13848,
29387,-14493,29387,-14493,29387,-14493,29387,-14493,
29064,-15131,29064,-15131,29064,-15131,29064,-15131,
28727,-15761,28727,-15761,28727,-15761,28727,-15761,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
28012,-16999,28012,-16999,28012,-16999,28012,-16999,
27635,-17606,27635,-17606,27635,-17606,27635,-17606,
27244,-18205,27244,-18205,27244,-18205,27244,-18205,
26841,-18795,26841,-18795,26841,-18795,26841,-18795,
26424,-19376,26424,-19376,26424,-19376,26424,-19376,
25995,-19948,25995,-19948,25995,-19948,25995,-19948,
25554,-20510,25554,-20510,25554,-20510,25554,-20510,
25100,-21063,25100,-21063,25100,-21063,25100,-21063,
24635,-21605,24635,-21605,24635,-21605,24635,-21605,
24158,-22138,24158,-22138,24158,-22138,24158,-22138,
23669,-22659,23669,-22659,23669,-22659,23669,-22659,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
22658,-23670,22658,-23670,22658,-23670,22658,-23670,
22137,-24159,22137,-24159,22137,-24159,22137,-24159,
21604,-24636,21604,-24636,21604,-24636,21604,-24636,
21062,-25101,21062,-25101,21062,-25101,21062,-25101,
20509,-25555,20509,-25555,20509,-25555,20509,-25555,
19947,-25996,19947,-25996,19947,-25996,19947,-25996,
19375,-26425,19375,-26425,19375,-26425,19375,-26425,
18794,-26842,18794,-26842,18794,-26842,18794,-26842,
18204,-27245,18204,-27245,18204,-27245,18204,-27245,
17605,-27636,17605,-27636,17605,-27636,17605,-27636,
16998,-28013,16998,-28013,16998,-28013,16998,-28013,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
15760,-28728,15760,-28728,15760,-28728,15760,-28728,
15130,-29065,15130,-29065,15130,-29065,15130,-29065,
14492,-29388,14492,-29388,14492,-29388,14492,-29388,
13847,-29697,13847,-29697,13847,-29697,13847,-29697,
13196,-29993,13196,-29993,13196,-29993,13196,-29993,
12539,-30273,12539,-30273,12539,-30273,12539,-30273,
11876,-30540,11876,-30540,11876,-30540,11876,-30540,
11206,-30791,11206,-30791,11206,-30791,11206,-30791,
10532,-31029,10532,-31029,10532,-31029,10532,-31029,
9853,-31251,9853,-31251,9853,-31251,9853,-31251,
9169,-31458,9169,-31458,9169,-31458,9169,-31458,
8480,-31651,8480,-31651,8480,-31651,8480,-31651,
7788,-31828,7788,-31828,7788,-31828,7788,-31828,
7092,-31991,7092,-31991,7092,-31991,7092,-31991,
6392,-32138,6392,-32138,6392,-32138,6392,-32138,
5689,-32270,5689,-32270,5689,-32270,5689,-32270,
4984,-32386,4984,-32386,4984,-32386,4984,-32386,
4276,-32487,4276,-32487,4276,-32487,4276,-32487,
3567,-32573,3567,-32573,3567,-32573,3567,-32573,
2855,-32643,2855,-32643,2855,-32643,2855,-32643,
2143,-32697,2143,-32697,2143,-32697,2143,-32697,
1429,-32736,1429,-32736,1429,-32736,1429,-32736,
714,-32760,714,-32760,714,-32760,714,-32760,
0,-32767,0,-32767,0,-32767,0,-32767,
-715,-32760,-715,-32760,-715,-32760,-715,-32760,
-1430,-32736,-1430,-32736,-1430,-32736,-1430,-32736,
-2144,-32697,-2144,-32697,-2144,-32697,-2144,-32697,
-2856,-32643,-2856,-32643,-2856,-32643,-2856,-32643,
-3568,-32573,-3568,-32573,-3568,-32573,-3568,-32573,
-4277,-32487,-4277,-32487,-4277,-32487,-4277,-32487,
-4985,-32386,-4985,-32386,-4985,-32386,-4985,-32386,
-5690,-32270,-5690,-32270,-5690,-32270,-5690,-32270,
-6393,-32138,-6393,-32138,-6393,-32138,-6393,-32138,
-7093,-31991,-7093,-31991,-7093,-31991,-7093,-31991,
-7789,-31828,-7789,-31828,-7789,-31828,-7789,-31828,
-8481,-31651,-8481,-31651,-8481,-31651,-8481,-31651,
-9170,-31458,-9170,-31458,-9170,-31458,-9170,-31458,
-9854,-31251,-9854,-31251,-9854,-31251,-9854,-31251,
-10533,-31029,-10533,-31029,-10533,-31029,-10533,-31029,
-11207,-30791,-11207,-30791,-11207,-30791,-11207,-30791,
-11877,-30540,-11877,-30540,-11877,-30540,-11877,-30540,
-12540,-30273,-12540,-30273,-12540,-30273,-12540,-30273,
-13197,-29993,-13197,-29993,-13197,-29993,-13197,-29993,
-13848,-29697,-13848,-29697,-13848,-29697,-13848,-29697,
-14493,-29388,-14493,-29388,-14493,-29388,-14493,-29388,
-15131,-29065,-15131,-29065,-15131,-29065,-15131,-29065,
-15761,-28728,-15761,-28728,-15761,-28728,-15761,-28728,
-16384,-28378,-16384,-28378,-16384,-28378,-16384,-28378,
-16999,-28013,-16999,-28013,-16999,-28013,-16999,-28013,
-17606,-27636,-17606,-27636,-17606,-27636,-17606,-27636,
-18205,-27245,-18205,-27245,-18205,-27245,-18205,-27245,
-18795,-26842,-18795,-26842,-18795,-26842,-18795,-26842,
-19376,-26425,-19376,-26425,-19376,-26425,-19376,-26425,
-19948,-25996,-19948,-25996,-19948,-25996,-19948,-25996,
-20510,-25555,-20510,-25555,-20510,-25555,-20510,-25555,
-21063,-25101,-21063,-25101,-21063,-25101,-21063,-25101,
-21605,-24636,-21605,-24636,-21605,-24636,-21605,-24636,
-22138,-24159,-22138,-24159,-22138,-24159,-22138,-24159,
-22659,-23670,-22659,-23670,-22659,-23670,-22659,-23670,
-23170,-23170,-23170,-23170,-23170,-23170,-23170,-23170,
-23670,-22659,-23670,-22659,-23670,-22659,-23670,-22659,
-24159,-22138,-24159,-22138,-24159,-22138,-24159,-22138,
-24636,-21605,-24636,-21605,-24636,-21605,-24636,-21605,
-25101,-21063,-25101,-21063,-25101,-21063,-25101,-21063,
-25555,-20510,-25555,-20510,-25555,-20510,-25555,-20510,
-25996,-19948,-25996,-19948,-25996,-19948,-25996,-19948,
-26425,-19376,-26425,-19376,-26425,-19376,-26425,-19376,
-26842,-18795,-26842,-18795,-26842,-18795,-26842,-18795,
-27245,-18205,-27245,-18205,-27245,-18205,-27245,-18205,
-27636,-17606,-27636,-17606,-27636,-17606,-27636,-17606,
-28013,-16999,-28013,-16999,-28013,-16999,-28013,-16999,
-28378,-16384,-28378,-16384,-28378,-16384,-28378,-16384,
-28728,-15761,-28728,-15761,-28728,-15761,-28728,-15761,
-29065,-15131,-29065,-15131,-29065,-15131,-29065,-15131,
-29388,-14493,-29388,-14493,-29388,-14493,-29388,-14493,
-29697,-13848,-29697,-13848,-29697,-13848,-29697,-13848,
-29993,-13197,-29993,-13197,-29993,-13197,-29993,-13197,
-30273,-12540,-30273,-12540,-30273,-12540,-30273,-12540,
-30540,-11877,-30540,-11877,-30540,-11877,-30540,-11877,
-30791,-11207,-30791,-11207,-30791,-11207,-30791,-11207,
-31029,-10533,-31029,-10533,-31029,-10533,-31029,-10533,
-31251,-9854,-31251,-9854,-31251,-9854,-31251,-9854,
-31458,-9170,-31458,-9170,-31458,-9170,-31458,-9170,
-31651,-8481,-31651,-8481,-31651,-8481,-31651,-8481,
-31828,-7789,-31828,-7789,-31828,-7789,-31828,-7789,
-31991,-7093,-31991,-7093,-31991,-7093,-31991,-7093,
-32138,-6393,-32138,-6393,-32138,-6393,-32138,-6393,
-32270,-5690,-32270,-5690,-32270,-5690,-32270,-5690,
-32386,-4985,-32386,-4985,-32386,-4985,-32386,-4985,
-32487,-4277,-32487,-4277,-32487,-4277,-32487,-4277,
-32573,-3568,-32573,-3568,-32573,-3568,-32573,-3568,
-32643,-2856,-32643,-2856,-32643,-2856,-32643,-2856,
-32697,-2144,-32697,-2144,-32697,-2144,-32697,-2144,
-32736,-1430,-32736,-1430,-32736,-1430,-32736,-1430,
-32760,-715,-32760,-715,-32760,-715,-32760,-715,
-32767,-1,-32767,-1,-32767,-1,-32767,-1,
-32760,714,-32760,714,-32760,714,-32760,714,
-32736,1429,-32736,1429,-32736,1429,-32736,1429,
-32697,2143,-32697,2143,-32697,2143,-32697,2143,
-32643,2855,-32643,2855,-32643,2855,-32643,2855,
-32573,3567,-32573,3567,-32573,3567,-32573,3567,
-32487,4276,-32487,4276,-32487,4276,-32487,4276,
-32386,4984,-32386,4984,-32386,4984,-32386,4984,
-32270,5689,-32270,5689,-32270,5689,-32270,5689,
-32138,6392,-32138,6392,-32138,6392,-32138,6392,
-31991,7092,-31991,7092,-31991,7092,-31991,7092,
-31828,7788,-31828,7788,-31828,7788,-31828,7788,
-31651,8480,-31651,8480,-31651,8480,-31651,8480,
-31458,9169,-31458,9169,-31458,9169,-31458,9169,
-31251,9853,-31251,9853,-31251,9853,-31251,9853,
-31029,10532,-31029,10532,-31029,10532,-31029,10532,
-30791,11206,-30791,11206,-30791,11206,-30791,11206,
-30540,11876,-30540,11876,-30540,11876,-30540,11876,
-30273,12539,-30273,12539,-30273,12539,-30273,12539,
-29993,13196,-29993,13196,-29993,13196,-29993,13196,
-29697,13847,-29697,13847,-29697,13847,-29697,13847,
-29388,14492,-29388,14492,-29388,14492,-29388,14492,
-29065,15130,-29065,15130,-29065,15130,-29065,15130,
-28728,15760,-28728,15760,-28728,15760,-28728,15760,
-28378,16383,-28378,16383,-28378,16383,-28378,16383,
-28013,16998,-28013,16998,-28013,16998,-28013,16998,
-27636,17605,-27636,17605,-27636,17605,-27636,17605,
-27245,18204,-27245,18204,-27245,18204,-27245,18204,
-26842,18794,-26842,18794,-26842,18794,-26842,18794,
-26425,19375,-26425,19375,-26425,19375,-26425,19375,
-25996,19947,-25996,19947,-25996,19947,-25996,19947,
-25555,20509,-25555,20509,-25555,20509,-25555,20509,
-25101,21062,-25101,21062,-25101,21062,-25101,21062,
-24636,21604,-24636,21604,-24636,21604,-24636,21604,
-24159,22137,-24159,22137,-24159,22137,-24159,22137,
-23670,22658,-23670,22658,-23670,22658,-23670,22658,
-23170,23169,-23170,23169,-23170,23169,-23170,23169,
-22659,23669,-22659,23669,-22659,23669,-22659,23669,
-22138,24158,-22138,24158,-22138,24158,-22138,24158,
-21605,24635,-21605,24635,-21605,24635,-21605,24635,
-21063,25100,-21063,25100,-21063,25100,-21063,25100,
-20510,25554,-20510,25554,-20510,25554,-20510,25554,
-19948,25995,-19948,25995,-19948,25995,-19948,25995,
-19376,26424,-19376,26424,-19376,26424,-19376,26424,
-18795,26841,-18795,26841,-18795,26841,-18795,26841,
-18205,27244,-18205,27244,-18205,27244,-18205,27244,
-17606,27635,-17606,27635,-17606,27635,-17606,27635,
-16999,28012,-16999,28012,-16999,28012,-16999,28012};

void dft576(int16_t *x,int16_t *y,unsigned char scale_flag){ // 192 x 3
  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa576[0];
  __m128i *twb128=(__m128i *)&twb576[0];
  __m128i x2128[576];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[576];//=&ytmp128array3[0];



  for (i=0,j=0;i<192;i++,j+=3) {
    x2128[i]    = x128[j];
    x2128[i+192] = x128[j+1];
    x2128[i+384] = x128[j+2];
  }
  

  dft192((int16_t *)x2128,(int16_t *)ytmp128,1);
  dft192((int16_t *)(x2128+192),(int16_t *)(ytmp128+192),1);
  dft192((int16_t *)(x2128+384),(int16_t *)(ytmp128+384),1);
  
  bfly3_tw1(ytmp128,ytmp128+192,ytmp128+384,y128,y128+192,y128+384);
  
  for (i=1,j=0;i<192;i++,j++) {
    bfly3(ytmp128+i,
	  ytmp128+192+i,
	  ytmp128+384+i,
	  y128+i,
	  y128+192+i,
	  y128+384+i,
	  twa128+j,
	  twb128+j);
	  }
  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(dft_norm_table[14]);
    
    for (i=0;i<576;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();
};

/* Twiddles generated with
twa = floor(32767*exp(-sqrt(-1)*2*pi*(1:299)/600));
twa2 = zeros(1,2*299);
twa2(1:2:end) = real(twa);
twa2(2:2:end) = imag(twa);
fd=fopen("twiddle_tmp.txt","w");
fprintf(fd,"static int16_t twa600[299*2*4] = {");
for i=1:2:(2*298)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
fclose(fd);
 */
static int16_t twa600[299*2*4] = {32765,-344,32765,-344,32765,-344,32765,-344,
32759,-687,32759,-687,32759,-687,32759,-687,
32750,-1030,32750,-1030,32750,-1030,32750,-1030,
32738,-1373,32738,-1373,32738,-1373,32738,-1373,
32722,-1715,32722,-1715,32722,-1715,32722,-1715,
32702,-2058,32702,-2058,32702,-2058,32702,-2058,
32679,-2400,32679,-2400,32679,-2400,32679,-2400,
32652,-2742,32652,-2742,32652,-2742,32652,-2742,
32621,-3084,32621,-3084,32621,-3084,32621,-3084,
32587,-3426,32587,-3426,32587,-3426,32587,-3426,
32549,-3767,32549,-3767,32549,-3767,32549,-3767,
32508,-4107,32508,-4107,32508,-4107,32508,-4107,
32463,-4447,32463,-4447,32463,-4447,32463,-4447,
32415,-4787,32415,-4787,32415,-4787,32415,-4787,
32363,-5126,32363,-5126,32363,-5126,32363,-5126,
32308,-5465,32308,-5465,32308,-5465,32308,-5465,
32249,-5803,32249,-5803,32249,-5803,32249,-5803,
32186,-6140,32186,-6140,32186,-6140,32186,-6140,
32120,-6477,32120,-6477,32120,-6477,32120,-6477,
32050,-6813,32050,-6813,32050,-6813,32050,-6813,
31977,-7148,31977,-7148,31977,-7148,31977,-7148,
31901,-7483,31901,-7483,31901,-7483,31901,-7483,
31821,-7817,31821,-7817,31821,-7817,31821,-7817,
31737,-8149,31737,-8149,31737,-8149,31737,-8149,
31650,-8481,31650,-8481,31650,-8481,31650,-8481,
31559,-8812,31559,-8812,31559,-8812,31559,-8812,
31465,-9142,31465,-9142,31465,-9142,31465,-9142,
31368,-9471,31368,-9471,31368,-9471,31368,-9471,
31267,-9799,31267,-9799,31267,-9799,31267,-9799,
31163,-10126,31163,-10126,31163,-10126,31163,-10126,
31055,-10452,31055,-10452,31055,-10452,31055,-10452,
30944,-10776,30944,-10776,30944,-10776,30944,-10776,
30829,-11100,30829,-11100,30829,-11100,30829,-11100,
30711,-11422,30711,-11422,30711,-11422,30711,-11422,
30590,-11743,30590,-11743,30590,-11743,30590,-11743,
30465,-12063,30465,-12063,30465,-12063,30465,-12063,
30338,-12381,30338,-12381,30338,-12381,30338,-12381,
30206,-12698,30206,-12698,30206,-12698,30206,-12698,
30072,-13014,30072,-13014,30072,-13014,30072,-13014,
29934,-13328,29934,-13328,29934,-13328,29934,-13328,
29792,-13641,29792,-13641,29792,-13641,29792,-13641,
29648,-13952,29648,-13952,29648,-13952,29648,-13952,
29500,-14262,29500,-14262,29500,-14262,29500,-14262,
29349,-14570,29349,-14570,29349,-14570,29349,-14570,
29195,-14876,29195,-14876,29195,-14876,29195,-14876,
29038,-15181,29038,-15181,29038,-15181,29038,-15181,
28877,-15485,28877,-15485,28877,-15485,28877,-15485,
28713,-15786,28713,-15786,28713,-15786,28713,-15786,
28547,-16086,28547,-16086,28547,-16086,28547,-16086,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
28203,-16680,28203,-16680,28203,-16680,28203,-16680,
28027,-16975,28027,-16975,28027,-16975,28027,-16975,
27848,-17267,27848,-17267,27848,-17267,27848,-17267,
27666,-17558,27666,-17558,27666,-17558,27666,-17558,
27480,-17847,27480,-17847,27480,-17847,27480,-17847,
27292,-18133,27292,-18133,27292,-18133,27292,-18133,
27100,-18418,27100,-18418,27100,-18418,27100,-18418,
26906,-18701,26906,-18701,26906,-18701,26906,-18701,
26709,-18982,26709,-18982,26709,-18982,26709,-18982,
26509,-19260,26509,-19260,26509,-19260,26509,-19260,
26305,-19537,26305,-19537,26305,-19537,26305,-19537,
26099,-19811,26099,-19811,26099,-19811,26099,-19811,
25891,-20084,25891,-20084,25891,-20084,25891,-20084,
25679,-20354,25679,-20354,25679,-20354,25679,-20354,
25464,-20621,25464,-20621,25464,-20621,25464,-20621,
25247,-20887,25247,-20887,25247,-20887,25247,-20887,
25027,-21150,25027,-21150,25027,-21150,25027,-21150,
24804,-21411,24804,-21411,24804,-21411,24804,-21411,
24578,-21670,24578,-21670,24578,-21670,24578,-21670,
24350,-21926,24350,-21926,24350,-21926,24350,-21926,
24119,-22180,24119,-22180,24119,-22180,24119,-22180,
23886,-22431,23886,-22431,23886,-22431,23886,-22431,
23649,-22680,23649,-22680,23649,-22680,23649,-22680,
23411,-22926,23411,-22926,23411,-22926,23411,-22926,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
22925,-23412,22925,-23412,22925,-23412,22925,-23412,
22679,-23650,22679,-23650,22679,-23650,22679,-23650,
22430,-23887,22430,-23887,22430,-23887,22430,-23887,
22179,-24120,22179,-24120,22179,-24120,22179,-24120,
21925,-24351,21925,-24351,21925,-24351,21925,-24351,
21669,-24579,21669,-24579,21669,-24579,21669,-24579,
21410,-24805,21410,-24805,21410,-24805,21410,-24805,
21149,-25028,21149,-25028,21149,-25028,21149,-25028,
20886,-25248,20886,-25248,20886,-25248,20886,-25248,
20620,-25465,20620,-25465,20620,-25465,20620,-25465,
20353,-25680,20353,-25680,20353,-25680,20353,-25680,
20083,-25892,20083,-25892,20083,-25892,20083,-25892,
19810,-26100,19810,-26100,19810,-26100,19810,-26100,
19536,-26306,19536,-26306,19536,-26306,19536,-26306,
19259,-26510,19259,-26510,19259,-26510,19259,-26510,
18981,-26710,18981,-26710,18981,-26710,18981,-26710,
18700,-26907,18700,-26907,18700,-26907,18700,-26907,
18417,-27101,18417,-27101,18417,-27101,18417,-27101,
18132,-27293,18132,-27293,18132,-27293,18132,-27293,
17846,-27481,17846,-27481,17846,-27481,17846,-27481,
17557,-27667,17557,-27667,17557,-27667,17557,-27667,
17266,-27849,17266,-27849,17266,-27849,17266,-27849,
16974,-28028,16974,-28028,16974,-28028,16974,-28028,
16679,-28204,16679,-28204,16679,-28204,16679,-28204,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
16085,-28548,16085,-28548,16085,-28548,16085,-28548,
15785,-28714,15785,-28714,15785,-28714,15785,-28714,
15484,-28878,15484,-28878,15484,-28878,15484,-28878,
15180,-29039,15180,-29039,15180,-29039,15180,-29039,
14875,-29196,14875,-29196,14875,-29196,14875,-29196,
14569,-29350,14569,-29350,14569,-29350,14569,-29350,
14261,-29501,14261,-29501,14261,-29501,14261,-29501,
13951,-29649,13951,-29649,13951,-29649,13951,-29649,
13640,-29793,13640,-29793,13640,-29793,13640,-29793,
13327,-29935,13327,-29935,13327,-29935,13327,-29935,
13013,-30073,13013,-30073,13013,-30073,13013,-30073,
12697,-30207,12697,-30207,12697,-30207,12697,-30207,
12380,-30339,12380,-30339,12380,-30339,12380,-30339,
12062,-30466,12062,-30466,12062,-30466,12062,-30466,
11742,-30591,11742,-30591,11742,-30591,11742,-30591,
11421,-30712,11421,-30712,11421,-30712,11421,-30712,
11099,-30830,11099,-30830,11099,-30830,11099,-30830,
10775,-30945,10775,-30945,10775,-30945,10775,-30945,
10451,-31056,10451,-31056,10451,-31056,10451,-31056,
10125,-31164,10125,-31164,10125,-31164,10125,-31164,
9798,-31268,9798,-31268,9798,-31268,9798,-31268,
9470,-31369,9470,-31369,9470,-31369,9470,-31369,
9141,-31466,9141,-31466,9141,-31466,9141,-31466,
8811,-31560,8811,-31560,8811,-31560,8811,-31560,
8480,-31651,8480,-31651,8480,-31651,8480,-31651,
8148,-31738,8148,-31738,8148,-31738,8148,-31738,
7816,-31822,7816,-31822,7816,-31822,7816,-31822,
7482,-31902,7482,-31902,7482,-31902,7482,-31902,
7147,-31978,7147,-31978,7147,-31978,7147,-31978,
6812,-32051,6812,-32051,6812,-32051,6812,-32051,
6476,-32121,6476,-32121,6476,-32121,6476,-32121,
6139,-32187,6139,-32187,6139,-32187,6139,-32187,
5802,-32250,5802,-32250,5802,-32250,5802,-32250,
5464,-32309,5464,-32309,5464,-32309,5464,-32309,
5125,-32364,5125,-32364,5125,-32364,5125,-32364,
4786,-32416,4786,-32416,4786,-32416,4786,-32416,
4446,-32464,4446,-32464,4446,-32464,4446,-32464,
4106,-32509,4106,-32509,4106,-32509,4106,-32509,
3766,-32550,3766,-32550,3766,-32550,3766,-32550,
3425,-32588,3425,-32588,3425,-32588,3425,-32588,
3083,-32622,3083,-32622,3083,-32622,3083,-32622,
2741,-32653,2741,-32653,2741,-32653,2741,-32653,
2399,-32680,2399,-32680,2399,-32680,2399,-32680,
2057,-32703,2057,-32703,2057,-32703,2057,-32703,
1714,-32723,1714,-32723,1714,-32723,1714,-32723,
1372,-32739,1372,-32739,1372,-32739,1372,-32739,
1029,-32751,1029,-32751,1029,-32751,1029,-32751,
686,-32760,686,-32760,686,-32760,686,-32760,
343,-32766,343,-32766,343,-32766,343,-32766,
0,-32767,0,-32767,0,-32767,0,-32767,
-344,-32766,-344,-32766,-344,-32766,-344,-32766,
-687,-32760,-687,-32760,-687,-32760,-687,-32760,
-1030,-32751,-1030,-32751,-1030,-32751,-1030,-32751,
-1373,-32739,-1373,-32739,-1373,-32739,-1373,-32739,
-1715,-32723,-1715,-32723,-1715,-32723,-1715,-32723,
-2058,-32703,-2058,-32703,-2058,-32703,-2058,-32703,
-2400,-32680,-2400,-32680,-2400,-32680,-2400,-32680,
-2742,-32653,-2742,-32653,-2742,-32653,-2742,-32653,
-3084,-32622,-3084,-32622,-3084,-32622,-3084,-32622,
-3426,-32588,-3426,-32588,-3426,-32588,-3426,-32588,
-3767,-32550,-3767,-32550,-3767,-32550,-3767,-32550,
-4107,-32509,-4107,-32509,-4107,-32509,-4107,-32509,
-4447,-32464,-4447,-32464,-4447,-32464,-4447,-32464,
-4787,-32416,-4787,-32416,-4787,-32416,-4787,-32416,
-5126,-32364,-5126,-32364,-5126,-32364,-5126,-32364,
-5465,-32309,-5465,-32309,-5465,-32309,-5465,-32309,
-5803,-32250,-5803,-32250,-5803,-32250,-5803,-32250,
-6140,-32187,-6140,-32187,-6140,-32187,-6140,-32187,
-6477,-32121,-6477,-32121,-6477,-32121,-6477,-32121,
-6813,-32051,-6813,-32051,-6813,-32051,-6813,-32051,
-7148,-31978,-7148,-31978,-7148,-31978,-7148,-31978,
-7483,-31902,-7483,-31902,-7483,-31902,-7483,-31902,
-7817,-31822,-7817,-31822,-7817,-31822,-7817,-31822,
-8149,-31738,-8149,-31738,-8149,-31738,-8149,-31738,
-8481,-31651,-8481,-31651,-8481,-31651,-8481,-31651,
-8812,-31560,-8812,-31560,-8812,-31560,-8812,-31560,
-9142,-31466,-9142,-31466,-9142,-31466,-9142,-31466,
-9471,-31369,-9471,-31369,-9471,-31369,-9471,-31369,
-9799,-31268,-9799,-31268,-9799,-31268,-9799,-31268,
-10126,-31164,-10126,-31164,-10126,-31164,-10126,-31164,
-10452,-31056,-10452,-31056,-10452,-31056,-10452,-31056,
-10776,-30945,-10776,-30945,-10776,-30945,-10776,-30945,
-11100,-30830,-11100,-30830,-11100,-30830,-11100,-30830,
-11422,-30712,-11422,-30712,-11422,-30712,-11422,-30712,
-11743,-30591,-11743,-30591,-11743,-30591,-11743,-30591,
-12063,-30466,-12063,-30466,-12063,-30466,-12063,-30466,
-12381,-30339,-12381,-30339,-12381,-30339,-12381,-30339,
-12698,-30207,-12698,-30207,-12698,-30207,-12698,-30207,
-13014,-30073,-13014,-30073,-13014,-30073,-13014,-30073,
-13328,-29935,-13328,-29935,-13328,-29935,-13328,-29935,
-13641,-29793,-13641,-29793,-13641,-29793,-13641,-29793,
-13952,-29649,-13952,-29649,-13952,-29649,-13952,-29649,
-14262,-29501,-14262,-29501,-14262,-29501,-14262,-29501,
-14570,-29350,-14570,-29350,-14570,-29350,-14570,-29350,
-14876,-29196,-14876,-29196,-14876,-29196,-14876,-29196,
-15181,-29039,-15181,-29039,-15181,-29039,-15181,-29039,
-15485,-28878,-15485,-28878,-15485,-28878,-15485,-28878,
-15786,-28714,-15786,-28714,-15786,-28714,-15786,-28714,
-16086,-28548,-16086,-28548,-16086,-28548,-16086,-28548,
-16384,-28378,-16384,-28378,-16384,-28378,-16384,-28378,
-16680,-28204,-16680,-28204,-16680,-28204,-16680,-28204,
-16975,-28028,-16975,-28028,-16975,-28028,-16975,-28028,
-17267,-27849,-17267,-27849,-17267,-27849,-17267,-27849,
-17558,-27667,-17558,-27667,-17558,-27667,-17558,-27667,
-17847,-27481,-17847,-27481,-17847,-27481,-17847,-27481,
-18133,-27293,-18133,-27293,-18133,-27293,-18133,-27293,
-18418,-27101,-18418,-27101,-18418,-27101,-18418,-27101,
-18701,-26907,-18701,-26907,-18701,-26907,-18701,-26907,
-18982,-26710,-18982,-26710,-18982,-26710,-18982,-26710,
-19260,-26510,-19260,-26510,-19260,-26510,-19260,-26510,
-19537,-26306,-19537,-26306,-19537,-26306,-19537,-26306,
-19811,-26100,-19811,-26100,-19811,-26100,-19811,-26100,
-20084,-25892,-20084,-25892,-20084,-25892,-20084,-25892,
-20354,-25680,-20354,-25680,-20354,-25680,-20354,-25680,
-20621,-25465,-20621,-25465,-20621,-25465,-20621,-25465,
-20887,-25248,-20887,-25248,-20887,-25248,-20887,-25248,
-21150,-25028,-21150,-25028,-21150,-25028,-21150,-25028,
-21411,-24805,-21411,-24805,-21411,-24805,-21411,-24805,
-21670,-24579,-21670,-24579,-21670,-24579,-21670,-24579,
-21926,-24351,-21926,-24351,-21926,-24351,-21926,-24351,
-22180,-24120,-22180,-24120,-22180,-24120,-22180,-24120,
-22431,-23887,-22431,-23887,-22431,-23887,-22431,-23887,
-22680,-23650,-22680,-23650,-22680,-23650,-22680,-23650,
-22926,-23412,-22926,-23412,-22926,-23412,-22926,-23412,
-23170,-23170,-23170,-23170,-23170,-23170,-23170,-23170,
-23412,-22926,-23412,-22926,-23412,-22926,-23412,-22926,
-23650,-22680,-23650,-22680,-23650,-22680,-23650,-22680,
-23887,-22431,-23887,-22431,-23887,-22431,-23887,-22431,
-24120,-22180,-24120,-22180,-24120,-22180,-24120,-22180,
-24351,-21926,-24351,-21926,-24351,-21926,-24351,-21926,
-24579,-21670,-24579,-21670,-24579,-21670,-24579,-21670,
-24805,-21411,-24805,-21411,-24805,-21411,-24805,-21411,
-25028,-21150,-25028,-21150,-25028,-21150,-25028,-21150,
-25248,-20887,-25248,-20887,-25248,-20887,-25248,-20887,
-25465,-20621,-25465,-20621,-25465,-20621,-25465,-20621,
-25680,-20354,-25680,-20354,-25680,-20354,-25680,-20354,
-25892,-20084,-25892,-20084,-25892,-20084,-25892,-20084,
-26100,-19811,-26100,-19811,-26100,-19811,-26100,-19811,
-26306,-19537,-26306,-19537,-26306,-19537,-26306,-19537,
-26510,-19260,-26510,-19260,-26510,-19260,-26510,-19260,
-26710,-18982,-26710,-18982,-26710,-18982,-26710,-18982,
-26907,-18701,-26907,-18701,-26907,-18701,-26907,-18701,
-27101,-18418,-27101,-18418,-27101,-18418,-27101,-18418,
-27293,-18133,-27293,-18133,-27293,-18133,-27293,-18133,
-27481,-17847,-27481,-17847,-27481,-17847,-27481,-17847,
-27667,-17558,-27667,-17558,-27667,-17558,-27667,-17558,
-27849,-17267,-27849,-17267,-27849,-17267,-27849,-17267,
-28028,-16975,-28028,-16975,-28028,-16975,-28028,-16975,
-28204,-16680,-28204,-16680,-28204,-16680,-28204,-16680,
-28378,-16384,-28378,-16384,-28378,-16384,-28378,-16384,
-28548,-16086,-28548,-16086,-28548,-16086,-28548,-16086,
-28714,-15786,-28714,-15786,-28714,-15786,-28714,-15786,
-28878,-15485,-28878,-15485,-28878,-15485,-28878,-15485,
-29039,-15181,-29039,-15181,-29039,-15181,-29039,-15181,
-29196,-14876,-29196,-14876,-29196,-14876,-29196,-14876,
-29350,-14570,-29350,-14570,-29350,-14570,-29350,-14570,
-29501,-14262,-29501,-14262,-29501,-14262,-29501,-14262,
-29649,-13952,-29649,-13952,-29649,-13952,-29649,-13952,
-29793,-13641,-29793,-13641,-29793,-13641,-29793,-13641,
-29935,-13328,-29935,-13328,-29935,-13328,-29935,-13328,
-30073,-13014,-30073,-13014,-30073,-13014,-30073,-13014,
-30207,-12698,-30207,-12698,-30207,-12698,-30207,-12698,
-30339,-12381,-30339,-12381,-30339,-12381,-30339,-12381,
-30466,-12063,-30466,-12063,-30466,-12063,-30466,-12063,
-30591,-11743,-30591,-11743,-30591,-11743,-30591,-11743,
-30712,-11422,-30712,-11422,-30712,-11422,-30712,-11422,
-30830,-11100,-30830,-11100,-30830,-11100,-30830,-11100,
-30945,-10776,-30945,-10776,-30945,-10776,-30945,-10776,
-31056,-10452,-31056,-10452,-31056,-10452,-31056,-10452,
-31164,-10126,-31164,-10126,-31164,-10126,-31164,-10126,
-31268,-9799,-31268,-9799,-31268,-9799,-31268,-9799,
-31369,-9471,-31369,-9471,-31369,-9471,-31369,-9471,
-31466,-9142,-31466,-9142,-31466,-9142,-31466,-9142,
-31560,-8812,-31560,-8812,-31560,-8812,-31560,-8812,
-31651,-8481,-31651,-8481,-31651,-8481,-31651,-8481,
-31738,-8149,-31738,-8149,-31738,-8149,-31738,-8149,
-31822,-7817,-31822,-7817,-31822,-7817,-31822,-7817,
-31902,-7483,-31902,-7483,-31902,-7483,-31902,-7483,
-31978,-7148,-31978,-7148,-31978,-7148,-31978,-7148,
-32051,-6813,-32051,-6813,-32051,-6813,-32051,-6813,
-32121,-6477,-32121,-6477,-32121,-6477,-32121,-6477,
-32187,-6140,-32187,-6140,-32187,-6140,-32187,-6140,
-32250,-5803,-32250,-5803,-32250,-5803,-32250,-5803,
-32309,-5465,-32309,-5465,-32309,-5465,-32309,-5465,
-32364,-5126,-32364,-5126,-32364,-5126,-32364,-5126,
-32416,-4787,-32416,-4787,-32416,-4787,-32416,-4787,
-32464,-4447,-32464,-4447,-32464,-4447,-32464,-4447,
-32509,-4107,-32509,-4107,-32509,-4107,-32509,-4107,
-32550,-3767,-32550,-3767,-32550,-3767,-32550,-3767,
-32588,-3426,-32588,-3426,-32588,-3426,-32588,-3426,
-32622,-3084,-32622,-3084,-32622,-3084,-32622,-3084,
-32653,-2742,-32653,-2742,-32653,-2742,-32653,-2742,
-32680,-2400,-32680,-2400,-32680,-2400,-32680,-2400,
-32703,-2058,-32703,-2058,-32703,-2058,-32703,-2058,
-32723,-1715,-32723,-1715,-32723,-1715,-32723,-1715,
-32739,-1373,-32739,-1373,-32739,-1373,-32739,-1373,
-32751,-1030,-32751,-1030,-32751,-1030,-32751,-1030,
-32760,-687,-32760,-687,-32760,-687,-32760,-687,
-32766,-344,-32766,-344,-32766,-344,-32766,-344};

void dft600(int16_t *x,int16_t *y,unsigned char scale_flag){ // 300 x 2
  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *tw128=(__m128i *)&twa600[0];
  __m128i x2128[600];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[600];//=&ytmp128array2[0];


  for (i=0,j=0;i<300;i++,j+=2) {
    x2128[i]    = x128[j];
    x2128[i+300] = x128[j+1];
  }

  dft300((int16_t *)x2128,(int16_t *)ytmp128,1);
  dft300((int16_t *)(x2128+300),(int16_t *)(ytmp128+300),1);


  bfly2_tw1(ytmp128,ytmp128+300,y128,y128+300);
  for (i=1,j=0;i<300;i++,j++) {
    bfly2(ytmp128+i,
	  ytmp128+300+i,
	  y128+i,
	  y128+300+i,
	  tw128+j);
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(ONE_OVER_SQRT2_Q15);
    
    for (i=0;i<600;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();
};


/*
Twiddles generated with

twa = floor(32767*exp(-sqrt(-1)*2*pi*(1:215)/648));
twb = floor(32767*exp(-sqrt(-1)*2*pi*2*(1:215)/648));
twa2 = zeros(1,2*215);
twb2 = zeros(1,2*215);
twa2(1:2:end) = real(twa);
twa2(2:2:end) = imag(twa);
twb2(1:2:end) = real(twb);
twb2(2:2:end) = imag(twb);
fd=fopen("twiddle_tmp.txt","w");
fprintf(fd,"static int16_t twa648[215*2*4] = {");
for i=1:2:(2*214)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
fprintf(fd,"static int16_t twb648[215*2*4] = {");
for i=1:2:(2*214)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
fclose(fd);
*/
static int16_t twa648[215*2*4] = {32765,-318,32765,-318,32765,-318,32765,-318,
32760,-636,32760,-636,32760,-636,32760,-636,
32753,-954,32753,-954,32753,-954,32753,-954,
32742,-1271,32742,-1271,32742,-1271,32742,-1271,
32728,-1588,32728,-1588,32728,-1588,32728,-1588,
32711,-1906,32711,-1906,32711,-1906,32711,-1906,
32691,-2223,32691,-2223,32691,-2223,32691,-2223,
32668,-2540,32668,-2540,32668,-2540,32668,-2540,
32642,-2856,32642,-2856,32642,-2856,32642,-2856,
32613,-3173,32613,-3173,32613,-3173,32613,-3173,
32580,-3489,32580,-3489,32580,-3489,32580,-3489,
32545,-3805,32545,-3805,32545,-3805,32545,-3805,
32507,-4120,32507,-4120,32507,-4120,32507,-4120,
32465,-4435,32465,-4435,32465,-4435,32465,-4435,
32421,-4749,32421,-4749,32421,-4749,32421,-4749,
32373,-5064,32373,-5064,32373,-5064,32373,-5064,
32322,-5377,32322,-5377,32322,-5377,32322,-5377,
32269,-5690,32269,-5690,32269,-5690,32269,-5690,
32212,-6003,32212,-6003,32212,-6003,32212,-6003,
32152,-6315,32152,-6315,32152,-6315,32152,-6315,
32090,-6627,32090,-6627,32090,-6627,32090,-6627,
32024,-6937,32024,-6937,32024,-6937,32024,-6937,
31955,-7248,31955,-7248,31955,-7248,31955,-7248,
31883,-7557,31883,-7557,31883,-7557,31883,-7557,
31808,-7866,31808,-7866,31808,-7866,31808,-7866,
31731,-8174,31731,-8174,31731,-8174,31731,-8174,
31650,-8481,31650,-8481,31650,-8481,31650,-8481,
31566,-8788,31566,-8788,31566,-8788,31566,-8788,
31480,-9093,31480,-9093,31480,-9093,31480,-9093,
31390,-9398,31390,-9398,31390,-9398,31390,-9398,
31297,-9702,31297,-9702,31297,-9702,31297,-9702,
31202,-10005,31202,-10005,31202,-10005,31202,-10005,
31103,-10307,31103,-10307,31103,-10307,31103,-10307,
31002,-10608,31002,-10608,31002,-10608,31002,-10608,
30898,-10908,30898,-10908,30898,-10908,30898,-10908,
30790,-11207,30790,-11207,30790,-11207,30790,-11207,
30680,-11505,30680,-11505,30680,-11505,30680,-11505,
30567,-11802,30567,-11802,30567,-11802,30567,-11802,
30451,-12098,30451,-12098,30451,-12098,30451,-12098,
30333,-12393,30333,-12393,30333,-12393,30333,-12393,
30211,-12687,30211,-12687,30211,-12687,30211,-12687,
30087,-12979,30087,-12979,30087,-12979,30087,-12979,
29959,-13270,29959,-13270,29959,-13270,29959,-13270,
29829,-13560,29829,-13560,29829,-13560,29829,-13560,
29696,-13848,29696,-13848,29696,-13848,29696,-13848,
29561,-14136,29561,-14136,29561,-14136,29561,-14136,
29422,-14422,29422,-14422,29422,-14422,29422,-14422,
29281,-14706,29281,-14706,29281,-14706,29281,-14706,
29137,-14990,29137,-14990,29137,-14990,29137,-14990,
28990,-15271,28990,-15271,28990,-15271,28990,-15271,
28841,-15552,28841,-15552,28841,-15552,28841,-15552,
28689,-15831,28689,-15831,28689,-15831,28689,-15831,
28534,-16108,28534,-16108,28534,-16108,28534,-16108,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
28216,-16658,28216,-16658,28216,-16658,28216,-16658,
28054,-16931,28054,-16931,28054,-16931,28054,-16931,
27888,-17202,27888,-17202,27888,-17202,27888,-17202,
27720,-17472,27720,-17472,27720,-17472,27720,-17472,
27549,-17740,27549,-17740,27549,-17740,27549,-17740,
27376,-18006,27376,-18006,27376,-18006,27376,-18006,
27200,-18271,27200,-18271,27200,-18271,27200,-18271,
27022,-18534,27022,-18534,27022,-18534,27022,-18534,
26841,-18795,26841,-18795,26841,-18795,26841,-18795,
26657,-19054,26657,-19054,26657,-19054,26657,-19054,
26471,-19312,26471,-19312,26471,-19312,26471,-19312,
26283,-19568,26283,-19568,26283,-19568,26283,-19568,
26092,-19822,26092,-19822,26092,-19822,26092,-19822,
25898,-20074,25898,-20074,25898,-20074,25898,-20074,
25702,-20324,25702,-20324,25702,-20324,25702,-20324,
25504,-20572,25504,-20572,25504,-20572,25504,-20572,
25304,-20818,25304,-20818,25304,-20818,25304,-20818,
25100,-21063,25100,-21063,25100,-21063,25100,-21063,
24895,-21305,24895,-21305,24895,-21305,24895,-21305,
24687,-21546,24687,-21546,24687,-21546,24687,-21546,
24477,-21784,24477,-21784,24477,-21784,24477,-21784,
24265,-22020,24265,-22020,24265,-22020,24265,-22020,
24050,-22254,24050,-22254,24050,-22254,24050,-22254,
23833,-22487,23833,-22487,23833,-22487,23833,-22487,
23614,-22717,23614,-22717,23614,-22717,23614,-22717,
23393,-22945,23393,-22945,23393,-22945,23393,-22945,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
22944,-23394,22944,-23394,22944,-23394,22944,-23394,
22716,-23615,22716,-23615,22716,-23615,22716,-23615,
22486,-23834,22486,-23834,22486,-23834,22486,-23834,
22253,-24051,22253,-24051,22253,-24051,22253,-24051,
22019,-24266,22019,-24266,22019,-24266,22019,-24266,
21783,-24478,21783,-24478,21783,-24478,21783,-24478,
21545,-24688,21545,-24688,21545,-24688,21545,-24688,
21304,-24896,21304,-24896,21304,-24896,21304,-24896,
21062,-25101,21062,-25101,21062,-25101,21062,-25101,
20817,-25305,20817,-25305,20817,-25305,20817,-25305,
20571,-25505,20571,-25505,20571,-25505,20571,-25505,
20323,-25703,20323,-25703,20323,-25703,20323,-25703,
20073,-25899,20073,-25899,20073,-25899,20073,-25899,
19821,-26093,19821,-26093,19821,-26093,19821,-26093,
19567,-26284,19567,-26284,19567,-26284,19567,-26284,
19311,-26472,19311,-26472,19311,-26472,19311,-26472,
19053,-26658,19053,-26658,19053,-26658,19053,-26658,
18794,-26842,18794,-26842,18794,-26842,18794,-26842,
18533,-27023,18533,-27023,18533,-27023,18533,-27023,
18270,-27201,18270,-27201,18270,-27201,18270,-27201,
18005,-27377,18005,-27377,18005,-27377,18005,-27377,
17739,-27550,17739,-27550,17739,-27550,17739,-27550,
17471,-27721,17471,-27721,17471,-27721,17471,-27721,
17201,-27889,17201,-27889,17201,-27889,17201,-27889,
16930,-28055,16930,-28055,16930,-28055,16930,-28055,
16657,-28217,16657,-28217,16657,-28217,16657,-28217,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
16107,-28535,16107,-28535,16107,-28535,16107,-28535,
15830,-28690,15830,-28690,15830,-28690,15830,-28690,
15551,-28842,15551,-28842,15551,-28842,15551,-28842,
15270,-28991,15270,-28991,15270,-28991,15270,-28991,
14989,-29138,14989,-29138,14989,-29138,14989,-29138,
14705,-29282,14705,-29282,14705,-29282,14705,-29282,
14421,-29423,14421,-29423,14421,-29423,14421,-29423,
14135,-29562,14135,-29562,14135,-29562,14135,-29562,
13847,-29697,13847,-29697,13847,-29697,13847,-29697,
13559,-29830,13559,-29830,13559,-29830,13559,-29830,
13269,-29960,13269,-29960,13269,-29960,13269,-29960,
12978,-30088,12978,-30088,12978,-30088,12978,-30088,
12686,-30212,12686,-30212,12686,-30212,12686,-30212,
12392,-30334,12392,-30334,12392,-30334,12392,-30334,
12097,-30452,12097,-30452,12097,-30452,12097,-30452,
11801,-30568,11801,-30568,11801,-30568,11801,-30568,
11504,-30681,11504,-30681,11504,-30681,11504,-30681,
11206,-30791,11206,-30791,11206,-30791,11206,-30791,
10907,-30899,10907,-30899,10907,-30899,10907,-30899,
10607,-31003,10607,-31003,10607,-31003,10607,-31003,
10306,-31104,10306,-31104,10306,-31104,10306,-31104,
10004,-31203,10004,-31203,10004,-31203,10004,-31203,
9701,-31298,9701,-31298,9701,-31298,9701,-31298,
9397,-31391,9397,-31391,9397,-31391,9397,-31391,
9092,-31481,9092,-31481,9092,-31481,9092,-31481,
8787,-31567,8787,-31567,8787,-31567,8787,-31567,
8480,-31651,8480,-31651,8480,-31651,8480,-31651,
8173,-31732,8173,-31732,8173,-31732,8173,-31732,
7865,-31809,7865,-31809,7865,-31809,7865,-31809,
7556,-31884,7556,-31884,7556,-31884,7556,-31884,
7247,-31956,7247,-31956,7247,-31956,7247,-31956,
6936,-32025,6936,-32025,6936,-32025,6936,-32025,
6626,-32091,6626,-32091,6626,-32091,6626,-32091,
6314,-32153,6314,-32153,6314,-32153,6314,-32153,
6002,-32213,6002,-32213,6002,-32213,6002,-32213,
5689,-32270,5689,-32270,5689,-32270,5689,-32270,
5376,-32323,5376,-32323,5376,-32323,5376,-32323,
5063,-32374,5063,-32374,5063,-32374,5063,-32374,
4748,-32422,4748,-32422,4748,-32422,4748,-32422,
4434,-32466,4434,-32466,4434,-32466,4434,-32466,
4119,-32508,4119,-32508,4119,-32508,4119,-32508,
3804,-32546,3804,-32546,3804,-32546,3804,-32546,
3488,-32581,3488,-32581,3488,-32581,3488,-32581,
3172,-32614,3172,-32614,3172,-32614,3172,-32614,
2855,-32643,2855,-32643,2855,-32643,2855,-32643,
2539,-32669,2539,-32669,2539,-32669,2539,-32669,
2222,-32692,2222,-32692,2222,-32692,2222,-32692,
1905,-32712,1905,-32712,1905,-32712,1905,-32712,
1587,-32729,1587,-32729,1587,-32729,1587,-32729,
1270,-32743,1270,-32743,1270,-32743,1270,-32743,
953,-32754,953,-32754,953,-32754,953,-32754,
635,-32761,635,-32761,635,-32761,635,-32761,
317,-32766,317,-32766,317,-32766,317,-32766,
0,-32767,0,-32767,0,-32767,0,-32767,
-318,-32766,-318,-32766,-318,-32766,-318,-32766,
-636,-32761,-636,-32761,-636,-32761,-636,-32761,
-954,-32754,-954,-32754,-954,-32754,-954,-32754,
-1271,-32743,-1271,-32743,-1271,-32743,-1271,-32743,
-1588,-32729,-1588,-32729,-1588,-32729,-1588,-32729,
-1906,-32712,-1906,-32712,-1906,-32712,-1906,-32712,
-2223,-32692,-2223,-32692,-2223,-32692,-2223,-32692,
-2540,-32669,-2540,-32669,-2540,-32669,-2540,-32669,
-2856,-32643,-2856,-32643,-2856,-32643,-2856,-32643,
-3173,-32614,-3173,-32614,-3173,-32614,-3173,-32614,
-3489,-32581,-3489,-32581,-3489,-32581,-3489,-32581,
-3805,-32546,-3805,-32546,-3805,-32546,-3805,-32546,
-4120,-32508,-4120,-32508,-4120,-32508,-4120,-32508,
-4435,-32466,-4435,-32466,-4435,-32466,-4435,-32466,
-4749,-32422,-4749,-32422,-4749,-32422,-4749,-32422,
-5064,-32374,-5064,-32374,-5064,-32374,-5064,-32374,
-5377,-32323,-5377,-32323,-5377,-32323,-5377,-32323,
-5690,-32270,-5690,-32270,-5690,-32270,-5690,-32270,
-6003,-32213,-6003,-32213,-6003,-32213,-6003,-32213,
-6315,-32153,-6315,-32153,-6315,-32153,-6315,-32153,
-6627,-32091,-6627,-32091,-6627,-32091,-6627,-32091,
-6937,-32025,-6937,-32025,-6937,-32025,-6937,-32025,
-7248,-31956,-7248,-31956,-7248,-31956,-7248,-31956,
-7557,-31884,-7557,-31884,-7557,-31884,-7557,-31884,
-7866,-31809,-7866,-31809,-7866,-31809,-7866,-31809,
-8174,-31732,-8174,-31732,-8174,-31732,-8174,-31732,
-8481,-31651,-8481,-31651,-8481,-31651,-8481,-31651,
-8788,-31567,-8788,-31567,-8788,-31567,-8788,-31567,
-9093,-31481,-9093,-31481,-9093,-31481,-9093,-31481,
-9398,-31391,-9398,-31391,-9398,-31391,-9398,-31391,
-9702,-31298,-9702,-31298,-9702,-31298,-9702,-31298,
-10005,-31203,-10005,-31203,-10005,-31203,-10005,-31203,
-10307,-31104,-10307,-31104,-10307,-31104,-10307,-31104,
-10608,-31003,-10608,-31003,-10608,-31003,-10608,-31003,
-10908,-30899,-10908,-30899,-10908,-30899,-10908,-30899,
-11207,-30791,-11207,-30791,-11207,-30791,-11207,-30791,
-11505,-30681,-11505,-30681,-11505,-30681,-11505,-30681,
-11802,-30568,-11802,-30568,-11802,-30568,-11802,-30568,
-12098,-30452,-12098,-30452,-12098,-30452,-12098,-30452,
-12393,-30334,-12393,-30334,-12393,-30334,-12393,-30334,
-12687,-30212,-12687,-30212,-12687,-30212,-12687,-30212,
-12979,-30088,-12979,-30088,-12979,-30088,-12979,-30088,
-13270,-29960,-13270,-29960,-13270,-29960,-13270,-29960,
-13560,-29830,-13560,-29830,-13560,-29830,-13560,-29830,
-13848,-29697,-13848,-29697,-13848,-29697,-13848,-29697,
-14136,-29562,-14136,-29562,-14136,-29562,-14136,-29562,
-14422,-29423,-14422,-29423,-14422,-29423,-14422,-29423,
-14706,-29282,-14706,-29282,-14706,-29282,-14706,-29282,
-14990,-29138,-14990,-29138,-14990,-29138,-14990,-29138,
-15271,-28991,-15271,-28991,-15271,-28991,-15271,-28991,
-15552,-28842,-15552,-28842,-15552,-28842,-15552,-28842,
-15831,-28690,-15831,-28690,-15831,-28690,-15831,-28690,
-16108,-28535,-16108,-28535,-16108,-28535,-16108,-28535};
static int16_t twb648[215*2*4] = {32760,-636,32760,-636,32760,-636,32760,-636,
32742,-1271,32742,-1271,32742,-1271,32742,-1271,
32711,-1906,32711,-1906,32711,-1906,32711,-1906,
32668,-2540,32668,-2540,32668,-2540,32668,-2540,
32613,-3173,32613,-3173,32613,-3173,32613,-3173,
32545,-3805,32545,-3805,32545,-3805,32545,-3805,
32465,-4435,32465,-4435,32465,-4435,32465,-4435,
32373,-5064,32373,-5064,32373,-5064,32373,-5064,
32269,-5690,32269,-5690,32269,-5690,32269,-5690,
32152,-6315,32152,-6315,32152,-6315,32152,-6315,
32024,-6937,32024,-6937,32024,-6937,32024,-6937,
31883,-7557,31883,-7557,31883,-7557,31883,-7557,
31731,-8174,31731,-8174,31731,-8174,31731,-8174,
31566,-8788,31566,-8788,31566,-8788,31566,-8788,
31390,-9398,31390,-9398,31390,-9398,31390,-9398,
31202,-10005,31202,-10005,31202,-10005,31202,-10005,
31002,-10608,31002,-10608,31002,-10608,31002,-10608,
30790,-11207,30790,-11207,30790,-11207,30790,-11207,
30567,-11802,30567,-11802,30567,-11802,30567,-11802,
30333,-12393,30333,-12393,30333,-12393,30333,-12393,
30087,-12979,30087,-12979,30087,-12979,30087,-12979,
29829,-13560,29829,-13560,29829,-13560,29829,-13560,
29561,-14136,29561,-14136,29561,-14136,29561,-14136,
29281,-14706,29281,-14706,29281,-14706,29281,-14706,
28990,-15271,28990,-15271,28990,-15271,28990,-15271,
28689,-15831,28689,-15831,28689,-15831,28689,-15831,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
28054,-16931,28054,-16931,28054,-16931,28054,-16931,
27720,-17472,27720,-17472,27720,-17472,27720,-17472,
27376,-18006,27376,-18006,27376,-18006,27376,-18006,
27022,-18534,27022,-18534,27022,-18534,27022,-18534,
26657,-19054,26657,-19054,26657,-19054,26657,-19054,
26283,-19568,26283,-19568,26283,-19568,26283,-19568,
25898,-20074,25898,-20074,25898,-20074,25898,-20074,
25504,-20572,25504,-20572,25504,-20572,25504,-20572,
25100,-21063,25100,-21063,25100,-21063,25100,-21063,
24687,-21546,24687,-21546,24687,-21546,24687,-21546,
24265,-22020,24265,-22020,24265,-22020,24265,-22020,
23833,-22487,23833,-22487,23833,-22487,23833,-22487,
23393,-22945,23393,-22945,23393,-22945,23393,-22945,
22944,-23394,22944,-23394,22944,-23394,22944,-23394,
22486,-23834,22486,-23834,22486,-23834,22486,-23834,
22019,-24266,22019,-24266,22019,-24266,22019,-24266,
21545,-24688,21545,-24688,21545,-24688,21545,-24688,
21062,-25101,21062,-25101,21062,-25101,21062,-25101,
20571,-25505,20571,-25505,20571,-25505,20571,-25505,
20073,-25899,20073,-25899,20073,-25899,20073,-25899,
19567,-26284,19567,-26284,19567,-26284,19567,-26284,
19053,-26658,19053,-26658,19053,-26658,19053,-26658,
18533,-27023,18533,-27023,18533,-27023,18533,-27023,
18005,-27377,18005,-27377,18005,-27377,18005,-27377,
17471,-27721,17471,-27721,17471,-27721,17471,-27721,
16930,-28055,16930,-28055,16930,-28055,16930,-28055,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
15830,-28690,15830,-28690,15830,-28690,15830,-28690,
15270,-28991,15270,-28991,15270,-28991,15270,-28991,
14705,-29282,14705,-29282,14705,-29282,14705,-29282,
14135,-29562,14135,-29562,14135,-29562,14135,-29562,
13559,-29830,13559,-29830,13559,-29830,13559,-29830,
12978,-30088,12978,-30088,12978,-30088,12978,-30088,
12392,-30334,12392,-30334,12392,-30334,12392,-30334,
11801,-30568,11801,-30568,11801,-30568,11801,-30568,
11206,-30791,11206,-30791,11206,-30791,11206,-30791,
10607,-31003,10607,-31003,10607,-31003,10607,-31003,
10004,-31203,10004,-31203,10004,-31203,10004,-31203,
9397,-31391,9397,-31391,9397,-31391,9397,-31391,
8787,-31567,8787,-31567,8787,-31567,8787,-31567,
8173,-31732,8173,-31732,8173,-31732,8173,-31732,
7556,-31884,7556,-31884,7556,-31884,7556,-31884,
6936,-32025,6936,-32025,6936,-32025,6936,-32025,
6314,-32153,6314,-32153,6314,-32153,6314,-32153,
5689,-32270,5689,-32270,5689,-32270,5689,-32270,
5063,-32374,5063,-32374,5063,-32374,5063,-32374,
4434,-32466,4434,-32466,4434,-32466,4434,-32466,
3804,-32546,3804,-32546,3804,-32546,3804,-32546,
3172,-32614,3172,-32614,3172,-32614,3172,-32614,
2539,-32669,2539,-32669,2539,-32669,2539,-32669,
1905,-32712,1905,-32712,1905,-32712,1905,-32712,
1270,-32743,1270,-32743,1270,-32743,1270,-32743,
635,-32761,635,-32761,635,-32761,635,-32761,
0,-32767,0,-32767,0,-32767,0,-32767,
-636,-32761,-636,-32761,-636,-32761,-636,-32761,
-1271,-32743,-1271,-32743,-1271,-32743,-1271,-32743,
-1906,-32712,-1906,-32712,-1906,-32712,-1906,-32712,
-2540,-32669,-2540,-32669,-2540,-32669,-2540,-32669,
-3173,-32614,-3173,-32614,-3173,-32614,-3173,-32614,
-3805,-32546,-3805,-32546,-3805,-32546,-3805,-32546,
-4435,-32466,-4435,-32466,-4435,-32466,-4435,-32466,
-5064,-32374,-5064,-32374,-5064,-32374,-5064,-32374,
-5690,-32270,-5690,-32270,-5690,-32270,-5690,-32270,
-6315,-32153,-6315,-32153,-6315,-32153,-6315,-32153,
-6937,-32025,-6937,-32025,-6937,-32025,-6937,-32025,
-7557,-31884,-7557,-31884,-7557,-31884,-7557,-31884,
-8174,-31732,-8174,-31732,-8174,-31732,-8174,-31732,
-8788,-31567,-8788,-31567,-8788,-31567,-8788,-31567,
-9398,-31391,-9398,-31391,-9398,-31391,-9398,-31391,
-10005,-31203,-10005,-31203,-10005,-31203,-10005,-31203,
-10608,-31003,-10608,-31003,-10608,-31003,-10608,-31003,
-11207,-30791,-11207,-30791,-11207,-30791,-11207,-30791,
-11802,-30568,-11802,-30568,-11802,-30568,-11802,-30568,
-12393,-30334,-12393,-30334,-12393,-30334,-12393,-30334,
-12979,-30088,-12979,-30088,-12979,-30088,-12979,-30088,
-13560,-29830,-13560,-29830,-13560,-29830,-13560,-29830,
-14136,-29562,-14136,-29562,-14136,-29562,-14136,-29562,
-14706,-29282,-14706,-29282,-14706,-29282,-14706,-29282,
-15271,-28991,-15271,-28991,-15271,-28991,-15271,-28991,
-15831,-28690,-15831,-28690,-15831,-28690,-15831,-28690,
-16384,-28378,-16384,-28378,-16384,-28378,-16384,-28378,
-16931,-28055,-16931,-28055,-16931,-28055,-16931,-28055,
-17472,-27721,-17472,-27721,-17472,-27721,-17472,-27721,
-18006,-27377,-18006,-27377,-18006,-27377,-18006,-27377,
-18534,-27023,-18534,-27023,-18534,-27023,-18534,-27023,
-19054,-26658,-19054,-26658,-19054,-26658,-19054,-26658,
-19568,-26284,-19568,-26284,-19568,-26284,-19568,-26284,
-20074,-25899,-20074,-25899,-20074,-25899,-20074,-25899,
-20572,-25505,-20572,-25505,-20572,-25505,-20572,-25505,
-21063,-25101,-21063,-25101,-21063,-25101,-21063,-25101,
-21546,-24688,-21546,-24688,-21546,-24688,-21546,-24688,
-22020,-24266,-22020,-24266,-22020,-24266,-22020,-24266,
-22487,-23834,-22487,-23834,-22487,-23834,-22487,-23834,
-22945,-23394,-22945,-23394,-22945,-23394,-22945,-23394,
-23394,-22945,-23394,-22945,-23394,-22945,-23394,-22945,
-23834,-22487,-23834,-22487,-23834,-22487,-23834,-22487,
-24266,-22020,-24266,-22020,-24266,-22020,-24266,-22020,
-24688,-21546,-24688,-21546,-24688,-21546,-24688,-21546,
-25101,-21063,-25101,-21063,-25101,-21063,-25101,-21063,
-25505,-20572,-25505,-20572,-25505,-20572,-25505,-20572,
-25899,-20074,-25899,-20074,-25899,-20074,-25899,-20074,
-26284,-19568,-26284,-19568,-26284,-19568,-26284,-19568,
-26658,-19054,-26658,-19054,-26658,-19054,-26658,-19054,
-27023,-18534,-27023,-18534,-27023,-18534,-27023,-18534,
-27377,-18006,-27377,-18006,-27377,-18006,-27377,-18006,
-27721,-17472,-27721,-17472,-27721,-17472,-27721,-17472,
-28055,-16931,-28055,-16931,-28055,-16931,-28055,-16931,
-28378,-16384,-28378,-16384,-28378,-16384,-28378,-16384,
-28690,-15831,-28690,-15831,-28690,-15831,-28690,-15831,
-28991,-15271,-28991,-15271,-28991,-15271,-28991,-15271,
-29282,-14706,-29282,-14706,-29282,-14706,-29282,-14706,
-29562,-14136,-29562,-14136,-29562,-14136,-29562,-14136,
-29830,-13560,-29830,-13560,-29830,-13560,-29830,-13560,
-30088,-12979,-30088,-12979,-30088,-12979,-30088,-12979,
-30334,-12393,-30334,-12393,-30334,-12393,-30334,-12393,
-30568,-11802,-30568,-11802,-30568,-11802,-30568,-11802,
-30791,-11207,-30791,-11207,-30791,-11207,-30791,-11207,
-31003,-10608,-31003,-10608,-31003,-10608,-31003,-10608,
-31203,-10005,-31203,-10005,-31203,-10005,-31203,-10005,
-31391,-9398,-31391,-9398,-31391,-9398,-31391,-9398,
-31567,-8788,-31567,-8788,-31567,-8788,-31567,-8788,
-31732,-8174,-31732,-8174,-31732,-8174,-31732,-8174,
-31884,-7557,-31884,-7557,-31884,-7557,-31884,-7557,
-32025,-6937,-32025,-6937,-32025,-6937,-32025,-6937,
-32153,-6315,-32153,-6315,-32153,-6315,-32153,-6315,
-32270,-5690,-32270,-5690,-32270,-5690,-32270,-5690,
-32374,-5064,-32374,-5064,-32374,-5064,-32374,-5064,
-32466,-4435,-32466,-4435,-32466,-4435,-32466,-4435,
-32546,-3805,-32546,-3805,-32546,-3805,-32546,-3805,
-32614,-3173,-32614,-3173,-32614,-3173,-32614,-3173,
-32669,-2540,-32669,-2540,-32669,-2540,-32669,-2540,
-32712,-1906,-32712,-1906,-32712,-1906,-32712,-1906,
-32743,-1271,-32743,-1271,-32743,-1271,-32743,-1271,
-32761,-636,-32761,-636,-32761,-636,-32761,-636,
-32767,-1,-32767,-1,-32767,-1,-32767,-1,
-32761,635,-32761,635,-32761,635,-32761,635,
-32743,1270,-32743,1270,-32743,1270,-32743,1270,
-32712,1905,-32712,1905,-32712,1905,-32712,1905,
-32669,2539,-32669,2539,-32669,2539,-32669,2539,
-32614,3172,-32614,3172,-32614,3172,-32614,3172,
-32546,3804,-32546,3804,-32546,3804,-32546,3804,
-32466,4434,-32466,4434,-32466,4434,-32466,4434,
-32374,5063,-32374,5063,-32374,5063,-32374,5063,
-32270,5689,-32270,5689,-32270,5689,-32270,5689,
-32153,6314,-32153,6314,-32153,6314,-32153,6314,
-32025,6936,-32025,6936,-32025,6936,-32025,6936,
-31884,7556,-31884,7556,-31884,7556,-31884,7556,
-31732,8173,-31732,8173,-31732,8173,-31732,8173,
-31567,8787,-31567,8787,-31567,8787,-31567,8787,
-31391,9397,-31391,9397,-31391,9397,-31391,9397,
-31203,10004,-31203,10004,-31203,10004,-31203,10004,
-31003,10607,-31003,10607,-31003,10607,-31003,10607,
-30791,11206,-30791,11206,-30791,11206,-30791,11206,
-30568,11801,-30568,11801,-30568,11801,-30568,11801,
-30334,12392,-30334,12392,-30334,12392,-30334,12392,
-30088,12978,-30088,12978,-30088,12978,-30088,12978,
-29830,13559,-29830,13559,-29830,13559,-29830,13559,
-29562,14135,-29562,14135,-29562,14135,-29562,14135,
-29282,14705,-29282,14705,-29282,14705,-29282,14705,
-28991,15270,-28991,15270,-28991,15270,-28991,15270,
-28690,15830,-28690,15830,-28690,15830,-28690,15830,
-28378,16383,-28378,16383,-28378,16383,-28378,16383,
-28055,16930,-28055,16930,-28055,16930,-28055,16930,
-27721,17471,-27721,17471,-27721,17471,-27721,17471,
-27377,18005,-27377,18005,-27377,18005,-27377,18005,
-27023,18533,-27023,18533,-27023,18533,-27023,18533,
-26658,19053,-26658,19053,-26658,19053,-26658,19053,
-26284,19567,-26284,19567,-26284,19567,-26284,19567,
-25899,20073,-25899,20073,-25899,20073,-25899,20073,
-25505,20571,-25505,20571,-25505,20571,-25505,20571,
-25101,21062,-25101,21062,-25101,21062,-25101,21062,
-24688,21545,-24688,21545,-24688,21545,-24688,21545,
-24266,22019,-24266,22019,-24266,22019,-24266,22019,
-23834,22486,-23834,22486,-23834,22486,-23834,22486,
-23394,22944,-23394,22944,-23394,22944,-23394,22944,
-22945,23393,-22945,23393,-22945,23393,-22945,23393,
-22487,23833,-22487,23833,-22487,23833,-22487,23833,
-22020,24265,-22020,24265,-22020,24265,-22020,24265,
-21546,24687,-21546,24687,-21546,24687,-21546,24687,
-21063,25100,-21063,25100,-21063,25100,-21063,25100,
-20572,25504,-20572,25504,-20572,25504,-20572,25504,
-20074,25898,-20074,25898,-20074,25898,-20074,25898,
-19568,26283,-19568,26283,-19568,26283,-19568,26283,
-19054,26657,-19054,26657,-19054,26657,-19054,26657,
-18534,27022,-18534,27022,-18534,27022,-18534,27022,
-18006,27376,-18006,27376,-18006,27376,-18006,27376,
-17472,27720,-17472,27720,-17472,27720,-17472,27720,
-16931,28054,-16931,28054,-16931,28054,-16931,28054};

void dft648(int16_t *x,int16_t *y,unsigned char scale_flag){ // 216 x 3
  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa648[0];
  __m128i *twb128=(__m128i *)&twb648[0];
  __m128i x2128[648];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[648];//=&ytmp128array3[0];



  for (i=0,j=0;i<216;i++,j+=3) {
    x2128[i]    = x128[j];
    x2128[i+216] = x128[j+1];
    x2128[i+432] = x128[j+2];
  }

  dft216((int16_t *)x2128,(int16_t *)ytmp128,1);
  dft216((int16_t *)(x2128+216),(int16_t *)(ytmp128+216),1);
  dft216((int16_t *)(x2128+432),(int16_t *)(ytmp128+432),1);

  bfly3_tw1(ytmp128,ytmp128+216,ytmp128+432,y128,y128+216,y128+432);
  for (i=1,j=0;i<216;i++,j++) {
    bfly3(ytmp128+i,
	  ytmp128+216+i,
	  ytmp128+432+i,
	  y128+i,
	  y128+216+i,
	  y128+432+i,
	  twa128+j,
	  twb128+j);
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(dft_norm_table[14]);
    
    for (i=0;i<648;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

};


/* Twiddles generated with
twa = floor(32767*exp(-sqrt(-1)*2*pi*(1:179)/720));
twb = floor(32767*exp(-sqrt(-1)*2*pi*2*(1:179)/720));
twc = floor(32767*exp(-sqrt(-1)*2*pi*3*(1:179)/720));
twa2 = zeros(1,2*179);
twb2 = zeros(1,2*179);
twc2 = zeros(1,2*179);
twa2(1:2:end) = real(twa);
twa2(2:2:end) = imag(twa);
twb2(1:2:end) = real(twb);
twb2(2:2:end) = imag(twb);
twc2(1:2:end) = real(twc);
twc2(2:2:end) = imag(twc);
fd=fopen("twiddle_tmp.txt","w");
fprintf(fd,"static int16_t twa720[179*2*4] = {");
for i=1:2:(2*178)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
fprintf(fd,"\nstatic int16_t twb720[179*2*4] = {");
for i=1:2:(2*178)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
fprintf(fd,"\nstatic int16_t twc720[179*2*4] = {");
for i=1:2:(2*178)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twc2(i),twc2(i+1),twc2(i),twc2(i+1),twc2(i),twc2(i+1),twc2(i),twc2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twc2(i),twc2(i+1),twc2(i),twc2(i+1),twc2(i),twc2(i+1),twc2(i),twc2(i+1));
fclose(fd);
 */

static int16_t twa720[179*2*4] = {32765,-286,32765,-286,32765,-286,32765,-286,
32762,-572,32762,-572,32762,-572,32762,-572,
32755,-858,32755,-858,32755,-858,32755,-858,
32747,-1144,32747,-1144,32747,-1144,32747,-1144,
32735,-1430,32735,-1430,32735,-1430,32735,-1430,
32722,-1715,32722,-1715,32722,-1715,32722,-1715,
32705,-2001,32705,-2001,32705,-2001,32705,-2001,
32687,-2286,32687,-2286,32687,-2286,32687,-2286,
32665,-2571,32665,-2571,32665,-2571,32665,-2571,
32642,-2856,32642,-2856,32642,-2856,32642,-2856,
32616,-3141,32616,-3141,32616,-3141,32616,-3141,
32587,-3426,32587,-3426,32587,-3426,32587,-3426,
32556,-3710,32556,-3710,32556,-3710,32556,-3710,
32522,-3994,32522,-3994,32522,-3994,32522,-3994,
32486,-4277,32486,-4277,32486,-4277,32486,-4277,
32448,-4561,32448,-4561,32448,-4561,32448,-4561,
32407,-4844,32407,-4844,32407,-4844,32407,-4844,
32363,-5126,32363,-5126,32363,-5126,32363,-5126,
32317,-5409,32317,-5409,32317,-5409,32317,-5409,
32269,-5690,32269,-5690,32269,-5690,32269,-5690,
32218,-5972,32218,-5972,32218,-5972,32218,-5972,
32164,-6253,32164,-6253,32164,-6253,32164,-6253,
32109,-6533,32109,-6533,32109,-6533,32109,-6533,
32050,-6813,32050,-6813,32050,-6813,32050,-6813,
31990,-7093,31990,-7093,31990,-7093,31990,-7093,
31927,-7371,31927,-7371,31927,-7371,31927,-7371,
31861,-7650,31861,-7650,31861,-7650,31861,-7650,
31793,-7928,31793,-7928,31793,-7928,31793,-7928,
31723,-8205,31723,-8205,31723,-8205,31723,-8205,
31650,-8481,31650,-8481,31650,-8481,31650,-8481,
31575,-8757,31575,-8757,31575,-8757,31575,-8757,
31497,-9032,31497,-9032,31497,-9032,31497,-9032,
31417,-9307,31417,-9307,31417,-9307,31417,-9307,
31335,-9581,31335,-9581,31335,-9581,31335,-9581,
31250,-9854,31250,-9854,31250,-9854,31250,-9854,
31163,-10126,31163,-10126,31163,-10126,31163,-10126,
31073,-10398,31073,-10398,31073,-10398,31073,-10398,
30981,-10668,30981,-10668,30981,-10668,30981,-10668,
30887,-10938,30887,-10938,30887,-10938,30887,-10938,
30790,-11207,30790,-11207,30790,-11207,30790,-11207,
30691,-11476,30691,-11476,30691,-11476,30691,-11476,
30590,-11743,30590,-11743,30590,-11743,30590,-11743,
30486,-12010,30486,-12010,30486,-12010,30486,-12010,
30381,-12275,30381,-12275,30381,-12275,30381,-12275,
30272,-12540,30272,-12540,30272,-12540,30272,-12540,
30162,-12804,30162,-12804,30162,-12804,30162,-12804,
30049,-13066,30049,-13066,30049,-13066,30049,-13066,
29934,-13328,29934,-13328,29934,-13328,29934,-13328,
29816,-13589,29816,-13589,29816,-13589,29816,-13589,
29696,-13848,29696,-13848,29696,-13848,29696,-13848,
29575,-14107,29575,-14107,29575,-14107,29575,-14107,
29450,-14365,29450,-14365,29450,-14365,29450,-14365,
29324,-14621,29324,-14621,29324,-14621,29324,-14621,
29195,-14876,29195,-14876,29195,-14876,29195,-14876,
29064,-15131,29064,-15131,29064,-15131,29064,-15131,
28931,-15384,28931,-15384,28931,-15384,28931,-15384,
28796,-15636,28796,-15636,28796,-15636,28796,-15636,
28658,-15886,28658,-15886,28658,-15886,28658,-15886,
28518,-16136,28518,-16136,28518,-16136,28518,-16136,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
28233,-16631,28233,-16631,28233,-16631,28233,-16631,
28086,-16877,28086,-16877,28086,-16877,28086,-16877,
27938,-17121,27938,-17121,27938,-17121,27938,-17121,
27787,-17364,27787,-17364,27787,-17364,27787,-17364,
27635,-17606,27635,-17606,27635,-17606,27635,-17606,
27480,-17847,27480,-17847,27480,-17847,27480,-17847,
27323,-18086,27323,-18086,27323,-18086,27323,-18086,
27165,-18324,27165,-18324,27165,-18324,27165,-18324,
27004,-18560,27004,-18560,27004,-18560,27004,-18560,
26841,-18795,26841,-18795,26841,-18795,26841,-18795,
26676,-19028,26676,-19028,26676,-19028,26676,-19028,
26509,-19260,26509,-19260,26509,-19260,26509,-19260,
26339,-19491,26339,-19491,26339,-19491,26339,-19491,
26168,-19720,26168,-19720,26168,-19720,26168,-19720,
25995,-19948,25995,-19948,25995,-19948,25995,-19948,
25820,-20174,25820,-20174,25820,-20174,25820,-20174,
25643,-20398,25643,-20398,25643,-20398,25643,-20398,
25464,-20621,25464,-20621,25464,-20621,25464,-20621,
25283,-20843,25283,-20843,25283,-20843,25283,-20843,
25100,-21063,25100,-21063,25100,-21063,25100,-21063,
24916,-21281,24916,-21281,24916,-21281,24916,-21281,
24729,-21498,24729,-21498,24729,-21498,24729,-21498,
24541,-21713,24541,-21713,24541,-21713,24541,-21713,
24350,-21926,24350,-21926,24350,-21926,24350,-21926,
24158,-22138,24158,-22138,24158,-22138,24158,-22138,
23964,-22348,23964,-22348,23964,-22348,23964,-22348,
23768,-22556,23768,-22556,23768,-22556,23768,-22556,
23570,-22762,23570,-22762,23570,-22762,23570,-22762,
23371,-22967,23371,-22967,23371,-22967,23371,-22967,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
22966,-23372,22966,-23372,22966,-23372,22966,-23372,
22761,-23571,22761,-23571,22761,-23571,22761,-23571,
22555,-23769,22555,-23769,22555,-23769,22555,-23769,
22347,-23965,22347,-23965,22347,-23965,22347,-23965,
22137,-24159,22137,-24159,22137,-24159,22137,-24159,
21925,-24351,21925,-24351,21925,-24351,21925,-24351,
21712,-24542,21712,-24542,21712,-24542,21712,-24542,
21497,-24730,21497,-24730,21497,-24730,21497,-24730,
21280,-24917,21280,-24917,21280,-24917,21280,-24917,
21062,-25101,21062,-25101,21062,-25101,21062,-25101,
20842,-25284,20842,-25284,20842,-25284,20842,-25284,
20620,-25465,20620,-25465,20620,-25465,20620,-25465,
20397,-25644,20397,-25644,20397,-25644,20397,-25644,
20173,-25821,20173,-25821,20173,-25821,20173,-25821,
19947,-25996,19947,-25996,19947,-25996,19947,-25996,
19719,-26169,19719,-26169,19719,-26169,19719,-26169,
19490,-26340,19490,-26340,19490,-26340,19490,-26340,
19259,-26510,19259,-26510,19259,-26510,19259,-26510,
19027,-26677,19027,-26677,19027,-26677,19027,-26677,
18794,-26842,18794,-26842,18794,-26842,18794,-26842,
18559,-27005,18559,-27005,18559,-27005,18559,-27005,
18323,-27166,18323,-27166,18323,-27166,18323,-27166,
18085,-27324,18085,-27324,18085,-27324,18085,-27324,
17846,-27481,17846,-27481,17846,-27481,17846,-27481,
17605,-27636,17605,-27636,17605,-27636,17605,-27636,
17363,-27788,17363,-27788,17363,-27788,17363,-27788,
17120,-27939,17120,-27939,17120,-27939,17120,-27939,
16876,-28087,16876,-28087,16876,-28087,16876,-28087,
16630,-28234,16630,-28234,16630,-28234,16630,-28234,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
16135,-28519,16135,-28519,16135,-28519,16135,-28519,
15885,-28659,15885,-28659,15885,-28659,15885,-28659,
15635,-28797,15635,-28797,15635,-28797,15635,-28797,
15383,-28932,15383,-28932,15383,-28932,15383,-28932,
15130,-29065,15130,-29065,15130,-29065,15130,-29065,
14875,-29196,14875,-29196,14875,-29196,14875,-29196,
14620,-29325,14620,-29325,14620,-29325,14620,-29325,
14364,-29451,14364,-29451,14364,-29451,14364,-29451,
14106,-29576,14106,-29576,14106,-29576,14106,-29576,
13847,-29697,13847,-29697,13847,-29697,13847,-29697,
13588,-29817,13588,-29817,13588,-29817,13588,-29817,
13327,-29935,13327,-29935,13327,-29935,13327,-29935,
13065,-30050,13065,-30050,13065,-30050,13065,-30050,
12803,-30163,12803,-30163,12803,-30163,12803,-30163,
12539,-30273,12539,-30273,12539,-30273,12539,-30273,
12274,-30382,12274,-30382,12274,-30382,12274,-30382,
12009,-30487,12009,-30487,12009,-30487,12009,-30487,
11742,-30591,11742,-30591,11742,-30591,11742,-30591,
11475,-30692,11475,-30692,11475,-30692,11475,-30692,
11206,-30791,11206,-30791,11206,-30791,11206,-30791,
10937,-30888,10937,-30888,10937,-30888,10937,-30888,
10667,-30982,10667,-30982,10667,-30982,10667,-30982,
10397,-31074,10397,-31074,10397,-31074,10397,-31074,
10125,-31164,10125,-31164,10125,-31164,10125,-31164,
9853,-31251,9853,-31251,9853,-31251,9853,-31251,
9580,-31336,9580,-31336,9580,-31336,9580,-31336,
9306,-31418,9306,-31418,9306,-31418,9306,-31418,
9031,-31498,9031,-31498,9031,-31498,9031,-31498,
8756,-31576,8756,-31576,8756,-31576,8756,-31576,
8480,-31651,8480,-31651,8480,-31651,8480,-31651,
8204,-31724,8204,-31724,8204,-31724,8204,-31724,
7927,-31794,7927,-31794,7927,-31794,7927,-31794,
7649,-31862,7649,-31862,7649,-31862,7649,-31862,
7370,-31928,7370,-31928,7370,-31928,7370,-31928,
7092,-31991,7092,-31991,7092,-31991,7092,-31991,
6812,-32051,6812,-32051,6812,-32051,6812,-32051,
6532,-32110,6532,-32110,6532,-32110,6532,-32110,
6252,-32165,6252,-32165,6252,-32165,6252,-32165,
5971,-32219,5971,-32219,5971,-32219,5971,-32219,
5689,-32270,5689,-32270,5689,-32270,5689,-32270,
5408,-32318,5408,-32318,5408,-32318,5408,-32318,
5125,-32364,5125,-32364,5125,-32364,5125,-32364,
4843,-32408,4843,-32408,4843,-32408,4843,-32408,
4560,-32449,4560,-32449,4560,-32449,4560,-32449,
4276,-32487,4276,-32487,4276,-32487,4276,-32487,
3993,-32523,3993,-32523,3993,-32523,3993,-32523,
3709,-32557,3709,-32557,3709,-32557,3709,-32557,
3425,-32588,3425,-32588,3425,-32588,3425,-32588,
3140,-32617,3140,-32617,3140,-32617,3140,-32617,
2855,-32643,2855,-32643,2855,-32643,2855,-32643,
2570,-32666,2570,-32666,2570,-32666,2570,-32666,
2285,-32688,2285,-32688,2285,-32688,2285,-32688,
2000,-32706,2000,-32706,2000,-32706,2000,-32706,
1714,-32723,1714,-32723,1714,-32723,1714,-32723,
1429,-32736,1429,-32736,1429,-32736,1429,-32736,
1143,-32748,1143,-32748,1143,-32748,1143,-32748,
857,-32756,857,-32756,857,-32756,857,-32756,
571,-32763,571,-32763,571,-32763,571,-32763,
285,-32766,285,-32766,285,-32766,285,-32766};

static int16_t twb720[179*2*4] = {32762,-572,32762,-572,32762,-572,32762,-572,
32747,-1144,32747,-1144,32747,-1144,32747,-1144,
32722,-1715,32722,-1715,32722,-1715,32722,-1715,
32687,-2286,32687,-2286,32687,-2286,32687,-2286,
32642,-2856,32642,-2856,32642,-2856,32642,-2856,
32587,-3426,32587,-3426,32587,-3426,32587,-3426,
32522,-3994,32522,-3994,32522,-3994,32522,-3994,
32448,-4561,32448,-4561,32448,-4561,32448,-4561,
32363,-5126,32363,-5126,32363,-5126,32363,-5126,
32269,-5690,32269,-5690,32269,-5690,32269,-5690,
32164,-6253,32164,-6253,32164,-6253,32164,-6253,
32050,-6813,32050,-6813,32050,-6813,32050,-6813,
31927,-7371,31927,-7371,31927,-7371,31927,-7371,
31793,-7928,31793,-7928,31793,-7928,31793,-7928,
31650,-8481,31650,-8481,31650,-8481,31650,-8481,
31497,-9032,31497,-9032,31497,-9032,31497,-9032,
31335,-9581,31335,-9581,31335,-9581,31335,-9581,
31163,-10126,31163,-10126,31163,-10126,31163,-10126,
30981,-10668,30981,-10668,30981,-10668,30981,-10668,
30790,-11207,30790,-11207,30790,-11207,30790,-11207,
30590,-11743,30590,-11743,30590,-11743,30590,-11743,
30381,-12275,30381,-12275,30381,-12275,30381,-12275,
30162,-12804,30162,-12804,30162,-12804,30162,-12804,
29934,-13328,29934,-13328,29934,-13328,29934,-13328,
29696,-13848,29696,-13848,29696,-13848,29696,-13848,
29450,-14365,29450,-14365,29450,-14365,29450,-14365,
29195,-14876,29195,-14876,29195,-14876,29195,-14876,
28931,-15384,28931,-15384,28931,-15384,28931,-15384,
28658,-15886,28658,-15886,28658,-15886,28658,-15886,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
28086,-16877,28086,-16877,28086,-16877,28086,-16877,
27787,-17364,27787,-17364,27787,-17364,27787,-17364,
27480,-17847,27480,-17847,27480,-17847,27480,-17847,
27165,-18324,27165,-18324,27165,-18324,27165,-18324,
26841,-18795,26841,-18795,26841,-18795,26841,-18795,
26509,-19260,26509,-19260,26509,-19260,26509,-19260,
26168,-19720,26168,-19720,26168,-19720,26168,-19720,
25820,-20174,25820,-20174,25820,-20174,25820,-20174,
25464,-20621,25464,-20621,25464,-20621,25464,-20621,
25100,-21063,25100,-21063,25100,-21063,25100,-21063,
24729,-21498,24729,-21498,24729,-21498,24729,-21498,
24350,-21926,24350,-21926,24350,-21926,24350,-21926,
23964,-22348,23964,-22348,23964,-22348,23964,-22348,
23570,-22762,23570,-22762,23570,-22762,23570,-22762,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
22761,-23571,22761,-23571,22761,-23571,22761,-23571,
22347,-23965,22347,-23965,22347,-23965,22347,-23965,
21925,-24351,21925,-24351,21925,-24351,21925,-24351,
21497,-24730,21497,-24730,21497,-24730,21497,-24730,
21062,-25101,21062,-25101,21062,-25101,21062,-25101,
20620,-25465,20620,-25465,20620,-25465,20620,-25465,
20173,-25821,20173,-25821,20173,-25821,20173,-25821,
19719,-26169,19719,-26169,19719,-26169,19719,-26169,
19259,-26510,19259,-26510,19259,-26510,19259,-26510,
18794,-26842,18794,-26842,18794,-26842,18794,-26842,
18323,-27166,18323,-27166,18323,-27166,18323,-27166,
17846,-27481,17846,-27481,17846,-27481,17846,-27481,
17363,-27788,17363,-27788,17363,-27788,17363,-27788,
16876,-28087,16876,-28087,16876,-28087,16876,-28087,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
15885,-28659,15885,-28659,15885,-28659,15885,-28659,
15383,-28932,15383,-28932,15383,-28932,15383,-28932,
14875,-29196,14875,-29196,14875,-29196,14875,-29196,
14364,-29451,14364,-29451,14364,-29451,14364,-29451,
13847,-29697,13847,-29697,13847,-29697,13847,-29697,
13327,-29935,13327,-29935,13327,-29935,13327,-29935,
12803,-30163,12803,-30163,12803,-30163,12803,-30163,
12274,-30382,12274,-30382,12274,-30382,12274,-30382,
11742,-30591,11742,-30591,11742,-30591,11742,-30591,
11206,-30791,11206,-30791,11206,-30791,11206,-30791,
10667,-30982,10667,-30982,10667,-30982,10667,-30982,
10125,-31164,10125,-31164,10125,-31164,10125,-31164,
9580,-31336,9580,-31336,9580,-31336,9580,-31336,
9031,-31498,9031,-31498,9031,-31498,9031,-31498,
8480,-31651,8480,-31651,8480,-31651,8480,-31651,
7927,-31794,7927,-31794,7927,-31794,7927,-31794,
7370,-31928,7370,-31928,7370,-31928,7370,-31928,
6812,-32051,6812,-32051,6812,-32051,6812,-32051,
6252,-32165,6252,-32165,6252,-32165,6252,-32165,
5689,-32270,5689,-32270,5689,-32270,5689,-32270,
5125,-32364,5125,-32364,5125,-32364,5125,-32364,
4560,-32449,4560,-32449,4560,-32449,4560,-32449,
3993,-32523,3993,-32523,3993,-32523,3993,-32523,
3425,-32588,3425,-32588,3425,-32588,3425,-32588,
2855,-32643,2855,-32643,2855,-32643,2855,-32643,
2285,-32688,2285,-32688,2285,-32688,2285,-32688,
1714,-32723,1714,-32723,1714,-32723,1714,-32723,
1143,-32748,1143,-32748,1143,-32748,1143,-32748,
571,-32763,571,-32763,571,-32763,571,-32763,
0,-32767,0,-32767,0,-32767,0,-32767,
-572,-32763,-572,-32763,-572,-32763,-572,-32763,
-1144,-32748,-1144,-32748,-1144,-32748,-1144,-32748,
-1715,-32723,-1715,-32723,-1715,-32723,-1715,-32723,
-2286,-32688,-2286,-32688,-2286,-32688,-2286,-32688,
-2856,-32643,-2856,-32643,-2856,-32643,-2856,-32643,
-3426,-32588,-3426,-32588,-3426,-32588,-3426,-32588,
-3994,-32523,-3994,-32523,-3994,-32523,-3994,-32523,
-4561,-32449,-4561,-32449,-4561,-32449,-4561,-32449,
-5126,-32364,-5126,-32364,-5126,-32364,-5126,-32364,
-5690,-32270,-5690,-32270,-5690,-32270,-5690,-32270,
-6253,-32165,-6253,-32165,-6253,-32165,-6253,-32165,
-6813,-32051,-6813,-32051,-6813,-32051,-6813,-32051,
-7371,-31928,-7371,-31928,-7371,-31928,-7371,-31928,
-7928,-31794,-7928,-31794,-7928,-31794,-7928,-31794,
-8481,-31651,-8481,-31651,-8481,-31651,-8481,-31651,
-9032,-31498,-9032,-31498,-9032,-31498,-9032,-31498,
-9581,-31336,-9581,-31336,-9581,-31336,-9581,-31336,
-10126,-31164,-10126,-31164,-10126,-31164,-10126,-31164,
-10668,-30982,-10668,-30982,-10668,-30982,-10668,-30982,
-11207,-30791,-11207,-30791,-11207,-30791,-11207,-30791,
-11743,-30591,-11743,-30591,-11743,-30591,-11743,-30591,
-12275,-30382,-12275,-30382,-12275,-30382,-12275,-30382,
-12804,-30163,-12804,-30163,-12804,-30163,-12804,-30163,
-13328,-29935,-13328,-29935,-13328,-29935,-13328,-29935,
-13848,-29697,-13848,-29697,-13848,-29697,-13848,-29697,
-14365,-29451,-14365,-29451,-14365,-29451,-14365,-29451,
-14876,-29196,-14876,-29196,-14876,-29196,-14876,-29196,
-15384,-28932,-15384,-28932,-15384,-28932,-15384,-28932,
-15886,-28659,-15886,-28659,-15886,-28659,-15886,-28659,
-16384,-28378,-16384,-28378,-16384,-28378,-16384,-28378,
-16877,-28087,-16877,-28087,-16877,-28087,-16877,-28087,
-17364,-27788,-17364,-27788,-17364,-27788,-17364,-27788,
-17847,-27481,-17847,-27481,-17847,-27481,-17847,-27481,
-18324,-27166,-18324,-27166,-18324,-27166,-18324,-27166,
-18795,-26842,-18795,-26842,-18795,-26842,-18795,-26842,
-19260,-26510,-19260,-26510,-19260,-26510,-19260,-26510,
-19720,-26169,-19720,-26169,-19720,-26169,-19720,-26169,
-20174,-25821,-20174,-25821,-20174,-25821,-20174,-25821,
-20621,-25465,-20621,-25465,-20621,-25465,-20621,-25465,
-21063,-25101,-21063,-25101,-21063,-25101,-21063,-25101,
-21498,-24730,-21498,-24730,-21498,-24730,-21498,-24730,
-21926,-24351,-21926,-24351,-21926,-24351,-21926,-24351,
-22348,-23965,-22348,-23965,-22348,-23965,-22348,-23965,
-22762,-23571,-22762,-23571,-22762,-23571,-22762,-23571,
-23170,-23170,-23170,-23170,-23170,-23170,-23170,-23170,
-23571,-22762,-23571,-22762,-23571,-22762,-23571,-22762,
-23965,-22348,-23965,-22348,-23965,-22348,-23965,-22348,
-24351,-21926,-24351,-21926,-24351,-21926,-24351,-21926,
-24730,-21498,-24730,-21498,-24730,-21498,-24730,-21498,
-25101,-21063,-25101,-21063,-25101,-21063,-25101,-21063,
-25465,-20621,-25465,-20621,-25465,-20621,-25465,-20621,
-25821,-20174,-25821,-20174,-25821,-20174,-25821,-20174,
-26169,-19720,-26169,-19720,-26169,-19720,-26169,-19720,
-26510,-19260,-26510,-19260,-26510,-19260,-26510,-19260,
-26842,-18795,-26842,-18795,-26842,-18795,-26842,-18795,
-27166,-18324,-27166,-18324,-27166,-18324,-27166,-18324,
-27481,-17847,-27481,-17847,-27481,-17847,-27481,-17847,
-27788,-17364,-27788,-17364,-27788,-17364,-27788,-17364,
-28087,-16877,-28087,-16877,-28087,-16877,-28087,-16877,
-28378,-16384,-28378,-16384,-28378,-16384,-28378,-16384,
-28659,-15886,-28659,-15886,-28659,-15886,-28659,-15886,
-28932,-15384,-28932,-15384,-28932,-15384,-28932,-15384,
-29196,-14876,-29196,-14876,-29196,-14876,-29196,-14876,
-29451,-14365,-29451,-14365,-29451,-14365,-29451,-14365,
-29697,-13848,-29697,-13848,-29697,-13848,-29697,-13848,
-29935,-13328,-29935,-13328,-29935,-13328,-29935,-13328,
-30163,-12804,-30163,-12804,-30163,-12804,-30163,-12804,
-30382,-12275,-30382,-12275,-30382,-12275,-30382,-12275,
-30591,-11743,-30591,-11743,-30591,-11743,-30591,-11743,
-30791,-11207,-30791,-11207,-30791,-11207,-30791,-11207,
-30982,-10668,-30982,-10668,-30982,-10668,-30982,-10668,
-31164,-10126,-31164,-10126,-31164,-10126,-31164,-10126,
-31336,-9581,-31336,-9581,-31336,-9581,-31336,-9581,
-31498,-9032,-31498,-9032,-31498,-9032,-31498,-9032,
-31651,-8481,-31651,-8481,-31651,-8481,-31651,-8481,
-31794,-7928,-31794,-7928,-31794,-7928,-31794,-7928,
-31928,-7371,-31928,-7371,-31928,-7371,-31928,-7371,
-32051,-6813,-32051,-6813,-32051,-6813,-32051,-6813,
-32165,-6253,-32165,-6253,-32165,-6253,-32165,-6253,
-32270,-5690,-32270,-5690,-32270,-5690,-32270,-5690,
-32364,-5126,-32364,-5126,-32364,-5126,-32364,-5126,
-32449,-4561,-32449,-4561,-32449,-4561,-32449,-4561,
-32523,-3994,-32523,-3994,-32523,-3994,-32523,-3994,
-32588,-3426,-32588,-3426,-32588,-3426,-32588,-3426,
-32643,-2856,-32643,-2856,-32643,-2856,-32643,-2856,
-32688,-2286,-32688,-2286,-32688,-2286,-32688,-2286,
-32723,-1715,-32723,-1715,-32723,-1715,-32723,-1715,
-32748,-1144,-32748,-1144,-32748,-1144,-32748,-1144,
-32763,-572,-32763,-572,-32763,-572,-32763,-572};

static int16_t twc720[179*2*4] = {32755,-858,32755,-858,32755,-858,32755,-858,
32722,-1715,32722,-1715,32722,-1715,32722,-1715,
32665,-2571,32665,-2571,32665,-2571,32665,-2571,
32587,-3426,32587,-3426,32587,-3426,32587,-3426,
32486,-4277,32486,-4277,32486,-4277,32486,-4277,
32363,-5126,32363,-5126,32363,-5126,32363,-5126,
32218,-5972,32218,-5972,32218,-5972,32218,-5972,
32050,-6813,32050,-6813,32050,-6813,32050,-6813,
31861,-7650,31861,-7650,31861,-7650,31861,-7650,
31650,-8481,31650,-8481,31650,-8481,31650,-8481,
31417,-9307,31417,-9307,31417,-9307,31417,-9307,
31163,-10126,31163,-10126,31163,-10126,31163,-10126,
30887,-10938,30887,-10938,30887,-10938,30887,-10938,
30590,-11743,30590,-11743,30590,-11743,30590,-11743,
30272,-12540,30272,-12540,30272,-12540,30272,-12540,
29934,-13328,29934,-13328,29934,-13328,29934,-13328,
29575,-14107,29575,-14107,29575,-14107,29575,-14107,
29195,-14876,29195,-14876,29195,-14876,29195,-14876,
28796,-15636,28796,-15636,28796,-15636,28796,-15636,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
27938,-17121,27938,-17121,27938,-17121,27938,-17121,
27480,-17847,27480,-17847,27480,-17847,27480,-17847,
27004,-18560,27004,-18560,27004,-18560,27004,-18560,
26509,-19260,26509,-19260,26509,-19260,26509,-19260,
25995,-19948,25995,-19948,25995,-19948,25995,-19948,
25464,-20621,25464,-20621,25464,-20621,25464,-20621,
24916,-21281,24916,-21281,24916,-21281,24916,-21281,
24350,-21926,24350,-21926,24350,-21926,24350,-21926,
23768,-22556,23768,-22556,23768,-22556,23768,-22556,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
22555,-23769,22555,-23769,22555,-23769,22555,-23769,
21925,-24351,21925,-24351,21925,-24351,21925,-24351,
21280,-24917,21280,-24917,21280,-24917,21280,-24917,
20620,-25465,20620,-25465,20620,-25465,20620,-25465,
19947,-25996,19947,-25996,19947,-25996,19947,-25996,
19259,-26510,19259,-26510,19259,-26510,19259,-26510,
18559,-27005,18559,-27005,18559,-27005,18559,-27005,
17846,-27481,17846,-27481,17846,-27481,17846,-27481,
17120,-27939,17120,-27939,17120,-27939,17120,-27939,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
15635,-28797,15635,-28797,15635,-28797,15635,-28797,
14875,-29196,14875,-29196,14875,-29196,14875,-29196,
14106,-29576,14106,-29576,14106,-29576,14106,-29576,
13327,-29935,13327,-29935,13327,-29935,13327,-29935,
12539,-30273,12539,-30273,12539,-30273,12539,-30273,
11742,-30591,11742,-30591,11742,-30591,11742,-30591,
10937,-30888,10937,-30888,10937,-30888,10937,-30888,
10125,-31164,10125,-31164,10125,-31164,10125,-31164,
9306,-31418,9306,-31418,9306,-31418,9306,-31418,
8480,-31651,8480,-31651,8480,-31651,8480,-31651,
7649,-31862,7649,-31862,7649,-31862,7649,-31862,
6812,-32051,6812,-32051,6812,-32051,6812,-32051,
5971,-32219,5971,-32219,5971,-32219,5971,-32219,
5125,-32364,5125,-32364,5125,-32364,5125,-32364,
4276,-32487,4276,-32487,4276,-32487,4276,-32487,
3425,-32588,3425,-32588,3425,-32588,3425,-32588,
2570,-32666,2570,-32666,2570,-32666,2570,-32666,
1714,-32723,1714,-32723,1714,-32723,1714,-32723,
857,-32756,857,-32756,857,-32756,857,-32756,
0,-32767,0,-32767,0,-32767,0,-32767,
-858,-32756,-858,-32756,-858,-32756,-858,-32756,
-1715,-32723,-1715,-32723,-1715,-32723,-1715,-32723,
-2571,-32666,-2571,-32666,-2571,-32666,-2571,-32666,
-3426,-32588,-3426,-32588,-3426,-32588,-3426,-32588,
-4277,-32487,-4277,-32487,-4277,-32487,-4277,-32487,
-5126,-32364,-5126,-32364,-5126,-32364,-5126,-32364,
-5972,-32219,-5972,-32219,-5972,-32219,-5972,-32219,
-6813,-32051,-6813,-32051,-6813,-32051,-6813,-32051,
-7650,-31862,-7650,-31862,-7650,-31862,-7650,-31862,
-8481,-31651,-8481,-31651,-8481,-31651,-8481,-31651,
-9307,-31418,-9307,-31418,-9307,-31418,-9307,-31418,
-10126,-31164,-10126,-31164,-10126,-31164,-10126,-31164,
-10938,-30888,-10938,-30888,-10938,-30888,-10938,-30888,
-11743,-30591,-11743,-30591,-11743,-30591,-11743,-30591,
-12540,-30273,-12540,-30273,-12540,-30273,-12540,-30273,
-13328,-29935,-13328,-29935,-13328,-29935,-13328,-29935,
-14107,-29576,-14107,-29576,-14107,-29576,-14107,-29576,
-14876,-29196,-14876,-29196,-14876,-29196,-14876,-29196,
-15636,-28797,-15636,-28797,-15636,-28797,-15636,-28797,
-16384,-28378,-16384,-28378,-16384,-28378,-16384,-28378,
-17121,-27939,-17121,-27939,-17121,-27939,-17121,-27939,
-17847,-27481,-17847,-27481,-17847,-27481,-17847,-27481,
-18560,-27005,-18560,-27005,-18560,-27005,-18560,-27005,
-19260,-26510,-19260,-26510,-19260,-26510,-19260,-26510,
-19948,-25996,-19948,-25996,-19948,-25996,-19948,-25996,
-20621,-25465,-20621,-25465,-20621,-25465,-20621,-25465,
-21281,-24917,-21281,-24917,-21281,-24917,-21281,-24917,
-21926,-24351,-21926,-24351,-21926,-24351,-21926,-24351,
-22556,-23769,-22556,-23769,-22556,-23769,-22556,-23769,
-23170,-23170,-23170,-23170,-23170,-23170,-23170,-23170,
-23769,-22556,-23769,-22556,-23769,-22556,-23769,-22556,
-24351,-21926,-24351,-21926,-24351,-21926,-24351,-21926,
-24917,-21281,-24917,-21281,-24917,-21281,-24917,-21281,
-25465,-20621,-25465,-20621,-25465,-20621,-25465,-20621,
-25996,-19948,-25996,-19948,-25996,-19948,-25996,-19948,
-26510,-19260,-26510,-19260,-26510,-19260,-26510,-19260,
-27005,-18560,-27005,-18560,-27005,-18560,-27005,-18560,
-27481,-17847,-27481,-17847,-27481,-17847,-27481,-17847,
-27939,-17121,-27939,-17121,-27939,-17121,-27939,-17121,
-28378,-16384,-28378,-16384,-28378,-16384,-28378,-16384,
-28797,-15636,-28797,-15636,-28797,-15636,-28797,-15636,
-29196,-14876,-29196,-14876,-29196,-14876,-29196,-14876,
-29576,-14107,-29576,-14107,-29576,-14107,-29576,-14107,
-29935,-13328,-29935,-13328,-29935,-13328,-29935,-13328,
-30273,-12540,-30273,-12540,-30273,-12540,-30273,-12540,
-30591,-11743,-30591,-11743,-30591,-11743,-30591,-11743,
-30888,-10938,-30888,-10938,-30888,-10938,-30888,-10938,
-31164,-10126,-31164,-10126,-31164,-10126,-31164,-10126,
-31418,-9307,-31418,-9307,-31418,-9307,-31418,-9307,
-31651,-8481,-31651,-8481,-31651,-8481,-31651,-8481,
-31862,-7650,-31862,-7650,-31862,-7650,-31862,-7650,
-32051,-6813,-32051,-6813,-32051,-6813,-32051,-6813,
-32219,-5972,-32219,-5972,-32219,-5972,-32219,-5972,
-32364,-5126,-32364,-5126,-32364,-5126,-32364,-5126,
-32487,-4277,-32487,-4277,-32487,-4277,-32487,-4277,
-32588,-3426,-32588,-3426,-32588,-3426,-32588,-3426,
-32666,-2571,-32666,-2571,-32666,-2571,-32666,-2571,
-32723,-1715,-32723,-1715,-32723,-1715,-32723,-1715,
-32756,-858,-32756,-858,-32756,-858,-32756,-858,
-32767,-1,-32767,-1,-32767,-1,-32767,-1,
-32756,857,-32756,857,-32756,857,-32756,857,
-32723,1714,-32723,1714,-32723,1714,-32723,1714,
-32666,2570,-32666,2570,-32666,2570,-32666,2570,
-32588,3425,-32588,3425,-32588,3425,-32588,3425,
-32487,4276,-32487,4276,-32487,4276,-32487,4276,
-32364,5125,-32364,5125,-32364,5125,-32364,5125,
-32219,5971,-32219,5971,-32219,5971,-32219,5971,
-32051,6812,-32051,6812,-32051,6812,-32051,6812,
-31862,7649,-31862,7649,-31862,7649,-31862,7649,
-31651,8480,-31651,8480,-31651,8480,-31651,8480,
-31418,9306,-31418,9306,-31418,9306,-31418,9306,
-31164,10125,-31164,10125,-31164,10125,-31164,10125,
-30888,10937,-30888,10937,-30888,10937,-30888,10937,
-30591,11742,-30591,11742,-30591,11742,-30591,11742,
-30273,12539,-30273,12539,-30273,12539,-30273,12539,
-29935,13327,-29935,13327,-29935,13327,-29935,13327,
-29576,14106,-29576,14106,-29576,14106,-29576,14106,
-29196,14875,-29196,14875,-29196,14875,-29196,14875,
-28797,15635,-28797,15635,-28797,15635,-28797,15635,
-28378,16383,-28378,16383,-28378,16383,-28378,16383,
-27939,17120,-27939,17120,-27939,17120,-27939,17120,
-27481,17846,-27481,17846,-27481,17846,-27481,17846,
-27005,18559,-27005,18559,-27005,18559,-27005,18559,
-26510,19259,-26510,19259,-26510,19259,-26510,19259,
-25996,19947,-25996,19947,-25996,19947,-25996,19947,
-25465,20620,-25465,20620,-25465,20620,-25465,20620,
-24917,21280,-24917,21280,-24917,21280,-24917,21280,
-24351,21925,-24351,21925,-24351,21925,-24351,21925,
-23769,22555,-23769,22555,-23769,22555,-23769,22555,
-23170,23169,-23170,23169,-23170,23169,-23170,23169,
-22556,23768,-22556,23768,-22556,23768,-22556,23768,
-21926,24350,-21926,24350,-21926,24350,-21926,24350,
-21281,24916,-21281,24916,-21281,24916,-21281,24916,
-20621,25464,-20621,25464,-20621,25464,-20621,25464,
-19948,25995,-19948,25995,-19948,25995,-19948,25995,
-19260,26509,-19260,26509,-19260,26509,-19260,26509,
-18560,27004,-18560,27004,-18560,27004,-18560,27004,
-17847,27480,-17847,27480,-17847,27480,-17847,27480,
-17121,27938,-17121,27938,-17121,27938,-17121,27938,
-16384,28377,-16384,28377,-16384,28377,-16384,28377,
-15636,28796,-15636,28796,-15636,28796,-15636,28796,
-14876,29195,-14876,29195,-14876,29195,-14876,29195,
-14107,29575,-14107,29575,-14107,29575,-14107,29575,
-13328,29934,-13328,29934,-13328,29934,-13328,29934,
-12540,30272,-12540,30272,-12540,30272,-12540,30272,
-11743,30590,-11743,30590,-11743,30590,-11743,30590,
-10938,30887,-10938,30887,-10938,30887,-10938,30887,
-10126,31163,-10126,31163,-10126,31163,-10126,31163,
-9307,31417,-9307,31417,-9307,31417,-9307,31417,
-8481,31650,-8481,31650,-8481,31650,-8481,31650,
-7650,31861,-7650,31861,-7650,31861,-7650,31861,
-6813,32050,-6813,32050,-6813,32050,-6813,32050,
-5972,32218,-5972,32218,-5972,32218,-5972,32218,
-5126,32363,-5126,32363,-5126,32363,-5126,32363,
-4277,32486,-4277,32486,-4277,32486,-4277,32486,
-3426,32587,-3426,32587,-3426,32587,-3426,32587,
-2571,32665,-2571,32665,-2571,32665,-2571,32665,
-1715,32722,-1715,32722,-1715,32722,-1715,32722,
-858,32755,-858,32755,-858,32755,-858,32755};

void dft720(int16_t *x,int16_t *y,unsigned char scale_flag){ // 180 x 4
  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa720[0];
  __m128i *twb128=(__m128i *)&twb720[0];
  __m128i *twc128=(__m128i *)&twc720[0];
  __m128i x2128[720];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[720];//=&ytmp128array2[0];



  for (i=0,j=0;i<180;i++,j+=4) {
    x2128[i]    = x128[j];
    x2128[i+180] = x128[j+1];
    x2128[i+360] = x128[j+2];
    x2128[i+540] = x128[j+3];
  }

  dft180((int16_t *)x2128,(int16_t *)ytmp128,1);
  dft180((int16_t *)(x2128+180),(int16_t *)(ytmp128+180),1);
  dft180((int16_t *)(x2128+360),(int16_t *)(ytmp128+360),1);
  dft180((int16_t *)(x2128+540),(int16_t *)(ytmp128+540),1);

  bfly4_tw1(ytmp128,ytmp128+180,ytmp128+360,ytmp128+540,y128,y128+180,y128+360,y128+540);
  for (i=1,j=0;i<180;i++,j++) {
    bfly4(ytmp128+i,
	  ytmp128+180+i,
	  ytmp128+360+i,
	  ytmp128+540+i,
	  y128+i,
	  y128+180+i,
	  y128+360+i,
	  y128+540+i,
	  twa128+j,
	  twb128+j,
	  twc128+j);
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(16384);//dft_norm_table[13]);
    
    for (i=0;i<720;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

};

/* Twiddles generated with
twa = floor(32767*exp(-sqrt(-1)*2*pi*(1:287)/864));
twb = floor(32767*exp(-sqrt(-1)*2*pi*2*(1:287)/864));
twa2 = zeros(1,2*287);
twb2 = zeros(1,2*287);
twa2(1:2:end) = real(twa);
twa2(2:2:end) = imag(twa);
twb2(1:2:end) = real(twb);
twb2(2:2:end) = imag(twb);
fd=fopen("twiddle_tmp.txt","w");
fprintf(fd,"static int16_t twa864[287*2*4] = {");
for i=1:2:(2*286)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
fprintf(fd,"static int16_t twb864[287*2*4] = {");
for i=1:2:(2*286)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
fclose(fd);
*/
static int16_t twa864[287*2*4] = {32766,-239,32766,-239,32766,-239,32766,-239,
32763,-477,32763,-477,32763,-477,32763,-477,
32759,-715,32759,-715,32759,-715,32759,-715,
32753,-954,32753,-954,32753,-954,32753,-954,
32745,-1192,32745,-1192,32745,-1192,32745,-1192,
32735,-1430,32735,-1430,32735,-1430,32735,-1430,
32724,-1668,32724,-1668,32724,-1668,32724,-1668,
32711,-1906,32711,-1906,32711,-1906,32711,-1906,
32696,-2144,32696,-2144,32696,-2144,32696,-2144,
32680,-2381,32680,-2381,32680,-2381,32680,-2381,
32662,-2619,32662,-2619,32662,-2619,32662,-2619,
32642,-2856,32642,-2856,32642,-2856,32642,-2856,
32620,-3094,32620,-3094,32620,-3094,32620,-3094,
32597,-3331,32597,-3331,32597,-3331,32597,-3331,
32572,-3568,32572,-3568,32572,-3568,32572,-3568,
32545,-3805,32545,-3805,32545,-3805,32545,-3805,
32516,-4041,32516,-4041,32516,-4041,32516,-4041,
32486,-4277,32486,-4277,32486,-4277,32486,-4277,
32454,-4514,32454,-4514,32454,-4514,32454,-4514,
32421,-4749,32421,-4749,32421,-4749,32421,-4749,
32385,-4985,32385,-4985,32385,-4985,32385,-4985,
32348,-5221,32348,-5221,32348,-5221,32348,-5221,
32309,-5456,32309,-5456,32309,-5456,32309,-5456,
32269,-5690,32269,-5690,32269,-5690,32269,-5690,
32226,-5925,32226,-5925,32226,-5925,32226,-5925,
32183,-6159,32183,-6159,32183,-6159,32183,-6159,
32137,-6393,32137,-6393,32137,-6393,32137,-6393,
32090,-6627,32090,-6627,32090,-6627,32090,-6627,
32041,-6860,32041,-6860,32041,-6860,32041,-6860,
31990,-7093,31990,-7093,31990,-7093,31990,-7093,
31937,-7325,31937,-7325,31937,-7325,31937,-7325,
31883,-7557,31883,-7557,31883,-7557,31883,-7557,
31827,-7789,31827,-7789,31827,-7789,31827,-7789,
31770,-8020,31770,-8020,31770,-8020,31770,-8020,
31711,-8251,31711,-8251,31711,-8251,31711,-8251,
31650,-8481,31650,-8481,31650,-8481,31650,-8481,
31587,-8711,31587,-8711,31587,-8711,31587,-8711,
31523,-8941,31523,-8941,31523,-8941,31523,-8941,
31457,-9170,31457,-9170,31457,-9170,31457,-9170,
31390,-9398,31390,-9398,31390,-9398,31390,-9398,
31321,-9626,31321,-9626,31321,-9626,31321,-9626,
31250,-9854,31250,-9854,31250,-9854,31250,-9854,
31177,-10081,31177,-10081,31177,-10081,31177,-10081,
31103,-10307,31103,-10307,31103,-10307,31103,-10307,
31028,-10533,31028,-10533,31028,-10533,31028,-10533,
30950,-10758,30950,-10758,30950,-10758,30950,-10758,
30871,-10983,30871,-10983,30871,-10983,30871,-10983,
30790,-11207,30790,-11207,30790,-11207,30790,-11207,
30708,-11431,30708,-11431,30708,-11431,30708,-11431,
30624,-11654,30624,-11654,30624,-11654,30624,-11654,
30539,-11877,30539,-11877,30539,-11877,30539,-11877,
30451,-12098,30451,-12098,30451,-12098,30451,-12098,
30363,-12319,30363,-12319,30363,-12319,30363,-12319,
30272,-12540,30272,-12540,30272,-12540,30272,-12540,
30180,-12760,30180,-12760,30180,-12760,30180,-12760,
30087,-12979,30087,-12979,30087,-12979,30087,-12979,
29992,-13197,29992,-13197,29992,-13197,29992,-13197,
29895,-13415,29895,-13415,29895,-13415,29895,-13415,
29796,-13632,29796,-13632,29796,-13632,29796,-13632,
29696,-13848,29696,-13848,29696,-13848,29696,-13848,
29595,-14064,29595,-14064,29595,-14064,29595,-14064,
29492,-14279,29492,-14279,29492,-14279,29492,-14279,
29387,-14493,29387,-14493,29387,-14493,29387,-14493,
29281,-14706,29281,-14706,29281,-14706,29281,-14706,
29173,-14919,29173,-14919,29173,-14919,29173,-14919,
29064,-15131,29064,-15131,29064,-15131,29064,-15131,
28953,-15342,28953,-15342,28953,-15342,28953,-15342,
28841,-15552,28841,-15552,28841,-15552,28841,-15552,
28727,-15761,28727,-15761,28727,-15761,28727,-15761,
28612,-15970,28612,-15970,28612,-15970,28612,-15970,
28495,-16177,28495,-16177,28495,-16177,28495,-16177,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
28257,-16590,28257,-16590,28257,-16590,28257,-16590,
28135,-16795,28135,-16795,28135,-16795,28135,-16795,
28012,-16999,28012,-16999,28012,-16999,28012,-16999,
27888,-17202,27888,-17202,27888,-17202,27888,-17202,
27762,-17405,27762,-17405,27762,-17405,27762,-17405,
27635,-17606,27635,-17606,27635,-17606,27635,-17606,
27506,-17807,27506,-17807,27506,-17807,27506,-17807,
27376,-18006,27376,-18006,27376,-18006,27376,-18006,
27244,-18205,27244,-18205,27244,-18205,27244,-18205,
27111,-18403,27111,-18403,27111,-18403,27111,-18403,
26977,-18599,26977,-18599,26977,-18599,26977,-18599,
26841,-18795,26841,-18795,26841,-18795,26841,-18795,
26703,-18990,26703,-18990,26703,-18990,26703,-18990,
26564,-19183,26564,-19183,26564,-19183,26564,-19183,
26424,-19376,26424,-19376,26424,-19376,26424,-19376,
26283,-19568,26283,-19568,26283,-19568,26283,-19568,
26140,-19758,26140,-19758,26140,-19758,26140,-19758,
25995,-19948,25995,-19948,25995,-19948,25995,-19948,
25850,-20136,25850,-20136,25850,-20136,25850,-20136,
25702,-20324,25702,-20324,25702,-20324,25702,-20324,
25554,-20510,25554,-20510,25554,-20510,25554,-20510,
25404,-20695,25404,-20695,25404,-20695,25404,-20695,
25253,-20880,25253,-20880,25253,-20880,25253,-20880,
25100,-21063,25100,-21063,25100,-21063,25100,-21063,
24947,-21245,24947,-21245,24947,-21245,24947,-21245,
24791,-21426,24791,-21426,24791,-21426,24791,-21426,
24635,-21605,24635,-21605,24635,-21605,24635,-21605,
24477,-21784,24477,-21784,24477,-21784,24477,-21784,
24318,-21961,24318,-21961,24318,-21961,24318,-21961,
24158,-22138,24158,-22138,24158,-22138,24158,-22138,
23996,-22313,23996,-22313,23996,-22313,23996,-22313,
23833,-22487,23833,-22487,23833,-22487,23833,-22487,
23669,-22659,23669,-22659,23669,-22659,23669,-22659,
23504,-22831,23504,-22831,23504,-22831,23504,-22831,
23337,-23001,23337,-23001,23337,-23001,23337,-23001,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
23000,-23338,23000,-23338,23000,-23338,23000,-23338,
22830,-23505,22830,-23505,22830,-23505,22830,-23505,
22658,-23670,22658,-23670,22658,-23670,22658,-23670,
22486,-23834,22486,-23834,22486,-23834,22486,-23834,
22312,-23997,22312,-23997,22312,-23997,22312,-23997,
22137,-24159,22137,-24159,22137,-24159,22137,-24159,
21960,-24319,21960,-24319,21960,-24319,21960,-24319,
21783,-24478,21783,-24478,21783,-24478,21783,-24478,
21604,-24636,21604,-24636,21604,-24636,21604,-24636,
21425,-24792,21425,-24792,21425,-24792,21425,-24792,
21244,-24948,21244,-24948,21244,-24948,21244,-24948,
21062,-25101,21062,-25101,21062,-25101,21062,-25101,
20879,-25254,20879,-25254,20879,-25254,20879,-25254,
20694,-25405,20694,-25405,20694,-25405,20694,-25405,
20509,-25555,20509,-25555,20509,-25555,20509,-25555,
20323,-25703,20323,-25703,20323,-25703,20323,-25703,
20135,-25851,20135,-25851,20135,-25851,20135,-25851,
19947,-25996,19947,-25996,19947,-25996,19947,-25996,
19757,-26141,19757,-26141,19757,-26141,19757,-26141,
19567,-26284,19567,-26284,19567,-26284,19567,-26284,
19375,-26425,19375,-26425,19375,-26425,19375,-26425,
19182,-26565,19182,-26565,19182,-26565,19182,-26565,
18989,-26704,18989,-26704,18989,-26704,18989,-26704,
18794,-26842,18794,-26842,18794,-26842,18794,-26842,
18598,-26978,18598,-26978,18598,-26978,18598,-26978,
18402,-27112,18402,-27112,18402,-27112,18402,-27112,
18204,-27245,18204,-27245,18204,-27245,18204,-27245,
18005,-27377,18005,-27377,18005,-27377,18005,-27377,
17806,-27507,17806,-27507,17806,-27507,17806,-27507,
17605,-27636,17605,-27636,17605,-27636,17605,-27636,
17404,-27763,17404,-27763,17404,-27763,17404,-27763,
17201,-27889,17201,-27889,17201,-27889,17201,-27889,
16998,-28013,16998,-28013,16998,-28013,16998,-28013,
16794,-28136,16794,-28136,16794,-28136,16794,-28136,
16589,-28258,16589,-28258,16589,-28258,16589,-28258,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
16176,-28496,16176,-28496,16176,-28496,16176,-28496,
15969,-28613,15969,-28613,15969,-28613,15969,-28613,
15760,-28728,15760,-28728,15760,-28728,15760,-28728,
15551,-28842,15551,-28842,15551,-28842,15551,-28842,
15341,-28954,15341,-28954,15341,-28954,15341,-28954,
15130,-29065,15130,-29065,15130,-29065,15130,-29065,
14918,-29174,14918,-29174,14918,-29174,14918,-29174,
14705,-29282,14705,-29282,14705,-29282,14705,-29282,
14492,-29388,14492,-29388,14492,-29388,14492,-29388,
14278,-29493,14278,-29493,14278,-29493,14278,-29493,
14063,-29596,14063,-29596,14063,-29596,14063,-29596,
13847,-29697,13847,-29697,13847,-29697,13847,-29697,
13631,-29797,13631,-29797,13631,-29797,13631,-29797,
13414,-29896,13414,-29896,13414,-29896,13414,-29896,
13196,-29993,13196,-29993,13196,-29993,13196,-29993,
12978,-30088,12978,-30088,12978,-30088,12978,-30088,
12759,-30181,12759,-30181,12759,-30181,12759,-30181,
12539,-30273,12539,-30273,12539,-30273,12539,-30273,
12318,-30364,12318,-30364,12318,-30364,12318,-30364,
12097,-30452,12097,-30452,12097,-30452,12097,-30452,
11876,-30540,11876,-30540,11876,-30540,11876,-30540,
11653,-30625,11653,-30625,11653,-30625,11653,-30625,
11430,-30709,11430,-30709,11430,-30709,11430,-30709,
11206,-30791,11206,-30791,11206,-30791,11206,-30791,
10982,-30872,10982,-30872,10982,-30872,10982,-30872,
10757,-30951,10757,-30951,10757,-30951,10757,-30951,
10532,-31029,10532,-31029,10532,-31029,10532,-31029,
10306,-31104,10306,-31104,10306,-31104,10306,-31104,
10080,-31178,10080,-31178,10080,-31178,10080,-31178,
9853,-31251,9853,-31251,9853,-31251,9853,-31251,
9625,-31322,9625,-31322,9625,-31322,9625,-31322,
9397,-31391,9397,-31391,9397,-31391,9397,-31391,
9169,-31458,9169,-31458,9169,-31458,9169,-31458,
8940,-31524,8940,-31524,8940,-31524,8940,-31524,
8710,-31588,8710,-31588,8710,-31588,8710,-31588,
8480,-31651,8480,-31651,8480,-31651,8480,-31651,
8250,-31712,8250,-31712,8250,-31712,8250,-31712,
8019,-31771,8019,-31771,8019,-31771,8019,-31771,
7788,-31828,7788,-31828,7788,-31828,7788,-31828,
7556,-31884,7556,-31884,7556,-31884,7556,-31884,
7324,-31938,7324,-31938,7324,-31938,7324,-31938,
7092,-31991,7092,-31991,7092,-31991,7092,-31991,
6859,-32042,6859,-32042,6859,-32042,6859,-32042,
6626,-32091,6626,-32091,6626,-32091,6626,-32091,
6392,-32138,6392,-32138,6392,-32138,6392,-32138,
6158,-32184,6158,-32184,6158,-32184,6158,-32184,
5924,-32227,5924,-32227,5924,-32227,5924,-32227,
5689,-32270,5689,-32270,5689,-32270,5689,-32270,
5455,-32310,5455,-32310,5455,-32310,5455,-32310,
5220,-32349,5220,-32349,5220,-32349,5220,-32349,
4984,-32386,4984,-32386,4984,-32386,4984,-32386,
4748,-32422,4748,-32422,4748,-32422,4748,-32422,
4513,-32455,4513,-32455,4513,-32455,4513,-32455,
4276,-32487,4276,-32487,4276,-32487,4276,-32487,
4040,-32517,4040,-32517,4040,-32517,4040,-32517,
3804,-32546,3804,-32546,3804,-32546,3804,-32546,
3567,-32573,3567,-32573,3567,-32573,3567,-32573,
3330,-32598,3330,-32598,3330,-32598,3330,-32598,
3093,-32621,3093,-32621,3093,-32621,3093,-32621,
2855,-32643,2855,-32643,2855,-32643,2855,-32643,
2618,-32663,2618,-32663,2618,-32663,2618,-32663,
2380,-32681,2380,-32681,2380,-32681,2380,-32681,
2143,-32697,2143,-32697,2143,-32697,2143,-32697,
1905,-32712,1905,-32712,1905,-32712,1905,-32712,
1667,-32725,1667,-32725,1667,-32725,1667,-32725,
1429,-32736,1429,-32736,1429,-32736,1429,-32736,
1191,-32746,1191,-32746,1191,-32746,1191,-32746,
953,-32754,953,-32754,953,-32754,953,-32754,
714,-32760,714,-32760,714,-32760,714,-32760,
476,-32764,476,-32764,476,-32764,476,-32764,
238,-32767,238,-32767,238,-32767,238,-32767,
0,-32767,0,-32767,0,-32767,0,-32767,
-239,-32767,-239,-32767,-239,-32767,-239,-32767,
-477,-32764,-477,-32764,-477,-32764,-477,-32764,
-715,-32760,-715,-32760,-715,-32760,-715,-32760,
-954,-32754,-954,-32754,-954,-32754,-954,-32754,
-1192,-32746,-1192,-32746,-1192,-32746,-1192,-32746,
-1430,-32736,-1430,-32736,-1430,-32736,-1430,-32736,
-1668,-32725,-1668,-32725,-1668,-32725,-1668,-32725,
-1906,-32712,-1906,-32712,-1906,-32712,-1906,-32712,
-2144,-32697,-2144,-32697,-2144,-32697,-2144,-32697,
-2381,-32681,-2381,-32681,-2381,-32681,-2381,-32681,
-2619,-32663,-2619,-32663,-2619,-32663,-2619,-32663,
-2856,-32643,-2856,-32643,-2856,-32643,-2856,-32643,
-3094,-32621,-3094,-32621,-3094,-32621,-3094,-32621,
-3331,-32598,-3331,-32598,-3331,-32598,-3331,-32598,
-3568,-32573,-3568,-32573,-3568,-32573,-3568,-32573,
-3805,-32546,-3805,-32546,-3805,-32546,-3805,-32546,
-4041,-32517,-4041,-32517,-4041,-32517,-4041,-32517,
-4277,-32487,-4277,-32487,-4277,-32487,-4277,-32487,
-4514,-32455,-4514,-32455,-4514,-32455,-4514,-32455,
-4749,-32422,-4749,-32422,-4749,-32422,-4749,-32422,
-4985,-32386,-4985,-32386,-4985,-32386,-4985,-32386,
-5221,-32349,-5221,-32349,-5221,-32349,-5221,-32349,
-5456,-32310,-5456,-32310,-5456,-32310,-5456,-32310,
-5690,-32270,-5690,-32270,-5690,-32270,-5690,-32270,
-5925,-32227,-5925,-32227,-5925,-32227,-5925,-32227,
-6159,-32184,-6159,-32184,-6159,-32184,-6159,-32184,
-6393,-32138,-6393,-32138,-6393,-32138,-6393,-32138,
-6627,-32091,-6627,-32091,-6627,-32091,-6627,-32091,
-6860,-32042,-6860,-32042,-6860,-32042,-6860,-32042,
-7093,-31991,-7093,-31991,-7093,-31991,-7093,-31991,
-7325,-31938,-7325,-31938,-7325,-31938,-7325,-31938,
-7557,-31884,-7557,-31884,-7557,-31884,-7557,-31884,
-7789,-31828,-7789,-31828,-7789,-31828,-7789,-31828,
-8020,-31771,-8020,-31771,-8020,-31771,-8020,-31771,
-8251,-31712,-8251,-31712,-8251,-31712,-8251,-31712,
-8481,-31651,-8481,-31651,-8481,-31651,-8481,-31651,
-8711,-31588,-8711,-31588,-8711,-31588,-8711,-31588,
-8941,-31524,-8941,-31524,-8941,-31524,-8941,-31524,
-9170,-31458,-9170,-31458,-9170,-31458,-9170,-31458,
-9398,-31391,-9398,-31391,-9398,-31391,-9398,-31391,
-9626,-31322,-9626,-31322,-9626,-31322,-9626,-31322,
-9854,-31251,-9854,-31251,-9854,-31251,-9854,-31251,
-10081,-31178,-10081,-31178,-10081,-31178,-10081,-31178,
-10307,-31104,-10307,-31104,-10307,-31104,-10307,-31104,
-10533,-31029,-10533,-31029,-10533,-31029,-10533,-31029,
-10758,-30951,-10758,-30951,-10758,-30951,-10758,-30951,
-10983,-30872,-10983,-30872,-10983,-30872,-10983,-30872,
-11207,-30791,-11207,-30791,-11207,-30791,-11207,-30791,
-11431,-30709,-11431,-30709,-11431,-30709,-11431,-30709,
-11654,-30625,-11654,-30625,-11654,-30625,-11654,-30625,
-11877,-30540,-11877,-30540,-11877,-30540,-11877,-30540,
-12098,-30452,-12098,-30452,-12098,-30452,-12098,-30452,
-12319,-30364,-12319,-30364,-12319,-30364,-12319,-30364,
-12540,-30273,-12540,-30273,-12540,-30273,-12540,-30273,
-12760,-30181,-12760,-30181,-12760,-30181,-12760,-30181,
-12979,-30088,-12979,-30088,-12979,-30088,-12979,-30088,
-13197,-29993,-13197,-29993,-13197,-29993,-13197,-29993,
-13415,-29896,-13415,-29896,-13415,-29896,-13415,-29896,
-13632,-29797,-13632,-29797,-13632,-29797,-13632,-29797,
-13848,-29697,-13848,-29697,-13848,-29697,-13848,-29697,
-14064,-29596,-14064,-29596,-14064,-29596,-14064,-29596,
-14279,-29493,-14279,-29493,-14279,-29493,-14279,-29493,
-14493,-29388,-14493,-29388,-14493,-29388,-14493,-29388,
-14706,-29282,-14706,-29282,-14706,-29282,-14706,-29282,
-14919,-29174,-14919,-29174,-14919,-29174,-14919,-29174,
-15131,-29065,-15131,-29065,-15131,-29065,-15131,-29065,
-15342,-28954,-15342,-28954,-15342,-28954,-15342,-28954,
-15552,-28842,-15552,-28842,-15552,-28842,-15552,-28842,
-15761,-28728,-15761,-28728,-15761,-28728,-15761,-28728,
-15970,-28613,-15970,-28613,-15970,-28613,-15970,-28613,
-16177,-28496,-16177,-28496,-16177,-28496,-16177,-28496};
static int16_t twb864[287*2*4] = {32763,-477,32763,-477,32763,-477,32763,-477,
32753,-954,32753,-954,32753,-954,32753,-954,
32735,-1430,32735,-1430,32735,-1430,32735,-1430,
32711,-1906,32711,-1906,32711,-1906,32711,-1906,
32680,-2381,32680,-2381,32680,-2381,32680,-2381,
32642,-2856,32642,-2856,32642,-2856,32642,-2856,
32597,-3331,32597,-3331,32597,-3331,32597,-3331,
32545,-3805,32545,-3805,32545,-3805,32545,-3805,
32486,-4277,32486,-4277,32486,-4277,32486,-4277,
32421,-4749,32421,-4749,32421,-4749,32421,-4749,
32348,-5221,32348,-5221,32348,-5221,32348,-5221,
32269,-5690,32269,-5690,32269,-5690,32269,-5690,
32183,-6159,32183,-6159,32183,-6159,32183,-6159,
32090,-6627,32090,-6627,32090,-6627,32090,-6627,
31990,-7093,31990,-7093,31990,-7093,31990,-7093,
31883,-7557,31883,-7557,31883,-7557,31883,-7557,
31770,-8020,31770,-8020,31770,-8020,31770,-8020,
31650,-8481,31650,-8481,31650,-8481,31650,-8481,
31523,-8941,31523,-8941,31523,-8941,31523,-8941,
31390,-9398,31390,-9398,31390,-9398,31390,-9398,
31250,-9854,31250,-9854,31250,-9854,31250,-9854,
31103,-10307,31103,-10307,31103,-10307,31103,-10307,
30950,-10758,30950,-10758,30950,-10758,30950,-10758,
30790,-11207,30790,-11207,30790,-11207,30790,-11207,
30624,-11654,30624,-11654,30624,-11654,30624,-11654,
30451,-12098,30451,-12098,30451,-12098,30451,-12098,
30272,-12540,30272,-12540,30272,-12540,30272,-12540,
30087,-12979,30087,-12979,30087,-12979,30087,-12979,
29895,-13415,29895,-13415,29895,-13415,29895,-13415,
29696,-13848,29696,-13848,29696,-13848,29696,-13848,
29492,-14279,29492,-14279,29492,-14279,29492,-14279,
29281,-14706,29281,-14706,29281,-14706,29281,-14706,
29064,-15131,29064,-15131,29064,-15131,29064,-15131,
28841,-15552,28841,-15552,28841,-15552,28841,-15552,
28612,-15970,28612,-15970,28612,-15970,28612,-15970,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
28135,-16795,28135,-16795,28135,-16795,28135,-16795,
27888,-17202,27888,-17202,27888,-17202,27888,-17202,
27635,-17606,27635,-17606,27635,-17606,27635,-17606,
27376,-18006,27376,-18006,27376,-18006,27376,-18006,
27111,-18403,27111,-18403,27111,-18403,27111,-18403,
26841,-18795,26841,-18795,26841,-18795,26841,-18795,
26564,-19183,26564,-19183,26564,-19183,26564,-19183,
26283,-19568,26283,-19568,26283,-19568,26283,-19568,
25995,-19948,25995,-19948,25995,-19948,25995,-19948,
25702,-20324,25702,-20324,25702,-20324,25702,-20324,
25404,-20695,25404,-20695,25404,-20695,25404,-20695,
25100,-21063,25100,-21063,25100,-21063,25100,-21063,
24791,-21426,24791,-21426,24791,-21426,24791,-21426,
24477,-21784,24477,-21784,24477,-21784,24477,-21784,
24158,-22138,24158,-22138,24158,-22138,24158,-22138,
23833,-22487,23833,-22487,23833,-22487,23833,-22487,
23504,-22831,23504,-22831,23504,-22831,23504,-22831,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
22830,-23505,22830,-23505,22830,-23505,22830,-23505,
22486,-23834,22486,-23834,22486,-23834,22486,-23834,
22137,-24159,22137,-24159,22137,-24159,22137,-24159,
21783,-24478,21783,-24478,21783,-24478,21783,-24478,
21425,-24792,21425,-24792,21425,-24792,21425,-24792,
21062,-25101,21062,-25101,21062,-25101,21062,-25101,
20694,-25405,20694,-25405,20694,-25405,20694,-25405,
20323,-25703,20323,-25703,20323,-25703,20323,-25703,
19947,-25996,19947,-25996,19947,-25996,19947,-25996,
19567,-26284,19567,-26284,19567,-26284,19567,-26284,
19182,-26565,19182,-26565,19182,-26565,19182,-26565,
18794,-26842,18794,-26842,18794,-26842,18794,-26842,
18402,-27112,18402,-27112,18402,-27112,18402,-27112,
18005,-27377,18005,-27377,18005,-27377,18005,-27377,
17605,-27636,17605,-27636,17605,-27636,17605,-27636,
17201,-27889,17201,-27889,17201,-27889,17201,-27889,
16794,-28136,16794,-28136,16794,-28136,16794,-28136,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
15969,-28613,15969,-28613,15969,-28613,15969,-28613,
15551,-28842,15551,-28842,15551,-28842,15551,-28842,
15130,-29065,15130,-29065,15130,-29065,15130,-29065,
14705,-29282,14705,-29282,14705,-29282,14705,-29282,
14278,-29493,14278,-29493,14278,-29493,14278,-29493,
13847,-29697,13847,-29697,13847,-29697,13847,-29697,
13414,-29896,13414,-29896,13414,-29896,13414,-29896,
12978,-30088,12978,-30088,12978,-30088,12978,-30088,
12539,-30273,12539,-30273,12539,-30273,12539,-30273,
12097,-30452,12097,-30452,12097,-30452,12097,-30452,
11653,-30625,11653,-30625,11653,-30625,11653,-30625,
11206,-30791,11206,-30791,11206,-30791,11206,-30791,
10757,-30951,10757,-30951,10757,-30951,10757,-30951,
10306,-31104,10306,-31104,10306,-31104,10306,-31104,
9853,-31251,9853,-31251,9853,-31251,9853,-31251,
9397,-31391,9397,-31391,9397,-31391,9397,-31391,
8940,-31524,8940,-31524,8940,-31524,8940,-31524,
8480,-31651,8480,-31651,8480,-31651,8480,-31651,
8019,-31771,8019,-31771,8019,-31771,8019,-31771,
7556,-31884,7556,-31884,7556,-31884,7556,-31884,
7092,-31991,7092,-31991,7092,-31991,7092,-31991,
6626,-32091,6626,-32091,6626,-32091,6626,-32091,
6158,-32184,6158,-32184,6158,-32184,6158,-32184,
5689,-32270,5689,-32270,5689,-32270,5689,-32270,
5220,-32349,5220,-32349,5220,-32349,5220,-32349,
4748,-32422,4748,-32422,4748,-32422,4748,-32422,
4276,-32487,4276,-32487,4276,-32487,4276,-32487,
3804,-32546,3804,-32546,3804,-32546,3804,-32546,
3330,-32598,3330,-32598,3330,-32598,3330,-32598,
2855,-32643,2855,-32643,2855,-32643,2855,-32643,
2380,-32681,2380,-32681,2380,-32681,2380,-32681,
1905,-32712,1905,-32712,1905,-32712,1905,-32712,
1429,-32736,1429,-32736,1429,-32736,1429,-32736,
953,-32754,953,-32754,953,-32754,953,-32754,
476,-32764,476,-32764,476,-32764,476,-32764,
0,-32767,0,-32767,0,-32767,0,-32767,
-477,-32764,-477,-32764,-477,-32764,-477,-32764,
-954,-32754,-954,-32754,-954,-32754,-954,-32754,
-1430,-32736,-1430,-32736,-1430,-32736,-1430,-32736,
-1906,-32712,-1906,-32712,-1906,-32712,-1906,-32712,
-2381,-32681,-2381,-32681,-2381,-32681,-2381,-32681,
-2856,-32643,-2856,-32643,-2856,-32643,-2856,-32643,
-3331,-32598,-3331,-32598,-3331,-32598,-3331,-32598,
-3805,-32546,-3805,-32546,-3805,-32546,-3805,-32546,
-4277,-32487,-4277,-32487,-4277,-32487,-4277,-32487,
-4749,-32422,-4749,-32422,-4749,-32422,-4749,-32422,
-5221,-32349,-5221,-32349,-5221,-32349,-5221,-32349,
-5690,-32270,-5690,-32270,-5690,-32270,-5690,-32270,
-6159,-32184,-6159,-32184,-6159,-32184,-6159,-32184,
-6627,-32091,-6627,-32091,-6627,-32091,-6627,-32091,
-7093,-31991,-7093,-31991,-7093,-31991,-7093,-31991,
-7557,-31884,-7557,-31884,-7557,-31884,-7557,-31884,
-8020,-31771,-8020,-31771,-8020,-31771,-8020,-31771,
-8481,-31651,-8481,-31651,-8481,-31651,-8481,-31651,
-8941,-31524,-8941,-31524,-8941,-31524,-8941,-31524,
-9398,-31391,-9398,-31391,-9398,-31391,-9398,-31391,
-9854,-31251,-9854,-31251,-9854,-31251,-9854,-31251,
-10307,-31104,-10307,-31104,-10307,-31104,-10307,-31104,
-10758,-30951,-10758,-30951,-10758,-30951,-10758,-30951,
-11207,-30791,-11207,-30791,-11207,-30791,-11207,-30791,
-11654,-30625,-11654,-30625,-11654,-30625,-11654,-30625,
-12098,-30452,-12098,-30452,-12098,-30452,-12098,-30452,
-12540,-30273,-12540,-30273,-12540,-30273,-12540,-30273,
-12979,-30088,-12979,-30088,-12979,-30088,-12979,-30088,
-13415,-29896,-13415,-29896,-13415,-29896,-13415,-29896,
-13848,-29697,-13848,-29697,-13848,-29697,-13848,-29697,
-14279,-29493,-14279,-29493,-14279,-29493,-14279,-29493,
-14706,-29282,-14706,-29282,-14706,-29282,-14706,-29282,
-15131,-29065,-15131,-29065,-15131,-29065,-15131,-29065,
-15552,-28842,-15552,-28842,-15552,-28842,-15552,-28842,
-15970,-28613,-15970,-28613,-15970,-28613,-15970,-28613,
-16384,-28378,-16384,-28378,-16384,-28378,-16384,-28378,
-16795,-28136,-16795,-28136,-16795,-28136,-16795,-28136,
-17202,-27889,-17202,-27889,-17202,-27889,-17202,-27889,
-17606,-27636,-17606,-27636,-17606,-27636,-17606,-27636,
-18006,-27377,-18006,-27377,-18006,-27377,-18006,-27377,
-18403,-27112,-18403,-27112,-18403,-27112,-18403,-27112,
-18795,-26842,-18795,-26842,-18795,-26842,-18795,-26842,
-19183,-26565,-19183,-26565,-19183,-26565,-19183,-26565,
-19568,-26284,-19568,-26284,-19568,-26284,-19568,-26284,
-19948,-25996,-19948,-25996,-19948,-25996,-19948,-25996,
-20324,-25703,-20324,-25703,-20324,-25703,-20324,-25703,
-20695,-25405,-20695,-25405,-20695,-25405,-20695,-25405,
-21063,-25101,-21063,-25101,-21063,-25101,-21063,-25101,
-21426,-24792,-21426,-24792,-21426,-24792,-21426,-24792,
-21784,-24478,-21784,-24478,-21784,-24478,-21784,-24478,
-22138,-24159,-22138,-24159,-22138,-24159,-22138,-24159,
-22487,-23834,-22487,-23834,-22487,-23834,-22487,-23834,
-22831,-23505,-22831,-23505,-22831,-23505,-22831,-23505,
-23170,-23170,-23170,-23170,-23170,-23170,-23170,-23170,
-23505,-22831,-23505,-22831,-23505,-22831,-23505,-22831,
-23834,-22487,-23834,-22487,-23834,-22487,-23834,-22487,
-24159,-22138,-24159,-22138,-24159,-22138,-24159,-22138,
-24478,-21784,-24478,-21784,-24478,-21784,-24478,-21784,
-24792,-21426,-24792,-21426,-24792,-21426,-24792,-21426,
-25101,-21063,-25101,-21063,-25101,-21063,-25101,-21063,
-25405,-20695,-25405,-20695,-25405,-20695,-25405,-20695,
-25703,-20324,-25703,-20324,-25703,-20324,-25703,-20324,
-25996,-19948,-25996,-19948,-25996,-19948,-25996,-19948,
-26284,-19568,-26284,-19568,-26284,-19568,-26284,-19568,
-26565,-19183,-26565,-19183,-26565,-19183,-26565,-19183,
-26842,-18795,-26842,-18795,-26842,-18795,-26842,-18795,
-27112,-18403,-27112,-18403,-27112,-18403,-27112,-18403,
-27377,-18006,-27377,-18006,-27377,-18006,-27377,-18006,
-27636,-17606,-27636,-17606,-27636,-17606,-27636,-17606,
-27889,-17202,-27889,-17202,-27889,-17202,-27889,-17202,
-28136,-16795,-28136,-16795,-28136,-16795,-28136,-16795,
-28378,-16384,-28378,-16384,-28378,-16384,-28378,-16384,
-28613,-15970,-28613,-15970,-28613,-15970,-28613,-15970,
-28842,-15552,-28842,-15552,-28842,-15552,-28842,-15552,
-29065,-15131,-29065,-15131,-29065,-15131,-29065,-15131,
-29282,-14706,-29282,-14706,-29282,-14706,-29282,-14706,
-29493,-14279,-29493,-14279,-29493,-14279,-29493,-14279,
-29697,-13848,-29697,-13848,-29697,-13848,-29697,-13848,
-29896,-13415,-29896,-13415,-29896,-13415,-29896,-13415,
-30088,-12979,-30088,-12979,-30088,-12979,-30088,-12979,
-30273,-12540,-30273,-12540,-30273,-12540,-30273,-12540,
-30452,-12098,-30452,-12098,-30452,-12098,-30452,-12098,
-30625,-11654,-30625,-11654,-30625,-11654,-30625,-11654,
-30791,-11207,-30791,-11207,-30791,-11207,-30791,-11207,
-30951,-10758,-30951,-10758,-30951,-10758,-30951,-10758,
-31104,-10307,-31104,-10307,-31104,-10307,-31104,-10307,
-31251,-9854,-31251,-9854,-31251,-9854,-31251,-9854,
-31391,-9398,-31391,-9398,-31391,-9398,-31391,-9398,
-31524,-8941,-31524,-8941,-31524,-8941,-31524,-8941,
-31651,-8481,-31651,-8481,-31651,-8481,-31651,-8481,
-31771,-8020,-31771,-8020,-31771,-8020,-31771,-8020,
-31884,-7557,-31884,-7557,-31884,-7557,-31884,-7557,
-31991,-7093,-31991,-7093,-31991,-7093,-31991,-7093,
-32091,-6627,-32091,-6627,-32091,-6627,-32091,-6627,
-32184,-6159,-32184,-6159,-32184,-6159,-32184,-6159,
-32270,-5690,-32270,-5690,-32270,-5690,-32270,-5690,
-32349,-5221,-32349,-5221,-32349,-5221,-32349,-5221,
-32422,-4749,-32422,-4749,-32422,-4749,-32422,-4749,
-32487,-4277,-32487,-4277,-32487,-4277,-32487,-4277,
-32546,-3805,-32546,-3805,-32546,-3805,-32546,-3805,
-32598,-3331,-32598,-3331,-32598,-3331,-32598,-3331,
-32643,-2856,-32643,-2856,-32643,-2856,-32643,-2856,
-32681,-2381,-32681,-2381,-32681,-2381,-32681,-2381,
-32712,-1906,-32712,-1906,-32712,-1906,-32712,-1906,
-32736,-1430,-32736,-1430,-32736,-1430,-32736,-1430,
-32754,-954,-32754,-954,-32754,-954,-32754,-954,
-32764,-477,-32764,-477,-32764,-477,-32764,-477,
-32767,-1,-32767,-1,-32767,-1,-32767,-1,
-32764,476,-32764,476,-32764,476,-32764,476,
-32754,953,-32754,953,-32754,953,-32754,953,
-32736,1429,-32736,1429,-32736,1429,-32736,1429,
-32712,1905,-32712,1905,-32712,1905,-32712,1905,
-32681,2380,-32681,2380,-32681,2380,-32681,2380,
-32643,2855,-32643,2855,-32643,2855,-32643,2855,
-32598,3330,-32598,3330,-32598,3330,-32598,3330,
-32546,3804,-32546,3804,-32546,3804,-32546,3804,
-32487,4276,-32487,4276,-32487,4276,-32487,4276,
-32422,4748,-32422,4748,-32422,4748,-32422,4748,
-32349,5220,-32349,5220,-32349,5220,-32349,5220,
-32270,5689,-32270,5689,-32270,5689,-32270,5689,
-32184,6158,-32184,6158,-32184,6158,-32184,6158,
-32091,6626,-32091,6626,-32091,6626,-32091,6626,
-31991,7092,-31991,7092,-31991,7092,-31991,7092,
-31884,7556,-31884,7556,-31884,7556,-31884,7556,
-31771,8019,-31771,8019,-31771,8019,-31771,8019,
-31651,8480,-31651,8480,-31651,8480,-31651,8480,
-31524,8940,-31524,8940,-31524,8940,-31524,8940,
-31391,9397,-31391,9397,-31391,9397,-31391,9397,
-31251,9853,-31251,9853,-31251,9853,-31251,9853,
-31104,10306,-31104,10306,-31104,10306,-31104,10306,
-30951,10757,-30951,10757,-30951,10757,-30951,10757,
-30791,11206,-30791,11206,-30791,11206,-30791,11206,
-30625,11653,-30625,11653,-30625,11653,-30625,11653,
-30452,12097,-30452,12097,-30452,12097,-30452,12097,
-30273,12539,-30273,12539,-30273,12539,-30273,12539,
-30088,12978,-30088,12978,-30088,12978,-30088,12978,
-29896,13414,-29896,13414,-29896,13414,-29896,13414,
-29697,13847,-29697,13847,-29697,13847,-29697,13847,
-29493,14278,-29493,14278,-29493,14278,-29493,14278,
-29282,14705,-29282,14705,-29282,14705,-29282,14705,
-29065,15130,-29065,15130,-29065,15130,-29065,15130,
-28842,15551,-28842,15551,-28842,15551,-28842,15551,
-28613,15969,-28613,15969,-28613,15969,-28613,15969,
-28378,16383,-28378,16383,-28378,16383,-28378,16383,
-28136,16794,-28136,16794,-28136,16794,-28136,16794,
-27889,17201,-27889,17201,-27889,17201,-27889,17201,
-27636,17605,-27636,17605,-27636,17605,-27636,17605,
-27377,18005,-27377,18005,-27377,18005,-27377,18005,
-27112,18402,-27112,18402,-27112,18402,-27112,18402,
-26842,18794,-26842,18794,-26842,18794,-26842,18794,
-26565,19182,-26565,19182,-26565,19182,-26565,19182,
-26284,19567,-26284,19567,-26284,19567,-26284,19567,
-25996,19947,-25996,19947,-25996,19947,-25996,19947,
-25703,20323,-25703,20323,-25703,20323,-25703,20323,
-25405,20694,-25405,20694,-25405,20694,-25405,20694,
-25101,21062,-25101,21062,-25101,21062,-25101,21062,
-24792,21425,-24792,21425,-24792,21425,-24792,21425,
-24478,21783,-24478,21783,-24478,21783,-24478,21783,
-24159,22137,-24159,22137,-24159,22137,-24159,22137,
-23834,22486,-23834,22486,-23834,22486,-23834,22486,
-23505,22830,-23505,22830,-23505,22830,-23505,22830,
-23170,23169,-23170,23169,-23170,23169,-23170,23169,
-22831,23504,-22831,23504,-22831,23504,-22831,23504,
-22487,23833,-22487,23833,-22487,23833,-22487,23833,
-22138,24158,-22138,24158,-22138,24158,-22138,24158,
-21784,24477,-21784,24477,-21784,24477,-21784,24477,
-21426,24791,-21426,24791,-21426,24791,-21426,24791,
-21063,25100,-21063,25100,-21063,25100,-21063,25100,
-20695,25404,-20695,25404,-20695,25404,-20695,25404,
-20324,25702,-20324,25702,-20324,25702,-20324,25702,
-19948,25995,-19948,25995,-19948,25995,-19948,25995,
-19568,26283,-19568,26283,-19568,26283,-19568,26283,
-19183,26564,-19183,26564,-19183,26564,-19183,26564,
-18795,26841,-18795,26841,-18795,26841,-18795,26841,
-18403,27111,-18403,27111,-18403,27111,-18403,27111,
-18006,27376,-18006,27376,-18006,27376,-18006,27376,
-17606,27635,-17606,27635,-17606,27635,-17606,27635,
-17202,27888,-17202,27888,-17202,27888,-17202,27888,
-16795,28135,-16795,28135,-16795,28135,-16795,28135};

void dft864(int16_t *x,int16_t *y,unsigned char scale_flag){ // 288 x 3
  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa864[0];
  __m128i *twb128=(__m128i *)&twb864[0];
  __m128i x2128[864];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[864];//=&ytmp128array3[0];



  for (i=0,j=0;i<288;i++,j+=3) {
    x2128[i]    = x128[j];
    x2128[i+288] = x128[j+1];
    x2128[i+576] = x128[j+2];
  }

  dft288((int16_t *)x2128,(int16_t *)ytmp128,1);
  dft288((int16_t *)(x2128+288),(int16_t *)(ytmp128+288),1);
  dft288((int16_t *)(x2128+576),(int16_t *)(ytmp128+576),1);

  bfly3_tw1(ytmp128,ytmp128+288,ytmp128+576,y128,y128+288,y128+576);
  for (i=1,j=0;i<288;i++,j++) {
    bfly3(ytmp128+i,
	  ytmp128+288+i,
	  ytmp128+576+i,
	  y128+i,
	  y128+288+i,
	  y128+576+i,
	  twa128+j,
	  twb128+j);
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(dft_norm_table[14]);
    
    for (i=0;i<864;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

};

/* Twiddles generated with
twa = floor(32767*exp(-sqrt(-1)*2*pi*(1:299)/900));
twb = floor(32767*exp(-sqrt(-1)*2*pi*2*(1:299)/900));
twa2 = zeros(1,2*299);
twb2 = zeros(1,2*299);
twa2(1:2:end) = real(twa);
twa2(2:2:end) = imag(twa);
twb2(1:2:end) = real(twb);
twb2(2:2:end) = imag(twb);
fd=fopen("twiddle_tmp.txt","w");
fprintf(fd,"static int16_t twa900[299*2*4] = {");
for i=1:2:(2*298)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
fprintf(fd,"static int16_t twb900[299*2*4] = {");
for i=1:2:(2*298)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
fclose(fd);
*/
static int16_t twa900[299*2*4] = {32766,-229,32766,-229,32766,-229,32766,-229,
32763,-458,32763,-458,32763,-458,32763,-458,
32759,-687,32759,-687,32759,-687,32759,-687,
32754,-915,32754,-915,32754,-915,32754,-915,
32747,-1144,32747,-1144,32747,-1144,32747,-1144,
32738,-1373,32738,-1373,32738,-1373,32738,-1373,
32727,-1601,32727,-1601,32727,-1601,32727,-1601,
32715,-1830,32715,-1830,32715,-1830,32715,-1830,
32702,-2058,32702,-2058,32702,-2058,32702,-2058,
32687,-2286,32687,-2286,32687,-2286,32687,-2286,
32670,-2514,32670,-2514,32670,-2514,32670,-2514,
32652,-2742,32652,-2742,32652,-2742,32652,-2742,
32632,-2970,32632,-2970,32632,-2970,32632,-2970,
32610,-3198,32610,-3198,32610,-3198,32610,-3198,
32587,-3426,32587,-3426,32587,-3426,32587,-3426,
32562,-3653,32562,-3653,32562,-3653,32562,-3653,
32536,-3880,32536,-3880,32536,-3880,32536,-3880,
32508,-4107,32508,-4107,32508,-4107,32508,-4107,
32479,-4334,32479,-4334,32479,-4334,32479,-4334,
32448,-4561,32448,-4561,32448,-4561,32448,-4561,
32415,-4787,32415,-4787,32415,-4787,32415,-4787,
32381,-5013,32381,-5013,32381,-5013,32381,-5013,
32345,-5239,32345,-5239,32345,-5239,32345,-5239,
32308,-5465,32308,-5465,32308,-5465,32308,-5465,
32269,-5690,32269,-5690,32269,-5690,32269,-5690,
32228,-5916,32228,-5916,32228,-5916,32228,-5916,
32186,-6140,32186,-6140,32186,-6140,32186,-6140,
32142,-6365,32142,-6365,32142,-6365,32142,-6365,
32097,-6589,32097,-6589,32097,-6589,32097,-6589,
32050,-6813,32050,-6813,32050,-6813,32050,-6813,
32002,-7037,32002,-7037,32002,-7037,32002,-7037,
31952,-7260,31952,-7260,31952,-7260,31952,-7260,
31901,-7483,31901,-7483,31901,-7483,31901,-7483,
31848,-7705,31848,-7705,31848,-7705,31848,-7705,
31793,-7928,31793,-7928,31793,-7928,31793,-7928,
31737,-8149,31737,-8149,31737,-8149,31737,-8149,
31679,-8371,31679,-8371,31679,-8371,31679,-8371,
31620,-8592,31620,-8592,31620,-8592,31620,-8592,
31559,-8812,31559,-8812,31559,-8812,31559,-8812,
31497,-9032,31497,-9032,31497,-9032,31497,-9032,
31433,-9252,31433,-9252,31433,-9252,31433,-9252,
31368,-9471,31368,-9471,31368,-9471,31368,-9471,
31301,-9690,31301,-9690,31301,-9690,31301,-9690,
31233,-9908,31233,-9908,31233,-9908,31233,-9908,
31163,-10126,31163,-10126,31163,-10126,31163,-10126,
31091,-10343,31091,-10343,31091,-10343,31091,-10343,
31018,-10560,31018,-10560,31018,-10560,31018,-10560,
30944,-10776,30944,-10776,30944,-10776,30944,-10776,
30868,-10992,30868,-10992,30868,-10992,30868,-10992,
30790,-11207,30790,-11207,30790,-11207,30790,-11207,
30711,-11422,30711,-11422,30711,-11422,30711,-11422,
30631,-11636,30631,-11636,30631,-11636,30631,-11636,
30549,-11850,30549,-11850,30549,-11850,30549,-11850,
30465,-12063,30465,-12063,30465,-12063,30465,-12063,
30381,-12275,30381,-12275,30381,-12275,30381,-12275,
30294,-12487,30294,-12487,30294,-12487,30294,-12487,
30206,-12698,30206,-12698,30206,-12698,30206,-12698,
30117,-12909,30117,-12909,30117,-12909,30117,-12909,
30026,-13119,30026,-13119,30026,-13119,30026,-13119,
29934,-13328,29934,-13328,29934,-13328,29934,-13328,
29840,-13537,29840,-13537,29840,-13537,29840,-13537,
29745,-13745,29745,-13745,29745,-13745,29745,-13745,
29648,-13952,29648,-13952,29648,-13952,29648,-13952,
29550,-14159,29550,-14159,29550,-14159,29550,-14159,
29450,-14365,29450,-14365,29450,-14365,29450,-14365,
29349,-14570,29349,-14570,29349,-14570,29349,-14570,
29247,-14774,29247,-14774,29247,-14774,29247,-14774,
29143,-14978,29143,-14978,29143,-14978,29143,-14978,
29038,-15181,29038,-15181,29038,-15181,29038,-15181,
28931,-15384,28931,-15384,28931,-15384,28931,-15384,
28823,-15585,28823,-15585,28823,-15585,28823,-15585,
28713,-15786,28713,-15786,28713,-15786,28713,-15786,
28603,-15986,28603,-15986,28603,-15986,28603,-15986,
28490,-16185,28490,-16185,28490,-16185,28490,-16185,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
28261,-16582,28261,-16582,28261,-16582,28261,-16582,
28145,-16779,28145,-16779,28145,-16779,28145,-16779,
28027,-16975,28027,-16975,28027,-16975,28027,-16975,
27908,-17170,27908,-17170,27908,-17170,27908,-17170,
27787,-17364,27787,-17364,27787,-17364,27787,-17364,
27666,-17558,27666,-17558,27666,-17558,27666,-17558,
27542,-17751,27542,-17751,27542,-17751,27542,-17751,
27418,-17943,27418,-17943,27418,-17943,27418,-17943,
27292,-18133,27292,-18133,27292,-18133,27292,-18133,
27165,-18324,27165,-18324,27165,-18324,27165,-18324,
27036,-18513,27036,-18513,27036,-18513,27036,-18513,
26906,-18701,26906,-18701,26906,-18701,26906,-18701,
26775,-18888,26775,-18888,26775,-18888,26775,-18888,
26642,-19075,26642,-19075,26642,-19075,26642,-19075,
26509,-19260,26509,-19260,26509,-19260,26509,-19260,
26373,-19445,26373,-19445,26373,-19445,26373,-19445,
26237,-19629,26237,-19629,26237,-19629,26237,-19629,
26099,-19811,26099,-19811,26099,-19811,26099,-19811,
25960,-19993,25960,-19993,25960,-19993,25960,-19993,
25820,-20174,25820,-20174,25820,-20174,25820,-20174,
25679,-20354,25679,-20354,25679,-20354,25679,-20354,
25536,-20532,25536,-20532,25536,-20532,25536,-20532,
25392,-20710,25392,-20710,25392,-20710,25392,-20710,
25247,-20887,25247,-20887,25247,-20887,25247,-20887,
25100,-21063,25100,-21063,25100,-21063,25100,-21063,
24953,-21237,24953,-21237,24953,-21237,24953,-21237,
24804,-21411,24804,-21411,24804,-21411,24804,-21411,
24654,-21584,24654,-21584,24654,-21584,24654,-21584,
24503,-21755,24503,-21755,24503,-21755,24503,-21755,
24350,-21926,24350,-21926,24350,-21926,24350,-21926,
24196,-22095,24196,-22095,24196,-22095,24196,-22095,
24042,-22264,24042,-22264,24042,-22264,24042,-22264,
23886,-22431,23886,-22431,23886,-22431,23886,-22431,
23728,-22597,23728,-22597,23728,-22597,23728,-22597,
23570,-22762,23570,-22762,23570,-22762,23570,-22762,
23411,-22926,23411,-22926,23411,-22926,23411,-22926,
23250,-23089,23250,-23089,23250,-23089,23250,-23089,
23088,-23251,23088,-23251,23088,-23251,23088,-23251,
22925,-23412,22925,-23412,22925,-23412,22925,-23412,
22761,-23571,22761,-23571,22761,-23571,22761,-23571,
22596,-23729,22596,-23729,22596,-23729,22596,-23729,
22430,-23887,22430,-23887,22430,-23887,22430,-23887,
22263,-24043,22263,-24043,22263,-24043,22263,-24043,
22094,-24197,22094,-24197,22094,-24197,22094,-24197,
21925,-24351,21925,-24351,21925,-24351,21925,-24351,
21754,-24504,21754,-24504,21754,-24504,21754,-24504,
21583,-24655,21583,-24655,21583,-24655,21583,-24655,
21410,-24805,21410,-24805,21410,-24805,21410,-24805,
21236,-24954,21236,-24954,21236,-24954,21236,-24954,
21062,-25101,21062,-25101,21062,-25101,21062,-25101,
20886,-25248,20886,-25248,20886,-25248,20886,-25248,
20709,-25393,20709,-25393,20709,-25393,20709,-25393,
20531,-25537,20531,-25537,20531,-25537,20531,-25537,
20353,-25680,20353,-25680,20353,-25680,20353,-25680,
20173,-25821,20173,-25821,20173,-25821,20173,-25821,
19992,-25961,19992,-25961,19992,-25961,19992,-25961,
19810,-26100,19810,-26100,19810,-26100,19810,-26100,
19628,-26238,19628,-26238,19628,-26238,19628,-26238,
19444,-26374,19444,-26374,19444,-26374,19444,-26374,
19259,-26510,19259,-26510,19259,-26510,19259,-26510,
19074,-26643,19074,-26643,19074,-26643,19074,-26643,
18887,-26776,18887,-26776,18887,-26776,18887,-26776,
18700,-26907,18700,-26907,18700,-26907,18700,-26907,
18512,-27037,18512,-27037,18512,-27037,18512,-27037,
18323,-27166,18323,-27166,18323,-27166,18323,-27166,
18132,-27293,18132,-27293,18132,-27293,18132,-27293,
17942,-27419,17942,-27419,17942,-27419,17942,-27419,
17750,-27543,17750,-27543,17750,-27543,17750,-27543,
17557,-27667,17557,-27667,17557,-27667,17557,-27667,
17363,-27788,17363,-27788,17363,-27788,17363,-27788,
17169,-27909,17169,-27909,17169,-27909,17169,-27909,
16974,-28028,16974,-28028,16974,-28028,16974,-28028,
16778,-28146,16778,-28146,16778,-28146,16778,-28146,
16581,-28262,16581,-28262,16581,-28262,16581,-28262,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
16184,-28491,16184,-28491,16184,-28491,16184,-28491,
15985,-28604,15985,-28604,15985,-28604,15985,-28604,
15785,-28714,15785,-28714,15785,-28714,15785,-28714,
15584,-28824,15584,-28824,15584,-28824,15584,-28824,
15383,-28932,15383,-28932,15383,-28932,15383,-28932,
15180,-29039,15180,-29039,15180,-29039,15180,-29039,
14977,-29144,14977,-29144,14977,-29144,14977,-29144,
14773,-29248,14773,-29248,14773,-29248,14773,-29248,
14569,-29350,14569,-29350,14569,-29350,14569,-29350,
14364,-29451,14364,-29451,14364,-29451,14364,-29451,
14158,-29551,14158,-29551,14158,-29551,14158,-29551,
13951,-29649,13951,-29649,13951,-29649,13951,-29649,
13744,-29746,13744,-29746,13744,-29746,13744,-29746,
13536,-29841,13536,-29841,13536,-29841,13536,-29841,
13327,-29935,13327,-29935,13327,-29935,13327,-29935,
13118,-30027,13118,-30027,13118,-30027,13118,-30027,
12908,-30118,12908,-30118,12908,-30118,12908,-30118,
12697,-30207,12697,-30207,12697,-30207,12697,-30207,
12486,-30295,12486,-30295,12486,-30295,12486,-30295,
12274,-30382,12274,-30382,12274,-30382,12274,-30382,
12062,-30466,12062,-30466,12062,-30466,12062,-30466,
11849,-30550,11849,-30550,11849,-30550,11849,-30550,
11635,-30632,11635,-30632,11635,-30632,11635,-30632,
11421,-30712,11421,-30712,11421,-30712,11421,-30712,
11206,-30791,11206,-30791,11206,-30791,11206,-30791,
10991,-30869,10991,-30869,10991,-30869,10991,-30869,
10775,-30945,10775,-30945,10775,-30945,10775,-30945,
10559,-31019,10559,-31019,10559,-31019,10559,-31019,
10342,-31092,10342,-31092,10342,-31092,10342,-31092,
10125,-31164,10125,-31164,10125,-31164,10125,-31164,
9907,-31234,9907,-31234,9907,-31234,9907,-31234,
9689,-31302,9689,-31302,9689,-31302,9689,-31302,
9470,-31369,9470,-31369,9470,-31369,9470,-31369,
9251,-31434,9251,-31434,9251,-31434,9251,-31434,
9031,-31498,9031,-31498,9031,-31498,9031,-31498,
8811,-31560,8811,-31560,8811,-31560,8811,-31560,
8591,-31621,8591,-31621,8591,-31621,8591,-31621,
8370,-31680,8370,-31680,8370,-31680,8370,-31680,
8148,-31738,8148,-31738,8148,-31738,8148,-31738,
7927,-31794,7927,-31794,7927,-31794,7927,-31794,
7704,-31849,7704,-31849,7704,-31849,7704,-31849,
7482,-31902,7482,-31902,7482,-31902,7482,-31902,
7259,-31953,7259,-31953,7259,-31953,7259,-31953,
7036,-32003,7036,-32003,7036,-32003,7036,-32003,
6812,-32051,6812,-32051,6812,-32051,6812,-32051,
6588,-32098,6588,-32098,6588,-32098,6588,-32098,
6364,-32143,6364,-32143,6364,-32143,6364,-32143,
6139,-32187,6139,-32187,6139,-32187,6139,-32187,
5915,-32229,5915,-32229,5915,-32229,5915,-32229,
5689,-32270,5689,-32270,5689,-32270,5689,-32270,
5464,-32309,5464,-32309,5464,-32309,5464,-32309,
5238,-32346,5238,-32346,5238,-32346,5238,-32346,
5012,-32382,5012,-32382,5012,-32382,5012,-32382,
4786,-32416,4786,-32416,4786,-32416,4786,-32416,
4560,-32449,4560,-32449,4560,-32449,4560,-32449,
4333,-32480,4333,-32480,4333,-32480,4333,-32480,
4106,-32509,4106,-32509,4106,-32509,4106,-32509,
3879,-32537,3879,-32537,3879,-32537,3879,-32537,
3652,-32563,3652,-32563,3652,-32563,3652,-32563,
3425,-32588,3425,-32588,3425,-32588,3425,-32588,
3197,-32611,3197,-32611,3197,-32611,3197,-32611,
2969,-32633,2969,-32633,2969,-32633,2969,-32633,
2741,-32653,2741,-32653,2741,-32653,2741,-32653,
2513,-32671,2513,-32671,2513,-32671,2513,-32671,
2285,-32688,2285,-32688,2285,-32688,2285,-32688,
2057,-32703,2057,-32703,2057,-32703,2057,-32703,
1829,-32716,1829,-32716,1829,-32716,1829,-32716,
1600,-32728,1600,-32728,1600,-32728,1600,-32728,
1372,-32739,1372,-32739,1372,-32739,1372,-32739,
1143,-32748,1143,-32748,1143,-32748,1143,-32748,
914,-32755,914,-32755,914,-32755,914,-32755,
686,-32760,686,-32760,686,-32760,686,-32760,
457,-32764,457,-32764,457,-32764,457,-32764,
228,-32767,228,-32767,228,-32767,228,-32767,
0,-32767,0,-32767,0,-32767,0,-32767,
-229,-32767,-229,-32767,-229,-32767,-229,-32767,
-458,-32764,-458,-32764,-458,-32764,-458,-32764,
-687,-32760,-687,-32760,-687,-32760,-687,-32760,
-915,-32755,-915,-32755,-915,-32755,-915,-32755,
-1144,-32748,-1144,-32748,-1144,-32748,-1144,-32748,
-1373,-32739,-1373,-32739,-1373,-32739,-1373,-32739,
-1601,-32728,-1601,-32728,-1601,-32728,-1601,-32728,
-1830,-32716,-1830,-32716,-1830,-32716,-1830,-32716,
-2058,-32703,-2058,-32703,-2058,-32703,-2058,-32703,
-2286,-32688,-2286,-32688,-2286,-32688,-2286,-32688,
-2514,-32671,-2514,-32671,-2514,-32671,-2514,-32671,
-2742,-32653,-2742,-32653,-2742,-32653,-2742,-32653,
-2970,-32633,-2970,-32633,-2970,-32633,-2970,-32633,
-3198,-32611,-3198,-32611,-3198,-32611,-3198,-32611,
-3426,-32588,-3426,-32588,-3426,-32588,-3426,-32588,
-3653,-32563,-3653,-32563,-3653,-32563,-3653,-32563,
-3880,-32537,-3880,-32537,-3880,-32537,-3880,-32537,
-4107,-32509,-4107,-32509,-4107,-32509,-4107,-32509,
-4334,-32480,-4334,-32480,-4334,-32480,-4334,-32480,
-4561,-32449,-4561,-32449,-4561,-32449,-4561,-32449,
-4787,-32416,-4787,-32416,-4787,-32416,-4787,-32416,
-5013,-32382,-5013,-32382,-5013,-32382,-5013,-32382,
-5239,-32346,-5239,-32346,-5239,-32346,-5239,-32346,
-5465,-32309,-5465,-32309,-5465,-32309,-5465,-32309,
-5690,-32270,-5690,-32270,-5690,-32270,-5690,-32270,
-5916,-32229,-5916,-32229,-5916,-32229,-5916,-32229,
-6140,-32187,-6140,-32187,-6140,-32187,-6140,-32187,
-6365,-32143,-6365,-32143,-6365,-32143,-6365,-32143,
-6589,-32098,-6589,-32098,-6589,-32098,-6589,-32098,
-6813,-32051,-6813,-32051,-6813,-32051,-6813,-32051,
-7037,-32003,-7037,-32003,-7037,-32003,-7037,-32003,
-7260,-31953,-7260,-31953,-7260,-31953,-7260,-31953,
-7483,-31902,-7483,-31902,-7483,-31902,-7483,-31902,
-7705,-31849,-7705,-31849,-7705,-31849,-7705,-31849,
-7928,-31794,-7928,-31794,-7928,-31794,-7928,-31794,
-8149,-31738,-8149,-31738,-8149,-31738,-8149,-31738,
-8371,-31680,-8371,-31680,-8371,-31680,-8371,-31680,
-8592,-31621,-8592,-31621,-8592,-31621,-8592,-31621,
-8812,-31560,-8812,-31560,-8812,-31560,-8812,-31560,
-9032,-31498,-9032,-31498,-9032,-31498,-9032,-31498,
-9252,-31434,-9252,-31434,-9252,-31434,-9252,-31434,
-9471,-31369,-9471,-31369,-9471,-31369,-9471,-31369,
-9690,-31302,-9690,-31302,-9690,-31302,-9690,-31302,
-9908,-31234,-9908,-31234,-9908,-31234,-9908,-31234,
-10126,-31164,-10126,-31164,-10126,-31164,-10126,-31164,
-10343,-31092,-10343,-31092,-10343,-31092,-10343,-31092,
-10560,-31019,-10560,-31019,-10560,-31019,-10560,-31019,
-10776,-30945,-10776,-30945,-10776,-30945,-10776,-30945,
-10992,-30869,-10992,-30869,-10992,-30869,-10992,-30869,
-11207,-30791,-11207,-30791,-11207,-30791,-11207,-30791,
-11422,-30712,-11422,-30712,-11422,-30712,-11422,-30712,
-11636,-30632,-11636,-30632,-11636,-30632,-11636,-30632,
-11850,-30550,-11850,-30550,-11850,-30550,-11850,-30550,
-12063,-30466,-12063,-30466,-12063,-30466,-12063,-30466,
-12275,-30382,-12275,-30382,-12275,-30382,-12275,-30382,
-12487,-30295,-12487,-30295,-12487,-30295,-12487,-30295,
-12698,-30207,-12698,-30207,-12698,-30207,-12698,-30207,
-12909,-30118,-12909,-30118,-12909,-30118,-12909,-30118,
-13119,-30027,-13119,-30027,-13119,-30027,-13119,-30027,
-13328,-29935,-13328,-29935,-13328,-29935,-13328,-29935,
-13537,-29841,-13537,-29841,-13537,-29841,-13537,-29841,
-13745,-29746,-13745,-29746,-13745,-29746,-13745,-29746,
-13952,-29649,-13952,-29649,-13952,-29649,-13952,-29649,
-14159,-29551,-14159,-29551,-14159,-29551,-14159,-29551,
-14365,-29451,-14365,-29451,-14365,-29451,-14365,-29451,
-14570,-29350,-14570,-29350,-14570,-29350,-14570,-29350,
-14774,-29248,-14774,-29248,-14774,-29248,-14774,-29248,
-14978,-29144,-14978,-29144,-14978,-29144,-14978,-29144,
-15181,-29039,-15181,-29039,-15181,-29039,-15181,-29039,
-15384,-28932,-15384,-28932,-15384,-28932,-15384,-28932,
-15585,-28824,-15585,-28824,-15585,-28824,-15585,-28824,
-15786,-28714,-15786,-28714,-15786,-28714,-15786,-28714,
-15986,-28604,-15986,-28604,-15986,-28604,-15986,-28604,
-16185,-28491,-16185,-28491,-16185,-28491,-16185,-28491};
static int16_t twb900[299*2*4] = {32763,-458,32763,-458,32763,-458,32763,-458,
32754,-915,32754,-915,32754,-915,32754,-915,
32738,-1373,32738,-1373,32738,-1373,32738,-1373,
32715,-1830,32715,-1830,32715,-1830,32715,-1830,
32687,-2286,32687,-2286,32687,-2286,32687,-2286,
32652,-2742,32652,-2742,32652,-2742,32652,-2742,
32610,-3198,32610,-3198,32610,-3198,32610,-3198,
32562,-3653,32562,-3653,32562,-3653,32562,-3653,
32508,-4107,32508,-4107,32508,-4107,32508,-4107,
32448,-4561,32448,-4561,32448,-4561,32448,-4561,
32381,-5013,32381,-5013,32381,-5013,32381,-5013,
32308,-5465,32308,-5465,32308,-5465,32308,-5465,
32228,-5916,32228,-5916,32228,-5916,32228,-5916,
32142,-6365,32142,-6365,32142,-6365,32142,-6365,
32050,-6813,32050,-6813,32050,-6813,32050,-6813,
31952,-7260,31952,-7260,31952,-7260,31952,-7260,
31848,-7705,31848,-7705,31848,-7705,31848,-7705,
31737,-8149,31737,-8149,31737,-8149,31737,-8149,
31620,-8592,31620,-8592,31620,-8592,31620,-8592,
31497,-9032,31497,-9032,31497,-9032,31497,-9032,
31368,-9471,31368,-9471,31368,-9471,31368,-9471,
31233,-9908,31233,-9908,31233,-9908,31233,-9908,
31091,-10343,31091,-10343,31091,-10343,31091,-10343,
30944,-10776,30944,-10776,30944,-10776,30944,-10776,
30790,-11207,30790,-11207,30790,-11207,30790,-11207,
30631,-11636,30631,-11636,30631,-11636,30631,-11636,
30465,-12063,30465,-12063,30465,-12063,30465,-12063,
30294,-12487,30294,-12487,30294,-12487,30294,-12487,
30117,-12909,30117,-12909,30117,-12909,30117,-12909,
29934,-13328,29934,-13328,29934,-13328,29934,-13328,
29745,-13745,29745,-13745,29745,-13745,29745,-13745,
29550,-14159,29550,-14159,29550,-14159,29550,-14159,
29349,-14570,29349,-14570,29349,-14570,29349,-14570,
29143,-14978,29143,-14978,29143,-14978,29143,-14978,
28931,-15384,28931,-15384,28931,-15384,28931,-15384,
28713,-15786,28713,-15786,28713,-15786,28713,-15786,
28490,-16185,28490,-16185,28490,-16185,28490,-16185,
28261,-16582,28261,-16582,28261,-16582,28261,-16582,
28027,-16975,28027,-16975,28027,-16975,28027,-16975,
27787,-17364,27787,-17364,27787,-17364,27787,-17364,
27542,-17751,27542,-17751,27542,-17751,27542,-17751,
27292,-18133,27292,-18133,27292,-18133,27292,-18133,
27036,-18513,27036,-18513,27036,-18513,27036,-18513,
26775,-18888,26775,-18888,26775,-18888,26775,-18888,
26509,-19260,26509,-19260,26509,-19260,26509,-19260,
26237,-19629,26237,-19629,26237,-19629,26237,-19629,
25960,-19993,25960,-19993,25960,-19993,25960,-19993,
25679,-20354,25679,-20354,25679,-20354,25679,-20354,
25392,-20710,25392,-20710,25392,-20710,25392,-20710,
25100,-21063,25100,-21063,25100,-21063,25100,-21063,
24804,-21411,24804,-21411,24804,-21411,24804,-21411,
24503,-21755,24503,-21755,24503,-21755,24503,-21755,
24196,-22095,24196,-22095,24196,-22095,24196,-22095,
23886,-22431,23886,-22431,23886,-22431,23886,-22431,
23570,-22762,23570,-22762,23570,-22762,23570,-22762,
23250,-23089,23250,-23089,23250,-23089,23250,-23089,
22925,-23412,22925,-23412,22925,-23412,22925,-23412,
22596,-23729,22596,-23729,22596,-23729,22596,-23729,
22263,-24043,22263,-24043,22263,-24043,22263,-24043,
21925,-24351,21925,-24351,21925,-24351,21925,-24351,
21583,-24655,21583,-24655,21583,-24655,21583,-24655,
21236,-24954,21236,-24954,21236,-24954,21236,-24954,
20886,-25248,20886,-25248,20886,-25248,20886,-25248,
20531,-25537,20531,-25537,20531,-25537,20531,-25537,
20173,-25821,20173,-25821,20173,-25821,20173,-25821,
19810,-26100,19810,-26100,19810,-26100,19810,-26100,
19444,-26374,19444,-26374,19444,-26374,19444,-26374,
19074,-26643,19074,-26643,19074,-26643,19074,-26643,
18700,-26907,18700,-26907,18700,-26907,18700,-26907,
18323,-27166,18323,-27166,18323,-27166,18323,-27166,
17942,-27419,17942,-27419,17942,-27419,17942,-27419,
17557,-27667,17557,-27667,17557,-27667,17557,-27667,
17169,-27909,17169,-27909,17169,-27909,17169,-27909,
16778,-28146,16778,-28146,16778,-28146,16778,-28146,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
15985,-28604,15985,-28604,15985,-28604,15985,-28604,
15584,-28824,15584,-28824,15584,-28824,15584,-28824,
15180,-29039,15180,-29039,15180,-29039,15180,-29039,
14773,-29248,14773,-29248,14773,-29248,14773,-29248,
14364,-29451,14364,-29451,14364,-29451,14364,-29451,
13951,-29649,13951,-29649,13951,-29649,13951,-29649,
13536,-29841,13536,-29841,13536,-29841,13536,-29841,
13118,-30027,13118,-30027,13118,-30027,13118,-30027,
12697,-30207,12697,-30207,12697,-30207,12697,-30207,
12274,-30382,12274,-30382,12274,-30382,12274,-30382,
11849,-30550,11849,-30550,11849,-30550,11849,-30550,
11421,-30712,11421,-30712,11421,-30712,11421,-30712,
10991,-30869,10991,-30869,10991,-30869,10991,-30869,
10559,-31019,10559,-31019,10559,-31019,10559,-31019,
10125,-31164,10125,-31164,10125,-31164,10125,-31164,
9689,-31302,9689,-31302,9689,-31302,9689,-31302,
9251,-31434,9251,-31434,9251,-31434,9251,-31434,
8811,-31560,8811,-31560,8811,-31560,8811,-31560,
8370,-31680,8370,-31680,8370,-31680,8370,-31680,
7927,-31794,7927,-31794,7927,-31794,7927,-31794,
7482,-31902,7482,-31902,7482,-31902,7482,-31902,
7036,-32003,7036,-32003,7036,-32003,7036,-32003,
6588,-32098,6588,-32098,6588,-32098,6588,-32098,
6139,-32187,6139,-32187,6139,-32187,6139,-32187,
5689,-32270,5689,-32270,5689,-32270,5689,-32270,
5238,-32346,5238,-32346,5238,-32346,5238,-32346,
4786,-32416,4786,-32416,4786,-32416,4786,-32416,
4333,-32480,4333,-32480,4333,-32480,4333,-32480,
3879,-32537,3879,-32537,3879,-32537,3879,-32537,
3425,-32588,3425,-32588,3425,-32588,3425,-32588,
2969,-32633,2969,-32633,2969,-32633,2969,-32633,
2513,-32671,2513,-32671,2513,-32671,2513,-32671,
2057,-32703,2057,-32703,2057,-32703,2057,-32703,
1600,-32728,1600,-32728,1600,-32728,1600,-32728,
1143,-32748,1143,-32748,1143,-32748,1143,-32748,
686,-32760,686,-32760,686,-32760,686,-32760,
228,-32767,228,-32767,228,-32767,228,-32767,
-229,-32767,-229,-32767,-229,-32767,-229,-32767,
-687,-32760,-687,-32760,-687,-32760,-687,-32760,
-1144,-32748,-1144,-32748,-1144,-32748,-1144,-32748,
-1601,-32728,-1601,-32728,-1601,-32728,-1601,-32728,
-2058,-32703,-2058,-32703,-2058,-32703,-2058,-32703,
-2514,-32671,-2514,-32671,-2514,-32671,-2514,-32671,
-2970,-32633,-2970,-32633,-2970,-32633,-2970,-32633,
-3426,-32588,-3426,-32588,-3426,-32588,-3426,-32588,
-3880,-32537,-3880,-32537,-3880,-32537,-3880,-32537,
-4334,-32480,-4334,-32480,-4334,-32480,-4334,-32480,
-4787,-32416,-4787,-32416,-4787,-32416,-4787,-32416,
-5239,-32346,-5239,-32346,-5239,-32346,-5239,-32346,
-5690,-32270,-5690,-32270,-5690,-32270,-5690,-32270,
-6140,-32187,-6140,-32187,-6140,-32187,-6140,-32187,
-6589,-32098,-6589,-32098,-6589,-32098,-6589,-32098,
-7037,-32003,-7037,-32003,-7037,-32003,-7037,-32003,
-7483,-31902,-7483,-31902,-7483,-31902,-7483,-31902,
-7928,-31794,-7928,-31794,-7928,-31794,-7928,-31794,
-8371,-31680,-8371,-31680,-8371,-31680,-8371,-31680,
-8812,-31560,-8812,-31560,-8812,-31560,-8812,-31560,
-9252,-31434,-9252,-31434,-9252,-31434,-9252,-31434,
-9690,-31302,-9690,-31302,-9690,-31302,-9690,-31302,
-10126,-31164,-10126,-31164,-10126,-31164,-10126,-31164,
-10560,-31019,-10560,-31019,-10560,-31019,-10560,-31019,
-10992,-30869,-10992,-30869,-10992,-30869,-10992,-30869,
-11422,-30712,-11422,-30712,-11422,-30712,-11422,-30712,
-11850,-30550,-11850,-30550,-11850,-30550,-11850,-30550,
-12275,-30382,-12275,-30382,-12275,-30382,-12275,-30382,
-12698,-30207,-12698,-30207,-12698,-30207,-12698,-30207,
-13119,-30027,-13119,-30027,-13119,-30027,-13119,-30027,
-13537,-29841,-13537,-29841,-13537,-29841,-13537,-29841,
-13952,-29649,-13952,-29649,-13952,-29649,-13952,-29649,
-14365,-29451,-14365,-29451,-14365,-29451,-14365,-29451,
-14774,-29248,-14774,-29248,-14774,-29248,-14774,-29248,
-15181,-29039,-15181,-29039,-15181,-29039,-15181,-29039,
-15585,-28824,-15585,-28824,-15585,-28824,-15585,-28824,
-15986,-28604,-15986,-28604,-15986,-28604,-15986,-28604,
-16384,-28378,-16384,-28378,-16384,-28378,-16384,-28378,
-16779,-28146,-16779,-28146,-16779,-28146,-16779,-28146,
-17170,-27909,-17170,-27909,-17170,-27909,-17170,-27909,
-17558,-27667,-17558,-27667,-17558,-27667,-17558,-27667,
-17943,-27419,-17943,-27419,-17943,-27419,-17943,-27419,
-18324,-27166,-18324,-27166,-18324,-27166,-18324,-27166,
-18701,-26907,-18701,-26907,-18701,-26907,-18701,-26907,
-19075,-26643,-19075,-26643,-19075,-26643,-19075,-26643,
-19445,-26374,-19445,-26374,-19445,-26374,-19445,-26374,
-19811,-26100,-19811,-26100,-19811,-26100,-19811,-26100,
-20174,-25821,-20174,-25821,-20174,-25821,-20174,-25821,
-20532,-25537,-20532,-25537,-20532,-25537,-20532,-25537,
-20887,-25248,-20887,-25248,-20887,-25248,-20887,-25248,
-21237,-24954,-21237,-24954,-21237,-24954,-21237,-24954,
-21584,-24655,-21584,-24655,-21584,-24655,-21584,-24655,
-21926,-24351,-21926,-24351,-21926,-24351,-21926,-24351,
-22264,-24043,-22264,-24043,-22264,-24043,-22264,-24043,
-22597,-23729,-22597,-23729,-22597,-23729,-22597,-23729,
-22926,-23412,-22926,-23412,-22926,-23412,-22926,-23412,
-23251,-23089,-23251,-23089,-23251,-23089,-23251,-23089,
-23571,-22762,-23571,-22762,-23571,-22762,-23571,-22762,
-23887,-22431,-23887,-22431,-23887,-22431,-23887,-22431,
-24197,-22095,-24197,-22095,-24197,-22095,-24197,-22095,
-24504,-21755,-24504,-21755,-24504,-21755,-24504,-21755,
-24805,-21411,-24805,-21411,-24805,-21411,-24805,-21411,
-25101,-21063,-25101,-21063,-25101,-21063,-25101,-21063,
-25393,-20710,-25393,-20710,-25393,-20710,-25393,-20710,
-25680,-20354,-25680,-20354,-25680,-20354,-25680,-20354,
-25961,-19993,-25961,-19993,-25961,-19993,-25961,-19993,
-26238,-19629,-26238,-19629,-26238,-19629,-26238,-19629,
-26510,-19260,-26510,-19260,-26510,-19260,-26510,-19260,
-26776,-18888,-26776,-18888,-26776,-18888,-26776,-18888,
-27037,-18513,-27037,-18513,-27037,-18513,-27037,-18513,
-27293,-18133,-27293,-18133,-27293,-18133,-27293,-18133,
-27543,-17751,-27543,-17751,-27543,-17751,-27543,-17751,
-27788,-17364,-27788,-17364,-27788,-17364,-27788,-17364,
-28028,-16975,-28028,-16975,-28028,-16975,-28028,-16975,
-28262,-16582,-28262,-16582,-28262,-16582,-28262,-16582,
-28491,-16185,-28491,-16185,-28491,-16185,-28491,-16185,
-28714,-15786,-28714,-15786,-28714,-15786,-28714,-15786,
-28932,-15384,-28932,-15384,-28932,-15384,-28932,-15384,
-29144,-14978,-29144,-14978,-29144,-14978,-29144,-14978,
-29350,-14570,-29350,-14570,-29350,-14570,-29350,-14570,
-29551,-14159,-29551,-14159,-29551,-14159,-29551,-14159,
-29746,-13745,-29746,-13745,-29746,-13745,-29746,-13745,
-29935,-13328,-29935,-13328,-29935,-13328,-29935,-13328,
-30118,-12909,-30118,-12909,-30118,-12909,-30118,-12909,
-30295,-12487,-30295,-12487,-30295,-12487,-30295,-12487,
-30466,-12063,-30466,-12063,-30466,-12063,-30466,-12063,
-30632,-11636,-30632,-11636,-30632,-11636,-30632,-11636,
-30791,-11207,-30791,-11207,-30791,-11207,-30791,-11207,
-30945,-10776,-30945,-10776,-30945,-10776,-30945,-10776,
-31092,-10343,-31092,-10343,-31092,-10343,-31092,-10343,
-31234,-9908,-31234,-9908,-31234,-9908,-31234,-9908,
-31369,-9471,-31369,-9471,-31369,-9471,-31369,-9471,
-31498,-9032,-31498,-9032,-31498,-9032,-31498,-9032,
-31621,-8592,-31621,-8592,-31621,-8592,-31621,-8592,
-31738,-8149,-31738,-8149,-31738,-8149,-31738,-8149,
-31849,-7705,-31849,-7705,-31849,-7705,-31849,-7705,
-31953,-7260,-31953,-7260,-31953,-7260,-31953,-7260,
-32051,-6813,-32051,-6813,-32051,-6813,-32051,-6813,
-32143,-6365,-32143,-6365,-32143,-6365,-32143,-6365,
-32229,-5916,-32229,-5916,-32229,-5916,-32229,-5916,
-32309,-5465,-32309,-5465,-32309,-5465,-32309,-5465,
-32382,-5013,-32382,-5013,-32382,-5013,-32382,-5013,
-32449,-4561,-32449,-4561,-32449,-4561,-32449,-4561,
-32509,-4107,-32509,-4107,-32509,-4107,-32509,-4107,
-32563,-3653,-32563,-3653,-32563,-3653,-32563,-3653,
-32611,-3198,-32611,-3198,-32611,-3198,-32611,-3198,
-32653,-2742,-32653,-2742,-32653,-2742,-32653,-2742,
-32688,-2286,-32688,-2286,-32688,-2286,-32688,-2286,
-32716,-1830,-32716,-1830,-32716,-1830,-32716,-1830,
-32739,-1373,-32739,-1373,-32739,-1373,-32739,-1373,
-32755,-915,-32755,-915,-32755,-915,-32755,-915,
-32764,-458,-32764,-458,-32764,-458,-32764,-458,
-32767,-1,-32767,-1,-32767,-1,-32767,-1,
-32764,457,-32764,457,-32764,457,-32764,457,
-32755,914,-32755,914,-32755,914,-32755,914,
-32739,1372,-32739,1372,-32739,1372,-32739,1372,
-32716,1829,-32716,1829,-32716,1829,-32716,1829,
-32688,2285,-32688,2285,-32688,2285,-32688,2285,
-32653,2741,-32653,2741,-32653,2741,-32653,2741,
-32611,3197,-32611,3197,-32611,3197,-32611,3197,
-32563,3652,-32563,3652,-32563,3652,-32563,3652,
-32509,4106,-32509,4106,-32509,4106,-32509,4106,
-32449,4560,-32449,4560,-32449,4560,-32449,4560,
-32382,5012,-32382,5012,-32382,5012,-32382,5012,
-32309,5464,-32309,5464,-32309,5464,-32309,5464,
-32229,5915,-32229,5915,-32229,5915,-32229,5915,
-32143,6364,-32143,6364,-32143,6364,-32143,6364,
-32051,6812,-32051,6812,-32051,6812,-32051,6812,
-31953,7259,-31953,7259,-31953,7259,-31953,7259,
-31849,7704,-31849,7704,-31849,7704,-31849,7704,
-31738,8148,-31738,8148,-31738,8148,-31738,8148,
-31621,8591,-31621,8591,-31621,8591,-31621,8591,
-31498,9031,-31498,9031,-31498,9031,-31498,9031,
-31369,9470,-31369,9470,-31369,9470,-31369,9470,
-31234,9907,-31234,9907,-31234,9907,-31234,9907,
-31092,10342,-31092,10342,-31092,10342,-31092,10342,
-30945,10775,-30945,10775,-30945,10775,-30945,10775,
-30791,11206,-30791,11206,-30791,11206,-30791,11206,
-30632,11635,-30632,11635,-30632,11635,-30632,11635,
-30466,12062,-30466,12062,-30466,12062,-30466,12062,
-30295,12486,-30295,12486,-30295,12486,-30295,12486,
-30118,12908,-30118,12908,-30118,12908,-30118,12908,
-29935,13327,-29935,13327,-29935,13327,-29935,13327,
-29746,13744,-29746,13744,-29746,13744,-29746,13744,
-29551,14158,-29551,14158,-29551,14158,-29551,14158,
-29350,14569,-29350,14569,-29350,14569,-29350,14569,
-29144,14977,-29144,14977,-29144,14977,-29144,14977,
-28932,15383,-28932,15383,-28932,15383,-28932,15383,
-28714,15785,-28714,15785,-28714,15785,-28714,15785,
-28491,16184,-28491,16184,-28491,16184,-28491,16184,
-28262,16581,-28262,16581,-28262,16581,-28262,16581,
-28028,16974,-28028,16974,-28028,16974,-28028,16974,
-27788,17363,-27788,17363,-27788,17363,-27788,17363,
-27543,17750,-27543,17750,-27543,17750,-27543,17750,
-27293,18132,-27293,18132,-27293,18132,-27293,18132,
-27037,18512,-27037,18512,-27037,18512,-27037,18512,
-26776,18887,-26776,18887,-26776,18887,-26776,18887,
-26510,19259,-26510,19259,-26510,19259,-26510,19259,
-26238,19628,-26238,19628,-26238,19628,-26238,19628,
-25961,19992,-25961,19992,-25961,19992,-25961,19992,
-25680,20353,-25680,20353,-25680,20353,-25680,20353,
-25393,20709,-25393,20709,-25393,20709,-25393,20709,
-25101,21062,-25101,21062,-25101,21062,-25101,21062,
-24805,21410,-24805,21410,-24805,21410,-24805,21410,
-24504,21754,-24504,21754,-24504,21754,-24504,21754,
-24197,22094,-24197,22094,-24197,22094,-24197,22094,
-23887,22430,-23887,22430,-23887,22430,-23887,22430,
-23571,22761,-23571,22761,-23571,22761,-23571,22761,
-23251,23088,-23251,23088,-23251,23088,-23251,23088,
-22926,23411,-22926,23411,-22926,23411,-22926,23411,
-22597,23728,-22597,23728,-22597,23728,-22597,23728,
-22264,24042,-22264,24042,-22264,24042,-22264,24042,
-21926,24350,-21926,24350,-21926,24350,-21926,24350,
-21584,24654,-21584,24654,-21584,24654,-21584,24654,
-21237,24953,-21237,24953,-21237,24953,-21237,24953,
-20887,25247,-20887,25247,-20887,25247,-20887,25247,
-20532,25536,-20532,25536,-20532,25536,-20532,25536,
-20174,25820,-20174,25820,-20174,25820,-20174,25820,
-19811,26099,-19811,26099,-19811,26099,-19811,26099,
-19445,26373,-19445,26373,-19445,26373,-19445,26373,
-19075,26642,-19075,26642,-19075,26642,-19075,26642,
-18701,26906,-18701,26906,-18701,26906,-18701,26906,
-18324,27165,-18324,27165,-18324,27165,-18324,27165,
-17943,27418,-17943,27418,-17943,27418,-17943,27418,
-17558,27666,-17558,27666,-17558,27666,-17558,27666,
-17170,27908,-17170,27908,-17170,27908,-17170,27908,
-16779,28145,-16779,28145,-16779,28145,-16779,28145};

void dft900(int16_t *x,int16_t *y,unsigned char scale_flag){ // 300 x 3
  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa900[0];
  __m128i *twb128=(__m128i *)&twb900[0];
  __m128i x2128[900];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[900];//=&ytmp128array3[0];



  for (i=0,j=0;i<300;i++,j+=3) {
    x2128[i]    = x128[j];
    x2128[i+300] = x128[j+1];
    x2128[i+600] = x128[j+2];
  }

  dft300((int16_t *)x2128,(int16_t *)ytmp128,1);
  dft300((int16_t *)(x2128+300),(int16_t *)(ytmp128+300),1);
  dft300((int16_t *)(x2128+600),(int16_t *)(ytmp128+600),1);

  bfly3_tw1(ytmp128,ytmp128+300,ytmp128+600,y128,y128+300,y128+600);
  for (i=1,j=0;i<300;i++,j++) {
    bfly3(ytmp128+i,
	  ytmp128+300+i,
	  ytmp128+600+i,
	  y128+i,
	  y128+300+i,
	  y128+600+i,
	  twa128+j,
	  twb128+j);
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(dft_norm_table[14]);
    
    for (i=0;i<900;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

};


/* Twiddles generated with
twa = floor(32767*exp(-sqrt(-1)*2*pi*(1:239)/960));
twb = floor(32767*exp(-sqrt(-1)*2*pi*2*(1:239)/960));
twc = floor(32767*exp(-sqrt(-1)*2*pi*3*(1:239)/960));
twa2 = zeros(1,2*239);
twb2 = zeros(1,2*239);
twc2 = zeros(1,2*239);
twa2(1:2:end) = real(twa);
twa2(2:2:end) = imag(twa);
twb2(1:2:end) = real(twb);
twb2(2:2:end) = imag(twb);
twc2(1:2:end) = real(twc);
twc2(2:2:end) = imag(twc);
fd=fopen("twiddle_tmp.txt","w");
fprintf(fd,"static int16_t twa960[239*2*4] = {");
for i=1:2:(2*238)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
fprintf(fd,"\nstatic int16_t twb960[239*2*4] = {");
for i=1:2:(2*238)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
fprintf(fd,"\nstatic int16_t twc960[239*2*4] = {");
for i=1:2:(2*238)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twc2(i),twc2(i+1),twc2(i),twc2(i+1),twc2(i),twc2(i+1),twc2(i),twc2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twc2(i),twc2(i+1),twc2(i),twc2(i+1),twc2(i),twc2(i+1),twc2(i),twc2(i+1));
fclose(fd);
*/
static int16_t twa960[239*2*4] = {32766,-215,32766,-215,32766,-215,32766,-215,
32764,-429,32764,-429,32764,-429,32764,-429,
32760,-644,32760,-644,32760,-644,32760,-644,
32755,-858,32755,-858,32755,-858,32755,-858,
32749,-1073,32749,-1073,32749,-1073,32749,-1073,
32741,-1287,32741,-1287,32741,-1287,32741,-1287,
32732,-1501,32732,-1501,32732,-1501,32732,-1501,
32722,-1715,32722,-1715,32722,-1715,32722,-1715,
32710,-1930,32710,-1930,32710,-1930,32710,-1930,
32696,-2144,32696,-2144,32696,-2144,32696,-2144,
32682,-2358,32682,-2358,32682,-2358,32682,-2358,
32665,-2571,32665,-2571,32665,-2571,32665,-2571,
32648,-2785,32648,-2785,32648,-2785,32648,-2785,
32629,-2999,32629,-2999,32629,-2999,32629,-2999,
32609,-3212,32609,-3212,32609,-3212,32609,-3212,
32587,-3426,32587,-3426,32587,-3426,32587,-3426,
32564,-3639,32564,-3639,32564,-3639,32564,-3639,
32539,-3852,32539,-3852,32539,-3852,32539,-3852,
32513,-4065,32513,-4065,32513,-4065,32513,-4065,
32486,-4277,32486,-4277,32486,-4277,32486,-4277,
32457,-4490,32457,-4490,32457,-4490,32457,-4490,
32427,-4702,32427,-4702,32427,-4702,32427,-4702,
32396,-4914,32396,-4914,32396,-4914,32396,-4914,
32363,-5126,32363,-5126,32363,-5126,32363,-5126,
32329,-5338,32329,-5338,32329,-5338,32329,-5338,
32293,-5550,32293,-5550,32293,-5550,32293,-5550,
32256,-5761,32256,-5761,32256,-5761,32256,-5761,
32218,-5972,32218,-5972,32218,-5972,32218,-5972,
32178,-6183,32178,-6183,32178,-6183,32178,-6183,
32137,-6393,32137,-6393,32137,-6393,32137,-6393,
32094,-6603,32094,-6603,32094,-6603,32094,-6603,
32050,-6813,32050,-6813,32050,-6813,32050,-6813,
32005,-7023,32005,-7023,32005,-7023,32005,-7023,
31959,-7232,31959,-7232,31959,-7232,31959,-7232,
31911,-7441,31911,-7441,31911,-7441,31911,-7441,
31861,-7650,31861,-7650,31861,-7650,31861,-7650,
31810,-7858,31810,-7858,31810,-7858,31810,-7858,
31758,-8066,31758,-8066,31758,-8066,31758,-8066,
31705,-8274,31705,-8274,31705,-8274,31705,-8274,
31650,-8481,31650,-8481,31650,-8481,31650,-8481,
31594,-8688,31594,-8688,31594,-8688,31594,-8688,
31536,-8895,31536,-8895,31536,-8895,31536,-8895,
31477,-9101,31477,-9101,31477,-9101,31477,-9101,
31417,-9307,31417,-9307,31417,-9307,31417,-9307,
31356,-9512,31356,-9512,31356,-9512,31356,-9512,
31293,-9717,31293,-9717,31293,-9717,31293,-9717,
31228,-9922,31228,-9922,31228,-9922,31228,-9922,
31163,-10126,31163,-10126,31163,-10126,31163,-10126,
31096,-10330,31096,-10330,31096,-10330,31096,-10330,
31028,-10533,31028,-10533,31028,-10533,31028,-10533,
30958,-10736,30958,-10736,30958,-10736,30958,-10736,
30887,-10938,30887,-10938,30887,-10938,30887,-10938,
30815,-11140,30815,-11140,30815,-11140,30815,-11140,
30741,-11342,30741,-11342,30741,-11342,30741,-11342,
30666,-11543,30666,-11543,30666,-11543,30666,-11543,
30590,-11743,30590,-11743,30590,-11743,30590,-11743,
30513,-11943,30513,-11943,30513,-11943,30513,-11943,
30434,-12143,30434,-12143,30434,-12143,30434,-12143,
30354,-12341,30354,-12341,30354,-12341,30354,-12341,
30272,-12540,30272,-12540,30272,-12540,30272,-12540,
30190,-12738,30190,-12738,30190,-12738,30190,-12738,
30106,-12935,30106,-12935,30106,-12935,30106,-12935,
30020,-13132,30020,-13132,30020,-13132,30020,-13132,
29934,-13328,29934,-13328,29934,-13328,29934,-13328,
29846,-13524,29846,-13524,29846,-13524,29846,-13524,
29757,-13719,29757,-13719,29757,-13719,29757,-13719,
29666,-13913,29666,-13913,29666,-13913,29666,-13913,
29575,-14107,29575,-14107,29575,-14107,29575,-14107,
29482,-14300,29482,-14300,29482,-14300,29482,-14300,
29387,-14493,29387,-14493,29387,-14493,29387,-14493,
29292,-14685,29292,-14685,29292,-14685,29292,-14685,
29195,-14876,29195,-14876,29195,-14876,29195,-14876,
29097,-15067,29097,-15067,29097,-15067,29097,-15067,
28998,-15257,28998,-15257,28998,-15257,28998,-15257,
28897,-15447,28897,-15447,28897,-15447,28897,-15447,
28796,-15636,28796,-15636,28796,-15636,28796,-15636,
28693,-15824,28693,-15824,28693,-15824,28693,-15824,
28589,-16011,28589,-16011,28589,-16011,28589,-16011,
28483,-16198,28483,-16198,28483,-16198,28483,-16198,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
28269,-16569,28269,-16569,28269,-16569,28269,-16569,
28160,-16754,28160,-16754,28160,-16754,28160,-16754,
28049,-16938,28049,-16938,28049,-16938,28049,-16938,
27938,-17121,27938,-17121,27938,-17121,27938,-17121,
27825,-17304,27825,-17304,27825,-17304,27825,-17304,
27711,-17485,27711,-17485,27711,-17485,27711,-17485,
27596,-17666,27596,-17666,27596,-17666,27596,-17666,
27480,-17847,27480,-17847,27480,-17847,27480,-17847,
27363,-18026,27363,-18026,27363,-18026,27363,-18026,
27244,-18205,27244,-18205,27244,-18205,27244,-18205,
27125,-18383,27125,-18383,27125,-18383,27125,-18383,
27004,-18560,27004,-18560,27004,-18560,27004,-18560,
26882,-18736,26882,-18736,26882,-18736,26882,-18736,
26758,-18912,26758,-18912,26758,-18912,26758,-18912,
26634,-19087,26634,-19087,26634,-19087,26634,-19087,
26509,-19260,26509,-19260,26509,-19260,26509,-19260,
26382,-19434,26382,-19434,26382,-19434,26382,-19434,
26254,-19606,26254,-19606,26254,-19606,26254,-19606,
26125,-19777,26125,-19777,26125,-19777,26125,-19777,
25995,-19948,25995,-19948,25995,-19948,25995,-19948,
25864,-20117,25864,-20117,25864,-20117,25864,-20117,
25732,-20286,25732,-20286,25732,-20286,25732,-20286,
25599,-20454,25599,-20454,25599,-20454,25599,-20454,
25464,-20621,25464,-20621,25464,-20621,25464,-20621,
25329,-20788,25329,-20788,25329,-20788,25329,-20788,
25192,-20953,25192,-20953,25192,-20953,25192,-20953,
25054,-21117,25054,-21117,25054,-21117,25054,-21117,
24916,-21281,24916,-21281,24916,-21281,24916,-21281,
24776,-21444,24776,-21444,24776,-21444,24776,-21444,
24635,-21605,24635,-21605,24635,-21605,24635,-21605,
24493,-21766,24493,-21766,24493,-21766,24493,-21766,
24350,-21926,24350,-21926,24350,-21926,24350,-21926,
24206,-22085,24206,-22085,24206,-22085,24206,-22085,
24061,-22243,24061,-22243,24061,-22243,24061,-22243,
23915,-22400,23915,-22400,23915,-22400,23915,-22400,
23768,-22556,23768,-22556,23768,-22556,23768,-22556,
23620,-22711,23620,-22711,23620,-22711,23620,-22711,
23471,-22865,23471,-22865,23471,-22865,23471,-22865,
23320,-23018,23320,-23018,23320,-23018,23320,-23018,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
23017,-23321,23017,-23321,23017,-23321,23017,-23321,
22864,-23472,22864,-23472,22864,-23472,22864,-23472,
22710,-23621,22710,-23621,22710,-23621,22710,-23621,
22555,-23769,22555,-23769,22555,-23769,22555,-23769,
22399,-23916,22399,-23916,22399,-23916,22399,-23916,
22242,-24062,22242,-24062,22242,-24062,22242,-24062,
22084,-24207,22084,-24207,22084,-24207,22084,-24207,
21925,-24351,21925,-24351,21925,-24351,21925,-24351,
21765,-24494,21765,-24494,21765,-24494,21765,-24494,
21604,-24636,21604,-24636,21604,-24636,21604,-24636,
21443,-24777,21443,-24777,21443,-24777,21443,-24777,
21280,-24917,21280,-24917,21280,-24917,21280,-24917,
21116,-25055,21116,-25055,21116,-25055,21116,-25055,
20952,-25193,20952,-25193,20952,-25193,20952,-25193,
20787,-25330,20787,-25330,20787,-25330,20787,-25330,
20620,-25465,20620,-25465,20620,-25465,20620,-25465,
20453,-25600,20453,-25600,20453,-25600,20453,-25600,
20285,-25733,20285,-25733,20285,-25733,20285,-25733,
20116,-25865,20116,-25865,20116,-25865,20116,-25865,
19947,-25996,19947,-25996,19947,-25996,19947,-25996,
19776,-26126,19776,-26126,19776,-26126,19776,-26126,
19605,-26255,19605,-26255,19605,-26255,19605,-26255,
19433,-26383,19433,-26383,19433,-26383,19433,-26383,
19259,-26510,19259,-26510,19259,-26510,19259,-26510,
19086,-26635,19086,-26635,19086,-26635,19086,-26635,
18911,-26759,18911,-26759,18911,-26759,18911,-26759,
18735,-26883,18735,-26883,18735,-26883,18735,-26883,
18559,-27005,18559,-27005,18559,-27005,18559,-27005,
18382,-27126,18382,-27126,18382,-27126,18382,-27126,
18204,-27245,18204,-27245,18204,-27245,18204,-27245,
18025,-27364,18025,-27364,18025,-27364,18025,-27364,
17846,-27481,17846,-27481,17846,-27481,17846,-27481,
17665,-27597,17665,-27597,17665,-27597,17665,-27597,
17484,-27712,17484,-27712,17484,-27712,17484,-27712,
17303,-27826,17303,-27826,17303,-27826,17303,-27826,
17120,-27939,17120,-27939,17120,-27939,17120,-27939,
16937,-28050,16937,-28050,16937,-28050,16937,-28050,
16753,-28161,16753,-28161,16753,-28161,16753,-28161,
16568,-28270,16568,-28270,16568,-28270,16568,-28270,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
16197,-28484,16197,-28484,16197,-28484,16197,-28484,
16010,-28590,16010,-28590,16010,-28590,16010,-28590,
15823,-28694,15823,-28694,15823,-28694,15823,-28694,
15635,-28797,15635,-28797,15635,-28797,15635,-28797,
15446,-28898,15446,-28898,15446,-28898,15446,-28898,
15256,-28999,15256,-28999,15256,-28999,15256,-28999,
15066,-29098,15066,-29098,15066,-29098,15066,-29098,
14875,-29196,14875,-29196,14875,-29196,14875,-29196,
14684,-29293,14684,-29293,14684,-29293,14684,-29293,
14492,-29388,14492,-29388,14492,-29388,14492,-29388,
14299,-29483,14299,-29483,14299,-29483,14299,-29483,
14106,-29576,14106,-29576,14106,-29576,14106,-29576,
13912,-29667,13912,-29667,13912,-29667,13912,-29667,
13718,-29758,13718,-29758,13718,-29758,13718,-29758,
13523,-29847,13523,-29847,13523,-29847,13523,-29847,
13327,-29935,13327,-29935,13327,-29935,13327,-29935,
13131,-30021,13131,-30021,13131,-30021,13131,-30021,
12934,-30107,12934,-30107,12934,-30107,12934,-30107,
12737,-30191,12737,-30191,12737,-30191,12737,-30191,
12539,-30273,12539,-30273,12539,-30273,12539,-30273,
12340,-30355,12340,-30355,12340,-30355,12340,-30355,
12142,-30435,12142,-30435,12142,-30435,12142,-30435,
11942,-30514,11942,-30514,11942,-30514,11942,-30514,
11742,-30591,11742,-30591,11742,-30591,11742,-30591,
11542,-30667,11542,-30667,11542,-30667,11542,-30667,
11341,-30742,11341,-30742,11341,-30742,11341,-30742,
11139,-30816,11139,-30816,11139,-30816,11139,-30816,
10937,-30888,10937,-30888,10937,-30888,10937,-30888,
10735,-30959,10735,-30959,10735,-30959,10735,-30959,
10532,-31029,10532,-31029,10532,-31029,10532,-31029,
10329,-31097,10329,-31097,10329,-31097,10329,-31097,
10125,-31164,10125,-31164,10125,-31164,10125,-31164,
9921,-31229,9921,-31229,9921,-31229,9921,-31229,
9716,-31294,9716,-31294,9716,-31294,9716,-31294,
9511,-31357,9511,-31357,9511,-31357,9511,-31357,
9306,-31418,9306,-31418,9306,-31418,9306,-31418,
9100,-31478,9100,-31478,9100,-31478,9100,-31478,
8894,-31537,8894,-31537,8894,-31537,8894,-31537,
8687,-31595,8687,-31595,8687,-31595,8687,-31595,
8480,-31651,8480,-31651,8480,-31651,8480,-31651,
8273,-31706,8273,-31706,8273,-31706,8273,-31706,
8065,-31759,8065,-31759,8065,-31759,8065,-31759,
7857,-31811,7857,-31811,7857,-31811,7857,-31811,
7649,-31862,7649,-31862,7649,-31862,7649,-31862,
7440,-31912,7440,-31912,7440,-31912,7440,-31912,
7231,-31960,7231,-31960,7231,-31960,7231,-31960,
7022,-32006,7022,-32006,7022,-32006,7022,-32006,
6812,-32051,6812,-32051,6812,-32051,6812,-32051,
6602,-32095,6602,-32095,6602,-32095,6602,-32095,
6392,-32138,6392,-32138,6392,-32138,6392,-32138,
6182,-32179,6182,-32179,6182,-32179,6182,-32179,
5971,-32219,5971,-32219,5971,-32219,5971,-32219,
5760,-32257,5760,-32257,5760,-32257,5760,-32257,
5549,-32294,5549,-32294,5549,-32294,5549,-32294,
5337,-32330,5337,-32330,5337,-32330,5337,-32330,
5125,-32364,5125,-32364,5125,-32364,5125,-32364,
4913,-32397,4913,-32397,4913,-32397,4913,-32397,
4701,-32428,4701,-32428,4701,-32428,4701,-32428,
4489,-32458,4489,-32458,4489,-32458,4489,-32458,
4276,-32487,4276,-32487,4276,-32487,4276,-32487,
4064,-32514,4064,-32514,4064,-32514,4064,-32514,
3851,-32540,3851,-32540,3851,-32540,3851,-32540,
3638,-32565,3638,-32565,3638,-32565,3638,-32565,
3425,-32588,3425,-32588,3425,-32588,3425,-32588,
3211,-32610,3211,-32610,3211,-32610,3211,-32610,
2998,-32630,2998,-32630,2998,-32630,2998,-32630,
2784,-32649,2784,-32649,2784,-32649,2784,-32649,
2570,-32666,2570,-32666,2570,-32666,2570,-32666,
2357,-32683,2357,-32683,2357,-32683,2357,-32683,
2143,-32697,2143,-32697,2143,-32697,2143,-32697,
1929,-32711,1929,-32711,1929,-32711,1929,-32711,
1714,-32723,1714,-32723,1714,-32723,1714,-32723,
1500,-32733,1500,-32733,1500,-32733,1500,-32733,
1286,-32742,1286,-32742,1286,-32742,1286,-32742,
1072,-32750,1072,-32750,1072,-32750,1072,-32750,
857,-32756,857,-32756,857,-32756,857,-32756,
643,-32761,643,-32761,643,-32761,643,-32761,
428,-32765,428,-32765,428,-32765,428,-32765,
214,-32767,214,-32767,214,-32767,214,-32767};

static int16_t twb960[239*2*4] = {32764,-429,32764,-429,32764,-429,32764,-429,
32755,-858,32755,-858,32755,-858,32755,-858,
32741,-1287,32741,-1287,32741,-1287,32741,-1287,
32722,-1715,32722,-1715,32722,-1715,32722,-1715,
32696,-2144,32696,-2144,32696,-2144,32696,-2144,
32665,-2571,32665,-2571,32665,-2571,32665,-2571,
32629,-2999,32629,-2999,32629,-2999,32629,-2999,
32587,-3426,32587,-3426,32587,-3426,32587,-3426,
32539,-3852,32539,-3852,32539,-3852,32539,-3852,
32486,-4277,32486,-4277,32486,-4277,32486,-4277,
32427,-4702,32427,-4702,32427,-4702,32427,-4702,
32363,-5126,32363,-5126,32363,-5126,32363,-5126,
32293,-5550,32293,-5550,32293,-5550,32293,-5550,
32218,-5972,32218,-5972,32218,-5972,32218,-5972,
32137,-6393,32137,-6393,32137,-6393,32137,-6393,
32050,-6813,32050,-6813,32050,-6813,32050,-6813,
31959,-7232,31959,-7232,31959,-7232,31959,-7232,
31861,-7650,31861,-7650,31861,-7650,31861,-7650,
31758,-8066,31758,-8066,31758,-8066,31758,-8066,
31650,-8481,31650,-8481,31650,-8481,31650,-8481,
31536,-8895,31536,-8895,31536,-8895,31536,-8895,
31417,-9307,31417,-9307,31417,-9307,31417,-9307,
31293,-9717,31293,-9717,31293,-9717,31293,-9717,
31163,-10126,31163,-10126,31163,-10126,31163,-10126,
31028,-10533,31028,-10533,31028,-10533,31028,-10533,
30887,-10938,30887,-10938,30887,-10938,30887,-10938,
30741,-11342,30741,-11342,30741,-11342,30741,-11342,
30590,-11743,30590,-11743,30590,-11743,30590,-11743,
30434,-12143,30434,-12143,30434,-12143,30434,-12143,
30272,-12540,30272,-12540,30272,-12540,30272,-12540,
30106,-12935,30106,-12935,30106,-12935,30106,-12935,
29934,-13328,29934,-13328,29934,-13328,29934,-13328,
29757,-13719,29757,-13719,29757,-13719,29757,-13719,
29575,-14107,29575,-14107,29575,-14107,29575,-14107,
29387,-14493,29387,-14493,29387,-14493,29387,-14493,
29195,-14876,29195,-14876,29195,-14876,29195,-14876,
28998,-15257,28998,-15257,28998,-15257,28998,-15257,
28796,-15636,28796,-15636,28796,-15636,28796,-15636,
28589,-16011,28589,-16011,28589,-16011,28589,-16011,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
28160,-16754,28160,-16754,28160,-16754,28160,-16754,
27938,-17121,27938,-17121,27938,-17121,27938,-17121,
27711,-17485,27711,-17485,27711,-17485,27711,-17485,
27480,-17847,27480,-17847,27480,-17847,27480,-17847,
27244,-18205,27244,-18205,27244,-18205,27244,-18205,
27004,-18560,27004,-18560,27004,-18560,27004,-18560,
26758,-18912,26758,-18912,26758,-18912,26758,-18912,
26509,-19260,26509,-19260,26509,-19260,26509,-19260,
26254,-19606,26254,-19606,26254,-19606,26254,-19606,
25995,-19948,25995,-19948,25995,-19948,25995,-19948,
25732,-20286,25732,-20286,25732,-20286,25732,-20286,
25464,-20621,25464,-20621,25464,-20621,25464,-20621,
25192,-20953,25192,-20953,25192,-20953,25192,-20953,
24916,-21281,24916,-21281,24916,-21281,24916,-21281,
24635,-21605,24635,-21605,24635,-21605,24635,-21605,
24350,-21926,24350,-21926,24350,-21926,24350,-21926,
24061,-22243,24061,-22243,24061,-22243,24061,-22243,
23768,-22556,23768,-22556,23768,-22556,23768,-22556,
23471,-22865,23471,-22865,23471,-22865,23471,-22865,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
22864,-23472,22864,-23472,22864,-23472,22864,-23472,
22555,-23769,22555,-23769,22555,-23769,22555,-23769,
22242,-24062,22242,-24062,22242,-24062,22242,-24062,
21925,-24351,21925,-24351,21925,-24351,21925,-24351,
21604,-24636,21604,-24636,21604,-24636,21604,-24636,
21280,-24917,21280,-24917,21280,-24917,21280,-24917,
20952,-25193,20952,-25193,20952,-25193,20952,-25193,
20620,-25465,20620,-25465,20620,-25465,20620,-25465,
20285,-25733,20285,-25733,20285,-25733,20285,-25733,
19947,-25996,19947,-25996,19947,-25996,19947,-25996,
19605,-26255,19605,-26255,19605,-26255,19605,-26255,
19259,-26510,19259,-26510,19259,-26510,19259,-26510,
18911,-26759,18911,-26759,18911,-26759,18911,-26759,
18559,-27005,18559,-27005,18559,-27005,18559,-27005,
18204,-27245,18204,-27245,18204,-27245,18204,-27245,
17846,-27481,17846,-27481,17846,-27481,17846,-27481,
17484,-27712,17484,-27712,17484,-27712,17484,-27712,
17120,-27939,17120,-27939,17120,-27939,17120,-27939,
16753,-28161,16753,-28161,16753,-28161,16753,-28161,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
16010,-28590,16010,-28590,16010,-28590,16010,-28590,
15635,-28797,15635,-28797,15635,-28797,15635,-28797,
15256,-28999,15256,-28999,15256,-28999,15256,-28999,
14875,-29196,14875,-29196,14875,-29196,14875,-29196,
14492,-29388,14492,-29388,14492,-29388,14492,-29388,
14106,-29576,14106,-29576,14106,-29576,14106,-29576,
13718,-29758,13718,-29758,13718,-29758,13718,-29758,
13327,-29935,13327,-29935,13327,-29935,13327,-29935,
12934,-30107,12934,-30107,12934,-30107,12934,-30107,
12539,-30273,12539,-30273,12539,-30273,12539,-30273,
12142,-30435,12142,-30435,12142,-30435,12142,-30435,
11742,-30591,11742,-30591,11742,-30591,11742,-30591,
11341,-30742,11341,-30742,11341,-30742,11341,-30742,
10937,-30888,10937,-30888,10937,-30888,10937,-30888,
10532,-31029,10532,-31029,10532,-31029,10532,-31029,
10125,-31164,10125,-31164,10125,-31164,10125,-31164,
9716,-31294,9716,-31294,9716,-31294,9716,-31294,
9306,-31418,9306,-31418,9306,-31418,9306,-31418,
8894,-31537,8894,-31537,8894,-31537,8894,-31537,
8480,-31651,8480,-31651,8480,-31651,8480,-31651,
8065,-31759,8065,-31759,8065,-31759,8065,-31759,
7649,-31862,7649,-31862,7649,-31862,7649,-31862,
7231,-31960,7231,-31960,7231,-31960,7231,-31960,
6812,-32051,6812,-32051,6812,-32051,6812,-32051,
6392,-32138,6392,-32138,6392,-32138,6392,-32138,
5971,-32219,5971,-32219,5971,-32219,5971,-32219,
5549,-32294,5549,-32294,5549,-32294,5549,-32294,
5125,-32364,5125,-32364,5125,-32364,5125,-32364,
4701,-32428,4701,-32428,4701,-32428,4701,-32428,
4276,-32487,4276,-32487,4276,-32487,4276,-32487,
3851,-32540,3851,-32540,3851,-32540,3851,-32540,
3425,-32588,3425,-32588,3425,-32588,3425,-32588,
2998,-32630,2998,-32630,2998,-32630,2998,-32630,
2570,-32666,2570,-32666,2570,-32666,2570,-32666,
2143,-32697,2143,-32697,2143,-32697,2143,-32697,
1714,-32723,1714,-32723,1714,-32723,1714,-32723,
1286,-32742,1286,-32742,1286,-32742,1286,-32742,
857,-32756,857,-32756,857,-32756,857,-32756,
428,-32765,428,-32765,428,-32765,428,-32765,
0,-32767,0,-32767,0,-32767,0,-32767,
-429,-32765,-429,-32765,-429,-32765,-429,-32765,
-858,-32756,-858,-32756,-858,-32756,-858,-32756,
-1287,-32742,-1287,-32742,-1287,-32742,-1287,-32742,
-1715,-32723,-1715,-32723,-1715,-32723,-1715,-32723,
-2144,-32697,-2144,-32697,-2144,-32697,-2144,-32697,
-2571,-32666,-2571,-32666,-2571,-32666,-2571,-32666,
-2999,-32630,-2999,-32630,-2999,-32630,-2999,-32630,
-3426,-32588,-3426,-32588,-3426,-32588,-3426,-32588,
-3852,-32540,-3852,-32540,-3852,-32540,-3852,-32540,
-4277,-32487,-4277,-32487,-4277,-32487,-4277,-32487,
-4702,-32428,-4702,-32428,-4702,-32428,-4702,-32428,
-5126,-32364,-5126,-32364,-5126,-32364,-5126,-32364,
-5550,-32294,-5550,-32294,-5550,-32294,-5550,-32294,
-5972,-32219,-5972,-32219,-5972,-32219,-5972,-32219,
-6393,-32138,-6393,-32138,-6393,-32138,-6393,-32138,
-6813,-32051,-6813,-32051,-6813,-32051,-6813,-32051,
-7232,-31960,-7232,-31960,-7232,-31960,-7232,-31960,
-7650,-31862,-7650,-31862,-7650,-31862,-7650,-31862,
-8066,-31759,-8066,-31759,-8066,-31759,-8066,-31759,
-8481,-31651,-8481,-31651,-8481,-31651,-8481,-31651,
-8895,-31537,-8895,-31537,-8895,-31537,-8895,-31537,
-9307,-31418,-9307,-31418,-9307,-31418,-9307,-31418,
-9717,-31294,-9717,-31294,-9717,-31294,-9717,-31294,
-10126,-31164,-10126,-31164,-10126,-31164,-10126,-31164,
-10533,-31029,-10533,-31029,-10533,-31029,-10533,-31029,
-10938,-30888,-10938,-30888,-10938,-30888,-10938,-30888,
-11342,-30742,-11342,-30742,-11342,-30742,-11342,-30742,
-11743,-30591,-11743,-30591,-11743,-30591,-11743,-30591,
-12143,-30435,-12143,-30435,-12143,-30435,-12143,-30435,
-12540,-30273,-12540,-30273,-12540,-30273,-12540,-30273,
-12935,-30107,-12935,-30107,-12935,-30107,-12935,-30107,
-13328,-29935,-13328,-29935,-13328,-29935,-13328,-29935,
-13719,-29758,-13719,-29758,-13719,-29758,-13719,-29758,
-14107,-29576,-14107,-29576,-14107,-29576,-14107,-29576,
-14493,-29388,-14493,-29388,-14493,-29388,-14493,-29388,
-14876,-29196,-14876,-29196,-14876,-29196,-14876,-29196,
-15257,-28999,-15257,-28999,-15257,-28999,-15257,-28999,
-15636,-28797,-15636,-28797,-15636,-28797,-15636,-28797,
-16011,-28590,-16011,-28590,-16011,-28590,-16011,-28590,
-16384,-28378,-16384,-28378,-16384,-28378,-16384,-28378,
-16754,-28161,-16754,-28161,-16754,-28161,-16754,-28161,
-17121,-27939,-17121,-27939,-17121,-27939,-17121,-27939,
-17485,-27712,-17485,-27712,-17485,-27712,-17485,-27712,
-17847,-27481,-17847,-27481,-17847,-27481,-17847,-27481,
-18205,-27245,-18205,-27245,-18205,-27245,-18205,-27245,
-18560,-27005,-18560,-27005,-18560,-27005,-18560,-27005,
-18912,-26759,-18912,-26759,-18912,-26759,-18912,-26759,
-19260,-26510,-19260,-26510,-19260,-26510,-19260,-26510,
-19606,-26255,-19606,-26255,-19606,-26255,-19606,-26255,
-19948,-25996,-19948,-25996,-19948,-25996,-19948,-25996,
-20286,-25733,-20286,-25733,-20286,-25733,-20286,-25733,
-20621,-25465,-20621,-25465,-20621,-25465,-20621,-25465,
-20953,-25193,-20953,-25193,-20953,-25193,-20953,-25193,
-21281,-24917,-21281,-24917,-21281,-24917,-21281,-24917,
-21605,-24636,-21605,-24636,-21605,-24636,-21605,-24636,
-21926,-24351,-21926,-24351,-21926,-24351,-21926,-24351,
-22243,-24062,-22243,-24062,-22243,-24062,-22243,-24062,
-22556,-23769,-22556,-23769,-22556,-23769,-22556,-23769,
-22865,-23472,-22865,-23472,-22865,-23472,-22865,-23472,
-23170,-23170,-23170,-23170,-23170,-23170,-23170,-23170,
-23472,-22865,-23472,-22865,-23472,-22865,-23472,-22865,
-23769,-22556,-23769,-22556,-23769,-22556,-23769,-22556,
-24062,-22243,-24062,-22243,-24062,-22243,-24062,-22243,
-24351,-21926,-24351,-21926,-24351,-21926,-24351,-21926,
-24636,-21605,-24636,-21605,-24636,-21605,-24636,-21605,
-24917,-21281,-24917,-21281,-24917,-21281,-24917,-21281,
-25193,-20953,-25193,-20953,-25193,-20953,-25193,-20953,
-25465,-20621,-25465,-20621,-25465,-20621,-25465,-20621,
-25733,-20286,-25733,-20286,-25733,-20286,-25733,-20286,
-25996,-19948,-25996,-19948,-25996,-19948,-25996,-19948,
-26255,-19606,-26255,-19606,-26255,-19606,-26255,-19606,
-26510,-19260,-26510,-19260,-26510,-19260,-26510,-19260,
-26759,-18912,-26759,-18912,-26759,-18912,-26759,-18912,
-27005,-18560,-27005,-18560,-27005,-18560,-27005,-18560,
-27245,-18205,-27245,-18205,-27245,-18205,-27245,-18205,
-27481,-17847,-27481,-17847,-27481,-17847,-27481,-17847,
-27712,-17485,-27712,-17485,-27712,-17485,-27712,-17485,
-27939,-17121,-27939,-17121,-27939,-17121,-27939,-17121,
-28161,-16754,-28161,-16754,-28161,-16754,-28161,-16754,
-28378,-16384,-28378,-16384,-28378,-16384,-28378,-16384,
-28590,-16011,-28590,-16011,-28590,-16011,-28590,-16011,
-28797,-15636,-28797,-15636,-28797,-15636,-28797,-15636,
-28999,-15257,-28999,-15257,-28999,-15257,-28999,-15257,
-29196,-14876,-29196,-14876,-29196,-14876,-29196,-14876,
-29388,-14493,-29388,-14493,-29388,-14493,-29388,-14493,
-29576,-14107,-29576,-14107,-29576,-14107,-29576,-14107,
-29758,-13719,-29758,-13719,-29758,-13719,-29758,-13719,
-29935,-13328,-29935,-13328,-29935,-13328,-29935,-13328,
-30107,-12935,-30107,-12935,-30107,-12935,-30107,-12935,
-30273,-12540,-30273,-12540,-30273,-12540,-30273,-12540,
-30435,-12143,-30435,-12143,-30435,-12143,-30435,-12143,
-30591,-11743,-30591,-11743,-30591,-11743,-30591,-11743,
-30742,-11342,-30742,-11342,-30742,-11342,-30742,-11342,
-30888,-10938,-30888,-10938,-30888,-10938,-30888,-10938,
-31029,-10533,-31029,-10533,-31029,-10533,-31029,-10533,
-31164,-10126,-31164,-10126,-31164,-10126,-31164,-10126,
-31294,-9717,-31294,-9717,-31294,-9717,-31294,-9717,
-31418,-9307,-31418,-9307,-31418,-9307,-31418,-9307,
-31537,-8895,-31537,-8895,-31537,-8895,-31537,-8895,
-31651,-8481,-31651,-8481,-31651,-8481,-31651,-8481,
-31759,-8066,-31759,-8066,-31759,-8066,-31759,-8066,
-31862,-7650,-31862,-7650,-31862,-7650,-31862,-7650,
-31960,-7232,-31960,-7232,-31960,-7232,-31960,-7232,
-32051,-6813,-32051,-6813,-32051,-6813,-32051,-6813,
-32138,-6393,-32138,-6393,-32138,-6393,-32138,-6393,
-32219,-5972,-32219,-5972,-32219,-5972,-32219,-5972,
-32294,-5550,-32294,-5550,-32294,-5550,-32294,-5550,
-32364,-5126,-32364,-5126,-32364,-5126,-32364,-5126,
-32428,-4702,-32428,-4702,-32428,-4702,-32428,-4702,
-32487,-4277,-32487,-4277,-32487,-4277,-32487,-4277,
-32540,-3852,-32540,-3852,-32540,-3852,-32540,-3852,
-32588,-3426,-32588,-3426,-32588,-3426,-32588,-3426,
-32630,-2999,-32630,-2999,-32630,-2999,-32630,-2999,
-32666,-2571,-32666,-2571,-32666,-2571,-32666,-2571,
-32697,-2144,-32697,-2144,-32697,-2144,-32697,-2144,
-32723,-1715,-32723,-1715,-32723,-1715,-32723,-1715,
-32742,-1287,-32742,-1287,-32742,-1287,-32742,-1287,
-32756,-858,-32756,-858,-32756,-858,-32756,-858,
-32765,-429,-32765,-429,-32765,-429,-32765,-429};

static int16_t twc960[239*2*4] = {32760,-644,32760,-644,32760,-644,32760,-644,
32741,-1287,32741,-1287,32741,-1287,32741,-1287,
32710,-1930,32710,-1930,32710,-1930,32710,-1930,
32665,-2571,32665,-2571,32665,-2571,32665,-2571,
32609,-3212,32609,-3212,32609,-3212,32609,-3212,
32539,-3852,32539,-3852,32539,-3852,32539,-3852,
32457,-4490,32457,-4490,32457,-4490,32457,-4490,
32363,-5126,32363,-5126,32363,-5126,32363,-5126,
32256,-5761,32256,-5761,32256,-5761,32256,-5761,
32137,-6393,32137,-6393,32137,-6393,32137,-6393,
32005,-7023,32005,-7023,32005,-7023,32005,-7023,
31861,-7650,31861,-7650,31861,-7650,31861,-7650,
31705,-8274,31705,-8274,31705,-8274,31705,-8274,
31536,-8895,31536,-8895,31536,-8895,31536,-8895,
31356,-9512,31356,-9512,31356,-9512,31356,-9512,
31163,-10126,31163,-10126,31163,-10126,31163,-10126,
30958,-10736,30958,-10736,30958,-10736,30958,-10736,
30741,-11342,30741,-11342,30741,-11342,30741,-11342,
30513,-11943,30513,-11943,30513,-11943,30513,-11943,
30272,-12540,30272,-12540,30272,-12540,30272,-12540,
30020,-13132,30020,-13132,30020,-13132,30020,-13132,
29757,-13719,29757,-13719,29757,-13719,29757,-13719,
29482,-14300,29482,-14300,29482,-14300,29482,-14300,
29195,-14876,29195,-14876,29195,-14876,29195,-14876,
28897,-15447,28897,-15447,28897,-15447,28897,-15447,
28589,-16011,28589,-16011,28589,-16011,28589,-16011,
28269,-16569,28269,-16569,28269,-16569,28269,-16569,
27938,-17121,27938,-17121,27938,-17121,27938,-17121,
27596,-17666,27596,-17666,27596,-17666,27596,-17666,
27244,-18205,27244,-18205,27244,-18205,27244,-18205,
26882,-18736,26882,-18736,26882,-18736,26882,-18736,
26509,-19260,26509,-19260,26509,-19260,26509,-19260,
26125,-19777,26125,-19777,26125,-19777,26125,-19777,
25732,-20286,25732,-20286,25732,-20286,25732,-20286,
25329,-20788,25329,-20788,25329,-20788,25329,-20788,
24916,-21281,24916,-21281,24916,-21281,24916,-21281,
24493,-21766,24493,-21766,24493,-21766,24493,-21766,
24061,-22243,24061,-22243,24061,-22243,24061,-22243,
23620,-22711,23620,-22711,23620,-22711,23620,-22711,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
22710,-23621,22710,-23621,22710,-23621,22710,-23621,
22242,-24062,22242,-24062,22242,-24062,22242,-24062,
21765,-24494,21765,-24494,21765,-24494,21765,-24494,
21280,-24917,21280,-24917,21280,-24917,21280,-24917,
20787,-25330,20787,-25330,20787,-25330,20787,-25330,
20285,-25733,20285,-25733,20285,-25733,20285,-25733,
19776,-26126,19776,-26126,19776,-26126,19776,-26126,
19259,-26510,19259,-26510,19259,-26510,19259,-26510,
18735,-26883,18735,-26883,18735,-26883,18735,-26883,
18204,-27245,18204,-27245,18204,-27245,18204,-27245,
17665,-27597,17665,-27597,17665,-27597,17665,-27597,
17120,-27939,17120,-27939,17120,-27939,17120,-27939,
16568,-28270,16568,-28270,16568,-28270,16568,-28270,
16010,-28590,16010,-28590,16010,-28590,16010,-28590,
15446,-28898,15446,-28898,15446,-28898,15446,-28898,
14875,-29196,14875,-29196,14875,-29196,14875,-29196,
14299,-29483,14299,-29483,14299,-29483,14299,-29483,
13718,-29758,13718,-29758,13718,-29758,13718,-29758,
13131,-30021,13131,-30021,13131,-30021,13131,-30021,
12539,-30273,12539,-30273,12539,-30273,12539,-30273,
11942,-30514,11942,-30514,11942,-30514,11942,-30514,
11341,-30742,11341,-30742,11341,-30742,11341,-30742,
10735,-30959,10735,-30959,10735,-30959,10735,-30959,
10125,-31164,10125,-31164,10125,-31164,10125,-31164,
9511,-31357,9511,-31357,9511,-31357,9511,-31357,
8894,-31537,8894,-31537,8894,-31537,8894,-31537,
8273,-31706,8273,-31706,8273,-31706,8273,-31706,
7649,-31862,7649,-31862,7649,-31862,7649,-31862,
7022,-32006,7022,-32006,7022,-32006,7022,-32006,
6392,-32138,6392,-32138,6392,-32138,6392,-32138,
5760,-32257,5760,-32257,5760,-32257,5760,-32257,
5125,-32364,5125,-32364,5125,-32364,5125,-32364,
4489,-32458,4489,-32458,4489,-32458,4489,-32458,
3851,-32540,3851,-32540,3851,-32540,3851,-32540,
3211,-32610,3211,-32610,3211,-32610,3211,-32610,
2570,-32666,2570,-32666,2570,-32666,2570,-32666,
1929,-32711,1929,-32711,1929,-32711,1929,-32711,
1286,-32742,1286,-32742,1286,-32742,1286,-32742,
643,-32761,643,-32761,643,-32761,643,-32761,
0,-32767,0,-32767,0,-32767,0,-32767,
-644,-32761,-644,-32761,-644,-32761,-644,-32761,
-1287,-32742,-1287,-32742,-1287,-32742,-1287,-32742,
-1930,-32711,-1930,-32711,-1930,-32711,-1930,-32711,
-2571,-32666,-2571,-32666,-2571,-32666,-2571,-32666,
-3212,-32610,-3212,-32610,-3212,-32610,-3212,-32610,
-3852,-32540,-3852,-32540,-3852,-32540,-3852,-32540,
-4490,-32458,-4490,-32458,-4490,-32458,-4490,-32458,
-5126,-32364,-5126,-32364,-5126,-32364,-5126,-32364,
-5761,-32257,-5761,-32257,-5761,-32257,-5761,-32257,
-6393,-32138,-6393,-32138,-6393,-32138,-6393,-32138,
-7023,-32006,-7023,-32006,-7023,-32006,-7023,-32006,
-7650,-31862,-7650,-31862,-7650,-31862,-7650,-31862,
-8274,-31706,-8274,-31706,-8274,-31706,-8274,-31706,
-8895,-31537,-8895,-31537,-8895,-31537,-8895,-31537,
-9512,-31357,-9512,-31357,-9512,-31357,-9512,-31357,
-10126,-31164,-10126,-31164,-10126,-31164,-10126,-31164,
-10736,-30959,-10736,-30959,-10736,-30959,-10736,-30959,
-11342,-30742,-11342,-30742,-11342,-30742,-11342,-30742,
-11943,-30514,-11943,-30514,-11943,-30514,-11943,-30514,
-12540,-30273,-12540,-30273,-12540,-30273,-12540,-30273,
-13132,-30021,-13132,-30021,-13132,-30021,-13132,-30021,
-13719,-29758,-13719,-29758,-13719,-29758,-13719,-29758,
-14300,-29483,-14300,-29483,-14300,-29483,-14300,-29483,
-14876,-29196,-14876,-29196,-14876,-29196,-14876,-29196,
-15447,-28898,-15447,-28898,-15447,-28898,-15447,-28898,
-16011,-28590,-16011,-28590,-16011,-28590,-16011,-28590,
-16569,-28270,-16569,-28270,-16569,-28270,-16569,-28270,
-17121,-27939,-17121,-27939,-17121,-27939,-17121,-27939,
-17666,-27597,-17666,-27597,-17666,-27597,-17666,-27597,
-18205,-27245,-18205,-27245,-18205,-27245,-18205,-27245,
-18736,-26883,-18736,-26883,-18736,-26883,-18736,-26883,
-19260,-26510,-19260,-26510,-19260,-26510,-19260,-26510,
-19777,-26126,-19777,-26126,-19777,-26126,-19777,-26126,
-20286,-25733,-20286,-25733,-20286,-25733,-20286,-25733,
-20788,-25330,-20788,-25330,-20788,-25330,-20788,-25330,
-21281,-24917,-21281,-24917,-21281,-24917,-21281,-24917,
-21766,-24494,-21766,-24494,-21766,-24494,-21766,-24494,
-22243,-24062,-22243,-24062,-22243,-24062,-22243,-24062,
-22711,-23621,-22711,-23621,-22711,-23621,-22711,-23621,
-23170,-23170,-23170,-23170,-23170,-23170,-23170,-23170,
-23621,-22711,-23621,-22711,-23621,-22711,-23621,-22711,
-24062,-22243,-24062,-22243,-24062,-22243,-24062,-22243,
-24494,-21766,-24494,-21766,-24494,-21766,-24494,-21766,
-24917,-21281,-24917,-21281,-24917,-21281,-24917,-21281,
-25330,-20788,-25330,-20788,-25330,-20788,-25330,-20788,
-25733,-20286,-25733,-20286,-25733,-20286,-25733,-20286,
-26126,-19777,-26126,-19777,-26126,-19777,-26126,-19777,
-26510,-19260,-26510,-19260,-26510,-19260,-26510,-19260,
-26883,-18736,-26883,-18736,-26883,-18736,-26883,-18736,
-27245,-18205,-27245,-18205,-27245,-18205,-27245,-18205,
-27597,-17666,-27597,-17666,-27597,-17666,-27597,-17666,
-27939,-17121,-27939,-17121,-27939,-17121,-27939,-17121,
-28270,-16569,-28270,-16569,-28270,-16569,-28270,-16569,
-28590,-16011,-28590,-16011,-28590,-16011,-28590,-16011,
-28898,-15447,-28898,-15447,-28898,-15447,-28898,-15447,
-29196,-14876,-29196,-14876,-29196,-14876,-29196,-14876,
-29483,-14300,-29483,-14300,-29483,-14300,-29483,-14300,
-29758,-13719,-29758,-13719,-29758,-13719,-29758,-13719,
-30021,-13132,-30021,-13132,-30021,-13132,-30021,-13132,
-30273,-12540,-30273,-12540,-30273,-12540,-30273,-12540,
-30514,-11943,-30514,-11943,-30514,-11943,-30514,-11943,
-30742,-11342,-30742,-11342,-30742,-11342,-30742,-11342,
-30959,-10736,-30959,-10736,-30959,-10736,-30959,-10736,
-31164,-10126,-31164,-10126,-31164,-10126,-31164,-10126,
-31357,-9512,-31357,-9512,-31357,-9512,-31357,-9512,
-31537,-8895,-31537,-8895,-31537,-8895,-31537,-8895,
-31706,-8274,-31706,-8274,-31706,-8274,-31706,-8274,
-31862,-7650,-31862,-7650,-31862,-7650,-31862,-7650,
-32006,-7023,-32006,-7023,-32006,-7023,-32006,-7023,
-32138,-6393,-32138,-6393,-32138,-6393,-32138,-6393,
-32257,-5761,-32257,-5761,-32257,-5761,-32257,-5761,
-32364,-5126,-32364,-5126,-32364,-5126,-32364,-5126,
-32458,-4490,-32458,-4490,-32458,-4490,-32458,-4490,
-32540,-3852,-32540,-3852,-32540,-3852,-32540,-3852,
-32610,-3212,-32610,-3212,-32610,-3212,-32610,-3212,
-32666,-2571,-32666,-2571,-32666,-2571,-32666,-2571,
-32711,-1930,-32711,-1930,-32711,-1930,-32711,-1930,
-32742,-1287,-32742,-1287,-32742,-1287,-32742,-1287,
-32761,-644,-32761,-644,-32761,-644,-32761,-644,
-32767,-1,-32767,-1,-32767,-1,-32767,-1,
-32761,643,-32761,643,-32761,643,-32761,643,
-32742,1286,-32742,1286,-32742,1286,-32742,1286,
-32711,1929,-32711,1929,-32711,1929,-32711,1929,
-32666,2570,-32666,2570,-32666,2570,-32666,2570,
-32610,3211,-32610,3211,-32610,3211,-32610,3211,
-32540,3851,-32540,3851,-32540,3851,-32540,3851,
-32458,4489,-32458,4489,-32458,4489,-32458,4489,
-32364,5125,-32364,5125,-32364,5125,-32364,5125,
-32257,5760,-32257,5760,-32257,5760,-32257,5760,
-32138,6392,-32138,6392,-32138,6392,-32138,6392,
-32006,7022,-32006,7022,-32006,7022,-32006,7022,
-31862,7649,-31862,7649,-31862,7649,-31862,7649,
-31706,8273,-31706,8273,-31706,8273,-31706,8273,
-31537,8894,-31537,8894,-31537,8894,-31537,8894,
-31357,9511,-31357,9511,-31357,9511,-31357,9511,
-31164,10125,-31164,10125,-31164,10125,-31164,10125,
-30959,10735,-30959,10735,-30959,10735,-30959,10735,
-30742,11341,-30742,11341,-30742,11341,-30742,11341,
-30514,11942,-30514,11942,-30514,11942,-30514,11942,
-30273,12539,-30273,12539,-30273,12539,-30273,12539,
-30021,13131,-30021,13131,-30021,13131,-30021,13131,
-29758,13718,-29758,13718,-29758,13718,-29758,13718,
-29483,14299,-29483,14299,-29483,14299,-29483,14299,
-29196,14875,-29196,14875,-29196,14875,-29196,14875,
-28898,15446,-28898,15446,-28898,15446,-28898,15446,
-28590,16010,-28590,16010,-28590,16010,-28590,16010,
-28270,16568,-28270,16568,-28270,16568,-28270,16568,
-27939,17120,-27939,17120,-27939,17120,-27939,17120,
-27597,17665,-27597,17665,-27597,17665,-27597,17665,
-27245,18204,-27245,18204,-27245,18204,-27245,18204,
-26883,18735,-26883,18735,-26883,18735,-26883,18735,
-26510,19259,-26510,19259,-26510,19259,-26510,19259,
-26126,19776,-26126,19776,-26126,19776,-26126,19776,
-25733,20285,-25733,20285,-25733,20285,-25733,20285,
-25330,20787,-25330,20787,-25330,20787,-25330,20787,
-24917,21280,-24917,21280,-24917,21280,-24917,21280,
-24494,21765,-24494,21765,-24494,21765,-24494,21765,
-24062,22242,-24062,22242,-24062,22242,-24062,22242,
-23621,22710,-23621,22710,-23621,22710,-23621,22710,
-23170,23169,-23170,23169,-23170,23169,-23170,23169,
-22711,23620,-22711,23620,-22711,23620,-22711,23620,
-22243,24061,-22243,24061,-22243,24061,-22243,24061,
-21766,24493,-21766,24493,-21766,24493,-21766,24493,
-21281,24916,-21281,24916,-21281,24916,-21281,24916,
-20788,25329,-20788,25329,-20788,25329,-20788,25329,
-20286,25732,-20286,25732,-20286,25732,-20286,25732,
-19777,26125,-19777,26125,-19777,26125,-19777,26125,
-19260,26509,-19260,26509,-19260,26509,-19260,26509,
-18736,26882,-18736,26882,-18736,26882,-18736,26882,
-18205,27244,-18205,27244,-18205,27244,-18205,27244,
-17666,27596,-17666,27596,-17666,27596,-17666,27596,
-17121,27938,-17121,27938,-17121,27938,-17121,27938,
-16569,28269,-16569,28269,-16569,28269,-16569,28269,
-16011,28589,-16011,28589,-16011,28589,-16011,28589,
-15447,28897,-15447,28897,-15447,28897,-15447,28897,
-14876,29195,-14876,29195,-14876,29195,-14876,29195,
-14300,29482,-14300,29482,-14300,29482,-14300,29482,
-13719,29757,-13719,29757,-13719,29757,-13719,29757,
-13132,30020,-13132,30020,-13132,30020,-13132,30020,
-12540,30272,-12540,30272,-12540,30272,-12540,30272,
-11943,30513,-11943,30513,-11943,30513,-11943,30513,
-11342,30741,-11342,30741,-11342,30741,-11342,30741,
-10736,30958,-10736,30958,-10736,30958,-10736,30958,
-10126,31163,-10126,31163,-10126,31163,-10126,31163,
-9512,31356,-9512,31356,-9512,31356,-9512,31356,
-8895,31536,-8895,31536,-8895,31536,-8895,31536,
-8274,31705,-8274,31705,-8274,31705,-8274,31705,
-7650,31861,-7650,31861,-7650,31861,-7650,31861,
-7023,32005,-7023,32005,-7023,32005,-7023,32005,
-6393,32137,-6393,32137,-6393,32137,-6393,32137,
-5761,32256,-5761,32256,-5761,32256,-5761,32256,
-5126,32363,-5126,32363,-5126,32363,-5126,32363,
-4490,32457,-4490,32457,-4490,32457,-4490,32457,
-3852,32539,-3852,32539,-3852,32539,-3852,32539,
-3212,32609,-3212,32609,-3212,32609,-3212,32609,
-2571,32665,-2571,32665,-2571,32665,-2571,32665,
-1930,32710,-1930,32710,-1930,32710,-1930,32710,
-1287,32741,-1287,32741,-1287,32741,-1287,32741,
-644,32760,-644,32760,-644,32760,-644,32760};

void dft960(int16_t *x,int16_t *y,unsigned char scale_flag){ // 240 x 4
  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa960[0];
  __m128i *twb128=(__m128i *)&twb960[0];
  __m128i *twc128=(__m128i *)&twc960[0];
  __m128i x2128[960];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[960];//=&ytmp128array2[0];



  for (i=0,j=0;i<240;i++,j+=4) {
    x2128[i]    = x128[j];
    x2128[i+240] = x128[j+1];
    x2128[i+480] = x128[j+2];
    x2128[i+720] = x128[j+3];
  }

  dft240((int16_t *)x2128,(int16_t *)ytmp128,1);
  dft240((int16_t *)(x2128+240),(int16_t *)(ytmp128+240),1);
  dft240((int16_t *)(x2128+480),(int16_t *)(ytmp128+480),1);
  dft240((int16_t *)(x2128+720),(int16_t *)(ytmp128+720),1);

  bfly4_tw1(ytmp128,ytmp128+240,ytmp128+480,ytmp128+720,y128,y128+240,y128+480,y128+720);
  for (i=1,j=0;i<240;i++,j++) {
    bfly4(ytmp128+i,
	  ytmp128+240+i,
	  ytmp128+480+i,
	  ytmp128+720+i,
	  y128+i,
	  y128+240+i,
	  y128+480+i,
	  y128+720+i,
	  twa128+j,
	  twb128+j,
	  twc128+j);
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(16384);//dft_norm_table[13]);
    
    for (i=0;i<960;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

};

/* Twiddles generated with
twa = floor(32767*exp(-sqrt(-1)*2*pi*(1:323)/972));
twb = floor(32767*exp(-sqrt(-1)*2*pi*2*(1:323)/972));
twa2 = zeros(1,2*323);
twb2 = zeros(1,2*323);
twa2(1:2:end) = real(twa);
twa2(2:2:end) = imag(twa);
twb2(1:2:end) = real(twb);
twb2(2:2:end) = imag(twb);
fd=fopen("twiddle_tmp.txt","w");
fprintf(fd,"static int16_t twa972[323*2*4] = {");
for i=1:2:(2*322)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
fprintf(fd,"static int16_t twb972[323*2*4] = {");
for i=1:2:(2*322)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
fclose(fd);
*/
static int16_t twa972[323*2*4] = {32766,-212,32766,-212,32766,-212,32766,-212,
32764,-424,32764,-424,32764,-424,32764,-424,
32760,-636,32760,-636,32760,-636,32760,-636,
32756,-848,32756,-848,32756,-848,32756,-848,
32749,-1059,32749,-1059,32749,-1059,32749,-1059,
32742,-1271,32742,-1271,32742,-1271,32742,-1271,
32733,-1483,32733,-1483,32733,-1483,32733,-1483,
32723,-1694,32723,-1694,32723,-1694,32723,-1694,
32711,-1906,32711,-1906,32711,-1906,32711,-1906,
32698,-2117,32698,-2117,32698,-2117,32698,-2117,
32684,-2328,32684,-2328,32684,-2328,32684,-2328,
32668,-2540,32668,-2540,32668,-2540,32668,-2540,
32651,-2751,32651,-2751,32651,-2751,32651,-2751,
32632,-2962,32632,-2962,32632,-2962,32632,-2962,
32613,-3173,32613,-3173,32613,-3173,32613,-3173,
32591,-3383,32591,-3383,32591,-3383,32591,-3383,
32569,-3594,32569,-3594,32569,-3594,32569,-3594,
32545,-3805,32545,-3805,32545,-3805,32545,-3805,
32520,-4015,32520,-4015,32520,-4015,32520,-4015,
32493,-4225,32493,-4225,32493,-4225,32493,-4225,
32465,-4435,32465,-4435,32465,-4435,32465,-4435,
32436,-4645,32436,-4645,32436,-4645,32436,-4645,
32405,-4854,32405,-4854,32405,-4854,32405,-4854,
32373,-5064,32373,-5064,32373,-5064,32373,-5064,
32340,-5273,32340,-5273,32340,-5273,32340,-5273,
32305,-5482,32305,-5482,32305,-5482,32305,-5482,
32269,-5690,32269,-5690,32269,-5690,32269,-5690,
32231,-5899,32231,-5899,32231,-5899,32231,-5899,
32192,-6107,32192,-6107,32192,-6107,32192,-6107,
32152,-6315,32152,-6315,32152,-6315,32152,-6315,
32111,-6523,32111,-6523,32111,-6523,32111,-6523,
32068,-6730,32068,-6730,32068,-6730,32068,-6730,
32024,-6937,32024,-6937,32024,-6937,32024,-6937,
31978,-7144,31978,-7144,31978,-7144,31978,-7144,
31931,-7351,31931,-7351,31931,-7351,31931,-7351,
31883,-7557,31883,-7557,31883,-7557,31883,-7557,
31834,-7763,31834,-7763,31834,-7763,31834,-7763,
31783,-7969,31783,-7969,31783,-7969,31783,-7969,
31731,-8174,31731,-8174,31731,-8174,31731,-8174,
31677,-8379,31677,-8379,31677,-8379,31677,-8379,
31622,-8583,31622,-8583,31622,-8583,31622,-8583,
31566,-8788,31566,-8788,31566,-8788,31566,-8788,
31509,-8992,31509,-8992,31509,-8992,31509,-8992,
31450,-9195,31450,-9195,31450,-9195,31450,-9195,
31390,-9398,31390,-9398,31390,-9398,31390,-9398,
31329,-9601,31329,-9601,31329,-9601,31329,-9601,
31266,-9803,31266,-9803,31266,-9803,31266,-9803,
31202,-10005,31202,-10005,31202,-10005,31202,-10005,
31136,-10207,31136,-10207,31136,-10207,31136,-10207,
31070,-10408,31070,-10408,31070,-10408,31070,-10408,
31002,-10608,31002,-10608,31002,-10608,31002,-10608,
30933,-10808,30933,-10808,30933,-10808,30933,-10808,
30862,-11008,30862,-11008,30862,-11008,30862,-11008,
30790,-11207,30790,-11207,30790,-11207,30790,-11207,
30717,-11406,30717,-11406,30717,-11406,30717,-11406,
30643,-11605,30643,-11605,30643,-11605,30643,-11605,
30567,-11802,30567,-11802,30567,-11802,30567,-11802,
30490,-12000,30490,-12000,30490,-12000,30490,-12000,
30412,-12197,30412,-12197,30412,-12197,30412,-12197,
30333,-12393,30333,-12393,30333,-12393,30333,-12393,
30252,-12589,30252,-12589,30252,-12589,30252,-12589,
30170,-12784,30170,-12784,30170,-12784,30170,-12784,
30087,-12979,30087,-12979,30087,-12979,30087,-12979,
30002,-13173,30002,-13173,30002,-13173,30002,-13173,
29916,-13367,29916,-13367,29916,-13367,29916,-13367,
29829,-13560,29829,-13560,29829,-13560,29829,-13560,
29741,-13752,29741,-13752,29741,-13752,29741,-13752,
29652,-13944,29652,-13944,29652,-13944,29652,-13944,
29561,-14136,29561,-14136,29561,-14136,29561,-14136,
29469,-14327,29469,-14327,29469,-14327,29469,-14327,
29376,-14517,29376,-14517,29376,-14517,29376,-14517,
29281,-14706,29281,-14706,29281,-14706,29281,-14706,
29185,-14895,29185,-14895,29185,-14895,29185,-14895,
29089,-15084,29089,-15084,29089,-15084,29089,-15084,
28990,-15271,28990,-15271,28990,-15271,28990,-15271,
28891,-15458,28891,-15458,28891,-15458,28891,-15458,
28791,-15645,28791,-15645,28791,-15645,28791,-15645,
28689,-15831,28689,-15831,28689,-15831,28689,-15831,
28586,-16016,28586,-16016,28586,-16016,28586,-16016,
28482,-16200,28482,-16200,28482,-16200,28482,-16200,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
28270,-16567,28270,-16567,28270,-16567,28270,-16567,
28162,-16749,28162,-16749,28162,-16749,28162,-16749,
28054,-16931,28054,-16931,28054,-16931,28054,-16931,
27943,-17112,27943,-17112,27943,-17112,27943,-17112,
27832,-17292,27832,-17292,27832,-17292,27832,-17292,
27720,-17472,27720,-17472,27720,-17472,27720,-17472,
27606,-17651,27606,-17651,27606,-17651,27606,-17651,
27492,-17829,27492,-17829,27492,-17829,27492,-17829,
27376,-18006,27376,-18006,27376,-18006,27376,-18006,
27259,-18183,27259,-18183,27259,-18183,27259,-18183,
27141,-18359,27141,-18359,27141,-18359,27141,-18359,
27022,-18534,27022,-18534,27022,-18534,27022,-18534,
26901,-18708,26901,-18708,26901,-18708,26901,-18708,
26780,-18882,26780,-18882,26780,-18882,26780,-18882,
26657,-19054,26657,-19054,26657,-19054,26657,-19054,
26533,-19226,26533,-19226,26533,-19226,26533,-19226,
26409,-19397,26409,-19397,26409,-19397,26409,-19397,
26283,-19568,26283,-19568,26283,-19568,26283,-19568,
26156,-19737,26156,-19737,26156,-19737,26156,-19737,
26028,-19906,26028,-19906,26028,-19906,26028,-19906,
25898,-20074,25898,-20074,25898,-20074,25898,-20074,
25768,-20241,25768,-20241,25768,-20241,25768,-20241,
25637,-20407,25637,-20407,25637,-20407,25637,-20407,
25504,-20572,25504,-20572,25504,-20572,25504,-20572,
25371,-20736,25371,-20736,25371,-20736,25371,-20736,
25236,-20900,25236,-20900,25236,-20900,25236,-20900,
25100,-21063,25100,-21063,25100,-21063,25100,-21063,
24964,-21225,24964,-21225,24964,-21225,24964,-21225,
24826,-21385,24826,-21385,24826,-21385,24826,-21385,
24687,-21546,24687,-21546,24687,-21546,24687,-21546,
24548,-21705,24548,-21705,24548,-21705,24548,-21705,
24407,-21863,24407,-21863,24407,-21863,24407,-21863,
24265,-22020,24265,-22020,24265,-22020,24265,-22020,
24122,-22177,24122,-22177,24122,-22177,24122,-22177,
23978,-22332,23978,-22332,23978,-22332,23978,-22332,
23833,-22487,23833,-22487,23833,-22487,23833,-22487,
23688,-22640,23688,-22640,23688,-22640,23688,-22640,
23541,-22793,23541,-22793,23541,-22793,23541,-22793,
23393,-22945,23393,-22945,23393,-22945,23393,-22945,
23244,-23095,23244,-23095,23244,-23095,23244,-23095,
23094,-23245,23094,-23245,23094,-23245,23094,-23245,
22944,-23394,22944,-23394,22944,-23394,22944,-23394,
22792,-23542,22792,-23542,22792,-23542,22792,-23542,
22639,-23689,22639,-23689,22639,-23689,22639,-23689,
22486,-23834,22486,-23834,22486,-23834,22486,-23834,
22331,-23979,22331,-23979,22331,-23979,22331,-23979,
22176,-24123,22176,-24123,22176,-24123,22176,-24123,
22019,-24266,22019,-24266,22019,-24266,22019,-24266,
21862,-24408,21862,-24408,21862,-24408,21862,-24408,
21704,-24549,21704,-24549,21704,-24549,21704,-24549,
21545,-24688,21545,-24688,21545,-24688,21545,-24688,
21384,-24827,21384,-24827,21384,-24827,21384,-24827,
21224,-24965,21224,-24965,21224,-24965,21224,-24965,
21062,-25101,21062,-25101,21062,-25101,21062,-25101,
20899,-25237,20899,-25237,20899,-25237,20899,-25237,
20735,-25372,20735,-25372,20735,-25372,20735,-25372,
20571,-25505,20571,-25505,20571,-25505,20571,-25505,
20406,-25638,20406,-25638,20406,-25638,20406,-25638,
20240,-25769,20240,-25769,20240,-25769,20240,-25769,
20073,-25899,20073,-25899,20073,-25899,20073,-25899,
19905,-26029,19905,-26029,19905,-26029,19905,-26029,
19736,-26157,19736,-26157,19736,-26157,19736,-26157,
19567,-26284,19567,-26284,19567,-26284,19567,-26284,
19396,-26410,19396,-26410,19396,-26410,19396,-26410,
19225,-26534,19225,-26534,19225,-26534,19225,-26534,
19053,-26658,19053,-26658,19053,-26658,19053,-26658,
18881,-26781,18881,-26781,18881,-26781,18881,-26781,
18707,-26902,18707,-26902,18707,-26902,18707,-26902,
18533,-27023,18533,-27023,18533,-27023,18533,-27023,
18358,-27142,18358,-27142,18358,-27142,18358,-27142,
18182,-27260,18182,-27260,18182,-27260,18182,-27260,
18005,-27377,18005,-27377,18005,-27377,18005,-27377,
17828,-27493,17828,-27493,17828,-27493,17828,-27493,
17650,-27607,17650,-27607,17650,-27607,17650,-27607,
17471,-27721,17471,-27721,17471,-27721,17471,-27721,
17291,-27833,17291,-27833,17291,-27833,17291,-27833,
17111,-27944,17111,-27944,17111,-27944,17111,-27944,
16930,-28055,16930,-28055,16930,-28055,16930,-28055,
16748,-28163,16748,-28163,16748,-28163,16748,-28163,
16566,-28271,16566,-28271,16566,-28271,16566,-28271,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
16199,-28483,16199,-28483,16199,-28483,16199,-28483,
16015,-28587,16015,-28587,16015,-28587,16015,-28587,
15830,-28690,15830,-28690,15830,-28690,15830,-28690,
15644,-28792,15644,-28792,15644,-28792,15644,-28792,
15457,-28892,15457,-28892,15457,-28892,15457,-28892,
15270,-28991,15270,-28991,15270,-28991,15270,-28991,
15083,-29090,15083,-29090,15083,-29090,15083,-29090,
14894,-29186,14894,-29186,14894,-29186,14894,-29186,
14705,-29282,14705,-29282,14705,-29282,14705,-29282,
14516,-29377,14516,-29377,14516,-29377,14516,-29377,
14326,-29470,14326,-29470,14326,-29470,14326,-29470,
14135,-29562,14135,-29562,14135,-29562,14135,-29562,
13943,-29653,13943,-29653,13943,-29653,13943,-29653,
13751,-29742,13751,-29742,13751,-29742,13751,-29742,
13559,-29830,13559,-29830,13559,-29830,13559,-29830,
13366,-29917,13366,-29917,13366,-29917,13366,-29917,
13172,-30003,13172,-30003,13172,-30003,13172,-30003,
12978,-30088,12978,-30088,12978,-30088,12978,-30088,
12783,-30171,12783,-30171,12783,-30171,12783,-30171,
12588,-30253,12588,-30253,12588,-30253,12588,-30253,
12392,-30334,12392,-30334,12392,-30334,12392,-30334,
12196,-30413,12196,-30413,12196,-30413,12196,-30413,
11999,-30491,11999,-30491,11999,-30491,11999,-30491,
11801,-30568,11801,-30568,11801,-30568,11801,-30568,
11604,-30644,11604,-30644,11604,-30644,11604,-30644,
11405,-30718,11405,-30718,11405,-30718,11405,-30718,
11206,-30791,11206,-30791,11206,-30791,11206,-30791,
11007,-30863,11007,-30863,11007,-30863,11007,-30863,
10807,-30934,10807,-30934,10807,-30934,10807,-30934,
10607,-31003,10607,-31003,10607,-31003,10607,-31003,
10407,-31071,10407,-31071,10407,-31071,10407,-31071,
10206,-31137,10206,-31137,10206,-31137,10206,-31137,
10004,-31203,10004,-31203,10004,-31203,10004,-31203,
9802,-31267,9802,-31267,9802,-31267,9802,-31267,
9600,-31330,9600,-31330,9600,-31330,9600,-31330,
9397,-31391,9397,-31391,9397,-31391,9397,-31391,
9194,-31451,9194,-31451,9194,-31451,9194,-31451,
8991,-31510,8991,-31510,8991,-31510,8991,-31510,
8787,-31567,8787,-31567,8787,-31567,8787,-31567,
8582,-31623,8582,-31623,8582,-31623,8582,-31623,
8378,-31678,8378,-31678,8378,-31678,8378,-31678,
8173,-31732,8173,-31732,8173,-31732,8173,-31732,
7968,-31784,7968,-31784,7968,-31784,7968,-31784,
7762,-31835,7762,-31835,7762,-31835,7762,-31835,
7556,-31884,7556,-31884,7556,-31884,7556,-31884,
7350,-31932,7350,-31932,7350,-31932,7350,-31932,
7143,-31979,7143,-31979,7143,-31979,7143,-31979,
6936,-32025,6936,-32025,6936,-32025,6936,-32025,
6729,-32069,6729,-32069,6729,-32069,6729,-32069,
6522,-32112,6522,-32112,6522,-32112,6522,-32112,
6314,-32153,6314,-32153,6314,-32153,6314,-32153,
6106,-32193,6106,-32193,6106,-32193,6106,-32193,
5898,-32232,5898,-32232,5898,-32232,5898,-32232,
5689,-32270,5689,-32270,5689,-32270,5689,-32270,
5481,-32306,5481,-32306,5481,-32306,5481,-32306,
5272,-32341,5272,-32341,5272,-32341,5272,-32341,
5063,-32374,5063,-32374,5063,-32374,5063,-32374,
4853,-32406,4853,-32406,4853,-32406,4853,-32406,
4644,-32437,4644,-32437,4644,-32437,4644,-32437,
4434,-32466,4434,-32466,4434,-32466,4434,-32466,
4224,-32494,4224,-32494,4224,-32494,4224,-32494,
4014,-32521,4014,-32521,4014,-32521,4014,-32521,
3804,-32546,3804,-32546,3804,-32546,3804,-32546,
3593,-32570,3593,-32570,3593,-32570,3593,-32570,
3382,-32592,3382,-32592,3382,-32592,3382,-32592,
3172,-32614,3172,-32614,3172,-32614,3172,-32614,
2961,-32633,2961,-32633,2961,-32633,2961,-32633,
2750,-32652,2750,-32652,2750,-32652,2750,-32652,
2539,-32669,2539,-32669,2539,-32669,2539,-32669,
2327,-32685,2327,-32685,2327,-32685,2327,-32685,
2116,-32699,2116,-32699,2116,-32699,2116,-32699,
1905,-32712,1905,-32712,1905,-32712,1905,-32712,
1693,-32724,1693,-32724,1693,-32724,1693,-32724,
1482,-32734,1482,-32734,1482,-32734,1482,-32734,
1270,-32743,1270,-32743,1270,-32743,1270,-32743,
1058,-32750,1058,-32750,1058,-32750,1058,-32750,
847,-32757,847,-32757,847,-32757,847,-32757,
635,-32761,635,-32761,635,-32761,635,-32761,
423,-32765,423,-32765,423,-32765,423,-32765,
211,-32767,211,-32767,211,-32767,211,-32767,
0,-32767,0,-32767,0,-32767,0,-32767,
-212,-32767,-212,-32767,-212,-32767,-212,-32767,
-424,-32765,-424,-32765,-424,-32765,-424,-32765,
-636,-32761,-636,-32761,-636,-32761,-636,-32761,
-848,-32757,-848,-32757,-848,-32757,-848,-32757,
-1059,-32750,-1059,-32750,-1059,-32750,-1059,-32750,
-1271,-32743,-1271,-32743,-1271,-32743,-1271,-32743,
-1483,-32734,-1483,-32734,-1483,-32734,-1483,-32734,
-1694,-32724,-1694,-32724,-1694,-32724,-1694,-32724,
-1906,-32712,-1906,-32712,-1906,-32712,-1906,-32712,
-2117,-32699,-2117,-32699,-2117,-32699,-2117,-32699,
-2328,-32685,-2328,-32685,-2328,-32685,-2328,-32685,
-2540,-32669,-2540,-32669,-2540,-32669,-2540,-32669,
-2751,-32652,-2751,-32652,-2751,-32652,-2751,-32652,
-2962,-32633,-2962,-32633,-2962,-32633,-2962,-32633,
-3173,-32614,-3173,-32614,-3173,-32614,-3173,-32614,
-3383,-32592,-3383,-32592,-3383,-32592,-3383,-32592,
-3594,-32570,-3594,-32570,-3594,-32570,-3594,-32570,
-3805,-32546,-3805,-32546,-3805,-32546,-3805,-32546,
-4015,-32521,-4015,-32521,-4015,-32521,-4015,-32521,
-4225,-32494,-4225,-32494,-4225,-32494,-4225,-32494,
-4435,-32466,-4435,-32466,-4435,-32466,-4435,-32466,
-4645,-32437,-4645,-32437,-4645,-32437,-4645,-32437,
-4854,-32406,-4854,-32406,-4854,-32406,-4854,-32406,
-5064,-32374,-5064,-32374,-5064,-32374,-5064,-32374,
-5273,-32341,-5273,-32341,-5273,-32341,-5273,-32341,
-5482,-32306,-5482,-32306,-5482,-32306,-5482,-32306,
-5690,-32270,-5690,-32270,-5690,-32270,-5690,-32270,
-5899,-32232,-5899,-32232,-5899,-32232,-5899,-32232,
-6107,-32193,-6107,-32193,-6107,-32193,-6107,-32193,
-6315,-32153,-6315,-32153,-6315,-32153,-6315,-32153,
-6523,-32112,-6523,-32112,-6523,-32112,-6523,-32112,
-6730,-32069,-6730,-32069,-6730,-32069,-6730,-32069,
-6937,-32025,-6937,-32025,-6937,-32025,-6937,-32025,
-7144,-31979,-7144,-31979,-7144,-31979,-7144,-31979,
-7351,-31932,-7351,-31932,-7351,-31932,-7351,-31932,
-7557,-31884,-7557,-31884,-7557,-31884,-7557,-31884,
-7763,-31835,-7763,-31835,-7763,-31835,-7763,-31835,
-7969,-31784,-7969,-31784,-7969,-31784,-7969,-31784,
-8174,-31732,-8174,-31732,-8174,-31732,-8174,-31732,
-8379,-31678,-8379,-31678,-8379,-31678,-8379,-31678,
-8583,-31623,-8583,-31623,-8583,-31623,-8583,-31623,
-8788,-31567,-8788,-31567,-8788,-31567,-8788,-31567,
-8992,-31510,-8992,-31510,-8992,-31510,-8992,-31510,
-9195,-31451,-9195,-31451,-9195,-31451,-9195,-31451,
-9398,-31391,-9398,-31391,-9398,-31391,-9398,-31391,
-9601,-31330,-9601,-31330,-9601,-31330,-9601,-31330,
-9803,-31267,-9803,-31267,-9803,-31267,-9803,-31267,
-10005,-31203,-10005,-31203,-10005,-31203,-10005,-31203,
-10207,-31137,-10207,-31137,-10207,-31137,-10207,-31137,
-10408,-31071,-10408,-31071,-10408,-31071,-10408,-31071,
-10608,-31003,-10608,-31003,-10608,-31003,-10608,-31003,
-10808,-30934,-10808,-30934,-10808,-30934,-10808,-30934,
-11008,-30863,-11008,-30863,-11008,-30863,-11008,-30863,
-11207,-30791,-11207,-30791,-11207,-30791,-11207,-30791,
-11406,-30718,-11406,-30718,-11406,-30718,-11406,-30718,
-11605,-30644,-11605,-30644,-11605,-30644,-11605,-30644,
-11802,-30568,-11802,-30568,-11802,-30568,-11802,-30568,
-12000,-30491,-12000,-30491,-12000,-30491,-12000,-30491,
-12197,-30413,-12197,-30413,-12197,-30413,-12197,-30413,
-12393,-30334,-12393,-30334,-12393,-30334,-12393,-30334,
-12589,-30253,-12589,-30253,-12589,-30253,-12589,-30253,
-12784,-30171,-12784,-30171,-12784,-30171,-12784,-30171,
-12979,-30088,-12979,-30088,-12979,-30088,-12979,-30088,
-13173,-30003,-13173,-30003,-13173,-30003,-13173,-30003,
-13367,-29917,-13367,-29917,-13367,-29917,-13367,-29917,
-13560,-29830,-13560,-29830,-13560,-29830,-13560,-29830,
-13752,-29742,-13752,-29742,-13752,-29742,-13752,-29742,
-13944,-29653,-13944,-29653,-13944,-29653,-13944,-29653,
-14136,-29562,-14136,-29562,-14136,-29562,-14136,-29562,
-14327,-29470,-14327,-29470,-14327,-29470,-14327,-29470,
-14517,-29377,-14517,-29377,-14517,-29377,-14517,-29377,
-14706,-29282,-14706,-29282,-14706,-29282,-14706,-29282,
-14895,-29186,-14895,-29186,-14895,-29186,-14895,-29186,
-15084,-29090,-15084,-29090,-15084,-29090,-15084,-29090,
-15271,-28991,-15271,-28991,-15271,-28991,-15271,-28991,
-15458,-28892,-15458,-28892,-15458,-28892,-15458,-28892,
-15645,-28792,-15645,-28792,-15645,-28792,-15645,-28792,
-15831,-28690,-15831,-28690,-15831,-28690,-15831,-28690,
-16016,-28587,-16016,-28587,-16016,-28587,-16016,-28587,
-16200,-28483,-16200,-28483,-16200,-28483,-16200,-28483};
static int16_t twb972[323*2*4] = {32764,-424,32764,-424,32764,-424,32764,-424,
32756,-848,32756,-848,32756,-848,32756,-848,
32742,-1271,32742,-1271,32742,-1271,32742,-1271,
32723,-1694,32723,-1694,32723,-1694,32723,-1694,
32698,-2117,32698,-2117,32698,-2117,32698,-2117,
32668,-2540,32668,-2540,32668,-2540,32668,-2540,
32632,-2962,32632,-2962,32632,-2962,32632,-2962,
32591,-3383,32591,-3383,32591,-3383,32591,-3383,
32545,-3805,32545,-3805,32545,-3805,32545,-3805,
32493,-4225,32493,-4225,32493,-4225,32493,-4225,
32436,-4645,32436,-4645,32436,-4645,32436,-4645,
32373,-5064,32373,-5064,32373,-5064,32373,-5064,
32305,-5482,32305,-5482,32305,-5482,32305,-5482,
32231,-5899,32231,-5899,32231,-5899,32231,-5899,
32152,-6315,32152,-6315,32152,-6315,32152,-6315,
32068,-6730,32068,-6730,32068,-6730,32068,-6730,
31978,-7144,31978,-7144,31978,-7144,31978,-7144,
31883,-7557,31883,-7557,31883,-7557,31883,-7557,
31783,-7969,31783,-7969,31783,-7969,31783,-7969,
31677,-8379,31677,-8379,31677,-8379,31677,-8379,
31566,-8788,31566,-8788,31566,-8788,31566,-8788,
31450,-9195,31450,-9195,31450,-9195,31450,-9195,
31329,-9601,31329,-9601,31329,-9601,31329,-9601,
31202,-10005,31202,-10005,31202,-10005,31202,-10005,
31070,-10408,31070,-10408,31070,-10408,31070,-10408,
30933,-10808,30933,-10808,30933,-10808,30933,-10808,
30790,-11207,30790,-11207,30790,-11207,30790,-11207,
30643,-11605,30643,-11605,30643,-11605,30643,-11605,
30490,-12000,30490,-12000,30490,-12000,30490,-12000,
30333,-12393,30333,-12393,30333,-12393,30333,-12393,
30170,-12784,30170,-12784,30170,-12784,30170,-12784,
30002,-13173,30002,-13173,30002,-13173,30002,-13173,
29829,-13560,29829,-13560,29829,-13560,29829,-13560,
29652,-13944,29652,-13944,29652,-13944,29652,-13944,
29469,-14327,29469,-14327,29469,-14327,29469,-14327,
29281,-14706,29281,-14706,29281,-14706,29281,-14706,
29089,-15084,29089,-15084,29089,-15084,29089,-15084,
28891,-15458,28891,-15458,28891,-15458,28891,-15458,
28689,-15831,28689,-15831,28689,-15831,28689,-15831,
28482,-16200,28482,-16200,28482,-16200,28482,-16200,
28270,-16567,28270,-16567,28270,-16567,28270,-16567,
28054,-16931,28054,-16931,28054,-16931,28054,-16931,
27832,-17292,27832,-17292,27832,-17292,27832,-17292,
27606,-17651,27606,-17651,27606,-17651,27606,-17651,
27376,-18006,27376,-18006,27376,-18006,27376,-18006,
27141,-18359,27141,-18359,27141,-18359,27141,-18359,
26901,-18708,26901,-18708,26901,-18708,26901,-18708,
26657,-19054,26657,-19054,26657,-19054,26657,-19054,
26409,-19397,26409,-19397,26409,-19397,26409,-19397,
26156,-19737,26156,-19737,26156,-19737,26156,-19737,
25898,-20074,25898,-20074,25898,-20074,25898,-20074,
25637,-20407,25637,-20407,25637,-20407,25637,-20407,
25371,-20736,25371,-20736,25371,-20736,25371,-20736,
25100,-21063,25100,-21063,25100,-21063,25100,-21063,
24826,-21385,24826,-21385,24826,-21385,24826,-21385,
24548,-21705,24548,-21705,24548,-21705,24548,-21705,
24265,-22020,24265,-22020,24265,-22020,24265,-22020,
23978,-22332,23978,-22332,23978,-22332,23978,-22332,
23688,-22640,23688,-22640,23688,-22640,23688,-22640,
23393,-22945,23393,-22945,23393,-22945,23393,-22945,
23094,-23245,23094,-23245,23094,-23245,23094,-23245,
22792,-23542,22792,-23542,22792,-23542,22792,-23542,
22486,-23834,22486,-23834,22486,-23834,22486,-23834,
22176,-24123,22176,-24123,22176,-24123,22176,-24123,
21862,-24408,21862,-24408,21862,-24408,21862,-24408,
21545,-24688,21545,-24688,21545,-24688,21545,-24688,
21224,-24965,21224,-24965,21224,-24965,21224,-24965,
20899,-25237,20899,-25237,20899,-25237,20899,-25237,
20571,-25505,20571,-25505,20571,-25505,20571,-25505,
20240,-25769,20240,-25769,20240,-25769,20240,-25769,
19905,-26029,19905,-26029,19905,-26029,19905,-26029,
19567,-26284,19567,-26284,19567,-26284,19567,-26284,
19225,-26534,19225,-26534,19225,-26534,19225,-26534,
18881,-26781,18881,-26781,18881,-26781,18881,-26781,
18533,-27023,18533,-27023,18533,-27023,18533,-27023,
18182,-27260,18182,-27260,18182,-27260,18182,-27260,
17828,-27493,17828,-27493,17828,-27493,17828,-27493,
17471,-27721,17471,-27721,17471,-27721,17471,-27721,
17111,-27944,17111,-27944,17111,-27944,17111,-27944,
16748,-28163,16748,-28163,16748,-28163,16748,-28163,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
16015,-28587,16015,-28587,16015,-28587,16015,-28587,
15644,-28792,15644,-28792,15644,-28792,15644,-28792,
15270,-28991,15270,-28991,15270,-28991,15270,-28991,
14894,-29186,14894,-29186,14894,-29186,14894,-29186,
14516,-29377,14516,-29377,14516,-29377,14516,-29377,
14135,-29562,14135,-29562,14135,-29562,14135,-29562,
13751,-29742,13751,-29742,13751,-29742,13751,-29742,
13366,-29917,13366,-29917,13366,-29917,13366,-29917,
12978,-30088,12978,-30088,12978,-30088,12978,-30088,
12588,-30253,12588,-30253,12588,-30253,12588,-30253,
12196,-30413,12196,-30413,12196,-30413,12196,-30413,
11801,-30568,11801,-30568,11801,-30568,11801,-30568,
11405,-30718,11405,-30718,11405,-30718,11405,-30718,
11007,-30863,11007,-30863,11007,-30863,11007,-30863,
10607,-31003,10607,-31003,10607,-31003,10607,-31003,
10206,-31137,10206,-31137,10206,-31137,10206,-31137,
9802,-31267,9802,-31267,9802,-31267,9802,-31267,
9397,-31391,9397,-31391,9397,-31391,9397,-31391,
8991,-31510,8991,-31510,8991,-31510,8991,-31510,
8582,-31623,8582,-31623,8582,-31623,8582,-31623,
8173,-31732,8173,-31732,8173,-31732,8173,-31732,
7762,-31835,7762,-31835,7762,-31835,7762,-31835,
7350,-31932,7350,-31932,7350,-31932,7350,-31932,
6936,-32025,6936,-32025,6936,-32025,6936,-32025,
6522,-32112,6522,-32112,6522,-32112,6522,-32112,
6106,-32193,6106,-32193,6106,-32193,6106,-32193,
5689,-32270,5689,-32270,5689,-32270,5689,-32270,
5272,-32341,5272,-32341,5272,-32341,5272,-32341,
4853,-32406,4853,-32406,4853,-32406,4853,-32406,
4434,-32466,4434,-32466,4434,-32466,4434,-32466,
4014,-32521,4014,-32521,4014,-32521,4014,-32521,
3593,-32570,3593,-32570,3593,-32570,3593,-32570,
3172,-32614,3172,-32614,3172,-32614,3172,-32614,
2750,-32652,2750,-32652,2750,-32652,2750,-32652,
2327,-32685,2327,-32685,2327,-32685,2327,-32685,
1905,-32712,1905,-32712,1905,-32712,1905,-32712,
1482,-32734,1482,-32734,1482,-32734,1482,-32734,
1058,-32750,1058,-32750,1058,-32750,1058,-32750,
635,-32761,635,-32761,635,-32761,635,-32761,
211,-32767,211,-32767,211,-32767,211,-32767,
-212,-32767,-212,-32767,-212,-32767,-212,-32767,
-636,-32761,-636,-32761,-636,-32761,-636,-32761,
-1059,-32750,-1059,-32750,-1059,-32750,-1059,-32750,
-1483,-32734,-1483,-32734,-1483,-32734,-1483,-32734,
-1906,-32712,-1906,-32712,-1906,-32712,-1906,-32712,
-2328,-32685,-2328,-32685,-2328,-32685,-2328,-32685,
-2751,-32652,-2751,-32652,-2751,-32652,-2751,-32652,
-3173,-32614,-3173,-32614,-3173,-32614,-3173,-32614,
-3594,-32570,-3594,-32570,-3594,-32570,-3594,-32570,
-4015,-32521,-4015,-32521,-4015,-32521,-4015,-32521,
-4435,-32466,-4435,-32466,-4435,-32466,-4435,-32466,
-4854,-32406,-4854,-32406,-4854,-32406,-4854,-32406,
-5273,-32341,-5273,-32341,-5273,-32341,-5273,-32341,
-5690,-32270,-5690,-32270,-5690,-32270,-5690,-32270,
-6107,-32193,-6107,-32193,-6107,-32193,-6107,-32193,
-6523,-32112,-6523,-32112,-6523,-32112,-6523,-32112,
-6937,-32025,-6937,-32025,-6937,-32025,-6937,-32025,
-7351,-31932,-7351,-31932,-7351,-31932,-7351,-31932,
-7763,-31835,-7763,-31835,-7763,-31835,-7763,-31835,
-8174,-31732,-8174,-31732,-8174,-31732,-8174,-31732,
-8583,-31623,-8583,-31623,-8583,-31623,-8583,-31623,
-8992,-31510,-8992,-31510,-8992,-31510,-8992,-31510,
-9398,-31391,-9398,-31391,-9398,-31391,-9398,-31391,
-9803,-31267,-9803,-31267,-9803,-31267,-9803,-31267,
-10207,-31137,-10207,-31137,-10207,-31137,-10207,-31137,
-10608,-31003,-10608,-31003,-10608,-31003,-10608,-31003,
-11008,-30863,-11008,-30863,-11008,-30863,-11008,-30863,
-11406,-30718,-11406,-30718,-11406,-30718,-11406,-30718,
-11802,-30568,-11802,-30568,-11802,-30568,-11802,-30568,
-12197,-30413,-12197,-30413,-12197,-30413,-12197,-30413,
-12589,-30253,-12589,-30253,-12589,-30253,-12589,-30253,
-12979,-30088,-12979,-30088,-12979,-30088,-12979,-30088,
-13367,-29917,-13367,-29917,-13367,-29917,-13367,-29917,
-13752,-29742,-13752,-29742,-13752,-29742,-13752,-29742,
-14136,-29562,-14136,-29562,-14136,-29562,-14136,-29562,
-14517,-29377,-14517,-29377,-14517,-29377,-14517,-29377,
-14895,-29186,-14895,-29186,-14895,-29186,-14895,-29186,
-15271,-28991,-15271,-28991,-15271,-28991,-15271,-28991,
-15645,-28792,-15645,-28792,-15645,-28792,-15645,-28792,
-16016,-28587,-16016,-28587,-16016,-28587,-16016,-28587,
-16384,-28378,-16384,-28378,-16384,-28378,-16384,-28378,
-16749,-28163,-16749,-28163,-16749,-28163,-16749,-28163,
-17112,-27944,-17112,-27944,-17112,-27944,-17112,-27944,
-17472,-27721,-17472,-27721,-17472,-27721,-17472,-27721,
-17829,-27493,-17829,-27493,-17829,-27493,-17829,-27493,
-18183,-27260,-18183,-27260,-18183,-27260,-18183,-27260,
-18534,-27023,-18534,-27023,-18534,-27023,-18534,-27023,
-18882,-26781,-18882,-26781,-18882,-26781,-18882,-26781,
-19226,-26534,-19226,-26534,-19226,-26534,-19226,-26534,
-19568,-26284,-19568,-26284,-19568,-26284,-19568,-26284,
-19906,-26029,-19906,-26029,-19906,-26029,-19906,-26029,
-20241,-25769,-20241,-25769,-20241,-25769,-20241,-25769,
-20572,-25505,-20572,-25505,-20572,-25505,-20572,-25505,
-20900,-25237,-20900,-25237,-20900,-25237,-20900,-25237,
-21225,-24965,-21225,-24965,-21225,-24965,-21225,-24965,
-21546,-24688,-21546,-24688,-21546,-24688,-21546,-24688,
-21863,-24408,-21863,-24408,-21863,-24408,-21863,-24408,
-22177,-24123,-22177,-24123,-22177,-24123,-22177,-24123,
-22487,-23834,-22487,-23834,-22487,-23834,-22487,-23834,
-22793,-23542,-22793,-23542,-22793,-23542,-22793,-23542,
-23095,-23245,-23095,-23245,-23095,-23245,-23095,-23245,
-23394,-22945,-23394,-22945,-23394,-22945,-23394,-22945,
-23689,-22640,-23689,-22640,-23689,-22640,-23689,-22640,
-23979,-22332,-23979,-22332,-23979,-22332,-23979,-22332,
-24266,-22020,-24266,-22020,-24266,-22020,-24266,-22020,
-24549,-21705,-24549,-21705,-24549,-21705,-24549,-21705,
-24827,-21385,-24827,-21385,-24827,-21385,-24827,-21385,
-25101,-21063,-25101,-21063,-25101,-21063,-25101,-21063,
-25372,-20736,-25372,-20736,-25372,-20736,-25372,-20736,
-25638,-20407,-25638,-20407,-25638,-20407,-25638,-20407,
-25899,-20074,-25899,-20074,-25899,-20074,-25899,-20074,
-26157,-19737,-26157,-19737,-26157,-19737,-26157,-19737,
-26410,-19397,-26410,-19397,-26410,-19397,-26410,-19397,
-26658,-19054,-26658,-19054,-26658,-19054,-26658,-19054,
-26902,-18708,-26902,-18708,-26902,-18708,-26902,-18708,
-27142,-18359,-27142,-18359,-27142,-18359,-27142,-18359,
-27377,-18006,-27377,-18006,-27377,-18006,-27377,-18006,
-27607,-17651,-27607,-17651,-27607,-17651,-27607,-17651,
-27833,-17292,-27833,-17292,-27833,-17292,-27833,-17292,
-28055,-16931,-28055,-16931,-28055,-16931,-28055,-16931,
-28271,-16567,-28271,-16567,-28271,-16567,-28271,-16567,
-28483,-16200,-28483,-16200,-28483,-16200,-28483,-16200,
-28690,-15831,-28690,-15831,-28690,-15831,-28690,-15831,
-28892,-15458,-28892,-15458,-28892,-15458,-28892,-15458,
-29090,-15084,-29090,-15084,-29090,-15084,-29090,-15084,
-29282,-14706,-29282,-14706,-29282,-14706,-29282,-14706,
-29470,-14327,-29470,-14327,-29470,-14327,-29470,-14327,
-29653,-13944,-29653,-13944,-29653,-13944,-29653,-13944,
-29830,-13560,-29830,-13560,-29830,-13560,-29830,-13560,
-30003,-13173,-30003,-13173,-30003,-13173,-30003,-13173,
-30171,-12784,-30171,-12784,-30171,-12784,-30171,-12784,
-30334,-12393,-30334,-12393,-30334,-12393,-30334,-12393,
-30491,-12000,-30491,-12000,-30491,-12000,-30491,-12000,
-30644,-11605,-30644,-11605,-30644,-11605,-30644,-11605,
-30791,-11207,-30791,-11207,-30791,-11207,-30791,-11207,
-30934,-10808,-30934,-10808,-30934,-10808,-30934,-10808,
-31071,-10408,-31071,-10408,-31071,-10408,-31071,-10408,
-31203,-10005,-31203,-10005,-31203,-10005,-31203,-10005,
-31330,-9601,-31330,-9601,-31330,-9601,-31330,-9601,
-31451,-9195,-31451,-9195,-31451,-9195,-31451,-9195,
-31567,-8788,-31567,-8788,-31567,-8788,-31567,-8788,
-31678,-8379,-31678,-8379,-31678,-8379,-31678,-8379,
-31784,-7969,-31784,-7969,-31784,-7969,-31784,-7969,
-31884,-7557,-31884,-7557,-31884,-7557,-31884,-7557,
-31979,-7144,-31979,-7144,-31979,-7144,-31979,-7144,
-32069,-6730,-32069,-6730,-32069,-6730,-32069,-6730,
-32153,-6315,-32153,-6315,-32153,-6315,-32153,-6315,
-32232,-5899,-32232,-5899,-32232,-5899,-32232,-5899,
-32306,-5482,-32306,-5482,-32306,-5482,-32306,-5482,
-32374,-5064,-32374,-5064,-32374,-5064,-32374,-5064,
-32437,-4645,-32437,-4645,-32437,-4645,-32437,-4645,
-32494,-4225,-32494,-4225,-32494,-4225,-32494,-4225,
-32546,-3805,-32546,-3805,-32546,-3805,-32546,-3805,
-32592,-3383,-32592,-3383,-32592,-3383,-32592,-3383,
-32633,-2962,-32633,-2962,-32633,-2962,-32633,-2962,
-32669,-2540,-32669,-2540,-32669,-2540,-32669,-2540,
-32699,-2117,-32699,-2117,-32699,-2117,-32699,-2117,
-32724,-1694,-32724,-1694,-32724,-1694,-32724,-1694,
-32743,-1271,-32743,-1271,-32743,-1271,-32743,-1271,
-32757,-848,-32757,-848,-32757,-848,-32757,-848,
-32765,-424,-32765,-424,-32765,-424,-32765,-424,
-32767,-1,-32767,-1,-32767,-1,-32767,-1,
-32765,423,-32765,423,-32765,423,-32765,423,
-32757,847,-32757,847,-32757,847,-32757,847,
-32743,1270,-32743,1270,-32743,1270,-32743,1270,
-32724,1693,-32724,1693,-32724,1693,-32724,1693,
-32699,2116,-32699,2116,-32699,2116,-32699,2116,
-32669,2539,-32669,2539,-32669,2539,-32669,2539,
-32633,2961,-32633,2961,-32633,2961,-32633,2961,
-32592,3382,-32592,3382,-32592,3382,-32592,3382,
-32546,3804,-32546,3804,-32546,3804,-32546,3804,
-32494,4224,-32494,4224,-32494,4224,-32494,4224,
-32437,4644,-32437,4644,-32437,4644,-32437,4644,
-32374,5063,-32374,5063,-32374,5063,-32374,5063,
-32306,5481,-32306,5481,-32306,5481,-32306,5481,
-32232,5898,-32232,5898,-32232,5898,-32232,5898,
-32153,6314,-32153,6314,-32153,6314,-32153,6314,
-32069,6729,-32069,6729,-32069,6729,-32069,6729,
-31979,7143,-31979,7143,-31979,7143,-31979,7143,
-31884,7556,-31884,7556,-31884,7556,-31884,7556,
-31784,7968,-31784,7968,-31784,7968,-31784,7968,
-31678,8378,-31678,8378,-31678,8378,-31678,8378,
-31567,8787,-31567,8787,-31567,8787,-31567,8787,
-31451,9194,-31451,9194,-31451,9194,-31451,9194,
-31330,9600,-31330,9600,-31330,9600,-31330,9600,
-31203,10004,-31203,10004,-31203,10004,-31203,10004,
-31071,10407,-31071,10407,-31071,10407,-31071,10407,
-30934,10807,-30934,10807,-30934,10807,-30934,10807,
-30791,11206,-30791,11206,-30791,11206,-30791,11206,
-30644,11604,-30644,11604,-30644,11604,-30644,11604,
-30491,11999,-30491,11999,-30491,11999,-30491,11999,
-30334,12392,-30334,12392,-30334,12392,-30334,12392,
-30171,12783,-30171,12783,-30171,12783,-30171,12783,
-30003,13172,-30003,13172,-30003,13172,-30003,13172,
-29830,13559,-29830,13559,-29830,13559,-29830,13559,
-29653,13943,-29653,13943,-29653,13943,-29653,13943,
-29470,14326,-29470,14326,-29470,14326,-29470,14326,
-29282,14705,-29282,14705,-29282,14705,-29282,14705,
-29090,15083,-29090,15083,-29090,15083,-29090,15083,
-28892,15457,-28892,15457,-28892,15457,-28892,15457,
-28690,15830,-28690,15830,-28690,15830,-28690,15830,
-28483,16199,-28483,16199,-28483,16199,-28483,16199,
-28271,16566,-28271,16566,-28271,16566,-28271,16566,
-28055,16930,-28055,16930,-28055,16930,-28055,16930,
-27833,17291,-27833,17291,-27833,17291,-27833,17291,
-27607,17650,-27607,17650,-27607,17650,-27607,17650,
-27377,18005,-27377,18005,-27377,18005,-27377,18005,
-27142,18358,-27142,18358,-27142,18358,-27142,18358,
-26902,18707,-26902,18707,-26902,18707,-26902,18707,
-26658,19053,-26658,19053,-26658,19053,-26658,19053,
-26410,19396,-26410,19396,-26410,19396,-26410,19396,
-26157,19736,-26157,19736,-26157,19736,-26157,19736,
-25899,20073,-25899,20073,-25899,20073,-25899,20073,
-25638,20406,-25638,20406,-25638,20406,-25638,20406,
-25372,20735,-25372,20735,-25372,20735,-25372,20735,
-25101,21062,-25101,21062,-25101,21062,-25101,21062,
-24827,21384,-24827,21384,-24827,21384,-24827,21384,
-24549,21704,-24549,21704,-24549,21704,-24549,21704,
-24266,22019,-24266,22019,-24266,22019,-24266,22019,
-23979,22331,-23979,22331,-23979,22331,-23979,22331,
-23689,22639,-23689,22639,-23689,22639,-23689,22639,
-23394,22944,-23394,22944,-23394,22944,-23394,22944,
-23095,23244,-23095,23244,-23095,23244,-23095,23244,
-22793,23541,-22793,23541,-22793,23541,-22793,23541,
-22487,23833,-22487,23833,-22487,23833,-22487,23833,
-22177,24122,-22177,24122,-22177,24122,-22177,24122,
-21863,24407,-21863,24407,-21863,24407,-21863,24407,
-21546,24687,-21546,24687,-21546,24687,-21546,24687,
-21225,24964,-21225,24964,-21225,24964,-21225,24964,
-20900,25236,-20900,25236,-20900,25236,-20900,25236,
-20572,25504,-20572,25504,-20572,25504,-20572,25504,
-20241,25768,-20241,25768,-20241,25768,-20241,25768,
-19906,26028,-19906,26028,-19906,26028,-19906,26028,
-19568,26283,-19568,26283,-19568,26283,-19568,26283,
-19226,26533,-19226,26533,-19226,26533,-19226,26533,
-18882,26780,-18882,26780,-18882,26780,-18882,26780,
-18534,27022,-18534,27022,-18534,27022,-18534,27022,
-18183,27259,-18183,27259,-18183,27259,-18183,27259,
-17829,27492,-17829,27492,-17829,27492,-17829,27492,
-17472,27720,-17472,27720,-17472,27720,-17472,27720,
-17112,27943,-17112,27943,-17112,27943,-17112,27943,
-16749,28162,-16749,28162,-16749,28162,-16749,28162};

void dft972(int16_t *x,int16_t *y,unsigned char scale_flag){ // 324 x 3
  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa972[0];
  __m128i *twb128=(__m128i *)&twb972[0];
  __m128i x2128[972];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[972];//=&ytmp128array3[0];



  for (i=0,j=0;i<324;i++,j+=3) {
    x2128[i]    = x128[j];
    x2128[i+324] = x128[j+1];
    x2128[i+648] = x128[j+2];
  }

  dft324((int16_t *)x2128,(int16_t *)ytmp128,1);
  dft324((int16_t *)(x2128+324),(int16_t *)(ytmp128+324),1);
  dft324((int16_t *)(x2128+648),(int16_t *)(ytmp128+648),1);

  bfly3_tw1(ytmp128,ytmp128+324,ytmp128+648,y128,y128+324,y128+648);
  for (i=1,j=0;i<324;i++,j++) {
    bfly3(ytmp128+i,
	  ytmp128+324+i,
	  ytmp128+648+i,
	  y128+i,
	  y128+324+i,
	  y128+648+i,
	  twa128+j,
	  twb128+j);
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(dft_norm_table[14]);
    
    for (i=0;i<972;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

};

/* Twiddles generated with
twa = floor(32767*exp(-sqrt(-1)*2*pi*(1:359)/1080));
twb = floor(32767*exp(-sqrt(-1)*2*pi*2*(1:359)/1080));
twa2 = zeros(1,2*359);
twb2 = zeros(1,2*359);
twa2(1:2:end) = real(twa);
twa2(2:2:end) = imag(twa);
twb2(1:2:end) = real(twb);
twb2(2:2:end) = imag(twb);
fd=fopen("twiddle_tmp.txt","w");
fprintf(fd,"static int16_t twa1080[359*2*4] = {");
for i=1:2:(2*358)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
fprintf(fd,"static int16_t twb1080[359*2*4] = {");
for i=1:2:(2*358)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
fclose(fd);
*/
static int16_t twa1080[359*2*4] = {32766,-191,32766,-191,32766,-191,32766,-191,
32764,-382,32764,-382,32764,-382,32764,-382,
32762,-572,32762,-572,32762,-572,32762,-572,
32758,-763,32758,-763,32758,-763,32758,-763,
32753,-954,32753,-954,32753,-954,32753,-954,
32747,-1144,32747,-1144,32747,-1144,32747,-1144,
32739,-1335,32739,-1335,32739,-1335,32739,-1335,
32731,-1525,32731,-1525,32731,-1525,32731,-1525,
32722,-1715,32722,-1715,32722,-1715,32722,-1715,
32711,-1906,32711,-1906,32711,-1906,32711,-1906,
32699,-2096,32699,-2096,32699,-2096,32699,-2096,
32687,-2286,32687,-2286,32687,-2286,32687,-2286,
32673,-2476,32673,-2476,32673,-2476,32673,-2476,
32658,-2666,32658,-2666,32658,-2666,32658,-2666,
32642,-2856,32642,-2856,32642,-2856,32642,-2856,
32625,-3046,32625,-3046,32625,-3046,32625,-3046,
32606,-3236,32606,-3236,32606,-3236,32606,-3236,
32587,-3426,32587,-3426,32587,-3426,32587,-3426,
32567,-3615,32567,-3615,32567,-3615,32567,-3615,
32545,-3805,32545,-3805,32545,-3805,32545,-3805,
32522,-3994,32522,-3994,32522,-3994,32522,-3994,
32498,-4183,32498,-4183,32498,-4183,32498,-4183,
32474,-4372,32474,-4372,32474,-4372,32474,-4372,
32448,-4561,32448,-4561,32448,-4561,32448,-4561,
32421,-4749,32421,-4749,32421,-4749,32421,-4749,
32392,-4938,32392,-4938,32392,-4938,32392,-4938,
32363,-5126,32363,-5126,32363,-5126,32363,-5126,
32333,-5315,32333,-5315,32333,-5315,32333,-5315,
32301,-5503,32301,-5503,32301,-5503,32301,-5503,
32269,-5690,32269,-5690,32269,-5690,32269,-5690,
32235,-5878,32235,-5878,32235,-5878,32235,-5878,
32200,-6066,32200,-6066,32200,-6066,32200,-6066,
32164,-6253,32164,-6253,32164,-6253,32164,-6253,
32128,-6440,32128,-6440,32128,-6440,32128,-6440,
32090,-6627,32090,-6627,32090,-6627,32090,-6627,
32050,-6813,32050,-6813,32050,-6813,32050,-6813,
32010,-6999,32010,-6999,32010,-6999,32010,-6999,
31969,-7186,31969,-7186,31969,-7186,31969,-7186,
31927,-7371,31927,-7371,31927,-7371,31927,-7371,
31883,-7557,31883,-7557,31883,-7557,31883,-7557,
31839,-7742,31839,-7742,31839,-7742,31839,-7742,
31793,-7928,31793,-7928,31793,-7928,31793,-7928,
31747,-8112,31747,-8112,31747,-8112,31747,-8112,
31699,-8297,31699,-8297,31699,-8297,31699,-8297,
31650,-8481,31650,-8481,31650,-8481,31650,-8481,
31600,-8665,31600,-8665,31600,-8665,31600,-8665,
31549,-8849,31549,-8849,31549,-8849,31549,-8849,
31497,-9032,31497,-9032,31497,-9032,31497,-9032,
31444,-9215,31444,-9215,31444,-9215,31444,-9215,
31390,-9398,31390,-9398,31390,-9398,31390,-9398,
31335,-9581,31335,-9581,31335,-9581,31335,-9581,
31278,-9763,31278,-9763,31278,-9763,31278,-9763,
31221,-9945,31221,-9945,31221,-9945,31221,-9945,
31163,-10126,31163,-10126,31163,-10126,31163,-10126,
31103,-10307,31103,-10307,31103,-10307,31103,-10307,
31043,-10488,31043,-10488,31043,-10488,31043,-10488,
30981,-10668,30981,-10668,30981,-10668,30981,-10668,
30919,-10848,30919,-10848,30919,-10848,30919,-10848,
30855,-11028,30855,-11028,30855,-11028,30855,-11028,
30790,-11207,30790,-11207,30790,-11207,30790,-11207,
30725,-11386,30725,-11386,30725,-11386,30725,-11386,
30658,-11565,30658,-11565,30658,-11565,30658,-11565,
30590,-11743,30590,-11743,30590,-11743,30590,-11743,
30521,-11921,30521,-11921,30521,-11921,30521,-11921,
30451,-12098,30451,-12098,30451,-12098,30451,-12098,
30381,-12275,30381,-12275,30381,-12275,30381,-12275,
30309,-12452,30309,-12452,30309,-12452,30309,-12452,
30236,-12628,30236,-12628,30236,-12628,30236,-12628,
30162,-12804,30162,-12804,30162,-12804,30162,-12804,
30087,-12979,30087,-12979,30087,-12979,30087,-12979,
30011,-13154,30011,-13154,30011,-13154,30011,-13154,
29934,-13328,29934,-13328,29934,-13328,29934,-13328,
29856,-13502,29856,-13502,29856,-13502,29856,-13502,
29777,-13675,29777,-13675,29777,-13675,29777,-13675,
29696,-13848,29696,-13848,29696,-13848,29696,-13848,
29615,-14021,29615,-14021,29615,-14021,29615,-14021,
29533,-14193,29533,-14193,29533,-14193,29533,-14193,
29450,-14365,29450,-14365,29450,-14365,29450,-14365,
29366,-14536,29366,-14536,29366,-14536,29366,-14536,
29281,-14706,29281,-14706,29281,-14706,29281,-14706,
29195,-14876,29195,-14876,29195,-14876,29195,-14876,
29108,-15046,29108,-15046,29108,-15046,29108,-15046,
29020,-15215,29020,-15215,29020,-15215,29020,-15215,
28931,-15384,28931,-15384,28931,-15384,28931,-15384,
28841,-15552,28841,-15552,28841,-15552,28841,-15552,
28750,-15719,28750,-15719,28750,-15719,28750,-15719,
28658,-15886,28658,-15886,28658,-15886,28658,-15886,
28565,-16053,28565,-16053,28565,-16053,28565,-16053,
28471,-16219,28471,-16219,28471,-16219,28471,-16219,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
28281,-16549,28281,-16549,28281,-16549,28281,-16549,
28184,-16713,28184,-16713,28184,-16713,28184,-16713,
28086,-16877,28086,-16877,28086,-16877,28086,-16877,
27988,-17040,27988,-17040,27988,-17040,27988,-17040,
27888,-17202,27888,-17202,27888,-17202,27888,-17202,
27787,-17364,27787,-17364,27787,-17364,27787,-17364,
27686,-17526,27686,-17526,27686,-17526,27686,-17526,
27584,-17687,27584,-17687,27584,-17687,27584,-17687,
27480,-17847,27480,-17847,27480,-17847,27480,-17847,
27376,-18006,27376,-18006,27376,-18006,27376,-18006,
27271,-18165,27271,-18165,27271,-18165,27271,-18165,
27165,-18324,27165,-18324,27165,-18324,27165,-18324,
27058,-18481,27058,-18481,27058,-18481,27058,-18481,
26950,-18638,26950,-18638,26950,-18638,26950,-18638,
26841,-18795,26841,-18795,26841,-18795,26841,-18795,
26731,-18951,26731,-18951,26731,-18951,26731,-18951,
26620,-19106,26620,-19106,26620,-19106,26620,-19106,
26509,-19260,26509,-19260,26509,-19260,26509,-19260,
26396,-19414,26396,-19414,26396,-19414,26396,-19414,
26283,-19568,26283,-19568,26283,-19568,26283,-19568,
26168,-19720,26168,-19720,26168,-19720,26168,-19720,
26053,-19872,26053,-19872,26053,-19872,26053,-19872,
25937,-20023,25937,-20023,25937,-20023,25937,-20023,
25820,-20174,25820,-20174,25820,-20174,25820,-20174,
25702,-20324,25702,-20324,25702,-20324,25702,-20324,
25584,-20473,25584,-20473,25584,-20473,25584,-20473,
25464,-20621,25464,-20621,25464,-20621,25464,-20621,
25344,-20769,25344,-20769,25344,-20769,25344,-20769,
25223,-20916,25223,-20916,25223,-20916,25223,-20916,
25100,-21063,25100,-21063,25100,-21063,25100,-21063,
24978,-21208,24978,-21208,24978,-21208,24978,-21208,
24854,-21353,24854,-21353,24854,-21353,24854,-21353,
24729,-21498,24729,-21498,24729,-21498,24729,-21498,
24604,-21641,24604,-21641,24604,-21641,24604,-21641,
24477,-21784,24477,-21784,24477,-21784,24477,-21784,
24350,-21926,24350,-21926,24350,-21926,24350,-21926,
24222,-22067,24222,-22067,24222,-22067,24222,-22067,
24093,-22208,24093,-22208,24093,-22208,24093,-22208,
23964,-22348,23964,-22348,23964,-22348,23964,-22348,
23833,-22487,23833,-22487,23833,-22487,23833,-22487,
23702,-22625,23702,-22625,23702,-22625,23702,-22625,
23570,-22762,23570,-22762,23570,-22762,23570,-22762,
23437,-22899,23437,-22899,23437,-22899,23437,-22899,
23304,-23035,23304,-23035,23304,-23035,23304,-23035,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
23034,-23305,23034,-23305,23034,-23305,23034,-23305,
22898,-23438,22898,-23438,22898,-23438,22898,-23438,
22761,-23571,22761,-23571,22761,-23571,22761,-23571,
22624,-23703,22624,-23703,22624,-23703,22624,-23703,
22486,-23834,22486,-23834,22486,-23834,22486,-23834,
22347,-23965,22347,-23965,22347,-23965,22347,-23965,
22207,-24094,22207,-24094,22207,-24094,22207,-24094,
22066,-24223,22066,-24223,22066,-24223,22066,-24223,
21925,-24351,21925,-24351,21925,-24351,21925,-24351,
21783,-24478,21783,-24478,21783,-24478,21783,-24478,
21640,-24605,21640,-24605,21640,-24605,21640,-24605,
21497,-24730,21497,-24730,21497,-24730,21497,-24730,
21352,-24855,21352,-24855,21352,-24855,21352,-24855,
21207,-24979,21207,-24979,21207,-24979,21207,-24979,
21062,-25101,21062,-25101,21062,-25101,21062,-25101,
20915,-25224,20915,-25224,20915,-25224,20915,-25224,
20768,-25345,20768,-25345,20768,-25345,20768,-25345,
20620,-25465,20620,-25465,20620,-25465,20620,-25465,
20472,-25585,20472,-25585,20472,-25585,20472,-25585,
20323,-25703,20323,-25703,20323,-25703,20323,-25703,
20173,-25821,20173,-25821,20173,-25821,20173,-25821,
20022,-25938,20022,-25938,20022,-25938,20022,-25938,
19871,-26054,19871,-26054,19871,-26054,19871,-26054,
19719,-26169,19719,-26169,19719,-26169,19719,-26169,
19567,-26284,19567,-26284,19567,-26284,19567,-26284,
19413,-26397,19413,-26397,19413,-26397,19413,-26397,
19259,-26510,19259,-26510,19259,-26510,19259,-26510,
19105,-26621,19105,-26621,19105,-26621,19105,-26621,
18950,-26732,18950,-26732,18950,-26732,18950,-26732,
18794,-26842,18794,-26842,18794,-26842,18794,-26842,
18637,-26951,18637,-26951,18637,-26951,18637,-26951,
18480,-27059,18480,-27059,18480,-27059,18480,-27059,
18323,-27166,18323,-27166,18323,-27166,18323,-27166,
18164,-27272,18164,-27272,18164,-27272,18164,-27272,
18005,-27377,18005,-27377,18005,-27377,18005,-27377,
17846,-27481,17846,-27481,17846,-27481,17846,-27481,
17686,-27585,17686,-27585,17686,-27585,17686,-27585,
17525,-27687,17525,-27687,17525,-27687,17525,-27687,
17363,-27788,17363,-27788,17363,-27788,17363,-27788,
17201,-27889,17201,-27889,17201,-27889,17201,-27889,
17039,-27989,17039,-27989,17039,-27989,17039,-27989,
16876,-28087,16876,-28087,16876,-28087,16876,-28087,
16712,-28185,16712,-28185,16712,-28185,16712,-28185,
16548,-28282,16548,-28282,16548,-28282,16548,-28282,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
16218,-28472,16218,-28472,16218,-28472,16218,-28472,
16052,-28566,16052,-28566,16052,-28566,16052,-28566,
15885,-28659,15885,-28659,15885,-28659,15885,-28659,
15718,-28751,15718,-28751,15718,-28751,15718,-28751,
15551,-28842,15551,-28842,15551,-28842,15551,-28842,
15383,-28932,15383,-28932,15383,-28932,15383,-28932,
15214,-29021,15214,-29021,15214,-29021,15214,-29021,
15045,-29109,15045,-29109,15045,-29109,15045,-29109,
14875,-29196,14875,-29196,14875,-29196,14875,-29196,
14705,-29282,14705,-29282,14705,-29282,14705,-29282,
14535,-29367,14535,-29367,14535,-29367,14535,-29367,
14364,-29451,14364,-29451,14364,-29451,14364,-29451,
14192,-29534,14192,-29534,14192,-29534,14192,-29534,
14020,-29616,14020,-29616,14020,-29616,14020,-29616,
13847,-29697,13847,-29697,13847,-29697,13847,-29697,
13674,-29778,13674,-29778,13674,-29778,13674,-29778,
13501,-29857,13501,-29857,13501,-29857,13501,-29857,
13327,-29935,13327,-29935,13327,-29935,13327,-29935,
13153,-30012,13153,-30012,13153,-30012,13153,-30012,
12978,-30088,12978,-30088,12978,-30088,12978,-30088,
12803,-30163,12803,-30163,12803,-30163,12803,-30163,
12627,-30237,12627,-30237,12627,-30237,12627,-30237,
12451,-30310,12451,-30310,12451,-30310,12451,-30310,
12274,-30382,12274,-30382,12274,-30382,12274,-30382,
12097,-30452,12097,-30452,12097,-30452,12097,-30452,
11920,-30522,11920,-30522,11920,-30522,11920,-30522,
11742,-30591,11742,-30591,11742,-30591,11742,-30591,
11564,-30659,11564,-30659,11564,-30659,11564,-30659,
11385,-30726,11385,-30726,11385,-30726,11385,-30726,
11206,-30791,11206,-30791,11206,-30791,11206,-30791,
11027,-30856,11027,-30856,11027,-30856,11027,-30856,
10847,-30920,10847,-30920,10847,-30920,10847,-30920,
10667,-30982,10667,-30982,10667,-30982,10667,-30982,
10487,-31044,10487,-31044,10487,-31044,10487,-31044,
10306,-31104,10306,-31104,10306,-31104,10306,-31104,
10125,-31164,10125,-31164,10125,-31164,10125,-31164,
9944,-31222,9944,-31222,9944,-31222,9944,-31222,
9762,-31279,9762,-31279,9762,-31279,9762,-31279,
9580,-31336,9580,-31336,9580,-31336,9580,-31336,
9397,-31391,9397,-31391,9397,-31391,9397,-31391,
9214,-31445,9214,-31445,9214,-31445,9214,-31445,
9031,-31498,9031,-31498,9031,-31498,9031,-31498,
8848,-31550,8848,-31550,8848,-31550,8848,-31550,
8664,-31601,8664,-31601,8664,-31601,8664,-31601,
8480,-31651,8480,-31651,8480,-31651,8480,-31651,
8296,-31700,8296,-31700,8296,-31700,8296,-31700,
8111,-31748,8111,-31748,8111,-31748,8111,-31748,
7927,-31794,7927,-31794,7927,-31794,7927,-31794,
7741,-31840,7741,-31840,7741,-31840,7741,-31840,
7556,-31884,7556,-31884,7556,-31884,7556,-31884,
7370,-31928,7370,-31928,7370,-31928,7370,-31928,
7185,-31970,7185,-31970,7185,-31970,7185,-31970,
6998,-32011,6998,-32011,6998,-32011,6998,-32011,
6812,-32051,6812,-32051,6812,-32051,6812,-32051,
6626,-32091,6626,-32091,6626,-32091,6626,-32091,
6439,-32129,6439,-32129,6439,-32129,6439,-32129,
6252,-32165,6252,-32165,6252,-32165,6252,-32165,
6065,-32201,6065,-32201,6065,-32201,6065,-32201,
5877,-32236,5877,-32236,5877,-32236,5877,-32236,
5689,-32270,5689,-32270,5689,-32270,5689,-32270,
5502,-32302,5502,-32302,5502,-32302,5502,-32302,
5314,-32334,5314,-32334,5314,-32334,5314,-32334,
5125,-32364,5125,-32364,5125,-32364,5125,-32364,
4937,-32393,4937,-32393,4937,-32393,4937,-32393,
4748,-32422,4748,-32422,4748,-32422,4748,-32422,
4560,-32449,4560,-32449,4560,-32449,4560,-32449,
4371,-32475,4371,-32475,4371,-32475,4371,-32475,
4182,-32499,4182,-32499,4182,-32499,4182,-32499,
3993,-32523,3993,-32523,3993,-32523,3993,-32523,
3804,-32546,3804,-32546,3804,-32546,3804,-32546,
3614,-32568,3614,-32568,3614,-32568,3614,-32568,
3425,-32588,3425,-32588,3425,-32588,3425,-32588,
3235,-32607,3235,-32607,3235,-32607,3235,-32607,
3045,-32626,3045,-32626,3045,-32626,3045,-32626,
2855,-32643,2855,-32643,2855,-32643,2855,-32643,
2665,-32659,2665,-32659,2665,-32659,2665,-32659,
2475,-32674,2475,-32674,2475,-32674,2475,-32674,
2285,-32688,2285,-32688,2285,-32688,2285,-32688,
2095,-32700,2095,-32700,2095,-32700,2095,-32700,
1905,-32712,1905,-32712,1905,-32712,1905,-32712,
1714,-32723,1714,-32723,1714,-32723,1714,-32723,
1524,-32732,1524,-32732,1524,-32732,1524,-32732,
1334,-32740,1334,-32740,1334,-32740,1334,-32740,
1143,-32748,1143,-32748,1143,-32748,1143,-32748,
953,-32754,953,-32754,953,-32754,953,-32754,
762,-32759,762,-32759,762,-32759,762,-32759,
571,-32763,571,-32763,571,-32763,571,-32763,
381,-32765,381,-32765,381,-32765,381,-32765,
190,-32767,190,-32767,190,-32767,190,-32767,
0,-32767,0,-32767,0,-32767,0,-32767,
-191,-32767,-191,-32767,-191,-32767,-191,-32767,
-382,-32765,-382,-32765,-382,-32765,-382,-32765,
-572,-32763,-572,-32763,-572,-32763,-572,-32763,
-763,-32759,-763,-32759,-763,-32759,-763,-32759,
-954,-32754,-954,-32754,-954,-32754,-954,-32754,
-1144,-32748,-1144,-32748,-1144,-32748,-1144,-32748,
-1335,-32740,-1335,-32740,-1335,-32740,-1335,-32740,
-1525,-32732,-1525,-32732,-1525,-32732,-1525,-32732,
-1715,-32723,-1715,-32723,-1715,-32723,-1715,-32723,
-1906,-32712,-1906,-32712,-1906,-32712,-1906,-32712,
-2096,-32700,-2096,-32700,-2096,-32700,-2096,-32700,
-2286,-32688,-2286,-32688,-2286,-32688,-2286,-32688,
-2476,-32674,-2476,-32674,-2476,-32674,-2476,-32674,
-2666,-32659,-2666,-32659,-2666,-32659,-2666,-32659,
-2856,-32643,-2856,-32643,-2856,-32643,-2856,-32643,
-3046,-32626,-3046,-32626,-3046,-32626,-3046,-32626,
-3236,-32607,-3236,-32607,-3236,-32607,-3236,-32607,
-3426,-32588,-3426,-32588,-3426,-32588,-3426,-32588,
-3615,-32568,-3615,-32568,-3615,-32568,-3615,-32568,
-3805,-32546,-3805,-32546,-3805,-32546,-3805,-32546,
-3994,-32523,-3994,-32523,-3994,-32523,-3994,-32523,
-4183,-32499,-4183,-32499,-4183,-32499,-4183,-32499,
-4372,-32475,-4372,-32475,-4372,-32475,-4372,-32475,
-4561,-32449,-4561,-32449,-4561,-32449,-4561,-32449,
-4749,-32422,-4749,-32422,-4749,-32422,-4749,-32422,
-4938,-32393,-4938,-32393,-4938,-32393,-4938,-32393,
-5126,-32364,-5126,-32364,-5126,-32364,-5126,-32364,
-5315,-32334,-5315,-32334,-5315,-32334,-5315,-32334,
-5503,-32302,-5503,-32302,-5503,-32302,-5503,-32302,
-5690,-32270,-5690,-32270,-5690,-32270,-5690,-32270,
-5878,-32236,-5878,-32236,-5878,-32236,-5878,-32236,
-6066,-32201,-6066,-32201,-6066,-32201,-6066,-32201,
-6253,-32165,-6253,-32165,-6253,-32165,-6253,-32165,
-6440,-32129,-6440,-32129,-6440,-32129,-6440,-32129,
-6627,-32091,-6627,-32091,-6627,-32091,-6627,-32091,
-6813,-32051,-6813,-32051,-6813,-32051,-6813,-32051,
-6999,-32011,-6999,-32011,-6999,-32011,-6999,-32011,
-7186,-31970,-7186,-31970,-7186,-31970,-7186,-31970,
-7371,-31928,-7371,-31928,-7371,-31928,-7371,-31928,
-7557,-31884,-7557,-31884,-7557,-31884,-7557,-31884,
-7742,-31840,-7742,-31840,-7742,-31840,-7742,-31840,
-7928,-31794,-7928,-31794,-7928,-31794,-7928,-31794,
-8112,-31748,-8112,-31748,-8112,-31748,-8112,-31748,
-8297,-31700,-8297,-31700,-8297,-31700,-8297,-31700,
-8481,-31651,-8481,-31651,-8481,-31651,-8481,-31651,
-8665,-31601,-8665,-31601,-8665,-31601,-8665,-31601,
-8849,-31550,-8849,-31550,-8849,-31550,-8849,-31550,
-9032,-31498,-9032,-31498,-9032,-31498,-9032,-31498,
-9215,-31445,-9215,-31445,-9215,-31445,-9215,-31445,
-9398,-31391,-9398,-31391,-9398,-31391,-9398,-31391,
-9581,-31336,-9581,-31336,-9581,-31336,-9581,-31336,
-9763,-31279,-9763,-31279,-9763,-31279,-9763,-31279,
-9945,-31222,-9945,-31222,-9945,-31222,-9945,-31222,
-10126,-31164,-10126,-31164,-10126,-31164,-10126,-31164,
-10307,-31104,-10307,-31104,-10307,-31104,-10307,-31104,
-10488,-31044,-10488,-31044,-10488,-31044,-10488,-31044,
-10668,-30982,-10668,-30982,-10668,-30982,-10668,-30982,
-10848,-30920,-10848,-30920,-10848,-30920,-10848,-30920,
-11028,-30856,-11028,-30856,-11028,-30856,-11028,-30856,
-11207,-30791,-11207,-30791,-11207,-30791,-11207,-30791,
-11386,-30726,-11386,-30726,-11386,-30726,-11386,-30726,
-11565,-30659,-11565,-30659,-11565,-30659,-11565,-30659,
-11743,-30591,-11743,-30591,-11743,-30591,-11743,-30591,
-11921,-30522,-11921,-30522,-11921,-30522,-11921,-30522,
-12098,-30452,-12098,-30452,-12098,-30452,-12098,-30452,
-12275,-30382,-12275,-30382,-12275,-30382,-12275,-30382,
-12452,-30310,-12452,-30310,-12452,-30310,-12452,-30310,
-12628,-30237,-12628,-30237,-12628,-30237,-12628,-30237,
-12804,-30163,-12804,-30163,-12804,-30163,-12804,-30163,
-12979,-30088,-12979,-30088,-12979,-30088,-12979,-30088,
-13154,-30012,-13154,-30012,-13154,-30012,-13154,-30012,
-13328,-29935,-13328,-29935,-13328,-29935,-13328,-29935,
-13502,-29857,-13502,-29857,-13502,-29857,-13502,-29857,
-13675,-29778,-13675,-29778,-13675,-29778,-13675,-29778,
-13848,-29697,-13848,-29697,-13848,-29697,-13848,-29697,
-14021,-29616,-14021,-29616,-14021,-29616,-14021,-29616,
-14193,-29534,-14193,-29534,-14193,-29534,-14193,-29534,
-14365,-29451,-14365,-29451,-14365,-29451,-14365,-29451,
-14536,-29367,-14536,-29367,-14536,-29367,-14536,-29367,
-14706,-29282,-14706,-29282,-14706,-29282,-14706,-29282,
-14876,-29196,-14876,-29196,-14876,-29196,-14876,-29196,
-15046,-29109,-15046,-29109,-15046,-29109,-15046,-29109,
-15215,-29021,-15215,-29021,-15215,-29021,-15215,-29021,
-15384,-28932,-15384,-28932,-15384,-28932,-15384,-28932,
-15552,-28842,-15552,-28842,-15552,-28842,-15552,-28842,
-15719,-28751,-15719,-28751,-15719,-28751,-15719,-28751,
-15886,-28659,-15886,-28659,-15886,-28659,-15886,-28659,
-16053,-28566,-16053,-28566,-16053,-28566,-16053,-28566,
-16219,-28472,-16219,-28472,-16219,-28472,-16219,-28472};
static int16_t twb1080[359*2*4] = {32764,-382,32764,-382,32764,-382,32764,-382,
32758,-763,32758,-763,32758,-763,32758,-763,
32747,-1144,32747,-1144,32747,-1144,32747,-1144,
32731,-1525,32731,-1525,32731,-1525,32731,-1525,
32711,-1906,32711,-1906,32711,-1906,32711,-1906,
32687,-2286,32687,-2286,32687,-2286,32687,-2286,
32658,-2666,32658,-2666,32658,-2666,32658,-2666,
32625,-3046,32625,-3046,32625,-3046,32625,-3046,
32587,-3426,32587,-3426,32587,-3426,32587,-3426,
32545,-3805,32545,-3805,32545,-3805,32545,-3805,
32498,-4183,32498,-4183,32498,-4183,32498,-4183,
32448,-4561,32448,-4561,32448,-4561,32448,-4561,
32392,-4938,32392,-4938,32392,-4938,32392,-4938,
32333,-5315,32333,-5315,32333,-5315,32333,-5315,
32269,-5690,32269,-5690,32269,-5690,32269,-5690,
32200,-6066,32200,-6066,32200,-6066,32200,-6066,
32128,-6440,32128,-6440,32128,-6440,32128,-6440,
32050,-6813,32050,-6813,32050,-6813,32050,-6813,
31969,-7186,31969,-7186,31969,-7186,31969,-7186,
31883,-7557,31883,-7557,31883,-7557,31883,-7557,
31793,-7928,31793,-7928,31793,-7928,31793,-7928,
31699,-8297,31699,-8297,31699,-8297,31699,-8297,
31600,-8665,31600,-8665,31600,-8665,31600,-8665,
31497,-9032,31497,-9032,31497,-9032,31497,-9032,
31390,-9398,31390,-9398,31390,-9398,31390,-9398,
31278,-9763,31278,-9763,31278,-9763,31278,-9763,
31163,-10126,31163,-10126,31163,-10126,31163,-10126,
31043,-10488,31043,-10488,31043,-10488,31043,-10488,
30919,-10848,30919,-10848,30919,-10848,30919,-10848,
30790,-11207,30790,-11207,30790,-11207,30790,-11207,
30658,-11565,30658,-11565,30658,-11565,30658,-11565,
30521,-11921,30521,-11921,30521,-11921,30521,-11921,
30381,-12275,30381,-12275,30381,-12275,30381,-12275,
30236,-12628,30236,-12628,30236,-12628,30236,-12628,
30087,-12979,30087,-12979,30087,-12979,30087,-12979,
29934,-13328,29934,-13328,29934,-13328,29934,-13328,
29777,-13675,29777,-13675,29777,-13675,29777,-13675,
29615,-14021,29615,-14021,29615,-14021,29615,-14021,
29450,-14365,29450,-14365,29450,-14365,29450,-14365,
29281,-14706,29281,-14706,29281,-14706,29281,-14706,
29108,-15046,29108,-15046,29108,-15046,29108,-15046,
28931,-15384,28931,-15384,28931,-15384,28931,-15384,
28750,-15719,28750,-15719,28750,-15719,28750,-15719,
28565,-16053,28565,-16053,28565,-16053,28565,-16053,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
28184,-16713,28184,-16713,28184,-16713,28184,-16713,
27988,-17040,27988,-17040,27988,-17040,27988,-17040,
27787,-17364,27787,-17364,27787,-17364,27787,-17364,
27584,-17687,27584,-17687,27584,-17687,27584,-17687,
27376,-18006,27376,-18006,27376,-18006,27376,-18006,
27165,-18324,27165,-18324,27165,-18324,27165,-18324,
26950,-18638,26950,-18638,26950,-18638,26950,-18638,
26731,-18951,26731,-18951,26731,-18951,26731,-18951,
26509,-19260,26509,-19260,26509,-19260,26509,-19260,
26283,-19568,26283,-19568,26283,-19568,26283,-19568,
26053,-19872,26053,-19872,26053,-19872,26053,-19872,
25820,-20174,25820,-20174,25820,-20174,25820,-20174,
25584,-20473,25584,-20473,25584,-20473,25584,-20473,
25344,-20769,25344,-20769,25344,-20769,25344,-20769,
25100,-21063,25100,-21063,25100,-21063,25100,-21063,
24854,-21353,24854,-21353,24854,-21353,24854,-21353,
24604,-21641,24604,-21641,24604,-21641,24604,-21641,
24350,-21926,24350,-21926,24350,-21926,24350,-21926,
24093,-22208,24093,-22208,24093,-22208,24093,-22208,
23833,-22487,23833,-22487,23833,-22487,23833,-22487,
23570,-22762,23570,-22762,23570,-22762,23570,-22762,
23304,-23035,23304,-23035,23304,-23035,23304,-23035,
23034,-23305,23034,-23305,23034,-23305,23034,-23305,
22761,-23571,22761,-23571,22761,-23571,22761,-23571,
22486,-23834,22486,-23834,22486,-23834,22486,-23834,
22207,-24094,22207,-24094,22207,-24094,22207,-24094,
21925,-24351,21925,-24351,21925,-24351,21925,-24351,
21640,-24605,21640,-24605,21640,-24605,21640,-24605,
21352,-24855,21352,-24855,21352,-24855,21352,-24855,
21062,-25101,21062,-25101,21062,-25101,21062,-25101,
20768,-25345,20768,-25345,20768,-25345,20768,-25345,
20472,-25585,20472,-25585,20472,-25585,20472,-25585,
20173,-25821,20173,-25821,20173,-25821,20173,-25821,
19871,-26054,19871,-26054,19871,-26054,19871,-26054,
19567,-26284,19567,-26284,19567,-26284,19567,-26284,
19259,-26510,19259,-26510,19259,-26510,19259,-26510,
18950,-26732,18950,-26732,18950,-26732,18950,-26732,
18637,-26951,18637,-26951,18637,-26951,18637,-26951,
18323,-27166,18323,-27166,18323,-27166,18323,-27166,
18005,-27377,18005,-27377,18005,-27377,18005,-27377,
17686,-27585,17686,-27585,17686,-27585,17686,-27585,
17363,-27788,17363,-27788,17363,-27788,17363,-27788,
17039,-27989,17039,-27989,17039,-27989,17039,-27989,
16712,-28185,16712,-28185,16712,-28185,16712,-28185,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
16052,-28566,16052,-28566,16052,-28566,16052,-28566,
15718,-28751,15718,-28751,15718,-28751,15718,-28751,
15383,-28932,15383,-28932,15383,-28932,15383,-28932,
15045,-29109,15045,-29109,15045,-29109,15045,-29109,
14705,-29282,14705,-29282,14705,-29282,14705,-29282,
14364,-29451,14364,-29451,14364,-29451,14364,-29451,
14020,-29616,14020,-29616,14020,-29616,14020,-29616,
13674,-29778,13674,-29778,13674,-29778,13674,-29778,
13327,-29935,13327,-29935,13327,-29935,13327,-29935,
12978,-30088,12978,-30088,12978,-30088,12978,-30088,
12627,-30237,12627,-30237,12627,-30237,12627,-30237,
12274,-30382,12274,-30382,12274,-30382,12274,-30382,
11920,-30522,11920,-30522,11920,-30522,11920,-30522,
11564,-30659,11564,-30659,11564,-30659,11564,-30659,
11206,-30791,11206,-30791,11206,-30791,11206,-30791,
10847,-30920,10847,-30920,10847,-30920,10847,-30920,
10487,-31044,10487,-31044,10487,-31044,10487,-31044,
10125,-31164,10125,-31164,10125,-31164,10125,-31164,
9762,-31279,9762,-31279,9762,-31279,9762,-31279,
9397,-31391,9397,-31391,9397,-31391,9397,-31391,
9031,-31498,9031,-31498,9031,-31498,9031,-31498,
8664,-31601,8664,-31601,8664,-31601,8664,-31601,
8296,-31700,8296,-31700,8296,-31700,8296,-31700,
7927,-31794,7927,-31794,7927,-31794,7927,-31794,
7556,-31884,7556,-31884,7556,-31884,7556,-31884,
7185,-31970,7185,-31970,7185,-31970,7185,-31970,
6812,-32051,6812,-32051,6812,-32051,6812,-32051,
6439,-32129,6439,-32129,6439,-32129,6439,-32129,
6065,-32201,6065,-32201,6065,-32201,6065,-32201,
5689,-32270,5689,-32270,5689,-32270,5689,-32270,
5314,-32334,5314,-32334,5314,-32334,5314,-32334,
4937,-32393,4937,-32393,4937,-32393,4937,-32393,
4560,-32449,4560,-32449,4560,-32449,4560,-32449,
4182,-32499,4182,-32499,4182,-32499,4182,-32499,
3804,-32546,3804,-32546,3804,-32546,3804,-32546,
3425,-32588,3425,-32588,3425,-32588,3425,-32588,
3045,-32626,3045,-32626,3045,-32626,3045,-32626,
2665,-32659,2665,-32659,2665,-32659,2665,-32659,
2285,-32688,2285,-32688,2285,-32688,2285,-32688,
1905,-32712,1905,-32712,1905,-32712,1905,-32712,
1524,-32732,1524,-32732,1524,-32732,1524,-32732,
1143,-32748,1143,-32748,1143,-32748,1143,-32748,
762,-32759,762,-32759,762,-32759,762,-32759,
381,-32765,381,-32765,381,-32765,381,-32765,
0,-32767,0,-32767,0,-32767,0,-32767,
-382,-32765,-382,-32765,-382,-32765,-382,-32765,
-763,-32759,-763,-32759,-763,-32759,-763,-32759,
-1144,-32748,-1144,-32748,-1144,-32748,-1144,-32748,
-1525,-32732,-1525,-32732,-1525,-32732,-1525,-32732,
-1906,-32712,-1906,-32712,-1906,-32712,-1906,-32712,
-2286,-32688,-2286,-32688,-2286,-32688,-2286,-32688,
-2666,-32659,-2666,-32659,-2666,-32659,-2666,-32659,
-3046,-32626,-3046,-32626,-3046,-32626,-3046,-32626,
-3426,-32588,-3426,-32588,-3426,-32588,-3426,-32588,
-3805,-32546,-3805,-32546,-3805,-32546,-3805,-32546,
-4183,-32499,-4183,-32499,-4183,-32499,-4183,-32499,
-4561,-32449,-4561,-32449,-4561,-32449,-4561,-32449,
-4938,-32393,-4938,-32393,-4938,-32393,-4938,-32393,
-5315,-32334,-5315,-32334,-5315,-32334,-5315,-32334,
-5690,-32270,-5690,-32270,-5690,-32270,-5690,-32270,
-6066,-32201,-6066,-32201,-6066,-32201,-6066,-32201,
-6440,-32129,-6440,-32129,-6440,-32129,-6440,-32129,
-6813,-32051,-6813,-32051,-6813,-32051,-6813,-32051,
-7186,-31970,-7186,-31970,-7186,-31970,-7186,-31970,
-7557,-31884,-7557,-31884,-7557,-31884,-7557,-31884,
-7928,-31794,-7928,-31794,-7928,-31794,-7928,-31794,
-8297,-31700,-8297,-31700,-8297,-31700,-8297,-31700,
-8665,-31601,-8665,-31601,-8665,-31601,-8665,-31601,
-9032,-31498,-9032,-31498,-9032,-31498,-9032,-31498,
-9398,-31391,-9398,-31391,-9398,-31391,-9398,-31391,
-9763,-31279,-9763,-31279,-9763,-31279,-9763,-31279,
-10126,-31164,-10126,-31164,-10126,-31164,-10126,-31164,
-10488,-31044,-10488,-31044,-10488,-31044,-10488,-31044,
-10848,-30920,-10848,-30920,-10848,-30920,-10848,-30920,
-11207,-30791,-11207,-30791,-11207,-30791,-11207,-30791,
-11565,-30659,-11565,-30659,-11565,-30659,-11565,-30659,
-11921,-30522,-11921,-30522,-11921,-30522,-11921,-30522,
-12275,-30382,-12275,-30382,-12275,-30382,-12275,-30382,
-12628,-30237,-12628,-30237,-12628,-30237,-12628,-30237,
-12979,-30088,-12979,-30088,-12979,-30088,-12979,-30088,
-13328,-29935,-13328,-29935,-13328,-29935,-13328,-29935,
-13675,-29778,-13675,-29778,-13675,-29778,-13675,-29778,
-14021,-29616,-14021,-29616,-14021,-29616,-14021,-29616,
-14365,-29451,-14365,-29451,-14365,-29451,-14365,-29451,
-14706,-29282,-14706,-29282,-14706,-29282,-14706,-29282,
-15046,-29109,-15046,-29109,-15046,-29109,-15046,-29109,
-15384,-28932,-15384,-28932,-15384,-28932,-15384,-28932,
-15719,-28751,-15719,-28751,-15719,-28751,-15719,-28751,
-16053,-28566,-16053,-28566,-16053,-28566,-16053,-28566,
-16384,-28378,-16384,-28378,-16384,-28378,-16384,-28378,
-16713,-28185,-16713,-28185,-16713,-28185,-16713,-28185,
-17040,-27989,-17040,-27989,-17040,-27989,-17040,-27989,
-17364,-27788,-17364,-27788,-17364,-27788,-17364,-27788,
-17687,-27585,-17687,-27585,-17687,-27585,-17687,-27585,
-18006,-27377,-18006,-27377,-18006,-27377,-18006,-27377,
-18324,-27166,-18324,-27166,-18324,-27166,-18324,-27166,
-18638,-26951,-18638,-26951,-18638,-26951,-18638,-26951,
-18951,-26732,-18951,-26732,-18951,-26732,-18951,-26732,
-19260,-26510,-19260,-26510,-19260,-26510,-19260,-26510,
-19568,-26284,-19568,-26284,-19568,-26284,-19568,-26284,
-19872,-26054,-19872,-26054,-19872,-26054,-19872,-26054,
-20174,-25821,-20174,-25821,-20174,-25821,-20174,-25821,
-20473,-25585,-20473,-25585,-20473,-25585,-20473,-25585,
-20769,-25345,-20769,-25345,-20769,-25345,-20769,-25345,
-21063,-25101,-21063,-25101,-21063,-25101,-21063,-25101,
-21353,-24855,-21353,-24855,-21353,-24855,-21353,-24855,
-21641,-24605,-21641,-24605,-21641,-24605,-21641,-24605,
-21926,-24351,-21926,-24351,-21926,-24351,-21926,-24351,
-22208,-24094,-22208,-24094,-22208,-24094,-22208,-24094,
-22487,-23834,-22487,-23834,-22487,-23834,-22487,-23834,
-22762,-23571,-22762,-23571,-22762,-23571,-22762,-23571,
-23035,-23305,-23035,-23305,-23035,-23305,-23035,-23305,
-23305,-23035,-23305,-23035,-23305,-23035,-23305,-23035,
-23571,-22762,-23571,-22762,-23571,-22762,-23571,-22762,
-23834,-22487,-23834,-22487,-23834,-22487,-23834,-22487,
-24094,-22208,-24094,-22208,-24094,-22208,-24094,-22208,
-24351,-21926,-24351,-21926,-24351,-21926,-24351,-21926,
-24605,-21641,-24605,-21641,-24605,-21641,-24605,-21641,
-24855,-21353,-24855,-21353,-24855,-21353,-24855,-21353,
-25101,-21063,-25101,-21063,-25101,-21063,-25101,-21063,
-25345,-20769,-25345,-20769,-25345,-20769,-25345,-20769,
-25585,-20473,-25585,-20473,-25585,-20473,-25585,-20473,
-25821,-20174,-25821,-20174,-25821,-20174,-25821,-20174,
-26054,-19872,-26054,-19872,-26054,-19872,-26054,-19872,
-26284,-19568,-26284,-19568,-26284,-19568,-26284,-19568,
-26510,-19260,-26510,-19260,-26510,-19260,-26510,-19260,
-26732,-18951,-26732,-18951,-26732,-18951,-26732,-18951,
-26951,-18638,-26951,-18638,-26951,-18638,-26951,-18638,
-27166,-18324,-27166,-18324,-27166,-18324,-27166,-18324,
-27377,-18006,-27377,-18006,-27377,-18006,-27377,-18006,
-27585,-17687,-27585,-17687,-27585,-17687,-27585,-17687,
-27788,-17364,-27788,-17364,-27788,-17364,-27788,-17364,
-27989,-17040,-27989,-17040,-27989,-17040,-27989,-17040,
-28185,-16713,-28185,-16713,-28185,-16713,-28185,-16713,
-28378,-16384,-28378,-16384,-28378,-16384,-28378,-16384,
-28566,-16053,-28566,-16053,-28566,-16053,-28566,-16053,
-28751,-15719,-28751,-15719,-28751,-15719,-28751,-15719,
-28932,-15384,-28932,-15384,-28932,-15384,-28932,-15384,
-29109,-15046,-29109,-15046,-29109,-15046,-29109,-15046,
-29282,-14706,-29282,-14706,-29282,-14706,-29282,-14706,
-29451,-14365,-29451,-14365,-29451,-14365,-29451,-14365,
-29616,-14021,-29616,-14021,-29616,-14021,-29616,-14021,
-29778,-13675,-29778,-13675,-29778,-13675,-29778,-13675,
-29935,-13328,-29935,-13328,-29935,-13328,-29935,-13328,
-30088,-12979,-30088,-12979,-30088,-12979,-30088,-12979,
-30237,-12628,-30237,-12628,-30237,-12628,-30237,-12628,
-30382,-12275,-30382,-12275,-30382,-12275,-30382,-12275,
-30522,-11921,-30522,-11921,-30522,-11921,-30522,-11921,
-30659,-11565,-30659,-11565,-30659,-11565,-30659,-11565,
-30791,-11207,-30791,-11207,-30791,-11207,-30791,-11207,
-30920,-10848,-30920,-10848,-30920,-10848,-30920,-10848,
-31044,-10488,-31044,-10488,-31044,-10488,-31044,-10488,
-31164,-10126,-31164,-10126,-31164,-10126,-31164,-10126,
-31279,-9763,-31279,-9763,-31279,-9763,-31279,-9763,
-31391,-9398,-31391,-9398,-31391,-9398,-31391,-9398,
-31498,-9032,-31498,-9032,-31498,-9032,-31498,-9032,
-31601,-8665,-31601,-8665,-31601,-8665,-31601,-8665,
-31700,-8297,-31700,-8297,-31700,-8297,-31700,-8297,
-31794,-7928,-31794,-7928,-31794,-7928,-31794,-7928,
-31884,-7557,-31884,-7557,-31884,-7557,-31884,-7557,
-31970,-7186,-31970,-7186,-31970,-7186,-31970,-7186,
-32051,-6813,-32051,-6813,-32051,-6813,-32051,-6813,
-32129,-6440,-32129,-6440,-32129,-6440,-32129,-6440,
-32201,-6066,-32201,-6066,-32201,-6066,-32201,-6066,
-32270,-5690,-32270,-5690,-32270,-5690,-32270,-5690,
-32334,-5315,-32334,-5315,-32334,-5315,-32334,-5315,
-32393,-4938,-32393,-4938,-32393,-4938,-32393,-4938,
-32449,-4561,-32449,-4561,-32449,-4561,-32449,-4561,
-32499,-4183,-32499,-4183,-32499,-4183,-32499,-4183,
-32546,-3805,-32546,-3805,-32546,-3805,-32546,-3805,
-32588,-3426,-32588,-3426,-32588,-3426,-32588,-3426,
-32626,-3046,-32626,-3046,-32626,-3046,-32626,-3046,
-32659,-2666,-32659,-2666,-32659,-2666,-32659,-2666,
-32688,-2286,-32688,-2286,-32688,-2286,-32688,-2286,
-32712,-1906,-32712,-1906,-32712,-1906,-32712,-1906,
-32732,-1525,-32732,-1525,-32732,-1525,-32732,-1525,
-32748,-1144,-32748,-1144,-32748,-1144,-32748,-1144,
-32759,-763,-32759,-763,-32759,-763,-32759,-763,
-32765,-382,-32765,-382,-32765,-382,-32765,-382,
-32767,-1,-32767,-1,-32767,-1,-32767,-1,
-32765,381,-32765,381,-32765,381,-32765,381,
-32759,762,-32759,762,-32759,762,-32759,762,
-32748,1143,-32748,1143,-32748,1143,-32748,1143,
-32732,1524,-32732,1524,-32732,1524,-32732,1524,
-32712,1905,-32712,1905,-32712,1905,-32712,1905,
-32688,2285,-32688,2285,-32688,2285,-32688,2285,
-32659,2665,-32659,2665,-32659,2665,-32659,2665,
-32626,3045,-32626,3045,-32626,3045,-32626,3045,
-32588,3425,-32588,3425,-32588,3425,-32588,3425,
-32546,3804,-32546,3804,-32546,3804,-32546,3804,
-32499,4182,-32499,4182,-32499,4182,-32499,4182,
-32449,4560,-32449,4560,-32449,4560,-32449,4560,
-32393,4937,-32393,4937,-32393,4937,-32393,4937,
-32334,5314,-32334,5314,-32334,5314,-32334,5314,
-32270,5689,-32270,5689,-32270,5689,-32270,5689,
-32201,6065,-32201,6065,-32201,6065,-32201,6065,
-32129,6439,-32129,6439,-32129,6439,-32129,6439,
-32051,6812,-32051,6812,-32051,6812,-32051,6812,
-31970,7185,-31970,7185,-31970,7185,-31970,7185,
-31884,7556,-31884,7556,-31884,7556,-31884,7556,
-31794,7927,-31794,7927,-31794,7927,-31794,7927,
-31700,8296,-31700,8296,-31700,8296,-31700,8296,
-31601,8664,-31601,8664,-31601,8664,-31601,8664,
-31498,9031,-31498,9031,-31498,9031,-31498,9031,
-31391,9397,-31391,9397,-31391,9397,-31391,9397,
-31279,9762,-31279,9762,-31279,9762,-31279,9762,
-31164,10125,-31164,10125,-31164,10125,-31164,10125,
-31044,10487,-31044,10487,-31044,10487,-31044,10487,
-30920,10847,-30920,10847,-30920,10847,-30920,10847,
-30791,11206,-30791,11206,-30791,11206,-30791,11206,
-30659,11564,-30659,11564,-30659,11564,-30659,11564,
-30522,11920,-30522,11920,-30522,11920,-30522,11920,
-30382,12274,-30382,12274,-30382,12274,-30382,12274,
-30237,12627,-30237,12627,-30237,12627,-30237,12627,
-30088,12978,-30088,12978,-30088,12978,-30088,12978,
-29935,13327,-29935,13327,-29935,13327,-29935,13327,
-29778,13674,-29778,13674,-29778,13674,-29778,13674,
-29616,14020,-29616,14020,-29616,14020,-29616,14020,
-29451,14364,-29451,14364,-29451,14364,-29451,14364,
-29282,14705,-29282,14705,-29282,14705,-29282,14705,
-29109,15045,-29109,15045,-29109,15045,-29109,15045,
-28932,15383,-28932,15383,-28932,15383,-28932,15383,
-28751,15718,-28751,15718,-28751,15718,-28751,15718,
-28566,16052,-28566,16052,-28566,16052,-28566,16052,
-28378,16383,-28378,16383,-28378,16383,-28378,16383,
-28185,16712,-28185,16712,-28185,16712,-28185,16712,
-27989,17039,-27989,17039,-27989,17039,-27989,17039,
-27788,17363,-27788,17363,-27788,17363,-27788,17363,
-27585,17686,-27585,17686,-27585,17686,-27585,17686,
-27377,18005,-27377,18005,-27377,18005,-27377,18005,
-27166,18323,-27166,18323,-27166,18323,-27166,18323,
-26951,18637,-26951,18637,-26951,18637,-26951,18637,
-26732,18950,-26732,18950,-26732,18950,-26732,18950,
-26510,19259,-26510,19259,-26510,19259,-26510,19259,
-26284,19567,-26284,19567,-26284,19567,-26284,19567,
-26054,19871,-26054,19871,-26054,19871,-26054,19871,
-25821,20173,-25821,20173,-25821,20173,-25821,20173,
-25585,20472,-25585,20472,-25585,20472,-25585,20472,
-25345,20768,-25345,20768,-25345,20768,-25345,20768,
-25101,21062,-25101,21062,-25101,21062,-25101,21062,
-24855,21352,-24855,21352,-24855,21352,-24855,21352,
-24605,21640,-24605,21640,-24605,21640,-24605,21640,
-24351,21925,-24351,21925,-24351,21925,-24351,21925,
-24094,22207,-24094,22207,-24094,22207,-24094,22207,
-23834,22486,-23834,22486,-23834,22486,-23834,22486,
-23571,22761,-23571,22761,-23571,22761,-23571,22761,
-23305,23034,-23305,23034,-23305,23034,-23305,23034,
-23035,23304,-23035,23304,-23035,23304,-23035,23304,
-22762,23570,-22762,23570,-22762,23570,-22762,23570,
-22487,23833,-22487,23833,-22487,23833,-22487,23833,
-22208,24093,-22208,24093,-22208,24093,-22208,24093,
-21926,24350,-21926,24350,-21926,24350,-21926,24350,
-21641,24604,-21641,24604,-21641,24604,-21641,24604,
-21353,24854,-21353,24854,-21353,24854,-21353,24854,
-21063,25100,-21063,25100,-21063,25100,-21063,25100,
-20769,25344,-20769,25344,-20769,25344,-20769,25344,
-20473,25584,-20473,25584,-20473,25584,-20473,25584,
-20174,25820,-20174,25820,-20174,25820,-20174,25820,
-19872,26053,-19872,26053,-19872,26053,-19872,26053,
-19568,26283,-19568,26283,-19568,26283,-19568,26283,
-19260,26509,-19260,26509,-19260,26509,-19260,26509,
-18951,26731,-18951,26731,-18951,26731,-18951,26731,
-18638,26950,-18638,26950,-18638,26950,-18638,26950,
-18324,27165,-18324,27165,-18324,27165,-18324,27165,
-18006,27376,-18006,27376,-18006,27376,-18006,27376,
-17687,27584,-17687,27584,-17687,27584,-17687,27584,
-17364,27787,-17364,27787,-17364,27787,-17364,27787,
-17040,27988,-17040,27988,-17040,27988,-17040,27988,
-16713,28184,-16713,28184,-16713,28184,-16713,28184};

void dft1080(int16_t *x,int16_t *y,unsigned char scale_flag){ // 360 x 3
  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa1080[0];
  __m128i *twb128=(__m128i *)&twb1080[0];
  __m128i x2128[1080];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[1080];//=&ytmp128array3[0];



  for (i=0,j=0;i<360;i++,j+=3) {
    x2128[i]    = x128[j];
    x2128[i+360] = x128[j+1];
    x2128[i+720] = x128[j+2];
  }

  dft360((int16_t *)x2128,(int16_t *)ytmp128,1);
  dft360((int16_t *)(x2128+360),(int16_t *)(ytmp128+360),1);
  dft360((int16_t *)(x2128+720),(int16_t *)(ytmp128+720),1);

  bfly3_tw1(ytmp128,ytmp128+360,ytmp128+720,y128,y128+360,y128+720);
  for (i=1,j=0;i<360;i++,j++) {
    bfly3(ytmp128+i,
	  ytmp128+360+i,
	  ytmp128+720+i,
	  y128+i,
	  y128+360+i,
	  y128+720+i,
	  twa128+j,
	  twb128+j);
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(dft_norm_table[14]);
    
    for (i=0;i<1080;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

};

/* Twiddles generated with
twa = floor(32767*exp(-sqrt(-1)*2*pi*(1:287)/1152));
twb = floor(32767*exp(-sqrt(-1)*2*pi*2*(1:287)/1152));
twc = floor(32767*exp(-sqrt(-1)*2*pi*3*(1:287)/1152));
twa2 = zeros(1,2*287);
twb2 = zeros(1,2*287);
twc2 = zeros(1,2*287);
twa2(1:2:end) = real(twa);
twa2(2:2:end) = imag(twa);
twb2(1:2:end) = real(twb);
twb2(2:2:end) = imag(twb);
twc2(1:2:end) = real(twc);
twc2(2:2:end) = imag(twc);
fd=fopen("twiddle_tmp.txt","w");
fprintf(fd,"static int16_t twa1152[287*2*4] = {");
for i=1:2:(2*286)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1),twa2(i),twa2(i+1));
fprintf(fd,"\nstatic int16_t twb1152[287*2*4] = {");
for i=1:2:(2*286)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1),twb2(i),twb2(i+1));
fprintf(fd,"\nstatic int16_t twc1152[287*2*4] = {");
for i=1:2:(2*286)
  fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d,\n",twc2(i),twc2(i+1),twc2(i),twc2(i+1),twc2(i),twc2(i+1),twc2(i),twc2(i+1));
end
i=i+2;
fprintf(fd,"%d,%d,%d,%d,%d,%d,%d,%d};\n",twc2(i),twc2(i+1),twc2(i),twc2(i+1),twc2(i),twc2(i+1),twc2(i),twc2(i+1));
fclose(fd);
*/
static int16_t twa1152[287*2*4] = {32766,-179,32766,-179,32766,-179,32766,-179,
32765,-358,32765,-358,32765,-358,32765,-358,
32762,-537,32762,-537,32762,-537,32762,-537,
32759,-715,32759,-715,32759,-715,32759,-715,
32754,-894,32754,-894,32754,-894,32754,-894,
32749,-1073,32749,-1073,32749,-1073,32749,-1073,
32743,-1251,32743,-1251,32743,-1251,32743,-1251,
32735,-1430,32735,-1430,32735,-1430,32735,-1430,
32727,-1608,32727,-1608,32727,-1608,32727,-1608,
32718,-1787,32718,-1787,32718,-1787,32718,-1787,
32708,-1965,32708,-1965,32708,-1965,32708,-1965,
32696,-2144,32696,-2144,32696,-2144,32696,-2144,
32684,-2322,32684,-2322,32684,-2322,32684,-2322,
32671,-2500,32671,-2500,32671,-2500,32671,-2500,
32657,-2678,32657,-2678,32657,-2678,32657,-2678,
32642,-2856,32642,-2856,32642,-2856,32642,-2856,
32626,-3034,32626,-3034,32626,-3034,32626,-3034,
32609,-3212,32609,-3212,32609,-3212,32609,-3212,
32591,-3390,32591,-3390,32591,-3390,32591,-3390,
32572,-3568,32572,-3568,32572,-3568,32572,-3568,
32552,-3745,32552,-3745,32552,-3745,32552,-3745,
32531,-3923,32531,-3923,32531,-3923,32531,-3923,
32509,-4100,32509,-4100,32509,-4100,32509,-4100,
32486,-4277,32486,-4277,32486,-4277,32486,-4277,
32462,-4455,32462,-4455,32462,-4455,32462,-4455,
32438,-4632,32438,-4632,32438,-4632,32438,-4632,
32412,-4808,32412,-4808,32412,-4808,32412,-4808,
32385,-4985,32385,-4985,32385,-4985,32385,-4985,
32357,-5162,32357,-5162,32357,-5162,32357,-5162,
32329,-5338,32329,-5338,32329,-5338,32329,-5338,
32299,-5514,32299,-5514,32299,-5514,32299,-5514,
32269,-5690,32269,-5690,32269,-5690,32269,-5690,
32237,-5866,32237,-5866,32237,-5866,32237,-5866,
32205,-6042,32205,-6042,32205,-6042,32205,-6042,
32171,-6218,32171,-6218,32171,-6218,32171,-6218,
32137,-6393,32137,-6393,32137,-6393,32137,-6393,
32102,-6568,32102,-6568,32102,-6568,32102,-6568,
32065,-6743,32065,-6743,32065,-6743,32065,-6743,
32028,-6918,32028,-6918,32028,-6918,32028,-6918,
31990,-7093,31990,-7093,31990,-7093,31990,-7093,
31951,-7267,31951,-7267,31951,-7267,31951,-7267,
31911,-7441,31911,-7441,31911,-7441,31911,-7441,
31869,-7615,31869,-7615,31869,-7615,31869,-7615,
31827,-7789,31827,-7789,31827,-7789,31827,-7789,
31785,-7962,31785,-7962,31785,-7962,31785,-7962,
31741,-8135,31741,-8135,31741,-8135,31741,-8135,
31696,-8308,31696,-8308,31696,-8308,31696,-8308,
31650,-8481,31650,-8481,31650,-8481,31650,-8481,
31603,-8654,31603,-8654,31603,-8654,31603,-8654,
31556,-8826,31556,-8826,31556,-8826,31556,-8826,
31507,-8998,31507,-8998,31507,-8998,31507,-8998,
31457,-9170,31457,-9170,31457,-9170,31457,-9170,
31407,-9341,31407,-9341,31407,-9341,31407,-9341,
31356,-9512,31356,-9512,31356,-9512,31356,-9512,
31303,-9683,31303,-9683,31303,-9683,31303,-9683,
31250,-9854,31250,-9854,31250,-9854,31250,-9854,
31196,-10024,31196,-10024,31196,-10024,31196,-10024,
31141,-10194,31141,-10194,31141,-10194,31141,-10194,
31085,-10364,31085,-10364,31085,-10364,31085,-10364,
31028,-10533,31028,-10533,31028,-10533,31028,-10533,
30970,-10702,30970,-10702,30970,-10702,30970,-10702,
30911,-10871,30911,-10871,30911,-10871,30911,-10871,
30851,-11039,30851,-11039,30851,-11039,30851,-11039,
30790,-11207,30790,-11207,30790,-11207,30790,-11207,
30729,-11375,30729,-11375,30729,-11375,30729,-11375,
30666,-11543,30666,-11543,30666,-11543,30666,-11543,
30603,-11710,30603,-11710,30603,-11710,30603,-11710,
30539,-11877,30539,-11877,30539,-11877,30539,-11877,
30473,-12043,30473,-12043,30473,-12043,30473,-12043,
30407,-12209,30407,-12209,30407,-12209,30407,-12209,
30340,-12375,30340,-12375,30340,-12375,30340,-12375,
30272,-12540,30272,-12540,30272,-12540,30272,-12540,
30203,-12705,30203,-12705,30203,-12705,30203,-12705,
30134,-12869,30134,-12869,30134,-12869,30134,-12869,
30063,-13034,30063,-13034,30063,-13034,30063,-13034,
29992,-13197,29992,-13197,29992,-13197,29992,-13197,
29919,-13361,29919,-13361,29919,-13361,29919,-13361,
29846,-13524,29846,-13524,29846,-13524,29846,-13524,
29772,-13686,29772,-13686,29772,-13686,29772,-13686,
29696,-13848,29696,-13848,29696,-13848,29696,-13848,
29621,-14010,29621,-14010,29621,-14010,29621,-14010,
29544,-14172,29544,-14172,29544,-14172,29544,-14172,
29466,-14332,29466,-14332,29466,-14332,29466,-14332,
29387,-14493,29387,-14493,29387,-14493,29387,-14493,
29308,-14653,29308,-14653,29308,-14653,29308,-14653,
29227,-14813,29227,-14813,29227,-14813,29227,-14813,
29146,-14972,29146,-14972,29146,-14972,29146,-14972,
29064,-15131,29064,-15131,29064,-15131,29064,-15131,
28981,-15289,28981,-15289,28981,-15289,28981,-15289,
28897,-15447,28897,-15447,28897,-15447,28897,-15447,
28813,-15604,28813,-15604,28813,-15604,28813,-15604,
28727,-15761,28727,-15761,28727,-15761,28727,-15761,
28641,-15918,28641,-15918,28641,-15918,28641,-15918,
28554,-16073,28554,-16073,28554,-16073,28554,-16073,
28465,-16229,28465,-16229,28465,-16229,28465,-16229,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
28287,-16539,28287,-16539,28287,-16539,28287,-16539,
28196,-16693,28196,-16693,28196,-16693,28196,-16693,
28105,-16846,28105,-16846,28105,-16846,28105,-16846,
28012,-16999,28012,-16999,28012,-16999,28012,-16999,
27919,-17152,27919,-17152,27919,-17152,27919,-17152,
27825,-17304,27825,-17304,27825,-17304,27825,-17304,
27731,-17455,27731,-17455,27731,-17455,27731,-17455,
27635,-17606,27635,-17606,27635,-17606,27635,-17606,
27538,-17757,27538,-17757,27538,-17757,27538,-17757,
27441,-17907,27441,-17907,27441,-17907,27441,-17907,
27343,-18056,27343,-18056,27343,-18056,27343,-18056,
27244,-18205,27244,-18205,27244,-18205,27244,-18205,
27145,-18353,27145,-18353,27145,-18353,27145,-18353,
27044,-18501,27044,-18501,27044,-18501,27044,-18501,
26943,-18648,26943,-18648,26943,-18648,26943,-18648,
26841,-18795,26841,-18795,26841,-18795,26841,-18795,
26738,-18941,26738,-18941,26738,-18941,26738,-18941,
26634,-19087,26634,-19087,26634,-19087,26634,-19087,
26530,-19232,26530,-19232,26530,-19232,26530,-19232,
26424,-19376,26424,-19376,26424,-19376,26424,-19376,
26318,-19520,26318,-19520,26318,-19520,26318,-19520,
26211,-19663,26211,-19663,26211,-19663,26211,-19663,
26104,-19806,26104,-19806,26104,-19806,26104,-19806,
25995,-19948,25995,-19948,25995,-19948,25995,-19948,
25886,-20089,25886,-20089,25886,-20089,25886,-20089,
25776,-20230,25776,-20230,25776,-20230,25776,-20230,
25665,-20370,25665,-20370,25665,-20370,25665,-20370,
25554,-20510,25554,-20510,25554,-20510,25554,-20510,
25442,-20649,25442,-20649,25442,-20649,25442,-20649,
25329,-20788,25329,-20788,25329,-20788,25329,-20788,
25215,-20926,25215,-20926,25215,-20926,25215,-20926,
25100,-21063,25100,-21063,25100,-21063,25100,-21063,
24985,-21199,24985,-21199,24985,-21199,24985,-21199,
24869,-21335,24869,-21335,24869,-21335,24869,-21335,
24753,-21471,24753,-21471,24753,-21471,24753,-21471,
24635,-21605,24635,-21605,24635,-21605,24635,-21605,
24517,-21739,24517,-21739,24517,-21739,24517,-21739,
24398,-21873,24398,-21873,24398,-21873,24398,-21873,
24278,-22005,24278,-22005,24278,-22005,24278,-22005,
24158,-22138,24158,-22138,24158,-22138,24158,-22138,
24037,-22269,24037,-22269,24037,-22269,24037,-22269,
23915,-22400,23915,-22400,23915,-22400,23915,-22400,
23792,-22530,23792,-22530,23792,-22530,23792,-22530,
23669,-22659,23669,-22659,23669,-22659,23669,-22659,
23545,-22788,23545,-22788,23545,-22788,23545,-22788,
23421,-22916,23421,-22916,23421,-22916,23421,-22916,
23295,-23044,23295,-23044,23295,-23044,23295,-23044,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
23043,-23296,23043,-23296,23043,-23296,23043,-23296,
22915,-23422,22915,-23422,22915,-23422,22915,-23422,
22787,-23546,22787,-23546,22787,-23546,22787,-23546,
22658,-23670,22658,-23670,22658,-23670,22658,-23670,
22529,-23793,22529,-23793,22529,-23793,22529,-23793,
22399,-23916,22399,-23916,22399,-23916,22399,-23916,
22268,-24038,22268,-24038,22268,-24038,22268,-24038,
22137,-24159,22137,-24159,22137,-24159,22137,-24159,
22004,-24279,22004,-24279,22004,-24279,22004,-24279,
21872,-24399,21872,-24399,21872,-24399,21872,-24399,
21738,-24518,21738,-24518,21738,-24518,21738,-24518,
21604,-24636,21604,-24636,21604,-24636,21604,-24636,
21470,-24754,21470,-24754,21470,-24754,21470,-24754,
21334,-24870,21334,-24870,21334,-24870,21334,-24870,
21198,-24986,21198,-24986,21198,-24986,21198,-24986,
21062,-25101,21062,-25101,21062,-25101,21062,-25101,
20925,-25216,20925,-25216,20925,-25216,20925,-25216,
20787,-25330,20787,-25330,20787,-25330,20787,-25330,
20648,-25443,20648,-25443,20648,-25443,20648,-25443,
20509,-25555,20509,-25555,20509,-25555,20509,-25555,
20369,-25666,20369,-25666,20369,-25666,20369,-25666,
20229,-25777,20229,-25777,20229,-25777,20229,-25777,
20088,-25887,20088,-25887,20088,-25887,20088,-25887,
19947,-25996,19947,-25996,19947,-25996,19947,-25996,
19805,-26105,19805,-26105,19805,-26105,19805,-26105,
19662,-26212,19662,-26212,19662,-26212,19662,-26212,
19519,-26319,19519,-26319,19519,-26319,19519,-26319,
19375,-26425,19375,-26425,19375,-26425,19375,-26425,
19231,-26531,19231,-26531,19231,-26531,19231,-26531,
19086,-26635,19086,-26635,19086,-26635,19086,-26635,
18940,-26739,18940,-26739,18940,-26739,18940,-26739,
18794,-26842,18794,-26842,18794,-26842,18794,-26842,
18647,-26944,18647,-26944,18647,-26944,18647,-26944,
18500,-27045,18500,-27045,18500,-27045,18500,-27045,
18352,-27146,18352,-27146,18352,-27146,18352,-27146,
18204,-27245,18204,-27245,18204,-27245,18204,-27245,
18055,-27344,18055,-27344,18055,-27344,18055,-27344,
17906,-27442,17906,-27442,17906,-27442,17906,-27442,
17756,-27539,17756,-27539,17756,-27539,17756,-27539,
17605,-27636,17605,-27636,17605,-27636,17605,-27636,
17454,-27732,17454,-27732,17454,-27732,17454,-27732,
17303,-27826,17303,-27826,17303,-27826,17303,-27826,
17151,-27920,17151,-27920,17151,-27920,17151,-27920,
16998,-28013,16998,-28013,16998,-28013,16998,-28013,
16845,-28106,16845,-28106,16845,-28106,16845,-28106,
16692,-28197,16692,-28197,16692,-28197,16692,-28197,
16538,-28288,16538,-28288,16538,-28288,16538,-28288,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
16228,-28466,16228,-28466,16228,-28466,16228,-28466,
16072,-28555,16072,-28555,16072,-28555,16072,-28555,
15917,-28642,15917,-28642,15917,-28642,15917,-28642,
15760,-28728,15760,-28728,15760,-28728,15760,-28728,
15603,-28814,15603,-28814,15603,-28814,15603,-28814,
15446,-28898,15446,-28898,15446,-28898,15446,-28898,
15288,-28982,15288,-28982,15288,-28982,15288,-28982,
15130,-29065,15130,-29065,15130,-29065,15130,-29065,
14971,-29147,14971,-29147,14971,-29147,14971,-29147,
14812,-29228,14812,-29228,14812,-29228,14812,-29228,
14652,-29309,14652,-29309,14652,-29309,14652,-29309,
14492,-29388,14492,-29388,14492,-29388,14492,-29388,
14331,-29467,14331,-29467,14331,-29467,14331,-29467,
14171,-29545,14171,-29545,14171,-29545,14171,-29545,
14009,-29622,14009,-29622,14009,-29622,14009,-29622,
13847,-29697,13847,-29697,13847,-29697,13847,-29697,
13685,-29773,13685,-29773,13685,-29773,13685,-29773,
13523,-29847,13523,-29847,13523,-29847,13523,-29847,
13360,-29920,13360,-29920,13360,-29920,13360,-29920,
13196,-29993,13196,-29993,13196,-29993,13196,-29993,
13033,-30064,13033,-30064,13033,-30064,13033,-30064,
12868,-30135,12868,-30135,12868,-30135,12868,-30135,
12704,-30204,12704,-30204,12704,-30204,12704,-30204,
12539,-30273,12539,-30273,12539,-30273,12539,-30273,
12374,-30341,12374,-30341,12374,-30341,12374,-30341,
12208,-30408,12208,-30408,12208,-30408,12208,-30408,
12042,-30474,12042,-30474,12042,-30474,12042,-30474,
11876,-30540,11876,-30540,11876,-30540,11876,-30540,
11709,-30604,11709,-30604,11709,-30604,11709,-30604,
11542,-30667,11542,-30667,11542,-30667,11542,-30667,
11374,-30730,11374,-30730,11374,-30730,11374,-30730,
11206,-30791,11206,-30791,11206,-30791,11206,-30791,
11038,-30852,11038,-30852,11038,-30852,11038,-30852,
10870,-30912,10870,-30912,10870,-30912,10870,-30912,
10701,-30971,10701,-30971,10701,-30971,10701,-30971,
10532,-31029,10532,-31029,10532,-31029,10532,-31029,
10363,-31086,10363,-31086,10363,-31086,10363,-31086,
10193,-31142,10193,-31142,10193,-31142,10193,-31142,
10023,-31197,10023,-31197,10023,-31197,10023,-31197,
9853,-31251,9853,-31251,9853,-31251,9853,-31251,
9682,-31304,9682,-31304,9682,-31304,9682,-31304,
9511,-31357,9511,-31357,9511,-31357,9511,-31357,
9340,-31408,9340,-31408,9340,-31408,9340,-31408,
9169,-31458,9169,-31458,9169,-31458,9169,-31458,
8997,-31508,8997,-31508,8997,-31508,8997,-31508,
8825,-31557,8825,-31557,8825,-31557,8825,-31557,
8653,-31604,8653,-31604,8653,-31604,8653,-31604,
8480,-31651,8480,-31651,8480,-31651,8480,-31651,
8307,-31697,8307,-31697,8307,-31697,8307,-31697,
8134,-31742,8134,-31742,8134,-31742,8134,-31742,
7961,-31786,7961,-31786,7961,-31786,7961,-31786,
7788,-31828,7788,-31828,7788,-31828,7788,-31828,
7614,-31870,7614,-31870,7614,-31870,7614,-31870,
7440,-31912,7440,-31912,7440,-31912,7440,-31912,
7266,-31952,7266,-31952,7266,-31952,7266,-31952,
7092,-31991,7092,-31991,7092,-31991,7092,-31991,
6917,-32029,6917,-32029,6917,-32029,6917,-32029,
6742,-32066,6742,-32066,6742,-32066,6742,-32066,
6567,-32103,6567,-32103,6567,-32103,6567,-32103,
6392,-32138,6392,-32138,6392,-32138,6392,-32138,
6217,-32172,6217,-32172,6217,-32172,6217,-32172,
6041,-32206,6041,-32206,6041,-32206,6041,-32206,
5865,-32238,5865,-32238,5865,-32238,5865,-32238,
5689,-32270,5689,-32270,5689,-32270,5689,-32270,
5513,-32300,5513,-32300,5513,-32300,5513,-32300,
5337,-32330,5337,-32330,5337,-32330,5337,-32330,
5161,-32358,5161,-32358,5161,-32358,5161,-32358,
4984,-32386,4984,-32386,4984,-32386,4984,-32386,
4807,-32413,4807,-32413,4807,-32413,4807,-32413,
4631,-32439,4631,-32439,4631,-32439,4631,-32439,
4454,-32463,4454,-32463,4454,-32463,4454,-32463,
4276,-32487,4276,-32487,4276,-32487,4276,-32487,
4099,-32510,4099,-32510,4099,-32510,4099,-32510,
3922,-32532,3922,-32532,3922,-32532,3922,-32532,
3744,-32553,3744,-32553,3744,-32553,3744,-32553,
3567,-32573,3567,-32573,3567,-32573,3567,-32573,
3389,-32592,3389,-32592,3389,-32592,3389,-32592,
3211,-32610,3211,-32610,3211,-32610,3211,-32610,
3033,-32627,3033,-32627,3033,-32627,3033,-32627,
2855,-32643,2855,-32643,2855,-32643,2855,-32643,
2677,-32658,2677,-32658,2677,-32658,2677,-32658,
2499,-32672,2499,-32672,2499,-32672,2499,-32672,
2321,-32685,2321,-32685,2321,-32685,2321,-32685,
2143,-32697,2143,-32697,2143,-32697,2143,-32697,
1964,-32709,1964,-32709,1964,-32709,1964,-32709,
1786,-32719,1786,-32719,1786,-32719,1786,-32719,
1607,-32728,1607,-32728,1607,-32728,1607,-32728,
1429,-32736,1429,-32736,1429,-32736,1429,-32736,
1250,-32744,1250,-32744,1250,-32744,1250,-32744,
1072,-32750,1072,-32750,1072,-32750,1072,-32750,
893,-32755,893,-32755,893,-32755,893,-32755,
714,-32760,714,-32760,714,-32760,714,-32760,
536,-32763,536,-32763,536,-32763,536,-32763,
357,-32766,357,-32766,357,-32766,357,-32766,
178,-32767,178,-32767,178,-32767,178,-32767};

static int16_t twb1152[287*2*4] = {32765,-358,32765,-358,32765,-358,32765,-358,
32759,-715,32759,-715,32759,-715,32759,-715,
32749,-1073,32749,-1073,32749,-1073,32749,-1073,
32735,-1430,32735,-1430,32735,-1430,32735,-1430,
32718,-1787,32718,-1787,32718,-1787,32718,-1787,
32696,-2144,32696,-2144,32696,-2144,32696,-2144,
32671,-2500,32671,-2500,32671,-2500,32671,-2500,
32642,-2856,32642,-2856,32642,-2856,32642,-2856,
32609,-3212,32609,-3212,32609,-3212,32609,-3212,
32572,-3568,32572,-3568,32572,-3568,32572,-3568,
32531,-3923,32531,-3923,32531,-3923,32531,-3923,
32486,-4277,32486,-4277,32486,-4277,32486,-4277,
32438,-4632,32438,-4632,32438,-4632,32438,-4632,
32385,-4985,32385,-4985,32385,-4985,32385,-4985,
32329,-5338,32329,-5338,32329,-5338,32329,-5338,
32269,-5690,32269,-5690,32269,-5690,32269,-5690,
32205,-6042,32205,-6042,32205,-6042,32205,-6042,
32137,-6393,32137,-6393,32137,-6393,32137,-6393,
32065,-6743,32065,-6743,32065,-6743,32065,-6743,
31990,-7093,31990,-7093,31990,-7093,31990,-7093,
31911,-7441,31911,-7441,31911,-7441,31911,-7441,
31827,-7789,31827,-7789,31827,-7789,31827,-7789,
31741,-8135,31741,-8135,31741,-8135,31741,-8135,
31650,-8481,31650,-8481,31650,-8481,31650,-8481,
31556,-8826,31556,-8826,31556,-8826,31556,-8826,
31457,-9170,31457,-9170,31457,-9170,31457,-9170,
31356,-9512,31356,-9512,31356,-9512,31356,-9512,
31250,-9854,31250,-9854,31250,-9854,31250,-9854,
31141,-10194,31141,-10194,31141,-10194,31141,-10194,
31028,-10533,31028,-10533,31028,-10533,31028,-10533,
30911,-10871,30911,-10871,30911,-10871,30911,-10871,
30790,-11207,30790,-11207,30790,-11207,30790,-11207,
30666,-11543,30666,-11543,30666,-11543,30666,-11543,
30539,-11877,30539,-11877,30539,-11877,30539,-11877,
30407,-12209,30407,-12209,30407,-12209,30407,-12209,
30272,-12540,30272,-12540,30272,-12540,30272,-12540,
30134,-12869,30134,-12869,30134,-12869,30134,-12869,
29992,-13197,29992,-13197,29992,-13197,29992,-13197,
29846,-13524,29846,-13524,29846,-13524,29846,-13524,
29696,-13848,29696,-13848,29696,-13848,29696,-13848,
29544,-14172,29544,-14172,29544,-14172,29544,-14172,
29387,-14493,29387,-14493,29387,-14493,29387,-14493,
29227,-14813,29227,-14813,29227,-14813,29227,-14813,
29064,-15131,29064,-15131,29064,-15131,29064,-15131,
28897,-15447,28897,-15447,28897,-15447,28897,-15447,
28727,-15761,28727,-15761,28727,-15761,28727,-15761,
28554,-16073,28554,-16073,28554,-16073,28554,-16073,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
28196,-16693,28196,-16693,28196,-16693,28196,-16693,
28012,-16999,28012,-16999,28012,-16999,28012,-16999,
27825,-17304,27825,-17304,27825,-17304,27825,-17304,
27635,-17606,27635,-17606,27635,-17606,27635,-17606,
27441,-17907,27441,-17907,27441,-17907,27441,-17907,
27244,-18205,27244,-18205,27244,-18205,27244,-18205,
27044,-18501,27044,-18501,27044,-18501,27044,-18501,
26841,-18795,26841,-18795,26841,-18795,26841,-18795,
26634,-19087,26634,-19087,26634,-19087,26634,-19087,
26424,-19376,26424,-19376,26424,-19376,26424,-19376,
26211,-19663,26211,-19663,26211,-19663,26211,-19663,
25995,-19948,25995,-19948,25995,-19948,25995,-19948,
25776,-20230,25776,-20230,25776,-20230,25776,-20230,
25554,-20510,25554,-20510,25554,-20510,25554,-20510,
25329,-20788,25329,-20788,25329,-20788,25329,-20788,
25100,-21063,25100,-21063,25100,-21063,25100,-21063,
24869,-21335,24869,-21335,24869,-21335,24869,-21335,
24635,-21605,24635,-21605,24635,-21605,24635,-21605,
24398,-21873,24398,-21873,24398,-21873,24398,-21873,
24158,-22138,24158,-22138,24158,-22138,24158,-22138,
23915,-22400,23915,-22400,23915,-22400,23915,-22400,
23669,-22659,23669,-22659,23669,-22659,23669,-22659,
23421,-22916,23421,-22916,23421,-22916,23421,-22916,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
22915,-23422,22915,-23422,22915,-23422,22915,-23422,
22658,-23670,22658,-23670,22658,-23670,22658,-23670,
22399,-23916,22399,-23916,22399,-23916,22399,-23916,
22137,-24159,22137,-24159,22137,-24159,22137,-24159,
21872,-24399,21872,-24399,21872,-24399,21872,-24399,
21604,-24636,21604,-24636,21604,-24636,21604,-24636,
21334,-24870,21334,-24870,21334,-24870,21334,-24870,
21062,-25101,21062,-25101,21062,-25101,21062,-25101,
20787,-25330,20787,-25330,20787,-25330,20787,-25330,
20509,-25555,20509,-25555,20509,-25555,20509,-25555,
20229,-25777,20229,-25777,20229,-25777,20229,-25777,
19947,-25996,19947,-25996,19947,-25996,19947,-25996,
19662,-26212,19662,-26212,19662,-26212,19662,-26212,
19375,-26425,19375,-26425,19375,-26425,19375,-26425,
19086,-26635,19086,-26635,19086,-26635,19086,-26635,
18794,-26842,18794,-26842,18794,-26842,18794,-26842,
18500,-27045,18500,-27045,18500,-27045,18500,-27045,
18204,-27245,18204,-27245,18204,-27245,18204,-27245,
17906,-27442,17906,-27442,17906,-27442,17906,-27442,
17605,-27636,17605,-27636,17605,-27636,17605,-27636,
17303,-27826,17303,-27826,17303,-27826,17303,-27826,
16998,-28013,16998,-28013,16998,-28013,16998,-28013,
16692,-28197,16692,-28197,16692,-28197,16692,-28197,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
16072,-28555,16072,-28555,16072,-28555,16072,-28555,
15760,-28728,15760,-28728,15760,-28728,15760,-28728,
15446,-28898,15446,-28898,15446,-28898,15446,-28898,
15130,-29065,15130,-29065,15130,-29065,15130,-29065,
14812,-29228,14812,-29228,14812,-29228,14812,-29228,
14492,-29388,14492,-29388,14492,-29388,14492,-29388,
14171,-29545,14171,-29545,14171,-29545,14171,-29545,
13847,-29697,13847,-29697,13847,-29697,13847,-29697,
13523,-29847,13523,-29847,13523,-29847,13523,-29847,
13196,-29993,13196,-29993,13196,-29993,13196,-29993,
12868,-30135,12868,-30135,12868,-30135,12868,-30135,
12539,-30273,12539,-30273,12539,-30273,12539,-30273,
12208,-30408,12208,-30408,12208,-30408,12208,-30408,
11876,-30540,11876,-30540,11876,-30540,11876,-30540,
11542,-30667,11542,-30667,11542,-30667,11542,-30667,
11206,-30791,11206,-30791,11206,-30791,11206,-30791,
10870,-30912,10870,-30912,10870,-30912,10870,-30912,
10532,-31029,10532,-31029,10532,-31029,10532,-31029,
10193,-31142,10193,-31142,10193,-31142,10193,-31142,
9853,-31251,9853,-31251,9853,-31251,9853,-31251,
9511,-31357,9511,-31357,9511,-31357,9511,-31357,
9169,-31458,9169,-31458,9169,-31458,9169,-31458,
8825,-31557,8825,-31557,8825,-31557,8825,-31557,
8480,-31651,8480,-31651,8480,-31651,8480,-31651,
8134,-31742,8134,-31742,8134,-31742,8134,-31742,
7788,-31828,7788,-31828,7788,-31828,7788,-31828,
7440,-31912,7440,-31912,7440,-31912,7440,-31912,
7092,-31991,7092,-31991,7092,-31991,7092,-31991,
6742,-32066,6742,-32066,6742,-32066,6742,-32066,
6392,-32138,6392,-32138,6392,-32138,6392,-32138,
6041,-32206,6041,-32206,6041,-32206,6041,-32206,
5689,-32270,5689,-32270,5689,-32270,5689,-32270,
5337,-32330,5337,-32330,5337,-32330,5337,-32330,
4984,-32386,4984,-32386,4984,-32386,4984,-32386,
4631,-32439,4631,-32439,4631,-32439,4631,-32439,
4276,-32487,4276,-32487,4276,-32487,4276,-32487,
3922,-32532,3922,-32532,3922,-32532,3922,-32532,
3567,-32573,3567,-32573,3567,-32573,3567,-32573,
3211,-32610,3211,-32610,3211,-32610,3211,-32610,
2855,-32643,2855,-32643,2855,-32643,2855,-32643,
2499,-32672,2499,-32672,2499,-32672,2499,-32672,
2143,-32697,2143,-32697,2143,-32697,2143,-32697,
1786,-32719,1786,-32719,1786,-32719,1786,-32719,
1429,-32736,1429,-32736,1429,-32736,1429,-32736,
1072,-32750,1072,-32750,1072,-32750,1072,-32750,
714,-32760,714,-32760,714,-32760,714,-32760,
357,-32766,357,-32766,357,-32766,357,-32766,
0,-32767,0,-32767,0,-32767,0,-32767,
-358,-32766,-358,-32766,-358,-32766,-358,-32766,
-715,-32760,-715,-32760,-715,-32760,-715,-32760,
-1073,-32750,-1073,-32750,-1073,-32750,-1073,-32750,
-1430,-32736,-1430,-32736,-1430,-32736,-1430,-32736,
-1787,-32719,-1787,-32719,-1787,-32719,-1787,-32719,
-2144,-32697,-2144,-32697,-2144,-32697,-2144,-32697,
-2500,-32672,-2500,-32672,-2500,-32672,-2500,-32672,
-2856,-32643,-2856,-32643,-2856,-32643,-2856,-32643,
-3212,-32610,-3212,-32610,-3212,-32610,-3212,-32610,
-3568,-32573,-3568,-32573,-3568,-32573,-3568,-32573,
-3923,-32532,-3923,-32532,-3923,-32532,-3923,-32532,
-4277,-32487,-4277,-32487,-4277,-32487,-4277,-32487,
-4632,-32439,-4632,-32439,-4632,-32439,-4632,-32439,
-4985,-32386,-4985,-32386,-4985,-32386,-4985,-32386,
-5338,-32330,-5338,-32330,-5338,-32330,-5338,-32330,
-5690,-32270,-5690,-32270,-5690,-32270,-5690,-32270,
-6042,-32206,-6042,-32206,-6042,-32206,-6042,-32206,
-6393,-32138,-6393,-32138,-6393,-32138,-6393,-32138,
-6743,-32066,-6743,-32066,-6743,-32066,-6743,-32066,
-7093,-31991,-7093,-31991,-7093,-31991,-7093,-31991,
-7441,-31912,-7441,-31912,-7441,-31912,-7441,-31912,
-7789,-31828,-7789,-31828,-7789,-31828,-7789,-31828,
-8135,-31742,-8135,-31742,-8135,-31742,-8135,-31742,
-8481,-31651,-8481,-31651,-8481,-31651,-8481,-31651,
-8826,-31557,-8826,-31557,-8826,-31557,-8826,-31557,
-9170,-31458,-9170,-31458,-9170,-31458,-9170,-31458,
-9512,-31357,-9512,-31357,-9512,-31357,-9512,-31357,
-9854,-31251,-9854,-31251,-9854,-31251,-9854,-31251,
-10194,-31142,-10194,-31142,-10194,-31142,-10194,-31142,
-10533,-31029,-10533,-31029,-10533,-31029,-10533,-31029,
-10871,-30912,-10871,-30912,-10871,-30912,-10871,-30912,
-11207,-30791,-11207,-30791,-11207,-30791,-11207,-30791,
-11543,-30667,-11543,-30667,-11543,-30667,-11543,-30667,
-11877,-30540,-11877,-30540,-11877,-30540,-11877,-30540,
-12209,-30408,-12209,-30408,-12209,-30408,-12209,-30408,
-12540,-30273,-12540,-30273,-12540,-30273,-12540,-30273,
-12869,-30135,-12869,-30135,-12869,-30135,-12869,-30135,
-13197,-29993,-13197,-29993,-13197,-29993,-13197,-29993,
-13524,-29847,-13524,-29847,-13524,-29847,-13524,-29847,
-13848,-29697,-13848,-29697,-13848,-29697,-13848,-29697,
-14172,-29545,-14172,-29545,-14172,-29545,-14172,-29545,
-14493,-29388,-14493,-29388,-14493,-29388,-14493,-29388,
-14813,-29228,-14813,-29228,-14813,-29228,-14813,-29228,
-15131,-29065,-15131,-29065,-15131,-29065,-15131,-29065,
-15447,-28898,-15447,-28898,-15447,-28898,-15447,-28898,
-15761,-28728,-15761,-28728,-15761,-28728,-15761,-28728,
-16073,-28555,-16073,-28555,-16073,-28555,-16073,-28555,
-16384,-28378,-16384,-28378,-16384,-28378,-16384,-28378,
-16693,-28197,-16693,-28197,-16693,-28197,-16693,-28197,
-16999,-28013,-16999,-28013,-16999,-28013,-16999,-28013,
-17304,-27826,-17304,-27826,-17304,-27826,-17304,-27826,
-17606,-27636,-17606,-27636,-17606,-27636,-17606,-27636,
-17907,-27442,-17907,-27442,-17907,-27442,-17907,-27442,
-18205,-27245,-18205,-27245,-18205,-27245,-18205,-27245,
-18501,-27045,-18501,-27045,-18501,-27045,-18501,-27045,
-18795,-26842,-18795,-26842,-18795,-26842,-18795,-26842,
-19087,-26635,-19087,-26635,-19087,-26635,-19087,-26635,
-19376,-26425,-19376,-26425,-19376,-26425,-19376,-26425,
-19663,-26212,-19663,-26212,-19663,-26212,-19663,-26212,
-19948,-25996,-19948,-25996,-19948,-25996,-19948,-25996,
-20230,-25777,-20230,-25777,-20230,-25777,-20230,-25777,
-20510,-25555,-20510,-25555,-20510,-25555,-20510,-25555,
-20788,-25330,-20788,-25330,-20788,-25330,-20788,-25330,
-21063,-25101,-21063,-25101,-21063,-25101,-21063,-25101,
-21335,-24870,-21335,-24870,-21335,-24870,-21335,-24870,
-21605,-24636,-21605,-24636,-21605,-24636,-21605,-24636,
-21873,-24399,-21873,-24399,-21873,-24399,-21873,-24399,
-22138,-24159,-22138,-24159,-22138,-24159,-22138,-24159,
-22400,-23916,-22400,-23916,-22400,-23916,-22400,-23916,
-22659,-23670,-22659,-23670,-22659,-23670,-22659,-23670,
-22916,-23422,-22916,-23422,-22916,-23422,-22916,-23422,
-23170,-23170,-23170,-23170,-23170,-23170,-23170,-23170,
-23422,-22916,-23422,-22916,-23422,-22916,-23422,-22916,
-23670,-22659,-23670,-22659,-23670,-22659,-23670,-22659,
-23916,-22400,-23916,-22400,-23916,-22400,-23916,-22400,
-24159,-22138,-24159,-22138,-24159,-22138,-24159,-22138,
-24399,-21873,-24399,-21873,-24399,-21873,-24399,-21873,
-24636,-21605,-24636,-21605,-24636,-21605,-24636,-21605,
-24870,-21335,-24870,-21335,-24870,-21335,-24870,-21335,
-25101,-21063,-25101,-21063,-25101,-21063,-25101,-21063,
-25330,-20788,-25330,-20788,-25330,-20788,-25330,-20788,
-25555,-20510,-25555,-20510,-25555,-20510,-25555,-20510,
-25777,-20230,-25777,-20230,-25777,-20230,-25777,-20230,
-25996,-19948,-25996,-19948,-25996,-19948,-25996,-19948,
-26212,-19663,-26212,-19663,-26212,-19663,-26212,-19663,
-26425,-19376,-26425,-19376,-26425,-19376,-26425,-19376,
-26635,-19087,-26635,-19087,-26635,-19087,-26635,-19087,
-26842,-18795,-26842,-18795,-26842,-18795,-26842,-18795,
-27045,-18501,-27045,-18501,-27045,-18501,-27045,-18501,
-27245,-18205,-27245,-18205,-27245,-18205,-27245,-18205,
-27442,-17907,-27442,-17907,-27442,-17907,-27442,-17907,
-27636,-17606,-27636,-17606,-27636,-17606,-27636,-17606,
-27826,-17304,-27826,-17304,-27826,-17304,-27826,-17304,
-28013,-16999,-28013,-16999,-28013,-16999,-28013,-16999,
-28197,-16693,-28197,-16693,-28197,-16693,-28197,-16693,
-28378,-16384,-28378,-16384,-28378,-16384,-28378,-16384,
-28555,-16073,-28555,-16073,-28555,-16073,-28555,-16073,
-28728,-15761,-28728,-15761,-28728,-15761,-28728,-15761,
-28898,-15447,-28898,-15447,-28898,-15447,-28898,-15447,
-29065,-15131,-29065,-15131,-29065,-15131,-29065,-15131,
-29228,-14813,-29228,-14813,-29228,-14813,-29228,-14813,
-29388,-14493,-29388,-14493,-29388,-14493,-29388,-14493,
-29545,-14172,-29545,-14172,-29545,-14172,-29545,-14172,
-29697,-13848,-29697,-13848,-29697,-13848,-29697,-13848,
-29847,-13524,-29847,-13524,-29847,-13524,-29847,-13524,
-29993,-13197,-29993,-13197,-29993,-13197,-29993,-13197,
-30135,-12869,-30135,-12869,-30135,-12869,-30135,-12869,
-30273,-12540,-30273,-12540,-30273,-12540,-30273,-12540,
-30408,-12209,-30408,-12209,-30408,-12209,-30408,-12209,
-30540,-11877,-30540,-11877,-30540,-11877,-30540,-11877,
-30667,-11543,-30667,-11543,-30667,-11543,-30667,-11543,
-30791,-11207,-30791,-11207,-30791,-11207,-30791,-11207,
-30912,-10871,-30912,-10871,-30912,-10871,-30912,-10871,
-31029,-10533,-31029,-10533,-31029,-10533,-31029,-10533,
-31142,-10194,-31142,-10194,-31142,-10194,-31142,-10194,
-31251,-9854,-31251,-9854,-31251,-9854,-31251,-9854,
-31357,-9512,-31357,-9512,-31357,-9512,-31357,-9512,
-31458,-9170,-31458,-9170,-31458,-9170,-31458,-9170,
-31557,-8826,-31557,-8826,-31557,-8826,-31557,-8826,
-31651,-8481,-31651,-8481,-31651,-8481,-31651,-8481,
-31742,-8135,-31742,-8135,-31742,-8135,-31742,-8135,
-31828,-7789,-31828,-7789,-31828,-7789,-31828,-7789,
-31912,-7441,-31912,-7441,-31912,-7441,-31912,-7441,
-31991,-7093,-31991,-7093,-31991,-7093,-31991,-7093,
-32066,-6743,-32066,-6743,-32066,-6743,-32066,-6743,
-32138,-6393,-32138,-6393,-32138,-6393,-32138,-6393,
-32206,-6042,-32206,-6042,-32206,-6042,-32206,-6042,
-32270,-5690,-32270,-5690,-32270,-5690,-32270,-5690,
-32330,-5338,-32330,-5338,-32330,-5338,-32330,-5338,
-32386,-4985,-32386,-4985,-32386,-4985,-32386,-4985,
-32439,-4632,-32439,-4632,-32439,-4632,-32439,-4632,
-32487,-4277,-32487,-4277,-32487,-4277,-32487,-4277,
-32532,-3923,-32532,-3923,-32532,-3923,-32532,-3923,
-32573,-3568,-32573,-3568,-32573,-3568,-32573,-3568,
-32610,-3212,-32610,-3212,-32610,-3212,-32610,-3212,
-32643,-2856,-32643,-2856,-32643,-2856,-32643,-2856,
-32672,-2500,-32672,-2500,-32672,-2500,-32672,-2500,
-32697,-2144,-32697,-2144,-32697,-2144,-32697,-2144,
-32719,-1787,-32719,-1787,-32719,-1787,-32719,-1787,
-32736,-1430,-32736,-1430,-32736,-1430,-32736,-1430,
-32750,-1073,-32750,-1073,-32750,-1073,-32750,-1073,
-32760,-715,-32760,-715,-32760,-715,-32760,-715,
-32766,-358,-32766,-358,-32766,-358,-32766,-358};

static int16_t twc1152[287*2*4] = {32762,-537,32762,-537,32762,-537,32762,-537,
32749,-1073,32749,-1073,32749,-1073,32749,-1073,
32727,-1608,32727,-1608,32727,-1608,32727,-1608,
32696,-2144,32696,-2144,32696,-2144,32696,-2144,
32657,-2678,32657,-2678,32657,-2678,32657,-2678,
32609,-3212,32609,-3212,32609,-3212,32609,-3212,
32552,-3745,32552,-3745,32552,-3745,32552,-3745,
32486,-4277,32486,-4277,32486,-4277,32486,-4277,
32412,-4808,32412,-4808,32412,-4808,32412,-4808,
32329,-5338,32329,-5338,32329,-5338,32329,-5338,
32237,-5866,32237,-5866,32237,-5866,32237,-5866,
32137,-6393,32137,-6393,32137,-6393,32137,-6393,
32028,-6918,32028,-6918,32028,-6918,32028,-6918,
31911,-7441,31911,-7441,31911,-7441,31911,-7441,
31785,-7962,31785,-7962,31785,-7962,31785,-7962,
31650,-8481,31650,-8481,31650,-8481,31650,-8481,
31507,-8998,31507,-8998,31507,-8998,31507,-8998,
31356,-9512,31356,-9512,31356,-9512,31356,-9512,
31196,-10024,31196,-10024,31196,-10024,31196,-10024,
31028,-10533,31028,-10533,31028,-10533,31028,-10533,
30851,-11039,30851,-11039,30851,-11039,30851,-11039,
30666,-11543,30666,-11543,30666,-11543,30666,-11543,
30473,-12043,30473,-12043,30473,-12043,30473,-12043,
30272,-12540,30272,-12540,30272,-12540,30272,-12540,
30063,-13034,30063,-13034,30063,-13034,30063,-13034,
29846,-13524,29846,-13524,29846,-13524,29846,-13524,
29621,-14010,29621,-14010,29621,-14010,29621,-14010,
29387,-14493,29387,-14493,29387,-14493,29387,-14493,
29146,-14972,29146,-14972,29146,-14972,29146,-14972,
28897,-15447,28897,-15447,28897,-15447,28897,-15447,
28641,-15918,28641,-15918,28641,-15918,28641,-15918,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
28105,-16846,28105,-16846,28105,-16846,28105,-16846,
27825,-17304,27825,-17304,27825,-17304,27825,-17304,
27538,-17757,27538,-17757,27538,-17757,27538,-17757,
27244,-18205,27244,-18205,27244,-18205,27244,-18205,
26943,-18648,26943,-18648,26943,-18648,26943,-18648,
26634,-19087,26634,-19087,26634,-19087,26634,-19087,
26318,-19520,26318,-19520,26318,-19520,26318,-19520,
25995,-19948,25995,-19948,25995,-19948,25995,-19948,
25665,-20370,25665,-20370,25665,-20370,25665,-20370,
25329,-20788,25329,-20788,25329,-20788,25329,-20788,
24985,-21199,24985,-21199,24985,-21199,24985,-21199,
24635,-21605,24635,-21605,24635,-21605,24635,-21605,
24278,-22005,24278,-22005,24278,-22005,24278,-22005,
23915,-22400,23915,-22400,23915,-22400,23915,-22400,
23545,-22788,23545,-22788,23545,-22788,23545,-22788,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
22787,-23546,22787,-23546,22787,-23546,22787,-23546,
22399,-23916,22399,-23916,22399,-23916,22399,-23916,
22004,-24279,22004,-24279,22004,-24279,22004,-24279,
21604,-24636,21604,-24636,21604,-24636,21604,-24636,
21198,-24986,21198,-24986,21198,-24986,21198,-24986,
20787,-25330,20787,-25330,20787,-25330,20787,-25330,
20369,-25666,20369,-25666,20369,-25666,20369,-25666,
19947,-25996,19947,-25996,19947,-25996,19947,-25996,
19519,-26319,19519,-26319,19519,-26319,19519,-26319,
19086,-26635,19086,-26635,19086,-26635,19086,-26635,
18647,-26944,18647,-26944,18647,-26944,18647,-26944,
18204,-27245,18204,-27245,18204,-27245,18204,-27245,
17756,-27539,17756,-27539,17756,-27539,17756,-27539,
17303,-27826,17303,-27826,17303,-27826,17303,-27826,
16845,-28106,16845,-28106,16845,-28106,16845,-28106,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
15917,-28642,15917,-28642,15917,-28642,15917,-28642,
15446,-28898,15446,-28898,15446,-28898,15446,-28898,
14971,-29147,14971,-29147,14971,-29147,14971,-29147,
14492,-29388,14492,-29388,14492,-29388,14492,-29388,
14009,-29622,14009,-29622,14009,-29622,14009,-29622,
13523,-29847,13523,-29847,13523,-29847,13523,-29847,
13033,-30064,13033,-30064,13033,-30064,13033,-30064,
12539,-30273,12539,-30273,12539,-30273,12539,-30273,
12042,-30474,12042,-30474,12042,-30474,12042,-30474,
11542,-30667,11542,-30667,11542,-30667,11542,-30667,
11038,-30852,11038,-30852,11038,-30852,11038,-30852,
10532,-31029,10532,-31029,10532,-31029,10532,-31029,
10023,-31197,10023,-31197,10023,-31197,10023,-31197,
9511,-31357,9511,-31357,9511,-31357,9511,-31357,
8997,-31508,8997,-31508,8997,-31508,8997,-31508,
8480,-31651,8480,-31651,8480,-31651,8480,-31651,
7961,-31786,7961,-31786,7961,-31786,7961,-31786,
7440,-31912,7440,-31912,7440,-31912,7440,-31912,
6917,-32029,6917,-32029,6917,-32029,6917,-32029,
6392,-32138,6392,-32138,6392,-32138,6392,-32138,
5865,-32238,5865,-32238,5865,-32238,5865,-32238,
5337,-32330,5337,-32330,5337,-32330,5337,-32330,
4807,-32413,4807,-32413,4807,-32413,4807,-32413,
4276,-32487,4276,-32487,4276,-32487,4276,-32487,
3744,-32553,3744,-32553,3744,-32553,3744,-32553,
3211,-32610,3211,-32610,3211,-32610,3211,-32610,
2677,-32658,2677,-32658,2677,-32658,2677,-32658,
2143,-32697,2143,-32697,2143,-32697,2143,-32697,
1607,-32728,1607,-32728,1607,-32728,1607,-32728,
1072,-32750,1072,-32750,1072,-32750,1072,-32750,
536,-32763,536,-32763,536,-32763,536,-32763,
0,-32767,0,-32767,0,-32767,0,-32767,
-537,-32763,-537,-32763,-537,-32763,-537,-32763,
-1073,-32750,-1073,-32750,-1073,-32750,-1073,-32750,
-1608,-32728,-1608,-32728,-1608,-32728,-1608,-32728,
-2144,-32697,-2144,-32697,-2144,-32697,-2144,-32697,
-2678,-32658,-2678,-32658,-2678,-32658,-2678,-32658,
-3212,-32610,-3212,-32610,-3212,-32610,-3212,-32610,
-3745,-32553,-3745,-32553,-3745,-32553,-3745,-32553,
-4277,-32487,-4277,-32487,-4277,-32487,-4277,-32487,
-4808,-32413,-4808,-32413,-4808,-32413,-4808,-32413,
-5338,-32330,-5338,-32330,-5338,-32330,-5338,-32330,
-5866,-32238,-5866,-32238,-5866,-32238,-5866,-32238,
-6393,-32138,-6393,-32138,-6393,-32138,-6393,-32138,
-6918,-32029,-6918,-32029,-6918,-32029,-6918,-32029,
-7441,-31912,-7441,-31912,-7441,-31912,-7441,-31912,
-7962,-31786,-7962,-31786,-7962,-31786,-7962,-31786,
-8481,-31651,-8481,-31651,-8481,-31651,-8481,-31651,
-8998,-31508,-8998,-31508,-8998,-31508,-8998,-31508,
-9512,-31357,-9512,-31357,-9512,-31357,-9512,-31357,
-10024,-31197,-10024,-31197,-10024,-31197,-10024,-31197,
-10533,-31029,-10533,-31029,-10533,-31029,-10533,-31029,
-11039,-30852,-11039,-30852,-11039,-30852,-11039,-30852,
-11543,-30667,-11543,-30667,-11543,-30667,-11543,-30667,
-12043,-30474,-12043,-30474,-12043,-30474,-12043,-30474,
-12540,-30273,-12540,-30273,-12540,-30273,-12540,-30273,
-13034,-30064,-13034,-30064,-13034,-30064,-13034,-30064,
-13524,-29847,-13524,-29847,-13524,-29847,-13524,-29847,
-14010,-29622,-14010,-29622,-14010,-29622,-14010,-29622,
-14493,-29388,-14493,-29388,-14493,-29388,-14493,-29388,
-14972,-29147,-14972,-29147,-14972,-29147,-14972,-29147,
-15447,-28898,-15447,-28898,-15447,-28898,-15447,-28898,
-15918,-28642,-15918,-28642,-15918,-28642,-15918,-28642,
-16384,-28378,-16384,-28378,-16384,-28378,-16384,-28378,
-16846,-28106,-16846,-28106,-16846,-28106,-16846,-28106,
-17304,-27826,-17304,-27826,-17304,-27826,-17304,-27826,
-17757,-27539,-17757,-27539,-17757,-27539,-17757,-27539,
-18205,-27245,-18205,-27245,-18205,-27245,-18205,-27245,
-18648,-26944,-18648,-26944,-18648,-26944,-18648,-26944,
-19087,-26635,-19087,-26635,-19087,-26635,-19087,-26635,
-19520,-26319,-19520,-26319,-19520,-26319,-19520,-26319,
-19948,-25996,-19948,-25996,-19948,-25996,-19948,-25996,
-20370,-25666,-20370,-25666,-20370,-25666,-20370,-25666,
-20788,-25330,-20788,-25330,-20788,-25330,-20788,-25330,
-21199,-24986,-21199,-24986,-21199,-24986,-21199,-24986,
-21605,-24636,-21605,-24636,-21605,-24636,-21605,-24636,
-22005,-24279,-22005,-24279,-22005,-24279,-22005,-24279,
-22400,-23916,-22400,-23916,-22400,-23916,-22400,-23916,
-22788,-23546,-22788,-23546,-22788,-23546,-22788,-23546,
-23170,-23170,-23170,-23170,-23170,-23170,-23170,-23170,
-23546,-22788,-23546,-22788,-23546,-22788,-23546,-22788,
-23916,-22400,-23916,-22400,-23916,-22400,-23916,-22400,
-24279,-22005,-24279,-22005,-24279,-22005,-24279,-22005,
-24636,-21605,-24636,-21605,-24636,-21605,-24636,-21605,
-24986,-21199,-24986,-21199,-24986,-21199,-24986,-21199,
-25330,-20788,-25330,-20788,-25330,-20788,-25330,-20788,
-25666,-20370,-25666,-20370,-25666,-20370,-25666,-20370,
-25996,-19948,-25996,-19948,-25996,-19948,-25996,-19948,
-26319,-19520,-26319,-19520,-26319,-19520,-26319,-19520,
-26635,-19087,-26635,-19087,-26635,-19087,-26635,-19087,
-26944,-18648,-26944,-18648,-26944,-18648,-26944,-18648,
-27245,-18205,-27245,-18205,-27245,-18205,-27245,-18205,
-27539,-17757,-27539,-17757,-27539,-17757,-27539,-17757,
-27826,-17304,-27826,-17304,-27826,-17304,-27826,-17304,
-28106,-16846,-28106,-16846,-28106,-16846,-28106,-16846,
-28378,-16384,-28378,-16384,-28378,-16384,-28378,-16384,
-28642,-15918,-28642,-15918,-28642,-15918,-28642,-15918,
-28898,-15447,-28898,-15447,-28898,-15447,-28898,-15447,
-29147,-14972,-29147,-14972,-29147,-14972,-29147,-14972,
-29388,-14493,-29388,-14493,-29388,-14493,-29388,-14493,
-29622,-14010,-29622,-14010,-29622,-14010,-29622,-14010,
-29847,-13524,-29847,-13524,-29847,-13524,-29847,-13524,
-30064,-13034,-30064,-13034,-30064,-13034,-30064,-13034,
-30273,-12540,-30273,-12540,-30273,-12540,-30273,-12540,
-30474,-12043,-30474,-12043,-30474,-12043,-30474,-12043,
-30667,-11543,-30667,-11543,-30667,-11543,-30667,-11543,
-30852,-11039,-30852,-11039,-30852,-11039,-30852,-11039,
-31029,-10533,-31029,-10533,-31029,-10533,-31029,-10533,
-31197,-10024,-31197,-10024,-31197,-10024,-31197,-10024,
-31357,-9512,-31357,-9512,-31357,-9512,-31357,-9512,
-31508,-8998,-31508,-8998,-31508,-8998,-31508,-8998,
-31651,-8481,-31651,-8481,-31651,-8481,-31651,-8481,
-31786,-7962,-31786,-7962,-31786,-7962,-31786,-7962,
-31912,-7441,-31912,-7441,-31912,-7441,-31912,-7441,
-32029,-6918,-32029,-6918,-32029,-6918,-32029,-6918,
-32138,-6393,-32138,-6393,-32138,-6393,-32138,-6393,
-32238,-5866,-32238,-5866,-32238,-5866,-32238,-5866,
-32330,-5338,-32330,-5338,-32330,-5338,-32330,-5338,
-32413,-4808,-32413,-4808,-32413,-4808,-32413,-4808,
-32487,-4277,-32487,-4277,-32487,-4277,-32487,-4277,
-32553,-3745,-32553,-3745,-32553,-3745,-32553,-3745,
-32610,-3212,-32610,-3212,-32610,-3212,-32610,-3212,
-32658,-2678,-32658,-2678,-32658,-2678,-32658,-2678,
-32697,-2144,-32697,-2144,-32697,-2144,-32697,-2144,
-32728,-1608,-32728,-1608,-32728,-1608,-32728,-1608,
-32750,-1073,-32750,-1073,-32750,-1073,-32750,-1073,
-32763,-537,-32763,-537,-32763,-537,-32763,-537,
-32767,-1,-32767,-1,-32767,-1,-32767,-1,
-32763,536,-32763,536,-32763,536,-32763,536,
-32750,1072,-32750,1072,-32750,1072,-32750,1072,
-32728,1607,-32728,1607,-32728,1607,-32728,1607,
-32697,2143,-32697,2143,-32697,2143,-32697,2143,
-32658,2677,-32658,2677,-32658,2677,-32658,2677,
-32610,3211,-32610,3211,-32610,3211,-32610,3211,
-32553,3744,-32553,3744,-32553,3744,-32553,3744,
-32487,4276,-32487,4276,-32487,4276,-32487,4276,
-32413,4807,-32413,4807,-32413,4807,-32413,4807,
-32330,5337,-32330,5337,-32330,5337,-32330,5337,
-32238,5865,-32238,5865,-32238,5865,-32238,5865,
-32138,6392,-32138,6392,-32138,6392,-32138,6392,
-32029,6917,-32029,6917,-32029,6917,-32029,6917,
-31912,7440,-31912,7440,-31912,7440,-31912,7440,
-31786,7961,-31786,7961,-31786,7961,-31786,7961,
-31651,8480,-31651,8480,-31651,8480,-31651,8480,
-31508,8997,-31508,8997,-31508,8997,-31508,8997,
-31357,9511,-31357,9511,-31357,9511,-31357,9511,
-31197,10023,-31197,10023,-31197,10023,-31197,10023,
-31029,10532,-31029,10532,-31029,10532,-31029,10532,
-30852,11038,-30852,11038,-30852,11038,-30852,11038,
-30667,11542,-30667,11542,-30667,11542,-30667,11542,
-30474,12042,-30474,12042,-30474,12042,-30474,12042,
-30273,12539,-30273,12539,-30273,12539,-30273,12539,
-30064,13033,-30064,13033,-30064,13033,-30064,13033,
-29847,13523,-29847,13523,-29847,13523,-29847,13523,
-29622,14009,-29622,14009,-29622,14009,-29622,14009,
-29388,14492,-29388,14492,-29388,14492,-29388,14492,
-29147,14971,-29147,14971,-29147,14971,-29147,14971,
-28898,15446,-28898,15446,-28898,15446,-28898,15446,
-28642,15917,-28642,15917,-28642,15917,-28642,15917,
-28378,16383,-28378,16383,-28378,16383,-28378,16383,
-28106,16845,-28106,16845,-28106,16845,-28106,16845,
-27826,17303,-27826,17303,-27826,17303,-27826,17303,
-27539,17756,-27539,17756,-27539,17756,-27539,17756,
-27245,18204,-27245,18204,-27245,18204,-27245,18204,
-26944,18647,-26944,18647,-26944,18647,-26944,18647,
-26635,19086,-26635,19086,-26635,19086,-26635,19086,
-26319,19519,-26319,19519,-26319,19519,-26319,19519,
-25996,19947,-25996,19947,-25996,19947,-25996,19947,
-25666,20369,-25666,20369,-25666,20369,-25666,20369,
-25330,20787,-25330,20787,-25330,20787,-25330,20787,
-24986,21198,-24986,21198,-24986,21198,-24986,21198,
-24636,21604,-24636,21604,-24636,21604,-24636,21604,
-24279,22004,-24279,22004,-24279,22004,-24279,22004,
-23916,22399,-23916,22399,-23916,22399,-23916,22399,
-23546,22787,-23546,22787,-23546,22787,-23546,22787,
-23170,23169,-23170,23169,-23170,23169,-23170,23169,
-22788,23545,-22788,23545,-22788,23545,-22788,23545,
-22400,23915,-22400,23915,-22400,23915,-22400,23915,
-22005,24278,-22005,24278,-22005,24278,-22005,24278,
-21605,24635,-21605,24635,-21605,24635,-21605,24635,
-21199,24985,-21199,24985,-21199,24985,-21199,24985,
-20788,25329,-20788,25329,-20788,25329,-20788,25329,
-20370,25665,-20370,25665,-20370,25665,-20370,25665,
-19948,25995,-19948,25995,-19948,25995,-19948,25995,
-19520,26318,-19520,26318,-19520,26318,-19520,26318,
-19087,26634,-19087,26634,-19087,26634,-19087,26634,
-18648,26943,-18648,26943,-18648,26943,-18648,26943,
-18205,27244,-18205,27244,-18205,27244,-18205,27244,
-17757,27538,-17757,27538,-17757,27538,-17757,27538,
-17304,27825,-17304,27825,-17304,27825,-17304,27825,
-16846,28105,-16846,28105,-16846,28105,-16846,28105,
-16384,28377,-16384,28377,-16384,28377,-16384,28377,
-15918,28641,-15918,28641,-15918,28641,-15918,28641,
-15447,28897,-15447,28897,-15447,28897,-15447,28897,
-14972,29146,-14972,29146,-14972,29146,-14972,29146,
-14493,29387,-14493,29387,-14493,29387,-14493,29387,
-14010,29621,-14010,29621,-14010,29621,-14010,29621,
-13524,29846,-13524,29846,-13524,29846,-13524,29846,
-13034,30063,-13034,30063,-13034,30063,-13034,30063,
-12540,30272,-12540,30272,-12540,30272,-12540,30272,
-12043,30473,-12043,30473,-12043,30473,-12043,30473,
-11543,30666,-11543,30666,-11543,30666,-11543,30666,
-11039,30851,-11039,30851,-11039,30851,-11039,30851,
-10533,31028,-10533,31028,-10533,31028,-10533,31028,
-10024,31196,-10024,31196,-10024,31196,-10024,31196,
-9512,31356,-9512,31356,-9512,31356,-9512,31356,
-8998,31507,-8998,31507,-8998,31507,-8998,31507,
-8481,31650,-8481,31650,-8481,31650,-8481,31650,
-7962,31785,-7962,31785,-7962,31785,-7962,31785,
-7441,31911,-7441,31911,-7441,31911,-7441,31911,
-6918,32028,-6918,32028,-6918,32028,-6918,32028,
-6393,32137,-6393,32137,-6393,32137,-6393,32137,
-5866,32237,-5866,32237,-5866,32237,-5866,32237,
-5338,32329,-5338,32329,-5338,32329,-5338,32329,
-4808,32412,-4808,32412,-4808,32412,-4808,32412,
-4277,32486,-4277,32486,-4277,32486,-4277,32486,
-3745,32552,-3745,32552,-3745,32552,-3745,32552,
-3212,32609,-3212,32609,-3212,32609,-3212,32609,
-2678,32657,-2678,32657,-2678,32657,-2678,32657,
-2144,32696,-2144,32696,-2144,32696,-2144,32696,
-1608,32727,-1608,32727,-1608,32727,-1608,32727,
-1073,32749,-1073,32749,-1073,32749,-1073,32749,
-537,32762,-537,32762,-537,32762,-537,32762};

void dft1152(int16_t *x,int16_t *y,unsigned char scale_flag){ // 288 x 4

  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa1152[0];
  __m128i *twb128=(__m128i *)&twb1152[0];
  __m128i *twc128=(__m128i *)&twc1152[0];
  __m128i x2128[1152];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[1152];//=&ytmp128array2[0];



  for (i=0,j=0;i<288;i++,j+=4) {
    x2128[i]    = x128[j];
    x2128[i+288] = x128[j+1];
    x2128[i+576] = x128[j+2];
    x2128[i+864] = x128[j+3];
  }

  dft288((int16_t *)x2128,(int16_t *)ytmp128,1);
  dft288((int16_t *)(x2128+288),(int16_t *)(ytmp128+288),1);
  dft288((int16_t *)(x2128+576),(int16_t *)(ytmp128+576),1);
  dft288((int16_t *)(x2128+864),(int16_t *)(ytmp128+864),1);

  bfly4_tw1(ytmp128,ytmp128+288,ytmp128+576,ytmp128+864,y128,y128+288,y128+576,y128+864);
  for (i=1,j=0;i<288;i++,j++) {
    bfly4(ytmp128+i,
	  ytmp128+288+i,
	  ytmp128+576+i,
	  ytmp128+864+i,
	  y128+i,
	  y128+288+i,
	  y128+576+i,
	  y128+864+i,
	  twa128+j,
	  twb128+j,
	  twc128+j);
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(16384);//dft_norm_table[13]);
    
    for (i=0;i<1152;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();
};

int16_t twa1200[4784] = { 32766,-172,32766,-172,32766,-172,32766,-172,
32765,-344,32765,-344,32765,-344,32765,-344,
32762,-515,32762,-515,32762,-515,32762,-515,
32759,-687,32759,-687,32759,-687,32759,-687,
32755,-858,32755,-858,32755,-858,32755,-858,
32750,-1030,32750,-1030,32750,-1030,32750,-1030,
32744,-1201,32744,-1201,32744,-1201,32744,-1201,
32738,-1373,32738,-1373,32738,-1373,32738,-1373,
32730,-1544,32730,-1544,32730,-1544,32730,-1544,
32722,-1715,32722,-1715,32722,-1715,32722,-1715,
32712,-1887,32712,-1887,32712,-1887,32712,-1887,
32702,-2058,32702,-2058,32702,-2058,32702,-2058,
32691,-2229,32691,-2229,32691,-2229,32691,-2229,
32679,-2400,32679,-2400,32679,-2400,32679,-2400,
32665,-2571,32665,-2571,32665,-2571,32665,-2571,
32652,-2742,32652,-2742,32652,-2742,32652,-2742,
32637,-2913,32637,-2913,32637,-2913,32637,-2913,
32621,-3084,32621,-3084,32621,-3084,32621,-3084,
32604,-3255,32604,-3255,32604,-3255,32604,-3255,
32587,-3426,32587,-3426,32587,-3426,32587,-3426,
32569,-3596,32569,-3596,32569,-3596,32569,-3596,
32549,-3767,32549,-3767,32549,-3767,32549,-3767,
32529,-3937,32529,-3937,32529,-3937,32529,-3937,
32508,-4107,32508,-4107,32508,-4107,32508,-4107,
32486,-4277,32486,-4277,32486,-4277,32486,-4277,
32463,-4447,32463,-4447,32463,-4447,32463,-4447,
32440,-4617,32440,-4617,32440,-4617,32440,-4617,
32415,-4787,32415,-4787,32415,-4787,32415,-4787,
32389,-4957,32389,-4957,32389,-4957,32389,-4957,
32363,-5126,32363,-5126,32363,-5126,32363,-5126,
32336,-5296,32336,-5296,32336,-5296,32336,-5296,
32308,-5465,32308,-5465,32308,-5465,32308,-5465,
32279,-5634,32279,-5634,32279,-5634,32279,-5634,
32249,-5803,32249,-5803,32249,-5803,32249,-5803,
32218,-5972,32218,-5972,32218,-5972,32218,-5972,
32186,-6140,32186,-6140,32186,-6140,32186,-6140,
32154,-6309,32154,-6309,32154,-6309,32154,-6309,
32120,-6477,32120,-6477,32120,-6477,32120,-6477,
32086,-6645,32086,-6645,32086,-6645,32086,-6645,
32050,-6813,32050,-6813,32050,-6813,32050,-6813,
32014,-6981,32014,-6981,32014,-6981,32014,-6981,
31977,-7148,31977,-7148,31977,-7148,31977,-7148,
31940,-7316,31940,-7316,31940,-7316,31940,-7316,
31901,-7483,31901,-7483,31901,-7483,31901,-7483,
31861,-7650,31861,-7650,31861,-7650,31861,-7650,
31821,-7817,31821,-7817,31821,-7817,31821,-7817,
31779,-7983,31779,-7983,31779,-7983,31779,-7983,
31737,-8149,31737,-8149,31737,-8149,31737,-8149,
31694,-8315,31694,-8315,31694,-8315,31694,-8315,
31650,-8481,31650,-8481,31650,-8481,31650,-8481,
31605,-8647,31605,-8647,31605,-8647,31605,-8647,
31559,-8812,31559,-8812,31559,-8812,31559,-8812,
31513,-8977,31513,-8977,31513,-8977,31513,-8977,
31465,-9142,31465,-9142,31465,-9142,31465,-9142,
31417,-9307,31417,-9307,31417,-9307,31417,-9307,
31368,-9471,31368,-9471,31368,-9471,31368,-9471,
31318,-9635,31318,-9635,31318,-9635,31318,-9635,
31267,-9799,31267,-9799,31267,-9799,31267,-9799,
31215,-9963,31215,-9963,31215,-9963,31215,-9963,
31163,-10126,31163,-10126,31163,-10126,31163,-10126,
31109,-10289,31109,-10289,31109,-10289,31109,-10289,
31055,-10452,31055,-10452,31055,-10452,31055,-10452,
31000,-10614,31000,-10614,31000,-10614,31000,-10614,
30944,-10776,30944,-10776,30944,-10776,30944,-10776,
30887,-10938,30887,-10938,30887,-10938,30887,-10938,
30829,-11100,30829,-11100,30829,-11100,30829,-11100,
30771,-11261,30771,-11261,30771,-11261,30771,-11261,
30711,-11422,30711,-11422,30711,-11422,30711,-11422,
30651,-11583,30651,-11583,30651,-11583,30651,-11583,
30590,-11743,30590,-11743,30590,-11743,30590,-11743,
30528,-11903,30528,-11903,30528,-11903,30528,-11903,
30465,-12063,30465,-12063,30465,-12063,30465,-12063,
30402,-12222,30402,-12222,30402,-12222,30402,-12222,
30338,-12381,30338,-12381,30338,-12381,30338,-12381,
30272,-12540,30272,-12540,30272,-12540,30272,-12540,
30206,-12698,30206,-12698,30206,-12698,30206,-12698,
30139,-12856,30139,-12856,30139,-12856,30139,-12856,
30072,-13014,30072,-13014,30072,-13014,30072,-13014,
30003,-13171,30003,-13171,30003,-13171,30003,-13171,
29934,-13328,29934,-13328,29934,-13328,29934,-13328,
29863,-13485,29863,-13485,29863,-13485,29863,-13485,
29792,-13641,29792,-13641,29792,-13641,29792,-13641,
29721,-13797,29721,-13797,29721,-13797,29721,-13797,
29648,-13952,29648,-13952,29648,-13952,29648,-13952,
29575,-14107,29575,-14107,29575,-14107,29575,-14107,
29500,-14262,29500,-14262,29500,-14262,29500,-14262,
29425,-14416,29425,-14416,29425,-14416,29425,-14416,
29349,-14570,29349,-14570,29349,-14570,29349,-14570,
29273,-14723,29273,-14723,29273,-14723,29273,-14723,
29195,-14876,29195,-14876,29195,-14876,29195,-14876,
29117,-15029,29117,-15029,29117,-15029,29117,-15029,
29038,-15181,29038,-15181,29038,-15181,29038,-15181,
28958,-15333,28958,-15333,28958,-15333,28958,-15333,
28877,-15485,28877,-15485,28877,-15485,28877,-15485,
28796,-15636,28796,-15636,28796,-15636,28796,-15636,
28713,-15786,28713,-15786,28713,-15786,28713,-15786,
28630,-15936,28630,-15936,28630,-15936,28630,-15936,
28547,-16086,28547,-16086,28547,-16086,28547,-16086,
28462,-16235,28462,-16235,28462,-16235,28462,-16235,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
28290,-16532,28290,-16532,28290,-16532,28290,-16532,
28203,-16680,28203,-16680,28203,-16680,28203,-16680,
28116,-16828,28116,-16828,28116,-16828,28116,-16828,
28027,-16975,28027,-16975,28027,-16975,28027,-16975,
27938,-17121,27938,-17121,27938,-17121,27938,-17121,
27848,-17267,27848,-17267,27848,-17267,27848,-17267,
27757,-17413,27757,-17413,27757,-17413,27757,-17413,
27666,-17558,27666,-17558,27666,-17558,27666,-17558,
27573,-17703,27573,-17703,27573,-17703,27573,-17703,
27480,-17847,27480,-17847,27480,-17847,27480,-17847,
27386,-17990,27386,-17990,27386,-17990,27386,-17990,
27292,-18133,27292,-18133,27292,-18133,27292,-18133,
27197,-18276,27197,-18276,27197,-18276,27197,-18276,
27100,-18418,27100,-18418,27100,-18418,27100,-18418,
27004,-18560,27004,-18560,27004,-18560,27004,-18560,
26906,-18701,26906,-18701,26906,-18701,26906,-18701,
26808,-18842,26808,-18842,26808,-18842,26808,-18842,
26709,-18982,26709,-18982,26709,-18982,26709,-18982,
26609,-19121,26609,-19121,26609,-19121,26609,-19121,
26509,-19260,26509,-19260,26509,-19260,26509,-19260,
26407,-19399,26407,-19399,26407,-19399,26407,-19399,
26305,-19537,26305,-19537,26305,-19537,26305,-19537,
26203,-19674,26203,-19674,26203,-19674,26203,-19674,
26099,-19811,26099,-19811,26099,-19811,26099,-19811,
25995,-19948,25995,-19948,25995,-19948,25995,-19948,
25891,-20084,25891,-20084,25891,-20084,25891,-20084,
25785,-20219,25785,-20219,25785,-20219,25785,-20219,
25679,-20354,25679,-20354,25679,-20354,25679,-20354,
25572,-20488,25572,-20488,25572,-20488,25572,-20488,
25464,-20621,25464,-20621,25464,-20621,25464,-20621,
25356,-20754,25356,-20754,25356,-20754,25356,-20754,
25247,-20887,25247,-20887,25247,-20887,25247,-20887,
25137,-21019,25137,-21019,25137,-21019,25137,-21019,
25027,-21150,25027,-21150,25027,-21150,25027,-21150,
24916,-21281,24916,-21281,24916,-21281,24916,-21281,
24804,-21411,24804,-21411,24804,-21411,24804,-21411,
24692,-21541,24692,-21541,24692,-21541,24692,-21541,
24578,-21670,24578,-21670,24578,-21670,24578,-21670,
24465,-21798,24465,-21798,24465,-21798,24465,-21798,
24350,-21926,24350,-21926,24350,-21926,24350,-21926,
24235,-22053,24235,-22053,24235,-22053,24235,-22053,
24119,-22180,24119,-22180,24119,-22180,24119,-22180,
24003,-22306,24003,-22306,24003,-22306,24003,-22306,
23886,-22431,23886,-22431,23886,-22431,23886,-22431,
23768,-22556,23768,-22556,23768,-22556,23768,-22556,
23649,-22680,23649,-22680,23649,-22680,23649,-22680,
23530,-22803,23530,-22803,23530,-22803,23530,-22803,
23411,-22926,23411,-22926,23411,-22926,23411,-22926,
23290,-23049,23290,-23049,23290,-23049,23290,-23049,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
23048,-23291,23048,-23291,23048,-23291,23048,-23291,
22925,-23412,22925,-23412,22925,-23412,22925,-23412,
22802,-23531,22802,-23531,22802,-23531,22802,-23531,
22679,-23650,22679,-23650,22679,-23650,22679,-23650,
22555,-23769,22555,-23769,22555,-23769,22555,-23769,
22430,-23887,22430,-23887,22430,-23887,22430,-23887,
22305,-24004,22305,-24004,22305,-24004,22305,-24004,
22179,-24120,22179,-24120,22179,-24120,22179,-24120,
22052,-24236,22052,-24236,22052,-24236,22052,-24236,
21925,-24351,21925,-24351,21925,-24351,21925,-24351,
21797,-24466,21797,-24466,21797,-24466,21797,-24466,
21669,-24579,21669,-24579,21669,-24579,21669,-24579,
21540,-24693,21540,-24693,21540,-24693,21540,-24693,
21410,-24805,21410,-24805,21410,-24805,21410,-24805,
21280,-24917,21280,-24917,21280,-24917,21280,-24917,
21149,-25028,21149,-25028,21149,-25028,21149,-25028,
21018,-25138,21018,-25138,21018,-25138,21018,-25138,
20886,-25248,20886,-25248,20886,-25248,20886,-25248,
20753,-25357,20753,-25357,20753,-25357,20753,-25357,
20620,-25465,20620,-25465,20620,-25465,20620,-25465,
20487,-25573,20487,-25573,20487,-25573,20487,-25573,
20353,-25680,20353,-25680,20353,-25680,20353,-25680,
20218,-25786,20218,-25786,20218,-25786,20218,-25786,
20083,-25892,20083,-25892,20083,-25892,20083,-25892,
19947,-25996,19947,-25996,19947,-25996,19947,-25996,
19810,-26100,19810,-26100,19810,-26100,19810,-26100,
19673,-26204,19673,-26204,19673,-26204,19673,-26204,
19536,-26306,19536,-26306,19536,-26306,19536,-26306,
19398,-26408,19398,-26408,19398,-26408,19398,-26408,
19259,-26510,19259,-26510,19259,-26510,19259,-26510,
19120,-26610,19120,-26610,19120,-26610,19120,-26610,
18981,-26710,18981,-26710,18981,-26710,18981,-26710,
18841,-26809,18841,-26809,18841,-26809,18841,-26809,
18700,-26907,18700,-26907,18700,-26907,18700,-26907,
18559,-27005,18559,-27005,18559,-27005,18559,-27005,
18417,-27101,18417,-27101,18417,-27101,18417,-27101,
18275,-27198,18275,-27198,18275,-27198,18275,-27198,
18132,-27293,18132,-27293,18132,-27293,18132,-27293,
17989,-27387,17989,-27387,17989,-27387,17989,-27387,
17846,-27481,17846,-27481,17846,-27481,17846,-27481,
17702,-27574,17702,-27574,17702,-27574,17702,-27574,
17557,-27667,17557,-27667,17557,-27667,17557,-27667,
17412,-27758,17412,-27758,17412,-27758,17412,-27758,
17266,-27849,17266,-27849,17266,-27849,17266,-27849,
17120,-27939,17120,-27939,17120,-27939,17120,-27939,
16974,-28028,16974,-28028,16974,-28028,16974,-28028,
16827,-28117,16827,-28117,16827,-28117,16827,-28117,
16679,-28204,16679,-28204,16679,-28204,16679,-28204,
16531,-28291,16531,-28291,16531,-28291,16531,-28291,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
16234,-28463,16234,-28463,16234,-28463,16234,-28463,
16085,-28548,16085,-28548,16085,-28548,16085,-28548,
15935,-28631,15935,-28631,15935,-28631,15935,-28631,
15785,-28714,15785,-28714,15785,-28714,15785,-28714,
15635,-28797,15635,-28797,15635,-28797,15635,-28797,
15484,-28878,15484,-28878,15484,-28878,15484,-28878,
15332,-28959,15332,-28959,15332,-28959,15332,-28959,
15180,-29039,15180,-29039,15180,-29039,15180,-29039,
15028,-29118,15028,-29118,15028,-29118,15028,-29118,
14875,-29196,14875,-29196,14875,-29196,14875,-29196,
14722,-29274,14722,-29274,14722,-29274,14722,-29274,
14569,-29350,14569,-29350,14569,-29350,14569,-29350,
14415,-29426,14415,-29426,14415,-29426,14415,-29426,
14261,-29501,14261,-29501,14261,-29501,14261,-29501,
14106,-29576,14106,-29576,14106,-29576,14106,-29576,
13951,-29649,13951,-29649,13951,-29649,13951,-29649,
13796,-29722,13796,-29722,13796,-29722,13796,-29722,
13640,-29793,13640,-29793,13640,-29793,13640,-29793,
13484,-29864,13484,-29864,13484,-29864,13484,-29864,
13327,-29935,13327,-29935,13327,-29935,13327,-29935,
13170,-30004,13170,-30004,13170,-30004,13170,-30004,
13013,-30073,13013,-30073,13013,-30073,13013,-30073,
12855,-30140,12855,-30140,12855,-30140,12855,-30140,
12697,-30207,12697,-30207,12697,-30207,12697,-30207,
12539,-30273,12539,-30273,12539,-30273,12539,-30273,
12380,-30339,12380,-30339,12380,-30339,12380,-30339,
12221,-30403,12221,-30403,12221,-30403,12221,-30403,
12062,-30466,12062,-30466,12062,-30466,12062,-30466,
11902,-30529,11902,-30529,11902,-30529,11902,-30529,
11742,-30591,11742,-30591,11742,-30591,11742,-30591,
11582,-30652,11582,-30652,11582,-30652,11582,-30652,
11421,-30712,11421,-30712,11421,-30712,11421,-30712,
11260,-30772,11260,-30772,11260,-30772,11260,-30772,
11099,-30830,11099,-30830,11099,-30830,11099,-30830,
10937,-30888,10937,-30888,10937,-30888,10937,-30888,
10775,-30945,10775,-30945,10775,-30945,10775,-30945,
10613,-31001,10613,-31001,10613,-31001,10613,-31001,
10451,-31056,10451,-31056,10451,-31056,10451,-31056,
10288,-31110,10288,-31110,10288,-31110,10288,-31110,
10125,-31164,10125,-31164,10125,-31164,10125,-31164,
9962,-31216,9962,-31216,9962,-31216,9962,-31216,
9798,-31268,9798,-31268,9798,-31268,9798,-31268,
9634,-31319,9634,-31319,9634,-31319,9634,-31319,
9470,-31369,9470,-31369,9470,-31369,9470,-31369,
9306,-31418,9306,-31418,9306,-31418,9306,-31418,
9141,-31466,9141,-31466,9141,-31466,9141,-31466,
8976,-31514,8976,-31514,8976,-31514,8976,-31514,
8811,-31560,8811,-31560,8811,-31560,8811,-31560,
8646,-31606,8646,-31606,8646,-31606,8646,-31606,
8480,-31651,8480,-31651,8480,-31651,8480,-31651,
8314,-31695,8314,-31695,8314,-31695,8314,-31695,
8148,-31738,8148,-31738,8148,-31738,8148,-31738,
7982,-31780,7982,-31780,7982,-31780,7982,-31780,
7816,-31822,7816,-31822,7816,-31822,7816,-31822,
7649,-31862,7649,-31862,7649,-31862,7649,-31862,
7482,-31902,7482,-31902,7482,-31902,7482,-31902,
7315,-31941,7315,-31941,7315,-31941,7315,-31941,
7147,-31978,7147,-31978,7147,-31978,7147,-31978,
6980,-32015,6980,-32015,6980,-32015,6980,-32015,
6812,-32051,6812,-32051,6812,-32051,6812,-32051,
6644,-32087,6644,-32087,6644,-32087,6644,-32087,
6476,-32121,6476,-32121,6476,-32121,6476,-32121,
6308,-32155,6308,-32155,6308,-32155,6308,-32155,
6139,-32187,6139,-32187,6139,-32187,6139,-32187,
5971,-32219,5971,-32219,5971,-32219,5971,-32219,
5802,-32250,5802,-32250,5802,-32250,5802,-32250,
5633,-32280,5633,-32280,5633,-32280,5633,-32280,
5464,-32309,5464,-32309,5464,-32309,5464,-32309,
5295,-32337,5295,-32337,5295,-32337,5295,-32337,
5125,-32364,5125,-32364,5125,-32364,5125,-32364,
4956,-32390,4956,-32390,4956,-32390,4956,-32390,
4786,-32416,4786,-32416,4786,-32416,4786,-32416,
4616,-32441,4616,-32441,4616,-32441,4616,-32441,
4446,-32464,4446,-32464,4446,-32464,4446,-32464,
4276,-32487,4276,-32487,4276,-32487,4276,-32487,
4106,-32509,4106,-32509,4106,-32509,4106,-32509,
3936,-32530,3936,-32530,3936,-32530,3936,-32530,
3766,-32550,3766,-32550,3766,-32550,3766,-32550,
3595,-32570,3595,-32570,3595,-32570,3595,-32570,
3425,-32588,3425,-32588,3425,-32588,3425,-32588,
3254,-32605,3254,-32605,3254,-32605,3254,-32605,
3083,-32622,3083,-32622,3083,-32622,3083,-32622,
2912,-32638,2912,-32638,2912,-32638,2912,-32638,
2741,-32653,2741,-32653,2741,-32653,2741,-32653,
2570,-32666,2570,-32666,2570,-32666,2570,-32666,
2399,-32680,2399,-32680,2399,-32680,2399,-32680,
2228,-32692,2228,-32692,2228,-32692,2228,-32692,
2057,-32703,2057,-32703,2057,-32703,2057,-32703,
1886,-32713,1886,-32713,1886,-32713,1886,-32713,
1714,-32723,1714,-32723,1714,-32723,1714,-32723,
1543,-32731,1543,-32731,1543,-32731,1543,-32731,
1372,-32739,1372,-32739,1372,-32739,1372,-32739,
1200,-32745,1200,-32745,1200,-32745,1200,-32745,
1029,-32751,1029,-32751,1029,-32751,1029,-32751,
857,-32756,857,-32756,857,-32756,857,-32756,
686,-32760,686,-32760,686,-32760,686,-32760,
514,-32763,514,-32763,514,-32763,514,-32763,
343,-32766,343,-32766,343,-32766,343,-32766,
171,-32767,171,-32767,171,-32767,171,-32767};

int16_t twb1200[4784] = {32765,-344,32765,-344,32765,-344,32765,-344,
32759,-687,32759,-687,32759,-687,32759,-687,
32750,-1030,32750,-1030,32750,-1030,32750,-1030,
32738,-1373,32738,-1373,32738,-1373,32738,-1373,
32722,-1715,32722,-1715,32722,-1715,32722,-1715,
32702,-2058,32702,-2058,32702,-2058,32702,-2058,
32679,-2400,32679,-2400,32679,-2400,32679,-2400,
32652,-2742,32652,-2742,32652,-2742,32652,-2742,
32621,-3084,32621,-3084,32621,-3084,32621,-3084,
32587,-3426,32587,-3426,32587,-3426,32587,-3426,
32549,-3767,32549,-3767,32549,-3767,32549,-3767,
32508,-4107,32508,-4107,32508,-4107,32508,-4107,
32463,-4447,32463,-4447,32463,-4447,32463,-4447,
32415,-4787,32415,-4787,32415,-4787,32415,-4787,
32363,-5126,32363,-5126,32363,-5126,32363,-5126,
32308,-5465,32308,-5465,32308,-5465,32308,-5465,
32249,-5803,32249,-5803,32249,-5803,32249,-5803,
32186,-6140,32186,-6140,32186,-6140,32186,-6140,
32120,-6477,32120,-6477,32120,-6477,32120,-6477,
32050,-6813,32050,-6813,32050,-6813,32050,-6813,
31977,-7148,31977,-7148,31977,-7148,31977,-7148,
31901,-7483,31901,-7483,31901,-7483,31901,-7483,
31821,-7817,31821,-7817,31821,-7817,31821,-7817,
31737,-8149,31737,-8149,31737,-8149,31737,-8149,
31650,-8481,31650,-8481,31650,-8481,31650,-8481,
31559,-8812,31559,-8812,31559,-8812,31559,-8812,
31465,-9142,31465,-9142,31465,-9142,31465,-9142,
31368,-9471,31368,-9471,31368,-9471,31368,-9471,
31267,-9799,31267,-9799,31267,-9799,31267,-9799,
31163,-10126,31163,-10126,31163,-10126,31163,-10126,
31055,-10452,31055,-10452,31055,-10452,31055,-10452,
30944,-10776,30944,-10776,30944,-10776,30944,-10776,
30829,-11100,30829,-11100,30829,-11100,30829,-11100,
30711,-11422,30711,-11422,30711,-11422,30711,-11422,
30590,-11743,30590,-11743,30590,-11743,30590,-11743,
30465,-12063,30465,-12063,30465,-12063,30465,-12063,
30338,-12381,30338,-12381,30338,-12381,30338,-12381,
30206,-12698,30206,-12698,30206,-12698,30206,-12698,
30072,-13014,30072,-13014,30072,-13014,30072,-13014,
29934,-13328,29934,-13328,29934,-13328,29934,-13328,
29792,-13641,29792,-13641,29792,-13641,29792,-13641,
29648,-13952,29648,-13952,29648,-13952,29648,-13952,
29500,-14262,29500,-14262,29500,-14262,29500,-14262,
29349,-14570,29349,-14570,29349,-14570,29349,-14570,
29195,-14876,29195,-14876,29195,-14876,29195,-14876,
29038,-15181,29038,-15181,29038,-15181,29038,-15181,
28877,-15485,28877,-15485,28877,-15485,28877,-15485,
28713,-15786,28713,-15786,28713,-15786,28713,-15786,
28547,-16086,28547,-16086,28547,-16086,28547,-16086,
28377,-16384,28377,-16384,28377,-16384,28377,-16384,
28203,-16680,28203,-16680,28203,-16680,28203,-16680,
28027,-16975,28027,-16975,28027,-16975,28027,-16975,
27848,-17267,27848,-17267,27848,-17267,27848,-17267,
27666,-17558,27666,-17558,27666,-17558,27666,-17558,
27480,-17847,27480,-17847,27480,-17847,27480,-17847,
27292,-18133,27292,-18133,27292,-18133,27292,-18133,
27100,-18418,27100,-18418,27100,-18418,27100,-18418,
26906,-18701,26906,-18701,26906,-18701,26906,-18701,
26709,-18982,26709,-18982,26709,-18982,26709,-18982,
26509,-19260,26509,-19260,26509,-19260,26509,-19260,
26305,-19537,26305,-19537,26305,-19537,26305,-19537,
26099,-19811,26099,-19811,26099,-19811,26099,-19811,
25891,-20084,25891,-20084,25891,-20084,25891,-20084,
25679,-20354,25679,-20354,25679,-20354,25679,-20354,
25464,-20621,25464,-20621,25464,-20621,25464,-20621,
25247,-20887,25247,-20887,25247,-20887,25247,-20887,
25027,-21150,25027,-21150,25027,-21150,25027,-21150,
24804,-21411,24804,-21411,24804,-21411,24804,-21411,
24578,-21670,24578,-21670,24578,-21670,24578,-21670,
24350,-21926,24350,-21926,24350,-21926,24350,-21926,
24119,-22180,24119,-22180,24119,-22180,24119,-22180,
23886,-22431,23886,-22431,23886,-22431,23886,-22431,
23649,-22680,23649,-22680,23649,-22680,23649,-22680,
23411,-22926,23411,-22926,23411,-22926,23411,-22926,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
22925,-23412,22925,-23412,22925,-23412,22925,-23412,
22679,-23650,22679,-23650,22679,-23650,22679,-23650,
22430,-23887,22430,-23887,22430,-23887,22430,-23887,
22179,-24120,22179,-24120,22179,-24120,22179,-24120,
21925,-24351,21925,-24351,21925,-24351,21925,-24351,
21669,-24579,21669,-24579,21669,-24579,21669,-24579,
21410,-24805,21410,-24805,21410,-24805,21410,-24805,
21149,-25028,21149,-25028,21149,-25028,21149,-25028,
20886,-25248,20886,-25248,20886,-25248,20886,-25248,
20620,-25465,20620,-25465,20620,-25465,20620,-25465,
20353,-25680,20353,-25680,20353,-25680,20353,-25680,
20083,-25892,20083,-25892,20083,-25892,20083,-25892,
19810,-26100,19810,-26100,19810,-26100,19810,-26100,
19536,-26306,19536,-26306,19536,-26306,19536,-26306,
19259,-26510,19259,-26510,19259,-26510,19259,-26510,
18981,-26710,18981,-26710,18981,-26710,18981,-26710,
18700,-26907,18700,-26907,18700,-26907,18700,-26907,
18417,-27101,18417,-27101,18417,-27101,18417,-27101,
18132,-27293,18132,-27293,18132,-27293,18132,-27293,
17846,-27481,17846,-27481,17846,-27481,17846,-27481,
17557,-27667,17557,-27667,17557,-27667,17557,-27667,
17266,-27849,17266,-27849,17266,-27849,17266,-27849,
16974,-28028,16974,-28028,16974,-28028,16974,-28028,
16679,-28204,16679,-28204,16679,-28204,16679,-28204,
16383,-28378,16383,-28378,16383,-28378,16383,-28378,
16085,-28548,16085,-28548,16085,-28548,16085,-28548,
15785,-28714,15785,-28714,15785,-28714,15785,-28714,
15484,-28878,15484,-28878,15484,-28878,15484,-28878,
15180,-29039,15180,-29039,15180,-29039,15180,-29039,
14875,-29196,14875,-29196,14875,-29196,14875,-29196,
14569,-29350,14569,-29350,14569,-29350,14569,-29350,
14261,-29501,14261,-29501,14261,-29501,14261,-29501,
13951,-29649,13951,-29649,13951,-29649,13951,-29649,
13640,-29793,13640,-29793,13640,-29793,13640,-29793,
13327,-29935,13327,-29935,13327,-29935,13327,-29935,
13013,-30073,13013,-30073,13013,-30073,13013,-30073,
12697,-30207,12697,-30207,12697,-30207,12697,-30207,
12380,-30339,12380,-30339,12380,-30339,12380,-30339,
12062,-30466,12062,-30466,12062,-30466,12062,-30466,
11742,-30591,11742,-30591,11742,-30591,11742,-30591,
11421,-30712,11421,-30712,11421,-30712,11421,-30712,
11099,-30830,11099,-30830,11099,-30830,11099,-30830,
10775,-30945,10775,-30945,10775,-30945,10775,-30945,
10451,-31056,10451,-31056,10451,-31056,10451,-31056,
10125,-31164,10125,-31164,10125,-31164,10125,-31164,
9798,-31268,9798,-31268,9798,-31268,9798,-31268,
9470,-31369,9470,-31369,9470,-31369,9470,-31369,
9141,-31466,9141,-31466,9141,-31466,9141,-31466,
8811,-31560,8811,-31560,8811,-31560,8811,-31560,
8480,-31651,8480,-31651,8480,-31651,8480,-31651,
8148,-31738,8148,-31738,8148,-31738,8148,-31738,
7816,-31822,7816,-31822,7816,-31822,7816,-31822,
7482,-31902,7482,-31902,7482,-31902,7482,-31902,
7147,-31978,7147,-31978,7147,-31978,7147,-31978,
6812,-32051,6812,-32051,6812,-32051,6812,-32051,
6476,-32121,6476,-32121,6476,-32121,6476,-32121,
6139,-32187,6139,-32187,6139,-32187,6139,-32187,
5802,-32250,5802,-32250,5802,-32250,5802,-32250,
5464,-32309,5464,-32309,5464,-32309,5464,-32309,
5125,-32364,5125,-32364,5125,-32364,5125,-32364,
4786,-32416,4786,-32416,4786,-32416,4786,-32416,
4446,-32464,4446,-32464,4446,-32464,4446,-32464,
4106,-32509,4106,-32509,4106,-32509,4106,-32509,
3766,-32550,3766,-32550,3766,-32550,3766,-32550,
3425,-32588,3425,-32588,3425,-32588,3425,-32588,
3083,-32622,3083,-32622,3083,-32622,3083,-32622,
2741,-32653,2741,-32653,2741,-32653,2741,-32653,
2399,-32680,2399,-32680,2399,-32680,2399,-32680,
2057,-32703,2057,-32703,2057,-32703,2057,-32703,
1714,-32723,1714,-32723,1714,-32723,1714,-32723,
1372,-32739,1372,-32739,1372,-32739,1372,-32739,
1029,-32751,1029,-32751,1029,-32751,1029,-32751,
686,-32760,686,-32760,686,-32760,686,-32760,
343,-32766,343,-32766,343,-32766,343,-32766,
0,-32767,0,-32767,0,-32767,0,-32767,
-344,-32766,-344,-32766,-344,-32766,-344,-32766,
-687,-32760,-687,-32760,-687,-32760,-687,-32760,
-1030,-32751,-1030,-32751,-1030,-32751,-1030,-32751,
-1373,-32739,-1373,-32739,-1373,-32739,-1373,-32739,
-1715,-32723,-1715,-32723,-1715,-32723,-1715,-32723,
-2058,-32703,-2058,-32703,-2058,-32703,-2058,-32703,
-2400,-32680,-2400,-32680,-2400,-32680,-2400,-32680,
-2742,-32653,-2742,-32653,-2742,-32653,-2742,-32653,
-3084,-32622,-3084,-32622,-3084,-32622,-3084,-32622,
-3426,-32588,-3426,-32588,-3426,-32588,-3426,-32588,
-3767,-32550,-3767,-32550,-3767,-32550,-3767,-32550,
-4107,-32509,-4107,-32509,-4107,-32509,-4107,-32509,
-4447,-32464,-4447,-32464,-4447,-32464,-4447,-32464,
-4787,-32416,-4787,-32416,-4787,-32416,-4787,-32416,
-5126,-32364,-5126,-32364,-5126,-32364,-5126,-32364,
-5465,-32309,-5465,-32309,-5465,-32309,-5465,-32309,
-5803,-32250,-5803,-32250,-5803,-32250,-5803,-32250,
-6140,-32187,-6140,-32187,-6140,-32187,-6140,-32187,
-6477,-32121,-6477,-32121,-6477,-32121,-6477,-32121,
-6813,-32051,-6813,-32051,-6813,-32051,-6813,-32051,
-7148,-31978,-7148,-31978,-7148,-31978,-7148,-31978,
-7483,-31902,-7483,-31902,-7483,-31902,-7483,-31902,
-7817,-31822,-7817,-31822,-7817,-31822,-7817,-31822,
-8149,-31738,-8149,-31738,-8149,-31738,-8149,-31738,
-8481,-31651,-8481,-31651,-8481,-31651,-8481,-31651,
-8812,-31560,-8812,-31560,-8812,-31560,-8812,-31560,
-9142,-31466,-9142,-31466,-9142,-31466,-9142,-31466,
-9471,-31369,-9471,-31369,-9471,-31369,-9471,-31369,
-9799,-31268,-9799,-31268,-9799,-31268,-9799,-31268,
-10126,-31164,-10126,-31164,-10126,-31164,-10126,-31164,
-10452,-31056,-10452,-31056,-10452,-31056,-10452,-31056,
-10776,-30945,-10776,-30945,-10776,-30945,-10776,-30945,
-11100,-30830,-11100,-30830,-11100,-30830,-11100,-30830,
-11422,-30712,-11422,-30712,-11422,-30712,-11422,-30712,
-11743,-30591,-11743,-30591,-11743,-30591,-11743,-30591,
-12063,-30466,-12063,-30466,-12063,-30466,-12063,-30466,
-12381,-30339,-12381,-30339,-12381,-30339,-12381,-30339,
-12698,-30207,-12698,-30207,-12698,-30207,-12698,-30207,
-13014,-30073,-13014,-30073,-13014,-30073,-13014,-30073,
-13328,-29935,-13328,-29935,-13328,-29935,-13328,-29935,
-13641,-29793,-13641,-29793,-13641,-29793,-13641,-29793,
-13952,-29649,-13952,-29649,-13952,-29649,-13952,-29649,
-14262,-29501,-14262,-29501,-14262,-29501,-14262,-29501,
-14570,-29350,-14570,-29350,-14570,-29350,-14570,-29350,
-14876,-29196,-14876,-29196,-14876,-29196,-14876,-29196,
-15181,-29039,-15181,-29039,-15181,-29039,-15181,-29039,
-15485,-28878,-15485,-28878,-15485,-28878,-15485,-28878,
-15786,-28714,-15786,-28714,-15786,-28714,-15786,-28714,
-16086,-28548,-16086,-28548,-16086,-28548,-16086,-28548,
-16384,-28378,-16384,-28378,-16384,-28378,-16384,-28378,
-16680,-28204,-16680,-28204,-16680,-28204,-16680,-28204,
-16975,-28028,-16975,-28028,-16975,-28028,-16975,-28028,
-17267,-27849,-17267,-27849,-17267,-27849,-17267,-27849,
-17558,-27667,-17558,-27667,-17558,-27667,-17558,-27667,
-17847,-27481,-17847,-27481,-17847,-27481,-17847,-27481,
-18133,-27293,-18133,-27293,-18133,-27293,-18133,-27293,
-18418,-27101,-18418,-27101,-18418,-27101,-18418,-27101,
-18701,-26907,-18701,-26907,-18701,-26907,-18701,-26907,
-18982,-26710,-18982,-26710,-18982,-26710,-18982,-26710,
-19260,-26510,-19260,-26510,-19260,-26510,-19260,-26510,
-19537,-26306,-19537,-26306,-19537,-26306,-19537,-26306,
-19811,-26100,-19811,-26100,-19811,-26100,-19811,-26100,
-20084,-25892,-20084,-25892,-20084,-25892,-20084,-25892,
-20354,-25680,-20354,-25680,-20354,-25680,-20354,-25680,
-20621,-25465,-20621,-25465,-20621,-25465,-20621,-25465,
-20887,-25248,-20887,-25248,-20887,-25248,-20887,-25248,
-21150,-25028,-21150,-25028,-21150,-25028,-21150,-25028,
-21411,-24805,-21411,-24805,-21411,-24805,-21411,-24805,
-21670,-24579,-21670,-24579,-21670,-24579,-21670,-24579,
-21926,-24351,-21926,-24351,-21926,-24351,-21926,-24351,
-22180,-24120,-22180,-24120,-22180,-24120,-22180,-24120,
-22431,-23887,-22431,-23887,-22431,-23887,-22431,-23887,
-22680,-23650,-22680,-23650,-22680,-23650,-22680,-23650,
-22926,-23412,-22926,-23412,-22926,-23412,-22926,-23412,
-23170,-23170,-23170,-23170,-23170,-23170,-23170,-23170,
-23412,-22926,-23412,-22926,-23412,-22926,-23412,-22926,
-23650,-22680,-23650,-22680,-23650,-22680,-23650,-22680,
-23887,-22431,-23887,-22431,-23887,-22431,-23887,-22431,
-24120,-22180,-24120,-22180,-24120,-22180,-24120,-22180,
-24351,-21926,-24351,-21926,-24351,-21926,-24351,-21926,
-24579,-21670,-24579,-21670,-24579,-21670,-24579,-21670,
-24805,-21411,-24805,-21411,-24805,-21411,-24805,-21411,
-25028,-21150,-25028,-21150,-25028,-21150,-25028,-21150,
-25248,-20887,-25248,-20887,-25248,-20887,-25248,-20887,
-25465,-20621,-25465,-20621,-25465,-20621,-25465,-20621,
-25680,-20354,-25680,-20354,-25680,-20354,-25680,-20354,
-25892,-20084,-25892,-20084,-25892,-20084,-25892,-20084,
-26100,-19811,-26100,-19811,-26100,-19811,-26100,-19811,
-26306,-19537,-26306,-19537,-26306,-19537,-26306,-19537,
-26510,-19260,-26510,-19260,-26510,-19260,-26510,-19260,
-26710,-18982,-26710,-18982,-26710,-18982,-26710,-18982,
-26907,-18701,-26907,-18701,-26907,-18701,-26907,-18701,
-27101,-18418,-27101,-18418,-27101,-18418,-27101,-18418,
-27293,-18133,-27293,-18133,-27293,-18133,-27293,-18133,
-27481,-17847,-27481,-17847,-27481,-17847,-27481,-17847,
-27667,-17558,-27667,-17558,-27667,-17558,-27667,-17558,
-27849,-17267,-27849,-17267,-27849,-17267,-27849,-17267,
-28028,-16975,-28028,-16975,-28028,-16975,-28028,-16975,
-28204,-16680,-28204,-16680,-28204,-16680,-28204,-16680,
-28378,-16384,-28378,-16384,-28378,-16384,-28378,-16384,
-28548,-16086,-28548,-16086,-28548,-16086,-28548,-16086,
-28714,-15786,-28714,-15786,-28714,-15786,-28714,-15786,
-28878,-15485,-28878,-15485,-28878,-15485,-28878,-15485,
-29039,-15181,-29039,-15181,-29039,-15181,-29039,-15181,
-29196,-14876,-29196,-14876,-29196,-14876,-29196,-14876,
-29350,-14570,-29350,-14570,-29350,-14570,-29350,-14570,
-29501,-14262,-29501,-14262,-29501,-14262,-29501,-14262,
-29649,-13952,-29649,-13952,-29649,-13952,-29649,-13952,
-29793,-13641,-29793,-13641,-29793,-13641,-29793,-13641,
-29935,-13328,-29935,-13328,-29935,-13328,-29935,-13328,
-30073,-13014,-30073,-13014,-30073,-13014,-30073,-13014,
-30207,-12698,-30207,-12698,-30207,-12698,-30207,-12698,
-30339,-12381,-30339,-12381,-30339,-12381,-30339,-12381,
-30466,-12063,-30466,-12063,-30466,-12063,-30466,-12063,
-30591,-11743,-30591,-11743,-30591,-11743,-30591,-11743,
-30712,-11422,-30712,-11422,-30712,-11422,-30712,-11422,
-30830,-11100,-30830,-11100,-30830,-11100,-30830,-11100,
-30945,-10776,-30945,-10776,-30945,-10776,-30945,-10776,
-31056,-10452,-31056,-10452,-31056,-10452,-31056,-10452,
-31164,-10126,-31164,-10126,-31164,-10126,-31164,-10126,
-31268,-9799,-31268,-9799,-31268,-9799,-31268,-9799,
-31369,-9471,-31369,-9471,-31369,-9471,-31369,-9471,
-31466,-9142,-31466,-9142,-31466,-9142,-31466,-9142,
-31560,-8812,-31560,-8812,-31560,-8812,-31560,-8812,
-31651,-8481,-31651,-8481,-31651,-8481,-31651,-8481,
-31738,-8149,-31738,-8149,-31738,-8149,-31738,-8149,
-31822,-7817,-31822,-7817,-31822,-7817,-31822,-7817,
-31902,-7483,-31902,-7483,-31902,-7483,-31902,-7483,
-31978,-7148,-31978,-7148,-31978,-7148,-31978,-7148,
-32051,-6813,-32051,-6813,-32051,-6813,-32051,-6813,
-32121,-6477,-32121,-6477,-32121,-6477,-32121,-6477,
-32187,-6140,-32187,-6140,-32187,-6140,-32187,-6140,
-32250,-5803,-32250,-5803,-32250,-5803,-32250,-5803,
-32309,-5465,-32309,-5465,-32309,-5465,-32309,-5465,
-32364,-5126,-32364,-5126,-32364,-5126,-32364,-5126,
-32416,-4787,-32416,-4787,-32416,-4787,-32416,-4787,
-32464,-4447,-32464,-4447,-32464,-4447,-32464,-4447,
-32509,-4107,-32509,-4107,-32509,-4107,-32509,-4107,
-32550,-3767,-32550,-3767,-32550,-3767,-32550,-3767,
-32588,-3426,-32588,-3426,-32588,-3426,-32588,-3426,
-32622,-3084,-32622,-3084,-32622,-3084,-32622,-3084,
-32653,-2742,-32653,-2742,-32653,-2742,-32653,-2742,
-32680,-2400,-32680,-2400,-32680,-2400,-32680,-2400,
-32703,-2058,-32703,-2058,-32703,-2058,-32703,-2058,
-32723,-1715,-32723,-1715,-32723,-1715,-32723,-1715,
-32739,-1373,-32739,-1373,-32739,-1373,-32739,-1373,
-32751,-1030,-32751,-1030,-32751,-1030,-32751,-1030,
-32760,-687,-32760,-687,-32760,-687,-32760,-687,
-32766,-344,-32766,-344,-32766,-344,-32766,-344};
 
int16_t twc1200[4784] = { 32762,-515,32762,-515,32762,-515,32762,-515,
32750,-1030,32750,-1030,32750,-1030,32750,-1030,
32730,-1544,32730,-1544,32730,-1544,32730,-1544,
32702,-2058,32702,-2058,32702,-2058,32702,-2058,
32665,-2571,32665,-2571,32665,-2571,32665,-2571,
32621,-3084,32621,-3084,32621,-3084,32621,-3084,
32569,-3596,32569,-3596,32569,-3596,32569,-3596,
32508,-4107,32508,-4107,32508,-4107,32508,-4107,
32440,-4617,32440,-4617,32440,-4617,32440,-4617,
32363,-5126,32363,-5126,32363,-5126,32363,-5126,
32279,-5634,32279,-5634,32279,-5634,32279,-5634,
32186,-6140,32186,-6140,32186,-6140,32186,-6140,
32086,-6645,32086,-6645,32086,-6645,32086,-6645,
31977,-7148,31977,-7148,31977,-7148,31977,-7148,
31861,-7650,31861,-7650,31861,-7650,31861,-7650,
31737,-8149,31737,-8149,31737,-8149,31737,-8149,
31605,-8647,31605,-8647,31605,-8647,31605,-8647,
31465,-9142,31465,-9142,31465,-9142,31465,-9142,
31318,-9635,31318,-9635,31318,-9635,31318,-9635,
31163,-10126,31163,-10126,31163,-10126,31163,-10126,
31000,-10614,31000,-10614,31000,-10614,31000,-10614,
30829,-11100,30829,-11100,30829,-11100,30829,-11100,
30651,-11583,30651,-11583,30651,-11583,30651,-11583,
30465,-12063,30465,-12063,30465,-12063,30465,-12063,
30272,-12540,30272,-12540,30272,-12540,30272,-12540,
30072,-13014,30072,-13014,30072,-13014,30072,-13014,
29863,-13485,29863,-13485,29863,-13485,29863,-13485,
29648,-13952,29648,-13952,29648,-13952,29648,-13952,
29425,-14416,29425,-14416,29425,-14416,29425,-14416,
29195,-14876,29195,-14876,29195,-14876,29195,-14876,
28958,-15333,28958,-15333,28958,-15333,28958,-15333,
28713,-15786,28713,-15786,28713,-15786,28713,-15786,
28462,-16235,28462,-16235,28462,-16235,28462,-16235,
28203,-16680,28203,-16680,28203,-16680,28203,-16680,
27938,-17121,27938,-17121,27938,-17121,27938,-17121,
27666,-17558,27666,-17558,27666,-17558,27666,-17558,
27386,-17990,27386,-17990,27386,-17990,27386,-17990,
27100,-18418,27100,-18418,27100,-18418,27100,-18418,
26808,-18842,26808,-18842,26808,-18842,26808,-18842,
26509,-19260,26509,-19260,26509,-19260,26509,-19260,
26203,-19674,26203,-19674,26203,-19674,26203,-19674,
25891,-20084,25891,-20084,25891,-20084,25891,-20084,
25572,-20488,25572,-20488,25572,-20488,25572,-20488,
25247,-20887,25247,-20887,25247,-20887,25247,-20887,
24916,-21281,24916,-21281,24916,-21281,24916,-21281,
24578,-21670,24578,-21670,24578,-21670,24578,-21670,
24235,-22053,24235,-22053,24235,-22053,24235,-22053,
23886,-22431,23886,-22431,23886,-22431,23886,-22431,
23530,-22803,23530,-22803,23530,-22803,23530,-22803,
23169,-23170,23169,-23170,23169,-23170,23169,-23170,
22802,-23531,22802,-23531,22802,-23531,22802,-23531,
22430,-23887,22430,-23887,22430,-23887,22430,-23887,
22052,-24236,22052,-24236,22052,-24236,22052,-24236,
21669,-24579,21669,-24579,21669,-24579,21669,-24579,
21280,-24917,21280,-24917,21280,-24917,21280,-24917,
20886,-25248,20886,-25248,20886,-25248,20886,-25248,
20487,-25573,20487,-25573,20487,-25573,20487,-25573,
20083,-25892,20083,-25892,20083,-25892,20083,-25892,
19673,-26204,19673,-26204,19673,-26204,19673,-26204,
19259,-26510,19259,-26510,19259,-26510,19259,-26510,
18841,-26809,18841,-26809,18841,-26809,18841,-26809,
18417,-27101,18417,-27101,18417,-27101,18417,-27101,
17989,-27387,17989,-27387,17989,-27387,17989,-27387,
17557,-27667,17557,-27667,17557,-27667,17557,-27667,
17120,-27939,17120,-27939,17120,-27939,17120,-27939,
16679,-28204,16679,-28204,16679,-28204,16679,-28204,
16234,-28463,16234,-28463,16234,-28463,16234,-28463,
15785,-28714,15785,-28714,15785,-28714,15785,-28714,
15332,-28959,15332,-28959,15332,-28959,15332,-28959,
14875,-29196,14875,-29196,14875,-29196,14875,-29196,
14415,-29426,14415,-29426,14415,-29426,14415,-29426,
13951,-29649,13951,-29649,13951,-29649,13951,-29649,
13484,-29864,13484,-29864,13484,-29864,13484,-29864,
13013,-30073,13013,-30073,13013,-30073,13013,-30073,
12539,-30273,12539,-30273,12539,-30273,12539,-30273,
12062,-30466,12062,-30466,12062,-30466,12062,-30466,
11582,-30652,11582,-30652,11582,-30652,11582,-30652,
11099,-30830,11099,-30830,11099,-30830,11099,-30830,
10613,-31001,10613,-31001,10613,-31001,10613,-31001,
10125,-31164,10125,-31164,10125,-31164,10125,-31164,
9634,-31319,9634,-31319,9634,-31319,9634,-31319,
9141,-31466,9141,-31466,9141,-31466,9141,-31466,
8646,-31606,8646,-31606,8646,-31606,8646,-31606,
8148,-31738,8148,-31738,8148,-31738,8148,-31738,
7649,-31862,7649,-31862,7649,-31862,7649,-31862,
7147,-31978,7147,-31978,7147,-31978,7147,-31978,
6644,-32087,6644,-32087,6644,-32087,6644,-32087,
6139,-32187,6139,-32187,6139,-32187,6139,-32187,
5633,-32280,5633,-32280,5633,-32280,5633,-32280,
5125,-32364,5125,-32364,5125,-32364,5125,-32364,
4616,-32441,4616,-32441,4616,-32441,4616,-32441,
4106,-32509,4106,-32509,4106,-32509,4106,-32509,
3595,-32570,3595,-32570,3595,-32570,3595,-32570,
3083,-32622,3083,-32622,3083,-32622,3083,-32622,
2570,-32666,2570,-32666,2570,-32666,2570,-32666,
2057,-32703,2057,-32703,2057,-32703,2057,-32703,
1543,-32731,1543,-32731,1543,-32731,1543,-32731,
1029,-32751,1029,-32751,1029,-32751,1029,-32751,
514,-32763,514,-32763,514,-32763,514,-32763,
0,-32767,0,-32767,0,-32767,0,-32767,
-515,-32763,-515,-32763,-515,-32763,-515,-32763,
-1030,-32751,-1030,-32751,-1030,-32751,-1030,-32751,
-1544,-32731,-1544,-32731,-1544,-32731,-1544,-32731,
-2058,-32703,-2058,-32703,-2058,-32703,-2058,-32703,
-2571,-32666,-2571,-32666,-2571,-32666,-2571,-32666,
-3084,-32622,-3084,-32622,-3084,-32622,-3084,-32622,
-3596,-32570,-3596,-32570,-3596,-32570,-3596,-32570,
-4107,-32509,-4107,-32509,-4107,-32509,-4107,-32509,
-4617,-32441,-4617,-32441,-4617,-32441,-4617,-32441,
-5126,-32364,-5126,-32364,-5126,-32364,-5126,-32364,
-5634,-32280,-5634,-32280,-5634,-32280,-5634,-32280,
-6140,-32187,-6140,-32187,-6140,-32187,-6140,-32187,
-6645,-32087,-6645,-32087,-6645,-32087,-6645,-32087,
-7148,-31978,-7148,-31978,-7148,-31978,-7148,-31978,
-7650,-31862,-7650,-31862,-7650,-31862,-7650,-31862,
-8149,-31738,-8149,-31738,-8149,-31738,-8149,-31738,
-8647,-31606,-8647,-31606,-8647,-31606,-8647,-31606,
-9142,-31466,-9142,-31466,-9142,-31466,-9142,-31466,
-9635,-31319,-9635,-31319,-9635,-31319,-9635,-31319,
-10126,-31164,-10126,-31164,-10126,-31164,-10126,-31164,
-10614,-31001,-10614,-31001,-10614,-31001,-10614,-31001,
-11100,-30830,-11100,-30830,-11100,-30830,-11100,-30830,
-11583,-30652,-11583,-30652,-11583,-30652,-11583,-30652,
-12063,-30466,-12063,-30466,-12063,-30466,-12063,-30466,
-12540,-30273,-12540,-30273,-12540,-30273,-12540,-30273,
-13014,-30073,-13014,-30073,-13014,-30073,-13014,-30073,
-13485,-29864,-13485,-29864,-13485,-29864,-13485,-29864,
-13952,-29649,-13952,-29649,-13952,-29649,-13952,-29649,
-14416,-29426,-14416,-29426,-14416,-29426,-14416,-29426,
-14876,-29196,-14876,-29196,-14876,-29196,-14876,-29196,
-15333,-28959,-15333,-28959,-15333,-28959,-15333,-28959,
-15786,-28714,-15786,-28714,-15786,-28714,-15786,-28714,
-16235,-28463,-16235,-28463,-16235,-28463,-16235,-28463,
-16680,-28204,-16680,-28204,-16680,-28204,-16680,-28204,
-17121,-27939,-17121,-27939,-17121,-27939,-17121,-27939,
-17558,-27667,-17558,-27667,-17558,-27667,-17558,-27667,
-17990,-27387,-17990,-27387,-17990,-27387,-17990,-27387,
-18418,-27101,-18418,-27101,-18418,-27101,-18418,-27101,
-18842,-26809,-18842,-26809,-18842,-26809,-18842,-26809,
-19260,-26510,-19260,-26510,-19260,-26510,-19260,-26510,
-19674,-26204,-19674,-26204,-19674,-26204,-19674,-26204,
-20084,-25892,-20084,-25892,-20084,-25892,-20084,-25892,
-20488,-25573,-20488,-25573,-20488,-25573,-20488,-25573,
-20887,-25248,-20887,-25248,-20887,-25248,-20887,-25248,
-21281,-24917,-21281,-24917,-21281,-24917,-21281,-24917,
-21670,-24579,-21670,-24579,-21670,-24579,-21670,-24579,
-22053,-24236,-22053,-24236,-22053,-24236,-22053,-24236,
-22431,-23887,-22431,-23887,-22431,-23887,-22431,-23887,
-22803,-23531,-22803,-23531,-22803,-23531,-22803,-23531,
-23170,-23170,-23170,-23170,-23170,-23170,-23170,-23170,
-23531,-22803,-23531,-22803,-23531,-22803,-23531,-22803,
-23887,-22431,-23887,-22431,-23887,-22431,-23887,-22431,
-24236,-22053,-24236,-22053,-24236,-22053,-24236,-22053,
-24579,-21670,-24579,-21670,-24579,-21670,-24579,-21670,
-24917,-21281,-24917,-21281,-24917,-21281,-24917,-21281,
-25248,-20887,-25248,-20887,-25248,-20887,-25248,-20887,
-25573,-20488,-25573,-20488,-25573,-20488,-25573,-20488,
-25892,-20084,-25892,-20084,-25892,-20084,-25892,-20084,
-26204,-19674,-26204,-19674,-26204,-19674,-26204,-19674,
-26510,-19260,-26510,-19260,-26510,-19260,-26510,-19260,
-26809,-18842,-26809,-18842,-26809,-18842,-26809,-18842,
-27101,-18418,-27101,-18418,-27101,-18418,-27101,-18418,
-27387,-17990,-27387,-17990,-27387,-17990,-27387,-17990,
-27667,-17558,-27667,-17558,-27667,-17558,-27667,-17558,
-27939,-17121,-27939,-17121,-27939,-17121,-27939,-17121,
-28204,-16680,-28204,-16680,-28204,-16680,-28204,-16680,
-28463,-16235,-28463,-16235,-28463,-16235,-28463,-16235,
-28714,-15786,-28714,-15786,-28714,-15786,-28714,-15786,
-28959,-15333,-28959,-15333,-28959,-15333,-28959,-15333,
-29196,-14876,-29196,-14876,-29196,-14876,-29196,-14876,
-29426,-14416,-29426,-14416,-29426,-14416,-29426,-14416,
-29649,-13952,-29649,-13952,-29649,-13952,-29649,-13952,
-29864,-13485,-29864,-13485,-29864,-13485,-29864,-13485,
-30073,-13014,-30073,-13014,-30073,-13014,-30073,-13014,
-30273,-12540,-30273,-12540,-30273,-12540,-30273,-12540,
-30466,-12063,-30466,-12063,-30466,-12063,-30466,-12063,
-30652,-11583,-30652,-11583,-30652,-11583,-30652,-11583,
-30830,-11100,-30830,-11100,-30830,-11100,-30830,-11100,
-31001,-10614,-31001,-10614,-31001,-10614,-31001,-10614,
-31164,-10126,-31164,-10126,-31164,-10126,-31164,-10126,
-31319,-9635,-31319,-9635,-31319,-9635,-31319,-9635,
-31466,-9142,-31466,-9142,-31466,-9142,-31466,-9142,
-31606,-8647,-31606,-8647,-31606,-8647,-31606,-8647,
-31738,-8149,-31738,-8149,-31738,-8149,-31738,-8149,
-31862,-7650,-31862,-7650,-31862,-7650,-31862,-7650,
-31978,-7148,-31978,-7148,-31978,-7148,-31978,-7148,
-32087,-6645,-32087,-6645,-32087,-6645,-32087,-6645,
-32187,-6140,-32187,-6140,-32187,-6140,-32187,-6140,
-32280,-5634,-32280,-5634,-32280,-5634,-32280,-5634,
-32364,-5126,-32364,-5126,-32364,-5126,-32364,-5126,
-32441,-4617,-32441,-4617,-32441,-4617,-32441,-4617,
-32509,-4107,-32509,-4107,-32509,-4107,-32509,-4107,
-32570,-3596,-32570,-3596,-32570,-3596,-32570,-3596,
-32622,-3084,-32622,-3084,-32622,-3084,-32622,-3084,
-32666,-2571,-32666,-2571,-32666,-2571,-32666,-2571,
-32703,-2058,-32703,-2058,-32703,-2058,-32703,-2058,
-32731,-1544,-32731,-1544,-32731,-1544,-32731,-1544,
-32751,-1030,-32751,-1030,-32751,-1030,-32751,-1030,
-32763,-515,-32763,-515,-32763,-515,-32763,-515,
-32767,-1,-32767,-1,-32767,-1,-32767,-1,
-32763,514,-32763,514,-32763,514,-32763,514,
-32751,1029,-32751,1029,-32751,1029,-32751,1029,
-32731,1543,-32731,1543,-32731,1543,-32731,1543,
-32703,2057,-32703,2057,-32703,2057,-32703,2057,
-32666,2570,-32666,2570,-32666,2570,-32666,2570,
-32622,3083,-32622,3083,-32622,3083,-32622,3083,
-32570,3595,-32570,3595,-32570,3595,-32570,3595,
-32509,4106,-32509,4106,-32509,4106,-32509,4106,
-32441,4616,-32441,4616,-32441,4616,-32441,4616,
-32364,5125,-32364,5125,-32364,5125,-32364,5125,
-32280,5633,-32280,5633,-32280,5633,-32280,5633,
-32187,6139,-32187,6139,-32187,6139,-32187,6139,
-32087,6644,-32087,6644,-32087,6644,-32087,6644,
-31978,7147,-31978,7147,-31978,7147,-31978,7147,
-31862,7649,-31862,7649,-31862,7649,-31862,7649,
-31738,8148,-31738,8148,-31738,8148,-31738,8148,
-31606,8646,-31606,8646,-31606,8646,-31606,8646,
-31466,9141,-31466,9141,-31466,9141,-31466,9141,
-31319,9634,-31319,9634,-31319,9634,-31319,9634,
-31164,10125,-31164,10125,-31164,10125,-31164,10125,
-31001,10613,-31001,10613,-31001,10613,-31001,10613,
-30830,11099,-30830,11099,-30830,11099,-30830,11099,
-30652,11582,-30652,11582,-30652,11582,-30652,11582,
-30466,12062,-30466,12062,-30466,12062,-30466,12062,
-30273,12539,-30273,12539,-30273,12539,-30273,12539,
-30073,13013,-30073,13013,-30073,13013,-30073,13013,
-29864,13484,-29864,13484,-29864,13484,-29864,13484,
-29649,13951,-29649,13951,-29649,13951,-29649,13951,
-29426,14415,-29426,14415,-29426,14415,-29426,14415,
-29196,14875,-29196,14875,-29196,14875,-29196,14875,
-28959,15332,-28959,15332,-28959,15332,-28959,15332,
-28714,15785,-28714,15785,-28714,15785,-28714,15785,
-28463,16234,-28463,16234,-28463,16234,-28463,16234,
-28204,16679,-28204,16679,-28204,16679,-28204,16679,
-27939,17120,-27939,17120,-27939,17120,-27939,17120,
-27667,17557,-27667,17557,-27667,17557,-27667,17557,
-27387,17989,-27387,17989,-27387,17989,-27387,17989,
-27101,18417,-27101,18417,-27101,18417,-27101,18417,
-26809,18841,-26809,18841,-26809,18841,-26809,18841,
-26510,19259,-26510,19259,-26510,19259,-26510,19259,
-26204,19673,-26204,19673,-26204,19673,-26204,19673,
-25892,20083,-25892,20083,-25892,20083,-25892,20083,
-25573,20487,-25573,20487,-25573,20487,-25573,20487,
-25248,20886,-25248,20886,-25248,20886,-25248,20886,
-24917,21280,-24917,21280,-24917,21280,-24917,21280,
-24579,21669,-24579,21669,-24579,21669,-24579,21669,
-24236,22052,-24236,22052,-24236,22052,-24236,22052,
-23887,22430,-23887,22430,-23887,22430,-23887,22430,
-23531,22802,-23531,22802,-23531,22802,-23531,22802,
-23170,23169,-23170,23169,-23170,23169,-23170,23169,
-22803,23530,-22803,23530,-22803,23530,-22803,23530,
-22431,23886,-22431,23886,-22431,23886,-22431,23886,
-22053,24235,-22053,24235,-22053,24235,-22053,24235,
-21670,24578,-21670,24578,-21670,24578,-21670,24578,
-21281,24916,-21281,24916,-21281,24916,-21281,24916,
-20887,25247,-20887,25247,-20887,25247,-20887,25247,
-20488,25572,-20488,25572,-20488,25572,-20488,25572,
-20084,25891,-20084,25891,-20084,25891,-20084,25891,
-19674,26203,-19674,26203,-19674,26203,-19674,26203,
-19260,26509,-19260,26509,-19260,26509,-19260,26509,
-18842,26808,-18842,26808,-18842,26808,-18842,26808,
-18418,27100,-18418,27100,-18418,27100,-18418,27100,
-17990,27386,-17990,27386,-17990,27386,-17990,27386,
-17558,27666,-17558,27666,-17558,27666,-17558,27666,
-17121,27938,-17121,27938,-17121,27938,-17121,27938,
-16680,28203,-16680,28203,-16680,28203,-16680,28203,
-16235,28462,-16235,28462,-16235,28462,-16235,28462,
-15786,28713,-15786,28713,-15786,28713,-15786,28713,
-15333,28958,-15333,28958,-15333,28958,-15333,28958,
-14876,29195,-14876,29195,-14876,29195,-14876,29195,
-14416,29425,-14416,29425,-14416,29425,-14416,29425,
-13952,29648,-13952,29648,-13952,29648,-13952,29648,
-13485,29863,-13485,29863,-13485,29863,-13485,29863,
-13014,30072,-13014,30072,-13014,30072,-13014,30072,
-12540,30272,-12540,30272,-12540,30272,-12540,30272,
-12063,30465,-12063,30465,-12063,30465,-12063,30465,
-11583,30651,-11583,30651,-11583,30651,-11583,30651,
-11100,30829,-11100,30829,-11100,30829,-11100,30829,
-10614,31000,-10614,31000,-10614,31000,-10614,31000,
-10126,31163,-10126,31163,-10126,31163,-10126,31163,
-9635,31318,-9635,31318,-9635,31318,-9635,31318,
-9142,31465,-9142,31465,-9142,31465,-9142,31465,
-8647,31605,-8647,31605,-8647,31605,-8647,31605,
-8149,31737,-8149,31737,-8149,31737,-8149,31737,
-7650,31861,-7650,31861,-7650,31861,-7650,31861,
-7148,31977,-7148,31977,-7148,31977,-7148,31977,
-6645,32086,-6645,32086,-6645,32086,-6645,32086,
-6140,32186,-6140,32186,-6140,32186,-6140,32186,
-5634,32279,-5634,32279,-5634,32279,-5634,32279,
-5126,32363,-5126,32363,-5126,32363,-5126,32363,
-4617,32440,-4617,32440,-4617,32440,-4617,32440,
-4107,32508,-4107,32508,-4107,32508,-4107,32508,
-3596,32569,-3596,32569,-3596,32569,-3596,32569,
-3084,32621,-3084,32621,-3084,32621,-3084,32621,
-2571,32665,-2571,32665,-2571,32665,-2571,32665,
-2058,32702,-2058,32702,-2058,32702,-2058,32702,
-1544,32730,-1544,32730,-1544,32730,-1544,32730,
-1030,32750,-1030,32750,-1030,32750,-1030,32750,
-515,32762,-515,32762,-515,32762,-515,32762
};


void dft1200(int16_t *x,int16_t *y,unsigned char scale_flag){

  int i,j;
  __m128i *x128=(__m128i *)x;
  __m128i *y128=(__m128i *)y;
  __m128i *twa128=(__m128i *)&twa1200[0];
  __m128i *twb128=(__m128i *)&twb1200[0];
  __m128i *twc128=(__m128i *)&twc1200[0];
  __m128i x2128[1200];// = (__m128i *)&x2128array[0];
  __m128i ytmp128[1200];//=&ytmp128array2[0];



  for (i=0,j=0;i<300;i++,j+=4) {
    x2128[i]    = x128[j];
    x2128[i+300] = x128[j+1];
    x2128[i+600] = x128[j+2];
    x2128[i+900] = x128[j+3];
  }

  dft300((int16_t *)x2128,(int16_t *)ytmp128,1);
  dft300((int16_t *)(x2128+300),(int16_t *)(ytmp128+300),1);
  dft300((int16_t *)(x2128+600),(int16_t *)(ytmp128+600),1);
  dft300((int16_t *)(x2128+900),(int16_t *)(ytmp128+900),1);

  bfly4_tw1(ytmp128,ytmp128+300,ytmp128+600,ytmp128+900,y128,y128+300,y128+600,y128+900);
  for (i=1,j=0;i<300;i++,j++) {
    bfly4(ytmp128+i,
	  ytmp128+300+i,
	  ytmp128+600+i,
	  ytmp128+900+i,
	  y128+i,
	  y128+300+i,
	  y128+600+i,
	  y128+900+i,
	  twa128+j,
	  twb128+j,
	  twc128+j);
  }

  if (scale_flag==1) {
    norm128 = _mm_set1_epi16(16384);//dft_norm_table[13]);
    
    for (i=0;i<1200;i++) {
      y128[i] = _mm_slli_epi16(_mm_mulhi_epi16(y128[i],norm128),1);
    }
  }

  _mm_empty();
  _m_empty();

}


#ifdef MR_MAIN

int opp_enabled=1;
int main(int argc, char**argv) {


  time_stats_t ts;
  __m128i x[2048],y[2048],tw0,tw1,tw2,tw3;


  int i;

  set_taus_seed(0);
/*
  ((int16_t *)&tw0)[0] = 32767;
  ((int16_t *)&tw0)[1] = 0;
  ((int16_t *)&tw0)[2] = 32767;
  ((int16_t *)&tw0)[3] = 0;
  ((int16_t *)&tw0)[4] = 32767;
  ((int16_t *)&tw0)[5] = 0;
  ((int16_t *)&tw0)[6] = 32767;
  ((int16_t *)&tw0)[7] = 0;

  ((int16_t *)&tw1)[0] = 32767;
  ((int16_t *)&tw1)[1] = 0;
  ((int16_t *)&tw1)[2] = 32767;
  ((int16_t *)&tw1)[3] = 0;
  ((int16_t *)&tw1)[4] = 32767;
  ((int16_t *)&tw1)[5] = 0;
  ((int16_t *)&tw1)[6] = 32767;
  ((int16_t *)&tw1)[7] = 0;

  ((int16_t *)&tw2)[0] = 32767;
  ((int16_t *)&tw2)[1] = 0;
  ((int16_t *)&tw2)[2] = 32767;
  ((int16_t *)&tw2)[3] = 0;
  ((int16_t *)&tw2)[4] = 32767;
  ((int16_t *)&tw2)[5] = 0;
  ((int16_t *)&tw2)[6] = 32767;
  ((int16_t *)&tw2)[7] = 0;

  ((int16_t *)&tw3)[0] = 32767;
  ((int16_t *)&tw3)[1] = 0;
  ((int16_t *)&tw3)[2] = 32767;
  ((int16_t *)&tw3)[3] = 0;
  ((int16_t *)&tw3)[4] = 32767;
  ((int16_t *)&tw3)[5] = 0;
  ((int16_t *)&tw3)[6] = 32767;
  ((int16_t *)&tw3)[7] = 0;
  
  for (i=0;i<300;i++) {
    x[i] = _mm_set1_epi32(taus());
    x[i] = _mm_srai_epi16(x[i],4);
  }

  bfly2_tw1(x,x+1,y,y+1);
  printf("(%d,%d) (%d,%d) => (%d,%d) (%d,%d)\n",((int16_t*)&x[0])[0],((int16_t*)&x[0])[1],((int16_t*)&x[1])[0],((int16_t*)&x[1])[1],((int16_t*)&y[0])[0],((int16_t*)&y[0])[1],((int16_t*)&y[1])[0],((int16_t*)&y[1])[1]);
  printf("(%d,%d) (%d,%d) => (%d,%d) (%d,%d)\n",((int16_t*)&x[0])[0],((int16_t*)&x[0])[1],((int16_t*)&x[1])[0],((int16_t*)&x[1])[1],((int16_t*)&y[0])[2],((int16_t*)&y[0])[3],((int16_t*)&y[1])[2],((int16_t*)&y[1])[3]);
  printf("(%d,%d) (%d,%d) => (%d,%d) (%d,%d)\n",((int16_t*)&x[0])[0],((int16_t*)&x[0])[1],((int16_t*)&x[1])[0],((int16_t*)&x[1])[1],((int16_t*)&y[0])[4],((int16_t*)&y[0])[5],((int16_t*)&y[1])[4],((int16_t*)&y[1])[5]);
  printf("(%d,%d) (%d,%d) => (%d,%d) (%d,%d)\n",((int16_t*)&x[0])[0],((int16_t*)&x[0])[1],((int16_t*)&x[1])[0],((int16_t*)&x[1])[1],((int16_t*)&y[0])[6],((int16_t*)&y[0])[7],((int16_t*)&y[1])[6],((int16_t*)&y[1])[7]);
  bfly2(x,x+1,y,y+1, &tw0);
  printf("0(%d,%d) (%d,%d) => (%d,%d) (%d,%d)\n",((int16_t*)&x[0])[0],((int16_t*)&x[0])[1],((int16_t*)&x[1])[0],((int16_t*)&x[1])[1],((int16_t*)&y[0])[0],((int16_t*)&y[0])[1],((int16_t*)&y[1])[0],((int16_t*)&y[1])[1]);
  printf("1(%d,%d) (%d,%d) => (%d,%d) (%d,%d)\n",((int16_t*)&x[0])[0],((int16_t*)&x[0])[1],((int16_t*)&x[1])[0],((int16_t*)&x[1])[1],((int16_t*)&y[0])[2],((int16_t*)&y[0])[3],((int16_t*)&y[1])[2],((int16_t*)&y[1])[3]);
  printf("2(%d,%d) (%d,%d) => (%d,%d) (%d,%d)\n",((int16_t*)&x[0])[0],((int16_t*)&x[0])[1],((int16_t*)&x[1])[0],((int16_t*)&x[1])[1],((int16_t*)&y[0])[4],((int16_t*)&y[0])[5],((int16_t*)&y[1])[4],((int16_t*)&y[1])[5]);
  printf("3(%d,%d) (%d,%d) => (%d,%d) (%d,%d)\n",((int16_t*)&x[0])[0],((int16_t*)&x[0])[1],((int16_t*)&x[1])[0],((int16_t*)&x[1])[1],((int16_t*)&y[0])[6],((int16_t*)&y[0])[7],((int16_t*)&y[1])[6],((int16_t*)&y[1])[7]);
  bfly2(x,x+1,y,y+1, &tw0);


  bfly3_tw1(x,x+1,x+2,y, y+1,y+2);
  printf("0(%d,%d) (%d,%d) (%d %d) => (%d,%d) (%d,%d) (%d %d)\n",((int16_t*)&x[0])[0],((int16_t*)&x[0])[1],((int16_t*)&x[1])[0],((int16_t*)&x[1])[1],((int16_t*)&x[2])[0],((int16_t*)&x[2])[1],((int16_t*)&y[0])[0],((int16_t*)&y[0])[1],((int16_t*)&y[1])[0],((int16_t*)&y[1])[1],((int16_t*)&y[2])[0],((int16_t*)&y[2])[1]);
  printf("1(%d,%d) (%d,%d) (%d %d) => (%d,%d) (%d,%d) (%d %d)\n",((int16_t*)&x[0])[0],((int16_t*)&x[0])[1],((int16_t*)&x[1])[0],((int16_t*)&x[1])[1],((int16_t*)&x[2])[0],((int16_t*)&x[2])[1],((int16_t*)&y[0])[2],((int16_t*)&y[0])[3],((int16_t*)&y[1])[2],((int16_t*)&y[1])[3],((int16_t*)&y[2])[2],((int16_t*)&y[2])[3]);
  printf("2(%d,%d) (%d,%d) (%d %d) => (%d,%d) (%d,%d) (%d %d)\n",((int16_t*)&x[0])[0],((int16_t*)&x[0])[1],((int16_t*)&x[1])[0],((int16_t*)&x[1])[1],((int16_t*)&x[2])[0],((int16_t*)&x[2])[1],((int16_t*)&y[0])[4],((int16_t*)&y[0])[5],((int16_t*)&y[1])[4],((int16_t*)&y[1])[5],((int16_t*)&y[2])[4],((int16_t*)&y[2])[5]);
  printf("3(%d,%d) (%d,%d) (%d %d) => (%d,%d) (%d,%d) (%d %d)\n",((int16_t*)&x[0])[0],((int16_t*)&x[0])[1],((int16_t*)&x[1])[0],((int16_t*)&x[1])[1],((int16_t*)&x[2])[0],((int16_t*)&x[2])[1],((int16_t*)&y[0])[6],((int16_t*)&y[0])[7],((int16_t*)&y[1])[6],((int16_t*)&y[1])[7],((int16_t*)&y[2])[6],((int16_t*)&y[2])[7]);
  bfly3(x,x+1,x+2,y, y+1,y+2,&tw0,&tw1);

  printf("0(%d,%d) (%d,%d) (%d %d) => (%d,%d) (%d,%d) (%d %d)\n",((int16_t*)&x[0])[0],((int16_t*)&x[0])[1],((int16_t*)&x[1])[0],((int16_t*)&x[1])[1],((int16_t*)&x[2])[0],((int16_t*)&x[2])[1],((int16_t*)&y[0])[0],((int16_t*)&y[0])[1],((int16_t*)&y[1])[0],((int16_t*)&y[1])[1],((int16_t*)&y[2])[0],((int16_t*)&y[2])[1]);
  printf("1(%d,%d) (%d,%d) (%d %d) => (%d,%d) (%d,%d) (%d %d)\n",((int16_t*)&x[0])[0],((int16_t*)&x[0])[1],((int16_t*)&x[1])[0],((int16_t*)&x[1])[1],((int16_t*)&x[2])[0],((int16_t*)&x[2])[1],((int16_t*)&y[0])[2],((int16_t*)&y[0])[3],((int16_t*)&y[1])[2],((int16_t*)&y[1])[3],((int16_t*)&y[2])[2],((int16_t*)&y[2])[3]);
  printf("2(%d,%d) (%d,%d) (%d %d) => (%d,%d) (%d,%d) (%d %d)\n",((int16_t*)&x[0])[0],((int16_t*)&x[0])[1],((int16_t*)&x[1])[0],((int16_t*)&x[1])[1],((int16_t*)&x[2])[0],((int16_t*)&x[2])[1],((int16_t*)&y[0])[4],((int16_t*)&y[0])[5],((int16_t*)&y[1])[4],((int16_t*)&y[1])[5],((int16_t*)&y[2])[4],((int16_t*)&y[2])[5]);
  printf("3(%d,%d) (%d,%d) (%d %d) => (%d,%d) (%d,%d) (%d %d)\n",((int16_t*)&x[0])[0],((int16_t*)&x[0])[1],((int16_t*)&x[1])[0],((int16_t*)&x[1])[1],((int16_t*)&x[2])[0],((int16_t*)&x[2])[1],((int16_t*)&y[0])[6],((int16_t*)&y[0])[7],((int16_t*)&y[1])[6],((int16_t*)&y[1])[7],((int16_t*)&y[2])[6],((int16_t*)&y[2])[7]);

  
  bfly4_tw1(x,x+1,x+2,x+3,y, y+1,y+2,y+3);
  printf("(%d,%d) (%d,%d) (%d %d) (%d,%d) => (%d,%d) (%d,%d) (%d %d) (%d,%d)\n",
	 ((int16_t*)&x[0])[0],((int16_t*)&x[0])[1],((int16_t*)&x[1])[0],((int16_t*)&x[1])[1],
	 ((int16_t*)&x[2])[0],((int16_t*)&x[2])[1],((int16_t*)&x[3])[0],((int16_t*)&x[3])[1],
	 ((int16_t*)&y[0])[0],((int16_t*)&y[0])[1],((int16_t*)&y[1])[0],((int16_t*)&y[1])[1],
	 ((int16_t*)&y[2])[0],((int16_t*)&y[2])[1],((int16_t*)&y[3])[0],((int16_t*)&y[3])[1]);

  bfly4(x,x+1,x+2,x+3,y, y+1,y+2,y+3,&tw0,&tw1,&tw2);
  printf("0(%d,%d) (%d,%d) (%d %d) (%d,%d) => (%d,%d) (%d,%d) (%d %d) (%d,%d)\n",
	 ((int16_t*)&x[0])[0],((int16_t*)&x[0])[1],((int16_t*)&x[1])[0],((int16_t*)&x[1])[1],
	 ((int16_t*)&x[2])[0],((int16_t*)&x[2])[1],((int16_t*)&x[3])[0],((int16_t*)&x[3])[1],
	 ((int16_t*)&y[0])[0],((int16_t*)&y[0])[1],((int16_t*)&y[1])[0],((int16_t*)&y[1])[1],
	 ((int16_t*)&y[2])[0],((int16_t*)&y[2])[1],((int16_t*)&y[3])[0],((int16_t*)&y[3])[1]);
  printf("1(%d,%d) (%d,%d) (%d %d) (%d,%d) => (%d,%d) (%d,%d) (%d %d) (%d,%d)\n",
	 ((int16_t*)&x[0])[0],((int16_t*)&x[0])[1],((int16_t*)&x[1])[0],((int16_t*)&x[1])[1],
	 ((int16_t*)&x[2])[0],((int16_t*)&x[2])[1],((int16_t*)&x[3])[0],((int16_t*)&x[3])[1],
	 ((int16_t*)&y[0])[2],((int16_t*)&y[0])[3],((int16_t*)&y[1])[2],((int16_t*)&y[1])[3],
	 ((int16_t*)&y[2])[2],((int16_t*)&y[2])[3],((int16_t*)&y[3])[2],((int16_t*)&y[3])[3]);
  printf("2(%d,%d) (%d,%d) (%d %d) (%d,%d) => (%d,%d) (%d,%d) (%d %d) (%d,%d)\n",
	 ((int16_t*)&x[0])[0],((int16_t*)&x[0])[1],((int16_t*)&x[1])[0],((int16_t*)&x[1])[1],
	 ((int16_t*)&x[2])[0],((int16_t*)&x[2])[1],((int16_t*)&x[3])[0],((int16_t*)&x[3])[1],
	 ((int16_t*)&y[0])[4],((int16_t*)&y[0])[5],((int16_t*)&y[1])[4],((int16_t*)&y[1])[5],
	 ((int16_t*)&y[2])[4],((int16_t*)&y[2])[5],((int16_t*)&y[3])[4],((int16_t*)&y[3])[5]);
  printf("3(%d,%d) (%d,%d) (%d %d) (%d,%d) => (%d,%d) (%d,%d) (%d %d) (%d,%d)\n",
	 ((int16_t*)&x[0])[0],((int16_t*)&x[0])[1],((int16_t*)&x[1])[0],((int16_t*)&x[1])[1],
	 ((int16_t*)&x[2])[6],((int16_t*)&x[2])[7],((int16_t*)&x[3])[6],((int16_t*)&x[3])[7],
	 ((int16_t*)&y[0])[6],((int16_t*)&y[0])[7],((int16_t*)&y[1])[6],((int16_t*)&y[1])[7],
	 ((int16_t*)&y[2])[0],((int16_t*)&y[2])[1],((int16_t*)&y[3])[0],((int16_t*)&y[3])[1]);

  bfly5_tw1(x,x+1,x+2,x+3,x+4,y,y+1,y+2,y+3,y+4);

  for (i=0;i<5;i++)
    printf("%d,%d,",
	   ((int16_t*)&x[i])[0],((int16_t*)&x[i])[1]);
  printf("\n");
  for (i=0;i<5;i++)
    printf("%d,%d,",
	   ((int16_t*)&y[i])[0],((int16_t*)&y[i])[1]);
  printf("\n");

  bfly5(x,x+1,x+2,x+3,x+4,y, y+1,y+2,y+3,y+4,&tw0,&tw1,&tw2,&tw3);
  for (i=0;i<5;i++)
    printf("%d,%d,",
	   ((int16_t*)&x[i])[0],((int16_t*)&x[i])[1]);
  printf("\n");
  for (i=0;i<5;i++)
    printf("%d,%d,",
	   ((int16_t*)&y[i])[0],((int16_t*)&y[i])[1]);
  printf("\n");


  printf("\n\n12-point\n");   
  dft12f(x,
	 x+1,
	 x+2,
	 x+3,
	 x+4,
	 x+5,
	 x+6,
	 x+7,
	 x+8,
	 x+9,
	 x+10,
	 x+11,
	 y,
	 y+1,
	 y+2,
	 y+3,
	 y+4,
	 y+5,
	 y+6,
	 y+7,
	 y+8,
	 y+9,
	 y+10,
	 y+11);


  printf("X: ");
  for (i=0;i<12;i++)
    printf("%d,%d,",((int16_t*)(&x[i]))[0],((int16_t *)(&x[i]))[1]);
  printf("\nY:");
  for (i=0;i<12;i++)
    printf("%d,%d,",((int16_t*)(&y[i]))[0],((int16_t *)(&y[i]))[1]);
  printf("\n");
  
  dft24((int16_t *)x,(int16_t *)y,1);
  printf("\n\n24-point\n");
  printf("X: ");
  for (i=0;i<24;i++)
    printf("%d,%d,",((int16_t*)(&x[i]))[0],((int16_t *)(&x[i]))[1]);
  printf("\nY:");
  for (i=0;i<24;i++)
    printf("%d,%d,",((int16_t*)(&y[i]))[0],((int16_t *)(&y[i]))[1]);
  printf("\n");
  
  dft36((int16_t *)x,(int16_t *)y,1);
  printf("\n\n36-point\n");
  printf("X: ");
  for (i=0;i<36;i++)
    printf("%d,%d,",((int16_t*)(&x[i]))[0],((int16_t *)(&x[i]))[1]);
  printf("\nY:");
  for (i=0;i<36;i++)
    printf("%d,%d,",((int16_t*)(&y[i]))[0],((int16_t *)(&y[i]))[1]);
  printf("\n");

  dft48((int16_t *)x,(int16_t *)y,1);
  printf("\n\n48-point\n");
  printf("X: ");
  for (i=0;i<48;i++)
    printf("%d,%d,",((int16_t*)(&x[i]))[0],((int16_t *)(&x[i]))[1]);
  printf("\nY:");
  for (i=0;i<48;i++)
    printf("%d,%d,",((int16_t*)(&y[i]))[0],((int16_t *)(&y[i]))[1]);
  printf("\n");

  dft60((int16_t *)x,(int16_t *)y,1);
  printf("\n\n60-point\n");
  printf("X: ");
  for (i=0;i<60;i++)
    printf("%d,%d,",((int16_t*)(&x[i]))[0],((int16_t *)(&x[i]))[1]);
  printf("\nY:");
  for (i=0;i<60;i++)
    printf("%d,%d,",((int16_t*)(&y[i]))[0],((int16_t *)(&y[i]))[1]);
  printf("\n");

  dft72((int16_t *)x,(int16_t *)y,1);
  printf("\n\n72-point\n");
  printf("X: ");
  for (i=0;i<72;i++)
    printf("%d,%d,",((int16_t*)(&x[i]))[0],((int16_t *)(&x[i]))[1]);
  printf("\nY:");
  for (i=0;i<72;i++)
    printf("%d: %d,%d\n",i,((int16_t*)(&y[i]))[0],((int16_t *)(&y[i]))[1]);
  printf("\n");

  dft96((int16_t *)x,(int16_t *)y,1);
  printf("\n\n96-point\n");
  printf("X: ");
  for (i=0;i<96;i++)
    printf("%d,%d,",((int16_t*)(&x[i]))[0],((int16_t *)(&x[i]))[1]);
  printf("\nY:");
  for (i=0;i<96;i++)
    printf("%d: %d,%d\n",i,((int16_t*)(&y[i]))[0],((int16_t *)(&y[i]))[1]);
  printf("\n");

  dft108((int16_t *)x,(int16_t *)y,1);
  printf("\n\n108-point\n");
  printf("X: ");
  for (i=0;i<108;i++)
    printf("%d,%d,",((int16_t*)(&x[i]))[0],((int16_t *)(&x[i]))[1]);
  printf("\nY:");
  for (i=0;i<108;i++)
    printf("%d: %d,%d\n",i,((int16_t*)(&y[i]))[0],((int16_t *)(&y[i]))[1]);
  printf("\n");

  dft120((int16_t *)x,(int16_t *)y,1);
  printf("\n\n120-point\n");
  printf("X: ");
  for (i=0;i<120;i++)
    printf("%d,%d,",((int16_t*)(&x[i]))[0],((int16_t *)(&x[i]))[1]);
  printf("\nY:");
  for (i=0;i<120;i++)
    printf("%d: %d,%d\n",i,((int16_t*)(&y[i]))[0],((int16_t *)(&y[i]))[1]);
  printf("\n");

  dft144((int16_t *)x,(int16_t *)y,1);
  printf("\n\n144-point\n");
  printf("X: ");
  for (i=0;i<144;i++)
    printf("%d,%d,",((int16_t*)(&x[i]))[0],((int16_t *)(&x[i]))[1]);
  printf("\nY:");
  for (i=0;i<144;i++)
    printf("%d: %d,%d\n",i,((int16_t*)(&y[i]))[0],((int16_t *)(&y[i]))[1]);
  printf("\n");

  dft180((int16_t *)x,(int16_t *)y,1);
  printf("\n\n180-point\n");
  printf("X: ");
  for (i=0;i<180;i++)
    printf("%d,%d,",((int16_t*)(&x[i]))[0],((int16_t *)(&x[i]))[1]);
  printf("\nY:");
  for (i=0;i<180;i++)
    printf("%d: %d,%d\n",i,((int16_t*)(&y[i]))[0],((int16_t *)(&y[i]))[1]);
  printf("\n");

  dft192((int16_t *)x,(int16_t *)y,1);
  printf("\n\n192-point\n");
  printf("X: ");
  for (i=0;i<192;i++)
    printf("%d,%d,",((int16_t*)(&x[i]))[0],((int16_t *)(&x[i]))[1]);
  printf("\nY:");
  for (i=0;i<192;i++)
    printf("%d: %d,%d\n",i,((int16_t*)(&y[i]))[0],((int16_t *)(&y[i]))[1]);
  printf("\n");

  dft216((int16_t *)x,(int16_t *)y,1);
  printf("\n\n216-point\n");
  printf("X: ");
  for (i=0;i<216;i++)
    printf("%d,%d,",((int16_t*)(&x[i]))[0],((int16_t *)(&x[i]))[1]);
  printf("\nY:");
  for (i=0;i<216;i++)
    printf("%d: %d,%d\n",i,((int16_t*)(&y[i]))[0],((int16_t *)(&y[i]))[1]);
  printf("\n");

  dft240((int16_t *)x,(int16_t *)y,1);
  printf("\n\n240-point\n");
  printf("X: ");
  for (i=0;i<240;i++)
    printf("%d,%d,",((int16_t*)(&x[i]))[0],((int16_t *)(&x[i]))[1]);
  printf("\nY:");
  for (i=0;i<240;i++)
    printf("%d: %d,%d\n",i,((int16_t*)(&y[i]))[0],((int16_t *)(&y[i]))[1]);
  printf("\n");

  dft288((int16_t *)x,(int16_t *)y,1);
  printf("\n\n288-point\n");
  printf("X: ");
  for (i=0;i<288;i++)
    printf("%d,%d,",((int16_t*)(&x[i]))[0],((int16_t *)(&x[i]))[1]);
  printf("\nY:");
  for (i=0;i<288;i++)
    printf("%d: %d,%d\n",i,((int16_t*)(&y[i]))[0],((int16_t *)(&y[i]))[1]);
  printf("\n");

  dft300((int16_t *)x,(int16_t *)y,1);
  printf("\n\n300-point\n");
  printf("X: ");
  for (i=0;i<300;i++)
    printf("%d,%d,",((int16_t*)(&x[i]))[0],((int16_t *)(&x[i]))[1]);
  printf("\nY:");
  for (i=0;i<300;i++)
    printf("%d: %d,%d\n",i,((int16_t*)(&y[i]))[0],((int16_t *)(&y[i]))[1]);
  printf("\n");

  for (i=0;i<32;i++) {
    ((int16_t*)x)[i] = (int16_t)((taus()&0xffff))>>5;
  }
  memset((void*)&y[0],0,16*4);
  idft16((int16_t *)x,(int16_t *)y);
  printf("\n\n16-point\n");
  printf("X: ");
  for (i=0;i<4;i++)
    printf("%d,%d,%d,%d,%d,%d,%d,%d,",((int16_t*)&x[i])[0],((int16_t *)&x[i])[1],((int16_t*)&x[i])[2],((int16_t *)&x[i])[3],((int16_t*)&x[i])[4],((int16_t*)&x[i])[5],((int16_t*)&x[i])[6],((int16_t*)&x[i])[7]);
  printf("\nY:");

  for (i=0;i<4;i++)
    printf("%d,%d,%d,%d,%d,%d,%d,%d,",((int16_t*)&y[i])[0],((int16_t *)&y[i])[1],((int16_t*)&y[i])[2],((int16_t *)&y[i])[3],((int16_t*)&y[i])[4],((int16_t *)&y[i])[5],((int16_t*)&y[i])[6],((int16_t *)&y[i])[7]);
  printf("\n");
*/
  for (i=0;i<128;i++) {
    ((int16_t*)x)[i] = (int16_t)((taus()&0xffff))>>5;
  }
  memset((void*)&y[0],0,64*4);
  dft64((int16_t *)x,(int16_t *)y,1);
  dft64((int16_t *)x,(int16_t *)y,1);
  dft64((int16_t *)x,(int16_t *)y,1); 
  reset_meas(&ts);
  for (i=0;i<10000000;i++) {
    start_meas(&ts);
    dft64((int16_t *)x,(int16_t *)y,1);
    stop_meas(&ts);

  }
  printf("\n\n64-point (%f cycles)\n",(double)ts.diff/(double)ts.trials);
  /*printf("X: ");
  for (i=0;i<16;i++)
    printf("%d,%d,%d,%d,%d,%d,%d,%d,",((int16_t*)&x[i])[0],((int16_t *)&x[i])[1],((int16_t*)&x[i])[2],((int16_t *)&x[i])[3],((int16_t*)&x[i])[4],((int16_t*)&x[i])[5],((int16_t*)&x[i])[6],((int16_t*)&x[i])[7]);
  printf("\nY:");

  for (i=0;i<16;i++)
    printf("%d,%d,%d,%d,%d,%d,%d,%d,",((int16_t*)&y[i])[0],((int16_t *)&y[i])[1],((int16_t*)&y[i])[2],((int16_t *)&y[i])[3],((int16_t*)&y[i])[4],((int16_t *)&y[i])[5],((int16_t*)&y[i])[6],((int16_t *)&y[i])[7]);
  printf("\n");
*/
  for (i=0;i<256;i++) {
    ((int16_t*)x)[i] = (int16_t)((taus()&0xffff))>>5;
  }
  memset((void*)&y[0],0,128*4);
  reset_meas(&ts);
  for (i=0;i<10000;i++) {
    start_meas(&ts);
    dft128((int16_t *)x,(int16_t *)y,0);
    stop_meas(&ts);
  }
  printf("\n\n128-point(%f cycles)\n",(double)ts.diff/(double)ts.trials);
 /* printf("X: ");
  for (i=0;i<32;i++)
    printf("%d,%d,%d,%d,%d,%d,%d,%d,",((int16_t*)&x[i])[0],((int16_t *)&x[i])[1],((int16_t*)&x[i])[2],((int16_t *)&x[i])[3],((int16_t*)&x[i])[4],((int16_t*)&x[i])[5],((int16_t*)&x[i])[6],((int16_t*)&x[i])[7]);
  printf("\nY:");

  for (i=0;i<32;i++)
    printf("%d,%d,%d,%d,%d,%d,%d,%d,",((int16_t*)&y[i])[0],((int16_t *)&y[i])[1],((int16_t*)&y[i])[2],((int16_t *)&y[i])[3],((int16_t*)&y[i])[4],((int16_t *)&y[i])[5],((int16_t*)&y[i])[6],((int16_t *)&y[i])[7]);
  printf("\n");
*/
  for (i=0;i<512;i++) {
    ((int16_t*)x)[i] = (int16_t)((taus()&0xffff))>>5;
  }
  memset((void*)&y[0],0,256*4);
  reset_meas(&ts);
  for (i=0;i<10000;i++) {
    start_meas(&ts);
    dft256((int16_t *)x,(int16_t *)y,1);
    stop_meas(&ts);
  }
  printf("\n\n256-point(%f cycles)\n",(double)ts.diff/(double)ts.trials);
 /* printf("X: ");
  for (i=0;i<64;i++)
    printf("%d,%d,%d,%d,%d,%d,%d,%d,",((int16_t*)&x[i])[0],((int16_t *)&x[i])[1],((int16_t*)&x[i])[2],((int16_t *)&x[i])[3],((int16_t*)&x[i])[4],((int16_t*)&x[i])[5],((int16_t*)&x[i])[6],((int16_t*)&x[i])[7]);
  printf("\nY:");

  for (i=0;i<64;i++)
    printf("%d,%d,%d,%d,%d,%d,%d,%d,",((int16_t*)&y[i])[0],((int16_t *)&y[i])[1],((int16_t*)&y[i])[2],((int16_t *)&y[i])[3],((int16_t*)&y[i])[4],((int16_t *)&y[i])[5],((int16_t*)&y[i])[6],((int16_t *)&y[i])[7]);
  printf("\n");
*/
  reset_meas(&ts);
  for (i=0;i<10000;i++) {
    start_meas(&ts);
    dft512((int16_t *)x,(int16_t *)y,1);
    stop_meas(&ts);
  }
  printf("\n\n512-point(%f cycles)\n",(double)ts.diff/(double)ts.trials);

  reset_meas(&ts);
  for (i=0;i<10000;i++) {
    start_meas(&ts);
    dft1024((int16_t *)x,(int16_t *)y,1);
    stop_meas(&ts);
  }
  printf("\n\n1024-point(%f cycles)\n",(double)ts.diff/(double)ts.trials);

  reset_meas(&ts);
  for (i=0;i<10000;i++) {
    start_meas(&ts);
    dft2048((int16_t *)x,(int16_t *)y,1);
    stop_meas(&ts);
  }
  printf("\n\n2048-point(%f cycles)\n",(double)ts.diff/(double)ts.trials);
  return(0);
}


#endif
