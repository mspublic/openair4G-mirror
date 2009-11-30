#include "defs.h"
#include "PHY/extern.h"

// this function fills the PHY_vars->PHY_measurement structure
int lte_ue_measurements(LTE_UE_COMMON *ue_common_vars,
			LTE_DL_FRAME_PARMS *frame_parms,
			PHY_MEASUREMENTS *phy_measurements) {

  int aarx,aatx;

  /*
  Elements of phy_measurements
  int            rx_power[NUMBER_OF_eNB_MAX][NB_ANTENNAS_RX];     //! estimated received signal power (linear)
  int            n0_power[NUMBER_OF_eNB_MAX][NB_ANTENNAS_RX];     //! estimated noise power (linear)
  unsigned short rx_power_dB[NUMBER_OF_eNB_MAX][NB_ANTENNAS_RX];  //! estimated received signal power (dB)
  unsigned short n0_power_dB[NUMBER_OF_eNB_MAX][NB_ANTENNAS_RX];  //! estimated noise power (dB)
  short          rx_avg_power_dB[NUMBER_OF_eNB_MAX];              //! estimated avg received signal power (dB)
  short		 rx_rssi_dBm[NUMBER_OF_eNB_MAX];                  //! estimated rssi (dBm)
  */

  phy_measurements->rx_avg_power_dB[0] = 0;
  for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++) {
    phy_measurements->rx_power[0][aarx] = 0;
    for (aatx=0; aatx<frame_parms->nb_antennas_tx; aatx++) {
      phy_measurements->rx_power[0][aarx] += signal_energy(ue_common_vars->dl_ch_estimates[aatx*frame_parms->nb_antennas_tx + aarx],
					 frame_parms->ofdm_symbol_size);
    }
    phy_measurements->rx_power[0][aarx]/=frame_parms->nb_antennas_tx;
    phy_measurements->rx_power_dB[0][aarx] = dB_fixed(phy_measurements->rx_power[0][aarx]);
    phy_measurements->rx_avg_power_dB[0] += phy_measurements->rx_power_dB[0][aarx];
  }
  phy_measurements->rx_avg_power_dB[0]/=frame_parms->nb_antennas_rx;
  phy_measurements->rx_rssi_dBm[0] = phy_measurements->rx_avg_power_dB[0]-PHY_vars->rx_vars[0].rx_total_gain_dB;
}
