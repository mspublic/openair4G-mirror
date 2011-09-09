#include <emmintrin.h>
#include <xmmintrin.h>
#ifdef __SSE3__
#include <pmmintrin.h>
#include <tmmintrin.h>
#endif
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "defs.h"
#include "extern.h"
//#define DEBUG_ULSCH

#ifndef __SSE3__
__m128i zeroU;
#define _mm_abs_epi16(xmmx) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zeroU,(xmmx)))
#define _mm_sign_epi16(xmmx,xmmy) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zeroU,(xmmy)))
#endif

__m128i idft_in128[3][1200],idft_out128[3][1200];

#ifndef OFDMA_ULSCH
void lte_idft(LTE_DL_FRAME_PARMS *frame_parms,int *z, unsigned short Msc_PUSCH) {

  int *idft_in0=(int*)idft_in128[0],*idft_out0=(int*)idft_out128[0];
  int *idft_in1=(int*)idft_in128[1],*idft_out1=(int*)idft_out128[1];
  int *idft_in2=(int*)idft_in128[2],*idft_out2=(int*)idft_out128[2];


  int *z0,*z1,*z2,*z3,*z4,*z5,*z6,*z7,*z8,*z9,*z10,*z11;
  int i,ip;

  //  printf("Doing lte_idft for Msc_PUSCH %d\n",Msc_PUSCH);

  if (frame_parms->Ncp == 0) { // Normal prefix
    z0 = z;
    z1 = z0+(frame_parms->N_RB_DL*12);
    z2 = z1+(frame_parms->N_RB_DL*12);
    //pilot
    z3 = z2+(2*frame_parms->N_RB_DL*12);
    z4 = z3+(frame_parms->N_RB_DL*12);
    z5 = z4+(frame_parms->N_RB_DL*12);

    z6 = z5+(frame_parms->N_RB_DL*12);
    z7 = z6+(frame_parms->N_RB_DL*12);
    z8 = z7+(frame_parms->N_RB_DL*12);
    //pilot
    z9 = z8+(2*frame_parms->N_RB_DL*12);
    z10 = z9+(frame_parms->N_RB_DL*12);
    // srs
    z11 = z10+(frame_parms->N_RB_DL*12);
  }
  else {   // extended prefix
    z0 = z;
    z1 = z0+(frame_parms->N_RB_DL*12);
    //pilot
    z2 = z1+(2*frame_parms->N_RB_DL*12);
    z3 = z2+(frame_parms->N_RB_DL*12);
    z4 = z3+(frame_parms->N_RB_DL*12);

    z5 = z4+(frame_parms->N_RB_DL*12);
    z6 = z5+(frame_parms->N_RB_DL*12);
    //pilot
    z7 = z6+(2*frame_parms->N_RB_DL*12);
    z8 = z7+(frame_parms->N_RB_DL*12);
    // srs
    z9 = z8+(frame_parms->N_RB_DL*12);
  }
  // conjugate input
  for (i=0;i<(Msc_PUSCH>>2);i++) {
    *&(((__m128i*)z0)[i])=_mm_sign_epi16(*&(((__m128i*)z0)[i]),*(__m128i*)&conjugate2[0]);
    *&(((__m128i*)z1)[i])=_mm_sign_epi16(*&(((__m128i*)z1)[i]),*(__m128i*)&conjugate2[0]);
    *&(((__m128i*)z2)[i])=_mm_sign_epi16(*&(((__m128i*)z2)[i]),*(__m128i*)&conjugate2[0]);
    *&(((__m128i*)z3)[i])=_mm_sign_epi16(*&(((__m128i*)z3)[i]),*(__m128i*)&conjugate2[0]);
    *&(((__m128i*)z4)[i])=_mm_sign_epi16(*&(((__m128i*)z4)[i]),*(__m128i*)&conjugate2[0]);
    *&(((__m128i*)z5)[i])=_mm_sign_epi16(*&(((__m128i*)z5)[i]),*(__m128i*)&conjugate2[0]);
    *&(((__m128i*)z6)[i])=_mm_sign_epi16(*&(((__m128i*)z6)[i]),*(__m128i*)&conjugate2[0]);
    *&(((__m128i*)z7)[i])=_mm_sign_epi16(*&(((__m128i*)z7)[i]),*(__m128i*)&conjugate2[0]);
    *&(((__m128i*)z8)[i])=_mm_sign_epi16(*&(((__m128i*)z8)[i]),*(__m128i*)&conjugate2[0]);
    *&(((__m128i*)z9)[i])=_mm_sign_epi16(*&(((__m128i*)z9)[i]),*(__m128i*)&conjugate2[0]);
    if (frame_parms->Ncp==0) {
      *&(((__m128i*)z10)[i])=_mm_sign_epi16(*&(((__m128i*)z10)[i]),*(__m128i*)&conjugate2[0]);
      *&(((__m128i*)z11)[i])=_mm_sign_epi16(*&(((__m128i*)z11)[i]),*(__m128i*)&conjugate2[0]);
    }
  } 
  
  for (i=0,ip=0;i<Msc_PUSCH;i++,ip+=4) { 
    idft_in0[ip+0]   =  z0[i];
    idft_in0[ip+1] =  z1[i];
    idft_in0[ip+2] =  z2[i];
    idft_in0[ip+3] =  z3[i];
    idft_in1[ip+0] =  z4[i];
    idft_in1[ip+1] =  z5[i];
    idft_in1[ip+2] =  z6[i];
    idft_in1[ip+3] =  z7[i];
    idft_in2[ip+0]   =  z8[i];
    idft_in2[ip+1] =  z9[i];
    if (frame_parms->Ncp==0) {
      idft_in2[ip+2] =  z10[i];
      idft_in2[ip+3] =  z11[i];
    }
  }
  
  
  switch (Msc_PUSCH) {
  case 12:
    //    dft12f(dft_in0,dft_out0,1);
    //    dft12f(idft_in1,idft_out1,1);
    //    dft12f(idft_in2,idft_out2,1);
    break;
  case 24:
    dft24(idft_in0,idft_out0,1);
    dft24(idft_in1,idft_out1,1);
    dft24(idft_in2,idft_out2,1);
    break;
  case 36:
    dft36(idft_in0,idft_out0,1);
    dft36(idft_in1,idft_out1,1);
    dft36(idft_in2,idft_out2,1);
    break;
  case 48:
    dft48(idft_in0,idft_out0,1);
    dft48(idft_in1,idft_out1,1);
    dft48(idft_in2,idft_out2,1);
    break;
  case 60:
    dft60(idft_in0,idft_out0,1);
    dft60(idft_in1,idft_out1,1);
    dft60(idft_in2,idft_out2,1);
    break;
  case 72:
    dft72(idft_in0,idft_out0,1);
    dft72(idft_in1,idft_out1,1);
    dft72(idft_in2,idft_out2,1);
    break;
  case 96:
    dft96(idft_in0,idft_out0,1);
    dft96(idft_in1,idft_out1,1);
    dft96(idft_in2,idft_out2,1);
    break;
  case 108:
    dft108(idft_in0,idft_out0,1);
    dft108(idft_in1,idft_out1,1);
    dft108(idft_in2,idft_out2,1);
    break;
  case 120:
    dft120(idft_in0,idft_out0,1);
    dft120(idft_in1,idft_out1,1);
    dft120(idft_in2,idft_out2,1);
    break;
  case 144:
    dft144(idft_in0,idft_out0,1);
    dft144(idft_in1,idft_out1,1);
    dft144(idft_in2,idft_out2,1);
    break;
  case 180:
    dft180(idft_in0,idft_out0,1);
    dft180(idft_in1,idft_out1,1);
    dft180(idft_in2,idft_out2,1);
    break;
  case 192:
    dft192(idft_in0,idft_out0,1);
    dft192(idft_in1,idft_out1,1);
    dft192(idft_in2,idft_out2,1);
    break;
  case 240:
    dft240(idft_in0,idft_out0,1);
    dft240(idft_in1,idft_out1,1);
    dft240(idft_in2,idft_out2,1);
    break;
  case 288:
    dft288(idft_in0,idft_out0,1);
    dft288(idft_in1,idft_out1,1);
    dft288(idft_in2,idft_out2,1);
    break;
  case 300:
    dft300(idft_in0,idft_out0,1);
    dft300(idft_in1,idft_out1,1);
    dft300(idft_in2,idft_out2,1);
    break;
    
  }

    

  for (i=0,ip=0;i<Msc_PUSCH;i++,ip+=4) {
    z0[i]     = idft_out0[ip];
    /*
      printf("out0 (%d,%d),(%d,%d),(%d,%d),(%d,%d)\n",
      ((short*)&idft_out0[ip])[0],((short*)&idft_out0[ip])[1],
      ((short*)&idft_out0[ip+1])[0],((short*)&idft_out0[ip+1])[1],
      ((short*)&idft_out0[ip+2])[0],((short*)&idft_out0[ip+2])[1],
      ((short*)&idft_out0[ip+3])[0],((short*)&idft_out0[ip+3])[1]);
    */
    z1[i]     = idft_out0[ip+1]; 
    z2[i]     = idft_out0[ip+2]; 
    z3[i]     = idft_out0[ip+3]; 
    z4[i]     = idft_out1[ip+0]; 
    z5[i]     = idft_out1[ip+1]; 
    z6[i]     = idft_out1[ip+2]; 
    z7[i]     = idft_out1[ip+3]; 
    z8[i]     = idft_out2[ip]; 
    z9[i]     = idft_out2[ip+1]; 
    if (frame_parms->Ncp==0) {
      z10[i]    = idft_out2[ip+2]; 
      z11[i]    = idft_out2[ip+3];
    }
  }
  
  // conjugate output
  for (i=0;i<(Msc_PUSCH>>2);i++) {
    ((__m128i*)z0)[i]=_mm_sign_epi16(((__m128i*)z0)[i],*(__m128i*)&conjugate2[0]);
    ((__m128i*)z1)[i]=_mm_sign_epi16(((__m128i*)z1)[i],*(__m128i*)&conjugate2[0]);
    ((__m128i*)z2)[i]=_mm_sign_epi16(((__m128i*)z2)[i],*(__m128i*)&conjugate2[0]);
    ((__m128i*)z3)[i]=_mm_sign_epi16(((__m128i*)z3)[i],*(__m128i*)&conjugate2[0]);
    ((__m128i*)z4)[i]=_mm_sign_epi16(((__m128i*)z4)[i],*(__m128i*)&conjugate2[0]);
    ((__m128i*)z5)[i]=_mm_sign_epi16(((__m128i*)z5)[i],*(__m128i*)&conjugate2[0]);
    ((__m128i*)z6)[i]=_mm_sign_epi16(((__m128i*)z6)[i],*(__m128i*)&conjugate2[0]);
    ((__m128i*)z7)[i]=_mm_sign_epi16(((__m128i*)z7)[i],*(__m128i*)&conjugate2[0]);
    ((__m128i*)z8)[i]=_mm_sign_epi16(((__m128i*)z8)[i],*(__m128i*)&conjugate2[0]);
    ((__m128i*)z9)[i]=_mm_sign_epi16(((__m128i*)z9)[i],*(__m128i*)&conjugate2[0]);
    if (frame_parms->Ncp==0) {
      ((__m128i*)z10)[i]=_mm_sign_epi16(((__m128i*)z10)[i],*(__m128i*)&conjugate2[0]);
      ((__m128i*)z11)[i]=_mm_sign_epi16(((__m128i*)z11)[i],*(__m128i*)&conjugate2[0]);
    }
  } 
}
#endif


