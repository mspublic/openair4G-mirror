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
#include "pci.h"

extern struct pci_dev *pdev[MAX_CARDS];
extern void __iomem *bar[MAX_CARDS];

extern exmimo_id_t exmimo_id[MAX_CARDS];

extern int major;

//extern unsigned int openair_irq_enabled;

extern unsigned long bigshm_size_pages;

extern void *bigshm_head[MAX_CARDS];
extern void *bigshm_currentptr[MAX_CARDS];
extern dma_addr_t bigshm_head_phys[MAX_CARDS];

//extern unsigned int RX_DMA_BUFFER_kptr[MAX_CARDS][4];
//extern unsigned int TX_DMA_BUFFER_kptr[MAX_CARDS][4];
//extern unsigned int mbox_kptr[MAX_CARDS];


extern char number_of_cards;

extern exmimo_sharedmemory_vars_ptr_t exmimo_shm_vars_kvirt[MAX_CARDS];
extern exmimo_sharedmemory_vars_ptr_t exmimo_shm_vars_phys[MAX_CARDS];

extern exmimo_pci_interface_bot_t *exmimo_pci_bot_ptr[MAX_CARDS];       // kvirt pointer to structure containing phys ptr in shared mem
extern dma_addr_t                  exmimo_pci_bot_physptr[MAX_CARDS];  // phys  pointer to pci_bot structure in shared mem

extern char *exmimo_firmware_block_ptr[MAX_CARDS];
extern char *exmimo_printk_buffer_ptr[MAX_CARDS];
extern exmimo_pci_interface_t     *exmimo_pci_interface_ptr[MAX_CARDS];       // kvirt pointer to pci_intf structure in shared mem


#endif // USER_MODE
#endif // __EXTERN_H__
