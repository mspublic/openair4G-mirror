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
#include "PHY/defs.h"
#include "PHY/extern.h"
#include <emmintrin.h>
#include <xmmintrin.h>
//#define DEBUG_CH

#ifndef __SSE3__
__m128i zeroE;//,tmp_over_sqrt_10,tmp_sum_4_over_sqrt_10,tmp_sign,tmp_sign_3_over_sqrt_10;
//#define _mm_abs_epi16(xmmx) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zero,(xmmx)))
#define _mm_abs_epi16(xmmx) _mm_add_epi16(_mm_xor_si128((xmmx),_mm_cmpgt_epi16(zeroE,(xmmx))),_mm_srli_epi16(_mm_cmpgt_epi16(zeroE,(xmmx)),15))
#define _mm_sign_epi16(xmmx,xmmy) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zeroE,(xmmy)))
#endif

// For Channel Estimation in Distributed Alamouti Scheme
//static int16_t temp_out_ifft[2048*4] __attribute__((aligned(16)));
static int16_t temp_out_fft_0[2048*4] __attribute__((aligned(16)));
static int16_t temp_out_fft_1[2048*4] __attribute__((aligned(16)));
static int16_t temp_out_ifft_0[2048*4] __attribute__((aligned(16)));
static int16_t temp_out_ifft_1[2048*4] __attribute__((aligned(16)));


static int32_t temp_in_ifft_0[2048*2] __attribute__((aligned(16)));
static int32_t temp_in_ifft_1[2048*2] __attribute__((aligned(16)));
static int32_t temp_in_fft_0[2048*2] __attribute__((aligned(16)));
static int32_t temp_in_fft_1[2048*2] __attribute__((aligned(16)));

// round(exp(sqrt(-1)*(pi/2)*[0:1:N-1]/N)*pow2(15))
static int16_t ru_90[2*128] = {32767, 0,32766, 402,32758, 804,32746, 1206,32729, 1608,32706, 2009,32679, 2411,32647, 2811,32610, 3212,32568, 3612,32522, 4011,32470, 4410,32413, 4808,32352, 5205,32286, 5602,32214, 5998,32138, 6393,32058, 6787,31972, 7180,31881, 7571,31786, 7962,31686, 8351,31581, 8740,31471, 9127,31357, 9512,31238, 9896,31114, 10279,30986, 10660,30853, 11039,30715, 11417,30572, 11793,30425, 12167,30274, 12540,30118, 12910,29957, 13279,29792, 13646,29622, 14010,29448, 14373,29269, 14733,29086, 15091,28899, 15447,28707, 15800,28511, 16151,28311, 16500,28106, 16846,27897, 17190,27684, 17531,27467, 17869,27246, 18205,27020, 18538,26791, 18868,26557, 19195,26320, 19520,26078, 19841,25833, 20160,25583, 20475,25330, 20788,25073, 21097,24812, 21403,24548, 21706,24279, 22006,24008, 22302,23732, 22595,23453, 22884,23170, 23170,22884, 23453,22595, 23732,22302, 24008,22006, 24279,21706, 24548,21403, 24812,21097, 25073,20788, 25330,20475, 25583,20160, 25833,19841, 26078,19520, 26320,19195, 26557,18868, 26791,18538, 27020,18205, 27246,17869, 27467,17531, 27684,17190, 27897,16846, 28106,16500, 28311,16151, 28511,15800, 28707,15447, 28899,15091, 29086,14733, 29269,14373, 29448,14010, 29622,13646, 29792,13279, 29957,12910, 30118,12540, 30274,12167, 30425,11793, 30572,11417, 30715,11039, 30853,10660, 30986,10279, 31114,9896, 31238,9512, 31357,9127, 31471,8740, 31581,8351, 31686,7962, 31786,7571, 31881,7180, 31972,6787, 32058,6393, 32138,5998, 32214,5602, 32286,5205, 32352,4808, 32413,4410, 32470,4011, 32522,3612, 32568,3212, 32610,2811, 32647,2411, 32679,2009, 32706,1608, 32729,1206, 32746,804, 32758,402, 32766};

static int16_t ru_90c[2*128] = {32767, 0,32766, -402,32758, -804,32746, -1206,32729, -1608,32706, -2009,32679, -2411,32647, -2811,32610, -3212,32568, -3612,32522, -4011,32470, -4410,32413, -4808,32352, -5205,32286, -5602,32214, -5998,32138, -6393,32058, -6787,31972, -7180,31881, -7571,31786, -7962,31686, -8351,31581, -8740,31471, -9127,31357, -9512,31238, -9896,31114, -10279,30986, -10660,30853, -11039,30715, -11417,30572, -11793,30425, -12167,30274, -12540,30118, -12910,29957, -13279,29792, -13646,29622, -14010,29448, -14373,29269, -14733,29086, -15091,28899, -15447,28707, -15800,28511, -16151,28311, -16500,28106, -16846,27897, -17190,27684, -17531,27467, -17869,27246, -18205,27020, -18538,26791, -18868,26557, -19195,26320, -19520,26078, -19841,25833, -20160,25583, -20475,25330, -20788,25073, -21097,24812, -21403,24548, -21706,24279, -22006,24008, -22302,23732, -22595,23453, -22884,23170, -23170,22884, -23453,22595, -23732,22302, -24008,22006, -24279,21706, -24548,21403, -24812,21097, -25073,20788, -25330,20475, -25583,20160, -25833,19841, -26078,19520, -26320,19195, -26557,18868, -26791,18538, -27020,18205, -27246,17869, -27467,17531, -27684,17190, -27897,16846, -28106,16500, -28311,16151, -28511,15800, -28707,15447, -28899,15091, -29086,14733, -29269,14373, -29448,14010, -29622,13646, -29792,13279, -29957,12910, -30118,12540, -30274,12167, -30425,11793, -30572,11417, -30715,11039, -30853,10660, -30986,10279, -31114,9896, -31238,9512, -31357,9127, -31471,8740, -31581,8351, -31686,7962, -31786,7571, -31881,7180, -31972,6787, -32058,6393, -32138,5998, -32214,5602, -32286,5205, -32352,4808, -32413,4410, -32470,4011, -32522,3612, -32568,3212, -32610,2811, -32647,2411, -32679,2009, -32706,1608, -32729,1206, -32746,804, -32758,402, -32766};

