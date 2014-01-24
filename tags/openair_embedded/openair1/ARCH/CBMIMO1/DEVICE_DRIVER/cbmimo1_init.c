#ifndef USER_MODE
#define __NO_VERSION__

//#include "rt_compat.h"

#ifdef RTAI_ENABLED
#include <rtai.h>
//#include <rtai_posix.h>
#include <rtai_fifos.h>
#endif //RTAI_ENABLED


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
#include "SCHED/defs.h"
#include "SCHED/extern.h"

#include "from_grlib_softconfig.h"
#include "from_grlib_softregs.h"

#ifdef RTAI_ENABLED
#include "PHY/defs.h"
#include "PHY/extern.h"
#endif //RTAI_ENABLED

/*
#ifdef RTAI_ENABLED
//------------------------------------------------------------------------------
int create_rt_fifo(int rt_fifoP, int sizeP) {
  //------------------------------------------------------------------------------
  rtf_destroy(rt_fifoP);
  switch (rtf_create(rt_fifoP, sizeP)) {
  case -ENODEV:
    printk("[WCDMA][ERROR] create_rt_fifo() %d fifo is greater than or equal to RTF_NO\n", rt_fifoP);
    return  -ENODEV;
    break;
  case -EBUSY:
    printk("[WCDMA][ERROR] create_rt_fifo() %d fifo is already in use. Choose a different ID\n", rt_fifoP);
    return  -EBUSY;
    break;
  case -ENOMEM:
    printk("[WCDMA][ERROR] create_rt_fifo() %d bytes could not be allocated for the RT-FIFO %d\n", sizeP, rt_fifoP);
    return  -ENOMEM;
    break;
  case 0:
    printk("[WCDMA] RT-FIFO %d CREATED\n", rt_fifoP);
    rtf_flush(rt_fifoP);
    return rt_fifoP; // not necessary, but...
    break;
  default:
    printk("[WCDMA] create_rt_fifo() returned ???\n");
    return -1;
  }
}

#endif //RTAI_ENABLED
*/

//------------------------------------------------------------------------------
int setup_regs() {

  //------------------------------------------------------------------------------


  int i;

#ifdef RTAI_ENABLED
  
#ifndef NOCARD_TEST    



  //  openair_writel(NUMBER_OF_SYMBOLS_PER_FRAME,// OFDM_SYMBOLS_PER_FRAME
  //		 OFDM_SYMBOLS_PER_FRAME_REG);  // setup PCI size

//  if (openair_daq_vars.tx_test==0)
    pci_interface->ofdm_symbols_per_frame = NUMBER_OF_SYMBOLS_PER_FRAME;
//  else
//    pci_interface->ofdm_symbols_per_frame = 16;

//  printk("[openair][INIT] NUMBER_OF_SYMBOLS_PER_FRAME = %d\n",pci_interface->ofdm_symbols_per_frame);

//  printk("[openair][INIT] DAQ_NODE_ID = %d\n",openair_daq_vars.node_id);

  pci_interface->node_id = openair_daq_vars.node_id;

  //  printk("[openair][INIT] TX_RX_SWITCH_POINT = %d\n",openair_daq_vars.tx_rx_switch_point);

  pci_interface->tx_rx_switch_point = openair_daq_vars.tx_rx_switch_point;
  pci_interface->timing_advance = openair_daq_vars.timing_advance;

  pci_interface->cyclic_prefix_length  = CYCLIC_PREFIX_LENGTH;
  pci_interface->log2_ofdm_symbol_size = LOG2_NUMBER_OF_OFDM_CARRIERS; 
  pci_interface->samples_per_frame = FRAME_LENGTH_COMPLEX_SAMPLES;
  pci_interface->frame_offset = FRAME_LENGTH_COMPLEX_SAMPLES-1;

  for (i=0;i<NB_ANTENNAS_RX;i++) {
    pci_interface->adc_head[i] = (unsigned int)virt_to_phys((volatile void*)RX_DMA_BUFFER[i]);
  }
  for (i=0;i<NB_ANTENNAS_TX;i++){
    pci_interface->dac_head[i] = (unsigned int)virt_to_phys((volatile void*)TX_DMA_BUFFER[i]);
  }

  pci_interface->freq_info = openair_daq_vars.freq_info;
  //printk("[openair][INIT] freq0 = %d, freq1 = %d\n",(pci_interface->freq_info>>1)&3,(pci_interface->freq_info>>3)&3);

  pci_interface->rx_rf_mode = openair_daq_vars.rx_rf_mode;

  pci_interface->rx_gain_val = openair_daq_vars.rx_gain_val;

  pci_interface->tcxo_dac = openair_daq_vars.tcxo_dac;

#endif // RTAI_ENABLED
    
  //  printk("[openair][INIT] : Returning\n");
  return(0);
#else //NOCARD_TEST
  return(0);
#endif //NOCARD_TEST
}