#include "defs.h"
#include "PHY/extern.h"

// this function fills the PHY_vars->PHY_measurement structure

#define k1 31
#define k2 (32-k1)

int rx_power_avg[3];

int lte_ue_measurements(LTE_UE_COMMON *ue_common_vars,
			LTE_DL_FRAME_PARMS *frame_parms,
			PHY_MEASUREMENTS *phy_measurements,
			unsigned int subframe_offset,
			unsigned char N0_symbol,
			unsigned char init_averaging) {

  int aarx,aatx,eNb_id=0,rx_power_correction;
  int rx_power[3], n0_power;
  int i;

  /*
    Elements of phy_measurements
    int            rx_power[NUMBER_OF_eNB_MAX][NB_ANTENNAS_RX];     //! estimated received signal power (linear)
    int            n0_power[NUMBER_OF_eNB_MAX][NB_ANTENNAS_RX];     //! estimated noise power (linear)
    unsigned short rx_power_dB[NUMBER_OF_eNB_MAX][NB_ANTENNAS_RX];  //! estimated received signal power (dB)
    unsigned short n0_power_dB[NUMBER_OF_eNB_MAX][NB_ANTENNAS_RX];  //! estimated noise power (dB)
    short          rx_avg_power_dB[NUMBER_OF_eNB_MAX];              //! estimated avg received signal power (dB)
    short		 rx_rssi_dBm[NUMBER_OF_eNB_MAX];                  //! estimated rssi (dBm)
  */

  for (eNb_id=0;eNb_id<3;eNb_id++) {
    phy_measurements->rx_avg_power_dB[eNb_id] = 0;
    if (init_averaging == 1)
      rx_power_avg[eNb_id] = 0;
    rx_power[eNb_id] = 0;
  }

  if ( (frame_parms->ofdm_symbol_size == 128) ||
       (frame_parms->ofdm_symbol_size == 512) )
    rx_power_correction = 2;
  else
    rx_power_correction = 1;

  for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++) {

    for (eNb_id=0;eNb_id<3;eNb_id++) {
      phy_measurements->rx_power[eNb_id][aarx] = 0;
    }
    if (N0_symbol == 0) {
      phy_measurements->n0_power_dB[aarx] = -105 + PHY_vars->rx_vars[0].rx_total_gain_dB;
    }      
    for (aatx=0; aatx<frame_parms->nb_antennas_tx; aatx++) {
      for (eNb_id=0;eNb_id<3;eNb_id++) {
	  
	phy_measurements->rx_spatial_power[eNb_id][aatx][aarx] = signal_energy(&ue_common_vars->dl_ch_estimates[eNb_id][(aatx*frame_parms->nb_antennas_tx) + aarx][8],(frame_parms->N_RB_DL*12)-8)*rx_power_correction;
	  
	  
	  
	phy_measurements->rx_spatial_power_dB[eNb_id][aatx][aarx] = dB_fixed(phy_measurements->rx_spatial_power[eNb_id][aatx][aarx]);
	/*	
	  printf("lte_ue_measurements: aarx %d aatx %d eNb %d: %d (%d dB)\n",
	  aarx,aatx,eNb_id,phy_measurements->rx_spatial_power[eNb_id][aatx][aarx],
	  phy_measurements->rx_spatial_power_dB[eNb_id][aatx][aarx]);
	*/
	phy_measurements->rx_power[eNb_id][aarx] += phy_measurements->rx_spatial_power[eNb_id][aatx][aarx];
      }
    }

    for (eNb_id = 0; eNb_id < 3; eNb_id++){
      //      phy_measurements->rx_power[eNb_id][aarx]/=frame_parms->nb_antennas_tx;
      phy_measurements->rx_power_dB[eNb_id][aarx] = dB_fixed(phy_measurements->rx_power[eNb_id][aarx]);
      rx_power[eNb_id] += phy_measurements->rx_power[eNb_id][aarx];
      //      phy_measurements->rx_avg_power_dB[eNb_id] += phy_measurements->rx_power_dB[eNb_id][aarx];
    }
  }

  for (eNb_id = 0; eNb_id < 3; eNb_id++){
    //    phy_measurements->rx_avg_power_dB[eNb_id]/=frame_parms->nb_antennas_rx;
    rx_power_avg[eNb_id] = ((k1*rx_power_avg[eNb_id]) + (k2*rx_power[eNb_id]))>>5;
    phy_measurements->rx_avg_power_dB[eNb_id]=dB_fixed(rx_power_avg[eNb_id]);

    phy_measurements->wideband_sinr[eNb_id] = phy_measurements->rx_avg_power_dB[eNb_id] - phy_measurements->n0_power_dB[0];


    phy_measurements->rx_rssi_dBm[eNb_id] = phy_measurements->rx_avg_power_dB[eNb_id]-PHY_vars->rx_vars[0].rx_total_gain_dB;

    for (i=0;i<15;i++)
      //      printf("lte_cqi_snr_dB[%d]=%d\n",i,lte_cqi_snr_dB[i]);
      if (lte_cqi_snr_dB[i] > (char)phy_measurements->wideband_sinr[eNb_id]) {
	phy_measurements->wideband_cqi[eNb_id] = i-2;
	break;
      }
 
    //    printf("lte_ue_measurements: rx_power_dB[%d] %d (%f), sinr %d,cqi %d\n",eNb_id,phy_measurements->rx_avg_power_dB[eNb_id],10*log10(rx_power[eNb_id]),phy_measurements->wideband_sinr[eNb_id],phy_measurements->wideband_cqi[eNb_id]);
  }

  /*
    write_output("dlsch00_ch0.m","dl00_ch0",&(lte_ue_common_vars->dl_ch_estimates[0][0][0]),(6*(lte_frame_parms->ofdm_symbol_size)),1,1);

    write_output("dlsch01_ch0.m","dl01_ch0",&(lte_ue_common_vars->dl_ch_estimates[0][1][0]),(6*(lte_frame_parms->ofdm_symbol_size)),1,1);
    write_output("dlsch10_ch0.m","dl10_ch0",&(lte_ue_common_vars->dl_ch_estimates[0][2][0]),(6*(lte_frame_parms->ofdm_symbol_size)),1,1);
    write_output("dlsch11_ch0.m","dl11_ch0",&(lte_ue_common_vars->dl_ch_estimates[0][3][0]),(6*(lte_frame_parms->ofdm_symbol_size)),1,1);

    write_output("dlsch00_ch1.m","dl00_ch1",&(lte_ue_common_vars->dl_ch_estimates[1][0][0]),(6*(lte_frame_parms->ofdm_symbol_size)),1,1);

    write_output("dlsch01_ch1.m","dl01_ch1",&(lte_ue_common_vars->dl_ch_estimates[1][1][0]),(6*(lte_frame_parms->ofdm_symbol_size)),1,1);
    write_output("dlsch10_ch1.m","dl10_ch1",&(lte_ue_common_vars->dl_ch_estimates[1][2][0]),(6*(lte_frame_parms->ofdm_symbol_size)),1,1);
    write_output("dlsch11_ch1.m","dl11_ch1",&(lte_ue_common_vars->dl_ch_estimates[1][3][0]),(6*(lte_frame_parms->ofdm_symbol_size)),1,1);
  */
  //    exit(-1);

  // noise measurements
  // for the moment we measure the noise on the second OFDM symbol. This has to be changed later.
  n0_power = 0;
  for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++) {
    phy_measurements->n0_power[aarx] = signal_energy(&ue_common_vars->rxdata[aarx][frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples],frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples);
    phy_measurements->n0_power_dB[aarx] = dB_fixed(phy_measurements->n0_power[aarx]);
    n0_power +=  phy_measurements->n0_power[aarx];
  }
  phy_measurements->n0_avg_power_dB = dB_fixed(n0_power);

}
