#include <stdio.h>
#include <stdlib.h>

#include "emmintrin.h"
#include "xmmintrin.h"

#define cmultr(ar,ai,br,bi) (_mm_slli_epi16(_mm_subs_epi16(_mm_mulhi_epi16((ar),(br)),_mm_mulhi_epi16((ai),(bi))),1))
#define cmulti(ar,ai,br,bi) (_mm_slli_epi16(_mm_adds_epi16(_mm_mulhi_epi16((ar),(bi)),_mm_mulhi_epi16((ai),(br))),1))

static short W13rs[8]__attribute__((aligned(16))) = {-16384,-16384,-16384,-16384,-16384,-16384,-16384,-16384};
static short W13is[8]__attribute__((aligned(16))) = {-28378,-28378,-28378,-28378,-28378,-28378,-28378,-28378};
static short W23rs[8]__attribute__((aligned(16))) = {-16384,-16384,-16384,-16384,-16384,-16384,-16384,-16384};
static short W23is[8]__attribute__((aligned(16))) = {28378,28378,28378,28378,28378,28378,28378,28378};

static short W15rs[8]__attribute__((aligned(16))) = {10126,10126,10126,10126,10126,10126,10126,10126};
static short W15is[8]__attribute__((aligned(16))) = {-31164,-31164,-31164,-31164,-31164,-31164,-31164,-31164};
static short W25rs[8]__attribute__((aligned(16))) = {-26510,-26510,-26510,-26510,-26510,-26510,-26510,-26510};
static short W25is[8]__attribute__((aligned(16))) = {-19261,-19261,-19261,-19261,-19261,-19261,-19261,-19261};
static short W35rs[8]__attribute__((aligned(16))) = {-26510,-26510,-26510,-26510,-26510,-26510,-26510,-26510};
static short W35is[8]__attribute__((aligned(16))) = {19261,19261,19261,19261,19261,19261,19261,19261};
static short W45rs[8]__attribute__((aligned(16))) = {10126,10126,10126,10126,10126,10126,10126,10126};
static short W45is[8]__attribute__((aligned(16))) = {31164,31164,31164,31164,31164,31164,31164};

__m128i *W13r=(__m128i *)W13rs;
__m128i *W13i=(__m128i *)W13is;
__m128i *W23r=(__m128i *)W23rs;
__m128i *W23i=(__m128i *)W23is;

__m128i *W15r=(__m128i *)W15rs;
__m128i *W15i=(__m128i *)W15is;
__m128i *W25r=(__m128i *)W25rs;
__m128i *W25i=(__m128i *)W25is;
__m128i *W35r=(__m128i *)W35rs;
__m128i *W35i=(__m128i *)W35is;
__m128i *W45r=(__m128i *)W45rs;
__m128i *W45i=(__m128i *)W45is;

static inline void bfly2(__m128i *x0r, __m128i *x1r,__m128i *y0r, __m128i *y1r,__m128i *twr,
		  __m128i *x0i, __m128i *x1i,__m128i *y0i, __m128i *y1i,__m128i *twi) {

  __m128i x1r_2,x1i_2;

  x1r_2 = cmultr(*x1r,*x1i,*twr,*twi);
  x1i_2 = cmulti(*x1r,*x1i,*twr,*twi);
  //  printf("x0i %d, x1i2 %d\n",((short *)x0i)[0],((short *)&x1i_2)[0]);
  //  printf("x1r %d x1r_2 %d, x1i %d x1i_2 %d\n",((short *)x1r)[0],((short *)&x1r_2)[0],((short *)x1i)[0],((short *)&x1i_2)[0]);
  *y0r = _mm_adds_epi16(*x0r,x1r_2);
  *y1r = _mm_subs_epi16(*x0r,x1r_2);
  *y0i = _mm_adds_epi16(*x0i,x1i_2);
  //  printf("y0i %d\n",((short *)y0i)[0]);
  *y1i = _mm_subs_epi16(*x0i,x1i_2);
}

static inline void bfly2_tw1(__m128i *x0r, __m128i *x1r,
			     __m128i *y0r, __m128i *y1r,
			     __m128i *x0i, __m128i *x1i,
			     __m128i *y0i, __m128i *y1i) {

  *y0r = _mm_adds_epi16(*x0r,*x1r);
  *y1r = _mm_subs_epi16(*x0r,*x1r);
  *y0i = _mm_adds_epi16(*x0i,*x1i);
  *y1i = _mm_subs_epi16(*x0i,*x1i);
}

/*
static inline void bfly3(__m128i *x0r, __m128i *x1r, __m128i *x2r,
		  __m128i *y0r, __m128i *y1r,__m128i *y2r,
		  __m128i *twr1,__m128i *twr2,
		  __m128i *x0i, __m128i *x1i, __m128i *x2i,
		  __m128i *y0i, __m128i *y1i,__m128i *y2i,
		  __m128i *twi1,__m128i *twi2) {

  __m128i x1r_2,x1i_2;
  __m128i x2r_2,x2i_2;
  __m128i tmp1,tmp2;


  x1r_2 = cmultr(*x1r,*x1i,*twr1,*twi1);
  x1i_2 = cmulti(*x1r,*x1i,*twr1,*twi1);
  x2r_2 = cmultr(*x2r,*x2i,*twr2,*twi2);
  x2i_2 = cmulti(*x2r,*x2i,*twr2,*twi2);

  //  printf("x1r_2 %d, x1i_2 %d x2r_2 %d, x2i_2 %d\n",((short *)&x1r_2)[0],((short *)&x1i_2)[0],((short *)&x2r_2)[0],((short *)&x2i_2)[0]);

  tmp1 = _mm_adds_epi16(x1r_2,x2r_2);
  tmp2 = _mm_adds_epi16(x1i_2,x2i_2);


  *y0r = _mm_adds_epi16(*x0r,_mm_adds_epi16(x1r_2,x2r_2));

  *y1r = _mm_adds_epi16(*x0r,_mm_adds_epi16(cmultr(*W13r,*W13i,x1r_2,x1i_2),cmultr(*W23r,*W23i,x2r_2,x2i_2)));
  *y2r = _mm_adds_epi16(*x0r,_mm_adds_epi16(cmultr(*W23r,*W23i,x1r_2,x1i_2),cmultr(*W13r,*W13i,x2r_2,x2i_2)));

  *y0i = _mm_adds_epi16(*x0i,_mm_adds_epi16(x1i_2,x2i_2));
  *y1i = _mm_adds_epi16(*x0i,_mm_adds_epi16(cmulti(*W13r,*W13i,x1r_2,x1i_2),cmulti(*W23r,*W23i,x2r_2,x2i_2)));
  *y2i = _mm_adds_epi16(*x0i,_mm_adds_epi16(cmulti(*W23r,*W23i,x1r_2,x1i_2),cmulti(*W13r,*W13i,x2r_2,x2i_2)));
}

*/


__m128i x1r_2,x1i_2;
__m128i x2r_2,x2i_2;
__m128i tmp1,tmp2;

#define bfly3(x0r,x1r,x2r,y0r,y1r,y2r,twr1,twr2,x0i,x1i,x2i,y0i,y1i,y2i,twi1,twi2) x1r_2 = cmultr(*(x1r),*(x1i),*(twr1),*(twi1)); \
  x1i_2 = cmulti(*(x1r),*(x1i),*(twr1),*(twi1)); \
  x2r_2 = cmultr(*(x2r),*(x2i),*(twr2),*(twi2)); \
  x2i_2 = cmulti(*(x2r),*(x2i),*(twr2),*(twi2)); \
  tmp1 = _mm_adds_epi16(x1r_2,x2r_2); \
  tmp2 = _mm_adds_epi16(x1i_2,x2i_2); \
  *(y0r) = _mm_adds_epi16(*(x0r),_mm_adds_epi16(x1r_2,x2r_2)); \
  *(y1r) = _mm_adds_epi16(*(x0r),_mm_adds_epi16(cmultr(*W13r,*W13i,x1r_2,x1i_2),cmultr(*W23r,*W23i,x2r_2,x2i_2))); \
  *(y2r) = _mm_adds_epi16(*(x0r),_mm_adds_epi16(cmultr(*W23r,*W23i,x1r_2,x1i_2),cmultr(*W13r,*W13i,x2r_2,x2i_2))); \
  *(y0i) = _mm_adds_epi16(*(x0i),_mm_adds_epi16(x1i_2,x2i_2)); \
  *(y1i) = _mm_adds_epi16(*(x0i),_mm_adds_epi16(cmulti(*W13r,*W13i,x1r_2,x1i_2),cmulti(*W23r,*W23i,x2r_2,x2i_2))); \
  *(y2i) = _mm_adds_epi16(*(x0i),_mm_adds_epi16(cmulti(*W23r,*W23i,x1r_2,x1i_2),cmulti(*W13r,*W13i,x2r_2,x2i_2)))


/*
static inline void bfly3_tw1(__m128i *x0r, __m128i *x1r, __m128i *x2r,
		      __m128i *y0r, __m128i *y1r,__m128i *y2r,
		      __m128i *x0i, __m128i *x1i, __m128i *x2i,
		      __m128i *y0i, __m128i *y1i,__m128i *y2i){

  __m128i tmp1,tmp2;


  //  printf("x1r_2 %d, x1i_2 %d x2r_2 %d, x2i_2 %d\n",((short *)&x1r_2)[0],((short *)&x1i_2)[0],((short *)&x2r_2)[0],((short *)&x2i_2)[0]);

  tmp1 = _mm_adds_epi16(*x1r,*x2r);
  tmp2 = _mm_adds_epi16(*x1i,*x2i);


  *y0r = _mm_adds_epi16(*x0r,_mm_adds_epi16(*x1r,*x2r));

  *y1r = _mm_adds_epi16(*x0r,_mm_adds_epi16(cmultr(*W13r,*W13i,*x1r,*x1i),cmultr(*W23r,*W23i,*x2r,*x2i)));
  *y2r = _mm_adds_epi16(*x0r,_mm_adds_epi16(cmultr(*W23r,*W23i,*x1r,*x1i),cmultr(*W13r,*W13i,*x2r,*x2i)));

  *y0i = _mm_adds_epi16(*x0i,_mm_adds_epi16(*x1i,*x2i));
  *y1i = _mm_adds_epi16(*x0i,_mm_adds_epi16(cmulti(*W13r,*W13i,*x1r,*x1i),cmulti(*W23r,*W23i,*x2r,*x2i)));
  *y2i = _mm_adds_epi16(*x0i,_mm_adds_epi16(cmulti(*W23r,*W23i,*x1r,*x1i),cmulti(*W13r,*W13i,*x2r,*x2i)));
}
*/



#define bfly3_tw1(x0r,x1r,x2r,y0r,y1r,y2r,x0i,x1i,x2i,y0i,y1i,y2i)  tmp1 = _mm_adds_epi16(*(x1r),*(x2r));\
  tmp2 = _mm_adds_epi16(*(x1i),*(x2i)); \
  *(y0r) = _mm_adds_epi16(*(x0r),_mm_adds_epi16(*(x1r),*(x2r)));\
  *(y1r) = _mm_adds_epi16(*(x0r),_mm_adds_epi16(cmultr(*W13r,*W13i,*(x1r),*(x1i)),cmultr(*W23r,*W23i,*(x2r),*(x2i))));\
  *(y2r) = _mm_adds_epi16(*(x0r),_mm_adds_epi16(cmultr(*W23r,*W23i,*(x1r),*(x1i)),cmultr(*W13r,*W13i,*(x2r),*(x2i))));\
  *(y0i) = _mm_adds_epi16(*(x0i),_mm_adds_epi16(*(x1i),*(x2i)));\
  *(y1i) = _mm_adds_epi16(*(x0i),_mm_adds_epi16(cmulti(*W13r,*W13i,*(x1r),*(x1i)),cmulti(*W23r,*W23i,*(x2r),*(x2i))));\
  *(y2i) = _mm_adds_epi16(*(x0i),_mm_adds_epi16(cmulti(*W23r,*W23i,*(x1r),*(x1i)),cmulti(*W13r,*W13i,*(x2r),*(x2i))))

/*
static inline void bfly4(__m128i *x0r, __m128i *x1r, __m128i *x2r, __m128i *x3r,
		  __m128i *y0r, __m128i *y1r, __m128i *y2r, __m128i *y3r,
		  __m128i *twr1,__m128i *twr2,__m128i *twr3,
		  __m128i *x0i, __m128i *x1i, __m128i *x2i, __m128i *x3i,
		  __m128i *y0i, __m128i *y1i, __m128i *y2i, __m128i *y3i,
		  __m128i *twi1,__m128i *twi2,__m128i *twi3) {

  __m128i x1r_2,x1i_2;
  __m128i x2r_2,x2i_2;
  __m128i x3r_2,x3i_2;

  x1r_2 = cmultr(*x1r,*x1i,*twr1,*twi1);
  x1i_2 = cmulti(*x1r,*x1i,*twr1,*twi1);
  x2r_2 = cmultr(*x2r,*x2i,*twr2,*twi2);
  x2i_2 = cmulti(*x2r,*x2i,*twr2,*twi2);
  x3r_2 = cmultr(*x3r,*x3i,*twr3,*twi3);
  x3i_2 = cmulti(*x3r,*x3i,*twr3,*twi3);

  *y0r = _mm_adds_epi16(*x0r,_mm_adds_epi16(x1r_2,_mm_adds_epi16(x2r_2,x3r_2)));
  *y0i = _mm_adds_epi16(*x0i,_mm_adds_epi16(x1i_2,_mm_adds_epi16(x2i_2,x3i_2)));

  *y1r = _mm_adds_epi16(*x0r,_mm_subs_epi16(x1i_2,_mm_adds_epi16(x2r_2,x3i_2)));
  // y1r = x0r + (x1i - (x2r + x3i))
  *y1i = _mm_subs_epi16(*x0i,_mm_adds_epi16(x1r_2,_mm_subs_epi16(x2i_2,x3r_2)));
  // y1i = x0i - x1r - x2i + x3r = x0i - (x1r + (x2i - x3r))
  *y2r = _mm_subs_epi16(*x0r,_mm_subs_epi16(x1r_2,_mm_subs_epi16(x2r_2,x3r_2)));
  // y2r = x0r - x1r + x2r - x3r = x0r - (x1r - (x2r+x3r))
  *y2i = _mm_subs_epi16(*x0i,_mm_subs_epi16(x1i_2,_mm_subs_epi16(x2i_2,x3i_2)));
  // y2i = x0i - x1i + x2i - x3i = x0i - (x1i - (x2i+x3i))
  *y3r = _mm_subs_epi16(*x0r,_mm_adds_epi16(x1i_2,_mm_subs_epi16(x2r_2,x3i_2)));
  //y3r = x0r - x1i - x2r + x3i = x0r - (x1i + (x2r - x3i))
  *y3i = _mm_adds_epi16(*x0i,_mm_subs_epi16(x1r_2,_mm_adds_epi16(x2i_2,x3r_2)));
  //y3i = x0i + x1r - x2i - x3r = x0i + (x1r - (x2i + x3r))
}
*/

//__m128i x1r_2,x1i_2;
//__m128i x2r_2,x2i_2;
__m128i x3r_2,x3i_2;

#define  bfly4(x0r,x1r,x2r,x3r,y0r,y1r,y2r,y3r,twr1,twr2,twr3,x0i,x1i,x2i,x3i,y0i,y1i,y2i,y3i,twi1,twi2,twi3)   x1r_2 = cmultr(*(x1r),*(x1i),*(twr1),*(twi1));\
  x1i_2 = cmulti(*(x1r),*(x1i),*(twr1),*(twi1));\
  x2r_2 = cmultr(*(x2r),*(x2i),*(twr2),*(twi2));\
  x2i_2 = cmulti(*(x2r),*(x2i),*(twr2),*(twi2));\
  x3r_2 = cmultr(*(x3r),*(x3i),*(twr3),*(twi3));\
  x3i_2 = cmulti(*(x3r),*(x3i),*(twr3),*(twi3));\
  *(y0r) = _mm_adds_epi16(*(x0r),_mm_adds_epi16(x1r_2,_mm_adds_epi16(x2r_2,x3r_2)));\
  *(y0i) = _mm_adds_epi16(*(x0i),_mm_adds_epi16(x1i_2,_mm_adds_epi16(x2i_2,x3i_2)));\
  *(y1r) = _mm_adds_epi16(*(x0r),_mm_subs_epi16(x1i_2,_mm_adds_epi16(x2r_2,x3i_2)));\
  *(y1i) = _mm_subs_epi16(*(x0i),_mm_adds_epi16(x1r_2,_mm_subs_epi16(x2i_2,x3r_2)));\
  *(y2r) = _mm_subs_epi16(*(x0r),_mm_subs_epi16(x1r_2,_mm_subs_epi16(x2r_2,x3r_2)));\
  *(y2i) = _mm_subs_epi16(*(x0i),_mm_subs_epi16(x1i_2,_mm_subs_epi16(x2i_2,x3i_2)));\
  *(y3r) = _mm_subs_epi16(*(x0r),_mm_adds_epi16(x1i_2,_mm_subs_epi16(x2r_2,x3i_2)));\
  *(y3i) = _mm_adds_epi16(*(x0i),_mm_subs_epi16(x1r_2,_mm_adds_epi16(x2i_2,x3r_2)))


/*
static inline void bfly4_tw1(__m128i *x0r, __m128i *x1r, __m128i *x2r, __m128i *x3r,
		      __m128i *y0r, __m128i *y1r, __m128i *y2r, __m128i *y3r,
		      __m128i *x0i, __m128i *x1i, __m128i *x2i, __m128i *x3i,
		      __m128i *y0i, __m128i *y1i,__m128i *y2i, __m128i *y3i) {

  *y0r = _mm_adds_epi16(*x0r,_mm_adds_epi16(*x1r,_mm_adds_epi16(*x2r,*x3r)));
  *y0i = _mm_adds_epi16(*x0i,_mm_adds_epi16(*x1i,_mm_adds_epi16(*x2i,*x3i)));

  *y1r = _mm_adds_epi16(*x0r,_mm_subs_epi16(*x1i,_mm_adds_epi16(*x2r,*x3i)));
  // y1r = x0r + (x1i - (x2r + x3i))
  *y1i = _mm_subs_epi16(*x0i,_mm_adds_epi16(*x1r,_mm_subs_epi16(*x2i,*x3r)));
  // y1i = x0i - x1r - x2i + x3r = x0i - (x1r + (x2i - x3r))
  *y2r = _mm_subs_epi16(*x0r,_mm_subs_epi16(*x1r,_mm_subs_epi16(*x2r,*x3r)));
  // y2r = x0r - x1r + x2r - x3r = x0r - (x1r - (x2r+x3r))
  *y2i = _mm_subs_epi16(*x0i,_mm_subs_epi16(*x1i,_mm_subs_epi16(*x2i,*x3i)));
  // y2i = x0i - x1i + x2i - x3i = x0i - (x1i - (x2i+x3i))
  *y3r = _mm_subs_epi16(*x0r,_mm_adds_epi16(*x1i,_mm_subs_epi16(*x2r,*x3i)));
  //y3r = x0r - x1i - x2r + x3i = x0r - (x1i + (x2r - x3i))
  *y3i = _mm_adds_epi16(*x0i,_mm_subs_epi16(*x1r,_mm_adds_epi16(*x2i,*x3r)));
  //y3i = x0i + x1r - x2i - x3r = x0i + (x1r - (x2i + x3r))
}
*/

#define bfly4_tw1(x0r,x1r,x2r,x3r,y0r,y1r,y2r,y3r,x0i,x1i,x2i,x3i,y0i,y1i,y2i,y3i)  *(y0r) = _mm_adds_epi16(*(x0r),_mm_adds_epi16(*(x1r),_mm_adds_epi16(*(x2r),*(x3r)))); \
  *(y0i) = _mm_adds_epi16(*(x0i),_mm_adds_epi16(*(x1i),_mm_adds_epi16(*(x2i),*(x3i)))); \
  *(y1r) = _mm_adds_epi16(*(x0r),_mm_subs_epi16(*(x1i),_mm_adds_epi16(*(x2r),*(x3i)))); \
  *(y1i) = _mm_subs_epi16(*(x0i),_mm_adds_epi16(*(x1r),_mm_subs_epi16(*(x2i),*(x3r)))); \
  *(y2r) = _mm_subs_epi16(*(x0r),_mm_subs_epi16(*(x1r),_mm_subs_epi16(*(x2r),*(x3r)))); \
  *(y2i) = _mm_subs_epi16(*(x0i),_mm_subs_epi16(*(x1i),_mm_subs_epi16(*(x2i),*(x3i)))); \
  *(y3r) = _mm_subs_epi16(*(x0r),_mm_adds_epi16(*(x1i),_mm_subs_epi16(*(x2r),*(x3i)))); \
  *(y3i) = _mm_adds_epi16(*(x0i),_mm_subs_epi16(*(x1r),_mm_adds_epi16(*(x2i),*(x3r))))