__m128i mmtmpU0,mmtmpU1,mmtmpU2,mmtmpU3;
__m128i *llr128U;
short *llrU;

int ulsch_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
		   int **rxdataF_comp,
		   short *ulsch_llr,
		   unsigned char symbol,
		   unsigned short nb_rb) {

  __m128i *rxF=(__m128i*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
  int i;

  if (symbol == 0)
    llr128U = (__m128i*)ulsch_llr;
 
  if (!llr128U) {
    msg("ulsch_qpsk_llr: llr is null, symbol %d, llr128=%p\n",symbol, llr128U);
    return(-1);
  }
  //  printf("qpsk llr for symbol %d (pos %d), llr offset %d\n",symbol,(symbol*frame_parms->N_RB_DL*12),llr128-(__m128i*)ulsch_llr);

  for (i=0;i<(nb_rb*3);i++) {
    *llr128U = *rxF;
    rxF++;
    llr128U++;
  }

  _mm_empty();
  _m_empty();

  return(0);

}

void ulsch_16qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
		     int **rxdataF_comp,
		     short *ulsch_llr,
		     int **ul_ch_mag,
		     unsigned char symbol,
		     unsigned short nb_rb) {

  __m128i *rxF=(__m128i*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
  __m128i *ch_mag;
  int i;
  //  unsigned char symbol_mod;

  //  printf("ulsch_rx.c: ulsch_16qam_llr: symbol %d\n",symbol);

  if (symbol == 0)
    llr128U = (__m128i*)&ulsch_llr[0];

  //  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;

  ch_mag =(__m128i*)&ul_ch_mag[0][(symbol*frame_parms->N_RB_DL*12)];


  for (i=0;i<(nb_rb*3);i++) {


    mmtmpU0 = _mm_abs_epi16(rxF[i]);
    //    print_shorts("tmp0",&tmp0);

    mmtmpU0 = _mm_subs_epi16(mmtmpU0,ch_mag[i]);


    llr128U[0] = _mm_unpacklo_epi16(rxF[i],mmtmpU0);
    llr128U[1] = _mm_unpackhi_epi16(rxF[i],mmtmpU0);
    llr128U+=2;

    //    print_bytes("rxF[i]",&rxF[i]);
    //    print_bytes("rxF[i+1]",&rxF[i+1]);
  }

  _mm_empty();
  _m_empty();

}

void ulsch_64qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
		     int **rxdataF_comp,
		     short *ulsch_llr,
		     int **ul_ch_mag,
		     int **ul_ch_magb,
		     unsigned char symbol,
		     unsigned short nb_rb) {

  __m128i *rxF=(__m128i*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
  __m128i *ch_mag,*ch_magb;
  int j=0,i;
  //  unsigned char symbol_mod;


  if (symbol == 0)
    llrU = ulsch_llr;

  //  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;

  ch_mag =(__m128i*)&ul_ch_mag[0][(symbol*frame_parms->N_RB_DL*12)];
  ch_magb =(__m128i*)&ul_ch_magb[0][(symbol*frame_parms->N_RB_DL*12)];


  for (i=0;i<(nb_rb*3);i++) {


    mmtmpU1 = _mm_abs_epi16(rxF[i]);
    mmtmpU1  = _mm_subs_epi16(mmtmpU1,ch_mag[i]);
    mmtmpU2 = _mm_abs_epi16(mmtmpU1);
    mmtmpU2 = _mm_subs_epi16(mmtmpU2,ch_magb[i]);

    for (j=0;j<8;j++) {
      llrU[0] = ((short *)&rxF[i])[j];
      llrU[1] = ((short *)&mmtmpU1)[j];
      llrU[2] = ((short *)&mmtmpU2)[j];
      llrU+=3;
    }

  }

  _mm_empty();
  _m_empty();

}

void ulsch_detection_mrc(LTE_DL_FRAME_PARMS *frame_parms,
			 int **rxdataF_comp,
			 int **ul_ch_mag,
			 int **ul_ch_magb,
			 unsigned char symbol,
			 unsigned short nb_rb) {



  __m128i *rxdataF_comp128_0,*ul_ch_mag128_0,*ul_ch_mag128_0b;
  __m128i *rxdataF_comp128_1,*ul_ch_mag128_1,*ul_ch_mag128_1b;

  int i;

  if (frame_parms->nb_antennas_rx>1) {
    rxdataF_comp128_0   = (__m128i *)&rxdataF_comp[0][symbol*frame_parms->N_RB_DL*12];  
    rxdataF_comp128_1   = (__m128i *)&rxdataF_comp[1][symbol*frame_parms->N_RB_DL*12];  
    ul_ch_mag128_0      = (__m128i *)&ul_ch_mag[0][symbol*frame_parms->N_RB_DL*12];  
    ul_ch_mag128_1      = (__m128i *)&ul_ch_mag[1][symbol*frame_parms->N_RB_DL*12];  
    ul_ch_mag128_0b     = (__m128i *)&ul_ch_magb[0][symbol*frame_parms->N_RB_DL*12];  
    ul_ch_mag128_1b     = (__m128i *)&ul_ch_magb[1][symbol*frame_parms->N_RB_DL*12];  

    // MRC on each re of rb, both on MF output and magnitude (for 16QAM/64QAM llr computation)
    for (i=0;i<nb_rb*3;i++) {
      rxdataF_comp128_0[i] = _mm_adds_epi16(_mm_srai_epi16(rxdataF_comp128_0[i],1),_mm_srai_epi16(rxdataF_comp128_1[i],1));
      ul_ch_mag128_0[i]    = _mm_adds_epi16(_mm_srai_epi16(ul_ch_mag128_0[i],1),_mm_srai_epi16(ul_ch_mag128_1[i],1));
      ul_ch_mag128_0b[i]    = _mm_adds_epi16(_mm_srai_epi16(ul_ch_mag128_0b[i],1),_mm_srai_epi16(ul_ch_mag128_1b[i],1));
    }
    // remove any bias (DC component after IDFT)
    ((u32*)rxdataF_comp128_0)[0]=0;
  }

  _mm_empty();
  _m_empty();

}

void ulsch_extract_rbs_single(int **rxdataF,
			      int **rxdataF_ext,
			      unsigned int first_rb,
			      unsigned int nb_rb,
			      unsigned char l,
			      unsigned char Ns,
			      LTE_DL_FRAME_PARMS *frame_parms) {


  unsigned short nb_rb1,nb_rb2;
  unsigned char aarx;
  int *rxF,*rxF_ext;
  
  //unsigned char symbol = l+Ns*frame_parms->symbols_per_tti/2;
  unsigned char symbol = l+((7-frame_parms->Ncp)*(Ns&1)); ///symbol within sub-frame

  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
    
    nb_rb1 = cmin(cmax((int)(frame_parms->N_RB_UL) - (int)(2*first_rb),(int)0),(int)(2*nb_rb));    // 2 times no. RBs before the DC
    nb_rb2 = 2*nb_rb - nb_rb1;                                   // 2 times no. RBs after the DC
#ifdef DEBUG_ULSCH
    msg("ulsch_extract_rbs_single: 2*nb_rb1 = %d, 2*nb_rb2 = %d\n",nb_rb1,nb_rb2);
#endif

    rxF_ext   = &rxdataF_ext[aarx][(symbol*frame_parms->N_RB_UL*12)*2];
    
    if (nb_rb1) {
      rxF = &rxdataF[aarx][(first_rb*12 + frame_parms->first_carrier_offset + symbol*frame_parms->ofdm_symbol_size)*2];
      memcpy(rxF_ext, rxF, nb_rb1*12*sizeof(int));
      rxF_ext += nb_rb1*12;
    
      if (nb_rb2)  {
#ifdef OFDMA_ULSCH
	rxF = &rxdataF[aarx][(1 + symbol*frame_parms->ofdm_symbol_size)*2];
#else
	rxF = &rxdataF[aarx][(symbol*frame_parms->ofdm_symbol_size)*2];
#endif
	memcpy(rxF_ext, rxF, nb_rb2*12*sizeof(int));
	rxF_ext += nb_rb2*12;
      } 
    }
    else { //there is only data in the second half
#ifdef OFDMA_ULSCH
      rxF = &rxdataF[aarx][(1 + 6*(2*first_rb - frame_parms->N_RB_UL) + symbol*frame_parms->ofdm_symbol_size)*2];
#else
      rxF = &rxdataF[aarx][(6*(2*first_rb - frame_parms->N_RB_UL) + symbol*frame_parms->ofdm_symbol_size)*2];
#endif
      memcpy(rxF_ext, rxF, nb_rb2*12*sizeof(int));
      rxF_ext += nb_rb2*12;
    }
  }

  _mm_empty();
  _m_empty();

}

