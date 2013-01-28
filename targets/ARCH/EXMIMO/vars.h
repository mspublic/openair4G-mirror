#ifndef __VARS_H__
#define __VARS_H__

#ifndef USER_MODE
#define __NO_VERSION__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>

#ifdef KERNEL2_6
#include <linux/slab.h>
#endif

#include "defs.h"
#include "pci.h"

unsigned int openair_irq_enabled[MAX_CARDS] = INIT_ZEROS;
unsigned int openair_chrdev_registered = 0;
unsigned int openair_pci_device_enabled[MAX_CARDS] = INIT_ZEROS;

struct pci_dev *pdev[MAX_CARDS] = INIT_ZEROS;
void __iomem *bar[MAX_CARDS] = INIT_ZEROS;

// card and bitstream IDs, set through PCI subsystem ID and an APB block
exmimo_id_t exmimo_id[MAX_CARDS];

resource_size_t mmio_start[MAX_CARDS] = INIT_ZEROS, mmio_length[MAX_CARDS];
unsigned int    mmio_flags[MAX_CARDS];

int major;

#endif

char number_of_cards;

void *bigshm_head[MAX_CARDS] = INIT_ZEROS;
void *bigshm_currentptr[MAX_CARDS];
dma_addr_t bigshm_head_phys[MAX_CARDS] = INIT_ZEROS;

unsigned long bigshm_size_pages;

exmimo_sharedmemory_vars_ptr_t exmimo_shm_vars_kvirt[MAX_CARDS]; // structure containing kvirt pointers to shared mem vars
exmimo_sharedmemory_vars_ptr_t exmimo_shm_vars_phys[MAX_CARDS]; // structure containing DMA physical pointers to shared mem vars

exmimo_pci_interface_bot_t *exmimo_pci_bot_ptr[MAX_CARDS];     // Kernel virtual pointer to shared pci_bot structure
dma_addr_t                  exmimo_pci_bot_physptr[MAX_CARDS]; // (Physical) DMA address to shared pci_bot structure

// kernel virtual pointers to fw-, printk- and pci_interface blocks
char *exmimo_firmware_block_ptr[MAX_CARDS];
char *exmimo_printk_buffer_ptr[MAX_CARDS];
exmimo_pci_interface_t *exmimo_pci_interface_ptr[MAX_CARDS];


#endif
