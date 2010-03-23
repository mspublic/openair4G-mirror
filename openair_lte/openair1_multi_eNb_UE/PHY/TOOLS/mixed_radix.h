#define cmultr(ar,ai,br,bi) (_mm_slli(_mm_subs(_mm_mulhi_epi16((ar),(br)),_mm_mulhi_epi16((br),(bi))),1))
#define cmulti(ar,ai,br,bi) (_mm_slli(_mm_adds(_mm_mulhi_epi16((ar),(bi)),_mm_mulhi_epi16((ai),(br))),1))

inline void bfly2(__m128i *x0r, __m128i *x1r,__m128i *y0r, __m128i *y1r,__m128i *twr,
	     __m128i *x0i, __m128i *x1i,__m128i *y0i, __m128i *y1i,__m128i *twi) {

  _mm128i x1r_2,x1i_2;

  x1r_2 = cmultr(*x1r,*x1i,*twr,*twi);
  x1i_2 = cmulti(*x1r,*x1i,*twr,*twi);
  *y0r = _mm_padds_epi16(*x0r,x1r_2);
  *y1r = _mm_subs_epi16(*x0r,x1r_2);
  *y0i = _mm_padds_epi16(*x0i,x1i_2);
  *y1i = _mm_subs_epi16(*x0i,x1i_2);
}

inline void bfly3(__m128i *x0r, __m128i *x1r, __m128i *x2r,__m128i *y0r, __m128i *y1r,__m128i *y1r,__m128i *twr1,*twr2,
		  __m128i *x0i, __m128i *x1i, __m128i *x2i,__m128i *y0i, __m128i *y1i,__m128i *y1i,__m128i *twi1,*twi2) {

  _mm128i x1r_2,x1i_2;
  _mm128i x2r_2,x2i_2;
  _mm128i tmp1,tmp2;

  x1r_2 = cmultr(*x1r,*x1i,*twr1,*twi1);
  x1i_2 = cmulti(*x1r,*x1i,*twr1,*twi1);
  x2r_2 = cmultr(*x2r,*x2i,*twr2,*twi2);
  x2i_2 = cmulti(*x2r,*x2i,*twr2,*twi2);

  tmp1 = _mm_padds(x1r_2,x2r_2);
  tmp2 = _mm_padds(x1i_2,x2i_2);

  *y0r = _mm_padds_epi16(*x0r,_mm_padds_epi16(x1r_2,x2r_2));
  *y1r = _mm_padds_epi16(*x0r,_mm_padds(cmultr(W13r,W13i,x1r_2,x1i_2),cmultr(W23r,W23i,x2r_2,x2i_2)));
  *y2r = _mm_padds_epi16(*x0r,cmultr(W13r,W13i,tmp1,tmp2));

  *y0i = _mm_padds_epi16(*x0i,_mm_padds_epi16(x1i_2,x2i_2));
  *y1i = _mm_padds_epi16(*x0i,_mm_padds(cmulti(W13r,W13i,x1r_2,x1i_2),cmulti(W23r,W23i,x2r_2,x2i_2)));
  *y2i = _mm_padds_epi16(*x0i,cmulti(W13r,W13i,tmp1,tmp2));
}



