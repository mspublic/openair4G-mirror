#ifndef USER_MODE
#define __NO_VERSION__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>

#ifdef KERNEL2_6
//#include <linux/config.h>
#include <linux/slab.h>
#endif

#include "cbmimo1_pci.h"



unsigned int openair_irq;



struct pci_dev *pdev[4];

void __iomem *mmio_start,*mmio_length,*bar[4];
unsigned int mmio_flags;

unsigned int vid,did;

char card,master_id;

int major;


//dma_addr_t dummy_dma_ptr;

unsigned int pci_buffer[4][2*NB_ANTENNAS_RX];
unsigned int mbox;

PCI_interface_t *pci_interface[4];
exmimo_pci_interface_t *exmimo_pci_interface[4];

unsigned short NODE_ID[1];
//EXPORT_SYMBOL(NODE_ID);

#endif

char number_of_cards;


