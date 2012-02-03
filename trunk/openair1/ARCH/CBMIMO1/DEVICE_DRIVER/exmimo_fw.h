#ifndef EXMIMO_FW_H
#define EXMIMO_FW_H

typedef struct {
  unsigned int firmware_block_ptr;
  unsigned int printk_buffer_ptr;
  unsigned int pci_interface_ptr;
} exmimo_pci_interface_bot;

#define EXMIMO_PCIE_INIT  0x0000
#define EXMIMO_FW_INIT    0x0001
#define EXMIMO_CLEAR_BSS  0x0002
#define EXMIMO_START_EXEC 0x0003
#define EXMIMO_REBOOT     0x0004
#define EXMIMO_CONFIG     0x0005
#define EXMIMO_GET_FRAME  0x0006

#define SLOT_INTERRUPT 0x1111
#define PCI_PRINTK 0x2222

void pci_fifo_printk();

#endif
