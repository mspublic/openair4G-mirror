
#include "comp_def.h"
#include "ublaze_defs.h"
#include "ublaze_extern.h"
#include "SPI.h"

#include "pci_commands.h"




void PCI_Handler(void * baseaddr_p) {
  
  Xuint32 Command, baseaddr, IntrStatus,cmd;
  int adac_config_reg;
  int fft_scale0,fft_scale1;
  char tmp0, tmp1;
  UBLAZE_MAC_XFACE *ublaze_mac_xface_pci;
  char get_signal_flag=0;

  // FOR DEBUG
  //  XIo_Out32(XPAR_PCI_REG_0_BASEADDR+0x30, PCI_handler_debug_id);  

  baseaddr = (Xuint32) baseaddr_p;

  IntrStatus = PCI_REG_mReadReg(baseaddr, PCI_REG_INTR_DISR_OFFSET);
  PCI_REG_mWriteReg(baseaddr, PCI_REG_INTR_DISR_OFFSET, IntrStatus);



  Command = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG0_OFFSET);

  switch ( Command ) {


  case START_1ARY_CLUSTER_HEAD:

    DestinationADC0 = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG1_OFFSET);
    SourceDAC0 = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG2_OFFSET);
    OFDM_SYMBOLS_PER_FRAME = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG3_OFFSET);
    freq_corr = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG7_OFFSET)&1;
    rf_freq0 = (PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG7_OFFSET)>>1)&3;
    rf_freq1 = (PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG7_OFFSET)>>3)&3;
    ADAC_CNT_PTR  = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG9_OFFSET);
    if (ADAC_CNT_PTR != 0)
      rxgain = (RXGAIN_t *)pci_to_opb(ADAC_CNT_PTR + 4);



#ifndef PC_DSP
    ublaze_mac_xface_pci = (UBLAZE_MAC_XFACE *)(pci_to_opb(PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG4_OFFSET)));
    /*
    ublaze_memcpy((unsigned int*)ublaze_mac_xface_pci,
		  (unsigned int*)ublaze_mac_xface,
		  sizeof(UBLAZE_MAC_XFACE) ); // len_bytes
    */
 
    ublaze_mac_xface->mac_chbch_data_tx[0].data = ublaze_mac_xface_pci->mac_chbch_data_tx[0].data;
    ublaze_mac_xface->mac_chbch_data_tx[0].num_bytes = ublaze_mac_xface_pci->mac_chbch_data_tx[0].num_bytes;

    XIo_Out32(XPAR_PCI_REG_0_BASEADDR + PCI_REG_SLAVE_REG4_OFFSET, ublaze_mac_xface_pci);
    XIo_Out32(XPAR_PCI_REG_0_BASEADDR + PCI_REG_SLAVE_REG6_OFFSET, pci_to_opb(ublaze_mac_xface->mac_chbch_data_tx[0].data));
    XIo_Out32(XPAR_PCI_REG_0_BASEADDR + PCI_REG_SLAVE_REG12_OFFSET, ublaze_mac_xface->mac_chbch_data_tx[0].num_bytes);
#else
    tx_rx_switch_point = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG4_OFFSET);
#endif // PC_DSP
    
#ifndef SIMULATION 
    Program_ADACs(freq_corr);
    Program_PLLs(rf_freq0,0);
    Program_PLLs(rf_freq1,1);
#endif SIMULATION

    //    led_output_pattern=0;
    //    write_LED(0xc);

    SECONDARY_CH_INDEX = 0;
    node_id = PRIMARY_CH;
    chbch_enabled = 1;
    signal_clear_ADC = 1;
    tx_test_enable = 0;
    freq_corr = 1;
    mchrach_enabled[0] = 0;
    mchrach_enabled[1] = 0;
    in_sync = 0;//1;             // remove later
#ifdef PC_DSP
    rt_acquisition=1;
#else
    rt_acquisition=0;
#endif //PC_DSP
    get_frame=0;
    meas_freq = 0;
    break;


  case START_RT_ACQUISITION:


    DestinationADC0        = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, ADC0_HEAD);
    SourceDAC0             = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, DAC0_HEAD);
    OFDM_SYMBOLS_PER_FRAME = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, OFDM_SYMBOLS_PER_FRAME_REG);
    tx_rx_switch_point     = 48;//PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, TX_RX_SWITCH);
    node_id                = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, NODE_REG);
    ADAC_CNT_PTR           = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, MBOX);

    //#ifdef SIMULATION
    XIo_Out32(XPAR_PCI_REG_0_BASEADDR + PCI_REG_SLAVE_REG14_OFFSET, tx_rx_switch_point);  // debug (REMOVE LATER)
    XIo_Out32(XPAR_PCI_REG_0_BASEADDR + PCI_REG_SLAVE_REG13_OFFSET, OFDM_SYMBOLS_PER_FRAME);  // debug (REMOVE LATER)
    //#endif
    
    if (ADAC_CNT_PTR != 0)
      rxgain = (RXGAIN_t *)pci_to_opb(ADAC_CNT_PTR + 4);

    rf_freq0 = (PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, FREQ_CORR)>>1)&3;
    rf_freq1 = (PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, FREQ_CORR)>>3)&3;


