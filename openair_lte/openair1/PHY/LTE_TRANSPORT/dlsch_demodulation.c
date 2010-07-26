#include <emmintrin.h>
#include <xmmintrin.h>
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


//#define max(a,b) ((a)>(b) ? (a) : (b))

#define is_not_pilot(pilots,first_pilot,re) (pilots==0) || \
((pilots==1)&&(first_pilot==1)&&(((re>2)&&(re<6))||((re>8)&&(re<12)))) || \
((pilots==1)&&(first_pilot==0)&&(((re<3))||((re>5)&&(re<9))))


__m128i mmtmpD0,mmtmpD1,mmtmpD2,mmtmpD3;

//#define DEBUG_DLSCH_DEMOD

#ifdef DEBUG_DLSCH_DEMOD

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

#endif

__m128i *llr128;
short *llr;


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
			 unsigned short nb_rb) {

  __m128i *rxF=(__m128i*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
  __m128i *rxF_i=(__m128i*)&rxdataF_comp_i[0][(symbol*frame_parms->N_RB_DL*12)];
  __m128i *rho=(__m128i*)&rho_i[0][(symbol*frame_parms->N_RB_DL*12)];

  //  printf("dlsch_qpsk_qpsk: symbol %d\n",symbol);

  if (symbol == frame_parms->first_dlsch_symbol)
    llr128 = (__m128i*)dlsch_llr;
 
  if (!llr128) {
    msg("dlsch_qpsk_qpsk_llr: llr is null, symbol %d\n",symbol);
    return(-1);
  }

  qpsk_qpsk((short *)rxF,
	    (short *)rxF_i,
	    (short *)llr128,
	    (short *)rho,
	    nb_rb*12);

  llr128 += nb_rb*3;


}

int dlsch_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
		    int **rxdataF_comp,
		    short *dlsch_llr,
		    unsigned char symbol,
		    unsigned short nb_rb) {

  __m128i *rxF=(__m128i*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
  int i;

  if (symbol == frame_parms->first_dlsch_symbol)
    llr128 = (__m128i*)dlsch_llr;
 
  if (!llr128) {
    msg("dlsch_qpsk_llr: llr is null, symbol %d, llr128=%p\n",symbol, llr128);
    return(-1);
  } 
  //printf("qpsk llr for symbol %d (pos %d), llr offset %d\n",symbol,(symbol*frame_parms->N_RB_DL*12),llr128-(__m128i*)dlsch_llr);

  for (i=0;i<(nb_rb*3);i++) {
    *llr128 = *rxF;
    rxF++;
    llr128++;
  }

  _mm_empty();
  _m_empty();

  return(0);

}

void dlsch_16qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
		     int **rxdataF_comp,
		     short *dlsch_llr,
		     int **dl_ch_mag,
		     unsigned char symbol,
		     unsigned short nb_rb) {

  __m128i *rxF=(__m128i*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
  __m128i *ch_mag;
  int i;
  //  unsigned char symbol_mod;

//  printf("dlsch_rx.c: dlsch_16qam_llr: symbol %d\n",symbol);

  if (symbol == frame_parms->first_dlsch_symbol)
    llr128 = (__m128i*)&dlsch_llr[0];

  //  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;

  ch_mag =(__m128i*)&dl_ch_mag[0][(symbol*frame_parms->N_RB_DL*12)];


  for (i=0;i<(nb_rb*3);i++) {


    mmtmpD0 = _mm_abs_epi16(rxF[i]);
    //    print_shorts("tmp0",&tmp0);

    mmtmpD0 = _mm_subs_epi16(mmtmpD0,ch_mag[i]);


    llr128[0] = _mm_unpacklo_epi16(rxF[i],mmtmpD0);
    llr128[1] = _mm_unpackhi_epi16(rxF[i],mmtmpD0);
    llr128+=2;

    //    print_bytes("rxF[i]",&rxF[i]);
    //    print_bytes("rxF[i+1]",&rxF[i+1]);
  }

  _mm_empty();
  _m_empty();

}

void dlsch_64qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
		     int **rxdataF_comp,
		     short *dlsch_llr,
		     int **dl_ch_mag,
		     int **dl_ch_magb,
		     unsigned char symbol,
		     unsigned short nb_rb) {

  __m128i *rxF=(__m128i*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
  __m128i *ch_mag,*ch_magb;
  int j=0,i;
  //  unsigned char symbol_mod;


  if (symbol == frame_parms->first_dlsch_symbol)
    llr = dlsch_llr;

  //  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;

  ch_mag =(__m128i*)&dl_ch_mag[0][(symbol*frame_parms->N_RB_DL*12)];
  ch_magb =(__m128i*)&dl_ch_magb[0][(symbol*frame_parms->N_RB_DL*12)];


  for (i=0;i<(nb_rb*3);i++) {


    mmtmpD1 = _mm_abs_epi16(rxF[i]);
    mmtmpD1  = _mm_subs_epi16(mmtmpD1,ch_mag[i]);
    mmtmpD2 = _mm_abs_epi16(mmtmpD1);
    mmtmpD2 = _mm_subs_epi16(mmtmpD2,ch_magb[i]);

    for (j=0;j<8;j++) {
      llr[0] = ((short *)&rxF[i])[j];
      llr[1] = ((short *)&mmtmpD1)[j];
      llr[2] = ((short *)&mmtmpD2)[j];
      llr+=3;
    }

  }

  _mm_empty();
  _m_empty();

}

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


  //  printf("Doing alamouti!\n");
  rxF0     = (short*)&rxdataF_comp[0][jj];  //tx antenna 0  h0*y
  rxF1     = (short*)&rxdataF_comp[2][jj];  //tx antenna 1  h1*y
  ch_mag0 = (__m128i *)&dl_ch_mag[0][jj];
  ch_mag1 = (__m128i *)&dl_ch_mag[2][jj];
  ch_mag0b = (__m128i *)&dl_ch_magb[0][jj];
  ch_mag1b = (__m128i *)&dl_ch_magb[2][jj];
  for (rb=0;rb<nb_rb;rb++) {

    for (re=0;re<12;re+=2) {

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
    ch_mag0[2] = _mm_adds_epi16(ch_mag0[2],ch_mag1[2]);
    ch_mag0b[0] = _mm_adds_epi16(ch_mag0b[0],ch_mag1b[0]);
    ch_mag0b[1] = _mm_adds_epi16(ch_mag0b[1],ch_mag1b[1]);
    ch_mag0b[2] = _mm_adds_epi16(ch_mag0b[2],ch_mag1b[2]);

    ch_mag0+=3;
    ch_mag1+=3;
    ch_mag0b+=3;
    ch_mag1b+=3;
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
					LTE_DL_FRAME_PARMS *frame_parms) {


  unsigned short rb,nb_rb=0;
  unsigned char rb_alloc_ind;
  unsigned char i,aarx;
  int *dl_ch0,*dl_ch0_ext,*rxF,*rxF_ext;



  unsigned char symbol_mod;

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
  //  printf("extract_rbs: symbol_mod %d\n",symbol_mod);
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
	    printf("(%d %d)",((short *)dl_ch)[i<<1],((short*)dl_ch)[1+(i<<1)]);
	    printf("\n");*/
	  
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
	  /*	  
		 printf("rb %d\n",rb);
		 for (i=0;i<12;i++)
		 printf("(%d %d)",((short *)dl_ch0)[i<<1],((short*)dl_ch0)[1+(i<<1)]);
		 printf("\n");
	  */

	  memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	  for (i=0;i<12;i++)
	    rxF_ext[i]=rxF[i<<1];
	  nb_rb++;
	  dl_ch0_ext+=12;
	  rxF_ext+=12;
	}
	dl_ch0+=12;
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
      //	printf("dlch_ext %d\n",dl_ch0_ext-&dl_ch_estimates_ext[aarx][0]);      
      if (rb_alloc_ind==1) {
	
	for (i=0;i<6;i++) {
	  dl_ch0_ext[i]=dl_ch0[i];
	  rxF_ext[i]=rxF[i<<1];
	}
	rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))*2];
	for (;i<12;i++) {
	  dl_ch0_ext[i]=dl_ch0[i];
	  rxF_ext[i]=rxF[(1+i)<<1];
	}


	nb_rb++;
	dl_ch0_ext+=12;
	rxF_ext+=12;
      }
      else {
	rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))*2];
      }
      dl_ch0+=12;
