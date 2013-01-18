#ifndef EXMIMO_FW_H
#define EXMIMO_FW_H

#include "defs.h"

#define MAX_FIRMWARE_BLOCK_SIZE_B   262144
#define MAX_PRINTK_BUFFER_B           1024

#define TRX_CNT_SIZE_B                        (1*4)
#define ADAC_BUFFERSZ_PERCHAN_B  ((76800+2048)*1*4)
// 76800+2048: LTE frame+tail, *4 (7.68*4 MSPS), *4 Bytes/smp


// ------------------------------
// structures for communication between ExMIMO and kernel
// ------------------------------


typedef struct
{
    unsigned int firmware_block_ptr;
    unsigned int printk_buffer_ptr;
    unsigned int pci_interface_ptr;
} exmimo_pci_interface_bot_t;


// ------------------------------
// structures for communication between kernel and userspace
// ------------------------------

// this is for legacy applications
typedef struct
{
    char *TX_DMA_BUFFER[2];
    int *RX_DMA_BUFFER[2];

} TX_RX_VARS;


typedef struct
{
    unsigned int *rx_cnt_ptr[MAX_ANTENNA];
    unsigned int *tx_cnt_ptr[MAX_ANTENNA];
    unsigned int *adc_head[MAX_ANTENNA];
    unsigned int *dac_head[MAX_ANTENNA];
} exmimo_sharedmemory_vars_ptr_t;


typedef struct
{
    uint16_t bitstream_vendor_id;
    uint16_t bitstream_id;
    uint32_t bitstream_build_date;
    uint16_t software_vendor_id;
    uint16_t software_id;
    uint32_t software_build_date;
    uint16_t dsp_bitstream_vendor_id;
    uint16_t dsp_bitstream_id;
    uint32_t dsp_bitstream_build_date;
} exmimo_system_id_t;

typedef struct
{
    unsigned short board_vendor;
    unsigned short board_exmimoversion;
    unsigned short board_hwrev;
    unsigned short board_swrev;
    exmimo_system_id_t system_id;
} exmimo_id_t;

#define EXMIMO_PCIE_INIT            0x0000
#define EXMIMO_FW_INIT              0x0001
#define EXMIMO_CLEAR_BSS            0x0002
#define EXMIMO_START_EXEC           0x0003
#define EXMIMO_REBOOT               0x0004
#define EXMIMO_CONFIG               0x0005
#define EXMIMO_GET_FRAME            0x0006
#define EXMIMO_START_RT_ACQUISITION 0x0007
#define EXMIMO_STOP                 0x0008

#define SLOT_INTERRUPT 0x1111
#define PCI_PRINTK 0x2222
#define GET_FRAME_DONE 0x3333

void pci_fifo_printk(void);

#endif