#ifndef SIMULATION
    Program_ADACs(0);
    //    Program_PLLs(rf_freq0,0);
    //    Program_PLLs(rf_freq1,1);
#endif

    get_frame = 0;
    rt_acquisition = 1;
    sync_done = 0;

    ADC_addr_dump_frame = 0;

    error_cnt = 0;
#ifdef OFDM_COPRO_DMA
    dma0_error_cnt=0;
    dma1_error_cnt=0;
#endif
    // For GET_FRAME to PCI
    dest0_base_addr = pci_to_opb(DestinationADC0);
    src0_base_addr  = pci_to_opb(SourceDAC0);

    // For GET_FRAME to SDRAM
    //    dest0_base_addr = XPAR_OPB_SDRAM_0_BASEADDR;
    //    dest1_base_addr = XPAR_OPB_SDRAM_0_BASEADDR+(OFDM_SYMBOLS_PER_FRAME*288*4);


    signal_clear_ADC = 1;
    tx_test_enable = 0;
    freq_corr = 1;
    chbch_enabled = 0;
    mchrach_enabled[0] = 0;
    mchrach_enabled[1] = 0;

    led_output_pattern = 1;
    led_cnt = 0;
    led_period = 128;
    led_word = 128;
    //    write_LED(0xc);

    central_dma_status = 0;
    break;

  case START_2ARY_CLUSTER_HEAD:

    DestinationADC0        = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG1_OFFSET);
    SourceDAC0             = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG2_OFFSET);
    OFDM_SYMBOLS_PER_FRAME = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG3_OFFSET);
    SECONDARY_CH_INDEX     = 1; //PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG6_OFFSET);
    frame_offset_symbols   = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG7_OFFSET)+1;
    frame_offset           = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG8_OFFSET);
    ADAC_CNT_PTR  = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG9_OFFSET);
    if (ADAC_CNT_PTR != 0)
      rxgain = (RXGAIN_t *)pci_to_opb(ADAC_CNT_PTR + 4);

    XIo_Out32(XPAR_OFDM_COPRO_0_BASEADDR + OFDM_COPRO_SLAVE_REG13_OFFSET, (frame_offset - 1));    // FRAME_START
    // Set mode to Normal RX (i.e. cyclic prefix removal)
    config_val =0x83f000e0;
    XIo_Out32( XPAR_OFDM_COPRO_0_BASEADDR + OFDM_COPRO_SLAVE_REG10_OFFSET, config_val);  // log2_symbol_size(0:3),

    //    freq_corr = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG7_OFFSET);
    //    Program_ADACs(freq_corr);

    //    clear_signal_buffers();
    dest0_base_addr = XPAR_PCI_IPIFBAR_0 + DestinationADC0;
    src0_base_addr = XPAR_PCI_IPIFBAR_0 + SourceDAC0;

    node_id = SECONDARY_CH;

    // signal_clear_ADC = 1;
    in_sync = 1;
    chbch_enabled = 1;  // remove later
    tx_test_enable = 0;
    freq_corr = 1;
    mchrach_enabled[0] = 0;
    mchrach_enabled[1] = 0;

    led_output_pattern=1;
    write_LED(0x8);
    break;


  case ADJUST_SYNCH:

    frame_offset           = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG8_OFFSET);

    XIo_Out32(XPAR_OFDM_COPRO_0_BASEADDR + OFDM_COPRO_SLAVE_REG13_OFFSET, frame_offset);    // FRAME_START

    break;

  case DMA_STOP :

    //    DestinationADC0        = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG1_OFFSET);
    //    SourceDAC1        = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG2_OFFSET);
    //    OFDM_SYMBOLS_PER_FRAME = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG3_OFFSET);

    DestinationADC0        = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, ADC0_HEAD);
    SourceDAC0             = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, DAC0_HEAD);
    OFDM_SYMBOLS_PER_FRAME = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, OFDM_SYMBOLS_PER_FRAME_REG);
    tx_rx_switch_point     = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, TX_RX_SWITCH);
    node_id                = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, NODE_REG);
    ADAC_CNT_PTR           = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, ADAC_CNT);

    config_val = 0x83f000c0;
    XIo_Out32( XPAR_OFDM_COPRO_0_BASEADDR + OFDM_COPRO_SLAVE_REG10_OFFSET, config_val); // writing ADAC config reg

    continuous_receive = 0;
    chbch_detected = 0;

    
