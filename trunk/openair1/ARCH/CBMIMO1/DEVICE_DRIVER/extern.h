#ifndef USER_MODE
#define __NO_VERSION__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>

#ifdef KERNEL2_6
//#include <linux/config.h>
#include <linux/slab.h>
#endif

//#include "pci_commands.h"
#include "cbmimo1_pci.h"



extern struct pci_dev *pdev[4];
extern unsigned long bar[4],iobar[4],bar_len[4];

extern char card,master_id;

extern int major;

extern unsigned short eedata[];

extern unsigned int openair_irq;


//extern dma_addr_t dummy_dma_ptr;

extern unsigned int pci_buffer[4][2*NB_ANTENNAS_RX];
extern unsigned int RX_DMA_BUFFER[4][NB_ANTENNAS_RX];
extern unsigned int TX_DMA_BUFFER[4][NB_ANTENNAS_TX];
extern unsigned int mbox;

extern PCI_interface_t *pci_interface[4];

extern unsigned short NODE_ID[1];
#endif

extern char number_of_cards;