void ulsch_correct_ext(int **rxdataF_ext,
		       int **rxdataF_ext2,
		       unsigned short symbol,
		       LTE_DL_FRAME_PARMS *frame_parms,
		       unsigned short nb_rb) {

  int i,j,aarx;
  int *rxF_ext2,*rxF_ext;

  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
    rxF_ext2 = &rxdataF_ext2[aarx][symbol*12*frame_parms->N_RB_DL];
    rxF_ext  = &rxdataF_ext[aarx][2*symbol*12*frame_parms->N_RB_DL];

    for (i=0,j=0;i<12*nb_rb;i++,j+=2)
      rxF_ext2[i] = rxF_ext[j]; 
  }
}

__m128i QAM_amp128U,QAM_amp128bU;

void ulsch_channel_compensation(int **rxdataF_ext,
				int **ul_ch_estimates_ext,
				int **ul_ch_mag,
				int **ul_ch_magb,
				int **rxdataF_comp,
				LTE_DL_FRAME_PARMS *frame_parms,
				unsigned char symbol,
				unsigned char Qm,
				unsigned short nb_rb,
				unsigned char output_shift) {
  
  unsigned short rb;
  __m128i *ul_ch128,*ul_ch_mag128,*ul_ch_mag128b,*rxdataF128,*rxdataF_comp128;
  unsigned char aarx;//,symbol_mod;


  //  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;

#ifndef __SSE3__
  zeroU = _mm_xor_si128(zeroU,zeroU);
#endif

  //    printf("comp: symbol %d\n",symbol);

  
  if (Qm == 4)
    QAM_amp128U = _mm_set1_epi16(QAM16_n1);
  else if (Qm == 6) {
    QAM_amp128U  = _mm_set1_epi16(QAM64_n1);
    QAM_amp128bU = _mm_set1_epi16(QAM64_n2);
  }
  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
    
    ul_ch128          = (__m128i *)&ul_ch_estimates_ext[aarx][symbol*frame_parms->N_RB_DL*12];
    ul_ch_mag128      = (__m128i *)&ul_ch_mag[aarx][symbol*frame_parms->N_RB_DL*12];
    ul_ch_mag128b     = (__m128i *)&ul_ch_magb[aarx][symbol*frame_parms->N_RB_DL*12];
    rxdataF128        = (__m128i *)&rxdataF_ext[aarx][symbol*frame_parms->N_RB_DL*12];
    rxdataF_comp128   = (__m128i *)&rxdataF_comp[aarx][symbol*frame_parms->N_RB_DL*12];


    for (rb=0;rb<nb_rb;rb++) {
      //      printf("comp: symbol %d rb %d\n",symbol,rb);
#ifdef OFDMA_ULSCH
      if (Qm>2) {  
	// get channel amplitude if not QPSK

	mmtmpU0 = _mm_madd_epi16(ul_ch128[0],ul_ch128[0]);
	
	mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift);
	
	mmtmpU1 = _mm_madd_epi16(ul_ch128[1],ul_ch128[1]);
	mmtmpU1 = _mm_srai_epi32(mmtmpU1,output_shift);
	mmtmpU0 = _mm_packs_epi32(mmtmpU0,mmtmpU1);
	
	ul_ch_mag128[0] = _mm_unpacklo_epi16(mmtmpU0,mmtmpU0);
	ul_ch_mag128b[0] = ul_ch_mag128[0];
	ul_ch_mag128[0] = _mm_mulhi_epi16(ul_ch_mag128[0],QAM_amp128U);
	ul_ch_mag128[0] = _mm_slli_epi16(ul_ch_mag128[0],2);  // 2 to compensate the scale channel estimate
	ul_ch_mag128[1] = _mm_unpackhi_epi16(mmtmpU0,mmtmpU0);
	ul_ch_mag128b[1] = ul_ch_mag128[1];
	ul_ch_mag128[1] = _mm_mulhi_epi16(ul_ch_mag128[1],QAM_amp128U);
	ul_ch_mag128[1] = _mm_slli_epi16(ul_ch_mag128[1],2);  // 2 to compensate the scale channel estimate
	
	mmtmpU0 = _mm_madd_epi16(ul_ch128[2],ul_ch128[2]);
	mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift);
	mmtmpU1 = _mm_packs_epi32(mmtmpU0,mmtmpU0);
	
	ul_ch_mag128[2] = _mm_unpacklo_epi16(mmtmpU1,mmtmpU1);
	ul_ch_mag128b[2] = ul_ch_mag128[2];
	
	ul_ch_mag128[2] = _mm_mulhi_epi16(ul_ch_mag128[2],QAM_amp128U);
	ul_ch_mag128[2] = _mm_slli_epi16(ul_ch_mag128[2],2); // 2 to compensate the scale channel estimate	  
	
	
	ul_ch_mag128b[0] = _mm_mulhi_epi16(ul_ch_mag128b[0],QAM_amp128bU);
	ul_ch_mag128b[0] = _mm_slli_epi16(ul_ch_mag128b[0],2); // 2 to compensate the scale channel estimate
	
	
	ul_ch_mag128b[1] = _mm_mulhi_epi16(ul_ch_mag128b[1],QAM_amp128bU);
	ul_ch_mag128b[1] = _mm_slli_epi16(ul_ch_mag128b[1],2); // 2 to compensate the scale channel estimate
	
	ul_ch_mag128b[2] = _mm_mulhi_epi16(ul_ch_mag128b[2],QAM_amp128bU);
	ul_ch_mag128b[2] = _mm_slli_epi16(ul_ch_mag128b[2],2);// 2 to compensate the scale channel estimate	   
	
      }
