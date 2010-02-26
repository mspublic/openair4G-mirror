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
#define DEBUG_ULSCH

#ifndef __SSE3__
__m128i zeroU;
#define _mm_abs_epi16(xmmx) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zeroU,(xmmx)))
#define _mm_sign_epi16(xmmx,xmmy) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zeroU,(xmmy)))
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
			 int **dl_ch_mag,
			 int **dl_ch_magb,
			 unsigned char symbol,
			 unsigned short nb_rb) {



  __m128i *rxdataF_comp128_0,*dl_ch_mag128_0,*dl_ch_mag128_0b;
  __m128i *rxdataF_comp128_1,*dl_ch_mag128_1,*dl_ch_mag128_1b;;

  int i;

  if (frame_parms->nb_antennas_rx>1) {
    rxdataF_comp128_0   = (__m128i *)&rxdataF_comp[0][symbol*frame_parms->N_RB_DL*12];  
    rxdataF_comp128_1   = (__m128i *)&rxdataF_comp[1][symbol*frame_parms->N_RB_DL*12];  
    dl_ch_mag128_0      = (__m128i *)&dl_ch_mag[0][symbol*frame_parms->N_RB_DL*12];  
    dl_ch_mag128_1      = (__m128i *)&dl_ch_mag[1][symbol*frame_parms->N_RB_DL*12];  
    dl_ch_mag128_0b     = (__m128i *)&dl_ch_magb[0][symbol*frame_parms->N_RB_DL*12];  
    dl_ch_mag128_1b     = (__m128i *)&dl_ch_magb[1][symbol*frame_parms->N_RB_DL*12];  

    // MRC on each re of rb, both on MF output and magnitude (for 16QAM/64QAM llr computation)
    for (i=0;i<nb_rb*3;i++) {
      rxdataF_comp128_0[i] = _mm_adds_epi16(_mm_srai_epi16(rxdataF_comp128_0[i],1),_mm_srai_epi16(rxdataF_comp128_1[i],1));
      dl_ch_mag128_0[i]    = _mm_adds_epi16(_mm_srai_epi16(dl_ch_mag128_0[i],1),_mm_srai_epi16(dl_ch_mag128_1[i],1));
      dl_ch_mag128_0b[i]    = _mm_adds_epi16(_mm_srai_epi16(dl_ch_mag128_0b[i],1),_mm_srai_epi16(dl_ch_mag128_1b[i],1));
    }
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
    
    nb_rb1 = min(max((int)(frame_parms->N_RB_UL) - (int)(2*first_rb),0),2*nb_rb);    // 2 times no. RBs before the DC
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
	rxF = &rxdataF[aarx][(1 + symbol*frame_parms->ofdm_symbol_size)*2];
	memcpy(rxF_ext, rxF, nb_rb2*12*sizeof(int));
	rxF_ext += nb_rb2*12;
      } 
    }
    else { //there is only data in the second half
      rxF = &rxdataF[aarx][(1 + 6*(2*first_rb - frame_parms->N_RB_UL) + symbol*frame_parms->ofdm_symbol_size)*2];
      memcpy(rxF_ext, rxF, nb_rb2*12*sizeof(int));
      rxF_ext += nb_rb2*12;
    }
  }

  _mm_empty();
  _m_empty();

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
  unsigned char aarx,symbol_mod;


  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;

#ifndef __SSE3__
  zeroU = _mm_xor_si128(zeroU,zeroU);
