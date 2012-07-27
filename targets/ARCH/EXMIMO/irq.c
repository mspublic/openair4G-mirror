/** exmimo_if.c
      - IRQ Handler: IRQ from Leon to PCIe/Kernel: exmimo_irq_handler
      - sends received packets to userspace

      - send command from PC to Leon and trigger IRQ on Leon
      - CONTROL1 commands are defined in ../pcie_defs.h
	  
   Authors: 
       Raymond Knopp <raymond.knopp@eurecom.fr>
       Matthias Ihmig <matthias.ihmig@mytum.de>, 2011 
   */

#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/swab.h>

#include "device.h"
#include "extern.h"

unsigned int intr_cnt=0;

void pcie_printk(void);


irqreturn_t openair_irq_handler(int irq, void *cookie) {

  unsigned int irqval;
  unsigned int irqcmd;

  // check interrupt status register
  //pci_read_config_word(pdev[0],6 , &irqval);
  
  // get AHBPCIE interrupt line
  irqval = ioread32(bar[0]);
  printk("irq hndl called:%i\n", irqval);

  if ((irqval&0x80) != 0) {
    // clear PCIE interrupt bit (bit 7 of register 0x0)
    iowrite32(irqval&0xffffff7f,bar[0]);
    irqcmd = ioread32(bar[0]+0x4);
    
    if (irqcmd == SLOT_INTERRUPT) {
      //	process_slot_interrupt();
      
      intr_cnt++;
    }
    else if (irqcmd == PCI_PRINTK) {
      printk("Got PCIe interrupt for printk ...\n");
      pcie_printk();
    }
    else if (irqcmd == GET_FRAME_DONE) {
      printk("Got PCIe interrupt for GET_FRAME_DONE ...\n");
  
    }
  }
  else {

  }

  return IRQ_HANDLED;
}
