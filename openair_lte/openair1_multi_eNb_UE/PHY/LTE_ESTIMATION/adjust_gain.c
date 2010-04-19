#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"

#ifndef USER_MODE
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_device.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#endif

void
phy_adjust_gain (unsigned char clear,short coef,unsigned char chsch_ind,PHY_VARS_UE *phy_vars_ue)
{
  int i;
  int ncoef;
  int rx_power;
  static int rx_power_fil = 0;
  unsigned short rx_power_fil_dB;

  ncoef = 1024 - coef;
  
  // Find average received power across RX antennas (in dB) 
  rx_power = phy_vars_ue->PHY_measurements.wideband_cqi[chsch_ind][0];

  for (i=1;i<NB_ANTENNAS_RX;i++)
    if (phy_vars_ue->PHY_measurements.wideband_cqi[chsch_ind][i] > rx_power)
      rx_power = phy_vars_ue->PHY_measurements.wideband_cqi[chsch_ind][i];


  // filter rx_power to reduce measurement noise		 
  if (clear == 1)
    rx_power_fil = rx_power;
  else
    rx_power_fil = ((rx_power_fil * coef) + (rx_power * ncoef)) >> 10;

  rx_power_fil_dB = dB_fixed(rx_power_fil);

  // Gain control with hysterisis
  // Adjust gain in PHY_vars->rx_vars[0].rx_total_gain_dB

  if ( (rx_power_fil_dB < TARGET_RX_POWER - 5) && (phy_vars_ue->rx_total_gain_dB < MAX_RF_GAIN) )
    phy_vars_ue->rx_total_gain_dB+=5;
  else if ( (rx_power_fil_dB > TARGET_RX_POWER + 5) && (phy_vars_ue->rx_total_gain_dB > MIN_RF_GAIN) )
    phy_vars_ue->rx_total_gain_dB-=5;

#ifndef USER_MODE
  openair_set_rx_gain_cal_openair(phy_vars_ue->rx_total_gain_dB);
#endif

#ifdef DEBUG_PHY
  if ((mac_xface->frame%100==0) || (mac_xface->frame < 10))
    msg("[PHY][ADJUST_GAIN] frame %d, clear = %d, rx_power = %d, rx_power_fil = %d, rx_power_fil_dB = %d, coef=%d, ncoef=%d, rx_total_gain_dB = %d (%d,%d,%d)\n",
	mac_xface->frame,clear,rx_power,rx_power_fil,rx_power_fil_dB,coef,ncoef,phy_vars_ue->rx_total_gain_dB,
	TARGET_RX_POWER,MAX_RF_GAIN,MIN_RF_GAIN);
#endif //DEBUG_PHY
	
}