#ifdef KHZ66_NULL
	dl_ch0_ext[8] = 0;
	dl_ch0_ext[9] = 0;
	dl_ch0_ext[10] = 0;
	dl_ch0_ext[11] = 0;
#endif
      rxF+=14;
      rb++;
      
      for (;rb<frame_parms->N_RB_DL;rb++) {
	//	printf("dlch_ext %d\n",dl_ch0_ext-&dl_ch_estimates_ext[aarx][0]);	
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
	  /*
  	    printf("rb %d\n",rb);
	    for (i=0;i<12;i++)
	    printf("(%d %d)",((short *)dl_ch0)[i<<1],((short*)dl_ch0)[1+(i<<1)]);
	    printf("\n");
	  */

	  memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	  for (i=0;i<12;i++)
	    rxF_ext[i]=rxF[i<<1];
	  nb_rb++;
	  dl_ch0_ext+=12;
	  rxF_ext+=12;
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
  unsigned char symbol_mod;
  unsigned char *pmi_loc = pmi_ext;

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
  //  printf("extract_rbs: symbol_mod %d\n",symbol_mod);
  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
    
    dl_ch0     = &dl_ch_estimates[aarx][5+(symbol_mod*(frame_parms->ofdm_symbol_size))];
    dl_ch0_ext = &dl_ch_estimates_ext[aarx][symbol_mod*(frame_parms->N_RB_DL*12)];
    dl_ch1     = &dl_ch_estimates[2+aarx][5+(symbol_mod*(frame_parms->ofdm_symbol_size))];
    dl_ch1_ext = &dl_ch_estimates_ext[2+aarx][symbol_mod*(frame_parms->N_RB_DL*12)];

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
	  //	  printf("rb %d, sb %d, pmi %d\n",rb,rb>>2,*pmi_loc);
	  pmi_loc++;
	  memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	  memcpy(dl_ch1_ext,dl_ch1,12*sizeof(int));
	  for (i=0;i<12;i++)
	    rxF_ext[i]=rxF[i<<1];
	  nb_rb++;
	  dl_ch0_ext+=12;
	  dl_ch1_ext+=12;
	  rxF_ext+=12;
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
	//	printf("rb %d, sb %d, pmi %d\n",rb,rb>>2,*pmi_loc);
	pmi_loc++;

	for (i=0;i<6;i++) {
	  dl_ch0_ext[i]=dl_ch0[i];
	  dl_ch1_ext[i]=dl_ch1[i];
	  rxF_ext[i]=rxF[i<<1];
	}

	rxF       = &rxdataF[aarx][((symbol*(frame_parms->ofdm_symbol_size)))*2];
	for (;i<12;i++) {
	  dl_ch0_ext[i]=dl_ch0[i];
	  dl_ch1_ext[i]=dl_ch1[i];
	  rxF_ext[i]=rxF[(1+i)<<1];
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
	nb_rb++;
	dl_ch0_ext+=12;
	dl_ch1_ext+=12;
	rxF_ext+=12;
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
	  //	  printf("rb %d, sb %d, pmi %d\n",rb,rb>>2,*pmi_loc);
	  pmi_loc++;

	  memcpy(dl_ch0_ext,dl_ch0,12*sizeof(int));
	  memcpy(dl_ch1_ext,dl_ch1,12*sizeof(int));
	  for (i=0;i<12;i++)
	    rxF_ext[i]=rxF[i<<1];
	  nb_rb++;
	  dl_ch0_ext+=12;
	  dl_ch1_ext+=12;
	  rxF_ext+=12;
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
				unsigned char mod_order,
				unsigned short nb_rb,
				unsigned char output_shift) {

  unsigned short rb;
  __m128i *dl_ch128,*dl_ch128_2,*dl_ch_mag128,*dl_ch_mag128b,*rxdataF128,*rxdataF_comp128,*rho128;
  unsigned char aatx,aarx,symbol_mod;


  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;

#ifndef __SSE3__
  zero = _mm_xor_si128(zero,zero);
#endif


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
	  
	  mmtmpD0 = _mm_madd_epi16(dl_ch128[2],dl_ch128[2]);
	  mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
	  mmtmpD1 = _mm_packs_epi32(mmtmpD0,mmtmpD0);
	  
	  dl_ch_mag128[2] = _mm_unpacklo_epi16(mmtmpD1,mmtmpD1);
	  dl_ch_mag128b[2] = dl_ch_mag128[2];

	  dl_ch_mag128[2] = _mm_mulhi_epi16(dl_ch_mag128[2],QAM_amp128);
	  dl_ch_mag128[2] = _mm_slli_epi16(dl_ch_mag128[2],1);	  


	  dl_ch_mag128b[0] = _mm_mulhi_epi16(dl_ch_mag128b[0],QAM_amp128b);
	  dl_ch_mag128b[0] = _mm_slli_epi16(dl_ch_mag128b[0],1);
	  

	  dl_ch_mag128b[1] = _mm_mulhi_epi16(dl_ch_mag128b[1],QAM_amp128b);
	  dl_ch_mag128b[1] = _mm_slli_epi16(dl_ch_mag128b[1],1);
	  
	  dl_ch_mag128b[2] = _mm_mulhi_epi16(dl_ch_mag128b[2],QAM_amp128b);
	  dl_ch_mag128b[2] = _mm_slli_epi16(dl_ch_mag128b[2],1);	  
	  
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
      
      if (symbol == frame_parms->first_dlsch_symbol) {
	PHY_vars->PHY_measurements.rx_correlation[0][aarx] = signal_energy(&rho[aarx][symbol*frame_parms->N_RB_DL*12],rb*12);
      } 
         
    }

  }

  _mm_empty();
  _m_empty();

}     