/*
static inline void bfly5(__m128i *x0r, __m128i *x1r, __m128i *x2r, __m128i *x3r,__m128i *x4r,
		  __m128i *y0r, __m128i *y1r, __m128i *y2r, __m128i *y3r,__m128i *y4r,
		  __m128i *twr1,__m128i *twr2,__m128i *twr3,__m128i *twr4,
		  __m128i *x0i, __m128i *x1i, __m128i *x2i, __m128i *x3i,__m128i *x4i,
		  __m128i *y0i, __m128i *y1i, __m128i *y2i, __m128i *y3i,__m128i *y4i,
		  __m128i *twi1,__m128i *twi2,__m128i *twi3,__m128i *twi4) {

  __m128i x1r_2,x1i_2;
  __m128i x2r_2,x2i_2;
  __m128i x3r_2,x3i_2;
  __m128i x4r_2,x4i_2;

  x1r_2 = cmultr(*x1r,*x1i,*twr1,*twi1);
  x1i_2 = cmulti(*x1r,*x1i,*twr1,*twi1);
  x2r_2 = cmultr(*x2r,*x2i,*twr2,*twi2);
  x2i_2 = cmulti(*x2r,*x2i,*twr2,*twi2);
  x3r_2 = cmultr(*x3r,*x3i,*twr3,*twi3);
  x3i_2 = cmulti(*x3r,*x3i,*twr3,*twi3);
  x4r_2 = cmultr(*x4r,*x4i,*twr4,*twi4);
  x4i_2 = cmulti(*x4r,*x4i,*twr4,*twi4);

  *y0r = _mm_adds_epi16(*x0r,_mm_adds_epi16(x1r_2,_mm_adds_epi16(x2r_2,_mm_adds_epi16(x3r_2,x4r_2))));
  *y0i = _mm_adds_epi16(*x0i,_mm_adds_epi16(x1i_2,_mm_adds_epi16(x2i_2,_mm_adds_epi16(x3i_2,x4i_2))));

  *y1r = _mm_adds_epi16(*x0r,_mm_adds_epi16(cmultr(x1r_2,x1i_2,*W15r,*W15i),_mm_adds_epi16(cmultr(x2r_2,x2i_2,*W25r,*W25i),_mm_adds_epi16(cmultr(x3r_2,x3i_2,*W35r,*W35i),cmultr(x4r_2,x4i_2,*W45r,*W45i)))));
  *y1i = _mm_adds_epi16(*x0i,_mm_adds_epi16(cmulti(x1r_2,x1i_2,*W15r,*W15i),_mm_adds_epi16(cmulti(x2r_2,x2i_2,*W25r,*W25i),_mm_adds_epi16(cmulti(x3r_2,x3i_2,*W35r,*W35i),cmulti(x4r_2,x4i_2,*W45r,*W45i)))));

  *y2r = _mm_adds_epi16(*x0r,_mm_adds_epi16(cmultr(x1r_2,x1i_2,*W25r,*W25i),_mm_adds_epi16(cmultr(x2r_2,x2i_2,*W45r,*W45i),_mm_adds_epi16(cmultr(x3r_2,x3i_2,*W15r,*W15i),cmultr(x4r_2,x4i_2,*W35r,*W35i)))));
  *y2i = _mm_adds_epi16(*x0i,_mm_adds_epi16(cmulti(x1r_2,x1i_2,*W25r,*W25i),_mm_adds_epi16(cmulti(x2r_2,x2i_2,*W45r,*W45i),_mm_adds_epi16(cmulti(x3r_2,x3i_2,*W15r,*W15i),cmulti(x4r_2,x4i_2,*W35r,*W35i)))));

  *y3r = _mm_adds_epi16(*x0r,_mm_adds_epi16(cmultr(x1r_2,x1i_2,*W35r,*W35i),_mm_adds_epi16(cmultr(x2r_2,x2i_2,*W15r,*W15i),_mm_adds_epi16(cmultr(x3r_2,x3i_2,*W45r,*W45i),cmultr(x4r_2,x4i_2,*W25r,*W25i)))));
  *y3i = _mm_adds_epi16(*x0i,_mm_adds_epi16(cmulti(x1r_2,x1i_2,*W35r,*W35i),_mm_adds_epi16(cmulti(x2r_2,x2i_2,*W15r,*W15i),_mm_adds_epi16(cmulti(x3r_2,x3i_2,*W45r,*W45i),cmulti(x4r_2,x4i_2,*W25r,*W25i)))));

  *y4r = _mm_adds_epi16(*x0r,_mm_adds_epi16(cmultr(x1r_2,x1i_2,*W45r,*W45i),_mm_adds_epi16(cmultr(x2r_2,x2i_2,*W35r,*W35i),_mm_adds_epi16(cmultr(x3r_2,x3i_2,*W25r,*W25i),cmultr(x4r_2,x4i_2,*W15r,*W15i)))));
  *y4i = _mm_adds_epi16(*x0i,_mm_adds_epi16(cmulti(x1r_2,x1i_2,*W45r,*W45i),_mm_adds_epi16(cmulti(x2r_2,x2i_2,*W35r,*W35i),_mm_adds_epi16(cmulti(x3r_2,x3i_2,*W25r,*W25i),cmulti(x4r_2,x4i_2,*W15r,*W15i)))));
}  
*/

#define bfly5_tw1(x0r, x1r, x2r, x3r,x4r,y0r, y1r, y2r, y3r,y4r,x0i, x1i, x2i, x3i,x4i,y0i, y1i, y2i, y3i,y4i) \
  *(y0r) = _mm_adds_epi16(*(x0r),_mm_adds_epi16(*(x1r),_mm_adds_epi16(*(x2r),_mm_adds_epi16(*(x3r),*(x4r)))));\
  *(y0i) = _mm_adds_epi16(*(x0i),_mm_adds_epi16(*(x1i),_mm_adds_epi16(*(x2i),_mm_adds_epi16(*(x3i),*(x4i)))));\
  *(y1r) = _mm_adds_epi16(*(x0r),_mm_adds_epi16(cmultr(*(x1r),*(x1i),*W15r,*W15i),_mm_adds_epi16(cmultr(*(x2r),*(x2i),*W25r,*W25i),_mm_adds_epi16(cmultr(*(x3r),*(x3i),*W35r,*W35i),cmultr(*(x4r),*(x4i),*W45r,*W45i)))));\
  *(y1i) = _mm_adds_epi16(*(x0i),_mm_adds_epi16(cmulti(*(x1r),*(x1i),*W15r,*W15i),_mm_adds_epi16(cmulti(*(x2r),*(x2i),*W25r,*W25i),_mm_adds_epi16(cmulti(*(x3r),*(x3i),*W35r,*W35i),cmulti(*(x4r),*(x4i),*W45r,*W45i)))));\
  *(y2r) = _mm_adds_epi16(*(x0r),_mm_adds_epi16(cmultr(*(x1r),*(x1i),*W25r,*W25i),_mm_adds_epi16(cmultr(*(x2r),*(x2i),*W45r,*W45i),_mm_adds_epi16(cmultr(*(x3r),*(x3i),*W15r,*W15i),cmultr(*(x4r),*(x4i),*W35r,*W35i)))));\
  *(y2i) = _mm_adds_epi16(*(x0i),_mm_adds_epi16(cmulti(*(x1r),*(x1i),*W25r,*W25i),_mm_adds_epi16(cmulti(*(x2r),*(x2i),*W45r,*W45i),_mm_adds_epi16(cmulti(*(x3r),*(x3i),*W15r,*W15i),cmulti(*(x4r),*(x4i),*W35r,*W35i)))));\
  *(y3r) = _mm_adds_epi16(*(x0r),_mm_adds_epi16(cmultr(*(x1r),*(x1i),*W35r,*W35i),_mm_adds_epi16(cmultr(*(x2r),*(x2i),*W15r,*W15i),_mm_adds_epi16(cmultr(*(x3r),*(x3i),*W45r,*W45i),cmultr(*(x4r),*(x4i),*W25r,*W25i)))));\
  *(y3i) = _mm_adds_epi16(*(x0i),_mm_adds_epi16(cmulti(*(x1r),*(x1i),*W35r,*W35i),_mm_adds_epi16(cmulti(*(x2r),*(x2i),*W15r,*W15i),_mm_adds_epi16(cmulti(*(x3r),*(x3i),*W45r,*W45i),cmulti(*(x4r),*(x4i),*W25r,*W25i)))));\
  *(y4r) = _mm_adds_epi16(*(x0r),_mm_adds_epi16(cmultr(*(x1r),*(x1i),*W45r,*W45i),_mm_adds_epi16(cmultr(*(x2r),*(x2i),*W35r,*W35i),_mm_adds_epi16(cmultr(*(x3r),*(x3i),*W25r,*W25i),cmultr(*(x4r),*(x4i),*W15r,*W15i)))));\
  *(y4i) = _mm_adds_epi16(*(x0i),_mm_adds_epi16(cmulti(*(x1r),*(x1i),*W45r,*W45i),_mm_adds_epi16(cmulti(*(x2r),*(x2i),*W35r,*W35i),_mm_adds_epi16(cmulti(*(x3r),*(x3i),*W25r,*W25i),cmulti(*(x4r),*(x4i),*W15r,*W15i)))))
//printf("%d %d\n",((short*)y4r)[0],((short*)y4i)[0]);

__m128i x4r_2,x4i_2;

#define bfly5(x0r, x1r, x2r, x3r,x4r,\
	      y0r, y1r, y2r, y3r,y4r,\
	      twr1,twr2,twr3,twr4,\
	      x0i, x1i, x2i, x3i,x4i,\
	      y0i, y1i, y2i, y3i,y4i,\
	      twi1,twi2,twi3,twi4)\
  x1r_2 = cmultr(*(x1r),*(x1i),*(twr1),*(twi1));\
  x1i_2 = cmulti(*(x1r),*(x1i),*(twr1),*(twi1));\
  x2r_2 = cmultr(*(x2r),*(x2i),*(twr2),*(twi2));\
  x2i_2 = cmulti(*(x2r),*(x2i),*(twr2),*(twi2));\
  x3r_2 = cmultr(*(x3r),*(x3i),*(twr3),*(twi3));\
  x3i_2 = cmulti(*(x3r),*(x3i),*(twr3),*(twi3));\
  x4r_2 = cmultr(*(x4r),*(x4i),*(twr4),*(twi4));\
  x4i_2 = cmulti(*(x4r),*(x4i),*(twr4),*(twi4));\
  *(y0r) = _mm_adds_epi16(*(x0r),_mm_adds_epi16(x1r_2,_mm_adds_epi16(x2r_2,_mm_adds_epi16(x3r_2,x4r_2))));\
  *(y0i) = _mm_adds_epi16(*(x0i),_mm_adds_epi16(x1i_2,_mm_adds_epi16(x2i_2,_mm_adds_epi16(x3i_2,x4i_2))));\
  *(y1r) = _mm_adds_epi16(*(x0r),_mm_adds_epi16(cmultr(x1r_2,x1i_2,*W15r,*W15i),_mm_adds_epi16(cmultr(x2r_2,x2i_2,*W25r,*W25i),_mm_adds_epi16(cmultr(x3r_2,x3i_2,*W35r,*W35i),cmultr(x4r_2,x4i_2,*W45r,*W45i)))));\
  *(y1i) = _mm_adds_epi16(*(x0i),_mm_adds_epi16(cmulti(x1r_2,x1i_2,*W15r,*W15i),_mm_adds_epi16(cmulti(x2r_2,x2i_2,*W25r,*W25i),_mm_adds_epi16(cmulti(x3r_2,x3i_2,*W35r,*W35i),cmulti(x4r_2,x4i_2,*W45r,*W45i)))));\
  *(y2r) = _mm_adds_epi16(*(x0r),_mm_adds_epi16(cmultr(x1r_2,x1i_2,*W25r,*W25i),_mm_adds_epi16(cmultr(x2r_2,x2i_2,*W45r,*W45i),_mm_adds_epi16(cmultr(x3r_2,x3i_2,*W15r,*W15i),cmultr(x4r_2,x4i_2,*W35r,*W35i)))));\
  *(y2i) = _mm_adds_epi16(*(x0i),_mm_adds_epi16(cmulti(x1r_2,x1i_2,*W25r,*W25i),_mm_adds_epi16(cmulti(x2r_2,x2i_2,*W45r,*W45i),_mm_adds_epi16(cmulti(x3r_2,x3i_2,*W15r,*W15i),cmulti(x4r_2,x4i_2,*W35r,*W35i)))));\
  *(y3r) = _mm_adds_epi16(*(x0r),_mm_adds_epi16(cmultr(x1r_2,x1i_2,*W35r,*W35i),_mm_adds_epi16(cmultr(x2r_2,x2i_2,*W15r,*W15i),_mm_adds_epi16(cmultr(x3r_2,x3i_2,*W45r,*W45i),cmultr(x4r_2,x4i_2,*W25r,*W25i)))));\
  *(y3i) = _mm_adds_epi16(*(x0i),_mm_adds_epi16(cmulti(x1r_2,x1i_2,*W35r,*W35i),_mm_adds_epi16(cmulti(x2r_2,x2i_2,*W15r,*W15i),_mm_adds_epi16(cmulti(x3r_2,x3i_2,*W45r,*W45i),cmulti(x4r_2,x4i_2,*W25r,*W25i)))));\
  *(y4r) = _mm_adds_epi16(*(x0r),_mm_adds_epi16(cmultr(x1r_2,x1i_2,*W45r,*W45i),_mm_adds_epi16(cmultr(x2r_2,x2i_2,*W35r,*W35i),_mm_adds_epi16(cmultr(x3r_2,x3i_2,*W25r,*W25i),cmultr(x4r_2,x4i_2,*W15r,*W15i)))));\
  *(y4i) = _mm_adds_epi16(*(x0i),_mm_adds_epi16(cmulti(x1r_2,x1i_2,*W45r,*W45i),_mm_adds_epi16(cmulti(x2r_2,x2i_2,*W35r,*W35i),_mm_adds_epi16(cmulti(x3r_2,x3i_2,*W25r,*W25i),cmulti(x4r_2,x4i_2,*W15r,*W15i)))))


static short W1_12res[8]__attribute__((aligned(16))) = {28377,28377,28377,28377,28377,28377,28377,28377};
static short W1_12ims[8]__attribute__((aligned(16))) = {-16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383};
static short W2_12res[8]__attribute__((aligned(16))) = {16383,16383,16383,16383,16383,16383,16383,16383};
static short W2_12ims[8]__attribute__((aligned(16))) = {-28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377};
static short W3_12res[8]__attribute__((aligned(16))) = {0,0,0,0,0,0,0,0};
static short W3_12ims[8]__attribute__((aligned(16))) = {-32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767};
static short W4_12res[8]__attribute__((aligned(16))) = {-16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383};
static short W4_12ims[8]__attribute__((aligned(16))) = {-28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377};
static short W6_12res[8]__attribute__((aligned(16))) = {-32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767};
static short W6_12ims[8]__attribute__((aligned(16))) = {0,0,0,0,0,0,0,0};

__m128i *W1_12re=(__m128i *)W1_12res;
__m128i *W1_12im=(__m128i *)W1_12ims;
__m128i *W2_12re=(__m128i *)W2_12res;
__m128i *W2_12im=(__m128i *)W2_12ims;
__m128i *W3_12re=(__m128i *)W3_12res;
__m128i *W3_12im=(__m128i *)W3_12ims;
__m128i *W4_12re=(__m128i *)W4_12res;
__m128i *W4_12im=(__m128i *)W4_12ims;
__m128i *W6_12re=(__m128i *)W6_12res;
__m128i *W6_12im=(__m128i *)W6_12ims;


static inline void dft12f(__m128i *x0re,
	   __m128i *x1re,
	   __m128i *x2re,
	   __m128i *x3re,
	   __m128i *x4re,
	   __m128i *x5re,
	   __m128i *x6re,
	   __m128i *x7re,
	   __m128i *x8re,
	   __m128i *x9re,
	   __m128i *x10re,
	   __m128i *x11re,
	   __m128i *y0re,
	   __m128i *y1re,
	   __m128i *y2re,
	   __m128i *y3re,
	   __m128i *y4re,
	   __m128i *y5re,
	   __m128i *y6re,
	   __m128i *y7re,
	   __m128i *y8re,
	   __m128i *y9re,
	   __m128i *y10re,
	   __m128i *y11re,
	   __m128i *x0im,
	   __m128i *x1im,
	   __m128i *x2im,
	   __m128i *x3im,
	   __m128i *x4im,
	   __m128i *x5im,
	   __m128i *x6im,
	   __m128i *x7im,
	   __m128i *x8im,
	   __m128i *x9im,
	   __m128i *x10im,
	   __m128i *x11im,
	   __m128i *y0im,
	   __m128i *y1im,
	   __m128i *y2im,
	   __m128i *y3im,
	   __m128i *y4im,
	   __m128i *y5im,
	   __m128i *y6im,
	   __m128i *y7im,
	   __m128i *y8im,
	   __m128i *y9im,
	   __m128i *y10im,
	   __m128i *y11im) {

  __m128i tmpre[12],tmpim[12];

  bfly4_tw1(x0re, 
	    x3re, 
	    x6re, 
	    x9re,
	    tmpre,
	    tmpre+3,
	    tmpre+6,
	    tmpre+9,
	    x0im, 
	    x3im, 
	    x6im, 
	    x9im,
	    tmpim,
	    tmpim+3,
	    tmpim+6,
	    tmpim+9);

  bfly4_tw1(x1re, 
	    x4re, 
	    x7re, 
	    x10re,
	    tmpre+1,
	    tmpre+4,
	    tmpre+7,
	    tmpre+10,
	    x1im, 
	    x4im, 
	    x7im, 
	    x10im,
	    tmpim+1,
	    tmpim+4,
	    tmpim+7,
	    tmpim+10);

  bfly4_tw1(x2re, 
	    x5re, 
	    x8re, 
	    x11re,
	    tmpre+2,
	    tmpre+5,
	    tmpre+8,
	    tmpre+11,
	    x2im, 
	    x5im, 
	    x8im, 
	    x11im,
	    tmpim+2,
	    tmpim+5,
	    tmpim+8,
	    tmpim+11);

  //  k2=0;
  bfly3_tw1(tmpre,
	    tmpre+1,
	    tmpre+2,
	    y0re,
	    y4re,
	    y8re,
	    tmpim,
	    tmpim+1,
	    tmpim+2,
	    y0im,
	    y4im,
	    y8im);

  //  k2=1;
  bfly3(tmpre+3,
	tmpre+4,
	tmpre+5,
	y1re,
	y5re,
	y9re,
	W1_12re,
	W2_12re,
	tmpim+3,
	tmpim+4,
	tmpim+5,
	y1im,
	y5im,
	y9im,
	W1_12im,
	W2_12im);

  //  k2=2;
  bfly3(tmpre+6,
	tmpre+7,
	tmpre+8,
	y2re,
	y6re,
	y10re,
	W2_12re,
	W4_12re,
	tmpim+6,
	tmpim+7,
	tmpim+8,
	y2im,
	y6im,
	y10im,
	W2_12im,
	W4_12im);

  //  k2=3;
  bfly3(tmpre+9,
	tmpre+10,
	tmpre+11,
	y3re,
	y7re,
	y11re,	
	W3_12re,
	W6_12re,
	tmpim+9,
	tmpim+10,
	tmpim+11,
	y3im,
	y7im,
	y11im,
	W3_12im,
	W6_12im);
	    
}


__m128i tmpre[12],tmpim[12];

#define dft12(x0re,x1re,x2re,x3re,x4re,x5re,x6re,x7re,x8re,x9re,x10re,x11re,y0re,y1re,y2re,y3re,y4re,y5re,y6re,y7re,y8re,y9re,y10re,y11re,x0im,x1im,x2im,x3im,x4im,x5im,x6im,x7im,x8im,x9im,x10im,x11im,y0im,y1im,y2im,y3im,y4im,y5im,y6im,y7im,y8im,y9im,y10im,y11im) \
  bfly4_tw1((x0re),(x3re),(x6re),(x9re),tmpre,tmpre+3,tmpre+6,tmpre+9,(x0im),(x3im),(x6im),(x9im),tmpim,tmpim+3,tmpim+6,tmpim+9);\
  bfly4_tw1((x1re),(x4re),(x7re),(x10re),tmpre+1,tmpre+4,tmpre+7,tmpre+10,(x1im),(x4im),(x7im),(x10im),tmpim+1,tmpim+4,tmpim+7,tmpim+10);\
  bfly4_tw1((x2re),(x5re),(x8re),(x11re),tmpre+2,tmpre+5,tmpre+8,tmpre+11,(x2im),(x5im),(x8im),(x11im),tmpim+2,tmpim+5,tmpim+8,tmpim+11);\
  bfly3_tw1(tmpre,tmpre+1,tmpre+2,(y0re),(y4re),(y8re),tmpim,tmpim+1,tmpim+2,(y0im),(y4im),(y8im));\
  bfly3(tmpre+3,tmpre+4,tmpre+5,(y1re),(y5re),(y9re),W1_12re,W2_12re,tmpim+3,tmpim+4,tmpim+5,(y1im),(y5im),(y9im),W1_12im,W2_12im);\
  bfly3(tmpre+6,tmpre+7,tmpre+8,(y2re),(y6re),(y10re),W2_12re,W4_12re,tmpim+6,tmpim+7,tmpim+8,(y2im),(y6im),(y10im),W2_12im,W4_12im);\
  bfly3(tmpre+9,tmpre+10,tmpre+11,(y3re),(y7re),(y11re),W3_12re,W6_12re,tmpim+9,tmpim+10,tmpim+11,(y3im),(y7im),(y11im),W3_12im,W6_12im)


static short twr24[88]__attribute__((aligned(16))) = {31650,31650,31650,31650,31650,31650,31650,31650,\
			     28377,28377,28377,28377,28377,28377,28377,28377,\
			     23170,23170,23170,23170,23170,23170,23170,23170,\
			     16383,16383,16383,16383,16383,16383,16383,16383,\
			     8480,8480,8480,8480,8480,8480,8480,8480,\
			     0,0,0,0,0,0,0,0,\
			     -8480,-8480,-8480,-8480,-8480,-8480,-8480,-8480,\
			     -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,\
			     -23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,\
			     -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,\
			     -31650,-31650,-31650,-31650,-31650,-31650,-31650,-31650};

static short twi24[88]__attribute__((aligned(16))) = {-8480,-8480,-8480,-8480,-8480,-8480,-8480,-8480,\
			     -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,\
			     -23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,\
			     -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,\
			     -31650,-31650,-31650,-31650,-31650,-31650,-31650,-31650,\
			     -32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767,\
			     -31650,-31650,-31650,-31650,-31650,-31650,-31650,-31650,\
			     -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,\
			     -23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,\
			     -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,\
			     -8480,-8480,-8480,-8480,-8480,-8480,-8480,-8480};


