#ifdef __SSE2__
#include <emmintrin.h>
#include <xmmintrin.h>
#endif
#ifdef __SSE3__
#include <pmmintrin.h>
#include <tmmintrin.h>
#endif
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "defs.h"
#include "extern.h"

#ifndef __SSE3__
__m128i zero;
#define _mm_abs_epi16(xmmx) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zero,(xmmx)))
#define _mm_sign_epi16(xmmx,xmmy) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zero,(xmmy)))
#endif

#define abs_pi16(x,zero,res,sign)     sign=_mm_cmpgt_pi16(zero,x) ; res=_mm_xor_si64(x,sign);   //negate negatives


//#define cmax(a,b) ((a)>(b) ? (a) : (b))

#define is_not_pilot(pilots,first_pilot,re) (pilots==0) || \
((pilots==1)&&(first_pilot==1)&&(((re>2)&&(re<6))||((re>8)&&(re<12)))) || \
((pilots==1)&&(first_pilot==0)&&(((re<3))||((re>5)&&(re<9))))


__m128i mmtmpD0,mmtmpD1,mmtmpD2,mmtmpD3;

//#define DEBUG_DLSCH_DEMOD
//#define DEBUG_PHY 1

#ifdef DEBUG_DLSCH_DEMOD
/*
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

void print_shorts2(char *s,__m64 *x) {

  short *tempb = (short *)x;

  printf("%s  : %d,%d,%d,%d\n",s,
         tempb[0],tempb[1],tempb[2],tempb[3]);

}

void print_ints(char *s,__m128i *x) {

  int *tempb = (int *)x;

  printf("%s  : %d,%d,%d,%d\n",s,
         tempb[0],tempb[1],tempb[2],tempb[3]
         );

}
*/
#endif




#ifndef USER_MODE
#define NOCYGWIN_STATIC static
#else
#define NOCYGWIN_STATIC 
#endif

NOCYGWIN_STATIC __m64 ONE_OVER_SQRT_8  __attribute__((aligned(16))); 



NOCYGWIN_STATIC __m64 rho_rpi __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  rho_rmi __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0r __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y1r __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y1i __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0r_over2 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y0i_over2 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y1r_over2 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  y1i_over2 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  xmm0 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  xmm1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  xmm2 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  xmm3 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64 logmax_num_re0 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  logmax_num_im0 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  logmax_den_re0 __attribute__ ((aligned(16))); 
NOCYGWIN_STATIC __m64  logmax_den_im0 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  logmax_num_re1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  logmax_num_im1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  logmax_den_re1 __attribute__ ((aligned(16))); 
NOCYGWIN_STATIC __m64  logmax_den_im1 __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  A __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  B __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  C __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  D __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  E __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  F __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  G __attribute__ ((aligned(16)));
NOCYGWIN_STATIC __m64  H __attribute__ ((aligned(16))); 


void qpsk_qpsk(short *stream0_in,
	       short *stream1_in,
	       short *stream0_out,
	       short *rho01,
	       int length
	       ) {

  __m64 *rho01_64 = (__m64 *)rho01;
  __m64 *stream0_64_in = (__m64 *)stream0_in;
  __m64 *stream1_64_in = (__m64 *)stream1_in;
  __m64 *stream0_64_out = (__m64 *)stream0_out;


  int i;

  ((short*)&ONE_OVER_SQRT_8)[0] = 23170;
  ((short*)&ONE_OVER_SQRT_8)[1] = 23170;
  ((short*)&ONE_OVER_SQRT_8)[2] = 23170;
  ((short*)&ONE_OVER_SQRT_8)[3] = 23170;

  for (i=0;i<length>>1;i+=2) {



    // STREAM 0


    xmm0 = rho01_64[i];
    xmm1 = rho01_64[i+1];

    
    //    print_shorts2("rho01_0:",&xmm0);
    //    print_shorts2("rho01_1:",&xmm1);    
    
      // put (rho_r + rho_i)/2sqrt2 in rho_rpi
      // put (rho_r - rho_i)/2sqrt2 in rho_rmi
    
    xmm0 = _mm_shuffle_pi16(xmm0,0xd8);//_MM_SHUFFLE(0,2,1,3));
    xmm1 = _mm_shuffle_pi16(xmm1,0xd8);//_MM_SHUFFLE(0,2,1,3));
    
    
    xmm2 = _mm_unpacklo_pi32(xmm0,xmm1);
    xmm3 = _mm_unpackhi_pi32(xmm0,xmm1);
    
    
    
    rho_rpi = _mm_adds_pi16(xmm2,xmm3);
    rho_rmi = _mm_subs_pi16(xmm2,xmm3);
    
    
    rho_rpi = _mm_mulhi_pi16(rho_rpi,ONE_OVER_SQRT_8);
    rho_rmi = _mm_mulhi_pi16(rho_rmi,ONE_OVER_SQRT_8);

    //    print_shorts2("rho_rpi:",&rho_rpi);
    //    print_shorts2("rho_rmi:",&rho_rmi);    

    xmm0 = stream0_64_in[i];
    xmm1 = stream0_64_in[i+1];
    //    print_shorts2("y0_0:",&xmm0);
    //    print_shorts2("y0_1:",&xmm1);        

    xmm0 = _mm_shuffle_pi16(xmm0,0xd8);//_MM_SHUFFLE(0,2,1,3));
    xmm1 = _mm_shuffle_pi16(xmm1,0xd8);//_MM_SHUFFLE(0,2,1,3));
    y0r  = _mm_unpacklo_pi32(xmm0,xmm1);
    y0r_over2  = _mm_srai_pi16(y0r,1);
    y0i  = _mm_unpackhi_pi32(xmm0,xmm1);
    y0i_over2  = _mm_srai_pi16(y0i,1);
    
    xmm0 = stream1_64_in[i];
    xmm1 = stream1_64_in[i+1];
    //    print_shorts2("y1_0:",&xmm0);
    //    print_shorts2("y1_1:",&xmm1);        
    
    xmm0 = _mm_shuffle_pi16(xmm0,0xd8);//_MM_SHUFFLE(0,2,1,3));
    xmm1 = _mm_shuffle_pi16(xmm1,0xd8);//_MM_SHUFFLE(0,2,1,3));
    y1r  = _mm_unpacklo_pi32(xmm0,xmm1);
    y1r_over2  = _mm_srai_pi16(y1r,1);
    y1i  = _mm_unpackhi_pi32(xmm0,xmm1);
    y1i_over2  = _mm_srai_pi16(y1i,1);
    
    // Detection for y0r
    
    xmm0 = _mm_xor_si64(xmm0,xmm0);   // ZERO
    
    xmm3 = _mm_subs_pi16(y1r_over2,rho_rpi); 
    abs_pi16(xmm3,xmm0,A,xmm1);       
    xmm2 = _mm_adds_pi16(A,y0i_over2); 
    xmm3 = _mm_subs_pi16(y1i_over2,rho_rmi); 
    abs_pi16(xmm3,xmm0,B,xmm1);       
    logmax_num_re0 = _mm_adds_pi16(B,xmm2); 
    //    print_shorts2("logmax_num_re:",&logmax_num_re0);

    xmm3 = _mm_subs_pi16(y1r_over2,rho_rmi); 
    abs_pi16(xmm3,xmm0,C,xmm1);       
    xmm2 = _mm_subs_pi16(C,y0i_over2); 
    xmm3 = _mm_adds_pi16(y1i_over2,rho_rpi); 
    abs_pi16(xmm3,xmm0,D,xmm1);       
    xmm2 = _mm_adds_pi16(xmm2,D); 
    logmax_num_re0 = _mm_max_pi16(logmax_num_re0,xmm2);  
    //    print_shorts2("logmax_num_re:",&logmax_num_re0);

    xmm3 = _mm_adds_pi16(y1r_over2,rho_rmi); 
    abs_pi16(xmm3,xmm0,E,xmm1);       
    xmm2 = _mm_adds_pi16(E,y0i_over2); 
    xmm3 = _mm_subs_pi16(y1i_over2,rho_rpi); 
    abs_pi16(xmm3,xmm0,F,xmm1);       
    logmax_den_re0 = _mm_adds_pi16(F,xmm2); 
    //    print_shorts2("logmax_den_re:",&logmax_den_re0);
    
    xmm3 = _mm_adds_pi16(y1r_over2,rho_rpi); 
    abs_pi16(xmm3,xmm0,G,xmm1);       
    xmm2 = _mm_subs_pi16(G,y0i_over2); 
    xmm3 = _mm_adds_pi16(y1i_over2,rho_rmi); 
    abs_pi16(xmm3,xmm0,H,xmm1);       
    xmm2 = _mm_adds_pi16(xmm2,H); 
    
    logmax_den_re0 = _mm_max_pi16(logmax_den_re0,xmm2);  
    //    print_shorts2("logmax_den_re:",&logmax_num_re0);

    // Detection for y0i
    
    xmm2 = _mm_adds_pi16(A,y0r_over2); 
    logmax_num_im0 = _mm_adds_pi16(B,xmm2); 
    
    xmm2 = _mm_subs_pi16(E,y0r_over2); 
    xmm2 = _mm_adds_pi16(xmm2,F); 
    
    logmax_num_im0 = _mm_max_pi16(logmax_num_im0,xmm2);
    
    xmm2 = _mm_adds_pi16(C,y0r_over2); 
    logmax_den_im0 = _mm_adds_pi16(D,xmm2); 
    
    xmm2 = _mm_subs_pi16(G,y0r_over2); 
    xmm2 = _mm_adds_pi16(xmm2,H); 
    
    logmax_den_im0 = _mm_max_pi16(logmax_den_im0,xmm2);  
    
    y0r = _mm_adds_pi16(y0r,logmax_num_re0);
    y0r = _mm_subs_pi16(y0r,logmax_den_re0);
    
    y0i = _mm_adds_pi16(y0i,logmax_num_im0);
    y0i = _mm_subs_pi16(y0i,logmax_den_im0);
    
    
    stream0_64_out[i] = _mm_unpacklo_pi16(y0r,y0i);
    if (i<((length>>1) - 1))
      stream0_64_out[i+1] = _mm_unpackhi_pi16(y0r,y0i);
    
  }

  _mm_empty();
  _m_empty();

}