static __m128i  one_over_sqrt2;
 	 
void prec2A_128(unsigned char pmi,__m128i *ch0,__m128i *ch1) {
  
  __m128i one_over_sqrt2 = _mm_set1_epi16(ONE_OVER_SQRT2_Q15);

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
				     unsigned char symbol,
				     unsigned char mod_order,
				     unsigned short nb_rb,
				     unsigned char output_shift) {
  
  unsigned short rb;
  __m128i *dl_ch128_0,*dl_ch128_1,*dl_ch_mag128,*dl_ch_mag128b,*rxdataF128,*rxdataF_comp128;
  unsigned char aatx=0,aarx=0,symbol_mod;
  int precoded_signal_strength=0,rx_power_correction;

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;

  if ( (frame_parms->ofdm_symbol_size == 128) ||
       (frame_parms->ofdm_symbol_size == 512) )
    rx_power_correction = 2;
  else
    rx_power_correction = 1;

#ifndef __SSE3__
  zero = _mm_xor_si128(zero,zero);
#endif

  //  printf("comp prec: symbol %d\n",symbol);

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
      //#ifdef DEBUG_DLSCH_DEMOD
      //printf("mode 6 prec: rb %d, pmi->%d\n",rb,pmi_ext[rb]);

      
      //      print_shorts("ch0(0):",&dl_ch128_0[0]);
      //      print_shorts("ch1(0):",&dl_ch128_1[0]);
      prec2A_128(pmi_ext[rb],&dl_ch128_0[0],&dl_ch128_1[0]);
      //      print_shorts("prec(ch0,ch1):",&dl_ch128_0[0]);
      //      print_shorts("ch0(1):",&dl_ch128_0[1]);
      //      print_shorts("ch1(1):",&dl_ch128_1[1]);
      prec2A_128(pmi_ext[rb],&dl_ch128_0[1],&dl_ch128_1[1]);
      //      print_shorts("prec(ch0,ch1):",&dl_ch128_0[1]);
      //      print_shorts("ch0(2):",&dl_ch128_0[2]);
      //      print_shorts("ch1(2):",&dl_ch128_1[2]); 
      prec2A_128(pmi_ext[rb],&dl_ch128_0[2],&dl_ch128_1[2]); 

      //      print_shorts("prec(ch0,ch1):",&dl_ch128_0[2]);      
      

      //      print_shorts("prec(ch0,ch1):",&dl_ch128_0[2]);      
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
	
	dl_ch_mag128[1] = _mm_unpackhi_epi16(mmtmpD0,mmtmpD0);
	dl_ch_mag128b[1] = dl_ch_mag128[1];
	dl_ch_mag128[1] = _mm_mulhi_epi16(dl_ch_mag128[1],QAM_amp128);
	dl_ch_mag128[1] = _mm_slli_epi16(dl_ch_mag128[1],1);
	
	mmtmpD0 = _mm_madd_epi16(dl_ch128_0[2],dl_ch128_0[2]);
	mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
	mmtmpD1 = _mm_packs_epi32(mmtmpD0,mmtmpD0);
	
	dl_ch_mag128[2] = _mm_unpacklo_epi16(mmtmpD1,mmtmpD1);
	dl_ch_mag128b[2] = dl_ch_mag128[2];
	
	dl_ch_mag128[2] = _mm_mulhi_epi16(dl_ch_mag128[2],QAM_amp128);
	dl_ch_mag128[2] = _mm_slli_epi16(dl_ch_mag128[2],1);	  
	
	
	dl_ch_mag128b[0] = _mm_mulhi_epi16(dl_ch_mag128b[0],QAM_amp128b);
	dl_ch_mag128b[0] = _mm_slli_epi16(dl_ch_mag128b[0],1);
	
	
	dl_ch_mag128b[1] = _mm_mulhi_epi16(dl_ch_mag128b[1],QAM_amp128b);
	dl_ch_mag128b[1] = _mm_slli_epi16(dl_ch_mag128b[1],1);
	
	dl_ch_mag128b[2] = _mm_mulhi_epi16(dl_ch_mag128b[2],QAM_amp128b);
	dl_ch_mag128b[2] = _mm_slli_epi16(dl_ch_mag128b[2],1);	  
	
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

    precoded_signal_strength += ((signal_energy_nodc(&dl_ch_estimates_ext[aarx][symbol_mod*frame_parms->N_RB_DL*12],
						    (nb_rb*12))*rx_power_correction) - 
      (phy_measurements->n0_power[aarx]));
  } // rx_antennas

  phy_measurements->precoded_cqi_dB[0][0] = dB_fixed2(precoded_signal_strength,phy_measurements->n0_power_tot);

  _mm_empty();
  _m_empty();
  
}     
	 
    
__m128i avg128D;