void dft24(short *xre,short *yre, short *xim, short *yim) {
  
  __m128i *xre128=(__m128i *)xre;
  __m128i *yre128=(__m128i *)yre;
  __m128i *xim128=(__m128i *)xim;
  __m128i *yim128=(__m128i *)yim;
  __m128i *twr128=(__m128i *)&twr24[0];
  __m128i *twi128=(__m128i *)&twi24[0];
  __m128i ytmpre128array[24],ytmpim128array[24];
  __m128i *ytmpre128=&ytmpre128array[0],*ytmpim128=&ytmpim128array[0];
  int i,j,k;

  dft12(xre128,
	xre128+2,
	xre128+4,
	xre128+6,
	xre128+8,
	xre128+10,
	xre128+12,
	xre128+14,
	xre128+16,
	xre128+18,
	xre128+20,
	xre128+22,
	ytmpre128,
	ytmpre128+2,
	ytmpre128+4,
	ytmpre128+6,
	ytmpre128+8,
	ytmpre128+10,
	ytmpre128+12,
	ytmpre128+14,
	ytmpre128+16,
	ytmpre128+18,
	ytmpre128+20,
	ytmpre128+22,
	xim128,
	xim128+2,
	xim128+4,
	xim128+6,
	xim128+8,
	xim128+10,
	xim128+12,
	xim128+14,
	xim128+16,
	xim128+18,
	xim128+20,
	xim128+22,
	ytmpim128,
	ytmpim128+2,
	ytmpim128+4,
	ytmpim128+6,
	ytmpim128+8,
	ytmpim128+10,
	ytmpim128+12,
	ytmpim128+14,
	ytmpim128+16,
	ytmpim128+18,
	ytmpim128+20,
	ytmpim128+22);

  dft12(xre128+1,
	xre128+3,
	xre128+5,
	xre128+7,
	xre128+9,
	xre128+11,
	xre128+13,
	xre128+15,
	xre128+17,
	xre128+19,
	xre128+21,
	xre128+23,
	ytmpre128+1,
	ytmpre128+3,
	ytmpre128+5,
	ytmpre128+7,
	ytmpre128+9,
	ytmpre128+11,
	ytmpre128+13,
	ytmpre128+15,
	ytmpre128+17,
	ytmpre128+19,
	ytmpre128+21,
	ytmpre128+23,
	xim128+1,
	xim128+3,
	xim128+5,
	xim128+7,
	xim128+9,
	xim128+11,
	xim128+13,
	xim128+15,
	xim128+17,
	xim128+19,
	xim128+21,
	xim128+23,
	ytmpim128+1,
	ytmpim128+3,
	ytmpim128+5,
	ytmpim128+7,
	ytmpim128+9,
	ytmpim128+11,
	ytmpim128+13,
	ytmpim128+15,
	ytmpim128+17,
	ytmpim128+19,
	ytmpim128+21,
	ytmpim128+23);



  bfly2_tw1(ytmpre128, 
	    ytmpre128+1,
	    yre128, 
	    yre128+12,
	    ytmpim128, 
	    ytmpim128+1,
	    yim128, 
	    yim128+12);

  
  for (i=2,j=1,k=0;i<24;i+=2,j++,k++) {
    
    bfly2(ytmpre128+i, 
	  ytmpre128+i+1,
	  yre128+j, 
	  yre128+j+12,
	  twr128+k,
	  ytmpim128+i, 
	  ytmpim128+i+1,
	  yim128+j, 
	  yim128+j+12,
	  twi128+k);
  }
}


static short twra36[136]__attribute__((aligned(16))) = {32269,32269,32269,32269,32269,32269,32269,32269,
			   30790,30790,30790,30790,30790,30790,30790,30790,
			   28377,28377,28377,28377,28377,28377,28377,28377,
			   25100,25100,25100,25100,25100,25100,25100,25100,
			   21062,21062,21062,21062,21062,21062,21062,21062,
			   16383,16383,16383,16383,16383,16383,16383,16383,
			   11206,11206,11206,11206,11206,11206,11206,11206,
			   5689,5689,5689,5689,5689,5689,5689,5689,
			   0,0,0,0,0,0,0,0,
			   -5689,-5689,-5689,-5689,-5689,-5689,-5689,-5689,
			   -11206,-11206,-11206,-11206,-11206,-11206,-11206,-11206,
			   -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
			   -21062,-21062,-21062,-21062,-21062,-21062,-21062,-21062,
			   -25100,-25100,-25100,-25100,-25100,-25100,-25100,-25100,
			   -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
			   -30790,-30790,-30790,-30790,-30790,-30790,-30790,-30790,
			   -32269,-32269,-32269,-32269,-32269,-32269,-32269,-32269};

static short twia36[136]__attribute__((aligned(16))) = {-5689,-5689,-5689,-5689,-5689,-5689,-5689,-5689,
			   -11206,-11206,-11206,-11206,-11206,-11206,-11206,-11206,
			   -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
			   -21062,-21062,-21062,-21062,-21062,-21062,-21062,-21062,
			   -25100,-25100,-25100,-25100,-25100,-25100,-25100,-25100,
			   -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
			   -30790,-30790,-30790,-30790,-30790,-30790,-30790,-30790,
			   -32269,-32269,-32269,-32269,-32269,-32269,-32269,-32269,
			   -32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767,
			   -32269,-32269,-32269,-32269,-32269,-32269,-32269,-32269,
			   -30790,-30790,-30790,-30790,-30790,-30790,-30790,-30790,
			   -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
			   -25100,-25100,-25100,-25100,-25100,-25100,-25100,-25100,
			   -21062,-21062,-21062,-21062,-21062,-21062,-21062,-21062,
			   -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
			   -11206,-11206,-11206,-11206,-11206,-11206,-11206,-11206,
			   -5689,-5689,-5689,-5689,-5689,-5689,-5689,-5689};

static short twrb36[136]__attribute__((aligned(16))) = {30790,30790,30790,30790,30790,30790,30790,30790,
			    25100,25100,25100,25100,25100,25100,25100,25100,
			    16383,16383,16383,16383,16383,16383,16383,16383,
			    5689,5689,5689,5689,5689,5689,5689,5689,
			    -5689,-5689,-5689,-5689,-5689,-5689,-5689,-5689,
			    -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
			    -25100,-25100,-25100,-25100,-25100,-25100,-25100,-25100,
			    -30790,-30790,-30790,-30790,-30790,-30790,-30790,-30790,
			    -32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767,
			    -30790,-30790,-30790,-30790,-30790,-30790,-30790,-30790,
			    -25100,-25100,-25100,-25100,-25100,-25100,-25100,-25100,
			    -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
			    -5689,-5689,-5689,-5689,-5689,-5689,-5689,-5689,
			    5689,5689,5689,5689,5689,5689,5689,5689,
			    16383,16383,16383,16383,16383,16383,16383,16383,
			    25100,25100,25100,25100,25100,25100,25100,25100,
			    30790,30790,30790,30790,30790,30790,30790,30790};

static short twib36[136]__attribute__((aligned(16))) = {-11206,-11206,-11206,-11206,-11206,-11206,-11206,-11206,
			    -21062,-21062,-21062,-21062,-21062,-21062,-21062,-21062,
			    -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
			    -32269,-32269,-32269,-32269,-32269,-32269,-32269,-32269,
			    -32269,-32269,-32269,-32269,-32269,-32269,-32269,-32269,
			    -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
			    -21062,-21062,-21062,-21062,-21062,-21062,-21062,-21062,
			    -11206,-11206,-11206,-11206,-11206,-11206,-11206,-11206,
			    0,0,0,0,0,0,0,0,
			    11206,11206,11206,11206,11206,11206,11206,11206,
			    21062,21062,21062,21062,21062,21062,21062,21062,
			    28377,28377,28377,28377,28377,28377,28377,28377,
			    32269,32269,32269,32269,32269,32269,32269,32269,
			    32269,32269,32269,32269,32269,32269,32269,32269,
			    28377,28377,28377,28377,28377,28377,28377,28377,
			    21062,21062,21062,21062,21062,21062,21062,21062,
			    11206,11206,11206,11206,11206,11206,11206,11206};

void dft36(short *xre,short *yre, short *xim, short *yim) {
  
  __m128i *xre128=(__m128i *)xre;
  __m128i *yre128=(__m128i *)yre;
  __m128i *xim128=(__m128i *)xim;
  __m128i *yim128=(__m128i *)yim;
  __m128i *twra128=(__m128i *)&twra36[0];
  __m128i *twia128=(__m128i *)&twia36[0];
  __m128i *twrb128=(__m128i *)&twrb36[0];
  __m128i *twib128=(__m128i *)&twib36[0];
  __m128i ytmpre128array[36],ytmpim128array[36];
  __m128i *ytmpre128=&ytmpre128array[0],*ytmpim128=&ytmpim128array[0];
  int i,j,k;

  dft12(xre128,
	xre128+3,
	xre128+6,
	xre128+9,
	xre128+12,
	xre128+15,
	xre128+18,
	xre128+21,
	xre128+24,
	xre128+27,
	xre128+30,
	xre128+33,
	ytmpre128,
	ytmpre128+3,
	ytmpre128+6,
	ytmpre128+9,
	ytmpre128+12,
	ytmpre128+15,
	ytmpre128+18,
	ytmpre128+21,
	ytmpre128+24,
	ytmpre128+27,
	ytmpre128+30,
	ytmpre128+33,
	xim128,
	xim128+3,
	xim128+6,
	xim128+9,
	xim128+12,
	xim128+15,
	xim128+18,
	xim128+21,
	xim128+24,
	xim128+27,
	xim128+30,
	xim128+33,
	ytmpim128,
	ytmpim128+3,
	ytmpim128+6,
	ytmpim128+9,
	ytmpim128+12,
	ytmpim128+15,
	ytmpim128+18,
	ytmpim128+21,
	ytmpim128+24,
	ytmpim128+27,
	ytmpim128+30,
	ytmpim128+33);

  dft12(xre128+1,
	xre128+4,
	xre128+7,
	xre128+10,
	xre128+13,
	xre128+16,
	xre128+19,
	xre128+22,
	xre128+25,
	xre128+28,
	xre128+31,
	xre128+34,
	ytmpre128+1,
	ytmpre128+4,
	ytmpre128+7,
	ytmpre128+10,
	ytmpre128+13,
	ytmpre128+16,
	ytmpre128+19,
	ytmpre128+22,
	ytmpre128+25,
	ytmpre128+28,
	ytmpre128+31,
	ytmpre128+34,
	xim128+1,
	xim128+4,
	xim128+7,
	xim128+10,
	xim128+13,
	xim128+16,
	xim128+19,
	xim128+22,
	xim128+25,
	xim128+28,
	xim128+31,
	xim128+34,
	ytmpim128+1,
	ytmpim128+4,
	ytmpim128+7,
	ytmpim128+10,
	ytmpim128+13,
	ytmpim128+16,
	ytmpim128+19,
	ytmpim128+22,
	ytmpim128+25,
	ytmpim128+28,
	ytmpim128+31,
	ytmpim128+34);

  dft12(xre128+2,
	xre128+5,
	xre128+8,
	xre128+11,
	xre128+14,
	xre128+17,
	xre128+20,
	xre128+23,
	xre128+26,
	xre128+29,
	xre128+32,
	xre128+35,
	ytmpre128+2,
	ytmpre128+5,
	ytmpre128+8,
	ytmpre128+11,
	ytmpre128+14,
	ytmpre128+17,
	ytmpre128+20,
	ytmpre128+23,
	ytmpre128+26,
	ytmpre128+29,
	ytmpre128+32,
	ytmpre128+35,
	xim128+2,
	xim128+5,
	xim128+8,
	xim128+11,
	xim128+14,
	xim128+17,
	xim128+20,
	xim128+23,
	xim128+26,
	xim128+29,
	xim128+32,
	xim128+35,
	ytmpim128+2,
	ytmpim128+5,
	ytmpim128+8,
	ytmpim128+11,
	ytmpim128+14,
	ytmpim128+17,
	ytmpim128+20,
	ytmpim128+23,
	ytmpim128+26,
	ytmpim128+29,
	ytmpim128+32,
	ytmpim128+35);



  bfly3_tw1(ytmpre128, 
	    ytmpre128+1,
	    ytmpre128+2,
	    yre128, 
	    yre128+12,
	    yre128+24,
	    ytmpim128, 
	    ytmpim128+1,
	    ytmpim128+2,
	    yim128, 
	    yim128+12,
	    yim128+24);

  
  for (i=3,j=1,k=0;i<36;i+=3,j++,k++) {
    
    bfly3(ytmpre128+i, 
	  ytmpre128+i+1,
	  ytmpre128+i+2,
	  yre128+j, 
	  yre128+j+12,
	  yre128+j+24,
	  twra128+k,
	  twrb128+k,
	  ytmpim128+i, 
	  ytmpim128+i+1,
	  ytmpim128+i+2,
	  yim128+j, 
	  yim128+j+12,
	  yim128+j+24,
	  twia128+k,
	  twib128+k);
  }
}


static short twra48[184]__attribute__((aligned(16))) = {32486,32486,32486,32486,32486,32486,32486,32486,
			    31650,31650,31650,31650,31650,31650,31650,31650,
			    30272,30272,30272,30272,30272,30272,30272,30272,
			    28377,28377,28377,28377,28377,28377,28377,28377,
			    25995,25995,25995,25995,25995,25995,25995,25995,
			    23169,23169,23169,23169,23169,23169,23169,23169,
			    19947,19947,19947,19947,19947,19947,19947,19947,
			    16383,16383,16383,16383,16383,16383,16383,16383,
			    12539,12539,12539,12539,12539,12539,12539,12539,
			    8480,8480,8480,8480,8480,8480,8480,8480,
			    4276,4276,4276,4276,4276,4276,4276,4276,
			    0,0,0,0,0,0,0,0,
			    -4276,-4276,-4276,-4276,-4276,-4276,-4276,-4276,
			    -8480,-8480,-8480,-8480,-8480,-8480,-8480,-8480,
			    -12539,-12539,-12539,-12539,-12539,-12539,-12539,-12539,
			    -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
			    -19947,-19947,-19947,-19947,-19947,-19947,-19947,-19947,
			    -23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
			    -25995,-25995,-25995,-25995,-25995,-25995,-25995,-25995,
			    -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
			    -30272,-30272,-30272,-30272,-30272,-30272,-30272,-30272,
			    -31650,-31650,-31650,-31650,-31650,-31650,-31650,-31650,
			    -32486,-32486,-32486,-32486,-32486,-32486,-32486,-32486};

static short twia48[184]__attribute__((aligned(16))) = {-4276,-4276,-4276,-4276,-4276,-4276,-4276,-4276,
			    -8480,-8480,-8480,-8480,-8480,-8480,-8480,-8480,
			    -12539,-12539,-12539,-12539,-12539,-12539,-12539,-12539,
			    -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
			    -19947,-19947,-19947,-19947,-19947,-19947,-19947,-19947,
			    -23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
			    -25995,-25995,-25995,-25995,-25995,-25995,-25995,-25995,
			    -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
			    -30272,-30272,-30272,-30272,-30272,-30272,-30272,-30272,
			    -31650,-31650,-31650,-31650,-31650,-31650,-31650,-31650,
			    -32486,-32486,-32486,-32486,-32486,-32486,-32486,-32486,
			    -32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767,
			    -32486,-32486,-32486,-32486,-32486,-32486,-32486,-32486,
			    -31650,-31650,-31650,-31650,-31650,-31650,-31650,-31650,
			    -30272,-30272,-30272,-30272,-30272,-30272,-30272,-30272,
			    -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
			    -25995,-25995,-25995,-25995,-25995,-25995,-25995,-25995,
			    -23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
			    -19947,-19947,-19947,-19947,-19947,-19947,-19947,-19947,
			    -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
			    -12539,-12539,-12539,-12539,-12539,-12539,-12539,-12539,
			    -8480,-8480,-8480,-8480,-8480,-8480,-8480,-8480,
			    -4276,-4276,-4276,-4276,-4276,-4276,-4276,-4276};

static short twrb48[184]__attribute__((aligned(16))) = {31650,31650,31650,31650,31650,31650,31650,31650,
			    28377,28377,28377,28377,28377,28377,28377,28377,
			    23169,23169,23169,23169,23169,23169,23169,23169,
			    16383,16383,16383,16383,16383,16383,16383,16383,
			    8480,8480,8480,8480,8480,8480,8480,8480,
			    0,0,0,0,0,0,0,0,
			    -8480,-8480,-8480,-8480,-8480,-8480,-8480,-8480,
			    -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
			    -23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
			    -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
			    -31650,-31650,-31650,-31650,-31650,-31650,-31650,-31650,
			    -32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767,
			    -31650,-31650,-31650,-31650,-31650,-31650,-31650,-31650,
			    -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
			    -23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
			    -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
			    -8480,-8480,-8480,-8480,-8480,-8480,-8480,-8480,
			    0,0,0,0,0,0,0,0,
			    8480,8480,8480,8480,8480,8480,8480,8480,
			    16383,16383,16383,16383,16383,16383,16383,16383,
			    23169,23169,23169,23169,23169,23169,23169,23169,
			    28377,28377,28377,28377,28377,28377,28377,28377,
			    31650,31650,31650,31650,31650,31650,31650,31650};

static short twib48[184]__attribute__((aligned(16))) = {-8480,-8480,-8480,-8480,-8480,-8480,-8480,-8480,
			    -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
			    -23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
			    -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
			    -31650,-31650,-31650,-31650,-31650,-31650,-31650,-31650,
			    -32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767,
			    -31650,-31650,-31650,-31650,-31650,-31650,-31650,-31650,
			    -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
			    -23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
			    -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
			    -8480,-8480,-8480,-8480,-8480,-8480,-8480,-8480,
			    0,0,0,0,0,0,0,0,
			    8480,8480,8480,8480,8480,8480,8480,8480,
			    16383,16383,16383,16383,16383,16383,16383,16383,
			    23169,23169,23169,23169,23169,23169,23169,23169,
			    28377,28377,28377,28377,28377,28377,28377,28377,
			    31650,31650,31650,31650,31650,31650,31650,31650,
			    32767,32767,32767,32767,32767,32767,32767,32767,
			    31650,31650,31650,31650,31650,31650,31650,31650,
			    28377,28377,28377,28377,28377,28377,28377,28377,
			    23169,23169,23169,23169,23169,23169,23169,23169,
			    16383,16383,16383,16383,16383,16383,16383,16383,
			    8480,8480,8480,8480,8480,8480,8480,8480};

static short twrc48[184]__attribute__((aligned(16))) = {30272,30272,30272,30272,30272,30272,30272,30272,
			    23169,23169,23169,23169,23169,23169,23169,23169,
			    12539,12539,12539,12539,12539,12539,12539,12539,
			    0,0,0,0,0,0,0,0,
			    -12539,-12539,-12539,-12539,-12539,-12539,-12539,-12539,
			    -23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
			    -30272,-30272,-30272,-30272,-30272,-30272,-30272,-30272,
			    -32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767,
			    -30272,-30272,-30272,-30272,-30272,-30272,-30272,-30272,
			    -23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
			    -12539,-12539,-12539,-12539,-12539,-12539,-12539,-12539,
			    0,0,0,0,0,0,0,0,
			    12539,12539,12539,12539,12539,12539,12539,12539,
			    23169,23169,23169,23169,23169,23169,23169,23169,
			    30272,30272,30272,30272,30272,30272,30272,30272,
			    32767,32767,32767,32767,32767,32767,32767,32767,
			    30272,30272,30272,30272,30272,30272,30272,30272,
			    23169,23169,23169,23169,23169,23169,23169,23169,
			    12539,12539,12539,12539,12539,12539,12539,12539,
			    0,0,0,0,0,0,0,0,
			    -12539,-12539,-12539,-12539,-12539,-12539,-12539,-12539,
			    -23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
			    -30272,-30272,-30272,-30272,-30272,-30272,-30272,-30272};

static short twic48[184]__attribute__((aligned(16))) = {-12539,-12539,-12539,-12539,-12539,-12539,-12539,-12539,
			    -23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
			    -30272,-30272,-30272,-30272,-30272,-30272,-30272,-30272,
			    -32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767,
			    -30272,-30272,-30272,-30272,-30272,-30272,-30272,-30272,
			    -23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
			    -12539,-12539,-12539,-12539,-12539,-12539,-12539,-12539,
			    0,0,0,0,0,0,0,0,
			    12539,12539,12539,12539,12539,12539,12539,12539,
			    23169,23169,23169,23169,23169,23169,23169,23169,
			    30272,30272,30272,30272,30272,30272,30272,30272,
			    32767,32767,32767,32767,32767,32767,32767,32767,
			    30272,30272,30272,30272,30272,30272,30272,30272,
			    23169,23169,23169,23169,23169,23169,23169,23169,
			    12539,12539,12539,12539,12539,12539,12539,12539,
			    0,0,0,0,0,0,0,0,
			    -12539,-12539,-12539,-12539,-12539,-12539,-12539,-12539,
			    -23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
			    -30272,-30272,-30272,-30272,-30272,-30272,-30272,-30272,
			    -32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767,
			    -30272,-30272,-30272,-30272,-30272,-30272,-30272,-30272,
			    -23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
			    -12539,-12539,-12539,-12539,-12539,-12539,-12539,-12539};