#ifndef PC_DSP    
    ublaze_mac_xface_pci = (UBLAZE_MAC_XFACE *)(pci_to_opb(PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG4_OFFSET)));
    /*
    ublaze_memcpy((unsigned int*)ublaze_mac_xface_pci,
		  (unsigned int*)ublaze_mac_xface,
		  sizeof(UBLAZE_MAC_XFACE) ); // len_bytes
    */

    ublaze_mac_xface->mac_chbch_data_tx[0].data = ublaze_mac_xface_pci->mac_chbch_data_tx[0].data;
    ublaze_mac_xface->mac_chbch_data_tx[0].num_bytes = ublaze_mac_xface_pci->mac_chbch_data_tx[0].num_bytes;

    XIo_Out32(XPAR_PCI_REG_0_BASEADDR + PCI_REG_SLAVE_REG4_OFFSET, ublaze_mac_xface_pci);
    XIo_Out32(XPAR_PCI_REG_0_BASEADDR + PCI_REG_SLAVE_REG6_OFFSET, pci_to_opb(ublaze_mac_xface->mac_chbch_data_tx[0].data));
    XIo_Out32(XPAR_PCI_REG_0_BASEADDR + PCI_REG_SLAVE_REG12_OFFSET, (int)(ublaze_mac_xface->mac_chbch_data_tx[0].num_bytes));

#endif PC_DSP

    frame_offset_symbols   = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG7_OFFSET) +1;
    frame_offset           = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG8_OFFSET);
    ADAC_CNT_PTR  = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG9_OFFSET);
    if (ADAC_CNT_PTR != 0) {
      rxgain = (RXGAIN_t *)pci_to_opb(ADAC_CNT_PTR + 4);
    }
    // Remove later !
    XIo_Out32(XPAR_PCI_REG_0_BASEADDR + PCI_REG_SLAVE_REG4_OFFSET, rxgain->val);//ublaze_mac_xface_pci);
    XIo_Out32(XPAR_PCI_REG_0_BASEADDR + PCI_REG_SLAVE_REG6_OFFSET, gain_tab[rxgain->val].input_level_dBm_0);//pci_to_opb(ublaze_mac_xface->mac_chbch_data_tx[0].data));
    XIo_Out32(XPAR_PCI_REG_0_BASEADDR + PCI_REG_SLAVE_REG12_OFFSET,gain_tab[rxgain->val].gain1_0);
#ifdef OFDM_COPRO_DMA
    XIo_Out32(XPAR_PCI_REG_0_BASEADDR + DMA_ERROR_CNT,(dma0_error_cnt&0xffff) + ((dma1_error_cnt&0xffff) << 16));
#endif
    mchrach_enabled[0] = 0;
    mchrach_enabled[1] = 0;
    chbch_enabled = 0;
    in_sync=0;
    tx_test_enable = 0;
    led_output_pattern = 1;
    led_cnt = 0;
    led_period = 128;
    led_word = 128;
    rt_acquisition=0;
    get_frame=0;
    central_dma_status = 0;
    break;


  case GET_FRAME:    // for Time/Freq Synch procedure
    
    DestinationADC0 = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG1_OFFSET);
    SourceDAC1      = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG2_OFFSET);
    OFDM_SYMBOLS_PER_FRAME = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG3_OFFSET);
    ADAC_CNT_PTR  = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG9_OFFSET);
    if (ADAC_CNT_PTR != 0)
      rxgain = (RXGAIN_t *)pci_to_opb(ADAC_CNT_PTR + 4);
    rf_freq0 = (PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG7_OFFSET)>>1)&3;
    rf_freq1 = (PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG7_OFFSET)>>3)&3;


#ifndef SIMULATION
    Program_ADACs(1);
    Program_PLLs(rf_freq0,0);
    Program_PLLs(rf_freq1,1);
#endif

    get_frame = 1;
    //    start = 3;
    signal_DumpFrame = 1;
    signal_DumpFrame_DONE = 0;

    signal_clear_ADC = 1;
    tx_test_enable = 0;
    freq_corr = 1;
    chbch_enabled = 0;

    // For GET_FRAME to PCI
    dest0_base_addr = XPAR_PCI_IPIFBAR_0 + DestinationADC0;
    src0_base_addr = XPAR_PCI_IPIFBAR_0 + SourceDAC0;

    mchrach_enabled[0] = 0;
    mchrach_enabled[1] = 0;
    break;

#ifndef PC_DSP
  case PHASE_ESTIMATE:    // Phase estimation
    
    DestinationADC0 = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG1_OFFSET);
    SourceDAC0      = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG2_OFFSET);
    OFDM_SYMBOLS_PER_FRAME = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG3_OFFSET);
    ADAC_CNT_PTR  = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG9_OFFSET);
    if (ADAC_CNT_PTR != 0)
      rxgain = (RXGAIN_t *)pci_to_opb(ADAC_CNT_PTR + 4);
    rf_freq0 = (PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG7_OFFSET)>>1)&3;
    rf_freq1 = (PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG7_OFFSET)>>3)&3;    
    freq_corr = 1;
    