#endif

  //  printf("comp: symbol %d\n",symbol);

  
  if (Qm == 4)
    QAM_amp128U = _mm_set1_epi16(QAM16_n1);
  else if (Qm == 6) {
    QAM_amp128U  = _mm_set1_epi16(QAM64_n1);
    QAM_amp128bU = _mm_set1_epi16(QAM64_n2);
  }
  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
    
    ul_ch128          = (__m128i *)&ul_ch_estimates_ext[aarx][symbol_mod*frame_parms->N_RB_DL*12];
    ul_ch_mag128      = (__m128i *)&ul_ch_mag[aarx][symbol*frame_parms->N_RB_DL*12];
    ul_ch_mag128b     = (__m128i *)&ul_ch_magb[aarx][symbol*frame_parms->N_RB_DL*12];
    rxdataF128        = (__m128i *)&rxdataF_ext[aarx][symbol*frame_parms->N_RB_DL*12];
    rxdataF_comp128   = (__m128i *)&rxdataF_comp[aarx][symbol*frame_parms->N_RB_DL*12];


    for (rb=0;rb<nb_rb;rb++) {
      //	printf("comp: rb %d\n",rb);
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
	ul_ch_mag128[0] = _mm_slli_epi16(ul_ch_mag128[0],1);
	
	ul_ch_mag128[1] = _mm_unpackhi_epi16(mmtmpU0,mmtmpU0);
	ul_ch_mag128b[1] = ul_ch_mag128[1];
	ul_ch_mag128[1] = _mm_mulhi_epi16(ul_ch_mag128[1],QAM_amp128U);
	ul_ch_mag128[1] = _mm_slli_epi16(ul_ch_mag128[1],1);
	
	mmtmpU0 = _mm_madd_epi16(ul_ch128[2],ul_ch128[2]);
	mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift);
	mmtmpU1 = _mm_packs_epi32(mmtmpU0,mmtmpU0);
	
	ul_ch_mag128[2] = _mm_unpacklo_epi16(mmtmpU1,mmtmpU1);
	ul_ch_mag128b[2] = ul_ch_mag128[2];
	
	ul_ch_mag128[2] = _mm_mulhi_epi16(ul_ch_mag128[2],QAM_amp128U);
	ul_ch_mag128[2] = _mm_slli_epi16(ul_ch_mag128[2],1);	  
	
	
	ul_ch_mag128b[0] = _mm_mulhi_epi16(ul_ch_mag128b[0],QAM_amp128bU);
	ul_ch_mag128b[0] = _mm_slli_epi16(ul_ch_mag128b[0],1);
	
	
	ul_ch_mag128b[1] = _mm_mulhi_epi16(ul_ch_mag128b[1],QAM_amp128bU);
	ul_ch_mag128b[1] = _mm_slli_epi16(ul_ch_mag128b[1],1);
	
	ul_ch_mag128b[2] = _mm_mulhi_epi16(ul_ch_mag128b[2],QAM_amp128bU);
	ul_ch_mag128b[2] = _mm_slli_epi16(ul_ch_mag128b[2],1);	  
	
      }
      
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
      //	print_shorts("rx:",rxdataF128);
      //	print_shorts("ch:",ul_ch128);
      //	print_shorts("pack:",rxdataF_comp128);
      
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
      //	print_shorts("rx:",rxdataF128+1);
      //	print_shorts("ch:",ul_ch128+1);
      //	print_shorts("pack:",rxdataF_comp128+1);	
      // multiply by conjugated channel
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
      //	print_shorts("rx:",rxdataF128+2);
      //	print_shorts("ch:",ul_ch128+2);
      //      	print_shorts("pack:",rxdataF_comp128+2);
      
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
    ul_ch128=(__m128i *)&drs_ch_estimates_ext[aarx][0];

      for (rb=0;rb<nb_rb;rb++) {
    
	avg128U = _mm_add_epi32(avg128U,_mm_madd_epi16(ul_ch128[0],ul_ch128[0]));
	avg128U = _mm_add_epi32(avg128U,_mm_madd_epi16(ul_ch128[1],ul_ch128[1]));
	avg128U = _mm_add_epi32(avg128U,_mm_madd_epi16(ul_ch128[2],ul_ch128[2]));

	ul_ch128+=3;	
	/*
	  if (rb==0) {
	  print_shorts("dl_ch128",&dl_ch128[0]);
	  print_shorts("dl_ch128",&dl_ch128[1]);
	  print_shorts("dl_ch128",&dl_ch128[2]);
	  }
	*/
      }

      avg[aarx] = (((int*)&avg128U)[0] + 
		   ((int*)&avg128U)[1] + 
		   ((int*)&avg128U)[2] + 
		   ((int*)&avg128U)[3])/(nb_rb*12);

      //            printf("Channel level : %d\n",avg[(aatx<<1)+aarx]);
  }
  _mm_empty();
  _m_empty();

}
int avgU[2];

