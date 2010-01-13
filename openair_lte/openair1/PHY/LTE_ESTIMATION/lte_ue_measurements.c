#include "defs.h"
#include "PHY/extern.h"

// this function fills the PHY_vars->PHY_measurement structure
int lte_ue_measurements(LTE_UE_COMMON *ue_common_vars,
			LTE_DL_FRAME_PARMS *frame_parms,
			PHY_MEASUREMENTS *phy_measurements,
			unsigned int subframe_offset) {

  int aarx,aatx,eNb_id=0,rx_power_correction;
  int rx_power[3], n0_power;

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
    rx_power[eNb_id] = 0;
  }

  if ( (frame_parms->ofdm_symbol_size == 128) ||
       (frame_parms->ofdm_symbol_size == 512) )
    rx_power_correction = 2;
  else
    rx_power_correction = 1;

  for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++) {

    for (eNb_id=0;eNb_id<3;eNb_id++) 
      phy_measurements->rx_power[eNb_id][aarx] = 0;

    for (aatx=0; aatx<frame_parms->nb_antennas_tx; aatx++) {
      for (eNb_id=0;eNb_id<3;eNb_id++) {
	phy_measurements->rx_spatial_power[eNb_id][aatx][aarx] = signal_energy(&ue_common_vars->dl_ch_estimates[eNb_id][(aatx*frame_parms->nb_antennas_tx) + aarx][4],frame_parms->N_RB_DL*12)*rx_power_correction;
	phy_measurements->rx_spatial_power_dB[eNb_id][aatx][aarx] = dB_fixed(phy_measurements->rx_spatial_power[eNb_id][aatx][aarx]);
      
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
    phy_measurements->rx_avg_power_dB[eNb_id]=dB_fixed(rx_power[eNb_id]);
    phy_measurements->rx_rssi_dBm[eNb_id] = phy_measurements->rx_avg_power_dB[eNb_id]-PHY_vars->rx_vars[0].rx_total_gain_dB;

  }

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