#ifndef SIMULATION
    Program_ADACs(freq_corr);
    Program_PLLs(rf_freq0,0);
    Program_PLLs(rf_freq1,1);
#endif
    
    meas_freq = 1;
    signal_DumpFrame = 1;
    signal_DumpFrame_DONE = 0;
    Nframe = 0;
    //    res = 0;
    
    signal_clear_ADC = 1;
    tx_test_enable = 0;
    freq_corr = 1;
    chbch_enabled = 0;
    mchrach_enabled[0] = 0;
    mchrach_enabled[1] = 0;
    break;

    
  case DO_CHBCH_SYNCH :
    

    DestinationADC0 = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG1_OFFSET);
    SourceDAC0      = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG2_OFFSET);
    OFDM_SYMBOLS_PER_FRAME = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG3_OFFSET);
    ADAC_CNT_PTR  = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG9_OFFSET);
    if (ADAC_CNT_PTR != 0)
      rxgain = (RXGAIN_t *)pci_to_opb(ADAC_CNT_PTR + 4);
    rf_freq0 = (PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG7_OFFSET)>>1)&3;
    rf_freq1 = (PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG7_OFFSET)>>3)&3;
    freq_corr = 1;
    continuous_receive = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG5_OFFSET);
    
    
#ifndef SIMULATION
    ublaze_mac_xface_pci = (UBLAZE_MAC_XFACE *)(pci_to_opb(PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG4_OFFSET)));
    
    ublaze_memcpy((unsigned int*)ublaze_mac_xface_pci,
		  (unsigned int*)ublaze_mac_xface,
		  sizeof(UBLAZE_MAC_XFACE) ); // len_bytes
    
    Program_ADACs(freq_corr);
    Program_PLLs(rf_freq0,0);
    Program_PLLs(rf_freq1,1);
#endif // SIMULATION
    
    get_frame = 1;
    signal_DumpFrame = 1;
    signal_DumpFrame_DONE = 0;
    signal_DumpFrame2sdram = 1;
    signal_DumpFrame2MEM = 0;
    do_synchronization = 0;
    frame_length_samples = OFDM_SYMBOLS_PER_FRAME*OFDM_SYMBOL_LENGTH_WITH_PREFIX_SAMPLES;
    synch_done_samples = 0;
    synch_DMAs = 0;
    chbch_detected = 0;

#ifdef SIMULATION
    //    in_sync = 1;             // remove (for Simulation Only!)
    continuous_receive = 1;  // remove (for Simulation Only!)
#endif // SIMULATION
      
    first_sync = 0;
    signal_clear_ADC = 1;
    tx_test_enable = 0;
    chbch_enabled = 0;
    mchrach_enabled[0] = 0;
    mchrach_enabled[1] = 0;
    led_output_pattern=0;

    break;


  case SET_FFT_SCALE :
    
    fft_scale0 = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG1_OFFSET);
    fft_scale1 = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG2_OFFSET);
    
    FFT_SCALE = (fft_scale1<<8) + fft_scale0;
    
    
    break;



  case GET_SIGNALS :
    
    DestinationADC0 = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG1_OFFSET);
    OFDM_SYMBOLS_PER_FRAME = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG3_OFFSET);
    ADAC_CNT_PTR  = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG9_OFFSET);
    if (ADAC_CNT_PTR != 0)
      rxgain = (RXGAIN_t *)pci_to_opb(ADAC_CNT_PTR + 4);
    get_signal_flag = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG2_OFFSET);

    switch (get_signal_flag) {
    case 0 :
      get_synch_symbols = 1;
      break;
    case 1 :
      get_channel_estimate = 1;
      break;
    case 2 :
      get_estimated_data = 1;
      break;
    case 3 :
      get_decoded_data = 1;
      break;
    default :
      get_synch_symbols = 1;
      break;
    }
    
    get_signal = 1;
    symbol_offset = 0;
    dump_data = 0;
    do_dmas = 0;
    
    signal_clear_ADC = 1;
    tx_test_enable = 0;
    freq_corr = 1;
    chbch_enabled = 0;
    mchrach_enabled[0] = 0;
    mchrach_enabled[1] = 0;
    break;


  case FFT_TEST :
    DestinationADC0 = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG1_OFFSET);
    OFDM_SYMBOLS_PER_FRAME = 16; //PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG3_OFFSET);
    ADAC_CNT_PTR  = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG9_OFFSET);
    if (ADAC_CNT_PTR != 0)
      rxgain = (RXGAIN_t *)pci_to_opb(ADAC_CNT_PTR + 4);
    fft_scale0 = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG4_OFFSET);
    fft_scale1 = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG5_OFFSET);
    
    tmp0 = ((((char*)&fft_scale0)[0]&3)<<6) + ((((char*)&fft_scale0)[1]&3)<<4) + ((((char*)&fft_scale0)[2]&3)<<2) + (((char*)&fft_scale0)[3]&3);

    tmp1 = ((((char*)&fft_scale1)[0]&3)<<6) + ((((char*)&fft_scale1)[1]&3)<<4) + ((((char*)&fft_scale1)[2]&3)<<2) + (((char*)&fft_scale1)[3]&3);

    FFT_SCALE = (tmp1<<8) + tmp0;

    XIo_Out32(XPAR_PCI_REG_0_BASEADDR + PCI_REG_SLAVE_REG5_OFFSET, FFT_SCALE);  // remove later!
    
    fft_opt_test = 1;
    fft_opt_DONE = 0;

    signal_clear_ADC = 1;
    tx_test_enable = 0;
    freq_corr = 1;
    chbch_enabled = 0;
    mchrach_enabled[0] = 0;
    mchrach_enabled[1] = 0;
    break;

