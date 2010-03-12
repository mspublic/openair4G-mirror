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
phy_adjust_gain (unsigned char clear,short coef,unsigned char chsch_ind)
{
  int i;
  short ncoef;
  unsigned int rx_power;
  static unsigned int rx_power_fil = 0;
  unsigned short rx_power_fil_dB;

  ncoef = 32767 - coef;
  
  // Find average received power across RX antennas (in dB) 
  rx_power = PHY_vars->PHY_measurements.wideband_cqi[chsch_ind][0];

  for (i=1;i<NB_ANTENNAS_RX;i++)
    if (PHY_vars->PHY_measurements.wideband_cqi[chsch_ind][i] > rx_power)
      rx_power = PHY_vars->PHY_measurements.wideband_cqi[chsch_ind][i];


  // filter rx_power to reduce measurement noise		 
  if (clear == 1)
    rx_power_fil = rx_power;
  else
    rx_power_fil = ((rx_power_fil * coef) + (rx_power * ncoef)) >> 15;

  rx_power_fil_dB = dB_fixed(rx_power_fil);

  // Gain control with hysterisis
  // Adjust gain in PHY_vars->rx_vars[0].rx_total_gain_dB

  if ( (rx_power_fil_dB < TARGET_RX_POWER - 5) && (PHY_vars->rx_vars[0].rx_total_gain_dB < MAX_RF_GAIN) )
    PHY_vars->rx_vars[0].rx_total_gain_dB+=5;
  else if ( (rx_power_fil_dB > TARGET_RX_POWER + 5) && (PHY_vars->rx_vars[0].rx_total_gain_dB > MIN_RF_GAIN) )
    PHY_vars->rx_vars[0].rx_total_gain_dB-=5;

#ifndef USER_MODE
  openair_set_rx_gain_cal_openair(PHY_vars->rx_vars[0].rx_total_gain_dB);
#endif

  //#ifdef DEBUG_PHY
  //  if (mac_xface->frame%100==0)
    msg("[PHY][ADJUST_GAIN] Frame %d, ncoef %d: wideband_cqi (%d dB, %d) clear = %d, rx_power_fil_dB = %d, rx_total_gain_dB = %d\n",mac_xface->frame,ncoef,dB_fixed(rx_power),rx_power,clear,rx_power_fil_dB,PHY_vars->rx_vars[0].rx_total_gain_dB);
  //#endif //DEBUG_PHY
	
}