#else

	mmtmpU0 = _mm_madd_epi16(ul_ch128[0],ul_ch128[0]);
	
	mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift-1);
	
	mmtmpU1 = _mm_madd_epi16(ul_ch128[1],ul_ch128[1]);
	mmtmpU1 = _mm_srai_epi32(mmtmpU1,output_shift-1);
	mmtmpU0 = _mm_packs_epi32(mmtmpU0,mmtmpU1);
	
	ul_ch_mag128[0] = _mm_unpacklo_epi16(mmtmpU0,mmtmpU0);
	ul_ch_mag128[1] = _mm_unpackhi_epi16(mmtmpU0,mmtmpU0);
	
	mmtmpU0 = _mm_madd_epi16(ul_ch128[2],ul_ch128[2]);
	mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift-1);
	mmtmpU1 = _mm_packs_epi32(mmtmpU0,mmtmpU0);
	ul_ch_mag128[2] = _mm_unpacklo_epi16(mmtmpU1,mmtmpU1);

	//	printf("comp: symbol %d rb %d => %d,%d,%d\n",symbol,rb,*((short*)&ul_ch_mag128[0]),*((short*)&ul_ch_mag128[1]),*((short*)&ul_ch_mag128[2]));	
#endif          
      // multiply by conjugated channel
      mmtmpU0 = _mm_madd_epi16(ul_ch128[0],rxdataF128[0]);
      //	print_ints("re",&mmtmpU0);
      
      // mmtmpU0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpU1 = _mm_shufflelo_epi16(ul_ch128[0],_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_shufflehi_epi16(mmtmpU1,_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_sign_epi16(mmtmpU1,*(__m128i*)&conjugate[0]);
      //	print_ints("im",&mmtmpU1);
      mmtmpU1 = _mm_madd_epi16(mmtmpU1,rxdataF128[0]);
      // mmtmpU1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift);
      //	print_ints("re(shift)",&mmtmpU0);
      mmtmpU1 = _mm_srai_epi32(mmtmpU1,output_shift);
      //	print_ints("im(shift)",&mmtmpU1);
      mmtmpU2 = _mm_unpacklo_epi32(mmtmpU0,mmtmpU1);
      mmtmpU3 = _mm_unpackhi_epi32(mmtmpU0,mmtmpU1);
      //       	print_ints("c0",&mmtmpU2);
      //	print_ints("c1",&mmtmpU3);
      rxdataF_comp128[0] = _mm_packs_epi32(mmtmpU2,mmtmpU3);
      //      	print_shorts("rx:",rxdataF128[0]);
      //      	print_shorts("ch:",ul_ch128[0]);
      //      	print_shorts("pack:",rxdataF_comp128[0]);
      
      // multiply by conjugated channel
      mmtmpU0 = _mm_madd_epi16(ul_ch128[1],rxdataF128[1]);
      // mmtmpU0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpU1 = _mm_shufflelo_epi16(ul_ch128[1],_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_shufflehi_epi16(mmtmpU1,_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_sign_epi16(mmtmpU1,*(__m128i*)conjugate);
      mmtmpU1 = _mm_madd_epi16(mmtmpU1,rxdataF128[1]);
      // mmtmpU1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift);
      mmtmpU1 = _mm_srai_epi32(mmtmpU1,output_shift);
      mmtmpU2 = _mm_unpacklo_epi32(mmtmpU0,mmtmpU1);
      mmtmpU3 = _mm_unpackhi_epi32(mmtmpU0,mmtmpU1);
      
      rxdataF_comp128[1] = _mm_packs_epi32(mmtmpU2,mmtmpU3);
      //      	print_shorts("rx:",rxdataF128[1]);
      //      	print_shorts("ch:",ul_ch128[1]);
      //      	print_shorts("pack:",rxdataF_comp128[1]);	
      //       multiply by conjugated channel
      mmtmpU0 = _mm_madd_epi16(ul_ch128[2],rxdataF128[2]);
      // mmtmpU0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpU1 = _mm_shufflelo_epi16(ul_ch128[2],_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_shufflehi_epi16(mmtmpU1,_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_sign_epi16(mmtmpU1,*(__m128i*)conjugate);
      mmtmpU1 = _mm_madd_epi16(mmtmpU1,rxdataF128[2]);
      // mmtmpU1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift);
      mmtmpU1 = _mm_srai_epi32(mmtmpU1,output_shift);
      mmtmpU2 = _mm_unpacklo_epi32(mmtmpU0,mmtmpU1);
      mmtmpU3 = _mm_unpackhi_epi32(mmtmpU0,mmtmpU1);
      
      rxdataF_comp128[2] = _mm_packs_epi32(mmtmpU2,mmtmpU3);
      //      	print_shorts("rx:",rxdataF128[2]);
      //      	print_shorts("ch:",ul_ch128[2]);
      //        print_shorts("pack:",rxdataF_comp128[2]);
      
      ul_ch128+=3;
      ul_ch_mag128+=3;
      ul_ch_mag128b+=3;
      rxdataF128+=3;
      rxdataF_comp128+=3;
      
    }
  }


  _mm_empty();
  _m_empty();

}     





__m128i QAM_amp128U_0,QAM_amp128bU_0,QAM_amp128U_1,QAM_amp128bU_1;

