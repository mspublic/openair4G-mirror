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
#include "pcie_interface.h"

extern char number_of_cards;

extern int major;

extern struct pci_dev *pdev[MAX_CARDS];
extern void __iomem *bar[MAX_CARDS];

extern void *bigshm_head[MAX_CARDS];
extern void *bigshm_currentptr[MAX_CARDS];
extern dma_addr_t bigshm_head_phys[MAX_CARDS];

extern dma_addr_t                      pphys_exmimo_pci_phys[MAX_CARDS]; 
extern exmimo_pci_interface_bot_t         *p_exmimo_pci_phys[MAX_CARDS];
extern exmimo_pci_interface_bot_virtual_t    exmimo_pci_kvirt[MAX_CARDS];

#endif // USER_MODE
#endif // __EXTERN_H__
