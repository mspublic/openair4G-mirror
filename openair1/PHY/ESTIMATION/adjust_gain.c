#ifdef PC_TARGET
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#endif 

#ifndef USER_MODE
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_device.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#endif

void
phy_adjust_gain (unsigned char clear,short coef,unsigned char chsch_ind)
{
  int i;
  short ncoef;
  short rx_power;
  static int      rx_power_fil = 0;

  ncoef = 32767 - coef;
  
  // Find average received power across RX antennas (in dB) 
  rx_power = PHY_vars->PHY_measurements.rx_power_dB[chsch_ind][0];

  for (i=1;i<NB_ANTENNAS_RX;i++)
    if (PHY_vars->PHY_measurements.rx_power_dB[chsch_ind][i] > rx_power)
      rx_power = PHY_vars->PHY_measurements.rx_power_dB[chsch_ind][i];


  // filter rx_power to reduce measurement noise		 
  if (clear == 1)
    rx_power_fil = rx_power;
  else
    rx_power_fil = ((rx_power_fil * coef) + (rx_power * ncoef)) >> 15;

  // Gain control with hysterisis
  // Adjust gain in PHY_vars->rx_vars[0].rx_total_gain_dB

  if ( (rx_power_fil < TARGET_RX_POWER - 5) && (PHY_vars->rx_vars[0].rx_total_gain_dB < MAX_RF_GAIN) )
    PHY_vars->rx_vars[0].rx_total_gain_dB+=5;
  else if ( (rx_power_fil > TARGET_RX_POWER + 5) && (PHY_vars->rx_vars[0].rx_total_gain_dB > MIN_RF_GAIN) )
    PHY_vars->rx_vars[0].rx_total_gain_dB-=5;

  openair_set_rx_gain_cal_openair(PHY_vars->rx_vars[0].rx_total_gain_dB);

#ifdef DEBUG_PHY	
  msg("[PHY][ADJUST_GAIN] clear = %d, rx_power_fil = %d, rx_total_gain_dB = %d\n",clear,rx_power_fil,PHY_vars->rx_vars[0].rx_total_gain_dB);
#endif //DEBUG_PHY
	
}

void
phy_adjust_gain_mesh (unsigned char clear,short coef)
{
  int i,aa;
  short ncoef;
  short rx_power[NUMBER_OF_CHSCH_SYMBOLS_MAX];
  short rx_power_dB[NUMBER_OF_CHSCH_SYMBOLS_MAX];
  static int rx_power_fil[NUMBER_OF_CHSCH_SYMBOLS_MAX];
  short maxVal = 0;
  short minVal = 0x7FFF;
  int maxIdx = 1;
  int minIdx = 1;
  
  ncoef = 32767 - coef;

  // Find average received power across RX antennas (in dB) 
  for (i=0; i<NUMBER_OF_CHSCH_SYMBOLS_MAX; i++) {
    rx_power_dB[i] = PHY_vars->PHY_measurements.rx_power_dB[i][0];
    for (aa=1;aa<NB_ANTENNAS_RX;aa++)
      if (rx_power_dB[i] < PHY_vars->PHY_measurements.rx_power_dB[i][aa])
	rx_power_dB[i] = PHY_vars->PHY_measurements.rx_power_dB[i][aa];
    

    //rx_power_dB[i] = dB_fixed(rx_power[i]);
  }

  // filter rx_power to reduce measurement noise		 
  for (i=0; i<NUMBER_OF_CHSCH_SYMBOLS_MAX; i++) {
    if (clear == 1)
      rx_power_fil[i] = rx_power_dB[i];
    else
      rx_power_fil[i] = ((rx_power_fil[i] * coef) + (rx_power_dB[i] * ncoef)) >> 15;
  }

#ifdef DEBUG_PHY
  msg("[PHY][ADJUST_GAIN] rx_power_fil = %d %d %d %d\n",
      rx_power_fil[0],
      rx_power_fil[1],
      rx_power_fil[2],
      rx_power_fil[3]);
#endif //DEBUG_PHY

  // sort the powers
  for (i=1; i<NUMBER_OF_CHSCH_SYMBOLS_MAX-1; i++) {
    if (rx_power_fil[i] < minVal && rx_power_fil[i] > TARGET_RX_POWER_MIN) {
      minVal = rx_power_fil[i];
      minIdx = i;
    }    
    if (rx_power_fil[i] > maxVal) {
      maxVal = rx_power_fil[i];
      maxIdx = i;
    }
  }


  // Gain control with hysterisis
  // Adjust gain in PHY_vars->rx_vars[0].rx_total_gain_dB

  if ( (rx_power_fil[minIdx] < TARGET_RX_POWER - 5) && 
       (PHY_vars->rx_vars[0].rx_total_gain_dB < MAX_RF_GAIN) && 
       (rx_power_fil[maxIdx] < TARGET_RX_POWER_MAX))
    PHY_vars->rx_vars[0].rx_total_gain_dB+=5;
  else if ( ((rx_power_fil[minIdx] > TARGET_RX_POWER + 5) || (rx_power_fil[maxIdx] > TARGET_RX_POWER_MAX)) && 
	    (PHY_vars->rx_vars[0].rx_total_gain_dB > MIN_RF_GAIN)  )
    PHY_vars->rx_vars[0].rx_total_gain_dB-=5;

  /*
  if (rx_gain == RX_VGC_GAIN_MAX)
		rx_gain--;
  else if (rx_gain == 0)
    rx_gain++;
  */

  openair_set_rx_gain_cal_openair(PHY_vars->rx_vars[0].rx_total_gain_dB);

#ifdef DEBUG_PHY	
  msg("[PHY][ADJUST_GAIN] clear = %d, CHSCH minIdx=%d (%d dB), CHSCH maxIdx=%d (%d dB), rx_total_gain_dB = %d\n",
      clear,
      minIdx, rx_power_fil[minIdx],
      maxIdx, rx_power_fil[maxIdx],
      PHY_vars->rx_vars[0].rx_total_gain_dB);
#endif //DEBUG_PHY
	
}
