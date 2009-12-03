#ifndef USER_MODE
#define __NO_VERSION__

//#include "rt_compat.h"

#ifdef RTAI_ENABLED
#include <rtai.h>
//#include <rtai_posix.h>
#include <rtai_fifos.h>

#endif


#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <asm/segment.h>
#include <asm/page.h>
#include <asm/delay.h>

#include <linux/init.h>
#include <linux/module.h>
//#include <linux/malloc.h>
#endif


#include "cbmimo1_device.h"
#include "defs.h"
#include "extern.h"
#include "cbmimo1_pci.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "SCHED/defs.h"
#include "SCHED/extern.h"


void openair_set_rx_rf_mode(unsigned int arg) {

#ifndef NOCARD_TEST
  printk("[openair][RF_CNTL] Setting RX_RF MODE to %x\n",arg);

  if (pci_interface)
    pci_interface->rx_rf_mode = arg;



  //  openair_dma(SET_RX_RF_MODE);
#endif

}





void openair_set_tcxo_dac(unsigned int arg) {

#ifndef NOCARD_TEST  
  printk("[openair][RF_CNTL] Setting TCXO_DAC to %x\n",arg);

  openair_daq_vars.tcxo_dac = arg;
  //  openair_writel(arg,bar[0]+REG_BAR+0x4);
  // PA Gain control line is connected to TCXO tuning frequency input
  //  openair_dma(SET_PA_GAIN);
#endif

}

void openair_set_tx_gain_openair(unsigned char txgain00,unsigned char txgain10,unsigned char txgain01, unsigned char txgain11) {

#ifndef NOCARD_TEST
  printk("[openair][RF_CNTL] Setting TX gains to %d,%d,%d,%d\n",txgain00,txgain10,txgain01,txgain11);

  if (pci_interface) {
    pci_interface->tx_gain00 = (unsigned int)txgain00;
    pci_interface->tx_gain01 = (unsigned int)txgain01;
    pci_interface->tx_gain10 = (unsigned int)txgain10;
    pci_interface->tx_gain11 = (unsigned int)txgain11;
  }
  //  openair_writel((unsigned int)txgain00,bar[0]+REG_BAR+0x4);
  //  openair_writel((unsigned int)txgain10,bar[0]+REG_BAR+0x8);
  //  openair_writel((unsigned int)txgain01,bar[0]+REG_BAR+0xc);
  //  openair_writel((unsigned int)txgain11,bar[0]+REG_BAR+0x10);
  //  openair_dma(SET_TX_GAIN);
#endif

} 
 
void openair_set_rx_gain_openair(unsigned char rxgain00,unsigned char rxgain01,unsigned char rxgain10,unsigned char rxgain11) {

#ifndef NOCARD_TEST
  unsigned int rxgain;

  // Concatenate the 4 gain values into one 32-bit register (flip byte endian)

  rxgain = rxgain00 | (rxgain01 << 8) | (rxgain10 << 16) | (rxgain11 << 24);
  //  printk("[openair][RF_CNTL**] Setting RX gains to %d,%d,%d,%d -> %x\n",rxgain00,rxgain01,rxgain10,rxgain11,rxgain);

  // Store the result in shared PCI memory so that the FPGA can detect and read the new value
  openair_daq_vars.rx_gain_val  = rxgain;



#endif
}

/*
void openair_set_rx_gain_cal_openair(unsigned int gain_dB) {

#ifndef NOCARD_TEST

  printk("[openair][RF_CNTL] Setting RX gains to %d dB \n",gain_dB);



  
  // Store the result in shared PCI memory so that the FPGA can detect and read the new value
  if (pci_interface) {
		pci_interface->rx_gain_cval  = gain_dB;
  }
  else
    printk("[openair][RF_CNTL] rxgainreg not configured\n");


#endif
}
*/
void openair_set_lo_freq_openair(char freq0,char freq1) {
#ifndef NOCARD_TEST
  printk("[openair][RF_CNTL] Setting LO frequencies to %d,%d\n",freq0,freq1);
  //  openair_writel(freq0,bar[0]+0x4);
  //  openair_writel(freq1,bar[0]+0x8);
  //  openair_dma(SET_LO_FREQ);
  openair_daq_vars.freq_info = 1 + (freq0<<1) + (freq1<<4);
  pci_interface->freq_info = openair_daq_vars.freq_info;
#endif

}