void ulsch_channel_compensation_alamouti(int **rxdataF_ext,                 // For Distributed Alamouti Combining
					 int **ul_ch_estimates_ext_0,
					 int **ul_ch_estimates_ext_1,
					 int **ul_ch_mag_0,
					 int **ul_ch_magb_0,
					 int **ul_ch_mag_1,
					 int **ul_ch_magb_1,
					 int **rxdataF_comp_0,
					 int **rxdataF_comp_1,
					 LTE_DL_FRAME_PARMS *frame_parms,
					 unsigned char symbol,
					 unsigned char Qm,
					 unsigned short nb_rb,
					 unsigned char output_shift_0,
					 unsigned char output_shift_1) {
  
  unsigned short rb;
  __m128i *ul_ch128_0,*ul_ch128_1,*ul_ch_mag128_0,*ul_ch_mag128_1,*ul_ch_mag128b_0,*ul_ch_mag128b_1,*rxdataF128,*rxdataF_comp128_0,*rxdataF_comp128_1;
  unsigned char aarx;//,symbol_mod;


  //  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;

#ifndef __SSE3__
  zeroU = _mm_xor_si128(zeroU,zeroU);
#endif

  //    printf("comp: symbol %d\n",symbol);

  
  if (Qm == 4) {  
    QAM_amp128U_0 = _mm_set1_epi16(QAM16_n1);
    QAM_amp128U_1 = _mm_set1_epi16(QAM16_n1);
  }
  else if (Qm == 6) {
    QAM_amp128U_0  = _mm_set1_epi16(QAM64_n1);
    QAM_amp128bU_0 = _mm_set1_epi16(QAM64_n2);

    QAM_amp128U_1  = _mm_set1_epi16(QAM64_n1);
    QAM_amp128bU_1 = _mm_set1_epi16(QAM64_n2);
  }
  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
    
    ul_ch128_0          = (__m128i *)&ul_ch_estimates_ext_0[aarx][symbol*frame_parms->N_RB_DL*12];
    ul_ch_mag128_0      = (__m128i *)&ul_ch_mag_0[aarx][symbol*frame_parms->N_RB_DL*12];
    ul_ch_mag128b_0     = (__m128i *)&ul_ch_magb_0[aarx][symbol*frame_parms->N_RB_DL*12];
    ul_ch128_1          = (__m128i *)&ul_ch_estimates_ext_1[aarx][symbol*frame_parms->N_RB_DL*12];
    ul_ch_mag128_1      = (__m128i *)&ul_ch_mag_1[aarx][symbol*frame_parms->N_RB_DL*12];
    ul_ch_mag128b_1     = (__m128i *)&ul_ch_magb_1[aarx][symbol*frame_parms->N_RB_DL*12];
    rxdataF128        = (__m128i *)&rxdataF_ext[aarx][symbol*frame_parms->N_RB_DL*12];
    rxdataF_comp128_0   = (__m128i *)&rxdataF_comp_0[aarx][symbol*frame_parms->N_RB_DL*12];
    rxdataF_comp128_1   = (__m128i *)&rxdataF_comp_1[aarx][symbol*frame_parms->N_RB_DL*12];


    for (rb=0;rb<nb_rb;rb++) {
      //      printf("comp: symbol %d rb %d\n",symbol,rb);
      if (Qm>2) {  
	// get channel amplitude if not QPSK

	mmtmpU0 = _mm_madd_epi16(ul_ch128_0[0],ul_ch128_0[0]);
	
	mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift_0);
	
	mmtmpU1 = _mm_madd_epi16(ul_ch128_0[1],ul_ch128_0[1]);
	mmtmpU1 = _mm_srai_epi32(mmtmpU1,output_shift_0);
	mmtmpU0 = _mm_packs_epi32(mmtmpU0,mmtmpU1);
	
	ul_ch_mag128_0[0] = _mm_unpacklo_epi16(mmtmpU0,mmtmpU0);
	ul_ch_mag128b_0[0] = ul_ch_mag128_0[0];
	ul_ch_mag128_0[0] = _mm_mulhi_epi16(ul_ch_mag128_0[0],QAM_amp128U_0);
	ul_ch_mag128_0[0] = _mm_slli_epi16(ul_ch_mag128_0[0],2); // 2 to compensate the scale channel estimate
	
	ul_ch_mag128_0[1] = _mm_unpackhi_epi16(mmtmpU0,mmtmpU0);
	ul_ch_mag128b_0[1] = ul_ch_mag128_0[1];
	ul_ch_mag128_0[1] = _mm_mulhi_epi16(ul_ch_mag128_0[1],QAM_amp128U_0);
	ul_ch_mag128_0[1] = _mm_slli_epi16(ul_ch_mag128_0[1],2); // 2 to scale compensate the scale channel estimate
	
	mmtmpU0 = _mm_madd_epi16(ul_ch128_0[2],ul_ch128_0[2]);
	mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift_0);
	mmtmpU1 = _mm_packs_epi32(mmtmpU0,mmtmpU0);
	
	ul_ch_mag128_0[2] = _mm_unpacklo_epi16(mmtmpU1,mmtmpU1);
	ul_ch_mag128b_0[2] = ul_ch_mag128_0[2];
	
	ul_ch_mag128_0[2] = _mm_mulhi_epi16(ul_ch_mag128_0[2],QAM_amp128U_0);
	ul_ch_mag128_0[2] = _mm_slli_epi16(ul_ch_mag128_0[2],2);	//  2 to scale compensate the scale channel estimat
	
	
	ul_ch_mag128b_0[0] = _mm_mulhi_epi16(ul_ch_mag128b_0[0],QAM_amp128bU_0);
	ul_ch_mag128b_0[0] = _mm_slli_epi16(ul_ch_mag128b_0[0],2);  //  2 to scale compensate the scale channel estima
	
	
	ul_ch_mag128b_0[1] = _mm_mulhi_epi16(ul_ch_mag128b_0[1],QAM_amp128bU_0);
	ul_ch_mag128b_0[1] = _mm_slli_epi16(ul_ch_mag128b_0[1],2);   //  2 to scale compensate the scale channel estima
	
	ul_ch_mag128b_0[2] = _mm_mulhi_epi16(ul_ch_mag128b_0[2],QAM_amp128bU_0);
	ul_ch_mag128b_0[2] = _mm_slli_epi16(ul_ch_mag128b_0[2],2);	 //  2 to scale compensate the scale channel estima 
	

	

	mmtmpU0 = _mm_madd_epi16(ul_ch128_1[0],ul_ch128_1[0]);
	
	mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift_1);
	
	mmtmpU1 = _mm_madd_epi16(ul_ch128_1[1],ul_ch128_1[1]);
	mmtmpU1 = _mm_srai_epi32(mmtmpU1,output_shift_1);
	mmtmpU0 = _mm_packs_epi32(mmtmpU0,mmtmpU1);
	
	ul_ch_mag128_1[0] = _mm_unpacklo_epi16(mmtmpU0,mmtmpU0);
	ul_ch_mag128b_1[0] = ul_ch_mag128_1[0];
	ul_ch_mag128_1[0] = _mm_mulhi_epi16(ul_ch_mag128_1[0],QAM_amp128U_1);
	ul_ch_mag128_1[0] = _mm_slli_epi16(ul_ch_mag128_1[0],2); // 2 to compensate the scale channel estimate
	
	ul_ch_mag128_1[1] = _mm_unpackhi_epi16(mmtmpU0,mmtmpU0);
	ul_ch_mag128b_1[1] = ul_ch_mag128_1[1];
	ul_ch_mag128_1[1] = _mm_mulhi_epi16(ul_ch_mag128_1[1],QAM_amp128U_1);
	ul_ch_mag128_1[1] = _mm_slli_epi16(ul_ch_mag128_1[1],2); // 2 to scale compensate the scale channel estimate
	
	mmtmpU0 = _mm_madd_epi16(ul_ch128_1[2],ul_ch128_1[2]);
	mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift_1);
	mmtmpU1 = _mm_packs_epi32(mmtmpU0,mmtmpU0);
	
	ul_ch_mag128_1[2] = _mm_unpacklo_epi16(mmtmpU1,mmtmpU1);
	ul_ch_mag128b_1[2] = ul_ch_mag128_1[2];
	
	ul_ch_mag128_1[2] = _mm_mulhi_epi16(ul_ch_mag128_1[2],QAM_amp128U_0);
	ul_ch_mag128_1[2] = _mm_slli_epi16(ul_ch_mag128_1[2],2);	//  2 to scale compensate the scale channel estimat
	
	
	ul_ch_mag128b_1[0] = _mm_mulhi_epi16(ul_ch_mag128b_1[0],QAM_amp128bU_1);
	ul_ch_mag128b_1[0] = _mm_slli_epi16(ul_ch_mag128b_1[0],2);  //  2 to scale compensate the scale channel estima
	
	
	ul_ch_mag128b_1[1] = _mm_mulhi_epi16(ul_ch_mag128b_1[1],QAM_amp128bU_1);
	ul_ch_mag128b_1[1] = _mm_slli_epi16(ul_ch_mag128b_1[1],2);   //  2 to scale compensate the scale channel estima
	
	ul_ch_mag128b_1[2] = _mm_mulhi_epi16(ul_ch_mag128b_1[2],QAM_amp128bU_1);
	ul_ch_mag128b_1[2] = _mm_slli_epi16(ul_ch_mag128b_1[2],2);	 //  2 to scale compensate the scale channel estima 
      }
      

      /************************For Computing (y)*(h0*)********************************************/

      // multiply by conjugated channel
      mmtmpU0 = _mm_madd_epi16(ul_ch128_0[0],rxdataF128[0]);
      //	print_ints("re",&mmtmpU0);
      
      // mmtmpU0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpU1 = _mm_shufflelo_epi16(ul_ch128_0[0],_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_shufflehi_epi16(mmtmpU1,_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_sign_epi16(mmtmpU1,*(__m128i*)&conjugate[0]);
      //	print_ints("im",&mmtmpU1);
      mmtmpU1 = _mm_madd_epi16(mmtmpU1,rxdataF128[0]);
      // mmtmpU1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift_0);
      //	print_ints("re(shift)",&mmtmpU0);
      mmtmpU1 = _mm_srai_epi32(mmtmpU1,output_shift_0);
      //	print_ints("im(shift)",&mmtmpU1);
      mmtmpU2 = _mm_unpacklo_epi32(mmtmpU0,mmtmpU1);
      mmtmpU3 = _mm_unpackhi_epi32(mmtmpU0,mmtmpU1);
      //       	print_ints("c0",&mmtmpU2);
      //	print_ints("c1",&mmtmpU3);
      rxdataF_comp128_0[0] = _mm_packs_epi32(mmtmpU2,mmtmpU3);
      //      	print_shorts("rx:",rxdataF128[0]);
      //      	print_shorts("ch:",ul_ch128_0[0]);
      //      	print_shorts("pack:",rxdataF_comp128_0[0]);
      
      // multiply by conjugated channel
      mmtmpU0 = _mm_madd_epi16(ul_ch128_0[1],rxdataF128[1]);
      // mmtmpU0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpU1 = _mm_shufflelo_epi16(ul_ch128_0[1],_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_shufflehi_epi16(mmtmpU1,_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_sign_epi16(mmtmpU1,*(__m128i*)conjugate);
      mmtmpU1 = _mm_madd_epi16(mmtmpU1,rxdataF128[1]);
      // mmtmpU1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift_0);
      mmtmpU1 = _mm_srai_epi32(mmtmpU1,output_shift_0);
      mmtmpU2 = _mm_unpacklo_epi32(mmtmpU0,mmtmpU1);
      mmtmpU3 = _mm_unpackhi_epi32(mmtmpU0,mmtmpU1);
      
      rxdataF_comp128_0[1] = _mm_packs_epi32(mmtmpU2,mmtmpU3);
      //      	print_shorts("rx:",rxdataF128[1]);
      //      	print_shorts("ch:",ul_ch128_0[1]);
      //      	print_shorts("pack:",rxdataF_comp128_0[1]);	
      //       multiply by conjugated channel
      mmtmpU0 = _mm_madd_epi16(ul_ch128_0[2],rxdataF128[2]);
      // mmtmpU0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpU1 = _mm_shufflelo_epi16(ul_ch128_0[2],_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_shufflehi_epi16(mmtmpU1,_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_sign_epi16(mmtmpU1,*(__m128i*)conjugate);
      mmtmpU1 = _mm_madd_epi16(mmtmpU1,rxdataF128[2]);
      // mmtmpU1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift_0);
      mmtmpU1 = _mm_srai_epi32(mmtmpU1,output_shift_0);
      mmtmpU2 = _mm_unpacklo_epi32(mmtmpU0,mmtmpU1);
      mmtmpU3 = _mm_unpackhi_epi32(mmtmpU0,mmtmpU1);
      
      rxdataF_comp128_0[2] = _mm_packs_epi32(mmtmpU2,mmtmpU3);
      //      	print_shorts("rx:",rxdataF128[2]);
      //      	print_shorts("ch:",ul_ch128_0[2]);
      //        print_shorts("pack:",rxdataF_comp128_0[2]);
      



      /*************************For Computing (y*)*(h1)************************************/
      // multiply by conjugated signal
      mmtmpU0 = _mm_madd_epi16(ul_ch128_1[0],rxdataF128[0]);
      //	print_ints("re",&mmtmpU0);
      
      // mmtmpU0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpU1 = _mm_shufflelo_epi16(rxdataF128[0],_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_shufflehi_epi16(mmtmpU1,_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_sign_epi16(mmtmpU1,*(__m128i*)&conjugate[0]);
      //	print_ints("im",&mmtmpU1);
      mmtmpU1 = _mm_madd_epi16(mmtmpU1,ul_ch128_1[0]);
      // mmtmpU1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift_1);
      //	print_ints("re(shift)",&mmtmpU0);
      mmtmpU1 = _mm_srai_epi32(mmtmpU1,output_shift_1);
      //	print_ints("im(shift)",&mmtmpU1);
      mmtmpU2 = _mm_unpacklo_epi32(mmtmpU0,mmtmpU1);
      mmtmpU3 = _mm_unpackhi_epi32(mmtmpU0,mmtmpU1);
      //       	print_ints("c0",&mmtmpU2);
      //	print_ints("c1",&mmtmpU3);
      rxdataF_comp128_1[0] = _mm_packs_epi32(mmtmpU2,mmtmpU3);
      //      	print_shorts("rx:",rxdataF128[0]);
      //      	print_shorts("ch_conjugate:",ul_ch128_1[0]);
      //      	print_shorts("pack:",rxdataF_comp128_1[0]);


      // multiply by conjugated signal
      mmtmpU0 = _mm_madd_epi16(ul_ch128_1[1],rxdataF128[1]);
      // mmtmpU0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpU1 = _mm_shufflelo_epi16(rxdataF128[1],_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_shufflehi_epi16(mmtmpU1,_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_sign_epi16(mmtmpU1,*(__m128i*)conjugate);
      mmtmpU1 = _mm_madd_epi16(mmtmpU1,ul_ch128_1[1]);
      // mmtmpU1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift_1);
      mmtmpU1 = _mm_srai_epi32(mmtmpU1,output_shift_1);
      mmtmpU2 = _mm_unpacklo_epi32(mmtmpU0,mmtmpU1);
      mmtmpU3 = _mm_unpackhi_epi32(mmtmpU0,mmtmpU1);
      
      rxdataF_comp128_1[1] = _mm_packs_epi32(mmtmpU2,mmtmpU3);
      //      	print_shorts("rx:",rxdataF128[1]);
      //      	print_shorts("ch_conjugate:",ul_ch128_1[1]);
      //      	print_shorts("pack:",rxdataF_comp128_1[1]);


      //       multiply by conjugated signal
      mmtmpU0 = _mm_madd_epi16(ul_ch128_1[2],rxdataF128[2]);
      // mmtmpU0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpU1 = _mm_shufflelo_epi16(rxdataF128[2],_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_shufflehi_epi16(mmtmpU1,_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_sign_epi16(mmtmpU1,*(__m128i*)conjugate);
      mmtmpU1 = _mm_madd_epi16(mmtmpU1,ul_ch128_1[2]);
      // mmtmpU1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift_1);
      mmtmpU1 = _mm_srai_epi32(mmtmpU1,output_shift_1);
      mmtmpU2 = _mm_unpacklo_epi32(mmtmpU0,mmtmpU1);
      mmtmpU3 = _mm_unpackhi_epi32(mmtmpU0,mmtmpU1);
      
      rxdataF_comp128_1[2] = _mm_packs_epi32(mmtmpU2,mmtmpU3);
      //      	print_shorts("rx:",rxdataF128[2]);
      //      	print_shorts("ch_conjugate:",ul_ch128_0[2]);
      //        print_shorts("pack:",rxdataF_comp128_1[2]);



      ul_ch128_0+=3;
      ul_ch_mag128_0+=3;
      ul_ch_mag128b_0+=3;
      ul_ch128_1+=3;
      ul_ch_mag128_1+=3;
      ul_ch_mag128b_1+=3;
      rxdataF128+=3;
      rxdataF_comp128_0+=3;
      rxdataF_comp128_1+=3;
      
    }
  }


  _mm_empty();
  _m_empty();

}     