#endif // PC_DSP

  case SET_TX_GAIN :

    tx_gain00 = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG1_OFFSET);
    tx_gain10 = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG2_OFFSET);
    tx_gain01 = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG3_OFFSET);
    tx_gain11 = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG4_OFFSET);


    signal_tx_gains = 1;

    /*
    switch (tx_gain0) {
      
    case 0 :
      writegain(VGC_2394_0,TX_GAIN_0_dBm);
      break;
    case 5 :
      writegain(VGC_2394_0,TX_GAIN_5_dBm);
      break;
    case 10 :
      writegain(VGC_2394_0,TX_GAIN_10_dBm);
      break;
    case 15 :
      writegain(VGC_2394_0,TX_GAIN_15_dBm);
      break;
    case 20 :
      writegain(VGC_2394_0,TX_GAIN_20_dBm);
      break;
    case 25 :
      writegain(VGC_2394_0,TX_GAIN_25_dBm);
      break;
    default:
      writegain(VGC_2394_0,TX_GAIN_OFF);
      break;

    }
    switch (tx_gain1) {
      
    case 0 :
      writegain(VGC_2394_1,TX_GAIN_0_dBm);
      break;
    case 5 :
      writegain(VGC_2394_1,TX_GAIN_5_dBm);
      break;
    case 10 :
      writegain(VGC_2394_1,TX_GAIN_10_dBm);
      break;
    case 15 :
      writegain(VGC_2394_1,TX_GAIN_15_dBm);
      break;
    case 20 :
      writegain(VGC_2394_1,TX_GAIN_20_dBm);
      break;
    case 25 :
      writegain(VGC_2394_1,TX_GAIN_25_dBm);
      break;
    default:
      writegain(VGC_2394_1,TX_GAIN_OFF);
      break;
    }

    */
    break;
        
  case SET_RX_GAIN :
    /*
    rx_gain00 = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG1_OFFSET);
    rx_gain10 = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG2_OFFSET);
    rx_gain01 = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG3_OFFSET);
    rx_gain11 = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG4_OFFSET);
    
    signal_rx_gains = 1;


    
    switch (rx_gain) {
      
    case 0 :
      writegain(VGC_2391_0,RX_GAIN_0_dBm);
      break;
    case 5 :
      writegain(VGC_2391_0,RX_GAIN_5_dBm);
      break;
    case 10 :
      writegain(VGC_2391_0,RX_GAIN_10_dBm);
      break;
    case 15 :
      writegain(VGC_2391_0,RX_GAIN_15_dBm);
      break;
    case 20 :
      writegain(VGC_2391_0,RX_GAIN_20_dBm);
      break;
    case 25 :
      writegain(VGC_2391_0,RX_GAIN_25_dBm);
      break;
    default:
      writegain(VGC_2391_0,RX_GAIN_OFF);
      break;
    break;

    switch (tx_gain) {
      
    case 0 :
      writegain(VGC_2391_1,RX_GAIN_0_dBm);
      break;
    case 5 :
      writegain(VGC_2391_1,RX_GAIN_5_dBm);
      break;
    case 10 :
      writegain(VGC_2391_1,RX_GAIN_10_dBm);
      break;
    case 15 :
      writegain(VGC_2391_1,RX_GAIN_15_dBm);
      break;
    case 20 :
      writegain(VGC_2391_1,RX_GAIN_20_dBm);
      break;
    case 25 :
      writegain(VGC_2391_1,RX_GAIN_25_dBm);
      break;
    default:
      writegain(VGC_2391_1,RX_GAIN_OFF);
      break;
    */

    break;


  case SET_PA_GAIN :

    tx_gain00 = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG1_OFFSET);
    tx_gain10 = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG2_OFFSET);
    
    if ((tx_gain00 < 256) && (tx_gain00>=0))
      writegain(VGC_PA_0,tx_gain00);
#ifndef SPI_BLOCK
    if ((tx_gain10 < 256) && (tx_gain10>=0))
      writegain(VGC_PA_1,tx_gain10);
