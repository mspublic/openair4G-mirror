// this function fills the PHY_vars->PHY_measurement structure

#include "PHY/defs.h"
#include "PHY/extern.h"

#include "emmintrin.h"

#ifdef __SSE3__
#include "pmmintrin.h"
#include "tmmintrin.h"
#else
__m128i zeroPMI;
#define _mm_abs_epi16(xmmx) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zeroPMI,(xmmx)))
#define _mm_sign_epi16(xmmx,xmmy) _mm_xor_si128((xmmx),_mm_cmpgt_epi16(zeroPMI,(xmmy)))
#endif

//#define k1 1000
#define k1 1
#define k2 (1024-k1)

int rx_power_avg[3];

#ifdef USER_MODE
void print_shorts(char *s,__m128i *x) {

  short *tempb = (short *)x;

  printf("%s  : %d,%d,%d,%d,%d,%d,%d,%d\n",s,
         tempb[0],tempb[1],tempb[2],tempb[3],tempb[4],tempb[5],tempb[6],tempb[7]
         );

}
void print_ints(char *s,__m128i *x) {

  int *tempb = (int *)x;

  printf("%s  : %d,%d,%d,%d\n",s,
         tempb[0],tempb[1],tempb[2],tempb[3]
         );

}
#endif

__m128i pmi128_re,pmi128_im;
__m128i mmtmpPMI0,mmtmpPMI1,mmtmpPMI2,mmtmpPMI3;