#define SCALE 0x3FFF

int32_t lte_ul_channel_estimation(PHY_VARS_eNB *phy_vars_eNB,
			      uint8_t eNB_id,
			      uint8_t UE_id,
			      uint8_t sched_subframe,
			      unsigned char l,
			      unsigned char Ns,
			      uint8_t cooperation_flag) {

  LTE_DL_FRAME_PARMS *frame_parms = &phy_vars_eNB->lte_frame_parms;
  LTE_eNB_PUSCH *eNB_pusch_vars = phy_vars_eNB->lte_eNB_pusch_vars[UE_id];
  int32_t **ul_ch_estimates=eNB_pusch_vars->drs_ch_estimates[eNB_id];
  int32_t **ul_ch_estimates_time=  eNB_pusch_vars->drs_ch_estimates_time[eNB_id];
  int32_t **ul_ch_estimates_0=  eNB_pusch_vars->drs_ch_estimates_0[eNB_id];
  int32_t **ul_ch_estimates_1=  eNB_pusch_vars->drs_ch_estimates_1[eNB_id];
  int32_t **rxdataF_ext=  eNB_pusch_vars->rxdataF_ext[eNB_id];
  int subframe = phy_vars_eNB->proc[sched_subframe].subframe_rx;
  uint8_t harq_pid = subframe2harq_pid(frame_parms,phy_vars_eNB->proc[sched_subframe].frame_rx,subframe);
  int16_t delta_phase = 0;
  int16_t *ru1 = ru_90;
  int16_t *ru2 = ru_90;
  int16_t current_phase1,current_phase2;
  uint16_t N_rb_alloc = phy_vars_eNB->ulsch_eNB[UE_id]->harq_processes[harq_pid]->nb_rb;
  uint16_t aa,Msc_RS,Msc_RS_idx;
  uint16_t * Msc_idx_ptr;
  int k,pilot_pos1 = 3 - frame_parms->Ncp, pilot_pos2 = 10 - 2*frame_parms->Ncp;
  int16_t alpha, beta;
  int32_t *ul_ch1=NULL, *ul_ch2=NULL;
  int32_t *ul_ch1_0=NULL,*ul_ch2_0=NULL,*ul_ch1_1=NULL,*ul_ch2_1=NULL;
  int16_t ul_ch_estimates_re,ul_ch_estimates_im;
  int32_t rx_power_correction;

  //uint8_t nb_antennas_rx = frame_parms->nb_antennas_tx_eNB;
  uint8_t nb_antennas_rx = frame_parms->nb_antennas_rx;
  uint8_t cyclic_shift; 

  uint32_t alpha_ind;
  uint32_t u=frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.grouphop[Ns+(subframe<<1)];
  uint32_t v=frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.seqhop[Ns+(subframe<<1)];
  int32_t tmp_estimates[N_rb_alloc*12] __attribute__((aligned(16)));

  int symbol_offset,i,j;

  //debug_msg("lte_ul_channel_estimation: cyclic shift %d\n",cyclicShift);


  int16_t alpha_re[12] = {32767, 28377, 16383,     0,-16384,  -28378,-32768,-28378,-16384,    -1, 16383, 28377};
  int16_t alpha_im[12] = {0,     16383, 28377, 32767, 28377,   16383,     0,-16384,-28378,-32768,-28378,-16384};

  int32_t *in_fft_ptr_0 = (int32_t*)0,*in_fft_ptr_1 = (int32_t*)0,
    *temp_out_fft_0_ptr = (int32_t*)0,*out_fft_ptr_0 = (int32_t*)0,
    *temp_out_fft_1_ptr = (int32_t*)0,*out_fft_ptr_1 = (int32_t*)0,
    *temp_in_ifft_ptr = (int32_t*)0;

#ifdef NEW_FFT
  __m128i *rxdataF128,*ul_ref128,*ul_ch128;
  __m128i mmtmpU0,mmtmpU1,mmtmpU2,mmtmpU3;
 #endif

  Msc_RS = N_rb_alloc*12;

  cyclic_shift = (frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.cyclicShift +
		  phy_vars_eNB->ulsch_eNB[UE_id]->harq_processes[harq_pid]->n_DMRS2 +
		  frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.nPRS[(subframe<<1)+Ns]) % 12;

#if defined(USER_MODE) 
  Msc_idx_ptr = (uint16_t*) bsearch(&Msc_RS, dftsizes, 33, sizeof(uint16_t), compareints);
  if (Msc_idx_ptr)
    Msc_RS_idx = Msc_idx_ptr - dftsizes;
  else {
    msg("lte_ul_channel_estimation: index for Msc_RS=%d not found\n",Msc_RS);
    return(-1);
  }
#else
  uint8_t b;
  for (b=0;b<33;b++) 
    if (Msc_RS==dftsizes[b])
      Msc_RS_idx = b;
#endif

  //  LOG_I(PHY,"subframe %d, Ns %d, l %d, Msc_RS = %d, Msc_RS_idx = %d, u %d, v %d, cyclic_shift %d\n",subframe,Ns,l,Msc_RS, Msc_RS_idx,u,v,cyclic_shift);
#ifdef DEBUG_CH

#ifdef USER_MODE
  if (Ns==0)
    write_output("drs_seq0.m","drsseq0",ul_ref_sigs_rx[u][v][Msc_RS_idx],2*Msc_RS,2,1);
  else
    write_output("drs_seq1.m","drsseq1",ul_ref_sigs_rx[u][v][Msc_RS_idx],2*Msc_RS,2,1);
#endif
#endif

#ifndef NEW_FFT
  if ( (frame_parms->ofdm_symbol_size == 128) ||
       (frame_parms->ofdm_symbol_size == 512) )
    rx_power_correction = 2;
  else
    rx_power_correction = 1;
#else
  rx_power_correction = 1;
#endif

  if (l == (3 - frame_parms->Ncp)) {

    symbol_offset = frame_parms->N_RB_UL*12*(l+((7-frame_parms->Ncp)*(Ns&1)));

    for (aa=0; aa<nb_antennas_rx; aa++){
      //           msg("Componentwise prod aa %d, symbol_offset %d,ul_ch_estimates %p,ul_ch_estimates[aa] %p,ul_ref_sigs_rx[0][0][Msc_RS_idx] %p\n",aa,symbol_offset,ul_ch_estimates,ul_ch_estimates[aa],ul_ref_sigs_rx[0][0][Msc_RS_idx]);

#ifndef NEW_FFT
      mult_cpx_vector_norep2((int16_t*) &rxdataF_ext[aa][symbol_offset<<1],
			     (int16_t*) ul_ref_sigs_rx[u][v][Msc_RS_idx],
			     (int16_t*) &ul_ch_estimates[aa][symbol_offset],
			     Msc_RS,
			     15);
#else
      rxdataF128 = (__m128i *)&rxdataF_ext[aa][symbol_offset];
      ul_ch128   = (__m128i *)&ul_ch_estimates[aa][symbol_offset];
      ul_ref128  = (__m128i *)ul_ref_sigs_rx[u][v][Msc_RS_idx];

      for (i=0;i<Msc_RS/12;i++) {
      // multiply by conjugated channel
	mmtmpU0 = _mm_madd_epi16(ul_ref128[0],rxdataF128[0]);
	// mmtmpU0 contains real part of 4 consecutive outputs (32-bit)
	mmtmpU1 = _mm_shufflelo_epi16(ul_ref128[0],_MM_SHUFFLE(2,3,0,1));
	mmtmpU1 = _mm_shufflehi_epi16(mmtmpU1,_MM_SHUFFLE(2,3,0,1));
	mmtmpU1 = _mm_sign_epi16(mmtmpU1,*(__m128i*)&conjugate[0]);
	mmtmpU1 = _mm_madd_epi16(mmtmpU1,rxdataF128[0]);
	// mmtmpU1 contains imag part of 4 consecutive outputs (32-bit)
	mmtmpU0 = _mm_srai_epi32(mmtmpU0,15);
	mmtmpU1 = _mm_srai_epi32(mmtmpU1,15);
	mmtmpU2 = _mm_unpacklo_epi32(mmtmpU0,mmtmpU1);
	mmtmpU3 = _mm_unpackhi_epi32(mmtmpU0,mmtmpU1);

	ul_ch128[0] = _mm_packs_epi32(mmtmpU2,mmtmpU3);
	//	printf("rb %d ch: %d %d\n",i,((int16_t*)ul_ch128)[0],((int16_t*)ul_ch128)[1]);
	// multiply by conjugated channel
	mmtmpU0 = _mm_madd_epi16(ul_ref128[1],rxdataF128[1]);
	// mmtmpU0 contains real part of 4 consecutive outputs (32-bit)
	mmtmpU1 = _mm_shufflelo_epi16(ul_ref128[1],_MM_SHUFFLE(2,3,0,1));
	mmtmpU1 = _mm_shufflehi_epi16(mmtmpU1,_MM_SHUFFLE(2,3,0,1));
	mmtmpU1 = _mm_sign_epi16(mmtmpU1,*(__m128i*)conjugate);
	mmtmpU1 = _mm_madd_epi16(mmtmpU1,rxdataF128[1]);
	// mmtmpU1 contains imag part of 4 consecutive outputs (32-bit)
	mmtmpU0 = _mm_srai_epi32(mmtmpU0,15);
	mmtmpU1 = _mm_srai_epi32(mmtmpU1,15);
	mmtmpU2 = _mm_unpacklo_epi32(mmtmpU0,mmtmpU1);
	mmtmpU3 = _mm_unpackhi_epi32(mmtmpU0,mmtmpU1);
	
	ul_ch128[1] = _mm_packs_epi32(mmtmpU2,mmtmpU3);

	mmtmpU0 = _mm_madd_epi16(ul_ref128[2],rxdataF128[2]);
	// mmtmpU0 contains real part of 4 consecutive outputs (32-bit)
	mmtmpU1 = _mm_shufflelo_epi16(ul_ref128[2],_MM_SHUFFLE(2,3,0,1));
	mmtmpU1 = _mm_shufflehi_epi16(mmtmpU1,_MM_SHUFFLE(2,3,0,1));
	mmtmpU1 = _mm_sign_epi16(mmtmpU1,*(__m128i*)conjugate);
	mmtmpU1 = _mm_madd_epi16(mmtmpU1,rxdataF128[2]);
	// mmtmpU1 contains imag part of 4 consecutive outputs (32-bit)
	mmtmpU0 = _mm_srai_epi32(mmtmpU0,15);
	mmtmpU1 = _mm_srai_epi32(mmtmpU1,15);
	mmtmpU2 = _mm_unpacklo_epi32(mmtmpU0,mmtmpU1);
	mmtmpU3 = _mm_unpackhi_epi32(mmtmpU0,mmtmpU1);
	
	ul_ch128[2] = _mm_packs_epi32(mmtmpU2,mmtmpU3);
	
	ul_ch128+=3;
	ul_ref128+=3;
	rxdataF128+=3;
      }
#endif

      alpha_ind = 0;
      if((cyclic_shift != 0)){
	// Compensating for the phase shift introduced at the transmitte
#ifdef DEBUG_CH	
	write_output("drs_est_pre.m","drsest_pre",ul_ch_estimates[0],300*12,1,1);
#endif
	for(i=symbol_offset;i<symbol_offset+Msc_RS;i++){
	  ul_ch_estimates_re = ((int16_t*) ul_ch_estimates[aa])[i<<1];
	  ul_ch_estimates_im = ((int16_t*) ul_ch_estimates[aa])[(i<<1)+1];
	  //	  ((int16_t*) ul_ch_estimates[aa])[i<<1] =  (i%2 == 1? 1:-1) * ul_ch_estimates_re;
	  ((int16_t*) ul_ch_estimates[aa])[i<<1] =  
	    (int16_t) (((int32_t) (alpha_re[alpha_ind]) * (int32_t) (ul_ch_estimates_re) + 
		    (int32_t) (alpha_im[alpha_ind]) * (int32_t) (ul_ch_estimates_im))>>15);
	  
	  //((int16_t*) ul_ch_estimates[aa])[(i<<1)+1] =  (i%2 == 1? 1:-1) * ul_ch_estimates_im;
	  ((int16_t*) ul_ch_estimates[aa])[(i<<1)+1] = 
	    (int16_t) (((int32_t) (alpha_re[alpha_ind]) * (int32_t) (ul_ch_estimates_im) -  
		    (int32_t) (alpha_im[alpha_ind]) * (int32_t) (ul_ch_estimates_re))>>15);

	  alpha_ind+=cyclic_shift;
	  if (alpha_ind>11)
	    alpha_ind-=12;
	}
#ifdef DEBUG_CH
	write_output("drs_est_post.m","drsest_post",ul_ch_estimates[0],300*12,1,1);
#endif
      }

      //copy MIMO channel estimates to temporary buffer for EMOS
      //memcpy(&ul_ch_estimates_0[aa][symbol_offset],&ul_ch_estimates[aa][symbol_offset],frame_parms->ofdm_symbol_size*sizeof(int32_t)*2);

      memset(temp_in_ifft_0,0,frame_parms->ofdm_symbol_size*sizeof(int32_t)*2);   
      // Convert to time domain for visualization
      for(i=0;i<Msc_RS;i++)
	((int32_t*)temp_in_ifft_0)[i] = ul_ch_estimates[aa][symbol_offset+i];
	    
      fft( (int16_t*) temp_in_ifft_0,                          
	   (int16_t*) ul_ch_estimates_time[aa],
	   frame_parms->twiddle_ifft,
	   frame_parms->rev,
	   (frame_parms->log2_symbol_size),
	   (frame_parms->log2_symbol_size)/2,
	   0);

      /*
      // zero out second half of time domain channel estimate and transform back (-16 because of the cyclic pre-causal part of the channel estimate)
      for(j=frame_parms->ofdm_symbol_size-16;j<frame_parms->ofdm_symbol_size*2-16;j++){
	ul_ch_estimates_time[aa][j] = 0;
      }

      fft( (int16_t*) ul_ch_estimates_time[aa],
	   (int16_t*) &ul_ch_estimates[aa][symbol_offset],
	   frame_parms->twiddle_fft,
	   frame_parms->rev,
	   (frame_parms->log2_symbol_size),
	   (frame_parms->log2_symbol_size)/2,
	   1);
      */

#ifdef DEBUG_CH      
      if (aa==0) {
	if (Ns == 0) {
	  write_output("rxdataF_ext.m","rxF_ext",&rxdataF_ext[aa][symbol_offset],512*2,2,1);
	  write_output("tmpin_ifft.m","drs_in",temp_in_ifft_0,512,1,1);
	  write_output("drs_est0.m","drs0",ul_ch_estimates_time[aa],512*2,2,1);
	}
	else
	  write_output("drs_est1.m","drs1",ul_ch_estimates_time[aa],512*2,2,1);
      }
#endif


      if(cooperation_flag == 2) {
          memset(temp_in_ifft_0,0,frame_parms->ofdm_symbol_size*sizeof(int32_t*)*2);
          memset(temp_in_ifft_1,0,frame_parms->ofdm_symbol_size*sizeof(int32_t*)*2);
          memset(temp_in_fft_0,0,frame_parms->ofdm_symbol_size*sizeof(int32_t*)*2);
          memset(temp_in_fft_1,0,frame_parms->ofdm_symbol_size*sizeof(int32_t*)*2);
          
          temp_in_ifft_ptr = &temp_in_ifft_0[0];
          
          i = symbol_offset;
          
          for(j=0;j<(frame_parms->N_RB_UL*12);j++){
              temp_in_ifft_ptr[j] = ul_ch_estimates[aa][i];
              i++;
          }
          
          alpha_ind = 0;
          // Compensating for the phase shift introduced at the transmitter
          for(i=symbol_offset;i<symbol_offset+Msc_RS;i++){
              ul_ch_estimates_re = ((int16_t*) ul_ch_estimates[aa])[i<<1];
              ul_ch_estimates_im = ((int16_t*) ul_ch_estimates[aa])[(i<<1)+1];
              //	  ((int16_t*) ul_ch_estimates[aa])[i<<1] =  (i%2 == 1? 1:-1) * ul_ch_estimates_re;
              ((int16_t*) ul_ch_estimates[aa])[i<<1] =  
                  (int16_t) (((int32_t) (alpha_re[alpha_ind]) * (int32_t) (ul_ch_estimates_re) + 
                          (int32_t) (alpha_im[alpha_ind]) * (int32_t) (ul_ch_estimates_im))>>15);
              
              //((int16_t*) ul_ch_estimates[aa])[(i<<1)+1] =  (i%2 == 1? 1:-1) * ul_ch_estimates_im;
              ((int16_t*) ul_ch_estimates[aa])[(i<<1)+1] = 
                  (int16_t) (((int32_t) (alpha_re[alpha_ind]) * (int32_t) (ul_ch_estimates_im) -  
                          (int32_t) (alpha_im[alpha_ind]) * (int32_t) (ul_ch_estimates_re))>>15);
              
              alpha_ind+=10;
              if (alpha_ind>11)
                  alpha_ind-=12;
          }
	
	//Extracting Channel Estimates for Distributed Alamouti Receiver Combining
	
	temp_in_ifft_ptr = &temp_in_ifft_1[0];
	
	i = symbol_offset;
	
	for(j=0;j<(frame_parms->N_RB_UL*12);j++){
	  temp_in_ifft_ptr[j] = ul_ch_estimates[aa][i];
	  i++;
	}
	
	
	fft((int16_t*) &temp_in_ifft_0[0],                          // Performing IFFT on Combined Channel Estimates
	    temp_out_ifft_0, 
	    frame_parms->twiddle_ifft,
	    frame_parms->rev,
	    (frame_parms->log2_symbol_size),
	    (frame_parms->log2_symbol_size)/2,
	    0);

	fft((int16_t*) &temp_in_ifft_1[0],                          // Performing IFFT on Combined Channel Estimates
	    temp_out_ifft_1, 
	    frame_parms->twiddle_ifft,
	    frame_parms->rev,
	    (frame_parms->log2_symbol_size),
	    (frame_parms->log2_symbol_size)/2,
	    0);
	



	// because the ifft is not power preserving, we should apply the factor sqrt(power_correction) here, but we rather apply power_correction here and nothing after the next fft
	in_fft_ptr_0 = &temp_in_fft_0[0];
	in_fft_ptr_1 = &temp_in_fft_1[0];
	 
	for(j=0;j<(1<<(frame_parms->log2_symbol_size))/12;j++)
	  {
	    if (j>19) {
	      ((int16_t*)in_fft_ptr_0)[-40+(2*j)] = ((int16_t*)temp_out_ifft_0)[-80+(4*j)]*rx_power_correction;
	      ((int16_t*)in_fft_ptr_0)[-40+(2*j)+1] = ((int16_t*)temp_out_ifft_0)[-80+(4*j+1)]*rx_power_correction;
	      ((int16_t*)in_fft_ptr_1)[-40+(2*j)] = ((int16_t*)temp_out_ifft_1)[-80+(4*j)]*rx_power_correction;
	      ((int16_t*)in_fft_ptr_1)[-40+(2*j)+1] = ((int16_t*)temp_out_ifft_1)[-80+(4*j)+1]*rx_power_correction;
	    }
	    else {
	      ((int16_t*)in_fft_ptr_0)[2*(frame_parms->ofdm_symbol_size-20+j)] = ((int16_t*)temp_out_ifft_0)[4*(frame_parms->ofdm_symbol_size-20+j)]*rx_power_correction;
	      ((int16_t*)in_fft_ptr_0)[2*(frame_parms->ofdm_symbol_size-20+j)+1] = ((int16_t*)temp_out_ifft_0)[4*(frame_parms->ofdm_symbol_size-20+j)+1]*rx_power_correction;
	      ((int16_t*)in_fft_ptr_1)[2*(frame_parms->ofdm_symbol_size-20+j)] = ((int16_t*)temp_out_ifft_1)[4*(frame_parms->ofdm_symbol_size-20+j)]*rx_power_correction;
	      ((int16_t*)in_fft_ptr_1)[2*(frame_parms->ofdm_symbol_size-20+j)+1] = ((int16_t*)temp_out_ifft_1)[4*(frame_parms->ofdm_symbol_size-20+j)+1]*rx_power_correction;
	    }
	  }
	  

	fft((int16_t*) &temp_in_fft_0[0],                        // Performing FFT to obtain the Channel Estimates for UE0 to eNB1
	    temp_out_fft_0,
	    frame_parms->twiddle_fft,
	    frame_parms->rev,
	    frame_parms->log2_symbol_size,
	    frame_parms->log2_symbol_size>>1,
	    0);

	out_fft_ptr_0 = &ul_ch_estimates_0[aa][symbol_offset]; // CHANNEL ESTIMATES FOR UE0 TO eNB1
	temp_out_fft_0_ptr = (int32_t*) temp_out_fft_0;

	i=0;

	for(j=0;j<frame_parms->N_RB_UL*12;j++){
	  out_fft_ptr_0[i] = temp_out_fft_0_ptr[2*j];
	  i++;
	}

	fft((int16_t*) &temp_in_fft_1[0],                          // Performing FFT to obtain the Channel Estimates for UE1 to eNB1
	    temp_out_fft_1,
	    frame_parms->twiddle_fft,
	    frame_parms->rev,
	    frame_parms->log2_symbol_size,
	    frame_parms->log2_symbol_size>>1,
	    0);

	out_fft_ptr_1 = &ul_ch_estimates_1[aa][symbol_offset];   // CHANNEL ESTIMATES FOR UE1 TO eNB1
	temp_out_fft_1_ptr = (int32_t*) temp_out_fft_1;

	i=0;
	  
	for(j=0;j<frame_parms->N_RB_UL*12;j++){
	  out_fft_ptr_1[i] = temp_out_fft_1_ptr[2*j];
	  i++;
	}

#ifdef DEBUG_CH
#ifdef USER_MODE
	if((aa == 0)&& (cooperation_flag == 2)){
	  write_output("test1.m","t1",temp_in_ifft_0,512,1,1);
	  write_output("test2.m","t2",temp_out_ifft_0,512*2,2,1);
	  //	    write_output("test2.m","t2",ul_ch_estimates_time[aa],512*2,2,1);
	  write_output("test3.m","t3",temp_in_fft_0,512,1,1);  
	  write_output("test4.m","t4",temp_out_fft_0,512,1,1);
	  write_output("test5.m","t5",temp_in_fft_1,512,1,1);  
	  write_output("test6.m","t6",temp_out_fft_1,512,1,1);
	}
#endif
#endif

      }//cooperation_flag == 2

      if (Ns&1) {//we are in the second slot of the sub-frame, so do the interpolation

	ul_ch1 = &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*pilot_pos1];
	ul_ch2 = &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*pilot_pos2];


	if(cooperation_flag == 2)// For Distributed Alamouti
	  {
	    ul_ch1_0 = &ul_ch_estimates_0[aa][frame_parms->N_RB_UL*12*pilot_pos1];
	    ul_ch2_0 = &ul_ch_estimates_0[aa][frame_parms->N_RB_UL*12*pilot_pos2];

	    ul_ch1_1 = &ul_ch_estimates_1[aa][frame_parms->N_RB_UL*12*pilot_pos1];
	    ul_ch2_1 = &ul_ch_estimates_1[aa][frame_parms->N_RB_UL*12*pilot_pos2];
	  }

    // Estimation of phase difference between the 2 channel estimates
    delta_phase = lte_ul_freq_offset_estimation(frame_parms,
                                                ul_ch_estimates[aa],
                                                N_rb_alloc);
    // negative phase index indicates negative Im of ru
    //    msg("delta_phase: %d\n",delta_phase);

#ifdef DEBUG_CH
	msg("lte_ul_channel_estimation: ul_ch1 = %p, ul_ch2 = %p, pilot_pos1=%d, pilot_pos2=%d\n",ul_ch1, ul_ch2, pilot_pos1,pilot_pos2); 
#endif
	for (k=0;k<frame_parms->symbols_per_tti;k++) {

	  // we scale alpha and beta by SCALE (instead of 0x7FFF) to avoid overflows 
	  alpha = (int16_t) (((int32_t) SCALE * (int32_t) (pilot_pos2-k))/(pilot_pos2-pilot_pos1));
	  beta  = (int16_t) (((int32_t) SCALE * (int32_t) (k-pilot_pos1))/(pilot_pos2-pilot_pos1));
	  
	  
#ifdef DEBUG_CH
	  msg("lte_ul_channel_estimation: k=%d, alpha = %d, beta = %d\n",k,alpha,beta); 
#endif
	  //symbol_offset_subframe = frame_parms->N_RB_UL*12*k;

	  // interpolate between estimates
      if ((k != pilot_pos1) && (k != pilot_pos2))  {
          //          multadd_complex_vector_real_scalar((int16_t*) ul_ch1,alpha,(int16_t*) &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*k],1,Msc_RS);
          //          multadd_complex_vector_real_scalar((int16_t*) ul_ch2,beta ,(int16_t*) &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*k],0,Msc_RS);

          //          multadd_complex_vector_real_scalar((int16_t*) ul_ch1,SCALE,(int16_t*) &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*k],1,Msc_RS);
          //          multadd_complex_vector_real_scalar((int16_t*) ul_ch2,SCALE,(int16_t*) &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*k],0,Msc_RS);
          //          msg("phase = %d\n",ru[2*cmax(((delta_phase/7)*(k-3)),0)]);
                    		    
          // the phase is linearly interpolated
          current_phase1 = (delta_phase/7)*(k-pilot_pos1);
          current_phase2 = (delta_phase/7)*(k-pilot_pos2);
          //          msg("sym: %d, current_phase1: %d, current_phase2: %d\n",k,current_phase1,current_phase2);
          // set the right quadrant
          (current_phase1 > 0) ? (ru1 = ru_90) : (ru1 = ru_90c);
          (current_phase2 > 0) ? (ru2 = ru_90) : (ru2 = ru_90c);
          // take absolute value and clip
          current_phase1 = cmin(abs(current_phase1),127);
          current_phase2 = cmin(abs(current_phase2),127);

          //          msg("sym: %d, current_phase1: %d, ru: %d + j%d, current_phase2: %d, ru: %d + j%d\n",k,current_phase1,ru1[2*current_phase1],ru1[2*current_phase1+1],current_phase2,ru2[2*current_phase2],ru2[2*current_phase2+1]);   

          // rotate channel estimates by estimated phase
          rotate_cpx_vector_norep((int16_t*) ul_ch1, 
                                  &ru1[2*current_phase1], 
                                  (int16_t*) &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*k],
                                  Msc_RS, 
                                  15);

          rotate_cpx_vector_norep((int16_t*) ul_ch2, 
                                  &ru2[2*current_phase2], 
                                  (int16_t*) &tmp_estimates[0],
                                  Msc_RS, 
                                  15);

          // Combine the two rotated estimates 
          multadd_complex_vector_real_scalar((int16_t*) &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*k],SCALE,(int16_t*) &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*k],1,Msc_RS);
          multadd_complex_vector_real_scalar((int16_t*) &tmp_estimates[0],SCALE,(int16_t*) &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*k],0,Msc_RS);
          
          /*
          if ((k<pilot_pos1) || ((k>pilot_pos2))) {
              
              multadd_complex_vector_real_scalar((int16_t*) &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*k],SCALE>>1,(int16_t*) &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*k],1,Msc_RS);

              multadd_complex_vector_real_scalar((int16_t*) &tmp_estimates[0],SCALE>>1,(int16_t*) &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*k],0,Msc_RS);
              
          } else {
              
              multadd_complex_vector_real_scalar((int16_t*) &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*k],SCALE>>1,(int16_t*) &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*k],1,Msc_RS);

              multadd_complex_vector_real_scalar((int16_t*) &tmp_estimates[0],SCALE>>1,(int16_t*) &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*k],0,Msc_RS);

              //              multadd_complex_vector_real_scalar((int16_t*) &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*k],alpha,(int16_t*) &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*k],1,Msc_RS);

              //              multadd_complex_vector_real_scalar((int16_t*) &tmp_estimates[0],beta ,(int16_t*) &ul_ch_estimates[aa][frame_parms->N_RB_UL*12*k],0,Msc_RS);

          }
          */

	    //	    memcpy(&ul_ch_estimates[aa][frame_parms->N_RB_UL*12*k],ul_ch1,Msc_RS*sizeof(int32_t));
	    if(cooperation_flag == 2)// For Distributed Alamouti
	      {
		multadd_complex_vector_real_scalar((int16_t*) ul_ch1_0,beta ,(int16_t*) &ul_ch_estimates_0[aa][frame_parms->N_RB_UL*12*k],1,Msc_RS);
		multadd_complex_vector_real_scalar((int16_t*) ul_ch2_0,alpha,(int16_t*) &ul_ch_estimates_0[aa][frame_parms->N_RB_UL*12*k],0,Msc_RS);

		multadd_complex_vector_real_scalar((int16_t*) ul_ch1_1,beta ,(int16_t*) &ul_ch_estimates_1[aa][frame_parms->N_RB_UL*12*k],1,Msc_RS);
		multadd_complex_vector_real_scalar((int16_t*) ul_ch2_1,alpha,(int16_t*) &ul_ch_estimates_1[aa][frame_parms->N_RB_UL*12*k],0,Msc_RS);
	      }

	  }
	} //for(k=...

	// because of the scaling of alpha and beta we also need to scale the final channel estimate at the pilot positions 
	
    //    multadd_complex_vector_real_scalar((int16_t*) ul_ch1,SCALE,(int16_t*) ul_ch1,1,Msc_RS);
    //    multadd_complex_vector_real_scalar((int16_t*) ul_ch2,SCALE,(int16_t*) ul_ch2,1,Msc_RS);
	
	if(cooperation_flag == 2)// For Distributed Alamouti
	  {
	    multadd_complex_vector_real_scalar((int16_t*) ul_ch1_0,SCALE,(int16_t*) ul_ch1_0,1,Msc_RS);
	    multadd_complex_vector_real_scalar((int16_t*) ul_ch2_0,SCALE,(int16_t*) ul_ch2_0,1,Msc_RS);

	    multadd_complex_vector_real_scalar((int16_t*) ul_ch1_1,SCALE,(int16_t*) ul_ch1_1,1,Msc_RS);
	    multadd_complex_vector_real_scalar((int16_t*) ul_ch2_1,SCALE,(int16_t*) ul_ch2_1,1,Msc_RS);
	  }

	//write_output("drs_est.m","drsest",ul_ch_estimates[0],300*12,1,1);
	/*if(cooperation_flag == 2)// For Distributed Alamouti
	  {
	  write_output("drs_est.m","drsest",ul_ch_estimates[0],300*12,1,1);
	  write_output("drs_est0.m","drsest0",ul_ch_estimates_0[0],300*12,1,1);
	  write_output("drs_est1.m","drsest1",ul_ch_estimates_1[0],300*12,1,1);
	  }*/


      } //if (Ns&1)

      /*
      memset(temp_in_ifft_0,0,frame_parms->ofdm_symbol_size*sizeof(int32_t)*2);   
      // Convert to time domain for visualization
      for(i=0;i<Msc_RS;i++)
          ((int32_t*)temp_in_ifft_0)[i] = ul_ch_estimates[aa][symbol_offset+i];
	    
	  
      fft( (int16_t*) temp_in_ifft_0,                          
           (Ns%2) ? ((int16_t*) ul_ch_estimates_time[aa]) : ((int16_t*) &ul_ch_estimates_time[aa][2*frame_parms->ofdm_symbol_size]),
	   frame_parms->twiddle_ifft,
	   frame_parms->rev,
	   (frame_parms->log2_symbol_size),
	   (frame_parms->log2_symbol_size)/2,
	   0);
      */

    } //for(aa=...
    
  } //if(l==...


  
  return(0);
}       