#endif
    
    break;


  case SET_RX_RF_MODE:
    // retrieves a 32-bit word (4 8-bit fields) from PC in REG1 containing MODE BITS for RX Chipsets
    // Encoding: |OPCTL 1|CONFIG 1|OPCTL 0|CONFIG 0|
    rx_rf_mode = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG1_OFFSET);
    set_rx_rf_mode(rx_rf_mode&3);


   break;

  case SET_LO_FREQ :

    // retrieves two 32-bit words (lower 2-bits used in each) from PC in REG1 and REG2 containing |xxxxxxxx|freq|
    // freq : 0 = channel 0, 1 = channel 1 , ....

    rf_freq0 = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG1_OFFSET);
    rf_freq1 = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG2_OFFSET);

    if ((rf_freq0 >= 0) && (rf_freq0<4))
      Program_PLLs(rf_freq0,0);
    if ((rf_freq1 >= 0) && (rf_freq1<4))
      Program_PLLs(rf_freq1,1);

    break;

  case GENERATE_FS4_TONE :

    
    generate_fs4_tone();
    freq_corr = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG7_OFFSET);
    Program_ADACs(freq_corr);

    tx_test_enable = 1;
    signal_clear_ADC = 1;
    OFDM_SYMBOLS_PER_FRAME=16;
    freq_corr = 1;

    write_LED(0x3);
    led_output_pattern=0;
    break;

  case GENERATE_FS4_TONE_WITH_DC :

    
//    generate_fs4_tone_with_dc();
    freq_corr = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG7_OFFSET);
    Program_ADACs(freq_corr);

    tx_test_enable = 1;
    signal_clear_ADC = 1;
    OFDM_SYMBOLS_PER_FRAME=16;
    freq_corr = 1;

    write_LED(0x3);
    led_output_pattern=0;
    break;

  case GENERATE_OFDM_SIGNAL :



    freq_corr = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG7_OFFSET);
    generate_ofdm_signal();
    Program_ADACs(freq_corr);

    tx_test_enable = 1;
    signal_clear_ADC = 1;
    OFDM_SYMBOLS_PER_FRAME=16;
    freq_corr = 1;
    write_LED(0x2);
    led_output_pattern=0;
    break;

  case GENERATE_IQ_IMPULSES :
    generate_iq_impulses();
    freq_corr = 0;
    tx_test_enable = 1;
    signal_clear_ADC = 1;
    OFDM_SYMBOLS_PER_FRAME=16;

  case GENERATE_QAM16_SIGNAL :

    freq_corr = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG7_OFFSET);
    Program_ADACs(freq_corr);

    tx_test_enable = 1;
    signal_clear_ADC = 1;
    OFDM_SYMBOLS_PER_FRAME=16;
    freq_corr = 1;
    break;

  case GENERATE_QPSK_SIGNAL :

    freq_corr = PCI_REG_mReadReg(XPAR_PCI_REG_0_BASEADDR, PCI_REG_SLAVE_REG7_OFFSET);
    Program_ADACs(freq_corr);

    tx_test_enable = 1;
    signal_clear_ADC = 1;
    OFDM_SYMBOLS_PER_FRAME=16;
    freq_corr = 1;
    break;

  default:
    break;

  }
}





