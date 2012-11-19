#include "extern.h"
#include "defs.h"

#define PCI_FIFO_NO 60
#define PCI_FIFO_MAX_STRING_SIZE 500
#define PCI_FIFO_PRINTF_SIZE 1024

unsigned int exmimo_firmware_block_ptr, exmimo_printk_buffer_ptr,exmimo_pci_bot_phys=0;
void exmimo_firmware_init() {
  size_t size=0;

  if (exmimo_pci_bot_phys == 0) {
    // increase exmimo_pci_interface_bot to multiple of 128 bytes 
    size = sizeof(exmimo_pci_interface_bot);
    size = size >> 7;
    size++;
    size = size << 7;
    
    exmimo_pci_bot = (exmimo_pci_interface_bot *)pci_alloc_consistent(pdev[0], 
								      size,
								      &exmimo_pci_bot_phys);
    
    printk("Intializing EXMIMO firmware support (exmimo_pci_bot at %p, phys %x)\n",exmimo_pci_bot,exmimo_pci_bot_phys);
    
    exmimo_firmware_block_ptr = (unsigned int )pci_alloc_consistent(pdev[0], 
								    262144,
								    &exmimo_pci_bot->firmware_block_ptr);
    
    printk("firmware_code_block_ptr : %x\n",exmimo_pci_bot->firmware_block_ptr);
    
    exmimo_printk_buffer_ptr  = (unsigned int)pci_alloc_consistent(pdev[0], 
								   1024,
								   &exmimo_pci_bot->printk_buffer_ptr);
    
    printk("printk_buffer_ptr : %x\n",exmimo_pci_bot->printk_buffer_ptr);
    
    exmimo_pci_interface  = (exmimo_pci_interface_t *)pci_alloc_consistent(pdev[0], 
									   sizeof(exmimo_pci_interface_t),
									   &exmimo_pci_bot->pci_interface_ptr);
  }

  iowrite32((unsigned int)exmimo_pci_bot_phys,(bar[0]+0x1c));
  iowrite32(0,(bar[0]+0x20));

  openair_dma(0,EXMIMO_PCIE_INIT);

}

void exmimo_firmware_cleanup() {

  size_t size=0;

  // increase exmimo_pci_interface_bot to multiple of 128 bytes 
  size = sizeof(exmimo_pci_interface_bot);
  size = size >> 7;
  size++;
  size = size << 7;

  pci_free_consistent(pdev[0], sizeof(exmimo_pci_interface_t),(void*)exmimo_pci_interface,(dma_addr_t)exmimo_pci_bot->pci_interface_ptr);
  pci_free_consistent(pdev[0],1024,(void *)exmimo_printk_buffer_ptr,(dma_addr_t)exmimo_pci_bot->printk_buffer_ptr);
  pci_free_consistent(pdev[0],262144,(void *)exmimo_firmware_block_ptr,(dma_addr_t)exmimo_pci_bot->firmware_block_ptr);
  pci_free_consistent(pdev[0],size,(void *)exmimo_pci_bot,(dma_addr_t)exmimo_pci_bot_phys);
}
void pcie_printk() {

  char *buffer = (char*) exmimo_printk_buffer_ptr;
  unsigned int len = ((unsigned int *)buffer)[0];
  unsigned int off=0,i;
  unsigned char *dword;
  unsigned char tmp;

  printk("In pci_fifo_printk : buffer %p, len %d\n",buffer,len);

  if (len<256) {
    if ((len&3)>0)
      off=1;
    
    for (i=0;i<(off+(len>>2));i++) {
      dword = &((unsigned char *)buffer)[(1+i)<<2];
      tmp = dword[3];
      dword[3] = dword[0];
      dword[0] = tmp;
      tmp = dword[2];
      dword[2] = dword[1];
      dword[1] = tmp;
    }
    for (i=0;i<len;i++)
      printk("%c",((char*)&buffer[4])[i]);

  }
}