int dlsch_qpsk_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
			int **rxdataF_comp,
			int **rxdataF_comp_i,
			int **rho_i,
			short *dlsch_llr,
			unsigned char symbol,
			unsigned char first_symbol_flag,
			unsigned short nb_rb,
			u16 pbch_pss_sss_adjust,
			short **llr128p) {

  __m128i *rxF=(__m128i*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
  __m128i *rxF_i=(__m128i*)&rxdataF_comp_i[0][(symbol*frame_parms->N_RB_DL*12)];
  __m128i *rho=(__m128i*)&rho_i[0][(symbol*frame_parms->N_RB_DL*12)];
  __m128i *llr128;
  int len;
  u8 symbol_mod = (symbol >= (7-frame_parms->Ncp))? (symbol-(7-frame_parms->Ncp)) : symbol;
  //  printf("dlsch_qpsk_qpsk: symbol %d\n",symbol);
  
  if (first_symbol_flag == 1) {
    llr128 = (__m128i*)dlsch_llr;
  }
  else {
    llr128 = (__m128i*)(*llr128p);
  }
  

  if (!llr128) {
    msg("dlsch_qpsk_qpsk_llr: llr is null, symbol %d\n",symbol);
    return -1;
  }

  if ((symbol_mod==0) || (symbol_mod==(4-frame_parms->Ncp))) {
    // if symbol has pilots
    if (frame_parms->mode1_flag==0)
      // in 2 antenna ports we have 8 REs per symbol per RB
      len = (nb_rb*8) - (2*pbch_pss_sss_adjust/3);
    else
      // for 1 antenna port we have 10 REs per symbol per RB 
      len = (nb_rb*10) - (5*pbch_pss_sss_adjust/6);
  }
  else {
    // symbol has no pilots
    len = (nb_rb*12) - pbch_pss_sss_adjust;
  }

  qpsk_qpsk((short *)rxF,
	    (short *)rxF_i,
	    (short *)llr128,
	    (short *)rho,
	    len);

  llr128 += (len>>2);
  *llr128p = (short *)llr128;

  return(0);
}

int dlsch_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
		   int **rxdataF_comp,
		   short *dlsch_llr,
		   unsigned char symbol,
		   u8 first_symbol_flag,
		   u16 nb_rb,
		   u16 pbch_pss_sss_adjust,
		   short **llr32p) {

  u32 *rxF=(u32*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
  u32 *llr32;
  int i,len;
  u8 symbol_mod = (symbol >= (7-frame_parms->Ncp))? (symbol-(7-frame_parms->Ncp)) : symbol;

  if (first_symbol_flag==1) {
    llr32 = (u32*)dlsch_llr;
  }
  else {
    llr32 = (u32*)(*llr32p);
  }
 
  if (!llr32) {
    msg("dlsch_qpsk_llr: llr is null, symbol %d, llr32=%p\n",symbol, llr32);
    return(-1);
  } 


  if ((symbol_mod==0) || (symbol_mod==(4-frame_parms->Ncp))) {
    if (frame_parms->mode1_flag==0)
      len = (nb_rb*8) - (2*pbch_pss_sss_adjust/3);
    else
      len = (nb_rb*10) - (5*pbch_pss_sss_adjust/6);
  }
  else {
    len = (nb_rb*12) - pbch_pss_sss_adjust;
  }
  //  printf("qpsk llr for symbol %d (len %d,pos %d,nb_rb %d), llr offset %d\n",symbol,len,(symbol*frame_parms->N_RB_DL*12),nb_rb,llr32-(u32*)dlsch_llr);

  for (i=0;i<len;i++) {
    *llr32 = *rxF;
    rxF++;
    llr32++;
  }

  *llr32p = (short *)llr32;

  _mm_empty();
  _m_empty();

  return(0);

}

void dlsch_16qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
		     int **rxdataF_comp,
		     short *dlsch_llr,
		     int **dl_ch_mag,
		     unsigned char symbol,
		     u8 first_symbol_flag,
		     unsigned short nb_rb,
		     u16 pbch_pss_sss_adjust,
		     short **llr128p) {

  __m128i *rxF=(__m128i*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
  __m128i *ch_mag;
  __m128i *llr128;
  int i,len;
  unsigned char symbol_mod,len_mod4=0;
  
//  printf("dlsch_rx.c: dlsch_16qam_llr: symbol %d\n",symbol);

  if (first_symbol_flag==1) {
    llr128 = (__m128i*)dlsch_llr;
  }
  else {
    llr128 = (__m128i*)(*llr128p);
  }
  
  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;

  ch_mag =(__m128i*)&dl_ch_mag[0][(symbol*frame_parms->N_RB_DL*12)];

  if ((symbol_mod==0) || (symbol_mod==(4-frame_parms->Ncp))) {
    if (frame_parms->mode1_flag==0)
      len = nb_rb*8 - (2*pbch_pss_sss_adjust/3);
    else
      len = nb_rb*10 - (5*pbch_pss_sss_adjust/6);
  }
  else {
    len = nb_rb*12 - pbch_pss_sss_adjust;
  }

  //  printf("16qam llr symbol %d : len %d, pbch_pss_sss_adjust %d\n",symbol,len,pbch_pss_sss_adjust);
  len_mod4 = len&3;
  len>>=2;  // length in quad words (4 REs)
  len+=(len_mod4>>1);
  //printf("16qam llr symbol %d : len %d (%d)\n",symbol,len,len_mod4);
  for (i=0;i<len;i++) {


    mmtmpD0 = _mm_abs_epi16(rxF[i]);
    //    print_shorts("tmp0",&tmp0);

    //    mmtmpD0 = _mm_subs_epi16(mmtmpD0,ch_mag[i]);
    mmtmpD0 = _mm_subs_epi16(ch_mag[i],mmtmpD0);


    llr128[0] = _mm_unpacklo_epi32(rxF[i],mmtmpD0);
    llr128[1] = _mm_unpackhi_epi32(rxF[i],mmtmpD0);
    llr128+=2;

    //    print_bytes("rxF[i]",&rxF[i]);
    //    print_bytes("rxF[i+1]",&rxF[i+1]);
  }

  if (len_mod4>0)
    *llr128p = (short *)(llr128-1);
  else
    *llr128p = (short *)(llr128);

  _mm_empty();
  _m_empty();

}

short *llr;

void dlsch_64qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
		     int **rxdataF_comp,
		     short *dlsch_llr,
		     int **dl_ch_mag,
		     int **dl_ch_magb,
		     unsigned char symbol,
		     u8 first_symbol_flag,
		     unsigned short nb_rb,
		     u16 pbch_pss_sss_adjust) {

  __m128i *rxF=(__m128i*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
  __m128i *ch_mag,*ch_magb;
  int j=0,i,len;
  unsigned char symbol_mod,len_mod4;


  if (first_symbol_flag==1)
    llr = dlsch_llr;

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;

  ch_mag =(__m128i*)&dl_ch_mag[0][(symbol*frame_parms->N_RB_DL*12)];
  ch_magb =(__m128i*)&dl_ch_magb[0][(symbol*frame_parms->N_RB_DL*12)];

//  printf("symbol %d (%d) pbch_pss_sss_adjust %d => len %d\n",symbol,(int)(llr-dlsch_llr),pbch_pss_sss_adjust,len);

  if ((symbol_mod==0) || (symbol_mod==(4-frame_parms->Ncp))) {
    if (frame_parms->mode1_flag==0)
      len = nb_rb*8 - (2*pbch_pss_sss_adjust/3);
    else
      len = nb_rb*10 - (5*pbch_pss_sss_adjust/6);
  }
  else {
    len = nb_rb*12 - pbch_pss_sss_adjust;
  }

  //  printf("16qam llr symbol %d : len %d\n",symbol,len);
  len_mod4 = len&3;
  len>>=2;  // length in quad words (4 REs)
  len+=(len_mod4>>1);
  //  printf("16qam llr symbol %d : len %d (%d)\n",symbol,len,len_mod4);

  for (i=0;i<len;i++) {


    mmtmpD1 = _mm_abs_epi16(rxF[i]);
    mmtmpD1  = _mm_subs_epi16(ch_mag[i],mmtmpD1);
    mmtmpD2 = _mm_abs_epi16(mmtmpD1);
    mmtmpD2 = _mm_subs_epi16(ch_magb[i],mmtmpD2);

    for (j=0;j<8;j+=2) {
      llr[0] = ((short *)&rxF[i])[j];
      llr[1] = ((short *)&rxF[i])[j+1];
      llr[2] = ((short *)&mmtmpD1)[j];
      llr[3] = ((short *)&mmtmpD1)[j+1];
      llr[4] = ((short *)&mmtmpD2)[j];
      llr[5] = ((short *)&mmtmpD2)[j+1];
      llr+=6;
    }

  }

  if (len_mod4>0)
    llr = llr - 12;
 
  _mm_empty();
  _m_empty();

}
/*
void dlsch_siso(LTE_DL_FRAME_PARMS *frame_parms,
		int **rxdataF_comp,
		int **rxdataF_comp_i,
		unsigned char l,
		unsigned short nb_rb) {

  unsigned char nsymb,Nsymb,slot_alloc=3,pilots,first_pilot;
  unsigned char symbol_offset,second_pilot,rb,re,jj,ii;

  nsymb = (frame_parms->Ncp==0) ? 7 : 6;
  Nsymb = nsymb<<1;
  symbol_offset = (slot_alloc==2) ? nsymb : 0; 
  nsymb = nsymb * ((slot_alloc>2)?2:1);
  second_pilot = (frame_parms->Ncp==0) ? 4 : 3;

  pilots=0;
  if ((l==0)||(l==(Nsymb>>1))){
    pilots=1;
    first_pilot=1;
  }
  
  if ((l==second_pilot)||(l==(second_pilot+(Nsymb>>1)))) {
    pilots=1;
    first_pilot=0;
  }

  jj=0;
  ii=0;
  if (pilots==1) {
    for (rb=0;rb<nb_rb;rb++) {

      for (re=0;re<12;re++) {

	if (is_not_pilot(pilots,first_pilot,re)) { 
	  rxdataF_comp[0][jj++] = rxdataF_comp[0][ii];
	  rxdataF_comp_i[0][jj++] = rxdataF_comp_i[0][ii];
	}
	ii++;
      }
    }
  }
}
*/

void dlsch_alamouti(LTE_DL_FRAME_PARMS *frame_parms,
		    int **rxdataF_comp,
		    int **dl_ch_mag,
		    int **dl_ch_magb,
		    unsigned char symbol,
		    unsigned short nb_rb) {


  short *rxF0,*rxF1;
  __m128i *ch_mag0,*ch_mag1,*ch_mag0b,*ch_mag1b;
  unsigned char rb,re;
  int jj=(symbol*frame_parms->N_RB_DL*12);
  u8 symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
  u8 pilots = ((symbol_mod==0)||(symbol_mod==(4-frame_parms->Ncp))) ? 1 : 0;


  //    printf("Doing alamouti!\n");
  rxF0     = (short*)&rxdataF_comp[0][jj];  //tx antenna 0  h0*y
  rxF1     = (short*)&rxdataF_comp[2][jj];  //tx antenna 1  h1*y
  ch_mag0 = (__m128i *)&dl_ch_mag[0][jj];
  ch_mag1 = (__m128i *)&dl_ch_mag[2][jj];
  ch_mag0b = (__m128i *)&dl_ch_magb[0][jj];
  ch_mag1b = (__m128i *)&dl_ch_magb[2][jj];
  
  for (rb=0;rb<nb_rb;rb++) {

    for (re=0;re<((pilots==0)?12:8);re+=2) {

      // Alamouti RX combining
      
      rxF0[0] = rxF0[0] + rxF1[2];
      rxF0[1] = rxF0[1] - rxF1[3];

      rxF0[2] = rxF0[2] - rxF1[0];
      rxF0[3] = rxF0[3] + rxF1[1];
 
      rxF0+=4;
      rxF1+=4;

    }

    // compute levels for 16QAM or 64 QAM llr unit
    ch_mag0[0] = _mm_adds_epi16(ch_mag0[0],ch_mag1[0]);
    ch_mag0[1] = _mm_adds_epi16(ch_mag0[1],ch_mag1[1]);

    ch_mag0b[0] = _mm_adds_epi16(ch_mag0b[0],ch_mag1b[0]);
    ch_mag0b[1] = _mm_adds_epi16(ch_mag0b[1],ch_mag1b[1]);

    if (pilots==0) {
      ch_mag0[2] = _mm_adds_epi16(ch_mag0[2],ch_mag1[2]);
      ch_mag0b[2] = _mm_adds_epi16(ch_mag0b[2],ch_mag1b[2]);

      ch_mag0+=3;
      ch_mag1+=3;
      ch_mag0b+=3;
      ch_mag1b+=3;
    }
    else {
      ch_mag0+=2;
      ch_mag1+=2;
      ch_mag0b+=2;
      ch_mag1b+=2;
    }
  }

  _mm_empty();
  _m_empty();
  
}

void dlsch_antcyc(LTE_DL_FRAME_PARMS *frame_parms,
		  int **rxdataF_comp,
		  int **dl_ch_mag,
		  int **dl_ch_magb,
		  unsigned char symbol,
		  unsigned short nb_rb) {

  unsigned char rb,re;
  int jj=1+(symbol*frame_parms->N_RB_DL*12);

  //  printf("Doing antcyc rx\n");
  for (rb=0;rb<nb_rb;rb++) {

    for (re=0;re<12;re+=2) {
      rxdataF_comp[0][jj] = rxdataF_comp[2][jj];  //copy odd carriers from tx antenna 1
      dl_ch_mag[0][jj]    = dl_ch_mag[2][jj];
      dl_ch_magb[0][jj]    = dl_ch_magb[2][jj];
      jj+=2;
    }
  }
  
}

void dlsch_detection_mrc(LTE_DL_FRAME_PARMS *frame_parms,
			 int **rxdataF_comp,
			 int **rxdataF_comp_i,
			 int **rho,
			 int **rho_i,
			 int **dl_ch_mag,
			 int **dl_ch_magb,
			 unsigned char symbol,
			 unsigned short nb_rb,
			 unsigned char dual_stream_UE) {

  unsigned char aatx;

  __m128i *rxdataF_comp128_0,*rxdataF_comp128_1,*rxdataF_comp128_i0,*rxdataF_comp128_i1,*dl_ch_mag128_0,*dl_ch_mag128_1,*dl_ch_mag128_0b,*dl_ch_mag128_1b,*rho128_0,*rho128_1,*rho128_i0,*rho128_i1;
  int i;

  if (frame_parms->nb_antennas_rx>1) {

    for (aatx=0;aatx<frame_parms->nb_antennas_tx;aatx++) {

      rxdataF_comp128_0   = (__m128i *)&rxdataF_comp[(aatx<<1)][symbol*frame_parms->N_RB_DL*12];  
      rxdataF_comp128_1   = (__m128i *)&rxdataF_comp[(aatx<<1)+1][symbol*frame_parms->N_RB_DL*12];  
      dl_ch_mag128_0      = (__m128i *)&dl_ch_mag[(aatx<<1)][symbol*frame_parms->N_RB_DL*12];  
      dl_ch_mag128_1      = (__m128i *)&dl_ch_mag[(aatx<<1)+1][symbol*frame_parms->N_RB_DL*12];  
      dl_ch_mag128_0b     = (__m128i *)&dl_ch_magb[(aatx<<1)][symbol*frame_parms->N_RB_DL*12];  
      dl_ch_mag128_1b     = (__m128i *)&dl_ch_magb[(aatx<<1)+1][symbol*frame_parms->N_RB_DL*12];  

      // MRC on each re of rb, both on MF output and magnitude (for 16QAM/64QAM llr computation)
      for (i=0;i<nb_rb*3;i++) {
	rxdataF_comp128_0[i] = _mm_adds_epi16(_mm_srai_epi16(rxdataF_comp128_0[i],1),_mm_srai_epi16(rxdataF_comp128_1[i],1));
	dl_ch_mag128_0[i]    = _mm_adds_epi16(_mm_srai_epi16(dl_ch_mag128_0[i],1),_mm_srai_epi16(dl_ch_mag128_1[i],1));
	dl_ch_mag128_0b[i]    = _mm_adds_epi16(_mm_srai_epi16(dl_ch_mag128_0b[i],1),_mm_srai_epi16(dl_ch_mag128_1b[i],1));
      }
    }

    if (rho) {
      rho128_0 = (__m128i *) &rho[0][symbol*frame_parms->N_RB_DL*12];
      rho128_1 = (__m128i *) &rho[1][symbol*frame_parms->N_RB_DL*12];
      for (i=0;i<nb_rb*3;i++) {
	rho128_0[i] = _mm_adds_epi16(_mm_srai_epi16(rho128_0[i],1),_mm_srai_epi16(rho128_1[i],1));
      }
    }

    if (dual_stream_UE == 1) {
      rho128_i0 = (__m128i *) &rho_i[0][symbol*frame_parms->N_RB_DL*12];
      rho128_i1 = (__m128i *) &rho_i[1][symbol*frame_parms->N_RB_DL*12];
      rxdataF_comp128_i0   = (__m128i *)&rxdataF_comp_i[0][symbol*frame_parms->N_RB_DL*12];  
      rxdataF_comp128_i1   = (__m128i *)&rxdataF_comp_i[1][symbol*frame_parms->N_RB_DL*12];  
      for (i=0;i<nb_rb*3;i++) {
	rxdataF_comp128_i0[i] = _mm_adds_epi16(_mm_srai_epi16(rxdataF_comp128_i0[i],1),_mm_srai_epi16(rxdataF_comp128_i1[i],1));
	rho128_i0[i]           = _mm_adds_epi16(_mm_srai_epi16(rho128_i0[i],1),_mm_srai_epi16(rho128_i1[i],1));
      }
    }
  }
  _mm_empty();
  _m_empty();

}


unsigned short dlsch_extract_rbs_single(int **rxdataF,
					int **dl_ch_estimates,
					int **rxdataF_ext,
					int **dl_ch_estimates_ext,
					unsigned short pmi,
					unsigned char *pmi_ext,
					unsigned int *rb_alloc,
					unsigned char symbol,
					unsigned char subframe,
					LTE_DL_FRAME_PARMS *frame_parms) {


  unsigned short rb,nb_rb=0;
  unsigned char rb_alloc_ind;
  unsigned char i,aarx,l,nsymb,skip_half=0,sss_symb,pss_symb;
  int *dl_ch0,*dl_ch0_ext,*rxF,*rxF_ext;



  unsigned char symbol_mod,pilots=0,j=0,poffset=0;

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
  pilots = ((symbol_mod==0)||(symbol_mod==(4-frame_parms->Ncp))) ? 1 : 0;
  l=symbol;
  nsymb = (frame_parms->Ncp==0) ? 14:12;

  if (frame_parms->frame_type == 1) {  // TDD
    sss_symb = nsymb-1;
  }
  else {
    sss_symb = (nsymb>>1)-2;
    pss_symb = (nsymb>>1)-1;
  }
  
  if (symbol_mod==(4-frame_parms->Ncp))
    poffset=3;

  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
    
    dl_ch0     = &dl_ch_estimates[aarx][5+(symbol_mod*(frame_parms->ofdm_symbol_size))];
    dl_ch0_ext = &dl_ch_estimates_ext[aarx][symbol_mod*(frame_parms->N_RB_DL*12)];

    rxF_ext   = &rxdataF_ext[aarx][symbol*(frame_parms->N_RB_DL*12)];
    
    rxF       = &rxdataF[aarx][(frame_parms->first_carrier_offset + (symbol*(frame_parms->ofdm_symbol_size)))*2];
    
    if ((frame_parms->N_RB_DL&1) == 0)  // even number of RBs
      for (rb=0;rb<frame_parms->N_RB_DL;rb++) {
	
	if (rb < 32)
	  rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
	else if (rb < 64)
	  rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
	else if (rb < 96)
	  rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
	else if (rb < 100)
	  rb_alloc_ind = (rb_alloc[0]>>(rb-96)) & 1;
	else
	  rb_alloc_ind = 0;
	
	// For second half of RBs skip DC carrier
	if (rb==(frame_parms->N_RB_DL>>1)) {
	  rxF       = &rxdataF[aarx][(1 + (symbol*(frame_parms->ofdm_symbol_size)))*2];
	  //dl_ch0++; 
	}
	
	if (rb_alloc_ind==1) {
	  *pmi_ext = (pmi>>((rb>>2)<<1))&3;
	  memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	  /*	  
	    printf("rb %d\n",rb);
	    for (i=0;i<12;i++)
	    printf("(%d %d)",((short *)dl_ch0)[i<<1],((short*)dl_ch0)[1+(i<<1)]);
	    printf("\n");
	  */
	  for (i=0;i<12;i++) {
	    rxF_ext[i]=rxF[i<<1];
	    //	      printf("%d : (%d,%d)\n",(rxF+(2*i)-&rxdataF[(aatx<<1)+aarx][( (symbol*(frame_parms->ofdm_symbol_size)))*2])/2,
	    //     ((short*)&rxF[i<<1])[0],((short*)&rxF[i<<1])[0]);
	  }
	  nb_rb++;
	  dl_ch0_ext+=12;
	  rxF_ext+=12;
	}
	dl_ch0+=12;
	rxF+=24;

      }
    else {  // Odd number of RBs
      for (rb=0;rb<frame_parms->N_RB_DL>>1;rb++) {
	//	printf("dlch_ext %d\n",dl_ch0_ext-&dl_ch_estimates_ext[aarx][0]);
	skip_half=0;
	if (rb < 32)
	  rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
	else if (rb < 64)
	  rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
	else if (rb < 96)
	  rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
	else if (rb < 100)
	  rb_alloc_ind = (rb_alloc[0]>>(rb-96)) & 1;
	else
	  rb_alloc_ind = 0;

	if (rb_alloc_ind==1)
	  nb_rb++;

	// PBCH
	if ((subframe==0) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l>=nsymb>>1) && (l<((nsymb>>1) + 4))) {
	  rb_alloc_ind = 0;
	}
	//PBCH subframe 0, symbols nsymb>>1 ... nsymb>>1 + 3
	if ((subframe==0) && (rb==((frame_parms->N_RB_DL>>1)-3)) && (l>=nsymb>>1) && (l<((nsymb>>1) + 4)))
	  skip_half=1;
	else if ((subframe==0) && (rb==((frame_parms->N_RB_DL>>1)+3)) && (l>=nsymb>>1) && (l<((nsymb>>1) + 4)))
	  skip_half=2;
	
	//SSS

	if (((subframe==0)||(subframe==5)) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==sss_symb) ) {
	  rb_alloc_ind = 0;
	}
	//SSS 
	if (((subframe==0)||(subframe==5)) && (rb==((frame_parms->N_RB_DL>>1)-3)) && (l==sss_symb))
	  skip_half=1;
	else if (((subframe==0)||(subframe==5)) && (rb==((frame_parms->N_RB_DL>>1)+3)) && (l==nsymb-1))
	  skip_half=2;

	//PSS
	if (frame_parms->frame_type == 0) {
	  if (((subframe==0)||(subframe==5)) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb) ) {
	    rb_alloc_ind = 0;
	  }
	  //PSS 
	  if (((subframe==0)||(subframe==5)) && (rb==((frame_parms->N_RB_DL>>1)-3)) && (l==sss_symb))
	    skip_half=1;
	  else if (((subframe==0)||(subframe==5)) && (rb==((frame_parms->N_RB_DL>>1)+3)) && (l==nsymb-1))
	    skip_half=2;
	}

	if (rb_alloc_ind==1) {
	  /*	  	  
		 printf("rb %d\n",rb);
		 for (i=0;i<12;i++)
		 printf("(%d %d)",((short *)dl_ch0)[i<<1],((short*)dl_ch0)[1+(i<<1)]);
		 printf("\n");
	  */
	  if (pilots==0) {
	    //	    printf("Extracting w/o pilots (symbol %d, rb %d, skip_half %d)\n",l,rb,skip_half);
	    if (skip_half==1) {
	      memcpy(dl_ch0_ext,dl_ch0,6*sizeof(int));
	      for (i=0;i<6;i++)
		rxF_ext[i]=rxF[i<<1];
	      dl_ch0_ext+=6;
	      rxF_ext+=6;
	    }
	    else if (skip_half==2) {
	      memcpy(dl_ch0_ext,dl_ch0+6,6*sizeof(int));
	      for (i=0;i<6;i++)
		rxF_ext[i]=rxF[(i+6)<<1];
	      dl_ch0_ext+=6;
	      rxF_ext+=6;
	    }
	    else {
	      memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	      for (i=0;i<12;i++)
		rxF_ext[i]=rxF[i<<1];
	      dl_ch0_ext+=12;
	      rxF_ext+=12;
	    }
	  }
	  else {
	    //	    printf("Extracting with pilots (symbol %d, rb %d, skip_half %d)\n",l,rb,skip_half);
	    j=0;
	    if (skip_half==1) {
	      for (i=0;i<6;i++) {
		if (i!=(frame_parms->nushift+poffset)) {
		  rxF_ext[j]=rxF[i<<1];
		  //		  		  printf("extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
		  dl_ch0_ext[j++]=dl_ch0[i];
		}
	      }
	      dl_ch0_ext+=5;
	      rxF_ext+=5;
	    }
	    else if (skip_half==2) {
	      for (i=0;i<6;i++) {
		if (i!=(frame_parms->nushift+poffset)) {
		  rxF_ext[j]=rxF[(i+6)<<1];
		  //		  		  printf("extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
		  dl_ch0_ext[j++]=dl_ch0[i+6];
		}
	      }
	      dl_ch0_ext+=5;
	      rxF_ext+=5;
	    }
	    else {
	      for (i=0;i<12;i++) {
		if ((i!=(frame_parms->nushift+poffset)) &&
		    (i!=(frame_parms->nushift+poffset+6))) {
		  rxF_ext[j]=rxF[i<<1];
		  //		  		  printf("extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
		  dl_ch0_ext[j++]=dl_ch0[i];
		  
		}
	      }
	      dl_ch0_ext+=10;
	      rxF_ext+=10;
	    }
	  }
	}	    
	dl_ch0+=12;
	rxF+=24;
      } // first half loop


      // Do middle RB (around DC)
      if (rb < 32)
	rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
      else if (rb < 64)
	rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
      else if (rb < 96)
	rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
      else if (rb < 100)
	rb_alloc_ind = (rb_alloc[0]>>(rb-96)) & 1;
      else
	rb_alloc_ind = 0;

      if (rb_alloc_ind==1)
	nb_rb++;

      // PBCH
      if ((subframe==0) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l>=nsymb>>1) && (l<((nsymb>>1) + 4))) {
	rb_alloc_ind = 0;
      }
      //SSS
      if (((subframe==0)||(subframe==5)) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==sss_symb) ) {
	rb_alloc_ind = 0;
      }
      if (frame_parms->frame_type == 0) {
      //PSS
	if (((subframe==0)||(subframe==5)) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb) ) {
	  rb_alloc_ind = 0;
	}
      }
      //	printf("dlch_ext %d\n",dl_ch0_ext-&dl_ch_estimates_ext[aarx][0]);      
      //      printf("DC rb %d (%p)\n",rb,rxF);
      if (rb_alloc_ind==1) {
	if (pilots==0) {
	  for (i=0;i<6;i++) {
	    dl_ch0_ext[i]=dl_ch0[i];
	    rxF_ext[i]=rxF[i<<1];
	  }
	  rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))*2];
	  for (;i<12;i++) {
	    dl_ch0_ext[i]=dl_ch0[i];
	    rxF_ext[i]=rxF[(1+i-6)<<1];
	  }
	  dl_ch0_ext+=12;
	  rxF_ext+=12;
	}
	else { // pilots==1
	  j=0;
	  for (i=0;i<6;i++) {
	    if (i!=(frame_parms->nushift+poffset)) {
	      dl_ch0_ext[j]=dl_ch0[i];
	      rxF_ext[j++]=rxF[i<<1];
	      //	           	      printf("**extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j-1],*(1+(short*)&rxF_ext[j-1]));
	    }
	  }
	  rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))*2];
	  for (;i<12;i++) {
	    if (i!=(frame_parms->nushift+6+poffset)) {
	      dl_ch0_ext[j]=dl_ch0[i];
	      rxF_ext[j++]=rxF[(1+i-6)<<1];
	      //	            	      printf("**extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j-1],*(1+(short*)&rxF_ext[j-1]));
	    }
	  }
	  dl_ch0_ext+=10;
	  rxF_ext+=10;
	} // symbol_mod==0
      } // rballoc==1
      else {
	rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))*2];
      }
      dl_ch0+=12;
      rxF+=14;
      rb++;
      
      for (;rb<frame_parms->N_RB_DL;rb++) {
	//	printf("dlch_ext %d\n",dl_ch0_ext-&dl_ch_estimates_ext[aarx][0]);	
	//	printf("rb %d (%p)\n",rb,rxF);
	skip_half=0;
	if (rb < 32)
	  rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
	else if (rb < 64)
	  rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
	else if (rb < 96)
	  rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
	else if (rb < 100)
	  rb_alloc_ind = (rb_alloc[0]>>(rb-96)) & 1;
	else
	  rb_alloc_ind = 0;

	if (rb_alloc_ind==1)
	  nb_rb++;

	// PBCH
	if ((subframe==0) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l>=nsymb>>1) && (l<((nsymb>>1) + 4))) {
	  rb_alloc_ind = 0;
	}
	//PBCH subframe 0, symbols nsymb>>1 ... nsymb>>1 + 3
	if ((subframe==0) && (rb==((frame_parms->N_RB_DL>>1)-3)) && (l>=nsymb>>1) && (l<((nsymb>>1) + 4)))
	  skip_half=1;
	else if ((subframe==0) && (rb==((frame_parms->N_RB_DL>>1)+3)) && (l>=nsymb>>1) && (l<((nsymb>>1) + 4)))
	  skip_half=2;
	//SSS
	if (((subframe==0)||(subframe==5)) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==sss_symb) ) {
	  rb_alloc_ind = 0;
	}
	//SSS 
	if (((subframe==0)||(subframe==5)) && (rb==((frame_parms->N_RB_DL>>1)-3)) && (l==sss_symb))
	  skip_half=1;
	else if (((subframe==0)||(subframe==5)) && (rb==((frame_parms->N_RB_DL>>1)+3)) && (l==sss_symb))
	  skip_half=2;
      
	if (frame_parms->frame_type == 0) {
	  //PSS
	  if (((subframe==0)||(subframe==5)) && (rb>((frame_parms->N_RB_DL>>1)-3)) && (rb<((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb) ) {
	    rb_alloc_ind = 0;
	  }
	  //PSS 
	  if (((subframe==0)||(subframe==5)) && (rb==((frame_parms->N_RB_DL>>1)-3)) && (l==pss_symb))
	    skip_half=1;
	  else if (((subframe==0)||(subframe==5)) && (rb==((frame_parms->N_RB_DL>>1)+3)) && (l==pss_symb))
	    skip_half=2;
	}
	
	if (rb_alloc_ind==1) {
	  /*
  	    printf("rb %d\n",rb);
	    for (i=0;i<12;i++)
	    printf("(%d %d)",((short *)dl_ch0)[i<<1],((short*)dl_ch0)[1+(i<<1)]);
	    printf("\n");
	  */
	  if (pilots==0) {
	    //	    printf("Extracting w/o pilots (symbol %d, rb %d, skip_half %d)\n",l,rb,skip_half);
	    if (skip_half==1) {
	      memcpy(dl_ch0_ext,dl_ch0,6*sizeof(int));
	      for (i=0;i<6;i++)
		rxF_ext[i]=rxF[i<<1];
	      dl_ch0_ext+=6;
	      rxF_ext+=6;

	    }
	    else if (skip_half==2) {
	      memcpy(dl_ch0_ext,dl_ch0+6,6*sizeof(int));
	      for (i=0;i<6;i++)
		rxF_ext[i]=rxF[(i+6)<<1];
	      dl_ch0_ext+=6;
	      rxF_ext+=6;

	    }
	    else {
	      memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	      for (i=0;i<12;i++)
		rxF_ext[i]=rxF[i<<1];
	      dl_ch0_ext+=12;
	      rxF_ext+=12;
	    }
	  }
	  else {
	    //	    printf("Extracting with pilots (symbol %d, rb %d, skip_half %d)\n",l,rb,skip_half);
	    j=0;
	    if (skip_half==1) {
	      for (i=0;i<6;i++) {
		if (i!=(frame_parms->nushift+poffset)) {
		  rxF_ext[j]=rxF[i<<1];
		  //		  printf("extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
		  dl_ch0_ext[j++]=dl_ch0[i];
		}
	      }
	      dl_ch0_ext+=5;
	      rxF_ext+=5;
	    }
	    else if (skip_half==2) {
	      for (i=0;i<6;i++) {
		if (i!=(frame_parms->nushift+poffset)) {
		  rxF_ext[j]=rxF[(i+6)<<1];
		  //		  printf("extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
		  dl_ch0_ext[j++]=dl_ch0[i+6];
		}
	      }
	      dl_ch0_ext+=5;
	      rxF_ext+=5;
	    }
	    else {
	      for (i=0;i<12;i++) {
		if ((i!=(frame_parms->nushift+poffset)) &&
		    (i!=(frame_parms->nushift+poffset+6))) {
		  rxF_ext[j]=rxF[i<<1];
		  //		  printf("extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
		  dl_ch0_ext[j++]=dl_ch0[i];
		}
	      }
	      dl_ch0_ext+=10;
	      rxF_ext+=10;
	    }
	  } // pilots=0
	}
	dl_ch0+=12;
	rxF+=24;

      }
    }
  }

  _mm_empty();
  _m_empty();

  return(nb_rb/frame_parms->nb_antennas_rx);
}