void OFDM_COPRO_Handler(void * baseaddr_p) {

  Xuint32 ofdm_copro_intr_status, user_logic_intr_status;

#ifdef OFDM_COPRO_DMA
  Xuint32 dma_intr_status;
#endif  
  // FOR DEBUG
  //  XIo_Out32(XPAR_PCI_REG_0_BASEADDR+0x30, OFDM_handler_debug_id);  

  
  // SPI_BLOCK slv_reg8(8) start'low
  XIo_Out8(XPAR_SPI_0_BASEADDR + SPI_SLAVE_REG8_OFFSET + 1, 0x00);  // truns off spi writes


  ofdm_copro_intr_status = XIo_In32( XPAR_OFDM_COPRO_0_BASEADDR + OFDM_COPRO_INTR_DISR_OFFSET );



  if ( (ofdm_copro_intr_status&INTR_IPIR_MASK) == INTR_IPIR_MASK ) {  
    // ***************************
    // interrupt from ADAC_counter
    // ***************************

    user_logic_intr_status = XIo_In32( XPAR_OFDM_COPRO_0_BASEADDR + OFDM_COPRO_INTR_ISR_OFFSET );

    if ( (user_logic_intr_status&1) == 0x00000001 ) {
      /*
      adac_cnt++;
      adac_cnt_corrected = adac_cnt - frame_offset_symbols;
      */

      adac_cnt_corrected = ((XIo_In32(XPAR_OFDM_COPRO_0_BASEADDR + OFDM_COPRO_SLAVE_REG9_OFFSET)&(0x01ff0000))>>16);
      
      //XIo_Out32 ( XPAR_PCI_REG_0_BASEADDR + PCI_REG_SLAVE_REG14_OFFSET, adac_cnt_corrected);

#ifdef SIMULATION
      adac_cnt_mod_frame_corrected = (adac_cnt_corrected+SIMULATION_TIME_OFFSET)&(OFDM_SYMBOLS_PER_FRAME-1);
#else
      adac_cnt_mod_frame_corrected = (adac_cnt_corrected)&(OFDM_SYMBOLS_PER_FRAME-1);
#endif

      synch_cnt = (synch_cnt+1)&31;
      
      XIo_Out32( XPAR_PCI_REG_0_BASEADDR + PCI_REG_SLAVE_REG15_OFFSET, adac_cnt_mod_frame_corrected );

      if (do_synchronization==1)  // Synch mode
	fft_time = synch_cnt;
      else
	fft_time = adac_cnt_mod_frame_corrected;
      
      signal_new_symbol = 1;      
      
    }  // end of ADAC_counter interrupt


    // *************************************
    // interrupt from OFDM_COPRO_Interleaver
    // *************************************
    if ( (user_logic_intr_status&2) == 0x00000002 ) {
      
      //reseting encoder fifo
      
      // OFDM_COPRO RST_FIFO'high
      XIo_Out32(XPAR_OFDM_COPRO_0_BASEADDR + OFDM_COPRO_SLAVE_REG7_OFFSET, 0x00000002);
      
      // OFDM_COPRO RST_FIFO'low
      //      XIo_Out32(XPAR_OFDM_COPRO_0_BASEADDR + OFDM_COPRO_SLAVE_REG7_OFFSET, 0x00000000);
      
      
      //reseting encoder
      
      // OFDM_COPRO RST_encoder'high
      XIo_Out32(XPAR_OFDM_COPRO_0_BASEADDR + OFDM_COPRO_SLAVE_REG7_OFFSET, 0x00000004);
      
      // OFDM_COPRO RST_encoder'low
      //      XIo_Out32(XPAR_OFDM_COPRO_0_BASEADDR + OFDM_COPRO_SLAVE_REG7_OFFSET, 0x00000000);
      
      
      signal_generate_interleaving_DONE = 1;
      
    }  // end of OFDM_COPRO_interleaver interrupt

    
    // **********************************
    // interrupt caused by OFDM_COPRO_FFT
    // **********************************
    if ( (user_logic_intr_status&4) == 0x00000004 ) {
      
      signal_generate_fft_DONE = 1;

    }  // end of OFDM_COPRO_FFT interrupt

        
    // ***************************************
    // interrupt caused by OFDM_COPRO_DECODER
    // ***************************************
    if ( (user_logic_intr_status&8) == 0x00000008 ) {
                                                                                                       
      signal_generate_decoding_DONE = 1;
    }  // end of OFDM_COPRO_DECODER interrupt

    
    //clearing interrupt bit
    XIo_Out32( XPAR_OFDM_COPRO_0_BASEADDR + OFDM_COPRO_INTR_ISR_OFFSET, user_logic_intr_status );
    
  }  // end of USER LOGIC interrupt

#ifdef OFDM_COPRO_DMA 
  else if ( (ofdm_copro_intr_status&INTR_TERR_MASK) == INTR_TERR_MASK ) {  

    dma1_error_cnt++;
    //clearing interrupt bit
    XIo_Out32(XPAR_OFDM_COPRO_0_BASEADDR + OFDM_COPRO_INTR_DISR_OFFSET, ofdm_copro_intr_status);

  }

  else if ( (ofdm_copro_intr_status&INTR_DPTO_MASK) == INTR_DPTO_MASK ) {  

    dma0_error_cnt++;
    //clearing interrupt bit
    XIo_Out32(XPAR_OFDM_COPRO_0_BASEADDR + OFDM_COPRO_INTR_DISR_OFFSET, ofdm_copro_intr_status);

  }

  /*
  else if ((ofdm_copro_intr_status&INTR_DMA0_MASK) == INTR_DMA0_MASK) {

  /////////////////////////////////////////
  // interrupt from DMA0 controller
  /////////////////////////////////////////
    dma_intr_status = OFDM_COPRO_mReadReg(XPAR_OFDM_COPRO_0_BASEADDR, OFDM_COPRO_DMA0_ISR_OFFSET);

    if ( OFDM_COPRO_mDMA0Done(XPAR_OFDM_COPRO_0_BASEADDR)) {
    
      //clearing interrupt bit
      XIo_Out32( XPAR_OFDM_COPRO_0_BASEADDR + OFDM_COPRO_DMA0_ISR_OFFSET, dma_intr_status );
      
      disable_OFDM_COPRO();
      
    }  // end of DMA0 interrupt
  
    else if ( OFDM_COPRO_mDMA0Error(XPAR_OFDM_COPRO_0_BASEADDR)) {
      //clearing interrupt bit
      XIo_Out32( XPAR_OFDM_COPRO_0_BASEADDR + OFDM_COPRO_DMA0_ISR_OFFSET, dma_intr_status );
      dma0_error_cnt++;
      disable_OFDM_COPRO();
    }
  }
  else if ((ofdm_copro_intr_status&INTR_DMA1_MASK) == INTR_DMA1_MASK) {
    /////////////////////////////////////////
    // interrupt from DMA1 controller
    /////////////////////////////////////////

    dma_intr_status = OFDM_COPRO_mReadReg(XPAR_OFDM_COPRO_0_BASEADDR, OFDM_COPRO_DMA1_ISR_OFFSET);

    if ( OFDM_COPRO_mDMA1Done(XPAR_OFDM_COPRO_0_BASEADDR)) {
    
      //clearing interrupt bit
      XIo_Out32( XPAR_OFDM_COPRO_0_BASEADDR + OFDM_COPRO_DMA1_ISR_OFFSET, dma_intr_status );
      
      disable_OFDM_COPRO();
      
    }  // end of DMA0 interrupt
  
    else if ( OFDM_COPRO_mDMA1Error(XPAR_OFDM_COPRO_0_BASEADDR)) {
      //clearing interrupt bit
      XIo_Out32( XPAR_OFDM_COPRO_0_BASEADDR + OFDM_COPRO_DMA1_ISR_OFFSET, dma_intr_status );
      dma1_error_cnt++;
      disable_OFDM_COPRO();
    }

*/
#endif OFDM_COPRO_DMA




}