int rx_ulsch(LTE_eNB_ULSCH **lte_eNB_ulsch_vars,
	     LTE_DL_FRAME_PARMS *frame_parms,
	     unsigned int subframe_offset,
	     unsigned char eNb_id,  // this is the effective sector id
	     unsigned char UE_id,   // this is the UE instance to act upon
	     LTE_eNb_ULSCH_t *ulsch) {

  unsigned int l;
  int avgs;
  unsigned char log2_maxh,aarx;

  unsigned int subframe = (unsigned char)(subframe_offset/frame_parms->samples_per_tti);
  unsigned char harq_pid = subframe2harq_pid_tdd_eNBrx(frame_parms->tdd_config,subframe);
  unsigned char Qm = get_Qm(ulsch->harq_processes[harq_pid]->mcs);

  for (l=0;l<lte_frame_parms->symbols_per_tti;l++) {
      
    //printf("subframe_offset = %d\n",subframe_offset);
    
    slot_fep_ul(lte_frame_parms,
		lte_eNB_common_vars,
		l%(lte_frame_parms->symbols_per_tti/2),
		l/(lte_frame_parms->symbols_per_tti/2),
		subframe_offset,
		0);
    
    ulsch_extract_rbs_single(lte_eNB_common_vars->rxdataF[eNb_id],
			     lte_eNB_ulsch_vars[UE_id]->rxdataF_ext[eNb_id],
			     ulsch->first_rb,
			     ulsch->nb_rb,
			     l%(lte_frame_parms->symbols_per_tti/2),
			     l/(lte_frame_parms->symbols_per_tti/2),
			     lte_frame_parms);
    
    lte_ul_channel_estimation(lte_eNB_ulsch_vars[UE_id]->drs_ch_estimates[eNb_id],
			      lte_eNB_ulsch_vars[UE_id]->rxdataF_ext[eNb_id],
			      lte_frame_parms,
			      l%(lte_frame_parms->symbols_per_tti/2),
			      l/(lte_frame_parms->symbols_per_tti/2),
			      ulsch->nb_rb);
  
  

    if (l==0) {
      ulsch_channel_level(lte_eNB_ulsch_vars[UE_id]->drs_ch_estimates[eNb_id],
			  frame_parms,
			  avgU,
			  ulsch->nb_rb);
    
  //  msg("[ULSCH] avg[0] %d\n",avg[0]);
  

      avgs = 0;
      for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++)
	avgs = max(avgs,avgU[(aarx<<1)]);
      
      log2_maxh = 4+(log2_approx(avgs)/2);
#ifdef DEBUG_PHY
      msg("[ULSCH] log2_maxh = %d (%d,%d)\n",log2_maxh,avgU[0],avgs);
#endif
    }

    ulsch_channel_compensation(lte_eNB_ulsch_vars[UE_id]->rxdataF_ext[eNb_id],
			       lte_eNB_ulsch_vars[UE_id]->drs_ch_estimates[eNb_id],
			       lte_eNB_ulsch_vars[UE_id]->ul_ch_mag[eNb_id],
			       lte_eNB_ulsch_vars[UE_id]->ul_ch_magb[eNb_id],
			       lte_eNB_ulsch_vars[UE_id]->rxdataF_comp[eNb_id],
			       frame_parms,
			       l,
			       Qm,
			       ulsch->nb_rb,
			       log2_maxh); // log2_maxh+I0_shift

    if (frame_parms->nb_antennas_rx > 1)
      ulsch_detection_mrc(frame_parms,
			  lte_eNB_ulsch_vars[UE_id]->rxdataF_comp[eNb_id],
			  lte_eNB_ulsch_vars[UE_id]->ul_ch_mag[eNb_id],
			  lte_eNB_ulsch_vars[UE_id]->ul_ch_magb[eNb_id],
			  l,
			  ulsch->nb_rb);
    
    switch (Qm) {
    case 2 : 
      ulsch_qpsk_llr(frame_parms,
		     lte_eNB_ulsch_vars[UE_id]->rxdataF_comp[eNb_id],
		     lte_eNB_ulsch_vars[UE_id]->llr,
		     l,
		     ulsch->nb_rb);
      break;
    case 4 :
      dlsch_16qam_llr(frame_parms,
		      lte_eNB_ulsch_vars[UE_id]->rxdataF_comp[eNb_id],
		      lte_eNB_ulsch_vars[UE_id]->llr,
		      lte_eNB_ulsch_vars[UE_id]->ul_ch_mag[eNb_id],
		      l,ulsch->nb_rb);
      break;
    case 6 :
      dlsch_64qam_llr(frame_parms,
		      lte_eNB_ulsch_vars[UE_id]->rxdataF_comp[eNb_id],
		      lte_eNB_ulsch_vars[UE_id]->llr,
		      lte_eNB_ulsch_vars[UE_id]->ul_ch_mag[eNb_id],
		      lte_eNB_ulsch_vars[UE_id]->ul_ch_magb[eNb_id],
		      l,ulsch->nb_rb);
      break;
    default:
      msg("ulsch_demodulation.c (rx_ulsch): Unknown Qm!!!!\n");
      return(-1);
      break;
    }
  }
}
