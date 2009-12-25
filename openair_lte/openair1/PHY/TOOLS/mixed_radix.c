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

inline void bfly2(__m128i *x0r, __m128i *x1r,__m128i *y0r, __m128i *y1r,__m128i *twr,
		  __m128i *x0i, __m128i *x1i,__m128i *y0i, __m128i *y1i,__m128i *twi) {

  __m128i x1r_2,x1i_2;

  x1r_2 = cmultr(*x1r,*x1i,*twr,*twi);
  x1i_2 = cmulti(*x1r,*x1i,*twr,*twi);
  //  printf("x0i %d, x1i2 %d\n",((short *)x0i)[0],((short *)&x1i_2)[0]);
  printf("x1r %d x1r_2 %d, x1i %d x1i_2 %d\n",((short *)x1r)[0],((short *)&x1r_2)[0],((short *)x1i)[0],((short *)&x1i_2)[0]);
  *y0r = _mm_adds_epi16(*x0r,x1r_2);
  *y1r = _mm_subs_epi16(*x0r,x1r_2);
  *y0i = _mm_adds_epi16(*x0i,x1i_2);
  //  printf("y0i %d\n",((short *)y0i)[0]);
  *y1i = _mm_subs_epi16(*x0i,x1i_2);
}

inline void bfly3(__m128i *x0r, __m128i *x1r, __m128i *x2r,
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

  printf("x1r_2 %d, x1i_2 %d x2r_2 %d, x2i_2 %d\n",((short *)&x1r_2)[0],((short *)&x1i_2)[0],((short *)&x2r_2)[0],((short *)&x2i_2)[0]);

  tmp1 = _mm_adds_epi16(x1r_2,x2r_2);
  tmp2 = _mm_adds_epi16(x1i_2,x2i_2);


  *y0r = _mm_adds_epi16(*x0r,_mm_adds_epi16(x1r_2,x2r_2));

  *y1r = _mm_adds_epi16(*x0r,_mm_adds_epi16(cmultr(*W13r,*W13i,x1r_2,x1i_2),cmultr(*W23r,*W23i,x2r_2,x2i_2)));
  *y2r = _mm_adds_epi16(*x0r,_mm_adds_epi16(cmultr(*W23r,*W23i,x1r_2,x1i_2),cmultr(*W13r,*W13i,x2r_2,x2i_2)));

  *y0i = _mm_adds_epi16(*x0i,_mm_adds_epi16(x1i_2,x2i_2));
  *y1i = _mm_adds_epi16(*x0i,_mm_adds_epi16(cmulti(*W13r,*W13i,x1r_2,x1i_2),cmulti(*W23r,*W23i,x2r_2,x2i_2)));
  *y2i = _mm_adds_epi16(*x0i,_mm_adds_epi16(cmulti(*W23r,*W23i,x1r_2,x1i_2),cmulti(*W13r,*W13i,x2r_2,x2i_2)));
}

inline void bfly4(__m128i *x0r, __m128i *x1r, __m128i *x2r, __m128i *x3r,__m128i *y0r, __m128i *y1r,__m128i *y2r, __m128i *y3r,__m128i *twr1,__m128i *twr2,__m128i *twr3,
		  __m128i *x0i, __m128i *x1i, __m128i *x2i, __m128i *x3i,__m128i *y0i, __m128i *y1i,__m128i *y2i, __m128i *y3i,__m128i *twi1,__m128i *twi2,__m128i *twi3) {

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

inline void bfly5(__m128i *x0r, __m128i *x1r, __m128i *x2r, __m128i *x3r,__m128i *x4r,
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


int main(int argc, char**argv) {


  __m128i xr[5],xi[5],yr[5],yi[5],twr,twi,twr2,twi2,twr3,twi3,twr4,twi4;

  twr = _mm_set1_epi16(14570);
  twi = _mm_set1_epi16(29351);
  twr2 = _mm_set1_epi16(29649);
  twi2 = _mm_set1_epi16(13952);
  twr3 = _mm_set1_epi16(5465);
  twi3 = _mm_set1_epi16(32309);
  twr4 = _mm_set1_epi16(4107);
  twi4 = _mm_set1_epi16(-32510);

  xr[0] = _mm_set1_epi16(533);
  xi[0] = _mm_set1_epi16(233);
  xr[1] = _mm_set1_epi16(89);
  xi[1] = _mm_set1_epi16(122);
  xr[2] = _mm_set1_epi16(1889);
  xi[2] = _mm_set1_epi16(99);
  xr[3] = _mm_set1_epi16(188);
  xi[3] = _mm_set1_epi16(993);
  xr[4] = _mm_set1_epi16(2889);
  xi[4] = _mm_set1_epi16(199);

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

  bfly5(xr,xr+1,xr+2,xr+3,xr+4,yr, yr+1,yr+2,yr+3,yr+4,&twr,&twr2,&twr3,&twr4,xi,xi+1,xi+2,xi+3,xi+4,yi,yi+1,yi+2,yi+3,yi+4,&twi,&twi2,&twi3,&twi4);
  printf("(%d,%d) (%d,%d) (%d %d) (%d,%d) (%d,%d) => (%d,%d) (%d,%d) (%d %d) (%d,%d) (%d,%d)\n",
	 ((short*)&xr[0])[0],((short*)&xi[0])[0],((short*)&xr[1])[0],((short*)&xi[1])[0],
	 ((short*)&xr[2])[0],((short*)&xi[2])[0],((short*)&xr[3])[0],((short*)&xi[3])[0],
	 ((short*)&xr[4])[0],((short*)&xi[4])[0],
	 ((short*)&yr[0])[0],((short*)&yi[0])[0],((short*)&yr[1])[0],((short*)&yi[1])[0],
	 ((short*)&yr[2])[0],((short*)&yi[2])[0],((short*)&yr[3])[0],((short*)&yi[3])[0],
	 ((short*)&yr[4])[0],((short*)&yi[4])[0]);

  return(0);
}