void Central_DMA_Handler(void * baseaddr_p) {

  Xuint32 baseaddr, dma_intr_status, dma_status_reg;
  baseaddr = (Xuint32) baseaddr_p;


  // SPI_BLOCK slv_reg8(8) start'low
  XIo_Out8(XPAR_SPI_0_BASEADDR + SPI_SLAVE_REG8_OFFSET + 1, 0x00);  // truns off spi writes

  dma_status_reg = XIo_In32( XPAR_OPB_CENTRAL_DMA_0_BASEADDR + 0x14 );
  dma_intr_status = XIo_In32( XPAR_OPB_CENTRAL_DMA_0_BASEADDR + 0x2c );
  

  if ( dma_intr_status == 0x00000001 ) {

    XIo_Out32( XPAR_OPB_CENTRAL_DMA_0_BASEADDR + 0x2c, dma_intr_status );


    ///////////////////
    // Start Encoding
    ///////////////////

    if ( transfer_MAC_data == 1 ) {
      signal_start_encoding = 1;
      transfer_MAC_data = 0;
    }
    
    if ( DMA2_flag == 1 ) {
      DMA2_flag = 0;
      launch_DMA2 = 1;
    }
    
    //    disable_OFDM_COPRO();
    central_dma_status = 0;
  }

  else if ( dma_intr_status == 0x00000003 ) {  // DMA end with an error

    // here codes to deal with error conditions

    error_cnt++;
    XIo_Out32(XPAR_PCI_REG_0_BASEADDR + PCI_REG_SLAVE_REG11_OFFSET, error_cnt);
    
    XIo_Out32( XPAR_OPB_CENTRAL_DMA_0_BASEADDR + 0x2c, dma_intr_status );

    //    disable_OFDM_COPRO();
    central_dma_status = 0;
    
  }

}

#ifdef PCI_DMA
void PCIV3Bridge_Handler() {
   
  Xuint32 Core_IntrStatus, DMA_IntrStatus;
   
  Core_IntrStatus = XIo_In32( XPAR_PCI_BASEADDR );
  DMA_IntrStatus = XIo_In32(XPAR_PCI_DMA_BASEADDR + 0x2c );
   
  
  ////////////////////////////////////////////
  // interrupt from PCI local DMA controller
  ////////////////////////////////////////////
  
  // DMA finished successfully!
  if ( DMA_IntrStatus == 0x00000001 ) {
    XIo_Out32( XPAR_PCI_DMA_BASEADDR + 0x2c, DMA_IntrStatus );

    disable_OFDM_COPRO();
    
    if ( DMA2_flag == 1 ) {
      DMA2_flag = 0;
      //      launch_DMA2 = 1;

      enable_OFDM_COPRO(4);
      dma_pci( DMA2_addr, DMA2_dest_addr, DMA2_length_bytes,OPB2PCI);
    }
    
       
  } else if ( DMA_IntrStatus == 0x00000003 ) {
    XIo_Out32( XPAR_PCI_DMA_BASEADDR + 0x2c, DMA_IntrStatus );
    
    // here codes to deal with Error conditions
    error_cnt++;
    XIo_Out32(XPAR_PCI_REG_0_BASEADDR + PCI_REG_SLAVE_REG11_OFFSET, error_cnt);    
  }
  
  XIo_Out32( XPAR_PCI_BASEADDR, Core_IntrStatus );  // clearing core interrupt status!
  
}
#endif
