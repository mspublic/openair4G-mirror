#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"

#ifdef CBMIMO1
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_device.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#endif

#ifdef EXMIMO
#include "ARCH/CBMIMO1/DEVICE_DRIVER/cbmimo1_device.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/defs.h"
#include "ARCH/CBMIMO1/DEVICE_DRIVER/extern.h"
#endif

void
phy_adjust_gain (PHY_VARS_UE *phy_vars_ue, u8 eNB_id) {

  u16 rx_power_fil_dB;
#ifdef CBMIMO1
  int i;
#endif

  rx_power_fil_dB = dB_fixed(phy_vars_ue->PHY_measurements.rssi);//phy_vars_ue->PHY_measurements.rx_power_avg_dB[eNB_id];

  // Gain control with hysterisis
  // Adjust gain in phy_vars_ue->rx_vars[0].rx_total_gain_dB

  if ( (rx_power_fil_dB < TARGET_RX_POWER - 5) && (phy_vars_ue->rx_total_gain_dB < MAX_RF_GAIN) )
    phy_vars_ue->rx_total_gain_dB+=5;
  else if ( (rx_power_fil_dB > TARGET_RX_POWER + 5) && (phy_vars_ue->rx_total_gain_dB > MIN_RF_GAIN) )
    phy_vars_ue->rx_total_gain_dB-=5;

#ifdef CBMIMO1
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


  for (i=0;i<number_of_cards;i++) {
    //openair_set_rx_rf_mode(i,openair_daq_vars.rx_rf_mode);
    openair_set_rx_gain_cal_openair(i,phy_vars_ue->rx_total_gain_dB);
  }
#else
#ifdef EXMIMO

  switch (phy_vars_ue->rx_gain_mode[0]) {
  case max_gain:
    if (phy_vars_ue->rx_total_gain_dB>phy_vars_ue->rx_gain_max[0]) {
      phy_vars_ue->rx_total_gain_dB = phy_vars_ue->rx_gain_max[0];
      exmimo_pci_interface->rf.rx_gain00 = 50;
      exmimo_pci_interface->rf.rx_gain10 = 50;
    }
    /*
    else if (phy_vars_ue->rx_total_gain_dB<(phy_vars_ue->rx_gain_max[0]-30)) {
      phy_vars_ue->rx_gain_mode[0] = byp;
      phy_vars_ue->rx_gain_mode[1] = byp;
      exmimo_pci_interface->rf.rf_mode0 = 22991;
      exmimo_pci_interface->rf.rf_mode1 = 22991;

      if (phy_vars_ue->rx_total_gain_dB<(phy_vars_ue->rx_gain_byp[0]-30)) {
	exmimo_pci_interface->rf.rx_gain00 = 0;
	exmimo_pci_interface->rf.rx_gain10 = 0;
      }
    }
    */
    else {
      exmimo_pci_interface->rf.rx_gain00 = 50 - phy_vars_ue->rx_gain_max[0] + phy_vars_ue->rx_total_gain_dB;
      exmimo_pci_interface->rf.rx_gain10 = 50 - phy_vars_ue->rx_gain_max[1] + phy_vars_ue->rx_total_gain_dB;
    }
    break;
  case med_gain:
  case byp_gain:
    if (phy_vars_ue->rx_total_gain_dB>phy_vars_ue->rx_gain_byp[0]) {
      phy_vars_ue->rx_gain_mode[0]   = max_gain;
      phy_vars_ue->rx_gain_mode[1]   = max_gain;
      exmimo_pci_interface->rf.rf_mode0 = 55759;
      exmimo_pci_interface->rf.rf_mode1 = 55759;
 
      if (phy_vars_ue->rx_total_gain_dB>phy_vars_ue->rx_gain_max[0]) {
	exmimo_pci_interface->rf.rx_gain00 = 50;
	exmimo_pci_interface->rf.rx_gain10 = 50;
      }
      else {
	exmimo_pci_interface->rf.rx_gain00 = 50 - phy_vars_ue->rx_gain_max[0] + phy_vars_ue->rx_total_gain_dB;
	exmimo_pci_interface->rf.rx_gain10 = 50 - phy_vars_ue->rx_gain_max[1] + phy_vars_ue->rx_total_gain_dB;
      }
    }
    else if (phy_vars_ue->rx_total_gain_dB<(phy_vars_ue->rx_gain_byp[0]-50)) {
	exmimo_pci_interface->rf.rx_gain00 = 0;
	exmimo_pci_interface->rf.rx_gain10 = 0;
      }
    else {
      exmimo_pci_interface->rf.rx_gain00 = 50 - phy_vars_ue->rx_gain_byp[0] + phy_vars_ue->rx_total_gain_dB;
      exmimo_pci_interface->rf.rx_gain10 = 50 - phy_vars_ue->rx_gain_byp[1] + phy_vars_ue->rx_total_gain_dB;
    }
    break;
  default:
    exmimo_pci_interface->rf.rx_gain00 = 50;
    exmimo_pci_interface->rf.rx_gain10 = 50;
    break;
  }

#endif
#endif

#ifdef DEBUG_PHY
  if ((mac_xface->frame%100==0) || (mac_xface->frame < 10))
    msg("[PHY][ADJUST_GAIN] frame %d, clear = %d, rx_power = %d, rx_power_fil = %d, rx_power_fil_dB = %d, coef=%d, ncoef=%d, rx_total_gain_dB = %d (%d,%d,%d)\n",
	mac_xface->frame,clear,rx_power,rx_power_fil,rx_power_fil_dB,coef,ncoef,phy_vars_ue->rx_total_gain_dB,
	TARGET_RX_POWER,MAX_RF_GAIN,MIN_RF_GAIN);
#endif //DEBUG_PHY
	
}