unsigned short dlsch_extract_rbs_dual(int **rxdataF,
				      int **dl_ch_estimates,
				      int **rxdataF_ext,
				      int **dl_ch_estimates_ext,
				      unsigned short pmi,
				      unsigned char *pmi_ext,
				      unsigned int *rb_alloc,
				      unsigned char symbol,
				      LTE_DL_FRAME_PARMS *frame_parms) {


  unsigned short rb,nb_rb=0;
  unsigned char rb_alloc_ind;
  unsigned char i,aarx;
  int *dl_ch0,*dl_ch0_ext,*dl_ch1,*dl_ch1_ext,*rxF,*rxF_ext;
  unsigned char symbol_mod,pilots=0,j=0;
  unsigned char *pmi_loc;

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
  //  printf("extract_rbs: symbol_mod %d\n",symbol_mod);

  if ((symbol_mod == 0) || (symbol_mod == (4-frame_parms->Ncp)))
    pilots=1;

  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
    
    dl_ch0     = &dl_ch_estimates[aarx][5+(symbol_mod*(frame_parms->ofdm_symbol_size))];
    dl_ch0_ext = &dl_ch_estimates_ext[aarx][symbol_mod*(frame_parms->N_RB_DL*12)];
    dl_ch1     = &dl_ch_estimates[2+aarx][5+(symbol_mod*(frame_parms->ofdm_symbol_size))];
    dl_ch1_ext = &dl_ch_estimates_ext[2+aarx][symbol_mod*(frame_parms->N_RB_DL*12)];
    pmi_loc = pmi_ext;

    rxF_ext   = &rxdataF_ext[aarx][symbol*(frame_parms->N_RB_DL*12)];
    
    rxF       = &rxdataF[aarx][(frame_parms->first_carrier_offset + (symbol*(frame_parms->ofdm_symbol_size)))*2];

    //debug_msg("Doing extraction with pmi %x\n",pmi2hex_2Ar1(pmi));

    if ((frame_parms->N_RB_DL&1) == 0)  // even number of RBs
      for (rb=0;rb<frame_parms->N_RB_DL;rb++) {
	
	if (rb < 32)
	  rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
	else if (rb < 64)
	  rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
	else if (rb < 96)
	  rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
	else if (rb < 100)
	  rb_alloc_ind = (rb_alloc[0]>>(rb-96)) & 1;
	else
	  rb_alloc_ind = 0;
	
	// For second half of RBs skip DC carrier
	if (rb==(frame_parms->N_RB_DL>>1)) {
	  rxF       = &rxdataF[aarx][(1 + (symbol*(frame_parms->ofdm_symbol_size)))*2];
	  //dl_ch0++;
	  //dl_ch1++;
	}
	
	if (rb_alloc_ind==1) {


	  *pmi_loc = (pmi>>((rb>>2)<<1))&3;
	  //	  printf("rb %d: sb %d : pmi %d\n",rb,rb>>2,*pmi_loc);

	  pmi_loc++;


	  if (pilots == 0) {

	    memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	    memcpy(dl_ch1_ext,dl_ch1,12*sizeof(int));
	    /*
	      printf("rb %d\n",rb);
	      for (i=0;i<12;i++)
	      printf("(%d %d)",((short *)dl_ch)[i<<1],((short*)dl_ch)[1+(i<<1)]);
	      printf("\n");*/
	    
	    for (i=0;i<12;i++) {
	      rxF_ext[i]=rxF[i<<1];
	      //	      printf("%d : (%d,%d)\n",(rxF+(2*i)-&rxdataF[(aatx<<1)+aarx][( (symbol*(frame_parms->ofdm_symbol_size)))*2])/2,
	      //     ((short*)&rxF[i<<1])[0],((short*)&rxF[i<<1])[0]);
	    }
	    nb_rb++;
	    dl_ch0_ext+=12;
	    dl_ch1_ext+=12;
	    rxF_ext+=12;
	  }
	  else {
	    j=0;
	    for (i=0;i<12;i++) {
	      if ((i!=frame_parms->nushift) &&
		  (i!=frame_parms->nushift+3) &&
		  (i!=frame_parms->nushift+6) &&
		  (i!=frame_parms->nushift+9)) {
		rxF_ext[j]=rxF[i<<1];
		//	      printf("extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
		dl_ch0_ext[j]=dl_ch0[i];
		dl_ch1_ext[j++]=dl_ch1[i];
	      }
	    }
	    nb_rb++;
	    dl_ch0_ext+=8;
	    dl_ch1_ext+=8;
	    rxF_ext+=8;
	  } // pilots==1
	}
	dl_ch0+=12;
	dl_ch1+=12;
	rxF+=24;

      }
    else {  // Odd number of RBs
      for (rb=0;rb<frame_parms->N_RB_DL>>1;rb++) {
	if (rb < 32)
	  rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
	else if (rb < 64)
	  rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
	else if (rb < 96)
	  rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
	else if (rb < 100)
	  rb_alloc_ind = (rb_alloc[0]>>(rb-96)) & 1;
	else
	  rb_alloc_ind = 0;
	
	if (rb_alloc_ind==1) {
	  *pmi_loc = (pmi>>((rb>>2)<<1))&3;
	  //	  printf("symbol_mod %d (pilots %d) rb %d, sb %d, pmi %d (pmi_loc %p,rxF %p, ch00 %p, ch01 %p, rxF_ext %p dl_ch0_ext %p dl_ch1_ext %p)\n",symbol_mod,pilots,rb,rb>>2,*pmi_loc,pmi_loc,rxF,dl_ch0, dl_ch1, rxF_ext,dl_ch0_ext,dl_ch1_ext);

	  pmi_loc++;
	  if (pilots==0) {
	    memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	    memcpy(dl_ch1_ext,dl_ch1,12*sizeof(int));
	    for (i=0;i<12;i++)
	      rxF_ext[i]=rxF[i<<1];
	    nb_rb++;
	    dl_ch0_ext+=12;
	    dl_ch1_ext+=12;
	    rxF_ext+=12;
	  }
	  else {
	    j=0;
	    for (i=0;i<12;i++) {
	      if ((i!=frame_parms->nushift) &&
		  (i!=frame_parms->nushift+3) &&
		  (i!=frame_parms->nushift+6) &&
		  (i!=frame_parms->nushift+9)) {
		rxF_ext[j]=rxF[i<<1];
		//			      printf("extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));

		dl_ch0_ext[j]  =dl_ch0[i];
		dl_ch1_ext[j++]=dl_ch1[i];

		//			      printf("extract rb %d, re %d => ch0 (%d,%d) ch1 (%d,%d)\n",rb,i,
		//				     *(short *)&dl_ch0_ext[j-1],*(1+(short*)&dl_ch0_ext[j-1]),
		//				     *(short *)&dl_ch1_ext[j-1],*(1+(short*)&dl_ch1_ext[j-1]));
	      }
	    }
	    nb_rb++;
	    dl_ch0_ext+=8;
	    dl_ch1_ext+=8;
	    rxF_ext+=8;		
	  }
	}
	dl_ch0+=12;
	dl_ch1+=12;
	rxF+=24;

      }
      // Do middle RB (around DC)
      if (rb < 32)
	rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
      else if (rb < 64)
	rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
      else if (rb < 96)
	rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
      else if (rb < 100)
	rb_alloc_ind = (rb_alloc[0]>>(rb-96)) & 1;
      else
	rb_alloc_ind = 0;
	
      if (rb_alloc_ind==1) {

	*pmi_loc = (pmi>>((rb>>2)<<1))&3;
	//	printf("rb %d, sb %d, pmi %d (pmi_loc %p,rxF %p, ch00 %p, ch01 %p, rxF_ext %p dl_ch0_ext %p dl_ch1_ext %p)\n",rb,rb>>2,*pmi_loc,pmi_loc,rxF,dl_ch0, dl_ch1, rxF_ext,dl_ch0_ext,dl_ch1_ext);
	pmi_loc++;

	if (pilots==0) {
	  for (i=0;i<6;i++) {
	    dl_ch0_ext[i]=dl_ch0[i];
	    dl_ch1_ext[i]=dl_ch1[i];
	    rxF_ext[i]=rxF[i<<1];
	  }
	  rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))*2];
	  for (;i<12;i++) {
	    dl_ch0_ext[i]=dl_ch0[i];
	    dl_ch1_ext[i]=dl_ch1[i];
	    rxF_ext[i]=rxF[(1+i-6)<<1];
	  }
	  nb_rb++;
	  dl_ch0_ext+=12;
	  dl_ch1_ext+=12;
	  rxF_ext+=12;	
	}
	else {  // pilots==1
	  j=0;
	  for (i=0;i<6;i++) {
	    if ((i!=frame_parms->nushift) &&
		(i!=frame_parms->nushift+3)){
	      dl_ch0_ext[j]=dl_ch0[i];
	      dl_ch1_ext[j]=dl_ch1[i];
	      rxF_ext[j++]=rxF[i<<1];
	      //	      printf("**extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j-1],*(1+(short*)&rxF_ext[j-1]));
	      //	      printf("extract rb %d, re %d => ch0 (%d,%d) ch1 (%d,%d)\n",rb,i,
	      //		     *(short *)&dl_ch0_ext[j-1],*(1+(short*)&dl_ch0_ext[j-1]),
	      //		     *(short *)&dl_ch1_ext[j-1],*(1+(short*)&dl_ch1_ext[j-1]));
	    }
	  }
	  rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))*2];
	  for (;i<12;i++) {
	    if ((i!=frame_parms->nushift+6) &&
		(i!=frame_parms->nushift+9)){
	      dl_ch0_ext[j]=dl_ch0[i];
	      dl_ch1_ext[j]=dl_ch1[i];
	      rxF_ext[j++]=rxF[(1+i-6)<<1];
	      //	      printf("**extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j-1],*(1+(short*)&rxF_ext[j-1]));
	    }
	  }
	  nb_rb++;
	  dl_ch0_ext+=8;
	  dl_ch1_ext+=8;
	  rxF_ext+=8; 
	}


