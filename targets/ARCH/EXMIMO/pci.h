#ifndef OPENAIR_PCI_H
#define OPENAIR_PCI_H


/* This file is intended for prototypes & declarations to be SHARED with underlying hardware. */

#define PRIMARY_CH 0
#define SECONDARY_CH 1
#define NODE 2

#include "exmimo_fw.h"

typedef struct {
  unsigned int global_top_dma_ahb_addr;
  unsigned int one_dma_nbwords;
  unsigned int dma_pci_addr;
  unsigned int dma_ahb_addr;
  unsigned int dma_busy;
  unsigned int dma_direction;
} exmimo_pcidma_t;

typedef struct {
  uint32_t mbox;
  uint32_t adc_head[4];            // PCI addresses of ADC buffers in PC memory (Read by LEON during init)
  uint32_t dac_head[4];            // PCI addresses of DAC buffers in PC memory (Read by LEON during init)
  uint32_t rf_freq_rx0;
  uint32_t rf_freq_rx1;
  uint32_t rf_freq_rx2;
  uint32_t rf_freq_rx3;
  uint32_t rf_freq_tx0;
  uint32_t rf_freq_tx1;
  uint32_t rf_freq_tx2;
  uint32_t rf_freq_tx3;
  // Lime0 TX VGA1
  uint32_t tx_gain00;
  // Lime0 TX VGA2 
  uint32_t tx_gain01; 
  uint32_t tx_gain10; 
  uint32_t tx_gain11; 
  uint32_t tx_gain20;
  uint32_t tx_gain21; 
  uint32_t tx_gain30; 
  uint32_t tx_gain31; 
  // Lime0 RX VGA1
  uint32_t rx_gain00;
  // Lime0 RX VGA2
  uint32_t rx_gain01; 
  uint32_t rx_gain10; 
  uint32_t rx_gain11; 
  uint32_t rx_gain20;
  uint32_t rx_gain21; 
  uint32_t rx_gain30; 
  uint32_t rx_gain31; 
  //LIME RF modes
  // 21    | 20:19 | 18:16 |15:14  | 13:12|11:8 |  7    |6:3  |2      |1   |0   |
  // TXBYP | RXBYP | RF/BB |LNAMode| LNA  |RXLPF|RXLPFen|TXLPF|TXLPFen|TXen|RXen|
  uint32_t rf_mode0;
  uint32_t rf_mode1;
  uint32_t rf_mode2;
  uint32_t rf_mode3;
  // LIME LO Calibration Constants
  // | RXLOQ | RXLOI | TXLOQ | TXLOI |
  // | 23:18 | 17:12 | 11:6  | 5:0   |
  uint32_t rf_local0;
  uint32_t rf_local1;
  uint32_t rf_local2;
  uint32_t rf_local3;
  // LIME RX DC OFFSET
  // | RXDCQ | RXDCI |
  // | 15:8  | 7:0   |
  uint32_t rf_rxdc0;
  uint32_t rf_rxdc1;
  uint32_t rf_rxdc2;
  uint32_t rf_rxdc3;
  // LIME VCO Calibration Constants
  // | RXVCOCAP | TXVCOCAP |
  // | 11:6     | 5:0      |
  uint32_t rf_vcocal0;
  uint32_t rf_vcocal1;
  uint32_t rf_vcocal2;
  uint32_t rf_vcocal3;

  // LIME calibration parameters
} exmimo_rf_t;

#define TXEN 1
#define RXEN 2

#define TXLPFENMASK 4
#define TXLPFEN 4


#define TXLPFMASK     (15<<3)
#define TXLPF14       0
#define TXLPF10       (1<<3)
#define TXLPF7        (2<<3)
#define TXLPF6        (3<<3)
#define TXLPF5        (4<<3)
#define TXLPF4375     (5<<3)
#define TXLPF35       (6<<3)
#define TXLPF3        (7<<3)
#define TXLPF275      (8<<3)
#define TXLPF25       (9<<3)
#define TXLPF192      (10<<3)
#define TXLPF15       (11<<3)
#define TXLPF1375     (12<<3)
#define TXLPF125      (13<<3)
#define TXLPF0875     (14<<3)
#define TXLPF075      (15<<3)


#define RXLPFENMASK (1<<7)
#define RXLPFEN     128

#define RXLPFMASK   (15<<8)
#define RXLPF14     0
#define RXLPF10     (1<<8)
#define RXLPF7      (2<<8)
#define RXLPF6      (3<<8)
#define RXLPF5      (4<<8)
#define RXLPF4375   (5<<8)
#define RXLPF35     (6<<8)
#define RXLPF3      (7<<8)
#define RXLPF275    (8<<8)
#define RXLPF25     (9<<8)
#define RXLPF192    (10<<8)
#define RXLPF15     (11<<8)
#define RXLPF1375   (12<<8)
#define RXLPF125    (13<<8)
#define RXLPF0875   (14<<8)
#define RXLPF075    (15<<8)

#define LNAMASK (3<<12)
#define LNADIS  0
#define LNA1ON  (1<<12)
#define LNA2ON  (2<<12) 
#define LNA3ON  (3<<12)

#define LNAGAINMASK (3<<14)
#define LNAByp     (1<<14)
#define LNAMed     (2<<14)
#define LNAMax     (3<<14)

#define RFBBMASK   (7<<16)
#define RFBBNORM   0
#define RFBBRXLPF  (1<<16)
#define RFBBRXVGA  (2<<<16)
#define RFBBOUTPUT (3<<16)
#define RFBBLNA1   (4<<16)
#define RFBBLNA2   (5<<16)
#define RFBBLNA3   (6<<16)

#define TXLPFMODEMASK (1<<21)
#define TXLPFNORM     0
#define TXLPFBYP      (1<<21)

#define RXLPFMODEMASK (3<<19)
#define RXLPFNORM     0
#define RXLPFBYP      (1<<19)
#define RXLPFBPY2     (3<<19)

#define TXLOIMASK 63
#define TXLOQMASK (63<<6)
#define RXLOIMASK (63<<12)
#define RXLOQMASK  (63<<18)

typedef struct {
  uint32_t tdd;
  uint32_t tdd_config;
  uint32_t eNB_flag;
} exmimo_framing_t;

typedef struct {
  //  uint32_t mbox[4];
  exmimo_rf_t rf;
  exmimo_framing_t framing;
} exmimo_pci_interface_t;

#endif /* OPENAIR_PCI_H */
