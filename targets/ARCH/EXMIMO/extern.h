#ifndef __EXTERN_H__
#define __EXTERN_H__
#ifndef USER_MODE
#define __NO_VERSION__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>

#ifdef KERNEL2_6
#include <linux/slab.h>
#endif

#include "defs.h"


extern struct pci_dev *pdev[4];
extern void __iomem *bar[4];


extern char card,master_id;

extern int major;

extern unsigned short eedata[];

extern unsigned int openair_irq;

extern u32 openair_irq_enabled;

//extern dma_addr_t dummy_dma_ptr;

extern unsigned int pci_buffer[4][4*4];
extern unsigned int RX_DMA_BUFFER[4][4];
extern unsigned int TX_DMA_BUFFER[4][4];
extern unsigned int mbox;

extern unsigned int vid,did;

//extern unsigned short NODE_ID[1];

#include "pci.h"

extern char number_of_cards;

extern exmimo_pci_interface_bot *exmimo_pci_bot;
extern exmimo_pci_interface_t *exmimo_pci_interface;

#endif
#endif