#ifdef KHZ66_NULL
	dl_ch0_ext[8] = 0;
	dl_ch1_ext[8] = 0;
	dl_ch0_ext[9] = 0;
	dl_ch1_ext[9] = 0;
	dl_ch0_ext[10] = 0;
	dl_ch1_ext[10] = 0;
	dl_ch0_ext[11] = 0;
	dl_ch1_ext[11] = 0;
#endif
	//	nb_rb++;
	//	dl_ch0_ext+=12;
	//	dl_ch1_ext+=12;
	//	rxF_ext+=12;
      }
      else {
	rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))*2];
      }
      dl_ch0+=12;
      dl_ch1+=12;
      rxF+=14;
      rb++;

      for (;rb<frame_parms->N_RB_DL;rb++) {

	if (rb < 32)
	  rb_alloc_ind = (rb_alloc[0]>>rb) & 1;
	else if (rb < 64)
	  rb_alloc_ind = (rb_alloc[1]>>(rb-32)) & 1;
	else if (rb < 96)
	  rb_alloc_ind = (rb_alloc[2]>>(rb-64)) & 1;
	else if (rb < 100)
	  rb_alloc_ind = (rb_alloc[0]>>(rb-96)) & 1;
	else
	  rb_alloc_ind = 0;
	  
	if (rb_alloc_ind==1) {

	  *pmi_loc = (pmi>>((rb>>2)<<1))&3;
	  //	  printf("rb %d, sb %d, pmi %d (pmi_loc %p,rxF %p, ch00 %p, ch01 %p, rxF_ext %p dl_ch0_ext %p dl_ch1_ext %p)\n",rb,rb>>2,*pmi_loc,pmi_loc,rxF,dl_ch0, dl_ch1, rxF_ext,dl_ch0_ext,dl_ch1_ext);

	  pmi_loc++;

	  if (pilots==0) {
	    memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	    memcpy(dl_ch1_ext,dl_ch1,12*sizeof(int));
	    for (i=0;i<12;i++)
	      rxF_ext[i]=rxF[i<<1];
	    nb_rb++;
	    dl_ch0_ext+=12;
	    dl_ch1_ext+=12;
	    rxF_ext+=12;
	  }
	  else {
	    j=0;
	    for (i=0;i<12;i++) {
	      if ((i!=frame_parms->nushift) &&
		  (i!=frame_parms->nushift+3) &&
		  (i!=frame_parms->nushift+6) &&
		  (i!=frame_parms->nushift+9)) {
		rxF_ext[j]=rxF[i<<1];
		//		printf("extract rb %d, re %d => (%d,%d)\n",rb,i,*(short *)&rxF_ext[j],*(1+(short*)&rxF_ext[j]));
		dl_ch0_ext[j]=dl_ch0[i];
		dl_ch1_ext[j++]=dl_ch1[i];
		//			      printf("extract rb %d, re %d => ch0 (%d,%d) ch1 (%d,%d)\n",rb,i,
		//				     *(short *)&dl_ch0_ext[j-1],*(1+(short*)&dl_ch0_ext[j-1]),
		//				     *(short *)&dl_ch1_ext[j-1],*(1+(short*)&dl_ch1_ext[j-1]));
	      }
	    }
	    nb_rb++;
	    dl_ch0_ext+=8;
	    dl_ch1_ext+=8;
	    rxF_ext+=8;	
	  }
	}
	dl_ch0+=12;
	dl_ch1+=12;
	rxF+=24;
      }
    }
  }

  _mm_empty();
  _m_empty();

  return(nb_rb/frame_parms->nb_antennas_rx);
}