void dft48(short *xre,short *yre, short *xim, short *yim) {
  
  __m128i *xre128=(__m128i *)xre;
  __m128i *yre128=(__m128i *)yre;
  __m128i *xim128=(__m128i *)xim;
  __m128i *yim128=(__m128i *)yim;
  __m128i *twra128=(__m128i *)&twra48[0];
  __m128i *twia128=(__m128i *)&twia48[0];
  __m128i *twrb128=(__m128i *)&twrb48[0];
  __m128i *twib128=(__m128i *)&twib48[0];
  __m128i *twrc128=(__m128i *)&twrc48[0];
  __m128i *twic128=(__m128i *)&twic48[0];
  __m128i ytmpre128array[48],ytmpim128array[48];
  __m128i *ytmpre128=&ytmpre128array[0],*ytmpim128=&ytmpim128array[0];
  int i,j,k;

  dft12(xre128,
	xre128+4,
	xre128+8,
	xre128+12,
	xre128+16,
	xre128+20,
	xre128+24,
	xre128+28,
	xre128+32,
	xre128+36,
	xre128+40,
	xre128+44,
	ytmpre128,
	ytmpre128+4,
	ytmpre128+8,
	ytmpre128+12,
	ytmpre128+16,
	ytmpre128+20,
	ytmpre128+24,
	ytmpre128+28,
	ytmpre128+32,
	ytmpre128+36,
	ytmpre128+40,
	ytmpre128+44,
	xim128,
	xim128+4,
	xim128+8,
	xim128+12,
	xim128+16,
	xim128+20,
	xim128+24,
	xim128+28,
	xim128+32,
	xim128+36,
	xim128+40,
	xim128+44,
	ytmpim128,
	ytmpim128+4,
	ytmpim128+8,
	ytmpim128+12,
	ytmpim128+16,
	ytmpim128+20,
	ytmpim128+24,
	ytmpim128+28,
	ytmpim128+32,
	ytmpim128+36,
	ytmpim128+40,
	ytmpim128+44);

  dft12(xre128+1,
	xre128+5,
	xre128+9,
	xre128+13,
	xre128+17,
	xre128+21,
	xre128+25,
	xre128+29,
	xre128+33,
	xre128+37,
	xre128+41,
	xre128+45,
	ytmpre128+1,
	ytmpre128+5,
	ytmpre128+9,
	ytmpre128+13,
	ytmpre128+17,
	ytmpre128+21,
	ytmpre128+25,
	ytmpre128+29,
	ytmpre128+33,
	ytmpre128+37,
	ytmpre128+41,
	ytmpre128+45,
	xim128+1,
	xim128+5,
	xim128+9,
	xim128+13,
	xim128+17,
	xim128+21,
	xim128+25,
	xim128+29,
	xim128+33,
	xim128+37,
	xim128+41,
	xim128+44,
	ytmpim128+1,
	ytmpim128+5,
	ytmpim128+9,
	ytmpim128+13,
	ytmpim128+17,
	ytmpim128+21,
	ytmpim128+25,
	ytmpim128+29,
	ytmpim128+33,
	ytmpim128+37,
	ytmpim128+41,
	ytmpim128+45);

  dft12(xre128+2,
	xre128+6,
	xre128+10,
	xre128+14,
	xre128+18,
	xre128+22,
	xre128+26,
	xre128+30,
	xre128+34,
	xre128+38,
	xre128+42,
	xre128+46,
	ytmpre128+2,
	ytmpre128+6,
	ytmpre128+10,
	ytmpre128+14,
	ytmpre128+18,
	ytmpre128+22,
	ytmpre128+26,
	ytmpre128+30,
	ytmpre128+34,
	ytmpre128+38,
	ytmpre128+42,
	ytmpre128+46,
	xim128+2,
	xim128+6,
	xim128+10,
	xim128+14,
	xim128+18,
	xim128+22,
	xim128+26,
	xim128+30,
	xim128+34,
	xim128+38,
	xim128+42,
	xim128+46,
	ytmpim128+2,
	ytmpim128+6,
	ytmpim128+10,
	ytmpim128+14,
	ytmpim128+18,
	ytmpim128+22,
	ytmpim128+26,
	ytmpim128+30,
	ytmpim128+34,
	ytmpim128+38,
	ytmpim128+42,
	ytmpim128+46);

  dft12(xre128+3,
	xre128+7,
	xre128+11,
	xre128+15,
	xre128+19,
	xre128+23,
	xre128+27,
	xre128+31,
	xre128+35,
	xre128+39,
	xre128+43,
	xre128+47,
	ytmpre128+3,
	ytmpre128+7,
	ytmpre128+11,
	ytmpre128+15,
	ytmpre128+19,
	ytmpre128+23,
	ytmpre128+27,
	ytmpre128+31,
	ytmpre128+35,
	ytmpre128+39,
	ytmpre128+43,
	ytmpre128+47,
	xim128+3,
	xim128+7,
	xim128+11,
	xim128+15,
	xim128+19,
	xim128+23,
	xim128+27,
	xim128+31,
	xim128+35,
	xim128+39,
	xim128+43,
	xim128+47,
	ytmpim128+3,
	ytmpim128+7,
	ytmpim128+11,
	ytmpim128+15,
	ytmpim128+19,
	ytmpim128+23,
	ytmpim128+27,
	ytmpim128+31,
	ytmpim128+35,
	ytmpim128+39,
	ytmpim128+43,
	ytmpim128+47);


  bfly4_tw1(ytmpre128, 
	    ytmpre128+1,
	    ytmpre128+2,
	    ytmpre128+3,
	    yre128, 
	    yre128+12,
	    yre128+24,
	    yre128+36,
	    ytmpim128, 
	    ytmpim128+1,
	    ytmpim128+2,
	    ytmpim128+3,
	    yim128, 
	    yim128+12,
	    yim128+24,
	    yim128+36);

  
  for (i=4,j=1,k=0;i<48;i+=4,j++,k++) {
    
    bfly4(ytmpre128+i, 
	  ytmpre128+i+1,
	  ytmpre128+i+2,
	  ytmpre128+i+3,
	  yre128+j, 
	  yre128+j+12,
	  yre128+j+24,
	  yre128+j+36,
	  twra128+k,
	  twrb128+k,
	  twrc128+k,
	  ytmpim128+i, 
	  ytmpim128+i+1,
	  ytmpim128+i+2,
	  ytmpim128+i+3,
	  yim128+j, 
	  yim128+j+12,
	  yim128+j+24,
	  yim128+j+48,
	  twia128+k,
	  twib128+k,
	  twic128+k);
  }
}


static short twra60[232]__attribute__((aligned(16))) = {32587,32587,32587,32587,32587,32587,32587,32587,
			    32050,32050,32050,32050,32050,32050,32050,32050,
			    31163,31163,31163,31163,31163,31163,31163,31163,
			    29934,29934,29934,29934,29934,29934,29934,29934,
			    28377,28377,28377,28377,28377,28377,28377,28377,
			    26509,26509,26509,26509,26509,26509,26509,26509,
			    24350,24350,24350,24350,24350,24350,24350,24350,
			    21925,21925,21925,21925,21925,21925,21925,21925,
			    19259,19259,19259,19259,19259,19259,19259,19259,
			    16383,16383,16383,16383,16383,16383,16383,16383,
			    13327,13327,13327,13327,13327,13327,13327,13327,
			    10125,10125,10125,10125,10125,10125,10125,10125,
			    6812,6812,6812,6812,6812,6812,6812,6812,
			    3425,3425,3425,3425,3425,3425,3425,3425,
			    0,0,0,0,0,0,0,0,
			    -3425,-3425,-3425,-3425,-3425,-3425,-3425,-3425,
			    -6812,-6812,-6812,-6812,-6812,-6812,-6812,-6812,
			    -10125,-10125,-10125,-10125,-10125,-10125,-10125,-10125,
			    -13327,-13327,-13327,-13327,-13327,-13327,-13327,-13327,
			    -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
			    -19259,-19259,-19259,-19259,-19259,-19259,-19259,-19259,
			    -21925,-21925,-21925,-21925,-21925,-21925,-21925,-21925,
			    -24350,-24350,-24350,-24350,-24350,-24350,-24350,-24350,
			    -26509,-26509,-26509,-26509,-26509,-26509,-26509,-26509,
			    -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
			    -29934,-29934,-29934,-29934,-29934,-29934,-29934,-29934,
			    -31163,-31163,-31163,-31163,-31163,-31163,-31163,-31163,
			    -32050,-32050,-32050,-32050,-32050,-32050,-32050,-32050,
			    -32587,-32587,-32587,-32587,-32587,-32587,-32587,-32587
};

static short twia60[232]__attribute__((aligned(16))) = {-3425,-3425,-3425,-3425,-3425,-3425,-3425,-3425,
			    -6812,-6812,-6812,-6812,-6812,-6812,-6812,-6812,
			    -10125,-10125,-10125,-10125,-10125,-10125,-10125,-10125,
			    -13327,-13327,-13327,-13327,-13327,-13327,-13327,-13327,
			    -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
			    -19259,-19259,-19259,-19259,-19259,-19259,-19259,-19259,
			    -21925,-21925,-21925,-21925,-21925,-21925,-21925,-21925,
			    -24350,-24350,-24350,-24350,-24350,-24350,-24350,-24350,
			    -26509,-26509,-26509,-26509,-26509,-26509,-26509,-26509,
			    -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
			    -29934,-29934,-29934,-29934,-29934,-29934,-29934,-29934,
			    -31163,-31163,-31163,-31163,-31163,-31163,-31163,-31163,
			    -32050,-32050,-32050,-32050,-32050,-32050,-32050,-32050,
			    -32587,-32587,-32587,-32587,-32587,-32587,-32587,-32587,
			    -32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767,
			    -32587,-32587,-32587,-32587,-32587,-32587,-32587,-32587,
			    -32050,-32050,-32050,-32050,-32050,-32050,-32050,-32050,
			    -31163,-31163,-31163,-31163,-31163,-31163,-31163,-31163,
			    -29934,-29934,-29934,-29934,-29934,-29934,-29934,-29934,
			    -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
			    -26509,-26509,-26509,-26509,-26509,-26509,-26509,-26509,
			    -24350,-24350,-24350,-24350,-24350,-24350,-24350,-24350,
			    -21925,-21925,-21925,-21925,-21925,-21925,-21925,-21925,
			    -19259,-19259,-19259,-19259,-19259,-19259,-19259,-19259,
			    -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
			    -13327,-13327,-13327,-13327,-13327,-13327,-13327,-13327,
			    -10125,-10125,-10125,-10125,-10125,-10125,-10125,-10125,
			    -6812,-6812,-6812,-6812,-6812,-6812,-6812,-6812,
			    -3425,-3425,-3425,-3425,-3425,-3425,-3425,-3425
};

static short twrb60[232]__attribute__((aligned(16))) = {32050,32050,32050,32050,32050,32050,32050,32050,
			    29934,29934,29934,29934,29934,29934,29934,29934,
			    26509,26509,26509,26509,26509,26509,26509,26509,
			    21925,21925,21925,21925,21925,21925,21925,21925,
			    16383,16383,16383,16383,16383,16383,16383,16383,
			    10125,10125,10125,10125,10125,10125,10125,10125,
			    3425,3425,3425,3425,3425,3425,3425,3425,
			    -3425,-3425,-3425,-3425,-3425,-3425,-3425,-3425,
			    -10125,-10125,-10125,-10125,-10125,-10125,-10125,-10125,
			    -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
			    -21925,-21925,-21925,-21925,-21925,-21925,-21925,-21925,
			    -26509,-26509,-26509,-26509,-26509,-26509,-26509,-26509,
			    -29934,-29934,-29934,-29934,-29934,-29934,-29934,-29934,
			    -32050,-32050,-32050,-32050,-32050,-32050,-32050,-32050,
			    -32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767,
			    -32050,-32050,-32050,-32050,-32050,-32050,-32050,-32050,
			    -29934,-29934,-29934,-29934,-29934,-29934,-29934,-29934,
			    -26509,-26509,-26509,-26509,-26509,-26509,-26509,-26509,
			    -21925,-21925,-21925,-21925,-21925,-21925,-21925,-21925,
			    -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
			    -10125,-10125,-10125,-10125,-10125,-10125,-10125,-10125,
			    -3425,-3425,-3425,-3425,-3425,-3425,-3425,-3425,
			    3425,3425,3425,3425,3425,3425,3425,3425,
			    10125,10125,10125,10125,10125,10125,10125,10125,
			    16383,16383,16383,16383,16383,16383,16383,16383,
			    21925,21925,21925,21925,21925,21925,21925,21925,
			    26509,26509,26509,26509,26509,26509,26509,26509,
			    29934,29934,29934,29934,29934,29934,29934,29934,
			    32050,32050,32050,32050,32050,32050,32050,32050
};

static short twib60[232]__attribute__((aligned(16))) = {-6812,-6812,-6812,-6812,-6812,-6812,-6812,-6812,
			    -13327,-13327,-13327,-13327,-13327,-13327,-13327,-13327,
			    -19259,-19259,-19259,-19259,-19259,-19259,-19259,-19259,
			    -24350,-24350,-24350,-24350,-24350,-24350,-24350,-24350,
			    -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
			    -31163,-31163,-31163,-31163,-31163,-31163,-31163,-31163,
			    -32587,-32587,-32587,-32587,-32587,-32587,-32587,-32587,
			    -32587,-32587,-32587,-32587,-32587,-32587,-32587,-32587,
			    -31163,-31163,-31163,-31163,-31163,-31163,-31163,-31163,
			    -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
			    -24350,-24350,-24350,-24350,-24350,-24350,-24350,-24350,
			    -19259,-19259,-19259,-19259,-19259,-19259,-19259,-19259,
			    -13327,-13327,-13327,-13327,-13327,-13327,-13327,-13327,
			    -6812,-6812,-6812,-6812,-6812,-6812,-6812,-6812,
			    0,0,0,0,0,0,0,0,
			    6812,6812,6812,6812,6812,6812,6812,6812,
			    13327,13327,13327,13327,13327,13327,13327,13327,
			    19259,19259,19259,19259,19259,19259,19259,19259,
			    24350,24350,24350,24350,24350,24350,24350,24350,
			    28377,28377,28377,28377,28377,28377,28377,28377,
			    31163,31163,31163,31163,31163,31163,31163,31163,
			    32587,32587,32587,32587,32587,32587,32587,32587,
			    32587,32587,32587,32587,32587,32587,32587,32587,
			    31163,31163,31163,31163,31163,31163,31163,31163,
			    28377,28377,28377,28377,28377,28377,28377,28377,
			    24350,24350,24350,24350,24350,24350,24350,24350,
			    19259,19259,19259,19259,19259,19259,19259,19259,
			    13327,13327,13327,13327,13327,13327,13327,13327,
			    6812,6812,6812,6812,6812,6812,6812,6812
};

static short twrc60[232]__attribute__((aligned(16))) = {32050,32050,32050,32050,32050,32050,32050,32050,
			    29934,29934,29934,29934,29934,29934,29934,29934,
			    26509,26509,26509,26509,26509,26509,26509,26509,
			    21925,21925,21925,21925,21925,21925,21925,21925,
			    16383,16383,16383,16383,16383,16383,16383,16383,
			    10125,10125,10125,10125,10125,10125,10125,10125,
			    3425,3425,3425,3425,3425,3425,3425,3425,
			    -3425,-3425,-3425,-3425,-3425,-3425,-3425,-3425,
			    -10125,-10125,-10125,-10125,-10125,-10125,-10125,-10125,
			    -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
			    -21925,-21925,-21925,-21925,-21925,-21925,-21925,-21925,
			    -26509,-26509,-26509,-26509,-26509,-26509,-26509,-26509,
			    -29934,-29934,-29934,-29934,-29934,-29934,-29934,-29934,
			    -32050,-32050,-32050,-32050,-32050,-32050,-32050,-32050,
			    -32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767,
			    -32050,-32050,-32050,-32050,-32050,-32050,-32050,-32050,
			    -29934,-29934,-29934,-29934,-29934,-29934,-29934,-29934,
			    -26509,-26509,-26509,-26509,-26509,-26509,-26509,-26509,
			    -21925,-21925,-21925,-21925,-21925,-21925,-21925,-21925,
			    -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
			    -10125,-10125,-10125,-10125,-10125,-10125,-10125,-10125,
			    -3425,-3425,-3425,-3425,-3425,-3425,-3425,-3425,
			    3425,3425,3425,3425,3425,3425,3425,3425,
			    10125,10125,10125,10125,10125,10125,10125,10125,
			    16383,16383,16383,16383,16383,16383,16383,16383,
			    21925,21925,21925,21925,21925,21925,21925,21925,
			    26509,26509,26509,26509,26509,26509,26509,26509,
			    29934,29934,29934,29934,29934,29934,29934,29934,
			    32050,32050,32050,32050,32050,32050,32050,32050
};

static short twic60[232]__attribute__((aligned(16))) = {-6812,-6812,-6812,-6812,-6812,-6812,-6812,-6812,
			    -13327,-13327,-13327,-13327,-13327,-13327,-13327,-13327,
			    -19259,-19259,-19259,-19259,-19259,-19259,-19259,-19259,
			    -24350,-24350,-24350,-24350,-24350,-24350,-24350,-24350,
			    -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
			    -31163,-31163,-31163,-31163,-31163,-31163,-31163,-31163,
			    -32587,-32587,-32587,-32587,-32587,-32587,-32587,-32587,
			    -32587,-32587,-32587,-32587,-32587,-32587,-32587,-32587,
			    -31163,-31163,-31163,-31163,-31163,-31163,-31163,-31163,
			    -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
			    -24350,-24350,-24350,-24350,-24350,-24350,-24350,-24350,
			    -19259,-19259,-19259,-19259,-19259,-19259,-19259,-19259,
			    -13327,-13327,-13327,-13327,-13327,-13327,-13327,-13327,
			    -6812,-6812,-6812,-6812,-6812,-6812,-6812,-6812,
			    0,0,0,0,0,0,0,0,
			    6812,6812,6812,6812,6812,6812,6812,6812,
			    13327,13327,13327,13327,13327,13327,13327,13327,
			    19259,19259,19259,19259,19259,19259,19259,19259,
			    24350,24350,24350,24350,24350,24350,24350,24350,
			    28377,28377,28377,28377,28377,28377,28377,28377,
			    31163,31163,31163,31163,31163,31163,31163,31163,
			    32587,32587,32587,32587,32587,32587,32587,32587,
			    32587,32587,32587,32587,32587,32587,32587,32587,
			    31163,31163,31163,31163,31163,31163,31163,31163,
			    28377,28377,28377,28377,28377,28377,28377,28377,
			    24350,24350,24350,24350,24350,24350,24350,24350,
			    19259,19259,19259,19259,19259,19259,19259,19259,
			    13327,13327,13327,13327,13327,13327,13327,13327,
			    6812,6812,6812,6812,6812,6812,6812,6812
};

static short twrd60[232]__attribute__((aligned(16))) = {31163,31163,31163,31163,31163,31163,31163,31163,
			    26509,26509,26509,26509,26509,26509,26509,26509,
			    19259,19259,19259,19259,19259,19259,19259,19259,
			    10125,10125,10125,10125,10125,10125,10125,10125,
			    0,0,0,0,0,0,0,0,
			    -10125,-10125,-10125,-10125,-10125,-10125,-10125,-10125,
			    -19259,-19259,-19259,-19259,-19259,-19259,-19259,-19259,
			    -26509,-26509,-26509,-26509,-26509,-26509,-26509,-26509,
			    -31163,-31163,-31163,-31163,-31163,-31163,-31163,-31163,
			    -32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767,
			    -31163,-31163,-31163,-31163,-31163,-31163,-31163,-31163,
			    -26509,-26509,-26509,-26509,-26509,-26509,-26509,-26509,
			    -19259,-19259,-19259,-19259,-19259,-19259,-19259,-19259,
			    -10125,-10125,-10125,-10125,-10125,-10125,-10125,-10125,
			    0,0,0,0,0,0,0,0,
			    10125,10125,10125,10125,10125,10125,10125,10125,
			    19259,19259,19259,19259,19259,19259,19259,19259,
			    26509,26509,26509,26509,26509,26509,26509,26509,
			    31163,31163,31163,31163,31163,31163,31163,31163,
			    32767,32767,32767,32767,32767,32767,32767,32767,
			    31163,31163,31163,31163,31163,31163,31163,31163,
			    26509,26509,26509,26509,26509,26509,26509,26509,
			    19259,19259,19259,19259,19259,19259,19259,19259,
			    10125,10125,10125,10125,10125,10125,10125,10125,
			    0,0,0,0,0,0,0,0,
			    -10125,-10125,-10125,-10125,-10125,-10125,-10125,-10125,
			    -19259,-19259,-19259,-19259,-19259,-19259,-19259,-19259,
			    -26509,-26509,-26509,-26509,-26509,-26509,-26509,-26509,
			    -31163,-31163,-31163,-31163,-31163,-31163,-31163,-31163
};

static short twid60[232]__attribute__((aligned(16))) = {-10125,-10125,-10125,-10125,-10125,-10125,-10125,-10125,
			    -19259,-19259,-19259,-19259,-19259,-19259,-19259,-19259,
			    -26509,-26509,-26509,-26509,-26509,-26509,-26509,-26509,
			    -31163,-31163,-31163,-31163,-31163,-31163,-31163,-31163,
			    -32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767,
			    -31163,-31163,-31163,-31163,-31163,-31163,-31163,-31163,
			    -26509,-26509,-26509,-26509,-26509,-26509,-26509,-26509,
			    -19259,-19259,-19259,-19259,-19259,-19259,-19259,-19259,
			    -10125,-10125,-10125,-10125,-10125,-10125,-10125,-10125,
			    0,0,0,0,0,0,0,0,
			    10125,10125,10125,10125,10125,10125,10125,10125,
			    19259,19259,19259,19259,19259,19259,19259,19259,
			    26509,26509,26509,26509,26509,26509,26509,26509,
			    31163,31163,31163,31163,31163,31163,31163,31163,
			    32767,32767,32767,32767,32767,32767,32767,32767,
			    31163,31163,31163,31163,31163,31163,31163,31163,
			    26509,26509,26509,26509,26509,26509,26509,26509,
			    19259,19259,19259,19259,19259,19259,19259,19259,
			    10125,10125,10125,10125,10125,10125,10125,10125,
			    0,0,0,0,0,0,0,0,
			    -10125,-10125,-10125,-10125,-10125,-10125,-10125,-10125,
			    -19259,-19259,-19259,-19259,-19259,-19259,-19259,-19259,
			    -26509,-26509,-26509,-26509,-26509,-26509,-26509,-26509,
			    -31163,-31163,-31163,-31163,-31163,-31163,-31163,-31163,
			    -32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767,
			    -31163,-31163,-31163,-31163,-31163,-31163,-31163,-31163,
			    -26509,-26509,-26509,-26509,-26509,-26509,-26509,-26509,
			    -19259,-19259,-19259,-19259,-19259,-19259,-19259,-19259,
			    -10125,-10125,-10125,-10125,-10125,-10125,-10125,-10125
};


