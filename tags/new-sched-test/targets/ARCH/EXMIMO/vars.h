#ifndef __VARS_H__
#define __VARS_H__
#ifndef USER_MODE
#define __NO_VERSION__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>

#ifdef KERNEL2_6
//#include <linux/config.h>
#include <linux/slab.h>
#endif

#include "defs.h"

unsigned int openair_irq;

u32 openair_irq_enabled=0;

struct pci_dev *pdev[4];

void __iomem *bar[4];
resource_size_t mmio_start,mmio_length;
unsigned int mmio_flags;

unsigned int vid,did;

char card,master_id;

int major;


//dma_addr_t dummy_dma_ptr;

unsigned int pci_buffer[4][2*4];
unsigned int mbox;

//unsigned short NODE_ID[1];
//EXPORT_SYMBOL(NODE_ID);

#endif

#include "pci.h"

char number_of_cards;

exmimo_pci_interface_bot *exmimo_pci_bot;
exmimo_pci_interface_t *exmimo_pci_interface;

#endif