void ulsch_alamouti(LTE_DL_FRAME_PARMS *frame_parms,// For Distributed Alamouti Receiver Combining
		    int **rxdataF_comp,
		    int **rxdataF_comp_0,
		    int **rxdataF_comp_1,
		    int **ul_ch_mag,
		    int **ul_ch_magb,
		    int **ul_ch_mag_0,
		    int **ul_ch_magb_0,
		    int **ul_ch_mag_1,
		    int **ul_ch_magb_1,
		    unsigned char symbol,
		    unsigned short nb_rb)   {

  short *rxF,*rxF0,*rxF1;
  __m128i *ch_mag,*ch_magb,*ch_mag0,*ch_mag1,*ch_mag0b,*ch_mag1b;
  unsigned char rb,re,aarx;
  int jj=(symbol*frame_parms->N_RB_DL*12);


  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {

    rxF      = (short*)&rxdataF_comp[aarx][jj];
    rxF0     = (short*)&rxdataF_comp_0[aarx][jj];   // Contains (y)*(h0*)
    rxF1     = (short*)&rxdataF_comp_1[aarx][jj];   // Contains (y*)*(h1)
    ch_mag   = (__m128i *)&ul_ch_mag[aarx][jj];
    ch_mag0 = (__m128i *)&ul_ch_mag_0[aarx][jj];
    ch_mag1 = (__m128i *)&ul_ch_mag_1[aarx][jj];
    ch_magb = (__m128i *)&ul_ch_magb[aarx][jj];
    ch_mag0b = (__m128i *)&ul_ch_magb_0[aarx][jj];
    ch_mag1b = (__m128i *)&ul_ch_magb_1[aarx][jj];
    for (rb=0;rb<nb_rb;rb++) {

      for (re=0;re<12;re+=2) {

	// Alamouti RX combining
      
	rxF[0] = rxF0[0] + rxF1[2];                   // re((y0)*(h0*))+ re((y1*)*(h1)) = re(x0)
	rxF[1] = rxF0[1] + rxF1[3];                   // im((y0)*(h0*))+ im((y1*)*(h1)) = im(x0)

	rxF[2] = rxF0[2] - rxF1[0];                   // re((y1)*(h0*))- re((y0*)*(h1)) = re(x1)
	rxF[3] = rxF0[3] - rxF1[1];                   // im((y1)*(h0*))- im((y0*)*(h1)) = im(x1)
 
	rxF+=4;
	rxF0+=4;
	rxF1+=4;
      }
 
      // compute levels for 16QAM or 64 QAM llr unit
      ch_mag[0] = _mm_adds_epi16(ch_mag0[0],ch_mag1[0]);
      ch_mag[1] = _mm_adds_epi16(ch_mag0[1],ch_mag1[1]);
      ch_mag[2] = _mm_adds_epi16(ch_mag0[2],ch_mag1[2]);
      ch_magb[0] = _mm_adds_epi16(ch_mag0b[0],ch_mag1b[0]);
      ch_magb[1] = _mm_adds_epi16(ch_mag0b[1],ch_mag1b[1]);
      ch_magb[2] = _mm_adds_epi16(ch_mag0b[2],ch_mag1b[2]);

      ch_mag+=3;
      ch_mag0+=3;
      ch_mag1+=3;
      ch_magb+=3;
      ch_mag0b+=3;
      ch_mag1b+=3;
    }
  }

  _mm_empty();
  _m_empty();
  
}





__m128i avg128U;

