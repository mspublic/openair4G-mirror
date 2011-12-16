#ifndef USER_MODE
#define __NO_VERSION__

//#include "rt_compat.h"

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
int setup_regs(unsigned char card_id, LTE_DL_FRAME_PARMS *frame_parms) {

  //------------------------------------------------------------------------------


  int i;

#ifdef RTAI_ENABLED
  
#ifndef NOCARD_TEST    



  //  openair_writel(NUMBER_OF_SYMBOLS_PER_FRAME,// OFDM_SYMBOLS_PER_FRAME
  //		 OFDM_SYMBOLS_PER_FRAME_REG);  // setup PCI size

  if (vid != XILINX_VENDOR) {
    
    pci_interface[card_id]->node_id = openair_daq_vars.node_id;
    printk("[openair][INIT] DAQ_NODE_ID = %d\n",openair_daq_vars.node_id);
    
    pci_interface[card_id]->timing_advance = openair_daq_vars.timing_advance;
    //printk("[openair][INIT] Card %d TIMING_ADVANCE = %d\n",card_id,openair_daq_vars.timing_advance);
    
    pci_interface[card_id]->samples_per_frame = FRAME_LENGTH_COMPLEX_SAMPLES;
    printk("[openair][INIT] Card %d samples_per_frame = %d\n",card_id,FRAME_LENGTH_COMPLEX_SAMPLES);

    pci_interface[card_id]->frame_offset = 1;//FRAME_LENGTH_COMPLEX_SAMPLES-1;
    
    for (i=0;i<NB_ANTENNAS_RX;i++) {
      pci_interface[card_id]->adc_head[i] = (unsigned int)virt_to_phys((volatile void*)RX_DMA_BUFFER[card_id][i]);
    }
    for (i=0;i<NB_ANTENNAS_TX;i++){
      pci_interface[card_id]->dac_head[i] = (unsigned int)virt_to_phys((volatile void*)TX_DMA_BUFFER[card_id][i]);
    }
    
    pci_interface[card_id]->freq_info = openair_daq_vars.freq_info;
    //printk("[openair][INIT] freq0 = %d, freq1 = %d\n",(pci_interface[card_id]->freq_info>>1)&3,(pci_interface[card_id]->freq_info>>3)&3);
    
    
    pci_interface[card_id]->rx_rf_mode = openair_daq_vars.rx_rf_mode;
    
    //printk("[openair][INIT] rx_gain_val = %d\n",openair_daq_vars.rx_gain_val);
    pci_interface[card_id]->rx_gain_val = openair_daq_vars.rx_gain_val;
    
    pci_interface[card_id]->tcxo_dac = openair_daq_vars.tcxo_dac;
    
    //printk("[openair][INIT] Card %d tdd = %d, dual_tx = %d\n",card_id,openair_daq_vars.tdd,openair_daq_vars.dual_tx);
    pci_interface[card_id]->tdd = openair_daq_vars.tdd;

    pci_interface[card_id]->tdd_config = (unsigned int)frame_parms->tdd_config;
    
    pci_interface[card_id]->dual_tx = openair_daq_vars.dual_tx;
    
    pci_interface[card_id]->mast_flag = (card_id==0)? 1 : 0;
  }
  else {

    exmimo_pci_interface[card_id]->framing.tx_rx_switch_point = openair_daq_vars.tx_rx_switch_point;
    //printk("[openair][INIT] Card %d TX_RX_SWITCH_POINT = %d\n",card_id,openair_daq_vars.tx_rx_switch_point);
    
    exmimo_pci_interface[card_id]->framing.timing_advance = openair_daq_vars.timing_advance;
    //printk("[openair][INIT] Card %d TIMING_ADVANCE = %d\n",card_id,openair_daq_vars.timing_advance);
    
    exmimo_pci_interface[card_id]->framing.cyclic_prefix_mode  = frame_parms->Ncp;
    //printk("[openair][INIT] CYCLIC_PREFIX_LENGTH = %d\n",card_id,exmimo_pci_interface[card_id]->framing.cyclic_prefix_length);
    
    exmimo_pci_interface[card_id]->framing.log2_ofdm_symbol_size = LOG2_NUMBER_OF_OFDM_CARRIERS; 
    exmimo_pci_interface[card_id]->framing.samples_per_frame = FRAME_LENGTH_COMPLEX_SAMPLES;
    exmimo_pci_interface[card_id]->framing.frame_offset = 19;//FRAME_LENGTH_COMPLEX_SAMPLES-1;
    
    for (i=0;i<NB_ANTENNAS_RX;i++) {
      exmimo_pci_interface[card_id]->rf.adc_head[i] = (unsigned int)virt_to_phys((volatile void*)RX_DMA_BUFFER[card_id][i]);
    }
    for (i=0;i<NB_ANTENNAS_TX;i++){
      exmimo_pci_interface[card_id]->rf.dac_head[i] = (unsigned int)virt_to_phys((volatile void*)TX_DMA_BUFFER[card_id][i]);
    }
  }
#endif // RTAI_ENABLED
    
  //  printk("[openair][INIT] : Returning\n");
  return(0);
#else //NOCARD_TEST
  return(0);
#endif //NOCARD_TEST
}
