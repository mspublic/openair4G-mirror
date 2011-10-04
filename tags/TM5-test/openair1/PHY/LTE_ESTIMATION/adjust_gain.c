#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"

#ifndef USER_MODE
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_device.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#endif

void
phy_adjust_gain (PHY_VARS_UE *phy_vars_ue, unsigned char eNB_id)
{
  int i;
  int rx_power;
  unsigned short rx_power_fil_dB;

  rx_power_fil_dB = phy_vars_ue->PHY_measurements.rx_power_avg_dB[eNB_id];

  // Gain control with hysterisis
  // Adjust gain in phy_vars_ue->rx_vars[0].rx_total_gain_dB

  if ( (rx_power_fil_dB < TARGET_RX_POWER - 5) && (phy_vars_ue->rx_total_gain_dB < MAX_RF_GAIN) )
    phy_vars_ue->rx_total_gain_dB+=5;
  else if ( (rx_power_fil_dB > TARGET_RX_POWER + 5) && (phy_vars_ue->rx_total_gain_dB > MIN_RF_GAIN) )
    phy_vars_ue->rx_total_gain_dB-=5;

  if (phy_vars_ue->rx_total_gain_dB>MAX_RF_GAIN) {
    /*
    if ((openair_daq_vars.rx_rf_mode==0) && (openair_daq_vars.mode == openair_NOT_SYNCHED)) {
      openair_daq_vars.rx_rf_mode=1;
      phy_vars_ue->rx_total_gain_dB = max(MIN_RF_GAIN,MAX_RF_GAIN-25);
    }
    else {
    */
    phy_vars_ue->rx_total_gain_dB = MAX_RF_GAIN;
  }
  else if (phy_vars_ue->rx_total_gain_dB<MIN_RF_GAIN) {
    /*
    if ((openair_daq_vars.rx_rf_mode==1) && (openair_daq_vars.mode == openair_NOT_SYNCHED)) {
      openair_daq_vars.rx_rf_mode=0;
      phy_vars_ue->rx_total_gain_dB = min(MAX_RF_GAIN,MIN_RF_GAIN+25);
    }
    else {
    */
    phy_vars_ue->rx_total_gain_dB = MIN_RF_GAIN;
  }

#ifndef USER_MODE
  for (i=0;i<number_of_cards;i++) {
    //openair_set_rx_rf_mode(i,openair_daq_vars.rx_rf_mode);
    openair_set_rx_gain_cal_openair(i,phy_vars_ue->rx_total_gain_dB);
  }
#endif

#ifdef DEBUG_PHY
  if ((mac_xface->frame%100==0) || (mac_xface->frame < 10))
    msg("[PHY][ADJUST_GAIN] frame %d, clear = %d, rx_power = %d, rx_power_fil = %d, rx_power_fil_dB = %d, coef=%d, ncoef=%d, rx_total_gain_dB = %d (%d,%d,%d)\n",
	mac_xface->frame,clear,rx_power,rx_power_fil,rx_power_fil_dB,coef,ncoef,phy_vars_ue->rx_total_gain_dB,
	TARGET_RX_POWER,MAX_RF_GAIN,MIN_RF_GAIN);
#endif //DEBUG_PHY
	
}