//compute average channel_level on each (TX,RX) antenna pair
void dlsch_channel_level(int **dl_ch_estimates_ext,
			 LTE_DL_FRAME_PARMS *frame_parms,
			 int *avg,
			 unsigned short nb_rb){

  short rb;
  unsigned char aatx,aarx;
  __m128i *dl_ch128;
  

  for (aatx=0;aatx<frame_parms->nb_antennas_tx;aatx++)
    for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
      //clear average level
      avg128D = _mm_xor_si128(avg128D,avg128D);
      dl_ch128=(__m128i *)&dl_ch_estimates_ext[(aatx<<1)+aarx][frame_parms->first_dlsch_symbol*frame_parms->N_RB_DL*12];

      for (rb=0;rb<nb_rb;rb++) {
    
	avg128D = _mm_add_epi32(avg128D,_mm_madd_epi16(dl_ch128[0],dl_ch128[0]));
	avg128D = _mm_add_epi32(avg128D,_mm_madd_epi16(dl_ch128[1],dl_ch128[1]));
	avg128D = _mm_add_epi32(avg128D,_mm_madd_epi16(dl_ch128[2],dl_ch128[2]));

	dl_ch128+=3;	
	/*
	  if (rb==0) {
	  print_shorts("dl_ch128",&dl_ch128[0]);
	  print_shorts("dl_ch128",&dl_ch128[1]);
	  print_shorts("dl_ch128",&dl_ch128[2]);
	  }
	*/
      }

      avg[(aatx<<1)+aarx] = (((int*)&avg128D)[0] + 
			     ((int*)&avg128D)[1] + 
			     ((int*)&avg128D)[2] + 
			     ((int*)&avg128D)[3])/(nb_rb*12);

      //            printf("Channel level : %d\n",avg[(aatx<<1)+aarx]);
    }
  _mm_empty();
  _m_empty();

}