extern uint16_t transmission_offset_tdd[16];
#define DEBUG_SRS

int32_t lte_srs_channel_estimation(LTE_DL_FRAME_PARMS *frame_parms,
			       LTE_eNB_COMMON *eNb_common_vars,
			       LTE_eNB_SRS *eNb_srs_vars,
			       SOUNDINGRS_UL_CONFIG_DEDICATED *soundingrs_ul_config_dedicated,
			       unsigned char sub_frame_number,
			       unsigned char eNb_id) {

  int T_SFC,aa;
  int N_symb,symbol;
  uint8_t nb_antennas_rx = frame_parms->nb_antennas_tx_eNB;
#ifdef DEBUG_SRS
  char fname[40], vname[40];
#endif

  uint8_t Ssrs  = frame_parms->soundingrs_ul_config_common.srs_SubframeConfig;

  N_symb = 2*7-frame_parms->Ncp;
  symbol = (sub_frame_number+1)*N_symb-1; //SRS is always in last symbol of subframe
  T_SFC = (Ssrs<=7 ? 5 : 10);

  /* 
     msg("SRS channel estimation eNb %d, subframs %d, %d %d %d %d %d\n",eNb_id,sub_frame_number,
     SRS_parms->Csrs,
     SRS_parms->Bsrs,
     SRS_parms->kTC,
     SRS_parms->n_RRC,
     SRS_parms->Ssrs);
  */

  if ((1<<(sub_frame_number%T_SFC))&transmission_offset_tdd[Ssrs]) {

    if (generate_srs_rx(frame_parms, 
			soundingrs_ul_config_dedicated,
			eNb_srs_vars->srs)==-1) {
      msg("lte_srs_channel_estimation: Error in generate_srs_rx\n");
      return(-1);
    }

    for (aa=0;aa<nb_antennas_rx;aa++) {
#ifdef DEBUG_SRS
      msg("SRS channel estimation eNb %d, subframs %d, aarx %d, %p, %p, %p\n",eNb_id,sub_frame_number,aa,
	  &eNb_common_vars->rxdataF[eNb_id][aa][2*frame_parms->ofdm_symbol_size*symbol],
	  eNb_srs_vars->srs,
	  eNb_srs_vars->srs_ch_estimates[eNb_id][aa]);
#endif

      //write_output("eNb_rxF.m","rxF",&eNb_common_vars->rxdataF[0][aa][2*frame_parms->ofdm_symbol_size*symbol],2*(frame_parms->ofdm_symbol_size),2,1);
      //write_output("eNb_srs.m","srs_eNb",eNb_common_vars->srs,(frame_parms->ofdm_symbol_size),1,1);

      mult_cpx_vector_norep((int16_t*) &eNb_common_vars->rxdataF[eNb_id][aa][2*frame_parms->ofdm_symbol_size*symbol],
			    (int16_t*) eNb_srs_vars->srs,
			    (int16_t*) eNb_srs_vars->srs_ch_estimates[eNb_id][aa],
			    frame_parms->ofdm_symbol_size,
			    15);

      //msg("SRS channel estimation cmult out\n");
#ifdef USER_MODE
#ifdef DEBUG_SRS
      sprintf(fname,"eNB_id%d_an%d_srs_ch_est.m",eNb_id,aa);
      sprintf(vname,"eNB%d_%d_srs_ch_est",eNb_id,aa);
      write_output(fname,vname,eNb_srs_vars->srs_ch_estimates[eNb_id][aa],frame_parms->ofdm_symbol_size,1,1);
#endif
#endif
    }
  }
  /*
    else {
    for (aa=0;aa<nb_antennas_rx;aa++) 
    bzero(eNb_srs_vars->srs_ch_estimates[eNb_id][aa],frame_parms->ofdm_symbol_size*sizeof(int));
    }
  */
  return(0);
}