void ulsch_channel_level(int **drs_ch_estimates_ext,
			 LTE_DL_FRAME_PARMS *frame_parms,
			 int *avg,
			 unsigned short nb_rb){

  short rb;
  unsigned char aarx;
  __m128i *ul_ch128;
  

  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
    //clear average level
    avg128U = _mm_xor_si128(avg128U,avg128U);
    ul_ch128=(__m128i *)drs_ch_estimates_ext[aarx];

    for (rb=0;rb<nb_rb;rb++) {
      
      avg128U = _mm_add_epi32(avg128U,_mm_madd_epi16(ul_ch128[0],ul_ch128[0]));
      avg128U = _mm_add_epi32(avg128U,_mm_madd_epi16(ul_ch128[1],ul_ch128[1]));
      avg128U = _mm_add_epi32(avg128U,_mm_madd_epi16(ul_ch128[2],ul_ch128[2]));
      
      ul_ch128+=3;	
            
      if (rb==0) {
	//	print_shorts("ul_ch128",&ul_ch128[0]);
	//	print_shorts("ul_ch128",&ul_ch128[1]);
	//	print_shorts("ul_ch128",&ul_ch128[2]);
      }
      
    }
    
    avg[aarx] = (((int*)&avg128U)[0] + 
		 ((int*)&avg128U)[1] + 
		 ((int*)&avg128U)[2] + 
		 ((int*)&avg128U)[3])/(nb_rb*12);
    
    //    printf("Channel level : %d\n",avg[aarx]);
  }
  _mm_empty();
  _m_empty();

}

int avgU[2];
int avgU_0[2],avgU_1[2]; // For the Distributed Alamouti Scheme
/* --> moved to LTE_eNB_ULSCH structure
int ulsch_power[2];
int ulsch_power_0[2],ulsch_power_1[2];// For the distributed Alamouti Scheme
*/

void rx_ulsch(LTE_eNB_COMMON *eNB_common_vars,
	      LTE_eNB_ULSCH *eNB_ulsch_vars,
	      LTE_DL_FRAME_PARMS *frame_parms,
	      unsigned int subframe,
	      unsigned char eNB_id,  // this is the effective sector id
	      LTE_eNB_ULSCH_t *ulsch,
	      u8 cooperation_flag) {

  unsigned int l,i;
  int avgs;
  unsigned char log2_maxh,aarx;

 
  int avgs_0,avgs_1;
  unsigned log2_maxh_0,log2_maxh_1;
  

  //  unsigned char harq_pid = ( ulsch->RRCConnRequest_flag== 0) ? subframe2harq_pid_tdd(frame_parms->tdd_config,subframe) : 0;
  unsigned char harq_pid = subframe2harq_pid(frame_parms,subframe);
  unsigned char Qm = get_Qm(ulsch->harq_processes[harq_pid]->mcs);
  unsigned short rx_power_correction;

#ifdef DEBUG_ULSCH
  debug_msg("rx_ulsch: eNB_id %d, harq_pid %d, nb_rb %d first_rb %d, cooperation %d\n",eNB_id,harq_pid,ulsch->harq_processes[harq_pid]->nb_rb,ulsch->harq_processes[harq_pid]->first_rb, cooperation_flag);
#endif //DEBUG_ULSCH

  if ( (frame_parms->ofdm_symbol_size == 128) ||
       (frame_parms->ofdm_symbol_size == 512) )
    rx_power_correction = 2;
  else
    rx_power_correction = 1;

  for (l=0;l<frame_parms->symbols_per_tti-1;l++) {
          
#ifdef DEBUG_ULSCH
    msg("rx_ulsch : symbol %d (first_rb %d,nb_rb %d), rxdataF %p, rxdataF_ext %p\n",l,
	ulsch->harq_processes[harq_pid]->first_rb,
	ulsch->harq_processes[harq_pid]->nb_rb,
	eNB_common_vars->rxdataF[eNB_id],
    	eNB_ulsch_vars->rxdataF_ext[eNB_id]);
#endif //DEBUG_ULSCH

    ulsch_extract_rbs_single(eNB_common_vars->rxdataF[eNB_id],
			     eNB_ulsch_vars->rxdataF_ext[eNB_id],
			     ulsch->harq_processes[harq_pid]->first_rb,
			     ulsch->harq_processes[harq_pid]->nb_rb,
			     l%(frame_parms->symbols_per_tti/2),
			     l/(frame_parms->symbols_per_tti/2),
			     frame_parms);
    
    lte_ul_channel_estimation(eNB_ulsch_vars->drs_ch_estimates[eNB_id],
			      eNB_ulsch_vars->drs_ch_estimates_time[eNB_id],
			      eNB_ulsch_vars->drs_ch_estimates_0[eNB_id],
			      eNB_ulsch_vars->drs_ch_estimates_1[eNB_id],
			      eNB_ulsch_vars->rxdataF_ext[eNB_id],
			      frame_parms,
			      l%(frame_parms->symbols_per_tti/2),
			      l/(frame_parms->symbols_per_tti/2),
			      ulsch->harq_processes[harq_pid]->nb_rb,
			      ulsch->cyclicShift,
			      cooperation_flag);



    ulsch_correct_ext(eNB_ulsch_vars->rxdataF_ext[eNB_id],
		      eNB_ulsch_vars->rxdataF_ext2[eNB_id],
		      l,
		      frame_parms,
		      ulsch->harq_processes[harq_pid]->nb_rb);  
    
    if(cooperation_flag == 2)
      {
	for (i=0;i<frame_parms->nb_antennas_rx;i++){
	  eNB_ulsch_vars->ulsch_power_0[i] = signal_energy_nodc(eNB_ulsch_vars->drs_ch_estimates_0[eNB_id][i],
						ulsch->harq_processes[harq_pid]->nb_rb*12)*rx_power_correction;
	  eNB_ulsch_vars->ulsch_power_1[i] = signal_energy_nodc(eNB_ulsch_vars->drs_ch_estimates_1[eNB_id][i],
						ulsch->harq_processes[harq_pid]->nb_rb*12)*rx_power_correction;
	}
      }
    else
      {
	for (i=0;i<frame_parms->nb_antennas_rx;i++)
	  eNB_ulsch_vars->ulsch_power[i] = signal_energy_nodc(eNB_ulsch_vars->drs_ch_estimates[eNB_id][i],
					      ulsch->harq_processes[harq_pid]->nb_rb*12)*rx_power_correction;
      }
  }


  if(cooperation_flag == 2)
    {
      ulsch_channel_level(eNB_ulsch_vars->drs_ch_estimates_0[eNB_id],
			  frame_parms,
			  avgU_0,
			  ulsch->harq_processes[harq_pid]->nb_rb);
    
      //  msg("[ULSCH] avg_0[0] %d\n",avgU_0[0]);
  

      avgs_0 = 0;
      for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++)
	avgs_0 = cmax(avgs_0,avgU_0[(aarx<<1)]);
  
      log2_maxh_0 = 4+(log2_approx(avgs_0)/2);
#ifdef DEBUG_ULSCH
      msg("[ULSCH] log2_maxh_0 = %d (%d,%d)\n",log2_maxh_0,avgU_0[0],avgs_0);
#endif

      ulsch_channel_level(eNB_ulsch_vars->drs_ch_estimates_1[eNB_id],
			  frame_parms,
			  avgU_1,
			  ulsch->harq_processes[harq_pid]->nb_rb);
    
      //  msg("[ULSCH] avg_1[0] %d\n",avgU_1[0]);
  

      avgs_1 = 0;
      for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++)
	avgs_1 = cmax(avgs_1,avgU_1[(aarx<<1)]);
  
      log2_maxh_1 = 4+(log2_approx(avgs_1)/2);
#ifdef DEBUG_ULSCH
      msg("[ULSCH] log2_maxh_1 = %d (%d,%d)\n",log2_maxh_1,avgU_1[0],avgs_1);
#endif
    }
  else
    {
      ulsch_channel_level(eNB_ulsch_vars->drs_ch_estimates[eNB_id],
			  frame_parms,
			  avgU,
			  ulsch->harq_processes[harq_pid]->nb_rb);
    
      //  msg("[ULSCH] avg[0] %d\n",avgU[0]);
  

      avgs = 0;
      for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++)
	avgs = cmax(avgs,avgU[(aarx<<1)]);
  
      log2_maxh = 2+(log2_approx(avgs)/2);
#ifdef DEBUG_ULSCH
      msg("[ULSCH] log2_maxh = %d (%d,%d)\n",log2_maxh,avgU[0],avgs);