void dft60(short *xre,short *yre, short *xim, short *yim) {
  
  __m128i *xre128=(__m128i *)xre;
  __m128i *yre128=(__m128i *)yre;
  __m128i *xim128=(__m128i *)xim;
  __m128i *yim128=(__m128i *)yim;
  __m128i *twra128=(__m128i *)&twra60[0];
  __m128i *twia128=(__m128i *)&twia60[0];
  __m128i *twrb128=(__m128i *)&twrb60[0];
  __m128i *twib128=(__m128i *)&twib60[0];
  __m128i *twrc128=(__m128i *)&twrc60[0];
  __m128i *twic128=(__m128i *)&twic60[0];
  __m128i *twrd128=(__m128i *)&twrd60[0];
  __m128i *twid128=(__m128i *)&twid60[0];
  __m128i ytmpre128array[60],ytmpim128array[60];
  __m128i *ytmpre128=&ytmpre128array[0],*ytmpim128=&ytmpim128array[0];
  int i,j,k;

  dft12(xre128,
	xre128+3,
	xre128+6,
	xre128+9,
	xre128+12,
	xre128+15,
	xre128+18,
	xre128+21,
	xre128+24,
	xre128+27,
	xre128+30,
	xre128+33,
	ytmpre128,
	ytmpre128+3,
	ytmpre128+6,
	ytmpre128+9,
	ytmpre128+12,
	ytmpre128+15,
	ytmpre128+18,
	ytmpre128+21,
	ytmpre128+24,
	ytmpre128+27,
	ytmpre128+30,
	ytmpre128+33,
	xim128,
	xim128+3,
	xim128+6,
	xim128+9,
	xim128+12,
	xim128+15,
	xim128+18,
	xim128+21,
	xim128+24,
	xim128+27,
	xim128+30,
	xim128+33,
	ytmpim128,
	ytmpim128+3,
	ytmpim128+6,
	ytmpim128+9,
	ytmpim128+12,
	ytmpim128+15,
	ytmpim128+18,
	ytmpim128+21,
	ytmpim128+24,
	ytmpim128+27,
	ytmpim128+30,
	ytmpim128+33);

  dft12(xre128+1,
	xre128+4,
	xre128+7,
	xre128+10,
	xre128+13,
	xre128+16,
	xre128+19,
	xre128+22,
	xre128+25,
	xre128+28,
	xre128+31,
	xre128+34,
	ytmpre128+1,
	ytmpre128+4,
	ytmpre128+7,
	ytmpre128+10,
	ytmpre128+13,
	ytmpre128+16,
	ytmpre128+19,
	ytmpre128+22,
	ytmpre128+25,
	ytmpre128+28,
	ytmpre128+31,
	ytmpre128+34,
	xim128+1,
	xim128+4,
	xim128+7,
	xim128+10,
	xim128+13,
	xim128+16,
	xim128+19,
	xim128+22,
	xim128+25,
	xim128+28,
	xim128+31,
	xim128+34,
	ytmpim128+1,
	ytmpim128+4,
	ytmpim128+7,
	ytmpim128+10,
	ytmpim128+13,
	ytmpim128+16,
	ytmpim128+19,
	ytmpim128+22,
	ytmpim128+25,
	ytmpim128+28,
	ytmpim128+31,
	ytmpim128+34);

  dft12(xre128+2,
	xre128+5,
	xre128+8,
	xre128+11,
	xre128+14,
	xre128+17,
	xre128+20,
	xre128+23,
	xre128+26,
	xre128+29,
	xre128+32,
	xre128+35,
	ytmpre128+2,
	ytmpre128+5,
	ytmpre128+8,
	ytmpre128+11,
	ytmpre128+14,
	ytmpre128+17,
	ytmpre128+20,
	ytmpre128+23,
	ytmpre128+26,
	ytmpre128+29,
	ytmpre128+32,
	ytmpre128+35,
	xim128+2,
	xim128+5,
	xim128+8,
	xim128+11,
	xim128+14,
	xim128+17,
	xim128+20,
	xim128+23,
	xim128+26,
	xim128+29,
	xim128+32,
	xim128+35,
	ytmpim128+2,
	ytmpim128+5,
	ytmpim128+8,
	ytmpim128+11,
	ytmpim128+14,
	ytmpim128+17,
	ytmpim128+20,
	ytmpim128+23,
	ytmpim128+26,
	ytmpim128+29,
	ytmpim128+32,
	ytmpim128+35);



  bfly5_tw1(ytmpre128, 
	    ytmpre128+1,
	    ytmpre128+2,
	    ytmpre128+3,
	    ytmpre128+4,
	    yre128, 
	    yre128+12,
	    yre128+24,
	    yre128+36,
	    yre128+48,
	    ytmpim128, 
	    ytmpim128+1,
	    ytmpim128+2,
	    ytmpim128+3,
	    ytmpim128+4,
	    yim128, 
	    yim128+12,
	    yim128+24,
	    yim128+36,
	    yim128+48);

  
  for (i=5,j=1,k=0;i<36;i+=5,j++,k++) {
    
    bfly5(ytmpre128+i, 
	  ytmpre128+i+1,
	  ytmpre128+i+2,
	  ytmpre128+i+3,
	  ytmpre128+i+4,
	  yre128+j, 
	  yre128+j+12,
	  yre128+j+24,
	  yre128+j+36,
	  yre128+j+48,
	  twra128+k,
	  twrb128+k,
	  twrc128+k,
	  twrd128+k,
	  ytmpim128+i, 
	  ytmpim128+i+1,
	  ytmpim128+i+2,
	  ytmpim128+i+3,
	  ytmpim128+i+4,
	  yim128+j, 
	  yim128+j+12,
	  yim128+j+24,
	  yim128+j+36,
	  yim128+j+48,
	  twia128+k,
	  twib128+k,
	  twic128+k,
	  twid128+k);
  }
}

short twr72[280]__attribute__((aligned(16))) = {32642,32642,32642,32642,32642,32642,32642,32642,
						 32269,32269,32269,32269,32269,32269,32269,32269,
						 31650,31650,31650,31650,31650,31650,31650,31650,
						 30790,30790,30790,30790,30790,30790,30790,30790,
						 29696,29696,29696,29696,29696,29696,29696,29696,
						 28377,28377,28377,28377,28377,28377,28377,28377,
						 26841,26841,26841,26841,26841,26841,26841,26841,
						 25100,25100,25100,25100,25100,25100,25100,25100,
						 23169,23169,23169,23169,23169,23169,23169,23169,
						 21062,21062,21062,21062,21062,21062,21062,21062,
						 18794,18794,18794,18794,18794,18794,18794,18794,
						 16383,16383,16383,16383,16383,16383,16383,16383,
						 13847,13847,13847,13847,13847,13847,13847,13847,
						 11206,11206,11206,11206,11206,11206,11206,11206,
						 8480,8480,8480,8480,8480,8480,8480,8480,
						 5689,5689,5689,5689,5689,5689,5689,5689,
						 2855,2855,2855,2855,2855,2855,2855,2855,
						 0,0,0,0,0,0,0,0,
						 -2855,-2855,-2855,-2855,-2855,-2855,-2855,-2855,
						 -5689,-5689,-5689,-5689,-5689,-5689,-5689,-5689,
						 -8480,-8480,-8480,-8480,-8480,-8480,-8480,-8480,
						 -11206,-11206,-11206,-11206,-11206,-11206,-11206,-11206,
						 -13847,-13847,-13847,-13847,-13847,-13847,-13847,-13847,
						 -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
						 -18794,-18794,-18794,-18794,-18794,-18794,-18794,-18794,
						 -21062,-21062,-21062,-21062,-21062,-21062,-21062,-21062,
						 -23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
						 -25100,-25100,-25100,-25100,-25100,-25100,-25100,-25100,
						 -26841,-26841,-26841,-26841,-26841,-26841,-26841,-26841,
						 -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
						 -29696,-29696,-29696,-29696,-29696,-29696,-29696,-29696,
						 -30790,-30790,-30790,-30790,-30790,-30790,-30790,-30790,
						 -31650,-31650,-31650,-31650,-31650,-31650,-31650,-31650,
						 -32269,-32269,-32269,-32269,-32269,-32269,-32269,-32269,
						 -32642,-32642,-32642,-32642,-32642,-32642,-32642,-32642};
short twi72[280]__attribute__((aligned(16))) = {-2855,-2855,-2855,-2855,-2855,-2855,-2855,-2855,
						 -5689,-5689,-5689,-5689,-5689,-5689,-5689,-5689,
						 -8480,-8480,-8480,-8480,-8480,-8480,-8480,-8480,
						 -11206,-11206,-11206,-11206,-11206,-11206,-11206,-11206,
						 -13847,-13847,-13847,-13847,-13847,-13847,-13847,-13847,
						 -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
						 -18794,-18794,-18794,-18794,-18794,-18794,-18794,-18794,
						 -21062,-21062,-21062,-21062,-21062,-21062,-21062,-21062,
						 -23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
						 -25100,-25100,-25100,-25100,-25100,-25100,-25100,-25100,
						 -26841,-26841,-26841,-26841,-26841,-26841,-26841,-26841,
						 -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
						 -29696,-29696,-29696,-29696,-29696,-29696,-29696,-29696,
						 -30790,-30790,-30790,-30790,-30790,-30790,-30790,-30790,
						 -31650,-31650,-31650,-31650,-31650,-31650,-31650,-31650,
						 -32269,-32269,-32269,-32269,-32269,-32269,-32269,-32269,
						 -32642,-32642,-32642,-32642,-32642,-32642,-32642,-32642,
						 -32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767,
						 -32642,-32642,-32642,-32642,-32642,-32642,-32642,-32642,
						 -32269,-32269,-32269,-32269,-32269,-32269,-32269,-32269,
						 -31650,-31650,-31650,-31650,-31650,-31650,-31650,-31650,
						 -30790,-30790,-30790,-30790,-30790,-30790,-30790,-30790,
						 -29696,-29696,-29696,-29696,-29696,-29696,-29696,-29696,
						 -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
						 -26841,-26841,-26841,-26841,-26841,-26841,-26841,-26841,
						 -25100,-25100,-25100,-25100,-25100,-25100,-25100,-25100,
						 -23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
						 -21062,-21062,-21062,-21062,-21062,-21062,-21062,-21062,
						 -18794,-18794,-18794,-18794,-18794,-18794,-18794,-18794,
						 -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
						 -13847,-13847,-13847,-13847,-13847,-13847,-13847,-13847,
						 -11206,-11206,-11206,-11206,-11206,-11206,-11206,-11206,
						 -8480,-8480,-8480,-8480,-8480,-8480,-8480,-8480,
						 -5689,-5689,-5689,-5689,-5689,-5689,-5689,-5689,
						 -2855,-2855,-2855,-2855,-2855,-2855,-2855,-2855};

void dft72(short *xre,short *yre, short *xim, short *yim){

  int i,j;
  __m128i *xre128=(__m128i *)xre;
  __m128i *yre128=(__m128i *)yre;
  __m128i *xim128=(__m128i *)xim;
  __m128i *yim128=(__m128i *)yim;
  __m128i *twr128=(__m128i *)&twr72[0];
  __m128i *twi128=(__m128i *)&twi72[0];
  __m128i x2re128array[72],x2im128array[72];
  __m128i *x2re128 = (__m128i *)&x2re128array[0],*x2im128 = (__m128i *)&x2im128array[0];
  __m128i ytmpre128array[72],ytmpim128array[72];
  __m128i *ytmpre128=&ytmpre128array[0],*ytmpim128=&ytmpim128array[0];


  for (i=0,j=0;i<36;i++,j+=2) {
    x2re128[i]    = xre128[j];
    x2re128[i+36] = xre128[j+1];
    x2im128[i]    = xim128[j];
    x2im128[i+36] = xim128[j+1];
  }

  dft36((short *)x2re128,(short *)ytmpre128,(short *)xim128,(short *)ytmpim128);
  dft36((short *)(x2re128+36),(short *)(ytmpre128+36),(short *)(x2im128+36),(short *)(ytmpim128+36));

  bfly2_tw1(ytmpre128,ytmpre128+36,yre128,yre128+36,ytmpim128,ytmpim128+36,yim128,yim128+36);
  for (i=0;i<35;i++) {
    bfly2(ytmpre128+i,
	  ytmpre128+36+i,
	  yre128+i,
	  yre128+36+i,
	  twr128+i,
	  ytmpim128+i,
	  ytmpim128+i+36,
	  yim128+i,
	  yim128+i+36,
	  twi128+i);
  }
}

short twr96[376]__attribute__((aligned(16))) = {32696,32696,32696,32696,32696,32696,32696,32696,
						32486,32486,32486,32486,32486,32486,32486,32486,
						32137,32137,32137,32137,32137,32137,32137,32137,
						31650,31650,31650,31650,31650,31650,31650,31650,
						31028,31028,31028,31028,31028,31028,31028,31028,
						30272,30272,30272,30272,30272,30272,30272,30272,
						29387,29387,29387,29387,29387,29387,29387,29387,
						28377,28377,28377,28377,28377,28377,28377,28377,
						27244,27244,27244,27244,27244,27244,27244,27244,
						25995,25995,25995,25995,25995,25995,25995,25995,
						24635,24635,24635,24635,24635,24635,24635,24635,
						23169,23169,23169,23169,23169,23169,23169,23169,
						21604,21604,21604,21604,21604,21604,21604,21604,
						19947,19947,19947,19947,19947,19947,19947,19947,
						18204,18204,18204,18204,18204,18204,18204,18204,
						16383,16383,16383,16383,16383,16383,16383,16383,
						14492,14492,14492,14492,14492,14492,14492,14492,
						12539,12539,12539,12539,12539,12539,12539,12539,
						10532,10532,10532,10532,10532,10532,10532,10532,
						8480,8480,8480,8480,8480,8480,8480,8480,
						6392,6392,6392,6392,6392,6392,6392,6392,
						4276,4276,4276,4276,4276,4276,4276,4276,
						2143,2143,2143,2143,2143,2143,2143,2143,
						0,0,0,0,0,0,0,0,
						-2143,-2143,-2143,-2143,-2143,-2143,-2143,-2143,
						-4276,-4276,-4276,-4276,-4276,-4276,-4276,-4276,
						-6392,-6392,-6392,-6392,-6392,-6392,-6392,-6392,
						-8480,-8480,-8480,-8480,-8480,-8480,-8480,-8480,
						-10532,-10532,-10532,-10532,-10532,-10532,-10532,-10532,
						-12539,-12539,-12539,-12539,-12539,-12539,-12539,-12539,
						-14492,-14492,-14492,-14492,-14492,-14492,-14492,-14492,
						-16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
						-18204,-18204,-18204,-18204,-18204,-18204,-18204,-18204,
						-19947,-19947,-19947,-19947,-19947,-19947,-19947,-19947,
						-21604,-21604,-21604,-21604,-21604,-21604,-21604,-21604};

short twi96[376]__attribute__((aligned(16))) = {-2143,-2143,-2143,-2143,-2143,-2143,-2143,-2143,
						-4276,-4276,-4276,-4276,-4276,-4276,-4276,-4276,
						-6392,-6392,-6392,-6392,-6392,-6392,-6392,-6392,
						-8480,-8480,-8480,-8480,-8480,-8480,-8480,-8480,
						-10532,-10532,-10532,-10532,-10532,-10532,-10532,-10532,
						-12539,-12539,-12539,-12539,-12539,-12539,-12539,-12539,
						-14492,-14492,-14492,-14492,-14492,-14492,-14492,-14492,
						-16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
						-18204,-18204,-18204,-18204,-18204,-18204,-18204,-18204,
						-19947,-19947,-19947,-19947,-19947,-19947,-19947,-19947,
						-21604,-21604,-21604,-21604,-21604,-21604,-21604,-21604,
						-23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
						-24635,-24635,-24635,-24635,-24635,-24635,-24635,-24635,
						-25995,-25995,-25995,-25995,-25995,-25995,-25995,-25995,
						-27244,-27244,-27244,-27244,-27244,-27244,-27244,-27244,
						-28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
						-29387,-29387,-29387,-29387,-29387,-29387,-29387,-29387,
						-30272,-30272,-30272,-30272,-30272,-30272,-30272,-30272,
						-31028,-31028,-31028,-31028,-31028,-31028,-31028,-31028,
						-31650,-31650,-31650,-31650,-31650,-31650,-31650,-31650,
						-32137,-32137,-32137,-32137,-32137,-32137,-32137,-32137,
						-32486,-32486,-32486,-32486,-32486,-32486,-32486,-32486,
						-32696,-32696,-32696,-32696,-32696,-32696,-32696,-32696,
						-32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767,
						-32696,-32696,-32696,-32696,-32696,-32696,-32696,-32696,
						-32486,-32486,-32486,-32486,-32486,-32486,-32486,-32486,
						-32137,-32137,-32137,-32137,-32137,-32137,-32137,-32137,
						-31650,-31650,-31650,-31650,-31650,-31650,-31650,-31650,
						-31028,-31028,-31028,-31028,-31028,-31028,-31028,-31028,
						-30272,-30272,-30272,-30272,-30272,-30272,-30272,-30272,
						-29387,-29387,-29387,-29387,-29387,-29387,-29387,-29387,
						-28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
						-27244,-27244,-27244,-27244,-27244,-27244,-27244,-27244,
						-25995,-25995,-25995,-25995,-25995,-25995,-25995,-25995,
						-24635,-24635,-24635,-24635,-24635,-24635,-24635,-24635};

void dft96(short *xre,short *yre, short *xim, short *yim){


  int i,j;
  __m128i *xre128=(__m128i *)xre;
  __m128i *yre128=(__m128i *)yre;
  __m128i *xim128=(__m128i *)xim;
  __m128i *yim128=(__m128i *)yim;
  __m128i *twr128=(__m128i *)&twr96[0];
  __m128i *twi128=(__m128i *)&twi96[0];
  __m128i x2re128array[72],x2im128array[72];
  __m128i *x2re128 = (__m128i *)&x2re128array[0],*x2im128 = (__m128i *)&x2im128array[0];
  __m128i ytmpre128array[96],ytmpim128array[96];
  __m128i *ytmpre128=&ytmpre128array[0],*ytmpim128=&ytmpim128array[0];


  for (i=0,j=0;i<48;i++,j+=2) {
    x2re128[i]    = xre128[j];
    x2re128[i+48] = xre128[j+1];
    x2im128[i]    = xim128[j];
    x2im128[i+48] = xim128[j+1];
  }

  dft48((short *)x2re128,(short *)ytmpre128,(short *)xim128,(short *)ytmpim128);
  dft48((short *)(x2re128+48),(short *)(ytmpre128+48),(short *)(x2im128+48),(short *)(ytmpim128+48));


  bfly2_tw1(ytmpre128,ytmpre128+48,yre128,yre128+48,ytmpim128,ytmpim128+48,yim128,yim128+48);
  for (i=0;i<47;i++) {
    bfly2(ytmpre128+i,
	  ytmpre128+48+i,
	  yre128+i,
	  yre128+48+i,
	  twr128+i,
	  ytmpim128+i,
	  ytmpim128+i+48,
	  yim128+i,
	  yim128+i+48,
	  twi128+i);
  }
}

short twra108[280]__attribute__((aligned(16))) = { 32711,32711,32711,32711,32711,32711,32711,32711,
						   32545,32545,32545,32545,32545,32545,32545,32545,
						   32269,32269,32269,32269,32269,32269,32269,32269,
						   31883,31883,31883,31883,31883,31883,31883,31883,
						   31390,31390,31390,31390,31390,31390,31390,31390,
						   30790,30790,30790,30790,30790,30790,30790,30790,
						   30087,30087,30087,30087,30087,30087,30087,30087,
						   29281,29281,29281,29281,29281,29281,29281,29281,
						   28377,28377,28377,28377,28377,28377,28377,28377,
						   27376,27376,27376,27376,27376,27376,27376,27376,
						   26283,26283,26283,26283,26283,26283,26283,26283,
						   25100,25100,25100,25100,25100,25100,25100,25100,
						   23833,23833,23833,23833,23833,23833,23833,23833,
						   22486,22486,22486,22486,22486,22486,22486,22486,
						   21062,21062,21062,21062,21062,21062,21062,21062,
						   19567,19567,19567,19567,19567,19567,19567,19567,
						   18005,18005,18005,18005,18005,18005,18005,18005,
						   16383,16383,16383,16383,16383,16383,16383,16383,
						   14705,14705,14705,14705,14705,14705,14705,14705,
						   12978,12978,12978,12978,12978,12978,12978,12978,
						   11206,11206,11206,11206,11206,11206,11206,11206,
						   9397,9397,9397,9397,9397,9397,9397,9397,
						   7556,7556,7556,7556,7556,7556,7556,7556,
						   5689,5689,5689,5689,5689,5689,5689,5689,
						   3804,3804,3804,3804,3804,3804,3804,3804,
						   1905,1905,1905,1905,1905,1905,1905,1905,
						   0,0,0,0,0,0,0,0,
						   -1905,-1905,-1905,-1905,-1905,-1905,-1905,-1905,
						   -3804,-3804,-3804,-3804,-3804,-3804,-3804,-3804,
						   -5689,-5689,-5689,-5689,-5689,-5689,-5689,-5689,
						   -7556,-7556,-7556,-7556,-7556,-7556,-7556,-7556,
						   -9397,-9397,-9397,-9397,-9397,-9397,-9397,-9397,
						   -11206,-11206,-11206,-11206,-11206,-11206,-11206,-11206,
						   -12978,-12978,-12978,-12978,-12978,-12978,-12978,-12978,
						   -14705,-14705,-14705,-14705,-14705,-14705,-14705,-14705};
						   
