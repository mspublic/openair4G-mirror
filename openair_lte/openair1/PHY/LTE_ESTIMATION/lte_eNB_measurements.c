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

int rx_power_avg_eNB[3][3];


void lte_eNB_I0_measurements(LTE_eNB_COMMON *eNB_common_vars,
			     LTE_DL_FRAME_PARMS *frame_parms,
			     PHY_MEASUREMENTS_eNB *phy_measurements,
			     unsigned char eNB_id) {

  unsigned int aarx;

  // noise measurements
  // for the moment we measure the noise on the 7th OFDM symbol (in S subframe) 
  phy_measurements->n0_power_tot = 0;

  for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++) {
#ifdef USER_MODE
      phy_measurements->n0_power[aarx] = signal_energy(&eNB_common_vars->rxdata[eNB_id][aarx][19*(frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples)],frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples);
#else
      phy_measurements->n0_power[aarx] = signal_energy(&eNB_common_vars->rxdata[eNB_id][aarx][19*frame_parms->ofdm_symbol_size],frame_parms->ofdm_symbol_size);
#endif
      phy_measurements->n0_power_dB[aarx] = (unsigned short) dB_fixed(phy_measurements->n0_power[aarx]);
      phy_measurements->n0_power_tot +=  phy_measurements->n0_power[aarx];
  }

  phy_measurements->n0_power_tot_dB = (unsigned short) dB_fixed(phy_measurements->n0_power_tot);
    //    printf("n0_power %d\n",phy_measurements->n0_avg_power_dB);

}

void lte_eNB_srs_measurements(LTE_eNB_COMMON *eNB_common_vars,
			      LTE_DL_FRAME_PARMS *frame_parms,
			      LTE_eNB_UE_stats *eNB_UE_stats,
			      PHY_MEASUREMENTS_eNB *phy_measurements,
			      unsigned char eNB_id,
			      unsigned char UE_id,
			      unsigned char init_averaging){


  int aarx,rx_power_correction;
  int rx_power;
  int i;
  unsigned int limit,rb;
  int *ul_ch;

  if (init_averaging == 1)
    rx_power_avg_eNB[eNB_id][UE_id] = 0;
  rx_power = 0;
  

  if ( (frame_parms->ofdm_symbol_size == 128) ||
       (frame_parms->ofdm_symbol_size == 512) )
    rx_power_correction = 2;
  else
    rx_power_correction = 1;



  for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++) {

	  
    phy_measurements->rx_spatial_power[UE_id][0][aarx] = 
      ((signal_energy_nodc(&eNB_common_vars->srs_ch_estimates[eNB_id][aarx][frame_parms->first_carrier_offset],
			   (frame_parms->N_RB_DL*6)) + 
	signal_energy_nodc(&eNB_common_vars->srs_ch_estimates[eNB_id][aarx][1],
			   (frame_parms->N_RB_DL*6)))*rx_power_correction) - 
      phy_measurements->n0_power[aarx];

    phy_measurements->rx_spatial_power[UE_id][0][aarx]<<=1;  // because of noise only in odd samples
	  
    phy_measurements->rx_spatial_power_dB[UE_id][0][aarx] = (unsigned short) dB_fixed(phy_measurements->rx_spatial_power[UE_id][0][aarx]);

    phy_measurements->wideband_cqi[eNB_id][aarx] = phy_measurements->rx_spatial_power[eNB_id][0][aarx];

  
    
  //      phy_measurements->rx_power[eNB_id][aarx]/=frame_parms->nb_antennas_tx;
    phy_measurements->wideband_cqi_dB[eNB_id][aarx] = (unsigned short) dB_fixed(phy_measurements->wideband_cqi[eNB_id][aarx]);
    rx_power += phy_measurements->wideband_cqi[eNB_id][aarx];
    //      phy_measurements->rx_avg_power_dB[eNB_id] += phy_measurements->rx_power_dB[eNB_id][aarx];
  }

  

  //    phy_measurements->rx_avg_power_dB[eNB_id]/=frame_parms->nb_antennas_rx;
  if (init_averaging == 0)
    rx_power_avg_eNB[UE_id][eNB_id] = ((k1*rx_power_avg_eNB[UE_id][eNB_id]) + (k2*rx_power))>>10;
  else
    rx_power_avg_eNB[UE_id][eNB_id] = rx_power;

  phy_measurements->wideband_cqi_tot[UE_id] = dB_fixed2(rx_power,2*phy_measurements->n0_power_tot);
  // times 2 since we have noise only in the odd carriers of the srs comb

  phy_measurements->rx_rssi_dBm[UE_id] = (int)dB_fixed(rx_power_avg_eNB[UE_id][eNB_id])-PHY_vars->rx_total_gain_eNB_dB;
 
    //    if (eNB_id == 0)
    //      printf("rx_power_avg[0] %d (%d,%d)\n",rx_power_avg[0],phy_measurements->rx_avg_power_dB[0],phy_measurements->rx_rssi_dBm[eNB_id]);
 
    //    printf("lte_ue_measurements: rx_power_dB[%d] %d (%f), sinr %d,cqi %d\n",eNB_id,phy_measurements->rx_avg_power_dB[eNB_id],10*log10(rx_power[eNB_id]),phy_measurements->wideband_sinr[eNB_id],phy_measurements->wideband_cqi[eNB_id]);
  

  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {

    
    for (rb=0;rb<frame_parms->N_RB_DL;rb++) {

      if (rb < 12)
	ul_ch    = &eNB_common_vars->srs_ch_estimates[eNB_id][aarx][frame_parms->first_carrier_offset + (rb*12)];
      else if (rb>12)
	ul_ch    = &eNB_common_vars->srs_ch_estimates[eNB_id][aarx][6 + (rb-13)*12];
      else {
	phy_measurements->subband_cqi_dB[eNB_id][aarx][rb] = 0;
	continue;
      }
      // cqi
      if (aarx==0)
	phy_measurements->subband_cqi_tot[UE_id][rb]=0;
      
      phy_measurements->subband_cqi[eNB_id][aarx][rb] = (signal_energy_nodc(ul_ch,12))*rx_power_correction - phy_measurements->n0_power[aarx];
      phy_measurements->subband_cqi_tot[eNB_id][rb] += phy_measurements->subband_cqi[eNB_id][aarx][rb];
      phy_measurements->subband_cqi_dB[eNB_id][aarx][rb] = dB_fixed2(phy_measurements->subband_cqi[eNB_id][aarx][rb],
									  2*phy_measurements->n0_power[aarx]);							
      // 2*n0_power since we have noise from the odd carriers in the comb of the srs

	//	  msg("subband_cqi[%d][%d][%d] => %d (%d dB)\n",eNB_id,aarx,rb,phy_measurements->subband_cqi[eNB_id][aarx][rb],phy_measurements->subband_cqi_dB[eNB_id][aarx][rb]);
      }
      
  }


  for (rb=0;rb<frame_parms->N_RB_DL;rb++) {
    phy_measurements->subband_cqi_tot_dB[eNB_id][rb] = dB_fixed2(phy_measurements->subband_cqi_tot[eNB_id][rb],phy_measurements->n0_power_tot);
    //	  msg("subband_cqi_tot[%d][%d] => %d dB (n0 %d)\n",eNB_id,rb,phy_measurements->subband_cqi_tot_dB[eNB_id][rb],phy_measurements->n0_power_tot);
  }
  
}
      




  