#endif
    }

  for (l=0;l<frame_parms->symbols_per_tti-1;l++) {

    if (((frame_parms->Ncp == 0) && ((l==3) || (l==10)))||   // skip pilots
	((frame_parms->Ncp == 1) && ((l==2) || (l==8)))) {
      l++;
    }    

    if(cooperation_flag == 2)
      {

	ulsch_channel_compensation_alamouti(eNB_ulsch_vars->rxdataF_ext2[eNB_id],
					    eNB_ulsch_vars->drs_ch_estimates_0[eNB_id],
					    eNB_ulsch_vars->drs_ch_estimates_1[eNB_id],
					    eNB_ulsch_vars->ul_ch_mag_0[eNB_id],
					    eNB_ulsch_vars->ul_ch_magb_0[eNB_id],
					    eNB_ulsch_vars->ul_ch_mag_1[eNB_id],
					    eNB_ulsch_vars->ul_ch_magb_1[eNB_id],
					    eNB_ulsch_vars->rxdataF_comp_0[eNB_id],
					    eNB_ulsch_vars->rxdataF_comp_1[eNB_id],
					    frame_parms,
					    l,
					    Qm,
					    ulsch->harq_processes[harq_pid]->nb_rb,
					    log2_maxh_0,
					    log2_maxh_1); // log2_maxh+I0_shift

	ulsch_alamouti(frame_parms,
		       eNB_ulsch_vars->rxdataF_comp[eNB_id],
		       eNB_ulsch_vars->rxdataF_comp_0[eNB_id],
		       eNB_ulsch_vars->rxdataF_comp_1[eNB_id],
		       eNB_ulsch_vars->ul_ch_mag[eNB_id],
		       eNB_ulsch_vars->ul_ch_magb[eNB_id],
		       eNB_ulsch_vars->ul_ch_mag_0[eNB_id],
		       eNB_ulsch_vars->ul_ch_magb_0[eNB_id],
		       eNB_ulsch_vars->ul_ch_mag_1[eNB_id],
		       eNB_ulsch_vars->ul_ch_magb_1[eNB_id],
		       l,
		       ulsch->harq_processes[harq_pid]->nb_rb);
      }
    else
      {
	ulsch_channel_compensation(eNB_ulsch_vars->rxdataF_ext2[eNB_id],
				   eNB_ulsch_vars->drs_ch_estimates[eNB_id],
				   eNB_ulsch_vars->ul_ch_mag[eNB_id],
				   eNB_ulsch_vars->ul_ch_magb[eNB_id],
				   eNB_ulsch_vars->rxdataF_comp[eNB_id],
				   frame_parms,
				   l,
				   Qm,
				   ulsch->harq_processes[harq_pid]->nb_rb,
				   log2_maxh); // log2_maxh+I0_shift
      }
    if (frame_parms->nb_antennas_rx > 1)
      ulsch_detection_mrc(frame_parms,
			  eNB_ulsch_vars->rxdataF_comp[eNB_id],
			  eNB_ulsch_vars->ul_ch_mag[eNB_id],
			  eNB_ulsch_vars->ul_ch_magb[eNB_id],
			  l,
			  ulsch->harq_processes[harq_pid]->nb_rb);
#ifndef OFDMA_ULSCH
    freq_equalization(frame_parms,
		      eNB_ulsch_vars->rxdataF_comp[eNB_id],
		      eNB_ulsch_vars->ul_ch_mag[eNB_id],
		      eNB_ulsch_vars->ul_ch_magb[eNB_id],
		      l,
		      ulsch->harq_processes[harq_pid]->nb_rb*12,
		      Qm);
		      
#endif
  }

#ifndef OFDMA_ULSCH
        
  //#ifdef DEBUG_ULSCH
  // Inverse-Transform equalized outputs
  //  msg("Doing IDFTs\n");
  lte_idft(frame_parms,
	   eNB_ulsch_vars->rxdataF_comp[eNB_id][0],
	   ulsch->harq_processes[harq_pid]->nb_rb*12);
  //  msg("Done\n"); 
  //#endif //DEBUG_ULSCH

#endif

  for (l=0;l<frame_parms->symbols_per_tti-1;l++) {
    
    if (((frame_parms->Ncp == 0) && ((l==3) || (l==10)))||   // skip pilots
	((frame_parms->Ncp == 1) && ((l==2) || (l==8)))) {
      l++;
    }    
    
    switch (Qm) {
    case 2 : 
      ulsch_qpsk_llr(frame_parms,
		     eNB_ulsch_vars->rxdataF_comp[eNB_id],
		     eNB_ulsch_vars->llr,
		     l,
		     ulsch->harq_processes[harq_pid]->nb_rb);
      break;
    case 4 :
      ulsch_16qam_llr(frame_parms,
		      eNB_ulsch_vars->rxdataF_comp[eNB_id],
		      eNB_ulsch_vars->llr,
		      eNB_ulsch_vars->ul_ch_mag[eNB_id],
		      l,ulsch->harq_processes[harq_pid]->nb_rb);
      break;
    case 6 :
      ulsch_64qam_llr(frame_parms,
		      eNB_ulsch_vars->rxdataF_comp[eNB_id],
		      eNB_ulsch_vars->llr,
		      eNB_ulsch_vars->ul_ch_mag[eNB_id],
		      eNB_ulsch_vars->ul_ch_magb[eNB_id],
		      l,ulsch->harq_processes[harq_pid]->nb_rb);
      break;
    default:
#ifdef DEBUG_ULSCH
      msg("ulsch_demodulation.c (rx_ulsch): Unknown Qm!!!!\n");
#endif //DEBUG_ULSCH
      break;
    }
  }

}

void rx_ulsch_emul(PHY_VARS_eNB *phy_vars_eNB,
		   u8 subframe,
		   u8 sect_id,
		   u8 UE_index) {
  msg("[PHY] EMUL eNB %d rx_ulsch_emul : subframe %d, sect_id %d, UE_index %d\n",phy_vars_eNB->Mod_id,subframe,sect_id,UE_index);
  phy_vars_eNB->lte_eNB_ulsch_vars[UE_index]->ulsch_power[0] = 31622; //=45dB;
  phy_vars_eNB->lte_eNB_ulsch_vars[UE_index]->ulsch_power[1] = 31622; //=45dB;

}

#ifdef USER_MODE
void dump_ulsch(PHY_VARS_eNB *PHY_vars_eNB) {

  unsigned int nsymb = (PHY_vars_eNB->lte_frame_parms.Ncp == 0) ? 14 : 12;

  write_output("rxsigF0.m","rxsF0", &PHY_vars_eNB->lte_eNB_common_vars.rxdataF[0][0][0],512*nsymb*2,2,1);
  if (PHY_vars_eNB->lte_frame_parms.nb_antennas_tx>1)
    write_output("rxsigF1.m","rxsF1", &PHY_vars_eNB->lte_eNB_common_vars.rxdataF[0][1][0],512*nsymb*2,2,1);
  write_output("rxsigF0_ext.m","rxsF0_ext", &PHY_vars_eNB->lte_eNB_ulsch_vars[0]->rxdataF_ext[0][0][0],300*nsymb*2,2,1);
  if (PHY_vars_eNB->lte_frame_parms.nb_antennas_rx>1)
    write_output("rxsigF1_ext.m","rxsF1_ext", &PHY_vars_eNB->lte_eNB_ulsch_vars[0]->rxdataF_ext[1][0][0],300*nsymb*2,2,1);
  write_output("srs_est0.m","srsest0",PHY_vars_eNB->lte_eNB_srs_vars[0].srs_ch_estimates[0][0],512,1,1);
  if (PHY_vars_eNB->lte_frame_parms.nb_antennas_rx>1)
    write_output("srs_est1.m","srsest1",PHY_vars_eNB->lte_eNB_srs_vars[0].srs_ch_estimates[0][1],512,1,1);
  write_output("drs_est0.m","drsest0",PHY_vars_eNB->lte_eNB_ulsch_vars[0]->drs_ch_estimates[0][0],300*nsymb,1,1);
  if (PHY_vars_eNB->lte_frame_parms.nb_antennas_rx>1)
    write_output("drs_est1.m","drsest1",PHY_vars_eNB->lte_eNB_ulsch_vars[0]->drs_ch_estimates[0][1],300*nsymb,1,1);
  write_output("ulsch_rxF_comp0.m","ulsch0_rxF_comp0",&PHY_vars_eNB->lte_eNB_ulsch_vars[0]->rxdataF_comp[0][0][0],300*nsymb,1,1);
  write_output("ulsch_rxF_llr.m","ulsch_llr",PHY_vars_eNB->lte_eNB_ulsch_vars[0]->llr,PHY_vars_eNB->ulsch_eNB[0]->harq_processes[0]->nb_rb*12*2*9,1,0);	
  write_output("ulsch_ch_mag.m","ulsch_ch_mag",&PHY_vars_eNB->lte_eNB_ulsch_vars[0]->ul_ch_mag[0][0][0],300*nsymb,1,1);	  
	  
}
#endif
