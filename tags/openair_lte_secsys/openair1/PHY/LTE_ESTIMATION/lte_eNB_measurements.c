#include "PHY/defs.h"
#include "PHY/extern.h"

#include "emmintrin.h"

#ifdef __SSE3__
#include "pmmintrin.h"
#include "tmmintrin.h"
#else
#endif

//#define k1 1000
#define k1 1024
#define k2 (1024-k1)

int rx_power_avg_eNB[3][3];


void lte_eNB_I0_measurements(PHY_VARS_eNB *phy_vars_eNb,
			     unsigned char eNB_id,
			     unsigned char clear) {
  LTE_eNB_COMMON *eNB_common_vars = &phy_vars_eNb->lte_eNB_common_vars;
  LTE_DL_FRAME_PARMS *frame_parms = &phy_vars_eNb->lte_frame_parms;
  PHY_MEASUREMENTS_eNB *phy_measurements = &phy_vars_eNb->PHY_measurements_eNB[eNB_id];
			     
			     
  unsigned int aarx,rx_power_correction;
  unsigned int rb;
  int *ul_ch;
  int n0_power_tot;
  int i;

  // noise measurements
  // for the moment we measure the noise on the 7th OFDM symbol (in S subframe) 
  
phy_measurements->n0_power_tot = 0;

  /*  printf("rxdataF0 %p, rxdataF1 %p\n",
	 (&eNB_common_vars->rxdataF[0][0][(frame_parms->ofdm_symbol_size + frame_parms->first_carrier_offset)<<1 ]),
	 (&eNB_common_vars->rxdataF[0][1][(frame_parms->ofdm_symbol_size + frame_parms->first_carrier_offset)<<1 ]));
  */
  /*
  for (i=0;i<512;i++)
    printf("sector 0 antenna 0 : %d,%d\n",((short *)&eNB_common_vars->rxdataF[0][0][(19*frame_parms->ofdm_symbol_size)<<1])[i<<1],
	   ((short *)&eNB_common_vars->rxdataF[0][0][(19*frame_parms->ofdm_symbol_size)<<1])[1+(i<<1)]);
  
  for (i=0;i<12;i++)
    //    printf("sector 0 antenna 1 : %d,%d\n",((short *)&eNB_common_vars->rxdataF[0][1][(19*frame_parms->ofdm_symbol_size)<<1])[i<<1],
	   ((short *)&eNB_common_vars->rxdataF[0][1][(19*frame_parms->ofdm_symbol_size)<<1])[1+(i<<1)]);
  */

  if ( (frame_parms->ofdm_symbol_size == 128) ||
       (frame_parms->ofdm_symbol_size == 512) )
    rx_power_correction = 2;
  else
    rx_power_correction = 1;

  for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++) {
    if (clear == 1)
      phy_measurements->n0_power[aarx]=0;
#ifdef USER_MODE
    phy_measurements->n0_power[aarx] = ((k1*signal_energy(&eNB_common_vars->rxdata[eNB_id][aarx][19*(frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples)],frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples)) + k2*phy_measurements->n0_power[aarx])>>10;
#else
    phy_measurements->n0_power[aarx] = ((k1*signal_energy(&eNB_common_vars->rxdata[eNB_id][aarx][19*frame_parms->ofdm_symbol_size],frame_parms->ofdm_symbol_size))+k2*phy_measurements->n0_power[aarx])>>10;    
#endif
    phy_measurements->n0_power_dB[aarx] = (unsigned short) dB_fixed(phy_measurements->n0_power[aarx]);
    phy_measurements->n0_power_tot +=  phy_measurements->n0_power[aarx];
  }

  phy_measurements->n0_power_tot_dB = (unsigned short) dB_fixed(phy_measurements->n0_power_tot);
  
  phy_measurements->n0_power_tot_dBm = phy_measurements->n0_power_tot_dB - phy_vars_eNb->rx_total_gain_eNB_dB;
  //    printf("n0_power %d\n",phy_measurements->n0_avg_power_dB);
  

  for (rb=0;rb<frame_parms->N_RB_UL;rb++) {

    n0_power_tot=0;
    for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {
      
      
      if (rb < 12)
	//	ul_ch    = &eNB_common_vars->rxdataF[eNB_id][aarx][((19*(frame_parms->ofdm_symbol_size)) + frame_parms->first_carrier_offset + (rb*12))<<1];
	ul_ch    = &eNB_common_vars->rxdataF[eNB_id][aarx][((7*frame_parms->ofdm_symbol_size) + frame_parms->first_carrier_offset + (rb*12))<<1];
      else if (rb>12)
	ul_ch    = &eNB_common_vars->rxdataF[eNB_id][aarx][((7*frame_parms->ofdm_symbol_size) + 6 + (rb-13)*12)<<1];
      else {
	ul_ch = NULL;
      }

      if (clear == 1)
	phy_measurements->n0_subband_power[aarx][rb]=0;

      if (ul_ch) {
	//	for (i=0;i<24;i+=2)
	//	  printf("re %d => %d\n",i/2,ul_ch[i]);
	phy_measurements->n0_subband_power[aarx][rb] = ((k1*(signal_energy_nodc(ul_ch,24))*rx_power_correction) + (k2*phy_measurements->n0_subband_power[aarx][rb]))>>11;  // 11 and 24 to compensate for repeated signal format    

	phy_measurements->n0_subband_power_dB[aarx][rb] = dB_fixed(phy_measurements->n0_subband_power[aarx][rb]);	
	//	printf("eNb %d, aarx %d, rb %d : energy %d (%d dB)\n",eNB_id,aarx,rb,signal_energy_nodc(ul_ch,24),	phy_measurements->n0_subband_power_dB[aarx][rb]);
	n0_power_tot +=	phy_measurements->n0_subband_power[aarx][rb];
      }
      else {
	phy_measurements->n0_subband_power[aarx][rb] = 1;
	phy_measurements->n0_subband_power_dB[aarx][rb] = -99;
	n0_power_tot = 1;
      }
    }
    phy_measurements->n0_subband_power_tot_dB[rb] = dB_fixed(n0_power_tot);
    phy_measurements->n0_subband_power_tot_dBm[rb] = phy_measurements->n0_subband_power_tot_dB[rb] - phy_vars_eNb->rx_total_gain_eNB_dB - 14;
    
  }
}