short twrb108[280]__attribute__((aligned(16))) = {32545,32545,32545,32545,32545,32545,32545,32545,
						  31883,31883,31883,31883,31883,31883,31883,31883,
						  30790,30790,30790,30790,30790,30790,30790,30790,
						  29281,29281,29281,29281,29281,29281,29281,29281,
						  27376,27376,27376,27376,27376,27376,27376,27376,
						  25100,25100,25100,25100,25100,25100,25100,25100,
						  22486,22486,22486,22486,22486,22486,22486,22486,
						  19567,19567,19567,19567,19567,19567,19567,19567,
						  16383,16383,16383,16383,16383,16383,16383,16383,
						  12978,12978,12978,12978,12978,12978,12978,12978,
						  9397,9397,9397,9397,9397,9397,9397,9397,
						  5689,5689,5689,5689,5689,5689,5689,5689,
						  1905,1905,1905,1905,1905,1905,1905,1905,
						  -1905,-1905,-1905,-1905,-1905,-1905,-1905,-1905,
						  -5689,-5689,-5689,-5689,-5689,-5689,-5689,-5689,
						  -9397,-9397,-9397,-9397,-9397,-9397,-9397,-9397,
						  -12978,-12978,-12978,-12978,-12978,-12978,-12978,-12978,
						  -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
						  -19567,-19567,-19567,-19567,-19567,-19567,-19567,-19567,
						  -22486,-22486,-22486,-22486,-22486,-22486,-22486,-22486,
						  -25100,-25100,-25100,-25100,-25100,-25100,-25100,-25100,
						  -27376,-27376,-27376,-27376,-27376,-27376,-27376,-27376,
						  -29281,-29281,-29281,-29281,-29281,-29281,-29281,-29281,
						  -30790,-30790,-30790,-30790,-30790,-30790,-30790,-30790,
						  -31883,-31883,-31883,-31883,-31883,-31883,-31883,-31883,
						  -32545,-32545,-32545,-32545,-32545,-32545,-32545,-32545,
						  -32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767,
						  -32545,-32545,-32545,-32545,-32545,-32545,-32545,-32545,
						  -31883,-31883,-31883,-31883,-31883,-31883,-31883,-31883,
						  -30790,-30790,-30790,-30790,-30790,-30790,-30790,-30790,
						  -29281,-29281,-29281,-29281,-29281,-29281,-29281,-29281,
						  -27376,-27376,-27376,-27376,-27376,-27376,-27376,-27376,
						  -25100,-25100,-25100,-25100,-25100,-25100,-25100,-25100,
						  -22486,-22486,-22486,-22486,-22486,-22486,-22486,-22486,
						  -19567,-19567,-19567,-19567,-19567,-19567,-19567,-19567};

short twia108[280]__attribute__((aligned(16))) = { -1905,-1905,-1905,-1905,-1905,-1905,-1905,-1905,
						   -3804,-3804,-3804,-3804,-3804,-3804,-3804,-3804,
						   -5689,-5689,-5689,-5689,-5689,-5689,-5689,-5689,
						   -7556,-7556,-7556,-7556,-7556,-7556,-7556,-7556,
						   -9397,-9397,-9397,-9397,-9397,-9397,-9397,-9397,
						   -11206,-11206,-11206,-11206,-11206,-11206,-11206,-11206,
						   -12978,-12978,-12978,-12978,-12978,-12978,-12978,-12978,
						   -14705,-14705,-14705,-14705,-14705,-14705,-14705,-14705,
						   -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
						   -18005,-18005,-18005,-18005,-18005,-18005,-18005,-18005,
						   -19567,-19567,-19567,-19567,-19567,-19567,-19567,-19567,
						   -21062,-21062,-21062,-21062,-21062,-21062,-21062,-21062,
						   -22486,-22486,-22486,-22486,-22486,-22486,-22486,-22486,
						   -23833,-23833,-23833,-23833,-23833,-23833,-23833,-23833,
						   -25100,-25100,-25100,-25100,-25100,-25100,-25100,-25100,
						   -26283,-26283,-26283,-26283,-26283,-26283,-26283,-26283,
						   -27376,-27376,-27376,-27376,-27376,-27376,-27376,-27376,
						   -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
						   -29281,-29281,-29281,-29281,-29281,-29281,-29281,-29281,
						   -30087,-30087,-30087,-30087,-30087,-30087,-30087,-30087,
						   -30790,-30790,-30790,-30790,-30790,-30790,-30790,-30790,
						   -31390,-31390,-31390,-31390,-31390,-31390,-31390,-31390,
						   -31883,-31883,-31883,-31883,-31883,-31883,-31883,-31883,
						   -32269,-32269,-32269,-32269,-32269,-32269,-32269,-32269,
						   -32545,-32545,-32545,-32545,-32545,-32545,-32545,-32545,
						   -32711,-32711,-32711,-32711,-32711,-32711,-32711,-32711,
						   -32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767,
						   -32711,-32711,-32711,-32711,-32711,-32711,-32711,-32711,
						   -32545,-32545,-32545,-32545,-32545,-32545,-32545,-32545,
						   -32269,-32269,-32269,-32269,-32269,-32269,-32269,-32269,
						   -31883,-31883,-31883,-31883,-31883,-31883,-31883,-31883,
						   -31390,-31390,-31390,-31390,-31390,-31390,-31390,-31390,
						   -30790,-30790,-30790,-30790,-30790,-30790,-30790,-30790,
						   -30087,-30087,-30087,-30087,-30087,-30087,-30087,-30087,
						   -29281,-29281,-29281,-29281,-29281,-29281,-29281,-29281};

short twib108[280]__attribute__((aligned(16))) = {-3804,-3804,-3804,-3804,-3804,-3804,-3804,-3804,
						  -7556,-7556,-7556,-7556,-7556,-7556,-7556,-7556,
						  -11206,-11206,-11206,-11206,-11206,-11206,-11206,-11206,
						  -14705,-14705,-14705,-14705,-14705,-14705,-14705,-14705,
						  -18005,-18005,-18005,-18005,-18005,-18005,-18005,-18005,
						  -21062,-21062,-21062,-21062,-21062,-21062,-21062,-21062,
						  -23833,-23833,-23833,-23833,-23833,-23833,-23833,-23833,
						  -26283,-26283,-26283,-26283,-26283,-26283,-26283,-26283,
						  -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
						  -30087,-30087,-30087,-30087,-30087,-30087,-30087,-30087,
						  -31390,-31390,-31390,-31390,-31390,-31390,-31390,-31390,
						  -32269,-32269,-32269,-32269,-32269,-32269,-32269,-32269,
						  -32711,-32711,-32711,-32711,-32711,-32711,-32711,-32711,
						  -32711,-32711,-32711,-32711,-32711,-32711,-32711,-32711,
						  -32269,-32269,-32269,-32269,-32269,-32269,-32269,-32269,
						  -31390,-31390,-31390,-31390,-31390,-31390,-31390,-31390,
						  -30087,-30087,-30087,-30087,-30087,-30087,-30087,-30087,
						  -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
						  -26283,-26283,-26283,-26283,-26283,-26283,-26283,-26283,
						  -23833,-23833,-23833,-23833,-23833,-23833,-23833,-23833,
						  -21062,-21062,-21062,-21062,-21062,-21062,-21062,-21062,
						  -18005,-18005,-18005,-18005,-18005,-18005,-18005,-18005,
						  -14705,-14705,-14705,-14705,-14705,-14705,-14705,-14705,
						  -11206,-11206,-11206,-11206,-11206,-11206,-11206,-11206,
						  -7556,-7556,-7556,-7556,-7556,-7556,-7556,-7556,
						  -3804,-3804,-3804,-3804,-3804,-3804,-3804,-3804,
						  0,0,0,0,0,0,0,0,
						  3804,3804,3804,3804,3804,3804,3804,3804,
						  7556,7556,7556,7556,7556,7556,7556,7556,
						  11206,11206,11206,11206,11206,11206,11206,11206,
						  14705,14705,14705,14705,14705,14705,14705,14705,
						  18005,18005,18005,18005,18005,18005,18005,18005,
						  21062,21062,21062,21062,21062,21062,21062,21062,
						  23833,23833,23833,23833,23833,23833,23833,23833,
						  26283,26283,26283,26283,26283,26283,26283,26283};

void dft108(short *xre,short *yre, short *xim, short *yim){


  int i,j;
  __m128i *xre128=(__m128i *)xre;
  __m128i *yre128=(__m128i *)yre;
  __m128i *xim128=(__m128i *)xim;
  __m128i *yim128=(__m128i *)yim;
  __m128i *twra128=(__m128i *)&twra108[0];
  __m128i *twia128=(__m128i *)&twia108[0];
  __m128i *twrb128=(__m128i *)&twrb108[0];
  __m128i *twib128=(__m128i *)&twib108[0];
  __m128i x2re128array[108],x2im128array[108];
  __m128i *x2re128 = (__m128i *)&x2re128array[0],*x2im128 = (__m128i *)&x2im128array[0];
  __m128i ytmpre128array[108],ytmpim128array[108];
  __m128i *ytmpre128=&ytmpre128array[0],*ytmpim128=&ytmpim128array[0];


  for (i=0,j=0;i<36;i++,j+=3) {
    x2re128[i]    = xre128[j];
    x2re128[i+36] = xre128[j+1];
    x2re128[i+72] = xre128[j+2];
    x2im128[i]    = xim128[j];
    x2im128[i+36] = xim128[j+1];
    x2im128[i+72] = xim128[j+2];
  }

  dft36((short *)x2re128,(short *)ytmpre128,(short *)xim128,(short *)ytmpim128);
  dft36((short *)(x2re128+36),(short *)(ytmpre128+36),(short *)(x2im128+36),(short *)(ytmpim128+36));
  dft36((short *)(x2re128+72),(short *)(ytmpre128+72),(short *)(x2im128+72),(short *)(ytmpim128+72));

  bfly3_tw1(ytmpre128,ytmpre128+36,ytmpre128+72,yre128,yre128+36,yre128+72,ytmpim128,ytmpim128+36,ytmpim128+72,yim128,yim128+36,yim128+72);
  for (i=0;i<35;i++) {
    bfly3(ytmpre128+i,
	  ytmpre128+36+i,
	  ytmpre128+72+i,
	  yre128+i,
	  yre128+36+i,
	  yre128+72+i,
	  twra128+i,
	  twrb128+i,
	  ytmpim128+i,
	  ytmpim128+i+36,
	  ytmpim128+i+72,
	  yim128+i,
	  yim128+i+36,
	  yim128+i+72,
	  twia128+i,
	  twib128+i);
  }
}

short twr120[472]__attribute__((aligned(16))) = {32722,32722,32722,32722,32722,32722,32722,32722,
						 32587,32587,32587,32587,32587,32587,32587,32587,
						 32363,32363,32363,32363,32363,32363,32363,32363,
						 32050,32050,32050,32050,32050,32050,32050,32050,
						 31650,31650,31650,31650,31650,31650,31650,31650,
						 31163,31163,31163,31163,31163,31163,31163,31163,
						 30590,30590,30590,30590,30590,30590,30590,30590,
						 29934,29934,29934,29934,29934,29934,29934,29934,
						 29195,29195,29195,29195,29195,29195,29195,29195,
						 28377,28377,28377,28377,28377,28377,28377,28377,
						 27480,27480,27480,27480,27480,27480,27480,27480,
						 26509,26509,26509,26509,26509,26509,26509,26509,
						 25464,25464,25464,25464,25464,25464,25464,25464,
						 24350,24350,24350,24350,24350,24350,24350,24350,
						 23169,23169,23169,23169,23169,23169,23169,23169,
						 21925,21925,21925,21925,21925,21925,21925,21925,
						 20620,20620,20620,20620,20620,20620,20620,20620,
						 19259,19259,19259,19259,19259,19259,19259,19259,
						 17846,17846,17846,17846,17846,17846,17846,17846,
						 16383,16383,16383,16383,16383,16383,16383,16383,
						 14875,14875,14875,14875,14875,14875,14875,14875,
						 13327,13327,13327,13327,13327,13327,13327,13327,
						 11742,11742,11742,11742,11742,11742,11742,11742,
						 10125,10125,10125,10125,10125,10125,10125,10125,
						 8480,8480,8480,8480,8480,8480,8480,8480,
						 6812,6812,6812,6812,6812,6812,6812,6812,
						 5125,5125,5125,5125,5125,5125,5125,5125,
						 3425,3425,3425,3425,3425,3425,3425,3425,
						 1714,1714,1714,1714,1714,1714,1714,1714,
						 0,0,0,0,0,0,0,0,
						 -1714,-1714,-1714,-1714,-1714,-1714,-1714,-1714,
						 -3425,-3425,-3425,-3425,-3425,-3425,-3425,-3425,
						 -5125,-5125,-5125,-5125,-5125,-5125,-5125,-5125,
						 -6812,-6812,-6812,-6812,-6812,-6812,-6812,-6812,
						 -8480,-8480,-8480,-8480,-8480,-8480,-8480,-8480,
						 -10125,-10125,-10125,-10125,-10125,-10125,-10125,-10125,
						 -11742,-11742,-11742,-11742,-11742,-11742,-11742,-11742,
						 -13327,-13327,-13327,-13327,-13327,-13327,-13327,-13327,
						 -14875,-14875,-14875,-14875,-14875,-14875,-14875,-14875,
						 -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
						 -17846,-17846,-17846,-17846,-17846,-17846,-17846,-17846,
						 -19259,-19259,-19259,-19259,-19259,-19259,-19259,-19259,
						 -20620,-20620,-20620,-20620,-20620,-20620,-20620,-20620,
						 -21925,-21925,-21925,-21925,-21925,-21925,-21925,-21925,
						 -23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
						 -24350,-24350,-24350,-24350,-24350,-24350,-24350,-24350,
						 -25464,-25464,-25464,-25464,-25464,-25464,-25464,-25464,
						 -26509,-26509,-26509,-26509,-26509,-26509,-26509,-26509,
						 -27480,-27480,-27480,-27480,-27480,-27480,-27480,-27480,
						 -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
						 -29195,-29195,-29195,-29195,-29195,-29195,-29195,-29195,
						 -29934,-29934,-29934,-29934,-29934,-29934,-29934,-29934,
						 -30590,-30590,-30590,-30590,-30590,-30590,-30590,-30590,
						 -31163,-31163,-31163,-31163,-31163,-31163,-31163,-31163,
						 -31650,-31650,-31650,-31650,-31650,-31650,-31650,-31650,
						 -32050,-32050,-32050,-32050,-32050,-32050,-32050,-32050,
						 -32363,-32363,-32363,-32363,-32363,-32363,-32363,-32363,
						 -32587,-32587,-32587,-32587,-32587,-32587,-32587,-32587,
						 -32722,-32722,-32722,-32722,-32722,-32722,-32722,-32722};

short twi120[472]__attribute__((aligned(16))) = {-1714,-1714,-1714,-1714,-1714,-1714,-1714,-1714,
						 -3425,-3425,-3425,-3425,-3425,-3425,-3425,-3425,
						 -5125,-5125,-5125,-5125,-5125,-5125,-5125,-5125,
						 -6812,-6812,-6812,-6812,-6812,-6812,-6812,-6812,
						 -8480,-8480,-8480,-8480,-8480,-8480,-8480,-8480,
						 -10125,-10125,-10125,-10125,-10125,-10125,-10125,-10125,
						 -11742,-11742,-11742,-11742,-11742,-11742,-11742,-11742,
						 -13327,-13327,-13327,-13327,-13327,-13327,-13327,-13327,
						 -14875,-14875,-14875,-14875,-14875,-14875,-14875,-14875,
						 -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
						 -17846,-17846,-17846,-17846,-17846,-17846,-17846,-17846,
						 -19259,-19259,-19259,-19259,-19259,-19259,-19259,-19259,
						 -20620,-20620,-20620,-20620,-20620,-20620,-20620,-20620,
						 -21925,-21925,-21925,-21925,-21925,-21925,-21925,-21925,
						 -23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
						 -24350,-24350,-24350,-24350,-24350,-24350,-24350,-24350,
						 -25464,-25464,-25464,-25464,-25464,-25464,-25464,-25464,
						 -26509,-26509,-26509,-26509,-26509,-26509,-26509,-26509,
						 -27480,-27480,-27480,-27480,-27480,-27480,-27480,-27480,
						 -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
						 -29195,-29195,-29195,-29195,-29195,-29195,-29195,-29195,
						 -29934,-29934,-29934,-29934,-29934,-29934,-29934,-29934,
						 -30590,-30590,-30590,-30590,-30590,-30590,-30590,-30590,
						 -31163,-31163,-31163,-31163,-31163,-31163,-31163,-31163,
						 -31650,-31650,-31650,-31650,-31650,-31650,-31650,-31650,
						 -32050,-32050,-32050,-32050,-32050,-32050,-32050,-32050,
						 -32363,-32363,-32363,-32363,-32363,-32363,-32363,-32363,
						 -32587,-32587,-32587,-32587,-32587,-32587,-32587,-32587,
						 -32722,-32722,-32722,-32722,-32722,-32722,-32722,-32722,
						 -32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767,
						 -32722,-32722,-32722,-32722,-32722,-32722,-32722,-32722,
						 -32587,-32587,-32587,-32587,-32587,-32587,-32587,-32587,
						 -32363,-32363,-32363,-32363,-32363,-32363,-32363,-32363,
						 -32050,-32050,-32050,-32050,-32050,-32050,-32050,-32050,
						 -31650,-31650,-31650,-31650,-31650,-31650,-31650,-31650,
						 -31163,-31163,-31163,-31163,-31163,-31163,-31163,-31163,
						 -30590,-30590,-30590,-30590,-30590,-30590,-30590,-30590,
						 -29934,-29934,-29934,-29934,-29934,-29934,-29934,-29934,
						 -29195,-29195,-29195,-29195,-29195,-29195,-29195,-29195,
						 -28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
						 -27480,-27480,-27480,-27480,-27480,-27480,-27480,-27480,
						 -26509,-26509,-26509,-26509,-26509,-26509,-26509,-26509,
						 -25464,-25464,-25464,-25464,-25464,-25464,-25464,-25464,
						 -24350,-24350,-24350,-24350,-24350,-24350,-24350,-24350,
						 -23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
						 -21925,-21925,-21925,-21925,-21925,-21925,-21925,-21925,
						 -20620,-20620,-20620,-20620,-20620,-20620,-20620,-20620,
						 -19259,-19259,-19259,-19259,-19259,-19259,-19259,-19259,
						 -17846,-17846,-17846,-17846,-17846,-17846,-17846,-17846,
						 -16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
						 -14875,-14875,-14875,-14875,-14875,-14875,-14875,-14875,
						 -13327,-13327,-13327,-13327,-13327,-13327,-13327,-13327,
						 -11742,-11742,-11742,-11742,-11742,-11742,-11742,-11742,
						 -10125,-10125,-10125,-10125,-10125,-10125,-10125,-10125,
						 -8480,-8480,-8480,-8480,-8480,-8480,-8480,-8480,
						 -6812,-6812,-6812,-6812,-6812,-6812,-6812,-6812,
						 -5125,-5125,-5125,-5125,-5125,-5125,-5125,-5125,
						 -3425,-3425,-3425,-3425,-3425,-3425,-3425,-3425,
						 -1714,-1714,-1714,-1714,-1714,-1714,-1714,-1714};


void dft120(short *xre,short *yre, short *xim, short *yim){


  int i,j;
  __m128i *xre128=(__m128i *)xre;
  __m128i *yre128=(__m128i *)yre;
  __m128i *xim128=(__m128i *)xim;
  __m128i *yim128=(__m128i *)yim;
  __m128i *twr128=(__m128i *)&twr120[0];
  __m128i *twi128=(__m128i *)&twi120[0];
  __m128i x2re128array[120],x2im128array[120];
  __m128i *x2re128 = (__m128i *)&x2re128array[0],*x2im128 = (__m128i *)&x2im128array[0];
  __m128i ytmpre128array[120],ytmpim128array[120];
  __m128i *ytmpre128=&ytmpre128array[0],*ytmpim128=&ytmpim128array[0];


  for (i=0,j=0;i<60;i++,j+=2) {
    x2re128[i]    = xre128[j];
    x2re128[i+60] = xre128[j+1];
    x2im128[i]    = xim128[j];
    x2im128[i+60] = xim128[j+1];
  }

  dft60((short *)x2re128,(short *)ytmpre128,(short *)xim128,(short *)ytmpim128);
  dft60((short *)(x2re128+60),(short *)(ytmpre128+60),(short *)(x2im128+60),(short *)(ytmpim128+60));


  bfly2_tw1(ytmpre128,ytmpre128+48,yre128,yre128+48,ytmpim128,ytmpim128+48,yim128,yim128+48);
  for (i=0;i<59;i++) {
    bfly2(ytmpre128+i,
	  ytmpre128+60+i,
	  yre128+i,
	  yre128+60+i,
	  twr128+i,
	  ytmpim128+i,
	  ytmpim128+i+60,
	  yim128+i,
	  yim128+i+60,
	  twi128+i);
  }
}