void dlsch_dual_stream_correlation(LTE_DL_FRAME_PARMS *frame_parms,
				   unsigned char symbol,
				   unsigned short nb_rb,
				   int **dl_ch_estimates_ext,
				   int **dl_ch_estimates_ext_i,
				   int **dl_ch_rho_ext,
				   unsigned char output_shift) {

  unsigned short rb;
  __m128i *dl_ch128,*dl_ch128i,*dl_ch_rho128;
  unsigned char aarx,symbol_mod;

  //  printf("dlsch_dual_stream_correlation: symbol %d\n",symbol);

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;


  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {

    dl_ch128          = (__m128i *)&dl_ch_estimates_ext[aarx][symbol_mod*frame_parms->N_RB_DL*12];
    dl_ch128i         = (__m128i *)&dl_ch_estimates_ext_i[aarx][symbol_mod*frame_parms->N_RB_DL*12];
    dl_ch_rho128      = (__m128i *)&dl_ch_rho_ext[aarx][symbol*frame_parms->N_RB_DL*12];


    for (rb=0;rb<nb_rb;rb++) {
      // multiply by conjugated channel
      mmtmpD0 = _mm_madd_epi16(dl_ch128[0],dl_ch128i[0]);
      //	print_ints("re",&mmtmpD0);
      
      // mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpD1 = _mm_shufflelo_epi16(dl_ch128[0],_MM_SHUFFLE(2,3,0,1));
      mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
      mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)&conjugate[0]);
      //	print_ints("im",&mmtmpD1);
      mmtmpD1 = _mm_madd_epi16(mmtmpD1,dl_ch128i[0]);
      // mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
      //	print_ints("re(shift)",&mmtmpD0);
      mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
      //	print_ints("im(shift)",&mmtmpD1);
      mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
      mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
      //       	print_ints("c0",&mmtmpD2);
      //	print_ints("c1",&mmtmpD3);
      dl_ch_rho128[0] = _mm_packs_epi32(mmtmpD2,mmtmpD3);
      
      //print_shorts("rx:",dl_ch128_2);
      //print_shorts("ch:",dl_ch128);
      //print_shorts("pack:",rho128);
      
      // multiply by conjugated channel
      mmtmpD0 = _mm_madd_epi16(dl_ch128[1],dl_ch128i[1]);
      // mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpD1 = _mm_shufflelo_epi16(dl_ch128[1],_MM_SHUFFLE(2,3,0,1));
      mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
      mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)conjugate);
      mmtmpD1 = _mm_madd_epi16(mmtmpD1,dl_ch128i[1]);
      // mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
      mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
      mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
      mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
      
      
      dl_ch_rho128[1] =_mm_packs_epi32(mmtmpD2,mmtmpD3);
      //print_shorts("rx:",dl_ch128_2+1);
      //print_shorts("ch:",dl_ch128+1);
      //print_shorts("pack:",rho128+1);	
      // multiply by conjugated channel
      mmtmpD0 = _mm_madd_epi16(dl_ch128[2],dl_ch128i[2]);
      // mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpD1 = _mm_shufflelo_epi16(dl_ch128[2],_MM_SHUFFLE(2,3,0,1));
      mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
      mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)conjugate);
      mmtmpD1 = _mm_madd_epi16(mmtmpD1,dl_ch128i[2]);
      // mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
      mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
      mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
      mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
      
      dl_ch_rho128[2] = _mm_packs_epi32(mmtmpD2,mmtmpD3);
      //print_shorts("rx:",dl_ch128_2+2);
      //print_shorts("ch:",dl_ch128+2);
      //print_shorts("pack:",rho128+2);
      
      dl_ch128+=3;
      dl_ch128i+=3;
      dl_ch_rho128+=3;
      
      
      
      
    }	
    
  }
  
  _mm_empty();
  _m_empty();
  
  
}

__m128i QAM_amp128,QAM_amp128b;

void dlsch_channel_compensation(int **rxdataF_ext,
				int **dl_ch_estimates_ext,
				int **dl_ch_mag,
				int **dl_ch_magb,
				int **rxdataF_comp,
				int **rho,
				LTE_DL_FRAME_PARMS *frame_parms,
				unsigned char symbol,
				u8 first_symbol_flag,
				unsigned char mod_order,
				unsigned short nb_rb,
				unsigned char output_shift,
				PHY_MEASUREMENTS *phy_measurements) {

  unsigned short rb;
  __m128i *dl_ch128,*dl_ch128_2,*dl_ch_mag128,*dl_ch_mag128b,*rxdataF128,*rxdataF_comp128,*rho128;
  unsigned char aatx,aarx,symbol_mod,pilots=0;


  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;

#ifndef __SSE3__
  zero = _mm_xor_si128(zero,zero);
#endif

  if ((symbol_mod == 0) || (symbol_mod == (4-frame_parms->Ncp))) {

    if (frame_parms->mode1_flag==1) // 10 out of 12 so don't reduce size    
      nb_rb=1+(5*nb_rb/6);
    else  
      pilots=1;
    
  }

  for (aatx=0;aatx<frame_parms->nb_antennas_tx;aatx++) {
    if (mod_order == 4)
      QAM_amp128 = _mm_set1_epi16(QAM16_n1);
    else if (mod_order == 6) {
      QAM_amp128  = _mm_set1_epi16(QAM64_n1);
      QAM_amp128b = _mm_set1_epi16(QAM64_n2);
    }
    //    printf("comp: rxdataF_comp %p, symbol %d\n",rxdataF_comp[0],symbol);

    for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {

      dl_ch128          = (__m128i *)&dl_ch_estimates_ext[(aatx<<1)+aarx][symbol_mod*frame_parms->N_RB_DL*12];
      dl_ch_mag128      = (__m128i *)&dl_ch_mag[(aatx<<1)+aarx][symbol*frame_parms->N_RB_DL*12];
      dl_ch_mag128b     = (__m128i *)&dl_ch_magb[(aatx<<1)+aarx][symbol*frame_parms->N_RB_DL*12];
      rxdataF128        = (__m128i *)&rxdataF_ext[aarx][symbol*frame_parms->N_RB_DL*12];
      rxdataF_comp128   = (__m128i *)&rxdataF_comp[(aatx<<1)+aarx][symbol*frame_parms->N_RB_DL*12];


      for (rb=0;rb<nb_rb;rb++) {
	if (mod_order>2) {  
	  // get channel amplitude if not QPSK

	  mmtmpD0 = _mm_madd_epi16(dl_ch128[0],dl_ch128[0]);

	  mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
	  
	  mmtmpD1 = _mm_madd_epi16(dl_ch128[1],dl_ch128[1]);
	  mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
	  mmtmpD0 = _mm_packs_epi32(mmtmpD0,mmtmpD1);
	   
	  dl_ch_mag128[0] = _mm_unpacklo_epi16(mmtmpD0,mmtmpD0);
	  dl_ch_mag128b[0] = dl_ch_mag128[0];
	  dl_ch_mag128[0] = _mm_mulhi_epi16(dl_ch_mag128[0],QAM_amp128);
	  dl_ch_mag128[0] = _mm_slli_epi16(dl_ch_mag128[0],1);

	  dl_ch_mag128[1] = _mm_unpackhi_epi16(mmtmpD0,mmtmpD0);
	  dl_ch_mag128b[1] = dl_ch_mag128[1];
	  dl_ch_mag128[1] = _mm_mulhi_epi16(dl_ch_mag128[1],QAM_amp128);
	  dl_ch_mag128[1] = _mm_slli_epi16(dl_ch_mag128[1],1);

	  if (pilots==0) {
	    mmtmpD0 = _mm_madd_epi16(dl_ch128[2],dl_ch128[2]);
	    mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
	    mmtmpD1 = _mm_packs_epi32(mmtmpD0,mmtmpD0);
	    
	    dl_ch_mag128[2] = _mm_unpacklo_epi16(mmtmpD1,mmtmpD1);
	    dl_ch_mag128b[2] = dl_ch_mag128[2];
	    
	    dl_ch_mag128[2] = _mm_mulhi_epi16(dl_ch_mag128[2],QAM_amp128);
	    dl_ch_mag128[2] = _mm_slli_epi16(dl_ch_mag128[2],1);	  
	  }

	  dl_ch_mag128b[0] = _mm_mulhi_epi16(dl_ch_mag128b[0],QAM_amp128b);
	  dl_ch_mag128b[0] = _mm_slli_epi16(dl_ch_mag128b[0],1);
	  

	  dl_ch_mag128b[1] = _mm_mulhi_epi16(dl_ch_mag128b[1],QAM_amp128b);
	  dl_ch_mag128b[1] = _mm_slli_epi16(dl_ch_mag128b[1],1);

	  if (pilots==0) {
	    dl_ch_mag128b[2] = _mm_mulhi_epi16(dl_ch_mag128b[2],QAM_amp128b);
	    dl_ch_mag128b[2] = _mm_slli_epi16(dl_ch_mag128b[2],1);	  
	  }
	}
	
	// multiply by conjugated channel
	mmtmpD0 = _mm_madd_epi16(dl_ch128[0],rxdataF128[0]);
	//	print_ints("re",&mmtmpD0);
	
	// mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
	mmtmpD1 = _mm_shufflelo_epi16(dl_ch128[0],_MM_SHUFFLE(2,3,0,1));
	mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
	mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)&conjugate[0]);
	//	print_ints("im",&mmtmpD1);
	mmtmpD1 = _mm_madd_epi16(mmtmpD1,rxdataF128[0]);
	// mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
	mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
	//	print_ints("re(shift)",&mmtmpD0);
	mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
	//	print_ints("im(shift)",&mmtmpD1);
	mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
	mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
	//       	print_ints("c0",&mmtmpD2);
	//	print_ints("c1",&mmtmpD3);
	rxdataF_comp128[0] = _mm_packs_epi32(mmtmpD2,mmtmpD3);
	//	print_shorts("rx:",rxdataF128);
	//	print_shorts("ch:",dl_ch128);
	//	print_shorts("pack:",rxdataF_comp128);

	// multiply by conjugated channel
	mmtmpD0 = _mm_madd_epi16(dl_ch128[1],rxdataF128[1]);
	// mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
	mmtmpD1 = _mm_shufflelo_epi16(dl_ch128[1],_MM_SHUFFLE(2,3,0,1));
	mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
	mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)conjugate);
	mmtmpD1 = _mm_madd_epi16(mmtmpD1,rxdataF128[1]);
	// mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
	mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
	mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
	mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
	mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
	
	rxdataF_comp128[1] = _mm_packs_epi32(mmtmpD2,mmtmpD3);
	//	print_shorts("rx:",rxdataF128+1);
	//	print_shorts("ch:",dl_ch128+1);
	//	print_shorts("pack:",rxdataF_comp128+1);	

	if (pilots==0) {
	  // multiply by conjugated channel
	  mmtmpD0 = _mm_madd_epi16(dl_ch128[2],rxdataF128[2]);
	  // mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
	  mmtmpD1 = _mm_shufflelo_epi16(dl_ch128[2],_MM_SHUFFLE(2,3,0,1));
	  mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
	  mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)conjugate);
	  mmtmpD1 = _mm_madd_epi16(mmtmpD1,rxdataF128[2]);
	  // mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
	  mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
	  mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
	  mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
	  mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
	  
	  rxdataF_comp128[2] = _mm_packs_epi32(mmtmpD2,mmtmpD3);
	  //	print_shorts("rx:",rxdataF128+2);
	  //	print_shorts("ch:",dl_ch128+2);
	  //      	print_shorts("pack:",rxdataF_comp128+2);
	
	  dl_ch128+=3;
	  dl_ch_mag128+=3;
	  dl_ch_mag128b+=3;
	  rxdataF128+=3;
	  rxdataF_comp128+=3;
	}
	else { // we have a smaller PDSCH in symbols with pilots so skip last group of 4 REs and increment less
	  dl_ch128+=2;
	  dl_ch_mag128+=2;
	  dl_ch_mag128b+=2;
	  rxdataF128+=2;
	  rxdataF_comp128+=2;
	}
	
      }
    }
  }

  if (rho) {


    for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
      rho128        = (__m128i *)&rho[aarx][symbol*frame_parms->N_RB_DL*12];
      dl_ch128      = (__m128i *)&dl_ch_estimates_ext[aarx][symbol_mod*frame_parms->N_RB_DL*12];
      dl_ch128_2    = (__m128i *)&dl_ch_estimates_ext[2+aarx][symbol_mod*frame_parms->N_RB_DL*12];

      for (rb=0;rb<nb_rb;rb++) {
	// multiply by conjugated channel
	mmtmpD0 = _mm_madd_epi16(dl_ch128[0],dl_ch128_2[0]);
	//	print_ints("re",&mmtmpD0);
	
	// mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
	mmtmpD1 = _mm_shufflelo_epi16(dl_ch128[0],_MM_SHUFFLE(2,3,0,1));
	mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
	mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)&conjugate[0]);
	//	print_ints("im",&mmtmpD1);
	mmtmpD1 = _mm_madd_epi16(mmtmpD1,dl_ch128_2[0]);
	// mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
	mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
	//	print_ints("re(shift)",&mmtmpD0);
	mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
	//	print_ints("im(shift)",&mmtmpD1);
	mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
	mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
	//       	print_ints("c0",&mmtmpD2);
	//	print_ints("c1",&mmtmpD3);
	rho128[0] = _mm_packs_epi32(mmtmpD2,mmtmpD3);

	//print_shorts("rx:",dl_ch128_2);
	//print_shorts("ch:",dl_ch128);
	//print_shorts("pack:",rho128);
	
	// multiply by conjugated channel
	mmtmpD0 = _mm_madd_epi16(dl_ch128[1],dl_ch128_2[1]);
	// mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
	mmtmpD1 = _mm_shufflelo_epi16(dl_ch128[1],_MM_SHUFFLE(2,3,0,1));
	mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
	mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)conjugate);
	mmtmpD1 = _mm_madd_epi16(mmtmpD1,dl_ch128_2[1]);
	// mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
	mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
	mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
	mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
	mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);

	
	rho128[1] =_mm_packs_epi32(mmtmpD2,mmtmpD3);
	//print_shorts("rx:",dl_ch128_2+1);
	//print_shorts("ch:",dl_ch128+1);
	//print_shorts("pack:",rho128+1);	
	// multiply by conjugated channel
	mmtmpD0 = _mm_madd_epi16(dl_ch128[2],dl_ch128_2[2]);
	// mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
	mmtmpD1 = _mm_shufflelo_epi16(dl_ch128[2],_MM_SHUFFLE(2,3,0,1));
	mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
	mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)conjugate);
	mmtmpD1 = _mm_madd_epi16(mmtmpD1,dl_ch128_2[2]);
	// mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
	mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
	mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
	mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
	mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
	
	rho128[2] = _mm_packs_epi32(mmtmpD2,mmtmpD3);
	//print_shorts("rx:",dl_ch128_2+2);
	//print_shorts("ch:",dl_ch128+2);
	//print_shorts("pack:",rho128+2);
	
	dl_ch128+=3;
	dl_ch128_2+=3;
	rho128+=3;
	
      }	
      
      if (first_symbol_flag==1) {
	phy_measurements->rx_correlation[0][aarx] = signal_energy(&rho[aarx][symbol*frame_parms->N_RB_DL*12],rb*12);
      } 
         
    }

  }

  _mm_empty();
  _m_empty();

}     