int16_t lte_ul_freq_offset_estimation(LTE_DL_FRAME_PARMS *frame_parms,
                                 int32_t *ul_ch_estimates,
                                 uint16_t nb_rb) {

    int k, rb;
    int a_idx = 64;
    uint8_t conj_flag = 0;
    uint8_t output_shift;
    int pilot_pos1 = 3 - frame_parms->Ncp;
    int pilot_pos2 = 10 - 2*frame_parms->Ncp;
    __m128i *ul_ch1 = (__m128i*)&ul_ch_estimates[pilot_pos1*frame_parms->N_RB_UL*12];
    __m128i *ul_ch2 = (__m128i*)&ul_ch_estimates[pilot_pos2*frame_parms->N_RB_UL*12];
    int32_t avg[2]; 
    int16_t Ravg[2]; Ravg[0]=0; Ravg[1]=0; 
    int16_t iv, rv, phase_idx;
    __m128i avg128U1, avg128U2, R[3], mmtmpD0,mmtmpD1,mmtmpD2,mmtmpD3;

    // round(tan((pi/4)*[1:1:N]/N)*pow2(15))
    int16_t alpha[128] = {201, 402, 603, 804, 1006, 1207, 1408, 1610, 1811, 2013, 2215, 2417, 2619, 2822, 3024, 3227, 3431, 3634, 3838, 4042, 4246, 4450, 4655, 4861, 5066, 5272, 5479, 5686, 5893, 6101, 6309, 6518, 6727, 6937, 7147, 7358, 7570, 7782, 7995, 8208, 8422, 8637, 8852, 9068, 9285, 9503, 9721, 9940, 10160, 10381, 10603, 10825, 11049, 11273, 11498, 11725, 11952, 12180, 12410, 12640, 12872, 13104, 13338, 13573, 13809, 14046, 14285, 14525, 14766, 15009, 15253, 15498, 15745, 15993, 16243, 16494, 16747, 17001, 17257, 17515, 17774, 18035, 18298, 18563, 18829, 19098, 19368, 19640, 19915, 20191, 20470, 20750, 21033, 21318, 21605, 21895, 22187, 22481, 22778, 23078, 23380, 23685, 23992, 24302, 24615, 24931, 25250, 25572, 25897, 26226, 26557, 26892, 27230, 27572, 27917, 28266, 28618, 28975, 29335, 29699, 30067, 30440, 30817, 31198, 31583, 31973, 32368, 32767};
 
    // compute log2_maxh (output_shift)
    avg128U1 = _mm_xor_si128(avg128U1,avg128U1);
    avg128U2 = _mm_xor_si128(avg128U2,avg128U2);

    for (rb=0;rb<nb_rb;rb++) {      
        avg128U1 = _mm_add_epi32(avg128U1,_mm_madd_epi16(ul_ch1[0],ul_ch1[0]));
        avg128U1 = _mm_add_epi32(avg128U1,_mm_madd_epi16(ul_ch1[1],ul_ch1[1]));
        avg128U1 = _mm_add_epi32(avg128U1,_mm_madd_epi16(ul_ch1[2],ul_ch1[2]));
        
        avg128U2 = _mm_add_epi32(avg128U2,_mm_madd_epi16(ul_ch2[0],ul_ch2[0]));
        avg128U2 = _mm_add_epi32(avg128U2,_mm_madd_epi16(ul_ch2[1],ul_ch2[1]));
        avg128U2 = _mm_add_epi32(avg128U2,_mm_madd_epi16(ul_ch2[2],ul_ch2[2]));

        ul_ch1+=3;	
        ul_ch2+=3;	
    }
    
    avg[0] = (((int*)&avg128U1)[0] + 
              ((int*)&avg128U1)[1] + 
              ((int*)&avg128U1)[2] + 
              ((int*)&avg128U1)[3])/(nb_rb*12);

    avg[1] = (((int*)&avg128U2)[0] + 
              ((int*)&avg128U2)[1] + 
              ((int*)&avg128U2)[2] + 
              ((int*)&avg128U2)[3])/(nb_rb*12);

    //    msg("avg0 = %d, avg1 = %d\n",avg[0],avg[1]);
	avg[0] = cmax(avg[0],avg[1]);
    avg[1] = log2_approx(avg[0]);
    output_shift = cmax(0,avg[1]-10);
    //output_shift  = (log2_approx(avg[0])/2)+ log2_approx(frame_parms->nb_antennas_rx-1)+1;
    //    msg("avg= %d, shift = %d\n",avg[0],output_shift);

   ul_ch1 = (__m128i*)&ul_ch_estimates[pilot_pos1*frame_parms->N_RB_UL*12];
   ul_ch2 = (__m128i*)&ul_ch_estimates[pilot_pos2*frame_parms->N_RB_UL*12];

   // correlate and average the 2 channel estimates ul_ch1*ul_ch2
   for (rb=0;rb<nb_rb;rb++) {
       mmtmpD0 = _mm_madd_epi16(ul_ch1[0],ul_ch2[0]);
       mmtmpD1 = _mm_shufflelo_epi16(ul_ch1[0],_MM_SHUFFLE(2,3,0,1));
       mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
       mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)&conjugate);
       mmtmpD1 = _mm_madd_epi16(mmtmpD1,ul_ch2[0]);
       mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
       mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
       mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
       mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
       R[0] = _mm_packs_epi32(mmtmpD2,mmtmpD3);

       mmtmpD0 = _mm_madd_epi16(ul_ch1[1],ul_ch2[1]);
       mmtmpD1 = _mm_shufflelo_epi16(ul_ch1[1],_MM_SHUFFLE(2,3,0,1));
       mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
       mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)&conjugate);
       mmtmpD1 = _mm_madd_epi16(mmtmpD1,ul_ch2[1]);
       mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
       mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
       mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
       mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
       R[1] = _mm_packs_epi32(mmtmpD2,mmtmpD3);

       mmtmpD0 = _mm_madd_epi16(ul_ch1[2],ul_ch2[2]);
       mmtmpD1 = _mm_shufflelo_epi16(ul_ch1[2],_MM_SHUFFLE(2,3,0,1));
       mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
       mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)&conjugate);
       mmtmpD1 = _mm_madd_epi16(mmtmpD1,ul_ch2[2]);
       mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift);
       mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift);
       mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
       mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
       R[2] = _mm_packs_epi32(mmtmpD2,mmtmpD3);

       R[0] = _mm_add_epi16(_mm_srai_epi16(R[0],1),_mm_srai_epi16(R[1],1));
       R[0] = _mm_add_epi16(_mm_srai_epi16(R[0],1),_mm_srai_epi16(R[2],1));
       
       Ravg[0] += (((short*)&R)[0] + 
                   ((short*)&R)[2] + 
                   ((short*)&R)[4] + 
                   ((short*)&R)[6])/(nb_rb*4);
       
       Ravg[1] += (((short*)&R)[1] + 
                   ((short*)&R)[3] + 
                   ((short*)&R)[5] + 
                   ((short*)&R)[7])/(nb_rb*4);
       
       ul_ch1+=3;	
       ul_ch2+=3;	
   }

   // phase estimation on Ravg
   //   Ravg[0] = 56;
   //   Ravg[1] = 0;
   rv = Ravg[0];
   iv = Ravg[1];

   if (iv<0)
       iv = -Ravg[1];

   if (rv<iv) {
       rv = iv;
       iv = Ravg[0];
       conj_flag = 1;
   }

   //   msg("rv = %d, iv = %d\n",rv,iv);
   //   msg("max_avg = %d, log2_approx = %d, shift = %d\n",avg[0], avg[1], output_shift);

   for (k=0;k<6;k++) {
       (iv<(((int32_t)(alpha[a_idx]*rv))>>15)) ? (a_idx -= 32>>k) : (a_idx += 32>>k);
   }

   (conj_flag==1) ? (phase_idx = 127-(a_idx>>1)) : (phase_idx = (a_idx>>1));

   if (Ravg[1]<0)
       phase_idx = -phase_idx;

    return(phase_idx);
}