short twra144[376]__attribute__((aligned(16)))={32735,32735,32735,32735,32735,32735,32735,32735,
32642,32642,32642,32642,32642,32642,32642,32642,
32486,32486,32486,32486,32486,32486,32486,32486,
32269,32269,32269,32269,32269,32269,32269,32269,
31990,31990,31990,31990,31990,31990,31990,31990,
31650,31650,31650,31650,31650,31650,31650,31650,
31250,31250,31250,31250,31250,31250,31250,31250,
30790,30790,30790,30790,30790,30790,30790,30790,
30272,30272,30272,30272,30272,30272,30272,30272,
29696,29696,29696,29696,29696,29696,29696,29696,
29064,29064,29064,29064,29064,29064,29064,29064,
28377,28377,28377,28377,28377,28377,28377,28377,
27635,27635,27635,27635,27635,27635,27635,27635,
26841,26841,26841,26841,26841,26841,26841,26841,
25995,25995,25995,25995,25995,25995,25995,25995,
25100,25100,25100,25100,25100,25100,25100,25100,
24158,24158,24158,24158,24158,24158,24158,24158,
23169,23169,23169,23169,23169,23169,23169,23169,
22137,22137,22137,22137,22137,22137,22137,22137,
21062,21062,21062,21062,21062,21062,21062,21062,
19947,19947,19947,19947,19947,19947,19947,19947,
18794,18794,18794,18794,18794,18794,18794,18794,
17605,17605,17605,17605,17605,17605,17605,17605,
16383,16383,16383,16383,16383,16383,16383,16383,
15130,15130,15130,15130,15130,15130,15130,15130,
13847,13847,13847,13847,13847,13847,13847,13847,
12539,12539,12539,12539,12539,12539,12539,12539,
11206,11206,11206,11206,11206,11206,11206,11206,
9853,9853,9853,9853,9853,9853,9853,9853,
8480,8480,8480,8480,8480,8480,8480,8480,
7092,7092,7092,7092,7092,7092,7092,7092,
5689,5689,5689,5689,5689,5689,5689,5689,
4276,4276,4276,4276,4276,4276,4276,4276,
2855,2855,2855,2855,2855,2855,2855,2855,
1429,1429,1429,1429,1429,1429,1429,1429,
0,0,0,0,0,0,0,0,
-1429,-1429,-1429,-1429,-1429,-1429,-1429,-1429,
-2855,-2855,-2855,-2855,-2855,-2855,-2855,-2855,
-4276,-4276,-4276,-4276,-4276,-4276,-4276,-4276,
-5689,-5689,-5689,-5689,-5689,-5689,-5689,-5689,
-7092,-7092,-7092,-7092,-7092,-7092,-7092,-7092,
-8480,-8480,-8480,-8480,-8480,-8480,-8480,-8480,
-9853,-9853,-9853,-9853,-9853,-9853,-9853,-9853,
-11206,-11206,-11206,-11206,-11206,-11206,-11206,-11206,
-12539,-12539,-12539,-12539,-12539,-12539,-12539,-12539,
-13847,-13847,-13847,-13847,-13847,-13847,-13847,-13847,
  -15130,-15130,-15130,-15130,-15130,-15130,-15130,-15130};

short twia144[376]__attribute__((aligned(16)))={-1429,-1429,-1429,-1429,-1429,-1429,-1429,-1429,
-2855,-2855,-2855,-2855,-2855,-2855,-2855,-2855,
-4276,-4276,-4276,-4276,-4276,-4276,-4276,-4276,
-5689,-5689,-5689,-5689,-5689,-5689,-5689,-5689,
-7092,-7092,-7092,-7092,-7092,-7092,-7092,-7092,
-8480,-8480,-8480,-8480,-8480,-8480,-8480,-8480,
-9853,-9853,-9853,-9853,-9853,-9853,-9853,-9853,
-11206,-11206,-11206,-11206,-11206,-11206,-11206,-11206,
-12539,-12539,-12539,-12539,-12539,-12539,-12539,-12539,
-13847,-13847,-13847,-13847,-13847,-13847,-13847,-13847,
-15130,-15130,-15130,-15130,-15130,-15130,-15130,-15130,
-16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
-17605,-17605,-17605,-17605,-17605,-17605,-17605,-17605,
-18794,-18794,-18794,-18794,-18794,-18794,-18794,-18794,
-19947,-19947,-19947,-19947,-19947,-19947,-19947,-19947,
-21062,-21062,-21062,-21062,-21062,-21062,-21062,-21062,
-22137,-22137,-22137,-22137,-22137,-22137,-22137,-22137,
-23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
-24158,-24158,-24158,-24158,-24158,-24158,-24158,-24158,
-25100,-25100,-25100,-25100,-25100,-25100,-25100,-25100,
-25995,-25995,-25995,-25995,-25995,-25995,-25995,-25995,
-26841,-26841,-26841,-26841,-26841,-26841,-26841,-26841,
-27635,-27635,-27635,-27635,-27635,-27635,-27635,-27635,
-28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
-29064,-29064,-29064,-29064,-29064,-29064,-29064,-29064,
-29696,-29696,-29696,-29696,-29696,-29696,-29696,-29696,
-30272,-30272,-30272,-30272,-30272,-30272,-30272,-30272,
-30790,-30790,-30790,-30790,-30790,-30790,-30790,-30790,
-31250,-31250,-31250,-31250,-31250,-31250,-31250,-31250,
-31650,-31650,-31650,-31650,-31650,-31650,-31650,-31650,
-31990,-31990,-31990,-31990,-31990,-31990,-31990,-31990,
-32269,-32269,-32269,-32269,-32269,-32269,-32269,-32269,
-32486,-32486,-32486,-32486,-32486,-32486,-32486,-32486,
-32642,-32642,-32642,-32642,-32642,-32642,-32642,-32642,
-32735,-32735,-32735,-32735,-32735,-32735,-32735,-32735,
-32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767,
-32735,-32735,-32735,-32735,-32735,-32735,-32735,-32735,
-32642,-32642,-32642,-32642,-32642,-32642,-32642,-32642,
-32486,-32486,-32486,-32486,-32486,-32486,-32486,-32486,
-32269,-32269,-32269,-32269,-32269,-32269,-32269,-32269,
-31990,-31990,-31990,-31990,-31990,-31990,-31990,-31990,
-31650,-31650,-31650,-31650,-31650,-31650,-31650,-31650,
-31250,-31250,-31250,-31250,-31250,-31250,-31250,-31250,
-30790,-30790,-30790,-30790,-30790,-30790,-30790,-30790,
-30272,-30272,-30272,-30272,-30272,-30272,-30272,-30272,
-29696,-29696,-29696,-29696,-29696,-29696,-29696,-29696,
  -29064,-29064,-29064,-29064,-29064,-29064,-29064,-29064};

short twrb144[376]__attribute__((aligned(16)))={32642,32642,32642,32642,32642,32642,32642,32642,
32269,32269,32269,32269,32269,32269,32269,32269,
31650,31650,31650,31650,31650,31650,31650,31650,
30790,30790,30790,30790,30790,30790,30790,30790,
29696,29696,29696,29696,29696,29696,29696,29696,
28377,28377,28377,28377,28377,28377,28377,28377,
26841,26841,26841,26841,26841,26841,26841,26841,
25100,25100,25100,25100,25100,25100,25100,25100,
23169,23169,23169,23169,23169,23169,23169,23169,
21062,21062,21062,21062,21062,21062,21062,21062,
18794,18794,18794,18794,18794,18794,18794,18794,
16383,16383,16383,16383,16383,16383,16383,16383,
13847,13847,13847,13847,13847,13847,13847,13847,
11206,11206,11206,11206,11206,11206,11206,11206,
8480,8480,8480,8480,8480,8480,8480,8480,
5689,5689,5689,5689,5689,5689,5689,5689,
2855,2855,2855,2855,2855,2855,2855,2855,
0,0,0,0,0,0,0,0,
-2855,-2855,-2855,-2855,-2855,-2855,-2855,-2855,
-5689,-5689,-5689,-5689,-5689,-5689,-5689,-5689,
-8480,-8480,-8480,-8480,-8480,-8480,-8480,-8480,
-11206,-11206,-11206,-11206,-11206,-11206,-11206,-11206,
-13847,-13847,-13847,-13847,-13847,-13847,-13847,-13847,
-16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
-18794,-18794,-18794,-18794,-18794,-18794,-18794,-18794,
-21062,-21062,-21062,-21062,-21062,-21062,-21062,-21062,
-23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
-25100,-25100,-25100,-25100,-25100,-25100,-25100,-25100,
-26841,-26841,-26841,-26841,-26841,-26841,-26841,-26841,
-28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
-29696,-29696,-29696,-29696,-29696,-29696,-29696,-29696,
-30790,-30790,-30790,-30790,-30790,-30790,-30790,-30790,
-31650,-31650,-31650,-31650,-31650,-31650,-31650,-31650,
-32269,-32269,-32269,-32269,-32269,-32269,-32269,-32269,
-32642,-32642,-32642,-32642,-32642,-32642,-32642,-32642,
-32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767,
-32642,-32642,-32642,-32642,-32642,-32642,-32642,-32642,
-32269,-32269,-32269,-32269,-32269,-32269,-32269,-32269,
-31650,-31650,-31650,-31650,-31650,-31650,-31650,-31650,
-30790,-30790,-30790,-30790,-30790,-30790,-30790,-30790,
-29696,-29696,-29696,-29696,-29696,-29696,-29696,-29696,
-28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
-26841,-26841,-26841,-26841,-26841,-26841,-26841,-26841,
-25100,-25100,-25100,-25100,-25100,-25100,-25100,-25100,
-23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
-21062,-21062,-21062,-21062,-21062,-21062,-21062,-21062,
  -18794,-18794,-18794,-18794,-18794,-18794,-18794,-18794};


short twib144[376]__attribute__((aligned(16)))= {-2855,-2855,-2855,-2855,-2855,-2855,-2855,-2855,
-5689,-5689,-5689,-5689,-5689,-5689,-5689,-5689,
-8480,-8480,-8480,-8480,-8480,-8480,-8480,-8480,
-11206,-11206,-11206,-11206,-11206,-11206,-11206,-11206,
-13847,-13847,-13847,-13847,-13847,-13847,-13847,-13847,
-16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
-18794,-18794,-18794,-18794,-18794,-18794,-18794,-18794,
-21062,-21062,-21062,-21062,-21062,-21062,-21062,-21062,
-23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
-25100,-25100,-25100,-25100,-25100,-25100,-25100,-25100,
-26841,-26841,-26841,-26841,-26841,-26841,-26841,-26841,
-28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
-29696,-29696,-29696,-29696,-29696,-29696,-29696,-29696,
-30790,-30790,-30790,-30790,-30790,-30790,-30790,-30790,
-31650,-31650,-31650,-31650,-31650,-31650,-31650,-31650,
-32269,-32269,-32269,-32269,-32269,-32269,-32269,-32269,
-32642,-32642,-32642,-32642,-32642,-32642,-32642,-32642,
-32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767,
-32642,-32642,-32642,-32642,-32642,-32642,-32642,-32642,
-32269,-32269,-32269,-32269,-32269,-32269,-32269,-32269,
-31650,-31650,-31650,-31650,-31650,-31650,-31650,-31650,
-30790,-30790,-30790,-30790,-30790,-30790,-30790,-30790,
-29696,-29696,-29696,-29696,-29696,-29696,-29696,-29696,
-28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
-26841,-26841,-26841,-26841,-26841,-26841,-26841,-26841,
-25100,-25100,-25100,-25100,-25100,-25100,-25100,-25100,
-23169,-23169,-23169,-23169,-23169,-23169,-23169,-23169,
-21062,-21062,-21062,-21062,-21062,-21062,-21062,-21062,
-18794,-18794,-18794,-18794,-18794,-18794,-18794,-18794,
-16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
-13847,-13847,-13847,-13847,-13847,-13847,-13847,-13847,
-11206,-11206,-11206,-11206,-11206,-11206,-11206,-11206,
-8480,-8480,-8480,-8480,-8480,-8480,-8480,-8480,
-5689,-5689,-5689,-5689,-5689,-5689,-5689,-5689,
-2855,-2855,-2855,-2855,-2855,-2855,-2855,-2855,
0,0,0,0,0,0,0,0,
2855,2855,2855,2855,2855,2855,2855,2855,
5689,5689,5689,5689,5689,5689,5689,5689,
8480,8480,8480,8480,8480,8480,8480,8480,
11206,11206,11206,11206,11206,11206,11206,11206,
13847,13847,13847,13847,13847,13847,13847,13847,
16383,16383,16383,16383,16383,16383,16383,16383,
18794,18794,18794,18794,18794,18794,18794,18794,
21062,21062,21062,21062,21062,21062,21062,21062,
23169,23169,23169,23169,23169,23169,23169,23169,
25100,25100,25100,25100,25100,25100,25100,25100,
  26841,26841,26841,26841,26841,26841,26841,26841};


void dft144(short *xre,short *yre, short *xim, short *yim){

  int i,j;
  __m128i *xre128=(__m128i *)xre;
  __m128i *yre128=(__m128i *)yre;
  __m128i *xim128=(__m128i *)xim;
  __m128i *yim128=(__m128i *)yim;
  __m128i *twra128=(__m128i *)&twra144[0];
  __m128i *twia128=(__m128i *)&twia144[0];
  __m128i *twrb128=(__m128i *)&twrb144[0];
  __m128i *twib128=(__m128i *)&twib144[0];
  __m128i x2re128array[144],x2im128array[144];
  __m128i *x2re128 = (__m128i *)&x2re128array[0],*x2im128 = (__m128i *)&x2im128array[0];
  __m128i ytmpre128array[144],ytmpim128array[144];
  __m128i *ytmpre128=&ytmpre128array[0],*ytmpim128=&ytmpim128array[0];


  for (i=0,j=0;i<48;i++,j+=3) {
    x2re128[i]    = xre128[j];
    x2re128[i+48] = xre128[j+1];
    x2re128[i+96] = xre128[j+2];
    x2im128[i]    = xim128[j];
    x2im128[i+48] = xim128[j+1];
    x2im128[i+96] = xim128[j+2];
  }

  dft48((short *)x2re128,(short *)ytmpre128,(short *)xim128,(short *)ytmpim128);
  dft48((short *)(x2re128+48),(short *)(ytmpre128+48),(short *)(x2im128+48),(short *)(ytmpim128+48));
  dft48((short *)(x2re128+96),(short *)(ytmpre128+96),(short *)(x2im128+96),(short *)(ytmpim128+96));

  bfly3_tw1(ytmpre128,ytmpre128+48,ytmpre128+96,yre128,yre128+48,yre128+96,ytmpim128,ytmpim128+48,ytmpim128+96,yim128,yim128+48,yim128+96);
  for (i=0;i<47;i++) {
    bfly3(ytmpre128+i,
	  ytmpre128+48+i,
	  ytmpre128+96+i,
	  yre128+i,
	  yre128+48+i,
	  yre128+96+i,
	  twra128+i,
	  twrb128+i,
	  ytmpim128+i,
	  ytmpim128+i+48,
	  ytmpim128+i+96,
	  yim128+i,
	  yim128+i+48,
	  yim128+i+96,
	  twia128+i,
	  twib128+i);
  }
}

short twra180[472]__attribute__((aligned(16)))={32747,32747,32747,32747,32747,32747,32747,32747,
32687,32687,32687,32687,32687,32687,32687,32687,
32587,32587,32587,32587,32587,32587,32587,32587,
32448,32448,32448,32448,32448,32448,32448,32448,
32269,32269,32269,32269,32269,32269,32269,32269,
32050,32050,32050,32050,32050,32050,32050,32050,
31793,31793,31793,31793,31793,31793,31793,31793,
31497,31497,31497,31497,31497,31497,31497,31497,
31163,31163,31163,31163,31163,31163,31163,31163,
30790,30790,30790,30790,30790,30790,30790,30790,
30381,30381,30381,30381,30381,30381,30381,30381,
29934,29934,29934,29934,29934,29934,29934,29934,
29450,29450,29450,29450,29450,29450,29450,29450,
28931,28931,28931,28931,28931,28931,28931,28931,
28377,28377,28377,28377,28377,28377,28377,28377,
27787,27787,27787,27787,27787,27787,27787,27787,
27165,27165,27165,27165,27165,27165,27165,27165,
26509,26509,26509,26509,26509,26509,26509,26509,
25820,25820,25820,25820,25820,25820,25820,25820,
25100,25100,25100,25100,25100,25100,25100,25100,
24350,24350,24350,24350,24350,24350,24350,24350,
23570,23570,23570,23570,23570,23570,23570,23570,
22761,22761,22761,22761,22761,22761,22761,22761,
21925,21925,21925,21925,21925,21925,21925,21925,
21062,21062,21062,21062,21062,21062,21062,21062,
20173,20173,20173,20173,20173,20173,20173,20173,
19259,19259,19259,19259,19259,19259,19259,19259,
18323,18323,18323,18323,18323,18323,18323,18323,
17363,17363,17363,17363,17363,17363,17363,17363,
16383,16383,16383,16383,16383,16383,16383,16383,
15383,15383,15383,15383,15383,15383,15383,15383,
14364,14364,14364,14364,14364,14364,14364,14364,
13327,13327,13327,13327,13327,13327,13327,13327,
12274,12274,12274,12274,12274,12274,12274,12274,
11206,11206,11206,11206,11206,11206,11206,11206,
10125,10125,10125,10125,10125,10125,10125,10125,
9031,9031,9031,9031,9031,9031,9031,9031,
7927,7927,7927,7927,7927,7927,7927,7927,
6812,6812,6812,6812,6812,6812,6812,6812,
5689,5689,5689,5689,5689,5689,5689,5689,
4560,4560,4560,4560,4560,4560,4560,4560,
3425,3425,3425,3425,3425,3425,3425,3425,
2285,2285,2285,2285,2285,2285,2285,2285,
1143,1143,1143,1143,1143,1143,1143,1143,
0,0,0,0,0,0,0,0,
-1143,-1143,-1143,-1143,-1143,-1143,-1143,-1143,
-2285,-2285,-2285,-2285,-2285,-2285,-2285,-2285,
-3425,-3425,-3425,-3425,-3425,-3425,-3425,-3425,
-4560,-4560,-4560,-4560,-4560,-4560,-4560,-4560,
-5689,-5689,-5689,-5689,-5689,-5689,-5689,-5689,
-6812,-6812,-6812,-6812,-6812,-6812,-6812,-6812,
-7927,-7927,-7927,-7927,-7927,-7927,-7927,-7927,
-9031,-9031,-9031,-9031,-9031,-9031,-9031,-9031,
-10125,-10125,-10125,-10125,-10125,-10125,-10125,-10125,
-11206,-11206,-11206,-11206,-11206,-11206,-11206,-11206,
-12274,-12274,-12274,-12274,-12274,-12274,-12274,-12274,
-13327,-13327,-13327,-13327,-13327,-13327,-13327,-13327,
-14364,-14364,-14364,-14364,-14364,-14364,-14364,-14364,
  -15383,-15383,-15383,-15383,-15383,-15383,-15383,-15383};


short twia180[472]__attribute__((aligned(16))) = {-1143,-1143,-1143,-1143,-1143,-1143,-1143,-1143,
-2285,-2285,-2285,-2285,-2285,-2285,-2285,-2285,
-3425,-3425,-3425,-3425,-3425,-3425,-3425,-3425,
-4560,-4560,-4560,-4560,-4560,-4560,-4560,-4560,
-5689,-5689,-5689,-5689,-5689,-5689,-5689,-5689,
-6812,-6812,-6812,-6812,-6812,-6812,-6812,-6812,
-7927,-7927,-7927,-7927,-7927,-7927,-7927,-7927,
-9031,-9031,-9031,-9031,-9031,-9031,-9031,-9031,
-10125,-10125,-10125,-10125,-10125,-10125,-10125,-10125,
-11206,-11206,-11206,-11206,-11206,-11206,-11206,-11206,
-12274,-12274,-12274,-12274,-12274,-12274,-12274,-12274,
-13327,-13327,-13327,-13327,-13327,-13327,-13327,-13327,
-14364,-14364,-14364,-14364,-14364,-14364,-14364,-14364,
-15383,-15383,-15383,-15383,-15383,-15383,-15383,-15383,
-16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
-17363,-17363,-17363,-17363,-17363,-17363,-17363,-17363,
-18323,-18323,-18323,-18323,-18323,-18323,-18323,-18323,
-19259,-19259,-19259,-19259,-19259,-19259,-19259,-19259,
-20173,-20173,-20173,-20173,-20173,-20173,-20173,-20173,
-21062,-21062,-21062,-21062,-21062,-21062,-21062,-21062,
-21925,-21925,-21925,-21925,-21925,-21925,-21925,-21925,
-22761,-22761,-22761,-22761,-22761,-22761,-22761,-22761,
-23570,-23570,-23570,-23570,-23570,-23570,-23570,-23570,
-24350,-24350,-24350,-24350,-24350,-24350,-24350,-24350,
-25100,-25100,-25100,-25100,-25100,-25100,-25100,-25100,
-25820,-25820,-25820,-25820,-25820,-25820,-25820,-25820,
-26509,-26509,-26509,-26509,-26509,-26509,-26509,-26509,
-27165,-27165,-27165,-27165,-27165,-27165,-27165,-27165,
-27787,-27787,-27787,-27787,-27787,-27787,-27787,-27787,
-28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
-28931,-28931,-28931,-28931,-28931,-28931,-28931,-28931,
-29450,-29450,-29450,-29450,-29450,-29450,-29450,-29450,
-29934,-29934,-29934,-29934,-29934,-29934,-29934,-29934,
-30381,-30381,-30381,-30381,-30381,-30381,-30381,-30381,
-30790,-30790,-30790,-30790,-30790,-30790,-30790,-30790,
-31163,-31163,-31163,-31163,-31163,-31163,-31163,-31163,
-31497,-31497,-31497,-31497,-31497,-31497,-31497,-31497,
-31793,-31793,-31793,-31793,-31793,-31793,-31793,-31793,
-32050,-32050,-32050,-32050,-32050,-32050,-32050,-32050,
-32269,-32269,-32269,-32269,-32269,-32269,-32269,-32269,
-32448,-32448,-32448,-32448,-32448,-32448,-32448,-32448,
-32587,-32587,-32587,-32587,-32587,-32587,-32587,-32587,
-32687,-32687,-32687,-32687,-32687,-32687,-32687,-32687,
-32747,-32747,-32747,-32747,-32747,-32747,-32747,-32747,
-32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767,
-32747,-32747,-32747,-32747,-32747,-32747,-32747,-32747,
-32687,-32687,-32687,-32687,-32687,-32687,-32687,-32687,
-32587,-32587,-32587,-32587,-32587,-32587,-32587,-32587,
-32448,-32448,-32448,-32448,-32448,-32448,-32448,-32448,
-32269,-32269,-32269,-32269,-32269,-32269,-32269,-32269,
-32050,-32050,-32050,-32050,-32050,-32050,-32050,-32050,
-31793,-31793,-31793,-31793,-31793,-31793,-31793,-31793,
-31497,-31497,-31497,-31497,-31497,-31497,-31497,-31497,
-31163,-31163,-31163,-31163,-31163,-31163,-31163,-31163,
-30790,-30790,-30790,-30790,-30790,-30790,-30790,-30790,
-30381,-30381,-30381,-30381,-30381,-30381,-30381,-30381,
-29934,-29934,-29934,-29934,-29934,-29934,-29934,-29934,
-29450,-29450,-29450,-29450,-29450,-29450,-29450,-29450,
  -28931,-28931,-28931,-28931,-28931,-28931,-28931,-28931};