//static __m128i  one_over_sqrt2;
 	 
void prec2A_128(unsigned char pmi,__m128i *ch0,__m128i *ch1) {
  
  switch (pmi) {
 
  case 0 :   // +1 +1
    //    print_shorts("phase 0 :ch0",ch0);
    //    print_shorts("phase 0 :ch1",ch1);
    ch0[0] = _mm_adds_epi16(ch0[0],ch1[0]);   

    //    ch0[0] = _mm_mulhi_epi16(ch0[0],one_over_sqrt2);
    //    ch0[0] = _mm_slli_epi16(ch0[0],1);
    break;
  case 1 :   // +1 -1
    //    print_shorts("phase 1 :ch0",ch0);
    //    print_shorts("phase 1 :ch1",ch1);
    ch0[0] = _mm_subs_epi16(ch0[0],ch1[0]);
    //    print_shorts("phase 1 :ch0-ch1",ch0);
    //    ch0[0] = _mm_mulhi_epi16(ch0[0],one_over_sqrt2);
    //    ch0[0] = _mm_slli_epi16(ch0[0],1);   
    break;
  case 2 :   // +1 +j
    ch1[0] = _mm_sign_epi16(ch1[0],*(__m128i*)&conjugate[0]);
    ch1[0] = _mm_shufflelo_epi16(ch1[0],_MM_SHUFFLE(2,3,0,1));
    ch1[0] = _mm_shufflehi_epi16(ch1[0],_MM_SHUFFLE(2,3,0,1));
    ch0[0] = _mm_subs_epi16(ch0[0],ch1[0]);
    //    ch0[0] = _mm_mulhi_epi16(ch0[0],one_over_sqrt2);
    //    ch0[0] = _mm_slli_epi16(ch0[0],1);
    break;   // +1 -j
  case 3 :
    ch1[0] = _mm_sign_epi16(ch1[0],*(__m128i*)&conjugate[0]);
    ch1[0] = _mm_shufflelo_epi16(ch1[0],_MM_SHUFFLE(2,3,0,1));
    ch1[0] = _mm_shufflehi_epi16(ch1[0],_MM_SHUFFLE(2,3,0,1));
    ch0[0] = _mm_adds_epi16(ch0[0],ch1[0]);
    //    ch0[0] = _mm_mulhi_epi16(ch0[0],one_over_sqrt2);
    //    ch0[0] = _mm_slli_epi16(ch0[0],1);
    break;
  }
  _mm_empty();
  _m_empty();
}

void dlsch_channel_compensation_prec(int **rxdataF_ext,
				     int **dl_ch_estimates_ext,
				     int **dl_ch_mag,
				     int **dl_ch_magb,
				     int **rxdataF_comp,
				     unsigned char *pmi_ext,
				     LTE_DL_FRAME_PARMS *frame_parms,
				     PHY_MEASUREMENTS *phy_measurements,
				     int eNB_id,
				     unsigned char symbol,
				     u8 first_symbol_flag,
				     unsigned char mod_order,
				     unsigned short nb_rb,
				     unsigned char output_shift) {
  
  unsigned short rb,Nre;
  __m128i *dl_ch128_0,*dl_ch128_1,*dl_ch_mag128,*dl_ch_mag128b,*rxdataF128,*rxdataF_comp128;
  unsigned char aarx=0,symbol_mod,pilots=0;
  int precoded_signal_strength=0,rx_power_correction;

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;

  if ((symbol_mod == 0) || (symbol_mod == (4-frame_parms->Ncp)))
    pilots=1;

  if ( (frame_parms->ofdm_symbol_size == 128) ||
       (frame_parms->ofdm_symbol_size == 512) )
    rx_power_correction = 2;
  else
    rx_power_correction = 1;

#ifndef __SSE3__
  zero = _mm_xor_si128(zero,zero);
#endif

  //printf("comp prec: symbol %d, pilots %d\n",symbol, pilots);

  if (mod_order == 4)
    QAM_amp128 = _mm_set1_epi16(QAM16_n1);
  else if (mod_order == 6) {
    QAM_amp128  = _mm_set1_epi16(QAM64_n1);
    QAM_amp128b = _mm_set1_epi16(QAM64_n2);
  }

  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {

    dl_ch128_0          = (__m128i *)&dl_ch_estimates_ext[aarx][symbol_mod*frame_parms->N_RB_DL*12];
    dl_ch128_1          = (__m128i *)&dl_ch_estimates_ext[2+aarx][symbol_mod*frame_parms->N_RB_DL*12];

    
    dl_ch_mag128      = (__m128i *)&dl_ch_mag[aarx][symbol*frame_parms->N_RB_DL*12];
    dl_ch_mag128b     = (__m128i *)&dl_ch_magb[aarx][symbol*frame_parms->N_RB_DL*12];
    rxdataF128        = (__m128i *)&rxdataF_ext[aarx][symbol*frame_parms->N_RB_DL*12];
    rxdataF_comp128   = (__m128i *)&rxdataF_comp[aarx][symbol*frame_parms->N_RB_DL*12];


    for (rb=0;rb<nb_rb;rb++) {
      // combine TX channels using precoder from pmi
#ifdef DEBUG_DLSCH_DEMOD
      printf("mode 6 prec: rb %d, pmi->%d\n",rb,pmi_ext[rb]);
#endif
	
      //print_shorts("ch0(0):",&dl_ch128_0[0]);
      //print_shorts("ch1(0):",&dl_ch128_1[0]);
      prec2A_128(pmi_ext[rb],&dl_ch128_0[0],&dl_ch128_1[0]);
      //print_shorts("prec(ch0,ch1):",&dl_ch128_0[0]);
      
      //print_shorts("ch0(1):",&dl_ch128_0[1]);
      //print_shorts("ch1(1):",&dl_ch128_1[1]);
      prec2A_128(pmi_ext[rb],&dl_ch128_0[1],&dl_ch128_1[1]);
      //print_shorts("prec(ch0,ch1):",&dl_ch128_0[1]);
      
      if (pilots==0) {
	//print_shorts("ch0(2):",&dl_ch128_0[2]);
	//print_shorts("ch1(2):",&dl_ch128_1[2]); 
	prec2A_128(pmi_ext[rb],&dl_ch128_0[2],&dl_ch128_1[2]); 
	//print_shorts("prec(ch0,ch1):",&dl_ch128_0[2]);      
      }

      //#endif      

      if (mod_order>2) {  
	// get channel amplitude if not QPSK
	
	mmtmpD0 = _mm_madd_epi16(dl_ch128_0[0],dl_ch128_0[0]);
	
	mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
	
	mmtmpD1 = _mm_madd_epi16(dl_ch128_0[1],dl_ch128_0[1]);
	mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
	mmtmpD0 = _mm_packs_epi32(mmtmpD0,mmtmpD1);
	
	dl_ch_mag128[0] = _mm_unpacklo_epi16(mmtmpD0,mmtmpD0);
	dl_ch_mag128b[0] = dl_ch_mag128[0];
	dl_ch_mag128[0] = _mm_mulhi_epi16(dl_ch_mag128[0],QAM_amp128);
	dl_ch_mag128[0] = _mm_slli_epi16(dl_ch_mag128[0],1);

	//print_shorts("dl_ch_mag128[0]=",&dl_ch_mag128[0]);
	
	dl_ch_mag128[1] = _mm_unpackhi_epi16(mmtmpD0,mmtmpD0);
	dl_ch_mag128b[1] = dl_ch_mag128[1];
	dl_ch_mag128[1] = _mm_mulhi_epi16(dl_ch_mag128[1],QAM_amp128);
	dl_ch_mag128[1] = _mm_slli_epi16(dl_ch_mag128[1],1);

	if (pilots==0) {
	  mmtmpD0 = _mm_madd_epi16(dl_ch128_0[2],dl_ch128_0[2]);
	  mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
	  mmtmpD1 = _mm_packs_epi32(mmtmpD0,mmtmpD0);
	  
	  dl_ch_mag128[2] = _mm_unpacklo_epi16(mmtmpD1,mmtmpD1);
	  dl_ch_mag128b[2] = dl_ch_mag128[2];
	  
	  dl_ch_mag128[2] = _mm_mulhi_epi16(dl_ch_mag128[2],QAM_amp128);
	  dl_ch_mag128[2] = _mm_slli_epi16(dl_ch_mag128[2],1);	  
	}
	
	dl_ch_mag128b[0] = _mm_mulhi_epi16(dl_ch_mag128b[0],QAM_amp128b);
	dl_ch_mag128b[0] = _mm_slli_epi16(dl_ch_mag128b[0],1);
	
	//print_shorts("dl_ch_mag128b[0]=",&dl_ch_mag128b[0]);
	
	dl_ch_mag128b[1] = _mm_mulhi_epi16(dl_ch_mag128b[1],QAM_amp128b);
	dl_ch_mag128b[1] = _mm_slli_epi16(dl_ch_mag128b[1],1);
	
	if (pilots==0) {
	  dl_ch_mag128b[2] = _mm_mulhi_epi16(dl_ch_mag128b[2],QAM_amp128b);
	  dl_ch_mag128b[2] = _mm_slli_epi16(dl_ch_mag128b[2],1);	  
	}
      }
      
      // multiply by conjugated channel
      mmtmpD0 = _mm_madd_epi16(dl_ch128_0[0],rxdataF128[0]);
      //	print_ints("re",&mmtmpD0);
      
      // mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpD1 = _mm_shufflelo_epi16(dl_ch128_0[0],_MM_SHUFFLE(2,3,0,1));
      mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
      mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)&conjugate[0]);
      //	print_ints("im",&mmtmpD1);
      mmtmpD1 = _mm_madd_epi16(mmtmpD1,rxdataF128[0]);
      // mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
      //	print_ints("re(shift)",&mmtmpD0);
      mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
      //	print_ints("im(shift)",&mmtmpD1);
      mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
      mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
      //       	print_ints("c0",&mmtmpD2);
      //	print_ints("c1",&mmtmpD3);
      rxdataF_comp128[0] = _mm_packs_epi32(mmtmpD2,mmtmpD3);
      //	print_shorts("rx:",rxdataF128);
      //	print_shorts("ch:",dl_ch128);
      //	print_shorts("pack:",rxdataF_comp128);
      
      // multiply by conjugated channel
      mmtmpD0 = _mm_madd_epi16(dl_ch128_0[1],rxdataF128[1]);
      // mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpD1 = _mm_shufflelo_epi16(dl_ch128_0[1],_MM_SHUFFLE(2,3,0,1));
      mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
      mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)conjugate);
      mmtmpD1 = _mm_madd_epi16(mmtmpD1,rxdataF128[1]);
      // mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
      mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
      mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
      mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
      
      rxdataF_comp128[1] = _mm_packs_epi32(mmtmpD2,mmtmpD3);
      //	print_shorts("rx:",rxdataF128+1);
      //	print_shorts("ch:",dl_ch128+1);
      //	print_shorts("pack:",rxdataF_comp128+1);	

      if (pilots==0) {
	// multiply by conjugated channel
	mmtmpD0 = _mm_madd_epi16(dl_ch128_0[2],rxdataF128[2]);
	// mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
	mmtmpD1 = _mm_shufflelo_epi16(dl_ch128_0[2],_MM_SHUFFLE(2,3,0,1));
	mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
	mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)conjugate);
	mmtmpD1 = _mm_madd_epi16(mmtmpD1,rxdataF128[2]);
	// mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
	mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
	mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
	mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
	mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
	
	rxdataF_comp128[2] = _mm_packs_epi32(mmtmpD2,mmtmpD3);
      //	print_shorts("rx:",rxdataF128+2);
      //	print_shorts("ch:",dl_ch128+2);
      //      	print_shorts("pack:",rxdataF_comp128+2);
      
	dl_ch128_0+=3;
	dl_ch128_1+=3;
	dl_ch_mag128+=3;
	dl_ch_mag128b+=3;
	rxdataF128+=3;
	rxdataF_comp128+=3;
      }
      else {
	dl_ch128_0+=2;
	dl_ch128_1+=2;
	dl_ch_mag128+=2;
	dl_ch_mag128b+=2;
	rxdataF128+=2;
	rxdataF_comp128+=2;
      }
    }

    Nre = (pilots==0) ? 12 : 8;

    precoded_signal_strength += ((signal_energy_nodc(&dl_ch_estimates_ext[aarx][symbol_mod*frame_parms->N_RB_DL*Nre],
						     (nb_rb*Nre))*rx_power_correction) - (phy_measurements->n0_power[aarx]));
  } // rx_antennas

  phy_measurements->precoded_cqi_dB[eNB_id][0] = dB_fixed2(precoded_signal_strength,phy_measurements->n0_power_tot);
	
  //printf("eNB_id %d, symbol %d: precoded CQI %d dB\n",eNB_id,symbol,
  //	 phy_measurements->precoded_cqi_dB[eNB_id][0]);

  _mm_empty();
  _m_empty();
  
}     
	 
    
__m128i avg128D;

