#ifndef OPENAIR_PCI_H
#define OPENAIR_PCI_H

/* This file is intended for prototypes & declarations to be SHARED with underlying hardware. */

#define PRIMARY_CH 0
#define SECONDARY_CH 1
#define NODE 2

#define UNCALIBRATED 0
#define CALIBRATED 1

typedef struct  {
  int adc_head[2];            /// PCI addresses of ADC buffers in PC memory (Read by LEON during init)
  int dac_head[2];            /// PCI addresses of DAC buffers in PC memory (Read by LEON during init)
  int samples_per_frame;      /// Length of frame in samples
  int timing_advance;         /// TX/RX switch position (Read by LEON during init)
  int dual_tx;                /// 1 for dual-antenna TX, 0 for single-antenna TX
  int tdd;                    /// 1 for TDD mode, 0 for FDD mode
  int tdd_config;
  int node_id;                /// Node type (Read by LEON during init)
  int freq_info;              /// Frequency info (Read by LEON during init)
  int frame_offset;           /// Frame offset (Read by LEON during init and on resynch procedure)
  int adac_cnt;               /// ADAC Interrupt counter (Written by LEON once per frame)
  int rx_gain_cval;           /// RX gain calibrated val (Read by LEON during init and every frame)
  int rx_gain_val;            /// RX gain val   (Read by LEON during init and every frame)
  int tx_gain00;             /// TX GAIN 00  (Read by LEON during init)
  int tx_gain01;             /// TX GAIN 01
  int tx_gain10;             /// TX GAIN 10
  int tx_gain11;             /// TX GAIN 11
  int tcxo_dac;               /// TCXO tuning voltage  (Read by LEON during init)
  int rx_rf_mode;             /// RX RF mode           (Read by LEON during init)
  int freq_offset;            /// Freq offset for compensation //20 bits for frequency (7.68e6/pow2(20)), 1 bit for direction
  int nb_rb;
  int first_rb;
  int mast_flag;
  int dac_dma_error_cnt;          /// PCI DMA error counter (Written by LEON)
  int adc_dma_error_cnt;          /// PCI DMA error counter (Written by LEON)
  //  cmd_t pci_cmd;              /// CMD register
  unsigned int ADF4108_Func0;
  unsigned int ADF4108_Ref_Cnt;
  unsigned int ADF4108_N_Cnt;
  unsigned int LFSW190410_CharCmd;
  unsigned int LFSW190410_KHZ_0;
  unsigned int LFSW190410_KHZ_1;
  unsigned int RFswitches_onoff;
  unsigned int RFswitches_mask;
  unsigned int settx_raw_word;
  unsigned int setrx_raw_word;
  unsigned int nb_posted_rfctl_ADF4108;
  unsigned int nb_posted_rfctl_LFSW;
  unsigned int nb_posted_rfctl_RFSW;
  unsigned int nb_posted_rfctl_SETTX;
  unsigned int nb_posted_rfctl_SETRX;
} PCI_interface_t;


typedef struct {
  unsigned int global_top_dma_ahb_addr;
  unsigned int one_dma_nbwords;
  unsigned int dma_pci_addr;
  unsigned int dma_ahb_addr;
  unsigned int dma_busy;
  unsigned int dma_direction;
} exmimo_pcidma_t;

typedef struct {
  int adc_head[4];            // PCI addresses of ADC buffers in PC memory (Read by LEON during init)
  int dac_head[4];            // PCI addresses of DAC buffers in PC memory (Read by LEON during init)
  unsigned int rf_freq0;
  unsigned int rf_freq1;
  unsigned int rf_freq2;
  unsigned int rf_freq3;
  unsigned int tx_gain00;
  unsigned int tx_gain01; 
  unsigned int tx_gain10; 
  unsigned int tx_gain11; 
  unsigned int tx_gain20;
  unsigned int tx_gain21; 
  unsigned int tx_gain30; 
  unsigned int tx_gain31; 
  unsigned int rx_gain00;
  unsigned int rx_gain01; 
  unsigned int rx_gain10; 
  unsigned int rx_gain11; 
  unsigned int rx_gain20;
  unsigned int rx_gain21; 
  unsigned int rx_gain30; 
  unsigned int rx_gain31; 
  unsigned int rf_mode0;
  unsigned int rf_mode1;
  unsigned int rf_mode2;
  unsigned int rf_mode3;
} exmimo_rf_t;

typedef struct {
  unsigned int cyclic_prefix_mode;
  unsigned int log2_ofdm_symbol_size;
  unsigned int samples_per_frame;
  unsigned int tx_rx_switch_point;
  unsigned int timing_advance;
  unsigned int frame_offset;
} exmimo_framing_t;

typedef struct {
  exmimo_rf_t rf;
  exmimo_framing_t framing;
} exmimo_pci_interface_t;

//#define PENDING_POSTED_RFCTL_LFSW     0x00000001
//#define PENDING_POSTED_RFCTL_ADF4108  0x00000002
//#define PENDING_POSTED_RFCTL_SETTX    0x00000003
//#define PENDING_POSTED_RFCTL_SETRX    0x00000004
//#define PENDING_POSTED_RFCTL_RFSW     0x00000005

#endif /* OPENAIR_PCI_H */