short twrb180[472]__attribute__((aligned(16))) ={
32687,32687,32687,32687,32687,32687,32687,32687,
32448,32448,32448,32448,32448,32448,32448,32448,
32050,32050,32050,32050,32050,32050,32050,32050,
31497,31497,31497,31497,31497,31497,31497,31497,
30790,30790,30790,30790,30790,30790,30790,30790,
29934,29934,29934,29934,29934,29934,29934,29934,
28931,28931,28931,28931,28931,28931,28931,28931,
27787,27787,27787,27787,27787,27787,27787,27787,
26509,26509,26509,26509,26509,26509,26509,26509,
25100,25100,25100,25100,25100,25100,25100,25100,
23570,23570,23570,23570,23570,23570,23570,23570,
21925,21925,21925,21925,21925,21925,21925,21925,
20173,20173,20173,20173,20173,20173,20173,20173,
18323,18323,18323,18323,18323,18323,18323,18323,
16383,16383,16383,16383,16383,16383,16383,16383,
14364,14364,14364,14364,14364,14364,14364,14364,
12274,12274,12274,12274,12274,12274,12274,12274,
10125,10125,10125,10125,10125,10125,10125,10125,
7927,7927,7927,7927,7927,7927,7927,7927,
5689,5689,5689,5689,5689,5689,5689,5689,
3425,3425,3425,3425,3425,3425,3425,3425,
1143,1143,1143,1143,1143,1143,1143,1143,
-1143,-1143,-1143,-1143,-1143,-1143,-1143,-1143,
-3425,-3425,-3425,-3425,-3425,-3425,-3425,-3425,
-5689,-5689,-5689,-5689,-5689,-5689,-5689,-5689,
-7927,-7927,-7927,-7927,-7927,-7927,-7927,-7927,
-10125,-10125,-10125,-10125,-10125,-10125,-10125,-10125,
-12274,-12274,-12274,-12274,-12274,-12274,-12274,-12274,
-14364,-14364,-14364,-14364,-14364,-14364,-14364,-14364,
-16383,-16383,-16383,-16383,-16383,-16383,-16383,-16383,
-18323,-18323,-18323,-18323,-18323,-18323,-18323,-18323,
-20173,-20173,-20173,-20173,-20173,-20173,-20173,-20173,
-21925,-21925,-21925,-21925,-21925,-21925,-21925,-21925,
-23570,-23570,-23570,-23570,-23570,-23570,-23570,-23570,
-25100,-25100,-25100,-25100,-25100,-25100,-25100,-25100,
-26509,-26509,-26509,-26509,-26509,-26509,-26509,-26509,
-27787,-27787,-27787,-27787,-27787,-27787,-27787,-27787,
-28931,-28931,-28931,-28931,-28931,-28931,-28931,-28931,
-29934,-29934,-29934,-29934,-29934,-29934,-29934,-29934,
-30790,-30790,-30790,-30790,-30790,-30790,-30790,-30790,
-31497,-31497,-31497,-31497,-31497,-31497,-31497,-31497,
-32050,-32050,-32050,-32050,-32050,-32050,-32050,-32050,
-32448,-32448,-32448,-32448,-32448,-32448,-32448,-32448,
-32687,-32687,-32687,-32687,-32687,-32687,-32687,-32687,
-32767,-32767,-32767,-32767,-32767,-32767,-32767,-32767,
-32687,-32687,-32687,-32687,-32687,-32687,-32687,-32687,
-32448,-32448,-32448,-32448,-32448,-32448,-32448,-32448,
-32050,-32050,-32050,-32050,-32050,-32050,-32050,-32050,
-31497,-31497,-31497,-31497,-31497,-31497,-31497,-31497,
-30790,-30790,-30790,-30790,-30790,-30790,-30790,-30790,
-29934,-29934,-29934,-29934,-29934,-29934,-29934,-29934,
-28931,-28931,-28931,-28931,-28931,-28931,-28931,-28931,
-27787,-27787,-27787,-27787,-27787,-27787,-27787,-27787,
-26509,-26509,-26509,-26509,-26509,-26509,-26509,-26509,
-25100,-25100,-25100,-25100,-25100,-25100,-25100,-25100,
-23570,-23570,-23570,-23570,-23570,-23570,-23570,-23570,
-21925,-21925,-21925,-21925,-21925,-21925,-21925,-21925,
-20173,-20173,-20173,-20173,-20173,-20173,-20173,-20173,
-18323,-18323,-18323,-18323,-18323,-18323,-18323,-18323};

short twib180[472]__attribute__((aligned(16))) ={
-2285,-2285,-2285,-2285,-2285,-2285,-2285,-2285,
-4560,-4560,-4560,-4560,-4560,-4560,-4560,-4560,
-6812,-6812,-6812,-6812,-6812,-6812,-6812,-6812,
-9031,-9031,-9031,-9031,-9031,-9031,-9031,-9031,
-11206,-11206,-11206,-11206,-11206,-11206,-11206,-11206,
-13327,-13327,-13327,-13327,-13327,-13327,-13327,-13327,
-15383,-15383,-15383,-15383,-15383,-15383,-15383,-15383,
-17363,-17363,-17363,-17363,-17363,-17363,-17363,-17363,
-19259,-19259,-19259,-19259,-19259,-19259,-19259,-19259,
-21062,-21062,-21062,-21062,-21062,-21062,-21062,-21062,
-22761,-22761,-22761,-22761,-22761,-22761,-22761,-22761,
-24350,-24350,-24350,-24350,-24350,-24350,-24350,-24350,
-25820,-25820,-25820,-25820,-25820,-25820,-25820,-25820,
-27165,-27165,-27165,-27165,-27165,-27165,-27165,-27165,
-28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
-29450,-29450,-29450,-29450,-29450,-29450,-29450,-29450,
-30381,-30381,-30381,-30381,-30381,-30381,-30381,-30381,
-31163,-31163,-31163,-31163,-31163,-31163,-31163,-31163,
-31793,-31793,-31793,-31793,-31793,-31793,-31793,-31793,
-32269,-32269,-32269,-32269,-32269,-32269,-32269,-32269,
-32587,-32587,-32587,-32587,-32587,-32587,-32587,-32587,
-32747,-32747,-32747,-32747,-32747,-32747,-32747,-32747,
-32747,-32747,-32747,-32747,-32747,-32747,-32747,-32747,
-32587,-32587,-32587,-32587,-32587,-32587,-32587,-32587,
-32269,-32269,-32269,-32269,-32269,-32269,-32269,-32269,
-31793,-31793,-31793,-31793,-31793,-31793,-31793,-31793,
-31163,-31163,-31163,-31163,-31163,-31163,-31163,-31163,
-30381,-30381,-30381,-30381,-30381,-30381,-30381,-30381,
-29450,-29450,-29450,-29450,-29450,-29450,-29450,-29450,
-28377,-28377,-28377,-28377,-28377,-28377,-28377,-28377,
-27165,-27165,-27165,-27165,-27165,-27165,-27165,-27165,
-25820,-25820,-25820,-25820,-25820,-25820,-25820,-25820,
-24350,-24350,-24350,-24350,-24350,-24350,-24350,-24350,
-22761,-22761,-22761,-22761,-22761,-22761,-22761,-22761,
-21062,-21062,-21062,-21062,-21062,-21062,-21062,-21062,
-19259,-19259,-19259,-19259,-19259,-19259,-19259,-19259,
-17363,-17363,-17363,-17363,-17363,-17363,-17363,-17363,
-15383,-15383,-15383,-15383,-15383,-15383,-15383,-15383,
-13327,-13327,-13327,-13327,-13327,-13327,-13327,-13327,
-11206,-11206,-11206,-11206,-11206,-11206,-11206,-11206,
-9031,-9031,-9031,-9031,-9031,-9031,-9031,-9031,
-6812,-6812,-6812,-6812,-6812,-6812,-6812,-6812,
-4560,-4560,-4560,-4560,-4560,-4560,-4560,-4560,
-2285,-2285,-2285,-2285,-2285,-2285,-2285,-2285,
0,0,0,0,0,0,0,0,
2285,2285,2285,2285,2285,2285,2285,2285,
4560,4560,4560,4560,4560,4560,4560,4560,
6812,6812,6812,6812,6812,6812,6812,6812,
9031,9031,9031,9031,9031,9031,9031,9031,
11206,11206,11206,11206,11206,11206,11206,11206,
13327,13327,13327,13327,13327,13327,13327,13327,
15383,15383,15383,15383,15383,15383,15383,15383,
17363,17363,17363,17363,17363,17363,17363,17363,
19259,19259,19259,19259,19259,19259,19259,19259,
21062,21062,21062,21062,21062,21062,21062,21062,
22761,22761,22761,22761,22761,22761,22761,22761,
24350,24350,24350,24350,24350,24350,24350,24350,
25820,25820,25820,25820,25820,25820,25820,25820,
27165,27165,27165,27165,27165,27165,27165,27165};

void dft180(short *xre,short *yre, short *xim, short *yim){

  int i,j;
  __m128i *xre128=(__m128i *)xre;
  __m128i *yre128=(__m128i *)yre;
  __m128i *xim128=(__m128i *)xim;
  __m128i *yim128=(__m128i *)yim;
  __m128i *twra128=(__m128i *)&twra180[0];
  __m128i *twia128=(__m128i *)&twia180[0];
  __m128i *twrb128=(__m128i *)&twrb180[0];
  __m128i *twib128=(__m128i *)&twib180[0];
  __m128i x2re128array[180],x2im128array[180];
  __m128i *x2re128 = (__m128i *)&x2re128array[0],*x2im128 = (__m128i *)&x2im128array[0];
  __m128i ytmpre128array[180],ytmpim128array[180];
  __m128i *ytmpre128=&ytmpre128array[0],*ytmpim128=&ytmpim128array[0];


  for (i=0,j=0;i<60;i++,j+=3) {
    x2re128[i]    = xre128[j];
    x2re128[i+60] = xre128[j+1];
    x2re128[i+120] = xre128[j+2];
    x2im128[i]    = xim128[j];
    x2im128[i+60] = xim128[j+1];
    x2im128[i+120] = xim128[j+2];
  }

  dft60((short *)x2re128,(short *)ytmpre128,(short *)xim128,(short *)ytmpim128);
  dft60((short *)(x2re128+60),(short *)(ytmpre128+60),(short *)(x2im128+60),(short *)(ytmpim128+60));
  dft60((short *)(x2re128+120),(short *)(ytmpre128+120),(short *)(x2im128+120),(short *)(ytmpim128+120));

  bfly3_tw1(ytmpre128,ytmpre128+60,ytmpre128+120,yre128,yre128+60,yre128+120,ytmpim128,ytmpim128+60,ytmpim128+120,yim128,yim128+60,yim128+120);
  for (i=0;i<59;i++) {
    bfly3(ytmpre128+i,
	  ytmpre128+60+i,
	  ytmpre128+120+i,
	  yre128+i,
	  yre128+60+i,
	  yre128+120+i,
	  twra128+i,
	  twrb128+i,
	  ytmpim128+i,
	  ytmpim128+i+60,
	  ytmpim128+i+120,
	  yim128+i,
	  yim128+i+60,
	  yim128+i+120,
	  twia128+i,
	  twib128+i);
  }
}

/*
void dft192(short *xre,short *yre, short *xim, short *yim){


  int i,j;
  __m128i *xre128=(__m128i *)xre;
  __m128i *yre128=(__m128i *)yre;
  __m128i *xim128=(__m128i *)xim;
  __m128i *yim128=(__m128i *)yim;
  __m128i *twra128=(__m128i *)&twra192[0];
  __m128i *twia128=(__m128i *)&twia192[0];
  __m128i *twrb128=(__m128i *)&twrb192[0];
  __m128i *twib128=(__m128i *)&twib192[0];
  __m128i *twrc128=(__m128i *)&twrc192[0];
  __m128i *twic128=(__m128i *)&twic192[0];
  __m128i x2re128array[192],x2im128array[192];
  __m128i *x2re128 = (__m128i *)&x2re128array[0],*x2im128 = (__m128i *)&x2im128array[0];
  __m128i ytmpre128array[192],ytmpim128array[192];
  __m128i *ytmpre128=&ytmpre128array[0],*ytmpim128=&ytmpim128array[0];


  for (i=0,j=0;i<48;i++,j+=4) {
    x2re128[i]     = xre128[j];
    x2re128[i+48]  = xre128[j+1];
    x2re128[i+96]  = xre128[j+2];
    x2re128[i+144] = xre128[j+3];
    x2im128[i]     = xim128[j];
    x2im128[i+48]  = xim128[j+1];
    x2im128[i+96]  = xim128[j+2];
    x2im128[i+144] = xim128[j+3];
  }

  dft48((short *)x2re128,(short *)ytmpre128,(short *)xim128,(short *)ytmpim128);
  dft48((short *)(x2re128+48),(short *)(ytmpre128+48),(short *)(x2im128+48),(short *)(ytmpim128+48));
  dft48((short *)(x2re128+96),(short *)(ytmpre128+96),(short *)(x2im128+96),(short *)(ytmpim128+96));
  dft48((short *)(x2re128+144),(short *)(ytmpre128+144),(short *)(x2im128+144),(short *)(ytmpim128+144));

  bfly4_tw1(ytmpre128,ytmpre128+48,ytmpre128+96,ytmpre128+144,yre128,yre128+48,yre128+96,yre128+144,ytmpim128,ytmpim128+48,ytmpim128+96,ytmpim128+144,yim128,yim128+48,yim128+96,yim128+144);
  for (i=0;i<47;i++) {
    bfly4(ytmpre128+i,
	  ytmpre128+48+i,
	  ytmpre128+96+i,
	  ytmpre128+144+i,
	  yre128+i,
	  yre128+48+i,
	  yre128+96+i,
	  yre128+144+i,
	  twra128+i,
	  twrb128+i,
	  twrc128+i,
	  ytmpim128+i,
	  ytmpim128+i+48,
	  ytmpim128+i+96,
	  ytmpim128+i+144,
	  yim128+i,
	  yim128+i+48,
	  yim128+i+96,
	  yim128+i+144,
	  twia128+i,
	  twib128+i,
	  twic128+i);
  }
}


void dft240(short *xre,short *yre, short *xim, short *yim){

  dft60();
  dft60();
  dft60();
  dft60();
  
  bfly4_tw1();
  for (i=0;i<59;i++) {
    bfly4();
  }
}


void dft300(short *xre,short *yre, short *xim, short *yim){

  dft60();
  dft60();
  dft60();
  dft60();
  dft60();

  bfly5_tw1();
  for (i=0;i<59;i++) {
    bfly5();
  }
}
*/

#ifdef MR_MAIN
int main(int argc, char**argv) {


  __m128i xr[300],xi[300],yr[300],yi[300],twr,twi,twr2,twi2,twr3,twi3,twr4,twi4;

  int i;

  set_taus_seed(0);

  twr = _mm_set1_epi16(14570);
  twi = _mm_set1_epi16(29351);
  twr2 = _mm_set1_epi16(29649);
  twi2 = _mm_set1_epi16(13952);
  twr3 = _mm_set1_epi16(5465);
  twi3 = _mm_set1_epi16(32309);
  twr4 = _mm_set1_epi16(4107);
  twi4 = _mm_set1_epi16(-32510);

  for (i=0;i<300;i++) 
    xr[i] = _mm_set1_epi16(taus()>>24);

  bfly2(xr,xr+1,yr, yr+1,&twr,xi,xi+1,yi,yi+1,&twi);
  printf("(%d,%d) (%d,%d) => (%d,%d) (%d,%d)\n",((short*)&xr[0])[0],((short*)&xi[0])[0],((short*)&xr[1])[1],((short*)&xi[1])[1],((short*)&yr[0])[0],((short*)&yi[0])[0],((short*)&yr[1])[1],((short*)&yi[1])[1]);

  bfly3(xr,xr+1,xr+2,yr, yr+1,yr+2,&twr,&twr2,xi,xi+1,xi+2,yi,yi+1,yi+2,&twi,&twi2);
  printf("(%d,%d) (%d,%d) (%d %d) => (%d,%d) (%d,%d) (%d %d)\n",((short*)&xr[0])[0],((short*)&xi[0])[0],((short*)&xr[1])[0],((short*)&xi[1])[0],((short*)&xr[2])[0],((short*)&xi[2])[0],((short*)&yr[0])[0],((short*)&yi[0])[0],((short*)&yr[1])[0],((short*)&yi[1])[0],((short*)&yr[2])[0],((short*)&yi[2])[0]);

  bfly4(xr,xr+1,xr+2,xr+3,yr, yr+1,yr+2,yr+3,&twr,&twr2,&twr3,xi,xi+1,xi+2,xi+3,yi,yi+1,yi+2,yi+3,&twi,&twi2,&twi3);
  printf("(%d,%d) (%d,%d) (%d %d) (%d,%d) => (%d,%d) (%d,%d) (%d %d) (%d,%d)\n",
	 ((short*)&xr[0])[0],((short*)&xi[0])[0],((short*)&xr[1])[0],((short*)&xi[1])[0],
	 ((short*)&xr[2])[0],((short*)&xi[2])[0],((short*)&xr[3])[0],((short*)&xi[3])[0],
	 ((short*)&yr[0])[0],((short*)&yi[0])[0],((short*)&yr[1])[0],((short*)&yi[1])[0],
	 ((short*)&yr[2])[0],((short*)&yi[2])[0],((short*)&yr[3])[0],((short*)&yi[3])[0]);

  //  bfly5(xr,xr+1,xr+2,xr+3,xr+4,yr, yr+1,yr+2,yr+3,yr+4,&twr,&twr2,&twr3,&twr4,xi,xi+1,xi+2,xi+3,xi+4,yi,yi+1,yi+2,yi+3,yi+4,&twi,&twi2,&twi3,&twi4);
  bfly5_tw1(xr,xr+1,xr+2,xr+3,xr+4,yr, yr+1,yr+2,yr+3,yr+4,xi,xi+1,xi+2,xi+3,xi+4,yi,yi+1,yi+2,yi+3,yi+4);

  for (i=0;i<5;i++)
    printf("%d,%d,",
	   ((short*)&xr[i])[0],((short*)&xi[i])[0]);
  printf("\n");
  for (i=0;i<5;i++)
    printf("%d,%d,",
	   ((short*)&yr[i])[0],((short*)&yi[i])[0]);
  printf("\n");

  dft12(xr,
	xr+1,
	xr+2,
	xr+3,
	xr+4,
	xr+5,
	xr+6,
	xr+7,
	xr+8,
	xr+9,
	xr+10,
	xr+11,
	yr,
	yr+1,
	yr+2,
	yr+3,
	yr+4,
	yr+5,
	yr+6,
	yr+7,
	yr+8,
	yr+9,
	yr+10,
	yr+11,
	xi,
	xi+1,
	xi+2,
	xi+3,
	xi+4,
	xi+5,
	xi+6,
	xi+7,
	xi+8,
	xi+9,
	xi+10,
	xi+11,
	yi,
	yi+1,
	yi+2,
	yi+3,
	yi+4,
	yi+5,
	yi+6,
	yi+7,
	yi+8,
	yi+9,
	yi+10,
	yi+11);

  printf("\n\n12-point\n");
  printf("X: ");
  for (i=0;i<12;i++)
    printf("%d,%d,",((short*)(&xr[i]))[0],((short *)(&xi[i]))[0]);
  printf("\nY:");
  for (i=0;i<12;i++)
    printf("%d,%d,",((short*)(&yr[i]))[0],((short *)(&yi[i]))[0]);
  printf("\n");

  dft24((short *)xr,(short *)yr,(short *)xi,(short *)yi);
  printf("\n\n24-point\n");
  printf("X: ");
  for (i=0;i<24;i++)
    printf("%d,%d,",((short*)(&xr[i]))[0],((short *)(&xi[i]))[0]);
  printf("\nY:");
  for (i=0;i<24;i++)
    printf("%d,%d,",((short*)(&yr[i]))[0],((short *)(&yi[i]))[0]);
  printf("\n");

  dft36((short *)xr,(short *)yr,(short *)xi,(short *)yi);
  printf("\n\n36-point\n");
  printf("X: ");
  for (i=0;i<36;i++)
    printf("%d,%d,",((short*)(&xr[i]))[0],((short *)(&xi[i]))[0]);
  printf("\nY:");
  for (i=0;i<36;i++)
    printf("%d,%d,",((short*)(&yr[i]))[0],((short *)(&yi[i]))[0]);
  printf("\n");


  return(0);
}


#endif