//compute average channel_level on each (TX,RX) antenna pair
void dlsch_channel_level(int **dl_ch_estimates_ext,
			 LTE_DL_FRAME_PARMS *frame_parms,
			 int *avg,
			 u8 symbol_mod,
			 unsigned short nb_rb){

  short rb;
  unsigned char aatx,aarx,nre=12;
  __m128i *dl_ch128;
  
  for (aatx=0;aatx<frame_parms->nb_antennas_tx;aatx++)
    for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
      //clear average level
      avg128D = _mm_xor_si128(avg128D,avg128D);
      // 5 is always a symbol with no pilots for both normal and extended prefix

      dl_ch128=(__m128i *)&dl_ch_estimates_ext[(aatx<<1)+aarx][symbol_mod*frame_parms->N_RB_DL*12];

      for (rb=0;rb<nb_rb;rb++) {
	//	printf("rb %d : ",rb);
	//	print_shorts("ch",&dl_ch128[0]);
	avg128D = _mm_add_epi32(avg128D,_mm_madd_epi16(dl_ch128[0],dl_ch128[0]));
	avg128D = _mm_add_epi32(avg128D,_mm_madd_epi16(dl_ch128[1],dl_ch128[1]));
	if (((symbol_mod == 0) || (symbol_mod == (frame_parms->Ncp-1)))&&(frame_parms->mode1_flag==0)) {
	  avg128D = _mm_add_epi32(avg128D,_mm_madd_epi16(dl_ch128[2],dl_ch128[2]));

	  dl_ch128+=2;	
	}
	else 
	  dl_ch128+=3;	
	/*
	  if (rb==0) {
	  print_shorts("dl_ch128",&dl_ch128[0]);
	  print_shorts("dl_ch128",&dl_ch128[1]);
	  print_shorts("dl_ch128",&dl_ch128[2]);
	  }
	*/
      }

      if (((symbol_mod == 0) || (symbol_mod == (frame_parms->Ncp-1)))&&(frame_parms->mode1_flag==0))
	nre=8;
      else if (((symbol_mod == 0) || (symbol_mod == (frame_parms->Ncp-1)))&&(frame_parms->mode1_flag==1))
	nre=10;
      else
	nre=12;

      avg[(aatx<<1)+aarx] = (((int*)&avg128D)[0] + 
			     ((int*)&avg128D)[1] + 
			     ((int*)&avg128D)[2] + 
			     ((int*)&avg128D)[3])/(nb_rb*nre);

      //            printf("Channel level : %d\n",avg[(aatx<<1)+aarx]);
    }
  _mm_empty();
  _m_empty();

}

int avg[4];

int rx_dlsch(LTE_UE_COMMON *lte_ue_common_vars,
	     LTE_UE_DLSCH **lte_ue_dlsch_vars,
	     LTE_DL_FRAME_PARMS *frame_parms,
	     unsigned char eNB_id,
	     unsigned char eNB_id_i, //if this == NUMBER_OF_eNB_MAX, we assume MU interference
	     LTE_UE_DLSCH_t **dlsch_ue,
	     u8 subframe,
	     unsigned char symbol,
	     unsigned char first_symbol_flag,
	     unsigned char dual_stream_flag,
	     PHY_MEASUREMENTS *phy_measurements,
	     unsigned char i_mod) {
  
  unsigned short nb_rb;

  unsigned char aatx,aarx,symbol_mod,pilots=0;
  int avgs, rb;

  unsigned char harq_pid0;

  if (eNB_id > 2) {
    msg("dlsch_demodulation.c: Illegal eNB_id %d\n",eNB_id);
   return(-1);
  }

  if (!lte_ue_common_vars) {
    msg("dlsch_demodulation.c: Null lte_ue_common_vars\n");
    return(-1);
  }

  if (!dlsch_ue[0]) {
    msg("dlsch_demodulation.c: Null dlsch_ue pointer\n");
    return(-1);
  }

  if (!lte_ue_dlsch_vars) {
    msg("dlsch_demodulation.c: Null lte_ue_dlsch_vars pointer\n");
    return(-1);
  }

  if (!frame_parms) {
    msg("dlsch_demodulation.c: Null lte_frame_parms\n");
    return(-1);
  }

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
  if ((symbol_mod == 0) || (symbol_mod == (4-frame_parms->Ncp)))
    pilots=1;
  else 
    pilots=0;

  harq_pid0 = dlsch_ue[0]->current_harq_pid;

  if (frame_parms->nb_antennas_tx>1) {
#ifdef DEBUG_DLSCH_MOD     
    msg("dlsch: using pmi %x (%p), rb_alloc %x\n",pmi2hex_2Ar1(dlsch_ue[0]->pmi_alloc),dlsch_ue[0],dlsch_ue[0]->rb_alloc[0]);
#endif
    nb_rb = dlsch_extract_rbs_dual(lte_ue_common_vars->rxdataF,
				   lte_ue_common_vars->dl_ch_estimates[eNB_id],
				   lte_ue_dlsch_vars[eNB_id]->rxdataF_ext,
				   lte_ue_dlsch_vars[eNB_id]->dl_ch_estimates_ext,
				   dlsch_ue[0]->pmi_alloc,
				   lte_ue_dlsch_vars[eNB_id]->pmi_ext,
				   dlsch_ue[0]->rb_alloc,
				   symbol,
				   frame_parms);

    if (dual_stream_flag==1) {
      if (eNB_id_i!=NUMBER_OF_eNB_MAX)
	nb_rb = dlsch_extract_rbs_dual(lte_ue_common_vars->rxdataF,
				       lte_ue_common_vars->dl_ch_estimates[eNB_id_i],
				       lte_ue_dlsch_vars[eNB_id_i]->rxdataF_ext,
				       lte_ue_dlsch_vars[eNB_id_i]->dl_ch_estimates_ext,
				       dlsch_ue[0]->pmi_alloc,
				       lte_ue_dlsch_vars[eNB_id_i]->pmi_ext,
				       dlsch_ue[0]->rb_alloc,
				       symbol,
				       frame_parms);
      else
	nb_rb = dlsch_extract_rbs_dual(lte_ue_common_vars->rxdataF,
				       lte_ue_common_vars->dl_ch_estimates[eNB_id],
				       lte_ue_dlsch_vars[eNB_id_i]->rxdataF_ext,
				       lte_ue_dlsch_vars[eNB_id_i]->dl_ch_estimates_ext,
				       dlsch_ue[0]->pmi_alloc,
				       lte_ue_dlsch_vars[eNB_id_i]->pmi_ext,
				       dlsch_ue[0]->rb_alloc,
				       symbol,
				       frame_parms);
    }
  } // if n_tx>1
  else { 

    nb_rb = dlsch_extract_rbs_single(lte_ue_common_vars->rxdataF,
				     lte_ue_common_vars->dl_ch_estimates[eNB_id],
				     lte_ue_dlsch_vars[eNB_id]->rxdataF_ext,
				     lte_ue_dlsch_vars[eNB_id]->dl_ch_estimates_ext,
				     dlsch_ue[0]->pmi_alloc,
				     lte_ue_dlsch_vars[eNB_id]->pmi_ext,
				     dlsch_ue[0]->rb_alloc,
				     symbol,
				     subframe,
				     frame_parms);
    
    if (dual_stream_flag==1) {
      if (eNB_id_i!=NUMBER_OF_eNB_MAX)
	nb_rb = dlsch_extract_rbs_single(lte_ue_common_vars->rxdataF,
					 lte_ue_common_vars->dl_ch_estimates[eNB_id_i],
					 lte_ue_dlsch_vars[eNB_id_i]->rxdataF_ext,
					 lte_ue_dlsch_vars[eNB_id_i]->dl_ch_estimates_ext,    
					 dlsch_ue[0]->pmi_alloc,
					 lte_ue_dlsch_vars[eNB_id_i]->pmi_ext,
					 dlsch_ue[0]->rb_alloc,
					 symbol,
					 subframe,
					 frame_parms);
      else 
	nb_rb = dlsch_extract_rbs_single(lte_ue_common_vars->rxdataF,
					 lte_ue_common_vars->dl_ch_estimates[eNB_id],
					 lte_ue_dlsch_vars[eNB_id_i]->rxdataF_ext,
					 lte_ue_dlsch_vars[eNB_id_i]->dl_ch_estimates_ext,    
					 dlsch_ue[0]->pmi_alloc,
					 lte_ue_dlsch_vars[eNB_id_i]->pmi_ext,
					 dlsch_ue[0]->rb_alloc,
					 symbol,
					 subframe,
					 frame_parms);
    }
  } //else

  //  printf("nb_rb = %d, eNB_id %d\n",nb_rb,eNB_id);
  if (nb_rb==0) {
    msg("dlsch_modulation.c: nb_rb=0\n");
    return(-1);
  }


  if (first_symbol_flag==1) {
    dlsch_channel_level(lte_ue_dlsch_vars[eNB_id]->dl_ch_estimates_ext,
			frame_parms,
			avg,
			symbol_mod,
			nb_rb);
#ifdef DEBUG_PHY
    msg("[DLSCH] avg[0] %d\n",avg[0]);
#endif
      
    avgs = 0;
    for (aatx=0;aatx<frame_parms->nb_antennas_tx;aatx++)
      for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++)
	avgs = cmax(avgs,avg[(aarx<<1)+aatx]);
    lte_ue_dlsch_vars[eNB_id]->log2_maxh = (log2_approx(avgs)/2);//-1;
#ifdef DEBUG_PHY
    msg("[DLSCH] log2_maxh = %d (%d,%d)\n",lte_ue_dlsch_vars[eNB_id]->log2_maxh,avg[0],avgs);
    msg("[DLSCH] mimo_mode = %d\n", dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode);
#endif
  }
  aatx = frame_parms->nb_antennas_tx;
  aarx = frame_parms->nb_antennas_rx;

  if (dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode<UNIFORM_PRECODING11) {// no precoding

    dlsch_channel_compensation(lte_ue_dlsch_vars[eNB_id]->rxdataF_ext,
			       lte_ue_dlsch_vars[eNB_id]->dl_ch_estimates_ext,
			       lte_ue_dlsch_vars[eNB_id]->dl_ch_mag,
			       lte_ue_dlsch_vars[eNB_id]->dl_ch_magb,
			       lte_ue_dlsch_vars[eNB_id]->rxdataF_comp,
			       (aatx>1) ? lte_ue_dlsch_vars[eNB_id]->rho : NULL,
			       frame_parms,
			       symbol,
			       first_symbol_flag,
			       get_Qm(dlsch_ue[0]->harq_processes[harq_pid0]->mcs),
			       nb_rb,
			       lte_ue_dlsch_vars[eNB_id]->log2_maxh,
			       phy_measurements); // log2_maxh+I0_shift
#ifdef DEBUG_PHY
    write_output("rxF_comp_d.m","rxF_c_d",&lte_ue_dlsch_vars[eNB_id]->rxdataF_comp[0][symbol*frame_parms->N_RB_DL*12],frame_parms->N_RB_DL*12,1,1);
#endif
    
    if ((dual_stream_flag==1) && (eNB_id_i!=NUMBER_OF_eNB_MAX)) {
      
      // get MF output for interfering stream
      dlsch_channel_compensation(lte_ue_dlsch_vars[eNB_id_i]->rxdataF_ext,
				 lte_ue_dlsch_vars[eNB_id_i]->dl_ch_estimates_ext,
				 lte_ue_dlsch_vars[eNB_id_i]->dl_ch_mag,
				 lte_ue_dlsch_vars[eNB_id_i]->dl_ch_magb,
				 lte_ue_dlsch_vars[eNB_id_i]->rxdataF_comp,
				 (aatx>1) ? lte_ue_dlsch_vars[eNB_id_i]->rho : NULL,
				 frame_parms,
				 symbol,
				 first_symbol_flag,
				 i_mod,
				 nb_rb,
				 lte_ue_dlsch_vars[eNB_id_i]->log2_maxh,
				 phy_measurements); // log2_maxh+I0_shift
#ifdef DEBUG_PHY
      if (symbol == 5) {
	write_output("rxF_comp_d.m","rxF_c_d",&lte_ue_dlsch_vars[eNB_id]->rxdataF_comp[0][symbol*frame_parms->N_RB_DL*12],frame_parms->N_RB_DL*12,1,1);
	write_output("rxF_comp_i.m","rxF_c_i",&lte_ue_dlsch_vars[eNB_id_i]->rxdataF_comp[0][symbol*frame_parms->N_RB_DL*12],frame_parms->N_RB_DL*12,1,1);     
      }
#endif 

      // compute correlation between signal and interference channels
      dlsch_dual_stream_correlation(frame_parms,
				    symbol,
				    nb_rb,
				    lte_ue_dlsch_vars[eNB_id]->dl_ch_estimates_ext,
				    lte_ue_dlsch_vars[eNB_id_i]->dl_ch_estimates_ext,
				    lte_ue_dlsch_vars[eNB_id]->dl_ch_rho_ext,
				    lte_ue_dlsch_vars[eNB_id]->log2_maxh);
    
    }
  }
  else if (dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode<DUALSTREAM_UNIFORM_PRECODING1) {// single-layer precoding
    //    printf("Channel compensation for precoding\n");
    dlsch_channel_compensation_prec(lte_ue_dlsch_vars[eNB_id]->rxdataF_ext,
				    lte_ue_dlsch_vars[eNB_id]->dl_ch_estimates_ext,
				    lte_ue_dlsch_vars[eNB_id]->dl_ch_mag,
				    lte_ue_dlsch_vars[eNB_id]->dl_ch_magb,
				    lte_ue_dlsch_vars[eNB_id]->rxdataF_comp,
				    lte_ue_dlsch_vars[eNB_id]->pmi_ext,
				    frame_parms,
				    phy_measurements,
				    eNB_id,
				    symbol,
				    first_symbol_flag,
				    get_Qm(dlsch_ue[0]->harq_processes[harq_pid0]->mcs),
				    nb_rb,
				    lte_ue_dlsch_vars[eNB_id]->log2_maxh);
    //    printf("Channel compensation for precoding done\n");

    
    if ((dual_stream_flag==1) && (eNB_id_i==NUMBER_OF_eNB_MAX)) {

      // if interference source is MU interference, assume opposite precoder was used at eNB

      // calcualte opposite PMI
      for (rb=0;rb<nb_rb;rb++) {
	switch(lte_ue_dlsch_vars[eNB_id]->pmi_ext[rb]) {
	case 0:
	  lte_ue_dlsch_vars[eNB_id_i]->pmi_ext[rb]=1;
	  break;
	case 1:
	  lte_ue_dlsch_vars[eNB_id_i]->pmi_ext[rb]=0;
	  break;
	case 2:
	  lte_ue_dlsch_vars[eNB_id_i]->pmi_ext[rb]=3;
	  break;
	case 3:
	  lte_ue_dlsch_vars[eNB_id_i]->pmi_ext[rb]=2;
	  break;
	}
	
      }

      // apply opposite precoder to calculate interfering stream
      dlsch_channel_compensation_prec(lte_ue_dlsch_vars[eNB_id_i]->rxdataF_ext,
				      lte_ue_dlsch_vars[eNB_id_i]->dl_ch_estimates_ext,
				      lte_ue_dlsch_vars[eNB_id_i]->dl_ch_mag,
				      lte_ue_dlsch_vars[eNB_id_i]->dl_ch_magb,
				      lte_ue_dlsch_vars[eNB_id_i]->rxdataF_comp,
				      lte_ue_dlsch_vars[eNB_id_i]->pmi_ext,
				      frame_parms,
				      phy_measurements,
				      eNB_id_i,
				      symbol,
				      first_symbol_flag,
				      i_mod, 
				      nb_rb,
				      lte_ue_dlsch_vars[eNB_id]->log2_maxh);
  

      // compute correlation between precoded channel and channel precoded with opposite PMI
      dlsch_dual_stream_correlation(frame_parms,
				    symbol,
				    nb_rb,
				    lte_ue_dlsch_vars[eNB_id]->dl_ch_estimates_ext,
				    lte_ue_dlsch_vars[eNB_id_i]->dl_ch_estimates_ext,
				    lte_ue_dlsch_vars[eNB_id]->dl_ch_rho_ext,
				    lte_ue_dlsch_vars[eNB_id]->log2_maxh);
    
    }
  }

  //  printf("MRC\n");
  if (frame_parms->nb_antennas_rx > 1)
    dlsch_detection_mrc(frame_parms,
			lte_ue_dlsch_vars[eNB_id]->rxdataF_comp,
			lte_ue_dlsch_vars[eNB_id_i]->rxdataF_comp,
			lte_ue_dlsch_vars[eNB_id]->rho,
			lte_ue_dlsch_vars[eNB_id]->dl_ch_rho_ext,
			lte_ue_dlsch_vars[eNB_id]->dl_ch_mag,
			lte_ue_dlsch_vars[eNB_id]->dl_ch_magb,
			symbol,
			nb_rb,
			dual_stream_flag); 
