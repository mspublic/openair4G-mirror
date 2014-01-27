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


  if (vid != XILINX_VENDOR) {
    
    for (i=0;i<NB_ANTENNAS_RX;i++) {
      pci_interface[card_id]->adc_head[i] = (unsigned int)virt_to_phys((volatile void*)RX_DMA_BUFFER[card_id][i]);
    }
    for (i=0;i<NB_ANTENNAS_TX;i++){
      pci_interface[card_id]->dac_head[i] = (unsigned int)virt_to_phys((volatile void*)TX_DMA_BUFFER[card_id][i]);
    }
    
#ifndef FW2011
    pci_interface[card_id]->ofdm_symbols_per_frame = NUMBER_OF_SYMBOLS_PER_FRAME;
    //printk("[openair][INIT] NUMBER_OF_SYMBOLS_PER_FRAME = %d\n",pci_interface[card_id]->ofdm_symbols_per_frame);
    pci_interface[card_id]->log2_ofdm_symbol_size = LOG2_NUMBER_OF_OFDM_CARRIERS; 
    pci_interface[card_id]->cyclic_prefix_length  = CYCLIC_PREFIX_LENGTH;
    //printk("[openair][INIT] CYCLIC_PREFIX_LENGTH = %d\n",card_id,pci_interface[card_id]->cyclic_prefix_length);
#endif
    
    pci_interface[card_id]->samples_per_frame = FRAME_LENGTH_COMPLEX_SAMPLES;
    printk("[openair][INIT] samples_per_frame = %d\n",pci_interface[card_id]->samples_per_frame);
    
#ifndef FW2011
    pci_interface[card_id]->tx_rx_switch_point = openair_daq_vars.tx_rx_switch_point;
#else
    pci_interface[card_id]->tdd_config = frame_parms->tdd_config;
#endif
    
    pci_interface[card_id]->timing_advance = openair_daq_vars.timing_advance;
    
    pci_interface[card_id]->dual_tx = frame_parms->dual_tx;
    pci_interface[card_id]->tdd     = frame_parms->frame_type;
    pci_interface[card_id]->node_id = frame_parms->node_id;
    printk("[openair][INIT] node_id %d, dual_tx %d, tdd %d, tdd_config %d\n",frame_parms->node_id, frame_parms->dual_tx, frame_parms->frame_type, frame_parms->tdd_config );

    
    pci_interface[card_id]->freq_info = openair_daq_vars.freq_info;
    //printk("[openair][INIT] freq0 = %d, freq1 = %d\n",(pci_interface[card_id]->freq_info>>1)&3,(pci_interface[card_id]->freq_info>>3)&3);
    
    pci_interface[card_id]->rx_rf_mode = openair_daq_vars.rx_rf_mode;
    
    pci_interface[card_id]->rx_gain_val = openair_daq_vars.rx_gain_val;
    
    pci_interface[card_id]->tcxo_dac = openair_daq_vars.tcxo_dac;
    
    pci_interface[card_id]->mast_flag = (card_id==0)? 1 : 0;
    
  }
  else {
    
    exmimo_pci_interface->framing.tx_rx_switch_point = openair_daq_vars.tx_rx_switch_point;
    //    printk("[openair][INIT] Card %d TX_RX_SWITCH_POINT = %d\n",card_id,openair_daq_vars.tx_rx_switch_point);
    
    exmimo_pci_interface->framing.timing_advance = openair_daq_vars.timing_advance;
    //    printk("[openair][INIT] Card %d TIMING_ADVANCE = %d\n",card_id,openair_daq_vars.timing_advance);
    
    exmimo_pci_interface->framing.cyclic_prefix_mode  = frame_parms->Ncp;
    exmimo_pci_interface->framing.log2_ofdm_symbol_size = LOG2_NUMBER_OF_OFDM_CARRIERS; 
    exmimo_pci_interface->framing.samples_per_frame = FRAME_LENGTH_COMPLEX_SAMPLES;
    exmimo_pci_interface->framing.frame_offset = 19;//FRAME_LENGTH_COMPLEX_SAMPLES-1;
    
    
    for (i=0;i<NB_ANTENNAS_RX;i++) {
      exmimo_pci_interface->rf.adc_head[i] = (unsigned int)virt_to_phys((volatile void*)RX_DMA_BUFFER[card_id][i]);
      printk("exmimo_pci_interface->rf.adc_head[%d] = %x\n",i,exmimo_pci_interface->rf.adc_head[i]);
    }
    for (i=0;i<NB_ANTENNAS_TX;i++){
      exmimo_pci_interface->rf.dac_head[i] = (unsigned int)virt_to_phys((volatile void*)TX_DMA_BUFFER[card_id][i]);
    }

    printk("Freq %d,%d,%d,%d, Gain %d,%d,%d,%d\n",
	   frame_parms->carrier_freq[0],frame_parms->carrier_freq[1],frame_parms->carrier_freq[2],frame_parms->carrier_freq[3],
	   frame_parms->rxgain[0],frame_parms->rxgain[1],frame_parms->rxgain[2],frame_parms->rxgain[3]);
    exmimo_pci_interface->rf.rf_freq_rx0          = frame_parms->carrier_freq[0];
    exmimo_pci_interface->rf.rx_gain00            = frame_parms->rxgain[0];
    exmimo_pci_interface->rf.rf_freq_rx1          = frame_parms->carrier_freq[1];
    exmimo_pci_interface->rf.rx_gain10            = frame_parms->rxgain[1];
    exmimo_pci_interface->rf.rf_freq_rx2          = frame_parms->carrier_freq[2];
    exmimo_pci_interface->rf.rx_gain20            = frame_parms->rxgain[2];
    exmimo_pci_interface->rf.rf_freq_rx3          = frame_parms->carrier_freq[3];
    exmimo_pci_interface->rf.rx_gain30            = frame_parms->rxgain[3];
  }
#endif // RTAI_ENABLED
    
  //  printk("[openair][INIT] : Returning\n");
  return(0);
#else //NOCARD_TEST
  return(0);
#endif //NOCARD_TEST
}