int avg[4];

int rx_dlsch(LTE_UE_COMMON *lte_ue_common_vars,
	     LTE_UE_DLSCH **lte_ue_dlsch_vars,
	     LTE_DL_FRAME_PARMS *frame_parms,
	     unsigned char eNb_id,
	     unsigned char eNb_id_i,
	     LTE_UE_DLSCH_t **dlsch_ue,
	     unsigned char symbol,
	     unsigned char dual_stream_UE) {
  
  unsigned short nb_rb;

  unsigned char log2_maxh,aatx,aarx;
  int avgs;

  unsigned char harq_pid0;

  if (eNb_id > 2) {
    msg("dlsch_demodulation.c: Illegal eNb_id %d\n",eNb_id);
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

  if (!lte_frame_parms) {
    msg("dlsch_demodulation.c: Null lte_frame_parms\n");
    return(-1);
  }

  harq_pid0 = dlsch_ue[0]->current_harq_pid;
  //  printf("rx_dlsch (eNb_id %d, dlsch_vars %p): symbol %d, rb_alloc[0] %x, pmi %x\n",eNb_id,lte_ue_dlsch_vars[eNb_id],symbol,dlsch_ue[0]->rb_alloc[0],pmi2hex_2Ar1(dlsch_ue[0]->pmi_alloc));

  if (frame_parms->nb_antennas_tx>1) {
    //debug_msg("dlsch: using pmi %x (%p), rb_alloc %x\n",pmi2hex_2Ar1(dlsch_ue[0]->pmi_alloc),dlsch_ue[0],dlsch_ue[0]->rb_alloc[0]);

    nb_rb = dlsch_extract_rbs_dual(lte_ue_common_vars->rxdataF,
				   lte_ue_common_vars->dl_ch_estimates[eNb_id],
				   lte_ue_dlsch_vars[eNb_id]->rxdataF_ext,
				   lte_ue_dlsch_vars[eNb_id]->dl_ch_estimates_ext,
				   dlsch_ue[0]->pmi_alloc,
				   lte_ue_dlsch_vars[eNb_id]->pmi_ext,
				   dlsch_ue[0]->rb_alloc,
				   symbol,
				   frame_parms);
    if (dual_stream_UE == 1)
      nb_rb = dlsch_extract_rbs_dual(lte_ue_common_vars->rxdataF,
				     lte_ue_common_vars->dl_ch_estimates[eNb_id_i],
				     lte_ue_dlsch_vars[eNb_id_i]->rxdataF_ext,
				     lte_ue_dlsch_vars[eNb_id_i]->dl_ch_estimates_ext,
				     dlsch_ue[0]->pmi_alloc,
				     lte_ue_dlsch_vars[eNb_id]->pmi_ext,
				     dlsch_ue[0]->rb_alloc,
				     symbol,
				     frame_parms);
  }
  else {



      nb_rb = dlsch_extract_rbs_single(lte_ue_common_vars->rxdataF,
				       lte_ue_common_vars->dl_ch_estimates[eNb_id],
				       lte_ue_dlsch_vars[eNb_id]->rxdataF_ext,
				       lte_ue_dlsch_vars[eNb_id]->dl_ch_estimates_ext,
				       dlsch_ue[0]->pmi_alloc,
				       lte_ue_dlsch_vars[eNb_id]->pmi_ext,
				       dlsch_ue[0]->rb_alloc,
				       symbol,
				       frame_parms);
      if (dual_stream_UE == 1)
	nb_rb = dlsch_extract_rbs_single(lte_ue_common_vars->rxdataF,
					 lte_ue_common_vars->dl_ch_estimates[eNb_id_i],
					 lte_ue_dlsch_vars[eNb_id_i]->rxdataF_ext,
					 lte_ue_dlsch_vars[eNb_id_i]->dl_ch_estimates_ext,    
					 dlsch_ue[0]->pmi_alloc,
					 lte_ue_dlsch_vars[eNb_id]->pmi_ext,
					 dlsch_ue[0]->rb_alloc,
					 symbol,
					 frame_parms);

  }

  //    printf("nb_rb = %d, eNb_id %d\n",nb_rb,eNb_id);

  if (symbol==frame_parms->first_dlsch_symbol) {
    dlsch_channel_level(lte_ue_dlsch_vars[eNb_id]->dl_ch_estimates_ext,
			frame_parms,
			avg,
			nb_rb);
#ifdef DEBUG_PHY
    msg("[DLSCH] avg[0] %d\n",avg[0]);
#endif
  }

  avgs = 0;
  for (aatx=0;aatx<frame_parms->nb_antennas_tx;aatx++)
    for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++)
      avgs = max(avgs,avg[(aarx<<1)+aatx]);

  log2_maxh = 4+(log2_approx(avgs)/2);
#ifdef DEBUG_PHY
  msg("[DLSCH] log2_maxh = %d (%d,%d)\n",log2_maxh,avg[0],avgs);
  msg("[DLSCH] mimo_mode = %d\n", dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode);
#endif
 

  if (dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode<UNIFORM_PRECODING11) {// no precoding

    //    printf("channel compensation, no precoding, eNb_id %d, lte_ue_dlsch_vars %p, lte_ue_dlsch_vars[eNb_id] %p\n",eNb_id,lte_ue_dlsch_vars,lte_ue_dlsch_vars[eNb_id]);
    dlsch_channel_compensation(lte_ue_dlsch_vars[eNb_id]->rxdataF_ext,
			       lte_ue_dlsch_vars[eNb_id]->dl_ch_estimates_ext,
			       lte_ue_dlsch_vars[eNb_id]->dl_ch_mag,
			       lte_ue_dlsch_vars[eNb_id]->dl_ch_magb,
			       lte_ue_dlsch_vars[eNb_id]->rxdataF_comp,
			       (aatx>1) ? lte_ue_dlsch_vars[eNb_id]->rho : NULL,
			       frame_parms,
			       symbol,
			       get_Qm(dlsch_ue[0]->harq_processes[harq_pid0]->mcs),
			       nb_rb,
			       log2_maxh); // log2_maxh+I0_shift

    if (dual_stream_UE == 1) {
      // get MF output for interfering stream
      dlsch_channel_compensation(lte_ue_dlsch_vars[eNb_id]->rxdataF_ext,
				 lte_ue_dlsch_vars[eNb_id_i]->dl_ch_estimates_ext,
				 lte_ue_dlsch_vars[eNb_id_i]->dl_ch_mag,
				 lte_ue_dlsch_vars[eNb_id_i]->dl_ch_magb,
				 lte_ue_dlsch_vars[eNb_id_i]->rxdataF_comp,
				 (aatx>1) ? lte_ue_dlsch_vars[eNb_id_i]->rho : NULL,
				 frame_parms,
				 symbol,
				 get_Qm(dlsch_ue[1]->harq_processes[harq_pid0]->mcs),
				 nb_rb,
				 log2_maxh); // log2_maxh+I0_shift
      

    // compute correlation between signal and interference channels
      dlsch_dual_stream_correlation(frame_parms,
				    symbol,
				    nb_rb,
				    lte_ue_dlsch_vars[eNb_id]->dl_ch_estimates_ext,
				    lte_ue_dlsch_vars[eNb_id_i]->dl_ch_estimates_ext,
				    lte_ue_dlsch_vars[eNb_id]->dl_ch_rho_ext,
				    log2_maxh);
    
    }

  }
  else if (dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode<DUALSTREAM_UNIFORM_PRECODING1) {// single-layer precoding
    //    printf("Channel compensation for precoding\n");
    dlsch_channel_compensation_prec(lte_ue_dlsch_vars[eNb_id]->rxdataF_ext,
				    lte_ue_dlsch_vars[eNb_id]->dl_ch_estimates_ext,
				    lte_ue_dlsch_vars[eNb_id]->dl_ch_mag,
				    lte_ue_dlsch_vars[eNb_id]->dl_ch_magb,
				    lte_ue_dlsch_vars[eNb_id]->rxdataF_comp,
				    lte_ue_dlsch_vars[eNb_id]->pmi_ext,
				    frame_parms,
				    &PHY_vars->PHY_measurements,
				    symbol,
				    get_Qm(dlsch_ue[0]->harq_processes[harq_pid0]->mcs),
				    nb_rb,
				    log2_maxh);
    //    printf("Channel compensation for precoding done\n");
  }
  if (frame_parms->nb_antennas_rx > 1)
    dlsch_detection_mrc(frame_parms,
			lte_ue_dlsch_vars[eNb_id]->rxdataF_comp,
			lte_ue_dlsch_vars[eNb_id_i]->rxdataF_comp,
			lte_ue_dlsch_vars[eNb_id]->rho,
			lte_ue_dlsch_vars[eNb_id]->dl_ch_rho_ext,
			lte_ue_dlsch_vars[eNb_id]->dl_ch_mag,
			lte_ue_dlsch_vars[eNb_id]->dl_ch_magb,
			symbol,
			nb_rb,
			dual_stream_UE);

  // Single-layer transmission formats
  if (dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode<DUALSTREAM_UNIFORM_PRECODING1) {
    if ((dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode == SISO) ||
	((dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode >= UNIFORM_PRECODING11) &&
	 (dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode <= PUSCH_PRECODING0)))
      dlsch_siso(frame_parms,lte_ue_dlsch_vars[eNb_id]->rxdataF_comp,lte_ue_dlsch_vars[eNb_id_i]->rxdataF_comp,symbol,nb_rb);
    else if (dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode == ALAMOUTI)
      dlsch_alamouti(frame_parms,lte_ue_dlsch_vars[eNb_id]->rxdataF_comp,lte_ue_dlsch_vars[eNb_id]->dl_ch_mag,lte_ue_dlsch_vars[eNb_id]->dl_ch_magb,symbol,nb_rb);
    else if (dlsch_ue[0]->harq_processes[harq_pid0]->mimo_mode == ANTCYCLING)
      dlsch_antcyc(frame_parms,lte_ue_dlsch_vars[eNb_id]->rxdataF_comp,lte_ue_dlsch_vars[eNb_id]->dl_ch_mag,lte_ue_dlsch_vars[eNb_id]->dl_ch_magb,symbol,nb_rb);
    else {
      msg("dlsch_rx: Unknown MIMO mode\n");
      return (-1);
    }
    switch (get_Qm(dlsch_ue[0]->harq_processes[harq_pid0]->mcs)) {
    case 2 : 
      if (dual_stream_UE == 0)
	dlsch_qpsk_llr(frame_parms,lte_ue_dlsch_vars[eNb_id]->rxdataF_comp,lte_ue_dlsch_vars[eNb_id]->llr[0],symbol,nb_rb);
      else
	dlsch_qpsk_qpsk_llr(frame_parms,
			    lte_ue_dlsch_vars[eNb_id]->rxdataF_comp,
			    lte_ue_dlsch_vars[eNb_id_i]->rxdataF_comp,
			    lte_ue_dlsch_vars[eNb_id]->dl_ch_rho_ext,
			    lte_ue_dlsch_vars[eNb_id]->llr[0],symbol,nb_rb);
      break;
    case 4 :
      dlsch_16qam_llr(frame_parms,lte_ue_dlsch_vars[eNb_id]->rxdataF_comp,lte_ue_dlsch_vars[eNb_id]->llr[0],lte_ue_dlsch_vars[eNb_id]->dl_ch_mag,symbol,nb_rb);
      break;
    case 6 :
      dlsch_64qam_llr(frame_parms,lte_ue_dlsch_vars[eNb_id]->rxdataF_comp,lte_ue_dlsch_vars[eNb_id]->llr[0],lte_ue_dlsch_vars[eNb_id]->dl_ch_mag,lte_ue_dlsch_vars[eNb_id]->dl_ch_magb,symbol,nb_rb);
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