void lte_eNB_srs_measurements(PHY_VARS_eNB *phy_vars_eNb,
			      unsigned char eNB_id,
			      unsigned char UE_id,
			      unsigned char init_averaging){
  LTE_eNB_COMMON *eNB_common_vars = &phy_vars_eNb->lte_eNB_common_vars;
  LTE_DL_FRAME_PARMS *frame_parms = &phy_vars_eNb->lte_frame_parms;
  LTE_eNB_UE_stats *eNB_UE_stats = &phy_vars_eNb->eNB_UE_stats[0];
  PHY_MEASUREMENTS_eNB *phy_measurements = &phy_vars_eNb->PHY_measurements_eNB[eNB_id];

  int aarx,rx_power_correction;
  int rx_power;
  int i;
  unsigned int limit,rb;
  int *ul_ch;

  //printf("Running eNB_srs_measurements for eNB_id %d\n",eNB_id);

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

    phy_measurements->wideband_cqi[UE_id][aarx] = phy_measurements->rx_spatial_power[UE_id][0][aarx];

  
    
  //      phy_measurements->rx_power[UE_id][aarx]/=frame_parms->nb_antennas_tx;
    phy_measurements->wideband_cqi_dB[UE_id][aarx] = (unsigned short) dB_fixed(phy_measurements->wideband_cqi[UE_id][aarx]);
    rx_power += phy_measurements->wideband_cqi[UE_id][aarx];
    //      phy_measurements->rx_avg_power_dB[UE_id] += phy_measurements->rx_power_dB[UE_id][aarx];
  }

  

  //    phy_measurements->rx_avg_power_dB[UE_id]/=frame_parms->nb_antennas_rx;
  if (init_averaging == 0)
    rx_power_avg_eNB[UE_id][eNB_id] = ((k1*rx_power_avg_eNB[UE_id][eNB_id]) + (k2*rx_power))>>10; 
  else
    rx_power_avg_eNB[UE_id][eNB_id] = rx_power;

  phy_measurements->wideband_cqi_tot[UE_id] = dB_fixed2(rx_power,2*phy_measurements->n0_power_tot);
  // times 2 since we have noise only in the odd carriers of the srs comb

  phy_measurements->rx_rssi_dBm[UE_id] = (int)dB_fixed(rx_power_avg_eNB[UE_id][eNB_id])-phy_vars_eNb->rx_total_gain_eNB_dB;
 
 
  

  for (aarx=0;aarx<frame_parms->nb_antennas_rx;aarx++) {

    
    for (rb=0;rb<frame_parms->N_RB_DL;rb++) {

      //      printf("eNB_common_vars->srs_ch_estimates[0] => %x\n",eNB_common_vars->srs_ch_estimates[0]);
      if (rb < 12)
	ul_ch    = &eNB_common_vars->srs_ch_estimates[eNB_id][aarx][frame_parms->first_carrier_offset + (rb*12)];
      else if (rb>12)
	ul_ch    = &eNB_common_vars->srs_ch_estimates[eNB_id][aarx][6 + (rb-13)*12];
      else {
	phy_measurements->subband_cqi_dB[UE_id][aarx][rb] = 0;
	continue;
      }
      // cqi
      if (aarx==0)
	phy_measurements->subband_cqi_tot[UE_id][rb]=0;
      
      phy_measurements->subband_cqi[UE_id][aarx][rb] = (signal_energy_nodc(ul_ch,12))*rx_power_correction - phy_measurements->n0_power[aarx];
      if (phy_measurements->subband_cqi[UE_id][aarx][rb] < 0)
	phy_measurements->subband_cqi[UE_id][aarx][rb]=0;

      phy_measurements->subband_cqi_tot[UE_id][rb] += phy_measurements->subband_cqi[UE_id][aarx][rb];
      phy_measurements->subband_cqi_dB[UE_id][aarx][rb] = dB_fixed2(phy_measurements->subband_cqi[UE_id][aarx][rb],
									  2*phy_measurements->n0_power[aarx]);							
      // 2*n0_power since we have noise from the odd carriers in the comb of the srs

	//	  msg("subband_cqi[%d][%d][%d] => %d (%d dB)\n",eNB_id,aarx,rb,phy_measurements->subband_cqi[eNB_id][aarx][rb],phy_measurements->subband_cqi_dB[eNB_id][aarx][rb]);
      }
      
  }


  for (rb=0;rb<frame_parms->N_RB_DL;rb++) {
    phy_measurements->subband_cqi_tot_dB[UE_id][rb] = dB_fixed2(phy_measurements->subband_cqi_tot[UE_id][rb],
								 phy_measurements->n0_power_tot);
    /*
    if (phy_measurements->subband_cqi_tot_dB[UE_id][rb] == 65)
      msg("eNB meas error *****subband_cqi_tot[%d][%d] %d => %d dB (n0 %d)\n",UE_id,rb,phy_measurements->subband_cqi_tot[UE_id][rb],phy_measurements->subband_cqi_tot_dB[UE_id][rb],phy_measurements->n0_power_tot);
    */
  }
  
}
      




  