void lte_ue_measurements(LTE_UE_COMMON *ue_common_vars,
			 LTE_DL_FRAME_PARMS *frame_parms,
			 PHY_MEASUREMENTS *phy_measurements,
			 unsigned int subframe_offset,
			 unsigned char N0_symbol,
			 unsigned char init_averaging){


  int aarx,aatx,eNB_id=0,rx_power_correction;
  int rx_power[3];
  int i;
  unsigned int limit,subband;
  __m128i *dl_ch0_128,*dl_ch1_128;
  int *dl_ch0,*dl_ch1;

#ifndef __SSE3__
  zeroPMI = _mm_xor_si128(zeroPMI,zeroPMI);
#endif

  for (eNB_id=0;eNB_id<3;eNB_id++) {
    if (init_averaging == 1)
      rx_power_avg[eNB_id] = 0;
    rx_power[eNB_id] = 0;
  }

  if ( (frame_parms->ofdm_symbol_size == 128) ||
       (frame_parms->ofdm_symbol_size == 512) )
    rx_power_correction = 2;
  else
    rx_power_correction = 1;


  // noise measurements
  // for the moment we measure the noise on the third OFDM symbol (e.g. S subframe) 
  if (N0_symbol == 1)
    phy_measurements->n0_power_tot = 0;

  for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++) {
    if (N0_symbol == 0) {
      phy_measurements->n0_power_dB[aarx] = -105 + PHY_vars->rx_total_gain_dB;
    } 
    else if (N0_symbol == 1) {
#ifdef USER_MODE
      phy_measurements->n0_power[aarx] = signal_energy(&ue_common_vars->rxdata[aarx][subframe_offset+frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples],frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples);
#else
      phy_measurements->n0_power[aarx] = signal_energy(&ue_common_vars->rxdata[aarx][subframe_offset+frame_parms->ofdm_symbol_size],frame_parms->ofdm_symbol_size);
#endif
      phy_measurements->n0_power_dB[aarx] = (unsigned short) dB_fixed(phy_measurements->n0_power[aarx]);
      phy_measurements->n0_power_tot +=  phy_measurements->n0_power[aarx];
    }
  }

  if (N0_symbol == 1) {
    phy_measurements->n0_power_tot_dB = (unsigned short) dB_fixed(phy_measurements->n0_power_tot);
    //    printf("n0_power %d\n",phy_measurements->n0_avg_power_dB);
  }

  for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++) {

    for (aatx=0; aatx<frame_parms->nb_antennas_tx; aatx++) {
      for (eNB_id=0;eNB_id<3;eNB_id++) {
	  
	phy_measurements->rx_spatial_power[eNB_id][aatx][aarx] = 
	  (signal_energy_nodc(&ue_common_vars->dl_ch_estimates[eNB_id][(aatx*frame_parms->nb_antennas_tx) + aarx][8],
			      (frame_parms->N_RB_DL*12)-8)*rx_power_correction) - 
	  phy_measurements->n0_power[aarx];
	  
	phy_measurements->rx_spatial_power_dB[eNB_id][aatx][aarx] = (unsigned short) dB_fixed(phy_measurements->rx_spatial_power[eNB_id][aatx][aarx]);

	if (aatx==0)
	  phy_measurements->wideband_cqi[eNB_id][aarx] = phy_measurements->rx_spatial_power[eNB_id][aatx][aarx];
	else
	  phy_measurements->wideband_cqi[eNB_id][aarx] += phy_measurements->rx_spatial_power[eNB_id][aatx][aarx];
      }
    }

    for (eNB_id = 0; eNB_id < 3; eNB_id++){
      //      phy_measurements->rx_power[eNB_id][aarx]/=frame_parms->nb_antennas_tx;
      phy_measurements->wideband_cqi_dB[eNB_id][aarx] = (unsigned short) dB_fixed(phy_measurements->wideband_cqi[eNB_id][aarx]);
      rx_power[eNB_id] += phy_measurements->wideband_cqi[eNB_id][aarx];
      //      phy_measurements->rx_avg_power_dB[eNB_id] += phy_measurements->rx_power_dB[eNB_id][aarx];
    }

  }

  for (eNB_id = 0; eNB_id < 3; eNB_id++){
    //    phy_measurements->rx_avg_power_dB[eNB_id]/=frame_parms->nb_antennas_rx;
    if (init_averaging == 0)
      rx_power_avg[eNB_id] = ((k1*rx_power_avg[eNB_id]) + (k2*rx_power[eNB_id]))>>10;
    else
      rx_power_avg[eNB_id] = rx_power[eNB_id];

    phy_measurements->wideband_cqi_tot[eNB_id] = dB_fixed2(rx_power[eNB_id],phy_measurements->n0_power_tot);

    phy_measurements->rx_rssi_dBm[eNB_id] = (int)dB_fixed(rx_power_avg[eNB_id])-PHY_vars->rx_total_gain_dB;
 
    //    if (eNB_id == 0)
    //      printf("rx_power_avg[0] %d (%d,%d)\n",rx_power_avg[0],phy_measurements->rx_avg_power_dB[0],phy_measurements->rx_rssi_dBm[eNB_id]);
 
    //    printf("lte_ue_measurements: rx_power_dB[%d] %d (%f), sinr %d,cqi %d\n",eNB_id,phy_measurements->rx_avg_power_dB[eNB_id],10*log10(rx_power[eNB_id]),phy_measurements->wideband_sinr[eNB_id],phy_measurements->wideband_cqi[eNB_id]);
  


  // cqi/pmi information
    if (eNB_id == 0) {

      for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
	dl_ch0    = &ue_common_vars->dl_ch_estimates[eNB_id][aarx][4];
	dl_ch1    = &ue_common_vars->dl_ch_estimates[eNB_id][2+aarx][4];
	
	for (subband=0;subband<7;subband++) {
	  
	  // cqi
	  if (aarx==0)
	    phy_measurements->subband_cqi_tot[eNB_id][subband]=0;

	  if (subband<6) {
	    //	    for (i=0;i<48;i++)
	    //	      printf("subband %d (%d) : %d,%d\n",subband,i,((short *)dl_ch0)[2*i],((short *)dl_ch0)[1+(2*i)]);
	    phy_measurements->subband_cqi[eNB_id][aarx][subband] = 
	      (signal_energy_nodc(dl_ch0,48) + signal_energy_nodc(dl_ch1,48))*rx_power_correction - phy_measurements->n0_power[aarx];
	      
	    phy_measurements->subband_cqi_tot[eNB_id][subband] += phy_measurements->subband_cqi[eNB_id][aarx][subband];
	    phy_measurements->subband_cqi_dB[eNB_id][aarx][subband] = dB_fixed2(phy_measurements->subband_cqi[eNB_id][aarx][subband],
										phy_measurements->n0_power[aarx]);
	  }
	  else {
	    //	    for (i=0;i<12;i++)
	      //	      printf("subband %d (%d) : %d,%d\n",subband,i,((short *)dl_ch0)[2*i],((short *)dl_ch0)[1+(2*i)]); 
	    phy_measurements->subband_cqi[eNB_id][aarx][subband] = (signal_energy_nodc(dl_ch0,12) + signal_energy_nodc(dl_ch1,12))*rx_power_correction - phy_measurements->n0_power[aarx];
	    phy_measurements->subband_cqi_tot[eNB_id][subband] += phy_measurements->subband_cqi[eNB_id][aarx][subband];
	    phy_measurements->subband_cqi_dB[eNB_id][aarx][subband] = dB_fixed2(phy_measurements->subband_cqi[eNB_id][aarx][subband],
										phy_measurements->n0_power[aarx]);							
	  }
	  dl_ch1+=48;
	  dl_ch0+=48;
	  //	  msg("subband_cqi[%d][%d][%d] => %d (%d dB)\n",eNB_id,aarx,subband,phy_measurements->subband_cqi[eNB_id][aarx][subband],phy_measurements->subband_cqi_dB[eNB_id][aarx][subband]);
	}
	
      }
      for (subband=0;subband<7;subband++) {
	phy_measurements->subband_cqi_tot_dB[eNB_id][subband] = dB_fixed2(phy_measurements->subband_cqi_tot[eNB_id][subband],phy_measurements->n0_power_tot);
	//	  msg("subband_cqi_tot[%d][%d] => %d dB (n0 %d)\n",eNB_id,subband,phy_measurements->subband_cqi_tot_dB[eNB_id][subband],phy_measurements->n0_power_tot);
      }
      if (frame_parms->nb_antennas_tx == 2) {
	
	
	for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
	  dl_ch0_128    = (__m128i *)&ue_common_vars->dl_ch_estimates[eNB_id][aarx][8];
	  dl_ch1_128    = (__m128i *)&ue_common_vars->dl_ch_estimates[eNB_id][2+aarx][8];
	  
	  for (subband=0;subband<7;subband++) {
	    
	    
	    // pmi
	    
	    pmi128_re = _mm_xor_si128(pmi128_re,pmi128_re);
	    pmi128_im = _mm_xor_si128(pmi128_im,pmi128_im);
	    // limit is the number of groups of 4 REs in a subband (12 = 4 RBs, 3 = 1 RB)
	    // for 5 MHz channelization, there are 7 subbands, 6 of size 4 RBs and 1 of size 1 RB
	    limit = (subband < 6) ? 12 : 3;
	    for (i=0;i<limit;i++) {
	      // multiply by conjugated channel
	      //	      print_shorts("ch0",dl_ch0_128);
	      //	      print_shorts("ch1",dl_ch1_128);
	      
	      mmtmpPMI0 = _mm_madd_epi16(dl_ch0_128[0],dl_ch1_128[0]);
	      //	      print_ints("re",&mmtmpPMI0);
	      
	      // mmtmpPMI0 contains real part of 4 consecutive outputs (32-bit)
	      mmtmpPMI1 = _mm_shufflelo_epi16(dl_ch0_128[0],_MM_SHUFFLE(2,3,0,1));
	      mmtmpPMI1 = _mm_shufflehi_epi16(mmtmpPMI1,_MM_SHUFFLE(2,3,0,1));
	      mmtmpPMI1 = _mm_sign_epi16(mmtmpPMI1,*(__m128i*)&conjugate[0]);
	      mmtmpPMI1 = _mm_madd_epi16(mmtmpPMI1,dl_ch1_128[0]);
	      //	      print_ints("im",&mmtmpPMI1);
	      // mmtmpPMI1 contains imag part of 4 consecutive outputs (32-bit)
	      
	      pmi128_re = _mm_add_epi32(pmi128_re,mmtmpPMI0);
	      pmi128_im = _mm_add_epi32(pmi128_im,mmtmpPMI1);
	      dl_ch0_128++;
	      dl_ch1_128++;
	    }
	    phy_measurements->subband_pmi_re[eNB_id][subband][aarx] = (((int *)&pmi128_re)[0] + ((short *)&pmi128_re)[1] + ((short *)&pmi128_re)[2] + ((short *)&pmi128_re)[3])>>2;
	    phy_measurements->subband_pmi_im[eNB_id][subband][aarx] = (((int *)&pmi128_im)[0] + ((short *)&pmi128_im)[1] + ((short *)&pmi128_im)[2] + ((short *)&pmi128_im)[3])>>2;
	    phy_measurements->wideband_pmi_re[eNB_id][aarx] += phy_measurements->subband_pmi_re[eNB_id][subband][aarx];
	    phy_measurements->wideband_pmi_im[eNB_id][aarx] += phy_measurements->subband_pmi_im[eNB_id][subband][aarx];
	    //	    msg("subband_pmi[%d][%d][%d] => (%d,%d)\n",eNB_id,subband,aarx,phy_measurements->subband_pmi_re[eNB_id][subband][aarx],phy_measurements->subband_pmi_im[eNB_id][subband][aarx]);
	    
	  } // subband loop
	} // rx antenna loop
      }  
    }  // if eNB_id == 0

    if (eNB_id == 0) {
      phy_measurements->rank[eNB_id] = 0;
      for (i=0;i<NUMBER_OF_SUBBANDS;i++) {
	if (phy_measurements->subband_cqi_dB[eNB_id][0][i] >= phy_measurements->subband_cqi_dB[eNB_id][1][i])
	  phy_measurements->selected_rx_antennas[eNB_id][i] = 0;
	else
	  phy_measurements->selected_rx_antennas[eNB_id][i] = 1;
      }
    }
  }  // eNB_id loop
}