#ifdef DEBUG_PHY
  write_output("rxF_comp_d.m","rxF_c_d",&lte_ue_dlsch_vars[eNB_id]->rxdataF_comp[0][symbol*frame_parms->N_RB_DL*12],frame_parms->N_RB_DL*12,1,1);
  write_output("rxF_comp_i.m","rxF_c_i",&lte_ue_dlsch_vars[eNB_id_i]->rxdataF_comp[0][symbol*frame_parms->N_RB_DL*12],frame_parms->N_RB_DL*12,1,1);    
#endif  

  //  printf("Combining");
  // Single-layer transmission formats
  if (dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode<DUALSTREAM_UNIFORM_PRECODING1) {
    if ((dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode == SISO) ||
	((dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode >= UNIFORM_PRECODING11) &&
	 (dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode <= PUSCH_PRECODING0)))
      {}//dlsch_siso(frame_parms,lte_ue_dlsch_vars[eNB_id]->rxdataF_comp,lte_ue_dlsch_vars[eNB_id_i]->rxdataF_comp,symbol,nb_rb);
    else if (dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode == ALAMOUTI)
      dlsch_alamouti(frame_parms,lte_ue_dlsch_vars[eNB_id]->rxdataF_comp,lte_ue_dlsch_vars[eNB_id]->dl_ch_mag,lte_ue_dlsch_vars[eNB_id]->dl_ch_magb,symbol,nb_rb);
    else if (dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode == ANTCYCLING)
      dlsch_antcyc(frame_parms,lte_ue_dlsch_vars[eNB_id]->rxdataF_comp,lte_ue_dlsch_vars[eNB_id]->dl_ch_mag,lte_ue_dlsch_vars[eNB_id]->dl_ch_magb,symbol,nb_rb);
    else {
      msg("dlsch_rx: Unknown MIMO mode\n");
      return (-1);
    }

    //    printf("LLR");

    switch (get_Qm(dlsch_ue[0]->harq_processes[harq_pid0]->mcs)) {
    case 2 : 
      if (dual_stream_flag == 0)
	dlsch_qpsk_llr(frame_parms,
		       lte_ue_dlsch_vars[eNB_id]->rxdataF_comp,
		       lte_ue_dlsch_vars[eNB_id]->llr[0],
		       symbol,first_symbol_flag,nb_rb,
		       adjust_G2(frame_parms,dlsch_ue[0]->rb_alloc,2,subframe,symbol),
		       lte_ue_dlsch_vars[eNB_id]->llr128);
      else if (i_mod == 2) 
	dlsch_qpsk_qpsk_llr(frame_parms,
			    lte_ue_dlsch_vars[eNB_id]->rxdataF_comp,
			    lte_ue_dlsch_vars[eNB_id_i]->rxdataF_comp,
			    lte_ue_dlsch_vars[eNB_id]->dl_ch_rho_ext,
			    lte_ue_dlsch_vars[eNB_id]->llr[0],
			    symbol,first_symbol_flag,nb_rb,
			    adjust_G2(frame_parms,dlsch_ue[0]->rb_alloc,2,subframe,symbol),
			    lte_ue_dlsch_vars[eNB_id]->llr128);
      else {
	msg("rx_dlsch.c : IC receiver only implemented for 4QAM-4QAM\n");
	return(-1);
      }
      
      break;
    case 4 :
      if (dual_stream_flag == 0)
	dlsch_16qam_llr(frame_parms,
			lte_ue_dlsch_vars[eNB_id]->rxdataF_comp,
			lte_ue_dlsch_vars[eNB_id]->llr[0],
			lte_ue_dlsch_vars[eNB_id]->dl_ch_mag,
			symbol,first_symbol_flag,nb_rb,
			adjust_G2(frame_parms,dlsch_ue[0]->rb_alloc,4,subframe,symbol),
			lte_ue_dlsch_vars[eNB_id]->llr128);
      else {
	msg("rx_dlsch.c : IC receiver for 16QAM not yet implemented\n");
	return(-1);
      }
      break;
    case 6 :
      if (dual_stream_flag == 0)
	dlsch_64qam_llr(frame_parms,
			lte_ue_dlsch_vars[eNB_id]->rxdataF_comp,
			lte_ue_dlsch_vars[eNB_id]->llr[0],
			lte_ue_dlsch_vars[eNB_id]->dl_ch_mag,
			lte_ue_dlsch_vars[eNB_id]->dl_ch_magb,
			symbol,first_symbol_flag,nb_rb,
			adjust_G2(frame_parms,dlsch_ue[0]->rb_alloc,6,subframe,symbol));
      else {
	msg("rx_dlsch.c : IC receiver for 64QAM not yet implemented\n");
	return(-1);
      }
      break;
    default:
      msg("rx_dlsch.c : Unknown mod_order!!!!\n");
      return(-1);
      break;
    }
  } // single-layer transmission
  else  {
      msg("rx_dlsch.c : Dualstream not yet implemented\n");
      return(-1);
  }

  return(0);    
  }

#ifdef USER_MODE

void dump_dlsch2(PHY_VARS_UE *phy_vars_ue,u8 eNB_id,u16 coded_bits_per_codeword) {

  unsigned int nsymb = (phy_vars_ue->lte_frame_parms.Ncp == 0) ? 14 : 12;
  char fname[32],vname[32];

  sprintf(fname,"dlsch%d_rxF_ext0.m",eNB_id);
  sprintf(vname,"dl%d_rxF_ext0",eNB_id);
  write_output(fname,vname,phy_vars_ue->lte_ue_dlsch_vars[eNB_id]->rxdataF_ext[0],300*nsymb,1,1);
  sprintf(fname,"dlsch%d_ch_ext00.m",eNB_id);
  sprintf(vname,"dl%d_ch_ext00",eNB_id);
  write_output(fname,vname,phy_vars_ue->lte_ue_dlsch_vars[eNB_id]->dl_ch_estimates_ext[0],300*nsymb,1,1);
  /*
    write_output("dlsch%d_ch_ext01.m","dl01_ch0_ext",lte_ue_dlsch_vars[eNB_id]->dl_ch_estimates_ext[1],300*nsymb,1,1);
    write_output("dlsch%d_ch_ext10.m","dl10_ch0_ext",lte_ue_dlsch_vars[eNB_id]->dl_ch_estimates_ext[2],300*nsymb,1,1);
    write_output("dlsch%d_ch_ext11.m","dl11_ch0_ext",lte_ue_dlsch_vars[eNB_id]->dl_ch_estimates_ext[3],300*nsymb,1,1);
    write_output("dlsch%d_rho.m","dl_rho",lte_ue_dlsch_vars[eNB_id]->rho[0],300*nsymb,1,1);
  */
  sprintf(fname,"dlsch%d_rxF_comp0.m",eNB_id);
  sprintf(vname,"dl%d_rxF_comp0",eNB_id);
  write_output(fname,vname,phy_vars_ue->lte_ue_dlsch_vars[eNB_id]->rxdataF_comp[0],300*nsymb,1,1);
  sprintf(fname,"dlsch%d_rxF_llr.m",eNB_id);
  sprintf(vname,"dl%d_llr",eNB_id);
  write_output(fname,vname, phy_vars_ue->lte_ue_dlsch_vars[eNB_id]->llr[0],coded_bits_per_codeword,1,0);
  /*
  write_output("dlsch%d_mag1.m","dl%d_mag1",phy_vars_ue->lte_ue_dlsch_vars[eNB_id]->dl_ch_mag,300*nsymb,1,1);
  write_output("dlsch%d_mag2.m","dl%d_mag2",phy_vars_ue->lte_ue_dlsch_vars[eNB_id]->dl_ch_magb,300*nsymb,1,1);
  */
}

#endif
